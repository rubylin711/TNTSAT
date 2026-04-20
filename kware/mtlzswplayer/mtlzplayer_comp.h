/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : mtlzplayer_comp.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player Component Definition.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __MTLZ_PLAYER_COMP_H__
#define __MTLZ_PLAYER_COMP_H__

#include <stddef.h>
#include <sys/types.h>
#include <inttypes.h>

#ifdef __cplusplus
#if __cplusplus
	 extern "C"{
#endif
#endif /* __cplusplus */

/* Component Index */
enum mtlz_comp_index_e
{
	MTLZ_COMP_SOURCE_INDEX = 0,
	MTLZ_COMP_CODEC_INDEX,
	MTLZ_COMP_SINK_INDEX,
	MTLZ_COMP_PACKER_INDEX,
};

/* Component Source Type Index */
enum mtlz_comp_source_index_e
{
	MTLZ_COMP_SOURCE_FILE_INDEX = 0,
	MTLZ_COMP_SOURCE_DMX_INDEX,

	MTLZ_COMP_SOURCE_BUTT,
};

/* Component Codec Type Index */
enum mtlz_comp_codec_index_e
{
	MTLZ_COMP_CODEC_MPEG2_INDEX = 0,
	MTLZ_COMP_CODEC_MPEG4_INDEX,
	MTLZ_COMP_CODEC_H264_INDEX,
	MTLZ_COMP_CODEC_H265_INDEX,

/* Not Support */
//	MTLZ_COMP_CODEC_H263_INDEX,
//	MTLZ_COMP_CODEC_AVS_INDEX,
//	MTLZ_COMP_CODEC_VC1_INDEX,
//	MTLZ_COMP_CODEC_VP6_INDEX,
//	MTLZ_COMP_CODEC_VP8_INDEX,

	MTLZ_COMP_CODEC_BUT,
};

/* Component Sink Type Index */
enum mtlz_comp_sink_index_e
{
	MTLZ_COMP_SINK_FAKE_INDEX = 0,
	MTLZ_COMP_SINK_DISPLAY_INDEX,

	MTLZ_COMP_SINK_BUTT,
};

/**
 * @brief Montage-LZ SW Player Packet
 */
struct mtlzplayer_packet_st
{
	unsigned char *data;
	unsigned int size;
};

/**
 * @brief Montage-LZ SW Player Frame
 */
struct mtlzplayer_frame_st
{
	void *buf_y;
	void *buf_u;
	void *buf_v;

	unsigned int line_stride;
	unsigned int width;
	unsigned int height;

	unsigned int chroma_width;
	unsigned int chroma_height;

	/* 0: YUV420 */
	/* 1: YUV422 */
	/* 2: YUV444 */
	int format;

	unsigned long long pts;
};

/**
 * @brief Montage-LZ SW Player Source Component
 */
struct mtlzplayer_comp_source_st
{
	ulong (*open)(const char *path, int oflag);
	int (*close)(ulong fd);
	int (*eos)(ulong fd);
	ssize_t (*read)(ulong fd, void *buf, size_t count);
};

/**
 * @brief Montage-LZ SW Player Packer Filter Component
 */
struct mtlzplayer_comp_packer_st
{
	ulong (*init)(void);
	int (*destroy)(ulong fd);

	int (*get_buffer)(ulong fd, unsigned char **buf, unsigned int *size);
	int (*put_buffer)(ulong fd, unsigned char *buf, unsigned int size);
	int (*read_packet)(ulong fd, struct mtlzplayer_packet_st *pkt, int last_packet);
};

/**
 * @brief Montage-LZ SW Player Codec Component
 */
struct mtlzplayer_comp_codec_st
{
	int (*init)(void);
	int (*close)(int decoder);
	int (*decode)(int decoder, const struct mtlzplayer_packet_st *pkt, struct mtlzplayer_frame_st *frame);
};

/**
 * @brief Montage-LZ SW Player Sink Component
 */
struct mtlzplayer_comp_sink_st
{
	int (*setup)(int pic_width, int pic_height);
	int (*teardown)(int sink);
	int (*draw)(int sink, const struct mtlzplayer_frame_st *frame);
};

enum mtlz_comp_source_index_e get_source_type_index(const char *url);
enum mtlz_comp_codec_index_e get_codec_type_index(const char *url);
enum mtlz_comp_sink_index_e get_sink_type_index(const char *url);
ulong mtlz_comp_get(int index, int type);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTLZ_PLAYER_COMP_H__ */

