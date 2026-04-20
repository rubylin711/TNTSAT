///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSmoothTransport.h"
#include "CSmoothTransportStatusHandler.h"
#include "CDecoderConfiguration.h"
#include "CAVEngineConfiguration.h"
#include "CPipeId.h"
#include "CRendererState.h"
#include "StringUtils.h"
#include "Trace.h"

#include "IPTVDecoderHal.h"


using namespace std;

//#define SmoothTransport_SPEW
#if defined(SmoothTransport_SPEW)
#define SmoothTransport_MSG(x) TRACE(x)
#else
#define SmoothTransport_MSG(x)
#endif

ISmoothTransport* ISmoothTransport::CreateSmoothTransport()
{
    return NEW_NO_THROW CSmoothTransport(*IAVManager::Instance(), "FULLSCREEN");
}

ISmoothTransport* ISmoothTransport::CreateSmoothTransport(_In_ const std::string& pipeId)
{
    return NEW_NO_THROW CSmoothTransport(*IAVManager::Instance(), pipeId);
}

void ISmoothTransport::DestroySmoothTransport(_In_ ISmoothTransport* SmoothTransport)
{
    delete SmoothTransport;
}

CSmoothTransport::CSmoothTransport(IAVManager& avManager, const std::string& pipeId)
    : _pipeId(pipeId)
    , _handle(0)
    , _uniqueId(0)
    , _avManager(avManager)
    , _tunerSession(NULL)
    , _tunerSessionFactory(avManager.GetTunerSessionFactory())
    , _smoothTuneParameters(avManager)
    , _statusCallback(NULL)
    , _errorCallback(NULL)
    , _pManifestCallback(NULL)
    , _firstTune(false)
    , _transportOpen(false)
{
    ASSERT(_tunerSessionFactory);

    CSmoothTransportStatusHelper::Init();
}

CSmoothTransport::~CSmoothTransport()
{
    // check if tuner has been detuned before cleaning up
    if (SmoothTransportTunerState_Unknown != _status.GetStatus_CurrentState() )
    {
        Close();
    }

    // shutdown CMbrManifest before tuner session is destroyed
    _smoothTuneParameters.Reset();

    if (_tunerSession)
    {
        _tunerSessionFactory->DestroyTunerSession(_tunerSession);
        _tunerSession = NULL;
    }
}

pkRESULT CSmoothTransport::Init(void)
{
    AutoLock lock(&_transportLock);

    _status.Reset();
    _error.Reset();

    _currentPlaybackTime.ResetTicks();

    gDecoderConfiguration.SwitchPipeToOwnAudio(PipeId_StringToU32(_pipeId));

    if (!_tunerSession)
    {
        uint32 featuresEnabled = 0;

        //This is a FULLSCREEN pipe
        featuresEnabled |= CRendererState::eFeature_FullscreenMode;
        //We are rendering Av on this pipe
        featuresEnabled |= CRendererState::eFeature_RenderingPipe;
        //Node status is enabled
        //featuresEnabled |= CRendererState::eFeature_NodeStatus;
        //Periodic diags enabled
        featuresEnabled |= CRendererState::eFeature_DiagsProvider;
        //Closed captioning is enabled
        //featuresEnabled |= CRendererState::eFeature_ClosedCaptioning;
        //Teletext is disabled
        featuresEnabled |= CRendererState::eFeature_Teletext;
        //Send stream info/language update notifications
        //featuresEnabled |= CRendererState::eFeature_StreamNotification;
        //Splicing is enabled
        //featuresEnabled |= CRendererState::eFeature_Splicing;
        //Clock correction enabled
        //featuresEnabled |= (CRendererState::eFeature_ClockCorrection | CRendererState::eFeature_ClockValidation);
        //Macrovision/CGMS-A signalling done by decoders
        featuresEnabled |= CRendererState::eFeature_VideoOPL;
        //Playready OPL enabled for this pipe
        featuresEnabled |= CRendererState::eFeature_PlayreadyOPL;

        _tunerSession = _tunerSessionFactory->CreateTunerSession(this, _pipeId.c_str(), featuresEnabled, _handle);
    }

    if (!_tunerSession)
    {
        return pkE_OUTOFMEMORY;
    }

    return pkS_OK;
}

pkRESULT CSmoothTransport::Open(_In_ const string& tuneUrl, _In_ ESmoothTransportProtocol protocol, _In_ bool bAutoPlay)
{
    SmoothTransport_MSG(("CSmoothTransport::Open %s autoplay: %s", tuneUrl.c_str(), bAutoPlay ? "TRUE" : "FALSE"));
    AutoLock lock(&_transportLock);

	IPTV_HAL_Decoder_SetSeekFlag(false);//clear the seek flag

    // Initialize, if it hasn't been done already
    if (!_tunerSession)
    {
        pkRESULT pkResult = Init();
        if (pkFAILED(pkResult))
        {
            return pkResult;
        }
    }
    else
    {
        Close();
    }

    if (!_tunerSession)
    {
        return pkE_INVALID_REQUEST;
    }

    if( _smoothTuneParameters.Init(_pipeId, tuneUrl, protocol, &_status) )
    {
        _firstTune = true;
        _transportOpen = true;
        if (bAutoPlay)
        {
            Play();
        }
    }

    return pkS_OK;
}

pkRESULT CSmoothTransport::Close(void)
{
    SmoothTransport_MSG(("CSmoothTransport::Close"));

    AutoLock lock(&_transportLock);

    if( _transportOpen )
    {
        _transportOpen = false;

        if ( _tunerSession )
        {
            Detune(ITunerSession::eDetuneClose);
        }
        
        _smoothTuneParameters.Reset();
        _status.Reset();
        _error.Reset();

        _currentPlaybackTime.ResetTicks();
    }
    return pkS_OK;
}

pkRESULT CSmoothTransport::Play(_In_ float speed)
{
    SmoothTransport_MSG(("CSmoothTransport::Play(%f)", speed));

    AutoLock lock(&_transportLock);

    if ((!_transportOpen) || (!_tunerSession))
    {
        return pkE_INVALID_REQUEST;
    }

    if (!SpeedIsValid(speed))
    {
        return pkE_INVALIDARG;
    }

	IPTV_HAL_Decoder_SetSeekFlag(false);//clear the seek flag

    float curSpeed = _smoothTuneParameters.GetSpeed();
    float preSpeed = _smoothTuneParameters.GetPreviousSpeed();

    TimeSpan_s httpTimeout = TimeSpan_s::FromTicks( gAVEngineConfiguration.HttpResponseTimeout );

    // resume to 1x speed playback only if we were paused from 1x speed playback and
    // not live or the current time is not too close to the left edge

    if ( ( preSpeed == 1.0 && curSpeed == 0.0 && speed == 1.0 )
         && ( ( !_smoothTuneParameters.GetIsLive() )
         || ( _currentPlaybackTime > _smoothTuneParameters.GetStartTime() + TimeSpan_NTP::ConvertFrom( httpTimeout ) ) ) )
    {
        _smoothTuneParameters.SetSpeed(speed);
        _status.Resume();
        _tunerSession->Play();
        return pkS_OK;
    }
    else if (curSpeed != speed || SmoothTransportTunerState_MediaEnded == _status.GetStatus_CurrentState() )
    {
        Detune(ITunerSession::eDetuneNoClose);

        // set tune parameter for the new tune
        _smoothTuneParameters.SetSpeed(speed);
        _smoothTuneParameters.SetMediaTime(_currentPlaybackTime);

        // issue the new tune
        Tune();
        return pkS_OK;
    }

    return pkE_INVALID_REQUEST;
}

pkRESULT CSmoothTransport::Pause(void)
{
    SmoothTransport_MSG(("CSmoothTransport::Pause"));

    AutoLock lock(&_transportLock);

    if (!_transportOpen)
    {
        return pkE_INVALID_REQUEST;
    }

    float curSpeed = _smoothTuneParameters.GetSpeed();

    if ( (_tunerSession) && (curSpeed != 0.0) )
    {
        // remember the paused playback time to determine where to resume from,
        // this must be done before the speed is set in the case where the state is
        // mediaEnded, the current playback time is determined using the speed
        _currentPlaybackTime = GetCurrentPlaybackTime();

        _smoothTuneParameters.SetSpeed(0.0);

        // change the status to 'pause'
        _status.Pause();
        _tunerSession->Pause();
    }

    return pkS_OK;
}

// Extensible commands
pkRESULT CSmoothTransport::SendExtendedCommand(_In_ const std::string& command, _In_ const std::vector<std::string>& args, _In_ ECommandTarget target)
{
    AutoLock lock(&_transportLock);

    bool bHandled = false;

    if (_tunerSession)
    {
        bHandled = _tunerSession->Command(command, args);
    }

    if (!bHandled && CommandTarget_Global == target)
    {
        bHandled = _avManager.SetGlobalConfiguration(command, args);
    }

    return bHandled ? pkS_OK : pkE_FAIL;
}

pkRESULT CSmoothTransport::SendExtendedCommand(_In_ const char* command, _In_ int argc, _In_ const char* argv[], _In_ ECommandTarget target)
{
    string cmdString;
    vector<string> cmdArgs;

    ASSERT(command);
    cmdString = command;

    for (int i = 0; i < argc; i++)
    {
        cmdArgs.push_back(argv[i]);
    }
    return pkSUCCEEDED(SendExtendedCommand(cmdString, cmdArgs, target)) ? pkS_OK : pkE_FAIL;
}


pkRESULT CSmoothTransport::RegisterStatusCallback(_In_opt_ ISmoothTransportStatusSink* callback, _In_ bool bRegister)
{
    AutoLock lock(&_callbackLock);

    _statusCallback = bRegister ? callback : NULL;

    return pkS_OK;
}

pkRESULT CSmoothTransport::RegisterErrorCallback(_In_opt_ ISmoothTransportErrorSink* callback, _In_ bool bRegister)
{
    AutoLock lock(&_callbackLock);

    _errorCallback = bRegister ? callback : NULL;

    return pkS_OK;
}

pkRESULT CSmoothTransport::SetManifestCallback(_In_opt_ IManifestReadyCallback* pCallback)
{
    AutoLock lock(&_callbackLock);

    _pManifestCallback = pCallback;
    _smoothTuneParameters.SetManifestCallback(pCallback);

    return pkS_OK;
}

void CSmoothTransport::GetCurrentPlaybackTime( _Out_ TimeSpan_NTP* pTime )
{
    AutoLock lock(&_transportLock);

    // remember the latest time in case the clock is stopped
    // and the time is queried, it has the latest
    _currentPlaybackTime = GetCurrentPlaybackTime();
    *pTime = _currentPlaybackTime;
}

pkRESULT CSmoothTransport::PlayAt(_In_ TimeSpan_NTP timestamp, _In_ float speed)
{
    SmoothTransport_MSG(("CSmoothTransport::PlayAt(%lld, %f)", timestamp.Ticks(), speed));

    AutoLock lock(&_transportLock);

    if (!_tunerSession || !_transportOpen)
    {
        return pkE_INVALID_REQUEST;
    }

    if (!SpeedIsValid(speed))
    {
        return pkE_INVALIDARG;
    }

    Detune(ITunerSession::eDetuneNoClose);

    Executive_Sleep(500);//fix bug seek will tune resume play before detune release flush decoder
	
    _smoothTuneParameters.SetSpeed(speed);
    _smoothTuneParameters.SetMediaTime(timestamp);

    // store the media time so that current playback time
    // is properly reported when the clock is not yet started
    _currentPlaybackTime = _smoothTuneParameters.GetMediaTime();

    // don't treat PlayAt like a first tune since caller is requesting to play at
    // a specific timestamp
    _firstTune = false;

	IPTV_HAL_Decoder_SetSeekFlag(TRUE);//set the seek flag

    Tune();
    return pkS_OK;
}

// seek
pkRESULT CSmoothTransport::Seek(_In_ TimeSpan_NTP timestamp)
{
    SmoothTransport_MSG(("CSmoothTransport::Seek(%lld)", timestamp.Ticks()));

    AutoLock lock(&_transportLock);

    if (!_tunerSession || !_transportOpen)
    {
        return pkE_INVALID_REQUEST;
    }

    return PlayAt(timestamp, 1.0);
}

pkRESULT CSmoothTransport::Skip(_In_ TimeSpan_NTP duration)
{
    SmoothTransport_MSG(("CSmoothTransport::Skip(%lld)", duration.Ticks()));

    AutoLock lock(&_transportLock);

    if ( !_tunerSession 
      || !_transportOpen 
      || (_status.GetStatus_CurrentState() != SmoothTransportTunerState_Playing 
            && _status.GetStatus_CurrentState() != SmoothTransportTunerState_Paused))
    {
        return pkE_INVALID_REQUEST;
    }

    Detune(ITunerSession::eDetuneNoClose);

    _smoothTuneParameters.Skip(_currentPlaybackTime, duration);

    Tune();
    return pkS_OK;
}

pkRESULT CSmoothTransport::SetPlaybackRangeAsync(_In_ ISetPlaybackRangeCallback* pCallback, 
                                                 _In_ TimeSpan_NTP minTime, 
                                                 _In_ TimeSpan_NTP maxTime /* = TimeSpan_NTP::ConvertFrom( TimeSpan_s::FromTicks( MAX_TIME64 ) ) */ )
{
    SmoothTransport_MSG(("CSmoothTransport::SetPlaybackRangeAsync(%.3f, %.3f)", minTime.ToSeconds(), maxTime.ToSeconds() ));

    AutoLock lock(&_transportLock);

    bool clockStarted = false;
    TimeSpan_NTP currentTime = GetCurrentPlaybackTime(&clockStarted);

    if(clockStarted && minTime > currentTime)
    {
        TRACE_ERROR(("CSmoothTransport::SetPlaybackRangeAsync(%.3f, %.3f) failed currentTime (%.3f) is less than minTime",
            minTime.ToSeconds(), 
            maxTime.ToSeconds(),
            currentTime.ToSeconds()));

        return pkE_INVALID_REQUEST;
    }

    return _smoothTuneParameters.RequestPlaybackRange(pCallback, minTime, maxTime);
}

void CSmoothTransport::StatusCallback(_In_ const std::string& statusString, _In_opt_ bool shouldCallCallback /* = true */)
{
    map<string,string> args;
    nameValue(statusString, args);

    SmoothTransportStatus status = _status.OnStatusEvent(args, GetCurrentPlaybackTime(), _smoothTuneParameters);
    SmoothTransportError error = _error.OnStatusEvent(args);

    CSmoothTransportStatusHelper::DumpStatus(status, error);

    if (status._currentState == SmoothTransportTunerState_Detuned)
    {
        AutoLock lock(&_callbackLock);
        if (_errorCallback)
        {
            _errorCallback->ErrorCallback(error);
        }
    }
    else
    {
        AutoLock lock(&_callbackLock);
        if (_statusCallback)
        {
            if ( shouldCallCallback && ShouldReportStatus(status) )
            {
                _statusCallback->StatusCallback(status);
            }
        }
    }
}

void CSmoothTransport::Tune(void)
{
    AutoLock lock(&_transportLock);

    CHECK_ALLOC(_tunerSession);

    string url = _smoothTuneParameters.GetTuneUrl(_firstTune);
    _firstTune = false;

    // change the status to 'tuning'
    _status.Tune(_smoothTuneParameters.GetSpeed());

    // Leave access control unblocked
    _tunerSession->SetAccessControl(false, MIN_TIME64, MAX_TIME64);
    // New tune
    _uniqueId++;

    _tunerSession->Tune(url, 0, NULL);
}

void CSmoothTransport::Detune(_In_ ITunerSession::DetuneReason reason)
{
    AutoLock lock(&_transportLock);

    // remember the current playback time before detune
    _currentPlaybackTime = GetCurrentPlaybackTime();
    // detune
    _tunerSession->Detune(true, true, false, reason); //force socket to close, keep last frame in decoder, not a channel change
}

bool CSmoothTransport::SpeedIsValid(_In_ float speed)
{
    // Values between -1.0 and +1.0 are not supported
    if ( (-1.0 < speed) && (speed < 1.0) )
    {
        return false;
    }

    // Non-integer speeds are not supported
    if (((int32)speed) != speed)
    {
        return false;
    }

    return true;
}

TimeSpan_NTP CSmoothTransport::GetCurrentPlaybackTime(_Out_ bool* pClockStarted /* = NULL */)
{
    bool clockStarted = false;
    ESmoothTransportState state = _status.GetStatus_CurrentState(&clockStarted);
    TimeSpan_NTP currentPlaybackTime = _currentPlaybackTime;

    if(pClockStarted)
    {
        *pClockStarted = clockStarted;
    }

    if (state == SmoothTransportTunerState_MediaEnded)
    {
        currentPlaybackTime = _smoothTuneParameters.GetMediaEndedTime();

        SmoothTransport_MSG(("CSmoothTransport::GetCurrentPlaybackTime [mediaEnded] %.3f secs",
            currentPlaybackTime.ToSeconds() ));
    }
    else if (_tunerSession && clockStarted)
    {
        uint64 tunerPlaybackTime = _tunerSession->GetCurrentMediaTime(false);
        if (IS_VALID_TIME(tunerPlaybackTime))
        {
            currentPlaybackTime = TimeSpan_NTP::FromTicks( tunerPlaybackTime );
        }

        SmoothTransport_MSG(("CSmoothTransport::GetCurrentPlaybackTime [running] %.3f secs",
            currentPlaybackTime.ToSeconds() ));
    }
    else
    {
        SmoothTransport_MSG(("CSmoothTransport::GetCurrentPlaybackTime [other] %.3f secs",
            currentPlaybackTime.ToSeconds() ));
    }

    return currentPlaybackTime;
}

bool CSmoothTransport::ShouldReportStatus(_In_ const SmoothTransportStatus& status)
{
    // report if at the window edge during paused, ff, rew
    if ( status._update == SmoothTransportStatus_AtWindowEdge )
    {
        if( status._currentState != SmoothTransportTunerState_Paused )
        {
            return false;
        }
    }

    return true;
}
