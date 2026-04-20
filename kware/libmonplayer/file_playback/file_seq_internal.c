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

#define MODULE_TAG "FPI"
#include "mutil.h"
#include "mlog.h"
#include "mt_type.h"
#include "fifo_kw.h"
#include "mt_drv_disp.h"
#include "ts_sequence.h"
#include "libmpdemux/stheader.h"

#include "drv_adp.h"
#include "file_playback_sequence.h"
#include "demux_mp.h"
#include "file_seq_internal.h"
#include "file_seq_misc.h"
#include "mtos_mem.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_task.h"
#include "mtos_misc.h"
#include "libmpdemux/demuxer.h"
#include "stream/stream.h"
#include "mt_audio_codec.h"
#include "mt_unf_video.h"
#include "mt_unf_avplay.h"

#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "pts_list.h"
#include "libavcodec/avcodec.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_video.h"

#ifdef DRM_SMP_ENABLE
#include "MTDrmApi.h"
#endif

extern int io_isnetworkstream ;
extern int bsf_vcodec_flag;
extern FILE_SEQ_T *x_get_cur_instance(void);
extern void ds_reset_trickplay_para(demuxer_t *demuxer);
#define  VIDEO_ES_PKT_UNDERFLOW_CNT           3

#define SUB_TIMECODE_CONV(sh, sm, ss, sc, eh, em, es, ec) \
    do {                                                  \
        sh = sc / 36000 ;                                 \
        sc -= 36000 * sh;                                 \
        sm = sc /  600  ;                                 \
        sc -=   600 * sm;                                 \
        ss = sc /   10  ;                                 \
        sc -=    10 * ss;                                 \
        sc *= 100       ;                                 \
        eh = ec / 36000 ;                                 \
        ec -= 36000 * eh;                                 \
        em = ec /  600  ;                                 \
        ec -=   600 * em;                                 \
        es = ec /   10  ;                                 \
        ec -=    10 * es;                                 \
        ec *= 100       ;                                 \
    } while (0)

double first_vpts = -2;
Node *list_I_vpts = NULL;
extern MT_BOOL check_task_finish(FILE_SEQ_T *p_file_seq);

static int check_is_pts_leap(int es_type)
{
    int ret = 0;
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    demux_stream_t *ds_a = NULL;
    demux_stream_t *ds_v = NULL;
    int normal_play = (p_file_seq->cur_speed == TS_SEQ_NORMAL_PLAY || p_file_seq->cur_speed == TS_SEQ_FAST_PLAY_2X);
    if (!normal_play) {
        return 0;
    }

    ds_a = p_file_seq->p_cur_ds_audio;
    ds_v = p_file_seq->p_cur_ds_video;
    switch (es_type) {
        case 0: {
            if ((fabs(ds_a->pts - ds_v->pts) > 3.f) && ds_v->bytes > 0) {
                ret = 1;
            }

            break;
        }
        case 1: {
            if ((fabs(ds_a->pts - ds_v->pts) > 3.f) && ds_a->bytes > 0) {
                ret = 1;
            }
            break;
        }
        case 2:
            break;
        default:
            break;
    }

    return ret;
}

RET_CODE player_vdec_start(FILE_SEQ_T *p_file_seq, int format, int mode)
{
    demuxer_t *p_demuxer = (demuxer_t *)p_file_seq->p_demuxer;
    sh_video_t *p_sh_video = p_demuxer->video->sh;
    RET_CODE ret;

    ret = vdec_start(p_file_seq->p_vdec_dev, format, mode);
    if(p_file_seq->video_fps > 0) {
        ret |= set_video_decoder_framerate(p_file_seq->video_fps);
    }
    {
        int rret;
        rret = set_hdr_info(NULL, (MT_UNF_VIDEO_DISP_HDR_INFO_S *)&p_sh_video->hdr);
        if (rret) {
            MLOGE("[%s_%d] set hdr info fail!!!\n", __func__, __LINE__);
        }
    }
    return ret;
}

void player_vdec_trick_mode(FILE_SEQ_T *p_file_seq, int curr_speed, int next_speed)
{
    MT_UNF_DEC_TRICK_PARAM_S trick_para = {MT_FALSE,  MT_UNF_DEC_TM_NORMAL};

    if (curr_speed == next_speed) {
        return;
    }

    if (fp_is_normal_play(next_speed)) {
        trick_para.trick_mode = MT_UNF_DEC_TM_NORMAL;
        (void) vdec_set_trick_mode(p_file_seq->p_vdec_dev, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 1);
        (void) vdec_set_dec_frm_type(p_file_seq->p_vdec_dev, MT_UNF_DEC_FRM_ALL);
    }

    if (fp_is_fast_forward(next_speed)) {
        trick_para.trick_mode = MT_UNF_DEC_TM_FFWD;
        (void) vdec_set_trick_mode(p_file_seq->p_vdec_dev, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 2);
        if (TS_SEQ_FAST_PLAY_2X == next_speed) {
            (void) vdec_set_dec_frm_type(p_file_seq->p_vdec_dev, MT_UNF_DEC_FRM_ALL);
        } else {
            trick_para.is_incomplete_stream = MT_TRUE;
            (void) vdec_set_dec_frm_type(p_file_seq->p_vdec_dev, MT_UNF_DEC_FRM_IP);
        }
    }

    if (fp_is_fast_backward(next_speed)) {
        trick_para.trick_mode = MT_UNF_DEC_TM_FREV;
        trick_para.is_incomplete_stream = MT_TRUE;
        (void) vdec_set_trick_mode(p_file_seq->p_vdec_dev, MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD, 2);
        (void) vdec_set_dec_frm_type(p_file_seq->p_vdec_dev, MT_UNF_DEC_FRM_IP);
    }

    (void) vdec_set_trick_cfg(p_file_seq->p_vdec_dev, &trick_para);
}

int  file_seq_av_decoder_init(void *p_handle)
{
    int ret = 0;
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_handle;

    if (p_file_seq->init_av_dev == MT_FALSE) {
        p_file_seq->init_av_dev = MT_TRUE;
        x_init_av_device(p_file_seq);
    }

    if (p_file_seq->p_audio_dev) {
        aud_stop_vsb(p_file_seq->p_audio_dev);
    }

    if (p_file_seq->p_vdec_dev) {
        if (fp_is_timeshift_file() == 1) {
            vdec_freeze_stop(p_file_seq->p_vdec_dev);
        } else {
            vdec_stop(p_file_seq->p_vdec_dev);
        }
    }

    mtos_task_sleep(20);
    demuxer_t  *demuxer = (demuxer_t *)p_file_seq->p_demuxer;
    if (p_file_seq->is_ts || TYPE_ADAPTIVE_STREAM_HLS == demuxer->type_adaptive_stream) {
        vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 0);//VDEC_AVSYNC_FILEPLAYTS
    } else if (io_isnetworkstream) {
        vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 4);
    } else {
        vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 0);//VDEC_AVSYNC_FILEPLAY
    }

    ret = x_start_av_decoder(p_file_seq);

    MLOGD("###cgf debug[%s]. %d x_start_av_decoder ret = %d\n", __func__, __LINE__, ret);

    return ret;
}

static int start_buffering(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if ((unsigned int) p_internal->buffering_stat != (unsigned int) FILE_SEQ_BUFFERING) {
        p_file_seq->m_play_state_backup = p_file_seq->m_play_state;
        p_internal->buffering_stat = FILE_SEQ_BUFFERING;

        vdec_pause(p_file_seq->p_vdec_dev);
        aud_pause_vsb(p_file_seq->p_audio_dev);

        MLOGD("%s %d!!!####@@@@@@@\n", __FUNCTION__, __LINE__);

        mtos_task_sleep(500);
        MLOGD("%s %d!!!####@@@@@@@\n", __FUNCTION__, __LINE__);
    }
    return 0;
}

/**
 * Function : Just set pcm audio codec audio paramter for audio decoder
*/
#define INVALID_HANDLE        0
#define NORMAL_PCM_EXTWORD    1
#define WIFIDSP_LPCM_EXTWORD  2

static void reconfig_audio_extradata(
    FILE_SEQ_AUDIO_T *audio, sh_common_t *sh)
{
    /* codec such as vorbis need more memory, so we use the sh data */
    audio->codec_extradata      = sh->codec_extradata;
    audio->codec_extradata_size = sh->codec_extradata_size;
}

int set_audio_param_to_vsb(void)
{
    int ret = 0;
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if (NULL != p_file_seq->event_cb) {
        p_file_seq->event_cb(FILE_PLAYBACK_SWITCH_AUDIO_SPDIF_MODE, (unsigned long)&p_file_seq->m_audio_codec_type);
    }

    if (HA_AUDIO_ID_PCM == p_file_seq->m_audio_codec_type) {
        WAV_FORMAT_S audio_param;
        int is_big_endian = 0;
        int bits = 16;
        int channels = 2;
        int sample_rate = 44100;

        memset(&audio_param, 0, sizeof(WAV_FORMAT_S));
        if (p_file_seq->is_ts) {
            ts_get_pcm_info(p_file_seq->p_demuxer, &is_big_endian, &bits, &channels, &sample_rate);
        } else {
            bits          = p_file_seq->audio_sample_bits;
            sample_rate   = p_file_seq->audio_samplerate;
            channels      = p_file_seq->audio_channels;
            is_big_endian = pbi->audio.pcm_be;
        }

        if (MT_TRUE == is_big_endian) {
            audio_param.cbSize = 4;
            audio_param.cbExtWord[0] = NORMAL_PCM_EXTWORD; //choose normal pcm decoder
            //stWavFormat.cbExtWord[0] = WIFIDSP_LPCM_EXTWORD; //choose wifi_dsp_lpcm decoder
        }

        audio_param.nSamplesPerSec = sample_rate;
        audio_param.nChannels = channels;
        audio_param.wBitsPerSample = bits;
        audio_param.cbExtWord[0] = is_big_endian;
        audio_param.cbExtWord[1] = channels >> 1;

        MLOGI("[PCM]:big %d ch %d bits %d, sr %d\n", is_big_endian, channels, bits, sample_rate);
        aud_set_dec_param_vsb(p_file_seq->p_audio_dev, &audio_param, p_file_seq->m_audio_codec_type);
        ret = aud_start_vsb(p_file_seq, HA_AUDIO_ID_PCM);
    } else if (HA_AUDIO_ID_COOK == p_file_seq->m_audio_codec_type ||
               HA_AUDIO_ID_AMRNB == p_file_seq->m_audio_codec_type ||
               HA_AUDIO_ID_AMRWB == p_file_seq->m_audio_codec_type) {
        HA_FFMPEG_DECODE_OPENCONFIG_S ffmpge_cfg;
        demuxer_t *demux = (demuxer_t *)p_file_seq->p_demuxer;
        demux_stream_t *ds_a = demux->audio;
        sh_audio_t *p_sh_audio = ds_a->sh;

        if (p_sh_audio) {
            if (demux->type == DEMUXER_TYPE_LAVF) {
                if (!p_sh_audio->hAvCtx) {
                    MLOGI("ffmpeg_dec_opencfg.hAvCtx = p_sh_audio->codec is NULL \n");
                }

                ffmpge_cfg.hAvCtx = p_sh_audio->hAvCtx;
            } else {
                MLOGI("codec %d should use ffmpeg demux but use dmx %d\n", p_file_seq->p_audio_dev, demux->type);
            }
        }

        aud_set_dec_param_vsb(p_file_seq->p_audio_dev, &ffmpge_cfg, p_file_seq->m_audio_codec_type);
        MLOGI("set ffmpeg param to dec \n");
        ret = aud_start_vsb(p_file_seq, p_file_seq->m_audio_codec_type);
    } else if (HA_AUDIO_ID_WMA9STD == p_file_seq->m_audio_codec_type) {
        int i;
        WMA_FORMAT_S wma_param;
        memset(&wma_param, 0, sizeof(WMA_FORMAT_S));
        wma_get_dec_info(p_file_seq->p_demuxer, (void *) &wma_param);

        MLOGI("%s ch %d sr %d br %d blockalign %d codec %d bits %d\n", __func__,
              wma_param.nChannels, wma_param.nSamplesPerSec, wma_param.nAvgBytesPerSec,
              wma_param.nBlockAlign, wma_param.wFormatTag, wma_param.wBitsPerSample);

        MLOGD("%s extrasize %d ", __func__, wma_param.cbSize);
        for (i = 0; i < wma_param.cbSize; i++) {
            MLOGD("%x ", wma_param.cbExtWord[i]);
        }
        MLOGD("\n");
        aud_set_dec_param_vsb(p_file_seq->p_audio_dev, &wma_param, p_file_seq->m_audio_codec_type);
        ret = aud_start_vsb(p_file_seq, p_file_seq->m_audio_codec_type);
    } else if (HA_AUDIO_ID_FLAC      == p_file_seq->m_audio_codec_type ||
               HA_AUDIO_ID_OPUS      == p_file_seq->m_audio_codec_type ||
               HA_AUDIO_ID_VORBIS    == p_file_seq->m_audio_codec_type ||
               HA_AUDIO_ID_DOLBY_AC4 == p_file_seq->m_audio_codec_type) {
        demuxer_t *demux = (demuxer_t *) p_file_seq->p_demuxer;
        demux_stream_t *ds_a = demux->audio;
        reconfig_audio_extradata(&pbi->audio, (sh_common_t *)(ds_a->sh));
        ret = aud_start_vsb(p_file_seq, p_file_seq->m_audio_codec_type);
    } else {
        ret = aud_start_vsb(p_file_seq, p_file_seq->m_audio_codec_type);
    }

    return ret;

}

static int av_decoder_reset(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    demuxer_t  *demuxer  = (demuxer_t *)p_file_seq->p_demuxer;

    mtos_sem_take((os_sem_t *)(&(((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal))->seek_mutex)), 0);
    start_buffering();

    if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
        aud_stop_vsb(p_file_seq->p_audio_dev);
    }
    vdec_resume(p_file_seq->p_vdec_dev);
    MLOGI("%s %d!!!####@@@@@@@\n", __FUNCTION__, __LINE__);

    vdec_freeze_stop(p_file_seq->p_vdec_dev);

    if (p_file_seq->only_audio_mode == MT_FALSE) {
        ds_reset_trickplay_para(p_file_seq->p_cur_demuxer);
        if (p_file_seq->is_ts || TYPE_ADAPTIVE_STREAM_HLS == demuxer->type_adaptive_stream) {
            vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 2);//VDEC_AVSYNC_FILEPLAYTS
        } else if (io_isnetworkstream) {
            vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 4);
        } else {
            vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 1);//VDEC_AVSYNC_FILEPLAY
        }
        vdec_file_clearesbuffer(p_file_seq->pb_internal);

        player_vdec_start(p_file_seq, p_file_seq->m_video_codec_type, 2);
        vdec_set_dec_frm_type(p_file_seq->p_vdec_dev, MT_UNF_DEC_FRM_ALL);

        if (p_file_seq->cur_speed == TS_SEQ_FAST_PLAY_2X) {
            vdec_set_trick_mode(p_file_seq->p_vdec_dev, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 2);//VDEC_TM_FFWD
        }
    }
	
	//update for tts support , don't stop adec
    if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
        set_audio_param_to_vsb();
    }

    vdec_pause(p_file_seq->p_vdec_dev);
    aud_pause_vsb(p_file_seq->p_audio_dev);
    mtos_sem_give((os_sem_t *)(&(((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal))->seek_mutex)));
    p_file_seq->needNewAudioData  = MT_TRUE;
    p_file_seq->needNewVideoData  = MT_TRUE;
    p_file_seq->m_tmp_ves_buf_pos = 0;
    p_file_seq->isAudioBufferFull = MT_FALSE;
    p_file_seq->isVideoBufferFull = MT_FALSE;
    p_file_seq->left_a_pkt_bytes  = 0;
    p_file_seq->left_v_pkt_bytes  = 0;
#if 1
    p_file_seq->isAudioEsEnd = MT_FALSE;
    p_file_seq->isVideoEsEnd = MT_FALSE;
    ((demux_stream_t *) p_file_seq->p_cur_ds_audio)->eof = 0;
    ((demux_stream_t *) p_file_seq->p_cur_ds_video)->eof = 0;
#endif
    p_file_seq->vpts_upload = 1;
    p_file_seq->apts_upload = 0;
    return 0;
}


int file_seq_get_reset_state(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    return p_internal->av_demuxer_reset;
}


int file_seq_set_reset_state(int state)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    MLOGI("%s %d!!!####@@@@@@@\n", __FUNCTION__, __LINE__);
    p_internal->av_demuxer_reset = state;
    return 0;
}


int file_seq_switch_reset(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    MLOGI("%s %d!!!####@@@@@@@\n", __FUNCTION__, __LINE__);
    //mtos_sem_take((os_sem_t *)(&(((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal))->seek_mutex)), 0);
    //demuxer reset
    ds_free_packs(p_file_seq->p_cur_ds_audio);
    ds_free_packs(p_file_seq->p_cur_ds_video);

    //decoder reset
    if (file_seq_get_reset_state() == 1) {
        av_decoder_reset();
        file_seq_set_reset_state(2);
    }
    //mtos_sem_give((os_sem_t *)(&(((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal))->seek_mutex)));

    return 0;
}

static void disble_audio_track(demuxer_t *demuxer)
{
    MLOGI("Discard audio stream data, id:%d\n", demuxer->audio->id);
    (void) demuxer_disable_track(demuxer, demuxer->audio->id);
}

static int get_video_underflow_cnt(PLAYBACK_INTERNAL_T *pbi)
{
    mlzp_mutex_lock(pbi->video.mutex);
    int pkt_cnt_underflow = pbi->video.pkt_cnt_underflow;
    mlzp_mutex_unlock(pbi->video.mutex);
    return pkt_cnt_underflow;
}

static void set_video_underflow_cnt(PLAYBACK_INTERNAL_T *pbi, int cnt)
{
    mlzp_mutex_lock(pbi->video.mutex);
    pbi->video.pkt_cnt_underflow = cnt;
    mlzp_mutex_unlock(pbi->video.mutex);
}

static void increase_video_underflow_cnt(PLAYBACK_INTERNAL_T *pbi)
{
    mlzp_mutex_lock(pbi->video.mutex);
    pbi->video.pkt_cnt_underflow++;
    mlzp_mutex_unlock(pbi->video.mutex);
}

MT_BOOL x_check_av_codec_type(void *p_handle)
{
    int video_codec  = MT_UNF_VCODEC_TYPE_BUTT;
    int audio_codec  = HA_AUDIO_ID_INVALID;
    MT_BOOL isSupportVideoType = MT_FALSE;
    MT_BOOL isSupportAudioType = MT_FALSE;
    FILE_SEQ_T *p_file_seq  = (FILE_SEQ_T *)p_handle;
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    demuxer_t *p_demuxer = (demuxer_t *)p_file_seq->p_demuxer;
    sh_video_t *p_sh_video = p_demuxer->video->sh;

    if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {
        MLOGW("[%s] FILE_SEQ_EXIT...\n", __func__);
        return MT_FALSE;
    }
    MLOGI("[%s] only_audio_mode: %d\n", __func__, p_file_seq->only_audio_mode);
    if (p_file_seq->only_audio_mode) {
        p_file_seq->audio_bps = ds_get_audio_bps(p_file_seq->p_ds_audio);

        if (p_file_seq->audio_bps <= 0 || p_file_seq->audio_bps >= AUDIO_MAX_BPS) {
            p_file_seq->audio_bps = AUDIO_DEFAULT_BPS;
            MLOGE("No audio bps, set default 15000!\n");
        }

        MLOGD("[%s] audio_bps[%d]\n", __func__, p_file_seq->audio_bps);
        (void) ds_get_audio_codec_info(p_file_seq->p_ds_audio, &p_file_seq->m_audio_codec_type,
            &pbi->audio.pcm_be, &pbi->audio.codec_id, &p_file_seq->audio_pid, p_file_seq->audio_output_mode);

        audio_codec = p_file_seq->m_audio_codec_type;
        isSupportAudioType = fp_is_support_audio_codec(p_file_seq, audio_codec);
        if (isSupportAudioType == MT_FALSE) {
            if (NULL != p_file_seq->event_cb) {
                p_file_seq->event_cb(FILE_PLAYBACK_UNSUPPORT_AUDIO, pbi->audio.codec_id);
            }
            return MT_FALSE;
        } else {
            p_file_seq->is_av_codec_support = p_file_seq->is_av_codec_support | 0x1;
            p_file_seq->checkAvTypeOK = MT_TRUE;
        }

        MLOGD("[%s] this container has only audio stream !!!!\n", __func__);
        return MT_TRUE;
    }

    p_file_seq->audio_bps = ds_get_audio_bps(p_file_seq->p_ds_audio);
    MLOGD("[%s] type[%d]\n",
          __func__, ((demuxer_t *)(p_file_seq->p_demuxer))->type);

    if (io_isnetworkstream == 0) {
        p_file_seq->video_bps = ds_get_video_bps(p_file_seq->p_ds_video);
    }

    ds_get_video_codec_type(p_file_seq->p_ds_video,
        (int *)&p_file_seq->m_video_codec_type,
        &p_file_seq->video_pid, &p_file_seq->pcr_pid);
    (void) ds_get_audio_codec_info(p_file_seq->p_ds_audio,
        &p_file_seq->m_audio_codec_type,
        &pbi->audio.pcm_be, &pbi->audio.codec_id, &p_file_seq->audio_pid, p_file_seq->audio_output_mode);
    MLOGD("[%s] vcodec type=%d, video_pid=%d, pcr_pid=%d\n", __func__, \
          p_file_seq->m_video_codec_type, p_file_seq->video_pid, p_file_seq->pcr_pid);
    MLOGD("[%s] a_codec_type=%d, audio_pid=%d\n", __func__, p_file_seq->m_audio_codec_type, p_file_seq->audio_pid);
    MLOGD("[%s] acodec id=%x, vcodec id=%x\n", __func__, pbi->audio.codec_id, p_sh_video->codec_id);
    video_codec = p_file_seq->m_video_codec_type;

    switch (video_codec) {
        case  MT_UNF_VCODEC_TYPE_VP8:
        case  MT_UNF_VCODEC_TYPE_REAL8:
        case  MT_UNF_VCODEC_TYPE_REAL9:
        case  MT_UNF_VCODEC_TYPE_H263:
        case  MT_UNF_VCODEC_TYPE_MPEG4:
        case  MT_UNF_VCODEC_TYPE_AVS:
        case  MT_UNF_VCODEC_TYPE_AVS2:
        case  MT_UNF_VCODEC_TYPE_H264:
        case  MT_UNF_VCODEC_TYPE_MPEG2:
        case  MT_UNF_VCODEC_TYPE_VC1:
        case  MT_UNF_VCODEC_TYPE_HEVC:
        case  MT_UNF_VCODEC_TYPE_VP9:
            if (MT_FALSE == fpi_codec_in_blacklist(video_codec,
                   MT_UNF_VCODEC_TYPE_BUTT, pbi->video.codec_blacklist)) {
                isSupportVideoType = MT_TRUE;
                p_file_seq->is_av_codec_support |= 0x2;
                if (video_codec == MT_UNF_VCODEC_TYPE_VP9 && pbi->chip_type < CHIPTYPE_SYMPHONY4) {
                    //only sym4 support VP9
                    isSupportVideoType = MT_FALSE;
                    p_file_seq->is_av_codec_support &= 0xFFFFFFFD; //clear bit [1] video valid bit
                } else if ((MT_UNF_VCODEC_TYPE_REAL8 == video_codec || MT_UNF_VCODEC_TYPE_REAL9 == video_codec) &&
                    pbi->chip_type < CHIPTYPE_SYMPHONY6) {
                    /* symphony not support rv8,9 */
                    isSupportVideoType = MT_FALSE;
                    p_file_seq->is_av_codec_support &= 0xFFFFFFFD; //clear bit [1] video valid bit
                    MLOGW("[%s:%d] not support rm8 rm9\n",__func__,__LINE__);
                }
            } else {
                MLOGW("Video codec %d in black list, not support\n", video_codec);
            }
            break;
        default:
            break;
    }

    MLOGI("[%s] isSupportVideoType=%d, video_codec=%d\n", __func__, isSupportVideoType, video_codec);
    if(isSupportVideoType == MT_FALSE){
        MLOGW("have video but not support\n");
        p_file_seq->only_audio_mode = 1;
    }
    audio_codec = p_file_seq->m_audio_codec_type;
    isSupportAudioType = fp_is_support_audio_codec(p_file_seq, audio_codec);
    if (MT_TRUE == isSupportAudioType) {
        p_file_seq->is_av_codec_support = p_file_seq->is_av_codec_support | 0x1;
    }

    MLOGI("[%s] isSupportAudioType=%d, audio_codec=0x%x\n", __func__, isSupportAudioType, audio_codec);
    if (isSupportVideoType) {
        p_file_seq->checkAvTypeOK = MT_TRUE;
        if (MT_FALSE == isSupportAudioType) {
            disble_audio_track((demuxer_t *) p_file_seq->p_demuxer);
        }
        return MT_TRUE;
    } else {
        if (((video_codec == 99) || (video_codec == MT_UNF_VCODEC_TYPE_MJPEG)) && (isSupportAudioType)) { // linda zhu add, we support the audio file with MJPG cover.
            p_file_seq->only_audio_mode = MT_TRUE;
            p_file_seq->checkAvTypeOK = MT_TRUE;
            return MT_TRUE;
        }
        if (NULL != p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_UNSUPPORT_VIDEO, p_sh_video->codec_id);  //vince for bug 30923
        }

        MLOGD("[%s] fail to check av codec type !!!\n", __func__);
        MLOGD("[%s] video_codec[%d]  audio_codec[%d] !!!\n", __func__, video_codec, audio_codec);
    }

    return MT_FALSE;
}

static void do_play_at_time_hls_sync(FILE_SEQ_T *p_file_seq)
{
    int pack_num = 0;
    int try_cnt  = 0;
    demux_stream_t *ds_a = p_file_seq->p_cur_ds_audio;
    demux_stream_t *ds_v = p_file_seq->p_cur_ds_video;
    int cur_audio_packet_size = 0;
    int cur_video_packet_size = 0;
    const static int MAX_TRY_CNT  = 500;
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    do {
        cur_video_packet_size = fpi_get_ds_video_packet(
           p_file_seq, ds_v, &(p_file_seq->p_v_pkt_start), p_file_seq->cur_speed);
        try_cnt++;
    } while (!ds_v->flags && try_cnt < MAX_TRY_CNT);

    if (ds_v->pts < 0.0) {
        ds_v->pts = 0.0;
    }

    p_file_seq->orig_vpts =  ds_v->pts * TIME_BASE;
    p_file_seq->left_v_pkt_bytes = cur_video_packet_size;
    p_file_seq->sys_vpts = TIME_BASE * (p_file_seq->orig_vpts) ;
    p_file_seq->m_tmp_ves_buf_pos = 0;
    if (HA_AUDIO_ID_INVALID != p_file_seq->m_audio_codec_type) {
        PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
        do {
            if (!ds_a->eof) {
                p_file_seq->extra_audio_size = 0;
                p_file_seq->p_extra_aud_buf = NULL;
                cur_audio_packet_size = fpi_get_ds_audio_packet(p_file_seq, (void *) ds_a, &(p_file_seq->p_a_pkt_start),
                    &(p_file_seq->p_extra_aud_buf), (uint8_t *) &(p_file_seq->extra_audio_size));

                if (cur_audio_packet_size <= 0 || p_file_seq->p_a_pkt_start == NULL) {
                    MLOGW("[%s][error]  >>>> NO audio strream!!<<<<\n", __func__);
                    break;
                }

                p_file_seq->needNewVideoData = MT_FALSE;
                p_file_seq->left_a_pkt_bytes = cur_audio_packet_size;
            } else {
                MLOGI("[%s][ok]  >>>> Audio Stream Demuxer Is Finished !!<<<<\n", __func__);
                p_file_seq->isAudioEsEnd = MT_TRUE;
                break;
            }

            if (ds_a->pts < 0.0) {
                ds_a->pts = 0.0;
            }

            p_file_seq->orig_apts = ds_a->pts * TIME_BASE;
            if (p_file_seq->orig_apts > p_file_seq->max_audio_pts) {
                p_file_seq->max_audio_pts = p_file_seq->orig_apts;
            }

            if (p_file_seq->orig_apts - p_file_seq->orig_vpts >= 0) {
                p_file_seq->needNewAudioData = MT_FALSE;
                p_file_seq->sys_apts = p_file_seq->orig_apts * TIME_BASE ;
                break;
            }

            if (p_file_seq->p_extra_aud_buf) {
                p_file_seq->p_extra_aud_buf = NULL;
                p_internal->is_need_extra_audio = 0;
            }
        } while (pack_num++ < MAX_TRY_CNT);

        if (p_file_seq->needNewAudioData) {
            if (p_file_seq->p_extra_aud_buf) {
                p_file_seq->p_extra_aud_buf = NULL;
                p_internal->is_need_extra_audio = 0;
            }
        }
        MLOGI("[%s]throw[%d %d]packet,first[%lld]ms, apts[%lld]ms,vpts[%lld]ms flag:%d\n", __func__, pack_num, try_cnt,
              (int64_t)p_file_seq->first_vpts, (int64_t)p_file_seq->orig_apts, (int64_t)p_file_seq->orig_vpts, ds_v->flags);
    }
}

void do_playAtTime(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    demuxer_t  *p_demuxer = (demuxer_t *)p_file_seq->p_demuxer;
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    MLOGI("Enter %s\n", __func__);
    if (!p_demuxer->seekable) {
        ClearFlag(p_file_seq->m_user_cmd, CMD_PLAY_AT_TIME);

        if (NULL != p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_UNSUPPORT_SEEK, 0);
        }

        MLOGW("---FILE_PLAYBACK_UNSUPPORT_SEEK---\n");
        return;
    }
    set_video_underflow_cnt(p_internal, -1);
    if ((p_file_seq->is_play_at_time == 1) &&
        ((p_file_seq->seek_seconds + 1) > 0)) {
        mtos_sem_take((os_sem_t *)(&(p_internal->seek_mutex)), 0);

        vdec_flush(p_file_seq->pb_internal);
        if (p_demuxer->subt_info.cnt > 0) {
            clear_sub_fifo_kw(p_file_seq->p_sub_fifo_handle);    //clear fifo when do seek
        }

        MLOGI("[%s] real seek time: %f s\n", __func__, (p_file_seq->seek_seconds));

        u64 min_vpts_upload = (u64)(p_file_seq->seek_seconds * 1000) * 1000;
        if (min_vpts_upload > (DELIVER_VPTS_INTERVAL * 1.5)) {
            min_vpts_upload -= (DELIVER_VPTS_INTERVAL * 1.5);
        }

        MLOGD("vpts_upload = %llu, min_vpts_upload = %llu\n", p_file_seq->vpts_upload, min_vpts_upload);
        p_file_seq->vpts_upload = min_vpts_upload;//changed by zhouxiang, seek start, dont send old pts to ui!
        if (p_file_seq->total_path > 1) {
            //demux_seek(p_file_seq->p_cur_demuxer, p_file_seq->seek_seconds , 0, 0) ;
        } else {
            if (p_demuxer->type == DEMUXER_TYPE_MPEG_PS) {
                if (p_file_seq->only_audio_mode) {
                    p_file_seq->seek_seconds = p_file_seq->seek_seconds - p_demuxer->audio->pts + first_vpts;
                } else {
                    p_file_seq->seek_seconds = p_file_seq->seek_seconds - p_demuxer->video->pts + first_vpts;
                }
                demux_seek(p_demuxer, p_file_seq->seek_seconds, 0, 0) ;
            } else {
                u32 prev_I_pts = (u32)(p_file_seq->seek_seconds * 1000);
                u32 post_I_pts = (u32)(p_file_seq->seek_seconds * 1000);
                float seek_I_pts = p_file_seq->seek_seconds;
                if (p_demuxer->type != DEMUXER_TYPE_LAVF && list_I_vpts && ListLength(list_I_vpts) > 0) {
                    get_accurate_vpts_I_frame(list_I_vpts, (u32)(p_file_seq->seek_seconds * 1000), &prev_I_pts, &post_I_pts);

                    if ((post_I_pts - prev_I_pts > 15000) || (post_I_pts < p_file_seq->seek_seconds * 1000)) {
                        seek_I_pts = p_file_seq->seek_seconds;
                    } else {
                        if (abs(post_I_pts - (u32)(seek_I_pts * 1000)) < abs(prev_I_pts - (u32)(seek_I_pts * 1000))) {
                            seek_I_pts = post_I_pts / 1000.0;
                        } else {
                            seek_I_pts = prev_I_pts / 1000.0;
                        }
                    }
                }
                demux_seek(p_demuxer, seek_I_pts, 0, SEEK_ABSOLUTE);
                MLOGI("seek_I_pts = %f\n", seek_I_pts);
            }
        }

        if (p_file_seq->only_audio_mode == MT_FALSE) {
            ds_reset_trickplay_para(p_demuxer);
            if (p_file_seq->is_ts || TYPE_ADAPTIVE_STREAM_HLS == p_demuxer->type_adaptive_stream) {
                vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 2);//VDEC_AVSYNC_FILEPLAYTS
            } else if (io_isnetworkstream) {
                vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 4);
            } else {
                vdec_set_avsync_mode(p_file_seq->p_vdec_dev, 1);//VDEC_AVSYNC_FILEPLAY
            }
            vdec_file_clearesbuffer(p_file_seq->pb_internal);

            if (p_file_seq->cur_speed == TS_SEQ_FAST_PLAY_2X) {
                vdec_set_trick_mode(p_file_seq->p_vdec_dev, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 2);    //VDEC_TM_FFWD
            }
            MLOGD("[%s] ------- call vdec_start\n", __func__);
        }


			//update for tts support , don't stop adec
        if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
            set_audio_param_to_vsb();
        }

        MLOGD("[%s] ------- call aud_start_vsb\n", __func__);
        p_file_seq->needNewAudioData = MT_TRUE;
        p_file_seq->needNewVideoData = MT_TRUE;
        p_file_seq->m_tmp_ves_buf_pos = 0;
        p_file_seq->isAudioBufferFull = MT_FALSE;
        p_file_seq->isVideoBufferFull = MT_FALSE;

        p_file_seq->isAudioEsEnd = MT_FALSE;
        p_file_seq->isVideoEsEnd = MT_FALSE;

        if (p_file_seq->is_support_seek_finish) {
            p_file_seq->is_play_to_end = 0;
        }

        ((demux_stream_t *) p_file_seq->p_cur_ds_audio)->eof = 0;
        ((demux_stream_t *) p_file_seq->p_cur_ds_video)->eof = 0;

        p_file_seq->apts_upload = 0;
        if (TYPE_ADAPTIVE_STREAM_HLS  == p_demuxer->type_adaptive_stream ||
            TYPE_ADAPTIVE_STREAM_DASH == p_demuxer->type_adaptive_stream) {
            do_play_at_time_hls_sync(p_file_seq);
        }

        p_file_seq->is_play_at_time = 0;
        p_file_seq->last_audio_pts = 0;
        if (p_file_seq->total_path <= 1) {
            p_file_seq->seek_seconds = -1;
        }
        p_internal->seek_buffer = 1;
        p_internal->net_buffer.fp_start_tick = mtos_ticks_get();
        p_internal->net_buffer.is_enbale     = MT_FALSE;
        mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
    }

    ClearFlag(p_file_seq->m_user_cmd, CMD_PLAY_AT_TIME);
    if (NULL != p_file_seq->event_cb) {
        p_file_seq->event_cb(FILE_PLAYBACK_SEEK_FINISHED, 0);
    }
    MLOGI("Exit %s!\n", __func__);
}


void do_changeAudioTrack(void)
{
    return;
}

MT_BOOL  x_get_av_dec_cap(void *pHandle)
{
    FILE_SEQ_T *p_file_seq = pHandle;
    int ves_buf_size = 0;
    int aes_buf_size = 0;

    vdec_get_es_buf_size(p_file_seq->pb_internal, (u32 *)&ves_buf_size);
    p_file_seq->dec_cap.max_ves_num = ves_buf_size << 10;
    aud_file_gettotalesbuffer_vsb(p_file_seq->pb_internal, (u32 *)&aes_buf_size);
    p_file_seq->dec_cap.max_aes_num = aes_buf_size;
    MLOGI("[%s] ves %d aes %d...\n", __func__,
          p_file_seq->dec_cap.max_ves_num, p_file_seq->dec_cap.max_aes_num);

    return MT_TRUE;
}

/*
 * unmap ves/aes  tmp buf which was allocated in linux kernel
 */
void x_unmap_es_buffer(void *pHandle)
{
    FILE_SEQ_T *p_file_seq = pHandle;
    MLOGD("[%s] start start ...\n", __func__);
    audio_free_es_tmp_buf(p_file_seq->p_audio_dev);
    MLOGD("[%s] end end ...\n", __func__);
}

int x_start_av_decoder(void *pHandle)
{
    FILE_SEQ_T *p_file_seq = pHandle;
    int ret = 0;

    MLOGD("[%s] start start ...\n", __func__);
    p_file_seq->isAudioDecoderStart = MT_FALSE;
    if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
        ret = set_audio_param_to_vsb();
        if (ret != 0) {
            MLOGD("[%s] set_audio_param_to_vsb fail\n", __func__);
        } else {
            p_file_seq->isAudioDecoderStart = MT_TRUE;
        }
    }

    MLOGD("[%s] p_file_seq->m_audio_codec_type[%d]\n", __func__, p_file_seq->m_audio_codec_type);
    /*
     *  require necessary resource for video firmware
     */
    p_file_seq->isVideoDecoderStart = MT_FALSE;
    if (p_file_seq->only_audio_mode == MT_FALSE) {
        ret = player_vdec_start(p_file_seq, p_file_seq->m_video_codec_type, 2);
        if (ret == 0) {
            p_file_seq->isVideoDecoderStart = MT_TRUE;
            vdec_set_dec_frm_type(p_file_seq->p_vdec_dev, MT_UNF_DEC_FRM_ALL);
            MLOGD("[%s] p_file_seq->m_video_codec_type:%d\n", __func__, p_file_seq->m_video_codec_type);
        } else {
            MLOGD("[%s] player_vdec_start fail\n", __func__);
        }
    }

    MLOGD("[%s] end end ...\n", __func__);
    return ret;
}

static void set_fp_video_info(
    FILE_SEQ_T *p_file_seq,
    MT_UNF_VIDEO_FRAME_INFO_S *p_video_frame_info)
{
    p_file_seq->video_disp_w = p_video_frame_info->u32Width;
    p_file_seq->video_disp_h = p_video_frame_info->u32Height;

    p_file_seq->video_whfps.video_disp_w = p_file_seq->video_disp_w;
    p_file_seq->video_whfps.video_disp_h = p_file_seq->video_disp_h;
    p_file_seq->video_whfps.video_fps    = p_file_seq->video_fps;

    MLOGI("Gain video info from driver [%dx%d]\n",
        p_video_frame_info->u32Width, p_video_frame_info->u32Height);
}

static mt_s32 mt_unf_event_callback(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, unsigned long ulong_para)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    switch (enEvent) {
        case MT_UNF_AVPLAY_EVENT_EOS: {
            MLOGI("[%s] drv send MT_UNF_AVPLAY_EVENT_EOS!!\n", __func__);
            p_file_seq->is_play_to_end = 1;
            break;
        }
        case MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT:{
            demuxer_t *p_demuxer = (demuxer_t *)p_file_seq->p_demuxer;
            sh_video_t *p_sh_video = p_demuxer->video->sh;
            MLOGI("[%s] drv send FILE_PLAYBACK_UNSUPPORT_VIDEO and force stop!!\n", __func__);
            if(p_file_seq->event_cb) {
                p_file_seq->event_cb(FILE_PLAYBACK_UNSUPPORT_VIDEO, p_sh_video->codec_id);
            }
            p_file_seq->force_stop(p_file_seq);
            break;
        }
        case MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT:
            MLOGI("[%s] drv send MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT and force stop!!\n", __func__);
            if(p_file_seq->event_cb) {
                p_file_seq->event_cb(FILE_PLAYBACK_UNSUPPORT_AUDIO, pbi->audio.codec_id);
            }
            p_file_seq->force_stop(p_file_seq);
            break;
        case MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME: {
            MT_UNF_VIDEO_FRAME_INFO_S *p_video_frame_info = (MT_UNF_VIDEO_FRAME_INFO_S *)ulong_para;
            int fps_integer = (int) p_video_frame_info->stFrameRate.u32fpsInteger;
            if (p_file_seq->video_fps != fps_integer && fps_integer != 0) {
               p_file_seq->video_fps = fps_integer;
            }
            if (0 == p_file_seq->vdec_resolution_callback ||
                p_file_seq->video_disp_w != p_video_frame_info->u32Width ||
                p_file_seq->video_disp_h != p_video_frame_info->u32Height) {
                if (p_video_frame_info->u32Width > 0 && p_video_frame_info->u32Height > 0) {
                    MLOGI("[%s] driver send MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME!!video_fps:%d \n", __func__, p_file_seq->video_fps);
                    set_fp_video_info(p_file_seq, p_video_frame_info);
                    p_file_seq->vdec_resolution_callback = 1;
                    SetFlag(p_file_seq->internal_event, GET_ES_VIDEO_W_H);
                } else {
                    MLOGI("[%s] driver report NEW_VID_FRAME error(%d x %d)\n",
                        __func__, p_video_frame_info->u32Width, p_video_frame_info->u32Height);
                }
            }
            mlzp_mutex_lock(pbi->video.mutex);
            pbi->video.pkt_cnt_in_buf--;
            mlzp_mutex_unlock(pbi->video.mutex);
            notify_video_pts((s64)p_video_frame_info->u64Pts);
            pbi->video_decoder_over = MT_FALSE;
            if(p_file_seq->event_cb) {
                p_file_seq->event_cb(FILE_PLAYBACK_PLAY_NEW_VID_FRAME, 0);
            }
            break;
        }
        case MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED:{
            pbi->video_decoder_over = MT_TRUE;
            break;
        }
        default:
            MLOGD("%s un handle event %d!!!\n", __func__, enEvent);
            break;
    }
    return 0;
}

int mtavsf_msg_callback(
    void *restrict ctx, const int type, void *data)
{
    if (!ctx) {
        return -1;
    }

    FILE_SEQ_T *p_file_seq = ctx;
    if (MTAVSTREAM_MSG_CDM_KEXPIRED == type) {
        MLOGE("CDM Key expired\n");
        // p_file_seq->is_play_to_end = 1;
    }
    return 0;
}

void fpi_unregister_avplay_event(FILE_SEQ_T *p_file_seq)
{
    file_seq_unregister_event(MT_UNF_AVPLAY_EVENT_EOS);
    file_seq_unregister_event(MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT);
    file_seq_unregister_event(MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT);
    file_seq_unregister_event(MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME);
    file_seq_unregister_event(MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED);
}

void fpi_register_avplay_event(FILE_SEQ_T *p_file_seq)
{
    p_file_seq->vdec_resolution_callback = 0;

    fpi_unregister_avplay_event(p_file_seq);
    file_seq_register_event(MT_UNF_AVPLAY_EVENT_EOS          , mt_unf_event_callback);
    file_seq_register_event(MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT, mt_unf_event_callback);
    file_seq_register_event(MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT, mt_unf_event_callback);
    file_seq_register_event(MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME, mt_unf_event_callback);
    file_seq_register_event(MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED, mt_unf_event_callback);
}

void x_init_av_device(void *pHandle)
{
    FILE_SEQ_T *p_file_seq = pHandle;
    MLOGD("[%s] start start ...\n", __func__);

    static int inited = 0;
    fpi_register_avplay_event(p_file_seq);
    if (inited) {
        p_file_seq->p_audio_dev = NULL;
        p_file_seq->p_vdec_dev  = NULL;
        p_file_seq->p_disp_dev  = NULL;
        if (HA_AUDIO_ID_INVALID != p_file_seq->m_audio_codec_type) {
            p_file_seq->p_audio_dev = (void *)dev_find_identifier(NULL, 0, 0);
            MLOGD("Init audio success\n");
        }

        if (p_file_seq->only_audio_mode == MT_FALSE) {
            p_file_seq->p_vdec_dev = (void *)dev_find_identifier(NULL, 0, 0);
            p_file_seq->p_disp_dev = (void *)dev_find_identifier(NULL, 0, 0);
            MLOGD("Init vdec & disply success\n");
        }
        return;
    }

    inited = 1;
    if (p_file_seq->only_audio_mode == MT_FALSE) {
        /* pti */
        p_file_seq->p_disp_dev = (void *)dev_find_identifier(NULL, 0, 0);
        MLOGD("Init disply success\n");
        /* vdec */
        p_file_seq->p_vdec_dev = (void *)dev_find_identifier(NULL, 0, 0);
        dev_open(p_file_seq->p_vdec_dev, NULL);
        MLOGD("Init vdec success\n");
    }

    /* audio */
    if (HA_AUDIO_ID_INVALID != p_file_seq->m_audio_codec_type) {
        p_file_seq->p_audio_dev = (void *)dev_find_identifier(NULL, 0, 0);
        /*ret =*/ dev_open(p_file_seq->p_audio_dev, NULL);
        MLOGD("Init audio  success\n");
    }

    MLOGD("[%s] end end ...\n", __func__);
}

MT_BOOL adec_push_audio_es(u8 *start_a, int audioFrameLen, u64 apts, u32 eos)
{
    int ret = 0;
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();

    ret =  aud_file_pushesbuffer_vsb(
        p_file_seq->pb_internal, start_a, audioFrameLen, apts, eos);

    /*
        bug#27092:
            when return MTAVSF_DRM_DECRYPT_FAIL(-2),notify up layer.
    */
    if (ret == MTAVSF_DRM_DECRYPT_FAIL) {
        if (p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_DRM_DECRYPT_FAIL,0);
        }
    }

    return MT_TRUE;
}

/*******************************************************************************************
 *   //xinwei update 20231007
 *************************************************************************************************/
static void  update_bps(mt_u32 all, mt_u32 used)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    u32   cur_BPS            = 0;
    u32   cur_percentage     = 0;
    u32   bps_percent_val     = 0;
    u32   cur_tick;
    u32   par;

    if (p_file_seq) {
        PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

        if(used == p_internal->net_buffer.last_used) {
            return;
        }

        cur_tick = mtos_ticks_get();

        cur_BPS = (used - p_internal->net_buffer.start_used)/(((cur_tick - p_internal->net_buffer.start_tick) * 10));

        if(p_internal->net_buffer.resolution_type == 0) {
            par = 500;                      // 20%
        } else if (p_internal->net_buffer.resolution_type == 1) {
            par = 333;//1000/3;             // 30%
        } else {
            par = 125;                      // 40%
        }

        if(all) {
            cur_percentage = par*used/(all);
        }

        bps_percent_val = (cur_BPS & 0x0000ffff) | (cur_percentage << 16);
        MLOGD(">>>11 [%d KB/sec]  [%d]!!!<<<\n", cur_BPS, cur_percentage);
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_UPDATE_BPS, bps_percent_val);

        p_internal->net_buffer.last_used = used;
    }
}
/*
 * The audio packet maybe come from  demuxer's buffer of ffmepg
 * preload buffer or audio buffering buffer
 */
extern int hls_fps_first ;
extern int hls_update_fps;
extern int hls_detect_fps;
extern int hls_fps_done;
extern int detect_pks_num;
static void handle_file_vhdr(FILE_SEQ_T *p_file_seq)
{
    demux_stream_t *ds_a = NULL;
    demux_stream_t *ds_v = NULL;
    int cur_video_packet_size = 0;
    unsigned int videoFreeSpaceSize = 0;
    int i = 0;
    unsigned int state = -1;
    unsigned char *p_hdr = NULL;
    int vol_len = 0;
    int has_vol = 0;

    ds_a = p_file_seq->p_cur_ds_audio;
    ds_v = p_file_seq->p_cur_ds_video;
    cur_video_packet_size = ds_get_packet(ds_v, &(p_file_seq->p_v_pkt_start));
    /*cur_audio_packet_size =*/ ds_get_packet(ds_a, &(p_file_seq->p_a_pkt_start));

    if (ds_a->pts < 0.0) {
        ds_a->pts = 0.0;
    }

    if (ds_v->pts < 0.0) {
        ds_v->pts = 0.0;
    }

    if ((first_vpts) < -1) {
        first_vpts = ds_v->pts;
    }

    p_file_seq->orig_apts =  ds_a->pts * 1000.0;
    p_file_seq->sys_vpts = TIME_BASE * (p_file_seq->orig_vpts);
    p_hdr = p_file_seq->p_v_pkt_start;

    if (p_file_seq->m_video_codec_type == MT_UNF_VCODEC_TYPE_H264) {
        for (i = 0; i < cur_video_packet_size; i++) {
            state = (state << 8) | (p_hdr[i] & 0xbf);
            if (state == 0x127) {
                has_vol = 1;
            }
            if ((state == 0x125 || state == 0x121) && has_vol) {
                vol_len = i - 3;
                break;
            }
        }
    } else if (p_file_seq->m_video_codec_type == 0) {
        for (i = 0; i < cur_video_packet_size; i++) {
            state = (state << 8) | (p_hdr[i] & 0xff);
            if (state == 0x1b3) {
                has_vol = 1;
            }
            if ((state == 0x100 || state == 0x1b8) && has_vol) {
                vol_len = i - 3;
                break;
            }
        }
    }

    if (vol_len > 6) {
        vdec_get_es_buf_space(p_file_seq->pb_internal, (u32 *)&videoFreeSpaceSize);
        if (videoFreeSpaceSize > 100) {
            vdec_dec_push_es(p_file_seq->pb_internal, p_file_seq->p_v_pkt_start, vol_len, p_file_seq->sys_vpts, 0);
        }
    }

    MLOGD("[%s]has_vol[%d],vol_len[%d],free[%d]\n", __func__, has_vol, vol_len, videoFreeSpaceSize);
}

static void reset_audio_codec_info(
    FILE_SEQ_T *p_file_seq, FILE_SEQ_AUDIO_T *ainfo, sh_audio_t *sha)
{
    unsigned int need_reset =
         ainfo->sample_rate != sha->samplerate || ainfo->channels != sha->channels;

    if (need_reset) {
        ainfo->sample_rate = sha->samplerate;
        ainfo->channels = sha->channels;
    }
}

static int change_demux_stream_header(
    demux_stream_t *ds, AVPacket *pkt)
{
    demuxer_t *demux = ds->demuxer;
    if (!pkt || DEMUXER_TYPE_LAVF != demux->type) {
        return 0;
    }

    int stream_id = pkt->stream_index;
    /* drain buffering packet when change playlist, ds id can be not equal to stream id */
    if (stream_id != ds->id) {
        return 0;
    }

    void *sh = NULL;
    if (demux->audio == ds) {
        sh = demux->a_streams[stream_id];
    } else if (demux->video == ds) {
        sh = demux->v_streams[stream_id];
    } else if (demux->sub == ds) {
        sh = demux->s_streams[stream_id];
    }
    if (!sh || (ds->sh == sh)) {
        return 0;
    }

    ds->sh = sh;
    return 1;
}

static void reset_vcodec_info_l(
    FILE_SEQ_VIDEO_T *vinfo, sh_video_t *shv)
{
    unsigned int need_reset =
        vinfo->width      != shv->disp_w   ||
        vinfo->height     != shv->disp_h   ||
        vinfo->codec_id   != shv->codec_id ||
        vinfo->frame_rate.num != shv->fps.num ||
        vinfo->frame_rate.den != shv->fps.den ||
        vinfo->codec_extradata      != shv->codec_extradata ||
        vinfo->codec_extradata_size != shv->codec_extradata_size;

    if (need_reset) {
        vinfo->width      = shv->disp_w;
        vinfo->height     = shv->disp_h;
        vinfo->codec_id   = shv->codec_id;
        vinfo->frame_rate.num = shv->fps.num;
        vinfo->frame_rate.den = shv->fps.den;
        vinfo->codec_extradata      = shv->codec_extradata;
        vinfo->codec_extradata_size = shv->codec_extradata_size;
    }
}

static void reset_video_codec_info(FILE_SEQ_T *p_file_seq)
{
    demux_stream_t      *dsv = (demux_stream_t *) p_file_seq->p_cur_ds_video;
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    FILE_SEQ_VIDEO_T  *vinfo = &(pbi->video);

    int changed = change_demux_stream_header(dsv, vinfo->ff_av_pkt);
    if (changed) {
        ds_get_video_codec_type(dsv,
            (int *) &p_file_seq->m_video_codec_type,
            &p_file_seq->video_pid, &p_file_seq->pcr_pid);
        pbi->audio.state = FPBI_STATE_PLAYLIST_CHANGED;
        pbi->video.state = FPBI_STATE_PLAYLIST_CHANGED;
    }
    reset_vcodec_info_l(vinfo, (sh_video_t *) dsv->sh);
}

int fpi_get_ds_audio_packet(FILE_SEQ_T *p_file_seq,
    void *ds, unsigned char **start, uint8_t **extra_buf, uint8_t *extra_size)
{
    demux_stream_t *ds_a = ds;
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if (extra_buf) {
        *extra_buf = NULL;
    }
    if (extra_size) {
        *extra_size = 0;
    }

    int pkt_size = ds_get_packet_audio(ds_a, start, extra_buf, (uint8_t *) extra_size);
    if (pkt_size <= 0 || !*start) {
        pbi->audio.ff_av_pkt = NULL;
    } else {
        ((PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal))->audio_get_pkt_fail = MT_FALSE;
        pbi->audio.is_secure = pbi->audio.is_secure ? 1 : ds_a->is_secure ? MT_TRUE : MT_FALSE;
        pbi->audio.ff_av_pkt = ds_a->current ? ds_a->current->ff_av_pkt : NULL;
        reset_audio_codec_info(p_file_seq, (FILE_SEQ_AUDIO_T *) &pbi->audio, (sh_audio_t *) ds_a->sh);
    }

    return pkt_size;
}

int fpi_get_ds_video_packet(
    FILE_SEQ_T *p_file_seq,
    void *ds, unsigned char **start, int8_t speed)
{
    demux_stream_t *ds_v = ds;
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    int pkt_size = ds_get_packet_video(ds_v, start, speed);
    if (pkt_size <= 0 || !*start) {
        MLOGE("[%s][ERROR] cur_video_packet_size [%d]!!\n", __func__, pkt_size);
        pbi->video_get_pkt_fail = MT_TRUE;
        pbi->video.ff_av_pkt = NULL;
    } else {
        pbi->video_get_pkt_fail = MT_FALSE;
        pbi->video.is_secure = pbi->video.is_secure ? 1 : ds_v->is_secure ? MT_TRUE : MT_FALSE;
        pbi->video.ff_av_pkt = ds_v->current ? ds_v->current->ff_av_pkt : NULL;
    }
    return pkt_size;
}

static int ds_get_buffer_pkt_video(demux_stream_t *ds, unsigned char **start, int8_t speed)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    MT_ASSERT(p_file_seq != NULL);
    es_pkt_data_t *p_node =  NULL;

    *start = NULL;

    if (p_file_seq->total_path == 1) {
        return fpi_get_ds_video_packet(p_file_seq, ds, start, speed);
    }

    if (ds->eof) {
        *start = NULL;
        return 0;
    }

    if /*while*/ (p_file_seq->p_video_buf == NULL) {    // for tsscan
        return 0;
    }

    p_node = p_file_seq->p_video_buf;

    while (p_node->next_pkt == NULL) {
        if (p_node->eof) {
            ds->eof = 1;
            MLOGD("get pkt, video is eof\n");
            break;
        } else { //wait data come
            return 0;
        }
    }

    ds->pts = p_node->pts;
    *start = p_node->data;
    p_file_seq->p_video_buf = p_file_seq->p_video_buf->next_pkt;
    return p_node->data_len;
}

int get_video_es_packet(FILE_SEQ_T *p_file_seq, u32 *orig_vpts)
{
    demux_stream_t *ds_v = NULL;
    ds_v = p_file_seq->p_cur_ds_video;
    int cur_video_packet_size = 0;

    if (!ds_v->eof) {
        cur_video_packet_size = ds_get_buffer_pkt_video(ds_v, &(p_file_seq->p_v_pkt_start), p_file_seq->cur_speed);

        if (cur_video_packet_size <= 0 ||
            p_file_seq->p_v_pkt_start == NULL) {
            return -1;
        }
        *orig_vpts = ds_v->pts * 1000;
        //fix bug 100046
        //end fix bug 100046
    } else {
        p_file_seq->isVideoEsEnd = MT_TRUE;
        p_file_seq->isTrickPlay = MT_FALSE;
    }

    return cur_video_packet_size;
}



int get_pack_magic_num(FILE_SEQ_T *p_file_seq, int pkt_size, unsigned int orig_vpts)
{
    unsigned char *tmp = NULL;
    int ret = -1;
    if (pkt_size > 10) {
        tmp = p_file_seq->p_v_pkt_start;
        ret = (tmp[0] | (tmp[1] << 8) | (tmp[2] << 16) | ((unsigned int)tmp[3] << 24));
        tmp = p_file_seq->p_v_pkt_start + (pkt_size >> 1);
        ret ^= (tmp[0] | (tmp[1] << 8) | (tmp[2] << 16) | ((unsigned int)tmp[3] << 24));
        tmp = p_file_seq->p_v_pkt_start + pkt_size - 4;
        ret ^= (tmp[0] | (tmp[1] << 8) | (tmp[2] << 16) | ((unsigned int)tmp[3] << 24));
        ret ^= pkt_size + orig_vpts;
    } else {
        ret = pkt_size + orig_vpts;
    }
    return ret;
}

static int handle_ves_push_normal_frame(FILE_SEQ_T *p_file_seq, unsigned int push_ves_size)
{
    demux_stream_t *dsv = (demux_stream_t *) p_file_seq->p_cur_ds_video;
    u64 push_pts = (MT_TRUE == dsv->pts_valide) ? (u64)(p_file_seq->sys_vpts) : MP_NOPTS_VALUE;

    reset_video_codec_info(p_file_seq);
    return vdec_dec_push_es(p_file_seq->pb_internal,
        (p_file_seq->p_v_pkt_start + (p_file_seq->m_tmp_ves_buf_pos)), push_ves_size, push_pts, 0);
}

static int handle_ves_push_vpx_frame(FILE_SEQ_T *p_file_seq, int pkt_size)
{
    int ret;
    demux_stream_t      *dsv   = (demux_stream_t *) p_file_seq->p_cur_ds_video;
    PLAYBACK_INTERNAL_T *pbi   = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    sh_video_t          *shv   = (sh_video_t *)dsv->sh;
    FILE_SEQ_VIDEO_T    *vinfo = &(pbi->video);

    u64 push_pts = (MT_TRUE == dsv->pts_valide) ? (u64)(p_file_seq->sys_vpts) : MP_NOPTS_VALUE;
    if ((vinfo->frame_rate.num != shv->fps.num ||
         vinfo->frame_rate.den != shv->fps.den) && (0 != shv->fps.num) && (0 != shv->fps.den)) {
        vinfo->need_insert_header = 1;
    }

    if (vinfo->need_insert_header) {
        reset_video_codec_info(p_file_seq);
        vinfo->duration = p_file_seq->file_duration;
    }

    MLOGD("Vpx pkt is%s keyframe, pts:%llu, size:%d\n",
          dsv->flags ? "" : " not", p_file_seq->sys_vpts, pkt_size);
    ret = vdec_dec_push_vpx_es((void *)pbi,
        p_file_seq->p_v_pkt_start, pkt_size, push_pts, 0);
    return ret;
}

void handle_video_es(FILE_SEQ_T *p_file_seq)
{
    demux_stream_t *ds_a = NULL;
    demux_stream_t *ds_v = NULL;
    demux_stream_t *ds_sub = NULL;
    ds_a = p_file_seq->p_cur_ds_audio;
    ds_v = p_file_seq->p_cur_ds_video;
    ds_sub = p_file_seq->p_cur_ds_sub;
    int cur_video_packet_size = 0;
    demuxer_t *demuxer = (demuxer_t *)(p_file_seq->p_demuxer);
    int record_push_size = 0;
    int push_cnt = 0;
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if (!list_I_vpts) {
        list_I_vpts = InitList();
    }

    if (hls_update_fps
        && ((!hls_fps_first && (hls_detect_fps == 25 || hls_detect_fps == 30))
            || (hls_fps_done && hls_detect_fps > 20 && hls_detect_fps < 31))) {
        MLOGD("[%s]f[%d],fps[%d],done[%d],pks[%d]\n", __func__, hls_fps_first, hls_detect_fps, hls_fps_done, detect_pks_num);
        hls_fps_first = 1;

        p_file_seq->video_fps = hls_detect_fps;

        if (hls_fps_done) {
            hls_update_fps = 0;
            hls_detect_fps = 0;
        }
    }

PUSH_VIDEO_AGAIN:
    if (p_file_seq->needNewVideoData || (p_file_seq->m_play_state == FILE_SEQ_EXIT)) {
        if (!ds_v->eof) {
            if(p_file_seq->left_v_pkt_bytes == 0) {
                cur_video_packet_size = fpi_get_ds_video_packet(p_file_seq,
                    ds_v, &(p_file_seq->p_v_pkt_start), p_file_seq->cur_speed);
            } else {
                cur_video_packet_size = p_file_seq->left_v_pkt_bytes;
            }
            if (cur_video_packet_size <= 0 ||
                p_file_seq->p_v_pkt_start == NULL) {
                MLOGE("[%s][ERROR] cur_video_packet_size [%d]!!\n", __func__, cur_video_packet_size);
                pbi->video_get_pkt_fail = MT_TRUE;
                return;
            } else {
                pbi->video_get_pkt_fail = MT_FALSE;
            }

        } else {
            unsigned char tmp_data[1024] = {0};
            memset(tmp_data, 0, 1024);
            pbi->video.ff_av_pkt = NULL;
            /*+++++++++++Start+++++++++++*/
            /* To push the original es out in wb_case_9902, and can't get the EOS any more by MT_UNF_AVPLAY_IsBuffEmpty */
            /* and MT_UNF_AVPLAY_GetStatusInfo with this way*/
            vdec_dec_push_es(p_file_seq->pb_internal, tmp_data, 1024, (u64)0, 1);
            p_file_seq->isVideoEsEnd = MT_TRUE;
            p_file_seq->isTrickPlay = MT_FALSE;
            MLOGI("[%s] >>>Video Stream Demuxer Is Finished !!<<<\n", __func__);
            return;
        }

        if ((first_vpts) < -1) {
            first_vpts = ds_v->pts;
        }

        if (ds_a->pts < 0.0) {
            ds_a->pts = 0.0;
        }

        if (ds_v->pts < 0.0) {
            ds_v->pts = 0.0;
        }

        if (ds_sub->pts < 0.0) {
            ds_sub->pts = 0.0;
        }

        if (p_file_seq->is_ts) {
            pbi->gop_seg_num = ds_v->gop_seg_num;
        }

        //p_file_seq->orig_apts = ds_a->pts * TIME_BASE;
        p_file_seq->orig_vpts = ds_v->pts * TIME_BASE;
        p_file_seq->orig_spts = ds_sub->pts * TIME_BASE;

        p_file_seq->sys_apts = TIME_BASE * (p_file_seq->orig_apts);
        p_file_seq->sys_vpts = TIME_BASE * (p_file_seq->orig_vpts);
        if (p_file_seq->orig_vpts > p_file_seq->max_video_pts) {
            p_file_seq->max_video_pts = p_file_seq->orig_vpts;

            /* from trcik play mode to normal play mode */
            if (p_file_seq->isTrickToNormal) {
                p_file_seq->ref_first_video_pts = p_file_seq->max_video_pts;
                p_file_seq->totalVesNum = 0;
            }
        }

        /*
         *  if  cur_video_packet_size is less than 1016 , zero padding should be filled at seg_buf
         *  the fields of vpts/dts ocuppy 8 bytes
         */
        p_file_seq->left_v_pkt_bytes = cur_video_packet_size;
        p_file_seq->m_tmp_ves_buf_pos = 0;
        p_file_seq->needNewVideoData = MT_FALSE;
    }

    /* push ves to dma ring buffer */
    unsigned int videoFreeSpaceSize = 0;
    unsigned int push_ves_size = 0;
    int ret;

    if (p_file_seq->m_video_codec_type == 0xffff) {
        p_file_seq->left_v_pkt_bytes = 0;
        p_file_seq->isVideoBufferFull = MT_FALSE;
        p_file_seq->needNewVideoData = MT_TRUE;
    } else {
        /*  length  of video es to be pushed should not be */
        push_ves_size = p_file_seq->left_v_pkt_bytes;
        mtos_sem_take((os_sem_t *)(&(pbi->seek_mutex)), 0);

        if (0 == p_file_seq->m_tmp_ves_buf_pos &&
            (MT_UNF_VCODEC_TYPE_VP8 == p_file_seq->m_video_codec_type ||
             MT_UNF_VCODEC_TYPE_VP9 == p_file_seq->m_video_codec_type)) {
            ret = push_ves_size = handle_ves_push_vpx_frame(p_file_seq, push_ves_size);
        } else {
            ret = handle_ves_push_normal_frame(p_file_seq, push_ves_size);
        }

        /*
        bug#27092: when return MTAVSF_DRM_DECRYPT_FAIL(-2),notify up layer.
        although return failed,we still discard data to avoid dead loop.
        */
        if (ret == MTAVSF_DRM_DECRYPT_FAIL) {
            if (p_file_seq->event_cb) {
                p_file_seq->event_cb(FILE_PLAYBACK_DRM_DECRYPT_FAIL,0);
            }
        }
        //if(ret != MT_FAILURE)
        /*bug#27092 end*/
        {
            record_push_size += push_ves_size;

            pbi->vitual_video_es_cur_size -= push_ves_size;
            p_file_seq->totalVesNum += push_ves_size;
            p_file_seq->left_v_pkt_bytes -= push_ves_size;
            p_file_seq->m_tmp_ves_buf_pos += push_ves_size;

            if (p_file_seq->left_v_pkt_bytes == 0) {
                p_file_seq->isVideoBufferFull = MT_FALSE;
                p_file_seq->needNewVideoData = MT_TRUE;
                p_file_seq->available_ves_bytes = p_file_seq->dec_cap.max_ves_num - videoFreeSpaceSize;
                p_file_seq->m_tmp_ves_buf_pos = 0;
                if ((ds_v->flags)) {
                    list_rwlock_lock();
                    AddFromEnd(list_I_vpts, (u32)(ds_v->pts * 1000));
                    list_rwlock_unlock();
                }

                if (list_I_vpts && ds_v->flags) {
                    list_rwlock_lock();
                    Show(list_I_vpts);
                    list_rwlock_unlock();
                }
            }
        }
        mtos_sem_give((os_sem_t *)(&(pbi->seek_mutex)));

        if (p_file_seq->orig_vpts != MP_NOPTS_VALUE && p_file_seq->orig_vpts < (p_file_seq->file_start_time * 1000) \
            && p_file_seq->orig_vpts > 0) {
            p_file_seq->file_start_time = p_file_seq->orig_vpts / 1000.0;
            MLOGD("[%s][%d] Update file start time=%d !!!\n", __func__, __LINE__, (int)(p_file_seq->file_start_time * 1000));
        }
#ifdef DRM_SMP_ENABLE
        if ((cur_video_packet_size > 0) && MT_FALSE == pbi->video.is_secure &&
            (0 == MTDrm_GetDecryptType(p_file_seq->drm_eDrmIndex))) {
            pbi->pack_magic = get_pack_magic_num(p_file_seq, cur_video_packet_size, p_file_seq->orig_vpts);
        }
#else
        if (cur_video_packet_size > 0) {
            pbi->pack_magic = get_pack_magic_num(p_file_seq, cur_video_packet_size, p_file_seq->orig_vpts);
        }
#endif

        if (DEMUXER_TYPE_MPEG_PS == demuxer->type && push_ves_size > 1500 && push_ves_size < 2500)
        {
          static double  mpeg_ps_last_vpts = 1;
          if(fabs(mpeg_ps_last_vpts - p_file_seq->orig_vpts) == 0)    // xingwei @20190321 to fix 108788 , send up to one frame of data at a time
          {
              if(p_file_seq->cur_speed == TS_SEQ_NORMAL_PLAY)     // xingwei @20190313 bug 107877
              {
                  if(record_push_size < 200*1024 && push_cnt ++ < 100)
                  {
                      goto PUSH_VIDEO_AGAIN;
                  }
              }
              else if(p_file_seq->cur_speed >= TS_SEQ_REV_FAST_PLAY_2X && p_file_seq->cur_speed <= TS_SEQ_REV_FAST_PLAY_32X)
              {
                 if(push_cnt%2 == 0)
                 {
                   mtos_task_sleep(10-(p_file_seq->cur_speed - TS_SEQ_SLOW_PLAY_4X));
                 }

                  if(record_push_size < 20*1024 && push_cnt ++ < 2)
                      goto PUSH_VIDEO_AGAIN;
              }

          }
          else{
              mpeg_ps_last_vpts = p_file_seq->orig_vpts;
          }
        }
    }
}

static int ds_get_buffer_pkt_audio(demux_stream_t *ds, unsigned char **start, uint8_t **extra_buf, uint8_t *extra_size)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    MT_ASSERT(p_file_seq != NULL);
    es_pkt_data_t *p_node =  NULL;
    //preload_buffer_t * p_audio_buffer = &p_file_seq->preload_audio_buffer;
    *start = NULL;
    *extra_buf = NULL;
    *extra_size = 0;
    //p_file_seq->is_audio_buffering_packet = MT_FALSE;

    /*
    * Secondly,if path total is only one,
    * we fetch audio packet from audio buffer of ffmpeg's demuxer
    *
    */

    if (p_file_seq->total_path == 1) {
        return fpi_get_ds_audio_packet(p_file_seq, (void *)  ds, start, extra_buf, (uint8_t *)extra_size);
    }

    if (ds->eof) {
        *start = NULL;
        return 0;
    }

    if /*while*/ (p_file_seq->p_audio_buf == NULL) {    // for tsscan
        return 0;
    }

    /*
    *  At last,if path total is more than one,
    *  we fetch audio packet from internal buffer of audio preloader.
    */
    p_node = p_file_seq->p_audio_buf;

    while (p_node->next_pkt == NULL) {
        if (p_node->eof) {
            ds->eof = 1;
            MLOGD("get pkt, audio is eof\n");
            break;
        } else { //wait data come
            return 0;
        }
    }

    ds->pts = p_node->pts;
    *start = p_node->data;

    *extra_size = p_node->extra_data_len;

    if (p_node->extra_data_len > 0) {
        static unsigned char aac_head_fp[16];
        //*extra_buf = malloc(p_node->extra_data_len);
        memset(aac_head_fp, 0, 16);
        *extra_buf = aac_head_fp;
        if (p_node->extra_data_len > 16) {
            memcpy(*extra_buf, p_node->extra_data, *extra_size);
        } else {
            MLOGD("%s %d extra size error %d \n", __func__, __LINE__, p_node->extra_data_len);
        }

    }
    p_file_seq->p_audio_buf = p_file_seq->p_audio_buf->next_pkt;
    return p_node->data_len;
}

static void drop_surplus_trick_audio_pkt(demux_stream_t *ds_a, demux_stream_t *ds_v)
{
    /* limit try cny to prevent barricade issue*/
    int retry_cnt      = 0;
    unsigned char *buf = NULL;
    int a_demuxer_type = ds_a->demuxer->file_format;
    const static int MAX_RETRY_CNT = 100;

    /* ffmpeg drop packets in demux */
    if (a_demuxer_type == DEMUXER_TYPE_LAVF) {
        return;
    }

    /* video larger than 0.5 seconds, drop more audio pkt when trick */
    while ((!ds_a->eof)  &&
           ((ds_v->pts - ds_a->pts) > 0.5) && (retry_cnt < MAX_RETRY_CNT)) {
        retry_cnt++;
        (void) ds_get_packet(ds_a, &buf);
    }
}

int get_audio_es_packet(FILE_SEQ_T *p_file_seq, u32 *audio_pts)
{
    PLAYBACK_INTERNAL_T *p_pb_internal =  p_file_seq->pb_internal;
    demux_stream_t *ds_a = p_file_seq->p_cur_ds_audio;
    demux_stream_t *ds_v = p_file_seq->p_cur_ds_video;
    int cur_audio_packet_size = 0;

    /*
    *  get audio paket from  ffmpeg demuxer
    */
    if (p_file_seq->cur_speed != TS_SEQ_NORMAL_PLAY &&
        p_file_seq->cur_speed != TS_SEQ_FAST_PLAY_2X) {
        p_file_seq->left_a_pkt_bytes = 0;
    }

    if (!ds_a->eof &&
        ((p_file_seq->left_a_pkt_bytes == 0) ||
         (p_pb_internal->is_audio_support == MT_FALSE))) {
        if (p_file_seq->isTrickPlay) {
            if ((!ds_a->eof)  && ((ds_a->pts - ds_v->pts) > 0.05)) {
                return -1;
            }
            drop_surplus_trick_audio_pkt(ds_a, ds_v);
        }

        if (ds_a->eof) {
            MLOGD("[%s] end audio es !!! !!!\n", __func__);
        }

        p_file_seq->extra_audio_size = 0;
        p_file_seq->p_extra_aud_buf = NULL;
        cur_audio_packet_size = ds_get_buffer_pkt_audio(ds_a, &(p_file_seq->p_a_pkt_start),
            &(p_file_seq->p_extra_aud_buf), (uint8_t *) & (p_file_seq->extra_audio_size));

        if (cur_audio_packet_size <= 0 || p_file_seq->p_a_pkt_start == NULL) {
            cur_audio_packet_size = 0;
        }
        if (p_file_seq->p_extra_aud_buf && (p_file_seq->extra_audio_size > 0)) {
            p_pb_internal->is_need_extra_audio = 1;
        } else {
            p_pb_internal->is_need_extra_audio = 0;
        }
        p_file_seq->left_a_pkt_bytes = cur_audio_packet_size;
        *audio_pts = ds_a->pts * 1000 ; // ms
    } else if (ds_a->eof) {
        MLOGI("[%s][ok]    >>>> Audio Stream Demuxer Is Finished !!<<<<\n", __func__);
        p_file_seq->isAudioEsEnd = MT_TRUE;
    }
    return cur_audio_packet_size;
}


void setup_audio_pts(FILE_SEQ_T *p_file_seq, int pkt_size)
{
    demux_stream_t *ds_a = NULL;

    ds_a = p_file_seq->p_cur_ds_audio;

    if (ds_a->pts < 0.0) {
        ds_a->pts = 0.0;
    }

    p_file_seq->orig_apts = TIME_BASE * ds_a->pts;

    p_file_seq->sys_apts  = (u64)(TIME_BASE * p_file_seq->orig_apts);

    if (p_file_seq->orig_apts > p_file_seq->max_audio_pts) {
        p_file_seq->max_audio_pts = p_file_seq->orig_apts;

        if (p_file_seq->isTrickToNormal) {
            p_file_seq->ref_first_audio_pts = p_file_seq->max_audio_pts;
            p_file_seq->isTrickToNormal = MT_FALSE;
        }
    }

    /*normal play*/
    if (p_file_seq->isNormalPlay) {
        p_file_seq->needNewAudioData = MT_FALSE;
    } else {
        p_file_seq->needNewAudioData = MT_TRUE;

        if (p_file_seq->p_extra_aud_buf && p_file_seq->extra_audio_size) {
            //free(p_file_seq->p_extra_aud_buf);
            p_file_seq->p_extra_aud_buf = NULL;
            PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
            p_internal->is_need_extra_audio = 0;
            p_file_seq->totalAesNum += (pkt_size + p_file_seq->extra_audio_size);
        }

        p_file_seq->isAudioBufferFull = MT_FALSE;

    }
    return;
}

int push_audio_es_packet(FILE_SEQ_T *p_file_seq)
{
    demux_stream_t *ds_a = p_file_seq->p_cur_ds_audio;
    u64 push_pts = (MT_TRUE == ds_a->pts_valide) ? (u64) (p_file_seq->sys_apts) : MP_NOPTS_VALUE;
    if(ds_a->sample_size && (ds_a->sample_size > p_file_seq->left_a_pkt_bytes)) {
        MLOGD("Change audio pts to zero\n");
        push_pts = 0;
    }

    if (!(p_file_seq->isNormalPlay) ||
        p_file_seq->left_a_pkt_bytes <= 0) {
        return 0;
    }

    if (p_file_seq->m_audio_codec_type == HA_AUDIO_ID_INVALID) {
        p_file_seq->left_a_pkt_bytes = 0;
        p_file_seq->isAudioBufferFull = MT_FALSE;
        p_file_seq->needNewAudioData = MT_TRUE;
        return 0;
    }

    int ret = adec_push_audio_es(p_file_seq->p_a_pkt_start, p_file_seq->left_a_pkt_bytes, push_pts, 0);
    if(ret == MT_TRUE) {
        p_file_seq->left_a_pkt_bytes = 0;
    }

    p_file_seq->needNewAudioData = MT_TRUE;

    return 0;
}

void handle_audio_es(FILE_SEQ_T *p_file_seq)
{
    int audio_pkt_size = 0;
    u32 tmp_pts = 0;
    int ret = 0;

    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)p_file_seq->pb_internal;

    if (p_file_seq->needNewAudioData || (p_file_seq->m_play_state == FILE_SEQ_EXIT)) {
        audio_pkt_size = get_audio_es_packet(p_file_seq, &tmp_pts);
        if (audio_pkt_size <= 0 && p_file_seq->left_a_pkt_bytes == 0) {
            //mtos_printk(">>>Audio Stream Demuxer Is Finished !!<<< audio_pkt_size:%d \n",audio_pkt_size);
            pbi->audio_get_pkt_fail = MT_TRUE;
            return;
        } else {
            pbi->audio_get_pkt_fail = MT_FALSE;
        }
        setup_audio_pts(p_file_seq, audio_pkt_size);
    }

    /* Not normal play, drop this packet */
    /* bug 18887, rollback the audio es modifition, send audio es to drv while 2x speed! */
    if (file_seq_get_reset_state() == 0 && MT_FALSE == p_file_seq->isTrickPlay
        /*&& p_file_seq->cur_speed != TS_SEQ_FAST_PLAY_2X*/) {
        ret = push_audio_es_packet(p_file_seq);
        pbi->vitual_audio_es_cur_size -= audio_pkt_size;
        p_file_seq->last_audio_pts = p_file_seq->orig_apts;
    } else {
        p_file_seq->left_a_pkt_bytes = 0;
        p_file_seq->needNewAudioData =  MT_TRUE;
    }
    if(ret < 0) {
        mtos_task_sleep(30);
    }
    return;
}

extern char *g_subt_ext_param;
void handle_subtitle_data(FILE_SEQ_T *p_file_seq)
{
    int ass_offset = 0;
    MT_BOOL add_ass_sub_header     = MT_FALSE;
    demux_stream_t *ds_a    = p_file_seq->p_cur_ds_audio;
    demux_stream_t *ds_v    = p_file_seq->p_cur_ds_video;
    demux_stream_t *ds_sub  = p_file_seq->p_cur_ds_sub;
    unsigned int cur_sub_packet_size = 0;
    char sub_text_head[200] = {0};
    demux_subtitle *p_subtitle = ((demuxer_t *)(p_file_seq->p_demuxer))->subt_info.subtitle;
    int i = 0;
    int got_pkt_flag = 0;
    int int_head_len = 0;//tizhang@20180822 for 104045
    int ext_head_len = 0;//tizhang@20180822 for 104045
    double orig_endspts = MP_NOPTS_VALUE;

    if (ds_v->eof) {
        goto end;
    }
    ds_sub->id = p_file_seq->subt_id;

    cur_sub_packet_size = ds_get_packet_sub(ds_sub, &(p_file_seq->p_sub_pkt_start), &(p_file_seq->orig_spts), &orig_endspts);
    if (fabs(p_file_seq->orig_spts - MP_NOPTS_VALUE) == 0.0) {
        p_file_seq->orig_spts = ds_sub->pts;
    }
    if (orig_endspts < (p_file_seq->orig_spts + 1.5) || fabs(orig_endspts - MP_NOPTS_VALUE) == 0.0 || got_pkt_flag) {
        orig_endspts = p_file_seq->orig_spts + 1.5;
    } else if (orig_endspts > (p_file_seq->orig_spts + 10.0)) {
        orig_endspts = p_file_seq->orig_spts + 10.0;
    }
    if (cur_sub_packet_size == (unsigned int)-1) {
        goto end;
    }

    if (p_file_seq->p_sub_fifo_handle == NULL) {
        void *p_tmp = NULL;
        p_tmp = mtos_malloc(SUB_FIFO_LEN);

        p_file_seq->p_sub_fifo_handle = init_fifo_kw(p_tmp, SUB_FIFO_LEN);
    }

    int_head_len = 8;
    for (i = 0; i < DEMUX_SUBTITLE_CNT; i++) {
        if (ds_sub->id == p_subtitle[i].id) {
            int sh, sm, ss, eh, em, es;
            int sc = (int)(10 * p_file_seq->orig_spts);
            int ec = (int)(10 * orig_endspts);
            if ((0 == strcmp(p_subtitle[i].code, "text")) || (0 == strcmp(p_subtitle[i].code, "movtext"))) {
                SUB_TIMECODE_CONV(sh, sm, ss, sc, eh, em, es, ec);
                sprintf(sub_text_head, "%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\r\n",
                        sh, sm, ss, sc, eh, em, es, ec);
                ext_head_len = strlen(sub_text_head);
                MLOGI("sub header000 [%s] \n", sub_text_head);
            } else if (0 == strcmp(p_subtitle[i].code, "vobsub")) {
                SUB_TIMECODE_CONV(sh, sm, ss, sc, eh, em, es, ec);
                sprintf(sub_text_head, "fmt:vobsub:%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\r\n%s",
                        sh, sm, ss, sc, eh, em, es, ec, g_subt_ext_param);
                ext_head_len = strlen(sub_text_head) + 1;
                MLOGI("sub header111 [%s], ext_len=%d \n", sub_text_head, ext_head_len);
            } else if (0 == strcmp(p_subtitle[i].code, "dvb-teletext") || 0 == strcmp(p_subtitle[i].code, "dvb")) {
                SUB_TIMECODE_CONV(sh, sm, ss, sc, eh, em, es, ec);
                sprintf(sub_text_head, "fmt:%s:%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\r\n",
                        p_subtitle[i].code, sh, sm, ss, sc, eh, em, es, ec);
                ext_head_len = strlen(sub_text_head) + 1;
                MLOGI("sub header222 [%s], ext_len=%d \n", sub_text_head, ext_head_len);
            } else if ((0 == strcmp(p_subtitle[i].code, "ass")) || (0 == strcmp(p_subtitle[i].code, "ssa"))) {
                unsigned char *ptr = p_file_seq->p_sub_pkt_start, *end = ptr + cur_sub_packet_size;
                char layer[32] = {0};
                int j = 0;
                for (; *ptr != ',' && ptr < end - 1; ptr++) {
                    ass_offset ++;
                }
                if (*ptr == ',') {
                    ptr++;
                }
                ass_offset ++;
                for (; *ptr != ',' && ptr < end - 1; ptr++) {
                    layer[j] = *ptr;
                    j++;
                    ass_offset++;
                }
                layer[j] = '\0';
                if (*ptr == ',') {
                    SUB_TIMECODE_CONV(sh, sm, ss, sc, eh, em, es, ec);
                    ass_offset++;
                    add_ass_sub_header = MT_TRUE;
                    sprintf(sub_text_head, "Dialogue: %s,%d:%02d:%02d.%02d,%d:%02d:%02d.%02d,",
                            layer, sh, sm, ss, sc, eh, em, es, ec);
                    ext_head_len = strlen(sub_text_head);
                }
            }
            break;
        }
    }

    SUBT_DATA *subt_data = (SUBT_DATA *) mtos_malloc(sizeof(SUBT_DATA));
    if (subt_data == NULL) {
        printf("%s %d\n", __func__, __LINE__);
        return;
    }
    subt_data->data = mtos_malloc(cur_sub_packet_size + int_head_len + ext_head_len);
    if (subt_data->data == NULL) {
        mtos_free(subt_data);

        printf("%s %d\n", __func__, __LINE__);
        return;
    }

    if (subt_data && subt_data->data) {
        memset(subt_data->data, 0, cur_sub_packet_size + int_head_len + ext_head_len);

        subt_data->pts = p_file_seq->orig_spts;
        subt_data->size = cur_sub_packet_size + ext_head_len;
        subt_data->data[0] = ((subt_data->size) & 0xff000000) >> 24;
        subt_data->data[1] = ((subt_data->size) & 0xff0000) >> 16;
        subt_data->data[2] = ((subt_data->size) & 0xff00) >> 8;
        subt_data->data[3] = ((subt_data->size)) & 0xff;
        subt_data->data[4] = ((subt_data->pts) & 0xff000000) >> 24;
        subt_data->data[5] = ((subt_data->pts) & 0xff0000) >> 16;
        subt_data->data[6] = ((subt_data->pts) & 0xff00) >> 8;
        subt_data->data[7] = ((subt_data->pts)) & 0xff;
        if (ext_head_len) {
            strncpy((void *)(subt_data->data + int_head_len), sub_text_head, ext_head_len);
            if (add_ass_sub_header) {
                memcpy(subt_data->data + int_head_len + ext_head_len, p_file_seq->p_sub_pkt_start + ass_offset, cur_sub_packet_size - ass_offset);
            } else {
                memcpy(subt_data->data + int_head_len + ext_head_len, p_file_seq->p_sub_pkt_start, cur_sub_packet_size);
            }
        } else {
            memcpy(subt_data->data + int_head_len, p_file_seq->p_sub_pkt_start, cur_sub_packet_size);
        }

        if (ext_head_len) {
            MLOGI("[%s] sub id[%d] data:%s\n", __func__, p_file_seq->subt_id, subt_data->data + int_head_len);
        }
        mtos_sem_take((os_sem_t *)(&(p_file_seq->sub_fifo_mutex)), 0);
        write_sub_fifo_kw(p_file_seq->p_sub_fifo_handle, subt_data->data, cur_sub_packet_size + int_head_len + ext_head_len);
        mtos_sem_give((os_sem_t *)(&(p_file_seq->sub_fifo_mutex)));
        MLOGD("[%p] FILE_PLAYBACK_NEW_SUB_DATA_RECEIVE", p_file_seq->event_cb);
        if (NULL != p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_NEW_SUB_DATA_RECEIVE, 0);
        }
    }
    mtos_free(subt_data->data);
    mtos_free(subt_data);
end:
    if (ds_a->pts < 0.0) {
        ds_a->pts = 0.0;
    }

    if (ds_v->pts < 0.0) {
        ds_v->pts = 0.0;
    }

    if (ds_sub->pts < 0.0) {
        ds_sub->pts = 0.0;
    }

    p_file_seq->orig_apts =  ds_a->pts * 1000;
    p_file_seq->orig_vpts =  ds_v->pts * 1000;
    p_file_seq->orig_spts = ds_sub->pts * 1000;
}

static  void check_ves_water_level(void)
{

    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    u32 videoFreeSpaceSize = 0;

    if (!io_isnetworkstream) {
        return;
    }

    if (p_file_seq->cur_speed != TS_SEQ_NORMAL_PLAY &&
        p_internal->buffering_stat != FILE_SEQ_BUFFERING) {
        return;
    }

    if (p_file_seq->isVideoEsEnd  ||  p_file_seq->isAudioEsEnd) {
        if (p_internal->buffering_stat == FILE_SEQ_BUFFERING) {
            p_internal->buffering_stat = FILE_SEQ_BUFFERING_END;
            if (p_file_seq->m_play_state == FILE_SEQ_PLAY) {
                vdec_resume(p_file_seq->p_vdec_dev);
                aud_resume_vsb(p_file_seq->p_audio_dev);
            }

            p_internal->buffering_stat = FILE_SEQ_BUFFERING_END;
        }

        return;
    }

    if ((p_file_seq->vpts_upload < 1 ||
		p_file_seq->video_disp_h <= 0 ||
		p_file_seq->video_disp_w <= 0 ||
		p_file_seq->is_stable == MT_FALSE) &&
		p_internal->buffering_stat != FILE_SEQ_BUFFERING &&
		p_internal->seek_buffer != 1) {
        return;
    } else {
        int vitual_es_size = (p_internal->vitual_video_es_cur_size) >> 10;
        if (vitual_es_size < (p_internal->min_ves_buf_size)) {
            vdec_get_es_buf_space(p_file_seq->pb_internal, (u32 *)&videoFreeSpaceSize);
        } else {
            videoFreeSpaceSize = vitual_es_size;
        }
    }
}

static MT_BOOL is_handle_video_es(FILE_SEQ_T *p_file_seq)
{
    int ret = 0;
    int atrack_level_ctl_wait = 0;
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
	double vbuf_pts_diff = 0.0;
	double report_pts = 0.0;

    if (p_file_seq->isVideoEsEnd) {
		MLOGD("%s VideoEsEnd\n", __func__);
        return 0;
    }

    demuxer_t *p_demuxer = (demuxer_t *)(p_file_seq->p_demuxer);
    if (p_demuxer) {
        vbuf_pts_diff = 0.0;

        if (p_file_seq->vpts_upload > TIME_BASE &&
            p_file_seq->vpts_upload < INT64_MAX) {
            report_pts = p_file_seq->first_vpts + (double) p_file_seq->vpts_upload / TIME_BASE;
            vbuf_pts_diff = p_file_seq->orig_vpts - report_pts;
        }

        if (!io_isnetworkstream &&
            (p_internal->audio_track_cnt > 1) &&
            ( vbuf_pts_diff  > 3000)) {
            atrack_level_ctl_wait = 1;
        }

        ret = !atrack_level_ctl_wait &&
              (check_is_pts_leap(0) ||
               (p_file_seq->orig_vpts <= p_file_seq->orig_apts && check_is_pts_leap(1) == 0) ||
               (p_file_seq->orig_vpts > p_file_seq->orig_apts && p_file_seq->is_audio_deecoder_error) ||
               (p_file_seq->orig_vpts <= p_file_seq->orig_spts) ||
               (p_file_seq->isAudioEsEnd));

        if (ret == 0 && p_internal->audio_get_pkt_fail) {
            ret = 1;
        }
    }

    int driver_report_pkt_num = -1;
    if (MT_SUCCESS != vdec_get_es_buf_pkt_num(
            p_file_seq->pb_internal, &driver_report_pkt_num)) {
        /* use driver reported pkt count as the real pkt count in es buffer, if fail, larger than 1 will be ok */
        driver_report_pkt_num = 30;
    }
    /* at least 2 frame, decoder start to decode */
    if (!ret && driver_report_pkt_num < 30 && p_internal->audio.pts_hold_counters > 10) {
        ret = 1;
    }

    if (ret && p_internal->vitual_video_es_cur_size < 0) {
        if(vdec_file_get_water_level_vsb(p_internal) == MT_FAILURE) {
            MLOGA("video exceed the water level\n");
            ret = 0;
        }
    }

    /* No es in vdec buffer, num is 30 if fail and ignore first push after flush */
    if (ret && -1 != get_video_underflow_cnt(p_internal) &&
        driver_report_pkt_num < VIDEO_ES_PKT_UNDERFLOW_CNT) {
        MLOGA("driver_report_pkt_num:%d\n", driver_report_pkt_num);
        increase_video_underflow_cnt(p_internal);
    }

    return ret;
}

static int is_aes_segment_behind(FILE_SEQ_T *p_file_seq)
{
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if (MP_NOPTS_VALUE == pbi->audio.segment_start_pts ||
        MP_NOPTS_VALUE == pbi->video.segment_start_pts) {
        return 0;
    }

    int64_t curr_audio_pts = MP_NOPTS_VALUE;
    int64_t curr_video_pts = MP_NOPTS_VALUE;
    demux_stream_t *dsa = (demux_stream_t *) p_file_seq->p_cur_ds_audio;
    demux_stream_t *dsv = (demux_stream_t *) p_file_seq->p_cur_ds_video;

    if (dsa && (MT_TRUE == dsa->pts_valide)) {
        curr_audio_pts = (int64_t) (dsa->pts * TIME_BASE * TIME_BASE);
    }
    if (dsv && (MT_TRUE == dsv->pts_valide)) {
        curr_video_pts = (int64_t) (dsv->pts * TIME_BASE * TIME_BASE);
    }

    if (MP_NOPTS_VALUE == curr_audio_pts ||
        MP_NOPTS_VALUE == curr_video_pts) {
        return 0;
    }

    curr_audio_pts -= pbi->audio.segment_start_pts;
    curr_video_pts -= pbi->video.segment_start_pts;
    if (curr_audio_pts < curr_video_pts) {
        MLOGD("av segment start pts(%lld, %lld), curr(%lld %lld)\n",
            pbi->audio.segment_start_pts, pbi->video.segment_start_pts,
            curr_audio_pts, curr_video_pts);
        return 1;
    }

    return 0;
}

static int is_handle_audio_es(FILE_SEQ_T *p_file_seq)
{
    int ret = 0;
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    if (p_file_seq->isAudioEsEnd) {
        MLOGD("%s AudioEsEnd\n", __func__);
        return 0;
    }
    /* Please do not check aes buffer low, unsupport stream may cause much problem */
    ret = (check_is_pts_leap(1)//fp layer for avsync
            || (p_file_seq->orig_apts <= p_file_seq->orig_vpts && check_is_pts_leap(0) == 0)
            || (p_file_seq->orig_apts < p_file_seq->orig_spts)
            || (p_file_seq->isVideoEsEnd)
            || (is_aes_segment_behind(p_file_seq)));

    if(ret == 0 && p_internal->video_get_pkt_fail) {
        MLOGI("video get pkt failed\n");
        ret = 1;
    }

    if(ret) {
        if(p_internal->vitual_audio_es_cur_size < 0) {
            if(aud_file_get_water_level_vsb(p_file_seq->pb_internal) == MT_FAILURE) {
                MLOGD("audio exceed the water level\n");
                ret = 0;
            }
        }
    }

    demux_stream_t *dsa = (demux_stream_t *) p_file_seq->p_cur_ds_audio;
    if ((MT_TRUE == dsa->pts_valide)) {
        p_internal->audio.pts_hold_counters = 0;
    } else if (ret && MT_FALSE == dsa->pts_valide) {
        p_internal->audio.pts_hold_counters++;
    }

    return ret;
}

static int is_handle_sub_es(FILE_SEQ_T *p_file_seq)
{
    int ret = 0;
    int normal_play = (p_file_seq->cur_speed == TS_SEQ_NORMAL_PLAY ||
                       p_file_seq->cur_speed == TS_SEQ_FAST_PLAY_2X);

    if (!normal_play) {
        p_file_seq->orig_spts = p_file_seq->orig_vpts;
    }
    /* No need check av ds->bytes, what decide whether handle subtitle es is the pts. */
    ret = (normal_play && (
            (p_file_seq->orig_vpts >= p_file_seq->orig_spts) ||
            (p_file_seq->orig_apts >= p_file_seq->orig_spts)));

    return ret;
}

static int check_adjust_aspectratio(void *dev, MT_UNF_AVPLAY_STREAM_INFO_S *stream_info)
{
    MT_S32 Ret;
    mt_handle g_hWin;

    file_seq_get_vo_handle(&g_hWin);
    if (g_hWin == -1) {
        return -1;
    }

    Ret = vdec_get_stream_info(dev, stream_info);
    if (Ret != MT_SUCCESS) {
        return -1;
    }

    MT_UNF_WINDOW_ATTR_S winattr;
    Ret = MT_UNF_VO_GetWindowAttr(g_hWin, &winattr);

    if (Ret != MT_SUCCESS || winattr.stWinAspectAttr.enAspectCvrs != MT_UNF_VO_ASPECT_CVRS_COMBINED) {
        return -1;
    }

    MT_UNF_DISP_ASPECT_RATIO_S stDispAspectRatio = {MT_UNF_DISP_ASPECT_RATIO_AUTO};

    MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY0, &stDispAspectRatio);
    if (stDispAspectRatio.enDispAspectRatio != MT_UNF_DISP_ASPECT_RATIO_AUTO) {
        return -1;
    }

    return 0;
}

static void restore_aspectratio_mode(MT_UNF_DISP_ASPECT_RATIO_E enDispAspectRatio)
{
    MT_UNF_DISP_ASPECT_RATIO_S stDispAspectRatio;
    MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY0, &stDispAspectRatio);
    stDispAspectRatio.enDispAspectRatio = enDispAspectRatio;
    MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY0, &stDispAspectRatio);

    MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY1, &stDispAspectRatio);
    stDispAspectRatio.enDispAspectRatio = enDispAspectRatio;
    MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &stDispAspectRatio);


    MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY0, &stDispAspectRatio);
}

static void adjust_aspectratio_mode(int w)
{
    MT_UNF_DISP_ASPECT_RATIO_E enDispAspectRatio;

    if (w == 4) {
        enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_4TO3;
    } else if (w == 16) {
        enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_16TO9;
    } else if (w == 1) {
        enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_1TO1;
    } else {
        return;
    }
    restore_aspectratio_mode(enDispAspectRatio);
}

void fpi_init_codec_blacklist(PLAYBACK_INTERNAL_T *fpi)
{
    int idx;
    int *black_list = NULL;

    black_list = fpi->audio.codec_blacklist;
    for (idx = 0; idx < MAX_BLACKLIST_CODEC_CNTS; idx++) {
        black_list[idx] = HA_AUDIO_ID_INVALID;
    }

    black_list = fpi->video.codec_blacklist;
    for (idx = 0; idx < MAX_BLACKLIST_CODEC_CNTS; idx++) {
        black_list[idx] = MT_UNF_VCODEC_TYPE_BUTT;
    }
}

int fpi_codec_in_blacklist(int codec,const int invalid_data, int *black_list)
{
    int idx = 0;
    while (idx < MAX_BLACKLIST_CODEC_CNTS && invalid_data != black_list[idx]) {
        if (codec == black_list[idx]) {
            return MT_TRUE;
        }
        idx++;
    }
    return MT_FALSE;
}

int fpi_set_codec_blacklist(PLAYBACK_INTERNAL_T *fpi,unsigned int stream_type, int num, int *list)
{
    int idx;
    int *black_list = NULL;
    int counts = MIN(num, MAX_BLACKLIST_CODEC_CNTS);

    if (STREAM_TYPE_AUD == stream_type) {
        black_list = fpi->audio.codec_blacklist;
    } else if (STREAM_TYPE_VID == stream_type) {
        black_list = fpi->video.codec_blacklist;
    } else {
        MLOGW("Not support stream type:%d\n", stream_type);
        return MT_FAILURE;
    }

    for (idx = 0; idx < counts; idx++) {
        black_list[idx] = list[idx];
    }

    return MT_SUCCESS;
}

int fpi_get_playlist(FILE_SEQ_T *p_file_seq)
{
    demuxer_t *demuxer =
        (demuxer_t *) p_file_seq->p_demuxer;

    int ret = demuxer_get_playlist(demuxer);
    if (MT_TRUE != ret || !(demuxer->num_adaptive_playlist) || !(demuxer->adaptive_playlist)) {
        return MT_FAILURE;
    }

    FILE_PLAYBACK_ADAPTIVE_PLAYLIST_T *fp_pls = &p_file_seq->adaptive_playlist;
    if (demuxer->num_adaptive_playlist > 0 &&
        fp_pls->num != demuxer->num_adaptive_playlist) {
        free33(fp_pls->list);
        fp_pls->num  = demuxer->num_adaptive_playlist;
        fp_pls->list = (FILE_SEQ_ADAPTIVE_PLAYLIST_T *)
            mlzp_malloc(fp_pls->num * sizeof(FILE_SEQ_ADAPTIVE_PLAYLIST_T));
    }
    if (!fp_pls->list) {
        fp_pls->num = 0;
        return MT_FAILURE;
    }

    demuxer_adaptive_playlist_t *demuxer_lists = demuxer->adaptive_playlist;
    for (int num = 0; num < fp_pls->num; num++) {
        FILE_SEQ_ADAPTIVE_PLAYLIST_T *l = &fp_pls->list[num];
        l->index  = demuxer_lists[num].index;
        l->flags  = demuxer_lists[num].flags;
        l->width  = demuxer_lists[num].width;
        l->height = demuxer_lists[num].height;
        l->codec_tag = demuxer_lists[num].codec_tag;
        l->bandwidth = demuxer_lists[num].bandwidth;
        l->framerate.num = demuxer_lists[num].framerate.num;
        l->framerate.den = demuxer_lists[num].framerate.den;
    }
    fp_pls->curr_idx = demuxer->idx_adaptive_playlist;
    return MT_SUCCESS;
}

int fpi_switch_playlist(FILE_SEQ_T *p_file_seq, int playlist_index)
{
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    pbi->playlist_index = playlist_index;

    return MT_SUCCESS;
}

static inline int es_buffer_is_overflow(
    int frame_rate, int pkt_num, int percent)
{
    int packet_high = (frame_rate << 3) + (frame_rate << 2);
    int packet_overflow = (frame_rate << 4);
    /* for low resolution clip */
    if (pkt_num > packet_overflow) {
        return 1;
    }
    /* for hight resolution clip */
    if (pkt_num > packet_high && percent >= 78) {
        return 1;
    }
    return 0;
}

static inline int es_buffer_is_high(
    int frame_rate, int pkt_num, int percent)
{
    int packet_low = (frame_rate << 1);
    int packet_high = (frame_rate << 3);
    /* for low resolution clip */
    if (pkt_num > packet_high) {
        return 1;
    }
    /* for hight resolution clip */
    if (pkt_num > packet_low && percent > 60) {
        return 1;
    }

    return 0;
}

static inline int es_buffer_live_high(
    PLAYBACK_INTERNAL_T *pbi, int frame_rate, int pkt_num, int percent)
{
    int packet_low = (frame_rate << 1);
    int packet_high = (frame_rate << 2);
    /* for low resolution clip */
    if (pkt_num > packet_high) {
        return 1;
    } else if (pkt_num >= frame_rate && !(get_video_underflow_cnt(pbi))) {
        return 1;
    } else if (pkt_num > packet_low && percent > 60) { /* for hight resolution clip */
        return 1;
    }

    return 0;
}

static inline int es_buffer_is_low(
    PLAYBACK_INTERNAL_T *pbi,
    int frame_rate, int pkt_num, int percent)
{
    /* some enctrypted stream need about 2s to start */
    int packet_low = (frame_rate << 1);
    if (pkt_num < packet_low) {
        return 1;
    }
    if (get_video_underflow_cnt(pbi) >= 4) {
        return 1;
    }
    return 0;
}

static inline int es_buffer_is_live_low(
    PLAYBACK_INTERNAL_T *pbi,
    int frame_rate, int pkt_num, int percent)
{
    int packet_low = frame_rate;
    if (pkt_num < packet_low && pbi->video.pkt_cnt_in_buf < 0) {
        return 1;
    }
    if (get_video_underflow_cnt(pbi) >= 4) {
        return 1;
    }
    return 0;
}

static int estimate_playlist_change_mode_live(
    PLAYBACK_INTERNAL_T *pbi, int mode, int frame_rate, int pkt_num, int percent)
{
    int ret = DEMUXER_BITRATE_CHANGE_NONE;
    /* 1. Less packet in es buffer, but bitrate ok, seems wrong */
    if (es_buffer_is_live_low(pbi, frame_rate, pkt_num, percent)) {
        ret = DEMUXER_BITRATE_CHANGE_NOR_DOWN;
        if (DEMUXER_BITRATE_CHANGE_NOR_DOWN != mode) {
            ret = DEMUXER_BITRATE_CHANGE_STEP_DOWN;
        }
    /* 2. live mode, no more packet */
    } else if (es_buffer_live_high(
        pbi, frame_rate, pkt_num, percent)) {
        if (DEMUXER_BITRATE_CHANGE_NOR_UP == mode ||
            DEMUXER_BITRATE_CHANGE_STEP_UP == mode) {
            ret = mode;
        }
    }

    return ret;
}

static int estimate_playlist_change_mode_vod(
    PLAYBACK_INTERNAL_T *pbi, int mode, int frame_rate, int pkt_num, int percent)
{
    int ret = DEMUXER_BITRATE_CHANGE_NONE;

    /* 1. Less packet in es buffer, but bitrate ok, seems wrong */
    if (es_buffer_is_low(pbi, frame_rate, pkt_num, percent)) {
        ret = DEMUXER_BITRATE_CHANGE_NOR_DOWN;
        if (DEMUXER_BITRATE_CHANGE_NOR_DOWN != mode) {
            ret = DEMUXER_BITRATE_CHANGE_STEP_DOWN;
        }
    /* 2. 10s more data, Try change */
    } else if (es_buffer_is_overflow(frame_rate, pkt_num, percent)) {
        if (DEMUXER_BITRATE_CHANGE_NOR_UP == mode) {
            ret = DEMUXER_BITRATE_CHANGE_STEP_UP;
        }
    /* 3. More packet in es buffer, and bitrate large */
    } else if (es_buffer_is_high(frame_rate, pkt_num, percent)) {
        if (DEMUXER_BITRATE_CHANGE_NOR_UP == mode) {
            ret = DEMUXER_BITRATE_CHANGE_STEP_UP;
        }
    }
    return ret;
}

static int estimate_playlist_change_mode(
    PLAYBACK_INTERNAL_T *pbi, int mode, int frame_rate, int pkt_num, int percent)
{
    if (pbi->is_live) {
        return estimate_playlist_change_mode_live(
            pbi, mode, frame_rate, pkt_num, percent);
    }

    return estimate_playlist_change_mode_vod(
        pbi, mode, frame_rate, pkt_num, percent);
}

#define FPI_UNSUPPORT_FPS(fps)  ((fps) < 12 || (fps) > 120)
int fpi_change_playlist(FILE_SEQ_T *p_file_seq, int need_change)
{
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    FILE_SEQ_VIDEO_T *v = &pbi->video;

    int ret = 0;
    int driver_report_pkt_num = -1;
    int frame_rate = v->frame_rate.den == 0 ? 0 : v->frame_rate.num / v->frame_rate.den;
    int mode = need_change ? need_change : DEMUXER_BITRATE_CHANGE_NONE;
    int total_buf_size = 0, used_size = 0, percent = 0;

    /* No need change when we seeking */
    if (IS_SET(p_file_seq->m_user_cmd, CMD_PLAY_AT_TIME) ||
        IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_AUDIO_TRACK) ||
        FPBI_STATE_FLUSHED == pbi->video.state) {
        mode = DEMUXER_BITRATE_CHANGE_NONE;
        goto done;
    }

    if (FPI_UNSUPPORT_FPS(frame_rate)) {
        if (FPI_UNSUPPORT_FPS(p_file_seq->video_fps)) {
            /* No fps can use, use bitrate only */
            goto done;
        }
        frame_rate = p_file_seq->video_fps;
    }
    /* ignore first seg pkt info after flush */
    if (-1 == (get_video_underflow_cnt(pbi))) {
        goto done;
    }

    /* use driver reported pkt count as the real pkt count in es buffer */
    if (MT_SUCCESS != vdec_get_es_buf_pkt_num(
        p_file_seq->pb_internal, &driver_report_pkt_num)) {
        /* driver not send pkt count, ignore */
        goto done;
    }

    if (MT_SUCCESS == vdec_file_get_es_buf_info(
        p_file_seq->pb_internal, &total_buf_size, &used_size)) {
        if (total_buf_size > 0 && used_size > 0)  {
            percent = used_size * 100 / total_buf_size;
        }
    }
    mode = estimate_playlist_change_mode(pbi,
        mode, frame_rate, driver_report_pkt_num, percent);
done:
    if (mode != DEMUXER_BITRATE_CHANGE_NONE) {
        mlzp_mutex_lock(pbi->video.mutex);
        pbi->video.pkt_cnt_in_buf = 0;
        mlzp_mutex_unlock(pbi->video.mutex);
    }
    av_log(NULL, AV_LOG_INFO, "Try change play list, pkt:%d %d mem(%d %d %d) fr:%d num:%d den:%d need:%d mode:%d of:%d\n",
        v->pkt_cnt_in_buf, driver_report_pkt_num, used_size, total_buf_size, percent,
        frame_rate, v->frame_rate.num, v->frame_rate.den, need_change, mode, pbi->video.pkt_cnt_underflow);
    set_video_underflow_cnt(pbi, 0);
    return mode;
}

int fpi_query_idle_time(FILE_SEQ_T *p_file_seq, s64 *idle_time)
{
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    *idle_time = pbi->push_idle_time * 1000; /* ms->us */
    pbi->push_idle_time = 0;
    return 0;
}

int fp_is_support_audio_codec(
    FILE_SEQ_T *p_file_seq, int audio_codec)
{
    (void) p_file_seq;
    int is_support = MT_FALSE;
    switch (audio_codec) {
        case  HA_AUDIO_ID_MP2:
        case  HA_AUDIO_ID_MP3:
        case  HA_AUDIO_ID_DTSHD:
        case  HA_AUDIO_ID_DOLBY_TRUEHD:
        case  HA_AUDIO_ID_DOLBY_PLUS:
        case  HA_AUDIO_ID_DTSPASSTHROUGH:
        case  HA_AUDIO_ID_DOLBY_AC4:
        case  HA_AUDIO_ID_VVID:
        case  HA_AUDIO_ID_AAC:
        case  HA_AUDIO_ID_PCM:
        case  HA_AUDIO_ID_WMA9STD:
        case  HA_AUDIO_ID_FLAC:
        case  HA_AUDIO_ID_VORBIS:
        case  HA_AUDIO_ID_APE:
        case  HA_AUDIO_ID_OPUS:
            is_support = MT_TRUE;
            break;
        default:
            break;
    }
    return is_support;
}

void fill_aud_es_task(void *p_param)
{
#define MAX_AUDIO_FRAME_SIZE (320 * 1024)

    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_param;
    MT_ASSERT(p_file_seq != NULL);

    uint8_t *p_extra_aud_buf       = NULL;
    uint8_t   extra_audio_size     = 0;
    int cur_audio_packet_size      = 0;
    unsigned char *tmp_aud_buf     = NULL;
    demux_stream_t *ds_a           = NULL;
    demux_stream_t *ds_v           = NULL;
    double read_pkt_size           = 0;
    MT_BOOL is_send_eof            = MT_FALSE;

    int pkt_cnt      = 0;
    int pkt_offset   = 0;
    double orig_pts1 = 0;
    double orig_pts2 = 0;
    double orig_pts3 = 0;
    mt_set_pthread_name(__FUNCTION__);
    first_vpts       = -2;
    MLOGD("[%s] start start ...\n", __func__);
    p_file_seq->is_task_alive       = MT_TRUE;
    p_file_seq->needNewVideoData    = MT_TRUE;
    p_file_seq->needNewAudioData    = MT_TRUE;
    p_file_seq->isTrickToNormal     = MT_FALSE;
    p_file_seq->isNormalPlay        = MT_TRUE;
    p_file_seq->isTrickPlay         = MT_FALSE;
    p_file_seq->ref_first_audio_pts = 0;
    p_file_seq->ref_first_video_pts = 0;
    p_file_seq->isAudMute           = MT_FALSE;
    p_file_seq->is_play_to_end      = 0;

    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    p_internal->vitual_audio_es_cur_size   = 0;
    p_internal->is_drm                     = 0;
    p_internal->seek_buffer                = 0;
    p_internal->is_need_extra_audio        = 0;
    p_internal->extio_ctx                  = NULL;

    tmp_aud_buf = malloc33(MAX_AUDIO_FRAME_SIZE);
    if (tmp_aud_buf) {
        memset(tmp_aud_buf, 0, MAX_AUDIO_FRAME_SIZE);
    } else {
        MLOGE("[%s][ERROR] fail to malloc AUDIO tmp buffer!!!\n", __func__);
        return;
    }

    cur_audio_packet_size = 0;
    ds_a = p_file_seq->p_ds_audio;
    ds_v = p_file_seq->p_ds_video;
    p_file_seq->m_play_state = FILE_SEQ_PLAY;

    if (p_file_seq->start_seconds > 0) {
        demux_seek(p_file_seq->p_demuxer, p_file_seq->start_seconds, 0, SEEK_ABSOLUTE);
    }

    MLOGI("Start main audio loop !!!!!!!!!!!\n");
    SetFlag(p_file_seq->internal_event, AUTO_DELIVER_VPTS);
    open_es_dump_file((PLAYBACK_INTERNAL_T *) p_file_seq->pb_internal, STREAM_TYPE_AUD);
    drv_adp_init_avif(p_file_seq->pb_internal, p_file_seq, mtavsf_msg_callback);
    while (1) {
        do_user_cmd();
        /* directly free video packet in audio only mode
         * it seems that we should drop it after demux to avoid memory copy.
         * we call new_demuxer prepare stream, so ds_v not null.
         */
        ds_free_packs(ds_v);
        if (p_file_seq->m_play_state == FILE_SEQ_EXIT) {
            break;
        } else if ((!p_file_seq->is_support_seek_finish) && (p_file_seq->m_play_state == FILE_SEQ_PAUSE)) {
            mtos_task_sleep(30);
            continue;
        }

        if(p_internal->vitual_audio_es_cur_size < 0) {
            if(aud_file_get_water_level_vsb(p_file_seq->pb_internal) == MT_FAILURE) {
                /* Not allow sleep to long once when there is event  */
                for (int sleep_cnt = 0; sleep_cnt < 10; sleep_cnt++) {
                    mtos_task_sleep(20);
                    handle_pending_event();
                }
                continue;
            }
        }

        if ((!p_file_seq->is_support_seek_finish) && (p_file_seq->is_play_to_end == 1)) {
            break;
        }

        if (check_task_finish(p_file_seq)) {
            if (p_file_seq->is_support_seek_finish) {
                MLOGI("func [%s] line [%d] is_send_eof [%d],task finish, break!\n", __func__, __LINE__, is_send_eof);
                if(!is_send_eof && (NULL != p_file_seq->event_cb)) {
                    p_file_seq->event_cb(FILE_PLAYBACK_PLAY_EOF, 0);
                    is_send_eof = MT_TRUE;
                }
            } else {
                break;
            }
        } else if (is_send_eof && p_file_seq->is_support_seek_finish) {
            is_send_eof = MT_FALSE;
        }

        if (p_file_seq->isAudioEsEnd == MT_FALSE) {
            /* get audio frame through ffmpeg demuxer */
            if (p_file_seq->needNewAudioData) {
                if (!ds_a->eof) {
                    if (p_file_seq->isTrickPlay) {
                        /* if we not get video es packet, ds_v->pts always 0 */
                        while ((!ds_a->eof)  && ((ds_v->pts - ds_a->pts) > 0.0)) {
                            ds_get_packet(ds_a, &(p_file_seq->p_a_pkt_start));
                        }
                    }

                    if (p_file_seq->isTrickToNormal) {
                        while ((!ds_a->eof)  && ((ds_v->pts - ds_a->pts) > 0.0)) {
                            ds_get_packet(ds_a, &(p_file_seq->p_a_pkt_start));
                        }
                    }

                    if (!p_file_seq->is_support_seek_finish) {
                        if (ds_a->eof) {
                            MLOGD("Break for audio stream is finished !!!\n");
                            break;
                        }
                    }

                    extra_audio_size = 0;
                    p_extra_aud_buf = NULL;
                    cur_audio_packet_size = fpi_get_ds_audio_packet(p_file_seq,
                        (void *)  ds_a, &(p_file_seq->p_a_pkt_start), &p_extra_aud_buf, (uint8_t *)(&extra_audio_size));

                    if (cur_audio_packet_size <= 0) {
                        MLOGD("[%s][WARNING] audio packet size [%d]!!!!\n", __func__, cur_audio_packet_size);
                        continue;
                    } else {
                        read_pkt_size += cur_audio_packet_size;
                    }
                } else {
                    unsigned char tmp_buf[512];
                    memset(tmp_buf, 0, sizeof(tmp_buf));
                    MLOGI("[%s][ok]  >>>> Audio Stream Demuxer Is Finished !!<<<<\n", __func__);
                    adec_push_audio_es(tmp_buf, sizeof(tmp_buf), 0, 1);
                    p_file_seq->isAudioEsEnd = MT_TRUE;
                    continue;
                }

                if (ds_a->pts < 0.0) {
                    ds_a->pts = 0.0;
                }

                if ((first_vpts) < -1) {
                    first_vpts = ds_a->pts;
                }

                p_file_seq->orig_apts = ds_a->pts * TIME_BASE;
                /* demux lavf keep second pts correct */
                if (ds_a->demuxer->type != DEMUXER_TYPE_LAVF) {
                    pkt_cnt ++;
                    pkt_offset = pkt_cnt % 3;
                    if (pkt_offset == 1) {
                        orig_pts1 = p_file_seq->orig_apts;
                    } else if (pkt_offset == 2) {
                        orig_pts2 = p_file_seq->orig_apts;
                    } else if (pkt_offset == 0) {
                        orig_pts3 = p_file_seq->orig_apts;
                    }

                    if (fabs(orig_pts1) == 0.0 && fabs(orig_pts2) == 0.0 && fabs(orig_pts3) == 0.0 && p_file_seq->audio_bps) {
                        //if seek to head this will be a wrong pts
                        // so we should 3 pts all is zero to make sure
                        p_file_seq->orig_apts = read_pkt_size * 1000 / p_file_seq->audio_bps; //ms
                    }
                }

                p_file_seq->sys_apts = (u64)(TIME_BASE * (p_file_seq->orig_apts)) ;
                if (p_file_seq->orig_apts > p_file_seq->max_audio_pts) {
                    p_file_seq->max_audio_pts = p_file_seq->orig_apts;

                    if (p_file_seq->isTrickToNormal) {
                        p_file_seq->ref_first_audio_pts = p_file_seq->max_audio_pts;
                        p_file_seq->isTrickToNormal = MT_FALSE;
                    }
                }

                /*normal play*/
                if (p_file_seq->isNormalPlay) {
                    p_file_seq->needNewAudioData = MT_FALSE;
                } else {
                    p_file_seq->needNewAudioData = MT_TRUE;

                    if (p_extra_aud_buf && extra_audio_size) {

                        p_extra_aud_buf = NULL;
                        p_file_seq->totalAesNum += (cur_audio_packet_size + extra_audio_size);
                    }

                    p_file_seq->isAudioBufferFull = MT_FALSE;
                    continue;
                }
            }

            /*push aes to dma ring buffer */
            if (p_file_seq->isNormalPlay) {
                if (cur_audio_packet_size > AUD_ASSEMBLE_BUF_LEN) {   //  split big package of audio
                    int i;
                    int n = cur_audio_packet_size/(p_file_seq->audio_bps/10);

                    for(i = 0; i < n; i ++) {
                        if (p_extra_aud_buf && extra_audio_size) {
                            memcpy(tmp_aud_buf, p_extra_aud_buf, extra_audio_size);
                        }
                        memcpy(tmp_aud_buf + extra_audio_size, (void *)p_file_seq->p_a_pkt_start + i*p_file_seq->audio_bps/10, p_file_seq->audio_bps/10);
                        adec_push_audio_es(tmp_aud_buf, p_file_seq->audio_bps/10 + extra_audio_size, p_file_seq->sys_apts + i*100000, 0);
                    }
                    p_file_seq->needNewAudioData = MT_TRUE;
                    p_file_seq->totalAesNum += cur_audio_packet_size;
                    p_file_seq->isAudioBufferFull = MT_FALSE;
                    p_extra_aud_buf = NULL;
                }else  {
                    if (p_extra_aud_buf && extra_audio_size) {
                        memcpy(tmp_aud_buf, p_extra_aud_buf, extra_audio_size);

                        p_extra_aud_buf = NULL;
                    }

                    memcpy(tmp_aud_buf + extra_audio_size, (void *)p_file_seq->p_a_pkt_start, cur_audio_packet_size);
                    adec_push_audio_es(tmp_aud_buf, cur_audio_packet_size + extra_audio_size, p_file_seq->sys_apts, 0);

                    p_file_seq->needNewAudioData = MT_TRUE;
                    p_file_seq->totalAesNum += cur_audio_packet_size;
                    p_file_seq->isAudioBufferFull = MT_FALSE;
                }
            }
            p_internal->vitual_audio_es_cur_size -= (cur_audio_packet_size + extra_audio_size);
        }

        handle_pending_event();
        MT_USLEEP(1000);
    }

    MLOGI("[%s]  @@@ END audio MAIN LOOP @@@@, is_play_to_end [%d]\n", __func__, p_file_seq->is_play_to_end);

    if (tmp_aud_buf) {
        free33(tmp_aud_buf);
        tmp_aud_buf = NULL;
    }

    close_es_dump_file((PLAYBACK_INTERNAL_T *) p_file_seq->pb_internal, STREAM_TYPE_AUD);
    handle_exit_fill_es_task(p_file_seq);
    MLOGI("[%s] end end ...\n", __func__);
}

static int start_prerolling_es(FILE_SEQ_T *p_file_seq)
{
    if (!p_file_seq->is_ts) {
        return 0;
    }

    int preload_cnt  = 0;
    int es_task_init = 0;
    demux_stream_t *dsv =
        (demux_stream_t *) p_file_seq->p_cur_ds_video;
    sh_video_t     *shv = (sh_video_t *)dsv->sh;
    int frame_rate = (shv->fps.den > 0 && shv->fps.num > 0) ?
        ((shv->fps.num + shv->fps.den - 1) / shv->fps.den) : 50;
    int max_preload_cnt = frame_rate;

    vdec_pause(p_file_seq->p_vdec_dev);
    if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
        aud_pause_vsb(p_file_seq->p_audio_dev);
    }

    MLOGI("Star Prerolling...\n");
    while (preload_cnt++ < max_preload_cnt) {
        int handle_video = is_handle_video_es(p_file_seq);
        if(handle_video) {
            handle_video_es(p_file_seq);
        }

        if (es_task_init == 0) {
            p_file_seq->first_vpts = p_file_seq->orig_vpts;
            if (p_file_seq->first_vpts > 1) {
                es_task_init = 1;
            }
        }
        if (es_task_init == 1 && p_file_seq->video_start_pts < 0) {
            p_file_seq->video_start_pts = p_file_seq->first_vpts;
            MLOGI("video_start_pts = %f\n",p_file_seq->video_start_pts);
        }

        int handle_audio = is_handle_audio_es(p_file_seq);
        if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID &&
          (p_file_seq->orig_apts < p_file_seq->orig_vpts) && handle_audio) {
            handle_audio_es(p_file_seq);
        }

        if (!handle_video && !handle_audio) {
            break;
        }
    }

    vdec_resume(p_file_seq->p_vdec_dev);
    if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
        aud_resume_vsb(p_file_seq->p_audio_dev);
    }

    MLOGI("Stop Prerolling,rate:%d cnt:%d max:%d\n", frame_rate, preload_cnt, max_preload_cnt);
    return es_task_init;
}

void fill_es_task(void * p_param)
{
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)p_param;
    prctl(PR_SET_NAME, "fill_es_task");
    int ret                           = 0;
    int es_task_init                  = 0;
    int preload_cnt                   = 0;
    MT_BOOL is_send_eof               = MT_FALSE;

    first_vpts                        = -2;
    int start_tick                    = 0 ;

    MLOGI("[%s] start start ...\n", __func__);
    p_file_seq->needNewVideoData    = MT_TRUE;
    p_file_seq->needNewAudioData    = MT_TRUE;
    p_file_seq->isTrickToNormal     = MT_FALSE;
    p_file_seq->isNormalPlay        = MT_TRUE;
    p_file_seq->isTrickPlay         = MT_FALSE;
    p_file_seq->ref_first_audio_pts = 0;
    p_file_seq->ref_first_video_pts = 0;
    p_file_seq->isAudMute           = MT_FALSE;
    p_file_seq->first_vpts          = (double) MP_NOPTS_VALUE;
    p_file_seq->orig_vpts           = 0;
    p_file_seq->orig_apts           = 0;
    p_file_seq->orig_spts           = 0;
    p_file_seq->m_tmp_ves_buf_pos   = 0;
    p_file_seq->is_play_to_end      = 0;

    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    p_internal->vitual_video_es_cur_size   = 0;
    p_internal->vitual_audio_es_cur_size   = 0;
    p_internal->audio.codec_extradata      = NULL;
    p_internal->video.codec_extradata      = NULL;
    p_internal->audio.codec_extradata_size = 0;
    p_internal->video.codec_extradata_size = 0;
    p_internal->audio.is_secure            = MT_FALSE;
    p_internal->video.is_secure            = MT_FALSE;
    p_internal->audio.ff_av_pkt            = NULL;
    p_internal->video.ff_av_pkt            = NULL;
    p_internal->audio.segment_start_pts    = MP_NOPTS_VALUE;
    p_internal->video.segment_start_pts    = MP_NOPTS_VALUE;
    p_internal->audio.pts_hold_counters    = 0;
    p_internal->video.pkt_cnt_in_buf       = 0;
    p_internal->video.pkt_cnt_underflow    = 0;
    p_internal->is_drm                     = 0;
    p_internal->seek_buffer                = 0;
    p_internal->is_need_extra_audio        = 0;
    p_internal->work_status                = FILE_SEQ_STATUS_WORK;
    p_internal->net_buffer.is_enbale       = MT_FALSE;
    p_internal->video_decoder_over         = MT_FALSE;
    p_internal->extio_ctx                  = NULL;
    p_internal->playlist_index             = -1;

#ifdef MT_DRM_SUPPORT
    int index1 = -1;
    int index2 = -1;
    mp_ffmpeg422_get_drm_index(&index1, &index2);
    if((index1 && (index1 != -1)) || (index2 && (index2 != -1))) {
        p_internal->is_drm = 1;
    }
#endif

    //add by doreen for only video stream, 16-06-06
    if (((demux_stream_t *)(p_file_seq->p_ds_audio))->sh == NULL) {
        MLOGI("[%s] only video mode!! !! \n", __func__);
        p_file_seq->isAudioEsEnd = 1;
    }

    p_file_seq->ves_seg_buf = mtos_malloc(VES_SEG_LEN);

    if (p_file_seq->ves_seg_buf) {
        memset(p_file_seq->ves_seg_buf, 0, VES_SEG_LEN);
    } else {
        MLOGD("[%s][ERROR] fail to malloc audie tmp buffer!!!\n", __func__);
        return;
    }

    p_file_seq->vpts_upload = 0;

    if (p_file_seq->total_path <= 1) {
        p_file_seq->is_task_alive = MT_TRUE;
    }

    file_seq_check_trickplay(p_file_seq);

    ret = file_seq_av_decoder_init(p_file_seq);  //@@@@@@@@@@@
    MLOGD("%s %d  av_decoder_init ret = %d\n", __func__, __LINE__, ret);
    MLOGI("p_file_seq->start_seconds :%d\n",p_file_seq->start_seconds);
    if (p_file_seq->start_seconds > 0) {
        if (p_file_seq->total_path > 1) {
            MLOGI("%s %d: can not support muti-path!!!!!!!!!!!!!!!!!!\n", __func__, __LINE__);
        } else {
            handle_file_vhdr(p_file_seq);
            if (es_task_init == 0) {
				if(p_file_seq->first_vpts > 1) {

                	p_file_seq->first_vpts = p_file_seq->orig_vpts;
                	es_task_init = 1;
				} else if (p_file_seq->orig_apts > 1) {

                    p_file_seq->first_vpts = p_file_seq->orig_apts;
					es_task_init = 1;
                }
            }

			if (es_task_init == 1 && p_file_seq->video_start_pts <0) {
				p_file_seq->video_start_pts = p_file_seq->first_vpts;
				MLOGI("video_start_pts = %f\n",p_file_seq->video_start_pts);
			}
            demux_seek(p_file_seq->p_demuxer, p_file_seq->start_seconds, 0, SEEK_ABSOLUTE);
        }
    }

    if (p_file_seq->total_path > 1) {
        p_file_seq->is_task_alive = MT_TRUE;
    }

    if (MT_UNF_VCODEC_TYPE_VP8 == p_file_seq->m_video_codec_type ||
        MT_UNF_VCODEC_TYPE_VP9 == p_file_seq->m_video_codec_type) {
        p_internal->video.need_insert_header = 1;
    }

    drv_adp_init_avif(p_internal, p_file_seq, mtavsf_msg_callback);
    open_es_dump_file(p_internal, STREAM_TYPE_AUD);
    open_es_dump_file(p_internal, STREAM_TYPE_VID);
    open_es_dump_file(p_internal, STREAM_TYPE_SUB);

    es_task_init = start_prerolling_es(p_file_seq);
    MLOGI("[%s] @@@ START MAIN LOOP @@@ !![%d]\n", __func__, preload_cnt);

    if (p_file_seq->pb_internal && p_internal->buffering_stat ==  FILE_SEQ_BUFFERING_END
        && io_isnetworkstream) {

        p_file_seq->m_play_state_backup = p_file_seq->m_play_state;
        p_internal->buffering_stat = FILE_SEQ_BUFFERING;

        MLOGI("[%s] >>>>>>>>>START   BUFFERING ................<<<<<<<<<<<\n", __func__);
    }

    MLOGI(" fileplay time node 2 [%s %s_%d]+\n", __FILE__, __FUNCTION__, __LINE__);

    if(((demux_stream_t *)(p_file_seq->p_ds_video))->sh) {
        p_internal->net_buffer.fp_start_tick = mtos_ticks_get();
    }

    start_tick = mtos_ticks_get();
    int fp_cnt = 0;
    int audio_decode_error_count = 0;
    audio_decode_error_count = 0;
    MLOGD("++++++ [%s] [%s] [%d] start_tick = %d\n", __FILE__, __func__, __LINE__, start_tick);
    MT_UNF_AVPLAY_STREAM_INFO_S stream_info;
    memset(&stream_info, 0, sizeof(MT_UNF_AVPLAY_STREAM_INFO_S));
    int adjust_flag = -1;

#ifdef MT_DRM_SUPPORT
    {
        int index1 = -1;
        int index2 = -1;
        mp_ffmpeg422_get_drm_index(&index1, &index2);
        if(index1 != -1)
            p_file_seq->drm_eDrmIndex = index1;
        else if(index2 != -1)
            p_file_seq->drm_eDrmIndex = index2;
    }
#endif

    while (1) {
        fp_cnt++;
        if (fp_cnt == 50 && adjust_flag == -1) {
            adjust_flag = check_adjust_aspectratio(p_file_seq->p_vdec_dev, &stream_info);
            if (adjust_flag == 0) {
                adjust_aspectratio_mode(stream_info.stVidStreamInfo.u32AspectWidth);
            }
        }

        if (file_seq_get_reset_state() > 0) {
            file_seq_switch_reset();
        }

        check_ves_water_level();
        if (p_file_seq->m_play_state == FILE_SEQ_EXIT
            && (p_file_seq->ott_playmode != OTT_LIVE_MODE
                || p_file_seq->force_stopped == 1)) {
            MLOGI("[%s]----p_file_seq->m_play_state == FILE_SEQ_EXIT,break!!!!!!", __func__);

            if (p_file_seq->total_path > 1) {
                p_file_seq->m_preload_state = FILE_SEQ_EXIT;
            }

            break;
        }

        if ((!p_file_seq->is_support_seek_finish) && (p_file_seq->m_play_state == FILE_SEQ_PAUSE)) {
            mtos_task_sleep(30);
            continue;
        }

        if ((!p_file_seq->is_support_seek_finish) && (p_file_seq->is_play_to_end == 1)) {
            MLOGI("func [%s] line [%d] is_play_to_end, break!\n", __func__, __LINE__);\
            break;
        }

        /* identify whether file sequencer can exit */
        if (check_task_finish(p_file_seq)) {
            if (p_file_seq->is_support_seek_finish) {
                MLOGI("func [%s] line [%d] is_send_eof [%d],task finish, break!\n", __func__, __LINE__, is_send_eof);
                if(!is_send_eof && (NULL != p_file_seq->event_cb)) {
                    p_file_seq->event_cb(FILE_PLAYBACK_PLAY_EOF, 0);
                    is_send_eof = MT_TRUE;
                }
            }
            else {
                break;
            }
        } else if(is_send_eof && p_file_seq->is_support_seek_finish) {
            is_send_eof = MT_FALSE;
        }

        /* fetch ves and try to push them to ves buffer */
        int is_video = is_handle_video_es(p_file_seq);
        int is_audio = is_handle_audio_es(p_file_seq);
        int is_sub   = is_handle_sub_es(p_file_seq);

        if (is_video) {
            handle_video_es(p_file_seq);
        }

        /* fetch aes and try to push them to aes buffer */
        if (is_audio) {
            handle_audio_es(p_file_seq);
            //check audio decode
            if(mtos_ticks_get() - start_tick > 10* 10 && (p_file_seq->m_play_state == FILE_SEQ_PLAY)) {
                drv_pts_info_t vstate;
                vdec_get_pts(p_file_seq->p_vdec_dev, &vstate);
                if(vstate.pts > 0 && vstate.apts == 0) {
                    audio_decode_error_count++;
                    if(audio_decode_error_count > 30) {
                        p_file_seq->is_audio_deecoder_error = MT_TRUE;
                    } else {
                        p_file_seq->is_audio_deecoder_error = MT_FALSE;
                    }
                } else {
                    audio_decode_error_count = 0;
                    p_file_seq->is_audio_deecoder_error = MT_FALSE;
                }
                MLOGD("[%s] audio_decode_error_count:%d \n", __func__, audio_decode_error_count);
            }
        }

        /* fetch subtile data and try to push them to middleware */
        if (((demuxer_t *)(p_file_seq->p_cur_demuxer))->subt_info.cnt > 0 && is_sub) {
            if (file_seq_handle_subtitle_packet(p_file_seq) &&
                (!IS_SET(p_file_seq->m_user_cmd, CMD_PLAY_AT_TIME))) {
                handle_subtitle_data(p_file_seq);
            }
        }

        demuxer_t * p_demuxer = (demuxer_t *)(p_file_seq->p_demuxer);
        if (is_audio == 0 && is_video == 0) {
            mlzp_msleep(10);
            p_internal->push_idle_time += 10;
        }else if(p_internal->chip_type < CHIPTYPE_SYMPHONY4 && p_demuxer->type == DEMUXER_TYPE_MPEG_TS
                 && p_file_seq->cur_speed >= TS_SEQ_FAST_PLAY_2X && p_file_seq->video_bps > 0x100000) { /*SYMPHONY2 CPU  is slow  fix bug15863*/

            int multiple = 1;

            if(p_file_seq->video_bps) {
                multiple = p_file_seq->video_bps/(1*1024*1024) + 1;
            }

            if((p_file_seq->cur_speed >= TS_SEQ_REV_FAST_PLAY_4X
               && p_file_seq->cur_speed <= TS_SEQ_REV_FAST_PLAY_8X) ||
               (p_file_seq->cur_speed >= TS_SEQ_FAST_PLAY_4X
               && p_file_seq->cur_speed <= TS_SEQ_FAST_PLAY_8X)) {
                mtos_task_sleep(30*multiple);
            } else if((p_file_seq->cur_speed >= TS_SEQ_REV_FAST_PLAY_16X
               && p_file_seq->cur_speed <= TS_SEQ_REV_FAST_PLAY_32X) ||
               (p_file_seq->cur_speed >= TS_SEQ_FAST_PLAY_16X
               && p_file_seq->cur_speed <= TS_SEQ_FAST_PLAY_32X)) {
                mtos_task_sleep(40*multiple);
            } else if(is_video == 0) {
                mtos_task_sleep(10);
            }
        }

        SetFlag(p_file_seq->internal_event, AUTO_DELIVER_VPTS);
        handle_pending_event();

        if (es_task_init == 0) {
            if (p_file_seq->start_seconds > 0) {
                p_file_seq->first_vpts =
                    p_file_seq->orig_vpts - (double) p_file_seq->start_seconds * 1000;
                if (p_file_seq->total_path > 1) {
                    p_file_seq->first_vpts = 2;
                }
            } else {
                p_file_seq->first_vpts = p_file_seq->orig_vpts;
            }
            if (p_file_seq->first_vpts != (double) MP_NOPTS_VALUE) {
                es_task_init = 1;
            }
        }

        /* excute user's command */
        do_user_cmd();//for bug 128450, after get p_file_seq->first_vpts, do seek user cmd.
    }

    MLOGI("[%s]  @@@ END MAIN LOOP @@@@, is_play_to_end [%d]\n", __func__, p_file_seq->is_play_to_end);

    int end_tick;
    end_tick = mtos_ticks_get();
    MLOGD("++++++ [%s] [%s] [%d] end_tick = %d, time = %d\n", __FILE__, __func__, __LINE__, end_tick, end_tick - start_tick);

    if (adjust_flag == 0) {
        restore_aspectratio_mode(MT_UNF_DISP_ASPECT_RATIO_AUTO);
    }

    if (p_file_seq->ves_seg_buf) {
        mtos_free(p_file_seq->ves_seg_buf);
        p_file_seq->ves_seg_buf = NULL;
    }
    /* change to normal play */
    if (p_file_seq->cur_speed != TS_SEQ_NORMAL_PLAY) {
        p_file_seq->isTrickPlay         = MT_FALSE;
        p_file_seq->isNormalPlay        = MT_TRUE;
        p_file_seq->isTrickToNormal     = MT_TRUE;
        p_file_seq->totalVesNum         = 0;
        p_file_seq->totalAesNum         = 0;
        p_file_seq->ref_first_video_pts = p_file_seq->max_video_pts;
        if (p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_TRICKMODE_LEAVE, 0);
        }
        vdec_set_trick_mode(p_file_seq->p_vdec_dev, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 1);
        p_file_seq->cur_speed           = TS_SEQ_NORMAL_PLAY;
        p_file_seq->last_speed          = TS_SEQ_NORMAL_PLAY;
        p_file_seq->tmp_speed           = 0;
    }

    p_file_seq->is_play_to_end = 1;
    if (p_file_seq->pb_internal &&
        p_internal->buffering_stat ==  FILE_SEQ_BUFFERING) {
        p_internal->buffering_stat = FILE_SEQ_BUFFERING_END;
    }
    p_internal->audio.codec_extradata = NULL;
    p_internal->video.codec_extradata = NULL;
    close_es_dump_file(p_internal, STREAM_TYPE_AUD);
    close_es_dump_file(p_internal, STREAM_TYPE_VID);
    close_es_dump_file(p_internal, STREAM_TYPE_SUB);
    handle_exit_fill_es_task(p_file_seq);
    MLOGI("[%s] end end ...\n", __func__);
}

static void check_video_underflow(FILE_SEQ_T *p_file_seq, mt_s64 *start_time)
{
    /* 250 ms */
    const static mt_s64 check_interval = SECOND_TO_MICROSECOND_BASE >> 2;
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);

    mt_s64 current = mclock_get_utime();
    if (current - *start_time >= check_interval) {
        *start_time = current;

        int driver_report_pkt_num = -1;
        if (MT_SUCCESS != vdec_get_es_buf_pkt_num(pbi, &driver_report_pkt_num)) {
            return;
        }

        /* No es in vdec buffer, num is 30 if fail and ignore first push after flush */
        if (-1 != get_video_underflow_cnt(pbi) &&
            driver_report_pkt_num <= VIDEO_ES_PKT_UNDERFLOW_CNT) {
            increase_video_underflow_cnt(pbi);
        }
    }
}

void check_buffering_thread(void *p_param)
{
    prctl(PR_SET_NAME, "check_buffering_task");
    FILE_SEQ_T *p_file_seq = p_param;
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    sh_video_t *sh_video = ((demux_stream_t *)(p_file_seq->p_ds_video))->sh;
    sh_audio_t *sh_audio = ((demux_stream_t *)(p_file_seq->p_ds_audio))->sh;

    mt_s64 check_video_underflow_start_time = mclock_get_utime();
    p_file_seq->is_check_buffering_task_alive = MT_TRUE;
    MLOGI("[%s]start[%d]\n", __func__, __LINE__);
    mt_set_pthread_name(__FUNCTION__);
    while (p_file_seq->m_play_state != FILE_SEQ_STOP && p_file_seq->m_play_state != FILE_SEQ_EXIT) {
        check_video_underflow(p_file_seq, &check_video_underflow_start_time);
        if (p_file_seq->m_play_state == FILE_SEQ_PAUSE) {
            mlzp_msleep(30);
            continue;
        }

        mlzp_msleep(100);
        if (!p_file_seq->is_support_seek_finish) {
            if (p_file_seq->is_play_to_end == 1) {
                break;
            }
        }

        if(p_file_seq->cur_speed != TS_SEQ_NORMAL_PLAY) {
            continue;
        }

        if ((sh_video && p_file_seq->isVideoEsEnd) ||
            (sh_audio && p_file_seq->isAudioEsEnd)) {
            if (p_internal->net_buffer.is_buffering == MT_TRUE) {
                p_internal->net_buffer.is_buffering = MT_FALSE;
                p_internal->net_buffer.fp_start_tick = mtos_ticks_get();
                p_internal->net_buffer.is_enbale = MT_FALSE;

                p_file_seq->event_cb(FILE_PLAYBACK_SEQ_FINISH_BUFFERING, 0);

                if (p_file_seq->m_play_state == FILE_SEQ_PLAY) {
                    vdec_resume(p_file_seq->p_vdec_dev);
                    aud_resume_vsb(p_file_seq->p_audio_dev);
                }
            }

            continue;
        }

        if ((p_file_seq->vpts_upload < 1
             || p_file_seq->video_disp_h <= 0
             || p_file_seq->video_disp_w <= 0
             || p_file_seq->is_stable == MT_FALSE)
            && p_internal->seek_buffer != 1) {
            continue;
        }

        mtos_sem_take((os_sem_t *)(&(p_internal->seek_mutex)), 0);
        if(p_internal->net_buffer.is_open == MT_TRUE) {
            mt_u32 start_wl, end_wl;
            int ignored_tick = 0;
            if((p_file_seq->video_disp_w * p_file_seq->video_disp_h) <= (720 * 576)) {
                p_internal->net_buffer.resolution_type = 0;
                ignored_tick = 600;    // 6 seconds
            } else if ((p_file_seq->video_disp_w * p_file_seq->video_disp_h) >= (1280 * 720)) {
                p_internal->net_buffer.resolution_type = 2;     // 1080P
                ignored_tick = 800;    // 8 seconds
            } else {
                p_internal->net_buffer.resolution_type = 1;     // 720P
                ignored_tick = 1000;    // 10 seconds
            }

            if(p_internal->net_buffer.is_enbale == MT_FALSE) {
                if((mtos_ticks_get() - p_internal->net_buffer.fp_start_tick) < ignored_tick) {  // calculating playback buffer after starting playback 2(3/4) seconds
                    mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
                    continue;
                }
                p_internal->net_buffer.is_enbale = MT_TRUE;
                p_internal->net_buffer.is_buffering = MT_FALSE;
            }

            mt_u32 all_size, used_size;
            if(vdec_file_get_es_buf_info(p_file_seq->pb_internal, &all_size, &used_size) != MT_SUCCESS) {
                mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
                continue;
            }

            if(p_internal->net_buffer.resolution_type == 0) {
                start_wl = 50*used_size; // 2% <
                end_wl = 5*used_size;    // > 20%
            } else if (p_internal->net_buffer.resolution_type == 2) { // 1080P
                start_wl = 10*used_size; // 10% <
                end_wl = 5*used_size/2;    // > 40%
            } else {                                                    // 720P
                start_wl = 20*used_size; // 5% <
                end_wl = 3*used_size/10;    // > 30%
            }

            if(p_internal->net_buffer.is_buffering == MT_FALSE) {
                if(start_wl < all_size) {
                    p_internal->net_buffer.is_buffering = MT_TRUE;
                    p_internal->net_buffer.start_used =
                    p_internal->net_buffer.last_used = used_size;
                    p_internal->net_buffer.start_tick = mtos_ticks_get();

                    vdec_pause(p_file_seq->p_vdec_dev);
                    aud_pause_vsb(p_file_seq->p_audio_dev);
                    p_file_seq->event_cb(FILE_PLAYBACK_SEQ_START_BUFFERING, 0);
                    mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
                    continue;
                }
            } else {
                if(end_wl > all_size || aud_file_get_water_level_vsb(p_file_seq->pb_internal) == MT_FAILURE) {
                    vdec_resume(p_file_seq->p_vdec_dev);
                    aud_resume_vsb(p_file_seq->p_audio_dev);
                    p_internal->net_buffer.is_buffering = MT_FALSE;
                    p_internal->net_buffer.fp_start_tick = mtos_ticks_get();
                    p_internal->net_buffer.is_enbale = MT_FALSE;
                    p_file_seq->event_cb(FILE_PLAYBACK_SEQ_FINISH_BUFFERING, 0);
                } else {
                    update_bps(all_size, used_size);
                }
                mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
                continue;
            }
        }
        mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
    }

    MLOGI("[%s]end[%d]\n", __func__, __LINE__);

    MLOGI("%s %d task exit,state[%d]\n", __func__, __LINE__, p_file_seq->m_play_state);
    p_file_seq->is_check_buffering_task_alive = MT_FALSE;

    mtos_task_exit();
}
