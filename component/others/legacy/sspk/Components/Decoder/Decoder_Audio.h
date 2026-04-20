///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Decoder_DRM.h"

// ===============================================================================================================
// Bounds a frame - contains all information for an incoming frame
//
// Currently the DRM information is being pushed for every frame; optimization for memory as well as
// perf are warranted where we push the DRM Key Id information only once when they appear and just
// use the sample id from there on until a new ECM blob pops up.
// ===============================================================================================================

class CFrameKeyId
{
public:
    //DRM Key Id
    byte    mDrmKeyId[DRM_KEYID_SIZE];
    uint32  mDrmKeyIdLen;
};

class CFrame
{
public:
    //Each audio buffer is 2K so this will give a limit of
    //one second to a PES frame in the worst case.
    static const int cBufferCountCap = 32;

    //DRM Sample Id for this frame
    uint64       mDrmSampleId;
    //DRM Key Id for this frame
    CFrameKeyId* mDrmKeyId;
    //DRM handle for this frame
    uint32       mDrmHandle;
    //Buffer chain for this frame
    BufferChain  mChain;
    //PTS for this frame
    uint64       mPTS;
    //DTS for this frame
    uint64       mDTS;
    //Whether this is potentially a bad PES frame
    bool         mBadFrame;
    //Fade parsed from AD descriptor, used to fade main audio
    uint8        mFade;
    //Panning for AD, parsed from AD descriptor
    int8         mPan;
    //The tick count that this frame was pushed to the decoder HAL
    uint32       mTick;
    //How long after frame was pushed until we need to update volume/panning
    uint32       mTicksUntilUpdate;
    //Total bytes of this frame
    uint32       mFrameSize;
    //Total buffer count chained up in this frame
    uint32       mBufferCount;
    //Points to next frame
    CFrame*      mNext;
};

// ===============================================================================================================
// ===============================================================================================================

class DecoderAudio : public DecoderDRM
{
public:
    DecoderAudio(IReceiverControl* receiverControl, const CStreamInfo& si);
    virtual ~DecoderAudio();

    __override bool                 Acquire(void);
    __override void                 Release(void);
    __override DECODER_ERR          IoControl(DECODER_CONTROL control, void* inBuf=NULL, int inSize=0, void* outBuf=NULL, int outSize=0);
    __override bool                 CheckStall(void);
    __override void                 SetKeyId(__in_bcount(len) const byte* signedKeyId, uint32 len);
    __override void                 SetSampleId(uint64 sampleId);
    __override void                 SetDRMHandle(uint32 drmHandle);
    __override void                 OnSync(bool sync, bool teardownPicture, bool cleanStall);
    __override int                  CheckAccessControl(uint64 pts = 0);
    __override bool                 GetCurrentPTS(uint64* pPTS);
    __override bool                 SetExpirationTime(bool ondestruct, uint64 pts);
    __override uint8                GetAudioDescriptionFade(void);
    __override void                 SetFadeAndPan(uint8 fade, int pan);
    __override void                 OnTimeslice(void);
    __override void                 SetEndOfStream();
    __override bool                 IsRenderingDone(uint64 stc);
    __override uint64               GetLastPtsSent() const;

private:
    __override Buffer*              AddSample(Buffer* chain);

    bool                            AcquireInternal(void);
    void                            ReleaseInternal(void);
    //Dump all frames queued after the givel time
    void                            DeleteSamples(uint64 pts);

    //Initialize FrameFifo stuff
    void                            FrameFifoInit(void);
    //Write data buffers to FIFO
    void                            FrameFifoPush(Buffer* chain, uint64 pts, uint64 dts, bool rap);
    //Reads data from FIFO and pushes into the given decoder
    void                            FrameFifoPopActive(void);
    //Dump stale data from FIFO for inactive decoders
    void                            FrameFifoPopInactive(void);
    //Pops a frame from the front of the FIFO and deletes it
    CFrame*                         FrameFifoPop(bool deleteFrame = true);
    //Flush the FIFO
    void                            FrameFifoClear(void);

    //Initialize the volume/pan control fifo
    void                            FadePanQInit(void);
    //Push volume control info from CFrame into the FIFO
    void                            FadePanQPush(CFrame* frame);
    //Check to see if it's time to update volume/panning in the decoder HAL
    void                            FadePanQUpdate(void);
    //Flush the volume control FIFO
    void                            FadePanQClear(void);

    //Set volume and panning according to AD volume setting from UI and AD descriptor
    void                            UpdateVolumeAndPan(void);

private:
    //Indicate if the data must be framed (WMA)
    bool                            mFramedData;

    //DRM Key id
    byte                            mDrmKeyId[DRM_KEYID_SIZE];
    uint32                          mDrmKeyIdLen;
    //DRM handle
    uint32                          mDrmHandle;
    //DRM sample
    uint64                          mDrmSampleId;

    //Caching audio decoder parameters
    IPTV_HAL_DECODER_AUDIO_HEADER   mAudioParameters;
    bool                            mAudioParametersSet;

    //Buffering frames for this decoder
    CFrame*                         mFifoFirstFrame;
    CFrame*                         mFifoLastFrame;
    //Queue of volume/panning controls to be sent to decoder
    CFrame*                         mFirstFadePanQ;
    CFrame*                         mLastFadePanQ;
    //Keeping track of how much data is buffered
    uint64                          mFifoFirstFramePts;
    uint64                          mFifoLastFramePts;
    uint64                          mPtsDeltaBetweenFrames;
    //The first frame's pts after the decoder is opened
    uint64                          mInitialFramePts;

    //Set to expire a decoder when it is rendering audio as a hangling decoder
    //and it needs to go away when complteted rendering
    uint64                          mExpireAtStc;

    //Parsed from AD descriptor in PES header
    uint8                           mParsedFade;
    int8                            mParsedPan;
    //Current fade and panning that should be used by the decoders
    uint8                           mCurrentFade;
    int8                            mCurrentPan;
    //Previous volume and pan values that have been set in this decoder
    uint32                          mPrevVolume;
    int32                           mPrevPan;

    //Tracking decoder stalls
    int                             mStallCount;
    uint32                          mLastFifoReadPtr;

    //Last pts sent to decoder
    uint64                          mLastSentPts;

    //No more frames are going to be sent
    bool                            mEndOfStream;

    //Lock for fifo queues
    Lockable                        mFifoLock;
};

// ===============================================================================================================
// ===============================================================================================================
