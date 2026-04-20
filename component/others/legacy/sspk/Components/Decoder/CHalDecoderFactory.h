///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAVManager.h"
#include "IHalDecoder.h"
#include "AutoLock.h"
#include "Thread.h"
#include "CEvent.h"
#include <map>

// ===============================================================================================================
// ===============================================================================================================

class CHalDecoder;

class CHalDecoderFactory : public IHalDecoderFactory, public IRunnable
{
public:
    //Constructor
    CHalDecoderFactory(IAVManager* avManager);
    //Cleanup HAL decoder factory
    virtual ~CHalDecoderFactory();

    //Retrieve an instance of IHalClock object
    __override IHalClock*           AcquireHalClock(bool bAVSync);

    //Acquire HAL video decoder of given type
    __override LPVOID               AcquireVideoDecoder(uint32 pipeIdN, IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext, const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode);
    //Release HAL video decoder back to factory
    __override void                 ReleaseVideoDecoder(uint32 pipeIdN, LPVOID pDecoderContext, bool flushDecoder, bool teardownPicture);

    //Acquire HAL audio decoder of given type
    __override LPVOID               AcquireAudioDecoder(uint32 pipeIdN, IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext, const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode);
    //Release audio HAL decoder back to factory
    __override void                 ReleaseAudioDecoder(uint32 pipeIdN, LPVOID pDecoderContext, bool flushDecoder);

    //Purge decoders associated with given pipe
    __override bool                 PurgeDecoders(uint32 pipeIdN);

    //Teardown picture for give pipe
    __override void                 TeardownPicture(uint32 pipeIdN);

    //Returns current simple definition only tune flag
    __override bool                 GetSimpleDefinitionOnlyTune(void);
    //Turn simple definition only tune on
    __override void                 SetSimpleDefinitionOnlyTune(void);
    //Turn simple definition only tune off
    __override void                 ClearSimpleDefinitionOnlyTune(void);

    //Returns current karaoke state
    __override bool                 GetKaraokeTune(void);
    //Turns the karaoke on
    __override void                 SetKaraokeTune(void);
    //Turns the karaoke off
    __override void                 ClearKaraokeTune(void);
    //Updates HAL karaoke state
    __override int                  UpdateKaraokeState(LPVOID context);

private:
    __override void                 OnThreadRun();
    __override void                 OnThreadStop();

    bool                            DecodersReleased(uint32 pipeIdN);

private:
    //Factory of factories
    IAVManager*                     mAVManager;

    //Protecting access to the factory
    Lockable                        mHalDecoderLock;

    //HAL decoders that are allocated
    std::map<uint32,CHalDecoder*>   mHalVideoDecoder;
    CHalDecoder*                    mHalAudioDecoder;
    CHalDecoder*                    mHalAudioDescriptionDecoder;

    //Simple definition only tune flag
    bool                            mIsSimpleDefinitionOnlyTune;
    //Karaoke tune
    bool                            mIsKaraokeTune;

    //Thread to handle asynchronous flush/release of HAL decoders
    bool                            mThreadStopped;
    CEvent                          mThreadEvent;
    Thread                          mThread;
};

// ===============================================================================================================
// ===============================================================================================================
