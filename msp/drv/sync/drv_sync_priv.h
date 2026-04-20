/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_SYNC_PRIV_H__
#define __DRV_SYNC_PRIV_H__

#include "mt_drv_log.h"
#include "mt_debug.h"
#include "drv_sync_ioctl.h"

#undef LOG_TAG
#define LOG_TAG                   "SYNC"
#define SYNC_LOG_LEVEL            KERN_ERR
#define SYNC_VIDEO_FRMAE_INFO    (0x00000001)
#define SYNC_AUDIO_FRMAE_INFO    (0x00000002)
#define SYNC_VIDEO_PTS_INFO      (0x00000004)
#define SYNC_AUDIO_PTS_INFO      (0x00000008)
#define SYNC_VIDEO_SYNCED_INFO   (0x00000010)
#define SYNC_VIDEO_PAUSE_INFO    (0x00000020)
#define SYNC_VIDEO_SKIP_INFO     (0x00000040)
#define SYNC_VIDEO_FREE_INFO     (0x00000080)
#define SYNC_AUDIO_SYNCED_INFO   (0x00000100)
#define SYNC_AUDIO_PAUSE_INFO    (0x00000200)
#define SYNC_AUDIO_SKIP_INFO     (0x00000400)
#define SYNC_AUDIO_FREE_INFO     (0x00000800)
#define SYNC_AUDIO_PUSH_INFO     (0x00001000)
#define SYNC_PCR_DEBUG_INFO      (0x00002000)
#define SYNC_STC_DEBUG_INFO      (0x00004000)
#define SYNC_AUDIO_DATA_INFO     (0x00008000)
#define SYNC_VIDEO_DATA_INFO     (0x00010000)
#define SYNC_AUDIO_POP_INFO      (0x00020000)

#define SYNC_LOGVF(fmt, ...)            do{if (priv->avsync_loglevel & SYNC_VIDEO_FRMAE_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGAF(fmt, ...)            do{if (priv->avsync_loglevel & SYNC_AUDIO_FRMAE_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGVP(fmt, ...)            do{if (priv->avsync_loglevel & SYNC_VIDEO_PTS_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGAP(fmt, ...)            do{if (priv->avsync_loglevel & SYNC_AUDIO_PTS_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGV_SYNCED(fmt, ...)      do{if (priv->avsync_loglevel & SYNC_VIDEO_SYNCED_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGV_PAUSE(fmt, ...)       do{if (priv->avsync_loglevel & SYNC_VIDEO_PAUSE_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGV_SKIP(fmt, ...)        do{if (priv->avsync_loglevel & SYNC_VIDEO_SKIP_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGV_FREE(fmt, ...)        do{if (priv->avsync_loglevel & SYNC_VIDEO_FREE_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGA_SYNCED(fmt, ...)      do{if (priv->avsync_loglevel & SYNC_AUDIO_SYNCED_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGA_PAUSE(fmt, ...)       do{if (priv->avsync_loglevel & SYNC_AUDIO_PAUSE_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGA_SKIP(fmt, ...)        do{if (priv->avsync_loglevel & SYNC_AUDIO_SKIP_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGA_FREE(fmt, ...)        do{if (priv->avsync_loglevel & SYNC_AUDIO_FREE_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGA_PUSH(fmt, ...)        do{if (priv->avsync_loglevel & SYNC_AUDIO_PUSH_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGP_DEBUG(fmt, ...)       do{if (priv->avsync_loglevel & SYNC_PCR_DEBUG_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGT_DEBUG(fmt, ...)       do{if (priv->avsync_loglevel & SYNC_STC_DEBUG_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGA_DATA(fmt, ...)        do{if (priv->avsync_loglevel & SYNC_AUDIO_DATA_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGV_DATA(fmt, ...)        do{if (priv->avsync_loglevel & SYNC_VIDEO_DATA_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOGA_POP(fmt, ...)         do{if (priv->avsync_loglevel & SYNC_AUDIO_POP_INFO){printk(SYNC_LOG_LEVEL"[%s %d]" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define SYNC_LOG_ALWAYS                 printk

#define AVSYNC_DEFAULT_SYNCED_NUMBER          (1)
#define AVSYNC_DEFAULT_UNSYNCED_NUMBER        (10)
#define MAX_PTS_FIFO_NODE_COUNT               (512)//(256)
#define MAX_AV_VID_DIFF_FRAME                 (120)
#define MAX_AV_AUD_DIFF_FRAME                 (130)
#define AVSYNC_MAX_AUD_READY_NUMBER           (10)
#define AVSYNC_MAX_VID_READY_NUMBER           (4)
#define AVSYNC_MAX_UNSYNCED_FRAME_NUMBER      (2)
#define AVSYNC_AUD_MIN_OUTPUT_FRAME           (5)
#define AVSYNC_AUD_FORCE_OUTPUT_FRAME         (5)
#define AVSYNC_VID_MAX_AVDIFF_TIME            (8)
#define AVSYNC_AUD_MAX_AVDIFF_TIME            (4)
#define AVSYNC_VID_MAX_AVDIFF_TIME_FP         (60)
#define AVSYNC_AUD_MAX_AVDIFF_TIME_FP         (60)
#define AVSYNC_SLOW_SYNC_PLAY_FRM_CNT         (1)
#define AVSYNC_SLOW_SYNC_PAUSE_FRM_CNT        (2)
//#define AVSYNC_DEFAULT_TIME_OUT_DISPLAY       (120)
#define AVSYNC_APTS_ADJUST_MS_MAX             (0x3fff)
#define AVSYNC_APTS_ADJUST_INCREASE_FLAG      (0x8000)
#define AVSYNC_AUD_PCM_UNDERFLOW_SIZE         (384)

#define AVSYNC_PCR_DIFF_TOO_SMALL         (10)
#define STC_CNT_PER_MS                    (45)
#define PCR_BAD_THRESHOLD                 (15)
#define APTS_BAD_THRESHOLD                (2)
#define AVSYNC_C0COUNTER_SLOPE            (22) //co counter freq:1M, pts freq:45000, 22 = 1M / 45000
#define AVSYNC_SLOPE_THRESHOLD            (3)
#define ALMOST_EQUAL_THRESHOLD            (STC_CNT_PER_MS * 80)
#define LITTLE_DIFFERENT_THRESHOLD        (ALMOST_EQUAL_THRESHOLD << 1)
#define DIFFERENT_THRESHOLD               (LITTLE_DIFFERENT_THRESHOLD << 1)
#define AVSYNC_PCR_SET_STC_DELAY          (200 * STC_CNT_PER_MS)
#define APTS_STC_DIFF_THRD_MAX            (100 * STC_CNT_PER_MS)
#define AVSYNC_SLOW_SYNC_PTS_DIFF         (400*45)
#define AVSYNC_STREAM_ROLLABCK_DIFF       (10000*45)
#define VID_PAUSE_CNT_BEFORE_AUD_PUSH     (40)
#define AVSYNC_AV_DIFF_FREE_THRESHOLD     (45*9*1000)
#define AVSYNC_AVPTS_DIFF_TOO_LARGE       (45*15*1000)
#define AVSYNC_DEFAULT_TIME_OUT_DISPLAY   (120)
#define AVSYNC_UNSYNCED_THRESHOLD         (10)
#define AVSYNC_AUDIO_TRACK_DIFF		      (500*45)

#define AVSYNC_DDP2997_VID_STEP0              (33*45)
#define AVSYNC_DDP2997_VID_STEP1              (34*45)
#define AVSYNC_DDP2500_VID_STEP               (40*45)
#define AVSYNC_DDP5000_VID_STEP               (20*45)
#define AVSYNC_DDP5994_VID_STEP0              (16*45)
#define AVSYNC_DDP5994_VID_STEP1              (17*45)

#define AVSYNC_PROG1_APID                     (64)
#define AVSYNC_PROG1_VPID                     (48)
#define AVSYNC_PROG2_APID                     (52)
#define AVSYNC_PROG2_VPID                     (49)
#define AVSYNC_MAX_64BIT_VALUE                (0xffffffffffffffff)
#define AVSYNC_MAX_32BIT_VALUE                (0xffffffff)

typedef enum
{
    AVSYNC_SRC_NIM = 0,
    AVSYNC_SRC_FILE_ES,
    AVSYNC_SRC_PVR,
    AVSYNC_SRC_FILE_TS,
    AVSYNC_SRC_IPTV,
}avsync_input_src_e;

typedef enum
{
    AVSYNC_REF_NONE_MODE = 0,
    AVSYNC_REF_AUDIO_MODE,
    AVSYNC_REF_VIDEO_MODE,
    AVSYNC_REF_PCR_MODE,
    AVSYNC_REF_AV_MODE,
}avsync_sync_mode_e;

enum COMPARE_RESULT
{
    ALMOST_EQUAL     = 0,
    LITTLE_LESS_THAN = 1,
    LITTLE_MORE_THAN = 2,
    LESS_THAN        = 3,
    MORE_THAN        = 4,
    MUCH_LESS_THAN   = 5,
    MUCH_MORE_THAN   = 6
};

enum STC_REFERENCE
{
    TM_PCR = 1,
    TM_APTS,
    TM_VPTS,
};

typedef struct avsync_play_info 
{
    mt_u32 a_pid;
    mt_u32 v_pid;
    mt_u32 a_type;
    mt_u32 v_type;
    mt_u32 tvformat;
	mt_u32 is_hdmi_connected;
}avsync_play_info_t;


typedef struct
{
    avsync_sync_mode_e avsync_sync_mode;
    avsync_input_src_e avsync_input_src;
    mt_u32 avsync_slow_sync;
    mt_u32 avsync_pause_threhold;
    mt_u32 avsync_skip_threhold;
}avsync_cfg_info_t;


typedef struct
{
    mt_u32 avsync_sync_mode;
    mt_u32 avsync_sync_flag;
    mt_u32 avsync_vpts;
    mt_u32 avsync_vfrm_id;
    mt_u32 avsync_vid_play_cnt;
    mt_u32 avsync_vid_skip_cnt;
    mt_u32 avsync_vid_pause_cnt;
    mt_u32 avsync_vid_free_cnt;
    mt_u32 avsync_apts;
    mt_u32 avsync_afrm_id;
    mt_u32 avsync_aud_play_cnt;
    mt_u32 avsync_aud_skip_cnt;
    mt_u32 avsync_aud_pause_cnt;
    mt_u32 avsync_aud_free_cnt;

}avsync_sta_info_t;


typedef enum avsync_ddp_prog 
{
    AVSYNC_DD_NONE_PG = 0,		//0
    AVSYNC_MPEG2_DD_2500_PG1,	//1
    AVSYNC_MPEG2_DD_2500_PG2,	//2
    AVSYNC_MPEG2_DDP_2500_PG1,	//3
    AVSYNC_MPEG2_DDP_2500_PG2,	//4
    AVSYNC_H264_DD_2500_PG1,	//5
    AVSYNC_H264_DD_2500_PG2,	//6
    AVSYNC_H264_DDP_2500_PG1,	//7
    AVSYNC_H264_DDP_2500_PG2,	//8
    AVSYNC_H264_DD_2997_PG1,	//9
    AVSYNC_H264_DD_2997_PG2,	//10
    AVSYNC_H264_DDP_2997_PG1,	//11
    AVSYNC_H264_DDP_2997_PG2,	//12
    
    AVSYNC_MPEG2_DDP_2500_ATMOS,//13
    
    AVSYNC_H264_DDP_2500_ATMOS,	//14
    AVSYNC_H264_DDP_5000_ATMOS,	//15
    AVSYNC_H264_DDP_2997_ATMOS,	//16

	AVSYNC_H265_DDP_2500_ATMOS,	//17
	AVSYNC_H265_DDP_5000_ATMOS,	//18
    AVSYNC_H265_DDP_5994_ATMOS,	//19
    AVSYNC_H265_DDP_2997_ATMOS,	//20
    
    AVSYNC_H264_AC4_2500_ATMOS,	//21
    AVSYNC_H264_AC4_5000_ATMOS, //22
    AVSYNC_H264_AC4_2997_ATMOS,	//23    
    
    AVSYNC_H265_AC4_2500_ATMOS, //24
    AVSYNC_H265_AC4_5000_ATMOS,	//25
    AVSYNC_H265_AC4_2997_ATMOS, //26
    AVSYNC_H265_AC4_5994_ATMOS,	//27    
} avsync_ddp_prog_e;

typedef enum avsync_aud_out {
    AVSYNC_AUD_OUT_PCM,
    AVSYNC_AUD_OUT_SPDIF,
	AVSYNC_AUD_OUT_MAT,
    AVSYNC_AUD_OUT_BUTT,//this is a invalid enum 
}avsync_aud_out_e;

typedef enum avsync_av_dev_sta {
    DEV_DEINIT = 0,
    DEV_INIT,
}avsync_av_dev_sta_e;

typedef struct pcr_info_t
{
    mt_u64 latest_tick;
    mt_u32 latest_pcr;
    mt_u64 last_reliable_tick;
    mt_u32 last_reliable_pcr;
    mt_u32 total_pcr_cnt;
    mt_u32 pcr_unreliable_total;
    mt_u32 pcr_probe_start_tick;
    mt_u32 last_pcr;
    mt_u8 pcr_reliable;
    mt_u8 pcr_got;
    mt_u8 bpcr_loop;
    mt_u8 check_pts_pcr_cnt;

}pcr_info_t;

struct a_stat_t
{
    mt_u32 total_skip_cnt;
    mt_u32 total_pause_cnt;
    mt_u32 total_free_cnt;
    mt_u32 total_play_cnt;
    mt_u32 total_request_cnt;
    mt_u32 aes_full_cnt;
    mt_u32 aes_empty_cnt;
    mt_u32 switch_audio_reason;
    mt_u32 aud_free_reason;
    mt_u32 aud_pause_reason;
    mt_u32 aud_skip_reason;
};

struct v_stat_t
{
    mt_u32 conti_skip_cnt;
    mt_u32 conti_pause_cnt;
    mt_u32 conti_free_cnt;
    mt_u32 total_free_cnt;
    mt_u32 total_play_cnt;
    mt_u32 total_skip_cnt;
    mt_u32 total_pause_cnt;
    mt_u32 total_request_cnt;
    mt_u32 vbv_full_cnt;
    mt_u32 vbv_empty_cnt;
    mt_u8 hold_cnt_before_audio_output;
    mt_u32 switch_video_reason;
    mt_u32 vid_freerun_reason;
    mt_u32 vid_pause_reason;
    mt_u32 vid_skip_reason;
    mt_u32 vid_free_forever;
};

struct vsync_t
{
    struct v_stat_t stat;
    mt_u32 vpts;
    mt_u32 latest_parsed_vpts;
    mt_u32 last_vpts;
    mt_u32 last_vpts_id;
};

struct async_t
{
    struct a_stat_t stat;
    mt_u32 latest_parsed_apts;
    mt_u64 first_parsed_tick;
    mt_u64 latest_parsed_tick;
    mt_u32 last_apts;
    mt_u32 last_apts_id;
    
};

struct pts_node_t
{
    mt_u32 pts;
    mt_u32 pts_id;
    mt_u32 pts_step;
};

struct pts_fifo_t
{
    mt_u32 fifo_init;
    mt_u32 fifo_len;
    mt_u32 fifo_cnt;
    mt_u32 fifo_wr;
    mt_u32 fifo_rd;
    struct pts_node_t fifo_buf[MAX_PTS_FIFO_NODE_COUNT];
    spinlock_t fifo_spin_lock;
};

typedef struct
{
    unsigned int ridx;
    unsigned int widx;
    unsigned int total;
    unsigned char *pdata;
}avsync_recorder_queue;

typedef enum
{
    /*!
    The state of video layer is handled by user
    */
    AVSYNC_UNBLANK_USER,
    /*!
    The video layer will be displayed When AV is sync
    */
    AVSYNC_UNBLANK_SYNC,
    /*!
    The video layer will be displayed when video decoding is ready for display.
    */
    AVSYNC_UNBLANK_STABLE,
    /*!
    The video layer will be displayed when video get first I image.
    */
    AVSYNC_UNBLANK_FAST
}avsync_screen_open_e;

typedef struct
{
    avsync_play_info_t av_play_info;
    avsync_cfg_info_t avsync_cfg_info;
    avsync_sta_info_t avsync_sta_info;
    mt_u32 avsync_do_aud_track;		//1:audio track or change AD
    mt_u32 avsync_do_trick_seek;
    mt_u32 avsync_aud_rollback;
    SYNC_S  *p_avsync_psync;
    mt_u32 do_ddp_verf;
    avsync_ddp_prog_e ddp_prog;
    mt_u32 avsync_pause_threhold;
    mt_u32 avsync_skip_threhold;
    mt_u32 av_sync_flag;
    mt_u32 ddp_atmos;
    avsync_aud_out_e avsync_aud_output;
    avsync_av_dev_sta_e aud_dev_sta;
    avsync_av_dev_sta_e vid_dev_sta;
    mt_u32 vid_pause_cnt_before_aud_push;
    avsync_sync_mode_e avsync_sync_mode;
    MT_SYNC_DATA_SOURCE_E data_source_mode;
    mt_u32 vid_sync_sys_time_ms;
    mt_u32 vid_dolby_ply_cnt;
    mt_u32 dolby_sync_done;
    mt_u32 normal_sync_done;
	mt_u32 normal_vfirst;
    mt_u32 aud_sync_sys_time_ms;
    mt_u32 aud_frame_sample_num;
    mt_u32 dolby_avsync;
    mt_u32 avsync_vloop_cnt;
    pcr_info_t pcr_info;
    mt_u32 last_pcr;
    mt_u32 init_stc;
    mt_u32 stream_total_time;
    mt_u32 pcr_bad_cnt;
    mt_u32 reliable_pcr;
    struct async_t async;
    struct vsync_t vsync;
    mt_u32 first_vpts;
    mt_u32 cur_vpts;
    mt_u32 cur_vpts_id;
    mt_u32 cur_vpts_valid;
    mt_u32 vpts_step;
    mt_u32 vstabled_flag;
    mt_u32 vstabled_cnt;
    mt_u32 v_synced_flag;
    mt_u32 v_unsynced_cnt;
    mt_u32 end_of_stream_flag;
    mt_u32 first_apts;
    mt_u32 cur_apts;
    mt_u32 cur_play_apts;
    mt_u32 cur_apts_id;
    mt_u32 apts_step;
    mt_u32 astabled_flag;
    mt_u32 astabled_cnt;
    mt_u32 a_synced_flag;
    mt_u32 a_unsynced_cnt;
    mt_u32 cur_stc;
    mt_u32 cur_pcr;
    mt_u32 calc_pcr_time_start;
    mt_u32 pcr_reduce_cnt;
    mt_u32 pcr_fast_offset;
    mt_u32 fast_mode_in_adjust;
    mt_u32 hold_video_num_before_audio_output;
    enum STC_REFERENCE stc_reference;
    mt_u32 apts_bad_cnt;
    mt_u32 apts_total_cnt;
    mt_u32 apts_total_cnt_before_sw_sync_mode;
    mt_u32 stc_offset;
    mt_u32 av_synced_flag;
    mt_u32 aud_skip_on;
    mt_u32 aud_pause_on;
    mt_u32 vid_skip_on;
    mt_u32 vid_pause_on;
    mt_u32 disp_flag;
    struct  avsync_vsync_info_t vsync_info;
    struct  avsync_async_info_t async_info;
    struct pts_fifo_t apts_fifo;
    struct pts_node_t isr_pop;
    mt_u32 apts_fifo_push_cnt;
    mt_u32 apts_fifo_pop_cnt;
    mt_u32 apts_fifo_push_pts;
    mt_u32 aud_push_cnt;
    mt_u32 aud_play_cnt;
    mt_u32 aud_free_cnt;
    mt_u32 aud_pause_cnt;
    mt_u32 aud_skip_cnt;
    mt_u32 cur_pcm_time;
    mt_u32 aud_underflow_size;
    mt_u32 aud_underflow_cnt;
    mt_u32 vid_push_cnt;
    mt_u32 vid_play_cnt;
    mt_u32 vid_free_cnt;
    mt_u32 vid_pause_cnt;
    mt_u32 vid_skip_cnt;
    mt_u32 slow_sync_en;
    mt_u32 slow_sync_cnt;
    mt_u32 av_play_flag;
    mt_u32 vsync_diff;
    mt_u32 avsync_loglevel;
    mt_u32 atoms_looped;
	mt_u32 dolby_adjust;//If it has synced. 0:don't adjust according to pts. 1:adjust according to pts
	mt_u32 audio_bypass_last_apts;
	mt_u32 audio_bypass_apts_adjust;  	//bit15: 0-decrease, 1-increase; bit14-bit0:the value
	mt_u32 audio_track_set_av_mode_flag;//if audio track need to set av mode, use this flag to ensure only set once.
	u32 pts_offset;
}avsync_priv_t;

#endif


