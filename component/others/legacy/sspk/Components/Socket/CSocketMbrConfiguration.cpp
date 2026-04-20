///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSocketMbrConfiguration.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <vector>
#include "pkExecutive.h"
#include "AutoLock.h"

using namespace std;

// ===============================================================================================================
// Global configuration parameters controlling MBR socket behavior
// ===============================================================================================================

CSocketMbrConfiguration::CSocketMbrConfiguration()
{
    //Maximum total CPU allowed before taking action in AVEngine (0 = disabled)
    AVCpuLimit = 0;

    //Smooth streaming pipeline multiplier
    SSPipelineMultiplier = cDefaultPipelineMultiplier;

    //Smooth streaming rate control switch
    //Off by default
    SSRateControl = false;

    //Rate control parameters
    SSRateControlOverhead = cDefaultRateControlOverhead;
    SSRateControlThreshold = cDefaultRateControlThreshold;

    //Seconds backed off from current live server time
    SSLiveBackOffSeconds = cDefaultSSLiveBackOffSeconds;

    //Seconds playback is backed off from the value of the SSLiveBackOffSeconds when tuning to 'live'
    SSLivePlaybackOffsetSeconds = cDefaultLivePlaybackOffsetSeconds;

    //Seconds after first chunk to start playing at when tuning to left edge
    SSLeftEdgeBufferSeconds = cDefaultLeftEdgeBufferSeconds;
}

//Process commands
bool CSocketMbrConfiguration::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    uint32 numargs = (uint32)args.size();

    //Maximum total CPU allowed before taking action in AVEngine (0 = disabled)
    if (command == "avcpulimit")
    {
        if (numargs == 1)
        {
            AVCpuLimit = atoi(args[0].c_str());
            TRACE(("AVCpuLimit=%u", AVCpuLimit));
        }
        return true;
    }

    //Smooth streaming rate control switch
    if (command == "ssratecontrol")
    {
        if (numargs == 1)
        {
            SSRateControl = toLower(args[0]) == "true";
            TRACE(("SSRateControl=%s", SSRateControl ? "true" : "false"));
        }
        return true;
    }

    //Smooth streaming pipeline multiplier
    if (command == "sspipeline")
    {
        if (numargs == 1)
        {
            uint32 pipeline = atoi(args[0].c_str());

            if (pipeline >= cDefaultPipelineMultiplier)
            {
                SSPipelineMultiplier = pipeline;
                TRACE(("SSPipelineMultiplier=%d", SSPipelineMultiplier));
            }
        }
        return true;
    }

    //Smooth streaming rate control upper threshold
    if (command == "ssratecontrolupperthreshold")
    {
        if (numargs == 1)
        {
            uint32 threshold = atoi(args[0].c_str());

            if (threshold >= cMinRateControlOverhead && threshold <= cMaxRateControlOverhead)
            {
                SSRateControlOverhead = threshold;
                TRACE(("SSRateControlOverhead=%d", SSRateControlOverhead));
            }
        }
        return true;
    }

    //Smooth streaming rate control lower threshold
    if (command == "ssratecontrollowerthreshold")
    {
        if (numargs == 1)
        {
            uint32 threshold = atoi(args[0].c_str());

            // We used to check threshold >= cMinRateControlThreshold, don't bother, both are unsigned so this is always true
            pkCT_ASSERT(0 == cMinRateControlThreshold);           // If this fails, need to check threshold >= cMinRateControlThreshold
            pkCT_ASSERT_TYPEISUNSIGNED(cMinRateControlThreshold); // If this fails, need to check threshold >= cMinRateControlThreshold
            pkCT_ASSERT_TYPEISUNSIGNED(threshold);                // If this fails, need to check threshold >= cMinRateControlThreshold
            if (threshold <= cMaxRateControlThreshold)
            {
                SSRateControlThreshold = threshold;
                TRACE(("SSRateControlThreshold=%d", SSRateControlThreshold));
            }
        }
        return true;
    }
    
    //Seconds backed off from current live server time
    if (command == "sslivebackoffsec")
    {
        if (numargs == 1)
        {
            SSLiveBackOffSeconds = atoi(args[0].c_str());
            TRACE(("SSLiveBackOffSeconds=%u", SSLiveBackOffSeconds));
        }
        return true;
    }

    //Seconds playback is backed off from the value of the SSLiveBackOffSeconds when tuning to 'live'
    if (command == "ssliveplaybackoffsetsec")
    {
        if (numargs == 1)
        {
            SSLivePlaybackOffsetSeconds = atoi(args[0].c_str());
            TRACE(("SSLivePlaybackOffsetSeconds=%u", SSLivePlaybackOffsetSeconds));
        }
        return true;
    }

    //Seconds after first chunk to start playing at when tuning to left edge
    if (command == "sslivemintimebuffersec")
    {
        if (numargs == 1)
        {
            SSLeftEdgeBufferSeconds = atoi(args[0].c_str());
            TRACE(("SSLeftEdgeBufferSeconds=%u", SSLeftEdgeBufferSeconds));
        }
        return true;
    }

    //HTTP fragment request header to use for reporting bandwidth measurements [empty->off]
    if (command == "ssbandwidthreportheader")
    {
        AutoLock lock(&valueLock);
        
        if (numargs == 1 && args[0].length() > 2) // size for at least "x-"
        {
            SSBandwidthReportHeader = args[0];
            TRACE(("SSBandwidthReportHeader=%s", SSBandwidthReportHeader.c_str()));
        }
        else // empty the string to indicate no reporting
        {
            SSBandwidthReportHeader.clear();
            TRACE(("SSBandwidthReportHeader DISABLED"));
        }
        return true;
    }

    return false;
}

// ===============================================================================================================
// ===============================================================================================================

CSocketMbrConfiguration gSocketMbrConfiguration;

// ===============================================================================================================
// ===============================================================================================================
