/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_MP3_H__
#define __MTSI_AUDIO_DECODER_MP3_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HA_MP3_ID 0x0003        
#define HA_CODEC_GET_MP3_CHNANEL_MODE_CMD  ((((mt_u32)HA_MP3_ID) << 16) | 0x1000)

typedef enum
{
    MP3_CHANNEL_MODE_STEREO = 0,
    MP3_CHANNEL_MODE_JOINTSTERERO,
    MP3_CHANNEL_MODE_DUALSTERERO,
    MP3_CHANNEL_MODE_MONO,
}HA_MP3_CHANNEL_MODE;

typedef struct
{
    mt_u32  enCmd;              /* HA_CODEC_GET_MP3_CHNANEL_MODE_CMD */
    HA_MP3_CHANNEL_MODE *peChannelMode;
} HA_MP3_GET_CHNANEL_MODE_S;

#define HA_MP3_DecGetDefalutOpenParam(pOpenParam) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = MT_NULL; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = 0; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
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

#endif /* __MTSI_AUDIO_DECODER_MP3_H__ */

