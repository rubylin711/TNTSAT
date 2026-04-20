/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "adaptive.h"
#include "internal.h"
#include "avio_internal.h"
#include "libavutil/time.h"
#include "libavutil/thread.h"
#include "libavformat/url.h"
#include "libavutil/opt.h"
#include "hlsplaylist.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Not advance fragment */
#define ADAPTIVE_ADVANCE_NONE               0
/* Advance fragment normal */
#define ADAPTIVE_ADVANCE_NORMAL             1
/* Advance fragment but current packet belongs to next fragment, need drop */
#define ADAPTIVE_ADVANCE_DROP_PACKET        2
#define DEFAULT_BITRATE_LIMIT              0.8f
#define REPLACE_AND_SAVE(new, save, replace) \
do {                                         \
    (save) = (replace);                      \
    (replace) = (new);                       \
} while (0)

static int64_t update_current_bitrate(AdaptivePlaylistContext *c);
static int adaptive_to_app_message(
    AVFormatContext *s, int type, void *data, size_t data_size);

static void *ff_adaptive_child_next(void *obj, void *prev)
{
    void *ret = NULL;
    AVIOContext *s = obj;
    AdaptivePlaylistContext *c = s->opaque;
    const AVClass *cls = *(AVClass **) c;
    if (cls->child_next) {
        s->opaque = c->opaque;
        ret = cls->child_next(s, prev);
        s->opaque = (void *) c;
    }

    return ret;
}

const static AVClass *s_avio_class = NULL;
static const AVClass *ff_adaptive_child_class_next(const AVClass *prev)
{
	return prev ? NULL : s_avio_class;
}

static int get_std_framerate(int i)
{
    if (i < 30*12)
        return (i + 1) * 1001;
    i -= 30*12;

    if (i < 30)
        return (i + 31) * 1001 * 12;
    i -= 30;

    if (i < 3)
        return ((const int[]) { 80, 120, 240})[i] * 1001 * 12;

    i -= 3;

    return ((const int[]) { 24, 30, 60, 12, 15, 48 })[i] * 1000 * 12;
}

static void check_stream_framerate(
    AVFormatContext *s, AVRational *pls_fps, AVStream *st)
{
    if (pls_fps->den == 0 || pls_fps->num == 0) {
        return;
    }

    AVRational guess_fps = av_guess_frame_rate(s, st, NULL);
    int use_mpd_fps = (guess_fps.den == 0 || guess_fps.num == 0);

    if (use_mpd_fps) {
        goto done;
    }

    int prefer_mpd = 0;
    int prefer_guess = 0;
    for (int j = 0; j < MAX_STD_TIMEBASES; j++) {
        AVRational std_fps = { get_std_framerate(j), 12 * 1001 };
        if (guess_fps.den * std_fps.num == guess_fps.num * std_fps.den) {
            prefer_guess = 1;
        }
        if (pls_fps->den * std_fps.num == pls_fps->num * std_fps.den) {
            prefer_mpd = 1;
        }
        if (prefer_guess && prefer_mpd) {
            return;
        }
    }

    if (!prefer_guess && prefer_mpd) {
        use_mpd_fps = 1;
    }
done:
    if (use_mpd_fps) {
        st->avg_frame_rate.den = st->r_frame_rate.den = pls_fps->den;
        st->avg_frame_rate.num = st->r_frame_rate.num = pls_fps->num;
    }

    if (!st->avg_frame_rate.num || !st->avg_frame_rate.den) {
        if (st->r_frame_rate.num && st->r_frame_rate.den) {
            st->avg_frame_rate.den = st->r_frame_rate.den;
            st->avg_frame_rate.num = st->r_frame_rate.num;
        } else if (pls_fps->num && pls_fps->den) {
            st->avg_frame_rate.den = st->r_frame_rate.den = pls_fps->den;
            st->avg_frame_rate.num = st->r_frame_rate.num = pls_fps->num;
        }
    }

    av_log(s, AV_LOG_INFO, "update stream framerate num:%d den:%d\n", pls_fps->num, pls_fps->den);
}

/* Store cluster, which will be changed by fllowing parse */
static int web_check_stream(
    AdaptivePlaylistContext *c, AVStream *st)
{
    AVIndexEntry *index_entries = NULL;
    AdaptiveFormat *format = &c->format;
    AdaptiveFragment *frag = &format->frag;
    unsigned int index_entries_allocated_size = 0;

    if (frag->index_entries) {
        av_freep(&frag->index_entries);
    }

    frag->current = -1;
    frag->nb_index_entries = 0;
    index_entries_allocated_size =
        st->nb_index_entries * sizeof(*st->index_entries);
    index_entries = av_malloc(index_entries_allocated_size);
    if (!index_entries) {
        return AVERROR(ENOMEM);
    }

    memcpy(index_entries, st->index_entries, index_entries_allocated_size);
    frag->index_entries    = index_entries;
    frag->nb_index_entries = st->nb_index_entries;
    for (int i = 0; i < frag->nb_index_entries; i++) {
        av_log(c->ctx, AV_LOG_DEBUG, "adaptive web check stream pos:%" PRId64
            " timestamp:%" PRId64"\n",	frag->index_entries[i].pos, frag->index_entries[i].timestamp);
    }
    return 0;
}

/* Multi stsd entry for one track */
static int mp4_check_stream_extradata(
    AdaptivePlaylistContext *c, AVStream *st)
{
    AVFormatContext *s = c->ctx;
    MOVContext *mov = s->priv_data;
    AVCodecParameters *codecpar = st->codecpar;
    MOVStreamContext *sc = st->priv_data;
    if (!sc            || sc->stsd_count <= 1  || /* only one stsd entry */
        !codecpar      || !codecpar->extradata ||
        !sc->extradata || !sc->extradata_size  ||
        1 != mov->fragment.found_tfhd) {
        return 0;
    }

    MOVFragmentIndex *frag_index = &mov->frag_index;
    if (!frag_index->item ||
        frag_index->current >= frag_index->nb_items) {
        return 0;
    }

    MOVFragmentIndexItem *item =
        &frag_index->item[frag_index->current];
    if (!item->stream_info) {
        return 0;
    }

    MOVFragmentStreamInfo *frag_stream_info = NULL;
    for (int i = 0; i < item->nb_stream_info; i++) {
        frag_stream_info = &item->stream_info[i];
        if (st->id == frag_stream_info->id) {
            break;
        }
        frag_stream_info = NULL;
    }

    if (!frag_stream_info) {
        return 0;
    }

    int stsd_sample_description_id = frag_stream_info->stsd_id - 1;
    if (stsd_sample_description_id < 0 ||
        stsd_sample_description_id >= sc->stsd_count) {
        return 0;
    }

    uint8_t *extradata = sc->extradata[stsd_sample_description_id];
    int extradata_size = sc->extradata_size[stsd_sample_description_id];
    if (extradata_size == codecpar->extradata_size &&
        !memcmp(extradata, codecpar->extradata, codecpar->extradata_size)) {
        return 0;
    }

    if (ff_alloc_extradata(codecpar, extradata_size) < 0) {
        av_log(s, AV_LOG_ERROR, "Reset stream extra data and memory malloc fail\n");
        return AVERROR(ENOMEM);
    }
    memcpy(codecpar->extradata, extradata, extradata_size);
    return 0;
}

static int mp4_check_stream(
    AdaptivePlaylistContext *c, AVStream *st) {
    return mp4_check_stream_extradata(c, st);
}

static int webm_advance_fragment(
    AdaptivePlaylistContext *c, AVPacket *pkt, int64_t *timestamp)
{
    int ret = ADAPTIVE_ADVANCE_NONE;
    AVFormatContext *s = c->ctx;
    AdaptiveFormat *format = &c->format;
    AdaptiveFragment *frag = &format->frag;

    if (!frag->index_entries) {
        return ADAPTIVE_ADVANCE_NONE;
    }

    /* find out current cluster index */
    if (-1 == frag->current) {
        frag->current =
            ff_index_search_timestamp(
                frag->index_entries,
                frag->nb_index_entries, pkt->pts, AVSEEK_FLAG_BACKWARD);
    }

    if (frag->current + 1 >= frag->nb_index_entries) {
        return ADAPTIVE_ADVANCE_NONE;
    }

    AVIndexEntry *e = &frag->index_entries[frag->current + 1];
    int64_t next_pos = pkt->pos + pkt->size;
    if (next_pos >= e->pos) {
        ret = ADAPTIVE_ADVANCE_NORMAL;
        av_log(s, AV_LOG_INFO,
               "[%d] advance fragment pos:%lld size:%d next cluster:%lld sample:%lld\n",
               frag->current, pkt->pos, pkt->size, e->pos, next_pos);
        frag->current += 1;
        if (timestamp) {
            AVStream *stream = s->streams[0];
            /* precision loss in future time to timestamp, so need 50ms more time */
            *timestamp = av_rescale_q_rnd(e->timestamp,
                stream->time_base, AV_TIME_BASE_Q, AV_ROUND_UP) + 50000;
        }
        /* packet belongs to next cluster */
        if (pkt->pos >= e->pos) {
            ret = ADAPTIVE_ADVANCE_DROP_PACKET;
        }
    }
    return ret;
}

static int mp4_advance_fragment(
    AdaptivePlaylistContext *c, AVPacket *pkt, int64_t *timestamp)
{
    int ret = ADAPTIVE_ADVANCE_NONE;
    AVFormatContext *s = c->ctx;
    MOVContext *mov = s->priv_data;
    MOVFragmentIndex *frag_index = &mov->frag_index;
    if (frag_index->current + 1 >= frag_index->nb_items) {
        return ADAPTIVE_ADVANCE_NONE;
    }

    MOVFragmentIndexItem *item = &frag_index->item[frag_index->current + 1];
    int64_t next_pos = pkt->pos + pkt->size;
    if (next_pos >= item->moof_offset) {
        ret = ADAPTIVE_ADVANCE_NORMAL;
        av_log(s, AV_LOG_INFO,
               "[%d] advance fragment pos:%lld size:%d next moof:%lld sample:%lld\n",
               frag_index->current, pkt->pos, pkt->size, item->moof_offset, next_pos);
        AVStream *st = s->streams[0];
        if (timestamp) {
            MOVFragmentStreamInfo *sinfo = &item->stream_info[0];
            *timestamp = av_rescale_q_rnd(sinfo->sidx_pts, st->time_base, AV_TIME_BASE_Q, AV_ROUND_UP);
        }
    }
    return ret;
}

static int mp4_release_format(AdaptivePlaylistContext *c)
{
    return 0;
}

static int webm_release_format(AdaptivePlaylistContext *c)
{
    AdaptiveFormat *format = &c->format;
    if (format->frag.index_entries) {
        av_freep(&format->frag.index_entries);
    }
    return 0;
}

#define OFFSET(x) offsetof(URLContext,x)
#define E AV_OPT_FLAG_ENCODING_PARAM
#define D AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    { NULL }
};

AVClass ff_adatpive_class = {
    .class_name = "Adaptive AVIOContext",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
    .option     = options,
    .child_next = ff_adaptive_child_next,
    .child_class_next = ff_adaptive_child_class_next,
};

static void dump_expose_adaptive_information(AVFormatContext *s)
{
    AVAdaptive *adaptive = &s->adaptive;
    av_log(s, AV_LOG_INFO,
        "Dump exposed adaptive playlist, list num:%d mode:%d\n",
        adaptive->n_list_num, adaptive->adaptive_resolution_mode);
    for (int i = 0; i < adaptive->n_list_num; i++) {
        AVAdaptiveList *list = &adaptive->lists[i];
        av_log(s, AV_LOG_INFO,
            "[%d] Enable:%d\n"
            "Playlist Index:%d Type:%d Stream num:%d\n"
            "Width:%d Height:%d Bandwidth:%d Rramerate:%d %d\n"
            "Streams:%p Priority:%d Penalty count:%d\n", i,
            list->enable, list->playlist_index, list->type, list->nb_streams,
            list->width, list->height, list->bandwidth,
            list->framerate.den, list->framerate.num, list->streams, list->priority, list->penalty_cnt);
        for (int j = 0; j < list->nb_streams; j++) {
            av_log(s, AV_LOG_INFO, "stream[%d]:%p\n%s", j, list->streams[j], (j == list->nb_streams - 1) ? "\n" : "");
        }
    }

    av_log(s, AV_LOG_INFO, "Dump exposed adaptive playlist end\n\n");
}


/* return -1: a ahead, 1: b ahead, 0:not change */
static int compare_bandwith(const void *a, const void *b) {
    AVAdaptiveList *la = (AVAdaptiveList *)(a);
    AVAdaptiveList *lb = (AVAdaptiveList *)(b);

    /* bigger,move back */
    if (la->bandwidth > lb->bandwidth) {
        return 1;
    } else if (la->bandwidth < lb->bandwidth) {
        return -1;
    }

    return 0;
}

static void free_adaptive(AVAdaptive *adpt)
{
    int list_num = 0;

    if (adpt->lists) {
        av_freep(&adpt->lists);
    }
}

static void reset_download_info(PlaylistDownloadInfo *info)
{
    memset(info, 0, sizeof(PlaylistDownloadInfo));
    info->download_start_time = -1;
}

static int download_start(AdaptivePlaylistContext *c)
{
    if (!c) {
        return 0;
    }

    PlaylistDownloadInfo *info = &c->info;
    reset_download_info(&c->info);

    info->download_start_time = av_gettime();
    return 0;
}

static int64_t download_finish(
    AVFormatContext *parent, AdaptivePlaylistContext *c)
{
    if (!c) {
        return 0;
    }

    int64_t idle_time = 0;
    int ret = adaptive_to_app_message(parent,
        AV_ADAPTIVE_QUERY_IDLE_TIME, &idle_time, sizeof(idle_time));
    if (ret < 0) {
        idle_time = 0;
    }

    PlaylistDownloadInfo *info = &c->info;
    if (-1 == info->download_start_time ||
         0 == info->fragment_bytes_downloaded) {
        info->last_bitrate = -1;
        return 0;
    }

    int64_t sum_download_time = info->sum_download_time;
    int64_t all_download_time = av_gettime() - info->download_start_time;
    info->last_download_time  = all_download_time - idle_time;
    if (sum_download_time <= 0 ||
        all_download_time <= 0 ||
        info->last_download_time <= 0) {
        info->last_bitrate = -1;
        return 0;
    }
    /* More time in sending packet, can be some small packet */
    if (!c->is_live && info->last_download_time > (sum_download_time * 10)) {
        /* Download bytes:884736 time(us) 5859, all:11655202 idle:5640000,
         * pkt:644 644 mem(1672320 4194304 39) fr:30 lower resolution
         */
        unsigned int shit_bits = 1;
        if (info->last_download_time > (sum_download_time * 1000)) {
            shit_bits = 3;
        } else if (info->last_download_time > (sum_download_time * 100)) {
            shit_bits = 2;
        }
        info->last_download_time = (info->last_download_time + sum_download_time) >> shit_bits;
    } else if (c->is_live) {
        info->last_download_time = sum_download_time;
    }
    av_log(c->ctx, AV_LOG_INFO,
           "Download bytes:%lld time(us) %lld, all:%lld idle:%lld\n",
               info->fragment_bytes_downloaded, sum_download_time, all_download_time, idle_time);

    info->last_bitrate = av_rescale(
        info->fragment_bytes_downloaded * AV_TIME_BASE, 8, info->last_download_time);

    return update_current_bitrate(c);
}

static int64_t update_average_bitrate(
    PlaylistDownloadInfo *info, int64_t new_bitrate)
{
    int index = info->moving_index % NUM_LOOKBACK_FRAGMENTS;

    info->moving_bitrate -= info->fragment_bitrates[index];
    info->fragment_bitrates[index] = new_bitrate;
    info->moving_bitrate += new_bitrate;

    info->moving_index += 1;

    if (info->moving_index > NUM_LOOKBACK_FRAGMENTS) {
        return info->moving_bitrate / NUM_LOOKBACK_FRAGMENTS;
    }
    return info->moving_bitrate / info->moving_index;
}

/* must be called with manifest_lock taken */
static int64_t update_current_bitrate(AdaptivePlaylistContext *c)
{
    int64_t average_bitrate;
    int64_t fragment_bitrate;
    PlaylistDownloadInfo *info = &c->info;

    fragment_bitrate = info->last_bitrate;
    av_log(c->ctx, AV_LOG_INFO,
           "Download bitrate is : %lld bps\n", fragment_bitrate);

    average_bitrate = update_average_bitrate(info, fragment_bitrate);

    av_log(c->ctx, AV_LOG_INFO,
           "last fragment bitrate was %llu\n", fragment_bitrate);
    av_log(c->ctx, AV_LOG_INFO,
           "Last %u fragments average bitrate is %llu\n", NUM_LOOKBACK_FRAGMENTS, average_bitrate);

    /* Conservative approach, make sure we don't upgrade too fast */
    info->current_download_rate = FFMIN(average_bitrate, fragment_bitrate);
    info->current_download_rate = (int64_t) (info->current_download_rate * DEFAULT_BITRATE_LIMIT);
    av_log(c->ctx, AV_LOG_INFO,
           "Bitrate after bitrate limit (%0.2f): %llu\n",
           DEFAULT_BITRATE_LIMIT, info->current_download_rate);

    return info->current_download_rate;
}

static int adaptive_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    AdaptivePlaylistContext *c = opaque;
    if (!c || !c->read_packet || !buf || buf_size <= 0) {
        return 0;
    }

    int64_t download_start_time = av_gettime();
    int read_size = c->read_packet(c->opaque, buf, buf_size);
    PlaylistDownloadInfo *info = &c->info;
    if (-1 != info->download_start_time && read_size >= 0) {
        info->sum_download_time += av_gettime() - download_start_time;
        info->fragment_bytes_downloaded += read_size;
    }
    return read_size;
}

static int adaptive_write_packet(void *opaque, uint8_t *buf, int buf_size)
{
    AdaptivePlaylistContext *c = opaque;
    if (!c || !c->write_packet || !buf || buf_size <= 0) {
        return 0;
    }
    return c->write_packet(c->opaque, buf, buf_size);
}

static int64_t adaptive_seek(void *opaque, int64_t offset, int whence)
{
    AdaptivePlaylistContext *c = opaque;
    if (!c || !c->seek) {
        return 0;
    }

    int64_t ret = c->seek(c->opaque, offset, whence);
    reset_download_info(&c->info);
    return ret;
}

static int adaptive_short_seek(void *opaque)
{
    AdaptivePlaylistContext *c = opaque;
    if (!c || !c->short_seek_get) {
        return 0;
    }
    int ret = c->short_seek_get(c->opaque);
    reset_download_info(&c->info);
    return ret;
}

static int adaptive_read_pause(void *opaque, int pause)
{
    AdaptivePlaylistContext *c = opaque;
    if (!c || !c->read_pause) {
        return 0;
    }
    return c->read_pause(c->opaque, pause);
}

static int64_t adaptive_read_seek(void *opaque,
    int stream_index, int64_t timestamp, int flags)
{
    AdaptivePlaylistContext *c = opaque;
    if (!c || !c->read_seek) {
        return 0;
    }
    int64_t ret = c->read_seek(c->opaque, stream_index, timestamp, flags);
    reset_download_info(&c->info);
    return ret;
}

static int adaptive_to_app_message(
    AVFormatContext *s, int type, void *data, size_t data_size)
{
    if (!s->control_message_cb) {
        return AVERROR(ENOSYS);
    }
    return s->control_message_cb(s, type, data, data_size);
}

static void adaptive_playlist_io_restore(
    AdaptivePlaylistContext *c, AVIOContext **pb)
{
    REPLACE_AND_SAVE(NULL, (*pb)->seek          , c->seek          );
    REPLACE_AND_SAVE(NULL, (*pb)->opaque        , c->opaque        );
    REPLACE_AND_SAVE(NULL, (*pb)->av_class      , c->av_class      );
    REPLACE_AND_SAVE(NULL, (*pb)->read_seek     , c->read_seek     );
    REPLACE_AND_SAVE(NULL, (*pb)->read_pause    , c->read_pause    );
    REPLACE_AND_SAVE(NULL, (*pb)->read_packet   , c->read_packet   );
    REPLACE_AND_SAVE(NULL, (*pb)->write_packet  , c->write_packet  );
    REPLACE_AND_SAVE(NULL, (*pb)->short_seek_get, c->short_seek_get);
}

/* Other adaptive related fucntion */
#define MAX_URL_FIELD_LEN       1024
#define INITIAL_BUFFER_SIZE    32768

struct urls_ctx {
    AVIOContext *s;
    unsigned int buffer_pos;
};
struct urls_info {
    char audio[MAX_URL_FIELD_LEN];
    char video[MAX_URL_FIELD_LEN];
    char subtitle[MAX_URL_FIELD_LEN];
    char type[8];
};

static void handle_urls_args(
    struct urls_info *info, const char *key,
    int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "AUDIO=", key_len)) {
        *dest     =        info->audio;
        *dest_len = sizeof(info->audio);
    } else if (!strncmp(key, "VIDEO=", key_len)) {
        *dest     =        info->video;
        *dest_len = sizeof(info->video);
    } else if (!strncmp(key, "SUBTITLES=", key_len)) {
        *dest     =        info->subtitle;
        *dest_len = sizeof(info->subtitle);
    } else if (!strncmp(key, "TYPE=", key_len)) {
        *dest     =        info->type;
        *dest_len = sizeof(info->type);
    }
}

static int64_t seperate_media_read_seek(void *opaque, int64_t offset, int whence)
{
    struct urls_ctx *ctx = opaque;
    AVIOContext *s = ctx->s;

    if (AVSEEK_SIZE == whence) {
        return s->orig_buffer_size;
    }

    if (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END)
        return AVERROR(EINVAL);

    int64_t newoffset = 0;
    if (whence == SEEK_CUR) {
        newoffset = ctx->buffer_pos + offset;
    } else if (whence == SEEK_SET) {
        newoffset = offset;
    } else if (whence == SEEK_END) {
        newoffset = (int64_t) s->orig_buffer_size - offset;
    }
    if (newoffset <= 0) {
        newoffset = 0;
    } else if (newoffset > s->orig_buffer_size) {
        newoffset = s->orig_buffer_size;
    }
    ctx->buffer_pos = newoffset;
    return ctx->buffer_pos;
}

static int seperate_media_read_data(void *opaque, uint8_t *buf, int buf_size)
{
    struct urls_ctx *ctx = opaque;
    AVIOContext *s = ctx->s;

    buf_size = FFMIN(buf_size,
        (s->orig_buffer_size - ctx->buffer_pos));
    if (buf_size <= 0) {
        return AVERROR_EOF;
    }
    memcpy(buf, &s->buffer[ctx->buffer_pos], buf_size);
    ctx->buffer_pos += buf_size;
    return buf_size;
}

static int seperate_media_gen_playlist(
    AVIOContext *pb, const char *url)
{
    int url_num = 0;
    avio_printf(pb, "%s\n", url);
    return 0;
}

int ff_adaptive_open_seperate_media(AVIOContext **s, const char *filename)
{
    int ret = 0;
    const char *ptr = NULL;
    uint8_t *avio_ctx_buffer = NULL;
    if (!s ||
        !av_strstart(filename, SEPERATE_URL_PREFIX, &ptr)) {
        return AVERROR_INVALIDDATA;
    }

    struct urls_info urls = {0};
    ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_urls_args, &urls);

    if (!urls.audio[0] && !urls.video[0]) {
        return AVERROR(EINVAL);
    }

    avio_ctx_buffer = av_malloc(INITIAL_BUFFER_SIZE);
    if (!avio_ctx_buffer) {
        return AVERROR(ENOMEM);
    }

    struct urls_ctx *opaque =
        av_mallocz(sizeof(struct urls_ctx));
    if (!opaque) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    AVIOContext *pb = avio_alloc_context(
        avio_ctx_buffer, INITIAL_BUFFER_SIZE, 1, NULL, NULL, NULL, NULL);
    if (!pb) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }
    (void) seperate_media_gen_playlist(pb, filename);
    unsigned int avio_ctx_buffer_data_bytes = pb->buf_ptr - pb->buffer;
    ffio_init_context(pb, avio_ctx_buffer, avio_ctx_buffer_data_bytes,
        0, opaque, seperate_media_read_data, NULL, seperate_media_read_seek);
    av_log(pb, AV_LOG_INFO,
        "Sperate meida aud:%s vid:%s sub:%s type:%s pls:%s\n",
        urls.audio, urls.video, urls.subtitle, urls.type, avio_ctx_buffer);
    *s = opaque->s = pb;
    return 0;
fail:
    av_freep(&opaque);
    av_freep(&avio_ctx_buffer);
    return ret;
}
/* Other adaptive related fucntion End */

void ff_adaptive_async_open(AVFormatContext *parent)
{
    (void) adaptive_to_app_message(
        parent, AV_ADAPTIVE_QUERY_ASYNC_OPEN, NULL, 0);
}

void ff_adaptive_async_done(AVFormatContext *parent)
{
    (void) adaptive_to_app_message(
        parent, AV_ADAPTIVE_QUERY_ASYNC_DONE, NULL, 0);
}

int ff_adaptive_query_play_state(AVFormatContext *parent)
{
    int state = 0;
    (void) adaptive_to_app_message(parent,
        AV_ADAPTIVE_QUERY_PLAY_STATE, &state, sizeof(state));
    return state;
}

void ff_adaptive_report_server_error(
    AVFormatContext *s,
    int orign_server_error_code, int last_ret)
{
    if (!orign_server_error_code || !s->control_message_cb  || last_ret > 0 ||
        AVERROR_EXIT == last_ret || AVERROR_EOF == last_ret || AVERROR_INPUT_CHANGED == last_ret) {
        return;
    }

    (void) s->control_message_cb(s,
        AV_ADAPTIVE_STREAMING_SERVER_ERROR,
        &orign_server_error_code, sizeof(orign_server_error_code));
}

void ff_adaptive_set_next_input_status(
    AdaptiveInputContext *c, ADAPTIVE_INPUT_NEXT_STATUS_E status)
{
    c->input_next_requested = status;
}

volatile ADAPTIVE_INPUT_NEXT_STATUS_E ff_adaptive_get_next_input_status(AdaptiveInputContext *c)
{
    return c->input_next_requested;
}

/* reload playlist or open next playlist need aysnc finish */
int ff_adaptive_read_data_wait_async(
    AdaptiveInputContext *c, AVIOInterruptCB *cb)
{
    ADAPTIVE_INPUT_NEXT_STATUS_E state =
        ff_adaptive_get_next_input_status(c);
    if (ADAPTIVE_INPUT_NEXT_ASYNC    != state &&
        ADAPTIVE_INPUT_NEXT_ASYNCING != state) {
        return 0;
    }

    while (ADAPTIVE_INPUT_NEXT_NONE  != ff_adaptive_get_next_input_status(c) &&
           ADAPTIVE_INPUT_NEXT_READY != ff_adaptive_get_next_input_status(c)) {
        if (ff_check_interrupt(cb)) {
            return AVERROR_EXIT;
        }
        av_log(NULL, AV_LOG_VERBOSE, "Waitting next input ready!\n");
        av_usleep(10);
    }

    if (ADAPTIVE_INPUT_NEXT_READY ==
        ff_adaptive_get_next_input_status(c)) {
        return 0;
    }

    return AVERROR_HTTP_NOT_FOUND;
}

static void reset_adaptive_input_context(
    AVFormatContext *parent, AdaptiveInputContext *c)
{
    if (c->input_next) {
        if (!parent) {
            av_log(parent, AV_LOG_ERROR, "Reset adaptive input ctx error\n");
        } else {
            ff_format_io_close(parent, &c->input_next);
            c->input_next = NULL;
        }
    }

    c->next_fragment_size   = -1;
    c->next_fragment_seq_no = -1;
}

void ff_adaptive_reinit_input_context(
    AVFormatContext *parent, AdaptiveInputContext *c)
{
    if (!c) {
        av_log(parent, AV_LOG_ERROR, "Init next input context fail\n");
        return;
    }

    reset_adaptive_input_context(parent, c);
}

int ff_adaptive_async_init_ctx(AVFormatContext *parent,
    AdaptiveAsync *async, void *(*dispatcher_thread)(void *))
{
    int ret = 0;
    if (!async->use_thread) {
        return AVERROR(ENOSYS);
    }

    ff_mutex_init(&async->manifest_lock, NULL);
    ret = pthread_create(&async->dispatcher_task, NULL, dispatcher_thread, parent);
    /* error occus */
    if (ret) {
        av_log(parent, AV_LOG_ERROR, "Create manifest updates thread fail:%d\n", ret);
    }
    return ret;
}

void ff_adaptive_async_deinit_ctx(AdaptiveAsync *async)
{
    int ret = 0;
    unsigned char cmpbuf[sizeof(pthread_t)] = {0};

    async->stage = ADAPTIVE_STAGE_CLOSE;
    if (!async->use_thread) {
        return;
    }
    if (memcmp(&async->init_thread, cmpbuf, sizeof(pthread_t))) {
        av_log(NULL, AV_LOG_DEBUG, "Start join init thread\n");
        pthread_join(async->init_thread, NULL);
        av_log(NULL, AV_LOG_DEBUG, "Join init thread done\n");
    }

    if (memcmp(&async->dispatcher_task, cmpbuf, sizeof(pthread_t))) {
        av_log(NULL, AV_LOG_DEBUG, "Start join dispatcher thread\n");
        pthread_join(async->dispatcher_task, NULL);
        av_log(NULL, AV_LOG_DEBUG, "Join dispatcher thread done\n");
    }

    ff_mutex_destroy(&async->manifest_lock);
}

int ff_adaptive_playlist_io_init(
    AVFormatContext *parent, AdaptivePlaylistContext *c, AVIOContext *pb)
{
    if (!parent || !c || !pb) {
        av_log(parent, AV_LOG_TRACE, "Adaptive Demux Context init io fail\n");
        return AVERROR(EINVAL);
    }

    if (!s_avio_class) {
        s_avio_class = pb->av_class;
    }

    av_log(parent, AV_LOG_DEBUG,
        "[c:%p pb:%p]Adaptive Demux Context init io, pb op:%p c op:%p\n",c, pb, pb->opaque,c->opaque);
    REPLACE_AND_SAVE(adaptive_seek        ,    c->seek          , pb->seek          );
    REPLACE_AND_SAVE((void *) c           ,    c->opaque        , pb->opaque        );
    REPLACE_AND_SAVE(&ff_adatpive_class   ,    c->av_class      , pb->av_class      );
    REPLACE_AND_SAVE(adaptive_read_seek   ,    c->read_seek     , pb->read_seek     );
    REPLACE_AND_SAVE(adaptive_read_pause  ,    c->read_pause    , pb->read_pause    );
    REPLACE_AND_SAVE(adaptive_read_packet ,    c->read_packet   , pb->read_packet   );
    REPLACE_AND_SAVE(adaptive_write_packet,    c->write_packet  , pb->write_packet  );
    REPLACE_AND_SAVE(adaptive_short_seek  ,    c->short_seek_get, pb->short_seek_get);

	return 0;
}

void ff_adaptive_playlist_io_restore(
    AdaptivePlaylistContext *c, AVIOContext **pb)
{
    if (!pb || !*pb) {
        return;
    }

    /* c->opaque == NULL when no need io_init for adaptive header read */
    if (c && c->opaque) {
        adaptive_playlist_io_restore(c, pb);
    }
}

void ff_adaptive_playlist_io_deinit(
    AVFormatContext *parent, AdaptivePlaylistContext *c, AVIOContext **pb)
{
    if (!parent || !pb || !*pb) {
        av_log(parent, AV_LOG_TRACE, "[%p]Adaptive Demux Context deinit io already\n", pb);
        return;
    }

    /* c->opaque == NULL when no need io_init for adaptive header read */
    if (c && c->opaque) {
        adaptive_playlist_io_restore(c, pb);
    }

    av_log(parent, AV_LOG_VERBOSE, "[%p][%p] Adaptive Demux Context deinit io\n", pb, *pb);
    ff_format_io_close(parent, pb);
}

void ff_adaptive_playlist_seek_flush(AdaptivePlaylistContext *c)
{
    if (!c) {
        return;
    }

    c->format.frag.current = -1;
    reset_download_info(&c->info);
}

/* use information sucha as fps from MPD or m3u8 playlist */
void ff_adaptive_playlist_check_stream(
    AdaptivePlaylistContext *c, AVRational *fps)
{
    if (!c || !(c->ctx) || !(c->ctx->streams)) {
        return;
    }

    AVFormatContext *s = c->ctx;
    for (unsigned int i = 0; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];
        if (!st || !st->codecpar ||
            AVMEDIA_TYPE_VIDEO != st->codecpar->codec_type) {
            continue;
        }
        AdaptiveFormat *format = &c->format;
        if (format->check) {
            (void) format->check(c, st);
        }

        if (fps && fps->den > 0 && fps->num > 0) {
            check_stream_framerate(s, fps, st);
        }
    }
}

int ff_adaptive_playlist_advance_fragment(AVFormatContext *parent,
    AdaptivePlaylistContext *c, AVPacket *pkt, int is_restart_needed, int64_t *sync_timestamp)
{
    int ret = 0;
    if (sync_timestamp) {
        *sync_timestamp = 0;
    }
    if (!c) {
        av_log(parent, AV_LOG_TRACE, "Adaptive Demux Context advance fragment fail\n");
        return 0;
    }

    AdaptiveFormat *format = &c->format;
    if (!is_restart_needed && !format->advance) {
        return 0;
    }
    AVAdaptive *adaptive = &parent->adaptive;
    int update_bitrate = ADAPTIVE_ADVANCE_NONE;
    if (is_restart_needed) {
        update_bitrate = ADAPTIVE_ADVANCE_NORMAL;
    } else if (format->advance) {
        update_bitrate = format->advance(c, pkt, sync_timestamp);
    }
    if (ADAPTIVE_ADVANCE_NONE == update_bitrate) {
        return 0;
    }

    int64_t bitrate = 0;
    if (AV_ADAPTIVE_RESOLUTION_MODE_AUTO ==
        adaptive->adaptive_resolution_mode) {
        bitrate = download_finish(parent, c);
        if (bitrate > 0) {
            av_log(parent, AV_LOG_INFO,
                "Expose Bitrate:%lld\n", bitrate);
            ret = adaptive_to_app_message(parent,
                    AV_ADAPTIVE_BITRATE_UPDATED, &bitrate, sizeof(bitrate));
            if (ret > 0 && ADAPTIVE_ADVANCE_DROP_PACKET == update_bitrate) {
                av_packet_unref(pkt);
            }
        }
        (void) download_start(c);
    } else {
        ret = adaptive_to_app_message(parent,
                AV_ADAPTIVE_BITRATE_UPDATED, &bitrate, sizeof(bitrate));
        if (ret > 0 && ADAPTIVE_ADVANCE_DROP_PACKET == update_bitrate) {
            av_packet_unref(pkt);
        }
    }
    return (ret <= 0 ? 0 : 1);
}

void ff_adaptive_playlist_context_open(
    AVFormatContext *s, AdaptivePlaylistContext *c, int n_fragments)
{
    if (!s || !c) {
        av_log(s, AV_LOG_TRACE, "Adaptive Demux Context open fail\n");
        return;
    }
    c->ctx = s;
    c->format.check   = NULL;
    c->format.advance = NULL;
    c->format.release = NULL;
    if (s->iformat && s->iformat->long_name) {
        const char *name = s->iformat->long_name;
        if (strstr(name, "QuickTime / MOV")) {
            c->format.check   = mp4_check_stream;
            c->format.advance = (1 == n_fragments) ? mp4_advance_fragment : NULL;
            c->format.release = mp4_release_format;
        } else if (strstr(name, "Matroska / WebM") || strstr(name, "WebM DASH Manifest")) {
            c->format.check   = web_check_stream;
            c->format.advance = (1 == n_fragments) ? webm_advance_fragment : NULL;
            c->format.release = webm_release_format;
        }
        c->format.frag.current = -1;
    }
}

void ff_adaptive_playlist_context_close(AdaptivePlaylistContext *c)
{
    if (!c) {
        av_log(NULL, AV_LOG_TRACE, "Adaptive Demux Context close fail\n");
        return;
    }
    c->ctx = NULL;
}
/* Dash: called when parse_manifest_representation
 * HLS : called after open stream because media type cannot be determined before.
*/
AdaptivePlaylistContext *ff_adaptive_playlist_context_new(
    AVFormatContext *parent, enum AVMediaType type, int is_live)
{
    if (!parent || AVMEDIA_TYPE_VIDEO != type) {
        return NULL;
    }

    AdaptivePlaylistContext *c =
        av_mallocz(sizeof(AdaptivePlaylistContext));
    if (!c) {
        av_log(NULL, AV_LOG_ERROR, "Alloc Adaptive Demux Context fail\n");
        return NULL;
    }

#ifdef ADAPTIVE_USE_THREAD
    ff_mutex_init(&c->mutex, NULL);
#endif
    c->is_live = is_live ? 1 : 0;
    c->format.frag.current = -1;
    (void) adaptive_to_app_message(
        parent, AV_ADAPTIVE_LIVE_MEDIA_UPDATE, &(c->is_live), sizeof(int));
    reset_download_info(&c->info);
    return c;
}

void ff_adaptive_playlist_context_free(AdaptivePlaylistContext **c)
{
    if (!c || !*c) {
        return;
    }
#ifdef ADAPTIVE_USE_THREAD
    ff_mutex_destroy(&((*c)->mutex));
#endif
    AdaptiveFormat *format = &((*c)->format);
    if (format->release) {
        format->release(*c);
    }

    av_freep(c);
}

int ff_adaptive_list_init(AVFormatContext *parent, AVAdaptiveList *list)
{
    if (!parent || !list || AVMEDIA_TYPE_VIDEO != list->type) {
        return AVERROR(EINVAL);
    }

    AVAdaptive *adaptive = &parent->adaptive;
    adaptive->lists = av_realloc_array(adaptive->lists,
        adaptive->n_list_num + 1, sizeof(*adaptive->lists));
    if (!adaptive->lists) {
        av_log(parent, AV_LOG_ERROR, "Alloc adaptive lists fail\n");
        return AVERROR(ENOMEM);
    }
    AVAdaptiveList *l =
        &adaptive->lists[adaptive->n_list_num++];

    *l = *list;
    l->priority    = 0;
    l->penalty_cnt = 0;
    // dump_expose_adaptive_information(parent);
    if (adaptive->n_list_num < 2) {
        return 0;
    }

    qsort(adaptive->lists, adaptive->n_list_num, sizeof(AVAdaptiveList), compare_bandwith);
    return 0;
}

void ff_adaptive_list_free(AVFormatContext *parent)
{
    if (!parent) {
        return;
    }

    free_adaptive(&parent->adaptive);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
