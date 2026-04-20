/*
 * Apple HTTP Live Streaming demuxer
 * Copyright (c) 2010 Martin Storsjo
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

#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/time.h"
#include "avformat.h"
#include "internal.h"
#include "avio_internal.h"
#include "url.h"
#include <sys/time.h>
#include <ctype.h>

#define INITIAL_BUFFER_SIZE 32768

#define VIDEO_CHOOSED_BANDWIDTH 1000000
/*
 * An apple http stream consists of a playlist with media mt_segment files,
 * played sequentially. There may be several playlists with the same
 * video content, in different bandwidth variants, that are played in
 * parallel (preferably only one bandwidth mt_variant at a time). In this case,
 * the user supplied the url to a main playlist that only lists the mt_variant
 * playlists.
 *
 * If the main playlist doesn't point at any variants, we still create
 * one anonymous toplevel mt_variant for this, to maintain the structure.
 */

enum KeyType {
    KEY_NONE,
    KEY_AES_128,
    KEY_PLAYREADY,
};

struct mt_segment {
    int64_t duration;
    char url[MAX_URL_SIZE];
    char key[MAX_URL_SIZE];
    enum KeyType key_type;
    uint8_t iv[16];
};

extern int g_dash_playmode;
/*
 * Each mt_variant has its own demuxer. If it currently is active,
 * it has an open AVIOContext too, and potentially an AVPacket
 * containing the next packet from this stream.
 */
#define MAX_STREAMS 20
#define MAX_FIELD_LEN 64
struct mt_variant {
    enum AVMediaType codec_type;
    int bandwidth;
    char url[MAX_URL_SIZE];
    AVIOContext pb;
    uint8_t* read_buffer;
    URLContext *input;
    AVFormatContext *parent;
    int index;
    AVFormatContext *ctx;
    AVPacket pkt;
    int stream_offset;

    int finished;
    int64_t target_duration;
    int start_seq_no;
    int n_segments;
    struct mt_segment **segments;
    int needed, cur_needed;
    int selected_playing;//zhouxiang add for one audio, one video track, never changed in playing
    int cur_seq_no;
    int64_t last_load_time;

    char key_url[MAX_URL_SIZE];
    uint8_t key[16];

    int download_bps;
	int64_t lastin_ticket;
	int64_t last_sw_tick;
	int lastuporddown;
    int lastswbps;
	double ts_get_tick;
	int get_cnt;
	int64_t ts_size_cnt;
	char language[MAX_FIELD_LEN];
    char name[MAX_FIELD_LEN];

    int64_t seek_timestamp;
};

#define BPS_CNT 8
typedef struct MT_HLSContext {
    int n_variants;
    struct mt_variant **variants;
	int cur_audio_var;
    int cur_video_var;
    int cur_audio_seq_no;
    int end_of_segment;
    //int first_packet;
    int64_t first_timestamp;
    int64_t seek_timestamp;
    int seek_flags;
    AVIOInterruptCB *interrupt_callback;
    char *user_agent;                    ///< holds HTTP user agent set as an AVOption to the HTTP protocol context
    char *cookies;                       ///< holds HTTP cookie values set in either the initial response or as an AVOption to the HTTP protocol context

    int bps[BPS_CNT];
    //int cur_var;
	int next_video_var;//auto switch
    int bps_index;

    int change_state; //0 normal, 1 switch video, 2 switch audio
    int64_t last_audio_pkt_dts;//save last pkt dts, for audio track switch
} MT_HLSContext;

#define HLS_MONT_DEBUG printf

//#define READDATA_DEBUG
extern int file_seq_get_reset_state();
extern int file_seq_set_reset_state(int state);
#define BPS_THRESH (0.9)
//static int selected_url= 0 ;
static int recent_bps_is_enough(MT_HLSContext * c)
{
   int ret =1 ;
   int i =0;
   struct mt_variant * vv = c->variants[c->next_video_var];
   int next_bps =  vv->bandwidth;
   return 1;
   for(i=0;i<BPS_CNT;i++)
   {
  	    //HLS_MONT_DEBUG("%s %d, index[i] bps %d\n",__FUNCTION__,__LINE__,i, c->bps[i]);
  	    if(c->bps[i]!=0&&c->bps[i]*BPS_THRESH<next_bps)
  		{
      		ret = 0;
    		break;
  		}
   }
   return ret;
}
#ifndef MIN
#define MIN(a,b)(((a)>(b))?(b):(a))
#endif

static int find_recent_min_bps(MT_HLSContext * c)
{
   int ret =1 ;
   int i =0;

   struct mt_variant * vv = c->variants[c->next_video_var];
   int min_bps =  0x7fffffff;
   for(i=0;i< BPS_CNT;i++)
   {
      //HLS_MONT_DEBUG("%s %d, index[%d] bps %d\n",__FUNCTION__,__LINE__,i, c->bps[i]);
  	  if(c->bps[i]!=0)
  	  {
  		min_bps = MIN(c->bps[i],min_bps);
  	  }
  	}
   HLS_MONT_DEBUG("c->bps_index %d, min_bps %d\n", c->bps_index, min_bps);
   return min_bps;
}

/*//return ms
static int64_t ff_mtos_ticks_get(HLSContext * c)
{
    int64_t os_ticks;
    struct  timeval  tv;
    gettimeofday(&tv,NULL);
    os_ticks = (tv.tv_sec * (int64_t)1000 + tv.tv_usec/1000)/(int64_t)10;
    os_ticks = tv.tv_sec * 1000 + tv.tv_usec/1000;
    //HLS_DEBUG("\n%s %d tick  %lld \n",__FUNCTION__,__LINE__,os_ticks);

    return os_ticks;
}*/
//return ms
static int64_t ff_get_ms()
{
    int64_t os_ticks;
    struct  timeval  tv;
    gettimeofday(&tv,NULL);
    os_ticks = tv.tv_sec * 1000 + tv.tv_usec/1000 + 1;
    //DASH_DEBUG("\n%s %d tick  %lld \n",__FUNCTION__,__LINE__,os_ticks);

    return os_ticks;
}



//static int readlen_eachstream = 0;
//static int requirelen_eachstream = 0;
//#define REPEAT_TIMES 20
//int g_content_len = 0;
#define BPS_THRESH (0.9)
static int multi_video_change_to_next_hls(MT_HLSContext * c)//only for video, audio track switch by user
{
    struct mt_variant * vv = NULL;
    int now_bps = 0;
    int  i = 0;
    int last_bps = 0;
    int next_cur = 0;
    int upordown =0;
    c->next_video_var = -1;
    vv = c->variants[c->cur_video_var];
	c->bps[c->bps_index] = vv->download_bps;
    if(vv->download_bps*BPS_THRESH > vv->bandwidth)
    {
        now_bps = find_recent_min_bps(c);

    	if(now_bps > vv->download_bps )
    		now_bps = vv->download_bps;
    }
    else
    	now_bps = vv->download_bps;


    c->bps_index++;
    if(c->bps_index>= BPS_CNT)
        c->bps_index = 0;
    for (i = 0; i < c->n_variants; i++) {
        struct mt_variant * v = c->variants[i];
        //HLS_MONT_DEBUG("now_bps %d, i[%d] bandwidth %d, last_bps %d\n", (int)(now_bps * BPS_THRESH), i, v->bandwidth, last_bps);
        if (now_bps * BPS_THRESH > (v->bandwidth)
            && v->bandwidth > last_bps) {
            last_bps = v->bandwidth;
            c->next_video_var = i;

        }
    }

    if (-1 == c->next_video_var) {
        int min_bps = 0;

        for (i = 0; i < c->n_variants; i++) {
            struct mt_variant * v = c->variants[i];

            if (v->bandwidth < min_bps || min_bps == 0)\
            {
                min_bps  = v->bandwidth;
                c->next_video_var = i;
            }
        }
    }


    HLS_MONT_DEBUG("c->cur_video_var %d, c->next_video_var %d\n", c->cur_video_var, c->next_video_var);
    if (c->cur_video_var != c->next_video_var) {
		 struct mt_variant * v = c->variants[c->next_video_var];
		 //selected_url  = 1;
		 if(vv->bandwidth<v->bandwidth)
			upordown = 1;
	     else if(vv->bandwidth>v->bandwidth)
			upordown = -1;
         HLS_MONT_DEBUG("c->cur_video_var %d, c->next_video_var %d, upordown %d\n", c->cur_video_var, c->next_video_var, upordown);

		if(upordown!=1
			||(/*upordown==1&&*/recent_bps_is_enough(c)==1)     // for tsscan
			)
	 	{
            vv->last_sw_tick = ff_get_ms();
			vv->lastuporddown = upordown;
            vv->lastswbps = now_bps;
            vv = c->variants[c->next_video_var];

            //file_seq_set_reset_state(1);
            c->change_state = 1;

            HLS_MONT_DEBUG("video c->cur_video_var %d will changed to %d\n", c->cur_video_var, c->next_video_var);
        }

    }

    return 0;
}

static int read_chomp_line(AVIOContext *s, char *buf, int maxlen)
{
    int len = ff_get_line(s, buf, maxlen);
    while (len > 0 && av_isspace(buf[len - 1]))
        buf[--len] = '\0';
    return len;
}

static void free_segment_list(struct mt_variant *var)
{
    int i;
    for (i = 0; i < var->n_segments; i++)
        av_free(var->segments[i]);
    av_freep(&var->segments);
    var->n_segments = 0;
}

static void free_variant_list(MT_HLSContext *c)
{
    int i;
    for (i = 0; i < c->n_variants; i++) {
        struct mt_variant *var = c->variants[i];
        free_segment_list(var);
        av_free_packet(&var->pkt);
        av_free(var->pb.buffer);
        if (var->input)
            ffurl_close(var->input);
        if (var->ctx) {
            var->ctx->pb = NULL;
            avformat_close_input(&var->ctx);
        }
        av_free(var);
    }
    av_freep(&c->variants);
    av_freep(&c->cookies);
    av_freep(&c->user_agent);
    g_dash_playmode = 0;
    c->n_variants = 0;
}

/*
 * Used to reset a statically allocated AVPacket to a clean slate,
 * containing no data.
 */
static void reset_packet(AVPacket *pkt)
{
    av_init_packet(pkt);
    pkt->data = NULL;
}

typedef struct variant_info {
    char bandwidth[20];
    /* mt_variant group ids: */
    char audio[MAX_FIELD_LEN];
    char video[MAX_FIELD_LEN];
    char subtitles[MAX_FIELD_LEN];
    char resolution[MAX_FIELD_LEN];
}variant_info;

typedef struct rendition_info {
    char type[16];
    char uri[MAX_URL_SIZE];
    char group_id[MAX_FIELD_LEN];
    char language[MAX_FIELD_LEN];
    char assoc_language[MAX_FIELD_LEN];
    char name[MAX_FIELD_LEN];
    char defaultr[4];
    char forced[4];
    //char characteristics[MAX_CHARACTERISTICS_LEN];
}rendition_info;

/*******************************************************************************
*
*
*
*
*
*
*
*
*********************************************************************************/

static struct mt_variant *new_variant(MT_HLSContext *c, struct variant_info *info,
                                   const char *url, const char *base)
{
	int i = 0;
	int v_size = 0;
	struct mt_variant *var = av_mallocz(sizeof(struct mt_variant));
	if (!var)
		return NULL;
	v_size =(base?strlen(base):0)+(url?strlen(url):0)+2;
    memset(var->url, 0, MAX_URL_SIZE);

	reset_packet(&var->pkt);
	if (info)
	{
        var->bandwidth = atoi(info->bandwidth);
        //strcpy(var->audio_group, info->audio);
        //strcpy(var->video_group, info->video);
        //strcpy(var->subtitles_group, info->subtitles);
        HLS_MONT_DEBUG("info->audio %s, video %s, bandwidth %d\n",info->audio, info->video, var->bandwidth);

        if(strlen(info->audio) > 0 && strlen(info->video) == 0)
        {
            var->codec_type = AVMEDIA_TYPE_VIDEO;
        }
        else if(strlen(info->audio) == 0 && strlen(info->video) > 0)
        {
            var->codec_type = AVMEDIA_TYPE_AUDIO;
        }
        else if(strlen(info->audio) > 0 && strlen(info->video) > 0)
        {
            var->codec_type = AVMEDIA_TYPE_SUBTITLE;
        }
	}

	ff_make_absolute_url(var->url, v_size, base, url);

	//var->base_pkt_pts = 0;
	//var->base_pkt_dts = 0;
	//var->seek_time_pos = 0;
	/*for(i = 0; i < MAX_STREAMS; i++)
	{
		var->last_pkt_dts[i] = 0;
		var->last_pkt_dts[i] = 0;
	}*/


	dynarray_add(&c->variants, &c->n_variants, var);
	return var;
}

static struct mt_variant *new_variant2(MT_HLSContext *c, struct rendition_info *info,
                                    const char *base)
{
	int i = 0;
	int v_size = 0;
    enum AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
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
	struct mt_variant *var = av_mallocz(sizeof(struct mt_variant));
	if (!var)
		return NULL;
    var->codec_type = type;
    strcpy(var->language, info->language);
    strcpy(var->name, info->name);

	v_size =(base?strlen(base):0)+(info->uri?strlen(info->uri):0)+2;
	memset(var->url, 0, MAX_URL_SIZE);

	reset_packet(&var->pkt);

	ff_make_absolute_url(var->url, v_size, base, info->uri);

	//var->base_pkt_pts = 0;
	//var->base_pkt_dts = 0;
	//var->seek_time_pos = 0;
	/*for(i = 0; i < MAX_STREAMS; i++)
	{
		var->last_pkt_dts[i] = 0;
		var->last_pkt_dts[i] = 0;
	}*/


	dynarray_add(&c->variants, &c->n_variants, var);
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
    } else if (!strncmp(key, "RESOLUTION=", key_len)) {
        *dest     =        info->resolution;
        *dest_len = sizeof(info->resolution);
    }
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
    //} else if (!strncmp(key, "CHARACTERISTICS=", key_len)) {
    //    *dest     =        info->characteristics;
    //    *dest_len = sizeof(info->characteristics);
    }

    /*
     * ignored:
     * - AUTOSELECT: client may autoselect based on e.g. system language
     * - INSTREAM-ID: EIA-608 closed caption number ("CC1".."CC4")
     */
}

struct key_info {
     char uri[MAX_URL_SIZE];
     char method[10];
     char iv[35];
     char keyformat[48];
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

extern int g_hls_playmode;

static int multi_hls_parse_playlist(MT_HLSContext *c, const char *url,
                          struct mt_variant *var, AVIOContext *in)
{
    int ret = 0, is_segment = 0, is_variant = 0;
    int64_t duration = 0;
    enum KeyType key_type = KEY_NONE;
    uint8_t iv[16] = "";
    int has_iv = 0;
    struct variant_info   info1 = {{0}};
    struct rendition_info info2 = {{0}};
    char key[MAX_URL_SIZE] = "";
	char line[MAX_URL_SIZE]= "";
    const char *ptr;
    int close_in = 0;

    if (!in) {
        AVDictionary *opts = NULL;
        close_in = 1;
        /* Some HLS servers don't like being sent the range header */
        av_dict_set(&opts, "seekable", "0", 0);

        // broker prior HTTP options that should be consistent across requests
        av_dict_set(&opts, "user-agent", c->user_agent, 0);
        av_dict_set(&opts, "cookies", c->cookies, 0);

        HLS_MONT_DEBUG("%s ffurl_open url %s\n",__func__, url);
        ret = avio_open2(&in, url, AVIO_FLAG_READ,
                         c->interrupt_callback, &opts);
        av_dict_free(&opts);
        if (ret < 0)
            return ret;
    }

    read_chomp_line(in, line, sizeof(line));
    if (strcmp(line, "#EXTM3U")) {
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

    if (var) {
        free_segment_list(var);
        var->finished = 0;
        g_hls_playmode = 0;
    }
    while (!avio_feof(in)) {
		if(av_player_is_exit())
		{
			ret = AVERROR_EOF;

			goto fail;
		}
        read_chomp_line(in, line, sizeof(line));
        //HLS_MONT_DEBUG("line:%s\n",line);
        if (av_strstart(line, "#EXT-X-STREAM-INF:", &ptr)) {
            is_variant = 1;
            memset(&info1, 0, sizeof(variant_info));
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_variant_args,
                               &info1);

		}
		else if (av_strstart(line, "#EXT-X-MEDIA:", &ptr))
		{
            memset(&info2, 0, sizeof(rendition_info));
			ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_rendition_args,
                               &info2);
            if (!new_variant2(c, &info2, url)) {
					ret = AVERROR(ENOMEM);
					goto fail;
			}
			is_variant = 0;
		}
        else if (av_strstart(line, "#EXT-X-KEY:", &ptr)) {
            struct key_info info = {{0}};
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_key_args,
                               &info);
            key_type = KEY_NONE;
            has_iv = 0;
            if (!strcmp(info.method, "AES-128"))
                key_type = KEY_AES_128;
            if (!strncmp(info.iv, "0x", 2) || !strncmp(info.iv, "0X", 2)) {
                ff_hex_to_data(iv, info.iv + 2);
                has_iv = 1;
            }

            if(strstr(info.keyformat, "playready"))
            {
                key_type = KEY_PLAYREADY;
                char *p_start = strstr(info.uri, "base64,");
                if(p_start)
                {
                    g_dash_playmode = 10;//for hls fragment mp4
                    p_start += strlen("base64,");
                    if(var)                 // for tsscan
                    av_strlcpy(var->key_url, p_start, sizeof(key));
                }
            }
            else
                av_strlcpy(key, info.uri, sizeof(key));
        } else if (av_strstart(line, "#EXT-X-TARGETDURATION:", &ptr)) {
            if (!var) {
                var = new_variant(c, 0, url, NULL);
                if (!var) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
            }
            var->target_duration = atoi(ptr) * AV_TIME_BASE;
        } else if (av_strstart(line, "#EXT-X-MEDIA-SEQUENCE:", &ptr)) {
            if (!var) {
                var = new_variant(c, 0, url, NULL);
                if (!var) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
            }
            var->start_seq_no = atoi(ptr);
        } else if (av_strstart(line, "#EXT-X-ENDLIST", &ptr)) {
            if (var){
				g_hls_playmode = 1;//vod
				var->finished = 1;
			}
        } else if (av_strstart(line, "#EXT-X-MAP:", &ptr)) {
            struct mt_segment *seg;
            if (!var) {
                var = new_variant(c, 0, url, NULL);
                if (!var) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
            }
            seg = av_malloc(sizeof(struct mt_segment));
            if (!seg) {
                ret = AVERROR(ENOMEM);
                goto fail;
            }
            seg->duration = 0;
            seg->key_type = key_type;
            if (has_iv) {
                memcpy(seg->iv, iv, sizeof(iv));
            } else {
                int seq = var->start_seq_no + var->n_segments;
                memset(seg->iv, 0, sizeof(seg->iv));
                AV_WB32(seg->iv + 12, seq);
            }
            ff_make_absolute_url(seg->key, sizeof(seg->key), url, key);
            char tmp_line[MAX_URL_SIZE] = {0};
            char *p_start, *p_end;
            p_start = strstr(line, "URI=\"");
            if(p_start)
            {
                p_start += 5;
                p_end = strstr(p_start+1, "\"");
                if(p_end)
                {
                    memcpy(tmp_line, p_start, p_end - p_start);
                }
            }
            ff_make_absolute_url(seg->url, sizeof(seg->url), url, tmp_line);
            dynarray_add(&var->segments, &var->n_segments, seg);
            is_segment = 0;
        } else if (av_strstart(line, "#EXTINF:", &ptr)) {
            is_segment = 1;
            float tmp_duration = 0.0;
            sscanf(ptr, "%f", &tmp_duration);
            //duration   = atof(ptr) * AV_TIME_BASE;
            duration   = (int64_t)(tmp_duration * AV_TIME_BASE);
            //printf("duration %lld\n", duration);
        } else if (av_strstart(line, "#", NULL)) {
            continue;
        } else if (line[0]) {
            if (is_variant) {
				if (!new_variant(c, &info1, line, url)) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
                is_variant = 0;
            }
            if (is_segment) {
                struct mt_segment *seg;
                if (!var) {
                    var = new_variant(c, 0, url, NULL);
                    if (!var) {
                        ret = AVERROR(ENOMEM);
                        goto fail;
                    }
                }
				seg = av_mallocz(sizeof(struct mt_segment));
                if (!seg) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
                seg->duration = duration;
                seg->key_type = key_type;
                if (has_iv) {
                    memcpy(seg->iv, iv, sizeof(iv));
                } else {
                    int seq = var->start_seq_no + var->n_segments;
                    memset(seg->iv, 0, sizeof(seg->iv));
                    AV_WB32(seg->iv + 12, seq);
                }
                ff_make_absolute_url(seg->key, sizeof(seg->key), url, key);
                ff_make_absolute_url(seg->url, sizeof(seg->url), url, line);
                dynarray_add(&var->segments, &var->n_segments, seg);
                is_segment = 0;
            }
        }
    }
    if (var)
        var->last_load_time = av_gettime();

fail:
    if (close_in)
        avio_close(in);
    return ret;
}

static int open_input(MT_HLSContext *c, struct mt_variant *var)
{
    AVDictionary *opts = NULL;
    int ret;
    struct mt_segment *seg = var->segments[var->cur_seq_no - var->start_seq_no];

    // broker prior HTTP options that should be consistent across requests
    av_dict_set(&opts, "user-agent", c->user_agent, 0);
    av_dict_set(&opts, "cookies", c->cookies, 0);
    av_dict_set(&opts, "seekable", "0", 0);

    HLS_MONT_DEBUG("zx tmp seg->url:%s\n", seg->url);
    if (seg->key_type == KEY_NONE || seg->key_type == KEY_PLAYREADY) {
#if defined(CFG_SMART_HTTP_PTOTOCOL)
                #include "smart_http.h"
		ret = ffurl_open(&var->input, seg->url,
                            Smart_Http_Is_Enable() ? AVIO_FLAG_READ|URL_PROTOCOL_FLAG_NETWORK_SMART_HTTP|NOT_RECONNECT_FLAG : AVIO_FLAG_READ,
				&var->parent->interrupt_callback, &opts);
#else
        ret = ffurl_open(&var->input, seg->url, AVIO_FLAG_READ,
                          &var->parent->interrupt_callback, &opts);
#endif
        goto cleanup;
    } else if (seg->key_type == KEY_AES_128) {
        char iv[33], key[33], url[MAX_URL_SIZE];
        if (strcmp(seg->key, var->key_url)) {
            URLContext *uc;
            if (ffurl_open(&uc, seg->key, AVIO_FLAG_READ,
                           &var->parent->interrupt_callback, &opts) == 0) {
                if (ffurl_read_complete(uc, var->key, sizeof(var->key))
                    != sizeof(var->key)) {
                    av_log(NULL, AV_LOG_ERROR, "Unable to read key file %s\n",
                           seg->key);
                }
                ffurl_close(uc);
            } else {
                av_log(NULL, AV_LOG_ERROR, "Unable to open key file %s\n",
                       seg->key);
            }
            av_strlcpy(var->key_url, seg->key, sizeof(var->key_url));
        }
        ff_data_to_hex(iv, seg->iv, sizeof(seg->iv), 0);
        ff_data_to_hex(key, var->key, sizeof(var->key), 0);
        iv[32] = key[32] = '\0';
        if (strstr(seg->url, "://"))
            snprintf(url, sizeof(url), "crypto+%s", seg->url);
        else
            snprintf(url, sizeof(url), "crypto:%s", seg->url);
        if ((ret = ffurl_alloc(&var->input, url, AVIO_FLAG_READ,
                               &var->parent->interrupt_callback)) < 0)
            goto cleanup;
        av_opt_set(var->input->priv_data, "key", key, 0);
        av_opt_set(var->input->priv_data, "iv", iv, 0);
        /* Need to repopulate options */
        av_dict_free(&opts);
        av_dict_set(&opts, "seekable", "0", 0);
        if ((ret = ffurl_connect(var->input, &opts)) < 0) {
            ffurl_close(var->input);
            var->input = NULL;
            goto cleanup;
        }
        ret = 0;
    }
    else
      ret = AVERROR(ENOSYS);

cleanup:
    av_dict_free(&opts);
    return ret;
}

static int read_data(void *opaque, uint8_t *buf, int buf_size)
{
    struct mt_variant *v = opaque;
    MT_HLSContext *c = v->parent->priv_data;
    int ret, i;
    int64_t tick1,tick2;

restart:
    if (!v->input || file_seq_get_reset_state() == 2 ) {
        /* If this is a live stream and the reload interval has elapsed since
         * the last playlist reload, reload the mt_variant playlists now. */
        int64_t reload_interval = v->n_segments > 0 ?
                                  v->segments[v->n_segments - 1]->duration :
                                  v->target_duration;

reload:
        if (av_player_is_exit()) {
            HLS_MONT_DEBUG("\n[%s] ---detect stop commond at line %d! \n", __func__, __LINE__);
            return AVERROR_EOF;
        }

        if ((!v->finished &&
            av_gettime() - v->last_load_time >= reload_interval)
            || (c->change_state == 1 && v->codec_type == AVMEDIA_TYPE_VIDEO)
            || (c->change_state == 2 && v->codec_type == AVMEDIA_TYPE_AUDIO))//(file_seq_get_reset_state() == 2 )
            {
                const char * curv_url = v->url;
                if (c->change_state == 1 && v->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    c->cur_video_var = c->next_video_var;
    		        v->cur_seq_no --;
                    struct mt_variant * vv = NULL;
                    vv = c->variants[c->cur_video_var];
                    curv_url = vv->url;
                    vv->lastin_ticket = ff_get_ms();
                    v->pb.buf_end = v->pb.buf_ptr = v->pb.buffer;
                    file_seq_set_reset_state(0);
                    c->change_state = 0;
                    HLS_MONT_DEBUG("%s %d video real switch\n", __func__, __LINE__);
                }
                else if (c->change_state == 2 && v->codec_type == AVMEDIA_TYPE_AUDIO)
                {
                    struct mt_variant * vv = NULL;
                    vv = c->variants[c->cur_audio_var];
                    curv_url = vv->url;
                    v->pb.buf_end = v->pb.buf_ptr = v->pb.buffer;
                    file_seq_set_reset_state(0);
                    c->change_state = 0;
                    HLS_MONT_DEBUG("%s %d audio real switch\n", __func__, __LINE__);
                }

                if ((ret = multi_hls_parse_playlist(c, curv_url, v, NULL)) < 0)
                    return ret;
                /* If we need to reload the playlist again below (if
                 * there's still no more segments), switch to a reload
                 * interval of half the target duration. */
                reload_interval = v->target_duration / 2;
        }
        if (v->cur_seq_no < v->start_seq_no) {
            av_log(NULL, AV_LOG_WARNING,
                   "skipping %d segments ahead, expired from playlists\n",
                   v->start_seq_no - v->cur_seq_no);
            v->cur_seq_no = v->start_seq_no;
        }
        if (v->cur_seq_no >= v->start_seq_no + v->n_segments) {
            if (v->finished)
                return AVERROR_EOF;
            while (av_gettime() - v->last_load_time < reload_interval) {
                if (ff_check_interrupt(c->interrupt_callback))
                    return AVERROR_EXIT;
                av_usleep(100*1000);
            }
            /* Enough time has elapsed since the last reload */
            goto reload;
        }

        if(v->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            struct mt_variant * vv = NULL;
            vv = c->variants[c->cur_video_var];
            vv->ts_get_tick = 0; //ff_get_ms();
            vv->ts_size_cnt = 0;
    		//c->opened = 0;
    	    tick1 = ff_get_ms();
        }

        ret = open_input(c, v);//if switched, base url same, after parse m3u8, segments is the real variant datas.
        if (ret < 0)
            return ret;

        if(ret >= 0 && v->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            struct mt_variant * vv = NULL;
            vv = c->variants[c->cur_video_var];
            tick2 = ff_get_ms();
    	  	//c->opened = 1;
            vv->ts_get_tick += ((tick2 - tick1));
            //HLS_MONT_DEBUG("\n%s %d tick %f  ret %d\n",__FUNCTION__,__LINE__,(vv->tick2 - vv->tick1),ret);
        }
    }

    if(v->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        //struct mt_variant * vv = NULL;
        //vv = c->variants[c->cur_video_var];
        tick1 = ff_get_ms();
        //HLS_MONT_DEBUG("\n%s %d tick %f ret %d\n",__FUNCTION__,__LINE__,(tick1),ret);
    }

    if(file_seq_get_reset_state()!=2)
    {
        ret = ffurl_read(v->input, buf, buf_size);

        if(v->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            struct mt_variant * vv = NULL;
            vv = c->variants[c->cur_video_var];
    	    if(ret >0)
    	    {
                vv->ts_size_cnt += ret;
                tick2 = ff_get_ms();
                vv->ts_get_tick += ((tick2 - tick1));
    	        //HLS_MONT_DEBUG("%s %d: zxtmp tick %lld, %lld ret %d\n",__FUNCTION__,__LINE__, (tick2 - tick1), (vv->ts_get_tick), vv->ts_size_cnt);
    	     }
        }
    }
    else
    {
		ret = -1;
	}

    if (ret > 0)
        return ret;
    ffurl_close(v->input);
    v->input = NULL;

    if(file_seq_get_reset_state()==0 && v->codec_type == AVMEDIA_TYPE_VIDEO)
	{
        struct mt_variant * vv = NULL;
        float downloadbps = 0;

        vv = c->variants[c->cur_video_var];
		//c->opened = 0;
#define BPS_ALPHA 0.8
        downloadbps = 1000.f * vv->ts_size_cnt * 8.f / ((vv->ts_get_tick));
        //HLS_MONT_DEBUG(" ts_size_cnt %lld vv->ts_get_tick %d\n",  vv->ts_size_cnt, (int)(vv->ts_get_tick*100));

        if (vv->download_bps == 0) {
            vv->download_bps = downloadbps;
        } else {
            vv->download_bps = vv->download_bps * (1 - BPS_ALPHA) + downloadbps * BPS_ALPHA;
        }

        HLS_MONT_DEBUG("video vv->download_bps %d\n", vv->download_bps);
        if ((++(vv->get_cnt)) % 3 == 0) {
            multi_video_change_to_next_hls(c);
        }
    }

    v->cur_seq_no++;

    c->end_of_segment = 1;
    if(v->codec_type == AVMEDIA_TYPE_AUDIO)
        c->cur_audio_seq_no = v->cur_seq_no;

#if 0
    //remove by zhouxiang, dont open all streams
    if (v->ctx && v->ctx->nb_streams &&
        v->parent->nb_streams >= v->stream_offset + v->ctx->nb_streams) {
        v->needed = 0;
        for (i = v->stream_offset; i < v->stream_offset + v->ctx->nb_streams;
             i++) {
            if (v->parent->streams[i]->discard < AVDISCARD_ALL)
                v->needed = 1;
        }
    }
    if (!v->needed) {
        av_log(v->parent, AV_LOG_INFO, "No longer receiving mt_variant %d\n",
               v->index);
        return AVERROR_EOF;
    }
#endif
    goto restart;
}

static int hls_mont_read_header(AVFormatContext *s)
{
    URLContext *u = (s->flags & AVFMT_FLAG_CUSTOM_IO) ? NULL : s->pb->opaque;
    MT_HLSContext *c = s->priv_data;
    int ret = 0, i, j, stream_offset = 0;

    c->interrupt_callback = &s->interrupt_callback;

    // if the URL context is good, read important options we must broker later
    /*if (u && u->prot->priv_data_class) {
        // get the previous user agent & set back to null if string size is zero
        av_freep(&c->user_agent);
        av_opt_get(u->priv_data, "user-agent", 0, (uint8_t**)&(c->user_agent));
        if (c->user_agent && !strlen(c->user_agent))
            av_freep(&c->user_agent);

        // get the previous cookies & set back to null if string size is zero
        av_freep(&c->cookies);
        av_opt_get(u->priv_data, "cookies", 0, (uint8_t**)&(c->cookies));
        if (c->cookies && !strlen(c->cookies))
            av_freep(&c->cookies);
    }*/

    g_hls_playmode = 0;//live for default

    if ((ret = multi_hls_parse_playlist(c, s->filename, NULL, s->pb)) < 0)
        goto fail;

    if (c->n_variants == 0) {
        av_log(NULL, AV_LOG_WARNING, "Empty playlist\n");
        ret = AVERROR_EOF;
        goto fail;
    }
    /* If the playlist only contained variants, parse each individual
     * mt_variant playlist. */
    c->cur_audio_var = -1;
    c->cur_video_var = -1;
	if (c->n_variants > 1 || c->variants[0]->n_segments == 0) {
		//HLS_MONT_DEBUG("total n_variants %d, select_variants %d bandwidth %d\n",
		//    c->n_variants, select_variants, c->variants[select_variants]->bandwidth);

        int audio_selected = 0;
        int video_selected = 0;
        for (i = 0; i < c->n_variants; i++) {
            struct mt_variant *v = c->variants[i];
            //HLS_MONT_DEBUG("i %d, codec_type %d %s\n", i, v->codec_type, v->url);
            if(v->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                if(video_selected == 0)
                {
                    c->cur_video_var = i;
                    video_selected = 1;
                }
            }
            else if(v->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                if(audio_selected == 0)
                {
                    c->cur_audio_var = i;
                    audio_selected = 1;
                }
            }
        }

        if(c->cur_audio_var != -1)
        {
            //int tha_selected = -1;
            int eng_selected = -1;
            //select tha then eng
            for (i = 0; i < c->n_variants; i++)
            {
                struct mt_variant *v = c->variants[i];
                if(v->codec_type == AVMEDIA_TYPE_AUDIO)
                {
                    //if(tha_selected == -1 && tolower(v->language[0]) == 't' && tolower(v->language[1]) == 'h'
                        //&& tolower(v->language[2]) == 'a')
                    //{
                    //    tha_selected = i;
                    //}
                    if(eng_selected == -1 && tolower(v->language[0]) == 'e' && tolower(v->language[1]) == 'n')
                    {
                        eng_selected = i;
                    }
                }
            }

            //if(tha_selected != -1)
            //{
            //    c->cur_audio_var = tha_selected;
            //}
            if(eng_selected != -1)
            {
                c->cur_audio_var = eng_selected;
            }
        }

        if(c->cur_video_var != -1)
        {
            int v_720p_selected = -1;
            int max_diff = 0x7FFFFFF;
            //select tha then eng
            for (i = 0; i < c->n_variants; i++)
            {
                struct mt_variant *v = c->variants[i];
                if(v->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    if(abs(v->bandwidth - VIDEO_CHOOSED_BANDWIDTH) < max_diff)
                    {
                        v_720p_selected = i;
                        max_diff = abs(v->bandwidth - VIDEO_CHOOSED_BANDWIDTH);
                    }
                }
            }

            if(v_720p_selected != -1)
                c->cur_video_var = v_720p_selected;
        }

        av_log(NULL, AV_LOG_ERROR, "c->cur_audio_var %d %s, c->cur_video_var %d, bandwidth %d\n",
            c->cur_audio_var, c->variants[c->cur_audio_var]->language,
            c->cur_video_var, c->variants[c->cur_video_var]->bandwidth);
		for (i = 0; i < c->n_variants; i++)
        {
			if(i == c->cur_audio_var || i == c->cur_video_var)
			{
                struct mt_variant *v = c->variants[i];
    			HLS_MONT_DEBUG("i %d open inside %s\n",i, v->url);
    			if ((ret = multi_hls_parse_playlist(c, v->url, v, NULL)) < 0)
    				goto fail;
			}

		}
	}

    if (c->cur_audio_var == -1 || c->cur_video_var == -1)
    {
        av_log(NULL, AV_LOG_WARNING, "Choose start video and audio track failed!!!\n");
		ret = AVERROR_EOF;
		goto fail;
    }

	if (c->variants[c->cur_audio_var]->n_segments == 0
        && c->variants[c->cur_video_var]->n_segments == 0)
    {
		av_log(NULL, AV_LOG_WARNING, "Empty playlist\n");
		ret = AVERROR_EOF;
		goto fail;
	}

    /* If this isn't a live stream, calculate the total duration of the
     * stream. */
    if (c->variants[c->cur_video_var]->finished) {
        int64_t duration = 0;
        for (i = 0; i < c->variants[c->cur_video_var]->n_segments; i++)
            duration += c->variants[c->cur_video_var]->segments[i]->duration;
        s->duration = duration;
    }
    HLS_MONT_DEBUG("%s %d, s->duration %lld\n", __func__, __LINE__, s->duration);

    /* Open the demuxer for each mt_variant */
    for (i = 0; i < c->n_variants; i++) {
        struct mt_variant *v = c->variants[i];
        AVInputFormat *in_fmt = NULL;
        char bitrate_str[20];
        AVProgram *program;
		//only open 1 audio and 1 video
		v->index  = i;
		v->needed = 0;
        v->selected_playing = 0;//never changed in playing
		if(! (i == c->cur_audio_var || i == c->cur_video_var))
		{
			continue;
		}
        if (v->n_segments == 0)
            continue;

        if (!(v->ctx = avformat_alloc_context())) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        v->index  = i;
        v->needed = 1;
        v->selected_playing = 1;
        v->parent = s;

        /* If this is a live stream with more than 3 segments, start at the
         * third last mt_segment. */
        v->cur_seq_no = v->start_seq_no;
        if (!v->finished && v->n_segments > 3)
            v->cur_seq_no = v->start_seq_no + v->n_segments - 3;

        v->read_buffer = av_malloc(INITIAL_BUFFER_SIZE);
        ffio_init_context(&v->pb, v->read_buffer, INITIAL_BUFFER_SIZE, 0, v,
                          read_data, NULL, NULL);
        v->pb.seekable = 0;
        ret = av_probe_input_buffer(&v->pb, &in_fmt, v->segments[0]->url,
                                    NULL, 0, (1<<20));
        if (ret < 0) {
            /* Free the ctx - it isn't initialized properly at this point,
             * so avformat_close_input shouldn't be called. If
             * avformat_open_input fails below, it frees and zeros the
             * context, so it doesn't need any special treatment like this. */
            av_log(s, AV_LOG_ERROR, "Error when loading first mt_segment '%s'\n", v->segments[0]->url);
            avformat_free_context(v->ctx);
            v->ctx = NULL;
            goto fail;
        }
        if(strlen(v->key_url) > 5)
                v->ctx->p_content_protection = v->key_url;
            else
                v->ctx->p_content_protection = NULL;
        v->ctx->pb       = &v->pb;
        v->stream_offset = stream_offset;
        ret = avformat_open_input(&v->ctx, v->segments[0]->url, in_fmt, NULL);
        if (ret < 0)
            goto fail;

        v->ctx->ctx_flags &= ~AVFMTCTX_NOHEADER;
        ret = avformat_find_stream_info(v->ctx, NULL);
        if (ret < 0)
            goto fail;
        snprintf(bitrate_str, sizeof(bitrate_str), "%d", v->bandwidth);

        program = av_new_program(s, i);
        if (!program)
            goto fail;
        av_dict_set(&program->metadata, "variant_bitrate", bitrate_str, 0);

        /* Create new AVStreams for each stream in this mt_variant */
        for (j = 0; j < v->ctx->nb_streams; j++) {
            AVStream *ist = v->ctx->streams[j];
            //HLS_MONT_DEBUG("avformat_new_stream codec_type %d\n", ist->codecpar->codec_type);
            if(! (ist->codecpar->codec_type == AVMEDIA_TYPE_VIDEO
                || ist->codecpar->codec_type == AVMEDIA_TYPE_AUDIO))
                continue;//ffmpeg422 will first add AVMEDIA_TYPE_DATA

            AVStream *st = avformat_new_stream(s, NULL);
            if (!st) {
                ret = AVERROR(ENOMEM);
                goto fail;
            }
            av_program_add_stream_index(s, i, stream_offset);
            st->id = i;
            avcodec_parameters_copy(st->codecpar, ist->codecpar);
            avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
            avcodec_copy_context(st->codec, v->ctx->streams[j]->codec);
            if (v->bandwidth)
                av_dict_set(&st->metadata, "variant_bitrate", bitrate_str,
                                 0);
            if(strlen(v->language) > 0)
            {
                av_dict_set(&st->metadata, "language", v->language, 0);
            }
            stream_offset += 1;
        }
    }

    //add no selecked audio tracks, but empty tracks!!!
    struct mt_variant *selected_audio_v = c->variants[c->cur_audio_var];
    for (i = 0; i < c->n_variants; i++) {
        struct mt_variant *v = c->variants[i];
        AVInputFormat *in_fmt = NULL;
        AVProgram *program;
		//only open 1 audio and 1 video
		if(i == c->cur_audio_var || v->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			continue;
		}
        AVStream *st = avformat_new_stream(s, NULL);
        AVStream *ist = selected_audio_v->ctx->streams[0];
        if (!st) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        st->id = i;
        avcodec_parameters_copy(st->codecpar, ist->codecpar);
        avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
        avcodec_copy_context(st->codec, selected_audio_v->ctx->streams[0]->codec);
        if(strlen(v->language) > 0)
        {
            av_dict_set(&st->metadata, "language", v->language, 0);
        }
        v->seek_timestamp  = AV_NOPTS_VALUE;
        v->stream_offset = stream_offset;
        stream_offset += 1;
    }

    //nb_streams, all audio tracks + video tracks
    //HLS_MONT_DEBUG("s->nb_streams %d, c->cur_audio_var %d , c->cur_video_var %d\n",
    //    s->nb_streams, c->cur_audio_var, c->cur_video_var);
    //for (i = 0; i < c->n_variants; i++) {
    //    struct mt_variant *v = c->variants[i];
    //    HLS_MONT_DEBUG("var i %d stream_offset %d\n", i, v->stream_offset);
    //}

    HLS_MONT_DEBUG("%s %d zxtest c->cur_video_var %d, c->cur_audio_var %d\n", __func__, __LINE__,c->cur_video_var, c->cur_audio_var);

    c->change_state = 0;
    //c->first_packet = 0;//already choose, dont recheck_discard_flags
    c->first_timestamp = AV_NOPTS_VALUE;
    c->seek_timestamp  = AV_NOPTS_VALUE;
    return 0;
fail:
    free_variant_list(c);
    return ret;
}

static int recheck_discard_flags(AVFormatContext *s, int first)
{
    MT_HLSContext *c = s->priv_data;
    int i, changed = 0;

    /* Check if any new streams are needed */
    for (i = 0; i < c->n_variants; i++)
        c->variants[i]->cur_needed = 0;

    for (i = 0; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];
        struct mt_variant *var = c->variants[s->streams[i]->id];
        if (st->discard < AVDISCARD_ALL)
            var->cur_needed = 1;
    }
    int64_t last_audio_timestamp = 0;
    for (i = 0; i < c->n_variants; i++) {
        struct mt_variant *v = c->variants[i];
        if(v->codec_type != AVMEDIA_TYPE_AUDIO)
        {
            continue;
        }
        if (v->cur_needed && !v->needed) {
            v->needed = 1;
            changed = 1;
            v->cur_seq_no = c->cur_audio_seq_no;
            c->cur_audio_var = i;
            //av_log(s, AV_LOG_INFO, "Now receiving mt_variant %d\n", i);
            HLS_MONT_DEBUG("Now receiving mt_variant %d\n", i);
        } else if (!v->cur_needed && v->needed) {
            v->needed = 0;
            changed = 1;
            //av_log(s, AV_LOG_INFO, "No longer receiving mt_variant %d\n", i);
            HLS_MONT_DEBUG("No longer receiving mt_variant %d\n", i);
        }
    }

    if(changed)
    {
        for (i = 0; i < c->n_variants; i++) {
            struct mt_variant *v = c->variants[i];
            if(v->codec_type == AVMEDIA_TYPE_AUDIO && v->selected_playing)
            {
                if (v->input)
                    ffurl_close(v->input);
                v->input = NULL;
                //file_seq_set_reset_state(2);
                c->change_state = 2;
                v->pb.eof_reached = 0;
                /* Clear any buffered data */
                v->pb.buf_end = v->pb.buf_ptr = v->pb.buffer;
                /* Reset the pos, to let the mpegts demuxer know we've seeked. */
                v->pb.pos = 0;
                int j =0;
                AVStream * st = NULL;
                for (j = 0; j < v->ctx->nb_streams; j++) {
                    st = v->ctx->streams[v->pkt.stream_index];
                    if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                        break;
                }
                last_audio_timestamp = av_rescale_rnd(c->last_audio_pkt_dts, AV_TIME_BASE,
                                         st->time_base.den, AV_ROUND_DOWN);
            }
        }
        for (i = 0; i < c->n_variants; i++) {
            struct mt_variant *v = c->variants[i];
            if(i == c->cur_audio_var)
            {
                v->seek_timestamp = last_audio_timestamp;
                HLS_MONT_DEBUG("var %d seek_timestamp %lld\n", i, v->seek_timestamp);
            }
        }
    }
    return changed;
}

static int hls_mont_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    MT_HLSContext *c = s->priv_data;
    int ret, i, minvariant = -1;

    if(s->event_flags == 0x0100)//#define AVFMT_EVENT_FLAG_SWITCH_AUDIO     0x0100
    {
        HLS_MONT_DEBUG("s->event_flags 0x%x\n", s->event_flags);
        recheck_discard_flags(s, 0);
        s->event_flags = 0;
        //c->first_packet = 0;
    }


    //HLS_MONT_DEBUG("%s %d zxtest c->cur_video_var %d, c->cur_audio_var %d\n", __func__, __LINE__,c->cur_video_var, c->cur_audio_var);
start:
    c->end_of_segment = 0;
    for (i = 0; i < c->n_variants; i++) {
        struct mt_variant *var = c->variants[i];
        /* Make sure we've got one buffered packet from each open mt_variant
         * stream */
        if (var->selected_playing && !var->pkt.data) {
            while (1) {
                int64_t ts_diff;
                AVStream *st;
                ret = av_read_frame(var->ctx, &var->pkt);
                if (ret < 0) {
                    if (!avio_feof(&var->pb) && ret != AVERROR_EOF)
                        return ret;
                    reset_packet(&var->pkt);
                    break;
                } else {
                    if (c->first_timestamp == AV_NOPTS_VALUE &&
                        var->pkt.dts       != AV_NOPTS_VALUE)
                        c->first_timestamp = av_rescale_q(var->pkt.dts,
                            var->ctx->streams[var->pkt.stream_index]->time_base,
                            AV_TIME_BASE_Q);
                }

                //HLS_MONT_DEBUG("%s i %d, var->seek_timestamp %lld\n",__func__, i, var->seek_timestamp);
                if (var->seek_timestamp == AV_NOPTS_VALUE)
                    break;

                if (var->pkt.dts == AV_NOPTS_VALUE) {
                    c->seek_timestamp = AV_NOPTS_VALUE;
                    var->seek_timestamp = AV_NOPTS_VALUE;
                    break;
                }

                st = var->ctx->streams[var->pkt.stream_index];
                ts_diff = av_rescale_rnd(var->pkt.dts, AV_TIME_BASE,
                                         st->time_base.den, AV_ROUND_DOWN) -
                          var->seek_timestamp;
                HLS_MONT_DEBUG("%s i %d, var->seek_timestamp %lld, ts_diff %lld\n",__func__, i, var->seek_timestamp, ts_diff);
                if (ts_diff >= 0 && (c->seek_flags  & AVSEEK_FLAG_ANY ||
                                     var->pkt.flags & AV_PKT_FLAG_KEY)) {
                    HLS_MONT_DEBUG("%s i %d, var->seek_timestamp %lld, ptk dts %lld\n",__func__, i, var->seek_timestamp,
                                av_rescale_rnd(var->pkt.dts, AV_TIME_BASE,
                                         st->time_base.den, AV_ROUND_DOWN));
                    c->seek_timestamp = AV_NOPTS_VALUE;
                    var->seek_timestamp = AV_NOPTS_VALUE;
                    break;
                }
                HLS_MONT_DEBUG("%s av_free_packet i %d, dts %lld\n",__func__, i, var->pkt.dts);
                av_free_packet(&var->pkt);
                reset_packet(&var->pkt);
            }
        }
        /* Check if this stream still is on an earlier mt_segment number, or
         * has the packet with the lowest dts */
        if (var->pkt.data) {
            struct mt_variant *minvar = c->variants[minvariant];
            if (minvariant < 0 || var->cur_seq_no < minvar->cur_seq_no) {
                minvariant = i;
            } else if (var->cur_seq_no == minvar->cur_seq_no) {
                int64_t dts     =    var->pkt.dts;
                int64_t mindts  = minvar->pkt.dts;
                AVStream *st    =    var->ctx->streams[var->pkt.stream_index];
                AVStream *minst = minvar->ctx->streams[minvar->pkt.stream_index];

                if (dts == AV_NOPTS_VALUE) {
                    minvariant = i;
                } else if (mindts != AV_NOPTS_VALUE) {
                    if (st->start_time    != AV_NOPTS_VALUE)
                        dts    -= st->start_time;
                    if (minst->start_time != AV_NOPTS_VALUE)
                        mindts -= minst->start_time;

                    if (av_compare_ts(dts, st->time_base,
                                      mindts, minst->time_base) < 0)
                        minvariant = i;
                }
            }
        }
    }

    /* If we got a packet, return it */
    if (minvariant >= 0) {
        *pkt = c->variants[minvariant]->pkt;
        if(c->variants[minvariant]->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            pkt->stream_index += c->variants[c->cur_audio_var]->stream_offset;
            c->last_audio_pkt_dts = pkt->dts;
        }
        else
            pkt->stream_index += c->variants[minvariant]->stream_offset;
        //HLS_MONT_DEBUG("pkt->stream_index %d, c->variants[minvariant %d]->stream_offset %d, c->cur_audio_var %d %d\n",
        //    pkt->stream_index, minvariant, c->variants[minvariant]->stream_offset, c->cur_audio_var, c->variants[c->cur_audio_var]->stream_offset);
        reset_packet(&c->variants[minvariant]->pkt);
        return 0;
    }
    return AVERROR_EOF;
}

static int hls_mont_close(AVFormatContext *s)
{
    MT_HLSContext *c = s->priv_data;

    free_variant_list(c);
    return 0;
}

static int hls_mont_read_seek(AVFormatContext *s, int stream_index,
                               int64_t timestamp, int flags)
{
    MT_HLSContext *c = s->priv_data;
    int i, j, ret;

    //if ((flags & AVSEEK_FLAG_BYTE) || !c->variants[c->cur_video_var]->finished)
    if ((flags & AVSEEK_FLAG_BYTE) )
        return AVERROR(ENOSYS);

    c->seek_flags     = flags;
    c->seek_timestamp = stream_index < 0 ? timestamp :
                        av_rescale_rnd(timestamp, AV_TIME_BASE,
                                       s->streams[stream_index]->time_base.den,
                                       flags & AVSEEK_FLAG_BACKWARD ?
                                       AV_ROUND_DOWN : AV_ROUND_UP);
    HLS_MONT_DEBUG("%s c->seek_timestamp %lld, \n", __func__, c->seek_timestamp);
    timestamp = av_rescale_rnd(timestamp, AV_TIME_BASE, stream_index >= 0 ?
                               s->streams[stream_index]->time_base.den :
                               AV_TIME_BASE, flags & AVSEEK_FLAG_BACKWARD ?
                               AV_ROUND_DOWN : AV_ROUND_UP);
    if (s->duration < c->seek_timestamp) {
        c->seek_timestamp = AV_NOPTS_VALUE;
        for (i = 0; i < c->n_variants; i++) {
            struct mt_variant *var = c->variants[i];
            var->seek_timestamp = AV_NOPTS_VALUE;
        }
        return AVERROR(EIO);
    }

    HLS_MONT_DEBUG("%s c 0x%x ,%d\n", __func__, c, c->n_variants);
    for (i = 0; i < c->n_variants; i++) {
        /* Reset reading */
        ret = AVERROR(EIO);
        struct mt_variant *var = c->variants[i];
        int64_t pos = c->first_timestamp == AV_NOPTS_VALUE ?
                      0 : c->first_timestamp;
        if (var->input) {
            ffurl_close(var->input);
            var->input = NULL;
        }
        av_free_packet(&var->pkt);
        reset_packet(&var->pkt);
        var->pb.eof_reached = 0;
        /* Clear any buffered data */
        var->pb.buf_end = var->pb.buf_ptr = var->pb.buffer;
        /* Reset the pos, to let the mpegts demuxer know we've seeked. */
        var->pb.pos = 0;
        var->seek_timestamp = c->seek_timestamp;//each variant only one audio or video

        /* Locate the mt_segment that contains the target timestamp */
        for (j = 0; j < var->n_segments; j++) {
            if (timestamp >= pos &&
                timestamp < pos + var->segments[j]->duration) {
                var->cur_seq_no = var->start_seq_no + j;
                ret = 0;
                break;
            }
            pos += var->segments[j]->duration;
        }
        if (ret)
        {
            var->seek_timestamp = AV_NOPTS_VALUE;
        }
        HLS_MONT_DEBUG("%s i %d, ret %d, var->cur_seq_no %d, var->seek_timestamp %lld\n",
            __func__, i, ret, var->cur_seq_no, var->seek_timestamp);
    }
    return ret;
}

static int hls_mont_probe(const AVProbeData *p)
{
    /* Require #EXTM3U at the start, and either one of the ones below
     * somewhere for a proper match. */
    if (strncmp(p->buf, "#EXTM3U", 7))
        return 0;
    if (strstr(p->buf, "#EXT-X-STREAM-INF:")     ||
        strstr(p->buf, "#EXT-X-TARGETDURATION:") ||
        strstr(p->buf, "#EXT-X-MEDIA-SEQUENCE:") ||
        strstr(p->buf,"#EXTM3U") ) {

        if(strstr(p->buf, "#EXT-X-MEDIA:")){
            return 0;
        }
	}
    return 0;
}

AVInputFormat ff_hls_mont_demuxer = {
    .name           = "hls_mt",
    .long_name      = NULL_IF_CONFIG_SMALL("Apple HTTP Live Streaming"),
    .priv_data_size = sizeof(MT_HLSContext),
    .read_probe     = hls_mont_probe,
    .read_header    = hls_mont_read_header,
    .read_packet    = hls_mont_read_packet,
    .read_close     = hls_mont_close,
    .read_seek      = hls_mont_read_seek,
};
