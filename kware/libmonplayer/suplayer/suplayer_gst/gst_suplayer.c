/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <glib/gprintf.h>
#include <pthread.h>
#include <sys/times.h>
#include <unistd.h>
#include "config.h"
#include <locale.h>
#include <glib.h>
#include "mt_type.h"
#include "mt_type.h"
#include "mt_common.h"
#include "mtsu_type.h"
#include "mtsu_svr_player.h"
#include "suplayer_internal.h"
#include "file_playback_sequence.h"
#include "avplayal.h"

#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <gst/video/video.h>
#include <gst/pbutils/pbutils.h>
#include <gst/tag/tag.h>
#include <gst/math-compat.h>

#define GST_URL_LEN 4096
typedef enum
{
	GST_PLAY_TRICK_MODE_NONE = 0,
	GST_PLAY_TRICK_MODE_DEFAULT,
	GST_PLAY_TRICK_MODE_DEFAULT_NO_AUDIO,
	GST_PLAY_TRICK_MODE_KEY_UNITS,
	GST_PLAY_TRICK_MODE_KEY_UNITS_NO_AUDIO,
	GST_PLAY_TRICK_MODE_LAST
} GstPlayTrickMode;


typedef enum
{
	GST_PLAY_TRACK_TYPE_INVALID = 0,
	GST_PLAY_TRACK_TYPE_AUDIO,
	GST_PLAY_TRACK_TYPE_VIDEO,
	GST_PLAY_TRACK_TYPE_SUBTITLE
} GstPlayTrackType;

typedef MT_U32  (*gst_player_event_cb_fun_t)(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent);

typedef struct
{
	//  gchar **uris;
	//  guint num_uris;
	//  gint cur_idx;

	void *playbin;

	GMainLoop *loop;
	guint bus_watch;
	//  guint timeout;

	/* missing plugin messages */
	//  GList *missing;

	gboolean buffering;
	gboolean is_live;

	GstState desired_state;       /* as per user interaction, PAUSED or PLAYING */

	//  gulong deep_notify_id;

	/* configuration */
	//  gboolean gapless;

	GstPlayTrickMode trick_mode;
	gdouble rate;

	char url[GST_URL_LEN];

	///pipe struct
	//// pipes[0] read pipe , pipes[1] write pipe
	int pipes[2];	////pipe
	gulong		  io_source;


	gst_player_event_cb_fun_t gst_event_cb ;
} GstPlay;




typedef enum
{
	GST_PLAY_CMD_INVALID = 0,
	GST_PLAY_CMD_SET_URL,
	GST_PLAY_CMD_START,
	GST_PLAY_CMD_STOP,
	GST_PLAY_CMD_PAUSE,
	GST_PLAY_CMD_RESUME,
	GST_PLAY_CMD_FASTSPEED,
	GST_PLAY_CMD_SEEK,
	GST_PLAY_CMD_EXIT,
	GST_PLAY_CMD_AUDIO_SELECT_TRACK,

	GST_PLAY_CMD_MAX_VALUE,
} GstPlayCmdType;



typedef struct
{
	GstPlayCmdType type ;
	union {
		char url[GST_URL_LEN];////GST_PLAY_CMD_SET_URL
		char speed ; /// -32 ~ + 32

		int seektime;
        int track_id;
	};
}gst_cmd_info_t ;




GST_DEBUG_CATEGORY (play_debug);
#define GST_CAT_DEFAULT play_debug


#if  0
	static void
play_about_to_finish (GstElement * playbin, gpointer user_data)
{
	GstPlay *play = user_data;
	if (!play->gapless)
		return;
}
	static gchar *
play_uri_get_display_name (GstPlay * play, const gchar * uri)
{
	gchar *loc;

	if (gst_uri_has_protocol (uri, "file")) {
		loc = g_filename_from_uri (uri, NULL, NULL);
	} else if (gst_uri_has_protocol (uri, "pushfile")) {
		loc = g_filename_from_uri (uri + 4, NULL, NULL);
	} else {
		loc = g_strdup (uri);
	}

	/* Maybe additionally use glib's filename to display name function */
	return loc;
}
#endif

#if 1//MT_DES("USE GST PLAYER", 0)

static void   play_reset (GstPlay * play)
{
	play->buffering = FALSE;
	play->is_live = FALSE;
}

static gboolean   play_do_seek (GstPlay * play, gint64 pos, gdouble rate, GstPlayTrickMode mode, gboolean is_skip)
{
	GstSeekFlags seek_flags;
	GstQuery *query;
	GstEvent *seek;
	gboolean seekable = FALSE;

	g_printf("[%s]==========pos:%lld==========, is_skip:%d\n",__func__,pos, is_skip);


	query = gst_query_new_seeking (GST_FORMAT_TIME);
	if (!gst_element_query (play->playbin, query)) {
		gst_query_unref (query);
		g_printf("[%s][ERROR]=====fail to query====!!!!\n",__func__);
		return FALSE;
	}

	gst_query_parse_seeking (query, NULL, &seekable, NULL, NULL);
	gst_query_unref (query);

	if (!seekable){
		g_print ("[%s]=====not support seek=====!!!!!\n",__func__);
		return FALSE;
	}
	seek_flags = GST_SEEK_FLAG_FLUSH;

	switch (mode) {

		case GST_PLAY_TRICK_MODE_DEFAULT:
			seek_flags |= GST_SEEK_FLAG_TRICKMODE;
			break;

		case GST_PLAY_TRICK_MODE_DEFAULT_NO_AUDIO:
			seek_flags |= GST_SEEK_FLAG_TRICKMODE | GST_SEEK_FLAG_TRICKMODE_NO_AUDIO;
			break;

		case GST_PLAY_TRICK_MODE_KEY_UNITS:
			seek_flags |= GST_SEEK_FLAG_TRICKMODE_KEY_UNITS;
			break;

		case GST_PLAY_TRICK_MODE_KEY_UNITS_NO_AUDIO:
			seek_flags |=
				GST_SEEK_FLAG_TRICKMODE_KEY_UNITS | GST_SEEK_FLAG_TRICKMODE_NO_AUDIO;
			break;

		case GST_PLAY_TRICK_MODE_NONE:
		default:

			break;

	}

    if(is_skip)//for audio track selection, video dont change
        seek_flags |= GST_SEEK_FLAG_SKIP;

	if (rate >= 0)
        seek = gst_event_new_seek(rate, GST_FORMAT_TIME,
                                  seek_flags | GST_SEEK_FLAG_ACCURATE,
                                  /* start */ GST_SEEK_TYPE_SET, pos,
                                  /* stop */ GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE);
    else {
        {
            gint64  dur = -1;
            gst_element_query_duration(play->playbin, GST_FORMAT_TIME, &dur);
            printf("\n%s %d %f %f\n", __FUNCTION__, __LINE__, pos * 1.f / GST_SECOND, dur * 1.f / GST_SECOND);
            //pos = dur * 1.f / GST_SECOND * 0.125 * GST_SECOND;
            printf("\n%s %d %f %f\n", __FUNCTION__, __LINE__, pos * 1.f / GST_SECOND, dur * 1.f / GST_SECOND);
        }
        seek = gst_event_new_seek(rate, GST_FORMAT_TIME,
                                  seek_flags | GST_SEEK_FLAG_ACCURATE,
                                  /* start */ GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE,
                                  /* stop */ GST_SEEK_TYPE_SET, pos);
    }


	if (!gst_element_send_event (play->playbin, seek))
	{
		g_printf("[%s]=====fail to send seek event====!!!!\n",__func__);
		return FALSE;
	}

	play->rate = rate;
	play->trick_mode = mode;
	return TRUE;


}




#if 0       // compile warning, ignore it
static gboolean  play_set_rate_and_trick_mode (GstPlay * play, gdouble rate,
		GstPlayTrickMode mode)
{
	gint64 pos = -1;

	g_return_val_if_fail (rate != 0, FALSE);

	if (!gst_element_query_position (play->playbin, GST_FORMAT_TIME, &pos))
	{
		g_printf("[%s]=====fail to set_rate_and_trick_mode====!!!!\n",__func__);
		return FALSE;
	}

	return play_do_seek (play, pos, rate, mode, FALSE);
}





static void   play_set_playback_rate (GstPlay * play, gdouble rate)
{
	if (play_set_rate_and_trick_mode (play, rate, play->trick_mode)) {
		g_print ("Playback rate: %.2f", rate);
		g_print ("                               \n");
	} else {
		g_print ("\n");
		g_print ("Could not change playback rate to %.2f", rate);
		g_print (".\n");
	}
}





static void  play_set_relative_playback_rate (GstPlay * play, gdouble rate_step,
		gboolean reverse_direction)
{
	gdouble new_rate = play->rate + rate_step;

	if (reverse_direction)
		new_rate *= -1.0;

	play_set_playback_rate (play, new_rate);
}
#endif




static void  play_uri (GstPlay * play, const char * next_uri)
{
	gchar *loc;

	gst_element_set_state (play->playbin, GST_STATE_READY);
	play_reset (play);

#if  0
	loc = play_uri_get_display_name (play, next_uri);
	g_print (("Now playing %s\n"), loc);
	g_free (loc);
#endif

	loc = g_strdup (next_uri);
	g_object_set (play->playbin, "uri", loc, NULL);

	switch (gst_element_set_state (play->playbin, GST_STATE_PAUSED)) {
		case GST_STATE_CHANGE_FAILURE:
			/* ignore, we should get an error message posted on the bus */
			g_print ("GST_STATE_CHANGE_FAILURE...\r");
			break;
		case GST_STATE_CHANGE_NO_PREROLL:
			g_print ("Pipeline is live.\n");
			play->is_live = TRUE;
			break;
		case GST_STATE_CHANGE_ASYNC:
			g_print ("Prerolling...\r");
			break;
		default:
			g_print ("default ...\r");
			break;
	}
	g_print (" change status to = %d	...\n",play->desired_state);

	if (play->desired_state != GST_STATE_PAUSED)
	{
		gst_element_set_state (play->playbin, play->desired_state);
	}

	g_print(" %s %d  end end !\n", __FUNCTION__, __LINE__);
}

static void
play_select_track (GstPlay * play, GstPlayTrackType track_type, int track_id)
{
  const gchar *prop_cur, *prop_n, *prop_get, *name;
  gint cur = -1, n = -1;

  switch (track_type) {
    case GST_PLAY_TRACK_TYPE_AUDIO:
      prop_get = "get-audio-tags";
      prop_cur = "current-audio";
      prop_n = "n-audio";
      name = "audio";
      break;
    case GST_PLAY_TRACK_TYPE_VIDEO:
      prop_get = "get-video-tags";
      prop_cur = "current-video";
      prop_n = "n-video";
      name = "video";
      break;
    case GST_PLAY_TRACK_TYPE_SUBTITLE:
      prop_get = "get-text-tags";
      prop_cur = "current-text";
      prop_n = "n-text";
      name = "subtitle";
      break;
    default:
      return;
  }

  g_object_get (play->playbin, prop_cur, &cur, prop_n, &n, NULL);

  if (n < 1) {
    g_print ("No %s tracks.\n", name);
  } else if (n == 1) {
    g_print ("No other %s tracks to switch to.\n", name);
  } else {
    gchar *lcode = NULL, *lname = NULL;
    const gchar *lang = NULL;
    GstTagList *tags = NULL;

    //cur = (cur + 1) % n;//cycle
    if(track_id <0 || track_id >= n)
    {
        g_print ("ERROR: track_id[%d] is not in range, max track_num[%d].\n", track_id, n);
        return;
    }

    if(cur == track_id)
    {
        g_print ("same tarck, donothing.\n");
        return;
    }

    cur = track_id;
    g_signal_emit_by_name (play->playbin, prop_get, cur, &tags);
    if (tags != NULL) {
      if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &lcode))
        lang = gst_tag_get_language_name (lcode);
      else if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_NAME, &lname))
        lang = lname;
      gst_tag_list_unref (tags);
    }
    if (lang != NULL)
      g_print ("Switching to %s track %d of %d (%s).\n", name, cur, n,
          lang);
    else
      g_print ("Switching to %s track %d of %d.\n", name, cur, n);
    g_object_set (play->playbin, prop_cur, cur, NULL);
    g_free (lcode);
    g_free (lname);
  }
}

#if 0
#define VOLUME_STEPS 20
	static void
play_set_relative_volume (GstPlay * play, gdouble volume_step)
{
	gdouble volume;

	volume = gst_stream_volume_get_volume (GST_STREAM_VOLUME (play->playbin),
			GST_STREAM_VOLUME_FORMAT_CUBIC);

	volume = round ((volume + volume_step) * VOLUME_STEPS) / VOLUME_STEPS;
	volume = CLAMP (volume, 0.0, 10.0);

	gst_stream_volume_set_volume (GST_STREAM_VOLUME (play->playbin),
			GST_STREAM_VOLUME_FORMAT_CUBIC, volume);

	g_print (("Volume: %.0f%%"), volume * 100);
	g_print ("                  \n");
}

	static gboolean
play_timeout (gpointer user_data)
{
	GstPlay *play = user_data;
	gint64 pos = -1, dur = -1;
	const gchar *paused = "Paused";
	gchar *status;

	if (play->buffering)
		return TRUE;

	gst_element_query_position (play->playbin, GST_FORMAT_TIME, &pos);
	gst_element_query_duration (play->playbin, GST_FORMAT_TIME, &dur);

	if (play->desired_state == GST_STATE_PAUSED) {
		status = (gchar *) paused;
	} else {
		gint len = g_utf8_strlen (paused, -1);
		status = g_newa (gchar, len + 1);
		memset (status, ' ', len);
		status[len] = '\0';
	}

	if (pos >= 0 && dur > 0) {
		gchar dstr[32], pstr[32];

		/* FIXME: pretty print in nicer format */
		g_snprintf (pstr, 32, "%" GST_TIME_FORMAT, GST_TIME_ARGS (pos));
		pstr[9] = '\0';
		g_snprintf (dstr, 32, "%" GST_TIME_FORMAT, GST_TIME_ARGS (dur));
		dstr[9] = '\0';
		g_print ("%s / %s %s\r", pstr, dstr, status);
	}

	return TRUE;
}
#endif


static void   play_free (GstPlay * play)
{
	/* No need to see all those pad caps going to NULL etc., it's just noise */
	play_reset (play);

	gst_element_set_state (play->playbin, GST_STATE_NULL);
	gst_object_unref (play->playbin);

	g_source_remove (play->bus_watch);
	//  g_source_remove (play->timeout);
	g_main_loop_unref (play->loop);

	//  g_strfreev (play->uris);
	g_free (play);
}



static void gstmain_loop_quit(GstPlay * play)
{
	if(play)
	{
		g_main_loop_quit (play->loop);
	}
}


static void *gstmain_loop_func(void * handle)
{
	GMainLoop *loop = (GMainLoop*) handle ;
	if(loop)
	{
		mt_set_pthread_name(__FUNCTION__);
		g_main_loop_run (loop);
	}
}


static gboolean  gst_query_pos (GstPlay *play)
{
	gint64 pos = -1, dur = -1;
	//const gchar *paused = "Paused";
	//gchar *status;

	//g_print ("%s: enter enter play->desired_state[%d] buffering[%d]\n",
	//  __func__, play->desired_state, play->buffering);

	/*if (play->buffering){
		g_print ("[%s] buffering : do nothing\n",__func__);
		return TRUE;
	}*/

	gst_element_query_position (play->playbin, GST_FORMAT_TIME, &pos);
	gst_element_query_duration (play->playbin, GST_FORMAT_TIME, &dur);

	//if (play->desired_state == GST_STATE_PAUSED)
	//{
		//status = (gchar *) paused;
	//}
	//else
	//{
		//gint len = g_utf8_strlen (paused, -1);
		//status = g_newa (gchar, len + 1);
		//memset (status, ' ', len);
		//status[len] = '\0';
	//}

	if (pos >= 0 && dur > 0) {
		gchar dstr[64], pstr[64];
		memset(dstr,0,64);
		memset(pstr,0,64);

		/* FIXME: pretty print in nicer format */
		g_snprintf (pstr, 32, "%" GST_TIME_FORMAT, GST_TIME_ARGS (pos));
		pstr[9] = '\0';

		g_snprintf (dstr, 32, "%" GST_TIME_FORMAT, GST_TIME_ARGS (dur));
		dstr[9] = '\0';

		g_print ("playing :[pts]%s / [duration]%s\n", pstr, dstr);
	}
	else{

		g_print ("playing :[pts]%lld / [duration]%lld\n", pos, dur);

	}

	return TRUE;

}



static void * gstget_pts_func (void * user_data)
{


	GstPlay *play = (GstPlay *)user_data;
	int cnt = 0;

	g_print ("[%s]: @@@@@@@@@@@@@@@@@@@@@@@@@enter enter play->desired_state[%d]\n", __func__, play->desired_state);
	mt_set_pthread_name(__FUNCTION__);
	while(play->desired_state == GST_STATE_PAUSED || play->desired_state == GST_STATE_PLAYING)
	{
		MT_USLEEP(1000000);
		gst_query_pos(play);
		cnt++;
	}


	g_print ("[%s]:@@@@@@@@@@@@@@@@@@ end end desired_state[%d]\n", __func__, play->desired_state);

	return NULL;
}



static gboolean gst_play_pipe_cmd_io_cb (GIOChannel * channel, GIOCondition condition, gpointer user_data)
{
	GIOStatus  status;
	gst_cmd_info_t tmp={0};
	GstPlay *play = (GstPlay*) user_data ;
	gsize	   read_bytes = 0;

	g_assert_cmpuint (condition, ==, G_IO_IN);

	for (status = g_io_channel_read_chars (channel, (gchar*)&tmp, sizeof (gst_cmd_info_t), &read_bytes, NULL);
			status == G_IO_STATUS_NORMAL;
			status = g_io_channel_read_chars (channel, (gchar*)&tmp, sizeof (gst_cmd_info_t), &read_bytes, NULL))
	{
		g_print("[%s]==== get a msg ,status = %d  play = 0x%x !\n",__func__,tmp.type,play);

		switch(tmp.type)
		{
			case GST_PLAY_CMD_SET_URL:
				g_print("[%s]==== get GST_PLAY_CMD_SET_URL msg ,url = %s  !\n",__func__,tmp.url);
				memset(play->url, 0, GST_URL_LEN);
				memcpy(play->url,tmp.url,strlen(tmp.url));
				if(play->gst_event_cb)
				{
					//play->gst_event_cb(FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS,0);
				}
				break ;


			case GST_PLAY_CMD_START:
				play->desired_state = GST_STATE_PLAYING;
				g_print("[%s]==== get GST_PLAY_CMD_START msg  !\n",__func__);
				g_print("[%s]===play->url:%s\n",__func__,play->url);



				if( (strstr(play->url,"http://") != NULL)
					|| (strstr(play->url,"https://") != NULL)
					|| (strstr(play->url,"rtmp://") != NULL) )
				{
					g_print("[%s]====this is network stream =====\n",__func__);

				}
			    else{

					g_print("[%s]====local file=====\n",__func__);
					gchar *uri	= gst_filename_to_uri (play->url, NULL);
					if(uri)
					{
						play->url[0]=0;
						strcpy(play->url,uri);
						g_free(uri);
					}

				}

				play_uri(play,play->url);
				pthread_t tid = 0;
				pthread_create(&tid, NULL, gstget_pts_func, (void *)play);
				break ;


			case GST_PLAY_CMD_STOP:
				g_print("[%s]==== get GST_PLAY_CMD_STOP msg  !\n",__func__);
				{
					//g_object_set (play->playbin, "uri", next_uri, NULL);
					play_reset(play);
					play->desired_state = GST_STATE_NULL;
					gst_element_set_state (play->playbin, play->desired_state);
				}
				break ;


			case GST_PLAY_CMD_PAUSE:
				g_print("[%s]==== get GST_PLAY_CMD_PAUSE msg  !\n",__func__);
				{
					play->desired_state = GST_STATE_PAUSED;

					if (!play->buffering) {
						gst_element_set_state (play->playbin, play->desired_state);
					}
				}
				break ;


			case GST_PLAY_CMD_RESUME:
				g_print("[%s]==== get GST_PLAY_CMD_RESUME msg  !\n",__func__);
				{
					play->desired_state = GST_STATE_PLAYING;

					if (!play->buffering) {
						gst_element_set_state (play->playbin, play->desired_state);
					} else if (play->desired_state == GST_STATE_PLAYING) {
						g_print ("\nWill play as soon as buffering finishes)\n");
					}
				}
				break ;



			case GST_PLAY_CMD_FASTSPEED:
				{
					gint64 pos = -1;

					if (!gst_element_query_position (play->playbin, GST_FORMAT_TIME, &pos))
						return FALSE;

					g_print("[%s]==== get GST_PLAY_CMD_FASTSPEED msg  spped[%d]!\n",__func__,tmp.speed);
					play->rate = tmp.speed;
					play_do_seek (play,pos, play->rate, play->trick_mode, FALSE);
				}
				break;



			case GST_PLAY_CMD_SEEK:
				{
					gint64 pos = -1;

					if (!gst_element_query_position (play->playbin, GST_FORMAT_TIME, &pos))
						return FALSE;

					g_print("[%s]==== get GST_PLAY_CMD_SEEK msg  pos[%lld] seektime[%d]!\n",__func__, pos, tmp.seektime);
					pos = (gint64)tmp.seektime *(1000000LL) ; //ms to us

					play_do_seek (play,pos, play->rate, play->trick_mode, FALSE);
				}
				break;

            case GST_PLAY_CMD_AUDIO_SELECT_TRACK:
                {
					g_print("[%s]==== get GST_PLAY_CMD_AUDIO_SELECT_TRACK msg  track_id[%d] !\n",__func__, tmp.track_id);
                    gint64 pos = -1;

					if (!gst_element_query_position (play->playbin, GST_FORMAT_TIME, &pos))
						return FALSE;
					play_select_track (play, GST_PLAY_TRACK_TYPE_AUDIO, tmp.track_id);
                    play_do_seek (play,pos, play->rate, play->trick_mode, TRUE);
				}
				break;

			case GST_PLAY_CMD_EXIT:
				g_print("[%s]==== get GST_PLAY_CMD_EXIT msg  !\n",__func__);
				{
					gstmain_loop_quit(play);
					play_free (play);
				}
				break ;



			default:
				g_print("[%s]==== get null msg  !\n",__func__);
				break;


		}
	}

	g_assert_cmpuint (status, ==, G_IO_STATUS_AGAIN);

	return TRUE;

}


static void  gst_play_pipe_cmd_handler(GstPlay *play)
{

	g_printf("[%s]====start start====\n",__func__);


	GIOChannel	  * channel;
	int flags;
	const char	  * line_term;
	int 			line_term_len;

	if (0 > pipe (play->pipes))
	{
		g_error ("[%s] error creating pipe: %s",__func__,g_strerror (errno));
		return;
	}

	channel = g_io_channel_unix_new (play->pipes[0]);
	g_io_channel_set_close_on_unref (channel, TRUE);
	g_io_channel_set_encoding (channel, NULL, NULL);
	g_io_channel_set_buffered (channel, FALSE);
	flags = g_io_channel_get_flags (channel);
	g_io_channel_set_flags (channel, flags | G_IO_FLAG_NONBLOCK, NULL);
	g_assert (g_io_channel_get_line_term (channel, NULL) == NULL);
	g_io_channel_set_line_term (channel, "\n", 1);
	line_term = g_io_channel_get_line_term (channel, &line_term_len);
	g_assert_cmpint (*line_term, ==, '\n');
	g_assert_cmpint (line_term_len, ==, 1);

	g_assert (g_io_channel_get_close_on_unref (channel));
	g_assert (g_io_channel_get_encoding (channel) == NULL);
	g_assert (!g_io_channel_get_buffered (channel));

	play->io_source = g_io_add_watch_full (channel, G_PRIORITY_DEFAULT, G_IO_IN,
			(GIOFunc) gst_play_pipe_cmd_io_cb, play, NULL) ;

	g_io_channel_unref (channel);


	g_printf("[%s]====end end====\n",__func__);


}



static void gst_main_loop_start(GstPlay *play)
{
	pthread_t tid = 0;
	pthread_create(&tid, NULL, gstmain_loop_func, play->loop);

}

static gboolean play_bus_msg (GstBus * bus, GstMessage * msg, gpointer user_data)
{
	//g_print("[%s]====start start, msg=%s, from=%s\n",__func__,GST_MESSAGE_TYPE_NAME(msg), GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)));
	GstPlay *play = user_data;
	MT_U32 (*pfunc)(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent);
	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_ASYNC_DONE:
			g_print("[%s]====GST_MESSAGE_ASYNC_DONE====\n",__func__);
			/* dump graph on preroll */
			GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (play->playbin),
					GST_DEBUG_GRAPH_SHOW_ALL, "gst-play.async-done");

			g_print ("Prerolled.\r");
			break;
		case GST_MESSAGE_BUFFERING:{
						   gint percent;
						   //g_print("[%s]====GST_MESSAGE_BUFFERING= play->buffering = %d ===\n",__func__,play->buffering);
						   //if (!play->buffering)
							//   g_print ("!play->buffering \n");

						   gst_message_parse_buffering (msg, &percent);
						   /* get more stats */
                           GstBufferingMode mode;
                           gint avg_in, avg_out;
                           gint64 buffering_left;
                           gst_message_parse_buffering_stats (msg, &mode, &avg_in, &avg_out,&buffering_left);

						   if (percent == 100) {
							   /* a 100% message means buffering is done */
							   if (play->buffering) {
								   play->buffering = FALSE;
								   /* no state management needed for live pipelines */
								   if (!play->is_live){

									   g_print ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
									   gst_element_set_state (play->playbin, play->desired_state);
								   }
							   }
						   } else {
							   /* buffering... */
							   if (!play->buffering) {
								   if (!play->is_live){
                                        //changed by zhouxiang, queue dont buffering, only by es waterlevel,
                                        //so add gst_message_new_buffering_1 for this
                                        if(buffering_left == 1000)//real buffering mark
                                        {
									        g_print ("@@@@@@@@@@@@@@@@@@@@GST_STATE_PAUSED@@@@@@@@@@@@@\n");
									        gst_element_set_state (play->playbin, GST_STATE_PAUSED);
                                            play->buffering = TRUE;
                                        }
                                        //else
                                        //    g_print ("dont care queue buffering msg!!!!\n");
								   }
							   }
						   }
						   break;
					   }
		case GST_MESSAGE_CLOCK_LOST:{
						    g_print("[%s]====GST_MESSAGE_CLOCK_LOST====\n",__func__);
						    g_print (("Clock lost, selecting a new one\n"));
						    gst_element_set_state (play->playbin, GST_STATE_PAUSED);
						    gst_element_set_state (play->playbin, GST_STATE_PLAYING);
						    break;
					    }
		case GST_MESSAGE_LATENCY:
					    g_print ("Redistribute latency...\n");
					    gst_bin_recalculate_latency (GST_BIN (play->playbin));
					    break;
		case GST_MESSAGE_REQUEST_STATE:{
						       GstState state;
						       gchar *name;

						       name = gst_object_get_path_string (GST_MESSAGE_SRC (msg));

						       gst_message_parse_request_state (msg, &state);

						       g_print ("Setting state to %s as requested by %s...\n",
								       gst_element_state_get_name (state), name);

						       gst_element_set_state (play->playbin, state);
						       g_free (name);
						       break;
					       }
		case GST_MESSAGE_EOS:
					       g_print("[%s]====GST_MESSAGE_EOS====\n",__func__);
					       /* print final position at end */
					       {
						       MT_SVR_PLAYER_EVENT_S event;
						       MT_SVR_PLAYER_STATE_E data;

						       data = MT_SVR_PLAYER_STATE_STOP;
						       event.eEvent = MT_SVR_PLAYER_EVENT_STATE_CHANGED;
						       event.pu8Data = (MT_U8 *)(&data);
						       event.u32Len = sizeof(MT_SVR_PLAYER_STATE_E);
						       pfunc = play->gst_event_cb;
						       (*pfunc)(NULL, &event);
					       }
					       g_print ("\n");
					       break;
		case GST_MESSAGE_WARNING:{
						 g_print("[%s]====GST_MESSAGE_WARNING====\n",__func__);
						 GError *err;
						 gchar *dbg = NULL;

						 /* dump graph on warning */
						 GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (play->playbin),
								 GST_DEBUG_GRAPH_SHOW_ALL, "gst-play.warning");

						 gst_message_parse_warning (msg, &err, &dbg);
						 g_printerr ("WARNING %s\n", err->message);
						 if (dbg != NULL)
							 g_printerr ("WARNING debug information: %s\n", dbg);
						 g_clear_error (&err);
						 g_free (dbg);
						 break;
					 }
		case GST_MESSAGE_ERROR:{
					       GError *err;
					       gchar *dbg;
					       g_print("[%s]====GST_MESSAGE_ERROR====\n",__func__);
					       /* dump graph on error */
					       GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (play->playbin),
							       GST_DEBUG_GRAPH_SHOW_ALL, "gst-play.error");

					       gst_message_parse_error (msg, &err, &dbg);
					       g_printerr ("ERROR %s for %s\n", err->message, play->url);
					       if (dbg != NULL)
						       g_printerr ("ERROR debug information: %s\n", dbg);
					       g_clear_error (&err);
					       g_free (dbg);

					       /* flush any other error messages from the bus and clean up */
					       //gst_element_set_state (play->playbin, GST_STATE_NULL);
					       {
						       MT_SVR_PLAYER_EVENT_S event;
						       MT_SVR_PLAYER_STATE_E data;

						       data = MT_SVR_PLAYER_STATE_STOP;
						       event.eEvent = MT_SVR_PLAYER_EVENT_ERROR;
						       event.pu8Data = (MT_U8 *)(&data);
						       event.u32Len = sizeof(MT_SVR_PLAYER_STATE_E);
						       pfunc = play->gst_event_cb;
						       (*pfunc)(NULL, &event);
					       }
					       break;
				       }
		case GST_MESSAGE_ELEMENT:
				       {
					       break;
				       }
		default:
				       break;
	}

	return TRUE;
}

static void gst_play_cmd_pass(void *handle,gst_cmd_info_t* cmd)
{
	GstPlay *play = (GstPlay*) handle ;
	//	char str[128] = {0};
	//	sprintf(str,"%d",cmd);
	write(play->pipes[1],cmd,sizeof(gst_cmd_info_t));
}



/*
   public api for player

 */
static void * gst_player_init(void)
{


	g_printf("[%s]====start start======\n",__func__);

	GstPlay * play = NULL ;
	//	  GPtrArray *playlist;
//	gboolean verbose = FALSE;
//	gboolean interactive = TRUE;
//	gboolean gapless = FALSE;
//	gboolean shuffle = FALSE;
//	gboolean print_version = FALSE;
//	gboolean quiet = FALSE;
//	gdouble volume = -1;
//	gchar **filenames = NULL;
//	gchar *audio_sink = NULL;
//	gchar *video_sink = NULL;
	//	  gchar **uris;
//	gchar *flags = NULL;
//	guint num, i;
	GError *err = NULL;
	GOptionContext *ctx = NULL;
	//	  gchar *playlist_file = NULL;
	GOptionEntry options[] = {
#if 0
		{"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
			("Output status information and property notifications"), NULL},
		{"flags", 0, 0, G_OPTION_ARG_STRING, &flags,
			("Control playback behaviour setting playbin 'flags' property"),
			NULL},
		{"version", 0, 0, G_OPTION_ARG_NONE, &print_version,
			("Print version information and exit"), NULL},
		{"videosink", 0, 0, G_OPTION_ARG_STRING, &video_sink,
			("Video sink to use (default is autovideosink)"), NULL},
		{"audiosink", 0, 0, G_OPTION_ARG_STRING, &audio_sink,
			("Audio sink to use (default is autoaudiosink)"), NULL},
		{"gapless", 0, 0, G_OPTION_ARG_NONE, &gapless,
			("Enable gapless playback"), NULL},
#if 0
		{"shuffle", 0, 0, G_OPTION_ARG_NONE, &shuffle,
			N_("Shuffle playlist"), NULL},
#endif
		{"no-interactive", 0, G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,
			&interactive,
			("Disable interactive control via the keyboard"), NULL},
		{"volume", 0, 0, G_OPTION_ARG_DOUBLE, &volume,
			("Volume"), NULL},
#if 0
		{"playlist", 0, 0, G_OPTION_ARG_FILENAME, &playlist_file,
			N_("Playlist file containing input media files"), NULL},
#endif
		{"quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet,
			("Do not print any output (apart from errors)"), NULL},

		{G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &filenames, NULL},
#endif
		{NULL}
	};

	setlocale (LC_ALL, "");

	//g_printf("[%s]======111==============\n",__func__);
	//	system("cat /proc/meminfo");

	g_set_prgname ("mt_gst-play");

	ctx = g_option_context_new ("FILE1|URI1 [FILE2|URI2] [FILE3|URI3] ...");
	g_option_context_add_main_entries (ctx, options, "libsoup");
	g_option_context_add_group (ctx, gst_init_get_option_group ());
	if (!g_option_context_parse (ctx, NULL,NULL, &err)) {
		g_print ("Error initializing: %s\n", GST_STR_NULL (err->message));
		g_option_context_free (ctx);
		g_clear_error (&err);
		return NULL;
	}
	g_option_context_free (ctx);


	GST_DEBUG_CATEGORY_INIT (play_debug, "play", 0, "mt_gst-play");



	//these code only for static plugins
	GST_PLUGIN_STATIC_REGISTER(playback);
	GST_PLUGIN_STATIC_REGISTER(coreelements);
	GST_PLUGIN_STATIC_REGISTER(monmulti_queue);
	//GST_PLUGIN_STATIC_REGISTER(isomp4);
	//GST_PLUGIN_STATIC_REGISTER(matroska);
	//GST_PLUGIN_STATIC_REGISTER(avi);
	//GST_PLUGIN_STATIC_REGISTER(mpegtsdemux);
	//GST_PLUGIN_STATIC_REGISTER(flv);

	//g_printf("regster neon hls libav ------------------------------------\n");
	GST_PLUGIN_STATIC_REGISTER(neon);
	GST_PLUGIN_STATIC_REGISTER(hls);
	GST_PLUGIN_STATIC_REGISTER(libav);

#if 1//def  CONFIG_MT_ENABLE_GST_DASH_DEMUX
	GST_PLUGIN_STATIC_REGISTER(dashdemux);
#endif

	GST_PLUGIN_STATIC_REGISTER(typefindfunctions);
	GST_PLUGIN_STATIC_REGISTER(vdec);
	GST_PLUGIN_STATIC_REGISTER(adec);
	//GST_PLUGIN_STATIC_REGISTER(monvdec);
	//GST_PLUGIN_STATIC_REGISTER(monadec);
	GST_PLUGIN_STATIC_REGISTER(montvsink);
	GST_PLUGIN_STATIC_REGISTER(montasink);
	GST_PLUGIN_STATIC_REGISTER(montssink);
	//GST_PLUGIN_STATIC_REGISTER(videoparsersbad);//
	//GST_PLUGIN_STATIC_REGISTER(audioparsers);//


#if 0
	playlist = g_ptr_array_new ();

	if (playlist_file != NULL) {
		gchar *playlist_contents = NULL;
		gchar **lines = NULL;

		if (g_file_get_contents (playlist_file, &playlist_contents, NULL, &err)) {
			lines = g_strsplit (playlist_contents, "\n", 0);
			num = g_strv_length (lines);

			for (i = 0; i < num; i++) {
				if (lines[i][0] != '\0') {
					GST_LOG ("Playlist[%d]: %s", i + 1, lines[i]);
					add_to_playlist (playlist, lines[i]);
				}
			}
			g_strfreev (lines);
			g_free (playlist_contents);
		} else {
			g_printerr ("Could not read playlist: %s\n", err->message);
			g_clear_error (&err);
		}
		g_free (playlist_file);
		playlist_file = NULL;
	}

	if (playlist->len == 0 && (filenames == NULL || *filenames == NULL)) {
		g_printerr (("Usage: %s FILE1|URI1 [FILE2|URI2] [FILE3|URI3] ..."),
				"gst-play-1.0" );
		g_printerr ("\n\n"),
			   g_printerr ("%s\n\n",
					   _("You must provide at least one filename or URI to play."));
		/* No input provided. Free array */
		g_ptr_array_free (playlist, TRUE);

		g_free (audio_sink);
		g_free (video_sink);

		return 1;
	}

	/* fill playlist */
	if (filenames != NULL && *filenames != NULL) {
		num = g_strv_length (filenames);
		for (i = 0; i < num; ++i) {
			GST_LOG ("command line argument: %s", filenames[i]);
			add_to_playlist (playlist, filenames[i]);
		}
		g_strfreev (filenames);
	}

	num = playlist->len;
	g_ptr_array_add (playlist, NULL);

	uris = (gchar **) g_ptr_array_free (playlist, FALSE);
#endif


//	GstElement * sink = NULL;
    GstElement * playbin = NULL;

	playbin = gst_element_factory_make ("playbin", "playbin");
	if (playbin == NULL)
	{
		g_print(" %s  [ERROR] gst_element_factory_make error happen ! \n", __FUNCTION__);
		return NULL;
	}
	play = g_new0 (GstPlay, 1);
	//	play->uris = uris;
	//	play->num_uris = g_strv_length (uris);
	//	play->cur_idx = -1;
	play->playbin = playbin;
#if  0
	if (audio_sink != NULL) {
		g_warning ("=====================audio_sink != NULL========================== ");
		if (strchr (audio_sink, ' ') != NULL)
			sink = gst_parse_bin_from_description (audio_sink, TRUE, NULL);
		else
			sink = gst_element_factory_make (audio_sink, NULL);

		if (sink != NULL)
			g_object_set (play->playbin, "audio-sink", sink, NULL);
		else
			g_warning ("Couldn't create specified audio sink '%s'", audio_sink);
	}
	else{

		g_warning ("=====================audio_sink == NULL========================== ");
	}

	if (video_sink != NULL) {

		g_warning ("=====================video_sink != NULL========================== ");

		if (strchr (video_sink, ' ') != NULL)
			sink = gst_parse_bin_from_description (video_sink, TRUE, NULL);
		else
			sink = gst_element_factory_make (video_sink, NULL);

		if (sink != NULL)
			g_object_set (play->playbin, "video-sink", sink, NULL);
		else
			g_warning ("Couldn't create specified video sink '%s'", video_sink);
	}
	else{
		g_warning ("=====================video_sink == NULL========================== ");

	}

	if (flags != NULL) {
		GParamSpec *pspec;
		GValue val = { 0, };
		pspec =
			g_object_class_find_property (G_OBJECT_GET_CLASS (playbin), "flags");
		g_value_init (&val, pspec->value_type);
		if (gst_value_deserialize (&val, flags))
			g_object_set_property (G_OBJECT (play->playbin), "flags", &val);
		else
			g_printerr ("Couldn't convert '%s' to playbin flags!\n", flags);
		g_value_unset (&val);
	}
	if (verbose) {
		play->deep_notify_id = g_signal_connect (play->playbin, "deep-notify",
				G_CALLBACK (gst_object_default_deep_notify), NULL);
	}
#endif

	play->loop = g_main_loop_new (NULL, FALSE);
	play->bus_watch = gst_bus_add_watch (GST_ELEMENT_BUS (play->playbin),
			play_bus_msg, play);
	/* FIXME: make configurable incl. 0 for disable */
	//	play->timeout = g_timeout_add (100, play_timeout, play);
	//	play->missing = NULL;
	play->buffering = FALSE;
	play->is_live = FALSE;
	play->desired_state = GST_STATE_PLAYING;

#if  0
	play->gapless = gapless;
	if (gapless) {
		g_signal_connect (play->playbin, "about-to-finish",
				G_CALLBACK (play_about_to_finish), play);
	}
	if (volume != -1)
	{
		play_set_relative_volume (play, volume - 1.0);
	}
#endif


	play->rate = 1.0;
	play->trick_mode = GST_PLAY_TRICK_MODE_NONE;

	play->gst_event_cb = NULL ;
	play->io_source = 0 ;
	memset(play->url, 0, GST_URL_LEN);

	gst_play_pipe_cmd_handler(play);
	gst_main_loop_start(play);

	g_print(" %s %d ===== end end ====\n", __FUNCTION__, __LINE__);

	return play;

}



static void gst_player_register_cb(void * p_handle, void * p_cb)
{
	printf("[%s] start start ...\n", __func__);
	GstPlay * gst_player_handle = (GstPlay *)p_handle;
	gst_player_handle->gst_event_cb = p_cb;
	printf("[%s] end end ...\n", __func__);
    return MT_SUCCESS;
}




static int gst_player_set_url(void * p_handle,const char* purl)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_SET_URL ;
	memset(tmp.url, 0, GST_URL_LEN);
	memcpy(tmp.url, purl, strlen(purl));
	printf("[%s] p_handle = 0x%x ...\n", __func__,p_handle);
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}



static int gst_player_start(void * p_handle)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_START ;
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}



static int gst_player_stop(void * p_handle)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_STOP ;
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}



static int gst_player_pause(void * p_handle)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_PAUSE ;
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}



static int gst_player_resume(void * p_handle)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_RESUME ;
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}



static int gst_player_set_playspeed(void * p_handle,const char speed)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_FASTSPEED ;
	tmp.speed = speed ;
	printf("[%s] p_handle = 0x%x ...\n", __func__,p_handle);
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}



static int gst_player_seek(void * p_handle,const int timems)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_SEEK ;
	tmp.seektime = timems ;
	printf("[%s] p_handle = 0x%x ...\n", __func__,p_handle);
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}

static int gst_player_audio_track_selection(void * p_handle, const int track_id)
{
    gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_AUDIO_SELECT_TRACK ;
	tmp.track_id= track_id ;
	printf("[%s] p_handle = 0x%x ...\n", __func__,p_handle);
	gst_play_cmd_pass(p_handle,&tmp);
    return MT_SUCCESS;
}


static int gst_player_exit(void * p_handle)
{
	gst_cmd_info_t tmp={0};
	tmp.type = GST_PLAY_CMD_EXIT ;
	gst_play_cmd_pass(p_handle,&tmp);

}




/***************************************************************************************
*
*
*
*
*
*
*
*
*                                public  api  for suplayer
*
*
*
*
*
*
*
*
*****************************************************************************************/
static mtUNF_SUPLAYER_STATUS_S suplayer_status[MT_SRV_SUPLAYER_INSTANCE_MAX]={{0}};
static pthread_mutex_t   g_suplayMutex = PTHREAD_MUTEX_INITIALIZER;
#define MT_SUPLAY_LOCK()        (void)pthread_mutex_lock(&g_suplayMutex);
#define MT_SUPLAY_UNLOCK()      (void)pthread_mutex_unlock(&g_suplayMutex);


typedef struct mtUNF_MPLAYER_PRIVATE
{
	MT_U32	load_success;
	MT_HANDLE hdl;
	void *gst_play;
}mtUNF_GST_PLAYER_PRIVATE_S;




MT_VOID* MT_SVR_PLAYER_Init(mt_void* args)
{
	mtUNF_SUPLAYER_IN_ARG_S * in_args = NULL;
	mtUNF_SUPLAYER_STATUS_S * ret = NULL;
	//SUPLAYER_FUNC_S *pfunc;
	MT_U32 i=0,j=0;

	//printf("\n%s %d in_args->ptype[%d]\n", __FUNCTION__, __LINE__, in_args->ptype);

	MT_SUPLAY_LOCK();

	in_args = (mtUNF_SUPLAYER_IN_ARG_S *)args;

	if(in_args->ptype == MT_SUPLAYER_GSTREAMER){

		printf("[%s]\n  MT_SUPLAYER_GSTREAMER !!!\n",__func__);

#if !defined(ONLY_GST_DEBUG)
        audio_decoder_open();
#endif

		for(i=0; i<MT_SRV_SUPLAYER_INSTANCE_MAX; i++){

			if(suplayer_status[i].ptype== MT_SUPLAYER_MPLAYER){
				goto INIT_FAIL2;
			}else if(suplayer_status[i].ptype == MT_SUPLAYER_UNKNOWN){
				j = i;
				if(i == MT_SRV_SUPLAYER_INSTANCE_MAX - 1){
					break;
				}
			}

		}

		if(i == MT_SRV_SUPLAYER_INSTANCE_MAX){
			goto INIT_FAIL2;
		}


		ret = &suplayer_status[j];
		ret->instance = j+1;
		ret->ptype = MT_SUPLAYER_GSTREAMER;
		ret->status = MT_SVR_PLAYER_STATE_INIT;
		ret->pri = NULL;
		//		gst_player_preinit();
		printf("\n[%s]suplayer type=%d,%d\n",__func__,suplayer_status[j].ptype,ret->ptype);
		///init do nothing , just keep the value
	}else{
		//ret->ptype = MT_SUPLAYER_UNKNOWN;
		//ret->status = MT_SVR_PLAYER_STATE_BUTT;
		printf("\n[%s] Unknown Player type!!!\n",__func__);
	}

	MT_SUPLAY_UNLOCK();

	printf("\n%s %d \n", __FUNCTION__, __LINE__);

	return (MT_VOID *)ret;

INIT_FAIL2:
	//mt_free(pfunc);
//INIT_FAIL0:
	MT_SUPLAY_UNLOCK();
	return NULL;
}




MT_S32 MT_SVR_PLAYER_Create(const MT_SVR_PLAYER_PARAM_S *pstruParam, MT_HANDLE *phPlayer)
{
	mtUNF_SUPLAYER_STATUS_S *iargs = (mtUNF_SUPLAYER_STATUS_S *)pstruParam->suplayer_status;
	MT_S32	ret = SUCCESS;
	printf("\n%s %d ptype[%d]\n", __FUNCTION__, __LINE__, iargs->ptype);
	if(iargs->ptype == MT_SUPLAYER_GSTREAMER){

		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		if((MT_HANDLE *)(*phPlayer) == NULL){
			*phPlayer = (MT_HANDLE )mtsu_malloc(sizeof(MT_PLAYBACK_INTERNAL_T));
			if((MT_HANDLE *)(*phPlayer) == NULL){
				goto FAIL0;
			}
			gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S * )mtsu_malloc(sizeof(mtUNF_GST_PLAYER_PRIVATE_S));
			if(gstplayer_pri == NULL){
				goto FAIL1;
			}
			printf("\n%s %d \n", __FUNCTION__, __LINE__);
			memset(gstplayer_pri,0,sizeof(mtUNF_GST_PLAYER_PRIVATE_S));
			gstplayer_pri->hdl = *phPlayer;
			gstplayer_pri->load_success = -1;
			gstplayer_pri->gst_play = NULL;
			suplayer_status[iargs->instance - 1].pri = (MT_VOID*)(gstplayer_pri);
			((MT_PLAYBACK_INTERNAL_T*)(*phPlayer))->suplayer_status = (MT_VOID*)(&suplayer_status[iargs->instance - 1]);

		}else{

			goto FAIL0;

		}
		/////init for player handle
		gstplayer_pri->gst_play = gst_player_init();
		if(gstplayer_pri->gst_play == NULL) {
			printf("file sequence creat error\n");
			goto FAIL1;
		}

		//		gstplayer_pri->gst_play->register_event_cb(gstplayer_pri->gst_play, mplayer_event_callback);

		printf("[%s] file sequence creat ok! \n", __func__);

	}
	else{
	}
	printf("\n%s %d \n", __FUNCTION__, __LINE__);
	return ret;


FAIL1:
	if(phPlayer){
		mtsu_free(phPlayer);
		phPlayer = NULL;
	}


FAIL0:
	ret = ERR_FAILURE;

	return ret;
}





MT_S32 MT_SVR_PLAYER_LOADMEDIA_GetFileInfo(MT_HANDLE hPlayer, MT_FORMAT_FILE_INFO_S **ppstruInfo)
{

	int ret = MT_SUCCESS;

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	printf("\n%s %d \n", __FUNCTION__, __LINE__);

	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){

//		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
//		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		ret = MT_SUCCESS;

	}else{

	}

	printf("\n%s %d \n", __FUNCTION__, __LINE__);

	return ret;
}




MT_S32 MT_SVR_PLAYER_SetMedia(MT_HANDLE hPlayer, MT_U32 eType, MT_SVR_PLAYER_MEDIA_S *pstruMedia)
{

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);

	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	memset(p_MonPlayer->url, 0, URL_LEN);
	memcpy(p_MonPlayer->url, pstruMedia->aszUrl, strlen(pstruMedia->aszUrl));

	//printf("\n%s %d %p %s \n", __FUNCTION__, __LINE__, p_MonPlayer, p_MonPlayer->url);

	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){

		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		gst_player_set_url(gstplayer_pri->gst_play, p_MonPlayer->url);

	}else{

	}
	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}




MT_S32 MT_SVR_PLAYER_Play(MT_HANDLE hPlayer, MT_S64 start_msec)
{

	MT_U32 ret = SUCCESS;

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);


	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	if(phdl->ptype == MT_SUPLAYER_GSTREAMER)
	{

		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_PLAY;
		gst_player_start((void *)gstplayer_pri->gst_play);
		ret = SUCCESS;

	}
	else
	{

	}

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return ret;


}

MT_S32 MT_SVR_PLAYER_GetPlayerInfo(MT_HANDLE hPlayer, MT_SVR_PLAYER_INFO_S *pstruInfo)
{
	printf("\n%s %d ===dont support==\n", __FUNCTION__, __LINE__);
	return MT_SUCCESS;
}



MT_S32 MT_SVR_PLAYER_Stop(MT_HANDLE hPlayer)
{

	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	MT_S32 ret = MT_SUCCESS;
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);


	printf("[%s] zx phdl = 0x%x ...\n", __func__,phdl);


	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_STOP;
		gst_player_stop((void *)gstplayer_pri->gst_play);
		ret = SUCCESS;
	}else{

	}

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return ret;
}



MT_S32 MT_SVR_PLAYER_Destroy(MT_HANDLE hPlayer)
{

	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);
    MT_S32 i = 0;
//	MT_S32 ret = MT_SUCCESS;
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	if(hPlayer == NULL){
		printf("\n%s [ERROR] =hPlayer == NULL!!!!\n", __FUNCTION__);
		return MT_SUCCESS;
	}

	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl == NULL){

		printf("\n%s [ERROR] =phdl == NULL!!!!\n", __FUNCTION__);
		return MT_SUCCESS;

	}

	printf("[%s] zx phdl = 0x%x ...\n", __func__,phdl);

	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){

		mtUNF_GST_PLAYER_PRIVATE_S * gstplayer_pri;

		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_STOP;
		gst_player_exit((void *)gstplayer_pri->gst_play);

		mtsu_free((mtUNF_GST_PLAYER_PRIVATE_S *)gstplayer_pri);
		gstplayer_pri = NULL;

		mtsu_free((MT_PLAYBACK_INTERNAL_T *)hPlayer);
		hPlayer = NULL;

		//printf("[%s] zx hPlayer = 0x%x ...\n", __func__,hPlayer);

	}else{

	}

    for(i=0; i<MT_SRV_SUPLAYER_INSTANCE_MAX; i++)
        suplayer_status[i].ptype = MT_SUPLAYER_UNKNOWN;

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);


	return MT_SUCCESS;
}




MT_S32 MT_SVR_PLAYER_RegCallback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_FN pfnCallback)
{
	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);

	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){

		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		gst_player_register_cb((void *)gstplayer_pri->gst_play, pfnCallback);

	}else{

	}


	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}





MT_S32 MT_SVR_PLAYER_Pause(MT_HANDLE hPlayer)
{
	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_GSTREAMER)
	{
		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		gst_player_pause((void *)gstplayer_pri->gst_play);
	}
	else
	{

	}

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;

}




MT_S32 MT_SVR_PLAYER_Resume(MT_HANDLE hPlayer)
{
	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		gst_player_resume((void *)gstplayer_pri->gst_play);
	}else{


	}

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);


	return MT_SUCCESS;
}





MT_S32 MT_SVR_PLAYER_TPlay(MT_HANDLE hPlayer, MT_S32 s32Speed)
{
	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);

	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		gst_player_set_playspeed((void *)gstplayer_pri->gst_play,s32Speed);
	}else{
	}


	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}





MT_S32 MT_SVR_PLAYER_Seek(MT_HANDLE hPlayer, mt_s64 s64TimeInMs)
{
	printf("\n%s %d ===start start=%lld=\n", __FUNCTION__, __LINE__,s64TimeInMs);

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){

		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		//int t_sec = (MT_U32)s64TimeInMs/1000;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		gst_player_seek((void *)gstplayer_pri->gst_play, s64TimeInMs);
	}
	else
	{


	}

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}






MT_S32 MT_SVR_PLAYER_SeekPos(MT_HANDLE hPlayer, mt_s64 s64Offset)
{

#if 0 //not support now
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MPLAYER){
		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		//int t_sec = (MT_U32)s64TimeInMs/1000;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		gstplayer_pri->gst_play->play_at_time((void *)gstplayer_pri->gst_play, t_sec);
	}else if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
	}else{
	}
#endif

	return MT_SUCCESS;
}






MT_S32 MT_SVR_PLAYER_Deinit(MT_HANDLE hPlayer)
{

	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	MT_SUPLAY_LOCK();
	MT_SUPLAY_UNLOCK();

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);


	return 0;
}








MT_S32 MT_SVR_PLAYER_SetParam(MT_HANDLE hPlayer, MT_SVR_PLAYER_ATTR_E eAttrId, const MT_VOID *pArg)
{
	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
//		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
//		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		switch(eAttrId){
			case MT_SVR_PLAYER_ATTR_STREAMID:
				{
//					MT_SVR_PLAYER_STREAMID_S *tmp = (MT_SVR_PLAYER_STREAMID_S *)pArg;
					//gstplayer_pri->gst_play->change_audio_track((void *)gstplayer_pri->gst_play, tmp->u16AudStreamId);
					//gstplayer_pri->gst_play->change_video_track((void *)gstplayer_pri->gst_play, tmp->u16VidStreamId);
					//gstplayer_pri->gst_play->set_subt_id((void *)gstplayer_pri->gst_play, tmp->u16VidStreamId);
				}
				break;
			case MT_SVR_PLAYER_ATTR_SYNC:
				break;
			default:
				break;
		}
	}else{


	}

	printf("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);
	return MT_SUCCESS;
}








MT_S32 MT_SVR_PLAYER_GetParam(MT_HANDLE hPlayer, MT_SVR_PLAYER_ATTR_E eAttrId, MT_VOID *pArg)
{

	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
//		mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
//		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
		switch(eAttrId){
			case MT_SVR_PLAYER_ATTR_STREAMID:
				{
					MT_SVR_PLAYER_STREAMID_S *tmp = (MT_SVR_PLAYER_STREAMID_S *)pArg;
					//					FILM_INFO_T film;
					if(tmp == NULL) return MT_FAILURE;
					//					gstplayer_pri->gst_play->get_film_info((void *)gstplayer_pri->gst_play, &film);
					//					tmp->u16AudStreamId = film.audio_track_id;
					//					tmp->u16VidStreamId = film.video_track_num;
					//					tmp->u16SubStreamId = gstplayer_pri->gst_play->subt_id;
				}
				break;
			case MT_SVR_PLAYER_ATTR_SYNC:
				break;
			default:
				break;
		}


	}/*else if(phdl->ptype == MT_SUPLAYER_GSTREAMER){           // for tsscan


	}else{


	}
*/
	printf("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	return MT_SUCCESS;
}


MT_S32 MT_SVR_PLAYER_Set_Subtitle(MT_HANDLE hPlayer, int sub_id)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    phdl =(mtUNF_SUPLAYER_STATUS_S *)(((MT_PLAYBACK_INTERNAL_T *)hPlayer)->suplayer_status);
    printf("\n%s_%d:set subtitle_id=%d\n",__FUNCTION__,__LINE__,sub_id);
    if(phdl->ptype == MT_SUPLAYER_MPLAYER){
        printf("\n%s_%d: here is gstreamer\n",__FUNCTION__,__LINE__);
        return MT_FAILURE;
    }else if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
    }else{
    }
    return MT_SUCCESS;
}

MT_S32 MT_SVR_DUMP_VIDEO_DATA(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    phdl =(mtUNF_SUPLAYER_STATUS_S *)(((MT_PLAYBACK_INTERNAL_T *)hPlayer)->suplayer_status);

    printf("\n%s_%d\n",__FUNCTION__,__LINE__);
    if(phdl->ptype == MT_SUPLAYER_MPLAYER){
        printf("\n%s_%d: here is gstreamer\n",__FUNCTION__,__LINE__);
        return MT_FAILURE;
    }else if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
    }else{
    }
    return MT_SUCCESS;
}

MT_S32 MT_SVR_DUMP_AUDIO_DATA(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    phdl =(mtUNF_SUPLAYER_STATUS_S *)(((MT_PLAYBACK_INTERNAL_T *)hPlayer)->suplayer_status);

    printf("\n%s_%d\n",__FUNCTION__,__LINE__);
    if(phdl->ptype == MT_SUPLAYER_MPLAYER){
        printf("\n%s_%d: here is gstreamer\n",__FUNCTION__,__LINE__);
        return MT_FAILURE;
    }else if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
    }else{
    }
    return MT_SUCCESS;
}


MT_S32 MT_SVR_PLAYER_Set_Aud_Track(MT_HANDLE hPlayer, int track_id)
{
	mtUNF_SUPLAYER_STATUS_S *phdl;
	phdl =(mtUNF_SUPLAYER_STATUS_S *)(((MT_PLAYBACK_INTERNAL_T *)hPlayer)->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MPLAYER){
		printf("\n%s_%d: here is gstreamer\n",__FUNCTION__,__LINE__);
	}else if(phdl->ptype == MT_SUPLAYER_GSTREAMER){
	    mtUNF_GST_PLAYER_PRIVATE_S *gstplayer_pri;
		//int t_sec = (MT_U32)s64TimeInMs/1000;
		gstplayer_pri = (mtUNF_GST_PLAYER_PRIVATE_S *)phdl->pri;
	    gst_player_audio_track_selection(gstplayer_pri->gst_play, track_id);
	}else{
	}
	return MT_SUCCESS;
}


//just for compile
void  update_BPS(void)
{
    //do nothing
}

#define CFG_ENABLE_FFMPEG_422
#ifdef CFG_ENABLE_FFMPEG_422
void * malloc33(u32 size)
{
    void * buf = NULL;
    return buf;
}

void * realloc33(void * old_ptr, u32 size)
{
    void * new_ptr = NULL;
    return new_ptr;
}

void free33(void * ptr)
{
}

char * strdup33(const char * s)
{
    char * new_string = NULL;
    return new_string;
}

void * calloc33(u32 n, u32 size)
{
    void * buf = NULL;
    return buf;
}
#endif

#endif
