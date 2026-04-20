/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _AVPLAYAL_H_
#define _AVPLAYAL_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mt_type.h"

/* decode mode */
typedef enum {
    FLUSH_NO = 0,
    FLUSH_START,
    FLUSH_END,
    FLUSH_MAX /* discard B before get first P */
} FLUSH_OUTPUT_E;

//added by zhouxiang for audio/video raw synchronize
#define PTS_S64_MAX 0X7FFFFFFFFFFFFFFF
//added by doreenyu for ts streams starting with non zero pts (24mbps.ts, ch9.ts)
#define  FIRST_NPKTS_NO_SYNC  5

typedef enum OMX_TRACK_INFO {
    OMX_TrackNone = 0,
    OMX_OnlyAudio = 1,
    OMX_OnlyVideo = 2,
    OMX_BothAudioANDVideo = 3
} OMX_TRACK_INFO;

// GST_SECOND: (value 1000000000) (type GstClockTime)
#define PTS_DIFF_THREHOLD_MIN		3000000000
#define PTS_DIFF_THREHOLD_MAX		8000000000

typedef struct OMX_GLOBAL_PTS
{
    uint8_t  is_started;
    int64_t video_pts;
    int64_t audio_pts;
    uint32_t pkt_cnt;
    int64_t cur_playing_pts;
    OMX_TRACK_INFO type;
}OMX_GLOBAL_PTS;

//----------------------------------------------------------------------------//
typedef struct
{
    //uint32_t phyaddr;
    uint32_t buffer_len;
    uint32_t data_offset;
    uint32_t data_len;
    uint32_t flags;
    int64_t timestamp;
    //eBUFFER_TYPE buffer_type;
    //ePORT_DIR dir;
    void *bufferaddr;
    //void *client_data; //OMX_BUFFERHEADERTYPE *
    //OMXVDEC_FRAME_S out_frame;
} OMXVDEC_BUF_DESC;

static OMX_GLOBAL_PTS g_pts_info;

void mon_sync_init(OMX_TRACK_INFO type);
uint32_t mon_handle_video(int64_t v_pts);
void mon_update_vpts(int64_t v_pts);
uint32_t mon_handle_audio(int64_t a_pts);
void mon_update_apts(int64_t a_pts);
void mon_update_playing_pts(int64_t cur_pts);
int64_t mon_get_playing_pts(void);
void mon_sync_deinit(OMX_TRACK_INFO type);

typedef void* AUDIO_DECODER_HANDLE;
typedef struct
{
    MT_HANDLE avplay_handle;
    MT_HANDLE track_handle;
    pthread_mutex_t *pVMutex;
    mt_u32 adec_type;
    mt_u32 is_big_endian;
    mt_u32 bits;
    mt_u32 channels;
    mt_u32 sample_rate;
    MT_BOOL flag_sound_init;
    MT_BOOL eos_flag;
    mt_u32 adec_ffmpeg_type;
} AUD_DEC_HANDLE_T;

typedef struct {
	unsigned int flags;		// buffer flags, such as AUDIO_BUFFER_FLAG_EOS
	unsigned long counter;	// buffer counter, 0 if unknown
	void *addr;				// buffer address
	size_t offset;			// available data offset
	size_t len;				// data length
	size_t size;			// buffser size
	int64_t pts;			// present time stamp in us, -1 if unknown
	//TODO...
	// add your parameters here:
} audio_decoder_buffer_t;


MT_S32 audio_decoder_open0(MT_HANDLE *pAvplay, MT_HANDLE *pTrack);
AUDIO_DECODER_HANDLE audio_decoder_open(void);
AUDIO_DECODER_HANDLE audio_decoder_get_handle(void);
MT_S32 audio_decoder_close(AUDIO_DECODER_HANDLE handle);
MT_S32 audio_decoder_start(AUDIO_DECODER_HANDLE handle);
MT_S32 audio_decoder_stop(AUDIO_DECODER_HANDLE handle);
MT_S32 audio_decoder_pause(AUDIO_DECODER_HANDLE handle);
MT_S32 audio_decoder_resume(AUDIO_DECODER_HANDLE handle);
MT_S32 audio_decoder_flush(AUDIO_DECODER_HANDLE handle, MT_S32 flush_dir);
void audio_get_pts(AUDIO_DECODER_HANDLE handle, int64_t *out_pts);
MT_S32 audio_decoder_push_es(AUDIO_DECODER_HANDLE handle, void *p1);
//MT_S32 audio_decoder_get_pcm(AUDIO_DECODER_HANDLE handle, MT_UNF_AO_FRAMEINFO_S *p_ao_fram);
//MT_S32 audio_decoder_release_pcm(AUDIO_DECODER_HANDLE handle, MT_UNF_AO_FRAMEINFO_S *p_ao_fram);
MT_S32 audio_decoder_check_buffer_empty(AUDIO_DECODER_HANDLE handle, MT_BOOL *p_empty);
MT_S32 audio_decoder_reset_buffer(AUDIO_DECODER_HANDLE handle, mt_s32 reset_flag);
MT_S32 audio_decoder_flush_stop(void);
mt_s32 MT_AVPlay_SetAdecAttr(AUD_DEC_HANDLE_T *p_adec_handle, 
			mt_u32 enADecType, MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly);
MT_S32 audio_decoder_get_pcm(AUDIO_DECODER_HANDLE handle, 
					MT_UNF_AO_FRAMEINFO_S *p_ao_fram);
MT_S32 audio_decoder_release_pcm(AUDIO_DECODER_HANDLE handle, 
								MT_UNF_AO_FRAMEINFO_S *p_ao_fram);


typedef struct {
    uint32_t       v_codec_id;
    uint32_t       frame_rate;
    MT_BOOL   has_audio;
    MT_BOOL   is_tunnel;
    MT_HANDLE avplay_handle;
    MT_HANDLE video_win;
    MT_HANDLE video_vir_win;
    uint32_t       vdecType;
} mt_video_dec_t;


//MT_S32 MT_AVPlay_SetVdecAttr(MT_HANDLE hAvplay,MT_UNF_VCODEC_TYPE_E enType,MT_UNF_VCODEC_MODE_E enMode);
MT_S32 video_get_outbuf(OMXVDEC_BUF_DESC *pstBuf);
//MT_S32 video_decoder_open(MT_HANDLE *pHandle, MT_UNF_VCODEC_TYPE_E VdecType, MT_BOOL is_tunnel, MT_BOOL has_audio);//by video_decoder_init
MT_S32 video_decoder_close(void *p1);
MT_S32 video_decoder_stop(void);
MT_S32 video_decoder_set_framerate(uint32_t frame_rate);
//MT_S32 video_decoder_start(MT_HANDLE handle, MT_UNF_VCODEC_TYPE_E VdecType);//by video_decoder_init
MT_S32 video_decoder_pause(void);
MT_S32 video_decoder_resume(void);
MT_S32 video_decoder_flush(void);
MT_S32 video_decoder_check_buffer_empty(MT_BOOL *p_empty);
MT_S32 video_decoder_push_es(OMXVDEC_BUF_DESC *puser_buf);
MT_S32 video_decoder_set_avsync_none(void);
MT_S32 video_decoder_init(void *p1);
void video_get_pts(int64_t *out_pts);
void* video_decoder_get_handle(void);
MT_S32 video_decoder_check_waterlevel(MT_S32 *audio_percent, MT_S32 *video_percent);
MT_S32 video_decoder_get_es_buf_space(uint32_t *video_size, uint32_t *video_total_size, uint32_t *audio_size, uint32_t *audio_total_size);
MT_S32 video_decoder_flush_stop(void);
MT_S32 video_decoder_set_trick_mode(int playrate);
MT_S32 video_decoder_start(MT_HANDLE handle, MT_UNF_VCODEC_TYPE_E VdecType);
MT_S32 MTADP_Disp_Init_decoder(MT_UNF_ENC_FMT_E enFormat);
MT_S32 MT_AVPlay_SetVdecAttr(MT_HANDLE hAvplay,
					MT_UNF_VCODEC_TYPE_E enType,MT_UNF_VCODEC_MODE_E enMode);
MT_S32 video_decoder_open(MT_HANDLE *pHandle, 
		MT_UNF_VCODEC_TYPE_E VdecType, MT_BOOL is_tunnel, MT_BOOL has_audio);

int displaysetting_init_disp(void);
void check_task_finish_av_instance(void *args);


typedef int (*gst_avplayer_eventcb_fun_t)(void *sink, int seqnum, int event);
typedef struct {
    void *vsink;
    int vseqnum;
    gst_avplayer_eventcb_fun_t v_event_cb;
    void *asink;
    int aseqnum;
    gst_avplayer_eventcb_fun_t a_event_cb;
} mt_check_task_t;
//codec_type video is 1, audio is 2
MT_S32 avplayer_check_task_finish(int codec_type, void* sink, int seqnum, gst_avplayer_eventcb_fun_t event_cb);

MT_S32 avplayal_open(MT_HANDLE *pHandle);
MT_S32 avplayal_close(MT_HANDLE handle);

#endif
