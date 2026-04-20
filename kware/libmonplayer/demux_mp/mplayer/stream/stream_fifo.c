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
#define MODULE_TAG "FP STREAM FIFO"
#include "config.h"
#include "stream.h"
#include "mt_type.h"
#include "mlog.h"
#include "mutil.h"
#include "libavutil/mem.h"
#include "file_playback_sequence.h"

#define FIFO_IDENTIFICATION_PREFIX         "fifo"

static int fifo_fill_buffer(stream_t *s, char *buffer, int max_len)
{
    FILE_PLAYBACK_EXTIO_CONTEXT_T *ctx = s->priv;
    if (!ctx || !ctx->read) {
        MLOGE("[%p] Not support Read\n", ctx);
        return -1;
    }

    int r = ctx->read(ctx->opaque, buffer, max_len);
    if (r <= 0) {
        return -1;
    }
    return r;
}

static int fifo_write_buffer(stream_t *s, char *buffer, int len)
{
    (void) s;
    (void ) buffer;
    return len;
}

static int fifo_seek(stream_t *s, off_t newpos)
{
    FILE_PLAYBACK_EXTIO_CONTEXT_T *ctx = s->priv;
    if (!ctx || !ctx->seek || !ctx->get_seekable ||
        (ctx->get_seekable && !ctx->get_seekable(ctx->opaque))) {
        MLOGD("[%p] Not support seek\n", ctx);
        return 0;
    }

    if (ctx->seek (ctx->opaque, newpos, SEEK_SET) < 0) {
        s->eof = 1;
        return 0;
    }

    s->pos = newpos;
    return 1;
}

static int fifo_control(stream_t *s, int cmd, void *arg)
{
    int64_t size, ts;
    double pts;
    FILE_PLAYBACK_EXTIO_CONTEXT_T *ctx = s->priv;

    if (!ctx) {
        return STREAM_UNSUPPORTED;
    }
    MLOGD("stream fifo control, cmd:%d\n", cmd);
    if (STREAM_CTRL_GET_SIZE == cmd) {
        if (!ctx->get_size) {
            return STREAM_UNSUPPORTED;
        }
        int64_t size = ctx->get_size (ctx->opaque);
        if (size >= 0) {
            *(off_t *) arg = size;
            MLOGD("stream fifo get size:%lld,return STREAM_OK\n", size);
            return STREAM_OK;
        }
    } else if (STREAM_CTRL_GET_SOURCE_TYPE == cmd) {
        *(unsigned int *) arg = STREAM_SOURCE_TYPE_EXTERNAL_IO;
        return STREAM_OK;
    }
    return STREAM_UNSUPPORTED;
}

static void fifo_close_f(stream_t *stream)
{
    if (!stream->priv) {
        return;
    }

    FILE_PLAYBACK_EXTIO_CONTEXT_T *ctx = stream->priv;
    if (ctx->deinit) {
        ctx->deinit (ctx->opaque);
    }
    av_freep(&stream->priv);
}

static int fifo_open_f_normal_open(
    FILE_PLAYBACK_EXTIO_CONTEXT_T **ctx, void *opts)
{
    int ret = STREAM_OK;
    FILE_PLAYBACK_EXTIO_CONTEXT_T *extio =
        av_mallocz(sizeof(FILE_PLAYBACK_EXTIO_CONTEXT_T));
    if (!extio) {
        return STREAM_UNSUPPORTED;
    }
    *ctx   = extio;
    *extio = *(FILE_PLAYBACK_EXTIO_CONTEXT_T *) opts;
    return ret;
}

static int fifo_open_f_normal(stream_t *stream,
    int mode, void *opts, int *file_format)
{
    FILE_PLAYBACK_EXTIO_CONTEXT_T *ctx = NULL;
    if (strncmp(stream->url,
        FIFO_IDENTIFICATION_PREFIX,
        strlen(FIFO_IDENTIFICATION_PREFIX))) {
        return STREAM_UNSUPPORTED;
    }

    int ret = fifo_open_f_normal_open(&ctx, opts);
    if (STREAM_OK != ret) {
        return STREAM_UNSUPPORTED;
    }

    stream->priv = ctx;
    if (ctx->get_size) {
        int64_t size = ctx->get_size (ctx->opaque);
        if (size >= 0) {
            stream->end_pos = size;
        }
    }

    stream->seek = fifo_seek;
    stream->type = STREAMTYPE_FILE;
    if (!(ctx->get_seekable) || !(ctx->get_seekable(ctx->opaque))) {
        stream->type = STREAMTYPE_STREAM;
        stream->seek = NULL;
    }
    return STREAM_OK;
}

static int fifo_open_f(stream_t *stream, int mode, void *opts, int *file_format)
{
    int ret = fifo_open_f_normal(stream, mode, opts, file_format);
    if (STREAM_OK == ret) {
        stream->close        = fifo_close_f;
        stream->control      = fifo_control;
        stream->fill_buffer  = fifo_fill_buffer;
        stream->write_buffer = fifo_write_buffer;
    }

    MLOGI("%sUse external I/O\n", (STREAM_OK == ret) ? "" : "NOT ");
    return ret;
}

const stream_info_t stream_info_fifo = {
    "FIFO",
    "fifo",
    "Montage",
    "For external user defined i/o context",
    fifo_open_f,
    {FIFO_IDENTIFICATION_PREFIX, NULL},
    NULL,
    1 // Urls are an option string
};
