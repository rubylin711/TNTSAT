/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __AV_PVR_TYPE_H__
#define __AV_PVR_TYPE_H__

/*!
  This structure defines the supported formats of video source
  */
typedef enum
{
  /*!
    MPEG
    */
  VIDEO_MPEG,
  /*!
    H.264/AVC
    */
  VIDEO_H264,
  /*!
    AVS
    */
  VIDEO_AVS,
  /*!
    MPEG4
    */
  VIDEO_MPEG4,
    /*!
    MPEG ES
    */
  VIDEO_MPEG_ES,
  /*!
    H.264/AVC ES
    */
  VIDEO_H264_ES,
  /*!
    AVS ES
    */
  VIDEO_AVS_ES,
  /*!
    MPEG4 ES
    */
  VIDEO_MPEG4_ES,
  /*!
    Unknown video format
    */
  VIDEO_UNKNOWN
}vdec_src_fmt_t;

/*!
  This structure defines the supported formats of audio source
  */
typedef enum
{
  /*!
    PCM
    */
  AUDIO_PCM = 0,
  /*!
    MPEG audio layer I
    */
  AUDIO_MP1 = 1,
  /*!
    MPEG audio layer II
    */
  AUDIO_MP2 = 2,
  /*!
    MPEG audio layer III
    */
  AUDIO_MP3 = 3,
  /*!
     AC3
    */
  AUDIO_AC3_VSB = 4,
  /*!
    EAC3
    */
  AUDIO_EAC3 = 5,
  /*!
    HE_AAC
    */
  AUDIO_AAC = 6,
  /*!
    AAC_V2
    */
  AUDIO_AAC_V2 = 7,
  /*!
    SPDIF
    */
  AUDIO_SPDIF = 107,
  /*!
    dolby convert
    */
  AUDIO_DOLBY_CONVERT = 108,
  /*!
   SPDIF_AC3
    */
  AUDIO_SPDIF_AC3,
  /*!
   SPDIF_EAC3
    */
  AUDIO_SPDIF_EAC3,
  /*!
    Unknown audio format.
    */
  AUDIO_UNKNOWN  
}adec_src_fmt_vsb_t;

#endif

