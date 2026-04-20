/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_SUPLAYER_IOCTL_H__
#define __DRV_SUPLAYER_IOCTL_H__

#include "mt_type.h"
#include "mt_mpi_mem.h"
#include "mt_mpi_stat.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif

//#define  SUPLAYER_VID_THREAD

#define  SUPLAYER_MAX_NUM                 16

/* ++++++++++++ proc struct for drv start +++++++++++++*/
/* The following structs should be the same with that in mt_unf_suplayer.h which named */
/* xx_OUT_T */
typedef enum mtSOURCE_TYPE_E
{
    SOURCE_TYPE_LOCAL = 0,

    SOURCE_TYPE_NETWORK,

    SOURCE_TYPE__BUTT
}SOURCE_TYPE_T;

typedef struct mtSUPLAYER_FILE_INFO_VIDIO_S
{
    mt_u32                          stream_idx;
    char                                fourcc[5];
    mt_u32                          codec_id;
    mt_u32                          width;
    mt_u32                          height;
    mt_u32                          fps;
    mt_u32                          bps;
    mt_u32                          duration;
}SUPLAYER_FILE_INFO_VIDIO_T;

typedef struct mtSUPLAYER_FILE_INFO_AUDIO_S
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
}SUPLAYER_FILE_INFO_AUDIO_T;

typedef struct mtSUPLAYER_CONTROL_AV_INFO_S
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
}SUPLAYER_CONTROL_AV_INFO_T;

typedef struct mtSUPLAYER_CONTROL_TPLAY_INFO_S
{
    mt_u32                          totol_send_frame;
    mt_u32                          sys_time_of_send_last_iframe;
    mt_u32                          total_iframe_scale;
    mt_u32                          base_systime_of_tplay;
    mt_u32                          base_iframe_pts_of_tplay;
    mt_u32                          num_of_discard_iframe;
    mt_u32                          num_of_send_pbframe;
}SUPLAYER_CONTROL_TPLAY_INFO_T;

typedef struct mtSUPLAYER_CONTROL_NETBUF_INFO_S
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
}SUPLAYER_CONTROL_NETBUF_NETBUF_T;

typedef struct mtSUPLAYER_CONTROL_INFO_S
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
    mt_s32                          speed;
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
    SUPLAYER_CONTROL_AV_INFO_T  vid;
    SUPLAYER_CONTROL_AV_INFO_T  aud;
    SUPLAYER_CONTROL_AV_INFO_T  subt;
    SUPLAYER_CONTROL_TPLAY_INFO_T tplay;
    SUPLAYER_CONTROL_NETBUF_NETBUF_T netbuf;
}SUPLAYER_CONTROL_INFO_T;

typedef struct mtSUPLAYER_FILE_INFO_S
{
    SOURCE_TYPE_T           src_type;
    mt_u64                          filesize;
    mt_u32                          starttime;
    mt_u32                          duration;
    mt_u32                          bps;
    SUPLAYER_FILE_INFO_VIDIO_T vid_info;
    SUPLAYER_FILE_INFO_AUDIO_T aud_info;
}SUPLAYER_FILE_INFO_T;

typedef struct mtSUPLAYER_FORMAT_INFO_S
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
}SUPLAYER_FORMAT_INFO_T;

typedef struct mtSUPLAYER_ADAPTER_INFO_S
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
}SUPLAYER_ADAPTER_INFO_T;

#if 1
typedef struct mtSUPLAYER_S
{
    mt_handle                       hSuplayer;
    SUPLAYER_CONTROL_INFO_T                 ctrl;
    SUPLAYER_FILE_INFO_T    file;
    SUPLAYER_FORMAT_INFO_T  format;
    SUPLAYER_ADAPTER_INFO_T adapter;
/* Bug: for size of this structure, kernel not match with msp api.
 * move to tail, and add some reserved fileds.
 */
#ifndef __KERNEL__
    pthread_mutex_t                 *pSuplayerThreadMutex;     /*mutex for data safety use*/
    pthread_mutex_t                 *pSuplayerVidThreadMutex;     /*mutex for data safety use*/
    pthread_mutex_t                 *pSuplayerMutex;            /* mutex for interface safety use */
#else
	/*
	 3*(sizeof(pthread_t)=4)
	 + (sizeof(pthread_attr_t)=36)
	 + 3*(sizeof(pthread_mutex_t)=24)
	*/
	mt_u32 reserved[(3*24)/4];
#endif

}SUPLAYER_S;
#endif
/* ------------ proc struct for drv end--------------*/
#pragma pack()

typedef struct mtSUPLAYER_CREATE_S
{
    mt_u32     SuplayerId;
    phys_addr_t     SuplayerPhyAddr;
}SUPLAYER_CREATE_S;

typedef struct mtSUPLAYER_USR_ADDR_S
{
    mt_u32     SuplayerId;
    ulong     SuplayerUsrAddr;    /* SUPLAYER address in user model */
}SUPLAYER_USR_ADDR_S;

typedef struct mtSUPLAYER_INFO_S
{
    SUPLAYER_S   *pSuplayer;         /* SUPLAYER pointer in kernel model */
    phys_addr_t     SuplayerPhyAddr;    /* SUPLAYER physical address */
    ulong     File;             /*avplay file handle*//* CNcomment: SUPLAYER���ڽ��̾�� */
    ulong     SuplayerUsrAddr;    /* SUPLAYER address in user model */
}SUPLAYER_INFO_S;

typedef struct mtSUPLAYER_GLOBAL_STATE_S
{
    SUPLAYER_INFO_S  SuplayerInfo[SUPLAYER_MAX_NUM];
    mt_u32         SuplayerCount;
}SUPLAYER_GLOBAL_STATE_S;


typedef enum mtIOC_SUPLAYER_E
{
    IOC_SUPLAYER_GET_NUM = 0,

    IOC_SUPLAYER_CREATE,
    IOC_SUPLAYER_DESTROY,

    IOC_SUPLAYER_CHECK_ID,
    IOC_SUPLAYER_SET_USRADDR,
    IOC_SUPLAYER_CHECK_NUM,
    IOC_SUPLAYER_SET_CPUFREQ,

    IOC_SUPLAYER_SET_BUTT
}IOC_SUPLAYER_E;


#define CMD_SUPLAYER_CREATE            _IOWR(MT_ID_SUPLAYER, IOC_SUPLAYER_CREATE, SUPLAYER_CREATE_S)
#define CMD_SUPLAYER_DESTROY           _IOW(MT_ID_SUPLAYER, IOC_SUPLAYER_DESTROY, mt_u32)

#define CMD_SUPLAYER_CHECK_ID       _IOWR(MT_ID_SUPLAYER, IOC_SUPLAYER_CHECK_ID, SUPLAYER_USR_ADDR_S)
#define CMD_SUPLAYER_SET_USRADDR    _IOW(MT_ID_SUPLAYER, IOC_SUPLAYER_SET_USRADDR, SUPLAYER_USR_ADDR_S)
#define CMD_SUPLAYER_CHECK_NUM      _IOWR(MT_ID_SUPLAYER, IOC_SUPLAYER_CHECK_NUM, mt_u32)
#define CMD_SUPLAYER_SET_CPUFREQ    _IO(MT_ID_SUPLAYER, IOC_SUPLAYER_SET_CPUFREQ)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


