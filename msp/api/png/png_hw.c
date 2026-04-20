/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h> /* mmap */
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/types.h>
#include <pthread.h>

#include "mt_drv_disp.h"
#include "png.h"
#include "mt_type.h"
#include "mt_png_api.h"
#include "mt_debug.h"
#include "png_hw.h"
#include "mpi_memdev.h"
#include "mt_module_debug.h"


// #define MT_DBG_PNG printf

extern ulong png_reg_virt_addr;

// reference to register map.
typedef enum {
   DOWN_SAMPLE_ARGB444 = 0x0,
   DOWN_SAMPLE_ARGB1555 = 0x1,
   DOWN_SAMPLE_RGB565 = 0x2,
   DOWN_SAMPLE_RGB555 = 0x3,
   DOWN_SAMPLE_RGB444 = 0x4,

   DOWN_SAMPLE_NONE = -1
}DOWN_SAMPLE_FMT;

static void png_write_register(unsigned int addr, unsigned int data)
{
   ulong *reg_addr;

    reg_addr = (ulong *)(addr + (ulong)png_reg_virt_addr);
    mpi_write_reg32(reg_addr, data);

}

static unsigned int png_read_register(unsigned int addr)
{
    ulong *reg_addr;

    reg_addr = (ulong *)(addr + (ulong)png_reg_virt_addr);
    return mpi_read_reg32(reg_addr);

}


void dump_reg(void)
{
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_START", PNG_START, png_read_register(PNG_START));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_SIZE", PNG_SIZE, png_read_register(PNG_SIZE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_TYPE", PNG_TYPE, png_read_register(PNG_TYPE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_TRNS_COLOR_R", PNG_TRNS_COLOR_R, png_read_register(PNG_TRNS_COLOR_R));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_TRNS_COLOR_G", PNG_TRNS_COLOR_G, png_read_register(PNG_TRNS_COLOR_G));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_TRNS_COLOR_B", PNG_TRNS_COLOR_B, png_read_register(PNG_TRNS_COLOR_B));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_OUT_TYPE", PNG_OUT_TYPE, png_read_register(PNG_OUT_TYPE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_CONV_CFG", PNG_CONV_CFG, png_read_register(PNG_CONV_CFG));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PIX_ALP_CFG", PNG_PIX_ALP_CFG, png_read_register(PNG_PIX_ALP_CFG));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_STATE", PNG_STATE, png_read_register(PNG_STATE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_INT_STATE", PNG_INT_STATE, png_read_register(PNG_INT_STATE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_INT_EN", PNG_INT_EN, png_read_register(PNG_INT_EN));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_STATE_CLR", PNG_STATE_CLR, png_read_register(PNG_STATE_CLR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_INT_STATE_CLR", PNG_INT_STATE_CLR, png_read_register(PNG_INT_STATE_CLR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_IDAT_CNT", PNG_IDAT_CNT, png_read_register(PNG_IDAT_CNT));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_AXI_CFG", PNG_AXI_CFG, png_read_register(PNG_AXI_CFG));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_AXI_BUSY", PNG_AXI_BUSY, png_read_register(PNG_AXI_BUSY));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_AXI_TEST", PNG_AXI_TEST, png_read_register(PNG_AXI_TEST));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_AXI_IDLE", PNG_AXI_IDLE, png_read_register(PNG_AXI_IDLE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_CFG", PNG_ZLIB_CFG, png_read_register(PNG_ZLIB_CFG));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_HDR_INFO", PNG_ZLIB_HDR_INFO, png_read_register(PNG_ZLIB_HDR_INFO));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_BLK_HDR_INFO", PNG_ZLIB_BLK_HDR_INFO, png_read_register(PNG_ZLIB_BLK_HDR_INFO));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_DICTID", PNG_ZLIB_DICTID, png_read_register(PNG_ZLIB_DICTID));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_ADLER32_REF", PNG_ZLIB_ADLER32_REF, png_read_register(PNG_ZLIB_ADLER32_REF));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_HNUM", PNG_ZLIB_HNUM, png_read_register(PNG_ZLIB_HNUM));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_STATUS", PNG_ZLIB_STATUS, png_read_register(PNG_ZLIB_STATUS));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_ADLER32_IMP", PNG_ZLIB_ADLER32_IMP, png_read_register(PNG_ZLIB_ADLER32_IMP));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_DBG_CFG", PNG_ZLIB_DBG_CFG, png_read_register(PNG_ZLIB_DBG_CFG));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_HLIT_MAX", PNG_ZLIB_HLIT_MAX, png_read_register(PNG_ZLIB_HLIT_MAX));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_HDIST_MAX", PNG_ZLIB_HDIST_MAX, png_read_register(PNG_ZLIB_HDIST_MAX));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_HCLEN_MAX", PNG_ZLIB_HCLEN_MAX, png_read_register(PNG_ZLIB_HCLEN_MAX));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_SYM_TAB", PNG_ZLIB_SYM_TAB, png_read_register(PNG_ZLIB_SYM_TAB));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_BS_OUT_CNT", PNG_ZLIB_BS_OUT_CNT, png_read_register(PNG_ZLIB_BS_OUT_CNT));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_BUF_START_ADDR", PNG_BUF_START_ADDR, png_read_register(PNG_BUF_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_BUF_END_ADDR", PNG_BUF_END_ADDR, png_read_register(PNG_BUF_END_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_BS_START_ADDR", PNG_BS_START_ADDR, png_read_register(PNG_BS_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_BS_END_ADDR", PNG_BS_END_ADDR, png_read_register(PNG_BS_END_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_BS_CUR_ADDR", PNG_BS_CUR_ADDR, png_read_register(PNG_BS_CUR_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS1_START_ADDR", PNG_PASS1_START_ADDR, png_read_register(PNG_PASS1_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS2_START_ADDR", PNG_PASS2_START_ADDR, png_read_register(PNG_PASS2_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS3_START_ADDR", PNG_PASS3_START_ADDR, png_read_register(PNG_PASS3_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS4_START_ADDR", PNG_PASS4_START_ADDR, png_read_register(PNG_PASS4_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS5_START_ADDR", PNG_PASS5_START_ADDR, png_read_register(PNG_PASS5_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS6_START_ADDR", PNG_PASS6_START_ADDR, png_read_register(PNG_PASS6_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS7_START_ADDR", PNG_PASS7_START_ADDR, png_read_register(PNG_PASS7_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS1_STRIDE", PNG_PASS1_STRIDE, png_read_register(PNG_PASS1_STRIDE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS2_STRIDE", PNG_PASS2_STRIDE, png_read_register(PNG_PASS2_STRIDE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS3_STRIDE", PNG_PASS3_STRIDE, png_read_register(PNG_PASS3_STRIDE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS4_STRIDE", PNG_PASS4_STRIDE, png_read_register(PNG_PASS4_STRIDE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS5_STRIDE", PNG_PASS5_STRIDE, png_read_register(PNG_PASS5_STRIDE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS6_STRIDE", PNG_PASS6_STRIDE, png_read_register(PNG_PASS6_STRIDE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_PASS7_STRIDE", PNG_PASS7_STRIDE, png_read_register(PNG_PASS7_STRIDE));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_FLT_START_ADDR", PNG_FLT_START_ADDR, png_read_register(PNG_FLT_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_START_ADDR", PNG_ZLIB_START_ADDR, png_read_register(PNG_ZLIB_START_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_END_ADDR", PNG_ZLIB_END_ADDR, png_read_register(PNG_ZLIB_END_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_RD_ADDR", PNG_ZLIB_RD_ADDR, png_read_register(PNG_ZLIB_RD_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_CUR_ADDR", PNG_ZLIB_CUR_ADDR, png_read_register(PNG_ZLIB_CUR_ADDR));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_OUT_CNT", PNG_ZLIB_OUT_CNT, png_read_register(PNG_ZLIB_OUT_CNT));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_HUFF_OUT_CNT", PNG_HUFF_OUT_CNT, png_read_register(PNG_HUFF_OUT_CNT));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_BS_TEST_CFG", PNG_ZLIB_BS_TEST_CFG, png_read_register(PNG_ZLIB_BS_TEST_CFG));
	MT_DBG_PNG("\r\n [0x%08x]:0x%08x PNG_ZLIB_BS_TEST_DAT", PNG_ZLIB_BS_TEST_DAT, png_read_register(PNG_ZLIB_BS_TEST_DAT));

}


 mt_u32 png_get_bpp(pix_fmt_t fmt)
{
  mt_u32 bpp;
	switch(fmt)
	{
	case PIX_FMT_ARGB8888:
	case PIX_FMT_RGBA8888_SMALL_ENDIAN:
	case PIX_FMT_AGRAY_16_16:
		bpp = 32;
		break;
	case PIX_FMT_RGBPALETTE1:
	case PIX_FMT_GRAY_1:
		bpp = 1;
		break;
	case PIX_FMT_RGBPALETTE2:
	case PIX_FMT_GRAY_2:
		bpp = 2;
		break;
	case PIX_FMT_RGBPALETTE4:
	case PIX_FMT_GRAY_4:
		bpp = 4;
		break;
	case PIX_FMT_RGBPALETTE8:
	case PIX_FMT_GRAY_8:
		bpp = 8;
		break;
	case PIX_FMT_AGRAY_8_8:
	case PIX_FMT_GRAY_16:
    case PIX_FMT_ARGB4444:
    case PIX_FMT_ARGB1555:
    case PIX_FMT_RGB565:
		bpp = 16;
		break;
	case PIX_FMT_BGR888:
		bpp = 24;
		break;
	case PIX_FMT_BGR48:
		bpp = 48;
		break;
	case PIX_FMT_ABGR64:
		bpp = 64;
		break;
	default:
		bpp = 32;
		break;
	}
  return bpp;
}

static mt_s32 set_output_format(pix_fmt_t out_fmt, mt_u32 in_color_type, mt_u32 in_bit_depth, MT_BOOL with_tRNS)
{
	mt_u32 out_bpp = 0;
	mt_u32 pix_conv_cfg = 0;
	MT_BOOL expand_to_byte_en = 0;
	MT_BOOL gray_bgr_en = 0;
	MT_BOOL add_alpha_en = 0;
	MT_BOOL bgr_rgb_en = 0;
	MT_BOOL depth16to8_en = 0;
	MT_BOOL trans_en = 0;
	MT_BOOL argb8_ds_en = 0;

    DOWN_SAMPLE_FMT down_sample_fmt;
    pix_fmt_t org_out_fmt = out_fmt;

    switch (out_fmt) {
        // TODO: Due to pix_fmt_t haven't these fmts, so here cannot down sample it.
    #if 0
        case PIX_FMT_RGBA555:
            down_sample_fmt = DOWN_SAMPLE_RGB555;
            break;
        case PIX_FMT_RGB444:
            down_sample_fmt = DOWN_SAMPLE_RGB444;
            break;
    #endif
        case PIX_FMT_ARGB4444:
            down_sample_fmt = DOWN_SAMPLE_ARGB444;
            break;
        case PIX_FMT_ARGB1555:
            down_sample_fmt = DOWN_SAMPLE_ARGB1555;
            break;
        case PIX_FMT_RGB565:
            down_sample_fmt = DOWN_SAMPLE_RGB565;
            break;
        case PIX_FMT_ARGB8888:
        default:
            down_sample_fmt = DOWN_SAMPLE_NONE;
            break;
    }
    if (down_sample_fmt != DOWN_SAMPLE_NONE)
    {
        // converto to ARGB8888 first, then process down sample.
        out_fmt = PIX_FMT_ARGB8888;
    }

	if(in_color_type == PNG_COLOR_TYPE_PALETTE)
	{
		if((out_fmt == PIX_FMT_RGBPALETTE1 && in_bit_depth == 1) ||
			(out_fmt == PIX_FMT_RGBPALETTE2 && in_bit_depth == 2) ||
			(out_fmt == PIX_FMT_RGBPALETTE4 && in_bit_depth == 4) ||
			(out_fmt == PIX_FMT_RGBPALETTE8 && in_bit_depth == 8))
		{
		}
		else if(out_fmt == PIX_FMT_RGBPALETTE8)
		{
			expand_to_byte_en = 1;
		}
		else
		{
			MT_ERR_PNG("\r\n set_output_format fail, infmt:%d, outfmt:%d", in_color_type, out_fmt);
			return MT_ERR_PNG_INVALID_PARA;
		}
	}
	else if(in_color_type == PNG_COLOR_TYPE_GRAY)
	{
		if((out_fmt == PIX_FMT_GRAY_1 && in_bit_depth == 1) ||
			(out_fmt == PIX_FMT_GRAY_2 && in_bit_depth == 2) ||
			(out_fmt == PIX_FMT_GRAY_4 && in_bit_depth == 4) ||
			(out_fmt == PIX_FMT_GRAY_8 && in_bit_depth == 8) ||
			(out_fmt == PIX_FMT_GRAY_16 && in_bit_depth == 16))
		{
		}
		else if(out_fmt == PIX_FMT_GRAY_8)
		{
			if(in_bit_depth == 16)
				depth16to8_en = 1;
			else
				expand_to_byte_en = 1;
		}
		else if(out_fmt == PIX_FMT_ARGB8888)
		{
			if(in_bit_depth == 16)
				depth16to8_en = 1;
			else
				expand_to_byte_en = 1;
			gray_bgr_en = 1;
			add_alpha_en = 1;
		}
		else
		{
			MT_ERR_PNG("\r\n set_output_format fail, infmt:%d, outfmt:%d", in_color_type, out_fmt);
			return MT_ERR_PNG_INVALID_PARA;
		}
	}
	else if(in_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		if((out_fmt == PIX_FMT_AGRAY_8_8 && in_bit_depth == 8) ||
			(out_fmt == PIX_FMT_AGRAY_16_16 && in_bit_depth == 16))
		{
		}
		else if(out_fmt == PIX_FMT_AGRAY_8_8)
		{
			depth16to8_en = 1;
		}
		else if(out_fmt == PIX_FMT_ARGB8888)
		{
			if(in_bit_depth == 16)
				depth16to8_en = 1;
			gray_bgr_en = 1;
		}
		else
		{
			MT_ERR_PNG("\r\n set_output_format fail, infmt:%d, outfmt:%d", in_color_type, out_fmt);
			return MT_ERR_PNG_INVALID_PARA;
		}
	}
	else if(in_color_type == PNG_COLOR_TYPE_RGB)
	{
		if((out_fmt == PIX_FMT_BGR888 && in_bit_depth == 8) ||
			(out_fmt == PIX_FMT_BGR48 && in_bit_depth == 16))
		{
		}
		else if(out_fmt == PIX_FMT_BGR888)
		{
			depth16to8_en = 1;
		}
		else if(out_fmt == PIX_FMT_ARGB8888)
		{
			if(in_bit_depth == 16)
				depth16to8_en = 1;
			add_alpha_en = 1;
			bgr_rgb_en = 1;
		}
		else
		{
			MT_ERR_PNG("\r\n set_output_format fail, infmt:%d, outfmt:%d", in_color_type, out_fmt);
			return MT_ERR_PNG_INVALID_PARA;
		}
	}
	else if(in_color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		if((out_fmt == PIX_FMT_RGBA8888_SMALL_ENDIAN && in_bit_depth == 8) ||
			(out_fmt == PIX_FMT_ABGR64 && in_bit_depth == 16))
		{
		}
		else if(out_fmt == PIX_FMT_ARGB8888)
		{
			if(in_bit_depth == 16)
				depth16to8_en = 1;
			bgr_rgb_en = 1;
		}
		else
		{
			MT_ERR_PNG("\r\n set_output_format fail, infmt:%d, outfmt:%d", in_color_type, out_fmt);
			return MT_ERR_PNG_INVALID_PARA;
		}
	}
	else
	{
		MT_ERR_PNG("\r\n set_output_format fail, infmt:%d, outfmt:%d", in_color_type, out_fmt);
		return MT_ERR_PNG_INVALID_PARA;
	}

	if(with_tRNS && (out_fmt == PIX_FMT_ARGB8888))
	{
		if((in_color_type == PNG_COLOR_TYPE_GRAY) ||
			(in_color_type == PNG_COLOR_TYPE_RGB))
		{
			trans_en = 1;
		}
	}

	pix_conv_cfg = (expand_to_byte_en << 1)
		| (gray_bgr_en << 2)
		| (add_alpha_en << 3)
		| (bgr_rgb_en << 5)
		| (depth16to8_en << 7)
		| (trans_en << 12)
		| (1 << 16); //expand to byte, low bits use gray

    if (down_sample_fmt != DOWN_SAMPLE_NONE) {
        argb8_ds_en = 1;
        pix_conv_cfg |=  (argb8_ds_en << 8) | (down_sample_fmt << 20);
    }

	out_bpp = png_get_bpp(org_out_fmt);
	png_write_register(PNG_OUT_TYPE, out_bpp << 8);
	png_write_register(PNG_CONV_CFG, pix_conv_cfg);
	png_write_register(PNG_PIX_ALP_CFG, 0xffff);
	return MT_SUCCESS;
}

mt_u32 calc_filter_buf_size(png_structp pngdec_ptr, png_infop info_ptr)
{
	mt_u32 width = 0;
	mt_u32 bit_depth = 0;
	mt_u32 chan_num = 0;
	mt_u32 filter_buf_size = 0;

	width = png_get_image_width(pngdec_ptr, info_ptr);
	chan_num = png_get_channels(pngdec_ptr,info_ptr);
	if(chan_num == 3)
		chan_num = 4;
	bit_depth = png_get_bit_depth(pngdec_ptr, info_ptr);
	filter_buf_size = PNG_ALIGN(PNG_ALIGN(width * chan_num * bit_depth, 8) / 8, 16);

	return filter_buf_size;
}

void get_img_buf_stride(MT_BOOL is_interlace, pix_fmt_t out_fmt, mt_u32 width, mt_u32 *p_stride)
{
	mt_u32 bpp;
	// mt_u32 width_aligned = PNG_ALIGN(width, 8);
	// mt_u32 width_aligned = width;

	bpp = png_get_bpp(out_fmt);
	if(is_interlace)
	{
		int i;
		p_stride[0] = PNG_ALIGN(width * bpp / 8 / 8, 16);
		p_stride[1] = p_stride[0];
		p_stride[2] = PNG_ALIGN(width * bpp / 8 / 4, 16);
		p_stride[3] = p_stride[2];
		p_stride[4] = PNG_ALIGN(width * bpp / 8 / 2, 16);
		p_stride[5] = p_stride[4];
		p_stride[6] = PNG_ALIGN(width * bpp / 8, 16);
		for(i = 0; i < 7; i++)
		{
			if(p_stride[i] == 0)
				p_stride[i] = 16;
		}
	}
	else
	{
		p_stride[0] = PNG_ALIGN(PNG_ALIGN(width * bpp, 8) / 8, 16);
	}
}


static void png_hw_reset(void)
{
  mt_u32 val = 0;
  mt_u32 bak_val = 0;
  
  mt_sys_read_register(0xbf1c1010, &bak_val);
  mt_sys_read_register(0xbf50a70c, &val);

  //axi
  val &= ~0x2;
  mt_sys_write_register(0xbf50a70c, val);
  //ahb
  val &= ~0x1;
  mt_sys_write_register(0xbf50a70c, val);
  //core
  val &= ~0x4;
  mt_sys_write_register(0xbf50a70c, val);

  MT_USLEEP(1000);
  /*cancle reset */


  //core
  val |= 0x4;
  mt_sys_write_register(0xbf50a70c, val);
  //ahb
  val |= 0x1;
  mt_sys_write_register(0xbf50a70c, val);
  //axi
  val |= 0x2;
  mt_sys_write_register(0xbf50a70c, val);

  mt_sys_write_register(0xbf1c1010, bak_val);

}


void png_hw_force_exit(void)
{
    mt_u32 dtmp = 0;

     //stop png
     dtmp = png_read_register(PNG_AXI_CFG) | 0x1;
     png_write_register(PNG_AXI_CFG, dtmp);
     dtmp = png_read_register(PNG_AXI_IDLE);
     while(0x3 != (dtmp & 0x3))  //AXI_BUS_IDLE
     {
       dtmp = png_read_register(PNG_AXI_IDLE);
     }
     //reset png
     png_hw_reset();
}



mt_s32 png_hw_start(png_structp pngdec_ptr, png_infop info_ptr, png_cfg_t *p_cfg)
{
  mt_u32 width = 0;
  mt_u32 height = 0;
  mt_u32 color_type = 0;
  mt_u32 bit_depth = 0;
  mt_u32 interlace = 0;
  mt_u32 with_tRNS = 0;
  png_bytep trans_alpha = NULL;
  int num_trans;
  png_color_16p trans_color;
  mt_u32 dtmp;
  mt_u32 stride[7] = {0};


	png_hw_reset();
#ifdef CONFIG_MT_FPGA_GPE
	png_int_enable();
#endif

  width = png_get_image_width(pngdec_ptr, info_ptr);
  height = png_get_image_height(pngdec_ptr, info_ptr);
  png_write_register(PNG_SIZE, (height << 16) | width);

  color_type = png_get_color_type(pngdec_ptr, info_ptr);
  bit_depth = png_get_bit_depth(pngdec_ptr, info_ptr);
  interlace = (png_get_interlace_type(pngdec_ptr, info_ptr) == PNG_INTERLACE_NONE) ? 0 : 1;

  png_write_register(PNG_TYPE, ((!p_cfg->comp_dis) << 16) | (interlace << 12) | (color_type << 8) | bit_depth);

  with_tRNS = ((png_get_tRNS(pngdec_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color) & PNG_INFO_tRNS) == PNG_INFO_tRNS) ? 1 : 0;
	if(with_tRNS)
  {
  	if(color_type == PNG_COLOR_TYPE_GRAY)
  	{
  		mt_u32 gray = trans_color->gray;
  		if(bit_depth < 8)
			{
				switch(bit_depth)
				{
					case 1:
						gray = (gray & 0x01) * 0xff;
						break;
					case 2:
						gray = (gray & 0x03) * 0x55;
						break;
					case 4:
						gray = (gray & 0x0f) * 0x11;
						break;
					case 8:
						gray = gray & 0xff;
						break;
					default:
						break;
				}
			}
			png_write_register(PNG_TRNS_COLOR_R, gray);
			png_write_register(PNG_TRNS_COLOR_G, gray);
			png_write_register(PNG_TRNS_COLOR_B, gray);
  	}
		else if(color_type == PNG_COLOR_TYPE_RGB)
		{
			if(bit_depth == 8)
			{
				png_write_register(PNG_TRNS_COLOR_R, trans_color->red & 0xff);
				png_write_register(PNG_TRNS_COLOR_G, trans_color->green & 0xff);
				png_write_register(PNG_TRNS_COLOR_B, trans_color->blue & 0xff);
			}
			else
			{
				png_write_register(PNG_TRNS_COLOR_R, trans_color->red);
				png_write_register(PNG_TRNS_COLOR_G, trans_color->green);
				png_write_register(PNG_TRNS_COLOR_B, trans_color->blue);

			}
		}
		MT_DBG_PNG("\r\n color_type:%d bit_depth:%d, trans gray:%04x, red:%04x, green:%04x, blue:%04x",
			color_type, bit_depth, trans_color->gray, trans_color->red, trans_color->green, trans_color->blue);
  }
	MT_DBG_PNG("\r\n is interlace:%d, with_tRNS:%d p_cfg->out_fmt: %d color_type: %d", interlace, with_tRNS, p_cfg->out_fmt, color_type);
  set_output_format(p_cfg->out_fmt, color_type, bit_depth, with_tRNS);

  png_write_register(PNG_FLT_START_ADDR, p_cfg->filter_buf_start);
  png_write_register(PNG_ZLIB_START_ADDR, p_cfg->zout_buf_start);
	png_write_register(PNG_ZLIB_END_ADDR, p_cfg->zout_buf_end);
	png_write_register(PNG_ZLIB_RD_ADDR, p_cfg->zout_buf_end + 1);

#ifdef CONFIG_MT_FPGA_GPE
    if(interlace && p_cfg->comp_dis)
    {
        u32 height_aligned = PNG_ALIGN(height, 8);
		get_img_buf_stride(TRUE, p_cfg->out_fmt, width, stride);
			
		png_write_register(PNG_PASS1_STRIDE, stride[0]);
		png_write_register(PNG_PASS2_STRIDE, stride[1]);
		png_write_register(PNG_PASS3_STRIDE, stride[2]);
		png_write_register(PNG_PASS4_STRIDE, stride[3]);
		png_write_register(PNG_PASS5_STRIDE, stride[4]);
		png_write_register(PNG_PASS6_STRIDE, stride[5]);
		png_write_register(PNG_PASS7_STRIDE, stride[6]);
		dtmp = p_cfg->img_buf_start;
		png_write_register(PNG_PASS1_START_ADDR, dtmp);	
		dtmp += stride[0] * height_aligned / 8;
		png_write_register(PNG_PASS2_START_ADDR, dtmp);
		dtmp += stride[1] * height_aligned / 8;
		png_write_register(PNG_PASS3_START_ADDR, dtmp);
		dtmp += stride[2] * height_aligned / 8;
		png_write_register(PNG_PASS4_START_ADDR, dtmp);
		dtmp += stride[3] * height_aligned / 4;
		png_write_register(PNG_PASS5_START_ADDR, dtmp);
		dtmp += stride[4] * height_aligned / 4;
		png_write_register(PNG_PASS6_START_ADDR, dtmp);
		dtmp += stride[5] * height_aligned / 2;
		png_write_register(PNG_PASS7_START_ADDR, dtmp);	
	}
	else
#endif	
    {
	    get_img_buf_stride(MT_FALSE, p_cfg->out_fmt, width, stride);
        png_write_register(PNG_PASS1_STRIDE, stride[0]);
        png_write_register(PNG_PASS1_START_ADDR, p_cfg->img_buf_start);
   }

 	png_write_register(PNG_BUF_START_ADDR, p_cfg->bs_buf_start);
	png_write_register(PNG_BUF_END_ADDR, p_cfg->bs_buf_end);
	png_write_register(PNG_BS_START_ADDR, p_cfg->bs_start);
	png_write_register(PNG_BS_END_ADDR, p_cfg->bs_end);

	if(p_cfg->eof_flag)
	{
		dtmp = png_read_register(PNG_ZLIB_CFG);
		dtmp |= (1 << 4);
		png_write_register(PNG_ZLIB_CFG, dtmp);
	}
	else
	{
		dtmp = png_read_register(PNG_ZLIB_CFG);
		dtmp &= ~(1 << 4);
		png_write_register(PNG_ZLIB_CFG, dtmp);
	}

	dump_reg();
	png_write_register(PNG_START, 0x1);
	return MT_SUCCESS;
}

mt_s32 png_hw_restart(mt_u32 bs_start, mt_u32 bs_end, MT_BOOL eof_flag)
{
	mt_u32 dtmp;

	if(eof_flag)
	{
		dtmp = png_read_register(PNG_ZLIB_CFG);
		dtmp |= (1 << 4);
		png_write_register(PNG_ZLIB_CFG, dtmp);
	}
	else
	{
		dtmp = png_read_register(PNG_ZLIB_CFG);
		dtmp &= ~(1 << 4);
		png_write_register(PNG_ZLIB_CFG, dtmp);
	}
	dtmp = png_read_register(PNG_STATE);
	dtmp |= PNG_STATE_BS_IN_DONE;
	png_write_register(PNG_STATE_CLR, dtmp);
	png_write_register(PNG_BS_START_ADDR, bs_start);
	png_write_register(PNG_BS_END_ADDR, bs_end);
	png_write_register(PNG_START, 0x10);
	MT_DBG_PNG("\r\n bs_start:0x%x, bs_end:0x%x, eof_flag:%d", bs_start, bs_end, eof_flag);
  return MT_SUCCESS;
}


MT_BOOL is_png_dec_done(void)
{
	if((png_read_register(PNG_STATE) & PNG_STATE_DONE) == PNG_STATE_DONE)
		return MT_TRUE;
	else
		return MT_FALSE;
}

MT_BOOL is_png_bs_in_done(void)
{
	if((png_read_register(PNG_STATE) & PNG_STATE_BS_IN_DONE) == PNG_STATE_BS_IN_DONE)
		return MT_TRUE;
	else
		return MT_FALSE;
}

MT_BOOL is_png_err_occured(void)
{
	if((png_read_register(PNG_STATE) & (0x01fff000)) != 0)
  {
    MT_ERR_PNG("\r\n error occured:%d", png_read_register(PNG_STATE));
		return MT_TRUE;
  }
	else
		return MT_FALSE;
}

#ifdef CONFIG_MT_FPGA_GPE
void png_int_enable(void)
{
    png_write_register(PNG_INT_EN, 0x01fff3ff);
    
    printf("<%s> : PNG_INT_EN : %x \n", __FUNCTION__, png_read_register(PNG_INT_EN));
}
void png_int_disable(void)
{
    png_write_register(PNG_INT_EN, 0);
    
    printf("<%s> : PNG_INT_EN : %x \n", __FUNCTION__, png_read_register(PNG_INT_EN));
}
#endif

