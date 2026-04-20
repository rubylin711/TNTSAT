/*
 * MPEG-ES video parser
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdint.h>
#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#else
#include "mp_func_trans.h"
#endif
#include <unistd.h>
#include "mutil.h"
#define MODULE_TAG "PASER"
#include "mlog.h"
#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"

#include "av_helpers.h"
#include "stream/stream.h"
#include "demuxer.h"
#include "parse_es.h"
#include "libavutil/mathematics.h"

#define SET_UBIT_N(val, n)   ((val) |=  ((unsigned int) 0x1 << (n)))
#define CLEAR_UBIT_N(val, n) ((val) &= ~((unsigned int) 0x1 << (n)))

//static unsigned char videobuffer[MAX_VIDEO_PACKET_SIZE];
unsigned char *videobuffer = NULL;
int videobuf_len = 0;
int next_nal = -1;
///! legacy variable, 4 if stream is synced, 0 if not
int videobuf_code_len = 0;

#define MAX_SYNCLEN (10 * 1024 * 1024)
// sync video stream, and returns next packet code
int sync_video_packet(demux_stream_t *ds,int *start_len)
{
    if(start_len != NULL) {
        *start_len = 0;
    }
    int skipped = 0;

    if (!videobuf_code_len) {
        if (!demux_pattern_3(ds, NULL, MAX_SYNCLEN, &skipped, 0x100)) {
            if (skipped == MAX_SYNCLEN) {
                mp_msg(MSGT_DEMUXER, MSGL_ERR, "parse_es: could not sync video stream!\n");
            }
            goto eof_out;
        }
        next_nal = demux_getc(ds);
        if (next_nal < 0) {
            goto eof_out;
        }
        videobuf_code_len = 4;
        if (skipped) {
            mp_dbg(MSGT_PARSEES, MSGL_DBG2, "videobuf: %d bytes skipped  (next: 0x1%02X)\n", skipped, next_nal);
        }
        if(start_len != NULL) {
            *start_len = skipped + 1;
        }
    }

    if(start_len != NULL && skipped == 0) {
        *start_len = 4;
    }

    return 0x100 | next_nal;
eof_out:
    next_nal = -1;
    videobuf_code_len = 0;
    return 0;
}

// return: packet length
int read_video_packet(demux_stream_t *ds)
{
    int packet_start;
    int res, read;

    if (VIDEOBUFFER_SIZE - videobuf_len < 5) {
        return 0;
    }
    // SYNC STREAM
//  if(!sync_video_packet(ds ,NULL)) return 0; // cannot sync (EOF)

    // COPY STARTCODE:
    packet_start = videobuf_len;
    videobuffer[videobuf_len + 0] = 0;
    videobuffer[videobuf_len + 1] = 0;
    videobuffer[videobuf_len + 2] = 1;
    videobuffer[videobuf_len + 3] = next_nal;
    videobuf_len += 4;

    // READ PACKET:
    res = demux_pattern_3(ds, &videobuffer[videobuf_len],
                          VIDEOBUFFER_SIZE - videobuf_len, &read, 0x100);
    videobuf_len += read;
    if (!res) {
        goto eof_out;
    }

    videobuf_len -= 3;

    mp_dbg(MSGT_PARSEES, MSGL_DBG2, "videobuf: packet 0x1%02X  len=%d  (total=%d)\n",
        videobuffer[packet_start + 3], videobuf_len - packet_start, videobuf_len);

    // Save next packet code:
    next_nal = demux_getc(ds);
    if (next_nal < 0) {
        goto eof_out;
    }
    videobuf_code_len = 4;

    return videobuf_len - packet_start;

eof_out:
    next_nal = -1;
    videobuf_code_len = 0;
    return videobuf_len - packet_start;
}

// return: next packet code
int skip_video_packet(demux_stream_t *ds)
{

    // SYNC STREAM
//  if(!sync_video_packet(ds ,NULL)) return 0; // cannot sync (EOF)

    videobuf_code_len = 0; // force resync

    // SYNC AGAIN:
    return sync_video_packet(ds,NULL);
}

/* stripped down version of a52_syncinfo() from liba52
 * copyright belongs to Michel Lespinasse <walken@zoy.org>
 * and Aaron Holtzman <aholtzma@ess.engr.uvic.ca> */
int mp_a52_framesize(uint8_t *buf, int *srate)
{
    int rate[] = {  32,  40,  48,  56,  64,  80,  96, 112,
                    128, 160, 192, 224, 256, 320, 384, 448,
                    512, 576, 640
                 };
    uint8_t halfrate[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3 };
    int frmsizecod, bitrate, half;

    if ((buf[0] != 0x0b) || (buf[1] != 0x77)) {  /* syncword */
        return 0;
    }

    if (buf[5] >= 0x60) {                        /* bsid >= 12 */
        return 0;
    }

    half = halfrate[buf[5] >> 3];

    frmsizecod = buf[4] & 63;
    if (frmsizecod >= 38) {
        return 0;
    }

    bitrate = rate[frmsizecod >> 1];

    switch (buf[4] & 0xc0) {
        case 0:    /* 48 KHz */
            *srate = 48000 >> half;
            return 4 * bitrate;
        case 0x40: /* 44.1 KHz */
            *srate = 44100 >> half;
            return 2 * (320 * bitrate / 147 + (frmsizecod & 1));
        case 0x80: /* 32 KHz */
            *srate = 32000 >> half;
            return 6 * bitrate;
    }

    return 0;
}

int find_mpeg12_frame_start(unsigned char *buf,
    int len, int *start_code_offset, int *parsed_data_offset)
{
    int idx;
    int got_vseq = MT_FALSE;
    unsigned int start_code = 0xFFFFffff;

    *start_code_offset = *parsed_data_offset = MPEG2_NO_START_CODE_VAL;
    for (idx = 0; idx < len; idx++) {
        start_code = (start_code << 8) | buf[idx];
        if (MPEG2_VIDEO_SEQ_START_CODE == start_code) {
            got_vseq = MT_TRUE;
            *start_code_offset = idx - DEFAULT_FRAME_HEADER_BYTES + 1;
        }
        if (MPEG2_PICTURE_START_CODE == start_code) {
            if (MT_FALSE == got_vseq) {
                *start_code_offset = idx - DEFAULT_FRAME_HEADER_BYTES + 1;
            }

            *parsed_data_offset = idx - DEFAULT_FRAME_HEADER_BYTES + 1;
            return MT_TRUE;
        }
    }
    /* index of last parsed data in unit of DEFAULT_FRAME_HEADER_BYTES */
    *parsed_data_offset = idx - DEFAULT_FRAME_HEADER_BYTES;
    return MT_FALSE;
}

static int covert_to_ff_codec_id(unsigned int format)
{
    int idx;
    static const struct {
        unsigned int our;
        unsigned int your;
    } type_list[] = {
        {VIDEO_MPEG1_MP   , AV_CODEC_ID_MPEG1VIDEO},
        {VIDEO_MPEG2_MP   , AV_CODEC_ID_MPEG2VIDEO},
        {VIDEO_MPEG4_MP   , AV_CODEC_ID_MPEG4     },
        {VIDEO_H264_MP    , AV_CODEC_ID_H264      },
        {VIDEO_AVS_MP     , AV_CODEC_ID_CAVS      },
        {VIDEO_AVC_MP     , AV_CODEC_ID_NONE},
        {VIDEO_DIRAC_MP   , AV_CODEC_ID_NONE},
        {VIDEO_HEVC_MP    , AV_CODEC_ID_NONE},
        {VIDEO_VC1_MP     , AV_CODEC_ID_NONE},
        {AUDIO_MP2_MP     , AV_CODEC_ID_NONE},
        {AUDIO_A52_MP     , AV_CODEC_ID_NONE},
        {AUDIO_DTS_MP     , AV_CODEC_ID_NONE},
        {AUDIO_LPCM_BE_MP , AV_CODEC_ID_NONE},
        {AUDIO_AAC_MP     , AV_CODEC_ID_NONE},
        {AUDIO_AAC_LATM_MP, AV_CODEC_ID_NONE},
        {AUDIO_TRUEHD_MP  , AV_CODEC_ID_NONE},
        {AUDIO_TRUEHD_AC3_MP, AV_CODEC_ID_AC3},
        {AUDIO_S302M_MP   , AV_CODEC_ID_NONE},
        {AUDIO_PCM_BR_MP  , AV_CODEC_ID_NONE},
        {SPU_DVD_MP       , AV_CODEC_ID_NONE},
        {SPU_DVB_MP       , AV_CODEC_ID_NONE},
        {SPU_TELETEXT_MP  , AV_CODEC_ID_NONE},
        {SPU_PGS_MP       , AV_CODEC_ID_NONE},
        {PES_PRIVATE1_MP  , AV_CODEC_ID_NONE},
        {SL_PES_STREAM_MP , AV_CODEC_ID_NONE},
        {SL_SECTION_MP    , AV_CODEC_ID_NONE},
        {MP4_OD_MP        , AV_CODEC_ID_NONE},
    };

    for (idx = 0; idx < ARRAY_CNT(type_list); idx++) {
        if (format == type_list[idx].our) {
            return type_list[idx].your;
        }
    }

    return AV_CODEC_ID_NONE;
}

static int es_need_parse(es_stream_type_t type)
{
    int idx;
    static const struct {
        unsigned int es_type;
        unsigned int need_parse;
    } parse_list[] = {
        {VIDEO_MPEG1_MP   , MT_TRUE },
        {VIDEO_MPEG2_MP   , MT_TRUE },
        {VIDEO_MPEG4_MP   , MT_TRUE },
        {VIDEO_H264_MP    , MT_TRUE },
        {VIDEO_AVS_MP     , MT_TRUE },
        {VIDEO_AVC_MP     , MT_FALSE},
        {VIDEO_DIRAC_MP   , MT_FALSE},
        {VIDEO_HEVC_MP    , MT_FALSE},
        {VIDEO_VC1_MP     , MT_FALSE},
        {AUDIO_MP2_MP     , MT_FALSE},
        {AUDIO_A52_MP     , MT_FALSE},
        {AUDIO_DTS_MP     , MT_FALSE},
        {AUDIO_LPCM_BE_MP , MT_FALSE},
        {AUDIO_AAC_MP     , MT_FALSE},
        {AUDIO_AAC_LATM_MP, MT_FALSE},
        {AUDIO_TRUEHD_MP  , MT_FALSE},
        {AUDIO_TRUEHD_AC3_MP, MT_FALSE},
        {AUDIO_S302M_MP   , MT_FALSE},
        {AUDIO_PCM_BR_MP  , MT_FALSE},
        {SPU_DVD_MP       , MT_FALSE},
        {SPU_DVB_MP       , MT_FALSE},
        {SPU_TELETEXT_MP  , MT_FALSE},
        {SPU_PGS_MP       , MT_FALSE},
        {PES_PRIVATE1_MP  , MT_FALSE},
        {SL_PES_STREAM_MP , MT_FALSE},
        {SL_SECTION_MP    , MT_FALSE},
        {MP4_OD_MP        , MT_FALSE},
    };

    for (idx = 0; idx < ARRAY_CNT(parse_list); idx++) {
        if (type == parse_list[idx].es_type) {
            return parse_list[idx].need_parse;
        }
    }
    return MT_FALSE;
}

static inline int parser_is_opened(
    unsigned int opened, es_stream_type_t type)
{
    return ((opened >> type) & 0x01);
}

static struct parser_ctx *get_parser_ctx(
    parser_ctx_handle_t *handle, int es_type)
{
    if (es_type >= ES_TYPE_IS_MAX) {
        return NULL;
    }

    return &(handle->ctx[es_type]);
}

static int get_es_type_by_stream(
    demuxer_t *demuxer, demux_stream_t *ds)
{
    int idx;
    struct {
        int type;
        demux_stream_t *ds;
    } es_table[ES_TYPE_IS_MAX] = {
        {ES_TYPE_IS_AUD, demuxer->audio},
        {ES_TYPE_IS_VID, demuxer->video},
        {ES_TYPE_IS_SUB, demuxer->sub},
    };

    for (idx = 0; idx < ES_TYPE_IS_MAX; idx++) {
        if (ds == es_table[idx].ds) {
            return es_table[idx].type;
        }
    }

    return ES_TYPE_IS_MAX;
}

static void av_parse_add_pkt(
    demux_packet_t **head, demux_packet_t *dp)
{
    demux_packet_t **list = head;

    while (*list != NULL) {
        list = &((*list)->next);
    }

    *list = dp;
}

/* some codec parser need avctx information */
static void copy_prop_to_avctx(demuxer_t *demuxer,
    demux_stream_t *ds, AVCodecContext *avctx)
{
    (void) demuxer;
    (void) ds;
    (void) avctx;
}

static void init_parse_para(demux_packet_t *dp_in, uint8_t **es_buf,
    int *es_size, int64_t *es_pts, int64_t *es_dts, int64_t *es_pos, int *flags)
{
    if (NULL == dp_in) {
        *flags   = 0;
        *es_pos  = -1;
        *es_size = 0;
        *es_buf  = NULL;
        *es_pts  =
        *es_dts  = AV_NOPTS_VALUE;
    } else {
        *flags   = dp_in->flags;
        *es_pos  = dp_in->pos;
        *es_size = dp_in->len;
        *es_buf  = dp_in->buffer;
        /* second to microsecond */
        *es_pts  = (MP_NOPTS_VALUE == dp_in->pts) ? AV_NOPTS_VALUE : (int64_t) (dp_in->pts * SECOND_TO_MICROSECOND_BASE);
        *es_dts  = (MP_NOPTS_VALUE == dp_in->dts) ? AV_NOPTS_VALUE : dp_in->dts;
    }
}

static int close_parser(demuxer_t *demuxer,
    demux_stream_t *ds, parser_ctx_handle_t *handle)
{
    int ill_para, es_type;
    struct parser_ctx *pc_ctx = NULL;

    ill_para = (NULL == demuxer || NULL == ds || NULL == handle);
    if (ill_para) {
        MLOGW("%s para error\n", __FUNCTION__);
        return MT_FAILURE;
    }

    es_type = get_es_type_by_stream(demuxer, ds);
    if (es_type >= ES_TYPE_IS_MAX) {
        MLOGW("%s es type:%d error\n", __FUNCTION__, es_type);
        return MT_FAILURE;
    }

    pc_ctx = get_parser_ctx(handle, es_type);
    if (NULL == pc_ctx || NULL == pc_ctx->pctx) {
        return MT_FAILURE;
    }

    av_parser_close(pc_ctx->pctx);
    pc_ctx->pctx = NULL;

    CLEAR_UBIT_N(handle->parser_opened, es_type);
    return MT_SUCCESS;
}

#define RELATIVE_TS_BASE (INT64_MAX - (1LL<<48))
static int is_relative_pts(int64_t ts)
{
    return ts > (RELATIVE_TS_BASE - (1LL << 48));
}

static AVRational recompute_pkt_duration(AVStream *st,
    AVCodecContext *avctx, AVPacket *pkt)
{
    int num = 0;
    int den = 0;
    int64_t cur_dts = RELATIVE_TS_BASE;

    if (0 == pkt->duration) {
        if (st->r_frame_rate.num) {
            num = st->r_frame_rate.den;
            den = st->r_frame_rate.num;
#ifdef CFG_ENABLE_FFMPEG_422
        } else if (avctx->framerate.num) {
            num = avctx->framerate.den;
            den = avctx->framerate.num;
#else
        } else if (avctx->time_base.num * 1000LL > avctx->time_base.den) {
            den = avctx->time_base.den;
            num = avctx->time_base.num;
#endif
        }
    }

    if (den && num) {
        pkt->duration = av_rescale_rnd(1,
           num * (int64_t) st->time_base.den,
           den * (int64_t) st->time_base.num, AV_ROUND_DOWN);
    }

    AVRational duration = (AVRational) {num, den};
    if (0 == pkt->duration) {
        return duration;
    }

    if (st->first_dts != AV_NOPTS_VALUE) {
        cur_dts = st->first_dts;
    } else if (st->cur_dts != RELATIVE_TS_BASE) {
        return duration;
    }

    if ((pkt->pts == pkt->dts ||
         pkt->pts == AV_NOPTS_VALUE) &&
        (pkt->dts == AV_NOPTS_VALUE ||
         pkt->dts == st->first_dts ||
         pkt->dts == RELATIVE_TS_BASE) &&
        !pkt->duration) {
        pkt->dts = cur_dts;
        if (!avctx->has_b_frames) {
            pkt->pts = cur_dts;
        }
    }
    if (AV_NOPTS_VALUE != st->first_dts && AV_NOPTS_VALUE != pkt->dts) {
        st->cur_dts = pkt->dts + pkt->duration;
    }
    return duration;
}

static void update_start_timestamps(
    AVStream *st, int64_t dts, int64_t pts, AVPacket *pkt)
{
    uint64_t shift;

    if (st->first_dts != AV_NOPTS_VALUE ||
        dts           == AV_NOPTS_VALUE ||
        st->cur_dts   == AV_NOPTS_VALUE ||
        st->cur_dts < INT_MIN + RELATIVE_TS_BASE ||
        is_relative_pts(dts)) {
        return;
    }

    st->first_dts = dts - (st->cur_dts - RELATIVE_TS_BASE);
    st->cur_dts   = dts;
    shift         = (uint64_t)st->first_dts - RELATIVE_TS_BASE;

    if (is_relative_pts(pts)) {
        pts += shift;
    }

    if (is_relative_pts(pkt->pts)) {
        pkt->pts += shift;
    }

    if (is_relative_pts(pkt->dts)) {
        pkt->dts += shift;
    }

    if (st->start_time == AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE) {
        st->start_time = pkt->pts;
    }

    if (st->start_time == AV_NOPTS_VALUE) {
        st->start_time = pts;
    }
}

static int64_t inc_timestamps(AVRational ts_tb, int64_t ts, AVRational inc_tb, int64_t inc)
{
#ifdef CFG_ENABLE_FFMPEG_422
    return av_add_stable(ts_tb, ts, inc_tb, inc);
#else
    return ts + av_rescale_rnd(1, inc_tb.num * (int64_t)ts_tb.den,
        inc_tb.den * (int64_t)ts_tb.num, AV_ROUND_DOWN);
#endif
}

static void recompute_pkt_pts(struct parser_ctx *pc_ctx,
    AVPacket *pkt, AVRational duration, int64_t next_pts, int64_t next_dts)
{
    int i;
    AVStream             *st = pc_ctx->st;
    AVCodecContext    *avctx = pc_ctx->avctx;
    AVCodecParserContext *pc = pc_ctx->pctx;
    int presentation_delayed = avctx->has_b_frames && AV_PICTURE_TYPE_B != pc->pict_type;

    if (presentation_delayed) {
        /* DTS = decompression timestamp */
        /* PTS = presentation timestamp */
        if (pkt->dts == AV_NOPTS_VALUE) {
            pkt->dts = st->last_IP_pts;
        }
        update_start_timestamps(st, pkt->dts, pkt->pts, pkt);
        if (pkt->dts == AV_NOPTS_VALUE) {
            pkt->dts = st->cur_dts;
        }

        /* This is tricky: the dts must be incremented by the duration
         * of the frame we are displaying, i.e. the last I- or P-frame. */
        if (st->last_IP_duration == 0 && (uint64_t)pkt->duration <= INT32_MAX) {
            st->last_IP_duration = pkt->duration;
        }
        if (pkt->dts != AV_NOPTS_VALUE) {
            st->cur_dts = pkt->dts + st->last_IP_duration;
        }
        if (pkt->dts != AV_NOPTS_VALUE &&
            pkt->pts == AV_NOPTS_VALUE &&
            st->last_IP_duration > 0 &&
            ((uint64_t)st->cur_dts - (uint64_t)next_dts + 1) <= 2 &&
            next_dts != next_pts &&
            next_pts != AV_NOPTS_VALUE) {
            pkt->pts = next_dts;
        }

        if ((uint64_t)pkt->duration <= INT32_MAX) {
            st->last_IP_duration = pkt->duration;
        }
        st->last_IP_pts      = pkt->pts;
        /* Cannot compute PTS if not present (we can compute it only
         * by knowing the future. */
    } else if (pkt->pts != AV_NOPTS_VALUE ||
               pkt->dts != AV_NOPTS_VALUE ||
               pkt->duration) {

        /* presentation is not delayed : PTS and DTS are the same */
        if (pkt->pts == AV_NOPTS_VALUE) {
            pkt->pts = pkt->dts;
        }
        update_start_timestamps(st, pkt->pts, pkt->pts, pkt);
        if (pkt->pts == AV_NOPTS_VALUE) {
            pkt->pts = st->cur_dts;
        }
        pkt->dts = pkt->pts;
        if (pkt->pts != AV_NOPTS_VALUE) {
            st->cur_dts = inc_timestamps(st->time_base, pkt->pts, duration, 1);
        }
    }

    if (pkt->pts != AV_NOPTS_VALUE && avctx->has_b_frames <= MAX_REORDER_DELAY) {
        st->pts_buffer[0] = pkt->pts;
        for (i = 0; i< avctx->has_b_frames && st->pts_buffer[i] > st->pts_buffer[i + 1]; i++) {
            FFSWAP(int64_t, st->pts_buffer[i], st->pts_buffer[i + 1]);
        }

        pkt->dts = st->pts_buffer[0];
    }
    if (pkt->dts > st->cur_dts) {
        st->cur_dts = pkt->dts;
    }
}
/* can only handle video */
static void recompute_pkt_time_info(AVFormatContext *s,
    struct parser_ctx *pc_ctx, AVPacket *pkt, int64_t in_pts, int64_t in_dts)
{
    AVStream             *st = pc_ctx->st;
    AVCodecContext    *avctx = pc_ctx->avctx;
    AVRational       user_tb = (AVRational) {1, SECOND_TO_MICROSECOND_BASE};

    int need_cacal = AV_CODEC_ID_MPEG2VIDEO == pc_ctx->codec_id;
    if (!need_cacal) {
        return;
    }

    /* convert timestamp to timebased */
    if (AV_NOPTS_VALUE != pkt->pts) {
        pkt->pts = av_rescale_q(pkt->pts, user_tb, st->time_base);
    }

    if (AV_NOPTS_VALUE != pkt->dts) {
        pkt->dts = av_rescale_q(pkt->dts, user_tb, st->time_base);
    }

    AVRational duration = recompute_pkt_duration(st, avctx, pkt);
    recompute_pkt_pts(pc_ctx, pkt, duration, in_pts, in_dts);
    /* convert timebased timestamp to microsecod */
    if (AV_NOPTS_VALUE != pkt->pts) {
        pkt->pts = av_rescale_q(pkt->pts, st->time_base, user_tb);
    }

    if (AV_NOPTS_VALUE != pkt->dts) {
        pkt->dts = av_rescale_q(pkt->dts, st->time_base, user_tb);
    }
}

static void set_dp_fileds(AVFormatContext *s,
    struct parser_ctx *pc_ctx, demux_packet_t *dp, int64_t in_pts, int64_t in_dts)
{
    AVPacket out_pkt = {0};

    out_pkt.stream_index = pc_ctx->st->index;
    out_pkt.pts          = pc_ctx->pctx->pts;
    out_pkt.dts          = pc_ctx->pctx->dts;
    out_pkt.pos          = pc_ctx->pctx->pos;

    // recompute_pkt_time_info(s, pc_ctx, &out_pkt, in_pts, in_dts);
    if (AV_NOPTS_VALUE != out_pkt.pts) {
        dp->pts = ((double) out_pkt.pts) / (SECOND_TO_MICROSECOND_BASE);
    } else {
        dp->pts = MP_NOPTS_VALUE;
    }

    dp->dts = (AV_NOPTS_VALUE == out_pkt.dts) ? MP_NOPTS_VALUE : out_pkt.dts;
    dp->pos = (off_t)(out_pkt.pos);
}

static demux_packet_t *av_parse(AVFormatContext *s, struct parser_ctx *pc_ctx,
    uint8_t *buf, int buf_size, int64_t in_pts, int64_t in_dts, int64_t pos, int flags)
{
    int len;
    demux_packet_t *dp, *dp_head;
    int      out_size  = 0;
    int64_t pts        = in_pts;
    int64_t dts        = in_dts;
    uint8_t *out_buff  = NULL;
    uint8_t *es_data   = buf;
    int      es_size   = (NULL == buf) ? 0 : buf_size;
    int got_output     = (NULL == buf) ? 1 : 0;
    AVCodecParserContext *pctx = pc_ctx->pctx;

    dp = dp_head = NULL;
    while (es_size > 0 || (buf == NULL && got_output)) {
        out_buff = NULL;
        out_size = 0;

        len = av_parser_parse2(pctx, pc_ctx->avctx,
            &out_buff, &out_size, es_data, es_size, pts, dts, pos);
        /* increment read pointer */
        es_data += len;
        es_size -= len;

        got_output = !!out_size;
        if (!out_size) {
            continue;
        }

        if (NULL == (dp = new_demux_packet(out_size))) {
            return dp_head;
        }

        set_dp_fileds(s, pc_ctx, dp, pts, dts);
        pts = dts = AV_NOPTS_VALUE;
        pos = -1;

        if (pctx->key_frame == 1 ||
            (pctx->key_frame == -1 && pctx->pict_type == AV_PICTURE_TYPE_I)) {
            dp->flags |= AV_PKT_FLAG_KEY;
        }

        if (pctx->key_frame == -1 &&
            pctx->pict_type == AV_PICTURE_TYPE_NONE && (flags)) {
            dp->flags |= AV_PKT_FLAG_KEY;
        }

        memcpy(dp->buffer, out_buff, out_size);
        av_parse_add_pkt(&dp_head, dp);
    }

    return dp_head;
}

static demux_packet_t *parse_av_packet(
    demuxer_t *demuxer, demux_stream_t *ds,
    parser_ctx_handle_t *handle, demux_packet_t *dp_in)
{
    int flags;
    int ill_para, es_size, es_type;
    int64_t es_pts, es_dts, es_pos;
    uint8_t *es_buf             = NULL;
    demux_packet_t *dp_head     = NULL;
    struct parser_ctx *pc_ctx   = NULL;

    ill_para = (NULL == demuxer || NULL == ds || NULL == ds->sh || NULL == handle);
    if (ill_para) {
        MLOGW("%s para error\n", __FUNCTION__);
        return dp_in;
    }

    es_type = get_es_type_by_stream(demuxer, ds);
    if (es_type >= ES_TYPE_IS_MAX ||
        !parser_is_opened(handle->parser_opened, es_type)) {
        MLOGA("%s es type:%d error\n", __FUNCTION__, es_type);
        return dp_in;
    }

    pc_ctx = get_parser_ctx(handle, es_type);
    /* need parse case *pctx will null and return here! */
    ill_para = (NULL == pc_ctx || NULL == pc_ctx->pctx);
    if (ill_para) {
        return dp_in;
    }

    init_parse_para(dp_in, &es_buf,
        &es_size, &es_pts, &es_dts, &es_pos, &flags);
    copy_prop_to_avctx(demuxer, ds, pc_ctx->avctx);

    dp_head = av_parse(handle->fmt_ctx, pc_ctx,
        es_buf, es_size, es_pts, es_dts, es_pos, flags);
    if (NULL == dp_in) {
        close_parser(demuxer, ds, handle);
    }
    return dp_head;
}

static int init_parser(demuxer_t *demuxer,
    demux_stream_t *ds, parser_ctx_handle_t *handle)
{
    unsigned int format;
    int ill_para, es_type;
    struct parser_ctx *pc_ctx = NULL;

    ill_para = (NULL == demuxer || NULL == ds || NULL == ds->sh || NULL == handle);
    if (ill_para) {
        MLOGA("%s para error\n", __FUNCTION__);
        return MT_FAILURE;
    }

    es_type = get_es_type_by_stream(demuxer, ds);
    if (es_type >= ES_TYPE_IS_MAX) {
        MLOGW("%s es type:%d error\n", __FUNCTION__, es_type);
        return MT_FAILURE;
    }

    pc_ctx = get_parser_ctx(handle, es_type);
    if (NULL == pc_ctx) {
        return MT_FAILURE;
    }

    if (pc_ctx->pctx) {
        av_parser_close(pc_ctx->pctx);
    }

    format       = ((sh_common_t *)(ds->sh))->format;
    pc_ctx->pctx = NULL;
    if (MT_FALSE == es_need_parse(format)) {
        return MT_FAILURE;
    }

    pc_ctx->codec_id = covert_to_ff_codec_id(format);
    pc_ctx->pctx     = av_parser_init(pc_ctx->codec_id);
    if (NULL == pc_ctx->pctx) {
        return MT_FAILURE;
    }

    SET_UBIT_N(handle->parser_opened, es_type);
    return MT_SUCCESS;
}

static int open_parser_all(
    demuxer_t *demuxer, parser_ctx_handle_t *handle)
{
    int ret;

    ret = init_parser(demuxer, demuxer->audio, handle);
    if (MT_SUCCESS != ret) {
        CLEAR_UBIT_N(handle->parser_opened, ES_TYPE_IS_AUD);
    }

    ret = init_parser(demuxer, demuxer->video, handle);
    if (MT_SUCCESS != ret) {
        CLEAR_UBIT_N(handle->parser_opened, ES_TYPE_IS_VID);
    }

    ret = init_parser(demuxer, demuxer->sub, handle);
    if (MT_SUCCESS != ret) {
        CLEAR_UBIT_N(handle->parser_opened, ES_TYPE_IS_SUB);
    }

    return MT_SUCCESS;
}

static void close_parser_all(
    demuxer_t *demuxer, parser_ctx_handle_t *handle)
{
    (void) close_parser(demuxer, demuxer->audio, handle);
    (void) close_parser(demuxer, demuxer->video, handle);
    (void) close_parser(demuxer, demuxer->sub  , handle);
}

int parse_es_open_parser_all(
    demuxer_t *demuxer, parser_ctx_handle_t *handle)
{
    int idx, ret;

    init_avformat();
    if (NULL == demuxer || NULL == handle) {
        MLOGW("%s para error\n", __FUNCTION__);
        return MT_FAILURE;
    }

    if (!(handle->fmt_ctx = avformat_alloc_context())) {
        return MT_FAILURE;
    }

    ret = open_parser_all(demuxer, handle);
    if (MT_SUCCESS != ret) {
        return ret;
    }

    for (idx = 0; idx < ES_TYPE_IS_MAX; idx++) {
        if ((NULL == handle->ctx[idx].avctx) &&
            ((handle->parser_opened >> idx) & 0x01)) {
            handle->ctx[idx].st = avformat_new_stream(handle->fmt_ctx, NULL);
            handle->ctx[idx].avctx = avcodec_alloc_context3(NULL);
            if (NULL == handle->ctx[idx].st ||
                NULL == handle->ctx[idx].avctx) {
                return MT_FAILURE;
            }
            /* for mpeg2ts and mpeg2ps */
            handle->ctx[idx].st->time_base = (AVRational) {1, 90000};
            handle->ctx[idx].st->cur_dts   = RELATIVE_TS_BASE;
        }
    }
    MLOGI("Open parser:%d success!\n", handle->parser_opened);
    return MT_SUCCESS;
}

void parse_es_close_parser_all(
    demuxer_t *demuxer, parser_ctx_handle_t *handle)
{
    int idx;
    if (NULL == demuxer || NULL == handle) {
        MLOGE("%s para error\n", __FUNCTION__);
        return;
    }

    close_parser_all(demuxer, handle);
    for (idx = 0; idx < ES_TYPE_IS_MAX; idx++) {
        if (NULL != handle->ctx[idx].avctx) {
#ifdef CFG_ENABLE_FFMPEG_422
            avcodec_free_context(&(handle->ctx[idx].avctx));
#else
            av_freep(&(handle->ctx[idx].avctx));
#endif
        }
    }
    /* including avstream free */
    avformat_close_input(&(handle->fmt_ctx));
    MLOGI("Close parser success!\n");
}

/* one cannot use dp any more for which maybe freed by ts parser process */
int parse_es_add_packet(
    demuxer_t *demuxer, demux_stream_t *ds,
    parser_ctx_handle_t *handle, demux_packet_t *dp)
{
    int pkt_add_flag;
    int no_need_parse;
    if (NULL == ds || (NULL == dp && !(handle->parser_opened))) {
        MLOGW("%s para error\n", __FUNCTION__);
        return MT_FAILURE;
    }

    no_need_parse = (NULL == handle ||
        NULL == demuxer || NULL == ds->sh || !(handle->parser_opened));
    if(no_need_parse) {
        if (NULL != dp) {
            (void) ds_add_packet(ds, dp);
            return MT_SUCCESS;
        }
        return MT_FAILURE;
    }

    demux_packet_t *new_dp =
        parse_av_packet(demuxer, ds, handle, dp);
    /* equals when some error occus */
    if (new_dp != dp) {
        free_demux_packet(dp);
    }

    pkt_add_flag = MT_FALSE;
    while (new_dp != NULL) {
        if (MT_FALSE == ds_add_packet2(ds,&new_dp)) {
            break;
        }
        new_dp = new_dp->next;
        pkt_add_flag = MT_TRUE;
    }

    return (MT_TRUE == pkt_add_flag) ? MT_SUCCESS : MT_FAILURE;
}

int parse_es_reset_parser(demuxer_t *demuxer,
    demux_stream_t *ds, parser_ctx_handle_t * handle)
{
    if (NULL == demuxer || NULL == ds ||
        NULL == handle || !(handle->parser_opened)) {
        MLOGW("%s para error\n", __FUNCTION__);
        return MT_FAILURE;
    }
    (void) close_parser(demuxer, ds, handle);
    return init_parser(demuxer, ds, handle);
}

void parse_es_flush(
    demuxer_t *demuxer, parser_ctx_handle_t *handle)
{
    if (NULL == demuxer) {
        MLOGW("%s para error\n", __FUNCTION__);
        return;
    }

    if (NULL != handle && handle->parser_opened) {
        (void) close_parser_all(demuxer, handle);
        (void) open_parser_all(demuxer, handle);
    }
    demux_flush(demuxer);
}

