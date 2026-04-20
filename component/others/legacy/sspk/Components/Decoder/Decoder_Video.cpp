///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Decoder_Video.h"
#include "CDecoderConfiguration.h"
#include "IDrmManager.h"
#include "IHalDecoder.h"
#include "CPipeId.h"
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

DecoderVideo::DecoderVideo(IReceiverControl* receiverControl, const CStreamInfo& si)
    : DecoderDRM(receiverControl, si)
    , mPUDHandler(NULL)
    , mWmvParametersSet(false)
    , mIFramePts(INVALID_TIME)
    , mDiscardNextSample(false)
    , mFingerprintFlagsLast(0)
    , mStallCount(0)
    , mLastFifoReadPtr(0)
    , mFrameWidth(0)
    , mFrameHeight(0)
    , mLastSentPts(INVALID_TIME)
{
}

DecoderVideo::~DecoderVideo()
{
    ASSERT(mPUDHandler == NULL);
}

bool DecoderVideo::Acquire(void)
{
    //Initialize decoder diagnostics
    mDecoderDiagnostics.OnInitialize(mStreamType, mPid);

    //Acquire a HAL Video decoder
    if (mCodec == INVALID_CODEC)
    {
        mDecoderDiagnostics.OnFailed();
        DECODER_MSG(("Failed to acquire Hardware Video Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));
        return false;
    }

    IPTV_HAL_DECODER_ACQUIRE acqparams;
    acqparams.eCodecType = (IPTV_HAL_DECODER_CODECTYPE)mCodec;
    acqparams.eResolution = mRendererState.IsFullScreen ? IPTV_HAL_DECODER_RESOLUTION_HD : IPTV_HAL_DECODER_RESOLUTION_PIP;
    acqparams.eAudioType = IPTV_HAL_DECODER_AUDIOTYPE_NORMAL; //Not used by video decoders
    iptv_hal_error errorCode;
    mDecoderContext = mHalDecoderFactory->AcquireVideoDecoder(mPipeIdN, acqparams, mClock.GetContext(), mWmvParametersSet ? &mWmvParameters : NULL, mWmvParametersSet ? sizeof(mWmvParameters) : 0, errorCode);
    if (!mDecoderContext)
    {
        //Send an event on failure during acquisition of HAL decoder
        mDecoderDiagnostics.OnFailed(errorCode);
        DECODER_MSG(("Failed to acquire Hardware Video Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));
        mReceiverControl->NotifyStatus("status=decodererror&pkresult=" + toString(errorCode));
        return false;
    }
    DECODER_MSG(("Acquired Hardware Video Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));

    //Set playmode mode if the decoder should come up in I frame rendering mode only
    DECODER_MSG(("[%08x] Setting ImmediateMode=%d IFrameOnlyMode=%d IsMHEGMode=%d", mPipeIdN, mRendererState.IsImmediateMode, mRendererState.IsIFrameOnlyMode, mRendererState.IsMHEGMode));

    float playbackRate = mRendererState.PlaybackRate;
    IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_PLAYRATE, &playbackRate, sizeof(playbackRate));

    CE_AV_DECODER_LOG(CE_AV_DECODER_SETPLAYMODE);
    IPTV_HAL_Decoder_SetPlayMode(mDecoderContext, mRendererState.IsImmediateMode || mRendererState.IsIFrameOnlyMode || mRendererState.IsMHEGMode);
    CE_AV_DECODER_LOG(CE_AV_DECODER_SETPLAYMODE_DONE);

    bool endOfStream = false;
    IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_EOS, &endOfStream, sizeof(endOfStream));

    //Picture user Data handler
    if (mRendererState.IsFeatureEnabled(CRendererState::eFeature_ClosedCaptioning))
    {
        mPUDHandler = IPUDHandler::Create(mReceiverControl, mSI, mDecoderContext);
        CHECK_ALLOC(mPUDHandler);
    }

    //Other initializations
    if (mRendererState.IsFeatureEnabled(CRendererState::eFeature_VideoOPL))
    {
        //Reset fingerprinting value
        mFingerprintFlagsLast = 0;

        //Initialize decoder's macrovision/cgms-a level
        uint32 macrovisionLevel = 0;
        mRendererState.OPL_GetMacrovision(macrovisionLevel);
        TRACE(("Set default Macrovision value in Decoder Video = %x", (int)macrovisionLevel));

        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_MACROVISION, &macrovisionLevel, sizeof(uint32));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);

        uint32 cgmsaLevel = 0;
        mRendererState.OPL_GetCGMSA(cgmsaLevel);
        TRACE(("Set default CGMS/A value in Decoder Video = %x", (int)cgmsaLevel));

        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_CGMSA, &cgmsaLevel, sizeof(uint32));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);

        //Turn on line21 passthrough upon codec creation
        IPTV_HAL_DECODER_VALUE_VIDEO_CCPASSTHROUGH pass_through;
        pass_through.bEnableCCPassthrough = !mReceiverControl->IsTrickMode();
        TRACE(("Set passthrough mode = %x", pass_through.bEnableCCPassthrough));

        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_CCPASSTHROUGH, &pass_through, sizeof(IPTV_HAL_DECODER_VALUE_VIDEO_CCPASSTHROUGH));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);
    }

    //Is the video blocked by parental control?
    if (mRendererState.IsFullScreen)
    {
        mDecoderFactory->NotifyFullscreenBlockStatus(CheckAccessControl() == ACCESS_TIME_BOUNDARY::BLOCK_BLOCKED);
    }

    //Update the clock start frame delay in renderer for PIP
    mClock.SetClockStartFrameDelay(mStreamType);
    //Let the clock know that it has the HAl Video context
    mClock.SetVideoContext(this);
    //Let the renderer know that it has the HAL Video context
    mRendererState.SetVideoContext(this);
    //Register video decoder with the renderer
    mDecoderFactory->RegisterDecoder(mPipeIdN, this, mDecoderContext);
    //Send an event on successful acquisition of HAL decoder
    mDecoderDiagnostics.OnAcquired(mDecoderContext);

    //Do not forget to call the base class
    return DecoderDRM::Acquire();
}

void DecoderVideo::Release()
{
    //Quit if no HAL Video Decoder ever acquired
    if (!mDecoderContext)
        return;

    DECODER_MSG(("Releasing Hardware Video Decoder for %s with pid=%d, steamType=%d, pipe=%08x", CHalDecoder::CodecName(mCodec), mPid, mStreamType, mPipeIdN));

    //Send an event on successful release of HAL decoder
    mDecoderDiagnostics.OnReleased();
    //Unregister video decoder with the renderer
    mDecoderFactory->UnRegisterDecoder(mPipeIdN, this, mDecoderContext);
    //Let the clock know that it has lost the HAl Video context
    mClock.SetVideoContext(NULL);
    //Let the renderer know that it has lost the HAL Video context
    mRendererState.SetVideoContext(NULL);

    //Dump the Picture user Data handler
    if (mPUDHandler)
    {
        delete mPUDHandler;
        mPUDHandler = NULL;
    }

    //Release the HAL Video Decoder
    mHalDecoderFactory->ReleaseVideoDecoder(mPipeIdN, mDecoderContext, mRendererState.FlushVideoDecoderOnRelease, mLastBlock == ACCESS_TIME_BOUNDARY::BLOCK_BLOCKED || mRendererState.TearDownPicture);
    //We are done with the IPTV HAL decoder
    mDecoderContext = NULL;

    //Do not forget to call the base class
    DecoderDRM::Release();
}

DECODER_ERR DecoderVideo::IoControl(DECODER_CONTROL control, void* inBuf, int inSize, void* outBuf, int outSize)
{
    if (control == DECODER_SETWMVHEADER)
    {
        ASSERT(inBuf && inSize == sizeof(IPTV_HAL_DECODER_WMV_HEADER));
        if (inBuf && inSize == sizeof(IPTV_HAL_DECODER_WMV_HEADER))
        {
            mWmvParameters = *((IPTV_HAL_DECODER_WMV_HEADER*)inBuf);
            mWmvParametersSet = true;
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
    case DECODER_HASRENDEREDFIRSTFRAME:
        if (inSize == sizeof(bool))
        {
            //Current implemenation uses PTS
            uint64 pts = 0;
            *((bool*)inBuf) = GetCurrentPTS(&pts) && pts != 0;
        }
        else result = DECODER_ERR_BAD_PARAMETER;
        break;

    case DECODER_CCREGISTER_TRIGGER:
        if (inSize == sizeof(uint32))
        {
            if (mPUDHandler)
            {
                mPUDHandler->EnableTriggers(*((const uint32*)inBuf) == 1);
            }
            else result = DECODER_ERR_FAILED;
        }
        else result = DECODER_ERR_BAD_PARAMETER;
        break;

    case DECODER_GETPICTUREINFO:
        ASSERT(sizeof(DECODER_VIDEO_PICTUREINFO) == sizeof(IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO));
        if (outBuf && outSize == sizeof(IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO))
        {
            if (IPTV_HAL_ERROR_RET_FAILED(IPTV_HAL_Decoder_GetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO, outBuf, (uint32*)&outSize)))
            {
                result = DECODER_ERR_FAILED;
            }
        }
        else result = DECODER_ERR_BAD_PARAMETER;
        break;

    case DECODER_GETVIDEODIAGS:
        ASSERT(sizeof(DECODER_VIDEO_DIAGS) == sizeof(IPTV_HAL_DECODER_VALUE_VIDEO_DIAGS));
        if (outBuf && outSize == sizeof(IPTV_HAL_DECODER_VALUE_VIDEO_DIAGS))
        {
            if (IPTV_HAL_ERROR_RET_FAILED(IPTV_HAL_Decoder_GetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_DIAGS, outBuf, (uint32*)&outSize)))
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

bool DecoderVideo::CheckStall(void)
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
        TRACE_ERROR(("VIDEO DECODER STALL[%08x]", mPipeIdN));
    }

    return isStalled;
}

Buffer* DecoderVideo::AddSample(Buffer* chain)
{
    //No decoder means just release the buffers
    if (!mDecoderContext)
    {
        return Buffer::ReleaseChain(chain);
    }

    //In immediate mode we send in only video I-frames and discard P and B.
    if (mRendererState.IsImmediateMode)
    {
        //PTS transitions are used to detect I frames
        //Also in immediate mode we use the PTS of first I-frame to start the clock.

        //Use rap indication to find I-frames
        if (mRap)
        {
            //I-Frame starts here in the GOP
            if (IS_VALID_TIME(mPts))
            {
                mIFramePts = mPts;
                mDiscardNextSample = false;
            }
        }
        else
        {
            if (IS_VALID_TIME(mPts) && (mIFramePts != mPts))
            {
                mDiscardNextSample = true;
            }
        }
        if (mDiscardNextSample)
        {
            //We discard any part of the P/B frame that could sneak in since we read full RTP
            //packets from the DVRFS. If we put the decoder in I-frame mode, it would ignore
            //them any way. But just being strict here.
            return Buffer::ReleaseChain(chain);
        }
    }

    uint64 origPts = mPts;
    Clock::SampleAction action = mClock.AddVideoSample( &mPts );

    int len = chain->Mark;

    if( action == Clock::eDeliverSample )
    {
        IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX   pictInfo;

        pictInfo.hdtDTS = (IPTV_HAL_DECODER_TIME)mDts;
        pictInfo.hdtDuration = mDuration;
        pictInfo.u16QualityLevel = mQualityLevel;
        pictInfo.u16Height = (uint16)mFrameHeight;
        pictInfo.u16Width = (uint16)mFrameWidth;
        pictInfo.u16Aspx = 1;
        pictInfo.u16Aspy = 1;
        pictInfo.u32Flags =
            (mRap ? IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX_FLAGS_RAP : 0) |
            (mSyncPoint ? IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX_FLAGS_SYNC : 0);

        IPTV_HAL_Decoder_SetValue(
            mDecoderContext,
            IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO_EX,
            &pictInfo,
            sizeof(pictInfo) );

        // during trickplay send the original pts, since the pts sent in IPTV_HAL_Decoder_PrepareForDecode
        // is synthesized for pipelines that can only play at 1x
        if( mRendererState.IsImmediateMode )
        {
            IPTV_HAL_Decoder_SetValue(
                mDecoderContext,
                IPTV_HAL_DECODER_VALUETYPE_VIDEO_ORIGPTS,
                &origPts,
                sizeof(origPts) );
            HeartBeatEvent(origPts);
        }

        // remember the latest pts to know when rendering is done 
        // in case the HAL doesn't implement the rendering done flag
        if(IS_VALID_TIME(mPts))
        {
            if(IS_INVALID_TIME(mLastSentPts) || mPts > mLastSentPts)
            {
                mLastSentPts = mPts;
            }
        }

        chain = WriteHALDecoder(chain, mPts);
    }
    else if( action == Clock::eDiscardSample )
    {
        //Track decoder state
        mDecoderDiagnostics.OnDropping(len);
        //Dump the chain
        chain = Buffer::ReleaseChain(chain);
    }

    return chain;
}

void DecoderVideo::SetKeyId(__in_bcount(len) const byte* signedKeyId, uint32 len)
{
    SetCryptoKeyId(signedKeyId, len);
}

void DecoderVideo::SetSampleId(uint64 sampleId)
{
    SetCryptoSampleId(sampleId);
}

void DecoderVideo::SetDRMHandle(uint32 drmHandle)
{
    DecoderDRM::SetDRMHandle(drmHandle);

    if (NULL == mDecoderContext)
    {
        TRACE(("DecoderVideo::SetDRMHandle called without mDecoderContext set"));
    }
    else
    {
        IPTV_HAL_Decoder_SetValue(
            mDecoderContext,
            IPTV_HAL_DECODER_VALUETYPE_DRM_SETHANDLE,
            &drmHandle,
            sizeof(drmHandle));
    }
}

void DecoderVideo::OnSync(bool sync, bool teardownPicture, bool cleanStall)
{
    //Reset fingerprinting value
    mFingerprintFlagsLast = 0;

    //Do not forget to call the base class
    DecoderDRM::OnSync(sync, teardownPicture, cleanStall);
}

void DecoderVideo::SetPassThroughAfterDecryption(void)
{
    //We do this only for fullscreen
    if (!mRendererState.IsFeatureEnabled(CRendererState::eFeature_VideoOPL))
        return;

    //Extract the CGMS-A/Macrovision/Finger print parameters from ECM
    IDrmDecrypter::SecurityFlags flags;
    if (mDRMContext && IPTV_HAL_ERROR_RET_SUCCESS(mDRMContext->GetSecurityFlags(&flags)))
    {
        //Bit 6 should be 1 and bit 5 should be 0 when CGMS/A info is present
        uint32 value = static_cast<uint32>(flags.CGMS_AFlags);
        if ((value & 0x60) == 0x40)
        {
            //Return value of bit 4 and 3
            mRendererState.OPL_SetECMCGMSA((value & 0x18) >> 3);
        }

        //Set the macrovision setting if it has changed
        mRendererState.OPL_SetECMMacrovision(static_cast<uint32>(flags.MacrovisionFlags));

        //Handle fingerprint
        byte fingerprintLevel = flags.FingerprintFlags;
        if (mFingerprintFlagsLast != fingerprintLevel)
        {
            mFingerprintFlagsLast = fingerprintLevel;
            mReceiverControl->NotifyStatus("status=fingerprint&state=" + toString((int)fingerprintLevel));
            DECODER_MSG(("Setting Finger Printing to:%d", mFingerprintFlagsLast));
        }
    }

    //We will be applying worst case Macrovision setting soming from
    //either of WSS or ECM or from DRM (PlayReady or WMDRM)
    uint32 macrovisionLevel;
    if (mRendererState.OPL_GetMacrovision(macrovisionLevel))
    {
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_MACROVISION, &macrovisionLevel, sizeof(macrovisionLevel));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);

        DECODER_MSG(("Setting Macrovision to:%d", macrovisionLevel));
    }

    //We will be applying worst case CGMS-A setting soming from
    //either of WSS or ECM or from DRM (PlayReady or WMDRM)
    uint32 cgmsaLevel;
    if (mRendererState.OPL_GetCGMSA(cgmsaLevel))
    {
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_CGMSA, &cgmsaLevel, sizeof(cgmsaLevel));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);

        DECODER_MSG(("Setting CGMS/A to:%d", cgmsaLevel));
    }
}

int DecoderVideo::CheckAccessControl(uint64 pts)
{
    uint64 currentNtp = mClock.CurrentNtp(false);

    //We've started (we knew our last block status), but somehow still
    //get a zero currentNtp, return the last blocking state.
    //
    //This happens on rewinding to the begining or fast forwarding to the end
    //of pause buffer, we've finished reading data but not yet finished showing
    //it.
    //
    //It could also happen when we are building PCRPTS delay buffer following
    //a decoder flush caused by discontinuity
    if ((mLastBlock != ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN) && ((currentNtp == 0) || IS_INVALID_TIME(currentNtp)))
    {
        DECODER_MSG(("[%08x] AccessControl state unchanged. currentntp:%llx", mPipeIdN, currentNtp));
        CAccessControl::ResetSkipCounter();
        return mLastBlock;
    }

    int pre_last_block = mLastBlock;
    int ret = mRendererState.VideoAccessControl.CheckTimeBoundary(currentNtp, mLastBlock, mReceiverControl, mDecoderDiagnostics);

    //Keep reseting skip counter (so we'll not skip next check) until we know our block state
    if (mLastBlock == ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN)
    {
        CAccessControl::ResetSkipCounter();
    }

    //Switch line21 passthrough based on access control
    if (mLastBlock != pre_last_block && mRendererState.IsFeatureEnabled(CRendererState::eFeature_VideoOPL))
    {
        IPTV_HAL_DECODER_VALUE_VIDEO_CCPASSTHROUGH pass_through;
        pass_through.bEnableCCPassthrough = !mReceiverControl->IsTrickMode() && (mLastBlock == ACCESS_TIME_BOUNDARY::BLOCK_UNBLOCKED);

        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_CCPASSTHROUGH, &pass_through, sizeof(IPTV_HAL_DECODER_VALUE_VIDEO_CCPASSTHROUGH));
        CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);
    }
    return ret;
}

bool DecoderVideo::GetCurrentPTS(uint64* pPTS)
{
    if (mDecoderContext)
    {
        IPTV_HAL_DECODER_VALUE_VIDEO_CURRENTPTS currpts;
        uint32 size = sizeof(currpts);
        if (IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_GetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_CURRENTPTS, &currpts, &size)))
        {
            *pPTS = currpts.CurrentPTS;
            return true;
        }
    }
    *pPTS = 0;
    return false;
}

void DecoderVideo::SetEndOfStream()
{
    if (mDecoderContext)
    {
        bool endOfStream = true;

        // Let decoder know end of stream
        // Note: It is ok for this call to fail since it is optional for the HAL to handle this 
        // if it can ensure the stc will advance to or beyond the mLastSentPts
        IPTV_HAL_Decoder_SetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_EOS, &endOfStream, sizeof(endOfStream));
    }
}

bool DecoderVideo::IsRenderingDone(uint64 stc)
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
            DECODER_MSG(("DecoderVideo::IsRenderingDone HAL signaled %s", 
                isDone ? "TRUE" : "FALSE"));
        }
        // Check if the stc has moved past the last frame pts that was pushed
        else if (IS_VALID_TIME(mLastSentPts))
        {   
            DECODER_MSG(("DecoderVideo::IsRenderingDone: %s (pts:%lld stc=%lld)", 
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

uint64 DecoderVideo::GetLastPtsSent() const
{
    return mLastSentPts;
}

// ===============================================================================================================
// ===============================================================================================================
