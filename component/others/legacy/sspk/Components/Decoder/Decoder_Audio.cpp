///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Decoder_Audio.h"
#include "CDecoderConfiguration.h"
#include "IHalDecoder.h"
#include "StringUtils.h"
#include "Trace.h"

//#define DECODER_SPEW
#if defined(DECODER_SPEW)
#define DECODER_MSG(x) TRACE(x)
#else
#define DECODER_MSG(x)
#endif

// ===============================================================================================================
// ===============================================================================================================

//Threshold for detecting stale frames for inactive audio decoders
//Currently at 1/100 seconds
#define INACTIVE_AUDIO_THRESHOLD 900

// ===============================================================================================================
// ===============================================================================================================

DecoderAudio::DecoderAudio(IReceiverControl* receiverControl, const CStreamInfo& si)
    : DecoderDRM(receiverControl, si)
    , mFramedData(false)
//  , mDrmKeyId
    , mDrmKeyIdLen(0)
    , mDrmHandle(0)
    , mDrmSampleId(INVALID_SAMPLEID)
//  , mAudioParameters
    , mAudioParametersSet(false)
    , mFifoFirstFrame(NULL)
    , mFifoLastFrame(NULL)
    , mFirstFadePanQ(NULL)
    , mLastFadePanQ(NULL)
    , mFifoFirstFramePts(0)
    , mFifoLastFramePts(0)
    , mPtsDeltaBetweenFrames(0)
    , mInitialFramePts(INVALID_TIME)
    , mExpireAtStc(INVALID_TIME)
    , mParsedFade(0)
    , mParsedPan(0)
    , mCurrentFade(0)
    , mCurrentPan(0)
    , mPrevVolume(0)
    , mPrevPan(0)
    , mStallCount(0)
    , mLastFifoReadPtr(0)
    , mLastSentPts(INVALID_TIME)
    , mEndOfStream(false)
{
    //WMA is problematic - since it contains no start codes we have to deliver
    //it in complete frames or not at all.  Thus for WMA set mFramedData to true
    //to make sure that we send in complete frames only
    mFramedData = (mStreamType == StreamType_Audio_WMA || mStreamType == StreamType_Audio_WMAPRO || mStreamType == StreamType_Audio_WMA_WITH_HEADER);

    //Initialize the frame FIFO
    FrameFifoInit();
    //Initialize the volume/pan control FIFO
    FadePanQInit();

    //Register this decoder for the timeslice callback
    mReceiverControl->RegisterForTimeslice(this);
}

DecoderAudio::~DecoderAudio()
{
    //UnRegister this decoder from the timeslice callback
    mReceiverControl->UnRegisterForTimeslice(this);

    //Clear the FIFO
    FrameFifoClear();
    //Clear volume/pan control FIFO
    FadePanQClear();
}

bool DecoderAudio::Acquire(void)
{
    return true;
}

bool DecoderAudio::AcquireInternal(void)
{
    //Initialize decoder diagnostics
    mDecoderDiagnostics.OnInitialize(mStreamType, mPid);

    //Acquire a HAL Audio Decoder
    if (mCodec == INVALID_CODEC)
    {
        mDecoderDiagnostics.OnFailed();
        DECODER_MSG(("Failed to acquire Hardware Audio Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));
        return false;
    }

    IPTV_HAL_DECODER_ACQUIRE acqparams;
    acqparams.eCodecType = (IPTV_HAL_DECODER_CODECTYPE)mCodec;
    acqparams.eResolution = IPTV_HAL_DECODER_RESOLUTION_SD; // not used by audio decoders
    acqparams.eAudioType = mIsAudioDescription ? IPTV_HAL_DECODER_AUDIOTYPE_DESCRIPTION : IPTV_HAL_DECODER_AUDIOTYPE_NORMAL;
    iptv_hal_error errorCode;
    mDecoderContext = mHalDecoderFactory->AcquireAudioDecoder(mPipeIdN, acqparams, mClock.GetContext(), mAudioParametersSet ? &mAudioParameters : NULL, mAudioParametersSet ? sizeof(mAudioParameters) : 0, errorCode);
    if (!mDecoderContext)
    {
        //Send an event on failure during acquisition of HAL decoder
        mDecoderDiagnostics.OnFailed(errorCode);
        DECODER_MSG(("Failed to acquire Hardware Audio Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));
        mReceiverControl->NotifyStatus("status=decodererror&pkresult=" + toString(errorCode));
        return false;
    }
    DECODER_MSG(("Acquired Hardware Audio Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));

    if (mIsAudioDescription)
    {
        //Let the renderer know that it has the HAL audio context for AD
        mRendererState.SetAudioDescriptionContext(this);
        //This pipe owns the audio description now
        gDecoderConfiguration.SetPipeOwnsAudioDescription(mPipeIdN);
    }
    else
    {
        //Do special things when in fullscreen
        if (mRendererState.IsFullScreen)
        {
            mHalDecoderFactory->UpdateKaraokeState(mDecoderContext);
        }

        //Let the clock know that it has the HAL audio context
        mClock.SetAudioContext(this);
        //Let the renderer know that it has the HAL audio context
        mRendererState.SetAudioContext(this);
        //This pipe owns the audio now
        gDecoderConfiguration.SetPipeOwnsAudio(mPipeIdN);
    }

    //Set default volume and panning
    mCurrentFade = 0;
    mCurrentPan = 0;

    mPrevVolume = 100;
    CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
    IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_VOLUME, &mPrevVolume, sizeof(mPrevVolume));
    CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);

    mPrevPan = 0;
    if (mIsAudioDescription)
    {
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_PANNING, &mPrevPan, sizeof(mPrevPan));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);
    }

    bool endOfStream = false;
    IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_EOS, &endOfStream, sizeof(endOfStream));
    
    float playbackRate = mRendererState.PlaybackRate;
    IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_PLAYRATE, &playbackRate, sizeof(playbackRate));

    //Send an event on successful acquisition of HAL decoder
    mDecoderDiagnostics.OnAcquired(mDecoderContext);

    //Do not forget to call the base class
    return DecoderDRM::Acquire();
}

void DecoderAudio::Release(void)
{
    ReleaseInternal();
}

void DecoderAudio::ReleaseInternal(void)
{
    //Quit if no HAL Audio Decoder ever acquired
    if (!mDecoderContext)
        return;

    DECODER_MSG(("Releasing Hardware Audio Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));

    //Send an event on successful release of HAL decoder
    mDecoderDiagnostics.OnReleased();

    if (mIsAudioDescription)
    {
        //This pipe has given up audio description now
        gDecoderConfiguration.SetPipeOwnsAudioDescription(0);
        //Let the renderer know that it has lost the HAL audio context
        mRendererState.SetAudioDescriptionContext(NULL);
    }
    else
    {
        //This pipe has given up audio now
        gDecoderConfiguration.SetPipeOwnsAudio(0);
        //Let the clock know that it has lost the HAL audio context
        mClock.SetAudioContext(NULL);
        //Let the renderer know that it has lost the HAL audio context
        mRendererState.SetAudioContext(NULL);
    }

    //Release the HAL Audio Decoder
    mHalDecoderFactory->ReleaseAudioDecoder(mPipeIdN, mDecoderContext, mRendererState.FlushAudioDecoderOnRelease);
    //We are done with the IPTV HAL decoder
    mDecoderContext = NULL;

    //Reset AD descriptor parsing
    mParsedFade = 0;
    mParsedPan = 0;

    //Do not forget to call the base class
    DecoderDRM::Release();
}

DECODER_ERR DecoderAudio::IoControl(DECODER_CONTROL control, void* inBuf, int inSize, void* outBuf, int outSize)
{
    if (control == DECODER_SETAUDIOHEADER)
    {
        ASSERT(inBuf && inSize == sizeof(IPTV_HAL_DECODER_AUDIO_HEADER));
        if (inBuf && inSize == sizeof(IPTV_HAL_DECODER_AUDIO_HEADER))
        {
            mAudioParameters = *((IPTV_HAL_DECODER_AUDIO_HEADER*)inBuf);
            mAudioParametersSet = true;
            return DECODER_ERR_SUCCESS;
        }
        return DECODER_ERR_BAD_PARAMETER;
    }

    if (!mDecoderContext)
    {
        return DECODER_ERR_FAILED;
    }

    DECODER_ERR result = DECODER_ERR_SUCCESS;

    switch (control)
    {
    case DECODER_GETAUDIODIAGS:
        ASSERT(sizeof(DECODER_AUDIO_DIAGS) == sizeof(IPTV_HAL_DECODER_VALUE_AUDIO_DIAGS));
        if (outBuf && outSize == sizeof(IPTV_HAL_DECODER_VALUE_AUDIO_DIAGS))
        {
            if (IPTV_HAL_ERROR_RET_FAILED(IPTV_HAL_Decoder_GetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_DIAGS, outBuf, (uint32*)&outSize)))
            {
                result = DECODER_ERR_FAILED;
            }
        }
        else result = DECODER_ERR_BAD_PARAMETER;
        break;

    default:
        result = DECODER_ERR_NOT_IMPLEMENTED;
        break;
    }

    return result;
}

bool DecoderAudio::CheckStall(void)
{
    bool isStalled;
    uint32 isStalledSize = sizeof(isStalled);
    if (IPTV_HAL_ERROR_RET_FAILED(IPTV_HAL_Decoder_GetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_GET_DECODERSTALL, &isStalled, &isStalledSize)))
    {
        isStalled = false;
        TRACE(("Warning: IPTV_HAL_DECODER_VALUETYPE_STREAM_DECODERSTALL is not implemented, defaulting to false"));
    }

    if (isStalled)
    {
        TRACE_ERROR(("AUDIO DECODER STALL[%08x]", mPipeIdN));
    }

    return isStalled;
}

Buffer* DecoderAudio::AddSample(Buffer* chain)
{
    //No audio during immediate mode
    if (mRendererState.IsImmediateMode)
    {
        return Buffer::ReleaseChain(chain);
    }

    //Push the buffer into FIFO
    FrameFifoPush(chain, mPts, mDts, mRap);

    //If the clock hasn't started yet, call OnTimeslice() so that packets are pushed directly
    //from the FIFO without having to wait for the next timeslice
    if (!mClock.IsStarted())
    {
        OnTimeslice();
    }
    return NULL;
}

void DecoderAudio::SetKeyId(__in_bcount(len)const byte* signedKeyId, uint32 len)
{
    if (len > 0 && len <= DRM_KEYID_SIZE)
    {
        memcpy_s(mDrmKeyId, DRM_KEYID_SIZE, signedKeyId, mDrmKeyIdLen = len);
    }
}

void DecoderAudio::SetSampleId(uint64 sampleId)
{
    mDrmSampleId = sampleId;
}

void DecoderAudio::SetDRMHandle(uint32 drmHandle)
{
    mDrmHandle = drmHandle;
}

void DecoderAudio::OnSync(bool sync, bool teardownPicture, bool cleanStall)
{
    if (sync)
    {
        //DRM Key Id caching
        mDrmKeyIdLen = 0;
        //DRM Sample Id caching
        mDrmSampleId = INVALID_SAMPLEID;
        //DRM handle caching
        mDrmHandle = 0;

        //Reset AD descriptor parsing
        mParsedFade = 0;
        mParsedPan = 0;

        //Clear the FIFO
        FrameFifoClear();
        //Clear volume/pan control FIFO
        FadePanQClear();
    }

    //Do not forget to call the base class
    DecoderDRM::OnSync(sync, teardownPicture, cleanStall);
}

int DecoderAudio::CheckAccessControl(uint64 pts)
{
    //Return last remembered value for invalid pts
    if (IS_INVALID_TIME(pts))
        return mLastBlock;

    uint64 currentNtp = mClock.PTS2NTP(pts);

    //we've started (we knew our last block status), but somehow still
    //get a zero currentNtp, return the last blocking state.
    //
    //this happens on rewinding to the begining or fast forwarding to the end
    //of pause buffer, we've finished reading data but not yet finished showing
    //it.
    //
    //it could also happen when we are building PCRPTS delay buffer following
    //a decoder flush caused by discontinuity
    if ((mLastBlock != ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN) && ((currentNtp == 0) || IS_INVALID_TIME(currentNtp)))
    {
        CAccessControl::ResetSkipCounter();
        return mLastBlock;
    }

    //Now check with parental control logic
    int ret = mIsAudioDescription
                    ? mRendererState.AudioDescriptionAccessControl.CheckTimeBoundary(currentNtp, mLastBlock, mReceiverControl, mDecoderDiagnostics)
                    : mRendererState.AudioAccessControl.CheckTimeBoundary(currentNtp, mLastBlock, mReceiverControl, mDecoderDiagnostics);

    //keep reseting skip counter (so we'll not skip next check) until we know our block state
    if (mLastBlock == ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN)
    {
        CAccessControl::ResetSkipCounter();
    }

    return ret;
}

bool DecoderAudio::GetCurrentPTS(uint64* pPTS)
{
    if (mDecoderContext)
    {
        IPTV_HAL_DECODER_VALUE_AUDIO_CURRENTPTS currpts;
        uint32 size = sizeof(currpts);
        if (IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_GetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_CURRENTPTS, &currpts, &size)))
        {
            *pPTS = currpts.CurrentPTS;
            return true;
        }
    }
    *pPTS = 0;
    return false;
}

void DecoderAudio::FrameFifoInit(void)
{
    AutoLock lock(&mFifoLock);

    //Initialize frame FIFO
    mFifoFirstFrame = NULL;
    mFifoLastFrame = NULL;

    //Initialize frame FIFO diagnostics parameters
    mFifoFirstFramePts = 0;
    mFifoLastFramePts = 0;
    mPtsDeltaBetweenFrames = 0;

    mInitialFramePts = INVALID_TIME;
    mLastSentPts = INVALID_TIME;
    mEndOfStream = false;
}

void DecoderAudio::FrameFifoPush(Buffer* chain, uint64 pts, uint64 dts, bool rap)
{
    AutoLock lock(&mFifoLock);

    //Is this start of a new frame?
    if (IS_VALID_TIME(pts))
    {
        //Start a new frame
        CFrame* newFrame = new CFrame();
        if (newFrame)
        {
            //Set AD fade and pan from the most recently parsed values from the PES header
            newFrame->mFade = mParsedFade;
            newFrame->mPan = mParsedPan;
            newFrame->mTick = 0;
            newFrame->mTicksUntilUpdate = 0;
            newFrame->mNext = NULL;

            //Append the new frame at the end of the FIFO
            if (mFifoLastFrame)
            {
                //We have a discontinuity before the new frame so the last frame will
                //be incomplete only is it was not signalled to be the end of frame
                if (chain->GetFlag(BUFFER_DISCONTINUITY) && !mFifoLastFrame->mChain.IsEndFrame())
                {
                    //If we see a discontinuity before the start of a new frame and
                    //had not accumulated the last frame completely means that we
                    //have a bad PES frame.
                    //
                    //Note: we don't want to drop the last frame right here as it might contain
                    //      a valid key-id which shouldn't be dropped.
                    mFifoLastFrame->mBadFrame = true;
                }
                mFifoLastFrame->mNext = newFrame;
            }
            else
            {
                mFifoFirstFrame = newFrame;
                mFifoFirstFramePts = pts;
            }
            mFifoLastFrame = newFrame;

            //Save DRM Key Id
            if (mDrmKeyIdLen)
            {
                //Cache the new DRM Key Id
                mFifoLastFrame->mDrmKeyId = new CFrameKeyId();
                if (mFifoLastFrame->mDrmKeyId)
                {
                    //Save the new Key Id
                    mFifoLastFrame->mDrmKeyId->mDrmKeyIdLen = mDrmKeyIdLen;
                    memcpy_s(mFifoLastFrame->mDrmKeyId->mDrmKeyId, DRM_KEYID_SIZE, mDrmKeyId, mDrmKeyIdLen);
                    //We will consume the DRM Key Id when we cache it
                    mDrmKeyIdLen = 0;
                }
            }
            else
            {
                mFifoLastFrame->mDrmKeyId = NULL;
            }
            //Save DRM Sample Id
            mFifoLastFrame->mDrmSampleId = mDrmSampleId;
            //Save DRM handle
            mFifoLastFrame->mDrmHandle = mDrmHandle;
            //Save chain buffers
            mFifoLastFrame->mChain.SetBuffers( chain );
            mFifoLastFrame->mFrameSize = mFifoLastFrame->mChain.GetTotalSize(&mFifoLastFrame->mBufferCount);
            //Save all incoming parameters for this frame
            mFifoLastFrame->mPTS = pts;
            mFifoLastFrame->mDTS = dts;
            mFifoLastFrame->mBadFrame = false;

            //We are the last frame at this point
            mFifoLastFrame->mNext = NULL;

            //Keeping track of delta between frames
            //We do this if there are no expected discontinuities between frames
            if (!mFifoLastFrame->mBadFrame)
            {
                //Also clamp the delta to 1/2 second for now
                int64 delta = (int64)(pts - mFifoLastFramePts);
                if (delta >= 0 && delta < 45000)
                {
                    mPtsDeltaBetweenFrames = delta;
                }
            }
            //Keeping track of buffered data
            mFifoLastFramePts = pts;

            //We have consumed the incoming buffers
            return;
        }
    }
    else
    //Otherwise the buffers could belong to a frame already being built
    if (mFifoLastFrame)
    {
        //It is probably a bad frame if we have accumulated
        //too much data in one single PES frame
        if (mFifoLastFrame->mBufferCount > (uint32)CFrame::cBufferCountCap)
        {
            //If we see a discontinuity before the start of a new frame and
            //had not accumulated the last frame completely means that we
            //have a bad PES frame.
            //
            //Note: we don't want to drop the last frame right here as it might contain
            //      a valid key-id which shouldn't be dropped.
            mFifoLastFrame->mBadFrame = true;

            //We do not need to cache the rest of the buffers in this PES anymore
            //so fall through to release the buffers
        }
        else
        {
            //Is this buffer coming after a discontinuity
            if (chain->GetFlag(BUFFER_DISCONTINUITY))
            {
                //We'll set the discontinuity bit on the first buffer of that frame and
                //continue on.
                //
                //Note: we don't want to drop the last frame right here as it might contain
                //      a valid key-id which shouldn't be dropped.
                mFifoLastFrame->mBadFrame = true;
            }

            //Add the chain to the current PES frame being built
            mFifoLastFrame->mChain.Add(chain);
            mFifoLastFrame->mFrameSize = mFifoLastFrame->mChain.GetTotalSize(&mFifoLastFrame->mBufferCount);

            //We have consumed the incoming buffers
            return;
        }
    }

    //Otherwise we could not consume the incoming buffer
    //So just drop them and continue on
    Buffer::ReleaseChain(chain);
}

void DecoderAudio::FrameFifoPopActive()
{
    AutoLock lock(&mFifoLock);

    while (mReceiverControl->IsRunning() && mFifoFirstFrame)
    {
        //Keeping track amount of buffered data
        mFifoFirstFramePts = mFifoFirstFrame->mPTS;

        //We will not pop the head of frame FIFO until a new frame is available
        //
        //Note that more data for the frame at the head of FIFO could still arrive
        //and we do not want to throw the buffering algorithm completely offguard
        //when this happens.
        //
        //Thus wait until we have a new frame before popping the first one
        if (!mFifoFirstFrame->mNext)
        {
            return;
        }

        //Push DRM handle
        if (0 != mFifoFirstFrame->mDrmHandle)
        {
            if (NULL == mDecoderContext)
            {
                TRACE(("DecoderAudio::FrameFifoPopActive mDrmHandle without mDecoderContext set"));
            }
            else
            {
                IPTV_HAL_Decoder_SetValue(
                    mDecoderContext,
                    IPTV_HAL_DECODER_VALUETYPE_DRM_SETHANDLE,
                    &(mFifoFirstFrame->mDrmHandle),
                    sizeof(mFifoFirstFrame->mDrmHandle));

                DecoderDRM::SetDRMHandle(mFifoFirstFrame->mDrmHandle);
            }
        }

        //Push DRM Key Id
        if (mFifoFirstFrame->mDrmKeyId)
        {
            SetCryptoKeyId(mFifoFirstFrame->mDrmKeyId->mDrmKeyId, mFifoFirstFrame->mDrmKeyId->mDrmKeyIdLen);
        }
        //Push DRM Sample Id
        SetCryptoSampleId(mFifoFirstFrame->mDrmSampleId);

        //Check with clock
        uint32 frameSize = mFifoFirstFrame->mFrameSize;

        Clock::SampleAction action = mClock.AddAudioSample(
                                                mFifoFirstFrame->mPTS,
                                                mFifoLastFramePts,
                                                mIsAudioDescription );

        if ( action != Clock::eDeliverSample )
        {
            //Dump the frame at the head of the FIFO and move on to the next one
            FrameFifoPop();
            mDecoderDiagnostics.OnDropping(frameSize);
            continue;
        }

        // remember the timestamp of the first sample
        // sent to the hal
        if(IS_INVALID_TIME(mInitialFramePts))
        {
            mInitialFramePts = mFifoFirstFrame->mPTS;
        }

        //Return if we have already pushed enough data into the decoder
        //This is where we throttle the buffers pushed into the HAL decoder,  
        //which is only done once the buffer has been initially filled.
 
        if (!mClock.IsStarted() && 
            mFifoFirstFrame->mPTS - mInitialFramePts > gDecoderConfiguration.GetAudioHalBufferingThreshold())
        {
            return;
        }
        
        uint64 stc = mClock.CurrentStc();
        
        if (mClock.IsStarted() && 
            mFifoFirstFrame->mPTS - mInitialFramePts > gDecoderConfiguration.GetAudioHalBufferingThreshold() &&
            mFifoFirstFrame->mPTS > stc + gDecoderConfiguration.GetAudioHalBufferingThreshold())
        {
            return;
        }

        //Drop the frame if it is potentially a bad frame
        //We do this only for WMA because the hard decoder may not like
        //incomplete WMA PES frames
        if (mFramedData && mFifoFirstFrame->mBadFrame)
        {
            //Dump the frame at the head of the FIFO and move on to the next one
            FrameFifoPop();
            mDecoderDiagnostics.OnDiscontinuity(frameSize);
            continue;
        }

        //Check access control for audio - BLOCK_UNKNOWN is treated as blocked
        if (CheckAccessControl(mFifoFirstFrame->mPTS) != ACCESS_TIME_BOUNDARY::BLOCK_UNBLOCKED)
        {
            //Dump the frame at the head of the FIFO and move on to the next one
            FrameFifoPop();
            continue;
        }
        
        // remember the latest pts to know when rendering is done 
        // in case the HAL doesn't implement the rendering done flag
        mLastSentPts = mFifoFirstFrame->mPTS;

        mFifoFirstFrame->mChain.SetBuffers( WriteHALDecoder(mFifoFirstFrame->mChain.HeadBuffer(), mFifoFirstFrame->mPTS) );
        if (mFifoFirstFrame->mChain.GetChainFirst() != NULL)
        {
            //The only reason we are getting a non-null buffer back is decoder buffer is full,
            //so return and retry on next tick
            return;
        }

        //Pop the frame at the head of the FIFO and save off the volume control information
        CFrame* frame = FrameFifoPop(false);
        if (frame)
        {
            if (mIsAudioDescription)
            {
                //Mark when we're pushing this frame and when we will need to update the volume and panning.
                //Volume and panning happen instantly on the output, which is why we need to account for
                //the buffering delay. Using system time should be accurate enough and will avoid excessive
                //calls to the Decoder HAL to get the STC.
                frame->mTick = Executive_GetTickCount();
                int64 ptsdelta = (int64)(frame->mPTS - stc); // < 1s because of HalBufferingThreshold check
                frame->mTicksUntilUpdate = (ptsdelta > 0) ? (uint32)(ptsdelta / 90) : 0;

                //Queue up the volume control information.
                FadePanQPush(frame);
            }
            else
            {
                //There is no fade/pan information to save off for non-AD streams
                delete frame;
            }
        }
    }
}

void DecoderAudio::FrameFifoPopInactive(void)
{
    AutoLock lock(&mFifoLock);

    while (mReceiverControl->IsRunning() && mFifoFirstFrame)
    {
        //Keeping track amount of buffered data
        mFifoFirstFramePts = mFifoFirstFrame->mPTS;

        //We will not pop the head of frame FIFO until a new frame is available
        //
        //Note that more data for the frame at the head of FIFO could still arrive
        //and we do not want to throw the buffering algorithm completely offguard
        //when this happens.
        //
        //Thus wait until we have a new frame before popping the first one
        if (!mFifoFirstFrame->mNext)
            return;

        //Return if the frame at the head of FIFO is not yet stale
        //This is where we throttle the buffers in case a switch happens to this decoder
        if (!mClock.IsStarted() || mFifoFirstFrame->mPTS >= mClock.CurrentStc() + INACTIVE_AUDIO_THRESHOLD)
        {
            return;
        }

        //Push DRM handle
        if (0 != mFifoFirstFrame->mDrmHandle)
        {
            if (NULL == mDecoderContext)
            {
                TRACE(("DecoderAudio::FrameFifoPopInactive mDrmHandle without mDecoderContext set"));
            }
            else
            {
                IPTV_HAL_Decoder_SetValue(
                    mDecoderContext,
                    IPTV_HAL_DECODER_VALUETYPE_DRM_SETHANDLE,
                    &(mFifoFirstFrame->mDrmHandle),
                    sizeof(mFifoFirstFrame->mDrmHandle));

                DecoderDRM::SetDRMHandle(mFifoFirstFrame->mDrmHandle);
            }
        }

        //Push DRM Key Id
        if (mFifoFirstFrame->mDrmKeyId)
        {
            SetCryptoKeyId(mFifoFirstFrame->mDrmKeyId->mDrmKeyId, mFifoFirstFrame->mDrmKeyId->mDrmKeyIdLen);
        }
        //Push DRM Sample Id
        SetCryptoSampleId(mFifoFirstFrame->mDrmSampleId);

        //Otherwise dump the frame at the head of the FIFO and move on to the next one
        FrameFifoPop();
    }
}

CFrame* DecoderAudio::FrameFifoPop(bool deleteFrame)
{
    AutoLock lock(&mFifoLock);

    CFrame* frame = mFifoFirstFrame;
    if (frame)
    {
        //Move on to the next frame
        mFifoFirstFrame = frame->mNext;
        frame->mNext = NULL;
        //Release the buffer chain
        frame->mChain.Release();
        //Delete the key id
        if (frame->mDrmKeyId) delete frame->mDrmKeyId;
        frame->mDrmKeyId = NULL;
        //Delete the frame
        if (deleteFrame)
        {
            delete frame;
            frame = NULL;
        }
    }
        
    // send end of stream notification to the the HAL when no more samples are going to be sent
    if (mEndOfStream && (!mFifoFirstFrame || !mFifoFirstFrame->mNext))
    {
        mEndOfStream = false;

        if (mDecoderContext)
        {
            bool endOfStream = true;

            // It is ok for this call to fail since it is optional for the HAL to handle this 
            // if it can ensure the stc will advance to or beyond the mLastSentPts
            IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_EOS, &endOfStream, sizeof(endOfStream));
        }
    }
    return frame;
}

void DecoderAudio::FrameFifoClear(void)
{
    AutoLock lock(&mFifoLock);

    //Clear the frame FIFO
    while (mFifoFirstFrame)
    {
        FrameFifoPop();
    }

    //Reinitialize the FIFO controlling parameters
    FrameFifoInit();
}

bool DecoderAudio::SetExpirationTime(bool ondestruct, uint64 pts)
{
    AutoLock lock(&mFifoLock);

    //Quit right away if this decoder doesn't have the HAL audio decoder
    if (mDecoderContext == NULL)
        return false;

    //Quit right away if this is called while destructing and we are not in splicing mode
    if (ondestruct && mRendererState.FlushAudioDecoderOnRelease)
        return false;

    //Delete audio samples from decoder queue to match them up with video samples
    DeleteSamples(pts);

    //Quit right away since this decoder doesn't have any frames it needs to render
    if (mFifoLastFrame == NULL)
        return false;

    //Save it's expiry time
    mExpireAtStc = mFifoLastFramePts + mPtsDeltaBetweenFrames;

    //Give up this decoder to the receiver because we need
    //it to continue to play at given expiration time
    if (mIsAudioDescription)
    {
        mRendererState.SaveHangingAudioDescriptionDecoder(this);
    }
    else
    {
        mRendererState.SaveHangingAudioDecoder(this);
    }
    return true;
}

void DecoderAudio::DeleteSamples(uint64 pts)
{
    AutoLock lock(&mFifoLock);

    if (IS_INVALID_TIME(pts) || pts <= mPtsDeltaBetweenFrames)
    {
        TRACE(("[%08x][%d]: Dropping all samples [pts=%lld]", mPipeIdN, mStreamType, pts));

        //Clear the FIFO
        FrameFifoClear();
        //Clear volume/pan control FIFO
        FadePanQClear();
    }
    else
    {
        TRACE(("[%08x][%d]: Dropping sample after [pts=%lld][delta=%lld]", mPipeIdN, mStreamType, pts, mPtsDeltaBetweenFrames));

        //Allow for the average audio frame length
        pts -= mPtsDeltaBetweenFrames;

        //Skip frames until we reach the given pts time
        CFrame* frame = mFifoFirstFrame;
        while (frame && pts > frame->mPTS)
        {
            mFifoLastFramePts = frame->mPTS;
            mFifoLastFrame = frame;
            frame = frame->mNext;
        }

        //Update the first and last frame that are going
        //to be left over in the FIFO
        if (frame == mFifoFirstFrame)
        {
            mFifoFirstFrame = mFifoLastFrame = NULL;
        }
        else
        if (mFifoLastFrame)
        {
            mFifoLastFrame->mNext = NULL;
        }

        //Dump rest of audio frames in FIFO
        while (frame)
        {
            TRACE(("[%08x][%d]: Dropping sample at [%lld]", mPipeIdN, mStreamType, frame->mPTS));

            //Save the next frame in FIFO
            CFrame* next = frame->mNext;
            frame->mNext = NULL;
            //Release the buffer chain
            frame->mChain.Release();
            //Delete the key id
            if (frame->mDrmKeyId) delete frame->mDrmKeyId;
            frame->mDrmKeyId = NULL;
            //Delete the frame
            delete frame;
            //And go to the next one
            frame = next;
        }
    }
}

void DecoderAudio::FadePanQInit(void)
{
    AutoLock lock(&mFifoLock);

    //Initialize volume control FIFO
    mFirstFadePanQ = NULL;
    mLastFadePanQ = NULL;
}

void DecoderAudio::FadePanQPush(CFrame* frame)
{
    AutoLock lock(&mFifoLock);

    if (mLastFadePanQ)
    {
        mLastFadePanQ->mNext = frame;
        mLastFadePanQ = frame;
    }
    else
    {
        mFirstFadePanQ = frame;
        mLastFadePanQ = frame;
    }
}

void DecoderAudio::FadePanQUpdate(void)
{
    AutoLock lock(&mFifoLock);
    if (!mFirstFadePanQ)
        return;

    uint32 now = Executive_GetTickCount();
    while (mFirstFadePanQ)
    {
        CFrame* frame = mFirstFadePanQ;
        //If there's less than half a timeslice interval to go, update fade/pan now
        if (now - frame->mTick + 16 > frame->mTicksUntilUpdate)
        {
            //New fade/pan
            mCurrentFade = frame->mFade;
            mCurrentPan = frame->mPan;
            //Pop the current frame, remove it, and check the next one
            mFirstFadePanQ = mFirstFadePanQ->mNext;
            if (mFirstFadePanQ == NULL)
            {
                mLastFadePanQ = NULL;
            }
            delete frame;
        }
        else
        {
            break;
        }
    }
}

void DecoderAudio::FadePanQClear(void)
{
    AutoLock lock(&mFifoLock);

    //Clear the volume control FIFO
    while (mFirstFadePanQ)
    {
        CFrame* frame = mFirstFadePanQ;
        mFirstFadePanQ = mFirstFadePanQ->mNext;
        delete frame;
    }
    mLastFadePanQ = NULL;
}

uint8 DecoderAudio::GetAudioDescriptionFade(void)
{
    //First check the time to see if the current fade value needs updated
    FadePanQUpdate();
    //and then return it
    return mCurrentFade;
}

void DecoderAudio::SetFadeAndPan(uint8 fade, int pan)
{
    //Parsed from AD descriptor in PES header
    mParsedFade = fade;
    //Parsed from AD descriptor in PES header
    mParsedPan = (int8) (pan & 0xFF);
}

//Update volume and panning in the DecoderHAL on every timeslice. Changes from the UI
//propogate right away, but changes due to the AD descriptor must be timed so that they
//take effect when the sample to which it is attached is playing. This is accomplished
//through the FadePanQ in the AD decoder.
void DecoderAudio::UpdateVolumeAndPan(void)
{
    //Relative volume, range of -100 to 100
    int volume = mRendererState.GetAudioDescriptionVolume();

    if (mIsAudioDescription)
    {
        //Update the current fade/pan values
        FadePanQUpdate();

        //Translate the relative volume to an absolute one for AD
        if (volume < 0)
        {
            //Negative means attenuate AD, clamp from 0 to 100
            volume = (uint8)((volume < -100) ? 0 : 100+volume);
        }
        else
        {
            //Positive means AD at full volume, attenuate main
            volume = 100;
        }

        //Check for panning changes only for AD
        if (mCurrentPan != mPrevPan)
        {
            DECODER_MSG(("Updating pan: pid %d pan %d prev %d", mPid, mCurrentPan, mPrevPan));
            mPrevPan = mCurrentPan;

            //Panning factor in decibels, 1 dB per step is a good approximation to the stereophonic
            //law of sines cited in BBC R&D Whitepaper WHP051, "Audio Description: what it is and
            //how it works" (Tanton, et al, revised July 2004). For values beyond the +/-21 range,
            //mute the opposite channel.
            int scaledPan;
            if (mCurrentPan >= 21)
                scaledPan = 100;
            else if (mCurrentPan <= -21)
                scaledPan = -100;
            else
                scaledPan = (int)mCurrentPan << 2;

            CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
            IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_PANNING, &scaledPan, sizeof(scaledPan));
            CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);
        }
    }
    else
    {
        //Translate the relative volume to an absolute one for main audio
        if (volume > 0)
        {
            //Positive means attenuate main, clamp from 0 to 100
            volume = (volume > 100) ? 0 : 100-volume;
        }
        else
        {
            //Negative means main at full volume, attenuate AD
            volume = 100;
        }

        //Get fade from AD descriptor and map it to a volume scale. Main audio must query
        //the AD decoder because the current fade value is tracked by the FadePanQ there.
        uint8 fade = mRendererState.GetAudioDescriptionFade();

        //Further adjust volume by fade amount. Since volume and fade are in dB, the attenuation
        //is added instead of multiplied. Volume is in 0.25 dB steps, but the fade descriptor is
        //in 0.3 dB steps, so scale fade by 0.3/0.25 (6/5).
        //
        //Example: User/app wants to attenuate main audio by 5 dB, so AD volume is set to 120,
        //which makes main volume 80 (100 - 5 dB / 0.25 dB = 80). The current AD fade descriptor
        //is set to 10, specifying 3 dB of additional attenuation on main audio (10 * 0.3dB).
        //This is scaled to 12 0.25dB steps, so the final volume is reduced to 68. This corresponds
        //to a total attenuation of (100-68) * 0.25dB = 8 dB = 5 dB + 3 dB.
        volume -= (12 * (int)fade + 5) / 10;
        volume = (volume < 0) ? 0 : volume;
    }

    //Update the calculated volume if it's changed
    if ((uint32)volume != mPrevVolume)
    {
        DECODER_MSG(("Updating volume: pid %d volume %d prev %d", mPid, volume, mPrevVolume));
        mPrevVolume = (uint32)volume;

        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_VOLUME, &volume, sizeof(volume));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);
    }
}

void DecoderAudio::OnTimeslice(void)
{
    //Check if we need to acquire the HAL audio decoder because we are now activated
    //But we also need to wait until the currently active audio decoder has dropped the HAL decoder
    if (mDecoderContext == NULL)
    {
        bool shouldAcquire = false;
        if (mIsAudioDescription)
        {
            //Should the given audio pid acquire the audio renderer
            shouldAcquire = gDecoderConfiguration.CanPipeOwnAudioDescription(mPipeIdN) && mRendererState.AudioDescriptionLanguage.IsUsedPid(mPid);
        }
        else
        {
            //Should the given audio pid acquire the audio renderer
            shouldAcquire = gDecoderConfiguration.CanPipeOwnAudio(mPipeIdN) && mRendererState.AudioLanguage.IsUsedPid(mPid);
        }
        if (shouldAcquire)
        {
            //Acquire the decoder
            AcquireInternal();
            //Skip the frames which may already be stale
            FrameFifoPopInactive();
            //Notify clock that we've changed audio streams
            mClock.OnStreamChange(true);
        }
    }
    else
    //Check if we need to release the HAL decoder because we have been deactivated
    if (mDecoderContext != NULL)
    {
        bool shouldRelease = false;
        //Check if owner pipe id has changed - we release the decoder right away if so
        if (!gDecoderConfiguration.DoesPipeOwnAudio(mPipeIdN))
        {
            shouldRelease = true;
        }
        else
        //Check if the expiry was set and it has actually expired
        if (IS_VALID_TIME(mExpireAtStc))
        {
            shouldRelease = mClock.HasPtsExpired(mExpireAtStc);
        }
        else
        //Check if we need to release the HAL decoder because a different audio decoder
        //has been activated by application
        if (mIsAudioDescription)
        {
            if (!mRendererState.AudioDescriptionLanguage.IsUsedPid(mPid))
            {
                shouldRelease = mClock.HasPtsExpired(mExpireAtStc);
            }
        }
        else
        {
            if (!mRendererState.AudioLanguage.IsUsedPid(mPid))
            {
                shouldRelease = mClock.HasPtsExpired(mExpireAtStc);
            }
        }
        if (shouldRelease)
        {
            mExpireAtStc = INVALID_TIME;
            OnSync(false, false, false);
            ReleaseInternal();
        }
    }

    //Handle FIFO
    if (mDecoderContext)
    {
        //Process queued volume and panning controls and update them in DecoderHAL
        UpdateVolumeAndPan();
        //And throttle audio data to the HAL decoder based on our buffering limit
        FrameFifoPopActive();
    }
    else
    {
        //And throttle the inactive decoder frame FIFO by dumping stale frames
        FrameFifoPopInactive();
    }
}

void DecoderAudio::SetEndOfStream()
{
    AutoLock lock(&mFifoLock);

    // let the HAL know EoS when no more samples are going to be sent, otherwise take note of it to tell it later
    if (!mFifoFirstFrame || !mFifoFirstFrame->mNext)
    {
        if (mDecoderContext)
        {
            bool endOfStream = true;

            // It is ok for this call to fail since it is optional for the HAL to handle this 
            // if it can ensure the stc will advance to or beyond the mLastSentPts
            IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_EOS, &endOfStream, sizeof(endOfStream));
        }
    }
    else
    {
        mEndOfStream = true;
    }
}

bool DecoderAudio::IsRenderingDone(uint64 stc)
{
    bool isDone = true;
    uint32 isDoneSize = sizeof(isDone);

    if (mDecoderContext)
    {
         // check if the HAL indicates that rendering is done
        if(IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_GetValue(mDecoderContext, 
                                                                IPTV_HAL_DECODER_VALUETYPE_STREAM_GET_RENDERDONE, 
                                                                &isDone, 
                                                                &isDoneSize)))
        {
            DECODER_MSG(("DecoderAudio::IsRenderingDone HAL signaled %s", 
                isDone ? "TRUE" : "FALSE"));
        }
        // Check if the stc has moved past the last frame pts that was pushed
        else if (IS_VALID_TIME(mLastSentPts))
        {   
            DECODER_MSG(("DecoderAudio::IsRenderingDone: %s (pts:%lld stc=%lld)", 
                (mLastSentPts <= stc) ? "TRUE" : "FALSE", mLastSentPts, stc));

            isDone = (mLastSentPts <= stc);
        }
        else
        {
            isDone = true;
        }
    }

    return isDone;
}

uint64 DecoderAudio::GetLastPtsSent() const
{
    return mLastSentPts;
}
// ===============================================================================================================
// ===============================================================================================================
