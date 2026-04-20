///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ITunerSession.h"
#include "IAVManager.h"
#include "CSocket.h"
#include "SmoothTuneParameters.h"
#include "CSmoothTransportStatusHandler.h"

using namespace SSPK;

class CSmoothTransport : public ISmoothTransport, public ITunerSessionCallback
{
public:
    CSmoothTransport(IAVManager& avManager, const std::string& pipeId);
    ~CSmoothTransport();

    // ISmoothTransport
    __override pkRESULT            Init(void);
    __override pkRESULT            Open(_In_ const std::string& tuneUrl, _In_ ESmoothTransportProtocol protocol, _In_ bool bAutoPlay = false);
    __override pkRESULT            Close(void);
    __override pkRESULT            Play(_In_ float speed = 1.0);
    __override pkRESULT            Pause(void);
    __override pkRESULT            SendExtendedCommand(_In_ const std::string& command, _In_ const std::vector<std::string>& args, _In_ ECommandTarget target = CommandTarget_Global);
    __override pkRESULT            SendExtendedCommand(_In_ const char* command, _In_ int argc, _In_ const char* argv[], _In_ ECommandTarget target = CommandTarget_Global);
    __override pkRESULT            RegisterStatusCallback(_In_opt_ ISmoothTransportStatusSink* callback, _In_ bool bRegister = true);
    __override pkRESULT            RegisterErrorCallback(_In_opt_ ISmoothTransportErrorSink* callback, _In_ bool bRegister = true);
    __override pkRESULT            SetManifestCallback(_In_opt_ IManifestReadyCallback* pCallback = NULL);
    __override pkRESULT            SetPlaybackRangeAsync(_In_ ISetPlaybackRangeCallback* pCallback, 
                                                         _In_ TimeSpan_NTP minTime, 
                                                         _In_ TimeSpan_NTP maxTime = TimeSpan_NTP::ConvertFrom( TimeSpan_s::FromTicks( MAX_TIME64 ) ) );

    //Trick functions
    __override void                GetCurrentPlaybackTime(_Out_ TimeSpan_NTP* pTime);
    __override pkRESULT            PlayAt(_In_ TimeSpan_NTP timestamp, _In_ float speed = 1.0);
    __override pkRESULT            Seek(_In_ TimeSpan_NTP timestamp);
    __override pkRESULT            Skip(_In_ TimeSpan_NTP duration);

    // ITunerSessionCallback
    __override void                StatusCallback(_In_ const std::string& status, _In_opt_ bool shouldCallCallback = true);

private:
    void                           Tune(void);
    void                           Detune(_In_ ITunerSession::DetuneReason reason);
    TimeSpan_NTP                   GetCurrentPlaybackTime(_Out_ bool* pClockStarted = NULL);
    bool                           ShouldReportStatus(_In_ const SmoothTransportStatus& status);
    bool                           SpeedIsValid(_In_ float speed);

    std::string                    _pipeId;
    uint32                         _handle;
    uint32                         _uniqueId;

    IAVManager&                    _avManager;

    ITunerSession*                 _tunerSession;
    ITunerSessionFactory*          _tunerSessionFactory;

    CSmoothTuneParameters           _smoothTuneParameters;
    SmoothTransportStatusInternal   _status;
    SmoothTransportErrorInternal    _error;

    ISmoothTransportStatusSink*     _statusCallback;
    ISmoothTransportErrorSink*      _errorCallback;
    IManifestReadyCallback*         _pManifestCallback;

    TimeSpan_NTP                    _currentPlaybackTime;

    Lockable                        _transportLock;
    Lockable                        _callbackLock;

    bool                            _firstTune;
    bool                            _transportOpen;
};
