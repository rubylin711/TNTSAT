/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "avformat.h"
#include "internal.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/internal.h"
#include "avio_internal.h"
#include "adaptive.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*
 * Each playlist has its own demuxer. If it currently is active,
 * it has an open AVIOContext too, and potentially an AVPacket
 * containing the next packet from this stream.
 */
struct mtsurl_playlist {
    char url[MAX_URL_SIZE];
    AVIOContext pb;
    AVIOContext *input;
    AVFormatContext *parent;
    AVFormatContext *ctx;
    uint8_t *read_buffer;

    /* main demuxer streams associated with this playlist
     * indexed by the subdemuxer stream indexes */
    AVStream **main_streams;
    int n_main_streams;

    enum AVMediaType type;
    int64_t cur_timestamp;
    int64_t start_timestamp;
    int64_t seek_timestamp;
};

typedef struct MTSURLContext {
    AVClass *class;
    AVFormatContext *ctx;

    struct mtsurl_playlist *audio;
    struct mtsurl_playlist *video;
    struct mtsurl_playlist *subtitle;

    int is_live;
    AVPacket *first_packet;
    int64_t first_timestamp;
    AVIOInterruptCB *interrupt_callback;
    char *headers;
    char *user_agent;
    char *cookies;          ///< holds newline (\n) delimited Set-Cookie header field values (without the "Set-Cookie: " field name)
    AVDictionary *avio_opts;
    int64_t qos_net_bitrate;
} MTSURLContext;

/* Other adaptive related fucntion */
#define MAX_URL_FIELD_LEN       2048
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

static int is_private_seperate_file(
    const char *url, const char *name)
{
    if (url && av_match_ext(url, "m3u8,m3u")) {
        return 0;
    }
    if (name) {
        if (!strncmp(name, "hls", 3)) {
            return 0;
        } else if (!strncmp(name, "dash", 4)) {
            return 0;
        }
    }
    return 1;
}

static void free_playlist(
    MTSURLContext *c,
    struct mtsurl_playlist *pls)
{
    if (pls->main_streams) {
        av_freep(&pls->main_streams);
    }
    if (pls->input)
        ff_format_io_close(pls->parent, &pls->input);

    if (pls->ctx) {
        pls->ctx->pb = NULL;
        avformat_close_input(&pls->ctx);
    }
   // if (pls->read_buffer) { // Free by avio
   //     av_freep(&pls->read_buffer);
   // }
}

static struct mtsurl_playlist *new_playlist(
    const char *url, const enum AVMediaType type)
{
    struct mtsurl_playlist *pls =
        av_mallocz(sizeof(struct mtsurl_playlist));
    if (!pls) {
        return NULL;
    }
    pls->type = type;
    pls->cur_timestamp   = AV_NOPTS_VALUE;
    pls->start_timestamp = AV_NOPTS_VALUE;
    pls->seek_timestamp  = AV_NOPTS_VALUE;
    ff_make_absolute_url(pls->url, MAX_URL_SIZE, NULL, url);
    return pls;
}

static int mtsurl_close(AVFormatContext *s)
{
    MTSURLContext *c = s->priv_data;
    if (c->video) {
        free_playlist(c, c->video);
        av_freep(&c->video);
    }
    if (c->audio) {
        free_playlist(c, c->audio);
        av_freep(&c->audio);
    }
    if (c->subtitle) {
        free_playlist(c, c->subtitle);
        av_freep(&c->subtitle);
    }
    if (c->first_packet) {
        av_packet_free(&c->first_packet);
        c->first_packet = NULL;
    }
    av_dict_free(&c->avio_opts);
    return 0;
}

static int open_input(struct mtsurl_playlist *pls, AVIOContext **in)
{
    int ret;
    AVFormatContext *s = pls->parent;
    MTSURLContext *c = s->priv_data;
    AVDictionary *tmp = NULL;

    av_dict_copy(&tmp, c->avio_opts, 0);
    ret = s->io_open(s, in, pls->url, AVIO_FLAG_READ, &tmp);

    av_dict_free(&tmp);
    return ret;
}

static int read_data_l(
    struct mtsurl_playlist *pls,
    uint8_t *buf, int buf_size)
{
    int ret = 0;
    if (!pls->input) {
        ret = open_input(pls, &pls->input);
        if (ret < 0) {
            return ret;
        }
    }
    ret = avio_read(pls->input, buf, buf_size);
    return ret;
}

static int read_data(void *opaque, uint8_t *buf, int buf_size)
{
    int ret = 0;
    struct mtsurl_playlist *pls = opaque;

    if (!buf || buf_size <= 0) {
        return 0;
    }

    ret = read_data_l(pls, buf, buf_size);
    if (pls->input && AVMEDIA_TYPE_VIDEO == pls->type) {
        int64_t qos_net_bitrate = 0;
        if (av_opt_get_int(pls->input,
            "qos_net_bitrate", AV_OPT_SEARCH_CHILDREN, &qos_net_bitrate) >= 0) {
            AVFormatContext *s = pls->parent;
            MTSURLContext *c = s->priv_data;
            c->qos_net_bitrate = qos_net_bitrate;
        }
    }
    if (ret < 0 && AVERROR_EXIT != ret && AVERROR_EOF != ret) {
        ff_adaptive_report_server_error(pls->parent, ret, ret);
    }
    return ret;
}

static int64_t seek_data(void *opaque, int64_t offset, int whence)
{
    struct mtsurl_playlist *v = opaque;
    if (!v->input) {
        return AVERROR(ENOSYS);
    }

    if (v->input->seek && AVSEEK_SIZE == whence) {
        return v->input->seek(v->input->opaque, offset, whence);
    }
    int64_t ret = avio_seek(v->input, offset, whence);
    return ret;
}

static int save_avio_options(AVFormatContext *s)
{
    MTSURLContext *c = s->priv_data;
    const char *opts[] = {
        "headers", "user_agent", "cookies", "http_proxy", "referer", "rw_timeout", NULL };
    const char **opt = opts;
    uint8_t *buf = NULL;
    int ret = 0;

    while (*opt) {
        if (av_opt_get(s, *opt, AV_OPT_SEARCH_CHILDREN, &buf) >= 0) {
            if (buf[0] != '\0') {
                ret = av_dict_set(&c->avio_opts, *opt, buf, AV_DICT_DONT_STRDUP_VAL);
                if (ret < 0) {
                    av_freep(&buf);
                    return ret;
                }
            } else {
                av_freep(&buf);
            }
        }
        opt++;
    }

    return ret;
}

static int nested_io_open(AVFormatContext *s, AVIOContext **pb, const char *url,
                          int flags, AVDictionary **opts)
{
    av_log(s, AV_LOG_ERROR,
           "A MTSURL playlist item '%s' referred to an external file '%s'. "
           "Opening this file was forbidden for security reasons\n", s->url, url);
    return AVERROR(EPERM);
}

static void read_packet_update_info(
    MTSURLContext *c,
    struct mtsurl_playlist *pls, AVPacket *pkt)
{
    if (AV_NOPTS_VALUE != pkt->dts) {
        AVStream *st = pls->ctx->streams[pkt->stream_index];
        pls->cur_timestamp =
            av_rescale_q(pkt->dts, st->time_base, AV_TIME_BASE_Q);
        if (AV_NOPTS_VALUE == pls->start_timestamp) {
            pls->start_timestamp = pls->cur_timestamp;
        }
        if (AV_NOPTS_VALUE == c->first_timestamp) {
            c->first_timestamp = pls->cur_timestamp;
        }
    }

    pkt->stream_index = pls->main_streams[pkt->stream_index]->index;
}

static int read_packet_by_av_read(
    MTSURLContext *c,
    struct mtsurl_playlist *pls, AVPacket *pkt)
{
    int ret = 0;
    while (!ff_check_interrupt(c->interrupt_callback) && !ret) {
        ret = av_read_frame(pls->ctx, pkt);
        av_log(c->ctx, AV_LOG_TRACE, "[%d] pts:%lld, size:%d ret:%d\n", pkt->stream_index, pkt->pts, pkt->size, ret);
        if (ret < 0) {
            return AVERROR_EOF;
        }
        /* If we got a packet, return it */
        read_packet_update_info(c, pls, pkt);
        if (AV_NOPTS_VALUE != pls->seek_timestamp) {
            if (AV_NOPTS_VALUE == pkt->pts) {
                pls->seek_timestamp = AV_NOPTS_VALUE;
                return 0;
            }

            AVStream *st = c->ctx->streams[pkt->stream_index];
            int64_t cur_pts = av_rescale_q(
                pkt->pts, st->time_base, AV_TIME_BASE_Q);
            /* drop large pts pkt, 40ms */
            if (cur_pts + 40000 < pls->seek_timestamp) {
                av_packet_unref(pkt);
                continue;
            }
            pls->seek_timestamp = AV_NOPTS_VALUE;
        }
        return 0;
    }
    return AVERROR_EOF;
}

static int read_seek_internal(MTSURLContext *c,
    struct mtsurl_playlist *pls, int64_t *timestamp, int flags)
{
    if (!pls || !pls->ctx || !pls->ctx->iformat) {
        return 0;
    }

    ff_read_frame_flush(pls->ctx);
    pls->cur_timestamp  = AV_NOPTS_VALUE;
    pls->seek_timestamp = AV_NOPTS_VALUE;
    int seekable = pls->pb.seekable;

    if (is_private_seperate_file(pls->url, pls->ctx->iformat->name)) {
        pls->pb.seekable = 1;
    }
    pls->ctx->iformat->flags |= AVFMT_NOGENSEARCH;
    int ret = av_seek_frame(pls->ctx, -1, *timestamp, AVSEEK_FLAG_BACKWARD);
    pls->pb.seekable = seekable;
    if (ret < 0 ) {
        return ret;
    }

    if (AVMEDIA_TYPE_VIDEO != pls->type) {
        pls->seek_timestamp = *timestamp;
        return 0;
    }

    av_packet_unref(c->first_packet);
    ret = read_packet_by_av_read(c, pls, c->first_packet);
    if (ret < 0) {
        return ret;
    }
    AVPacket *pkt = c->first_packet;
    AVStream *st = c->ctx->streams[pkt->stream_index];
    *timestamp =
        av_rescale_q(pkt->pts, st->time_base, AV_TIME_BASE_Q);
    return ret;
}

static int mtsurl_read_seek(AVFormatContext *s, int stream_index,
                               int64_t timestamp, int flags)
{
    MTSURLContext *c = s->priv_data;

    if (c->is_live || (flags & AVSEEK_FLAG_BYTE) || (c->ctx->ctx_flags & AVFMTCTX_UNSEEKABLE))
        return AVERROR(ENOSYS);

    int ret = 0;
    AVStream *st = NULL;
    int64_t first_timestamp, seek_timestamp, duration;

    seek_timestamp =
        av_rescale_rnd(timestamp, AV_TIME_BASE,
            s->streams[stream_index]->time_base.den,
            flags & AVSEEK_FLAG_BACKWARD ? AV_ROUND_DOWN : AV_ROUND_UP);
    if (c->video && AVMEDIA_TYPE_VIDEO !=
        s->streams[stream_index]->codecpar->codec_type) {
        for (int i = 0; i < c->video->n_main_streams; i++) {
            st = c->video->main_streams[i];
            if (st->discard < AVDISCARD_ALL &&
                AVMEDIA_TYPE_VIDEO == st->codecpar->codec_type) {
                stream_index = st->index;
                break;
            }
        }
    }

    first_timestamp = c->first_timestamp == AV_NOPTS_VALUE ? 0 : c->first_timestamp;
    duration = s->duration == AV_NOPTS_VALUE ? 0 : s->duration;
    if (0 < duration && duration < seek_timestamp - first_timestamp)
        return AVERROR(EIO);

    ret = read_seek_internal(c, c->video, &seek_timestamp, flags);
    if (ret < 0) {
        return ret;
    }
    ret = read_seek_internal(c, c->audio, &seek_timestamp, flags);
    if (ret < 0) {
        return ret;
    }
    ret = read_seek_internal(c, c->subtitle, &seek_timestamp, flags);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

static int playlist_timestamp_samll(
    struct mtsurl_playlist *cur, struct mtsurl_playlist *us)
{
    if (!cur || !us) {
        return 0;
    }
    return (cur->cur_timestamp - cur->start_timestamp) > (us->cur_timestamp - us->start_timestamp);
}

static int mtsurl_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    MTSURLContext *c = s->priv_data;
    int ret = 0;

    if (c->first_packet && c->first_packet->data) {
        av_packet_move_ref(pkt, c->first_packet);
        return 0;
    }

    struct mtsurl_playlist *cur = c->video;
    if (c->audio && (!cur || playlist_timestamp_samll(cur, c->audio))) {
        cur = c->audio;
    } else if (c->subtitle && (!cur || playlist_timestamp_samll(cur, c->subtitle))) {
        cur = c->subtitle;
    }

    return read_packet_by_av_read(c, cur, pkt);
}

/* add new subdemuxer streams to our context, if any */
static int expose_streams_from_subdemuxer(
    AVFormatContext *s, struct mtsurl_playlist *pls, int type)
{
    if (!pls || !pls->ctx) {
        return 0;
    }
    while (pls->n_main_streams < (int) pls->ctx->nb_streams) {
        int ist_idx = pls->n_main_streams;
        AVStream *st = avformat_new_stream(s, NULL);
        AVStream *ist = pls->ctx->streams[ist_idx];

        if (!st)
            return AVERROR(ENOMEM);

        st->id = type;
        dynarray_add(&pls->main_streams, &pls->n_main_streams, st);

        av_program_add_stream_index(s, 0, st->index);
        int err = avcodec_parameters_copy(st->codecpar, ist->codecpar);
        if (err < 0)
            return err;
        /* update internal codec context, for the parser */
        err = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
        if (err < 0)
            return err;
        st->avg_frame_rate = st->r_frame_rate = ist->avg_frame_rate;
        avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
    }
    if (pls->ctx && AV_NOPTS_VALUE != pls->ctx->duration) {
        if (AV_NOPTS_VALUE == s->duration ||
            pls->ctx->duration > s->duration) {
            s->duration = pls->ctx->duration;
        }
    }

    return 0;
}

static int read_header_open_demux(
    MTSURLContext *c, const enum AVMediaType type,
    const char *url, struct mtsurl_playlist **playlist)
{
    if (url && !url[0]) {
        return 0;
    }

    int ret = 0;
    ff_const59 AVInputFormat *in_fmt = NULL;

    struct mtsurl_playlist *pls = new_playlist(url, type);
    if (!pls)
        return AVERROR(ENOMEM);

    *playlist = pls;
    pls->parent = c->ctx;
    if (!(pls->ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    pls->read_buffer = av_malloc(INITIAL_BUFFER_SIZE);
    if (!pls->read_buffer){
        ret = AVERROR(ENOMEM);
        avformat_free_context(pls->ctx);
        pls->ctx = NULL;
        goto fail;
    }
    if (c->is_live) {
        ffio_init_context(&pls->pb, pls->read_buffer, INITIAL_BUFFER_SIZE, 0, pls, read_data, NULL, NULL);
    } else {
        ffio_init_context(&pls->pb, pls->read_buffer, INITIAL_BUFFER_SIZE, 0, pls, read_data, NULL, seek_data);
    }

    pls->pb.seekable = 0;
    ret = av_probe_input_buffer(&pls->pb, &in_fmt, pls->url, NULL, 0, 0);
    if (ret < 0) {
        /* Free the ctx - it isn't initialized properly at this point,
         * so avformat_close_input shouldn't be called. If
         * avformat_open_input fails below, it frees and zeros the
         * context, so it doesn't need any special treatment like this. */
        av_log(c->ctx, AV_LOG_ERROR, "Error when loading first segment '%s' reason:%s\n", pls->url, av_err2str(ret));
        avformat_free_context(pls->ctx);
        pls->ctx = NULL;
        goto fail;
    }

    pls->ctx->pb       = &pls->pb;
    //pls->ctx->io_open  = nested_io_open;
    pls->ctx->flags   |= c->ctx->flags & ~AVFMT_FLAG_CUSTOM_IO;
    ret = avformat_open_input(&pls->ctx, pls->url, in_fmt, NULL);
    if (ret < 0)
        goto fail;

    ret = avformat_find_stream_info(pls->ctx, NULL);
    if (ret < 0)
        goto fail;
fail:
    return ret;
}

static int mtsurl_read_header(AVFormatContext *s)
{
    MTSURLContext *c = s->priv_data;
    int ret = 0;
    const char *ptr = NULL;
    char line[MAX_URL_SIZE << 2] = {0};

    c->ctx                = s;
    c->interrupt_callback = &s->interrupt_callback;
    c->first_timestamp    = AV_NOPTS_VALUE;
    ff_get_chomp_line(s->pb, line, sizeof(line));

    if ((ret = save_avio_options(s)) < 0)
        goto fail;

    if (!av_strstart(line, SEPERATE_URL_PREFIX, &ptr)) {
        return AVERROR_INVALIDDATA;
    }

    struct urls_info urls = {0};
    ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_urls_args, &urls);

    if (!urls.audio[0] && !urls.video[0]) {
        return AVERROR(EINVAL);
    }
    if (urls.type[0] && !strncmp(urls.type, "live", 4)) {
        c->is_live = 1;
    }
    c->first_packet = av_packet_alloc();
    if (!c->first_packet) {
        return AVERROR(ENOMEM);
    }

    ret = read_header_open_demux(c, AVMEDIA_TYPE_VIDEO, urls.video, &c->video);
    if (ret < 0)
        goto fail;
    ret = read_header_open_demux(c, AVMEDIA_TYPE_AUDIO, urls.audio, &c->audio);
    if (ret < 0)
        goto fail;
    (void) read_header_open_demux(c, AVMEDIA_TYPE_SUBTITLE, urls.subtitle, &c->subtitle);

    AVProgram *program = av_new_program(s, 0);
    if (!program)
        goto fail;

    ret = expose_streams_from_subdemuxer(s, c->video, 0);
    if (ret < 0)
        goto fail;
    ret = expose_streams_from_subdemuxer(s, c->audio, 1);
    if (ret < 0)
        goto fail;
    (void) expose_streams_from_subdemuxer(s, c->subtitle, 2);
    return 0;
fail:
    mtsurl_close(s);
    return ret;
}

static int mtsurl_probe(const AVProbeData *p)
{
    /* user defined sperate urls */
    if (strncmp(p->buf, SEPERATE_URL_PREFIX, strlen(SEPERATE_URL_PREFIX))) {
        return 0;
    }

    return AVPROBE_SCORE_MAX;
}

#define OFFSET(x) offsetof(MTSURLContext, x)
#define FLAGS AV_OPT_FLAG_DECODING_PARAM
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
static const AVOption mtsurl_options[] = {
    { "headers", "set custom HTTP headers, can override built in default headers", OFFSET(headers), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    { "user_agent", "override User-Agent header", OFFSET(user_agent), AV_OPT_TYPE_STRING, { .str = "Lavf/58.29.100" }, 0, 0, D },
    { "cookies", "set cookies to be sent in applicable future requests, use newline delimited Set-Cookie HTTP field value syntax", OFFSET(cookies), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D },
    { "qos_net_bitrate", "bitrate of network for qos",
        OFFSET(qos_net_bitrate), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, FLAGS},
    {NULL}
};

static const AVClass mtsurl_class = {
    .class_name = "mtsurl demuxer",
    .item_name  = av_default_item_name,
    .option     = mtsurl_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_mtsurl_demuxer = {
    .name           = "mtsurl",
    .long_name      = NULL_IF_CONFIG_SMALL("Montage multiple seperate url streaming"),
    .priv_class     = &mtsurl_class,
    .priv_data_size = sizeof(MTSURLContext),
    .flags          = AVFMT_NOGENSEARCH,
    .read_probe     = mtsurl_probe,
    .read_header    = mtsurl_read_header,
    .read_packet    = mtsurl_read_packet,
    .read_close     = mtsurl_close,
    .read_seek      = mtsurl_read_seek,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
