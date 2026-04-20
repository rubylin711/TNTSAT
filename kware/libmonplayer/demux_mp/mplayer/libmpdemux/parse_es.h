/*
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

#ifndef MPLAYER_PARSE_ES_H
#define MPLAYER_PARSE_ES_H

#include <stdint.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "demuxer.h"
#include "aviheader.h"
#include "stheader.h"

#define MPEG2_VIDEO_SEQ_START_CODE      0x1B3
#define MPEG2_VIDEO_GOP_START_CODE      0x1B8

#define MPEG2_PACK_START_CODE           0x1BA
#define MPEG2_SYSTEM_HEADER_START_CODE  0x1BB
#define MPEG2_PRIVATE_START_CODE        0x1BD

#define MPEG2_AUDIO_PES_START_ID_HEAD   0x1C0
#define MPEG2_AUDIO_PES_END_ID_HEAD     0x1DF
#define MPEG2_VIDEO_PES_START_ID_HEAD   0x1E0
#define MPEG2_VIDEO_PES_END_ID_HEAD     0x1EF
#define MPEG2_PICTURE_START_CODE        0x00000100
#define MPEG2_NO_START_CODE_VAL         (-1)
#define MPEG4_VOP_STARTCODE             0x1B6

#define MAX_VIDEO_PACKET_SIZE (224*1024+4)
#define VIDEOBUFFER_SIZE 0x100000

#define DEFAULT_FRAME_HEADER_BYTES        4
#define VC1_SEQUENCE_HEADER          (0x0000010F)
#define HEVC_NAL_TYPE(type)          (((type) & 0x7e) >> 1)
#define IS_HEVC_IRAP_NAL(type)       (HEVC_NAL_TYPE(type) >= 16 && HEVC_NAL_TYPE(type) <= 23)

enum {
    ES_TYPE_IS_AUD = 0,
    ES_TYPE_IS_VID,
    ES_TYPE_IS_SUB,
    ES_TYPE_IS_MAX,
};

typedef enum {
    UNKNOWN_MP           = 0,
    VIDEO_MPEG1_MP       = 0x10000001,
    VIDEO_MPEG2_MP       = 0x10000002,
    VIDEO_MPEG4_MP       = 0x10000004,
    VIDEO_H264_MP        = 0x10000005,
    VIDEO_AVS_MP         = 0x10000042,
    VIDEO_AVS2_MP        = MKTAG('A', 'V', 'S', '2'),
    VIDEO_AVC_MP         = MKTAG('a', 'v', 'c', '1'),
    VIDEO_HEVC_MP        = MKTAG('H', 'E', 'V', 'C'),
    VIDEO_DIRAC_MP       = MKTAG('d', 'r', 'a', 'c'),
    VIDEO_VC1_MP         = MKTAG('W', 'V', 'C', '1'),
    AUDIO_MP2_MP         = 0x50,
    AUDIO_A52_MP         = 0x2000, /* AC3 and EAC3 */
    AUDIO_DTS_MP         = 0x2001,
    AUDIO_AC4_MP         = 0x2002,
    AUDIO_LPCM_BE_MP     = 0x10001,
    AUDIO_AV3A_MP        = MKTAG('A', 'V', '3', 'A'),
    AUDIO_AAC_MP         = MKTAG('M', 'P', '4', 'A'),
    AUDIO_AAC_LATM_MP    = MKTAG('M', 'P', '4', 'L'),
    AUDIO_TRUEHD_MP      = MKTAG('T', 'R', 'H', 'D'),
    AUDIO_TRUEHD_AC3_MP  = MKTAG('T', 'D', 'A', '3'), /* Dolby TrueHD with embed ac3, Not support Dolby TrueHD, so play ebed ac3 only */
    AUDIO_S302M_MP       = MKTAG('B', 'S', 'S', 'D'),
    AUDIO_PCM_BR_MP      = MKTAG('B', 'P', 'C', 'M'),
    SPU_DVD_MP           = 0x3000000,
    SPU_DVB_MP           = 0x3000001,
    SPU_TELETEXT_MP      = 0x3000002,
    SPU_PGS_MP           = 0x3000003,
    PES_PRIVATE1_MP      = 0xBD00000,
    SL_PES_STREAM_MP     = 0xD000000,
    SL_SECTION_MP        = 0xD100000,
    MP4_OD_MP            = 0xD200000,
} es_stream_type_t;

/**
 * Table 7-3: NAL unit type codes
 */
enum NALUnitType {
    HEVC_NAL_TRAIL_N     =  0,
    HEVC_NAL_TRAIL_R     =  1,
    HEVC_NAL_TSA_N       =  2,
    HEVC_NAL_TSA_R       =  3,
    HEVC_NAL_STSA_N      =  4,
    HEVC_NAL_STSA_R      =  5,
    HEVC_NAL_RADL_N      =  6,
    HEVC_NAL_RADL_R      =  7,
    HEVC_NAL_RASL_N      =  8,
    HEVC_NAL_RASL_R      =  9,
    HEVC_NAL_BLA_W_LP    = 16,
    HEVC_NAL_BLA_W_RADL  = 17,
    HEVC_NAL_BLA_N_LP    = 18,
    HEVC_NAL_IDR_W_RADL  = 19,
    HEVC_NAL_IDR_N_LP    = 20,
    HEVC_NAL_CRA_NUT     = 21,
    HEVC_NAL_VPS         = 32,
    HEVC_NAL_SPS         = 33,
    HEVC_NAL_PPS         = 34,
    HEVC_NAL_AUD         = 35,
    HEVC_NAL_EOS_NUT     = 36,
    HEVC_NAL_EOB_NUT     = 37,
    HEVC_NAL_FD_NUT      = 38,
    HEVC_NAL_SEI_PREFIX  = 39,
    HEVC_NAL_SEI_SUFFIX  = 40,
};

typedef struct parser_ctx {
    int                  codec_id;
    AVStream             *st;
    AVCodecContext       *avctx;
    AVCodecParserContext *pctx;
} parser_ctx_t;

typedef struct parser_ctx_handle {
    /* bit [0]audio [1]video,[2] sub */
    unsigned int parser_opened;
    AVFormatContext *fmt_ctx;
   struct parser_ctx ctx[ES_TYPE_IS_MAX];
} parser_ctx_handle_t;

extern unsigned char* videobuffer;
extern int videobuf_len;
extern unsigned char videobuf_code[4];
extern int videobuf_code_len;

// sync video stream, and returns next packet code
int sync_video_packet(demux_stream_t *ds,int *start_len);

// return: packet length
int read_video_packet(demux_stream_t *ds);

// return: next packet code
int skip_video_packet(demux_stream_t *ds);

int mp_a52_framesize(uint8_t *buf, int *srate);
int find_mpeg12_frame_start(unsigned char *buf,
    int len, int *start_code_offset, int *parsed_data_offset);

int parse_es_open_parser_all(
    demuxer_t *demuxer, parser_ctx_handle_t *handle);

void parse_es_close_parser_all(
    demuxer_t *demuxer, parser_ctx_handle_t *handle);

/* one cannot use dp any more for which maybe freed by tes parser process */
int parse_es_add_packet(
    demuxer_t *demuxer, demux_stream_t *ds,
    parser_ctx_handle_t *handle, demux_packet_t *dp);

int parse_es_reset_parser(demuxer_t *demuxer,
    demux_stream_t *ds, parser_ctx_handle_t * handle);

void parse_es_flush(
    demuxer_t *demuxer, parser_ctx_handle_t *handle);

#endif /* MPLAYER_PARSE_ES_H */
