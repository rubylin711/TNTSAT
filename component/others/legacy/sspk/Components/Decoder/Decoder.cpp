///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Decoder.h"
#include "IPTVDecoderHal.h"
#include "Trace.h"
#include <string>
using namespace std;

//#define DECODER_SPEW
#if defined(DECODER_SPEW)
#define DECODER_MSG(x) TRACE(x)
#else
#define DECODER_MSG(x)
#endif

// ===============================================================================================================
// Base Decoder class:
// Clock contexts can't be used except by the HAL implementation,
// so clocks should be kept as Clock until we call into the HAL.
// ===============================================================================================================

Decoder::Decoder(IReceiverControl* receiverControl, const CStreamInfo& si)
    : mReceiverControl(receiverControl)
    , mHalDecoderFactory(mReceiverControl->GetAVManager()->GetHalDecoderFactory())
    , mDecoderFactory(mReceiverControl->GetAVManager()->GetDecoderFactory())
    , mRendererState(mReceiverControl->GetRendererState())
    , mDiagnostics(mReceiverControl->GetDiagnostics())
    , mClock(mReceiverControl->GetClock())
    , mPipeIdN(mRendererState.mPipeIdN)
    , mSI(si)
    , mStreamType(mSI.Format)
    , mPid(mSI.StreamId)
    , mPtsAdjustor(mSI.IsVideo())
    , mAcquired(0)
    , mDropFrame(false)
    , mRap(false)
    , mSyncPoint(false)
    , mPts(INVALID_TIME)
    , mDts(INVALID_TIME)
    , mDuration(0)
    , mEventSink(NULL)
{
    //Used for rolling over pts
    mLastPts = INVALID_TIME;

    //Clear access control last blocking state
    mLastBlock = ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN;
    //Reset access control skip counter
    CAccessControl::ResetSkipCounter();
}

Decoder::~Decoder()
{
    //Sanity check
    ASSERT(mAcquired == 0);
}

bool Decoder::Acquire(void)
{
    //Decoder is acquired, switch the flag on
    InterlockedExchange(&mAcquired, 1);
    return true;
}

void Decoder::Release()
{
    //IMPORTENT
    //
    //Initialize fields used across acquire and release calls
    //Please note that we may be calling Acquire and Release
    //many times through the lifecycle of the decoder

    //Clear access control last blocking state
    mLastBlock = ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN;
    //Reset access control skip counter
    CAccessControl::ResetSkipCounter();

    //Decoder is released, switch the flag off
    InterlockedExchange(&mAcquired, 0);
}

long Decoder::IsAcquired()
{
    //Returns whether a decoder has been acquired
    return InterlockedCompareExchange(&mAcquired, 0, 2);
}

void Decoder::OnSync(bool sync, bool teardownPicture, bool cleanStall)
{
    if (sync)
    {
        //IMPORTANT
        //
        //Since we are resyncing to start of a new stream reinit
        //all parameters used across acquire/release as well

        //Whether next frame should be dropped
        mDropFrame = false;
        //Assume we are not starting with a RAP
        mRap = false;
        //Assume we are not starting with a sync point
        mSyncPoint = false;
        //Current pts
        mPts = INVALID_TIME;
        //Current dts
        mDts = INVALID_TIME;
        //Duration of upcoming sample
        mDuration = 0;
        //Used for rolling over pts
        mLastPts = INVALID_TIME;

        //Clear access control last blocking state
        mLastBlock = ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN;
        //Reset access control skip counter
        CAccessControl::ResetSkipCounter();
    }
}

void Decoder::OnRap(void)
{
    mRap = true;
}

void Decoder::OnSyncPoint(void)
{
    mSyncPoint = true;
}

void Decoder::SetPTS(uint64 pts)
{
    if (IS_VALID_TIME(pts))
    {
        if (IS_INVALID_TIME(mLastPts))
        {
            mLastPts = pts;
        }

        mPts = mLastPts = pts;

        //Make necessary adjustments to the PTS, as needed
        mDropFrame = mClock.AdjustPts(mPtsAdjustor, mPts);
    }
    else
    {
        mPts = pts;
    }
}

void Decoder::SetDTS(uint64 dts)
{
    mDts = dts;
}

void Decoder::SetDuration(uint64 duration)
{
    mDuration = duration;
}

void Decoder::SetQualityLevel(uint16 qualityLevel)
{
    mQualityLevel = qualityLevel;
}

Buffer* Decoder::Write(Buffer* buffer)
{
    Buffer* decode;

    //Check if we have been asked to drop the frame
    if (mDropFrame)
    {
        TRACE(("[%08x][%04x]: Dropping Buffer at [PTS=%lld][PS=%d][PE=%d][Enc=%s][EOF=%s][Disc=%s]",
                            mPipeIdN, mStreamType,
                            mPts,
                            buffer->HALBuffer.u32DataStart, 
                            buffer->Mark,
                            buffer->GetFlag(BUFFER_ENCRYPTED) ? "true" : "false",
                            buffer->GetFlag(BUFFER_ENDFRAME) ? "true" : "false",
                            buffer->GetFlag(BUFFER_DISCONTINUITY) ? "true" : "false"));

        //Dump the buffer chain
        decode = Buffer::ReleaseChain(buffer);
    }
    else
    {
        //Returns NULL if all buffers are consumed
        //Otherwise Buffer should be written again
        decode = AddSample(buffer);
    }
    //Was the buffer consumed
    //Clear out buffer write related fields
    if (!decode)
    {
        mRap = false;
        mSyncPoint = false;
        mPts = INVALID_TIME;
        mDts = INVALID_TIME;
        mDuration = 0;
    }
    return decode;
}

void Decoder::SetEventSink(IDecoderEventSink* eventSink)
{
    mEventSink = eventSink;
}

void Decoder::SendEvent(DecoderEvent eventType, uint32 ref, const string& eventData)
{
    if (mEventSink != NULL)
    {
        mEventSink->OnDecoderEvent(eventType, ref, eventData);
    }
}

// ===============================================================================================================
// ===============================================================================================================
