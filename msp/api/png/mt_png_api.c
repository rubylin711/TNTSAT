/******************************************************************************
* Montage Technology (Shanghai) Co., Ltd.                                                  
* Montage Proprietary and Confidential                                                     
* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies
******************************************************************************/

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
#include <sys/time.h>


#include "mt_drv_disp.h"
#include "png.h"
#include "mt_type.h"
#include "mt_png_api.h"
#include "mt_debug.h"
#include "png_hw.h"
#include "mt_module_debug.h"
#include "mpi_memdev.h"

#include "mt_cache.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

// #define MT_INFO_PNG printf

static const char *g_pszPngDevName = "/dev/mt_png";

static mt_s32 g_s32PngFd = -1;

static mt_s32 g_s32PngRef = 0;

static pthread_mutex_t png_mutex;

ulong png_reg_virt_addr;


static mt_s32 PNG_FreeMem(phys_addr_t u32Phyaddr)
{
	return mt_mmz_delete(u32Phyaddr);
}

static phys_addr_t PNG_AllocMem(mt_u32 u32Size , mt_u32 u32Align)
{
//	return mt_mmz_new(u32Size, u32Align, "png", "PNG");
  return mt_mmz_new(u32Size, u32Align, NULL, "png");
}
/*
static mt_void *PNG_Map(phys_addr_t u32PhyAddr)
{
	return mt_mmz_map(u32PhyAddr,MT_FALSE);
}
*/

static mt_void *PNG_MapCached(phys_addr_t u32PhyAddr)
{
	return mt_mmz_map(u32PhyAddr,MT_TRUE);
}
static mt_s32 PNG_Unmap(void* vAddr)
{
	return mt_mmz_unmap(vAddr);
}

static void defaultStreamReset(void* userdata)
{
    MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(userdata);
    MT_ASSERT(pstDecInfo != NULL);

    pstDecInfo->streamReadPtr = pstDecInfo->pStreamAddr;
}

static mt_u32 defaultStreamGetLength(void* userdata)
{
    MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(userdata);
    MT_ASSERT(pstDecInfo != NULL);

    return pstDecInfo->streamLen;
}

static mt_s32 defaultStreamGetData(void* userdata, mt_u32 length, void* ret_data, mt_u32* ret_read, MT_BOOL* eof)
{
    MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(userdata);
    MT_ASSERT(pstDecInfo != NULL);
    MT_ASSERT(ret_read != NULL);
    MT_ASSERT(ret_data != NULL);
    MT_ASSERT(eof != NULL);

    mt_u32 leftSize = pstDecInfo->pStreamAddr + pstDecInfo->streamLen - pstDecInfo->streamReadPtr;
    if (leftSize <= length)
    {
        *eof = MT_TRUE;
        *ret_read = leftSize;
    }
    else
    {
        *eof = MT_FALSE;
        *ret_read = length;
    }
    memcpy(ret_data, pstDecInfo->streamReadPtr, *ret_read);
    pstDecInfo->streamReadPtr += *ret_read;

    return MT_SUCCESS;
}

mt_s32 MT_PNG_SetStream(mt_handle hPngHandle, ulong addr, mt_u32 len)
{
  MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);
  pstDecInfo->pStreamAddr = (mt_char *)addr;
  pstDecInfo->streamLen = len;

  static MT_PNG_STREAM_CB funcs = {
      .Reset = defaultStreamReset,
      .GetData = defaultStreamGetData,
      .GetLength = defaultStreamGetLength
  };
  MT_PNG_SetStreamCallBack(hPngHandle, &funcs, (void *)hPngHandle);

  return MT_SUCCESS;
}

#ifdef CONFIG_MT_FPGA_GPE

static void png_write_register(ulong addr, unsigned int data)
{
    ulong *reg_addr;

    reg_addr = (ulong *)(addr + (ulong)png_reg_virt_addr);
 //   printf("<%s> : %lx %lx %x\n", reg_addr, png_reg_virt_addr, data);
	  mpi_write_reg32(reg_addr, data);

}

static unsigned int png_read_register(unsigned int addr)
{
    ulong *reg_addr;

    reg_addr = (ulong *)(addr + (ulong)png_reg_virt_addr);
   // printf("<%s> : %lx %lx\n", reg_addr, png_reg_virt_addr);
    return mpi_read_reg32(reg_addr);

}

void MT_PNG_reg_test(void)
{
    mt_u32 i = 0;
    mt_u32 dtmp = 0;
    mt_u32 arr[60][3]={
    	{0x0,0x0,2}, {0x10, 0x1,1}, {0x40, 0x0, 1}, {0x44,0x0,1}, 
    	{0x50,0x0,1}, {0x54,0x0,1}, {0x58, 0x0,1}, {0x80,0x0,1},
    	{0x84,0x0,1}, {0xe0,0x0,0}, {0xe4,0x0,0}, {0xe8,0x0,1}, 
    	{0xf0,0x0,2}, {0xf4,0x0,2}, {0x100,0x0,0}, {0x104,0x3300,1}, 
    	{0x108,0x0,0}, {0x10c,0x0,1}, {0x110,0x3,0}, {0x120,0x10,1}, 
    	{0x124,0x0,0}, {0x128,0x0,0}, {0x12c,0x0,0}, {0x130,0x0,0}, 
    	{0x134,0x0,0}, {0x140,0x0,0}, {0x144,0x0,0}, {0x150,0x0,1}, 
    	{0x154,0x0,0}, {0x158,0x0,0}, {0x16c,0x0,0}, {0x170,0x0,0}, 
    	{0x174,0x51001605,0}, {0x178,0x0,0}, {0x180,0x0,1}, {0x184,0x0,1}, 
    	{0x188,0x0,1}, {0x18c,0x0,1}, {0x190,0x0,0}, {0x1a0,0x0,1}, 
    	{0x1a4,0x0,1}, {0x1a8,0x0,1}, {0xab0,0x0,1}, {0x1b4,0x0,1}, 
    	{0x1b8,0x0,1}, {0x1bc,0x0,1}, {0x1c0,0x0,1}, {0x1c4,0x0,1}, 
    	{0x1c8,0x0,1}, {0x1d0,0x0,1}, {0x1e0,0x0,1}, {0x1e4,0x0,1}, 
    	{0x1e8,0x0,1}, {0x1ec,0x0,1}, {0x1f0,0x0,1}, {0x1f4,0x0,0}, 
    	{0x200,0x0,0}, {0x204,0x0,0}, {0x300,0x100,1}, {0x304,0x0,1}};

    for(i = 0; i < 60; i++)
    {
        if(arr[i][2] != 0)
        { 
            dtmp = png_read_register(arr[i][0]);
            if(dtmp != arr[i][1])
            {
                printf("\r\n reg error default addr:0x%08x, value:0x%08x", 0xbf1c1000 + arr[i][0], dtmp);
            }
        }
    }
    printf("\r\n reg default check finish\n");

    for(i = 1; i < 60; i++)
    {
    	if(arr[i][2] == 1)
    	{
    		png_write_register(arr[i][0], 0);
    		dtmp = png_read_register(arr[i][0]);
    		printf("\r\n addr:0x%08x, value:0x%08x", 0xbf1c1000 + arr[i][0], dtmp);
    	}
    }
    printf("\r\n reg write 0 check finish");	

    for(i = 1; i < 60; i++)
    {
        if(arr[i][2] == 1)
        {
            png_write_register(arr[i][0], 0xffffffff);
            dtmp = png_read_register(arr[i][0]);
            printf("\r\n addr:0x%08x, value:0x%08x", 0xbf1c1000 + arr[i][0], dtmp);		
        }
    }
    printf("\r\n reg write 0xffffffff check finish\n");
}
#endif


mt_s32 MT_PNG_Open(mt_handle *hPngHandle)
{
  MT_PNG_DECINFO_S *pPng;
  mt_u32 dtmp = 0;

  pPng = (MT_PNG_DECINFO_S *)malloc(sizeof(MT_PNG_DECINFO_S));
  if(pPng == NULL)
    return MT_ERR_PNG_NO_MEM;
  else
    *hPngHandle = (ulong)pPng;
  
  mt_sys_read_register(SYMPHONY_IO_PA(0xBF138140), &dtmp); 
  if(dtmp == (0x1 << 1))
  {
      dtmp = 0;
      mt_sys_read_register(SYMPHONY_IO_PA(0xBF138148), &dtmp);  //read auto start(auto clk)
      dtmp |=  (0x1 << 1);  //enable png auto start to enbale png IP clk start
      mt_sys_write_register(SYMPHONY_IO_PA(0xBF138148), dtmp);
  }

  //printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
  if (-1 != g_s32PngFd)
  {
    g_s32PngRef++;
    return MT_SUCCESS;
  }

  g_s32PngFd = open(g_pszPngDevName, O_RDWR, 0);
  if (g_s32PngFd < 0)
  {
    MT_ERR_PNG("[PNG] fail to open png module \n");
    return MT_ERR_PNG_DEV_OPEN_FAILED;
  }

  pthread_mutex_init(&png_mutex, MT_NULL);

  png_reg_virt_addr = (ulong)mmap(NULL, 0x400, PROT_READ | PROT_WRITE, MAP_SHARED, g_s32PngFd, 0);
  if ((void *)png_reg_virt_addr == MAP_FAILED)
  {
    MT_ERR_PNG("[PNG] fail to map png registers \n");
    return MT_ERR_PNG_DEV_OPEN_FAILED;
  }

  MT_INFO_PNG("MT_PNG_Open: reg base = 0x%x \n", png_reg_virt_addr);
  g_s32PngRef++;

  return MT_SUCCESS;
}

mt_void MT_PNG_Close(mt_handle hPngHandle)
{

  if (-1 == g_s32PngFd)
  {
    return;
  }
  g_s32PngRef--;

  if (g_s32PngRef > 0)
  {
    return;
  }
  else
  {
    g_s32PngRef = 0;
  }
#ifdef CONFIG_MT_FPGA_GPE
  png_int_disable();
#endif

  close(g_s32PngFd);

  g_s32PngFd = -1;

  if (png_reg_virt_addr != 0)
  {
    munmap((void *)png_reg_virt_addr, 0x400);
    png_reg_virt_addr = 0;
    MT_INFO_PNG("MT_PNG_Close: munmap ok\n");
  }

  pthread_mutex_destroy(&png_mutex);

  free((void *)hPngHandle);

  return;
}


pix_fmt_t colortype_to_pixfmt(mt_u32 color_type, mt_u32 color_depth)
{
	pix_fmt_t out_fmt;
	if(color_type == PNG_COLOR_TYPE_PALETTE)
	{
		if(color_depth == 8)
			out_fmt = PIX_FMT_RGBPALETTE8;
		else if(color_depth == 4)
			out_fmt = PIX_FMT_RGBPALETTE4;
		else if(color_depth == 2)
			out_fmt = PIX_FMT_RGBPALETTE2;
		else if(color_depth == 1)
			out_fmt = PIX_FMT_RGBPALETTE1;
		else
			out_fmt = PIX_FMT_RGBPALETTE8;
	}
	else if(color_type == PNG_COLOR_TYPE_GRAY)
	{
		if(color_depth == 1)
			out_fmt = PIX_FMT_GRAY_1;
		else if(color_depth == 2)
			out_fmt = PIX_FMT_GRAY_2;
		else if(color_depth == 4)
			out_fmt = PIX_FMT_GRAY_4;
		else if(color_depth == 8)
			out_fmt = PIX_FMT_GRAY_8;
		else
			out_fmt = PIX_FMT_GRAY_16;
	}
	else if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		if(color_depth == 8)
			out_fmt = PIX_FMT_AGRAY_8_8;
		else
			out_fmt = PIX_FMT_AGRAY_16_16;
	}
	else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		if(color_depth == 8)
			out_fmt = PIX_FMT_RGBA8888_SMALL_ENDIAN;
		else
			out_fmt = PIX_FMT_ABGR64;
	}
	else if(color_type == PNG_COLOR_TYPE_RGB)
	{
		if(color_depth == 8)
			out_fmt = PIX_FMT_BGR888;
		else
			out_fmt = PIX_FMT_BGR48;
	}
	else
		out_fmt = PIX_FMT_ARGB8888;
	return out_fmt;
}

static mt_u8 GetPixelbyIndex_sw(mt_u8 *p_dest, int x, int biBitCount)
{
  mt_u8 pos = 0;
  mt_u8 *p_iDst = (mt_u8 *)p_dest + (x * biBitCount >> 3);
  mt_u8 data = *p_iDst;
  if(biBitCount == 4)
  {
    pos = (mt_u8)(4 * (1 - x % 2));
    data = *p_iDst & (0x0F << pos);
  }
  else if(biBitCount == 2)
  {
    pos = (mt_u8)(2 * (3 - x % 4));
    data = *p_iDst & (0x3 << pos);
  }
  else if(biBitCount == 1)
  {
    pos = (mt_u8)(7 - x % 8);
    data = *p_iDst & (0x01 << pos);
  }

  return (mt_u8)(data >> pos);
}

#ifdef CONFIG_MT_FPGA_GPE
static void fmt_cvt_argb32(pix_fmt_t src_fmt, u32 w, u32 h, u8 *p_src, u8 *p_dst)
{
	u32 src_stride, dst_stride; 	
	u32 i, j;
	u32 dtmp;
      
	get_img_buf_stride(FALSE, src_fmt, w, &src_stride);
	get_img_buf_stride(FALSE, PIX_FMT_ARGB8888, w, &dst_stride);

	switch(src_fmt)
	{
	case PIX_FMT_RGBA8888_SMALL_ENDIAN:
        for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
                p_dst[i * dst_stride + 4*j] = p_src[i * src_stride + 4*j + 2];
                p_dst[i * dst_stride +  4*j + 1] = p_src[i * src_stride + 4*j + 1];
                p_dst[i * dst_stride +  4*j + 2] = p_src[i * src_stride + 4*j];
                p_dst[i * dst_stride +  4*j + 3] = p_src[i * src_stride + 4*j + 3];
#if 0				
                if((i == (h/2))&&( j < (w/2)))
    			    printf("\n--[%d %d] : 0x%2x", i, j,  p_src[i * src_stride + j]);
#endif
			}
		}		
		break;
	case PIX_FMT_BGR888:
        for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				p_dst[i * dst_stride + 4*j] = p_src[i * src_stride + 3*j + 2];
            	p_dst[i * dst_stride +  4*j + 1] = p_src[i * src_stride + 3*j + 1];
                p_dst[i * dst_stride +  4*j + 2] = p_src[i * src_stride + 3*j];
                p_dst[i * dst_stride +  4*j + 3] = 0xff;
                            
#if 0				
                if((i == (h/2))&&( j < (w/2)))
    				printf("\n-[%d %d] : 0x%2x", i, j,  p_src[i * src_stride + j]);
#endif
			}
		}		
		break;
            
		break;
	case PIX_FMT_GRAY_16:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				p_dst[i * dst_stride + 4 * j] = p_src[i * src_stride + j * 2 + 1];
				p_dst[i * dst_stride + 4 * j + 1] = p_src[i * src_stride + j * 2 + 1];
				p_dst[i * dst_stride + 4 * j + 2] = p_src[i * src_stride + j * 2 + 1];
				p_dst[i * dst_stride + 4 * j + 3] = 0xff;
			}
		}		
		break;		
	case PIX_FMT_AGRAY_16_16:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				p_dst[i * dst_stride + 4 * j] = p_src[i * src_stride + j * 4 + 1];
				p_dst[i * dst_stride + 4 * j + 1] = p_src[i * src_stride + j * 4 + 1];
				p_dst[i * dst_stride + 4 * j + 2] = p_src[i * src_stride + j * 4 + 1];
				p_dst[i * dst_stride + 4 * j + 3] = p_src[i * src_stride + j * 4 + 3];
			}
		}		
		break;			
	case PIX_FMT_GRAY_8:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				p_dst[i * dst_stride + 4 * j] = p_src[i * src_stride + j];
				p_dst[i * dst_stride + 4 * j + 1] = p_src[i * src_stride + j];
				p_dst[i * dst_stride + 4 * j + 2] = p_src[i * src_stride + j];
				p_dst[i * dst_stride + 4 * j + 3] = 0xff;
#if 0				
				mtos_printk("\r\n i:%d, j:%d, src:%02x, dst:%02x,%02x,%02x,%02x",
					i, j, p_src[i * src_stride + j], p_dst[i * dst_stride + 4 * j], p_dst[i * dst_stride + 4 * j + 1],
					p_dst[i * dst_stride + 4 * j + 2], p_dst[i * dst_stride + 4 * j + 3]);
#endif
#if 0				
                 if((i == (h/2)) && ( j < (w/2)))
                     printf("\n-[%d %d] : 0x%2x", i, j,  p_src[i * src_stride + j]);
#endif

			}
		}		
		break;
	case PIX_FMT_AGRAY_8_8:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				p_dst[i * dst_stride + 4 * j] = p_src[i * src_stride + j * 2];
				p_dst[i * dst_stride + 4 * j + 1] = p_src[i * src_stride + j * 2];
				p_dst[i * dst_stride + 4 * j + 2] = p_src[i * src_stride + j * 2];
				p_dst[i * dst_stride + 4 * j + 3] = p_src[i * src_stride + j * 2 + 1];
			}
		}		
		break;
	case PIX_FMT_GRAY_4:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				dtmp = p_src[i * src_stride + j / 2];
				if(j % 2 == 1)
					dtmp = dtmp & 0xf;
				else
					dtmp = (dtmp >> 4) & 0xf;
				p_dst[i * dst_stride + 4 * j] = (dtmp << 4) | dtmp;
				p_dst[i * dst_stride + 4 * j + 1] = (dtmp << 4) | dtmp;
				p_dst[i * dst_stride + 4 * j + 2] = (dtmp << 4) | dtmp;
				p_dst[i * dst_stride + 4 * j + 3] = 0xff;
			}
		}		
		break;
	case PIX_FMT_GRAY_2:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				dtmp = p_src[i * src_stride + j / 4];
				dtmp = (dtmp >> (6 - (j % 4) * 2)) & 0x3;
				p_dst[i * dst_stride + 4 * j] = (dtmp << 6) | (dtmp << 4) | (dtmp << 2) | dtmp;
				p_dst[i * dst_stride + 4 * j + 1] = (dtmp << 6) | (dtmp << 4) | (dtmp << 2) | dtmp;
				p_dst[i * dst_stride + 4 * j + 2] = (dtmp << 6) | (dtmp << 4) | (dtmp << 2) | dtmp;
				p_dst[i * dst_stride + 4 * j + 3] = 0xff;
			}
		}		
		break;
	case PIX_FMT_GRAY_1:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				dtmp = p_src[i * src_stride + j / 8];
				dtmp = (dtmp >> (7 - (j % 8))) & 0x1;
		
				p_dst[i * dst_stride + 4 * j] = (dtmp << 7) | (dtmp << 6) | (dtmp << 5) | (dtmp << 4) 
					| (dtmp << 3)	| (dtmp << 2) | (dtmp << 1) | dtmp;
				p_dst[i * dst_stride + 4 * j + 1] = (dtmp << 7) | (dtmp << 6) | (dtmp << 5) | (dtmp << 4) 
					| (dtmp << 3)	| (dtmp << 2) | (dtmp << 1) | dtmp;
				p_dst[i * dst_stride + 4 * j + 2] = (dtmp << 7) | (dtmp << 6) | (dtmp << 5) | (dtmp << 4) 
					| (dtmp << 3)	| (dtmp << 2) | (dtmp << 1) | dtmp;
				p_dst[i * dst_stride + 4 * j + 3] = 0xff;
			}
		}		
		break;
	case PIX_FMT_ABGR64:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				p_dst[i * dst_stride + 4 * j] = p_src[i * src_stride + j * 8 + 5];
				p_dst[i * dst_stride + 4 * j + 1] = p_src[i * src_stride + j * 8 + 3];
				p_dst[i * dst_stride + 4 * j + 2] = p_src[i * src_stride + j * 8 + 1];
				p_dst[i * dst_stride + 4 * j + 3] = p_src[i * src_stride + j * 8 + 7];
			}
		}			
		break;
	case PIX_FMT_BGR48:
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				p_dst[i * dst_stride + 4 * j] = p_src[i * src_stride + j * 6 + 5];
				p_dst[i * dst_stride + 4 * j + 1] = p_src[i * src_stride + j * 6 + 3];
				p_dst[i * dst_stride + 4 * j + 2] = p_src[i * src_stride + j * 6 + 1];
				p_dst[i * dst_stride + 4 * j + 3] = 0xff;
			}
		}			
		break;
        default:
            printf("fmt_cvt_argb32: format not support(%d)\n", src_fmt);
            break;
	}
	
}


static u32 calc_img_buf_size(MT_BOOL is_interlace, pix_fmt_t out_fmt, u32 width, u32 height)
{
	u32 height_arr[7] = {0};
	//u32 bpp;
	//u32 width_aligned = PNG_ALIGN(width, 8);
	u32 height_aligned = PNG_ALIGN(height, 8);
	int i;
	u32 stride[7] = {0};
	u32 size = 0;

	//bpp = png_get_bpp(out_fmt);
	get_img_buf_stride(is_interlace, out_fmt, width, stride);
	
  if(is_interlace)
  {
		height_arr[0] = height_aligned / 8;
		height_arr[1] = height_arr[0];
		height_arr[2] = height_arr[0];
		height_arr[3] = height_aligned / 4;
		height_arr[4] = height_arr[3];
		height_arr[5] = height_aligned / 2;
		height_arr[6] = height_arr[5];

		for(i = 0; i < 7; i++)
			size += stride[i] * height_arr[i];
  }
	else
	{
		size = stride[0] * height;
	}
	return size + PNG_ALIGN_N;
}
#endif

mt_s32 MT_PNG_SetStreamCallBack(mt_handle hPngHandle, MT_PNG_STREAM_CB* funcs, void* userdata)
{
    MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);
    pstDecInfo->streamFuncs = funcs;
    pstDecInfo->userdata = userdata;    
    return MT_SUCCESS;
}

mt_s32 MT_PNG_SetDecInfo(mt_handle hPngHandle, png_structp pPng, png_infop pInfo)
{
  MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);
  pstDecInfo->pInfo = pInfo;
  pstDecInfo->pPng = pPng;
  return MT_SUCCESS;
}

mt_s32 MT_PNG_SetOutImgInfo(mt_handle hPngHandle, const MT_PNG_IMAGE_INFO_S *imageInfo)
{
  MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);
  MT_ASSERT(pstDecInfo != NULL);
  MT_ASSERT(imageInfo != NULL);

  pstDecInfo->pImgPhyAddr = imageInfo->phyAddr;
  pstDecInfo->pImgVirAddr = imageInfo->virAddr;
  pstDecInfo->outImageFmt = imageInfo->pixFormat;
  return MT_SUCCESS;
}


mt_s32 MT_PNG_Decode(mt_handle hPngHandle)
{
  mt_u32 streamBufSize;
  mt_u32 filterBufSize;
  phys_addr_t pSrcPhyBuf = 0;
  mt_char *pSrcVirBuf = MT_NULL;
  phys_addr_t pFilterPhyBuf = 0;
  phys_addr_t pZoutPhyBuf = 0;
  phys_addr_t pImgPhyBuf = 0;
  mt_char *pImgVirBuf = MT_NULL;
  png_cfg_t png_cfg = {0};
  MT_BOOL eof_flag = MT_FALSE;
  png_structp pngdec_ptr = MT_NULL;
  png_infop info_ptr = MT_NULL;
  mt_u32 w = 0;
  mt_u32 h = 0;
  mt_u32 color_depth = 0;
  mt_u32 interlace = 0;
  mt_u32 color_type = 0;
  mt_u32 stride = 0;
  mt_u32 read_size = 0;
  struct timeval t1, t2;
  mt_u32 force_stop = 0;
  MT_BOOL handlePalette = MT_FALSE;
  MT_PNG_STREAM_CB* streamFuncs = NULL;
  MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);
  mt_u32 i = 0, j = 0;

    png_colorp pallet = NULL;
    int num_palette = 0;
    mt_u32 src_stride = 0, dst_stride = 0;
    mt_u8 *p_dst = NULL;
    mt_u8 *p_src = NULL;
    mt_u8 pixel = 0;

    png_bytep trans_alpha = NULL;
    int num_trans = 0;
    png_color_16p trans_color = NULL;
#ifdef CONFIG_MT_FPGA_GPE	
    mt_u32 *p_src_data = NULL;
    mt_u32 size = 0;
#endif
    
  if (MT_NULL == pstDecInfo || pstDecInfo->streamFuncs == NULL)
  {
      return MT_ERR_PNG_NULL_PTR;
  }
  if (g_s32PngFd == -1)
  {
      return MT_ERR_PNG_DEV_NOT_OPEN;
  }
  streamFuncs = pstDecInfo->streamFuncs;
  streamFuncs->Reset(pstDecInfo->userdata);
  streamBufSize = streamFuncs->GetLength(pstDecInfo->userdata);
  if (streamBufSize > PNG_BS_BUF_SIZE)
  {
    streamBufSize = PNG_BS_BUF_SIZE;
  }
  else
  {
      streamBufSize = (streamBufSize+ PNG_ALIGN_N - 1) & (~(PNG_ALIGN_N - 1));
  }

  pngdec_ptr = pstDecInfo->pPng;
  info_ptr = pstDecInfo->pInfo;
	w = png_get_image_width(pngdec_ptr, info_ptr);
	h = png_get_image_height(pngdec_ptr, info_ptr);
	color_depth = png_get_bit_depth(pngdec_ptr, info_ptr);
	interlace = (png_get_interlace_type(pngdec_ptr, info_ptr) == PNG_INTERLACE_NONE) ? 0 : 1;
	color_type = png_get_color_type(pngdec_ptr, info_ptr);

    pSrcPhyBuf = PNG_AllocMem(streamBufSize, PNG_ALIGN_N);
	if(MT_NULL == pSrcPhyBuf)
	{
		MT_ERR_PNG("PNG_AllocMem failure\n");
        return MT_ERR_PNG_NO_MEM;
	}
    pSrcVirBuf = (mt_char*)PNG_MapCached(pSrcPhyBuf);
	if(MT_NULL == pSrcVirBuf)
	{
	  PNG_FreeMem(pSrcPhyBuf);
      MT_ERR_PNG("PNG_MapCached failure\n");
      return MT_ERR_PNG_MEM_MAP_FAILED;
	}
    filterBufSize = calc_filter_buf_size(pstDecInfo->pPng, pstDecInfo->pInfo);
    pFilterPhyBuf = PNG_AllocMem(filterBufSize, PNG_ALIGN_N);
	if(MT_NULL == pFilterPhyBuf)
	{
      PNG_Unmap(pSrcVirBuf);
      PNG_FreeMem((phys_addr_t)pSrcPhyBuf);
      MT_ERR_PNG("PNG_AllocMem failure\n");
      return MT_ERR_PNG_NO_MEM;
	}
    pZoutPhyBuf = PNG_AllocMem(PNG_ZOUT_BUF_SIZE, PNG_ALIGN_N);
	if(MT_NULL == pZoutPhyBuf)
	{
      PNG_Unmap(pSrcVirBuf);
      PNG_FreeMem((phys_addr_t)pSrcPhyBuf);
      PNG_FreeMem(pFilterPhyBuf);
	  MT_ERR_PNG("PNG_MapCached failure\n");
      return MT_ERR_PNG_NO_MEM;
	}

    streamFuncs->GetData(pstDecInfo->userdata, streamBufSize, pSrcVirBuf, &read_size, &eof_flag);
    mt_mmz_flush(pSrcVirBuf, 0, 0);


  png_cfg.bs_buf_start = pSrcPhyBuf;
  png_cfg.bs_buf_end = pSrcPhyBuf + streamBufSize - 1;
  png_cfg.filter_buf_start = pFilterPhyBuf;
  png_cfg.zout_buf_start = pZoutPhyBuf;
  png_cfg.zout_buf_end = pZoutPhyBuf + PNG_ZOUT_BUF_SIZE - 1;
#ifdef CONFIG_MT_FPGA_GPE  
  png_cfg.comp_dis = (interlace == PNG_INTERLACE_NONE) ? MT_TRUE : pstDecInfo->dbg_cfg.comp_dis;
#else
	png_cfg.comp_dis = (interlace == PNG_INTERLACE_NONE) ? MT_TRUE : MT_FALSE;
#endif



	if(color_type == PNG_COLOR_TYPE_PALETTE)
	{		
#ifdef CONFIG_MT_FPGA_GPE
		if(pstDecInfo->dbg_cfg.clut_gray_to_8bit)
			png_cfg.out_fmt = PIX_FMT_RGBPALETTE8;
		else
#endif			   
			png_cfg.out_fmt = colortype_to_pixfmt(color_type, color_depth);
        handlePalette = MT_TRUE;

	}
#ifdef CONFIG_MT_FPGA_GPE
	else if(color_type == PNG_COLOR_TYPE_GRAY)
	{
		if(pstDecInfo->dbg_cfg.clut_gray_to_8bit)
		{
			png_cfg.out_fmt = PIX_FMT_GRAY_8;
		}
		else if(pstDecInfo->dbg_cfg.conv2argb_dis)
		{
			png_cfg.out_fmt = colortype_to_pixfmt(color_type, color_depth);
		}
		else
		{
			png_cfg.out_fmt = pstDecInfo->outImageFmt;
		}
	}
	else if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		if(pstDecInfo->dbg_cfg.clut_gray_to_8bit)
		{
			png_cfg.out_fmt = PIX_FMT_AGRAY_8_8;
		}
		else if(pstDecInfo->dbg_cfg.conv2argb_dis)
		{
			png_cfg.out_fmt = colortype_to_pixfmt(color_type, color_depth);
		}		
		else	
		{
			png_cfg.out_fmt = pstDecInfo->outImageFmt;
		}
	}
#endif	
	else
	{
#ifdef CONFIG_MT_FPGA_GPE
		if(pstDecInfo->dbg_cfg.conv2argb_dis)
		{
			png_cfg.out_fmt = colortype_to_pixfmt(color_type, color_depth); 					
		}
		else
#endif
		{
			png_cfg.out_fmt =  pstDecInfo->outImageFmt;
		}
	}

  if(handlePalette && png_cfg.out_fmt != pstDecInfo->outImageFmt)
  {
      handlePalette = MT_TRUE;
  }
  else
  {
      handlePalette = MT_FALSE;
  }
  
  #ifdef CONFIG_MT_FPGA_GPE	
    if(interlace && png_cfg.comp_dis)
    {
    	printf("\r\n comp disabled!!!!!!!!!!");
    	if(pstDecInfo->dbg_cfg.conv2argb_dis)
    		png_cfg.out_fmt = colortype_to_pixfmt(color_type, color_depth); 
    	else
    		png_cfg.out_fmt = PIX_FMT_ARGB8888;
        
    	size = calc_img_buf_size(TRUE, png_cfg.out_fmt, w, h);
       pImgPhyBuf = PNG_AllocMem(size, PNG_ALIGN_N);
      	if(MT_NULL == pImgPhyBuf)
      	{
            PNG_Unmap(pSrcVirBuf);
            PNG_FreeMem(pSrcPhyBuf);
            PNG_FreeMem(pFilterPhyBuf);
            PNG_FreeMem(pZoutPhyBuf);
            MT_ERR_PNG("PNG_AllocMem failure\n");
            return MT_ERR_PNG_NO_MEM;
      	}
        pImgVirBuf = (mt_char*)PNG_MapCached((phys_addr_t)pImgPhyBuf);
      	if(MT_NULL == pImgVirBuf)
      	{
            PNG_Unmap(pSrcVirBuf);
            PNG_FreeMem(pSrcPhyBuf);
            PNG_FreeMem(pFilterPhyBuf);
            PNG_FreeMem(pZoutPhyBuf);
            PNG_FreeMem(pImgPhyBuf);
            MT_ERR_PNG("PNG_MapCached failure\n");
            return MT_ERR_PNG_MEM_MAP_FAILED;
      	}
       png_cfg.img_buf_start = pImgPhyBuf;    
       
   //    printf("\r\n--><%s> :<%d>  handlePalette== %d   pImgVirBuf: %lx %llx",__FUNCTION__, __LINE__, handlePalette, (ulong)pImgVirBuf, pImgPhyBuf);
    	
    }
    else
#endif	
    {
        if(png_cfg.out_fmt == pstDecInfo->outImageFmt)// (!handlePalette )
        {
          png_cfg.img_buf_start = pstDecInfo->pImgPhyAddr;
          //printf("\r\n--handlePalette == 0!!!!!!!!!!");
        }
        else
        {
            get_img_buf_stride(MT_FALSE, png_cfg.out_fmt, w, &stride);
            pImgPhyBuf = (phys_addr_t)PNG_AllocMem(stride * h, PNG_ALIGN_N);
            
            if(MT_NULL == pImgPhyBuf)
            {
                PNG_Unmap(pSrcVirBuf);
                PNG_FreeMem(pSrcPhyBuf);
                PNG_FreeMem(pFilterPhyBuf);
                PNG_FreeMem(pZoutPhyBuf);
                MT_ERR_PNG("PNG_AllocMem failure\n");
                return MT_ERR_PNG_NO_MEM;
            }
            pImgVirBuf = (mt_char*)PNG_MapCached((phys_addr_t)pImgPhyBuf);
            
            if(MT_NULL == pImgVirBuf)
            {
                PNG_Unmap(pSrcVirBuf);
                PNG_FreeMem(pSrcPhyBuf);
                PNG_FreeMem(pFilterPhyBuf);
                PNG_FreeMem(pZoutPhyBuf);
                PNG_FreeMem(pImgPhyBuf);
                MT_ERR_PNG("PNG_MapCached failure\n");
                return MT_ERR_PNG_MEM_MAP_FAILED;
            }
            png_cfg.img_buf_start = pImgPhyBuf;           
        }
    }
  png_cfg.eof_flag = eof_flag;
  png_cfg.bs_start = (phys_addr_t)pSrcPhyBuf + 8;
  png_cfg.bs_end = (phys_addr_t)pSrcPhyBuf + read_size - 1;

  png_hw_start(pstDecInfo->pPng, pstDecInfo->pInfo, &png_cfg);

	gettimeofday(&t1, MT_NULL);
	while(1)
	{
		if(is_png_dec_done())
			break;
		if(is_png_bs_in_done() && !eof_flag)
		{
            streamFuncs->GetData(pstDecInfo->userdata, streamBufSize, pSrcVirBuf, &read_size, &eof_flag);
            if (read_size > 0)
            {
                mt_mmz_flush(pSrcVirBuf, 0, 0);     
                png_hw_restart((phys_addr_t)pSrcPhyBuf, (phys_addr_t)pSrcPhyBuf + read_size - 1, eof_flag);
            }
		}

    if(is_png_err_occured())
    {
        MT_ERR_PNG("error occured, force exit\n");
        png_hw_force_exit();
        force_stop = 1;
        break;
    }
#ifdef CONFIG_MT_FPGA_GPE
		if(pstDecInfo->dbg_cfg.reset_test)
		{
			mt_u32 dtmp = png_read_register(PNG_STATE);
			png_hw_force_exit();
			printf("\r\n~~~~~reset_test : force exit, png state:0x%08x", dtmp);
			break;
		}
			
#endif
    gettimeofday(&t2, MT_NULL);
    if(MAX_PNG_TIME < (((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000))
    {
      MT_ERR_PNG("hw timeout, force exit\n");
			dump_reg();
			png_hw_force_exit();
			force_stop = 2;
			break;
    }

		MT_USLEEP(1000);
	}

  if(force_stop)
  {
    PNG_Unmap(pSrcVirBuf);
    PNG_FreeMem(pSrcPhyBuf);
    PNG_FreeMem(pFilterPhyBuf);
    PNG_FreeMem(pZoutPhyBuf);
    if(pImgPhyBuf)
    {
      PNG_Unmap(pImgVirBuf);
      PNG_FreeMem(pImgPhyBuf);
    }
    if(force_stop == 1)
    {
      return MT_ERR_PNG_ERR_OCCURED;
    }
    else if(force_stop == 2)
    {
      return MT_ERR_PNG_JOB_TIMEOUT;
    }
  }
#ifdef CONFIG_MT_FPGA_GPE	
    if(interlace && png_cfg.comp_dis)
    {
        u32 height_aligned = PNG_ALIGN(h, 8);
        u32 stride_covt[7] = {0};
        u32 *p_pass[7];
        u32 *p_dst_covt;     
        int k;
        unsigned int png_pass_h_inc[7] = {8, 8, 4, 4, 2, 2, 1};
        unsigned int png_pass_h_offset[7] = {0, 4, 0, 2, 0, 1, 0};
        unsigned int png_pass_v_inc[7] = {8, 8, 8, 4, 4, 2, 2};
        unsigned int png_pass_v_offset[7] = {0, 0, 4, 0, 2, 0, 1};
        u32 xoffset;
        //      MT_ASSERT(png_cfg.out_fmt == PIX_FMT_ARGB8888);
        get_img_buf_stride(TRUE, png_cfg.out_fmt, w, stride_covt);     
        get_img_buf_stride(FALSE, PIX_FMT_ARGB8888, w, &dst_stride);

        p_pass[0] = (u32 *)pImgVirBuf;
        p_pass[1] = (u32 *)((ulong)p_pass[0] + stride_covt[0] * height_aligned / 8);
        p_pass[2] = (u32 *)((ulong)p_pass[1] + stride_covt[1] * height_aligned / 8);
        p_pass[3] = (u32 *)((ulong)p_pass[2] + stride_covt[2] * height_aligned / 8);
        p_pass[4] = (u32 *)((ulong)p_pass[3] + stride_covt[3] * height_aligned / 4);
        p_pass[5] = (u32 *)((ulong)p_pass[4] + stride_covt[4] * height_aligned / 4);
        p_pass[6] = (u32 *)((ulong)p_pass[5] + stride_covt[5] * height_aligned / 2);

        if(png_cfg.out_fmt == PIX_FMT_ARGB8888)
        {
            mt_u32 bpp = 4;
            
     //       printf("<%s> : <%d> <%x %x>\n", __FUNCTION__, __LINE__, p_pass[0], pImgVirBuf);    
            for(k = 0; k < 7; k++)
            {
                for(i = 0; i < height_aligned / png_pass_v_inc[k]; i++)
                {
                    p_src_data = (u32 *)((ulong)p_pass[k] + stride_covt[k] * i);
                    p_dst_covt = (u32 *)((ulong)pstDecInfo->pImgVirAddr  + dst_stride * (i * png_pass_v_inc[k] + png_pass_v_offset[k]));
                 //   printf("\r\n p_src:%x %x", p_src, p_dst);
                    if((i * png_pass_v_inc[k] + png_pass_v_offset[k]) >= h)
                      continue;
               //     printf("\r\n dst line:%d", (i * png_pass_v_inc[k] + png_pass_v_offset[k]));
                    for(j = 0; j < stride_covt[k] / bpp; j++)
                    {
                        xoffset = png_pass_h_offset[k] + j * png_pass_h_inc[k];
                        if(xoffset >= w)
                          continue;
                        p_dst_covt[xoffset] = p_src_data[j];
                      //  printf("\r\n xoffset:%d, i:%d, j:%d, k:%d, 0x%08x, 0x%08x", xoffset, i ,j, k, p_src[j], &p_dst[xoffset]);
                    }
                }
            }
        }
          else if(png_cfg.out_fmt == PIX_FMT_GRAY_1)
          {
              u8 dtmp;
        //       printf("<%s> : <%d> <%x %x>\n", __FUNCTION__, __LINE__, p_pass[0], pImgVirBuf);
              for(k = 0; k < 7; k++)
              {
                  for(i = 0; i < height_aligned / png_pass_v_inc[k]; i++)
                  {
                      p_src = (u8 *)((ulong)p_pass[k] + stride_covt[k] * i);
                      p_dst_covt = (u32 *)((ulong)pstDecInfo->pImgVirAddr  + dst_stride * (i * png_pass_v_inc[k] + png_pass_v_offset[k]));
                      
                      if((i * png_pass_v_inc[k] + png_pass_v_offset[k]) >= h)
                          continue;
                      
                  //  printf("\r\n dst line:%d", (i * png_pass_v_inc[k] + png_pass_v_offset[k]));
                      for(j = 0; j < stride_covt[k] * 8; j+=8)
                      {
                          xoffset = png_pass_h_offset[k] + j * png_pass_h_inc[k];                     
                          if(xoffset >= w)
                              continue;
                   //     printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 7) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;                      
                          xoffset = png_pass_h_offset[k] + (j + 1) * png_pass_h_inc[k];
                          
                          if(xoffset >= w)
                              continue;
                          
                  //      printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 6) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;  
                          
                          xoffset = png_pass_h_offset[k] + (j + 2) * png_pass_h_inc[k];
                          if(xoffset >= w)
                              continue;
                          
                    //    printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 5) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;  
                          
                          xoffset = png_pass_h_offset[k] + (j + 3) * png_pass_h_inc[k];
                          if(xoffset >= w)
                              continue;
                          
                     //   printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 4) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;  
                          
                          xoffset = png_pass_h_offset[k] + (j + 4) * png_pass_h_inc[k];
                          if(xoffset >= w)
                              continue;
                          
                    //    printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 3) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;  
                          
                          xoffset = png_pass_h_offset[k] + (j + 5) * png_pass_h_inc[k];
                          if(xoffset >= w)
                              continue;
                          
                  //      printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 2) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;  
                          
                          xoffset = png_pass_h_offset[k] + (j + 6) * png_pass_h_inc[k];
                          if(xoffset >= w)
                              continue;
                          
                //        printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 1) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;  
                          
                          xoffset = png_pass_h_offset[k] + (j + 7) * png_pass_h_inc[k];
                          if(xoffset >= w)
                              continue;
                          
                   //     printf("\r\n k:%d, i:%d, j:%d, src:%02x, xoffset:%d", k, i, j, p_src[j/8], xoffset);
                          dtmp = ((p_src[j/8] >> 0) & 0x1) * 0xff;
                          p_dst_covt[xoffset] = (0xff << 24) | (dtmp << 16) | (dtmp << 8) | dtmp;                  
                                                                          
                      }
                  }
              }           
          }

            mt_mmz_flush(pstDecInfo->pImgVirAddr, 0, 0);              
 //       printf("-----------------------------><%s> : <%d> remove mt_mmz_flush tmp\n", __FUNCTION__, __LINE__);
            PNG_Unmap(pImgVirBuf);
            PNG_FreeMem(pImgPhyBuf);
      }
#endif	
//printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);

#ifdef CONFIG_MT_FPGA_GPE	
    if(interlace == FALSE || png_cfg.comp_dis == FALSE)
    {
        if (png_cfg.out_fmt != pstDecInfo->outImageFmt)
        {
           if(png_cfg.out_fmt == PIX_FMT_RGBPALETTE1 ||
        		png_cfg.out_fmt == PIX_FMT_RGBPALETTE2 ||
        		png_cfg.out_fmt == PIX_FMT_RGBPALETTE4 ||
        		png_cfg.out_fmt == PIX_FMT_RGBPALETTE8 )
#else
  if (handlePalette)					
#endif
  {

    png_get_tRNS(pngdec_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
    png_get_PLTE(pngdec_ptr, info_ptr, &pallet, &num_palette);

    get_img_buf_stride(MT_FALSE, png_cfg.out_fmt, w, &src_stride);
    get_img_buf_stride(MT_FALSE, pstDecInfo->outImageFmt, w, &dst_stride);
    if(png_cfg.out_fmt == PIX_FMT_RGBPALETTE8)
      color_depth = 8;

    mt_mmz_invalidate(pImgVirBuf, 0, 0);		/* invalid whole buffer */
    for(i = 0; i < h; i++)
    {
      p_dst = (mt_u8 *)pstDecInfo->pImgVirAddr + dst_stride * i;
      p_src = (mt_u8 *)pImgVirBuf + src_stride * i;

      for(j=0; j < w;j ++)
      {
        pixel = GetPixelbyIndex_sw(p_src, j, color_depth);
        p_dst[0] = pallet[pixel].blue;
        p_dst[1] = pallet[pixel].green;
        p_dst[2] = pallet[pixel].red;

        if(trans_alpha && pixel < num_trans)
          p_dst[3] = trans_alpha[pixel];
        else
          p_dst[3] = 0xff;
        p_dst += 4;
      }
    }
    mt_mmz_flush(pstDecInfo->pImgVirAddr, 0, 0);    
    PNG_Unmap(pImgVirBuf);
    PNG_FreeMem(pImgPhyBuf);
  }
#ifdef CONFIG_MT_FPGA_GPE	
   else if(png_cfg.out_fmt != PIX_FMT_ARGB8888)
   {
       mt_mmz_invalidate(pImgVirBuf, 0, 0);		/* invalid whole buffer */
       fmt_cvt_argb32(png_cfg.out_fmt, w, h, (u8 *)pImgVirBuf, (u8 *)pstDecInfo->pImgVirAddr);
       mt_mmz_flush(pstDecInfo->pImgVirAddr, 0, 0);    
       PNG_Unmap(pImgVirBuf);
       PNG_FreeMem(pImgPhyBuf);
   }
   }  
   else
   {
       if(png_cfg.out_fmt != PIX_FMT_ARGB8888)
       {
          mt_mmz_invalidate(pImgVirBuf, 0, 0);		/* invalid whole buffer */
          fmt_cvt_argb32(png_cfg.out_fmt, w, h, (u8 *)pImgVirBuf, (u8 *)pstDecInfo->pImgVirAddr);
          mt_mmz_flush(pstDecInfo->pImgVirAddr, 0, 0);    
          PNG_Unmap(pImgVirBuf);
          PNG_FreeMem(pImgPhyBuf);
       }
   }
   }
   printf("\r\n ---->idat_cnt:%d", png_read_register(PNG_IDAT_CNT));   
   printf("\r\n outfmt:%d", png_cfg.out_fmt);
#endif

  PNG_Unmap(pSrcVirBuf);
  PNG_FreeMem(pSrcPhyBuf);
  PNG_FreeMem(pFilterPhyBuf);
  PNG_FreeMem(pZoutPhyBuf);
  return MT_SUCCESS;
}


mt_s32 MT_PNG_GetPalette(mt_handle hPngHandle, mt_u32 *Palette, mt_u32 size)
{
	MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);
	png_structp pngdec_ptr = MT_NULL;
	png_infop info_ptr = MT_NULL;
	png_colorp pallet = NULL;
	int num_palette = 0;
	int i  = 0;
	png_byte alpha = 0xff;

	png_bytep trans_alpha = NULL;
	int num_trans = 0;
	png_color_16p trans_color = NULL;
	mt_s32 ret = MT_FAILURE;

	
	if (MT_NULL == pstDecInfo)
	{
		return MT_ERR_PNG_NULL_PTR;
	}
	pngdec_ptr = pstDecInfo->pPng;
	info_ptr = pstDecInfo->pInfo;

	png_get_tRNS(pngdec_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
	png_get_PLTE(pngdec_ptr, info_ptr, &pallet, &num_palette);

	if(num_palette > 0 && pallet != NULL)
	{
		for(i =0; i < num_palette && i < size; i++)
		{
	              if(trans_alpha && i < num_trans)
		        	alpha = trans_alpha[i];
		        else
		       	alpha = 0xff;
			Palette[i] =  (alpha << 24)|(pallet[i].red << 16) |  (pallet[i].green << 8) |  pallet[i].blue;
		}
		ret = MT_SUCCESS;
	}
 	 return ret;

}

mt_s32 MT_PNG_GetRowBytes(mt_handle hPngHandle, pix_fmt_t pixForamt, mt_u32 *pstStride)
{
  png_structp pngdec_ptr = MT_NULL;
  png_infop info_ptr = MT_NULL;
  mt_u32 w = 0;
  MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);

  if (MT_NULL == pstDecInfo)
  {
      return MT_ERR_PNG_NULL_PTR;
  }
  if (g_s32PngFd == -1)
  {
      return MT_ERR_PNG_DEV_NOT_OPEN;
  }

  pngdec_ptr = pstDecInfo->pPng;
  info_ptr = pstDecInfo->pInfo;
  w = png_get_image_width(pngdec_ptr, info_ptr);

  get_img_buf_stride(MT_FALSE, pixForamt, w, pstStride);

  return MT_SUCCESS;
}

#ifdef CONFIG_MT_FPGA_GPE
MT_BOOL MT_PNG_IfHwSupport(MT_BOOL dbg_HWSupport)
{
	printf("<%s> : <%d> hwsupport : %d\n", __FUNCTION__, __LINE__, dbg_HWSupport);
		return (dbg_HWSupport);		
}

mt_s32 MT_PNG_SetDbgInfo(mt_handle hPngHandle, MT_PNG_DBGCFG_S *dbg_cfg)
{
    MT_PNG_DECINFO_S *pstDecInfo = (MT_PNG_DECINFO_S *)(hPngHandle);
    pstDecInfo->dbg_cfg.comp_dis = dbg_cfg->comp_dis;
    pstDecInfo->dbg_cfg.sw_dec = dbg_cfg->sw_dec;
    pstDecInfo->dbg_cfg.clut_gray_to_8bit = dbg_cfg->clut_gray_to_8bit;
    pstDecInfo->dbg_cfg.conv2argb_dis = dbg_cfg->conv2argb_dis;
    pstDecInfo->dbg_cfg.reset_test = dbg_cfg->reset_test;
  return MT_SUCCESS;
}


#else
MT_BOOL MT_PNG_IfHwSupport(png_structp pPng, png_infop pInfo)
{
  return MT_TRUE;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
