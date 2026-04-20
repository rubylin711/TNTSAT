///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PKTestDefines.h"
#include "STWrapper.h"
#include "PKTestSuite.h"
#include "PKTestSuiteUtils.h"
#include "STUtilities.h"
#include "pkSockets.h"
#include "pkExecutive.h"
#include "IDiagsManager.h"
#include "IAVManager.h"
#include "StringUtils.h"
#include <time.h>
#include <math.h>
#include "CEvent.h"
#include "SSPKHelpers.h"

using namespace std;
using namespace SSPK;
using namespace SSPKTest;


static const int32_t _SLEEP_TIME_FOR_VALIDATING_POSITION = 500;
static const float SKIP_THRESHOLD = 1.0f;
static const float TICKS_PER_SEC = 1000.0f;
static const float PLAY_RATE_THRESHOLD = 0.45f;
static const float COMPARE_DOUBLE_THRESHOLD = 0.01f;
static const int32_t MAX_CHUNK_DURATION_SAMPLE_SIZE = 10;
static const int32_t MIN_CHUNK_DURATION_SAMPLE_SIZE = 2;

static bool diagEnabled = false;

//Diag string parsing
static int32_t GetIntFromDiagEventString(wstring field, wstring eventData);
static void GetValueFromDiagEventString(wstring field, wstring eventData, wstring& outValue);
static SSPKChunkInfo GetChunkInfoFromDiagEventsString(wstring eventData);

STWrapper::STWrapper(void) : _mediaOpened(false), _mediaFailed(false), _isSelectStreamCompleted(false), _isManifestReadyCallbackRaised(false), _isLive(false), _startTime(0), _endTime(0)
{
    pkRESULT result = (SOCKET_SUCCESS == Socket_Startup()) ? pkS_OK : pkE_FAIL;
    if (pkFAILED(result))
    {
        LogTestError("FAILED to start Socket: 0x%X\n", result);
    }

    IAVManager::Create();

    _SmoothTransportPtr = ISmoothTransport::CreateSmoothTransport();

    pkRESULT pkResult = _SmoothTransportPtr->RegisterStatusCallback(this);
    if (pkFAILED(pkResult))
    {
        LogTestError("FAILED to register status callback: 0x%X\n", pkResult);
    }

    pkResult = _SmoothTransportPtr->RegisterErrorCallback(this);
    if (pkFAILED(pkResult))
    {
        LogTestError("FAILED to register error callback: 0x%X\n", pkResult);
    }

    _statusUpdatesList = NEW_NO_THROW SSPKCurrentStateUpdates();

    //Initializing status
    InitializeStatus();

    _assertWhenMediaFailed = true;

    Init(_SLEEP_TIME_FOR_VALIDATING_POSITION);
    _lastPosition = 0;
    _lastTickTimeStamp = Executive_GetTickCount();
    _latestBitrateKbps = 0;

    _selectedStreamsEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto,false);
    _fragmentDownloadEvent =  NEW_NO_THROW CEvent(CEvent::eResetModeAuto,false);
    _fragmentInfoDownloadEvent =  NEW_NO_THROW CEvent(CEvent::eResetModeAuto,false);
    _mediaFailedEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto, false);
    _mediaEndedEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto, false);
    _mediaFailedOrMediaEndedEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto, false);
    _mediaRenderingStartedEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto, false);
    _atWndwEdgeEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto, false);
    _setPlaybackRangeCompletedEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto, false);

    _streamSelectionCount = 0;
    _mainActualPlaybackRate = 0;
    //ChunkPrams
    _isChunkComplete = true;
    _chunkTimestamp = 0;
    _chunkInfoDownloadResult = pkE_UNEXPECTED;
    _chunkDownloadResult = pkE_UNEXPECTED;
    _setPlaybackRangeResult = pkE_UNEXPECTED;
    _chunkInfoTimestamp = 0;
    _chunkBufferDownloadedSize = 0;
    _totalChunkSize = 0;

    //Default value will be overwritten on manifest ready
    _nonAccurateSeekThreshold = NON_ACCURATE_SEEK_THRESHOLD;

    _hDiagThread = 0;
    _diagManager = NULL;

    SetManifestReadyMethod( this, &STWrapper::ManifestReadyPassThrough );
}
STWrapper::~STWrapper(void)
{
    Stop();
    diagEnabled = false;
    if ( _hDiagThread )
    {
        Executive_WaitForThread(_hDiagThread, INFINITE);
        Executive_CloseThread(_hDiagThread);
    }

    if(NULL != _SmoothTransportPtr)
    {
        ISmoothTransport::DestroySmoothTransport(_SmoothTransportPtr);
    }

    delete _statusUpdatesList;
    delete _selectedStreamsEvent;
    delete _fragmentDownloadEvent;
    delete _fragmentInfoDownloadEvent;
    delete _mediaFailedEvent;
    delete _mediaEndedEvent;
    delete _mediaFailedOrMediaEndedEvent;
    delete _mediaRenderingStartedEvent;
    delete _atWndwEdgeEvent;
    delete _setPlaybackRangeCompletedEvent;

    ResetPlayRateList();

    IAVManager::Destroy();
    Socket_Cleanup();
}

void STWrapper::DestroySmoothTransport()
{
    Stop();
    ISmoothTransport::DestroySmoothTransport(_SmoothTransportPtr);
    _SmoothTransportPtr = NULL;
}

void STWrapper::InitializeStatus()
{
    //Initializing status
    _currentStatus._currentState = SmoothTransportTunerState_Unknown;
    _currentStatus._currentTime.ResetTicks();
    _currentStatus._startTime.ResetTicks();
    _currentStatus._endTime.ResetTicks();
    _currentStatus._speed = 0;
    _currentStatus._update = SmoothTransportStatus_Heartbeat;
    _currentStatus._clockStarted = false;
}

void STWrapper::ResetPlayRateTimer()
{
    _lastPosition = 0;
    _lastTickTimeStamp = Executive_GetTickCount();
    _latestBitrateKbps = 0;

    ResetPlayRateList();
    _mediaRenderingStartedEvent->Reset();
}

void STWrapper::ResetPlayRateList()
{
    AutoLock lock(&_lock);
    _mainActualPlaybackRateList.clear();
}

pkRESULT STWrapper::OpenVideo(const std::string& url)
{
    return OpenVideo(url, true);
}
pkRESULT STWrapper::OpenVideo(const std::string& url, bool autoPlay, bool assertWhenFailed)
{
    _assertWhenMediaFailed = assertWhenFailed;

    _mediaFailed = false;
    _isSelectStreamCompleted = false;
    _isManifestReadyCallbackRaised = false;
    _mediaFailedOrMediaEndedEvent->Reset();
    InitializeStatus();
    ResetPlayRateTimer();
    {
        AutoLock lock(&_vectorLock);
        _smoothStatusCallbackVector.clear();
        _smoothErrorVector.clear();
        _smoothChunkVector.clear();
        AutoLock fragmentlock(&_fragmentInfoLock);
        _smoothChunksByStream.clear();
    }

    Start();
    _mediaRenderingStartedEvent->Reset();
    return _SmoothTransportPtr->Open(url, SmoothTransportProtocol_Mbr, autoPlay);
}
void STWrapper::OpenVideoAndValidate(const std::string& url, bool autoPlay, bool assertWhenOpenFailed)
{
    PKTEST_HRESULT_EXIT(OpenVideo(url, autoPlay, assertWhenOpenFailed));

    PKTEST_ASSERT_MSG_EXIT( WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );
    if(autoPlay)
    {
        PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
    }

exit:
    return;
}

pkRESULT STWrapper::Play(float speed)
{
    _mediaRenderingStartedEvent->Reset();
    return _SmoothTransportPtr->Play(speed);
}
pkRESULT STWrapper::PlayAt(int64_t timestamp, float speed)
{
    _mediaRenderingStartedEvent->Reset();
    return _SmoothTransportPtr->PlayAt( TimeSpan_NTP::ConvertFrom( TimeSpan_hns::FromTicks( timestamp ) ), speed );
}

bool STWrapper::IsPlaying(bool checkPlaybackRate)
{
    LogTestComment("Current position %lld", GetCurrentPlayBackTime());
    {
        AutoLock lock(&_lock);
        if( _currentStatus._currentState == SmoothTransportTunerState_MediaEnded )
        {
            // removing all zeros after Media Ended.
            while( !_mainActualPlaybackRateList.empty()
                && _mainActualPlaybackRateList.back() == 0 )
            {
                 _mainActualPlaybackRateList.pop_back();
            }
        }
        else if(_currentStatus._currentState != SmoothTransportTunerState_Playing)
        {
            LogTestComment("Expected the current state to be SmoothTransportTunerState_Playing but it is instead %s", GetTunerStateName(_currentStatus._currentState));
            return false;
        }
    }
    
    //TODO.playback rate given from mediaTransport (status callback)
    // We need to make sure the speed reported by status callback is 1.
    if(!checkPlaybackRate)
    {
        return true;
    }
    double averagePlaybackRate = 0;
    {
        AutoLock lock(&_lock);

        //Skipping the first element in the list, because we will have a big jump in the media position, when we do a seek or skip or playat.
        if (_mainActualPlaybackRateList.size() > 1)
        {
            for(uint32_t i = 1 ; i <= _mainActualPlaybackRateList.size() -1 ; i++)
            {
                averagePlaybackRate += (_mainActualPlaybackRateList[i]);
            }

            averagePlaybackRate /= (_mainActualPlaybackRateList.size() - 1);
        }
    }
    if(averagePlaybackRate > (1 + PLAY_RATE_THRESHOLD) || averagePlaybackRate < (1 - PLAY_RATE_THRESHOLD))
    {
        LogTestComment("IsPlaying failed because playrate (%.3f) is outside of expected bounds, using %lld samples", averagePlaybackRate, (int64_t)(_mainActualPlaybackRateList.size() - 1) );
        return false;
    }

    return true;
}
bool STWrapper::IsPaused()
{
    //TODO. Check for Media Opened.

    if(_currentStatus._currentState != SmoothTransportTunerState_Paused)
    {
        LogTestComment("IsPaused: Expected _currentStatus._currentSate to be SmoothTransportTunerState_Paused, but the actual value is %s", GetTunerStateName(_currentStatus._currentState));
        return false;
    }
    //TODO.playback rate given from mediaTransport
    // Add a threshold when comparing two doubles.
    if(_mainActualPlaybackRate > COMPARE_DOUBLE_THRESHOLD || _mainActualPlaybackRate < -COMPARE_DOUBLE_THRESHOLD)
    {
        LogTestComment("mainActualPlaybackRate (%.2f) is not within expected range: [%.2f,%.2f]", _mainActualPlaybackRate, COMPARE_DOUBLE_THRESHOLD,-COMPARE_DOUBLE_THRESHOLD);
        return false;
    }
    return true;
}
bool STWrapper::IsMediaEnded()
{
    if(_currentStatus._currentState == SmoothTransportTunerState_MediaEnded)
    {
        return true;
    }
    return false;
}
void STWrapper::PlayAndValidate(int32_t delay)
{
    Play();
    Delay(delay);
    PKTEST_ASSERT_EXIT( IsPlaying() );

exit:
    return;
}
void STWrapper::PlayAtAndValidate(int64_t timeStamp, float speed, int32_t delay)
{
    int64_t currentTime = 0, currentStartTime = 0, currentEndTime = 0, diff = 0;
    LogTestComment("PlayAt: %lld with Speed: %.2f", timeStamp, speed);

    PKTEST_HRESULT_EXIT( PlayAt(timeStamp, speed) );
    GetCurrentPlayablePosition(&currentStartTime, &currentEndTime);

    if(timeStamp < currentStartTime)
    {
        PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        currentTime = GetCurrentPlayBackTime();

        diff = currentTime > currentStartTime ? currentTime - currentStartTime: currentStartTime - currentTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(currentTime, currentStartTime , _nonAccurateSeekThreshold), "Current time After PlayAt (%lld) does not match given PlayAt time (%lld) with Threshold : %lld.  Actual difference is %lld (%.3f seconds)", currentTime, currentStartTime, _nonAccurateSeekThreshold, diff, TimeSpan_hns::FromTicks(diff).ToSeconds() );
    }
    else if(timeStamp > currentEndTime)
    {
        if(!IsLive() && speed >= 1)
        {
            PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingAndEnded(false, STATECHANGE_TIMEOUT), "Timeout before media ended");
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        }

        currentTime = GetCurrentPlayBackTime();

        diff = currentTime > currentEndTime ? currentTime - currentEndTime: currentEndTime - currentTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(currentTime, currentEndTime, _nonAccurateSeekThreshold), "Current time After PlayAt (%lld) does not match given PlayAt time (%lld) with Threshold : %lld.  Actual difference is %lld (%.3f seconds)", currentTime, currentEndTime, _nonAccurateSeekThreshold, diff, TimeSpan_hns::FromTicks(diff).ToSeconds() );
    }
    else
    {
        PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        currentTime = GetCurrentPlayBackTime();

        diff = currentTime > timeStamp ? currentTime - timeStamp: timeStamp - currentTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(currentTime, timeStamp, _nonAccurateSeekThreshold), "Current time After PlayAt (%lld) does not match given PlayAt time (%lld) with Threshold : %lld.  Actual difference is %lld (%.3f seconds)", currentTime, timeStamp, _nonAccurateSeekThreshold, diff, TimeSpan_hns::FromTicks(diff).ToSeconds() );
    }

    if(delay != 0)
    {
        Delay(delay);

        if( speed > 1.0 )
        {
            PKTEST_ASSERT_EXIT(IsInFastForward());
        }
        else if( speed < -1.0 )
        {
            PKTEST_ASSERT_EXIT(IsInRewind());
        }
        else if( speed == 1.0 )
        {
            PKTEST_ASSERT_EXIT(IsPlaying());
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "%.3f speed is not supported", speed);
        }
    }

exit:
    return;
}
pkRESULT STWrapper::Seek(int64_t timestamp)
{
    ResetPlayRateTimer();
    _smoothChunkVector.clear();
    LogTestComment("Seek at: %lld", timestamp);
    pkRESULT pkResult = _SmoothTransportPtr->Seek( TimeSpan_NTP::ConvertFrom( TimeSpan_hns::FromTicks( timestamp ) ) );
    return pkResult;
}
void STWrapper::SeekMilliseconds(int64_t milliseconds, bool validate)
{
    int64_t seekTo = TimeSpan_hns::ConvertFrom( TimeSpan_ms::FromTicks( milliseconds) ).Ticks();

    if(validate)
    {
        SeekAndValidate( seekTo );
    }
    else
    {
        PKTEST_HRESULT_EXIT( Seek( seekTo ) );
    }
exit:
    return;

}
void STWrapper::SeekAndValidate(int64_t timeStamp)
{
    int64_t currentTime = 0, currentStartTime = 0, currentEndTime = 0, diff = 0;

    PKTEST_HRESULT_EXIT(Seek(timeStamp));
    GetCurrentPlayablePosition(&currentStartTime, &currentEndTime);

    if(timeStamp < currentStartTime)
    {
        PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        currentTime = GetCurrentPlayBackTime();
        GetCurrentPlayablePosition(&currentStartTime, &currentEndTime);

        diff = currentTime > currentStartTime? currentTime - currentStartTime : currentStartTime  - currentTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(currentTime, currentStartTime , _nonAccurateSeekThreshold), "Current time After Seek (%lld) does not match given Seek time (%lld) with Threshold : %lld. Actual difference is %lld (%.3f seconds)", currentTime, currentStartTime, _nonAccurateSeekThreshold, diff, TimeSpan_hns::FromTicks( diff ).ToSeconds() );
    }
    else if(timeStamp > currentEndTime)
    {
        if(!IsLive())
        {
            PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingAndEnded(false, STATECHANGE_TIMEOUT), "Timeout before media ended");
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        }

        currentTime = GetCurrentPlayBackTime();
        GetCurrentPlayablePosition(&currentStartTime, &currentEndTime);

        diff = currentTime > currentEndTime? currentTime - currentEndTime : currentEndTime - currentTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(currentTime, currentEndTime, _nonAccurateSeekThreshold), "Current time After Seek (%lld) does not match given Seek time (%lld) with Threshold : %lld. Actual difference is %lld (%.3f seconds)", currentTime, currentEndTime, _nonAccurateSeekThreshold, diff, TimeSpan_hns::FromTicks( diff ).ToSeconds() );
    }
    else
    {
        PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        currentTime = GetCurrentPlayBackTime();
        

        diff = currentTime > timeStamp? currentTime - timeStamp : timeStamp - currentTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(currentTime, timeStamp, _nonAccurateSeekThreshold), "Current time After Seek (%lld) does not match given Seek time (%lld) with Threshold : %lld. Actual difference is %lld (%.3f seconds)", currentTime, timeStamp, _nonAccurateSeekThreshold, diff, TimeSpan_hns::FromTicks( diff ).ToSeconds() );
    }

exit:
    return;
}
void STWrapper::SeekToLive(bool validate)
{
    SeekMilliseconds(SecondsToMilliseconds(TOTAL_SECONDS_IN_A_DAY), validate);
}
void STWrapper::SeekFromEnd(int64_t millisecondsFromEnd, bool validate)
{
    //Seek to 0 if millisecondsFromEnd is too large
    int64_t seekTo = 0;
    int64_t endTime = GetCurrentEndTime();

    if(millisecondsFromEnd < endTime)
    {
        seekTo = endTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(millisecondsFromEnd)).Ticks();
    }

    LogTestComment("Seeking to %lld from %lld milliseconds", seekTo, TimestampToMilliseconds(GetCurrentPlayBackTime()));

    if(validate)
    {
        SeekAndValidate( seekTo );
    }
    else
    {
        PKTEST_HRESULT_EXIT( Seek( seekTo ) );
    }
exit:
    return;
}
pkRESULT STWrapper::Pause(int32_t delay)
{
    pkRESULT pkResult = _SmoothTransportPtr->Pause();
    Delay(delay);
    return pkResult;
}
void STWrapper::PauseAndValidate(int32_t delay)
{
    Pause(delay);
    PKTEST_ASSERT_EXIT( IsPaused() );

exit:
    return;
}
pkRESULT STWrapper::Close()
{
    pkRESULT pkResult = _SmoothTransportPtr->Close();
    Stop();
    AutoLock lock(&_lock);
    ResetPlayRateTimer();
    return pkResult;
}
void STWrapper::CloseAndValidate(int32_t delay)
{
    this->Close();
    Delay(delay);
    PKTEST_ASSERT_EXIT( IsClosed() );

exit:
    return;
}
bool STWrapper::IsClosed()
{
    // This is the case where tried open and failed.
    if(_mediaFailed)
    {
        return true;
    }
    if(_currentStatus._currentState == SmoothTransportTunerState_Closed)
    {
        return true;
    }
    if(GetCurrentPlayBackTime() == 0)
    {
        return true;
    }
    return false;
}
bool STWrapper::IsMediaFailed()
{
    return _mediaFailed;
}
pkRESULT STWrapper::Skip(int32_t skipSeconds)
{
    ResetPlayRateTimer();
    _smoothChunkVector.clear();
    pkRESULT pkResult = _SmoothTransportPtr->Skip( TimeSpan_NTP::ConvertFrom( TimeSpan_s::FromTicks( skipSeconds ) ) );
    return pkResult;
}
void STWrapper::SkipAndValidate(int32_t skipSeconds)
{
    int64 afterTimeStamp = 0;
    int64 beforeTimeStamp  = GetCurrentPlayBackTime();
    LogTestComment("Skip: %d seconds", skipSeconds);
    PKTEST_HRESULT_EXIT( Skip(skipSeconds) );
    PKTEST_ASSERT_MSG_EXIT(WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout occured before rendering");
    afterTimeStamp = GetCurrentPlayBackTime();

    PKTEST_ASSERT_MSG_EXIT(afterTimeStamp > beforeTimeStamp + (skipSeconds * TIMESCALE_10MHZ) - ((int64_t)_nonAccurateSeekThreshold + VALIDATION_DELAY) ||
        afterTimeStamp < beforeTimeStamp + (skipSeconds * TIMESCALE_10MHZ) + ((int64_t)_nonAccurateSeekThreshold + VALIDATION_DELAY),
        "Skip didn't happen as expected. Time Before Skip: %d  Time After Skip: %d. Difference(%d) is different from expected(%d)",
        beforeTimeStamp, afterTimeStamp, (double)(afterTimeStamp - beforeTimeStamp) / TIMESCALE_10MHZ, skipSeconds);
    LogTestWarning("skip done");
    PKTEST_ASSERT_EXIT(IsPlaying(false));

exit:
    return;
}
pkRESULT STWrapper::SetPlaybackRangeAsync(int64_t minEdge, int64_t maxEdge)
{
    TimeSpan_NTP maxEdgeNTP; 
    if(maxEdge != TimeSpan_hns::ConvertFrom(TimeSpan_NTP::FromTicks(TimeSpan_NTP::MAX_TICKS)).Ticks())
    {
        maxEdgeNTP = TimeSpan_NTP::ConvertFrom(TimeSpan_hns::FromTicks(maxEdge));  
    }
    else
    {
        maxEdgeNTP = TimeSpan_NTP::FromTicks(TimeSpan_NTP::MAX_TICKS);
    }
    return _SmoothTransportPtr->SetPlaybackRangeAsync(this, TimeSpan_NTP::ConvertFrom(TimeSpan_hns::FromTicks(minEdge)), maxEdgeNTP);
}
void STWrapper::SetPlaybackRangeAndValidate(int64_t minEdge, int32_t completedTimeout)
{
    int64_t newMinEdge = 0, oldMaxEdge = 0, newMaxEdge = 0;
    oldMaxEdge = GetCurrentEndTime(); 

    PKTEST_HRESULT_MSG_EXIT(SetPlaybackRangeAsync(minEdge), "SetPlaybackRange should not fail" );
    PKTEST_HRESULT_MSG_EXIT( WaitForSetPlaybackRangeCompleted(completedTimeout), "Timeout happened before SetPlayBackRangeCompleted callback" );

    newMinEdge = GetCurrentStartTime();
    newMaxEdge = GetCurrentEndTime(); 

    PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(minEdge, newMinEdge, _nonAccurateSeekThreshold ), "Proposed SetPlaybackRange time is different from new ");
    PKTEST_ASSERT_MSG_EXIT(oldMaxEdge <= newMaxEdge, "the old Right edge should be lesser or equal to new right edge as new chunks are added to the chunklist");

exit:
    return;
}
bool STWrapper::IsAtLive()
{
    int64_t lastChunkTimeStamp = 0;
    int64_t currentTimeStamp = 0;

    {
        AutoLock lock(&_vectorLock);
        for (int32_t i = _smoothStatusCallbackVector.size() - 1 ; i >= 0; i--)
        {
            if( _smoothStatusCallbackVector[i]._update == SmoothTransportStatus_StartEndTime)
            {
                lastChunkTimeStamp = TimeSpan_hns::ConvertFrom(_smoothStatusCallbackVector[i]._endTime).Ticks();
                break;
            }
        }
    }
    currentTimeStamp = GetCurrentPlayBackTime();
    if( lastChunkTimeStamp - currentTimeStamp > _nonAccurateSeekThreshold || ( lastChunkTimeStamp == 0 && currentTimeStamp == 0 ) )
    {
        return false;
    }
    return true;
}
pkRESULT STWrapper::FastForward(float rate)
{
    _mediaRenderingStartedEvent->Reset();
    float actualRate = max(1.0f, (float)fabs(rate));
    LogTestComment("Fast forward rate : %.2f", actualRate );
    return _SmoothTransportPtr->Play( actualRate );
}
bool STWrapper::IsInFastForward(bool checkPlaybackRate)
{
    return IsInDifferentSpeed(false, checkPlaybackRate);
}

void STWrapper::FastForwardAndValidate(float speed, int32_t delayMilliSeconds)
{
    //Set speed
    FastForward(speed);
    //Wait and validate
    Delay(delayMilliSeconds);
    PKTEST_ASSERT_EXIT( IsInFastForward() );

exit:
    return;
}
pkRESULT STWrapper::Rewind(float rate)
{
    _mediaRenderingStartedEvent->Reset();
    //Rate is arbitrarily limited to -0.1
    float actualRate = min(-0.1f, -1.0f * (float)fabs(rate));
    LogTestComment("Rewind rate : %.2f", actualRate );
    return _SmoothTransportPtr->Play( actualRate );
}
bool STWrapper::IsInRewind(bool checkPlaybackRate)
{
    return IsInDifferentSpeed(true, checkPlaybackRate);
}

void STWrapper::RewindAndValidate(float speed, int32_t delayMilliSeconds)
{
    //Set speed
    Rewind(speed);
    //Wait and validate
    Delay(delayMilliSeconds);
    PKTEST_ASSERT_EXIT( IsInRewind() );

exit:
    return;
}

//Slow motion is not supported at SSPK level; this method is for testing that it behaves properly when slow motion is attempted
pkRESULT STWrapper::SlowMotion(float speed)
{
    //Make sure that speed is in the proper range, (-1,1)
    speed = max(-1.0f + COMPARE_DOUBLE_THRESHOLD, speed);
    speed = min(speed, 1.0f - COMPARE_DOUBLE_THRESHOLD);

    //Play at given rate
    return _SmoothTransportPtr->Play( speed );

}

void STWrapper::ValidateIfManifestReadyCallbackRaised()
{
    PKTEST_ASSERT_MSG_EXIT(_isManifestReadyCallbackRaised, "Manifest Ready Callback has not occured");

exit:
    return;
}
void STWrapper::CheckNetworkHeuristicBasicBehaviour( AutoRefPtr<IManifest> pManifest, int32_t bandWidthCap )
{
    int32_t closestAvailableBitrate = 0;
    int32_t closestBitrateAboveCap = 0;
    int32_t differenceVar = INT_MAX;
    int32_t differenceVarAbove = INT_MIN;
    int32_t closestBitrateCount = 0;
    int32_t totalVideoChunkCount = 0;
    std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
    std::vector< AutoRefPtr<IManifestStream> > availableStreams;
    AutoRefPtr<IManifestStream> videoStream;

    PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams(&availableStreams) );
    for(uint32_t i = 0; i < availableStreams.size(); i++)
    {
        if( availableStreams[i]->Type() == MediaStreamTypeVideo )
        {
            videoStream = availableStreams[i];
            break;
        }
    }

    PKTEST_HRESULT_EXIT( videoStream->GetAvailableTracks( &availableTracks ) );

    for (uint32_t i = 0; i < availableTracks.size(); i++)
    {
        int32_t currentDifference = bandWidthCap - availableTracks[i]->Bitrate();
        if(currentDifference > 0)
        {
            if( currentDifference < differenceVar )
            {
                differenceVar = currentDifference;
                closestAvailableBitrate = availableTracks[i]->Bitrate();
            }
        }
        else
        {
            if( currentDifference > differenceVarAbove )
            {
                differenceVarAbove = currentDifference;
                closestBitrateAboveCap = availableTracks[i]->Bitrate();
            }
        }
    }

    {
        AutoLock lock(&_lock);
        for(uint32_t i = 0;i < _smoothChunkVector.size(); i++ )
        {
            if(differenceVar != INT_MAX)
            {
                if(_smoothChunkVector[i]._bitrate == closestAvailableBitrate)
                {
                    closestBitrateCount++;
                }
            }
            else
            {
                if(_smoothChunkVector[i]._bitrate == closestBitrateAboveCap)
                {
                    closestBitrateCount++;
                }
            }
            totalVideoChunkCount++;
        }
    }

    PKTEST_ASSERT_MSG_EXIT(closestBitrateCount > (totalVideoChunkCount * 2)/3 , "The count of the closestBitrate must be atleast greater than 2/3 rd of the total chunk count" );
    /// Need to add more sophisticated check here. But not high priority.

    {
        AutoLock lock(&_vectorLock);
        for (uint32_t j = 0 ; j < _smoothStatusCallbackVector.size(); j++)
        {
            PKTEST_ASSERT_MSG_EXIT( _smoothStatusCallbackVector[j]._update != SmoothTransportStatus_Underrun , "We should not have any underruns.");
        }
    }

exit:
    return;
}

void STWrapper::MakeRoomForFastForward()
{
    int64_t nearEnd = GetCurrentEndTime() - SHORT_PLAY_DURATION_10MHZ * 2;

    //Start over so that there is room to fast forward without hitting the end of the content
    if(GetCurrentPlayBackTime() >= nearEnd)
    {
        PlayAtAndValidate(GetCurrentStartTime() + SHORT_PLAY_DURATION_10MHZ * 2);
    }
}

void STWrapper::MakeRoomForRewind()
{
    int64_t nearEnd = GetCurrentEndTime() - DEFAULT_SKIP_10MHZ;
    //Go to near the end so that there is room to rewind without hitting the beginning of the content

    if(GetCurrentPlayBackTime() <=  nearEnd)
    {
        PlayAtAndValidate(nearEnd);
    }
}

void STWrapper::SendExtendedCommand(_In_ const char* command, _In_ int argc, _In_ const char* argv[])
{
    if (pkFAILED(_SmoothTransportPtr->SendExtendedCommand(command, argc, argv)))
    {
        LogTestWarning("SendExtendedCommand %s failed\n", command);
    }
}

int64_t STWrapper::GetCurrentPlayBackTime()
{
    TimeSpan_NTP currentTime;
    _SmoothTransportPtr->GetCurrentPlaybackTime(&currentTime);
    return TimeSpan_hns::ConvertFrom(currentTime).Ticks();
}

string STWrapper::GetMediaTransportInfo()
{
    if(NULL == _SmoothTransportPtr)
    {
        return "The MediaTransport object is null";
    }
    string result = "";
    result = result + "\nMediaTransport info = ";
    //result = result + "\n\tAutoPlay: " + _MediaTransportPtr->
    result = result + ("\n\tCurrent State: " + _currentStatus._currentState);

    return result;
}

int32_t STWrapper::GetStreamSelectionCount()
{
    return _streamSelectionCount;
}

SSPKCurrentStateUpdates& STWrapper::GetStatusUpdatesList() const
{
    return *_statusUpdatesList;
}

vector<SmoothTransportError> STWrapper::GetErrorCallbackVector()
{
    return _smoothErrorVector;
}

vector<SmoothTransportStatus> STWrapper::GetStatusCallbackVector()
{
    return _smoothStatusCallbackVector;
}

vector<SmoothTransportStatus> STWrapper::GetStatusCallbackVector(ESmoothTransportStatusUpdate updateType)
{
    AutoLock lock(&_vectorLock);
    vector<SSPK::SmoothTransportStatus> statusCallbacks;
    for(vector<SSPK::SmoothTransportStatus>::const_iterator current = _smoothStatusCallbackVector.begin(); current != _smoothStatusCallbackVector.end(); ++current)
    {
        if(current->_update == updateType)
        {
            statusCallbacks.push_back(*current);
        }
    }

    return statusCallbacks;
}

int32_t STWrapper::GetLatestBitrateKBPS()
{
	return _latestBitrateKbps;
}

AutoRefPtr<IManifest> STWrapper::GetManifest()
{
    return _pManifest;
}

std::vector<StreamChangedEventArgs> STWrapper::GetStreamSelectionArgs()
{
    return streamSelectionArgs;
}

void STWrapper::Delay(_In_ int32_t delayMilliSeconds)
{
    Sleep(delayMilliSeconds);
}

int64_t STWrapper::GetRandomSeekablePosition()
{
    int64_t startTime = 0, endTime = 0, seekablePosition = 0;

    GetCurrentPlayablePosition(&startTime, &endTime);
    
    seekablePosition = STUtilities::Rand(startTime, endTime);

    return seekablePosition;
}

int64_t STWrapper::GetRandomSeekablePositionFromManifest()
{
    int64_t startTime = 0, endTime = 0, seekablePosition = 0;

    GetCurrentPlayablePositionFromManifest(&startTime, &endTime);

    seekablePosition = STUtilities::Rand(startTime, endTime);

    return seekablePosition;
}


int64_t STWrapper::GetCurrentStartTime()
{
    return _startTime;
}

int64_t STWrapper::GetCurrentEndTime()
{
    return _endTime;
}

bool STWrapper::IsAtDVRWindowEnd()
{
    int64_t startTime = 0, endTime = 0;
    int64_t currentTime = GetCurrentPlayBackTime();

    if(_currentStatus._currentState == SmoothTransportTunerState_MediaEnded)
    {
        startTime = GetCurrentStartTime();
        endTime = GetCurrentEndTime();
    }
    else
    {
        GetCurrentPlayablePosition(&startTime, &endTime);
    }

    if( currentTime == endTime)
    {
        return true;
    }

    if( !CompareTimeStamps(currentTime, endTime, _nonAccurateSeekThreshold + LIVE_TO_FRAGINFO_START_TIME) )
    {
        int64_t diff = currentTime > endTime? currentTime - endTime: endTime - currentTime;
        LogTestComment("%s is larger", currentTime > endTime? "currentTime":"endTime");
        LogTestComment("Current time (%lld)is not near DVR End(%lld) with Threshold : %lld.  Actual difference is %lld (%.3f seconds)", currentTime, endTime, _nonAccurateSeekThreshold + LIVE_TO_FRAGINFO_START_TIME, diff, TimeSpan_hns::FromTicks(diff).ToSeconds());
        return false;
    }

    return true;
}

bool STWrapper::IsAtDVRWindowStart()
{
    int64_t startTime = 0, endTime = 0;
    int64_t currentTime = GetCurrentPlayBackTime();

    if(_currentStatus._currentState == SmoothTransportTunerState_MediaEnded)
    {
        startTime = GetCurrentStartTime();
        endTime = GetCurrentEndTime();
    }
    else
    {
        GetCurrentPlayablePosition(&startTime, &endTime);
    }

    if( currentTime == startTime)
    {
        return true;
    }

    if( !CompareTimeStamps(currentTime, startTime, _nonAccurateSeekThreshold) )
    {
        int64_t diff = currentTime > startTime? currentTime - startTime: startTime - currentTime;
        LogTestComment("%s is larger", currentTime > startTime? "currentTime":"startTime");
        LogTestComment("Current time (%lld)is not near DVR Start(%lld) with Threshold : %lld.  Actual difference is %lld (%.3f seconds)", currentTime, startTime, _nonAccurateSeekThreshold, diff, TimeSpan_hns::FromTicks(diff).ToSeconds());
        return false;
    }

    return true;
}

void STWrapper::GetCurrentPlayablePosition(int64_t* currentStartTime, int64_t* currentEndTime)
{
    if(_isLive && (_currentStatus._currentState == SmoothTransportTunerState_Playing || _currentStatus._currentState == SmoothTransportTunerState_Paused || _currentStatus._currentState == SmoothTransportTunerState_MediaEnded ) )
    {
        *currentStartTime = GetCurrentStartTime();
        if(_pManifest->Type() != ManifestType_Segmented)
        {
            *currentStartTime += LIVE_LEFT_BACKOFF_TIME;
        }
        *currentEndTime = (GetCurrentEndTime() < LIVE_BACKOFF_TIME) ? 0 : GetCurrentEndTime() - LIVE_BACKOFF_TIME;
    }
    else
    {
        *currentStartTime = GetCurrentStartTime();
        *currentEndTime = GetCurrentEndTime();
    }
}

void STWrapper::GetCurrentPlayablePositionFromManifest(int64_t* currentStartTime, int64_t* currentEndTime)
{
    *currentStartTime = TimeSpan_hns::ConvertFrom( TimeSpan_NTP::FromTicks( _pManifest->StartTime() ) ).Ticks();
    if(_isLive)
    {
        *currentEndTime = *currentStartTime + TimeSpan_hns::ConvertFrom( TimeSpan_s::FromTicks( _pManifest->DVRWindowLength()) ).Ticks();
    }
    else
    {
        *currentEndTime = *currentStartTime + _pManifest->Duration();
    }
}

uint32_t pkAPI DiagEventRetrieveFun(void* param)
{
    if (param)
    {
        STWrapper* stObject = ((STWrapper*)param);
        IDiagsManager* pDiagsManager = stObject->GetDiagManager();

        int32_t isError;
        static const size_t EVENT_LENGTH = 256;
        static const size_t EVENT_DATA_LENGTH = 4096;

        WCHAR wcsEventMsg[EVENT_LENGTH];
        WCHAR wcsEventData[EVENT_DATA_LENGTH];

        while (diagEnabled)
        {
            memset(wcsEventMsg, 0, sizeof(wcsEventMsg));
            memset(wcsEventData, 0, sizeof(wcsEventData));

            while (pDiagsManager->GetDiagnosticEvent(&isError, wcsEventMsg, EVENT_LENGTH, wcsEventData, EVENT_DATA_LENGTH))
            {
                wstring currentTrace = wcsEventMsg;
                //Heuristics
                if( (currentTrace.find(L"CreateChunkDownloader") != wstring::npos)
                    || (currentTrace.find(L"UpdateHeuristics") != wstring::npos)
                    || (currentTrace.find(L"StartChunkRequest") != wstring::npos)
                    || (currentTrace.find(L"EndChunkRequest") != wstring::npos)
                    || (currentTrace.find(L"DestroyChunkDownloader")!= wstring::npos) )
                {
                    currentTrace.append(L"     ");
                    currentTrace.append(wcsEventData);
                    currentTrace.append(L"\n");
                    stObject->AddDiagTrace(currentTrace);
                }
                //Manifest
                if( (currentTrace.find(L"StreamInManifest") != wstring::npos))
                {
                    stObject->AddStreamForFragInfoMonitoring(GetIntFromDiagEventString(L"SID", wcsEventData));
                }

                //FragInfo
                if( (currentTrace.find(L"StartFragInfoRequest") != wstring::npos))
                {
                    SSPKChunkInfo chunkInfo = GetChunkInfoFromDiagEventsString(wcsEventData);
                    stObject->AddFragInfoToMonitoredStream(chunkInfo._streamId, chunkInfo);
                }

            }

            //If no diag event, sleep for a while and try again
            Executive_Sleep(100);
        }
    }

    return 0;
}

void STWrapper::RegisterDiagEvents(vector<DiagsChannel> channelVector, DiagsPriority priority)
{
    _diagManager = IAVManager::Instance()->GetDiagsManager();
    _hDiagThread = 0;
    diagEnabled = true;

    if (_diagManager)
    {
        for (size_t i =0; i< channelVector.size(); i ++)
        {
            DiagSetChannelPriority( channelVector[i], priority );
        }

        Executive_CreateThread(DiagEventRetrieveFun, this, 0, &_hDiagThread);

        // only turn on diagnostics if the thread is created
        if ( _hDiagThread )
        {
            _diagManager->RegisterFilters(true);
            _completeTraces = "";
        }
    }
}

void STWrapper::AddDiagTrace(wstring currentTrace)
{
    _completeTraces.append(WStr2Str(currentTrace));
}

string STWrapper::GetCompleteTraces()
{
    return _completeTraces;
}

IDiagsManager* STWrapper::GetDiagManager()
{
    return _diagManager;
}

void STWrapper::OnTick()
{
    if(NULL!=_SmoothTransportPtr)
    {
        int64_t timeSinceLastTick = Executive_GetTickCount() - _lastTickTimeStamp;
        int64_t _position0 = _lastPosition;
        _lastPosition = GetCurrentPlayBackTime();

        double positionMoved = ((double)_lastPosition / TIMESCALE_10MHZ) - ((double)_position0 / TIMESCALE_10MHZ);
        _mainActualPlaybackRate = (positionMoved * TICKS_PER_SEC)/ timeSinceLastTick;
        LogTestComment("LastPosition : %lld(%.3f) LastTickTimeStamp : %lld positionMoved: %.3f timeSinceLastTick : %lld  _ActualPlaybackrate : %.3f",_lastPosition,(double)_lastPosition / TIMESCALE_10MHZ,_lastTickTimeStamp,positionMoved,timeSinceLastTick,_mainActualPlaybackRate);

        {
            AutoLock lock(&_lock);
            if( _mainActualPlaybackRateList.size() > 10 )
            {
                _mainActualPlaybackRateList.erase( _mainActualPlaybackRateList.begin() );
            }
            _mainActualPlaybackRateList.push_back(_mainActualPlaybackRate);
        }
        _lastTickTimeStamp = Executive_GetTickCount();
    }
}


bool STWrapper::WaitForManifestReady(uint32_t dwMsTimeout)
{
    return _sink.Wait(dwMsTimeout);
}

bool STWrapper::WaitForMediaRenderingStarted(uint32_t dwMsTimeout = INFINITE)
{
    CEvent::EWaitResult hr = _mediaRenderingStartedEvent->Wait(dwMsTimeout);
    return (hr == CEvent::eWaitSignaled)? true : false;
}

void STWrapper::StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs)
{
    _streamSelectionCount++;
    if (SUCCEEDED(pEventArgs->Result))
    {
        streamSelectionArgs = pEventArgs->StreamChanges;
        for (size_t i = 0; i < pEventArgs->StreamChanges.size(); ++i)
        {
            LogTestComment("Successfully %s stream %ls",
                pEventArgs->StreamChanges[i].Action == StreamChangedEventArgs::StreamSelected ? "selected" : "deselected",
                pEventArgs->StreamChanges[i].pStream->Name().c_str());
        }
    }
    else
    {
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED(pEventArgs->Result), "Streams Selected Callback returned a failure.");
    }

    _selectedStreamsEvent->Set();

exit:
    return;
}

bool STWrapper::WaitForSelectedStreams(uint32_t dwMsTimeout = INFINITE)
{
    CEvent::EWaitResult hr = _selectedStreamsEvent->Wait(dwMsTimeout);
    return (hr == CEvent::eWaitSignaled)? true : false;
}

bool STWrapper::WaitForMediaFailed(uint32_t playBackTimeout = INFINITE)
{
    CEvent::EWaitResult hr = _mediaFailedEvent->Wait(playBackTimeout);
    return (hr == CEvent::eWaitSignaled)? true : false;
}

bool STWrapper::WaitForMediaEnded(uint32_t playbackTimeout = INFINITE)
{
    CEvent::EWaitResult hr = _mediaEndedEvent->Wait(playbackTimeout);
    return (hr == CEvent::eWaitSignaled)? true : false;
}

bool STWrapper::WaitForMediaFailedOrMediaEnded(uint32_t playbackTimeout = INFINITE)
{
    CEvent::EWaitResult hr = _mediaFailedOrMediaEndedEvent->Wait(playbackTimeout);
    return (hr == CEvent::eWaitSignaled)? true : false;
}

bool STWrapper::WaitForMediaRenderingAndEnded(bool isRenderingNeeded, uint32_t mediaEndedTimeout )
{
    bool mediaRendered = WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT);
    if(!mediaRendered)
    {
        if(isRenderingNeeded)
        {
            return false;
        }
        else
        {
            LogTestComment( "TimeOut Happened during Wait For rendering, but is rendering is not required, so not treating it failure" );
        }
    }
    
    return WaitForMediaEnded(mediaEndedTimeout);
}

bool STWrapper::WaitForAtWndwEdge(uint32_t playbackTimeout = INFINITE)
{
    CEvent::EWaitResult hr = _atWndwEdgeEvent->Wait(playbackTimeout);
    return (hr == CEvent::eWaitSignaled)? true : false;
}

void STWrapper::OnChunkInfo(_In_ pkRESULT hrResult, _In_ const CHUNK_INFO* pChunk )
{
    _chunkInfoDownloadResult = hrResult;
    if( SUCCEEDED( hrResult ) )
    {
        _chunkInfoTimestamp = pChunk->chunkTime;
    }
    _fragmentInfoDownloadEvent->Set();
}

pkRESULT STWrapper::WaitForFragmentInfoAsync(_In_ uint32_t dwMsTimeout, _Out_ int64_t& chunkInfo)
{
    CEvent::EWaitResult hr = _fragmentInfoDownloadEvent->Wait(dwMsTimeout);
    if( hr == CEvent::eWaitSignaled )
    {
        if( SUCCEEDED( _chunkInfoDownloadResult ) )
        {
            chunkInfo = _chunkInfoTimestamp;
        }
        _fragmentInfoDownloadEvent->Reset();
        return _chunkInfoDownloadResult;
    }
    else
    {
        return pkE_TIMEOUT;
    }
}

pkRESULT STWrapper::OnFragmentData(_In_ pkRESULT hrResult, _In_ CHUNK_INFO* pChunkInfo, _In_opt_ IRefBuffer* pBuffer, _In_ bool fFinalBuffer, _In_ size_t cbTotalLength )
{
    std::string chunkText;
    _chunkDownloadResult = hrResult;
    if( SUCCEEDED( hrResult ) )
    {
        BytesToHexStr(pBuffer->Data(), cbTotalLength , &chunkText );
        //LogTestComment(chunkText.c_str());

        //set chunkparams
        _isChunkComplete = fFinalBuffer;
        _chunkTimestamp = pChunkInfo->chunkTime;
        _chunkBufferDownloadedSize = pBuffer->Length();
        _totalChunkSize = cbTotalLength;
    }

    _fragmentDownloadEvent->Set();

    return S_OK;
}

pkRESULT STWrapper::WaitForDownloadFragmentAsync(_In_ uint32_t dwMsTimeout, _Out_ bool& isChunkFinished, _Out_ int64_t& downloadedBufferSize, _Out_ int64_t& chunkTime, _Out_ int64_t& totalChunkSize)
{
    CEvent::EWaitResult hr = _fragmentDownloadEvent->Wait(dwMsTimeout);
    if( hr == CEvent::eWaitSignaled )
    {
        if( SUCCEEDED( _chunkDownloadResult ) )
        {
            isChunkFinished = _isChunkComplete;
            chunkTime = _chunkTimestamp;
            downloadedBufferSize = _chunkBufferDownloadedSize;
            totalChunkSize = _totalChunkSize;
        }
        _fragmentDownloadEvent->Reset();
        return _chunkDownloadResult;
    }
    else
    {
        return pkE_TIMEOUT;
    }
}

void STWrapper::SetPlaybackRangeCallback(_In_ IManifest* pManifest, _In_ pkRESULT result)
{
    PKTEST_ASSERT_MSG_EXIT(_pManifest == pManifest, "OldManifest object and the new one should be the same");
    _pManifest.Set(pManifest);
    _startTime = TimeSpan_hns::ConvertFrom(TimeSpan_NTP::FromTicks(pManifest->StartTime())).Ticks();
    _endTime = TimeSpan_hns::ConvertFrom(TimeSpan_NTP::FromTicks(pManifest->StartTime())).Ticks() + pManifest->Duration();

    LogTestComment("Start position changed to %lld", _startTime);

exit:
    _setPlaybackRangeResult = result;
    _setPlaybackRangeCompletedEvent->Set();
}

pkRESULT STWrapper::WaitForSetPlaybackRangeCompleted(uint32_t setPlaybackRangeTimeout)
{
    CEvent::EWaitResult hr = _setPlaybackRangeCompletedEvent->Wait(setPlaybackRangeTimeout);
    if( hr == CEvent::eWaitSignaled )
    {
        _setPlaybackRangeCompletedEvent->Reset();
        return _setPlaybackRangeResult;
    }
    else
    {
        return pkE_TIMEOUT;
    }
}

void STWrapper::StatusCallback(SmoothTransportStatus& status)
{
    _startTime = TimeSpan_hns::ConvertFrom(status._startTime).Ticks();
    _endTime = TimeSpan_hns::ConvertFrom(status._endTime).Ticks();

    _currentStatus = status;
    {
        AutoLock lock(&_vectorLock);
        _smoothStatusCallbackVector.push_back(status);
    }

    // once we get out of playing state(pause is not really getting out of rendering) we reset the _mediaRenderingStartedEvent.
    if( SmoothTransportTunerState_Playing != status._currentState && SmoothTransportTunerState_Paused != status._currentState )
    {
        _mediaRenderingStartedEvent->Reset();
    }

    if (SmoothTransportStatus_BitrateChanged == status._update)
    {
        _latestBitrateKbps = (atoi(status._additionalInfo.c_str()) + 500)/ 1000;
    }
    else if (SmoothTransportStatus_TunerStateChanged == status._update)
    {
        switch (status._currentState)
        {
            case SmoothTransportTunerState_Unknown:
                                                _statusUpdatesList->Unknown++;
                                                break;
            case SmoothTransportTunerState_Tuning:
                                                _statusUpdatesList->Tuning++;
                                                break;
            case SmoothTransportTunerState_Playing:
                                                _statusUpdatesList->Playing++;
                                                break;
            case SmoothTransportTunerState_Paused:
                                                _statusUpdatesList->Paused++;
                                                break;
            case SmoothTransportTunerState_MediaEnded:
                                                _statusUpdatesList->MediaEnded++;
                                                break;
            case SmoothTransportTunerState_Detuned:
                                                _statusUpdatesList->Detuned++;
                                                break;
            case SmoothTransportTunerState_Closed:
                                                _statusUpdatesList->Closed++;
                                                break;
            case SmoothTransportTunerState_Max:
                                                _statusUpdatesList->Max++;
                                                break;
        }
        //Do a Reset in playstates for all the tuner statechanges except when we go to media ended.
        if( status._currentState != SmoothTransportTunerState_MediaEnded)
        {
            ResetPlayRateList();
        }
        else 
        {
            _mediaEndedEvent->Set();
            _mediaFailedOrMediaEndedEvent->Set();
        }
    }
    else if ( SmoothTransportStatus_Rendering == status._update)
    {
        _mediaRenderingStartedEvent->Set();
        ResetPlayRateList();
        _lastPosition = TimeSpan_hns::ConvertFrom(status._currentTime).Ticks();
    }
    else if( SmoothTransportStatus_AtWindowEdge == status._update )
    {
        _atWndwEdgeEvent->Set();
    }
    
    int32_t startTimeSecs = (int32_t)status._startTime.ToSeconds() % TOTAL_SECONDS_IN_A_DAY;
    int32_t durationSecs = (int32_t)( status._endTime - status._startTime ).ToSeconds() % TOTAL_SECONDS_IN_A_DAY;
/*
    LogTestComment("[%.3f] Status: %s (%s) spd:%.2f [%.3f, %.3f, %.3f] (%.3f) [%02d:%02d:%02d (%02d:%02d)] kbps: %d pkResult: 0x%08x httpResponse: %d additionalInfo:%s",
            (double)Executive_GetTickCount() / TICKS_PER_SEC,
            GetTunerStateName(status._currentState),
            GetStatusUpdateName(status._update),
            status._speed,
            status._startTime.ToSeconds(),
            status._currentTime.ToSeconds(),
            status._endTime.ToSeconds(),
            ( status._currentTime > status._startTime ) ? ( status._currentTime - status._startTime ).ToSeconds() : 0,
            (startTimeSecs / 3600),      // hours
            (startTimeSecs % 3600) / 60, // minutes
            (startTimeSecs % 60),        // seconds
            (durationSecs / 60),         // duration minutes
            (durationSecs % 60),         // duration seconds
            _latestBitrateKbps,
            status._pkResult,
            status._httpResponse,
            status._additionalInfo.c_str() );
*/            
}

const char* STWrapper::GetTunerStateName( ESmoothTransportState smoothTransportState)
{
    switch (smoothTransportState)
    {
        case SmoothTransportTunerState_Unknown:     return "Unknown  ";
        case SmoothTransportTunerState_Tuning:      return "Tuning   ";
        case SmoothTransportTunerState_Playing:     return "Playing  ";
        case SmoothTransportTunerState_Paused:      return "Paused   ";
        case SmoothTransportTunerState_MediaEnded:  return "MediaEnded";
        case SmoothTransportTunerState_Detuned:     return "Detuned  ";
        case SmoothTransportTunerState_Closed:      return "Closed   ";
        case SmoothTransportTunerState_Max:         return "Max";
    }
    return "*invalid*";
}

const char* STWrapper::GetStatusUpdateName( ESmoothTransportStatusUpdate smoothTransportStatusUpdate)
{
    switch (smoothTransportStatusUpdate)
    {
        case SmoothTransportStatus_Heartbeat:               return "Heartbeat   ";
        case SmoothTransportStatus_TunerStateChanged:       return "TunerState  ";
        case SmoothTransportStatus_Streaming:               return "Streaming   ";
        case SmoothTransportStatus_Rendering:               return "Rendering   ";
        case SmoothTransportStatus_Underrun:                return "Underrun    ";
        case SmoothTransportStatus_Rebuffer:                return "Rebuffer    ";
        case SmoothTransportStatus_StartEndTime:            return "StartEndTime";
        case SmoothTransportStatus_DrmStateChanged:         return "DrmState    ";
        case SmoothTransportStatus_BitrateChanged:          return "BPS change  ";
        case SmoothTransportStatus_DecoderError:            return "Decoder err  ";
        case SmoothTransportStatus_ChunkConnectHttpInvalid: return "ChunkConnHttp";
        case SmoothTransportStatus_NextChunkHttpInvalid:    return "NextChunkHttp";
        case SmoothTransportStatus_ChunkHdrHttpInvalid:     return "ChunkHdrHttp ";
        case SmoothTransportStatus_ChunkHdrError:           return "ChunkHdrErr  ";
        case SmoothTransportStatus_AtWindowEdge:            return "AtWndwEdge   ";
        case SmoothTransportStatus_EndOfLive:               return "EndOfLive    ";
        case SmoothTransportStatus_OutsideWindowEdge:       return "OutsideWndw  ";
        case SmoothTransportStatus_SegmentManifestError:    return "SegManfstErr ";
        case SmoothTransportStatus_DrmInitError:            return "DrmInitErr   ";
        case SmoothTransportStatus_Max:                     return "Max";
    }
    return "*invalid*";
}

void STWrapper::ErrorCallback(SmoothTransportError& errorInfo)
{
    _mediaFailed = true;
    _smoothErrorVector.push_back(errorInfo);
    LogTestComment("[%.3f] TransportError: %s,  pkResult: 0x%08x httpResponse: %d",
        (double)Executive_GetTickCount() / TICKS_PER_SEC,
        GetErrorName(errorInfo._errorCode),
        errorInfo._pkResult,
        errorInfo._httpResponse);

    if (_assertWhenMediaFailed)
    {
        PKTEST_ASSERT_MSG_EXIT(!_mediaFailed, "Media failed. Recieved a error callback with error %s.", GetErrorName(errorInfo._errorCode));
    }

exit:
    _mediaFailedEvent->Set();
    _mediaFailedOrMediaEndedEvent->Set();
    return;
}

const char* STWrapper::GetErrorName( ESmoothTransportError error)
{
    switch (error)
    {
        case SmoothTransportError_None                              : return "None";
        case SmoothTransportError_Unknown                           : return "Unknown";

        // tuner errors
        case SmoothTransportError_TunerAllocationFailure            : return "TunerAllocationFailure";
        case SmoothTransportError_TunerSharedReceivers              : return "TunerSharedReceivers";

        //Manifest errors
        case SmoothTransportError_ManifestParseFailed               : return "ManifestParseFailed";
        case SmoothTransportError_ManifestVersionUnsupported        : return "ManifestVersionUnsupported";
        case SmoothTransportError_ManifestInvalid                   : return "ManifestInvalid";
        case SmoothTransportError_ManifestHttpInvalidResult         : return "ManifestHttpInvalidResult";

        // Socket errors
        case SmoothTransportError_SocketAlreadyClosed               : return "SocketAlreadyClosed";
        case SmoothTransportError_SocketReadError                   : return "SocketReadError";
        case SmoothTransportError_SocketOpenFailed                  : return "SocketOpenFailed";
        case SmoothTransportError_SocketConnectFailed               : return "SocketConnectFailed";
        case SmoothTransportError_SocketSendFailed                  : return "SocketSendFailed";
        case SmoothTransportError_SocketRecvFailed                  : return "SocketRecvFailed";

        //HTTP errors
        case SmoothTransportError_HttpParseResponseFailed           : return "HttpParseResponseFailed";
        case SmoothTransportError_HttpInvalidResult                 : return "HttpInvalidResult";
        case SmoothTransportError_HttpTooManyRedirect               : return "HttpTooManyRedirect";
        case SmoothTransportError_HttpRedirectFailed                : return "HttpRedirectFailed";
        case SmoothTransportError_HttpRedirectNotAllowed            : return "HttpRedirectNotAllowed";
        case SmoothTransportError_HttpCreateFailed                  : return "HttpCreateFailed";

        //Chunk socket
        case SmoothTransportError_ChunkConnectHttpInvalidResult     : return "ChunkConnectHttpInvalidResult";
        case SmoothTransportError_ChunkNextHttpInvalidResult        : return "ChunkNextHttpInvalidResult";
        case SmoothTransportError_ChunkHdrParseFailed               : return "ChunkHdrParseFailed";
        case SmoothTransportError_ChunkInvalidData                  : return "ChunkInvalidData";

        //Drm
        case SmoothTransportError_DrmInitFailed                     : return "DrmInitFailed";
    }
    return "*invalid*";
}

pkRESULT STWrapper::SetManifestCallback(IManifestReadyCallback *pCallback)
{
    return _SmoothTransportPtr->SetManifestCallback(pCallback);
}

bool STWrapper::IsInTrickModeBeforeMediaEnded(bool isInRewind)
{
    static const char* logMessageHeader = "IsInFastForward:";
    if(isInRewind)
    {
        logMessageHeader = "IsInRewind:";
    }
    {
        AutoLock lock(&_vectorLock);
        bool gotLastTunerChange = false;
        for(int32_t index = _smoothStatusCallbackVector.size() - 1; index >= 0; index--)
        {
            if( _smoothStatusCallbackVector[index]._update == SmoothTransportStatus_TunerStateChanged )
            {
                if( (!isInRewind && _smoothStatusCallbackVector[index]._speed <= 1 ) || (isInRewind && _smoothStatusCallbackVector[index]._speed >= 1 ))
                {
                    LogTestComment("%s Expected speed is %s than %d but is :%.3f", logMessageHeader, isInRewind ? "Less": "Greater", isInRewind ? -1 : 1, _smoothStatusCallbackVector[index]._speed );
                    return false;
                }
                if(!gotLastTunerChange)
                {
                    if(_smoothStatusCallbackVector[index]._currentState != SmoothTransportTunerState_MediaEnded)
                    {
                        LogTestComment("%s Expected last tunerstate change event to be SmoothTransportTunerState_MediaEnded, but the actual value is %s", logMessageHeader, GetTunerStateName(_smoothStatusCallbackVector[index]._currentState));
                        return false;
                    }
                    gotLastTunerChange = true;
                }
                else
                {
                    if(_smoothStatusCallbackVector[index]._currentState != SmoothTransportTunerState_Playing)
                    {
                        LogTestComment("%s Expected state before the mediaEnded to be SmoothTransportTunerState_Playing, but the actual value is %s", logMessageHeader, GetTunerStateName(_smoothStatusCallbackVector[index]._currentState));
                        return false;
                    }
                    break;
                }
            }
        }
    }
    // This delay is to make sure we have at least 2 tick Iterations after mediaEnded.
    Delay(MILLISECONDS_PER_SECOND);
    {
        AutoLock lock(&_lock);
        if(_mainActualPlaybackRateList.size() > 0)
        {
            bool gotLastSpeed = false;
            bool gotLastButOneSpeed = false;
            for (int32_t listIndex = _mainActualPlaybackRateList.size() - 1; listIndex >= 0; listIndex--)
            {
                if(!gotLastSpeed)
                {
                    if(_mainActualPlaybackRateList[listIndex] != 0)
                    {
                        LogTestComment("%s Expected speed of last tick entry is 0, but the actual value is %.3f", logMessageHeader, _mainActualPlaybackRateList[listIndex]);
                        return false;
                    }
                    gotLastSpeed = true;
                }
                else
                {
                    if(!gotLastButOneSpeed)
                    {
                        if(_mainActualPlaybackRateList[listIndex] != 0)
                        {
                            if( (isInRewind && (_mainActualPlaybackRateList[listIndex] >= 0 ) ) || ( !isInRewind && (_mainActualPlaybackRateList[listIndex] <= 0) ))
                            {
                                LogTestComment("%s Expected first Non zero speed from last is supposed to be %s than 0 , but the actual value is %.3f", logMessageHeader, isInRewind ? "Less": "Greater", _mainActualPlaybackRateList[listIndex]);
                                return false;
                            }
                            gotLastButOneSpeed = true;
                        }
                    }
                    else
                    {
                        if( (isInRewind && (_mainActualPlaybackRateList[listIndex] > 0 ) ) || ( !isInRewind && (_mainActualPlaybackRateList[listIndex] < 0) ))
                        {
                            LogTestComment("%s Expected 2nd Non zero speed from last is supposed to be %s than 0 , but the actual value is %.3f", logMessageHeader, isInRewind ? "Less": "Greater", _mainActualPlaybackRateList[listIndex]);
                            return false;
                        }
                        break;
                    }
                }
            }
            return true;
        }
        else
        {
            LogTestComment("%s Expecting the size of the _mainActualPlaybackList to be greater than 0, but the actual value is %d", logMessageHeader, _mainActualPlaybackRateList.size() );
            return false;
        }
    }
}

bool STWrapper::IsInDifferentSpeed(bool isInRewind, bool checkPlaybackRate)
{
    static const char* logMessageHeader = "IsInFastForward";
    if(isInRewind)
    {
        logMessageHeader = "IsInRewind";
    }

    //TODO. Check for Media Opened.
    if(_currentStatus._currentState != SmoothTransportTunerState_Playing && _currentStatus._currentState != SmoothTransportTunerState_MediaEnded )
    {
        LogTestComment("%s:  Expected _currentStatus._currentState to be SmoothTransportTunerState_Playing, but the actual value is %s", logMessageHeader, GetTunerStateName(_currentStatus._currentState));
        return false;
    }

    if( _currentStatus._currentState == SmoothTransportTunerState_MediaEnded )
    {
        return IsInTrickModeBeforeMediaEnded(isInRewind);
    }

    if( ( isInRewind && _currentStatus._speed >= 1) || ( !isInRewind && _currentStatus._speed <= 1 ))
    {
        LogTestComment("%s:  Expected _currentStatus._speed should be %s than 1, but the actual value is %.2f", logMessageHeader, (isInRewind) ? "less" : "greater", _currentStatus._speed );
        return false;
    }

    if(!checkPlaybackRate)
    {
        return true;
    }
    //TODO.playback rate given from smoothTransport
    // Check the actual playback rate is close to speed in status.
    double averagePlaybackRate = 0;
    {
        AutoLock lock(&_lock);
        //Skipping the first element in the list, because we will have a big jump in the media position, when we do a seek or skip or playat.
        if (_mainActualPlaybackRateList.size() > 1)
        {
            for(int32_t i = _mainActualPlaybackRateList.size() -1 ; i > 0; --i)
            {
                averagePlaybackRate += (_mainActualPlaybackRateList[i]);
            }

            averagePlaybackRate /= (_mainActualPlaybackRateList.size() - 1);
        }
    }
    if(averagePlaybackRate > (_currentStatus._speed + TRICK_PLAY_RATE_THRESHOLD ) || averagePlaybackRate < (_currentStatus._speed - TRICK_PLAY_RATE_THRESHOLD))
    {
        if(isInRewind)
        {
            if(averagePlaybackRate < (_currentStatus._speed + ( -TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) ) ) || averagePlaybackRate > (_currentStatus._speed - ( -TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) )))
            {
                LogTestComment("%s: Reported average playback rate is %.2f, but should be(taking extra frame in to account) in the range (%.2f,%.2f)", logMessageHeader, averagePlaybackRate, _currentStatus._speed + ( -TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) ), _currentStatus._speed - ( -TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) ));
                return false;
            }
            else
            {
                LogTestWarning("%s: Reported average playback rate is %.2f, but is in the range (%.2f,%.2f)", logMessageHeader, averagePlaybackRate, _currentStatus._speed + TRICK_PLAY_RATE_THRESHOLD, _currentStatus._speed - TRICK_PLAY_RATE_THRESHOLD);
            }
        }
        else
        {    
           if(averagePlaybackRate > (_currentStatus._speed + ( TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) ) ) || averagePlaybackRate < (_currentStatus._speed - ( TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) )))
            {
                LogTestComment("%s: Reported average playback rate is %.2f, but should be(taking extra frame in to account) in the range (%.2f,%.2f)", logMessageHeader, averagePlaybackRate, _currentStatus._speed + ( TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) ), _currentStatus._speed - ( TRICK_PLAY_RATE_THRESHOLD + (_currentStatus._speed * TRICK_PLAY_ERROR_RATE) ));
                return false;
            }
            else
            {
                LogTestWarning("%s: Reported average playback rate is %.2f, but is in the range (%.2f,%.2f)", logMessageHeader, averagePlaybackRate, _currentStatus._speed + TRICK_PLAY_RATE_THRESHOLD, _currentStatus._speed - TRICK_PLAY_RATE_THRESHOLD);
            }
        }
    }
    return true;
}

void STWrapper::SetIsLive(bool isLive)
{
    _isLive = isLive;
}

bool STWrapper::IsLive()
{
    return _isLive;
}

int64_t STWrapper::GetNonAccurateSeekThreshold()
{
    return _nonAccurateSeekThreshold;
}

uint32 STWrapper::GetStatusCallbackCount(ESmoothTransportStatusUpdate updateType)
{
    uint32_t count = 0;
    AutoLock lock(&_vectorLock);

    for(vector<SmoothTransportStatus>::const_iterator curr = _smoothStatusCallbackVector.begin(); curr != _smoothStatusCallbackVector.end(); ++curr)
    {
        if(curr->_update == updateType)
        {
            ++count;
        }
    }

    return count;
}

MapOfChunkInfoVectors STWrapper::GetFragInfos()
{
    AutoLock lock(&_fragmentInfoLock);
    return _smoothChunksByStream;
}

void STWrapper::AddStreamForFragInfoMonitoring(uint32_t streamID)
{
    AutoLock lock(&_fragmentInfoLock);
    vector<SSPKChunkInfo> chunkList;
    _smoothChunksByStream[streamID] = chunkList;
}

void STWrapper::AddFragInfoToMonitoredStream(uint32_t streamID, SSPKChunkInfo fragInfo)
{
    AutoLock lock(&_fragmentInfoLock);

    if(_smoothChunksByStream.end() != _smoothChunksByStream.find(streamID))
    {
        _smoothChunksByStream[streamID].push_back(fragInfo);
    }
    else
    {
        LogTestComment("Tried to add fragment info from unrecognized stream %d", streamID);
        PKTEST_ASSERT_EXIT( false );
    }

exit:
    return;
}

void STWrapper::SetNonAccurateSeekThreshold(AutoRefPtr<IManifest> manifest )
{
        VectorOfStreams selectedStreams;
        AutoRefPtr<IManifestStream> chunkStream;

        ChunkIterator itChunk;
        CHUNK_INFO chunkInfo;
        pkRESULT resultNext = pkS_OK, hr;

        int32_t chunkCount = 0;
        int64_t lastChunkTime = 0;
        double accumulatedChunkDuration = 0;

        if(manifest)
        {
            manifest->GetSelectedStreams(&selectedStreams);
        }
        else
        {
            if(!_pManifest)
            {
                LogTestComment("Tried to set the non accurate seek threshold but there was no manifest.  Using default value %lld", _nonAccurateSeekThreshold);
                return;
            }
            _pManifest->GetSelectedStreams(&selectedStreams);
        }

        //Use the primary stream which is the selected VideoStream.
        for(uint32_t i = 0; i < selectedStreams.size(); ++i)
        {
            if(MediaStreamTypeVideo == selectedStreams[i]->Type())
            {
                chunkStream = selectedStreams[i];
                break;
            }
        }

        //Get average distance between a set number of chunks
        itChunk = chunkStream->GetFirstInCurrentChunkList();

        for(; pkSUCCEEDED(resultNext) && chunkCount < MAX_CHUNK_DURATION_SAMPLE_SIZE; resultNext = itChunk.MoveNext(), ++chunkCount)
        {
            hr = chunkStream->TryGetChunkInfo( itChunk, &chunkInfo );
            if( pkFAILED(hr) && chunkCount < MIN_CHUNK_DURATION_SAMPLE_SIZE )
            {
                LogTestComment("Tried to set the non accurate seek threshold, but could not get chunk #%d to determine it.  Using default value %lld", chunkCount, _nonAccurateSeekThreshold);
                return;
            }

            //Accumulate durations
            if(chunkCount > 0)
            {
                accumulatedChunkDuration += chunkInfo.chunkTime - lastChunkTime;
            }
            lastChunkTime = chunkInfo.chunkTime;
        }

        _nonAccurateSeekThreshold = (int64_t) ceil(accumulatedChunkDuration / (chunkCount - 1)) * 2;
        if(_nonAccurateSeekThreshold > NON_ACCURATE_SEEK_THRESHOLD)
        {
            LogTestComment("Successfully set non accurate seek threshold using the first %d chunks. Value is %lld (%.3f seconds)", chunkCount, _nonAccurateSeekThreshold, TimeSpan_hns::FromTicks(_nonAccurateSeekThreshold).ToSeconds() );
        }
        else
        {
            LogTestComment("Tried to set non accurate seek threshold using the first %d chunks, but the value (%lld, %.3f seconds), was less than the default.  Setting to %.3f seconds", chunkCount, _nonAccurateSeekThreshold, TimeSpan_hns::FromTicks(_nonAccurateSeekThreshold).ToSeconds(), TimeSpan_hns::FromTicks(NON_ACCURATE_SEEK_THRESHOLD).ToSeconds() );
            _nonAccurateSeekThreshold = NON_ACCURATE_SEEK_THRESHOLD;
        }
        
}

//////////////////////////////////////////////////////////////
void STWrapper::ManifestReadyPassThrough(IManifest* pManifest, pkRESULT hr)
{
    if(!_isManifestReadyCallbackRaised)
    {
        _startTime = TimeSpan_hns::ConvertFrom(TimeSpan_NTP::FromTicks(pManifest->StartTime())).Ticks();
        _endTime = TimeSpan_hns::ConvertFrom(TimeSpan_NTP::FromTicks(pManifest->StartTime())).Ticks() + pManifest->Duration();

        _pManifest.Set(pManifest);
        _isLive = _pManifest->IsLive();
        SetNonAccurateSeekThreshold(_pManifest);
        _isManifestReadyCallbackRaised = true;
    }
}

//////////////////////////////////////////////////////////////
void GetValueFromDiagEventString(wstring field, wstring eventData, wstring& outValue)
{
    outValue = eventData.substr(eventData.find(field + L":"));
    outValue = outValue.substr(field.length() + 1 , outValue.find(L"[||]") - (field.length() + 1)); //+1 for the colon
}

int32_t GetIntFromDiagEventString(wstring field, wstring eventData)
{
    wstring intString;
    GetValueFromDiagEventString(field, eventData, intString);
    return toInt(WStr2Str(intString));
}

SSPKChunkInfo GetChunkInfoFromDiagEventsString(wstring eventData)
{
    SSPKChunkInfo chunkInfo;

    chunkInfo._streamId = GetIntFromDiagEventString(L"SID", eventData);
    chunkInfo._bitrate = GetIntFromDiagEventString(L"qualityLevelBitrate", eventData);
    chunkInfo._chunkIndex = GetIntFromDiagEventString(L"chunkIndex", eventData);
    wstring chunkUrl;
    GetValueFromDiagEventString(L"chunkUrl", eventData, chunkUrl);
    chunkInfo._chunkUrl = WStr2Str(chunkUrl);

    return chunkInfo;
}
