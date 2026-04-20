/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_ENCODER_AAC_H__
#define __MTSI_AUDIO_ENCODER_AAC_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define    HA_AAC_ID 0x001         

typedef enum
{
    AAC_QualityExcellent = 0,
    AAC_QualityHigh   = 1,
    AAC_QualityMedium = 2,
    AAC_QualityLow = 3,
} AAC_AuQuality;

typedef enum
{
    AAC_FORMAT_LC = 0,                  /**< AAC LC */
    AAC_FORMAT_EAAC = 1,                    /**< eAAC  (HEAAC or AAC+  or aacPlusV1) */
    AAC_FORMAT_EAACPLUS = 2,            /**< eAAC+ (AAC++ or aacPlusV2) */
} AAC_AuEncoderFormat;

typedef  struct
{
    AAC_AuQuality       quality;
    AAC_AuEncoderFormat coderFormat;
    mt_s16          bitsPerSample;
    mt_s32          sampleRate;    /**< audio file sample rate */
    mt_s32          bitRate;            /**< encoder bit rate in bits/sec */
    mt_s16          nChannelsIn;    /**< number of channels on input (1,2) */
    mt_s16          nChannelsOut;  /**< number of channels on output (1,2) */
    mt_s16          bandWidth;       /**< targeted audio bandwidth in Hz */
} AAC_ENC_CONFIG;

#define HA_AAC_GetDefaultConfig(pstConfig) \
do{ ((AAC_ENC_CONFIG *)(pstConfig))->coderFormat = AAC_FORMAT_LC; \
    ((AAC_ENC_CONFIG *)(pstConfig))->bitsPerSample = 16; \
    ((AAC_ENC_CONFIG *)(pstConfig))->quality = AAC_QualityHigh; \
    ((AAC_ENC_CONFIG *)(pstConfig))->bitRate = 128000; \
    ((AAC_ENC_CONFIG *)(pstConfig))->sampleRate = 48000; \
    ((AAC_ENC_CONFIG *)(pstConfig))->bandWidth    = ((AAC_ENC_CONFIG *)(pstConfig))->sampleRate / 2; \
    ((AAC_ENC_CONFIG *)(pstConfig))->nChannelsIn  = 2; \
    ((AAC_ENC_CONFIG *)(pstConfig))->nChannelsOut = 2; \
}while(0)

#define HA_AAC_GetEncDefaultOpenParam(pOpenParam, pstPrvateConfig) \
do{ mt_s32 inSamplePerFrame; \
    inSamplePerFrame = 1024; \
    if ((((AAC_ENC_CONFIG *)pstPrvateConfig)->coderFormat == AAC_FORMAT_EAAC) | (((AAC_ENC_CONFIG *)pstPrvateConfig)->coderFormat == AAC_FORMAT_EAACPLUS)) \
    { \
        inSamplePerFrame <<= 1; \
    } \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredOutChannels = ((AAC_ENC_CONFIG *)pstPrvateConfig)->nChannelsIn; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->bInterleaved = MT_TRUE; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->s32BitPerSample = ((AAC_ENC_CONFIG *)pstPrvateConfig)->bitsPerSample; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredSampleRate = ((AAC_ENC_CONFIG *)pstPrvateConfig)->sampleRate; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32SamplePerFrame = inSamplePerFrame; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrvateConfig; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(AAC_ENC_CONFIG); \
}while(0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_ENCODER_AAC_H__ */
