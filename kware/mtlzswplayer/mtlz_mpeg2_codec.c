/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_mpeg2_codec.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : MPEG2 SW Codec for Monage-LZ SW Player.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
//#include <stdio.h>
#include <inttypes.h>

#include "mpeg2.h"

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"
#include "mtlzplayer_os.h"

#define MAX_DECODER_COUNT	1

static struct mpeg2_decoder_st
{
	mpeg2dec_t *decoder;

	mpeg2_state_t last_state;

	//debug
	unsigned long data_size;
	unsigned long frm_cnt;
	unsigned long last_tick;

} mpeg2_decoders[MAX_DECODER_COUNT];

#define CHECK_ID(id)		MTLZ_ASSERT(id >= 0 && id < MAX_DECODER_COUNT)
#define GET_DEC(id)			&mpeg2_decoders[id]

//0: YUV420
//1: YUV422
//2: YUV444
static inline int get_format(unsigned int width, unsigned int height, unsigned int chroma_width, unsigned int chroma_height)
{
	return ((chroma_width==width?1:0) + (chroma_height==height?1:0));
}

static int mpeg2_init_wrap(void)
{
	MTLZ_DEBUG("Call mpeg2_init\n");

	mpeg2_decoders[0].decoder 		= mpeg2_init();
	if (mpeg2_decoders[0].decoder == NULL)
	{
		MTLZ_ERROR("[ERROR]%s: mpeg2_init failed!\n",__FUNCTION__);
		return MTLZ_INVALID_ID;
	}
	mpeg2_decoders[0].last_state 	= STATE_BUFFER;
	mpeg2_decoders[0].data_size 	= 0;
	mpeg2_decoders[0].frm_cnt 		= 0;
	mpeg2_decoders[0].last_tick 	= 0;

	return 0;
}

static int mpeg2_close_wrap(int id)
{
	mpeg2dec_t *decoder;
	struct mpeg2_decoder_st *dec;

	MTLZ_DEBUG("Call mpeg2_close(%d)\n",id);

	CHECK_ID(id);

	dec = GET_DEC(id);
	decoder = dec->decoder;

	mpeg2_close(decoder);

	return MTLZ_SUCCESS;
}

/**
 * @brief decode the buffer, and output one decoded frame
 *
 * @param[in] id codec id
 * @param[in] packet input buffer and size if needed.
 * @param[out] frame output decodec one frame if available.
 *
 * @retval
 *    MTLZ_CODEC_RET_BUFFER: buffer empty.
 *    MTLZ_CODEC_RET_FRAME: decoded one frame, and some bytes still left,
 *                          need call again(no need input buffer) till buffer empty.
 *    MTLZ_CODEC_RET_UNKNOWN_ERROR: unknown error.
 */
static int mpeg2_decode(int id, const struct mtlzplayer_packet_st *packet, struct mtlzplayer_frame_st *frame)
{
	mpeg2dec_t *decoder;
    const mpeg2_info_t *info;
    const mpeg2_sequence_t *sequence;
    mpeg2_state_t state;

	struct mpeg2_decoder_st *dec;

	CHECK_ID(id);

	dec = GET_DEC(id);
	decoder = dec->decoder;
	state = dec->last_state;

	/* last state */
	if (state == STATE_BUFFER)
	{
		MTLZ_VERBOSE("%s: last state=STATE_BUFFER\n",__FUNCTION__);

		if (packet == NULL || packet->data == NULL || packet->size == 0)
		{
			MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
			return MTLZ_CODEC_RET_BUFFER;
		}

		mpeg2_buffer(decoder, packet->data, packet->data + packet->size);
		dec->data_size += packet->size;
		MTLZ_VERBOSE("%s: total data_size=%lu\n",__FUNCTION__,dec->data_size);
	}

	//FIXME: how about dead loop?!
	for (;;)
	{
		state = mpeg2_parse(decoder);
		dec->last_state = state;

		switch (state)
		{
			case STATE_BUFFER:
				/* next call */
				MTLZ_VERBOSE("%s: state=STATE_BUFFER\n",__FUNCTION__);
				return MTLZ_CODEC_RET_BUFFER;
			    break;
			case STATE_SLICE:
			case STATE_END:
			case STATE_INVALID_END:
				info = mpeg2_info(decoder);

			    if (info->display_fbuf)
				{
					sequence = info->sequence;

					frame->buf_y 			= info->display_fbuf->buf[0];
					frame->buf_u 			= info->display_fbuf->buf[1];
					frame->buf_v 			= info->display_fbuf->buf[2];
					frame->line_stride		= sequence->width;	//FIXME?
					frame->width 			= sequence->width;
					frame->height 			= sequence->height;
					frame->chroma_width 	= sequence->chroma_width;
					frame->chroma_height 	= sequence->chroma_height;
					frame->format 			= get_format(sequence->width, sequence->height,
				  										sequence->chroma_width, sequence->chroma_height);
					frame->pts 				= 0;	/* not support yet! */

					dec->frm_cnt ++;
					dec->last_tick = mtlz_get_tick();

					MTLZ_VERBOSE("%s: decoded frame[%lu@%lu](%p, %d, %d, %d, %d), state=%d\n",__FUNCTION__,
						dec->frm_cnt,
						dec->last_tick,
						frame->buf_y,
						frame->width, frame->height,
						frame->chroma_width, frame->chroma_height,
						state);

					return MTLZ_CODEC_RET_FRAME;
				}
			    break;
			default:
			    break;
		}
	}

	MTLZ_ERROR("[ERROR]%s@%d: unknow error, should not happen!\n",__FUNCTION__,__LINE__);
	return MTLZ_CODEC_RET_UNKNOWN_ERROR;
}

struct mtlzplayer_comp_codec_st mpeg2_codec_comp =
{
	.init 	= mpeg2_init_wrap,
	.close 	= mpeg2_close_wrap,
	.decode = mpeg2_decode,
};

