///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

// ===============================================================================================================
// Global configuration parameters controlling clock behavior
// ===============================================================================================================

class CReceiverConfiguration
{
public:
    CReceiverConfiguration();

    //Process commands
    bool        Command(const std::string& command, const std::vector<std::string>& args);

public:
    //Amount of time to wait for signal loss detection
    uint32      SignalLostTime;

    //Default timeout for how long to hold last VPS PIL code
    //A value of zero means ignore this and keep the current behavior
    uint32      LastPILTimeout;

    //Maximum period between PtsDelta captures
    uint32      PtsDeltaPeriodMS;
    //PTS delta diagnostics event is sent when either of audio or video
    //buffer status changes by this much in milliseconds.
    int32       PtsDeltaThreshold;
    //PTS delta event is sent when the audio and video pts received
    //over network changes by more than this time in milliseconds.
    int32       AVOffThreshold;

    //Whether VPS is enabled
    bool        EnableVPS;
};

// ===============================================================================================================
// ===============================================================================================================

extern CReceiverConfiguration gReceiverConfiguration;

// ===============================================================================================================
// ===============================================================================================================
