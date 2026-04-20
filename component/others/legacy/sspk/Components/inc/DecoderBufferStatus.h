///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

typedef struct DecoderBufferStatus_tag
{
    uint64 currentStc90kHz;                         // Current STC, 90 kHz
    uint32 audioBufferLevel90kHz;                   // Current audio decoder buffer level, in 90 kHz unit
    uint32 videoBufferLevel90kHz;                   // Current video decoder buffer level, in 90 kHz unit
    uint32 maxVideoBufferLevel90kHz;                // Max allowed decoder buffer level, in 90 kHz unit
    uint32 minVideoBufferLevelToTakeFragment90kHz;  // Min decoder buffer level to accept another fragment, in 90kHz
    bool clockIsRunning;                            // true if the clock is running; false otherwise
}
DecoderBufferStatus;

