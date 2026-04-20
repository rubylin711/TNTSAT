/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_CODEC_G711_H__
#define __MTSI_AUDIO_CODEC_G711_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


#define HA_G711_ID 0x0102        

#define G711_FRAME_BLOCK_LEN_10MS  80


typedef struct
{
    mt_s32  isAlaw;  /* 1=A Law, 0= u Law  */
    MT_BOOL bVAD;  /* MT_TRUE=enable vad, MT_FALSE=disable vad  */
} G711_ENCODE_OPENCONFIG_S;

#define HA_G711_GetEncDefaultOpenParam(pOpenParam, pstPrivateParams) \
do{ ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredOutChannels = 1; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->bInterleaved = MT_TRUE; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->s32BitPerSample = 16; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredSampleRate = 8000; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32SamplePerFrame = G711_FRAME_BLOCK_LEN_10MS; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(G711_ENCODE_OPENCONFIG_S); \
}while(0)

typedef struct
{
    mt_s32  isAlaw;  /* 1=A Law, 0= u Law  */
} G711_DECODE_OPENCONFIG_S;

#define HA_G711_GetDecDefalutOpenParam(pOpenParam, pstPrivateParams) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 1; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 8000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(G711_DECODE_OPENCONFIG_S); \
}while(0)
    
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_CODEC_G711_H__ */
