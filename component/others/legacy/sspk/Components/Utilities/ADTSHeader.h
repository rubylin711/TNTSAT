///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

class ADTSHeader
{
public:
    static const uint32 SIZE = 7;

    ADTSHeader();

    void SetADTSHeader(uint8 profile, uint8 channels, uint32 samplingRate);
    void SetFrameLength(uint16 dataLength);
    byte* GetHeaderBits();

private:
    void SetChannelConfig(uint8 channelConfig);
    void SetSamplingFreqIndex(uint8 samplingFreqIndex);
    void SetProfile(uint8 profile);
    uint8 SamplingRateToFreqIndex(uint32 samplingRate);

private:
    // * ADTS Fixed Header: these don't change from frame to frame
    // syncword                                       12    always: '111111111111'
    // ID                                               1    0: MPEG-4, 1: MPEG-2
    // layer                                           2    always: '00'
    // protection_absent                               1
    // profile                                           2
    // sampling_frequency_index                        4
    // private_bit                                       1
    // channel_configuration                           3
    // original/copy                                   1
    // home                                            1
#if 0 // Removed in corrigendum 14496-3:2002
    // emphasis                                        2    only if ID == 0 (ie MPEG-4)
    uint16 emphasis                         :  2;
#endif

    // * ADTS Variable Header: these can change from frame to frame
    // copyright_identification_bit                    1
    // copyright_identification_start                   1
    // aac_frame_length                               13    length of the frame including header (in bytes)
    // adts_buffer_fullness                           11    0x7FF indicates VBR
    // no_raw_data_blocks_in_frame                       2


    // NOTE: the ADTS header has to be layed out as a bit stream.  In a bit
    // stream, the most significant bits are filled first.

    // byte 0:
    uint8 _syncword1                        :  8;
    // byte 1:
    uint8 _syncword2                        :  4;
    uint8 _ID                                :  1;
    uint8 _layer                            :  2;
    uint8 _protection_absent                :  1;
    // byte 2:
    uint8 _profile                            :  2;
    uint8 _sampling_frequency_index         :  4;
    uint8 _private_bit                        :  1;
    uint8 _channel_configuration1            :  1;
    // byte 3:
    uint8 _channel_configuration2            :  2;
    uint8 _original_copy                    :  1;
    uint8 _home                             :  1;
    uint8 _copyright_identification_bit     :  1;
    uint8 _copyright_identification_start    :  1;
    uint8 _aac_frame_length1                :  2;
    // byte 4:
    uint8 _aac_frame_length2                :  8;
    // byte 5:
    uint8 _aac_frame_length3                :  3;
    uint8 _adts_buffer_fullness1            :  5;
    // byte 6:
    uint8 _adts_buffer_fullness2            :  6;
    uint8 _no_raw_data_blocks_in_frame        :  2;

    // * ADTS Error check
    // crc_check                                      16    only if protection_absent == 0

    byte _buffer[SIZE];
};

