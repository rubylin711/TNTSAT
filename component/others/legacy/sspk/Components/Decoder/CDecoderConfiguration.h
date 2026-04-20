///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "AutoLock.h"
#include <string>
#include <vector>

// ===============================================================================================================
// CDecoderConfiguration
// - Manages the state for which decoeders are enabled
// - By default native AV engine supports all decoders
// ===============================================================================================================

class CDecoderConfiguration
{
public:
    //Constructor
    CDecoderConfiguration();

    //Is the given decoder enabled?
    bool        IsEnabled(int streamType);
    //Switching audio ownership to given pipe
    void        SwitchPipeToOwnAudio(uint32 pipeIdN);
    //Does the given pipe own audio decoder
    bool        DoesPipeOwnAudio(uint32 pipeIdN);
    //Current pipe which owns audio
    void        SetPipeOwnsAudio(uint32 pipeIdN);
    //Can the given pipe acquire the audio renderer now
    bool        CanPipeOwnAudio(uint32 pipeIdN);
    //Pipe owns audio description
    void        SetPipeOwnsAudioDescription(uint32 pipeIdN);
    //Should the given audio pid acquire the audio renderer
    bool        CanPipeOwnAudioDescription(uint32 pipeIdN);
    //Get how much audio buffer will be sent to HAL audio ES buffers (in 90KHz unit)
    uint32      GetAudioHalBufferingThreshold();
    //Process commands
    bool        Command(const std::string& command, const std::vector<std::string>& args);

private:
    Lockable    m_decoderConfigLock;
    
    //Video decoder enablement mask
    int32       m_videoMask;
    //Audio decoder enablement mask
    int32       m_audioMask;
    //Whether to teardown picture when detuning
    bool        m_teardownPictureOnDetune;
    //Enable mixing of audiod description track with main audio
    bool        m_enableAudioDescriptionPostMix;
    //Pipe which should own the audio playback
    uint32      m_pipeIdToOwnAudio;
    //Pipe which owns audio playback
    uint32      m_pipeIdOwnsAudio;
    //Pipe which owns audio description playback
    uint32      m_pipeIdOwnsAudioDescription;
    //How much audio buffer will be sent to HAL audio ES buffers (in 90KHz unit)
    uint32      m_audioHalBufferingThreshold;
};

// ===============================================================================================================
// ===============================================================================================================

extern CDecoderConfiguration gDecoderConfiguration;

// ===============================================================================================================
// ===============================================================================================================
