/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_FFMPEG_WMAPRO_H__
#define __MTSI_AUDIO_DECODER_FFMPEG_WMAPRO_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


#define HA_WMAPRO_ID 0x041f
#define HA_AUDIO_ID_FFMPEG_WMAPRO   HA_BUILD_CODEC_ID(VENDOR_MTSI,FORMAT_WMAPRO,HA_WMAPRO_ID)  /* support FORMAT_WMAPRO */


typedef struct hiWMAPro_FORMAT_S
{
    mt_void *               hAvCtx; /* Same as libavcodec AVCodecContext, Set/allocated/freed by user. */

    mt_u16 wFormatTag;          /* format type,0x160->WMAV1,0x161->WMAV2, 0x162->WMAV3 */
    mt_u16 nChannels;            /* number of channels (i.e. mono, stereo...) */
    mt_u32 nSamplesPerSec;   /* sample rate */
    mt_u32 nAvgBytesPerSec;  /* for buffer estimation */
    mt_u16 nBlockAlign;          /* block size of data */
    mt_u16 wBitsPerSample;   /* number of bits per sample of mono data */
    mt_u16 cbSize;                /* the count in bytes of the size of */
    mt_u8 cbExtWord[32];       /* extra information (after cbSize).
                                WMAV1: need  4 Bytes extra information at least
                                    WMAV2: need 10 Bytes extra information at least
                                WMAV3: need 18 Bytes extra information at least
                                 */
} WMAPro_FORMAT_S;


#define HA_FFMPEGC_WMAPROC_DecGetDefalutOpenParam(pOpenParam, pstWmaProConfig) \
    do { mt_s32 i; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved = MT_TRUE; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
         for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
         { \
             ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
         } \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = pstWmaProConfig; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(WMAPro_FORMAT_S); \
    } while (0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_FFMPEG_WMAPRO_H__ */
