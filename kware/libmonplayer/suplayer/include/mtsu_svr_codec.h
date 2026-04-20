/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file
 \brief Codec type. CNcomment:Codec 类型CNend
 \author Montage Technologies Co., Ltd.
 \date 2008-2018
 \version 1.0
 \author
 \date 2017-11-10
 */

#ifndef __MT_SVR_CODEC_H__
#define __MT_SVR_CODEC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/***************************** Structs Definition ******************************/
/** \addtogroup     Suplayer */
/** @{ */  /** <!--[Suplayer] */

/** Subtitle data type */
/** CNcomment:字幕数据类型 */
typedef enum mtFORMAT_SUBTITLE_TYPE_E
{
    MT_FORMAT_SUBTITLE_ASS = 0x0,    /**< ASS subtitle *//**< CNcomment:ASS字幕*/
    MT_FORMAT_SUBTITLE_LRC,       /**< LRC subtitle *//**< CNcomment:LRC字幕*/
    MT_FORMAT_SUBTITLE_SRT,       /**< SRT subtitle *//**< CNcomment:SRT字幕*/
    MT_FORMAT_SUBTITLE_SMI,       /**< SMI subtitle *//**< CNcomment:SMI字幕*/
    MT_FORMAT_SUBTITLE_SUB,       /**< SUB subtitle *//**< CNcomment:SUB字幕*/
    MT_FORMAT_SUBTITLE_TXT,       /**< RAW UTF8 subtitle *//**< CNcomment:RAW UTF8字幕*/

    MT_FORMAT_SUBTITLE_HDMV_PGS,      /**< pgs subtitle *//**< CNcomment:pgs字幕 */
    MT_FORMAT_SUBTITLE_DVB_SUB,       /**< DVB subtitle *//**< CNcomment:DVBsub字幕 */
    MT_FORMAT_SUBTITLE_DVD_SUB,       /**< DVD subtitle *//**< CNcomment:DVDsub字幕 */
    MT_FORMAT_SUBTITLE_TTML,          /**< TTML subtitle *//**< CNcomment:TTML字幕 */

    MT_FORMAT_SUBTITLE_BUTT
} MT_FORMAT_SUBTITLE_TYPE_E;

/** Audio data type */
/** CNcomment:音频数据类型 */
typedef enum mtFORMAT_AUDIO_TYPE_E
{
    MT_FORMAT_AUDIO_MP2 = 0x000,  /**< MPEG audio layer 1, 2.*//**<CNcomment:MPEG音频第一层、第二层 */
    MT_FORMAT_AUDIO_MP3,          /**< MPEG audio layer 1, 2, 3.*//**<CNcomment:MPEG音频第一层、第二层 、第三层 */
    MT_FORMAT_AUDIO_AAC,
    MT_FORMAT_AUDIO_AC3,
    MT_FORMAT_AUDIO_DTS,
    MT_FORMAT_AUDIO_VORBIS,
    MT_FORMAT_AUDIO_DVAUDIO,
    MT_FORMAT_AUDIO_WMAV1,
    MT_FORMAT_AUDIO_WMAV2,
    MT_FORMAT_AUDIO_MACE3,
    MT_FORMAT_AUDIO_MACE6,
    MT_FORMAT_AUDIO_VMDAUDIO,
    MT_FORMAT_AUDIO_SONIC,
    MT_FORMAT_AUDIO_SONIC_LS,
    MT_FORMAT_AUDIO_FLAC,
    MT_FORMAT_AUDIO_MP3ADU,
    MT_FORMAT_AUDIO_MP3ON4,
    MT_FORMAT_AUDIO_SHORTEN,
    MT_FORMAT_AUDIO_ALAC,
    MT_FORMAT_AUDIO_WESTWOOD_SND1,
    MT_FORMAT_AUDIO_GSM,
    MT_FORMAT_AUDIO_QDM2,
    MT_FORMAT_AUDIO_COOK,
    MT_FORMAT_AUDIO_TRUESPEECH,
    MT_FORMAT_AUDIO_TTA,
    MT_FORMAT_AUDIO_SMACKAUDIO,
    MT_FORMAT_AUDIO_QCELP,
    MT_FORMAT_AUDIO_WAVPACK,
    MT_FORMAT_AUDIO_DSICINAUDIO,
    MT_FORMAT_AUDIO_IMC,
    MT_FORMAT_AUDIO_MUSEPACK7,
    MT_FORMAT_AUDIO_MLP,
    MT_FORMAT_AUDIO_GSM_MS, /**< as found in WAV.*//**<CNcomment:存在WAV格式中 */
    MT_FORMAT_AUDIO_ATRAC3,
    MT_FORMAT_AUDIO_VOXWARE,
    MT_FORMAT_AUDIO_APE,
    MT_FORMAT_AUDIO_NELLYMOSER,
    MT_FORMAT_AUDIO_MUSEPACK8,
    MT_FORMAT_AUDIO_SPEEX,
    MT_FORMAT_AUDIO_WMAVOICE,
    MT_FORMAT_AUDIO_WMAPRO,
    MT_FORMAT_AUDIO_WMALOSSLESS,
    MT_FORMAT_AUDIO_ATRAC3P,
    MT_FORMAT_AUDIO_EAC3,
    MT_FORMAT_AUDIO_SIPR,
    MT_FORMAT_AUDIO_MP1,
    MT_FORMAT_AUDIO_TWINVQ,
    MT_FORMAT_AUDIO_TRUEHD,
    MT_FORMAT_AUDIO_MP4ALS,
    MT_FORMAT_AUDIO_ATRAC1,
    MT_FORMAT_AUDIO_BINKAUDIO_RDFT,
    MT_FORMAT_AUDIO_BINKAUDIO_DCT,
    MT_FORMAT_AUDIO_DRA,
    MT_FORMAT_AUDIO_DTS_EXPRESS,

    MT_FORMAT_AUDIO_PCM = 0x100,   /**< various PCM codecs. *//**<CNcomment:PCM格式 */
    MT_FORMAT_AUDIO_PCM_BLURAY = 0x121,

    MT_FORMAT_AUDIO_ADPCM = 0x130, /**< various ADPCM codecs. *//**<CNcomment:ADPCM格式 */

    MT_FORMAT_AUDIO_AMR_NB = 0x160,/**< various AMR codecs. *//**<CNcomment:AMR格式 */
    MT_FORMAT_AUDIO_AMR_WB,
    MT_FORMAT_AUDIO_AMR_AWB,

    MT_FORMAT_AUDIO_RA_144 = 0x170, /**< RealAudio codecs. *//**<CNcomment:RealAudio格式 */
    MT_FORMAT_AUDIO_RA_288,

    MT_FORMAT_AUDIO_DPCM = 0x180, /**< various DPCM codecs. *//**<CNcomment:DPCM格式 */

    MT_FORMAT_AUDIO_G711 = 0x190, /**< various G.7xx codecs. *//**<CNcomment:G.7xx格式 */
    MT_FORMAT_AUDIO_G722,
    MT_FORMAT_AUDIO_G7231,
    MT_FORMAT_AUDIO_G726,
    MT_FORMAT_AUDIO_G728,
    MT_FORMAT_AUDIO_G729AB,

    MT_FORMAT_AUDIO_MULTI = 0x1f0, /**< support multi codecs. *//**<CNcomment:多种格式 */

    MT_FORMAT_AUDIO_BUTT = 0x1ff,
} MT_FORMAT_AUDIO_TYPE_E;

/** Video data type */
/** CNcomment:视频数据类型 */
typedef enum mtFORMAT_VIDEO_TYPE_E
{
    MT_FORMAT_VIDEO_MPEG2 = 0x0, /**< MPEG2*/
    MT_FORMAT_VIDEO_MPEG4,       /**< MPEG4 DIVX4 DIVX5*/
    MT_FORMAT_VIDEO_AVS,         /**< AVS*/
    MT_FORMAT_VIDEO_H263,        /**< H263*/
    MT_FORMAT_VIDEO_H264,        /**< H264*/
    MT_FORMAT_VIDEO_REAL8,       /**< REAL*/
    MT_FORMAT_VIDEO_REAL9,       /**< REAL*/
    MT_FORMAT_VIDEO_VC1,         /**< VC-1*/
    MT_FORMAT_VIDEO_VP6,         /**< VP6*/
    MT_FORMAT_VIDEO_VP6F,        /**< VP6F*/
    MT_FORMAT_VIDEO_VP6A,        /**< VP6A*/
    MT_FORMAT_VIDEO_MJPEG,       /**< MJPEG*/
    MT_FORMAT_VIDEO_SORENSON,    /**< SORENSON SPARK*/
    MT_FORMAT_VIDEO_DIVX3,       /**< DIVX3, not supported*/
    MT_FORMAT_VIDEO_RAW,         /**< RAW*/
    MT_FORMAT_VIDEO_JPEG,        /**< JPEG added for VENC*/
    MT_FORMAT_VIDEO_VP8,         /**<VP8*/
    MT_FORMAT_VIDEO_MSMPEG4V1,   /**< MS private MPEG4 */
    MT_FORMAT_VIDEO_MSMPEG4V2,
    MT_FORMAT_VIDEO_MSVIDEO1,    /**< MS video */
    MT_FORMAT_VIDEO_WMV1,
    MT_FORMAT_VIDEO_WMV2,
    MT_FORMAT_VIDEO_RV10,
    MT_FORMAT_VIDEO_RV20,
    MT_FORMAT_VIDEO_SVQ1,        /**< Apple video */
    MT_FORMAT_VIDEO_SVQ3,        /**< Apple video */
    MT_FORMAT_VIDEO_H261,
    MT_FORMAT_VIDEO_VP3,
    MT_FORMAT_VIDEO_VP5,
    MT_FORMAT_VIDEO_CINEPAK,
    MT_FORMAT_VIDEO_INDEO2,
    MT_FORMAT_VIDEO_INDEO3,
    MT_FORMAT_VIDEO_INDEO4,
    MT_FORMAT_VIDEO_INDEO5,
    MT_FORMAT_VIDEO_MJPEGB,
    MT_FORMAT_VIDEO_MVC,
    MT_FORMAT_VIDEO_HEVC,        /**< HEVC(H265)*/
    MT_FORMAT_VIDEO_DV,
    MT_FORMAT_VIDEO_HUFFYUV,
    MT_FORMAT_VIDEO_DIVX,           /**< DIVX,not supported*/
    MT_FORMAT_VIDEO_REALMAGICMPEG4, /**< REALMAGIC MPEG4,not supported*/
    MT_FORMAT_VIDEO_BUTT
} MT_FORMAT_VIDEO_TYPE_E;


/** @} */  /** <!-- ==== Structure Definition end ==== */

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* #ifndef __MT_SVR_CODEC_H__ */
