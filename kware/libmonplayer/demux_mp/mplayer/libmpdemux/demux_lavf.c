/*
 * Copyright (C) 2004 Michael Niedermayer <michaelni@gmx.at>
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

#include <limits.h>
#ifndef __LINUX__
#include "mp_func_trans.h"
#else
#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <limits.h>
#endif

#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"
#include "av_opts.h"
#include "av_helpers.h"

#include "stream/stream.h"
#include "aviprint.h"
#include "demuxer.h"
#include "stheader.h"
#include "m_option.h"
#include "sub/sub.h"

#define MODULE_TAG "DMX LAVF"
#include "mlog.h"
#include "mt_type.h"
#include "mt_error_mpi.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/avutil.h"
#include "libavutil/avstring.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#ifndef CFG_ENABLE_FFMPEG_422
#include "libavutil/buffer_internal.h"
#endif
#include "file_playback_sequence.h"

#include "mp_taglists.h"
#ifdef CFG_ENABLE_FFMPEG_422
#include "ffmpeg2mp_adapter.h"
#endif
#ifdef __LINUX__
#define OS_PRINTF printf
#endif
#define INITIAL_PROBE_SIZE io_stream_buffer_size //STREAM_BUFFER_SIZE
#define SMALL_MAX_PROBE_SIZE (32 * 1024)
#define PROBE_BUF_SIZE (2 * 1024 * 1024)
#define MAX_TRY_CNT_FILL_BUFFER    (1024)

//#define SEEK_DEBUG
#ifdef SEEK_DEBUG
extern flag_debug, while_cnt;
#endif
extern int bsf_vcodec_flag;
static char *opt_format;
static char *opt_cryptokey;
static char *opt_avopt = NULL;
#define mtos_printk
const m_option_t lavfdopts_conf[] = {
    { "format", &(opt_format), CONF_TYPE_STRING, 0, 0, 0, NULL },
    { "cryptokey", &(opt_cryptokey), CONF_TYPE_STRING, 0, 0, 0, NULL },
    { "o", &opt_avopt, CONF_TYPE_STRING, 0, 0, 0, NULL },
    { NULL, NULL, 0, 0, 0, 0, NULL }
};
//static int add_key_header = 1;
//1.5m  1.5 * 1024 * 1024
//#define BIO_BUFFER_SIZE (262144)
extern int g_player_buffer_size_mode;


//add for h264 header
static void* h264bsfc = NULL;
//add for h265 header
static void* h265bsfc = NULL;
//add for mpeg4 header
static void* mpeg4bsfc = NULL;

#define VOB_SUB_PARAM_LEN (256)
static char subt_ext_param[VOB_SUB_PARAM_LEN] = {0};
char *g_subt_ext_param = subt_ext_param;        //tizhang@20190814 for 111908

typedef struct lavf_priv
{
    AVInputFormat *avif;
    AVFormatContext *avfc;
    AVIOContext *pb;
    uint8_t *buffer;
    int is_live;
    int audio_streams;
    int video_streams;
    int sub_streams;
    int64_t last_pts;
    int astreams[MAX_A_STREAMS];
    int vstreams[MAX_V_STREAMS];
    int sstreams[MAX_S_STREAMS];
    int cur_program;
    int nb_streams_last;
    int vc1_first_video_frame;
    /* For adaptive resolution change use */
    int current_plsidx;
    int expected_plsidx;
    int usr_next_plsidx;
    off_t mark_filepos; //jqw@20190516 for bug 109953,109958
} lavf_priv_t;
//extern ff_pmt_id ff_mpegts_pmt; //used outside ffmpeg
extern int io_isnetworkstream;
extern void search_id_in_pmt(void);

#if MT_DES("VC1 PROCESS", 1)
#define RCV_EN                  1
#define VC1_LEVEL_LOW           0
#define VC1_LEVEL_MEDIUM        2
#define VC1_LEVEL_HIGH          4

enum Profile {
    VC1_PROFILE_SIMPLE,
    VC1_PROFILE_MAIN,
    VC1_PROFILE_COMPLEX, ///< TODO: WMV9 specific
    VC1_PROFILE_ADVANCED
};
enum VC1Code {
    VC1_CODE_RES0       = 0x00000100,
    VC1_CODE_ENDOFSEQ   = 0x0000010A,
    VC1_CODE_SLICE,
    VC1_CODE_FIELD,
    VC1_CODE_FRAME,
    VC1_CODE_ENTRYPOINT,
    VC1_CODE_SEQHDR,
};

struct VC1_STRUCT_SEQUENCE_HEADER_A {
    int ver_size;
    int horz_size;
};

struct VC1_STRUCT_SEQUENCE_HEADER_B {
    /*
    * [0,  2    ] -> [Low, Medium      ] for the simple profile.
    * [0,  2,  4] -> [Low, Medium, High] for the main profile.
    * [0,1,2,3,4] -> [L0 through L4    ] For the advanced profile
    */
    unsigned int level               : 3; /* specify the encoding level */
    unsigned char cbr                : 1; /* 1:cbr,0:not */
    unsigned char res1               : 4; /* shall be set to zero */
    union {
        struct {
            unsigned int hrd_buffer  : 24; /* buffer size B in milliseconds.is given by(HRD_BUFFER * HRD_RATE)/ 1000 */
            unsigned int hrd_rate    : 32; /* specify the peak transmission rate R in bits per second. */
        } s_m;
        struct {
            unsigned char adv[7];          /* shall be set to zero */
        } adv;
    };
    unsigned int framerate           : 32; /* rounded frame rate.is 0xffffffff if unknown,unspecified,or non-constant */
};

struct VC1_STRUCT_SEQUENCE_HEADER_C {
    unsigned int profile                 : 4; /* 0,4 or 12 indicate Simple,Main,Advanced, only 2 bits in SEQUENCE LAYER */
    union {
        struct {
            unsigned int frmrtq_postproc : 3; /* Quantized Frame Rate for Post processing Indicator */
            unsigned int bitrtq_postproc : 5; /* Quantized Bit Rate for Post processing Indicator */
            unsigned int loopfilter      : 1; /* use loop filtering 1:yew, 0:no */
            unsigned int reserved3       : 1; /* shall be set to zero */
            unsigned int multires        : 1; /* indicate whether frames coded at smaller resolutions than the specified */
            unsigned int reserved4       : 1; /* shall be set to zero */
            unsigned int fastuvmc        : 1; /* control the subpixel interpolation and rounding of color-difference motion vectors */
            unsigned int extended_mv     : 1; /* indicate whether extended motion vectors are enabled (value 1) or disabled (value 0) */
            unsigned int dquant          : 2; /* dindicate whether or not the quantization step size may vary within a frame */
            unsigned int vstransform     : 1; /* indicate whether variable-sized transform coding is enabled */
            unsigned int reserved5       : 1; /* shall be set to zero */
            unsigned int overlap         : 1; /* indicate whether Overlapped Transforms  are used */
            unsigned int syncmarker      : 1; /* indicate whether synchronization markers may be present */
            unsigned int rangered        : 1; /* indicate whether range reduction is used for each frame */
            unsigned int maxbframes      : 3; /* Maximum Number of consecutive B frames between I or P frames */
            unsigned int quantizer       : 2; /* indicate the quantizer used for the sequence */
            unsigned int finterpflag     : 1; /* indicate whether INTERPFRM is present in the picture header */
            unsigned int reserved6       : 1; /* shall be set to zero */
        } s_m;
        struct {
            unsigned int reserved7       : 28; /* shall be set to zero */
        } adv;
    } u;
};

/* MSB will be put first if put_bits_num larger than 8 */
static int lavf_put_bits(unsigned char *bits_buf,
    int *bits_pos, unsigned int value, int put_bits_num)
{
    uint8_t bit_byte;
    int bit_shift;
    int bit_byte_pos;

    bit_byte_pos = *bits_pos / 8;
    bit_shift    = *bits_pos % 8;
    bit_byte     = *(bits_buf + bit_byte_pos);

    if (bit_shift + put_bits_num <= 8) {
        bit_byte = (bit_byte << put_bits_num) + value;
        *(bits_buf + bit_byte_pos) = bit_byte;
        *bits_pos += put_bits_num;
    } else {
        int new_n = bit_shift + put_bits_num - 8;
        int new_value = 0;
        bit_byte = (bit_byte << (8 - bit_shift)) + (value >> new_n);
        *(bits_buf + bit_byte_pos) = bit_byte;
        *bits_pos += (8 - bit_shift);
        new_value = value & ((1 << new_n) - 1);
        lavf_put_bits(bits_buf, bits_pos, new_value, new_n);
    }

    return 0;
}

static int add_frame_header(AVCodecContext *codec, AVPacket *pkt,
    AVStream *st, unsigned char *bits_buf, int *bits_pos)
{
    if (codec->codec_id == AV_CODEC_ID_WMV3) {

        AVRational refTimebase;
        unsigned int framesize = pkt->size;
        int timestamp, size;
        refTimebase.num = 1;
        refTimebase.den = 1000;

        //set key flag
        if (pkt->flags & AV_PKT_FLAG_KEY) {
            framesize = framesize | 0x80;
        }

        lavf_put_bits(bits_buf, bits_pos, (0x4d4f4e54), 32);
        lavf_put_bits(bits_buf, bits_pos, (0x534f4301), 32);

        size = pkt->size | ((pkt->flags & AV_PKT_FLAG_KEY) ? 0x80000000 : 0);
        lavf_put_bits(bits_buf, bits_pos, (size) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (size >> 8) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (size >> 16) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (size >> 24) & 0xff, 8);
        timestamp = (int)av_rescale_q(pkt->pts, st->time_base, refTimebase);
        //write timestamp
        lavf_put_bits(bits_buf, bits_pos, (timestamp) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (timestamp >> 8) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (timestamp >> 16) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (timestamp >> 24) & 0xff, 8);
    } else {
        int pkt_header = ((*pkt->data) << 24) + ((*(pkt->data + 1)) << 16)
                         + ((*(pkt->data + 2)) << 8) + (*(pkt->data + 3));

        if (pkt_header != VC1_CODE_FRAME && pkt_header != VC1_CODE_SEQHDR) {
            lavf_put_bits(bits_buf, bits_pos, VC1_CODE_FRAME, 32)  ;
        }
    }

    return 0;
}

static int add_sequence_header_adv(unsigned char *vc1_bits_buf, int *vc1_bits_pos)
{
    int level;
    int chromaformat;     ///< 2bits, 2=4:2:0, only defined
    int postprocflag;     ///< Per-frame processing suggestion flag present
    int broadcast;        ///< TFF/RFF present
    int interlace;        ///< Progressive/interlaced (RPTFTM syntax element)
    int tfcntrflag;       ///< TFCNTR present
    int frmrtq_postproc;  ///< 3bits,
    int bitrtq_postproc;  ///< 5bits, quantized framerate-based postprocessing strength
    int finterpflag;      ///< INTERPFRM present
    struct VC1_STRUCT_SEQUENCE_HEADER_A struct_a = {0};

    level = 3;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, level, 3);
    //
    chromaformat = 1;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, chromaformat, 2);
    frmrtq_postproc = 7;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, frmrtq_postproc, 3); //common
    bitrtq_postproc = 31;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, bitrtq_postproc, 5); //common
    postprocflag = 0;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, postprocflag, 1);  //common
    struct_a.ver_size = 639;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, struct_a.ver_size, 12)  ;

    struct_a.horz_size = 359;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, struct_a.horz_size, 12)  ;
    broadcast = 1;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, broadcast, 1);
    interlace = 0;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, interlace, 1);
    tfcntrflag = 0;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, tfcntrflag, 1);
    finterpflag = 0;
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, finterpflag, 1);
    lavf_put_bits(vc1_bits_buf, vc1_bits_pos, 0, 1); // reserved

    return 0;
}

#define IS_MARKER(x) (((x) & ~0xFF) == VC1_CODE_RES0)
static av_always_inline const uint8_t *
    find_next_marker(const uint8_t *src, const uint8_t *end)
{
    uint32_t mrk = 0xFFFFFFFF;

    if (end - src < 4) {
        return end;
    }

    while (src < end) {
        mrk = (mrk << 8) | *src++;

        if (IS_MARKER(mrk)) {
            return src - 4;
        }
    }

    return end;
}

static int get_vc1_level(unsigned int profile,
    unsigned int width, unsigned int height)
{
    int total_mbs;
    total_mbs = (width * height) >> 8;

    if (profile == VC1_PROFILE_SIMPLE) {
        if (total_mbs <= 99) { //low
            return VC1_LEVEL_LOW;
        } else if (total_mbs <= 396) { //medium
            return VC1_LEVEL_MEDIUM;
        } else {
            return -1;
        }
    } else if (profile == VC1_PROFILE_MAIN) {
        if (total_mbs <= 396) { //low
            return VC1_LEVEL_LOW;
        } else if (total_mbs <= 1620) { //medium
            return VC1_LEVEL_MEDIUM;
        } else if (total_mbs <= 8192) { //high
            return VC1_LEVEL_HIGH;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}
static int vc1_gen_display_bits_adv(
    AVCodecContext *codec, uint8_t *raw_bits,
    float fps, unsigned char *bits_buf, int *bits_pos)
{
    lavf_put_bits(bits_buf, bits_pos, (*raw_bits), 8);

    return 0;
}

static int vc1_add_sequence_header(
    AVCodecContext *codec, float fps, AVStream *st,
    AVPacket *pkt, unsigned char *bits_buf, int *bits_pos)
{
    uint8_t *next;
    uint8_t *start = codec->extradata;
    uint8_t *end = codec->extradata + codec->extradata_size;
    struct VC1_STRUCT_SEQUENCE_HEADER_B strct_b  = {0};
    struct VC1_STRUCT_SEQUENCE_HEADER_C struct_c = {{0}};

    /*
     * vc-1 consists of wvc1 and wmv3,
     * wvc1 is advanced profile,
     * and wmv3 is simple/main profile
     */
    if (codec->codec_id == AV_CODEC_ID_WMV3) {
#if RCV_EN
        int num_frames;
        int profile;

        profile = codec->extradata[0] >> 6; //2bits
        end = codec->extradata + 4;
        /* write seq start code MONTSOC0 */
        lavf_put_bits(bits_buf, bits_pos, 0x4d4f4e54, 32);
        lavf_put_bits(bits_buf, bits_pos, 0x534f4300, 32);

        strct_b.framerate = (int)fps;
        num_frames = st->duration * strct_b.framerate * 1.f / 1000;
        num_frames = 0;
        lavf_put_bits(bits_buf, bits_pos, (num_frames) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (num_frames >> 8) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (num_frames >> 16) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, 0xc5, 8);
        ///write 0x00000004
        lavf_put_bits(bits_buf, bits_pos, 0x4, 8);
        lavf_put_bits(bits_buf, bits_pos, 0, 24);
        ///write struct c
        for (next = start; next < end; next++) {
            lavf_put_bits(bits_buf, bits_pos, *next, 8);
        }

        ///write struct a
        lavf_put_bits(bits_buf, bits_pos, (codec->height) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (codec->height >> 8) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (codec->height >> 16) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (codec->height >> 24) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (codec->width) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (codec->width >> 8) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (codec->width >> 16) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (codec->width >> 24) & 0xff, 8);
        ///write 0x0000000c
        lavf_put_bits(bits_buf, bits_pos, 0xc, 8);
        lavf_put_bits(bits_buf, bits_pos, 0, 24);
        //write struct b
        strct_b.level = get_vc1_level(profile, codec->width, codec->height);
        lavf_put_bits(bits_buf, bits_pos, 0, 24);
        //lavf_put_bits(bits_buf, bits_pos, level << 3, 8);
        lavf_put_bits(bits_buf, bits_pos, 0x80, 8);
        lavf_put_bits(bits_buf, bits_pos, 0, 32);
        strct_b.framerate = 0xffffffff;
        lavf_put_bits(bits_buf, bits_pos, (strct_b.framerate) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (strct_b.framerate >> 8) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (strct_b.framerate >> 16) & 0xff, 8);
        lavf_put_bits(bits_buf, bits_pos, (strct_b.framerate >> 24) & 0xff, 8);
#else
        lavf_put_bits(bits_buf, bits_pos, VC1_CODE_SEQHDR, 32);

        for (next = start; next < end; next++) {
            lavf_put_bits(bits_buf, bits_pos, *next, 8);
        }

        struct_c.u.s_m.fastuvmc    = *(start + 2) >> 7;
        struct_c.u.s_m.extended_mv = (*(start + 2) >> 6) & 1;
        struct_c.u.s_m.dquant      = (*(start + 2) >> 4) & 0x3;
        struct_c.u.s_m.vstransform = (*(start + 2) >> 3) & 1;
        struct_c.u.s_m.overlap     = (*(start + 2) >> 1) & 1;
        struct_c.u.s_m.quantizer   = (*(start + 3) >> 2) & 0x3;

        lavf_put_bits(bits_buf, bits_pos, VC1_CODE_ENTRYPOINT, 32);
        //printf("\n%s %d w %d  h:%d\n", __FUNCTION__, __LINE__, codec->width, codec->height);
        u32 entry1 = 0x00c49fc5;
        lavf_put_bits(bits_buf, bits_pos, entry1 >> 27, 5);
        lavf_put_bits(bits_buf, bits_pos, struct_c.u.fastuvmc, 1);
        lavf_put_bits(bits_buf, bits_pos, struct_c.u.extended_mv, 1);
        lavf_put_bits(bits_buf, bits_pos, struct_c.u.dquant, 2);
        lavf_put_bits(bits_buf, bits_pos, struct_c.u.vstransform, 1);
        lavf_put_bits(bits_buf, bits_pos, struct_c.u.overlap, 1);
        lavf_put_bits(bits_buf, bits_pos, struct_c.u.quantizer, 2);
        //lavf_put_bits(bits_buf, bits_pos, (entry1 >> 20) & 0xf, 4);
        lavf_put_bits(bits_buf, bits_pos, 1, 1);
        lavf_put_bits(bits_buf, bits_pos, codec->width / 2 - 1, 12);
        lavf_put_bits(bits_buf, bits_pos, codec->height / 2 - 1, 12);
        u16 entry2 = 0x9c80;
        lavf_put_bits(bits_buf, bits_pos, entry2 & 0x3ff, 10);
#endif
    } else {
        int pkt_header = ((*pkt->data) << 24) + ((*(pkt->data + 1)) << 16)
                         + ((*(pkt->data + 2)) << 8) + (*(pkt->data + 3));
        if (pkt_header == VC1_CODE_SEQHDR) {
            return 0;
        }

        start = find_next_marker(start, end); // in WVC1 codec_extradata first byte is its size, but can be 0 in mkv
        for (next = start; next < start + 9; next++) {
            lavf_put_bits(bits_buf, bits_pos, *next, 8);
        }

        vc1_gen_display_bits_adv(codec, start + 9, fps, bits_buf, bits_pos);
        for (next = start + 10; next < end; next++) {
            lavf_put_bits(bits_buf, bits_pos, *next, 8);
        }
    }

    if (VC1_PROFILE_ADVANCED == struct_c.profile) {
        return add_sequence_header_adv(bits_buf, bits_pos);
    }

    return 0;
}
#endif /* end VC-1 */

static int mp_read(void * opaque, uint8_t * buf, int size)
{
    demuxer_t *demuxer = opaque;
    stream_t *stream = demuxer->stream;
    int ret;

    //MLOGA("%s    %d  start\n",__func__,__LINE__);
    ret = stream_read(stream, buf, size);
    //MLOGA("%d=mp_read(%p, %p, %d), pos: %"PRId64", eof:%d\n",
    //		ret, stream, buf, size, stream_tell(stream), stream->eof);
    mp_msg(MSGT_HEADER, MSGL_DBG2, "%d=mp_read(%p, %p, %d), pos: %" PRId64 ", eof:%d\n",
           ret, stream, buf, size, stream_tell(stream), stream->eof);
    return ret;
}

static int64_t mp_seek(void *opaque, int64_t pos, int whence)
{
    demuxer_t *demuxer = opaque;
    stream_t *stream = demuxer->stream;
    int64_t current_pos;
    MLOGA("mp_seek(%p, %" PRId64 ", %d)\n", stream, pos, whence);
    if (whence == SEEK_CUR) {
        pos += stream_tell(stream);
    } else if (whence == SEEK_END && stream->end_pos > 0) {
        pos += stream->end_pos;
    } else if (whence == SEEK_SET) {
        pos += stream->start_pos;
    } else if (whence == AVSEEK_SIZE && stream->end_pos > 0) {
        uint64_t size;
        stream_control(stream, STREAM_CTRL_GET_SIZE, &size);
        if (size > stream->end_pos) {
            stream->end_pos = size;
        }
        return stream->end_pos - stream->start_pos;
    } else if (whence == AVSEEK_SIZE && stream->is_chunked > 0) {
        uint64_t size;
        stream_control(stream, STREAM_CTRL_GET_SIZE, &size);
        //printf("%s %d size %llu, start_pos %lld , end_pos %lld\n", __func__, __LINE__, size);
        return size;
    } else {
        return -1;
    }

    if (pos < 0) {
        return -1;
    }
    current_pos = stream_tell(stream);
    if (stream_seek(stream, pos) == 0) {
        stream_reset(stream);
        stream_seek(stream, current_pos);
        return -1;
    }

    return pos - stream->start_pos;
}

static int64_t mp_read_seek(void *opaque, int stream_idx, int64_t ts, int flags)
{
    demuxer_t *demuxer = opaque;
    stream_t *stream = demuxer->stream;
    lavf_priv_t *priv = demuxer->priv;
    AVStream *st = priv->avfc->streams[stream_idx];
    int ret;
    double pts;

    pts = (double)ts * st->time_base.num / st->time_base.den;
    ret = stream_control(stream, STREAM_CTRL_SEEK_TO_TIME, &pts);
    if (ret < 0)
	ret = AVERROR(ENOSYS);
    return ret;
}

static void list_formats(void)
{
    AVInputFormat *fmt;
    MLOGA("Available lavf input formats:\n");
    for (fmt = av_iformat_next(NULL); fmt; fmt = av_iformat_next(fmt))
	MLOGA("%15s : %s\n", fmt->name, fmt->long_name);
}

static mt_u32 avformat_istream_is_opened(demuxer_t *demuxer)
{
    mt_u32 opened = 0;
#ifdef CFG_ENABLE_FFMPEG_422
    int idx = 0;
    mt_u32 (*opened_func[])(const char *) = {
        stream_is_rtp_type,
        stream_is_udp_type,
        stream_is_rtsp_type,
        stream_is_rtmp_type,
        NULL
    };

    if (NULL == demuxer->stream ||
        NULL == demuxer->stream->priv) {
        return 0;
    }

    while (NULL != opened_func[idx]) {
        opened |= opened_func[idx++](demuxer->stream->url);
    }
#endif
    return opened;
}

static mt_u32 av_iformat_is_obtained(
    demuxer_t *demuxer, lavf_priv_t *priv)
{
#ifdef CFG_ENABLE_FFMPEG_422
    AVFormatContext *fmt_ctx;
    if (!avformat_istream_is_opened(demuxer)) {
        return MT_FALSE;
    }

    fmt_ctx    = (AVFormatContext *) demuxer->stream->priv;
    priv->avif = fmt_ctx->iformat;
    return MT_TRUE;
#endif
    return MT_FALSE;
}

static int lavf_probe(demuxer_t *demuxer,
    lavf_priv_t *priv, AVProbeData *probe_data, int probe_size, int *score)
{
    int read_size = INITIAL_PROBE_SIZE;
    int probe_data_size = 0;
    int64_t current_pos = 0;
    int64_t filesize = 0;

    if (NULL == priv) {
        MLOGE("[%s] lavf priv null error\n", __func__);
        return MT_FAILURE;
    }

    if (av_iformat_is_obtained(demuxer, priv)) {
        *score = 100;
        return MT_SUCCESS;
    }

    do {
        read_size = stream_read(demuxer->stream, probe_data->buf + probe_data_size, read_size);
        MLOGA("[%s] --in while--- read_size: %d\n", __func__, read_size);
        if (read_size < 0) {
            MLOGE("[%s] --1--- return 0\n", __func__);
            return MT_FAILURE;
        }
    	probe_data_size += read_size;
    	probe_data->filename = demuxer->stream->url;
    	if (!probe_data->filename) {
    	    mp_msg(MSGT_DEMUX, MSGL_WARN, "Stream url is not set!\n");
    	    probe_data->filename = "";
    	}
    	if (!strncmp(probe_data->filename, "ffmpeg://", 9))
    	    probe_data->filename += 9;

    	probe_data->buf_size = probe_data_size;
    	*score = 0;
    	priv->avif = av_probe_input_format2(probe_data,  probe_data_size > 0, score);
        if(demuxer->stream->end_pos > 0) {
            current_pos = stream_tell(demuxer->stream);
    	    stream_control(demuxer->stream, STREAM_CTRL_GET_SIZE, &filesize);
            MLOGA("%s probe_data_size %d, current_pos %lld, filesize %lld\n",
                __func__, probe_data_size, current_pos, filesize);
            if(*score == AVPROBE_SCORE_MAX
                || current_pos >= filesize)
                break;
        }
    	read_size = FFMIN(2 * read_size, probe_size - probe_data_size);
    } while ((probe_data_size < SMALL_MAX_PROBE_SIZE) &&
             *score <= AVPROBE_SCORE_MAX / 4 &&
             read_size > 0 && probe_data_size < probe_size);

    MLOGI("[%s] --out of while--- read_size: %d\n", __func__, read_size);
    return MT_SUCCESS;
}

static int lavf_check_file(demuxer_t *demuxer)
{
#ifdef CFG_ENABLE_FFMPEG_422
    AVProbeData avpd = { 0 };
#else
    AVProbeData avpd;
#endif
    int score;
    lavf_priv_t *priv;
    int probe_size = PROBE_BUF_SIZE;
    int bio_buffer_size = 0;

    MLOGI("[%s] ----- start\n", __func__);
    if (!demuxer->priv) {
	    demuxer->priv = (void *)calloc33(sizeof(lavf_priv_t), 1);
    }

    priv = demuxer->priv;
    init_avformat();
    if (opt_format) {
    	if (strcmp(opt_format, "help") == 0) {
    	    list_formats();
    	    return 0;
    	}
    	priv->avif = av_find_input_format(opt_format);
    	if (!priv->avif) {
    	    MLOGE("Unknown lavf format %s\n", opt_format);
    	    return 0;
    	}
    	MLOGA("Forced lavf %s demuxer\n", priv->avif->long_name);
    	return DEMUXER_TYPE_LAVF;
    }

    if(g_player_buffer_size_mode==0) {
		bio_buffer_size = 262144;//256*1024
		probe_size = probe_size / 2;
    }
	else
		bio_buffer_size = 1048576;//1024*1024
    avpd.buf = av_mallocz(FFMAX(bio_buffer_size, probe_size) + FF_INPUT_BUFFER_PADDING_SIZE);
    while (avpd.buf == NULL && probe_size > bio_buffer_size) {
        MLOGE("[%s] av_mallocz fail\n", __func__);
        probe_size = probe_size / 2;
        avpd.buf = av_mallocz(FFMAX(bio_buffer_size, probe_size) + FF_INPUT_BUFFER_PADDING_SIZE);
    }

    if (NULL == avpd.buf) {
    	return 0;
    }

    if (MT_SUCCESS !=
            lavf_probe(demuxer, priv, &avpd, probe_size, &score)) {
        av_free(avpd.buf);
        return 0;
    }

    #ifdef CFG_ENABLE_FFMPEG_422
    if(score < 10){
        //score == 1 means that the name has the .ext check and it is usually unused
        priv->avif = NULL;
    }
    #endif

    av_free(avpd.buf);
    if (!priv->avif) {
    	MLOGE("LAVF_check: no clue about this gibberish!\n");
    	return 0;
    } else {
    	MLOGA("LAVF_check: %s\n", priv->avif->long_name);
    }
    priv->is_live         = 0;
    priv->current_plsidx  =
    priv->usr_next_plsidx =
    priv->expected_plsidx = -1;
    MLOGI("[%s] ----- end, return DEMUXER_TYPE_LAVF\n", __func__);
    return DEMUXER_TYPE_LAVF;
}

static uint8_t char2int(char c)
{
    if (c >= '0' && c <= '9')
	return c - '0';
    if (c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
	return c - 'A' + 10;
    return 0;
}

static void parse_cryptokey(AVFormatContext *avfc, const char *str)
{
    int len = strlen(str) / 2;
    uint8_t *key = av_mallocz(len);
    int i;
    avfc->keylen = len;
    avfc->key = key;
    for (i = 0; i < len; i++, str += 2)
	*key++ = (char2int(str[0]) << 4) | char2int(str[1]);
}

#ifdef CFG_ENABLE_FFMPEG_422
#include "libavutil/mastering_display_metadata.h"
static void mp_dump_mastering_display_metadata(void *ctx, AVPacketSideData* sd, sh_video_t *sh) {
    AVMasteringDisplayMetadata* metadata = (AVMasteringDisplayMetadata*)sd->data;
    const int chroma_den = 50000;
    const int luma_den = 10000;
    double tmp;

    MLOGD("Mastering Display Metadata," "has_primaries:%d has_luminance:%d "
           "r(%5.4f,%5.4f) g(%5.4f,%5.4f) b(%5.4f %5.4f) wp(%5.4f, %5.4f) "
           "min_luminance=%f, max_luminance=%f",
           metadata->has_primaries, metadata->has_luminance,
           av_q2d(metadata->display_primaries[0][0]),
           av_q2d(metadata->display_primaries[0][1]),
           av_q2d(metadata->display_primaries[1][0]),
           av_q2d(metadata->display_primaries[1][1]),
           av_q2d(metadata->display_primaries[2][0]),
           av_q2d(metadata->display_primaries[2][1]),
           av_q2d(metadata->white_point[0]), av_q2d(metadata->white_point[1]),
           av_q2d(metadata->min_luminance), av_q2d(metadata->max_luminance));

    sh->hdr.primary_r_chromaticity_x = (unsigned int)(av_q2d(metadata->display_primaries[0][0])*chroma_den);
    sh->hdr.primary_r_chromaticity_y = (unsigned int)(av_q2d(metadata->display_primaries[0][1])*chroma_den);
    sh->hdr.primary_g_chromaticity_x = (unsigned int)(av_q2d(metadata->display_primaries[1][0])*chroma_den);
    sh->hdr.primary_g_chromaticity_y = (unsigned int)(av_q2d(metadata->display_primaries[1][1])*chroma_den);
    sh->hdr.primary_b_chromaticity_x = (unsigned int)(av_q2d(metadata->display_primaries[2][0])*chroma_den);
    sh->hdr.primary_b_chromaticity_y = (unsigned int)(av_q2d(metadata->display_primaries[2][1])*chroma_den);
    sh->hdr.white_point_chromaticity_x = (unsigned int)(av_q2d(metadata->white_point[0])*chroma_den);
    sh->hdr.white_point_chromaticity_y = (unsigned int)(av_q2d(metadata->white_point[1])*chroma_den);
    sh->hdr.has_primaries = 1;

    tmp = av_q2d(metadata->max_luminance)*luma_den;
    sh->hdr.max_luminance = (unsigned int)tmp;

    tmp = av_q2d(metadata->min_luminance)*luma_den;
    sh->hdr.min_luminance = (unsigned int)tmp;
    sh->hdr.has_luminance = 1;
}

static void mp_dump_content_light_metadata(void *ctx, AVPacketSideData* sd, sh_video_t *sh)
{
    AVContentLightMetadata* metadata = (AVContentLightMetadata*)sd->data;
    MLOGD("Content Light Level Metadata, MaxCLL=%d, MaxFALL=%d",
           metadata->MaxCLL, metadata->MaxFALL);

    sh->hdr.max_light_level = metadata->MaxCLL;
    sh->hdr.max_pic_ave_light_level = metadata->MaxFALL;
}

static void dump_sidedata(void *ctx, AVStream *st, const char *indent, sh_video_t *sh)
{
    int i;

    if (st->nb_side_data)
        MLOGD("%sSide data:\n", indent);

    if(!st->codecpar->color_primaries || !st->codecpar->color_trc)
        return;

    sh->hdr.colour_primaries = st->codecpar->color_primaries;
    sh->hdr.transfer_characteristics = st->codecpar->color_trc;

    for (i = 0; i < st->nb_side_data; i++) {
        AVPacketSideData sd = st->side_data[i];
        MLOGD("%s  ", indent);

        switch (sd.type) {
        case AV_PKT_DATA_PALETTE:
            MLOGD("palette");
            break;
        case AV_PKT_DATA_NEW_EXTRADATA:
            MLOGD("new extradata");
            break;
        case AV_PKT_DATA_PARAM_CHANGE:
            MLOGD("paramchange: ");
            //dump_paramchange(ctx, &sd);
            break;
        case AV_PKT_DATA_H263_MB_INFO:
            MLOGD("H.263 macroblock info");
            break;
        case AV_PKT_DATA_REPLAYGAIN:
            MLOGD("replaygain: ");
            //dump_replaygain(ctx, &sd);
            break;
        case AV_PKT_DATA_DISPLAYMATRIX:
            //printf("displaymatrix: rotation of %.2f degrees",
                //   av_display_rotation_get((int32_t *)sd.data));
            break;
        case AV_PKT_DATA_STEREO3D:
            MLOGD("stereo3d: ");
            //dump_stereo3d(ctx, &sd);
            break;
        case AV_PKT_DATA_AUDIO_SERVICE_TYPE:
            MLOGD("audio service type: ");
            //dump_audioservicetype(ctx, &sd);
            break;
        case AV_PKT_DATA_QUALITY_STATS:
            //av_log(ctx, AV_LOG_INFO, "quality factor: %"PRId32", pict_type: %c",
                   //AV_RL32(sd.data), av_get_picture_type_char(sd.data[4]));
            break;
        case AV_PKT_DATA_CPB_PROPERTIES:
            //av_log(ctx, AV_LOG_INFO, "cpb: ");
            //dump_cpb(ctx, &sd);
            break;
        case AV_PKT_DATA_MASTERING_DISPLAY_METADATA:
            mp_dump_mastering_display_metadata(ctx, &sd, sh);
            break;
        case AV_PKT_DATA_SPHERICAL:
            //av_log(ctx, AV_LOG_INFO, "spherical: ");
            //dump_spherical(ctx, st->codecpar, &sd);
            break;
        case AV_PKT_DATA_CONTENT_LIGHT_LEVEL:
            mp_dump_content_light_metadata(ctx, &sd, sh);
            break;
        default:
            //av_log(ctx, AV_LOG_INFO,
           //        "unknown side data type %d (%d bytes)", sd.type, sd.size);
            break;
        }

        MLOGD("\n");
    }
    MLOGD("[%s_%d]c[%u %u] light[%u %u] r[%u %u] g[%u %u] b[%u %u] w[%u %u] max[%u] min[%u]\n",
        __func__, __LINE__, sh->hdr.colour_primaries, sh->hdr.transfer_characteristics,\
        sh->hdr.max_light_level,sh->hdr.max_pic_ave_light_level,\
        sh->hdr.primary_r_chromaticity_x,sh->hdr.primary_r_chromaticity_y,\
        sh->hdr.primary_g_chromaticity_x,sh->hdr.primary_g_chromaticity_y,\
        sh->hdr.primary_b_chromaticity_x,sh->hdr.primary_b_chromaticity_y,\
        sh->hdr.white_point_chromaticity_x, sh->hdr.white_point_chromaticity_y,\
        sh->hdr.max_luminance,sh->hdr.min_luminance);
}
#else
#endif

static void show_stream_index_tabel(AVStream *st)
{
#if MT_DES("Show Index Table", 0)
    int idx = 0;
    if (AVMEDIA_TYPE_VIDEO != st->codecpar->codec_type) {
        return;
    }
    MLOGW("Stream %d Index Table:\n", st->id);
    for (; idx < st->nb_index_entries; idx++) {
        AVIndexEntry *e = &st->index_entries[idx];
        if (e->flags) {
            MLOGI("[%d] pos:0x%llx time:(%llu %dms), flag:%d\n", idx, e->pos, e->timestamp,
                (int) ((double) e->timestamp * av_q2d(st->time_base) * 1000), (int) e->flags);
        }
    }
#endif
}

static void handle_stream_extradata(AVStream *st, sh_common_t *sh)
{
    uint8_t *extradata = NULL;
    int extradata_size = 0;
#ifdef CFG_ENABLE_FFMPEG_422
    AVCodecParameters *codec = st->codecpar;
    extradata      = codec->extradata;
    extradata_size = codec->extradata_size;
#else
    AVCodecContext *codec = st->codec;
    extradata      = codec->extradata;
    extradata_size = codec->extradata_size;
#endif
    SAFEFREE(sh->codec_extradata);
    sh->codec_extradata = NULL;
    sh->codec_extradata_size = 0;
    if (extradata && extradata_size) {
        sh->codec_extradata = (unsigned char *)malloc33(extradata_size);
        if (sh->codec_extradata) {
            memcpy(sh->codec_extradata, extradata, extradata_size);
            sh->codec_extradata_size = extradata_size;
        }
    }
}

static int handle_stream_audio(demuxer_t *demuxer, AVFormatContext *avfc, int i)
{
    int g;
    WAVEFORMATEX *wf      = NULL;
    sh_audio_t *sh_audio  = NULL;
    lavf_priv_t    *priv  = demuxer->priv;
    AVStream       *st    = avfc->streams[i];
    AVCodecContext *codec = st->codec;
    AVDictionaryEntry *lang  = av_dict_get(st->metadata, "language", NULL, 0);
    AVDictionaryEntry *title = av_dict_get(st->metadata, "title"   , NULL, 0);

    MLOGD("Audio track %d, %s \n", i, lang ? lang->value : NULL);
    sh_audio = new_sh_audio_aid(demuxer, i, priv->audio_streams, lang ? lang->value : NULL);
    if (!sh_audio) {
        MLOGE("[%s] new stream error \n", __func__);
        return MT_FAILURE;
    }

    priv->astreams[priv->audio_streams] = i;
    wf = (WAVEFORMATEX *)calloc33(sizeof(*wf) + codec->extradata_size, 1);
    if (NULL == wf) {
        MLOGE("[%s] malloc wav error \n", __func__);
        free_sh_audio(demuxer, i);
        return MT_FAILURE;
    }

#ifdef CFG_ENABLE_FFMPEG_422
    AVCodecParameters *codecpar = st->codecpar;
    codec->sample_rate = codecpar->sample_rate;
#endif

    codec->codec_tag = mp_codec_id2tag(codec->codec_id, codec->codec_tag, 1);
    wf->wFormatTag   = codec->codec_tag;
    wf->nChannels    = codec->channels;
    wf->nSamplesPerSec  = codec->sample_rate;
    wf->nAvgBytesPerSec = codec->bit_rate / 8;
    wf->nBlockAlign     = codec->block_align ? codec->block_align : 1;
    wf->wBitsPerSample  = codec->bits_per_coded_sample;
    wf->cbSize = codec->extradata_size;
    handle_stream_extradata(st, (sh_common_t *) sh_audio);
    sh_audio->wf = wf;
    sh_audio->audio.dwSampleSize = codec->block_align;
    if (codec->frame_size && codec->sample_rate) {
        sh_audio->audio.dwScale = codec->frame_size;
        sh_audio->audio.dwRate  = codec->sample_rate;
    } else {
        sh_audio->audio.dwScale = codec->block_align ? codec->block_align * 8 : 8;
        sh_audio->audio.dwRate  = codec->bit_rate;
    }

    g = av_gcd(sh_audio->audio.dwScale, sh_audio->audio.dwRate);
    sh_audio->audio.dwScale /= g;
    sh_audio->audio.dwRate /= g;
    sh_audio->ds = demuxer->audio;
    sh_audio->format = codec->codec_tag;
    sh_audio->channels = codec->channels;
    sh_audio->samplerate = codec->sample_rate;
    sh_audio->i_bps = codec->bit_rate / 8;

    sh_audio->hAvCtx = (void *)codec;
    switch (codec->codec_id) {
        case CODEC_ID_PCM_S8:
        case CODEC_ID_PCM_U8:
            sh_audio->samplesize = 1;
            break;
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            sh_audio->samplesize = 2;
            break;
        case AV_CODEC_ID_PCM_S24LE:
        case AV_CODEC_ID_PCM_S24BE:
        case AV_CODEC_ID_PCM_U24LE:
        case AV_CODEC_ID_PCM_U24BE:
        case AV_CODEC_ID_PCM_S24DAUD:
            sh_audio->samplesize = 3;
            break;
        case AV_CODEC_ID_PCM_S32LE:
        case AV_CODEC_ID_PCM_S32BE:
        case AV_CODEC_ID_PCM_U32LE:
        case AV_CODEC_ID_PCM_U32BE:
            sh_audio->samplesize = 4;
            break;
        case CODEC_ID_PCM_ALAW:
            sh_audio->format = 0x6;
            break;
        case CODEC_ID_PCM_MULAW:
            sh_audio->format = 0x7;
            break;
        default:
            break;
    }
    if (title && title->value) {
        MLOGD("ID_AID_%d_NAME=%s\n", priv->audio_streams, title->value);
        sh_audio->title = (char *)strdup33(title->value);
    } else {
        sh_audio->title = NULL;
    }
    if (st->disposition & AV_DISPOSITION_DEFAULT) {
        sh_audio->default_track = 1;
    }
    if (mp_msg_test(MSGT_HEADER, MSGL_V)) {
        print_wave_header(sh_audio->wf, MSGL_V);
    }
    // select the first audio stream if auto-selection is requested
    if (demuxer->audio->id == -1) {
        demuxer->audio->id = i;
        demuxer->audio->sh = demuxer->a_streams[i];
    }
    if (demuxer->audio->id != i) {
        st->discard = AVDISCARD_ALL;
    }
    return MT_SUCCESS;
}

static int handle_stream_video(demuxer_t *demuxer, AVFormatContext *avfc, int i)
{
    sh_video_t *sh_video;
    AVRational frame_rate;
    BITMAPINFOHEADER *bih;
    lavf_priv_t    *priv  = demuxer->priv;
    AVStream       *st    = avfc->streams[i];
    AVCodecContext *codec = st->codec;
    AVDictionaryEntry *title = av_dict_get(st->metadata, "title"   , NULL, 0);

    MLOGD("Video track %d\n", i);
    sh_video = new_sh_video_vid(demuxer, i, priv->video_streams);
    if (!sh_video) {
        MLOGE("[%s] new stream error \n", __func__);
        return MT_FAILURE;
    }

    priv->vstreams[priv->video_streams] = i;
    bih = (BITMAPINFOHEADER *) calloc33(sizeof(*bih) + codec->extradata_size, 1);
    if (NULL == bih) {
        MLOGE("[%s] malloc bitmap header error \n", __func__);
        free_sh_video(sh_video);
        return MT_FAILURE;
    }

#ifdef CFG_ENABLE_FFMPEG_422
    dump_sidedata(NULL, st, "    ", sh_video);
#endif
    if (codec->codec_id == CODEC_ID_RAWVIDEO) {
        switch (codec->pix_fmt) {
            case PIX_FMT_RGB24:
                codec->codec_tag = MKTAG(24, 'B', 'G', 'R');
            case PIX_FMT_BGR24:
                codec->codec_tag = MKTAG(24, 'R', 'G', 'B');
            default:
                break;
        }
    }
    codec->codec_tag = mp_codec_id2tag(codec->codec_id, codec->codec_tag, 0);
    bih->biSize    = sizeof(*bih) + codec->extradata_size;
    bih->biWidth   = codec->width;
    bih->biHeight  = codec->height;
    bih->biBitCount  = codec->bits_per_coded_sample;
    bih->biSizeImage = bih->biWidth * bih->biHeight * bih->biBitCount / 8;
    bih->biCompression = codec->codec_tag;
    sh_video->bih = bih;
    sh_video->disp_w = codec->width;
    sh_video->disp_h = codec->height;
    if (st->time_base.den) { /* if container has time_base, use that */
        sh_video->video.dwRate  = st->time_base.den;
        sh_video->video.dwScale = st->time_base.num;
    } else {
        sh_video->video.dwRate  = codec->time_base.den;
        sh_video->video.dwScale = codec->time_base.num;
    }

    frame_rate = av_guess_frame_rate(avfc, st, NULL);
    sh_video->fps.num = frame_rate.num;
    sh_video->fps.den = frame_rate.den;
    sh_video->frametime = 1 / av_q2d(st->r_frame_rate);
    sh_video->format = bih->biCompression;
    if (st->sample_aspect_ratio.num) {
        sh_video->aspect = codec->width * st->sample_aspect_ratio.num / (float)(codec->height * st->sample_aspect_ratio.den);
    } else {
        sh_video->aspect = codec->width * codec->sample_aspect_ratio.num / (float)(codec->height * codec->sample_aspect_ratio.den);
    }
    sh_video->i_bps = codec->bit_rate / 8;
    if (title && title->value) {
        MLOGD("ID_VID_%d_NAME=%s\n", priv->video_streams, title->value);
    }

    MLOGD("aspect= %d*%d/(%d*%d)\n", codec->width,
        codec->sample_aspect_ratio.num, codec->height, codec->sample_aspect_ratio.den);

    sh_video->ds = demuxer->video;
    if (codec->extradata_size) {
        memcpy(sh_video->bih + 1, codec->extradata, codec->extradata_size);
        handle_stream_extradata(st, (sh_common_t *) sh_video);
    }
    if (mp_msg_test(MSGT_HEADER, MSGL_V)) {
        print_video_header(sh_video->bih, MSGL_V);
    }
    // select the first video stream if auto-selection is requested
    if (demuxer->video->id == -1) {
        demuxer->video->id = i;
        demuxer->video->sh = demuxer->v_streams[i];
    }
    if (demuxer->video->id != i) {
        st->discard = AVDISCARD_ALL;
    }
    show_stream_index_tabel(st);
    return MT_SUCCESS;
}

static int handle_stream_subtitle(demuxer_t *demuxer, AVFormatContext *avfc, int i)
{
    sh_sub_t *sh_sub;
    /* t = text, v = VobSub, a = SSA/ASS */
    char type;
    lavf_priv_t    *priv  = demuxer->priv;
    AVStream       *st    = avfc->streams[i];
    AVCodecContext *codec = st->codec;
    AVDictionaryEntry *lang  = av_dict_get(st->metadata, "language", NULL, 0);
    AVDictionaryEntry *title = av_dict_get(st->metadata, "title"   , NULL, 0);

    MLOGD("Sub track %d\n", i);
    if (demuxer->sub->id == -1) {
        demuxer->sub->id = i;
    }

    if (codec->codec_id == CODEC_ID_TEXT ||
        codec->codec_id == AV_CODEC_ID_SUBRIP) {
        type = 't';
    } else if (codec->codec_id == CODEC_ID_MOV_TEXT) {
        type = 'm';
    } else if (codec->codec_id == CODEC_ID_SSA || AV_CODEC_ID_ASS == codec->codec_id) {
        type = 'a';
    } else if (codec->codec_id == CODEC_ID_DVD_SUBTITLE) {
        type = 'v';
    } else if (codec->codec_id == CODEC_ID_XSUB) {
        type = 'x';
    } else if (codec->codec_id == CODEC_ID_DVB_SUBTITLE) {
        type = 'b';
    } else if (codec->codec_id == CODEC_ID_DVB_TELETEXT) {
        type = 'd';
    } else if (codec->codec_id == CODEC_ID_HDMV_PGS_SUBTITLE) {
        type = 'p';
    }
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 14, 100)
    else if (codec->codec_id == CODEC_ID_EIA_608) {
        type = 'c';
    }
#endif
    else if (codec->codec_tag == MKTAG('c', '6', '0', '8')) {
        type = 'c';
    } else {
        MLOGW("[%s] Not support codec id:0x%x tag:%d\n", __func__, codec->codec_id, codec->codec_tag);
        return MT_FAILURE;
    }

    sh_sub = new_sh_sub_sid(demuxer, i, priv->sub_streams, lang ? lang->value : NULL);
    if (!sh_sub) {
        MLOGE("[%s] new stream error \n", __func__);
        return MT_FAILURE;
    }

    priv->sstreams[priv->sub_streams] = i;
    sh_sub->type = type;
    handle_stream_extradata(st, (sh_common_t *) sh_sub);
    if (title && title->value) {
        MLOGD("ID_SID_%d_NAME=%s\n", priv->sub_streams, title->value);
    }
    if (st->disposition & AV_DISPOSITION_DEFAULT) {
        sh_sub->default_track = 1;
    }

    return MT_SUCCESS;
}

static void handle_stream(demuxer_t *demuxer, AVFormatContext *avfc, int i)
{
    int stream_id;
    lavf_priv_t *priv = demuxer->priv;
    AVStream *st = avfc->streams[i];
    AVCodecContext *codec = st->codec;
    char *stream_type = NULL;
    AVDictionaryEntry *lang = av_dict_get(st->metadata, "language", NULL, 0);
    AVDictionaryEntry *title = av_dict_get(st->metadata, "title", NULL, 0);
    AVDictionaryEntry *size= av_dict_get(st->metadata, "size",    NULL, 0);//tizhang@20181022 for 105367
    AVDictionaryEntry *palette= av_dict_get(st->metadata, "palette",    NULL, 0);//tizhang@20190814 for 111908

    switch (codec->codec_type) {
        case AVMEDIA_TYPE_AUDIO: {
            if (MT_SUCCESS == handle_stream_audio(demuxer, avfc, i)) {
                stream_type = "aud";
                stream_id   = priv->audio_streams++;
            }
            break;
        }
        case AVMEDIA_TYPE_VIDEO: {
            if (MT_SUCCESS == handle_stream_video(demuxer, avfc, i)) {
                stream_type = "vid";
                stream_id   = priv->video_streams++;
            }
            break;
        }
        case AVMEDIA_TYPE_SUBTITLE: {
            if (MT_SUCCESS == handle_stream_subtitle(demuxer, avfc, i)) {
                stream_type = "sub";
                stream_id   = priv->sub_streams++;
            }
            break;
        }
        case AVMEDIA_TYPE_ATTACHMENT: {
            if (st->codec->codec_id == CODEC_ID_TTF) {
                AVDictionaryEntry *fnametag = av_dict_get(st->metadata, "filename", NULL, 0);
                demuxer_add_attachment(demuxer, fnametag ? fnametag->value : NULL,
                                       "application/x-truetype-font",
                                       codec->extradata, codec->extradata_size);
            }
            break;
        }
        default: {
            st->discard = AVDISCARD_ALL;
            break;
        }
    }

    if (stream_type) {
        const char *codec_name = avcodec_get_name(codec->codec_id);
        MLOGI("[%d] Append stream %d %s (%s), time num:%d den:%d lang:%s title:%s\n", i,
            stream_id, stream_type, codec_name, st->time_base.num, st->time_base.den,
            (lang && lang->value) ? lang->value : "NULL", (title && title->value) ? title->value : "NULL");

        AVCodec *avc = avcodec_find_decoder(codec->codec_id);
        codec_name = avc ? avc->name : "unknown";
        if (!avc && *stream_type == 's' && demuxer->s_streams[i]) {
            codec_name = sh_sub_type2str(((sh_sub_t *)demuxer->s_streams[i])->type);
        }

        if (((lang && lang->value) || (title && title->value)) && (*stream_type == 's')           &&
             ((0 == strcmp(codec_name,  "ass"))     || (0 == strcmp(codec_name,  "ssa"))          ||
              (0 == strcmp(codec_name,  "dvb"))     || (0 == strcmp(codec_name,  "dvb-teletext")) ||
              (0 == strcmp(codec_name,  "text"))    || (0 == strcmp(codec_name,  "srt"))          ||
              (0 == strcmp(codec_name,  "vobsub"))  || (0 == strcmp(codec_name,  "movtext")) )) { //tizhang@20180822 for 104045
          if(demuxer->subt_info.cnt < DEMUX_SUBTITLE_CNT) {//tizhang@20180380 for 104443

            demuxer->subt_info.subtitle[demuxer->subt_info.cnt].id = i;
            strncpy(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].code, codec_name, DEMUX_SUBTITLE_LEN);
            if (lang && lang->value) {
                strncpy(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].lang, lang->value, DEMUX_SUBTITLE_LEN);
            } else {
                strncpy(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].lang, "unknown", 7);
            }
            if (title && title->value) {
                strncpy(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].title, title->value, DEMUX_SUBTITLE_LEN);
            } else {
                strncpy(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].title, "unknown", 7);
            }

            if(0 == strcmp(codec_name,  "vobsub") && size){//tizhang@20181022 for 105367
      #define SIZE_START_POINT 13   //SIZE_START_POINT + 7(The number needs to be attached to title) = DEMUX_SUBTITLE_LEN
                int tlen = 0;
                char csub[7];
                int hlen = 0;

                if(size->value && strlen(size->value) > 3){
                    csub[0] = ' ';
                    for(tlen=strlen(size->value)-1; tlen>=0; tlen--){
                        if(size->value[tlen] >= '0' && size->value[tlen] <= '9')
                            hlen++;
                        else
                            break;
                    }

                    strncpy(csub+1, size->value+strlen(size->value) - hlen, hlen);
                    csub[hlen+1] = 0;
                    csub[hlen+2] = 0;
                }
                else{
                    strncpy(csub, " 00000", 6);
                    csub[6] = 0;
                }

                if(strstr(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].title, csub+1) == 0){
                    if(hlen)
                        csub[hlen+1] = 'p';
                    tlen = strlen(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].title);
                    if(tlen > SIZE_START_POINT - 1)
                        tlen = SIZE_START_POINT - 1;
                    if(size->value)
                    {
                        strncpy(demuxer->subt_info.subtitle[demuxer->subt_info.cnt].title+tlen, csub, DEMUX_SUBTITLE_LEN-SIZE_START_POINT);
                    }
                }
            }
            if(0 == strcmp(codec_name,  "vobsub") && (size ||palette)){//tizhang@20190814 for 111908
                int offset = 0;

                memset(g_subt_ext_param, 0, VOB_SUB_PARAM_LEN);
                if(size){
                    sprintf(g_subt_ext_param, "%s%s%s", "size: ", size->value, "\n");
                    offset = strlen("size: ") + strlen(size->value) + strlen("\n");
                }
                if(palette){
                    sprintf(g_subt_ext_param + offset, "%s%s%s", "palette: ", palette->value, "\n");
                    offset = strlen("palette: ") + strlen(palette->value) + strlen("\n");
                }
                MLOGA("\n[%s_%d]vobsub param:%s, len=%d\n",__func__,__LINE__,g_subt_ext_param, strlen(g_subt_ext_param));
            }
            demuxer->subt_info.cnt++;
        }
      }
      if(demuxer->subt_info.cnt == 0){
          demuxer->sub->id = -1;
        }
    }
}

extern int g_is_live_broadcast;
#if 1
typedef struct H264BSFContext {
	uint8_t  length_size;
	uint8_t  first_idr;
	int      extradata_parsed;
       int add_header;
} H264BSFContext;
//static H264BSFContext ctx1;

typedef struct HEVCBSFContext {
    uint8_t  length_size;
    int      extradata_parsed;

    int logged_nonmp4_warning;

    /* When private_spspps is zero then spspps_buf points to global codec_extradata
       and bsf does replace a global codec_extradata to own-allocated version (default
       behaviour).
       When private_spspps is non-zero the bsf uses a private version of spspps buf.
       This mode necessary when bsf uses in decoder, else bsf has issues after
       decoder re-initialization. Use the "private_spspps_buf" argument to
       activate this mode.
     */
    int      private_spspps;
    uint8_t *spspps_buf;
    uint32_t spspps_size;
} HEVCBSFContext;
//static HEVCBSFContext ctx_hevc;
#endif

extern AVFormatContext * p_lavf_avfc;
typedef enum filter_type{
    FILTER_H264 = 0,
    FILTER_HEVC,
    FILTER_MPEG4
}filter_type_e;

const static AVClass *s_lavf_next_ioclass = NULL;
static void *lavf_avio_child_next(void *obj, void *prev)
{
    AVIOContext *s = obj;
    demuxer_t *demuxer = s->opaque;
    return prev ? NULL : demuxer->stream->priv;
}

static const AVClass *lavf_avio_child_class_next(const AVClass *prev)
{
    return prev ? NULL : s_lavf_next_ioclass;
}

const AVClass lavf_avio_class = {
    .class_name = "LAVF IO Context",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
    .option     = NULL,
    .child_next = lavf_avio_child_next,
    .child_class_next = lavf_avio_child_class_next,
};

static void * demux_lavf_get_filter_para(demuxer_t *demuxer, int type)
{
    void *ret = NULL;
    //demuxer = demuxer;        // for tscan
    switch(type){
        case FILTER_H264:
            if(h264bsfc){
                ret = ((AVBitStreamFilterContext *)h264bsfc)->priv_data;
            }
            break;
        case FILTER_HEVC:
            if(h265bsfc){
                ret = ((AVBitStreamFilterContext *)h265bsfc)->priv_data;
            }
            break;
        case FILTER_MPEG4:
            if(mpeg4bsfc){
                ;
            }
            break;
        default:
            break;
    }

    return ret;
}

static int demux_lavf_set_filter_para(demuxer_t *demuxer, int type, void *arg)
{
    int ret = 0;

    if(arg == NULL){
        ret = -1;
        goto END;
    }
    switch(type){
        case FILTER_H264:
            if(h264bsfc){
                if( ((AVBitStreamFilterContext *)h264bsfc)->priv_data )
                    *(H264BSFContext *)(((AVBitStreamFilterContext *)h264bsfc)->priv_data) = *(H264BSFContext *)arg;
            }
            break;
        case FILTER_HEVC:
            if(h265bsfc){
                if( ((AVBitStreamFilterContext *)h265bsfc)->priv_data )
                    *(HEVCBSFContext *)(((AVBitStreamFilterContext *)h265bsfc)->priv_data) = *(HEVCBSFContext *)arg;
            }
            break;
        case FILTER_MPEG4:
            if(mpeg4bsfc){
                ;
            }
            break;
        default:
            break;
    }

END:
    return ret;
}

static int demux_lavf_set_add_head(demuxer_t *demuxer, int arg)
{
    H264BSFContext *h264_arg = NULL;
//    HEVCBSFContext *hevc_arg = NULL;
    int ret = 0;
#ifndef CFG_ENABLE_FFMPEG_422

    //MLOGA("\n[%s_%d] arg=%d,%p\n",__func__,__LINE__,arg,h264bsfc);
    h264_arg = demux_lavf_get_filter_para(demuxer, FILTER_H264);
    //MLOGA("\n[%s_%d] arg=%d,%p\n",__func__,__LINE__,arg,h264_arg);
    if(h264_arg){
        h264_arg->add_header = arg;
        ret = demux_lavf_set_filter_para(demuxer, FILTER_H264, h264_arg);
    }else{
        ret = -1;
    }
#else
    //the H264BSFContext struct is not the same in ffmpeg2 and ffmpeg4
    h264_arg = (H264BSFContext *)(unsigned long)arg;

#endif
    return ret;
}

static int detect_mpeg2(demuxer_t *demuxer)
{
    off_t tmppos;
    int ret = 0;
//	int i;
	unsigned char buf[4];

    tmppos = stream_tell(demuxer->stream);

    memset(buf,0,4);

    stream_seek(demuxer->stream, 0);
    stream_read(demuxer->stream, buf, 4);
    stream_seek(demuxer->stream, tmppos);

    if(buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x01 && buf[3] == 0xb3)   //00 00 01 b3 means mpeg video file header
    {
        ret = 1;
    }

    return ret;
}

static float get_frame_rate_value(unsigned int frame_rate_code)
{
    float ret = -1.0;

    switch(frame_rate_code)
    {
        case 1:
            ret = 24000.0/1001.0;
        break;
        case 2:
            ret = 24.0;
        break;
        case 3:
            ret = 25.0;
        break;
        case 4:
            ret = 30000.0/1001.0;
        break;
        case 5:
            ret = 30.0;
        break;
        case 6:

            ret = 50.0;
        break;
        case 7:
            ret = 60000.0/1001.0;
        break;
        case 8:
            ret = 60.0;
        break;
        default:
        break;
    }

    return ret;
}

static int get_frame_fps(unsigned char *buf,int len)
{
    float fps = -1.0;
    int i;
    unsigned char *ext_buf;
    unsigned int rate_value;
    float rate_code;

    rate_code = buf[7]&0xf;

    int state = -1;

    for(i = 0; i < len; i ++)
    {
        state = (state << 8) | buf[i];

        if (state == 0x000001b5)  //00 00 01 b5 means mpeg2 extension header
        {
            break;
        }
    }

    if(i == len)
    {
        return 0;
    }

    ext_buf = buf + i + 1;

    int frame_rate_extension_n;
    int frame_rate_extension_d;

    frame_rate_extension_n = (ext_buf[5] >> 1)&0x3;
    frame_rate_extension_d = ((ext_buf[5]&1) << 4)|((ext_buf[6] >> 4)&0xf);

    rate_value = get_frame_rate_value(rate_code);
    fps = rate_value*(frame_rate_extension_n + 1) + frame_rate_extension_d + 1;

    return (int)fps;
}

static int get_mpeg2_ibps(demuxer_t *demuxer)
{
    off_t tmppos;

    tmppos = stream_tell(demuxer->stream);

    stream_seek(demuxer->stream, 0);

    unsigned char buf[2048];
    int len, i;
    int state;
    int cnt = 0;
    off_t tmppos2 = 0;
//    int all = 0;
//    int gop_len;
    int fps = 0;

    state = -1;

    while(1)
    {
        len = stream_read(demuxer->stream, buf, 2048);

        if(fps == 0)
        {
            fps = get_frame_fps(buf,len);

            if(fps == 0)
            {
                return 0;
            }
        }

        if(len != 2048)
        {
            break;
        }

        for(i = 0; i < len; i ++)
        {
            state = (state << 8) | buf[i];

            if (state == 0x00000100)  //00 00 01 00 means mpeg2 picture header
            {
                cnt ++;
                if(cnt == fps)
                {
                    tmppos2 = stream_tell(demuxer->stream);
                    goto exit1;
                }
            }
        }
    }

exit1:

    while(1)
    {
        len = stream_read(demuxer->stream, buf, 2048);

        if(len != 2048)
        {
            break;
        }

        for(i = 0; i < len; i ++)
        {
            state = (state << 8) | buf[i];

            if (state == 0x000001B3)
            {
                tmppos2 += i - 3;
                goto exit2;
            }
        }

        tmppos2 += len;
    }

exit2:
    stream_seek(demuxer->stream, tmppos);

    return (int)tmppos2;
}

static inline void set_demux_seekable(
    demuxer_t *demuxer, lavf_priv_t *priv)
{
    if (priv->pb) {
        demuxer->seekable = priv->pb->seekable;
        MLOGI("set demux seekable %d \n", demuxer->seekable);
    }
}

static int open_av_input(demuxer_t *demuxer, AVFormatContext **fmt_ctx,
    const char *filename, AVInputFormat *fmt, AVDictionary **options)
{
    int ret = 0;

    if (!avformat_istream_is_opened(demuxer)) {
        ret = avformat_open_input(fmt_ctx, filename, fmt, options);
    }
    return ret;
}

static AVFormatContext *avformat_get_context(demuxer_t *demuxer)
{
    if (avformat_istream_is_opened(demuxer)) {
        return demuxer->stream->priv;
    } else {
        return avformat_alloc_context();
    }
}

static void alloc_lavf_ioctx(AVFormatContext *avfc,
    demuxer_t *demuxer, lavf_priv_t *priv)
{
    if (avformat_istream_is_opened(demuxer)) {
        return;
    }

    if (!(priv->avif->flags & AVFMT_NOFILE)) {
        int bio_buffer_size = 0;
		if(g_player_buffer_size_mode==0)
			bio_buffer_size = 262144;//256*1024
		else
			bio_buffer_size = 1048576;//1024*1024

        if(!priv->buffer) {
            priv->buffer = (uint8_t *)calloc33(sizeof(uint8_t),bio_buffer_size);
        }
        if(!priv->buffer) {
            MLOGE("[%s]----calloc is failed!!! -------\n", __FUNCTION__);
        }

        priv->pb = avio_alloc_context(
            priv->buffer, bio_buffer_size, 0, demuxer, mp_read, NULL, mp_seek);
        priv->pb->read_seek = mp_read_seek;
        if (!demuxer->stream->end_pos ||
            (demuxer->stream->flags & MP_STREAM_SEEK) != MP_STREAM_SEEK) {
            priv->pb->seekable = 0;
        }

        unsigned int source_type;
        int ret = stream_control(demuxer->stream,
            STREAM_CTRL_GET_SOURCE_TYPE, &source_type);
        if (STREAM_OK == ret &&
            STREAM_SOURCE_TYPE_EXTERNAL_IO == source_type) {
            FILE_PLAYBACK_EXTIO_CONTEXT_T *ctx =
                (FILE_PLAYBACK_EXTIO_CONTEXT_T * )demuxer->stream->priv;
            int stream_type = demuxer->stream->type;
            if (STREAMTYPE_FILE == stream_type) {
                priv->pb->seekable = 1;
            } else if (STREAMTYPE_STREAM == stream_type) {
                priv->pb->seekable = 0;
                priv->pb->seek = NULL;
                priv->pb->read_seek = NULL;
            }
            avfc->pb = priv->pb;
            MLOGI("[%s] use external io, seekable:%d, stream end_pos: %lld, flags: 0x%x\n",
                __func__, priv->pb->seekable, demuxer->stream->end_pos, demuxer->stream->flags);
        } else {
            AVIOContext *ctx = (AVIOContext * )demuxer->stream->priv;
            if(ctx && ctx->seekable == 1 && ctx->is_chunked == 1) {
                priv->pb->seekable = 1;
            }

            avfc->pb = priv->pb;
            avfc->pb->av_class = &lavf_avio_class;
            if (!s_lavf_next_ioclass && ctx) {
                s_lavf_next_ioclass = ctx->av_class;
            }
            MLOGI("[%s] seekable:%d, stream end_pos: %lld, flags: 0x%x\n",
                __func__, priv->pb->seekable, demuxer->stream->end_pos, demuxer->stream->flags);
        }
    }
}

#ifdef CFG_ENABLE_FFMPEG_422
#define DEFAULT_PENALTY_FACTOR    8

static inline int is_same_codec(AVStream *st, AVStream *ast)
{
    if (st->codecpar->codec_type == ast->codecpar->codec_type &&
        st->codecpar->codec_id   == ast->codecpar->codec_id) {
        return 1;
    }
    return 0;
}

static int need_switch_playlist(demuxer_t *demux)
{
    lavf_priv_t *lavf = demux->priv;

    if (-1 == lavf->current_plsidx  ||
        -1 == lavf->expected_plsidx) {
        return 0;
    }

    return (int) (lavf->current_plsidx != lavf->expected_plsidx);
}

static void punish_playlist_timer(
    AVAdaptiveList *lists, int n_list_num, int next_index)
{
    if (next_index < 0 ||
        next_index >= n_list_num) {
        return;
    }
    for (int i = 0; i <= next_index; i++) {
        if (AVMEDIA_TYPE_VIDEO != lists[i].type) {
            continue;
        }
        lists[i].priority++;
        /* 30 segs ok, get a bonus and minus penalty_cnt */
        if (lists[i].priority > 30) {
            if (lists[i].penalty_cnt >= 1) {
                lists[i].penalty_cnt--;
            }
            lists[i].priority = 0;
        }
    }
}

static void punish_playlist(
    AVAdaptiveList *lists, int n_list_num, int curr_index)
{
    lists[curr_index].penalty_cnt++;
    lists[curr_index].priority =
        -1 * DEFAULT_PENALTY_FACTOR * lists[curr_index].penalty_cnt;

    for (int i = curr_index + 1; i < n_list_num; i++) {
        if (AVMEDIA_TYPE_VIDEO != lists[i].type) {
            continue;
        }
        lists[i].priority += lists[curr_index].priority;
    }
}

static int check_valid_manual_expected(
    demuxer_t *demuxer, int expected_plsidx)
{
    lavf_priv_t *lavf = demuxer->priv;
    AVFormatContext *ctx = lavf->avfc;
    AVAdaptive *adaptive = &ctx->adaptive;
    /* auto mode decide by us */
    if (expected_plsidx >= 0                         &&
        expected_plsidx < adaptive->n_list_num       &&
        lavf->current_plsidx  >= 0                   &&
        lavf->current_plsidx  < adaptive->n_list_num &&
        expected_plsidx != lavf->current_plsidx      &&
        AV_ADAPTIVE_RESOLUTION_MODE_MANUAL == adaptive->adaptive_resolution_mode) {
        AVAdaptiveList *list = &ctx->adaptive.lists[expected_plsidx];
        if (list->enable) {
            return 1;
        }
    }
    return 0;
}

static inline enum AVCodecID find_playlist_codec_id(AVAdaptiveList *list)
{
    int i;
    enum AVCodecID codec_id = AV_CODEC_ID_NONE;
    for (i = 0; i < list->nb_streams; i++) {
        AVStream *st = list->streams[i];
        if (AVMEDIA_TYPE_VIDEO != st->codecpar->codec_type) {
            continue;
        }

        if (st->discard < AVDISCARD_ALL) {
            codec_id = st->codecpar->codec_id;
            break;
        }
    }
    return codec_id;
}

static int find_playlist_last_index(
    AVFormatContext *s, AVStream *st)
{
    int i, j;
    AVAdaptiveList *lists = s->adaptive.lists;
    /* Find last playlist */
    for (i = s->adaptive.n_list_num - 1; i >= 0; i--) {
        for (j = 0; j < lists[i].nb_streams; j++) {
            if (is_same_codec(st, lists[i].streams[j])) {
                return i;
            }
        }
    }
    return -1;
}

static int find_playlist_first_index(
    AVFormatContext *s, AVStream *st)
{
    int i, j;
    AVAdaptiveList *lists = s->adaptive.lists;
    /* Find first playlist */
    for (i = 0; i < s->adaptive.n_list_num; i++) {
        for (j = 0; j < lists[i].nb_streams; j++) {
            if (is_same_codec(st, lists[i].streams[j])) {
                return i;
            }
        }
    }
    return -1;
}

static void find_playlist_boundary(lavf_priv_t *lavf)
{
    AVFormatContext *ctx = lavf->avfc;
    AVAdaptiveList *lists = ctx->adaptive.lists;

    AVStream *st = NULL;
    for (int i = 0; i < lists[lavf->current_plsidx].nb_streams; i++) {
        AVStream *ist = lists[lavf->current_plsidx].streams[i];
        if (ist->discard < AVDISCARD_ALL &&
            AVMEDIA_TYPE_VIDEO == ist->codecpar->codec_type) {
            st = ist;
            break;
        }
    }
    if (!st) {
        return;
    }

    if (lavf->expected_plsidx < 0) {
        lavf->expected_plsidx = find_playlist_first_index(ctx, st);
    } else if (lavf->expected_plsidx >= ctx->adaptive.n_list_num) {
        lavf->expected_plsidx = find_playlist_last_index(ctx, st);
    }

    if (lavf->expected_plsidx < 0            ||
        !lists[lavf->expected_plsidx].enable) {
        lavf->expected_plsidx = lavf->current_plsidx;
    }
}

static void find_playlist_expected_by_br(
    AVFormatContext *s, int64_t bitrate)
{
    int i = 0, j = 0;
    demuxer_t *demuxer  = s->opaque;
    lavf_priv_t  *priv  = demuxer->priv;
    AVAdaptiveList *lists = s->adaptive.lists;

    enum AVCodecID codec_id =
        find_playlist_codec_id(&lists[priv->current_plsidx]);
    if (AV_CODEC_ID_NONE == codec_id) {
        return;
    }

    for (i = 0; i < s->adaptive.n_list_num; i++) {
        AVAdaptiveList *l = &lists[i];
        if (AVMEDIA_TYPE_VIDEO != l->type) {
            continue;
        }
        for (j = 0; j < l->nb_streams; j++) {
            AVStream *st = l->streams[j];
            if (codec_id == st->codecpar->codec_id) {
                if (l->bandwidth <= bitrate) {
                    priv->expected_plsidx = i;
                }
            }
        }
    }
}

static void find_playlist_expected_by_dir(
    AVFormatContext *s, lavf_priv_t *lavf, int dir)
{
    int i = 0, j = 0;
    AVAdaptiveList *lists = s->adaptive.lists;
    enum AVCodecID codec_id =
        find_playlist_codec_id(&lists[lavf->current_plsidx]);

    if (AV_CODEC_ID_NONE == codec_id) {
        return;
    }

    lavf->expected_plsidx =
        dir < 0 ? -1 : s->adaptive.n_list_num;
    for (i = lavf->current_plsidx + dir;
         i >= 0 && i < s->adaptive.n_list_num; i += dir) {
        AVAdaptiveList *l = &lists[i];
        if (AVMEDIA_TYPE_VIDEO != l->type) {
            continue;
        }
        for (j = 0; j < l->nb_streams; j++) {
            AVStream *st = l->streams[j];
            if (codec_id == st->codecpar->codec_id) {
                lavf->expected_plsidx = i;
                return;
            }
        }
    }
}

static void find_playlist_current(AVFormatContext *s)
{
    int i = 0, j = 0;
    demuxer_t *demuxer  = s->opaque;
    lavf_priv_t  *priv  = demuxer->priv;
    AVAdaptiveList *lists = s->adaptive.lists;
    /* has current video playlist */
    if (0 <= priv->current_plsidx &&
        s->adaptive.n_list_num > priv->current_plsidx &&
        AVMEDIA_TYPE_VIDEO == lists[priv->current_plsidx].type) {
        return;
    }

    /* Find current playlist */
    for (i = 0; i < s->adaptive.n_list_num; i++) {
        for (j = 0; j < lists[i].nb_streams; j++) {
            AVStream *st = lists[i].streams[j];
            if (st->discard < AVDISCARD_ALL &&
                AVMEDIA_TYPE_VIDEO == st->codecpar->codec_type) {
                priv->current_plsidx = i;
                return;
            }
        }
    }
}

static int find_playlist_by_mode(
    demuxer_t *demuxer, lavf_priv_t *lavf, int mode)
{
    AVFormatContext *ctx = lavf->avfc;
    AVAdaptiveList *lists = ctx->adaptive.lists;

    punish_playlist_timer(lists,
        ctx->adaptive.n_list_num, lavf->expected_plsidx);
    if (DEMUXER_BITRATE_CHANGE_NONE == mode) {
        return 0;
    }

    if (-1 == lavf->current_plsidx ||
        lavf->current_plsidx >= ctx->adaptive.n_list_num) {
        return 0;
    }

    if (DEMUXER_BITRATE_CHANGE_STEP_UP == mode ||
        DEMUXER_BITRATE_CHANGE_STEP_DOWN == mode) {
        int dir = (DEMUXER_BITRATE_CHANGE_STEP_UP == mode) ? 1 : -1;
        find_playlist_expected_by_dir(ctx, lavf, dir);
    }

    find_playlist_boundary(lavf);

    /* not allow change upwards in punishment */
    if (DEMUXER_BITRATE_CHANGE_NONE == mode ||
        (lists[lavf->expected_plsidx].priority < 0 &&
        lavf->expected_plsidx > lavf->current_plsidx)) {
        lavf->expected_plsidx = lavf->current_plsidx;
    }
    if (lavf->expected_plsidx == lavf->current_plsidx) {
        return 0;
    }

    /* if we try too many times from high to low, not allow to change anymore */
    if (lavf->expected_plsidx != lavf->current_plsidx) {
        if (lists[lavf->current_plsidx].bandwidth >
            lists[lavf->expected_plsidx].bandwidth) {
            punish_playlist(lists,
                ctx->adaptive.n_list_num, lavf->current_plsidx);
        }
    }

    return need_switch_playlist(demuxer);
}

static int handle_adaptive_bitrate_update_message(AVFormatContext *s, int64_t bitrate)
{
    int i = 0, j = 0;
    demuxer_t *demuxer  = s->opaque;
    lavf_priv_t  *priv  = demuxer->priv;
    AVAdaptiveList *lists = s->adaptive.lists;

    find_playlist_current(s);
    if (-1 == priv->current_plsidx) {
        av_log(s, AV_LOG_WARNING, "[%s] Cannot find out current playlist\n", __func__);
        priv->expected_plsidx = -1;
        return 0;
    }
    if (AV_ADAPTIVE_RESOLUTION_MODE_MANUAL ==
        s->adaptive.adaptive_resolution_mode) {
        if (check_valid_manual_expected(demuxer, priv->usr_next_plsidx)) {
            priv->expected_plsidx = priv->usr_next_plsidx;
            priv->usr_next_plsidx = -1;
            return 1;
        }
        priv->expected_plsidx = -1;
        return 0;
    }

    priv->expected_plsidx = -1;
    find_playlist_expected_by_br(s, bitrate);
    if (-1 == priv->expected_plsidx) {
        av_log(s, AV_LOG_WARNING,
            "[%s] Cannot find out expected playlist by bitrate %lld\n", __func__, bitrate);
        return 0;
    }

    int need_change = DEMUXER_BITRATE_CHANGE_NONE;
    if (lists[priv->expected_plsidx].bandwidth >
        lists[priv->current_plsidx].bandwidth) {
        need_change = DEMUXER_BITRATE_CHANGE_NOR_UP;
    } else if (lists[priv->expected_plsidx].bandwidth <
        lists[priv->current_plsidx].bandwidth) {
        need_change = DEMUXER_BITRATE_CHANGE_NOR_DOWN;
    }

    int mode = demuxer->app_message_callback(
                demuxer->callback_ctx,
                DEMUXER_ADAPTIVE_BITRATE_UPDATED,
                &need_change, sizeof(need_change));
    need_change = find_playlist_by_mode(demuxer, priv, mode);
    int expected_plsidx = priv->expected_plsidx;
    av_log(s, AV_LOG_INFO, "[%d->%d] Br:%lld bandwidth:[%d->%d] en:%d pri:%d %d mode:%d need:%d\n",
        priv->current_plsidx, expected_plsidx, (long long int) bitrate,
        lists[priv->current_plsidx].bandwidth, lists[expected_plsidx].bandwidth,
        lists[priv->current_plsidx].enable,
        lists[priv->current_plsidx].priority,
        -1 == expected_plsidx ? 0 : lists[expected_plsidx].priority, mode, need_change);
    if (!need_change) {
        priv->expected_plsidx = -1;
    }
    return need_change;
}

static int handle_adaptive_query_idle_time(AVFormatContext *s, int64_t *idle_time)
{
    int ret = 0;
    int64_t time = 0;
    demuxer_t *demuxer = s->opaque;
    ret = demuxer->app_message_callback(demuxer->callback_ctx,
            DEMUXER_ADAPTIVE_QUERY_IDLE_TIME, &time, sizeof(idle_time));
    *idle_time = time;
    return ret;
}

static int make_demuxer_adaptive_playlist(demuxer_t *demuxer, lavf_priv_t *lavf)
{
    AVFormatContext *ctx = lavf->avfc;
    AVAdaptiveList *lists = ctx->adaptive.lists;

    find_playlist_current(ctx);
    if (lavf->current_plsidx < 0 || ctx->adaptive.n_list_num <= 0) {
        return DEMUXER_CTRL_DONTKNOW;
    }
    if (demuxer->num_adaptive_playlist != ctx->adaptive.n_list_num) {
        free33(demuxer->adaptive_playlist);
        demuxer->num_adaptive_playlist = ctx->adaptive.n_list_num;
        demuxer->adaptive_playlist = (demuxer_adaptive_playlist_t *)
            calloc33(demuxer->num_adaptive_playlist, sizeof(demuxer_adaptive_playlist_t));
    }
    if (!demuxer->adaptive_playlist) {
        demuxer->num_adaptive_playlist = 0;
        return DEMUXER_CTRL_DONTKNOW;
    }

    enum AVCodecID codec_id =
        find_playlist_codec_id(&lists[lavf->current_plsidx]);
    for (int num = 0; num < demuxer->num_adaptive_playlist; num++) {
        for (int stream_id = 0; stream_id < lists[num].nb_streams; stream_id++) {
            AVStream *st = lists[num].streams[stream_id];
            AVCodecParameters *codecpar = st->codecpar;
            if (AVMEDIA_TYPE_VIDEO != codecpar->codec_type){
                continue;
            }
            demuxer_adaptive_playlist_t *l =
                &demuxer->adaptive_playlist[num];
            l->flags  = ADAPTIVE_PLAYLIST_FLAGS_NONE;
            l->width  = lists[num].width  > 0 ? lists[num].width  : codecpar->width;
            l->height = lists[num].height > 0 ? lists[num].height : codecpar->height;
            l->index     = num;
            l->bandwidth = lists[num].bandwidth;

            AVRational frame_rate =
                av_guess_frame_rate(ctx, st, NULL);
            l->framerate.num = lists[num].framerate.num > 0
                ? lists[num].framerate.num : frame_rate.num;
            l->framerate.den = lists[num].framerate.den > 0
                ? lists[num].framerate.den : frame_rate.den;
            l->codec_tag = mp_codec_id2tag(
                codecpar->codec_id, codecpar->codec_tag, 0);
            if (codecpar->codec_id == codec_id) {
                l->flags = ADAPTIVE_PLAYLIST_FLAGS_ENABLE;
            } else {
                lists[num].enable = 0;
            }
        }
    }
    return DEMUXER_CTRL_OK;
}

static int adaptive_to_app_message(
    AVFormatContext *s, int type, void *data, size_t data_size)
{
    int ret = 0;
    demuxer_t *demuxer = s->opaque;
    if (!demuxer) {
        return 0;
    }

    lavf_priv_t *lavf = demuxer->priv;
    if (AV_ADAPTIVE_BITRATE_UPDATED == type && data && data_size == sizeof(int64_t)) {
        int64_t bitrate = *((int64_t *) data);
        return handle_adaptive_bitrate_update_message(s, bitrate);
    } else if (AV_ADAPTIVE_LIVE_MEDIA_UPDATE == type && data && data_size == sizeof(int)) {
        if (lavf->is_live) {
            return 0; /* only report once */
        }
        lavf->is_live = *((int *) data);
        return demuxer->app_message_callback(
            demuxer->callback_ctx, DEMUXER_ADAPTIVE_LIVE_MEDIA_UPDATED, data, data_size);
    } else if (AV_ADAPTIVE_QUERY_IDLE_TIME == type && data && data_size == sizeof(int64_t)) {
        return handle_adaptive_query_idle_time(s, (int64_t *) data);
    } else if (AV_ADAPTIVE_QUERY_PLAY_STATE == type && data && data_size == sizeof(int)) {
        return demuxer->app_message_callback(
            demuxer->callback_ctx, DEMUXER_ADAPTIVE_QUERY_PLAY_STATE, data, data_size);
    } else if (AV_ADAPTIVE_STREAMING_SERVER_ERROR == type && data && data_size == sizeof(int)) {
        return demuxer->app_message_callback(
            demuxer->callback_ctx, DEMUXER_ADAPTIVE_SERVER_ERROR, data, data_size);
    } else if (AV_ADAPTIVE_QUERY_ASYNC_OPEN == type) {
        return demuxer->app_message_callback(
            demuxer->callback_ctx, DEMUXER_ADAPTIVE_ASYNC_OPEN, NULL, 0);
    } else if (AV_ADAPTIVE_QUERY_ASYNC_DONE == type) {
        ret = demuxer->app_message_callback(
            demuxer->callback_ctx, DEMUXER_ADAPTIVE_ASYNC_DONE, NULL, 0);
        (void) make_demuxer_adaptive_playlist(demuxer, lavf);
        return ret;
    }

    return 0;
}

static int transport_desdec_func(void *h,
        const void *info, const unsigned char *in, unsigned char *out, int len)
{
    AVFormatContext *s = h;
    demuxer_t *demuxer = s->opaque;
    if (!in || !out || !len ||
        !demuxer->stream->transport_desdec_func) {
        return -1;
    }
    return demuxer->stream->transport_desdec_func(
        demuxer->stream->transport_desdec_ctx, info, in, out, len);
}

static int control_message_callback(
    AVFormatContext *s, int type, void *data, size_t data_size)
{
    if (!s) {
        return -1;
    }

    if (AV_TRANSPORT_QUERY_DESDEC_CB == type) {
        if (data && data_size == sizeof(intptr_t)) {
            demuxer_t *demuxer = s->opaque;
            *((intptr_t *) data) = (intptr_t) NULL;
            if (demuxer->stream->transport_desdec_func) {
                *((intptr_t *) data) = (intptr_t) (transport_desdec_func);
            }
            return 0;
        }
        return -1;
    }
    return adaptive_to_app_message(s, type, data, data_size);
}

static int get_adaptive_playlist(demuxer_t *demuxer, lavf_priv_t *lavf)
{
    AVFormatContext *ctx = lavf->avfc;
    AVAdaptiveList *lists = ctx->adaptive.lists;

    int ret = DEMUXER_CTRL_OK;
    if (!demuxer->adaptive_playlist) {
        ret = make_demuxer_adaptive_playlist(demuxer, lavf);
    }

    demuxer->idx_adaptive_playlist = lavf->current_plsidx;
    return ret;
}

static void switch_playlist_change_id(demux_stream_t *ds,
    AVFormatContext *ctx, AVStream *curr_stream, AVStream *stream)
{
    if (!ds || !ctx || !curr_stream || !stream) {
        return;
    }

    for (int j = 0; j < ctx->nb_streams; j++) {
        if (ctx->streams[j] == stream) {
            MLOGI("[%p]Change play list ok,stream [%d]%s->[%d]%s\n", ds,
                ds->id, avcodec_descriptor_get(curr_stream->codecpar->codec_id)->name,
                j, avcodec_descriptor_get(stream->codecpar->codec_id)->name);
            ds->id = j;
            ds->eof = 0;
            ds->discard = 0;
            break;
        }
    }
}

static int switch_expected_playlist(
    demuxer_t *demuxer, lavf_priv_t *lavf, int expected)
{
    AVFormatContext *ctx = lavf->avfc;
    AVAdaptiveList *lists = ctx->adaptive.lists;

    lavf->usr_next_plsidx = -1;
    find_playlist_current(ctx);
    if (-1 == lavf->current_plsidx ||
        lavf->current_plsidx >= ctx->adaptive.n_list_num) {
        return DEMUXER_CTRL_DONTKNOW;
    }
    if (!demuxer->adaptive_playlist) {
        get_adaptive_playlist(demuxer, lavf);
    }
    if (!check_valid_manual_expected(demuxer, expected)) {
        MLOGE("Change playlist not expected:%d current:%d\n",
            expected, lavf->current_plsidx);
        return DEMUXER_CTRL_DONTKNOW;
    }
    lavf->usr_next_plsidx = expected;
    return DEMUXER_CTRL_OK;
}

static void switch_playlist_l(demuxer_t *demuxer,
    AVFormatContext *ctx, int curr_index, int next_index)
{
    AVAdaptive *adaptive = &ctx->adaptive;
    if (curr_index == next_index ||
        curr_index < 0 || curr_index >= adaptive->n_list_num ||
        next_index < 0 || next_index >= adaptive->n_list_num) {
        return;
    }

    int i, j;
    AVAdaptiveList *lists = ctx->adaptive.lists;
    for (i = 0; i < lists[curr_index].nb_streams; i++) {
        AVStream *curr_stream = lists[curr_index].streams[i];
        if (AVDISCARD_ALL == curr_stream->discard) {
            continue;
        }
        demux_stream_t *ds    = NULL;
        AVStream *next_stream = NULL;
        /* Find out expected stream */
        for (j = 0; j < lists[next_index].nb_streams; j++) {
            next_stream = lists[next_index].streams[j];
            if (AVDISCARD_ALL != next_stream->discard ||
                next_stream->codecpar->codec_id != curr_stream->codecpar->codec_id) {
                next_stream = NULL;
                continue;
            }
            curr_stream->discard = AVDISCARD_ALL;
            next_stream->discard = AVDISCARD_DEFAULT;
            if (AVMEDIA_TYPE_AUDIO == curr_stream->codecpar->codec_type) {
                ds = demuxer->audio;
            } else if (AVMEDIA_TYPE_VIDEO == curr_stream->codecpar->codec_type) {
                ds = demuxer->video;
            }  else if (AVMEDIA_TYPE_SUBTITLE == curr_stream->codecpar->codec_type) {
                ds = demuxer->sub;
            }

            switch_playlist_change_id(ds, ctx, curr_stream, next_stream);
        }
    }
    /* Not allow change from low to high frequently */
    for (i = curr_index; i < ctx->adaptive.n_list_num; i++) {
        lists[i].priority -= 2;
    }
}

static void switch_playlist_mode_set(lavf_priv_t *lavf, int mode)
{
    AVFormatContext *ctx = lavf->avfc;
    AVAdaptive *adaptive = &ctx->adaptive;
    if (AV_ADAPTIVE_RESOLUTION_MODE_AUTO   <= mode &&
        AV_ADAPTIVE_RESOLUTION_MODE_MANUAL >= mode) {
        adaptive->adaptive_resolution_mode = mode;
    }
}
#endif

static demuxer_t *demux_open_lavf(demuxer_t *demuxer)
{
    AVFormatContext *avfc = NULL;
    AVDictionaryEntry *t  = NULL;
    lavf_priv_t *priv = demuxer->priv;
    int i, ret;
    char mp_filename[4096] = "";
    stream_t *stream = demuxer->stream;

    stream_seek(stream, 0);
    avfc = avformat_get_context(demuxer);
    if (NULL == avfc) {
        MLOGE("avformat get context fail\n");
        return NULL;
    }

#ifdef CFG_ENABLE_FFMPEG_422
    avfc->opaque                      = (void *) demuxer;
    avfc->control_message_cb          = control_message_callback;
#endif
    avfc->interrupt_callback.opaque   = demuxer->callback_ctx;
    avfc->interrupt_callback.callback = demuxer->interrupt_callback;
    if (opt_cryptokey) {
        parse_cryptokey(avfc, opt_cryptokey);
    }
    if (user_correct_pts != 0) {
        avfc->flags |= AVFMT_FLAG_GENPTS;
    }

    if (stream->max_stream_probe_size > 0) {
        avfc->probesize = stream->max_stream_probe_size;
    }
    if (stream->max_stream_analyze_duration > 0) {
        avfc->max_analyze_duration = stream->max_stream_analyze_duration * AV_TIME_BASE;
    }

    if (opt_avopt) {
        if (parse_avopts(avfc, opt_avopt) < 0) {
            mp_msg(MSGT_HEADER, MSGL_ERR, "Your options /%s/ look like gibberish to me pal\n", opt_avopt);
            stream->end_pos = 0;
            return NULL;
        }
    }

    if (stream->url) {
        ////add by libin, 20150119 fix bug55361, rtmp need probesize to get codec id
        if (!strncmp(stream->url, "rtmp://", 7)) {
            unsigned int probesize = 1024 * 1024;
            if (av_opt_set_int(avfc, "probesize", probesize, 0) < 0) {
                mp_msg(MSGT_HEADER, MSGL_ERR, "demux_lavf, couldn't set option probesize to %u\n", probesize);
            }
        }

        if (!strncmp(stream->url, "ffmpeg://rtsp:", 14)) {
            av_strlcpy(mp_filename, demuxer->stream->url + 9, sizeof(mp_filename));
        } else {
            if (stream->streaming_ctrl == NULL) {
                av_strlcat(mp_filename, stream->url, sizeof(mp_filename));
            } else {
                if (stream->streaming_ctrl->url) {
                    if (stream->streaming_ctrl->url->url) {
                        av_strlcat(mp_filename, stream->streaming_ctrl->url->url, sizeof(mp_filename));
                    } else {
                        av_strlcat(mp_filename, stream->url, sizeof(mp_filename));
                    }
                }
            }
        }
    } else {
        av_strlcat(mp_filename, "foobar.dummy", sizeof(mp_filename));
    }

    alloc_lavf_ioctx(avfc, demuxer, priv);
    AVDictionary *avformat_opts = NULL;
    AVDictionary **ff_dict_opts = (AVDictionary **) stream->ff_dict_opts;
    int copy_dict_options = (int) (ff_dict_opts && (*ff_dict_opts));
    if (copy_dict_options) {
        av_dict_copy(&avformat_opts, *ff_dict_opts, 0);
    } else {
        av_dict_copy(&avformat_opts, ff_avformat_opts, 0);
    }
    ret = open_av_input(demuxer, &avfc, mp_filename, priv->avif, &avformat_opts);
    av_dict_free(&avformat_opts);
    if (ret < 0) {
        MLOGE("LAVF_header: av_open_input_stream() failed\n");
        return NULL;
    }
    /* return use demux_ts, but unseekable stream cannot use */
    if (strstr(priv->avif->name, "mpegts") &&
        ((!(priv->pb) || priv->pb->seekable) &&
          !(stream->transport_scrambling_control))) { /* use ffmpeg ts handle live and scrambled stream */
        //transport_scrambling_control
        stream->end_pos = 0;
        if (strncmp(mp_filename, "http", 4) == 0) {
            return (demuxer_t*) DEMUXER_LAVF_INVALID_VALUE;
        } else {
            return NULL;
        }
    }
    if ((!(stream->transport_desdec_func)) &&
        (stream->transport_scrambling_control)) {
        MLOGW("Scramble stream without descramble function\n");
    }

    //for hls vod, seek is support
    if (strstr(priv->avif->name, "hls")) {
        demuxer->type_adaptive_stream = TYPE_ADAPTIVE_STREAM_HLS;
    } else if (strstr(priv->avif->name, "dash")) {
        demuxer->type_adaptive_stream = TYPE_ADAPTIVE_STREAM_DASH;
        priv->pb->seekable = 1;
    }

    priv->avfc = avfc;
#ifdef CFG_ENABLE_FFMPEG_422
    av_format_inject_global_side_data(avfc);
#endif
    if (avformat_find_stream_info(avfc, NULL) < 0) {
        MLOGE("[%s] find %s stream info failed\n", __func__, priv->avif->name);
        stream->end_pos = 0;
        return NULL;
    }

    /* Add metadata. */
    while ((t = av_dict_get(avfc->metadata, "", t, AV_DICT_IGNORE_SUFFIX))) {
        demux_info_add(demuxer, t->key, t->value);
    }

    for (i = 0; i < avfc->nb_chapters; i++) {
        AVChapter *c = avfc->chapters[i];
        uint64_t start = av_rescale_q(c->start, c->time_base, (AVRational) { 1, 1000 });
        uint64_t end = av_rescale_q(c->end, c->time_base, (AVRational) { 1, 1000 });
        t = av_dict_get(c->metadata, "title", NULL, 0);
        demuxer_add_chapter(demuxer, t ? t->value : NULL, start, end);
    }

    for (i = 0; i < avfc->nb_streams; i++) {
        handle_stream(demuxer, avfc, i);
    }
    priv->nb_streams_last = avfc->nb_streams;

    if (avfc->nb_programs) {
        int p;
        for (p = 0; p < avfc->nb_programs; p++) {
            AVProgram *program = avfc->programs[p];
            t = av_dict_get(program->metadata, "title", NULL, 0);
            MLOGD("LAVF: Program %d %s\n", program->id, t ? t->value : "");
            MLOGD("PROGRAM_ID=%d\n", program->id);
        }
    }

    MLOGD("LAVF: %d audio and %d video streams found\n", priv->audio_streams, priv->video_streams);
    MLOGD("LAVF: build %d\n", LIBAVFORMAT_BUILD);
    if (!priv->audio_streams) {
        demuxer->audio->id = -2;    // nosound
    }

    if (!priv->video_streams) {
        if (!priv->audio_streams) {
            MLOGE("LAVF: no audio or video headers found - broken file?\n");
            //yliu add
            stream->end_pos = 0;
            return NULL;
        }
        demuxer->video->id = -2; // audio-only
    }

    set_demux_seekable(demuxer, priv);
    if (strstr(priv->avif->name, "mp3")) {
        //mp3 demuxer has no pts output,
        //so disable sekk
        demuxer->seekable = 0;
        MLOGA("[%s]this is ffmpeg mp3 ,seek is NOT support \n", __func__);
    }

    if (strstr(priv->avif->name, "asf") == NULL) { //add by zhouxiang, for asf get filesize
        stream->end_pos = 0;
    }
    p_lavf_avfc = priv->avfc;

    priv->vc1_first_video_frame = 1;
    MLOGD("[%s] %d bsf_vcodec_flag:%d\n", __func__, __LINE__, bsf_vcodec_flag);

    h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb");
    h265bsfc =  av_bitstream_filter_init("hevc_mp4toannexb");
    mpeg4bsfc = av_bitstream_filter_init("dump_extra");
    demux_lavf_set_add_head(NULL, 1);

    if (strstr(priv->avif->name, "mpegvideo") && priv->video_streams && demuxer->video->id >= 0 && demuxer->video->id < MAX_V_STREAMS) {
        sh_video_t *sh_video = (sh_video_t *)(demuxer->v_streams[demuxer->video->id]);

        if (sh_video->i_bps == 0 && detect_mpeg2(demuxer) == 1) {
            sh_video->i_bps = get_mpeg2_ibps(demuxer);
        }
    }

    MLOGI("[%s] %d avinputformat name:%s\n", __func__, __LINE__,priv->avif->long_name);
    return demuxer;
}

#define ROUNDUP(x, y)           (((x) + (y) - 1) & ~((y) - 1))
static int get_packet_by_av_read(demuxer_t *demux, AVPacket *pkt)
{
    lavf_priv_t *priv   = demux->priv;
    AVInputFormat *avif = priv->avif;

    demux->filepos = stream_tell(demux->stream);
    int ret = av_read_frame(priv->avfc, pkt);
    MLOGA("[%d] pts:%lld %lldus size:%d pos:%lld ret:%d\n",
        pkt->stream_index, pkt->pts,
        av_rescale_q(pkt->pts,
            priv->avfc->streams[pkt->stream_index]->time_base, AV_TIME_BASE_Q),
        pkt->size, pkt->pos, ret);
    if(ret < 0) {
        MLOGW("av_read_frame %s\n", av_err2str(ret));
        return MT_ERR_NORES;
    }
    return MT_SUCCESS;
}

static int get_packet_filter(
    demuxer_t *demux, AVBitStreamFilterContext *abf, AVPacket *pkt)
{
    mt_u32 no_need_filter;
    lavf_priv_t *priv   = demux->priv;
    AVInputFormat *avif = priv->avif;

    //only h264 5 go here ,and there is problem
    no_need_filter = (pkt->stream_index != demux->video->id) || (NULL == abf) ||
        strstr(avif->name, "hls") || strstr(avif->name, "mondash") || pkt->is_secure;
#if !defined(VMX_OTT_SVP) && defined(CFG_ENABLE_FFMPEG_422)
    if (strstr(avif->name, "dash") ||
        (strstr(avif->name,"mov") && (0 == demux->trick.speed || 1 == demux->trick.speed)) ||
        av_packet_get_side_data(pkt, AV_PKT_DATA_ENCRYPTION_INFO, NULL)) {
        no_need_filter = MT_TRUE;
    }
#endif

    if (no_need_filter) {
        return MT_SUCCESS;
    }

    AVPacket new_pkt = *pkt;
    AVStream *st = priv->avfc->streams[demux->video->id];
    AVCodecContext *codec = st->codec;
    /* in ffmpeg2.1.0, >=0 means sucess, negative error code in case of failure.
     * in ffmpeg4.2.2 0:try again or eof error, which caused by memory malloc often; 1:success; -1:failure
    */
    int r = av_bitstream_filter_filter(abf, codec, NULL,
            &new_pkt.data, &new_pkt.size, pkt->data, pkt->size, pkt->flags & AV_PKT_FLAG_KEY);
#ifdef CFG_ENABLE_FFMPEG_422
    if (r == 0 && new_pkt.size == 0 && new_pkt.side_data_elems == 0) {
        av_packet_unref(pkt);
        memset(pkt, 0, sizeof(AVPacket));
        MLOGA("\n[%s_%d]Oh Bad case!!!\n",__func__,__LINE__);
        return MT_FAILURE;
    }

    if(r == 0 && new_pkt.data != pkt->data) {
        uint8_t *t = av_malloc(new_pkt.size + AV_INPUT_BUFFER_PADDING_SIZE); //the new should be a subset of the old so cannot overflow
        if (t) {
            memcpy(t, new_pkt.data, new_pkt.size);
            memset(t + new_pkt.size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
            new_pkt.data = t;
            new_pkt.buf = NULL;
            r = 1;
            MLOGA("\n[%s_%d]ttt=0x%p_%p_%p Oh HOOO!!!\n",__func__,__LINE__,t,pkt->data,new_pkt.data);
        } else {
            r = AVERROR(ENOMEM);
            MLOGA("Should we free new pkt for ffmpe4 ???\n", new_pkt.data);
        }
    }

    if (r > 0) {
        new_pkt.buf = av_buffer_create(new_pkt.data, new_pkt.size,
                                       av_buffer_default_free, NULL, 0);
        if (new_pkt.buf) {
            pkt->side_data = NULL;
            pkt->side_data_elems = 0;
            av_packet_unref(pkt);
        } else {
            av_packet_unref(pkt);
            av_freep(&new_pkt.data);
            r = AVERROR(ENOMEM);
            MLOGA("Filter alloc AVBufferRef for new pkt fail\n");
            return MT_ERR_NORES;
        }
    }

    if (r < 0) {
        av_log(codec, AV_LOG_WARNING,
           "[%d]Failed to filter bitstream %d r=%d\n", pkt->stream_index, bsf_vcodec_flag, r);
       /* av_packet_unref(pkt);
        return MT_ERR_NORES;
        */
        new_pkt = *pkt;
    }
#else
    if(r == 0 && new_pkt.data != pkt->data && new_pkt.destruct) {
        uint8_t *t = av_malloc(new_pkt.size + FF_INPUT_BUFFER_PADDING_SIZE); //the new should be r subset of the old so cannot overflow
        if(t) {
            memcpy(t, new_pkt.data, new_pkt.size);
            memset(t + new_pkt.size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
            new_pkt.data = t;
            new_pkt.buf = NULL;
            r = 1;
        } else {
            r = AVERROR(ENOMEM);
        }
    }
    if (r > 0) {
        av_packet_unref(pkt);
        new_pkt.buf = av_buffer_create(new_pkt.data, new_pkt.size,
                                       av_buffer_default_free, NULL, 0);
        if (!new_pkt.buf) {
            MLOGA("%s %d MALLOC error \n",__func__,__LINE__);
            return MT_ERR_NORES;
        }
    } else if (r < 0) {
        MLOGA("%s %d add filter error r = %d, continue\n",__func__, __LINE__, r);
        new_pkt = *pkt;
    }
#endif

    *pkt = new_pkt;
    return MT_SUCCESS;
}

static int get_packet_from_demux(demuxer_t *demux, AVPacket *out)
{
    int try_cnt;
    AVPacket pkt;
    int ret = MT_SUCCESS;
    AVBitStreamFilterContext *abf = NULL;
    const static int MAX_TRY_CNT  = 30;

    if(bsf_vcodec_flag == 5) {
        abf = (AVBitStreamFilterContext *) h265bsfc;
    } else if(bsf_vcodec_flag == 3) {
        abf = (AVBitStreamFilterContext *) mpeg4bsfc;
    } else if(bsf_vcodec_flag == 1) {
        abf = (AVBitStreamFilterContext *) h264bsfc;
    }

    try_cnt = 0;
    while (try_cnt++ < MAX_TRY_CNT) {
        ret = get_packet_by_av_read(demux, &pkt);
        if (MT_SUCCESS != ret) {
            return ret;
        }

        ret = get_packet_filter(demux, abf, &pkt);
        /* MT_SUCCESS != ret in case of filter failure */
        if (MT_SUCCESS == ret) {
            break;
        }
        /* unref pkt in get_packet_filter when failure */
    }

    if (MT_SUCCESS != ret) {
        return ret;
    }

    av_packet_move_ref(out, &pkt);
    return MT_SUCCESS;
}

static void set_demuxer_video_info(sh_video_t *sh, AVStream *st)
{
    unsigned int need_set_info = 0;
    if (NULL == sh || NULL == st) {
        return;
    }

    AVRational frame_rate = st->avg_frame_rate;
    need_set_info = ((sh->fps.num != frame_rate.num ||
        sh->fps.den != frame_rate.den) && (0 != frame_rate.num) && (0 != frame_rate.den));
    /* Not support frame rate less than 10 or larger than 60 */
    if (need_set_info && av_q2d(frame_rate) <= 60 && av_q2d(frame_rate) >= 10) {
        sh->fps.den = frame_rate.den;
        sh->fps.num = frame_rate.num;
    }
}

static int lavf_fill_buffer(demuxer_t *demux, demux_stream_t *dsds, AVPacket *packet)
{
    int id;
    int ret = MT_TRUE;
    lavf_priv_t *priv = demux->priv;
    demux_packet_t *dp;
    demux_stream_t *ds;

    dp = new_demux_packet(0);
    if(!dp) {
        MLOGE("Alloct dp buffer fail\n");
        return MT_FALSE;
    }

    dp_ff_av_pkt_alloc(dp);
    if (!dp->ff_av_pkt) {
        MLOGE("Alloct av pkt fail\n");
        ret = MT_FALSE;
        goto fail;
    }

    AVPacket *pkt = dp->ff_av_pkt;
    if (MT_SUCCESS != get_packet_from_demux(demux, pkt)) {
        MLOGE("demux lavf fill buffer get packet error\n");
        ret = MT_FALSE;
        goto fail;
    }

    // handle any new streams that might have been added
    for (id = priv->nb_streams_last; id < priv->avfc->nb_streams; id++) {
        handle_stream(demux, priv->avfc, id);
    }

    priv->nb_streams_last = priv->avfc->nb_streams;
#if defined(VMX_OTT_SVP) || !defined(CFG_ENABLE_FFMPEG_422)
    if (pkt->stream_index == demux->video->id &&
        (priv->avfc->streams[demux->video->id]->codec->codec_id == CODEC_ID_WMV3 ||
         priv->avfc->streams[demux->video->id]->codec->codec_id == CODEC_ID_VC1)) {
        float video_fps = av_q2d(priv->avfc->streams[demux->video->id]->avg_frame_rate);
        int grow_size = 0;
        int old_size  = pkt->size;
        int vc1_bits_pos = 0;
        unsigned char vc1_bits_buf[1024] = {0};

        if ((pkt->flags & AV_PKT_FLAG_KEY) &&
            priv->avfc->streams[demux->video->id]->codec->extradata) {
            vc1_add_sequence_header(
                priv->avfc->streams[demux->video->id]->codec, video_fps,
                priv->avfc->streams[demux->video->id], pkt, vc1_bits_buf, &vc1_bits_pos);
        }

        add_frame_header(priv->avfc->streams[demux->video->id]->codec,
                         pkt, priv->avfc->streams[demux->video->id], vc1_bits_buf, &vc1_bits_pos);

        if ((vc1_bits_pos % 8) != 0) {
            lavf_put_bits(vc1_bits_buf, &vc1_bits_pos, 0, 8 - (vc1_bits_pos % 8));
        }

        grow_size = ROUNDUP(vc1_bits_pos, 8) / 8 ;
        av_grow_packet(pkt, grow_size);
        if (grow_size != 0) {
            void *tmp_pkt = (void *)malloc33(old_size);
            if (NULL != tmp_pkt) {
                memcpy(tmp_pkt, pkt->data, old_size);
                memcpy(pkt->data + grow_size, tmp_pkt, old_size);
                memcpy(pkt->data, vc1_bits_buf, grow_size);
                free33(tmp_pkt);
            }
        }
    }
#endif
    id = pkt->stream_index;
    if (id == demux->audio->id) {
        ds = demux->audio;
        if (!ds->sh) {
            ds->sh = demux->a_streams[id];
            mp_msg(MSGT_DEMUX, MSGL_V, "Auto-selected LAVF audio ID = %d\n", ds->id);
        }
    } else if (id == demux->video->id) {
        ds = demux->video;
        if (!ds->sh) {
            ds->sh = demux->v_streams[id];
            mp_msg(MSGT_DEMUX, MSGL_V, "Auto-selected LAVF video ID = %d\n", ds->id);
        }

        set_demuxer_video_info(demux->v_streams[id], priv->avfc->streams[id]);
    } else if (id == demux->sub->id) {
        ds = demux->sub;
        sub_utf8 = 1;
    } else if (!demux->audio->eof || !demux->video->eof || !demux->sub->eof) {
        goto fail;
    } else {
        goto fail;
    }

    if (0 == pkt->size) {
        goto fail;
    }
    *packet    = *pkt;
    dp->len    = pkt->size;
    dp->buffer = pkt->data;
    dp->is_secure = pkt->is_secure;
    if (pkt->pts != AV_NOPTS_VALUE) {
        dp->pts = pkt->pts * av_q2d(priv->avfc->streams[id]->time_base);
        priv->last_pts = dp->pts * AV_TIME_BASE;
        if (pkt->duration > 0) {
            dp->endpts = dp->pts + pkt->duration * av_q2d(priv->avfc->streams[id]->time_base);
        }
        /* subtitle durations are sometimes stored in convergence_duration */
        if (ds == demux->sub && pkt->convergence_duration > 0) {
            dp->endpts = dp->pts + pkt->convergence_duration * av_q2d(priv->avfc->streams[id]->time_base);
        }
    }
    dp->pos = pkt->pos;
    dp->sample_size = pkt->sample_size;
    dp->flags = !!(pkt->flags & AV_PKT_FLAG_KEY);

    /* append packet to DS stream */
    ds_add_packet(ds, dp);
    return MT_TRUE;
fail:
    free_demux_packet(dp);
    return ret;
}

static int demux_lavf_fill_buffer(demuxer_t *demux, demux_stream_t *ds)
{
    int ret = 0;
    int retry_num = 0;
    AVPacket pkt  = {0};
    lavf_priv_t *lavf = demux->priv;

retry:
    av_init_packet(&pkt);
    pkt.size = 0;
    pkt.data = NULL;
    ret = lavf_fill_buffer(demux, ds, &pkt);
#ifdef CFG_ENABLE_FFMPEG_422
    if (!need_switch_playlist(demux)) {
        return ret;
    }

    int need_change = 0;
    AVInputFormat *avif = lavf->avif;
    /* hls advance playlist when read data from network,
     * so pkt maybe not send in av_read_frame internal->packet_buffer
     */
    MLOGA("[%s]Playlist[%d->%d] [%d]pkt size:%d eof:%d discard:%d ret:%d\n", avif->name,
         lavf->current_plsidx, lavf->expected_plsidx, pkt.stream_index, pkt.size, ds->eof, ds->discard, ret);
    if (strstr(avif->name, "hls")) {
        if (0 == pkt.size) {
            need_change = 1;
        }
    } else if (strstr(avif->name, "dash")) { /* advance seg when av_read_frame  */
        need_change = 1;
    }
    if (need_change) {
        MLOGI("Switch playlist [%d->%d]\n",
            lavf->current_plsidx, lavf->expected_plsidx);
        switch_playlist_l(demux, lavf->avfc,
            lavf->current_plsidx, lavf->expected_plsidx);
        lavf->current_plsidx = lavf->expected_plsidx;
        if (0 == pkt.size) {
            goto retry;
        }
    }
#endif
    if (demux->interrupt_callback &&
        demux->interrupt_callback(demux->callback_ctx)) {
        return ret;
    }

    if (lavf->is_live && MT_FALSE == ret &&
        retry_num++ < MAX_TRY_CNT_FILL_BUFFER) {
        goto retry;
    }
    return ret;
}

static void demux_seek_lavf(demuxer_t *demuxer, float rel_seek_secs, float audio_delay, int flags)
{
    lavf_priv_t *priv = demuxer->priv;
    int avsflags = 0;

    demux_lavf_set_add_head(NULL, 1);
    if (flags & SEEK_ABSOLUTE) {
        priv->last_pts = priv->avfc->start_time != AV_NOPTS_VALUE ? priv->avfc->start_time : 0;
    } else {
        //if (FALSE == is_fp_seek_forward() && rel_seek_secs < 0) avsflags = AVSEEK_FLAG_BACKWARD;
    }

    if (flags & SEEK_FACTOR) {
        if (priv->avfc->duration == 0 || priv->avfc->duration == AV_NOPTS_VALUE) {
            return;
        }
        if(rel_seek_secs) {
            priv->last_pts += rel_seek_secs * priv->avfc->duration;
        } else {
            priv->last_pts = 0;
        }
    } else {
        if(rel_seek_secs) {
            priv->last_pts += rel_seek_secs * AV_TIME_BASE;
        } else {
            priv->last_pts = 0;
        }
    }

    MLOGD("[%p] demux_seek_lavf last:%lld (%f, %f, %d)\n",
        demuxer, priv->last_pts, rel_seek_secs, audio_delay, flags);
    if (av_seek_frame(priv->avfc, -1, priv->last_pts, avsflags) < 0) {
        priv->mark_filepos = stream_tell(demuxer->stream);//jqw@20190516 for bug 109953,109958
        avsflags ^= AVSEEK_FLAG_BACKWARD;
        av_seek_frame(priv->avfc, -1, priv->last_pts, avsflags);
    }
    /* Not change playlist if we are seeking */
    priv->expected_plsidx = -1;
}

static void disable_track(
    demuxer_t *demuxer, lavf_priv_t *lavf, int stream_id)
{
    if (stream_id >= 0 && stream_id < lavf->avfc->nb_streams) {
        lavf->avfc->streams[stream_id]->discard = AVDISCARD_ALL;
        demux_stream_t *ds = NULL;
        if (demuxer->audio && stream_id == demuxer->audio->id) {
            ds = demuxer->audio;
        } else if (demuxer->video && stream_id == demuxer->video->id) {
            ds = demuxer->video;
        } else if (demuxer->sub && stream_id == demuxer->sub->id) {
            ds = demuxer->sub;
        }
        if (ds) {
            ds->discard = 1;
        }
    }
}

static int lavf_control_switch_track(demuxer_t *demuxer, int cmd, void *arg)
{
    int id = *((int *)arg);
    int newid = -2;
    int i, curridx = -1;
    int nb_streams, *pstreams;
    demux_stream_t *ds;
    lavf_priv_t *priv = demuxer->priv;

    if (cmd == DEMUXER_CTRL_SWITCH_VIDEO) {
        ds = demuxer->video;
        nb_streams = priv->video_streams;
        pstreams = priv->vstreams;
    } else {
        ds = demuxer->audio;
        nb_streams = priv->audio_streams;
        pstreams = priv->astreams;
    }
    for (i = 0; i < nb_streams; i++) {
        if (pstreams[i] == ds->id) { //current stream id
            curridx = i;
            break;
        }
    }

    if (id == -2) { // no sound
        i = -1;
    } else if (id == -1) { // next track
        i = (curridx + 2) % (nb_streams + 1) - 1;
        if (i >= 0) {
            newid = pstreams[i];
        }
    } else { // select track by id
        if (id >= 0 && id < nb_streams) {
            i = id;
            newid = pstreams[i];
        }
    }
    if (i == curridx) {
        return DEMUXER_CTRL_NOTIMPL;
    }
    ds_free_packs(ds);
    if (ds->id >= 0) {
        priv->avfc->streams[ds->id]->discard = AVDISCARD_ALL;
    }
    *((int *)arg) = ds->id = newid;
    if (newid >= 0) {
        priv->avfc->streams[newid]->discard = AVDISCARD_NONE;
    }

#ifdef CFG_ENABLE_FFMPEG_422
    priv->avfc->event_flags = 0x100;//tell ffmpeg422 switch audio
#endif
    return DEMUXER_CTRL_OK;
}

static int lavf_control_switch_program(demuxer_t *demuxer, int cmd, void *arg)
{
    demux_program_t *prog = arg;
    AVProgram *program;
    int p, i;
    int start;

    lavf_priv_t *priv = demuxer->priv;
    prog->vid = prog->aid = prog->sid = -2; //no audio and no video by default
    if (priv->avfc->nb_programs < 1) {
        return DEMUXER_CTRL_DONTKNOW;
    }

    if (prog->progid == -1) {
        p = 0;
        while (p < priv->avfc->nb_programs && priv->avfc->programs[p]->id != priv->cur_program) {
            p++;
        }
        p = (p + 1) % priv->avfc->nb_programs;
    } else {
        for (i = 0; i < priv->avfc->nb_programs; i++)
            if (priv->avfc->programs[i]->id == prog->progid) {
                break;
            }
        if (i == priv->avfc->nb_programs) {
            return DEMUXER_CTRL_DONTKNOW;
        }
        p = i;
    }
    start = p;
redo:
    program = priv->avfc->programs[p];
    for (i = 0; i < program->nb_stream_indexes; i++) {
        switch (priv->avfc->streams[program->stream_index[i]]->codec->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            if (prog->vid == -2) {
                prog->vid = program->stream_index[i];
            }
            break;
        case AVMEDIA_TYPE_AUDIO:
            if (prog->aid == -2) {
                prog->aid = program->stream_index[i];
            }
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            if (prog->sid == -2) {
                prog->sid = program->stream_index[i];
            }
            break;
        }
    }
    if (prog->aid >= 0 && prog->aid < MAX_A_STREAMS &&
            demuxer->a_streams[prog->aid]) {
        sh_audio_t *sh = demuxer->a_streams[prog->aid];
        prog->aid = sh->aid;
    } else {
        prog->aid = -2;
    }
    if (prog->vid >= 0 && prog->vid < MAX_V_STREAMS &&
            demuxer->v_streams[prog->vid]) {
        sh_video_t *sh = demuxer->v_streams[prog->vid];
        prog->vid = sh->vid;
    } else {
        prog->vid = -2;
    }
    if (prog->progid == -1 && prog->vid == -2 && prog->aid == -2) {
        p = (p + 1) % priv->avfc->nb_programs;
        if (p == start) {
            return DEMUXER_CTRL_DONTKNOW;
        }
        goto redo;
    }
    priv->cur_program = prog->progid = program->id;
    return DEMUXER_CTRL_OK;
}

static int demux_lavf_control(demuxer_t *demuxer, int cmd, void *arg)
{
    lavf_priv_t *priv = demuxer->priv;

    switch (cmd) {
    case DEMUXER_CTRL_CORRECT_PTS:
        return DEMUXER_CTRL_OK;
    case DEMUXER_CTRL_GET_TIME_LENGTH:
        if (priv->avfc->duration == 0 || priv->avfc->duration == AV_NOPTS_VALUE) {
            *((double *)arg) = (double) 0;
        } else {
            *((double *)arg) = (double) priv->avfc->duration / AV_TIME_BASE;
        }
        return DEMUXER_CTRL_OK;

    case DEMUXER_CTRL_GET_PERCENT_POS:
        if (priv->avfc->duration == 0 || priv->avfc->duration == AV_NOPTS_VALUE) {
            return DEMUXER_CTRL_DONTKNOW;
        }

        *((int *)arg) = (int)((priv->last_pts - priv->avfc->start_time) * 100 / priv->avfc->duration);
        return DEMUXER_CTRL_OK;
    case DEMUXER_CTRL_SWITCH_AUDIO:
    case DEMUXER_CTRL_SWITCH_VIDEO: {
        return lavf_control_switch_track(demuxer, cmd, arg);
    }
    case DEMUXER_CTRL_IDENTIFY_PROGRAM: {
        return lavf_control_switch_track(demuxer, cmd, arg);
    }
    //jqw@20190516 for bug 109953,109958
    case DEMUXER_CTRL_GET_SEEK_MARKPOS: {
        *((off_t *)arg) = priv->mark_filepos;
        return DEMUXER_CTRL_OK;
    }
    case DEMUXER_CTRL_DISABLE_TRACK: {
        disable_track(demuxer, priv, *((int *)arg));
        return DEMUXER_CTRL_OK;
    }
    case DEMUXER_CTRL_SWITCH_PLAYLIST: {
        int ret = DEMUXER_CTRL_NOTIMPL;
#ifdef CFG_ENABLE_FFMPEG_422
        switch_playlist_mode_set(priv, AV_ADAPTIVE_RESOLUTION_MODE_MANUAL);
        ret = switch_expected_playlist(demuxer, priv, *((int *)arg));
#endif
        return ret;
    }
    case DEMUXER_CTRL_GET_DL_BITRATE: {
        int64_t qos_net_bitrate = 0;
        if (av_opt_get_int(priv->avfc, "qos_net_bitrate",
            AV_OPT_SEARCH_CHILDREN, &qos_net_bitrate) >= 0) {
            av_opt_set_int(priv->avfc, "qos_net_bitrate", qos_net_bitrate >> 10, AV_OPT_SEARCH_CHILDREN);
            if (arg) {
                *(int64_t *) arg = qos_net_bitrate;
                return DEMUXER_CTRL_OK;
            }
        }
        return DEMUXER_CTRL_DONTKNOW;
    }
    case DEMUXER_CTRL_GET_PLAYLIST: {
        int ret = DEMUXER_CTRL_NOTIMPL;
#ifdef CFG_ENABLE_FFMPEG_422
        ret = get_adaptive_playlist(demuxer, priv);
#endif
        return ret;
    }
    default:
        return DEMUXER_CTRL_NOTIMPL;
    }
}

static void demux_close_lavf(demuxer_t *demuxer)
{
    lavf_priv_t *priv = demuxer->priv;
    if (priv) {
        if (priv->avfc) {
            av_freep(&priv->avfc->key);
            avformat_close_input(&priv->avfc);
        }
        if (priv->pb) {
            if (priv->pb->buffer) {
                free33(priv->pb->buffer);   //linda zhu modify it, priv->buffer may be freed in ffio_set_buf_size
                priv->pb->buffer = NULL;
                priv->buffer = NULL;
            }
        }
        av_freep(&priv->pb);
        if (priv->buffer) {
            free33(priv->buffer);
        }
        free33(priv);
        demuxer->priv = NULL;
    }
    p_lavf_avfc = NULL;
    if (h264bsfc) {
        av_bitstream_filter_close((AVBitStreamFilterContext *)h264bsfc);
        h264bsfc = NULL;
    }
    if (h265bsfc) {
        av_bitstream_filter_close((AVBitStreamFilterContext *)h265bsfc);
        h265bsfc = NULL;
    }
    if (mpeg4bsfc) {
        av_bitstream_filter_close((AVBitStreamFilterContext *)mpeg4bsfc);
        mpeg4bsfc = NULL;
    }
}

const demuxer_desc_t demuxer_desc_lavf = {
    "libavformat demuxer",
    "lavf",
    "libavformat",
    "Michael Niedermayer",
    "supports many formats, requires libavformat",
    DEMUXER_TYPE_LAVF,
    0, // Check after other demuxer
    lavf_check_file,
    demux_lavf_fill_buffer,
    demux_open_lavf,
    demux_close_lavf,
    demux_seek_lavf,
    demux_lavf_control
};
