///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IPTVDecoderHal.h"
#include "AutoLock.h"
#include "CEvent.h"

// ===============================================================================================================
// Invalid codec type
// ===============================================================================================================

#define INVALID_CODEC -1

// ===============================================================================================================
// HAL Decoder Factory maintains a cache of all decoders that have been allocated based on their pipe id and
// reuses them whereever possible.  The HAl decoders are asynchronously flushed when they are released back to
// the pool.  The allocation of HAl decoders will wait until the decoders are flushed and released back to the
// pool.
// ===============================================================================================================

class CHalDecoderFactory;

class CHalDecoder
{
    friend class CHalDecoderFactory;

private:
    CHalDecoder(uint32 pipeIdN, bool reusable);
    ~CHalDecoder();

    bool                           CompareHeader(const LPVOID header, uint32 headerLength);
    LPVOID                         Create(IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext,
                                          __in_bcount(headerLength) const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode);
    void                           Release(void);
    bool                           IsItAllocated(void);
    void                           TeardownPicture(void);

private:
    Lockable                       Lock;
    uint32                         mPipeIdN;

    IPTV_HAL_DECODER_CODECTYPE     Codec;
    IPTV_HAL_DECODER_RESOLUTION       Resolution;
    LPVOID                         DecoderContext;
    LPVOID                         ClockContext;
    byte*                          Header;
    uint32                         HeaderLength;

    CEvent                         Available;
    bool                           IsAllocated;
    bool                           IsReleasing;
    bool                           IsFlushDecoder;
    bool                           IsTeardownPicture;
    bool                           IsReusable;
public:
    //Codec type helper APIs
    static int                     Stream2Codec(int streamType);
    static const char*             StreamName(int streamType);
    static const char*             CodecName(int codec);
};

// ===============================================================================================================
// ===============================================================================================================
