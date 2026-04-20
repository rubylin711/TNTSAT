/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __AVFORMAT_ADAPTIVE_DEMUX_H_H__
#define __AVFORMAT_ADAPTIVE_DEMUX_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include "url.h"
#include "isom.h"
#include "avio.h"
#include "avformat.h"
#include "libavutil/thread.h"
#include "libavutil/avstring.h"

#define NUM_LOOKBACK_FRAGMENTS       3
// #define ADAPTIVE_USE_THREAD    1
#define SLOW_CLOCK_UPDATE_INTERVAL  (1000000 * 30 * 60) /* 30 minutes */
#define FAST_CLOCK_UPDATE_INTERVAL  (1000000 * 30)      /* 30 seconds */
#define SEPERATE_URL_PREFIX         "mtsurl://"

struct AdaptivePlaylistContext;

typedef enum ADAPTIVE_STAGE {
    ADAPTIVE_STAGE_NONE             = 0,
    ADAPTIVE_STAGE_INIT                ,
    ADAPTIVE_STAGE_ASYNC_READ_HEADER   ,
    ADAPTIVE_STAGE_EXPOSE_STREAM       ,
    ADAPTIVE_STAGE_ALL_STREAM_READY    ,
    ADAPTIVE_STAGE_INIT_FAILED         ,
    ADAPTIVE_STAGE_CLOSE               ,
} ADAPTIVE_STAGE_E;

typedef enum ADAPTIVE_INPUT_NEXT_STATUS {
    ADAPTIVE_INPUT_NEXT_NONE        = 0, /* No need open next ahead of time */
    ADAPTIVE_INPUT_NEXT_NEED           , /* Open next ahead of time when read data */
    ADAPTIVE_INPUT_NEXT_ASYNC          , /* Open next ahead of time in async thread */
    ADAPTIVE_INPUT_NEXT_ASYNCING       , /* Open next ahead of time on going */
    ADAPTIVE_INPUT_NEXT_READY          , /* Next input is ready */
} ADAPTIVE_INPUT_NEXT_STATUS_E;

typedef struct AdaptiveAsync {
    int is_live;
    int use_thread;
    ADAPTIVE_STAGE_E stage;
    pthread_t init_thread;
    pthread_t dispatcher_task;
    AVMutex manifest_lock;
    int need_send_async_done;
} AdaptiveAsync;

typedef struct AdaptiveFragment {
    int current;
    int nb_index_entries;
    AVIndexEntry *index_entries;
} AdaptiveFragment;

typedef struct AdaptiveFormat {
    AdaptiveFragment frag;
    int (*check)  (struct AdaptivePlaylistContext *, AVStream *st);
    int (*advance)(struct AdaptivePlaylistContext *, AVPacket *, int64_t *);
    int (*release)(struct AdaptivePlaylistContext *);
} AdaptiveFormat;

typedef struct PlaylistDownloadInfo {
    /* unit us */
    int64_t download_start_time;
    int64_t download_total_bytes;
    int64_t current_download_rate;

    int64_t fragment_bytes_downloaded;
    int64_t last_bitrate;
    int64_t  last_latency;
    /* unit us */
    int64_t sum_download_time;
    int64_t last_download_time;

    /* Average for the last fragments */
    int64_t moving_bitrate;
    int32_t moving_index;
    int64_t fragment_bitrates[NUM_LOOKBACK_FRAGMENTS];
} PlaylistDownloadInfo;

typedef struct AdaptivePlaylistContext {
    const AVClass *av_class;
    void *opaque;
    AVFormatContext *ctx;
    int (*read_packet) (void *opaque, uint8_t *buf, int buf_size);
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
    int64_t (*seek) (void *opaque, int64_t offset, int whence);

    int (*read_pause)(void *opaque, int pause);
    int64_t (*read_seek)(void *opaque, int stream_index, int64_t timestamp, int flags);
    int (*short_seek_get)(void *opaque);

#ifdef ADAPTIVE_USE_THREAD
    AVMutex mutex;
#endif
    int is_live;
    AdaptiveFormat format;
    PlaylistDownloadInfo info;
} AdaptivePlaylistContext;

typedef struct AdaptiveInputContext {
    int64_t next_fragment_size;
    int64_t next_fragment_seq_no;
    AVIOContext *input_next;
    pthread_t input_next_thread;
    volatile ADAPTIVE_INPUT_NEXT_STATUS_E input_next_requested;
} AdaptiveInputContext;

/* need lock when manifest update */
#define MANIFEST_GET_LOCK(c) (&(c->async.manifest_lock))
#define MANIFEST_LOCK(c)                                               \
do {                                                                   \
    av_log(NULL, AV_LOG_TRACE,"[M] Locking from %d\n", __LINE__);      \
    if (c->use_thread) {                                               \
        ff_mutex_lock (MANIFEST_GET_LOCK (c));                         \
    }                                                                  \
} while (0)

#define MANIFEST_UNLOCK(c)                                             \
do {                                                                   \
    av_log(NULL, AV_LOG_TRACE, "[M] Unlocking from %d\n", __LINE__);   \
    if (c->use_thread) {                                               \
        ff_mutex_unlock (MANIFEST_GET_LOCK (c));                       \
    }                                                                  \
} while (0)

/* need lock when playlist update */
#define PLAYLIST_GET_LOCK(pls) (&(pls->mutex))
#define PLAYLIST_LOCK(c, pls)                                          \
do {                                                                   \
    av_log(NULL, AV_LOG_TRACE,"PLS Locking from %d\n", __LINE__);      \
    if (c->use_thread) {                                               \
        ff_mutex_lock (PLAYLIST_GET_LOCK (pls));                       \
    }                                                                  \
} while (0)

#define PLAYLIST_UNLOCK(c, pls)                                        \
do {                                                                   \
    av_log(NULL, AV_LOG_TRACE, "PLS Unlocking from %d\n", __LINE__);   \
    if (c->use_thread) {                                               \
        ff_mutex_unlock (PLAYLIST_GET_LOCK (pls));                     \
    }                                                                  \
} while (0)

int ff_adaptive_open_seperate_media(AVIOContext **s, const char *filename);

void ff_adaptive_async_open(AVFormatContext *parent);
void ff_adaptive_async_done(AVFormatContext *parent);
void ff_adaptive_report_server_error(
    AVFormatContext *s,
    int orign_server_error_code, int last_ret);
int ff_adaptive_query_play_state(AVFormatContext *parent);

void ff_adaptive_set_next_input_status(
    AdaptiveInputContext *c, ADAPTIVE_INPUT_NEXT_STATUS_E status);
volatile ADAPTIVE_INPUT_NEXT_STATUS_E ff_adaptive_get_next_input_status(AdaptiveInputContext *c);
void ff_adaptive_reinit_input_context(AVFormatContext *parent, AdaptiveInputContext *c);
int ff_adaptive_read_data_wait_async(
    AdaptiveInputContext *c, AVIOInterruptCB *cb);

int  ff_adaptive_async_init_ctx(AVFormatContext *parent,
        AdaptiveAsync *async, void *(*dispatcher_thread)(void *));
void ff_adaptive_async_deinit_ctx(AdaptiveAsync *async);

void ff_adaptive_playlist_seek_flush(AdaptivePlaylistContext *c);
void ff_adaptive_playlist_check_stream(
    AdaptivePlaylistContext *c, AVRational *fps);
int ff_adaptive_playlist_advance_fragment(AVFormatContext *parent,
    AdaptivePlaylistContext *c, AVPacket *pkt, int is_restart_needed, int64_t *sync_timestamp);

void ff_adaptive_playlist_context_open(
    AVFormatContext *s, AdaptivePlaylistContext *c, int n_fragments);
void ff_adaptive_playlist_context_close(AdaptivePlaylistContext *c);

int ff_adaptive_playlist_io_init(
    AVFormatContext *parent, AdaptivePlaylistContext *c, AVIOContext *pb);
void ff_adaptive_playlist_io_restore(
    AdaptivePlaylistContext *c, AVIOContext **pb);
void ff_adaptive_playlist_io_deinit(
    AVFormatContext *parent, AdaptivePlaylistContext *c, AVIOContext **pb);

/* parent is context from it's parent */
AdaptivePlaylistContext *ff_adaptive_playlist_context_new(
    AVFormatContext *parent, enum AVMediaType type, int is_live);
void ff_adaptive_playlist_context_free(AdaptivePlaylistContext **c);

/* s is context from itself */
int ff_adaptive_list_init(AVFormatContext *parent, AVAdaptiveList *list);
void ff_adaptive_list_free(AVFormatContext *parent);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __AVFORMAT_ADAPTIVE_DEMUX_H_H__ */
