/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <string.h>
#include "mt_type.h"
#include "mt_unf_avplay.h"
#include "mt_unf_video.h"
#include "mt_unf_dma.h"
#include "mt_unf_cipher_v2.h"

 #ifndef _DRV_ADP_H_
#define _DRV_ADP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* fw define invalid pts 0 */
#define   ADP_FW_INVALID_PTS     0

/*!
  This structure defines the video decoding state
  */
typedef struct
{
  /*!
    decoding error info
    */
  int err;
/*!
  The aspect ratio
  */
  int  ar;//aspect_ratio_t
  /*!
    current source data is supported
    */
  MT_BOOL is_sup;
  /*!
    video decoding is ready for display
    */
  MT_BOOL is_stable;
  /*!
    AV Sync is OK.
    */
  MT_BOOL is_sync;
  //add by HY 2012-3-19 begin
  /*!
    DMA overflow
    */
  MT_BOOL is_overflow;
    /*!
  current source format is supported by video decoder
  */
  MT_BOOL is_format;
  //add by HY 2012-3-19 end
  //add by HY for autotest 2012-3-28 begin
  /*!
    Key frame.
    */
  int is_key_frame[20];
  /*!
    luma address
    */
  unsigned int luma_addr[20];
  /*!
    chroma address
    */
  unsigned int chroma_addr[20];
  /*!
    heigth
    */
  unsigned int heigth;
  /*!
    width
    */
  unsigned int width;
  /*!
    cur frames pts, 0 is invalidt
    */
  u32 pts;
  //add by HY for autotest 2012-3-28 end
  /*!
    the format of input video
    */
  int vid_format;//disp_sys_t
  /*!
    Autotest flag
    */
  MT_BOOL is_autotest;
  /*!
    frames statistic, have display frames
    */
  u32 frames_stat;
  /*!
    frames statistic, have decoder frames
    */
  u32 decoder_frame_count;
  /*!
    open or close di flage
  */
  MT_BOOL open_di;
  /*!
    frame rate
  */
  u32 frame_rate;
  /*!
    no enough buffer for video decoder
  */
  MT_BOOL no_enough_buffer;
  /*!
    First I frame output
  */
  MT_BOOL I_frame_stable;
  /*!
    If image is sd image, the flage is true.
  */
  MT_BOOL sd_image_flag;
  /*!
    If image is right, the flage is true.
  */
  MT_BOOL image_info_enable;
} vdec_info_t;

/*!
  This structure defines the video current play time
  */
typedef struct {
  /*! cur frames pts, 0 is invalidt */
  u64 pts;
  /*!  the current playtime */
  u64 playtime;
  /*! cur audio frames pts, 0 is invalidt */
  u64 apts;
  /*! cur audio frames count */
  u64 aframe_count;
} drv_pts_info_t;

void file_seq_register_event(MT_UNF_AVPLAY_EVENT_E enEvent, MT_UNF_AVPLAY_EVENT_CB_FN pfnEventCB);
void file_seq_unregister_event(MT_UNF_AVPLAY_EVENT_E enEvent);
void file_seq_avplay_handle_set(MT_HANDLE handle_avplay, int handle_track);
void file_seq_suplayer_handle_set(int handle_suplayer);
void *dev_find_identifier(void *p_sdev, int ident_type, u32 ident);

RET_CODE set_video_decoder_framerate(uint32_t frame_rate);
RET_CODE dev_open(void *p_dev, void *p_param);
RET_CODE vdec_pause(void *p_dev);
RET_CODE vdec_resume(void *p_dev);
RET_CODE vdec_flush(void * p_dev);
RET_CODE vdec_reset(void);
RET_CODE vdec_freeze_stop(void *p_dev);
RET_CODE vdec_file_clearesbuffer(void *p_dev);
RET_CODE vdec_get_es_buf_space(void *p_dev, u32 *p_size);
RET_CODE vdec_file_get_es_buf_info(void *p_dev, mt_u32 *all_size, mt_u32 *use_size);
RET_CODE vdec_get_es_buf_pkt_num(void *p_dev, int *pkt_num);
RET_CODE vdec_file_get_water_level_vsb(void *p_dev);
RET_CODE vdec_set_avsync_mode(void *p_dev,u32 mode);
RET_CODE vdec_start(void *p_dev, int format, int mode);
RET_CODE vdec_stop(void *p_dev);
RET_CODE vdec_get_es_buf_size(void *p_dev, u32 *p_size);
RET_CODE vdec_get_info(void *p_dev, void *p_state);
RET_CODE vdec_get_pts(void *p_dev, void *p_state);
RET_CODE vdec_get_frame_info(void *p_dev, void *p_state);
RET_CODE vdec_set_trick_mode(void *p_dev, int mode, u8 sr);
RET_CODE vdec_set_trick_mode_2(mt_s32 playrate);
void vdec_get_pts_2(s64 *out_pts);
RET_CODE vdec_set_avsync_mode_2(void *p_dev,u32 mode);
RET_CODE vdec_check_exit(void);
RET_CODE vdec_dec_push_es(void *p_dev, unsigned char *src_addr, u32 size,u64 vpts, int eos_flag);
RET_CODE vdec_dec_push_vpx_es(void *p_dev,
    u8 *addr, u32 size, u64 vpts, int eos_flag);
RET_CODE vdec_do_avsync_cmd(void *p_dev, int cmd, u32 data);
void vdec_set_seek_frame_setup_mode(MT_BOOL is_setup);
MT_BOOL vdec_get_seek_frame_setup_mode(void);
RET_CODE aud_set_dec_param_vsb(void *p_dev, const void *p_param,int aType);
RET_CODE aud_pause_vsb(void *p_dev);
RET_CODE aud_resume_vsb(void *p_dev);
RET_CODE aud_start_vsb(void *p_dev, int type);
RET_CODE aud_stop_vsb(void *p_dev);
RET_CODE aud_file_getleft_ao_pcm_bytes(void *p_dev, u32 *p_size);
RET_CODE aud_file_getleftesbuffer_vsb(void *p_dev, u32 *p_size);
RET_CODE aud_file_get_water_level_vsb(void *p_dev);
RET_CODE aud_file_get_es_buf_info(void *p_dev, mt_u32 *all_size, mt_u32 *use_size);
RET_CODE aud_file_gettotalesbuffer_vsb(void *p_dev, u32 *p_size);
// RET_CODE aud_mute_onoff_vsb(void *p_dev, MT_BOOL is_on);
RET_CODE aud_get_pts(void *p_dev, void *p_state);
RET_CODE aud_file_pushesbuffer_vsb(void *p_dev, u8 *src_addr, u32 size, u64 apts, u32 eos);
RET_CODE aud_file_extra_pushesbuffer_vsb(void *p_dev, u64 apts, u32 eos);
RET_CODE disp_set_tv_sys(void *p_dev, int ch, int fmt);
RET_CODE disp_get_tv_sys(void *p_dev, int ch, int *p_fmt);
unsigned char *audio_free_es_tmp_buf(void *p_dev);
unsigned char *audio_get_es_tmp_buf(void *p_dev);
unsigned short *Convert_Utf8_To_Unicode(unsigned char * putf8,unsigned short* out);
RET_CODE vdec_set_dec_frm_type(void *p_dev, MT_UNF_DEC_FRM_TYPE_E dec_frm_type);
RET_CODE vdec_set_trick_cfg(void *p_dev, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam);
RET_CODE suplayer_is_stop_send_audio(void);
RET_CODE suplayer_is_stop_send_video(void);
RET_CODE suplayer_get_dump_aud_flag(void);
RET_CODE suplayer_get_dump_vid_flag(void);
RET_CODE suplayer_get_dump_subt_flag(void);

RET_CODE vdec_get_stream_info(void *p_dev, MT_UNF_AVPLAY_STREAM_INFO_S *pstStreamInfo);
RET_CODE fileplay_set_avsync_mode(MT_UNF_SYNC_REF_E enAVsyncMode);

RET_CODE set_hdr_info(void *p_dev, MT_UNF_VIDEO_DISP_HDR_INFO_S *hdrInfo);
RET_CODE set_tplay_normal_play_num(void *p_dev, mt_u32 t1xnum);
RET_CODE get_tplay_normal_play_num(void);
RET_CODE wb_auto_init_es_check_sum(int max_frame, int max_asize);
RET_CODE wb_auto_get_es_check_sum(void *ves, void *aes, void *ses);
RET_CODE wb_auto_aud_init_dec_param(void);
RET_CODE wb_auto_aud_get_dec_param(void *ptr, int size);

#if MT_DES("AV PUSH Interface", 1)
#include "mtlz_avfilter.h"
void drv_adp_init_avif(void *p_dev,
    void *ctx, MTAVSF_MSG_FUNC msb_cb);
void drv_adp_flush_avif(void *p_dev);
void drv_adp_deinit_avif(void *p_dev);

int drv_adp_push_audio(void *p_dev, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag);
int drv_adp_push_video(void *p_dev, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag);
#endif

/*
typedef int (*vpx_superframe_parser_t)(
    const unsigned char *, const unsigned int, vp9_superframe_info_t *);



vpx_superframe_parser_t vpx_get_superframe_parser(void);
int aud_file_pushesbuffer_vsb_vmx(void *p_dev, u8 *src_addr, 
											u32 size, u64 apts, u32 eos);
*/
unsigned long long get_pack_magic_num_wb(unsigned char *src, 
			int pkt_size, u64 pts, unsigned long long last_sum, 
			int cnt, int max_asize, int mode);
#ifdef __cplusplus
}
#endif

#endif
