/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_ffmpeg_codec.c
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
#include <libavcodec/avcodec.h>

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"

#define MAX_DECODER_COUNT	1

static int ffmpeg_inited = 0;

static struct ffmpeg_decoder_st
{
	enum AVCodecID codec_id;
    AVCodec *codec;
    AVCodecContext *ctx;
    AVFrame *frame;

    AVPacket avpkt;

	int last_retval;	/* last decode return value */

	//debug
	unsigned long frm_cnt;
	unsigned long last_tick;

} ffmpeg_decoders[MAX_DECODER_COUNT];

#define CHECK_ID(id)		MTLZ_ASSERT(id >= 0 && id < MAX_DECODER_COUNT)
#define GET_DEC(id)			&ffmpeg_decoders[id]

static int __ffmpeg_init(enum AVCodecID codec_id)
{
	//FIXME: how about multi-codec?
	int id = 0;
    AVCodec *codec = NULL;
    AVCodecContext *ctx = NULL;
    AVFrame *frame = NULL;
    AVPacket *avpkt = &ffmpeg_decoders[0].avpkt;

	MTLZ_DEBUG("Call avcodec_open2(Codec=%d)\n",codec_id);

	if (!ffmpeg_inited)
	{
	    /* register all the codecs */
	    avcodec_register_all();
		ffmpeg_inited = 1;
	}

	av_init_packet(avpkt);
    avpkt->data = NULL;
    avpkt->size = 0;

    codec = avcodec_find_decoder(codec_id);
    if (!codec)
	{
        MTLZ_ERROR("Codec not found\n");
        goto FAILED;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx)
	{
        MTLZ_ERROR("Could not allocate video codec context\n");
        goto FAILED;
    }

#ifdef FFMPEG_V2
	if(codec->capabilities&CODEC_CAP_TRUNCATED)
		ctx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
#else
	if(codec->capabilities&AV_CODEC_CAP_TRUNCATED)
		ctx->flags|= AV_CODEC_CAP_TRUNCATED; /* we do not send complete frames */
#endif

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    if (avcodec_open2(ctx, codec, NULL) < 0)
	{
        MTLZ_ERROR("Could not open codec\n");
        goto FAILED;
    }

    frame = av_frame_alloc();
    if (!frame)
	{
        MTLZ_ERROR("Could not allocate video frame\n");
        goto FAILED;
    }

	ffmpeg_decoders[0].codec_id			= codec_id;
	ffmpeg_decoders[0].codec 			= codec;
	ffmpeg_decoders[0].ctx 				= ctx;
	ffmpeg_decoders[0].frame 			= frame;

	ffmpeg_decoders[0].last_retval  	= MTLZ_CODEC_RET_BUFFER;
	ffmpeg_decoders[0].frm_cnt 			= 0;
	ffmpeg_decoders[0].last_tick 		= 0;

	return id;

FAILED:
	if (ctx != NULL)
	{
		avcodec_close(ctx);
		av_free(ctx);
	}
	if (frame != NULL)
		av_frame_free(&frame);

	return MTLZ_INVALID_ID;
}

static int ffmpeg_mpeg2_init(void)
{
	return __ffmpeg_init(AV_CODEC_ID_MPEG2VIDEO);
}

static int ffmpeg_mpeg4_init(void)
{
	return __ffmpeg_init(AV_CODEC_ID_MPEG4);
}

static int ffmpeg_h264_init(void)
{
	return __ffmpeg_init(AV_CODEC_ID_H264);
}

static int ffmpeg_h265_init(void)
{
	return __ffmpeg_init(AV_CODEC_ID_HEVC);
}

static int ffmpeg_close(int id)
{
	int ret;
	struct ffmpeg_decoder_st *dec;
    AVCodecContext *ctx;
    AVFrame *frame;

	MTLZ_DEBUG("Call avcodec_close\n");

	CHECK_ID(id);

	dec = GET_DEC(id);

	ret = avcodec_close(dec->ctx);
	if (ret != 0)
	{
		MTLZ_ERROR("[ERROR]%s: avcodec_close(%p) failed!\n",__FUNCTION__,dec->ctx);
	}
	av_free(dec->ctx);
	av_frame_free(&dec->frame);

	return MTLZ_SUCCESS;
}

/* Patch: h265 exception! */
#if 1
static unsigned char tmp_buffer[MAX_PACKET_SIZE+PACKET_PADDING_SIZE];
#endif

/**
 * @brief decode the buffer, and output one decoded frame
 *
 * @param[in] id codec id
 * @param[in] packet input buffer and size if needed.
 * @param[out] out_frame output decodec one frame if available.
 *
 * @retval
 *    MTLZ_CODEC_RET_BUFFER: buffer empty.
 *    MTLZ_CODEC_RET_FRAME: decoded one frame, and some bytes still left,
 *                          need call again(no need input buffer) till buffer empty.
 *    MTLZ_CODEC_RET_UNKNOWN_ERROR: unknown error.
 */
static int ffmpeg_decode(int id, struct mtlzplayer_packet_st *packet, struct mtlzplayer_frame_st *out_frame)
{
	int ret = MTLZ_CODEC_RET_BUFFER;
	struct ffmpeg_decoder_st *dec;
	AVCodecContext *avctx;
	AVFrame *frame;
	AVPacket *pkt;
    int len, got_frame;

	CHECK_ID(id);

	dec = GET_DEC(id);
	avctx = dec->ctx;
	frame = dec->frame;
	pkt = &dec->avpkt;

	if (dec->last_retval == MTLZ_CODEC_RET_BUFFER)
	{
		if (packet == NULL/* || packet->data == NULL || packet->size <= 0*/)
		{
			MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
			return MTLZ_CODEC_RET_BUFFER;
		}

		/* Patch: h265 exception! */
#if 1
		if (packet->data != NULL
			&& dec->codec_id == AV_CODEC_ID_HEVC)
		{
			memcpy(tmp_buffer, packet->data, packet->size);
			//memset(tmp_buffer+packet->size, 0, PACKET_PADDING_SIZE);
			pkt->data = tmp_buffer;
		}
		else
#endif
		{
			pkt->data = packet->data;
		}
		pkt->size = packet->size;

		MTLZ_VERBOSE("%s: data %p, size %d\n",__FUNCTION__,
			pkt->data, pkt->size);
	}

AGAIN:
    len = avcodec_decode_video2(avctx, frame, &got_frame, pkt);

	if (len < 0)
	{
		dec->last_retval = MTLZ_CODEC_RET_BUFFER;
		MTLZ_ERROR("%s: Error while decoding frame %lu\n",__FUNCTION__,dec->frm_cnt);
		return MTLZ_CODEC_RET_BUFFER;
	}

	if (pkt->data)
	{
		pkt->size -= len;
		pkt->data += len;
	}

	if (got_frame)
	{
		MTLZ_VERBOSE("%s: frame[%lu]: %p, %d, %d, %d, %d\n",__FUNCTION__,
				dec->frm_cnt,
				frame->data[0], frame->linesize[0],
				avctx->width, avctx->height, avctx->pix_fmt);

		out_frame->buf_y  		= frame->data[0];
		out_frame->buf_u  		= frame->data[1];
		out_frame->buf_v  		= frame->data[2];
		out_frame->line_stride 	= frame->linesize[0];
		out_frame->width  		= avctx->width;			//frame->width
		out_frame->height 		= avctx->height;		//frame->height

		switch (avctx->pix_fmt)							//frame->format
		{
			case PIX_FMT_YUV420P:
				out_frame->format = 0;
				out_frame->chroma_width  = out_frame->width / 2;
				out_frame->chroma_height = out_frame->height / 2;
				break;
			case PIX_FMT_YUV422P:
				out_frame->format = 1;
				out_frame->chroma_width  = out_frame->width / 2;
				out_frame->chroma_height = out_frame->height;
				break;
			case PIX_FMT_YUV444P:
				out_frame->format = 2;
				out_frame->chroma_width  = out_frame->width;
				out_frame->chroma_height = out_frame->height;
				break;
			default:
				MTLZ_ERROR("[ERROR]%s: unsupported pixel format(%d)!\n", __FUNCTION__, avctx->pix_fmt);
				dec->last_retval = MTLZ_CODEC_RET_BUFFER;
				return MTLZ_CODEC_RET_BUFFER;
				break;
		}

		out_frame->pts = frame->pts;

		dec->frm_cnt ++;
		dec->last_tick = mtlz_get_tick();

		ret = MTLZ_CODEC_RET_FRAME;
	}
	else
	{
		if (pkt->size > 0)
		{
			//wrong!
			//no frame, but has bytes left!?
			MTLZ_WARN("[WARNING]%s: %d bytes left!\n",__FUNCTION__,
				pkt->size);

			goto AGAIN;
		}
	}

	dec->last_retval = ret;

	return ret;
}

struct mtlzplayer_comp_codec_st ffmpeg_mpeg2_codec_comp =
{
	.init 	= ffmpeg_mpeg2_init,
	.close 	= ffmpeg_close,
	.decode = ffmpeg_decode,
};

struct mtlzplayer_comp_codec_st ffmpeg_mpeg4_codec_comp =
{
	.init 	= ffmpeg_mpeg4_init,
	.close 	= ffmpeg_close,
	.decode = ffmpeg_decode,
};

struct mtlzplayer_comp_codec_st ffmpeg_h264_codec_comp =
{
	.init 	= ffmpeg_h264_init,
	.close 	= ffmpeg_close,
	.decode = ffmpeg_decode,
};

struct mtlzplayer_comp_codec_st ffmpeg_h265_codec_comp =
{
	.init 	= ffmpeg_h265_init,
	.close 	= ffmpeg_close,
	.decode = ffmpeg_decode,
};

