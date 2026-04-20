/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "drv_adp.h"
#include "mtos_sem.h"
#include "mtos_task.h"
#include "mtos_misc.h"

#define MODULE_TAG "FP CMD"
#define THREAD_NAME "Trick"
#include "mlog.h"
#include "fifo_kw.h"
#include "mt_type.h"
#include "libmpdemux/stheader.h"
#include "libmpdemux/demuxer.h"
#include "stream/stream.h"

#include "ts_sequence.h"
#include "file_playback_sequence.h"
#include "demux_mp.h"
#include "file_seq_internal.h"
#include "pts_list.h"
#include "libavformat/avformat.h"
#include "mt_unf_video.h"

extern Node *list_I_vpts;
extern int io_isnetworkstream ;
extern double first_vpts;

#define TRICK_STEP_MS  1000


typedef struct SWITCH_AUD_TRACK {
    unsigned int cur_play_vpts;
    unsigned int pkt_vpts;
    unsigned int pkt_apts;
    unsigned int pkt_asize;
    unsigned int pkt_vsize;
    //CHECK SUPPORT
    int next_audio_support;
    //SEEK
    double vpts_beforeswicth;
    //DROP
    //FIND VIDEO BREAK POINT
    int find_vid_bt;
    //PUSH AUDIO
} aud_track_t;

enum SWITCH_AUD_TRACK_EXIT {
    SWITCH_AUDTRACK_EXIT0 = 1,
    SWITCH_AUDTRACK_EXIT1 = 2,
    SWITCH_AUDTRACK_EXIT2 = 3,
};
static int do_user_cmd_conv_speed(const int user_speed)
{
    int idx;
    int our_speed = -1;
    static const struct {
        short your;
        short our;
    } speed_list[] = {
        {-32, TS_SEQ_REV_FAST_PLAY_32X},
        {-16, TS_SEQ_REV_FAST_PLAY_16X},
        { -8, TS_SEQ_REV_FAST_PLAY_8X },
        { -4, TS_SEQ_REV_FAST_PLAY_4X },
        { -2, TS_SEQ_REV_FAST_PLAY_2X },
        {  0, TS_SEQ_NORMAL_PLAY      },
        {  1, TS_SEQ_NORMAL_PLAY      },
        {  2, TS_SEQ_FAST_PLAY_2X     },
        {  4, TS_SEQ_FAST_PLAY_4X     },
        {  8, TS_SEQ_FAST_PLAY_8X     },
        { 16, TS_SEQ_FAST_PLAY_16X    },
        { 32, TS_SEQ_FAST_PLAY_32X    },
    };

    for (idx = 0; idx < ARRAY_CNT(speed_list); idx++) {
        if (user_speed == (int) speed_list[idx].your) {
            our_speed = (int) speed_list[idx].our;
            break;
        }
    }

    return our_speed;
}

/*
 * NOTICE: please modify the function 'x_push_user_cmd' carefully
 * you shouldn't modify the variable 'rd_pos' !!!!!!!!!!!!
 *
 */
int x_push_user_cmd(void *hdl, FP_USER_CMD_T *p_cmd)
{
    MLOGD("[%s] start start ...\n ", __func__);
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    int tmp = 0;

    if (p_file_seq->is_cmd_buf_full) {
        MLOGE("[%s]CMD BUFFER IS FULL...!!!!!\n ", __func__);
        return -1;
    }

    /*cmd ring buffer is not full*/
    if (p_file_seq->rd_pos <=  p_file_seq->wr_pos) {
        memcpy(p_file_seq->cmd_fifo + p_file_seq->wr_pos, p_cmd, sizeof(FP_USER_CMD_T));
        tmp = (p_file_seq->wr_pos + 1) % USER_CMD_FIFO_LEN;

        /*rd_pos == 0; and wr_pos wrap back to 0*/
        if (tmp == p_file_seq->rd_pos) {
            p_file_seq->is_cmd_buf_full = 1;
        }
    } else { //rd_pos > wr_pos
        memcpy(p_file_seq->cmd_fifo + p_file_seq->wr_pos, p_cmd, sizeof(FP_USER_CMD_T));
        tmp = p_file_seq->wr_pos + 1 ;//wr

        if (tmp == p_file_seq->rd_pos) {
            p_file_seq->is_cmd_buf_full = 1;
        }
    }

    p_file_seq->wr_pos = tmp;
    return 0;
}

/*
 *   NOTICE:  please modify the function 'x_pop_user_cmd' carefully
 *
 *   you shouldn't modify the variable 'wr_pos' !!!!!!!!!!!!
 */
int x_pop_user_cmd(void *hdl,  FP_USER_CMD_T *p_out_cmd)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    int tmp = 0;

    if (p_file_seq->rd_pos  !=  p_file_seq->wr_pos  || p_file_seq->is_cmd_buf_full) {
        memcpy(p_out_cmd, p_file_seq->cmd_fifo + p_file_seq->rd_pos, sizeof(FP_USER_CMD_T));
        tmp = (p_file_seq->rd_pos + 1) % USER_CMD_FIFO_LEN;
        p_file_seq->rd_pos = tmp;
        p_file_seq->is_cmd_buf_full = 0;
        return 0;
    }

    return -1;
}

/*
 *    NOTICE:  please modify the function 'x_pop_user_cmd' carefully
 *    you shouldn't modify the variable 'wr_pos' !!!!!!!!!!!!
 */
void x_clear_cmd_fifo(void *hdl)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    p_file_seq->is_cmd_buf_full = 0;
    p_file_seq->wr_pos = p_file_seq->rd_pos;
    return;
}

/*
 *    NOTICE:  please modify the function 'x_pop_user_cmd' carefully
 *    you shouldn't modify the variable 'wr_pos' !!!!!!!!!!!!
 */
void x_reset_cmd_fifo(FILE_SEQ_T *p_file_seq)
{
    p_file_seq->is_cmd_buf_full = 0;
    p_file_seq->wr_pos = 0;
    p_file_seq->rd_pos = 0;
    memset(p_file_seq->cmd_fifo, 0, USER_CMD_FIFO_LEN * sizeof(FP_USER_CMD_T));
}

int is_audio_type_support(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    int is_support = 0;

    p_file_seq->is_av_codec_support = p_file_seq->is_av_codec_support & 0xfffe;
    (void) ds_get_audio_codec_info(p_file_seq->p_ds_audio, &p_file_seq->m_audio_codec_type,
           &pbi->audio.pcm_be, &pbi->audio.codec_id, &p_file_seq->audio_pid, p_file_seq->audio_output_mode);

    is_support = fp_is_support_audio_codec(p_file_seq, p_file_seq->m_audio_codec_type);
    if (MT_TRUE == is_support) {
        MLOGI("=====> audio type : 0x%x \n", p_file_seq->m_audio_codec_type);
        p_file_seq->is_av_codec_support |= 1;
    }

    return is_support;
}

/*************************************************************************************
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 **************************************************************************************/
static int get_cur_video_pts(void)
{
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();

    u64 vpts = 0 ;
    u32 video_free_space_size = 0;
    u32 es_buf_size = 0;
    u32 cur_pts = 0;   //minisecond
    u32 cur_ticks = 0, start_ticks = mtos_ticks_get();
    while (!vpts) {
        drv_pts_info_t vstate;
        //vdec_get_info(p_file_seq->p_vdec_dev, &vstate);
        vdec_get_pts(p_file_seq->p_vdec_dev, &vstate);
        vpts = vstate.pts;
        MLOGD("\n[%s_%d]time=%lld\n", __func__, __LINE__, vpts);
        if (p_file_seq->m_play_state ==  FILE_SEQ_EXIT || p_file_seq->m_play_state == FILE_SEQ_STOP) {
            return 0;
        }

        if (vpts != 0) {
            cur_pts = vpts / TIME_BASE;
            break;
        } else { //if there is no data in es buf for a long time, H264DecInit int will set pts_in_slot[loop] = 0; then use p_file_seq->orig_vpts instead.
            vdec_get_es_buf_space(p_file_seq->pb_internal, (u32 *)&video_free_space_size);
            vdec_get_es_buf_size(p_file_seq->pb_internal, (u32 *)&es_buf_size);

            if (video_free_space_size == es_buf_size) {
                cur_pts = p_file_seq->orig_vpts;
                break;
            }
        }
        cur_ticks = mtos_ticks_get();
        if ((cur_ticks > start_ticks && (cur_ticks - start_ticks > 150)) || (cur_ticks < start_ticks && (start_ticks - cur_ticks > 150))) {
            return p_file_seq->orig_vpts;
        }
    }
    return cur_pts;
}

/*discard video packet , avoid video ES repeat*/
static void find_video_parse_break_point(FILE_SEQ_T *p_file_seq, aud_track_t *aud_track)
{
    int now_pack_magic;
    PLAYBACK_INTERNAL_T *p_pb_internal = NULL;
    demux_stream_t *ds_v = NULL;

    p_pb_internal = p_file_seq->pb_internal;
    ds_v = p_file_seq->p_cur_ds_video;

    while (ds_v->bytes > 0) {
        now_pack_magic = get_pack_magic_num(p_file_seq, aud_track->pkt_vsize, aud_track->pkt_vpts);
        if (now_pack_magic != p_pb_internal->pack_magic) {
            aud_track->pkt_vsize = get_video_es_packet(p_file_seq, &(aud_track->pkt_vpts));
            if ( aud_track->pkt_vsize <= 0 || p_file_seq->p_v_pkt_start == NULL) {
                MLOGE("[%s][ERROR] video packet size [%d]!!!!\n", __func__, aud_track->pkt_vsize);
                if (ds_v->eof) {
                    break;
                }
            }
        } else {
            MLOGI("the breakpoint of video parse\n\t\t[new %d\t old %d]: break point video\n",
                p_pb_internal->pack_magic, now_pack_magic);
            aud_track->find_vid_bt = 1;
            break;
        }
    }
}

/* push the packets into audio es buffer ,
and discard the non-necessory video packets
*/
static int switch_audtrack_push_audio(FILE_SEQ_T *p_file_seq, aud_track_t *aud_track)
{
    int puch_packet_cnt = 0;
    int push_aud_total = 0;
    int ret = 0;
    PLAYBACK_INTERNAL_T *p_pb_internal = NULL;
    unsigned int *p_aud_pkt_size = &(aud_track->pkt_asize);
    demux_stream_t *ds_v = NULL;
    unsigned int *p_pkt_apts = &(aud_track->pkt_apts);
    unsigned int *p_pkt_asize = &(aud_track->pkt_apts);
    unsigned int get_packet_vpts = 0;
    unsigned int cur_play_vpts = aud_track->cur_play_vpts;
    u32 audio_pts = 0;

    ds_v = p_file_seq->p_cur_ds_video;
    p_pb_internal = p_file_seq->pb_internal;

    do {
        if (p_file_seq->m_play_state == FILE_SEQ_EXIT || p_file_seq->m_play_state == FILE_SEQ_STOP) {
            return SWITCH_AUDTRACK_EXIT0;
        }
        setup_audio_pts(p_file_seq, *p_aud_pkt_size);
        ret = push_audio_es_packet(p_file_seq);
        if (ret < 0) {
            MLOGD("[%s][%d] push audio es is ERROR !!!\n", __func__, __LINE__);
            break;
        }
        puch_packet_cnt++;
        push_aud_total += *p_aud_pkt_size;

        MLOGD("get_packet_apts = %d ms,cur_play_vpts=%d, puch_packet_cnt=%d,totalsize = %d\n",
        *p_pkt_apts, (int)(cur_play_vpts), puch_packet_cnt, push_aud_total);
        if ( *p_pkt_apts > (int)(cur_play_vpts + 1000)
            || push_aud_total > 256 * 1024
            || puch_packet_cnt > 200) {
            MLOGI("[%s] ------ pushed the last packet( apts=orig_apts )to aes buffer!! break!"
                "get_packet_apts = %d ms,cur_play_vpts=%d, puch_packet_cnt=%d\n",
                __func__, *p_pkt_apts, (int)(cur_play_vpts), puch_packet_cnt);
            break;
        }

        *p_aud_pkt_size = get_audio_es_packet(p_file_seq, p_pkt_apts);
        if ( aud_track->find_vid_bt == 0) {
            find_video_parse_break_point(p_file_seq, aud_track);
        }
    } while (1);

    return 0;
}

static int switch_audtrack_check_audio_support(FILE_SEQ_T *p_file_seq, aud_track_t *aud_track)
{
    demuxer_t *p_demuxer = (demuxer_t *)p_file_seq->p_demuxer;
    int ts_apid = 0;
    int cur_audio_support = 0;
    int next_audio_support = 0;

    cur_audio_support = is_audio_type_support();
    MLOGI("%s, %d, is_ts [%d], audio_track_id [%d]\n", __func__, __LINE__, p_file_seq->is_ts, p_file_seq->audio_track_id);
    if (p_file_seq->is_ts) {
        ts_apid = ts_get_audio_track_pid(p_demuxer, p_file_seq->ts_priv, p_file_seq->audio_track_id);
        p_file_seq->audio_pid = ts_apid;
        demuxer_switch_audio(p_demuxer, ts_apid);
    } else {
        demuxer_switch_audio(p_demuxer, p_file_seq->audio_track_id);
    }
    next_audio_support = is_audio_type_support();
    aud_track->next_audio_support = next_audio_support;
    if ((cur_audio_support == next_audio_support) && (cur_audio_support == 0)) {
        return SWITCH_AUDTRACK_EXIT1;
    }
    if (cur_audio_support == 1 && next_audio_support == 0) {
        return SWITCH_AUDTRACK_EXIT2;
    }

    if (p_demuxer->type == DEMUXER_TYPE_LAVF) {
        unsigned long *priv = (unsigned long *)(p_demuxer->priv);
        AVInputFormat *avif = (AVInputFormat *)( *priv);
        if (strstr(avif->name, "hls") && cur_audio_support == 1 && next_audio_support == 1) {
            MLOGE("avif->name %s goto EXIT_1;\n", avif->name);
            return SWITCH_AUDTRACK_EXIT1;
        }

        if (strstr(avif->name, "dash")) {
            MLOGE("avif->name %s goto EXIT_1;\n", avif->name);
            return SWITCH_AUDTRACK_EXIT1;
        }
    }

    return 0;
}

static int switch_audtrack_seek_back(FILE_SEQ_T *p_file_seq, aud_track_t *aud_track)
{
    u32 prev_I_pts = 0;
    u32 post_I_pts = aud_track->cur_play_vpts;
    unsigned int *p_play_vpts = &(aud_track->cur_play_vpts);
    u32 accurate_pts = 0;
    float  seek_step = 0.3;
    int seek_times = 0;
    unsigned int *p_aud_pkt_size = &(aud_track->pkt_asize);
    unsigned int *p_pkt_apts = &(aud_track->pkt_apts);
    unsigned int *p_vid_pkt_size = &(aud_track->pkt_vsize);
    unsigned int *p_pkt_vpts = &(aud_track->pkt_vpts);
    demux_stream_t *ds_a = NULL;
    demux_stream_t *ds_v = NULL;
    int ret = 0;
    unsigned int get_packet_apts_pre = 0;
    ds_a = p_file_seq->p_cur_ds_audio;
    ds_v = p_file_seq->p_cur_ds_video;

    p_file_seq->seek_seconds = ( *p_play_vpts - (int)(p_file_seq->file_start_time * 1000)) / 1000.0; //-33;
    if (p_file_seq->is_ts) {
        p_file_seq->seek_seconds -= seek_step;
    }
    p_file_seq->last_audio_pts = 0;

    do {
        //seek back until find the audio packet of current pts
        if (list_I_vpts && ListLength(list_I_vpts) > 0 && 0 == seek_times) {

            /*ret_pts =*/ get_accurate_vpts_I_frame(list_I_vpts, *p_play_vpts, &prev_I_pts, &post_I_pts);

            //feyang fix bug 133379
            if (post_I_pts < *p_play_vpts) {
                accurate_pts = post_I_pts - p_file_seq->first_vpts;
            } else {
                accurate_pts = prev_I_pts - p_file_seq->first_vpts;
            }
            //end fix bug 133379

            /* demuxed I pts not reliable */
            demux_seek(p_file_seq->p_demuxer, (accurate_pts / 1000.0), 0, 1) ;
        } else {
            MLOGD("\n[%s_%d]:seek to time=%d\n", __func__, __LINE__, (int)(p_file_seq->seek_seconds * 1000));
            demux_seek(p_file_seq->p_demuxer, p_file_seq->seek_seconds, 0, 1) ;
        }

        seek_times++;
        p_file_seq->extra_audio_size = 0;
        p_file_seq->p_extra_aud_buf = NULL;
        *p_aud_pkt_size = fpi_get_ds_audio_packet(p_file_seq, (void *) ds_a, &(p_file_seq->p_a_pkt_start),
            &(p_file_seq->p_extra_aud_buf), (uint8_t *)( &(p_file_seq->extra_audio_size)));
        if ( *p_aud_pkt_size <= 0 || p_file_seq->p_a_pkt_start == NULL) {
            MLOGE("[%s][ERROR] audio packet size [%d]!!!!\n", __func__, *p_aud_pkt_size);
            break;
        }

        *p_pkt_apts = ds_a->pts * 1000;//ms
        p_file_seq->left_a_pkt_bytes = *p_aud_pkt_size;
        *p_play_vpts = get_cur_video_pts();

        *p_vid_pkt_size = get_video_es_packet(p_file_seq, p_pkt_vpts);

        //check pts
        if (*p_pkt_apts <= *p_play_vpts && *p_pkt_vpts <= *p_play_vpts) {
            p_file_seq->seek_seconds = 0;
            MLOGI("[%s] seek back ok, seek %d times, next apts %d, next vpts:%d cur vpts %d\n",
                __func__, seek_times, *p_pkt_apts, *p_pkt_vpts, *p_play_vpts);
            break;
        }

        // ts to do
        if (p_file_seq->is_ts) {
            if (seek_times > 3) {
                break;
            }
        /* back 30 * 0.3 about 10 seconds */
        } else if (seek_times > 30) {
            MLOGW("Warning, demux seek 30 times\n");
            break;
        }

        if (p_file_seq->is_ts) {
            p_file_seq->seek_seconds -= seek_times *seek_step;
        } else {
            p_file_seq->seek_seconds -= seek_step;
        }

        if ( *p_pkt_apts == get_packet_apts_pre) {
            p_file_seq->seek_seconds = p_file_seq->seek_seconds - seek_times;
        }
        get_packet_apts_pre = *p_pkt_apts;
        if (p_file_seq->m_play_state == FILE_SEQ_EXIT || p_file_seq->m_play_state == FILE_SEQ_STOP) {
            ret = SWITCH_AUDTRACK_EXIT0;
            break;
        }
    } while (1);

    return ret;
}

static int switch_audtrack_drop_audpkt(FILE_SEQ_T *p_file_seq, aud_track_t *aud_track)
{
    unsigned int *p_pkt_apts = &(aud_track->pkt_apts);
    unsigned int *p_aud_pkt_size = &(aud_track->pkt_asize);
    demux_stream_t *ds_a = NULL;
    ds_a = p_file_seq->p_cur_ds_audio;
    int ret = 0;

    while ( *p_pkt_apts < aud_track->cur_play_vpts) {
        p_file_seq->extra_audio_size = 0;
        p_file_seq->p_extra_aud_buf = NULL;

        *p_aud_pkt_size = fpi_get_ds_audio_packet(p_file_seq, (void *) ds_a, &(p_file_seq->p_a_pkt_start),
                          &(p_file_seq->p_extra_aud_buf), (uint8_t *)( &(p_file_seq->extra_audio_size)));
        if ( *p_aud_pkt_size <= 0 || p_file_seq->p_a_pkt_start == NULL) {
            MLOGD("[%s][ERROR] audio packet size [%d]!!!!\n", __func__, *p_aud_pkt_size);
            if (ds_a->eof) {
                break;
            }
        }

        *p_pkt_apts = ds_a->pts * 1000;//ms
        p_file_seq->left_a_pkt_bytes = *p_aud_pkt_size;

        if (aud_track->find_vid_bt == 0) {
            find_video_parse_break_point(p_file_seq, aud_track);
        }

        if (p_file_seq->m_play_state == FILE_SEQ_EXIT || p_file_seq->m_play_state == FILE_SEQ_STOP) {
            ret = SWITCH_AUDTRACK_EXIT0;
            break;
        }
    }

    return ret;
}


/*discard video packet , avoid video ES repeat
  find the find the video pkt before switch audio track
*/
static int find_vpkt_before_switch_audtrak(FILE_SEQ_T *p_file_seq, aud_track_t *aud_track)
{
    PLAYBACK_INTERNAL_T *p_pb_internal = NULL;
    int ret = 0;
    p_pb_internal = p_file_seq->pb_internal;


    if (aud_track->find_vid_bt == 1) {
        MLOGI("already find the pkt which has same magic num\n");
    }
    while (1) {
        if (p_file_seq->m_play_state == FILE_SEQ_EXIT || p_file_seq->m_play_state == FILE_SEQ_STOP) {
            ret = SWITCH_AUDTRACK_EXIT0;
            break;
        }

        if (get_pack_magic_num(p_file_seq, aud_track->pkt_vsize, aud_track->pkt_vpts) == p_pb_internal->pack_magic) {
            aud_track->find_vid_bt = 1;
            MLOGI("find the video pkt before switch audio track!!!!!!!!!\n");
            break;
        }

        aud_track->pkt_vsize = get_video_es_packet(p_file_seq, &aud_track->pkt_vpts);
        if (aud_track->pkt_vsize <= 0 || p_file_seq->p_v_pkt_start == NULL) {
            MLOGE("[%s][ERROR] video packet size [%d]!!!!\n", __func__, aud_track->pkt_vsize);
            break;
        }
    }

    return ret;
}

static int switch_audtrack_check_vpts_diff(FILE_SEQ_T *p_file_seq, aud_track_t *aud_track)
{
    double orig_vpts_sv = 0; // recode video ES packet pts before swtich audio
    int ret = 0;
    unsigned int cur_play_vpts;
    PLAYBACK_INTERNAL_T *p_pb_internal = p_file_seq->pb_internal;

    orig_vpts_sv = p_file_seq->orig_vpts;// just assignment once,for video ES data
    cur_play_vpts = get_cur_video_pts();
    aud_track->cur_play_vpts = cur_play_vpts;
    aud_track->vpts_beforeswicth = orig_vpts_sv;

    MLOGI("cur_vpts:%d start:%lf orig_apts:%lf orig_vpts:%lf atrack:%d ves size:%d vbps:%d\n", \
          cur_play_vpts, p_file_seq->file_start_time, p_file_seq->orig_apts, orig_vpts_sv,
          p_pb_internal->audio_track_cnt,
          p_pb_internal->ves_buf_size, p_file_seq->video_bps);
    if ((cur_play_vpts >= orig_vpts_sv && cur_play_vpts - orig_vpts_sv < 1) ||
        (cur_play_vpts < orig_vpts_sv && orig_vpts_sv - cur_play_vpts < 1)) {
        ret = SWITCH_AUDTRACK_EXIT2;
    }

    return ret;
}
static int do_user_cmd_switch_audio_track(
    FILE_SEQ_T *p_file_seq, PLAYBACK_INTERNAL_T *p_pb_internal)
{
    int ret = 0;
    demuxer_t *p_demuxer = (demuxer_t *)p_file_seq->p_demuxer;

    if (!p_file_seq->needNewVideoData) {
        ret = 0;
        goto EXIT_0;
    }
    if (p_file_seq->left_v_pkt_bytes > 0 || p_file_seq->left_a_pkt_bytes > 0) {
        goto EXIT_0;
    }

    unsigned int cur_play_vpts = 0; // cur playing video  pts
    demux_stream_t *ds_a = NULL;
    demux_stream_t *ds_v = NULL;
    ds_a = p_file_seq->p_cur_ds_audio;
    ds_v = p_file_seq->p_cur_ds_video;
    aud_track_t audtrack = {0};

    MLOGI("********SETP0:check the next audio is support?**********\n");
    ret = switch_audtrack_check_audio_support(p_file_seq, &audtrack);
    if (ret == SWITCH_AUDTRACK_EXIT1) {
        goto EXIT_1;
    } else if (ret == SWITCH_AUDTRACK_EXIT2) {
        goto EXIT_2;
    }


    if (SWITCH_AUDTRACK_EXIT2 ==  switch_audtrack_check_vpts_diff(p_file_seq, &audtrack)) {
        goto EXIT_2;
    }
    if (!(p_demuxer->seekable)) {
        goto EXIT_1;
    }

    MLOGI("STEP1:seek\n");
    ret = switch_audtrack_seek_back(p_file_seq, &audtrack);
    if (ret == SWITCH_AUDTRACK_EXIT0) {
        goto EXIT_0;
    }


    /* discard audio packet, until the apts is the same as before*/
    aud_stop_vsb(p_file_seq->p_audio_dev);
    audtrack.pkt_vsize = get_video_es_packet(p_file_seq, &(audtrack.pkt_vpts));
    audtrack.cur_play_vpts = get_cur_video_pts();
    vdec_pause(p_file_seq->p_vdec_dev); // linux run slower than ucos

    MLOGI("STEP2:drop the audio which pts is same\n");
    if (switch_audtrack_drop_audpkt(p_file_seq, &audtrack) == SWITCH_AUDTRACK_EXIT0) {
        goto EXIT_0;
    }


    MLOGI("STEP3:push adequate audio to buffer\n");
    /*======start audio decoder =======*/
    if (audtrack.next_audio_support) {
        set_audio_param_to_vsb();
        aud_pause_vsb(p_file_seq->p_audio_dev);
    }

    if (switch_audtrack_push_audio(p_file_seq, &audtrack) == SWITCH_AUDTRACK_EXIT0) {
        goto  EXIT_0;
    }

    MLOGI("SETP4:drop video data in order to avoid video data repeat\n");
    ret = find_vpkt_before_switch_audtrak(p_file_seq, &audtrack);
    if (ret == SWITCH_AUDTRACK_EXIT0) {
        goto  EXIT_0;
    }

    //end fix bug start 100046
    vdec_resume(p_file_seq->p_vdec_dev); // linux run slower than ucos
    if (NULL != p_file_seq->event_cb) {
        p_file_seq->event_cb(FILE_PLAYBACK_SWITCH_AUDIO_SPDIF_MODE, (unsigned long) &p_file_seq->m_audio_codec_type);
    }
    aud_resume_vsb(p_file_seq->p_audio_dev);
    p_file_seq->m_play_state = FILE_SEQ_PLAY;

    p_file_seq->needNewAudioData = MT_TRUE;
    if (p_file_seq->left_v_pkt_bytes == 0) {
        p_file_seq->needNewVideoData = MT_TRUE;
    }

    MLOGI("change audio track ok!\n\taudio[pkts:%d, bytes:%d] video[pkts:%d,bytes%d]\n\t orig_pts[audio:%d,video:%d]\n", ds_a->packs,
          ds_a->bytes, ds_v->packs, ds_v->bytes, (int)(p_file_seq->orig_apts), (int)(p_file_seq->orig_vpts));
EXIT_2:
    aud_stop_vsb(p_file_seq->p_audio_dev);
    if (audtrack.next_audio_support) {
        set_audio_param_to_vsb();
    }
EXIT_1:
    ClearFlag(p_file_seq->m_user_cmd, CMD_CHANGE_AUDIO_TRACK);
EXIT_0:
    ret = 0;
    return ret;
}

static void reset_player_trick_para(FILE_SEQ_T *p_file_seq)
{
    p_file_seq->tmp_speed       = 0;
    p_file_seq->isNormalPlay    = MT_TRUE;
    p_file_seq->isTrickPlay     = MT_FALSE;
    p_file_seq->isTrickToNormal = MT_TRUE;
    p_file_seq->last_speed      = p_file_seq->cur_speed = TS_SEQ_NORMAL_PLAY;
}

static int do_aonly_trick(FILE_SEQ_T *p_file_seq)
{
    mt_s64 next_tirck_ms;
    mt_s64 next_tirck_pts;
    int ret             = MT_TRUE;
    int need_trick      = MT_FALSE;
    mt_s64 curr_time_ms = (mt_s64) mtos_ticks_get() * 10;
    demuxer_t *demuxer  = (demuxer_t *) p_file_seq->p_demuxer;

    mtos_sem_take((os_sem_t *)( &(p_file_seq->lock)), 0);
    next_tirck_ms = demuxer->trick.start_time +
        demuxer->trick.trick_cnt * (mt_s64) TRICK_STEP_MS;
    MLOGD("start:(%dms %lldms) speed:%d cnt:%d curr:%lld next:%lld\n",demuxer->trick.start_pts,
        demuxer->trick.start_time, demuxer->trick.speed, demuxer->trick.trick_cnt, curr_time_ms, next_tirck_ms);
    if (curr_time_ms >= next_tirck_ms) {
        next_tirck_pts = demuxer->trick.start_pts +
             demuxer->trick.speed * TRICK_STEP_MS * demuxer->trick.trick_cnt;
        if (next_tirck_pts < 0) {
            ret            = MT_FALSE;
            next_tirck_pts = 0;
        } else if (next_tirck_pts >= (int)(p_file_seq->duration * TIME_BASE)) {
            ret            = MT_FALSE;
            next_tirck_pts = (int)(p_file_seq->duration * TIME_BASE);
        }
        demuxer->trick.trick_cnt++;
        need_trick = MT_TRUE;
    }
    mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));

    if (MT_TRUE == need_trick) {
        p_file_seq->play_at_time(p_file_seq, next_tirck_pts / TIME_BASE);
    }

    return ret;
}

static int exit_trick(FILE_SEQ_T *p_file_seq)
{
    return (MT_TRUE == is_file_seq_exit()         ||
        FILE_SEQ_STOP == p_file_seq->m_play_state ||
        fp_is_normal_play(p_file_seq->cur_speed));
}

static void *trick_aolny_thread(void *arg)
{
    int ret = MT_TRUE;
    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *) arg;
    demuxer_t  *demuxer    = (demuxer_t *)p_file_seq->p_demuxer;

    while (!exit_trick(p_file_seq)) {
        ret = do_aonly_trick(p_file_seq);
        if (MT_TRUE != ret) {
            break;
        }
        mtos_task_sleep(TRICK_STEP_MS / 4);
    }
    demuxer->trick.tid = 0;
    MLOGI("Exit audio Trick\n");
    return NULL;
}

static int change_speed_aonly(FILE_SEQ_T *p_file_seq)
{
    int ret;
    int next_speed;
    /* 1tick equals 10ms */
    mt_s64 curr_time_ms = (mt_s64) mtos_ticks_get() * 10;
    demuxer_t *demuxer  = (demuxer_t *)p_file_seq->p_demuxer;

    next_speed = (s8) do_user_cmd_conv_speed(p_file_seq->tmp_speed);
    if (next_speed < 0) {
        MLOGW("[%s] speed %d not support\n", __func__, p_file_seq->tmp_speed);
        return MT_FALSE;
    }

    if (TS_SEQ_NORMAL_PLAY == next_speed) {
        ret = MT_FALSE;
        reset_player_trick_para(p_file_seq);
        if (0 != demuxer->trick.tid) {
            ret = mlzp_thread_join(demuxer->trick.tid, THREAD_NAME);
        }
        memset(&demuxer->trick, 0, sizeof(demuxer_trick_t));
        return ret;
    }

    if (p_file_seq->cur_speed != (s8) next_speed) {
        /* lock trick start time */
        MLOGI("user set speed:%d, cur:%d, last:%d, next:%d\n",
              p_file_seq->tmp_speed, (int) p_file_seq->cur_speed, (int) p_file_seq->last_speed, next_speed);
        mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
        p_file_seq->last_speed = p_file_seq->cur_speed;
        p_file_seq->cur_speed  = (s8) next_speed;
        demuxer->trick.speed      = p_file_seq->tmp_speed;
        /* apts_upload unit microsecond, but trick not need */
        demuxer->trick.start_pts  = (int)(p_file_seq->apts_upload / TIME_BASE);
        demuxer->trick.trick_cnt  = 0;
        demuxer->trick.start_time = curr_time_ms;
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
        if (0 == demuxer->trick.tid) {
            return mlzp_thread_create(&(demuxer->trick.tid),
                THREAD_NAME, trick_aolny_thread, (void *) p_file_seq);
        }
        return MT_TRUE;
    }

    return MT_TRUE;
}

static void change_speed_seek_vpts(FILE_SEQ_T *p_file_seq,
    const int curr_speed, const int next_speed)
{
    (void) curr_speed;
    int need_seek = MT_FALSE;
    float pts_shift = 0.0f;

    if (p_file_seq->vpts_upload > TIME_BASE &&
        p_file_seq->vpts_upload < INT64_MAX) {
        need_seek = MT_TRUE;
        pts_shift = (float) p_file_seq->vpts_upload / SECOND_TO_MICROSECOND_BASE -
            (float) (p_file_seq->orig_vpts - p_file_seq->first_vpts) / TIME_BASE;
    }

    MLOGI("Seek:%d upload:%lluus orig:%fms first:%fms shift:%fs\n", need_seek,
        p_file_seq->vpts_upload, p_file_seq->orig_vpts, p_file_seq->first_vpts, pts_shift);
    if (fp_is_fast_backward(next_speed)) {
        pts_shift += 1.5;
        p_file_seq->vpts_upload = INT64_MAX;
    } else {
        p_file_seq->vpts_upload = 1;
    }

    if (MT_TRUE == need_seek) {
        (void) demux_seek(p_file_seq->p_demuxer, pts_shift, 0, 0);
    }
}

static int do_user_cmd_change_speed(
    FILE_SEQ_T *p_file_seq, PLAYBACK_INTERNAL_T *p_pb_internal)
{
    unsigned int change_trick_mode_only;

    mp_ffmpeg_ext_cmd(MP_FFMPEG_SET_FILEPLAY_SPEED, MP_DO_SET_PARAM, (void *)(long)p_file_seq->tmp_speed);
    //jqw@20180928 for trickplay
    if (p_file_seq->only_audio_mode) {
        return change_speed_aonly(p_file_seq);
    }

    if (p_file_seq->unable_trickplay) {
        if (NULL != p_file_seq->event_cb) {
            p_file_seq->event_cb(FILE_PLAYBACK_UNSUPPORT_TRICK, 0);
        }
        return 0;
    }

    int next_speed = do_user_cmd_conv_speed(p_file_seq->tmp_speed);
    MLOGI("user set speed:%d, cur:%d, last:%d, next:%d\n",
        p_file_seq->tmp_speed, (int) p_file_seq->cur_speed, (int) p_file_seq->last_speed, next_speed);

    if (next_speed < 0) {
        MLOGW("[%s] not support %d\n", __func__, p_file_seq->tmp_speed);
        return 0;
    }

    if (next_speed == p_file_seq->cur_speed) {
        MLOGI("[%s] do nothing  !!!!!!\n", __func__);
        return 1;
    }

    if (TS_SEQ_NORMAL_PLAY == next_speed) {
        if (p_file_seq->event_cb) {
            MLOGI("[%s] FILE_PLAYBACK_TRICKMODE_LEAVE\n", __func__);
            p_file_seq->event_cb(FILE_PLAYBACK_TRICKMODE_LEAVE, 0);
        }
    } else {
        if (p_file_seq->event_cb) {
            MLOGI("[%s] FILE_PLAYBACK_TRICKMODE_ENTER, next_speed:%d\n", __func__, next_speed);
            p_file_seq->event_cb(FILE_PLAYBACK_TRICKMODE_ENTER, next_speed);
        }
    }

    if(TS_SEQ_NORMAL_PLAY == next_speed) {
        fileplay_set_avsync_mode( MT_UNF_SYNC_REF_AUDIO);
    }
    else  {
        fileplay_set_avsync_mode( MT_UNF_SYNC_REF_NONE);
    }

    //3. 2x <---->normal do nothing
    change_trick_mode_only =
        fp_is_normal_to_fast2x(p_file_seq->cur_speed, next_speed);
    if (change_trick_mode_only) {
        MLOGI("[%s] Set trick mode only!\n", __func__);
        player_vdec_trick_mode(p_file_seq, (int) p_file_seq->cur_speed, next_speed);
    } else {
        mtos_sem_take((os_sem_t *)(&(p_file_seq->lock)), 0);
        if (!io_isnetworkstream) {
            demux_stream_t *ds_a = NULL;
            demux_stream_t *ds_v = NULL;
            ds_a = p_file_seq->p_cur_ds_audio;
            ds_v = p_file_seq->p_cur_ds_video;
            ds_v->eof = 0;
            p_file_seq->isVideoEsEnd = MT_FALSE;
            if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
                ds_a->eof = 0;
                p_file_seq->isAudioEsEnd = MT_FALSE;
                p_pb_internal->is_audio_support = MT_TRUE;
            }
        }

        if (MT_UNF_VCODEC_TYPE_HEVC != p_file_seq->m_video_codec_type) {
            vdec_freeze_stop(p_file_seq->p_vdec_dev);
        } else {
            vdec_pause(p_file_seq->p_vdec_dev);
        }

        vdec_file_clearesbuffer(p_file_seq->pb_internal);
        if (MT_UNF_VCODEC_TYPE_HEVC != p_file_seq->m_video_codec_type) {
            player_vdec_start(p_file_seq, p_file_seq->m_video_codec_type, 2);
        }

        player_vdec_trick_mode(p_file_seq, (int) p_file_seq->cur_speed, next_speed);
        vdec_resume(p_file_seq->p_vdec_dev);
        if (next_speed == TS_SEQ_NORMAL_PLAY || next_speed == TS_SEQ_FAST_PLAY_2X) {
            //send to es open
            //audio start
            p_file_seq->isNormalPlay = MT_TRUE;
            p_file_seq->isTrickPlay  = MT_FALSE;
            p_file_seq->isTrickToNormal = MT_TRUE;
            if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
                set_audio_param_to_vsb();
            }

            //clear sub fifo
            if (((demuxer_t *)(p_file_seq->p_cur_demuxer))->subt_info.cnt > 0) {
                MLOGD("[%s] clear sub fifo\n", __func__);
                clear_sub_fifo_kw(p_file_seq->p_sub_fifo_handle);    //clear fifo when do seek
            }
        } else if (p_file_seq->cur_speed == TS_SEQ_FAST_PLAY_2X ||
            p_file_seq->cur_speed == TS_SEQ_NORMAL_PLAY) {
            //stop es send
            //audio stop
            p_file_seq->isNormalPlay = MT_FALSE;
            p_file_seq->isTrickPlay = MT_TRUE;
            p_file_seq->isTrickToNormal = MT_FALSE;

#if 0   //for tts ,don't stop adec
            if (p_file_seq->m_audio_codec_type != HA_AUDIO_ID_INVALID) {
                aud_stop_vsb(p_file_seq->p_audio_dev);
            }
#endif
            if (p_file_seq->cur_speed == 0) {
                p_file_seq->last_v_average_bps = p_file_seq->video_average_bps;
            }
        }
        mtos_sem_give((os_sem_t *)(&(p_file_seq->lock)));
    }

    if (!change_trick_mode_only) {
        p_file_seq->left_a_pkt_bytes = 0;
        p_file_seq->left_v_pkt_bytes = 0;
        p_file_seq->needNewAudioData = MT_TRUE;
        p_file_seq->needNewVideoData = MT_TRUE;
        p_file_seq->isAudioBufferFull = MT_FALSE;
        p_file_seq->isVideoBufferFull = MT_FALSE;
        change_speed_seek_vpts(p_file_seq, p_file_seq->cur_speed, next_speed);
        fp_enable_stream_file_header(p_file_seq);
    }
    p_file_seq->last_speed = p_file_seq->cur_speed;
    p_file_seq->cur_speed  = (s8) next_speed;
    p_file_seq->last_audio_pts = 0;

    SetFlag(p_file_seq->internal_event, CHECK_TRICKPLAY);
    return 1;
}

static int do_user_cmd_change_subtitle(
    FILE_SEQ_T *p_file_seq, PLAYBACK_INTERNAL_T *p_pb_internal)
{
    int orig_vpts_sv;
    int orig_apts_sv;
    int orig_spts_sv;
    unsigned int vpts = 0 ;
    unsigned int cur_play_pts   = 0;

    demuxer_t *p_demuxer = (demuxer_t *)p_file_seq->p_demuxer;
    if (!(p_demuxer->seekable) || !(p_file_seq->needNewVideoData) || fabs(p_file_seq->orig_spts) == 0) {
        MLOGW("[%s] do nothing! seekable[%d], needNewVideoData[%d] orig_spts[%d]\n",
              __func__, p_demuxer->seekable, p_file_seq->needNewVideoData, (int)(p_file_seq->orig_spts));
        return 0;
    }

    drv_pts_info_t vstate = {0};
    vdec_get_pts(p_file_seq->p_vdec_dev, &vstate);
    vpts = vstate.pts;

    if (vpts == 0) {
        MLOGW("[%s] do nothing! vpts[%d]\n", __func__, vpts);
        return 0;
    }

    cur_play_pts = vpts / TIME_BASE;
    //seek back to the point of playing pts
    orig_vpts_sv = p_file_seq->orig_vpts;
    orig_apts_sv = p_file_seq->orig_apts;
    orig_spts_sv = p_file_seq->orig_spts;
    MLOGD("[%s] ------ cur_play_pts = %d ms, orig_apts = %d ms, orig_vpts = %d ms, orig_spts = %d ms\n",
        __func__, cur_play_pts, orig_apts_sv, orig_vpts_sv, orig_spts_sv);

    if (p_file_seq->orig_spts == 0) {
        orig_spts_sv = p_file_seq->orig_vpts;
        MLOGD("[%s] orig_spts == 0, set orig_spts_sv to orig_vpts: %d\n", __func__, orig_spts_sv);
    }

    p_file_seq->seek_seconds = ((float)cur_play_pts - orig_spts_sv) / 1000.0;//-33;
    return 0;
}

static int do_user_cmd_change_playlist(
    FILE_SEQ_T *p_file_seq, PLAYBACK_INTERNAL_T *pbi)
{
    demuxer_t *demuxer =
        (demuxer_t *) p_file_seq->p_demuxer;
    int ret = demuxer_switch_playlist(demuxer, pbi->playlist_index);

    return ret;
}

int do_user_cmd(void)
{
    FILE_SEQ_T *p_file_seq = x_get_cur_instance();
    PLAYBACK_INTERNAL_T *p_internal = p_file_seq->pb_internal;
    int ret = 0;
    FP_USER_CMD_T cmd;

    memset(&cmd, 0, sizeof(FP_USER_CMD_T));
    if (x_pop_user_cmd(p_file_seq, &cmd) == 0) {
        switch (cmd.type) {
            case CMD_PAUSE:
                MLOGI("[%s] ===do pause\n", __func__);
                ClearFlag(p_file_seq->m_user_cmd, CMD_PAUSE);
                aud_pause_vsb(p_file_seq->p_audio_dev);

                if (p_file_seq->only_audio_mode == FALSE) {
                    vdec_pause(p_file_seq->p_vdec_dev);
                }

                p_file_seq->m_play_state = FILE_SEQ_PAUSE;
                ret = 0;
                break;
            case CMD_RESUME:
                MLOGI("[%s] ===do resume!!!!!\n", __func__);
                ClearFlag(p_file_seq->m_user_cmd, CMD_RESUME);

                if (p_file_seq->only_audio_mode == FALSE) {
                    vdec_resume(p_file_seq->p_vdec_dev);
                }

                mtos_task_sleep(30);
                aud_resume_vsb(p_file_seq->p_audio_dev);
                p_file_seq->m_play_state = FILE_SEQ_PLAY;
                ret = 0;
                break;
            case  CMD_PLAY_AT_TIME:
                MLOGI("[%s] ===do play at time!!!!! seek_seconds %d ms cur %llums duration %d ms\n", __func__,
                    (int)(cmd.param * 1000), p_file_seq->vpts_upload / 1000, (int)(p_file_seq->duration * 1000));
                mp_ffmpeg_ext_cmd(MP_FFMPEG_SET_FILEPLAY_SPEED, MP_DO_SET_PARAM, (void *)1);
                do_playAtTime();
                p_internal->push_idle_time = 0;
                ret = 1;
                break;
            default:
                break;
        }
        return ret;
    }

    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_SPEED)) {
        MLOGI("Change speed start!\n");
        mtos_sem_take((os_sem_t *)(&(p_internal->seek_mutex)), 0);
        ret = do_user_cmd_change_speed(p_file_seq, p_internal);
        ClearFlag(p_file_seq->m_user_cmd, CMD_CHANGE_SPEED);
        p_internal->net_buffer.fp_start_tick = mtos_ticks_get();
        p_internal->net_buffer.is_enbale     = MT_FALSE;
        mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
        MLOGI("Change speed end!\n");
    }

    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_AUDIO_TRACK)) {
        MLOGI("Change audio track start!\n");
        mtos_sem_take((os_sem_t *)(&(p_internal->seek_mutex)), 0);
        ret = do_user_cmd_switch_audio_track(p_file_seq, p_internal);
        p_internal->net_buffer.fp_start_tick = mtos_ticks_get();
        p_internal->net_buffer.is_enbale     = MT_FALSE;
        mtos_sem_give((os_sem_t *)(&(p_internal->seek_mutex)));
        MLOGI("Change audio track end!\n");
    }

    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_SUBT_ID)) {
        MLOGI("Change sub tittle start!\n");
        ret = do_user_cmd_change_subtitle(p_file_seq, p_internal);
        ClearFlag(p_file_seq->m_user_cmd, CMD_CHANGE_SUBT_ID);
        MLOGI("Change sub tittle end!\n");
    }

    if (IS_SET(p_file_seq->m_user_cmd, CMD_CHANGE_PLAYLIST)) {
        MLOGI("Change playlist start...\n");
        ret = do_user_cmd_change_playlist(p_file_seq, p_internal);
        ClearFlag(p_file_seq->m_user_cmd, CMD_CHANGE_PLAYLIST);
        MLOGI("Change playlist %s!\n", (MT_TRUE == ret) ? "success" : "fail");
    }

    return ret;
}
