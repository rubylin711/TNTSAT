///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Decoder_DRM.h"
#include "CHalDecoder.h"
#include "IDrmManager.h"
#include "IPTVDecoderHal.h"
#include "SSPKTimeSpan.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>

#include <sys/time.h>

using namespace std;

//#define DECODER_SPEW
#if defined(DECODER_SPEW)
#define DECODER_MSG(x) TRACE(x)
#else
#define DECODER_MSG(x)
#endif

// #define DECODER_SPEW_VERBOSE
#ifdef DECODER_SPEW_VERBOSE
#define DECODER_TRACE_VERBOSE(x) TRACE(x)
#else
#define DECODER_TRACE_VERBOSE(x)
#endif

// ===============================================================================================================
// Turn on this to dump elementary stream to hard disk
// ===============================================================================================================

//#define DUMP_ES_TO_FILE
#define DUMP_ES_BEFORE_DECRYPT (0)

#ifdef DUMP_ES_TO_FILE

#include <stdio.h>

typedef map<LPVOID, FILE*> FILEMAP;
static FILEMAP _esWritersMap;
static LONG lCount = 0;
static bool bEsDumpInitialized = false;

void ESDump_Initialize(void)
{
    if (bEsDumpInitialized)
        return;

    bEsDumpInitialized = true;

    TRACE(("ESDUMP - CREATE ES CAPTURE FOLDER"));
    TRACE(("ESDUMP - /mnt/captures"));

    //CreateDirectory(L"captures", NULL);

    TRACE(("ESDUMP - CAPTURE READY TO GO"));
}

void ESDump_OpenFile(LPVOID handle, const char* codecName)
{
    char szFilename[64];
    FILE* pFile;

    InterlockedIncrement(&lCount);

    //_snprintf_s(szFilename, sizeof(szFilename), "captures\\%s_%d.pts", codecName, lCount);
    snprintf(szFilename, sizeof(szFilename), "/mnt/captures/%s_%d.es", codecName, lCount);
    fopen_s(&pFile, (const char *)&szFilename, "wb");
    //fopen_s(&pFile, (const char *)&szFilename, "a+");
    DECODER_MSG(("ESDUMP: Dump filename: %s", szFilename));

    ASSERT(pFile != NULL);
    _esWritersMap[handle] = pFile;
}

void ESDump_CloseFile(LPVOID handle)
{
    FILE* pFile = _esWritersMap[handle];
    if (pFile != NULL)
    {
        fclose(pFile);
    }
    _esWritersMap.erase(handle);
}

//Write the pts
void ESDump_WritePts(LPVOID handle, uint64 pts)
{
    FILE* pFile = _esWritersMap[handle];
    fwrite(&pts, 8, 1, pFile);

    DECODER_MSG((" ESDUMP: pts = %lld", pts));
}

void ESDump_WriteData(LPVOID handle, iptv_hal_error result, Buffer* buffer)
{
    FILE* pFile = _esWritersMap[handle];
    if (result == IPTV_HAL_ERROR_SUCCESS)
    {
        Buffer* pB = (Buffer*)buffer;
        bool bWritePTS = false;
        uint64 nullpts = INVALID_TIME;
#if 0
        while (pB)
        {
            if (pB->Size)
            {
                if (bWritePTS)
                {
                    fwrite(&nullpts, 1, sizeof(nullpts), pFile);
                }

                DECODER_MSG((" ESDUMP: i32Len = %d", pB->Size));
                fwrite(&pB->Size, 1, sizeof(pB->Size), pFile);
                fwrite(pB->Data, 1, pB->Size, pFile);
                fflush(pFile);
                bWritePTS = true;
            }
            pB = pB->Next;
        }
#endif
        while (pB)
        {
            uint32 numbytes = pB->HALBuffer.u32DataEnd - pB->HALBuffer.u32DataStart;
            if (pB->HALBuffer.u32Size && numbytes > 0)
            {
                DECODER_MSG(("ESDump_WriteData() - ESDUMP: data i32Len = %d", numbytes));
                fwrite(pB->HALBuffer.pBuf + pB->HALBuffer.u32DataStart, 1, numbytes, pFile);
                fflush(pFile);
            }
            pB = pB->Next;
        }
    }
    else
    {
        DECODER_MSG((" ESDUMP: decryption failed!!!!!!!!! rewind back 8 bytes"));
        fseek(pFile, -8, SEEK_CUR);
    }
}

#endif

// ===============================================================================================================
// ===============================================================================================================

DecoderDRM::DecoderDRM(IReceiverControl* receiverControl, const CStreamInfo& si)
    : Decoder(receiverControl, si)
    , mIsVideo(mSI.IsVideo())
    , mIsAudioDescription(mSI.IsAudioDescription())
    , mCodec(CHalDecoder::Stream2Codec(mSI.Format))
    , mDecoderContext(NULL)
    , mDecoderFlushed(true)
    , mDRMContext(NULL)
    , mLastDrmError(0)
    , mDecoderDiagnostics(mIsVideo ? *(mDiagnostics.pVideo) : mIsAudioDescription ? *(mDiagnostics.pAudioDescription) : *(mDiagnostics.pAudio))
{
#ifdef DUMP_ES_TO_FILE
    ESDump_Initialize();
#endif
}

DecoderDRM::~DecoderDRM()
{
    if (mDRMContext)
    {
        mReceiverControl->GetAVManager()->GetDrmManager()->ReleaseDecrypter(mDRMContext);
    }

    ASSERT(mDecoderContext == NULL);
}

bool DecoderDRM::Acquire(void)
{
#ifdef DUMP_ES_TO_FILE
    ESDump_OpenFile(this, CHalDecoder::StreamName(mStreamType));
#endif

    //Do not forget to call the base class
    return Decoder::Acquire();
}

void DecoderDRM::Release(void)
{
#ifdef DUMP_ES_TO_FILE
    ESDump_CloseFile(this);
#endif

    //The decoder has been flushed
    mDecoderFlushed = true;

    //Reset DRM parameters
    if (mDRMContext)
    {
        mDRMContext->Reset();
    }

    //Do not forget to call the base class
    Decoder::Release();
}

void DecoderDRM::OnSync(bool sync, bool teardownPicture, bool cleanStall)
{
    //Flush HAL decoder if needed
    if (mDecoderContext)
    {
        if (mDecoderFlushed == false)
        {
            DECODER_MSG(("DecoderDRM::OnSync(), teardownPicture:%s, cleanStall:%s", teardownPicture?"yes":"no", cleanStall?"yes":"no"));

            CE_AV_DECODER_LOG(CE_AV_DECODER_FLUSH);
            IPTV_HAL_Decoder_Flush(mDecoderContext, teardownPicture, cleanStall);
            CE_AV_DECODER_LOG(CE_AV_DECODER_FLUSH_DONE);

            mDecoderFlushed = true;
        }

        //Track HAL state for active decoders only
        mDecoderDiagnostics.OnFlushed();
    }

    //Initialize DRM related parameters
    if (mDRMContext)
    {
        mDRMContext->Reset();
    }

    //Do not forget to call the base class
    Decoder::OnSync(sync, teardownPicture, cleanStall);
}

Buffer* DecoderDRM::WriteHALDecoder(Buffer* chain, uint64 pts)
{
    //Create DRMContext if one doesn't currently exist
    if (!mDRMContext)
    {
        //not to create for clear streaming
        //mDRMContext = mReceiverControl->GetAVManager()->GetDrmManager()->GetDecrypter(DrmDecryptionMode_IPTV);
        DECODER_TRACE_VERBOSE(("DecoderDRM::WriteHALDecoder(), mDRMContext=0x%x", mDRMContext));
    }

    //Set the payload end markers
    Buffer* b = chain;
    uint32 numbytes = 0;
    while (b)
    {
        b->HALBuffer.u32DataEnd = b->Mark;
        numbytes += b->Mark;
        b = b->Next;
    }

    //Prepare decoder buffer, this will be replaced by the new HAL_Decrypt api
    bool ptsValid = false;
    if (IS_VALID_TIME(pts))
    {
        //It is a valid pts
        ptsValid = true;
    }
    iptv_hal_error err = IPTV_HAL_Decoder_PrepareForDecode(mDecoderContext, &chain->HALBuffer, ptsValid, pts);
    if (IPTV_HAL_ERROR_RET_SUCCESS(err))
    {
#ifdef DUMP_ES_TO_FILE
        //no pts writen to file
        //ESDump_WritePts(this, pts);
#if DUMP_ES_BEFORE_DECRYPT
        //write before Decrypt
        ESDump_WriteData(this, IPTV_HAL_ERROR_SUCCESS, chain);
#endif
#endif

        //This should be replaced by the new HAL_Decrypt api
        uint32 cryptoError = IPTV_HAL_Decoder_Decrypt(mDecoderContext, DecoderDRM::DecryptCB, this);
        if (cryptoError != 0)
        {
            //Track HAL state
            mDecoderDiagnostics.OnDRMError(numbytes, cryptoError);

            //Send rights expired event to app
            if (cryptoError == IPTV_HAL_ERROR_CRYPTO_BOUNDARY_KEY_EXPIRED)
            {
                DrmErrorEvent(cryptoError);
            }
        }
        else
        {
#ifdef DUMP_ES_TO_FILE
#if !(DUMP_ES_BEFORE_DECRYPT)
            //write after Decrypt
            ESDump_WriteData(this, IPTV_HAL_ERROR_SUCCESS, chain);
#endif
#endif
            //If data is succesfully tranferred into the bitstream FIFO after decrypt/passthru
            //then signal the decoder to do the actual decoding.
            err = IPTV_HAL_Decoder_Decode(mDecoderContext);
            if (IPTV_HAL_ERROR_RET_SUCCESS(err))
            {
                //Handle cgms/a, macrovision, finger printing etc...
                SetPassThroughAfterDecryption();
                //Track decoder state
                mDecoderDiagnostics.OnProcessing(numbytes, pts);
                //send hartbeat
                //HeartBeatEvent(0);
            }
            else
            {
                //Track decoder state
                mDecoderDiagnostics.OnDecoderError(numbytes, err);
            }

            //Send an event if rights recovered on decrypt success
            DrmErrorEvent(cryptoError);
        }

        //We have consumed the buffer chain at this point - release them back to the pool
        chain = Buffer::ReleaseChain(chain);
        //Can flush again
        mDecoderFlushed = false;
    }
    else
    if (err != IPTV_HAL_ERROR_DECODER_ESFIFO_FULL)
    {
        //Track decoder state
        mDecoderDiagnostics.OnDecoderError(numbytes, err);
        //We will dump all buffers if we get any error other than FIFO being full
        chain = Buffer::ReleaseChain(chain);
    }
    return chain;
}

void DecoderDRM::DrmErrorEvent(uint32 cryptoError)
{
    if (mLastDrmError != cryptoError)
    {
        //Save last DRM decrypt result
        mLastDrmError = cryptoError;
        //Send an event to Media transport
        string eventString = "status=drmstate&stream=";
        eventString += mIsVideo ? "video" : "audio";
        mReceiverControl->NotifyStatus(eventString += "&pkresult=" + toString(mLastDrmError, true));
    }
}

#define MIN_HEART_GET_PTS_INTERVAL 100  //ms

void DecoderDRM::HeartBeatEvent(uint64 curPts)
{
    static TimeSpan_NTP lastNtpTime;
    static int g_last_heart_time = 0;
    IPTV_HAL_DECODER_VALUE_AUDIO_CURRENTPTS currpts;
    uint32 size = sizeof(currpts);
    struct timeval tvNow;

    //to avoid get time frequently
    gettimeofday(&tvNow, NULL);
    int cur_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
    if((cur_time - g_last_heart_time) > MIN_HEART_GET_PTS_INTERVAL)
    {
        g_last_heart_time = cur_time;
        //printf("HeartBeatEvent IsImmediateMode:%d\n",mRendererState.IsImmediateMode);
        if (mRendererState.IsImmediateMode){
            //printf("HeartBeatEvent curPts:%lld\n",curPts);
            TimeSpan_NTP ntpTime = TimeSpan_NTP::ConvertFrom(TimeSpan_PTS::FromTicks(curPts));
            if (lastNtpTime != ntpTime)
            {
                string eventString = "status=heartbeat&hearttime="+ toString64(ntpTime.Ticks(), true);
                mReceiverControl->NotifyStatus(eventString);
                lastNtpTime = ntpTime;
            }
        }else{
            if (IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_GetValue(mDecoderContext, IPTV_HAL_DECODER_VALUETYPE_AUDIO_CURRENTPTS, &currpts, &size)))
            {
                //printf("HeartBeatEvent CurrentPTS:%lld\n",currpts.CurrentPTS);
                TimeSpan_NTP ntpTime = TimeSpan_NTP::ConvertFrom(TimeSpan_PTS::FromTicks(currpts.CurrentPTS));
                if (lastNtpTime != ntpTime)
                {
                    string eventString = "status=heartbeat&hearttime="+ toString64(ntpTime.Ticks(), true);
                    mReceiverControl->NotifyStatus(eventString);
                    lastNtpTime = ntpTime;
                }
            }
        }
    }
}

void DecoderDRM::SetCryptoKeyId(__in_bcount(len) const byte* signedKeyId, uint32 len)
{
    if (mDRMContext && !mRendererState.IsPlayingAd)
    {
        mDRMContext->SetKeyID(signedKeyId, len);

        if (NULL != mDecoderContext )
        {
            IPTV_HAL_Decoder_SetValue(
                    mDecoderContext,
                    IPTV_HAL_DECODER_VALUETYPE_DRM_SETKEYID,
                    (void*)signedKeyId,
                    len);
        }
    }
}

void DecoderDRM::SetCryptoSampleId(uint64 sampleId)
{
    if (mDRMContext && !mRendererState.IsPlayingAd)
    {
        mDRMContext->SetSampleID(sampleId);

        if (NULL != mDecoderContext && INVALID_SAMPLEID != sampleId)
        {
            IPTV_HAL_Decoder_SetValue(
                mDecoderContext,
                IPTV_HAL_DECODER_VALUETYPE_DRM_SETSAMPLEID,
                &sampleId,
                sizeof(sampleId));
        }
    }
}

void DecoderDRM::SetDRMHandle(uint32 drmHandle)
{
    //Check for pre-existing IDrmDecrypter
    if (mDRMContext)
    {
        //If handle is the same, just return
        if (mDRMContext->GetLicenseHandle() == drmHandle)
            return;

        //Delete current context and create new context
        mReceiverControl->GetAVManager()->GetDrmManager()->ReleaseDecrypter(mDRMContext);
    }

    mDRMContext = mReceiverControl->GetAVManager()->GetDrmManager()->GetDecrypter(DrmDecryptionMode_SSProtectionHeader, drmHandle);
    DECODER_TRACE_VERBOSE(("DecoderDRM::SetDRMHandle(), mDRMContext=0x%x", mDRMContext));
}

iptv_hal_error DecoderDRM::DecryptCB(PIPTV_HAL_BUFFER pInBufList, PIPTV_HAL_BUFFER pOutBufList, PVOID pDecryptContext)
{
    DecoderDRM* pDecoderDRM = (DecoderDRM*)pDecryptContext;
    iptv_hal_error result = IPTV_HAL_ERROR_CRYPTO_SUCCESS;

    DECODER_TRACE_VERBOSE(("DecoderDRM::DecryptCB(), pInBufList:0x%x, pOutBufList:0x%x", pInBufList, pOutBufList));
    if (pDecoderDRM && pDecoderDRM->mDRMContext)
    {
        DECODER_TRACE_VERBOSE(("DecoderDRM::DecryptCB(), pDecoderDRM=0x%x, mDRMContext=0x%x", pDecoderDRM, pDecoderDRM->mDRMContext));
        result = pDecoderDRM->mDRMContext->DecryptBufferChain(pInBufList, pOutBufList);
    }

#ifdef DUMP_ES_TO_FILE
#if !(DUMP_ES_BEFORE_DECRYPT)
    //ESDump_WriteData(pDecoderDRM, result, (Buffer*)pOutBufList);
#endif
#endif
    DECODER_TRACE_VERBOSE(("DecoderDRM::DecryptCB(), return, mDRMContext=0x%x, result=%d", pDecoderDRM->mDRMContext, result));

    return result;
}

// ===============================================================================================================
// ===============================================================================================================
