/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_COOK_H__
#define __MTSI_AUDIO_DECODER_COOK_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define    HA_COOK_ID 0x0009        

/*
 * ra_format_info struct
 *
 */

typedef struct  hiRA_FORMAT_INFO_S
{
    mt_u32 ulSampleRate;
    mt_u32 ulActualRate;
    mt_u16 usBitsPerSample;
    mt_u16 usNumChannels;
    mt_u16 usAudioQuality;
    mt_u16 usFlavorIndex;
    mt_u32 ulBitsPerFrame;
    mt_u32 ulGranularity;
    mt_u32 ulOpaqueDataSize;
    mt_u8*  pOpaqueData;
} RA_FORMAT_INFO_S;


#define HA_COOK_DecGetDefalutOpenParam(pOpenParam, pstPrivateParams) \
		do { mt_s32 i; \
			 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
			 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
			 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved = MT_TRUE; \
			 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
			 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
			 for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
			 { \
				 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
			 } \
			 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
			 ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(RA_FORMAT_INFO_S); \
		} while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_COOK_H__ */

