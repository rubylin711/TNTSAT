///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/*
 * CSocketMbrManifest.h
 *
 * This file describes the functionality of the CSocketMbrManifest
 */
#pragma once

#include "CEvent.h"
#include "CStreamInfo.h"
#include "CTuneRequest.h"
#include "IAVManager.h"
#include "IDrmManager.h"
#include "IStreamerHttp.h"
#include "ITunePrepare.h"
#include "CManifestChunk.h"
#include "CChunkManifest.h"
#include "CSocketMbrRetry.h"
#include "MP4Parser.h"
#include "Thread.h"

using namespace MBR;

// ===============================================================================================================
// ===============================================================================================================

class CChunkInfoReader
{
public:
    CChunkInfoReader(IAVManager* avManager, uint32 pipeIdN);
    ~CChunkInfoReader();

    FMP4TrackInfo* GetChunkInfo(_In_ std::wstring& wstrChunkURL, _In_ int timeout);

    bool GetResponseHeader( _In_ const std::string& header, _Out_ std::string* pstrHeader );
    eSocketError GetSocketError(void) const { return _chunkSocket ?  _chunkSocket->GetSocketError() : eSocketErrorNone; }
    int GetHttpResponse(void) const { return (_chunkSocket && (_chunkSocket->GetSocketError() == eSocketErrorHttpInvalidResult)) ? _chunkSocket->GetHttpResponse() : 0; }
    pkRESULT GetPKResult(void) const { return _chunkSocket ? _chunkSocket->GetPKResult() : pkS_OK; }

private:
    bool Connect(CTuneRequest& tuneRequest);

    // Factory of factories
    IAVManager*     _socketAVManager;
    // Pipe to which this belongs
    uint32          _socketPipeIdN;

    // Http streamer to use
    IStreamerHttp*  _chunkSocket;
    // Chunk buffer pointer
    byte*           _chunkBuffer;
    // Block size of the chunk buffer pointer
    uint32          _chunkBlockSize;

    // MP4 parser
    FMP4Parser      _fmp4Parser;
};

// ===============================================================================================================
// ===============================================================================================================

class CSocketMbrManifest
{
public:
    CSocketMbrManifest(IAVManager* avManager, uint32 pipeIdN);
    virtual ~CSocketMbrManifest();

private:
    void                     ResetDeflate();

public:
    void                     Cleanup(void);
    bool                     IsConnected(void) const;
    eSocketError             GetSocketError(void) const { return _socketError; }
    pkRESULT                 GetPKResult(void) const { return _socketPKResult; }
    int                      GetHttpResponse(void) const { return _socketHttpResponse; }

    bool                     DownloadParseDefaultManifest(CTuneRequest& tuneRequest, uint64* pConnectResponseTime);
    bool                     DownloadParseSegmentManifest(_In_ const std::wstring& strUrl);
    void                     SetChunkManifest(MBR::CChunkManifest* pChunkManifest) { _apChunkManifest.Set(pChunkManifest); }
    bool                     GetSessionIdHeader(std::string& sessionId);

private:
    bool                     DownloadManifest(_Inout_ std::string* pStrUrl);
    int                      RecvCount(_Out_bytecap_(dstlen) byte* dst, _In_ int dstlen, _In_opt_ int timeout = 0);

    // XML parser stream reader callbacks
    static void              ReadManifestCB(_In_ HANDLE hStream, _Out_bytecap_(bufferSize) byte* pBuffer, _In_ uint32 bufferSize, _Out_ uint32* pLen);
    static void              ReadDeflateManifestCB(_In_ HANDLE hStream, _Out_bytecap_(bufferSize) byte* pBuffer, _In_ uint32 bufferSize, _Out_ uint32* pLen);

    int                      ReadManifest(_Out_bytecap_(len) byte* pBuffer, _In_ int len);
    int                      ReadDeflateManifest(_Out_bytecap_(len) byte* pBuffer, _In_ int len);

    //Factory of factories
    IAVManager*              _socketAVManager;
    // Pipe to which this belongs
    uint32                   _socketPipeIdN;

    // Http streamer to use
    IStreamerHttp*           _pStreamerHttp;

    //Actual chunk manifest parsed
    AutoRefPtr<MBR::CChunkManifest>     _apChunkManifest;

    int                      _manifestSize;
    bool                     _endOfManifest;

    eSocketError             _socketError;
    pkRESULT                 _socketPKResult;
    int                      _socketHttpResponse;

    // zlib deflate
    z_stream                 _zStream;
    bool                     _zStreamInit;
    byte*                    _zBuffer;

    static const int ZBUFFER_SIZE = 1024;
};

// ===============================================================================================================
// ===============================================================================================================
struct IInternalMbrManifest
{
    virtual bool ParentStreamsContainsTime(_In_ TimeSpan_hns time) = 0;
    virtual void ReportStatus(_In_ const std::string& info) = 0;
    virtual void PlaybackRangeComplete(pkRESULT result) = 0;
    virtual TimeSpan_hns GetRequestedMinTime() = 0;
    virtual void SetDVRMinTime(TimeSpan_hns pos) = 0;
    virtual bool IsDVRFull() = 0;
    virtual TimeSpan_hns MaxOfParentStreamMinTime() = 0;
    virtual TimeSpan_hns SegmentDuration() = 0;
    virtual pkRESULT GetSegmentManifestURL( _In_ TimeSpan_hns chunkStartTime, _Out_ std::wstring* pOutUrl ) = 0;
    virtual bool DownloadParseSegmentManifest( _In_ std::wstring& url, _Out_ int* pHttpResponse ) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
class CSegmentManifestFetcher
{
public:

    CSegmentManifestFetcher();

    ~CSegmentManifestFetcher();

    pkRESULT Initialize(_In_ IInternalMbrManifest* pMbrManifest );

private:
    void Execute();
    pkRESULT ContinueWork();
    void Shutdown();

    IInternalMbrManifest* m_pMbrManifest;
    pkHANDLE m_hWorkThread;
    Lockable m_Lock;

    static uint32_t pkAPI _WorkThreadEntryPoint( void* pParam )
    {
        reinterpret_cast<CSegmentManifestFetcher*>( pParam )->Execute();
        return( 0 );
    }
};

/// <summary>
/// Instantiates the default CSegmentManifestFetcher
/// <summary>
namespace DefaultSegmentManifestFetcher
{
    pkRESULT CreateInstance(
            _In_  IInternalMbrManifest* pStreamsInfo,   // owns the fetcher
            _Out_ CSegmentManifestFetcher** ppFetcher);
}

class CMbrManifest 
    : public ITunePrepare
    , public IRunnable
    , public IDrmCallbackSink
    , public IInternalMbrManifest
{
public:
    CMbrManifest(IAVManager* avManager, uint32 pipeIdN, CTuneRequest& tuneRequest);
    virtual ~CMbrManifest();

private:
    void                 CleanupManifestSocket();

public:
    // ITunePrepare interfaces
    __override           bool Prepare(void);
    __override           void GetPrepareResult(TunePrepareResult* pResult);
    __override           void CancelPrepare(void);
    __override           pkRESULT RequestPlaybackRange( _In_ ISetPlaybackRangeCallback* pCallback, 
                                                        _In_ TimeSpan_NTP minTime, 
                                                        _In_ TimeSpan_NTP maxTime );

    // IRunnable interfaces (chunk update thread)
    __override           void OnThreadRun();
    __override           void OnThreadStop();

    // IDrmCallbackSink interface
    __override           void OPLCallback(XDRM_OPL_DATA* pOPLData);

    // IInternalMbrManifest interface
    __override           bool ParentStreamsContainsTime(_In_ TimeSpan_hns time);
    __override           void ReportStatus(_In_ const std::string& info);
    __override           void PlaybackRangeComplete(_In_ pkRESULT result);
    __override           TimeSpan_hns GetRequestedMinTime();
    __override           void SetDVRMinTime(_In_ TimeSpan_hns pos);
    __override           bool IsDVRFull();
    __override           TimeSpan_hns MaxOfParentStreamMinTime();
    __override           TimeSpan_hns SegmentDuration();
    __override           pkRESULT GetSegmentManifestURL(_In_ TimeSpan_hns chunkStartTime, _Out_ std::wstring* pOutUrl );
    __override           bool DownloadParseSegmentManifest( _In_ std::wstring& url, _Out_ int* pHttpResponse );

    MBR::CChunkManifest* GetChunkManifest() { return _apChunkManifest; }
    IDrmDecrypter*       GetDrmObject() { return _pDrm; }
    CStreamInfoList*     GetStreamInfoList() { return &_streamInfoList; }
    XDRM_OPL_DATA*       GetDrmOPLData() { return &_drmOPLData; }
    void                 StoreSessionId(IStreamerHttp* pStreamerHttp = NULL); // NULL => use _manifestSocket

    eSocketError         GetSocketError(void) const { return _socketError; }
    pkRESULT             GetPKResult(void) const { return _socketPKResult; }
    int                  GetHttpResponse(void) const { return _socketHttpResponse; }

    void                 AddChunk(uint32 dwStreamId, FMP4TrackInfo* pTrackInfo);
    void                 EnableChunkUpdateThread(bool bEnable);
    void                 StopChunkUpdateThread();

    bool                 IsLive(void) { return (_apChunkManifest != NULL) ? _apChunkManifest->IsLive() : false; }
    bool                 IsEndOfLive() const { return _endOfLive; };
    pkRESULT             InitDrmWithHeader(byte *pbHeader, uint32 cbHeader, byte *pbKeyId, uint32 cbKeyId);

    bool                 IsDecrypterOnDemand(void) { return _fDecrypterOnDemand; }
    void                 ReportStartEndTime(bool forceReport = false);
    bool                 CanRequestFragment(_In_ CMediaStreamDescription* pMSD, _In_ int32 chunkIdx, _Out_ TimeSpan_hns* timeDelta);

private:

    // keeps track of retries
    struct CSocketMbrManifestRetry
    {
        bool doDelayRetry;
        CSocketMbrRetry retryState;

        CSocketMbrManifestRetry()
        {
            doDelayRetry = false;
        }
    };

    pkRESULT             InitDrm();
    void                 InitAudioLanguage();
    void                 AddLanguage(MBR::CMediaStreamDescription* pStreamInfo);

    uint64               GetCurrentStreamEndTime(void);
    void                 CheckAtWindowEdge(uint32 streamID);

    void                 InitChunkUpdateThread();
    bool                 ReadNextChunkHeader(_In_ uint32 streamIndex,
                                             _In_ uint64 currentTime,
                                             _Inout_ CSocketMbrManifestRetry* retryInfo);

    void                 StoreExtraTuneParameters();
    void                 NotifyStatus(_In_ const std::string& info, _In_opt_ bool shouldCallCallback = true);
    bool                 RetryChunkInfo( _Inout_ CSocketMbrManifestRetry* retryInfo,
                                         _Inout_ int32* chunkIndex,
                                         _Inout_ uint32* trackIndex,
                                         _In_ uint32 streamIndex);

    void                 ReportTrackInfoFailure(const wstring& url);

    // Factory of factories
    IAVManager*          _socketAVManager;
    IReferenceClock*     _referenceClock;
    // Pipe to which this belongs
    uint32               _socketPipeIdN;

    // Chunk update thread
    Thread*              _chunkUpdateThread;

    enum eUpdateThreadState
    {
        eUpdateThreadState_Init,
        eUpdateThreadState_Enabled,
        eUpdateThreadState_Disabled, // Disabled when playback is near live - can be re-enabled if we fall behind
        eUpdateThreadState_Stopped   // Stopped can only turn back on by explicit EnableChunkUpdateThread() call
    };
    eUpdateThreadState   _chunkUpdateThreadState;

    bool                 _chunkUpdateThreadExitFlag; // wake event is to exit thread
    CEvent               _chunkUpdateThreadWakeEvent;
    Lockable             _chunkUpdateLock;

    // Disable check if ability to request latest chunk via FragmentInfo(video=-1) is not supported
    bool                 _chunkRequestLatestEnabled;

    AutoRefPtr<MBR::CChunkManifest> _apChunkManifest;
    CChunkInfoReader     _chunkInfoReader;
    CSocketMbrManifest   _manifestSocket;

    eSocketError         _socketError;
    pkRESULT             _socketPKResult;
    int                  _socketHttpResponse;

    CTuneRequest         _tuneRequest;

    // Decryption
    IDrmDecrypter*       _pDrm;
    XDRM_OPL_DATA        _drmOPLData;

    CStreamInfoList      _streamInfoList;

    // signed delta between stream time and live time in 10MHz
    int64                _streamTimeDelta;

    // optional device and session Ids
    std::string          _deviceId;
    std::string          _sessionId;

    Lockable             _sessionIdLock;

    bool                 _fDecrypterOnDemand;
    uint64               _lastTimeStreamsUpdated;
    int64                _lastReportedChunkEndTime;
    bool                 _callStartEndTimeCallback;
    bool                 _endOfLive;
    bool                 _endOfLiveReported;
    int                  _recvTimeout;
    
    bool                _manifestReady;
    TimeSpan_hns        _requestedMinTime;
    Lockable            _setPlaybackRangeLock;

    ISetPlaybackRangeCallback* _pPlaybackRangeCallback;
    CSegmentManifestFetcher* _pSegmentFetcher;
};

// ===============================================================================================================
// Enumerates the sparse stream chunk info in a HTTP header. This class doesn't do strict header validation.
// ===============================================================================================================

class CSparseStreamChunkInfoHeaderParser
{
public:

    CSparseStreamChunkInfoHeaderParser( _In_ const char* pszHeader );

    bool MoveNext();

    const std::string& CurrentStreamName() const    { return( m_strCurrentStreamName ); }
    int64              CurrentChunkTime() const     { return( m_currentChunkTime ); }

private:

    const char* m_pszPos;
    std::string m_strCurrentStreamName;
    int64 m_currentChunkTime;
};

static const char* HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO[] =
{
    "Pragma",
    "Content-Type",
};

static const size_t NUM_OF_HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO = sizeof(HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO)/sizeof(HTTP_HEADERS_WITH_SPARSE_CHUNK_INFO[0]);

// ===============================================================================================================
// ===============================================================================================================
