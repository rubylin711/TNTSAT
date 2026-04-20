/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

// system
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define MODULE_TAG "FP"
#include "mutil.h"
#include "mlog.h"
#include "drv_adp.h"
#include "mtos_mem.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_task.h"
#include "mt_common.h"

#include "fifo_kw.h"
#include "libmpdemux/stheader.h"
#include "mt_unf_avplay.h"

#include "libmpdemux/demuxer.h"
#include "stream/stream.h"

#include "ts_sequence.h"
#include "file_playback_sequence.h"
#include "demux_mp.h"
#include "file_seq_internal.h"
#include "pts_list.h"
#include "lib_memp.h"
#include <pthread.h>
#include "http_file/download_api.h"
#include "file_seq_misc.h"
#include "libavformat/avio.h"
#if defined(CFG_SMART_HTTP_PTOTOCOL)
#include "libavformat/smart_http.h"
#endif

#ifdef MT_DRM_SUPPORT
#include "MTDrmApi.h"
#endif

#include "file_fake_mpd.h"

int g_player_buffer_size_mode = 0;  //davis ucos 0:64M config 1:128M config
#ifndef CONFIG_MT_CHIP_SYMPHONY4
extern int save_avi_memory_set_value(int new_info_num, int new_max_seg_num);
#endif

#include "mtos_misc.h"
static int x_force_stop_func(void *p_handle);

os_sem_t g_mplayer_handle_sem_lock;

extern double first_vpts;

extern  int audio_id, video_id, dvdsub_id;
extern int revise_fps;
int h264_frm_only_flag = 1;
int mp_field_pic_flag = 0;
int io_stream_buffer_size = 256 * 1024;
int io_isnetworkstream = 0;
static  void *g_player_handle = NULL;

//use outside ffmpeg,disable g_hls_playmode
int g_hls_playmode = 0;
int g_dash_playmode = 0;//0 is not dash , 1 is vod, 2 is live, 3 is main

static unsigned long int g_last_get_vpts = 0;
extern Node *list_I_vpts;

#ifdef CONFIG_MT_FILEPLAY_NONETWORK
static void replace_youtube_real_play_path(char * p_play_path) {
    MLOGD("p_play_path : %s\n", p_play_path);
    int ret = -1;
    request_url_t request_url;
    memset(&request_url, 0, sizeof(request_url_t));
    if (strstr(p_play_path, "youtu.be")) {
        request_url_t request_url;
        memset(&request_url, 0, sizeof(request_url_t));
        char tmppath[128];
        char *tmpstart = NULL;
        memset(tmppath, 0, 128);
        tmpstart = strstr(p_play_path, "youtu.be/");
        tmpstart += strlen("youtu.be/");
        sprintf(tmppath, "https://www.youtube.com/watch?v=%s", tmpstart);
        MLOGD("tmp path : %s\n", tmppath);
        ret = Nw_Request_Website_DownloadURL(WEBSITE_YOUTUBE, tmppath, (QEQUEST_VIDEO_SD + QEQUEST_VIDEO_HD), &request_url, 15);
    } else if (strstr(p_play_path, "youtube.com")) {
        ret = Nw_Request_Website_DownloadURL(WEBSITE_YOUTUBE, p_play_path, (QEQUEST_VIDEO_SD + QEQUEST_VIDEO_HD), &request_url, 15);
    }

    if (ret == 0) {
        char *downlaod_url = NULL;
        if (strlen(request_url.playUrlArray[1]) > 0) {
            downlaod_url = request_url.playUrlArray[1];
        } else if (strlen(request_url.playUrlArray[2]) > 0) {
            downlaod_url = request_url.playUrlArray[2];
        } else if (strlen(request_url.playUrlArray[0]) > 0) {
            downlaod_url = request_url.playUrlArray[0];
        }

        strcpy(p_play_path, downlaod_url);
        MLOGD("youtube choose h264EncUrlArray: [%s].\n", p_play_path);
    }
}
#endif

unsigned int backup_avsync_status;
int g_is_live_broadcast = 0;
char *mediaplay_http_header_useragent = NULL;
static char *g_mov_desc_key = NULL;
static char *g_mov_desc_key_len = NULL;

static void x_get_subt_info_func(void *p_handle, void **subt_info);
static  int x_pause_func(void *p_handle);
static void x_unloadmedia_func(void *p_handle);

static MT_BOOL x_check_aud_codec_type(void *p_handle)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    int audio_codec = HA_AUDIO_ID_INVALID ;
    MT_BOOL isSupportAudioType = MT_FALSE;
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    MLOGD("[%s] start start ...\n", __func__);
    MLOGD("[%s] -----para in:  p_ds_audio=%x, p_ds_video=%x\n",
          __func__, p_file_seq->p_ds_audio, p_file_seq->p_ds_video);

    p_file_seq->audio_bps = ds_get_audio_bps(p_file_seq->p_ds_audio);
    MLOGD("[%s] ----- audio_bps=%d\n", __func__, p_file_seq->audio_bps);
    (void) ds_get_audio_codec_info(p_file_seq->p_ds_audio, &p_file_seq->m_audio_codec_type,
            &pbi->audio.pcm_be, &pbi->audio.codec_id, &p_file_seq->audio_pid, p_file_seq->audio_output_mode);
    MLOGD("[%s] ----- m_audio_codec_type=%d, audio_pid=%d\n", \
          __func__, p_file_seq->m_audio_codec_type, p_file_seq->audio_pid);

    audio_codec = p_file_seq->m_audio_codec_type;
    isSupportAudioType = fp_is_support_audio_codec(p_file_seq, audio_codec);
    if (MT_TRUE == isSupportAudioType) {
        p_file_seq->checkAvTypeOK = MT_TRUE;
        p_file_seq->is_av_codec_support = 1;
        MLOGD("[%s] -----end, returen MT_TRUE  \n", __func__);
        return MT_TRUE;
    } else {
        MLOGE("[%s] Fail to check aud codec type[0x%x] !!!\n", __func__, audio_codec);
    }

    MLOGI("[%s] -----end, return MT_FALSE! \n", __func__);
    return MT_FALSE;
}

FILE_SEQ_T *x_get_cur_instance(void) {
    return g_player_handle;
}

FILE_SEQ_T *file_seq_get_instance(void) {
    return g_player_handle;
}

MT_BOOL is_file_seq_exit(void) {
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    if(!p_file_seq) {
        return MT_FALSE;
    }

    if (p_file_seq && p_file_seq->m_play_state == FILE_SEQ_EXIT &&
        (p_file_seq->is_task_alive || p_file_seq->is_load_task_alive)) {
        return MT_TRUE;
    } else {
        return MT_FALSE;
    }
}

static MT_BOOL check_file_seq_exit(FILE_SEQ_T *p_file_seq)
{
    if (!p_file_seq) {
        return MT_FALSE;
    }

    if (p_file_seq->m_play_state == FILE_SEQ_EXIT &&
        (p_file_seq->is_task_alive || p_file_seq->is_load_task_alive)) {
        return MT_TRUE;
    } else {
        FILE_PLAYBACK_USER_INTERRUPT_CB_T *user_cb = &p_file_seq->user_int_cb;
        if (user_cb->callback && user_cb->callback(user_cb->callback_ctx)) {
            return MT_TRUE;
        }
    }
    return MT_FALSE;
}

int  fp_is_timeshift_file(void) {
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    if(!p_file_seq) {
        return MT_FAILURE;
    }
    return ((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal))->is_timeshift ;
}

MT_BOOL  fp_is_loadmedia_state(void) {
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();

    if(!p_file_seq) {
        return MT_FALSE;
    }

    if (p_file_seq && p_file_seq->m_play_state == FILE_SEQ_LOADMEDIA) {
        return MT_TRUE;
    } else {
        return MT_FALSE;
    }
}

/* *dump video */
static void x_mp_dump(void)
{
    MLOGD("[%s] [%s] [%d] ==start start\n", __FILE__, __func__, __LINE__);
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    demuxer_t *demuxer;
    stream_t *stream;
    int file_format;

    FILE *f;
    demux_stream_t *ds = NULL;
    demux_stream_t *ds2 = NULL;
    int in_size = 0;
    unsigned char *start;
    unsigned char *start2;

    if(!p_file_seq) {
        return;
    }
    p_file_seq->mem_init(p_file_seq, (void *)p_file_seq->file_seq_mem_start, p_file_seq->file_seq_mem_size);
    stream  = open_stream(p_file_seq->m_path[0], 0, &file_format);
    if (stream == NULL) {
        return;
    }

    demuxer = demux_mp_open(stream, file_format, audio_id, video_id, dvdsub_id, p_file_seq->m_path[0]);
    if (demuxer == NULL) {
        return;
    }

    if (DEMUXER_TYPE_MPEG_TS == demuxer->type) {
        p_file_seq ->ts_priv = ds_ts_prog(demuxer);
        MLOGD("\nthis is ts file\n");

        if (demuxer) {
            free_demuxer(demuxer);
        }

        if (stream) {
            free_stream(stream);
        }

        return ;
    }

    ds = demuxer->video;
    ds2 = demuxer->audio;

    f = fopen("/media/casetest/video.dump", "wb");
    long all = 0;

    while (!ds->eof) {
        in_size = ds_get_packet(ds, &start);

        //skip audio if audio pts is smaller than video pts
        while (((ds->pts - ds2->pts) > 1) && (ds_get_packet(ds2, &start2) != -1)) {
            ;
        }

        if (in_size > 0) {
            all += in_size;
            if (f) {
                fwrite(start, 1, in_size, f);
            }
        }
    }

    if (in_size > 0) {
        all += in_size;
        if (f) {
            fwrite(start, 1, in_size, f);
        }
    }

    if (f) {
        fflush(f);
    }
    if (f) {
        fclose(f);
    }

    if (demuxer) {
        free_demuxer(demuxer);
    }

    if (stream) {
        free_stream(stream);    // kill cache thread
    }

    MLOGD("++++++ [%s] [%s] [%d] dump video file size = %d\n", __FILE__, __func__, __LINE__, all);

    sleep(1);
}

static void x_mp_dump_aud(void) {
    MLOGD("[%s] [%s] [%d] ==start start\n", __FILE__, __func__, __LINE__);
    // current_module = "demux_open";
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    demuxer_t *demuxer;
    stream_t *stream;
    int file_format;

    FILE *f;
    demux_stream_t *ds = NULL;
    demux_stream_t *ds2 = NULL;
    int in_size = 0;
    unsigned char *start;
    unsigned char *start2;

    if(!p_file_seq) {
        return;
    }
    p_file_seq->mem_init(p_file_seq, (void*)p_file_seq->file_seq_mem_start, p_file_seq->file_seq_mem_size);
    stream  = open_stream(p_file_seq->m_path[0], 0, &file_format);

    if (stream == NULL) {
        return;
    }

    MLOGD("\nmpctx->stream:%x,filename:%s\n", stream, p_file_seq->m_path[0]);
    demuxer = demux_mp_open(stream, file_format, audio_id, video_id, dvdsub_id, p_file_seq->m_path[0]);

    if (demuxer == NULL) {
        return;
    }

    if (DEMUXER_TYPE_MPEG_TS == demuxer->type) {
        p_file_seq->ts_priv = ds_ts_prog(demuxer);
        MLOGD("\nthis is ts file\n");

        if (demuxer) {
            free_demuxer(demuxer);
        }

        if (stream) {
            free_stream(stream);
        }

        return ;
    }

    // select stream to dump
    ds = demuxer->audio;
    ds2 = demuxer->video;

    f = fopen("/media/casetest/audio.dump", "wb");
    while (!ds->eof) {
        in_size = ds_get_packet(ds, &start);

        //skip video if video pts is smaller than audio pts
        while (((ds->pts - ds2->pts) > 1) && (ds_get_packet(ds2, &start2) != -1)) {
            ;
        }

        //MLOGI("***************get pkt size %d\n",in_size);
        if (in_size > 0) {
            if (f) {
                fwrite(start, 1, in_size, f);
            }
        }
    }

    if (in_size > 0) {
        if (f) {
            fwrite(start, 1, in_size, f);
        }
    }

    if (f) {
        fflush(f);
    }
    if (f) {
        fclose(f);
    }

    if (demuxer) {
        free_demuxer(demuxer);
    }

    if (stream) {
        free_stream(stream);    // kill cache thread
    }

    sleep(1);
}

static void free_file_seq_audio_metadata(FILE_SEQ_T *p_file_seq)
{
    if (p_file_seq->audio_track_num <= 0 || NULL == p_file_seq->audio_lang_array) {
        return;
    }

    int i;
    TRACK_LANG *p_audio_lang;
    for (i = 0; i < p_file_seq->audio_track_num; i++) {
        p_audio_lang = p_file_seq->audio_lang_array + i;

        if (p_audio_lang->lang) {
            mtos_free(p_audio_lang->lang);
        }

        if (p_audio_lang->title) {
            mtos_free(p_audio_lang->title);
        }
    }

    mtos_free(p_file_seq->audio_lang_array);
    p_file_seq->audio_track_num = 0;
    p_file_seq->audio_lang_array = NULL;
}


static void reset_file_seq_param(FILE_SEQ_T *pHandle)
{
    FILE_SEQ_T *p_file_seq =  pHandle ;
    if(p_file_seq == NULL) {
        MLOGE("!!!Error::p_file_seq == NULL\n");
        return ;
    }
    if(p_file_seq->pb_internal != NULL) {
        PLAYBACK_INTERNAL_T *pbi =
            ((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal));
        pbi->is_timeshift = 0;
        pbi->max_stream_probe_size = 0;
        pbi->max_stream_analyze_duration = 0;
    } else {
        MLOGE("!!!!!Error::p_file_seq->pb_internal == NULL!!!!!\n");
    }

    p_file_seq->isAudioEsEnd        = MT_FALSE;
    p_file_seq->isVideoEsEnd        = MT_FALSE;
    p_file_seq->isAudioBufferFull   = MT_FALSE;
    p_file_seq->isVideoBufferFull   = MT_FALSE;
    p_file_seq->exsubtitle          = 0;
    p_file_seq->totalVesNum         = 0;
    p_file_seq->totalAesNum         = 0;
    p_file_seq->p_ds_audio          = NULL;
    p_file_seq->p_ds_video          = NULL;
    p_file_seq->p_ds_sub            = NULL;
    p_file_seq->audio_average_bps   = 0;
    p_file_seq->video_average_bps   = 0;
    p_file_seq->max_audio_pts       = 0;
    p_file_seq->max_video_pts       = 0;
    p_file_seq->available_aes_bytes = 0;
    p_file_seq->available_ves_bytes = 0;
    p_file_seq->video_bps           = 0;
    p_file_seq->audio_bps           = 0;
    p_file_seq->cur_speed           = 0;
    p_file_seq->last_speed          = 0;
    p_file_seq->tmp_speed           = 0;
    p_file_seq->isTrickPlay         = MT_FALSE;
    p_file_seq->isNormalPlay        = MT_TRUE;
    p_file_seq->m_user_cmd          = 0;
    p_file_seq->internal_event      = 0;
    p_file_seq->m_tmp_ves_buf_pos   = 0;
    p_file_seq->m_aes_left          = 0;
    p_file_seq->checkAvTypeOK       = MT_FALSE;
    p_file_seq->checkDefinitionOK   = MT_FALSE;
    p_file_seq->is_av_codec_support = 0;
    p_file_seq->loadMedaiOK         = MT_FALSE;
    p_file_seq->isTrickToNormal     = MT_FALSE;
    p_file_seq->ref_first_audio_pts = 0;
    p_file_seq->ref_first_video_pts = 0;
    p_file_seq->isAudMute           = MT_FALSE;
    p_file_seq->is_ts               = 0;
    p_file_seq->is_play_at_time     = 0;
    p_file_seq->audio_track_id      = 0;
    p_file_seq->orig_apts           = 0;
    p_file_seq->first_vpts          = 0;
    p_file_seq->orig_vpts           = 0;
    p_file_seq->orig_spts           = 0;
    p_file_seq->sys_apts            = 0;
    p_file_seq->sys_vpts            = 0;
    p_file_seq->vpts_upload         = 0;
    p_file_seq->apts_upload         = 0;
    p_file_seq->only_audio_mode     = MT_FALSE;
    p_file_seq->p_v_pkt_start       = 0;
    p_file_seq->p_a_pkt_start       = 0;
    p_file_seq->p_sub_pkt_start     = 0;
    p_file_seq->left_v_pkt_bytes    = 0;
    p_file_seq->left_a_pkt_bytes    = 0;
    p_file_seq->p_extra_aud_buf     = 0;
    p_file_seq->extra_audio_size    = 0;
    p_file_seq->is_stable           = MT_FALSE;
    p_file_seq->isAudioDecoderStart = MT_FALSE;
    p_file_seq->isVideoDecoderStart = MT_FALSE;
    //feyang fix bug 133379
    p_file_seq->m_audio_codec_type = 0;
    p_file_seq->audio_pid = 0;
    p_file_seq->audio_output_mode = 0;
    //end fix bug 133379

    p_file_seq->video_fps = 0;
    memset(&p_file_seq->video_whfps, 0, sizeof(VIDEO_W_H_FPS));
    if (p_file_seq->total_path > 1) {

        mtos_free(p_file_seq->p_cur_demuxer);
        mtos_free(p_file_seq->p_cur_stream) ;
        mtos_free(p_file_seq->p_cur_ds_audio) ;
        mtos_free(p_file_seq->p_cur_ds_video) ;
        mtos_free(p_file_seq->p_cur_ds_sub) ;

        if (p_file_seq->p_m_duration) {
            mtos_free((void *)p_file_seq->p_m_duration);
            p_file_seq->p_m_duration = NULL;
        }

        p_file_seq->p_cur_demuxer   = NULL;
        p_file_seq->p_cur_stream    = NULL;
        p_file_seq->p_cur_ds_audio  = NULL;
        p_file_seq->p_cur_ds_video  = NULL;
        p_file_seq->p_cur_ds_sub    = NULL;

        p_file_seq->max_duration    = 0;
        p_file_seq->m_preload_state = FILE_SEQ_STOP;
    }
    if (p_file_seq->adaptive_playlist.list) {
        mlzp_free(p_file_seq->adaptive_playlist.list);
        p_file_seq->adaptive_playlist.num = 0;
        p_file_seq->adaptive_playlist.curr_idx = 0;
        p_file_seq->adaptive_playlist.list = NULL;
    }

    free_file_seq_audio_metadata(p_file_seq);
    ds_detect_hls_reset();
    io_isnetworkstream = 0;
    mp_field_pic_flag  = 0;
    h264_frm_only_flag = 1;
}

static void x_set_live_broadcast_func(void *p_handle, MT_BOOL value) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);
    g_is_live_broadcast = value;
}

static void file_seq_mem_release(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);

    //MLOGI("%s zx %d\n", __func__,p_file_seq->use_ext_heap);
    if (p_file_seq->use_ext_heap) {
        lib_memp_destroy(p_file_seq->p_ext_heap_hdl);
    }

    p_file_seq->use_ext_heap = MT_FALSE;
}

static void create_player_sub_handle(FILE_SEQ_T *p_file_seq) {
    if (NULL != p_file_seq->p_sub_fifo_handle) {
        clear_sub_fifo_kw(p_file_seq->p_sub_fifo_handle);
        return;
    }

    void *fifo_start_pos = mtos_malloc(SUB_FIFO_LEN);
    if (NULL == fifo_start_pos) {
        MLOGE("Alloc player sub fifo fail\n");
        return;
    }

    memset(fifo_start_pos, 0, SUB_FIFO_LEN);
    p_file_seq->p_sub_fifo_handle = init_fifo_kw(fifo_start_pos, SUB_FIFO_LEN);
    if (NULL == p_file_seq->p_sub_fifo_handle) {
        mtos_free(fifo_start_pos);
    }
}

static void destroy_player_sub_handle(FILE_SEQ_T *p_file_seq) {
    if (NULL == p_file_seq->p_sub_fifo_handle) {
        return;
    }

    fifo_type_t *fifo = (fifo_type_t *)(p_file_seq->p_sub_fifo_handle);
    void *p_start_pos = (void *)(fifo->start_pos);
    if (NULL != p_start_pos) {
        mtos_free(p_start_pos);
        fifo->start_pos = 0;
    }
    deinit_fifo_kw(p_file_seq->p_sub_fifo_handle);
    p_file_seq->p_sub_fifo_handle = NULL;
}

void handle_exit_fill_es_task(FILE_SEQ_T * p_file_seq)
{
    int send_event = -1;
    PLAYBACK_INTERNAL_T *pbi = NULL;

    MLOGI("[%s] start start ...\n", __func__);
    if (p_file_seq == NULL || p_file_seq->pb_internal == NULL) {
        return;
    }

    mtos_sem_take(&g_mplayer_handle_sem_lock, 0);
    pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    mp_ffmpeg_ext_cmd(MP_DEINIT_FFMPEG_MEM, MP_DO_CALLBACK, NULL);
    if (p_file_seq->force_stopped || (p_file_seq->ott_playmode != OTT_LIVE_MODE)) {
        x_stop_av_decoder(p_file_seq);
    }

    fpi_unregister_avplay_event(p_file_seq);
    {
        destroy_player_sub_handle(p_file_seq);

        x_unloadmedia_func(p_file_seq);

        x_set_live_broadcast_func(p_file_seq, MT_FALSE);
        if (p_file_seq ->ts_priv) {
            free33(((ts_info_t *)p_file_seq->ts_priv)->p_pmt) ;
            free33(p_file_seq ->ts_priv);
            p_file_seq ->ts_priv = NULL;
        }

        if (list_I_vpts) {
            list_rwlock_lock();
            destroy_list_vpts(list_I_vpts);
            list_rwlock_unlock();
            list_I_vpts = NULL;
        }

        file_seq_mem_release(p_file_seq);

        pbi->work_status = FILE_SEQ_STATUS_IDLE;
        mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
        if (NULL != p_file_seq->event_cb && pbi->self_exit == 0) {
            if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {
                p_file_seq->m_play_state = FILE_SEQ_STOP;
                send_event = FILE_PLAYBACK_SEQ_FILL_ES_TASK_EXIT;
            } else {
                p_file_seq->m_play_state = FILE_SEQ_STOP;
                send_event = FILE_PLAYBACK_SEQ_STOP;
            }
        }
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));

        while (MT_TRUE == p_file_seq->is_check_buffering_task_alive) {
            mtos_task_sleep(10);
        }
        if (-1 != send_event) {
            MLOGI("[%s] Send FILE_PLAYBACK_SEQ_%s!!\n", __func__,
                (send_event == FILE_PLAYBACK_SEQ_STOP) ? "STOP" : "FILL_ES_TASK_EXIT");
            mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
            p_file_seq->event_cb((FILE_PLAY_EVENT_E) send_event, 0);
            mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        }
    }

    pbi->self_exit = 0;
    if (pbi->extio_ctx) {
        mlzp_free(pbi->extio_ctx);
        pbi->extio_ctx = NULL;
    }
    p_file_seq->force_stopped = 0;

    drv_adp_deinit_avif(pbi);
    reset_file_seq_param(p_file_seq);
    show_sys_memory_info(&(pbi->start_mem_info), &(pbi->stop_mem_info), MT_TRUE);
    p_file_seq->is_task_alive = MT_FALSE;
    mtos_sem_give(&g_mplayer_handle_sem_lock);
    mtos_task_exit();
    MLOGI("%s %d end!!!!\n", __func__, __LINE__);
    return;
}

static MT_BOOL x_isNetStream_func(void *p_handle)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    int index;
    char *p_path;

    if(!p_file_seq) {
        return MT_FALSE;
    }
    index = p_file_seq->cur_play_index;
    p_path = p_file_seq->m_path[index];

    if (p_path) {
        if (strncmp(p_path, "http://", 7) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "https://", 8) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "rtsp://", 7) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "rtmp://", 7) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "rtmpe://", 8) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "mms://", 6) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "fifo:http://", 12) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "fifo:https://", 13) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "udp://@:", 8) == 0) {
            return MT_TRUE;
        } else if (strncmp(p_path, "srt://", 6) == 0) {
            MLOGI("[%s:%s:%d] ----davis  srt stream,network stream MT_TRUE\n", __FILE__, __func__, __LINE__);
            return MT_TRUE;
        } else {
            return MT_FALSE;
        }
    }

    return MT_FALSE;
}

void x_stop_av_decoder(void *pHandle) {
    FILE_SEQ_T *p_file_seq = pHandle;
    MT_ASSERT(p_file_seq != NULL);
    MLOGD("[%s] start start ...\n", __func__);

    if (p_file_seq->only_audio_mode == MT_FALSE) {
        if (fp_is_timeshift_file() == 1) {
            vdec_freeze_stop(p_file_seq->p_vdec_dev);
        } else {
            vdec_stop(p_file_seq->p_vdec_dev);
        }
    }

    if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
        aud_stop_vsb(p_file_seq->p_audio_dev);
    }

    vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 1);//VDEC_AVSYNC_DEFAULT_TS
    p_file_seq->m_aes_left = 0;
    MLOGD("[%s] end end ...\n", __func__);
}

static void x_unloadmedia_func(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return ;
    }

    MLOGI("[%s] start start ...\n", __func__);
    if (p_file_seq->p_demuxer) {
        demuxer_t *demuxer = p_file_seq->p_demuxer;
        p_file_seq->p_demuxer = NULL;
        free_demuxer(demuxer);
        MLOGD("[%s] free_demuxer\n", __func__);
    }

    if (p_file_seq->p_demuxer2) {
        demuxer_t *p_demux2 = (demuxer_t *)(p_file_seq->p_demuxer2);
        p_file_seq->p_demuxer2 = NULL;

        MLOGD("[%s] p_demux2->file_format: %d, p_demux2->type: %d\n", __func__, p_demux2->file_format, p_demux2->type);
        if (p_demux2->type == DEMUXER_TYPE_RTP) {
            MLOGD("[%s] call free_demuxer(p_file_seq->p_demuxer2)\n", __func__);
            free_demuxer(p_demux2);
        }
    }

    if (p_file_seq->p_stream) {
        free_stream(p_file_seq->p_stream);
        p_file_seq->p_stream = NULL;
        MLOGD("[%s] free_stream\n", __func__);
    }

    if (p_file_seq->ts_priv) {
        free33(((ts_info_t *)p_file_seq->ts_priv)->p_pmt) ;
        free33(p_file_seq->ts_priv);
        p_file_seq ->ts_priv = NULL;
    }

    p_file_seq->loadMedaiOK     = MT_FALSE;
    p_file_seq->isAudioEsEnd    = MT_FALSE;
    p_file_seq->only_audio_mode = MT_FALSE;

    MLOGI("[%s] end end ...\n", __func__);
}

char *get_demuxer_tag_info(void *demux_handle, char *tag)
{
    char **info;
    int n;

    MLOGI("[%s] start start ...\n", __func__);
    if (!demux_handle) {
        return NULL;
    }
    info = ((demuxer_t *)demux_handle)->info;
    if (!info || !tag) {
        return NULL;
    }

    for (n = 0; info[2 * n] != NULL; n++)
        if (!strcasecmp(info[2 * n], tag)) {
            break;
        }

    MLOGI("[%s] end end ...\n", __func__);
    return info[2 * n + 1] ? strdup33(info[2 * n + 1]) : NULL;
}

static MT_BOOL x_get_mp_tag_func(void *pHandle, char *p_tag_key, char *p_tag_value, int tag_value_len) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)pHandle;
    demuxer_t *p_demuxer = NULL;
    char *p_tmp_tag = NULL;

    MLOGD("[%s] p_file_seq [%p], p_tag_key [%p], p_tag_value [%p], tag_value_len [%d]\n", __func__, p_file_seq, p_tag_key, p_tag_value, tag_value_len);
    if (NULL != p_file_seq && NULL != p_tag_key && NULL != p_tag_value) {
        p_demuxer = (demuxer_t *)(p_file_seq->p_demuxer);

        MLOGD("[%s] p_demuxer [%p], p_demuxer->info [%p]\n", __func__, p_demuxer, p_demuxer->info);
        if (NULL != p_demuxer && NULL != p_demuxer->info) {
            p_tmp_tag = get_demuxer_tag_info(p_demuxer, p_tag_key);
        }

        if (NULL != p_tmp_tag) {
            strncpy(p_tag_value, p_tmp_tag, min(strlen(p_tmp_tag), tag_value_len));
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}

static void x_set_support_seek_finish(void *pHandle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)pHandle;

    if (NULL != p_file_seq) {
        p_file_seq->is_support_seek_finish = MT_TRUE;
    }
}

static void x_set_network_buffering(void *pHandle, MT_BOOL open) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)pHandle;

    if (NULL != p_file_seq){
        PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
        p_internal->net_buffer.is_open = open;
    }
}

static void set_demux_audio_info(FILE_SEQ_T *p_file_seq, sh_audio_t *sh_audio)
{
    /* Assume FOURCC if all bytes >= 0x20 (' ') */
    p_file_seq->audio_bps         = sh_audio->i_bps * 8;
    p_file_seq->audio_samplerate  = sh_audio->samplerate;
    p_file_seq->audio_channels    = sh_audio->channels;
    p_file_seq->audio_sample_bits = sh_audio->samplesize * 8;
    p_file_seq->audio_language    = sh_audio->lang;

    if (sh_audio->wf) {
        if (!p_file_seq->audio_samplerate || !p_file_seq->audio_channels) {
            p_file_seq->audio_channels    = sh_audio->wf->nChannels;
            p_file_seq->audio_samplerate  = sh_audio->wf->nSamplesPerSec;
            p_file_seq->audio_sample_bits = sh_audio->wf->wBitsPerSample;
            MLOGD("[%s] -----[audio] set sample rate & ch with wf(WAVEFORMATEX)\n", __func__);
        }
    }
}

MT_BOOL set_demux_media_info(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    demuxer_t *p_demuxer = (demuxer_t *)(p_file_seq->p_demuxer);
    sh_audio_t *p_sh_audio = p_demuxer->audio->sh;
    sh_video_t *p_sh_video = p_demuxer->video->sh;
    double video_start_pts = MP_NOPTS_VALUE;
    double start_pts       = MP_NOPTS_VALUE;

    struct stat stat_buf;

    MLOGD("[%s] start start!!!!!\n", __func__);
    if (p_sh_video) {
        p_file_seq->video_bps    = p_sh_video->i_bps * 8;
        p_file_seq->video_disp_h = p_sh_video->disp_h;
        p_file_seq->video_disp_w = p_sh_video->disp_w;
        p_file_seq->video_fps    = (int)(m_q2d(p_sh_video->fps) + 0.5);
        video_start_pts = ds_get_next_pts(p_demuxer->video);
		p_file_seq->video_start_pts = video_start_pts*TIME_BASE;

        MLOGD("[%s] -----[video] bps=%d, disp_h=%d, disp_w=%d, fps=%d,v_start_pts: %d\n", __func__, \
              p_file_seq->video_bps, p_file_seq->video_disp_h, p_file_seq->video_disp_w, p_file_seq->video_fps, (int)(video_start_pts * 1000));

        if (p_file_seq->video_fps >= 61) {
            p_file_seq->video_fps = 0;
        }
        if (!p_file_seq->video_fps) {
            revise_fps = 1;

        }

        if (!p_file_seq->video_disp_w) {
            p_file_seq->internal_event = GET_ES_VIDEO_W_H;
            MLOGD("[%s] -----[video] get video from decoder\n", __func__);
        }
    }

    if (p_sh_audio) {
        set_demux_audio_info(p_file_seq, p_sh_audio);
        start_pts = ds_get_next_pts(p_demuxer->audio);
        MLOGI("[%s] [audio] bps=%d, samplerate=%d, ch=%d, bits=%d, language: %s, a_start_pts: %d\n", __func__,
              p_file_seq->audio_bps, p_file_seq->audio_samplerate, p_file_seq->audio_channels,
              p_file_seq->audio_sample_bits, p_file_seq->audio_language, (int)(start_pts * 1000));
    }

    if (video_start_pts != MP_NOPTS_VALUE) {
        if (start_pts == MP_NOPTS_VALUE || !p_sh_audio ||
            (p_sh_video && video_start_pts < start_pts)) {
            start_pts = video_start_pts;
        }
    }

    if (start_pts != MP_NOPTS_VALUE) {
        MLOGD("[%s] -----file start time =%d ms\n", __func__, (int)(start_pts * 1000));
    } else {
        MLOGD("[%s] -----file start time =unknown\n", __func__);
    }

    p_file_seq->file_start_time = start_pts;
    p_file_seq->file_duration   = demuxer_get_time_length(p_demuxer);
    p_file_seq->duration = p_file_seq->file_duration;

    if (p_file_seq->duration - p_file_seq->max_duration > 0) {
        p_file_seq->max_duration = p_file_seq->duration;
    }

    stat(p_file_seq->m_path[0], &stat_buf);
    p_file_seq->file_size = stat_buf.st_size;

    if (p_demuxer->type == DEMUXER_TYPE_MPEG_TS) {
        MLOGD("[%s] -----p_demuxer->type == DEMUXER_TYPE_MPEG_TS\n", __func__);
        p_file_seq->is_ts = 1;
        set_ts_pmt_avs_info(p_file_seq->ts_priv);
        p_file_seq->internal_event = GET_TS_MEDIA_INFO;
    }

    MLOGI("file size:%lld duration:%dms\n",  p_file_seq->file_size, (int)(p_file_seq->file_duration * 1000));
    return MT_TRUE;
}

static MT_BOOL check_aonly_task_finish(FILE_SEQ_T *p_file_seq)
{
    int check_cnt = 0;
    if (!p_file_seq) {
        return MT_FALSE;
    }

    u32 pcm_size = 0;
    int last_aud_freespace = -1;
    MLOGI("[%s] audio  demux stream is finished  !!\n", __func__);
    while (1) {
        int   cur_aud_freespace  = 0;
        float aes_buf_consume_ms = 0.0;

        if (p_file_seq->m_play_state == FILE_SEQ_EXIT ||
            p_file_seq->m_play_state == FILE_SEQ_STOP) {
            return MT_TRUE;
        }

        if (do_user_cmd()) {
            return MT_FALSE;
        }

        handle_pending_event();
        aud_file_getleftesbuffer_vsb(p_file_seq->pb_internal, (u32 *) &cur_aud_freespace);
        aud_file_getleft_ao_pcm_bytes(p_file_seq->pb_internal, &pcm_size);
        aes_buf_consume_ms = (((float)(p_file_seq->dec_cap.max_aes_num - cur_aud_freespace) * 1.0) / (p_file_seq->audio_bps) * 1.0) * 1000.0;
        if (pcm_size <= 1024                        &&
            last_aud_freespace == cur_aud_freespace &&
            cur_aud_freespace >= (p_file_seq->dec_cap.max_aes_num - 1024)) {
            /*increase check count ,make sure some very short audio can be rendered*/
            mtos_task_sleep(10);
            if (check_cnt++ > 50) {
                if (NULL != p_file_seq->event_cb && aes_buf_consume_ms > 0) {
                    p_file_seq->event_cb(FILE_PLAYBACK_SEQ_GET_VPTS, (u32)(p_file_seq->max_audio_pts));
                    MLOGD("Send apts %u, %d, %u\n",
                          (u32)(p_file_seq->max_audio_pts), cur_aud_freespace, (u32)(aes_buf_consume_ms * 1000));
                    mtos_task_sleep(10);
                }

                p_file_seq->is_play_to_end = 1;
                MLOGI("Audio finished, adec es:%d pcm size:%d\n",
                    p_file_seq->dec_cap.max_aes_num - cur_aud_freespace, pcm_size);
                return MT_TRUE;
            }
        } else {
            check_cnt = 0;
            MLOGD("Ad es buffer not empty, max %d used %d free %d\n",
                  p_file_seq->dec_cap.max_aes_num,
                  p_file_seq->dec_cap.max_aes_num - cur_aud_freespace, cur_aud_freespace);
            mtos_task_sleep(200);
        }
        last_aud_freespace = cur_aud_freespace;
    }
}

static void send_last_vpts(FILE_SEQ_T *p_file_seq) {
    if (NULL != p_file_seq->event_cb && p_file_seq->max_video_pts > p_file_seq->first_vpts) {
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_GET_VPTS,
                             (u32)(p_file_seq->max_video_pts - p_file_seq->first_vpts));
    }
}

MT_BOOL check_task_finish(FILE_SEQ_T *p_file_seq) {
    /*
     * when all the audio and video es data has been demuxed,
     * sequencer should wait av_decoder finished !!!
    */
    if(!p_file_seq) {
        return MT_FALSE;
    }
    if (p_file_seq->only_audio_mode && p_file_seq->isAudioEsEnd) {
        return check_aonly_task_finish(p_file_seq);
    }

    if (p_file_seq->isVideoEsEnd &&
        (MT_FALSE == p_file_seq->isAudioDecoderStart || MT_TRUE == p_file_seq->is_audio_deecoder_error)) {
        p_file_seq->isAudioEsEnd = MT_TRUE;
    }

    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if ((MT_FALSE == p_file_seq->isVideoEsEnd || MT_FALSE == p_file_seq->isAudioEsEnd)
        && (p_internal->video_decoder_over == MT_FALSE)) {
        return MT_FALSE;
    }

    MLOGI("[%s] audio and video demux stream is finished  !!\n", __func__);
    do {
        drv_pts_info_t  cur_vdec_info;
        memset(&cur_vdec_info, 0, sizeof(drv_pts_info_t));

        u64 last_vpts             = p_file_seq->vpts_upload;
        int check_stable_vpts_cnt = 0;
        u64 vpts                  = 0;
        int freespace             = 0;
        int max_freespace         = 0;
        int check_stop_cnt        = 0;
        int ves_buf_size          = 0;

        if (OTT_LIVE_MODE != p_file_seq->ott_playmode) { // for CI test,EOF can't flush pts
            handle_pending_event();
        }

        vdec_get_es_buf_space(p_file_seq->pb_internal, (u32 *)&freespace);
        vdec_get_es_buf_size(p_file_seq->pb_internal, (u32 *)&ves_buf_size);

        max_freespace = freespace;
        while (freespace < ves_buf_size) {
            drv_pts_info_t vstate;
            vdec_get_pts(p_file_seq->p_vdec_dev, &vstate);

            vpts = vstate.pts;
            if ((p_file_seq->load_media_state == FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS &&
                 p_file_seq->ott_playmode == OTT_LIVE_MODE) || (p_file_seq->force_stopped == 1)) {
                return MT_TRUE;
            }

            if (vpts != last_vpts) {
                last_vpts = vpts;
                check_stable_vpts_cnt = 0;
            } else {
                if (p_file_seq->m_play_state != FILE_SEQ_PAUSE) {
                    check_stable_vpts_cnt++;
                }
                mtos_task_sleep(100);
            }

            if (do_user_cmd()) {
                MLOGI("[%s] Seek after es end!\n", __func__);
                return MT_FALSE;
            }

            if (OTT_LIVE_MODE != p_file_seq->ott_playmode) {
                handle_pending_event();
            }

            if ((p_file_seq->m_play_state == FILE_SEQ_EXIT || p_file_seq->m_play_state == FILE_SEQ_STOP) &&
                (p_file_seq->ott_playmode != OTT_LIVE_MODE || p_file_seq->force_stopped == 1)) {
                return MT_TRUE;
            }

            if (p_file_seq->m_play_state == FILE_SEQ_PAUSE) {
                continue;
            }

            vdec_get_es_buf_space(p_file_seq->pb_internal, (u32 *)&freespace);
            if (max_freespace == freespace) {
                check_stop_cnt++;
            }

            if (freespace > max_freespace) {
                max_freespace = freespace;
                check_stop_cnt = 0;
            }

            if (p_file_seq->m_play_state == FILE_SEQ_EXIT && check_stop_cnt > 10) {
                return MT_TRUE;
            }
            /*
             * decoder notify play end or pts stable 5s or
             * buffer meet the threashold and pts not change for 1 seconds
             */
            if (check_stable_vpts_cnt >= 50 || 1 == p_file_seq->is_play_to_end ||
                (freespace >= (ves_buf_size - 21) && check_stable_vpts_cnt > 10)) {
                send_last_vpts(p_file_seq);
                MLOGI("av decoder check finished, cnt:%d\n", check_stable_vpts_cnt);
                p_file_seq->is_play_to_end = 1;
                return MT_TRUE;
            }

            MLOGD("[%s] free space %d total %d\n", __func__, freespace, ves_buf_size);
        }
        send_last_vpts(p_file_seq);

        MLOGI("[%s] av decoder finished !!!\n", __func__);
        mtos_task_sleep(300);//forbug 66322
        p_file_seq->is_play_to_end = 1;
        return MT_TRUE;
    } while (MT_FALSE == check_file_seq_exit(p_file_seq));

    return MT_TRUE;
}

void file_seq_vo_handle_set(mt_handle vHandle)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    if(!p_file_seq) {
        return;
    }
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    if(NULL == p_file_seq->pb_internal){
        return ;
    }

    MLOGD("[%s_%d], application set VO handle\n", __FUNCTION__, __LINE__);
    p_internal->vo_handle = vHandle;
}

void file_seq_get_vo_handle(mt_handle *vHandle)
{
    PLAYBACK_INTERNAL_T *p_internal = NULL;
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    if(!p_file_seq) {
        return;
    }
    p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    if(!p_internal) {
        return;
    }


    *vHandle = p_internal->vo_handle;
    MLOGD("[%s_%d], get VO handle vHandle = %d\n", __FUNCTION__, __LINE__, *vHandle);
}

static int fileplay_interrupt_cb(void *para)
{
    s64 time_out = 30 * 1000 * 1000;
    FILE_SEQ_T *p_seq = para;
    if (check_file_seq_exit(p_seq)) {
        MLOGE("Interrupt by user exit\n");
        return AVERROR_EXIT;
    }
    if (p_seq->force_stopped) {
        MLOGE("Interrupt by force_stopped\n");
        return AVERROR_EXIT;
    }

    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_seq->pb_internal);
    /* Maybe changed after below line */
    s64 media_load_start_time = pbi->media_load_start_time;
    if (media_load_start_time < 0) {
        return MT_FALSE;
    }

    if (p_seq->m_path[0] && strstr(p_seq->m_path[0], "rtmp://")) {
        time_out = 60 * 1000 * 1000;
    }

    s64 current_time = mclock_get_utime();
    if (current_time - media_load_start_time > time_out) {
        MLOGE("Load media %lld timedout interrupt, start:%lld\n", time_out, media_load_start_time);
        if (media_load_start_time != 0) {
            return AVERROR(ETIMEDOUT);
        }
        return AVERROR_EXIT;
    }
    return MT_FALSE;
}

static int fileplay_message_cb(
    void *s, int type, void *data, size_t data_size)
{
    FILE_SEQ_T *p_file_seq = s;
    if (!p_file_seq || !p_file_seq->pb_internal) {
        return 0;
    }

    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if (DEMUXER_ADAPTIVE_BITRATE_UPDATED == type && data && data_size == sizeof(int)) {
        int need_change = *((int *) data);
        return fpi_change_playlist(p_file_seq, need_change);
    } else if (DEMUXER_ADAPTIVE_LIVE_MEDIA_UPDATED == type && data && data_size == sizeof(int)) {
        int is_live = *((int *) data);
        pbi->is_live = is_live;
    } else if (DEMUXER_ADAPTIVE_QUERY_IDLE_TIME == type) {
        return fpi_query_idle_time(p_file_seq, (s64 *)data);
    } else if (DEMUXER_ADAPTIVE_QUERY_PLAY_STATE == type && data && data_size == sizeof(int)) {
        if (FILE_SEQ_LOADED == p_file_seq->m_play_state ||
            FILE_SEQ_PLAY == p_file_seq->m_play_state) {
            *((int *) data) = 1;
        } else {
            *((int *) data) = 0;
        }
    } else if (DEMUXER_ADAPTIVE_SERVER_ERROR == type && data && data_size == sizeof(int)) {
        u32 error_code = (u32) *((int *) data);
        if (p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_LIVE_SERVER_ERROR, error_code);
        }
    } else if (DEMUXER_ADAPTIVE_ASYNC_OPEN == type) {
        pbi->need_async_done_msg = 1;
        if(p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_SEQ_ASYNC_OPEN, 0);
        }
    } else if (DEMUXER_ADAPTIVE_ASYNC_DONE == type) {
        pbi->audio.pts_hold_counters = 0; /* ignore async done before */
        if(p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_SEQ_ASYNC_DONE, 0);
        }
        pbi->need_async_done_msg = 0;
    }

    return 0;
}

static void set_stream_context(FILE_SEQ_T *p_file_seq)
{
    stream_t *stream = (stream_t *)(p_file_seq->p_stream);
    if (!stream) {
        return;
    }
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    stream->callback_ctx = (void *) p_file_seq;
    stream->interrupt_callback    = fileplay_interrupt_cb;
    stream->app_message_callback  = fileplay_message_cb;
    stream->transport_desdec_ctx  = pbi->transport_desdec_ctx;
    stream->transport_desdec_func = pbi->transport_desdec_func;
    if (pbi->max_stream_probe_size       > 0 &&
        pbi->max_stream_analyze_duration > 0) {
        stream->max_stream_probe_size       = pbi->max_stream_probe_size;
        stream->max_stream_analyze_duration = pbi->max_stream_analyze_duration;
    }
}

static void set_ffmpeg_dict_opts(
    FILE_SEQ_T *p_file_seq,
    const char *key, const char *value, int flags)
{
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    MT_BOOL use_ext_heap = p_file_seq->use_ext_heap;
    /* Not use heap for ffmpeg dict opts */
    p_file_seq->use_ext_heap = MT_FALSE;
    int ret = av_dict_set((AVDictionary **) (&(pbi->ff_dict_opts)), key, value, flags);
    if (ret < 0) {
        MLOGE("[%p] %s %s set fail!\n", p_file_seq, __func__, key);
    }
    p_file_seq->use_ext_heap = use_ext_heap;
}

static void destroy_ffmpeg_dict_opts(FILE_SEQ_T *p_file_seq)
{
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    MT_BOOL use_ext_heap = p_file_seq->use_ext_heap;
    /* Not use heap for ffmpeg dict opts */
    p_file_seq->use_ext_heap = MT_FALSE;
    av_dict_free((AVDictionary **) (&(pbi->ff_dict_opts)));
    p_file_seq->use_ext_heap = use_ext_heap;
}

static MT_BOOL x_loadMedia2(void *p_handle)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    if(NULL == p_file_seq || NULL == p_file_seq->pb_internal) {
        return MT_FALSE ;
    }

    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    char *file_path = (char *)(p_file_seq->m_path[p_file_seq->cur_play_index]);
    MLOGD("[%s] start start...\n", __func__);

    MLOGI(" fileplay time node 0 [%s %s_%d]+\n", __FILE__, __FUNCTION__, __LINE__);
    p_internal->vo_handle = -1;
    if (p_file_seq->loadMedaiOK) {
        x_unloadmedia_func(p_file_seq);
    }

    mp_ffmpeg_ext_cmd(MP_FFMPEG_MOV_KEYGEN, MP_DO_SET_PARAM, (void *)g_mov_desc_key);
    mp_ffmpeg_ext_cmd(MP_FFMPEG_MOV_KEYGEN_LEN, MP_DO_SET_PARAM, (void *)g_mov_desc_key_len);
    if (!p_file_seq->loadMedaiOK) {
        int file_format = 0;
        p_file_seq->stream_type = STREAM_LIVE;
        if (file_path && file_path[0]) {
            AVIOInterruptCB interrupt_callback =
                {.callback = fileplay_interrupt_cb, .opaque = (void *) p_file_seq};
            stream_io_options_t  io_options = {
                .interrupt_cb = &interrupt_callback,
                .ff_dict_opts = &(p_internal->ff_dict_opts),
            };

            p_internal->media_load_start_time = mclock_get_utime();
            p_file_seq->p_stream = open_stream(file_path,
                (char **) ((uintptr_t) &io_options), &file_format);
        } else if (p_internal->extio_ctx && p_internal->extio_ctx->opaque) {
            p_internal->media_load_start_time = mclock_get_utime();
            const char filename[64] = "fifo://";
            p_file_seq->p_stream = open_stream(
                filename, (char **) ((uintptr_t) &(p_internal->extio_ctx)), &file_format);
        }
        set_stream_context(p_file_seq);
        if (p_file_seq->p_stream == NULL) {
            MLOGE("[%s] fail to open_stream, path:%s!!!!!\n", __func__, file_path);
            return MT_FALSE;
        }
        MLOGD("%s %d mpctx->stream:%x,m_path:%s\n", __FUNCTION__, __LINE__, p_file_seq->p_stream, file_path);
        p_file_seq->p_demuxer = demux_mp_open((stream_t *)(p_file_seq->p_stream), file_format, audio_id, video_id, dvdsub_id, file_path);
        if (p_file_seq->p_demuxer == NULL) {
            MLOGD("[%s] fail to demux_open!!!!!\n", __func__);
            MLOGD("[%s] m_path:%s!!!!!\n", __func__, p_file_seq->m_path[p_file_seq->cur_play_index]);

            if (p_file_seq->p_stream) {
                free_stream(p_file_seq->p_stream);
                p_file_seq->p_stream = NULL;
            }

            return MT_FALSE;
        }
        p_file_seq->p_ds_audio = NULL;
        p_file_seq->p_ds_video = NULL;
        p_file_seq->p_ds_sub   = NULL;
        p_file_seq->exsubtitle = 0;
        p_file_seq->subt_id    = -1;
        p_file_seq->p_ds_audio = (void *)(((demuxer_t *)(p_file_seq->p_demuxer))->audio);
        p_file_seq->p_ds_video = (void *)(((demuxer_t *)(p_file_seq->p_demuxer))->video);
        p_file_seq->p_ds_sub   = (void *)(((demuxer_t *)(p_file_seq->p_demuxer))->sub);

        //p_file_seq->exsubtitle = load_subtitles_mp(p_file_seq->m_path, fps);
        if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {
            return MT_TRUE;
        }

        if (DEMUXER_TYPE_MPEG_TS == ((demuxer_t *)(p_file_seq->p_demuxer))->type) {
            p_file_seq ->ts_priv = ds_ts_prog(p_file_seq->p_demuxer);
        }

        ds_get_vfilter_type(p_file_seq->p_ds_video);
        set_demux_media_info(p_file_seq);
        MLOGI("[%s] ---------v_bps:%d  a_bps:%d duration:%lf \n", __func__, \
              p_file_seq->video_bps, p_file_seq->audio_bps, p_file_seq->duration);
        p_file_seq->loadMedaiOK = MT_TRUE;
        p_internal->media_load_start_time = -1;
    }

    MLOGI(" fileplay time node 1 [%s %s_%d]+\n", __FILE__, __FUNCTION__, __LINE__);
    MLOGD("[%s] end end...\n", __func__);
    return MT_TRUE;
}

static int create_buffering_thread(FILE_SEQ_T *p_file_seq)
{
    /* Not create buffering thread for local file */
    if (MT_FALSE == x_isNetStream_func(p_file_seq)) {
        return MT_SUCCESS;
    }

    pthread_t g_check_buffering_thread;
    int err = -1;
    pthread_attr_t attribs;
    pthread_attr_init(&attribs);
    pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
    err = pthread_create(&g_check_buffering_thread, &attribs, (void *)check_buffering_thread, (void *)p_file_seq);
    if (err != 0) {
        MLOGW("can't create check buffering thread!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        p_file_seq->m_play_state = FILE_SEQ_STOP;
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR, p_file_seq->path_id);
        MLOGE("[%s] %d mtos_task_create error.\n", __func__, __LINE__);
        p_file_seq->is_load_task_alive = MT_FALSE;
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static void do_load_media_action(void *p_param)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_param;
    if(NULL == p_file_seq){
        return;
    }

    prctl(PR_SET_NAME, "load_media_task");
    p_file_seq->is_load_task_alive = MT_TRUE;
    MT_BOOL  load_stat  = MT_FALSE;
    int  load_cnt    = 2;
    MT_BOOL unloadmedia = MT_FALSE;

    struct timeval time_start;
    struct timeval time_end;

    gettimeofday(&time_start, 0);
    mt_set_pthread_name(__FUNCTION__);

    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    while (p_internal->load_state != 1) {
        mtos_task_sleep(100);
    }

    if (p_file_seq->loadMedaiOK) {
        x_unloadmedia_func(p_file_seq);
    }

    MLOGD("@@@@@@@%s %d waiting ~[%d]~\n", __func__, __LINE__, p_file_seq->m_play_state);
    if (strstr(p_file_seq->m_path[0], "rtmp://")) {
        load_cnt = 2;
    }

    if (p_file_seq->load_media_times > 0) {
        load_cnt = p_file_seq->load_media_times;
    }

    MLOGI("[%s]  start start [%d].\n", __func__, __LINE__);
#ifdef CONFIG_MT_FILEPLAY_NONETWORK
    if (strstr(p_file_seq->m_path[0], "youtube.com") || strstr(p_file_seq->m_path[0], "youtu.be")) {
        replace_youtube_real_play_path(p_file_seq->m_path[0]);
        MLOGI("[%s]  replace_real_play_path done\n", __func__);
    }
#endif
    int max_mov_index_num = INT_MAX;
    if (MT_TRUE == p_file_seq->use_ext_heap) {
        /* about (2.5)/10000 of the memory */
        max_mov_index_num = MAX(256, (p_file_seq->file_seq_mem_size) / (4 * 1024));
        if (IS_STREAM_URL(p_file_seq->m_path[0])) {
            max_mov_index_num = 40000;
        }
    }
    do {
        load_cnt--;
        if (load_cnt < 0 || p_file_seq->m_play_state == FILE_SEQ_EXIT) {
            MLOGE("[%s] fail to x_loadMedia, state:%d\n", __func__, p_file_seq->m_play_state);

            mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
            if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {
                p_file_seq->m_play_state = FILE_SEQ_LOADED;
                p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT, p_file_seq->path_id);
            } else {
                p_file_seq->m_play_state = FILE_SEQ_STOP;
                p_file_seq->m_play_state = FILE_SEQ_LOADED;
                p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR, p_file_seq->path_id);
            }

            mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
            p_file_seq->is_load_task_alive = MT_FALSE;
            mtos_task_exit();
            return;
        }

        MLOGI("!@@@!!!![%s] %d  load:%d \n", __func__, __LINE__, load_cnt);
        p_file_seq->cur_play_index = 0;
        mp_ffmpeg_ext_cmd(MP_FFMPEG_MOV_SEG_NUM, MP_DO_SET_AVFMT, (void *) &max_mov_index_num);
        load_stat = x_loadMedia2(p_file_seq);
        mp_ffmpeg_ext_cmd(MP_DEINIT_AVFMT, MP_DO_SET_AVFMT, NULL);
        if (MT_FALSE == load_stat) {
            mp_ffmpeg_ext_cmd(MP_DEINIT_FFMPEG_MEM, MP_DO_CALLBACK, NULL);
        }
    } while (load_stat == MT_FALSE);

    create_player_sub_handle(p_file_seq);
    if (((demux_stream_t *)(p_file_seq->p_ds_video))->sh == NULL) {
        p_file_seq->only_audio_mode = MT_TRUE;
        MLOGE("[%s] p_ds_audio[0x%x] p_ds_video[0x%x] !! \n",
              __func__, p_file_seq->p_ds_audio, p_file_seq->p_ds_video);
    }

    MLOGD("[%s] --media type [%d] \n", __func__,
          ((demuxer_t *)(p_file_seq->p_demuxer))->type);

    if (DEMUXER_TYPE_AUDIO == ((demuxer_t *)(p_file_seq->p_demuxer))->type) {
        if (x_check_aud_codec_type(p_file_seq) == MT_FALSE) {
            MLOGE("[%s] fail to check_aud_codec_type ...\n", __func__);
            unloadmedia = MT_TRUE;
        }
    } else if (DEMUXER_TYPE_RTP == ((demuxer_t *)(p_file_seq->p_demuxer))->type) {
        p_file_seq->rtsp_play_mode = MT_TRUE;
        if (x_check_av_codec_type(p_file_seq) == MT_FALSE) {
            MLOGE("[%s] fail to check_av_codec_type ...\n", __func__);
            unloadmedia = MT_TRUE;
        }
    } else {
        if (x_check_av_codec_type(p_file_seq) == MT_FALSE) {
            MLOGE("[%s] fail to check_av_codec_type ...\n", __func__);
            unloadmedia = MT_TRUE;
        }
    }

    if (MT_TRUE == unloadmedia) {
        x_unloadmedia_func(p_file_seq);
        mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
        if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {

            p_file_seq->m_play_state = FILE_SEQ_LOADED;
            p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT, p_file_seq->path_id);
        } else {
            p_file_seq->m_play_state = FILE_SEQ_STOP;
            p_file_seq->m_play_state = FILE_SEQ_LOADED;
            p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR, p_file_seq->path_id);
        }

        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        p_file_seq->is_load_task_alive = MT_FALSE;
        mtos_task_exit();
        return;
    }

    p_file_seq->p_cur_demuxer  = p_file_seq->p_demuxer;
    p_file_seq->p_cur_stream   = p_file_seq->p_stream;
    p_file_seq->p_cur_ds_audio = p_file_seq->p_ds_audio;
    p_file_seq->p_cur_ds_video = p_file_seq->p_ds_video;
    p_file_seq->p_cur_ds_sub   = p_file_seq->p_ds_sub;

    MLOGD("[%s] -----out of x_check_av_codec_type MT_TRUE \n", __func__);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);

    g_last_get_vpts = 0;
    if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {
        x_unloadmedia_func(p_file_seq);

        if (p_file_seq->total_path > 1) {
            p_file_seq->m_preload_state = FILE_SEQ_EXIT;
        }

        MLOGI("[%s] drv send event 'FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT'\n", __func__);
        p_file_seq->m_play_state = FILE_SEQ_LOADED;
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT, p_file_seq->path_id);
    } else {
        p_file_seq->load_media_state = FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS;
        p_file_seq->m_play_state = FILE_SEQ_LOADED;
        if (MT_SUCCESS != create_buffering_thread(p_file_seq)) {
            mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
            return;
        }

        if (p_file_seq->m_play_state != FILE_SEQ_STOP && p_file_seq->m_play_state != FILE_SEQ_EXIT) {
            p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS, p_file_seq->path_id);
        } else {
            MLOGE("%s m_play_state change to %d, exit load media\n", __func__, p_file_seq->m_play_state);
            p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR, p_file_seq->path_id);
        }
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    p_file_seq->is_load_task_alive = MT_FALSE;

    gettimeofday(&time_end, 0); //time0.tv_sec = 0;
    MLOGI("[%s] end, consume %0.2f(ms)\n", __func__, (time_end.tv_sec - time_start.tv_sec) * 1000.0
          + ((time_end.tv_usec - time_start.tv_usec) * 1.0) / 1000.0);
    mtos_task_exit();
    return;
}

static void file_seq_mem_init(void *p_handle, u8 *p_ext_buf_addr, u32 ext_buf_size)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    MT_ASSERT(p_file_seq != NULL);
    MLOGI("Player heap hdl %p addr %p size:0x%x\n",
        p_file_seq->p_ext_heap_hdl, p_ext_buf_addr, ext_buf_size);
    if (p_ext_buf_addr != NULL && ext_buf_size != 0) {
        memset(p_file_seq->p_ext_heap_hdl, 0, sizeof(lib_memp_t));
        if (lib_memp_create(p_file_seq->p_ext_heap_hdl, p_ext_buf_addr, ext_buf_size) != SUCCESS) {
            MT_ASSERT(0);
        }
        p_file_seq->use_ext_heap = MT_TRUE;
    } else {
        p_file_seq->use_ext_heap = MT_FALSE;
    }
}

/*********************************************************************************
 * The fowwing APIs will be exposed  to upper layer code
 * peacer 2013-02-07
 ********************************************************************************************
 */
static int x_loadmedia_func(void *p_handle)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    if(NULL == p_file_seq){
        return -1;
    }
    MT_BOOL ret = MT_FALSE;
    MT_BOOL load_in_task = MT_FALSE;

    MLOGI("[%s] start start ...\n", __func__);
    if ((p_file_seq->m_play_state == FILE_SEQ_LOADMEDIA)) {
        MLOGE("[%s] error, cause:FILE_SEQ_LOADMEDIA\n", __func__);
        mtos_task_sleep(300);
        mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR, p_file_seq->path_id);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return 0;
    } else {
        while (p_file_seq->is_load_task_alive || (p_file_seq->is_task_alive)) {
            MLOGE("[%s] error, cause:task alive![%d %d]\n", __func__,
                p_file_seq->m_play_state, p_file_seq->is_load_task_alive);
            mtos_task_sleep(300);
        }
    }

    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    if (p_file_seq->m_path[0] == NULL && !p_internal->extio_ctx) {
        mtos_task_sleep(300);
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR, p_file_seq->path_id);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return 0;
    }

    if ((p_internal->work_status == FILE_SEQ_STATUS_WORK)) {
        MLOGE("[%s][ERROR] fail to loadmedia_task, state[%d]!!!\n", __func__, p_file_seq->m_play_state);
        if (p_file_seq->m_play_state != FILE_SEQ_EXIT) {
            p_file_seq->m_play_state  = FILE_SEQ_EXIT;
            p_internal->self_exit = 1;
        }

        //yliu add
        mtos_task_sleep(200);
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_NOT_READY, p_file_seq->path_id);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return -1;
    }

    p_file_seq->m_play_state = FILE_SEQ_LOADMEDIA;

    if (p_file_seq->file_seq_mem_size < 10 * 1024 * 1024) {
        g_player_buffer_size_mode = 0;
#ifndef CONFIG_MT_CHIP_SYMPHONY4
        //save_avi_memory_set_value(100, 30000);//avi
#endif
    } else {
        g_player_buffer_size_mode = 1;
    }

    file_seq_mem_init(p_file_seq, (void *)p_file_seq->file_seq_mem_start, p_file_seq->file_seq_mem_size);
    load_in_task = 1;
    MLOGI("!!!@@@%s %d load:%d\n", __func__, __LINE__, load_in_task);
    if (load_in_task == 1) {

        if (ret) {
            MLOGE("delete failed =%d \n", ret);
        }

        p_internal->load_state = 0;
        p_file_seq->load_media_times = 0;
        p_file_seq->is_check_buffering_task_alive = MT_FALSE;
        pthread_t loadmedia_thread;
        int err = -1;
        pthread_attr_t attribs;
        pthread_attr_init(&attribs);
        pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
        err = pthread_create(&loadmedia_thread, &attribs, (void *)do_load_media_action, (void *)p_handle);
        if (err != 0) {
            MLOGE("can't create load media thread!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            exit(1);
        }

        while (p_file_seq->is_load_task_alive == MT_FALSE) {
            MLOGD("%s %d waiting ~~\n", __func__, __LINE__);
            mtos_task_sleep(100);
        }

        p_internal->load_state = 1;
        MLOGD("@@@@@@@%s %d waiting ~~\n", __func__, __LINE__);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        MLOGI("%s %d task begin\n", __func__, __LINE__);
        return 0;
    }
    return 0;
}

static  int x_start_func(void *p_handle, unsigned int start_seconds) {
    int ret = 0;
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }
    void (* p_fill_es_task)(void *p_param) = NULL;

    MLOGI("[%s] start start ...\n", __func__);
    if ((p_file_seq->m_play_state  != FILE_SEQ_LOADED) || p_file_seq->loadMedaiOK == MT_FALSE) {
        MLOGE("[%s] is_task_alive[%d] loadMedaiOK[%d] state[%d]\n",
            __func__, p_file_seq->is_task_alive, p_file_seq->loadMedaiOK, p_file_seq->m_play_state);
        mtos_task_sleep(300);
        return -1;
    } else {
        while (p_file_seq->is_task_alive) {
            MLOGE("[%s] task alvie m_play_state[%d] is_task_alive[%d]\n",
                __func__, p_file_seq->m_play_state, p_file_seq->is_task_alive);
            mtos_task_sleep(300);
        }

        while (p_file_seq->is_load_task_alive) {
            MLOGE("[%s] load task alive m_play_state[%d] is_task_alive[%d]\n",
                __func__, p_file_seq->m_play_state, p_file_seq->is_task_alive);
            mtos_task_sleep(300);
        }
    }

    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    p_file_seq->seek_seconds  = 0.f;
    p_file_seq->m_seek_adj    = -1;
    p_file_seq->m_seek_cnt    = -1;
    p_file_seq->start_seconds = start_seconds;
    x_reset_cmd_fifo(p_file_seq);
    {
        p_file_seq->init_av_dev = MT_TRUE;
        x_init_av_device(p_file_seq);
        x_get_av_dec_cap(p_file_seq);
        p_file_seq->max_ves_size = p_file_seq->dec_cap.max_ves_num >> 10;//kBytes
    }

    p_fill_es_task = fill_es_task;
    if ((p_file_seq->p_demuxer && (DEMUXER_TYPE_AUDIO == ((demuxer_t *)(p_file_seq->p_demuxer))->type)) ||
        (p_file_seq->only_audio_mode && ((demuxer_t *)(p_file_seq->p_demuxer)) &&
        (DEMUXER_TYPE_RTP != ((demuxer_t *)(p_file_seq->p_demuxer))->type))) {
        MLOGI("[%s] this movie only include audio es !!!! type: %d\n, only_audio_mode:%d\n",
            __func__, ((demuxer_t *)(p_file_seq->p_demuxer))->type, p_file_seq->only_audio_mode);
        p_fill_es_task = fill_aud_es_task;
        ret = file_seq_av_decoder_init(p_file_seq);
        if (ret != 0) {
            mtos_task_sleep(300);
            mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
            return -1;
        }
    }

    if (p_file_seq->m_play_state == FILE_SEQ_PLAY ||
        p_file_seq->is_task_alive || p_file_seq->loadMedaiOK == MT_FALSE) {
        MLOGE("[%s] file seq state not stop! is_task_alive[%d] loadMedaiOK[%d] state[%d]\n",
            __func__, p_file_seq->is_task_alive, p_file_seq->loadMedaiOK, p_file_seq->m_play_state);
        mtos_task_sleep(300);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return -1;
    }

    if (p_fill_es_task == fill_es_task) {
        if (p_file_seq->p_vdec_dev) {
            if (fp_is_timeshift_file() == 1) {
                vdec_freeze_stop(p_file_seq->p_vdec_dev);
            } else {
                vdec_stop(p_file_seq->p_vdec_dev);
            }
        }
    }

    p_file_seq->force_stopped    = 0;
    p_file_seq->left_a_pkt_bytes = 0;
    p_file_seq->m_play_state     = FILE_SEQ_PLAY;
    pthread_t fill_es_thread;
    int err = -1;
    pthread_attr_t attribs;
    pthread_attr_init(&attribs);
    pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
    err = pthread_create(&fill_es_thread, &attribs, (void *)p_fill_es_task, (void *)p_handle);
    if (err != 0) {
        MLOGE("can't create fill es thread!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        exit(1);
    }

    while (p_file_seq->is_task_alive == MT_FALSE) {
        MLOGD("%s %d waiting ~~\n", __func__, __LINE__);
        mtos_task_sleep(50);
    }

    MLOGI("[%s] end end ...\n", __func__);
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    return 0;
}

static MT_BOOL  x_check_fillestask_alive_func(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    if(NULL == p_file_seq){
        return MT_FALSE;
    }

    if (p_file_seq->is_task_alive == MT_FALSE) {
        return MT_FALSE;
    } else {
        MLOGI("[%s] m_play_state %d is_task_alive %d\n", __func__,
            p_file_seq->m_play_state, p_file_seq->is_task_alive);
        return MT_TRUE;
    }
}

static int stop_player(FILE_SEQ_T *p_file_seq)
{
    MLOGI("[%s] state:%d\n", __func__, p_file_seq->m_play_state);
    if (p_file_seq->p_audio_dev) {
        aud_stop_vsb(p_file_seq->p_audio_dev);
    }

    if (p_file_seq->p_vdec_dev) {
        vdec_stop(p_file_seq->p_vdec_dev);
    }

    if (list_I_vpts) {
        list_rwlock_lock();
        destroy_list_vpts(list_I_vpts);
        list_rwlock_unlock();
        list_I_vpts = NULL;
    }
    x_clear_cmd_fifo(p_file_seq);
    if (p_file_seq->m_play_state == FILE_SEQ_PLAY  ||
        p_file_seq->m_play_state == FILE_SEQ_PAUSE ||
        p_file_seq->m_play_state == FILE_SEQ_LOADMEDIA) {

        p_file_seq->m_play_state = FILE_SEQ_EXIT;
        mp_ffmpeg_ext_cmd(MP_STATE_EXT, MP_DO_SET_PARAM, (void *)MT_TRUE);
        if (p_file_seq->total_path > 1) {
            p_file_seq->m_preload_state = FILE_SEQ_EXIT;
        }
    } else {
        if (NULL != p_file_seq->event_cb) {
            if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {
                MLOGI("[%s] drv send FILE_PLAYBACK_SEQ_FILL_ES_TASK_EXIT!!\n", __func__);
                p_file_seq->event_cb(FILE_PLAYBACK_SEQ_FILL_ES_TASK_EXIT, 0);
            } else if (p_file_seq->m_play_state == FILE_SEQ_STOP) {
                p_file_seq->event_cb(FILE_PLAYBACK_SEQ_FILL_ES_TASK_EXIT, 0);
            } else if (p_file_seq->m_play_state == FILE_SEQ_LOADED) {
                if (MT_TRUE == p_file_seq->loadMedaiOK) {
                    x_unloadmedia_func(p_file_seq);
                }
                p_file_seq->m_play_state = FILE_SEQ_EXIT;
                mp_ffmpeg_ext_cmd(MP_STATE_EXT, MP_DO_SET_PARAM, (void *)MT_TRUE);
                p_file_seq->event_cb(FILE_PLAYBACK_SEQ_FILL_ES_TASK_EXIT, 0);
            }
        }
        MLOGE("[%s] state[%d].\n", __func__, p_file_seq->m_play_state);
    }
    return MT_SUCCESS;
}

static int x_stop_func(void *p_handle)
{
    FILE_SEQ_T *p_file_seq =(FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }

    MLOGI("[%s] start start ...\n", __func__);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);

    p_file_seq->force_stopped = 0;
    (void) stop_player(p_file_seq);

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGI("[%s] stop stop ...\n", __func__);
    return 0;
}

static int x_force_stop_func(void *p_handle) {
    FILE_SEQ_T *p_file_seq =(FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }

    MLOGI("[%s] start start ...\n", __func__);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);

    p_file_seq->force_stopped = 1;
    (void) stop_player(p_file_seq);

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGI("[%s] stop stop ...\n", __func__);
    return 0;
}

static  int x_pause_func(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    MLOGI("[%s] start start ...\n", __func__);

    if (p_file_seq->m_play_state ==  FILE_SEQ_PLAY) {

        if (p_file_seq->is_support_seek_finish) {
            if (p_file_seq->cur_speed) {
                p_file_seq->cur_speed  = TS_SEQ_NORMAL_PLAY;
            }
        }

        if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
            aud_pause_vsb(p_file_seq->p_audio_dev);
        }

        if (p_file_seq->only_audio_mode == MT_FALSE) {
            vdec_pause(p_file_seq->p_vdec_dev);
        }

        p_file_seq->m_play_state = FILE_SEQ_PAUSE;
    } else {
        MLOGI("[%s] now state is not FILE_SEQ_PLAY..., m_play_state [%d]!!!\n", __func__, p_file_seq->m_play_state);
        MLOGI("[%s] do nothing...\n", __func__);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return 0;
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGI("[%s] stop stop ...\n", __func__);
    return 0;
}

static  int x_resume_func(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    MLOGI("[%s] start start ...\n", __func__);

    if (p_file_seq->m_play_state ==  FILE_SEQ_PAUSE) {
        if (p_file_seq->only_audio_mode == MT_FALSE) {
            vdec_resume(p_file_seq->p_vdec_dev);
        }

        mtos_task_sleep(10);

        if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
            aud_resume_vsb(p_file_seq->p_audio_dev);
        }

        p_file_seq->m_play_state = FILE_SEQ_PLAY;
    } else {
        MLOGI("[%s] do nothing...\n", __func__);
        MLOGI("[%s] the last state is not play ...!!!!\n", __func__);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return 0;
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGI("[%s] stop stop ...\n", __func__);
    return 0;
}

static   int x_set_speed_func(void *p_handle, int speed) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
	int tplay_1xnum = get_tplay_normal_play_num();
    if(NULL == p_file_seq){
        return -1;
    }
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    MLOGI("[%s] speed:%d start start ...\n", __func__, speed);
    if ((speed >= (-64)  && speed <= 64)|| tplay_1xnum == 1) {
        p_file_seq->tmp_speed = speed;
    } else {
        p_file_seq->tmp_speed = speed / tplay_1xnum;
    }

    /*
     *    ensure that no 'CMD_CHANGE_SPEED' to be processed !!!!
     */
    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_SPEED)) {
        do {
            mtos_task_sleep(10);

            if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_SPEED) == MT_FALSE) {
                break;
            }
        } while (0);
    }

    SetFlag(p_file_seq->m_user_cmd, CMD_CHANGE_SPEED);
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGI("[%s] stop stop ...\n", __func__);
    return 0;
}

static void x_wait_trickplay_finish_func(void *p_handle, int timeout) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }
    MT_U32 start_tick = mtos_ticks_get();
    while(IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_SPEED)) {
        if (mtos_ticks_get() > (start_tick + timeout / 10)) {
            MLOGI("seek wait timeout!\n");
            break;
        }

        mtos_task_sleep(10);
    }
    MLOGI("[%s] exit ...\n", __func__);
}

static void x_get_subt_info_func(void *p_handle, void **subt_info)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }
    MT_ASSERT(p_file_seq != NULL);
    if (p_file_seq->p_demuxer) {
        *(file_seq_video_subtitle_t **)subt_info = (void *) & (((demuxer_t *)(p_file_seq->p_demuxer))->subt_info);
    }
}

int file_seq_handle_subtitle_packet(FILE_SEQ_T *p_file_seq)
{
    int ret = 0;

    if(!p_file_seq) {
        return -1;
    }
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    ret = p_file_seq->subt_id >= 0;
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));

    return ret;
}

static void x_set_subt_id_func(void *p_handle, int subt_id) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    MT_ASSERT(p_file_seq != NULL);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    MLOGD("[%s] start start ...subt_id:%d\n", __func__, subt_id);
    /* ensure that no 'CMD_CHANGE_AUDIO_TRACK' to be processed !!!! */
    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_SUBT_ID)) {
        MLOGD("[%s] ----------- it is already changing subt id now, do nothing!\n", __func__);
    } else {
        if (p_file_seq->subt_id != subt_id) {
            /* Not allow sub before, thus without pts */
            if ((int64_t) (p_file_seq->orig_spts) == 0) {
                drv_pts_info_t vstate = {0};
                vdec_get_pts(p_file_seq->p_vdec_dev, &vstate);
                p_file_seq->orig_spts = vstate.pts;
            }
            SetFlag(p_file_seq->m_user_cmd, CMD_CHANGE_SUBT_ID);
            p_file_seq->subt_id = subt_id;
        }
    }
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGD("[%s] end end ...\n", __func__);
}

static int x_get_subt_func(void * p_handle, unsigned char ** start, int * pts) {
    int size_sub = 0;
    int size = 0;
    char *p_data = NULL;//tizhang@20180822 for 104045
    FILE_SEQ_T * p_file_seq = (FILE_SEQ_T *)p_handle;
    if(NULL == p_file_seq){
        return -1;
    }

    if (p_file_seq->is_task_alive == FALSE || p_file_seq->p_sub_fifo_handle == NULL
        || p_file_seq->is_play_to_end > 0) {

	    if (p_file_seq->p_sub_fifo_handle) {
            void * p_start_pos = NULL;
            p_start_pos = (void*)((fifo_type_t *)(p_file_seq->p_sub_fifo_handle))->start_pos;
            if(p_start_pos)
            {
                mtos_free(p_start_pos);
                ((fifo_type_t *)(p_file_seq->p_sub_fifo_handle))->start_pos = 0;
            }
            deinit_fifo_kw(p_file_seq->p_sub_fifo_handle);
            p_file_seq->p_sub_fifo_handle = NULL;
        }

        return 0;
    }

    mtos_sem_take((os_sem_t *)(&(p_file_seq->sub_fifo_mutex)), 0);
    //*start =  p_file_seq->subt_buf;
    p_data = p_file_seq->subt_buf_extra;
    if(p_data){
        memset(p_data,0,SUBT_BUF_SIZE_MAX);
        size = read_sub_fifo_kw(p_file_seq->p_sub_fifo_handle, p_data);

        if (size == 0) {
            mtos_sem_give((os_sem_t *)(&(p_file_seq->sub_fifo_mutex)));
            return 0;
        } else {
            size_sub = (p_data[0] << 24) + (p_data[1] << 16) + (p_data[2] << 8) + p_data[3];
            *pts = (p_data[4] << 24) + (p_data[5] << 16) + (p_data[6] << 8) + p_data[7];

            if (size_sub > SUBT_BUF_SIZE_MAX - 8) {
                size_sub = SUBT_BUF_SIZE_MAX - 8;
            }

            *start = (unsigned char *)(p_data + 8);
            //memcpy(*start, (p_data + 8), size_sub);
        }
    }
    else{
        size_sub = 0;
        *start = NULL;
        *pts = -1;
    }
    mtos_sem_give((os_sem_t *)(&(p_file_seq->sub_fifo_mutex)));
    return size_sub;
}

static  unsigned long int x_get_cur_vpts_func(void *p_handle) {
    unsigned long int vpts = 0;
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);

    if (p_file_seq->is_task_alive) {

        drv_pts_info_t vstate;
        vdec_get_pts(p_file_seq->p_vdec_dev, &vstate);
        vpts = vstate.pts;

        if (p_file_seq->cur_speed == TS_SEQ_NORMAL_PLAY) {
            if (vpts < g_last_get_vpts) {
                //50ms = 50 * 45 = 2250
                if ((g_last_get_vpts - vpts) < 22500) {
                    vpts = g_last_get_vpts;
                } else {
                    g_last_get_vpts = vpts;
                }
            } else {
                g_last_get_vpts = vpts;
            }
        }
    } else {
        MLOGD("[%s] do nothing ...!!!\n", __func__);
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    //MLOGD("[%s] stop stop ...\n",__func__);
    return vpts;
}

static  int x_playAtTime_func(void *p_handle, int sec)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);

    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    if (pbi->need_async_done_msg) {
        MLOGW("Dash/HLS async open not complete, please try again later\n");
        return 0;
    }

    MLOGI("[%s] start start... sec %ds\n", __func__, sec);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    /*
     * ensure that no 'CMD_CHANGE_SPEED' to be processed !!!!
     */
    if (IS_SET(p_file_seq->m_user_cmd, CMD_PLAY_AT_TIME)) {
        //yliu modify for nonblock
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return 0;
    }

    FP_USER_CMD_T cmd;
    memset(&cmd, 0, sizeof(FP_USER_CMD_T));
    cmd.type = CMD_PLAY_AT_TIME;
    cmd.param = sec;

    if (x_push_user_cmd(p_file_seq, &cmd) == 0) {
        p_file_seq->is_play_at_time = 1;

        if (p_file_seq->total_path > 1) {
            p_file_seq->seek_seconds = sec * 1.0;
        } else {
            p_file_seq->seek_seconds = sec * 1.0;
            if (p_file_seq->duration - 5 > 0.001) {
            	p_file_seq->seek_seconds = min(sec * 1.0, p_file_seq->duration - 5);    //-p_file_seq->vpts_upload*1.f/1000*1.f/TIME_BASE;
            }
        }
    }

    MLOGD("[%s] --------play at time: next %d seconds\n", __func__, (int)sec);
    MLOGD("[%s] stop stop ...\n", __func__);
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    return 0;
}

static void x_wait_seek_finish_func(void *p_handle, int timeout) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return -1;
    }
    MT_U32 start_tick = mtos_ticks_get();
    while(p_file_seq->is_play_at_time) {
        if (mtos_ticks_get() > (start_tick + timeout / 10)) {
            MLOGI("seek wait timeout!\n");
            break;
        }

        if (p_file_seq->is_task_alive) {
            do_user_cmd();
            break;
        }

        mtos_task_sleep(10);
    }
    MLOGD("[%s] exit ...\n", __func__);
}

static FILE_SEQ_STATUS x_get_status_func(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);
    return p_file_seq->m_play_state;
}

static int x_change_audio_track_func(void *p_handle, int trackId) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    int ret = 0;
    MT_ASSERT(p_file_seq != NULL);
    //demuxer_switch_audio(p_file_seq->p_demuxer,trackId);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    MLOGD("[%s] start start ...\n", __func__);

    /* ensure that no 'CMD_CHANGE_AUDIO_TRACK' to be processed !!!! */
    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_AUDIO_TRACK)) {
        MLOGW("[%s] It is already changing audio track now, do nothing!\n", __func__);
        ret = 1;//haven't processed this command.
    } else {
        SetFlag(p_file_seq->m_user_cmd, CMD_CHANGE_AUDIO_TRACK);
        p_file_seq->audio_track_id = trackId;
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGD("[%s] end end ...\n", __func__);
    return ret;
}

static int x_get_audio_track_lang_func(void *p_handle, TRACK_LANG *lang_array) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    MT_ASSERT(p_file_seq != NULL);
    demuxer_t *p_demuxer = (demuxer_t *)(p_file_seq->p_demuxer);
    sh_audio_t *sh = NULL;
    TRACK_LANG lang_array_tmp[MAX_A_STREAMS];
    char *lang = NULL;
    char *title = NULL;

    MLOGD("[%s] start start ...\n", __func__);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    memset(lang_array_tmp, 0, MAX_A_STREAMS * sizeof(TRACK_LANG));
    if (!p_demuxer) {
        goto done;
    }

    int i, track_num = -1;
    for (i = 0; i < MAX_A_STREAMS; i++) {
        sh = p_demuxer->a_streams[i];
        if (!sh) {
            continue;
        }

        if (DEMUXER_TYPE_MPEG_TS == ((demuxer_t *)(p_file_seq->p_demuxer))->type) {
            if (!ds_ts_check_audio_aid(p_demuxer, sh->aid)) {
                continue;
            }
        }
        ++track_num;
        if (track_num < p_file_seq->audio_track_num) {
            continue;
        }
        if (sh->lang) {
            lang = mtos_malloc(strlen(sh->lang) + 1);
            memset(lang, 0, strlen(sh->lang) + 1);
            strcpy(lang, sh->lang);
            MLOGD("[%s] -----------  for 1: audio track lang %d: %s\n", __func__, i, lang);
            lang_array_tmp[track_num].lang = lang;
        }
        if (sh->title) {
            //title = strdup(sh->title);
            title = mtos_malloc(strlen(sh->title) + 1);
            memset(title, 0, strlen(sh->title) + 1);
            strcpy(title, sh->title);
            lang_array_tmp[track_num].title = title;
            MLOGD("[%s] -----------  for 1: audio track title %d: %s\n", __func__, i, sh->title);
        }

        int pcm_be, audio_pid;
        unsigned int acodec_id, m_audio_codec_type;
        (void) ds_get_audio_codec_info(p_file_seq->p_ds_audio,
            &m_audio_codec_type, &pcm_be, &acodec_id, &audio_pid, p_file_seq->audio_output_mode);
        lang_array_tmp[track_num].track_id = i;
        if (aAUDIO_MP2 == acodec_id && HA_AUDIO_ID_MP3 == m_audio_codec_type) {
            m_audio_codec_type = HA_AUDIO_ID_MP2;
        }
        lang_array_tmp[track_num].format = m_audio_codec_type;
        MLOGD("[%s] -----------  aid:  %d\n", __func__, sh->aid);
        MLOGD("[%s] -----------  audio format:  0x%x acodec :0x%x \n\n", __func__, sh->format, acodec_id);
    }

    track_num++;
    if (track_num <= p_file_seq->audio_track_num) {
        goto done;
    }

    p_file_seq->audio_lang_array = (TRACK_LANG *) mtos_realloc(
        p_file_seq->audio_lang_array, sizeof(TRACK_LANG) * track_num);
    if (!p_file_seq->audio_lang_array) {
        goto done;
    }

    for (i = p_file_seq->audio_track_num; i < track_num; i++) {
        p_file_seq->audio_lang_array[i].track_id = lang_array_tmp[i].track_id;
        p_file_seq->audio_lang_array[i].lang = lang_array_tmp[i].lang;
        p_file_seq->audio_lang_array[i].title = lang_array_tmp[i].title;
        p_file_seq->audio_lang_array[i].format = lang_array_tmp[i].format;
    }

    p_file_seq->audio_track_num = track_num;
done:
    MLOGD("[%s] -----------  audio track (with lang) num: %d\n", __func__, p_file_seq->audio_track_num);
    if (lang_array && p_file_seq->audio_track_num > 0 && p_file_seq->audio_lang_array) {
        for (i = 0; i < p_file_seq->audio_track_num; i++) {
            lang_array[i].track_id = p_file_seq->audio_lang_array[i].track_id;
            lang_array[i].lang = p_file_seq->audio_lang_array[i].lang;
            lang_array[i].title = p_file_seq->audio_lang_array[i].title;
            lang_array[i].format = p_file_seq->audio_lang_array[i].format;
            MLOGD("[%s] -----------  for 2: audio track %d: %s %s 0x%x\n", __func__,
                lang_array[i].track_id, lang_array[i].lang, lang_array[i].title, lang_array[i].format);
        }
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    MLOGD("[%s] end end ...\n", __func__);
    return 0;
}

static int x_change_video_track_func(void *p_handle, int trackId)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    // TODO: liuyong  implement in future
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    return 0;
}

static MT_BOOL x_get_film_info_func(void *p_handle, FILM_INFO_T *pResult) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    FILM_INFO_T  film_info;
    MT_ASSERT(p_file_seq != NULL);
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    memset(&film_info, 0, sizeof(FILM_INFO_T));
    if (pResult) {
        memset(pResult, 0x00, sizeof(FILM_INFO_T));
    }
    if (p_file_seq->m_path[0] == NULL) {
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return MT_FALSE;
    }
    if (!(p_file_seq->checkAvTypeOK && p_file_seq->loadMedaiOK)) {
        MLOGD("[%s] fail to check avypte and load medai!!!\n", __func__);
        MLOGD("do nothing !!!\n");
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        return MT_FALSE;
    }

    memset(&film_info, 0, sizeof(FILM_INFO_T));
    if (p_file_seq->total_path > 1) {
        int j = 0;

        for (j = 0; j < p_file_seq->total_path; j++) {
            film_info.film_duration += p_file_seq->p_m_duration[j] * 1000;
        }
    } else {
        {
            stream_t   *fp_stream = (stream_t *)(p_file_seq->p_stream);
            if (fp_stream == NULL) {
                mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
                return MT_FALSE;
            }
            film_info.film_duration = (int)(p_file_seq->file_duration * 1000);
        }
    }

    film_info.file_size        = p_file_seq->file_size;
    film_info.audio_type       = p_file_seq->m_audio_codec_type;
    film_info.video_type       = p_file_seq->m_video_codec_type;
    film_info.audio_track_num  = ds_get_audio_count(p_file_seq->p_demuxer);
    film_info.video_track_num  = ds_get_video_count(p_file_seq->p_demuxer);
    film_info.canTrickPlay     = MT_TRUE;
    film_info.video_disp_w     = p_file_seq->video_disp_w;
    film_info.video_disp_h     = p_file_seq->video_disp_h;
    film_info.video_bps        = p_file_seq->video_bps;
    film_info.video_fps        = p_file_seq->video_fps;
    film_info.audio_track_id   = p_file_seq->audio_track_id;
    film_info.audio_bps        = p_file_seq->audio_bps;
    film_info.audio_samplerate = p_file_seq->audio_samplerate;
    film_info.file_name        = p_file_seq->m_path[p_file_seq->cur_play_index];
    film_info.audio_language   = p_file_seq->audio_language;

    if (pResult) {
        memcpy(pResult, &film_info, sizeof(FILM_INFO_T));
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    return MT_TRUE;
}

static void x_set_ott_playmode_func(void *p_handle, int playmode)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return ;
    }
    p_file_seq->ott_playmode = playmode;
    MLOGD("[%s] ott_playmode[%d],playmode[%d]\n", __func__, p_file_seq->ott_playmode, playmode);
}

static void get_av_ts_pid(char *buf) {
    char *pV = NULL;
    char *pA = NULL;
    char *p = NULL;
    char data[4];
    char len;
    unsigned char i;
    pV = strstr(buf, "vpid:");

    if (!pV) {
        return;
    }

    pA = strstr(buf, "apid:");

    if (!pA) {
        return;
    }

    memset(data, 0, 4);
    len = pA - pV - 5 - 2;

    if (len > 3) {
        return;
    }

    p = pV + 5;

    for (i = 0; i < len; i++) {
        data[i] = p[i];
    }

    memset(data, 0, 4);
    len = (buf + strlen(buf)) - pA - 5;

    if (len > 3) {
        return;
    }

    p = pA + 5;

    for (i = 0; i < len; i++) {
        data[i] = p[i];
    }

    return;
}

static void x_set_path_ex_func(void *p_handle, char *p_play_path, u32 path_id)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return ;
    }
    char *p_path = p_play_path;
    char *tmp_buf = NULL;

    MLOGI("[%s] start start ...\n", __func__);
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    pbi->need_async_done_msg = 0;
    if(NULL == p_file_seq->pb_internal){
        return ;
    }

    ((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal))->is_timeshift = 0;
    //play mode
    x_set_ott_playmode_func(p_file_seq, path_id);

    if (p_path && strstr(p_path, ".mont_rtmp")) {   // for tsscan
        MLOGI("[mont_rtmp]THIS FILE SHOULD BE REDIRECTED !!!!!\n");

        FILE *video_fp = NULL;
        video_fp = fopen(p_path, "rb");
        if (video_fp == NULL) {
            MLOGD("[%s] fopen file error\n", __func__);
            return ;
        }

        MLOGI("open[%s] OK\n", p_path);
        tmp_buf = (char *)mtos_malloc(1024);
        if (tmp_buf == NULL) {
            MLOGD("[%s] mtos_malloc tmp_buf error\n", __func__);
            fclose(video_fp);
            return;
        }

        memset(tmp_buf, 0, 1024);
        fread(tmp_buf, 1, 1024, video_fp);
        fclose(video_fp);
        get_av_ts_pid(tmp_buf);
        p_path = strstr(tmp_buf, "vpid:");
        if (p_path) {
            p_path -= 2;
            memset(p_path, 0, 1024 - (p_path - tmp_buf));
        }

        p_path = tmp_buf;
    }

    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    if (p_path && strlen(p_path) < LOCAL_FILE_PATH_LEN) {

        if (p_file_seq->m_path[0] == NULL) {
            p_file_seq->m_path[0] = mtos_malloc(LOCAL_FILE_PATH_LEN);
        }

        memset(p_file_seq->m_path[0], 0, LOCAL_FILE_PATH_LEN);
        strcpy(p_file_seq->m_path[0], p_path);

        p_file_seq->path_id = path_id;
        p_file_seq->total_path = 1;
    } else {
        MLOGE("[%s][ERROR] fail to set file path !!!\n", __func__);
    }

    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    if (x_isNetStream_func(p_file_seq) == MT_FALSE) {
        io_stream_buffer_size = 256 * 1024;
        io_isnetworkstream = 0;
        MLOGI("[%s] %d not network stream\n", __func__, __LINE__);
    } else {
        io_stream_buffer_size = 64 * 1024;
        io_isnetworkstream = 1;
        MLOGI("[%s] %d is network stream\n", __func__, __LINE__);
    }

    if (tmp_buf) {
        mtos_free(tmp_buf);
        tmp_buf = NULL;
    }

    MLOGI("[%s] end end ...\n", __func__);
    return;
}

static void x_set_path_func(void *p_handle, char *p_play_path) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    if(NULL == p_file_seq){
        return ;
    }

    fpi_init_codec_blacklist(p_file_seq->pb_internal);
    x_set_path_ex_func(p_file_seq,p_play_path,0);
    return;
}

static void x_register_cb_func(void *p_handle, void *p_cb) {
    MLOGD("[%s] start start ...\n", __func__);
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return;
    }
    p_file_seq->event_cb = p_cb;
    MLOGI("Register event cb %p\n", p_cb);
}

static void x_set_tv_system_func(void *p_handle, MT_BOOL auto_set_tv_sys) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if(NULL == p_file_seq){
        return ;
    }
    p_file_seq->tv_sys_auto_set = auto_set_tv_sys;
}

static FILE_SEQ_CHIPTYPE get_chip_type(void) {
    FILE_SEQ_CHIPTYPE chip_type = 0;
    mt_sys_version_s chip_ver;
    mt_s32 ret;
    chip_type = CHIPTYPE_BUFF;
    ret = mt_sys_get_version(&chip_ver);
	if (mt_chip_is_symphony1(chip_ver.enChipVersion) ||
		chip_ver.enChipVersion == MT_CHIP_SYMPHONY3_A0) {
		chip_type = CHIPTYPE_SYMPHONY1;
	}
	else if (mt_chip_is_symphony2(chip_ver.enChipVersion)) {
		chip_type = CHIPTYPE_SYMPHONY2;
	} else if (mt_chip_is_symphony4(chip_ver.enChipVersion)) {
		chip_type = CHIPTYPE_SYMPHONY4;
	} else if (mt_chip_is_symphony6(chip_ver.enChipVersion)) {
		chip_type = CHIPTYPE_SYMPHONY6;
	}
	else {
		chip_type = CHIPTYPE_BUFF;
	}

    //MLOGD("%s %d chip type:%d_%d\n", __func__, __LINE__, chip_type, ret);
    MLOGD("%s %d chip type:%d_%d\n", __func__, __LINE__, chip_type, ret);
    return chip_type;
}

int file_seq_get_audio_pid(
    FILE_SEQ_T *p_file_seq, const int id_num, int *pid_list) {
    if (NULL == p_file_seq || NULL == pid_list) {
        MLOGE("%s para error\n", __FUNCTION__);
        return MT_FAILURE;
    }

    if (!(p_file_seq->is_ts)) {
        MLOGE("Not ts file, no audio pid\n", __FUNCTION__);
        return MT_FAILURE;
    }

    return ds_get_audio_pid(p_file_seq->p_ds_audio, p_file_seq->ts_priv, id_num, pid_list);
}

int file_seq_get_video_pid(
    FILE_SEQ_T *p_file_seq, const int id_num, int *pid_list) {
    if (NULL == p_file_seq || NULL == pid_list) {
        MLOGE("%s para error\n", __FUNCTION__);
        return MT_FAILURE;
    }

    if (!(p_file_seq->is_ts)) {
        MLOGE("Not ts file, no video pid\n", __FUNCTION__);
        return MT_FAILURE;
    }
    return ds_get_video_pid(p_file_seq->p_ds_video, p_file_seq->ts_priv, id_num, pid_list);
}

int file_seq_get_network_bitrate(
    FILE_SEQ_T *p_file_seq, long long *bitrate)
{
    if (!p_file_seq || !bitrate || !p_file_seq->p_demuxer || p_file_seq->loadMedaiOK != MT_TRUE ||
        (FILE_SEQ_PAUSE != p_file_seq->m_play_state && FILE_SEQ_PLAY != p_file_seq->m_play_state)) {
        return MT_FAILURE;
    }

    return ds_get_network_bitrate(p_file_seq->p_ds_video, bitrate);
}

int file_seq_get_playlist(FILE_SEQ_T *p_file_seq)
{
    if (!p_file_seq || !p_file_seq->p_demuxer || p_file_seq->loadMedaiOK != MT_TRUE ||
        (FILE_SEQ_PAUSE != p_file_seq->m_play_state && FILE_SEQ_PLAY != p_file_seq->m_play_state)) {
        return MT_FAILURE;
    }

    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    int ret = fpi_get_playlist(p_file_seq);
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    return ret;
}

int file_seq_switch_playlist(FILE_SEQ_T *p_file_seq, int playlist_index)
{
    if (!p_file_seq || playlist_index < 0 ||
        !p_file_seq->p_demuxer || p_file_seq->loadMedaiOK != MT_TRUE ||
        (FILE_SEQ_PAUSE != p_file_seq->m_play_state && FILE_SEQ_PLAY != p_file_seq->m_play_state)) {
        return MT_FAILURE;
    }

    int ret = MT_FAILURE;
    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_PLAYLIST)) {
        MLOGW("[%s] Player is changing playlist now, do nothing!\n", __func__);
    } else {
        SetFlag(p_file_seq->m_user_cmd, CMD_CHANGE_PLAYLIST);
        ret = fpi_switch_playlist(p_file_seq, playlist_index);
    }
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    return ret;
}

int file_seq_set_extio_data_source(
    FILE_SEQ_T *p_file_seq, FILE_PLAYBACK_EXTIO_CONTEXT_T *extio)
{
    if(!p_file_seq || !p_file_seq->pb_internal || !extio){
        MLOGE("[%p] Set extio data source %p fail!\n", p_file_seq, extio);
        return MT_FAILURE;
    }

    MLOGI("[%s] start start ...\n", __func__);
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *) p_file_seq->pb_internal;
    pbi->is_timeshift = 0;
    x_set_ott_playmode_func(p_file_seq, 0);

    mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
    p_file_seq->path_id = 0;
    p_file_seq->total_path = 1;
    if (!pbi->extio_ctx) {
        pbi->extio_ctx = mlzp_malloc(
            (unsigned int) sizeof(FILE_PLAYBACK_EXTIO_CONTEXT_T));
    }
    if (pbi->extio_ctx) {
        memcpy(pbi->extio_ctx, extio,
            sizeof(FILE_PLAYBACK_EXTIO_CONTEXT_T));
    }
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));

    MLOGI("[%s] end end, ioctx:%p ...\n", __func__, pbi->extio_ctx);
    return MT_SUCCESS;
}

int file_seq_cfg_stream_probe_para(FILE_SEQ_T *p_file_seq,
    int max_stream_probe_size, int max_stream_analyze_duration)

{
    if(!p_file_seq || !p_file_seq->pb_internal ||
        max_stream_probe_size <= 0 || max_stream_analyze_duration <= 0){
        MLOGE("[%p] Cfg stream probe para fail!\n", p_file_seq);
        return MT_FAILURE;
    }

    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *) p_file_seq->pb_internal;

    pbi->max_stream_probe_size       = max_stream_probe_size;
    pbi->max_stream_analyze_duration = max_stream_analyze_duration;
    return MT_SUCCESS;
}

int file_seq_set_network_parameter(
    FILE_SEQ_T *p_file_seq, FILE_PLAYBACK_NETWORK_PARA_T *para)
{
    if(!p_file_seq || !p_file_seq->pb_internal || !para) {
        MLOGE("[%p] Network parameter para error!\n", p_file_seq);
        return MT_FAILURE;
    }

    destroy_ffmpeg_dict_opts(p_file_seq);
    if (para->headers) {
        set_ffmpeg_dict_opts(p_file_seq, "headers", para->headers, 0);
    }
    if (para->user_agent) {
        set_ffmpeg_dict_opts(p_file_seq, "user_agent", para->user_agent, 0);
    }
    if (para->cookies) {
        set_ffmpeg_dict_opts(p_file_seq, "cookies", para->cookies, 0);
    }

    return MT_SUCCESS;
}

static void create_player_lock(
    FILE_SEQ_T *p_file_seq, PLAYBACK_INTERNAL_T *p_pb_internal)
{
    MT_BOOL ret = MT_FALSE;

    ret = mtos_sem_create((os_sem_t *)(&(p_pb_internal->seek_mutex)), MT_TRUE);
    MT_ASSERT(ret == MT_TRUE);

    ret = mtos_sem_create((os_sem_t *)(&(p_file_seq->lock)), MT_TRUE);
    MT_ASSERT(ret == MT_TRUE);

    ret = mtos_sem_create((os_sem_t *)(&(p_file_seq->heap_lock)), MT_TRUE);
    MT_ASSERT(ret == MT_TRUE);

    ret = mtos_sem_create((os_sem_t *)(&(p_file_seq->sub_fifo_mutex)), MT_TRUE);
    MT_ASSERT(ret == MT_TRUE);

    ret = list_rwlock_init();
    MT_ASSERT(ret == MT_TRUE);

    ret = mtos_sem_create((os_sem_t *)(&g_mplayer_handle_sem_lock), MT_TRUE);
    MT_ASSERT(ret == MT_TRUE);

    ret = mlzp_mutex_init(&(p_pb_internal->video.mutex));
    MT_ASSERT(0 == ret);
}

static void destroy_player_lock(
    FILE_SEQ_T *p_handle, PLAYBACK_INTERNAL_T *p_pb_internal) {
    FILE_SEQ_T *p_file_seq = p_handle;
    if(NULL == p_file_seq){
        return ;
    }

    list_rwlock_deinit();
    mtos_sem_destroy((os_sem_t *)(&(p_file_seq->sub_fifo_mutex)), 0);
    mtos_sem_destroy((os_sem_t *)(&(p_file_seq->lock)), 0);
    mtos_sem_destroy((os_sem_t *)(&(p_file_seq->heap_lock)), 0);
    mtos_sem_destroy((os_sem_t *)(&(p_pb_internal->seek_mutex)), 0);
    (void) mlzp_mutex_destroy(&(p_pb_internal->video.mutex));
}

void *file_seq_mem_alloc(void *p_handle, u32 size) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);
    void *p_buf = NULL;

    if (p_file_seq->use_ext_heap) {
        mtos_sem_take((os_sem_t *)(&(p_file_seq->heap_lock)), 0);
        p_buf = lib_memp_alloc(p_file_seq->p_ext_heap_hdl, size);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->heap_lock)));
    } else {
        p_buf = malloc(size);
    }

    //MT_ASSERT(p_buf != NULL);
    if (p_buf == NULL) {
        MLOGE("[%s] malloc failed!!!!\n", __func__);
        x_force_stop_func(p_file_seq);
        p_file_seq->event_cb(FILE_PLAYBACK_NOT_ENOUGH_MEMORY, 0);
    }
    return p_buf;
}

void file_seq_mem_free(void *p_handle, void *p_buf) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    MT_ASSERT(p_file_seq != NULL);

    if (p_file_seq->use_ext_heap) {
        mtos_sem_take((os_sem_t *)(&(p_file_seq->heap_lock)), 0);
        lib_memp_free(p_file_seq->p_ext_heap_hdl, p_buf);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->heap_lock)));
    } else {
        free(p_buf);
    }
}

int fp_set_codec_blacklist(FILE_SEQ_T *p_file_seq,
                           STREAM_TYPE_E stream_type, int num, int *list) {
    if(!p_file_seq || !list) {
        return MT_FAILURE;
    }
    return fpi_set_codec_blacklist(p_file_seq->pb_internal, stream_type, num, list);
}

void *file_seq_create(PB_SEQ_PARAM_T *p_param) {
    FILE_SEQ_T *p_file_seq           = NULL;
    sys_mem_debug_t start_mem_info = {0};
    PLAYBACK_INTERNAL_T *p_pb_internal = NULL;

    MLOGI("[%s] start start ...!!!!\n", __func__);
    if (p_param) {
        MLOGD("[%s] vdec_policy[%ld] ...\n", __func__, p_param->vdec_policy);
    } else {
        MLOGE("[%s] do nothing ...\n", __func__);
        MT_ASSERT(0);
        return NULL;
    }

    show_sys_memory_info(&start_mem_info, NULL, MT_FALSE);
    p_file_seq      = (FILE_SEQ_T *) mtos_malloc(sizeof(FILE_SEQ_T));
    p_pb_internal = (PLAYBACK_INTERNAL_T *) mtos_malloc(sizeof(PLAYBACK_INTERNAL_T));
    if (p_file_seq) {
        if (NULL == p_pb_internal) {
            mtos_free(p_file_seq);
            p_file_seq = NULL;
            MLOGE("[%s] Fail to malloc PLAYBACK_INTERNAL_T!!!\n", __func__);
            return NULL;
        }

        memset(p_file_seq, 0, sizeof(FILE_SEQ_T));
        memset(p_pb_internal, 0, sizeof(PLAYBACK_INTERNAL_T));

        p_file_seq->pb_internal          = (void *)p_pb_internal;
        p_pb_internal->chip_type       = get_chip_type();
        p_pb_internal->task_idle       = p_param->task_idle;
        p_pb_internal->start_mem_info  = start_mem_info;
        p_pb_internal->net_buffer.is_open = MT_FALSE;
        p_pb_internal->media_load_start_time = -1;
        p_pb_internal->transport_desdec_ctx  = p_param->transport_desdec_ctx;
        p_pb_internal->transport_desdec_func = p_param->transport_desdec_func;

        p_file_seq->cur_speed            = 0;
        p_file_seq->audio_output_mode    = p_param->audio_output_mode;
        p_file_seq->vdec_policy          = p_param->vdec_policy;
        p_file_seq->loadmedia            = NULL;
        p_file_seq->loadmedia_task       = x_loadmedia_func;
        p_file_seq->loadmedia_times      = NULL;
        p_file_seq->unloadmedia          = x_unloadmedia_func;
        p_file_seq->register_event_cb    = x_register_cb_func;
        p_file_seq->start                = x_start_func;
        p_file_seq->stop                 = x_stop_func;
        p_file_seq->force_stop           = x_force_stop_func;
        p_file_seq->pause                = x_pause_func;
        p_file_seq->set_speed            = x_set_speed_func;
        p_file_seq->wait_tplay_finish    = x_wait_trickplay_finish_func;
        p_file_seq->resume               = x_resume_func;
        p_file_seq->check_bg_task_alive  = x_check_fillestask_alive_func;
        p_file_seq->set_file_path        = x_set_path_func;
        p_file_seq->set_file_path_ex     = x_set_path_ex_func;

        p_file_seq->get_film_info        = x_get_film_info_func;
        p_file_seq->change_video_track   = x_change_video_track_func;
        p_file_seq->change_audio_track   = x_change_audio_track_func;
        p_file_seq->get_audio_track_lang = x_get_audio_track_lang_func;
        p_file_seq->mp_dump              = x_mp_dump;
        p_file_seq->mp_dumpaudio         = x_mp_dump_aud;
        p_file_seq->get_status           = x_get_status_func;
        p_file_seq->get_vpts             = x_get_cur_vpts_func;
        p_file_seq->get_subt             = x_get_subt_func;
        p_file_seq->get_subt_info        = x_get_subt_info_func;
        p_file_seq->set_subt_id          = x_set_subt_id_func;
        p_file_seq->play_at_time         = x_playAtTime_func;
        p_file_seq->set_tv_sys           = x_set_tv_system_func;
        p_file_seq->set_ott_playmode     = x_set_ott_playmode_func;
        p_file_seq->is_network_stream    = x_isNetStream_func;
        p_file_seq->set_live_broadcast   = x_set_live_broadcast_func;
        p_file_seq->wait_seek_finish     = x_wait_seek_finish_func;
        p_file_seq->get_mp_tag           = x_get_mp_tag_func;
        p_file_seq->set_support_seek_finish = x_set_support_seek_finish;
        p_file_seq->set_network_buffering   = x_set_network_buffering;

        /*initialize method for allocate/free memory */
        MLOGI("Set player memory addr %p size 0x%x\n",
            p_param->pb_seq_mem_start,  p_param->pb_seq_mem_size);
        if (p_param->pb_seq_mem_start > 0 && p_param->pb_seq_mem_size > (4 * 1024 * 1024)) {
            p_file_seq->mem_init           = file_seq_mem_init;
            p_file_seq->mem_alloc          = file_seq_mem_alloc;
            p_file_seq->mem_free           = file_seq_mem_free;
            p_file_seq->mem_release        = file_seq_mem_release;
            p_file_seq->p_ext_heap_hdl     = (lib_memp_t *) mtos_malloc(sizeof(lib_memp_t));
            if (NULL == p_file_seq->p_ext_heap_hdl) {
                mtos_free(p_file_seq);
                mtos_free(p_pb_internal);
                p_file_seq = NULL;
                p_pb_internal = NULL;
                MLOGE("[%s] Fail to malloc ext_heap_hdl!\n", __func__);
                return NULL;
            }
            p_file_seq->file_seq_mem_size  = p_param->pb_seq_mem_size;
            p_file_seq->file_seq_mem_start = p_param->pb_seq_mem_start;

            memset(p_file_seq->p_ext_heap_hdl, 0, sizeof(lib_memp_t));
            MLOGI("Use External Heap allocated by UI\n");
        }

        p_file_seq->check_mem      = run_memory;
        p_file_seq->m_play_state   = FILE_SEQ_STOP;
        p_file_seq->is_support_seek_finish = MT_FALSE;
        memset(p_file_seq->m_path, 0x00, MAX_PATH_NUM * sizeof(char *));

        create_player_lock(p_file_seq, p_pb_internal);

        mtos_sem_take(&g_mplayer_handle_sem_lock, 0);
        g_player_handle = p_file_seq;
        mtos_sem_give(&g_mplayer_handle_sem_lock);
    } else {
        MLOGE("[%s] fail to malloc FILE_SEQ_T!!!\n", __func__);
        return NULL;
    }

    if (p_param->is_direct_url) {
        p_file_seq->needTranslateUrl = MT_FALSE;
    } else {
        p_file_seq->needTranslateUrl = MT_TRUE;
    }

    p_file_seq->subt_buf_extra = (char *)mtos_malloc(SUBT_BUF_SIZE_MAX);//tizhang@20180822 for 104045

    p_file_seq->cmd_fifo = (FP_USER_CMD_T *)mtos_malloc(USER_CMD_FIFO_LEN * sizeof(FP_USER_CMD_T));
    memset(p_file_seq->cmd_fifo, 0, USER_CMD_FIFO_LEN * sizeof(FP_USER_CMD_T));
    MLOGI("[%s] stop stop ...\n", __func__);
#if defined(CFG_SMART_HTTP_PTOTOCOL)
    if (p_param->task_smart_http_priority[0] > 0) {
        Smart_Http_Init(p_param->task_smart_http_priority);
    }
#endif

    mp_ffmpeg_ext_cmd(MP_INIT_FFMPEG_MEM, MP_DO_CALLBACK, NULL);//zhouxiang changed

    return (void *)p_file_seq;
}

void  file_seq_destroy(void)
{
    MLOGI("[%s] start start ...\n", __func__);
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    int i = 0;

    if (NULL == p_file_seq) {
        MLOGI("%s file instance null\n", __func__);
        return;
    }

    x_force_stop_func(p_file_seq);
    if (p_file_seq->is_task_alive ||
        (p_file_seq->m_play_state != FILE_SEQ_STOP) || p_file_seq->is_load_task_alive) {

        while (1) {
            if (p_file_seq->is_task_alive || p_file_seq->is_load_task_alive) {
                i++;

                mtos_task_sleep(200);

                if (i % 5 == 0) {
                    MLOGI("[%s] wait fill es task exit ...!!!!\n", __func__);
                    MLOGI("[%s] task alive[%d,%d], m_play_state:%d\n", __func__,
                        p_file_seq->is_task_alive, p_file_seq->is_load_task_alive, p_file_seq->m_play_state);
                    MLOGI("wait %d sec\n", ((i + 1) * 200) / 1000);
                }
            } else {
                break;
            }
        }
        /*

        if (p_file_seq->total_path > 0) {
            for (i = 0; i < p_file_seq->total_path; i++) {
                mtos_free(p_file_seq->m_path[i]);
                p_file_seq->m_path[i] = NULL;
            }
        }
            */
    }
    mtos_sem_take(&g_mplayer_handle_sem_lock, 0);

    x_unmap_es_buffer(p_file_seq);
    /*destroy all the url paths*/
    for (i = 0; i < MAX_PATH_NUM; i++) {
        if (p_file_seq->m_path[i]) {
            mtos_free(p_file_seq->m_path[i]);
            p_file_seq->m_path[i] = NULL;
        }
    }

    if (p_file_seq->loadMedaiOK) {
        x_unloadmedia_func(p_file_seq);
    }

    destroy_player_sub_handle(p_file_seq);
    if (list_I_vpts) {
        list_rwlock_lock();
        destroy_list_vpts(list_I_vpts);
        list_rwlock_unlock();
        list_I_vpts = NULL;
    }

    file_seq_mem_release(p_file_seq);
    if (p_file_seq->p_ext_heap_hdl) {
        mtos_free(p_file_seq->p_ext_heap_hdl);
        p_file_seq->p_ext_heap_hdl = NULL;
    }

    /*destroy internal command fifo*/
    if (p_file_seq->cmd_fifo) {
        mtos_free(p_file_seq->cmd_fifo);
        p_file_seq->cmd_fifo = NULL;
    }

    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *) p_file_seq->pb_internal;
    if (NULL != pbi) {
        if (pbi->extio_ctx) {
            mlzp_free(pbi->extio_ctx);
            pbi->extio_ctx = NULL;
        }
        destroy_ffmpeg_dict_opts(p_file_seq);
        destroy_player_lock(p_file_seq, pbi);
        show_sys_memory_info(&(pbi->start_mem_info), &(pbi->stop_mem_info), MT_TRUE);
        mtos_free(pbi);
        p_file_seq->pb_internal = NULL;
    }

    if(p_file_seq->subt_buf_extra){//tizhang@20180822 for 104045
    mtos_free(p_file_seq->subt_buf_extra);
    p_file_seq->subt_buf_extra = NULL;
    }
    mtos_free(p_file_seq);
    p_file_seq = NULL;

#if defined(CFG_SMART_HTTP_PTOTOCOL)
    Smart_Http_Deinit();
#endif

    g_player_handle = NULL;

    mtos_sem_give(&g_mplayer_handle_sem_lock);
    mtos_sem_destroy(&g_mplayer_handle_sem_lock, 0);

    MLOGI("[%s] stop stop ...\n", __func__);
}

void file_seq_check_trickplay(void *p_handle) {
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;
    int in_size = 0;

    if(!p_file_seq) {
        return;
    }

    demuxer_t *demuxer = (demuxer_t *)(p_file_seq->p_demuxer);
    p_file_seq->unable_trickplay = 0;
    if (demuxer->type_adaptive_stream > TYPE_ADAPTIVE_STREAM_NONE) { //vod hls file not support trickplay
        p_file_seq->unable_trickplay = 1;
        MLOGD("%s %d trickplay unsupport \n", __func__, __LINE__);
        return ;
    }

    if (0 == demuxer->seekable) {
        p_file_seq->unable_trickplay = 1;
        MLOGD("%s %d trickplay unsupport \n", __func__, __LINE__);
        return ;
    }

    //only h264 ts file for check
    if (p_file_seq->is_ts && p_file_seq->m_video_codec_type == 1) {
        demux_seek(demuxer, 0, 0, 1) ;
        in_size = ds_get_packet(p_file_seq->p_cur_ds_video, &(p_file_seq->p_v_pkt_start));

        if (in_size < 128) {
            p_file_seq->unable_trickplay = 1;
            MLOGD("%s %d trickplay unsupport \n", __func__, __LINE__);
        }
        demux_seek(demuxer, 0, 0, 1) ;
    }
}

int file_seq_set_http_header_useragent(char *useragent) {
    if (mediaplay_http_header_useragent) {
        free(mediaplay_http_header_useragent);
        mediaplay_http_header_useragent = NULL;
    }

    if (useragent != NULL) {
        int len = strlen(useragent) + 1;

        mediaplay_http_header_useragent = malloc(len);
        if (!mediaplay_http_header_useragent) {
            MLOGE("[%s] mediaplay_http_header_useragent == NULL...not enough memory to malloc\n", __func__);
            return -1;
        }

        memset(mediaplay_http_header_useragent, 0, len);
        strcpy(mediaplay_http_header_useragent, useragent);
        MLOGE("%s:%s\n", __func__, useragent);
    } else {
        mediaplay_http_header_useragent = useragent;
        MLOGE("%s: set mediaplay_http_header_useragent to NULL.\n",
              __func__);
    }

    return 0;
}

int mp_set_mov_desc_key(char *value) {
    g_mov_desc_key = value;
    return 0;
}

int mp_set_mov_desc_key_len(char * len) {
    g_mov_desc_key_len = len;
    return 0;
}
