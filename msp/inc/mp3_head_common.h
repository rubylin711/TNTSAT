

#ifndef __MP3_HEAD_COMMON_H__
#define __MP3_HEAD_COMMON_H__

#ifdef CONFIG_MT_CHIP_ARIA
#define   MP3_PLATFORM_ARIA_1
#endif
#define   AUD_MAX_CHANNEL   8

#define AV_PRINTF  printf

#include "mt_type.h"
//#include "../mt_type.h"
//#include "../aud_dec.h"


#include "mt_type.h"
#include "aud_dec.h"

// define in aud_dec.h

/*!
  This structure defines the supported PCM sample rates.
  */
typedef enum
{
  /*!
    Sample rate is 22.05k 
    */
  AUDIO_SAMPLE_22_VSB = 0,
  /*! 
    Sample rate is 24k 
    */
  AUDIO_SAMPLE_24_VSB = 1,
  /*!
    Sample rate is 16k 
    */
  AUDIO_SAMPLE_16_VSB = 2,
  /*!
    Sample rate reserved1
    */
  AUDIO_SAMPLE_RES1_VSB = 3,
  /*! 
    Sample rate is 44.1k 
    */
  AUDIO_SAMPLE_44_VSB = 4,
  /*! 
    Sample rate is 48k 
    */
  AUDIO_SAMPLE_48_VSB = 5,
  /*! 
    Sample rate is 32k
    */
  AUDIO_SAMPLE_32_VSB = 6,
  /*! 
    Sample rate reserved2
    */
  AUDIO_SAMPLE_RES2_VSB = 7,
  /*! 
    Sample rate is 11k
    */
  AUDIO_SAMPLE_11_VSB = 8,
  /*! 
    Sample rate is 12k
    */
  AUDIO_SAMPLE_12_VSB = 9,
  /*! 
    Sample rate is 8k
    */
  AUDIO_SAMPLE_8_VSB = 10,
  /*! 
    Sample rate reserved3
    */
  AUDIO_SAMPLE_RES3_VSB = 11,
  /*! 
    Sample rate is 88k
    */
  AUDIO_SAMPLE_88_VSB = 12,
  /*! 
    Sample rate is 96k
    */
  AUDIO_SAMPLE_96_VSB = 13,
  /*! 
    Sample rate is 64k
    */
  AUDIO_SAMPLE_64_VSB = 14,
  /*!
    RESERVED
    */
  AUDIO_SAMPLE_RESERVED,
} aud_sample_rate_vsb_t;


/*!
  This structure defines input audio source format when file play
  */
typedef enum
{

/*!
   ac3 audio frame  format type
  */
  AC3_TYPE = 0,
  
/*!
   eac3 audio frame  format type
  */ 
  EAC3_TYPE,

/*!
   AAC ADIF audio frame format type
  */
  AAC_ADIF_TYPE,

/*!
   AAC ADTS audio frame format type
  */
  AAC_ADTS_TYPE,

/*!
   PCM format type
  */
  PCM_TYPE,

/*!
   other unknown audio frame format type
  */
  UNKNOWN_TYPE
}audio_frm_type_t;

#if 0
typedef struct
{
  /*!
      errors
    */
  u32 error;

  /*!
      audio out clock
    */
  u32 aud_out_clock;

  /*!
      AP info addr
    */
  u32 aud_AP_info_addr;

  
  /*!
      audio channel exist
    */
  u8 channel;

  /*!
      audio channel exist before
    */
  u8 channel_pre;

  /*!
      audio channel exist changing counter
    */
  u8 channel_cnt;

  /*!
      for alin
    */
  u8 channel_reserved;

  /*!
      sampling_rate for hw actually used for audio output
    */
  u32 hw_sample_rate;
  
  /*!
      sampling_rate, current src audio sample
    */
  u32 sampling_rate;

  /*!
      sampling_rate before
    */
  u32 sampling_rate_pre;

  /*!
      sampling_rate changing counter
    */
  u32 sampling_rate_cnt;


  /*!
      buf to output
    */
  char *p_aud_check_buf[AUD_MAX_CHANNEL];

  /*!
      buf length
    */
  u32 aud_chech_frame_len;

  /*!
      frame id, when decoder one frame, and sned it to audio render, this frame_id++
    */
  u32 frame_id;

  /*!
    the aud pts is valid or not 
    */
  MT_BOOL aud_pts_valid;
  
  /*!
      audio pts, every frame sent to audio render, need be connected by a audio pts,and a frame_id
    */
  u32 aud_pts;

  /*!
      audio sync enable
    */
  MT_BOOL aud_sync_en;

  /*!
      es depth
    */
  u32 es_depth;

  /*!
      dec cnt
    */
  u32 dec_cnt;

  /*!
      Mono Left Right Stereo
    */
  u32 output_fmt;

  /*!
      Mono Left Right Stereo
    */
  u32 output_fmt_pre;

  /*!
      one frame sample num in src input, this value must be set by audio decoder
    */
  u32 src_frame_sample_num;

  /*!
      one frame sample num in src output, 
      this value maybe not same as the src_frame_sample_num, if src enable 

      output_frame_sample_num = src_frame_sample_num * output sample rate / input sample rate
    */
  u32 output_frame_sample_num;

  /*!
      output_frame_sample_num = 45000 * src_frame_sample_num / input sample rate
    */
  u32 apts_step;
  /*!
     the last aud pts id
    */
  u32 aud_pts_last;

  /*!
   the last aud pts id
  */
  u32 aud_pts_id_last;
  u32 aud_skip_mode;
  /*!
    audio pts from aout
    */
  u32 aud_pts_output;

  /*!
     mp3_bitrate
   */
  u32                 mp3_bitrate;

  /*!
     mp3_mode
   */
  u32                 mp3_mode;
}aud_check_t;
#endif
#endif
