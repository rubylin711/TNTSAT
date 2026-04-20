/******************************************************************************/
/* Copyright (c) 2012 Montage Tech - All Rights Reserved                      */
/******************************************************************************/
#ifndef __AV_AUD_DEC_H__
#define __AV_AUD_DEC_H__

/*!
  This declare an audio decoder handle
  */
#define INVALID_DEC_DATA         (0x00)  

/*!
  dec error reset dec 
  */
#define AUD_DEC_ERROR_CODE_RESET (1937)

/*!
  This define an audio decoder config info 
  */
typedef struct
{
/*!
  the buf address used for mp3 decoder,  uperlayer config
  */
  u32 buf_addr;

/*!
  the buf size used for mp3 decoder, uperlayer config
  */
  u32 buf_size;

/*!
  the output format to select PCM , AC3 or EAC3
  */
  u8 output_format;

/*!
  the data for personal using,itaf as argc
  */
  u32 tag1;
/*!
  the data for personal using, itaf as argv
  */
  u32 tag2;
/*!
  the data for personal using,itaf as pipe id
  */
  u32 tag3;
}adec_fw_config_t;



/*!
  This function  inquire the buf size of decoder
  */
typedef u32 (* dec_inquire_fun_t)(void);


/*!
  init the dec with the config info, return dec handle
  */
typedef void *(* dec_init_fun_t)(adec_fw_config_t *p_config);


/*!
  This declare an audio decoder deinit func 
  */
typedef void (* dec_deinit_fun_t)(void *p_priv);


/*!
  This declare an audio decoder main decoding func 
  */
typedef u32  (* dec_main_fun_t)(void *p_priv,
                                   char *p_start, 
                                   u32 length, 
                                   u32 *p_unconsumed,
                                   u32 *p_required,
                                   u32 tag);


/*!
  This structure defines an audio dec interface
  */
 typedef struct 
{
  /*!
    Pointer to dec private data handle
    */
  void *p_priv;

  /*!
    declare to dec inquire buf size function
    */
  dec_inquire_fun_t   dec_inquire;

  /*!
    declare to dec inquire buf size function
    */
  dec_init_fun_t      dec_init;

  /*!
    declare to dec inquire buf size function
    */
  dec_main_fun_t      dec_main;

  /*!
    declare to dec inquire buf size function
    */
  dec_deinit_fun_t    dec_deinit;
  
  
}adec_fw_dev_t;

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
      compare mode with PC
    */
  u32 comp_mode;

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
  char *p_aud_check_buf[8];

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
    
  /*!
      **
    */
  u32 aud_skip_mode;

  /*!
      **
    */
  MT_BOOL b_kernel_mute;

  /*!
    for ad
    */
  u32 ad_volue_mode;
  /*!
    for ad
    */
  u32 ad_volue_mode_pre;

  /*!
    for ad
    */ 
  u32 ad_vol_flag;

  /*!
      **
    */
  char *zero_buffer;

  /*!
     a temp value to save the pcm write buf addr(used in fpga)
   */
  u32                 pcm_write_buf;

  /*!
     pcm buf start addr
   */
  u32                 pcm_buf_start;

  /*!
     pcm buf end addr
   */
  u32                 pcm_buf_end;

  /*!
     spdif_buf_left
   */
  u32                 spdif_buf_left;

  /*!
     mp3_bitrate
   */
  u32                 mp3_bitrate;

  /*!
     mp3_mode
   */
  u32                 mp3_mode;
  /*!
    audio pts from aout
    */
  u32 aud_pts_output;

  /*!
    reset dec
    */
  u32 aud_reset_dec;


  /*!
     eos flag
   */
  MT_BOOL              b_eos;
  
  MT_HA_AUDIO_STREAM_INFO_S audio_info;
} aud_check_t;


#endif //  __AV_AUD_DEC_H__


