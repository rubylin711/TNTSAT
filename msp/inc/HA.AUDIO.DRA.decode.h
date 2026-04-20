/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_DRA_H__
#define __MTSI_AUDIO_DECODER_DRA_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define    HA_DRA_ID 0x007        

#define HA_CODEC_GET_DRA_DonwmixMultich_MODE_CMD  ((((mt_u32)HA_DRA_ID) << 16) | 0x1000)
typedef struct
{
    mt_u32  enCmd;              /* HA_DRA_GET_DonwmixMultich_MODE_S */
    MT_BOOL  *pbDonwmixMultichMode;
} HA_DRA_GET_DonwmixMultich_MODE_S;
#define HA_DRA_DecGetDefalutOpenParam(pOpenParam) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_SIMUL; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = MT_NULL; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = 0; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 24; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
}while(0)

#define HA_DRA_DecGetOpenParam_MultichPcm(pOpenParam) \
    do{ mt_s32 i; \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_SIMUL; \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = MT_NULL; \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = 0; \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 8; \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 24; \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
        for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
        { \
            ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
        } \
    }while(0)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_DRA_H__ */

