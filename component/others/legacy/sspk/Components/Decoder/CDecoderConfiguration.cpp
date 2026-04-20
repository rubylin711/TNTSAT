///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CDecoderConfiguration.h"
#include "CStreamInfo.h"
#include "CPipeId.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

// ===============================================================================================================
// Decoder Masks to allow enabling/disabling individul decoder types
// ===============================================================================================================

//32 different video decoder types starting from 0x00000001
enum eVideoDecoderTypes
{
    DECODER_VIDEO_NONE   = 0x00000000,
    DECODER_VIDEO_MPEG2  = 0x00000001,
    DECODER_VIDEO_H264   = 0x00000002,
    DECODER_VIDEO_VC1    = 0x00000004,
    DECODER_VIDEO_WMV9   = 0x00000008,
    DECODER_VIDEO_ALL    = 0xffffffff
};

//32 different audio decoder types starting from 0x00000001
enum eAudioDecoderTypes
{
    DECODER_AUDIO_NONE   = 0x00000000,
    DECODER_AUDIO_MPEG1  = 0x00000001,
    DECODER_AUDIO_MPEG2  = 0x00000002,
    DECODER_AUDIO_AC3    = 0x00000004,
    DECODER_AUDIO_WMA    = 0x00000008,
    DECODER_AUDIO_WMAPRO = 0x00000010,
    DECODER_AUDIO_AAC    = 0x00000020,
    DECODER_AUDIO_HEAAC  = 0x00000040,
    DECODER_AUDIO_EAC3   = 0x00000080,
    DECODER_AUDIO_ALL    = 0xffffffff
};

// ===============================================================================================================
// ===============================================================================================================

#define AUDIO_HAL_BUFFERING_THRESHOLD_90KHZ 90000

// ===============================================================================================================
// CDecoderConfiguration
// - Manages the state for which decoeders are enabled
// - By default native AV engine supports all decoders
// ===============================================================================================================

CDecoderConfiguration::CDecoderConfiguration()
    : m_videoMask(DECODER_VIDEO_ALL)
    , m_audioMask(DECODER_AUDIO_ALL)
    , m_teardownPictureOnDetune(false)
    , m_enableAudioDescriptionPostMix(true)
    , m_pipeIdToOwnAudio(0)
    , m_pipeIdOwnsAudio(0)
    , m_pipeIdOwnsAudioDescription(0)
    , m_audioHalBufferingThreshold(AUDIO_HAL_BUFFERING_THRESHOLD_90KHZ)
{
}

//Is the given decoder enabled?
bool CDecoderConfiguration::IsEnabled(int streamType)
{
    switch (streamType)
    {
        case StreamType_Video:                      return (m_videoMask & DECODER_VIDEO_MPEG2)  == DECODER_VIDEO_MPEG2;
        case StreamType_Video_Constrained:          return (m_videoMask & DECODER_VIDEO_MPEG2)  == DECODER_VIDEO_MPEG2;
        case StreamType_Video_AVC:                  return (m_videoMask & DECODER_VIDEO_H264)   == DECODER_VIDEO_H264;
        case StreamType_Video_VC1:                  return (m_videoMask & DECODER_VIDEO_VC1)    == DECODER_VIDEO_VC1;
        case StreamType_Video_WMV9:                 return (m_videoMask & DECODER_VIDEO_WMV9)   == DECODER_VIDEO_WMV9;

        case StreamType_Audio_11172:                return (m_audioMask & DECODER_AUDIO_MPEG1)  == DECODER_AUDIO_MPEG1;
        case StreamType_Audio_13818_3:              return (m_audioMask & DECODER_AUDIO_MPEG2)  == DECODER_AUDIO_MPEG2;
        case StreamType_Audio_AC3:                  return (m_audioMask & DECODER_AUDIO_AC3)    == DECODER_AUDIO_AC3;
        case StreamType_Audio_EnhancedAC3:          return (m_audioMask & DECODER_AUDIO_EAC3)   == DECODER_AUDIO_EAC3;
        case StreamType_Audio_AAC:                  return (m_audioMask & DECODER_AUDIO_AAC)    == DECODER_AUDIO_AAC;
        case StreamType_Audio_HEAAC:                return (m_audioMask & DECODER_AUDIO_HEAAC)  == DECODER_AUDIO_HEAAC;
        case StreamType_Audio_WMA:                  return (m_audioMask & DECODER_AUDIO_WMA)    == DECODER_AUDIO_WMA;
        case StreamType_Audio_WMA_WITH_HEADER:      return (m_audioMask & DECODER_AUDIO_WMA)    == DECODER_AUDIO_WMA;
        case StreamType_Audio_WMAPRO:               return (m_audioMask & DECODER_AUDIO_WMAPRO) == DECODER_AUDIO_WMAPRO;
    }
    return false;
}

//Audio stream playback switching at pipe level.
//This API is to request a switch of audio for the new pipe.
//This means the current pipe needs to give up the HAL decoder that it may
//be using.
void CDecoderConfiguration::SwitchPipeToOwnAudio(uint32 pipeIdN)
{
    AutoLock lock(&m_decoderConfigLock);
    m_pipeIdToOwnAudio = pipeIdN;
}

bool CDecoderConfiguration::DoesPipeOwnAudio(uint32 pipeIdN)
{
    AutoLock lock(&m_decoderConfigLock);
    return pipeIdN == m_pipeIdToOwnAudio;
}

void CDecoderConfiguration::SetPipeOwnsAudio(uint32 pipeIdN)
{
    AutoLock lock(&m_decoderConfigLock);
    m_pipeIdOwnsAudio = pipeIdN;
}

bool CDecoderConfiguration::CanPipeOwnAudio(uint32 pipeIdN)
{ 
    AutoLock lock(&m_decoderConfigLock);
    return (m_pipeIdOwnsAudio == 0 && pipeIdN == m_pipeIdToOwnAudio);
}

void CDecoderConfiguration::SetPipeOwnsAudioDescription(uint32 pipeIdN)
{
    AutoLock lock(&m_decoderConfigLock);
    m_pipeIdOwnsAudioDescription = pipeIdN;
}

bool CDecoderConfiguration::CanPipeOwnAudioDescription(uint32 pipeIdN)
{
    AutoLock lock(&m_decoderConfigLock);
    return m_pipeIdOwnsAudioDescription == 0 && pipeIdN == m_pipeIdToOwnAudio;
}

uint32 CDecoderConfiguration::GetAudioHalBufferingThreshold() 
{
    AutoLock lock(&m_decoderConfigLock);
    return m_audioHalBufferingThreshold; 
}

//Process commands
bool CDecoderConfiguration::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    uint32 numargs = (uint32)args.size();
    
    AutoLock lock(&m_decoderConfigLock);
    //Video decoder enablement mask
    if (command == "codecvideomask")
    {
        if (numargs == 1)
        {
            m_videoMask = atoi(args[0].c_str());

            TRACE(("VideoMask=%08x", m_videoMask));
        }
        return true;
    }

    //Audio decoder enablement mask
    if (command == "codecaudiomask")
    {
        if (numargs == 1)
        {
            m_audioMask = atoi(args[0].c_str());

            TRACE(("AudioMask=%08x", m_audioMask));
        }
        return true;
    }

    //Whether to teardown picture when detuning
    if (command == "teardownpicture")
    {
        if (numargs == 1)
        {
            m_teardownPictureOnDetune = toLower(args[0]) == "true";

            TRACE(("TeardownPictureOnDetune=%s", m_teardownPictureOnDetune ? "true" : "false"));
        }
        return true;
    }

    //Enable mixing of audio description track with main audio
    if (command == "disableadpostmix")
    {
        if (numargs == 1)
        {
            m_enableAudioDescriptionPostMix = toLower(args[0]) == "false";

            TRACE(("EnableAudioDescriptionPostMix=%s", m_enableAudioDescriptionPostMix ? "true" : "false"));
        }
        return true;
    }

    //How much audio buffer will be sent to HAL audio ES buffers
    if (command == "audiohalbufferingthreshold")
    {
        if (numargs == 1)
        {
            // convert from milliseconds to 90kHz
            m_audioHalBufferingThreshold = atoi(args[0].c_str()) * 90;
            if (m_audioHalBufferingThreshold < AUDIO_HAL_BUFFERING_THRESHOLD_90KHZ)
            {
                TRACE(("AudioHalBufferingThreshold=%u is too low setting to default", m_audioHalBufferingThreshold));
                m_audioHalBufferingThreshold = AUDIO_HAL_BUFFERING_THRESHOLD_90KHZ;
            }

            TRACE(("AudioHalBufferingThreshold=%u", m_audioHalBufferingThreshold));
        }
        return true;
    }

    return false;
}

// ===============================================================================================================
// Global decoder enablement tracker
// ===============================================================================================================

CDecoderConfiguration gDecoderConfiguration;

// ===============================================================================================================
// ===============================================================================================================
