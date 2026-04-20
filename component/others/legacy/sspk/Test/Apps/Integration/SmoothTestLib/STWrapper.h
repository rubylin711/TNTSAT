///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include <string>
#include "ISmoothTransport.h"
#include "DispatchTimer.h"
#include "ManifestReadyCallbackImpl.h"
#include "IDiagsManager.h"
#include <limits>
#include <vector>
#include <map>

using namespace std;
using namespace SSPK;

namespace SSPKTest
{
    struct SSPKCurrentStateUpdates;
    struct SSPKChunkInfo;
    typedef std::map< uint32_t , std::vector<SSPKChunkInfo> > MapOfChunkInfoVectors;
    
    class STWrapper
        : public ISmoothTransportStatusSink
        , public ISmoothTransportErrorSink
        , public IStreamsSelectedCallback
        , public IFragmentCallback
        , public IChunkInfoCallback
        , public ISetPlaybackRangeCallback
        , public DispatchTimer
    {
    private:
        ISmoothTransport* _SmoothTransportPtr;

        bool _mediaOpened;
        bool _mediaFailed;
        bool _isSelectStreamCompleted;
        bool _isManifestReadyCallbackRaised;
        int32_t _streamSelectionCount;
        bool _isLive;
        int64_t _startTime;
        int64_t _endTime;

        SSPKCurrentStateUpdates* _statusUpdatesList;
        SmoothTransportStatus _currentStatus;

        vector<SmoothTransportError> _smoothErrorVector;
        vector<SmoothTransportStatus> _smoothStatusCallbackVector;
        vector<SSPKChunkInfo> _smoothChunkVector;

        MapOfChunkInfoVectors _smoothChunksByStream;

        int64_t _nonAccurateSeekThreshold; //In timescale units

        // events
        CEvent* _selectedStreamsEvent;
        CEvent* _fragmentDownloadEvent;
        CEvent* _fragmentInfoDownloadEvent;
        CEvent* _mediaFailedEvent;
        CEvent* _mediaEndedEvent;
        CEvent* _mediaFailedOrMediaEndedEvent;
        CEvent* _mediaRenderingStartedEvent;
        CEvent* _atWndwEdgeEvent;
        CEvent* _setPlaybackRangeCompletedEvent;

        IDiagsManager* _diagManager;

        double _mainActualPlaybackRate;
        vector<double> _mainActualPlaybackRateList;
        int64_t _lastPosition;
        int64_t _lastTickTimeStamp;
        int32_t _latestBitrateKbps;
        Lockable _lock;
        Lockable _vectorLock;
        Lockable _fragmentInfoLock;
        bool _assertWhenMediaFailed;
        // chunk download params
        bool _isChunkComplete;
        int64_t _chunkTimestamp;
        pkRESULT _chunkDownloadResult;
        pkRESULT _chunkInfoDownloadResult;
        int64_t _chunkInfoTimestamp;
        int64_t _chunkBufferDownloadedSize;
        int64_t _totalChunkSize;
        pkRESULT _setPlaybackRangeResult;
        pkHANDLE _hDiagThread;

        string _completeTraces;
        CManifestReadyCallback _sink;
        AutoRefPtr<IManifest> _pManifest;


        void InitializeStatus();
        void ResetPlayRateTimer();
        void ResetPlayRateList();
        void ManifestReadyPassThrough(IManifest* pManifest, pkRESULT hr);

        bool IsInTrickModeBeforeMediaEnded(bool isInRewind);
        bool IsInDifferentSpeed(bool isInRewind, bool checkPlaybackRate);

    public:
        STWrapper(void);
        ~STWrapper(void);
        void DestroySmoothTransport();
        pkRESULT OpenVideo(const std::string& url);
        pkRESULT OpenVideo(const std::string& url, bool autoPlay, bool assertWhenOpenFailed = true);
        void OpenVideoAndValidate(const std::string& url, bool autoPlay, bool assertWhenOpenFailed = true);
        pkRESULT Play(float speed = 1.0);
        pkRESULT PlayAt(int64_t timestamp, float speed = 1.0);
        void PlayAndValidate(int32_t delayMilliSeconds);
        void PlayAtAndValidate(int64_t timeStamp, float speed = 1.0, int32_t delay = VALIDATION_DELAY);
        bool IsPlaying(bool checkPlaybackRate = true);
        pkRESULT Pause(int32_t delay);
        void PauseAndValidate(int32_t delayMilliSeconds);
        bool IsPaused();
        bool IsMediaEnded();
        pkRESULT Close();
        void CloseAndValidate(int32_t delay);
        bool IsClosed();
        bool IsMediaFailed();
        pkRESULT Seek(int64_t timestamp);
        void SeekMilliseconds(int64_t milliseconds, bool validate = true);
        void SeekAndValidate(int64_t timestamp);
        void SeekToLive(bool validate = true);
        void SeekFromEnd(int64_t millisecondsFromEnd, bool validate = true);
        pkRESULT Skip(int32_t skipSeconds);
        void SkipAndValidate(int32_t skipSeconds);
        pkRESULT SetPlaybackRangeAsync(int64_t leftEdge, _In_ int64_t rightEdge = TimeSpan_hns::ConvertFrom( TimeSpan_NTP::FromTicks(TimeSpan_NTP::MAX_TICKS) ).Ticks());
        void SetPlaybackRangeAndValidate(int64_t newLeftEdge, int32_t completedTimeout); 
        bool IsAtLive();
        pkRESULT FastForward(float rate = 2.0);
        bool IsInFastForward(bool checkPlaybackRate = true);
        void FastForwardAndValidate(float speed, int32_t delayMilliSeconds);
        pkRESULT Rewind(float rate =  -2.0);
        bool IsInRewind(bool checkPlaybackRate = true);
        void RewindAndValidate(float speed, int32_t delayMilliSeconds);
        pkRESULT SlowMotion(float speed = .5);
        void ValidateIfManifestReadyCallbackRaised();
        void CheckNetworkHeuristicBasicBehaviour( AutoRefPtr<IManifest> pManifest, int32_t bandWidthCap );
        void MakeRoomForFastForward();
        void MakeRoomForRewind();

        void SendExtendedCommand(_In_ const char* command, _In_ int argc, _In_ const char* argv[]);

        int64_t GetCurrentPlayBackTime();

        uint32_t GetStatusCallbackCount(ESmoothTransportStatusUpdate updateType);
        string GetMediaTransportInfo();
        SSPKCurrentStateUpdates& GetStatusUpdatesList() const;
        vector<SmoothTransportError> GetErrorCallbackVector();
        vector<SmoothTransportStatus> GetStatusCallbackVector();
        vector<StreamChangedEventArgs> GetStreamSelectionArgs();
        vector<SmoothTransportStatus> GetStatusCallbackVector(ESmoothTransportStatusUpdate updateType);

        int32_t GetStreamSelectionCount();
        int64_t GetRandomSeekablePosition();
        int64_t GetRandomSeekablePositionFromManifest();
        int64_t GetCurrentStartTime();
        int64_t GetCurrentEndTime();
        void GetCurrentPlayablePosition(int64_t* currentStartTime, int64_t* currentEndTime);
        void GetCurrentPlayablePositionFromManifest(int64_t* currentStartTime, int64_t* currentEndTime);
        bool IsAtDVRWindowEnd();
        bool IsAtDVRWindowStart();
        int32_t GetLatestBitrateKBPS();
        AutoRefPtr<IManifest> GetManifest();

        //Frag Info
        MapOfChunkInfoVectors GetFragInfos();
        void AddStreamForFragInfoMonitoring(uint32_t streamID);
        void AddFragInfoToMonitoredStream(uint32_t streamID, SSPKChunkInfo fragInfo);

        vector<StreamChangedEventArgs> streamSelectionArgs;
        bool WaitForMediaRenderingStarted(uint32_t dwMsTimeout);
        bool WaitForSelectedStreams(uint32_t dwMsTimeout);
        bool WaitForMediaFailed(uint32_t playBackTimeout);
        bool WaitForMediaEnded(uint32_t playBackTimeout);
        bool WaitForMediaFailedOrMediaEnded(uint32_t playBackTimeout);
        bool WaitForMediaRenderingAndEnded(bool waitForRendering, uint32_t mediaEndedTimeout );
        bool WaitForAtWndwEdge(uint32_t playBackTimeout);
        bool WaitForManifestReady(uint32_t dwMsTimeout);


        void StatusCallback(SmoothTransportStatus& status);
        void ErrorCallback(SmoothTransportError& errorInfo);
        pkRESULT SetManifestCallback(IManifestReadyCallback *pCallback);
        void ManifestReadyCallback( _In_ IManifest* pManifest, pkRESULT hr);
        void StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs);
        pkRESULT OnFragmentData(_In_ pkRESULT hrResult, _In_ CHUNK_INFO* pChunkInfo, _In_opt_ IRefBuffer* pBuffer, _In_ bool fFinalBuffer, _In_ size_t cbTotalLength );
        void OnChunkInfo(_In_ pkRESULT hrResult, _In_ const CHUNK_INFO* pChunk );
        pkRESULT WaitForFragmentInfoAsync(_In_ uint32_t dwMsTimeout, _Out_ int64_t& chunkInfo);
        pkRESULT WaitForDownloadFragmentAsync(_In_ uint32_t dwMsTimeout, _Out_ bool& isChunkComplete, _Out_ int64_t& downloadedBufferSize, _Out_ int64_t& chunkTimestamp, _Out_ int64_t& totalChunkSize);
        
        void SetPlaybackRangeCallback(_In_ IManifest* pManifest, _In_ pkRESULT result);
        pkRESULT WaitForSetPlaybackRangeCompleted(uint32_t setPlaybackRangeTimeout); 

        void Delay(_In_ int32_t delayTime);
        void RegisterDiagEvents(vector<DiagsChannel> channelVector, DiagsPriority pririty);
        void AddDiagTrace(wstring currentTrace);

        string GetCompleteTraces();
        IDiagsManager* GetDiagManager();

        void OnTick();

        const char* GetTunerStateName( ESmoothTransportState smoothTransportState);
        const char* GetStatusUpdateName( ESmoothTransportStatusUpdate smoothTransportStatusUpdate);
        const char* GetErrorName( ESmoothTransportError smoothTransportTunerError);

        void SetIsLive(bool isLive);
        void SetNonAccurateSeekThreshold( AutoRefPtr<IManifest> manifest );
        bool IsLive();
        int64_t GetNonAccurateSeekThreshold();

        template< class T >
        void SetManifestReadyMethod(T* pThis, void (T::*pFunc)( _In_ IManifest* pManifest, HRESULT hr ) )
        {
            _sink.Set( this, &STWrapper::ManifestReadyPassThrough, pThis, pFunc );
            SetManifestCallback( _sink ) ;  
        }

    };

    struct SSPKCurrentStateUpdates
    {
        int32_t Detuned;
        int32_t Tuning;
        int32_t Playing;
        int32_t Paused;
        int32_t MediaEnded;
        int32_t Closed;
        int32_t Unknown;
        int32_t Max;

        SSPKCurrentStateUpdates() 
            : Detuned(0)
            , Tuning(0)
            , Playing(0)
            , Paused(0)
            , MediaEnded(0)
            , Closed(0)
            , Unknown(0)
            , Max(0)
        {
        }

        void Reset()
        {
            Detuned = 0;
            Tuning = 0;
            Playing = 0;
            Paused = 0;
            MediaEnded = 0;
            Closed = 0;
            Unknown = 0;
            Max = 0;
        }
    };

    struct SSPKChunkInfo
    {
        int32_t _streamId;
        int32_t _chunkIndex;
        int32_t _bitrate;
        string _chunkUrl;

        SSPKChunkInfo() : _streamId(0), _chunkIndex(0), _bitrate(0)
        {
        }

        SSPKChunkInfo(int32_t streamId, int32_t chunkIndex, int32_t bitrate, string chunkUrl)
            : _streamId(streamId), _chunkIndex(chunkIndex), _bitrate(bitrate), _chunkUrl(chunkUrl)
        {
        }
    };
};
