/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "ts_sequence.h"
#include "libmpdemux/stheader.h"

#define MODULE_TAG "FP EVT"
#include "mutil.h"
#include "mlog.h"
#include "file_playback_sequence.h"
#include "file_seq_internal.h"
#include "file_seq_misc.h"
#include "drv_adp.h"
#include "mtos_misc.h"
#include "libavutil/log.h"

static int print_wallclock_vpts(FILE_SEQ_T *p_file_seq, u64 offset)
{
    static u64        g_run_m_cnt = 0;
    sys_mem_debug_t curr_mem_info = {0};

    if (g_run_m_cnt % 3 == 0) {
        show_sys_memory_info(&(curr_mem_info), &(curr_mem_info), MT_FALSE);
    }
    g_run_m_cnt++;

    return 0;
}


void notify_video_pts(s64 vpts) {
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    s64 target_send_vpts = 0 ;

    p_file_seq->is_stable = 1;

    MLOGD("speed (%d %d) vpts %lldms, upload %llums, seconds %dms first %dms org:%lfms\n",
          p_file_seq->last_speed, p_file_seq->cur_speed, vpts / TIME_BASE,  p_file_seq->vpts_upload / TIME_BASE,
          (int)(p_file_seq->seek_seconds * 1000), (int)(p_file_seq->first_vpts), p_file_seq->orig_vpts);
    if (p_file_seq->vpts_upload == 0) {
        p_file_seq->vpts_upload = 1;
        p_file_seq->event.pts_notify_time = mtos_ticks_get() * 10;
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_GET_VPTS, 0);
        return;
    }

    if (ADP_FW_INVALID_PTS == vpts) {
        return;
    }

    if (vpts >= p_file_seq->first_vpts * TIME_BASE) {
        vpts -= (s64)(p_file_seq->first_vpts * TIME_BASE);
    } else if(p_file_seq->first_vpts - (double)vpts/TIME_BASE > 500) {
    	//if first video pts far greater than the the video pts from video driver
    	MLOGD("do nothing\n");
	} else {
        p_file_seq->first_vpts = (double) vpts / TIME_BASE;
        vpts = 0;
    }

    /* tirck play, fast back */
    if (fp_is_fast_backward(p_file_seq->cur_speed)) {
        target_send_vpts = (s64) p_file_seq->vpts_upload - DELIVER_VPTS_INTERVAL;
        if (vpts <= target_send_vpts) {
            p_file_seq->vpts_upload = vpts;
            if (NULL != p_file_seq->event_cb) {
                if (vpts <= 0) {
                    p_file_seq->event_cb(FILE_PLAYBACK_SEQ_GET_VPTS, 0);
                } else {
                    p_file_seq->event_cb(FILE_PLAYBACK_SEQ_GET_VPTS, p_file_seq->vpts_upload / TIME_BASE);
                }
            }
        }
    } else {
        /* normal play or trick paly(fast forward) */
        if (vpts < p_file_seq->vpts_upload && p_file_seq->seek_seconds < 0) {
            p_file_seq->vpts_upload = MAX(0, (vpts - DELIVER_VPTS_INTERVAL - 1));
            p_file_seq->seek_seconds = -1;
        }

        if (NULL != p_file_seq->event_cb) {
            print_wallclock_vpts(p_file_seq, 0);
            s64 duration   = (s64) (p_file_seq->duration * SECOND_TO_MICROSECOND_BASE);
            u32 notify_pts = (u32) (vpts / TIME_BASE);
            if (p_file_seq->duration && vpts > duration) {
                /* Duration error case or seek pts gap, allow pts double DELIVER_VPTS_INTERVAL << 1 gap */
                s64 allow_gap = DELIVER_VPTS_INTERVAL << 1;
                if (vpts > p_file_seq->vpts_upload && vpts <= p_file_seq->vpts_upload + allow_gap) {
                    p_file_seq->duration += ((double)(vpts - duration) / SECOND_TO_MICROSECOND_BASE);
                } else {
                    notify_pts = (u32)(p_file_seq->duration * TIME_BASE);
                }
            }
            p_file_seq->event_cb(FILE_PLAYBACK_SEQ_GET_VPTS, notify_pts);
        }
        p_file_seq->vpts_upload = vpts;
    }
}

static void notify_audio_pts(FILE_SEQ_T *p_file_seq)
{
    int need_send_pts = MT_FALSE;
    int cur_aud_freespace = 0;
    float aes_buf_consume_ms = 0.0;
    u64 cur_time_ms = mtos_ticks_get() * 10;

    if (p_file_seq->apts_upload <= 0) {
        p_file_seq->apts_upload = TIME_BASE;
        p_file_seq->event.pts_notify_time = 0;
        need_send_pts = MT_TRUE;
    }

    if (p_file_seq->audio_bps) {

        aud_file_getleftesbuffer_vsb(p_file_seq->pb_internal, (u32 *)&cur_aud_freespace);
        aes_buf_consume_ms =
            (((float)(p_file_seq->dec_cap.max_aes_num - cur_aud_freespace) * 1.0) /
            (p_file_seq->audio_bps) * 1.0) * 1000.0;
        if (p_file_seq->orig_apts - aes_buf_consume_ms > (float)(p_file_seq->apts_upload) + 500.0) {
            p_file_seq->apts_upload = (u64)(p_file_seq->orig_apts - aes_buf_consume_ms) * TIME_BASE;
            if (aes_buf_consume_ms > 0) {
                need_send_pts = MT_TRUE;
            }
        }
    } else {
        drv_pts_info_t pts_info = {0};

        u64 target_send_pts = p_file_seq->apts_upload + DELIVER_VPTS_INTERVAL;
        (void) aud_get_pts(p_file_seq->p_vdec_dev, &pts_info);
        if (pts_info.pts > target_send_pts) {
            need_send_pts = MT_TRUE;
            p_file_seq->apts_upload = pts_info.pts;
        }
    }

    MLOGD("orgin %dms upload %llums br %d free %d buf has %f ms need %d cb %p\n", (int) p_file_seq->orig_apts,
        p_file_seq->apts_upload / TIME_BASE, p_file_seq->audio_bps,
        cur_aud_freespace, aes_buf_consume_ms, need_send_pts, p_file_seq->event_cb);

    if (MT_FALSE == need_send_pts &&
        p_file_seq->m_play_state != FILE_SEQ_PAUSE) {
        if( (cur_time_ms - p_file_seq->event.pts_notify_time) > 1500) { // > 1500 ,this is pause
            p_file_seq->apts_upload = p_file_seq->apts_upload + DELIVER_VPTS_INTERVAL;
            need_send_pts = MT_TRUE;
        } else {
            u64 time_elapsed_ms     =
                cur_time_ms - p_file_seq->event.pts_notify_time;
            u64 expected_upload_pts =
                p_file_seq->apts_upload + time_elapsed_ms * TIME_BASE;
            u64 last_upload_seconds     = p_file_seq->apts_upload / TIME_BASE / TIME_BASE;
            u64 expected_upload_seconds = expected_upload_pts / TIME_BASE / TIME_BASE;
            /* report at 200ms interval or seconds increased */
            if (time_elapsed_ms >= 100 ||
                last_upload_seconds != expected_upload_seconds) {
                p_file_seq->apts_upload = expected_upload_pts;
                need_send_pts = MT_TRUE;
                MLOGD("time_elapsed_ms %lld ms last:%lld expected:%lld\n",
                    time_elapsed_ms, last_upload_seconds, expected_upload_seconds);
            }
        }
    }

    if (MT_TRUE == need_send_pts && NULL != p_file_seq->event_cb) {
        MLOGD("send apts %lld ms\n", p_file_seq->apts_upload / TIME_BASE);
        p_file_seq->event_cb(FILE_PLAYBACK_SEQ_GET_VPTS, p_file_seq->apts_upload / TIME_BASE);
        p_file_seq->event.pts_notify_time = cur_time_ms;
    }
}

static void notify_pts(FILE_SEQ_T *p_file_seq)
{
    if (MT_FALSE == p_file_seq->only_audio_mode) {
        return;
    }
    notify_audio_pts(p_file_seq);
}

extern void ds_reset_trickplay_para(demuxer_t *demuxer);
void handle_pending_event(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();

    int log_level = mlog_check_gobal_log_level();
    if (log_level) {
        av_log_set_level(log_level);
    }
    if (IS_SET(p_file_seq->internal_event, AUTO_DELIVER_VPTS)) {
        notify_pts(p_file_seq);
    }

    if (IS_SET(p_file_seq->internal_event, CHECK_TRICKPLAY)) {
        //printf("################TRICK PLAY EVENT\n");
        if (p_file_seq->cur_speed >= TS_SEQ_REV_FAST_PLAY_2X) {
            //back to start and play normal
            int speed_index = 0;
            switch (p_file_seq->cur_speed) {
                case TS_SEQ_REV_FAST_PLAY_2X:
                    speed_index = 2;
                    break;
                case TS_SEQ_REV_FAST_PLAY_4X:
                    speed_index = 4;
                    break;
                case TS_SEQ_REV_FAST_PLAY_8X:
                    speed_index = 8;
                    break;
                case TS_SEQ_REV_FAST_PLAY_16X:
                    speed_index = 25;
                    break;
                case TS_SEQ_REV_FAST_PLAY_32X:
                    speed_index = 50;
                    break;
                default:
                    speed_index = 16;
                    break;
            }

            if (((p_file_seq->orig_vpts - p_file_seq->first_vpts) < 0.5 * 1000 ) ||
				((p_file_seq->orig_vpts - p_file_seq->video_start_pts) < 0.5 * 1000 )|| //add for first_pts change bug24232
                 (speed_index && (speed_index != 16) && (int)(p_file_seq->vpts_upload / (1000 * 1000) == 0))) { // bug19111 @weixing 20230620
                MLOGI("Back to head play normal, org:%lfms first:%lfms up:%lldms speed:%d idx:%d\n",
                    p_file_seq->orig_vpts, p_file_seq->first_vpts,
                    p_file_seq->vpts_upload / 1000, p_file_seq->cur_speed, speed_index);

                p_file_seq->vpts_upload = 3;
                if (p_file_seq->is_support_seek_finish) {
                    vdec_flush(p_file_seq->pb_internal);
                    (void) demux_seek(p_file_seq->p_demuxer, 0.0, 0.0, SEEK_ABSOLUTE);
                    ds_reset_trickplay_para(p_file_seq->p_demuxer);
                } else {
                    vdec_freeze_stop(p_file_seq->p_vdec_dev);
                }

                vdec_file_clearesbuffer(p_file_seq->pb_internal); //linda zhu, fix bug 54642, write_pointer of es_buffer is not reset in vdec_file_clearesbuffer of CHIPTYPE_CONCERTO, vdec need to be restarted.
                fileplay_set_avsync_mode(MT_UNF_SYNC_REF_AUDIO);

                if (!p_file_seq->is_support_seek_finish) {
                    player_vdec_start(p_file_seq, p_file_seq->m_video_codec_type, 2);
                    player_vdec_trick_mode(p_file_seq, (int) p_file_seq->cur_speed, TS_SEQ_NORMAL_PLAY);
                    vdec_resume(p_file_seq->p_vdec_dev);
                    p_file_seq->cur_speed  = TS_SEQ_NORMAL_PLAY;
                    p_file_seq->last_speed = TS_SEQ_NORMAL_PLAY;
                    p_file_seq->tmp_speed  = 0;
                    //mute set by up layer
                    //send to es open
                    //audio start
                    p_file_seq->isNormalPlay = MT_TRUE;
                    p_file_seq->isTrickPlay  = MT_FALSE;
                    p_file_seq->isTrickToNormal = MT_TRUE;

                    if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
                        set_audio_param_to_vsb();
                    }
                }
                else {
                    p_file_seq->m_tmp_ves_buf_pos = 0;
                }

                p_file_seq->last_speed = p_file_seq->cur_speed;
                p_file_seq->needNewAudioData  = MT_TRUE;
                p_file_seq->needNewVideoData  = MT_TRUE;
                p_file_seq->isAudioBufferFull = MT_FALSE;
                p_file_seq->isVideoBufferFull = MT_FALSE;
                if (NULL != p_file_seq->event_cb) {
                    p_file_seq->event_cb(FILE_PLAYBACK_SEQ_INVALID, 0);
                    MLOGI("Send event playback to head\n");
                }
                if (NULL != p_file_seq->event_cb) {
                    p_file_seq->event_cb(FILE_PLAYBACK_CHECK_TRICKPLAY, 0);
                    MLOGI("Send event check trickplay none\n");
                }

                ClearFlag(p_file_seq->internal_event, CHECK_TRICKPLAY);
                if (p_file_seq->event_cb) {
                    p_file_seq->event_cb(FILE_PLAYBACK_TRICKMODE_LEAVE, 0);
                }

                if (!p_file_seq->is_support_seek_finish) {
                    (void) demux_seek(p_file_seq->p_demuxer, 0.0, 0.0, SEEK_ABSOLUTE) ;
                }
                fp_enable_stream_file_header(p_file_seq);
            }
        } else if (fp_is_timeshift_file() == 1) {
            //back to start and play normal
        } else {
            ClearFlag(p_file_seq->internal_event, CHECK_TRICKPLAY);
        }
    }

    if (IS_SET(p_file_seq->internal_event, CLEAR_AUD_ES_BUF)) {
        ClearFlag(p_file_seq->internal_event, CLEAR_AUD_ES_BUF);
        return ;
    }

    if (IS_SET(p_file_seq->internal_event, CLEAR_VIDEO_ES_BUF)) {
        ClearFlag(p_file_seq->internal_event, CLEAR_VIDEO_ES_BUF);
        return ;
    }

    if (IS_SET(p_file_seq->internal_event, GET_TS_MEDIA_INFO) || IS_SET(p_file_seq->internal_event, GET_ES_VIDEO_W_H)) {
        if (p_file_seq->video_whfps.video_disp_w != 0 && p_file_seq->video_whfps.video_disp_h != 0) {
            MLOGD("Send video info %dx%d (w x h) %d fps cb:%p\n", p_file_seq->video_whfps.video_disp_w,
                p_file_seq->video_whfps.video_disp_h, p_file_seq->video_fps,  p_file_seq->event_cb);

            if (NULL != p_file_seq->event_cb) {
                p_file_seq->event_cb(FILE_PLAYBACK_SEQ_RECEIVE_VIDEO_INFO, (unsigned long) &(p_file_seq->video_whfps));
            }

            if (IS_SET(p_file_seq->internal_event, GET_TS_MEDIA_INFO)) {
                ClearFlag(p_file_seq->internal_event, GET_TS_MEDIA_INFO);
            } else {
                ClearFlag(p_file_seq->internal_event, GET_ES_VIDEO_W_H);
            }
        }
        return ;
    }

    if (IS_SET(p_file_seq->internal_event, CHECK_SYSTEM_MEM)) {
        MT_BOOL isFlushCacheMem = TRUE;

        run_memory(isFlushCacheMem);

        ClearFlag(p_file_seq->internal_event, CHECK_SYSTEM_MEM);
        return ;
    }
    return;
}
