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

#ifndef MPLAYER_DEMUX_TS_H
#define MPLAYER_DEMUX_TS_H

#include <sys/types.h>

#define TS_PH_PACKET_SIZE  192
#define TS_FEC_PACKET_SIZE 204
#define TS_PACKET_SIZE     188
#define NB_PID_MAX         8192
#define TS_MAX_PROBE_SIZE  8000000

typedef struct {
    uint8_t *buffer;
    uint16_t buffer_len;
} ts_section_t;

typedef struct {
    int size;
    unsigned char *start;
    uint16_t payload_size;
    es_stream_type_t type, subtype;
    double pts, last_pts;
    long long dts;
    unsigned int pts31bit;
    int pid;
    char lang[4];
    int last_cc;                // last cc code (-1 if first packet)
    int is_synced;
    ts_section_t section;
    uint8_t *extradata;
    int extradata_alloc, extradata_len;
    struct {
        uint8_t au_start, au_end, last_au_end;
    } sl;
    int scrambled_cnt;
    int not_scrambled_cnt;
} ES_stream_t;

typedef struct {
    void *sh;
    int id;
    int type;
} sh_av_t;

typedef struct MpegTSContext {
    int packet_size;        // raw packet size, including FEC if present e.g. 188 bytes
    ES_stream_t *pids[NB_PID_MAX];
    sh_av_t streams[NB_PID_MAX];
} MpegTSContext;

#define MAX_EXTRADATA_SIZE 64*1024
typedef struct {
    int32_t object_type;    //aka codec used
    int32_t stream_type;    //video, audio etc.
    uint8_t buf[MAX_EXTRADATA_SIZE];
    uint16_t buf_size;
    uint8_t szm1;
} mp4_decoder_config_t;

typedef struct {
    //flags
    uint8_t flags;
    uint8_t au_start;
    uint8_t au_end;
    uint8_t random_accesspoint;
    uint8_t random_accesspoint_only;
    uint8_t padding;
    uint8_t use_ts;
    uint8_t idle;
    uint8_t duration;

    uint32_t ts_resolution, ocr_resolution;
    uint8_t ts_len, ocr_len, au_len, instant_bitrate_len, degr_len, au_seqnum_len, packet_seqnum_len;
    uint32_t timescale;
    uint16_t au_duration, cts_duration;
    uint64_t ocr, dts, cts;
} mp4_sl_config_t;

typedef struct {
    uint16_t id;
    uint8_t flags;
    mp4_decoder_config_t decoder;
    mp4_sl_config_t sl;
} mp4_es_descr_t;

typedef struct {
    uint16_t id;
    uint8_t flags;
    mp4_es_descr_t *es;
    uint16_t es_cnt;
} mp4_od_t;

typedef struct {
    int audio_is_pcm;
    int bits_per_coded_sample;
    int sample_fmt;
    int bits_per_raw_sample;
    int sample_rate;
    uint64_t channel_layout;
    int channels;
    int bit_rate;
    int is_big_endian;
} pcm_info_t;

typedef struct {
    uint8_t skip;
    uint8_t table_id;
    uint8_t ssi;
    uint16_t section_length;
    uint16_t ts_id;
    uint8_t version_number;
    uint8_t curr_next;
    uint8_t section_number;
    uint8_t last_section_number;
    struct pat_progs_t {
        uint16_t id;
        uint16_t pmt_pid;
    } *progs;
    uint16_t progs_cnt;
    ts_section_t section;
} pat_t;

typedef struct {
    uint16_t progid;
    uint8_t skip;
    uint8_t table_id;
    uint8_t ssi;
    uint16_t section_length;
    uint8_t version_number;
    uint8_t curr_next;
    uint8_t section_number;
    uint8_t last_section_number;
    uint16_t PCR_PID;
    uint16_t prog_descr_length;
    ts_section_t section;
    uint16_t es_cnt;
    pcm_info_t pcm_info;
    struct pmt_es_t {
        uint16_t pid;
        uint32_t type;  //it's 8 bit long, but cast to the right type as FOURCC
        uint16_t descr_length;
        uint8_t format_descriptor[5];
        uint8_t lang[4];
        uint16_t mp4_es_id;
        uint16_t audio_eac3_flag;
    } *es;
    mp4_od_t iod, *od;
    mp4_es_descr_t *mp4es;
    int od_cnt, mp4es_cnt;
} pmt_t;

typedef struct {
    uint64_t size;
    float duration;
    double first_pts;
    double last_pts;
} TS_stream_info;

typedef struct {
    MpegTSContext ts;
    int last_pid;
    av_fifo_t fifo[3];  //0 for audio, 1 for video, 2 for subs
    pat_t pat;
    pmt_t *pmt;
    uint16_t pmt_cnt;
    uint32_t prog;
    uint32_t vbitrate;
    int keep_broken;
    int last_aid;
    int last_vid;
    int last_sid;
    int selected_program_idx;
    unsigned short selected_apid;
    unsigned short selected_vpid;
    char packet[TS_FEC_PACKET_SIZE];
    TS_stream_info vstr, astr;
    off_t seek_stop_filepos;
    off_t mark_filepos;
    int seek_stop_flag;
    double file_last_pts;
    double file_first_pts;
    off_t seek_start_filepos;
    parser_ctx_handle_t parser_handle;
} ts_priv_t;

typedef struct {
    es_stream_type_t type;
    ts_section_t section;
} TS_pids_t;

#if MT_DES("EXTERNAL PIERCE API", 1)
void ts_get_pcm_info(demuxer_t *demuxer,
    int *is_big_endian, int *bits, int *channel, int *sample_rate);
void set_ts_prog(demuxer_t *demuxer);
#endif

#endif /* MPLAYER_DEMUX_TS_H */
