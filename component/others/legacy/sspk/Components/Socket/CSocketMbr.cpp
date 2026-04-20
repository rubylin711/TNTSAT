///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Base64.h"
#include "CSocketMbr.h"
#include "CSocketMbrDiags.h"
#include "CAVEngineConfiguration.h"
#include "CSocketMbrConfiguration.h"
#include "CTrickConfiguration.h"
#include "ITunePrepare.h"
#include "StringUtils.h"
#include "Trace.h"
#include "SocketDiagEvents.h"
#include "IXDrm.h"
#include "StreamTypeTraits.h"
#include <string>
#include <map>

#include "IPTVDecoderHal.h"

using namespace std;
using namespace MBR;

//#define SOCKETMBR_SPEW
#ifdef SOCKETMBR_SPEW
#define SOCKETMBR_TRACE(x) TRACE(x)
#else
#define SOCKETMBR_TRACE(x)
#endif

//#define SOCKETMBR_SPEW_EXTRA
#ifdef SOCKETMBR_SPEW_EXTRA
#define SOCKETMBR_TRACE_EXTRA(x) TRACE(x)
#else
#define SOCKETMBR_TRACE_EXTRA(x)
#endif

// ===============================================================================================================
//
// MBR - Multi-Bit-Rate (Adaptive Streaming) uses two classes.
//
// CSocketMbr is the container class - takes a url of "mbr:///?src=manifest-location"
//     It reads and parses the manifest and creates the chunk readers for each track
//
// CSocketMbrChunk is the chunk reader class - takes a url in manifest-chunkurl format
//     It reads and parses the chunk header and provides streaming of frames within the
//     chunk.  Must have access to the manifest in CSocketMbr.
//
// ===============================================================================================================

#define VIDEO_BLOCK_SIZE (32*1024)
#define AUDIO_BLOCK_SIZE (8*1024)
#define TEXT_BLOCK_SIZE (16*1024)                  // 10K should be sufficient. Increase temporily for puppetry stream from DR

#define DEFAULT_RECV_BLOCK_SIZE (16*1024)          // 16 KB read on the socket by default
#define MAX_RECV_BLOCK_SIZE (8*DEFAULT_RECV_BLOCK_SIZE)

#define DEFAULT_MAX_BANDWIDTH (10*1024*1024)       // 10 mbps

#define RATE_PLUS_OVERHEAD(R, P) ((R) * (P) / 100)

#define MBR_MIN_LIVE_RETRY         2               //Minimum retries for non-412 response errors

#define MBR_MIN_WAIT_INTERVAL      100             //Minimum time to wait between retries
#define MBR_MAX_WAIT_INTERVAL      (60 * 1000)     //Maximum time to wait between retries

#define MBR_CHUNK_DOWNLOAD_PREROLL 200             //Reserve 200 ms time between us sending out the request and
                                                   //the first byte of data reaches decoder

// Interval in milliseconds to wait before retrying to send a fragment request when
// the FIFO doesn't have enough headroom for a bitrate measurement.

#define RETRY_INTERVAL_TO_SEND_CHUNK_REQUEST_WHEN_FIFO_IS_TOO_FULL_MS   100

static const size_t c_szHeader_maxsize = 120;

// ===============================================================================================================
// ===============================================================================================================

MbrTuneInfo::MbrTuneInfo()
    : _speed(1)
    , _floorIt(true)
    , _fps(DEFAULT_FPS_DURING_TRICKS)
    , _hnsStartPos(0)
    , _maxBandwidth(DEFAULT_MAX_BANDWIDTH)
    , _ql(0)
{
}

void MbrTuneInfo::Init( _In_ CTuneRequest& tuneRequest, _In_ int64 hnsMarkIn )
{
    //Frames per second to be played back during tricks
    _fps = gTrickConfiguration.FramesPerSecondForTricks;

    //Current speed of playback
    _speed = tuneRequest.Speed;

    //Taking a floor at the given position
    _floorIt = tuneRequest.FloorIt;

    //Bitrate limit in bps to cap the network bandwidth at
    _maxBandwidth = tuneRequest.BitRate ? tuneRequest.BitRate : DEFAULT_MAX_BANDWIDTH;

    //Fixed quality level (1=lowest, N=highest, 255=test)
    _ql = (byte)tuneRequest.GetInt(TUNE_REQUEST_QUALITYLEVEL);

    //Tune start at specified 10MHz time
    _hnsStartPos = NTP_UINT64TO10MHZ(tuneRequest.Rap);

    _hnsStartPos = max( _hnsStartPos, hnsMarkIn );
}


bool MbrTuneInfo::MoveChunkIndex(
    _Inout_ MBR::CMediaPosition* pPos,
    _In_ MBR::CMediaStreamDescription* pMSD )
{
    int32 firstChunkIndex = pMSD->GetDVRMinChunkIndex();
    int32 lastChunkIndex = pMSD->GetDVRMaxChunkIndex();

    //Adjust delta according to speed.
    if (IsInTrickMode())
    {
        int64 hnsDeltaPerFrame = (int64)_speed * 10000000 / _fps;

        int32 preChunkIndex = pPos->ChunkIndex();

        int64 targetPos = pPos->Time_hns() + hnsDeltaPerFrame;

        pMSD->FindPositionByTime( targetPos, _floorIt, pPos );

        int32 chunkIndex = pPos->ChunkIndex();

        if( preChunkIndex == chunkIndex )
        {
            if( ( chunkIndex == firstChunkIndex ) && ( _speed < 0 ) )
            {
                //Repeating first chunk means we hit the beginning,
                //set chunkIndex to off beginning so caller knows to stop

                --chunkIndex;
            }
            else if( ( chunkIndex == lastChunkIndex ) && ( _speed > 0 ) )
            {
                //Repeating the last chunk means we hit the end,
                //set chunkIndex to off end so caller knows to stop

                ++chunkIndex;
            }
            else
            {
                //Repeat the chunk.
                SOCKETMBR_TRACE(("MoveChunkIndex() stayed at : %d", chunkIndex));
            }
        }

        *pPos = MBR::CMediaPosition( targetPos, chunkIndex );
    }
    else
    {
        // The only speeds allowed for
        // non-trick mode are 1x or paused
        ASSERT( _speed == 1 || _speed == 0 );

        int32 chunkIndex = pPos->ChunkIndex();

        ++chunkIndex;

        int64 hnsChunkTime = pMSD->GetChunkStartPosition( chunkIndex );

        *pPos = MBR::CMediaPosition( hnsChunkTime, chunkIndex );
    }

    return( ( firstChunkIndex <= pPos->ChunkIndex() ) && ( pPos->ChunkIndex() <= lastChunkIndex ) );
}

// ===============================================================================================================
// Socket to read from an Http server
// ===============================================================================================================

CSocketMbrChunk::CSocketMbrChunk(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType/* = eSocketType_MbrChunk*/)
    : CSocketRaw(avManager, pipeIdN, tuneRequest, socketType)
    , _pChunkSocket(NULL)
    , _pMbrSocket(NULL)
    , _pMbrManifest(NULL)
    , _apChunkManifest(NULL)
    , _pHeuristics(NULL)
    , _pMSD(NULL)
    , _mediaType(MediaStreamTypeVideo)
    , _mediaSubType(MediaStreamSubTypeUnknown)
    , _streamId(0)
    , _fragmentsAhead(0)
    , _drmHandle(DRM_INVALID_LICENSE)
    , _isPlayReadyEncrypted(false)
    , _pTuneInfo(NULL)
    , _mustRequestNextChunk(true)
    , _chunkHeader(false)
    , _mbrIndex(INVALID_UINT32)
    , _nextMbrIndex(0)
    , _hnsNextDts(0)
    , _hnsStartPos(0)
    , _chunkBuffer(NULL)
    , _chunkBlockSize(0)
    , _chunkHeaderLength(0)
    , _remainingFrameSize(0)
    , _remainingBufferSize(0)
    , _bufferOffset(0)
    , _remainingMdatByteCount(0)
    , _recvBlockSize(DEFAULT_RECV_BLOCK_SIZE) // 16 KB read on the socket by default
    , _fourcc(0)
    , _pWaveFormatEx(NULL)
    , _signedKeyLen(0)
    , _signedKeyID(NULL)
    , _signedKeyChanged(false)
    , _sampleID(INVALID_SAMPLEID)
    , _subSampleEntries(0)
    , _subSampleAllocSize(0)
    , _subSampleClearBytes(NULL)
    , _subSampleEncryptedBytes(NULL)
    , _measuringRate(true)
    , _mdatSize(0)
    , _tcpWindowSize(0)
    , _ql(0)
    , _fmp4Info(0)
    , _rawDataType(CRawPacket::kDataType_Invalid)
    , _started(false)
    , _resetAudio(false)
    , _signalDiscontinuity(false)
    , _maxStreamBandwidth(0)
#ifdef TV2INTERNAL
    , _diagsForceQualitySwitch(false)
    , _diagsNewQualityLevel(-1)
    , _diagsFakeSSBitrate(0)
#endif
{
    memset(&_OPLData, 0, sizeof(_OPLData));
}

CSocketMbrChunk::~CSocketMbrChunk()
{
    AutoLock lock(&_socketLock);

    //Dump the socket if we have any
    if (_pChunkSocket != NULL)
    {
        //Make sure the socket is disconnected
        if (_pChunkSocket->IsConnected())
        {
            Close();
        }

        //Dump it
        delete _pChunkSocket;
    }

    SAFE_DELETE_ARRAY(_chunkBuffer);

    FreeSubSampleArrays();

    SAFE_DELETE_ARRAY(_signedKeyID);

    if( _pMSD != NULL )
    {
        ReportEvent_DestroyChunkDownloader( this );
        SAFE_RELEASE(_pMSD);
    }
}

void CSocketMbrChunk::Init(CSocketMbr* pSocketMbr, CMediaStreamDescription *pMSD, uint64 hnsPos, bool isTuning)
{
    CHECK_ALLOC(pSocketMbr);

    _pMbrSocket = pSocketMbr;
    _pMbrManifest = _pMbrSocket->_pMbrManifest;
    _apChunkManifest = _pMbrSocket->_apChunkManifest;
    _pHeuristics = _pMbrSocket->_pHeuristics;

    CHECK_ALLOC(_pMbrManifest);
    ASSERT((NULL != _apChunkManifest) && _pHeuristics);

    _pMSD = pMSD;
    _pMSD->AddRef();

    _mediaType = _pMSD->Type();
    _mediaSubType = _pMSD->GetSubType();
    _streamId = pMSD->GetStreamID();
    _fragmentsAhead = _pMSD->GetDVRCount();

    ReportEvent_CreateChunkDownloader(
                        this,
                        GetStreamTypeTraits( _mediaType )->pszDefaultName,
                        _streamId,
                        _pMSD->Name().c_str(),
                        &_rateControl );

    IDrmDecrypter* pDrm = _pMbrManifest->GetDrmObject();
    if (pDrm)
    {
        _drmHandle = pDrm->GetLicenseHandle();
        _isPlayReadyEncrypted = (pDrm->GetEncryptionType() == DrmEncryptionType_PlayReady);
    }

    //Get the tuning parameters
    _pTuneInfo = &_pMbrSocket->_tuneInfo;

    //Start position
    //If init is called at tune time (started is false),
    //get the start position from tune info.
    _hnsStartPos = isTuning ? _pTuneInfo->_hnsStartPos : hnsPos;
    _hnsNextDts = 0;

    //Bitrate limit in bps to cap the network bandwidth at
    _maxStreamBandwidth = _pTuneInfo->_maxBandwidth;
    //Quality level to be targetted as requested
    _ql = _pTuneInfo->_ql;

    if (gAVEngineConfiguration.VodMaxReadSizeKB > 0 && gAVEngineConfiguration.VodMaxReadSizeKB <= MAX_RECV_BLOCK_SIZE/1024)
        _recvBlockSize = gAVEngineConfiguration.VodMaxReadSizeKB * 1024;

    //Frame data is read in fragments, up to these block sizes
    if (_mediaType == MediaStreamTypeVideo)
    {
        _chunkBlockSize = MAX(VIDEO_BLOCK_SIZE, _recvBlockSize);
        _rawDataType = CRawPacket::kDataType_Video;
    }
    else
    if (_mediaType == MediaStreamTypeAudio)
    {
        _chunkBlockSize = MAX(AUDIO_BLOCK_SIZE, _recvBlockSize);
        _rawDataType = CRawPacket::kDataType_Audio;

        if (!isTuning)
        {
            _resetAudio = true;
        }
    }
    else
    if (_mediaType == MediaStreamTypeText)
    {
        _chunkBlockSize = MAX(TEXT_BLOCK_SIZE, _recvBlockSize);
        _rawDataType = CRawPacket::kDataType_DFXP;
    }

    if (_chunkBlockSize)
    {
        _chunkBuffer = new byte[_chunkBlockSize];
        CHECK_ALLOC(_chunkBuffer);
    }

    SOCKETMBR_TRACE(("CSocketMbrChunk::Init(), _mediaType:%d, _maxBandwidth:%d, _recvBlockSize:%d, _hnsStartPos:%lld",
        _mediaType, _maxStreamBandwidth, _recvBlockSize, _hnsStartPos));

    if (_mediaType == MediaStreamTypeAudio)
    {
        uint32 len = 0;

        //Force quality level to default for audio
        _ql = (byte) (_pMSD->GetDefaultQualityLevel() + 1);

        _pWaveFormatEx = (WAVEFORMATEX*)_pMSD->GetCodecBlob(_ql - 1, &len);

        //AAC ADTS fixed header
        if (_pWaveFormatEx)
        {
            if (_pWaveFormatEx->wFormatTag == 0x1601 || _pWaveFormatEx->wFormatTag == 0x00FF)    // RAW_AAC
            {
                _adtsHeader.SetADTSHeader(2, (uint8)_pWaveFormatEx->nChannels, _pWaveFormatEx->nSamplesPerSec);
            }
        }
        else
        {
            ASSERT(false);
        }
    }

    //Get the fourCC
    _fourcc = _pMSD->GetFourCC((_ql > 0 && _ql < 255) ? _ql - 1 : 0);

    //Enable rate control if stream is not live
    _rateControl.Enable(gSocketMbrConfiguration.SSRateControl && !_pMbrManifest->IsLive());

    //We've not started yet.
    _started = false;

    _pMSD->FindPositionByTime( _hnsStartPos, _pTuneInfo->_floorIt, &_mediaPos );

    SendDiagsEvent(new CDiagsSSStreamInfo(
            _mediaType,
            _streamId,
            _pMSD->NumberOfAvailableTracks(),
            _fourcc,
            _pMSD->GetDVRCount(),
            _pMSD->GetDVRMinChunkIndex(),
            _pMSD->GetDVRMaxChunkIndex(),
            _mediaPos.ChunkIndex()));
}

void CSocketMbrChunk::SetQualityLevel()
{
    int iLastSelectedTrack = _pMSD->GetIndexOfLastSelectedTrack();

    _nextMbrIndex = MIN( iLastSelectedTrack, _ql - 1 );

#ifdef TV2INTERNAL
    if (_ql == 255)        // use q=255 for sweep test
    {
        if (iLastSelectedTrack > 0)
        {
            _nextMbrIndex = _mediaPos.ChunkIndex() % iLastSelectedTrack;
            if ((_mediaPos.ChunkIndex() / iLastSelectedTrack) & 1)
                _nextMbrIndex = iLastSelectedTrack - _nextMbrIndex;
        }
        else
        {
            _nextMbrIndex = 0;
        }
    }
#endif
}

bool CSocketMbrChunk::PrepareChunkUrl()
{
    pkRESULT pkResult = pkS_OK;

#ifdef TV2INTERNAL
    if (_diagsForceQualitySwitch && _mediaType == MediaStreamTypeVideo)
    {
        _ql = (byte) _diagsNewQualityLevel;
        TRACE(("PrepareChunkUrl() -- Forcing the Quality Level to: %d", _ql));
    }
#endif

    //Find out quality level first.
    if (_ql)
    {
        //If quality level is specifically assigned, lock to it
        SetQualityLevel();
    }
    else
    {
        //Else, get quality level from heuristics
        //during seek we do not need to get the next chunk, we use the default chunk
        if(IPTV_HAL_Decoder_GetSeekFlag() == false){
            pkResult = _pHeuristics->GetNextChunk( _streamId, _mediaPos.ChunkIndex(), _nextMbrIndex, &_nextMbrIndex );
        }
    }

    if (pkResult == pkS_OK)
    {
        pkResult = _apChunkManifest->GetChunkURL(_streamId, _mediaPos.ChunkIndex(), _nextMbrIndex, _wstrChunkURL, _pTuneInfo->IsInTrickMode());
    }

    return (pkResult == pkS_OK);
}

// Connect using heuristics
bool CSocketMbrChunk::Connect(void)
{
    //Prepare chunk url
    if (PrepareChunkUrl() == false)
    {
        return false;
    }

    _mustRequestNextChunk = false;

    CTuneRequest chunkTuneRequest;
    chunkTuneRequest.ParseUrl(_wstrChunkURL);
    return Connect(chunkTuneRequest);
}

bool CSocketMbrChunk::Connect(CTuneRequest &tuneRequest)
{
    AutoLock socketLock(&_socketLock);

    //Quit if the socket has already been closed
    //Look in the base class for the logic flow on how this happens
    if (!CSocketRaw::Connect())
        return false;

    //Create a socket to read media
    _pChunkSocket = IStreamerHttp::CreateStreamerHttp();
    if (_pChunkSocket == NULL)
    {
        //Save the error state
        _socketError = eSocketErrorHttpCreateFailed;
        TRACE_ERROR(("Unknown protocol %s", tuneRequest.Protocol.c_str()));
        return false;
    }

    //Set the window size
    _pChunkSocket->SetTcpRecvBuffSize(TCP_BUFFER_SIZE_HTTP);

    //
    // During initial connection the start of the chunk request and the start
    // of the chunk response have to be reported together because unfortunately
    // the IStreamerHttp::Connect method implicitly waits for the response and
    // therefore doesn't offer the same granularity that the methods
    // IStreamerHttp::SendHttpRequest and IStreamerHttp::RecvHttpResponse do,
    // which allows the measurement of the interval between sending the request
    // and start receiving the response
    //

    ReportEvent_StartChunkRequest(
            this,
            _streamId,
            _mediaPos.ChunkIndex(),
            _nextMbrIndex,
            _pMSD->TrackWeakPtr( _nextMbrIndex )->Bitrate(),
            _pMSD->GetChunkSizeInKB( _nextMbrIndex, _mediaPos.ChunkIndex() ),
            _wstrChunkURL.c_str() );

    _chunkIndexInResponse = _mediaPos.ChunkIndex();

    ReportEvent_StartChunkResponseHeader( this, _streamId, _chunkIndexInResponse );

    string response;
    string extraHttpHeader;

    // Note: AddBwInfoHeader depends on _chunkIndexInResponse being set
    AddBwInfoHeader(&extraHttpHeader);

#ifdef TV2INTERNAL
    _tickStartChunkRequest = Executive_GetTickCount();
    _tickStartChunkResponseHeader = _tickStartChunkRequest;
#endif

    //Connect socket - prevent redirects for stream content
    if (!_pChunkSocket->Connect( &tuneRequest.TunerUrl, extraHttpHeader, &response, IStreamerHttp::eRedirectDisable))
    {
        //Save the error state
        _socketError = _pChunkSocket->GetSocketError();
        _socketPKResult = _pChunkSocket->GetPKResult();
        _socketHttpResponse = _pChunkSocket->GetHttpResponse();
        TRACE_ERROR(("Failed to tune '%s' err %d pkError 0x%08x httpResponse %d",
            tuneRequest.TunerUrl.c_str(),
            _socketError,
            _socketPKResult,
            _socketHttpResponse));
        return false;
    }

    //Look for media-room session id and store it if it exists
    _pMbrManifest->StoreSessionId(_pChunkSocket);

    //Determine TCP window size
    _tcpWindowSize = _pChunkSocket->GetTcpWindowSize();

    //Set window timeout (time until streamer resends a window update)
    _pChunkSocket->SetWindowTimeout(500);

    if( _pMSD->HasChildStreams() )
    {
        RecvChildChunkHeader();
    }

    SOCKETMBR_TRACE(("GET '%s' pos=%llu size=%d",
        tuneRequest.TunerUrl.c_str(),
        _hnsNextDts,
        _pMSD->GetChunkSizeInKB(_nextMbrIndex, _mediaPos.ChunkIndex()) * 1024));

    //Retrieve chunk header
    return RecvChunkHeader(_hnsNextDts);
}

bool CSocketMbrChunk::IsConnected(void) const
{
    AutoLock lock(&_socketLock);

    if (!_socketConnected)
        return false;

    return _pChunkSocket ? _pChunkSocket->IsConnected() : false;
}

bool CSocketMbrChunk::Close(void)
{
    //We are now disconnected
    CSocketRaw::Close();
    {
        AutoLock lock(&_socketLock);

        //Close the streamer
        if (_pChunkSocket != NULL)
        {
            _pChunkSocket->Close();
        }
    }
    return true;
}

bool CSocketMbrChunk::ConnectSocketAtPos(uint64 hnsTime)
{
    //The socket should not be connected
    ASSERT(!IsConnected());
    CSocketMbrChunkRetry retryInfo;

    _pMSD->FindPositionByTime( hnsTime, _pTuneInfo->_floorIt, &_mediaPos );

    _hnsNextDts = _pMSD->GetChunkStartPosition( _mediaPos.ChunkIndex() );

    while (true)
    {
        SOCKETMBR_TRACE(("CSocketMbrChunk::ConnectSocketAtPos(), media_type:%d, hnsTime:%lld, found chunk idx:%d, start_pos:%lld",
            _mediaType, hnsTime, _mediaPos.ChunkIndex(), _hnsNextDts));

        //Connect and ask server for the chunk
        if (Connect())
        {
            ASSERT(_chunkHeader != false);

            // clear any errors that were set that have been recovered
            _socketError = eSocketErrorNone;
            _socketPKResult = pkS_OK;
            _socketHttpResponse = 0;
            return true;
        }

        if( RetryChunk(&retryInfo) )
        {
            //send status as warning
            if(eSocketErrorHttpInvalidResult == _pChunkSocket->GetSocketError())
            {
                string info = "status=chunkconnecthttpinvalid&httpresponse=" + toString(_pChunkSocket->GetHttpResponse()) +
                              "&pkresult=" + toString(_pChunkSocket->GetPKResult()) +
                              "&url=" + WStr2Str(_wstrChunkURL);
                _pMbrSocket->FireNotification(kReceiverNotificationType_MediaTransportEvent, (CSocketMbr::NOTIFICATION_DATA_VARIANT_TYPE)info.c_str());
            }
            _hnsNextDts = _pMSD->GetChunkStartPosition(_mediaPos.ChunkIndex());
        }
        else
        {
             //be more specific about the error
            if(eSocketErrorHttpInvalidResult  == _socketError)
            {
                _socketError = eSocketErrorSSChunkConnectHttpInvalid;
            }
            return false;
        }
    }
}

// skip to the given position
int CSocketMbrChunk::SkipRecv(uint64 pos)
{
    uint32 skipByteCount = 0;

    MP4FrameInfo next_frame;

    while (1)
    {
        if (_fmp4Info && _fmp4Info->PeekNextFrame(next_frame))
        {
            // all time data returned are in timescale unit, convert them into 100ns unit
            if (_pMSD->TimeScale() != (int64)MBR_DEFAULT_TIMESCALE)
            {
                next_frame.dts = (uint64)((double)next_frame.dts/ _pMSD->TimeScale() * MBR_DEFAULT_TIMESCALE);
                next_frame.pts = (uint64)((double)next_frame.pts/ _pMSD->TimeScale() * MBR_DEFAULT_TIMESCALE);
                next_frame.duration = (uint64)((double)next_frame.duration / _pMSD->TimeScale() * MBR_DEFAULT_TIMESCALE);
            }

            // turn this assert on when DR's timestamp mismatch problem is resolved.
            //ASSERT(_hnsNextDts == next_frame.dts);

            if (next_frame.dts >= pos)
                break;

            _fmp4Info->MoveToNextFrame(next_frame);

            skipByteCount += next_frame.len;
        }
        else
            break;
    }

    if (skipByteCount)
    {
        _chunkHeaderLength = 0;

        _hnsNextDts = next_frame.dts;

        SOCKETMBR_TRACE(("SkipRecv(), media_type:%d, skipping %d bytes to %lld for target pos:%lld",
            _mediaType, skipByteCount, _hnsNextDts, pos));
        return SkipCount(skipByteCount);
    }

    return 0;
}

int CSocketMbrChunk::Recv(CRawPacket &rawPacket)
{
    int cbFrameSize = 0;

    if (_socketConnected)
    {
        if (!_started)
        {
            if (MediaStreamTypeVideo == _pMSD->Type())
            {
                _pHeuristics->Init(_pTuneInfo->IsInTrickMode(), gSocketMbrConfiguration.AVCpuLimit);
                
                // we'll start from quality level 0 unless there is some initial bandwidth sent
                // to the socket
                if(_pMbrSocket->_initialNetworkBitsPerSec != 0)
                {
                    _pHeuristics->AddBandwidth(_pMbrSocket->_initialNetworkBitsPerSec);
                }
            }

            SOCKETMBR_TRACE(("@%p: Init RateControl %p, mediaType %d", this, &_rateControl, _mediaType));

            // each read on socket (all stream types) will be no more than 16KB.
            _rateControl.Init
                (
                RATE_PLUS_OVERHEAD(_maxStreamBandwidth, gSocketMbrConfiguration.SSRateControlOverhead),
                RATE_PLUS_OVERHEAD(_pMSD->GetNominalBitrate(_nextMbrIndex), gSocketMbrConfiguration.SSRateControlThreshold),
                RateControl::MinTimeToMeasure,
                _recvBlockSize
                );

            // _hnsNextDts is set to start of chunk, so use this to connect socket
            if (ConnectSocketAtPos(_hnsStartPos) == false)
            {
                if (eSocketErrorSSDrmInitFailed == _socketError)
                {
                    return eRecvErrorDrmInit;
                }
                else
                {
                    return eRecvErrorReadFailed;
                }
            }

            // for audio streams, toss out any extra frames until we hit the desired trick time
            if (_mediaType == MediaStreamTypeAudio)
            {
                _rateControl.StopRateControl();
                cbFrameSize = SkipRecv(_hnsStartPos);
                _rateControl.ResumeRateControl();
            }

            _started = true;
        }

        if (cbFrameSize >= 0)
        {
            cbFrameSize = RecvNextFrame(rawPacket);
        }
    }

    return cbFrameSize;
}


int CSocketMbrChunk::RecvCount(__out_ecount(dstlen) byte* dst, int dstlen, int dataLength, int timeout/* = 0*/)
{
    int i = 0;

    if (dstlen < dataLength)
    {
        ASSERT(false);
        TRACE_ERROR(("RecvCount: destination buffer length (%d) less than read-count (%d)", dstlen, dataLength));
        return -1;
    }

    if (_chunkHeader && _remainingMdatByteCount < (uint32)dataLength)
    {
        ASSERT(false);
        TRACE_ERROR(("RecvCount: remaining data (%d) in chunk is less than read-count (%d)", _remainingMdatByteCount, dataLength));
        return -1;
    }

    if( _measuringRate && ( _mediaType == MediaStreamTypeVideo ) )
    {
        // For a video handler, turn off rate measurement once the buffer is over capacity,
        // because in that case the transfer is throttled and therefore the measurement
        // wouldn't be reliable.

        DecoderBufferStatus decoderBufferStatus;
        _pMbrSocket->GetDecoderBufferStatus( &decoderBufferStatus );
        _measuringRate = ( decoderBufferStatus.videoBufferLevel90kHz < decoderBufferStatus.maxVideoBufferLevel90kHz );

        // a delay by non-video downloads could throw off rate measure measurement for mid-chunk video downloads
        if(_measuringRate && _pMbrSocket->GetNonVideoIsDelayed())
        {
            _measuringRate = false;
        }
    }

    bool endOfHttp = false;
    while (!endOfHttp && i < dataLength)
    {
        int recv_len = min(dataLength -i, (int)_recvBlockSize);

        if( _measuringRate )
        {
            _rateControl.StartNextReceive(_mediaType,_mbrIndex);
        }

        int c = _pChunkSocket->Recv(dst + i, recv_len, &endOfHttp, timeout);
        if (c <= 0)
        {
            //Save the error state
            _socketError = _pChunkSocket->GetSocketError();
            _socketPKResult = _pChunkSocket->GetPKResult();
            _socketHttpResponse = _pChunkSocket->GetHttpResponse();
            return c;
        }

        i += c;

        if( _measuringRate )
        {
            _rateControl.ReceivedCount(c);
        }
    }

    if( _chunkHeader )
    {
        _remainingMdatByteCount -= dataLength;

        // check for inconsistent byte count
        if ( ( endOfHttp && _remainingMdatByteCount > 0 ) ||
             ( !endOfHttp && _remainingMdatByteCount == 0 ) )
        {
            TRACE_ERROR(("RecvCount: inconsistent byte count, endOfHttp (%s) _remainingMdatByteCount (%d)", 
                endOfHttp ? "TRUE" : "FALSE", _remainingMdatByteCount));

            return -1;
        }
    }

    // no more chunk data?
    if (endOfHttp)
    {
        _chunkHeader = false;

        ReportEvent_EndChunkRequest( this, _streamId, _chunkIndexInResponse );

#ifdef TV2INTERNAL
        _tickEndChunkRequest2 = _tickEndChunkRequest;
        _tickEndChunkRequest = Executive_GetTickCount();
#endif
    }
    else
    {
        // issue next http request before the data are completely drained.
        // this is to avoid burstiness on the wire. also an effort of maximizing
        // throughput.
        if( _chunkHeader && _remainingMdatByteCount <  _tcpWindowSize*gSocketMbrConfiguration.SSPipelineMultiplier && _mustRequestNextChunk )
        {
            if (SendNextChunkRequest() == pkS_OK)
            {
                _mustRequestNextChunk = false;

#ifdef TV2INTERNAL
                _processedBytes = _mdatSize - _remainingMdatByteCount;
                _remainingBytes = _remainingMdatByteCount;
#endif
            }
        }
    }

    return dataLength;
}

int CSocketMbrChunk::SkipCount(int bytesToSkip)
{
    uint32 bytes = (uint32)bytesToSkip;
    while (bytes)
    {
        uint32 len = min(bytes, _chunkBlockSize);
        int rc = RecvCount(_chunkBuffer, _chunkBlockSize, len);
        if (rc != (int) len)
            return rc;
        bytes -= len;
    }
    return bytesToSkip;
}

int CSocketMbrChunk::SkipToEnd()
{
    int bytesSkipped = 0;
    bool endOfHttp = false;
    while (!endOfHttp)
    {
        int rc = _pChunkSocket->Recv(_chunkBuffer, _chunkBlockSize, &endOfHttp);
        if (rc <= 0)
            return rc;
        bytesSkipped += rc;
    }

    return bytesSkipped;
}

bool CSocketMbrChunk::GetNextFrameInfo( _Out_ FRAME_INFO* pFI )
{
    MP4FrameInfo next_frame;

    if (_fmp4Info && _fmp4Info->GetNextFrame(next_frame))
    {
        SOCKETMBR_TRACE_EXTRA(("frame info(type:%d), dts:%lld, pts:%lld, duration:%lld, rap:%s, sync:%s, frame index:%d",
            _mediaType,
            next_frame.dts,
            next_frame.pts,
            next_frame.duration,
            next_frame.is_rap ? "yes" : "no",
            next_frame.is_sync ? "yes" : "no",
            next_frame.frame_index));

        // all time data returned are in timescale unit, convert them into 100ns unit
        if (_pMSD->TimeScale() != (int64)MBR_DEFAULT_TIMESCALE)
        {
            next_frame.duration = (uint64)((double)next_frame.duration / _pMSD->TimeScale() * MBR_DEFAULT_TIMESCALE);
            next_frame.dts = (uint64)((double)next_frame.dts / _pMSD->TimeScale() * MBR_DEFAULT_TIMESCALE);
            next_frame.pts = (uint64)((double)next_frame.pts / _pMSD->TimeScale() * MBR_DEFAULT_TIMESCALE);

            SOCKETMBR_TRACE_EXTRA(("---hns duration:%d, hns dts:%lld, hns pts:%lld",
                next_frame.duration, next_frame.dts, next_frame.pts));
        }
        _hnsNextDts = next_frame.dts + next_frame.duration;
        SOCKETMBR_TRACE_EXTRA(("frame info(type:%d), _hnsNextDts:%lld", _mediaType, _hnsNextDts));

        // fill out outgoing parameters
        pFI->isSync = next_frame.is_sync;
        pFI->isRAP = next_frame.is_rap;
        pFI->dts = next_frame.dts;
        pFI->pts = next_frame.pts;
        pFI->duration = next_frame.duration;
        pFI->cbFrameSize = next_frame.len;

        // fill out drm info
        SampleEncryptionSubSampleInfo* seInfo = next_frame.se_info;

        if (seInfo)
        {
            pFI->cDrmSubSamples = seInfo->_entry_count;
            pFI->prgDrmSubSampleClearBytes = seInfo->_clear_data;
            pFI->prgDrmSubSampleEncryptedBytes = seInfo->_encrypted_data;
            pFI->sampleID = ( seInfo->_iv_size == sizeof(uint64) ) ? seInfo->_iv : INVALID_SAMPLEID;

#ifdef SOCKETMBR_SPEW_EXTRA

            if( pFI->cDrmSubSamples != 0 )
            {
                SOCKETMBR_TRACE_EXTRA(("drm info: media type:%d, sampleID:%llx, drmEntries:%d",
                    _mediaType, pFI->sampleID, pFI->cDrmSubSamples));

                //
                // Trace up to the first 6 entries
                //

                uint32 cEntries = pFI->cDrmSubSamples;
                if (cEntries > 6)
                {
                    cEntries = 6;
                }

                for( uint32 iEntry = 0; iEntry < cEntries; ++iEntry )
                {
                    SOCKETMBR_TRACE_EXTRA(("entry [%d], clear bytes %d, encrypted bytes %d", iEntry, pFI->prgDrmSubSampleClearBytes[iEntry], pFI->prgDrmSubSampleEncryptedBytes[iEntry]));
                }

                if( cEntries < pFI->cDrmSubSamples )
                {
                    SOCKETMBR_TRACE_EXTRA(("entry [%d] ...", cEntries - 1 ));
                }
            }
#endif
        }

        return true;
    }
    else
    {
        SOCKETMBR_TRACE(("GetNextFrame() failed, media type:%d, _fmp4Info:%x", _mediaType, _fmp4Info));
    }

    return false;
}

int CSocketMbrChunk::RecvNextFrame(CRawPacket& rawPacket)
{
    int ret = 0;

    //If we don't have any open chunk, go get the next one
    if (_chunkHeader == false)
    {
        //check if RecvNextChunk failed from a socketerror
        if((RecvNextChunk() != pkS_OK) && (eSocketErrorNone != _socketError))
        {
            if (eSocketErrorSSDrmInitFailed == _socketError)
            {
                return eRecvErrorDrmInit;
            }
            else
            {
                return eRecvErrorReadFailed;
            }
        }

        if (_chunkHeader == false)
        {
            //No more chunks in this stream
            SOCKETMBR_TRACE(("RecvNextFrame() -- No more chunks"));
            return 0;
        }

#ifdef TV2INTERNAL
        uint32 ticks = Executive_GetTickCount();

        // Send a chunk info diag event
        if (_mediaPos.ChunkIndex() > 0)
        {
            SendDiagsEvent(new CDiagsSSChunkInfo
                (
                _streamId,
                _mediaPos.ChunkIndex() - 1,
                _tickEndChunkRequest - _tickStartChunkRequest,
                _tickStartChunkResponseHeader - _tickEndChunkRequest,
                _tickStartChunkResponseBody - _tickStartChunkResponseHeader,
                ticks - _tickStartChunkResponseBody,
                _tickEndChunkRequest - _tickEndChunkRequest2,
                _mbrIndex,
                _rateControl.GetSleepTimeSinceLastSampling(),
                _processedBytes,
                _remainingBytes
                ));
        }
#endif
    }

    if (_chunkHeaderLength > 0)
    {
        //Get the chunk header first
        //A receiver may be interested in it
        ret = RecvNextFrame_ChunkHeader(rawPacket);
    }
    else
    {
        //Get the next frame
        if (_mediaType == MediaStreamTypeVideo)
        {
            ret = RecvNextFrame_video(rawPacket);
        }
        else
        if (_mediaType == MediaStreamTypeAudio)
        {
            ret = RecvNextFrame_audio(rawPacket);
        }
        else
        if (_mediaType == MediaStreamTypeText)
        {
            ret = RecvNextFrame_text(rawPacket);
        }
        else
        {
            ASSERT(false);
        }
    }

    if (ret < 0)
    {
        //Close chunk header
        _chunkHeader = false;
    }

    return ret;
}

int CSocketMbrChunk::RecvNextFrame_ChunkHeader(CRawPacket& rawPacket)
{
    //***** TO BE DONE *****
    // TODO: Assumes that the chunk headers are always received in one shot
    //Is this a fair assumption?

    rawPacket.Init(_rawDataType, true);
    rawPacket.Data = _chunkBuffer;
    rawPacket.DataLength = _chunkHeaderLength;
    rawPacket.StreamId = _streamId;

    rawPacket.Pts = IS_VALID_TIME(_hnsNextDts) ? PTS_10MHZTO90KHZ(_hnsNextDts) : INVALID_TIME;
    rawPacket.Rap = true;
    rawPacket.PCR = rawPacket.Pts;
    rawPacket.NTP = NTP_10MHZTOUINT64(_hnsNextDts);

    rawPacket.QualityLevel = (uint16) _mbrIndex;

    SOCKETMBR_TRACE_EXTRA(("GetNextFrame_ChunkHeader(), media type:%d, header length:%d", _mediaType, _chunkHeaderLength));

    _chunkHeaderLength = 0;

    return rawPacket.DataLength;
}

int CSocketMbrChunk::RecvNextFrame_video(CRawPacket& rawPacket)
{
    FRAME_INFO fi;

    //Do we need to start a new frame?
    if (_remainingFrameSize == 0)
    {
        //Find out next frame's information
        bool isInfoValid = GetNextFrameInfo( &fi );

        _sampleID = fi.sampleID;
        _subSampleEntries = fi.cDrmSubSamples;
        _remainingFrameSize = fi.cbFrameSize;

        if (!isInfoValid)
        {
            TRACE_ERROR(("RecvNextFrame_video() : GetNextFrameInfo() failed."));
            return eRecvErrorBadData;
        }

        //Is there enough data left in mdat for the frame?
        if (_remainingFrameSize > _remainingMdatByteCount + _remainingBufferSize)
        {
            TRACE_ERROR(("RecvNextFrame_video(): frame info inconsistent!!!"));
            ASSERT(false);
            return eRecvErrorBadData;
        }

        if (_subSampleEntries)
        {
            //We need to make local copy in case we delete frame info
            SetSubSample(fi.cDrmSubSamples, fi.prgDrmSubSampleClearBytes, fi.prgDrmSubSampleEncryptedBytes);
        }
    }

    if ((_remainingBufferSize == 0) && (_remainingFrameSize > 0))
    {
        //Read another 32K block if possible (but only up to end of frame)
        int bytes2read = min(min(_chunkBlockSize, _remainingMdatByteCount), _remainingFrameSize);
        int rc = RecvCount(_chunkBuffer, _chunkBlockSize, bytes2read);
        if (rc != bytes2read)
        {
            //Something is wrong with the socket
            TRACE_ERROR(("RecvNextFrame_video(): RecvCount() failed: %d --> %d", bytes2read, rc));
            return eRecvErrorReadFailed;
        }

        //Buffer size
        _remainingBufferSize = bytes2read;
        //Reset buffer offset
        _bufferOffset = 0;
    }

    int dataLength = 0;
    rawPacket.Init(CRawPacket::kDataType_Video);

    //Now construct the frame
    if (_remainingFrameSize)
    {
        //How much data we can send to decoder?
        dataLength = min(_remainingBufferSize, _remainingFrameSize);

        //Stream Id
        rawPacket.StreamId = _streamId;

        //Prepare the raw data packet
        rawPacket.Pts = IS_VALID_TIME(fi.pts) ? PTS_10MHZTO90KHZ(fi.pts) : INVALID_TIME;
        rawPacket.Dts = IS_VALID_TIME(fi.dts) ? PTS_10MHZTO90KHZ(fi.dts) : INVALID_TIME;
        rawPacket.Rap = fi.isRAP;
        rawPacket.IsSync = fi.isSync;
        rawPacket.Duration = IS_VALID_TIME(fi.duration) ? PTS_10MHZTO90KHZ(fi.duration) : INVALID_TIME;

        //Some older content does not set isSync correctly
        if (fi.isRAP || fi.isSync)
        {
            rawPacket.PCR = rawPacket.Pts;
            rawPacket.NTP = NTP_10MHZTOUINT64(fi.pts);
        }

        // video frames must have fourCC value
        ASSERT(_fourcc);
        rawPacket.FourCC = _fourcc;

        rawPacket.DrmHandle = _drmHandle;
        rawPacket.SampleID = _sampleID;

        if (_signedKeyChanged && fi.isSync)
        {
            rawPacket.KeyID = _signedKeyID;
            rawPacket.KeyIDLength = _signedKeyLen;
            _signedKeyChanged = false;
        }

        rawPacket.SubSampleEntries = _subSampleEntries;
        if (_subSampleEntries)
        {
            rawPacket.SubSampleClearBytes = _subSampleClearBytes;
            rawPacket.SubSampleEncryptedBytes = _subSampleEncryptedBytes;
        }

        rawPacket.Data = _chunkBuffer + _bufferOffset;
        rawPacket.DataLength = dataLength;
        rawPacket.Video.FrameHeight = _pMSD->GetHeight(_mbrIndex);
        rawPacket.Video.FrameWidth = _pMSD->GetWidth(_mbrIndex);

        //Private data may be available, but it is already embedded in band - need a way
        //to determine whether this should or should not be attached
        uint32 nCodecBlobLen = 0;
        byte* pCodecBlob = _pMSD->GetCodecBlob(_mbrIndex, &nCodecBlobLen);

        if (pCodecBlob && (_fourcc == MAKEFOURCC('H','2','6','4') || (_fourcc == MAKEFOURCC('A','V','C','1'))))
        {
            rawPacket.Flags |= RAWPACKET_IS_MPEG4_PART15;
            rawPacket.Video.PrivateData = pCodecBlob;
            rawPacket.Video.PrivateDataLen = nCodecBlobLen;
            rawPacket.Video.NalUnitLength = _pMSD->TrackWeakPtr( _mbrIndex )->NALUnitLength();
        }
        else
        if (_fourcc == MAKEFOURCC('W','M','V','3'))
        {
            rawPacket.Video.PrivateData = pCodecBlob;
            rawPacket.Video.PrivateDataLen = nCodecBlobLen;
            rawPacket.Video.PixelAspectX = 1;
            rawPacket.Video.PixelAspectY = 1;
        }
        else
        {
            rawPacket.Flags |= RAWPACKET_IS_STARTCODE_INBAND;
            rawPacket.Video.PrivateData = 0;
            rawPacket.Video.PrivateDataLen = 0;
        }

        SOCKETMBR_TRACE_EXTRA(("RecvNextFrame(video), rap:%s, is sync:%s, is start-code-inband:%s, fourcc:%c%c%c%c, blobLen:%d, pts:%lld, data:%x, dataLength:%d, height:%d, width:%d",
            rawPacket.Rap ? "yes": "no",
            rawPacket.IsSync ? "yes": "no",
            ((rawPacket.Flags & RAWPACKET_IS_STARTCODE_INBAND) != 0) ? "yes": "no",
            ((char*)&rawPacket.FourCC)[0], ((char*)&rawPacket.FourCC)[1], ((char*)&rawPacket.FourCC)[2], ((char*)&rawPacket.FourCC)[3],
            rawPacket.Video.PrivateDataLen,
            rawPacket.Pts,
            rawPacket.Data,
            rawPacket.DataLength,
            rawPacket.Video.FrameHeight,
            rawPacket.Video.FrameWidth
            ));

        //Adjust member variables for the frame.
        ASSERT(_remainingFrameSize >= (uint32)dataLength);
        ASSERT(_remainingBufferSize >= (uint32)dataLength);
        ASSERT(_bufferOffset <= _chunkBlockSize);

        _remainingFrameSize -= dataLength;
        _remainingBufferSize -= dataLength;
        _bufferOffset += dataLength;

        if (_remainingFrameSize == 0)
        {
            rawPacket.Flags |= RAWPACKET_IS_ENDOFFRAME;
        }
    }

    rawPacket.QualityLevel = (uint16) _mbrIndex;

    return dataLength;
}

int CSocketMbrChunk::RecvNextFrame_audio(CRawPacket& rawPacket)
{
    FRAME_INFO fi;

    //Do we need to start a new frame?
    if (_remainingFrameSize == 0)
    {
        //Find out next frame's information
        bool isInfoValid = GetNextFrameInfo( &fi );

        _sampleID = fi.sampleID;
        _subSampleEntries = fi.cDrmSubSamples;
        _remainingFrameSize = fi.cbFrameSize;

        if (!isInfoValid)
        {
            TRACE_ERROR(("RecvNextFrame_audio() : GetNextFrameInfo() failed."));
            return eRecvErrorBadData;
        }

        //Is there enough data left in mdat for the frame?
        if (_remainingFrameSize > _remainingMdatByteCount + _remainingBufferSize)
        {
            TRACE_ERROR(("RecvNextFrame_audio(): frame info inconsistent!!!"));
            ASSERT(false);
            return eRecvErrorBadData;
        }

        if (_subSampleEntries)
        {
            //We need to make local copy in case we delete frame info
            SetSubSample( fi.cDrmSubSamples, fi.prgDrmSubSampleClearBytes, fi.prgDrmSubSampleEncryptedBytes );
        }
    }

    if ((_remainingBufferSize == 0) && (_remainingFrameSize > 0))
    {
        //Read another block if possible (but only up to end of frame)
        int bytes2read = min(min(_chunkBlockSize, _remainingMdatByteCount), _remainingFrameSize);
        int rc = RecvCount(_chunkBuffer, _chunkBlockSize, bytes2read);
        if (rc != bytes2read)
        {
            //Something is wrong with the socket
            TRACE_ERROR(("RecvNextFrame_audio(): RecvCount() failed: %d --> %d", bytes2read, rc));
            return eRecvErrorReadFailed;
        }

        //Buffer size
        _remainingBufferSize = bytes2read;
        //Reset buffer offset
        _bufferOffset = 0;
    }

    int dataLength = 0;
    rawPacket.Init(CRawPacket::kDataType_Audio);

    //Now construct the frame
    if (_remainingFrameSize)
    {
        //How much data we can send to decoder?
        dataLength = min(_remainingBufferSize, _remainingFrameSize);

        rawPacket.StreamId = _streamId;

        //Prepare the raw data packet
        rawPacket.Pts = IS_VALID_TIME(fi.pts) ? PTS_10MHZTO90KHZ(fi.pts) : INVALID_TIME;
        rawPacket.Dts = IS_VALID_TIME(fi.dts) ? PTS_10MHZTO90KHZ(fi.dts) : INVALID_TIME;
        rawPacket.Rap = false;
        rawPacket.IsSync = fi.isSync;
        rawPacket.Duration = IS_VALID_TIME(fi.duration) ? PTS_10MHZTO90KHZ(fi.duration) : INVALID_TIME;

        rawPacket.DrmHandle = _drmHandle;
        rawPacket.SampleID = _sampleID;

        if (_signedKeyChanged && fi.isSync)
        {
            rawPacket.KeyID = _signedKeyID;
            rawPacket.KeyIDLength = _signedKeyLen;
            _signedKeyChanged = false;
        }

        rawPacket.SubSampleEntries = _subSampleEntries;
        if (_subSampleEntries)
        {
            rawPacket.SubSampleClearBytes = _subSampleClearBytes;
            rawPacket.SubSampleEncryptedBytes = _subSampleEncryptedBytes;
        }

        rawPacket.Data = _chunkBuffer + _bufferOffset;
        rawPacket.DataLength = dataLength;

        rawPacket.ParseWaveFormatEx(_pWaveFormatEx);
        if (_fourcc)
        {
            //use fourcc as first priority
            rawPacket.FourCC = _fourcc;
        }

        if (_pWaveFormatEx && (_pWaveFormatEx->wFormatTag == 0x1601 || _pWaveFormatEx->wFormatTag == 0x00FF))
        {
            //AAC ADTS variable header
            _adtsHeader.SetFrameLength((uint16)rawPacket.DataLength + ADTSHeader::SIZE);
            rawPacket.Audio.HeaderDataLen = ADTSHeader::SIZE;
            rawPacket.Audio.HeaderData = _adtsHeader.GetHeaderBits();
        }

        SOCKETMBR_TRACE_EXTRA(("RecvNextFrame(audio), is sync:%s, fourcc:%d, pts:%lld, data:%x, dataLength:%d, version:%d, sampling rate:%d, channel num:%d, Bps:%d, blk_size:%d, encoder_option:%d",
            rawPacket.Rap ? "yes": "no",
            rawPacket.FourCC,
            rawPacket.Pts,
            rawPacket.Data,
            rawPacket.DataLength,
            rawPacket.Audio.VersionNumber,
            rawPacket.Audio.SamplingRate,
            rawPacket.Audio.ChannelNumber,
            rawPacket.Audio.BytesPerSec,
            rawPacket.Audio.BlockSize,
            rawPacket.Audio.EncoderOption
            ));

        _remainingFrameSize -= dataLength;
        _remainingBufferSize -= dataLength;
        _bufferOffset += dataLength;

        if (_remainingFrameSize == 0)
        {
            rawPacket.Flags |= RAWPACKET_IS_ENDOFFRAME;
        }

        if (_resetAudio)
        {
            rawPacket.Flags |= RAWPACKET_IS_RESETAUDIO;
            _resetAudio = false;
            SOCKETMBR_TRACE(("GetNextFrame_audio(), reset flag is set"));
        }
    }

    rawPacket.QualityLevel = (uint16) _mbrIndex;

    return dataLength;
}

int CSocketMbrChunk::RecvNextFrame_text(CRawPacket& rawPacket)
{
    FRAME_INFO fi;

    //Find out next frame's information
    if( !GetNextFrameInfo( &fi ) )
    {
        TRACE_ERROR(("RecvNextFrame_text() : GetNextFrameInfo() failed."));
        return eRecvErrorBadData;
    }

    _remainingFrameSize = fi.cbFrameSize;

    //Is there enough data left in mdat for the frame?
    if (_remainingFrameSize > _remainingMdatByteCount)
    {
        TRACE_ERROR(("RecvNextFrame_text(): frame info inconsistent!!!"));
        ASSERT(false);
        return eRecvErrorBadData;
    }

    //Read another block if possible
    int bytes2read = min(_chunkBlockSize, _remainingMdatByteCount);
    int rc = RecvCount(_chunkBuffer, _chunkBlockSize, bytes2read);
    if (rc != bytes2read)
    {
        //Something is wrong with the socket
        TRACE_ERROR(("RecvNextFrame_text(): RecvCount() failed: %d --> %d", bytes2read, rc));
        return eRecvErrorReadFailed;
    }

    //Buffer size
    _remainingBufferSize = bytes2read;

    int dataLength = 0;

    //How much data we can send to decoder?
    dataLength = _remainingBufferSize;

    rawPacket.Init(CRawPacket::kDataType_DFXP);
    rawPacket.StreamId = _streamId;

    //Prepare the raw data packet
    rawPacket.Flags |= RAWPACKET_IS_ENDOFFRAME;
    rawPacket.Rap = true;
    rawPacket.Data = _chunkBuffer;
    rawPacket.DataLength = dataLength;
    rawPacket.Subtitles.startTime = fi.pts;
    rawPacket.Duration = fi.duration;

    SOCKETMBR_TRACE_EXTRA(("RecvNextFrame(text),dataLength:%d",
        rawPacket.DataLength
        ));

    _remainingBufferSize = 0;
    _remainingFrameSize -= dataLength;

    return dataLength;
}

// Parse the HTTP header for information about the
// stream's sparse children
void CSocketMbrChunk::RecvChildChunkHeader()
{
    bool fFound = false;
    std::string strHeaderValue;

    for( size_t i = 0; i < NUM_OF_HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO; ++i )
    {
        if( _pChunkSocket->GetResponseHeader( HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO[i], &strHeaderValue ) )
        {
            CSparseStreamChunkInfoHeaderParser parser( strHeaderValue.c_str() );
            while( parser.MoveNext() )
            {
                _pMSD->AddSparseChildChunkInfo( parser.CurrentStreamName(), parser.CurrentChunkTime() );
                fFound = true;
            }

            if( fFound )
            {
                break;
            }
        }
    }
}

//Read a chunk at URL for size cbChunk @ hnsStartPos
bool CSocketMbrChunk::RecvChunkHeader(uint64 hnsStartPos)
{
    bool isChunkHdrRecv = true;
    uint32 offset = 0;

    _chunkHeaderLength = 0;

    ReportEvent_StartChunkResponseBody( this, _streamId, _chunkIndexInResponse );

#ifdef TV2INTERNAL
    _tickStartChunkResponseHeader = Executive_GetTickCount();
#endif

    // Tell rate control about the current stream rate.
    // It is a no-op if there is no quality level change.
    _rateControl.SetControlledRate
        (
        RATE_PLUS_OVERHEAD(_maxStreamBandwidth, gSocketMbrConfiguration.SSRateControlOverhead),
        RATE_PLUS_OVERHEAD(_pMSD->GetNominalBitrate(_nextMbrIndex), gSocketMbrConfiguration.SSRateControlThreshold)
        );

    static const size_t NUM_BYTES_BOX_TYPESIZE = 8; // 'mdat' length includes 8 bytes for the size and type
    
    _mdatSize = _fmp4Parser.Prefetch(MP4Atom::eAtomType_mdat, static_cast<MP4Streamer*>(this), _chunkBuffer, _chunkBlockSize, &offset) - NUM_BYTES_BOX_TYPESIZE;
    _remainingMdatByteCount = _mdatSize;

    if (_mdatSize)
    {
        _fmp4Parser.Init(MP4Feed::MP4Feed_Memory, _chunkBuffer, offset);

        if (_fmp4Parser.Parse())
        {
            //Get the info object
            _fmp4Info = (FMP4Info*)_fmp4Parser.GetInfo();

            if (_pMSD->TimeScale() != (int64)MBR_DEFAULT_TIMESCALE)
            {
                hnsStartPos = (uint64)((double)hnsStartPos / MBR_DEFAULT_TIMESCALE * _pMSD->TimeScale());
            }

            if (_fmp4Info->StartRetrieveFrameInfo(hnsStartPos))
            {
                _chunkHeader = true;
                _chunkHeaderLength = offset;

                FMP4TrackInfo* track_info = (FMP4TrackInfo*)_fmp4Info->GetTrackInfo();

                SOCKETMBR_TRACE_EXTRA(("RecvChunkHeader(), media type:%d, frame count:%d, track duration:%lld, track timescale:%d",
                    _mediaType, track_info->_frames_count, track_info->_track_duration, track_info->_track_time_scale));

                //_live_frags_ahead (fragment_count field in traf-uuid box)
                //may not be present in every chunk, but when it's present,
                //it has to be greater than 0.
                if (track_info->_live_frags_ahead)
                {
                    _fragmentsAhead = track_info->_live_frags_ahead;
                }
                else
                {
                    _fragmentsAhead -= 1;
                }

                GUID *pGuid = 0;


                if (_pMbrManifest->IsDecrypterOnDemand())
                {
                    bool fEncrypted = false;
                    if (track_info->_se_info)
                    {
                        pGuid = (GUID*)track_info->_se_info->_kid;
                        fEncrypted = track_info->_se_info->_algorithm_id != 0;
                    }

                    if (fEncrypted)
                    {
                        if (track_info->_pss_info)
                        {
                            ASSERT(track_info->_se_info != NULL);

                            pkRESULT pkResult = _pMbrManifest->InitDrmWithHeader(
                                        track_info->_pss_info->_data,
                                        track_info->_pss_info->_data_size,
                                        (byte*)track_info->_se_info->_kid,
                                        sizeof(GUID));

                            if (pkFAILED(pkResult) || pkResult == pkS_FALSE)
                            {
                                TRACE_ERROR(("RecvChunkHeader() SID %u, _pMbrManifest->InitDrmWithHeader() failed.",
                                    _pMSD->GetStreamID()));
                                _socketError = eSocketErrorSSDrmInitFailed;

                                 string info = "status=drminiterror";
                                _pMbrSocket->FireNotification(kReceiverNotificationType_MediaTransportEvent, (CSocketMbr::NOTIFICATION_DATA_VARIANT_TYPE)info.c_str());
                                isChunkHdrRecv = false;
                            }
                            else
                            {
                                IDrmDecrypter* pDrm = _pMbrManifest->GetDrmObject();
                                if (pDrm)
                                {
                                    _drmHandle = pDrm->GetLicenseHandle();
                                    _isPlayReadyEncrypted = (pDrm->GetEncryptionType() == DrmEncryptionType_PlayReady);

                                    XDRM_OPL_DATA *pOPLData = _pMbrManifest->GetDrmOPLData();
                                    assert(pOPLData != NULL);

                                    if (!XDRM_IS_OPL_DATA_EMPTY(pOPLData) && memcmp(pOPLData, &_OPLData, sizeof(XDRM_OPL_DATA)) != 0)
                                    {
                                        _OPLData = *pOPLData;

                                        // A reference of the OPL data cached in the manifest is sent along with the notification.
                                        // It is valid because of the fact that the manifest is still alive when the notification is handled.
                                        _pMbrSocket->FireNotification(kReceiverNotificationType_DrmOutputProtectionLevel, (CSocketMbr::NOTIFICATION_DATA_VARIANT_TYPE)pOPLData);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        _drmHandle = DRM_INVALID_LICENSE;
                        _isPlayReadyEncrypted = false;
                    }

                    if (pGuid && _isPlayReadyEncrypted)
                    {
                        SOCKETMBR_TRACE(("RecvChunkHeader() SID %u, calling SetSignedKeyID().",
                            _pMSD->GetStreamID()));
                        SetSignedKeyID((byte *) pGuid, sizeof(GUID));
                    }
                }

                //We're at the last known chunk, so disable chunk update thread and add new chunk(s) to buffer
                if (_pMbrManifest->IsLive() && (!_pTuneInfo->IsInTrickMode()) && (_mediaPos.ChunkIndex() >= (int32) _pMSD->GetDVRMaxChunkIndex()))
                {
                    SOCKETMBR_TRACE(("RecvChunkHeader() SID %u at last known chunk: %d >= %u",
                        _pMSD->GetStreamID(),
                        _mediaPos.ChunkIndex(),
                        _pMSD->GetDVRMaxChunkIndex()));

                    _pMbrManifest->EnableChunkUpdateThread(false);
                    _pMbrManifest->AddChunk(_streamId, track_info);
                }

                //Init chunk related members
                _remainingFrameSize = 0;
                _remainingBufferSize = 0;
                _bufferOffset = 0;
                _subSampleEntries = 0;

                _pTuneInfo->MoveChunkIndex( &_mediaPos, _pMSD );
            }
            else
            {
                TRACE_ERROR(("RecvChunkHeader(), StartRetrieveFrameInfo() failed."));
            }
        }
        else
        {
            TRACE_ERROR(("RecvChunkHeader(), mp4 parsing failed"));
        }
    }
    else
    {
        TRACE_ERROR(("RecvChunkHeader(), Prefetch() failed, _mediaType:%d, fetched", _mediaType));
    }

    //We should only get here on parse error or unsupported chunk, skip the rest of the chunk
    //so can get the next request.
    if (!_chunkHeader)
    {
        //Skip the rest of the chunk data if there was parsing error
        SkipToEnd();

        //notify of the parsing error
        string info = "status=chunkhdrerror&url=" + WStr2Str(_wstrChunkURL);
        _pMbrSocket->FireNotification(kReceiverNotificationType_MediaTransportEvent, (CSocketMbr::NOTIFICATION_DATA_VARIANT_TYPE)info.c_str());

        //record the failure
        _socketError = eSocketErrorSSChunkHdrParsingFailed;
        isChunkHdrRecv = false;
    }

    if (_nextMbrIndex != _mbrIndex)
    {
        //Remember last quality level
        _mbrIndex = _nextMbrIndex;

        //Get fourcc code for this quality level
        _fourcc = _pMSD->GetFourCC(_mbrIndex);

        if (_mediaType == MediaStreamTypeVideo)
        {
            _pMbrSocket->QualityChanged(this);
        }
    }

    _mustRequestNextChunk = true;

    return isChunkHdrRecv;
}

bool CSocketMbrChunk::RetryChunk(_Inout_ CSocketMbrChunkRetry* retryInfo)
{
    // retry when there is an http invalid (<200 or >=400) or chunkheader parsing error
    if (( eSocketErrorSSChunkHdrParsingFailed != _socketError ) && (eSocketErrorHttpInvalidResult != _socketError) )
    {
        return false;
    }

    // no retries in trickmode on 412s and live
    if( _pTuneInfo->IsInTrickMode() &&
        _pMbrManifest->IsLive() &&
        eSocketErrorHttpInvalidResult == _socketError &&
        HTTP_STATUS_PRECOND_FAILED == _pChunkSocket->GetHttpResponse() )
    {
        SOCKETMBR_TRACE(("SID %d: not retrying 412s in trick mode live, chunk:%d for media type %d",
                _streamId, _mediaPos.ChunkIndex(), _mediaType));
        return false;
    }

    CSocketMbrRetry::RetryAction retryAction = retryInfo->retryState.GetRetryAction( _socketError, _pChunkSocket->GetHttpResponse() );
    if( CSocketMbrRetry::NO_RETRY == retryAction )
    {
        return false;
    }

    if( CSocketMbrRetry::RETRY_SAME_IDX_SAME_BR == retryAction )
    {
        uint32 msWaited = 0;
        static const uint32 MS_WAIT_INTERVAL = 50;
        static const uint32 MS_TIME_TO_WAIT = 250;

        // Wait before retrying the same chunk, same BR
        while (msWaited < MS_TIME_TO_WAIT)
        {
            if (_socketConnected == false)
            {
                TRACE_ERROR(("SID %d:Exiting waiting for to retry chunk:%d, socket closed.",
                                _streamId,
                                _mediaPos.ChunkIndex() ));
                return false;
            }

            Executive_Sleep(MS_WAIT_INTERVAL);
            msWaited += MS_WAIT_INTERVAL;
        }


        _measuringRate = false;

        // take note of the delay during non-video downloads as this could affect
        // the rate measurement for video
        if (_mediaType != MediaStreamTypeVideo)
        {
            _pMbrSocket->SetNonVideoIsDelayed(true);
        }

        SOCKETMBR_TRACE(("SID %d: retry same chunk:%d same BR:%d for media type %d",
                            _streamId,
                            _mediaPos.ChunkIndex(),
                            _nextMbrIndex,
                            _mediaType
                            ));
    }
    else if ( CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR == retryAction )
    {
        if (!retryInfo->triedHigherBitRate && _pHeuristics->ForceQualityNextChunk(_streamId, _nextMbrIndex, eQualityDirection_Down)) 
        {
            // try the next lower quality level
            retryInfo->triedSameIndexDifferentBitRateCount++;
        }
        else if(0 == retryInfo->triedSameIndexDifferentBitRateCount 
            && _pHeuristics->ForceQualityNextChunk(_streamId, _nextMbrIndex, eQualityDirection_Up))
        {
            // on first retry and can't try lower quality level, then try next higher quality level
            retryInfo->triedHigherBitRate = true;
            retryInfo->triedSameIndexDifferentBitRateCount++;
        }
        else
        {
            // no bitrates available, move to next chunk
            retryAction = CSocketMbrRetry::RETRY_NEXT_IDX;
            retryInfo->retryState.UpdateCounters( _socketError );
        }
    }

    if ( CSocketMbrRetry::RETRY_NEXT_IDX == retryAction )
    {
        if( !_pTuneInfo->MoveChunkIndex( &_mediaPos, _pMSD ) )
        {
            SOCKETMBR_TRACE(("SID %d: no retry no more chunks:%d for media type %d, first chunk:%d, last chunk:%d",
                             _streamId,
                             _mediaPos.ChunkIndex(),
                             _mediaType,
                             _pMSD->GetDVRMinChunkIndex(),
                             _pMSD->GetDVRMaxChunkIndex()));
            return false;
        }

        //Discontinuity - signal reset once we find good chunk
        _signalDiscontinuity = true;

        // reset retry info since retry will happen on a different chunk
        retryInfo->triedHigherBitRate = false;
        retryInfo->triedSameIndexDifferentBitRateCount = 0;

        SOCKETMBR_TRACE(("SID %d: retry next chunk:%d for media type %d, first chunk:%d, last chunk:%d",
                         _streamId,
                         _mediaPos.ChunkIndex(),
                         _mediaType,
                         _pMSD->GetDVRMinChunkIndex(),
                         _pMSD->GetDVRMaxChunkIndex()));
    }

    return true;
}

pkRESULT CSocketMbrChunk::RecvNextChunk()
{
    //Update Heuristics
    UpdateHeuristics();

    pkRESULT pkResult = pkS_OK;
    CSocketMbrChunkRetry retryInfo;
    _signalDiscontinuity = false;

    //Leave asap when disconnected
    while (_socketConnected)
    {
        while( _mustRequestNextChunk )
        {
            pkResult = SendNextChunkRequest();

            if( pkE_NOT_READY == pkResult )
            {
                // Not ready to send the request because there's not enough room
                // in the FIFO to receive the data. Stall for a while and try
                // again.

                Executive_Sleep( RETRY_INTERVAL_TO_SEND_CHUNK_REQUEST_WHEN_FIFO_IS_TOO_FULL_MS );

                // Turn measurements off to take the previous sleep time out of the
                // bandwidth computation.

                _measuringRate = false;
                        
                // take note of the delay during non-video download as this could affect
                // the rate measurement for video
                if (_mediaType != MediaStreamTypeVideo)
                {
                    _pMbrSocket->SetNonVideoIsDelayed(true);
                }

                continue;
            }

            if( pkE_BEFORE_VALID_RANGE == pkResult )
            {
                string info = "status=outsidewindowedge";
                _pMbrSocket->FireNotification(kReceiverNotificationType_MediaTransportEvent, (CSocketMbr::NOTIFICATION_DATA_VARIANT_TYPE)info.c_str());
            }

            break;
        }

        _mustRequestNextChunk = false;

        if (pkResult == pkS_OK)
        {
            _chunkIndexInResponse = _mediaPos.ChunkIndex();

            ReportEvent_StartChunkResponseHeader( this, _streamId, _chunkIndexInResponse );

#ifdef TV2INTERNAL
            _tickStartChunkResponseHeader = Executive_GetTickCount();
#endif

            string response;
            if (_pChunkSocket->RecvHttpResponse(&response))
            {
                if (_signalDiscontinuity)
                {
                    _pMbrSocket->FireNotification(kReceiverNotificationType_Discontinuity, (CSocketMbr::NOTIFICATION_DATA_VARIANT_TYPE)_mediaType);
                    _pMbrSocket->FireNotification(kReceiverNotificationType_Reset);
                    _signalDiscontinuity = false;
                }

                //Look for session id in the server response and store it if it exists
                _pMbrManifest->StoreSessionId(_pChunkSocket);

                if( _pMSD->HasChildStreams() )
                {
                    RecvChildChunkHeader();
                }

                //Parse the chunk header
                if( !RecvChunkHeader(_pMSD->GetChunkStartPosition(_mediaPos.ChunkIndex())) )
                {
                    TRACE_ERROR(("ChunkHeader '%ls' failed", _wstrChunkURL.c_str()));
                    pkResult = pkS_FALSE;
                }
            }
            else
            {
                TRACE_ERROR(("GET '%ls' failed", _wstrChunkURL.c_str()));
                //inform that there was a http error
                if(eSocketErrorHttpInvalidResult  == _pChunkSocket->GetSocketError())
                {
                    string info = "status=nextchunkhttpinvalid&httpresponse=" + toString(_pChunkSocket->GetHttpResponse()) +
                                  "&pkresult=" + toString(_pChunkSocket->GetPKResult()) +
                                  "&url=" + WStr2Str(_wstrChunkURL);

                    _pMbrSocket->FireNotification(kReceiverNotificationType_MediaTransportEvent, (CSocketMbr::NOTIFICATION_DATA_VARIANT_TYPE)info.c_str());
                }
                    
                _socketError = _pChunkSocket->GetSocketError();
                pkResult = pkS_FALSE;
            }

            // Receiving the response or parsing http header failed
            if ( pkS_FALSE == pkResult )
            {
                //Do we want to retry?
                if ( RetryChunk(&retryInfo) )
                {
                    ++_pMbrSocket->_diags.RetryCount;

                    _mustRequestNextChunk = true;

                    continue;
                }
                else
                {
                    //Save the error state and make it more specific
                    if(eSocketErrorHttpInvalidResult  == _socketError)
                    {
                        _socketError = eSocketErrorSSNextChunkHttpInvalid;
                    }

                    _socketPKResult = _pChunkSocket->GetPKResult();
                    _socketHttpResponse = _pChunkSocket->GetHttpResponse();

                    TRACE_ERROR(("Failed RecvNextChunk '%ls' err: %d pkError: 0x%08x http response: %d",
                        _wstrChunkURL.c_str(),
                        _socketError,
                        _socketPKResult,
                        _socketHttpResponse));
                }
            }
        }

        break;
    }

    if ( pkS_OK == pkResult )
    {
        // clear any previous error since the receive was successful
        _socketError = eSocketErrorNone;
        _socketPKResult = pkS_OK;
        _socketHttpResponse = 0;
    }
    return pkResult;
}

pkRESULT CSocketMbrChunk::SendNextChunkRequest()
{
    if (!_pMSD)
    {
        SOCKETMBR_TRACE(("CSocketMbrChunk::SendNextChunkRequest(): no Media Stream Descriptor context!"));
        return pkE_UNEXPECTED;
    }

    if (!_socketConnected)
    {
        SOCKETMBR_TRACE(("CSocketMbrChunk::SendNextChunkRequest(): socket has been disconnected"));
        return pkE_SOCKET_SHUTDOWN;
    }

    // check if left edge has over taken the chunk index
    if (_pMbrManifest->IsLive()
        && !_pTuneInfo->IsInTrickMode()
        && !_pMSD->ContainsChunkIndex( _mediaPos.ChunkIndex() )
        && _mediaPos.ChunkIndex() < _pMSD->GetDVRMinChunkIndex())
    {
        SOCKETMBR_TRACE(("CSocketMbrChunk::SendNextChunkRequest() LIVE SID %d: chunkIndex() = %d, chunkList %d-%d, EXIT",
            _pMSD->GetStreamID(),
            _mediaPos.ChunkIndex(),
            _pMSD->GetDVRMinChunkIndex(),
            _pMSD->GetDVRMaxChunkIndex()));

        return pkE_BEFORE_VALID_RANGE;
    }

    // check if chunk index is within the chunklist or there is nothing else to download
    if( !_pMSD->ContainsChunkIndex( _mediaPos.ChunkIndex() ) ||
        ( _pMbrManifest->IsLive() && 0 == _fragmentsAhead ))
    {
        SOCKETMBR_TRACE(("CSocketMbrChunk::SendNextChunkRequest() %s SID %d: chunkIndex() = %d, chunkList %d-%d, _fragmentsAhead = %d, EXIT",
            _pMbrManifest->IsLive() ? "LIVE" : "VOD",
            _pMSD->GetStreamID(),
            _mediaPos.ChunkIndex(),
            _pMSD->GetDVRMinChunkIndex(),
            _pMSD->GetDVRMaxChunkIndex(),
            _fragmentsAhead));

        return pkE_NO_MORE_ITEMS;
    }

    // check if it is too soon to download this chunk
    TimeSpan_hns timeDelta;
    if (_pMbrManifest->IsLive()
        && !_pTuneInfo->IsInTrickMode()
        && !_pMbrManifest->CanRequestFragment(_pMSD, _mediaPos.ChunkIndex(), &timeDelta))
    {
        SOCKETMBR_TRACE(("CSocketMbrChunk::SendNextChunkRequest() SID %d: can't request chunkIdx: %d chunkList %d-%d yet, %lld ms",
            _pMSD->GetStreamID(),
            _mediaPos.ChunkIndex(),
            _pMSD->GetDVRMinChunkIndex(),
            _pMSD->GetDVRMaxChunkIndex(),
            TimeSpan_ms::ConvertFrom(timeDelta).Ticks()
            ));

        return pkE_NOT_READY;
    }

    if( _mediaType == MediaStreamTypeVideo )
    {
        DecoderBufferStatus decoderBufferStatus;

        _pMbrSocket->GetDecoderBufferStatus( &decoderBufferStatus );

        if( decoderBufferStatus.clockIsRunning
            && ( decoderBufferStatus.videoBufferLevel90kHz > decoderBufferStatus.minVideoBufferLevelToTakeFragment90kHz )
            && _pMbrSocket->CanWaitForFragment() )
        {
            SOCKETMBR_TRACE(("CSocketMbrChunk::SendNextChunkRequest() SID %d: can't request video fragment yet ( %u > %u )",
                _pMSD->GetStreamID(),
                decoderBufferStatus.videoBufferLevel90kHz,
                decoderBufferStatus.minVideoBufferLevelToTakeFragment90kHz
                ));

            // Not ready to send the request for the next video fragment because there's not enough
            // headroom in the video buffer to allow a bandwidth measurement. Also, the receiver
            // can wait for packets, so return an error to tell the caller it can wait and
            // retry again.

            return pkE_NOT_READY;
        }
    }

    if( !PrepareChunkUrl() )
    {
        return pkS_FALSE;
    }

    CTuneRequest tuneRequest;
    tuneRequest.ParseUrl(_wstrChunkURL);

    string extraHttpHeader;
    AddBwInfoHeader(&extraHttpHeader);

    ReportEvent_StartChunkRequest(
            this,
            _streamId,
            _mediaPos.ChunkIndex(),
            _nextMbrIndex,
            _pMSD->TrackWeakPtr( _nextMbrIndex )->Bitrate(),
            _pMSD->GetChunkSizeInKB( _nextMbrIndex, _mediaPos.ChunkIndex() ),
            _wstrChunkURL.c_str() );

#ifdef TV2INTERNAL
    _tickStartChunkRequest = Executive_GetTickCount();
#endif

    if( !_measuringRate )
    {
        // Measurements were turned off at some point of the previous
        // request when the download had to be throttled. Since this
        // is a new request it's OK to turn the measurements on again.

        _rateControl.ResetCount();
        _measuringRate = true;
    }
    
    // starting a new video request, it should disregard that there
    // was a delay previously in non-video downloads
    if (_mediaType == MediaStreamTypeVideo)
    {
        _pMbrSocket->SetNonVideoIsDelayed(false);
    }

    //Send the request

    if (!_pChunkSocket->SendHttpRequest(tuneRequest, extraHttpHeader) )
    {
        SOCKETMBR_TRACE(("SID %d: GET failed for chunkIx=%d",
            _pMSD->GetStreamID(),
            _mediaPos.ChunkIndex()
            ));

        return pkS_FALSE;
    }

    //Some sanity check
    if (_mediaPos.ChunkIndex() != (int32) _pMSD->GetDVRMaxChunkIndex())
    {
        //Only the duration of last chunk can be 0.
        ASSERT(_pMSD->GetChunkDuration(_mediaPos.ChunkIndex()) != 0);
    }

    return pkS_OK;
}


void CSocketMbrChunk::UpdateHeuristics()
{
    //Only video buffer is used for heuristics
    if (_mediaType == MediaStreamTypeVideo)
    {
        DecoderBufferStatus decoderBufferStatus;

        _pMbrSocket->GetDecoderBufferStatus( &decoderBufferStatus );

        uint32 reportBandwidth = _rateControl.GetBitrate();

#ifdef TV2INTERNAL
        if (_diagsFakeSSBitrate)
        {
            SOCKETMBR_TRACE(("CSocketMbrChunk::UpdateHeuristics, fake bps:%d, measured bps:%d", _diagsFakeSSBitrate, reportBandwidth));
            reportBandwidth = _diagsFakeSSBitrate;
        }
#endif

        _pHeuristics->UpdateBufferStatus(decoderBufferStatus);

        // only add bandwidth if there is valid data
        if ( INVALID_UINT32 != reportBandwidth )
        {
            _pHeuristics->AddBandwidth(reportBandwidth);
        }

        ReportEvent_UpdateHeuristics(
                            this,
                            _streamId,
                            _chunkIndexInResponse,
                            (INVALID_UINT32 == reportBandwidth) ? 0 : reportBandwidth,
                            _pHeuristics->GetAverageBandwidth(),
                            decoderBufferStatus.videoBufferLevel90kHz,
                            decoderBufferStatus.audioBufferLevel90kHz );

    }
}

void CSocketMbrChunk::AddBwInfoHeader(_Inout_ std::string *pHeaderString)
{
    if (_mediaType == MediaStreamTypeVideo)
    {
        AutoLock lock(&gSocketMbrConfiguration.valueLock);

        if (!gSocketMbrConfiguration.SSBandwidthReportHeader.empty())
        {
            char szHeader[c_szHeader_maxsize];

            DecoderBufferStatus decoderBufferStatus;

            _pMbrSocket->GetDecoderBufferStatus( &decoderBufferStatus );

            pkRESULT pkr = StringCchPrintfA(szHeader, c_szHeader_maxsize,
                "%s: %u,%d,%u,%u,%u,%u,%u\r\n",
                gSocketMbrConfiguration.SSBandwidthReportHeader.c_str(),
                _streamId,
                _chunkIndexInResponse,
                _pHeuristics->GetLatestBandwidth() / 1000, // kbps
                _pHeuristics->GetAverageBandwidth() / 1000, // kbps
                _pHeuristics->GetAverageBandwidthCount(),
                decoderBufferStatus.videoBufferLevel90kHz / 90, // ms
                decoderBufferStatus.audioBufferLevel90kHz / 90 ); // ms

            if (pkFAILED(pkr))
            {
                TRACE_ERROR(("CSocketMbrChunk::AddBwInfoHeader error: 0x%x for header string: %s",
                    pkr,
                    gSocketMbrConfiguration.SSBandwidthReportHeader.c_str()));
            }
            else
            {
                *pHeaderString += szHeader;

                SOCKETMBR_TRACE_EXTRA(("AddBwInfoHeader %s", szHeader));
            }
        }
    }
}


uint32 CSocketMbrChunk::GetCurrentBitrate()
{
    return _pMSD->TrackWeakPtr( (_mbrIndex != INVALID_UINT32) ? _mbrIndex : 0 )->Bitrate();
}

uint32 CSocketMbrChunk::GetCurrentQualityLevel()
{
    return (_mbrIndex != INVALID_UINT32) ? _mbrIndex : 0;
}

void CSocketMbrChunk::SetSignedKeyID(const byte* pKeyID, uint32 len)
{
    ASSERT(pKeyID && (len > 0));

    //Has signed key ID changed?
    if (_signedKeyID)
    {
        if ((len == _signedKeyLen) && (memcmp(pKeyID, _signedKeyID, len) == 0))
            return;

        //Delete previous key ID
        delete [] _signedKeyID;
        _signedKeyID = NULL;
        _signedKeyLen = 0;
    }

    _signedKeyID = new byte[len];
    if (!_signedKeyID)
    {
        //Out of memory
        ASSERT(false);
        return;
    }

    memcpy_s(_signedKeyID, len, pKeyID, len);
    _signedKeyLen = len;
    _signedKeyChanged = true;
}

void CSocketMbrChunk::SetSubSample(int entries, const uint16* clear, const uint32* encrypted)
{
    if (!entries)
        return;

    //Do we need to increase size of sub sample arrays?
    if (entries > _subSampleAllocSize)
    {
        FreeSubSampleArrays();

        _subSampleClearBytes = new uint16[entries];
        _subSampleEncryptedBytes = new uint32[entries];

        if (!_subSampleClearBytes || !_subSampleEncryptedBytes)
        {
            ASSERT(false);
            FreeSubSampleArrays();
            return;
        }

        _subSampleAllocSize = entries;
    }

    //Copy data to local arrays
    memcpy_s(_subSampleClearBytes, entries * sizeof(uint16), clear, entries * sizeof(uint16));
    memcpy_s(_subSampleEncryptedBytes, entries * sizeof(uint32), encrypted, entries * sizeof(uint32));
}

void CSocketMbrChunk::FreeSubSampleArrays()
{
    if (_subSampleClearBytes)
    {
        delete [] _subSampleClearBytes;
        _subSampleClearBytes = NULL;
    }

    if (_subSampleEncryptedBytes)
    {
        delete [] _subSampleEncryptedBytes;
        _subSampleEncryptedBytes = NULL;
    }

    _subSampleAllocSize = 0;
}

bool CSocketMbrChunk::Command(const string& command, const vector<string>& args)
{
#ifdef TV2INTERNAL
    //Forces specific quality level
    if (command == "forcequalitylevel")
    {
        const string& ql = args[0];

         if (ql.empty())
             return true;

        _diagsForceQualitySwitch = true;
        _diagsNewQualityLevel = atoi(ql.c_str());
        TRACE(("CSocketMbrChunk[%08x]: Quality Level set to %d", _socketPipeIdN, _diagsNewQualityLevel));
        return true;
    }

    //To stop faking, set this parameter to 0 from x-ray.
    if (command == "fakessbps")
    {
        const string& ssbps = args[0];

        if (ssbps.empty())
            return true;

        _diagsFakeSSBitrate = atoi(ssbps.c_str());
        TRACE(("CSocketMbrChunk[%08x]: Fake smooth streaming bitrate to %d", _socketPipeIdN, _diagsFakeSSBitrate));
        return true;
    }
#endif

    if (_pChunkSocket)
    {
        return _pChunkSocket->Command(command, args);
    }

    return false;
}

// ===============================================================================================================
// Multi-Bitrate Socket - This socket class handles adaptive streams
// ===============================================================================================================

CSocketMbr::CSocketMbr(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, uint32 initialNetworkBitsPerSec, eSocketType socketType/* = eSocketType_Mbr*/)
    : CSocketRaw(avManager, pipeIdN, tuneRequest, socketType)
    , _pMbrManifest(NULL)
    , _apChunkManifest(NULL)
    , _pHeuristics(NULL)
    , _lastTime(0)
    , _started(false)
    , _isClosed(false)
    , _isNonVideoDelayed(false)
    , _initialNetworkBitsPerSec(initialNetworkBitsPerSec)
{
    avManager->GetManifestUpdateManager()->SetUpdateWorker(this);
}

CSocketMbr::~CSocketMbr()
{
    _pSocketAVManager->GetManifestUpdateManager()->SetUpdateWorker(NULL);

    //The data sockets do not have a streaming thread, so they must be cleaned up on our thread
    for (uint32 i = 0; i < _streams.size(); ++i)
    {
        //Make sure the socket is disconnected
        if (_streams[i]->IsConnected())
        {
            _streams[i]->Close();
        }

        //Dump it
        delete _streams[i];
    }
    _streams.clear();

    if (_pHeuristics)
    {
        delete _pHeuristics;
        _pHeuristics = NULL;
    }
}

int CSocketMbr::Recv(CRawPacket& rawPacket)
{
    int frameSize = 0;
    if (_socketConnected)
    {
        AutoLock lock(&_socketLock);

        long ready = 0;
        if (_streams.size())
        {
            //Samples are interleaved across all streams by decoder timestamp
            uint64 earliest = _tuneInfo._speed > 0 ? -1 : 0;
            for (uint32 i = 0; i < _streams.size(); ++i)
            {
                uint64 next = _streams[i]->NextTimestamp();
                if (((_tuneInfo._speed > 0) && (next < earliest)) ||
                    ((_tuneInfo._speed < 0) && (next > earliest)))
                {
                    earliest = next;
                    ready = i;
                }
            }

            _lastTime = _streams[ready]->NextTimestamp();
            frameSize = _streams[ready]->Recv(rawPacket);
            if (frameSize < 0)
            {
                //Recv error - copy error codes from chunk socket and shut down update thread
                _socketError = _streams[ready]->GetSocketError();
                _socketPKResult = _streams[ready]->GetPKResult();
                _socketHttpResponse = _streams[ready]->GetHttpResponse();
                _pMbrManifest->StopChunkUpdateThread();
            }

            if (!_started)
            {
                _started = true;
            }

            if (frameSize > 0)
            {
                _pMbrManifest->ReportStartEndTime();
            }
        }
    }

    if (frameSize == 0)
    {
        frameSize = eRecvErrorEOF;
    }

    return frameSize;
}

bool CSocketMbr::ConnectStream(CMediaStreamDescription* pMSD, uint64 hnsPos, bool isTuning)
{
    // don't try to connect a stream if the socket has already been closed
    if (_isClosed)
    {
        return false;
    }

    //Create chunk socket
    CSocketMbrChunk* chunkSocket = new CSocketMbrChunk(_pSocketAVManager, _socketPipeIdN, _socketTuneRequest);
    if (chunkSocket == NULL)
        return false;

    //Save the streamer
    _streams.push_back(chunkSocket);

    //Init chunk socket
    chunkSocket->Init(this, pMSD, hnsPos, isTuning);

    if (pMSD->Type() == MediaStreamTypeVideo)
    {
        CMediaStreamDescription::Range rg = pMSD->GetBitrateRangeOfSelectedTracks();

        _diags.TotalQualityLevel = pMSD->GetIndexOfLastSelectedTrack() + 1;
        _diags.MinVideoBps = rg._min;
        _diags.MaxVideoBps = rg._max;
    }
    return true;
}

void CSocketMbr::CloseStream(uint32 streamId)
{
    CSocketMbrChunkVector::iterator iter;
    for (iter = _streams.begin(); iter != _streams.end(); ++iter)
    {
        if ((*iter)->GetStreamId() == streamId)
        {
            (*iter)->Close();
            delete (*iter);

            _streams.erase(iter);
            break;
        }
    }
}

bool CSocketMbr::IsStreamConnected(uint32 streamId)
{
    for (uint32 i = 0; i < _streams.size(); ++i)
    {
        if (_streams[i]->GetStreamId() == streamId)
            return true;
    }
    return false;
}

bool CSocketMbr::Prepare(void)
{
    uint32 handle = (uint32)_socketTuneRequest.GetInt(TUNE_REQUEST_TUNEPREPAREHANDLE);
    if (handle)
    {
        //Get manifest socket pointer from tune request
        _pMbrManifest = (CMbrManifest*)_pSocketAVManager->GetTunePrepareFactory()->GetTunePrepareInterface(handle);
        ASSERT(_pMbrManifest);

        //Fail the tune if can't find the manifest socket object
        if (_pMbrManifest == NULL)
            return false;
    }
    else
    {
        //Create our own manifest socket
        handle = _pSocketAVManager->GetTunePrepareFactory()->CreateTunePrepareObject(_socketPipeIdN, _socketTuneRequest);
        _pMbrManifest = (CMbrManifest*)_pSocketAVManager->GetTunePrepareFactory()->GetTunePrepareInterface(handle, false);
        ASSERT(_pMbrManifest);

        //Fail the tune if can't find the manifest socket object
        if (_pMbrManifest == NULL)
            return false;

        //Download and prepare the manifest
        _pMbrManifest->Prepare();
    }

    //Get chunk manifest
    _apChunkManifest.Set(_pMbrManifest->GetChunkManifest());
    if (_apChunkManifest == NULL)
    {
        //Pass along the socket errors.
        _socketError = _pMbrManifest->GetSocketError();
        _socketPKResult = _pMbrManifest->GetPKResult();
        _socketHttpResponse = _pMbrManifest->GetHttpResponse();
        return false;
    }

    return true;
}

bool CSocketMbr::Connect(void)
{
    AutoLock socketLock(&_socketLock);

    //Quit if the socket has already been closed
    if (!CSocketRaw::Connect())
        return false;

    //Prepare the manifest if one not available already
    if (!Prepare())
        return false;

    //Store tune info
    _tuneInfo.Init(_socketTuneRequest, _apChunkManifest->m_hnsMarkIn );

    //Socket allows retrying of packets in receivers
    _socketRetryAllowed = true;
    _socketResetOnStall = false;
    //For live, use "BufferTime" parameter from manifest
    //otherwise, use minimum buffer delay
    _socketBufferDelay = _pMbrManifest->IsLive() ? _apChunkManifest->m_dwBufferTime : 0;
    //Update the clock mode
    if (_pMbrManifest->IsLive())
    {
        _socketClockMode = 
            eClockMode_UseBufferDelay |
            eClockMode_TimeLimitFifo |
            eClockMode_RebufferWhenUnderrun |
            eClockMode_SyncOnUnderrun |
            eClockMode_TurnFastStartOffWhenSyncing;
    }
    else
    {
        _socketClockMode = 
            eClockMode_UseBufferDelay |
            //eClockMode_TimeLimitFifo |
            //eClockMode_RebufferWhenUnderrun |
            //eClockMode_SyncOnUnderrun |
            eClockMode_TurnFastStartOffWhenSyncing;
    }
	//delete eClockMode_TimeLimitFifo for fix seek long will case play failed, beacause stc is smaller than current pts

    SOCKETMBR_TRACE(("CSocketMbr::Connect - start time: hns %lld, ntp %llu, speed:%d",
        _tuneInfo._hnsStartPos, _socketTuneRequest.Rap, _tuneInfo._speed));

    //Instantiate heuristics module
    _pHeuristics = new CHeuristicsMBR(_apChunkManifest);
    if (!_pHeuristics)
        return false;

    //Initialize heuristics
    _pHeuristics->Init(_tuneInfo.IsInTrickMode(), gSocketMbrConfiguration.AVCpuLimit);

    //Send a diags event with information about the stream
    SendDiagsEvent(new CDiagsSSSocketInfo(
            _apChunkManifest->GetStreamCount(),
            gSocketMbrConfiguration.AVCpuLimit,
            _apChunkManifest->m_nTotalBps,
            (DWORD)_apChunkManifest->DVRWindowLength(),
            _pMbrManifest->IsLive()));

    //Connect to primary stream
    CMediaStreamDescription* pMSD = _apChunkManifest->GetPrimaryStreamDescription();
    if (pMSD)
    {
        if (_pMbrManifest->IsLive())
        {
            //Default to enabling chunk update on live urls
            _pMbrManifest->EnableChunkUpdateThread(true);

            TimeSpan_hns hnsLivePos = TimeSpan_hns::FromTicks(pMSD->GetChunkStartPosition( pMSD->GetDVRMaxChunkIndex() ));
            TimeSpan_hns hnsLiveDelay = TimeSpan_hns::ConvertFrom( TimeSpan_s::FromTicks(gSocketMbrConfiguration.SSLiveBackOffSeconds + gSocketMbrConfiguration.SSLivePlaybackOffsetSeconds) );
            hnsLivePos = (hnsLivePos > hnsLiveDelay) ? hnsLivePos - hnsLiveDelay : TimeSpan_hns::FromTicks(0);

            if (_socketTuneRequest.TuneToLive || (_tuneInfo._hnsStartPos > hnsLivePos.Ticks()))
            {
                CMediaPosition pos;

                if (pMSD->FindPositionByTime(hnsLivePos.Ticks(), true, &pos) == pkS_OK)
                {
                    hnsLivePos = TimeSpan_hns::FromTicks(pMSD->GetChunkStartPosition( pos.ChunkIndex() ));
                }
                
                //Disable chunk update if playing at live
                _pMbrManifest->EnableChunkUpdateThread(false);
                
                _tuneInfo._hnsStartPos = hnsLivePos.Ticks();
                SOCKETMBR_TRACE(("CSocketMbr::Connect - LIVE start index:%d(%d) pos:%lld delay:%lld",
                    pos.ChunkIndex(), pMSD->GetDVRMaxChunkIndex(), hnsLivePos.Ticks(), hnsLiveDelay.Ticks()));
            }

            // Figure out the earliest start position to figure out if the starting playback time needs to be snapped.
            // Default the start position to the left edge
            TimeSpan_hns hnsEarliestStartPos = TimeSpan_hns::FromTicks(pMSD->GetChunkStartPosition( pMSD->GetDVRMinChunkIndex() ));
            TimeSpan_hns hnsLeftEdgeBuffer = TimeSpan_hns::ConvertFrom( TimeSpan_s::FromTicks(gSocketMbrConfiguration.SSLeftEdgeBufferSeconds) );
            TimeSpan_hns hnsTimeLeftInDVR = pMSD->TimeRemainingInDVR();
            
            // see if the left edge will be moving soon
            if (hnsTimeLeftInDVR < hnsLeftEdgeBuffer)
            {
                // add breathing room so that left edge doesn't overtake the download position
                hnsEarliestStartPos = hnsEarliestStartPos + hnsLeftEdgeBuffer - hnsTimeLeftInDVR;
            }

            // make sure the earliest start position is not past the live position
            if (hnsEarliestStartPos > hnsLivePos)
            {
                hnsEarliestStartPos = hnsLivePos;
            }

            // check if the start position needs to be snapped forward
            if ( _tuneInfo._hnsStartPos < hnsEarliestStartPos.Ticks() )
            {
                CMediaPosition pos;

                if (pMSD->FindPositionByTime(hnsEarliestStartPos.Ticks(), true, &pos) == pkS_OK)
                {
                    hnsEarliestStartPos = TimeSpan_hns::FromTicks(pMSD->GetChunkStartPosition( pos.ChunkIndex()));
                }

                _tuneInfo._hnsStartPos = hnsEarliestStartPos.Ticks();
                SOCKETMBR_TRACE(("CSocketMbr::Connect - LEFT start index:%d(%d) pos:%lld buffer:%lld",
                    pos.ChunkIndex(), pMSD->GetDVRMinChunkIndex(), hnsEarliestStartPos.Ticks(), hnsLiveDelay.Ticks()));
            }

        }

        //Find out video's max bitrate
        _tuneInfo._maxBandwidth = pMSD->GetNominalBitrate( pMSD->GetIndexOfLastSelectedTrack() );

        //Just connect video stream at this point
        //Audio/text is connected depending on user's selection
        if (!ConnectStream(pMSD, _tuneInfo._hnsStartPos, true))
            return false;
    }

    //Set time base where we are starting the streaming from
    _lastTime = _tuneInfo._hnsStartPos;

    return true;
}

void CSocketMbr::ReceiversOnConnected(void)
{
    //Handle DRM OPL stuff
    CHECK_ALLOC(_pMbrManifest);

    //Handle DRM specific notifications
    IDrmDecrypter* pDrm = _pMbrManifest->GetDrmObject();
    if (pDrm && (pDrm->GetEncryptionType() == DrmEncryptionType_PlayReady))
    {
        XDRM_OPL_DATA *pOPLData = _pMbrManifest->GetDrmOPLData();
        assert( pOPLData != NULL );

        // The following line of code may introduce race condition when playing back key -rotation (on-demand) content.
        // When the notification is processed, the pDrm associated with the notification may have already been released (happens when a new chunk is received).
        // Temporarily comment out the code for now since it is only used for diagnostics. IIS/SSPK PS bug #26970 is tracking it.
        // FireNotification(kReceiverNotificationType_DrmLicenseTimes, (NOTIFICATION_DATA_VARIANT_TYPE)pDrm);

        if (!XDRM_IS_OPL_DATA_EMPTY(pOPLData))
        {
            // A reference of the OPL data cached in the manifest is sent along with the notification.
            // It is valid because of the fact that the manifest is still alive when the notification is handled.
            FireNotification(kReceiverNotificationType_DrmOutputProtectionLevel, (NOTIFICATION_DATA_VARIANT_TYPE)pOPLData);
        }
    }

    //Send stream info to media transport (should be before connecting chunk sockets to give IReceiver a chance to select a default stream)
    FireNotification(kReceiverNotificationType_StreamDescUpdate, (NOTIFICATION_DATA_VARIANT_TYPE)_pMbrManifest->GetStreamInfoList());

    //Calling the base
    CSocketRaw::ReceiversOnConnected();
}

uint32 CSocketMbr::GetMeasuredNetworkBitsPerSec()
{
    AutoLock lock(&_socketLock);
    return _pHeuristics ? _pHeuristics->GetAverageBandwidth() : _initialNetworkBitsPerSec;
}

bool CSocketMbr::Close(void)
{
    // close streamers first
    {
        AutoLock lock(&_socketLock);

        //Close the active streamers
        for (uint32 i = 0; i < _streams.size(); ++i)
        {
            _streams[i]->Close();
        }

        _isClosed = true;
    }

    // wait for the socket thread to close
    // Note: this must be done outside of the socketlock since the socket thread may try to grab the lock
    CSocketRaw::Close();

    // stop and decrement ref count on CMbrManifest
    if (_pMbrManifest)
    {
        _pMbrManifest->StopChunkUpdateThread();
        _pSocketAVManager->GetTunePrepareFactory()->ReleaseTunePrepare(_pMbrManifest->GetToken());
        _pMbrManifest = NULL;
    }

    return true;
}

void CSocketMbr::QualityChanged(CSocketMbrChunk* socketMbrChunk)
{
    CHECK_ALLOC(socketMbrChunk);
    CHECK_ALLOC(_pHeuristics);

    AutoLock lock(&_socketLock);

    //Collect the current bitrate info
    uint32 total_cur_bitrate = 0;
    for (uint32 i=0; i<_streams.size(); ++i)
    {
        total_cur_bitrate += _streams[i]->GetCurrentBitrate();
    }

    _diags.TotalBps = total_cur_bitrate;
    _diags.CurrentVideoBps = socketMbrChunk->GetCurrentBitrate();;
    _diags.CurrentQualityLevel = socketMbrChunk->GetCurrentQualityLevel();
    _diags.LastQualityChangeTs = socketMbrChunk->NextTimestamp();

    SendDiagsEvent(new CDiagsSSQualityChange(
            socketMbrChunk->GetMediaType(),
            socketMbrChunk->GetStreamId(),
            _diags.CurrentQualityLevel,
            _diags.CurrentVideoBps,
            _pHeuristics->GetThreadCpu(),
            _pHeuristics->GetTotalCpu()));

    SOCKETMBR_TRACE(("CSocketMbr::QualityChanged(), media type:%d, current quality level:%d, total bitrate:%d",
            socketMbrChunk->GetMediaType(),
            _diags.CurrentQualityLevel,
            _diags.TotalBps));

    FireNotification(kReceiverNotificationType_CurrentBitRate, (NOTIFICATION_DATA_VARIANT_TYPE)_diags.CurrentVideoBps);
}

bool CSocketMbr::SetAudioLanguage(int pid)
{
    AutoLock lock(&_socketLock);
    if (!_socketConnected)
        return false;

    if (_tuneInfo.IsInTrickMode())
        return true;

    //Let the new stream start at chunk download preroll time plus current-time.
    //If start at current time, we'll often end up with underrun
    //since it takes time to connect to the server and get the data,
    //meanwhile the clock is still running.
    uint64 hnsPos = GetCurrentPlaybackTime() + MBR_CHUNK_DOWNLOAD_PREROLL * 10000;
    SOCKETMBR_TRACE(("CSocketMbr::SetAudioLanguage(%d), hnsPos:%lld", pid, hnsPos));

    bool bSelectAll = (pid == ISocket::SETLANGUAGE_SELECTALL);
    for (uint32 i = 0; i < _apChunkManifest->m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pMSD = _apChunkManifest->m_availableStreams[i];
        if (pMSD->Type() == MediaStreamTypeAudio)
        {
            int32 streamId = (int)pMSD->GetStreamID();

            if (bSelectAll || streamId == pid)
            {
                if (IsStreamConnected(streamId) == false)
                {
                    ConnectStream(pMSD, hnsPos, !_started);
                }
            }
            else
            {
                CloseStream(streamId);
            }
        }
    }
    return true;
}

bool CSocketMbr::SetSubtitleLanguage(int pid)
{
    AutoLock lock(&_socketLock);
    if (!_socketConnected)
        return false;

    if (_tuneInfo.IsInTrickMode())
        return true;

    //Let the new stream start at 200ms plus current-time.
    //If start at current time, we'll often end up with underrun
    //since it takes time to connect to the server and get the data,
    //meanwhile the clock is still running.
    uint64 hnsPos = GetCurrentPlaybackTime() + MBR_CHUNK_DOWNLOAD_PREROLL * 10000;

    bool bSelectAll = (pid == ISocket::SETLANGUAGE_SELECTALL);
    for (uint32 i = 0; i < _apChunkManifest->m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pMSD = _apChunkManifest->m_availableStreams[i];

        if( !pMSD->IsSelected() )
        {
            continue;
        }

        MediaStreamSubType mediaSubType = pMSD->GetSubType();

        if ((pMSD->Type() == MediaStreamTypeText) &&
            (
            (mediaSubType == MediaStreamSubTypeSubtitles) ||
            (mediaSubType == MediaStreamSubTypeCaptions) ||
            (mediaSubType == MediaStreamSubTypeDescriptions)
            ))
        {
            int32 streamId = (int)pMSD->GetStreamID();

            if (bSelectAll || (streamId == pid))
            {
                if (IsStreamConnected(streamId) == false)
                {
                    ConnectStream(pMSD, hnsPos, !_started);
                }
            }
            else
            {
                CloseStream(streamId);
            }
        }
    }
    return true;
}

uint64 CSocketMbr::GetCurrentPlaybackTime(void)
{
    //Return current stream time in 100ns unit
    uint64 decoderTime = CSocketBase::GetCurrentPlaybackTime();
    if (IS_VALID_TIME(decoderTime))
    {
        //NTP to HNS
        return (NTP_UINT64TO10MHZ(decoderTime));
    }
    else
    {
        return _lastTime;
    }
}

void CSocketMbr::FireNotification(ReceiverNotificationType eType, NOTIFICATION_DATA_VARIANT_TYPE data)
{
    CReceiverNotificationData notificationData;

    switch(eType)
    {
    case kReceiverNotificationType_Discontinuity:
        {
            MediaStreamType mediaType = (MediaStreamType) data;
            uint32 ticks = Executive_GetTickCount();
            bool fireEvent = false;

            switch(mediaType)
            {
            case MediaStreamTypeVideo:
                fireEvent = _discontinuityStats.AddDiscontinuityCount(true, ticks);
                break;
            case MediaStreamTypeAudio:
                fireEvent = _discontinuityStats.AddDiscontinuityCount(false, ticks);
                break;
            default: 
                break;
            }

            if (!fireEvent)
                return;

            notificationData.DiscontinuityStats = &_discontinuityStats;
        }
        break;

    case kReceiverNotificationType_Reset:
        {
            notificationData.Reset.IsIFrameOnlyMode = _tuneInfo.IsInTrickMode();
            notificationData.Reset.Offset = 0;
            notificationData.Reset.Speed = 1;
        }
        break;

    case kReceiverNotificationType_CurrentBitRate:
        {
            notificationData.BitRate.BPS = (uint32)data;
        }
        break;

    case kReceiverNotificationType_StreamDescUpdate:
        {
            ASSERT(data);
            notificationData.StreamDescUpdate.StreamInfo = (CStreamInfoList*)data;
            notificationData.StreamDescUpdate.AudioPID = _apChunkManifest->GetActiveAudioId();
            notificationData.StreamDescUpdate.SubtitlePID = 0;
            notificationData.StreamDescUpdate.InitialSelection = true;
        }
        break;

    case kReceiverNotificationType_DrmLicenseTimes:
        if (data)
        {
            size_t len = sizeof(DrmLicenseTiming);
            IDrmDecrypter *pDrm = (IDrmDecrypter *) data;
            iptv_hal_error err = pDrm->GetProperty(DRM_PROPERTY_LICENSETIMES, (byte*) notificationData.DrmLicense.Timing, &len);
            if (IPTV_HAL_ERROR_RET_FAILED(err))
                return;
        }
        break;

    case kReceiverNotificationType_DrmOutputProtectionLevel:
        {
            XDRM_OPL_DATA *pOPLData = (XDRM_OPL_DATA *)data;
            assert(pOPLData != NULL);

            notificationData.DrmOutputProtectionLevel = *pOPLData;
        }
        break;

    case kReceiverNotificationType_MediaTransportEvent:
        {
            notificationData.MediaTransportEvent.EventStringPtr = (char*)data;
        }
        break;
        
    default: 
        break;
    }

    SendNotification(eType, &notificationData);

    if (eType == kReceiverNotificationType_StreamDescUpdate)
    {
        SetAudioLanguage(notificationData.StreamDescUpdate.AudioPID);
        SetSubtitleLanguage(notificationData.StreamDescUpdate.SubtitlePID);
    }
}

bool CSocketMbr::Command(const string& command, const vector<string>& args)
{
    if (CSocketRaw::Command(command, args))
        return true;

#ifdef TV2INTERNAL
    AutoLock lock(&_socketLock);

    if (command == "sspipeline")
    {
        const string& multiplier = args[0];
        if (!multiplier.empty())
        {
            gSocketMbrConfiguration.SSPipelineMultiplier = atoi(multiplier.c_str());
            TRACE(("CSocketMbrChunk[%08x]: Set http pipeline multiplier to :%d", _socketPipeIdN, gSocketMbrConfiguration.SSPipelineMultiplier));
        }
        return true;
    }

    for (uint32 i = 0; i < _streams.size(); ++i)
    {
        if (_streams[i]->GetStreamId() == _apChunkManifest->GetPrimaryStreamId())
        {
            return _streams[i]->Command(command, args);
        }
    }
#endif
    return false;
}

void CSocketMbr::DiagsReset(void)
{
    //Base class stuff
    CSocketRaw::DiagsReset();

    //Initialize diagnostics
    _diags.Init();
}

void CSocketMbr::DiagsRetrieve(IDiagsEvent* diagsEvent)
{
    //Base class stuff
    CSocketRaw::DiagsRetrieve(diagsEvent);

    //MBR socket related diagnostics
    CDiagsSocketMbrUpdateEvent* diagsSocketMbrEvent = (CDiagsSocketMbrUpdateEvent*)diagsEvent;
    if (diagsSocketMbrEvent)
    {
        //Default copy constructor
        diagsSocketMbrEvent->Stats = _diags;
    }
}

IDiagsEvent* CSocketMbr::DiagsRetrieve(void)
{
    CDiagsSocketMbrUpdateEvent* diagsSocketMbrEvent = new CDiagsSocketMbrUpdateEvent();
    if (diagsSocketMbrEvent)
    {
        DiagsRetrieve(diagsSocketMbrEvent);
    }
    return diagsSocketMbrEvent;
}

pkRESULT CSocketMbr::SelectStream( _In_ IManifestStream* pStream )
{
    AutoLock lock(&_socketLock);

    pkRESULT pkr = pkS_OK;
    CMediaStreamDescription* pMSD = NULL;

    if( _apChunkManifest == NULL )
    {
        // Trying to select a stream before tuned
        pkr = pkE_NOT_READY;
        goto exit;
    }

    pMSD = _apChunkManifest->GetStreamDescriptionByStream( pStream );

    if (NULL == pMSD)
    {
        // Precondition sanity check: the caller is not supposed to call
        // this for streams that don't exist in the manifest
        pkr = pkE_UNEXPECTED;
        ASSERT( FALSE );
        goto exit;
    }

    if (_tuneInfo.IsInTrickMode() && MediaStreamTypeAudio == pStream->Type())
    {
        goto exit;
    }

    SOCKETMBR_TRACE(("CSocketMbr::SelectStream(%d), hnsPos:%lld", pMSD->GetStreamID(), GetCurrentPlaybackTime()));

    if (MediaStreamTypeAudio == pMSD->Type())
    {
        // Notify the receiver the audio language change
        // This will trigger SetAudioLanguage called on the receiver
        CReceiverNotificationData notificationData;
        notificationData.StreamDescUpdate.StreamInfo = _pMbrManifest->GetStreamInfoList();
        notificationData.StreamDescUpdate.AudioPID = pMSD->GetStreamID();
        notificationData.StreamDescUpdate.SubtitlePID = 0;
        notificationData.StreamDescUpdate.InitialSelection = false;

        SendNotification(kReceiverNotificationType_StreamDescUpdate, &notificationData);
    }
    else
    {
        if (!_socketConnected)
        {
            pkr = pkE_NOT_CONNECTED;
            goto exit;
        }

        //Let the new stream start at chunk download preroll time plus current-time.
        //If start at current time, we'll often end up with underrun
        //since it takes time to connect to the server and get the data,
        //meanwhile the clock is still running.
        uint64 hnsPos = GetCurrentPlaybackTime() + MBR_CHUNK_DOWNLOAD_PREROLL * 10000;

        ConnectStream(pMSD, hnsPos, !_started);
        SOCKETMBR_TRACE(("Connect the socket for stream %ls with streamId %d at position %lld", pStream->Name().c_str(), pMSD->GetStreamID(), hnsPos));
    }

exit:

    return( pkr );
}

pkRESULT CSocketMbr::DeselectStream( _In_ IManifestStream* pStream )
{
    AutoLock lock(&_socketLock);

    pkRESULT pkr = pkS_OK;
    CMediaStreamDescription* pMSD = NULL;

    if (!_socketConnected)
    {
        pkr = pkE_NOT_CONNECTED;
        goto exit;
    }

    if( _apChunkManifest == NULL )
    {
        // Trying to deselect a stream before tuned
        pkr = pkE_NOT_READY;
        goto exit;
    }

    pMSD = _apChunkManifest->GetStreamDescriptionByStream(pStream);

    if (NULL == pMSD)
    {
        // Precondition sanity check: the caller is not supposed to call
        // this for streams that don't exist in the manifest
        pkr = pkE_UNEXPECTED;
        ASSERT( FALSE );
        goto exit;
    }

    if ( MediaStreamTypeAudio != pMSD->Type() )
    {
        CloseStream(pMSD->GetStreamID());
        SOCKETMBR_TRACE(("Disconnect the socket for stream %ls with streamId %d at position %lld", pStream->Name().c_str(), pMSD->GetStreamID(), GetCurrentPlaybackTime()));
    }

exit:
    return( pkr );
}

bool CSocketMbr::GetNonVideoIsDelayed()
{
    return _isNonVideoDelayed;
}

void CSocketMbr::SetNonVideoIsDelayed(bool isDelay)
{
    _isNonVideoDelayed = isDelay;
}

// ===============================================================================================================
// ===============================================================================================================
