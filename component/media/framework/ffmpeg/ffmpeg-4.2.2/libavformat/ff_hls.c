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

#define INITIAL_BUFFER_SIZE 32768

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
    KEY_PLAYREADY,
};

struct segment {
    int64_t duration;
    char *url;
    char *key;
    enum KeyType key_type;
    uint8_t iv[16];
};

extern int g_dash_playmode;
/*
 * Each variant has its own demuxer. If it currently is active,
 * it has an open AVIOContext too, and potentially an AVPacket
 * containing the next packet from this stream.
 */
struct variant {
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
    struct segment **segments;
    int needed, cur_needed;
    int cur_seq_no;
    int64_t last_load_time;

    char key_url[MAX_URL_SIZE];
    uint8_t key[16];

    int download_bps;
	int64_t lastin_ticket;
	int64_t last_sw_tick;
	int lastuporddown;
    int lastswbps;
	int64_t ts_get_tick;
	int get_cnt;
	int64_t ts_size_cnt;
};

#define BPS_CNT 8
typedef struct HLSContext {
    int n_variants;
    struct variant **variants;
    int cur_seq_no;
    int end_of_segment;
    int first_packet;
    int64_t first_timestamp;
    int64_t seek_timestamp;
    int seek_flags;
    AVIOInterruptCB *interrupt_callback;
    char *user_agent;                    ///< holds HTTP user agent set as an AVOption to the HTTP protocol context
    char *cookies;                       ///< holds HTTP cookie values set in either the initial response or as an AVOption to the HTTP protocol context

    int bps[BPS_CNT];
    int cur_var;
	int next_var;
    int bps_index;	
    char url[MAX_URL_SIZE];			/*the active url*/
} HLSContext;

struct variant_info {
    char bandwidth[20];
};

struct key_info {
     char uri[MAX_URL_SIZE];
     char method[10];
     char iv[35];
     char keyformat[48];
};

#define HLS_DEBUG printf

#ifndef MIN
#define MIN(a,b)(((a)>(b))?(b):(a))
#endif

//#define READDATA_DEBUG
extern int file_seq_get_reset_state();
extern int file_seq_set_reset_state(int state);

static int find_recent_min_bps(HLSContext * c)
{
	int ret =1 ;
	int i =0;

	struct variant * vv = c->variants[c->next_var];
	int min_bps =  0x7fffffff;
	for (i=0;i< BPS_CNT;i++) {
	//HLS_DEBUG("%s %d, index[%d] bps %d\n",__FUNCTION__,__LINE__,i, c->bps[i]);
		if (c->bps[i] != 0) {
			min_bps = MIN(c->bps[i],min_bps);
		}
	}
	//HLS_DEBUG("c->bps_index %d, min_bps %d bps\n", c->bps_index, min_bps);
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
#define BPS_THRESH (0.8)
static int change_to_next_hls(HLSContext * c,int to_low_bw)
{
	struct variant * vv = NULL;
	int now_bps = 0;
	int  i = 0;
	int last_bps = 0;
	int next_cur = 0;
	int upordown =0;
	int cur_bw = 0;
	
	c->next_var = -1;
	vv = c->variants[c->cur_var];
	
	c->bps[c->bps_index] = vv->download_bps;
	c->bps_index++;
	if(c->bps_index>= BPS_CNT) {
		c->bps_index = 0;
	}
	
	if (to_low_bw) {	 /*force change to lower bandwidth*/	
		cur_bw = vv->bandwidth;
		now_bps = cur_bw * BPS_THRESH;
	} else {
		if(vv->download_bps*BPS_THRESH > vv->bandwidth) {
			now_bps = find_recent_min_bps(c);
			if(now_bps > vv->download_bps )
				now_bps = vv->download_bps;
		} else {
			now_bps = vv->download_bps;
		}
	}

	//HLS_DEBUG("[%s:%d] now_bps:%d\n",__func__,__LINE__,now_bps);
   
    for (i = 0; i < c->n_variants; i++) {
        struct variant * v = c->variants[i];
        //HLS_DEBUG("now_bps %d, i[%d] bandwidth %d, last_bps %d\n", (int)(now_bps * BPS_THRESH), i, v->bandwidth, last_bps);
        if (now_bps * BPS_THRESH > (v->bandwidth)
            && v->bandwidth > last_bps) {
            last_bps = v->bandwidth;
            c->next_var = i;
			//HLS_DEBUG("next_var:%d,last_bps:%d bps\n",c->next_var,last_bps);

        }
    }

    if (-1 == c->next_var) {
        int min_bps = 0;
        for (i = 0; i < c->n_variants; i++) {
            struct variant * v = c->variants[i];
            if (v->bandwidth < min_bps || min_bps == 0) {
                min_bps  = v->bandwidth;
                c->next_var = i;				
            }
        }
    }

	HLS_DEBUG("c->cur_var %d, c->next_var %d\n", c->cur_var, c->next_var);
	if (c->cur_var != c->next_var) {
		struct variant * v = c->variants[c->next_var];
		//selected_url  = 1;
		if(vv->bandwidth<v->bandwidth)
		upordown = 1;
		else if(vv->bandwidth>v->bandwidth)
		upordown = -1;
		//HLS_DEBUG("c->cur_var %d, c->next_var %d, upordown %d\n", c->cur_var, c->next_var, upordown);
		{
			vv->last_sw_tick = ff_get_ms();
			vv->lastuporddown = upordown;
			vv->lastswbps = now_bps;
			vv = c->variants[c->cur_var];
            vv->get_cnt = 0;
			//file_seq_set_reset_state(1);
			file_seq_set_reset_state(2);
			HLS_DEBUG("c->cur_var %d will changed to %d\n", c->cur_var, c->next_var);
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

static void free_segment_list(struct variant *var)
{
    int i;
    for (i = 0; i < var->n_segments; i++)
    {
        if(var->segments[i]->key)
			av_freep(&(var->segments[i]->key));
		if(var->segments[i]->url)
			av_freep(&(var->segments[i]->url));
        av_free(var->segments[i]);
    }
    av_freep(&var->segments);
    var->n_segments = 0;
}

static void free_variant_list(HLSContext *c)
{
    int i;
    for (i = 0; i < c->n_variants; i++) {
        struct variant *var = c->variants[i];
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

static struct variant *new_variant(HLSContext *c, int bandwidth,
                                   const char *url, const char *base)
{
    struct variant *var = av_mallocz(sizeof(struct variant));
    if (!var)
        return NULL;
    reset_packet(&var->pkt);
    var->bandwidth = bandwidth;
    ff_make_absolute_url(var->url, sizeof(var->url), base, url);
    dynarray_add(&c->variants, &c->n_variants, var);
    return var;
}


static void handle_variant_args(struct variant_info *info, const char *key,
                                int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "BANDWIDTH=", key_len)) {
        *dest     =        info->bandwidth;
        *dest_len = sizeof(info->bandwidth);
    }
}

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

static int parse_playlist(HLSContext *c, const char *url,
                          struct variant *var, AVIOContext *in)
{
    int ret = 0, is_segment = 0, is_variant = 0, bandwidth = 0;
    int64_t duration = 0;
    enum KeyType key_type = KEY_NONE;
    uint8_t iv[16] = "";
    int has_iv = 0;
    char key[MAX_URL_SIZE] = "";
    char line[MAX_URL_SIZE];
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

        HLS_DEBUG("[%s:%d] ffurl_open url %s\n",__func__,__LINE__, url);
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
        read_chomp_line(in, line, sizeof(line));
        //HLS_DEBUG("line:%s\n",line);
        if (av_strstart(line, "#EXT-X-STREAM-INF:", &ptr)) {
            struct variant_info info = {{0}};
            is_variant = 1;
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_variant_args,
                               &info);
            bandwidth = atoi(info.bandwidth);
        } else if (av_strstart(line, "#EXT-X-KEY:", &ptr)) {
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
            struct segment *seg;
            if (!var) {
                var = new_variant(c, 0, url, NULL);
                if (!var) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
            }
            seg = av_malloc(sizeof(struct segment));
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
                if (!new_variant(c, bandwidth, line, url)) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
                is_variant = 0;
                bandwidth  = 0;
            }
            if (is_segment) {
                struct segment *seg;
                if (!var) {
                    var = new_variant(c, 0, url, NULL);
                    if (!var) {
                        ret = AVERROR(ENOMEM);
                        goto fail;
                    }
                }
                seg = av_mallocz(sizeof(struct segment));
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

                seg->url = av_mallocz(strlen(line)+1);
                memcpy(seg->url, line, strlen(line));
                //ff_make_absolute_url(seg->key, sizeof(seg->key), url, key);
                if(key_type!=KEY_NONE)
				{
					seg->key = av_mallocz(strlen(url)+strlen(key)+2);
					if(seg->key ==NULL)
					{
						av_free(seg->url);
						av_free(seg);
						ret = AVERROR(ENOMEM);
						goto fail;
					}
					ff_make_absolute_url(seg->key, (strlen(url)+strlen(key)+2), url, key);
				}
                //ff_make_absolute_url(seg->url, seg_url_len, url, line);

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

static int open_input(HLSContext *c, struct variant *var)
{
    AVDictionary *opts = NULL;
    int ret;
    struct segment *seg = var->segments[var->cur_seq_no - var->start_seq_no];

    // broker prior HTTP options that should be consistent across requests
    av_dict_set(&opts, "user-agent", c->user_agent, 0);
    av_dict_set(&opts, "cookies", c->cookies, 0);
    av_dict_set(&opts, "seekable", "0", 0);

    int seg_url_len = strlen(var->url)+strlen(seg->url) + 20;
    char tmp_url[4096] = {0};
    ff_make_absolute_url(tmp_url, seg_url_len, c->url, seg->url);
    if (seg->key_type == KEY_NONE || seg->key_type == KEY_PLAYREADY) {
#if defined(CFG_SMART_HTTP_PTOTOCOL)
                #include "smart_http.h"
		ret = ffurl_open(&var->input, tmp_url,
                            Smart_Http_Is_Enable() ? AVIO_FLAG_READ|URL_PROTOCOL_FLAG_NETWORK_SMART_HTTP|NOT_RECONNECT_FLAG : AVIO_FLAG_READ,
				&var->parent->interrupt_callback, &opts);
#else
        ret = ffurl_open(&var->input, tmp_url, AVIO_FLAG_READ,
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
        if (strstr(tmp_url, "://"))
            snprintf(url, sizeof(url), "crypto+%s", tmp_url);
        else
            snprintf(url, sizeof(url), "crypto:%s", tmp_url);
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
    struct variant *v = opaque;
    HLSContext *c = v->parent->priv_data;
    int ret, i;
    int64_t tick1,tick2;
	static uint32_t s_time = 0,e_time = 0;

restart:
    if (!v->input || file_seq_get_reset_state() == 2 ) {
        /* If this is a live stream and the reload interval has elapsed since
         * the last playlist reload, reload the variant playlists now. */
        int64_t reload_interval = v->n_segments > 0 ?
                                  v->segments[v->n_segments - 1]->duration :
                                  v->target_duration;

reload:
        if (av_player_is_exit()) {
            HLS_DEBUG("\n[%s] ---detect stop commond at line %d! \n", __func__, __LINE__);
            return AVERROR_EOF;
        }

        if ((!v->finished &&
            av_gettime() - v->last_load_time >= reload_interval)
            || file_seq_get_reset_state() == 2 ) {
            if (file_seq_get_reset_state() == 2) {
                c->cur_var = c->next_var;
                v->pb.buf_end = v->pb.buf_ptr = v->pb.buffer;
                file_seq_set_reset_state(0);
		        v->cur_seq_no --;
            }
           // const char * curv_url = v->url;
            struct variant * vv = NULL;
            vv = c->variants[c->cur_var];
			memset(c->url,0,sizeof(c->url));
			memcpy(c->url,vv->url,strlen(vv->url));
            if ((ret = parse_playlist(c, c->url, v, NULL)) < 0)
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

        {
            struct variant * vv = NULL;
            vv = c->variants[c->cur_var];
            vv->ts_get_tick = 0; //ff_get_ms();
            vv->ts_size_cnt = 0;
    		//c->opened = 0;
    	    tick1 = ff_get_ms();
        }

		s_time = time(NULL);
        ret = open_input(c, v);
        if (ret < 0)
            return ret;
        if(ret >= 0) {
            struct variant * vv = NULL;
            vv = c->variants[c->cur_var];
            tick2 = ff_get_ms();
    	  	//c->opened = 1;
            vv->ts_get_tick += ((tick2 - tick1));
            //HLS_DEBUG("\n%s %d tick %f  ret %d\n",__FUNCTION__,__LINE__,(vv->tick2 - vv->tick1),ret);
		
        }
    }

    tick1 = ff_get_ms();
    if (file_seq_get_reset_state() != 2) {		
		ret = ffurl_read(v->input, buf, buf_size);
		if (ret > 0) {	
			struct variant * vv = NULL;
			vv = c->variants[c->cur_var];
			vv->ts_size_cnt += ret;
			tick2 = ff_get_ms();
			vv->ts_get_tick += ((tick2 - tick1));
			//HLS_DEBUG("[%s %d]: tick %lld, total tick: %lld total size: %lld\n",__FUNCTION__,__LINE__, (tick2 - tick1), (vv->ts_get_tick), vv->ts_size_cnt);
		}	
    } else {
		ret = -1;
	}

    if (ret > 0)
        return ret;
	
    ffurl_close(v->input);
    v->input = NULL;	
	e_time = time(NULL);

    if(file_seq_get_reset_state()==0)
	{
        struct variant * vv = NULL;
        float downloadbps = 0;
		int64_t seg_dl_time = e_time - s_time;
		int64_t seg_dura = 0;
		int to_low_bw = 0;
	
        vv = c->variants[c->cur_var];	
		seg_dura = v->segments[v->cur_seq_no - v->start_seq_no]->duration;
		seg_dura = seg_dura*1.5;
		seg_dl_time = seg_dl_time*1000*1000;

		//c->opened = 0;
#define BPS_ALPHA 0.8
        downloadbps = 1000.f * vv->ts_size_cnt * 8.f/ ((vv->ts_get_tick));

		if (seg_dl_time > seg_dura && ((int)(vv->bandwidth*BPS_THRESH) > ((int)downloadbps/8))) {
			HLS_DEBUG("force switch to low bandwidth\n");
			to_low_bw = 1;
		}

        //HLS_DEBUG(" ts_size_cnt %lld vv->ts_get_tick %d\n",  vv->ts_size_cnt, (int)(vv->ts_get_tick*100));
        if (vv->download_bps == 0) {
            vv->download_bps = downloadbps;
        } else {
            vv->download_bps = vv->download_bps * (1 - BPS_ALPHA) + downloadbps * BPS_ALPHA;
        }
		if ((++(vv->get_cnt)) % 3 == 0 || to_low_bw == 1) {
         	 HLS_DEBUG("vv->download_bps %f bps, vv->get_cnt %d,bandwidth:%d\n", 
				vv->download_bps, vv->get_cnt,vv->bandwidth);
            change_to_next_hls(c,to_low_bw);
        }
    }

    v->cur_seq_no++;

    c->end_of_segment = 1;
    c->cur_seq_no = v->cur_seq_no;

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
        av_log(v->parent, AV_LOG_INFO, "No longer receiving variant %d\n",
               v->index);
        return AVERROR_EOF;
    }
    goto restart;
}

static int hls_read_header(AVFormatContext *s)
{
    URLContext *u = (s->flags & AVFMT_FLAG_CUSTOM_IO) ? NULL : s->pb->opaque;
    HLSContext *c = s->priv_data;
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

    if ((ret = parse_playlist(c, s->filename, NULL, s->pb)) < 0)
        goto fail;

    if (c->n_variants == 0) {
        av_log(NULL, AV_LOG_WARNING, "Empty playlist\n");
        ret = AVERROR_EOF;
        goto fail;
    }
    /* If the playlist only contained variants, parse each individual
     * variant playlist. */
     HLS_DEBUG("ffmpeg422 ff_hls c->n_variants %d, c->variants[0]->n_segments %d\n",
        c->n_variants, c->variants[0]->n_segments);
    if (c->n_variants > 1 || c->variants[0]->n_segments == 0) {
        //int select_variants = (c->n_variants >> 1) ;
        int select_variants = 0;
        for (i = 0; i < c->n_variants; i++) {
            //only parse the selected variant to save memory
             if(i != select_variants)
             {
                continue;
             }
            struct variant *v = c->variants[i];
            struct variant *v0 = c->variants[0];
			memset(c->url,0,sizeof(c->url));
			memcpy(c->url,v->url,strlen(v->url));
            if ((ret = parse_playlist(c, c->url, v0, NULL)) < 0)
                goto fail;
        }
        c->cur_var = select_variants;
    }
    else {
        	c->cur_var = 0;
			memset(c->url,0,sizeof(c->url));
			memcpy(c->url,c->variants[0]->url,strlen(c->variants[0]->url));
    }

    if (c->variants[0]->n_segments == 0) {
        av_log(NULL, AV_LOG_WARNING, "Empty playlist\n");
        ret = AVERROR_EOF;
        goto fail;
    }

    /* If this isn't a live stream, calculate the total duration of the
     * stream. */
    if (c->variants[0]->finished) {
        int64_t duration = 0;
        for (i = 0; i < c->variants[0]->n_segments; i++)
            duration += c->variants[0]->segments[i]->duration;
        s->duration = duration;
    }

    /* Open the demuxer for each variant */
    for (i = 0; i < c->n_variants; i++) {
        struct variant *v = c->variants[i];
        AVInputFormat *in_fmt = NULL;
        char bitrate_str[20];
        AVProgram *program;
        v->index  = i;
        v->needed = 0;
        if(i != c->cur_var)
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
        v->parent = s;

        /* If this is a live stream with more than 3 segments, start at the
         * third last segment. */
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
            av_log(s, AV_LOG_ERROR, "Error when loading first segment '%s'\n", v->segments[0]->url);
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

        /* Create new AVStreams for each stream in this variant */
        for (j = 0; j < v->ctx->nb_streams; j++) {
            AVStream *st = avformat_new_stream(s, NULL);
            AVStream *ist = v->ctx->streams[j];
            if (!st) {
                ret = AVERROR(ENOMEM);
                goto fail;
            }
            av_program_add_stream_index(s, i, stream_offset + j);
            st->id = i;
            avcodec_parameters_copy(st->codecpar, ist->codecpar);
            avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
            avcodec_copy_context(st->codec, v->ctx->streams[j]->codec);
            if (v->bandwidth)
                av_dict_set(&st->metadata, "variant_bitrate", bitrate_str,
                                 0);
        }
        stream_offset += v->ctx->nb_streams;
    }

    c->first_packet = 1;
    c->first_timestamp = AV_NOPTS_VALUE;
    c->seek_timestamp  = AV_NOPTS_VALUE;

    return 0;
fail:
    free_variant_list(c);
    return ret;
}

static int recheck_discard_flags(AVFormatContext *s, int first)
{
    HLSContext *c = s->priv_data;
    int i, changed = 0;

    /* Check if any new streams are needed */
    for (i = 0; i < c->n_variants; i++)
        c->variants[i]->cur_needed = 0;

    for (i = 0; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];
        struct variant *var = c->variants[s->streams[i]->id];
        if (st->discard < AVDISCARD_ALL)
            var->cur_needed = 1;
    }
    for (i = 0; i < c->n_variants; i++) {
        struct variant *v = c->variants[i];
        if (v->cur_needed && !v->needed) {
            v->needed = 1;
            changed = 1;
            v->cur_seq_no = c->cur_seq_no;
            v->pb.eof_reached = 0;
            av_log(s, AV_LOG_INFO, "Now receiving variant %d\n", i);
        } else if (first && !v->cur_needed && v->needed) {
            if (v->input)
                ffurl_close(v->input);
            v->input = NULL;
            v->needed = 0;
            changed = 1;
            av_log(s, AV_LOG_INFO, "No longer receiving variant %d\n", i);
        }
    }
    return changed;
}

static int hls_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    HLSContext *c = s->priv_data;
    int ret, i, minvariant = -1;

    if (c->first_packet) {
        recheck_discard_flags(s, 1);
        c->first_packet = 0;
    }

start:
    c->end_of_segment = 0;
    for (i = 0; i < c->n_variants; i++) {
        struct variant *var = c->variants[i];
        /* Make sure we've got one buffered packet from each open variant
         * stream */
        if (var->needed && !var->pkt.data) {
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

                //printf("%s %d\n",__func__, (c->seek_timestamp == AV_NOPTS_VALUE));
                if (c->seek_timestamp == AV_NOPTS_VALUE)
                    break;

                if (var->pkt.dts == AV_NOPTS_VALUE) {
                    c->seek_timestamp = AV_NOPTS_VALUE;
                    break;
                }

                st = var->ctx->streams[var->pkt.stream_index];
                ts_diff = av_rescale_rnd(var->pkt.dts, AV_TIME_BASE,
                                         st->time_base.den, AV_ROUND_DOWN) -
                          c->seek_timestamp;
                if (ts_diff >= 0 && (c->seek_flags  & AVSEEK_FLAG_ANY ||
                                     var->pkt.flags & AV_PKT_FLAG_KEY)) {
                    HLS_DEBUG("%s c->seek_timestamp %lld, ptk dts %lld\n",__func__, c->seek_timestamp,
                                av_rescale_rnd(var->pkt.dts, AV_TIME_BASE,
                                         st->time_base.den, AV_ROUND_DOWN));
                    c->seek_timestamp = AV_NOPTS_VALUE;
                    break;
                }
                av_free_packet(&var->pkt);
                reset_packet(&var->pkt);
            }
        }
        /* Check if this stream still is on an earlier segment number, or
         * has the packet with the lowest dts */
        if (var->pkt.data) {
            struct variant *minvar = c->variants[minvariant];
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
    if (c->end_of_segment) {
        if (recheck_discard_flags(s, 0))
            goto start;
    }
    /* If we got a packet, return it */
    if (minvariant >= 0) {
        *pkt = c->variants[minvariant]->pkt;
        pkt->stream_index += c->variants[minvariant]->stream_offset;
        reset_packet(&c->variants[minvariant]->pkt);
        return 0;
    }
    return AVERROR_EOF;
}

static int hls_close(AVFormatContext *s)
{
    HLSContext *c = s->priv_data;

    free_variant_list(c);
    return 0;
}

static int hls_read_seek(AVFormatContext *s, int stream_index,
                               int64_t timestamp, int flags)
{
    HLSContext *c = s->priv_data;
    int i, j, ret;

    if ((flags & AVSEEK_FLAG_BYTE) || !c->variants[0]->finished)
        return AVERROR(ENOSYS);

    c->seek_flags     = flags;
    c->seek_timestamp = stream_index < 0 ? timestamp :
                        av_rescale_rnd(timestamp, AV_TIME_BASE,
                                       s->streams[stream_index]->time_base.den,
                                       flags & AVSEEK_FLAG_BACKWARD ?
                                       AV_ROUND_DOWN : AV_ROUND_UP);
    HLS_DEBUG("%s c->seek_timestamp %lld, s->duration %lld\n", __func__, c->seek_timestamp, s->duration);
    timestamp = av_rescale_rnd(timestamp, AV_TIME_BASE, stream_index >= 0 ?
                               s->streams[stream_index]->time_base.den :
                               AV_TIME_BASE, flags & AVSEEK_FLAG_BACKWARD ?
                               AV_ROUND_DOWN : AV_ROUND_UP);
    if (s->duration < c->seek_timestamp) {
        c->seek_timestamp = AV_NOPTS_VALUE;
        return AVERROR(EIO);
    }

    ret = AVERROR(EIO);
    for (i = 0; i < c->n_variants; i++) {
        /* Reset reading */
        struct variant *var = c->variants[i];
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

        /* Locate the segment that contains the target timestamp */
        for (j = 0; j < var->n_segments; j++) {
            if (timestamp >= pos &&
                timestamp < pos + var->segments[j]->duration) {
                var->cur_seq_no = var->start_seq_no + j;
                HLS_DEBUG("hls seek j %d var->n_segments %d, timestamp %lld, pos %lld, s->duration %lld\n",
                    j, var->n_segments, timestamp, pos, s->duration);
                ret = 0;
                break;
            }
            pos += var->segments[j]->duration;
        }
        if (ret)
            c->seek_timestamp = AV_NOPTS_VALUE;
    }
    return ret;
}

static int hls_probe(const AVProbeData *p)
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
        } else{
            HLS_DEBUG("[%s] use ff hls simple demuxer !!!!\n",__func__);
        }
        return 0;
    }
    return 0;
}

AVInputFormat ff_hls_simple_demuxer = {
    .name           = "hls,applehttp",
    .long_name      = NULL_IF_CONFIG_SMALL("Apple HTTP Live Streaming"),
    .priv_data_size = sizeof(HLSContext),
    .read_probe     = hls_probe,
    .read_header    = hls_read_header,
    .read_packet    = hls_read_packet,
    .read_close     = hls_close,
    .read_seek      = hls_read_seek,
};
