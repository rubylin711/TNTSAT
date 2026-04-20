/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_PCM_H__
#define __MTSI_AUDIO_DECODER_PCM_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HA_PCM_ID 0x0000        

typedef struct hiWAV_FORMAT_S
{
    mt_u16 wFormatTag;          /* format category */
    mt_u16 nChannels;            /* number of channels (i.e. mono, stereo...) */
    mt_u32 nSamplesPerSec;   /* sample rate */
    mt_u32 nAvgBytesPerSec;  /* for buffer estimation */
    mt_u16 nBlockAlign;          /* the block alignment (in bytes) of the
                                      waveform data */
    mt_u16 wBitsPerSample;   /* number of bits per sample of mono data */
    mt_u16 cbSize;                /* number of bytes of wave raw data cbExtWord*/
    mt_u16 cbExtWord[16];       /* extra information (after cbSize).                      */
   /* note: big-endian pcm supprt(microsoft wav file only support little-endian pcm format):
        cbSize = 4;
        cbExtWord[0] = 1;//big-endian
        cbExtWord[1] ; //stero bit
   */
} WAV_FORMAT_S;

#define HA_PCM_DecGetDefalutOpenParam(pOpenParam, pstPrivateParams) \
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
    ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->u32CodecPrivateDataSize = sizeof(WAV_FORMAT_S); \
}while(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_PCM_H__ */

