/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_FFMPEG_DECODE_H__
#define __MTSI_AUDIO_DECODER_FFMPEG_DECODE_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


#define HA_FFMPEG_ID 0x03ff        

/**define status parameter of update information*/ /**CNcomment:状态信息更新，事件回调用户数据定义*/
typedef struct  hiFFMPEG_STATUS_INFO_S
{
    mt_char *name;
    mt_s32  codec_id;
    mt_s32  codec_type;
    mt_s32  sample_fmt;
    mt_s32  channels;
    mt_s32  sample_rate;
    mt_s32  bit_rate;
    mt_s32  block_align;
} FFMPEG_STATUS_INFO_S;

/**define the type of callback function*/ /**CNcomment:事件回调函数类型*/
typedef enum
{
    HA_FFMPEG_EVENT_STATUS_CHANGE = 0,                   /**<update status information by every frame*/ /**<CNcomment:状态信息更新, 每帧更新一次*/
    HA_FFMPEG_EVENT_BUTT
} HA_FFMPEG_EVENT_E;

/**define callback function*/ /**CNcomment:定义事件回调函数枚举类型*/
typedef mt_void (*FFMPEG_EVENT_CB_FN)(mt_void* pAppData1, mt_void* pAppData2, HA_FFMPEG_EVENT_E enEvent);

typedef struct  hiHA_FFMPEG_DECODE_OPENCONFIG_S
{
    mt_void *               hAvCtx; /* Same as libavcodec AVCodecContext, Set/allocated/freed by user. */
    FFMPEG_STATUS_INFO_S ffmpeg_info;	
    MT_BOOL                 bAutoDmxStereo; /* automatic downmix multi-ch to stereo. */
    MT_BOOL                 bConvert2fmt16bit; /* automatic convert none-16bit pcm to 16bit pcm. */
    FFMPEG_EVENT_CB_FN      pfnEvtCbFunc[HA_FFMPEG_EVENT_BUTT];    /* call back method */
    mt_void*                pAppData1[HA_FFMPEG_EVENT_BUTT];       /* application handle,   Set/allocated/freed by user. */
    mt_void*                pAppData2[HA_FFMPEG_EVENT_BUTT];       /* application defined value for call back method,  allocated/freed by user, set by ha_codec,*/
} HA_FFMPEG_DECODE_OPENCONFIG_S;

#define HA_FFMPEG_DecGetDefalutOpenConfig(pConfigParam) \
    do { \
        ((HA_FFMPEG_DECODE_OPENCONFIG_S *)(pConfigParam))->bAutoDmxStereo = MT_TRUE; \
        ((HA_FFMPEG_DECODE_OPENCONFIG_S *)(pConfigParam))->bConvert2fmt16bit = MT_TRUE; \
    } while (0)

#define HA_FFMPEGC_DecGetDefalutOpenParam(pOpenParam, pFfmpegConfig) \
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
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = pFfmpegConfig; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(HA_FFMPEG_DECODE_OPENCONFIG_S); \
         for (i = 0; i < HA_FFMPEG_EVENT_BUTT; i++) \
         { \
             ((HA_FFMPEG_DECODE_OPENCONFIG_S *)(pFfmpegConfig))->pfnEvtCbFunc[i] = MT_NULL; \
             ((HA_FFMPEG_DECODE_OPENCONFIG_S *)(pFfmpegConfig))->pAppData1[i] = MT_NULL; \
             ((HA_FFMPEG_DECODE_OPENCONFIG_S *)(pFfmpegConfig))->pAppData2[i] = MT_NULL; \
         } \
    } while (0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_FFMPEG_DECODE_H__ */
