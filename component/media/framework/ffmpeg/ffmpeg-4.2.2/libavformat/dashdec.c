/*
 * Dynamic Adaptive Streaming over HTTP demux
 * Copyright (c) 2017 samsamsam@o2.pl based on HLS demux
 * Copyright (c) 2017 Steven Liu
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
#include <libxml/parser.h>
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/parseutils.h"
#include "internal.h"
#include "avio_internal.h"
#include "dash.h"
#include "adaptive.h"
#include "decryption_comp.h"

#define RETRY_CNT_READ_DATA                3
#define INITIAL_BUFFER_SIZE              32768

struct fragment {
    char *base_url;
    int64_t url_offset;
    int64_t size;
    char *url;
};

/*
 * reference to : ISO_IEC_23009-1-DASH-2012
 * Section: 5.3.9.6.2
 * Table: Table 17 — Semantics of SegmentTimeline element
 * */
struct timeline {
    /* starttime: Element or Attribute Name
     * specifies the MPD start time, in @timescale units,
     * the first Segment in the series starts relative to the beginning of the Period.
     * The value of this attribute must be equal to or greater than the sum of the previous S
     * element earliest presentation time and the sum of the contiguous Segment durations.
     * If the value of the attribute is greater than what is expressed by the previous S element,
     * it expresses discontinuities in the timeline.
     * If not present then the value shall be assumed to be zero for the first S element
     * and for the subsequent S elements, the value shall be assumed to be the sum of
     * the previous S element's earliest presentation time and contiguous duration
     * (i.e. previous S@starttime + @duration * (@repeat + 1)).
     * */
    int64_t starttime;
    /* repeat: Element or Attribute Name
     * specifies the repeat count of the number of following contiguous Segments with
     * the same duration expressed by the value of @duration. This value is zero-based
     * (e.g. a value of three means four Segments in the contiguous series).
     * */
    int64_t repeat;
    /* duration: Element or Attribute Name
     * specifies the Segment duration, in units of the value of the @timescale.
     * */
    int64_t duration;
};

/*
 * Each playlist has its own demuxer. If it is currently active,
 * it has an opened AVIOContext too, and potentially an AVPacket
 * containing the next packet from this stream.
 */
struct representation {
    char *url_template;
    AVIOContext pb;
    AVIOContext *input;
    AVFormatContext *parent;
    AVFormatContext *ctx;
    //AVPacket pkt;
    int rep_idx;
    //int rep_count;
    int stream_index; /* source stream index, number of source streams not always equals to demuxer streams */

    enum AVMediaType type;
    char id[20];
    char *lang;
    int bandwidth;
    AVRational framerate;

    int n_assoc_stream;      /* demuxer stream number */
    int assoc_stream_index;  /* demuxer stream start index in whole exposed AVFormatContext *parent */
    AVStream **assoc_stream; /* demuxer stream associated with this representation */

    int n_fragments;
    struct fragment **fragments; /* VOD list of fragment for profile */

    int n_timelines;
    struct timeline **timelines;

    int64_t first_seq_no;
    int64_t last_seq_no;
    int64_t start_number; /* used in case when we have dynamic list of segment to know which segments are new one*/

    int64_t fragment_duration;
    int64_t fragment_timescale;

    int64_t presentation_timeoffset;

    int64_t cur_seq_no;
    int64_t cur_seg_offset;
    int64_t cur_seg_size;
    struct fragment *cur_seg;
    struct fragment *next_seg;

    /* Currently active Media Initialization Section */
    struct fragment *init_section;
    uint8_t *init_sec_buf;
    uint32_t init_sec_buf_size;
    uint32_t init_sec_data_len;
    uint32_t init_sec_buf_read_offset;
    int64_t cur_timestamp;
    int is_restart_needed;
    int64_t read_start_time;
    AVMutex mutex;
    AdaptiveInputContext adaptive_input_ctx;
    AdaptivePlaylistContext *adaptive;
    AVEncryptionInitInfo *encryption_info;
};

static int read_header_finished(AVFormatContext *s);
static int dash_seek(AVFormatContext *s, struct representation *pls, int64_t seek_pos_msec, int flags, int dry_run);

typedef struct DASHContext {
    const AVClass *class;
    char *base_url;
    char *adaptionset_contenttype_val;
    char *adaptionset_par_val;
    char *adaptionset_lang_val;
    char *adaptionset_minbw_val;
    char *adaptionset_maxbw_val;
    char *adaptionset_minwidth_val;
    char *adaptionset_maxwidth_val;
    char *adaptionset_minheight_val;
    char *adaptionset_maxheight_val;
    char *adaptionset_minframerate_val;
    char *adaptionset_maxframerate_val;
    char *adaptionset_segmentalignment_val;
    char *adaptionset_bitstreamswitching_val;

    int n_videos;
    struct representation **videos;
    int n_audios;
    struct representation **audios;
    int n_subtitles;
    struct representation **subtitles;
    /* (1 << AVMEDIA_TYPE_VIDEO): video start done;
     * (1 << AVMEDIA_TYPE_AUDIO): audio start done;
     * (1 << AVMEDIA_TYPE_SUBTITLE):subtitle start done
     */
    unsigned int media_start_done;

    unsigned int nb_streams;
    unsigned int nb_valid_rep;

    /* MediaPresentationDescription Attribute */
    uint64_t media_presentation_duration;
    uint64_t suggested_presentation_delay;
    uint64_t availability_start_time;
    uint64_t availability_end_time;
    uint64_t publish_time;
    uint64_t minimum_update_period;
    uint64_t time_shift_buffer_depth;
    uint64_t min_buffer_time;

    /* Period Attribute */
    uint64_t period_duration;
    uint64_t period_start;

    /* AdaptationSet Attribute */
    char *adaptionset_lang;

    int is_live;
    int use_thread;
    AdaptiveAsync async;
    AVIOInterruptCB *interrupt_callback;
    char *allowed_extensions;
    AVDictionary *avio_opts;
    int max_url_size;
    int need_send_async_done;
    /* Sync timestamp when change playlist */
    int64_t playlist_sync_timestamp;
    /* Flags for init section*/
    int is_init_section_common_video;
    int is_init_section_common_audio;
    int64_t qos_net_bitrate;
    /* For live clock compensation when system clock not correct */
    int64_t clock_compensation_sec;
} DASHContext;
static int64_t calc_max_seg_no(struct representation *pls, DASHContext *c);

static int ishttp(char *url)
{
    const char *proto_name = avio_find_protocol_name(url);
    return av_strstart(proto_name, "http", NULL);
}

static int aligned(int val)
{
    return ((val + 0x3F) >> 6) << 6;
}

static uint64_t get_current_time_in_sec(DASHContext *c)
{
    int64_t now_sec = av_gettime() / AV_TIME_BASE;
    if (-1 == c->clock_compensation_sec) {
        return now_sec;
    }

    return now_sec + c->clock_compensation_sec;
}

static int64_t get_update_interval_usec(DASHContext *c)
{
    int64_t update_interval_usec =
        FFMIN(c->minimum_update_period * AV_TIME_BASE, SLOW_CLOCK_UPDATE_INTERVAL);
    return update_interval_usec;
}

static uint64_t get_utc_date_time_insec(AVFormatContext *s, const char *datetime)
{
    struct tm timeinfo;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int ret = 0;
    float second = 0.0;

    /* ISO-8601 date parser */
    if (!datetime)
        return 0;

    ret = sscanf(datetime, "%d-%d-%dT%d:%d:%fZ", &year, &month, &day, &hour, &minute, &second);
    /* year, month, day, hour, minute, second  6 arguments */
    if (ret != 6) {
        av_log(s, AV_LOG_WARNING, "get_utc_date_time_insec get a wrong time format\n");
    }
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon  = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min  = minute;
    timeinfo.tm_sec  = (int)second;

    return av_timegm(&timeinfo);
}

static uint32_t get_duration_insec(AVFormatContext *s, const char *duration)
{
    /* ISO-8601 duration parser */
    uint32_t days = 0;
    uint32_t hours = 0;
    uint32_t mins = 0;
    uint32_t secs = 0;
    int size = 0;
    float value = 0;
    char type = '\0';
    const char *ptr = duration;

    while (*ptr) {
        if (*ptr == 'P' || *ptr == 'T') {
            ptr++;
            continue;
        }

        if (sscanf(ptr, "%f%c%n", &value, &type, &size) != 2) {
            av_log(s, AV_LOG_WARNING, "get_duration_insec get a wrong time format\n");
            return 0; /* parser error */
        }
        switch (type) {
            case 'D':
                days = (uint32_t)value;
                break;
            case 'H':
                hours = (uint32_t)value;
                break;
            case 'M':
                mins = (uint32_t)value;
                break;
            case 'S':
                secs = (uint32_t)value;
                break;
            default:
                // handle invalid type
                break;
        }
        ptr += size;
    }
    return  ((days * 24 + hours) * 60 + mins) * 60 + secs;
}

static int64_t get_segment_start_time_based_on_timeline(
    DASHContext *c, struct representation *pls, int64_t cur_seq_no)
{
    int64_t start_time = 0;
    int64_t i = 0;
    int64_t j = 0;
    int64_t num = 0;

    if (pls->n_timelines) {
        if (cur_seq_no > 0) {
            num = pls->first_seq_no;
        }
        for (i = 0; i < pls->n_timelines; i++) {
            if (pls->timelines[i]->starttime > 0) {
                start_time = pls->timelines[i]->starttime;
            }
            if (num == cur_seq_no)
                goto finish;

            start_time += pls->timelines[i]->duration;
            if (pls->timelines[i]->repeat == -1) {
                start_time = pls->timelines[i]->duration * cur_seq_no;
                goto finish;
            }

            for (j = 0; j < pls->timelines[i]->repeat; j++) {
                num++;
                if (num == cur_seq_no)
                    goto finish;
                start_time += pls->timelines[i]->duration;
            }
            num++;
        }
    }
finish:
    av_log(c, AV_LOG_VERBOSE,
        "[%p]Get seg time, first:%lld num:%lld total:%d curr:%lld start time:%lld\n",
        pls, pls->first_seq_no, num, pls->n_timelines, cur_seq_no, start_time);
    return start_time;
}

static int64_t calc_next_seg_no_from_timelines(struct representation *pls, int64_t cur_time)
{
    int64_t i = 0;
    int64_t j = 0;
    int64_t num = 0;
    int64_t start_time = 0;

    for (i = 0; i < pls->n_timelines; i++) {
        if (pls->timelines[i]->starttime > 0) {
            start_time = pls->timelines[i]->starttime;
        }
        if (start_time > cur_time)
            goto finish;

        start_time += pls->timelines[i]->duration;
        for (j = 0; j < pls->timelines[i]->repeat; j++) {
            num++;
            if (start_time > cur_time)
                goto finish;
            start_time += pls->timelines[i]->duration;
        }
        num++;
    }

    return -1;

finish:
    return num;
}

static void free_fragment(struct fragment **seg)
{
    if (!(*seg)) {
        return;
    }
    if ((*seg)->base_url)
        av_freep(&((*seg)->base_url));
    if ((*seg)->url)
        av_freep(&((*seg)->url));
    av_freep(seg);
}

static void free_fragment_list(struct representation *pls)
{
    int i;

    for (i = 0; i < pls->n_fragments; i++) {
        free_fragment(&pls->fragments[i]);
    }
    av_freep(&pls->fragments);
    pls->n_fragments = 0;
}

static void free_timelines_list(struct representation *pls)
{
    int i;

    for (i = 0; i < pls->n_timelines; i++) {
        if (pls->timelines[i]) {
            av_freep(&pls->timelines[i]);
        }
    }
    av_freep(&pls->timelines);
    pls->n_timelines = 0;
}

static void free_representation(struct representation *pls)
{
    free_fragment_list(pls);
    free_timelines_list(pls);
    free_fragment(&pls->cur_seg);
    free_fragment(&pls->next_seg);
    free_fragment(&pls->init_section);
    av_freep(&pls->lang);
    av_freep(&pls->init_sec_buf);
    av_freep(&pls->pb.buffer);
    av_freep(&pls->assoc_stream);

    ff_adaptive_reinit_input_context(pls->parent, &pls->adaptive_input_ctx);
    ff_adaptive_playlist_io_deinit(pls->parent, pls->adaptive, &pls->input);
    if (pls->ctx) {
        pls->ctx->pb = NULL;
        avformat_close_input(&pls->ctx);
    }
    if (pls->encryption_info) {
        ff_dash_free_content_protection(&pls->encryption_info);
        pls->encryption_info = NULL;
    }
    ff_adaptive_playlist_context_free(&pls->adaptive);
    av_freep(&pls->url_template);
    av_freep(&pls);
}

static void free_video_list(DASHContext *c)
{
    int i;
    for (i = 0; i < c->n_videos; i++) {
        struct representation *pls = c->videos[i];
        if (pls->ctx) {
            ff_mutex_destroy(&pls->mutex);
        }
        free_representation(pls);
    }
    av_freep(&c->videos);
    c->n_videos = 0;
}

static void free_audio_list(DASHContext *c)
{
    int i;
    for (i = 0; i < c->n_audios; i++) {
        struct representation *pls = c->audios[i];
        if (pls->ctx) {
            ff_mutex_destroy(&pls->mutex);
        }
        free_representation(pls);
    }
    av_freep(&c->audios);
    c->n_audios = 0;
}

static void free_subtitle_list(DASHContext *c)
{
    int i;
    for (i = 0; i < c->n_subtitles; i++) {
        struct representation *pls = c->subtitles[i];
        if (pls->ctx) {
            ff_mutex_destroy(&pls->mutex);
        }
        free_representation(pls);
    }
    av_freep(&c->subtitles);
    c->n_subtitles = 0;
}

static void dash_close_free(DASHContext *c)
{
    free_audio_list(c);
    free_video_list(c);
    free_subtitle_list(c);
    av_dict_free(&c->avio_opts);
    av_freep(&c->base_url);
}

static int dash_interrupt_callback(void *opaque)
{
    struct representation *rep = opaque;
    DASHContext *c = rep->parent->priv_data;

    if (ff_check_interrupt(c->interrupt_callback)) {
        return AVERROR_EXIT;
    }

    /* Don't care when vod */
    int64_t read_start_time = rep->read_start_time;
    if (!(c->is_live) || read_start_time < 0) {
        return 0;
    }

    int64_t update_interval_usec = get_update_interval_usec(c);
    update_interval_usec <<= 2; /* at most 4 segment time */
    int64_t time_elapsed = av_gettime() - read_start_time;
    if (update_interval_usec > 0 && time_elapsed >= update_interval_usec) {
        av_log(rep->parent, AV_LOG_ERROR,
            "Read data timedout cb, max:%lld, elapsed:%lld\n", update_interval_usec, time_elapsed);
        return AVERROR(ETIMEDOUT);
    }
    return 0;
}

static int open_url_l(
    AVFormatContext *s, AVIOContext **pb,
    struct representation *rep, const char *url,
    AVDictionary *opts, AVDictionary *opts2, int *is_http)
{
    DASHContext *c = s->priv_data;
    AVDictionary *tmp = NULL;
    const char *proto_name = NULL;
    int ret;

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
        ;
    } else
        return AVERROR_INVALIDDATA;

    if (!strncmp(proto_name, url, strlen(proto_name)) && url[strlen(proto_name)] == ':')
        ;
    else if (av_strstart(url, "crypto", NULL) && !strncmp(proto_name, url + 7, strlen(proto_name)) && url[7 + strlen(proto_name)] == ':')
        ;
    else if (strcmp(proto_name, "file") || !strncmp(url, "file,", 5))
        return AVERROR_INVALIDDATA;

    av_freep(pb);

    AVIOInterruptCB *interrupt_callback = rep->ctx ?
        (&(rep->ctx->interrupt_callback)) : (c->interrupt_callback);
    ret = avio_open2(pb, url, AVIO_FLAG_READ, interrupt_callback, &tmp);
    if (ret >= 0) {
        // update cookies on http response with setcookies.
        char *new_cookies = NULL;

        if (!(s->flags & AVFMT_FLAG_CUSTOM_IO))
            av_opt_get(*pb, "cookies", AV_OPT_SEARCH_CHILDREN, (uint8_t**)&new_cookies);

        if (new_cookies) {
            av_dict_set(&opts, "cookies", new_cookies, AV_DICT_DONT_STRDUP_VAL);
        }
    }

    av_dict_free(&tmp);

    if (is_http)
        *is_http = av_strstart(proto_name, "http", NULL);

    return ret;
}

static int open_url(AVFormatContext *s, AVIOContext **pb,
    struct representation *rep, const char *url, AVDictionary *opts, AVDictionary *opts2)
{
    int ret = 0;
    DASHContext *c = s->priv_data;

    AVIOContext *ptmp = NULL;
    if (!(ret = ff_check_interrupt(c->interrupt_callback))) {
        ret = open_url_l(s, pb, rep, url, opts, opts2, NULL);
    }
    return ret;
}

static char *get_content_url(xmlNodePtr *baseurl_nodes,
                             int n_baseurl_nodes,
                             int max_url_size,
                             char *rep_id_val,
                             char *rep_bandwidth_val,
                             char *val)
{
    int i;
    char *text;
    char *url = NULL;
    char *tmp_str = av_mallocz(max_url_size);
    char *tmp_str_2 = av_mallocz(max_url_size);

    if (!tmp_str || !tmp_str_2) {
        return NULL;
    }

    for (i = 0; i < n_baseurl_nodes; ++i) {
        if (baseurl_nodes[i] &&
            baseurl_nodes[i]->children &&
            baseurl_nodes[i]->children->type == XML_TEXT_NODE) {
            text = xmlNodeGetContent(baseurl_nodes[i]->children);
            if (text) {
                memset(tmp_str, 0, max_url_size);
                memset(tmp_str_2, 0, max_url_size);
                ff_make_absolute_url(tmp_str_2, max_url_size, tmp_str, text);
                av_strlcpy(tmp_str, tmp_str_2, max_url_size);
                xmlFree(text);
            }
        }
    }

    if (val)
        ff_make_absolute_url(tmp_str, max_url_size, tmp_str, val);

    if (rep_id_val) {
        url = av_strireplace(tmp_str, "$RepresentationID$", (const char*)rep_id_val);
        if (!url) {
            goto end;
        }
        av_strlcpy(tmp_str, url, max_url_size);
    }
    if (rep_bandwidth_val && tmp_str[0] != '\0') {
        // free any previously assigned url before reassigning
        av_free(url);
        url = av_strireplace(tmp_str, "$Bandwidth$", (const char*)rep_bandwidth_val);
        if (!url) {
            goto end;
        }
    }
end:
    av_free(tmp_str);
    av_free(tmp_str_2);
    return url;
}

static char *get_val_from_nodes_tab(xmlNodePtr *nodes, const int n_nodes, const char *attrname)
{
    int i;
    char *val;

    for (i = 0; i < n_nodes; ++i) {
        if (nodes[i]) {
            val = xmlGetProp(nodes[i], attrname);
            if (val)
                return val;
        }
    }

    return NULL;
}

static xmlNodePtr find_child_node_by_name(xmlNodePtr rootnode, const char *nodename)
{
    xmlNodePtr node = rootnode;
    if (!node) {
        return NULL;
    }

    node = xmlFirstElementChild(node);
    while (node) {
        if (!av_strcasecmp(node->name, nodename)) {
            return node;
        }
        node = xmlNextElementSibling(node);
    }
    return NULL;
}

static enum AVMediaType get_content_type(xmlNodePtr node)
{
    enum AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
    int i = 0;
    const char *attr;
    char *val = NULL;

    if (node) {
        for (i = 0; i < 2; i++) {
            attr = i ? "mimeType" : "contentType";
            val = xmlGetProp(node, attr);
            if (val) {
                if (av_stristr((const char *)val, "video")) {
                    type = AVMEDIA_TYPE_VIDEO;
                } else if (av_stristr((const char *)val, "audio")) {
                    type = AVMEDIA_TYPE_AUDIO;
                } else if (av_stristr((const char *)val, "text")) {
                    type = AVMEDIA_TYPE_SUBTITLE;
                }
                xmlFree(val);
            }
        }
    }
    return type;
}

static struct fragment * get_Fragment(char *range)
{
    struct fragment * seg = av_mallocz(sizeof(struct fragment));

    if (!seg)
        return NULL;

    seg->size = -1;
    if (range) {
        char *str_end_offset;
        char *str_offset = av_strtok(range, "-", &str_end_offset);
        seg->url_offset = strtoll(str_offset, NULL, 10);
        seg->size = strtoll(str_end_offset, NULL, 10) - seg->url_offset;
    }

    return seg;
}

static int parse_manifest_segmenturlnode(AVFormatContext *s,
                                         struct representation *rep, DASHContext *c,
                                         xmlNodePtr fragmenturl_node,
                                         xmlNodePtr *baseurl_nodes,
                                         char *rep_id_val,
                                         char *rep_bandwidth_val)
{
    char *initialization_val = NULL;
    char *media_val = NULL;
    char *range_val = NULL;
    int max_url_size = c ? c->max_url_size: MAX_URL_SIZE;

    if (!av_strcasecmp(fragmenturl_node->name, (const char *)"Initialization")) {
        initialization_val = xmlGetProp(fragmenturl_node, "sourceURL");
        range_val = xmlGetProp(fragmenturl_node, "range");
        if (initialization_val || range_val) {
            rep->init_section = get_Fragment(range_val);
            if (!rep->init_section) {
                xmlFree(initialization_val);
                xmlFree(range_val);
                return AVERROR(ENOMEM);
            }
            rep->init_section->url = get_content_url(baseurl_nodes, 4,
                                                     max_url_size,
                                                     rep_id_val,
                                                     rep_bandwidth_val,
                                                     initialization_val);

            if (!rep->init_section->url) {
                av_free(rep->init_section);
                xmlFree(initialization_val);
                xmlFree(range_val);
                return AVERROR(ENOMEM);
            }
            xmlFree(initialization_val);
            xmlFree(range_val);
        }
    } else if (!av_strcasecmp(fragmenturl_node->name, (const char *)"SegmentURL")) {
        media_val = xmlGetProp(fragmenturl_node, "media");
        range_val = xmlGetProp(fragmenturl_node, "mediaRange");
        if (media_val || range_val) {
            struct fragment *seg = get_Fragment(range_val);
            if (!seg) {
                xmlFree(media_val);
                xmlFree(range_val);
                return AVERROR(ENOMEM);
            }
            seg->url = get_content_url(baseurl_nodes, 4,
                                       max_url_size,
                                       rep_id_val,
                                       rep_bandwidth_val,
                                       media_val);
            if (!seg->url) {
                av_free(seg);
                xmlFree(media_val);
                xmlFree(range_val);
                return AVERROR(ENOMEM);
            }
            dynarray_add(&rep->fragments, &rep->n_fragments, seg);
            xmlFree(media_val);
            xmlFree(range_val);
        }
    }

    return 0;
}

static int parse_manifest_segmenttimeline(AVFormatContext *s, struct representation *rep,
                                          xmlNodePtr fragment_timeline_node)
{
    xmlAttrPtr attr = NULL;
    char *val  = NULL;

    if (!av_strcasecmp(fragment_timeline_node->name, (const char *)"S")) {
        struct timeline *tml = av_mallocz(sizeof(struct timeline));
        if (!tml) {
            return AVERROR(ENOMEM);
        }
        attr = fragment_timeline_node->properties;
        while (attr) {
            val = xmlGetProp(fragment_timeline_node, attr->name);

            if (!val) {
                av_log(s, AV_LOG_WARNING, "parse_manifest_segmenttimeline attr->name = %s val is NULL\n", attr->name);
                continue;
            }

            if (!av_strcasecmp(attr->name, (const char *)"t")) {
                tml->starttime = (int64_t)strtoll(val, NULL, 10);
            } else if (!av_strcasecmp(attr->name, (const char *)"r")) {
                tml->repeat =(int64_t) strtoll(val, NULL, 10);
            } else if (!av_strcasecmp(attr->name, (const char *)"d")) {
                tml->duration = (int64_t)strtoll(val, NULL, 10);
            }
            attr = attr->next;
            xmlFree(val);
        }
        dynarray_add(&rep->timelines, &rep->n_timelines, tml);
    }

    return 0;
}

static int parse_manifest_timeline_node(AVFormatContext *s,
    struct representation *rep, xmlNodePtr fragment_timeline_node)
{
    if (!fragment_timeline_node) {
        return 0;
    }

    int ret = 0;
    fragment_timeline_node = xmlFirstElementChild(fragment_timeline_node);
    while (fragment_timeline_node) {
        ret = parse_manifest_segmenttimeline(s, rep, fragment_timeline_node);
        if (ret < 0) {
            return ret;
        }
        fragment_timeline_node = xmlNextElementSibling(fragment_timeline_node);
    }

    if (!rep->timelines || !(rep->n_timelines)) {
        return ret;
    }

    int64_t start_time = 0;
    for (int i = 0; i < rep->n_timelines; i++) {
        if (rep->timelines[i]->starttime > 0) {
            start_time = rep->timelines[i]->starttime;
        } else if (start_time > 0) {
            rep->timelines[i]->starttime = start_time;
        }

        start_time += rep->timelines[i]->duration;
        if (rep->timelines[i]->repeat == -1) {
            return ret;
        }

        for (int j = 0; j < rep->timelines[i]->repeat; j++) {
            start_time += rep->timelines[i]->duration;
        }
    }

    return ret;
}

static int resolve_content_path(AVFormatContext *s, const char *url, int *max_url_size, xmlNodePtr *baseurl_nodes, int n_baseurl_nodes)
{
    char *tmp_str = NULL;
    char *path = NULL;
    char *mpdName = NULL;
    xmlNodePtr node = NULL;
    char *baseurl = NULL;
    char *root_url = NULL;
    char *text = NULL;
    char *tmp = NULL;
    int isRootHttp = 0;
    char token ='/';
    int start =  0;
    int rootId = 0;
    int updated = 0;
    int size = 0;
    int i;
    int tmp_max_url_size = strlen(url);

    for (i = n_baseurl_nodes-1; i >= 0 ; i--) {
        text = xmlNodeGetContent(baseurl_nodes[i]);
        if (!text)
            continue;
        tmp_max_url_size += strlen(text);
        if (ishttp(text)) {
            xmlFree(text);
            break;
        }
        xmlFree(text);
    }

    tmp_max_url_size = aligned(tmp_max_url_size);
    text = av_mallocz(tmp_max_url_size);
    if (!text) {
        updated = AVERROR(ENOMEM);
        goto end;
    }
    av_strlcpy(text, url, strlen(url)+1);
    tmp = text;
    while (mpdName = av_strtok(tmp, "/", &tmp))  {
        size = strlen(mpdName);
    }
    av_free(text);

    path = av_mallocz(tmp_max_url_size);
    tmp_str = av_mallocz(tmp_max_url_size);
    if (!tmp_str || !path) {
        updated = AVERROR(ENOMEM);
        goto end;
    }

    av_strlcpy (path, url, strlen(url) - size + 1);
    for (rootId = n_baseurl_nodes - 1; rootId > 0; rootId --) {
        if (!(node = baseurl_nodes[rootId])) {
            continue;
        }
        text = xmlNodeGetContent(node);
        if (ishttp(text)) {
            xmlFree(text);
            break;
        }
        xmlFree(text);
    }

    node = baseurl_nodes[rootId];
    baseurl = xmlNodeGetContent(node);
    root_url = (av_strcasecmp(baseurl, "")) ? baseurl : path;
    if (node) {
        xmlNodeSetContent(node, root_url);
        updated = 1;
    }

    size = strlen(root_url);
    isRootHttp = ishttp(root_url);

    if (root_url[size - 1] != token) {
        av_strlcat(root_url, "/", size + 2);
        size += 2;
    }

    for (i = 0; i < n_baseurl_nodes; ++i) {
        if (i == rootId) {
            continue;
        }
        text = xmlNodeGetContent(baseurl_nodes[i]);
        if (text) {
            memset(tmp_str, 0, strlen(tmp_str));
            if (!ishttp(text) && isRootHttp) {
                av_strlcpy(tmp_str, root_url, size + 1);
            }
            start = (text[0] == token);
            av_strlcat(tmp_str, text + start, tmp_max_url_size);
            xmlNodeSetContent(baseurl_nodes[i], tmp_str);
            updated = 1;
            xmlFree(text);
        }
    }

end:
    if (tmp_max_url_size > *max_url_size) {
        *max_url_size = tmp_max_url_size;
    }
    av_free(path);
    av_free(tmp_str);
    xmlFree(baseurl);
    return updated;

}

static int parse_manifest_representation(AVFormatContext *s,
                                         DASHContext *c, const char *url,
                                         AVEncryptionInitInfo *encryption_info,
                                         xmlNodePtr node,
                                         xmlNodePtr adaptionset_node,
                                         xmlNodePtr mpd_baseurl_node,
                                         xmlNodePtr period_baseurl_node,
                                         xmlNodePtr period_segmenttemplate_node,
                                         xmlNodePtr period_segmentlist_node,
                                         xmlNodePtr fragment_template_node,
                                         xmlNodePtr content_component_node,
                                         xmlNodePtr adaptionset_baseurl_node,
                                         xmlNodePtr adaptionset_segmentlist_node,
                                         xmlNodePtr adaptionset_supplementalproperty_node)
{
    int32_t ret = 0;
    int32_t subtitle_rep_idx = 0;
    int32_t audio_rep_idx = 0;
    int32_t video_rep_idx = 0;
    struct representation *rep = NULL;
    struct fragment *seg = NULL;
    xmlNodePtr representation_segmenttemplate_node = NULL;
    xmlNodePtr representation_baseurl_node = NULL;
    xmlNodePtr representation_segmentlist_node = NULL;
    xmlNodePtr representation_content_protection_node = NULL;
    xmlNodePtr segmentlists_tab[3];
    xmlNodePtr fragment_timeline_node = NULL;
    xmlNodePtr fragment_templates_tab[5];
    char *duration_val = NULL;
    char *presentation_timeoffset_val = NULL;
    char *startnumber_val = NULL;
    char *timescale_val = NULL;
    char *initialization_val = NULL;
    char *media_val = NULL;
    char *val = NULL;
    xmlNodePtr baseurl_nodes[4];
    xmlNodePtr representation_node = node;
    char *rep_id_val = xmlGetProp(representation_node, "id");
    char *rep_bandwidth_val = xmlGetProp(representation_node, "bandwidth");
    char *rep_framerate_val = xmlGetProp(representation_node, "frameRate");
    enum AVMediaType type = AVMEDIA_TYPE_UNKNOWN;

    // try get information from representation
    if (type == AVMEDIA_TYPE_UNKNOWN)
        type = get_content_type(representation_node);
    // try get information from contentComponen
    if (type == AVMEDIA_TYPE_UNKNOWN)
        type = get_content_type(content_component_node);
    // try get information from adaption set
    if (type == AVMEDIA_TYPE_UNKNOWN)
        type = get_content_type(adaptionset_node);
    if (type == AVMEDIA_TYPE_UNKNOWN) {
        av_log(s, AV_LOG_VERBOSE, "Parsing '%s' - skipp not supported representation type\n", url);
    } else if (type == AVMEDIA_TYPE_VIDEO || type == AVMEDIA_TYPE_AUDIO || type == AVMEDIA_TYPE_SUBTITLE) {
        // convert selected representation to our internal struct
        rep = av_mallocz(sizeof(struct representation));
        if (!rep) {
            ret = AVERROR(ENOMEM);
            goto end;
        }

        if (c->adaptionset_lang) {
            rep->lang = av_strdup(c->adaptionset_lang);
            if (!rep->lang) {
                av_log(s, AV_LOG_ERROR, "alloc language memory failure\n");
                av_freep(&rep);
                return AVERROR(ENOMEM);
            }
        }

        representation_segmenttemplate_node = find_child_node_by_name(representation_node, "SegmentTemplate");
        representation_baseurl_node = find_child_node_by_name(representation_node, "BaseURL");
        representation_segmentlist_node = find_child_node_by_name(representation_node, "SegmentList");
        representation_content_protection_node = find_child_node_by_name(representation_node, "ContentProtection");

        baseurl_nodes[0] = mpd_baseurl_node;
        baseurl_nodes[1] = period_baseurl_node;
        baseurl_nodes[2] = adaptionset_baseurl_node;
        baseurl_nodes[3] = representation_baseurl_node;

        ret = resolve_content_path(s, url, &c->max_url_size, baseurl_nodes, 4);
        c->max_url_size = aligned(c->max_url_size
                                  + (rep_id_val ? strlen(rep_id_val) : 0)
                                  + (rep_bandwidth_val ? strlen(rep_bandwidth_val) : 0));
        if (ret == AVERROR(ENOMEM) || ret == 0) {
            goto end;
        }
        if (!encryption_info) {
            xmlNodePtr rcp_node = representation_content_protection_node;
            while (rcp_node) {
                /* Not ContentProtection tag */
                if (av_strcasecmp(rcp_node->name, (const char *)"ContentProtection")) {
                    break;
                }
                ret = ff_dash_parse_content_protection(rcp_node, &encryption_info);
                if (ret < 0) {
                    av_log(s, AV_LOG_WARNING, "not supported representation content protection\n");
                }
                rcp_node = xmlNextElementSibling(rcp_node);
            }
        }
        if (representation_segmenttemplate_node || fragment_template_node || period_segmenttemplate_node) {
            fragment_timeline_node = NULL;
            fragment_templates_tab[0] = representation_segmenttemplate_node;
            fragment_templates_tab[1] = adaptionset_segmentlist_node;
            fragment_templates_tab[2] = fragment_template_node;
            fragment_templates_tab[3] = period_segmenttemplate_node;
            fragment_templates_tab[4] = period_segmentlist_node;

            presentation_timeoffset_val = get_val_from_nodes_tab(fragment_templates_tab, 4, "presentationTimeOffset");
            duration_val = get_val_from_nodes_tab(fragment_templates_tab, 4, "duration");
            startnumber_val = get_val_from_nodes_tab(fragment_templates_tab, 4, "startNumber");
            timescale_val = get_val_from_nodes_tab(fragment_templates_tab, 4, "timescale");
            initialization_val = get_val_from_nodes_tab(fragment_templates_tab, 4, "initialization");
            media_val = get_val_from_nodes_tab(fragment_templates_tab, 4, "media");

            if (initialization_val) {
                rep->init_section = av_mallocz(sizeof(struct fragment));
                if (!rep->init_section) {
                    av_free(rep);
                    ret = AVERROR(ENOMEM);
                    goto end;
                }
                c->max_url_size = aligned(c->max_url_size  + strlen(initialization_val));
                rep->init_section->url = get_content_url(baseurl_nodes, 4,  c->max_url_size, rep_id_val, rep_bandwidth_val, initialization_val);
                if (!rep->init_section->url) {
                    av_free(rep->init_section);
                    av_free(rep);
                    ret = AVERROR(ENOMEM);
                    goto end;
                }
                rep->init_section->size = -1;
                xmlFree(initialization_val);
            }

            if (media_val) {
                c->max_url_size = aligned(c->max_url_size  + strlen(media_val));
                rep->url_template = get_content_url(baseurl_nodes, 4, c->max_url_size, rep_id_val, rep_bandwidth_val, media_val);
                xmlFree(media_val);
            }

            if (presentation_timeoffset_val) {
                rep->presentation_timeoffset = (int64_t) strtoll(presentation_timeoffset_val, NULL, 10);
                av_log(s, AV_LOG_TRACE, "rep->presentation_timeoffset = [%"PRId64"]\n", rep->presentation_timeoffset);
                xmlFree(presentation_timeoffset_val);
            }
            if (duration_val) {
                rep->fragment_duration = (int64_t) strtoll(duration_val, NULL, 10);
                av_log(s, AV_LOG_TRACE, "rep->fragment_duration = [%"PRId64"]\n", rep->fragment_duration);
                xmlFree(duration_val);
            }
            if (timescale_val) {
                rep->fragment_timescale = (int64_t) strtoll(timescale_val, NULL, 10);
                av_log(s, AV_LOG_TRACE, "rep->fragment_timescale = [%"PRId64"]\n", rep->fragment_timescale);
                xmlFree(timescale_val);
            }
            if (startnumber_val) {
                rep->first_seq_no = (int64_t) strtoll(startnumber_val, NULL, 10);
                av_log(s, AV_LOG_TRACE, "rep->first_seq_no = [%"PRId64"]\n", rep->first_seq_no);
                xmlFree(startnumber_val);
            }
            if (adaptionset_supplementalproperty_node) {
                if (!av_strcasecmp(xmlGetProp(adaptionset_supplementalproperty_node,"schemeIdUri"), "http://dashif.org/guidelines/last-segment-number")) {
                    val = xmlGetProp(adaptionset_supplementalproperty_node,"value");
                    if (!val) {
                        av_log(s, AV_LOG_ERROR, "Missing value attribute in adaptionset_supplementalproperty_node\n");
                    } else {
                        rep->last_seq_no =(int64_t) strtoll(val, NULL, 10) - 1;
                        xmlFree(val);
                    }
                }
            }

            fragment_timeline_node = find_child_node_by_name(representation_segmenttemplate_node, "SegmentTimeline");

            if (!fragment_timeline_node)
                fragment_timeline_node = find_child_node_by_name(fragment_template_node, "SegmentTimeline");
            if (!fragment_timeline_node)
                fragment_timeline_node = find_child_node_by_name(adaptionset_segmentlist_node, "SegmentTimeline");
            if (!fragment_timeline_node)
                fragment_timeline_node = find_child_node_by_name(period_segmentlist_node, "SegmentTimeline");
            ret = parse_manifest_timeline_node(s, rep, fragment_timeline_node);
            if (ret < 0) {
                return ret;
            }
        } else if (representation_baseurl_node && !representation_segmentlist_node) {
            seg = av_mallocz(sizeof(struct fragment));
            if (!seg) {
                ret = AVERROR(ENOMEM);
                goto end;
            }
            seg->url = get_content_url(baseurl_nodes, 4, c->max_url_size, rep_id_val, rep_bandwidth_val, NULL);
            if (!seg->url) {
                av_free(seg);
                ret = AVERROR(ENOMEM);
                goto end;
            }
            seg->size = -1;
            dynarray_add(&rep->fragments, &rep->n_fragments, seg);
        } else if (representation_segmentlist_node) {
            // TODO: https://www.brendanlong.com/the-structure-of-an-mpeg-dash-mpd.html
            // http://www-itec.uni-klu.ac.at/dash/ddash/mpdGenerator.php?fragmentlength=15&type=full
            xmlNodePtr fragmenturl_node = NULL;
            segmentlists_tab[0] = representation_segmentlist_node;
            segmentlists_tab[1] = adaptionset_segmentlist_node;
            segmentlists_tab[2] = period_segmentlist_node;

            duration_val = get_val_from_nodes_tab(segmentlists_tab, 3, "duration");
            timescale_val = get_val_from_nodes_tab(segmentlists_tab, 3, "timescale");
            if (duration_val) {
                rep->fragment_duration = (int64_t) strtoll(duration_val, NULL, 10);
                av_log(s, AV_LOG_TRACE, "rep->fragment_duration = [%"PRId64"]\n", rep->fragment_duration);
                xmlFree(duration_val);
            }
            if (timescale_val) {
                rep->fragment_timescale = (int64_t) strtoll(timescale_val, NULL, 10);
                av_log(s, AV_LOG_TRACE, "rep->fragment_timescale = [%"PRId64"]\n", rep->fragment_timescale);
                xmlFree(timescale_val);
            }
            fragmenturl_node = xmlFirstElementChild(representation_segmentlist_node);
            while (fragmenturl_node) {
                ret = parse_manifest_segmenturlnode(s, rep, c, fragmenturl_node,
                                                    baseurl_nodes,
                                                    rep_id_val,
                                                    rep_bandwidth_val);
                if (ret < 0) {
                    return ret;
                }
                fragmenturl_node = xmlNextElementSibling(fragmenturl_node);
            }

            fragment_timeline_node = find_child_node_by_name(representation_segmenttemplate_node, "SegmentTimeline");

            if (!fragment_timeline_node)
                fragment_timeline_node = find_child_node_by_name(fragment_template_node, "SegmentTimeline");
            if (!fragment_timeline_node)
                fragment_timeline_node = find_child_node_by_name(adaptionset_segmentlist_node, "SegmentTimeline");
            if (!fragment_timeline_node)
                fragment_timeline_node = find_child_node_by_name(period_segmentlist_node, "SegmentTimeline");
            ret = parse_manifest_timeline_node(s, rep, fragment_timeline_node);
            if (ret < 0) {
                return ret;
            }
        } else {
            free_representation(rep);
            rep = NULL;
            av_log(s, AV_LOG_ERROR, "Unknown format of Representation node id[%s] \n", (const char *)rep_id_val);
        }

        if (rep) {
            if (rep->fragment_duration > 0 && !rep->fragment_timescale)
                rep->fragment_timescale = 1;
            rep->bandwidth = rep_bandwidth_val ? atoi(rep_bandwidth_val) : 0;
            strncpy(rep->id, rep_id_val ? rep_id_val : "", sizeof(rep->id));
            rep->framerate = av_make_q(0, 0);
            if (type == AVMEDIA_TYPE_VIDEO && !rep_framerate_val) {
                rep_framerate_val = xmlGetProp(adaptionset_node, "frameRate");
            }
            if (type == AVMEDIA_TYPE_VIDEO && rep_framerate_val) {
                ret = av_parse_video_rate(&rep->framerate, rep_framerate_val);
                if (ret < 0)
                    av_log(s, AV_LOG_VERBOSE, "Ignoring invalid frame rate '%s'\n", rep_framerate_val);
            }

            switch (type) {
                case AVMEDIA_TYPE_VIDEO:
                    rep->rep_idx = video_rep_idx;
                    dynarray_add(&c->videos, &c->n_videos, rep);
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    rep->rep_idx = audio_rep_idx;
                    dynarray_add(&c->audios, &c->n_audios, rep);
                    break;
                case AVMEDIA_TYPE_SUBTITLE:
                    rep->rep_idx = subtitle_rep_idx;
                    dynarray_add(&c->subtitles, &c->n_subtitles, rep);
                    break;
                default:
                    av_log(s, AV_LOG_WARNING, "Unsupported the stream type %d\n", type);
                    break;
            }
            rep->type     = type;
            rep->parent   = s;
            rep->adaptive = ff_adaptive_playlist_context_new(s, type, c->is_live);
            rep->read_start_time = -1;
            ff_adaptive_reinit_input_context(s, &rep->adaptive_input_ctx);
            (void) ff_dash_clone_content_protection(&rep->encryption_info, encryption_info);
        }
    }

    video_rep_idx += type == AVMEDIA_TYPE_VIDEO;
    audio_rep_idx += type == AVMEDIA_TYPE_AUDIO;
    subtitle_rep_idx += type == AVMEDIA_TYPE_SUBTITLE;

end:
    if (rep_id_val)
        xmlFree(rep_id_val);
    if (rep_bandwidth_val)
        xmlFree(rep_bandwidth_val);
    if (rep_framerate_val)
        xmlFree(rep_framerate_val);

    return ret;
}

static int parse_manifest_adaptationset_attr(
    AVFormatContext *s, DASHContext *c, xmlNodePtr adaptionset_node)
{
    if (!adaptionset_node) {
        av_log(s, AV_LOG_WARNING, "Cannot get AdaptionSet\n");
        return AVERROR(EINVAL);
    }
    c->adaptionset_lang = xmlGetProp(adaptionset_node, "lang");

    return 0;
}

static int parse_manifest_adaptationset(AVFormatContext *s,
                                        DASHContext *c, const char *url,
                                        xmlNodePtr adaptionset_node,
                                        xmlNodePtr mpd_baseurl_node,
                                        xmlNodePtr period_baseurl_node,
                                        xmlNodePtr period_segmenttemplate_node,
                                        xmlNodePtr period_segmentlist_node)
{
    int ret = 0;
    xmlNodePtr fragment_template_node = NULL;
    xmlNodePtr content_component_node = NULL;
    xmlNodePtr adaptionset_baseurl_node = NULL;
    xmlNodePtr adaptionset_segmentlist_node = NULL;
    xmlNodePtr adaptionset_supplementalproperty_node = NULL;
    xmlNodePtr node = NULL;

    ret = parse_manifest_adaptationset_attr(s, c, adaptionset_node);
    if (ret < 0)
        return ret;

    c->adaptionset_contenttype_val = xmlGetProp(adaptionset_node, "contentType");
    c->adaptionset_par_val = xmlGetProp(adaptionset_node, "par");
    c->adaptionset_lang_val = xmlGetProp(adaptionset_node, "lang");
    c->adaptionset_minbw_val = xmlGetProp(adaptionset_node, "minBandwidth");
    c->adaptionset_maxbw_val = xmlGetProp(adaptionset_node, "maxBandwidth");
    c->adaptionset_minwidth_val = xmlGetProp(adaptionset_node, "minWidth");
    c->adaptionset_maxwidth_val = xmlGetProp(adaptionset_node, "maxWidth");
    c->adaptionset_minheight_val = xmlGetProp(adaptionset_node, "minHeight");
    c->adaptionset_maxheight_val = xmlGetProp(adaptionset_node, "maxHeight");
    c->adaptionset_minframerate_val = xmlGetProp(adaptionset_node, "minFrameRate");
    c->adaptionset_maxframerate_val = xmlGetProp(adaptionset_node, "maxFrameRate");
    c->adaptionset_segmentalignment_val = xmlGetProp(adaptionset_node, "segmentAlignment");
    c->adaptionset_bitstreamswitching_val = xmlGetProp(adaptionset_node, "bitstreamSwitching");

    AVEncryptionInitInfo *encryption_info = NULL;
    node = xmlFirstElementChild(adaptionset_node);
    while (node) {
        if (!av_strcasecmp(node->name, (const char *)"SegmentTemplate")) {
            fragment_template_node = node;
        } else if (!av_strcasecmp(node->name, (const char *)"ContentComponent")) {
            content_component_node = node;
        } else if (!av_strcasecmp(node->name, (const char *)"ContentProtection")) {
            (void) ff_dash_parse_content_protection(node, &encryption_info);
        } else if (!av_strcasecmp(node->name, (const char *)"BaseURL")) {
            adaptionset_baseurl_node = node;
        } else if (!av_strcasecmp(node->name, (const char *)"SegmentList")) {
            adaptionset_segmentlist_node = node;
        } else if (!av_strcasecmp(node->name, (const char *)"SupplementalProperty")) {
            adaptionset_supplementalproperty_node = node;
        } else if (!av_strcasecmp(node->name, (const char *)"Representation")) {
            ret = parse_manifest_representation(s, c, url,
                                                encryption_info,
                                                node,
                                                adaptionset_node,
                                                mpd_baseurl_node,
                                                period_baseurl_node,
                                                period_segmenttemplate_node,
                                                period_segmentlist_node,
                                                fragment_template_node,
                                                content_component_node,
                                                adaptionset_baseurl_node,
                                                adaptionset_segmentlist_node,
                                                adaptionset_supplementalproperty_node);
            if (ret < 0) {
                goto err;
            }
        }
        node = xmlNextElementSibling(node);
    }

err:
    ff_dash_free_content_protection(&encryption_info);
    xmlFree(c->adaptionset_lang);
    c->adaptionset_lang = NULL;
    return ret;
}

static int parse_programinformation(AVFormatContext *s, xmlNodePtr node)
{
    xmlChar *val = NULL;

    node = xmlFirstElementChild(node);
    while (node) {
        if (!av_strcasecmp(node->name, "Title")) {
            val = xmlNodeGetContent(node);
            if (val) {
                av_dict_set(&s->metadata, "Title", val, 0);
            }
        } else if (!av_strcasecmp(node->name, "Source")) {
            val = xmlNodeGetContent(node);
            if (val) {
                av_dict_set(&s->metadata, "Source", val, 0);
            }
        } else if (!av_strcasecmp(node->name, "Copyright")) {
            val = xmlNodeGetContent(node);
            if (val) {
                av_dict_set(&s->metadata, "Copyright", val, 0);
            }
        }
        node = xmlNextElementSibling(node);
        xmlFree(val);
    }
    return 0;
}

static int parse_manifest_l(AVFormatContext *s, DASHContext *c, const char *url, AVIOContext *in)
{
    int ret = 0;
    int close_in = 0;
    uint8_t *new_url = NULL;
    int64_t filesize = 0;
    char *buffer = NULL;
    AVDictionary *opts = NULL;
    xmlDoc *doc = NULL;
    xmlNodePtr root_element = NULL;
    xmlNodePtr node = NULL;
    xmlNodePtr period_node = NULL;
    xmlNodePtr tmp_node = NULL;
    xmlNodePtr mpd_baseurl_node = NULL;
    xmlNodePtr period_baseurl_node = NULL;
    xmlNodePtr period_segmenttemplate_node = NULL;
    xmlNodePtr period_segmentlist_node = NULL;
    xmlNodePtr adaptionset_node = NULL;
    xmlAttrPtr attr = NULL;
    char *val  = NULL;
    uint32_t period_duration_sec = 0;
    uint32_t period_start_sec = 0;

    if (!in) {
        close_in = 1;
        av_dict_copy(&opts, c->avio_opts, 0);
        ret = avio_open2(&in, url, AVIO_FLAG_READ, c->interrupt_callback, &opts);
        av_dict_free(&opts);
        if (ret < 0)
            return ret;
    }

    if (av_opt_get(in, "location", AV_OPT_SEARCH_CHILDREN, &new_url) >= 0) {
        c->base_url = av_strdup(new_url);
    } else {
        c->base_url = av_strdup(url);
    }

    filesize = avio_size(in);
    if (filesize <= 0) {
        filesize = 8 * 1024;
    }

    buffer = av_mallocz(filesize);
    if (!buffer) {
        av_free(c->base_url);
        return AVERROR(ENOMEM);
    }

    filesize = avio_read(in, buffer, filesize);
    if (filesize <= 0) {
        av_log(s, AV_LOG_ERROR, "Unable to read to offset '%s'\n", url);
        ret = AVERROR_INVALIDDATA;
    } else {
        LIBXML_TEST_VERSION

            doc = xmlReadMemory(buffer, filesize, c->base_url, NULL, 0);
        root_element = xmlDocGetRootElement(doc);
        node = root_element;

        if (!node) {
            ret = AVERROR_INVALIDDATA;
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - missing root node\n", url);
            goto cleanup;
        }

        if (node->type != XML_ELEMENT_NODE ||
            av_strcasecmp(node->name, (const char *)"MPD")) {
            ret = AVERROR_INVALIDDATA;
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - wrong root node name[%s] type[%d]\n", url, node->name, (int)node->type);
            goto cleanup;
        }

        val = xmlGetProp(node, "type");
        if (!val) {
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - missing type attrib\n", url);
            ret = AVERROR_INVALIDDATA;
            goto cleanup;
        }
        if (!av_strcasecmp(val, (const char *)"dynamic"))
            c->is_live = 1;
        xmlFree(val);

        attr = node->properties;
        while (attr) {
            val = xmlGetProp(node, attr->name);

            if (!av_strcasecmp(attr->name, (const char *)"availabilityStartTime")) {
                c->availability_start_time = get_utc_date_time_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->availability_start_time = [%"PRId64"]\n", c->availability_start_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"availabilityEndTime")) {
                c->availability_end_time = get_utc_date_time_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->availability_end_time = [%"PRId64"]\n", c->availability_end_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"publishTime")) {
                c->publish_time = get_utc_date_time_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->publish_time = [%"PRId64"]\n", c->publish_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"minimumUpdatePeriod")) {
                c->minimum_update_period = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->minimum_update_period = [%"PRId64"]\n", c->minimum_update_period);
            } else if (!av_strcasecmp(attr->name, (const char *)"timeShiftBufferDepth")) {
                c->time_shift_buffer_depth = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->time_shift_buffer_depth = [%"PRId64"]\n", c->time_shift_buffer_depth);
            } else if (!av_strcasecmp(attr->name, (const char *)"minBufferTime")) {
                c->min_buffer_time = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->min_buffer_time = [%"PRId64"]\n", c->min_buffer_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"suggestedPresentationDelay")) {
                c->suggested_presentation_delay = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->suggested_presentation_delay = [%"PRId64"]\n", c->suggested_presentation_delay);
            } else if (!av_strcasecmp(attr->name, (const char *)"mediaPresentationDuration")) {
                c->media_presentation_duration = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->media_presentation_duration = [%"PRId64"]\n", c->media_presentation_duration);
            }
            attr = attr->next;
            xmlFree(val);
        }

        tmp_node = find_child_node_by_name(node, "BaseURL");
        if (tmp_node) {
            mpd_baseurl_node = xmlCopyNode(tmp_node,1);
        } else {
            mpd_baseurl_node = xmlNewNode(NULL, "BaseURL");
        }

        // at now we can handle only one period, with the longest duration
        node = xmlFirstElementChild(node);
        while (node) {
            if (!av_strcasecmp(node->name, (const char *)"Period")) {
                period_duration_sec = 0;
                period_start_sec = 0;
                attr = node->properties;
                while (attr) {
                    val = xmlGetProp(node, attr->name);
                    if (!av_strcasecmp(attr->name, (const char *)"duration")) {
                        period_duration_sec = get_duration_insec(s, (const char *)val);
                    } else if (!av_strcasecmp(attr->name, (const char *)"start")) {
                        period_start_sec = get_duration_insec(s, (const char *)val);
                    }
                    attr = attr->next;
                    xmlFree(val);
                }
                if ((period_duration_sec) >= (c->period_duration)) {
                    period_node = node;
                    c->period_duration = period_duration_sec;
                    c->period_start = period_start_sec;
                    if (c->period_start > 0)
                        c->media_presentation_duration = c->period_duration;
                }
            } else if (!av_strcasecmp(node->name, "ProgramInformation")) {
                parse_programinformation(s, node);
            }
            node = xmlNextElementSibling(node);
        }
        if (!period_node) {
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - missing Period node\n", url);
            ret = AVERROR_INVALIDDATA;
            goto cleanup;
        }

        adaptionset_node = xmlFirstElementChild(period_node);
        while (adaptionset_node) {
            if (!av_strcasecmp(adaptionset_node->name, (const char *)"BaseURL")) {
                period_baseurl_node = adaptionset_node;
            } else if (!av_strcasecmp(adaptionset_node->name, (const char *)"SegmentTemplate")) {
                period_segmenttemplate_node = adaptionset_node;
            } else if (!av_strcasecmp(adaptionset_node->name, (const char *)"SegmentList")) {
                period_segmentlist_node = adaptionset_node;
            } else if (!av_strcasecmp(adaptionset_node->name, (const char *)"AdaptationSet")) {
                parse_manifest_adaptationset(s, c, url, adaptionset_node, mpd_baseurl_node,
                    period_baseurl_node, period_segmenttemplate_node, period_segmentlist_node);
            }
            adaptionset_node = xmlNextElementSibling(adaptionset_node);
        }
cleanup:
        /*free the document */
        xmlFreeDoc(doc);
        xmlCleanupParser();
        xmlFreeNode(mpd_baseurl_node);
    }

    av_free(new_url);
    av_free(buffer);
    if (close_in) {
        avio_close(in);
    }
    return ret;
}

/* return -1: a ahead, 1: b ahead, 0:not change */
static int compare_rep_bandwith(const void *a, const void *b) {
    struct representation *ra = *((struct representation **)(a));
    struct representation *rb = *((struct representation **)(b));

    /* bigger,move back */
    if (ra->bandwidth > rb->bandwidth) {
        return 1;
    } else if (ra->bandwidth < rb->bandwidth) {
        return -1;
    }

    return 0;
}

static int parse_manifest(AVFormatContext *s, DASHContext *c, const char *url, AVIOContext *in, int next)
{
    int ret = parse_manifest_l(s, c, url, in);
    if (ret < 0) {
        return ret;
    }
    /* one video, no need resolution change */
    if (1 == c->n_videos) {
        struct representation *rep = c->videos[0];
        ff_adaptive_playlist_context_free(&rep->adaptive);
    } else {
        qsort(c->videos, c->n_videos, sizeof(struct representation *), compare_rep_bandwith);
    }
    return ret;
}

static int64_t get_timeline_safe_start_no(AVFormatContext *s, struct representation *pls)
{
    DASHContext *c = s->priv_data;
    int64_t max_seq_count = calc_max_seg_no(pls, c) - pls->first_seq_no;
    int64_t start_num = max_seq_count - NUM_LOOKBACK_FRAGMENTS;
    if (start_num < 0) {
        return -1;
    }
    return start_num;
}

static int64_t calc_cur_seg_no(AVFormatContext *s, struct representation *pls)
{
    DASHContext *c = s->priv_data;
    int64_t num = 0;
    int64_t start_time_offset = 0;

    if (c->is_live) {
        if (pls->n_fragments) {
            av_log(s, AV_LOG_TRACE, "in n_fragments mode\n");
            num = pls->first_seq_no;
        } else if (pls->n_timelines) {
            av_log(s, AV_LOG_TRACE, "in n_timelines mode\n");
            start_time_offset = get_segment_start_time_based_on_timeline(c, pls, -1) - 60 * pls->fragment_timescale; // 60 seconds before end
            int64_t num60 = calc_next_seg_no_from_timelines(pls, start_time_offset);
            num = get_timeline_safe_start_no(s, pls);
            if (num60 > num) {
                num = num60;
            }
            if (num == -1)
                num = pls->first_seq_no;
            else
                num += pls->first_seq_no;
        } else if (pls->fragment_duration){
            av_log(s, AV_LOG_TRACE, "in fragment_duration mode fragment_timescale = %"PRId64", presentation_timeoffset = %"PRId64"\n", pls->fragment_timescale, pls->presentation_timeoffset);
            if (pls->presentation_timeoffset) {
                num = pls->first_seq_no + (((get_current_time_in_sec(c) - c->availability_start_time) * pls->fragment_timescale)-pls->presentation_timeoffset) / pls->fragment_duration - c->min_buffer_time;
            } else if (c->publish_time > 0 && !c->availability_start_time) {
                if (c->min_buffer_time) {
                    num = pls->first_seq_no + (((c->publish_time + pls->fragment_duration) - c->suggested_presentation_delay) * pls->fragment_timescale) / pls->fragment_duration - c->min_buffer_time;
                } else {
                    num = pls->first_seq_no + (((c->publish_time - c->time_shift_buffer_depth + pls->fragment_duration) - c->suggested_presentation_delay) * pls->fragment_timescale) / pls->fragment_duration;
                }
            } else {
                int64_t curr_presentation_time = ((get_current_time_in_sec(c) - c->availability_start_time) - c->suggested_presentation_delay);
                if (!(c->suggested_presentation_delay)) {
                    int64_t presentation_delay = 0;
                    if (pls->fragment_timescale > 0) {
                        presentation_delay = NUM_LOOKBACK_FRAGMENTS * pls->fragment_duration / pls->fragment_timescale;
                    }
                    if (curr_presentation_time > presentation_delay) {
                        curr_presentation_time -= presentation_delay;
                    }
                }
                num = pls->first_seq_no + (curr_presentation_time * pls->fragment_timescale) / pls->fragment_duration;
            }
        }
    } else {
        num = pls->first_seq_no;
    }
    return num;
}

static int64_t calc_min_seg_no(AVFormatContext *s, struct representation *pls)
{
    DASHContext *c = s->priv_data;
    int64_t num = 0;

    if (c->is_live && pls->fragment_duration) {
        av_log(s, AV_LOG_TRACE, "in live mode\n");
        num = pls->first_seq_no + (((get_current_time_in_sec(c) - c->availability_start_time) - c->time_shift_buffer_depth) * pls->fragment_timescale) / pls->fragment_duration;
        if (!(pls->presentation_timeoffset) &&
            !(c->suggested_presentation_delay) &&
            !(c->publish_time)) {
            int64_t curr_presentation_time = ((get_current_time_in_sec(c) - c->availability_start_time) - c->suggested_presentation_delay);
            int64_t presentation_delay = 0;
            if (pls->fragment_timescale > 0) {
                presentation_delay = NUM_LOOKBACK_FRAGMENTS * pls->fragment_duration / pls->fragment_timescale;
            }
            if (curr_presentation_time > presentation_delay) {
                curr_presentation_time -= presentation_delay;
            }
            num = pls->first_seq_no + (curr_presentation_time * pls->fragment_timescale) / pls->fragment_duration;
            if (num < pls->first_seq_no) {
                num = pls->first_seq_no;
            }
        }
    } else {
        num = pls->first_seq_no;
    }
    return num;
}

static int64_t calc_max_seg_no(struct representation *pls, DASHContext *c)
{
    int64_t num = 0;

    if (pls->n_fragments) {
        num = pls->first_seq_no + pls->n_fragments - 1;
    } else if (pls->n_timelines) {
        int i = 0;
        num = pls->first_seq_no + pls->n_timelines - 1;
        for (i = 0; i < pls->n_timelines; i++) {
            if (pls->timelines[i]->repeat == -1) {
                int length_of_each_segment = pls->timelines[i]->duration / pls->fragment_timescale;
                num =  c->period_duration / length_of_each_segment;
            } else {
                num += pls->timelines[i]->repeat;
            }
        }
    } else if (c->is_live && pls->fragment_duration) {
        num = pls->first_seq_no + (((get_current_time_in_sec(c) - c->availability_start_time)) * pls->fragment_timescale)  / pls->fragment_duration;
    } else if (pls->fragment_duration) {
        num = pls->first_seq_no + (c->media_presentation_duration * pls->fragment_timescale) / pls->fragment_duration;
    }

    return num;
}

static int64_t calc_max_available_time(struct representation *pls, DASHContext *c)
{
    if (!pls->timelines || !pls->n_timelines || !c->is_live) {
        return -1;
    }
     struct timeline *last_timeline =
        pls->timelines[pls->n_timelines - 1];

    if (last_timeline->repeat == -1  ||
        last_timeline->starttime <  0 ||
        last_timeline->duration  <= 0) {
        return -1;
    }
    if (last_timeline->repeat == 0) {
        return last_timeline->starttime;
    }

    int64_t max_time = last_timeline->starttime + last_timeline->duration * last_timeline->repeat - 1;
    return max_time;
}

static void move_timelines(struct representation *rep_src, struct representation *rep_dest, DASHContext *c)
{
    if (rep_dest && rep_src) {
        free_timelines_list(rep_dest);
        rep_dest->timelines    = rep_src->timelines;
        rep_dest->n_timelines  = rep_src->n_timelines;
        rep_dest->first_seq_no = rep_src->first_seq_no;
        rep_dest->last_seq_no = calc_max_seg_no(rep_dest, c);
        rep_src->timelines = NULL;
        rep_src->n_timelines = 0;
        rep_dest->cur_seq_no = rep_src->cur_seq_no;
    }
}

static void move_segments(struct representation *rep_src, struct representation *rep_dest, DASHContext *c)
{
    if (rep_dest && rep_src) {
        free_fragment_list(rep_dest);
        if (rep_src->start_number > (rep_dest->start_number + rep_dest->n_fragments))
            rep_dest->cur_seq_no = 0;
        else
            rep_dest->cur_seq_no += rep_src->start_number - rep_dest->start_number;
        rep_dest->fragments    = rep_src->fragments;
        rep_dest->n_fragments  = rep_src->n_fragments;
        rep_dest->parent  = rep_src->parent;
        rep_dest->last_seq_no = calc_max_seg_no(rep_dest, c);
        rep_src->fragments = NULL;
        rep_src->n_fragments = 0;
    }
}

static void move_dash_ctx_swap_timelines(
    struct representation *rep_src,
    struct representation *rep_dest, DASHContext *c)
{
    if (!rep_dest || !rep_src) {
        return;
    }

    free_timelines_list(rep_dest);
    for (int i = 0; i < rep_src->n_timelines; i++) {
        if (!rep_src->timelines[i]) {
            continue;
        }
        struct timeline *tml = av_mallocz(sizeof(struct timeline));
        if (!tml) {
            continue;
        }
        *tml = *(rep_src->timelines[i]);
        dynarray_add(&rep_dest->timelines, &rep_dest->n_timelines, tml);
    }

    rep_dest->first_seq_no = rep_src->first_seq_no;
    rep_dest->cur_seq_no   = rep_src->cur_seq_no;
    rep_dest->last_seq_no  = rep_src->last_seq_no;
}

static void move_dash_ctx_reset_segments(
    struct representation *rep_src,
    struct representation *rep_dest, DASHContext *c)
{
    if (!rep_dest || !rep_src) {
        return;
    }

    free_fragment_list(rep_dest);
    for (int i = 0; i < rep_src->n_fragments; i++) {
        if (!rep_src->fragments[i]) {
            continue;
        }

        struct fragment *seg = av_mallocz(sizeof(struct fragment));
        if (!seg) {
            continue;
        }
        seg->url = av_strdup(rep_src->fragments[i]->url);
        if (!seg->url) {
            av_free(seg);
            continue;
        }
        seg->size  = rep_src->fragments[i]->size;
        seg->url_offset = rep_src->fragments[i]->url_offset;
        dynarray_add(&rep_dest->fragments, &rep_dest->n_fragments, seg);
    }
    rep_dest->first_seq_no = rep_src->first_seq_no;
	rep_dest->cur_seq_no   = rep_src->cur_seq_no;
    rep_dest->last_seq_no  = rep_src->last_seq_no;
}

static void refresh_update_timeline_cur_no(DASHContext *c,
    struct representation *oldrep, struct representation *newrep)
{
    // calc current time
    int64_t currentTime = get_segment_start_time_based_on_timeline(c, oldrep, oldrep->cur_seq_no) / oldrep->fragment_timescale;
    // update segments
    int64_t cur_seq_no = calc_next_seg_no_from_timelines(newrep, currentTime * oldrep->fragment_timescale - 1);
    if (-1 == cur_seq_no) {
        newrep->cur_seq_no = newrep->first_seq_no;
    } else {
        newrep->cur_seq_no = oldrep->cur_seq_no;
    }
}

static int refresh_manifest(AVFormatContext *s, DASHContext *ic, int next)
{
    int ret = 0, i;
    DASHContext *c = ic ? ic : s->priv_data;
    // save current context
    int n_videos = c->n_videos;
    struct representation **videos = c->videos;
    int n_audios = c->n_audios;
    struct representation **audios = c->audios;
    int n_subtitles = c->n_subtitles;
    struct representation **subtitles = c->subtitles;
    char *base_url = c->base_url;

    c->base_url = NULL;
    c->n_videos = 0;
    c->videos = NULL;
    c->n_audios = 0;
    c->audios = NULL;
    c->n_subtitles = 0;
    c->subtitles = NULL;
    ret = parse_manifest(s, c, s->url, NULL, next);
    if (ret)
        goto finish;

    if (c->n_videos != n_videos) {
        av_log(c, AV_LOG_ERROR,
               "new manifest has mismatched no. of video representations, %d -> %d\n",
               n_videos, c->n_videos);
        return AVERROR_INVALIDDATA;
    }
    if (c->n_audios != n_audios) {
        av_log(c, AV_LOG_ERROR,
               "new manifest has mismatched no. of audio representations, %d -> %d\n",
               n_audios, c->n_audios);
        return AVERROR_INVALIDDATA;
    }
    if (c->n_subtitles != n_subtitles) {
        av_log(c, AV_LOG_ERROR,
               "new manifest has mismatched no. of subtitles representations, %d -> %d\n",
               n_subtitles, c->n_subtitles);
        return AVERROR_INVALIDDATA;
    }

    for (i = 0; i < n_videos; i++) {
        struct representation *cur_video = videos[i];
        struct representation *ccur_video = c->videos[i];
        if (cur_video->timelines) {
            refresh_update_timeline_cur_no(c, cur_video, ccur_video);
            if (ccur_video->cur_seq_no >= 0) {
                move_timelines(ccur_video, cur_video, c);
            }
        }
        if (cur_video->fragments) {
            move_segments(ccur_video, cur_video, c);
        }
    }
    for (i = 0; i < n_audios; i++) {
        struct representation *cur_audio = audios[i];
        struct representation *ccur_audio = c->audios[i];
        if (cur_audio->timelines) {
            refresh_update_timeline_cur_no(c, cur_audio, ccur_audio);
            if (ccur_audio->cur_seq_no >= 0) {
                move_timelines(ccur_audio, cur_audio, c);
            }
        }
        if (cur_audio->fragments) {
            move_segments(ccur_audio, cur_audio, c);
        }
    }

finish:
    // restore context
    if (c->base_url)
        av_free(base_url);
    else
        c->base_url = base_url;
    /* free newly allocated rep inforamtion */
    if (c->subtitles)
        free_subtitle_list(c);
    if (c->audios)
        free_audio_list(c);
    if (c->videos)
        free_video_list(c);
    /* still use old rep context with new fragment information, updated by move_xx function */
    c->n_subtitles = n_subtitles;
    c->subtitles = subtitles;
    c->n_audios = n_audios;
    c->audios = audios;
    c->n_videos = n_videos;
    c->videos = videos;
    return ret;
}

static struct fragment *get_current_fragment_l(
    struct representation *pls, int64_t *cur_seq_no_in)
{
    int64_t min_seq_no = 0;
    int64_t max_seq_no = 0;
    struct fragment *seg = NULL;
    struct fragment *seg_ptr = NULL;
    DASHContext *c = pls->parent->priv_data;

    int64_t cur_seq_no = *cur_seq_no_in;
    while ((!ff_check_interrupt(c->interrupt_callback) && pls->n_fragments > 0)) {
        if (cur_seq_no < pls->n_fragments) {
            seg_ptr = pls->fragments[cur_seq_no];
            seg = av_mallocz(sizeof(struct fragment));
            if (!seg) {
                return NULL;
            }
            seg->url = av_strdup(seg_ptr->url);
            if (!seg->url) {
                av_free(seg);
                return NULL;
            }
            seg->size = seg_ptr->size;
            seg->url_offset = seg_ptr->url_offset;
            return seg;
        } else if (c->is_live) {
            /* not use thread to refresh manifest when init first stream */
            if (!c->use_thread/* || ADAPTIVE_STAGE_INIT >= c->async.stage*/) {
                refresh_manifest(pls->parent, NULL, 0);
            }
        } else {
            break;
        }
    }
    if (c->is_live) {
        min_seq_no = calc_min_seg_no(pls->parent, pls);
        max_seq_no = calc_max_seg_no(pls, c);

        if (pls->timelines || pls->fragments) {
            if (!c->use_thread/* || ADAPTIVE_STAGE_INIT >= c->async.stage*/) {
                refresh_manifest(pls->parent, NULL, 0);
            }
        }
        if (cur_seq_no <= min_seq_no) {
            av_log(pls->parent, AV_LOG_VERBOSE,
                "old fragment: cur[%"PRId64"] min[%"PRId64"] max[%"PRId64"], playlist %d\n",
                    (int64_t)cur_seq_no, min_seq_no, max_seq_no, (int)pls->rep_idx);
            *cur_seq_no_in = cur_seq_no = calc_cur_seg_no(pls->parent, pls);
        } else if (cur_seq_no > max_seq_no) {
            av_log(pls->parent, AV_LOG_VERBOSE,
                "new fragment: min[%"PRId64"] max[%"PRId64"], playlist %d\n",
                    min_seq_no, max_seq_no, (int)pls->rep_idx);
        }
        seg = av_mallocz(sizeof(struct fragment));
        if (!seg) {
            return NULL;
        }
    } else if (cur_seq_no <= pls->last_seq_no) {
        seg = av_mallocz(sizeof(struct fragment));
        if (!seg) {
            return NULL;
        }
    }
    if (seg) {
        char *tmpfilename= av_mallocz(c->max_url_size);
        if (!tmpfilename) {
            return NULL;
        }
        int64_t stime = get_segment_start_time_based_on_timeline(c, pls, cur_seq_no);
        ff_dash_fill_tmpl_params(tmpfilename, c->max_url_size, pls->url_template, 0, cur_seq_no, 0, stime);
        seg->url = av_strireplace(pls->url_template, pls->url_template, tmpfilename);
        if (!seg->url) {
            av_log(pls->parent, AV_LOG_WARNING, "Unable to resolve template url '%s', try to use origin template\n", pls->url_template);
            seg->url = av_strdup(pls->url_template);
            if (!seg->url) {
                av_log(pls->parent, AV_LOG_ERROR, "Cannot resolve template url '%s'\n", pls->url_template);
                av_free(tmpfilename);
                return NULL;
            }
        }
        av_free(tmpfilename);
        seg->size = -1;
    }

    return seg;
}

static struct fragment *get_current_fragment(struct representation *pls)
{
    if (pls->cur_seg) {
        return pls->cur_seg;
    }
    return get_current_fragment_l(pls, &(pls->cur_seq_no));
}

static struct fragment *get_next_fragment(struct representation *pls)
{
    if (pls->next_seg) {
        free_fragment(&pls->next_seg);
    }

    struct fragment *seg = NULL;
    AdaptiveInputContext *aic = &pls->adaptive_input_ctx;
    int64_t cur_seq_no = aic->next_fragment_seq_no;
    if (-1 == cur_seq_no) {
        ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_NONE);
        return NULL;
    }
    seg = get_current_fragment_l(pls, &cur_seq_no);
    if (!seg) {
        return NULL;
    }

    ff_adaptive_reinit_input_context(pls->parent, aic);
    aic->next_fragment_seq_no = cur_seq_no;

    av_log(NULL, AV_LOG_VERBOSE,
        "[%p][%p] Get next seg %lld first:%lld url:%s\n",
            pls, seg, cur_seq_no, pls->first_seq_no, (seg) ? seg->url : "NULL");
    return seg;
}

static int read_from_url_io(DASHContext *c,
    AVIOContext *input, unsigned char *buf, int buf_size)
{
    int ret = 0;

    unsigned int lock = (ADAPTIVE_STAGE_ASYNC_READ_HEADER >= c->async.stage) ? 0 : 1;
    if (lock) {
        MANIFEST_UNLOCK(c);
    }
    if (!(ret = ff_check_interrupt(c->interrupt_callback))) {
        ret = avio_read(input, buf, buf_size);
    }

    if (lock) {
        MANIFEST_LOCK(c);
    }
    return ret;
}

static int read_from_url(DASHContext *c,
    struct representation *pls,
    struct fragment *seg, uint8_t *buf, int buf_size)
{
    int ret;

    /* limit read if the fragment was only a part of a file */
    if (seg->size >= 0)
        buf_size = FFMIN(buf_size, pls->cur_seg_size - pls->cur_seg_offset);

    ret = read_from_url_io(c, pls->input, buf, buf_size);
    if (ret > 0)
        pls->cur_seg_offset += ret;

    return ret;
}

static int open_input_l(DASHContext *c,
    struct representation *pls, struct fragment *seg, char *base_url, int next)
{
    AVDictionary *opts = NULL;
    char *url = NULL;
    int ret = 0;

    url = av_mallocz(c->max_url_size);
    if (!url) {
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    if (seg->size >= 0) {
        /* try to restrict the HTTP request to the part we want
         * (if this is in fact a HTTP request) */
        av_dict_set_int(&opts, "offset", seg->url_offset, 0);
        av_dict_set_int(&opts, "end_offset", seg->url_offset + seg->size, 0);
    }

    ff_make_absolute_url(url, c->max_url_size, base_url, seg->url);
    av_log(pls->parent, AV_LOG_INFO, "DASH request for url '%s', offset %"PRId64", playlist %d\n",
           url, seg->url_offset, pls->rep_idx);

    AVIOContext *input = NULL;
    ret = open_url(pls->parent, &input, pls, url, c->avio_opts, opts);
    av_log(pls->parent, AV_LOG_VERBOSE, "DASH request url %s playlist %d\n", av_err2str(ret), pls->rep_idx);

cleanup:
    av_free(url);
    av_dict_free(&opts);
    if (next) {
        AdaptiveInputContext *aic = &pls->adaptive_input_ctx;
        aic->input_next = input;
        aic->next_fragment_size = seg->size;
    } else {
        pls->input = input;
        pls->cur_seg_offset = 0;
        pls->cur_seg_size = seg->size;
    }
    return ret;
}

static int open_input(DASHContext *c,
    struct representation *pls, struct fragment *seg, char *base_url, int next)
{
    int ret = 0;

    if (!base_url) {
        base_url = c->base_url;
    }

    unsigned int lock = (next || (ADAPTIVE_STAGE_ASYNC_READ_HEADER >= c->async.stage)) ? 0 : 1;
    if (lock) {
        MANIFEST_UNLOCK(c);
    }
    ret = open_input_l(c, pls, seg, base_url, next);
    if (lock) {
        MANIFEST_LOCK(c);
    }
    return ret;
}

static int update_init_section(struct representation *pls)
{
    static const int max_init_section_size = 1024 * 1024;
    DASHContext *c = pls->parent->priv_data;
    int64_t sec_size;
    int64_t urlsize;
    int ret;

    if (!pls->init_section || pls->init_sec_buf)
        return 0;

    ret = open_input(c, pls, pls->init_section, NULL, 0);
    if (ret < 0) {
        av_log(pls->parent, AV_LOG_WARNING,
               "Failed to open an initialization section in playlist %d\n",
               pls->rep_idx);
        return ret;
    }

    if (pls->init_section->size >= 0)
        sec_size = pls->init_section->size;
    else if ((urlsize = avio_size(pls->input)) >= 0)
        sec_size = urlsize;
    else
        sec_size = max_init_section_size;

    av_log(pls->parent, AV_LOG_DEBUG,
           "Downloading an initialization section of size %"PRId64"\n",
           sec_size);

    sec_size = FFMIN(sec_size, max_init_section_size);
    av_fast_malloc(&pls->init_sec_buf, &pls->init_sec_buf_size, sec_size);

    ret = read_from_url(c, pls, pls->init_section,
        pls->init_sec_buf, pls->init_sec_buf_size);
    ff_adaptive_playlist_io_deinit(pls->parent, pls->adaptive, &pls->input);

    if (ret < 0)
        return ret;

    pls->init_sec_data_len = ret;
    pls->init_sec_buf_read_offset = 0;

    return 0;
}

static int64_t seek_data(void *opaque, int64_t offset, int whence)
{
    struct representation *v = opaque;
    if (v->n_fragments && !v->init_sec_data_len) {
        if (v->input && v->input->seek && 1 == v->n_fragments && AVSEEK_SIZE == whence) {
            return v->input->seek(v->input->opaque, offset, whence);
        }
        return avio_seek(v->input, offset, whence);
    }

    return AVERROR(ENOSYS);
}

static void set_next_input_status(DASHContext *c, struct representation *v)
{
    AdaptiveInputContext *aic = &v->adaptive_input_ctx;
    aic->next_fragment_seq_no = v->cur_seq_no + 1;
    ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_NONE);
    if (!c->use_thread ||
        (AVMEDIA_TYPE_VIDEO != v->type && AVMEDIA_TYPE_AUDIO != v->type)) {
        return;
    }

    if (c->is_live) {
        if (v->timelines && v->fragment_timescale > 0) {
            int64_t update_interval_usec = get_update_interval_usec(c);
            int64_t max_available_time = calc_max_available_time(v, c) - 1;
            int64_t next_seg_time = get_segment_start_time_based_on_timeline(c, v, aic->next_fragment_seq_no);
            av_log(c, AV_LOG_VERBOSE,
                "[%p] max available time:%lld next seg time:%lld\n", v, max_available_time, next_seg_time);
            /* too small fragment need async */
            if (next_seg_time > max_available_time && update_interval_usec >= (4 * AV_TIME_BASE)) {
                ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_NONE);
                return;
            }
        }
    }
    ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_ASYNC);
}

static void read_data_swap_input(struct representation *v)
{
    AdaptiveInputContext *aic = &v->adaptive_input_ctx;
    FFSWAP(AVIOContext *, v->input, aic->input_next);
    FFSWAP(struct fragment *, v->cur_seg, v->next_seg);
    v->cur_seq_no     = aic->next_fragment_seq_no;
    v->cur_seg_size   = aic->next_fragment_size;
    v->cur_seg_offset = 0;
    aic->input_next    = NULL;
    aic->next_fragment_size   = -1;
    aic->next_fragment_seq_no = -1;
    av_log(NULL, AV_LOG_VERBOSE, "[%p][%p] Swap next %lld url:%s\n",
        v, v->cur_seg, v->cur_seq_no, (v->cur_seg) ? v->cur_seg->url : "NULL");
}

static int read_data_open_input_async(DASHContext *c, struct representation *v)
{
    int ret = 0;
    if ((AVMEDIA_TYPE_VIDEO != v->type && AVMEDIA_TYPE_AUDIO != v->type) ||
        ADAPTIVE_INPUT_NEXT_NONE == ff_adaptive_get_next_input_status(&(v->adaptive_input_ctx))) {
        return 0;
    }

    AVIOInterruptCB *interrupt_callback =
        (v->ctx) ? (&(v->ctx->interrupt_callback)) : c->interrupt_callback;

    MANIFEST_UNLOCK(c);
    int64_t start_time = av_gettime();
    ret = ff_adaptive_read_data_wait_async(
            &v->adaptive_input_ctx, interrupt_callback);
    av_log(NULL, AV_LOG_INFO, "[%p] wait async time:%lldus\n", v, av_gettime() - start_time);
    MANIFEST_LOCK(c);
    if (ret < 0) {
        return ret;
    }

    if (!(v->adaptive_input_ctx.input_next) ||
        ADAPTIVE_INPUT_NEXT_NONE ==
            ff_adaptive_get_next_input_status(&(v->adaptive_input_ctx))) {
        return 0;
    }

    ret = AVERROR(EAGAIN);
    if (ADAPTIVE_INPUT_NEXT_READY ==
            ff_adaptive_get_next_input_status(&(v->adaptive_input_ctx))) {
        read_data_swap_input(v);
        set_next_input_status(c, v);
        ret = 1;
    }

    return ret;
}

static void increase_cur_seq_no(DASHContext *c, struct representation *pls)
{
    (void) c;
    pls->cur_seq_no += 1;
}

static int read_data_open_input(DASHContext *c, struct representation *v)
{
    int ret = 0;

restart:
    if (v->input) {
        return 0;
    }

    free_fragment(&v->cur_seg);
    if ((ret = dash_interrupt_callback(v))) {
        goto end;
    }

    ret = read_data_open_input_async(c, v);
    if (ret > 0) {
        goto end;
    }

    /* load/update Media Initialization Section, if any */
    ret = update_init_section(v);
    if (ret) {
        goto end;
    }

    v->cur_seg = get_current_fragment(v);
    if (!v->cur_seg) {
        ret = AVERROR_EOF;
        goto end;
    }

    /* v->cur_seg duplicate from rep, don't care refresh_manifest when update */
    ret = open_input(c, v, v->cur_seg, NULL, 0);
    if (ret < 0) {
        av_log(v->parent, AV_LOG_WARNING, "Failed to open %s fragment of playlist %d\n", v->cur_seg->url, v->rep_idx);
        increase_cur_seq_no(c, v);
        /* At most try 4 seg */
        if (v->last_seq_no > 0 &&
            v->cur_seq_no > v->last_seq_no + 4) {
            goto end;
        }
        goto restart;
    }
end:
    if (v->input) {
        set_next_input_status(c, v);
        (void) ff_adaptive_playlist_io_init(v->parent, v->adaptive, v->input);
    }
    return ret;
}

static int read_data_l(DASHContext *c,
    struct representation *v, uint8_t *buf, int buf_size)
{
    int ret = read_data_open_input(c, v);
    if (ret < 0) {
        goto end;
    }

    if (v->init_sec_buf_read_offset < v->init_sec_data_len) {
        /* Push init section out first before first actual fragment */
        unsigned int copy_size = FFMIN(v->init_sec_data_len - v->init_sec_buf_read_offset, (unsigned int) buf_size);
        memcpy(buf, v->init_sec_buf, copy_size);
        v->init_sec_buf_read_offset += copy_size;
        ret = copy_size;
        goto end;
    }

    /* check the v->cur_seg, if it is null, get current and double check if the new v->cur_seg*/
    if (!v->cur_seg) {
        v->cur_seg = get_current_fragment(v);
    }
    if (!v->cur_seg) {
        ret = AVERROR_EOF;
        goto end;
    }
    ret = read_from_url(c, v, v->cur_seg, buf, buf_size);
    if (ret > 0)
        goto end;

    if (c->is_live || v->cur_seq_no < v->last_seq_no) {
        if (!v->is_restart_needed)
            increase_cur_seq_no(c, v);
        v->is_restart_needed = 1;
    }

end:
    return ret;
}

static int read_data(void *opaque, uint8_t *buf, int buf_size)
{
    int ret = 0;
    int retry_cnt = RETRY_CNT_READ_DATA;
    struct representation *v = opaque;
    DASHContext *c = v->parent->priv_data;
    int server_error_code = 0; /* some server not return error code after retry */

retry:
    if (ff_check_interrupt(c->interrupt_callback)) {
        goto end;
    }
    v->read_start_time = av_gettime();
    ret = read_data_l(c, v, buf, buf_size);
    if (v->input && AVMEDIA_TYPE_VIDEO == v->type) {
        int64_t qos_net_bitrate = 0;
        if (av_opt_get_int(v->input,
            "qos_net_bitrate", AV_OPT_SEARCH_CHILDREN, &qos_net_bitrate) >= 0) {
            c->qos_net_bitrate = qos_net_bitrate;
        }
    }

    /* VOD retry when read_data_open_input */
    if (ret < 0 && AVERROR_EXIT != ret && AVERROR_EOF != ret) {
        if (!server_error_code) {
            server_error_code = ret;
        }
        if (retry_cnt-- > 0 && c->is_live && 0 != v->read_start_time) {
            av_usleep(100 * 1000);
            goto retry;
        }
    }
end:
    v->read_start_time = -1;
    ff_adaptive_report_server_error(v->parent, server_error_code, ret);
    return ret;
}

static int save_avio_options(AVFormatContext *s)
{
    DASHContext *c = s->priv_data;
    const char *opts[] = {
        "headers", "user_agent", "cookies", "http_proxy", "referer", "rw_timeout", NULL };
    const char **opt = opts;
    uint8_t *buf = NULL;
    int ret = 0;

    while (*opt) {
        if (av_opt_get(s->pb, *opt, AV_OPT_SEARCH_CHILDREN, &buf) >= 0) {
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
           "A DASH playlist item '%s' referred to an external file '%s'. "
           "Opening this file was forbidden for security reasons\n",
           s->url, url);
    return AVERROR(EPERM);
}

static void close_demux_for_component(struct representation *pls)
{
    /* note: the internal buffer could have changed */
    av_freep(&pls->pb.buffer);
    memset(&pls->pb, 0x00, sizeof(AVIOContext));
    pls->ctx->pb = NULL;

    ff_adaptive_playlist_context_close(pls->adaptive);
    avformat_close_input(&pls->ctx);
    pls->ctx = NULL;
}

static void component_stream_inject_side_data(AVFormatContext *s,
    AVFormatContext *ctx, AVEncryptionInitInfo *encryption_info)
{
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

static int reopen_demux_for_component(AVFormatContext *s, struct representation *pls)
{
    unsigned int i;
    DASHContext *c = s->priv_data;
    ff_const59 AVInputFormat *in_fmt = NULL;
    AVDictionary  *in_fmt_opts = NULL;
    uint8_t *avio_ctx_buffer  = NULL;
    int ret = 0;

    if (pls->ctx) {
        close_demux_for_component(pls);
    }

    if (ff_check_interrupt(c->interrupt_callback)) {
        ret = AVERROR_EXIT;
        goto fail;
    }

    if (!(pls->ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    avio_ctx_buffer  = av_malloc(INITIAL_BUFFER_SIZE);
    if (!avio_ctx_buffer ) {
        ret = AVERROR(ENOMEM);
        avformat_free_context(pls->ctx);
        pls->ctx = NULL;
        goto fail;
    }
    if (c->is_live) {
        ffio_init_context(&pls->pb, avio_ctx_buffer , INITIAL_BUFFER_SIZE, 0, pls, read_data, NULL, NULL);
    } else {
        ffio_init_context(&pls->pb, avio_ctx_buffer , INITIAL_BUFFER_SIZE, 0, pls, read_data, NULL, seek_data);
    }

    pls->pb.seekable = 0;
    if ((ret = ff_copy_whiteblacklists(pls->ctx, s)) < 0)
        goto fail;

    pls->ctx->flags = AVFMT_FLAG_CUSTOM_IO;
    pls->ctx->probesize = s->probesize > 0 ? s->probesize : 1024 * 4;
    pls->ctx->max_analyze_duration = s->max_analyze_duration > 0 ? s->max_analyze_duration : 4 * AV_TIME_BASE;
    pls->ctx->interrupt_callback.opaque   = (void *) pls;
    pls->ctx->interrupt_callback.callback = dash_interrupt_callback;
    ret = av_probe_input_buffer(&pls->pb, &in_fmt, "", NULL, 0, 0);
    if (ret < 0) {
        av_log(s, AV_LOG_ERROR, "Error when loading first fragment, playlist %d\n", (int)pls->rep_idx);
        avformat_free_context(pls->ctx);
        pls->ctx = NULL;
        goto fail;
    }

    pls->ctx->pb = &pls->pb;
    pls->ctx->io_open  = nested_io_open;

    // provide additional information from mpd if available
    ret = avformat_open_input(&pls->ctx, "", in_fmt, &in_fmt_opts); //pls->init_section->url
    av_dict_free(&in_fmt_opts);
    if (ret < 0)
        goto fail;
    component_stream_inject_side_data(s, pls->ctx, pls->encryption_info);
    if (pls->n_fragments) {
#if FF_API_R_FRAME_RATE
        if (pls->framerate.den) {
            for (i = 0; i < pls->ctx->nb_streams; i++)
                pls->ctx->streams[i]->r_frame_rate = pls->framerate;
        }
#endif
        ret = avformat_find_stream_info(pls->ctx, NULL);
        if (ret < 0)
            goto fail;
    }
    //av_dump_format(pls->ctx, i, pls->cur_seg->url, 0);
    ff_adaptive_playlist_context_open(pls->ctx, pls->adaptive, pls->n_fragments);
fail:
    return ret;
}

static int open_demux_for_component(AVFormatContext *s, struct representation *pls)
{
    int ret = 0;

    pls->parent = s;
    pls->n_assoc_stream = 0;
    pls->cur_seq_no = calc_cur_seg_no(s, pls);

    if (!pls->last_seq_no) {
        pls->last_seq_no = calc_max_seg_no(pls, s->priv_data);
    }

    ret = reopen_demux_for_component(s, pls);
    if (ret < 0) {
        goto fail;
    }

    ff_adaptive_playlist_check_stream(pls->adaptive, &pls->framerate);
    pls->n_assoc_stream = pls->ctx->nb_streams;
    return 0;
fail:
    return ret;
}

static int is_common_init_section_exist(struct representation **pls, int n_pls)
{
    struct fragment *first_init_section = pls[0]->init_section;
    char *url =NULL;
    int64_t url_offset = -1;
    int64_t size = -1;
    int i = 0;

    if (first_init_section == NULL || n_pls == 0)
        return 0;

    url = first_init_section->url;
    url_offset = first_init_section->url_offset;
    size = pls[0]->init_section->size;
    for (i=0;i<n_pls;i++) {
        if (av_strcasecmp(pls[i]->init_section->url,url) || pls[i]->init_section->url_offset != url_offset || pls[i]->init_section->size != size) {
            return 0;
        }
    }
    return 1;
}

static int copy_init_section(struct representation *rep_dest, struct representation *rep_src)
{
    rep_dest->init_sec_buf = av_mallocz(rep_src->init_sec_buf_size);
    if (!rep_dest->init_sec_buf) {
        av_log(rep_dest->ctx, AV_LOG_WARNING, "Cannot alloc memory for init_sec_buf\n");
        return AVERROR(ENOMEM);
    }
    memcpy(rep_dest->init_sec_buf, rep_src->init_sec_buf, rep_src->init_sec_data_len);
    rep_dest->init_sec_buf_size = rep_src->init_sec_buf_size;
    rep_dest->init_sec_data_len = rep_src->init_sec_data_len;
    rep_dest->cur_timestamp = rep_src->cur_timestamp;

    return 0;
}

static void move_metadata(AVStream *st, const char *key, char **value)
{
    if (*value) {
        av_dict_set(&st->metadata, key, *value, AV_DICT_DONT_STRDUP_VAL);
        *value = NULL;
    }
}

static void set_representation_stream_metadata(struct representation *rep)
{
    for (int j = 0; j < rep->n_assoc_stream; j++) {
        AVStream *exp_st = rep->assoc_stream[j];
        AVStream *rep_st = rep->ctx->streams[j];
        AVDictionary *metadata = NULL;
        if (rep->bandwidth > 0) {
            av_dict_set_int(&exp_st->metadata, "variant_bitrate", rep->bandwidth, 0);
        }
        if (rep->id[0]) {
            av_dict_set(&exp_st->metadata, "id", rep->id, 0);
        }

        AVDictionaryEntry *lang =
            av_dict_get(rep_st->metadata, "language", NULL, 0);
        if (lang) {
            av_dict_set(&exp_st->metadata, "language", lang->value, 0);
        } else if (rep->lang) {
            move_metadata(exp_st, "language", &rep->lang);
        }
    }
}

static int read_header_finished(AVFormatContext *s)
{
    DASHContext *c = s->priv_data;
    return (ADAPTIVE_STAGE_INIT_FAILED == c->async.stage ||
        ADAPTIVE_STAGE_ALL_STREAM_READY == c->async.stage);
}

static int read_header_export_stream(
    AVFormatContext *s, struct representation *pls)
{
    unsigned int i;
    int ret = 0;

    if (!pls || !pls->ctx || !pls->ctx->nb_streams) {
        return 0;
    }

    pls->assoc_stream =
        av_realloc_array(pls->assoc_stream,
        pls->n_assoc_stream, sizeof(AVStream *));
    if (!pls->assoc_stream) {
        return AVERROR(ENOMEM);
    }

    pls->assoc_stream_index = s->nb_streams;
    for (i = 0; i < pls->ctx->nb_streams; i++) {
        AVStream *st = avformat_new_stream(s, NULL);
        AVStream *ist = pls->ctx->streams[i];
        if (!st) {
            ret = AVERROR(ENOMEM);
            return ret;
        }
        st->id = i;
        st->avg_frame_rate = st->r_frame_rate = ist->avg_frame_rate;
        avcodec_parameters_copy(st->codecpar, ist->codecpar);
        avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
        pls->assoc_stream[i] = st;
    }
    return 0;
}

static int read_header_handle_assoc_stream(
    AVFormatContext *s, struct representation *rep)
{
    for (int j = 0; j < rep->n_assoc_stream; j++) {
        av_program_add_stream_index(s, 0, rep->assoc_stream_index + j);
        /* set by read_header_export_stream */
        /* rep->assoc_stream[j] = s->streams[rep->assoc_stream_index + j]; */
    }
    set_representation_stream_metadata(rep);

    AVAdaptiveList list = {
        .enable         = 1,
        .playlist_index = rep->stream_index,
        .type       = rep->type,
        .nb_streams = rep->n_assoc_stream,
        .width  = s->streams[rep->assoc_stream_index]->codecpar->width,
        .height = s->streams[rep->assoc_stream_index]->codecpar->height,
        .bandwidth = rep->bandwidth,
        .framerate = rep->framerate,
        .streams   = rep->assoc_stream,
    };
    (void) ff_adaptive_list_init(s, &list);
    return 0;
}

static int read_header_expose_stream(
    AVFormatContext *s, struct representation *rep)
{
    int ret;
    ret = read_header_export_stream(s, rep);
    if (ret) {
        return ret;
    }
    ret = read_header_handle_assoc_stream(s, rep);
    if (ret) {
        return ret;
    }
    return 0;
}

static int read_header_update_stream_avctx(
    AVFormatContext *s, unsigned int updated_streams)
{
    int ret = 0;
    unsigned int i = 0;
    for (i = updated_streams; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];

        if (!st->internal->need_context_update)
            continue;

        /* close parser, because it depends on the codec */
        if (st->parser && st->internal->avctx->codec_id != st->codecpar->codec_id) {
            av_parser_close(st->parser);
            st->parser = NULL;
        }

        /* update internal codec context, for the parser */
        ret = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
        if (ret < 0)
            return ret;

#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
        /* update deprecated public codec context */
        ret = avcodec_parameters_to_context(st->codec, st->codecpar);
        if (ret < 0)
            return ret;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        st->internal->need_context_update = 0;
    }
    return 0;
}

static int read_header_expose_streams(
    AVFormatContext *s, int start, int end)
{
    int i, ret;
    DASHContext *c = s->priv_data;
    int updated_streams = (int) s->nb_streams;
    for (i = start; i < FFMIN(end, c->n_videos); i++) {
        ret = read_header_expose_stream(s, c->videos[i]);
        if (ret) {
            return ret;
        }
    }
    for (i = start; i < FFMIN(end, c->n_audios); i++) {
        (void) read_header_expose_stream(s, c->audios[i]);
    }
    for (i = start; i < FFMIN(end, c->n_subtitles); i++) {
        (void) read_header_expose_stream(s, c->subtitles[i]);
    }
    /* first playlist, stream handled by avformat_open_input */
    if (0 == updated_streams) {
        return 0;
    }

    read_header_update_stream_avctx(s, updated_streams);
    for (i = updated_streams; i < (int) s->nb_streams; i++) {
        AVStream *st = s->streams[i];
        st->discard = AVDISCARD_ALL;
        st->internal->orig_codec_id = st->codecpar->codec_id;
    }

    return 0;
}

static int read_header_handle_common(
    AVFormatContext *s, struct representation *rep)
{
    DASHContext *c = s->priv_data;

    int ret = open_demux_for_component(s, rep);
    if (ret) {
        return ret;
    }

    if (rep->ctx) {
        ff_mutex_init(&rep->mutex, NULL);
    }
     rep->stream_index = c->nb_valid_rep;
     c->nb_valid_rep++;
     c->nb_streams += rep->n_assoc_stream;
    return 0;
}

static int read_header_handle_video(AVFormatContext *s,
    struct representation **videos, int start, int end)
{
    int i, ret = 0;
    DASHContext *c = s->priv_data;
    struct representation *rep = NULL;
    /* Open the demuxer for video components if available */
    for (i = start; i < end; i++) {
        rep = videos[i];
        if (i > 0 && c->is_init_section_common_video) {
            ret = copy_init_section(rep, videos[0]);
            if (ret < 0) {
                return ret;
            }
        }
        ret = read_header_handle_common(s, rep);
        if (ret) {
            return ret;
        }
    }
    return 0;
}

static int read_header_handle_audio(AVFormatContext *s,
    struct representation **audios, int start, int end)
{
    int i, ret = 0;
    DASHContext *c = s->priv_data;
    struct representation *rep = NULL;
    /* Open the demuxer for audio components if available */
    for (i = start; i < end; i++) {
        rep = audios[i];
        if (i > 0 && c->is_init_section_common_audio) {
            ret = copy_init_section(rep, audios[0]);
            if (ret < 0) {
                return ret;
            }
        }
        ret = read_header_handle_common(s, rep);
        if (ret) {
            return ret;
        }
    }
    return 0;
}

static int read_header_handle_subtitle(AVFormatContext *s,
    struct representation **subtitles, int start, int end)
{
    int i, ret = 0;
    DASHContext *c = s->priv_data;
    struct representation *rep = NULL;
    /* Open the demuxer for subtitle components if available */
    for (i = start; i < end; i++) {
        rep = subtitles[i];
        if (i > 0 && c->is_init_section_common_audio) {
            ret = copy_init_section(rep, subtitles[0]);
            if (ret < 0) {
                return ret;
            }
        }
        ret = read_header_handle_common(s, rep);
        if (ret) {
            return ret;
        }
    }
    return 0;
}

static int read_header_thread_l(AVFormatContext *s)
{
    int ret = 0;
    DASHContext *c = s->priv_data;

    MANIFEST_LOCK(c);
    int n_videos = c->n_videos;
    struct representation **videos = c->videos;
    int n_audios = c->n_audios;
    struct representation **audios = c->audios;
    int n_subtitles = c->n_subtitles;
    struct representation **subtitles = c->subtitles;
    MANIFEST_UNLOCK(c);

    if(n_videos > 1) {
        /* Open the demuxer for video and audio components if available */
        ret = read_header_handle_video(s, videos, 1, n_videos);
        if (ret) {
            goto fail;
        }
    }
    if(n_audios > 1) {
        (void) read_header_handle_audio(s, audios, 1, n_audios);
    }

    if (n_subtitles > 1) {
        (void) read_header_handle_subtitle(s, subtitles, 1, n_subtitles);
    }
    c->async.stage = ADAPTIVE_STAGE_EXPOSE_STREAM;
fail:
    if (ret) {
        c->async.stage = ADAPTIVE_STAGE_INIT_FAILED;
    }

    return ret;
}

static void *read_header_thread(void *arg)
{
    AVFormatContext *s = arg;

    ff_thread_setname("Dash Read Header");
    av_log(s, AV_LOG_INFO, "Dash playlist read header start\n");
    int ret = read_header_thread_l(s);
    /* error send async done ahead */
    if (ret) {
        ff_adaptive_async_done(s);
    }
    av_log(s, AV_LOG_INFO, "Dash playlist read header completely, ret:%d\n", ret);
    return NULL;
}

static void *open_input_next_thread(void *arg)
{
    struct representation *rep = arg;
    if (!rep) {
        return NULL;
    }

    ff_thread_setname("Dash Open Next");
    AVFormatContext *s    = rep->parent;
    DASHContext     *c    = s->priv_data;
    struct fragment *seg = rep->next_seg;

    av_log(rep->parent, AV_LOG_DEBUG, "[%p] open next async start\n", rep);
    int ret = 0;
    int retry_cnt = 0;
    AdaptiveInputContext *aic = &rep->adaptive_input_ctx;
    ret = open_input(c, rep, seg, seg->base_url, 1);
    if (ret < 0) { /* No input now, continue read current */
        av_log(rep->parent, AV_LOG_ERROR, "[%p] open next %s fail\n", rep, seg->url);
        ff_adaptive_reinit_input_context(s, aic);
        ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_NONE);
    } else {
        av_log(rep->parent, AV_LOG_DEBUG, "[%p] open next async ok\n", rep);
        ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_READY);
    }
    return NULL;
}

static int join_open_input_task(
    AVFormatContext *s,
    int n_reps, struct representation **reps)
{
    AdaptiveInputContext *aic = NULL;
    unsigned char cmpbuf[sizeof(pthread_t)] = {0};
    for (int i = 0; i < n_reps; i++) {
        struct representation *rep = reps[i];
        aic = &rep->adaptive_input_ctx;
        if ((memcmp(&aic->input_next_thread, cmpbuf, sizeof(pthread_t)))) {
            pthread_join(aic->input_next_thread, NULL);
            memset(&aic->input_next_thread, 0, sizeof(pthread_t));
            break;
        }
    }
    return 0;
}

static int need_delay_open_with_fragment_duration(
    DASHContext *c, AdaptiveInputContext *aic, struct representation *rep)
{
    if (rep->presentation_timeoffset) {
        return 0;
    } else if (c->publish_time > 0 && !c->availability_start_time) {
        return 0;
    } else {
        int64_t max_seq_no = calc_max_seg_no(rep, c);
        if (aic->next_fragment_seq_no >= max_seq_no) {
            return 1;
        }
    }
    return 0;
}

static int need_delay_open_with_timelines(DASHContext *c,
    AdaptiveInputContext *aic, struct representation *rep)
{
    int64_t max_available_time = calc_max_available_time(rep, c);
    int64_t next_seg_time = get_segment_start_time_based_on_timeline(c, rep, aic->next_fragment_seq_no);

    av_log(c, AV_LOG_VERBOSE,
        "[%p] max available time:%lld next seg time:%lld diff:%lld no:%lld\n",
        rep, max_available_time, next_seg_time, next_seg_time - max_available_time, aic->next_fragment_seq_no);
    /* avoid timestamp error case, too large difference */
    if (max_available_time > 0 &&
        next_seg_time  > max_available_time &&
        (next_seg_time < max_available_time + 60 * rep->fragment_timescale)) {
        return 1;
    }
    return 0;
}

static int delay_open_input(DASHContext *c,
    AdaptiveInputContext *aic, struct representation *rep)
{
    if (!(c->is_live)) {
        return 0;
    }

    int ret = 0;
    if (rep->n_fragments) {
        ret = 0;
    } else if (rep->n_timelines) {
        ret = need_delay_open_with_timelines(c, aic, rep);
    } else if (rep->fragment_duration) {
        ret = need_delay_open_with_fragment_duration(c, aic, rep);
    }
    if (ret) {
        MANIFEST_UNLOCK(c);
        av_usleep(1000);
        MANIFEST_LOCK(c);
    }
    return ret;
}

static int create_open_input_task(
    AVFormatContext *s,
    int n_reps, struct representation **reps)
{
     AdaptiveInputContext *aic = NULL;
     DASHContext *c = s->priv_data;
     for (int i = 0; i < n_reps; i++) {
         struct representation *rep = reps[i];
         aic = &rep->adaptive_input_ctx;
         if (ADAPTIVE_INPUT_NEXT_ASYNC !=
             ff_adaptive_get_next_input_status(aic)) {
             continue;
        }

        join_open_input_task(s, 1, &reps[i]);
        if (delay_open_input(c, aic, rep)) {
            return 0;
        }

        rep->next_seg = get_next_fragment(rep);
        if (!rep->next_seg) {
            return 0;
        }

        ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_ASYNCING);
        int ret = pthread_create(&aic->input_next_thread, NULL, open_input_next_thread, rep);
        if (ret) {
            free_fragment(&rep->next_seg);
        }
        break;
     }
    return 0;
}

static void move_check_timeline_seg_no(
    AVFormatContext *s, DASHContext *cdst, DASHContext *csrc,
    struct representation *dst_rep, struct representation *src_rep)
{
    refresh_update_timeline_cur_no(cdst, dst_rep, src_rep);
    int64_t min_seq_no = src_rep->first_seq_no;
    int64_t max_seq_no = src_rep->last_seq_no;

    if (src_rep->cur_seq_no <= min_seq_no ||
        (cdst->is_live && dst_rep->cur_seq_no < (max_seq_no - NUM_LOOKBACK_FRAGMENTS))) {
        src_rep->cur_seq_no = calc_cur_seg_no(s, src_rep);
    } else if (src_rep->cur_seq_no > max_seq_no) {
        av_log(s, AV_LOG_WARNING,
            "Check timeline fragment: min[%"PRId64"] max[%"PRId64"], playlist %d\n",
            min_seq_no, max_seq_no, (int) src_rep->rep_idx);
    }
}

static void move_dash_ctx(AVFormatContext *s, DASHContext *cdst, DASHContext *csrc)
{
    int i;
    int n_videos = cdst->n_videos;
    struct representation **videos = cdst->videos;
    int n_audios = cdst->n_audios;
    struct representation **audios = cdst->audios;
    int n_subtitles = cdst->n_subtitles;
    struct representation **subtitles = cdst->subtitles;

    for (i = 0; i < n_videos; i++) {
        struct representation *dst_video = videos[i];
        struct representation *src_video = csrc->videos[i];
        if (dst_video->timelines) {
            move_check_timeline_seg_no(s, cdst, csrc, dst_video, src_video);
            if (src_video->cur_seq_no >= 0) {
                move_dash_ctx_swap_timelines(src_video, dst_video, csrc);
            }
        }
        if (dst_video->fragments) {
            move_dash_ctx_reset_segments(src_video, dst_video, csrc);
        }
    }
    for (i = 0; i < n_audios; i++) {
        struct representation *dst_audio = audios[i];
        struct representation *src_audio = csrc->audios[i];
        if (dst_audio->timelines) {
            move_check_timeline_seg_no(s, cdst, csrc, dst_audio, src_audio);
            if (src_audio->cur_seq_no >= 0) {
                move_dash_ctx_swap_timelines(src_audio, dst_audio, csrc);
            }
        }
        if (dst_audio->fragments) {
            move_dash_ctx_reset_segments(src_audio, dst_audio, csrc);
        }
    }

    cdst->n_subtitles = n_subtitles;
    cdst->subtitles = subtitles;
    cdst->n_audios = n_audios;
    cdst->audios = audios;
    cdst->n_videos = n_videos;
    cdst->videos = videos;
}

static int handle_start_streams(AVFormatContext *s, DASHContext *c)
{
    int init_num = 0;
    if(c->n_audios) {
        c->is_init_section_common_audio =
            is_common_init_section_exist(c->audios, c->n_audios);
        init_num = c->use_thread ? 1 : c->n_audios;
        (void) read_header_handle_audio(s, c->audios, 0, init_num);
    }

    c->media_start_done |= (1 << AVMEDIA_TYPE_AUDIO);
    if (c->n_subtitles) {
        c->is_init_section_common_audio =
            is_common_init_section_exist(c->subtitles, c->n_subtitles);
        init_num = c->use_thread ? 1 : c->n_subtitles;
        (void) read_header_handle_subtitle(s, c->subtitles, 0, init_num);
    }
    c->media_start_done |= (1 << AVMEDIA_TYPE_SUBTITLE);
    return 0;
}

static int need_update_manifest(DASHContext *c)
{
    if (!(c->is_live)) {
        return 0;
    }

    int i = 0;
    struct representation *pls = NULL;
    /* Need close all read thread */
    for (i = 0; i < c->n_videos; i++) {
        pls = c->videos[i];
        if (pls->timelines || pls->fragments) {
            return 1;
        }
    }
    for (i = 0; i < c->n_audios; i++) {
        pls = c->audios[i];
        if (pls->timelines || pls->fragments) {
            return 1;
        }
    }

    return 0;
}

static void *dispatcher_loop_thread(void *arg)
{
    int ret = 0;
    AVFormatContext *s = arg;
    DASHContext *c = s->priv_data;

    ff_thread_setname("Dash Dispatcher");
    int need_update = need_update_manifest(c);

    while (c->async.stage < ADAPTIVE_STAGE_INIT &&
        (!ff_check_interrupt(c->interrupt_callback)));

    handle_start_streams(s, c);

    DASHContext dashc = {0};
    int64_t start_time = av_gettime();
    if (need_update) {
        av_dict_copy(&dashc.avio_opts, c->avio_opts, 0);
        ret = parse_manifest(s, &dashc, s->url, NULL, 1);
    }
    int64_t update_interval_usec = get_update_interval_usec(c);
    av_log(s, AV_LOG_INFO, "Started updates task, interval:%lld\n", update_interval_usec);
    while (c->async.stage < ADAPTIVE_STAGE_INIT_FAILED &&
            (!ff_check_interrupt(c->interrupt_callback))) {
        if (c->async.stage <= ADAPTIVE_STAGE_INIT) {
            av_usleep(10 * 1000);
            continue;
        }

        int64_t curr_time = av_gettime();
        if (c->is_live && need_update &&
           (curr_time - start_time >= update_interval_usec)) {
            ret = refresh_manifest(s, &dashc, 1);
            if (ret) {
                av_log(s, AV_LOG_WARNING, "Refresh Manifest error:%s\n", av_err2str(ret));
                continue;
            }
            av_log(s, AV_LOG_DEBUG, "Start update manifest\n");
            MANIFEST_LOCK(c);
            move_dash_ctx(s, c, &dashc);
            MANIFEST_UNLOCK(c);
            start_time = curr_time;
            av_log(s, AV_LOG_DEBUG, "Start update manifest done, avaliable time:%lld\n", c->availability_start_time);
        }

        MANIFEST_LOCK(c);
        (void) create_open_input_task(s, c->n_videos, c->videos);
        (void) create_open_input_task(s, c->n_audios, c->audios);
        MANIFEST_UNLOCK(c);
    }
    if (need_update) {
        dash_close_free(&dashc);
    }
    join_open_input_task(s, c->n_videos, c->videos);
    join_open_input_task(s, c->n_audios, c->audios);
    av_log(s, AV_LOG_INFO, "Stop updates task request detected\n");
    return NULL;
}

static int enter_read_header_thread(AVFormatContext *s)
{
    int ret = 0;
    DASHContext *c = s->priv_data;
    if (ADAPTIVE_STAGE_ASYNC_READ_HEADER != c->async.stage) {
        return -1;
    }
    /* return 0 if success */
    ret = pthread_create(&c->async.init_thread, NULL, read_header_thread, s);

    return ret;
}

static void set_clock_compensation(AVFormatContext *s)
{
    DASHContext *c = s->priv_data;
    /* From 1970, 2023 at least 53 YEARS */
    const static uint64_t ONE_YEAR_SEC = (uint64_t) 53 * 365 * 24 * 3600;
    uint64_t now = get_current_time_in_sec(c);

    if (now > ONE_YEAR_SEC) {
        return;
    }

    av_log(s, AV_LOG_INFO, "System time:%lld\n", now);
    if (c->publish_time) {
        c->clock_compensation_sec = c->publish_time - (int64_t) now;
    } else if (c->availability_start_time) {
        c->clock_compensation_sec = c->availability_start_time - (int64_t) now;
    }
}

static int dash_read_header_parse_manifest(AVFormatContext *s)
{
    DASHContext *c = s->priv_data;
    int ret = 0;


    if ((ret = save_avio_options(s)) < 0)
        goto fail;

    if ((ret = parse_manifest(s, c, s->url, s->pb, 0)) < 0)
        goto fail;

    /* If this isn't a live stream, fill the total duration of the
     * stream. */
    if (!c->is_live) {
        s->duration = (int64_t) c->media_presentation_duration * AV_TIME_BASE;
    } else {
        av_dict_set(&c->avio_opts, "seekable", "0", 0);
        set_clock_compensation(s);
    }

fail:
    if (ret) {
        c->async.stage = ADAPTIVE_STAGE_INIT_FAILED;
    }
    return ret;
}

static int dash_read_header_l(AVFormatContext *s)
{
    DASHContext *c = s->priv_data;
    int ret = 0;

    int init_num = 0;
    if(c->n_videos) {
        c->is_init_section_common_video =
            is_common_init_section_exist(c->videos, c->n_videos);
        init_num = c->use_thread ? 1 : c->n_videos;
        /* Open the demuxer for video and audio components if available */
        ret = read_header_handle_video(s, c->videos, 0, init_num);
        if (ret) {
            goto fail;
        }
    }

    if (!c->use_thread) {
        handle_start_streams(s, c);
    }

    const unsigned int media_start_done =
        (1 << AVMEDIA_TYPE_AUDIO) | (1 << AVMEDIA_TYPE_SUBTITLE);

    while (c->media_start_done != media_start_done &&
        (!ff_check_interrupt(c->interrupt_callback)));
    if (!c->nb_valid_rep || !c->nb_streams) {
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

    /* Create a program */
    if (!ret) {
        AVProgram *program;
        program = av_new_program(s, 0);
        if (!program) {
            goto fail;
        }
        init_num = c->use_thread ? 1 : c->nb_valid_rep;
        ret = read_header_expose_streams(s, 0, init_num);
        if (ret) {
            goto fail;
        }
    }

    ret = 0;
    if (c->use_thread) {
        c->async.stage = ADAPTIVE_STAGE_ASYNC_READ_HEADER;
        ff_adaptive_async_open(s);
        if (0 != enter_read_header_thread(s)) {
            ff_adaptive_async_done(s);
        }
    } else {
        c->async.stage = ADAPTIVE_STAGE_ALL_STREAM_READY;
    }
fail:
    if (ret) {
        c->async.stage = ADAPTIVE_STAGE_INIT_FAILED;
    }
    return ret;
}

static void recheck_discard_flags(AVFormatContext *s, struct representation **p, int n)
{
    int i, j;
    DASHContext *c = s->priv_data;

    for (i = 0; i < n; i++) {
        struct representation *pls = p[i];
        int needed = !pls->assoc_stream;
        /* No stream created when read header, no need check */
        if (!pls->n_assoc_stream) {
            continue;
        }
        for (int j = 0; j < pls->n_assoc_stream; j++) {
            needed = needed || !(pls->assoc_stream[j]) ||
                pls->assoc_stream[j]->discard < AVDISCARD_ALL;
        }

        AdaptiveInputContext *aic = &pls->adaptive_input_ctx;
        if (needed && !pls->ctx) {
            pls->cur_seg_offset = 0;
            pls->init_sec_buf_read_offset = 0;
            /* Catch up */
            for (j = 0; j < n; j++) {
                pls->cur_seq_no = FFMAX(pls->cur_seq_no, p[j]->cur_seq_no);
            }
            ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_NONE);
            int ret = reopen_demux_for_component(s, pls);
            if (ret >= 0 && pls->ctx && 1 == pls->n_fragments && 0 != c->playlist_sync_timestamp) {
                /* sync timestamp when playlist change */
				if (pls->ctx->iformat && strstr(pls->ctx->iformat->name, "mov")) {
					MOVContext *mov = pls->ctx->priv_data;
					mov->stream_seek_type = 1;
				}
                dash_seek(s, pls, c->playlist_sync_timestamp / 1000, AVSEEK_FLAG_BACKWARD, 0);
                component_stream_inject_side_data(s, pls->ctx, NULL);
                ff_adaptive_playlist_advance_fragment(pls->parent, pls->adaptive, NULL, 1, NULL);
                av_log(s, AV_LOG_INFO, "Reopen:%s sync to:%lld\n", pls->cur_seg->url, c->playlist_sync_timestamp);
            }
            av_log(s, AV_LOG_INFO, "Now receiving stream_index %d\n", pls->stream_index);
        } else if (!needed && pls->ctx) {
            ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_NONE);
            close_demux_for_component(pls);
            ff_adaptive_reinit_input_context(pls->parent, &pls->adaptive_input_ctx);
            ff_adaptive_playlist_io_deinit(pls->parent, pls->adaptive, &pls->input);
            av_log(s, AV_LOG_INFO, "[%p] No longer receiving stream_index %d\n", pls, pls->stream_index);
        }
    }
}

static void read_packet_update_info(AVFormatContext *s,
    struct representation *rep, AVPacket *pkt)
{
    int assoc_stream_index = pkt->stream_index + rep->assoc_stream_index;
    AVStream *stream = rep->ctx->streams[pkt->stream_index];

    if (!stream) {
        return;
    }
    rep->cur_timestamp =
        av_rescale(pkt->pts, stream->time_base.num * 90000, stream->time_base.den);
    av_log(rep->ctx, AV_LOG_TRACE, "[%p] ID:%d->%d time:%lld\n", rep, pkt->stream_index, assoc_stream_index, rep->cur_timestamp);
    pkt->stream_index = assoc_stream_index;
}

static void read_packet_recheck_discard_flags(AVFormatContext *s)
{
    DASHContext *c = s->priv_data;
    if (ADAPTIVE_STAGE_ALL_STREAM_READY == c->async.stage) {
        /* send async done after stream got by player */
        if (c->need_send_async_done) {
            ff_adaptive_async_done(s);
            c->need_send_async_done = 0;
        }
    } else {
        if (ADAPTIVE_STAGE_EXPOSE_STREAM == c->async.stage) {
            int ret = read_header_expose_streams(s, 1, c->nb_valid_rep);
            if (ret) {
                c->async.stage = ADAPTIVE_STAGE_INIT_FAILED;
            } else {
                c->async.stage = ADAPTIVE_STAGE_ALL_STREAM_READY;
                c->need_send_async_done = 1;
            }
        }
        return;
    }
    recheck_discard_flags(s, c->videos, c->n_videos);
    recheck_discard_flags(s, c->audios, c->n_audios);
    recheck_discard_flags(s, c->subtitles, c->n_subtitles);
    if (0 != c->playlist_sync_timestamp) {
        c->playlist_sync_timestamp = 0;
    }
}

static int dash_read_packet_l(AVFormatContext *s, AVPacket *pkt)
{
    DASHContext *c = s->priv_data;
    int ret = 0, i;
    int64_t mints = 0;
    struct representation *cur = NULL;
    struct representation *rep = NULL;

    read_packet_recheck_discard_flags(s);
    for (i = 0; i < c->n_videos; i++) {
        rep = c->videos[i];
        if (!rep->ctx || !rep->ctx->iformat)
            continue;
        if (!cur || rep->cur_timestamp < mints) {
            cur = rep;
            mints = rep->cur_timestamp;
        }
        if (!read_header_finished(s)) {
            break;
        }
    }
    for (i = 0; i < c->n_audios; i++) {
        rep = c->audios[i];
        if (!rep->ctx || !rep->ctx->iformat)
            continue;
        if (!cur || rep->cur_timestamp < mints) {
            cur = rep;
            mints = rep->cur_timestamp;
        }
        if (!read_header_finished(s)) {
            break;
        }
    }

    for (i = 0; i < c->n_subtitles; i++) {
        rep = c->subtitles[i];
        if (!rep->ctx || !rep->ctx->iformat)
            continue;
        if (!cur || rep->cur_timestamp < mints) {
            cur = rep;
            mints = rep->cur_timestamp;
        }
        if (!read_header_finished(s)) {
            break;
        }
    }

    if (!cur) {
        return AVERROR_INVALIDDATA;
    }
    /* Should not seek when read data, or network op will use more time */
    if (1 == cur->n_fragments && cur->pb.seekable) {
        cur->pb.seekable = 0;
    }
    int neec_change = 0;
    while (!ff_check_interrupt(c->interrupt_callback) && !ret) {
        ret = av_read_frame(cur->ctx, pkt);
        av_log(s, AV_LOG_TRACE, "[%d] pts:%lld, size:%d ret:%d\n", pkt->stream_index, pkt->pts, pkt->size, ret);
        if (ADAPTIVE_STAGE_ALL_STREAM_READY == c->async.stage) {
            neec_change = ff_adaptive_playlist_advance_fragment(cur->parent,
                cur->adaptive, pkt, (int) (ret < 0 && cur->is_restart_needed), &c->playlist_sync_timestamp);
        }
        if (ret >= 0) {
            /* If we got a packet, return it */
            read_packet_update_info(s, cur, pkt);
            return 0;
        }
        if (neec_change) {
            return 0;
        }
        if (cur->is_restart_needed) {
            cur->cur_seg_offset = 0;
            cur->init_sec_buf_read_offset = 0;
            ff_adaptive_playlist_io_deinit(cur->parent, cur->adaptive, &cur->input);
            ret = reopen_demux_for_component(s, cur);
            cur->is_restart_needed = 0;
        }
    }
    return AVERROR_EOF;
}

static int dash_close_l(AVFormatContext *s)
{
    DASHContext *c = s->priv_data;
    c->async.stage = ADAPTIVE_STAGE_CLOSE;
	dash_close_free(c);
    ff_adaptive_list_free(s);
    return 0;
}

static int dash_seek(AVFormatContext *s, struct representation *pls, int64_t seek_pos_msec, int flags, int dry_run)
{
    int ret = 0;
    int i = 0;
    int j = 0;
    int64_t duration = 0;

    av_log(pls->parent, AV_LOG_VERBOSE, "DASH seek pos[%"PRId64"ms], playlist %d%s\n",
           seek_pos_msec, pls->rep_idx, dry_run ? " (dry)" : "");

    // single fragment mode
    if (pls->n_fragments == 1) {
        pls->cur_timestamp = 0;
        pls->cur_seg_offset = 0;
        if (dry_run)
            return 0;

        ff_read_frame_flush(pls->ctx);
        ff_adaptive_playlist_seek_flush(pls->adaptive);

        int seekable = pls->pb.seekable;
        /* n_fragments > 1 use dash fragment seek,
         * n_fragments == 0 is SegmentTemplate, should not seek.
         */
        pls->pb.seekable = 1;
        pls->ctx->iformat->flags |= AVFMT_NOGENSEARCH;
        ret = av_seek_frame(pls->ctx, -1, seek_pos_msec * 1000, flags);
        pls->pb.seekable = seekable;
        return ret;
    }

    ff_adaptive_playlist_io_deinit(pls->parent, pls->adaptive, &pls->input);
    // find the nearest fragment
    if (pls->n_timelines > 0 && pls->fragment_timescale > 0) {
        int64_t num = pls->first_seq_no;
        av_log(pls->parent, AV_LOG_VERBOSE, "dash_seek with SegmentTimeline start n_timelines[%d] "
               "last_seq_no[%"PRId64"], playlist %d.\n",
               (int)pls->n_timelines, (int64_t)pls->last_seq_no, (int)pls->rep_idx);
        for (i = 0; i < pls->n_timelines; i++) {
            if (pls->timelines[i]->starttime > 0) {
                duration = pls->timelines[i]->starttime;
            }
            duration += pls->timelines[i]->duration;
            if (seek_pos_msec < ((duration * 1000) /  pls->fragment_timescale)) {
                goto set_seq_num;
            }
            for (j = 0; j < pls->timelines[i]->repeat; j++) {
                duration += pls->timelines[i]->duration;
                num++;
                if (seek_pos_msec < ((duration * 1000) /  pls->fragment_timescale)) {
                    goto set_seq_num;
                }
            }
            num++;
        }

set_seq_num:
        pls->cur_seq_no = num > pls->last_seq_no ? pls->last_seq_no : num;
        av_log(pls->parent, AV_LOG_VERBOSE, "dash_seek with SegmentTimeline end cur_seq_no[%"PRId64"], playlist %d.\n",
               (int64_t)pls->cur_seq_no, (int)pls->rep_idx);
    } else if (pls->fragment_duration > 0) {
        pls->cur_seq_no = pls->first_seq_no + ((seek_pos_msec * pls->fragment_timescale) / pls->fragment_duration) / 1000;
    } else {
        av_log(pls->parent, AV_LOG_ERROR, "dash_seek missing timeline or fragment_duration\n");
        pls->cur_seq_no = pls->first_seq_no;
    }
    pls->cur_timestamp = 0;
    pls->cur_seg_offset = 0;
    pls->init_sec_buf_read_offset = 0;
    ret = dry_run ? 0 : reopen_demux_for_component(s, pls);

    return ret;
}

static int dash_read_seek_l(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
{
    int ret = 0, i;
    DASHContext *c = s->priv_data;
    int64_t seek_pos_msec = av_rescale_rnd(timestamp, 1000,
                                           s->streams[stream_index]->time_base.den,
                                           flags & AVSEEK_FLAG_BACKWARD ?
                                           AV_ROUND_DOWN : AV_ROUND_UP);
    if ((flags & AVSEEK_FLAG_BYTE) || c->is_live)
        return AVERROR(ENOSYS);
    if (c->use_thread && ADAPTIVE_STAGE_ALL_STREAM_READY != c->async.stage) {
        av_log(s, AV_LOG_ERROR, "Not support seek during initialization, try again later\n");
        return AVERROR(EAGAIN);
    }

    /* Seek in discarded streams with dry_run=1 to avoid reopening them */
    for (i = 0; i < c->n_videos; i++) {
        if (!ret)
            ret = dash_seek(s, c->videos[i], seek_pos_msec, flags, !c->videos[i]->ctx);
    }
    for (i = 0; i < c->n_audios; i++) {
        if (!ret)
            ret = dash_seek(s, c->audios[i], seek_pos_msec, flags, !c->audios[i]->ctx);
    }
    for (i = 0; i < c->n_subtitles; i++) {
        if (!ret)
            ret = dash_seek(s, c->subtitles[i], seek_pos_msec, flags, !c->subtitles[i]->ctx);
    }

    return ret;
}


static void read_header_update_seekable_flag(AVFormatContext *s, DASHContext *c)
{
    s->pb->seekable = c->is_live ? 0 : 1;
    av_log(s, AV_LOG_INFO, "Open demux update seekable flag:%d\n", s->pb->seekable);
}

static int dash_read_header(AVFormatContext *s)
{
    int ret = 0;
    DASHContext *c = s->priv_data;

    c->async.stage = ADAPTIVE_STAGE_NONE;
    c->interrupt_callback = &s->interrupt_callback;
    c->async.use_thread = c->use_thread;
    c->clock_compensation_sec  = -1;
    c->playlist_sync_timestamp = 0;
    c->need_send_async_done    = 0;
    c->nb_streams = c->nb_valid_rep = 0;
    c->use_thread = 0; /* No use thread first parse */
    if ((ret = dash_read_header_parse_manifest(s)) < 0) {
        return ret;
    }

    read_header_update_seekable_flag(s, c);
    c->use_thread = c->async.use_thread;
    if ((ret =
        ff_adaptive_async_init_ctx(
            s, &c->async, dispatcher_loop_thread))) {
        c->use_thread = 0;
    }

    c->async.stage = ADAPTIVE_STAGE_INIT;
    ret = dash_read_header_l(s);
    if (ret < 0) {
        ff_adaptive_async_deinit_ctx(&c->async);
    }
    return ret;
}

static void read_seek_reset_next_input(
    AVFormatContext *s, struct representation *rep)
{
    /* let async read return quickly */
    rep->read_start_time = 0;
    AdaptiveInputContext *aic = &rep->adaptive_input_ctx;
    ff_adaptive_reinit_input_context(s, aic);
    ff_adaptive_set_next_input_status(aic, ADAPTIVE_INPUT_NEXT_NONE);
}

static int dash_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
{
    int i, ret = 0;
    DASHContext *c = s->priv_data;
    MANIFEST_LOCK(c);
    /* Need close all read thread */
    for (i = 0; i < c->n_videos; i++) {
        read_seek_reset_next_input(s, c->videos[i]);
    }
    for (i = 0; i < c->n_audios; i++) {
        read_seek_reset_next_input(s, c->audios[i]);
    }
    for (i = 0; i < c->n_subtitles; i++) {
        read_seek_reset_next_input(s, c->subtitles[i]);
    }
    ret = dash_read_seek_l(s, stream_index, timestamp, flags);
    MANIFEST_UNLOCK(c);
    return ret;
}

static int dash_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret = 0;
    DASHContext *c = s->priv_data;
    MANIFEST_LOCK(c);
    ret = dash_read_packet_l(s, pkt);
    MANIFEST_UNLOCK(c);
    return ret;
}

static int dash_close(AVFormatContext *s)
{
    int ret = 0;
    DASHContext *c = s->priv_data;
    ff_adaptive_async_deinit_ctx(&c->async);
    ret = dash_close_l(s);
    return ret;
}

static int dash_probe(const AVProbeData *p)
{
    if (!av_stristr(p->buf, "<MPD"))
        return 0;

#ifdef VMX_OTT_SVP
    /* FIXME, protection stream use ff_dash for history reason */
    if (av_stristr(p->buf, "<ContentProtection") &&
        (!av_stristr(p->buf, "audio/webm") &&
         !av_stristr(p->buf, "video/webm"))) {
        return 0;
    }
#endif
    if (av_stristr(p->buf, "dash:profile:isoff-on-demand:2011") ||
        av_stristr(p->buf, "dash:profile:isoff-live:2011") ||
        av_stristr(p->buf, "dash:profile:isoff-live:2012") ||
        av_stristr(p->buf, "dash:profile:isoff-main:2011") ||
        av_stristr(p->buf, "3GPP:PSS:profile:DASH1")) {
        return AVPROBE_SCORE_MAX;
    }
    if (av_stristr(p->buf, "dash:profile")) {
        return AVPROBE_SCORE_MAX;
    }

    return 0;
}

#define OFFSET(x) offsetof(DASHContext, x)
#define FLAGS AV_OPT_FLAG_DECODING_PARAM
static const AVOption dash_options[] = {
    {"allowed_extensions", "List of file extensions that dash is allowed to access",
        OFFSET(allowed_extensions), AV_OPT_TYPE_STRING,
        {.str = "aac,m4a,m4s,m4v,mov,mp4,webm"},
        INT_MIN, INT_MAX, FLAGS},
    { "qos_net_bitrate", "bitrate of network for qos",
        OFFSET(qos_net_bitrate), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, FLAGS},
    { "use_thread", "Use thread when need",
        OFFSET(use_thread), AV_OPT_TYPE_INT, { .i64 = 1 }, 0, INT_MAX, .flags = FLAGS},
    {NULL}
};

static const AVClass dash_class = {
    .class_name = "dash",
    .item_name  = av_default_item_name,
    .option     = dash_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_dash_demuxer = {
    .name           = "dash",
    .long_name      = NULL_IF_CONFIG_SMALL("Dynamic Adaptive Streaming over HTTP"),
    .priv_class     = &dash_class,
    .priv_data_size = sizeof(DASHContext),
    .read_probe     = dash_probe,
    .read_header    = dash_read_header,
    .read_packet    = dash_read_packet,
    .read_close     = dash_close,
    .read_seek      = dash_read_seek,
    .flags          = AVFMT_NO_BYTE_SEEK,
};
