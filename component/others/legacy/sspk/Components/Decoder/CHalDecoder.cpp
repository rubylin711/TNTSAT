///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CHalDecoder.h"
#include "CStreamInfo.h"
#include "Trace.h"

#define DECODER_SPEW
#if defined(DECODER_SPEW)
#define DECODER_MSG(x) TRACE(x)
#else
#define DECODER_MSG(x)
#endif

// ===============================================================================================================
// HAL Decoder Factory maintains a cache of all decoders that have been allocated based on their pipe id and
// reuses them whereever possible.  The HAl decoders are asynchronously flushed when they are released back to
// the pool.  The allocation of HAl decoders will wait until the decoders are flushed and released back to the
// pool.
// ===============================================================================================================

CHalDecoder::CHalDecoder(uint32 pipeIdN, bool reusable)
    : Available(CEvent::eResetModeManual, true)
{
    mPipeIdN = pipeIdN;

    Codec = (IPTV_HAL_DECODER_CODECTYPE)-1;
    Resolution = (IPTV_HAL_DECODER_RESOLUTION)-1;
    DecoderContext = NULL;
    ClockContext = NULL;
    Header = NULL;
    HeaderLength = 0;

    IsAllocated = false;
    IsReleasing = false;
    IsFlushDecoder = true;
    IsTeardownPicture = false;
    IsReusable = reusable;
}

CHalDecoder::~CHalDecoder()
{
    if (DecoderContext)
    {
        DECODER_MSG(("[%04X] Releasing HAL Decoder for %s", mPipeIdN, CodecName(Codec)));

        CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE);
        IPTV_HAL_Decoder_ReleaseDecoder(DecoderContext);
        CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE_DONE);

        delete [] Header;
    }
}

bool CHalDecoder::CompareHeader(const LPVOID header, uint32 headerLength)
{
    //No existing header
    if (Header == NULL)
    {
        return header == NULL;
    }
    //There is an existing header
    if (header != NULL && headerLength == HeaderLength)
    {
        return memcmp(Header, header, headerLength) == 0;
    }
    return false;
}

LPVOID CHalDecoder::Create(IPTV_HAL_DECODER_ACQUIRE& acqParams, LPVOID pClockContext, const LPVOID header, uint32 headerLength, iptv_hal_error& errorCode)
{
    //Wait for decoder to become available
    uint32 waitedForTicks = 0;
    while (Available.Wait(1000) == CEvent::eWaitTimeout)
    {
        waitedForTicks += 1000;
        DECODER_MSG(("[%04X] Waiting for %d time while trying to acquire HAL Decoder for %s", mPipeIdN, waitedForTicks, CodecName(acqParams.eCodecType)));
    }
    {
        AutoLock lock(&Lock);

        //Look for an existing decoder - return if we found an exact match
        if (Codec == acqParams.eCodecType && ClockContext == pClockContext
            && ((acqParams.eCodecType > IPTV_HAL_DECODER_CODECTYPE_MAXVIDEO) || Resolution == acqParams.eResolution)
            && CompareHeader(header, headerLength))
        {
            DECODER_MSG(("[%04X] Reusing HAL Decoder for %s", mPipeIdN, CodecName(acqParams.eCodecType)));

            IsAllocated = true;
            Available.Reset();

            errorCode = IPTV_HAL_ERROR_SUCCESS;
            return DecoderContext;
        }

        //Release the last hardware decoder, if any
        if (DecoderContext)
        {
            DECODER_MSG(("[%04X] Releasing HAL Decoder for %s", mPipeIdN, CodecName(Codec)));

            CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE);
            IPTV_HAL_Decoder_ReleaseDecoder(DecoderContext);
            CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE_DONE);

            //Clear our state
            Codec = (IPTV_HAL_DECODER_CODECTYPE)-1;
            Resolution = (IPTV_HAL_DECODER_RESOLUTION)-1;
            DecoderContext = NULL;
            ClockContext = NULL;
            delete [] Header;
            Header = NULL;
            HeaderLength = 0;
        }

        //Create a new decoder of give type
        CE_AV_DECODER_LOG(CE_AV_DECODER_ACQUIRE);
        errorCode = IPTV_HAL_Decoder_Acquire(&acqParams, &DecoderContext);
        CE_AV_DECODER_LOG(CE_AV_DECODER_ACQUIRE_DONE);

        //Did we successfully acquire a HAL decoder...
        if (IPTV_HAL_ERROR_RET_SUCCESS(errorCode))
        {
            //We must have a decoder context on a successful return from decoder acquire API
            CHECK_ALLOC(DecoderContext);

            //Delete old header and save the new one
            ASSERT(Header == NULL && HeaderLength == 0);
            if (header)
            {
                Header = new byte[headerLength];
                memcpy_s(Header, headerLength, header, headerLength);
                HeaderLength = headerLength;

                //Now set the header
                if (acqParams.eCodecType > IPTV_HAL_DECODER_CODECTYPE_MAXVIDEO)
                {
                       ASSERT(HeaderLength == sizeof(IPTV_HAL_DECODER_VALUE_AUDIO_WMA9));

                    CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
                       errorCode = IPTV_HAL_Decoder_SetValue(DecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_WMA, Header, HeaderLength);
                    CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);
                }
                else
                {
                    ASSERT(HeaderLength == sizeof(IPTV_HAL_DECODER_VALUE_VIDEO_WMV9));

                    CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE);
                    errorCode = IPTV_HAL_Decoder_SetValue(DecoderContext, IPTV_HAL_DECODER_VALUETYPE_VIDEO_WMV9, Header, HeaderLength);
                    CE_AV_DECODER_LOG(CE_AV_DECODER_SETVALUE_DONE);
                }
            }

            if (IPTV_HAL_ERROR_RET_SUCCESS(errorCode))
            {
                //Initialize the decoder
                IPTV_HAL_DECODER_INIT InitParams;
                InitParams.pClockContext = pClockContext;

                CE_AV_DECODER_LOG(CE_AV_DECODER_INIT);
                errorCode = IPTV_HAL_Decoder_Init(DecoderContext, &InitParams);
                CE_AV_DECODER_LOG(CE_AV_DECODER_INIT_DONE);
            }

            //Did we successfully initialize the HAL decoder...
            //Note that an audio decoder initialization may fail if the clock
            //context that was provided was not appropriate for playing back
            //audio stream...
            if (IPTV_HAL_ERROR_RET_SUCCESS(errorCode))
            {
                //Successful acquisition/initialization of HAL decoder...
                DECODER_MSG(("[%04X] Creating HAL Decoder for %s", mPipeIdN, CodecName(acqParams.eCodecType)));

                //Update the state
                Codec = acqParams.eCodecType;
                Resolution = acqParams.eResolution;
                ClockContext = pClockContext;

                //And a HAL decoder is now available for use
                IsAllocated = true;
                Available.Reset();
            }
            else
            {
                //Initialization failed
                DECODER_MSG(("[%04X] Failed [%d] to initialize HAL Decoder for %s", mPipeIdN, errorCode, CodecName(acqParams.eCodecType)));

                //Release the decoder back to HAL
                CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE);
                IPTV_HAL_Decoder_ReleaseDecoder(DecoderContext);
                CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE_DONE);

                //Clear our state
                Codec = (IPTV_HAL_DECODER_CODECTYPE)-1;
                Resolution = (IPTV_HAL_DECODER_RESOLUTION)-1;
                DecoderContext = NULL;
                ClockContext = NULL;
                delete [] Header;
                Header = NULL;
                HeaderLength = 0;
            }
        }
        else
        {
            //Acquisition failed
            DECODER_MSG(("[%04X] Failed [%d] to Allocate HAL Decoder for %s", mPipeIdN, errorCode, CodecName(acqParams.eCodecType)));

            //Clear our state
            Codec = (IPTV_HAL_DECODER_CODECTYPE)-1;
            Resolution = (IPTV_HAL_DECODER_RESOLUTION)-1;
            DecoderContext = NULL;
            ClockContext = NULL;
            delete [] Header;
            Header = NULL;
            HeaderLength = 0;
        }
        return DecoderContext;
    }
}

void CHalDecoder::Release(void)
{
    AutoLock lock(&Lock);
    if (IsReleasing)
    {
        ASSERT(DecoderContext);
        if (IsReusable)
        {
            //We cache the fullscreen decoders
            //
            //So just flush the decoder to bring it back to clean state
            if (IsFlushDecoder)
            {
                DECODER_MSG(("[%04X] Flushing HAL Decoder for %s (IsTeardownPicture=%d)", mPipeIdN, CodecName(Codec), IsTeardownPicture));
                CE_AV_DECODER_LOG(CE_AV_DECODER_FLUSH);
                IPTV_HAL_Decoder_Flush(DecoderContext, IsTeardownPicture, false);//delete because when stop mss vdec reset will hangup
                CE_AV_DECODER_LOG(CE_AV_DECODER_FLUSH_DONE);
            }
            else
            {
                DECODER_MSG(("[%04X] Skip Flushing HAL Decoder for %s", mPipeIdN, CodecName(Codec)));
            }
        }
        else
        {
            //We always release the PIP decoders
            DECODER_MSG(("[%04X] Releasing HAL Decoder for %s", mPipeIdN, CodecName(Codec)));

            CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE);
            IPTV_HAL_Decoder_ReleaseDecoder(DecoderContext);
            CE_AV_DECODER_LOG(CE_AV_DECODER_RELEASE_DONE);

            //Clear our state
            Codec = (IPTV_HAL_DECODER_CODECTYPE)-1;
            Resolution = (IPTV_HAL_DECODER_RESOLUTION)-1;
            DecoderContext = NULL;
            ClockContext = NULL;
            delete [] Header;
            Header = NULL;
            HeaderLength = 0;
        }
        IsReleasing = false;
        IsAllocated = false;
        Available.Set();
    }
}

void CHalDecoder::TeardownPicture(void)
{
    AutoLock lock(&Lock);
    if (DecoderContext)
    {
        DECODER_MSG(("[%04X] Teardown picture for HAL Decoder for %s", mPipeIdN, CodecName(Codec)));

        CE_AV_DECODER_LOG(CE_AV_DECODER_FLUSH);
        IPTV_HAL_Decoder_Flush(DecoderContext, true, false);
        CE_AV_DECODER_LOG(CE_AV_DECODER_FLUSH_DONE);
    }
}

bool CHalDecoder::IsItAllocated(void)
{
    AutoLock lock(&Lock);
    return IsAllocated;
}

// ===============================================================================================================
// ===============================================================================================================

int CHalDecoder::Stream2Codec(int streamType)
{
    //Returns HAL codec type given M2TS stream type
    switch (streamType)
    {
    case StreamType_Video:                 return IPTV_HAL_DECODER_CODECTYPE_VIDEO_MPEG2;
    case StreamType_Video_Constrained:     return IPTV_HAL_DECODER_CODECTYPE_VIDEO_MPEG2;
    case StreamType_Video_AVC:             return IPTV_HAL_DECODER_CODECTYPE_VIDEO_H264;
    case StreamType_Video_VC1:             return IPTV_HAL_DECODER_CODECTYPE_VIDEO_VC1;
    case StreamType_Video_WMV9:            return IPTV_HAL_DECODER_CODECTYPE_VIDEO_WMV9;
    case StreamType_Audio_11172:
    case StreamType_Audio_13818_3:         return IPTV_HAL_DECODER_CODECTYPE_AUDIO_MPEG;
    case StreamType_Audio_AC3:             return IPTV_HAL_DECODER_CODECTYPE_AUDIO_AC3;
    case StreamType_Audio_EnhancedAC3:     return IPTV_HAL_DECODER_CODECTYPE_AUDIO_EAC3;
    case StreamType_Audio_AAC:
    case StreamType_Audio_HEAAC:           return IPTV_HAL_DECODER_CODECTYPE_AUDIO_AAC;
    case StreamType_Audio_WMA:             return IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMASTD;
    case StreamType_Audio_WMAPRO:          return IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMAPRO;
    case StreamType_Audio_WMA_WITH_HEADER: return IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMATS;
    }
    return INVALID_CODEC;
}

const char* CHalDecoder::StreamName(int streamType)
{
    //Returns name given M2TS stream type
    switch (streamType)
    {
    case StreamType_Video:                 return "mpeg1";
    case StreamType_Video_Constrained:     return "mpeg2";
    case StreamType_Video_AVC:             return "h264";;
    case StreamType_Video_VC1:             return "vc1";
    case StreamType_Video_WMV9:            return "wmv9";
    case StreamType_Audio_11172:           return "mpeg1";
    case StreamType_Audio_13818_3:         return "mpeg2";
    case StreamType_Audio_AC3:             return "ac3";
    case StreamType_Audio_EnhancedAC3:     return "eac3";
    case StreamType_Audio_AAC:             return "aac";
    case StreamType_Audio_HEAAC:           return "heaac";
    case StreamType_Audio_WMA:             return "wma";
    case StreamType_Audio_WMAPRO:          return "wmapro";
    case StreamType_Audio_WMA_WITH_HEADER: return "wma-ex";
    }
    return "unknown";
}

const char* CHalDecoder::CodecName(int codec)
{
    //Reurns the HAL decoder name
    switch (codec)
    {
    case IPTV_HAL_DECODER_CODECTYPE_VIDEO_MPEG2:  return "mpeg";
    case IPTV_HAL_DECODER_CODECTYPE_VIDEO_H264:   return "h264";
    case IPTV_HAL_DECODER_CODECTYPE_VIDEO_VC1:    return "vc1";
    case IPTV_HAL_DECODER_CODECTYPE_VIDEO_WMV9:   return "wmv9";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AC3:    return "ac3";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_EAC3:   return "eac3";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AAC:    return "aac";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_PCM:    return "pcm";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_MPEG:   return "mpeg";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMASTD: return "wma";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMAPRO: return "wmapro";
    case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMATS:  return "wma-ex";
    }
    return "unknown";
}

// ===============================================================================================================
// ===============================================================================================================
