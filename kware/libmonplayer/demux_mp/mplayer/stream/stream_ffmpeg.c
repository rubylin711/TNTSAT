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
#define MODULE_TAG "FP STREAM FFMEPG"
#include "config.h"
#include "stream.h"
#include "av_helpers.h"
#include "mt_type.h"
#include "mlog.h"
#include "mutil.h"
#include "libavutil/opt.h"
#include "libavformat/avio.h"
#include "libavutil/avstring.h"
#include "libavformat/avformat.h"

#if defined(CFG_SMART_HTTP_PTOTOCOL)
#include "libavformat/smart_http.h"
#endif

#if MT_DES("Support Seperate av media", 1)
#define SEPERATE_URL_PREFIX   "mtsurl://"
#endif

#define URL_PROTOCOL_FLAG_NETWORK_SMART_HTTP       4 /*should be the same value as ffmpeg used*/
static int fill_buffer(stream_t *s, char *buffer, int max_len)
{
    int r = avio_read(s->priv, buffer, max_len);
    if (r <= 0) {
        return -1;
    } else {
        return r;
    }
}

static int write_buffer(stream_t *s, char *buffer, int len)
{
    AVIOContext *ctx = s->priv;
    avio_write(s->priv, buffer, len);
    avio_flush(s->priv);

    if (ctx->error) {
        return -1;
    }

    return len;
}

static int seek(stream_t *s, off_t newpos)
{
    s->pos = newpos;
    if (avio_seek(s->priv, s->pos, SEEK_SET) < 0) {
        s->eof = 1;
        return 0;
    }

    return 1;
}

static int control(stream_t *s, int cmd, void *arg)
{
    AVIOContext *ctx = s->priv;
    int64_t size, ts;
    double pts;

    MLOGD("[%s] stream_ffmpeg control ~~~~~~~~\n", __func__);
    switch (cmd) {
        case STREAM_CTRL_GET_SIZE: {
            size = avio_size(s->priv);
            if (size >= 0) {
                *(off_t *)arg = size;
                MLOGD("[%s] stream_ffmpeg get size: %lld ,return 1\n", __func__, size);
                return STREAM_OK;
            }
            break;
        }
        case STREAM_CTRL_GET_SOURCE_TYPE: {
            *(unsigned int *) arg = STREAM_SOURCE_TYPE_FFMPEG;
            return STREAM_OK;
        }
        case STREAM_CTRL_SEEK_TO_TIME: {
            pts = *(double *)arg;
            ts = pts * AV_TIME_BASE;

            if (!ctx->read_seek) {
                break;
            }

            ts = avio_seek_time(ctx, -1, ts, 0);
            if (ts >= 0) {
                return 1;
            }
            break;
        }
    }
    return STREAM_UNSUPPORTED;
}

static void close_f(stream_t *stream)
{
    AVIOContext *s = stream->priv;
#ifdef CFG_ENABLE_FFMPEG_422
    /* Not seperate url */
    if (strncmp(stream->url, SEPERATE_URL_PREFIX, strlen(SEPERATE_URL_PREFIX))) {
        avio_close(s);
        return;
    }

    av_opt_free(s);
    av_freep(&s->opaque);
    av_freep(&s->buffer);
    avio_context_free(&s);
#else
    avio_close(s);
#endif
}

static int open_f_normal_open(AVIOContext **s,
    const char *filename, int flags, void *opts, AVDictionary **options)
{
    int ret = 0;
    AVIOInterruptCB *int_cb = opts;
#ifdef CFG_ENABLE_FFMPEG_422
    AVDictionary *tmp = NULL;
    av_dict_copy(&tmp, *options, 0);
    ret = avio_open2(s, filename, flags, int_cb, &tmp);
    av_dict_free(&tmp);
#else
    ret = avio_open2m(s, filename, flags, int_cb, options);
#endif
    return ret;
}

static int open_f_normal(stream_t *stream,
    int mode, void *opts, int *file_format)
{
    int64_t size;
    const char *filename;
    int flags = 0;
    AVIOContext *ctx = NULL;
    int res = STREAM_ERROR;

    if (mode == STREAM_READ) {
        flags = AVIO_FLAG_READ;
    } else if (mode == STREAM_WRITE) {
        flags = AVIO_FLAG_WRITE;
    } else {
        MLOGE("[ffmpeg] Unknown open mode %d\n", mode);
        res = STREAM_UNSUPPORTED;
        goto out;
    }

#ifdef AVIO_FLAG_DIRECT
    flags |= AVIO_FLAG_DIRECT;
#endif

    if (stream->url) {
        filename = stream->url;
    } else {
        MLOGE("[ffmpeg] No URL\n");
        goto out;
    }

    if (!strncmp(filename, "ffmpeg://", strlen("ffmpeg://"))) {
        filename += strlen("ffmpeg://");
    }

#if defined(CFG_SMART_HTTP_PTOTOCOL)
    if (Smart_Http_Is_Enable() && (strstr(filename, "http://") || strstr(filename, "https://") || strstr(filename, "srt://"))) {
        flags |= URL_PROTOCOL_FLAG_NETWORK_SMART_HTTP;
    }
#endif
    stream_io_options_t *iopara = (stream_io_options_t *) opts;
    if (open_f_normal_open(&ctx, filename, flags,
            iopara->interrupt_cb, (AVDictionary **) iopara->ff_dict_opts) < 0) {
        goto out;
    }

    stream->priv = ctx;
    size = avio_size(ctx);
    stream->is_chunked = ctx->is_chunked;
    stream->ff_dict_opts = iopara->ff_dict_opts;
    if (size >= 0) {
        stream->end_pos = size;
    }

    stream->type = STREAMTYPE_FILE;
    stream->seek = seek;
    if (!ctx->seekable) {
        stream->type = STREAMTYPE_STREAM;
    }

    if ((strncmp(filename, "http:", 5) == 0) || (strncmp(filename, "rtsp:", 5) == 0) || (strncmp(filename, "rtmp:", 5) == 0) || (strncmp(filename, "https:", 6) == 0)) {
        stream->type = STREAMTYPE_STREAM;
    }
    if ((strncmp(filename, "srt:", 4) == 0)) {
        stream->type = STREAMTYPE_STREAM;
    }

    if (strstr(filename, "avi")) {
        stream->type = STREAMTYPE_FILE;
    }
    res = STREAM_OK;
out:
    return res;
}

static int open_f(stream_t *stream, int mode, void *opts, int *file_format)
{
    int ret = 0;

    init_avformat();
    ret = open_f_normal(stream, mode, opts, file_format);
    if (STREAM_OK == ret) {
        stream->close        = close_f;
        stream->control      = control;
        stream->fill_buffer  = fill_buffer;
        stream->write_buffer = write_buffer;
    }

    MLOGI("%s stream->type:%d ret:%d\n", __func__, stream->type, ret);
    return ret;
}

const stream_info_t stream_info_ffmpeg = {
    "FFmpeg",
    "ffmpeg",
    "",
    "",
    open_f,
    //yliu add
    { "ffmpeg", "http", "https", "srt", "mtsurl", NULL },
    NULL,
    1 // Urls are an option string
};
