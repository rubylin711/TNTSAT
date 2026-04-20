/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_MAD_H__
#define __MTSI_AUDIO_DECODER_MAD_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HA_AUDIO_ID_MAD  0x10001001

static mt_s32 HA_MAD_DecGetDefalutOpenParam(MT_HADECODE_OPENPARAM_S * pOpenParam)
{
    mt_s32 i;

    if (!pOpenParam)
    {
        return HA_ErrorInvalidParameter;
    }

    pOpenParam->enDecMode = HD_DEC_MODE_RAWPCM;
    pOpenParam->pCodecPrivateData = MT_NULL;
    pOpenParam->u32CodecPrivateDataSize = 0;
    pOpenParam->sPcmformat.u32DesiredOutChannels = 2;
    pOpenParam->sPcmformat.bInterleaved  = MT_TRUE;
    pOpenParam->sPcmformat.u32BitPerSample = 16;
    pOpenParam->sPcmformat.u32DesiredSampleRate = 48000;
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++)
    {
        pOpenParam->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone;
    }
    return HA_ErrorNone;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_MAD_H__ */

