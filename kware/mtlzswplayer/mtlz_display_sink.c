/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_display_sink.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Display Output(Sink) for Monage-LZ SW Player.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <string.h>
#ifdef LINUX
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "mt_common.h"

#elif defined(__UC_OS__)
#include "mt_type.h"
#include "sys_define.h"
#include "mtos_mem.h"
#include "hal_misc.h"
#endif

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"
#include "mtlzplayer_os.h"

#ifdef LINUX
#define DISPLAY_REG_BASE			0x1f440000
#elif defined(__UC_OS__)
#define DISPLAY_REG_BASE			0xbf440000
#endif
#define REG_READ(reg)				(*((volatile unsigned int*)((ulong)reg)))
#define REG_WRITE(reg, val)			do{ *((volatile unsigned int*)((ulong)reg)) = (val); } while(0)

#define MAP_SIZE 					65536UL
#define MAP_MASK 					(MAP_SIZE - 1)

#define DISPLAY_STRIDE				MTLZ_DISPLAY_STRIDE
//#define DISPLAY_WIDTH				720
#define DISPLAY_HEIGHT				MTLZ_DISPLAY_HEIGHT

#define DISPLAY_WIDTH_HD			1920
#define DISPLAY_HEIGHT_HD			1080	//1080p
#define DISPLAY_HEIGHT_HALF_HD		540		//1080i

#define FRAME_BUFFER_CACHEABLE		1

/** display output sink */
static struct display_sink_st
{
	int mem_fd;

	ulong reg_base;
	ulong reg_mapped;

	unsigned int frame_buf_size_yuv;
	unsigned int frame_buf_size_y;
	unsigned int frame_buf_size_uv;

#ifdef LINUX
	/* mmz frame buffer */
	mt_mmz_buf_s stMBuf;
#endif

	phys_addr_t frame_buf_phy_addr_y;
	phys_addr_t frame_buf_phy_addr_uv;
	ulong frame_buf_usr_addr_y;
	ulong frame_buf_usr_addr_uv;

	//debug
	unsigned int frame_count;
	int width;
	int height;

} display_sink =
{
	.mem_fd					= -1,
	.reg_base 				= DISPLAY_REG_BASE,
	//720x576 YUV420, stride 1024
	.frame_buf_size_yuv		= DISPLAY_STRIDE*DISPLAY_HEIGHT*3/2,
	.frame_buf_size_y		= DISPLAY_STRIDE*DISPLAY_HEIGHT,
	.frame_buf_size_uv		= DISPLAY_STRIDE*DISPLAY_HEIGHT/2,
};

static inline void display_output_video_layer_enable(void)
{
	unsigned int val;

	val = REG_READ(display_sink.reg_mapped);
	val |= (unsigned int)(0x01 << 24);
	REG_WRITE(display_sink.reg_mapped, val);
}

static inline void display_output_video_layer_disable(void)
{
	unsigned int val;

	val = REG_READ(display_sink.reg_mapped);
	val &= (unsigned int)(~(0x01 << 24));
	REG_WRITE(display_sink.reg_mapped, val);

	mtlz_msleep(40); //avoid green screen issue! wait display no output.
}

static inline void display_output_set_frame_buffer_addr(
						unsigned int phy_addr_y,
						unsigned int phy_addr_uv)
{
	REG_WRITE(display_sink.reg_mapped + 0x104c, phy_addr_y / 8);
	REG_WRITE(display_sink.reg_mapped + 0x105c, phy_addr_uv / 8);
	REG_WRITE(display_sink.reg_mapped + 0x1070, phy_addr_y / 8);
	REG_WRITE(display_sink.reg_mapped + 0x1080, phy_addr_uv / 8);
	REG_WRITE(display_sink.reg_mapped + 0x10d0, phy_addr_y / 8);
	REG_WRITE(display_sink.reg_mapped + 0x10d4, phy_addr_uv / 8);
	REG_WRITE(display_sink.reg_mapped + 0x10d8, phy_addr_y / 8);
	REG_WRITE(display_sink.reg_mapped + 0x10dc, phy_addr_uv / 8);
}

static inline void display_output_set_window(void)
{
	unsigned int val;

	//hd window - x, 0x07810001
	val = 0x01;
	val |= (unsigned int)((DISPLAY_WIDTH_HD+1) << 16);
	REG_WRITE(display_sink.reg_mapped + 0x14, val);

	//hd window - y, 0x04390001
	val = 0x01;
	val |= (unsigned int)((DISPLAY_HEIGHT_HD+1) << 16);
	REG_WRITE(display_sink.reg_mapped + 0x18, val);
}

static inline void display_output_set_scale(int pic_w, int pic_h)
{
	unsigned int val;

	val = (unsigned int)((pic_w / DISPLAY_WIDTH_HD) & 0XF);
	val |= (unsigned int)(((pic_w % DISPLAY_WIDTH_HD) * 4096 / DISPLAY_WIDTH_HD) << 4);

	//TODO:
	//interlace
	val |= (unsigned int)(((pic_h / DISPLAY_HEIGHT_HALF_HD) & 0XF) << 16);
	val |= (unsigned int)(((pic_h % DISPLAY_HEIGHT_HALF_HD) * 4096 / DISPLAY_HEIGHT_HALF_HD) << 20);
	//or progressive
	//val |= ((res_h / DISPLAY_HEIGHT_HD) & 0XF) << 16;
	//val |= ((res_h % DISPLAY_HEIGHT_HD) * 4096 / DISPLAY_HEIGHT_HD) << 20;

	REG_WRITE(display_sink.reg_mapped + 0x04, val);
}

/**
 * @brief setup display output sink
 *
 * @param[in] pic_w,pic_h picture width/height
 *
 * @return display output sink handle
 */
static int display_output_setup(int pic_w, int pic_h)
{
	int error = 0;
	unsigned int val;
	//FIXME: how about supporting multi display output sink?
	int handle = 0;

	MTLZ_DEBUG("%s: w %d, h %d\n",__FUNCTION__,pic_w,pic_h);

	//check width/height
	if (pic_w > DISPLAY_STRIDE || pic_h > DISPLAY_HEIGHT)
	{
		MTLZ_ERROR("[ERROR]%s: (%d, %d) > (%d, %d)!\n",__FUNCTION__,pic_w,pic_h,
			DISPLAY_STRIDE,DISPLAY_HEIGHT);

		return MTLZ_INVALID_HANDLE;
	}

	if (display_sink.mem_fd == -1)
	{
		//Register
#ifdef LINUX
		display_sink.mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
		MTLZ_DEBUG("%s: fd=%d\n",__FUNCTION__,display_sink.mem_fd);
		if (display_sink.mem_fd == -1)
		{
			MTLZ_ERROR("[ERROR]%s: open %s failed!\n",__FUNCTION__,"/dev/mem");
			error = MTLZ_ENOENT;
			return MTLZ_INVALID_HANDLE;
		}

		display_sink.reg_mapped = (ulong)mmap(0,
													 MAP_SIZE,
													 PROT_READ | PROT_WRITE,
													 MAP_SHARED,
													 display_sink.mem_fd,
													 (off_t)(display_sink.reg_base & (~MAP_MASK)));
		MTLZ_DEBUG("%s: reg=0x%x\n",__FUNCTION__,display_sink.reg_mapped);
		if (MAP_FAILED == (void*)display_sink.reg_mapped)
		{
			MTLZ_ERROR("[ERROR]%s: map reg 0x%x failed!\n",__FUNCTION__,display_sink.reg_base);
			error = MTLZ_EIO;
			goto CLOSE_DEV_MEM;
		}

#elif defined(__UC_OS__)
		display_sink.mem_fd = 0;
		display_sink.reg_mapped = display_sink.reg_base;
#endif
		//video layer
		display_output_video_layer_disable();

#ifdef LINUX
		//frame buffer
	    memset(&display_sink.stMBuf, 0x00, sizeof(mt_mmz_buf_s));
	    display_sink.stMBuf.bufsize = display_sink.frame_buf_size_yuv;

		display_sink.stMBuf.phyaddr = (mt_u32)mt_mmz_new(display_sink.stMBuf.bufsize,
														8,
														NULL,
														(mt_char*)"DispOutFrm");
		if (display_sink.stMBuf.phyaddr == 0)
		{
			MTLZ_ERROR("[ERROR]%s: allocate display frame buffer failed!\n",__FUNCTION__);
			error = MTLZ_ENOMEM;
			goto UNMAP_REG;
		}

		display_sink.stMBuf.user_viraddr = mt_mmz_map(display_sink.stMBuf.phyaddr, FRAME_BUFFER_CACHEABLE);
		if (display_sink.stMBuf.user_viraddr == 0)
		{
			MTLZ_ERROR("[ERROR]%s: map display frame buffer failed!\n",__FUNCTION__);
			error = MTLZ_EIO;
			goto DEL_MMZ;
		}

		display_sink.frame_buf_phy_addr_y = display_sink.stMBuf.phyaddr;
		display_sink.frame_buf_usr_addr_y = (unsigned int)display_sink.stMBuf.user_viraddr;

#elif defined(__UC_OS__)
		display_sink.frame_buf_usr_addr_y = (unsigned int)mtos_align_malloc(
												display_sink.frame_buf_size_yuv,
												8);
		if (display_sink.frame_buf_usr_addr_y == 0)
		{
			MTLZ_ERROR("[ERROR]%s: allocate display frame buffer failed!\n",__FUNCTION__);
			error = MTLZ_ENOMEM;
			goto MALLOC_FAIL;
		}
		display_sink.frame_buf_phy_addr_y = hal_addr_nc(display_sink.frame_buf_usr_addr_y);
#endif
		MTLZ_DEBUG("%s: frame_buf_addr=0x%x\n",__FUNCTION__,display_sink.frame_buf_usr_addr_y);

		memset((void*)display_sink.frame_buf_usr_addr_y, 0, display_sink.frame_buf_size_yuv);

		display_sink.frame_buf_phy_addr_uv = display_sink.frame_buf_phy_addr_y
												 + display_sink.frame_buf_size_y;
		display_sink.frame_buf_usr_addr_uv = display_sink.frame_buf_usr_addr_y
												 + display_sink.frame_buf_size_y;

		//set frame buffer addr
		display_output_set_frame_buffer_addr(display_sink.frame_buf_phy_addr_y,
											display_sink.frame_buf_phy_addr_uv);

		//set linear input format, and video stride
		val = REG_READ(display_sink.reg_mapped + 0x3000);
		val &= (unsigned int)(~(0x03 << 4));
		switch (DISPLAY_STRIDE)
		{
			case 512:
				val |= (0x00 << 4);		//Stride: 512
				break;
			case 1024:
				val |= (0x01 << 4);		//Stride: 1024
				break;
			case 2048:
				val |= (0x02 << 4);		//Stride: 2048
				break;
			default:
				MTLZ_ERROR("[ERROR]%s: display stride %d not support!\n",__FUNCTION__,DISPLAY_STRIDE);
				val |= (0x01 << 4);		//Stride: 1024
				break;
		}
		val |= (0x01 << 8);				//Linear format
		REG_WRITE(display_sink.reg_mapped + 0x3000, val);

		//hd window
		display_output_set_window();

		//scale
		display_output_set_scale(pic_w, pic_h);

		//none di progressive
		REG_WRITE(display_sink.reg_mapped + 0x2058, 0x11);

		display_sink.frame_count = 0;
		display_sink.width = pic_w;
		display_sink.height = pic_h;
	}

	return handle;

#ifdef LINUX
DEL_MMZ:
	mt_mmz_delete(display_sink.stMBuf.phyaddr);

UNMAP_REG:
	munmap((void*)display_sink.reg_mapped, MAP_SIZE);

CLOSE_DEV_MEM:
	close(display_sink.mem_fd);

#elif defined(__UC_OS__)
MALLOC_FAIL:
#endif
	display_sink.mem_fd = -1;

	return MTLZ_INVALID_HANDLE;
}

/**
 * @brief teardown display output sink
 *
 * @param[in] sink display output sink handle
 *
 * @retval
 *    MTLZ_SUCCESS: success.
 */
static int display_output_teardown(int sink)
{
	MTLZ_DEBUG("Call display_output_teardown\n");

	if (display_sink.mem_fd != -1)
	{
#ifdef LINUX
		munmap((void*)display_sink.reg_mapped, MAP_SIZE);

		mt_mmz_unmap(display_sink.stMBuf.phyaddr);
		mt_mmz_delete(display_sink.stMBuf.phyaddr);

		close(display_sink.mem_fd);

#elif defined(__UC_OS__)
		mtos_align_free((void*)display_sink.frame_buf_usr_addr_y);
#endif
		display_sink.mem_fd = -1;
	}

	return MTLZ_SUCCESS;
}

/* NV12 + Little Endian */
static void YUV420_TO_NV12(unsigned char *src_y, unsigned char *src_u, unsigned char *src_v,
							int line_stride, int height)
{
	int i, j;
	unsigned char *dst_y;
	unsigned char *dst_uv;
	int chroma_width = line_stride / 2;
	int chroma_height = height / 2;
	int dst_stride = DISPLAY_STRIDE;	//FIXME
	int src_stride = line_stride;

	//Y
	dst_y = (unsigned char *)display_sink.frame_buf_usr_addr_y;
	for (i=0; i<height; i++)
	{
		for (j=0; j<line_stride; j+=8)
		{
			dst_y[j+0] = src_y[j+7];
			dst_y[j+1] = src_y[j+6];
			dst_y[j+2] = src_y[j+5];
			dst_y[j+3] = src_y[j+4];
			dst_y[j+4] = src_y[j+3];
			dst_y[j+5] = src_y[j+2];
			dst_y[j+6] = src_y[j+1];
			dst_y[j+7] = src_y[j+0];
		}
		dst_y += dst_stride;
		src_y += src_stride;
	}

	//UV
	dst_uv = (unsigned char *)display_sink.frame_buf_usr_addr_uv;
	for (i=0; i<chroma_height; i++)
	{
		for (j=0; j<chroma_width; j+=4)
		{
			//NV12 + Little Endian
			*dst_uv++ = src_v[3];
			*dst_uv++ = src_u[3];
			*dst_uv++ = src_v[2];
			*dst_uv++ = src_u[2];
			*dst_uv++ = src_v[1];
			*dst_uv++ = src_u[1];
			*dst_uv++ = src_v[0];
			*dst_uv++ = src_u[0];
			src_u += 4;
			src_v += 4;
		}
		dst_uv += (dst_stride - 2 * chroma_width);
	}

#ifdef LINUX
	if (FRAME_BUFFER_CACHEABLE)
		mt_mmz_flush((void *)display_sink.stMBuf.phyaddr, 0, 0);

#elif defined(__UC_OS__)
	if (FRAME_BUFFER_CACHEABLE)
		hal_dcache_flush((void*)display_sink.frame_buf_usr_addr_y,
						display_sink.frame_buf_size_yuv);
#endif
}

/* NV12 + Little Endian */
static void YUV422_TO_NV12(unsigned char *src_y, unsigned char *src_u, unsigned char *src_v,
							int line_stride, int height)
{
	int i, j;
	unsigned char *dst_y;
	unsigned char *dst_uv;
	int chroma_width = line_stride / 2;
	int chroma_height = height;
	int dst_stride = DISPLAY_STRIDE;	//FIXME
	int src_stride = line_stride;

	//Y
	dst_y = (unsigned char *)display_sink.frame_buf_usr_addr_y;
	for (i=0; i<height; i++)
	{
		for (j=0; j<line_stride; j+=8)
		{
			dst_y[j+0] = src_y[j+7];
			dst_y[j+1] = src_y[j+6];
			dst_y[j+2] = src_y[j+5];
			dst_y[j+3] = src_y[j+4];
			dst_y[j+4] = src_y[j+3];
			dst_y[j+5] = src_y[j+2];
			dst_y[j+6] = src_y[j+1];
			dst_y[j+7] = src_y[j+0];
		}
		dst_y += dst_stride;
		src_y += src_stride;
	}

	//UV
	dst_uv = (unsigned char *)display_sink.frame_buf_usr_addr_uv;
	for (i=0; i<chroma_height; i++)
	{
		if ((i & 0x01) == 0)	//1st field
		{
			for (j=0; j<chroma_width; j+=4)
			{
				//NV12 + Little Endian
				*dst_uv++ = src_v[3];
				*dst_uv++ = src_u[3];
				*dst_uv++ = src_v[2];
				*dst_uv++ = src_u[2];
				*dst_uv++ = src_v[1];
				*dst_uv++ = src_u[1];
				*dst_uv++ = src_v[0];
				*dst_uv++ = src_u[0];
				src_u += 4;
				src_v += 4;
			}
			dst_uv += (dst_stride - 2 * chroma_width);
		}
		else	//2nd field, ignore
		{
			src_u += chroma_width;
			src_v += chroma_width;
		}
	}

#ifdef LINUX
	if (FRAME_BUFFER_CACHEABLE)
		mt_mmz_flush((void *)display_sink.stMBuf.phyaddr, 0, 0);

#elif defined(__UC_OS__)
	if (FRAME_BUFFER_CACHEABLE)
		hal_dcache_flush((void*)display_sink.frame_buf_usr_addr_y,
						display_sink.frame_buf_size_yuv);
#endif
}

/* NV12 + Little Endian */
static void YUV444_TO_NV12(unsigned char *src_y, unsigned char *src_u, unsigned char *src_v,
							int line_stride, int height)
{
	int i, j;
	unsigned char *dst_y;
	unsigned char *dst_uv;
	int chroma_width = line_stride;
	int chroma_height = height;
	int dst_stride = DISPLAY_STRIDE;	//FIXME
	int src_stride = line_stride;

	//Y
	dst_y = (unsigned char *)display_sink.frame_buf_usr_addr_y;
	for (i=0; i<height; i++)
	{
		for (j=0; j<line_stride; j+=8)
		{
			dst_y[j+0] = src_y[j+7];
			dst_y[j+1] = src_y[j+6];
			dst_y[j+2] = src_y[j+5];
			dst_y[j+3] = src_y[j+4];
			dst_y[j+4] = src_y[j+3];
			dst_y[j+5] = src_y[j+2];
			dst_y[j+6] = src_y[j+1];
			dst_y[j+7] = src_y[j+0];
		}
		dst_y += dst_stride;
		src_y += src_stride;
	}

	//UV
	dst_uv = (unsigned char *)display_sink.frame_buf_usr_addr_uv;
	for (i=0; i<chroma_height; i++)
	{
		if ((i & 0x01) == 0)	//1st field
		{
			for (j=0; j<chroma_width; j+=8)
			{
				//NV12 + Little Endian
				*dst_uv++ = src_v[6];
				*dst_uv++ = src_u[6];
				*dst_uv++ = src_v[4];
				*dst_uv++ = src_u[4];
				*dst_uv++ = src_v[2];
				*dst_uv++ = src_u[2];
				*dst_uv++ = src_v[0];
				*dst_uv++ = src_u[0];
				src_u += 8;
				src_v += 8;
			}
			dst_uv += (dst_stride - chroma_width);
		}
		else	//2nd field, ignore
		{
			src_u += chroma_width;
			src_v += chroma_width;
		}
	}

#ifdef LINUX
	if (FRAME_BUFFER_CACHEABLE)
		mt_mmz_flush((void *)display_sink.stMBuf.phyaddr, 0, 0);

#elif defined(__UC_OS__)
if (FRAME_BUFFER_CACHEABLE)
	hal_dcache_flush((void*)display_sink.frame_buf_usr_addr_y,
					display_sink.frame_buf_size_yuv);
#endif
}

/**
 * @param[in] format YUV format
 *                   0: YUV420
 *                   1: YUV422
 *                   2: YUV444
 */
static int display_output_draw_frame(unsigned char *src_y, unsigned char *src_u, unsigned char *src_v,
							  int line_stride, int height,
							  unsigned long long pts,
							  int format)
{
	if (src_y == NULL || src_u == NULL || src_v == NULL)
	{
		MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
		return MTLZ_EINVAL;
	}

	MTLZ_VERBOSE("%s: [%u] (%d, %d) pts %llu fmt %d\n",__FUNCTION__,
			display_sink.frame_count,
			line_stride, height,
			pts,
			format);

	if (display_sink.mem_fd != -1)
	{
		//check width/height
		if (/*width != display_sink.width || */height != display_sink.height)
		{
			MTLZ_ERROR("[ERROR]%s: bad (%d, %d) or resolution changed!\n",__FUNCTION__,
				line_stride, height);
			return MTLZ_EINVAL;
		}

		if (format == 0)		//YUV420
		{
			YUV420_TO_NV12(src_y, src_u, src_v, line_stride, height);
		}
		else if (format == 1)	//YUV422
		{
			YUV422_TO_NV12(src_y, src_u, src_v, line_stride, height);
		}
		else if (format == 2)	//YUV444
		{
			YUV444_TO_NV12(src_y, src_u, src_v, line_stride, height);
		}
		else
		{
			MTLZ_ERROR("[ERROR]%s: format %d not support!\n",__FUNCTION__,format);
			return MTLZ_EINVAL;
		}

		if (display_sink.frame_count == 0)
			display_output_video_layer_enable();

		display_sink.frame_count ++;
	}
	else
	{
		MTLZ_ERROR("[ERROR]%s: display not opened!\n",__FUNCTION__);
		return MTLZ_FAILURE;
	}

	return MTLZ_SUCCESS;
}

/**
 * @brief display output sink draw one frame
 *
 * @param[in] sink display output sink handle
 * @param[in] frame display frame parameters
 *
 * @retval
 *    MTLZ_SUCCESS: success,
 *    MTLZ_FAILURE: failed.
 */
static int display_sink_draw_wrap(int sink, const struct mtlzplayer_frame_st *frame)
{
	return display_output_draw_frame(
				frame->buf_y,
				frame->buf_u,
				frame->buf_v,
				(int)frame->line_stride,
				(int)frame->height,
				frame->pts,
				frame->format);
}

struct mtlzplayer_comp_sink_st display_sink_comp =
{
	.setup 		= display_output_setup,
	.teardown 	= display_output_teardown,
	.draw 		= display_sink_draw_wrap,
};

/////////////////////////////////// For Test //////////////////////////////////
#if 0
int test_display_sink(void)
{
#include "yuv422_720_576.dat"

	int ret;
	int sink;
	struct mtlzplayer_frame_st frame;

	sink = display_output_setup(720, 576);

	frame.buf_y = yuv422_720_576;
	frame.buf_u = yuv422_720_576 + 720*576;
	frame.buf_v = frame.buf_u + 720*576/2;
	frame.width = 720;
	frame.height = 576;
	frame.chroma_width = 720 / 2;
	frame.chroma_height = 576;
	frame.format = 1;	/*1: YUV422*/
	frame.pts = 0;
	ret |= display_sink_draw_wrap(sink, &frame);

	ret |= display_output_teardown(sink);

	return ret;
}
#endif

