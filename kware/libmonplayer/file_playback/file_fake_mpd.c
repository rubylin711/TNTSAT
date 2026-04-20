/********************************************************************************************/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MODULE_TAG "FP"

#include "mutil.h"
#include "mlog.h"

#include "libmpdemux/stheader.h"
#include "mt_unf_avplay.h"
#include "libmpdemux/demuxer.h"
#include "file_playback_sequence.h"
#include "demux_mp.h"

#include "file_fake_mpd.h"


#define FAKE_DASH_URL  "http://__fake_dash.mpd"

static const char* fake_mpd_template =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
    "<MPD mediaPresentationDuration=\"PT%lldS\" profiles=\"urn:mpeg:dash:profile:isoff-on-demand:2011\" type=\"static\" xmlns=\"urn:mpeg:dash:schema:mpd:2011\">"
        "<Period>\n"
            "<AdaptationSet mimeType=\"audio/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n"
                "<Representation audioSamplingRate=\"44100\" bandwidth=\"141962\" codecs=\"mp4a.40.2\" id=\"audio-und\">\n"
                    "<BaseURL>%s</BaseURL>\n"
                "</Representation>\n"
            "</AdaptationSet>\n"
                "<AdaptationSet mimeType=\"video/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n"
                "<Representation bandwidth=\"8000000\" codecs=\"avc1.42C01F\" height=\"1080\" width=\"1920\">\n"
                "<BaseURL>%s</BaseURL>\n"
                "</Representation>\n"
            "</AdaptationSet>\n"
        "</Period>\n"
    "</MPD>\n";

static const char *esc_map(int val, size_t *len) /* I - Character value */
{
    switch (val)
    {
    case '&':
        if (len)
        {
            *len = 5;
        }
        return ("&amp;");
    case '<':
        if (len)
        {
            *len = 4;
        }
        return ("&lt;");
    case '>':
        if (len)
        {
            *len = 4;
        }
        return ("&gt;");
    case '\"':
        if (len)
        {
            *len = 6;
        }
        return ("&quot;");
    default:
        if (len)
        {
            *len = 0;
        }
        return (NULL);
    }
}
static char* make_xml_style_url(const char* url)
{
    if (!url) {
        return NULL;
    }

    size_t str_len = strlen(url);
    size_t i, j;
    size_t extra_size = 0;
    size_t escape_size = 0;
    for (i = 0; i < str_len; i++) {
        const char* escape_ch = esc_map(url[i], &escape_size);
        if (escape_ch) {
            extra_size += escape_size - 1;
        }
    }

    size_t new_str_len = str_len + extra_size;
    char *new_url = (char*)malloc(new_str_len + 1);
    if (!new_url) {
        return NULL;
    }
    for (i = 0, j = 0; i < str_len; i++) {
        const char* escape_ch = esc_map(url[i], &escape_size);
        if (escape_ch) {
            memcpy(&new_url[j], escape_ch, escape_size);
            j += escape_size;
        } else {
            new_url[j++] = url[i];
        }
    }
    new_url[j] = 0;

    MLOGD("url:%s\n", url);
    MLOGD("new_url:%s\n", new_url);
    return new_url;
}


char *set_dash_playurl(long long duration, char *audio_url, char *video_url)
{
    char *ret_url = NULL;
    char *new_audio_url = make_xml_style_url(audio_url);
    if (!new_audio_url) {
        MLOGE("[%s %d]audio_url is NULL.\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    char *new_video_url = make_xml_style_url(video_url);
    if (!new_video_url) {
        free(new_audio_url);
        MLOGE("[%s %d]video_url is NULL.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    // 64bit num max is 19bytes, here add 24 bytes for duration
    size_t len = strlen(fake_mpd_template) + strlen(new_audio_url) + strlen(new_video_url) + 24;
    char* fake_mpd_str = malloc(len);
    do {
        if (!fake_mpd_str) {
            ret_url = NULL;
            break;
        }
        snprintf(fake_mpd_str, len, fake_mpd_template, duration, new_audio_url, new_video_url);
        DASH_FAKE_MPD mpd = {0};
        mpd.url = FAKE_DASH_URL;
        mpd.mpd_str = fake_mpd_str;
        MLOGD("[%s %d]mpd.url: %s\n", __FUNCTION__, __LINE__, mpd.url);
        MLOGD("[%s %d]mpd.mpd_str:%s\n", __FUNCTION__, __LINE__, mpd.mpd_str);
        mp_ffmpeg_ext_cmd(MP_FFMPEG_SET_FAKE_MPD, MP_DO_SET_PARAM, (void*)&mpd);
        ret_url = FAKE_DASH_URL;
    }while (0);

    if (new_audio_url) {
        free(new_audio_url);
    }
    if (new_video_url) {
        free(new_video_url);
    }
    if (fake_mpd_str) {
        free(fake_mpd_str);
    }
    return ret_url;
}

