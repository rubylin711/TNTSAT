///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SmoothTuneParameters.h"
#include "ITunePrepare.h"
#include "CPipeId.h"
#include "CClockConfiguration.h"
#include "CTuneRequest.h"
#include "Trace.h"
#include "StringUtils.h"

#include <string>
using namespace std;

CSmoothTuneParameters::CSmoothTuneParameters(IAVManager& avManager)
    : _avManager(avManager)
    , _pTunePrepare(NULL)
    , _speed(INVALID_SPEED)
    , _previousSpeed(INVALID_SPEED)
    , _isLive(false)
    , _playbackOverTaken(false)
    , _pManifestCallback(NULL)
{
    memset(&_prepareResult, 0, sizeof(TunePrepareResult));
}

CSmoothTuneParameters::~CSmoothTuneParameters()
{
    Reset();
}

void CSmoothTuneParameters::Reset()
{
    if (_pTunePrepare)
    {
        _avManager.GetTunePrepareFactory()->ReleaseTunePrepare(_pTunePrepare->GetToken());
        _pTunePrepare = NULL;
    }

    memset(&_prepareResult, 0, sizeof(TunePrepareResult));
    _pipeId = "";
    _manifestUrl = "";
    _protocol = "";

    _startTime.ResetTicks();
    _endTime.ResetTicks();
    _mediaEndedTime.ResetTicks();
    _mediaTime.ResetTicks();
    _isLive = false;
    _playbackOverTaken = false;
    _speed = INVALID_SPEED;
    _previousSpeed = INVALID_SPEED;
}

bool CSmoothTuneParameters::Init(_In_ const string& pipeId, _In_ const string& manifestUrl, _In_ ESmoothTransportProtocol protocol, _Out_ SmoothTransportStatusInternal* status)
{
    string strProtocol;
    if ( SmoothTransportProtocol_Mbr == protocol )
    {
        strProtocol = URL_PROTOCOL_MBR;
    }
    else
    {
        TRACE(("Invalid protocol %d", protocol));
        return false;
    }

    Reset();

    _pipeId = pipeId;
    _manifestUrl = manifestUrl;
    _protocol = strProtocol;

    if (!Prepare())
    {
        return false;
    }

    status->SetStartEndTime(_prepareResult.StartTime, _prepareResult.EndTime);

    return true;
}

bool CSmoothTuneParameters::Prepare()
{
    if (_pTunePrepare == NULL)
    {
        uint32 pipeIdN = PipeId_StringToU32(_pipeId);

        uint32 token = _avManager.GetTunePrepareFactory()->CreateTunePrepareObject(pipeIdN, GetTuneUrl(false).c_str());
        _pTunePrepare = _avManager.GetTunePrepareFactory()->GetTunePrepareInterface(token, false);
        ASSERT(_pTunePrepare);

        //Fail the tune if can't find the manifest socket object
        if (_pTunePrepare == NULL)
            return false;

        // Set the manifest callback if it is set before _pTunePrepare is created
        _pTunePrepare->SetManifestCallback(_pManifestCallback);

        //Download and prepare the manifest
        if (_pTunePrepare->Prepare() == false)
            return false;

        memset(&_prepareResult, 0, sizeof(TunePrepareResult));
        _pTunePrepare->GetPrepareResult(&_prepareResult);
    }

    SetStartTime( _prepareResult.StartTime );
    SetEndTime( _prepareResult.EndTime );
    SetMediaTime( _prepareResult.StartTime );
    SetIsLive(_prepareResult.IsLive);

    return true;
}

pkRESULT CSmoothTuneParameters::RequestPlaybackRange( _In_ ISetPlaybackRangeCallback* pCallback, 
                                                      _In_ TimeSpan_NTP minTime, 
                                                      _In_ TimeSpan_NTP maxTime )
{
    if (_pTunePrepare == NULL)
    {
        return pkE_INVALID_REQUEST;
    }

    return (_pTunePrepare->RequestPlaybackRange(pCallback, minTime, maxTime));
}

void CSmoothTuneParameters::SetStartTime(_In_ TimeSpan_NTP time)
{
    _startTime = time;
}

TimeSpan_NTP CSmoothTuneParameters::GetStartTime() const
{
    return _startTime;
}

void CSmoothTuneParameters::SetEndTime(_In_ TimeSpan_NTP time)
{
    _endTime = time;
}

TimeSpan_NTP CSmoothTuneParameters::GetEndTime() const
{
    return _endTime;
}

void CSmoothTuneParameters::SetMediaTime(_In_ TimeSpan_NTP time)
{
    // reset playback overtaken since new media time is a new tune
    _playbackOverTaken = false;

    if (time > _endTime)
    {
        _mediaTime = _endTime;
    }
    else if (time < _startTime)
    {
        _mediaTime = _startTime;
    }
    else
    {
        _mediaTime = time;
    }
}

TimeSpan_NTP CSmoothTuneParameters::GetMediaTime() const
{
    return _mediaTime;
}

void CSmoothTuneParameters::SetSpeed(_In_ float speed)
{
    _previousSpeed = _speed;
    _speed = speed;
}

float CSmoothTuneParameters::GetSpeed() const
{
    return _speed;
}

float CSmoothTuneParameters::GetPreviousSpeed() const
{
    return _previousSpeed;
}

string CSmoothTuneParameters::GetTuneUrl(_In_ bool firstTune) const
{
    string url = _protocol + ":///?src=" + escape(_manifestUrl);

    string speedStr = floatToString(_speed);

    if (IsTrickMode())
    {
        url += "&isimmediate=true&trk=" + speedStr;
    }
    else
    {
        url += string("&trk=0");
    }

    url += "&speed=" + speedStr;

    string floorItStr = _speed < 0 ? "true" : "false";

    url += "&floorit=" + floorItStr;

    url += "&rap=" + uint64toString64(_mediaTime.Ticks(), false);

    if (_prepareResult.IsLive && firstTune)
    {
        url += "&tunetolive=true";
    }

    if (_prepareResult.Bitrate)
    {
        string bitRateStr = toString(_prepareResult.Bitrate);
        url += "&r=" + bitRateStr;
    }

    // Set clock mode to NTP for MBR
    url += "&ctm=" + toString((int) eClockTimingMode_NTPUnit);

    if (_pTunePrepare)
        url += "&tune-prepare-handle=" + uint32ToString(_pTunePrepare->GetToken());

    return url;
}

bool CSmoothTuneParameters::IsTrickMode() const
{
    return (_speed != 0.0 && _speed != 1.0);
}

void CSmoothTuneParameters::Skip(
    _In_ TimeSpan_NTP mediaTime,
    _In_ TimeSpan_NTP duration )
{
    TimeSpan_NTP newMediaTime = mediaTime + duration;

    // check for over/under flow
    if( newMediaTime.Ticks() < 0)
    {
        if(duration.Ticks() > 0)
        {
            newMediaTime = TimeSpan_NTP::ConvertFrom( TimeSpan_s::FromTicks( MAX_TIME64 ) );
        }
        else
        {
            newMediaTime.ResetTicks();
        }
    }

    SetMediaTime( newMediaTime );

    if (_speed == 0.0)
    {
        SetSpeed(1.0);
    }
}

void CSmoothTuneParameters::SetMediaEndedTime()
{
    if (_speed >= 0.0 && !_playbackOverTaken)
    {
        _mediaEndedTime = _endTime;
    }
    else
    {
        _mediaEndedTime = _startTime;
    }
}

TimeSpan_NTP CSmoothTuneParameters::GetMediaEndedTime() const
{
    return _mediaEndedTime;
}

void CSmoothTuneParameters::SetIsLive(_In_ bool isLive)
{
    _isLive = isLive;
}

void CSmoothTuneParameters::SetPlaybackOverTaken()
{
    _playbackOverTaken = true;
}

bool CSmoothTuneParameters::GetIsLive() const
{
    return _isLive;
}

void CSmoothTuneParameters::SetManifestCallback(_In_opt_ IManifestReadyCallback* pCallback)
{
    _pManifestCallback = pCallback;
    if (NULL != _pTunePrepare)
    {
        _pTunePrepare->SetManifestCallback(pCallback);
    }
}
