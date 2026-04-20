/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_TRUEHD_H__
#define __MTSI_AUDIO_DECODER_TRUEHD_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HA_TRUEHD_ID 0x0008        

#define HA_TRUEHD_DecGetDefalutOpenParam(pOpenParam) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_THRU; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)NULL; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = 0; \
}while(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_TRUEHD_H__ */

