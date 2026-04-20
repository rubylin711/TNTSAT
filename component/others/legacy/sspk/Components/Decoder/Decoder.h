///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "IDecoder.h"
#include "ITimeslice.h"
#include "IReceiverControl.h"
#include "CHalDecoder.h"
#include "CRendererState.h"
#include "CReceiverDiagnostics.h"
#include "Clock.h"
#include "CStreamInfo.h"
#include "Buffer.h"

// ===============================================================================================================
// Decoder Class
// ===============================================================================================================

class Decoder : public IDecoder, public ITimeslice
{
public:
    Decoder(IReceiverControl* receiverControl, const CStreamInfo& si);
    virtual ~Decoder();

    // === IDecoder ===

    //Acquire a decoder
    __override bool            Acquire(void);
    //Release the decoder
    __override void            Release(void);
    //Whether a decoder has been acquired
    __override long            IsAcquired(void);

    //Decoder control
    //inBuf/inSize contain optional parameters to the control
    //outBuf/outSize points to memory for optional returned data - if outSize is too small for the control, the call fails.
    __override DECODER_ERR     IoControl(DECODER_CONTROL control, void* inBuf=NULL, int inSize=0, void* outBuf=NULL, int outSize=0)
                               { return DECODER_ERR_NOT_IMPLEMENTED; }

    //Check decoder stall
    __override bool            CheckStall(void) { return false; }

    //Sets the current DRM decryption handle
    __override void            SetDRMHandle(uint32 drmHandle) {}
    //Sets the current DRM key id into the security core
    __override void            SetKeyId(__in_bcount(len) const byte* signedKeyId, uint32 len) {}
    //Sets the current DRM sample id into the security core
    __override void            SetSampleId(uint64 sampleId) {}

    //Check for parental control on this decoder
    __override int             CheckAccessControl(uint64 pts = 0) { return ACCESS_TIME_BOUNDARY::BLOCK_BLOCKED; }

    //Return current pts being rendererd on screen
    __override bool            GetCurrentPTS(uint64* pPTS) { return false; }

    //The decoder will expire at given STC
    __override bool            SetExpirationTime(bool ondestruct, uint64 pts) { return false; }

    //Return the fade parameter parsed out of the AD descriptor for this stream
    __override uint8           GetAudioDescriptionFade(void) { return 0; }

    //Initialization API
    __override void            OnSync(bool sync, bool teardownPicture, bool cleanStall);

    //Handle a RAP/sync
    __override void            OnRap(void);
    __override void            OnSyncPoint(void);
    //PTS DTS
    __override void            SetPTS(uint64 pts);
    __override uint64          GetPTS(void) const { return mPts; }
    __override void            SetDTS(uint64 dts);
    __override uint64          GetDTS(void) const { return mDts; }
    //Set duration of upcoming sample
    __override void            SetDuration(uint64 duration);
    //Set video frame size
    __override void            SetFrameSize(uint32 width, uint32 height) {}
    //Set the fade and pan for audio descriptor
    __override void            SetFadeAndPan(uint8 fade, int pan) {}
    //Set quality level
    __override void            SetQualityLevel(uint16 qualityLevel);
    //Write the given buffer to the decoder
    __override Buffer*         Write(Buffer* buffer);

    //Checks whether the decoder has finished rendering its stream
    __override bool            IsRenderingDone(uint64 stc) { return false; }

    //Let decoders know end of stream
    __override void            SetEndOfStream() { return; }

    //Send a event from a decoder to the outside world
    __override void            SetEventSink(IDecoderEventSink* eventSink);

    // === ITimeslice ===

    //Do work when the the time slice thread ticks
    __override void            OnTimeslice(void) {}

protected:
    //Write data to the Decoder
    virtual Buffer*            AddSample(Buffer* chain) = 0;

    //Send event to subscriber
    void                       SendEvent(DecoderEvent eventType, uint32 ref, const std::string& eventData);

protected:
    IReceiverControl*          mReceiverControl;  //Receiver control APIs
    IHalDecoderFactory*        mHalDecoderFactory;//HAL decoder factory to use
    IDecoderFactory*           mDecoderFactory;   //Decoder factory to use
    CRendererState&            mRendererState;    //Current rendering state
    CReceiverDiagnostics&      mDiagnostics;      //Diagnostics to use
    Clock&                     mClock;            //Clock to use
    uint32                     mPipeIdN;          //Pipe id

    CStreamInfo                mSI;               //We could use this reference here as well
    int                        mStreamType;       //Elementary stream type
    int                        mPid;              //Pid number for this stream
    bool                       mPtsAdjustor;      //Whether this stream is master of adjusting PTS timeline while splicing

    long                       mAcquired;         //Whether a HAL decoder has been acquired

    bool                       mDropFrame;        //Whether the next frame is being dropped
    bool                       mRap;              //Sample follows a RAP point
    bool                       mSyncPoint;        //Sample follows a sync point
    uint64                     mPts;              //Current pts
    uint64                     mDts;              //Current dts
    uint64                     mLastPts;          //Used for rolling over pts
    uint64                     mDuration;         //Duration of upcoming sample
    uint16                     mQualityLevel;     //Quality level

    int                        mLastBlock;        //Last parental control block state

    IDecoderEventSink*         mEventSink;        //Event sink to send events to
};

// ===============================================================================================================
// ===============================================================================================================
