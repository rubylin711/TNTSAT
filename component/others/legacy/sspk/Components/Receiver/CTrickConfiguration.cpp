///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CTrickConfiguration.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

// ===============================================================================================================
// Global configuration parameters controlling trick behavior
// ===============================================================================================================

CTrickConfiguration::CTrickConfiguration()
{
    //Length of pause buffer in seconds
    PauseBufferSeconds = 90 * 60;

    //Number of frames rendered per second when in trick mode
    FramesPerSecondForTricks = 4;

    //Default fast forward speeds
    memset(FastForwardSpeeds, 0, sizeof(FastForwardSpeeds));
    FastForwardSpeeds[0] = 5;
    FastForwardSpeeds[1] = 15;
    FastForwardSpeeds[2] = 60;
    FastForwardSpeeds[3] = 300;
    FastForwardSpeedsNum = 4;

    //Default correction to be applied to timeline when transitioning from fast forward to play
    AutoCorrectionForFastForward = 3000;
    //Correction to be applied to timeline when transitioning from fast forward to play from different speeds
    AutoCorrectionForFastForwardSpeeds[0] = 0;
    AutoCorrectionForFastForwardSpeeds[1] = 3000;
    AutoCorrectionForFastForwardSpeeds[2] = 2000;
    AutoCorrectionForFastForwardSpeeds[3] = 0;
    AutoCorrectionForFastForwardSpeedsNum = 4;

    //Default rewind speeds
    memset(RewindSpeeds, 0, sizeof(RewindSpeeds));
    RewindSpeeds[0] = -5;
    RewindSpeeds[1] = -15;
    RewindSpeeds[2] = -60;
    RewindSpeeds[3] = -300;
    RewindSpeedsNum = 4;

    //Default correction to be applied to timeline when transitioning from rewind to play
    AutoCorrectionForRewind = 0;
    //Correction to be applied to timeline when transitioning from rewind to play from different speeds
    AutoCorrectionForRewindSpeeds[0] = 0;
    AutoCorrectionForRewindSpeeds[1] = 0;
    AutoCorrectionForRewindSpeeds[2] = 0;
    AutoCorrectionForRewindSpeeds[3] = 0;
    AutoCorrectionForRewindSpeedsNum = 4;
}

//Process commands
bool CTrickConfiguration::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    uint32 numargs = (uint32)args.size();

    //Length of pause buffer in seconds
    if (command == "pausebufferseconds")
    {
        if (numargs == 1)
        {
            PauseBufferSeconds = atoi(args[0].c_str());

            TRACE(("PauseBufferSeconds=%d", PauseBufferSeconds));
        }
        return true;
    }

    //Frames per second to berendered while in trick mode
    if (command == "fps")
    {
        if (numargs == 1)
        {
            FramesPerSecondForTricks = atoi(args[0].c_str());

            TRACE(("FramesPerSecondForTricks=%d", FramesPerSecondForTricks));
        }
        return true;
    }

    //Different fast forward speeds supported
    if (command == "ffspeeds")
    {
        if (numargs > 0)
        {
            if (numargs > TRICKS_MAX_SPEEDS_SUPPORTED)
            {
                numargs = TRICKS_MAX_SPEEDS_SUPPORTED;
            }
            memset(FastForwardSpeeds, 0, sizeof(FastForwardSpeeds));
            for (uint32 i = 0; i < numargs; ++i)
            {
                FastForwardSpeeds[i] = atof(args[i].c_str());
            }
            FastForwardSpeedsNum = numargs;

            TRACE(("FastForwardSpeedsNum=%d", FastForwardSpeedsNum));
            TRACE(("FastForwardSpeeds={%d,%d,%d,%d}",
                        (int32)FastForwardSpeeds[0],
                        (int32)FastForwardSpeeds[1],
                        (int32)FastForwardSpeeds[2],
                        (int32)FastForwardSpeeds[3]));
        }
        return true;
    }

    //Default auto correction to be applied when transitioning from fast forward to play
    if (command == "acff")
    {
        if (numargs == 1)
        {
            AutoCorrectionForFastForward = atoi(args[0].c_str());

            TRACE(("AutoCorrectionForFastForward=%d", AutoCorrectionForFastForward));
        }
        return true;
    }

    //Auto correction to be applied when transitioning from fast forward to play based on different speeds
    if (command == "acffspeeds")
    {
        if (numargs > 0)
        {
            if (numargs > TRICKS_MAX_SPEEDS_SUPPORTED)
            {
                numargs = TRICKS_MAX_SPEEDS_SUPPORTED;
            }
            memset(AutoCorrectionForFastForwardSpeeds, 0, sizeof(AutoCorrectionForFastForwardSpeeds));
            for (uint32 i = 0; i < numargs; ++i)
            {
                AutoCorrectionForFastForwardSpeeds[i] = atoi(args[i].c_str());
            }
            AutoCorrectionForFastForwardSpeedsNum = numargs;

            TRACE(("AutoCorrectionForFastForwardSpeedsNum=%d", AutoCorrectionForFastForwardSpeedsNum));
            TRACE(("AutoCorrectionForFastForwardSpeeds={%d,%d,%d,%d}",
                        AutoCorrectionForFastForwardSpeeds[0],
                        AutoCorrectionForFastForwardSpeeds[1],
                        AutoCorrectionForFastForwardSpeeds[2],
                        AutoCorrectionForFastForwardSpeeds[3]));
        }
        return true;
    }

    //Different rewind speeds supported
    if (command == "rwspeeds")
    {
        if (numargs > 0)
        {
            if (numargs > TRICKS_MAX_SPEEDS_SUPPORTED)
            {
                numargs = TRICKS_MAX_SPEEDS_SUPPORTED;
            }
            memset(RewindSpeeds, 0, sizeof(RewindSpeeds));
            for (uint32 i = 0; i < numargs; ++i)
            {
                RewindSpeeds[i] = atof(args[i].c_str());
            }
            RewindSpeedsNum = numargs;

            TRACE(("RewindSpeedsNum=%d", RewindSpeedsNum));
            TRACE(("RewindSpeeds={%d,%d,%d,%d}",
                        (int32)RewindSpeeds[0],
                        (int32)RewindSpeeds[1],
                        (int32)RewindSpeeds[2],
                        (int32)RewindSpeeds[3]));
        }
        return true;
    }

    //Default auto correction to be applied when transitioning from rewind to play
    if (command == "acrw")
    {
        if (numargs == 1)
        {
            AutoCorrectionForRewind = atoi(args[0].c_str());

            TRACE(("AutoCorrectionForRewind=%d", AutoCorrectionForRewind));
        }
        return true;
    }

    //Auto correction to be applied when transitioning from rewind to play based on different speeds
    if (command == "acrwspeeds")
    {
        if (numargs > 0)
        {
            if (numargs > TRICKS_MAX_SPEEDS_SUPPORTED)
            {
                numargs = TRICKS_MAX_SPEEDS_SUPPORTED;
            }
            memset(AutoCorrectionForRewindSpeeds, 0, sizeof(AutoCorrectionForRewindSpeeds));
            for (uint32 i = 0; i < numargs; ++i)
            {
                AutoCorrectionForRewindSpeeds[i] = atoi(args[i].c_str());
            }
            AutoCorrectionForRewindSpeedsNum = numargs;

            TRACE(("AutoCorrectionForRewindSpeedsNum=%d", AutoCorrectionForRewindSpeedsNum));
            TRACE(("AutoCorrectionForRewindSpeeds={%d,%d,%d,%d}",
                        AutoCorrectionForRewindSpeeds[0],
                        AutoCorrectionForRewindSpeeds[1],
                        AutoCorrectionForRewindSpeeds[2],
                        AutoCorrectionForRewindSpeeds[3]));
        }
        return true;
    }

    return false;
}

// ===============================================================================================================
// ===============================================================================================================

CTrickConfiguration gTrickConfiguration;

// ===============================================================================================================
// ===============================================================================================================
