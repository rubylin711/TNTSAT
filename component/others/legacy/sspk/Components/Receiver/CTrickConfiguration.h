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
// Global configuration parameters controlling trick behavior
// ===============================================================================================================

class CTrickConfiguration
{
public:
    //Constructor
    CTrickConfiguration();

    //Process commands
    bool        Command(const std::string& command, const std::vector<std::string>& args);

public:
    //Maximum number of trick speeds supported
    //
    //The following is used to preallocate speace in teh config so
    //keep this value as >= 4 even if we are supporting less than 4
    //trick speeds.
    enum eTrickConfiguration
    {
        TRICKS_MAX_SPEEDS_SUPPORTED = 4,
    };

    //Length of pause buffer in seconds
    uint32      PauseBufferSeconds;

    //Number of frames rendered per second when in trick mode
    uint32      FramesPerSecondForTricks;

    //Default fast forward speeds
    double      FastForwardSpeeds[TRICKS_MAX_SPEEDS_SUPPORTED];
    uint32      FastForwardSpeedsNum;

    //Default correction to be applied to timeline when transitioning from fast forward to play
    uint32      AutoCorrectionForFastForward;
    //Correction to be applied to timeline when transitioning from fast forward to play from different speeds
    uint32      AutoCorrectionForFastForwardSpeeds[TRICKS_MAX_SPEEDS_SUPPORTED];
    uint32      AutoCorrectionForFastForwardSpeedsNum;

    //Default rewind speeds
    double      RewindSpeeds[TRICKS_MAX_SPEEDS_SUPPORTED];
    uint32      RewindSpeedsNum;

    //Default correction to be applied to timeline when transitioning from rewind to play
    uint32      AutoCorrectionForRewind;
    //Correction to be applied to timeline when transitioning from rewind to play from different speeds
    uint32      AutoCorrectionForRewindSpeeds[TRICKS_MAX_SPEEDS_SUPPORTED];
    uint32      AutoCorrectionForRewindSpeedsNum;
};

// ===============================================================================================================
// ===============================================================================================================

extern CTrickConfiguration gTrickConfiguration;

// ===============================================================================================================
// ===============================================================================================================
