///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsManager.h"
#include "CHalDecoderFactory.h"
#include "CHalClock.h"
#include "CHalDecoder.h"
#include "CHalDecoderDiags.h"
#include "CPipeId.h"
#include "Trace.h"
#include <map>
using namespace std;

//#define DECODER_SPEW
#if defined(DECODER_SPEW)
#define DECODER_MSG(x) TRACE(x)
#else
#define DECODER_MSG(x)
#endif

// ===============================================================================================================
// ===============================================================================================================

CHalDecoderFactory::CHalDecoderFactory(IAVManager* avManager)
    : mAVManager(avManager)
    , mThreadEvent(CEvent::eResetModeAuto)
{
    //HAL decoders
    mHalVideoDecoder.clear();
    mHalAudioDecoder = NULL;
    mHalAudioDescriptionDecoder = NULL;

    //Simple definition only tunes
    mIsSimpleDefinitionOnlyTune = false;
    //Karaoke tunes
    mIsKaraokeTune = false;

    //Start the thread
    mThreadStopped = false;
    mThread.Start(this);
}

CHalDecoderFactory::~CHalDecoderFactory()
{
    //Stop the thread
    //The cleanup of all HAl decoders is done by the thread itself
    mThread.Stop();
    ASSERT(mThreadStopped);

}

// ===============================================================================================================
// HAL clock
// ===============================================================================================================

IHalClock* CHalDecoderFactory::AcquireHalClock(bool bAVSync)
{
    return new CHalClock(bAVSync);
}

LPVOID CHalDecoderFactory::AcquireVideoDecoder(uint32 pipeIdN, IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext, const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode)
{
    //Force SD only tune
    if (acqParams.eResolution != IPTV_HAL_DECODER_RESOLUTION_PIP && CHalDecoderFactory::mIsSimpleDefinitionOnlyTune)
    {
        acqParams.eResolution = IPTV_HAL_DECODER_RESOLUTION_SD;
    }

    //Grab the decoder we want
    CHalDecoder* halDecoder = NULL;
    {
        AutoLock lock(&mHalDecoderLock);

        //Check if the hal decoder already allocated
        halDecoder = mHalVideoDecoder[pipeIdN];
        //If not then create one and save it in our map
        if (!halDecoder)
        {
            bool reusable = (pipeIdN == CPipeId::PipeIdN_Fullscreen);
            TRACE(("AcquireVideoDecoder reusable:%d\n" ,reusable));
            halDecoder = new CHalDecoder(pipeIdN, reusable);
            CHECK_ALLOC(halDecoder);
            mHalVideoDecoder[pipeIdN] = halDecoder;
        }
    }

    //And create a new HAL decoder
    return halDecoder->Create(acqParams, pClockContext, header, headerLength, errorCode);
}

void CHalDecoderFactory::ReleaseVideoDecoder(uint32 pipeIdN, LPVOID pDecoderContext, bool flushDecoder, bool teardownPicture)
{
    AutoLock lock(&mHalDecoderLock);

    CHalDecoder* halDecoder = mHalVideoDecoder[pipeIdN];
    CHECK_ALLOC(halDecoder);
    halDecoder->IsFlushDecoder = flushDecoder;
    halDecoder->IsTeardownPicture = teardownPicture;
    halDecoder->IsReleasing = true;
    mThreadEvent.Set();
}

LPVOID CHalDecoderFactory::AcquireAudioDecoder(uint32 pipeIdN, IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext, const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode)
{
    //Grab the decoder we want
    CHalDecoder* halDecoder = NULL;
    {
        AutoLock lock(&mHalDecoderLock);

        //Check if this is a normal audio stream or an audio description stream
        if (acqParams.eAudioType == IPTV_HAL_DECODER_AUDIOTYPE_DESCRIPTION)
        {
            //Check if the hal decoder already allocated
            halDecoder = mHalAudioDescriptionDecoder;
            //If not then create one and save it in our map
            if (!halDecoder)
            {
                //AD decoders should not be cached so we can play UI sounds when AD is off
                TRACE(("AcquireAudioDecoder reusable false\n"));
                halDecoder = new CHalDecoder(pipeIdN, false);
                CHECK_ALLOC(halDecoder);
                mHalAudioDescriptionDecoder = halDecoder;
            }
        }
        else
        {
            //Check if the hal decoder already allocated
            halDecoder = mHalAudioDecoder;
            //If not then create one and save it in our map
            if (!halDecoder)
            {
                bool reusable = (pipeIdN == CPipeId::PipeIdN_Fullscreen);
                TRACE(("AcquireAudioDecoder reusable:%d\n",reusable));
                halDecoder = new CHalDecoder(pipeIdN, reusable);
                CHECK_ALLOC(halDecoder);
                mHalAudioDecoder = halDecoder;
            }
        }
    }

    //And create a new decoder
    return halDecoder->Create(acqParams, pClockContext, header, headerLength, errorCode);
}

void CHalDecoderFactory::ReleaseAudioDecoder(uint32 pipeIdN, LPVOID pDecoderContext, bool flushDecoder)
{
    AutoLock lock(&mHalDecoderLock);

    CHalDecoder* halDecoder = NULL;
    if (mHalAudioDecoder && mHalAudioDecoder->DecoderContext == pDecoderContext)
    {
        halDecoder = mHalAudioDecoder;
    }
    else
    if (mHalAudioDescriptionDecoder && mHalAudioDescriptionDecoder->DecoderContext == pDecoderContext)
    {
        halDecoder = mHalAudioDescriptionDecoder;
    }
    CHECK_ALLOC(halDecoder);
    halDecoder->IsFlushDecoder = flushDecoder;
    halDecoder->IsReleasing = true;
    mThreadEvent.Set();
}

bool CHalDecoderFactory::DecodersReleased(uint32 pipeIdN)
{
    AutoLock lock(&mHalDecoderLock);
    if (mHalVideoDecoder[pipeIdN] && mHalVideoDecoder[pipeIdN]->IsItAllocated())
    {
        return false;
    }

    if (mHalAudioDecoder && mHalAudioDecoder->IsItAllocated())
    {
        return false;
    }

    if (mHalAudioDescriptionDecoder && mHalAudioDescriptionDecoder->IsItAllocated())
    {
        return false;
    }
    return true;
}

bool CHalDecoderFactory::PurgeDecoders(uint32 pipeIdN)
{
    // PurgeDecoders depends on the asynchronous release to be done on another thread,
    // CHalDecoderFactory::OnThreadRun.  Waiting once will normally give the chance for the 
    // thread to run.  10 ms period was chosen to make the wait responsive enough, 
    // but still want to exit the wait in case the release never happens,
    // which is unexpected.
    const static uint32 c_WaitReleaseDecoders_msec = 10;
    const static uint32 c_LimitReleaseDecoders_msec = 100;

    uint32 totalWaitReleaseDecoders = 0;

    // wait for the release to be done on CHalDecoderFactory::OnThreadRun
    while(!DecodersReleased(pipeIdN))
    {
        if(totalWaitReleaseDecoders > c_LimitReleaseDecoders_msec)
        {
            TRACE(("PurgeDecoders waiting for ReleaseDecoders failed"));
            ASSERT(false);
            return false;
        }
        DECODER_MSG(("PurgeDecoders waiting for ReleaseDecoders ..."));
        Executive_Sleep(c_WaitReleaseDecoders_msec);
        totalWaitReleaseDecoders += c_WaitReleaseDecoders_msec;
    }

    {
        AutoLock lock(&mHalDecoderLock);
        if(mHalVideoDecoder[pipeIdN])
        {
            delete mHalVideoDecoder[pipeIdN];
            mHalVideoDecoder.erase(pipeIdN);
        }

        if(mHalAudioDecoder)
        {
            delete mHalAudioDecoder;
            mHalAudioDecoder = NULL;
        }

        if(mHalAudioDescriptionDecoder)
        {
            delete mHalAudioDescriptionDecoder;
            mHalAudioDescriptionDecoder = NULL;
        }
    }

    return true;
}

void CHalDecoderFactory::TeardownPicture(uint32 pipeIdN)
{
    AutoLock lock(&mHalDecoderLock);

    CHalDecoder* halDecoder = mHalVideoDecoder[pipeIdN];
    if (halDecoder)
    {
        halDecoder->TeardownPicture();
    }
}

void CHalDecoderFactory::OnThreadRun()
{
    //Flush the decoders that are released asynchronously
    while (!mThreadStopped)
    {
        mThreadEvent.Wait();

        CE_AV_HALFACTORY_LOG(CE_AV_HALFACTORY_START);
        {
            AutoLock lock(&mHalDecoderLock);
            for (map<uint32,CHalDecoder*>::iterator it = mHalVideoDecoder.begin(); it != mHalVideoDecoder.end(); ++it)
            {
                if ((*it).second)
                {
                    TRACE(("release HalVideoDecoder\n"));
                    (*it).second->Release();
                }
            }
            if (mHalAudioDecoder)
            {
                TRACE(("release HalAudioDecoder\n"));
                mHalAudioDecoder->Release();
            }
            if (mHalAudioDescriptionDecoder)
            {
                TRACE(("release HalAudioDescriptionDecoder\n"));
                mHalAudioDescriptionDecoder->Release();
            }
        }
        CE_AV_HALFACTORY_LOG(CE_AV_HALFACTORY_END);
    }

    //Dump all HAL decoders that have been allocated
    {
        AutoLock lock(&mHalDecoderLock);
        for (map<uint32,CHalDecoder*>::iterator it = mHalVideoDecoder.begin(); it != mHalVideoDecoder.end(); ++it)
        {
            if ((*it).second)
            {
                TRACE(("delete HalVideoDecoder\n"));
                delete (*it).second;
            }
        }
        mHalVideoDecoder.clear();
        if (mHalAudioDecoder)
        {
            TRACE(("delete HalAudioDecoder\n"));
            delete mHalAudioDecoder;
            mHalAudioDecoder = NULL;
        }
        if (mHalAudioDescriptionDecoder)
        {
            TRACE(("delete HalAudioDescriptionDecoder\n"));
            delete mHalAudioDescriptionDecoder;
            mHalAudioDescriptionDecoder = NULL;
        }
    }
}

void CHalDecoderFactory::OnThreadStop()
{
    mThreadStopped = true;
    mThreadEvent.Set();
}

// ===============================================================================================================
// Manage SD only tune flag.  
//
// The app needs to make sure that all pipes are detuned and HAl decoders purged before setting/resetting the flag
// and use of HAl decoders.
// ===============================================================================================================

bool CHalDecoderFactory::GetSimpleDefinitionOnlyTune(void)
{
    return mIsSimpleDefinitionOnlyTune;
}

void CHalDecoderFactory::SetSimpleDefinitionOnlyTune(void)
{
    if (!mIsSimpleDefinitionOnlyTune)
    {
        mIsSimpleDefinitionOnlyTune = true;
        mAVManager->GetDiagsManager()->PostEvent(new CDiagsSDOnlyTuneEvent(mIsSimpleDefinitionOnlyTune));
    }
}

void CHalDecoderFactory::ClearSimpleDefinitionOnlyTune(void)
{
    if (mIsSimpleDefinitionOnlyTune)
    {
        mIsSimpleDefinitionOnlyTune = false;
        mAVManager->GetDiagsManager()->PostEvent(new CDiagsSDOnlyTuneEvent(mIsSimpleDefinitionOnlyTune));
    }
}

// ===============================================================================================================
// Karaoke tunes
// ===============================================================================================================

bool CHalDecoderFactory::GetKaraokeTune(void)
{
    return mIsKaraokeTune;
}

void CHalDecoderFactory::SetKaraokeTune(void)
{
    if (!mIsKaraokeTune)
    {
        mIsKaraokeTune = true;
        UpdateKaraokeState(NULL);
        mAVManager->GetDiagsManager()->PostEvent(new CDiagsKOKTuneEvent(mIsKaraokeTune));
    }
}

void CHalDecoderFactory::ClearKaraokeTune(void)
{
    if (mIsKaraokeTune)
    {
        mIsKaraokeTune = false;
        UpdateKaraokeState(NULL);
        mAVManager->GetDiagsManager()->PostEvent(new CDiagsKOKTuneEvent(mIsKaraokeTune));
    }
}

int CHalDecoderFactory::UpdateKaraokeState(LPVOID context)
{
    return IHalDecoderFactory::SetKaraokeState(context, mIsKaraokeTune);
}

// ===============================================================================================================
// ===============================================================================================================

//Send the karaoke flag down to the HAL.  Expectation is that
//any HAL not supporting the functionality will just ignore and
//return a not implemented error which we can ignore.
/*static*/ int IHalDecoderFactory::SetKaraokeState(LPVOID context, bool isKaraokeOn)
{
    IPTV_HAL_DECODER_VALUE_AUDIO_KARAOKE karaokeVal;
    karaokeVal.bEnable = isKaraokeOn;
    iptv_hal_error err = IPTV_HAL_Decoder_SetValue(context, IPTV_HAL_DECODER_VALUETYPE_AUDIO_KARAOKE, &karaokeVal, sizeof(IPTV_HAL_DECODER_VALUE_AUDIO_KARAOKE));
    return err;
}

// ===============================================================================================================
// ===============================================================================================================

//Various global audio output APIs
/*static*/ int IHalDecoderFactory::SetAudioOutputFormat(IPTV_HAL_DECODER_AUDIOOUTPUTTYPE format)
{
    TRACE(("Audio output format requested - %d", format));

    IPTV_HAL_DECODER_VALUE_AUDIO_OUTPUTFORMAT outputformat;
    outputformat.OutputType = format;
    iptv_hal_error err = IPTV_HAL_Decoder_SetValue(NULL, IPTV_HAL_DECODER_VALUETYPE_AUDIO_OUTPUTFORMAT, &outputformat, sizeof(IPTV_HAL_DECODER_VALUE_AUDIO_OUTPUTFORMAT));
    return err;
}

/*static*/ int IHalDecoderFactory::SetDolbyTB11(bool state)
{
    TRACE(("Dolby TB11 requested - %d", state));

    IPTV_HAL_DECODER_VALUE_AUDIO_DOLBYTB11 tb11state;
    tb11state.bDisable = !state;
    iptv_hal_error err = IPTV_HAL_Decoder_SetValue(NULL, IPTV_HAL_DECODER_VALUETYPE_AUDIO_DOLBYTB11, &tb11state, sizeof(tb11state));
    return err;
}

/*static*/ int IHalDecoderFactory::SetAudioLineState(uint32 linestate)
{
    IPTV_HAL_DECODER_VALUE_AUDIO_LINESTATE audiolinestate;
    audiolinestate.u32UnMuteMask = linestate;
    iptv_hal_error err = IPTV_HAL_Decoder_SetValue(NULL, IPTV_HAL_DECODER_VALUETYPE_AUDIO_LINESTATE, &audiolinestate, sizeof(audiolinestate));
    return err;
}

// ===============================================================================================================
// ===============================================================================================================
