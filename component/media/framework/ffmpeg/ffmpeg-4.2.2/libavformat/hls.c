/*
 * Apple HTTP Live Streaming demuxer
 * Copyright (c) 2010 Martin Storsjo
 * Copyright (c) 2013 Anssi Hannula
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

/**
 * @file
 * Apple HTTP Live Streaming demuxer
 * http://tools.ietf.org/html/draft-pantos-http-live-streaming
 */

#include "libavformat/http.h"
#include "libavutil/avstring.h"
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/time.h"
#include "avformat.h"
#include "internal.h"
#include "avio_internal.h"
#include "id3v2.h"
#include "adaptive.h"
#include "decryption_comp.h"

#define INITIAL_BUFFER_SIZE 32768
#define VIDEO_CHOOSED_BANDWIDTH 1000000

#define MAX_FIELD_LEN 64
#define MAX_CHARACTERISTICS_LEN 512
#define PLS_PARSE_ASYNC  (-2)
#define PLS_OPEN_DEMUX   (-1)
#define PLS_NEED_NO      (0)
#define PLS_NEED_YES     (1)
#define PLS_NEED_CHANGE  (2)

#define MPEG_TIME_BASE 90000
#define MPEG_TIME_BASE_Q (AVRational){1, MPEG_TIME_BASE}

/* hlsdemux must not get closer to the end of a live stream than
 * M3U8_LIVE_MIN_FRAGMENT_DISTANCE fragments. Section 6.3.3
 *  "Playing the Playlist file" of the HLS draft states that this
 *  value is three fragments
 */
#define M3U8_LIVE_MIN_FRAGMENT_DISTANCE 3

/*
 * An apple http stream consists of a playlist with media segment files,
 * played sequentially. There may be several playlists with the same
 * video content, in different bandwidth variants, that are played in
 * parallel (preferably only one bandwidth variant at a time). In this case,
 * the user supplied the url to a main playlist that only lists the variant
 * playlists.
 *
 * If the main playlist doesn't point at any variants, we still create
 * one anonymous toplevel variant for this, to maintain the structure.
 */

enum KeyType {
    KEY_NONE,
    KEY_AES_128,
    KEY_SAMPLE_AES,
    KEY_CHINA_DRM_SM4T,/* china drm with SM4-CTR */
    KEY_CHINA_DRM_SM4R,/* china drm with SAMPLE-SM4-CTR */
    KEY_CHINA_DRM_SM4C,/* china drm with SM4-CBC */
    KEY_CHINA_DRM_SM4S,/* china drm with SAMPLE-SM4 */
};

struct segment {
    int64_t duration;
    int64_t url_offset;
    int64_t size;
    char *url;
    char *key;
    enum KeyType key_type;
    uint8_t iv[16];
    /* associated Media Initialization Section, treated as a segment */
    struct segment *init_section;
};

struct rendition;

enum PlaylistType {
    PLS_TYPE_UNSPECIFIED,
    PLS_TYPE_EVENT,
    PLS_TYPE_VOD
};

/*
 * Each playlist has its own demuxer. If it currently is active,
 * it has an open AVIOContext too, and potentially an AVPacket
 * containing the next packet from this stream.
 */
struct playlist {
    char url[MAX_URL_SIZE];
    AVIOContext pb;
    uint8_t* read_buffer;
    AVIOContext *input;
    int input_read_done;
    AVIOContext *input_next;
    /* [-1]need async open, [0] need open [1]opened */
    int input_next_requested;
    AVFormatContext *parent;
    int index;
    AVFormatContext *ctx;
    AVPacket pkt;
    int has_noheader_flag;

    /* main demuxer streams associated with this playlist
     * indexed by the subdemuxer stream indexes */
    AVStream **main_streams;
    int n_main_streams;

    int finished;
    enum PlaylistType type;
    /* Set when update_streams_new_adaptive */
    enum AVMediaType media_type;
    int64_t target_duration;
    int64_t start_seq_no;
    int time_offset_flag;
    int64_t start_time_offset;
    int n_segments;
    struct segment **segments;
    /* [-2]need parse playlist
     * [-1]need expose header
     * [0]not need, [1]need
     */
    int needed;
    int broken;
    int64_t cur_seq_no;
    int64_t last_seq_no;
    int m3u8_discontinuity;
    int m3u8_hold_counters;
    int64_t cur_seg_offset;
    int64_t last_load_time;

    /* Currently active Media Initialization Section */
    struct segment *cur_init_section;
    uint8_t *init_sec_buf;
    unsigned int init_sec_buf_size;
    unsigned int init_sec_data_len;
    unsigned int init_sec_buf_read_offset;

    char key_url[MAX_URL_SIZE];
    uint8_t key[16];

    /* ID3 timestamp handling (elementary audio streams have ID3 timestamps
     * (and possibly other ID3 tags) in the beginning of each segment) */
    int is_id3_timestamped; /* -1: not yet known */
    int64_t id3_mpegts_timestamp; /* in mpegts tb */
    int64_t id3_offset; /* in stream original tb */
    uint8_t* id3_buf; /* temp buffer for id3 parsing */
    unsigned int id3_buf_size;
    AVDictionary *id3_initial; /* data from first id3 tag */
    int id3_found; /* ID3 tag found at some point */
    int id3_changed; /* ID3 tag data has changed at some point */
    ID3v2ExtraMeta *id3_deferred_extra; /* stored here until subdemuxer is opened */

    int64_t seek_timestamp;
    int seek_flags;
    int seek_stream_index; /* into subdemuxer stream array */

    /* Renditions associated with this playlist, if any.
     * Alternative rendition playlists have a single rendition associated
     * with them, and variant main Media Playlists may have
     * multiple (playlist-less) renditions associated with them. */
    int n_renditions;
    struct rendition **renditions;

    /* Media Initialization Sections (EXT-X-MAP) associated with this
     * playlist, if any. */
    int n_init_sections;
    struct segment **init_sections;
    int width;
    int height;
    int bandwidth;
    AVRational framerate;
    AVMutex mutex;
    enum DRM_SYSTEM_TYPE drm_sys_type;
    AdaptivePlaylistContext *adaptive;
    AVEncryptionInitInfo *encryption_info;
};

/*
 * Renditions are e.g. alternative subtitle or audio streams.
 * The rendition may either be an external playlist or it may be
 * contained in the main Media Playlist of the variant (in which case
 * playlist is NULL).
 */
struct rendition {
    enum AVMediaType type;
    struct playlist *playlist;
    char group_id[MAX_FIELD_LEN];
    char language[MAX_FIELD_LEN];
    char name[MAX_FIELD_LEN];
    int disposition;
};

struct variant {
    int bandwidth;

    /* every variant contains at least the main Media Playlist in index 0 */
    int n_playlists;
    struct playlist **playlists;

    char audio_group[MAX_FIELD_LEN];
    char video_group[MAX_FIELD_LEN];
    char subtitles_group[MAX_FIELD_LEN];
    char codec[MAX_FIELD_LEN];
    enum AVMediaType media_type;
};

typedef struct HLSContext {
    AVClass *class;
    AVFormatContext *ctx;
    int n_variants;
    struct variant **variants;
    int n_playlists;
    struct playlist **playlists;
    int n_renditions;
    struct rendition **renditions;

    int64_t cur_seq_no;
    int m3u8_hold_counters;
    int live_start_index;

    int use_thread;
    AdaptiveAsync async;
    int64_t highest_cur_seq_no;
    int prefer_x_start;
    int first_packet;
    int64_t first_timestamp;
    int64_t cur_timestamp;
    AVIOInterruptCB *interrupt_callback;
    AVDictionary *avio_opts;
    char *allowed_extensions;
    int max_reload;
    int http_persistent;
    int http_multiple;
    int seg_max_retry;
    AVIOContext *playlist_pb;
    int need_change_playlist;
    /* User decrypt data, not ffmpeg */
    int decrypt_by_user;
    /* No need to demux */
    int demux_by_user;
    int64_t qos_net_bitrate;
} HLSContext;

static int read_packet_av_read(AVFormatContext *s,
    HLSContext *c, struct playlist *pls);

static void close_format_io(
    AVFormatContext *s,
    struct playlist *pls, AVIOContext **pb)
{
    /* adaptive_playlist_io ready after open done and change input */
    AdaptivePlaylistContext *adaptive = pls ? pls->adaptive : NULL;
    /* Replace input, not change input_next */
    ff_adaptive_playlist_io_deinit(s, adaptive, pb);
}

static int is_prior_codec(const char *codec)
{
    if ((!av_strncasecmp(codec, "vp08", 4)) ||
        (!av_strncasecmp(codec, "vp09", 4))) {
        return 0;
    }
    return 1;
}

static int has_video_codec(const char *codec)
{
    if ((!av_strncasecmp(codec, "vp08", 4)) ||
        (!av_strncasecmp(codec, "vp09", 4)) ||
        (!av_strncasecmp(codec, "avc1", 4))) {
        return 1;
    }
    return 0;
}

static int is_private_seperate_file(
    const char *parent_url, const char *us)
{
    /* Not sperate defined usrl */
    if (strncmp(parent_url, SEPERATE_URL_PREFIX, strlen(SEPERATE_URL_PREFIX))) {
        return 0;
    }

    if (us && av_match_ext(us, "m3u8,m3u")) {
        return 0;
    }

    return 1;
}

static int is_short_video(AVFormatContext *s)
{
    HLSContext *c = s->priv_data;
    /* vod >=0.5s and < 60s */
    if (!c->async.is_live &&
        AV_NOPTS_VALUE != s->duration &&
        s->duration > (0.5 * AV_TIME_BASE)  &&
        s->duration < (60  * AV_TIME_BASE)) {
        return 1;
    }
    return 0;
}

static int is_playlist_normal(struct playlist *pls)
{
    if (PLS_NEED_NO  == pls->needed ||
        PLS_NEED_YES == pls->needed) {
        return 1;
    }
    return 0;
}

static void free_segment_dynarray(struct segment **segments, int n_segments)
{
    int i;

    for (i = 0; i < n_segments; i++) {
        if (segments[i]) {
            av_freep(&segments[i]->key);
            av_freep(&segments[i]->url);
            av_freep(&segments[i]);
        }
    }
}

static void move_segments(struct segment **src,
    int n_src, struct segment **dst, int n_dst)
{
    int i_dst, i_src = 0;

    for (i_dst = 0; i_dst < n_dst; i_dst++) {
        for (; i_src < n_src; i_src++) {
            if (src[i_src] && dst[i_dst] &&
                !strcmp(src[i_src]->url, dst[i_dst]->url)) {
                struct segment *tmp = dst[i_dst];
                dst[i_dst] = src[i_src];
                src[i_src] = NULL;
                free_segment_dynarray(&tmp, 1);
                i_src++;
                break;
            }
        }
    }
    for (i_dst = 0; i_dst < n_dst; i_dst++) {
        struct segment *seg = dst[i_dst];
        av_log(NULL, AV_LOG_TRACE, "Update seg:%p url:%s\n", seg, seg->url);
    }
}

static void free_segment_list(struct playlist *pls)
{
    free_segment_dynarray(pls->segments, pls->n_segments);
    av_freep(&pls->segments);
    pls->n_segments = 0;
}

static void free_init_section_list(struct playlist *pls)
{
    int i;
    for (i = 0; i < pls->n_init_sections; i++) {
        av_freep(&pls->init_sections[i]->url);
        av_freep(&pls->init_sections[i]);
    }
    av_freep(&pls->init_sections);
    pls->n_init_sections = 0;
}

static void free_playlist(HLSContext *c, struct playlist *pls)
{
    if (!pls) {
        return;
    }
    free_segment_list(pls);
    free_init_section_list(pls);
    av_freep(&pls->main_streams);
    av_freep(&pls->renditions);
    av_freep(&pls->id3_buf);
    av_dict_free(&pls->id3_initial);
    ff_id3v2_free_extra_meta(&pls->id3_deferred_extra);
    av_freep(&pls->init_sec_buf);
    av_packet_unref(&pls->pkt);
    av_freep(&pls->pb.buffer);
    if (pls->encryption_info) {
        av_encryption_init_info_free(pls->encryption_info);
        pls->encryption_info = NULL;
    }
    if (pls->input)
        close_format_io(c->ctx, pls, &pls->input);
    pls->input_read_done = 0;
    if (pls->input_next)
        close_format_io(c->ctx, NULL, &pls->input_next);
    pls->input_next_requested = 0;
    if (pls->ctx) {
        pls->ctx->pb = NULL;
        avformat_close_input(&pls->ctx);
    }
    ff_mutex_destroy(&pls->mutex);
    ff_adaptive_playlist_context_close(pls->adaptive);
    ff_adaptive_playlist_context_free(&pls->adaptive);
    av_free(pls);
}

static void free_playlist_list(HLSContext *c)
{
    int i;
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        free_playlist(c, pls);
    }
    av_freep(&c->playlists);
    c->n_playlists = 0;
}

static void free_variant_list(HLSContext *c)
{
    int i;
    for (i = 0; i < c->n_variants; i++) {
        struct variant *var = c->variants[i];
        av_freep(&var->playlists);
        av_free(var);
    }
    av_freep(&c->variants);
    c->n_variants = 0;
}

static void free_rendition_list(HLSContext *c)
{
    int i;
    for (i = 0; i < c->n_renditions; i++)
        av_freep(&c->renditions[i]);
    av_freep(&c->renditions);
    c->n_renditions = 0;
}

/*
 * Used to reset a statically allocated AVPacket to a clean state,
 * containing no data.
 */
static void reset_packet(AVPacket *pkt)
{
    av_init_packet(pkt);
    pkt->data = NULL;
}

static struct playlist *new_playlist(HLSContext *c, const char *url,
                                     const char *base)
{
    struct playlist *pls = av_mallocz(sizeof(struct playlist));
    if (!pls)
        return NULL;
    reset_packet(&pls->pkt);
    ff_make_absolute_url(pls->url, sizeof(pls->url), base, url);
    pls->seek_timestamp = AV_NOPTS_VALUE;

    pls->media_type = AVMEDIA_TYPE_UNKNOWN;
    pls->drm_sys_type = DRM_SYSTEM_TYPE_NONE;
    pls->is_id3_timestamped   = -1;
    pls->id3_mpegts_timestamp = AV_NOPTS_VALUE;

    dynarray_add(&c->playlists, &c->n_playlists, pls);
    ff_mutex_init(&pls->mutex, NULL);
    return pls;
}

struct variant_info {
    char bandwidth[20];
    /* variant group ids: */
    char audio[MAX_FIELD_LEN];
    char video[MAX_FIELD_LEN];
    char subtitles[MAX_FIELD_LEN];
    char codec[MAX_FIELD_LEN];
    char resolution[MAX_FIELD_LEN];
    char framerate[MAX_FIELD_LEN];
    char video_range[MAX_FIELD_LEN];
};

static is_digital(const char c)
{
    return c >= '0' && c < '9';
}

static char *get_first_digital(const char *ptr)
{
    while(*ptr && !is_digital(*ptr)) {
        ptr++;
    }
    return ptr;
}

static char *copy_variant_digital( char *dst, char * src)
{
    int num = 0;
    while (*src && is_digital(*src) && num < MAX_FIELD_LEN) {
        *dst++ = *src++;
    }
    return src;
}

static void parse_variant_video_info(
    struct variant_info *info, struct playlist *pls)
{
    char *ptr;
    if (info->framerate[0]) {
        ptr = &info->framerate[0];
        pls->framerate.den = 1;
        pls->framerate.num = atoi(ptr);
    }
    if (info->resolution[0]) {
        char width[MAX_FIELD_LEN] = {0};
        char height[MAX_FIELD_LEN] = {0};
        ptr = &info->resolution[0];
        ptr = get_first_digital(ptr);
        if (!(*ptr)) {
            return;
        }
        ptr = copy_variant_digital(width, ptr);
        (void) copy_variant_digital(
            height, get_first_digital(ptr));
        pls->width  = atoi(width);
        pls->height = atoi(height);
    }
}

static struct variant *new_variant(HLSContext *c, struct variant_info *info,
                                   const char *url, const char *base)
{
    struct variant *var;
    struct playlist *pls;

    pls = new_playlist(c, url, base);
    if (!pls)
        return NULL;

    var = av_mallocz(sizeof(struct variant));
    if (!var)
        return NULL;

    var->media_type = AVMEDIA_TYPE_UNKNOWN;
    if (info) {
        if (info->video[0]       ||
            info->framerate[0]   ||
            info->resolution[0]  ||
            info->video_range[0] ||
            has_video_codec(info->codec)) {
            var->media_type = AVMEDIA_TYPE_VIDEO;
        }
        pls->bandwidth = var->bandwidth = atoi(info->bandwidth);
        strcpy(var->audio_group, info->audio);
        strcpy(var->video_group, info->video);
        strcpy(var->subtitles_group, info->subtitles);
        strcpy(var->codec, info->codec);
        pls->media_type = var->media_type;
        if (AVMEDIA_TYPE_VIDEO == var->media_type) {
            parse_variant_video_info(info, pls);
        }
    }
    dynarray_add(&c->variants, &c->n_variants, var);
    dynarray_add(&var->playlists, &var->n_playlists, pls);
    return var;
}

static void handle_variant_args(struct variant_info *info, const char *key,
                                int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "BANDWIDTH=", key_len)) {
        *dest     =        info->bandwidth;
        *dest_len = sizeof(info->bandwidth);
    } else if (!strncmp(key, "AUDIO=", key_len)) {
        *dest     =        info->audio;
        *dest_len = sizeof(info->audio);
    } else if (!strncmp(key, "VIDEO=", key_len)) {
        *dest     =        info->video;
        *dest_len = sizeof(info->video);
    } else if (!strncmp(key, "SUBTITLES=", key_len)) {
        *dest     =        info->subtitles;
        *dest_len = sizeof(info->subtitles);
    } else if (!strncmp(key, "CODECS=", key_len)) {
        *dest     =        info->codec;
        *dest_len = sizeof(info->codec);
        av_log(NULL, AV_LOG_INFO, "Codec: %s\n", key);
    } else if (!strncmp(key, "RESOLUTION=", key_len)) {
        *dest     =        info->resolution;
        *dest_len = sizeof(info->resolution);
    } else if (!strncmp(key, "FRAME-RATE=", key_len)) {
        *dest     =        info->framerate;
        *dest_len = sizeof(info->framerate);
    } else if (!strncmp(key, "VIDEO-RANGE==", key_len)) {
        *dest     =        info->video_range;
        *dest_len = sizeof(info->video_range);
    }
}

struct key_info {
     char uri[MAX_URL_SIZE];
     char method[11];
     char iv[35];
     char keyformat[MAX_FIELD_LEN]; /* For chinadrm case */
};

static void handle_key_args(struct key_info *info, const char *key,
                            int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "METHOD=", key_len)) {
        *dest     =        info->method;
        *dest_len = sizeof(info->method);
    } else if (!strncmp(key, "URI=", key_len)) {
        *dest     =        info->uri;
        *dest_len = sizeof(info->uri);
    } else if (!strncmp(key, "IV=", key_len)) {
        *dest     =        info->iv;
        *dest_len = sizeof(info->iv);
    } else if (!strncmp(key, "KEYFORMAT=", key_len)) {
        *dest     =        info->keyformat;
        *dest_len = sizeof(info->keyformat);
    }
}

struct init_section_info {
    char uri[MAX_URL_SIZE];
    char byterange[32];
};

static struct segment *new_init_section(struct playlist *pls,
                                        struct init_section_info *info,
                                        const char *url_base)
{
    struct segment *sec;
    char *ptr;
    char tmp_str[MAX_URL_SIZE];

    if (!info->uri[0])
        return NULL;

    sec = av_mallocz(sizeof(*sec));
    if (!sec)
        return NULL;

    ff_make_absolute_url(tmp_str, sizeof(tmp_str), url_base, info->uri);
    sec->url = av_strdup(tmp_str);
    if (!sec->url) {
        av_free(sec);
        return NULL;
    }

    if (info->byterange[0]) {
        sec->size = strtoll(info->byterange, NULL, 10);
        ptr = strchr(info->byterange, '@');
        if (ptr)
            sec->url_offset = strtoll(ptr+1, NULL, 10);
    } else {
        /* the entire file is the init section */
        sec->size = -1;
    }

    dynarray_add(&pls->init_sections, &pls->n_init_sections, sec);

    return sec;
}

static void handle_init_section_args(struct init_section_info *info, const char *key,
                                           int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "URI=", key_len)) {
        *dest     =        info->uri;
        *dest_len = sizeof(info->uri);
    } else if (!strncmp(key, "BYTERANGE=", key_len)) {
        *dest     =        info->byterange;
        *dest_len = sizeof(info->byterange);
    }
}

struct rendition_info {
    char type[16];
    char uri[MAX_URL_SIZE];
    char group_id[MAX_FIELD_LEN];
    char language[MAX_FIELD_LEN];
    char assoc_language[MAX_FIELD_LEN];
    char name[MAX_FIELD_LEN];
    char defaultr[4];
    char forced[4];
    char characteristics[MAX_CHARACTERISTICS_LEN];
};

static struct rendition *new_rendition(HLSContext *c, struct rendition_info *info,
                                      const char *url_base)
{
    struct rendition *rend;
    enum AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
    char *characteristic;
    char *chr_ptr;
    char *saveptr;

    if (!strcmp(info->type, "AUDIO"))
        type = AVMEDIA_TYPE_AUDIO;
    else if (!strcmp(info->type, "VIDEO"))
        type = AVMEDIA_TYPE_VIDEO;
    else if (!strcmp(info->type, "SUBTITLES"))
        type = AVMEDIA_TYPE_SUBTITLE;
    else if (!strcmp(info->type, "CLOSED-CAPTIONS"))
        /* CLOSED-CAPTIONS is ignored since we do not support CEA-608 CC in
         * AVC SEI RBSP anyway */
        return NULL;

    if (type == AVMEDIA_TYPE_UNKNOWN) {
        av_log(c, AV_LOG_WARNING, "Can't support the type: %s\n", info->type);
        return NULL;
    }

    /* URI is mandatory for subtitles as per spec */
    if (type == AVMEDIA_TYPE_SUBTITLE && !info->uri[0]) {
        av_log(c, AV_LOG_ERROR, "The URI tag is REQUIRED for subtitle.\n");
        return NULL;
    }

    /* TODO: handle subtitles (each segment has to parsed separately) */
    if (c->ctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL)
        if (type == AVMEDIA_TYPE_SUBTITLE) {
            av_log(c, AV_LOG_WARNING, "Can't support the subtitle(uri: %s)\n", info->uri);
            return NULL;
        }

    rend = av_mallocz(sizeof(struct rendition));
    if (!rend)
        return NULL;

    dynarray_add(&c->renditions, &c->n_renditions, rend);

    rend->type = type;
    strcpy(rend->group_id, info->group_id);
    strcpy(rend->language, info->language);
    strcpy(rend->name, info->name);

    /* add the playlist if this is an external rendition */
    if (info->uri[0]) {
        rend->playlist = new_playlist(c, info->uri, url_base);
        if (rend->playlist) {
            dynarray_add(&rend->playlist->renditions,
                         &rend->playlist->n_renditions, rend);
            rend->playlist->media_type = type;
        }
    }

    if (info->assoc_language[0]) {
        int langlen = strlen(rend->language);
        if (langlen < sizeof(rend->language) - 3) {
            rend->language[langlen] = ',';
            strncpy(rend->language + langlen + 1, info->assoc_language,
                    sizeof(rend->language) - langlen - 2);
        }
    }

    if (!strcmp(info->defaultr, "YES"))
        rend->disposition |= AV_DISPOSITION_DEFAULT;
    if (!strcmp(info->forced, "YES"))
        rend->disposition |= AV_DISPOSITION_FORCED;

    chr_ptr = info->characteristics;
    while ((characteristic = av_strtok(chr_ptr, ",", &saveptr))) {
        if (!strcmp(characteristic, "public.accessibility.describes-music-and-sound"))
            rend->disposition |= AV_DISPOSITION_HEARING_IMPAIRED;
        else if (!strcmp(characteristic, "public.accessibility.describes-video"))
            rend->disposition |= AV_DISPOSITION_VISUAL_IMPAIRED;

        chr_ptr = NULL;
    }

    return rend;
}

static void handle_rendition_args(struct rendition_info *info, const char *key,
                                  int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "TYPE=", key_len)) {
        *dest     =        info->type;
        *dest_len = sizeof(info->type);
    } else if (!strncmp(key, "URI=", key_len)) {
        *dest     =        info->uri;
        *dest_len = sizeof(info->uri);
    } else if (!strncmp(key, "GROUP-ID=", key_len)) {
        *dest     =        info->group_id;
        *dest_len = sizeof(info->group_id);
    } else if (!strncmp(key, "LANGUAGE=", key_len)) {
        *dest     =        info->language;
        *dest_len = sizeof(info->language);
    } else if (!strncmp(key, "ASSOC-LANGUAGE=", key_len)) {
        *dest     =        info->assoc_language;
        *dest_len = sizeof(info->assoc_language);
    } else if (!strncmp(key, "NAME=", key_len)) {
        *dest     =        info->name;
        *dest_len = sizeof(info->name);
    } else if (!strncmp(key, "DEFAULT=", key_len)) {
        *dest     =        info->defaultr;
        *dest_len = sizeof(info->defaultr);
    } else if (!strncmp(key, "FORCED=", key_len)) {
        *dest     =        info->forced;
        *dest_len = sizeof(info->forced);
    } else if (!strncmp(key, "CHARACTERISTICS=", key_len)) {
        *dest     =        info->characteristics;
        *dest_len = sizeof(info->characteristics);
    }
    /*
     * ignored:
     * - AUTOSELECT: client may autoselect based on e.g. system language
     * - INSTREAM-ID: EIA-608 closed caption number ("CC1".."CC4")
     */
}

/* used by parse_playlist to allocate a new variant+playlist when the
 * playlist is detected to be a Media Playlist (not Master Playlist)
 * and we have no parent Master Playlist (parsing of which would have
 * allocated the variant and playlist already)
 * *pls == NULL  => Master Playlist or parentless Media Playlist
 * *pls != NULL => parented Media Playlist, playlist+variant allocated */
static int ensure_playlist(HLSContext *c, struct playlist **pls, const char *url)
{
    if (*pls)
        return 0;
    if (!new_variant(c, NULL, url, NULL))
        return AVERROR(ENOMEM);
    *pls = c->playlists[c->n_playlists - 1];
    return 0;
}

static int open_url_keepalive_l(AVFormatContext *s, AVIOContext **pb,
                              const char *url)
{
#if !CONFIG_HTTP_PROTOCOL
    return AVERROR_PROTOCOL_NOT_FOUND;
#else
    int ret;
    URLContext *uc = ffio_geturlcontext(*pb);
    av_assert0(uc);
    (*pb)->eof_reached = 0;
    ret = ff_http_do_new_request(uc, url);
    if (ret < 0) {
        close_format_io(s, NULL, pb);
    }
    return ret;
#endif
}

static int open_url_keepalive(AVFormatContext *s, AVIOContext **pb,
                              const char *url)
{
    int ret = 0;
    HLSContext *c = s->priv_data;

    ret = open_url_keepalive_l(s, pb, url);
    if (AVERROR(EINVAL) == ret) {
        c->http_persistent = 0;
    }
    return ret;
}

static int open_url_l(AVFormatContext *s, AVIOContext **pb, const char *url,
                    AVDictionary *opts, AVDictionary *opts2, int *is_http_out)
{
    HLSContext *c = s->priv_data;
    AVDictionary *tmp = NULL;
    const char *proto_name = NULL;
    int ret;
    int is_http = 0;

    av_dict_copy(&tmp, opts, 0);
    av_dict_copy(&tmp, opts2, 0);

    if (av_strstart(url, "crypto", NULL)) {
        if (url[6] == '+' || url[6] == ':')
            proto_name = avio_find_protocol_name(url + 7);
    }

    if (!proto_name)
        proto_name = avio_find_protocol_name(url);

    if (!proto_name)
        return AVERROR_INVALIDDATA;

    // only http(s) & file are allowed
    if (av_strstart(proto_name, "file", NULL)) {
        if (strcmp(c->allowed_extensions, "ALL") && !av_match_ext(url, c->allowed_extensions)) {
            av_log(s, AV_LOG_ERROR,
                "Filename extension of \'%s\' is not a common multimedia extension, blocked for security reasons.\n"
                "If you wish to override this adjust allowed_extensions, you can set it to \'ALL\' to allow all\n",
                url);
            return AVERROR_INVALIDDATA;
        }
    } else if (av_strstart(proto_name, "http", NULL)) {
        is_http = 1;
    } else
        return AVERROR_INVALIDDATA;

    if (!strncmp(proto_name, url, strlen(proto_name)) && url[strlen(proto_name)] == ':')
        ;
    else if (av_strstart(url, "crypto", NULL) && !strncmp(proto_name, url + 7, strlen(proto_name)) && url[7 + strlen(proto_name)] == ':')
        ;
    else if (strcmp(proto_name, "file") || !strncmp(url, "file,", 5))
        return AVERROR_INVALIDDATA;

    if (is_http && c->http_persistent && *pb) {
        ret = open_url_keepalive(c->ctx, pb, url);
        if (ret == AVERROR_EXIT) {
            return ret;
        } else if (ret < 0) {
            if (ret != AVERROR_EOF)
                av_log(s, AV_LOG_WARNING,
                    "keepalive request failed for '%s' when opening url, retrying with new connection: %s\n",
                    url, av_err2str(ret));
            ret = s->io_open(s, pb, url, AVIO_FLAG_READ, &tmp);
        }
    } else {
        ret = s->io_open(s, pb, url, AVIO_FLAG_READ, &tmp);
    }
    if (ret >= 0) {
        // update cookies on http response with setcookies.
        char *new_cookies = NULL;

        if (!(s->flags & AVFMT_FLAG_CUSTOM_IO))
            av_opt_get(*pb, "cookies", AV_OPT_SEARCH_CHILDREN, (uint8_t**)&new_cookies);

        if (new_cookies)
            av_dict_set(&opts, "cookies", new_cookies, AV_DICT_DONT_STRDUP_VAL);
    }

    av_dict_free(&tmp);

    if (is_http_out)
        *is_http_out = is_http;

    return ret;
}

static int open_url(AVFormatContext *s, AVIOContext **pb, const char *url,
                    AVDictionary *opts, AVDictionary *opts2, int *is_http_out)
{
    int ret = 0;
    HLSContext *c = s->priv_data;

    ret = open_url_l(s, pb, url, opts, opts2, is_http_out);

    return ret;
}

/* Find seq number when pls start number smaller than prev start number */
static void parse_playlist_set_seq_no(
    struct segment **src, int n_src, int start_no_src, struct playlist *pls)
{
    if (pls->start_seq_no >= start_no_src) {
        return;
    }

    int idx = 0;
    struct segment *cur = NULL;
    int n_dst = pls->n_segments;
    struct segment **dst = pls->segments;
    /* Next playlist not in prev segments */
    if (pls->cur_seq_no >= n_src + start_no_src) {
        cur = src[n_src - 1];
    } else if (pls->cur_seq_no > start_no_src){
        cur = src[pls->cur_seq_no - start_no_src - 1];
    }
    if (!cur) {
        av_log(pls->parent, AV_LOG_WARNING,
            "[%d] Not find out proper playlist curr:%d start:%d num:%d use new:%d\n",
            pls->index, pls->cur_seq_no, start_no_src, n_src, pls->start_seq_no);
        pls->last_seq_no = pls->cur_seq_no = pls->start_seq_no;
        return;
    }
    for (idx = n_dst - 1; idx >= 0; idx--) {
        if(!strcmp(cur->url, dst[idx]->url)) {
            break;
        }
    }
    pls->last_seq_no = pls->cur_seq_no = pls->start_seq_no + idx + 1;
    av_log(pls->parent, AV_LOG_WARNING,
        "[%d]Set cur seq no curr:%d start:%d num:%d use new:%d\n",
        pls->index, pls->cur_seq_no, start_no_src, n_src, pls->start_seq_no);
}

static enum KeyType get_ext_x_key_type(struct key_info *info)
{
    enum KeyType key_type = KEY_NONE;
    if (!strcmp(info->method, "AES-128"))
        key_type = KEY_AES_128;
    if (!strcmp(info->method, "SAMPLE-AES"))
        key_type = KEY_SAMPLE_AES;
    if (!strcmp(info->method, "SM4-CBC"))
        key_type = KEY_CHINA_DRM_SM4C;
    if (!strcmp(info->method, "SAMPLE-SM4"))
        key_type = KEY_CHINA_DRM_SM4S;

    return key_type;
}

static int parse_playlist_l(HLSContext *c,
    const char *url, struct playlist *pls, AVIOContext *in)
{
    int ret = 0, is_segment = 0, is_variant = 0;
    int64_t duration = 0;
    enum KeyType key_type = KEY_NONE;
    uint8_t iv[16] = "";
    int has_iv = 0;
    char key[MAX_URL_SIZE] = "";
    char line[MAX_URL_SIZE];
    const char *ptr;
    int close_in = 0;
    int64_t seg_offset = 0;
    int64_t seg_size = -1;
    uint8_t *new_url = NULL;
    struct variant_info variant_info;
    char tmp_str[MAX_URL_SIZE];
    struct segment *cur_init_section = NULL;
    int is_http = av_strstart(url, "http", NULL);
    struct segment **prev_segments = NULL;
    int prev_n_segments = 0;
    int64_t prev_start_seq_no = -1;

    if (is_http && !in && c->http_persistent && c->playlist_pb) {
        in = c->playlist_pb;
        ret = open_url_keepalive(c->ctx, &c->playlist_pb, url);
        if (ret == AVERROR_EXIT) {
            return ret;
        } else if (ret < 0) {
            if (ret != AVERROR_EOF)
                av_log(c->ctx, AV_LOG_WARNING,
                    "keepalive request failed for '%s' when parsing playlist, retrying with new connection: %s\n",
                    url, av_err2str(ret));
            in = NULL;
        }
    }

    if (!in) {
        AVDictionary *opts = NULL;
        av_dict_copy(&opts, c->avio_opts, 0);

        if (c->http_persistent)
            av_dict_set(&opts, "multiple_requests", "1", 0);

        ret = c->ctx->io_open(c->ctx, &in, url, AVIO_FLAG_READ, &opts);
        av_dict_free(&opts);
        if (ret < 0)
            return ret;

        if (is_http && c->http_persistent)
            c->playlist_pb = in;
        else
            close_in = 1;
    }

    if (av_opt_get(in, "location", AV_OPT_SEARCH_CHILDREN, &new_url) >= 0)
        url = new_url;

    ff_get_chomp_line(in, line, sizeof(line));
    if (strcmp(line, "#EXTM3U")) {
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

    if (pls) {
        prev_start_seq_no = pls->start_seq_no;
        prev_segments = pls->segments;
        prev_n_segments = pls->n_segments;
        pls->segments = NULL;
        pls->n_segments = 0;

        pls->finished = 0;
        pls->type = PLS_TYPE_UNSPECIFIED;
    }

    AVEncryptionInitInfo *encryption_info = NULL;
    while (!avio_feof(in)) {
        ff_get_chomp_line(in, line, sizeof(line));
        if (av_strstart(line, "#EXT-X-STREAM-INF:", &ptr)) {
            is_variant = 1;
            memset(&variant_info, 0, sizeof(variant_info));
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_variant_args,
                               &variant_info);
        } else if (av_strstart(line, "#EXT-X-KEY:", &ptr)) {
            struct key_info info = {{0}};
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_key_args,
                               &info);
            has_iv = 0;
            key_type = get_ext_x_key_type(&info);
            if (!av_strncasecmp(info.iv, "0x", 2)) {
                ff_hex_to_data(iv, info.iv + 2);
                has_iv = 1;
            }
            (void) ff_hls_parse_ext_x_key(info.uri, info.keyformat, &encryption_info);
            av_strlcpy(key, info.uri, sizeof(key));
        } else if (av_strstart(line, "#EXT-X-MEDIA:", &ptr)) {
            struct rendition_info info = {{0}};
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_rendition_args,
                               &info);
            new_rendition(c, &info, url);
        } else if (av_strstart(line, "#EXT-X-TARGETDURATION:", &ptr)) {
            int64_t t;
            ret = ensure_playlist(c, &pls, url);
            if (ret < 0)
                goto fail;
            t = strtoll(ptr, NULL, 10);
            if (t < 0 || t >= INT64_MAX / AV_TIME_BASE) {
                ret = AVERROR_INVALIDDATA;
                goto fail;
            }
            pls->target_duration = t * AV_TIME_BASE;
        } else if (av_strstart(line, "#EXT-X-MEDIA-SEQUENCE:", &ptr)) {
            uint64_t seq_no;
            ret = ensure_playlist(c, &pls, url);
            if (ret < 0)
                goto fail;
            seq_no = strtoull(ptr, NULL, 10);
            if (seq_no > INT64_MAX/2) {
                av_log(c->ctx, AV_LOG_DEBUG, "MEDIA-SEQUENCE higher than "
                        "INT64_MAX/2, mask out the highest bit\n");
                seq_no &= INT64_MAX/2;
            }
            pls->start_seq_no = seq_no;
        } else if (av_strstart(line, "#EXT-X-PLAYLIST-TYPE:", &ptr)) {
            ret = ensure_playlist(c, &pls, url);
            if (ret < 0)
                goto fail;
            if (!strcmp(ptr, "EVENT"))
                pls->type = PLS_TYPE_EVENT;
            else if (!strcmp(ptr, "VOD"))
                pls->type = PLS_TYPE_VOD;
        } else if (av_strstart(line, "#EXT-X-MAP:", &ptr)) {
            struct init_section_info info = {{0}};
            ret = ensure_playlist(c, &pls, url);
            if (ret < 0)
                goto fail;
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_init_section_args,
                               &info);
            cur_init_section = new_init_section(pls, &info, url);
            if (!cur_init_section) {
                ret = AVERROR(ENOMEM);
                goto fail;
            }
            cur_init_section->key_type = key_type;
            if (has_iv) {
                memcpy(cur_init_section->iv, iv, sizeof(iv));
            } else {
                int64_t seq = pls->start_seq_no + pls->n_segments;
                memset(cur_init_section->iv, 0, sizeof(cur_init_section->iv));
                AV_WB64(cur_init_section->iv + 8, seq);
            }

            if (key_type != KEY_NONE) {
                ff_make_absolute_url(tmp_str, sizeof(tmp_str), url, key);
                if (!tmp_str[0]) {
                    av_free(cur_init_section);
                    ret = AVERROR_INVALIDDATA;
                    goto fail;
                }
                cur_init_section->key = av_strdup(tmp_str);
                if (!cur_init_section->key) {
                    av_free(cur_init_section);
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
            } else {
                cur_init_section->key = NULL;
            }

        } else if (av_strstart(line, "#EXT-X-START:", &ptr)) {
            const char *time_offset_value = NULL;
            ret = ensure_playlist(c, &pls, url);
            if (ret < 0) {
                goto fail;
            }
            if (av_strstart(ptr, "TIME-OFFSET=", &time_offset_value)) {
                float offset = strtof(time_offset_value, NULL);
                pls->start_time_offset = (int64_t) (offset * AV_TIME_BASE);
                pls->time_offset_flag = 1;
            } else {
                av_log(c->ctx, AV_LOG_WARNING, "#EXT-X-START value is"
                                                "invalid, it will be ignored");
                continue;
            }
        } else if (av_strstart(line, "#EXT-X-ENDLIST", &ptr)) {
            if (pls)
                pls->finished = 1;
        } else if (av_strstart(line, "#EXTINF:", &ptr)) {
            is_segment = 1;
            duration   = atof(ptr) * AV_TIME_BASE;
        } else if (av_strstart(line, "#EXT-X-BYTERANGE:", &ptr)) {
            seg_size = strtoll(ptr, NULL, 10);
            ptr = strchr(ptr, '@');
            if (ptr)
                seg_offset = strtoll(ptr+1, NULL, 10);
        } else if (av_strstart(line, "#", NULL)) {
            av_log(c->ctx, AV_LOG_DEBUG, "Skip ('%s')\n", line);
            continue;
        } else if (line[0]) {
            if (is_variant) {
                if (!new_variant(c, &variant_info, line, url)) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
                is_variant = 0;
            }
            if (is_segment) {
                struct segment *seg;
                ret = ensure_playlist(c, &pls, url);
                if (ret < 0)
                    goto fail;
                seg = av_malloc(sizeof(struct segment));
                if (!seg) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
                if (has_iv) {
                    memcpy(seg->iv, iv, sizeof(iv));
                } else {
                    uint64_t seq = pls->start_seq_no + (uint64_t)pls->n_segments;
                    memset(seg->iv, 0, sizeof(seg->iv));
                    AV_WB64(seg->iv + 8, seq);
                }

                if (key_type != KEY_NONE) {
                    ff_make_absolute_url(tmp_str, sizeof(tmp_str), url, key);
                    if (!tmp_str[0]) {
                        ret = AVERROR_INVALIDDATA;
                        av_free(seg);
                        goto fail;
                    }
                    seg->key = av_strdup(tmp_str);
                    if (!seg->key) {
                        av_free(seg);
                        ret = AVERROR(ENOMEM);
                        goto fail;
                    }
                } else {
                    seg->key = NULL;
                }

                ff_make_absolute_url(tmp_str, sizeof(tmp_str), url, line);
                if (!tmp_str[0]) {
                    ret = AVERROR_INVALIDDATA;
                    if (seg->key)
                        av_free(seg->key);
                    av_free(seg);
                    goto fail;
                }
                seg->url = av_strdup(tmp_str);
                if (!seg->url) {
                    av_free(seg->key);
                    av_free(seg);
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }

                if (duration < 0.001 * AV_TIME_BASE) {
                    av_log(c->ctx, AV_LOG_WARNING, "Cannot get correct #EXTINF value of segment %s,"
                                    " set to default value to 1ms.\n", seg->url);
                    duration = 0.001 * AV_TIME_BASE;
                }
                seg->duration = duration;
                seg->key_type = key_type;
                dynarray_add(&pls->segments, &pls->n_segments, seg);
                is_segment = 0;

                seg->size = seg_size;
                if (seg_size >= 0) {
                    seg->url_offset = seg_offset;
                    seg_offset += seg_size;
                    seg_size = -1;
                } else {
                    seg->url_offset = 0;
                    seg_offset = 0;
                }

                seg->init_section = cur_init_section;
            }
        }
        if (pls && encryption_info) {
            if (pls->encryption_info) {
                av_encryption_init_info_free(pls->encryption_info);
            }
            pls->encryption_info = encryption_info;
            pls->drm_sys_type = ff_decrypt_comp_get_drm_system_type(encryption_info);
            encryption_info = NULL;
        }
    }
    if (prev_segments) {
        if (pls->start_seq_no > prev_start_seq_no && c->first_timestamp != AV_NOPTS_VALUE) {
            int64_t prev_timestamp = c->first_timestamp;
            int i;
            int64_t diff = pls->start_seq_no - prev_start_seq_no;
            for (i = 0; i < prev_n_segments && i < diff; i++) {
                c->first_timestamp += prev_segments[i]->duration;
            }
            av_log(c->ctx, AV_LOG_DEBUG, "Media sequence change (%"PRId64" -> %"PRId64")"
                   " reflected in first_timestamp: %"PRId64" -> %"PRId64"\n",
                   prev_start_seq_no, pls->start_seq_no,
                   prev_timestamp, c->first_timestamp);
        } else if (pls->start_seq_no < prev_start_seq_no) {
            pls->m3u8_discontinuity = 1;
            av_log(c->ctx, AV_LOG_ERROR, "Media sequence changed unexpectedly: %"PRId64" -> %"PRId64"\n",
                   prev_start_seq_no, pls->start_seq_no);
            parse_playlist_set_seq_no(prev_segments, prev_n_segments, prev_start_seq_no, pls);
        }
        move_segments(prev_segments,
            prev_n_segments, pls->segments, pls->n_segments);
        free_segment_dynarray(prev_segments, prev_n_segments);
        av_freep(&prev_segments);
    }
    if (pls)
        pls->last_load_time = av_gettime_relative();

fail:
    av_free(new_url);
    if (close_in)
        close_format_io(c->ctx, NULL, &in);
    c->ctx->ctx_flags = c->ctx->ctx_flags & ~(unsigned)AVFMTCTX_UNSEEKABLE;
    if (!c->n_variants || !c->variants[0]->n_playlists ||
        !(c->variants[0]->playlists[0]->finished ||
          c->variants[0]->playlists[0]->type == PLS_TYPE_EVENT))
        c->ctx->ctx_flags |= AVFMTCTX_UNSEEKABLE;
    return ret;
}

static int parse_playlist(HLSContext *c, const char *url,
                          struct playlist *pls, AVIOContext *in)
{
    int ret = 0;
    int try_cnt = 0;
    /* Not allow update playlist when open demux */
    if (pls && PLS_OPEN_DEMUX == pls->needed) {
        return 0;
    }
reload:
    ret = parse_playlist_l(c, url, pls, in);
    if (ret < 0) {
        if (try_cnt++ < c->max_reload &&
            !ff_check_interrupt(c->interrupt_callback)) {
            av_usleep(10*1000);
            goto reload;
        }
        if (pls) {
            av_log(pls->parent, AV_LOG_ERROR,
                   "[%d] Failed to reload playlist %s by 0x%x %s\n",
                   pls->index, pls->url, ret, av_err2str(ret));
        }
    }
    return ret;
}

static struct segment *current_segment(struct playlist *pls)
{
    int64_t n = pls->cur_seq_no - pls->start_seq_no;
    if (n >= pls->n_segments)
        return NULL;
    return pls->segments[n];
}

static struct segment *next_segment(struct playlist *pls)
{
    int64_t n = pls->cur_seq_no - pls->start_seq_no + 1;
    if (n >= pls->n_segments)
        return NULL;
    return pls->segments[n];
}

static AVRational get_timebase(struct playlist *pls)
{
    if (pls->is_id3_timestamped)
        return MPEG_TIME_BASE_Q;

    return pls->ctx->streams[pls->pkt.stream_index]->time_base;
}

static int read_from_url(struct playlist *pls, int64_t seg_size,
                         uint8_t *buf, int buf_size)
{
    int ret;

     /* limit read if the segment was only a part of a file */

    if (seg_size >= 0)
        buf_size = FFMIN(buf_size, seg_size - pls->cur_seg_offset);

    AVFormatContext *s = pls->parent;
    HLSContext *c = s->priv_data;

    av_log(s, AV_LOG_DEBUG, "[%p] Read from url offset:%lld need:%d\n", pls->input, pls->cur_seg_offset, buf_size);
    ret = avio_read(pls->input, buf, buf_size);
    if (ret > 0)
        pls->cur_seg_offset += ret;
    av_log(s, AV_LOG_DEBUG, "[%p] Read from url, readed:%d\n", pls->input, ret);

    return ret;
}

/* Parse the raw ID3 data and pass contents to caller */
static void parse_id3(AVFormatContext *s, AVIOContext *pb,
                      AVDictionary **metadata, int64_t *dts,
                      ID3v2ExtraMetaAPIC **apic, ID3v2ExtraMeta **extra_meta)
{
    static const char id3_priv_owner_ts[] = "com.apple.streaming.transportStreamTimestamp";
    ID3v2ExtraMeta *meta;

    ff_id3v2_read_dict(pb, metadata, ID3v2_DEFAULT_MAGIC, extra_meta);
    for (meta = *extra_meta; meta; meta = meta->next) {
        if (!strcmp(meta->tag, "PRIV")) {
            ID3v2ExtraMetaPRIV *priv = meta->data;
            if (priv->datasize == 8 && !strcmp(priv->owner, id3_priv_owner_ts)) {
                /* 33-bit MPEG timestamp */
                int64_t ts = AV_RB64(priv->data);
                av_log(s, AV_LOG_DEBUG, "HLS ID3 audio timestamp %"PRId64"\n", ts);
                if ((ts & ~((1ULL << 33) - 1)) == 0)
                    *dts = ts;
                else
                    av_log(s, AV_LOG_ERROR, "Invalid HLS ID3 audio timestamp %"PRId64"\n", ts);
            }
        } else if (!strcmp(meta->tag, "APIC") && apic)
            *apic = meta->data;
    }
}

/* Check if the ID3 metadata contents have changed */
static int id3_has_changed_values(struct playlist *pls, AVDictionary *metadata,
                                  ID3v2ExtraMetaAPIC *apic)
{
    AVDictionaryEntry *entry = NULL;
    AVDictionaryEntry *oldentry;
    /* check that no keys have changed values */
    while ((entry = av_dict_get(metadata, "", entry, AV_DICT_IGNORE_SUFFIX))) {
        oldentry = av_dict_get(pls->id3_initial, entry->key, NULL, AV_DICT_MATCH_CASE);
        if (!oldentry || strcmp(oldentry->value, entry->value) != 0)
            return 1;
    }

    /* check if apic appeared */
    if (apic && (pls->ctx->nb_streams != 2 || !pls->ctx->streams[1]->attached_pic.data))
        return 1;

    if (apic) {
        int size = pls->ctx->streams[1]->attached_pic.size;
        if (size != apic->buf->size - AV_INPUT_BUFFER_PADDING_SIZE)
            return 1;

        if (memcmp(apic->buf->data, pls->ctx->streams[1]->attached_pic.data, size) != 0)
            return 1;
    }

    return 0;
}

/* Parse ID3 data and handle the found data */
static void handle_id3(AVIOContext *pb, struct playlist *pls)
{
    AVDictionary *metadata = NULL;
    ID3v2ExtraMetaAPIC *apic = NULL;
    ID3v2ExtraMeta *extra_meta = NULL;
    int64_t timestamp = AV_NOPTS_VALUE;

    parse_id3(pls->ctx, pb, &metadata, &timestamp, &apic, &extra_meta);
    if (timestamp != AV_NOPTS_VALUE) {
        AVFormatContext *s = pls->parent;
        HLSContext *c = s->priv_data;
        if (c->first_packet) {
            if (AV_NOPTS_VALUE == pls->id3_mpegts_timestamp) { /* Not advance timestamp for pending pkt */
                pls->id3_mpegts_timestamp = timestamp;
            }
        } else {
            pls->id3_mpegts_timestamp = timestamp;
        }
        pls->id3_offset = 0;
    }

    if (!pls->id3_found) {
        /* initial ID3 tags */
        av_assert0(!pls->id3_deferred_extra);
        pls->id3_found = 1;

        /* get picture attachment and set text metadata */
        if (pls->ctx->nb_streams)
            ff_id3v2_parse_apic(pls->ctx, &extra_meta);
        else
            /* demuxer not yet opened, defer picture attachment */
            pls->id3_deferred_extra = extra_meta;

        ff_id3v2_parse_priv_dict(&metadata, &extra_meta);
        av_dict_copy(&pls->ctx->metadata, metadata, 0);
        pls->id3_initial = metadata;

    } else {
        if (!pls->id3_changed && id3_has_changed_values(pls, metadata, apic)) {
            avpriv_report_missing_feature(pls->ctx, "Changing ID3 metadata in HLS audio elementary stream");
            pls->id3_changed = 1;
        }
        av_dict_free(&metadata);
    }

    if (!pls->id3_deferred_extra)
        ff_id3v2_free_extra_meta(&extra_meta);
}

static void intercept_id3(struct playlist *pls, uint8_t *buf,
                         int buf_size, int *len)
{
    /* intercept id3 tags, we do not want to pass them to the raw
     * demuxer on all segment switches */
    int bytes;
    int id3_buf_pos = 0;
    int fill_buf = 0;
    struct segment *seg = current_segment(pls);

    /* gather all the id3 tags */
    while (1) {
        /* see if we can retrieve enough data for ID3 header */
        if (*len < ID3v2_HEADER_SIZE && buf_size >= ID3v2_HEADER_SIZE) {
            bytes = read_from_url(pls, seg->size, buf + *len, ID3v2_HEADER_SIZE - *len);
            if (bytes > 0) {

                if (bytes == ID3v2_HEADER_SIZE - *len)
                    /* no EOF yet, so fill the caller buffer again after
                     * we have stripped the ID3 tags */
                    fill_buf = 1;

                *len += bytes;

            } else if (*len <= 0) {
                /* error/EOF */
                *len = bytes;
                fill_buf = 0;
            }
        }

        if (*len < ID3v2_HEADER_SIZE)
            break;

        if (ff_id3v2_match(buf, ID3v2_DEFAULT_MAGIC)) {
            int64_t maxsize = seg->size >= 0 ? seg->size : 1024*1024;
            int taglen = ff_id3v2_tag_len(buf);
            int tag_got_bytes = FFMIN(taglen, *len);
            int remaining = taglen - tag_got_bytes;

            if (taglen > maxsize) {
                av_log(pls->ctx, AV_LOG_ERROR, "Too large HLS ID3 tag (%d > %"PRId64" bytes)\n",
                       taglen, maxsize);
                break;
            }

            /*
             * Copy the id3 tag to our temporary id3 buffer.
             * We could read a small id3 tag directly without memcpy, but
             * we would still need to copy the large tags, and handling
             * both of those cases together with the possibility for multiple
             * tags would make the handling a bit complex.
             */
            pls->id3_buf = av_fast_realloc(pls->id3_buf, &pls->id3_buf_size, id3_buf_pos + taglen);
            if (!pls->id3_buf)
                break;
            memcpy(pls->id3_buf + id3_buf_pos, buf, tag_got_bytes);
            id3_buf_pos += tag_got_bytes;

            /* strip the intercepted bytes */
            *len -= tag_got_bytes;
            memmove(buf, buf + tag_got_bytes, *len);
            av_log(pls->ctx, AV_LOG_DEBUG, "Stripped %d HLS ID3 bytes\n", tag_got_bytes);

            if (remaining > 0) {
                /* read the rest of the tag in */
                if (read_from_url(pls, seg->size, pls->id3_buf + id3_buf_pos, remaining) != remaining)
                    break;
                id3_buf_pos += remaining;
                av_log(pls->ctx, AV_LOG_DEBUG, "Stripped additional %d HLS ID3 bytes\n", remaining);
            }

        } else {
            /* no more ID3 tags */
            break;
        }
    }

    /* re-fill buffer for the caller unless EOF */
    if (*len >= 0 && (fill_buf || *len == 0)) {
        bytes = read_from_url(pls, seg->size, buf + *len, buf_size - *len);

        /* ignore error if we already had some data */
        if (bytes >= 0)
            *len += bytes;
        else if (*len == 0)
            *len = bytes;
    }

    if (pls->id3_buf) {
        /* Now parse all the ID3 tags */
        AVIOContext id3ioctx;
        ffio_init_context(&id3ioctx, pls->id3_buf, id3_buf_pos, 0, NULL, NULL, NULL, NULL);
        handle_id3(&id3ioctx, pls);
    }

    if (pls->is_id3_timestamped == -1)
        pls->is_id3_timestamped = (pls->id3_mpegts_timestamp != AV_NOPTS_VALUE);
}

static int open_input_l(HLSContext *c, struct playlist *pls, struct segment *seg, AVIOContext **in)
{
    AVDictionary *opts = NULL;
    int ret;
    int is_http = 0;
    AVFormatContext *s = pls->parent;

    if (c->http_persistent)
        av_dict_set(&opts, "multiple_requests", "1", 0);

    if (seg->size >= 0) {
        /* try to restrict the HTTP request to the part we want
         * (if this is in fact a HTTP request) */
        av_dict_set_int(&opts, "offset", seg->url_offset, 0);
        av_dict_set_int(&opts, "end_offset", seg->url_offset + seg->size, 0);
    }

    av_log(pls->parent, AV_LOG_VERBOSE, "HLS request for url '%s', offset %"PRId64", playlist %d\n",
           seg->url, seg->url_offset, pls->index);

    if (seg->key_type == KEY_NONE ||
        (KEY_CHINA_DRM_SM4T <= seg->key_type && KEY_CHINA_DRM_SM4S >= seg->key_type) ||
        (KEY_SAMPLE_AES == seg->key_type && DRM_SYSTEM_TYPE_CHINADRM == pls->drm_sys_type)) {
        ret = open_url(pls->parent, in, seg->url, c->avio_opts, opts, &is_http);
    } else if (seg->key_type == KEY_AES_128) {
        char iv[33], key[33], url[MAX_URL_SIZE];
        if (strcmp(seg->key, pls->key_url)) {
            AVIOContext *pb = NULL;
            if (open_url(pls->parent, &pb, seg->key, c->avio_opts, opts, NULL) == 0) {
                ret = avio_read(pb, pls->key, sizeof(pls->key));
                if (ret != sizeof(pls->key)) {
                    av_log(pls->parent, AV_LOG_ERROR, "Unable to read key file %s\n",
                           seg->key);
                }
                close_format_io(pls->parent, NULL, &pb);
            } else {
                av_log(pls->parent, AV_LOG_ERROR, "Unable to open key file %s\n",
                       seg->key);
            }
            av_strlcpy(pls->key_url, seg->key, sizeof(pls->key_url));
        }
        ff_data_to_hex(iv, seg->iv, sizeof(seg->iv), 0);
        ff_data_to_hex(key, pls->key, sizeof(pls->key), 0);
        iv[32] = key[32] = '\0';
        if (strstr(seg->url, "://"))
            snprintf(url, sizeof(url), "crypto+%s", seg->url);
        else
            snprintf(url, sizeof(url), "crypto:%s", seg->url);

        av_dict_set(&opts, "key", key, 0);
        av_dict_set(&opts, "iv", iv, 0);

        ret = open_url(pls->parent, in, url, c->avio_opts, opts, &is_http);
        if (ret < 0) {
            goto cleanup;
        }
        ret = 0;
    } else if (seg->key_type == KEY_SAMPLE_AES) {
        av_log(pls->parent, AV_LOG_ERROR,
               "SAMPLE-AES encryption is not supported yet\n");
        ret = AVERROR_PATCHWELCOME;
    }
    else
      ret = AVERROR(ENOSYS);

    /* Seek to the requested position. If this was a HTTP request, the offset
     * should already be where want it to, but this allows e.g. local testing
     * without a HTTP server.
     *
     * This is not done for HTTP at all as avio_seek() does internal bookkeeping
     * of file offset which is out-of-sync with the actual offset when "offset"
     * AVOption is used with http protocol, causing the seek to not be a no-op
     * as would be expected. Wrong offset received from the server will not be
     * noticed without the call, though.
     */
    if (ret == 0 && !is_http && seg->key_type == KEY_NONE && seg->url_offset) {
        int64_t seekret = avio_seek(*in, seg->url_offset, SEEK_SET);
        if (seekret < 0) {
            av_log(pls->parent, AV_LOG_ERROR, "Unable to seek to offset %"PRId64" of HLS segment '%s'\n", seg->url_offset, seg->url);
            ret = seekret;
            close_format_io(pls->parent, NULL, in);
        }
    }

cleanup:
    av_dict_free(&opts);
    pls->cur_seg_offset = 0;
    return ret;
}

static int open_input(HLSContext *c, struct playlist *pls, struct segment *seg, AVIOContext **in)
{
    int ret = open_input_l(c, pls, seg, in);
    if (ret < 0) {
        av_log(pls->parent, AV_LOG_ERROR,
               "[%d] Failed to open %s by %s from %s\n",
               pls->index, seg->url, av_err2str(ret), pls->url);
    }
    return ret;
}

static int update_init_section(struct playlist *pls, struct segment *seg)
{
    static const int max_init_section_size = 1024*1024;
    HLSContext *c = pls->parent->priv_data;
    int64_t sec_size;
    int64_t urlsize;
    int ret;

    if (seg->init_section == pls->cur_init_section)
        return 0;

    pls->cur_init_section = NULL;

    if (!seg->init_section)
        return 0;

    ret = open_input(c, pls, seg->init_section, &pls->input);
    if (ret < 0) {
        av_log(pls->parent, AV_LOG_WARNING,
               "Failed to open an initialization section in playlist %d\n",
               pls->index);
        return ret;
    }

    if (seg->init_section->size >= 0)
        sec_size = seg->init_section->size;
    else if ((urlsize = avio_size(pls->input)) >= 0)
        sec_size = urlsize;
    else
        sec_size = max_init_section_size;

    av_log(pls->parent, AV_LOG_DEBUG,
           "Downloading an initialization section of size %"PRId64"\n",
           sec_size);

    sec_size = FFMIN(sec_size, max_init_section_size);

    av_fast_malloc(&pls->init_sec_buf, &pls->init_sec_buf_size, sec_size);

    ret = read_from_url(pls, seg->init_section->size, pls->init_sec_buf,
                        pls->init_sec_buf_size);
    close_format_io(pls->parent, NULL, &pls->input);

    if (ret < 0)
        return ret;

    pls->cur_init_section = seg->init_section;
    pls->init_sec_data_len = ret;
    pls->init_sec_buf_read_offset = 0;

    /* spec says audio elementary streams do not have media initialization
     * sections, so there should be no ID3 timestamps */
    pls->is_id3_timestamped = 0;

    return 0;
}

static int64_t default_reload_interval(struct playlist *pls)
{
    return pls->n_segments > 0 ?
                          pls->segments[pls->n_segments - 1]->duration :
                          pls->target_duration;
}

static int playlist_needed(struct playlist *pls)
{
    AVFormatContext *s = pls->parent;
    int i, j;
    int stream_needed = 0;
    int first_st;

    HLSContext *c = s->priv_data;
    if (pls->needed < PLS_NEED_NO ||
        ADAPTIVE_STAGE_ASYNC_READ_HEADER == c->async.stage) {
        return pls->needed;
    }
    /* If there is no context or streams yet, the playlist is needed */
    if (!pls->ctx || !pls->n_main_streams)
        return PLS_NEED_YES;

    /* check if any of the streams in the playlist are needed */
    for (i = 0; i < pls->n_main_streams; i++) {
        if (pls->main_streams[i]->discard < AVDISCARD_ALL) {
            stream_needed = 1;
            break;
        }
    }

    /* If all streams in the playlist were discarded, the playlist is not
     * needed (regardless of whether whole programs are discarded or not). */
    if (!stream_needed)
        return PLS_NEED_NO;

    /* Otherwise, check if all the programs (variants) this playlist is in are
     * discarded. Since all streams in the playlist are part of the same programs
     * we can just check the programs of the first stream. */

    first_st = pls->main_streams[0]->index;

    for (i = 0; i < s->nb_programs; i++) {
        AVProgram *program = s->programs[i];
        if (program->discard < AVDISCARD_ALL) {
            for (j = 0; j < program->nb_stream_indexes; j++) {
                if (program->stream_index[j] == first_st) {
                    /* playlist is in an undiscarded program */
                    return PLS_NEED_YES;
                }
            }
        }
    }

    /* some streams were not discarded but all the programs were */
    return PLS_NEED_NO;
}

static int read_data_change_playlist(HLSContext *c, struct playlist *v)
{
    int need_change = 0;
    if (ADAPTIVE_STAGE_ALL_STREAM_READY == c->async.stage) {
        need_change =
            ff_adaptive_playlist_advance_fragment(
                v->parent, v->adaptive, NULL, 1, NULL);
    }
    if (need_change) {
        v->needed = PLS_NEED_CHANGE;
        c->need_change_playlist = 1;
    }
    return need_change;
}

static int read_data_correct_seq_no(HLSContext *c, struct playlist *v)
{
    if (v->cur_seq_no < v->start_seq_no) {
        av_log(v->parent, AV_LOG_WARNING,
               "skipping %"PRId64" segments ahead, expired from playlists\n",
               v->start_seq_no - v->cur_seq_no);
        v->cur_seq_no = v->start_seq_no;
    }
    if (v->cur_seq_no > v->last_seq_no) {
        v->last_seq_no = v->cur_seq_no;
        v->m3u8_hold_counters = 0;
    } else if (v->last_seq_no == v->cur_seq_no) {
        v->m3u8_hold_counters++;
        /* hold 4 segments and m3u8_discontinuity */
        if (v->m3u8_hold_counters >= 4 && v->m3u8_discontinuity) {
            v->last_seq_no = v->cur_seq_no = v->start_seq_no +
                FFMAX(0, v->n_segments - M3U8_LIVE_MIN_FRAGMENT_DISTANCE);
            v->m3u8_discontinuity = 0;
        } else if (v->m3u8_hold_counters >= c->m3u8_hold_counters) {
            return AVERROR_EOF;
        }
    } else {
        av_log(v->parent, AV_LOG_WARNING, "The m3u8 list sequence may have been wrapped.\n");
    }
    return 0;
}

static int read_data_open_current(
    HLSContext *c, struct playlist *v, struct segment *seg)
{
    int ret = 0;

    ret = open_input(c, v, seg, &v->input);
    /* Miss open input next for reason such as playlist not update */
    if (1 == c->http_multiple        &&
        0 == v->input_next_requested &&
        ADAPTIVE_STAGE_ALL_STREAM_READY == c->async.stage) {
        (void) ff_adaptive_playlist_io_init(v->parent, v->adaptive, v->input);
    }
    return ret;
}

static int read_data_open_next(HLSContext *c, struct playlist *v)
{
    int ret = 0;
    struct segment *seg = next_segment(v);
    if (c->http_multiple == 1 && 0 == v->input_next_requested &&
        seg && seg->key_type == KEY_NONE && av_strstart(seg->url, "http", NULL)) {
        if (c->use_thread && 0 &&
            ADAPTIVE_STAGE_ALL_STREAM_READY == c->async.stage) {
            v->input_next_requested = -1;
        } else if (/* ADAPTIVE_STAGE_INIT == c->async.stage || not open next when init and async open demux to accelerate startup */
                  (ADAPTIVE_STAGE_ASYNC_READ_HEADER == c->async.stage && PLS_NEED_YES != v->needed)) {
            return ret;
        } else {
            ret = open_input(c, v, seg, &v->input_next);
            if (ret < 0) {
                if (ff_check_interrupt(c->interrupt_callback))
                    return AVERROR_EXIT;
                av_log(v->parent, AV_LOG_WARNING, "Failed to open segment %"PRId64" of playlist %d\n",
                       v->cur_seq_no + 1,
                       v->index);
            } else {
                v->input_next_requested = 1;
            }
        }
    }
    return ret;
}

static int read_data_read(HLSContext *c,
    struct playlist *v, uint8_t *buf, int buf_size, int just_opened)
{
    int ret = 0;
    if (v->init_sec_buf_read_offset < v->init_sec_data_len) {
        /* Push init section out first before first actual segment */
        int copy_size = FFMIN(v->init_sec_data_len - v->init_sec_buf_read_offset, buf_size);
        memcpy(buf, v->init_sec_buf, copy_size);
        v->init_sec_buf_read_offset += copy_size;
        return copy_size;
    }

    struct segment *seg = current_segment(v);
    av_log(v->parent, AV_LOG_DEBUG, "[%d] Current seQ num:%d\n", v->index, v->cur_seq_no);
    ret = read_from_url(v, seg->size, buf, buf_size);
    if (ret > 0) {
        if (just_opened && v->is_id3_timestamped != 0) {
            /* Intercept ID3 tags here, elementary audio streams are required
             * to convey timestamps using them in the beginning of each segment. */
            intercept_id3(v, buf, buf_size, &ret);
        }
        return ret;
    }
    if (c->http_persistent &&
        seg->key_type == KEY_NONE && av_strstart(seg->url, "http", NULL)) {
        v->input_read_done = 1;
        ff_adaptive_playlist_io_restore(v->adaptive, &v->input);
    } else {
        close_format_io(v->parent, v, &v->input);
    }
    v->cur_seq_no++;
    c->cur_seq_no = v->cur_seq_no;
    if (read_data_change_playlist(c, v)) {
        return AVERROR_INPUT_CHANGED;
    }

    av_log(v->parent, AV_LOG_DEBUG, "read network data %s\n", av_err2str(ret));
    return ret;
}

static void read_data_swap_ioctx(
    HLSContext *c, struct playlist *v)
{
    v->cur_seg_offset = 0;
    v->input_next_requested = 0;
    ff_adaptive_playlist_io_restore(v->adaptive, &v->input);
    FFSWAP(AVIOContext *, v->input, v->input_next);
    if (ADAPTIVE_STAGE_ALL_STREAM_READY == c->async.stage) {
        (void) ff_adaptive_playlist_io_init(v->parent, v->adaptive, v->input);
    }
}

static void inline read_data_get_http_multiple(
    HLSContext *c, struct playlist *v)
{
    if (c->http_multiple != -1) {
        return;
    }

    uint8_t *http_version_opt = NULL;
    int r = av_opt_get(v->input, "http_version", AV_OPT_SEARCH_CHILDREN, &http_version_opt);
    if (r >= 0) {
        c->http_multiple = (!strncmp((const char *)http_version_opt, "1.1", 3) ||
                            !strncmp((const char *)http_version_opt, "2.0", 3));
        av_freep(&http_version_opt);
    }
}

static int64_t seek_data(void *opaque, int64_t offset, int whence)
{
    struct playlist *v = opaque;
    if (1 == v->n_segments && v->input) {
        if (v->input->seek && AVSEEK_SIZE == whence) {
            return v->input->seek(v->input->opaque, offset, whence);
        }
        int ret = avio_seek(v->input, offset, whence);
        av_log(v->parent, AV_LOG_INFO, "seek data seekable:%d offset:%lld ret:%d\n", v->input->seekable, offset, ret);
        return ret;
    }

    return AVERROR(ENOSYS);
}

static int read_data_l(HLSContext *c,
    struct playlist *v, uint8_t *buf, int buf_size)
{
    int ret;
    int just_opened = 0;
    int reload_count = 0;
    int segment_retries = 0;
    struct segment *seg;

restart:
    if (PLS_NEED_NO == v->needed || PLS_NEED_CHANGE == v->needed) {
        av_log(v->parent, AV_LOG_ERROR, "[%d] return eof by needed:%d\n", v->index, v->needed);
        return AVERROR_EOF;
    }

    if (!v->input || (c->http_persistent && v->input_read_done)) {
        int64_t reload_interval;
        /* Check that the playlist is still needed before opening a new
         * segment. */
        v->needed = playlist_needed(v);
        if (PLS_NEED_NO == v->needed) {
            av_log(v->parent, AV_LOG_ERROR, "No longer receiving playlist %d ('%s')\n",
                   v->index, v->url);
            return AVERROR_EOF;
        }

        /* If this is a live stream and the reload interval has elapsed since
         * the last playlist reload, reload the playlists now. */
        reload_interval = default_reload_interval(v);
reload:
        reload_count++;
        if (reload_count > c->max_reload) {
            av_log(v->parent, AV_LOG_ERROR,
                "[%d] return eof by max_reload:%d\n", v->index, c->max_reload);
            return AVERROR_EOF;
        }
        if (!v->finished &&
            av_gettime_relative() - v->last_load_time >= reload_interval) {
            if ((ret = parse_playlist(c, v->url, v, NULL)) < 0) {
                if (ret != AVERROR_EXIT)
                    av_log(v->parent, AV_LOG_ERROR, "Failed to reload playlist %d\n",
                           v->index);
                return ret;
            }
            /* If we need to reload the playlist again below (if
             * there's still no more segments), switch to a reload
             * interval of half the target duration. */
            reload_interval = v->target_duration / 2;
        }
        ret = read_data_correct_seq_no(c, v);
        if (ret < 0) {
            av_log(v->parent, AV_LOG_ERROR,
                "[%d] return 0x%x by seq number\n", v->index, ret);
            return ret;
        }

        if (v->cur_seq_no >= v->start_seq_no + v->n_segments) {
            if (v->finished)
                return AVERROR_EOF;
            while (av_gettime_relative() - v->last_load_time < reload_interval) {
                if (ff_check_interrupt(c->interrupt_callback))
                    return AVERROR_EXIT;
                av_usleep(100*1000);
            }
            /* Enough time has elapsed since the last reload */
            goto reload;
        }

        v->input_read_done = 0;
        seg = current_segment(v);
        /* load/update Media Initialization Section, if any */
        ret = update_init_section(v, seg);
        if (ret)
            return ret;

        if (1 == c->http_multiple && 1 == v->input_next_requested) {
            ret = 0;
            read_data_swap_ioctx(c, v);
        } else if (1 == c->http_multiple && -1 == v->input_next_requested) {
            segment_retries++;
            goto reload;
        } else {
            ret = read_data_open_current(c, v, seg);
        }

        if (ret < 0) {
            if (ff_check_interrupt(c->interrupt_callback))
                return AVERROR_EXIT;
            av_log(v->parent, AV_LOG_ERROR, "Failed to open segment %"PRId64" of playlist %d\n",
                   v->cur_seq_no,
                   v->index);
            if (segment_retries >= c->seg_max_retry) {
                av_log(v->parent, AV_LOG_ERROR, "Segment %"PRId64" of playlist %d failed too many times, skipping\n",
                       v->cur_seq_no,
                       v->index);
                v->cur_seq_no++;
                segment_retries = 0;
            } else {
                segment_retries++;
            }
            goto reload;
        }
        segment_retries = 0;
        just_opened = 1;
    }

    read_data_get_http_multiple(c, v);
    ret = read_data_open_next(c, v);
    if (ret < 0 && (!v->input || v->input_read_done)) { /* No input now, continue read current */
        av_log(v->parent, AV_LOG_ERROR, "[%d] open next fail\n", v->index);
        return ret;
    }
    ret = read_data_read(c, v, buf, buf_size, just_opened);
    if (ret > 0) {
        return ret;
    }
    /* we should not read much data when init,
     * especially for https which will take more time,
     * thus we think one segment will be ok.
     */
    if (AVERROR_INPUT_CHANGED == ret          ||
        ADAPTIVE_STAGE_INIT == c->async.stage ||
        PLS_OPEN_DEMUX == v->needed) {
        av_log(v->parent, AV_LOG_ERROR,
            "[%d] return eof by needed:%d ret:0x%x stage:%d\n",
                v->index, v->needed, ret, c->async.stage);
        return AVERROR_EOF;
    }
    goto restart;
}

static int read_data(void *opaque, uint8_t *buf, int buf_size)
{
    int ret = 0;
    int retry_cnt = 0;
    struct playlist *v = opaque;
    HLSContext *c = v->parent->priv_data;
    int server_error_code = 0; /* some server not return error code after retry */

    if (!buf || buf_size <= 0) {
        return 0;
    }

    do {
        ret = read_data_l(c, v, buf, buf_size);
        if (ret >= 0 || v->finished || AVERROR_EXIT == ret || AVERROR_EOF == ret) {
            break;
        }
        if (!server_error_code) {
            server_error_code = ret;
        }
    }  while (++retry_cnt < 3);

    if (v->input &&
        PLS_NEED_YES == v->needed &&
        AVMEDIA_TYPE_VIDEO == v->media_type) {
        int64_t qos_net_bitrate = 0;
        if (av_opt_get_int(v->input,
            "qos_net_bitrate", AV_OPT_SEARCH_CHILDREN, &qos_net_bitrate) >= 0) {
            c->qos_net_bitrate = qos_net_bitrate;
        }
    }
    if (ret < 0 && PLS_NEED_YES == v->needed) {
        av_log(v->parent, AV_LOG_ERROR,
            "[%d]Read data return 0x%x by %s needed:%d stage:%d url:%s\n",
                v->index, ret, av_err2str(ret), v->needed, c->async.stage, v->url);
        ff_adaptive_report_server_error(v->parent, server_error_code, ret);
    }
    return ret;
}

static void add_renditions_to_variant(HLSContext *c, struct variant *var,
                                      enum AVMediaType type, const char *group_id)
{
    int i;

    int pls_needed = PLS_NEED_NO;
    for (i = 0; i < c->n_renditions; i++) {
        struct rendition *rend = c->renditions[i];

        if (rend->type == type && !strcmp(rend->group_id, group_id)) {

            if (rend->playlist)
                /* rendition is an external playlist
                 * => add the playlist to the variant */
                dynarray_add(&var->playlists, &var->n_playlists, rend->playlist);
            else
                /* rendition is part of the variant main Media Playlist
                 * => add the rendition to the main Media Playlist */
                dynarray_add(&var->playlists[0]->renditions,
                             &var->playlists[0]->n_renditions,
                             rend);
            rend->playlist->needed = pls_needed;
            if (PLS_NEED_NO == pls_needed &&
                AVMEDIA_TYPE_AUDIO == type) {
                pls_needed = PLS_PARSE_ASYNC;
            }
        }
    }
}

static void add_metadata_from_renditions(struct playlist *pls,
                                         enum AVMediaType type)
{
    int rend_idx = 0;
    int i;

    for (i = 0; i < pls->n_main_streams; i++) {
        AVStream *st = pls->main_streams[i];

        if (st->codecpar->codec_type != type)
            continue;

        for (; rend_idx < pls->n_renditions; rend_idx++) {
            struct rendition *rend = pls->renditions[rend_idx];

            if (rend->type != type)
                continue;

            if (rend->language[0])
                av_dict_set(&st->metadata, "language", rend->language, 0);
            if (rend->name[0])
                av_dict_set(&st->metadata, "comment", rend->name, 0);

            st->disposition |= rend->disposition;
        }
        if (rend_idx >=pls->n_renditions)
            break;
    }
}

static int find_timestamp_seek_segment(
    HLSContext *c, struct playlist *pls,
    int64_t timestamp, int64_t *seg_start_ts)
{
    int ret = 0;
    if (1 != pls->n_segments) {
        av_log(pls->parent, AV_LOG_ERROR,
            "Not one segment, should not here\n");
        return 0;
    }

    int seekable = pls->pb.seekable;
    pls->pb.seekable = 1;
    pls->ctx->iformat->flags |= AVFMT_NOGENSEARCH;
    ret = av_seek_frame(pls->ctx, -1, timestamp, 0);
    pls->pb.seekable = seekable;
    if (ret < 0) {
        av_log(pls->parent, AV_LOG_ERROR, "Seek segment timestamp %lldus fail\n", timestamp);
        return 0;
    }
    av_log(pls->parent, AV_LOG_INFO, "Seek segment timestamp:%lld ok\n", timestamp);
    if (seg_start_ts) {
        ret = read_packet_av_read(pls->ctx, c, pls);
        if (ret >= 0) {
            *seg_start_ts = av_rescale_q(
                pls->pkt.dts, get_timebase(pls), AV_TIME_BASE_Q);
        }
    }
    return (ret >= 0) ? 1 : 0;
}

/* if timestamp was in valid range: returns 1 and sets seq_no
 * if not: returns 0 and sets seq_no to closest segment */
static int find_timestamp_in_playlist(HLSContext *c, struct playlist *pls,
                                      int64_t timestamp, int64_t *seq_no,
                                      int64_t *seg_start_ts)
{
    int i;
    int64_t pos = c->first_timestamp == AV_NOPTS_VALUE ?
                  0 : c->first_timestamp;

    if (timestamp < pos) {
        *seq_no = pls->start_seq_no;
        return 0;
    }

    if (pls->finished && 1 == pls->n_segments) { /* single fragment mode */
        *seq_no = pls->start_seq_no;
        return find_timestamp_seek_segment(c, pls, timestamp, seg_start_ts);
    }

    for (i = 0; i < pls->n_segments; i++) {
        int64_t diff = pos + pls->segments[i]->duration - timestamp;
        if (diff > 0) {
            *seq_no = pls->start_seq_no + i;
            if (seg_start_ts) {
                *seg_start_ts = pos;
            }
            return 1;
        }
        pos += pls->segments[i]->duration;
    }

    *seq_no = pls->start_seq_no + pls->n_segments - 1;

    return 0;
}

static int64_t select_cur_seq_no(HLSContext *c, struct playlist *pls)
{
    int64_t seq_no;

    if (!pls->finished && !c->first_packet &&
        av_gettime_relative() - pls->last_load_time >= default_reload_interval(pls))
        /* reload the playlist since it was suspended */
        parse_playlist(c, pls->url, pls, NULL);

    /* If playback is already in progress (we are just selecting a new
     * playlist) and this is a complete file, find the matching segment
     * by counting durations. */
    if (pls->finished && c->cur_timestamp != AV_NOPTS_VALUE) {
        find_timestamp_in_playlist(c, pls, c->cur_timestamp, &seq_no, NULL);
        return seq_no;
    }

    if (!pls->finished) {
        if (!c->first_packet && /* we are doing a segment selection during playback */
            c->cur_seq_no >= pls->start_seq_no &&
            c->cur_seq_no < pls->start_seq_no + pls->n_segments)
            /* While spec 3.4.3 says that we cannot assume anything about the
             * content at the same sequence number on different playlists,
             * in practice this seems to work and doing it otherwise would
             * require us to download a segment to inspect its timestamps. */
            return c->cur_seq_no;

        /* If this is a live stream, start live_start_index segments from the
         * start or end */
        if (c->live_start_index < 0)
            seq_no = pls->start_seq_no + FFMAX(pls->n_segments +
                                            c->live_start_index, 0);
        else
            seq_no = pls->start_seq_no + FFMIN(c->live_start_index,
                                            pls->n_segments - 1);

        /* If #EXT-X-START in playlist, need to recalculate */
        if (pls->time_offset_flag && c->prefer_x_start) {
            int64_t start_timestamp;
            int64_t playlist_duration = 0;
            int64_t cur_timestamp = c->cur_timestamp == AV_NOPTS_VALUE ? 0 :
                                    c->cur_timestamp;

            for (int i = 0; i < pls->n_segments; i++)
                playlist_duration += pls->segments[i]->duration;

            /* If the absolute value of TIME-OFFSET exceeds
             * the duration of the playlist, it indicates either the end of the
             * playlist (if positive) or the beginning of the playlist (if
             * negative). */
            if (pls->start_time_offset >=0 &&
                pls->start_time_offset > playlist_duration)
                start_timestamp = cur_timestamp + playlist_duration;
            else if (pls->start_time_offset >= 0 &&
                        pls->start_time_offset <= playlist_duration)
                start_timestamp = cur_timestamp + pls->start_time_offset;
            else if (pls->start_time_offset < 0 &&
                        pls->start_time_offset < -playlist_duration)
                start_timestamp = cur_timestamp;
            else if (pls->start_time_offset < 0 &&
                        pls->start_time_offset > -playlist_duration)
                start_timestamp = cur_timestamp + playlist_duration +
                                    pls->start_time_offset;
            else
                start_timestamp = cur_timestamp;

            find_timestamp_in_playlist(c, pls, start_timestamp, &seq_no, NULL);
        }
        return seq_no;
    }

    /* Otherwise just start on the first segment. */
    return pls->start_seq_no;
}

static int save_avio_options(AVFormatContext *s)
{
    HLSContext *c = s->priv_data;
    static const char * const opts[] = {
        "headers", "http_proxy", "user_agent", "cookies", "referer", "rw_timeout", NULL };
    const char * const * opt = opts;
    uint8_t *buf;
    int ret = 0;

    while (*opt) {
        if (av_opt_get(s->pb, *opt, AV_OPT_SEARCH_CHILDREN | AV_OPT_ALLOW_NULL, &buf) >= 0) {
            ret = av_dict_set(&c->avio_opts, *opt, buf,
                              AV_DICT_DONT_STRDUP_VAL);
            if (ret < 0)
                return ret;
        }
        opt++;
    }

    return ret;
}

static int nested_io_open(AVFormatContext *s, AVIOContext **pb, const char *url,
                          int flags, AVDictionary **opts)
{
    av_log(s, AV_LOG_ERROR,
           "A HLS playlist item '%s' referred to an external file '%s'. "
           "Opening this file was forbidden for security reasons\n",
           s->url, url);
    return AVERROR(EPERM);
}

static int variants_can_change(HLSContext *c)
{
    return c->n_variants >= 2;
}

static void add_stream_to_programs(AVFormatContext *s, struct playlist *pls, AVStream *stream)
{
    HLSContext *c = s->priv_data;
    int i, j;
    int bandwidth = -1;

    for (i = 0; i < c->n_variants; i++) {
        struct variant *v = c->variants[i];

        for (j = 0; j < v->n_playlists; j++) {
            if (v->playlists[j] != pls)
                continue;

            av_program_add_stream_index(s, i, stream->index);

            if (bandwidth < 0)
                bandwidth = v->bandwidth;
            else if (bandwidth != v->bandwidth)
                bandwidth = -1; /* stream in multiple variants with different bandwidths */
        }
    }

    if (bandwidth >= 0)
        av_dict_set_int(&stream->metadata, "variant_bitrate", bandwidth, 0);
}

static int set_stream_info_from_input_stream(AVStream *st, struct playlist *pls, AVStream *ist)
{
    int err;

    err = avcodec_parameters_copy(st->codecpar, ist->codecpar);
    if (err < 0)
        return err;

    if (pls->is_id3_timestamped) /* custom timestamps via id3 */
        avpriv_set_pts_info(st, 33, 1, MPEG_TIME_BASE);
    else
        avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);

    st->internal->need_context_update = 1;

    /** For ts only */
    size_t side_data_size = 0;
    uint8_t *side_data = av_stream_get_side_data(
        ist, AV_PKT_DATA_MPEGTS_CHINA_DRM_INFO, &side_data_size);
    if (!side_data || 0 == side_data_size || !(pls->encryption_info)) {
        return 0;
    }

    side_data =
        av_encryption_init_info_add_side_data(
            pls->encryption_info, &side_data_size);
    if (!side_data) {
        av_log(NULL, AV_LOG_ERROR, "HLS add encryption init info side data error, sz:%d data:%s\n", side_data_size, side_data);
        return AVERROR(ENOMEM);
    }

    err = av_stream_add_side_data(st, AV_PKT_DATA_ENCRYPTION_INIT_INFO, side_data, side_data_size);
    if (err < 0) {
        av_free(side_data);
        return AVERROR(EINVAL);
    }
    return 0;
}

static enum AVMediaType update_streams_find_avmedia_info(AVFormatContext *s,
    struct playlist *pls, int *width, int *height, AVRational *framerate)
{
    int i;
    enum AVMediaType codec_type = AVMEDIA_TYPE_UNKNOWN;
    for (i = 0; i < pls->n_main_streams; i++) {
        AVStream *st = pls->main_streams[i];
        AVCodecParameters *codecpar = st->codecpar;
        if (AVMEDIA_TYPE_VIDEO == codecpar->codec_type) {
            codec_type = AVMEDIA_TYPE_VIDEO;
            *width      = codecpar->width  > 0 ? codecpar->width  : pls->width;
            *height     = codecpar->height > 0 ? codecpar->height : pls->height;
            *framerate  = av_guess_frame_rate(pls->ctx, st, NULL);
            if (pls->width > 0 && pls->height > 0 &&
                (framerate->num <= 0 || framerate->den <= 0)) {
                *framerate = pls->framerate;
            }
        } else if (AVMEDIA_TYPE_VIDEO != codec_type &&
                   AVMEDIA_TYPE_AUDIO == codecpar->codec_type) {
            codec_type = AVMEDIA_TYPE_AUDIO;
        } else if (AVMEDIA_TYPE_VIDEO != codec_type &&
                   AVMEDIA_TYPE_AUDIO != codec_type &&
                   AVMEDIA_TYPE_SUBTITLE == codecpar->codec_type) {
            codec_type = AVMEDIA_TYPE_SUBTITLE;
        }
    }

    return codec_type;
}

static int update_streams_new_adaptive(AVFormatContext *s,
    struct playlist *pls, AVAdaptiveList *list, int update)
{
    int ret;
    int width = 0, height = 0;
    AVRational framerate = (AVRational){ 0, 1};
    enum AVMediaType media_type =
        update_streams_find_avmedia_info(
            s, pls, &width, &height, &framerate);
    if (AVMEDIA_TYPE_UNKNOWN == media_type) {
        return 0;
    }

    HLSContext *c = s->priv_data;
    /* c->n_variants < 2 : No playlist can switch */
    if (!variants_can_change(c) || !update) {
        pls->media_type = media_type;
        return 0;
    }

    if (!pls->adaptive && AVMEDIA_TYPE_VIDEO == media_type) {
        pls->adaptive =
            ff_adaptive_playlist_context_new(s, media_type, pls->finished ? 0 : 1);
        ff_adaptive_playlist_context_open(s, pls->adaptive, pls->n_segments);
    }

    if (!list) {
        AVAdaptiveList ilist = {
            .enable         = 1,
            .playlist_index = pls->index,
            .type       = media_type,
            .nb_streams = pls->n_main_streams,
            .width  = width,
            .height = height,
            .bandwidth = pls->bandwidth,
            .framerate = framerate,
            .streams   = pls->main_streams,
        };
        pls->media_type  = media_type;
        if ((ret = ff_adaptive_list_init(s, &ilist)) < 0) {
            return ret;
        }
    } else if (media_type != pls->media_type && list) {
        pls->media_type  = media_type;
        list->streams    = pls->main_streams;
        list->nb_streams = pls->n_main_streams;
    }
    return 0;
}

/* called before update_streams_from_subdemuxer */
static AVAdaptiveList *update_streams_find_adaptive_list(
                    AVFormatContext *s, struct playlist *pls)
{
    /* List not exist or not change, no need search */
    if (!pls->n_main_streams  ||
        !pls->ctx->nb_streams ||
        pls->n_main_streams == pls->ctx->nb_streams) {
        return NULL;
    }

    AVAdaptiveList *list = NULL;
    for (int i = 0; i < s->adaptive.n_list_num; i++) {
        list = &s->adaptive.lists[i];
        if (list->streams == pls->main_streams) {
            return list;
        }
    }
    return NULL;
}

/* add new subdemuxer streams to our context, if any */
static int update_streams_from_subdemuxer(AVFormatContext *s, struct playlist *pls)
{
    int err;
    int update = (int) (pls->n_main_streams != pls->ctx->nb_streams);
    /* update exposed playlist beacause new stream added in file such as ts */
    AVAdaptiveList *list =
        update_streams_find_adaptive_list(s, pls);

    while (pls->n_main_streams < pls->ctx->nb_streams) {
        int ist_idx = pls->n_main_streams;
        AVStream *st = avformat_new_stream(s, NULL);
        AVStream *ist = pls->ctx->streams[ist_idx];

        if (!st)
            return AVERROR(ENOMEM);

        st->id = pls->index;
        dynarray_add(&pls->main_streams, &pls->n_main_streams, st);

        add_stream_to_programs(s, pls, st);

        err = set_stream_info_from_input_stream(st, pls, ist);
        if (err < 0)
            return err;
    }

    (void) update_streams_new_adaptive(s, pls, list, update);
    return 0;
}

static void update_noheader_flag(AVFormatContext *s)
{
    HLSContext *c = s->priv_data;
    int flag_needed = 0;
    int i;

    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];

        if (pls->has_noheader_flag) {
            flag_needed = 1;
            break;
        }
    }

    if (flag_needed)
        s->ctx_flags |= AVFMTCTX_NOHEADER;
    else
        s->ctx_flags &= ~AVFMTCTX_NOHEADER;
}

static int hls_close(AVFormatContext *s)
{
    HLSContext *c = s->priv_data;
    ff_adaptive_async_deinit_ctx(&c->async);

    free_playlist_list(c);
    free_variant_list(c);
    free_rendition_list(c);

    av_dict_free(&c->avio_opts);
    close_format_io(c->ctx, NULL, &c->playlist_pb);
    ff_adaptive_list_free(s);
    return 0;
}

static int64_t updates_loop_get_reload_interval(HLSContext *c)
{
    int i;
    int64_t reload_interval = SLOW_CLOCK_UPDATE_INTERVAL;
    for (i = 0; i < c->n_playlists; i++) {
        int64_t interval =
            default_reload_interval(c->playlists[i]);
        if (interval < reload_interval) {
            reload_interval = interval;
        }
    }
    return reload_interval;
}

static void update_loop(
    AVFormatContext *s, HLSContext *c)
{
    int i, ret;
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];

        PLAYLIST_LOCK(c, pls);
        if (pls->needed > PLS_NEED_NO && !pls->finished &&
            av_gettime_relative() - pls->last_load_time >= default_reload_interval(pls)) {
            pls->m3u8_hold_counters = 0;
            if ((ret = parse_playlist(c, pls->url, pls, NULL)) < 0) {
                av_log(s, AV_LOG_WARNING, "Could not update the playlist, ret:%s [%s]\n", av_err2str(ret), pls->url);
                pls->broken = 1;
            } else {
                av_log(s, AV_LOG_INFO, "Updated playlist [%s] successfully\n", pls->url);
                if (pls->broken) {
                    pls->broken = 0;
                }
            }
        }
        PLAYLIST_UNLOCK(c, pls);
    }
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        if (pls->needed > PLS_NEED_NO && !pls->finished) {
            av_log(s, AV_LOG_INFO, "Playlist %s segments:\n", pls->url);
            for (int j = 0; j < pls->n_segments; j++) {
                av_log(s, AV_LOG_INFO, "[%d] %s\n", j, pls->segments[j]->url);
            }
        }
    }
}

static void *updates_loop_thread(void *arg)
{
    int ret = 0;
    AVFormatContext *s = arg;
    HLSContext *c = s->priv_data;

    return 0;

    ff_thread_setname("Hls Update");
    av_log(s, AV_LOG_INFO, "Started updates task\n");
    int64_t update_interval_usec =
        updates_loop_get_reload_interval(c);
    //c->async.use_thread = 0;
    while (c->async.stage < ADAPTIVE_STAGE_INIT_FAILED &&
            (!ff_check_interrupt(c->interrupt_callback))) {
        if (c->async.stage <= ADAPTIVE_STAGE_INIT) {
            av_usleep(10 * 1000);
            continue;
        }

        av_log(s, AV_LOG_INFO, "Updating playlist\n");
        int64_t start_time = av_gettime();
        update_loop(s, c);

        int64_t sleep_usec = update_interval_usec - (av_gettime() - start_time);
        /* cannot sleep longer when need exit */
        if (sleep_usec > 0) {
            av_log(s, AV_LOG_INFO, "Updating playlist sleep:%lld interval:%lld\n", sleep_usec, update_interval_usec);
            int sleep_cnt = (int) (sleep_usec / AV_TIME_BASE);
            av_usleep((unsigned) sleep_usec % AV_TIME_BASE);
            while (sleep_cnt-- > 0 &&
                c->async.stage < ADAPTIVE_STAGE_INIT_FAILED &&
                (!ff_check_interrupt(c->interrupt_callback))) {
                /* sleep one second */
                av_usleep(AV_TIME_BASE);
            }
        }
    }
    av_log(s, AV_LOG_INFO, "Stop updates task request detected\n");
    return NULL;
}

static void playlist_stream_inject_side_data(
    AVFormatContext *ctx, AVEncryptionInitInfo *encryption_info)
{
    /* ts not allow */
    if (strstr(ctx->iformat->name, "mpegts")) {
        return;
    }

    int inject_global_side_data = 0;
    for (unsigned int i = 0; i < ctx->nb_streams; i++) {
        AVStream *st = ctx->streams[i];
        (void) ff_decrypt_comp_merge_init_info(ctx->streams[i], encryption_info);
        if (av_stream_get_side_data(st,
            AV_PKT_DATA_ENCRYPTION_INIT_INFO, NULL)) {
            st->inject_global_side_data = 1;
            st->need_parsing = AVSTREAM_PARSE_NONE;
            inject_global_side_data = 1;
        }
    }
    if (inject_global_side_data) {
        ctx->internal->inject_global_side_data = 1;
    }
}

static int read_header_open_demux_input(
    AVFormatContext *s, HLSContext *c, struct playlist *pls)
{
    int ret;
    ff_const59 AVInputFormat *in_fmt = NULL;

    pls->read_buffer = av_malloc(INITIAL_BUFFER_SIZE);
    if (!pls->read_buffer){
        ret = AVERROR(ENOMEM);
        avformat_free_context(pls->ctx);
        pls->ctx = NULL;
        goto fail;
    }
    if (!pls->finished) {
        ffio_init_context(&pls->pb, pls->read_buffer, INITIAL_BUFFER_SIZE, 0, pls, read_data, NULL, NULL);
    } else {
        ffio_init_context(&pls->pb, pls->read_buffer, INITIAL_BUFFER_SIZE, 0, pls, read_data, NULL, seek_data);
    }
    pls->pb.seekable = 0;
    pls->ctx->probesize = s->probesize > 0 ? s->probesize : 1024 * 4;
    pls->ctx->max_analyze_duration = s->max_analyze_duration > 0 ? s->max_analyze_duration : 4 * AV_TIME_BASE;
    pls->ctx->interrupt_callback = s->interrupt_callback;
    ret = av_probe_input_buffer(&pls->pb, &in_fmt, pls->segments[0]->url, NULL, 0, 0);
    if (ret < 0) {
        /* Free the ctx - it isn't initialized properly at this point,
         * so avformat_close_input shouldn't be called. If
         * avformat_open_input fails below, it frees and zeros the
         * context, so it doesn't need any special treatment like this. */
        av_log(s, AV_LOG_ERROR, "Error when loading first segment '%s'\n", pls->segments[0]->url);
        avformat_free_context(pls->ctx);
        pls->ctx = NULL;
        goto fail;
    }
    pls->ctx->pb       = &pls->pb;
    pls->ctx->io_open  = nested_io_open;
    pls->ctx->flags   |= s->flags & ~AVFMT_FLAG_CUSTOM_IO;

    if ((ret = ff_copy_whiteblacklists(pls->ctx, s)) < 0)
        goto fail;

    ret = avformat_open_input(&pls->ctx, pls->segments[0]->url, in_fmt, NULL);
    if (ret < 0)
        goto fail;
    if (DRM_SYSTEM_TYPE_CHINADRM == pls->drm_sys_type) {
        playlist_stream_inject_side_data(pls->ctx, pls->encryption_info);
    }
    if (pls->id3_deferred_extra && pls->ctx->nb_streams == 1) {
        ff_id3v2_parse_apic(pls->ctx, &pls->id3_deferred_extra);
        avformat_queue_attached_pictures(pls->ctx);
        ff_id3v2_parse_priv(pls->ctx, &pls->id3_deferred_extra);
        ff_id3v2_free_extra_meta(&pls->id3_deferred_extra);
        pls->id3_deferred_extra = NULL;
    }

    if (pls->is_id3_timestamped == -1)
        av_log(s, AV_LOG_WARNING, "No expected HTTP requests have been made\n");
    if (!strcmp(pls->ctx->iformat->name, "mpegts") && PLS_NEED_YES == pls->needed) {
        int cutdown_probe_size = 1;
        for (int i = 0; i < pls->ctx->nb_streams; i++) {
            enum AVCodecID codec_id =
                pls->ctx->streams[i]->codecpar->codec_id;
            if (AV_CODEC_ID_AAC      != codec_id &&
                AV_CODEC_ID_AAC_LATM != codec_id &&
                AV_CODEC_ID_H264     != codec_id &&
                AV_CODEC_ID_HEVC     != codec_id) {
                cutdown_probe_size = 0;
            }
        }
        if (cutdown_probe_size) {
            pls->ctx->probesize = 4 * 1024;
        }
        pls->ctx->max_analyze_duration = AV_TIME_BASE;
    }
    /*
     * For ID3 timestamped raw audio streams we need to detect the packet
     * durations to calculate timestamps in fill_timing_for_id3_timestamped_stream(),
     * but for other streams we can rely on our user calling avformat_find_stream_info()
     * on us if they want to.
     */
    if (pls->is_id3_timestamped ||
        PLS_NEED_YES == pls->needed ||
        (pls->n_renditions > 0 && pls->renditions[0]->type == AVMEDIA_TYPE_AUDIO)) {
        ret = avformat_find_stream_info(pls->ctx, NULL);
        if (ret < 0)
            goto fail;
    }
fail:
    return ret;
}

static int read_header_open_demux_async(
    AVFormatContext *s, HLSContext *c)
{
    int ret = 0, i;
    for (i = 0; i < c->n_playlists; i++) {
        if (ff_check_interrupt(c->interrupt_callback)) {
            ret = AVERROR_EXIT;
            goto fail;
        }

        struct playlist *pls = c->playlists[i];
        if (pls->ctx) {
            continue;
        }

        if (!(pls->ctx = avformat_alloc_context())) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        if (pls->n_segments == 0)
            continue;

        pls->needed = PLS_OPEN_DEMUX;
        /*
         * If this is a live stream and this playlist looks like it is one segment
         * behind, try to sync it up so that every substream starts at the same
         * time position (so e.g. avformat_find_stream_info() will see packets from
         * all active streams within the first few seconds). This is not very generic,
         * though, as the sequence numbers are technically independent.
         */
        if (!pls->finished && pls->cur_seq_no == c->highest_cur_seq_no - 1 &&
            c->highest_cur_seq_no < pls->start_seq_no + pls->n_segments) {
            pls->cur_seq_no = c->highest_cur_seq_no;
        }

        /* use external demux, olny need the pls. */
        if (1 == s->demux_by_user) {
            continue;
        }

        PLAYLIST_LOCK(c, pls);
        ret = read_header_open_demux_input(s, c, pls);
        PLAYLIST_UNLOCK(c, pls);

        if (ret < 0) {
            goto fail;
        }

        pls->has_noheader_flag = !!(pls->ctx->ctx_flags & AVFMTCTX_NOHEADER);
    }

    c->async.stage = ADAPTIVE_STAGE_EXPOSE_STREAM;
    return 0;
fail:
    if (ret) {
        c->async.stage = ADAPTIVE_STAGE_INIT_FAILED;
    }
    return ret;
}

static AVIOContext *read_header_open_io(HLSContext *c, char *url)
{
    AVDictionary *opts = NULL;
    av_dict_copy(&opts, c->avio_opts, 0);

    if (c->http_persistent)
        av_dict_set(&opts, "multiple_requests", "1", 0);

    AVIOContext *open_io = NULL;
    (void) c->ctx->io_open(c->ctx, &open_io, url, AVIO_FLAG_READ, &opts);
    av_dict_free(&opts);

    return open_io;
}

static void read_header_async_parse_playlist(AVFormatContext *s, HLSContext *c)
{
    int ret;
    AVIOContext *in = NULL;
    for (int i = 0; i < c->n_playlists; i++) {
        if (ff_check_interrupt(c->interrupt_callback)) {
            break;
        }
        struct playlist *pls = c->playlists[i];
        if (PLS_PARSE_ASYNC != pls->needed) {
            continue;
        }
        if (!in) {
            in = read_header_open_io(c, pls->url);
        } else if (c->http_persistent) {
            ret = open_url_keepalive(c->ctx, &in, pls->url);
            if (ret == AVERROR_EXIT) {
                return;
            } else if (ret < 0) {
                if (ret != AVERROR_EOF)
                    av_log(c->ctx, AV_LOG_WARNING,
                        "Parsing playlist error %s ret:%s\n", pls->url, av_err2str(ret));
                in = read_header_open_io(c, pls->url);
                if (!in) {
                    continue;
                }
            }
        }

        pls->m3u8_hold_counters = 0;
        if ((ret = parse_playlist(c, pls->url, pls, in)) < 0) {
            av_log(s, AV_LOG_ERROR, "async parse_playlist error %s [%s]\n", av_err2str(ret), pls->url);
            pls->broken = 1;
        } else {
            av_log(s, AV_LOG_INFO, "[%d] async parse_playlist ok\n", i);
        }
        if (!c->http_persistent) {
            close_format_io(c->ctx, NULL, &in);
        }
    }
    close_format_io(c->ctx, NULL, &in);
}

static void *read_header_thread(void *arg)
{
    int ret;
    AVFormatContext *s = arg;
    HLSContext *c = s->priv_data;

    ff_thread_setname("Hls Async Parser");
    while (!is_short_video(s)                         &&
           !ff_check_interrupt(c->interrupt_callback) &&
           ADAPTIVE_STAGE_CLOSE != c->async.stage     &&
           ADAPTIVE_STAGE_ASYNC_READ_HEADER != c->async.stage) {
       /* Not occupy cpu */
       av_usleep(1000);
    }

    /* short video no need parse */
    if (is_short_video(s) ||
        ADAPTIVE_STAGE_CLOSE == c->async.stage ||
        ff_check_interrupt(c->interrupt_callback)) {
        return NULL;
    }

    while (!ff_adaptive_query_play_state(s) &&
        !ff_check_interrupt(c->interrupt_callback)) {
        av_log(s, AV_LOG_VERBOSE, "Wait player ready!\n");
        /* Not use more cpu */
        av_usleep(1000);
    }

    av_log(s, AV_LOG_INFO, "HLS playlist async read header start\n");
    /* If the playlist only contained playlists (Master Playlist),
     * parse each individual playlist. */
    if (c->n_playlists > 1 || c->playlists[0]->n_segments == 0) {
        read_header_async_parse_playlist(s, c);
    }
    ret = read_header_open_demux_async(s, s->priv_data);
    /* error send async done ahead */
    if (ret) {
        ff_adaptive_async_done(s);
    }
    av_log(s, AV_LOG_INFO, "HLS playlist async read header completely, ret:%d\n", ret);
    return NULL;
}

static int enter_read_header_thread(AVFormatContext *s)
{
    HLSContext *c = s->priv_data;

    if (!c->use_thread) {
        return 0;
    }

    AdaptiveAsync *async = &c->async;
    for (int i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        av_log(s, AV_LOG_DEBUG, "[%d]ctx:%p finish:%d need:%d\n", i, pls->ctx, pls->finished, pls->needed);
        if (!pls->finished &&
            PLS_PARSE_ASYNC != pls->needed) {
            c->async.is_live = 1;
        }
    }

    /* return 0 if success */
    int ret = pthread_create(&async->init_thread, NULL, read_header_thread, s);
    if (ret) {
        c->use_thread = 0;
    }
    return ret;
}

static int open_demux_playlist_in_variant(
    AVFormatContext *s, int variant_id, struct playlist *pls)
{
    HLSContext *c = s->priv_data;

    if (!c->use_thread) {
        return 1;
    }

    struct variant *v = c->variants[variant_id];
    for (int i = 0; i < v->n_playlists; i++) {
        if (pls == v->playlists[i] &&
            PLS_PARSE_ASYNC != pls->needed) {
            return 1;
        }
    }
    return 0;
}

static int open_demux_select_first_playlist(AVFormatContext *s, int variant_id)
{
    struct playlist *playlist = NULL;
    HLSContext *c = s->priv_data;
    struct variant *v = c->variants[variant_id];

    for (int i = 0; i < v->n_playlists; i++) {
        playlist = v->playlists[i];
        if (PLS_PARSE_ASYNC != playlist->needed &&
            AVMEDIA_TYPE_VIDEO == playlist->media_type) {
            break;
        }
    }

    if (!playlist) {
        return AVERROR(EINVAL);
    }
    for (int i = 0; i < c->n_playlists; i++) {
        if (playlist == c->playlists[i]) {
            return i;
        }
    }

    return AVERROR(EINVAL);
}

static void open_demux_update_seekable_flag(AVFormatContext *s, HLSContext *c)
{
    int i, seekable = 1;
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        if (!pls->finished &&
            is_playlist_normal(pls)) {
            seekable = 0;
        }
    }
    s->pb->seekable = seekable;
    av_log(s, AV_LOG_INFO, "Open demux update seekable flag:%d\n", seekable);
}

static int reader_header_sel_start_var(AVFormatContext *s, HLSContext *c)
{
    if (!variants_can_change(c)) {
        return 0;
    }

    int bw  = INT_MAX;
    int sel = (c->n_variants / 4);
    int max_diff = 0x7FFFFFF;
    for (int i = 0; i < c->n_variants; i++) {
        struct variant *v = c->variants[i];
        if (v->bandwidth <= 0                   ||
            v->media_type != AVMEDIA_TYPE_VIDEO ||
            !(is_prior_codec(v->codec))) {
            continue;
        } else if (abs(v->bandwidth - VIDEO_CHOOSED_BANDWIDTH) < max_diff) {
            bw  = v->bandwidth;
            max_diff = abs(v->bandwidth - VIDEO_CHOOSED_BANDWIDTH);
            sel = i;
        }
    }
    return sel;
}

static int read_header_open_playlist_demux(
    AVFormatContext *s, HLSContext *c, int start_variant, int playlist_index)
{
    int ret;
    struct playlist *pls = c->playlists[playlist_index];
    pls->index  = playlist_index;
    pls->parent = s;
    if (pls->ctx ||
        !open_demux_playlist_in_variant(s, start_variant, pls)) {
        return 0;
    }

    if (!(pls->ctx = avformat_alloc_context())) {
        return AVERROR(ENOMEM);
    }

    if (pls->n_segments == 0) {
        return 0;
    }
    pls->needed = PLS_NEED_YES;
    /*
     * If this is a live stream and this playlist looks like it is one segment
     * behind, try to sync it up so that every substream starts at the same
     * time position (so e.g. avformat_find_stream_info() will see packets from
     * all active streams within the first few seconds). This is not very generic,
     * though, as the sequence numbers are technically independent.
     */
    if (!pls->finished &&
        c->highest_cur_seq_no < pls->start_seq_no + pls->n_segments) {
        pls->cur_seq_no = c->highest_cur_seq_no;
    }

    PLAYLIST_LOCK(c, pls);
    ret = read_header_open_demux_input(s, c, pls);
    PLAYLIST_UNLOCK(c, pls);

    if (ret < 0) {
        return ret;
    }

    pls->has_noheader_flag = !!(pls->ctx->ctx_flags & AVFMTCTX_NOHEADER);
    return 0;
}

static int read_header_parse_playlist_serial(
    AVFormatContext *s, int playlist_index)
{
    int ret = 0;
    HLSContext *c = s->priv_data;
    struct playlist *pls = c->playlists[playlist_index];
    /* have parsed */
    if (ADAPTIVE_STAGE_INIT == c->async.stage && (0 < pls->n_segments)) {
        return 0;
    }
    if ((ret = parse_playlist(c, pls->url, pls, NULL)) < 0) {
        av_log(s, AV_LOG_WARNING, "parse_playlist error %s [%s]\n", av_err2str(ret), pls->url);
        pls->broken = 1;
        if (1 == c->n_playlists) {
            return AVERROR(EINVAL);
        }
    }
    if (0 == pls->n_segments) {
        pls->broken = 1;
        if (1 == c->n_playlists) {
            return AVERROR(EINVAL);
        }
        return 0;
    }

    pls->cur_seq_no = select_cur_seq_no(c, pls);
    if (!c->highest_cur_seq_no) {
        c->highest_cur_seq_no = FFMAX(c->highest_cur_seq_no, pls->cur_seq_no);
    }
    return 0;
}

static int read_header_open_demux(
    AVFormatContext *s, HLSContext *c)
{
    int ret = 0, i;
    int start_variant =
        reader_header_sel_start_var(s, c);

    int has_audio = 0, has_video = 0;
    int first =
        open_demux_select_first_playlist(s, start_variant);
    if (first >= 0 && first < c->n_playlists) {
        ret = read_header_parse_playlist_serial(s, first);
        if (ret >= 0) {
            ret = read_header_open_playlist_demux(s, c, start_variant, first);
            if (ret < 0 || !(c->playlists[first]->ctx))
                goto fail;
            AVFormatContext *ic = c->playlists[first]->ctx;
            for (i = 0; i < ic->nb_streams; i++) {
                AVCodecParameters *codecpar = ic->streams[i]->codecpar;
                if (AVMEDIA_TYPE_AUDIO == codecpar->codec_type) {
                    has_audio = 1;
                }
                if (AVMEDIA_TYPE_VIDEO == codecpar->codec_type) {
                    has_video = 1;
                }
            }
        }
    }

    if (!has_audio || !has_video) {
        for (i = 0; i < c->n_playlists; i++) {
            if (i == first) {
                continue;
            }
            ret = read_header_parse_playlist_serial(s, i);
            if (ret < 0) {
                continue;
            }
            ret = read_header_open_playlist_demux(s, c, start_variant, i);
            AVFormatContext *ic = c->playlists[i]->ctx;
            if (ret < 0 || !ic)
                goto fail;

            for (int j = 0; j < ic->nb_streams; j++) {
                AVCodecParameters *codecpar = ic->streams[j]->codecpar;
                if (AVMEDIA_TYPE_AUDIO == codecpar->codec_type) {
                    has_audio = 1;
                }
                if (AVMEDIA_TYPE_VIDEO == codecpar->codec_type) {
                    has_video = 1;
                }
            }
            if (has_audio && has_audio) {
                break;
            }
        }
    }

    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        if (!pls->ctx || !pls->ctx->nb_streams || !pls->ctx->streams) {
            pls->index  = i;
            pls->parent = s;
            pls->needed = PLS_PARSE_ASYNC;
            continue;
        }
        /* Create new AVStreams for each stream in this playlist */
        ret = update_streams_from_subdemuxer(s, pls);
        if (ret < 0)
            goto fail;

        /*
         * Copy any metadata from playlist to main streams, but do not set
         * event flags.
         */
        if (pls->n_main_streams)
            av_dict_copy(&pls->main_streams[0]->metadata, pls->ctx->metadata, 0);

        add_metadata_from_renditions(pls, AVMEDIA_TYPE_AUDIO);
        add_metadata_from_renditions(pls, AVMEDIA_TYPE_VIDEO);
        add_metadata_from_renditions(pls, AVMEDIA_TYPE_SUBTITLE);
    }
fail:
    open_demux_update_seekable_flag(s, c);
    return ret;
}

static int hls_read_header(AVFormatContext *s)
{
    HLSContext *c = s->priv_data;
    int ret = 0, i;

    c->ctx                = s;
    c->interrupt_callback = &s->interrupt_callback;

    c->first_packet = 1;
    c->async.stage  = ADAPTIVE_STAGE_NONE;
    c->first_timestamp = AV_NOPTS_VALUE;
    c->cur_timestamp = AV_NOPTS_VALUE;
    c->async.stage   = ADAPTIVE_STAGE_NONE;
    c->async.is_live = 0;
    s->duration = AV_NOPTS_VALUE;
    c->async.use_thread = c->use_thread;
    c->highest_cur_seq_no = 0;
    if ((ret = save_avio_options(s)) < 0)
        goto fail;

    /* Some HLS servers don't like being sent the range header */
    av_dict_set(&c->avio_opts, "seekable", "0", 0);

    if ((ret = parse_playlist(c, s->url, NULL, s->pb)) < 0)
        goto fail;

    if (c->n_variants == 0) {
        av_log(s, AV_LOG_WARNING, "Empty playlist\n");
        ret = AVERROR_EOF;
        goto fail;
    }
    /* Associate renditions with variants */
    for (i = 0; i < c->n_variants; i++) {
        struct variant *var = c->variants[i];

        if (var->audio_group[0])
            add_renditions_to_variant(c, var, AVMEDIA_TYPE_AUDIO, var->audio_group);
        if (var->video_group[0])
            add_renditions_to_variant(c, var, AVMEDIA_TYPE_VIDEO, var->video_group);
        if (var->subtitles_group[0])
            add_renditions_to_variant(c, var, AVMEDIA_TYPE_SUBTITLE, var->subtitles_group);
    }
    int start_variant =
        reader_header_sel_start_var(s, c);
    /* If the playlist only contained playlists (Master Playlist),
     * parse each individual playlist. */
    if (c->n_playlists > 1 || c->playlists[0]->n_segments == 0) {
        for (i = 0; i < c->n_playlists; i++) {
            struct playlist *pls = c->playlists[i];
            pls->m3u8_hold_counters = 0;
            if (!open_demux_playlist_in_variant(s, start_variant, pls)) {
                pls->needed = PLS_PARSE_ASYNC;
            }
            pls->needed = PLS_NEED_NO;
        }
    }
    /* Create a program for each variant */
    for (i = 0; i < c->n_variants; i++) {
        struct variant *v = c->variants[i];
        AVProgram *program;

        program = av_new_program(s, i);
        if (!program)
            goto fail;
        av_dict_set_int(&program->metadata, "variant_bitrate", v->bandwidth, 0);
    }
    ret = ff_adaptive_async_init_ctx(s, &c->async, updates_loop_thread);
    if (ret) {
        c->use_thread = 0;
    }

    c->async.stage = ADAPTIVE_STAGE_INIT;
    /* Open the demuxer for each playlist */
    ret = read_header_open_demux(s, c);
    if (ret < 0) {
        goto fail;
    }
    /* If this isn't a live stream, calculate the total duration of the
     * stream. */
    if (c->variants[start_variant]->playlists[0]->finished) {
        int64_t duration = 0;
        for (i = 0; i < c->variants[start_variant]->playlists[0]->n_segments; i++)
            duration += c->variants[start_variant]->playlists[0]->segments[i]->duration;
        s->duration = duration;
    }

    if (!c->use_thread) {
        c->async.stage = ADAPTIVE_STAGE_ALL_STREAM_READY;
    }
    update_noheader_flag(s);
    return 0;
fail:
    hls_close(s);
    return ret;
}

static void read_headr_async_update(AVFormatContext *s, HLSContext *c)
{
    int i;
    for (i = 0; i < c->n_variants; i++) {
        if (c->variants[i]->playlists[0]->n_segments == 0) {
            av_log(s, AV_LOG_WARNING, "[%d][%d] Async empty segment [%s]\n",
                i, c->variants[i]->playlists[0]->index, c->variants[i]->playlists[0]->url);
            c->variants[i]->playlists[0]->broken = 1;
        } else {
            c->variants[i]->playlists[0]->broken = 0;
        }
    }
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        if (!pls->ctx || PLS_OPEN_DEMUX != pls->needed) {
            continue;
        }
        for (int j = 0; j < pls->n_main_streams; j++) {
            pls->main_streams[j]->discard = AVDISCARD_ALL;
        }
        pls->needed = PLS_NEED_NO;
        /* close io to avoid buffer data */
        if (pls->input) {
            close_format_io(pls->parent, pls, &pls->input);
        }
        pls->input_read_done = 0;
        if (pls->input_next) {
            close_format_io(pls->parent, NULL, &pls->input_next);
        }
        pls->input_next_requested = 0;
    }
}

static int read_header_expose_async_streams(
    AVFormatContext *s, HLSContext *c)
{
    int ret = 0, i;

    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];

        if (!pls->ctx || PLS_OPEN_DEMUX != pls->needed) {
            continue;
        }

        /* Create new AVStreams for each stream in this playlist */
        ret = update_streams_from_subdemuxer(s, pls);
        if (ret < 0)
            goto fail;

        /*
         * Copy any metadata from playlist to main streams, but do not set
         * event flags.
         */
        if (pls->n_main_streams)
            av_dict_copy(&pls->main_streams[0]->metadata, pls->ctx->metadata, 0);

        add_metadata_from_renditions(pls, AVMEDIA_TYPE_AUDIO);
        add_metadata_from_renditions(pls, AVMEDIA_TYPE_VIDEO);
        add_metadata_from_renditions(pls, AVMEDIA_TYPE_SUBTITLE);
    }
    read_headr_async_update(s, c);
fail:
    open_demux_update_seekable_flag(s, c);
    return ret;
}

static void recheck_change_playlist(
    AVFormatContext *s,
    struct playlist *pls, struct playlist *close)
{
    if (!pls || !close) {
        return;
    }
    /* Not allow jump */
    pls->cur_seq_no = close->cur_seq_no;

    av_packet_unref(&pls->pkt);
    reset_packet(&pls->pkt);
    pls->pb.eof_reached = 0;
    /* Clear any buffered data */
    pls->pb.buf_end = pls->pb.buf_ptr = pls->pb.buffer;
    /* Reset the pos, to let the mpegts demuxer know we've seeked. */
    pls->pb.pos = 0;
    /* Flush the packet queue of the subdemuxer. */
    ff_read_frame_flush(pls->ctx);
}

static int recheck_discard_flags(AVFormatContext *s)
{
    int cur_needed;
    HLSContext *c = s->priv_data;
    int i, changed = 0;
    struct playlist *open_pls  = NULL;
    struct playlist *close_pls = NULL;

    /* Check if any new streams are needed */
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];

        cur_needed = playlist_needed(c->playlists[i]);
        if (pls->broken) {
            continue;
        }

        if (PLS_NEED_YES == cur_needed && PLS_NEED_NO == pls->needed) {
            pls->needed = PLS_NEED_YES;
            changed = 1;
            pls->cur_seq_no = select_cur_seq_no(c, pls);
            pls->pb.eof_reached = 0;
            if (c->cur_timestamp != AV_NOPTS_VALUE) {
                /* video need key frame and switch on the boundry */
                if (1 == pls->ctx->nb_streams) {
                    AVStream *ist = pls->ctx->streams[0];
                    if (ist->discard < AVDISCARD_ALL &&
                        0 == pls->is_id3_timestamped &&
                        AVMEDIA_TYPE_AUDIO == ist->codecpar->codec_type) {
                        /* catch up */
                        pls->seek_timestamp = c->cur_timestamp;
                        pls->seek_flags = AVSEEK_FLAG_ANY;
                        pls->seek_stream_index = -1;
                    }
                }
            }
            open_pls = pls;
            av_log(s, AV_LOG_INFO, "Now receiving playlist %d, segment %d\n", i, pls->cur_seq_no);
        } else if (PLS_NEED_NO == cur_needed && PLS_NEED_YES == pls->needed) {
            if (pls->input)
                close_format_io(pls->parent, pls, &pls->input);
            pls->input_read_done = 0;
            if (pls->input_next)
                close_format_io(pls->parent, NULL, &pls->input_next);
            pls->input_next_requested = 0;
            pls->needed = PLS_NEED_NO;
            changed = 1;
            close_pls = pls;
            av_log(s, AV_LOG_INFO, "No longer receiving playlist %d, segment %d\n", i, pls->cur_seq_no);
        }
    }
    if (changed) {
        recheck_change_playlist(s, open_pls, close_pls);
    }
    return changed;
}

static void read_packet_recheck_discard_flags(AVFormatContext *s, int first)
{
    HLSContext *c = s->priv_data;
    AdaptiveAsync *async = &c->async;

    if (first && c->use_thread &&
        ADAPTIVE_STAGE_INIT == c->async.stage) {
        /* Postpone async read after read packet for low quilty network case */
        ff_adaptive_async_open(s);
        c->async.stage = ADAPTIVE_STAGE_ASYNC_READ_HEADER;
        (void) enter_read_header_thread(s);
    }

    if (ADAPTIVE_STAGE_ALL_STREAM_READY == async->stage) {
        /* send async done after stream got by player */
        if (async->need_send_async_done) {
            ff_adaptive_async_done(s);
            async->need_send_async_done = 0;
        }
    } else {
        if (ADAPTIVE_STAGE_EXPOSE_STREAM == async->stage) {
            int ret = read_header_expose_async_streams(s, s->priv_data);
            if (ret) {
                async->stage = ADAPTIVE_STAGE_INIT_FAILED;
            } else {
                async->stage = ADAPTIVE_STAGE_ALL_STREAM_READY;
                async->need_send_async_done = 1;
            }
        }
        return;
    }
    /* Not check when change playlist */
    if (c->need_change_playlist) {
        return;
    }
    recheck_discard_flags(s);
}

static void fill_timing_for_id3_timestamped_stream(struct playlist *pls)
{
    if (pls->id3_offset >= 0) {
        pls->pkt.dts = pls->id3_mpegts_timestamp +
                                 av_rescale_q(pls->id3_offset,
                                              pls->ctx->streams[pls->pkt.stream_index]->time_base,
                                              MPEG_TIME_BASE_Q);
        if (pls->pkt.duration) {
            pls->id3_offset += pls->pkt.duration;
        } else {
            pls->id3_offset = -1;
        }
    } else {
        /* there have been packets with unknown duration
         * since the last id3 tag, should not normally happen */
        pls->pkt.dts = AV_NOPTS_VALUE;
    }

    if (pls->pkt.duration)
        pls->pkt.duration = av_rescale_q(pls->pkt.duration,
                                         pls->ctx->streams[pls->pkt.stream_index]->time_base,
                                         MPEG_TIME_BASE_Q);
    pls->pkt.pts = AV_NOPTS_VALUE;
}

static int compare_ts_with_wrapdetect(int64_t ts_a, struct playlist *pls_a,
                                      int64_t ts_b, struct playlist *pls_b)
{
    int64_t scaled_ts_a = av_rescale_q(ts_a, get_timebase(pls_a), MPEG_TIME_BASE_Q);
    int64_t scaled_ts_b = av_rescale_q(ts_b, get_timebase(pls_b), MPEG_TIME_BASE_Q);

    return av_compare_mod(scaled_ts_a, scaled_ts_b, 1LL << 33);
}

static int read_hls_raw_stream(struct playlist *pls, AVPacket *pkt)
{
    int read_size = read_data(pls, pkt->data, pkt->size);
    if (read_size > 0) {
        int ret = 0;
        struct segment *seg = current_segment(pls);
        /* Seg read for the first time and send key info */
        if (seg && seg->key && ((int) pls->cur_seg_offset == read_size)) {
            uint8_t *data = NULL;

            av_log(pls->ctx, AV_LOG_DEBUG,
                "Download url:%s read size:%d seg off:%lld\n", seg->url, read_size, pls->cur_seg_offset);
            data = av_packet_new_side_data(pkt, AV_PKT_DATA_ENCRYPTION_INFO, sizeof(seg->key_type));
            if (data != NULL) {
                memcpy(data, &(seg->key_type), sizeof(seg->key_type));
            } else {
                ret = AVERROR(ENOMEM);
            }

            data = av_packet_new_side_data(pkt, AV_PKT_DATA_STRINGS_METADATA, strlen(seg->key) + 1);
            if (data != NULL) {
                memcpy(data, seg->key, strlen(seg->key) + 1);
            } else {
                ret = AVERROR(ENOMEM);
            }

            data = av_packet_new_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA, 16);
            if (data != NULL) {
                memcpy(data, &(seg->iv), 16);
            } else {
                ret = AVERROR(ENOMEM);
            }
            if (0 != ret) {
                av_log(pls->ctx, AV_LOG_ERROR,
                    "Meet start seg, buf no memory for key info\n");
                av_packet_free_side_data(pkt);
            }
        }
    }

    return read_size;
}

static int read_packet_av_read(
    AVFormatContext *s,
    HLSContext *c, struct playlist *pls)
{
    int ret = 0;
    while (1) {
        int64_t ts_diff;
        AVRational tb;
        ret = av_read_frame(pls->ctx, &pls->pkt);
        if (ret < 0) {
            if (PLS_NEED_CHANGE == pls->needed && c->need_change_playlist) {
                c->need_change_playlist = 0;
                pls->needed = PLS_NEED_YES;
                return ret;
            }
            if (!avio_feof(&pls->pb) && ret != AVERROR_EOF)
                return ret;
            reset_packet(&pls->pkt);
            ret = 0;
            break;
        } else {
            /* stream_index check prevents matching picture attachments etc. */
            if (pls->is_id3_timestamped && pls->pkt.stream_index == 0) {
                /* audio elementary streams are id3 timestamped */
                fill_timing_for_id3_timestamped_stream(pls);
            }

            if (c->first_timestamp == AV_NOPTS_VALUE &&
                pls->pkt.dts       != AV_NOPTS_VALUE)
                c->first_timestamp = av_rescale_q(pls->pkt.dts,
                    get_timebase(pls), AV_TIME_BASE_Q);
        }

        if (pls->seek_timestamp == AV_NOPTS_VALUE)
            break;

        if (pls->seek_stream_index < 0 ||
            pls->seek_stream_index == pls->pkt.stream_index) {

            if (pls->pkt.dts == AV_NOPTS_VALUE) {
                pls->seek_timestamp = AV_NOPTS_VALUE;
                break;
            }

            tb = get_timebase(pls);
            ts_diff = av_rescale_rnd(pls->pkt.dts, AV_TIME_BASE,
                                    tb.den, AV_ROUND_DOWN) -
                    pls->seek_timestamp;
            if (ts_diff >= 0 && (pls->seek_flags  & AVSEEK_FLAG_ANY ||
                                pls->pkt.flags & AV_PKT_FLAG_KEY)) {
                pls->seek_timestamp = AV_NOPTS_VALUE;
                break;
            }
        }
        av_packet_unref(&pls->pkt);
        reset_packet(&pls->pkt);
    }
    return ret;
}

/* Must be china drm mpegts container, AV_PKT_DATA_MPEGTS_CHINA_DRM_INFO exists in mpegts container */
static unsigned int china_drm_encryption_method2type(unsigned int method)
{
    if (0x01 == method) {
        return ENCRYPTION_SCHEME_SM4S; /* SM4-SAMPLE-CBC */
    } else if (0x02 == method) {
        return ENCRYPTION_SCHEME_CBCS; /* AES-SAMPLE-CBC */
    } else if (0x03 == method) {
        return ENCRYPTION_SCHEME_SM4C; /* SM4-CBC */
    } else if (0x04 == method) {
        return ENCRYPTION_SCHEME_SM4R; /* SAMPLE-SM4-CTR */
    } else if (0x05 == method) {
        return ENCRYPTION_SCHEME_CBC1; /* AES-CBC */
    } else if (0x06 == method) {
        return ENCRYPTION_SCHEME_SM4T; /* SM4-CTR */
    } else if (0x07 == method) {
        return ENCRYPTION_SCHEME_CENS; /* AES-CTR SubSample */
    } else if (0x08 == method) {
        return ENCRYPTION_SCHEME_CENC; /* AES-CTR Sample */
    }
    return ENCRYPTION_SCHEME_NONE;
}

static int inject_chinadrm_encryption_sidedata(
    AVFormatContext *s, AVStream *ist, AVPacket *pkt)
{
    size_t side_data_size = 0;
    enum AVMediaType codec_type = ist->codecpar->codec_type;
    uint8_t *side_data = av_stream_get_side_data(
        ist, AV_PKT_DATA_MPEGTS_CHINA_DRM_INFO, &side_data_size);
    if (!side_data || 0 == side_data_size ||
        (AVMEDIA_TYPE_AUDIO != codec_type && AVMEDIA_TYPE_VIDEO != codec_type)) {
        return 0;
    }

    ChinaDrmDescr descr = {0};
    int ret = ff_transport_parse_china_drm_descriptor(s, side_data, side_data_size, &descr);
    if (ret < 0) {
        return ret;
    }
    AVEncryptionInfo *info =
        av_encryption_info_alloc(1, 16, 16);
    if (!info) {
        return AVERROR(ENOMEM);
    }

    if (AVMEDIA_TYPE_AUDIO == ist->codecpar->codec_type) {
        info->scheme = china_drm_encryption_method2type(descr.audio_encryption_method);
    } else if (AVMEDIA_TYPE_VIDEO == ist->codecpar->codec_type) {
        info->scheme = china_drm_encryption_method2type(descr.video_encryption_method);
    }

    memset(info->iv, 0xFF, info->iv_size);
    memset(info->key_id, 0xFF, info->key_id_size);
    side_data =
        av_encryption_info_add_side_data(info, &side_data_size);

    av_encryption_info_free(info);
    if (!side_data) {
        return AVERROR(ENOMEM);
    }
    ret = av_packet_add_side_data(pkt,
        AV_PKT_DATA_ENCRYPTION_INFO, side_data, side_data_size);
    if (ret < 0) {
        av_free(side_data);
    }
    return ret;
}


static int read_packet_return_pkt(AVFormatContext *s,
    HLSContext *c, AVPacket *pkt, int minplaylist)
{
    AVStream *ist;
    AVStream *st;

    struct playlist *pls = c->playlists[minplaylist];
    int ret = update_streams_from_subdemuxer(s, pls);
    if (ret < 0) {
        av_packet_unref(&pls->pkt);
        reset_packet(&pls->pkt);
        return ret;
    }

    // If sub-demuxer reports updated metadata, copy it to the first stream
    // and set its AVSTREAM_EVENT_FLAG_METADATA_UPDATED flag.
    if (pls->ctx->event_flags & AVFMT_EVENT_FLAG_METADATA_UPDATED) {
        if (pls->n_main_streams) {
            st = pls->main_streams[0];
            av_dict_copy(&st->metadata, pls->ctx->metadata, 0);
            st->event_flags |= AVSTREAM_EVENT_FLAG_METADATA_UPDATED;
        }
        pls->ctx->event_flags &= ~AVFMT_EVENT_FLAG_METADATA_UPDATED;
    }

    /* check if noheader flag has been cleared by the subdemuxer */
    if (pls->has_noheader_flag && !(pls->ctx->ctx_flags & AVFMTCTX_NOHEADER)) {
        pls->has_noheader_flag = 0;
        update_noheader_flag(s);
    }

    if (pls->pkt.stream_index >= pls->n_main_streams) {
        av_log(s, AV_LOG_ERROR, "stream index inconsistency: index %d, %d main streams, %d subdemuxer streams\n",
               pls->pkt.stream_index, pls->n_main_streams, pls->ctx->nb_streams);
        av_packet_unref(&pls->pkt);
        reset_packet(&pls->pkt);
        return AVERROR_BUG;
    }

    ist = pls->ctx->streams[pls->pkt.stream_index];
    st = pls->main_streams[pls->pkt.stream_index];

    *pkt = pls->pkt;
    (void) inject_chinadrm_encryption_sidedata(s, ist, pkt);
    pkt->stream_index = st->index;
    reset_packet(&c->playlists[minplaylist]->pkt);

    if (pkt->dts != AV_NOPTS_VALUE)
        c->cur_timestamp = av_rescale_q(pkt->dts,
                                        ist->time_base,
                                        AV_TIME_BASE_Q);

    /* There may be more situations where this would be useful, but this at least
     * handles newly probed codecs properly (i.e. request_probe by mpegts). */
    if (ist->codecpar->codec_id != st->codecpar->codec_id) {
        ret = set_stream_info_from_input_stream(st, pls, ist);
        if (ret < 0) {
            av_packet_unref(pkt);
            return ret;
        }
    }

    return 0;
}

static int inline read_packet_by_av_read(
    AVFormatContext *s,
    HLSContext *c, struct playlist *pls)
{
    int ret = 0;
    PLAYLIST_LOCK(c, pls);
    ret = read_packet_av_read(s, c, pls);
    PLAYLIST_UNLOCK(c, pls);
    return ret;
}

static int hls_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    HLSContext *c = s->priv_data;
    int ret, i, minplaylist = -1;

    read_packet_recheck_discard_flags(s, c->first_packet);
    c->first_packet = 0;
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        /* Make sure we've got one buffered packet from each open playlist
         * stream */
        if (pls->needed > PLS_NEED_NO && !pls->pkt.data) {
            /* Read raw stream from server */
            if (1 == s->demux_by_user) {
                pkt->stream_index = i;
                return read_hls_raw_stream(pls, pkt);
            }

            ret = read_packet_by_av_read(s, c, pls);
            if (ret < 0) {
                return ret;
            }
        }
        /* Check if this stream has the packet with the lowest dts */
        if (pls->pkt.data) {
            struct playlist *minpls =
                minplaylist < 0 ? NULL : c->playlists[minplaylist];
            if (minplaylist < 0) {
                minplaylist = i;
            } else {
                int64_t dts     =    pls->pkt.dts;
                int64_t mindts  = minpls->pkt.dts;
                if (dts == AV_NOPTS_VALUE ||
                    (mindts != AV_NOPTS_VALUE && compare_ts_with_wrapdetect(dts, pls, mindts, minpls) < 0))
                    minplaylist = i;
            }
        }
    }

    /* If we got a packet, return it */
    if (minplaylist >= 0) {
        ret = read_packet_return_pkt(s, c, pkt, minplaylist);
        return ret;
    }
    return AVERROR_EOF;
}

static void read_seek_reset_reading(AVFormatContext *s,
    struct playlist *pls, struct playlist *seek_pls, int64_t seek_timestamp, int flags)
{
    if (!is_playlist_normal(pls)) {
        return;
    }
    HLSContext *c = s->priv_data;
    /* Reset reading */
    if (pls->input)
        close_format_io(pls->parent, pls, &pls->input);
    pls->input_read_done = 0;
    if (pls->input_next)
        close_format_io(pls->parent, NULL, &pls->input_next);
    pls->input_next_requested = 0;
    av_packet_unref(&pls->pkt);
    reset_packet(&pls->pkt);
    pls->pb.eof_reached = 0;
    /* Clear any buffered data */
    pls->pb.buf_end = pls->pb.buf_ptr = pls->pb.buffer;
    /* Reset the pos, to let the mpegts demuxer know we've seeked. */
    pls->pb.pos = 0;
    ff_adaptive_playlist_seek_flush(pls->adaptive);
    /* Flush the packet queue of the subdemuxer. */
    ff_read_frame_flush(pls->ctx);

    pls->seek_timestamp = seek_timestamp;
    pls->seek_flags = flags;

    if (pls != seek_pls) {
        /* set closest segment seq_no for playlists not handled above */
        find_timestamp_in_playlist(c, pls, seek_timestamp, &pls->cur_seq_no, NULL);
        /* seek the playlist to the given position without taking
         * keyframes into account since this playlist does not have the
         * specified stream where we should look for the keyframes */
        if (pls->is_id3_timestamped) { /* id3 may have no pts */
            pls->seek_timestamp = AV_NOPTS_VALUE;
        } else {
            pls->seek_stream_index = -1;
            pls->seek_flags |= AVSEEK_FLAG_ANY;
        }
    }
}

static int hls_read_seek(AVFormatContext *s, int stream_index,
                               int64_t timestamp, int flags)
{
    HLSContext *c = s->priv_data;
    struct playlist *seek_pls = NULL;
    int i, j;
    int stream_subdemuxer_index;
    int64_t first_timestamp, seek_timestamp, duration;
    int64_t seq_no, seg_start_ts;

    if ((flags & AVSEEK_FLAG_BYTE) || (c->ctx->ctx_flags & AVFMTCTX_UNSEEKABLE))
        return AVERROR(ENOSYS);

    first_timestamp = c->first_timestamp == AV_NOPTS_VALUE ?
                      0 : c->first_timestamp;
    if (1 == s->demux_by_user) {
        seek_timestamp = timestamp;
    } else {
        seek_timestamp = av_rescale_rnd(timestamp, AV_TIME_BASE,
                                        s->streams[stream_index]->time_base.den,
                                        flags & AVSEEK_FLAG_BACKWARD ?
                                        AV_ROUND_DOWN : AV_ROUND_UP);
    }
    duration = s->duration == AV_NOPTS_VALUE ?
               0 : s->duration;

    if (0 < duration && duration < seek_timestamp - first_timestamp)
        return AVERROR(EIO);

    /* find the playlist with the specified stream */
    for (i = 0; i < c->n_playlists; i++) {
        struct playlist *pls = c->playlists[i];
        if (1 == s->demux_by_user && PLS_NEED_YES == pls->needed) {
            seek_pls = pls;
            break;
        }
        if (!is_playlist_normal(pls)) {
            continue;
        }
        for (j = 0; j < pls->n_main_streams; j++) {
            if (pls->main_streams[j] == s->streams[stream_index]) {
                seek_pls = pls;
                stream_subdemuxer_index = j;
                break;
            }
        }
    }
    /* check if the timestamp is valid for the playlist with the
     * specified stream index */
    if (!seek_pls || !find_timestamp_in_playlist(c, seek_pls, seek_timestamp, &seq_no, &seg_start_ts))
        return AVERROR(EIO);

    if (s->streams[stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
        flags & AVSEEK_FLAG_BACKWARD && !(flags & AVSEEK_FLAG_ANY)) {
        /* Seeking to start of segment ensures we seek to a keyframe located
         * before the given timestamp. */
        seek_timestamp = seg_start_ts;
    }

    /* set segment now so we do not need to search again below */
    seek_pls->cur_seq_no = seq_no;
    seek_pls->seek_stream_index = stream_subdemuxer_index;
    /* id3 playlist may have less timestamp and cannot seek */
    int is_needed_id3_pls = 0;
    for (i = 0; i < c->n_playlists; i++) {
        /* Reset reading */
        struct playlist *pls = c->playlists[i];

        PLAYLIST_LOCK(c, pls);
        read_seek_reset_reading(s,
            pls, seek_pls, seek_timestamp, flags);
        if (PLS_NEED_YES == pls->needed && pls->is_id3_timestamped) {
            is_needed_id3_pls = 1;
        }
        PLAYLIST_UNLOCK(c, pls);
    }
    if (is_needed_id3_pls ||
        (AVMEDIA_TYPE_VIDEO == s->streams[stream_index]->codecpar->codec_type)) {
        seek_pls->seek_timestamp = AV_NOPTS_VALUE;
        c->cur_timestamp = seg_start_ts;
    } else {
        c->cur_timestamp = seek_timestamp;
    }

    return 0;
}

static int hls_probe(const AVProbeData *p)
{
    /* Require #EXTM3U at the start, and either one of the ones below
     * somewhere for a proper match. */
    if (strncmp(p->buf, "#EXTM3U", 7))
        return 0;

    if (strstr(p->buf, "#EXT-X-STREAM-INF:")     ||
        strstr(p->buf, "#EXT-X-TARGETDURATION:") ||
        strstr(p->buf, "#EXT-X-MEDIA-SEQUENCE:")) {

        int mime_ok = p->mime_type && !(
            av_strcasecmp(p->mime_type, "application/vnd.apple.mpegurl") &&
            av_strcasecmp(p->mime_type, "audio/mpegurl")
            );

        int mime_x = p->mime_type && !(
            av_strcasecmp(p->mime_type, "audio/x-mpegurl") &&
            av_strcasecmp(p->mime_type, "application/x-mpegurl")
            );

        if (!mime_ok &&
            !mime_x &&
            !av_match_ext    (p->filename, "m3u8,m3u") &&
             ff_match_url_ext(p->filename, "m3u8,m3u") <= 0) {
            av_log(NULL, AV_LOG_ERROR, "Not detecting m3u8/hls with non standard extension and non standard mime type\n");
            return AVPROBE_SCORE_MIME;
        }
        if (mime_x)
            av_log(NULL, AV_LOG_WARNING, "mime type is not rfc8216 compliant\n");

        return AVPROBE_SCORE_MAX;
    }
    return 0;
}

#define OFFSET(x) offsetof(HLSContext, x)
#define FLAGS AV_OPT_FLAG_DECODING_PARAM
static const AVOption hls_options[] = {
    {"live_start_index", "segment index to start live streams at (negative values are from the end)",
        OFFSET(live_start_index), AV_OPT_TYPE_INT, {.i64 = -3}, INT_MIN, INT_MAX, FLAGS},
    {"prefer_x_start", "prefer to use #EXT-X-START if it's in playlist instead of live_start_index",
        OFFSET(prefer_x_start), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, FLAGS},
    {"allowed_extensions", "List of file extensions that hls is allowed to access",
        OFFSET(allowed_extensions), AV_OPT_TYPE_STRING,
        {.str = "3gp,aac,avi,ac3,eac3,flac,mkv,m3u8,m4a,m4s,m4v,mpg,mov,mp2,mp3,mp4,mpeg,mpegts,ogg,ogv,oga,ts,vob,wav"},
        INT_MIN, INT_MAX, FLAGS},
    {"max_reload", "Maximum number of times a insufficient list is attempted to be reloaded",
        OFFSET(max_reload), AV_OPT_TYPE_INT, {.i64 = 3}, 0, INT_MAX, FLAGS},
    {"m3u8_hold_counters", "The maximum number of times to load m3u8 when it refreshes without new segments",
        OFFSET(m3u8_hold_counters), AV_OPT_TYPE_INT, {.i64 = 1000}, 0, INT_MAX, FLAGS},
    {"http_persistent", "Use persistent HTTP connections",
        OFFSET(http_persistent), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, FLAGS },
    {"http_multiple", "Use multiple HTTP connections for fetching segments",
        OFFSET(http_multiple), AV_OPT_TYPE_BOOL, {.i64 = -1}, -1, 1, FLAGS},
    {"seg_max_retry", "Maximum number of times to reload a segment on error.",
     OFFSET(seg_max_retry), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, FLAGS},
    { "qos_net_bitrate", "bitrate of network for qos",
        OFFSET(qos_net_bitrate), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, FLAGS},
    { "use_thread", "Use thread when need",
        OFFSET(use_thread), AV_OPT_TYPE_INT, { .i64 = 1 }, 0, INT_MAX, .flags = FLAGS},
    {NULL}
};

static const AVClass hls_class = {
    .class_name = "hls demuxer",
    .item_name  = av_default_item_name,
    .option     = hls_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_hls_demuxer = {
    .name           = "hls",
    .long_name      = NULL_IF_CONFIG_SMALL("Apple HTTP Live Streaming"),
    .priv_class     = &hls_class,
    .priv_data_size = sizeof(HLSContext),
    .flags          = AVFMT_NOGENSEARCH,
    .read_probe     = hls_probe,
    .read_header    = hls_read_header,
    .read_packet    = hls_read_packet,
    .read_close     = hls_close,
    .read_seek      = hls_read_seek,
};
