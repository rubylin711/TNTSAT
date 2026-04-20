///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Decoder_DRM.h"
#include "IPUDHandler.h"

// ===============================================================================================================
// ===============================================================================================================

class DecoderVideo : public DecoderDRM
{
public:
    DecoderVideo(IReceiverControl* receiverControl, const CStreamInfo& si);
    virtual ~DecoderVideo();

    __override bool             Acquire(void);
    __override void             Release(void);
    __override DECODER_ERR      IoControl(DECODER_CONTROL control, void* inBuf=NULL, int inSize=0, void* outBuf=NULL, int outSize=0);
    __override bool             CheckStall(void);
    __override void             SetKeyId(__in_bcount(len) const byte* signedKeyId, uint32 len);
    __override void             SetSampleId(uint64 sampleId);
    __override void             SetDRMHandle(uint32 drmHandle);
    __override void             OnSync(bool sync, bool teardownPicture, bool cleanStall);
    __override int              CheckAccessControl(uint64 pts = 0);
    __override bool             GetCurrentPTS(uint64* pPTS);
    __override void             SetFrameSize(uint32 width, uint32 height) {mFrameWidth = width; mFrameHeight = height;}

    __override void             SetPassThroughAfterDecryption(void);
    __override bool             IsRenderingDone(uint64 stc);
    __override uint64           GetLastPtsSent() const;
    __override void             SetEndOfStream(void);

private:
    __override Buffer*          AddSample(Buffer* chain);

    //User data parser
    IPUDHandler*                mPUDHandler;

    //WMV header to use
    IPTV_HAL_DECODER_WMV_HEADER mWmvParameters;
    //Whether WMV header is available for use
    bool                        mWmvParametersSet;

    //Keeping track of IFrames while in immediate mode
    uint64                      mIFramePts;
    //Whether to discard next samples
    bool                        mDiscardNextSample;

    //Handling finger printing flags
    byte                        mFingerprintFlagsLast;

    //Tracking decoder stalls
    int                         mStallCount;
    //Tracking FIFO read pointer for stall check
    uint32                      mLastFifoReadPtr;

    //Frame size
    uint32                      mFrameWidth;
    uint32                      mFrameHeight;

    //Last pts sent to decoder
    uint64                      mLastSentPts;

};

// ===============================================================================================================
// ===============================================================================================================
