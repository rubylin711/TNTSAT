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
#include "AutoLock.h"

// ===============================================================================================================
// Global configuration parameters controlling MBR socket behavior
// ===============================================================================================================

class CSocketMbrConfiguration
{
public:
    static const uint32 cDefaultRateControlOverhead = 130;              // by default, rate is controlled at 130% of max stream rate
    static const uint32 cMaxRateControlOverhead = 150;
    static const uint32 cMinRateControlOverhead = 100;

    static const uint32 cDefaultRateControlThreshold = 0;               // by default, rate control is applied always when enabled
    static const uint32 cMaxRateControlThreshold = 110;
    static const uint32 cMinRateControlThreshold = 0;

    static const uint32 cDefaultPipelineMultiplier = 2;                 // 2 times tcp window

    static const uint32 cDefaultRecvBlockSize = 16*1024;                // 16 KB read on the socket by default
    static const uint32 cMaxRecvBlockSize = 8*cDefaultRecvBlockSize;    // max 128 KB read on the socket

    static const uint32 cDefaultLivePlaybackOffsetSeconds = 7;          // default seconds playback is backed off from the value of the SSLiveBackOffSeconds when tuning to 'live'
    static const uint32 cDefaultSSLiveBackOffSeconds = 3;               // default seconds backed off from current live server time
    static const uint32 cDefaultLeftEdgeBufferSeconds = 6;              // default seconds after first fragment start position to start playing at when tuning to left edge

public:
    //Constructor
    CSocketMbrConfiguration();

    //Process commands
    bool        Command(const std::string& command, const std::vector<std::string>& args);

public:
    Lockable    valueLock;                      //Lock for dynamic values
    uint32      AVCpuLimit;                     //Maximum total CPU allowed before taking action in AVEngine (0 = disabled)
    bool        SSRateControl;                  //Smooth streaming rate control switch [off]
    uint32      SSPipelineMultiplier;           //Controls how we send out the next chunk request
    uint32      SSRateControlOverhead;          //Percentile, the actual rate is to be kept at stream rate plus this overhead
    uint32      SSRateControlThreshold;         //Percentile, rate control is applied only when actual rate is above stream rate plus this overhead
    uint32      SSLivePlaybackOffsetSeconds;    //Seconds playback is backed off from the value of the SSLiveBackOffSeconds when tuning to 'live'
    uint32      SSLiveBackOffSeconds;           //Seconds backed off from current live server time
    uint32      SSLeftEdgeBufferSeconds;        //Seconds after first fragment start position to start playing at when tuning to left edge
    std::string SSBandwidthReportHeader;        //HTTP fragment request header to use for reporting bandwidth measurements [empty->off]
};

// ===============================================================================================================
// ===============================================================================================================

extern CSocketMbrConfiguration gSocketMbrConfiguration;

// ===============================================================================================================
// ===============================================================================================================
