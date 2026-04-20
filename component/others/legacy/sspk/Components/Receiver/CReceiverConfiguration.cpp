///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CReceiverConfiguration.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

// ===============================================================================================================
// Global configuration parameters controlling receiver behavior
// ===============================================================================================================

CReceiverConfiguration::CReceiverConfiguration()
{
    //Amount of time to wait for signal loss detection
    SignalLostTime = 0;

    //Default timeout for how long to hold last VPS PIL code
    //A value of zero means ignore this and keep the current behavior
    LastPILTimeout = 0;

    //Maximum period between PtsDelta captures
    PtsDeltaPeriodMS = 30000;
    //PTS delta diagnostics event is sent when either of audio or video
    //buffer status changes by this much in milliseconds.
    PtsDeltaThreshold = 500;
    //PTS delta event is sent when the audio and video pts received
    //over network changes by more than this time in milliseconds.
    AVOffThreshold = 1000;

    //Whether VPS is enabled
    EnableVPS = false;
}

//Process commands
bool CReceiverConfiguration::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    uint32 numargs = (uint32)args.size();

    //Amount of time to wait for signal loss detection
    if (command == "signallosttime")
    {
        if (numargs == 1)
        {
            SignalLostTime = atoi(args[0].c_str()) * 1000;
            TRACE(("SignalLostTime=%u", SignalLostTime));
        }
        return true;
    }

    //Default timeout for how long to hold last VPS PIL code
    //A value of zero means ignore this and keep the current behavior
    if (command == "lastpiltimeout")
    {
        if (numargs == 1)
        {
            LastPILTimeout = atoi(args[0].c_str());

            TRACE(("LastPILTimeout=%u", LastPILTimeout));
        }
        return true;
    }

    //Maximum period between PtsDelta captures
    if (command == "avptsdeltaperiodms")
    {
        if (numargs == 1)
        {
            PtsDeltaPeriodMS = atoi(args[0].c_str());

            TRACE(("PtsDeltaPeriodMS=%u", PtsDeltaPeriodMS));
        }
        return true;
    }

    //PTS delta diagnostics event is sent when either of audio or video
    //buffer status changes by this much in milliseconds.
    if (command == "avptsdeltathreshold")
    {
        if (numargs == 1)
        {
            PtsDeltaThreshold = atoi(args[0].c_str());

            TRACE(("PtsDeltaThreshold=%u", PtsDeltaThreshold));
        }
        return true;
    }

    //PTS delta event is sent when the audio and video pts received
    //over network changes by more than this time in milliseconds.
    if (command == "avoffthreshold")
    {
        if (numargs == 1)
        {
            AVOffThreshold = atoi(args[0].c_str());

            TRACE(("AVOffThreshold=%u", AVOffThreshold));
        }
        return true;
    }

    //Whether VPS is enabled
    if (command == "enablevps")
    {
        if (numargs == 1)
        {
            EnableVPS = toLower(args[0]) == "true";

            TRACE(("EnableVPS=%s", EnableVPS ? "true" : "false"));
        }
        return true;
    }

    return false;
}

// ===============================================================================================================
// ===============================================================================================================

CReceiverConfiguration gReceiverConfiguration;

// ===============================================================================================================
// ===============================================================================================================
