/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

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
#include "mpdmanifest.h"
#include <math.h>
#include <sys/time.h>
#include <ctype.h>


//#include "sys_types.h"
//#include "sys_define.h"

#define reconnect_no -108
#define INITIAL_BUFFER_SIZE 32768
#define AV_MAXTS_VALUE          ((int64_t)(0x7FFFFFFFFFFFFFFF))
#define GMT_8_HOUR  (8*3600)


#define DASH_DEBUG printf
#define DASH_ERROR printf

/*
 * An apple http stream consists of a playlist with media segment files,
 * played sequentially. There may be several playlists with the same
 * video content, in different bandwidth dash_varinats, that are played in
 * parallel (preferably only one bandwidth dash_varinat at a time). In this case,
 * the user supplied the url to a main playlist that only lists the dash_varinat
 * playlists.
 *
 * If the main playlist doesn't point at any dash_varinats, we still create
 * one anonymous toplevel dash_varinat for this, to maintain the structure.
 */

#define MAX_TIMELINE_SIZE  32
#define MAX_NUMBER_SEGMENT_SIZE  128

//for vod, save memory
struct segment {
    int duration;//us  AV_TIME_BASE
    char *url;
    char not_full_url;//0 is full url, 1 is for vod timeline, save memory, 2 normal vod number
    int64_t timeline_num;
};

/*
 * Each dash_varinat has its own demuxer. If it currently is active,
 * it has an open AVIOContext too, and potentially an AVPacket
 * containing the next packet from this stream.
 */
#define BPS_CNT 8

#define MAX_STREAMS 20
#define MAX_LANG_LEN   64
typedef struct dash_varinat {
    int bandwidth;
    //char url[MAX_URL_SIZE];
    int resolution;
    AVIOContext *pb;
    uint8_t * read_buffer;
    URLContext * input;
    AVFormatContext * parent;
    AVFormatContext * ctx;
    AVPacket pkt;
    int stream_offset;

    int index;
    int finished;
    int target_duration;
    int start_seq_no;
    int n_segments;
    int var_used_flag;// even changed streams, still use first variant pb and ctx
    struct segment ** segments;

    int timeline_type;//0 is not timeline, 1 is vod, 2 is live
    char timeline_base_url[MAX_URL_SIZE];
    char timeline_replace_url[MAX_URL_SIZE];
    int needed, cur_needed;
    int cur_seq_no;
    int live_start_number;
    int64_t last_load_time;

    //add by xxia
    int last_seq_no;
    //int64_t last_pkt_pts[MAX_STREAMS];
    //int64_t last_pkt_dts[MAX_STREAMS];

    int64_t seek_time_pos;

    char *p_content_protection;

    //char lasturl[MAX_URL_SIZE];
    enum AVMediaType codec_type; /* see CODEC_TYPE_xxx */

    int lastuporddown;
    int lastswbps;
    int download_bps;
    double ts_get_tick;
	int get_cnt;
	int64_t ts_size_cnt;
    int bps[BPS_CNT];
    int bps_index;
    int lang[MAX_LANG_LEN];
    int64_t seek_timestamp;
    int64_t last_opened_timeline;
    int     live_timeline_num;
    int     live_timeline_updated;
    char representation_id[16];
}dash_varinat;

typedef struct DASHContext {
    int n_dash_varinats;
    dash_varinat ** dash_varinats;
    int cur_audio_seq_no;
    int end_of_segment;
    //int first_packet;
    int64_t first_timestamp;
    int64_t seek_timestamp;
    int seek_flags;
    int seekable;
    AVIOInterruptCB * interrupt_callback;
    int profile_isoff_ondemand;
    char * user_agent;                   ///< holds HTTP user agent set as an AVOption to the HTTP protocol context
    char * cookies;                      ///< holds HTTP cookie values set in either the initial response or as an AVOption to the HTTP protocol context

    int cur_video_var;
    int cur_audio_var;
	int next_video_var;
    int next_audio_var;
    int is_support_change_next;

    int64_t last_audio_pkt_dts;//save last pkt dts, for audio track switch
} DASHContext;

extern int g_dash_playmode;//0 is not dash , 1 is vod, 2 is live, 3 is main
extern int g_is_http_chunked;
static int dash_get_parse_mpd(AVFormatContext *s, AVIOContext * in, MPDManifest *p_manifest, int is_update);

static int parse_manifest_update_var(AVFormatContext * s, MPDManifest * p_manifest);

static int delete_amp_from_urls(char **url)
{
    char *tmp_url = *url;
    int urllen = strlen(tmp_url);
    char *result = av_malloc(urllen+1);
    int i = 0, j = 0;

    //	delete amp;
    memset(result, 0x00, urllen + 1);


    for (i = 0; i < urllen;) {
	if (tmp_url[i + 0] == 'a' && tmp_url[i + 1] == 'm' && tmp_url[i + 2] == 'p' && tmp_url[i + 3] == ';') {
	    i += 4;
	} else {
	    result[j] = tmp_url[i];
	    i++;
	    j++;
	}
    }
    result[j] = 0;

    //DASH_DEBUG("real config url is %s\n", result);

    memset(*url, 0x00, urllen + 1);
    memcpy(*url, result, strlen(result));

    av_free(result);

    return 0;
}

static int dash_ffurl_close(dash_varinat * v)
{
    if(v->input)
        ffurl_close(v->input);
    v->input = NULL;
    aviobuf_reset(v->pb);
    return 0;
}


static int read_chomp_line(AVIOContext * s, char * buf, int maxlen)
{
    int len = ff_get_line(s, buf, maxlen);

    while (len > 0 && isspace(buf[len - 1])) {
        buf[--len] = '\0';
    }

    return len;
}

static void free_dash_segment_list(dash_varinat * var)
{
    int i;

    for (i = 0; i < var->n_segments; i++) {
        if(var->segments[i]->url)
            av_free(var->segments[i]->url);
        av_free(var->segments[i]);
    }

    av_freep(&var->segments);
    var->n_segments = 0;
}

static void free_dash_varinat_list(DASHContext * c)
{
    int i;

    for (i = 0; i < c->n_dash_varinats; i++) {
        dash_varinat * var = c->dash_varinats[i];
        free_dash_segment_list(var);
        av_free_packet(&var->pkt);
        if(var->pb && var->var_used_flag)//only start stream var_used_flag is 1!!
        {
            av_free(var->pb->buffer);
            av_free(var->pb);
        }

        if (var->input) {
            dash_ffurl_close(var);
        }

        av_usleep(100 * 1000);

        if (var->ctx && var->var_used_flag) {
            //   var->ctx->pb = NULL; // linda zhu remove it for "double_free crash" in mov_read_close()->avio_close(sc->pb);
            //av_usleep(100 * 1000);
            //var->ctx->pb->opaque = NULL;
            avformat_close_input(&var->ctx);
        }

        if(var->p_content_protection)
            av_free(var->p_content_protection);

        av_usleep(100 * 1000);
        av_free(var);
    }

    av_freep(&c->dash_varinats);
    av_freep(&c->cookies);
    av_freep(&c->user_agent);
    c->n_dash_varinats = 0;
#if 0//def USE_PB_FIFO peacer del 20180411

    if (p_seq->is_fifo_playback) {
        p_seq->dash_parser.enable = 0;

        if (p_seq->dash_parser.ts_url) {
            mtos_free(p_seq->dash_parser.ts_url);
            p_seq->dash_parser.ts_url = NULL;
        }
    }

#endif
}

/*
 * Used to reset a statically allocated AVPacket to a clean slate,
 * containing no data.
 */
static void reset_packet(AVPacket * pkt)
{
    av_init_packet(pkt);
    pkt->data = NULL;
}

static dash_varinat * new_dash_varinat(DASHContext * c, int bandwidth,
        const char * url, const char * base)
{
    int i = 0;
    dash_varinat * var = av_mallocz(sizeof(dash_varinat));

    if (!var) {
        return NULL;
    }

    reset_packet(&var->pkt);
    var->bandwidth = bandwidth;

    //ff_make_absolute_url(var->url, sizeof(var->url), base, url);
    /*if (url && strlen(url)) {
        memcpy(var->url, url, strlen(url));
    }*/

    var->seek_time_pos = 0;

    /*for (i = 0; i < MAX_STREAMS; i++) {
        var->last_pkt_dts[i] = 0;
        var->last_pkt_dts[i] = 0;
    }*/
    var->live_start_number = -1;
    var->seek_timestamp  = AV_NOPTS_VALUE;

    dynarray_add(&c->dash_varinats, &c->n_dash_varinats, var);
    return var;
}

static int readlen_eachstream = 0;
static int requirelen_eachstream = 0;
#define REPEAT_TIMES 3

static void strreplace(char * src, char * sub, char * replace, char * dst)
{
    char file_prefix[2048] = {0};
    char file_suffix[2048] = {0};
    char * file_pos = NULL;

    if (src == NULL || sub == NULL || replace == NULL || dst == NULL) {
        return;
    }

    file_pos = strstr(src, sub);

    if (file_pos) {
        memset(file_prefix, 0, sizeof(file_prefix));
        memset(file_suffix, 0, sizeof(file_suffix));
        strncpy(file_prefix, src, file_pos - src);
        strcpy(file_suffix, file_pos + strlen(sub));
        memset(dst, 0, strlen(dst));            //   for tsscan
        sprintf(dst, "%s%s%s", file_prefix, replace, file_suffix);
    }

    return;
}


//real_var is used to switch video, record the playing var.
static int dash_open_segmeng_url(DASHContext * c, struct segment * seg, dash_varinat * var, dash_varinat * real_var)
{
    AVDictionary * opts = NULL;
    int ret = 0;
    // broker prior HTTP options that should be consistent across requests
    av_dict_set(&opts, "user-agent", c->user_agent, 0);
    av_dict_set(&opts, "cookies", c->cookies, 0);
    //if(c->profile_isoff_ondemand)
    //    av_dict_set(&opts, "seekable", "1", 1);
    //else
    //    av_dict_set(&opts, "seekable", "0", 0);

    if(seg->not_full_url)
    {
        //DASH_DEBUG("%s timeline_base_url %s\n",__func__,real_var->timeline_base_url);
        //DASH_DEBUG("%s timeline_replace_url %s, seg->url %s\n",
        //    __func__,real_var->timeline_replace_url, seg->url);
        char file_name_tmp[MAX_URL_SIZE] = {0};
        if(seg->not_full_url == 1)
        {
            if(strstr(real_var->timeline_replace_url, "$Time$"))
            {
                strreplace(real_var->timeline_replace_url, "$Time$", seg->url, file_name_tmp);
            }
            else if(strstr(real_var->timeline_replace_url, "$Number$"))
            {
                strreplace(real_var->timeline_replace_url, "$Number$", seg->url, file_name_tmp);
            }
        }
        else if(seg->not_full_url == 2)
        {
            memset(file_name_tmp, 0, MAX_URL_SIZE);
            strcpy(file_name_tmp, seg->url);
        }
        char url[MAX_URL_SIZE] = {0};
        sprintf(url, "%s%s", real_var->timeline_base_url, file_name_tmp);
        DASH_DEBUG("%s %d codec %d url %s\n", __func__, __LINE__, real_var->codec_type, url);
        ret = ffurl_open(&var->input, url, AVIO_FLAG_READ,
                         &var->parent->interrupt_callback, &opts);
        if(seg->not_full_url == 1 && ret == 0)
            var->last_opened_timeline = seg->timeline_num;
    }
    else
    {
        DASH_DEBUG("%s %d seg->url %s\n", __func__, __LINE__, seg->url);
        ret = ffurl_open(&var->input, seg->url, AVIO_FLAG_READ,
                         &var->parent->interrupt_callback, &opts);
    }

    if(ret == 0) {
        var->pb->is_chunked = var->input->is_chunked;
        //DASH_DEBUG("%s %d var->pb 0x%x, var->input->is_chunked %d var->pb->is_chunked_reset %d\n",
        //    __func__, __LINE__, var->pb, var->input->is_chunked, var->pb->is_chunked_reset);
    }

cleanup:
    av_dict_free(&opts);
    return ret;
}

static int dash_open_input(DASHContext * c, dash_varinat * var)
{
    int ret;
    struct segment * seg = NULL;
    struct segment tmp_live_seg;
    dash_varinat *real_var = NULL;

    tmp_live_seg.url = NULL;
    if(g_dash_playmode == 2)
        real_var = var;
    else
    {
        if(var->codec_type == AVMEDIA_TYPE_VIDEO)
            real_var = c->dash_varinats[c->cur_video_var];
        if(var->codec_type == AVMEDIA_TYPE_AUDIO)
            real_var = c->dash_varinats[c->cur_audio_var];
    }
    if(g_dash_playmode == 2 && var->live_start_number != -1 && var->cur_seq_no > 0)
    {
        seg = var->segments[var->n_segments - 1];
        memcpy(&tmp_live_seg, seg, sizeof(struct segment));
        tmp_live_seg.url = (char *)av_mallocz(MAX_URL_SIZE);
        if(tmp_live_seg.url == NULL)
        {
            DASH_ERROR("[%s] %d, malloc MAX_URL_SIZE fail\n", __func__, __LINE__);
            return AVERROR_EOF;
        }
        sprintf(tmp_live_seg.url, seg->url, (var->live_start_number + var->cur_seq_no - 1));
        seg = &tmp_live_seg;
       //DASH_DEBUG("zx seg->url %s\n",seg->url);
    }
    else if (var->cur_seq_no - var->start_seq_no >= var->n_segments) {
        DASH_ERROR("[%s] %d, cur_seq_no %d,start_seq_no %d,n_segments %d\n", __func__, __LINE__, var->cur_seq_no, var->start_seq_no, var->n_segments);
        return AVERROR_EOF;
    }
    else
        seg = real_var->segments[var->cur_seq_no - var->start_seq_no];


    ret = dash_open_segmeng_url(c, seg, var, real_var);

    if(tmp_live_seg.url != NULL)
        av_free(tmp_live_seg.url);
    return ret;
}

static int64_t dash_live_read_seek(void * opaque, int64_t offset, int whence)
{
    dash_varinat * v = opaque;
    int64_t seek_ret;
    //printf("%s enter v->input 0x%x!!!\n", __func__, v->input);

    if(v->input == NULL)
        return 0LL;
    if (whence == AVSEEK_SIZE)
    {
        seek_ret = ffurl_seek(v->input, 0, AVSEEK_SIZE);
        return seek_ret;
    }
    else
        DASH_ERROR("%s dont support whence 0x%x!!!\n", __func__, whence);

    return 0LL;
}

static int64_t read_seek(void * opaque, int64_t offset, int whence)
{
    dash_varinat * v = opaque;
    int64_t seek_ret;
    DASHContext * c = v->parent->priv_data;
    int ret;
#if 0//def USE_PB_FIFO peacer del 20180411
    int fifo_read_repeat = 0;
    //DASH_DEBUG("%s input %x\n",__func__,v->input);
    FILE_SEQ_T * pFileSeq =  file_seq_get_instance();
    fifo_type_t * p_pb_fifo_handle = pFileSeq->p_pb_fifo_handle;
#endif

    if (!v->input) {
        int stream_reconnect = 0;
    reload:

        /*if (av_player_is_exit()) {
            DASH_DEBUG("[%s] ---detect stop commond at line %d! \n", __func__, __LINE__);
            return AVERROR_EOF;
        }*/

        DASH_DEBUG("%s:v->cur_seq_no %d v->start_seq_no %d segment %d\n",
                  __func__, v->cur_seq_no, v->start_seq_no, v->n_segments);

        //for ifeng hls start_seq_no will change from time to time
        if (v->cur_seq_no > v->start_seq_no + v->n_segments
            || v->cur_seq_no < v->start_seq_no) {
            DASH_ERROR("i think n_seg %d cur seq no %d start_seq_no %d has modify error\n",
                      v->n_segments, v->cur_seq_no, v->start_seq_no);
#if 0//def USE_PB_FIFO peacer del 20180411

            if (pFileSeq->is_fifo_playback) {
                pFileSeq->dash_parser.cur_seq_num = v->cur_seq_no;
            }

#endif
            DASH_DEBUG("AFTER COMPARE cur seq no %d start_seq_no %d \n",
                      v->cur_seq_no, v->start_seq_no);
        }

        if (v->cur_seq_no < v->start_seq_no) {
            av_log(NULL, AV_LOG_WARNING,
                   "skipping %d segments ahead, expired from playlists\n",
                   v->start_seq_no - v->cur_seq_no);
            v->cur_seq_no = v->start_seq_no;
#if 0//def USE_PB_FIFO peacer del 20180411

            if (pFileSeq->is_fifo_playback) {
                pFileSeq->dash_parser.cur_seq_num = v->cur_seq_no;
            }

#endif
        }

        ret = dash_open_input(c, v);

        if (ret < 0) {
            stream_reconnect ++;
            DASH_DEBUG("%s open_input ret %d ,cur_seq %d reconnect %d\n",
                      __func__, ret, v->cur_seq_no, stream_reconnect);

            if (stream_reconnect < REPEAT_TIMES) {
                goto reload;
            }

            return ret;
        }

        //copy the last ts url
        /*struct segment * seg = v->segments[v->cur_seq_no - v->start_seq_no];
        memset(v->lasturl, 0, MAX_URL_SIZE);
        memcpy(v->lasturl, seg->url, strlen(seg->url));*/
    }

    if (whence == AVSEEK_SIZE) {
        seek_ret = ffurl_seek(v->input, 0, AVSEEK_SIZE);
        return seek_ret;
    } else if (whence == SEEK_CUR) {
        seek_ret = ffurl_seek(v->input, 0, SEEK_CUR);
        return seek_ret;
    } else if (whence == SEEK_END) {
        seek_ret = ffurl_seek(v->input, -1, SEEK_END);
        return seek_ret;
    } else if (whence == SEEK_SET) {
        seek_ret = ffurl_seek(v->input, offset, SEEK_SET);
        return seek_ret;
    } else {
        return -1;
    }
}

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

#define BPS_THRESH (0.9)
static int dash_recent_bps_is_enough(DASHContext * c, int next_var)
{
   int ret =1 ;
   int i =0;
   dash_varinat * vv = c->dash_varinats[next_var];
   int next_bps =  vv->bandwidth;
   return 1;
   for(i=0;i<BPS_CNT;i++)
   {
  	    //DASH_DEBUG("%s %d, index[i] bps %d\n",__FUNCTION__,__LINE__,i, c->bps[i]);
  	    if(vv->bps[i]!=0&&vv->bps[i]*BPS_THRESH<next_bps)
  		{
      		ret = 0;
    		break;
  		}
   }
   return ret;
}
#define MIN(a,b)(((a)>(b))?(b):(a))

static int dash_find_recent_min_bps(dash_varinat * vv)
{
   int ret =1 ;
   int i =0;

   int min_bps =  0x7fffffff;
   for(i=0;i< BPS_CNT;i++)
   {
      //DASH_DEBUG("%s %d, index[%d] bps %d\n",__FUNCTION__,__LINE__,i, c->bps[i]);
  	  if(vv->bps[i]!=0)
  	  {
  		min_bps = MIN(vv->bps[i],min_bps);
  	  }
  	}
   //DASH_DEBUG("c->bps_index %d, min_bps %d\n", vv->bps_index, min_bps);
   return min_bps;
}

static int change_to_next_dash(DASHContext * c, dash_varinat * vv)
{
    int now_bps = 0;
    int  i = 0;
    int last_bps = 0;
    int next_cur = 0;
    int upordown =0;
    c->next_video_var = -1;
    c->next_audio_var = -1;
	vv->bps[vv->bps_index] = vv->download_bps;
    if(vv->download_bps*BPS_THRESH > vv->bandwidth)
    {
        now_bps = dash_find_recent_min_bps(vv);

    	if(now_bps > vv->download_bps )
    		now_bps = vv->download_bps;
    }
    else
    	now_bps = vv->download_bps;


    vv->bps_index++;
    if(vv->bps_index>= BPS_CNT)
        vv->bps_index = 0;
    for (i = 0; i < c->n_dash_varinats; i++) {
        dash_varinat * v = c->dash_varinats[i];
        if(v->codec_type == vv->codec_type)
        {
        //printf("now_bps %d, i[%d] bandwidth %d, last_bps %d\n", (int)(now_bps * BPS_THRESH), i, v->bandwidth, last_bps);
            if (now_bps * BPS_THRESH > (v->bandwidth)
                && v->bandwidth > last_bps) {
                last_bps = v->bandwidth;
                if(vv->codec_type == AVMEDIA_TYPE_VIDEO)
                    c->next_video_var = i;
                else if(vv->codec_type == AVMEDIA_TYPE_AUDIO)
                    c->next_audio_var = i;
            }
        }
    }

    if(last_bps == 0)
    {
        //now_bps is smaller than the smallest variant
        int min_diff_bps = 0x7fffffff;
        for (i = 0; i < c->n_dash_varinats; i++)
        {
            dash_varinat * v = c->dash_varinats[i];
            if(v->codec_type == vv->codec_type)
            {
                int cur_diff = abs(now_bps - v->bandwidth);
                if (cur_diff < min_diff_bps)
                {
                    min_diff_bps = cur_diff;
                    //DASH_DEBUG("min_diff_bps %d, bandwidth %d\n", min_diff_bps, v->bandwidth);
                    if(vv->codec_type == AVMEDIA_TYPE_VIDEO)
                        c->next_video_var = i;
                    else if(vv->codec_type == AVMEDIA_TYPE_AUDIO)
                        c->next_audio_var = i;
                }
            }
        }
        DASH_DEBUG("c->next_video_var %d, bandwidth %d\n", c->next_video_var, c->dash_varinats[c->next_video_var]->bandwidth);
    }

    DASH_DEBUG("c->cur_audio_var %d, c->next_audio_var %d\n", c->cur_audio_var, c->next_audio_var);
    DASH_DEBUG("c->cur_video_var %d, c->next_video_var %d\n", c->cur_video_var, c->next_video_var);

    int next_var = -1;
    if(vv->codec_type == AVMEDIA_TYPE_AUDIO && c->cur_audio_var != c->next_audio_var)
    {
        next_var = c->next_audio_var;
    }
    else if(vv->codec_type == AVMEDIA_TYPE_VIDEO&& c->cur_video_var != c->next_video_var)
    {
        next_var = c->next_video_var;
    }
    if (next_var != -1) {
		 dash_varinat * v = c->dash_varinats[next_var];
		 //selected_url  = 1;
		 if(vv->bandwidth < v->bandwidth)
			upordown = 1;
	     else if(vv->bandwidth>v->bandwidth)
			upordown = -1;

		if(upordown!=1
			||(/*upordown==1&&*/dash_recent_bps_is_enough(c, next_var)==1)      // for tsscan
			)
	 	{
            //vv->last_sw_tick = ff_get_ms(c);
			vv->lastuporddown = upordown;
            vv->lastswbps = now_bps;
            if(vv->codec_type == AVMEDIA_TYPE_AUDIO )
            {
                c->cur_audio_var = next_var;
                DASH_DEBUG("audio change to var %d, upordown %d!!!\n", next_var, upordown);
            }
            else if(vv->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                c->cur_video_var = next_var;
                DASH_DEBUG("video change to var %d, upordown %d!!!\n", next_var, upordown);

            }


            dash_varinat *new_vv = c->dash_varinats[next_var];
            new_vv->parent = vv->parent;
            new_vv->ctx = vv->ctx;
            new_vv->pb = vv->pb;
            new_vv->pb->opaque = (void *)new_vv;
            new_vv->ctx->pb->opaque = (void *)new_vv;
            new_vv->needed = 1;
            new_vv->cur_seq_no = vv->cur_seq_no;

            vv = new_vv;
            new_vv->ctx->is_change_next = 1;
            if(c->profile_isoff_ondemand)
            {
                av_free_packet(&new_vv->pkt);
                reset_packet(&new_vv->pkt);
                new_vv->pb->eof_reached = 0;
                /* Clear any buffered data */
                new_vv->pb->buf_end = new_vv->pb->buf_ptr = new_vv->pb->buffer;
                /* Reset the pos, to let the mpegts demuxer know we've seeked. */
                new_vv->pb->pos = 0;
                new_vv->needed = 1;
            }

            DASH_DEBUG("new_vv->codec_type %d, start_seq_no %d\n", new_vv->codec_type, new_vv->start_seq_no);
            if(new_vv->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                struct segment *seg = new_vv->segments[new_vv->start_seq_no];// open init.mp4 or new stream
                dash_open_segmeng_url(c, seg, new_vv, new_vv);

                //av_seek_frame(new_vv->ctx, 0, 0, AVSEEK_FLAG_BACKWARD);//dash only 1 stream
            }

            return 1;
            //file_seq_set_reset_state(1);
        }

    }

    return 0;
}

static int check_change_dash(DASHContext *c, dash_varinat *v)
{
    if(v->pb->is_chunked == 0 && v->ts_get_tick > 5 && c->is_support_change_next)
    {
        float downloadbps = 0;

		//c->opened = 0;
#define BPS_ALPHA 0.8
        downloadbps = 1000.f * v->ts_size_cnt * 8.f / ((v->ts_get_tick));
        //DASH_DEBUG(" ts_size_cnt %lld vv->ts_get_tick %d\n",  vv->ts_size_cnt, (int)(vv->ts_get_tick*100));

        if (v->download_bps == 0) {
            v->download_bps = downloadbps;
        } else {
            v->download_bps = v->download_bps * (1 - BPS_ALPHA) + downloadbps * BPS_ALPHA;
        }

        if ((++(v->get_cnt)) % 3 == 0 && v->codec_type == AVMEDIA_TYPE_VIDEO) {
            DASH_DEBUG("codec_type %d v->download_bps %d\n", v->codec_type, v->download_bps);
            return change_to_next_dash(c, v);
        }
    }

    return 0;
}

static int update_dash_mpd(AVFormatContext *s)
{
    MPDManifest manifest;
    int ret = 0;
    int64_t filesize;
    uint8_t * buf;
    AVIOContext * in = NULL;

    memset(&manifest, 0, sizeof(MPDManifest));
    if (!in) {
        //AVDictionary *opts = NULL;
        /* Some HLS servers don't like being sent the range header */
        //av_dict_set(&opts, "seekable", "0", 0);

        // broker prior HTTP options that should be consistent across requests
        //av_dict_set(&opts, "user-agent", c->user_agent, 0);
        //av_dict_set(&opts, "cookies", c->cookies, 0);

        DASH_DEBUG("%s ffurl_open url %s\n",__func__, s->filename);
        ret = avio_open2(&in, s->filename, AVIO_FLAG_READ,
                            NULL, NULL);
        //av_dict_free(&opts);
        if (ret < 0)
            return ret;
    }

    filesize = avio_size(in) + 1;

    //DASH_DEBUG("[%s] line[%d]!,filesize[%d] is_chunked[%d] \n", __FUNCTION__, __LINE__, filesize, in->is_chunked);
    /*if(g_is_http_chunked) {
        DASH_ERROR("[%s] line[%d] chunk mpd !\n", __FUNCTION__, __LINE__);
        filesize = 64*1024;
    }*/

    if (filesize <= 0) {
        DASH_ERROR("[%s] line[%d] filesize %d <= 0 !\n", __FUNCTION__, __LINE__, filesize);
        ret = -1;
        goto fail;
    }
    buf = av_mallocz(filesize * sizeof(uint8_t));

    if (!buf) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    int len = avio_read(in, buf, filesize);
    //DASH_DEBUG("[%s] line[%d]!,sizeofmanifest[%d]\n", __FUNCTION__, __LINE__, sizeof(MPDManifest));
    if (len <= 0) {
        DASH_ERROR("[%s] line[%d]!,filesize[%lld] len[%d]\n", __FUNCTION__, __LINE__, filesize, len);
        av_free(buf);
        goto fail;
    }

    if ((ret = ff_parse_mpd_manifest(buf, strlen(buf), &manifest)) < 0) {
        DASH_ERROR("[%s] line[%d] ff_parse_mpd_manifest failed\n", __FUNCTION__, __LINE__);
        av_free(buf);
        ff_free_mpd_manifest(&manifest);
        goto fail;
    }

    //DASH_DEBUG("[%s] line[%d],v[%d], s 0x%x &manifest 0x%x\n",
    //    __FUNCTION__, __LINE__, c->n_dash_varinats, s, &manifest);
    av_free(buf);

    if ((ret = parse_manifest_update_var(s, &manifest)) < 0) {
        goto fail;
    }

    ff_free_mpd_manifest(&manifest);
fail:
    if(in)
        avio_close(in);
    return 0;
}

static int dash_read_url(void * opaque, uint8_t * buf, int buf_size)
{
    dash_varinat * v = opaque;
    DASHContext * c = v->parent->priv_data;
    int ret, i;
    int64_t tick1,tick2;

restart:


    if (!v->input) {
        int stream_reconnect = 0;
live_timeline_update:
        if( (v->cur_seq_no == v->n_segments)
            && g_dash_playmode == 2 && v->timeline_type == 2) {//zx11
            DASH_DEBUG("%s v->live_timeline_num %d/%d live timeline, update mpd last_opened_timeline %lld\n",
                __func__, v->live_timeline_num, v->n_segments, v->last_opened_timeline);
            AVFormatContext *s = v->parent;
            if(update_dash_mpd(s) == 0)
            {
                //DASH_DEBUG("%s index %d v->live_timeline_num %d/%d live timeline, goto restart\n",
                //      __func__, v->index, v->cur_seq_no, v->n_segments);
                v->live_timeline_updated = 0;//this dont cancel ++, another stream will cancel ++
                goto restart;
            }
            else {
                DASH_DEBUG("[%s] %d update_dash_mpd error \n", __func__, __LINE__);
                return AVERROR_EOF;
            }
        }

reload:

        if (av_player_is_exit()) {
            DASH_DEBUG("[%s] ---detect stop commond at line %d! \n", __func__, __LINE__);
            return AVERROR_EOF;
        }

        DASH_DEBUG("%s: v 0x%x codec_type %d, v->cur_seq_no %d v->start_seq_no %d segment %d\n",
                  __func__, v, v->codec_type, v->cur_seq_no, v->start_seq_no, v->n_segments);

        if(v->live_start_number == -1 && v->timeline_type == 0)
        {
            //for ifeng hls start_seq_no will change from time to time
            if (v->cur_seq_no > v->start_seq_no + v->n_segments
                || v->cur_seq_no < v->start_seq_no) {
                DASH_ERROR("i think n_seg %d cur seq no %d start_seq_no %d has modify error\n",
                          v->n_segments, v->cur_seq_no, v->start_seq_no);

                DASH_ERROR("AFTER COMPARE cur seq no %d start_seq_no %d \n",
                          v->cur_seq_no, v->start_seq_no);
            }

            if (v->cur_seq_no < v->start_seq_no) {
                DASH_ERROR("skipping %d segments ahead, expired from playlists\n",
                       v->start_seq_no - v->cur_seq_no);
                v->cur_seq_no = v->start_seq_no;
            }
        }

        v->ts_get_tick = 0; //ff_get_ms();
        v->ts_size_cnt = 0;
        tick1 = ff_get_ms();

        ret = dash_open_input(c, v);

        if (ret < 0) {
            stream_reconnect ++;
            DASH_DEBUG("%s open_input ret %d ,cur_seq %d reconnect %d g_dash_playmode %d\n",
                      __func__, ret, v->cur_seq_no, stream_reconnect, g_dash_playmode);

            if (stream_reconnect < REPEAT_TIMES || g_dash_playmode == 2) {
                av_usleep(100 * 1000);
                goto reload;
            }

            return ret;
        }
        else if(v->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            tick2 = ff_get_ms();
    	  	//c->opened = 1;
            v->ts_get_tick += ((tick2 - tick1));
            // printf("\n%s %d tick %f  ret %d\n",__FUNCTION__,__LINE__,(vv->tick2 - vv->tick1),ret);
        }

        //copy the last ts url
        /*struct segment * seg = v->segments[v->cur_seq_no - v->start_seq_no];
        memset(v->lasturl, 0, MAX_URL_SIZE);
        memcpy(v->lasturl, seg->url, strlen(seg->url));*/
    }

    if(buf_size == -1)
    {
        if(c->profile_isoff_ondemand && v->ctx && v->ctx->streams)
        {
            if(v->ctx->streams[0]->nb_index_entries == 0)
            {
                if(check_change_dash(c, v) == 1)
                {
                    v->ts_get_tick = 0; //ff_get_ms();
                    v->ts_size_cnt = 0;
                    return 1;
                }
            }
        }
        return 0;
    }

    tick1 = ff_get_ms(c);

    ret = ffurl_read(v->input, buf, buf_size);

    if (ret > 0) {
        if(v->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            requirelen_eachstream += buf_size;
            readlen_eachstream += ret;
            v->ts_size_cnt += ret;
            tick2 = ff_get_ms(c);
            v->ts_get_tick += ((tick2 - tick1));
        }
        return ret;
    }

    if (ret < 0) {
        DASH_DEBUG("%s recv timeout 5s v %d ret %d ,total read size %d \n",
                  __func__, v->index, ret, readlen_eachstream);

        if (ret == reconnect_no) {
            dash_ffurl_close(v);
            goto restart;
        }
    }

    dash_ffurl_close(v);
    if(c->profile_isoff_ondemand)
            return ret;

    if(v->codec_type == AVMEDIA_TYPE_VIDEO && check_change_dash(c, v) == 1)
        return 0;

    if(v->live_timeline_updated == 1)
        v->live_timeline_updated = 0;//updated, this time dont ++
    else
        v->cur_seq_no ++;
    v->live_timeline_num ++;
    //DASH_DEBUG("%s line %d,v->cur_seq_no%d \n", __func__, __LINE__, v->cur_seq_no);
    readlen_eachstream = 0;
    requirelen_eachstream = 0;
    c->end_of_segment = 1;
    if(v->codec_type == AVMEDIA_TYPE_AUDIO)
        c->cur_audio_seq_no = v->cur_seq_no;

    if(v->pb->is_chunked == 1){
        v->pb->is_chunked_reset = 1;
    }
    //DASH_DEBUG("%s pb 0x%x, v->pb->is_chunked %d %d\n", __func__, v->pb, v->pb->is_chunked, v->pb->is_chunked_reset);

#if 0
    //remove by zhouxiang, dont open all streams
    if (v->ctx && v->ctx->nb_streams && v->parent->nb_streams >= v->stream_offset + v->ctx->nb_streams) {
        v->needed = 0;

        for (i = v->stream_offset; i < v->stream_offset + v->ctx->nb_streams;
             i++) {
            if (v->parent->streams[i]->discard < AVDISCARD_ALL) {
                v->needed = 1;
            }
        }
    }

    if (!v->needed) {
        av_log(v->parent, AV_LOG_INFO, "No longer receiving dash_varinat\n");
        DASH_DEBUG("%s line %d,No longer receiving dash_varinat\n", __func__, __LINE__);
        return AVERROR_EOF;
    }
#endif

    //DASH_DEBUG("%s line %d,AVERROR_EOF \n", __func__, __LINE__);
    goto restart;
}

static int read_data(void * opaque, uint8_t * buf, int buf_size)
{
    //bento4 demux mp4, must init.mp4+segx.m4f
    int len = dash_read_url(opaque, buf, buf_size);
    //printf("zxtmp dash read_data %d\n", len);
    return len;
}

static uint64_t get_current_time_in_sec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec;
}

static void live_get_seg_url_by_timeline(MPD_AdaptationSet *cur_adaptationset, MPD_Representation * cur_representation,
    char *file_name, char *mpd_base_url, char *adaptationSet_base_url, dash_varinat * var, float file_duration)
{
    int k = 0;
    long long segmentTime = 0;
    char number_string[MAX_TIMELINE_SIZE] = {0};
    struct segment * seg = NULL;
    int replace_type = 0;
    int dst_i = 1;

    //DASH_DEBUG("%s file_name %s\n", __func__, file_name);
    if(strstr(file_name, "$Time$"))
        replace_type = 0;
    else if(strstr(file_name, "$Number$"))
        replace_type = 1;
    else
    {
        DASH_ERROR("%s: ERROR file_name %s dont support segmentTimeLine\n", __func__, file_name);
        return;
    }

    memset(var->timeline_base_url, 0, MAX_URL_SIZE);
    sprintf(var->timeline_base_url, "%s%s", mpd_base_url,  adaptationSet_base_url);
    memset(var->timeline_replace_url, 0, MAX_URL_SIZE);
    memcpy(var->timeline_replace_url, file_name,  strlen(file_name));
    var->timeline_type = 2;
    DASH_DEBUG("%s var->index %d %s\n", __func__, var->index, var->timeline_replace_url);
    //DASH_DEBUG("%s segmentTimeLine len %d\n", __func__, cur_representation->segmentTimeLine.len);



    for (k = 0; k < cur_representation->segmentTimeLine.len; k++) {
        int r = cur_representation->segmentTimeLine.snode[k].r;
        long long t = cur_representation->segmentTimeLine.snode[k].t;
        long d = cur_representation->segmentTimeLine.snode[k].d;
        int m = 0;
        if (t > 0) {
            segmentTime = t;
        }
        do {
            DASH_DEBUG("%s %d, k:%d, m:%d, r:%d, d:%ld, t:%lld\n", __FUNCTION__, __LINE__, k, m, r, d, t);

            if(replace_type == 0)
            {
                sprintf(number_string, "%lld", segmentTime);
                //strreplace(file_name, "$Time$", number_string, file_name_tmp);
            }
            else if(replace_type == 1)
            {
                sprintf(number_string, "%d", dst_i);
                //strreplace(file_name, "$Number$", number_string, file_name_tmp);
                dst_i++;
            }
            seg = av_mallocz(sizeof(struct segment));
            seg->not_full_url = 1;
            seg->url = (char *)av_mallocz(MAX_TIMELINE_SIZE);
            memcpy(seg->url, number_string, strlen(number_string));
            if(replace_type == 0)
                seg->timeline_num = segmentTime;
            else if(replace_type == 1)
                seg->timeline_num = dst_i;
            DASH_DEBUG("update var index %d, n_segments:%d timeline seg->url=%s\n", var->index, var->n_segments, seg->url);
            dynarray_add(&var->segments, &var->n_segments, seg);

            segmentTime += d;

        } while (m++ < r);
    }

}


static void get_seg_url_by_timeline(MPD_AdaptationSet *cur_adaptationset, MPD_Representation * cur_representation,
    char *file_name, char *mpd_base_url, char *adaptationSet_base_url, dash_varinat * var, float file_duration,
    long *in_tmp_file_duration)
{
    int k = 0, m = 0;
    long long segmentTime = 0;
    char number_string[MAX_TIMELINE_SIZE] = {0};
    long tmp_file_duration = 0;
    struct segment * seg = NULL;
    int replace_type = 0;
    int dst_i = 1;

    int timescale = 1;
    if (cur_representation->timescale > 0) {
        timescale = cur_representation->timescale;
    } else if (cur_adaptationset->timescale > 0) {
        timescale = cur_adaptationset->timescale;
    }

    DASH_DEBUG("%s file_name %s\n", __func__, file_name);
    if(strstr(file_name, "$Time$"))
        replace_type = 0;
    else if(strstr(file_name, "$Number$"))
        replace_type = 1;
    else
    {
        DASH_ERROR("%s: ERROR file_name %s dont support segmentTimeLine\n", __func__, file_name);
        return;
    }

    memset(var->timeline_base_url, 0, MAX_URL_SIZE);
    sprintf(var->timeline_base_url, "%s%s", mpd_base_url,  adaptationSet_base_url);
    memset(var->timeline_replace_url, 0, MAX_URL_SIZE);
    memcpy(var->timeline_replace_url, file_name,  strlen(file_name));
    var->timeline_type = 1;

    if(cur_adaptationset->segmentTimeLine.len)
    {
        for (k = 0; k < cur_adaptationset->segmentTimeLine.len; k++) {

            if ((cur_adaptationset->segmentTimeLine.snode[k].r < 0) && (cur_adaptationset->segmentTimeLine.snode[k].d > 0) && (file_duration > 0.0)) {
                if (cur_adaptationset->timescale > 0.0) {
                    cur_adaptationset->segmentTimeLine.snode[k].r = (file_duration * cur_adaptationset->timescale - segmentTime) / cur_representation->segmentTimeLine.snode[k].d + 1;
                    //cur_representation->segmentTimeLine.snode[k].r = cur_representation->duration / cur_representation->timescale;
                }
            }

            //playready dash, r=n is n+1!!!
            if (cur_adaptationset->segmentTimeLine.snode[k].r >= 0) {
                int r_n = cur_adaptationset->segmentTimeLine.snode[k].r + 1;//playready dash, r=n is n+1!!!
                if(cur_adaptationset->segmentTimeLine.snode[k].t > 0)
                {
                    segmentTime = cur_adaptationset->segmentTimeLine.snode[k].t;
                    segmentTime -= cur_adaptationset->segmentTimeLine.snode[k].d;//first must 0.m4s, for loop will add
                    r_n++;
                }
                for (m = 0; m < r_n; m++) {
                    segmentTime = segmentTime + cur_adaptationset->segmentTimeLine.snode[k].d;
                    //printf("k %d d %d\n",k, cur_representation->segmentTimeLine.snode[k].d);
                    tmp_file_duration = tmp_file_duration + cur_adaptationset->segmentTimeLine.snode[k].d;
                    if(replace_type == 0)
                    {
                        sprintf(number_string, "%lld", segmentTime);
                    }
                    else if(replace_type == 1)
                    {
                        sprintf(number_string, "%d", dst_i);
                        dst_i++;
                    }
                    seg = av_mallocz(sizeof(struct segment));
                    seg->not_full_url = 1;
                    seg->url = (char *)av_mallocz(MAX_TIMELINE_SIZE);
                    memcpy(seg->url, number_string, strlen(number_string));
                    if(timescale > 0)
                        seg->duration = (int)(cur_adaptationset->segmentTimeLine.snode[k].d*((float)AV_TIME_BASE/(float)timescale));
                    dynarray_add(&var->segments, &var->n_segments, seg);
                }
            }
        }
    }
    else
    {
        for (k = 0; k < cur_representation->segmentTimeLine.len; k++) {

            if ((cur_representation->segmentTimeLine.snode[k].r < 0) && (cur_representation->segmentTimeLine.snode[k].d > 0) && (file_duration > 0.0)) {
                if (cur_representation->timescale > 0.0) {
                    cur_representation->segmentTimeLine.snode[k].r = (file_duration * cur_representation->timescale - segmentTime) / cur_representation->segmentTimeLine.snode[k].d + 1;
                    //cur_representation->segmentTimeLine.snode[k].r = cur_representation->duration / cur_representation->timescale;
                } //else if (cur_representation->timescale > 0.0) {
                  //  cur_representation->segmentTimeLine.snode[k].r = (file_duration * cur_representation->timescale - segmentTime) / cur_representation->segmentTimeLine.snode[k].d + 1;
                //}
            }

            //if(k<5)
                //mtos_printk("k = %d, t =%lld, r = %d , d = %d\n",
                //k, cur_representation->segmentTimeLine.snode[k].t,cur_representation->segmentTimeLine.snode[k].r
                //    ,cur_representation->segmentTimeLine.snode[k].d);
            //playready dash, r=n is n+1!!!
            if (cur_representation->segmentTimeLine.snode[k].r >= 0) {
                int r_n = cur_representation->segmentTimeLine.snode[k].r + 1;//playready dash, r=n is n+1!!!
                if(cur_representation->segmentTimeLine.snode[k].t > 0)
                {
                    segmentTime = cur_representation->segmentTimeLine.snode[k].t;
                    segmentTime -= cur_representation->segmentTimeLine.snode[k].d;//first must 0.m4s, for loop will add
                    r_n++;
                }
                for (m = 0; m < r_n; m++) {
                    segmentTime = segmentTime + cur_representation->segmentTimeLine.snode[k].d;
                    //printf("k %d d %d\n",k, cur_representation->segmentTimeLine.snode[k].d);
                    tmp_file_duration = tmp_file_duration + cur_representation->segmentTimeLine.snode[k].d;
                    if(replace_type == 0)
                    {
                        sprintf(number_string, "%lld", segmentTime);
                    }
                    else if(replace_type == 1)
                    {
                        sprintf(number_string, "%d", dst_i);
                        dst_i++;
                    }
                    seg = av_mallocz(sizeof(struct segment));
                    seg->not_full_url = 1;
                    seg->url = (char *)av_mallocz(MAX_TIMELINE_SIZE);
                    memcpy(seg->url, number_string, strlen(number_string));
                    if(timescale > 0)
                        seg->duration = (int)(cur_representation->segmentTimeLine.snode[k].d*((float)AV_TIME_BASE/(float)timescale));
                    dynarray_add(&var->segments, &var->n_segments, seg);
                }
            }
        }
    }

    *in_tmp_file_duration = tmp_file_duration;
}

static void get_mpd_base_url(AVFormatContext * s, MPD_Period * cur_period, char *mpd_base_url, char *file_name_tmp)
{
    if (cur_period->base_url && strlen(cur_period->base_url)) {
        sprintf(mpd_base_url, "%s%s", file_name_tmp, cur_period->base_url);
    } else {
        sprintf(mpd_base_url, "%s", file_name_tmp);
    }
    if(strlen(mpd_base_url) < 2)
    {
        char *p_end = s->filename + strlen(s->filename) - 1;
        while(*p_end != '/')
            p_end--;
        memcpy(mpd_base_url, s->filename, (p_end-s->filename+1));
    }
    if(strstr(mpd_base_url,"./") ) {
        char *p_end = s->filename + strlen(s->filename) - 1;
        while(*p_end != '/')
            p_end--;
        char tmp[4096] = {0};
        memcpy(tmp, s->filename, (p_end-s->filename+1));
        strreplace(mpd_base_url, "./", tmp, mpd_base_url);
    } else if(strncmp(mpd_base_url, "http", 4) != 0 ) {
        char *p_end = s->filename + strlen(s->filename) - 1;
        while(*p_end != '/')
            p_end--;
        char tmp[4096] = {0};
        memcpy(tmp, s->filename, (p_end-s->filename+1));
        memcpy(tmp + strlen(tmp), mpd_base_url, strlen(mpd_base_url));
        memcpy(mpd_base_url, tmp, strlen(tmp));
    }
}

static int parse_manifest(AVFormatContext * s, MPDManifest * p_manifest)
{
    dash_varinat * var = NULL;
    int ret = 0;
    int i = 0, j = 0, k = 0, adaptationset_j = 0;
    float file_duration, seg_duration = 0.0;
    long tmp_file_duration;
    int start_number = 0;
    int seg_num = 0;
    long long segmentTime = 0;
    DASHContext * c = (DASHContext*)s->priv_data;
    MPD_Period * cur_period = NULL;
    MPD_AdaptationSet * cur_adaptationset = NULL;
    MPD_Representation * cur_representation = NULL;
    MPD_SegmentURL * cur_media_url = NULL;
    struct segment * seg = NULL;
    char * file_pos = NULL;
    char mpd_base_url[MAX_URL_SIZE] = {0};
    char adaptationSet_base_url[MAX_URL_SIZE] = {0};
    char number_format[MAX_TIMELINE_SIZE] = {0};
    char number_string[MAX_TIMELINE_SIZE] = {0};
    char file_name[MAX_URL_SIZE] = {0};
    char file_name_tmp[MAX_URL_SIZE] = {0};
    //char strreplace_repchar[64] = {0};
    char strreplace_dstchar[64] = {0};
    char cur_codecs[32] = {0};

    DASH_DEBUG("[%s] line[%d]!\n", __FUNCTION__, __LINE__);
    /*****set file_path***********************/
    memset(mpd_base_url, 0, sizeof(mpd_base_url));

    if (p_manifest->base_url && strlen(p_manifest->base_url)) {
        strcpy(file_name_tmp, p_manifest->base_url);
    }

    /*****set file_duration***********************/
    file_duration = 0;
    s->duration = 0;

    if ((p_manifest->maxSubsegmentDuration) > 0 || (p_manifest->period_duration) > 0) {
        if ((p_manifest->maxSubsegmentDuration) > 0) {
            file_duration = p_manifest->maxSubsegmentDuration;
            s->duration = (int64_t)p_manifest->maxSubsegmentDuration * (int64_t)AV_TIME_BASE;
            DASH_DEBUG("%s %d s->duration=%lld\n", __func__, __LINE__, s->duration);
        } else {
            file_duration = p_manifest->period_duration;
            s->duration = (int64_t)p_manifest->period_duration * (int64_t)AV_TIME_BASE;
            DASH_DEBUG("%s %d s->duration=%lld\n", __func__, __LINE__, s->duration);
        }
    }

    /*******************************************/
    g_dash_playmode = p_manifest->g_dash_playmode; //0 is not dash , 1 is vod, 2 is live

    if (g_dash_playmode == 2 && ((p_manifest->maxSubsegmentDuration) > 0 || (p_manifest->period_duration) > 0))
    {
        //actually is vod, but profiles="urn:mpeg:dash:profile:isoff-live:2011"
        g_dash_playmode = 1;
    }
    DASH_DEBUG("[%s] get g_dash_playmode %d, p_manifest->nb_period %d!\n", __func__, g_dash_playmode, p_manifest->nb_period);
    for (i = 0; i < p_manifest->nb_period; i++) {
        cur_period = p_manifest->period[i];

        if (cur_period == NULL) {
            return AVERROR(ENOMEM);
        }

        get_mpd_base_url(s, cur_period, mpd_base_url, file_name_tmp);
        DASH_DEBUG("[%s] zx mpd_base_url %s cur_period->nb_adaptationset %d, i:%d\n",
            __FUNCTION__, mpd_base_url, cur_period->nb_adaptationset, i);

        if (i > 0) {
            DASH_DEBUG("[%s] More than one period! dont support!\n", __FUNCTION__);
        }

        for (adaptationset_j = 0; adaptationset_j < cur_period->nb_adaptationset; adaptationset_j++) {
            /*****set cur_adaptationset***********************/
            cur_adaptationset = NULL;
            cur_adaptationset = cur_period->adaptationset[adaptationset_j];

            if (cur_adaptationset == NULL) {
                return AVERROR(ENOMEM);
            }

            memset(adaptationSet_base_url, 0, sizeof(adaptationSet_base_url));

            if (cur_adaptationset->base_url && strlen(cur_adaptationset->base_url)) {
                strcpy(adaptationSet_base_url, cur_adaptationset->base_url);
            }
            if(cur_adaptationset->base_url)
                DASH_DEBUG("cur_adaptationset->base_url %s\n", cur_adaptationset->base_url);
            /*****************************************************/
          DASH_DEBUG("[%s %d] get g_dash_playmode %d, adaptationset_j:%d, nb_representation:%d\n", __FUNCTION__, __LINE__,
            g_dash_playmode, adaptationset_j, cur_adaptationset->nb_representation);
          for (j = 0; j < cur_adaptationset->nb_representation; j++)
          {
            if (cur_adaptationset->codecs && strlen(cur_adaptationset->codecs)) {
                strcpy(cur_codecs, cur_adaptationset->codecs);
            } else if (cur_adaptationset->representation[j]->codecs && strlen(cur_adaptationset->representation[j]->codecs)) {
                strcpy(cur_codecs, cur_adaptationset->representation[j]->codecs);
            }
            cur_representation = cur_adaptationset->representation[j];
            if (cur_representation == NULL) {
                continue;
            }
            /*****************************************************/
            if ( strstr(cur_codecs, "mp4a" ) || strstr(cur_codecs, "ac-3")
               || strstr(cur_codecs, "hev")    || strstr(cur_codecs, "avc") 
               || strstr(cur_codecs, "vp9")    || strstr(cur_codecs, "vorbis"))
            {
                if (g_dash_playmode == 2 || g_dash_playmode == 1) {
                    var = new_dash_varinat(c, cur_representation->bandwidth, cur_representation->base_url, NULL);

                    if (!var) {
                        ret = AVERROR(ENOMEM);
                        return ret;
                    }

                    if(cur_adaptationset->lang && strlen(cur_adaptationset->lang) > 0)
                        strcpy((char *)((uintptr_t)(var->lang)), cur_adaptationset->lang);

                    memset(var->representation_id, 0, 16);
                    if (cur_representation->id && strlen(cur_representation->id))
                        memcpy(var->representation_id, cur_representation->id, strlen(cur_representation->id));

                    if(cur_representation->init_url)
                        DASH_DEBUG("%s %d cur_representation->init_url=%s\n", __func__, __LINE__, cur_representation->init_url);

                    /*****set live init_url***********************/
                    if (cur_representation->init_url && strlen(cur_representation->init_url)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_representation->init_url);
                    } else if (cur_adaptationset->init_url && strlen(cur_adaptationset->init_url)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_adaptationset->init_url);
                    }

                    //DASH_DEBUG("cur_codecs %s, file_name %s\n", cur_codecs, file_name);
                    file_pos = strstr(file_name, "$Bandwidth$");

                    if (file_pos) {
                        sprintf(strreplace_dstchar, "%d", cur_representation->bandwidth);
                        strreplace(file_name, "$Bandwidth$", strreplace_dstchar, file_name);
                    };

                    file_pos = strstr(file_name, "$RepresentationID$");

                    while (file_pos) {
                        if (cur_representation->id && strlen(cur_representation->id)) {
                            strreplace(file_name, "$RepresentationID$", cur_representation->id, file_name);
                        }

                        file_pos = strstr(file_name, "$RepresentationID$");
                    }

                    seg = av_malloc(sizeof(struct segment));
                    memset(seg, 0, sizeof(struct segment));
                    seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                    if(strlen(cur_representation->base_url) > 2)
                    {
                        if(strncmp(cur_representation->base_url, "http", 4) == 0)
                        {
                            //only for youtube
                            char *tmp_str = cur_representation->base_url;
                            delete_amp_from_urls(&tmp_str);
                            sprintf(seg->url, "%s", cur_representation->base_url);
                        }
                        else
                            sprintf(seg->url, "%s%s%s", mpd_base_url, cur_representation->base_url, file_name);
                    }
                    else
                    {
                        sprintf(seg->url, "%s%s%s", mpd_base_url, adaptationSet_base_url, file_name);
                    }
                    DASH_DEBUG("[%s %d] seg->url:%s\n", __FUNCTION__, __LINE__, seg->url);
                    dynarray_add(&var->segments, &var->n_segments, seg);

                    /*****************************************************/
                    /***********************set live media_url******************************/
                    if (cur_representation->media && strlen(cur_representation->media)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_representation->media);
                    } else if (cur_adaptationset->media && strlen(cur_adaptationset->media)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_adaptationset->media);
                    }

                    file_pos = strstr(file_name, "$Bandwidth$");

                    if (file_pos) {
                        sprintf(strreplace_dstchar, "%d", cur_representation->bandwidth);
                        strreplace(file_name, "$Bandwidth$", strreplace_dstchar, file_name);
                    }

                    file_pos = strstr(file_name, "$RepresentationID$");

                    while (file_pos) {
                        if (cur_representation->id && strlen(cur_representation->id)) {
                            strreplace(file_name, "$RepresentationID$", cur_representation->id, file_name);
                        }

                        file_pos = strstr(file_name, "$RepresentationID$");
                    }

                    file_pos = strstr(file_name, "$Time$");
                    segmentTime = 0;
                    tmp_file_duration = 0;

                    if (file_pos) {
                        if (cur_representation->segmentTimeLine.len > 0) {
                            DASH_DEBUG("%s %d g_dash_playmode %d get_seg_url_by_timeline file_name %s\n",__func__, __LINE__, g_dash_playmode, file_name);
                            if(g_dash_playmode == 2)
                                live_get_seg_url_by_timeline(cur_adaptationset, cur_representation, file_name,
                                    mpd_base_url, adaptationSet_base_url, var, file_duration);
                            else {
                                get_seg_url_by_timeline(cur_adaptationset, cur_representation, file_name,
                                    mpd_base_url, adaptationSet_base_url, var, file_duration, &tmp_file_duration);

                                if (fabs(file_duration)<0.01f && tmp_file_duration > 0) {
                                    if (cur_representation->timescale > 0) {
                                        tmp_file_duration = tmp_file_duration / cur_representation->timescale;
                                    } else if (cur_adaptationset->timescale > 0) {
                                        tmp_file_duration = tmp_file_duration / cur_adaptationset->timescale;
                                    }

                                    file_duration = tmp_file_duration;
                                    DASH_DEBUG("%s %d file_duration=%d\n", __func__, __LINE__, (int)file_duration);
                                    s->duration = (int64_t)tmp_file_duration * (int64_t)AV_TIME_BASE;
                                }
                            }
                        }
                    } else {
                        file_pos = strstr(file_name, "$Number");

                        if (file_pos) {
                            memset(number_format, 0, sizeof(number_format));
                            sscanf(file_pos + 7, "%[^$]", number_format);

                            if (cur_representation->startNumber >= 0) {
                                start_number = cur_representation->startNumber;
                            } else if (cur_adaptationset->startNumber >= 0) {
                                start_number = cur_adaptationset->startNumber;
                            } else
                                start_number = -1;//invalid

                            if (cur_representation->duration) {
                                if (cur_representation->timescale) {
                                    seg_duration = cur_representation->duration / cur_representation->timescale;
                                } else {
                                    seg_duration = cur_representation->duration;
                                }
                            } else if (cur_adaptationset->duration) {
                                if (cur_adaptationset->timescale) {
                                    seg_duration = cur_adaptationset->duration / cur_adaptationset->timescale;
                                } else {
                                    seg_duration = cur_adaptationset->duration;
                                }
                            }

                            DASH_DEBUG("%s %d, start_number %d, file_duration %d,  seg_duration %d\n",
                                __func__, __LINE__, start_number, (int)(file_duration*1000), (int)(seg_duration*1000));

                            DASH_DEBUG("%s %d, g_dash_playmode %d, p_manifest->maxSegmentDuration %lu, p_manifest->minBufferTime %lu, p_manifest->availabilityStartTime %lld\n",
                                __func__, __LINE__, g_dash_playmode, p_manifest->maxSegmentDuration, p_manifest->minBufferTime, p_manifest->availabilityStartTime);

                            if ((file_duration > 0) && (seg_duration > 0)) {
                                seg_num = ceil(file_duration / seg_duration);
                                DASH_DEBUG("%s %d start_number %d, seg_num %d\n", __func__, __LINE__, start_number, seg_num);

                                int is_full_url = 1;
                                if(seg_num > 30)
                                {
                                    is_full_url = 0;
                                    memset(var->timeline_base_url, 0, MAX_URL_SIZE);
                                    sprintf(var->timeline_base_url, "%s%s", mpd_base_url,  adaptationSet_base_url);
                                }

                                for (k = start_number; k < start_number + seg_num; k++) {
                                    memset(strreplace_dstchar, 0, 64);
                                    memset(number_string, 0, 32);
                                    if (strlen(number_format)) {
                                        sprintf(strreplace_dstchar, number_format, k);
                                        sprintf(number_string, "$Number%s$", number_format);
                                    } else {
                                        sprintf(strreplace_dstchar, "%d", k);
                                        sprintf(number_string, "$Number$");
                                    }

                                    strreplace(file_name, number_string, strreplace_dstchar, file_name_tmp);
                                    seg = av_mallocz(sizeof(struct segment));
                                    if(is_full_url)
                                    {
                                        seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                                        sprintf(seg->url, "%s%s%s", mpd_base_url,  adaptationSet_base_url, file_name_tmp);
                                    }
                                    else
                                    {
                                        if(strlen(file_name_tmp) >= MAX_NUMBER_SEGMENT_SIZE)
                                            DASH_ERROR("%s %d MAX_NUMBER_SEGMENT_SIZE %d too short, strlen(file_name_tmp) %d!!!\n",
                                                __func__, __LINE__, MAX_NUMBER_SEGMENT_SIZE, strlen(file_name_tmp));
                                        seg->not_full_url = 2;
                                        seg->url = (char *)av_mallocz(MAX_NUMBER_SEGMENT_SIZE);
                                        snprintf(seg->url, "%s", file_name_tmp, (MAX_NUMBER_SEGMENT_SIZE-1));
                                    }
                                    seg->duration = (int)(seg_duration*AV_TIME_BASE);
                                    //DASH_DEBUG("%s %d seg->url=%s, seg->duration %d\n", __func__, __LINE__, seg->url,seg->duration);
                                    dynarray_add(&var->segments, &var->n_segments, seg);
                                }
                            }
                            else if(g_dash_playmode == 2
                                && p_manifest->availabilityStartTime >= 0
                                && (p_manifest->maxSegmentDuration > 0
                                    || p_manifest->minBufferTime > 0 ))//real live, no duration
                            {
                                uint64_t sys_time = get_current_time_in_sec();
                                DASH_DEBUG("sys_time %llu, p_manifest->publishTime %lld, timeShiftBufferDepth %lld, cur_period->start_time %lld\n",
                                    sys_time, p_manifest->publishTime, p_manifest->timeShiftBufferDepth, cur_period->start_time);
                                int int32_duration = (int)cur_adaptationset->duration;
                                int int32_timescale = (int)cur_adaptationset->timescale;
                                if(p_manifest->publishTime )//publishTime is 2014 added, can get local utctime
                                {
                                    if(!p_manifest->availabilityStartTime)
                                    {
                                        if(p_manifest->minBufferTime > 0)
                                        {
                                            DASH_DEBUG("var->start_seq_no %d int32_duration %d, int32_timescale %d, minBufferTime %d\n",
                                                var->start_seq_no, int32_duration, int32_timescale, p_manifest->minBufferTime);
                                            //num = pls->first_seq_no + (((c->publish_time + pls->fragment_duration) - c->suggested_presentation_delay) * pls->fragment_timescale) / pls->fragment_duration - c->min_buffer_time;
                                            if(int32_timescale==0 || int32_duration==0)
                                                var->live_start_number = (int)( (p_manifest->publishTime - cur_period->start_time )/(uint64_t)p_manifest->minBufferTime )  - p_manifest->minBufferTime;
                                            else
                                                var->live_start_number = (int)( ((p_manifest->publishTime - cur_period->start_time )*(uint64_t)int32_timescale)/(uint64_t)int32_duration )  - p_manifest->minBufferTime;
                                        }
                                        else
                                        {
                                            //num = pls->first_seq_no + (((c->publish_time - c->time_shift_buffer_depth + pls->fragment_duration) - c->suggested_presentation_delay) * pls->fragment_timescale) / pls->fragment_duration;
                                            DASH_DEBUG("timeShiftBufferDepth use, var->start_seq_no %d int32_duration %d, int32_timescale %d, minBufferTime %d\n",
                                                var->start_seq_no, int32_duration, int32_timescale, p_manifest->minBufferTime);
                                            var->live_start_number = (int)( ((p_manifest->publishTime - cur_period->start_time - p_manifest->timeShiftBufferDepth)*(uint64_t)int32_timescale)/(uint64_t)int32_duration );
                                        }
                                    }
                                    else
                                    {
                                        //num = pls->first_seq_no + (((get_current_time_in_sec() - c->availability_start_time) - c->suggested_presentation_delay) * pls->fragment_timescale) / pls->fragment_duration;
                                        DASH_ERROR("%s %d p_manifest->publishTime %lld, availabilityStartTime %lld, sys_time %lld, suggested_presentation_delay %lld\n",
                                                        __func__, __LINE__, p_manifest->publishTime, p_manifest->availabilityStartTime,
                                                        sys_time, p_manifest->suggested_presentation_delay);
                                        if( (int32_timescale==0 || int32_duration==0) && p_manifest->minBufferTime > 0)
                                            var->live_start_number = (int) (sys_time - cur_period->start_time - p_manifest->availabilityStartTime - p_manifest->suggested_presentation_delay)/(uint64_t)p_manifest->minBufferTime ;
                                        else
                                            var->live_start_number = (int)( ((sys_time - cur_period->start_time - p_manifest->availabilityStartTime - p_manifest->suggested_presentation_delay)*(uint64_t)int32_timescale)/(uint64_t)int32_duration );
                                    }

                                    DASH_DEBUG("var->live_start_number %d, start_number %d\n",
                                        var->live_start_number, start_number);

                                    if(var->live_start_number < 0)
                                        var->live_start_number = 0;

                                    if(start_number > 0)
                                         var->live_start_number += start_number;
                                    strreplace(file_name, "$Number$", "%d", file_name_tmp);
                                    seg = av_mallocz(sizeof(struct segment));
                                    memset(seg, 0, sizeof(struct segment));
                                    seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                                    sprintf(seg->url, "%s%s%s", mpd_base_url,  adaptationSet_base_url, file_name_tmp);
                                    //DASH_DEBUG("zx seg->url %s, var->live_start_number %d\n",seg->url, var->live_start_number);
                                    dynarray_add(&var->segments, &var->n_segments, seg);
                                }
                            }
                            else
                            {
                                DASH_DEBUG("%s %d cur_adaptationset->segmentTimeLine.len %d, cur_representation %d\n",
                                    __func__, __LINE__, cur_adaptationset->segmentTimeLine.len, cur_representation->segmentTimeLine.len);
                                if (cur_adaptationset->segmentTimeLine.len || cur_representation->segmentTimeLine.len > 0) {
                                    get_seg_url_by_timeline(cur_adaptationset, cur_representation, file_name,
                                        mpd_base_url, adaptationSet_base_url, var, file_duration, &tmp_file_duration);
                                }
                            }


                        } else {
                            if (cur_representation->segment_list.media_url_list) {
                                cur_media_url = cur_representation->segment_list.media_url_list;

                                for (k = 0; k < cur_representation->segment_list.len; k++) {
                                    if (cur_media_url) {
                                        memset(file_name, 0, sizeof(file_name));

                                        if (strlen(cur_media_url->media_url)) {
                                            strcpy(file_name, cur_media_url->media_url);
                                            seg = av_malloc(sizeof(struct segment));
                                            memset(seg, 0, sizeof(struct segment));
                                            seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                                            sprintf(seg->url, "%s%s%s", mpd_base_url,  adaptationSet_base_url, file_name);
                                            //DASH_DEBUG("%s %d seg->url=%s,  %f\n", __func__, __LINE__, seg->url,cur_representation->duration);
                                            dynarray_add(&var->segments, &var->n_segments, seg);
                                        }

                                        cur_media_url = cur_media_url->next_media_url;
                                    }
                                }
                            }
                            else
                            {
                                seg = av_malloc(sizeof(struct segment));
                                memset(seg, 0, sizeof(struct segment));
                                seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                                sprintf(seg->url, "%s%s%s", mpd_base_url,  adaptationSet_base_url, file_name);
                                //printf("%s,%s,%s\n", mpd_base_url,  adaptationSet_base_url, file_name);
                                seg->duration = seg_duration;
                                //DASH_DEBUG("%s %d seg->url=%s,  %f\n", __func__, __LINE__, seg->url,cur_representation->duration);
                                dynarray_add(&var->segments, &var->n_segments, seg);
                            }
                        }
                    }

                    var->bandwidth = cur_representation->bandwidth;

                    if (strstr(cur_codecs, "mp4a")) {
                        var->codec_type = AVMEDIA_TYPE_AUDIO;
                    } else if (strstr(cur_codecs, "ac-3")) {
                        var->codec_type = AVMEDIA_TYPE_AUDIO;
                    } else if (strstr(cur_codecs, "vorbis")) {
                        var->codec_type = AVMEDIA_TYPE_AUDIO;
                    } else if (strstr(cur_codecs, "avc")) {
                        var->codec_type = AVMEDIA_TYPE_VIDEO;
                    } else if (strstr(cur_codecs, "hev")) {
                        var->codec_type = AVMEDIA_TYPE_VIDEO;
                    } else if (strstr(cur_codecs, "vp9")) {
                        var->codec_type = AVMEDIA_TYPE_VIDEO;
                    }
                    if(var->codec_type == AVMEDIA_TYPE_VIDEO && cur_representation->height > 0)
                        var->resolution = cur_representation->height;
                    DASH_DEBUG("var %d resolution 11 %d var->bandwidth %d\n",
                        c->n_dash_varinats-1, var->resolution, var->bandwidth);

                    //DASH_DEBUG("[%s] line[%d]!,var->n_segments[%d]\n", __FUNCTION__, __LINE__, var->n_segments);
                    /*****************************************************/
                } else if (g_dash_playmode == 3) {
                    var = new_dash_varinat(c, cur_representation->bandwidth, cur_representation->base_url, NULL);

                    if (!var) {
                        ret = AVERROR(ENOMEM);
                        return ret;
                    }

                    /*****set main init_url***********************/
                    memset(file_name, 0, sizeof(file_name));
                    //if(cur_representation->init_url)
                    //    DASH_DEBUG("%s %d cur_representation->init_url=%s\n", __func__, __LINE__, cur_representation->init_url);

                    if(cur_adaptationset->lang && strlen(cur_adaptationset->lang) > 0)
                        strcpy((char *)((uintptr_t)(var->lang)), cur_adaptationset->lang);

                    if (cur_representation->init_url && strlen(cur_representation->init_url)) {
                        strcpy(file_name, cur_representation->init_url);
                    } else if (cur_adaptationset->init_url && strlen(cur_adaptationset->init_url)) {
                        strcpy(file_name, cur_adaptationset->init_url);
                    }

                    seg = av_malloc(sizeof(struct segment));
                    memset(seg, 0, sizeof(struct segment));
                    seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                    sprintf(seg->url, "%s%s%s", mpd_base_url,  adaptationSet_base_url, file_name);
                    //DASH_DEBUG("%s %d seg->url=%s,  %f\n", __func__, __LINE__, seg->url,cur_representation->duration);
                    dynarray_add(&var->segments, &var->n_segments, seg);

                    /*****************************************************/

                    /*****set main media url***********************/
                    if (cur_representation->segment_list.media_url_list) {
                        cur_media_url = cur_representation->segment_list.media_url_list;

                        for (k = 0; k < cur_representation->segment_list.len; k++) {
                            if (cur_media_url) {
                                memset(file_name, 0, sizeof(file_name));

                                if (strlen(cur_media_url->media_url)) {
                                    strcpy(file_name, cur_media_url->media_url);
                                    seg = av_malloc(sizeof(struct segment));
                                    memset(seg, 0, sizeof(struct segment));
                                    seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                                    sprintf(seg->url, "%s%s%s", mpd_base_url,  adaptationSet_base_url, file_name);
                                    //DASH_DEBUG("%s %d seg->url=%s,  %f\n", __func__, __LINE__, seg->url,cur_representation->duration);
                                    dynarray_add(&var->segments, &var->n_segments, seg);
                                }

                                cur_media_url = cur_media_url->next_media_url;
                            }
                        }
                    } else if (cur_adaptationset->segment_list.media_url_list) {
                        cur_media_url = cur_adaptationset->segment_list.media_url_list;

                        for (k = 0; k < cur_adaptationset->segment_list.len; k++) {
                            if (cur_media_url) {
                                memset(file_name, 0, sizeof(file_name));

                                if (strlen(cur_media_url->media_url)) {
                                    strcpy(file_name, cur_media_url->media_url);
                                    seg = av_malloc(sizeof(struct segment));
                                    memset(seg, 0, sizeof(struct segment));
                                    seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                                    sprintf(seg->url, "%s%s%s", mpd_base_url,  adaptationSet_base_url, file_name);
                                    //DASH_DEBUG("%s %d seg->url=%s,  %f\n", __func__, __LINE__, seg->url,cur_representation->duration);
                                }

                                cur_media_url = cur_media_url->next_media_url;
                            }
                        }
                    }

                    /********************************************/
                    var->bandwidth = cur_representation->bandwidth;

                    if (strstr(cur_codecs, "mp4a")) {
                        var->codec_type = AVMEDIA_TYPE_AUDIO;
                    } else if (strstr(cur_codecs, "ac-3")) {
                        var->codec_type = AVMEDIA_TYPE_AUDIO;
                    } else if (strstr(cur_codecs, "vorbis")) {
                        var->codec_type = AVMEDIA_TYPE_AUDIO;
                    } else if (strstr(cur_codecs, "avc")) {
                        var->codec_type = AVMEDIA_TYPE_VIDEO;
                    } else if (strstr(cur_codecs, "hev")) {
                        var->codec_type = AVMEDIA_TYPE_VIDEO;
                    } else if (strstr(cur_codecs, "vp9")) {
                        var->codec_type = AVMEDIA_TYPE_VIDEO;
                    }

                }

                //add protection info
                if(cur_adaptationset->contentProtection.mspr && strlen(cur_adaptationset->contentProtection.mspr) > 5 )
                {
                    var->p_content_protection = (char *)av_mallocz(strlen(cur_adaptationset->contentProtection.mspr) + 1);
                    memcpy(var->p_content_protection, cur_adaptationset->contentProtection.mspr, strlen(cur_adaptationset->contentProtection.mspr));
                }
            } else {
                continue;
            }

          }
        }
    }

    char *p_first_video = NULL;
    c->is_support_change_next = 0;
    for (i = 0; i < c->n_dash_varinats; i++)
    {
        if (c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if(p_first_video == NULL)
            {
                p_first_video = c->dash_varinats[i]->segments[0]->url;
            }
            else
            {
                char *p_cur_video = c->dash_varinats[i]->segments[0]->url;
                if(strcmp(p_first_video, p_cur_video) == 0)
                    c->is_support_change_next = 0;//video init.mp4 has more than 1 tracks
                else
                    c->is_support_change_next = 1;
            }
        }
    }


    DASH_DEBUG("%s c->n_dash_varinats %d c->is_support_change_next %d end end\n",
        __func__, c->n_dash_varinats, c->is_support_change_next);
    return ret;
}

static int parse_manifest_update_var(AVFormatContext * s, MPDManifest * p_manifest)
{
    //current, only for live timeline update
    DASHContext * c = s->priv_data;
    int i, j, period_i=0, adaptationset_j=0;
    MPD_Period * cur_period = NULL;
    MPD_AdaptationSet * cur_adaptationset = NULL;
    MPD_Representation * cur_representation = NULL;
    struct segment * seg = NULL;
    int64_t audio_last_opened_timeline = 0;
    int64_t video_last_opened_timeline = 0;
    char adaptationSet_base_url[MAX_URL_SIZE] = {0};
    char mpd_base_url[MAX_URL_SIZE] = {0};
    char cur_codecs[32] = {0};
    char file_name[MAX_URL_SIZE] = {0};
    char file_name_tmp[MAX_URL_SIZE] = {0};
    //char strreplace_repchar[64] = {0};
    char strreplace_dstchar[64] = {0};
    char * file_pos = NULL;
    dash_varinat * var = NULL;
    int contiue_flag = 0;

    if (p_manifest->base_url && strlen(p_manifest->base_url)) {
        strcpy(file_name_tmp, p_manifest->base_url);
    }
    for (i = 0; i < c->n_dash_varinats; i++) {
        dash_varinat * var = c->dash_varinats[i];
        free_dash_segment_list(var);
        //DASH_DEBUG("%s zxtmp var last_opened_timeline %lld\n", __func__, var->last_opened_timeline);
        if(var->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if(video_last_opened_timeline < var->last_opened_timeline)
                video_last_opened_timeline = var->last_opened_timeline;
        }
        else if(var->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            if(audio_last_opened_timeline < var->last_opened_timeline)
                audio_last_opened_timeline = var->last_opened_timeline;
        }
    }
    DASH_DEBUG("%s v %lld, a %lld\n",__func__, video_last_opened_timeline, audio_last_opened_timeline);


    for (period_i = 0; period_i < p_manifest->nb_period; period_i++) {
        cur_period = p_manifest->period[period_i];
        contiue_flag = 0;

        if (cur_period == NULL) {
            DASH_ERROR("%s %d error!\n",__func__, __LINE__);
            return AVERROR(ENOMEM);
        }
        get_mpd_base_url(s, cur_period, mpd_base_url, file_name_tmp);
        //DASH_DEBUG("[%s] zx mpd_base_url %s! cur_period->nb_adaptationset %d\n",
        //            __FUNCTION__, mpd_base_url, cur_period->nb_adaptationset);

        for (adaptationset_j = 0; adaptationset_j < cur_period->nb_adaptationset; adaptationset_j++) {
            /*****set cur_adaptationset***********************/
            cur_adaptationset = NULL;
            cur_adaptationset = cur_period->adaptationset[adaptationset_j];

            if (cur_adaptationset == NULL) {
                DASH_ERROR("%s %d error!\n",__func__, __LINE__);
                return AVERROR(ENOMEM);
            }
            memset(adaptationSet_base_url, 0, sizeof(adaptationSet_base_url));

            if (cur_adaptationset->base_url && strlen(cur_adaptationset->base_url)) {
                strcpy(adaptationSet_base_url, cur_adaptationset->base_url);
            }
            //if(cur_adaptationset->base_url)
            //    DASH_DEBUG("cur_adaptationset->base_url %s\n", cur_adaptationset->base_url);

            for (j = 0; j < cur_adaptationset->nb_representation; j++){
                //zx11
                if (cur_adaptationset->codecs && strlen(cur_adaptationset->codecs)) {
                    strcpy(cur_codecs, cur_adaptationset->codecs);
                } else if (cur_adaptationset->representation[j]->codecs && strlen(cur_adaptationset->representation[j]->codecs)) {
                    strcpy(cur_codecs, cur_adaptationset->representation[j]->codecs);
                }
                cur_representation = cur_adaptationset->representation[j];
                if (cur_representation == NULL) {
                    contiue_flag = 1;
                    continue;
                }
                //DASH_DEBUG("%s %d cur_codecs %s!\n",__func__, __LINE__, cur_codecs);
                if ( strstr(cur_codecs, "mp4a" ) || strstr(cur_codecs, "ac-3")  ||
                    strstr(cur_codecs, "hev")    || strstr(cur_codecs, "avc") ) {
                    var = NULL;
                    int tmpi = 0;
                    for (tmpi = 0; tmpi < c->n_dash_varinats; tmpi++) {
                        dash_varinat * tmp_var = c->dash_varinats[tmpi];
                        //DASH_DEBUG("tmp_var->bandwidth %d %s, cur_representation %d %s\n",
                        //    tmp_var->bandwidth, tmp_var->representation_id, cur_representation->bandwidth, cur_representation->id);
                        if(tmp_var->bandwidth == cur_representation->bandwidth
                            && cur_representation->id && strcmp(tmp_var->representation_id, cur_representation->id) == 0){
                            var = tmp_var;
                            break;
                        }
                    }
                    if(var == NULL) {
                        DASH_ERROR("%s update failed! var == NULL\n", __func__);
                        continue;
                    }
                    //var->live_timeline_num = 0;

                    //if(cur_representation->init_url)//zx11
                    //    DASH_DEBUG("%s %d cur_representation->init_url=%s\n", __func__, __LINE__, cur_representation->init_url);
                    if (cur_representation->init_url && strlen(cur_representation->init_url)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_representation->init_url);
                    } else if (cur_adaptationset->init_url && strlen(cur_adaptationset->init_url)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_adaptationset->init_url);
                    }

                    //DASH_DEBUG("cur_codecs %s, file_name %s\n", cur_codecs, file_name);
                    file_pos = strstr(file_name, "$Bandwidth$");

                    if (file_pos) {
                        sprintf(strreplace_dstchar, "%d", cur_representation->bandwidth);
                        strreplace(file_name, "$Bandwidth$", strreplace_dstchar, file_name);
                    };

                    file_pos = strstr(file_name, "$RepresentationID$");

                    while (file_pos) {
                        if (cur_representation->id && strlen(cur_representation->id)) {
                            strreplace(file_name, "$RepresentationID$", cur_representation->id, file_name);
                        }

                        file_pos = strstr(file_name, "$RepresentationID$");
                    }

                    seg = av_malloc(sizeof(struct segment));
                    memset(seg, 0, sizeof(struct segment));
                    seg->url = (char *)av_mallocz(MAX_URL_SIZE);
                    if(strlen(cur_representation->base_url) > 2)
                        sprintf(seg->url, "%s%s%s", mpd_base_url, cur_representation->base_url, file_name);
                    else
                        sprintf(seg->url, "%s%s%s", mpd_base_url, adaptationSet_base_url, file_name);
                    //DASH_DEBUG("%s %d seg->url=%s\n", __func__, __LINE__, seg->url);
                    dynarray_add(&var->segments, &var->n_segments, seg);

                    /*****************************************************/
                    /***********************set live media_url******************************/
                    if (cur_representation->media && strlen(cur_representation->media)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_representation->media);
                    } else if (cur_adaptationset->media && strlen(cur_adaptationset->media)) {
                        memset(file_name, 0, sizeof(file_name));
                        strcpy(file_name, cur_adaptationset->media);
                    }

                    file_pos = strstr(file_name, "$Bandwidth$");

                    if (file_pos) {
                        sprintf(strreplace_dstchar, "%d", cur_representation->bandwidth);
                        strreplace(file_name, "$Bandwidth$", strreplace_dstchar, file_name);
                    }

                    file_pos = strstr(file_name, "$RepresentationID$");

                    while (file_pos) {
                        if (cur_representation->id && strlen(cur_representation->id)) {
                            strreplace(file_name, "$RepresentationID$", cur_representation->id, file_name);
                        }

                        file_pos = strstr(file_name, "$RepresentationID$");
                    }

                    file_pos = strstr(file_name, "$Time$");

                    if (file_pos) {
                        if (cur_representation->segmentTimeLine.len > 0 && g_dash_playmode == 2) {
                            //DASH_DEBUG("%s %d g_dash_playmode %d get_seg_url_by_timeline file_name %s\n",__func__, __LINE__, g_dash_playmode, file_name);
                                live_get_seg_url_by_timeline(cur_adaptationset, cur_representation, file_name,
                                    mpd_base_url, adaptationSet_base_url, var, 0);
                        }
                    }
                }
            }
            if(contiue_flag == 1)
                continue;
        }
    }

    int64_t cur_var_last_opened_timeline = 0;
    for (i = 0; i < c->n_dash_varinats; i++) {
        dash_varinat * tmp_var = c->dash_varinats[i];
        if(tmp_var->codec_type == AVMEDIA_TYPE_VIDEO)
            cur_var_last_opened_timeline = video_last_opened_timeline;
        else if(tmp_var->codec_type == AVMEDIA_TYPE_AUDIO)
            cur_var_last_opened_timeline = audio_last_opened_timeline;
        else
            continue;

        tmp_var->start_seq_no = 0;
        for(tmp_var->cur_seq_no=1; tmp_var->cur_seq_no<tmp_var->n_segments; tmp_var->cur_seq_no++) {
            if(tmp_var->segments[tmp_var->cur_seq_no]->timeline_num > cur_var_last_opened_timeline){
                DASH_DEBUG("[%s] index %d cur_seq_no[%d] timeline_num %lld, last_opened_timeline %lld\n",
                    __func__, tmp_var->index, tmp_var->cur_seq_no, tmp_var->segments[tmp_var->cur_seq_no]->timeline_num, cur_var_last_opened_timeline);
                tmp_var->live_timeline_updated = 1;
                break;
            }
        }
    }

    DASH_DEBUG("[%s] end end\n", __func__);
    return 0;
}


static int dash_get_parse_mpd(AVFormatContext *s, AVIOContext * in, MPDManifest *p_manifest, int is_update)
{
    int ret = 0;
    int64_t filesize;
    uint8_t * buf;
    int av_audio_type = 0;
    int av_video_type = 0;
    int close_in = 0;

    memset(p_manifest, 0, sizeof(MPDManifest));
    if (!in) {
        close_in = 1;
        //AVDictionary *opts = NULL;
        /* Some HLS servers don't like being sent the range header */
        //av_dict_set(&opts, "seekable", "0", 0);

        // broker prior HTTP options that should be consistent across requests
        //av_dict_set(&opts, "user-agent", c->user_agent, 0);
        //av_dict_set(&opts, "cookies", c->cookies, 0);

        DASH_DEBUG("%s ffurl_open url %s\n",__func__, s->filename);
        ret = avio_open2(&in, s->filename, AVIO_FLAG_READ,
                            NULL, NULL);
        //av_dict_free(&opts);
        if (ret < 0)
            return ret;
    }

    filesize = avio_size(in) + 1;

    //DASH_DEBUG("[%s] line[%d]!,filesize[%d] is_chunked[%d] \n", __FUNCTION__, __LINE__, filesize, in->is_chunked);
    if(g_is_http_chunked) {
        DASH_ERROR("[%s] line[%d] chunk mpd !\n", __FUNCTION__, __LINE__);
        filesize = 64*1024;
    }

    if (filesize <= 0) {
        DASH_ERROR("[%s] line[%d] filesize %d <= 0 !\n", __FUNCTION__, __LINE__, filesize);
        return -1;
    }
    buf = av_mallocz(filesize * sizeof(uint8_t));

    if (!buf) {
        return AVERROR(ENOMEM);
    }

    int len = avio_read(in, buf, filesize);
    memset(p_manifest, 0x00, sizeof(MPDManifest));
    //DASH_DEBUG("[%s] line[%d]!,sizeofmanifest[%d]\n", __FUNCTION__, __LINE__, sizeof(MPDManifest));
    DASH_DEBUG("[%s] line[%d]!,filesize[%lld] len[%d]\n", __FUNCTION__, __LINE__, filesize, len);

    if ((ret = ff_parse_mpd_manifest(buf, strlen(buf), p_manifest)) < 0) {
        DASH_ERROR("[%s] line[%d] ff_parse_mpd_manifest failed\n", __FUNCTION__, __LINE__);
        av_free(buf);
        ff_free_mpd_manifest(p_manifest);
        return ret;
    }

    //DASH_DEBUG("[%s] line[%d],v[%d], s 0x%x &manifest 0x%x\n",
    //    __FUNCTION__, __LINE__, c->n_dash_varinats, s, &manifest);
    av_free(buf);

    if(is_update == 0) {
        if ((ret = parse_manifest(s, p_manifest)) < 0) {
            return ret;
        }
    }
    else {
        if ((ret = parse_manifest_update_var(s, p_manifest)) < 0) {
            return ret;
        }
    }

    if (close_in)
        avio_close(in);
    return 0;
}


static int mont_dash_read_header(AVFormatContext * s)
{
    DASH_DEBUG("[%s] line[%d] %s!\n", __FUNCTION__, __LINE__, s->filename);
    URLContext * u = (s->flags & AVFMT_FLAG_CUSTOM_IO) ? NULL : s->pb->opaque;
    DASHContext * c = s->priv_data;
    int ret = 0, i, j, stream_offset = 0;
    int seq_count = 0;
    MPDManifest manifest;
    AVIOContext * in = s->pb;
    char * p;
    int av_audio_type = 0;
    int av_video_type = 0;

    //av_strlcpy(c->base_url, s->filename, p - s->filename + 1);
#if 0//def USE_PB_FIFO peacer del 20180411
    FILE_SEQ_T * p_seq = file_seq_get_instance();
#endif
    c->interrupt_callback = &s->interrupt_callback;
    /*// if the URL context is good, read important options we must broker later
    if (u && u->prot->priv_data_class) {
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
    if(dash_get_parse_mpd(s, in, &manifest, 0) < 0)
        goto fail;

    c->profile_isoff_ondemand = manifest.profile_isoff_ondemand;
    s->profile_isoff_ondemand = c->profile_isoff_ondemand;

    ff_free_mpd_manifest(&manifest);
    //DASH_DEBUG("[%s] line[%d],v[%d]!\n", __FUNCTION__, __LINE__, c->n_dash_varinats);

    if (c->n_dash_varinats == 0) {
        av_log(NULL, AV_LOG_WARNING, "Empty playlist\n");
        ret = AVERROR_EOF;
        goto fail;
    }

    c->cur_audio_var = -1;
    c->cur_video_var = -1;
    if (c->n_dash_varinats >= 1) {
        /* Open the demuxer for each dash_varinat */
        for (i = 0; i < c->n_dash_varinats; i++) {
            dash_varinat * v = c->dash_varinats[i];
            AVInputFormat * in_fmt = NULL;
            char bitrate_str[20];
            AVProgram * program = NULL;

            v->index = i;
            if (v->n_segments == 0) {
                continue;
            }

            if ((c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_AUDIO && av_audio_type == 1) ||
                (c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_VIDEO && av_video_type == 1)) {
                continue;
            }

            if (c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_AUDIO) {
                av_audio_type = 1;
            } else if (c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_VIDEO) {
                av_video_type = 1;
            }

            c->dash_varinats[i]->var_used_flag = 1;

            if (!(v->ctx = avformat_alloc_context())) {
                ret = AVERROR(ENOMEM);
                goto fail;
            }
            v->ctx->profile_isoff_ondemand = s->profile_isoff_ondemand;

            if(c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_AUDIO)
                c->cur_audio_var = i;
            else if(c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_VIDEO)
                c->cur_video_var = i;
            v->needed = 1;
            v->parent = s;
            /* If this is a live stream with more than 3 segments, start at the
             * third last segment. */
            v->cur_seq_no = v->start_seq_no;
#if 0

            if (!v->finished && v->n_segments > 3) {
                v->cur_seq_no = v->start_seq_no + v->n_segments - 3;
            }

#endif
            v->last_seq_no = v->cur_seq_no;
            v->read_buffer = av_malloc(INITIAL_BUFFER_SIZE);
            //AVInputFormat *in_fmt = NULL;
            dash_ffurl_close(v);
            v->pb = av_mallocz(sizeof(AVIOContext));

            if (g_dash_playmode == 1) // vod
                ffio_init_context(v->pb, v->read_buffer, INITIAL_BUFFER_SIZE, 0, v,
                                  read_data, NULL, read_seek);
            else
                ffio_init_context(v->pb, v->read_buffer, INITIAL_BUFFER_SIZE, 0, v,
                                  read_data, NULL, dash_live_read_seek);

            if(c->profile_isoff_ondemand)
                v->pb->seekable = 1;
            else
            {
                ret = dash_open_segmeng_url(c, v->segments[v->cur_seq_no - v->start_seq_no], v, v);
                //DASH_DEBUG("file size %lld\n", avio_size(v->pb));
                v->pb->seekable = -1;
            }

            int max_probe_size = (int )(avio_size(v->pb) & 0xffffffff );
            if(max_probe_size == AVERROR(EINVAL) || max_probe_size == AVERROR(ENOSYS))
            {
                if (g_dash_playmode == 2 || !c->profile_isoff_ondemand)
                {
                    DASH_DEBUG("%s %d live get init.mp4 len error!!!\n", __func__, __LINE__);
                    max_probe_size = 0;
                }
                else
                    max_probe_size = (1<<20);
            }

            DASH_DEBUG("%s av_probe_input_buffer max_probe_size %d !!!!! init.mp4 probe dont read next file!!!\n",
                __func__, max_probe_size);
            ret = av_probe_input_buffer(v->pb, &in_fmt, v->segments[v->cur_seq_no - v->start_seq_no]->url,
                                        NULL, 0, (unsigned int)max_probe_size);


            if (ret < 0) {
                /* Free the ctx - it isn't initialized properly at this point,
                 * so avformat_close_input shouldn't be called. If
                 * avformat_open_input fails below, it frees and zeros the
                 * context, so it doesn't need any special treatment like this. */
                DASH_ERROR("Error %d when loading first segment '%s'\n", ret, v->segments[v->cur_seq_no - v->start_seq_no]->url);
                avformat_free_context(v->ctx);
                v->ctx = NULL;
                goto fail;
            }

            if(v->p_content_protection)
                v->ctx->p_content_protection = v->p_content_protection;
            else
                v->ctx->p_content_protection = NULL;
            v->ctx->pb       = v->pb;
            ret = avformat_open_input(&v->ctx, v->segments[v->cur_seq_no - v->start_seq_no]->url, in_fmt, NULL);

            if (ret < 0) {
                goto fail;
            }

            //v->segments[v->cur_seq_no - v->start_seq_no]->duration = s->duration / 1000000;
            v->stream_offset = stream_offset;
            v->ctx->ctx_flags &= ~AVFMTCTX_NOHEADER;
            ret = avformat_find_stream_info(v->ctx, NULL);

            if (ret < 0) {
                goto fail;
            }

            snprintf(bitrate_str, sizeof(bitrate_str), "%d", v->bandwidth);
            /* Create new AVprogram for dash_varinat i */
            program = av_new_program(s, i);

            if (!program) {
                goto fail;
            }

            av_dict_set(&program->metadata, "dash_varinat_bitrate", bitrate_str, 0);

            /* Create new AVStreams for each stream in this dash_varinat */
            for (j = 0; j < v->ctx->nb_streams; j++) {
                AVStream * st = avformat_new_stream(s, NULL);
                AVStream * ist = v->ctx->streams[j];

                if (!st) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }

                av_program_add_stream_index(s, i, stream_offset + j);
                st->id = i;
                avcodec_parameters_copy(st->codecpar, ist->codecpar);
                avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
                //DASH_DEBUG("st->codec 0x%x, j %d, v->ctx->streams[j]->codec 0x%x\n", st->codec, j, v->ctx->streams[j]->codec);

                avcodec_copy_context(st->codec, v->ctx->streams[j]->codec);
                DASH_DEBUG("st->nb_index_entries %d, ist->nb_index_entries %d\n", st->nb_index_entries, ist->nb_index_entries);
                if (v->bandwidth)
                    av_dict_set(&st->metadata, "dash_varinat_bitrate", bitrate_str,
                                0);
                if(strlen((const char *)((uintptr_t)(v->lang))) > 0)
                {
                    av_dict_set(&st->metadata, "language", (const char *)((uintptr_t)(v->lang)), 0);
                }
                st->discard = AVDISCARD_NONE;
            }

            v->seek_timestamp  = AV_NOPTS_VALUE;
            stream_offset += v->ctx->nb_streams;
            v->finished = 1;
        }

        //add other audio tracks for switch audio
        struct dash_varinat *selected_audio_v = c->dash_varinats[c->cur_audio_var];
        for (i = 0; i < c->n_dash_varinats; i++) {
            dash_varinat * v = c->dash_varinats[i];

            if (v->n_segments == 0 || i == c->cur_audio_var) {
                continue;
            }

            if (c->dash_varinats[i]->codec_type == AVMEDIA_TYPE_AUDIO )
            {
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
                if(strlen((const char *)((uintptr_t)(v->lang))) > 0)
                {
                    av_dict_set(&st->metadata, "language", (const char *)((uintptr_t)(v->lang)), 0);
                }
                st->discard = AVDISCARD_ALL;
                v->seek_timestamp  = AV_NOPTS_VALUE;
                v->stream_offset = stream_offset;
                stream_offset += 1;
            }
        }
    }

#ifdef MT_DRM_SUPPORT
    int a_index = -1;
    int v_index = -1;
    for (i = 0; i < c->n_dash_varinats; i++) {
        dash_varinat * v = c->dash_varinats[i];

        if (v->n_segments == 0) {
            continue;
        }

        if(v->var_used_flag) {
            AVStream *st = v->ctx->streams[0];
            if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                v_index = st->eDrmIndex;
            else if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                a_index = st->eDrmIndex;
        }
    }
    DASH_DEBUG("%s v_index %d a_index %d\n",__func__, v_index, a_index);
    av_drm_set_index_info(v_index, a_index);
#endif
    DASH_DEBUG("%s %d end\n", __func__, __LINE__);
    //c->first_packet = 1;
    c->first_timestamp = AV_NOPTS_VALUE;
    c->seek_timestamp  = AV_NOPTS_VALUE;
    return 0;
fail:
    g_dash_playmode = 0;
    free_dash_varinat_list(c);
    return ret;
}

static int recheck_dash_audio_discard_flags(AVFormatContext * s, int first)
{
    DASHContext * c = s->priv_data;
    int i, changed = 0;

    /* Check if any new streams are needed */
    for (i = 0; i < c->n_dash_varinats; i++) {
        c->dash_varinats[i]->cur_needed = 0;
    }

    for (i = 0; i < s->nb_streams; i++) {
        AVStream * st = s->streams[i];
        dash_varinat * var = c->dash_varinats[s->streams[i]->id];

        if (st->discard < AVDISCARD_ALL) {
            var->cur_needed = 1;
        }
    }

    for (i = 0; i < c->n_dash_varinats; i++) {
        dash_varinat * v = c->dash_varinats[i];

        if(v->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            if (v->cur_needed && !v->needed) {
                v->needed = 1;
                changed = 1;
                //v->pb->eof_reached = 0;
                v->cur_seq_no = c->cur_audio_seq_no;
                c->cur_audio_var = i;
                DASH_DEBUG("Now receiving dash_varinat %d\n", i);
            } else if (!v->cur_needed && v->needed) {
                v->needed = 0;
                changed = 1;
                DASH_DEBUG("No longer receiving dash_varinat %d\n", i);
            }
        }
    }

    if(changed)
    {
        dash_varinat *old_vv = NULL;
        int64_t last_audio_timestamp = 0;
        for (i = 0; i < c->n_dash_varinats; i++)
        {
            struct dash_varinat *v = c->dash_varinats[i];
            if(v->codec_type == AVMEDIA_TYPE_AUDIO && v->var_used_flag)
            {
                old_vv = v;
                DASH_DEBUG("old_vv %d, codec_type %d, var_used_flag %d\n", v->index, v->codec_type, v->var_used_flag);
                dash_ffurl_close(v);
                //file_seq_set_reset_state(2);
                //c->change_state = 2;
                int j =0;
                AVStream * st = NULL;
                for (j = 0; j < v->ctx->nb_streams; j++) {
                    st = v->ctx->streams[v->pkt.stream_index];
                    if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                        break;
                }
                last_audio_timestamp = av_rescale_rnd(c->last_audio_pkt_dts, AV_TIME_BASE,
                                         st->time_base.den, AV_ROUND_DOWN);
                break;
            }
        }

        if(old_vv)
        {
            dash_varinat *new_vv = c->dash_varinats[c->cur_audio_var];
            DASH_DEBUG("old_vv %d, new_vv %d\n", old_vv->index, new_vv->index);
            new_vv->parent = old_vv->parent;
            new_vv->ctx = old_vv->ctx;
            new_vv->pb = old_vv->pb;
            new_vv->pb->opaque = (void *)new_vv;
            new_vv->ctx->pb->opaque = (void *)new_vv;
            new_vv->seek_timestamp = last_audio_timestamp;
            //new_vv->needed = 1;
            //new_vv->cur_seq_no = c->cur_audio_seq_no;
            struct segment *seg = new_vv->segments[new_vv->start_seq_no];// open init.mp4 or new stream
            if(c->profile_isoff_ondemand)
            {
                dash_open_segmeng_url(c, seg, new_vv, new_vv);
                av_seek_frame(old_vv->ctx, 0, new_vv->seek_timestamp, AVSEEK_FLAG_BACKWARD);//dash only 1 stream
            }
            else
            {
                dash_open_segmeng_url(c, seg, new_vv, new_vv);
                av_seek_frame(old_vv->ctx, 0, 0, AVSEEK_FLAG_BACKWARD);//dash only 1 stream
            }
        }
    }

    return changed;
}


static int mont_dash_read_packet(AVFormatContext * s, AVPacket * pkt)
{
    DASHContext * c = s->priv_data;
    int ret, i, k, mindash_varinat = -1;
    int newseqfound = 0;

    if(s->event_flags == 0x0100)//#define AVFMT_EVENT_FLAG_SWITCH_AUDIO     0x0100
    {
        DASH_DEBUG("s->event_flags 0x%x\n", s->event_flags);
        recheck_dash_audio_discard_flags(s, 0);
        s->event_flags = 0;
    }

start:
    c->end_of_segment = 0;

    for (i = 0; i < c->n_dash_varinats; i++) {
        dash_varinat * var = c->dash_varinats[i];

        if (av_player_is_exit()) {
            DASH_ERROR("[%s] ---detect stop commond at line %d! AVERROR_EOF[%x]\n", __func__, __LINE__, AVERROR_EOF);
            return AVERROR_EOF;
        }

        if (!var->var_used_flag) {
            continue;
        }


        /* Make sure we've got one buffered packet from each open dash_varinat
         * stream */
        //for some reason ,pts will change from the beginning
        if (var->last_seq_no < var->start_seq_no) {
            var->last_seq_no = var->start_seq_no;
        }

        if (var->last_seq_no == var->cur_seq_no - 1 && c->seek_timestamp == AV_NOPTS_VALUE) {
            var->last_seq_no ++;
            newseqfound = 1;
        }

        //DASH_DEBUG("av_read_frame i[%d] 0x%x\n", i, var->pkt.data);
        if (!var->pkt.data) {//var->var_used_flag
            while (1) {
                int64_t ts_diff;
                AVStream * st;
                ret = av_read_frame(var->ctx, &var->pkt);

                //DASH_DEBUG("av_read_frame i[%d], size %d\n", i, var->pkt.size);
                if (ret < 0) {
                    DASH_DEBUG("%s %d error %d \n", __func__, __LINE__, ret);
                    //if (!avio_feof(&var->pb) && ret != AVERROR_EOF)
                    avio_feof(var->pb);//reset

                    if (ret != AVERROR_EOF) { //for exit outside
                        return ret;
                    } else if(ret == AVERROR(ENOMEM))
                        return ret;

                    reset_packet(&var->pkt);
                    break;
                } else {
                    if (c->first_timestamp == AV_NOPTS_VALUE) {
                        c->first_timestamp = var->pkt.dts;
                    }

                    st = var->ctx->streams[var->pkt.stream_index];

                    //var->last_pkt_pts[var->pkt.stream_index] = var->pkt.pts;
                    //var->last_pkt_dts[var->pkt.stream_index] = var->pkt.dts;
                }

                if (var->seek_timestamp == AV_NOPTS_VALUE) {
                    break;
                }

                if (var->pkt.dts == AV_NOPTS_VALUE) {
                    c->seek_timestamp = AV_NOPTS_VALUE;
                    var->seek_timestamp = AV_NOPTS_VALUE;
                    break;
                }

                st = var->ctx->streams[var->pkt.stream_index];

                for (k = 0; k < s->nb_streams; k++) {
                    if (s->streams[k]->id == i) {
                        //  DASH_DEBUG("%s %d i=%d k=%d var->pkt.dts=%lld var->pkt.pts %lld=%lld\n", __func__, __LINE__, i, k, var->pkt.dts, av_rescale_rnd(var->pkt.dts, AV_TIME_BASE, s->streams[k]->time_base.den, c->seek_flags & AVSEEK_FLAG_BACKWARD ? AV_ROUND_DOWN : AV_ROUND_UP));
                        ts_diff = av_rescale_rnd(var->pkt.dts, AV_TIME_BASE, s->streams[k]->time_base.den,
                                                 c->seek_flags & AVSEEK_FLAG_BACKWARD ? AV_ROUND_DOWN : AV_ROUND_UP) - var->seek_timestamp;
                        //DASH_DEBUG("%s %d var %d ts_diff=%lld\n", __func__, __LINE__, var->index, ts_diff);
                        break;
                    }
                }

                if (ts_diff >= 0) {
                    var->seek_timestamp = AV_NOPTS_VALUE;
                    break;
                }

                av_free_packet(&var->pkt);
                reset_packet(&var->pkt);
            }
        }

        /* Check if this stream has the packet with the lowest dts */
        if (var->pkt.data) {
            if (mindash_varinat < 0) {
                mindash_varinat = i;
            } else {
                dash_varinat * minvar = c->dash_varinats[mindash_varinat];
                int64_t dts    =    var->pkt.dts;
                int64_t mindts = minvar->pkt.dts;
                AVStream * st   =    var->ctx->streams[   var->pkt.stream_index];
                AVStream * minst = minvar->ctx->streams[minvar->pkt.stream_index];

                if (st->start_time != AV_NOPTS_VALUE) {
                    dts -=    st->start_time;
                }

                if (minst->start_time != AV_NOPTS_VALUE) {
                    mindts -= minst->start_time;
                }

                if (av_compare_ts(dts, st->time_base, mindts, minst->time_base) < 0) {
                    mindash_varinat = i;
                }
            }
        }
    }


    /* If we got a packet, return it */
    if (mindash_varinat >= 0) {
        *pkt = c->dash_varinats[mindash_varinat]->pkt;
        if(c->dash_varinats[mindash_varinat]->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            pkt->stream_index += c->dash_varinats[c->cur_audio_var]->stream_offset;
            c->last_audio_pkt_dts = pkt->dts;
        }
        else
            pkt->stream_index += c->dash_varinats[mindash_varinat]->stream_offset;
        reset_packet(&c->dash_varinats[mindash_varinat]->pkt);
        //DASH_DEBUG("%s stream_index[%d], %lld\n", __func__, pkt->stream_index, pkt->pts);
        return 0;
    }

    return AVERROR_EOF;
}


static int mont_dash_close(AVFormatContext * s)
{
    DASH_DEBUG("[%s] enter \n", __FUNCTION__);
    DASHContext * c = s->priv_data;
    g_dash_playmode = 0;
    free_dash_varinat_list(c);
    return 0;
}

static int mont_dash_read_seek(AVFormatContext * s, int stream_index,
                          int64_t timestamp, int flags)
{
    DASHContext * c = s->priv_data;
    int i, j, k, ret;
    int64_t first_timestamp;
    int64_t test_timestamp;
    //DASH_DEBUG("[%s] start!stream_index:%d, timestamp:%lld, flags:0x%x \n", __FUNCTION__, stream_index, timestamp, flags);

    if ((flags & AVSEEK_FLAG_BYTE) || !c->dash_varinats[0]->finished) {
        return AVERROR(ENOSYS);
    }

    c->seek_flags     = flags;
    c->seek_timestamp = stream_index < 0 ? timestamp :
                        av_rescale_rnd(timestamp, AV_TIME_BASE,
                                       s->streams[stream_index]->time_base.den,
                                       flags & AVSEEK_FLAG_BACKWARD ?
                                       AV_ROUND_DOWN : AV_ROUND_UP);
    /*timestamp = av_rescale_rnd(timestamp, 1, stream_index >= 0 ?
                               s->streams[stream_index]->time_base.den :
                               AV_TIME_BASE, flags & AVSEEK_FLAG_BACKWARD ?
                               AV_ROUND_DOWN : AV_ROUND_UP);*/
    first_timestamp = c->first_timestamp == AV_NOPTS_VALUE ? 0 :
                      av_rescale_rnd(c->first_timestamp, AV_TIME_BASE, stream_index >= 0 ?
                                     s->streams[stream_index]->time_base.den :
                                     AV_TIME_BASE, flags & AVSEEK_FLAG_BACKWARD ?
                                     AV_ROUND_DOWN : AV_ROUND_UP);

    DASH_DEBUG("[%s] s->duration[%lld] < c->seek_timestamp[%lld]  in timestamp[%lld]\n",
                  __FUNCTION__, s->duration + first_timestamp, c->seek_timestamp, timestamp);
    if (s->duration +  first_timestamp < c->seek_timestamp) {
        //DASH_DEBUG("[%s] s->duration[%lld] < c->seek_timestamp[%lld], seek to end!  \n",
        //          __FUNCTION__, s->duration + first_timestamp, c->seek_timestamp);
#if 0
        c->seek_timestamp = AV_NOPTS_VALUE;
        return AVERROR(EIO);
#else
        c->seek_timestamp = s->duration + first_timestamp - 2 * AV_TIME_BASE;
        timestamp = c->seek_timestamp / AV_TIME_BASE;
#endif
    }

    DASH_DEBUG("[%s] c->n_dash_varinats[%d], timestamp[%lld] c->seek_timestamp[%lld]\n", __FUNCTION__,c->n_dash_varinats,timestamp,c->seek_timestamp);
    //ret = AVERROR(EIO);
    ret = 0;

    for (i = 0; i < c->n_dash_varinats; i++) {
        /* Reset reading */
        dash_varinat * var = c->dash_varinats[i];
        int64_t pos = c->first_timestamp == AV_NOPTS_VALUE ? 0 :
                      av_rescale_rnd(c->first_timestamp, 1, stream_index >= 0 ?
                                     s->streams[stream_index]->time_base.den :
                                     AV_TIME_BASE, flags & AVSEEK_FLAG_BACKWARD ?
                                     AV_ROUND_DOWN : AV_ROUND_UP);

        if (!var->var_used_flag) {
            continue;
        }

        if (var->input) {
            dash_ffurl_close(var);
        }

        dash_varinat *real_var = NULL;
        if(var->codec_type == AVMEDIA_TYPE_VIDEO)
            real_var = c->dash_varinats[c->cur_video_var];
        if(var->codec_type == AVMEDIA_TYPE_AUDIO)
            real_var = c->dash_varinats[c->cur_audio_var];

        av_free_packet(&var->pkt);
        reset_packet(&var->pkt);
        var->pb->eof_reached = 0;
        /* Clear any buffered data */
        var->pb->buf_end = var->pb->buf_ptr = var->pb->buffer;
        /* Reset the pos, to let the mpegts demuxer know we've seeked. */
        var->pb->pos = 0;
        var->needed = 1;

        av_free_packet(&real_var->pkt);
        reset_packet(&real_var->pkt);
        real_var->pb->eof_reached = 0;
        /* Clear any buffered data */
        real_var->pb->buf_end = real_var->pb->buf_ptr = real_var->pb->buffer;
        /* Reset the pos, to let the mpegts demuxer know we've seeked. */
        real_var->pb->pos = 0;
        real_var->needed = 1;

        //DASH_DEBUG("[%s] i=%d pos:%lld \n", __FUNCTION__,i,pos);
        if(c->profile_isoff_ondemand)
        {
            //struct segment *seg = real_var->segments[real_var->cur_seq_no - real_var->start_seq_no];// reopen onedemand stream

            //ret = dash_open_segmeng_url(c, seg, real_var, real_var);
            //for bug 18344
            av_seek_frame(var->ctx, -1, c->seek_timestamp, AVSEEK_FLAG_BACKWARD);//dash only 1 stream
            ret = 0;
        }
        else
        {
            /* Locate the segment that contains the target timestamp */
            //DASH_DEBUG("[%s] line[%d] pos %lld\n", __func__, __LINE__, pos);
            ret = -1;
            for (j = 0; j < var->n_segments-1; j++) {
                //printf("zxtmp j %d, var->segments[j]->duration %d\n", j, var->segments[j]->duration);
                if (c->seek_timestamp >= pos &&
                    c->seek_timestamp < pos + var->segments[j]->duration) {
                    var->cur_seq_no = var->start_seq_no + j;
                    ret = 0;
                    //remember the total duration;
                    var->seek_time_pos = pos;
                    real_var->cur_seq_no = var->cur_seq_no;
                    real_var->seek_time_pos = var->seek_time_pos;
                    DASH_DEBUG("[%s] pos:%lld, j=%d, n_segments:%d,var->cur_seq_no:%d, break!\n", __FUNCTION__,pos,j,var->n_segments,var->cur_seq_no);
                    break;
                }

                pos += var->segments[j]->duration;
            }

            if(ret == 0)
            {
                struct segment *seg = real_var->segments[real_var->start_seq_no];// open init.mp4

                ret = dash_open_segmeng_url(c, seg, real_var, real_var);
                av_seek_frame(var->ctx, 0, 0, AVSEEK_FLAG_BACKWARD);//dash only 1 stream
                DASH_ERROR("[%s %d] var %d Seek end!!!\n", __func__, __LINE__, var->index);
            }
            else
            {
                DASH_ERROR("[%s] line[%d] Seek failed!!!\n", __func__, __LINE__);
                return ret;
            }
        }

        if (ret) {
            c->seek_timestamp = AV_NOPTS_VALUE;
        }

        var->seek_timestamp = c->seek_timestamp;
    }

    DASH_ERROR("[%s] end c->seek_timestamp %lld!\n", __func__, __LINE__, c->seek_timestamp);
    return ret;
}

static int mont_dash_probe(const AVProbeData * p)
{
    DASH_DEBUG("[%s]  enter!\n", __FUNCTION__);

    if (av_stristr(p->buf, "<MPD") &&
        av_stristr(p->buf, "<ContentProtection") &&
        (!av_stristr(p->buf, "audio/webm") && !av_stristr(p->buf, "video/webm"))) {
        g_dash_playmode = 2;//live for default
        return AVPROBE_SCORE_MAX;
    }

    return 0;
}

AVInputFormat ff_mont_dash_demuxer = {
    .name           = "mondash",
    .long_name      = "Mont Mpeg Dash",
    .priv_data_size = sizeof(DASHContext),
    .read_probe     = mont_dash_probe,
    .read_header    = mont_dash_read_header,
    .read_packet    = mont_dash_read_packet,
    .read_close     = mont_dash_close,
    .read_seek      = mont_dash_read_seek,
};
