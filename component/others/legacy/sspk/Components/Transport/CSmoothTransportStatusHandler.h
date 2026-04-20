///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ISmoothTransport.h"
#include "AutoLock.h"

#include <string>
#include <map>

typedef std::map<std::string,std::string> ArgsMap;

using namespace SSPK;
class CSmoothTuneParameters;

class SmoothTransportStatusInternal
{
public:
    SmoothTransportStatusInternal();
    void Reset();
    void Tune(float speed);
    void Pause();
    void Resume();
    void SetStartEndTime( TimeSpan_NTP startTime, TimeSpan_NTP endTime );
    SmoothTransportStatus OnStatusEvent(ArgsMap& args, TimeSpan_NTP currentTime, CSmoothTuneParameters& smoothTuneParameters);
    ESmoothTransportState GetStatus_CurrentState(_Out_opt_ bool* pClockStarted = NULL);

private:
    SmoothTransportStatus _status;
    Lockable             _lock;
};

class SmoothTransportErrorInternal
{
public:
    SmoothTransportErrorInternal();
    void Reset();
    SmoothTransportError OnStatusEvent(ArgsMap& args);

private:
    ESmoothTransportError MapError(eTunerError tunerError, eSocketError socketError);
    SmoothTransportError  _errorInfo;
    Lockable             _lock;
};

class CSmoothTransportStatusHelper
{
public:
    static std::map<std::string, ESmoothTransportState> _stateArgMap;
    static std::map<std::string, ESmoothTransportStatusUpdate> _statusArgMap;

    static void Init();
    static void DumpStatus(const SmoothTransportStatus& status, const SmoothTransportError& error);
};

