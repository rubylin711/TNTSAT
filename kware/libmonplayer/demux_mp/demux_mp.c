#include "mt_type.h"
#include "ts_seq/ts_sequence.h"
#include <string.h>

#include <sys/stat.h>
#include "parse_es.h"
#include "libmpdemux/stheader.h"
#include "libmpdemux/demux_ts.h"
#include "file_playback_sequence.h"

#ifdef __LINUX__
#ifndef CFG_ENABLE_FFMPEG_422
#include "libavcodec/mpegvideo.h"
#endif
#endif

#define MODULE_TAG "DX MP"
#include "mutil.h"
#include "mlog.h"
#include "mpcommon.h"
#include "demux_mp.h"
#include "av_helpers.h"
#include "libavformat/avformat.h"

#include "pts_list.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "mplayer_hw.h"
#include "h264_parser.h"
#include "file_seq_internal.h"
#include "drv_adp.h"
#include "pts_list.h"
#include "mtos_task.h"
#include "mtos_misc.h"
#include "file_fake_mpd.h"

static double gop_start, gop_end;
static int write_flag = 0;

static int seek_back_end = 0;
static int seek_back_cnt = 0;
static int seek_cnt = 0;

extern int io_isnetworkstream;
static int packet_cnt = 0;
//jqw@20180928 for trickplay
static int drop_cnt = 0;
static int last_packet_cnt = 1;
static int last_drop_cnt = 0;

extern Node *list_I_vpts;
static float last_pts;
static unsigned char *packet_I_bak_ptr = NULL;
static int packet_I_size = 0;
static unsigned char *packet_I_bak_ptr2 = NULL;
static int packet_I_size2 = 0;

#define DS_SEL_VDEC_TYPE_MPEG12  0
#define DS_SEL_VDEC_TYPE_H264    1
#define DS_SEL_VDEC_TYPE_HEVC    2
#define DS_SEL_VDEC_TYPE_DEFAULT 3
#define DS_SEL_VDEC_TYPE_AVS     5
#define DS_SEL_VDEC_TYPE_MPEG4   6
#define DS_SEL_VDEC_TYPE_VP9     7

typedef enum {
    WMA_CODEC_ID_WMAV1 = 0x160,
    WMA_CODEC_ID_WMAV2 = 0x161,
} WMA_CODEC_ID;

typedef enum {
    SEEK_GOP_START,
    SEEK_GOP_END,
    SEEK_SELECT,
    SEEK_SELECT_END,
} SEEK_TYPE;

//jqw@20180928 for trickplay
typedef struct {
    int slice_type;
    int last_slice_type;
    MT_BOOL idr_flag;
    MT_BOOL ref_flag;
    MT_BOOL last_ref_flag;
    int frame_num;
    int frame_mbs_only;
    int field_flag;
    int bottom_field_flag;
    int last_frame_num;
    int last_bottom_field_flag;
} picture_info_t;

/* ->push end|------> seek -----|push------>push end|
 *           v                  v                   v
 *     gop_start_time      gop_end_time    gop_start_time'
*/
static int select_type = SEEK_SELECT;
/* end push packet pts of last round*/
static float gop_start_time = 0.f;
/* first push packet pts this round */
static float gop_end_time = 0.f;
//static int last_seeked = 0;
static float gst_select_fps = 0.f;
static float last_gop_start = 0.f;
static int seeked = 0;
static float seek_step = 1.f;
static float last_selected_time = 0.f;
//#define STEP_TIME (2.f)
#define STEP_TIME (1.f) //jqw@20180928 for trickplay
#define STEP_TIME_MAX (20.f)  //tizhang@20180910 for 104537
static int g_file_end = 0; //tizhang@20180910 for 104537
static float i_start_time = 0.f; //jqw@20180928 for trickplay
//jqw@20180928 for trickplay begin
#define IDENTIFY_NALU_STARTCODE(x) \
    ((((unsigned char)(x)[0] == 0x00)) && (((unsigned char)(x)[1]) == 0x00) && \
     (((unsigned char)(x)[2]) == 0x01))

#define IDENTIFY_PICTURE_STARTCODE(x) \
    ((((unsigned char)(x)[0] == 0x00)) && (((unsigned char)(x)[1]) == 0x00) && \
     (((unsigned char)(x)[2]) == 0x01) && (((unsigned char)(x)[3]) == 0x00))

#define MAX_STARTCODE_CNT (10)
typedef struct {
    u8 *p_buf;
    u32 sc_cnt;
    u8 sc_val[MAX_STARTCODE_CNT];
    u32 sc_offset[MAX_STARTCODE_CNT];
    u32 sc_size[MAX_STARTCODE_CNT];
    u32 sc_valid[MAX_STARTCODE_CNT];
} sc_info_t;

static sc_info_t g_sc_info = {0};

#define SUPPORT_SPLIT

static int get_fp_fps(void)
{
    int res = 0;
    FILE_SEQ_T *p_file_seq =  file_seq_get_instance();
    if (p_file_seq) {
        res = p_file_seq->video_fps;
    }
    //jqw@20181009 for bug103261
    if (res == 0) {
        res = 25;
    }
    return res;
}

static int gst_forward_select_check_file_end(
    float rate, int sign, float stime, float ctime)
{
    //tizhang@20180910 for 104537
    int end = 0;
    static float ltime = 0.f;

    if (sign == 1) {
        if (seek_step < 2.0f) { //save the first curFrameTime
            ltime = ctime;
        }
        if (seek_step > STEP_TIME_MAX) {
            if (ctime - ltime < 1.0f) {
                end = 1;
            }
        }
        if (end) {
            g_file_end = 1;
        } else if (g_file_end) {
            end = g_file_end;
        }
        ltime = ctime;
    }
    return end;
}

static void set_seek_stop_pos(demuxer_t *demuxer, off_t filepos)
{
    if (demuxer->type == DEMUXER_TYPE_MPEG_TS) {
        demuxer->desc->control(demuxer, DEMUXER_CTRL_SET_SEEK_ENDPOS, &filepos);
    }
}

static int get_seek_stop_flag(demuxer_t *demuxer)
{
    int stop_flag;
    if (demuxer->type == DEMUXER_TYPE_MPEG_TS) {
        demuxer->desc->control(demuxer, DEMUXER_CTRL_GET_SEEK_STOPFLAG, &stop_flag);
        return stop_flag;
    } else {
        return 0;
    }
}

static off_t get_mark_filepos(demuxer_t *demuxer)
{
    off_t filepos;
    if (demuxer->type == DEMUXER_TYPE_MPEG_TS || demuxer->type == DEMUXER_TYPE_LAVF) {
        demuxer->desc->control(demuxer, DEMUXER_CTRL_GET_SEEK_MARKPOS, &filepos);
        return filepos;
    } else {
        return 0;
    }
}

static void set_seek_start_pos(demuxer_t *demuxer, off_t filepos)
{
    if (demuxer->type == DEMUXER_TYPE_MPEG_TS) {
        demuxer->desc->control(demuxer, DEMUXER_CTRL_SET_SEEK_STARTPOS, &filepos);
    }
}

static MT_BOOL is_frame_complete(picture_info_t *p_info, int vdec_type)//jqw@20181010 for MIA_H264_AC3_1080i.ts
{
    if (vdec_type == DS_SEL_VDEC_TYPE_H264) { //h264
        if ((!p_info->frame_mbs_only)
            && p_info->field_flag
            && (p_info->frame_num == p_info->last_frame_num)
            && ((p_info->last_slice_type == AV_PICTURE_TYPE_I && (p_info->slice_type == AV_PICTURE_TYPE_I || p_info->slice_type == AV_PICTURE_TYPE_P))
                || (p_info->slice_type == p_info->last_slice_type))
            && (p_info->last_ref_flag == p_info->ref_flag)) {
            return 0;
        } else {
            return 1;
        }
    } else if (vdec_type == DS_SEL_VDEC_TYPE_AVS) { //avs
        if (p_info->slice_type != AV_PICTURE_TYPE_NONE) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}

//jqw@20180928 for trickplay
static int gst_forward_select(float curFrameTime, float rate, demuxer_t *demuxer, int vdec_type, picture_info_t *p_info)
{
    int sign = rate > 0.1f ? 1 : -1;
    int res = DEMUX_SEL_GET_NEXT_PKT;
    float cur_rate = rate;
    float drop_rate;
    static int delta_time = 0;
    static int time_t0 = 0, time_t1 = 0;
    static int gop_time = 0;
    static int recovery_flag = 1;
    static int last_seeked_filepos = 0;
    static int delta_t = 0;

    MLOGA("s:%d start:%f end:%f cur:%f step:%f rate:%f fps:%f\n", select_type, gop_start_time,
        gop_end_time, curFrameTime, seek_step, rate, gst_select_fps);
    drop_rate = (last_packet_cnt + last_drop_cnt) * 1.f / last_packet_cnt;
    if (last_drop_cnt > 0) {
        cur_rate = rate / (drop_rate);
        if (cur_rate < 1.f) {
            cur_rate = 1.f;
        }
    } else {
        cur_rate = rate;
    }

    switch (select_type) {
        case SEEK_GOP_START: {
            gop_start_time = gop_end_time;
            select_type = SEEK_GOP_END;
            seeked = 0;
            //jqw@20181015,for bug 105243
            time_t0 = 0;
            time_t1 = 0;
            gop_time = 0;
            delta_time = 0;
            delta_t = 0;
            last_seeked_filepos = 0;
            seek_step = 1.f;
            break;
        }

        case SEEK_GOP_END: {
            if (seeked == 0) {
                float seektostart = 0.f;
                //fix bug 117241
                if (((float)(0 - delta_t) / 1000 >= 1.f * rate / gst_select_fps)) {
                    seektostart = gop_start_time + (fabs(cur_rate) * (seek_step * (float)(0 - delta_t)) / 1000) * sign;
                } else {
                    if (i_start_time > 0.f) {
                        seektostart = i_start_time  + (fabs(cur_rate) * (1.f * seek_step / gst_select_fps)) * sign;
                    } else {
                        seektostart = gop_start_time + (fabs(cur_rate) * (1.f * seek_step / gst_select_fps)) * sign;
                    }
                }
                //jqw@201901014 for bug 105022
                //jqw@20190516 for bug 209953,109958
                if (demuxer->stream->eof  == 1 || gst_forward_select_check_file_end(rate, sign, seektostart, curFrameTime)
                    || ((get_mark_filepos(demuxer) == demuxer->movi_end) && (sign == 1))) {
                    gop_end_time = curFrameTime;
                    select_type = SEEK_SELECT;
                    break;
                }

                set_seek_start_pos(demuxer, last_seeked_filepos);
                MLOGA("seek to time %f start\n", seektostart);
                demux_seek(demuxer, seektostart - curFrameTime, 0, 0); //forward 1s
                MLOGA("seek to time %f finish\n", seektostart);
                memset(&g_sc_info, 0, sizeof(sc_info_t));
                set_seek_start_pos(demuxer, 0);
                last_seeked_filepos = stream_tell(demuxer->stream);

                seeked = 1;
            } else {
                float diffTime;
                if (i_start_time > 0.f) {
                    diffTime = curFrameTime - i_start_time;
                } else {
                    diffTime = curFrameTime - gop_start_time;
                }

                int seek_again = (diffTime * sign > fabs(rate * 2) * 1.f / gst_select_fps) ? 0 : 1;
                MLOGA("Diff:%f flag:%d seek_again:%s\n", diffTime,
                    demuxer->video->flags, seek_again ? "TRUE" : "FALSE");
                if (!seek_again) {
                    if (0 == demuxer->video->flags &&
                        demuxer->type == DEMUXER_TYPE_LAVF) {
                        goto finish;
                    }
                    gop_end_time = curFrameTime;
                    time_t0 = mtos_ticks_get() * 10;
                    if (time_t1 > 0) {
                        gop_time = time_t0 - time_t1;
                    }
                    time_t1 = time_t0;

                    select_type = SEEK_SELECT;
                    last_selected_time = curFrameTime;
                    packet_cnt = 1;
                    seek_step = 1.f;
                    i_start_time = 0.f;
                    drop_cnt = 0;
                    res = DEMUX_SEL_PUSH_CUR_PKT;

                    if (vdec_type == DS_SEL_VDEC_TYPE_H264 && p_info->idr_flag == 0) {
                        recovery_flag = 0;
                    } else {
                        recovery_flag = 1;
                    }
                    last_seeked_filepos = 0;
                } else {
                    seek_step += STEP_TIME;
                }
                seeked = 0;
            }
            break;
        }
        case SEEK_SELECT: {
            if (demuxer->stream->eof == 1) {
                res = DEMUX_SEL_GET_NEXT_PKT;
                break;
            }

            if (gop_time > 0) {
                delta_time = (int)(fabs((gop_end_time - gop_start_time) / (rate * 2)) * 1000) - gop_time;    //jqw@20180930 for bug103225
            }

            if ((curFrameTime - gop_end_time  > fabs((gop_end_time - gop_start_time) * 1.f / cur_rate)
                 || (int)(packet_cnt) >= (int)(gst_select_fps * fabs((gop_end_time - gop_start_time) / rate)))
                || (delta_time < (int)(packet_cnt / gst_select_fps - fabs((gop_end_time - gop_start_time) * 1.f / rate) * 1000)) //jqw@20190107 for bug106866
                || recovery_flag == 0) { //jqw@20180930 for bug103225
                if (((vdec_type != DS_SEL_VDEC_TYPE_H264 && vdec_type != DS_SEL_VDEC_TYPE_AVS) ||
                     (is_frame_complete(p_info, vdec_type))) && (p_info->slice_type != AV_PICTURE_TYPE_S_DATA)) {
                    last_gop_start = gop_start_time;
                    gop_start_time = gop_end_time;
                    select_type = SEEK_GOP_END;
                    seeked = 0;
                    last_packet_cnt = packet_cnt;
                    last_drop_cnt = drop_cnt;
                    res = DEMUX_SEL_GET_NEXT_PKT;
                    delta_t += delta_time;
                    break;
                }
            }

            if (p_info->slice_type == AV_PICTURE_TYPE_I) {
                i_start_time = curFrameTime;
            }

            if (p_info->ref_flag == TRUE) {
                if (delta_time > last_packet_cnt) {
                    res = delta_time / last_packet_cnt;
                    //jqw@20181015,for bug 105243
                    if (res > 20) {
                        res = 20;
                    } else { //jqw@20190103, fix bug 106944/105369/106889, when res=5 error occurs
                        res = DEMUX_SEL_SLEEP_AND_PUSH_PKT;
                    }
                    if (res * packet_cnt >= delta_time) {
                        res = DEMUX_SEL_PUSH_CUR_PKT;
                    }

                    if (((vdec_type != DS_SEL_VDEC_TYPE_H264 && vdec_type != DS_SEL_VDEC_TYPE_AVS) ||
                         (is_frame_complete(p_info, vdec_type))) && (p_info->slice_type != AV_PICTURE_TYPE_S_DATA)) {
                        packet_cnt++;
                    }
                } else {
                    res = DEMUX_SEL_PUSH_CUR_PKT;
                    if (((vdec_type != DS_SEL_VDEC_TYPE_H264 && vdec_type != DS_SEL_VDEC_TYPE_AVS) ||
                         (is_frame_complete(p_info, vdec_type))) && (p_info->slice_type != AV_PICTURE_TYPE_S_DATA)) {
                        packet_cnt++;
                    }
                }
            } else {
                if (((vdec_type != DS_SEL_VDEC_TYPE_H264 && vdec_type != DS_SEL_VDEC_TYPE_AVS) ||
                     (is_frame_complete(p_info, vdec_type))) && (p_info->slice_type != AV_PICTURE_TYPE_S_DATA)) {
                    drop_cnt++;
                }
                res = DEMUX_SEL_GET_NEXT_PKT;
            }

            last_selected_time =  curFrameTime;
            break;
        }
        case SEEK_SELECT_END: {
            res = DEMUX_SEL_PUSH_CUR_END_PKT;
            break;
        }
    }
finish:
    MLOGA("res:%d\n", res);
    return res;
}

static int64_t get_video_pkt_pos(demuxer_t *demuxer, demux_stream_t *ds)
{
    if (DEMUXER_TYPE_LAVF == demuxer->type) {
        return ds->pos;
    }

    return stream_tell(demuxer->stream);
}
static void check_backward_position(demuxer_t *demuxer,
    float curFrameTime, float rate, float fps, float last, float *step)
{
    float sk_step = *step;

    mt_s64 pts_diff  = (mt_s64) fabs(curFrameTime * TIME_BASE - demuxer->trick.start_pts);
    mt_s64 time_diff = (mt_s64) mclock_get_utime() / TIME_BASE - demuxer->trick.start_time;
    mt_s64 fr_duration = (mt_s64) TIME_BASE / fps;

    /* Seek a frame duration (unit ms) per time */
    mt_s64 change_xtime = (mt_s64) (time_diff + fr_duration) * fabs(rate);
    mt_s64 pts_step     = (mt_s64) fabs(rate) * sk_step * fr_duration;
    mt_s64 change_xpts  = pts_diff + pts_step;

    /* Back too fast, next seek pts larger than system time in second unit */
    if (change_xpts / TIME_BASE > change_xtime / TIME_BASE) {
        /* large gap from pts step, so step should be adjusted */
        if (change_xtime > pts_diff) {
            /* By math mode change_xpts should equals change_xtime, be careful!!! */
            sk_step = (float) (change_xtime - pts_diff) / (fabs(rate) * fr_duration);
            MLOGA("Change step from %f->%f %lld %lld %lld\n", *step, sk_step, change_xpts, change_xtime, pts_diff);
        }
        /* 1. we should search further data
         * 2. pts already larger than system time, should not increase too large
         */
        if (sk_step <= last || change_xtime < pts_diff) {
            sk_step = last + 1.0f / fps;
        }
    }
    MLOGA("Change step from %f->%f->%f\n", last, *step, sk_step);
    *step = sk_step;
}

static int need_more_backward_packet(demuxer_t *demuxer, float curFrameTime, float first_pts, float rate)
{
    const static int MAX_SLEEP_MS = 250;
    mt_s64 pts_diff_ms  = (mt_s64) fabs(first_pts * TIME_BASE - demuxer->trick.start_pts);
    mt_s64 time_diff_ms = (mt_s64) mclock_get_utime() / TIME_BASE - demuxer->trick.start_time;

    /* time smaller and packet not send before */
    time_diff_ms *= fabs(rate);
    int ret = 0;
    if (time_diff_ms < pts_diff_ms) {
        MT_USLEEP(MAX_SLEEP_MS * TIME_BASE);
        packet_cnt += (int)(MAX_SLEEP_MS*gst_select_fps/1000);
        ret = 1;
    }

    return ret;
}

//jqw@20190107 for bug106866 end
static int gst_backward_select(float curFrameTime, float rate, demuxer_t *demuxer, int vdec_type, picture_info_t *p_info)
{
    int sign = rate > 0.1f ? 1 : -1;
    int res = DEMUX_SEL_GET_NEXT_PKT;
    static int last_res = 0;
    static int start_filepos = 0;
    static int64_t last_seekd_pos[2];
    static int stuck_cnt = 0;
    static int seek_err = 0;
    static float last_seek_pts = 0;
    int need_pkt_flg = 0;

    struct demuxer_trick *trick = &demuxer->trick;
    MLOGA("s:%d , seeked = %d start:%f end:%f cur:%f step:%f rate:%f fps:%f ptype:%d pos:%lld %lld\n", select_type, seeked ,gop_start_time,
        gop_end_time, curFrameTime, seek_step, rate, gst_select_fps, p_info->slice_type, last_seekd_pos[0], last_seekd_pos[1]);
    switch (select_type) {
        case SEEK_GOP_START: {
            gop_start_time = gop_end_time;
            select_type = SEEK_GOP_END;
            seeked = 0;
            seek_cnt = 0;
            seek_step = 1.f;
            last_seekd_pos[0] = 0;
            last_seekd_pos[1] = 0;
            stuck_cnt = 0;
            seek_err = 0;
            break;
        }
        case SEEK_GOP_END: {
            if (seeked == 0) {
                float seektostart = 0.f;
                if ((sign == -1) && (demuxer->type == DEMUXER_TYPE_MPEG_TS)) {
                    seek_cnt++;
                    if (seek_cnt == 1) {
                        start_filepos = stream_tell(demuxer->stream);
                    } else {
                        if (start_filepos > get_mark_filepos(demuxer)) {
                            start_filepos = get_mark_filepos(demuxer);
                        }
                    }
                }

                seektostart = gop_start_time + (fabs(rate) * (1.f * seek_step / gst_select_fps)) * sign;
                trick->trick_finish = (int64_t) (seektostart * TIME_BASE) > 0 ? 0 : 1;
                if (trick->trick_finish) {
                    seektostart = 0.f;
                    if (p_info->slice_type == AV_PICTURE_TYPE_I) {
                        res = DEMUX_SEL_SAVE_AND_PUSH_PKT;
                    } else {
                        res = DEMUX_SEL_PUSH_SAVED_KEY_PKT;
                    }
                }

                if ((demuxer->stream->eof == 1 && sign == 1)
                    || ((start_filepos == demuxer->movi_start) && (sign == -1) && (demuxer->type == DEMUXER_TYPE_MPEG_TS))
                    || ((get_mark_filepos(demuxer) == demuxer->movi_end) && (sign == 1))) {
                    gop_end_time = curFrameTime;
                    select_type = SEEK_SELECT;
                    seek_cnt = 0;
                    start_filepos = 0;
                    if (demuxer->type == DEMUXER_TYPE_LAVF) {
                        if (demuxer->video->flags == 1) {
                            res = DEMUX_SEL_PUSH_CUR_END_PKT;
                        } else {
                            res = DEMUX_SEL_GET_NEXT_PKT;
                        }
                    } else {
                        res = DEMUX_SEL_PUSH_CUR_END_PKT;
                    }
                    break;
                }

                if (sign == -1) {
                    set_seek_stop_pos(demuxer, start_filepos);
                }
                MLOGA("seek to time %f start\n", seektostart);
                if (trick->trick_finish) {
                    demux_seek(demuxer, 0.0f, 0.0f, SEEK_ABSOLUTE);
                } else {
                    if ((int64_t) seektostart == (int64_t) curFrameTime) {
                        demux_seek(demuxer, seektostart, 0, SEEK_ABSOLUTE);
                    } else {
                        demux_seek(demuxer, seektostart - curFrameTime, 0, 0);
                    }
                }
                if (DEMUXER_TYPE_LAVF == demuxer->type) {
                    if (!ds_fill_buffer(demuxer->video)) {
                        break;
                    }
                }
                memset(&g_sc_info, 0, sizeof(sc_info_t));
                if (sign == -1) {
                    set_seek_stop_pos(demuxer, 0);
                }

                int64_t demux_seeked_pos = get_video_pkt_pos(demuxer, demuxer->video);
                MLOGA("seek to time %f step %f pos %lld finish:%d err:%d\n", seektostart, seek_step, demux_seeked_pos, trick->trick_finish, seek_err);
                if ( seek_err && (!(trick->trick_finish)) &&
                    (((demux_seeked_pos == last_seekd_pos[0]) && (last_seekd_pos[0] > 0)) ||
                     ((demux_seeked_pos == last_seekd_pos[1]) && (last_seekd_pos[1] > 0)))) {
                    stuck_cnt++;
                    float last_step = seek_step;
                    if (stuck_cnt > 8) {
                        seek_step += STEP_TIME * gst_select_fps / fabs(rate) * 50;    //skip 50s
                    } else if (stuck_cnt > 3) {
                        seek_step += STEP_TIME * gst_select_fps / fabs(rate) * 10;    //skip 10s
                    } else {
                        seek_step += STEP_TIME * 6;
                    }

                    if(vdec_type != DS_SEL_VDEC_TYPE_VP9) {
                        /* Step is nearby, so may searced the same positon */
                        check_backward_position(demuxer, curFrameTime, rate, gst_select_fps, last_step, &seek_step);
                    }
                    break;
                }

                last_seekd_pos[1] = last_seekd_pos[0];
                last_seekd_pos[0] = demux_seeked_pos;
                seeked = 1;
            } else {
                int seek_again = ((curFrameTime - gop_start_time)*sign > fabs(rate) * 1.f / gst_select_fps) ? 0 : 1;
                seek_again = seek_again && (!(trick->trick_finish));
                MLOGA("Diff:%f cur:%f flag:%d seek_again:%s stype:%d\n", (curFrameTime - gop_start_time),
                   curFrameTime, demuxer->video->flags, seek_again ? "TRUE" : "FALSE", p_info->slice_type);
                if ((!seek_again) &&
                    (get_seek_stop_flag(demuxer) == 0 || p_info->slice_type == AV_PICTURE_TYPE_I)) {
                    if (demuxer->type == DEMUXER_TYPE_LAVF) {
                        if (vdec_type == DS_SEL_VDEC_TYPE_MPEG12 || vdec_type == DS_SEL_VDEC_TYPE_H264 ||
                            vdec_type == DS_SEL_VDEC_TYPE_HEVC   || vdec_type == DS_SEL_VDEC_TYPE_AVS) {  //mpeg2,h.264, hevc,avs
                            if (p_info->slice_type != AV_PICTURE_TYPE_I) {
                                seek_err = 1;
                                break;
                            }
                        } else if (demuxer->video->flags == 0) {
                            seek_err = 1;
                            break;
                        }
                    }

                    if (p_info->slice_type == AV_PICTURE_TYPE_S_DATA) {
                        break;
                    }
                    seek_cnt = 0;
                    seek_err = 0;
                    start_filepos = 0;
                    trick->last_start_pts = gop_end_time;
                    gop_end_time = curFrameTime;
                    select_type = SEEK_SELECT;
                    last_selected_time = curFrameTime;
                    packet_cnt = 1;
                    seek_step = 1.f;
                    res = DEMUX_SEL_SAVE_AND_PUSH_PKT;
                    if (stuck_cnt > 3) {
                        gop_start_time = gop_end_time;
                    }
                    stuck_cnt = 0;
                } else {
                    float last_step = seek_step;
                    if (((curFrameTime - gop_start_time >= 0) || (get_seek_stop_flag(demuxer))) && (sign == -1)) {
                        seek_step += STEP_TIME * 4;
                    } else {
                        seek_step += STEP_TIME * 2;
                    }

                    if(vdec_type != DS_SEL_VDEC_TYPE_VP9) {
                        /* Step is nearby, so may searced the same positon */
                        check_backward_position(demuxer, curFrameTime, rate, gst_select_fps, last_step, &seek_step);
                    }
                }
                seeked = 0;
            }
            break;
        }
        case SEEK_SELECT: {
            if(vdec_type == DS_SEL_VDEC_TYPE_VP9) {
                if((fabs(rate) > gst_select_fps && seek_step < gst_select_fps) || (seek_step < gst_select_fps/fabs(rate))) {
                    res = DEMUX_SEL_PUSH_SAVED_KEY_PKT;
                    seek_step ++;
                    mtos_task_sleep(10);
                } else {
                    last_gop_start = gop_start_time;
                    gop_start_time = gop_end_time;
                    select_type = SEEK_GOP_END;
                    seeked = 0;
                    seek_step += STEP_TIME * gst_select_fps / fabs(rate) * 1;    //move 1s
                }
                break;
            }
            if ((((vdec_type == DS_SEL_VDEC_TYPE_H264 || vdec_type == DS_SEL_VDEC_TYPE_AVS) &&
                  (is_frame_complete(p_info, vdec_type) == 0)) ||
                 (vdec_type == DS_SEL_VDEC_TYPE_MPEG12 && p_info->slice_type == AV_PICTURE_TYPE_S_DATA)) &&
                (last_res == DEMUX_SEL_SAVE_AND_PUSH_PKT || last_res == DEMUX_SEL_MERGE_AND_PUSH_PKT)) { //jqw@20190709 for bug111053
                res = DEMUX_SEL_MERGE_AND_PUSH_PKT;

                last_selected_time = curFrameTime;
                break;
            }
            /* should nout push other pkt because pts increase */
            if(need_more_backward_packet(demuxer, curFrameTime, gop_end_time, rate)) {
                MLOGA("cur:%fs gop_end:%fs gop_start:%fs gst_select_fps = %f pkt:%d\n",curFrameTime,gop_end_time,gop_start_time,gst_select_fps,packet_cnt);
            } else if (curFrameTime - gop_end_time > fabs((gop_end_time - gop_start_time) * 1.f / rate)
                || packet_cnt >= (int)(gst_select_fps * fabs((gop_end_time - gop_start_time) / rate))) {
                last_gop_start = gop_start_time;
                gop_start_time = gop_end_time;
                select_type = SEEK_GOP_END;
                seeked = 0;

                break;
            }

            res = DEMUX_SEL_PUSH_SAVED_KEY_PKT;
            if (fabs(last_selected_time - curFrameTime) > 0.001f) {
                packet_cnt++;
            }
            break;
        }
        case SEEK_SELECT_END: {
            res = DEMUX_SEL_PUSH_CUR_END_PKT;
            break;
        }
    }
    last_res = res;
    MLOGA("res:%d\n", res);
    return res;
}

static int gst_select_init(demuxer_t *demuxer, float curFrameTime, int speed)
{
    struct demuxer_trick *trick = &demuxer->trick;

    gop_start_time = curFrameTime;
    gop_end_time   = curFrameTime;

    FILE_SEQ_T *p_file_seq = (FILE_SEQ_T *)x_get_cur_instance();
    MT_UNF_AVPLAY_STREAM_INFO_S streamInfo;

    if (vdec_get_stream_info(p_file_seq->p_vdec_dev, &streamInfo) != MT_SUCCESS) {
        gst_select_fps = get_fp_fps();
    } else {
        gst_select_fps = (float)streamInfo.stVidStreamInfo.u32fpsInteger;
    }

    if (0 == (int) (gst_select_fps * 1000)) {
        gst_select_fps = get_fp_fps();
        MLOGE("Vdec report frame rate error, use:%ffps\n", gst_select_fps);
    }

    last_gop_start = 0.f;
    select_type = SEEK_GOP_START;//jqw@20181015,for bug 105243//SEEK_GOP_END;
    g_file_end = 0;
    seek_step = 1.f;

    //jqw@20180928 for trickplay
    packet_cnt = 0;

    i_start_time = 0.f;
    drop_cnt = 0;
    last_packet_cnt = 1;
    last_drop_cnt = 0;

    trick->speed        = speed;
    trick->trick_cnt    = 0;
    trick->start_time   = (mt_s64) mclock_get_utime() / TIME_BASE;
    trick->trick_finish = 0;
    trick->start_pts  = (int) (curFrameTime * TIME_BASE);

    return 0;
}


u32 is_avi_stream(demuxer_t *demuxer)
{
    u32 ret = 0;
    if (demuxer->type == DEMUXER_TYPE_LAVF) {
        unsigned long *priv = (unsigned long *)(demuxer->priv);
        AVInputFormat *avif = (AVInputFormat *)(*priv);
        if (strstr(avif->name, "avi")) {
            ret = 1;
        }
    }
    if (demuxer->type == DEMUXER_TYPE_AVI) {
        ret = 1;
    }
    return ret;
}

sub_data *subdata = NULL;
subtitle *vo_sub = NULL;
subtitle *sub_temp = NULL;
static int sub_flag = 0;
int bsf_vcodec_flag = -1;

static int IS_SUB(es_stream_type_t type)
{
    int v = type;
    switch (v) {
        case SPU_DVD_MP:
        case SPU_DVB_MP:
        case SPU_PGS_MP:
        case SPU_TELETEXT_MP:
            return 1;
    }
    return 0;
}
static int SUB2STR(es_stream_type_t type, char *codec)
{
    switch (type) {
        case SPU_DVD_MP:
            memcpy(codec, "SPU_DVD", strlen("SPU_DVD"));
            break;
        case SPU_DVB_MP:
            memcpy(codec, "SPU_DVB", strlen("SPU_DVB"));
            break;
        case SPU_PGS_MP:
            memcpy(codec, "SPU_PGS", strlen("SPU_PGS"));
            break;
        case SPU_TELETEXT_MP:
            memcpy(codec, "SPU_TTX", strlen("SPU_TTX"));
            break;
        default:
            break;
    }
    return 0;
}

ts_info_t *ds_ts_prog(demuxer_t *demuxer)
{
    ts_priv_t *priv = (ts_priv_t *)demuxer->priv;
    int k = 0;
    int g = 0;
    pmt_t *pmt1;
    ts_info_t *priv_res = NULL;
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();

    priv->selected_program_idx = 0;
    priv_res = (ts_info_t *)malloc33(sizeof(ts_info_t));
    if (NULL == priv_res) {
        return NULL;
    }
    memset(priv_res, 0, sizeof(ts_info_t));
    priv_res->pmt_cnt = priv->pmt_cnt;
    MLOGI("pmt cnt %d selected apid:%d vpid:%d\n",
        priv_res->pmt_cnt, priv->selected_apid, priv->selected_vpid);
    priv_res->p_pmt = (pmt_ts_t *)malloc33(sizeof(pmt_ts_t) * ((u32)priv->pmt_cnt + 1));
    if (NULL == priv_res->p_pmt) {
        free33(priv_res);
        return NULL;
    }

    memset(priv_res->p_pmt, 0, sizeof(pmt_ts_t) * ((u32)priv->pmt_cnt + 1));
    for (k = 0; k < priv->pmt_cnt; k++) {
        pmt1 = (pmt_t *)(&priv->pmt[k]);
        priv_res->p_pmt[k].progid = pmt1->progid;
        priv_res->p_pmt[k].pcr_pid = pmt1->PCR_PID;
        priv_res->p_pmt[k].es_cnt = pmt1->es_cnt;
        MLOGD("prog id %d, es_cnt %d\n", priv_res->p_pmt[k].progid, priv_res->p_pmt[k].es_cnt);
        demuxer->subt_info.cnt = 0;
        for (g = 0; g < pmt1->es_cnt; g++) {

            priv_res->p_pmt[k].es[g].pid = pmt1->es[g].pid;
            if ((priv->selected_vpid != -1) && (pmt1->es[g].pid == priv->selected_vpid)) {
                priv->selected_program_idx = k;
            }

            priv_res->p_pmt[k].es[g].type = (int)pmt1->es[g].type;
            if (IS_SUB(pmt1->es[g].type)) {

                demuxer->subt_info.subtitle[demuxer->subt_info.cnt].id = priv_res->p_pmt[k].es[g].pid;
                SUB2STR(pmt1->es[g].type, demuxer->subt_info.subtitle[demuxer->subt_info.cnt].code);

                unsigned int lang_size = MIN(
                    (unsigned int) sizeof(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].lang), (unsigned int) sizeof(pmt1->es[g].lang));
                memcpy(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].lang, pmt1->es[g].lang, lang_size);
                demuxer->subt_info.cnt++;
            }
            priv_res->p_pmt[k].es[g].audio_eac3_flag = pmt1->es[g].audio_eac3_flag;
            strcpy(priv_res->p_pmt[k].es[g].lang, (char *)pmt1->es[g].lang);
            MLOGD("pid %d  pid type %x lang: %s,eac3[%d]\n", priv_res->p_pmt[k].es[g].pid,
                priv_res->p_pmt[k].es[g].type, priv_res->p_pmt[k].es[g].lang, pmt1->es[g].audio_eac3_flag);
        }
    }

    set_ts_prog(demuxer);
    p_file_seq->audio_pid = priv->selected_apid;
    return priv_res;
}

int ds_ts_check_audio_aid(demuxer_t *demuxer, int apid)
{
    ts_priv_t *priv = demuxer->priv;
    pmt_t   *pmt = (pmt_t *)(&priv->pmt[priv->selected_program_idx]);
    int i = 0;
    int ret = 0;

    if (NULL == pmt || demuxer->audio->sh == NULL) {
        return 0;
    }
    for (i = 0; i < pmt->es_cnt; i++) {
        if (pmt->es[i].pid == apid) {
            ret = 1;
            break;
        }

    }
    return ret;
}

AVFormatContext *p_lavf_avfc = NULL;
void wma_get_dec_info(demuxer_t *demuxer, void *param)
{
    int audioid, i;
    demux_stream_t *audio = NULL;
    sh_audio_t *sh_audio = NULL;
    WMA_FORMAT_S *p_wma_param = (WMA_FORMAT_S *) param;

    MLOGI("%s start \n", __func__);
    if (demuxer->type == DEMUXER_TYPE_LAVF) {
        unsigned long  *priv = (unsigned long *)(demuxer->priv);
        AVInputFormat *avif = (AVInputFormat *)(*priv);

        AVStream *st = NULL;
        AVFormatContext *avfc = p_lavf_avfc;

        if (avfc == NULL) {
            MLOGI("%s avfc is null \n", __func__);
            return;
        }

        if (strstr(avif->name, "asf") || strstr(avif->name, "avi")||strstr(avif->name, "matroska,webm")) {
            audioid = demuxer->audio->id;
            if (audioid >= avfc->nb_streams || audioid < 0) {
                MLOGI("%s error audioid %d nb_stream %d \n", __func__, audioid, avfc->nb_streams);
                return;
            }

            st = avfc->streams[audioid];
            AVCodecContext *codec = st->codec;
            if (codec == NULL) {
                MLOGI("%s codec is null \n", __func__);
                return;
            }
            p_wma_param->nChannels        = codec->channels;
            p_wma_param->nSamplesPerSec   = (mt_u32)codec->sample_rate;
            p_wma_param->nBlockAlign      = codec->block_align;
            p_wma_param->nAvgBytesPerSec  = (mt_u32)codec->bit_rate;
            p_wma_param->wBitsPerSample   =  codec->bits_per_coded_sample;

            audio = demuxer->audio;
            if (audio) {
                sh_audio = audio->sh;
            }
            if (sh_audio) {
                if (sh_audio->format == 0x161) {
                    p_wma_param->wFormatTag = WMA_CODEC_ID_WMAV2;
                } else if (sh_audio->format == 0x160) {
                    p_wma_param->wFormatTag = WMA_CODEC_ID_WMAV1;
                }
            }
            p_wma_param->cbSize = codec->extradata_size;
            if (p_wma_param->cbSize > 16) {
                p_wma_param->cbSize = 16;
            }
            for (i = 0; i <  p_wma_param->cbSize; i++) {
                p_wma_param->cbExtWord[i] = codec->extradata[i];
            }
        }
    }
}

void set_ts_pmt_avs_info(ts_info_t *p_ts_priv)
{
    ts_info_t *ts_priv = p_ts_priv;
    pmt_ts_t *pmt;
    int i, j;
    MLOGI("[%s] start start!!!!!\n", __func__);
    MLOGI("[%s] p_ts_priv = 0x%lx\n", __func__, (ulong)p_ts_priv);

    for (i = 0; i < ts_priv->pmt_cnt; i++) {
        pmt = ts_priv->p_pmt + i;
        pmt->aud_stream_cnt = 0;
        pmt->vid_stream_cnt = 0;
        pmt->sub_title_cnt = 0;
        MLOGD("[%s] ---------for1 i=%d,pmtcnt[%d],escnt[%d]\n", __func__, i, ts_priv->pmt_cnt, pmt->es_cnt);

        for (j = 0; j < pmt->es_cnt; j++) {
            MLOGD("[%s] ---------for2 j=%d,es_cnt[%d],es_type[0x%x],pid[%d]\n", __func__, j, pmt->es_cnt, pmt->es[j].type, pmt->es[j].pid);
            switch (pmt->es[j].type) {
                case VIDEO_MPEG1_MP:
                case VIDEO_MPEG2_MP:
                case VIDEO_H264_MP:
                    pmt->vid_stream[pmt->vid_stream_cnt].type = pmt->es[j].type;
                    pmt->vid_stream[pmt->vid_stream_cnt].pid = pmt->es[j].pid;
                    strcpy(pmt->vid_stream[pmt->vid_stream_cnt].lang, pmt->es[j].lang);
                    pmt->vid_stream_cnt++;
                    break;
                case AUDIO_MP2_MP:
                case AUDIO_A52_MP:
                case AUDIO_AV3A_MP:
                case AUDIO_AAC_MP:
                case AUDIO_AAC_LATM_MP:
                case AUDIO_LPCM_BE_MP:
                case AUDIO_PCM_BR_MP:
                case AUDIO_DTS_MP:
                case AUDIO_TRUEHD_MP:
                case AUDIO_TRUEHD_AC3_MP:
                    pmt->aud_stream[pmt->aud_stream_cnt].type = pmt->es[j].type;
                    pmt->aud_stream[pmt->aud_stream_cnt].pid = pmt->es[j].pid;
                    strcpy(pmt->aud_stream[pmt->aud_stream_cnt].lang, pmt->es[j].lang);
                    pmt->aud_stream_cnt++;
                    break;
                case SPU_DVD_MP:
                case SPU_DVB_MP:
                case SPU_TELETEXT_MP:
                case SPU_PGS_MP:
                    pmt->sub_title[pmt->sub_title_cnt].type = pmt->es[j].type;
                    pmt->sub_title[pmt->sub_title_cnt].pid = pmt->es[j].pid;
                    strcpy(pmt->sub_title[pmt->sub_title_cnt].lang, pmt->es[j].lang);
                    pmt->sub_title_cnt++;
                    break;
                default:
                    break;
            }
        }
    }

    MLOGI("[%s] end end!!!!!\n", __func__);
}

int ts_get_audio_track_pid(
    demuxer_t *demuxer, ts_info_t *p_ts_priv, int id)
{
    ts_info_t *ts_priv = p_ts_priv;
    ts_priv_t *priv    = demuxer->priv;
    pmt_ts_t  *pmt = (pmt_ts_t *)(&ts_priv->p_pmt[priv->selected_program_idx]);

    if (!pmt->aud_stream_cnt) {
        return 0;
    }

    if (id > pmt->aud_stream_cnt - 1 ||
        id > MAX_DEMUX_MP_STREAM_CNT - 1) {
        return pmt->aud_stream[0].pid;
    }

    return pmt->aud_stream[id].pid;
}

static int ts_get_video_track_pid(
    demuxer_t *demuxer, ts_info_t *p_ts_priv, int id)
{
    ts_info_t *ts_priv = p_ts_priv;
    ts_priv_t *priv    = demuxer->priv;
    pmt_ts_t  *pmt = (pmt_ts_t *)(&ts_priv->p_pmt[priv->selected_program_idx]);

    if (!pmt->vid_stream_cnt) {
        return 0;
    }

    if (id > pmt->vid_stream_cnt - 1 ||
        id > MAX_DEMUX_MP_STREAM_CNT - 1) {
        return pmt->vid_stream[0].pid;
    }

    return pmt->vid_stream[id].pid;
}

int ds_get_audio_pid(demux_stream_t *ds,
    ts_info_t *ds_ts_priv, const int id_num, int *pid_list)
{
    int idx;
    if (NULL == ds || NULL == pid_list ||
        NB_PID_MAX < id_num || NULL == ds->demuxer || NULL == ds_ts_priv) {
        MLOGE("%s para error,ds:%p list:%p id:%d priv:%d\n",
            __FUNCTION__, ds, pid_list, id_num, ds_ts_priv);
        return MT_FAILURE;
    }

    for (idx = 0; idx < MIN(id_num, MAX_DEMUX_MP_STREAM_CNT); idx++) {
        pid_list[idx] = ts_get_audio_track_pid(ds->demuxer, ds_ts_priv, idx);
    }
    return MT_SUCCESS;
}

int ds_get_video_pid(demux_stream_t *ds,
    ts_info_t *ds_ts_priv, const int id_num, int *pid_list)
{
    int idx;

    if (NULL == ds || NULL == pid_list ||
        NB_PID_MAX < id_num || NULL == ds->demuxer || NULL == ds_ts_priv) {
        MLOGE("%s para error,ds:%p list:%p id:%d priv:%d\n",
            __FUNCTION__, ds, pid_list, id_num, ds_ts_priv);
        return MT_FAILURE;
    }
    for (idx = 0; idx < MIN(id_num, MAX_DEMUX_MP_STREAM_CNT); idx++) {
        pid_list[idx] = ts_get_video_track_pid(ds->demuxer, ds_ts_priv, idx);
    }
    return MT_SUCCESS;
}

int ds_get_network_bitrate(
    demux_stream_t *ds, long long *bitrate)
{
    if (!ds || !bitrate || !ds->demuxer) {
        return MT_FAILURE;
    }

    int ret = demux_control(ds->demuxer, DEMUXER_CTRL_GET_DL_BITRATE, (void *) bitrate);
    if (DEMUXER_CTRL_OK == ret) {
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}

demuxer_t *demux_mp_open(stream_t *vs, int file_format, int audio_id_mp,
                         int video_id_mp, int dvdsub_id_mp, char *filename)
{
    demuxer_t *res;
    MLOGI("[%s] -----file_format=%d\n", __func__, file_format);
    res = demux_open(vs, file_format, -1, -1, -1, filename);
    MLOGD("[%s] -----res=%x\n", __func__, (unsigned long)res);

    //if use_ext_heap is TRUE, packet_I_bak_ptr packet_I_bak_ptr2 memory is reset!
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    if (p_file_seq->use_ext_heap == FALSE) {
        if (packet_I_bak_ptr) {
            free33(packet_I_bak_ptr);
        }
        if (packet_I_bak_ptr2) {
            free33(packet_I_bak_ptr2);
        }
    }
    packet_I_bak_ptr = NULL;
    packet_I_bak_ptr2 = NULL;

    if (res) {
        MLOGD("[%s] -----res->type=%d, res->file_format=%d\n", __func__, res->type, res->file_format);
        if (DEMUXER_TYPE_MPEG_TS == res->type) {
            return res;
        }
    }
    return res;
}

int ds_get_audio_bps(demux_stream_t *ds)
{
    sh_audio_t *sh_audio = ds->sh;

    if (NULL == sh_audio) {
        return 0;
    }

    return sh_audio->i_bps;
}

int ds_get_video_bps(demux_stream_t *ds)
{
    sh_video_t *sh_video = ds->sh;
    double duration = 0;
    uint64_t file_size = 0;
#ifdef __LINUX__
    struct stat stat_buf;
#endif

    if (NULL == sh_video) {
        return 0;
    }

    if (!sh_video->i_bps) {
        demux_control(ds->demuxer, DEMUXER_CTRL_GET_TIME_LENGTH, (void *)&duration);
#ifndef __LINUX__
        file_size = ((ufs_file_t *)(ds->demuxer->stream->fd))->file_size;
#else
        stat(ds->demuxer->filename, &stat_buf);
        file_size = (uint64_t)stat_buf.st_size;
#endif
        sh_video->i_bps = file_size / duration;
    }

    return sh_video->i_bps;
}

void ds_get_vfilter_type(demux_stream_t *ds)
{
    sh_video_t *sh_video = ds->sh;
    unsigned long long biComp = 0;
    bsf_vcodec_flag = -1;

    if (DEMUXER_TYPE_LAVF == ds->demuxer->type && sh_video) {
        if (sh_video->bih) {
            biComp = le2me_32(sh_video->bih->biCompression);

            if ((strstr(((char *)&biComp), "mpg2")) || (strstr(((char *)&biComp), "mpg1"))) {
                bsf_vcodec_flag = 0;
            } else if (sh_video->bih->biCompression == 1) {
                bsf_vcodec_flag = 0;
            } else if (strstr(((char *)&biComp), "MPG2")) {
                bsf_vcodec_flag = 0;
            } else if ((strstr(((char *)&biComp), "H264")) || (strstr(((char *)&biComp), "avc1")) || (strstr(((char *)&biComp), "h264"))) {
                bsf_vcodec_flag = 1;
            } else if ((strstr(((char *)&biComp), "WVC1")) || (strstr(((char *)&biComp), "WMV"))) {
                bsf_vcodec_flag = 4;
            } else if ((strstr(((char *)&biComp), "MP4V")) || (strstr(((char *)&biComp), "XVID")) ||
                       (strstr(((char *) &biComp), "xvid")) ||
                       (strstr(((char *)&biComp), "DIV")) || (strstr(((char *)&biComp), "DX"))) {
                bsf_vcodec_flag = 3;
            } else if ((strstr(((char *) &biComp), "H265")) || (strstr(((char *) &biComp), "hevc"))
                       || (strstr(((char *) &biComp), "h265")) || (strstr(((char *) &biComp), "HEVC")) || (strstr(((char *) &biComp), "hvc1")) || (strstr(((char *) &biComp), "hev1"))) {

                bsf_vcodec_flag = 5;
            }
        }
    }
}

int ds_is_codec_h264(unsigned int id)
{
    return id == vVIDEO_H264;
}

static int is_old_wmv3(sh_video_t *sh_video)
{
    int ret = 0;
    MLOGI("%s sh->codec_extradata_size %d\n", __func__, sh_video->codec_extradata_size);
    if (sh_video->codec_extradata_size > 0) {
        uint8_t *start = sh_video->codec_extradata;
        uint8_t *end = sh_video->codec_extradata + sh_video->codec_extradata_size;
        uint8_t *next;
        int Res_SM ;
        int ResRTM;

        for (next = start; next < end; next++) {
            // put_bits(*next, 8);
        }
        Res_SM = (*start) >> 4 & (0x3);
        ResRTM = (*(start + 3)) & (0x1);
        if ((Res_SM != 0) || (ResRTM != 1)) {
            ret = 1;
        }
    }
    return ret;
}

extern unsigned int drv_adp_video_codec_id;
void ds_get_video_codec_type(demux_stream_t *ds, int *p_video_type, int *vpid, int *pcr_pid)
{
    sh_video_t *sh_video = ds->sh;
    if(sh_video == NULL) {
        return;
    }

    unsigned long long biComp = 0;
    int i;

    sh_video->codec_id = (unsigned int) -1;

    *p_video_type = MT_UNF_VCODEC_TYPE_BUTT;
    if (DEMUXER_TYPE_MPEG_TS == ds->demuxer->type) {
        ts_priv_t *priv = (ts_priv_t *)ds->demuxer->priv;
        pmt_t *pmt;

        pmt = (pmt_t *)(&priv->pmt[priv->selected_program_idx]);
        if (pmt == NULL) {
            *p_video_type = 0xffff;
            switch (sh_video->format) {
                case VIDEO_MPEG1_MP:
                    *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                    sh_video->codec_id = vVIDEO_MPEG1;
                    break;
                case VIDEO_MPEG2_MP:
                    *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                    sh_video->codec_id = vVIDEO_MPEG2;
                    break;
                case VIDEO_MPEG4_MP:
                    *p_video_type = MT_UNF_VCODEC_TYPE_MPEG4;
                    sh_video->codec_id = vVIDEO_MPEG4;
                    break;
                case VIDEO_H264_MP:
                    *p_video_type = MT_UNF_VCODEC_TYPE_H264;
                    sh_video->codec_id = vVIDEO_H264;
                    break;
                case VIDEO_VC1_MP:
                    *p_video_type = MT_UNF_VCODEC_TYPE_VC1;
                    sh_video->codec_id = vVIDEO_VC1;
                    break;
                case VIDEO_AVS_MP:
                    *p_video_type = MT_UNF_VCODEC_TYPE_AVS;
                    sh_video->codec_id = vVIDEO_AVS;
                    break;
                case VIDEO_HEVC_MP:
                    *p_video_type = MT_UNF_VCODEC_TYPE_HEVC;
                    sh_video->codec_id = vVIDEO_HEVC;
                    break;
                default:
                    MLOGI("%s %d\n", __func__, __LINE__);
                    break;
            }

            return;
        }

        for (i = 0; i < pmt->es_cnt; i++) {
            if ((pmt->es[i].type == VIDEO_MPEG1_MP)) {
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                *vpid = pmt->es[i].pid;
                *pcr_pid = pmt->PCR_PID;
                sh_video->codec_id = vVIDEO_MPEG1;
                break;
            } else if ((pmt->es[i].type == VIDEO_MPEG2_MP)) {
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                *vpid = pmt->es[i].pid;
                *pcr_pid = pmt->PCR_PID;
                sh_video->codec_id = vVIDEO_MPEG2;
                break;
            } else if ((pmt->es[i].type == VIDEO_H264_MP)) {
                *p_video_type = MT_UNF_VCODEC_TYPE_H264;
                *vpid = pmt->es[i].pid;
                *pcr_pid = pmt->PCR_PID;
                sh_video->codec_id = vVIDEO_H264;
                break;
            } else if (pmt->es[i].type == VIDEO_MPEG4_MP) {
                *pcr_pid = pmt->PCR_PID;
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG4;
                sh_video->codec_id = vVIDEO_MPEG4;
                break;
            } else if (pmt->es[i].type == VIDEO_VC1_MP) {
                *pcr_pid = pmt->PCR_PID;
                *p_video_type = MT_UNF_VCODEC_TYPE_VC1;
                sh_video->codec_id = vVIDEO_VC1;
                break;
            } else if ((pmt->es[i].type == VIDEO_AVS_MP)) {
                *p_video_type = MT_UNF_VCODEC_TYPE_AVS;  //avs codec
                *vpid = pmt->es[i].pid;
                *pcr_pid = pmt->PCR_PID;
                sh_video->codec_id = vVIDEO_AVS;
                break;
            } else if ((pmt->es[i].type == VIDEO_AVS2_MP)) {
                *p_video_type = MT_UNF_VCODEC_TYPE_AVS2;  //avs2 codec
                *vpid = pmt->es[i].pid;
                *pcr_pid = pmt->PCR_PID;
                sh_video->codec_id = vVIDEO_AVS;
                break;
            } else if ((pmt->es[i].type == VIDEO_HEVC_MP)) {
                *p_video_type = MT_UNF_VCODEC_TYPE_HEVC;
                *vpid = pmt->es[i].pid;
                *pcr_pid = pmt->PCR_PID;
                sh_video->codec_id = vVIDEO_HEVC;
                break;
            } else {
                *pcr_pid = pmt->PCR_PID;
                *p_video_type = 0xffff;
                sh_video->codec_id = (unsigned int)vUNKNOWN;

            }
        }
    } else if (DEMUXER_TYPE_MPEG_PS == ds->demuxer->type) {
        *p_video_type = 0;
        sh_video->codec_id = vVIDEO_MPEG2;
        switch (sh_video->format) {
            case VIDEO_MPEG1_MP:
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                sh_video->codec_id = vVIDEO_MPEG1;
                break;
            case VIDEO_MPEG2_MP:
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                sh_video->codec_id = vVIDEO_MPEG2;
                break;
            case VIDEO_MPEG4_MP:
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG4;
                sh_video->codec_id = vVIDEO_MPEG4;
                break;
            case VIDEO_H264_MP:
                *p_video_type = MT_UNF_VCODEC_TYPE_H264;
                sh_video->codec_id = vVIDEO_H264;
                break;
            case VIDEO_VC1_MP:
                *p_video_type = MT_UNF_VCODEC_TYPE_VC1;
                sh_video->codec_id = vVIDEO_VC1;
                break;
            case VIDEO_AVS_MP:
                *p_video_type = MT_UNF_VCODEC_TYPE_AVS;
                sh_video->codec_id = vVIDEO_AVS;
                break;
#if defined(SYMPHONY_UCOS)
            case VIDEO_HEVC_MP:
                *p_video_type = MT_UNF_VCODEC_TYPE_HEVC;
                sh_video->codec_id = vVIDEO_HEVC;
                break;
#endif
            default:
                break;
        }
    } else if (DEMUXER_TYPE_MPEG4_ES == ds->demuxer->type) {
        *p_video_type = MT_UNF_VCODEC_TYPE_MPEG4;
        sh_video->codec_id = vVIDEO_MPEG4;

    } else if (DEMUXER_TYPE_RTP == ds->demuxer->type) {
        if (sh_video->bih) {
            biComp = le2me_32(sh_video->bih->biCompression);
        }

        char *p_str = (char *)(&biComp);
        if (sh_video->bih) {
            MLOGI("[%s] p_str:%s,sh_video->bih->biCompression=%d\n", __func__, p_str, sh_video->bih->biCompression);
        }

        sh_video->codec_id = vVIDEO_MPEG2;
        if ((strstr(((char *)&biComp), "mpg2")) || (strstr(((char *)&biComp), "mpg1"))) {
            *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
        } else if (sh_video->bih && sh_video->bih->biCompression == 1) {        // for tsscan
            *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
        } else if ((strstr(((char *)&biComp), "MPG2")) || (strstr(((char *)&biComp), "MPG1"))) {
            *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
        } else if (strstr(((char *)&biComp), "H264")) {
            *p_video_type = MT_UNF_VCODEC_TYPE_H264;
            sh_video->codec_id = vVIDEO_H264;
#if defined(ENABLE_DEMUX_RTSP)
        } else if (demux_is_mpeg_rtp_stream(ds->demuxer)) {
            *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
            MLOGI("[%s]-------demux_is_mpeg_rtp_stream!\n", __func__);
#endif
        } else {
            MLOGI("ERROR not support %.4s !!!!!!!!!!!\n", (char *)&biComp);
            *p_video_type = 0xffff;
            sh_video->codec_id = (unsigned int)vUNKNOWN;
        }
    } else {
        if (sh_video->bih) {
            biComp = le2me_32(sh_video->bih->biCompression);
            //p_str  = (char *)(&biComp);

            if ((strstr(((char *)&biComp), "mpg2")) || (strstr(((char *)&biComp), "mpg1"))) {
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                sh_video->codec_id = vVIDEO_MPEG2;
            } else if (sh_video->bih->biCompression == 1) {
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                sh_video->codec_id = vVIDEO_MPEG1;
            } else if ((strstr(((char *)&biComp), "MPG2")) || (strstr(((char *)&biComp), "MPG1"))) {
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG2;
                sh_video->codec_id = vVIDEO_MPEG2;
            } else if ((strstr(((char *)&biComp), "H264")) ||
                (strstr(((char *)&biComp), "avc1"))        ||
                (strstr(((char *)&biComp), "h264"))) {
                *p_video_type = MT_UNF_VCODEC_TYPE_H264;
                sh_video->codec_id = vVIDEO_H264;
            } else if ((strstr(((char *) &biComp), "H265")) ||
                (strstr(((char *) &biComp), "hevc"))        ||
                (strstr(((char *) &biComp), "h265"))        ||
                (strstr(((char *) &biComp), "HEVC"))        ||
                (strstr(((char *) &biComp), "hvc1"))        ||
                (strstr(((char *) &biComp), "hev1"))) {
                *p_video_type = MT_UNF_VCODEC_TYPE_HEVC;
                sh_video->codec_id = vVIDEO_HEVC;
            } else if (strstr(((char *)&biComp), "RV30")) {
                *p_video_type = MT_UNF_VCODEC_TYPE_REAL8;
                sh_video->codec_id = vVIDEO_RV30;
            } else if (strstr(((char *)&biComp), "RV40")) {
                *p_video_type = MT_UNF_VCODEC_TYPE_REAL9;
                sh_video->codec_id = vVIDEO_RV40;
            } else if (strstr(((char *)&biComp), "VP80") || strstr(((char *)&biComp), "vp08")) {
                *p_video_type = MT_UNF_VCODEC_TYPE_VP8;
                sh_video->codec_id = vVIDEO_VP8;
            } else if (strstr(((char *) &biComp), "VP90")|| strstr(((char *)&biComp), "vp09")) {
                *p_video_type = MT_UNF_VCODEC_TYPE_VP9;
                sh_video->codec_id = vVIDEO_VP9;
            } else if ((strstr(((char *)&biComp), "WVC1")) ||
                (strstr(((char *) &biComp), "WMV3"))       ||
                (strstr(((char *) &biComp), "VC-1"))       ||
                (strstr(((char *) &biComp), "vc-1"))) {
                *p_video_type = MT_UNF_VCODEC_TYPE_VC1;
                sh_video->codec_id = vVIDEO_VC1;
                if ((strstr(((char *) &biComp), "WMV3"))) {
                    sh_video->codec_id = vVIDEO_VC1_WMV3;
                    MLOGI("%s WMV3\n", __func__);
                    if (is_old_wmv3(sh_video) != 1) {
                        MLOGI("%s old_wmv3\n", __func__);
                        sh_video->codec_id = vVIDEO_VC1SMP5;
                    }
                }
            } else if ((strstr(((char *)&biComp), "MP4V")) ||
                (strstr(((char *)&biComp), "XVID"))        ||
                (strstr(((char *) &biComp), "xvid"))       ||
                (strstr(((char *)&biComp), "DIV") && !strstr(((char *)&biComp), "DIV3")) ||
                (strstr(((char *)&biComp), "DX"))          ||
                (strstr(((char *)&biComp), "263"))         ||
                (strstr(((char *) &biComp), "FMP4"))) {
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG4;
                sh_video->codec_id = vVIDEO_MPEG4;
                *p_video_type = MT_UNF_VCODEC_TYPE_MPEG4;
            } else {
                *p_video_type = 0xffff;
                sh_video->codec_id = (unsigned int)vUNKNOWN;
                MLOGI("%s %d:ERROR may unsupport %.4s !!!!!!!!!!!\n", __func__, __LINE__, (char *)&biComp);
            }
        } else {
            MLOGI("ERROR may unsupport %.4s !!!!!!!!!!!\n", (char *)&biComp);
            *p_video_type = 0xffff;
            sh_video->codec_id = (unsigned int)vUNKNOWN;
        }
    }
    if (DEMUXER_TYPE_MPEG_PS == ds->demuxer->type) {
        if ((sh_video->i_bps > 8000000 || sh_video->i_bps == 0)) {
            *p_video_type = 0xffff;
            MLOGI("ERROR not support mpegps !bps == %d\n", sh_video->i_bps);
        }
    }

	/* other MJPG*/
    if (strstr(((char *)&biComp), "MJPG")) {
        *p_video_type = MT_UNF_VCODEC_TYPE_MJPEG;
        sh_video->codec_id = (unsigned int)vVIDEO_MJPEG;
    }
    MLOGI("[%s] biComp:%.4s,id[%x]\n", __func__, (char *)&biComp, sh_video->codec_id);

    drv_adp_video_codec_id = sh_video->codec_id;
}

static int ds_get_audio_common_codec_info(demux_stream_t *ds, unsigned int *p_audio_type,
    int *pcm_be, unsigned int *acodec_id, int *apid, AUDIO_OUT_MODE aout_mode)
{
    sh_audio_t *sh_audio = ds->sh;
    if (NULL == sh_audio) {
        return MT_FAILURE;
    }

    int audio_pcm_be = 0;
    int audio_eac3_flag  = 0;
    unsigned int audio_codec_id = (unsigned int) -1;
    unsigned int audio_cocec_type = HA_AUDIO_ID_INVALID;

    if (sh_audio->format == (int)MKTAG('M', 'P', '4', 'A') ||
        sh_audio->format == MKTAG('M', 'P', '4', 'A')      ||
        sh_audio->format == MKTAG('m', 'p', '4', 'l')      ||
        sh_audio->format == MKTAG('M', 'P', '4', 'L')) {
        audio_cocec_type = HA_AUDIO_ID_AAC; //6;
        audio_codec_id = aAUDIO_AAC;
    } else if (sh_audio->format == 0x2000 || sh_audio->format == MKTAG('E', 'A', 'C', '3')) {
        if (sh_audio->format == MKTAG('E', 'A', 'C', '3')) {
            audio_eac3_flag = 1;
        }
        if (aout_mode == AUDIO_LPCM) {
            if (audio_eac3_flag) {
                audio_cocec_type = HA_AUDIO_ID_DOLBY_PLUS;
            } else {
                audio_cocec_type = HA_AUDIO_ID_DOLBY_TRUEHD;
            }
        } else {
            if (audio_eac3_flag) {
                audio_cocec_type = HA_AUDIO_ID_EAC3PASSTHROUGH;
            } else {
                audio_cocec_type = HA_AUDIO_ID_AC3PASSTHROUGH;
            }
        }
        audio_codec_id = aAUDIO_AC3;
    } else if (MKTAG('a', 'c', '-', '4') == sh_audio->format ||
               MKTAG('A', 'C', '-', '4') == sh_audio->format) {
        audio_cocec_type = HA_AUDIO_ID_DOLBY_AC4;
        audio_codec_id = aAUDIO_AC4;
    } else if (MKTAG('a', 'v', '3', 'a') == sh_audio->format) {
        audio_cocec_type = HA_AUDIO_ID_VVID;
        audio_codec_id = aAUDIO_AV3A;
    } else if (sh_audio->format == 0x55 || sh_audio->format == 0x50) {
        audio_cocec_type = HA_AUDIO_ID_MP3; //MP3 MP2
        if (sh_audio->format == 0x50) {
            audio_codec_id = aAUDIO_MP2;
        } else {
            audio_codec_id = aAUDIO_MP3;
        }
    } else if (sh_audio->format == AUDIO_DTS_MP) { //CODEC_ID_DTS
        audio_cocec_type = HA_AUDIO_ID_DTSHD; // HA_AUDIO_ID_DTSPASSTHROUGH;
        audio_codec_id = aAUDIO_DTS;
    } else if (sh_audio->format == MKTAG('B', 'P', 'C', 'M')) { //CODEC_ID_PCM
        audio_cocec_type = HA_AUDIO_ID_PCM;
        audio_codec_id = aAUDIO_PCM_BR;
        audio_pcm_be = 1;
    } else if (sh_audio->format == AUDIO_LPCM_BE_MP) { //0x10001
        audio_cocec_type = HA_AUDIO_ID_PCM;
        audio_codec_id = aAUDIO_LPCM_BE;
        audio_pcm_be = 1;
    } else if (
        sh_audio->format == MKTAG('t', 'w', 'o', 's') ||
        sh_audio->format == MKTAG('i', 'n', '2', '4') ||
        sh_audio->format == MKTAG('i', 'n', '3', '2')) {
        audio_cocec_type = HA_AUDIO_ID_PCM;
        audio_codec_id = aAUDIO_LPCM_BE;
        audio_pcm_be = 1;
    } else if (sh_audio->format == 0x01) {
        audio_cocec_type = HA_AUDIO_ID_PCM;
        audio_codec_id = aAUDIO_LPCM_BE;
        audio_pcm_be = 0;
    } else if (sh_audio->format == 0x11) { //ADPCM unsupport
        audio_cocec_type = HA_AUDIO_ID_INVALID;
        audio_codec_id = aAUDIO_ADPCM;
    } else if (sh_audio->format == 0x6b6f6f63 /*ra cook */) {
        audio_cocec_type = HA_AUDIO_ID_COOK;
        //audio_codec_id = aAUDIO_ADPCM;
    } else if (sh_audio->format == 0x726d6173 /*amr nb */) {
        /* audio_cocec_type = HA_AUDIO_ID_AMRNB; not support so mark */
    } else if (sh_audio->format == 0x20455041 /*ape */) {
        audio_cocec_type = HA_AUDIO_ID_APE;//for temp fix me later
    } else if (sh_audio->format == 0x7375706f) {
        audio_cocec_type = HA_AUDIO_ID_OPUS;
    } else if (sh_audio->format == MKTAG('f', 'L', 'a', 'C') || sh_audio->format == 0xf1ac) {
        audio_codec_id   = aAUDIO_FLAC;
        audio_cocec_type = HA_AUDIO_ID_FLAC;
    } else if (sh_audio->format == 0x566f || sh_audio->format == 0x6770 || sh_audio->format == 0x6771/*ogg VORBIS */) {
        audio_codec_id   = aAUDIO_VORBIS;
        audio_cocec_type = HA_AUDIO_ID_VORBIS;//for temp fix me later
    } else if (sh_audio->format == 0x160 || sh_audio->format == 0x161) {
        if (sh_audio->channels != 1) {
            audio_codec_id   = aAUDIO_WMA;
            audio_cocec_type = HA_AUDIO_ID_WMA9STD;
        } else {
            MLOGW("Unsupport wma audio with 1ch\n");
        }
    } else {
        MLOGW("[%s] unkown audio type 0x%x\n", __func__, sh_audio->format);
        audio_cocec_type = HA_AUDIO_ID_INVALID;
    }
    if (pcm_be) {
        *pcm_be = audio_pcm_be;
    }
    if (acodec_id) {
        *acodec_id = audio_codec_id;
    }
    if (p_audio_type) {
        *p_audio_type = audio_cocec_type;
    }
    MLOGD("%s audio format 0x%x type:0x%x id:%d\n", __func__, sh_audio->format, audio_cocec_type, audio_codec_id);
    return MT_SUCCESS;
}

/*
* get the audio type audio codec id is_pcm? by  audio head info
*/
int ds_get_audio_codec_info(demux_stream_t *ds, unsigned int *p_audio_type, int *pcm_be,
                            unsigned int *acodec_id, int *apid, AUDIO_OUT_MODE aout_mode)
{
    sh_audio_t *sh_audio = ds->sh;
    int audio_eac3_flag  = 0;

    *pcm_be    = 0;
    *acodec_id = (unsigned int) -1;
    *p_audio_type = HA_AUDIO_ID_INVALID;
    if (NULL == sh_audio) {
        return MT_FAILURE;
    }

    if (DEMUXER_TYPE_MPEG_TS == ds->demuxer->type) {
        ts_priv_t *priv = (ts_priv_t *)ds->demuxer->priv;
        int i;
        pmt_t *pmt;

        pmt = (pmt_t *)(&priv->pmt[priv->selected_program_idx]);
        if (NULL == pmt && sh_audio) {
            MLOGE("%s %d sh_audio->format 0x%x ts without pmt....\n", __func__, __LINE__, sh_audio->format);
            switch (sh_audio->format) {
                case AUDIO_MP2_MP:
                    *p_audio_type = HA_AUDIO_ID_MP2;
                    *acodec_id = aAUDIO_MP2;
                    break;
                case MKTAG('B', 'P', 'C', 'M'):
                    *p_audio_type = HA_AUDIO_ID_PCM;
                    *acodec_id = aAUDIO_PCM_BR;
                    *pcm_be = 1;
                    break;
                case AUDIO_LPCM_BE_MP:
                    *p_audio_type = HA_AUDIO_ID_PCM;
                    *acodec_id = aAUDIO_LPCM_BE;
                    *pcm_be = 1;
                    break;
                case AUDIO_A52_MP:
                    if (aout_mode == AUDIO_LPCM) {
                        if (audio_eac3_flag) {
                            *p_audio_type = HA_AUDIO_ID_DOLBY_PLUS; //5;//AUDIO_EAC3
                        } else {
                            *p_audio_type = HA_AUDIO_ID_DOLBY_TRUEHD; //4;//AUDIO_AC3_VSB
                        }
                    } else {
                        if (audio_eac3_flag) {
                            *p_audio_type = HA_AUDIO_ID_EAC3PASSTHROUGH;
                        } else {
                            *p_audio_type = HA_AUDIO_ID_AC3PASSTHROUGH; //4
                        }
                    }

                    *acodec_id = aAUDIO_AC3;
                    break;
                case AUDIO_AAC_MP:
                case AUDIO_AAC_LATM_MP:
                    *p_audio_type = HA_AUDIO_ID_AAC; //6;
                    *acodec_id = aAUDIO_AAC;
                    break;
                case AUDIO_DTS_MP:                   //CODEC_ID_DTS
                    *p_audio_type = HA_AUDIO_ID_DTSPASSTHROUGH;//HA_AUDIO_ID_DTSHD; //
                    *acodec_id = aAUDIO_DTS;
                    break;
                default:
                    MLOGI("%s %d\n", __func__, __LINE__);
                    break;
            }
            return MT_SUCCESS;
        }
        if (!pmt) {
            return MT_FAILURE;
        }
        //this is for audio change track for specific apid
        for (i = 0; i < pmt->es_cnt; i++) {
            if (pmt->es[i].pid == *apid) {
                if (pmt->es[i].type == AUDIO_MP2_MP) {
                    *p_audio_type = HA_AUDIO_ID_MP2;
                    *acodec_id = aAUDIO_MP2;
                    return MT_SUCCESS;
                } else if (pmt->es[i].type == AUDIO_PCM_BR_MP) {
                    *p_audio_type = HA_AUDIO_ID_PCM;
                    *acodec_id = aAUDIO_PCM_BR;
                    *pcm_be = 1;
                    return MT_SUCCESS;
                } else if (pmt->es[i].type == AUDIO_LPCM_BE_MP) {
                    *p_audio_type = HA_AUDIO_ID_PCM;
                    *acodec_id = aAUDIO_LPCM_BE;
                    *pcm_be = 1;
                    return MT_SUCCESS;
                } else if (AUDIO_TRUEHD_AC3_MP == pmt->es[i].type || AUDIO_A52_MP == pmt->es[i].type) {
                    audio_eac3_flag = pmt->es[i].audio_eac3_flag;

                    if (aout_mode == AUDIO_LPCM) {
                        if (audio_eac3_flag) {
                            *p_audio_type = HA_AUDIO_ID_DOLBY_PLUS; //5;//AUDIO_EAC3
                        } else {
                            *p_audio_type = HA_AUDIO_ID_DOLBY_TRUEHD; //4;//AUDIO_AC3_VSB
                        }
                    } else {
                        if (audio_eac3_flag) {
                            *p_audio_type = HA_AUDIO_ID_EAC3PASSTHROUGH;
                        } else {
                            *p_audio_type = HA_AUDIO_ID_AC3PASSTHROUGH; //4
                        }
                    }
                    *acodec_id = (AUDIO_A52_MP == pmt->es[i].type) ? aAUDIO_AC3 : aAUDIO_TRUEHD;
                    return MT_SUCCESS;
                } else if (AUDIO_AC4_MP == pmt->es[i].type) {
                    *p_audio_type = HA_AUDIO_ID_DOLBY_AC4;//6;
                    *acodec_id = aAUDIO_AC4;
                    return MT_SUCCESS;
                } else if ((pmt->es[i].type == AUDIO_AAC_MP) || (pmt->es[i].type == AUDIO_AAC_LATM_MP)) {
                    *p_audio_type = HA_AUDIO_ID_AAC;//6;
                    *acodec_id = aAUDIO_AAC;
                    return MT_SUCCESS;
                } else if ((pmt->es[i].type == AUDIO_DTS_MP)) {//tizhang@20181017 for 105221
                    *p_audio_type = HA_AUDIO_ID_DTSPASSTHROUGH;
                    *acodec_id = aAUDIO_DTS;
                    return MT_SUCCESS;
                } else if (pmt->es[i].type == AUDIO_AV3A_MP) {
                    *p_audio_type = HA_AUDIO_ID_VVID;
                    *acodec_id = aAUDIO_AV3A;
                } else {
                    MLOGE("%s ERROR not support 0x%x !!!!!!!!!!!\n", __func__, pmt->es[i].type);
                    *p_audio_type = HA_AUDIO_ID_INVALID;
                    if (pmt->es[i].type == AUDIO_DTS_MP) {
                        *acodec_id = aAUDIO_DTS;
                    }
                }
                return MT_SUCCESS;
            }
        }

        sh_audio->samplerate = pmt->pcm_info.sample_rate;
        sh_audio->channels = pmt->pcm_info.channels;
        MLOGD("sh_audio->channels %d\n", sh_audio->channels);
    } else {
        int ret = ds_get_audio_common_codec_info(ds,
            p_audio_type, pcm_be, acodec_id, apid, aout_mode);
        if (MT_SUCCESS != ret) {
            return ret;
        }
    }

    MLOGD("[%s] audio format 0x%x,id[%x]\n", __func__, sh_audio->format, *acodec_id);
    return MT_SUCCESS;
}

void set_osd_subtitle(subtitle *subs)
{
    int i;
    vo_sub = subs;
    sub_temp = NULL;

    // reverse order, since newest set_osd_msg is displayed first
    for (i = SUB_MAX_TEXT - 1; i >= 0; i--) {
        if (!subs || i >= subs->lines || !subs->text[i]) {
        } else {
            // HACK: currently display time for each sub line except the last is set to 2 seconds.
            sub_temp = subs;
            sub_flag = 1;
        }
    }
}

/* NAL unit types */
enum {
    NAL_SLICE = 1,
    NAL_DPA,
    NAL_DPB,
    NAL_DPC,
    NAL_IDR_SLICE,
    NAL_SEI,
    NAL_SPS,
    NAL_PPS,
    NAL_AUD,
    NAL_END_SEQUENCE,
    NAL_END_STREAM,
    NAL_FILLER_DATA,
    NAL_SPS_EXT,
    NAL_AUXILIARY_SLICE = 19
};

static const uint8_t golomb_to_pict_type[5] = {
    AV_PICTURE_TYPE_P, AV_PICTURE_TYPE_B, AV_PICTURE_TYPE_I,
    AV_PICTURE_TYPE_SP, AV_PICTURE_TYPE_SI
};

static int parse_avc_info(const uint8_t *buf, int size, picture_info_t *p_info)
{
    unsigned int slice_type = 0;
    MT_BOOL ref_flag = FALSE;
    MT_BOOL idr_flag = FALSE;
    int i;
    static int last_frame_num = 0;
    static int last_bottom_field_flag = 0;
    static int last_nalu = 0;
    static MT_BOOL last_ref_flag = FALSE;
    static unsigned int last_slice_type = 0;

    for (i = 0; i < size - 3; i++) {
        if (IDENTIFY_NALU_STARTCODE(buf + i)) {
            if ((*(buf + i + 3) & 0x1f) == NAL_SLICE) {
                if ((*(buf + i + 3) & 0x60) != 0) {
                    ref_flag = TRUE;
                }

                slice_type = (unsigned int)x_check_slice_type3(buf + i, 0, size - i,
                                                               &p_info->frame_mbs_only, &p_info->field_flag, &p_info->bottom_field_flag, &p_info->frame_num);

                slice_type = golomb_to_pict_type[slice_type % 5];
                if ((slice_type == AV_PICTURE_TYPE_I) && (last_nalu == NAL_SEI)) {
                    idr_flag = TRUE;
                } else {
                    idr_flag = FALSE;
                }

                last_nalu = NAL_SLICE;

                if (ref_flag) {
                    break;
                }

            } else if ((*(buf + i + 3) & 0x1f) == NAL_IDR_SLICE) {
                if ((*(buf + i + 3) & 0x60) != 0) {
                    ref_flag = TRUE;
                }

                idr_flag = TRUE;

                slice_type = (unsigned int)x_check_slice_type3(buf + i, 0, size - i,
                                                               &p_info->frame_mbs_only, &p_info->field_flag, &p_info->bottom_field_flag, &p_info->frame_num);
                slice_type = golomb_to_pict_type[slice_type % 5]; //jqw@20181015,for bug 105243
                last_nalu = NAL_IDR_SLICE;

                if (ref_flag) {
                    break;
                }
            } else if ((*(buf + i + 3) & 0x1f) == NAL_SPS) {
                x_check_spsframe(buf + i, 0, size - i, &p_info->frame_mbs_only);
                last_nalu = NAL_SPS;
            } else if ((*(buf + i + 3) & 0x1f) == NAL_SEI) {
                if (last_nalu == NAL_SPS) {
                    if (x_check_recovery_point(buf + i, 0, size - i)) {
                        last_nalu = NAL_SEI;
                    }
                }
            }
        }
    }


    p_info->slice_type = (int)slice_type;
    p_info->idr_flag = idr_flag;
    p_info->ref_flag = ref_flag;

    p_info->last_bottom_field_flag = last_bottom_field_flag;
    p_info->last_ref_flag = last_ref_flag;
    p_info->last_slice_type = (int)last_slice_type;
    p_info->last_frame_num = last_frame_num;

    last_ref_flag = p_info->ref_flag;
    last_slice_type = (unsigned int)p_info->slice_type;
    last_frame_num = p_info->frame_num;
    last_bottom_field_flag = p_info->bottom_field_flag;

    if (slice_type != 0) {
        return 0;
    }
    return -1;

}

static int parse_mpeg2_info(const uint8_t *buf, int size, picture_info_t *p_info)
{
    int i;
    static int last_ref_flag = FALSE;
    p_info->slice_type = 0;
    p_info->ref_flag = FALSE;
    p_info->idr_flag = FALSE;

    for (i = 0; i < size - 4; i++) {
        if (IDENTIFY_PICTURE_STARTCODE(buf + i)) {
            if ((*(buf + i + 5) & 0x38) == 8) {
                p_info->slice_type = AV_PICTURE_TYPE_I;
            } else if ((*(buf + i + 5) & 0x38) == 0x10) {
                p_info->slice_type = AV_PICTURE_TYPE_P;
            } else if ((*(buf + i + 5) & 0x38) == 0x18) {
                p_info->slice_type = AV_PICTURE_TYPE_B;
            }

            p_info->idr_flag = TRUE;
            if (p_info->slice_type == AV_PICTURE_TYPE_I || p_info->slice_type == AV_PICTURE_TYPE_P) {
                p_info->ref_flag = TRUE;
            }

            last_ref_flag = p_info->ref_flag;
            return 0;
        }
    }

    p_info->slice_type = AV_PICTURE_TYPE_S_DATA;
    p_info->ref_flag = last_ref_flag;

    return -1;
}

static int parse_mpeg4_info(const uint8_t *buf, int size, picture_info_t *p_info)
{
    int i;
    unsigned int state       = 0xFFFFffff;
    static int last_ref_flag = FALSE;

    /* only useful for h264 */
    p_info->idr_flag   = FALSE;
    p_info->ref_flag   = FALSE;
    p_info->slice_type = 0;
    for (i = 0; i < size - 1; i++) {
        state = (state << 8) | buf[i];
        if (0x100 == (state & 0xFFFFFF00)) {
            last_ref_flag = p_info->ref_flag;
        }

        if (MPEG4_VOP_STARTCODE != state) {
            continue;
        }
        /* pict type, unsigned, most significant bit first, I=0,P=1,B=2,S=3 */
        p_info->slice_type = ((buf[i + 1] >> 6) & 0x3) + AV_PICTURE_TYPE_I;
        if (p_info->slice_type == AV_PICTURE_TYPE_I ||
            p_info->slice_type == AV_PICTURE_TYPE_P) {
            last_ref_flag = p_info->ref_flag = TRUE;
            return MT_SUCCESS;
        }
    }

    p_info->ref_flag   = last_ref_flag;
    p_info->slice_type = AV_PICTURE_TYPE_S_DATA;

    return MT_FAILURE;
}


static int parse_hevc_info(const uint8_t *buf, int size, picture_info_t *p_info)
{
    unsigned int slice_type = 0;
    MT_BOOL ref_flag = FALSE;
    int i;
    u32 type;

    for (i = 0; i < size - 3; i++) {
        if (IDENTIFY_NALU_STARTCODE(buf + i)) {
            type = HEVC_NAL_TYPE(*(buf + i + 3));
            if ((type >= HEVC_NAL_BLA_W_LP) && (type <= HEVC_NAL_IDR_N_LP)) {
                slice_type = AV_PICTURE_TYPE_I;
                ref_flag = TRUE;
                break;
            } else if (type == HEVC_NAL_CRA_NUT) {
                slice_type = AV_PICTURE_TYPE_I;
                ref_flag = TRUE;
                break;
            } else if (type == HEVC_NAL_TRAIL_R || type == HEVC_NAL_TSA_R || type == HEVC_NAL_STSA_R) {
                ref_flag = TRUE;
                break;
            }
        }
    }
    p_info->last_slice_type = p_info->slice_type;
    p_info->slice_type = (int)slice_type;
    p_info->idr_flag = FALSE;
    p_info->last_ref_flag = p_info->ref_flag;
    p_info->ref_flag = ref_flag;
    if (slice_type != 0) {
        return 0;
    }
    return -1;

}

static int parse_avs_info(const uint8_t *buf, int size, picture_info_t *p_info) //jqw@20190709 for bug111053
{
    int i;
    u32 type;
    unsigned int slice_type = AV_PICTURE_TYPE_NONE;
    MT_BOOL ref_flag = FALSE;
    static MT_BOOL last_ref_flag = FALSE;


    for (i = 0; i < size - 3; i++) {
        if (IDENTIFY_NALU_STARTCODE(buf + i)) {
            type = *(buf + i + 3) & 0xff;
            if (type == 0xB3) {
                slice_type = AV_PICTURE_TYPE_I;
                ref_flag = TRUE;
                break;
            } else if (type == 0xB6) {
                if ((*(buf + i + 6) & 0xc0) == 0x40) {
                    slice_type = AV_PICTURE_TYPE_P;
                    ref_flag = TRUE;
                    break;
                } else if ((*(buf + i + 6) & 0xc0) == 0x80) {
                    slice_type = AV_PICTURE_TYPE_B;
                    break;
                } else {
                    break;
                }
            }
        }
    }
    p_info->slice_type = slice_type;
    if (slice_type == AV_PICTURE_TYPE_NONE) {
        p_info->ref_flag = last_ref_flag;
    } else {
        p_info->ref_flag = ref_flag;
    }

    last_ref_flag = p_info->ref_flag;
	return 0;
}

static int parse_picture_info(int vdec_type, const uint8_t *buf, int size, picture_info_t *p_info)
{
    int ret = 0;

    if (vdec_type == DS_SEL_VDEC_TYPE_H264) {
        ret = parse_avc_info(buf, size, p_info);
    } else if (vdec_type == DS_SEL_VDEC_TYPE_MPEG4) {
        ret = parse_mpeg4_info(buf, size, p_info);
    } else if (vdec_type == DS_SEL_VDEC_TYPE_MPEG12) {
        ret = parse_mpeg2_info(buf, size, p_info);
    } else if (vdec_type == DS_SEL_VDEC_TYPE_HEVC) {
        ret = parse_hevc_info(buf, size, p_info);
    } else if (vdec_type == DS_SEL_VDEC_TYPE_AVS) {
        ret = parse_avs_info(buf, size, p_info);
    } else {
        p_info->ref_flag = TRUE;
    }
    return ret;
}

//jqw@20180928 for trickplay end
/* Start codes. */
#define SEQ_END_CODE            0x000001b7
#define SEQ_START_CODE          0x000001b3
#define GOP_START_CODE          0x000001b8
#define PICTURE_START_CODE      0x00000100
#define SLICE_MIN_START_CODE    0x00000101
#define SLICE_MAX_START_CODE    0x000001af
#define EXT_START_CODE          0x000001b5
#define USER_START_CODE         0x000001b

static uint8_t *mpv_find_start_code(uint8_t *p,
                                    uint8_t *end,
                                    uint32_t *state)
{
    int i;

    if (p >= end) {
        return end;
    }

    for (i = 0; i < 3; i++) {
        uint32_t tmp = *state << 8;
        *state = tmp + *(p++);

        if (tmp == 0x100 || p == end) {
            return p;
        }
    }

    while (p < end) {
        if (p[-1] > 1) {
            p += 3;
        } else if (p[-2]) {
            p += 2;
        } else if (p[-3] | (p[-1] - 1)) {
            p++;
        } else {
            p++;
            break;
        }
    }

    p = FFMIN(p, end) - 4;
    *state = MRD_BE32(p);
    return p + 4;
}

//called when backwards and fill i frame 0 2k
static int ps_mpeg_iframe_fill0(unsigned char *buf, int size)
{
    uint32_t start_code;
    uint8_t *buf_end;
    int bytes_left = 0;
    buf_end = buf + size;
    int pic_type = 0;
    start_code = (uint32_t) -1;
    while (buf < buf_end) {
		buf = mpv_find_start_code(buf, buf_end, &start_code);
		bytes_left = buf_end - buf;

		if (start_code == PICTURE_START_CODE) {
			if (bytes_left >= 2) {
				pic_type = (buf[1] >> 3) & 7;
				if (pic_type == 3 || pic_type == 2) {
					//MLOGI("B frame findout offset %d \n",size - bytes_left);
					memset(buf - 4, 0, (unsigned int)bytes_left + 4);
				}
			}
			return 0;
		} else {
			continue;
		}
    }

    //MLOGI("No frame findout\n");
    return 1;
}

/*!
parse sequence header, gop header and picture header, save offset and size
*/
static int parse_mpeg2_packet(const uint8_t *buf, int size, sc_info_t *p_info)
{
    int i;
    u8 val;

    p_info->sc_cnt = 0;
    for (i = 0; i < size - 3; i++) {
        if (IDENTIFY_NALU_STARTCODE(buf + i)) {
            val = (*(buf + i + 3)) & 0xff;
            if ((val == 0x00) || (val == 0xb3) || (val == 0xb8)) {
                if (p_info->sc_cnt > 0) {
                    if (p_info->sc_val[p_info->sc_cnt - 1] == 0xb3 && val == 0xb8) {
                        //merge sequence header with gop header
                        p_info->sc_val[p_info->sc_cnt - 1] = val;
                        continue;
                    } else if ((p_info->sc_val[p_info->sc_cnt - 1] == 0xb3 || p_info->sc_val[p_info->sc_cnt - 1] == 0xb8) && val == 0x00) {
                        //merge sequence header with gop header
                        p_info->sc_val[p_info->sc_cnt - 1] = val;
                        continue;
                    }
                }

                p_info->sc_val[p_info->sc_cnt] = val;
                p_info->sc_offset[p_info->sc_cnt] = i;
                p_info->sc_valid[p_info->sc_cnt] = 1;
                p_info->sc_cnt++;
            }
        }
    }

    for (i = 0; i < p_info->sc_cnt; i++) {
        if (i + 1 < p_info->sc_cnt) {
            p_info->sc_size[i] = p_info->sc_offset[i + 1] -  p_info->sc_offset[i];
        } else {
            p_info->sc_size[i] = size - p_info->sc_offset[i];
        }

        MLOGD("[%d]: val:%02x, offset:%d, size:%d", i, p_info->sc_val[i], p_info->sc_offset[i], p_info->sc_size[i]);
    }
    return 0;
}

/*!
split packet into sub packets, so that data from 2 slices will not be in one sub packet
*/
static int ds_split_packet_video(unsigned char **start, int *size, int vdec_type)
{
    int i;
	int ret = 0;
    if (vdec_type == DS_SEL_VDEC_TYPE_MPEG12) { //mpeg2
        if (g_sc_info.sc_cnt == 0) { //return the first sub packet after split
            memset(&g_sc_info, 0, sizeof(sc_info_t));
            g_sc_info.p_buf = *start;
            MLOGD("~~~~~~~~start:%x, total size:%d", *start, *size);
            parse_mpeg2_packet(*start, *size, &g_sc_info);

            if (g_sc_info.sc_cnt > 0 && (g_sc_info.sc_offset[0] > 1 || (g_sc_info.sc_offset[0] == 1 && (**start != 0x00)))) {
                *size = g_sc_info.sc_offset[0];
                MLOGD("first sc not at the start pos, 1st size:%d", *size);
            } else if (g_sc_info.sc_cnt == 0) {
                MLOGD("no sc, 1st size:%d", *size);
            } else {
                if (g_sc_info.sc_offset[0] == 1) {
                    *start = *start + 1;
                    *size = g_sc_info.sc_size[0];
                }
                MLOGD("one or more than one sc and the first one is at the start pos, 1st size:%d", *size);
                g_sc_info.sc_valid [0] = 0;
                if (g_sc_info.sc_cnt == 1) {
                    g_sc_info.sc_cnt = 0;
                }
                if (g_sc_info.sc_val[0] == 0xb3 || g_sc_info.sc_val[0] == 0xb8) {
                    ret = 1;
                }

            }
        } else { //return the other sub packets one by one after split
            for (i = 0; i < g_sc_info.sc_cnt; i++) {
                if (g_sc_info.sc_valid[i]) {
                    *start = g_sc_info.p_buf + g_sc_info.sc_offset[i];
                    *size = g_sc_info.sc_size[i];
                    g_sc_info.sc_valid[i] = 0;
                    if (i == g_sc_info.sc_cnt - 1) {
                        g_sc_info.sc_cnt = 0;
                    }

                    MLOGD("~~~~~i:%d~~~start:%x, size:%d", i, *start, *size);
                    if (g_sc_info.sc_val[i] == 0xb3 || g_sc_info.sc_val[i] == 0xb8) {
                        ret = 1;
                    }
                }
            }
        }
    } else {
        memset(&g_sc_info, 0, sizeof(sc_info_t));
    }
	return ret;
}

static int speed_to_speedindex(int speed)
{
	int speed_index;
	switch (speed) {
		case TS_SEQ_FAST_PLAY_2X:
			speed_index = 2;
			break;
		case TS_SEQ_FAST_PLAY_4X:
			speed_index = 4;
			break;
		case TS_SEQ_FAST_PLAY_8X:
			speed_index = 8;
			break;
		case TS_SEQ_FAST_PLAY_16X:
			speed_index = 16;
			break;
		case TS_SEQ_FAST_PLAY_32X:
			speed_index = 32;
			break;
		case TS_SEQ_REV_FAST_PLAY_2X:
			speed_index = -2;
			break;
		case TS_SEQ_REV_FAST_PLAY_4X:
			speed_index = -4;
			break;
		case TS_SEQ_REV_FAST_PLAY_8X:
			speed_index = -8;
			break;
		case TS_SEQ_REV_FAST_PLAY_16X:
			speed_index = -16;
			break;
		case TS_SEQ_REV_FAST_PLAY_32X:
			speed_index = -32;
			break;
		default:
			speed_index = 1;
			break;
	}
	return speed_index;
}

extern double get_stream_time(void);
int ps_trick_back = 0;
static double last_key_pts_back = 0.0f;



static int ps_trick_forward_select(int frame_max,ps_trick_t ps_trick,
	demux_stream_t *ds,int speed_index)
{
	frame_max = (2 * 25 / speed_index);
	if (ds->pts - gop_start > -0.01 && ds->pts - gop_start < 0.01) {
		//ps has the same pts is one frame
		return DEMUX_SEL_PUSH_CUR_PKT;
	}
	//MLOGI("%s pts %d  size %d packnum %d \n",__func__, (int)(ds->pts * 1000),size,pspacketnum);
	gop_start = ds->pts;
	write_flag ++;

	if (write_flag > frame_max) {
		// yliu add for ts trick
		float seek_modify = 0;

		if (ps_trick.seek_after_tick != 0) {
			seek_modify = (ps_trick.seek_after_tick - ps_trick.seek_before_tick) * 1.f / 100;
		}

		ps_trick.seek_before_tick = mtos_ticks_get();
		demux_seek(ds->demuxer, 2 + seek_modify, 0, 0); //forward 1s
		ps_trick.seek_after_tick = mtos_ticks_get();
		write_flag = 0;
		return DEMUX_SEL_GET_NEXT_PKT;
	}
	return DEMUX_SEL_PUSH_CUR_PKT;
}



static int ps_fb_need_sleep(demuxer_t *demuxer, float first_pts, float speed)
{
    const static int MAX_SLEEP_MS = 500;
    mt_s64 pts_diff_ms  = (mt_s64) fabs(first_pts * TIME_BASE - demuxer->trick.start_pts);
    mt_s64 real_time_diff_ms = (mt_s64) mclock_get_utime() / TIME_BASE - demuxer->trick.start_time;

    /* time smaller and packet not send before */
    real_time_diff_ms *= fabs(speed);
    int ret = 0;
    if (real_time_diff_ms < pts_diff_ms) {
        mt_s64 sleep_time = (pts_diff_ms - real_time_diff_ms) * TIME_BASE;
        sleep_time = MAX(TIME_BASE, sleep_time);
        MT_USLEEP(sleep_time);
        ret = 1;
    }

    return ret;
}


static int ps_trick_backward_select(int frame_max,ps_trick_t ps_trick,demux_stream_t *ds,
	int speed_index,unsigned char **start,int *size)
{
    int wait_flg = 0;
	static int same_pts_frame_count = 0;
	frame_max = -(30 / speed_index);
	static int ps_2k_last_piece = 0;

	if (seek_back_end == 0) {
		if (write_flag == 0) {
			gop_end = ds->pts;
		}
		//this is i frame
		seek_back_end = 1;
		write_flag ++;
		return DEMUX_SEL_PUSH_CUR_PKT;
	}

	if (seek_back_end == 1) {
		//this frame is located on right of i frame
		if ((fabs(ds->pts - gop_end) < 0.01) && (gop_end != 0)) {
			//ps has the same pts is one frame
			ps_2k_last_piece = 1;
			same_pts_frame_count++;
			return DEMUX_SEL_PUSH_CUR_PKT;
		}

		if (ps_2k_last_piece) {
			ps_2k_last_piece = 0;
			ps_mpeg_iframe_fill0(*start, *size);
			return DEMUX_SEL_PUSH_CUR_PKT;
		}

        wait_flg = ps_fb_need_sleep(ds->demuxer, gop_end, speed_index);

        if(wait_flg == 1) {
            //has been sleep for a while
		} else if (write_flag >= frame_max) {
			static int repeat_key = 0;
			if (fabs(last_key_pts_back - ds->pts) < 0.5f) {
				repeat_key++;
			} else {
				repeat_key = 0;
			}
			last_key_pts_back = ds->pts;
			if (repeat_key > 3) {
				demux_seek(ds->demuxer, -4, 0, 0);    //forward -2s
			} else {    //need to seek back 2 s
				demux_seek(ds->demuxer, -2, 0, 0);    //forward -2s
			}
			write_flag = 0;

		} else if (same_pts_frame_count >= 20) {
			float seek_sec = (speed_index * (1.f * 15 / gst_select_fps));
			if(seek_sec < 0.0) {
				seek_sec -= 1.0;
			}
			demux_seek(ds->demuxer, seek_sec, 0, 0);
			write_flag = 0;
			//MLOGI("---  seek_sec:%0.2f \n", seek_sec);

		} else {
			//need to seek back to the i frame just before
			demux_seek(ds->demuxer, gop_end, 0, 1);  //forward 1s
		}

		seek_back_end = 0;
		same_pts_frame_count = 0;
		return DEMUX_SEL_GET_NEXT_PKT;
	}
	return DEMUX_SEL_PUSH_CUR_PKT;
}


static int do_ps_trick_select(demux_stream_t *ds, unsigned char **start, int speed_index,
	int *size,ps_trick_t ps_trick)
{
	int frame_max = 0;

	if (speed_index > 0) {
		ps_trick_back = 0;
		return ps_trick_forward_select(frame_max,ps_trick,ds,speed_index);
	} else {
		ps_trick_back = 1;
		return ps_trick_backward_select(frame_max,ps_trick,ds,speed_index,start,size);
   	}
}

static int	ts_get_v_dec_by_pmt(pmt_t *pmt)
{
	int i = 0;
	int v_dec = -1;

	for (i = 0; i < pmt->es_cnt; i++) {
		if ((pmt->es[i].type == VIDEO_MPEG1_MP) || (pmt->es[i].type == VIDEO_MPEG2_MP)) {
			v_dec = DS_SEL_VDEC_TYPE_MPEG12;
			break;
		} else if (pmt->es[i].type == VIDEO_H264_MP) {
			v_dec = DS_SEL_VDEC_TYPE_H264;
			break;
		} else if (pmt->es[i].type == VIDEO_HEVC_MP) {
			v_dec = DS_SEL_VDEC_TYPE_HEVC;
			break;
		} else if (i == pmt->es_cnt - 1) { //jqw@20190118, for bug107189
			v_dec = DS_SEL_VDEC_TYPE_DEFAULT;
			break;
		} else if (pmt->es[i].type == VIDEO_MPEG4_MP) {
			v_dec = DS_SEL_VDEC_TYPE_MPEG4;
			break;
		}
	}

	return v_dec;
}


static int ts_get_v_dec_by_sh(sh_video_t *sh_video)
{
	int v_dec = 0;

	switch (sh_video->format) {
		case VIDEO_MPEG1_MP:
		case VIDEO_MPEG2_MP:
			v_dec = DS_SEL_VDEC_TYPE_MPEG12;
			break;
		case VIDEO_H264_MP:
			v_dec = DS_SEL_VDEC_TYPE_H264;
			break;
		case VIDEO_HEVC_MP:
			v_dec = DS_SEL_VDEC_TYPE_HEVC;
			break;
		case VIDEO_AVS_MP:
			v_dec = DS_SEL_VDEC_TYPE_AVS;
			break;
		default:
			v_dec = DS_SEL_VDEC_TYPE_DEFAULT;
			break;
	}

	return v_dec;
}

static int normal_get_v_dec_by_bicomp(unsigned long long bicomp,sh_video_t *sh_video)
{
	int v_dec = 0;

	if (strstr(((char *) &bicomp), "mpg2") || (strstr(((char *) &bicomp), "MPG2"))
		|| (strstr(((char *) &bicomp), "mpg1")) || (sh_video->bih->biCompression == 1)) {
		v_dec = DS_SEL_VDEC_TYPE_MPEG12;
	} else if (strstr(((char *) &bicomp), "H264")  || (strstr(((char *) &bicomp), "avc1"))
		|| strstr(((char *) &bicomp), "h264")) {
		v_dec = DS_SEL_VDEC_TYPE_H264;
	} else if ((strstr(((char *) &bicomp), "H265")) || (strstr(((char *) &bicomp), "hevc"))
		|| (strstr(((char *) &bicomp), "h265")) || (strstr(((char *) &bicomp), "HEVC"))
		|| (strstr(((char *) &bicomp), "hvc1")) || (strstr(((char *) &bicomp), "hev1"))) {
		v_dec = DS_SEL_VDEC_TYPE_HEVC;
	} else if (strstr(((char *)&bicomp), "MP4V")) {
		v_dec = DS_SEL_VDEC_TYPE_MPEG4;
	} else if (strstr(((char *)&bicomp), "VP90")) {
		v_dec = DS_SEL_VDEC_TYPE_VP9;
	} else {
		v_dec = DS_SEL_VDEC_TYPE_DEFAULT;
	}

	return v_dec;
}



static int ds_packet_select(demux_stream_t *ds, unsigned char **start,
							int speed, int *size)
{
    sh_video_t *sh_video = ds->sh;
    unsigned long long biComp = 0;
    int speed_index = 0;
    int i = 0;
    int v_dec = -1;
    static ps_trick_t ps_trick = { .seek_after_tick = 0,.seek_before_tick =0,};
	static int last_speed_index = 0;

    if (speed == TS_SEQ_NORMAL_PLAY || speed == TS_SEQ_FAST_PLAY_2X) {
        last_speed_index = 1;
        ps_trick.seek_after_tick = 0;
		ps_trick.seek_before_tick = 0;
        ps_trick_back = 0;
        return 1;
    }

    speed_index = speed_to_speedindex(speed);
	if(speed_index == 1){
		ps_trick.seek_after_tick = 0;
		ps_trick.seek_before_tick = 0;
		return 1;
	}

    if (last_speed_index != speed_index) {
        last_speed_index = speed_index;
        ps_trick.seek_after_tick = 0;
		ps_trick.seek_before_tick = 0;
        gst_select_init(ds->demuxer, ds->pts, speed_index);
    }

    if (ds->demuxer->type == DEMUXER_TYPE_MPEG_PS) {
        v_dec = DS_SEL_VDEC_TYPE_MPEG12;
        return do_ps_trick_select(ds, start, speed_index, size,ps_trick);
    } else if (DEMUXER_TYPE_MPEG_TS == ds->demuxer->type) {
        ts_priv_t *priv = (ts_priv_t *)ds->demuxer->priv;
        pmt_t *pmt;

        pmt = (pmt_t *)(&priv->pmt[priv->selected_program_idx]);
        if (pmt) {
            v_dec = ts_get_v_dec_by_pmt(pmt);
        } else if(sh_video){
        	v_dec = ts_get_v_dec_by_sh(sh_video);
        }
		if(-1 == v_dec){
			MLOGI("ts format can not get v_dec!!!!\n");
			return 1;
		}
		goto TS_TRICK;

    } else if (sh_video->bih) {
        biComp = le2me_32(sh_video->bih->biCompression);
		v_dec=normal_get_v_dec_by_bicomp(biComp,sh_video);
		goto NORMAL_TRICK;

    } else {
        MLOGI("ERROR not support %.4s !!!!!!!!!!!\n", (char *)&biComp);
        return 1;
    }

TS_TRICK:
    if (DEMUXER_TYPE_MPEG_TS == ds->demuxer->type) {
#ifdef SUPPORT_SPLIT
        if (ds_split_packet_video(start, size, v_dec)) {
            return 1;
        }
#endif
        picture_info_t info = {0};
        parse_picture_info(v_dec, *start, *size, &info);
        if (speed_index > 0) {
            return gst_forward_select(ds->pts, speed_index * 1.f / 2, ds->demuxer, v_dec, &info);
        } else {
            return gst_backward_select(ds->pts, speed_index, ds->demuxer, v_dec, &info);
        }
    }

    return 1;
NORMAL_TRICK: {
        picture_info_t info = {0};
        parse_picture_info(v_dec, *start, *size, &info);

        if(ds->demuxer->type == DEMUXER_TYPE_LAVF && ds->flags) {
            info.slice_type = AV_PICTURE_TYPE_I;
        }

        if (speed_index > 0) {
            return gst_forward_select(ds->pts, speed_index * 1.f / 2, ds->demuxer, v_dec, &info);
        } else {
            return gst_backward_select(ds->pts, speed_index * 1.f, ds->demuxer, v_dec, &info);
        }
    }

    return 1;
}

void ds_reset_trickplay_para(demuxer_t *demuxer)
{
    struct demuxer_trick *trick = &demuxer->trick;
    gop_start = 0;
    gop_end = 0;
    write_flag = 0;
    seek_back_end = 0;
    seek_back_cnt = 0;
    seek_cnt = 0;
    trick->speed = 0;
}

int ds_get_packet_video(demux_stream_t *ds, unsigned char **start, int8_t speed)
{
    int in_size = 0;
    sh_video_t *sh_video = ds->sh;
    static int8_t speed_old = TS_SEQ_NORMAL_PLAY;
    int ret = 0;

    while (1) {
        if (speed_old != speed) {
            ds_reset_trickplay_para(ds->demuxer);
            //fix bug 112761,jqw
            sh_video->sh.first_idr = 1;
            memset(&g_sc_info, 0, sizeof(sc_info_t));
        }

        speed_old = speed;
        if (g_sc_info.sc_cnt == 0) { //after all of the subpackets processed, get the new packet
            in_size = ds_get_packet(ds, start);
        }
        if (!((in_size > 0) || g_sc_info.sc_cnt)) {
            break;
        }

        ret = ds_packet_select(ds, start, speed, &in_size);
        if (ret) {

            if (in_size > 0) {
                //jqw@20180928 for trickplay
                if (ret == DEMUX_SEL_PUSH_CUR_PKT) { //jqw@20181012 for bug105243
                    break;
                } else if (ret >= DEMUX_SEL_SLEEP_AND_PUSH_PKT) {
                    mtos_task_sleep((unsigned int)ret);
                } else if (ret == DEMUX_SEL_SAVE_AND_PUSH_PKT) {
                    if (packet_I_bak_ptr) {
                        free33(packet_I_bak_ptr);
                        packet_I_bak_ptr = NULL;
                    }
                    if (packet_I_bak_ptr2) {
                        free33(packet_I_bak_ptr2);
                        packet_I_bak_ptr2 = NULL;
                    }
                    packet_I_bak_ptr = malloc33((unsigned int)in_size);
                    if (NULL != packet_I_bak_ptr) {
                        memcpy(packet_I_bak_ptr, *start, (unsigned int)in_size);
                        packet_I_size = in_size;
                    }
                } else if (ret == DEMUX_SEL_SAVE_PTS_AND_PUSH_PKT) {
                    if (gst_select_fps > 0.f
                        && fabs(last_pts - ds->pts) > 0.01f) {
                        mtos_task_sleep((unsigned int)(1000 * 1.f / gst_select_fps));
                        last_pts = ds->pts;
                    }
                }
                if (ret == DEMUX_SEL_PUSH_SAVED_KEY_PKT) {
                    last_pts = ds->pts;
                    if (packet_I_bak_ptr2) {
                        in_size = packet_I_size2;
                        *start = packet_I_bak_ptr2;
                        //memcpy(*start,packet_I_bak_ptr2,(unsigned int)in_size);
                    } else {
                        in_size = packet_I_size;
                        *start = packet_I_bak_ptr;
                        //memcpy(*start,packet_I_bak_ptr,(unsigned int)in_size);
                    }
                } else if (ret == DEMUX_SEL_MERGE_AND_PUSH_PKT) {
                    if (packet_I_bak_ptr2) {
                        free33(packet_I_bak_ptr2);
                        packet_I_bak_ptr2 = NULL;
                    }

                    packet_I_size2 = in_size + packet_I_size;
                    packet_I_bak_ptr2 = malloc33((unsigned int)packet_I_size2);
                    if (NULL != packet_I_bak_ptr2) {
                        memcpy(packet_I_bak_ptr2, packet_I_bak_ptr, (unsigned int)packet_I_size);
                        memcpy(packet_I_bak_ptr2 + packet_I_size, *start, (unsigned int)in_size);
                    }
                }
            }
            break;
        }
    }

    return in_size;
}
int ds_get_audio_count(demuxer_t *d)
{
    int i;
    int cnt = 0;

    if (d) {
        for (i = 0; i < MAX_A_STREAMS; ++i) {
            sh_audio_t *sh = d->a_streams[i];

            if (sh) {
                cnt++;
            }
        }
    }

    return cnt;
}

int ds_get_video_count(demuxer_t *d)
{
    int i;
    int cnt = 0;

    if (d) {
        for (i = 0; i < MAX_V_STREAMS; ++i) {
            sh_video_t *sh = d->v_streams[i];

            if (sh) {
                cnt++;
            }
        }
    }

    return cnt;
}

/**************************************************************************************
 * Function:    adts_UnpackFrmSize
 *
 * Description: parse the ADTS frame header and get
 *
 * Inputs:      readBuf
 *
 * Outputs:     framesize
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 **************************************************************************************/
static int adts_UnpackFrmSize(unsigned char *buf, int *framesize)
{
    unsigned char tmp = 0;
    //buf[0,1]=0xFF F0
    tmp = buf[1] & 0x06;

    if (tmp != 0x0) { //layer != 0
        return -3;
    }

    tmp = (buf[2] & 0xc0) >> 6;

    if (tmp != 1 && tmp != 0) { //AAC_PROFILE_LC && tmp != AAC_PROFILE_MP
        return -3;
    }

    tmp = (buf[2] & 0x3c) >> 2;

    if (tmp >= 12) { //sampRateIdx >= NUM_SAMPLE_RATES
        return -3;
    }

    tmp = (buf[2] & 0x1) << 2;
    tmp |= (buf[3] & 0xc0) >> 6;

    if (tmp >= 12) { //channelConfig >= NUM_DEF_CHAN_MAPS
        return -3;
    }

    return 0;
}

int ds_get_packet_audio(demux_stream_t *ds, unsigned char **start, uint8_t **extra_buf, uint8_t *extra_size)
{
#define MP4A_EXTRA_HEAD_LENGTH    7
#define MP4A_EXTRA_HEAD_BUF_BYTES 16
#define AAC_SAMPLE_RATE_TABLE_LEN 16
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    static unsigned char g_aac_head_mp[MP4A_EXTRA_HEAD_BUF_BYTES] = {0};

    if (DEMUXER_TYPE_MPEG_PS == (ds->demuxer)->type) {
        if (p_file_seq->is_av_codec_support == 2) {
            ds->eof = 1;
            *start = NULL;
            return -1;
        }
    }
    int in_size = ds_get_packet(ds, start);
    sh_audio_t *sh_audio = ds->sh;
    PLAYBACK_INTERNAL_T *pbi =
        (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    pbi->audio.is_secure = ds->is_secure ? MT_TRUE : MT_FALSE;
    *extra_size = 0;
    if (p_file_seq->m_audio_codec_type == HA_AUDIO_ID_PCM) {
        unsigned char *p_fix_start = NULL;
        if (sh_audio == NULL) {
            return in_size;
        }

        int channel = sh_audio->channels;
        p_fix_start = *start;

        if (pbi->audio.codec_id == aAUDIO_PCM_BR) {
             if (p_fix_start && in_size > DEFAULT_FRAME_HEADER_BYTES)
            {
                *start = (unsigned char *)((uintptr_t)p_fix_start + DEFAULT_FRAME_HEADER_BYTES);
                in_size = in_size - DEFAULT_FRAME_HEADER_BYTES;
            }
			return in_size;
        } else if (channel > 2) {
            if (8 == channel || 6 == channel || 4 == channel) {

				return in_size;
            } else {
                MLOGI("%s NORMAL PCM get data error ch %d\n", __func__, channel);
            }
        }
        return in_size;
    }

    if (sh_audio == NULL || (DEMUXER_TYPE_MPEG_TS == (ds->demuxer)->type)) {
        return in_size;
    }

    if (in_size > 0) {
        if (sh_audio->format == MKTAG('M', 'P', '4', 'A') || sh_audio->format == MKTAG('m', 'p', '4', 'a')) {
            unsigned char *p = *start;

            //hls doesnt needed
            if (ds->demuxer->type == DEMUXER_TYPE_LAVF) {
                unsigned long *priv = (unsigned long *)(ds->demuxer->priv);
                AVInputFormat *avif = (AVInputFormat *)(*priv);

                if (strstr(avif->name, "hls") || strstr(avif->name, "mondash") || pbi->audio.is_secure) {
                    return in_size;
                }
            }

            if (p && p[0] == 0xFF && ((p[1] & 0xF0) == 0xF0)) {
                //there are some pkts all data is 0xff, so parser head
                if (adts_UnpackFrmSize(p, &in_size) == 0) {
                    return in_size;
                }
            }

            uint8_t aac_buf[MP4A_EXTRA_HEAD_LENGTH] = { 0xff, 0xf1, 0x40, 0x00, 0x00, 0x1f, 0xfc };
            unsigned int num_data_block = (unsigned int)in_size / 1024;
            uint16_t frame_Length;
            int i = 0;
            int aud_sample_rate = sh_audio->samplerate;
            int aud_channels = sh_audio->channels;
            const int aac_sample_rates[AAC_SAMPLE_RATE_TABLE_LEN] = {
                96000, 88200, 64000, 48000, 44100, 32000,
                24000, 22050, 16000, 12000, 11025, 8000 , 7350
            };

            memset(g_aac_head_mp, 0, MP4A_EXTRA_HEAD_BUF_BYTES);
            *extra_size = MP4A_EXTRA_HEAD_LENGTH;
            *extra_buf  = g_aac_head_mp;
            if (!aud_sample_rate && sh_audio->wf) {
                aud_sample_rate = (int)sh_audio->wf->nSamplesPerSec;
                aud_channels = sh_audio->wf->nChannels;
            }

            for (i = 0; i < AAC_SAMPLE_RATE_TABLE_LEN; i++) {
                if (aud_sample_rate == aac_sample_rates[i]) {
                    break;
                }
            }

            frame_Length = in_size + MP4A_EXTRA_HEAD_LENGTH;
            /* frame size over last 2 bits */
            aac_buf[2] |= ((i & 0xf) << 2);
            aac_buf[3] |= (aud_channels << 6);

            if (aud_channels > 3) {
                aac_buf[2] |= (aud_channels >> 2);
            }

            aac_buf[3] |= (frame_Length & 0x1800) >> 11; // the upper 2 bit
            /* frame size continued over full byte */
            aac_buf[4] = (frame_Length & 0x1FF8) >> 3;   // the middle 8 bit
            /* frame size continued first 3 bits */
            aac_buf[5] |= (frame_Length & 0x7) << 5;     // the last 3 bit
            aac_buf[6] |= num_data_block & 0x03;         // Set raw Data blocks.
            if (*extra_buf) {
                memcpy(*extra_buf, aac_buf, (unsigned int)*extra_size);
            }
        }
    }
    return in_size;
}

int ds_packet_seekforward(demux_stream_t *ds, int speed_cntl)
{
    static int key_sel = 0;
    int ret = 0;

    if (key_sel >= speed_cntl) {
        key_sel = 0;
    }

    if (ds->flags) {
        key_sel++;
    }

    if ((key_sel == 1)) {
        ret = 1;
    }

    return ret;
}

#ifdef CFG_ENABLE_FFMPEG_422
static void ffmpeg_ext_cmd_set_para(PLAYER_FFMPEG_CTRL_T *ptr, int cmd, void *val)
{
    switch (cmd) {
        case MP_STATE_EXT:
            ptr->player_exit = (int) ((intptr_t) val);
            break;
        case MP_FFMPEG_MOV_KEYGEN:
            av_set_mov_decryption_key((char *)val);
            break;
        case MP_FFMPEG_MOV_KEYGEN_LEN:
            av_set_mov_decryption_key_len((int)(intptr_t)val);
            break;
        case MP_FFMPEG_SET_FILEPLAY_SPEED: {
            int speed = (int) ((intptr_t) val);
            if (speed == 0 || speed == 1) {
                ptr->speed_index = 0;
            } else if (speed > 1) {
                ptr->speed_index = 1;
            } else if (speed < 0) {
                ptr->speed_index = -1;
            } else {
                ptr->speed_index = 0;
            }
            break;
        }
        case MP_FFMPEG_SET_FAKE_MPD: {
            DASH_FAKE_MPD *mpd = (DASH_FAKE_MPD *)val;
            if (mpd) {
                av_dash_set_fake_mpd(mpd->url, mpd->mpd_str);
            }
            break;
        }
        default:
            break;
    }
}

static void ffmpeg_ext_cmd_set_avfmt(int cmd, void *val)
{
    switch (cmd) {
        case MP_FFMPEG_MOV_SEG_NUM : {
            if (!av_dict_get(ff_avformat_opts,
                "max_opti_index_entries", NULL, AV_DICT_MATCH_CASE)) {
                av_dict_set_int(&ff_avformat_opts,
                    "max_opti_index_entries", *((int *)val), AV_DICT_DONT_OVERWRITE);
            }
            break;
        }
        case MP_DEINIT_AVFMT : {
            av_dict_free(&ff_avformat_opts);
            ff_avformat_opts = NULL;
            break;
        }
        default :
            break;
    }
}
#endif

void mp_ffmpeg_ext_cmd(int cmd, int opt, void *val)
{
    MLOGD("[%s_%d]cmd[%d],opt[%d]\n", __func__, __LINE__, cmd, opt);
#ifdef CFG_ENABLE_FFMPEG_422
    switch (opt) {
        case MP_DO_CALLBACK: //do callback cmd
            switch (cmd) {
                case MP_INIT_FFMPEG_MEM:
                    av_init_priv_data_mem();
                    break;
                case MP_DEINIT_FFMPEG_MEM:
                    av_deinit_priv_data_mem(1);
                    break;
                default:
                    break;
            }
            break;
        case MP_DO_SET_PARAM: { //set param to ffmpeg
            PLAYER_FFMPEG_CTRL_T *ptr =
                (PLAYER_FFMPEG_CTRL_T *) av_get_priv_data_mem_ptr();
            if (ptr == NULL) {
                MLOGI("[%s_%d] set param fail, cmd=%d\n", __func__, __LINE__, cmd);
                break;
            }
            ffmpeg_ext_cmd_set_para(ptr, cmd, val);
            break;
        }
        case MP_DO_SET_AVFMT: {
            ffmpeg_ext_cmd_set_avfmt(cmd, val);
            break;
        }
        case MP_DO_GET_PARAM: //get param from ffmpeg
            break;
        default:
            break;
    }
#else
#endif
}

void mp_ffmpeg422_get_drm_index(int *index1, int *index2)
{
    *index1 = -1;
    *index2 = -1;
//#ifdef CFG_ENABLE_FFMPEG_422
    PLAYER_FFMPEG_CTRL_T *ptr =
                (PLAYER_FFMPEG_CTRL_T *) av_get_priv_data_mem_ptr();
    *index1 = ptr->drm_eDrmIndex1;
    *index2 = ptr->drm_eDrmIndex2;
//#endif
}

void mp_ffmpeg422_clear_drm_index(void) {
    av_drm_set_index_info(0, 0);
}
