/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_comp_static.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player's static components.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <inttypes.h>
#include <string.h>

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"

extern struct mtlzplayer_comp_source_st file_source_comp;
#ifdef MTLZ_DMX_SOURCE_SUPPORT
extern struct mtlzplayer_comp_source_st demux_source_comp;
#endif
extern struct mtlzplayer_comp_codec_st mpeg2_codec_comp;
extern struct mtlzplayer_comp_sink_st fake_sink_comp;
#ifdef MTLZ_DISPLAY_SINK_SUPPORT
extern struct mtlzplayer_comp_sink_st display_sink_comp;
#endif

#ifdef FFMPEG_SUPPORT
extern struct mtlzplayer_comp_codec_st ffmpeg_mpeg2_codec_comp;
extern struct mtlzplayer_comp_codec_st ffmpeg_mpeg4_codec_comp;
extern struct mtlzplayer_comp_codec_st ffmpeg_h264_codec_comp;
extern struct mtlzplayer_comp_codec_st ffmpeg_h265_codec_comp;
#endif

extern struct mtlzplayer_comp_packer_st mpeg4_packer_comp;
extern struct mtlzplayer_comp_packer_st h264_packer_comp;
extern struct mtlzplayer_comp_packer_st h265_packer_comp;

//[index][type]
//index:
//    MTLZ_COMP_SOURCE_INDEX: source
//    MTLZ_COMP_CODEC_INDEX: codec
//    MTLZ_COMP_SINK_INDEX: sink
//    MTLZ_COMP_PACKER_INDEX: packer
//type:
//    demux/mpeg2/display/...
static ulong g_comp_array[][64] =
{
	/* enum mtlz_comp_source_index_e */
	{ (ulong)&file_source_comp,
#ifdef MTLZ_DMX_SOURCE_SUPPORT
	  (ulong)&demux_source_comp,
#endif
	  0 },

	/* enum mtlz_comp_codec_index_e */
	{
#if 1
	  (ulong)&mpeg2_codec_comp,
#else
	  (uint32_t)&ffmpeg_mpeg2_codec_comp,
#endif

#ifdef FFMPEG_SUPPORT
	  (ulong)&ffmpeg_mpeg4_codec_comp,
      (ulong)&ffmpeg_h264_codec_comp,
      (ulong)&ffmpeg_h265_codec_comp,
#else
	  0,
	  0,
	  0,
#endif
	  0 },

	/* enum mtlz_comp_sink_index_e */
	{ (ulong)&fake_sink_comp,
#ifdef MTLZ_DISPLAY_SINK_SUPPORT
	  (ulong)&display_sink_comp,
#endif
	  0 },

	/* Packer: enum mtlz_comp_codec_index_e */
	{ 0, /* MPEG2 */
	  (ulong)&mpeg4_packer_comp,
 	  (ulong)&h264_packer_comp,
	  (ulong)&h265_packer_comp,
 	  0 },
};

//url:
//    demux: "demuxer=dmx,0", or "demuxer=demux,0"
//    file: others
enum mtlz_comp_source_index_e get_source_type_index(const char *url)
{
	char *source;

	source = strstr(url, "demuxer=");
	if (source != NULL
		&& (strstr(source, "dmx") != NULL
			|| strstr(source, "demux") != NULL))
	{
		return MTLZ_COMP_SOURCE_DMX_INDEX;
	}
	else
	{
		//FIXME: how about other url?
		return MTLZ_COMP_SOURCE_FILE_INDEX;
	}
}

//url:
//    "video=201,mpeg2"
enum mtlz_comp_codec_index_e get_codec_type_index(const char *url)
{
	char *str;
	char *szCodec = "mpeg2";

	str = strstr(url, "video=");
	if (str != NULL)
	{
		str = strchr(str, ',');
		if (str != NULL)
			szCodec = str + 1;
	}
	else
	{
		/* file extention, such as "*.m2v" */
		str = strrchr(url, '.');
		if (str != NULL)
			szCodec = str + 1;
	}

	if (strncmp(szCodec, "mpeg2", 5) == 0
		|| strncmp(szCodec, "mpeg1", 5) == 0
		|| strncmp(szCodec, "mpeg", 4) == 0
		|| strncmp(szCodec, "mpg", 3) == 0
		|| strncmp(szCodec, "m2v", 3) == 0)
	{
		return MTLZ_COMP_CODEC_MPEG2_INDEX;
	}
	else if (strncmp(szCodec, "mpeg4", 5) == 0
			 || strncmp(szCodec, "mp4", 3) == 0
			 || strncmp(szCodec, "m4v", 3) == 0)
	{
		return MTLZ_COMP_CODEC_MPEG4_INDEX;
	}
/* Not Support */
#if 0
	else if (strncmp(szCodec, "avs", 3) == 0)
	{
		return MTLZ_COMP_CODEC_AVS_INDEX;
	}
	else if (strncmp(szCodec, "h263", 4) == 0)
	{
		return MTLZ_COMP_CODEC_H263_INDEX;
	}
#endif
	else if (strncmp(szCodec, "h264", 4) == 0 || strncmp(szCodec, "avc", 3) == 0)
	{
		return MTLZ_COMP_CODEC_H264_INDEX;
	}
	else if (strncmp(szCodec, "h265", 4) == 0 || strncmp(szCodec, "hevc", 4) == 0)
	{
		return MTLZ_COMP_CODEC_H265_INDEX;
	}
/* Not Support */
#if 0
	else if (strncmp(szCodec, "vc1", 3) == 0)
	{
		return MTLZ_COMP_CODEC_VC1_INDEX;
	}
	else if (strncmp(szCodec, "vp6", 3) == 0)
	{
		return MTLZ_COMP_CODEC_VP6_INDEX;
	}
	else if (strncmp(szCodec, "vp8", 3) == 0)
	{
		return MTLZ_COMP_CODEC_VP6_INDEX;
	}
#endif
	else
	{
		MTLZ_ERROR("[ERROR]%s: unknown codec(%s)!\n",__FUNCTION__,url);
	}

	return MTLZ_COMP_CODEC_BUT;
}

//url:
//    "sink=display,0"
enum mtlz_comp_sink_index_e get_sink_type_index(const char *url)
{
	char *str;
	char *szSink = "display";

	str = strstr(url, "sink=");
	if (str != NULL)
		szSink = str + 5;

	if (strncmp(szSink, "display", 7) == 0)
	{
		return MTLZ_COMP_SINK_DISPLAY_INDEX;
	}
	else
	{
		MTLZ_ERROR("[ERROR]%s: unknown sink(%s)!\n",__FUNCTION__,url);
	}

	return MTLZ_COMP_SINK_FAKE_INDEX;
}

ulong mtlz_comp_get(int index, int type)
{
	return g_comp_array[index][type];
}

