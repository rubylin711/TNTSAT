///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAVManager.h"
#include "ITunePrepare.h"
#include "CSmoothTransportStatusHandler.h"

using namespace SSPK;

class CSmoothTuneParameters
{
public:
    const static int16 INVALID_SPEED = -1;

    CSmoothTuneParameters(IAVManager& avManager);
    ~CSmoothTuneParameters();

    void              Reset();
    bool              Init(_In_ const std::string& pipeId, _In_ const std::string& manifestUrl, _In_ ESmoothTransportProtocol protocol, _Out_ SmoothTransportStatusInternal* status);
    bool              Prepare();

    pkRESULT          RequestPlaybackRange( _In_ ISetPlaybackRangeCallback* pCallback, 
                                            _In_ TimeSpan_NTP minTime, 
                                            _In_ TimeSpan_NTP maxTime );

    void              SetStartTime(_In_ TimeSpan_NTP time);
    TimeSpan_NTP      GetStartTime() const;

    void              SetEndTime(_In_ TimeSpan_NTP time);
    TimeSpan_NTP      GetEndTime() const;

    void              SetMediaTime(_In_ TimeSpan_NTP time);
    TimeSpan_NTP      GetMediaTime() const;

    void              SetSpeed(_In_ float speed);
    float             GetSpeed() const;

    float             GetPreviousSpeed() const;

    std::string       GetTuneUrl(_In_ bool firstTune) const;

    bool              IsTrickMode() const;

    void              Skip(_In_ TimeSpan_NTP mediaTime, _In_ TimeSpan_NTP duration);

    TimeSpan_NTP      GetMediaEndedTime() const;
    void              SetMediaEndedTime();

    void              SetIsLive(_In_ bool isLive);
    void              SetPlaybackOverTaken();
    bool              GetIsLive() const;

    void              SetManifestCallback(_In_opt_ IManifestReadyCallback* pCallback = NULL);

private:
    IAVManager&       _avManager;
    std::string       _pipeId;
    std::string       _manifestUrl;
    std::string       _protocol;
    ITunePrepare*     _pTunePrepare;
    TunePrepareResult _prepareResult;
    TimeSpan_NTP      _startTime;
    TimeSpan_NTP      _endTime;
    TimeSpan_NTP      _mediaTime;
    float             _speed;
    float             _previousSpeed;
    TimeSpan_NTP      _mediaEndedTime;
    bool              _isLive;
    bool              _playbackOverTaken;
    IManifestReadyCallback* _pManifestCallback;
};

