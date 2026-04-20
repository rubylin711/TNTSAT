/*
 * Montage Technology (Shanghai) Co., Ltd.
 * Montage Proprietary and Confidential
 * Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies
 *
 * Description:stream_misc file
 * History:     Date        Author    Modification
 *   1.       2020-12-29   ChenZhimou      Create
 */
#define MODULE_TAG "STREAM MISC"

#include "stream_misc.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdint.h>
#include "stream.h"
#include "av_helpers.h"
#include "mt_module_debug.h"
#include "libavformat/avio.h"
#include "libavformat/avformat.h"

#if MT_DES("Internal Subroutine Definition", 1)
static mt_u32 stream_is_seekable(struct AVInputFormat *ifmt)
{
    mt_u32 seekable =
        !((NULL == ifmt->read_seek) && (NULL == ifmt->read_seek2));

    return seekable;
}

static int get_stream_type(struct AVInputFormat *ifmt, char *fname)
{
    mt_u32 stream_type =
        stream_is_seekable(ifmt) ? STREAMTYPE_FILE : STREAMTYPE_STREAM;

    stream_type |= stream_is_rtp_type(fname);
    stream_type |= stream_is_udp_type(fname);
    stream_type |= stream_is_http_type(fname);
    stream_type |= stream_is_rtsp_type(fname);
    stream_type |= stream_is_rtmp_type(fname);
    stream_type |= stream_is_srt_type(fname);

    return (stream_type ? STREAMTYPE_STREAM : STREAMTYPE_FILE);
}

static int ctrl_get_stream_size(
    AVFormatContext *fmt_ctx, int cmd, void *arg)
{
    int64_t size = 0;

    MT_LOGI("get stream size: %lld\n", size);
    if (size >= 0) {
        *(off_t *)arg = size;
        return STREAM_OK;
    }

    return STREAM_UNSUPPORTED;
}

static int ctrl_seek_to_time(
    AVFormatContext *fmt_ctx, int cmd, void *arg)
{
    int ret;
    int64_t ts;
    double pts;
    AVInputFormat *ifmt = fmt_ctx->iformat;

	pts = *(double *)arg;
	ts  = (int64_t)(pts * AV_TIME_BASE);
    MT_LOGI("read seek to time, pts:%lfs, ts:%lldus\n", pts, ts);
	if (ifmt && !ifmt->read_seek && !ifmt->read_seek2) {
        MT_LOGE("read seek without hook\n", pts, ts);
        return STREAM_UNSUPPORTED;
	}

    if (ifmt && !stream_is_seekable(ifmt)) {
        MT_LOGE("Stream not seekable\n");
        return STREAM_UNSUPPORTED;
    }

    ret = avformat_seek_file(fmt_ctx, -1, INT64_MIN, ts, INT64_MAX, 0);
    MT_LOGI("read seek to time %s!\n", (ret >= 0) ? "ok" : "fail");
    return ((ret >= 0) ? STREAM_OK : STREAM_UNSUPPORTED);
}

/* from source stream */
static int fill_buffer_from_stream(
    AVIOContext *io_ctx, char *buffer, int max_len)
{
    return 0;
    // return avio_read(io_ctx, buffer, max_len);
}

/* to destination stream */
static int write_buffer_to_stream(
    AVIOContext *io_ctx, char *buffer, int len)
{
    avio_write(io_ctx, buffer, len);
    avio_flush(io_ctx);

    return ((io_ctx->error) ? -1 : len);
}

#endif

#if MT_DES("Internal API Definition", 1)
static int stream_misc_fill_buffer(stream_t *s, char *buffer, int max_len)
{
    mt_u32 para_err;
    int read_bytes = 0;
    AVFormatContext *fmt_ctx = NULL;

    MT_LOGD("[%p] fill buffer start, buf:%p need:%d\n", s, buffer, max_len);
    para_err = (NULL == s) || (NULL == s->priv) || (NULL == buffer) || (0 >= max_len);
    if (para_err) {
        MT_LOGF("[%p] fill buffer para error, buf:%p, need:%d\n", s, buffer, max_len);
        return MT_FAILURE;
    }

    fmt_ctx = (AVFormatContext *)s->priv;
    if (fmt_ctx->pb) {
        /* Not read stream, because ffmpeg has it's own way to use stream.
         * if the unseekable stream is readed and used by other demux,
         * the stream header may miss */
        read_bytes = fill_buffer_from_stream(fmt_ctx->pb, buffer, max_len);
    }

    MT_LOGD("[%p] fill buffer done, read bytes:%d\n", s, read_bytes);
    return read_bytes;
}

static int stream_misc_write_buffer(stream_t *s, char *buffer, int len)
{
    mt_u32 para_err;
    int          write_bytes = len;
    AVInputFormat   *ifmt    = NULL;
    AVFormatContext *fmt_ctx = NULL;

    MT_LOGD("[%p] write buffer start, buf:%p len:%d\n", s, buffer, len);
    para_err = (NULL == s) || (NULL == s->priv) || (NULL == buffer) || (0 >= len);
    if (para_err) {
        MT_LOGF("[%p] write buffer para error, buf:%p len:%d\n", s, buffer, len);
        return MT_FAILURE;
    }

    fmt_ctx = (AVFormatContext *) s->priv;
    ifmt  = fmt_ctx->iformat;
    if (NULL == ifmt) {
        MT_LOGF("[%p] write buffer input format error\n", s);
        return MT_FAILURE;
    }

    if (fmt_ctx->pb) {
        write_bytes = write_buffer_to_stream(fmt_ctx->pb, buffer, len);
    }

    MT_LOGD("[%p] write buffer done, write bytes:%d\n", s, write_bytes);
    return len;
}

/* stream seek, used by mplayer type probe
 * when new_demuxer and demux_open_lavf(rewind stream).
 */
static int stream_misc_seek(stream_t *s, off_t newpos)
{
    int ret;
    mt_u32 para_err;
    AVInputFormat   *ifmt    = NULL;
    AVFormatContext *fmt_ctx = NULL;

    MT_LOGD("[%p] seek start, seek pos:%lld\n", s, newpos);
    para_err = (NULL == s) || (NULL == s->priv);
    if (para_err) {
        MT_LOGF("[%p] seek para error\n", s);
        return MT_FAILURE;
    }

    fmt_ctx = (AVFormatContext *) s->priv;
    ifmt    = fmt_ctx->iformat;
    if (NULL == ifmt) {
        MT_LOGF("[%p] seek input format error\n", s);
        return MT_FAILURE;
    }

    if (!stream_is_seekable(ifmt)) {
        MT_LOGE("[%p] stream not seekable\n", s);
        return STREAM_OK;
    }

    ret = avformat_seek_file(
        fmt_ctx, -1, INT64_MIN, newpos, INT64_MAX, AVSEEK_FLAG_BYTE);
    if (ret < 0) {
        s->eof = 1;
        MT_LOGE("[%p] seek fail, stream eof\n", s);
        return STREAM_UNSUPPORTED;
    }

    MT_LOGD("[%p] seek done\n", s);
    return STREAM_OK;
}

static int ctrl_get_source_type(
    AVFormatContext *fmt_ctx, int cmd, void *arg)
{
    *(unsigned int *) arg = STREAM_SOURCE_TYPE_FFMPEG_MISC;
    return STREAM_OK;
}

static int stream_misc_control(stream_t *s, int cmd, void *arg)
{
    int idx;
    mt_u32 para_err;
    AVInputFormat   *ifmt    = NULL;
    AVFormatContext *fmt_ctx = NULL;
    struct {
        int cmd;
        int (*func)(AVFormatContext *, int, void *);
    } list[] = {
        {STREAM_CTRL_GET_SIZE       , ctrl_get_stream_size},
        {STREAM_CTRL_SEEK_TO_TIME   , ctrl_seek_to_time   },
        {STREAM_CTRL_GET_SOURCE_TYPE, ctrl_get_source_type},
    };

    MT_LOGI("[%p] stream control, cmd:%d, arg:%p\n", s, cmd, arg);
    para_err = (NULL == s) || (NULL == s->priv);
    if (para_err) {
        MT_LOGF("[%p] stream control para error\n", s);
        return MT_FAILURE;
    }

    fmt_ctx = (AVFormatContext *) s->priv;
    ifmt    = fmt_ctx->iformat;
    if (NULL == ifmt) {
        MT_LOGF("[%p] stream control input format error\n", s);
        return MT_FAILURE;
    }

    for (idx = 0; idx < ARRAY_CNT(list); idx++) {
        if (cmd == list[idx].cmd) {
            return list[idx].func(fmt_ctx, cmd, arg);
        }
    }

    return STREAM_UNSUPPORTED;
}

static void stream_misc_close(stream_t *stream)
{
    if (NULL == stream || NULL == stream->priv) {
        MT_LOGF("[%p] close stream para illegal\n", stream);
    }

    MT_LOGI("[%p] close stream\n", stream);
    stream->priv = NULL;
    /* closed in demux_close_lavf */
    /* avformat_close_input(&fmt_ctx); */
}

static int stream_misc_open(stream_t *stream, int mode, void *opts, int *file_format)
{
    AVInputFormat   *ifmt    = NULL;
    AVFormatContext *fmt_ctx = NULL;

    (void) file_format;
    init_avformat();
    if (NULL == stream || NULL == stream->url) {
        MT_LOGF("[%p] open stream para illegal\n", stream);
    }

    AVIOInterruptCB *options = ((stream_io_options_t *) opts)->interrupt_cb;
    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx) {
        MT_LOGF("[%p] Could not allocate context.\n", stream);
        return MT_FAILURE;
    }

    fmt_ctx->interrupt_callback.callback = options->callback;
    fmt_ctx->interrupt_callback.opaque = options->opaque;
    /* open the input file */
    if (MT_SUCCESS !=
        avformat_open_input(&fmt_ctx, stream->url, NULL, NULL)) {
        MT_LOGE("[%p] Cannot open input file %s\n", stream, stream->url);
        return MT_FAILURE;
    }

    ifmt               = fmt_ctx->iformat;
    /*
    * Do NOT set AVIOContext pb if AVFMT_NOFILE flag is set,
    * In such a case, the muxer will handle I/O in some other way.
    */
    stream->priv       = (void *) fmt_ctx;
    stream->end_pos    = 0;
    stream->is_chunked = 0;

    stream->type         = get_stream_type(ifmt, stream->url);
    stream->seek         = stream_misc_seek;
    stream->close        = stream_misc_close;
    stream->control      = stream_misc_control;
    stream->fill_buffer  = stream_misc_fill_buffer;
    stream->write_buffer = stream_misc_write_buffer;

    MT_LOGI("[%p] open stream scucess, fmt ctx:%p, ifmt:%p, pb:%p input file:%s\n",
        stream, fmt_ctx, ifmt, fmt_ctx->pb, stream->url);
    return STREAM_OK;
}

#endif

#if MT_DES("External Definition", 1)
/* we initialize a null stream to match new ffmpeg follow,
 * which not use av io stream directly in some case
 */
const stream_info_t stream_info_misc = {
    "FFmpeg Misc",
    "ffmpeg misc",
    "",
    "",
    stream_misc_open,
    {"misc", "rtp", "udp", "rtsp", "rtmp", NULL},
    NULL,
    0
};
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

