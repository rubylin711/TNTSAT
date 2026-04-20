///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ADTSHeader.h"
#include "BitStream.h"

#define ADTS_SRT_LENGTH 13
static uint32 SamplingRateTable[ADTS_SRT_LENGTH] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000 , 7350
};

ADTSHeader::ADTSHeader()
    :_syncword1(0xff)
    ,_syncword2(0xf)
    ,_ID(0)
    ,_layer(0)
    ,_protection_absent(1)
    ,_profile(0)
    ,_sampling_frequency_index(0)
    ,_private_bit(0)
    ,_channel_configuration1(0)
    ,_channel_configuration2(0)
    ,_original_copy(1)
    ,_home(0)
    ,_copyright_identification_bit(0)
    ,_copyright_identification_start(0)
    ,_aac_frame_length1(0)
    ,_aac_frame_length2(0)
    ,_aac_frame_length3(0)
    ,_adts_buffer_fullness1(0x1f)
    ,_adts_buffer_fullness2(0x3f)
    ,_no_raw_data_blocks_in_frame(0)
{
    memset(_buffer, 0, SIZE);
}

void ADTSHeader::SetADTSHeader(uint8 profile, uint8 channels, uint32 samplingRate)
{
    SetChannelConfig(channels);
    SetProfile(profile);
    SetSamplingFreqIndex(SamplingRateToFreqIndex(samplingRate));
}

void ADTSHeader::SetFrameLength(uint16 dataLength)
{
    uint16 frameLength = dataLength;
    _aac_frame_length1 = (uint8)((0x1800 & frameLength) >> 11);
    _aac_frame_length2 = (uint8)((0x7f8  & frameLength) >> 3);
    _aac_frame_length3 = (uint8)( 0x7    & frameLength);
}

byte* ADTSHeader::GetHeaderBits()
{
    bit_string_writer bsw;

    bsw.init(_buffer, SIZE);

    // byte 0:
    bsw.put_bits(_syncword1, 8);
    // byte 1:
    bsw.put_bits(_syncword2, 4);
    bsw.put_bits(_ID, 1);
    bsw.put_bits(_layer, 2);
    bsw.put_bits(_protection_absent, 1);
    // byte 2:
    bsw.put_bits(_profile, 2);
    bsw.put_bits(_sampling_frequency_index, 4);
    bsw.put_bits(_private_bit, 1);
    bsw.put_bits(_channel_configuration1, 1);
    // byte 3:
    bsw.put_bits(_channel_configuration2, 2);
    bsw.put_bits(_original_copy, 1);
    bsw.put_bits(_home, 1);
    bsw.put_bits(_copyright_identification_bit, 1);
    bsw.put_bits(_copyright_identification_start, 1);
    bsw.put_bits(_aac_frame_length1, 2);
    // byte 4:
    bsw.put_bits(_aac_frame_length2, 8);
    // byte 5:
    bsw.put_bits(_aac_frame_length3, 3);
    bsw.put_bits(_adts_buffer_fullness1, 5);
    // byte 6:
    bsw.put_bits(_adts_buffer_fullness2, 6);
    bsw.put_bits(_no_raw_data_blocks_in_frame, 2);

    return _buffer;
}

void ADTSHeader::SetChannelConfig(uint8 channelConfig)
{
    _channel_configuration1 = (uint8)((0x4 & channelConfig) >> 2);
    _channel_configuration2 = (uint8)( 0x3 & channelConfig);
}

void ADTSHeader::SetSamplingFreqIndex(uint8 samplingFreqIndex)
{
    _sampling_frequency_index = samplingFreqIndex;
}

void ADTSHeader::SetProfile(uint8 profile)
{
    _profile = profile - 1;
}

uint8 ADTSHeader::SamplingRateToFreqIndex(uint32 samplingRate)
{
    int i = 0;
    while (i < ADTS_SRT_LENGTH)
    {
        if (SamplingRateTable[i] == samplingRate)
            break;

        ++i;
    }

    return (uint8) ((i == ADTS_SRT_LENGTH) ? 0xf : i);
}


