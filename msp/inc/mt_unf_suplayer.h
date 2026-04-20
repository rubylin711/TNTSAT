/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_suplayer.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*******************************************************************************/
/**
 * \file
 * \brief Describes the information about the audio/video player (SUPLAYER) module.
          CNcomment: “Ù ”∆µ≤•∑≈∆˜  CNend
 */
#ifndef __MT_UNF_SUPLAYER_H__
#define __MT_UNF_SUPLAYER_H__

#include "mt_unf_common.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

/*************************** Structure Definition ****************************/
/** \addtogroup      SUPLAYER */
/** @{ */  /** <!-- [SUPLAYER] */


/* ++++++++++++ proc struct for app start +++++++++++++*/
/* The following structs should be the same with that in mt_unf_drv_suplayer_ioctl.h which named */
/* xx_T */
typedef enum mtSOURCE_TYPE_OUT_E
{
    SOURCE_TYPE_LOCAL_OUT = 0,

    SOURCE_TYPE_NETWORK_OUT,

    SOURCE_TYPE__BUTT_OUT
}SOURCE_TYPE_OUT_T;

typedef struct mtSUPLAYER_FILE_INFO_VIDIO_OUT_S
{
    mt_u32                          stream_idx;
    char                                fourcc[5];
    mt_u32                          codec_id;
    mt_u32                          width;
    mt_u32                          height;
    mt_u32                          fps;
    mt_u32                          bps;
    mt_u32                          duration;
}SUPLAYER_FILE_INFO_VIDIO_OUT_T;

typedef struct mtSUPLAYER_FILE_INFO_AUDIO_OUT_S
{
    mt_u32                          stream_idx;
    char                                fourcc[5];
    mt_u32                          codec_id;
    mt_u32                          samplerate;
    mt_u32                          bitpersample;
    mt_u32                          channels;
    mt_u32                          bps;
    char                                lang[5];
    mt_u32                          subID;
    mt_u32                          duration;
    mt_u32                          track_cnt;
}SUPLAYER_FILE_INFO_AUDIO_OUT_T;

typedef struct mtSUPLAYER_FILE_INFO_OUT_S
{
    SOURCE_TYPE_OUT_T           src_type;
    mt_u64                          filesize;
    mt_u32                          starttime;
    mt_u32                          duration;
    mt_u32                          bps;
    SUPLAYER_FILE_INFO_VIDIO_OUT_T vid_info;
    SUPLAYER_FILE_INFO_AUDIO_OUT_T aud_info;
}SUPLAYER_FILE_INFO_OUT_T;

typedef struct mtSUPLAYER_CONTROL_AV_INFO_OUT_S
{
    mt_u32                          es_push_cnt;
    mt_u32                          es_hdl_true_cnt;
    mt_u32                          es_buf_full_cnt;
    mt_u32                          es_eof;
    mt_u32                          es_buf_size;
    mt_u32                          es_buf_free_size;
    mt_u32                          es_get_fail;
    mt_u32                          es_hungry;
    mt_u32                          ds_bytes;
    mt_u32                          orig_pts;
    mt_u32                          sys_pts;
    mt_u32                          play_pts;
    mt_u32                          codec_support;
    mt_u32                          id;
}SUPLAYER_CONTROL_AV_INFO_OUT_T;

typedef struct mtSUPLAYER_CONTROL_TPLAY_INFO_OUT_S
{
    mt_u32                          totol_send_frame;
    mt_u32                          sys_time_of_send_last_iframe;
    mt_u32                          total_iframe_scale;
    mt_u32                          base_systime_of_tplay;
    mt_u32                          base_iframe_pts_of_tplay;
    mt_u32                          num_of_discard_iframe;
    mt_u32                          num_of_send_pbframe;
}SUPLAYER_CONTROL_TPLAY_INFO_OUT_T;

typedef struct mtSUPLAYER_CONTROL_NETBUF_INFO_OUT_S
{
    mt_u32                          buf_status;
    mt_u32                          duration_of_format_buf;
    mt_u32                          size_of_format_buffer;
    mt_u32                          duration_of_player_buffer;
    mt_u32                          size_of_player_buffer;
    mt_u32                          download_finish;
    mt_u32                          reset_player;
    mt_u32                          report_first_frame_time;
    mt_u32                          set_buffer_under_run;
}SUPLAYER_CONTROL_NETBUF_NETBUF_OUT_T;

typedef struct mtSUPLAYER_CONTROL_INFO_OUT_S
{
    mt_u32                          send_aud;
    mt_u32                          send_vid;
    mt_u32                          send_subt;
    mt_u32                          dump_aes;
    mt_u32                          dump_ves;
    mt_u32                          dump_ses;
    mt_u32                          log_level;
    mt_u32                          chip_type;
    mt_u32                          fill_es_states;
    mt_u32                          aud_out_mode;
    mt_u32                          vdec_policy;
    char                          cur_status[8];
    mt_u32                          goto_status;
    mt_u32                          speed;
    mt_u32                          empty_cnt;
    mt_u32                          play_progress;
    mt_u32                          download_progress;
    mt_u32                          last_play_duration;
    mt_u32                          first_pts;
    mt_u32                          last_pts;
    mt_u32                          last_iframe_pts;
    mt_u32                          last_seek_pts;
    mt_u32                          call_seek_pts;
    mt_u32                          first_pts_after_seek;
    mt_u32                          call_seek_pos;
    mt_u32                          last_vid_pos;
    SUPLAYER_CONTROL_AV_INFO_OUT_T  vid;
    SUPLAYER_CONTROL_AV_INFO_OUT_T  aud;
    SUPLAYER_CONTROL_AV_INFO_OUT_T  subt;
    SUPLAYER_CONTROL_TPLAY_INFO_OUT_T tplay;
    SUPLAYER_CONTROL_NETBUF_NETBUF_OUT_T netbuf;
}SUPLAYER_CONTROL_INFO_OUT_T;

typedef struct mtSUPLAYER_FORMAT_INFO_OUT_S
{
    char                                path[4096];
    char                                demuxer[8];
    char                                support_format_name[8*20];
    char                                support_protocol_name[8*20];
    mt_u32                          buf_config_type;
    mt_u32                          max_buf_size;
    mt_u32                          time_out_of_network;
    mt_u32                          buf_start;
    mt_u32                          buf_enough;
    mt_u32                          buf_full;
    mt_u32                          hls_start_mode;
    mt_u32                          dst_subcode_type;
    mt_u32                          cookie;
    char                                user_agent[1024];
}SUPLAYER_FORMAT_INFO_OUT_T;

typedef struct mtSUPLAYER_ADAPTER_INFO_OUT_S
{
    char                                status[8];
    mt_u32                          bextra_avplay;
    MT_HANDLE                   avplay;
    MT_HANDLE                   aud_track_hdl;
    MT_HANDLE                   window_hdl;
    MT_HANDLE                   subo_hdl;
    mt_u32                          aud_mix_heigh;
    mt_u32                          aud_port;
    mt_u32                          display_id;
    mt_u32                          win_x;
    mt_u32                          win_y;
    mt_u32                          win_w;
    mt_u32                          win_h;
    mt_u32                          vdec_err_cover;
    mt_u32                          vid_pts_offset;
    mt_u32                          aud_pts_offset;
    mt_u32                          subt_pts_offset;
    mt_u32                          buse_ffmpeg_aud_codec;
    mt_s32                          speed;
    mt_u32                          bvid_start;
    mt_u32                          baud_start;
    mt_u32                          cur_pg_id;
    mt_u32                          cur_vid_id;
    mt_u32                          cur_aud_id;
    mt_u32                          cur_subt_id;
    mt_u32                          aud_frame_send_len;
    mt_u32                          eos;
}SUPLAYER_ADAPTER_INFO_OUT_T;

#if 1
typedef struct mtSUPLAYER_OUT_S
{
    SUPLAYER_CONTROL_INFO_OUT_T                 ctrl;
    SUPLAYER_FILE_INFO_OUT_T    file;
    SUPLAYER_FORMAT_INFO_OUT_T  format;
    SUPLAYER_ADAPTER_INFO_OUT_T adapter;
}SUPLAYER_OUT_S;
#endif
/* ------------ proc struct for app end -------------*/

mt_s32 MT_UNF_SUPLAYER_Init(mt_void);
mt_s32 MT_UNF_SUPLAYER_DeInit(mt_void);
mt_s32 MT_UNF_SUPLAYER_Create(/*const MT_UNF_SUPLAYER_ATTR_S *pstAvAttr*/void *pstAvAttr, mt_handle *phSuplayer);
mt_s32 MT_UNF_SUPLAYER_Destroy(mt_handle hSuplayer);
mt_s32 MT_UNF_SUPLAYER_Get_Log_Level(mt_handle hSuplayer);
void* MT_UNF_SUPLAYER_Get_Control(mt_handle hSuplayer);
mt_s32 MT_UNF_SUPLAYER_Set_File_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info);
mt_s32 MT_UNF_SUPLAYER_Set_Control_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info);
mt_s32 MT_UNF_SUPLAYER_Get_Push_Aud_Flg(mt_handle hSuplayer);
mt_s32 MT_UNF_SUPLAYER_Get_Push_Vid_Flg(mt_handle hSuplayer);
mt_s32 MT_UNF_SUPLAYER_Get_Push_Subt_Flg(mt_handle hSuplayer);
mt_s32 MT_UNF_SUPLAYER_Get_Dump_Aud_Flg(mt_handle hSuplayer);
mt_s32 MT_UNF_SUPLAYER_Get_Dump_Vid_Flg(mt_handle hSuplayer);
mt_s32 MT_UNF_SUPLAYER_Get_Dump_Subt_Flg(mt_handle hSuplayer);

mt_s32 MT_UNF_SUPLAYER_Set_Format_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info);
mt_s32 MT_UNF_SUPLAYER_Set_Adapter_Info(mt_handle hSuplayer, SUPLAYER_OUT_S *in_info);

/*********************************error  macro******************************************/
/*************************** Structure Definition ****************************/
/** \addtogroup      SUPLAYER */
/** @{ */  /** <!-- [SUPLAYER] */



/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif

#endif
