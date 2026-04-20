///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "wmaudiofmt.h"
#include "IPacket.h"
#include "MarshallingUtils.h"

// ===============================================================================================================
// Sockets package raw elementary stream data in this format and writes it to receiver
// ===============================================================================================================

#define RAWPACKET_IS_RESETAUDIO             0x00000001
#define RAWPACKET_IS_ENDOFFRAME             0x00000002
#define RAWPACKET_IS_STARTCODE_INBAND       0x00000010
#define RAWPACKET_IS_MPEG4_PART15           0x00000020

//If PID / StreamId is not available, use these default PID values
#define RAWPACKET_DEFAULT_VIDEO_PID         1001
#define RAWPACKET_DEFAULT_AUDIO_PID         1002

class CRawPacket : public IPacket
{
public:
    enum eDataType
    {
        kDataType_Invalid = -1,
        kDataType_Video = 0,
        kDataType_Audio,
        kDataType_DFXP,
        kDataType_TS,
    };

    CRawPacket(ePacketType packetType = kPacketType_Raw, eDataType dataType = kDataType_Invalid)
        : IPacket(packetType)
        , DataType(dataType)
        , IsMetaData(false)
    {
        Init_CRawPacket();
    }

    __override void Init()
    {
        IPacket::Init();
        Init_CRawPacket();
    }

    void Init(eDataType dataType, bool isMetaData = false)
    {
        DataType = dataType;
        IsMetaData = isMetaData;
        Init();

        //Only initialize data-type once when setting type
        if (!IsMetaData)
        {
            switch(DataType)
            {
            case kDataType_Video:
                Init_CRawVideoPacket();
                break;
            case kDataType_Audio:
                Init_CRawAudioPacket();
                break;
            case kDataType_DFXP:
                Init_CRawSubtitlePacket();
                break;
            default:
                //For all other cases, default to audio which takes the most space
                Init_CRawAudioPacket();
                break;
            }
        }
    }

    bool IsMetaDataPacket() const { return IsMetaData; }
    bool IsVideo() const { return DataType == kDataType_Video; }
    bool IsAudio() const { return DataType == kDataType_Audio; }
    bool IsSubtitle() const { return DataType == kDataType_DFXP; }

private:
    void Init_CRawPacket()
    {
        AudioOnly = false;
        IsSync = false;
        FourCC = 0;
        Pts = 0;
        Dts = 0;
        Duration = 0;

        SampleID = INVALID_SAMPLEID;
        KeyID = 0;
        KeyIDLength = 0;
        DrmHandle = 0;

        SubSampleEntries = 0;
        SubSampleClearBytes = NULL;
        SubSampleEncryptedBytes = NULL;

        QualityLevel = 0;
    }

    void Init_CRawVideoPacket()
    {
        StreamId = RAWPACKET_DEFAULT_VIDEO_PID;

        Video.FrameWidth = 0;
        Video.FrameHeight = 0;
        Video.PixelAspectX = 0;
        Video.PixelAspectY = 0;
        Video.NalUnitLength = 0;
        Video.PrivateDataLen = 0;
        Video.PrivateData = NULL;
    }

    void Init_CRawAudioPacket()
    {
        StreamId = RAWPACKET_DEFAULT_AUDIO_PID;

        Audio.VersionNumber = 0;
        Audio.SamplingRate = 0;
        Audio.ChannelNumber = 0;
        Audio.BytesPerSec = 0;
        Audio.BlockSize = 0;
        Audio.BitsPerSample = 0;
        Audio.ValidBitsPerSample = 0;
        Audio.EncoderOption = 0;
        Audio.ChannelMask = 0;
        Audio.HeaderDataLen = 0;
        Audio.HeaderData = NULL;
    }

    void Init_CRawSubtitlePacket()
    {
        Subtitles.startTime = 0;
    }

    eDataType      DataType;
    bool           IsMetaData;

public:
    bool IsEncrypted()
    {
        return DrmHandle != 0; // Must use zero (not NULL) since DrmHandle is an integer
    }

    void ParseWaveFormatEx(WAVEFORMATEX* pWaveFormatEx)
    {
        memset(&Audio, 0, sizeof(Audio));
        if (!pWaveFormatEx)
            return;

        FourCC = pWaveFormatEx->wFormatTag;
        Audio.SamplingRate = pWaveFormatEx->nSamplesPerSec;
        Audio.ChannelNumber = pWaveFormatEx->nChannels;
        Audio.BytesPerSec = pWaveFormatEx->nAvgBytesPerSec;
        Audio.BlockSize = pWaveFormatEx->nBlockAlign;
        Audio.BitsPerSample = pWaveFormatEx->wBitsPerSample;

        if (pWaveFormatEx->wFormatTag == 0x0160 && pWaveFormatEx->cbSize >= 4)
        {
            // wave format 1
            MSAUDIO1WAVEFORMAT* pWaveFormat1 = (MSAUDIO1WAVEFORMAT*)pWaveFormatEx;
            Audio.EncoderOption = pWaveFormat1->wEncodeOptions;
            Audio.VersionNumber = 1;
        }
        if (pWaveFormatEx->wFormatTag == 0x0161 && pWaveFormatEx->cbSize >= 10)
        {
            // wave format 2
            WMAUDIO2WAVEFORMAT* pWaveFormat2 = (WMAUDIO2WAVEFORMAT*)pWaveFormatEx;
            Audio.EncoderOption = pWaveFormat2->wEncodeOptions;
            Audio.VersionNumber = 2;
        }
        if (pWaveFormatEx->wFormatTag == 0x0162 && pWaveFormatEx->cbSize >= 12)
        {
            // wave format 3 - apparently there are two flavors - 3 and 3X
            if (pWaveFormatEx->cbSize >= 18)    // 3
            {
                WMAUDIO3WAVEFORMAT* pWaveFormat3 = (WMAUDIO3WAVEFORMAT*)pWaveFormatEx;
                Audio.EncoderOption = pWaveFormat3->wEncodeOptions;
                Audio.ValidBitsPerSample = pWaveFormat3->wValidBitsPerSample;
                Audio.ChannelMask = pWaveFormat3->dwChannelMask;
            }
            else                                // 3X
            {
                WMAUDIO3XWAVEFORMAT* pWaveFormat3X = (WMAUDIO3XWAVEFORMAT*)pWaveFormatEx;
                Audio.EncoderOption = pWaveFormat3X->wEncodeOptions;
                Audio.ValidBitsPerSample = pWaveFormat3X->wfx.Samples.wValidBitsPerSample;
                Audio.ChannelMask = pWaveFormat3X->wfx.dwChannelMask;
            }
            Audio.VersionNumber = 3;
        }

        // WAVEFORMATEX is persisted in the manifest using little endian
        // representations. Convert the fields to CPU representations.
        // (The rest of the WaveFormatEx fields are converted during manifest parsing).

        Audio.EncoderOption = LittleEndian::ToHost( Audio.EncoderOption );
        Audio.ValidBitsPerSample = LittleEndian::ToHost( Audio.ValidBitsPerSample );
        Audio.ChannelMask = LittleEndian::ToHost( Audio.ChannelMask );
    }

    bool           AudioOnly;
    bool           IsSync;
    uint32         FourCC;
    uint64         Pts;
    uint64         Dts;
    uint64         Duration;

    uint64         SampleID;
    byte*          KeyID;
    int32          KeyIDLength;
    uint32         DrmHandle;

    uint16         SubSampleEntries;
    uint16*        SubSampleClearBytes;
    uint32*        SubSampleEncryptedBytes;

    uint16         QualityLevel;

    union
    {
        struct
        {
            uint32 FrameWidth;
            uint32 FrameHeight;
            uint32 PixelAspectX;
            uint32 PixelAspectY;
            uint32 NalUnitLength;
            uint32 PrivateDataLen;
            byte*  PrivateData;
        } Video;

        struct
        {
            uint16 VersionNumber;
            uint32 SamplingRate;
            uint16 ChannelNumber;
            uint32 BytesPerSec;
            uint32 BlockSize;
            uint16 BitsPerSample;
            uint16 ValidBitsPerSample;
            uint16 EncoderOption;
            uint32 ChannelMask;
            uint32 HeaderDataLen;
            byte*  HeaderData;
        } Audio;

        struct
        {
            uint64 startTime;
        } Subtitles;
    };
};

// ===============================================================================================================
// ===============================================================================================================
