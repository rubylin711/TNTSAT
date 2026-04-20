/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_CODEC_G722_H__
#define __MTSI_AUDIO_CODEC_G722_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define HA_G722_ID 0x0105        

#define G722_FRAME_BLOCK_LEN_10MS  160
#define G722_FRAME_BLOCK_LEN_20MS  320

#define HA_G722_GetEncDefaultOpenParam(pOpenParam) \
do{ ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredOutChannels = 1; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->bInterleaved = MT_TRUE; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->s32BitPerSample = 16; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredSampleRate = 16000; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32SamplePerFrame = G722_FRAME_BLOCK_LEN_10MS; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = MT_NULL; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = 0; \
}while(0)

typedef struct
{
    mt_s32  mode;  /* 1=64kbps, 2= 56kbps, 3= 48kbps  */
} G722_DECODE_OPENCONFIG_S;

#define HA_G722_GetDecDefalutOpenParam(pOpenParam, pstPrivateParams) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 1; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 16000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(G722_DECODE_OPENCONFIG_S); \
}while(0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */


#endif /* __MTSI_AUDIO_CODEC_G722_H__ */
