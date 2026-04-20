///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IPTVDecoderHal.h"
#include "IHalClock.h"
#include "IDecoder.h"

// ===============================================================================================================
// ===============================================================================================================

class IHalDecoderFactory
{
public:
    //Destructor
    virtual ~IHalDecoderFactory() {}

    //Acquire instance of IHalClock
    virtual IHalClock*      AcquireHalClock(bool bAVSync) = 0;

    //Acquire HAL video decoder of given type
    virtual LPVOID          AcquireVideoDecoder(uint32 pipeIdN, IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext, const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode) = 0;
    //Release HAL video decoder back to factory
    virtual void            ReleaseVideoDecoder(uint32 pipeIdN, LPVOID pDecoderContext, bool flushDecoder, bool teardownPicture) = 0;

    //Acquire HAL audio decoder of given type
    virtual LPVOID          AcquireAudioDecoder(uint32 pipeIdN, IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext, const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode) = 0;
    //Release HAL audio decoder back to factory
    virtual void            ReleaseAudioDecoder(uint32 pipeIdN, LPVOID pDecoderContext, bool flushDecoder) = 0;

    //Purge decoders associated with given pipe
    virtual bool            PurgeDecoders(uint32 pipeIdN) = 0;

    //Teardown picture for give pipe
    virtual void            TeardownPicture(uint32 pipeIdN) = 0;

    //Returns current simple definition only tune flag
    virtual bool            GetSimpleDefinitionOnlyTune(void) = 0;
    //Turn simple definition only tune on
    virtual void            SetSimpleDefinitionOnlyTune(void) = 0;
    //Turn simple definition only tune off
    virtual void            ClearSimpleDefinitionOnlyTune(void) = 0;

    //Returns current karaoke state
    virtual bool            GetKaraokeTune(void) = 0;
    //Turns the karaoke on
    virtual void            SetKaraokeTune(void) = 0;
    //Turns the karaoke off
    virtual void            ClearKaraokeTune(void) = 0;
    //Updates HAL karaoke state
    virtual int             UpdateKaraokeState(LPVOID context) = 0;

    //Updates HAL karaoke state.  This could be called when no HAl decoders have been instantiated
    static int              SetKaraokeState(LPVOID context, bool isKaraokeOn);
    //Various global audio output APIs
    static int              SetAudioOutputFormat(IPTV_HAL_DECODER_AUDIOOUTPUTTYPE format);
    static int              SetDolbyTB11(bool state);
    static int              SetAudioLineState(uint32 linestate);
};

// ===============================================================================================================
// ===============================================================================================================
