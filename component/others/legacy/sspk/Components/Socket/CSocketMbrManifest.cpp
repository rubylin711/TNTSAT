///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <zlib.h>
#include <string>
#include <AutoRefPtr.h>

#include "CSocketMbrManifest.h"
#include "CAVEngineConfiguration.h"
#include "CDecoderConfiguration.h"
#include "CSocketMbrConfiguration.h"
#include "CSocketMbrDiags.h"
#include "CTrickConfiguration.h"

#include "IReferenceClock.h"
#include "Base64.h"
#include "MarshallingUtils.h"
#include "StringUtils.h"
#include "Trace.h"
#include "SocketDiagEvents.h"
#include "ITunerSession.h"
#include "ManifestParser.h"
#include "IXDrm.h"

using namespace std;
using namespace MBR;

#define _DUMP_MANIFEST_

// #define SOCKETMBRMANIFEST_SPEW
#ifdef SOCKETMBRMANIFEST_SPEW
#define SOCKETMBRMANIFEST_TRACE(x) TRACE(x)
#else
#define SOCKETMBRMANIFEST_TRACE(x)
#endif

// #define SOCKETMBRMANIFEST_SPEW_VERBOSE
#ifdef SOCKETMBRMANIFEST_SPEW_VERBOSE
#define SOCKETMBRMANIFEST_TRACE_VERBOSE(x) TRACE(x)
#else
#define SOCKETMBRMANIFEST_TRACE_VERBOSE(x)
#endif


// ===============================================================================================================
// Fragment info update
// ===============================================================================================================

// If playing near live point, let the main socket thread update chunks
// using the standard fragment headers, which is more efficient.
// However, if falling too far back (e.g. during a pause or skip back),
// re-enable the chunk update thread.
#define CHUNK_UPDATE_THRESHOLD_SECS           5

//    The maximum delay between expected live time and the
//    last updated chunk that triggers enabling fragment info reads.
#define CHUNK_UPDATE_STREAMS_SECS             2
#define CHUNK_UPDATE_POLL_PERIOD_STOPPED_MS   5000 // 5 sec
#define CHUNK_UPDATE_POLL_PERIOD_DEFAULT_MS   250

#define CHUNK_UPDATE_MAX_REQUESTS  5

// Actual time to wait for chunk info to be received is based on number of streams
static const int c_CHUNK_INFO_TIMEOUT_MS_LIMIT = 100;
static const int c_SEC_TO_MS = 1000;
    
static const uint32 c_DEFAULT_CHUNK_INFO_BLOCK_SIZE = (4 * 1024);
static const uint32 c_MAX_CHUNK_INFO_BLOCK_SIZE = (4 * c_DEFAULT_CHUNK_INFO_BLOCK_SIZE);

// ===============================================================================================================
// custom alloc routines for zlib (defaults have been known to cause instability)
// ===============================================================================================================

static voidpf zlib_malloc(voidpf o, unsigned items, unsigned size)
{
    return static_cast<voidpf>(NEW_NO_THROW char[items * size]);
}

static void zlib_free(voidpf o, voidpf ptr)
{
    char *p=static_cast<char*>(ptr);
    if(p)
    {
        delete [] p;
    }
}

// ===============================================================================================================
// custom comparer for quality levels with different audio codecs
// ===============================================================================================================

static StreamType AudioCodecPriority[] = { StreamType_Audio_AC3, StreamType_Audio_AAC, StreamType_Audio_WMAPRO, StreamType_Audio_WMA };

class CAudioCodecQualityLevelComparer : public IQualityLevelComparer
{
public:

    __override int Compare(
                    _In_ CManifestTrack* pt1,
                    _In_ CManifestTrack* pt2 )
    {
        // Use higher codec priority,
        // else prioritize by higher bitrate

        int pri1 = GetCodecPriority( pt1 );
        int pri2 = GetCodecPriority( pt2 );

        if( pri1 == pri2 )
        {
            pri1 = (int)pt1->Bitrate();
            pri2 = (int)pt1->Bitrate();
        }

        return( pri1 - pri2 );
    }

private:

    int GetCodecPriority( _In_ CManifestTrack* pTrack )
    {
        // Return lowest priority if codec not enabled
        if (!gDecoderConfiguration.IsEnabled(pTrack->StreamType()))
            return 0;

        int count = sizeof(AudioCodecPriority) / sizeof(StreamType);

        for (int i = 0; i < count; i++)
        {
            if (pTrack->StreamType() == AudioCodecPriority[i])
                return count - i;
        }

        // Return lowest priority if no match
        return 0;
    }
};

// ===============================================================================================================
// ===============================================================================================================

CSocketMbrManifest::CSocketMbrManifest(IAVManager* avManager, uint32 pipeIdN)
    : _socketAVManager(avManager)
    , _socketPipeIdN(pipeIdN)
    , _pStreamerHttp(NULL)
    , _apChunkManifest(NULL)
    , _manifestSize(0)
    , _endOfManifest(false)
    , _socketError(eSocketErrorNone)
    , _socketPKResult(pkS_OK)
    , _socketHttpResponse(0)
    , _zStreamInit(false)
    , _zBuffer(NULL)
{
}

CSocketMbrManifest::~CSocketMbrManifest()
{
    ResetDeflate();
    Cleanup();
}

void CSocketMbrManifest::ResetDeflate()
{
    if (_zStreamInit)
    {
        inflateEnd(&_zStream);
        _zStreamInit = false;
    }

    if (_zBuffer)
    {
        delete [] _zBuffer;
        _zBuffer = NULL;
    }
}
bool CSocketMbrManifest::DownloadManifest(_Inout_ std::string* pStrUrl)
{
    if (NULL == _apChunkManifest)
    {
        ASSERT(false);
        TRACE_ERROR(("CSocketMbrManifest::DownloadManifest -- Need to set _apChunkManifest first!!!"));
        return false;
    }

    if (NULL != _pStreamerHttp)
    {
        ASSERT(false);
        TRACE_ERROR(("CSocketMbrManifest::DownloadManifest -- _pStreamerHttp already set!!!"));
        Cleanup();
        return false;
    }

    _pStreamerHttp = IStreamerHttp::CreateStreamerHttp();
    if (NULL == _pStreamerHttp)
    {
        ASSERT(false);
        TRACE_ERROR(("CSocketMbrManifest::DownloadManifest -- CreateStreamerHttp failed!!!"));
        return false;
    }

    //Break out if already asked to disconnect
    if (!IsConnected())
    {
        return false;
    }

    // we support raw deflate compression
    string httpHeader = "Accept-Encoding: deflate\r\n";
    string response;

    // connect to the manifest
    if(!_pStreamerHttp->Connect( pStrUrl, httpHeader, &response, IStreamerHttp::eRedirectEnable))
    {
        SOCKETMBRMANIFEST_TRACE(("CSocketMbrManifest::DownloadManifest - failed to retrieve manifest %s: %s\n", pStrUrl->c_str(), response.c_str()));
        _socketHttpResponse = _pStreamerHttp->GetHttpResponse();
        _socketPKResult = _pStreamerHttp->GetPKResult();
        return false;
    }
    else
    {
        SOCKETMBRMANIFEST_TRACE(("CSocketMbrManifest::DownloadManifest - manifest %s: %s\n", pStrUrl->c_str(), response.c_str()));
    }
    return true;
}

bool CSocketMbrManifest::DownloadParseDefaultManifest(CTuneRequest& tuneRequest, uint64* pConnectResponseTime)
{
    string sourceUrl = tuneRequest.GetArg(TUNE_REQUEST_SOURCEURL);
    bool ret = DownloadManifest(&sourceUrl);
    if (ret)
    {
        if (NULL != pConnectResponseTime && NULL != _socketAVManager->GetReferenceClock())
        {
            *pConnectResponseTime = _socketAVManager->GetReferenceClock()->GetTime();
        }

        string encodingStr;
        bool deflateEncoding = _pStreamerHttp->GetResponseHeader("Content-Encoding", &encodingStr) && (encodingStr == "deflate");

        _apChunkManifest->SetManifestUrl( Str2WStr(sourceUrl).c_str() );

        _endOfManifest = false;

        CManifestParser manifestParser(_apChunkManifest);
        pkRESULT pkResult = manifestParser.Parse((HANDLE) this, deflateEncoding ? &CSocketMbrManifest::ReadDeflateManifestCB : &CSocketMbrManifest::ReadManifestCB);

        if (pkFAILED(pkResult))
        {
            // if socket error is already set, leave it as it.
            // otherwise, set socket error to indicate parsing failure.
            if (_socketError == eSocketErrorNone)
            {
                _socketError = eSocketErrorSSManifestParsingFailed;
            }
            _socketPKResult = pkResult;
            ret = false;
        }
    }
    else
    {
        _socketError = _pStreamerHttp->GetSocketError();

        //be more specific on the context of the error
        if(eSocketErrorHttpInvalidResult == _socketError)
        {
            _socketError = eSocketErrorSSManifestHttpInvalid;
        }
    }

    return ret;
}

bool CSocketMbrManifest::DownloadParseSegmentManifest(_In_ const std::wstring& wstrUrl)
{
    string strUrl = WStr2Str(wstrUrl);
    bool ret = DownloadManifest(&strUrl);
    if (ret)
    {
        string encodingStr;
        bool deflateEncoding = _pStreamerHttp->GetResponseHeader("Content-Encoding", &encodingStr) && (encodingStr == "deflate");

        _endOfManifest = false;

        CManifestParser manifestParser(_apChunkManifest);
        pkRESULT pkResult = manifestParser.Parse((HANDLE) this, 
                                                 deflateEncoding ? &CSocketMbrManifest::ReadDeflateManifestCB : &CSocketMbrManifest::ReadManifestCB,
                                                 eManifestMode_Subsequent);

        if (pkFAILED(pkResult))
        {
            _socketPKResult = pkResult;
            ret = false;
        }
    }

    return ret;
}

bool CSocketMbrManifest::IsConnected(void) const
{
    return( (NULL != _pStreamerHttp) ? _pStreamerHttp->IsConnected() : false);
}

void CSocketMbrManifest::Cleanup(void)
{
    //Dump the socket if we have any
    if (NULL != _pStreamerHttp)
    {
        //Make sure the socket is disconnected
        if (_pStreamerHttp->IsConnected())
        {
            _pStreamerHttp->Close();
        }

        //Dump it
        delete _pStreamerHttp;
        _pStreamerHttp = NULL;
    }
}

bool CSocketMbrManifest::GetSessionIdHeader(std::string& sessionId)
{
    return (NULL != _pStreamerHttp)
        && _pStreamerHttp->GetResponseSessionIdHeader( &sessionId );
}

// XML parser stream reader callbacks
void CSocketMbrManifest::ReadManifestCB(_In_ HANDLE hStream, _Out_bytecap_(bufferSize) byte* pBuffer, _In_ uint32 bufferSize, _Out_ uint32* pLen)
{
    CSocketMbrManifest* pSocket = (CSocketMbrManifest*)hStream;
    *pLen = pSocket->ReadManifest(pBuffer, bufferSize);
}

void CSocketMbrManifest::ReadDeflateManifestCB(_In_ HANDLE hStream, _Out_bytecap_(bufferSize) byte* pBuffer, _In_ uint32 bufferSize, _Out_ uint32* pLen)
{
    CSocketMbrManifest* pSocket = (CSocketMbrManifest*)hStream;
    *pLen = pSocket->ReadDeflateManifest(pBuffer, bufferSize);
}

// Stream reader for clear text manifests
int CSocketMbrManifest::ReadManifest(_Out_bytecap_(len) byte* pBuffer, _In_ int len)
{
    if (!IsConnected())
        return 0;

    if ( _endOfManifest )
    {
        return 0;
    }

    len = RecvCount(pBuffer, len);
    if (len <= 0)
    {
        len = 0;
    }

    return len;
}

// Stream reader for 'raw deflate' compressed manifests
int CSocketMbrManifest::ReadDeflateManifest(_Out_bytecap_(len) byte* pBuffer, _In_ int len)
{
    int ret = Z_OK;

    if (!_zStreamInit)
    {
        memset(&_zStream, 0, sizeof(z_stream));
        _zStream.zalloc = zlib_malloc;
        _zStream.zfree = zlib_free;

        _zBuffer = NEW_NO_THROW byte[ZBUFFER_SIZE];
        if (_zBuffer == NULL)
        {
            SOCKETMBRMANIFEST_TRACE(("CSocketMbrManifest::ReadManifestDeflateCB() - out of memory!"));
            goto bail_err;
        }

        // Initialize zlib (-15 = raw inflate with 32K window)
        ret = inflateInit2(&_zStream, -15);
        if (ret != Z_OK)
        {
            SOCKETMBRMANIFEST_TRACE(("CSocketMbrManifest::ReadManifestDeflateCB() - inflateInit2() err = %d", ret));
            goto bail_err;
        }

        _zStreamInit = true;
    }

    _zStream.next_out = pBuffer;
    _zStream.avail_out = len;

    // Loop until output buffer is full or stream is finished
    while (_zStream.avail_out > 0)
    {
        // Do we need to refill the input buffer?
        if (!_zStream.avail_in)
        {
            _zStream.next_in = _zBuffer;
            _zStream.avail_in = ReadManifest(_zBuffer, ZBUFFER_SIZE);

            // Are we out of data?
            if (!_zStream.avail_in)
                break;
        }

        ret = inflate(&_zStream, Z_SYNC_FLUSH);
        if ((ret != Z_OK) && (ret != Z_STREAM_END))
        {
            SOCKETMBRMANIFEST_TRACE(("CSocketMbrManifest::ReadManifestDeflateCB() - inflate() err = %d ('%s')", ret, _zStream.msg));
            goto bail_err;
        }
    }

    return len - _zStream.avail_out;

bail_err:
    ResetDeflate();
    return 0;
}

int CSocketMbrManifest::RecvCount(_Out_bytecap_(dstlen) byte* dst, _In_ int dstlen, _In_opt_ int timeout/* = 0*/)
{
    bool endOfResponse = false;
    int bytesRead = 0;
    if (!IsConnected())
    {
        endOfResponse = true;
        goto exit;
    }

    while (!endOfResponse && bytesRead < dstlen)
    {
        int c = _pStreamerHttp->Recv(dst + bytesRead, dstlen - bytesRead, &endOfResponse, timeout);
        if (c <= 0)
        {
            return c;
        }
        bytesRead += c;
    }

exit:
    _endOfManifest = endOfResponse;
    return bytesRead;
}

// ===============================================================================================================
// ===============================================================================================================

CMbrManifest::CMbrManifest(IAVManager* avManager, uint32 pipeIdN, CTuneRequest& tuneRequest)
    : _socketAVManager(avManager)
    , _referenceClock(avManager->GetReferenceClock())
    , _socketPipeIdN(pipeIdN)
    , _chunkUpdateThread(NULL)
    , _chunkUpdateThreadState(eUpdateThreadState_Init)
    , _chunkUpdateThreadExitFlag(false)
    , _chunkUpdateThreadWakeEvent(CEvent::eResetModeAuto)
    , _chunkRequestLatestEnabled(true)
    , _apChunkManifest(NULL)
    , _chunkInfoReader(_socketAVManager, _socketPipeIdN)
    , _manifestSocket(_socketAVManager, _socketPipeIdN)
    , _socketError(eSocketErrorNone)
    , _socketPKResult(pkS_OK)
    , _socketHttpResponse(0)
    , _pDrm(NULL)
    , _streamTimeDelta(0)
    , _fDecrypterOnDemand(true)
    , _lastTimeStreamsUpdated(0)
    , _lastReportedChunkEndTime(0)
    , _callStartEndTimeCallback(true)
    , _endOfLive(false)
    , _endOfLiveReported(false)
    , _recvTimeout(c_CHUNK_INFO_TIMEOUT_MS_LIMIT)
    , _manifestReady(false)
    , _pPlaybackRangeCallback(NULL)
    , _pSegmentFetcher(NULL)
{
    _requestedMinTime.ResetTicks();
    _tuneRequest = tuneRequest;

    memset(&_drmOPLData, 0, sizeof(_drmOPLData));
    _drmOPLData.HDCPAction = XDRM_OPL_ENABLE_ALWAYS;
}

CMbrManifest::~CMbrManifest()
{
    // delete the fetcher before the sockets are closed
    CSegmentManifestFetcher* pSegmentFetcher = NULL;  
    {
        AutoLock lock(&_setPlaybackRangeLock);
        pSegmentFetcher = _pSegmentFetcher;
        _pSegmentFetcher = NULL;
    }

    if(pSegmentFetcher)
    {
        delete pSegmentFetcher;
    }

    CleanupManifestSocket();

    SAFE_DELETE(_chunkUpdateThread);

    if (_pDrm)
    {
        _socketAVManager->GetDrmManager()->ReleaseDecrypter(_pDrm);
    }
}

void CMbrManifest::CleanupManifestSocket()
{
    _manifestSocket.Cleanup();
}

pkRESULT CMbrManifest::InitDrmWithHeader(byte *pbHeader, uint32 cbHeader, byte *pbKeyId, uint32 cbKeyId)
{
    pkRESULT pkResult = pkS_OK;

    // Release the current decrypter reference if it exists.
    if (_pDrm)
    {
        _socketAVManager->GetDrmManager()->ReleaseDecrypter(_pDrm);
        _pDrm = NULL;
    }

    // Initialize IDrmDecrypter
    _pDrm = _socketAVManager->GetDrmManager()->GetDecrypter(
        DrmDecryptionMode_SSProtectionHeader, cbHeader, pbHeader, cbKeyId, pbKeyId, this);
    pkResult = _pDrm ? pkS_OK : pkS_FALSE;

    return pkResult;
}

pkRESULT CMbrManifest::InitDrm()
{
    _fDecrypterOnDemand = true;

    if (_apChunkManifest == NULL)
        return pkS_FALSE;

    pkRESULT pkResult = pkS_OK;
    wstring wstrValue;
    if (_apChunkManifest->GetAttribute(MBR_MS_BLOB_PLAYREADY_OBJECT, &wstrValue))
    {
        string s = WStr2Str(wstrValue);

        // Base64 decoded blob will be 75% the size of encoded string
        uint32 dwLen = s.length();
        byte *pBlob = NEW_NO_THROW byte[dwLen];
        if (!pBlob)
        {
            ASSERT(false);
            return pkE_OUTOFMEMORY;
        }

        // Convert base64 encoded string into byte array
        Base64DecodeExA(s.c_str(), s.length(), pBlob, (DWORD*) &dwLen, NULL);
        pBlob[dwLen] = 0;

        if (!_socketAVManager->GetDrmManager()->IsDecrypterOnDemand(
                DrmDecryptionMode_SSProtectionHeader, dwLen, pBlob))
        {
            _fDecrypterOnDemand = false;
            pkResult = InitDrmWithHeader(pBlob, dwLen, NULL, 0);
        }
        delete[] pBlob;
    }

    return pkResult;
}

void CMbrManifest::InitAudioLanguage()
{
    if (_apChunkManifest == NULL)
    {
        return;
    }

    // collect bitrate and audio language info
    _streamInfoList.Clear();
    for (uint32 i = 0; i < _apChunkManifest->m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pStreamInfo = _apChunkManifest->m_availableStreams[i];
        if (pStreamInfo->Type() == MediaStreamTypeAudio)
        {
            CAudioCodecQualityLevelComparer* pComparer = NEW_NO_THROW CAudioCodecQualityLevelComparer;
            if (!pComparer)
            {
                ASSERT(false);
                return;
            }
            pStreamInfo->SetComparer(pComparer);
        }

        // pass in stream info to status formatter
        AddLanguage(pStreamInfo);
    }
}

void CMbrManifest::AddLanguage(CMediaStreamDescription *pStreamInfo)
{
    CStreamInfo item;

    switch (pStreamInfo->Type())
    {
    case MediaStreamTypeAudio:
        {
            uint32 len = 0;
            uint32 iMBR = pStreamInfo->GetDefaultQualityLevel();
            WAVEFORMATEX* pWaveFormatEx = (WAVEFORMATEX*)pStreamInfo->GetCodecBlob(iMBR, &len);

            string langString(pStreamInfo->Language().empty() ? string("") : WStr2Str(pStreamInfo->Language()));
            //uint32 dwFormat = pWaveFormatEx ? pWaveFormatEx->wFormatTag : pStreamInfo->GetFourCC(iMBR);
            uint32 dwFormat = pStreamInfo->GetFourCC(iMBR);
            item.SetAudioDescriptor((int)pStreamInfo->GetStreamID(), dwFormat, langString);
        }
        break;

    case MediaStreamTypeVideo:
        {
            item.SetVideoDescriptor(
                        pStreamInfo->GetStreamID(),
                        pStreamInfo->GetFourCC(0)
                        );
        }
        break;

    case MediaStreamTypeText:
        {
            MediaStreamSubType subType = pStreamInfo->GetSubType();
            if ((subType == MediaStreamSubTypeSubtitles) ||
                (subType == MediaStreamSubTypeCaptions) ||
                (subType == MediaStreamSubTypeDescriptions))
            {
                string langString(pStreamInfo->Language().empty() ? string("") : WStr2Str(pStreamInfo->Language()));
                item.SetSubtitleDescriptor((int)pStreamInfo->GetStreamID(), langString);
                break;
            }
        }
        // else unhandled text type so fall through to default
        __fallthrough;

    default:
        // unknown stream type
        return;
    }

    _streamInfoList.AddStreamInfo(item);
}

pkRESULT CMbrManifest::GetSegmentManifestURL(_In_ TimeSpan_hns chunkStartTime, _Out_ std::wstring* pOutUrl )
{
    // convert to timescale
    TimeSpanGeneric timeScaleTime;
    timeScaleTime.Set(chunkStartTime.Ticks(), chunkStartTime.TicksPerSecond(), _apChunkManifest->TimeScale());
    return _apChunkManifest->GetSegmentedManifestURL(timeScaleTime.Ticks(), pOutUrl);
}

bool CMbrManifest::DownloadParseSegmentManifest( _In_ std::wstring& url, _Out_ int* pHttpResponse )
{
    bool manifestDownloaded = false;
    if( NULL == pHttpResponse || NULL == _apChunkManifest )
    {
        goto exit;
    }

    manifestDownloaded = _manifestSocket.DownloadParseSegmentManifest(url);
    if (!manifestDownloaded)
    {
        *pHttpResponse = _manifestSocket.GetHttpResponse();
    }

    CleanupManifestSocket();
exit:
    return manifestDownloaded;
}

bool CMbrManifest::Prepare(void)
{
    bool ret = true;

    if (NULL == _apChunkManifest)
    {    
        if (FAILED(CChunkManifest::CreateInstance(_apChunkManifest.DerefOutPtr())))
        {
            ASSERT(false);
            ret = false;
            goto exit;
        }

        _manifestSocket.SetChunkManifest(_apChunkManifest);
        _apChunkManifest->SetManifestUpdateManager(_socketAVManager->GetManifestUpdateManager());

        // store media room specific tune-request-parameters.
        StoreExtraTuneParameters();

        uint64 manifestRequestTime = _referenceClock->GetTime();

        ret = _manifestSocket.DownloadParseDefaultManifest(_tuneRequest, &manifestRequestTime);
        if (!ret || !_manifestSocket.IsConnected())
        {
            _socketError = _manifestSocket.GetSocketError();
            _socketPKResult = _manifestSocket.GetPKResult();
            _socketHttpResponse = _manifestSocket.GetHttpResponse();
        }
        else
        {
            pkRESULT pkResult = pkS_OK;

            // get the tv2-session-id from response
            StoreSessionId();

            // suppress support for version 0.x
            if (_apChunkManifest->MajorVersion() == 0)
            {
                pkResult = pkE_FAIL;
                _socketError = eSocketErrorSSManifestVersionUnsupported;
            }
            else
            {
                // this doesn't just validate, it fixes up the chunk info data
                pkResult = _apChunkManifest->ValidateManifest();

                if (pkResult == pkS_OK)
                {
                    // Select default streams
                    pkResult = _apChunkManifest->SelectInitialStreams();
                }

                if (pkResult != pkS_OK)
                {
                    _socketError = eSocketErrorSSManifestInvalid;
                }
#ifdef _DUMP_MANIFEST_
                else
                {
                    _apChunkManifest->DumpManifest();
                }
#endif
            }

            // init drm
            if (pkResult == pkS_OK)
            {
                pkResult = InitDrm();
                if (pkResult != pkS_OK)
                {
                    _socketError = eSocketErrorSSDrmInitFailed;
                }
            }

            if (pkResult == pkS_OK)
            {
                CMediaStreamDescription *pSD = _apChunkManifest->GetPrimaryStreamDescription();
                if (pSD)
                {
                    pSD->SetLookAheadCount(0);

                    InitAudioLanguage();
                    InitChunkUpdateThread();

                    _streamTimeDelta = 0; //This will be ignored for VoD streams

                    if (_apChunkManifest->IsLive())
                    {
                        //Calculate 10MHz time delta for live streams
                        //
                        //Note that SS Live streams do not have any notion of absolute time
                        //so the following will be used to map the time of retrieval of SS Live
                        //manifest to calculate how far playback is behind the live edge.

                        uint64 lastStreamTime = GetCurrentStreamEndTime() - gSocketMbrConfiguration.SSLiveBackOffSeconds * MBR_DEFAULT_TIMESCALE;

                        _streamTimeDelta = (int64)(manifestRequestTime - lastStreamTime);

                        SOCKETMBRMANIFEST_TRACE(("CMbrManifest::Prepare: fetch time %lld ms; stream time offset: fetch %lld ms, stream %lld ms, delta %lld ms",
                            PTS_10MHZTO1KHZ(_referenceClock->GetTime() - manifestRequestTime),
                            PTS_10MHZTO1KHZ(manifestRequestTime),
                            PTS_10MHZTO1KHZ(lastStreamTime),
                            PTS_10MHZTO1KHZ(_streamTimeDelta)));
                    }
                }
            }
            else
            {
                _socketPKResult = pkResult;
                ret = false;
            }
        }

        CleanupManifestSocket();

        if (!ret)
        {
            // remove chunk manifest
            _apChunkManifest.Release();

            if (_pDrm)
            {
                _socketAVManager->GetDrmManager()->ReleaseDecrypter(_pDrm);
                _pDrm = NULL;
            }
        }
        else
        {
            _manifestReady = true;
            // Manifest is ready at this point. Call the manifest ready callback to the app
            if (NULL != _pManifestCallback)
            {
                _pManifestCallback->ManifestReadyCallback(_apChunkManifest, pkS_OK);
            }

            // Now manifest is parsed and merged
            // From this point on the application could change the manifest at anytime
            // We need to make sure all the access to the stream list/track list/chunk list
            // are thread safe.
            _apChunkManifest->SetManifestIsParsedAndMerged();
        }
    }
exit:
    if (!ret)
    {
        // if there was a socket failure, notify of the status
        if (eSocketErrorNone != _socketError)
        {
            string info;
            info = "state=detuned&tunererror=" + toString(eTunerErrorConnectFailed) +
                   "&socketerror="+toString(_socketError) +
                   "&pkresult="+toString(_socketPKResult) +
                   "&httpresponse="+toString(_socketHttpResponse);
            NotifyStatus(info);
        }
    }
    return ret;
}

pkRESULT CMbrManifest::RequestPlaybackRange( _In_ ISetPlaybackRangeCallback* pCallback, 
                                             _In_ TimeSpan_NTP minTime, 
                                             _In_ TimeSpan_NTP maxTime )
{
    // only live right edge is supported
    if(TimeSpan_NTP::FromTicks(TimeSpan_NTP::MAX_TICKS) != maxTime 
        || NULL == pCallback
        || minTime.Ticks() < 0)
    {
        return pkE_INVALIDARG;
    }

    AutoLock lock(&_setPlaybackRangeLock);

    // earliest that RequestPlaybackRange can be called is in manifest ready
    // another request is in progress
    if(!_manifestReady || !_apChunkManifest->IsSegmented() || _pPlaybackRangeCallback)
    {
        return pkE_INVALID_REQUEST;
    }

    _pPlaybackRangeCallback = pCallback;
    _requestedMinTime = TimeSpan_hns::ConvertFrom(minTime);

    return DefaultSegmentManifestFetcher::CreateInstance(this, &_pSegmentFetcher);
}

TimeSpan_hns CMbrManifest::SegmentDuration()
{
    return TimeSpan_hns::ConvertFrom(_apChunkManifest->SegmentDuration(), _apChunkManifest->TimeScale());
}

TimeSpan_hns CMbrManifest::GetRequestedMinTime()
{
    return _requestedMinTime;
}

void CMbrManifest::SetDVRMinTime(TimeSpan_hns pos)
{
    // turn calling of start/end time callbacks off until set playback range is complete
    _callStartEndTimeCallback = false;

    for (size_t i = 0; i < _apChunkManifest->m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pStreamInfo = _apChunkManifest->m_availableStreams[i];

        // specific position was not found, set to earliest time
        if (!pStreamInfo->SetDVRMinTime(pos))
        {
            pStreamInfo->SetDVRMinTime(TimeSpan_hns::FromTicks(TimeSpan_hns::MIN_TICKS));
        }
    }

    _apChunkManifest->UpdateManifestStartDuration();
}

bool CMbrManifest::IsDVRFull()
{
    for (size_t i = 0; i < _apChunkManifest->m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pStreamInfo = _apChunkManifest->m_availableStreams[i];
        if (TimeSpan_hns::FromTicks(0) == pStreamInfo->TimeRemainingInDVR())
        {
            return true;
        }
    }
    return false;
}

bool CMbrManifest::ParentStreamsContainsTime(_In_ TimeSpan_hns time)
{
    for (size_t i = 0; i < _apChunkManifest->m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pStreamInfo = _apChunkManifest->m_availableStreams[i];
        if (!pStreamInfo->HasParent() && !pStreamInfo->ChunkListContainsTime(time))
        {
            return false;
        }
    }
    return true;
}

TimeSpan_hns CMbrManifest::MaxOfParentStreamMinTime()
{
    int64 maxTime = -1;
    for (size_t i = 0; i < _apChunkManifest->m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pStreamInfo = _apChunkManifest->m_availableStreams[i];
        if (!pStreamInfo->HasParent() && pStreamInfo->GetChunkStartPosition(pStreamInfo->GetDVRMinChunkIndex()) > maxTime)
        {
            maxTime = pStreamInfo->GetChunkStartPosition(pStreamInfo->GetDVRMinChunkIndex());
        }
    }
    return TimeSpan_hns::FromTicks(maxTime);
}

void CMbrManifest::CancelPrepare(void)
{
    // cleanup the socket so it'll stop doing what it's doing.
    CleanupManifestSocket();
}

void CMbrManifest::GetPrepareResult(TunePrepareResult* pResult)
{
    if (!pResult)
        return;

    if (NULL != _apChunkManifest)
    {
        pResult->Bitrate = _apChunkManifest->m_nTotalBps;

        CMediaStreamDescription *pMSD = _apChunkManifest->GetStreamDescriptionByType(MediaStreamTypeVideo);
        if (pMSD)
        {
            CMediaStreamDescription::Range rg = pMSD->GetBitrateRangeOfSelectedTracks();

            pResult->MinVideoBitrate = rg._min;
            pResult->MaxVideoBitrate = rg._max;
        }
        else
        {
            pResult->MinVideoBitrate = 0;
            pResult->MaxVideoBitrate = 0;
        }

        pResult->IsLive = _apChunkManifest->IsLive();
        pResult->SessionId = _sessionId.empty() ? GUID_NULL : GuidFromString(_sessionId.c_str());

        pResult->ResourceId = GUID_NULL;
        if (_pDrm)
        {
            size_t len = sizeof(pResult->ResourceId);
            _pDrm->GetProperty(DRM_PROPERTY_RESOURCEID, (byte*) &(pResult->ResourceId), &len);
        }

        CMediaStreamDescription *pSD = _apChunkManifest->GetPrimaryStreamDescription();
        if (pSD)
        {
            pResult->StartTime.Set( TimeSpan_hns::FromTicks( pSD->GetChunkStartPosition( pSD->GetDVRMinChunkIndex() ) ) );
            pResult->EndTime.Set( TimeSpan_hns::FromTicks( pSD->GetChunkEndPosition( pSD->GetDVRMaxChunkIndex() ) ) );
        }
    }
    else
    {
        memset(pResult, 0, sizeof(TunePrepareResult));
    }
}

void CMbrManifest::OPLCallback(XDRM_OPL_DATA *pOPLData)
{
    _drmOPLData = *pOPLData;
}

void CMbrManifest::StoreExtraTuneParameters()
{
    // send the client(device) id - needed by the server for reporting purposes
    _deviceId = gAVEngineConfiguration.ClientID;

    // send the session id in case server is failing over because
    //    the server needs the session id of last server session
    _sessionId = _tuneRequest.GetArg(TUNE_REQUEST_SESSIONID);
}

void CMbrManifest::StoreSessionId(IStreamerHttp* pStreamerHttp)
{
    std::string sessionId;

    bool isHeaderAvailable;

    if (NULL != pStreamerHttp)
    {
        isHeaderAvailable = pStreamerHttp->GetResponseSessionIdHeader( &sessionId );
    }
    else
    {
        isHeaderAvailable = _manifestSocket.GetSessionIdHeader(sessionId);
    }

    if (isHeaderAvailable)
    {
        AutoLock lock(&_sessionIdLock);

        if (_sessionId != sessionId)
        {
            _sessionId = sessionId;
        }
    }
}

uint64 CMbrManifest::GetCurrentStreamEndTime(void)
{
    uint64 streamTimeHNS = 0;

    CMediaStreamDescription *pSD = _apChunkManifest->GetPrimaryStreamDescription();
    if (NULL != pSD)
    {
        streamTimeHNS = (uint64)pSD->GetChunkEndPosition(pSD->GetDVRMaxChunkIndex());
    }
    return streamTimeHNS;
}

void CMbrManifest::CheckAtWindowEdge(uint32 streamID)
{
    // get the current playback time
    ITunerSession* pITunerSession = (_socketAVManager->GetTunerSessionFactory()->GetTunerSession(_socketPipeIdN));

    if ( NULL == pITunerSession )
    {
        return;
    }

    TimeSpan_hns currentMediaTime = TimeSpan_hns::ConvertFrom( TimeSpan_NTP::FromTicks(pITunerSession->GetCurrentMediaTime(false)));

    CMediaStreamDescription *pSD = _apChunkManifest->GetStreamDescriptionById(streamID);
    if (NULL != pSD)
    {
        TimeSpan_hns startEdgeTime = TimeSpan_hns::FromTicks(pSD->GetChunkStartPosition(pSD->GetDVRMinChunkIndex()));
        TimeSpan_hns startBuffer = TimeSpan_hns::ConvertFrom( TimeSpan_s::FromTicks(gSocketMbrConfiguration.SSLeftEdgeBufferSeconds) );
        if (currentMediaTime < startEdgeTime + startBuffer)
        {
            SOCKETMBRMANIFEST_TRACE(("At WindowEdge %lld, SID[%d] type %d, currentTime %lld",
                (startEdgeTime + startBuffer).Ticks(), streamID, pSD->Type(), currentMediaTime.Ticks() ));

            string info = "status=atwindowedge";

            NotifyStatus(info);
        }
    }
}

void CMbrManifest::InitChunkUpdateThread()
{
    AutoLock lock(&_chunkUpdateLock);

    if (!_apChunkManifest->IsLive())
    {
        SOCKETMBRMANIFEST_TRACE(("CMbrManifest::InitChunkUpdateThread() - not starting thread because not Live content!"));
        return;
    }

    // recv timeout is a fraction of the number of streams to get through within CHUNK_UPDATE_STREAMS_SECS.
    // Timeout causes retry logic to be applied
    _recvTimeout = (CHUNK_UPDATE_STREAMS_SECS * c_SEC_TO_MS)/_apChunkManifest->m_availableStreams.size();

    // make sure the timeout is not below limit
    if( _recvTimeout < c_CHUNK_INFO_TIMEOUT_MS_LIMIT )
    {
        _recvTimeout = c_CHUNK_INFO_TIMEOUT_MS_LIMIT;
    }
    
    if (_chunkUpdateThread)
    {
        SOCKETMBRMANIFEST_TRACE(("CMbrManifest::InitChunkUpdateThread() - thread already exists!"));
        return;
    }

    _chunkUpdateThread = NEW_NO_THROW Thread;
    if (!_chunkUpdateThread)
    {
        // allocation failure
        ASSERT(false);
    }
    else
    {
        SOCKETMBRMANIFEST_TRACE(("CMbrManifest::InitChunkUpdateThread() - starting thread..."));
        _chunkUpdateThreadState = eUpdateThreadState_Enabled;
        _chunkUpdateThread->Start(this);
    }
}

void CMbrManifest::EnableChunkUpdateThread(bool bEnable)
{
    if ((bEnable && (_chunkUpdateThreadState == eUpdateThreadState_Enabled))
        || (!bEnable && (_chunkUpdateThreadState == eUpdateThreadState_Disabled)))
    {
        SOCKETMBRMANIFEST_TRACE(("CMbrManifest::EnableChunkUpdateThread() - update thread already %s",
            bEnable ? "enabled" : "disabled"));
    }
    else
    {
        SOCKETMBRMANIFEST_TRACE(("CMbrManifest::EnableChunkUpdateThread() - update thread now %s",
            bEnable ? "ENABLED" : "DISABLED"));

        _chunkUpdateThreadState = bEnable ? eUpdateThreadState_Enabled : eUpdateThreadState_Disabled;
        _chunkUpdateThreadWakeEvent.Set();
    }
}

void CMbrManifest::StopChunkUpdateThread()
{
    SOCKETMBRMANIFEST_TRACE(("CMbrManifest::StopChunkUpdateThread()"));

    _chunkUpdateThreadState = eUpdateThreadState_Stopped;
    _chunkUpdateThreadWakeEvent.Set();
}

void CMbrManifest::OnThreadRun()
{
    // Assumes number of streams can't change once manifest has been parsed
    uint32 numStreams = _apChunkManifest->m_availableStreams.size();
    CSocketMbrManifestRetry* retryInfo = NEW_NO_THROW CSocketMbrManifestRetry[numStreams];
    uint32 updatePollPeriodMs = CHUNK_UPDATE_POLL_PERIOD_DEFAULT_MS;

    SOCKETMBRMANIFEST_TRACE(("CMbrManifest::OnThreadRun() - thread ENTRY"));

    while (!_chunkUpdateThreadExitFlag)
    {
        _chunkUpdateThreadWakeEvent.Wait(updatePollPeriodMs);

        if (_chunkUpdateThreadExitFlag)
        {
            break;
        }

        if (_chunkUpdateThreadState == eUpdateThreadState_Stopped)
        {
            updatePollPeriodMs = CHUNK_UPDATE_POLL_PERIOD_STOPPED_MS;
            SOCKETMBRMANIFEST_TRACE(("CMbrManifest::OnThreadRun stopped"));
            continue;
        }
        else
        {
            updatePollPeriodMs = CHUNK_UPDATE_POLL_PERIOD_DEFAULT_MS;
        }

        uint64 currentTime = _referenceClock->GetTime() - _streamTimeDelta;

        bool isNearLive = false;

        // Is socket thread playing near live?
        if (_chunkUpdateThreadState == eUpdateThreadState_Disabled)
        {
            uint64 streamTime = GetCurrentStreamEndTime();
            int64 delta = (int64)(currentTime - streamTime);
            isNearLive = delta < ((int64) CHUNK_UPDATE_THRESHOLD_SECS * MBR_DEFAULT_TIMESCALE);

            SOCKETMBRMANIFEST_TRACE(("CMbrManifest::OnThreadRun %s, s %lld ms, c %lld ms, delta %lld ms",
                isNearLive ? "continues disabled" : "RE-ENABLED",
                PTS_10MHZTO1KHZ(streamTime),
                PTS_10MHZTO1KHZ(currentTime),
                PTS_10MHZTO1KHZ(delta)));

            if (!isNearLive)
            {
                _chunkUpdateThreadState = eUpdateThreadState_Enabled;
            }
        }
        else
        {
            SOCKETMBRMANIFEST_TRACE(("CMbrManifest::OnThreadRun continues enabled, currentTime %lld ms",
                PTS_10MHZTO1KHZ(currentTime)));
        }

        // check if this is a retry
        bool onRetry = false;
        if ((currentTime - _lastTimeStreamsUpdated) < ((int64)CHUNK_UPDATE_STREAMS_SECS * MBR_DEFAULT_TIMESCALE))
        {
            SOCKETMBRMANIFEST_TRACE(("CMbrManifest::OnThreadRun not enough time has passed skipping streamsUpdate, currentTime %lld ms, lastTime %lld ms ",
                PTS_10MHZTO1KHZ(currentTime),
                PTS_10MHZTO1KHZ(_lastTimeStreamsUpdated)));
            onRetry = true;
        }
        else
        {
            SOCKETMBRMANIFEST_TRACE(("CMbrManifest::OnThreadRun enough time has passed streamsUpdate, currentTime %lld ms, lastTime %lld ms ",
                    PTS_10MHZTO1KHZ(currentTime),
                    PTS_10MHZTO1KHZ(_lastTimeStreamsUpdated)));

            // update the time the streams have been updated
            _lastTimeStreamsUpdated = currentTime;

        }

        // Iterate through streams
        for (uint32 i = 0; i < numStreams; i++)
        {
            if ( onRetry && !retryInfo[i].doDelayRetry )
            {
                continue;
            }

            retryInfo[i].doDelayRetry = false;

            CMediaStreamDescription* pStreamInfo = _apChunkManifest->m_availableStreams[i];

            MediaStreamType streamType = pStreamInfo->Type();

            if( pStreamInfo->HasParent() )
            {
                // Parented streams don't request fragment info directly.
                // The information comes embedded in their parents' requests.
                continue;
            }

            if( ( MediaStreamTypeAudio == streamType )
                || ( MediaStreamTypeVideo == streamType ) )
            {
                if( isNearLive && pStreamInfo->IsSelected() )
                {
                    // No need to request the fragment info for the active streams near the
                    // live  edge as the information is already part of the stream's
                    // full fragment request.
                    continue;
                }
            }
            else if( MediaStreamTypeText == streamType )
            {
                // Until the implementation of IManifest::SelectStreamsAsync triggers a manifest
                // refresh, always request the fragment info for all available text streams
                // to keep the chunk list up to date.
            }
            else
            {
                // Do not process streams other than audio, video, or subtitles text:
                continue;
            }

            if ( !ReadNextChunkHeader(i, currentTime, &retryInfo[i]) )
            {
                break; // The stopped state has been set so exit the loop
            }
        }

        ReportStartEndTime();
    }

    delete [] retryInfo;
    SOCKETMBRMANIFEST_TRACE(("CMbrManifest::OnThreadRun() - thread EXIT"));
}

bool CMbrManifest::RetryChunkInfo(  _Inout_ CSocketMbrManifestRetry* retryInfo,
                                    _Inout_ int32* chunkIndex,
                                    _Inout_ uint32* trackIndex,
                                    _In_ uint32 streamIndex)
{
    CMediaStreamDescription* pMSD = _apChunkManifest->m_availableStreams[streamIndex];
    CSocketMbrRetry::RetryAction retryAction = retryInfo->retryState.GetRetryAction( _chunkInfoReader.GetSocketError(), _chunkInfoReader.GetHttpResponse() );
    if( CSocketMbrRetry::NO_RETRY == retryAction )
    {
        return false;
    }

    if( CSocketMbrRetry::RETRY_SAME_IDX_SAME_BR == retryAction )
    {
        // schedule to retry later
        retryInfo->doDelayRetry = true;

        SOCKETMBRMANIFEST_TRACE(("SID %d: retry same chunk:%d and QL: %d, first chunk idx:%d, last chunk idx:%d",
                                    pMSD->GetStreamID(),
                                    *chunkIndex,
                                    *trackIndex,
                                    pMSD->GetDVRMinChunkIndex(),
                                    pMSD->GetDVRMaxChunkIndex()));
        return false;
    } 
    else if ( CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR == retryAction )
    {
        if ( ++(*trackIndex) < pMSD->NumberOfAvailableTracks() )
        {
            SOCKETMBRMANIFEST_TRACE(("SID %d: retry higher quality level:%d for chunk %d",
                pMSD->GetStreamID(),
                *trackIndex,
                *chunkIndex));
        }
        else
        {
            // no bitrates available, move to next chunk
            retryAction = CSocketMbrRetry::RETRY_NEXT_IDX;
            retryInfo->retryState.UpdateCounters( _chunkInfoReader.GetSocketError() );
        }
    }

    if ( CSocketMbrRetry::RETRY_NEXT_IDX == retryAction )
    {
        // check to see if there is a next chunk available
        if ( *chunkIndex < pMSD->GetDVRMaxChunkIndex() )
        {
            //Move on to the next chunk
            (*chunkIndex)++;

            // retry with the lowest bitrate
            *trackIndex = 0;

            SOCKETMBRMANIFEST_TRACE(("SID %d: retry(404s:%d,412s:%d,total:%d) next chunk:%d, first chunk idx:%d, last chunk idx:%d",
                                     pMSD->GetStreamID(),
                                     *chunkIndex,
                                     pMSD->GetDVRMinChunkIndex(),
                                     pMSD->GetDVRMaxChunkIndex()));
        }
        else
        {
            return false;
        }
    }
    return true;
}

void CMbrManifest::ReportTrackInfoFailure(const _In_ wstring& url)
{
    string info;

    // GetLastError only returns http invalid results
    if (_chunkInfoReader.GetHttpResponse())
    {
        info = "status=chunkhdrhttpinvalid&httpresponse=" + toString(_chunkInfoReader.GetHttpResponse());
               
    }
    // A non-http invalid failure occurred, such as a socket error or parsing
    else
    {
        info = "status=chunkhdrerror";
    }
    info += "&pkresult=" + toString(_chunkInfoReader.GetPKResult()) +
            "&url=" + WStr2Str(url);

    NotifyStatus(info);
}

bool CMbrManifest::CanRequestFragment(_In_ CMediaStreamDescription* pMSD, _In_ int32 chunkIdx, _Out_ TimeSpan_hns* timeDelta)
{
    uint64 liveTime = _referenceClock->GetTime() - _streamTimeDelta;
    uint64 chunkTime = (uint64)pMSD->GetChunkEndPosition(chunkIdx);

    timeDelta->Set(TimeSpan_hns::FromTicks(liveTime - chunkTime));

    // allow request if the chunk time is less than the live server
    return (chunkTime < liveTime);
}

bool CMbrManifest::ReadNextChunkHeader(_In_ uint32 streamIndex, _In_ uint64 currentTime, _Inout_ CSocketMbrManifestRetry* retryInfo)
{
    wstring url;
    uint32 cRequests = 0;
    FMP4TrackInfo* pTrackInfo = NULL;
    CMediaStreamDescription* pMSD = _apChunkManifest->m_availableStreams[streamIndex];
    uint32 dwStreamID = pMSD->GetStreamID();
    bool bIsPrimaryStream = (dwStreamID == _apChunkManifest->GetPrimaryStreamId());

    SOCKETMBRMANIFEST_TRACE(("CMbrManifest::ReadNextChunkHeader() - SID %d %s: s %lld ms, c %lld ms",
        dwStreamID, bIsPrimaryStream ? "primary" : "secondary",
        PTS_10MHZTO1KHZ(pMSD->GetChunkEndPosition(pMSD->GetDVRMaxChunkIndex())),
        PTS_10MHZTO1KHZ(currentTime)));

    // always start with the lowest bitrate
    uint32 trackIndex = 0;
    int32 lastChunkIndex = pMSD->GetDVRMaxChunkIndex();
    uint64 streamTime = (uint64)pMSD->GetChunkEndPosition(lastChunkIndex);

    // Loop until trying to get the chunkinfo or we hit the expected current end time
    while(cRequests < CHUNK_UPDATE_MAX_REQUESTS)
    {
        TimeSpan_hns timeDelta;
        if (!CanRequestFragment(pMSD, lastChunkIndex, &timeDelta))
        {
            SOCKETMBRMANIFEST_TRACE(("SID %d (%d): Stream time > Current time by %lld ms; exiting ReadNextChunkHeader",
                dwStreamID, cRequests,
                TimeSpan_ms::ConvertFrom(timeDelta).Ticks()));

            break;
        }

        // Get url for the chunkInfo
        if (pkS_OK != _apChunkManifest->GetChunkURL(dwStreamID, lastChunkIndex, trackIndex, url, false, true))
            break;

        SOCKETMBRMANIFEST_TRACE(("SID %d (%d): index %d, s %lld ms, c %lld ms, lag %lld ms\n URL: %ls",
            dwStreamID, cRequests, lastChunkIndex,
            PTS_10MHZTO1KHZ(streamTime),
            PTS_10MHZTO1KHZ(currentTime),
            PTS_10MHZTO1KHZ(currentTime - streamTime),
            url.c_str()));

        ReportEvent_StartFragInfoRequest( dwStreamID, 
                                          lastChunkIndex, 
                                          trackIndex, 
                                          pMSD->TrackWeakPtr( trackIndex )->Bitrate(), 
                                          url.c_str() );

        pTrackInfo = _chunkInfoReader.GetChunkInfo(url, _recvTimeout);
        if (!pTrackInfo)
        {
            ReportTrackInfoFailure(url);

            if ( RetryChunkInfo( retryInfo, &lastChunkIndex, &trackIndex, streamIndex) )
            {
                continue;
            }
            else
            {
                // If we fail to read the chunk a few times, query server for latest available chunk and try to resume from there
                if ( _chunkRequestLatestEnabled
                     && !retryInfo->doDelayRetry
                     && (pkS_OK == _apChunkManifest->GetLatestChunkInfoURL(dwStreamID, 0, url)) )
                {
                    pTrackInfo = _chunkInfoReader.GetChunkInfo(url, _recvTimeout);
                    if (pTrackInfo)
                    {
                        if (pTrackInfo->_live_frag_absolute_time > streamTime)
                        {
                            // We have a more recent chunk available
                            SOCKETMBRMANIFEST_TRACE(("ReadNextChunkHeader: SID %d jumping to time %lld (%lld)",
                                dwStreamID, pTrackInfo->_live_frag_absolute_time, pTrackInfo->_live_frag_duration));
                        }
                    }
                    else if (_chunkInfoReader.GetHttpResponse() == HTTP_STATUS_BAD_REQUEST)
                    {
                        // Disable check if ability to request latest chunk via FragmentInfo(video=-1) is not supported
                        _chunkRequestLatestEnabled = false;
                        TRACE_ERROR(("ReadNextChunkHeader: request latest chunk not supported - disabling..."));
                    }

                    //check if retry failed and send status
                    if (!pTrackInfo)
                    {
                        ReportTrackInfoFailure(url);
                    }
                }
                break;
            }
        }

        // Getting chunkInfo was successful
        if (pTrackInfo)
        {
            AddChunk(dwStreamID, pTrackInfo);

            // after adding a chunk, check if the current
            // playback position is near the start of the window edge
            CheckAtWindowEdge(dwStreamID);

            // refresh the chunkInfo and stream time
            lastChunkIndex = pMSD->GetDVRMaxChunkIndex();
            streamTime = (uint64)pMSD->GetChunkEndPosition(lastChunkIndex);
            trackIndex = 0;

            // starting another request so need to reset
            retryInfo->retryState.Reset();
            cRequests++;

            // Parse the Content-Type header for information about the
            // stream's sparse children
            if( pMSD->HasChildStreams() )
            {
                bool fFound = false;
                std::string strHeaderValue;

                for( size_t i = 0; i < NUM_OF_HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO; ++i )
                {
                    if( _chunkInfoReader.GetResponseHeader( HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO[i], &strHeaderValue ) )

                    {
                        CSparseStreamChunkInfoHeaderParser parser( strHeaderValue.c_str() );
                        while( parser.MoveNext() )
                        {
                            pMSD->AddSparseChildChunkInfo( parser.CurrentStreamName(), parser.CurrentChunkTime() );
                            fFound = true;
                        }

                        if( fFound )
                        {
                            break;
                        }
                    }
                }
            }

            // Stop thread if chunk should have lookahead chunks but doesn't
            if ( bIsPrimaryStream && (pTrackInfo->_live_frags_ahead == 0) && (_apChunkManifest->LookAheadCount() > 0) )
            {
                SOCKETMBRMANIFEST_TRACE(("ReadNextChunkHeader: stopping thread - no look ahead fragments"));
                StopChunkUpdateThread();
                return false;
            }
        }
    }

    return true;
}

void CMbrManifest::OnThreadStop()
{
    SOCKETMBRMANIFEST_TRACE(("ChunkUpdateThread: signalling shutdown..."));
    _chunkUpdateThreadExitFlag = true;
    _chunkUpdateThreadWakeEvent.Set();
}

void CMbrManifest::AddChunk(uint32 dwStreamId, FMP4TrackInfo* pTrackInfo)
{
    CMediaStreamDescription* pMSD = _apChunkManifest->GetStreamDescriptionById(dwStreamId);
    if (!pMSD || !pTrackInfo)
        return;

    //Make sure not to lock when loading chunk headers - only when adding to chunk buffer
    AutoLock lock(&_chunkUpdateLock);

    uint64 chunkAbsoluteTime = pTrackInfo->_live_frag_absolute_time;
    uint64 lastChunkDuration = pTrackInfo->_live_frag_duration;
    // NOTE: In the case of no look-ahead, lastChunkDuration is an estimate based on the current fragment duration

    int32 lookAheadCount = pTrackInfo->_live_frags_ahead;

    //Use look ahead table, if available
    if (lookAheadCount > 0)
    {
        //Quick sanity check to make sure that timestamps are consistent
        if ((chunkAbsoluteTime + lastChunkDuration) != pTrackInfo->_fragment_absolute_time_table[0])
        {
            //ASSERT(false);
            TRACE_ERROR(("CMbrManifest::AddChunk() SID %d: ix %u t %lld + d %lld != t[0] %lld",
                dwStreamId,
                pMSD->GetDVRMaxChunkIndex(),
                chunkAbsoluteTime,
                lastChunkDuration,
                pTrackInfo->_fragment_absolute_time_table[0]));
        }

        for (int32 ix = 0; ix < lookAheadCount; ix++)
        {
            CChunkInfo* pChunkInfo = pMSD->AddChunk(pTrackInfo->_fragment_absolute_time_table[ix]);
            if (NULL == pChunkInfo)
            {
                SOCKETMBRMANIFEST_TRACE_VERBOSE(("SID %d: lookahead ix %u t %lldms d %lldms IN TABLE",
                    dwStreamId,
                    ix,
                    PTS_10MHZTO1KHZ(pTrackInfo->_fragment_absolute_time_table[ix]),
                    PTS_10MHZTO1KHZ(lastChunkDuration)));
            }
            else
            {
                lastChunkDuration = pTrackInfo->_fragment_duration_table[ix];

                ReportEvent_AddChunk( dwStreamId, pTrackInfo->_fragment_absolute_time_table[ix]);

                SOCKETMBRMANIFEST_TRACE(("SID %d: lookahead ix %u t %lldms d %lldms added",
                    dwStreamId,
                    ix,
                    PTS_10MHZTO1KHZ(pTrackInfo->_fragment_absolute_time_table[ix]),
                    PTS_10MHZTO1KHZ(lastChunkDuration)));
            }
        }

        pMSD->SetLookAheadCount(lookAheadCount - 1); // allow access to next chunk so the list can advance
    }
    else //no lookahead so just use current chunk info
    {
        CChunkInfo* pChunkInfo = pMSD->AddChunk(chunkAbsoluteTime);
        if (NULL == pChunkInfo)
        {
            SOCKETMBRMANIFEST_TRACE_VERBOSE(("SID %d: live t %lld d %lld IGNORED",
                dwStreamId, chunkAbsoluteTime, lastChunkDuration));
        }
        else
        {
            ReportEvent_AddChunk( dwStreamId, chunkAbsoluteTime);

            SOCKETMBRMANIFEST_TRACE(("SID %d: live t %lld d %lld added",
                dwStreamId, chunkAbsoluteTime, lastChunkDuration));
        }

        pMSD->SetLookAheadCount(0); // allow access to all chunks in the list
                    
        if(pMSD == _apChunkManifest->GetPrimaryStreamDescription())
        {
            _endOfLive = true;
        }
    }

    pMSD->SetLastChunkDuration((uint32)lastChunkDuration);
}

void CMbrManifest::ReportStartEndTime(bool forceReport /* = false */)
{
    CMediaStreamDescription* pMSD = _apChunkManifest->GetPrimaryStreamDescription();

    if (!pMSD || !_apChunkManifest->IsLive())
    {
        return;
    }

    AutoLock lock(&_chunkUpdateLock);
    int64 hnsEndTime = pMSD->GetChunkEndPosition(pMSD->GetDVRMaxChunkIndex());
    
    if (_lastReportedChunkEndTime != hnsEndTime || forceReport)
    {
        _lastReportedChunkEndTime = hnsEndTime;

        int64 hnsStartTime = pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex());

        string event_str;
        event_str = "status=startendtime&starttime=";
        event_str += uint64toString64(NTP_10MHZTOUINT64(hnsStartTime));
        event_str += "&endtime=";
        event_str += uint64toString64(NTP_10MHZTOUINT64(hnsEndTime));

#ifdef SOCKETMBRMANIFEST_SPEW_VERBOSE
        int firstindex = pMSD->GetDVRMinChunkIndex();
        int lastindex = pMSD->GetDVRMaxChunkIndex();
        uint64 firsthns = pMSD->GetChunkStartPosition(firstindex);
        uint64 lasthns = pMSD->GetChunkEndPosition(lastindex);
        uint64 firstntp = NTP_10MHZTOUINT64(firsthns);
        uint64 lastntp = NTP_10MHZTOUINT64(lasthns);
        SOCKETMBRMANIFEST_TRACE_VERBOSE(("startendtime event: index(%d, %d), hns(%llu, %llu, %llu), ntp(%llu, %llu, %llu)",
            firstindex, lastindex, firsthns, lasthns, lasthns-firsthns, firstntp, lastntp, lastntp-firstntp));
#endif

        NotifyStatus(event_str, _callStartEndTimeCallback);
    }

    // report if the live presentation is no longer live
    if ((_apChunkManifest->LookAheadCount() > 0) && (_endOfLive) && (!_endOfLiveReported))
    {
        _endOfLiveReported = true;
        NotifyStatus("status=endoflive");
    }  
}

void CMbrManifest::ReportStatus(const string& info)
{
    NotifyStatus(info);
}

void CMbrManifest::PlaybackRangeComplete(pkRESULT result)
{
    ISetPlaybackRangeCallback* pPlaybackRangeCallback = NULL;
    
    {
        AutoLock lock(&_setPlaybackRangeLock);
        pPlaybackRangeCallback = _pPlaybackRangeCallback;
        _pPlaybackRangeCallback = NULL;
    }

    // update with new start/end time
    ReportStartEndTime(true);

    if(pPlaybackRangeCallback)
    {
        pPlaybackRangeCallback->SetPlaybackRangeCallback(_apChunkManifest, result);
    }

    // turn on start/end time callbacks
    _callStartEndTimeCallback = true;
}

void CMbrManifest::NotifyStatus(_In_ const std::string& info, _In_opt_ bool shouldCallCallback /* = true */)
{
    //get the callback
    ITunerSessionCallback* tunerCallback = NULL;
    ITunerSession* tunerSession = _socketAVManager->GetTunerSessionFactory()->GetTunerSession(_socketPipeIdN);
    if (tunerSession)
    {
        tunerCallback = tunerSession->GetStatusCallback();
    }

    //call the callback
    if(tunerCallback)
    {
        tunerCallback->StatusCallback(info, shouldCallCallback);
    }
}

// ===============================================================================================================
// ===============================================================================================================

CChunkInfoReader::CChunkInfoReader(IAVManager* avManager, uint32 pipeIdN)
    : _socketAVManager(avManager)
    , _socketPipeIdN(pipeIdN)
    , _chunkSocket(NULL)
    , _chunkBuffer(NULL)
    , _chunkBlockSize(0)
{
}

CChunkInfoReader::~CChunkInfoReader()
{
    //Close and dump the socket
    if (_chunkSocket != NULL)
    {
        _chunkSocket->Close();
        delete _chunkSocket;
    }
    if (_chunkBuffer)
    {
        delete [] _chunkBuffer;
    }
}

bool CChunkInfoReader::Connect(CTuneRequest& tuneRequest)
{
    string response;
    string extraHeader;

    if (_chunkSocket == NULL)
    {
        _chunkBlockSize = c_DEFAULT_CHUNK_INFO_BLOCK_SIZE;
        _chunkBuffer = NEW_NO_THROW byte[_chunkBlockSize];
        if (!_chunkBuffer)
        {
            ASSERT(false);
            return false;
        }

        _chunkSocket = IStreamerHttp::CreateStreamerHttp();
        if (!_chunkSocket)
        {
            ASSERT(false);
            return false;
        }

        return static_cast<IStreamer*>(_chunkSocket)->Connect( &tuneRequest.TunerUrl );
    }

    return _chunkSocket->HttpRequestResponse(tuneRequest, extraHeader, &response);
}

FMP4TrackInfo* CChunkInfoReader::GetChunkInfo(_In_ wstring& wstrChunkURL, _In_ int timeout)
{
    CTuneRequest tuneRequest;
    tuneRequest.ParseUrl(wstrChunkURL);

    if (!Connect(tuneRequest))
        return NULL;

    uint32 bytesRead = 0;
    bool endOfResponse = false;
    while (!endOfResponse)
    {
        while (!endOfResponse && bytesRead < _chunkBlockSize)
        {
            int rc = _chunkSocket->Recv(_chunkBuffer + bytesRead, _chunkBlockSize - bytesRead, &endOfResponse, timeout);
            if (rc <= 0)
            {
                return NULL;
            }
            bytesRead += rc;
        }

        // need to allocate more space
        if (!endOfResponse)
        {
            byte* oldChunkBuffer = _chunkBuffer;
            uint32 oldChunkBlockSize = _chunkBlockSize;

            // double the buffer size
            _chunkBlockSize = _chunkBlockSize * 2;

            if ( _chunkBlockSize > c_MAX_CHUNK_INFO_BLOCK_SIZE )
            {
                _chunkBlockSize = c_MAX_CHUNK_INFO_BLOCK_SIZE;
                TRACE_ERROR(("GetChunkInfo: _chunkBlockSize is too large (%d)", c_MAX_CHUNK_INFO_BLOCK_SIZE));
                return NULL;
            }

            _chunkBuffer = NEW_NO_THROW byte[_chunkBlockSize];
            if (!_chunkBuffer)
                return NULL;

            if (oldChunkBuffer)
            {
                memcpy_s(_chunkBuffer, _chunkBlockSize, oldChunkBuffer, oldChunkBlockSize);
                delete [] oldChunkBuffer;
            }
        }
    }

    _fmp4Parser.Init(MP4Feed::MP4Feed_Memory, _chunkBuffer, bytesRead);
    if (!_fmp4Parser.Parse())
        return NULL;

    // return the track info object
    FMP4Info* fmp4Info = (FMP4Info*)_fmp4Parser.GetInfo();
    return fmp4Info ? (FMP4TrackInfo*)fmp4Info->GetTrackInfo() : NULL;
}

bool CChunkInfoReader::GetResponseHeader( _In_ const std::string& header, _Out_ std::string* pstrHeader )
{
    return( _chunkSocket->GetResponseHeader( header.c_str(), pstrHeader ) );
}

// ===============================================================================================================
// ===============================================================================================================


CSparseStreamChunkInfoHeaderParser::CSparseStreamChunkInfoHeaderParser( _In_ const char* pszHeader )
    : m_pszPos( NULL )
{
    //
    // Seek to the "ChildTrack=" field in the header
    //

    static const char CHILD_TRACK_TOKEN[] = "ChildTrack=\"";
    static const size_t CHILD_TRACK_TOKEN_LEN = sizeof(CHILD_TRACK_TOKEN)/sizeof(CHILD_TRACK_TOKEN[0]) - 1;

    while( *pszHeader != '\0' )
    {
        //
        // Check if the "ChildTrack" token is at the current position
        //

        pszHeader += strspn( pszHeader, " \t" );    // skip spaces

        if( strncmp( pszHeader, CHILD_TRACK_TOKEN, CHILD_TRACK_TOKEN_LEN ) == 0 )
        {
            m_pszPos = pszHeader + CHILD_TRACK_TOKEN_LEN;
            break;
        }

        //
        // Otherwise, skip to the next header field or quoted-string
        //

        pszHeader += strcspn( pszHeader, ",;\"\\" );

        if( *pszHeader == '\"' )
        {
            // Found the beginning of a quoted-string
            // (see http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2).
            // Search for its end.

            ++pszHeader;

            while( *pszHeader != '\0' )
            {
                pszHeader += strcspn( pszHeader, "\"\\" );
                if( *pszHeader != '\\' )
                {
                    break;
                }

                // Found a escaped char inside the string, skip it.

                ++pszHeader;    // skip the '\' (escaped char prefix)

                if( *pszHeader == '\0' )
                {
                    break;  // unexpected end of string after escape char prefix
                }

                ++pszHeader;    // skip the escaped char
            }

            if( *pszHeader == '\"' )
            {
                // Found the end of the quoted-string.
                ++pszHeader;
            }
        }
        else if(( *pszHeader == ';' ) || ( *pszHeader == ',' ))
        {
            ++pszHeader;
        }
        else if( *pszHeader == '\\' )
        {
            // Unexpected quoted-pair outside quoted-string
            break;
        }
    }
}

bool CSparseStreamChunkInfoHeaderParser::MoveNext()
{
    bool fFound = false;
    const char* pszNext = NULL;
    int64 chunkTime = 0;

    if( ( m_pszPos == NULL )
        || ( *m_pszPos == '\0' ) )
    {
        goto exit;
    }

    //
    // Parse the stream name
    //

    pszNext = m_pszPos + strcspn( m_pszPos, "=\"" );

    if( ( pszNext == m_pszPos )
        || ( *pszNext != '=' ) )
    {
        // No more stream names found.
        goto exit;
    }

    m_strCurrentStreamName.assign( m_pszPos, pszNext );

    //
    // Skip the '='
    //

    ++pszNext;

    //
    // Parse the chunk time
    //

    m_pszPos = pszNext;

    while( '0' <= *pszNext && *pszNext <= '9' )
    {
        int64 acc = ( chunkTime * 10 ) + ( *pszNext - '0' );

        if( acc < chunkTime )
        {
            // Overflow. Ignore this field.
            TRACE_ERROR(("@%p: Integer overflow parsing %s", this, m_pszPos));
            goto exit;
        }

        chunkTime = acc;
        ++pszNext;
    }

    if( m_pszPos == pszNext )
    {
        // Malformed header.
        // No value found.
        goto exit;
    }

    m_currentChunkTime = chunkTime;

    //
    // Skip the separator for the next item
    //

    if( *pszNext == ';' )
    {
        ++pszNext;
    }

    m_pszPos = pszNext;

    fFound = true;

exit:

    return( fFound );
}

