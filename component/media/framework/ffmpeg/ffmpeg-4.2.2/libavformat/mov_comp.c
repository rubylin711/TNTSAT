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

/******************************************************************************
 * Index table use two hierachy
 * 1: Anchor key frames table for seeking
 * 2: Packet read table
   between two frames, build it again when read fail but not eof.
******************************************************************************/
#include "mov_comp.h"
#include "isom.h"
#include "avio.h"
#include "internal.h"
#define MAX_MOV_SYNC_INDEX_ENTRIES             (1024)
#define MAX_MOV_COMPLETE_SAMPLE_NUM            (16 * 1024)
/* some clip with samller size works well, set by caller */
#define MAX_MOV_INDEX_ENTRIES                  (FFMAX(256, c->max_opti_index_entries))
/* sc MOVStreamContext type */
#define MOV_OPTI_GET_CTX(sc)                ((MOVOptiContext *)(((MOVStreamContext *)(sc))->opti_ctx))
#define MOV_OPTI_STCO(sc)                   ((MOVOptiStco *)&(MOV_OPTI_GET_CTX(sc)->stco))
#define MOV_OPTI_STSS(sc)                   ((MOVOptiStss *)&(MOV_OPTI_GET_CTX(sc)->stss))
#define MOV_OPTI_STSZ(sc)                   ((MOVOptiStsz *)&(MOV_OPTI_GET_CTX(sc)->stsz))
#define MOV_OPTI_ANCHOR(sc)                 ((MOVOptiAnchor *)&(MOV_OPTI_GET_CTX(sc)->anchor))
#define MOV_OPTI_ANCHOR_CURR(moc)                                                        \
    ((moc)->anchor.entry_index < ((moc)->anchor.nb_entries))                 ?           \
    ((MOVOptiAVIndex *)(&((moc)->anchor.entries[(moc)->anchor.entry_index]))) : NULL
#define MOV_OPTI_ANCHOR_NEXT(moc)                                                        \
    ((moc)->anchor.entry_index < ((moc)->anchor.nb_entries - 1))                 ?       \
    ((MOVOptiAVIndex *)(&((moc)->anchor.entries[(moc)->anchor.entry_index + 1]))) : NULL

typedef struct _MOVOptiRange {
    int start;
    int end;
} MOVOptiRange;

static int update_opti_index(MOVContext *mov, AVStream *st);

static int isnot_file(char *url)
{
    const char *proto_name = avio_find_protocol_name(url);
    int ret = proto_name &&
        (av_strstart(proto_name, "http", NULL)  ||
         av_strstart(proto_name, "https", NULL) ||
         av_strstart(proto_name, "rtsp", NULL)  ||
         av_strstart(proto_name, "rtmp", NULL)  ||
         av_strstart(proto_name, "rtp", NULL)   ||
         av_strstart(proto_name, "udp", NULL));
    return ret;
}

static int inline stream_has_anchor(MOVStreamContext *sc)
{
    MOVOptiContext   *moc = sc->opti_ctx;
    MOVOptiAnchor *anchor = moc ? MOV_OPTI_ANCHOR(sc) : NULL;
    /* Only opti mem case has achor and it's entry */
    if (!anchor || !anchor->entries || !anchor->nb_entries) {
        return 0;
    }
    return 1;
}

/**
 * Fix st->index_entries, so that it contains only the entries (and the entries
 * which are needed to decode them) that fall in the edit list time ranges.
 * Also fixes the timestamps of the index entries to match the timeline
 * specified the edit lists.
 */
static void mov_fix_index(MOVContext *mov, AVStream *st)
{
}

static inline int mov_stsc_index_valid(unsigned int index, unsigned int count)
{
    return index < count - 1;
}

static void inline init_opti_stream_stco(MOVOptiStco *stco)
{
    stco->entry_count   = 0;
    stco->entry_index   = 0;
    stco->base_sample   = 0;
    stco->sample_offset = 0;
    stco->base_position = -1;
}

static void inline init_opti_stream_stss(MOVOptiStss *stss)
{
    stss->entry_count            = 0;
    stss->entry_index            = 0;
    stss->base_position          = -1;
}

static void inline init_opti_stream_stsz(MOVOptiStsz *stsz)
{
    stsz->field_size    = 0;
    stsz->entry_count   = 0;
    stsz->entry_index   = 0;
    stsz->base_position = -1;
}

static void inline init_opti_stream_anchor(MOVOptiAnchor *anchor)
{
    if (anchor->entries) {
        av_free(anchor->entries);
    }
    anchor->entries                = NULL;
    anchor->nb_entries             = 0;
    anchor->entry_index            = 0;
    anchor->max_entry_sample_count = 0;
}


static void init_opti_stream_ctx(MOVOptiContext *c)
{
    c->keyframe_bitmap   = NULL;
    c->build_index_mode  = 0;
    c->rap_group_present = 0;
    init_opti_stream_stco(&c->stco);
    init_opti_stream_stss(&c->stss);
    init_opti_stream_stsz(&c->stsz);
    init_opti_stream_anchor(&c->anchor);
}

static void destroy_opti_stream_ctx(MOVOptiContext **ctx)
{
    MOVOptiContext *c = NULL;
    if (!ctx || !*ctx) {
        return;
    }

    c = *ctx;
    if (c->keyframe_bitmap) {
        av_freep(&c->keyframe_bitmap);
    }
    if (c->anchor.entries) {
        av_freep(&c->anchor.entries);
    }
    init_opti_stream_ctx(c);
    av_freep(ctx);
}

static int opti_read_rang_invalid(MOVOptiRange *rg, int entry_count)
{
    int ret;
    if (!rg || !entry_count) {
        return 1;
    }

    if (rg->end > entry_count) {
        rg->end = entry_count;
    }
    ret = rg->end <= rg->start || rg->start > entry_count;

    return ret ? 1 : 0;
}

static int opti_read_stco(MOVContext *c, AVStream *st, MOVOptiRange *rg)
{
    unsigned int i, entries;
    AVIOContext *pb = c->fc->pb;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiStco *stco = MOV_OPTI_STCO(sc);
    int64_t next_position = stco->base_position;

    if (opti_read_rang_invalid(rg, stco->entry_count) || -1 == stco->base_position) {
        av_log(c->fc, AV_LOG_ERROR, "opti read STCO range error\n");
        return AVERROR(EINVAL);
    }

    if (stco->type == MKTAG('s','t','c','o')) {
        next_position += (rg->start << 2);
    } else if (stco->type == MKTAG('c','o','6','4')) {
        next_position += (rg->start << 3);
    }

    if (avio_seek(pb, next_position, SEEK_SET) < 0) {
        av_log(c->fc, AV_LOG_ERROR, "opti read STCO seek 0x%lld fail\n", next_position);
        return AVERROR(EINVAL);
    }

    entries = (unsigned int) FFMIN((int) stco->entry_count, (rg->end - rg->start));
    if (sc->chunk_count < entries || !sc->chunk_offsets) {
        sc->chunk_count = 0;
        av_freep(&sc->chunk_offsets);
        sc->chunk_offsets = av_malloc_array(entries, sizeof(*sc->chunk_offsets));
        if (!sc->chunk_offsets) {
            return AVERROR(ENOMEM);
        }
    }

    sc->chunk_count = entries;
    if (stco->type == MKTAG('s','t','c','o')) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->chunk_offsets[i] = avio_rb32(pb);
        }
    } else if (stco->type == MKTAG('c','o','6','4')) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->chunk_offsets[i] = avio_rb64(pb);
        }
    }
    stco->entry_index = rg->end;

    return 0;
}

static int opti_read_stss(MOVContext *c, AVStream *st, MOVOptiRange *rg)
{
    unsigned int i, entries;
    AVIOContext *pb = c->fc->pb;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiStss *stss = MOV_OPTI_STSS(sc);

    if (opti_read_rang_invalid(rg, stss->entry_count) || -1 == stss->base_position) {
        av_log(c->fc, AV_LOG_ERROR, "opti read STSS range error\n");
        return AVERROR(EINVAL);
    }

    int64_t next_position = stss->base_position + rg->start * 4;
    if (avio_seek(pb, next_position, SEEK_SET) < 0) {
        av_log(c->fc, AV_LOG_ERROR, "opti read STSS seek 0x%lld fail\n", next_position);
        return AVERROR(EINVAL);
    }

    entries = (unsigned int) FFMIN((int) stss->entry_count, (rg->end - rg->start));
    if (sc->keyframe_count < entries || !sc->keyframes) {
        sc->keyframe_count = 0;
        av_freep(&sc->keyframes);
        sc->keyframes = av_malloc_array(entries, sizeof(*sc->keyframes));
        if (!sc->keyframes) {
            return AVERROR(ENOMEM);
        }
    }

    sc->keyframe_count = entries;
    for (i = 0; i < entries && !pb->eof_reached; i++) {
        sc->keyframes[i] = avio_rb32(pb);
    }

    stss->entry_index = rg->end;
    return 0;
}

static int opti_read_stsz(MOVContext *c, AVStream *st, MOVOptiRange *rg)
{
    unsigned int i, entries;
    AVIOContext *pb = c->fc->pb;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);

    if (opti_read_rang_invalid(rg, stsz->entry_count) || -1 == stsz->base_position) {
        av_log(c->fc, AV_LOG_ERROR, "opti read STSZ range error\n");
        return AVERROR(EINVAL);
    }
    /* fixed sample size, do nothing */
    if (sc->stsz_sample_size) {
        return 0;
    }

    int64_t next_position = stsz->base_position;
    if (4 == stsz->field_size) {
        next_position += (rg->start >> 1);
    } else if (8 == stsz->field_size) {
        next_position += rg->start;
    } else if (16 == stsz->field_size) {
        next_position += (rg->start << 1);
    } else if (32 == stsz->field_size) {
        next_position += (rg->start << 2);
    }
    if (avio_seek(pb, next_position, SEEK_SET) < 0) {
        av_log(c->fc, AV_LOG_ERROR, "opti read STSS seek 0x%lld fail\n", next_position);
        return AVERROR(EINVAL);
    }

    entries = (unsigned int) FFMIN((int) stsz->entry_count, (rg->end - rg->start));
    if (sc->sample_count < entries || !sc->sample_sizes) {
        sc->sample_count = 0;
        av_freep(&sc->sample_sizes);
        sc->sample_sizes = av_malloc_array(entries, sizeof(*sc->sample_sizes));
        if (!sc->sample_sizes) {
            return AVERROR(ENOMEM);
        }
    }

    sc->sample_count = entries;
    unsigned int sample_size;
    if (4 == stsz->field_size) {
        i = 0;
        /* Odd readed number before */
        if (stsz->entry_index & 1) {
            sample_size = avio_r8(pb);
            sc->sample_sizes[i++] = (sample_size & 0xF);
        }
        while (i < entries && !pb->eof_reached) {
            sample_size = avio_r8(pb);
            /* Big endin */
            sc->sample_sizes[i++] = (sample_size & 0xF0) >> 4;
            if (i < entries - 1) {
                sc->sample_sizes[i++] = (sample_size & 0xF);
            }
        }
    } else if (8 == stsz->field_size) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->sample_sizes[i] = avio_r8(pb);
        }
    } else if (16 == stsz->field_size) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->sample_sizes[i] = avio_rb16(pb);
        }
    } else if (32 == stsz->field_size) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->sample_sizes[i] = avio_rb32(pb);
        }
    }

    stsz->entry_index = rg->end;

    return 0;
}
/* return chunk_offset */
static int64_t read_stco_index(MOVContext *c, AVStream *st, int index)
{
    if (!c || !st || index < 0) {
        return AVERROR(EINVAL);
    }

    MOVStreamContext *sc = st->priv_data;
    if (!sc || !(sc->opti_ctx)) {
        return AVERROR(EINVAL);
    }
    MOVOptiStco *stco = MOV_OPTI_STCO(sc);
    if (!(stco->entry_count)) {
        return AVERROR(EINVAL);
    }

    MOVOptiRange rg = {.start = stco->entry_index - sc->chunk_count, .end = stco->entry_index};
retry:
    if (index >= rg.start && index < rg.end) {
        return sc->chunk_offsets[index - rg.start];
    }

    rg.start = index;
    rg.end   = rg.start + MAX_MOV_INDEX_ENTRIES;

    int ret = opti_read_stco(c, st, &rg);
    if (ret >= 0) {
        goto retry;
    }
    return AVERROR(EINVAL);
}

static int read_stss_index(MOVContext *c, AVStream *st, int index)
{
    if (!c || !st || index < 0) {
        return AVERROR(EINVAL);
    }

    MOVStreamContext *sc = st->priv_data;
    if (!sc || !(sc->opti_ctx)) {
        return AVERROR(EINVAL);
    }
    MOVOptiStss *stss = MOV_OPTI_STSS(sc);
    if (!(stss->entry_count)) {
        return AVERROR(EINVAL);
    }

    MOVOptiRange rg = {.start = stss->entry_index - sc->keyframe_count, .end = stss->entry_index};
retry:
    if (index >= rg.start && index < rg.end) {
        return sc->keyframes[index - rg.start];
    }

    rg.start = index;
    rg.end   = rg.start + MAX_MOV_INDEX_ENTRIES;

    int ret = opti_read_stss(c, st, &rg);
    if (ret >= 0) {
        goto retry;
    }
    return AVERROR(EINVAL);
}

static int read_stsz_index(MOVContext *c, AVStream *st, int index)
{
    if (!c || !st || index < 0) {
        return AVERROR(EINVAL);
    }

    MOVStreamContext *sc = st->priv_data;
    if (!sc || !(sc->opti_ctx)) {
        return AVERROR(EINVAL);
    }
    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);
    if (!(stsz->entry_count)) {
        return AVERROR(EINVAL);
    }

    MOVOptiRange rg = {.start = stsz->entry_index - sc->sample_count, .end = stsz->entry_index};
retry:
    if (index >= rg.start && index < rg.end) {
        return sc->sample_sizes[index - rg.start];
    }

    rg.start = index;
    rg.end   = rg.start + MAX_MOV_INDEX_ENTRIES;

    int ret = opti_read_stsz(c, st, &rg);
    if (ret >= 0) {
        goto retry;
    }
    return AVERROR(EINVAL);
}

static void mov_opti_read_pending_atom(MOVContext *c, AVStream *st)
{
    MOVOptiRange rg = {0};
    MOVStreamContext *sc = st->priv_data;

    rg.end = MOV_OPTI_STCO(sc)->entry_count;
    if (rg.end > 0 && sc->chunk_count < rg.end) {
        (void) opti_read_stco(c, st, &rg);
    }

    rg.end = MOV_OPTI_STSS(sc)->entry_count;
    if (rg.end > 0 && sc->keyframe_count < rg.end) {
        (void) opti_read_stss(c, st, &rg);
    }

    rg.end = MOV_OPTI_STSZ(sc)->entry_count;
    if (rg.end > 0 && sc->sample_count < rg.end) {
        (void) opti_read_stsz(c, st, &rg);
    }
}

static int opti_seek_anchor_sample(
    MOVContext *c, AVStream *st,
    int64_t wanted_timestamp, int flags)
{
    int a, b, m, sample;
    int64_t start_timestamp, end_timestamp;
    int entry_index = 0;
    MOVStreamContext  *sc = st->priv_data;
    MOVOptiContext   *moc = MOV_OPTI_GET_CTX(sc);
    MOVOptiAnchor *anchor = MOV_OPTI_ANCHOR(sc);

    a = -1;
    b = anchor->nb_entries;
    while (b - a > 1) {
        m = (a + b) >> 1;
        start_timestamp = anchor->entries[m].timestamp;
        if (m == anchor->nb_entries - 1) {
            end_timestamp = moc->last_sample_dts + 1;
        } else {
            end_timestamp = anchor->entries[m + 1].timestamp;
        }
        if (wanted_timestamp >= start_timestamp &&
            wanted_timestamp < end_timestamp) {
            entry_index = a = b = m;
            break;
        }

        if (wanted_timestamp <= start_timestamp) {
            b = m;
        }
        if (wanted_timestamp >= end_timestamp) {
            a = m;
        }
    }
    /* wanted_timestamp too samll, but forward ok*/
    if (-1 == a) {
        entry_index = 0;
        wanted_timestamp = anchor->entries[entry_index].timestamp;
    /* wanted_timestamp too big, but backward ok */
    } else if (m == anchor->nb_entries) {
        entry_index = anchor->nb_entries - 1;
    }

    if (entry_index != anchor->entry_index) {
        anchor->entry_index = entry_index;
        (void) update_opti_index(c, st);
    }
    /*
     * < 0 means backward wanted_timestamp too samll or forward wanted_timestamp too big,
     */
    sample = av_index_search_timestamp(st, wanted_timestamp, flags);
    if (sample < 0) {
        if (flags & AVSEEK_FLAG_BACKWARD) {
            sample = 0;
        /* forward end not find out keyframe, the first sample of next anchor is chosen */
        } else if (anchor->entry_index < (anchor->nb_entries - 1)) {
            anchor->entry_index++;
            (void) update_opti_index(c, st);
            sample = 0;
        }
    }
    if (sample < 0) /* not sure what to do */ {
        return AVERROR_INVALIDDATA;
    }

    return sample;
}

static AVIndexEntry *find_next_sample(MOVContext *c, AVStream *st)
{
    AVIndexEntry *sample = NULL;
    MOVStreamContext *sc = st->priv_data;

    unsigned int sample_count =
        stream_has_anchor(sc) ?
            MOV_OPTI_STSZ(sc)->entry_count : st->nb_index_entries;

    if (!sc->pb || sc->current_sample >= sample_count) {
        return NULL;
    }

    if (!stream_has_anchor(sc)) {
        return &st->index_entries[sc->current_sample];
    }

    MOVOptiContext  *moc = sc->opti_ctx;
    MOVOptiAVIndex *next = MOV_OPTI_ANCHOR_NEXT(moc);
    /* next null, sc->current_sample will bigger than stsz->entry_count or error occus */
    if (next && sc->current_sample >= next->base_sample) {
        /* read next entry */
        moc->anchor.entry_index++;
        (void) update_opti_index(c, st);
    }

    MOVOptiAVIndex *curr = MOV_OPTI_ANCHOR_CURR(moc);
    if (!curr) {
        return NULL;
    }

    int index_offset = sc->current_sample - curr->base_sample;
    if (index_offset >= st->nb_index_entries) {
        return NULL;
    }
    return &st->index_entries[index_offset];
}

static void show_anchor_entries(MOVContext *mov, MOVOptiContext *moc)
{
    MOVOptiAnchor *anchor = &moc->anchor;

    av_log(mov->fc, AV_LOG_DEBUG, "Anchor Entry num:%d index:%d max:%d\n",
        anchor->nb_entries, anchor->entry_index, anchor->max_entry_sample_count);
    for (unsigned int i = 0; i < anchor->nb_entries; i++) {
        av_log(mov->fc, AV_LOG_DEBUG,
            "dts:%lld offset:%lld stco_index:%d stts_index:%d stts_sample:%d "
            "stsc_index:%d stsc_sample:%d current_sample:%d\n",
            anchor->entries[i].current_offset,
            anchor->entries[i].timestamp, anchor->entries[i].stco_index,
            anchor->entries[i].stts_index, anchor->entries[i].stts_sample,
            anchor->entries[i].stsc_index, anchor->entries[i].stsc_sample, anchor->entries[i].base_sample);
    }
}

static void update_elst_current_dts(
    MOVContext *mov, AVStream *st, int64_t *current_dts)
{
    MOVStreamContext *sc = st->priv_data;
    if (!sc->elst_count) {
        return;
    }

    int edit_start_index = 0, multiple_edits = 0;
    int64_t empty_duration = 0; // empty duration of the first edit list entry
    int64_t start_time = 0; // start time of the media
    for (unsigned int i = 0; i < sc->elst_count; i++) {
        const MOVElst *e = &sc->elst_data[i];
        if (i == 0 && e->time == -1) {
            /* if empty, the first entry is the start time of the stream
             * relative to the presentation itself */
            empty_duration = e->duration;
            edit_start_index = 1;
        } else if (i == edit_start_index && e->time >= 0) {
            start_time = e->time;
        } else {
            multiple_edits = 1;
        }
    }

    if (multiple_edits && !mov->advanced_editlist)
        av_log(mov->fc, AV_LOG_WARNING, "multiple edit list entries, "
               "Use -advanced_editlist to correctly decode otherwise "
               "a/v desync might occur\n");

    /* adjust first dts according to edit list */
    if ((empty_duration || start_time) && mov->time_scale > 0) {
        if (empty_duration)
            empty_duration = av_rescale(empty_duration, sc->time_scale, mov->time_scale);
        sc->time_offset = start_time - empty_duration;
        sc->min_corrected_pts = start_time;
        if (!mov->advanced_editlist)
            *current_dts = -sc->time_offset;
    }

    if (!multiple_edits && !mov->advanced_editlist &&
        st->codecpar->codec_id == AV_CODEC_ID_AAC && start_time > 0)
        sc->start_pad = (int) start_time;
}

static inline unsigned int update_index_get_keyflag(AVStream *st, unsigned int current_sample)
{
    MOVStreamContext *sc = st->priv_data;
    MOVOptiContext *moc = sc->opti_ctx;
    if (!moc->keyframe_bitmap) {
        return 1;
    }

    unsigned int bits_index  = current_sample & 0x7;
    unsigned int bytes_index = current_sample >> 3;

    return (moc->keyframe_bitmap[bytes_index] & ((unsigned char) (1 << bits_index))) == 0 ? 0 : 1;
}

/* function name comes from ffmpeg commit message
 * 1. for video, subtitle
 * 2. audio with more stts entries, varible decoding Time to Sample
 * 3. audio with one stts entry and entry data(frame duration) valid
 */
static int update_index_use_chunk(MOVContext *mov, AVStream *st)
{
    unsigned int i, j, distance, sample_size, current_sample;
    int64_t last_dts, current_dts, current_offset, dts_correction;
    unsigned int stts_index, stsc_index, stco_index, stts_sample, stsc_sample;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiContext  *moc = sc->opti_ctx;

    int rap_group_present = moc->rap_group_present;
    if (moc->anchor.entry_index >= moc->anchor.nb_entries) {
        return AVERROR(EINVAL);
    }

    MOVOptiAVIndex *entry = MOV_OPTI_ANCHOR_CURR(moc);

    last_dts = current_dts = entry->timestamp;
    stts_index       = entry->stts_index;
    stsc_index       = entry->stsc_index;
    stco_index       = entry->stco_index;
    current_sample   = entry->base_sample;
    dts_correction   = entry->dts_correction;
    stts_sample      = entry->stts_sample;
    stsc_sample      = entry->stsc_sample;
    current_offset   = entry->current_offset;

    if (!st->index_entries) {
        if (av_reallocp_array(&st->index_entries,
			moc->anchor.max_entry_sample_count, sizeof(*st->index_entries)) < 0) {
            st->nb_index_entries = 0;
            return AVERROR(ENOMEM);
        }
        st->index_entries_allocated_size = moc->anchor.max_entry_sample_count * sizeof(*st->index_entries);
    }
    st->nb_index_entries = 0;

    MOVOptiStco *stco = MOV_OPTI_STCO(sc);
    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);
    unsigned int current_update_end_sample = stsz->entry_count;
    if (moc->anchor.entry_index < moc->anchor.nb_entries - 1) {
        current_update_end_sample =
            moc->anchor.entries[moc->anchor.entry_index + 1].base_sample;
    }

    for (i = stco_index; i < stco->entry_count; i++) {
        int64_t next_offset = i+1 < stco->entry_count ? read_stco_index(mov, st, i + 1) : INT64_MAX;
        if (i != stco_index) {
            current_offset = read_stco_index(mov, st, i);
        }
        while (mov_stsc_index_valid(stsc_index, sc->stsc_count) &&
            i + 1 == sc->stsc_data[stsc_index + 1].first) {
            stsc_index++;
        }

        if (next_offset > current_offset && sc->sample_size>0 && sc->sample_size < sc->stsz_sample_size &&
            sc->stsc_data[stsc_index].count * (int64_t)sc->stsz_sample_size > next_offset - current_offset) {
            av_log(mov->fc, AV_LOG_WARNING, "STSZ sample size %d invalid (too large), ignoring\n", sc->stsz_sample_size);
            sc->stsz_sample_size = sc->sample_size;
        }
        if (sc->stsz_sample_size>0 && sc->stsz_sample_size < sc->sample_size) {
            av_log(mov->fc, AV_LOG_WARNING, "STSZ sample size %d invalid (too small), ignoring\n", sc->stsz_sample_size);
            sc->stsz_sample_size = sc->sample_size;
        }

        for (j = (i == stco_index) ? stsc_sample : 0; j < sc->stsc_data[stsc_index].count; j++) {
            if (current_sample >= current_update_end_sample) {
                return 0;
            }
            if (current_sample >= stsz->entry_count) {
                av_log(mov->fc, AV_LOG_ERROR, "wrong sample count\n");
                return AVERROR(EINVAL);
            }

            int keyframe = update_index_get_keyflag(st, current_sample);
            if (keyframe) {
                distance = 0;
            }

            sample_size = sc->stsz_sample_size > 0 ? sc->stsz_sample_size : read_stsz_index(mov, st, current_sample);
            if (sc->pseudo_stream_id == -1 ||
               sc->stsc_data[stsc_index].id - 1 == sc->pseudo_stream_id) {
                if (sample_size > 0x3FFFFFFF) {
                    av_log(mov->fc, AV_LOG_ERROR, "Sample size %u is too large\n", sample_size);
                    return AVERROR(EINVAL);
                }

                AVIndexEntry *e = &st->index_entries[st->nb_index_entries++];
                e->pos = current_offset;
                e->timestamp = current_dts;
                e->size = sample_size;
                e->min_distance = distance;
                e->flags = keyframe ? AVINDEX_KEYFRAME : 0;

                av_log(mov->fc, AV_LOG_TRACE, "AVIndex stream %d, sample %u, offset %"PRIx64", dts %"PRId64", "
                        "size %u, distance %u, keyframe %d\n", st->index, current_sample,
                        current_offset, current_dts, sample_size, distance, keyframe);
            }

            current_offset += sample_size;
            /* A negative sample duration is invalid based on the spec,
             * but some samples need it to correct the DTS. */
            if (sc->stts_data[stts_index].duration < 0) {
                av_log(mov->fc, AV_LOG_WARNING,
                       "Invalid SampleDelta %d in STTS, at %d st:%d\n",
                       sc->stts_data[stts_index].duration, stts_index,
                       st->index);
                dts_correction += sc->stts_data[stts_index].duration - 1;
                sc->stts_data[stts_index].duration = 1;
            }
            current_dts += sc->stts_data[stts_index].duration;
            if (!dts_correction || current_dts + dts_correction > last_dts) {
                current_dts += dts_correction;
                dts_correction = 0;
            } else {
                /* Avoid creating non-monotonous DTS */
                dts_correction += current_dts - last_dts - 1;
                current_dts = last_dts + 1;
            }

            last_dts = current_dts;
            distance++;
            stts_sample++;
            current_sample++;
            if (stts_index + 1 < sc->stts_count && stts_sample == sc->stts_data[stts_index].count) {
                stts_sample = 0;
                stts_index++;
            }
        }
    }
    return 0;
}

/* function name comes from ffmpeg commit message
 * for audio
*/
static int update_index_split_chunk(MOVContext *mov, AVStream *st)
{
    unsigned int i, total;
    int64_t current_dts, current_offset;
    unsigned int stsc_index, stco_index;
    unsigned int chunk_samples, current_sample;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiContext  *moc = sc->opti_ctx;

    if (!(MOV_OPTI_STCO(sc)->entry_count)) {
        return AVERROR(EINVAL);
    }

    if (moc->anchor.entry_index >= moc->anchor.nb_entries) {
        return AVERROR(EINVAL);
    }

    total = MOV_OPTI_STSZ(sc)->entry_count;
    if (!st->index_entries) {
        if (av_reallocp_array(&st->index_entries,
			moc->anchor.max_entry_sample_count, sizeof(*st->index_entries)) < 0) {
            st->nb_index_entries = 0;
            return AVERROR(ENOMEM);
        }
        st->index_entries_allocated_size = moc->anchor.max_entry_sample_count * sizeof(*st->index_entries);
    }
    st->nb_index_entries = 0;

    MOVOptiAVIndex *entry = MOV_OPTI_ANCHOR_CURR(moc);
    stco_index     = entry->stco_index;
    stsc_index     = entry->stsc_index;
    chunk_samples  = entry->stsc_sample;
    current_sample = entry->base_sample;
    current_offset = entry->current_offset;
    current_dts    = entry->timestamp;

    MOVOptiStco *stco = MOV_OPTI_STCO(sc);
    unsigned int current_update_end_sample = total;
    if (moc->anchor.entry_index < moc->anchor.nb_entries - 1) {
        current_update_end_sample =
            moc->anchor.entries[moc->anchor.entry_index + 1].base_sample;
    }

    for (i = stco_index; i < stco->entry_count; i++) {
        if (i != stco_index) {
            current_offset = read_stco_index(mov, st, i);
        }

        if (mov_stsc_index_valid(stsc_index, sc->stsc_count) &&
            i + 1 == sc->stsc_data[stsc_index + 1].first)
            stsc_index++;

        if (i != stco_index) {
            chunk_samples = sc->stsc_data[stsc_index].count;
        }

        while (chunk_samples > 0) {
            AVIndexEntry *e;
            unsigned size, samples;

            if (current_sample >= current_update_end_sample) {
                return 0;
            }
            if (sc->samples_per_frame > 1 && !sc->bytes_per_frame) {
                avpriv_request_sample(mov->fc,
                       "Zero bytes per frame, but %d samples per frame",
                       sc->samples_per_frame);
                return AVERROR(EINVAL);
            }

            if (sc->samples_per_frame >= 160) { // gsm
                samples = sc->samples_per_frame;
                size = sc->bytes_per_frame;
            } else {
                if (sc->samples_per_frame > 1) {
                    samples = FFMIN((1024 / sc->samples_per_frame)*
                                    sc->samples_per_frame, chunk_samples);
                    size = (samples / sc->samples_per_frame) * sc->bytes_per_frame;
                } else {
                    samples = FFMIN(1024, chunk_samples);
                    size = samples * sc->sample_size;
                }
            }

            if (st->nb_index_entries >= total) {
                av_log(mov->fc, AV_LOG_ERROR, "wrong chunk count %u\n", total);
                return AVERROR(EINVAL);
            }
            if (size > 0x3FFFFFFF) {
                av_log(mov->fc, AV_LOG_ERROR, "Sample size %u is too large\n", size);
                return AVERROR(EINVAL);
            }
            e = &st->index_entries[st->nb_index_entries++];
            e->pos = current_offset;
            e->timestamp = current_dts;
            e->size = size;
            e->min_distance = 0;
            e->flags = AVINDEX_KEYFRAME;
            av_log(mov->fc, AV_LOG_TRACE, "AVIndex stream %d, chunk %u, offset %"PRIx64", dts %"PRId64", "
                   "size %u, duration %u\n", st->index, i, current_offset, current_dts, size, samples);

            current_offset += size;
            current_dts += samples;
            chunk_samples -= samples;
            current_sample++;
        }
    }
    return 0;
}

/* after update, caller should decide update moc->anchor.entry_index or not  */
static int update_opti_index(MOVContext *mov, AVStream *st)
{
    int ret = 0;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiContext  *moc = sc->opti_ctx;

    st->nb_index_entries = 0;
    /* only use old uncompressed audio chunk demuxing when stts specifies it */
    if (!(moc->build_index_mode)) {
        ret = update_index_use_chunk(mov, st);
    } else {
        ret = update_index_split_chunk(mov, st);
    }
    if (ret < 0) {
        return ret;
    }

    mov->ignore_editlist = 1;
    if (!mov->ignore_editlist && mov->advanced_editlist) {
        // Fix index according to edit lists.
        mov_fix_index(mov, st);
    }

    return 0;
}

static void build_index_set_bit(AVStream *st, unsigned int current_sample)
{
    MOVStreamContext *sc = st->priv_data;
    MOVOptiContext *moc = sc->opti_ctx;

    if (!moc->keyframe_bitmap) {
        return;
    }

    unsigned int bits_index  = current_sample & 0x7;
    unsigned int bytes_index = current_sample >> 3;
    moc->keyframe_bitmap[bytes_index] |= (unsigned char) (1 << bits_index);
}

static void build_index_add_entry(AVStream *st, int flags,
    MOVOptiAVIndex *entry, unsigned int current_sample, unsigned int first_sample)
{
    MOVStreamContext *sc  = st->priv_data;
    MOVOptiAnchor *anchor = MOV_OPTI_ANCHOR(sc);
    /* some clip not set first sample keyframe but cannot be dropped */
    if (!first_sample && AVINDEX_KEYFRAME != (flags & AVINDEX_KEYFRAME)) {
        return;
    }

    build_index_set_bit(st, current_sample);
    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);
    if (stsz->entry_count > 0 && anchor->nb_entries > 0) {
        unsigned int map_idx = (unsigned int ) av_rescale(
            anchor->entry_index,  stsz->entry_count, anchor->nb_entries);
        if (map_idx <= current_sample) {
            if (anchor->entry_index > 0) {
                anchor->max_entry_sample_count = FFMAX(anchor->max_entry_sample_count,
                    (current_sample - anchor->entries[anchor->entry_index - 1].base_sample));
            }
            anchor->entries[anchor->entry_index++] = *entry;
        }
    }
}

static int build_index_update_ctss_data(MOVContext *mov, AVStream *st)
{
    unsigned int i, j;
    MOVStreamContext *sc = st->priv_data;
    MOVStts *ctts_data_old = sc->ctts_data;
    unsigned int ctts_count_old = sc->ctts_count;

    if (!ctts_data_old) {
        return 0;
    }

    unsigned int sample_count =
        stream_has_anchor(sc) ?
            MOV_OPTI_STSZ(sc)->entry_count : st->nb_index_entries;

    // Expand ctts entries such that we have a 1-1 mapping with samples
    if (sample_count >= UINT_MAX / sizeof(*sc->ctts_data)) {
        return AVERROR(EINVAL);
    }
    sc->ctts_count = 0;
    sc->ctts_allocated_size = 0;
    sc->ctts_data = av_fast_realloc(NULL, &sc->ctts_allocated_size,
                            sample_count * sizeof(*sc->ctts_data));
    if (!sc->ctts_data) {
        av_free(ctts_data_old);
        return AVERROR(ENOMEM);
    }

    memset((uint8_t*)(sc->ctts_data), 0, sc->ctts_allocated_size);
    MOVOptiContext  *moc = sc->opti_ctx;
    for (i = 0; i < ctts_count_old &&
                sc->ctts_count < sample_count; i++) {
        for (j = 0; j < ctts_data_old[i].count &&
                    sc->ctts_count < sample_count; j++) {
            moc->add_ctts_entry(&sc->ctts_data, &sc->ctts_count,
                           &sc->ctts_allocated_size, 1,
                           ctts_data_old[i].duration);
        }
    }
    av_free(ctts_data_old);
    return 0;
}

/* function name comes from ffmpeg commit message */
static int build_keyframe_entry(MOVContext *c, AVStream *st)
{
    AVIOContext *pb = c->fc->pb;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiStss *stss = MOV_OPTI_STSS(sc);

    /* sample count must bigger than MAX_MOV_SYNC_INDEX_ENTRIES */
    unsigned int keyframe_num = MAX_MOV_SYNC_INDEX_ENTRIES;
    /* known keyframe smaller than default */
    if (stss->entry_count > 0 && stss->entry_count < keyframe_num) {
        keyframe_num = stss->entry_count;
    }

    MOVOptiAnchor *anchor = MOV_OPTI_ANCHOR(sc);
    anchor->entries = av_malloc_array(keyframe_num, sizeof(*anchor->entries));
    if (!anchor->entries) {
        av_log(c->fc, AV_LOG_ERROR, "No memory for key index\n");
        anchor->nb_entries = 0;
        return AVERROR(ENOMEM);
    }

    anchor->nb_entries = keyframe_num;
    anchor->entry_index = 0;
	MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);
    if (stsz->entry_count > 0 &&
        st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        MOVOptiContext *moc = sc->opti_ctx;
        /* 8 bits per byte */
        size_t bytes = (size_t) av_rescale_rnd(stsz->entry_count, 1, 8, AV_ROUND_UP);
        moc->keyframe_bitmap = (unsigned char *) av_mallocz(bytes);
        if (!moc->keyframe_bitmap) {
            av_log(c->fc, AV_LOG_ERROR, "keyframe bitmpa for key error\n");
            return AVERROR(ENOMEM);
        }
    }
    return 0;
}

static int build_complete_index(MOVContext *mov, AVStream *st)
{
    MOVStreamContext *sc = st->priv_data;
    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);

    if (stsz->entry_count >= 0 &&
        stsz->entry_count <= MAX_MOV_COMPLETE_SAMPLE_NUM) {
        mov_opti_read_pending_atom(mov, st);
        destroy_opti_stream_ctx((MOVOptiContext **)&sc->opti_ctx);
        return 1;
    }

    return 0;
}

/* function name comes from ffmpeg commit message
 * 1. for video, subtitle
 * 2. audio with more stts entries, varible decoding Time to Sample
 * 3. audio with one stts entry and entry data(frame duration) valid
 */
static int build_index_use_chunk(MOVContext *mov, AVStream *st, int64_t dts)
{
    unsigned int i, j;
    MOVStreamContext *sc = st->priv_data;
    MOVOptiContext  *moc = sc->opti_ctx;
    int64_t last_dts              = 0;
    int64_t current_dts           = 0;
    int64_t current_offset        = 0;
    int64_t dts_correction        = 0;
    uint64_t stream_size          = 0;
    unsigned int distance         = 0;
    unsigned int stts_index       = 0;
    unsigned int stsc_index       = 0;
    unsigned int stss_index       = 0;
    unsigned int stps_index       = 0;
    unsigned int stts_sample      = 0;
    unsigned int stsc_sample      = 0;
    unsigned int sample_size      = 0;
    unsigned int current_sample   = 0;
    unsigned int rap_group_index  = 0;
    unsigned int rap_group_sample = 0;

    int rap_group_present = sc->rap_group_count && sc->rap_group;
    int key_off = (sc->keyframe_count && sc->keyframes[0] > 0) || (sc->stps_count && sc->stps_data[0] > 0);

    moc->rap_group_present = rap_group_present;

    last_dts = current_dts;
    if (!sc->sample_count || st->nb_index_entries) {
        return AVERROR(EINVAL);
    }

    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);
    if (stsz->entry_count >= UINT_MAX / sizeof(*st->index_entries)) {
        return AVERROR(EINVAL);
    }

    int ret = build_index_update_ctss_data(mov, st);
    if (ret < 0) {
        return ret;
    }

    MOVOptiStco *stco = MOV_OPTI_STCO(sc);
    MOVOptiStss *stss = MOV_OPTI_STSS(sc);
    for (i = 0; i < stco->entry_count; i++) {
        int64_t next_offset = i+1 < stco->entry_count ? read_stco_index(mov, st, i + 1) : INT64_MAX;
        current_offset = read_stco_index(mov, st, i);

        MOVOptiAVIndex opti_entry = {0};
        opti_entry.stco_index = i;
        opti_entry.stsc_index = stsc_index;
        while (mov_stsc_index_valid(stsc_index, sc->stsc_count) &&
            i + 1 == sc->stsc_data[stsc_index + 1].first) {
            stsc_index++;
        }

        if (next_offset > current_offset && sc->sample_size>0 && sc->sample_size < sc->stsz_sample_size &&
            sc->stsc_data[stsc_index].count * (int64_t)sc->stsz_sample_size > next_offset - current_offset) {
            av_log(mov->fc, AV_LOG_WARNING, "STSZ sample size %d invalid (too large), ignoring\n", sc->stsz_sample_size);
            sc->stsz_sample_size = sc->sample_size;
        }
        if (sc->stsz_sample_size>0 && sc->stsz_sample_size < sc->sample_size) {
            av_log(mov->fc, AV_LOG_WARNING, "STSZ sample size %d invalid (too small), ignoring\n", sc->stsz_sample_size);
            sc->stsz_sample_size = sc->sample_size;
        }

        for (j = 0; j < sc->stsc_data[stsc_index].count; j++) {
            int keyframe = 0;
            if (current_sample >= stsz->entry_count) {
                av_log(mov->fc, AV_LOG_ERROR, "wrong sample count\n");
                return AVERROR(EINVAL);
            }

            opti_entry.stsc_sample    = j;
            opti_entry.stts_index     = stts_index;
            opti_entry.stts_sample    = stts_sample;
            opti_entry.dts_correction = dts_correction;
            if (!sc->keyframe_absent && (!stss->entry_count || (current_sample+key_off) == read_stss_index(mov, st, stss_index))) {
                keyframe = 1;
                if (stss_index + 1 < stss->entry_count) {
                    stss_index++;
                }
            } else if (sc->stps_count && current_sample+key_off == sc->stps_data[stps_index]) {
                keyframe = 1;
                if (stps_index + 1 < sc->stps_count) {
                    stps_index++;
                }
            }
            if (rap_group_present && rap_group_index < sc->rap_group_count) {
                if (sc->rap_group[rap_group_index].index > 0) {
                    keyframe = 1;
                }
                if (++rap_group_sample == sc->rap_group[rap_group_index].count) {
                    rap_group_sample = 0;
                    rap_group_index++;
                }
            }
            if (sc->keyframe_absent && !sc->stps_count && !rap_group_present &&
                (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || (i==0 && j==0))) {
                 keyframe = 1;
            }
            if (keyframe) {
                distance = 0;
            }

            sample_size = sc->stsz_sample_size > 0 ? sc->stsz_sample_size : read_stsz_index(mov, st, current_sample);
            if (sc->pseudo_stream_id == -1 ||
               sc->stsc_data[stsc_index].id - 1 == sc->pseudo_stream_id) {
                if (sample_size > 0x3FFFFFFF) {
                    av_log(mov->fc, AV_LOG_ERROR, "Sample size %u is too large\n", sample_size);
                    return AVERROR(EINVAL);
                }

                int flags = keyframe ? AVINDEX_KEYFRAME : 0;
                opti_entry.timestamp = current_dts;
                opti_entry.base_sample = current_sample;
                opti_entry.current_offset = current_offset;
                build_index_add_entry(st, flags, &opti_entry, current_sample, (0 == i && 0 == j));
                av_log(mov->fc, AV_LOG_TRACE, "AVIndex stream %d, sample %u, offset %"PRIx64", dts %"PRId64", "
                        "size %u, distance %u, keyframe %d\n", st->index, current_sample,
                        current_offset, current_dts, sample_size, distance, keyframe);
                if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && st->nb_index_entries < 100) {
                    ff_rfps_add_frame(mov->fc, st, current_dts);
                }
            }

            current_offset += sample_size;
            stream_size += sample_size;
            /* A negative sample duration is invalid based on the spec,
             * but some samples need it to correct the DTS. */
            if (sc->stts_data[stts_index].duration < 0) {
                av_log(mov->fc, AV_LOG_WARNING,
                       "Invalid SampleDelta %d in STTS, at %d st:%d\n",
                       sc->stts_data[stts_index].duration, stts_index,
                       st->index);
                dts_correction += sc->stts_data[stts_index].duration - 1;
                sc->stts_data[stts_index].duration = 1;
            }
            current_dts += sc->stts_data[stts_index].duration;
            if (!dts_correction || current_dts + dts_correction > last_dts) {
                current_dts += dts_correction;
                dts_correction = 0;
            } else {
                /* Avoid creating non-monotonous DTS */
                dts_correction += current_dts - last_dts - 1;
                current_dts = last_dts + 1;
            }

            last_dts = current_dts;
            distance++;
            stts_sample++;
            current_sample++;
            if (stts_index + 1 < sc->stts_count && stts_sample == sc->stts_data[stts_index].count) {
                stts_sample = 0;
                stts_index++;
            }
            moc->last_sample_dts = current_dts;
        }
    }
    if (st->duration > 0) {
        st->codecpar->bit_rate = stream_size*8*sc->time_scale/st->duration;
    }
    return 0;
}

/* function name comes from ffmpeg commit message
 * for audio
*/
static int build_index_split_chunk(MOVContext *mov, AVStream *st, int64_t dts)
{
    unsigned int i;
    unsigned int total = 0;
    unsigned int chunk_samples = 0;
    int64_t current_dts     = dts;
    int64_t current_offset  = 0;
    unsigned int stsc_index = 0;
    MOVStreamContext *sc = st->priv_data;

    if (!(MOV_OPTI_STCO(sc)->entry_count)) {
        return AVERROR(EINVAL);
    }

    // compute total chunk count
    for (i = 0; i < sc->stsc_count; i++) {
        unsigned count, chunk_count;

        chunk_samples = sc->stsc_data[i].count;
        if (i != sc->stsc_count - 1 &&
            sc->samples_per_frame && chunk_samples % sc->samples_per_frame) {
            av_log(mov->fc, AV_LOG_ERROR, "error unaligned chunk\n");
            return AVERROR(EINVAL);
        }

        if (sc->samples_per_frame >= 160) { // gsm
            count = chunk_samples / sc->samples_per_frame;
        } else if (sc->samples_per_frame > 1) {
            unsigned samples = (1024/sc->samples_per_frame)*sc->samples_per_frame;
            count = (chunk_samples+samples-1) / samples;
        } else {
            count = (chunk_samples+1023) / 1024;
        }

        if (mov_stsc_index_valid(i, sc->stsc_count))
            chunk_count = sc->stsc_data[i+1].first - sc->stsc_data[i].first;
        else
            chunk_count = MOV_OPTI_STCO(sc)->entry_count - (sc->stsc_data[i].first - 1);
        total += chunk_count * count;
    }

    av_log(mov->fc, AV_LOG_TRACE, "chunk count %u\n", total);
    if (total >= UINT_MAX / sizeof(*st->index_entries) - st->nb_index_entries) {
        return AVERROR(EINVAL);
    }

    // populate index
    unsigned int current_sample = 0;
    sc->sample_count = total;
	MOV_OPTI_STSZ(sc)->entry_count = total;
    MOVOptiContext  *moc = sc->opti_ctx;
    for (i = 0; i < MOV_OPTI_STCO(sc)->entry_count; i++) {
        MOVOptiAVIndex opti_entry = {0};

        opti_entry.stco_index = i;
        opti_entry.stsc_index = stsc_index;
        current_offset = read_stco_index(mov, st, i);
        if (mov_stsc_index_valid(stsc_index, sc->stsc_count) &&
            i + 1 == sc->stsc_data[stsc_index + 1].first) {
            stsc_index++;
        }

        chunk_samples = sc->stsc_data[stsc_index].count;
        while (chunk_samples > 0) {
            unsigned size, samples;

            if (sc->samples_per_frame > 1 && !sc->bytes_per_frame) {
                avpriv_request_sample(mov->fc,
                       "Zero bytes per frame, but %d samples per frame",
                       sc->samples_per_frame);
                return AVERROR(EINVAL);
            }

            opti_entry.stsc_sample    = chunk_samples;
            opti_entry.stts_index     = 0;
            opti_entry.stts_sample    = 0;
            opti_entry.dts_correction = 0;
            if (sc->samples_per_frame >= 160) { // gsm
                samples = sc->samples_per_frame;
                size = sc->bytes_per_frame;
            } else {
                if (sc->samples_per_frame > 1) {
                    samples = FFMIN((1024 / sc->samples_per_frame)*
                                    sc->samples_per_frame, chunk_samples);
                    size = (samples / sc->samples_per_frame) * sc->bytes_per_frame;
                } else {
                    samples = FFMIN(1024, chunk_samples);
                    size = samples * sc->sample_size;
                }
            }

            if (current_sample >= total) {
                av_log(mov->fc, AV_LOG_ERROR, "wrong chunk count %u\n", total);
                return AVERROR(EINVAL);
            }
            if (size > 0x3FFFFFFF) {
                av_log(mov->fc, AV_LOG_ERROR, "Sample size %u is too large\n", size);
                return AVERROR(EINVAL);
            }

            opti_entry.timestamp = current_dts;
            opti_entry.base_sample = current_sample;
            opti_entry.current_offset = current_offset;
            build_index_add_entry(st, AVINDEX_KEYFRAME, &opti_entry,
                current_sample, (0 == i && chunk_samples == sc->stsc_data[stsc_index].count));
            av_log(mov->fc, AV_LOG_TRACE, "AVIndex stream %d, chunk %u, offset %"PRIx64", dts %"PRId64", "
                   "size %u, duration %u\n", st->index, i, current_offset, current_dts, size, samples);
            current_offset += size;
            current_dts += samples;
            chunk_samples -= samples;
            current_sample++;
            moc->last_sample_dts = current_dts;
        }
    }
    return 0;
}

/* External function below */
/* From ISO ISE 14496-12-2015 8.8.2.1 Movie Extends Header Box
 * movie extends header box, optional, provides the overall duration, including fragments, of a
 * fragmented movie. If this box is not present, the overall duration must be computed by examining each
 * fragment.
 */
int ff_mov_read_mehd(MOVContext *c, AVIOContext *pb, MOVAtom atom)
{
    int version = 0;
    int64_t duration = 0;
    AVFormatContext *ctx = c->fc;

    version = avio_rb32(pb) >> 24;
    /*
     * declares length of the presentation of the whole movie
     * including fragments (in the timescale indicated in the Movie Header Box). The value of this field
     * corresponds to the duration of the longest track, including movie fragments. If an MP4 file is
     * created in real time, such as used in live streaming, it is not likely that the
     * fragment_duration is known in advance and this box may be omitted.
     */
    if (version == 1) {
        duration = avio_rb64(pb);
    } else {
        duration = (int64_t) avio_rb32(pb);
    }

    av_log(c->fc, AV_LOG_INFO, "mehd duration %lld\n", duration);
    if (!c->duration) {
        c->duration = duration;
        if (!ctx) {
            return 0;
        }
        if (c->time_scale  > 0 &&
            (ctx->duration < AV_TIME_BASE || ctx->duration == AV_NOPTS_VALUE)) {
            ctx->duration = av_rescale(c->duration, AV_TIME_BASE, c->time_scale);
            c->duration_type = MOV_DUR_CACAL_TYPE_MEHD;
        }
    }
    return 0;
}

int ff_mov_is_network_stream(MOVContext *c)
{
    if (!c || !c->fc || !c->fc->filename) {
        return 0;
    }

    if (isnot_file(c->fc->filename)) {
        return 1;
    }

    return 0;
}

/* some clip has more moofs, parse total moof will cosume more time,
 * so return to playbcack when found the next moof
 */
int ff_mov_more_moof_found(
    MOVContext *c, AVIOContext *pb, MOVAtom *atom)
{
    if (!c->fc || !c->found_moof || atom->type != MKTAG('m','o','o','f')) {
        return 0;
    }

    if (c->fc->nb_streams < 1) {
        return 0;
    }

    for (int i = 0; i < c->fc->nb_streams; i++) {
        AVStream *st = c->fc->streams[i];
        MOVStreamContext *sc = st->priv_data;
        /* has sidx, parse according to sidx fragment */
        if (sc && sc->has_sidx) {
            return 0;
        }
    }

    if (c->found_moov && c->found_mdat) {
        return 1;
    }
    return 0;
}

int ff_mov_parse_pssh_header(MOVContext *c, AVIOContext *pb,
    int64_t pssh_offset, int64_t pssh_size, AVEncryptionInitInfo *info)
{
    int ret = 0;
    if (!c || !pb || !info) {
        return AVERROR(EINVAL);
    }

    info->header_data = NULL;
    info->header_data_size = 0;
    int64_t curr_offset = avio_tell(pb);
    if (avio_seek(pb, pssh_offset, SEEK_SET) < 0) {
        av_log(c->fc, AV_LOG_ERROR, "parse pssh header 0x%lld seek fail\n", pssh_offset);
        return AVERROR(EINVAL);
    }

    info->header_data = (uint8_t *) av_malloc(pssh_size);
    if (!info->header_data) {
        av_log(c->fc, AV_LOG_ERROR, "parse pssh header allocate memory fail\n");
        return AVERROR(ENOMEM);
    }

    if (avio_read(pb, info->header_data, pssh_size) != pssh_size) {
        av_log(c->fc, AV_LOG_ERROR, "Failed to read the pssh header\n");
        av_freep(&info->header_data);
        ret = AVERROR_INVALIDDATA;
        goto done;
    }

    info->header_data_size = pssh_size;
    (void) avio_seek(pb, curr_offset, SEEK_SET);
done:
    return ret;
}

int ff_mov_append_default_encryption_info(
    MOVContext *c, AVEncryptionInitInfo *info)
{
    AVEncryptionInitInfo *cur =
        c->default_encryption_info;

    if (!cur) {
        c->default_encryption_info = info;
        return 0;
    }
    // Append to the end of the list.
    do {
        if (!cur->next) {
            cur->next = info;
            break;
        }
    } while ((cur = cur->next));
    return 0;
}

int ff_mov_check_pssh_encryption_info(
    MOVContext *c, AVStream *st, AVEncryptionInitInfo *info)
{
    if (!st || !c || !info) {
        return 0;
    }

    MOVStreamContext *sc = st->priv_data;
    if (info->num_key_ids || info->key_ids) {
        return 0;
    }

    if (!sc ||
        !sc->cenc.default_encrypted_sample ||
        !sc->cenc.default_encrypted_sample->key_id ||
        sc->cenc.default_encrypted_sample->key_id_size != 16) {
        return 0;
    }

    uint8_t *key_id =
        sc->cenc.default_encrypted_sample->key_id;
    uint32_t key_id_size =
        sc->cenc.default_encrypted_sample->key_id_size;
    uint8_t key_id_zero[16] = {0};
    if (!memcmp(key_id, &key_id_zero[0], 16)) {
        return 0;
    }

    info->key_ids =
        av_mallocz_array(1, sizeof(*info->key_ids));
    if (!info->key_ids) {
        return AVERROR(ENOMEM);
    }

    info->num_key_ids = 1;
    info->key_ids[0] = av_mallocz(key_id_size);
    if (!info->key_ids[0]) {
        av_free(info->key_ids);
        return AVERROR(ENOMEM);
    }

    memcpy(info->key_ids[0], key_id, key_id_size);
    return 0;
}

int ff_mov_inject_encryption_info(MOVContext *mov,
    AVStream *st, AVEncryptionInfo *encrypted_sample, AVPacket *pkt)
{
    int ret;
    size_t size;
    uint8_t *side_data = av_encryption_info_add_side_data(encrypted_sample, &size);
    if (!side_data)
        return AVERROR(ENOMEM);

    int find_init_info = 0;
    uint8_t  *init_side_data = NULL;
    size_t init_side_data_size = 0;
    init_side_data = av_stream_get_side_data(st, AV_PKT_DATA_ENCRYPTION_INIT_INFO, &init_side_data_size);
    if (!init_side_data_size || !init_side_data) /* No init data */ {
        if (pkt->stream_index >= 0 &&
            pkt->stream_index < mov->fc->nb_streams) {
            for (int i = pkt->stream_index - 1; i >= 0; i--) {
                AVStream *ist =  mov->fc->streams[i];
                init_side_data = av_stream_get_side_data(ist,
                    AV_PKT_DATA_ENCRYPTION_INIT_INFO, &init_side_data_size);
                if (init_side_data && init_side_data_size) {
                    find_init_info = 1;
                }
            }
            if (!find_init_info) {
                for (int i = pkt->stream_index + 1; i < mov->fc->nb_streams; i++) {
                    AVStream *ist =  mov->fc->streams[i];
                    init_side_data = av_stream_get_side_data(ist,
                        AV_PKT_DATA_ENCRYPTION_INIT_INFO, &init_side_data_size);
                    if (init_side_data && init_side_data_size) {
                        find_init_info = 1;
                    }
                }
            }
            if (find_init_info) {
                uint8_t  *data = av_stream_new_side_data(
                    st, AV_PKT_DATA_ENCRYPTION_INIT_INFO, init_side_data_size);
                if (data) {
                    st->inject_global_side_data = 1;
                    memcpy(data, init_side_data, init_side_data_size);
                }
            }

        }
    }
    ret = av_packet_add_side_data(pkt, AV_PKT_DATA_ENCRYPTION_INFO, side_data, size);
    if (ret < 0)
        av_free(side_data);
    return ret;
}

int ff_mov_opti_build_index(MOVContext *c, AVStream *st)
{
    int ret = 0;
    int64_t current_dts = 0;
    MOVContext *mov = c;

    update_elst_current_dts(mov, st, &current_dts);
    MOVStreamContext *sc = st->priv_data;
    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);

    /* samll sample case use origin flow */
    if (build_complete_index(mov, st)) {
        return AVERROR(EAGAIN);
    }

    if ((ret = build_keyframe_entry(c, st)) < 0) {
        return ret;
    }

    MOVOptiContext *moc = sc->opti_ctx;
    /* only use old uncompressed audio chunk demuxing when stts specifies it */
    if (!(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
          sc->stts_count == 1 && sc->stts_data[0].duration == 1)) {
        current_dts -= sc->dts_shift;
        ret = build_index_use_chunk(mov, st, current_dts);
        moc->build_index_mode = 0;
    } else {
        ret = build_index_split_chunk(mov, st, current_dts);
        moc->build_index_mode = 1;
    }

    moc->anchor.nb_entries = moc->anchor.entry_index;
    moc->anchor.entry_index = 0;
    if (ret < 0) {
        mov_opti_read_pending_atom(mov, st);
        destroy_opti_stream_ctx((MOVOptiContext **)&sc->opti_ctx);
        return AVERROR(EAGAIN);
    }

    show_anchor_entries(mov, sc->opti_ctx);
    ret = update_opti_index(mov, st);
    // Update start time of the stream.
    if (st->start_time == AV_NOPTS_VALUE && st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && st->nb_index_entries > 0) {
        st->start_time = st->index_entries[0].timestamp + sc->dts_shift;
        if (sc->ctts_data) {
            st->start_time += sc->ctts_data[0].duration;
        }
    }
    return ret;
}

int ff_mov_opti_read_stco(MOVContext *c, AVStream *st, AVIOContext *pb, MOVAtom atom)
{
    if (!c || !st || !pb) {
        return 0;
    }

    MOVStreamContext *sc = st->priv_data;
    if (!sc || !(sc->opti_ctx)) {
        return 0;
    }

    avio_r8(pb); /* version */
    avio_rb24(pb); /* flags */

    unsigned int i, entries;
    entries = avio_rb32(pb);
    if (!entries) {
        return 0;
    }

    sc->chunk_count = 0;
    av_freep(&(sc->chunk_offsets));
    MOVOptiStco *scto = MOV_OPTI_STCO(sc);

    scto->type = atom.type;
    scto->entry_count = entries;
    scto->base_position = avio_tell(pb);

    /* compressed data has different pb, if not equal
     * c->fc->pb is orgin data, pb is the decompressed data
     * only equal, we can seek to base pos and read.
     */
    if (c->fc->pb == pb) {
        entries = (unsigned int) FFMIN(entries, MAX_MOV_INDEX_ENTRIES);
    }
    sc->chunk_offsets = av_malloc_array(entries, sizeof(*sc->chunk_offsets));
    if (!sc->chunk_offsets) {
        sc->chunk_count = 0;
        return AVERROR(ENOMEM);
    }

    /* check entries number, must, because parse will check size we used */
    if (atom.type == MKTAG('s','t','c','o')) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->chunk_offsets[i] = avio_rb32(pb);
        }
        for (; i < scto->entry_count && !pb->eof_reached; i++) {
            (void) avio_rb32(pb);
        }
    } else if (atom.type == MKTAG('c','o','6','4')) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->chunk_offsets[i] = avio_rb64(pb);
        }
        for (; i < scto->entry_count && !pb->eof_reached; i++) {
            (void) avio_rb64(pb);
        }
    } else {
        sc->chunk_count = 0;
        av_freep(&(sc->chunk_offsets));
        scto->entry_count = 0;
        av_log(c->fc, AV_LOG_ERROR, "Invalid STCO atom size\n");
        return AVERROR_INVALIDDATA;
    }

    scto->entry_count = i;
    scto->entry_index = sc->chunk_count = FFMIN(i, entries);
    if (pb->eof_reached) {
        av_log(c->fc, AV_LOG_WARNING, "reached eof, corrupted STCO atom\n");
        return AVERROR_EOF;
    }

    return 0;
}

int ff_mov_opti_read_stss(MOVContext *c, AVStream *st, AVIOContext *pb, MOVAtom atom)
{
    if (!c || !st || !pb) {
        return 0;
    }

    MOVStreamContext *sc = st->priv_data;
    if (!sc || !(sc->opti_ctx)) {
        return 0;
    }

    avio_r8(pb);   /* version */
    avio_rb24(pb); /* flags */

    unsigned int i, entries;
    entries = avio_rb32(pb);
    av_log(c->fc, AV_LOG_TRACE, "keyframe_count = %u\n", entries);
    if (!entries) {
        sc->keyframe_absent = 1;
        if (!st->need_parsing && st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            st->need_parsing = AVSTREAM_PARSE_HEADERS;
        }
        av_log(c->fc, AV_LOG_WARNING, "keyframe miss, need parse header\n", entries);
        return 0;
    }

    if (entries >= UINT_MAX / sizeof(int)) {
        return AVERROR_INVALIDDATA;
    }

    sc->keyframe_count = 0;
    av_freep(&(sc->keyframes));
    MOVOptiStss *stss = MOV_OPTI_STSS(sc);

    stss->entry_count = entries;
    stss->base_position = avio_tell(pb);

    /* compressed data has different pb, if not equal
     * c->fc->pb is orgin data, pb is the decompressed data
     * only equal, we can seek to base pos and read.
     */
    if (c->fc->pb == pb) {
        entries = (unsigned int) FFMIN(entries, MAX_MOV_INDEX_ENTRIES);
    }
    sc->keyframes = av_malloc_array(entries, sizeof(*sc->keyframes));
    if (!sc->keyframes) {
        sc->keyframe_count = 0;
        return AVERROR(ENOMEM);
    }

    /* check entries number, must, because parse will check size we used */
    for (i = 0; i < entries && !pb->eof_reached; i++) {
        sc->keyframes[i] = avio_rb32(pb);
    }
    for (; i < stss->entry_count && !pb->eof_reached; i++) {
        (void) avio_rb32(pb);
    }

    stss->entry_count = i;
    stss->entry_index = sc->keyframe_count = FFMIN(i, entries);
    if (pb->eof_reached) {
        av_log(c->fc, AV_LOG_WARNING, "reached eof, corrupted STSS atom\n");
        return AVERROR_EOF;
    }
    return 0;
}

int ff_mov_opti_read_stsz(MOVContext *c, AVStream *st, AVIOContext *pb, MOVAtom atom)
{
    if (!c || !st || !pb) {
        return 0;
    }

    MOVStreamContext *sc = st->priv_data;
    if (!sc || !(sc->opti_ctx)) {
        return 0;
    }

    avio_r8(pb);   /* version */
    avio_rb24(pb); /* flags */
    unsigned int i = 0, entries, sample_size, field_size;
    if (atom.type == MKTAG('s','t','s','z')) {
        sample_size = avio_rb32(pb);
        if (!sc->sample_size) { /* do not overwrite value computed in stsd */
            sc->sample_size = sample_size;
        }
        sc->stsz_sample_size = sample_size;
        field_size = 32;
    } else {
        sample_size = 0;
        avio_rb24(pb); /* reserved */
        field_size = avio_r8(pb);
    }

    entries = avio_rb32(pb);
    av_log(c->fc, AV_LOG_TRACE, "sample_size = %u sample_count = %u\n", sc->sample_size, entries);

    MOVOptiStsz *stsz = MOV_OPTI_STSZ(sc);
    sc->sample_sizes = NULL;
    sc->sample_count = stsz->entry_count = entries;
    stsz->base_position = avio_tell(pb);
    /* fixed sample size */
    if (sample_size || !entries) {
        return 0;
    }

    if (field_size != 4 && field_size != 8 && field_size != 16 && field_size != 32) {
        av_log(c->fc, AV_LOG_ERROR, "Invalid sample field size %u\n", field_size);
        return AVERROR_INVALIDDATA;
    }

    if (entries >= (UINT_MAX - 4) / field_size) {
        return AVERROR_INVALIDDATA;
    }

    sc->sample_count = 0;
    av_freep(&(sc->sample_sizes));

    stsz->field_size    = field_size;
    stsz->entry_count   = entries;

    /* compressed data has different pb, if not equal
     * c->fc->pb is orgin data, pb is the decompressed data
     * only equal, we can seek to base pos and read.
     */
    if (c->fc->pb == pb) {
        entries = (unsigned int) FFMIN(entries, MAX_MOV_INDEX_ENTRIES);
    }
    sc->sample_sizes = av_malloc_array(entries, sizeof(*sc->sample_sizes));
    if (!sc->sample_sizes) {
        sc->sample_count = 0;
        return AVERROR(ENOMEM);
    }

    /* check sample_count and data size, must, because parse will check size we used */
    if (4 == stsz->field_size) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sample_size = avio_r8(pb);
            sc->sample_sizes[i] = (sample_size & 0xF0) >> 4;
            sc->data_size +=  sc->sample_sizes[i];
            if (i < entries - 1) {
                i++;
                sc->sample_sizes[i] = (sample_size & 0xF);
                sc->data_size +=  sc->sample_sizes[i];
            }
        }
        for (; i < stsz->entry_count && !pb->eof_reached; i++) {
            sample_size = avio_r8(pb);
            sc->data_size += (sample_size & 0xF0) >> 4;
            if (i < entries - 1) {
                i++;
                sc->data_size += (sample_size & 0xF);
            }
        }
    } else if (8 == stsz->field_size) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->sample_sizes[i] = avio_r8(pb);
            sc->data_size += sc->sample_sizes[i];
        }
        for (; i < stsz->entry_count && !pb->eof_reached; i++) {
            sc->data_size += avio_r8(pb);
        }
    } else if (16 == stsz->field_size) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->sample_sizes[i] = avio_rb16(pb);
            sc->data_size += sc->sample_sizes[i];
        }
        for (; i < stsz->entry_count && !pb->eof_reached; i++) {
            sc->data_size += avio_rb16(pb);
        }
    } else if (32 == stsz->field_size) {
        for (i = 0; i < entries && !pb->eof_reached; i++) {
            sc->sample_sizes[i] = avio_rb32(pb);
            sc->data_size += sc->sample_sizes[i];
        }
        for (; i < stsz->entry_count && !pb->eof_reached; i++) {
            sc->data_size += avio_rb32(pb);
        }
    } /* no else case */

    stsz->entry_count = i < stsz->entry_count ? i : stsz->entry_count;
    stsz->entry_index = sc->sample_count = FFMIN(i, entries);
    if (pb->eof_reached) {
        av_log(c->fc, AV_LOG_WARNING, "reached eof, corrupted STSZ atom\n");
        return AVERROR_EOF;
    }

    return 0;
}

AVIndexEntry *ff_mov_opti_get_next_sample(MOVContext *c, AVStream *st)
{
    if (!c || !st) {
      return  NULL;
    }

    return find_next_sample(c, st);
}

int ff_mov_opti_read_seek(MOVContext *c, AVStream *st, int64_t timestamp, int flags)
{
    if (!c || !st) {
        return AVERROR_INVALIDDATA;
    }

    MOVStreamContext  *sc = st->priv_data;
    if (!stream_has_anchor(sc)) {
        int sample = av_index_search_timestamp(st, timestamp, flags);
        av_log(c->fc, AV_LOG_TRACE, "stream %d, timestamp %"PRId64", sample %d\n", st->index, timestamp, sample);
        if (sample < 0 && st->nb_index_entries && timestamp < st->index_entries[0].timestamp) {
            sample = 0;
        }
        if (sample < 0) {
            /* not sure what to do */
            return AVERROR_INVALIDDATA;
        }
        return sample;
    }

    /* Has anchor , search anchor first */
    int sample = opti_seek_anchor_sample(c, st, timestamp, flags);
    if (sample < 0) {
        MOVOptiAnchor *anchor = MOV_OPTI_ANCHOR(sc);
        anchor->entry_index = anchor->nb_entries;
        av_log(c->fc, AV_LOG_WARNING, "[%d] seek timestamp %"PRId64" fail\n", st->index, timestamp);
        return AVERROR_INVALIDDATA;
    }
    return sample;
}

int ff_mov_opti_curr_base_sample_id(MOVContext *c, AVStream *st)
{
    if (!c || !st) {
        return 0;
    }
    MOVStreamContext  *sc = st->priv_data;
    if (!stream_has_anchor(sc)) {
        return 0;
    }

    (void) c;
    MOVOptiContext   *moc = sc->opti_ctx;
    MOVOptiAVIndex *curr = MOV_OPTI_ANCHOR_CURR(moc);
    /* seek fail, has no curr */
    if (!curr) {
        return 0;
    }
    return curr->base_sample;
}

/* use before and during index build */
int ff_mov_opti_stream_enable(MOVContext *c, AVStream *st)
{
    /* Not allow low memory optium */
    if(NULL == c) {
        return 0;
    }

    if (INT_MAX == c->max_opti_index_entries) {
        return 0;
    }

    int ret = 0;
    MOVStreamContext *sc = NULL;

    if (!c || !(c->fc) || !(c->fc->pb)) {
        goto done;
    }
    /* we need seek when parse table */
    if (!(c->fc->pb->seekable & AVIO_SEEKABLE_NORMAL)) {
        goto done;
    }

    if (!st || (!(st->priv_data))) {
        goto done;
    }
    sc = st->priv_data;
    /* sidx has low memory index */
    if (!sc || sc->has_sidx || !(sc->opti_ctx)) {
        goto done;
    }
    /* only support local file */
    if (isnot_file(c->fc->filename)) {
        goto done;
    }
    ret = 1;
done:
    if (!ret && sc && sc->opti_ctx) {
        /* create when setup stream, destroy when not need */
        destroy_opti_stream_ctx((MOVOptiContext **)&sc->opti_ctx);
    }
    return ret;
}

int ff_mov_opti_stream_create_ctx(MOVContext *c, AVStream *st)
{
    /* Not allow low memory optium */
    if (INT_MAX == c->max_opti_index_entries) {
        return 0;
    }

    MOVStreamContext *sc = st->priv_data;
    if (sc->opti_ctx) {
        destroy_opti_stream_ctx((MOVOptiContext **)&sc->opti_ctx);
    }

    MOVOptiContext *ctx = av_mallocz(sizeof(MOVOptiContext));
    if (!ctx) {
        return AVERROR(ENOMEM);
    }

    init_opti_stream_ctx(ctx);
    sc->opti_ctx = ctx;
    return 0;
}

int ff_mov_opti_stream_destroy_ctx(AVStream *st)
{
    if (!st || !(st->priv_data)) {
        return 0;
    }

    MOVStreamContext *sc = st->priv_data;
    destroy_opti_stream_ctx((MOVOptiContext **)&sc->opti_ctx);

    return 0;
}
