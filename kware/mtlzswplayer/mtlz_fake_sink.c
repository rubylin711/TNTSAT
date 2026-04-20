/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_fake_sink.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/18
 * Description    : Fake Sink for Monage-LZ SW Player.
 * History        :
 * 1.Date         : 2019/03/18
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include "mtlz_types.h"
#include "mtlzplayer_comp.h"

static int fake_sink_setup(int pic_width, int pic_height)
{
	MTLZ_DEBUG("%s: w %d, h %d\n",__FUNCTION__,pic_width,pic_height);
	return 0;
}

static int fake_sink_teardown(int sink)
{
	MTLZ_DEBUG("%s: handle %d\n",__FUNCTION__,sink);
	return MTLZ_SUCCESS;
}

static int fake_sink_draw(int sink, const struct mtlzplayer_frame_st *frame)
{
	MTLZ_DEBUG("%s: y %p u %p v %p\n s %d w %d h %d cw %d ch %d\n fmt %d\n pts %llu\n",__FUNCTION__,
		frame->buf_y,
		frame->buf_u,
		frame->buf_v,
		frame->line_stride,
		frame->width,
		frame->height,
		frame->chroma_width,
		frame->chroma_height,
		frame->format,
		frame->pts);

	return MTLZ_SUCCESS;
}

struct mtlzplayer_comp_sink_st fake_sink_comp =
{
	.setup 		= fake_sink_setup,
	.teardown 	= fake_sink_teardown,
	.draw 		= fake_sink_draw,
};

