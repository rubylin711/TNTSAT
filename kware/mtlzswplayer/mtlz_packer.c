/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_packer.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/20
 * Description    : Frame packer for Monage-LZ SW Player.
 * History        :
 * 1.Date         : 2019/03/20
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
/*
 * 有些Codec必须传入完整的packet才能解码,
 * 比如: ffmpeg H264/H265 codec.
 * mtlz packer: 将前端(比如DMX)送入的数据重新打包.
 */
#include <string.h>

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"
#include "mtlzplayer_os.h"

/* seek packet header mode */
enum seek_set_e
{
	SEEK_FRONT = 0,
	SEEK_PIC,
	SEEK_REAR,
};

struct mtlz_packer_st
{
	enum mtlz_comp_codec_index_e codec;

	unsigned char *addr;
	unsigned int size;		//buffer size
	unsigned int bytes;		//data size

	/*
     * __|_________________|___
     *   ^       ^         ^
     *   |       |         |
     * front  pic_offset  rear
     */
	int front;				//valid data offset
	int pic_offset;			//first picture data offset
	int rear;				//picture data tail

	//return header position,
	//		 -1 if no valid header.
	int (*seek_header)(unsigned char *data, int from, int size, enum seek_set_e seek_set);
};
static struct mtlz_packer_st mtlz_packer;

/*
 ISO/IEC 14496-2:2001(E)
 Table 6-3 — Start code values

 video_object_start_code 			00 through 1F
 video_object_layer_start_code 		20 through 2F
 visual_object_sequence_start_code 	B0
 visual_object_sequence_end_code 	B1
 user_data_start_code 				B2
 group_of_vop_start_code 			B3
 video_session_error_code 			B4
 visual_object_start_code 			B5
 vop_start_code 					B6
 fba_object_start_code 				BA
 fba_object_plane_start_code 		BB
 mesh_object_start_code 			BC
 mesh_object_plane_start_code 		BD
 still_texture_object_start_code 	BE
 texture_spatial_layer_start_code 	BF
 texture_snr_layer_start_code 		C0
 texture_tile_start_code 			C1
 texture_shape_layer_start_code 	C2
 stuffing_start_code 				C3
 System start codes (see note) 		C6 through FF
 */
//return header position,
//		 -1 if no valid header.
//https://www.jianshu.com/p/76bd6bdb6a46
static int mpeg4_seek_header(unsigned char *data, int from, int size, enum seek_set_e seek_set)
{
	unsigned int code = 0xFFFFFFFF;
	int offset;
	unsigned char start_code;

	data += from;
	offset = from;

	if (offset + 4 <= size)
		code = ((unsigned int)data[0] << 16) | ((unsigned int)data[1] << 8) | (unsigned int)data[2];

	//"00 00 01 XX"
	while (offset + 4 <= size)
	{
		code = (code << 8) | data[3];
		if ((code & 0xFFFFFF00) != 0x100)
		{
			data ++;
			offset ++;
			continue;
		}

		start_code = data[3];
		if (seek_set == SEEK_FRONT) //front
		{
			switch (start_code)
			{
				case 0x00:
				case 0x20:
				/* visual_object_sequence_start_code */
				case 0xB0:
				/* visual_object_start_code */
				case 0xB5:
				/* group_of_vop_start_code */
				case 0xB3:
				/* vop_start_code */
				case 0xB6:
					return offset;
					break;
				default:
					break;
			}
		}
		else if (seek_set == SEEK_PIC)	//pic head
		{
			switch (start_code)
			{
				/* vop_start_code */
				case 0xB6:
					return offset;
					break;
				default:
					break;
			}
		}
		else if (seek_set == SEEK_REAR) //rear
		{
			switch (start_code)
			{
				/* visual_object_sequence_start_code */
				case 0xB0:
				/* visual_object_sequence_end_code */
				case 0xB1:
				/* visual_object_start_code */
				case 0xB5:
				/* group_of_vop_start_code */
				case 0xB3:
				/* vop_start_code */
				case 0xB6:
					return offset;
					break;
				default:
					break;
			}
		}
		else
		{
			//Error!
		}

		data ++;
		offset ++;
	}

	return -1;
}


//return header position,
//		 -1 if no valid header.
//http://www.iosxxx.com/blog/2017-08-09-%E4%BB%8E%E9%9B%B6%E4%BA%86%E8%A7%A3H264%E7%BB%93%E6%9E%84.html
static int h264_seek_header(unsigned char *data, int from, int size, enum seek_set_e seek_set)
{
	unsigned int code = 0xFFFFFFFF;
	int offset;
	unsigned char nal_type;

	data += from;
	offset = from;

	if (offset + 4 <= size)
		code = ((unsigned int)data[0] << 16) | ((unsigned int)data[1] << 8) | (unsigned int)data[2];

	//"00 00 01 XX"
	//or "00 00 00 01 XX"
	while (offset + 4 <= size)
	{
		code = (code << 8) | data[3];
		if ((code & 0xFFFFFF00) != 0x100)
		{
			data ++;
			offset ++;
			continue;
		}

		nal_type = data[3] & 0x1F;
		if (seek_set == SEEK_FRONT)	//front
		{
			switch (nal_type)
			{
				/* Slice */
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				/* SPS */
				case 7:
				/* PPS */
				case 8:
					return offset;
					break;
				default:
					break;
			}
		}
		else if (seek_set == SEEK_PIC)	//pic head
		{
			switch (nal_type)
			{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					return offset;
					break;
				default:
					break;
			}
		}
		else if (seek_set == SEEK_REAR)	//rear
		{
			switch (nal_type)
			{
				/* SEI */
				case 6:
				case 7:
				case 8:
				/* Seperator */
				case 9:
				/* End of Sequence */
				case 10:
				/* EOS */
				case 11:
				/* Fill */
				case 12:
					return offset;
					break;
				default:
					break;
			}
		}
		else
		{
			//Error!
		}

		data ++;
		offset ++;
	}

	return -1;
}

//return header position,
//		 -1 if no valid header.
//https://zhuanlan.zhihu.com/p/33720871
//https://www.jianshu.com/p/00a2ed58a77b
//https://gosuncnstudio.github.io/2017/12/15/%E8%A7%86%E9%A2%91%E7%A0%81%E6%B5%81%E6%A0%BC%E5%BC%8F%E8%A7%A3%E6%9E%90/
static int h265_seek_header(unsigned char *data, int from, int size, enum seek_set_e seek_set)
{
	unsigned int code = 0xFFFFFFFF;
	int offset;
	unsigned char nal_type;

	data += from;
	offset = from;

	if (offset + 4 <= size)
		code = ((unsigned int)data[0] << 16) | ((unsigned int)data[1] << 8) | (unsigned int)data[2];

	//"00 00 01 XX"
	//or "00 00 00 01 XX"
	while (offset + 4 <= size)
	{
		code = (code << 8) | data[3];
		if ((code & 0xFFFFFF00) != 0x100)
		{
			data ++;
			offset ++;
			continue;
		}

		nal_type = (data[3] >> 1) & 0x3F;

		if (seek_set == SEEK_FRONT) //front
		{
			switch (nal_type)
			{
				/* Slice */
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
				/* VPS */
				case 32:
				/* SPS */
				case 33:
				/* PPS */
				case 34:
					return offset;
					break;
				default:
					break;
			}
		}
		else if (seek_set == SEEK_PIC)	//pic head
		{
			switch (nal_type)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
					return offset;
					break;
				default:
					break;
			}
		}
		else if (seek_set == SEEK_REAR) //rear
		{
			switch (nal_type)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
				case 32:
				case 33:
				case 34:
				/* Delimiter */
				case 35:
				/* EOS */
				case 36:
				/* EOB */
				case 37:
				/* Fill */
				case 38:
				/* Prefix SEI */
				case 39:
				/* Suffix SEI */
				case 40:
					return offset;
					break;
				default:
					break;
			}
		}
		else
		{
			//Error!
		}

		data ++;
		offset ++;
	}

	return -1;
}

static int mtlz_init_packer(struct mtlz_packer_st *packer,
						 enum mtlz_comp_codec_index_e codec,
						 int (*seek_header)(unsigned char *, int, int, enum seek_set_e))
{
	int ret = MTLZ_SUCCESS;

	packer->codec = codec;
	MTLZ_DEBUG("%s: codec=%d.\n",__FUNCTION__,codec);

	packer->addr = MTLZ_MALLOC(MAX_PACKET_SIZE+PACKET_PADDING_SIZE);
	MTLZ_ASSERT(packer->addr != NULL);
	packer->size = MAX_PACKET_SIZE;
	MTLZ_DEBUG("%s: buffer size %u.\n",__FUNCTION__,packer->size);
	packer->bytes = 0;
	packer->front = -1;
	packer->pic_offset = -1;
	packer->rear = -1;

	packer->seek_header = seek_header;

	return ret;
}

//static int mtlz_destroy_packer(struct mtlz_packer_st *packer)
static int mtlz_destroy_packer(ulong fd)
{
	struct mtlz_packer_st *packer = (struct mtlz_packer_st *)fd;

	if (packer == NULL)
	{
		MTLZ_ERROR("[ERROR]%s: invalid parameter!\n",__FUNCTION__);
		return MTLZ_EINVAL;
	}

	MTLZ_DEBUG("%s: codec=%d.\n",__FUNCTION__,packer->codec);

	MTLZ_ASSERT(packer->addr != NULL);
	MTLZ_FREE(packer->addr);
	packer->addr = NULL;

	return MTLZ_SUCCESS;
}

//static int mtlz_packer_get_buffer(struct mtlz_packer_st *packer, unsigned char **buf, unsigned int *size)
static int mtlz_packer_get_buffer(ulong fd, unsigned char **buf, unsigned int *size)
{
	struct mtlz_packer_st *packer = (struct mtlz_packer_st *)fd;
	unsigned int avail_size;

	if (packer == NULL || buf == NULL || size == NULL || *size <= 0)
	{
		MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
		return MTLZ_EINVAL;
	}

	avail_size = packer->size - packer->bytes;
	if (avail_size == 0)
	{
		//FIXME: reset buffer?
		MTLZ_ERROR("[ERROR]%s: buffer full! pls enlarge your packet buffer!\n",__FUNCTION__);
		MTLZ_ERROR("%s: front %d, pic %d, rear %d.\n",__FUNCTION__,
			packer->front, packer->pic_offset, packer->rear);
		MTLZ_BUG();
		return MTLZ_ENOMEM;
	}

	*size = MIN(avail_size, *size);

	*buf = packer->addr + packer->bytes;

	return MTLZ_SUCCESS;
}

//static int mtlz_packer_put_buffer(struct mtlz_packer_st *packer, unsigned char *buf, unsigned int size)
static int mtlz_packer_put_buffer(ulong fd, unsigned char *buf, unsigned int size)
{
	struct mtlz_packer_st *packer = (struct mtlz_packer_st *)fd;

	if (packer == NULL || buf == NULL || size == 0)
	{
		MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
		return MTLZ_EINVAL;
	}

	if (packer->bytes + size <= packer->size)
	{
		packer->bytes += size;
		MTLZ_VERBOSE("%s: put %u -> %u\n",__FUNCTION__,size,packer->bytes);
		return MTLZ_SUCCESS;
	}
	else
	{
		MTLZ_ERROR("[ERROR]%s: buffer overflow(%u + %u > %u)!\n",__FUNCTION__,
				packer->bytes,
				size,
				packer->size);
		MTLZ_BUG();
		return MTLZ_ENOMEM;
	}
}

//static int mtlz_packer_read_packet(struct mtlz_packer_st *packer,
//									struct mtlzplayer_packet_st *pkt,
//									int last_packet)
static int mtlz_packer_read_packet(ulong fd,
									struct mtlzplayer_packet_st *pkt,
									int last_packet)
{
	struct mtlz_packer_st *packer = (struct mtlz_packer_st *)fd;

	if (packer == NULL || pkt == NULL)
	{
		MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
		return MTLZ_EINVAL;
	}

	//previous packet consumed,
	//memmove rear->bytes to head
	if (packer->rear != -1)
	{
		if (packer->bytes > packer->rear)
			memmove(packer->addr, packer->addr+packer->rear, packer->bytes-(unsigned int)packer->rear);	//include the rear byte

		packer->bytes = packer->bytes - (unsigned int)packer->rear;
		packer->front = -1;
		packer->pic_offset = -1;
		packer->rear = -1;
	}

	if (packer->bytes > 0)
	{
		if (packer->front == -1)
			packer->front = packer->seek_header(packer->addr, 0, (int)packer->bytes, SEEK_FRONT);

		//if (packer->front == -1)
			//drop it?

		if (packer->front != -1
			&& packer->bytes > packer->front+4
			&& packer->pic_offset == -1)
			packer->pic_offset = packer->seek_header(packer->addr, packer->front, (int)packer->bytes, SEEK_PIC);

		if (packer->pic_offset != -1
			&& packer->bytes > packer->pic_offset+4)
			packer->rear = packer->seek_header(packer->addr, packer->pic_offset+4, (int)packer->bytes, SEEK_REAR);

		if (packer->rear != -1)
		{
			MTLZ_VERBOSE("%s: front %d, pic %d, rear %d bytes %u.\n",__FUNCTION__,
				packer->front, packer->pic_offset, packer->rear, packer->bytes);

			pkt->data = packer->addr + packer->front;
			pkt->size = (unsigned int)(packer->rear - packer->front);
			MTLZ_VERBOSE("%s: packet (%p, %u).\n",__FUNCTION__,
				pkt->data, pkt->size);

			return MTLZ_SUCCESS;
		}
		else
		{
			/* EOS Proc-2 */
			//last packet
			if (last_packet
				&& packer->front != -1
				&& packer->pic_offset != -1
				&& packer->bytes > packer->pic_offset)
			{
				packer->rear = (int)packer->bytes;
				pkt->data = packer->addr + packer->front;
				pkt->size = (unsigned int)(packer->rear - packer->front);
				MTLZ_DEBUG("%s: last packet (%p, %u).\n",__FUNCTION__,
					pkt->data, pkt->size);

				return MTLZ_SUCCESS;
			}

			return MTLZ_FAILURE;
		}
	}
	else
	{
		return MTLZ_FAILURE;
	}
}

static ulong mpeg4_init_packer(void)
{
	int ret;

	ret = mtlz_init_packer(&mtlz_packer,
							MTLZ_COMP_CODEC_MPEG4_INDEX,
							mpeg4_seek_header);

	if (ret == MTLZ_SUCCESS)
		return (ulong)&mtlz_packer;
	else
		return MTLZ_NULL_FD;
}

static ulong h264_init_packer(void)
{
	int ret;

	ret = mtlz_init_packer(&mtlz_packer,
							MTLZ_COMP_CODEC_H264_INDEX,
							h264_seek_header);

	if (ret == MTLZ_SUCCESS)
		return (ulong)&mtlz_packer;
	else
		return MTLZ_NULL_FD;
}

static ulong h265_init_packer(void)
{
	int ret;

	ret = mtlz_init_packer(&mtlz_packer,
							MTLZ_COMP_CODEC_H265_INDEX,
							h265_seek_header);

	if (ret == MTLZ_SUCCESS)
		return (ulong)&mtlz_packer;
	else
		return MTLZ_NULL_FD;
}

struct mtlzplayer_comp_packer_st mpeg4_packer_comp =
{
	.init 			= mpeg4_init_packer,
	.destroy 		= mtlz_destroy_packer,
	.get_buffer		= mtlz_packer_get_buffer,
	.put_buffer		= mtlz_packer_put_buffer,
	.read_packet	= mtlz_packer_read_packet,
};

struct mtlzplayer_comp_packer_st h264_packer_comp =
{
	.init 			= h264_init_packer,
	.destroy 		= mtlz_destroy_packer,
	.get_buffer 	= mtlz_packer_get_buffer,
	.put_buffer 	= mtlz_packer_put_buffer,
	.read_packet	= mtlz_packer_read_packet,
};

struct mtlzplayer_comp_packer_st h265_packer_comp =
{
	.init 			= h265_init_packer,
	.destroy 		= mtlz_destroy_packer,
	.get_buffer 	= mtlz_packer_get_buffer,
	.put_buffer 	= mtlz_packer_put_buffer,
	.read_packet	= mtlz_packer_read_packet,
};

