/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_BLURAYLPCM_H__
#define __MTSI_AUDIO_DECODER_BLURAYLPCM_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HA_BLYRAYLPCM_ID 0x021        

typedef struct  hiHA_BLURAYLPCM_DECODE_OPENCONFIG_S
{
    MT_BOOL                 bAutoDmxStereo; /* automatic downmix multi-ch to stereo. */
    MT_BOOL                 bConvert2fmt16bit; /* automatic convert none-16bit pcm to 16bit pcm. */
} HA_BLURAYLPCM_DECODE_OPENCONFIG_S;

#define HA_BLYRAYLPCM_DecGetDefalutOpenConfig(pConfigParam) \
    do { \
        ((HA_BLURAYLPCM_DECODE_OPENCONFIG_S *)(pConfigParam))->bAutoDmxStereo = MT_TRUE; \
        ((HA_BLURAYLPCM_DECODE_OPENCONFIG_S *)(pConfigParam))->bConvert2fmt16bit = MT_TRUE; \
    } while (0)


#define HA_BLYRAYLPCM_DecGetDefalutOpenParam(pOpenParam, pstPrivateParams) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->u32CodecPrivateDataSize = sizeof(HA_BLURAYLPCM_DECODE_OPENCONFIG_S); \
}while(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_BLURAYLPCM_H__ */

