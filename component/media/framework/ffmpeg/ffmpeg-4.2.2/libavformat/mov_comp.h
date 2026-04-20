/*
 * Copyright (c) 2011 Justin Ruggles
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVFORMAT_MOV_COMP_H
#define AVFORMAT_MOV_COMP_H

#include "avformat.h"
#include "isom.h"
#include "libavutil/encryption_info.h"

typedef struct _MOVOptiAVIndex {
    unsigned int stco_index;
    unsigned int stts_index;
    /* offset in this entry */
    unsigned int stts_sample;
    unsigned int stsc_index;
    /* offset in this entry */
    unsigned int stsc_sample;
    /* base sample number of the entry */
    unsigned int base_sample;
    int64_t timestamp;
    int64_t dts_correction;
    int64_t current_offset;
} MOVOptiAVIndex;

typedef struct _MOVOptiAnchor {
    /* actually allocated entries for keeping information for seek */
    unsigned int nb_entries;
    MOVOptiAVIndex *entries;
    /* Current used entry index */
    unsigned int entry_index;
    /* Max samples count between two index entries */
    unsigned int max_entry_sample_count;
} MOVOptiAnchor;

typedef struct _MOVOptiStco {
    /* scto or co64 */
    uint32_t type;

    /* total samples of chunks ahead, should be reset after seek */
    int32_t base_sample;
    /* sample offset under handling in current chunk */
    int32_t sample_offset;

    /* Total chunk entry count numbers */
    uint32_t entry_count;
    /* next read chunk entry index */
    uint32_t entry_index;
    /* stco base pisition in files */
    int64_t base_position;
    /* next read chunk position */
   // int64_t next_position;
} MOVOptiStco;

typedef struct _MOVOptiStss {
    /* Total sample entry count numbers */
    uint32_t entry_count;
    /* next read stss entry index */
    uint32_t entry_index;
    /* stss base pisition in files */
    int64_t base_position;
    /* next read stss entry position */
    //int64_t next_position;
} MOVOptiStss;

typedef struct _MOVOptiStsz {
    /* Total samples */
    uint32_t entry_count;
    /* next read stsz entry index */
    uint32_t entry_index;
    /* for compact sample sizes, 4, 8, 16, 32 */
    uint32_t field_size;
    /* stsz base pisition in files */
    int64_t base_position;
    /* next read stsz entry position */
    //int64_t next_position;
} MOVOptiStsz;

/* MOV low memory optium context */
typedef struct _MOVOptiContext {
    /* */
    MOVOptiStco stco;
    MOVOptiStss stss;
    MOVOptiStsz stsz;
    /* Anchor for seek information */
    MOVOptiAnchor anchor;

    int rap_group_present;
    /* 1 means has stss and stss has number start from 1 */
    // unsigned char keyframe_offset;
    /* default 0, set 1 means audio with only one error stts mode */
    unsigned char build_index_mode;
    /* total sample keyframes bitmap */
    unsigned char *keyframe_bitmap;
    int64_t last_sample_dts;

    int64_t (*add_ctts_entry)(MOVStts **, unsigned int *, unsigned int *, int, int);
} MOVOptiContext;

int ff_mov_read_mehd(MOVContext *c, AVIOContext *pb, MOVAtom atom);
int ff_mov_is_network_stream(MOVContext *c);
int ff_mov_more_moof_found(
    MOVContext *c, AVIOContext *pb, MOVAtom *atom);
int ff_mov_append_default_encryption_info(
    MOVContext *c, AVEncryptionInitInfo *info);
int ff_mov_check_pssh_encryption_info(
    MOVContext *c, AVStream *st, AVEncryptionInitInfo *info);
int ff_mov_inject_encryption_info(MOVContext *mov,
    AVStream *st, AVEncryptionInfo *encrypted_sample, AVPacket *pkt);

/* pssh_offset:start position of pssh in fileTest
 * pssh_size:including atom size, pssh tag and later content(set in atom.size)
 */
int ff_mov_parse_pssh_header(MOVContext *c, AVIOContext *pb,
    int64_t pssh_offset, int64_t pssh_size, AVEncryptionInitInfo *info);

/* used in low memory case */
int ff_mov_opti_stream_enable(MOVContext *c, AVStream *st);
int ff_mov_opti_read_stco(MOVContext *c, AVStream *st, AVIOContext *pb, MOVAtom atom);
int ff_mov_opti_read_stss(MOVContext *c, AVStream *st, AVIOContext *pb, MOVAtom atom);
int ff_mov_opti_read_stsz(MOVContext *c, AVStream *st, AVIOContext *pb, MOVAtom atom);

int ff_mov_opti_build_index(MOVContext *c, AVStream *st);
int ff_mov_opti_read_seek(MOVContext *c, AVStream *st, int64_t timestamp, int flags);
int ff_mov_opti_curr_base_sample_id(MOVContext *c, AVStream *st);
AVIndexEntry *ff_mov_opti_get_next_sample(MOVContext *c, AVStream *st);

int ff_mov_opti_stream_create_ctx(MOVContext *c, AVStream *st);
int ff_mov_opti_stream_destroy_ctx(AVStream *st);

#endif /* AVFORMAT_MOV_COMP_H */
