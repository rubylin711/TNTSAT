/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifdef CONFIG_MT_FPGA_GPE
#include <inttypes.h>
#include "sys_define.h"
#include "mt_common.h"
#include "mpi_memdev.h"
#include "../driver/gpe.h"
#include <string.h>
#include "scale.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration" 
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
#pragma GCC diagnostic ignored "-Wpointer-sign" 
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wenum-compare" 
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable" 
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wunused-function" 
#pragma GCC diagnostic ignored "-Wshadow"

#define FAST_2D_SCALE
#define GPE_SPN_ASSERT   MT_ASSERT

typedef struct  
{
  u8 *p_src1_buf;
  u8 *p_src2_buf;
  u8 *p_src3_buf;
  u8 *p_src0_buf;
  u8 *p_dst_buf;
  u8 *p_src2_buf_bak;
  u8 *p_src1_ck_buf;
  u8 *p_src2_ck_buf;
  u8 *p_src3_ck_buf;
  u16 *p_gradt_buf;
  MT_BOOL src1_in;
  MT_BOOL src2_in;
  MT_BOOL src3_in;
  MT_BOOL src1_color_expan;
  MT_BOOL src2_color_expan;
  u32 src_w;
  u32 src_h;
  u32 src0_w;  //用于奇数起点或宽度的sp420、sp422
  u32 src1_w_uyvy; //用于奇数起点或宽度的uyvy 
  u32 src2_w_uyvy; //用于奇数起点或宽度的uyvy  
  u32 src3_w_uyvy; //用于奇数起点或宽度的uyvy  
  u32 dst_w;
  u32 dst_h;
  u8 *scale3d_mask;
#ifndef OLD_CLIP
  u8 *p_clip_buf;
#endif
//  u32 dst_w_t;
//  u32 dst_w_b;
//  MT_BOOL scaler_2d;
}gpe_hw_t;

typedef struct
{
    int cscp_00;
    int cscp_01;
    int cscp_02;
    int cscdc_0;
    int cscp_10;
    int cscp_11;
    int cscp_12;
    int cscdc_1;
    int cscp_20;
    int cscp_21;
    int cscp_22;
    int cscdc_2;    
}csc_group_t;

//used when sw compare
static u8 *p_dst_buf2 = NULL; 
static csc_group_t csc[2];
extern spn_debug_t g_debug;

#define CLIP(x) ((x < 0) ? (0) : ((x > 255) ? 255 : x))

static void gpe_spn_invalidate_buf(void *addr, u32 size)
{

//	hal_dcache_invalidate(addr, size);
			
}

static void *gpe_malloc(u32 size)
{
	return malloc(size + 32); // mtos_align_dma_malloc(size, 4);
}

static void gpe_free(void *ptr)
{
	//mtos_align_dma_free(ptr);
	free(ptr);
}

static u32 get_bpp(u32 bpp)
{
  switch(bpp)
  {
  case GPE_ARIA_BPP_1BIT:
    return 1;
    break;
  case GPE_ARIA_BPP_2BIT:
    return 2;
    break;
  case GPE_ARIA_BPP_4BIT:
    return 4;
    break;
  case GPE_ARIA_BPP_8BIT:
    return 8;
    break;
  case GPE_ARIA_BPP_16BIT:
    return 16;
    break;
  case GPE_ARIA_BPP_24BIT:
    return 24;
    break;
  case GPE_ARIA_BPP_32BIT:
    return 32;
    break;
  default:
    return 32;
    break;
  }
}

static u8 swap_inbyte(color_format_t color_fmt, u8 indata)
{
    u8 outdata = 0;
    if(color_fmt == CLUT_1)
    {
        outdata = ((indata & 0x1) << 7) | (((indata >> 1) & 0x1) << 6) |
            (((indata >> 2) & 0x1) << 5) |(((indata >> 3) & 0x1) << 4) |
            (((indata >> 4) & 0x1) << 3) |(((indata >> 5) & 0x1) << 2) |
            (((indata >> 6) & 0x1) << 1) | ((indata >> 7) & 0x1);
    }
    else if(color_fmt == CLUT_2)
    {
        outdata = ((indata & 0x3) << 6) | (((indata >> 2) & 0x3) << 4) |
            (((indata >> 4) & 0x3) << 2) |((indata >> 6) & 0x3);        
    }
    else if((color_fmt == CLUT_4) || (color_fmt == ALUT44))
    {
        outdata = ((indata & 0xf) << 4) | ((indata >> 4) & 0xff);     
    }
    return outdata;
}

static void load_img(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe, MT_BOOL sw_compare)
{
  mt_char *p_buf = NULL;
  int size = 0;
  int i = 0;
  mt_char *p_ptr = NULL;
  int bpp = 0;
  int w1 = 0, w2 = 0, h2 = 0;
  int x = 0, y = 0;
  mt_char *p_ptr1 = NULL, *p_ptr2 = NULL;
  mt_char *dst_buf = NULL;
  mt_char *bg_buf = NULL;
  printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);

  if(p_gpe->src1_in)
  {    
    bpp = get_bpp(p_ctx->src_img.color_info.bpp);
    if(p_ctx->src_img.color_info.color_fmt == TILE_Y || p_ctx->src_img.color_info.color_fmt == TILE_C)
    {
        p_buf = (u8 *)mt_mem_map(p_ctx->src_img.buf, ((((p_ctx->src_img.width + 31)/ 32 * 32) * bpp)/8) *p_ctx->src_img.height);
    }
    else
    {
        p_buf = (u8 *)mt_mmz_map(p_ctx->src_img.buf, 0);//(p_ctx->src_img.buf);
    }
    GPE_SPN_ASSERT(p_buf != NULL);
    w1 = p_ctx->src_img.pitch * 8 / bpp;
    if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
    {
        //read in the whole src piture when scale enable
        w2 = p_ctx->src_img.width;
        h2 = p_ctx->src_img.height;
        x = 0;
    }
    else
    {
        w2 = p_ctx->src_img.rect.w;
        h2 = p_ctx->src_img.rect.h;
        x = p_ctx->src_img.rect.x;
    }    
    if(p_ctx->src_img.color_info.color_fmt == Y1VY0U)
    {
        //针对奇数起点和宽度的情况，多读一部分
        if((x + w2) % 2 != 0)
            w2 += 1;
        if(x % 2 != 0)
        {
            x -= 1;
            w2 += 1;
        }
        MT_ASSERT(w2 % 2 == 0);            
        p_gpe->src1_w_uyvy = w2;
    }
    if((p_ctx->src_img.color_info.color_fmt == XY) ||
        (p_ctx->src_img.color_info.color_fmt == XYC) ||
        (p_ctx->src_img.color_info.color_fmt == XYL) ||
        (p_ctx->src_img.color_info.color_fmt == XYLC))
    {
      bpp = 32;
      w1 = (p_ctx->xylc_cfg.xylc_num) * 4;
      w2 = w1;
      h2 = 1;
    }
    size = w2 * h2 * bpp / 8;

    //pixel aligned
    p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));
    GPE_SPN_ASSERT(p_ptr != NULL);

    if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
    {
        y = 0;
    }
    else
    {
        if(p_ctx->src_img.negative_stride)
            y =  p_ctx->src_img.height -  p_ctx->src_img.rect.y - p_ctx->src_img.rect.h;
        else
            y = p_ctx->src_img.rect.y;
    }
    if((p_ctx->src_img.color_info.color_fmt == XY) ||
        (p_ctx->src_img.color_info.color_fmt == XYC) ||
        (p_ctx->src_img.color_info.color_fmt == XYL) ||
        (p_ctx->src_img.color_info.color_fmt == XYLC))
    {
      x = 0;
      y = 0;
    }
    for(i = 0; i < h2; i++)
    {
      p_ptr1 = &p_ptr[i * w2 * bpp / 8];
      p_ptr2 = &p_buf[(i + y) * p_ctx->src_img.pitch + x * bpp / 8];
      memcpy(p_ptr1, p_ptr2, w2 * bpp / 8);
    }

    //symphony的clut1/clut2/clut4默认是反着读的，和concerto以及网络图片相反；
    //这里反一下，让后面的pix expend操作和concerto保持一致
    if(((g_debug.src1_bitswap == 0) && ((p_ctx->src_img.color_info.color_fmt == CLUT_1) 
        || (p_ctx->src_img.color_info.color_fmt == CLUT_2) 
        ||(p_ctx->src_img.color_info.color_fmt == CLUT_4))) 
        || ((g_debug.src1_bitswap == 1) && (p_ctx->src_img.color_info.color_fmt == ALUT44)))        
    {
        u32 j;
        for(i = 0; i < h2; i++)
        {
            for(j = 0; j < w2 * bpp / 8; j++)
            {
                p_ptr[i * w2 * bpp / 8 + j] = swap_inbyte(p_ctx->src_img.color_info.color_fmt, p_ptr[i * w2 * bpp / 8 + j] );
            }
        }
    }
    
    p_gpe->p_src1_buf = p_ptr;
    p_gpe->src1_in = TRUE;
    if(p_ctx->src_img.color_info.color_fmt == TILE_Y || p_ctx->src_img.color_info.color_fmt == TILE_C)
    {
        mt_mem_unmap(p_buf);
    }
    else
    {
        mt_mmz_unmap(p_buf);
    }

    //src0 buf
    if((p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y) 
        || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y) 
        || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
        || (p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y))
    {
        w1 = p_ctx->src0_pitch;        
        if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
        {
            //read in the whole src piture when scale enable
            w2 = p_ctx->src_img.width;
            h2 = p_ctx->src_img.height;
        }
        else
        {
            w2 = p_ctx->src_img.rect.w;
            h2 = p_ctx->src_img.rect.h;    
        }
       // printf("<%s> : <%d> p_ctx->src0_buf : %x  : %x\n", __FUNCTION__, __LINE__, p_ctx->src0_buf, p_ctx->src0_buf_vir);
      //  p_buf  = (u8 *)mt_mmz_map(p_ctx->src0_buf, 0);//(p_ctx->src0_buf);
    //     printf("<%s> : <%d> p_buf : %x\n", __FUNCTION__, __LINE__, p_buf);
        p_buf = (mt_char *)p_ctx->src0_buf_vir; // (u8 *)mt_mmz_map(p_ctx->src0_buf, 0);//(p_ctx->src0_buf);
        GPE_SPN_ASSERT(p_buf != NULL);
        
   //     printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
        if(p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y)
        {
            //针对奇数起点和宽度的情况
            //chroma 横向左右测各多读一个字节
            //chroma纵向多读下方一行    
            
            printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
            if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
            {
                x = 0;
                y = 0;
            }
            else
            {
                x = p_ctx->src_img.rect.x;
                y = p_ctx->src_img.rect.y / 2;  
            }
            if((x + w2) % 2 != 0)
                w2 += 1;
            if(x % 2 != 0)
            {
                x -= 1;
                w2 += 1;
            }
            MT_ASSERT(w2 % 2 == 0);
            if((p_ctx->src_img.rect.y + h2) % 2 != 0)
                h2 += 1;
            if(p_ctx->src_img.rect.y % 2 != 0)
            {
                h2 += 1;
            }       
            size = w2 * h2 / 2;        
            p_gpe->src0_w = w2;
            p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));
            GPE_SPN_ASSERT(p_ptr != NULL); 
      
            for(i = 0; i < h2 / 2; i++)
            {
              p_ptr1 = &p_ptr[i * w2];
              if(p_ctx->src_img.negative_stride)
                p_ptr2 = &p_buf[(h2 / 2 - 1 - y - i) * w1 + x];
              else
                p_ptr2 = &p_buf[(i + y) * w1 + x];
              memcpy(p_ptr1, p_ptr2, w2);
            }            
        }
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y)
        {
        
        printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
            if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
            {
                x = 0;
                y = 0;
            }
            else
            {
                x = p_ctx->src_img.rect.x;
                y = p_ctx->src_img.rect.y;     
            }
            if((x + w2) % 2 != 0)
                w2 += 1;
            if(x % 2 != 0)
            {
                x -= 1;
                w2 += 1;
            }       
            MT_ASSERT(w2 % 2 == 0);
            size = w2 * h2;
            p_gpe->src0_w = w2;                 
            p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));
            GPE_SPN_ASSERT(p_ptr != NULL); 

            for(i = 0; i < h2; i++)
            {
              p_ptr1 = &p_ptr[i * w2];
              if(p_ctx->src_img.negative_stride)
                p_ptr2 = &p_buf[(h2 - 1 - y - i) * w1 + x];
              else
                p_ptr2 = &p_buf[(i + y) * w1 + x];
              memcpy(p_ptr1, p_ptr2, w2);
            }
        }
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
        {            
        
        printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
            p_gpe->src0_w = w2;    
            if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
            {
                x = 0;
                y = 0;
            }
            else
            {
                x = p_ctx->src_img.rect.x * 2;
                y = p_ctx->src_img.rect.y / 2;     
            }
            if((p_ctx->src_img.rect.y + h2) % 2 != 0)
                h2 += 1;
            if(p_ctx->src_img.rect.y % 2 != 0)
            {
                h2 += 1;
            }      
            size = w2 * 2 * h2 / 2;
            p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));
            GPE_SPN_ASSERT(p_ptr != NULL); 

            for(i = 0; i < h2 / 2; i++)
            {
              p_ptr1 = &p_ptr[i * w2 * 2];
              if(p_ctx->src_img.negative_stride)
                p_ptr2 = &p_buf[(h2 / 2 - 1 - y - i) * w1 + x];
              else
                p_ptr2 = &p_buf[(i + y) * w1 + x];
              memcpy(p_ptr1, p_ptr2, w2 * 2);
            }
        }        
        else
        {
        
 //       printf("<%s> : <%d>p_ctx->scale_en < %d %d %d>\n", __FUNCTION__, __LINE__, p_ctx->gaussian_blur_en, p_ctx->scale_en, p_ctx->src_img.negative_stride);
            size = w2 * 2 * h2;
            p_gpe->src0_w = w2;    
            if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
            {
                x = 0;
                y = 0;
            }
            else
            {
                x = p_ctx->src_img.rect.x * 2;
                y = p_ctx->src_img.rect.y;     
            }
            p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));
            GPE_SPN_ASSERT(p_ptr != NULL); 
        //    printf("<%s> : <%d><%lx %lx %d <%d %d>\n", __FUNCTION__, __LINE__, p_ptr, p_buf, h2, x, y);
        //    printf("<%s> : <%d> : w1 : %d %d %d>\n", __FUNCTION__, __LINE__, w1, w2 ,h2);
            for(i = 0; i < h2; i++)
            {
              p_ptr1 = &p_ptr[i * w2 * 2];
              if(p_ctx->src_img.negative_stride)
                p_ptr2 = &p_buf[(h2 - 1 - y - i) * w1 + x];
              else
                p_ptr2 = &p_buf[(i + y) * w1 + x];
              memcpy(p_ptr1, p_ptr2, w2 * 2);
            }
        }
        p_gpe->p_src0_buf = p_ptr; 
    //    printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
        
       // mt_mmz_unmap(p_ctx->src0_buf);
    }
    else if(p_ctx->src_with_mask)
    {
    
//    printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
        w1 = p_ctx->src0_pitch;
        if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
        {
            w2 = p_ctx->src_img.width;
            h2 = p_ctx->src_img.height;    
            x = 0;
            y = 0;          
        }
        else
        {
            w2 = p_ctx->src_img.rect.w;
            h2 = p_ctx->src_img.rect.h;    
            x = p_ctx->src_img.rect.x;
            y = p_ctx->src_img.rect.y;     
        }
    //     printf("<%s> : <%d> p_ctx->src0_buf : %x\n", __FUNCTION__, __LINE__, p_ctx->src0_buf);
        p_buf = (u8 *)mt_mmz_map(p_ctx->src0_buf, 0);//(p_ctx->src0_buf);
        GPE_SPN_ASSERT(p_buf != NULL);
        size = w2 * h2;
        p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));
        GPE_SPN_ASSERT(p_ptr != NULL); 

        for(i = 0; i < h2; i++)
        {
          p_ptr1 = &p_ptr[i * w2];
          if(p_ctx->src_img.negative_stride)
            p_ptr2 = &p_buf[(h2 - 1 - y - i) * w1 + x];
          else
            p_ptr2 = &p_buf[(i + y) * w1 + x];
          memcpy(p_ptr1, p_ptr2, w2);
        }      
        p_gpe->p_src0_buf = p_ptr; 
        mt_mmz_unmap((void *)p_buf);
    }
    else if(p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
    {
    
    printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
         w1 = p_ctx->src0_pitch;
        if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
        {
            w2 = p_ctx->src_img.width;
            h2 = p_ctx->src_img.height;    
            x = 0;
            y = 0;          
        }
        else
        {
            w2 = p_ctx->src_img.rect.w;
            h2 = p_ctx->src_img.rect.h;    
            x = p_ctx->src_img.rect.x;
            y = p_ctx->src_img.rect.y;     
        }
        // printf("<%s> : <%d> p_ctx->src0_buf : %x\n", __FUNCTION__, __LINE__, p_ctx->src0_buf);
        p_buf = (u8 *)mt_mmz_map(p_ctx->src0_buf, 0);//(p_ctx->src0_buf);
        GPE_SPN_ASSERT(p_buf != NULL);
        size = w2 * h2 * 2;
        p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));
        GPE_SPN_ASSERT(p_ptr != NULL); 

        for(i = 0; i < h2; i++)
        {
          p_ptr1 = &p_ptr[i * w2 * 2];
          if(p_ctx->src_img.negative_stride)
            p_ptr2 = &p_buf[(h2 - 1 - y - i) * w1 + x];
          else
            p_ptr2 = &p_buf[(i + y) * w1 + x];
          memcpy(p_ptr1, p_ptr2, w2 * 2);
        }      
        p_gpe->p_src0_buf = p_ptr; 
        mt_mmz_unmap((void *)p_buf);
    }
  }
//  printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);

  if(p_gpe->src3_in)
  {
    bpp = get_bpp(p_ctx->ex_img.color_info.bpp);
    p_buf = (u8 *)mt_mmz_map(p_ctx->ex_img.buf, 0); //(p_ctx->ex_img.buf);
    GPE_SPN_ASSERT(p_buf != NULL);
    w1 = p_ctx->ex_img.pitch * 8 / bpp;
    w2 = p_ctx->ex_img.rect.w;
    h2 = p_ctx->ex_img.rect.h;    
    x = p_ctx->ex_img.rect.x;    
    if(p_ctx->ex_img.color_info.color_fmt == Y1VY0U)
    {
        //针对奇数起点和宽度的情况，多读一部分
        if((x + w2) % 2 != 0)
            w2 += 1;
        if(x % 2 != 0)
        {
            x -= 1;
            w2 += 1;
        }
        MT_ASSERT(w2 % 2 == 0);        
        p_gpe->src3_w_uyvy = w2;
    }    
    size = w2 * h2 * bpp / 8;
    //pixel aligned
    p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));

    GPE_SPN_ASSERT(p_ptr != NULL);

    if(p_ctx->ex_img.negative_stride)
        y =  p_ctx->ex_img.height -  p_ctx->ex_img.rect.y - p_ctx->ex_img.rect.h;
    else
        y = p_ctx->ex_img.rect.y;
    for(i = 0; i < h2; i++)
    {
      memcpy(&p_ptr[i * w2 * bpp / 8], 
          &p_buf[(i + y) * w1 * bpp / 8 + x * bpp / 8], w2 * bpp / 8);
    }
    //symphony的clut1/clut2/clut4默认是反着读的，和concerto以及网络图片相反；
    //这里反一下，让后面的pix expend操作和concerto保持一致    
    if(((g_debug.src1_bitswap == 0) && ((p_ctx->src_img.color_info.color_fmt == CLUT_1) 
        || (p_ctx->src_img.color_info.color_fmt == CLUT_2) 
        ||(p_ctx->src_img.color_info.color_fmt == CLUT_4))) 
        || ((g_debug.src1_bitswap == 1) && (p_ctx->src_img.color_info.color_fmt == ALUT44)))      
    {
        u32 j;
        for(i = 0; i < h2; i++)
        {
            for(j = 0; j < w2 * bpp / 8; j++)
            {
                p_ptr[i * w2 * bpp / 8 + j] = swap_inbyte(p_ctx->ex_img.color_info.color_fmt, p_ptr[i * w2 * bpp / 8 + j] );
            }
        }
    }    
    p_gpe->p_src3_buf = p_ptr;
    p_gpe->src3_in = TRUE;
     mt_mmz_unmap((void *)p_buf);
  }
 // printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);

  dst_buf = (u8 *)mt_mmz_map(p_ctx->dst_img.buf, 0);
  if(sw_compare)
  {
    u32 size = p_ctx->dst_img.pitch * p_ctx->dst_img.height;
    p_dst_buf2 = (u8 *)gpe_malloc(size);
    GPE_SPN_ASSERT(p_dst_buf2 != NULL);
    memcpy(p_dst_buf2, dst_buf, size);  // memcpy(p_dst_buf2,  (u8 *)(p_ctx->dst_img.buf), size);
    p_gpe->p_dst_buf = (u8 *)(p_dst_buf2);
  }
  else
  {
    p_gpe->p_dst_buf =dst_buf; // (u8 *)(p_ctx->dst_img.buf);
  }
  GPE_SPN_ASSERT(p_gpe->p_dst_buf != NULL);

  if(1) //src2 always read in
  {
    w2 = p_ctx->dst_img.rect.w;
    h2 = p_ctx->dst_img.rect.h;   
    if(p_ctx->bg_img_en)
    {
        bpp = get_bpp(p_ctx->bg_img.color_info.bpp);
        w1 = p_ctx->bg_img.pitch * 8 / bpp; 
        x = p_ctx->bg_img.rect.x;
        if(p_ctx->bg_img.color_info.color_fmt == Y1VY0U)
        {
            //针对奇数起点和宽度的情况，多读一部分
            if((x + w2) % 2 != 0)
                w2 += 1;
            if(x % 2 != 0)
            {
                x -= 1;
                w2 += 1;
            }
            MT_ASSERT(w2 % 2 == 0);           
        }          
        if(p_ctx->bg_img.negative_stride)
            y =  p_ctx->bg_img.height -  p_ctx->bg_img.rect.y - p_ctx->bg_img.rect.h;
        else
            y = p_ctx->bg_img.rect.y;        
        bg_buf = (u8 *)mt_mmz_map(p_ctx->bg_img.buf, 0);
        p_buf = bg_buf;  //(u8 *)(p_ctx->bg_img.buf);
    }
    else
    {
        bpp = get_bpp(p_ctx->dst_img.color_info.bpp);
        w1 = p_ctx->dst_img.pitch * 8 / bpp;          
        x = p_ctx->dst_img.rect.x;
        if(p_ctx->dst_img.negative_stride)
            y =  p_ctx->dst_img.height -  p_ctx->dst_img.rect.y - p_ctx->dst_img.rect.h;
        else
            y = p_ctx->dst_img.rect.y;          
        p_buf = dst_buf;//(u8 *)(p_ctx->dst_img.buf);
    }
    p_gpe->src2_w_uyvy = w2;
    size = w2 * h2 * bpp / 8;     
    p_ptr = (u8 *)gpe_malloc(size * sizeof(u8));

    GPE_SPN_ASSERT(p_ptr != NULL);
    for(i = 0; i < h2; i++)
    {
      memcpy(&p_ptr[i * w2 * bpp / 8], 
           &p_buf[(i + y) * w1 * bpp / 8 + x * bpp / 8], w2 * bpp / 8);
    }
    p_gpe->p_src2_buf = p_ptr;
    p_gpe->src2_in = TRUE;
    if(p_ctx->bg_img_en)
        mt_mmz_unmap((void *)bg_buf);
  }
   mt_mmz_unmap((void *)dst_buf);
}

static void free_gpe(gpe_hw_t *p_gpe)
{
  if(p_gpe->p_src0_buf != NULL)
    gpe_free(p_gpe->p_src0_buf);
  if(p_gpe->p_src1_buf != NULL)
    gpe_free(p_gpe->p_src1_buf);
  if(p_gpe->p_src2_buf != NULL)
    gpe_free(p_gpe->p_src2_buf);
  if(p_gpe->p_src3_buf != NULL)
    gpe_free(p_gpe->p_src3_buf);
  if(p_gpe->p_src1_ck_buf != NULL)
    gpe_free(p_gpe->p_src1_ck_buf);
  if(p_gpe->p_src2_ck_buf != NULL)
    gpe_free(p_gpe->p_src2_ck_buf);
  if(p_gpe->p_src3_ck_buf != NULL)
    gpe_free(p_gpe->p_src3_ck_buf);
  if(p_gpe->p_gradt_buf != NULL)
    gpe_free(p_gpe->p_gradt_buf);
  if(p_gpe->scale3d_mask != NULL)
    gpe_free(p_gpe->scale3d_mask);
  if(p_gpe->p_src2_buf_bak != NULL)
    gpe_free(p_gpe->p_src2_buf_bak);
}

static void load_param(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  csc[0].cscp_00 = 47;
  csc[0].cscp_01 = 157;
  csc[0].cscp_02 = 16;
  csc[0].cscdc_0 = 4096;
  csc[0].cscp_10 = -26;
  csc[0].cscp_11 = -87;
  csc[0].cscp_12 = 112;
  csc[0].cscdc_1 = 32768;
  csc[0].cscp_20 = 112;
  csc[0].cscp_21 = -102;
  csc[0].cscp_22 = -10;
  csc[0].cscdc_2 = 32768;  

  csc[1].cscp_00 = 298;
  csc[1].cscp_01 = 0;
  csc[1].cscp_02 = 459;
  csc[1].cscdc_0 = -63522;
  csc[1].cscp_10 = 298;
  csc[1].cscp_11 = -55;
  csc[1].cscp_12 = -136;
  csc[1].cscdc_1 = 19659;
  csc[1].cscp_20 = 298;
  csc[1].cscp_21 = 541;
  csc[1].cscp_22 = 0;
  csc[1].cscdc_2 = -74002;    
  
  if(p_ctx->src1_sel && p_ctx->src_img_en)
    p_gpe->src1_in = TRUE;
  if(p_ctx->src_img.color_info.color_fmt == GRAY_16)
    p_gpe->src1_in = FALSE;
  
//  if(p_ctx->src2_sel)
  p_gpe->src2_in = TRUE;
  if(p_ctx->ex_img_en)
    p_gpe->src3_in = TRUE;

  p_gpe->src_w = p_ctx->src_img.rect.w;
  p_gpe->src_h = p_ctx->src_img.rect.h;
  p_gpe->dst_w = p_ctx->dst_img.rect.w;
  p_gpe->dst_h = p_ctx->dst_img.rect.h;
//  p_gpe->dst_w_t = p_ctx->dst_trape.top_len;
//  p_gpe->dst_w_b = p_ctx->dst_trape.bottom_len;
//  p_gpe->scaler_2d = p_ctx->scale_2d_flag;
  if(p_gpe->src1_in)
  {
    p_gpe->src1_color_expan = FALSE;
    
    if(!p_ctx->dst_img.with_palette) //dst is not lut
    {
      p_gpe->src1_color_expan = TRUE;
    }
  }
  if(p_gpe->src2_in)
  {
    p_gpe->src2_color_expan = FALSE;
    if(!p_ctx->dst_img.with_palette) //dst is not lut
    {
      p_gpe->src2_color_expan = TRUE;
    }
  }
}
#define round(x) (((x) > 0) ? (int)((x)+0.5) : (int)((x)-0.5))               
#define AMP 256

static void ayuv2argb(u32 *p_src, u32 width, u32 height)
{
  int a, r,g,b;
  int y,u,v;
  u32 i = 0, j = 0;
  u32 tmp = 0;
  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width; j++)
    {
      tmp = p_src[i * width + j];
      a = (tmp >> 24) & 0xff;
      y = (tmp >> 16) & 0xff;
      u = (tmp >> 8) & 0xff;
      v = tmp & 0xff;

      r = ((((csc[1].cscp_00) * y >> 4) + ((csc[1].cscp_01) * u >> 4) + ((csc[1].cscp_02) * v >> 4) +  ((csc[1].cscdc_0) >> 4)))>> 4;
      g = ((((csc[1].cscp_10) * y >> 4) + ((csc[1].cscp_11) * u >> 4) + ((csc[1].cscp_12) * v >> 4) +  ((csc[1].cscdc_1) >> 4)))>> 4;
      b = ((((csc[1].cscp_20) * y >> 4) + ((csc[1].cscp_21) * u >> 4) + ((csc[1].cscp_22) * v >> 4) +  ((csc[1].cscdc_2) >> 4)))>> 4; 
      r = CLIP(r);
      g = CLIP(g);
      b = CLIP(b);

      p_src[i * width + j] = (a << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    }
  }
}

static void argb2ayuv(u32 *p_src, u32 width, u32 height)
{
  int a, r,g,b;
  int y,u,v;
  u32 i = 0, j = 0;
  u32 tmp = 0;
  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width; j++)
    {
      tmp = p_src[i * width + j];
      a = (tmp >> 24) & 0xff;
      r = (tmp >> 16) & 0xff;
      g = (tmp >> 8) & 0xff;
      b = tmp & 0xff;

      y = ((((csc[0].cscp_00) * r >> 4) + ((csc[0].cscp_01) * g >> 4) + ((csc[0].cscp_02) * b >> 4) +  ((csc[0].cscdc_0) >> 4)))>> 4;
      u = ((((csc[0].cscp_10) * r >> 4) + ((csc[0].cscp_11) * g >> 4) + ((csc[0].cscp_12) * b >> 4) +  ((csc[0].cscdc_1) >> 4)))>> 4;
      v = ((((csc[0].cscp_20) * r >> 4) + ((csc[0].cscp_21) * g >> 4) + ((csc[0].cscp_22) * b >> 4) +  ((csc[0].cscdc_2) >> 4)))>> 4;  
      y = CLIP(y);
      u = CLIP(u);
      v = CLIP(v);

      p_src[i * width + j] = (a << 24) | ((y & 0xff) << 16) | ((u & 0xff) << 8) | (v & 0xff);
    }
  }
}

static void ayuv2uyvy(u32 *p_src, u32 *p_dst, u32 width, u32 height)
{
  u32 i = 0, j = 0;
  u32 y0 = 0, y1 = 0, u0 = 0, v0 = 0;
  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width / 2; j++)
    {
      y0 = (p_src[i * width + 2 * j] >> 16) & 0xff;
      u0 = (p_src[i * width + 2 * j] >> 8) & 0xff;
      v0 = (p_src[i * width + 2 * j]) & 0xff;
      y1 = (p_src[i * width + 2 * j + 1] >> 16) & 0xff;

      p_dst[i * width / 2 + j] = (u0 << 24) | (y0 << 16) | (v0 << 8) | y1;
    }
  }
}

static void uyvy2ayuv(u32 *p_src, u32 *p_dst, u32 width, u32 height, u32 uyvy_width, u32 odd_start_x)
{
  u32 i = 0, j = 0;
  u32 y0 = 0, y1 = 0, u0 = 0, v0 = 0;
  if(odd_start_x == 0)
  {
      for(i = 0; i < height; i++)
      {
        for(j = 0; j < (width + 1) / 2; j++)
        {
          u0 = (p_src[i * uyvy_width / 2 + j] >> 24) & 0xff;
          y0 = (p_src[i * uyvy_width / 2 + j] >> 16) & 0xff;
          v0 = (p_src[i * uyvy_width / 2 + j] >> 8) & 0xff;
          y1 = (p_src[i * uyvy_width / 2 + j]) & 0xff;
          p_dst[i * width + 2 * j] = (0xff << 24) | (y0 << 16) | (u0 << 8) | v0;
          if(2 * j + 1 < width)
              p_dst[i * width + 2 * j + 1] = (0xff << 24) | (y1 << 16) | (u0 << 8) | v0;
        }
      }
  }
  else
  {
      for(i = 0; i < height; i++)
      {
        u0 = (p_src[i * uyvy_width / 2] >> 24) & 0xff;
        y0 = (p_src[i * uyvy_width / 2] >> 16) & 0xff;
        v0 = (p_src[i * uyvy_width / 2] >> 8) & 0xff;
        y1 = (p_src[i * uyvy_width / 2]) & 0xff;
        p_dst[i * width] =  (0xff << 24) | (y1 << 16) | (u0 << 8) | v0;       
        for(j = 1; j < (width + 1 + 1) / 2; j++)
        {
          u0 = (p_src[i * uyvy_width / 2 + j] >> 24) & 0xff;
          y0 = (p_src[i * uyvy_width / 2 + j] >> 16) & 0xff;
          v0 = (p_src[i * uyvy_width / 2 + j] >> 8) & 0xff;
          y1 = (p_src[i * uyvy_width / 2 + j]) & 0xff;

          p_dst[i * width + 2 * j - 1] = (0xff << 24) | (y0 << 16) | (u0 << 8) | v0;
          if(2 * j < width)
              p_dst[i * width + 2 * j] = (0xff << 24) | (y1 << 16) | (u0 << 8) | v0;
        }      
      }    
  }
}
//convert src to 32bpp and big endian
static void pixel_expand(u8 *p_src,
          u32 *p_dst,
          u32 width,
          u32 height,
          u32 bpp,
          MT_BOOL little_endian,
          u32 yuv422_flag,
          MT_BOOL negative_stride)
{
  u32 i = 0, j = 0;
  u8 *p_ptr8 = NULL;
  u16 *p_ptr16 = NULL;
  u32 *p_ptr32 = NULL;
  u32 tmp32 = 0;
  u16 tmp16 = 0;

  if(yuv422_flag)
    bpp = GPE_ARIA_BPP_32BIT;
  switch(bpp)
  {
  case GPE_ARIA_BPP_32BIT:
    p_ptr32 = (u32 *)p_src;
    if(little_endian == TRUE)
    {
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          if(negative_stride)
              tmp32 = p_ptr32[(height - 1 - i) * width + j];
          else
              tmp32 = p_ptr32[i * width + j];
          p_dst[i * width + j] = ((tmp32 & 0xff) << 24) |
                (((tmp32 >> 8) & 0xff) << 16) |
                (((tmp32 >> 16) & 0xff) << 8) |
                ((tmp32 >> 24) & 0xff);
        }
    }
    else
    {
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          if(negative_stride)
              tmp32 = p_ptr32[(height - 1 - i) * width + j];
          else        
              tmp32 = p_ptr32[i * width + j];
          p_dst[i * width + j] = tmp32;
        }
    }
    break;
  case GPE_ARIA_BPP_24BIT:
    p_ptr8 = (u8 *)p_src;
    {
      u8 byte0 = 0, byte1 = 0, byte2 = 0;
      for(i = 0; i < height; i++)
        for(j = 0; j < width * 3; j+=3)
        {
          if(negative_stride)
          {              
              byte0 = p_ptr8[(height - 1 - i) * width * 3 + j];
              byte1 = p_ptr8[(height - 1 - i) * width * 3 + j + 1];
              byte2 = p_ptr8[(height - 1 - i) * width * 3 + j + 2];
          }
          else       
          {
              byte0 = p_ptr8[i * width * 3 + j];
              byte1 = p_ptr8[i * width * 3 + j + 1];
              byte2 = p_ptr8[i * width * 3 + j + 2];
          }
          p_dst[i * width + j / 3] = (byte2 << 16) | (byte1 << 8) | byte0;
        }
    }      
    break;
  case GPE_ARIA_BPP_16BIT:
    p_ptr16 = (u16 *)p_src;
    if(little_endian == TRUE)
    {
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          if(negative_stride)
              tmp16 = p_ptr16[(height - 1 - i) * width + j];
          else         
              tmp16 = p_ptr16[i * width + j];
          p_dst[i * width + j] = ((tmp16 & 0xff) << 8) | ((tmp16 >> 8) & 0xff);
        }
    }
    else
    {
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          if(negative_stride)
              tmp16 = p_ptr16[(height - 1 - i) * width + j];
          else         
              tmp16 = p_ptr16[i * width + j];
          p_dst[i * width + j] = tmp16;
        }
    }
    break;
  case GPE_ARIA_BPP_8BIT:
    p_ptr8 = p_src;
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        if(negative_stride)
            p_dst[i * width + j] = p_ptr8[(height - 1 - i) * width + j];
        else
            p_dst[i * width + j] = p_ptr8[i * width + j];
      }
    break;
  case GPE_ARIA_BPP_4BIT:
    p_ptr8 = p_src;
    for(i = 0; i < height; i++)
    {
      for(j = 0; j < width; j += 2)
      {
        if(negative_stride)
        {
#if 1        //keep consistent with concerto
            if(width > 1)
            {        
                p_dst[i * width + j + 1] = p_ptr8[(height - 1 - i) * width / 2 + j /2] & 0xf;
                p_dst[i * width + j] = (p_ptr8[(height - 1 - i) * width / 2 + j /2] >> 4) & 0xf;    
            }
            else
            {        
                p_dst[i * width + j] = (p_ptr8[(height - 1 - i) * width / 2 + j /2] >> 4) & 0xf; 
            }
#else
            if(width > 1)
            {
                p_dst[i * width + j + 0] = p_ptr8[(height - 1 - i) * width / 2 + j /2] & 0xf;
                p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 2 + j /2] >> 4) & 0xf;  
            }
            else
            {
                p_dst[i * width + j] = p_ptr8[(height - 1 - i) * width / 2 + j /2] & 0xf;
            }
#endif
        }
        else
        {
#if 1        
            if(width > 1)          
            {           
                p_dst[i * width + j + 1] = p_ptr8[i * width / 2 + j /2] & 0xf;
                p_dst[i * width + j] = (p_ptr8[i * width / 2 + j /2] >> 4) & 0xf;
            }
            else
            {       
                p_dst[i * width + j] = (p_ptr8[i * width / 2 + j /2] >> 4) & 0xf;
            }
#else
            if(width > 1)          
            {
                p_dst[i * width + j + 0] = p_ptr8[i * width / 2 + j /2] & 0xf;
                p_dst[i * width + j + 1] = (p_ptr8[i * width / 2 + j /2] >> 4) & 0xf;                
            }
            else
            { 
                p_dst[i * width + j] = (p_ptr8[i * width / 2 + j /2]) & 0xf;
            }
#endif
        }
      }
    }
    break;
  case GPE_ARIA_BPP_2BIT:
    p_ptr8 = p_src;
    for(i = 0; i < height; i++)
    {
      for(j = 0; j < width; j += 4)
      {
        if(negative_stride)
        {
#if 1        
            if(width > 3)        
            {      
                p_dst[i * width + j + 3] = p_ptr8[(height - 1 - i) * width / 4 + j / 4] & 0x3;
                p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 2) & 0x3;
                p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 4) & 0x3;
                p_dst[i * width + j + 0] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 6) & 0x3;  
            }
            else
            {
                if(width > 2)                  
                    p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 2) & 0x3;

                if(width > 1)                 
                    p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 4) & 0x3;
                p_dst[i * width + j + 0] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 6) & 0x3;  
            }         
#else
            if(width > 3)        
            {
                p_dst[i * width + j + 0] = p_ptr8[(height - 1 - i) * width / 4 + j / 4] & 0x3;
                p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 2) & 0x3;
                p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 4) & 0x3;
                p_dst[i * width + j + 3] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 6) & 0x3; 
            }
            else
            {
                if(width > 2)
                    p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 4) & 0x3;

                if(width > 1)
                    p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 4 + j / 4] >> 2) & 0x3;
                p_dst[i * width + j + 0] = p_ptr8[(height - 1 - i) * width / 4 + j / 4] & 0x3;
            }  
#endif
        }
        else
        {
#if 1        
            if(width > 3)
            {           
                p_dst[i * width + j + 3] = p_ptr8[i * width / 4 + j / 4] & 0x3;
                p_dst[i * width + j + 2] = (p_ptr8[i * width / 4 + j / 4] >> 2) & 0x3;
                p_dst[i * width + j + 1] = (p_ptr8[i * width / 4 + j / 4] >> 4) & 0x3;
                p_dst[i * width + j + 0] = (p_ptr8[i * width / 4 + j / 4] >> 6) & 0x3;
            }
            else
            {
                if(width > 2)
                    p_dst[i * width + j + 2] = (p_ptr8[i * width / 4 + j / 4] >> 2) & 0x3;
                if(width > 1)
                    p_dst[i * width + j + 1] = (p_ptr8[i * width / 4 + j / 4] >> 4) & 0x3;
                p_dst[i * width + j + 0] = (p_ptr8[i * width / 4 + j / 4] >> 6) & 0x3;
            }        
#else
            if(width > 3)
            {
                p_dst[i * width + j + 0] = p_ptr8[i * width / 4 + j / 4] & 0x3;
                p_dst[i * width + j + 1] = (p_ptr8[i * width / 4 + j / 4] >> 2) & 0x3;
                p_dst[i * width + j + 2] = (p_ptr8[i * width / 4 + j / 4] >> 4) & 0x3;
                p_dst[i * width + j + 3] = (p_ptr8[i * width / 4 + j / 4] >> 6) & 0x3;
            }
            else
            {
                if(width > 2)
                    p_dst[i * width + j + 2] = (p_ptr8[i * width / 4 + j / 4] >> 4) & 0x3;
                if(width > 1)
                    p_dst[i * width + j + 1] =  (p_ptr8[i * width / 4 + j / 4] >> 2) & 0x3;
                p_dst[i * width + j + 0] = p_ptr8[i * width / 4 + j / 4] & 0x3;
            }
#endif
        }
      }
    }
    break;
  case GPE_ARIA_BPP_1BIT:
    p_ptr8 = p_src;
    for(i = 0; i < height; i++)
    {
      for(j = 0; j < width; j += 8)
      {
        if(negative_stride)
        {
#if 1        
            if(width > 7)
            {                  
                p_dst[i * width + j + 7] = p_ptr8[(height - 1 - i) * width / 8 + j / 8] & 0x1;
                p_dst[i * width + j + 6] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 1) & 0x1;
                p_dst[i * width + j + 5] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 2) & 0x1;
                p_dst[i * width + j + 4] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 3) & 0x1;
                p_dst[i * width + j + 3] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 4) & 0x1;
                p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 5) & 0x1;
                p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 6) & 0x1;
                p_dst[i * width + j] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 7) & 0x1;       
            }
            else
            {        
                if(width > 6)                
                    p_dst[i * width + j + 6] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 1) & 0x1;
                if(width > 5)                
                    p_dst[i * width + j + 5] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 2) & 0x1;
                if(width > 4)                
                    p_dst[i * width + j + 4] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 3) & 0x1;
                if(width > 3)                
                    p_dst[i * width + j + 3] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 4) & 0x1;
                if(width > 2)                
                    p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 5) & 0x1;
                if(width > 1)                
                    p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 6) & 0x1;
                p_dst[i * width + j] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 7) & 0x1;       
            }   
#else
            if(width > 7)
            {        
                p_dst[i * width + j + 0] = p_ptr8[(height - 1 - i) * width / 8 + j / 8] & 0x1;
                p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 1) & 0x1;
                p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 2) & 0x1;
                p_dst[i * width + j + 3] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 3) & 0x1;
                p_dst[i * width + j + 4] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 4) & 0x1;
                p_dst[i * width + j + 5] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 5) & 0x1;
                p_dst[i * width + j + 6] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 6) & 0x1;
                p_dst[i * width + j + 7] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 7) & 0x1;       
            }
            else
            {        
                if(width > 6)                
                    p_dst[i * width + j + 6] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 6) & 0x1;
                if(width > 5)                
                    p_dst[i * width + j + 5] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 5) & 0x1;
                if(width > 4)                
                    p_dst[i * width + j + 4] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 4) & 0x1;
                if(width > 3)                
                    p_dst[i * width + j + 3] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 3) & 0x1;
                if(width > 2)                
                    p_dst[i * width + j + 2] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 2) & 0x1;
                if(width > 1)                
                    p_dst[i * width + j + 1] = (p_ptr8[(height - 1 - i) * width / 8 + j / 8] >> 1) & 0x1;
                p_dst[i * width + j] = p_ptr8[(height - 1 - i) * width / 8 + j / 8] & 0x1;      
            }  
#endif
        }
        else
        {
#if 1        
            if(width > 7)
            {         
                p_dst[i * width + j + 7] = p_ptr8[i * width / 8 + j / 8] & 0x1;
                p_dst[i * width + j + 6] = (p_ptr8[i * width / 8 + j / 8] >> 1) & 0x1;
                p_dst[i * width + j + 5] = (p_ptr8[i * width / 8 + j / 8] >> 2) & 0x1;
                p_dst[i * width + j + 4] = (p_ptr8[i * width / 8 + j / 8] >> 3) & 0x1;
                p_dst[i * width + j + 3] = (p_ptr8[i * width / 8 + j / 8] >> 4) & 0x1;
                p_dst[i * width + j + 2] = (p_ptr8[i * width / 8 + j / 8] >> 5) & 0x1;
                p_dst[i * width + j + 1] = (p_ptr8[i * width / 8 + j / 8] >> 6) & 0x1;
                p_dst[i * width + j] = (p_ptr8[i * width / 8 + j / 8] >> 7) & 0x1;   
            }
            else
            {
                if(width > 6)
                   p_dst[i * width + j + 6] = (p_ptr8[i * width / 8 + j / 8] >> 1) & 0x1;
                if(width > 5)
                    p_dst[i * width + j + 5] = (p_ptr8[i * width / 8 + j / 8] >> 2) & 0x1;
                if(width > 4)
                    p_dst[i * width + j + 4] = (p_ptr8[i * width / 8 + j / 8] >> 3) & 0x1;
                if(width > 3)
                    p_dst[i * width + j + 3] = (p_ptr8[i * width / 8 + j / 8] >> 4) & 0x1;
                if(width > 2)
                    p_dst[i * width + j + 2] = (p_ptr8[i * width / 8 + j / 8] >> 5) & 0x1;
                if(width > 1)
                    p_dst[i * width + j + 1] = (p_ptr8[i * width / 8 + j / 8] >> 6) & 0x1;
                p_dst[i * width + j] = (p_ptr8[i * width / 8 + j / 8] >> 7) & 0x1;                  
            }
#else
            if(width > 7)
            {
                p_dst[i * width + j + 0] = p_ptr8[i * width / 8 + j / 8] & 0x1;
                p_dst[i * width + j + 1] = (p_ptr8[i * width / 8 + j / 8] >> 1) & 0x1;
                p_dst[i * width + j + 2] = (p_ptr8[i * width / 8 + j / 8] >> 2) & 0x1;
                p_dst[i * width + j + 3] = (p_ptr8[i * width / 8 + j / 8] >> 3) & 0x1;
                p_dst[i * width + j + 4] = (p_ptr8[i * width / 8 + j / 8] >> 4) & 0x1;
                p_dst[i * width + j + 5] = (p_ptr8[i * width / 8 + j / 8] >> 5) & 0x1;
                p_dst[i * width + j + 6] = (p_ptr8[i * width / 8 + j / 8] >> 6) & 0x1;
                p_dst[i * width + j + 7] = (p_ptr8[i * width / 8 + j / 8] >> 7) & 0x1;   
            }
            else
            {
                if(width > 6)
                   p_dst[i * width + j + 6] = (p_ptr8[i * width / 8 + j / 8] >> 6) & 0x1;
                if(width > 5)
                    p_dst[i * width + j + 5] = (p_ptr8[i * width / 8 + j / 8] >> 5) & 0x1;
                if(width > 4)
                    p_dst[i * width + j + 4] = (p_ptr8[i * width / 8 + j / 8] >> 4) & 0x1;
                if(width > 3)
                    p_dst[i * width + j + 3] = (p_ptr8[i * width / 8 + j / 8] >> 3) & 0x1;
                if(width > 2)
                    p_dst[i * width + j + 2] = (p_ptr8[i * width / 8 + j / 8] >> 2) & 0x1;
                if(width > 1)
                    p_dst[i * width + j + 1] = (p_ptr8[i * width / 8 + j / 8] >> 1) & 0x1;
                p_dst[i * width + j] = p_ptr8[i * width / 8 + j / 8] & 0x1;               
            }
#endif
        }
      }
    }
    break;
  default:
    printf("bpp not supported!\n");
    break;
  }
}

static void SP_YUV_expand(u32 *p_src, 
                                                        u16 *p_chroma, 
                                                        u32 width, 
                                                        u32 height,
                                                        u32 chroma_w,
                                                        u32 odd_start_x,
                                                        u32 odd_start_y,
                                                        color_format_t format, 
                                                        MT_BOOL chroma_little_endian)
{
    u32 i = 0, j = 0;
    u8 u;
    u8 v;
    u8 y;

    //hw bug, should modify in next versiion 
//    chroma_little_endian = !chroma_little_endian;
//printf("\r\n chroma_w:%d", chroma_w);
    switch(format)
    {
        case SP_YUV420_Y:
            for(i = 0; i < height; i++)
                for(j = 0; j < width; j++)
                {
                    y = p_src[i * width + j] & 0xff;    
                    if(chroma_little_endian)
                    {
                        v = (p_chroma[(i + odd_start_y) / 2 * chroma_w / 2 + (j + odd_start_x) / 2] >> 8) & 0xff;
                        u = p_chroma[(i + odd_start_y) / 2 * chroma_w / 2 + (j + odd_start_x) / 2] & 0xff;                    
                    }
                    else
                    {
                        u = (p_chroma[(i + odd_start_y) / 2 * chroma_w / 2 + (j + odd_start_x) / 2] >> 8) & 0xff;
                        v = p_chroma[(i + odd_start_y) / 2 * chroma_w / 2 + (j + odd_start_x) / 2] & 0xff;
                    }
                    p_src[i * width + j] = (0xff << 24) | (y << 16) | (u << 8) | v;        
                } 
            break;
        case SP_YUV422_Y:
            for(i = 0; i < height; i++)
                for(j = 0; j < width; j++)
                {
                    y = p_src[i * width + j] & 0xff;
                    if(chroma_little_endian)
                    {
                        v = (p_chroma[i * chroma_w / 2 + (j + odd_start_x) / 2] >> 8) & 0xff;
                        u = p_chroma[i * chroma_w / 2 + (j + odd_start_x) / 2] & 0xff;                   
                    }
                    else
                    {                    
                        u = (p_chroma[i * chroma_w / 2 + (j + odd_start_x) / 2] >> 8) & 0xff;
                        v = p_chroma[i * chroma_w / 2 + (j + odd_start_x) / 2] & 0xff;
                    }
                    p_src[i * width + j] = (0xff << 24) | (y << 16) | (u << 8) | v;        
                } 
            break;
        case SP_YUV422_Y2:
            for(i = 0; i < height; i++)
                for(j = 0; j < width; j++)
                {
                    y = p_src[i * width + j] & 0xff;
                    if(chroma_little_endian)
                    {
                        v = (p_chroma[(i + odd_start_y) / 2 * chroma_w + j] >> 8) & 0xff;
                        u = p_chroma[(i + odd_start_y) / 2 * chroma_w + j] & 0xff;                   
                    }
                    else
                    {                    
                        u = (p_chroma[(i + odd_start_y) / 2 * chroma_w + j] >> 8) & 0xff;
                        v = p_chroma[(i + odd_start_y) / 2 * chroma_w + j] & 0xff;
                    }
                    p_src[i * width + j] = (0xff << 24) | (y << 16) | (u << 8) | v;     
                } 
            break;            
        case SP_YUV444_Y:
            for(i = 0; i < height; i++)
                for(j = 0; j < width; j++)
                {
                    y = p_src[i * width + j] & 0xff;
                    if(chroma_little_endian)
                    {
                        v = (p_chroma[i * chroma_w + j] >> 8) & 0xff;
                        u = p_chroma[i * chroma_w + j] & 0xff;
                    }
                    else
                    {
                        u = (p_chroma[i * chroma_w + j] >> 8) & 0xff;
                        v = p_chroma[i * chroma_w + j] & 0xff;
                    }
                    p_src[i * width + j] = (0xff << 24) | (y << 16) | (u << 8) | v;        
                }            
            break;
        default:
            break;
    }
}

static void SP_CMYK_expand(u32 *p_src, 
                                                        u16 *p_yk, 
                                                        u32 width, 
                                                        u32 height,
                                                        MT_BOOL yk_little_endian)
{
    u32 i = 0, j = 0;
    u32 cmyk;
    u32 cm;
    u32 yk;

    for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
            cm = p_src[i * width + j];
            if(yk_little_endian)
                yk = ((p_yk[i * width + j] & 0xff) << 8) | ((p_yk[i * width + j] >> 8) & 0xff);
            else
                yk = p_yk[i * width + j];
            p_src[i * width + j] = (cm << 16) | yk; 
        } 
}
#define EXPAND(color, bit_num) ((color & 0x1) ? ((1 << (8 - bit_num)) - 1) : 0)

static void cmyk2rgb(u32 *p_src,
          u32 width,
          u32 height, 
          u32 cmyk_max, 
          u32 cmyk_coef)
{
    u32 i = 0, j = 0;
    u32 c = 0,y = 0,m = 0,k = 0;
    u32 r = 0, g = 0, b = 0;
    u32 tmp = 0;
    u32 coef = 0;

    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        tmp = p_src[i * width + j];
        k = tmp & 0xff;
        y = (tmp >> 8) & 0xff;
        m = (tmp >> 16) & 0xff;
        c = (tmp >> 24) & 0xff;

        if(cmyk_max == 100)
        {
            k = k > 100 ? 100 : k;
            y = y > 100 ? 100 : y;
            m = m > 100 ? 100 : m;
            c = c > 100 ? 100 : c;            
        }
        
        if(cmyk_max != 0)
        {
            c = cmyk_max - c;
            m = cmyk_max - m;
            y = cmyk_max - y;
            k = cmyk_max - k;            
        }
        coef = cmyk_coef * k;   
        coef = (coef >> 8) + ((coef >> 7) & 0x01);

        r = c * coef >> 8;
        g = m * coef >> 8;
        b = y * coef >> 8;

        if (r < 0)
            r = 0;
        if (g < 0)
            g = 0;
        if (b < 0)
            b = 0;        
        p_src[i * width + j] = (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
      }
//      printf("\r\n~~~~~~~~~~~cmyk_coeff:0x%08x, cmyk_max:0x%08x", cmyk_coef, cmyk_max);
}

//gray_8/rgb->argb8888, lut->argb8888/ayuv8888
static void color_expand(u32 *p_src,
          u32 width,
          u32 height,
          color_format_t format,
          u32 *p_palt_buf,
          palette_format_t palt_fmt,
          MT_BOOL palt_little_endian,
          MT_BOOL alpha_en,
          u32 expand_mode)
{
  u32 i = 0, j = 0;
  u32 a = 0, r = 0, g = 0, b = 0;
  u32 tmp = 0;
  u32 idx = 0;
  MT_BOOL alpha_disabled = !alpha_en;
  u32 expand = 0;
  switch(format)
  {
  case GRAY_8:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        a = p_src[i * width + j] & 0xff;
        p_src[i * width + j] = (a << 24) | (a << 16) | (a << 8) | a;        
      }
    break;
  case CLUT_1:
  case CLUT_2:
  case CLUT_4:
  case CLUT_8:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        idx = p_src[i * width + j];
        if((palt_fmt == GPE_ARIA_PALT_ARGB8888) || (palt_fmt == GPE_ARIA_PALT_AYUV8888))
        {
          if(palt_little_endian)
          {
            tmp = p_palt_buf[idx];
            if(alpha_disabled)
              a = 0xff;
            else
              a = tmp & 0xff;
            r = (tmp >> 8) & 0xff;
            g = (tmp >> 16) & 0xff;
            b = (tmp >> 24) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
          else
          {
            if(alpha_disabled)
              p_src[i * width + j] = p_palt_buf[idx] | 0xff000000;
            else
              p_src[i * width + j] = p_palt_buf[idx];
          }
        }
        else if((palt_fmt == GPE_ARIA_PALT_RGBA8888) || (palt_fmt == GPE_ARIA_PALT_YUVA8888))
        {
          if(palt_little_endian)
          {
            tmp = p_palt_buf[idx];
            r = tmp & 0xff;
            g = (tmp >> 8) & 0xff;
            b = (tmp >> 16) & 0xff;
            if(alpha_disabled)
              a = 0xff;
            else
              a = (tmp >> 24) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
          else
          {
            tmp = p_palt_buf[idx];
            if(alpha_disabled)
              a = 0xff;
            else
              a = tmp & 0xff;
            b = (tmp >> 8) & 0xff;
            g = (tmp >> 16) & 0xff;
            r = (tmp >> 24) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
        }
      }
    break;
  case ALUT44:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        idx = p_src[i * width + j] & 0xf;
        a = (p_src[i * width + j] >> 4) & 0xf;
        if(alpha_disabled)
          a = 0xff;
        else
        {
            if(expand_mode == EXP_PAD_LOW_BIT)            
              a = (a << 4) | EXPAND(a, 4);
            else if(expand_mode == EXP_PAD_OLD_MODE)
              a = (a << 4) | a ;
            else if(expand_mode == EXP_PAD_ZERO)
              a = a << 4;
        }

        if((palt_fmt == GPE_ARIA_PALT_ARGB8888) || (palt_fmt == GPE_ARIA_PALT_AYUV8888))
        {
          if(palt_little_endian)
          {
            tmp = p_palt_buf[idx];
            r = (tmp >> 8) & 0xff;
            g = (tmp >> 16) & 0xff;
            b = (tmp >> 24) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
          else
            p_src[i * width + j] = (p_palt_buf[idx] & 0xffffff) | (a << 24);
        }
        else if((palt_fmt == GPE_ARIA_PALT_RGBA8888) || (palt_fmt == GPE_ARIA_PALT_YUVA8888))
        {
          if(palt_little_endian)
          {
            tmp = p_palt_buf[idx];
            r = tmp & 0xff;
            g = (tmp >> 8) & 0xff;
            b = (tmp >> 16) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
          else
          {
            tmp = p_palt_buf[idx];
            b = (tmp >> 8) & 0xff;
            g = (tmp >> 16) & 0xff;
            r = (tmp >> 24) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
        }
      }
    break;
  case ALUT88:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        idx = p_src[i * width + j] & 0xff;
        if(alpha_disabled)
          a = 0xff;
        else
          a = (p_src[i * width + j] >> 8) & 0xff;

        if((palt_fmt == GPE_ARIA_PALT_ARGB8888) || (palt_fmt == GPE_ARIA_PALT_AYUV8888))
        {
          if(palt_little_endian)
          {
            tmp = p_palt_buf[idx];
            r = (tmp >> 8) & 0xff;
            g = (tmp >> 16) & 0xff;
            b = (tmp >> 24) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
          else
            p_src[i * width + j] = (p_palt_buf[idx] & 0xffffff) | (a << 24);
        }
        else if((palt_fmt == GPE_ARIA_PALT_RGBA8888) || (palt_fmt == GPE_ARIA_PALT_YUVA8888))
        {
          if(palt_little_endian)
          {
            tmp = p_palt_buf[idx];
            r = tmp & 0xff;
            g = (tmp >> 8) & 0xff;
            b = (tmp >> 16) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
          else
          {
            tmp = p_palt_buf[idx];
            b = (tmp >> 8) & 0xff;
            g = (tmp >> 16) & 0xff;
            r = (tmp >> 24) & 0xff;
            p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
          }
        }
      }
    break;
  case RGB233:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        tmp = p_src[i * width + j];
        b = tmp & 0x7;
        g = (tmp >> 3) & 0x7;
        r = (tmp >> 6) & 0x3;
        a = 0xff;
        if(expand_mode == EXP_PAD_LOW_BIT)     
        {            
            expand = (EXPAND(r, 2) << 16) | (EXPAND(g, 3) << 8) | EXPAND(b, 3);
            p_src[i * width + j] = (a << 24) | (r << 22) | (g << 13) | (b << 5) | expand;
        }
        else if(expand_mode == EXP_PAD_OLD_MODE)
        {
            p_src[i * width + j] = (a << 24) | (r << 22)  | (r << 20) | (r << 18) 
                | (g << 13) | (g << 10) | (b << 5) | (b << 2);
        }
        else if(expand_mode == EXP_PAD_ZERO)
        {
            p_src[i * width + j] = (a << 24) | (r << 22)
                | (g << 13) | (b << 5);            
        }
      }
    break;
  case RGB565:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {    
        tmp = p_src[i * width + j];
        b = tmp & 0x1f;
        g = (tmp >> 5) & 0x3f;
        r = (tmp >> 11) & 0x1f;
        a = 0xff;
        if(expand_mode == EXP_PAD_LOW_BIT)     
        {              
            expand = (EXPAND(r, 5) << 16) | (EXPAND(g, 6) << 8) | EXPAND(b, 5);
            p_src[i * width + j] = (a << 24) | (r << 19) | (g << 10) | (b << 3) | expand;
        }
        else if(expand_mode == EXP_PAD_OLD_MODE)
        {
            p_src[i * width + j] = (a << 24) | (r << 19) | (((r >> 5) & 0x7) << 16) |
                (g << 10) | (((g >> 6) & 0x3) << 8) | (b << 3) | ((b >> 5) & 0x7); 
        }
        else if(expand_mode == EXP_PAD_ZERO)
        {
            p_src[i * width + j] = (a << 24) | (r << 19) | (g << 10) | (b << 3);        
        }
      }
    break;
  case ARGB1555:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        tmp = p_src[i * width + j];
        b = tmp & 0x1f;
        g = (tmp >> 5) & 0x1f;
        r = (tmp >> 10) & 0x1f;
        if(alpha_disabled)
          a = 0xff;
        else
        {
            if(expand_mode == EXP_PAD_ZERO)
                a = ((tmp >> 15) & 0x1) << 7;
            else
                a = (((tmp >> 15) & 0x1)  == 1) ? 0xff : 0;
        }
        if(expand_mode == EXP_PAD_LOW_BIT)     
        {            
            expand = (EXPAND(r, 5) << 16) | (EXPAND(g, 5) << 8) | EXPAND(b, 5);
            p_src[i * width + j] = (a << 24) | (r << 19) | (g << 11) | (b << 3) | expand;
        }
        else if(expand_mode == EXP_PAD_OLD_MODE)
        {
            p_src[i * width + j] = (a << 24) | (r << 19) | (((r >> 5) & 0x7) << 16) |
                (g << 11) | (((g >> 5) & 0x7) << 8) | (b << 3) | ((b >> 5) & 0x7); 
        }        
        else if(expand_mode == EXP_PAD_ZERO)
        {
            p_src[i * width + j] = (a << 24) | (r << 19) | (g << 11) | (b << 3);        
        }
      }
    break;
  case RGBA5551:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {      
        tmp = p_src[i * width + j];
        if(alpha_disabled)
          a = 0xff;
        else
        {
            if(expand_mode == EXP_PAD_ZERO)
                a = (tmp & 0x1) << 7;
            else
                a = ((tmp & 0x1) == 1) ? 0xff : 0;
        }
        b = (tmp >> 1) & 0x1f;
        g = (tmp >> 6) & 0x1f;
        r = (tmp >> 11) & 0x1f;
        if(expand_mode == EXP_PAD_LOW_BIT)     
        {            
            expand = (EXPAND(r, 5) << 16) | (EXPAND(g, 5) << 8) | EXPAND(b, 5);
            p_src[i * width + j] = (a << 24) | (r << 19) | (g << 11) | (b << 3) | expand;
        }
        else if(expand_mode == EXP_PAD_OLD_MODE)
        {
            p_src[i * width + j] = (a << 24) | (r << 19) | (((r >> 5) & 0x7) << 16) |
                (g << 11) | (((g >> 5) & 0x7) << 8) | (b << 3) | ((b >> 5) & 0x7); 
        }        
        else if(expand_mode == EXP_PAD_ZERO)
        {
            p_src[i * width + j] = (a << 24) | (r << 19) | (g << 11) | (b << 3);        
        }
      }
    break;
  case ARGB4444:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {      
        tmp = p_src[i * width + j];
        b = tmp & 0xf;
        g = (tmp >> 4) & 0xf;
        r = (tmp >> 8) & 0xf;
        if(alpha_disabled)
          a = 0xff;
        else
        {
          a = (tmp >> 12) & 0xf;
          if(expand_mode == EXP_PAD_LOW_BIT)
          {
            a = (a << 4) | EXPAND(a, 4);
          }
          else if(expand_mode == EXP_PAD_OLD_MODE)
          {
            a = (a << 4) | a;
          }
          else if(expand_mode == EXP_PAD_ZERO)
          {
            a = a << 4;
          }
        }
        if(expand_mode == EXP_PAD_LOW_BIT)     
        {               
            expand = (EXPAND(r, 4) << 16) | (EXPAND(g, 4) << 8) | EXPAND(b, 4);
            p_src[i * width + j] = (a << 24)  | (r << 20)  | (g << 12) | (b << 4) | expand;
        }
        else if(expand_mode == EXP_PAD_OLD_MODE)
        {
            p_src[i * width + j] = (a << 24) | (r << 20) | (r << 16) 
                | (g << 12) | (g << 8) | (b << 4) | b;        
        }
        else if(expand_mode == EXP_PAD_ZERO)
        {
            p_src[i * width + j] = (a << 24) | (r << 20) | (g << 12) | (b << 4);        
        }
      }
    break;
  case RGBA4444:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {       
        tmp = p_src[i * width + j];
        if(alpha_disabled)
          a = 0xff;
        else
        {
          a = tmp & 0xf;
          if(expand_mode == EXP_PAD_LOW_BIT)
          {
            a = (a << 4) | EXPAND(a, 4);
          }
          else if(expand_mode == EXP_PAD_OLD_MODE)
          {
            a = (a << 4) | a;
          }
          else if(expand_mode == EXP_PAD_ZERO)
          {
            a = a << 4;
          }       
        }
        b = (tmp >> 4) & 0xf;
        g = (tmp >> 8) & 0xf;
        r = (tmp >> 12) & 0xf;

        if(expand_mode == EXP_PAD_LOW_BIT)     
        {               
            expand = (EXPAND(r, 4) << 16) | (EXPAND(g, 4) << 8) | EXPAND(b, 4);
            p_src[i * width + j] = (a << 24)  | (r << 20)  | (g << 12) | (b << 4) | expand;
        }
        else if(expand_mode == EXP_PAD_OLD_MODE)
        {
            p_src[i * width + j] = (a << 24) | (r << 20) | (r << 16) 
                | (g << 12) | (g << 8) | (b << 4) | b;        
        }
        else if(expand_mode == EXP_PAD_ZERO)
        {
            p_src[i * width + j] = (a << 24) | (r << 20) | (g << 12) | (b << 4);        
        }
      }
    break;
  case RGB888:
    printf("\r\n RGB888");
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        p_src[i * width + j] = (p_src[i * width + j] & 0x00ffffff) | 0xff000000;
      }    
    break;
  case BGR888:
    printf("\r\n BGR888");
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        tmp = p_src[i * width + j];
        b = (tmp >> 16) & 0xff;
        g = (tmp >> 8) & 0xff;
        r = tmp & 0xff;
        p_src[i * width + j] = (0xff << 24) | (r << 16) | (g << 8) | b;
      }    
    break;    
  case AYUV8888:
  case ARGB8888:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        tmp = p_src[i * width + j];
        if(alpha_disabled)
          a = 0xff000000;
        else
          a = tmp & 0xff000000;
        p_src[i * width + j] = (p_src[i * width + j] & 0x00ffffff) | a;
      }
    break;
  case Y1VY0U: //changed to uy0vy1
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        tmp = p_src[i * width + j];
        p_src[i * width + j] = ((tmp & 0xff) << 24) |
                (((tmp >> 8) & 0xff) << 16) |
                (((tmp >> 16) & 0xff) << 8) |
                ((tmp >> 24) & 0xff);
      }
    break;
  case YUVA8888:
  case RGBA8888:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
      {
        tmp = p_src[i * width + j];
        if(alpha_disabled)
          a = 0xff;
        else
          a = tmp & 0xff;
        b = (tmp >> 8) & 0xff;
        g = (tmp >> 16) & 0xff;
        r = (tmp >> 24) & 0xff;
        p_src[i * width + j] = (a << 24) | (r << 16) | (g << 8) | b;
      }
    break;
  case XY:
  case XYL:
  case XYC:
  case XYLC:
  case CMYK8888:
  case KYMC8888:
    break;
  default:
    printf("true rgb format not supported!\n");
    break;
  }
}

static MT_BOOL ck_match(u32 mod, u8 val, u8 ck_min, u8 ck_max)
{
    if(mod == KEY_MATCH_ALL)
        return TRUE;
    if((mod == KEY_MATCH_INSIDE_MIN_MAX) && (val >= ck_min) && (val <= ck_max))
        return TRUE;
    if((mod == KEY_MATCH_OUTSIDE_MIN_MAX) && ((val < ck_min) || (val > ck_max)))
        return TRUE;
    return FALSE;
}

static void generate_argb_ck_mask(u32 *p_src,
          u32 width,
          u32 height,
          u8 *p_buf_ck,
          u32 ck_min,
          u32 ck_max,
          u32 ck_mod,
          MT_BOOL yuv422flag)
{
    u32 i = 0, j = 0;
    u8 a = 0, r = 0, g = 0, b = 0;
    u8 ck_a_min = 0, ck_r_min = 0, ck_g_min = 0, ck_b_min = 0;
    u8 ck_a_max = 0, ck_r_max = 0, ck_g_max = 0, ck_b_max = 0;
    u8 ck_a_mod = 0, ck_r_mod = 0, ck_g_mod = 0, ck_b_mod = 0;
    MT_BOOL ck_a_match = FALSE, ck_r_match = FALSE, ck_g_match = FALSE, ck_b_match = FALSE;
    u32 tmp = 0;
    
    ck_a_mod = (ck_mod >> 12) & 0xf;
    ck_r_mod = (ck_mod >> 8) & 0xf;
    ck_g_mod = (ck_mod >> 4) & 0xf;
    ck_b_mod = ck_mod & 0xf;   

    if((ck_a_mod == KEY_MATCH_NONE) 
        || (ck_r_mod == KEY_MATCH_NONE) 
        || (ck_g_mod == KEY_MATCH_NONE) 
        || (ck_b_mod == KEY_MATCH_NONE))
        return;

    ck_a_min = (ck_min >> 24) & 0xff;
    ck_r_min = (ck_min >> 16) & 0xff;
    ck_g_min = (ck_min >> 8) & 0xff;
    ck_b_min = ck_min & 0xff;
    ck_a_max = (ck_max >> 24) & 0xff;
    ck_r_max = (ck_max >> 16) & 0xff;
    ck_g_max = (ck_max >> 8) & 0xff;
    ck_b_max = ck_max & 0xff;   

    if(yuv422flag)
    {
        u8 u, y0, v, y1;
        for(i = 0; i < height; i++)
          for(j = 0; j < width; j++)
          {
                tmp = p_src[i * width + j];                
                u = (tmp >> 24) & 0xff;
                y0 = (tmp >> 16) & 0xff;
                v = (tmp >> 8) & 0xff;
                y1 = tmp & 0xff;

                ck_a_match = ck_match(ck_r_mod, y0, ck_r_min, ck_r_max);
                ck_r_match = ck_match(ck_r_mod, y1, ck_r_min, ck_r_max);                
                ck_g_match = ck_match(ck_g_mod, u, ck_g_min, ck_g_max);
                ck_b_match = ck_match(ck_b_mod, v, ck_b_min, ck_b_max);
                if(ck_a_match && ck_r_match && ck_g_match && ck_b_match)
                    p_buf_ck[i * width + j] = !p_buf_ck[i * width + j];
                else if(ck_a_match && ck_g_match && ck_b_match)
                    p_buf_ck[i * width + j] = 2;
                else if(ck_r_match && ck_g_match && ck_b_match)
                    p_buf_ck[i * width + j] = 3;
          }    
    }
    else
    {
        for(i = 0; i < height; i++)
          for(j = 0; j < width; j++)
          {
                tmp = p_src[i * width + j];
                a = (tmp >> 24) & 0xff;
                r = (tmp >> 16) & 0xff;
                g = (tmp >> 8) & 0xff;
                b = tmp & 0xff;
                ck_a_match = ck_match(ck_a_mod, a, ck_a_min, ck_a_max);
                ck_r_match = ck_match(ck_r_mod, r, ck_r_min, ck_r_max);
                ck_g_match = ck_match(ck_g_mod, g, ck_g_min, ck_g_max);
                ck_b_match = ck_match(ck_b_mod, b, ck_b_min, ck_b_max);
                if(ck_a_match && ck_r_match && ck_g_match && ck_b_match)
                    p_buf_ck[i * width + j] = !p_buf_ck[i * width + j];
          }
    }
}

static void generate_lut_ck_mask(u32 *p_src,
          u32 width,
          u32 height,
          u8 *p_buf_ck,
          u32 ck_min,
          u32 ck_max,
          u32 ck_mod,
          u32 src_fmt)
{
    u32 i = 0, j = 0;
    u8 a = 0, r = 0, g = 0, b = 0;
    u8 ck_a_min = 0,  ck_b_min = 0;
    u8 ck_a_max = 0, ck_b_max = 0;
    u8 ck_a_mod = 0, ck_b_mod = 0;
    MT_BOOL ck_a_match = FALSE, ck_b_match = FALSE;
    u32 idx = 0;    
    
    ck_a_mod = (ck_mod >> 12) & 0xf;
    ck_b_mod = ck_mod & 0xf;   

    if((ck_a_mod == KEY_MATCH_NONE) 
        || (ck_b_mod == KEY_MATCH_NONE))
        return;
    ck_a_min = (ck_min >> 24) & 0xff;
    ck_b_min = ck_min & 0xff;
    ck_a_max = (ck_max >> 24) & 0xff;
    ck_b_max = ck_max & 0xff;   

    switch(src_fmt)
    {
    case CLUT_1:
    case CLUT_2:
    case CLUT_4:
    case CLUT_8:
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          idx = p_src[i * width + j];
          ck_b_match = ck_match(ck_b_mod, idx, ck_b_min, ck_b_max);
          if(ck_b_match)
                p_buf_ck[i * width + j] = !p_buf_ck[i * width + j];
        }
      break;
    case ALUT44:
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          idx = p_src[i * width + j] & 0xf;
          ck_b_match = ck_match(ck_b_mod, idx, ck_b_min, ck_b_max);
          a = (p_src[i * width + j] >> 4) & 0xf;
//          a = (a << 4) | EXPAND(a, 4);
          a = (a << 4) | a;
          ck_a_match = ck_match(ck_a_mod, a, ck_a_min, ck_a_max);
          if(ck_a_match && ck_b_match)
                p_buf_ck[i * width + j] = !p_buf_ck[i * width + j];
        }        
    case ALUT88:
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          idx = p_src[i * width + j] & 0xff;
          a = (p_src[i * width + j] >> 8) & 0xff;
          ck_b_match = ck_match(ck_b_mod, idx, ck_b_min, ck_b_max);
          ck_a_match = ck_match(ck_a_mod, a, ck_a_min, ck_a_max);
          if(ck_a_match && ck_b_match)
                p_buf_ck[i * width + j] = !p_buf_ck[i * width + j];          
        }        
      break;
    default:
      printf("lut format not supported!\n");
      break;
    }
}

//lut1/2/4/8,alut44/88->alut44,lut8,alut88
static void lut_expand(u32 *p_src, 
                         u32 width, 
                         u32 height, 
                         color_format_t src_fmt, 
                         color_format_t dst_fmt)
{
    u32 i = 0, j = 0;
    u8 a = 0;
    u8 idx = 0;

    switch(src_fmt)
    {
    case CLUT_1:
    case CLUT_2:
    case CLUT_4:
    case CLUT_8:
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          p_src[i * width + j] = p_src[i * width + j] | (0xff << 24);
        }
      break;
    case ALUT44:
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          u8 idx = p_src[i * width + j] & 0xf;
          u8 a = (p_src[i * width + j] >> 4) & 0xf;
//          p_src[i * width + j] = idx |(((a << 4) |EXPAND(a, 4)) << 24);
          p_src[i * width + j] = idx |(((a << 4) |a) << 24);
        }
      break;
    case ALUT88:
      for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
          u8 idx = p_src[i * width + j] & 0xff;
          u8 a = (p_src[i * width + j] >> 8) & 0xff;
          p_src[i * width + j] = idx |(a << 24);
        }        
      break;
    default:
      printf("lut format not supported!\n");
      break;        
    }
}

#if 1
static void ck_buf_422_to_444(u8 *p_src, u8 *p_dst, u32 w, u32 h, u32 uyvy_width, u32 odd_start_x)
{
  u32 i = 0, j = 0;
  printf("\r\n w:%d, h:%d, uyvy_width:%d, odd_start:%d", w, h, uyvy_width, odd_start_x);
  if(odd_start_x == 0)
  { 
      for(i = 0; i < h; i++)
      {
          for(j = 0; j < (w + 1) / 2; j++)
          {
            if(p_src[i * uyvy_width / 2 + j] == 2)
            {
                p_dst[i * w + 2 * j] = 1;
                if(2 * j + 1 < w)
                    p_dst[i * w + 2 * j + 1] = 0;
            }
            else if(p_src[i * uyvy_width / 2 + j] == 3)
            {
                p_dst[i * w + 2 * j] = 0;
                if(2 * j + 1 < w)
                    p_dst[i * w + 2 * j + 1] = 1;            
            }
            else
            {
                p_dst[i * w + 2 * j] = p_src[i * uyvy_width / 2 + j];
                if(2 * j + 1 < w)
                    p_dst[i * w + 2 * j + 1] = p_src[i * uyvy_width / 2 + j];
            }
          }
      }
  }
  else
  {
    for(i = 0; i < h; i++)
    {
        if(p_src[i * uyvy_width / 2] == 2)
            p_dst[i * w] = 0;
        else if(p_src[i * uyvy_width / 2] == 3)
            p_dst[i * w] = 1;
        else            
            p_dst[i * w] = p_src[i * uyvy_width / 2];
        for(j = 1; j < (w + 1 + 1) / 2; j++)
        {
            if(p_src[i * uyvy_width / 2 + j] == 2)
            {
                p_dst[i * w + 2 * j] = 1;
                if(2 * j + 1 < w)
                    p_dst[i * w + 2 * j + 1] = 0;                    
            }
            else if(p_src[i * uyvy_width / 2 + j] == 3)
            {
                p_dst[i * w + 2 * j] = 0;
                if(2 * j + 1 < w)
                    p_dst[i * w + 2 * j + 1] = 1;                  
            }
            else
            {
                p_dst[i * w + 2 * j - 1] = p_src[i * uyvy_width / 2 + j];
                if(2 * j < w)
                    p_dst[i * w + 2 * j] = p_src[i * uyvy_width / 2 + j];
            }
        }
    }
  }
}
#else
static void ck_buf_422_to_444(u8 *p_src, u8 *p_dst, u32 w, u32 h)
{
  u32 i = 0, j = 0;
  for(i = 0; i < h; i++)
    {
    for(j = 0; j < w / 2; j++)
    {
      p_dst[i * w + 2 * j] = p_src[i * w / 2 + j];
      p_dst[i * w + 2 * j + 1] = p_src[i * w / 2 + j];
      printf("%d, %d, ", p_dst[i * w + 2 * j], p_dst[i * w + 2 * j + 1]);
    }
    printf("\r\n");
    }
}
#endif
static void src_format_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u8 *p_ptr = NULL;
  u8 *p_buf_ck = NULL;
  u32 w = 0, h = 0;
  u32 yuv422_flag = 0;
  u32 *palt_buf = NULL;

  if(p_gpe->src1_in)
  {
    //////////////////////////////////////////
    // convert src to 32bpp and big endian
    if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
    {
        w = p_ctx->src_img.width;
        h = p_ctx->src_img.height;        
    }
    else
    {
        w = p_gpe->src_w;
        h = p_gpe->src_h;
    }
    if(p_ctx->src_img.color_info.color_fmt == Y1VY0U)
    {
      w = p_gpe->src1_w_uyvy / 2;
      yuv422_flag = 1;
    }
    else
      yuv422_flag = 0;
    if((p_ctx->src_img.color_info.color_fmt == XY) ||
        (p_ctx->src_img.color_info.color_fmt == XYC) ||
        (p_ctx->src_img.color_info.color_fmt == XYL) ||
        (p_ctx->src_img.color_info.color_fmt == XYLC))
    {
      w = (p_ctx->xylc_cfg.xylc_num) * 2;
      h = 2;
    }
    p_ptr = (u8 *)gpe_malloc(w * h * 4);
    GPE_SPN_ASSERT(p_ptr != NULL);
    pixel_expand(p_gpe->p_src1_buf,
                 (u32 *)p_ptr,
                 w,
                 h,
                 p_ctx->src_img.color_info.bpp,
                 p_ctx->src_img.color_info.little_endian,
                 yuv422_flag,
                 p_ctx->src_img.negative_stride);

    gpe_free(p_gpe->p_src1_buf);
    p_gpe->p_src1_buf = p_ptr;

    ////////////////////////////////////////////
    // gray_8/rgb->argb8888, lut->argb8888/ayuv8888
    // or lut1/2/4/8,alut44/88->alut44,lut8,alut88
    p_buf_ck = (u8 *)gpe_malloc(w * h);

    GPE_SPN_ASSERT(p_buf_ck != NULL);
    if(p_ctx->src_img.ck_select == MASK_KEY_MATCH)
        memset(p_buf_ck, 0, w * h);
    else
        memset(p_buf_ck, 1, w * h);

    
    if(p_gpe->src1_color_expan)
    {
      if(p_ctx->src_img.with_palette)
      {
          generate_lut_ck_mask((u32 *)p_gpe->p_src1_buf,
              w,
              h,
              p_buf_ck,
              p_ctx->src_img.ck_min,
              p_ctx->src_img.ck_max,
              p_ctx->src_img.ck_mod,
              p_ctx->src_img.color_info.color_fmt);        
      }      
      //sp scalexxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
      if((p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y) 
        || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y) 
        || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2) 
        || (p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y))
      {
        if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
        {
            SP_YUV_expand((u32 *)p_gpe->p_src1_buf, 
                                            (u16 *)p_gpe->p_src0_buf, 
                                            w, 
                                            h, 
                                            w,
                                            0,
                                            0,
                                            p_ctx->src_img.color_info.color_fmt,
                                            p_ctx->src_img.color_info.little_endian);        
        }
        else
        {
            SP_YUV_expand((u32 *)p_gpe->p_src1_buf, 
                                            (u16 *)p_gpe->p_src0_buf, 
                                            w, 
                                            h, 
                                            p_gpe->src0_w,
                                            p_ctx->src_img.rect.x % 2,
                                            p_ctx->src_img.rect.y % 2,
                                            p_ctx->src_img.color_info.color_fmt,
                                            p_ctx->src_img.color_info.little_endian);
        }
        gpe_free(p_gpe->p_src0_buf);
        p_gpe->p_src0_buf = NULL;
      }
      else if(p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
      {
        SP_CMYK_expand((u32 *)p_gpe->p_src1_buf, 
                                       (u16 *)p_gpe->p_src0_buf, 
                                            w, 
                                            h, 
                                            p_ctx->src_img.color_info.little_endian);
      }
      else
      {
            if(p_ctx->src_img.palt_buf != (phys_addr_t)0)                
                palt_buf = (u32 *)mt_mmz_map(p_ctx->src_img.palt_buf, 0);
            else
                palt_buf = NULL;
            color_expand((u32 *)p_gpe->p_src1_buf,
             w,
             h,
             p_ctx->src_img.color_info.color_fmt,
             palt_buf,
             p_ctx->src_img.palt_format,
             p_ctx->src_img.palt_little_endian,
             p_ctx->src_img.color_info.alpha_ch_en,
             p_ctx->color_exp_mode);
            if(p_ctx->src_img.palt_buf != (phys_addr_t)0)    
                mt_mmz_unmap((void *)palt_buf);
       }
//      printf("\r\n color_exp_mode:%d", p_ctx->color_exp_mode);
       if(p_ctx->src_img.with_palette == FALSE)
       {
             generate_argb_ck_mask((u32 *)p_gpe->p_src1_buf,
                 w,
                 h,
                 p_buf_ck,
                 p_ctx->src_img.ck_min,
                 p_ctx->src_img.ck_max,
                 p_ctx->src_img.ck_mod,
                 yuv422_flag);      
        }
    }
    else
    {
      generate_lut_ck_mask((u32 *)p_gpe->p_src1_buf,
          w,
          h,
          p_buf_ck,
          p_ctx->src_img.ck_min,
          p_ctx->src_img.ck_max,
          p_ctx->src_img.ck_mod,
          p_ctx->src_img.color_info.color_fmt);
   
      lut_expand((u32 *)p_gpe->p_src1_buf,
             w,
             h,
             p_ctx->src_img.color_info.color_fmt,
             p_ctx->dst_img.color_info.color_fmt); 
    }              
    p_gpe->p_src1_ck_buf = p_buf_ck;
    
    ////////////////////////////////////////////
    if((p_ctx->src_img.color_info.color_fmt == CMYK8888) 
        || (p_ctx->src_img.color_info.color_fmt == KYMC8888)
        || (p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM))
      {
          cmyk2rgb((u32 *)p_gpe->p_src1_buf, 
            w,
            h, 
            p_ctx->cmyk_max, 
            p_ctx->cmyk_coef);
      }   
    ////////////////////////////////////////////
    // yuv422->ayuv8888
    if(p_ctx->src_img.color_info.color_fmt == Y1VY0U)
    {
      if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
      {
          p_ptr = (u8 *)gpe_malloc(p_ctx->src_img.width * p_ctx->src_img.height  * 4);
          GPE_SPN_ASSERT(p_ptr != NULL);

          uyvy2ayuv((u32 *)p_gpe->p_src1_buf, (u32 *)p_ptr, p_ctx->src_img.width , p_ctx->src_img.height, p_ctx->src_img.width, 0);
          gpe_free(p_gpe->p_src1_buf);
          p_gpe->p_src1_buf = p_ptr;
          p_ptr = (u8 *)gpe_malloc(p_ctx->src_img.width * p_ctx->src_img.height);
          GPE_SPN_ASSERT(p_ptr != NULL);

            //colorkey 目前还暂不考虑奇数位置的情况
//          ck_buf_422_to_444(p_gpe->p_src1_ck_buf, p_ptr, p_ctx->src_img.width , p_ctx->src_img.height);        
            ck_buf_422_to_444(p_gpe->p_src1_ck_buf, p_ptr, p_ctx->src_img.width , p_ctx->src_img.height,  p_ctx->src_img.width, 0);        
      }
      else
      {
          p_ptr = (u8 *)gpe_malloc(p_gpe->src_w * p_gpe->src_h * 4);
          GPE_SPN_ASSERT(p_ptr != NULL);

          uyvy2ayuv((u32 *)p_gpe->p_src1_buf, (u32 *)p_ptr, p_gpe->src_w, p_gpe->src_h, p_gpe->src1_w_uyvy, p_ctx->src_img.rect.x % 2);
          gpe_free(p_gpe->p_src1_buf);
          p_gpe->p_src1_buf = p_ptr;
#if 0         
          p_ptr = (u8 *)mtos_malloc(p_gpe->src_w * p_gpe->src_h);
          GPE_SPN_ASSERT(p_ptr != NULL);

            //colorkey 目前还暂不考虑奇数位置的情况
          ck_buf_422_to_444(p_gpe->p_src1_ck_buf, p_ptr, p_gpe->src_w, p_gpe->src_h);
#else
          p_ptr = (u8 *)gpe_malloc(p_gpe->src_w * p_gpe->src_h);
          GPE_SPN_ASSERT(p_ptr != NULL);


            //colorkey 目前还暂不考虑奇数位置的情况
          ck_buf_422_to_444(p_gpe->p_src1_ck_buf, p_ptr, p_gpe->src_w, p_gpe->src_h, p_gpe->src1_w_uyvy, p_ctx->src_img.rect.x % 2);
#endif
      }
      gpe_free(p_gpe->p_src1_ck_buf);
      p_gpe->p_src1_ck_buf = p_ptr;
    }

    ////////////////////////////////////////////
    // ayuv8888 to argb8888
    if(p_ctx->scale_en || p_ctx->gaussian_blur_en)
    {
        if(p_ctx->src1_yuv2rgb_en)
          ayuv2argb((u32 *)p_gpe->p_src1_buf, p_ctx->src_img.width , p_ctx->src_img.height);
        // argb8888 to ayuv8888
        if(p_ctx->src1_rgb2yuv_en)
          argb2ayuv((u32 *)p_gpe->p_src1_buf, p_ctx->src_img.width , p_ctx->src_img.height);        
    }
    else
    {
        if(p_ctx->src1_yuv2rgb_en)
          ayuv2argb((u32 *)p_gpe->p_src1_buf, p_gpe->src_w, p_gpe->src_h);
        // argb8888 to ayuv8888
        if(p_ctx->src1_rgb2yuv_en)
          argb2ayuv((u32 *)p_gpe->p_src1_buf, p_gpe->src_w, p_gpe->src_h);
    }
  }
  else //used to store data after blend/rop, and before draw mask
  {
    p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
    GPE_SPN_ASSERT(p_ptr != NULL);

    p_gpe->p_src1_buf = p_ptr;
  }

    //merge mask with src colorkey
    if(p_ctx->src_with_mask)
    {
        u32 i = 0, j = 0;
        u8 *mask_ptr = (u8 *)p_gpe->p_src0_buf;
        for(i = 0; i < h; i++)
        {
            for(j = 0; j < w; j++)
            {
                if(((p_ctx->src_img.ck_select == MASK_KEY_MATCH) && (p_gpe->p_src1_ck_buf[i * w + j] == 0))
                    || ((p_ctx->src_img.ck_select != MASK_KEY_MATCH) && (p_gpe->p_src1_ck_buf[i * w + j] == 1)))
                {                    
                    if((mask_ptr[i * w + j] & 0x1) == 1)
                    {
                        p_gpe->p_src1_ck_buf[i * w + j] = !p_gpe->p_src1_ck_buf[i * w + j];
                    }
                }

            } 
        }
    }
    
  if(1) //read in src2 any time
  {
    u32 bpp;
    MT_BOOL little_endian;
    u32 color_format;
    u32 *p_palt_buf = NULL;
    u32 palt_format;
    MT_BOOL palt_little_endian;
    MT_BOOL alpha_ch_en;
    MT_BOOL negative_stride;
    if(p_ctx->bg_img_en)
    {
        ////////////////////////////////////////////
        // convert src to 32bpp and big endian
        w = p_gpe->dst_w;
        if(p_ctx->bg_img.color_info.color_fmt == Y1VY0U)
        {
          w = p_gpe->src2_w_uyvy / 2;
          yuv422_flag = 1;
        }
        else
          yuv422_flag = 0;
        bpp = p_ctx->bg_img.color_info.bpp;
        little_endian = p_ctx->bg_img.color_info.little_endian;
        color_format = p_ctx->bg_img.color_info.color_fmt;
        palt_format = p_ctx->bg_img.palt_format;
        palt_little_endian = p_ctx->bg_img.palt_little_endian;
        alpha_ch_en = p_ctx->bg_img.color_info.alpha_ch_en;    
        negative_stride = p_ctx->bg_img.negative_stride;
    }
    else
    {
        ////////////////////////////////////////////
        // convert src to 32bpp and big endian
        w = p_gpe->dst_w;
        if(p_ctx->dst_img.color_info.color_fmt == Y1VY0U)
        {
            w = w / 2;
            yuv422_flag = 1;
        }
        else
            yuv422_flag = 0;
        bpp = p_ctx->dst_img.color_info.bpp;
        little_endian = p_ctx->dst_img.color_info.little_endian;
        color_format = p_ctx->dst_img.color_info.color_fmt;
        if(p_ctx->dst_img.palt_buf != (phys_addr_t)0)
            p_palt_buf =  (u32 *)mt_mmz_map(p_ctx->dst_img.palt_buf, 0); //p_ctx->dst_img.palt_buf;
        else
            p_palt_buf = NULL;
        palt_format = p_ctx->dst_img.palt_format;
        palt_little_endian = p_ctx->dst_img.palt_little_endian;
        alpha_ch_en = p_ctx->dst_img.color_info.alpha_ch_en;   
        negative_stride = p_ctx->dst_img.negative_stride;
    }
    p_ptr = (u8 *)gpe_malloc(w * p_gpe->dst_h * 4);
    GPE_SPN_ASSERT(p_ptr != NULL);

    pixel_expand(p_gpe->p_src2_buf,
               (u32 *)p_ptr,
           w,
           p_gpe->dst_h,
           bpp,
           little_endian,
           yuv422_flag,
           negative_stride);
    gpe_free(p_gpe->p_src2_buf);
    p_gpe->p_src2_buf = p_ptr;
            ////////////////////////////////////////////
    // gray_8/rgb->argb8888, lut->argb8888/ayuv8888
    p_buf_ck = (u8 *)gpe_malloc(w * p_gpe->dst_h);
    GPE_SPN_ASSERT(p_buf_ck != NULL);
    if(p_ctx->dst_img.ck_select == MASK_KEY_MATCH)
        memset(p_buf_ck, 0, w * p_gpe->dst_h);
    else
        memset(p_buf_ck, 1, w * p_gpe->dst_h);

    if(p_gpe->src2_color_expan)
    {
      color_expand((u32 *)p_gpe->p_src2_buf,
             w,
             p_gpe->dst_h,
             color_format,
             p_palt_buf,
             palt_format,
             palt_little_endian,
             alpha_ch_en,
             p_ctx->color_exp_mode);

          generate_argb_ck_mask((u32 *)p_gpe->p_src2_buf,
                w,
                p_gpe->dst_h,
                p_buf_ck,
                p_ctx->dst_img.ck_min,
                p_ctx->dst_img.ck_max,
                p_ctx->dst_img.ck_mod,
                yuv422_flag);  
    }  
    else
    {
      generate_lut_ck_mask((u32 *)p_gpe->p_src2_buf,
          w,
          p_gpe->dst_h,
          p_buf_ck,
          p_ctx->dst_img.ck_min,
          p_ctx->dst_img.ck_max,
          p_ctx->dst_img.ck_mod,
          color_format);
    }
    p_gpe->p_src2_ck_buf = p_buf_ck;        
            ////////////////////////////////////////////
    // yuv422->ayuv8888
    if(color_format == Y1VY0U)
    {
      p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
      GPE_SPN_ASSERT(p_ptr != NULL);

      if(p_ctx->bg_img_en)
        uyvy2ayuv((u32 *)p_gpe->p_src2_buf, (u32 *)p_ptr, p_gpe->dst_w, p_gpe->dst_h, p_gpe->src2_w_uyvy, p_ctx->bg_img.rect.x % 2);
      else
        uyvy2ayuv((u32 *)p_gpe->p_src2_buf, (u32 *)p_ptr, p_gpe->dst_w, p_gpe->dst_h, p_gpe->src2_w_uyvy, p_ctx->dst_img.rect.x % 2);
      gpe_free(p_gpe->p_src2_buf);
      p_gpe->p_src2_buf = p_ptr;

      p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h);
      GPE_SPN_ASSERT(p_ptr != NULL);

      //ck_buf_422_to_444(p_gpe->p_src2_ck_buf, p_ptr, p_gpe->dst_w, p_gpe->dst_h);
      ck_buf_422_to_444(p_gpe->p_src2_ck_buf, p_ptr, p_gpe->dst_w, p_gpe->dst_h, p_gpe->dst_w, 0);
      gpe_free(p_gpe->p_src2_ck_buf);
      p_gpe->p_src2_ck_buf = p_ptr;
    }

    // ayuv8888 to argb8888
    if(p_ctx->src2_yuv2rgb_en)
      ayuv2argb((u32 *)p_gpe->p_src2_buf, p_gpe->dst_w, p_gpe->dst_h);
    // argb8888 to ayuv8888
    if(p_ctx->src2_rgb2yuv_en)
      argb2ayuv((u32 *)p_gpe->p_src2_buf, p_gpe->dst_w, p_gpe->dst_h);   

    if((g_debug.key_set == 0) && (g_debug.key_msk == 1))
    {
        p_gpe->p_src2_buf_bak = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
        memcpy(p_gpe->p_src2_buf_bak, p_gpe->p_src2_buf, p_gpe->dst_w * p_gpe->dst_h * 4);
    }
    if(p_ctx->dst_img.palt_buf != (phys_addr_t)0)
        mt_mmz_unmap((void *)p_palt_buf);
  }
  if(p_gpe->src3_in)
  {
    ////////////////////////////////////////////
    // convert src to 32bpp and big endian
    w = p_gpe->dst_w;
    if(p_ctx->ex_img.color_info.color_fmt == Y1VY0U)
    {
        w = p_gpe->src3_w_uyvy / 2;
        yuv422_flag = 1;
    }
    else
        yuv422_flag = 0;
    p_ptr = (u8 *)gpe_malloc(w * p_gpe->dst_h * 4);
    GPE_SPN_ASSERT(p_ptr != NULL);

    pixel_expand(p_gpe->p_src3_buf,
               (u32 *)p_ptr,
           w,
           p_gpe->dst_h,
           p_ctx->ex_img.color_info.bpp,
           p_ctx->ex_img.color_info.little_endian,
           yuv422_flag,
           p_ctx->ex_img.negative_stride);
    gpe_free(p_gpe->p_src3_buf);
    p_gpe->p_src3_buf = p_ptr;
    p_ptr = NULL;

    ////////////////////////////////////////////
    // gray_8/rgb->argb8888, lut->argb8888/ayuv8888
    p_buf_ck = (u8 *)gpe_malloc(w * p_gpe->dst_h);
    GPE_SPN_ASSERT(p_buf_ck != NULL);
    if(p_ctx->ex_img.ck_select == MASK_KEY_MATCH)
        memset(p_buf_ck, 0, w * p_gpe->dst_h);
    else
        memset(p_buf_ck, 1, w * p_gpe->dst_h);    

    if(p_ctx->ex_img.with_palette)
    {
        generate_lut_ck_mask((u32 *)p_gpe->p_src3_buf,
            w,
            p_gpe->dst_h,
            p_buf_ck,
            p_ctx->ex_img.ck_min,
            p_ctx->ex_img.ck_max,
            p_ctx->ex_img.ck_mod,
            p_ctx->ex_img.color_info.color_fmt);    
    }
    if(p_ctx->ex_img.palt_buf != (phys_addr_t)0)
        palt_buf = mt_mmz_map(p_ctx->ex_img.palt_buf, 0);
    else 
        palt_buf = NULL;
    color_expand((u32 *)p_gpe->p_src3_buf,
           w,
           p_gpe->dst_h,
           p_ctx->ex_img.color_info.color_fmt,
           palt_buf,
           p_ctx->ex_img.palt_format,
           p_ctx->ex_img.palt_little_endian,
           p_ctx->ex_img.color_info.alpha_ch_en,
           p_ctx->color_exp_mode);
    
     if(p_ctx->ex_img.palt_buf != (phys_addr_t)0)
        mt_mmz_unmap((void *)palt_buf);
    if(p_ctx->ex_img.with_palette == FALSE)
    {
        generate_argb_ck_mask((u32 *)p_gpe->p_src3_buf,
                w,
                p_gpe->dst_h,
                p_buf_ck,
                p_ctx->ex_img.ck_min,
                p_ctx->ex_img.ck_max,
                p_ctx->ex_img.ck_mod,
                yuv422_flag); 
    }
    p_gpe->p_src3_ck_buf = p_buf_ck;
    
    ////////////////////////////////////////////
    // yuv422->ayuv8888
    if(p_ctx->ex_img.color_info.color_fmt == Y1VY0U)
    {
      p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
      GPE_SPN_ASSERT(p_ptr != NULL);

      uyvy2ayuv((u32 *)p_gpe->p_src3_buf, (u32 *)p_ptr, p_gpe->dst_w, p_gpe->dst_h, p_gpe->src3_w_uyvy, p_ctx->ex_img.rect.x % 2);
      gpe_free(p_gpe->p_src3_buf);
      p_gpe->p_src3_buf = p_ptr;

      p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h);
      GPE_SPN_ASSERT(p_ptr != NULL);

      ck_buf_422_to_444(p_gpe->p_src3_ck_buf, p_ptr, p_gpe->dst_w, p_gpe->dst_h, p_gpe->dst_w, 0);
      gpe_free(p_gpe->p_src3_ck_buf);
      p_gpe->p_src3_ck_buf = p_ptr;
    }

    ////////////////////////////////////////////
    //ayuv8888 to argb8888
    if(p_ctx->src3_yuv2rgb_en)
      ayuv2argb((u32 *)p_gpe->p_src3_buf, p_gpe->dst_w, p_gpe->dst_h);
    // argb8888 to ayuv8888
    if(p_ctx->src3_rgb2yuv_en)
      argb2ayuv((u32 *)p_gpe->p_src3_buf, p_gpe->dst_w, p_gpe->dst_h);
  }
}

static void rotator(u32 *p_src, u32 *p_dst, u32 width, u32 height, u32 rotator_mod, MT_BOOL *trans)
{
  u32 i = 0, j = 0;

  *trans = FALSE;
  switch(rotator_mod)
  {
  case GPE_ARIA_TRANS:
    *trans = TRUE;
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[j * height + i];
    break;
  case GPE_ARIA_HORI_MIRROR:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[i * width + width - 1 - j];
    break;
  case GPE_ARIA_HORI_MIRROR_TRANS:
    *trans = TRUE;
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[j * height + height - 1 - i];
    break;
  case GPE_ARIA_VERT_MIRROR:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(height - 1 - i) * width + j];
    break;
  case GPE_ARIA_VERT_MIRROR_TRANS:
    *trans = TRUE;
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(width - 1 - j) * height + i];
    break;
  case GPE_ARIA_HORI_VERT_MIRROR:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(height - 1 - i) * width + width - 1 - j];
    break;
  case GPE_ARIA_HORI_VERT_MIRROR_TRANS:
    *trans = TRUE;
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(width - 1 - j) * height + height - 1 - i];
    break;
  default:
    printf("rotator not supported!\n");
    break;
  }
}

static void rotator_ck(u8 *p_src, u8 *p_dst, u32 width, u32 height, u32 rotator_mod)
{
  u32 i = 0, j = 0;

  switch(rotator_mod)
  {
  case GPE_ARIA_TRANS:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[j * height + i];
    break;
  case GPE_ARIA_HORI_MIRROR:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[i * width + width - 1 - j];
    break;
  case GPE_ARIA_HORI_MIRROR_TRANS:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[j * height + height - 1 - i];
    break;
  case GPE_ARIA_VERT_MIRROR:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(height - 1 - i) * width + j];
    break;
  case GPE_ARIA_VERT_MIRROR_TRANS:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(width - 1 - j) * height + i];
    break;
  case GPE_ARIA_HORI_VERT_MIRROR:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(height - 1 - i) * width + width - 1 - j];
    break;
  case GPE_ARIA_HORI_VERT_MIRROR_TRANS:
    for(i = 0; i < height; i++)
      for(j = 0; j < width; j++)
        p_dst[i * width + j] = p_src[(width - 1 - j) * height + height - 1 - i];
    break;
  default:
    printf("rotator not supported!\n");
    break;
  }
}

static void scale_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u8 *p_ptr = NULL;
  u8 *p_ck_ptr = NULL;
  if(p_ctx->scale_en)
  {
    u32 scale_w = 0;
    u32 scale_h = 0;
    if(p_ctx->rotator_en && ((p_ctx->rotator_op == GPE_ARIA_TRANS)
      || (p_ctx->rotator_op == GPE_ARIA_HORI_MIRROR_TRANS)
      || (p_ctx->rotator_op == GPE_ARIA_VERT_MIRROR_TRANS
      || (p_ctx->rotator_op == GPE_ARIA_HORI_VERT_MIRROR_TRANS))))
    {
      scale_w = p_gpe->dst_h;
      scale_h = p_gpe->dst_w;
    }
    else
    {
      scale_w = p_gpe->dst_w;
      scale_h = p_gpe->dst_h;
    }
    p_ptr = (u8 *)gpe_malloc(scale_w * scale_h * 4);
    GPE_SPN_ASSERT(p_ptr != NULL);
    p_ck_ptr = (u8 *)gpe_malloc(scale_w * scale_h);
    GPE_SPN_ASSERT(p_ck_ptr != NULL);

    p_gpe->scale3d_mask = (u8 *)gpe_malloc(scale_w * scale_h);
    memset(p_gpe->scale3d_mask, 0, scale_w * scale_h);

#ifdef SYMPHONY6_TEST
    get_scale_coeff_h(p_ctx->p_scale_tab);
    get_scale_coeff_v(p_ctx->p_scale_tab + 64);
#endif    
#ifdef FAST_2D_SCALE
   // printf("<%s> : <%d> cale_cfg.coef[5] : %d\n", __FUNCTION__, __LINE__, p_ctx->scale_cfg.coef[5] );
    if(p_ctx->scale_cfg.coef[5] == 0)
    {
    
//    printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
//    printf("p_src1_buf :<%x> src_rect<%d %d %d %d> src WH<%d %d>\n", p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w ,p_gpe->src_h,
//         p_ctx->src_img.width, p_ctx->src_img.height);
//    printf("ptr <%x> scaleWH<%d %d> scale_cfg<%x> p_src1_ck_buf : %x width :<%d> <%x  %x %x>\n", p_ptr, scale_w, scale_h,  &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ctx->src_img.width,
//        p_ck_ptr, p_gpe->scale3d_mask,  g_debug.zero_edge);
      scale_fast(p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w, p_gpe->src_h, 
        p_ctx->src_img.width, p_ctx->src_img.height, p_ptr, scale_w, scale_h,
         &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ck_ptr,p_ctx->src_img.width,g_debug.zero_edge);    
    }
    else
#endif
    {
        if(p_ctx->scale_cfg.scale_mod == SCALE_HORI_BLK_OUT)
        {
/*        printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
        printf("p_src1_buf :<%x> src_rect<%d %d %d %d> src WH<%d %d>\n", p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w ,p_gpe->src_h,
             p_ctx->src_img.width, p_ctx->src_img.height);
        printf("ptr <%x> scaleWH<%d %d> scale_cfg<%x> p_src1_ck_buf : %x width :<%d> <%x  %x %x>\n", p_ptr, scale_w, scale_h,  &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ctx->src_img.width,
            p_ck_ptr, p_gpe->scale3d_mask,  g_debug.zero_edge);*/
            scale_hb(p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w, p_gpe->src_h, 
                     p_ctx->src_img.width, p_ctx->src_img.height, p_ptr, scale_w, scale_h,
                     &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ctx->src_img.width, p_ck_ptr, p_gpe->scale3d_mask, g_debug.zero_edge);        
         }
        else if(p_ctx->scale_cfg.scale_mod == SCALE_HORI_LINE_OUT)
        {  
     /*   printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
        
        printf("p_src1_buf :<%x> src_rect<%d %d %d %d> src WH<%d %d>\n", p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w ,p_gpe->src_h,
             p_ctx->src_img.width, p_ctx->src_img.height);
        printf("ptr <%x> scaleWH<%d %d> scale_cfg<%x> p_src1_ck_buf : %x width :<%d> <%x  %x %x>\n", p_ptr, scale_w, scale_h,  &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ctx->src_img.width,
            p_ck_ptr, p_gpe->scale3d_mask,  g_debug.zero_edge);*/

            GPE_SPN_ASSERT(p_gpe->scale3d_mask != NULL);
            scale_hl(p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w, p_gpe->src_h,
                     p_ctx->src_img.width, p_ctx->src_img.height, p_ptr, scale_w, scale_h,
                     &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ctx->src_img.width, p_ck_ptr, p_gpe->scale3d_mask, g_debug.zero_edge);  
        }
        else
        {
      /*  printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
        
        printf("p_src1_buf :<%x> src_rect<%d %d %d %d> src WH<%d %d>\n", p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w ,p_gpe->src_h,
             p_ctx->src_img.width, p_ctx->src_img.height);
        printf("ptr <%x> scaleWH<%d %d> scale_cfg<%x> p_src1_ck_buf : %x width :<%d> <%x  %x %x>\n", p_ptr, scale_w, scale_h,  &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ctx->src_img.width,
            p_ck_ptr, p_gpe->scale3d_mask,  g_debug.zero_edge);*/

            scale_vb(p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_gpe->src_w, p_gpe->src_h,
                     p_ctx->src_img.width, p_ctx->src_img.height, p_ptr, scale_w, scale_h,
                     &(p_ctx->scale_cfg), p_gpe->p_src1_ck_buf, p_ctx->src_img.width, p_ck_ptr, p_gpe->scale3d_mask, g_debug.zero_edge);  
        }
    }
    gpe_free(p_gpe->p_src1_buf);
    p_gpe->p_src1_buf = p_ptr;

    gpe_free(p_gpe->p_src1_ck_buf);
    p_gpe->p_src1_ck_buf = p_ck_ptr;
    
   }
  else
    {
        printf("<%s> : <%d>  scale_en : %d\n", __FUNCTION__, __LINE__, p_ctx->scale_en);
  }
}

static void rotate_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  MT_BOOL trans = FALSE;
  u32 tmp = 0;
  u8 *p_ptr = NULL;
  u8 *p_buf = NULL;
  u32 rotator_op = p_ctx->rotator_op;

  if(p_ctx->rotator_en && (g_debug.rotator_op != GPE_ARIA_NO_OP))
    rotator_op = g_debug.rotator_op;
  if(p_ctx->rotator_en && (rotator_op != GPE_ARIA_NO_OP))
  {
    p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
    rotator((u32 *)p_gpe->p_src1_buf,
            (u32 *)p_ptr,
            p_gpe->dst_w,
            p_gpe->dst_h,
            rotator_op,
            &trans);
    printf("rotate_stage, free\n");
    gpe_free(p_gpe->p_src1_buf);
    p_gpe->p_src1_buf = p_ptr;

    if(p_ctx->src_img.ck_en || p_ctx->src_with_mask)
    {
      p_buf = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h);
      rotator_ck(p_gpe->p_src1_ck_buf,
                 p_buf,
                 p_gpe->dst_w,
                 p_gpe->dst_h,
                 rotator_op);
      gpe_free(p_gpe->p_src1_ck_buf);
      p_gpe->p_src1_ck_buf = p_buf;
    }

    if(p_gpe->scale3d_mask != NULL)
    {
      p_buf = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h);
      rotator_ck(p_gpe->scale3d_mask,
                 p_buf,
                 p_gpe->dst_w,
                 p_gpe->dst_h,
                 rotator_op);
      gpe_free(p_gpe->scale3d_mask);
      p_gpe->scale3d_mask = p_buf;        
    }
    
    if(trans)
    {
      tmp = p_gpe->src_w;
      p_gpe->src_w = p_gpe->src_h;
      p_gpe->src_h = tmp;
    }
  }
}

static void gaussian_blur_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u8 *p_ptr = NULL;
  if(p_ctx->gaussian_blur_en)
  {
    p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
    GPE_SPN_ASSERT(p_ptr != NULL);
    
    scale_blur(p_gpe->p_src1_buf, p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, 
        p_gpe->src_w, p_gpe->src_h, p_ctx->src_img.width, p_ctx->src_img.height,	
	 p_ptr, p_gpe->dst_w,	p_gpe->dst_h, p_ctx->blur_cfg.blur_tap);

    gpe_free(p_gpe->p_src1_buf);
    p_gpe->p_src1_buf = p_ptr;    
   }
}

static int my_floor2(int x, int n)
{
  return ((x >> n) << n);
}
#if 0
static void generate_liner_gradient(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u32 i = 0, j = 0;
  u32 w = p_ctx->dst_img.rect.w;
  u32 h = p_ctx->dst_img.rect.h;
  double gradt_start = 0.0;
  double step_x = 0.0;
  double step_y = 0.0;
  u16 *p_ptr = NULL;
  double gradt = 0.0;
  double gradt_tmp = 0.0;

  gradt_start = (double)p_ctx->paint.gradt_start / (double)(1 << 20);
  step_x = (double)p_ctx->paint.step_x / (double)(1 << 21);
  step_y = (double)p_ctx->paint.step_y / (double)(1 << 21);

  p_gpe->p_gradt_buf = (u16 *)malloc(w * h * 2);
  printf("p_gpe->p_gradt_buf=0x%0x\n", p_gpe->p_gradt_buf);
  if(p_gpe->p_gradt_buf  ==  NULL)
  {
    printf("\n\rgradt_buf malloc failed!");
    GPE_SPN_ASSERT(0);
  }
  p_ptr = p_gpe->p_gradt_buf;

  for(i = 0; i < h; i++)
  {
    gradt_tmp = gradt_start;
    for(j = 0; j < w; j++)
    {
      switch(p_ctx->paint.spread_mod)
      {
      case GPE_SPN_SPREAD_PAD:
        if(gradt_tmp > 1.0)
          gradt = 1.0;
        else if(gradt_tmp < 0.0)
          gradt = 0.0;
        else
          gradt = gradt_tmp;
        break;
      case GPE_SPN_SPREAD_REPEAT:
        if((gradt_tmp > 1.0) || (gradt_tmp < 0.0))
          gradt = gradt_tmp - my_floor(gradt_tmp);
        else
          gradt = gradt_tmp;
        break;
      case GPE_SPN_SPREAD_REFLECT:
        if((gradt_tmp > 1.0) || (gradt_tmp < 0.0))
        {
          if((my_floor(gradt_tmp) & 0x1)  ==  0)
            gradt = gradt_tmp - my_floor(gradt_tmp);
          else
            gradt = 1.0 - (gradt_tmp - my_floor(gradt_tmp));
        }
        else
          gradt = gradt_tmp;
        break;
      }
      if(gradt == 1.0)
        p_ptr[i * w + j] = (1 << 16) - 1;
      else
        p_ptr[i * w + j] = (u16)(gradt * (1 << 16));
      gradt_tmp  +=  step_x;
    }
    gradt_start  +=  step_y;
  }
}
#endif

#if 1
static void generate_liner_gradient(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u32 i = 0, j = 0;
  u32 w = p_ctx->dst_img.rect.w;
  u32 h = p_ctx->dst_img.rect.h;
  int gradt_start = 0;
  int step_x = 0;
  int step_y = 0;
  u16 *p_ptr = NULL;
  int gradt = 0;
  long long  gradt_tmp = 0;
  long long  gradt_start0 = 0;

  gradt_start = p_ctx->paint.gradt_start;
  step_x = p_ctx->paint.step_x;
  step_y = p_ctx->paint.step_y;
    
  p_gpe->p_gradt_buf = (u16 *)gpe_malloc(w * h * 2);

  if(p_gpe->p_gradt_buf == NULL)
  {
    printf("\n\rgradt_buf malloc failed!");
    GPE_SPN_ASSERT(0);
  }
  p_ptr = p_gpe->p_gradt_buf;

  gradt_start0 = (long long) gradt_start << 1;

  for(i = 0; i < h; i++)
  {
    gradt_tmp = gradt_start0;

    for(j = 0; j < w; j++)
    {
      switch(p_ctx->paint.spread_mod)
      {
      case GPE_ARIA_SPREAD_PAD:
        if(gradt_tmp > 0x1fffff)
          gradt = 0x1fffff;
        else if(gradt_tmp < 0)
          gradt = 0;
        else
          gradt = gradt_tmp;
        break;
      case GPE_ARIA_SPREAD_REPEAT:
        if((gradt_tmp > 0x1fffff) || (gradt_tmp < 0))
          gradt = gradt_tmp - my_floor2(gradt_tmp, 21);
        else
          gradt = gradt_tmp;
        break;
      case GPE_ARIA_SPREAD_REFLECT:
        if((gradt_tmp > 0x1fffff) || (gradt_tmp < 0))
        {
          if(((my_floor2(gradt_tmp, 21) >> 21) & 0x1) == 0)
            gradt = gradt_tmp - my_floor2(gradt_tmp, 21);
          else
            gradt = 0x1fffff - (gradt_tmp - my_floor2(gradt_tmp, 21));
        }
        else
          gradt = gradt_tmp;
        break;
      default:
        break;
      }

      p_ptr[i * w + j] = (u16)(gradt >> 9);
      gradt_tmp += step_x;
    }
    gradt_start0 += step_y;
  }
  printf("step_x:0x%0x\n",step_x);
}
#endif
static void integrateColorRamp(aria_gpe_context_t *p_ctx, u32 *dst, u16 *p_gradt_buf, u32 pitch)
{
  u32 i = 0, j = 0;
  u32 w = p_ctx->dst_img.rect.w;
  u32 h = p_ctx->dst_img.rect.h;
  int a0 = 0, r0 = 0, g0 = 0, b0 = 0;
  int a1 = 0, r1 = 0, g1 = 0, b1 = 0;
  u8 a = 0, r = 0, g = 0, b = 0;
  u32 offset0 = p_ctx->paint.stop0.offset;
  u32 offset1 = p_ctx->paint.stop1.offset;
  u32 offset2 = p_ctx->paint.stop2.offset;
  u16 gradt = 0;
  u32 stop_fact = 0;
  int tmp = 0;
  u32 argb0 = 0, argb1 = 0;

  for(i = 0; i < h; i++)
  {
    for(j = 0; j < w; j++)
    {
      if(p_ctx->paint.spread_mod == GPE_ARIA_FLAT_COLOR_FILL)
      {
        dst[i * w + j] = p_ctx->paint.paint_color;
        continue;
      }
      //gradt = (p_gradt_buf[i * w + j] >> 4);
      gradt = p_gradt_buf[i * pitch / 2 + j] & 0xfff;
      if(gradt <= offset1)
      {
        stop_fact = p_ctx->paint.stop_fact0;
        tmp = ((gradt - offset0) * stop_fact) >> 12;
        argb0 = p_ctx->paint.stop0.argb;
        argb1 = p_ctx->paint.stop1.argb;
      }
      else if(gradt <= offset2)
      {
        stop_fact = p_ctx->paint.stop_fact1;
        tmp = ((gradt - offset1) * stop_fact) >> 12;
        argb0 = p_ctx->paint.stop1.argb;
        argb1 = p_ctx->paint.stop2.argb;
      }
      else
      {
        stop_fact = p_ctx->paint.stop_fact2;
        tmp = ((gradt - offset2) * stop_fact) >> 12;
        argb0 = p_ctx->paint.stop2.argb;
        argb1 = p_ctx->paint.stop3.argb;
      }
      a0 = (argb0 >> 24) & 0xff;
      r0 = (argb0 >> 16) & 0xff;
      g0 = (argb0 >> 8) & 0xff;
      b0 = argb0 & 0xff;
      a1 = (argb1 >> 24) & 0xff;
      r1 = (argb1 >> 16) & 0xff;
      g1 = (argb1 >> 8) & 0xff;
      b1 = argb1 & 0xff;

      if(a1 > a0)
        a = CLIP(a0 + ((((a1 - a0) * tmp) + (1 << 11)) >> 12));
      else
        a = CLIP(a0 - ((((a0 - a1) * tmp) + (1 << 11)) >> 12));
      if(r1 > r0)
        r = CLIP(r0 + ((((r1 - r0) * tmp) + (1 << 11)) >> 12));
      else
        r = CLIP(r0 - ((((r0 - r1) * tmp) + (1 << 11)) >> 12));
      if(g1 > g0)
        g = CLIP(g0 + ((((g1 - g0) * tmp) + (1 << 11)) >> 12));
      else
        g = CLIP(g0 - ((((g0 - g1) * tmp) + (1 << 11)) >> 12));
      if(b1 > b0)
        b = CLIP(b0 + ((((b1 - b0) * tmp) + (1 << 11)) >> 12));
      else
        b = CLIP(b0 - ((((b0 - b1) * tmp) + (1 << 11)) >> 12));

      dst[i * w + j] = (a << 24) | (r << 16) | (g << 8) | b;
    }
  }
}
static int  RI_INT_MAX(int a, int b)
{
  return (a > b) ? a : b;
}
static int  RI_INT_MIN(int a, int b)
{
  return (a < b) ? a : b;
}
static int  RI_INT_MOD(int a, int b)
{
  int i = a % b;
  if(i < 0) i += b;
  return i;
}
static void pattern_paint(u32 *p_src, 
                  u32 *p_dst, 
                  u32 sw, 
                  u32 sh, 
                  u32 dw, 
                  u32 dh, 
                  int mod, 
                  u32 paint_color,
                  u32 x0,
                  u32 y0)
{
  u32 i = 0, j = 0;
  u32 u = 0, v = 0;
//	u32 dtmp;

  for(i = 0; i < dh; i++)
  {
    for(j = 0; j < dw; j++)
    {
      u = i - y0;
      v = j - x0;
      switch(mod)
      {
      case GPE_ARIA_TILE_FILL:
        if(u < 0 || v < 0 || u >= sh || v >= sw)
          p_dst[i * dw + j] = paint_color;
        else
          p_dst[i * dw + j] = p_src[u * sw + v];
        break;
      case GPE_ARIA_TILE_PAD:
        u = RI_INT_MIN(RI_INT_MAX(u, 0), sh - 1);
        v = RI_INT_MIN(RI_INT_MAX(v, 0), sw - 1);
        p_dst[i * dw + j] = p_src[u * sw + v];
        break;
      case GPE_ARIA_TILE_REPEAT:

	//			mtos_critical_enter(&dtmp);

        u = RI_INT_MOD(u, sh);
        v = RI_INT_MOD(v, sw);
	//			mtos_critical_exit(dtmp);
#if 0
				if(u > sh || v>sw)
					printf("\r\n u:%d, v:%d, i:%d, j:%d, sh:%d, sw:%d", u, v, i, j, sh, sw);
#endif			
        p_dst[i * dw + j] = p_src[u * sw + v];
        break;
      case GPE_ARIA_TILE_REFLECT:
	//			mtos_critical_enter(&dtmp);
        u = RI_INT_MOD(u, sh * 2);
        v = RI_INT_MOD(v, sw * 2);
	//			mtos_critical_exit(dtmp);
        if(u >= sh) 
          u = sh * 2 - 1 - u;
        if(v >= sw)
          v = sw * 2 - 1 - v;
        p_dst[i * dw + j] = p_src[u * sw + v];
        break;
      default:
        break;
      }
    }
  }
}

static void paint_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u8 *p_ptr = NULL;
  u16* p_gradt_buf = NULL;
  if(p_ctx->paint_en)
  {
    if(p_ctx->paint.is_pattern_paint)
    {
      p_ptr = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
      pattern_paint((u32 *)p_gpe->p_src1_buf,
                    (u32 *)p_ptr,
                    p_gpe->src_w,
                    p_gpe->src_h,
                    p_gpe->dst_w,
                    p_gpe->dst_h,
                    p_ctx->paint.tiling_mod,
                    p_ctx->paint.paint_color,
                    p_ctx->paint.pat_beg.x,
                    p_ctx->paint.pat_beg.y);
      gpe_free(p_gpe->p_src1_buf);
      p_gpe->p_src1_buf = p_ptr;
    }
    else
    {
      if((p_ctx->paint.gradt_type == GPE_ARIA_LINER_GRADT) && p_ctx->paint.true_liner_gradt)
      {
        generate_liner_gradient(p_ctx, p_gpe);
        integrateColorRamp(p_ctx, (u32 *)p_gpe->p_src1_buf, p_gpe->p_gradt_buf, p_ctx->dst_img.rect.w * 2);
      }
      else
      {
        p_gradt_buf = (u16 *)mt_mmz_map(p_ctx->p_gradt_buf, 0);
        integrateColorRamp(p_ctx, (u32 *)p_gpe->p_src1_buf, p_gradt_buf, p_ctx->src_img.pitch);
        mt_mmz_unmap((void *)p_gradt_buf);
      }
    }
  }
}

static void generate_xylc(u32 *p_src, 
                   u32 *p_dst, 
                   u8 *p_ck_buf, 
                   u32 mod, 
                   u32 count, 
                   u32 color, 
                   u32 w, 
                   u32 h)
{
  u32 i = 0, j = 0;
  u32 x = 0, y = 0, len = 0, c = 0;

  memset(p_dst, 0, w * h * 4);
  memset(p_ck_buf, 1, w * h);
  for(i = 0; i < count * 4; i += 4)
  {
     x = p_src[i];
     y = p_src[i + 1];
     len = p_src[i + 2];
     c = p_src[i + 3];
     if(mod == XY)
     {
       len = 1;
       c = color;
     }
     else if(mod == XYL)
     {
       c = color;
     }
     else if(mod == XYC)
     {
       len = 1;
     }
     for(j = x; j < x + len; j++)
     {
       if(y * w + j >= w * h)
        return;
       p_dst[y * w + j] = c;
       p_ck_buf[y * w + j] = 0;
     }
  }
}

static void xylc_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u32 *p_ptr = NULL;
  u32 *p_ptr2 = NULL;
  
  if(p_ctx->src_is_xylc)
  {
    u8 *p_buf_ck = NULL;
    u8 *p_buf_ck2 = NULL;
    int i = 0;
    int j = 0;
    p_ptr = (u32 *)gpe_malloc(p_ctx->src_img.width * p_ctx->src_img.height * 4);
    p_buf_ck = (u8 *)gpe_malloc(p_ctx->src_img.width * p_ctx->src_img.height);
    generate_xylc((u32 *)p_gpe->p_src1_buf,
                  p_ptr,
                  p_buf_ck,
                  p_ctx->src_img.color_info.color_fmt,
                  p_ctx->xylc_cfg.xylc_num,
                  p_ctx->xylc_cfg.xylc_color,
                  p_ctx->src_img.width,
                  p_ctx->src_img.height);

    gpe_free(p_gpe->p_src1_buf);
    gpe_free(p_gpe->p_src1_ck_buf);
    p_ptr2 = (u32 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h * 4);
    p_buf_ck2 = (u8 *)gpe_malloc(p_gpe->dst_w * p_gpe->dst_h);

    for(i = 0; i < p_gpe->dst_h; i++)
    {
        for(j = 0; j < p_gpe->dst_w; j++)
        {
            p_ptr2[i * p_gpe->dst_w + j] = p_ptr[(i + p_ctx->src_img.rect.y) * p_ctx->src_img.width + j + p_ctx->src_img.rect.x];
            p_buf_ck2[i * p_gpe->dst_w + j] = p_buf_ck[(i + p_ctx->src_img.rect.y) * p_ctx->src_img.width + j + p_ctx->src_img.rect.x];
        }
    }

    p_gpe->p_src1_buf = (u8*)p_ptr2;
    p_gpe->p_src1_ck_buf = p_buf_ck2;
    gpe_free(p_ptr);
    gpe_free(p_buf_ck);
    p_ctx->src_img.ck_en = 1;
  }
}

#define ROUND(x,n)  ((x) + (1 << ((n)-1))) >> (n)
#define N 2

static u16 round_mult(u16 a, u16 b)
{
  u16 r = 0;
  r = ROUND(a * b, 8 + N);
  return (u16)r;
}

typedef struct
{
  u16 *p_a;
  u16 *p_r;
  u16 *p_g;
  u16 *p_b;
}argb_t;

typedef struct
{
  u16 *p_aa;
  u16 *p_ar; //alpha for red component
  u16 *p_ag;
  u16 *p_ab;
}alpha_t;

static void alpha_premult(argb_t *p_dst,
           u32 *p_src,
           u32 w,
           u32 h,
           MT_BOOL plane_alpha_en,
           u32 plane_alpha,
           MT_BOOL premult_en)
{
  u32 i = 0, j = 0;
  u32 alpha = 0;
  u32 tmp = 0;

  if(plane_alpha_en)
    alpha = plane_alpha;
  else
    alpha = 256;

  //a*plane_alpha
  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      p_dst->p_a[i * w + j] = ((((p_src[i * w + j] >> 24) & 0xff) << N) * alpha + 128) >> 8;
    }
  //[argb].*[1aaa]
  if(premult_en)
  {
    for(i = 0; i < h; i++)
      for(j = 0; j < w; j++)
      {
        tmp = p_src[i * w + j];
        alpha = p_dst->p_a[i * w + j];
        p_dst->p_r[i * w + j] = round_mult(((tmp >> 16) & 0xff) << N, alpha);
        p_dst->p_g[i * w + j] = round_mult(((tmp >> 8) & 0xff) << N, alpha);
        p_dst->p_b[i * w + j] = round_mult((tmp & 0xff) << N, alpha);
      }
  }
  else
  {
    for(i = 0; i < h; i++)
      for(j = 0; j < w; j++)
      {
        tmp = p_src[i * w + j];
        
        if(plane_alpha_en)
          alpha = plane_alpha;
        else
          alpha = 256;
        p_dst->p_r[i * w + j] = ((((tmp >> 16) & 0xff) << N) * alpha + 128) >> 8;
        p_dst->p_g[i * w + j] = ((((tmp >> 8) & 0xff) << N) * alpha + 128) >> 8;
        p_dst->p_b[i * w + j] = (((tmp & 0xff) << N) * alpha + 128) >> 8;
        
      }
  }
}

static void draw_img_multiply(argb_t *p_src1, argb_t *p_src3, u32 w, u32 h)
{
  u32 i = 0, j = 0;
  //[argb]src1.*[argb]src3
  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      p_src1->p_a[i * w + j] = round_mult(p_src1->p_a[i * w + j], p_src3->p_a[i * w + j]);
      p_src1->p_r[i * w + j] = round_mult(p_src1->p_r[i * w + j], p_src3->p_r[i * w + j]);
      p_src1->p_g[i * w + j] = round_mult(p_src1->p_g[i * w + j], p_src3->p_g[i * w + j]);
      p_src1->p_b[i * w + j] = round_mult(p_src1->p_b[i * w + j], p_src3->p_b[i * w + j]);
    }
}

static void draw_img_stencil(argb_t *p_src1, argb_t *p_src3, alpha_t *p_src1_a, u32 w, u32 h)
{
  u32 i = 0, j = 0;
  //[aaaa]src1.*[argb]src3
  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      p_src1_a->p_aa[i * w + j] = round_mult(p_src1->p_a[i * w + j], p_src3->p_a[i * w + j]);
      p_src1_a->p_ar[i * w + j] = round_mult(p_src1->p_a[i * w + j], p_src3->p_r[i * w + j]);
      p_src1_a->p_ag[i * w + j] = round_mult(p_src1->p_a[i * w + j], p_src3->p_g[i * w + j]);
      p_src1_a->p_ab[i * w + j] = round_mult(p_src1->p_a[i * w + j], p_src3->p_b[i * w + j]);
    }
}

static void clip_mask_mix(argb_t *p_src1, argb_t *p_src3, u32 w, u32 h, u32 mix_mod)
{
  u32 i = 0, j = 0;
  //mix_mod0:[argb]src1.*[aaaa]src3
  //mix_mod1:[argb]src1.*[a111]src3
  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      p_src1->p_a[i * w + j] = round_mult(p_src1->p_a[i * w + j], p_src3->p_a[i * w + j]);
      if(mix_mod == 0)
      {
        p_src1->p_r[i * w + j] = round_mult(p_src1->p_r[i * w + j], p_src3->p_a[i * w + j]);
        p_src1->p_g[i * w + j] = round_mult(p_src1->p_g[i * w + j], p_src3->p_a[i * w + j]);
        p_src1->p_b[i * w + j] = round_mult(p_src1->p_b[i * w + j], p_src3->p_a[i * w + j]);
      }
    }
}

static void colorize(argb_t *p_src1, u32 argb, u32 w, u32 h)
{
  u32 i = 0, j = 0;
  argb_t color;
  color.p_r = ((argb >> 16) & 0xff) << N;
  color.p_g = ((argb >> 8) & 0xff) << N;
  color.p_b = (argb & 0xff) << N;
        
  //[rgb]src1.*[rgb]color
  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      p_src1->p_r[i * w + j] = round_mult(p_src1->p_r[i * w + j], color.p_r);
      p_src1->p_g[i * w + j] = round_mult(p_src1->p_g[i * w + j], color.p_g);
      p_src1->p_b[i * w + j] = round_mult(p_src1->p_b[i * w + j], color.p_b);
    }
}


typedef struct
{
  u16 fa;
  u16 fr;
  u16 fg;
  u16 fb;
}fact_t;

#define GPE_ONE ((1 << (8 + N)) -1)

#if 0
static void blend(argb_t *p_src1, 
            argb_t *p_src2, 
            alpha_t *p_src1_a, 
            u32 w, 
            u32 h, 
            u32 sf_mod, 
            u32 df_mod)
{
  u32 i = 0, j = 0, k = 0;
  fact_t sf = {0}, df = {0};
  u16 tmp1 = 0, tmp2 = 0;
  u32 mod = 0;
  fact_t *p_pf = NULL;

  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      for(k = 0; k < 2; k++)
      {
        if(k == 0)
        {
          mod = sf_mod;
          p_pf = &sf;
        }
        else
        {
          mod = df_mod;
          p_pf = &df;
        }
        switch(mod)
        {
        case GPE_SPN_GL_ZERO:
          p_pf->fa = 0;
          p_pf->fr = 0;
          p_pf->fg = 0;
          p_pf->fb = 0;
          break;
        case GPE_SPN_GL_ONE:
          p_pf->fa = GPE_ONE;
          p_pf->fr = GPE_ONE;
          p_pf->fg = GPE_ONE;
          p_pf->fb = GPE_ONE;
          break;
        case GPE_SPN_GL_DST_COLOR:
          p_pf->fa = p_src2->p_a[i * w + j];
          p_pf->fr = p_src2->p_r[i * w + j];
          p_pf->fg = p_src2->p_g[i * w + j];
          p_pf->fb = p_src2->p_b[i * w + j];
          break;
        case GPE_SPN_GL_SRC_COLOR:
          p_pf->fa = p_src1->p_a[i * w + j];
          p_pf->fr = p_src1->p_r[i * w + j];
          p_pf->fg = p_src1->p_g[i * w + j];
          p_pf->fb = p_src1->p_b[i * w + j];
          break;
        case GPE_SPN_GL_ONE_MINUS_DST_COLOR:
          p_pf->fa = GPE_ONE - p_src2->p_a[i * w + j];
          p_pf->fr = GPE_ONE - p_src2->p_r[i * w + j];
          p_pf->fg = GPE_ONE - p_src2->p_g[i * w + j];
          p_pf->fb = GPE_ONE - p_src2->p_b[i * w + j];
          break;
        case GPE_SPN_GL_ONE_MINUS_SRC_COLOR:
          p_pf->fa = GPE_ONE - p_src1->p_a[i * w + j];
          p_pf->fr = GPE_ONE - p_src1->p_r[i * w + j];
          p_pf->fg = GPE_ONE - p_src1->p_g[i * w + j];
          p_pf->fb = GPE_ONE - p_src1->p_b[i * w + j];
          break;
        case GPE_SPN_GL_SRC_ALPHA:
          p_pf->fa = p_src1->p_a[i * w + j];
          p_pf->fr = p_src1->p_a[i * w + j];
          p_pf->fg = p_src1->p_a[i * w + j];
          p_pf->fb = p_src1->p_a[i * w + j];
          break;
        case GPE_SPN_GL_ONE_MINUS_SRC_ALPHA:
          p_pf->fa = GPE_ONE - p_src1->p_a[i * w + j];
          p_pf->fr = GPE_ONE - p_src1->p_a[i * w + j];
          p_pf->fg = GPE_ONE - p_src1->p_a[i * w + j];
          p_pf->fb = GPE_ONE - p_src1->p_a[i * w + j];
          break;
        case GPE_SPN_GL_DST_ALPHA:
          p_pf->fa = p_src2->p_a[i * w + j];
          p_pf->fr = p_src2->p_a[i * w + j];
          p_pf->fg = p_src2->p_a[i * w + j];
          p_pf->fb = p_src2->p_a[i * w + j];
          break;
        case GPE_SPN_GL_ONE_MINUS_DST_ALPHA:
          p_pf->fa = GPE_ONE - p_src2->p_a[i * w + j];
          p_pf->fr = GPE_ONE - p_src2->p_a[i * w + j];
          p_pf->fg = GPE_ONE - p_src2->p_a[i * w + j];
          p_pf->fb = GPE_ONE - p_src2->p_a[i * w + j];
          break;
        case GPE_SPN_GL_SRC_ALPHA_SATURATE:
          p_pf->fa = GPE_ONE;
          tmp1 = p_src1->p_a[i * w + j];
          tmp2 = GPE_ONE - p_src2->p_a[i * w + j];
          tmp1 = (tmp1 < tmp2) ? tmp1 : tmp2;
          p_pf->fr = tmp1;
          p_pf->fg = tmp1;
          p_pf->fb = tmp1;
          break;
        case GPE_SPN_ST_ALPHA:
          p_pf->fa = p_src1_a->p_aa[i * w + j];
          p_pf->fr = p_src1_a->p_ar[i * w + j];
          p_pf->fg = p_src1_a->p_ag[i * w + j];
          p_pf->fb = p_src1_a->p_ab[i * w + j];
          break;
        case GPE_SPN_ST_ONE_MINUS_ALPHA:
          p_pf->fa = GPE_ONE - p_src1_a->p_aa[i * w + j];
          p_pf->fr = GPE_ONE - p_src1_a->p_ar[i * w + j];
          p_pf->fg = GPE_ONE - p_src1_a->p_ag[i * w + j];
          p_pf->fb = GPE_ONE - p_src1_a->p_ab[i * w + j];
          break;
        case GPE_SPN_ST_COLOR:
          p_pf->fa = p_src1_a->p_aa[i * w + j];
          p_pf->fr = p_src1->p_r[i * w + j];
          p_pf->fg = p_src1->p_g[i * w + j];
          p_pf->fb = p_src1->p_b[i * w + j];
          break;
        case GPE_SPN_ST_ONE_MINUS_COLOR:
          p_pf->fa = GPE_ONE - p_src1_a->p_aa[i * w + j];
          p_pf->fr = GPE_ONE - p_src1->p_r[i * w + j];
          p_pf->fg = GPE_ONE - p_src1->p_g[i * w + j];
          p_pf->fb = GPE_ONE - p_src1->p_b[i * w + j];
          break;
        case GPE_SPN_ST_ALPHA_SATURATE:
          p_pf->fa = GPE_ONE;
          tmp2 = GPE_ONE - p_src2->p_a[i * w + j];
          tmp1 = p_src1_a->p_ar[i * w + j];
          p_pf->fr = (tmp1 < tmp2) ? tmp1 : tmp2;
          tmp1 = p_src1_a->p_ag[i * w + j];
          p_pf->fg = (tmp1 < tmp2) ? tmp1 : tmp2;
          tmp1 = p_src1_a->p_ab[i * w + j];
          p_pf->fb = (tmp1 < tmp2) ? tmp1 : tmp2;
          break;
        default:
          break;
        }
      }
      p_src1->p_a[i * w + j] = round_mult(p_src1->p_a[i * w + j], sf.fa);
      p_src1->p_r[i * w + j] = round_mult(p_src1->p_r[i * w + j], sf.fr);
      p_src1->p_g[i * w + j] = round_mult(p_src1->p_g[i * w + j], sf.fg);
      p_src1->p_b[i * w + j] = round_mult(p_src1->p_b[i * w + j], sf.fb);

      p_src2->p_a[i * w + j] = round_mult(p_src2->p_a[i * w + j], df.fa);
      p_src2->p_r[i * w + j] = round_mult(p_src2->p_r[i * w + j], df.fr);
      p_src2->p_g[i * w + j] = round_mult(p_src2->p_g[i * w + j], df.fg);
      p_src2->p_b[i * w + j] = round_mult(p_src2->p_b[i * w + j], df.fb);

      p_src1->p_a[i * w + j] = p_src1->p_a[i * w + j] + p_src2->p_a[i * w + j];
      p_src1->p_r[i * w + j] = p_src1->p_r[i * w + j] + p_src2->p_r[i * w + j];
      p_src1->p_g[i * w + j] = p_src1->p_g[i * w + j] + p_src2->p_g[i * w + j];
      p_src1->p_b[i * w + j] = p_src1->p_b[i * w + j] + p_src2->p_b[i * w + j];
    }
}

#else
static void blend(argb_t *p_src1, 
            argb_t *p_src2, 
            alpha_t *p_src1_a, 
            u32 w, 
            u32 h, 
            u32 sf_mod_c, 
            u32 df_mod_c,
            u32 sf_mod_a,
            u32 df_mod_a)
{
  u32 i = 0, j = 0, k = 0;
  fact_t sf = {0}, df = {0};
  u16 tmp1 = 0, tmp2 = 0;
  u32 mod = 0;
  fact_t *p_pf = NULL;

  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      for(k = 0; k < 4; k++)
      {
        if(k == 0)
        {
          mod = sf_mod_c;
          p_pf = &sf;
        }
        else if(k == 2)
        {
          mod = sf_mod_a;
          p_pf = &sf;        
        }
        else if(k == 1)
        {
          mod = df_mod_c;
          p_pf = &df;
        }
        else if(k == 3)
        {
          mod = df_mod_a;
          p_pf = &df;
        }        
        switch(mod)
        {
        case GPE_ARIA_GL_ZERO:
            if(k > 1)
              p_pf->fa = 0;
            else
                {
          p_pf->fr = 0;
          p_pf->fg = 0;
          p_pf->fb = 0;
                }
          break;
        case GPE_ARIA_GL_ONE:
            if(k > 1)
          p_pf->fa = GPE_ONE;
            else
                {
          p_pf->fr = GPE_ONE;
          p_pf->fg = GPE_ONE;
          p_pf->fb = GPE_ONE;
                }
          break;
        case GPE_ARIA_GL_DST_COLOR:
            if(k > 1)
          p_pf->fa = p_src2->p_a[i * w + j];
            else
                {
          p_pf->fr = p_src2->p_r[i * w + j];
          p_pf->fg = p_src2->p_g[i * w + j];
          p_pf->fb = p_src2->p_b[i * w + j];
                }
          break;
        case GPE_ARIA_GL_SRC_COLOR:
            if(k > 1)
          p_pf->fa = p_src1->p_a[i * w + j];
            else
                {
          p_pf->fr = p_src1->p_r[i * w + j];
          p_pf->fg = p_src1->p_g[i * w + j];
          p_pf->fb = p_src1->p_b[i * w + j];
                }
          break;
        case GPE_ARIA_GL_ONE_MINUS_DST_COLOR:
            if(k > 1)
          p_pf->fa = GPE_ONE - p_src2->p_a[i * w + j];
            else
                {
          p_pf->fr = GPE_ONE - p_src2->p_r[i * w + j];
          p_pf->fg = GPE_ONE - p_src2->p_g[i * w + j];
          p_pf->fb = GPE_ONE - p_src2->p_b[i * w + j];
                }
          break;
        case GPE_ARIA_GL_ONE_MINUS_SRC_COLOR:
            if(k > 1)
          p_pf->fa = GPE_ONE - p_src1->p_a[i * w + j];
            else
                {
          p_pf->fr = GPE_ONE - p_src1->p_r[i * w + j];
          p_pf->fg = GPE_ONE - p_src1->p_g[i * w + j];
          p_pf->fb = GPE_ONE - p_src1->p_b[i * w + j];
                }
          break;
        case GPE_ARIA_GL_SRC_ALPHA:
            if(k > 1)
          p_pf->fa = p_src1->p_a[i * w + j];
            else
                {
          p_pf->fr = p_src1->p_a[i * w + j];
          p_pf->fg = p_src1->p_a[i * w + j];
          p_pf->fb = p_src1->p_a[i * w + j];
                }
          break;
        case GPE_ARIA_GL_ONE_MINUS_SRC_ALPHA:
            if(k > 1)
          p_pf->fa = GPE_ONE - p_src1->p_a[i * w + j];
            else
                {
          p_pf->fr = GPE_ONE - p_src1->p_a[i * w + j];
          p_pf->fg = GPE_ONE - p_src1->p_a[i * w + j];
          p_pf->fb = GPE_ONE - p_src1->p_a[i * w + j];
                }
          break;
        case GPE_ARIA_GL_DST_ALPHA:
            if(k > 1)
          p_pf->fa = p_src2->p_a[i * w + j];
            else
                {
          p_pf->fr = p_src2->p_a[i * w + j];
          p_pf->fg = p_src2->p_a[i * w + j];
          p_pf->fb = p_src2->p_a[i * w + j];
                }
          break;
        case GPE_ARIA_GL_ONE_MINUS_DST_ALPHA:
            if(k > 1)
          p_pf->fa = GPE_ONE - p_src2->p_a[i * w + j];
            else
                {
          p_pf->fr = GPE_ONE - p_src2->p_a[i * w + j];
          p_pf->fg = GPE_ONE - p_src2->p_a[i * w + j];
          p_pf->fb = GPE_ONE - p_src2->p_a[i * w + j];
                }
          break;
        case GPE_ARIA_GL_SRC_ALPHA_SATURATE:
            if(k > 1)
          p_pf->fa = GPE_ONE;
            else
                {
          tmp1 = p_src1->p_a[i * w + j];
          tmp2 = GPE_ONE - p_src2->p_a[i * w + j];
          tmp1 = (tmp1 < tmp2) ? tmp1 : tmp2;
          p_pf->fr = tmp1;
          p_pf->fg = tmp1;
          p_pf->fb = tmp1;
                }
          break;
        case GPE_ARIA_ST_ALPHA:
            if(k > 1)                
          p_pf->fa = p_src1_a->p_aa[i * w + j];
            else
                {
          p_pf->fr = p_src1_a->p_ar[i * w + j];
          p_pf->fg = p_src1_a->p_ag[i * w + j];
          p_pf->fb = p_src1_a->p_ab[i * w + j];
                }
          break;
        case GPE_ARIA_ST_ONE_MINUS_ALPHA:
            if(k > 1)
          p_pf->fa = GPE_ONE - p_src1_a->p_aa[i * w + j];
            else
                {
          p_pf->fr = GPE_ONE - p_src1_a->p_ar[i * w + j];
          p_pf->fg = GPE_ONE - p_src1_a->p_ag[i * w + j];
          p_pf->fb = GPE_ONE - p_src1_a->p_ab[i * w + j];
                }
          break;
        case GPE_ARIA_ST_COLOR:
            if(k > 1)
          p_pf->fa = p_src1_a->p_aa[i * w + j];
            else
                {
          p_pf->fr = p_src1->p_r[i * w + j];
          p_pf->fg = p_src1->p_g[i * w + j];
          p_pf->fb = p_src1->p_b[i * w + j];
                }
          break;
        case GPE_ARIA_ST_ONE_MINUS_COLOR:
            if(k > 1)
          p_pf->fa = GPE_ONE - p_src1_a->p_aa[i * w + j];
            else
                {
          p_pf->fr = GPE_ONE - p_src1->p_r[i * w + j];
          p_pf->fg = GPE_ONE - p_src1->p_g[i * w + j];
          p_pf->fb = GPE_ONE - p_src1->p_b[i * w + j];
                }
          break;
        case GPE_ARIA_ST_ALPHA_SATURATE:
            if(k > 1)
          p_pf->fa = GPE_ONE;
                {
          tmp2 = GPE_ONE - p_src2->p_a[i * w + j];
          tmp1 = p_src1_a->p_ar[i * w + j];
          p_pf->fr = (tmp1 < tmp2) ? tmp1 : tmp2;
          tmp1 = p_src1_a->p_ag[i * w + j];
          p_pf->fg = (tmp1 < tmp2) ? tmp1 : tmp2;
          tmp1 = p_src1_a->p_ab[i * w + j];
          p_pf->fb = (tmp1 < tmp2) ? tmp1 : tmp2;
                }
          break;
        default:
          break;
        }
      }
      p_src1->p_a[i * w + j] = round_mult(p_src1->p_a[i * w + j], sf.fa);
      p_src1->p_r[i * w + j] = round_mult(p_src1->p_r[i * w + j], sf.fr);
      p_src1->p_g[i * w + j] = round_mult(p_src1->p_g[i * w + j], sf.fg);
      p_src1->p_b[i * w + j] = round_mult(p_src1->p_b[i * w + j], sf.fb);

      p_src2->p_a[i * w + j] = round_mult(p_src2->p_a[i * w + j], df.fa);
      p_src2->p_r[i * w + j] = round_mult(p_src2->p_r[i * w + j], df.fr);
      p_src2->p_g[i * w + j] = round_mult(p_src2->p_g[i * w + j], df.fg);
      p_src2->p_b[i * w + j] = round_mult(p_src2->p_b[i * w + j], df.fb);

      p_src1->p_a[i * w + j] = p_src1->p_a[i * w + j] + p_src2->p_a[i * w + j];
      p_src1->p_r[i * w + j] = p_src1->p_r[i * w + j] + p_src2->p_r[i * w + j];
      p_src1->p_g[i * w + j] = p_src1->p_g[i * w + j] + p_src2->p_g[i * w + j];
      p_src1->p_b[i * w + j] = p_src1->p_b[i * w + j] + p_src2->p_b[i * w + j];
    }
}
#endif

static u8 rop3(u8 s, u8 d, u8 p, u32 id)
{
  u8 r = 0;
  switch(id)
  {
  case ROP_COPYPEN:
    r = s;
    break;
  case ROP_MERGEPEN:
    r = s | d;
    break;
  case ROP_MASKPEN:
    r = s & d;
    break;
  case ROP_XORPEN:
    r = s ^ d;
    break;
  case ROP_MASKPENNOT:
    r = s & (~d);
    break;
  case ROP_NOTCOPYPEN:
    r = ~s;
    break;
  case ROP_NOTMERGEPEN:
    r = (~s) & (~d);
    break;
  case ROP_MERGEPAINT:
    r = s & p;
    break;
  case ROP_MERGENOTPEN:
    r = (~s) | d;
    break;
  case ROP_PATCOPY:
    r = p;
    break;
  case ROP_PATPAINT:
    r = (~s) | p | d;
    break;
  case ROP_PATINVERT:
    r = p ^ d;
    break;
  case ROP_NOT:
    r = ~d;
    break;
  case ROP_NOP:
    r = d;
    break;
  case ROP_BLACK:
    r = 0;
    break;
  case ROP_WHITE:
    r = 255;
    break;
  case ROP_MASKNOTPEN:
    r = (~s) & d;
    break;
  case ROP_NOTMASKPEN:
    r = ~(s & d);
    break;
  case ROP_NOTXORPEN:
    r = ~(s ^ d);
    break;
  case ROP_MERGEPENNOT:
    r = s | (~d);
    break;
  default:
    printf("rop id not supported!\n");
    break;
  }
  return r;
}
static void rop(argb_t *p_src1, 
        argb_t *p_src2, 
        u32 w, 
        u32 h, 
        u32 rop_a_id, 
        u32 rop_c_id, 
        u32 pattern, 
        MT_BOOL src1_en, 
        MT_BOOL src2_en)
{
  u32 i = 0, j = 0;
  u16 sa = 0, sr = 0, sg = 0, sb = 0;
  u16 da = 0, dr = 0, dg = 0, db = 0;
  u16 pa = 0, pr = 0, pg = 0, pb = 0;

  pa = (pattern >> 24) & 0xff;
  pr = (pattern >> 16) & 0xff;
  pg = (pattern >> 8) & 0xff;
  pb = pattern & 0xff;

  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      if(src1_en)
      {
        sa = ROUND(p_src1->p_a[i * w + j], N);
        sr = ROUND(p_src1->p_r[i * w + j], N);
        sg = ROUND(p_src1->p_g[i * w + j], N);
        sb = ROUND(p_src1->p_b[i * w + j], N);
      }
      if(src2_en)
      {
        da = ROUND(p_src2->p_a[i * w + j], N);
        dr = ROUND(p_src2->p_r[i * w + j], N);
        dg = ROUND(p_src2->p_g[i * w + j], N);
        db = ROUND(p_src2->p_b[i * w + j], N);
      }

      p_src1->p_a[i * w + j] = rop3(sa, da, pa, rop_a_id) << N;
      p_src1->p_r[i * w + j] = rop3(sr, dr, pr, rop_c_id) << N;
      p_src1->p_g[i * w + j] = rop3(sg, dg, pg, rop_c_id) << N;
      p_src1->p_b[i * w + j] = rop3(sb, db, pb, rop_c_id) << N;
    }
}
 
static void comp_argb(argb_t *p_src, u32 *p_dst, u32 w, u32 h, MT_BOOL demultiply)
{
  u32 i = 0, j = 0;
  u16 a = 0, r = 0, g = 0, b = 0;
  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      if(demultiply)
      {
        a = CLIP(ROUND(p_src->p_a[i * w + j], N));
        if(a == 0)
        {
          p_dst[i * w + j] = 0;
          continue;
        }
        r = CLIP(((p_src->p_r[i * w + j]<<6)+a/2)/a);
        g = CLIP(((p_src->p_g[i * w + j]<<6)+a/2)/a);
        b = CLIP(((p_src->p_b[i * w + j]<<6)+a/2)/a);
      }
      else
      {
        a = CLIP(ROUND(p_src->p_a[i * w + j], N));
        r = CLIP(ROUND(p_src->p_r[i * w + j], N));
        g = CLIP(ROUND(p_src->p_g[i * w + j], N));
        b = CLIP(ROUND(p_src->p_b[i * w + j], N));        
      }
      p_dst[i * w + j] = (a << 24) | (r << 16) | (g << 8) | b;
    }
}

static void comp_ck_mask(u8 *p_src1_ck,
          u8 *p_src2_ck,
          u8 *p_src3_ck,
          u8 *p_ck_mask,
          MT_BOOL src1_ck_en,
          MT_BOOL src2_ck_en,
          MT_BOOL src3_ck_en,
          u32 w,
          u32 h)
{
  u32 i = 0, j = 0;
  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
    {
      if(src1_ck_en)
        p_ck_mask[i * w + j] |= p_src1_ck[i * w + j];
      if(src2_ck_en)
        p_ck_mask[i * w + j] |= p_src2_ck[i * w + j];
      if(src3_ck_en)
        p_ck_mask[i * w + j] |= p_src3_ck[i * w + j];
    }
}


static void composite_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  argb_t src1 = {0};
  argb_t src2 = {0};
  argb_t src3 = {0};
  alpha_t src1_a = {0};
  u32 w = 0, h = 0;
  MT_BOOL demultiply_en = FALSE;

  w = p_gpe->dst_w;
  h = p_gpe->dst_h;

  // -- -- -- -- -- -- -- -- -- --global alpha & premult
  src1.p_a = (u16 *)gpe_malloc(w * h * 2);
  GPE_SPN_ASSERT(src1.p_a  != NULL);
  src1.p_r = (u16 *)gpe_malloc(w * h * 2);
  GPE_SPN_ASSERT(src1.p_r != NULL);
  src1.p_g = (u16 *)gpe_malloc(w * h * 2);
  GPE_SPN_ASSERT(src1.p_g  != NULL);
  src1.p_b = (u16 *)gpe_malloc(w * h * 2);
  GPE_SPN_ASSERT(src1.p_b != NULL);
  if((src1.p_a  ==  NULL) || (src1.p_r  ==  NULL) || (src1.p_g  ==  NULL) || (src1.p_b  ==  NULL))
    printf("malloc src1 failed\n");

  if(p_ctx->src1_sel)
  {
    GPE_SPN_ASSERT(p_gpe->p_src1_buf != NULL);
    alpha_premult(&src1,
            (u32 *)p_gpe->p_src1_buf,
            w,
            h,
            p_ctx->src_img.plane_alpha_en,
            p_ctx->src_img.plane_alpha,
            p_ctx->src_img.color_info.alpha_pre_mult_en);
  }

  if(p_ctx->src2_sel)
  {
    gpe_img_t *p_img = NULL;

    if(p_ctx->bg_img_en)
      p_img = &(p_ctx->bg_img);
    else
      p_img = &(p_ctx->dst_img);  
    src2.p_a = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src2.p_a  != NULL);
    src2.p_r = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src2.p_r  != NULL);
    src2.p_g = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src2.p_g  != NULL);
    src2.p_b = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src2.p_b  != NULL);

    GPE_SPN_ASSERT(p_gpe->p_src2_buf != NULL);
    printf("\r\n plane_alpha_en:%d,%d,%d", p_img->plane_alpha_en, p_img->plane_alpha, p_img->color_info.alpha_pre_mult_en);
    alpha_premult(&src2,
            (u32 *)p_gpe->p_src2_buf,
            w,
            h,
            p_img->plane_alpha_en,
            p_img->plane_alpha,
            p_img->color_info.alpha_pre_mult_en);
  }

  if(p_ctx->src3_sel)
  {
    src3.p_a = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src3.p_a  != NULL);
    src3.p_r = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src3.p_r  != NULL);
    src3.p_g = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src3.p_g  != NULL);
    src3.p_b = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src3.p_b  != NULL);

    GPE_SPN_ASSERT(p_gpe->p_src3_buf != NULL);
    alpha_premult(&src3,
            (u32 *)p_gpe->p_src3_buf,
            w,
            h,
            p_ctx->ex_img.plane_alpha_en,
            p_ctx->ex_img.plane_alpha,
            p_ctx->ex_img.color_info.alpha_pre_mult_en);
  }

  // -- -- -- -- -- -- -- -- -- --draw image, clip mask mix
  if(p_ctx->is_draw_multiply)
  {
//    GPE_SPN_ASSERT(p_ctx->src1_sel && p_ctx->src3_sel);
    draw_img_multiply(&src1, &src3, w, h);
  }

  if(p_ctx->is_draw_stencil)
  {
//    GPE_SPN_ASSERT(p_ctx->src1_sel && p_ctx->src3_sel);
    src1_a.p_aa = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src1_a.p_aa  != NULL);
    src1_a.p_ar = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src1_a.p_ar  != NULL);
    src1_a.p_ag = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src1_a.p_ag  != NULL);
    src1_a.p_ab = (u16 *)gpe_malloc(w * h * 2);
    GPE_SPN_ASSERT(src1_a.p_ab != NULL);
    draw_img_stencil(&src1, &src3, &src1_a, w, h);
  }

  if(p_ctx->alpha_map_en)
  {
//    GPE_SPN_ASSERT(p_ctx->src1_sel && p_ctx->src3_sel);
    if(p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_MIX_NORMAL)
      clip_mask_mix(&src1, &src3, w, h, 0);
    else if(p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_MIX_EX)
      clip_mask_mix(&src1, &src3, w, h, 1);
  }

  if(p_ctx->colorize_en)
  {
    colorize(&src1, p_ctx->color, w, h);
  }
  // -- -- -- -- -- -- -- -- -- ---blend, rop
  if(p_ctx->blend_en)
  {
    if(g_debug.new_blend_en)
        blend(&src1, &src2, &src1_a, w, h, p_ctx->src_blend_fact, p_ctx->dst_blend_fact, g_debug.comset[0], g_debug.comset[1]);
    else
        blend(&src1, &src2, &src1_a, w, h, p_ctx->src_blend_fact, p_ctx->dst_blend_fact, p_ctx->src_blend_fact, p_ctx->dst_blend_fact);
  }

  if(p_ctx->rop_en)
    rop(&src1, &src2, w, h, p_ctx->rop_a_mod, p_ctx->rop_c_mod, 
         p_ctx->rop_pattern, p_ctx->src1_sel, p_ctx->src2_sel);

  if(!p_ctx->comp_en)
    rop(&src1, &src2, w, h, ROP_COPYPEN, 
         ROP_COPYPEN, 0, p_ctx->src1_sel, p_ctx->src2_sel);  
  if(p_ctx->blend_en && p_ctx->demultiply_en)
    demultiply_en = TRUE;
  printf("\r\n ~~~~~demultiply_en:%d", demultiply_en);
  comp_argb(&src1, (u32 *)p_gpe->p_src2_buf, w, h, demultiply_en);

  gpe_free(src1.p_a);
  gpe_free(src1.p_r);
  gpe_free(src1.p_g);
  gpe_free(src1.p_b);

  if(p_ctx->src2_sel)
  {
    gpe_free(src2.p_a);
    gpe_free(src2.p_r);
    gpe_free(src2.p_g);
    gpe_free(src2.p_b);
  }

  if(p_ctx->src3_sel)
  {
    gpe_free(src3.p_a);
    gpe_free(src3.p_r);
    gpe_free(src3.p_g);
    gpe_free(src3.p_b);
  }

  if(p_ctx->is_draw_stencil)
  {
    gpe_free(src1_a.p_aa);
    gpe_free(src1_a.p_ar);
    gpe_free(src1_a.p_ag);
    gpe_free(src1_a.p_ab);
  }
}

static unsigned int pixpremult(unsigned int d)
{
    unsigned int a,r,g,b;
    unsigned int coef;
    unsigned int out;
    a = (d >> 24) & 0xFF;
    r = (d >> 16) & 0xFF;
    g = (d >> 8 ) & 0xFF;
    b = d         & 0xFF;

    coef = a*0x101;
    coef = (coef >> 8) + ((coef>>7) & 0x01);

    r = r*coef >> 8;
    g = g*coef >> 8;
    b = b*coef >> 8;

    out = ((a<<24) + ((r&0xff)<<16) + ((g&0xff)<<8) + (b&0xff));

    return out;
}

static u32 dithering_tab2bit[2][2] = {{16, 48}, {32, 0}};
static u32 dithering_tab3bit[2][2] = {{8, 24}, {16, 0}};
static u32 dithering_tab4bit[2][2] = {{4, 12}, {8, 0}};
static u32 dithering_tab5bit[2][2] = {{2, 6}, {4, 0}};
static u32 dithering_tab6bit[2][2] = {{1, 3}, {2, 0}};

//argb8888->rgb
#ifndef OLD_CLIP
static void color_reduction(
           MT_BOOL clip_en,
           u8 *p_clip,
           u32 clip_pitch,
           u32 *p_src,
           u32 *p_src2_bak,
           u8 *p_dst,
           u8 *p_ck_buf,
           u32 *p_clip_mask_buf,
           u8 *scale3d_mask_buf,
           u32 w,
           u32 h,
           u32 pitch,
           u32 x,
           u32 y,
             color_format_t format,
           MT_BOOL little_endian,
           MT_BOOL ck_en,
           MT_BOOL clip_mask_en,
           u32 key_out,
           MT_BOOL rgb2yuv_flag,
           MT_BOOL dithering_en,
           MT_BOOL negative_stride,
           u32 dst_x,
           u32 dst_y,
           u32 ds_mod,
           u32 ds_choose,
           u16 *p_gradt_buf)

#else
static void color_reduction(u32 *p_src,
           u32 *p_src2_bak,
           u8 *p_dst,
           u8 *p_ck_buf,
           u32 *p_clip_mask_buf,
           u8 *scale3d_mask_buf,
           u32 w,
           u32 h,
           u32 pitch,
           u32 x,
           u32 y,
             color_format_t format,
           MT_BOOL little_endian,
           MT_BOOL ck_en,
           MT_BOOL clip_mask_en,
           u32 key_out,
           MT_BOOL rgb2yuv_flag,
           MT_BOOL dithering_en,
           MT_BOOL negative_stride,
           u32 dst_x,
           u32 dst_y,
           u32 ds_mod,
           u32 ds_choose,
           u16 *p_gradt_buf)
#endif           
{
  u32 i = 0, j = 0;
  int a = 0, r = 0, g = 0, b = 0;
  int a1 = 0, r1 = 0, g1 = 0, b1 = 0;
  int Y =0, U =0, V = 0;
  int w2 = 0;
  int Y1 = 0, U1 = 0, V1 = 0;
  u32 tmp = 0;
  u16 tmp16 = 0;
  u16 *p_ptr16 = NULL;
  u32 *p_ptr32 = NULL;
  u16 gradt_msk = 0;
  u32 key_flag = 0;
  
  printf("src=0x%0"PRIx64"\n", (ulong)p_src);
  printf("dst = 0x%0"PRIx64"\n", (ulong)p_dst);
  
  if(format == Y1VY0U)
    w2 = w / 2;
  else
    w2 = w;

  for(i = 0; i < h; i++)
    for(j = 0; j < w2; j++)
    { 
#ifndef OLD_CLIP
      if(clip_en)
      {
        if(p_clip[(i + y) * clip_pitch + j + x] == 0)
          continue;
      }
#endif
      if(negative_stride)
      {
          tmp = p_src[(h - 1 - i) * w + j];
          if(g_debug.dst_premult_en)
            tmp = pixpremult(tmp);
        
          if(format == Y1VY0U)
          {
              key_flag = 0;
              if(ds_choose == 1)
              {
                  if((ck_en && (p_ck_buf[(h - 1 - i) * w + j * 2] == 1)) || (ck_en && (p_ck_buf[(h - 1 - i) * w + j * 2 + 1] == 1)))
                      key_flag = 1;
              }
              else
              {
                  if(ds_mod == DS_MEDIA_UV)
                  {
                      if((ck_en && (p_ck_buf[(h - 1 - i) * w + j * 2] == 1)) || (ck_en && (p_ck_buf[(h - 1 - i) * w + j * 2 + 1] == 1)))
                          key_flag = 1;
                  }
                  else
                  {
                      if(ck_en && (p_ck_buf[(h - 1 - i) * w + j * 2] == 1))
                          key_flag = 2;
                      if(ck_en && (p_ck_buf[(h - 1 - i) * w + j * 2 + 1] == 1))
                          key_flag |= 4;                            
                  }
              }
              if(key_flag == 1)
              {
                if(key_out == 2)
                  tmp = 0xffffffff;
                else if(key_out == 1)
                  tmp = p_src2_bak[(h - 1 - i) * w + j];
                else
                  continue;
              }
              if(key_flag == 6)
                continue;			  
          }
          else
          {
              if(ck_en && (p_ck_buf[(h - 1 - i) * w + j] == 1))
              {
                if(key_out == 2)
                  tmp = 0xffffffff;
                else if(key_out == 1)
                  tmp = p_src2_bak[(h - 1 - i) * w + j];
                else
                  continue;
              }
          }
          if(clip_mask_en && ((p_clip_mask_buf[(h - 1 - i) * w + j] & 0x1) == 0))
          {
            if(key_out == 2)
              tmp = 0xffffffff;
            else if(key_out == 1)
              tmp = p_src2_bak[(h - 1 - i) * w + j];
            else
              continue;
          }       
          if(scale3d_mask_buf != NULL)
          {
            if((scale3d_mask_buf[(h - 1 - i) * w + j] & 0x1) == 1)
            {
                if(key_out == 2)
                  tmp = 0xffffffff;
                else if(key_out == 1)
                  tmp = p_src2_bak[(h - 1 - i) * w + j];
                else
                  continue;     
            }
          }
          if( p_gradt_buf != NULL )
          {
            gradt_msk = p_gradt_buf[i * w + j] >> 12;
            if( gradt_msk )
              continue;
          }
      }
      else
      {
          tmp = p_src[i * w + j];
          if(g_debug.dst_premult_en)
            tmp = pixpremult(tmp);          
          if(format == Y1VY0U)
          {
              key_flag = 0;
              if(ds_choose == 1)
              {
                  if((ck_en && (p_ck_buf[i * w + j * 2] == 1)) || (ck_en && (p_ck_buf[i * w + j * 2 + 1] == 1))) 
                      key_flag = 1;
              }
              else
              {
                  if(ds_mod == DS_MEDIA_UV)
                  {
                      if((ck_en && (p_ck_buf[i * w + j * 2] == 1)) || (ck_en && (p_ck_buf[i * w + j * 2 + 1] == 1))) 
                          key_flag = 1;
                  }
                  else if(ds_mod == DS_EVEN_UV)
                  {
                      if(ck_en && (p_ck_buf[i * w + j * 2] == 1))
                          key_flag = 2;
                      if(ck_en && (p_ck_buf[i * w + j * 2 + 1] == 1))
                          key_flag |= 4;                      
                  }                 
              }          
              if(key_flag == 1) 
              {
                if(key_out == 2)
                  tmp = 0xffffffff;
                else if(key_out == 1)
                  tmp = p_src2_bak[i * w + j];                
                else
                  continue;
              }    
              if(key_flag == 6)
                continue;
          }
          else
          {
              if(ck_en && (p_ck_buf[i * w + j] == 1))
              {
                if(key_out == 2)
                  tmp = 0xffffffff;
                else if(key_out == 1)
                {
                  tmp = p_src2_bak[i * w + j];  
                }
                else
                  continue;
              }
          }
          if(clip_mask_en && ((p_clip_mask_buf[i * w + j] & 0x1) == 0))
          {
            if(key_out == 2)
              tmp = 0xffffffff;
            else if(key_out == 1)
              tmp = p_src2_bak[i * w + j];                   
            else
              continue;
          } 
          if(scale3d_mask_buf != NULL)
          {
            if((scale3d_mask_buf[i * w + j] & 0x1) == 1)
            {
                if(key_out == 2)
                  tmp = 0xffffffff;
                else if(key_out == 1)
                  tmp = p_src2_bak[i * w + j];                       
                else
                  continue;     
            }
          }          
          if( p_gradt_buf != NULL )
          {
            gradt_msk = p_gradt_buf[i * w + j] >> 12;
            if( gradt_msk )
              continue;
          }
      }
      switch(format)
      {
      case CLUT_8:      
      case GRAY_8:       
        p_dst[(i + y) * pitch + j + x] = tmp & 0xff;
        break;
      case ALUT44:
        a = (tmp >> 28) & 0xf;
        p_dst[(i + y) * pitch + j + x] = (a << 4) | (tmp & 0xf);
        break;         
      case ALUT88:
        a = (tmp >> 24) & 0xff;
        tmp16 = (a << 8) | (tmp & 0xff);
        if(little_endian)
          tmp16 = ((tmp16 >> 8) & 0xff) | ((tmp16 & 0xff) << 8);
        p_ptr16 = (u16 *)p_dst;
        p_ptr16[(i + y) * pitch + j + x] = tmp16;
        break;
      case RGB233:
        if(dithering_en)
        {
            r = (tmp >> 16) & 0xff;
            g = (tmp >> 8) & 0xff;
            b = tmp  & 0xff;
            if((r & 0xc0) != 0xc0)
                r = ((r + dithering_tab2bit[(i + dst_y) % 2][(j + dst_x) % 2]) >> 6) & 0x3;
            else
                r = (r >> 6) & 0x3;
    
            if((g & 0xe0) != 0xe0)
                g = ((g + dithering_tab3bit[(i + dst_y) % 2][(j + dst_x) % 2]) >> 5) & 0x7;
            else
                g = (g >> 5) & 0x7;

            if((b & 0xe0) != 0xe0)
                b = ((b + dithering_tab3bit[(i + dst_y) % 2][(j + dst_x) % 2]) >> 5) & 0x7;
            else
                b = (b >> 5) & 0x7;
        }
        else
        {
            r = (tmp >> 22) & 0x3;
            g = (tmp >> 13) & 0x7;
            b = (tmp >> 5) & 0x7;
        }
        tmp = (r << 6) | (g << 3) | b;  
        p_dst[(i + y) * pitch + j + x] = tmp & 0xff;
        break;
      case RGB565:
        if(dithering_en)
        {
            r = (tmp >> 16) & 0xff;
            g = (tmp >> 8) & 0xff;
            b = tmp  & 0xff;
            if((r & 0xf8) != 0xf8)
                r = ((r + dithering_tab5bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
            else
                r = (r >> 3) & 0x1f;
            
            if((g & 0xfc) != 0xfc)
                g = ((g + dithering_tab6bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 2) & 0x3f;
            else
                g = (g >> 2) & 0x3f;
            
            if((b & 0xf8) != 0xf8)
                b = ((b + dithering_tab5bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
            else
                b = (b >> 3) & 0x1f;
        }
        else
        {
            r = (tmp >> 19) & 0x1f;
            g = (tmp >> 10) & 0x3f;
            b = (tmp >> 3) & 0x1f;
        }
        tmp16 = (r << 11) | (g << 5) | b;
        if(little_endian)
          tmp16 = ((tmp16 >> 8) & 0xff) | ((tmp16 & 0xff) << 8);        
        p_ptr16 = (u16 *)p_dst;
        p_ptr16[(i + y) * pitch + j + x] = tmp16;
        break;
      case ARGB1555:
        a = (tmp >> 31) & 0x1;
        if(dithering_en)
        {
          r = (tmp >> 16) & 0xff;
          g = (tmp >> 8) & 0xff;
          b = tmp  & 0xff;
          
          if((r & 0xf8) != 0xf8)
            r = ((r + dithering_tab5bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
          else
            r = (r >> 3) & 0x1f;
          
          if((g & 0xf8) != 0xf8)
            g = ((g + dithering_tab5bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
          else
            g = (g >> 3) & 0x1f;
          
          if((b & 0xf8) != 0xf8)
            b = ((b + dithering_tab5bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
          else
            b = (b >> 3) & 0x1f;
        }
        else
        {
          r = (tmp >> 19) & 0x1f;
          g = (tmp >> 11) & 0x1f;
          b = (tmp >> 3) & 0x1f;
        }
        tmp16 = (a << 15) | (r << 10) | (g << 5) | b;
        if(little_endian)
          tmp16 = ((tmp16 >> 8) & 0xff) | ((tmp16 & 0xff) << 8);
        p_ptr16 = (u16 *)p_dst;
        p_ptr16[(i + y) * pitch + j + x] = tmp16;
        break;
      case RGBA5551:
        a = (tmp >> 31) & 0x1;
        if(dithering_en)
        {
          r = (tmp >> 16) & 0xff;
          g = (tmp >> 8) & 0xff;
          b = tmp  & 0xff;
          
          if((r & 0xf8) != 0xf8)
            r = ((r + dithering_tab5bit[(i + dst_y) % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
          else
            r = (r >> 3) & 0x1f;
          
          if((g & 0xf8) != 0xf8)
            g = ((g + dithering_tab5bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
          else
            g = (g >> 3) & 0x1f;
          
          if((b & 0xf8) != 0xf8)
            b = ((b + dithering_tab5bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 3) & 0x1f;
          else
            b = (b >> 3) & 0x1f;
        }
        else
        {
          r = (tmp >> 19) & 0x1f;
          g = (tmp >> 11) & 0x1f;
          b = (tmp >> 3) & 0x1f;
        }
        tmp16 = (r << 11) | (g << 6) | (b << 1) | a;
        if(little_endian)
          tmp16 = ((tmp16 >> 8) & 0xff) | ((tmp16 & 0xff) << 8);
        p_ptr16 = (u16 *)p_dst;
        p_ptr16[(i + y) * pitch + j + x] = tmp16;
        break;
      case ARGB4444:
        a = (tmp >> 28) & 0xf;
        if(dithering_en)
        {
          r = (tmp >> 16) & 0xff;
          g = (tmp >> 8) & 0xff;
          b = tmp  & 0xff;
          
          if((r & 0xf0) != 0xf0)
            r = ((r + dithering_tab4bit[(i + dst_y) % 2][(j + dst_x) % 2]) >> 4) & 0xf;
          else
            r = (r >> 4) & 0xf;
          
          if((g & 0xf0) != 0xf0)
            g = ((g + dithering_tab4bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 4) & 0xf;
          else
            g = (g >> 4) & 0xf;
          
          if((b & 0xf0) != 0xf0)
            b = ((b + dithering_tab4bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 4) & 0xf;
          else
            b = (b >> 4) & 0xf;
        }
        else
        {
          r = (tmp >> 20) & 0xf;
          g = (tmp >> 12) & 0xf;
          b = (tmp >> 4) & 0xf;
        }
        tmp16 = (a << 12) | (r << 8) | (g << 4) | b;
        if(little_endian)
          tmp16 = ((tmp16 >> 8) & 0xff) | ((tmp16 & 0xff) << 8);
        p_ptr16 = (u16 *)p_dst;
        p_ptr16[(i + y) * pitch + j + x] = tmp16;
        break;
      case RGBA4444:
        a = (tmp >> 28) & 0xf;
        if(dithering_en)
        {
          r = (tmp >> 16) & 0xff;
          g = (tmp >> 8) & 0xff;
          b = tmp  & 0xff;
          if((r & 0xf0) != 0xf0)
            r = ((r + dithering_tab4bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 4) & 0xf;
          else
            r = (r >> 4) & 0xf;
          
          if((g & 0xf0) != 0xf0)
            g = ((g + dithering_tab4bit[(i + dst_y) % 2][(j + dst_x) % 2]) >> 4) & 0xf;
          else
            g = (g >> 4) & 0xf;
          
          if((b & 0xf0) != 0xf0)
            b = ((b + dithering_tab4bit[(i + dst_y)  % 2][(j + dst_x) % 2]) >> 4) & 0xf;
          else
            b = (b >> 4) & 0xf;
        }
        else
        {
          r = (tmp >> 20) & 0xf;
          g = (tmp >> 12) & 0xf;
          b = (tmp >> 4) & 0xf;
        }
        tmp16 = (r << 12) | (g << 8) | (b << 4) | a;
        if(little_endian)
          tmp16 = ((tmp16 >> 8) & 0xff) | ((tmp16 & 0xff) << 8);
        p_ptr16 = (u16 *)p_dst;
        p_ptr16[(i + y) * pitch + j + x] = tmp16;
        break;
      case Y1VY0U: 
        tmp = p_src[i * w + 2 * j];
        a = (tmp >> 24) & 0xff;
        r = (tmp >> 16) & 0xff;
        g = (tmp >> 8) & 0xff;
        b = tmp & 0xff;

        Y = ((((csc[0].cscp_00) * r >> 4) + ((csc[0].cscp_01) * g >> 4) + ((csc[0].cscp_02) * b >> 4) +  ((csc[0].cscdc_0) >> 4)))>> 4;
        U = ((((csc[0].cscp_10) * r >> 4) + ((csc[0].cscp_11) * g >> 4) + ((csc[0].cscp_12) * b >> 4) +  ((csc[0].cscdc_1) >> 4)))>> 4;
        V = ((((csc[0].cscp_20) * r >> 4) + ((csc[0].cscp_21) * g >> 4) + ((csc[0].cscp_22) * b >> 4) +  ((csc[0].cscdc_2) >> 4)))>> 4;  
        
        tmp = p_src[i * w + 2 * j + 1];
        a1 = (tmp >> 24) & 0xff;
        r1 = (tmp >> 16) & 0xff;
        g1 = (tmp >> 8) & 0xff;
        b1 = tmp & 0xff;

        Y1 = ((((csc[0].cscp_00) * r1 >> 4) + ((csc[0].cscp_01) * g1 >> 4) + ((csc[0].cscp_02) * b1 >> 4) +  ((csc[0].cscdc_0) >> 4)))>> 4;
        U1 = ((((csc[0].cscp_10) * r1 >> 4) + ((csc[0].cscp_11) * g1 >> 4) + ((csc[0].cscp_12) * b1 >> 4) +  ((csc[0].cscdc_1) >> 4)))>> 4;
        V1 = ((((csc[0].cscp_20) * r1 >> 4) + ((csc[0].cscp_21) * g1 >> 4) + ((csc[0].cscp_22) * b1 >> 4) +  ((csc[0].cscdc_2) >> 4)))>> 4;  
        
        p_ptr32 = (u32 *)p_dst;
        if(rgb2yuv_flag)
        {
          if(ds_mod == DS_MEDIA_UV)
          {
              if(little_endian == 0)
              {
                p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (Y1 << 24) | 
                     ((((V + V1) >> 1) & 0xff) << 16) | (Y << 8) | (((U + U1) >> 1) & 0xff);
              }
              else
              {
                p_ptr32[(i + y) * pitch / 2 + j + x / 2] = ((((U + U1) >> 1) & 0xff) << 24) 
                     | (Y << 16) | ((((V + V1) >> 1) & 0xff) << 8) | Y1;
              }
          }
          else
          {
              if(little_endian == 0)
              {
                if(key_flag == 0)
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (Y1 << 24) | (V << 16) | (Y << 8) | U;
                if(key_flag == 2)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (Y1 << 24) | (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0x00ffffff);
                }
                else if(key_flag == 4)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0xff000000) | (V << 16) | (Y << 8) | U;
                }                 
              }
              else
              {
                if(key_flag == 0)
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (U << 24) | (Y << 16) | (V << 8) | Y1;

                if(key_flag == 2)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0xffffff00) | Y1;
                }
                else if(key_flag == 4)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0xff) | (U << 24) | (Y << 16) | (V << 8);
                }                  
              }            
          }
        }
        else
        { 
          if(ds_mod == DS_MEDIA_UV)
          {        
              if(little_endian == 0)
                p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (r1 << 24) 
                  | ((((b1 + b) >> 1) & 0xff) << 16) | (r << 8) | (((g + g1) >> 1) & 0xff);
              else
                p_ptr32[(i + y) * pitch / 2+ j + x / 2] = ((((g + g1) >> 1) & 0xff) << 24) | (r << 16) 
                                                | ((((b1  + b) >> 1) & 0xff) << 8) | r1;       
          }
          else
          {
              if(little_endian == 0)
              {
                if(key_flag == 0)
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (r1 << 24) | (b << 16) | (r << 8) | g;
                if(key_flag == 2)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0x00ffffff) | (r1 << 24);
                }
                else if(key_flag == 4)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0xff000000) | (b << 16) | (r << 8) | g;
                }                 
              }
              else
              {
                if(key_flag == 0)
                  p_ptr32[(i + y) * pitch / 2+ j + x / 2] = (g << 24) | (r << 16) | (b << 8) | r1;        
                if(key_flag == 2)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0xffffff00) | r1;
                }
                else if(key_flag == 4)
                {
                  p_ptr32[(i + y) * pitch / 2 + j + x / 2] = (p_ptr32[(i + y) * pitch / 2 + j + x / 2] & 0xff) | (g << 24) | (r << 16) | (b << 8);
                }                 
              }
          }
        }
        break;
      case AYUV8888:
        a = (tmp >> 24) & 0xff;
        r = (tmp >> 16) & 0xff;
        g = (tmp >> 8) & 0xff;
        b = tmp & 0xff;
        Y = ((((csc[0].cscp_00) * r >> 4) + ((csc[0].cscp_01) * g >> 4) + ((csc[0].cscp_02) * b >> 4) +  ((csc[0].cscdc_0) >> 4)))>> 4;
        U = ((((csc[0].cscp_10) * r >> 4) + ((csc[0].cscp_11) * g >> 4) + ((csc[0].cscp_12) * b >> 4) +  ((csc[0].cscdc_1) >> 4)))>> 4;
        V = ((((csc[0].cscp_20) * r >> 4) + ((csc[0].cscp_21) * g >> 4) + ((csc[0].cscp_22) * b >> 4) +  ((csc[0].cscdc_2) >> 4)))>> 4;  
        Y = CLIP(Y);
        U = CLIP(U);
        V= CLIP(V);
      
        p_ptr32 = (u32 *)p_dst;
        if(rgb2yuv_flag)
        {
           if(little_endian)
             p_ptr32[(i + y) * pitch + j + x] = (V << 24) | (U << 16) | (Y << 8) | a;
           else
             p_ptr32[(i + y) * pitch + j + x] = (a << 24) | (Y << 16) | (U << 8) | V;
        }
        else
        {
           if(little_endian)
                 p_ptr32[(i + y) * pitch + j + x] = (b << 24) | (g << 16) | (r << 8) | a;
           else
             p_ptr32[(i + y) * pitch + j + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        break;
      case YUVA8888:
        a = (tmp >> 24) & 0xff;
        r = (tmp >> 16) & 0xff;
        g = (tmp >> 8) & 0xff;
        b = tmp & 0xff;

        Y = ((((csc[0].cscp_00) * r >> 4) + ((csc[0].cscp_01) * g >> 4) + ((csc[0].cscp_02) * b >> 4) +  ((csc[0].cscdc_0) >> 4)))>> 4;
        U = ((((csc[0].cscp_10) * r >> 4) + ((csc[0].cscp_11) * g >> 4) + ((csc[0].cscp_12) * b >> 4) +  ((csc[0].cscdc_1) >> 4)))>> 4;
        V = ((((csc[0].cscp_20) * r >> 4) + ((csc[0].cscp_21) * g >> 4) + ((csc[0].cscp_22) * b >> 4) +  ((csc[0].cscdc_2) >> 4)))>> 4;  
        Y = CLIP(Y);
        U = CLIP(U);
        V= CLIP(V);
        
        p_ptr32 = (u32 *)p_dst;
        if(rgb2yuv_flag)
        {
            if(little_endian)
              p_ptr32[(i + y) * pitch + j + x] = (a << 24) | (V << 16) | (U << 8) | Y;
            else
              p_ptr32[(i + y) * pitch + j + x] = (Y << 24) | (U << 16) | (V << 8) | a;
        }
        else
        {
            if(little_endian)
              p_ptr32[(i + y) * pitch + j + x] = (a << 24) | (b << 16) | (g << 8) | r;
            else
              p_ptr32[(i + y) * pitch + j + x] = (r << 24) | (g << 16) | (b << 8) | a;
        }
        break; 
      case ARGB8888:
        a = (tmp >> 24) & 0xff;
        r = (tmp >> 16) & 0xff;
        g = (tmp >> 8) & 0xff;
        b = tmp & 0xff;
        p_ptr32 = (u32 *)p_dst;
        if(little_endian)
          p_ptr32[(i + y) * pitch + j + x] = (b << 24) | (g << 16) | (r << 8) | a;
        else
          p_ptr32[(i + y) * pitch + j + x] = (a << 24) | (r << 16) | (g << 8) | b;
        break;     
      case RGBA8888:
        a = (tmp >> 24) & 0xff;
        r = (tmp >> 16) & 0xff;
        g = (tmp >> 8) & 0xff;
        b = tmp & 0xff;
        p_ptr32 = (u32 *)p_dst;
        if(little_endian)
          p_ptr32[(i + y) * pitch + j + x] = (a << 24) | (b << 16) | (g << 8) | r;
        else
          p_ptr32[(i + y) * pitch + j + x] = (r << 24) | (g << 16) | (b << 8) | a;
        break;
      default:
        printf("true rgb format not supported!\n");
        break;
      }
    }
}

static void dst_format_stage(aria_gpe_context_t *p_ctx, gpe_hw_t *p_gpe)
{
  u8 *p_ptr = NULL;
  u32 w = 0, h = 0, y = 0;
  u32 bpp = 0;
  MT_BOOL src1_ck_en = FALSE, src2_ck_en = FALSE, src3_ck_en = FALSE;
  MT_BOOL clip_mask_en = FALSE;
  u8 *p_ck_mask = NULL;
  MT_BOOL ck_en = FALSE;
  u32 key_out = 0;
  u8 *p_clip = NULL;
  u16 *p_gradt_buf = NULL;
  w = p_gpe->dst_w;
  h = p_gpe->dst_h;

  // -- -- -- -- -- -- -- -- -- -- --color key, clip mask logical
  src1_ck_en = (p_gpe->src1_in && p_ctx->src_img.ck_en) || p_ctx->src_with_mask;
  src2_ck_en = p_gpe->src2_in && p_ctx->dst_img.ck_en;
  src3_ck_en = p_gpe->src3_in && p_ctx->ex_img.ck_en;

  p_ck_mask = (u8 *)gpe_malloc(w * h);
  memset(p_ck_mask, 0, w * h);

  if(src1_ck_en || src2_ck_en || src3_ck_en)
  {
    comp_ck_mask(p_gpe->p_src1_ck_buf,
           p_gpe->p_src2_ck_buf,
         p_gpe->p_src3_ck_buf,
         p_ck_mask,
         src1_ck_en,
         src2_ck_en,
         src3_ck_en,
         w,
         h);
  }

  clip_mask_en = p_ctx->alpha_map_en && (p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_LOGICAL);
  ck_en = src1_ck_en || src2_ck_en || src3_ck_en;

  key_out = g_debug.key_msk + g_debug.key_set;
  bpp = get_bpp(p_ctx->dst_img.color_info.bpp);

 if(p_ctx->dst_img.negative_stride)
    y =  p_ctx->dst_img.height -  p_ctx->dst_img.rect.y - p_ctx->dst_img.rect.h;
 else
    y = p_ctx->dst_img.rect.y;
	
 #ifndef OLD_CLIP
   if(p_ctx->clip_en)
   {
    int i, j;
    u8 data0, data1;

     p_clip = (u8 *)gpe_malloc(p_ctx->dst_img.width * p_ctx->dst_img.height);
     if(p_ctx->clip.clip_mode == CLIP_MODE_INSIDE) 
      {
      data0 = 0;
      data1 = 1;
      }
     else
      {
      data0 = 1;
      data1 = 0;        
      }
      memset(p_clip, data0, p_ctx->dst_img.width * p_ctx->dst_img.height); 
     for(i = p_ctx->clip.clip_rect.s32Ypos; i < p_ctx->clip.clip_rect.s32Ypos + p_ctx->clip.clip_rect.u32Height; i++)
        for(j = p_ctx->clip.clip_rect.s32Xpos; j < p_ctx->clip.clip_rect.s32Xpos+ p_ctx->clip.clip_rect.u32Width; j++)
          p_clip[i * p_ctx->dst_img.width + j] = data1;
   }
#endif

 printf("\r\n ds_mode:%d, ds_choose:%d", p_ctx->dst_ds_mod, p_ctx->dst_ds_choose);
 if(p_ctx->p_gradt_buf != NULL)
    p_gradt_buf = (u16 *)mt_mmz_map(p_ctx->p_gradt_buf, 0);
 else
    p_gradt_buf = NULL;
  color_reduction(
  #ifndef OLD_CLIP
            p_ctx->clip_en,
            p_clip,
            p_ctx->dst_img.width,
#endif
		(u32 *)p_gpe->p_src2_buf,
           (u32 *)p_gpe->p_src2_buf_bak,
           p_gpe->p_dst_buf,
           p_ck_mask,
           (u32 *)p_gpe->p_src3_buf,
           p_gpe->scale3d_mask,           
           p_gpe->dst_w,
           p_gpe->dst_h,
           p_ctx->dst_img.pitch * 8 / bpp,
           p_ctx->dst_img.rect.x,
           y,
             p_ctx->dst_img.color_info.color_fmt,
           p_ctx->dst_img.color_info.little_endian,
           ck_en,
           clip_mask_en,
           key_out,
           p_ctx->dst_rgb2yuv_en,
           !p_ctx->no_dithering,
           p_ctx->dst_img.negative_stride,
           p_ctx->dst_img.rect.x,
           p_ctx->dst_img.rect.y,
           p_ctx->dst_ds_mod,
           p_ctx->dst_ds_choose,
          p_gradt_buf);
  if(p_ctx->p_gradt_buf != NULL)
    mt_mmz_unmap((void *)p_gradt_buf);
  
  gpe_free(p_ck_mask);
#ifndef OLD_CLIP  
  if(p_clip)
    gpe_free(p_clip);
#endif
}

#if 0
MT_BOOL gpe_sw_compare(aria_gpe_context_t *p_ctx)
{
  u32 i = 0;
  u32 count = 0;
  u32 diff_max = 0;
  u32 max_i = 0;
  u32 diff = 0;
  u32 size = p_ctx->dst_img.pitch * p_ctx->dst_img.height;
  u8 *p_ptr1 = (u8 *)p_dst_buf2;
  u8 *p_ptr2 = (u8 *)p_ctx->dst_img.buf;

  printf("\ncomparing result of sw and hw gpe......\n");

  for(i = 0; i < size; i++)
  {
    if(p_ptr1[i] != p_ptr2[i])
    {
      if(count < 10)
        printf("sw ptr1[%d]=0x%0x, hw ptr2[%d]=0x%0x\n", i, p_ptr1[i], i, p_ptr2[i]);
      count++;
      diff = (p_ptr1[i] > p_ptr2[i]) ? (p_ptr1[i] - p_ptr2[i]) : (p_ptr2[i] - p_ptr1[i]);
      if(diff > diff_max)
      {
        diff_max = diff;
        max_i = i;
      }
    }
    if((diff_max > 100) && (count >= 10))
      break;
  }

  if(count == 0)
    printf("result of hw and sw gpe are the same\n");
  else
    printf("max diff of hw and sw gpe is %d, max_i=%d, sw = %d, hw = %d\n", 
          diff_max, max_i, p_ptr1[max_i], p_ptr2[max_i]);
  mtos_free(p_dst_buf2);
  p_dst_buf2 = NULL;
  if(count == 0)
    return TRUE;
  else
    return FALSE;
}
#else
MT_BOOL gpe_sw_compare(aria_gpe_context_t *p_ctx)
{
  u32 i = 0, j = 0, k = 0;
  u32 count = 0;
  u32 diff_max = 0;
  u32 max_i = 0;
  u32 diff = 0;
  u32 size = p_ctx->dst_img.pitch * p_ctx->dst_img.height;
  u8 *p_ptr1 = (u8 *)p_dst_buf2;
  u8 *p_ptr2 = (u8 *)mt_mmz_map(p_ctx->dst_img.buf, 0);//p_ctx->dst_img.buf;
  u32 bpp;
  u32 hw = 0;
  u32 sw = 0;
  
  //printf("\ncomparing result of sw and hw gpe......p_ctx->dst_img.buf : %x\n", p_ctx->dst_img.buf);
  bpp = get_bpp(p_ctx->dst_img.color_info.bpp);
  //gpe_spn_invalidate_buf(p_ptr2, size);
  //printf("\r\n bpp:%d, x,y,w,h:[%d,%d,%d,%d]", bpp, p_ctx->dst_img.rect.x, p_ctx->dst_img.rect.y, p_ctx->dst_img.rect.w, p_ctx->dst_img.rect.h);
  /*
  if(bpp >= 8)
  { 
      bpp = bpp / 8;
//      for(i = p_ctx->dst_img.rect.y; i < p_ctx->dst_img.rect.h + p_ctx->dst_img.rect.y; i++)
      i = p_ctx->dst_img.rect.y;

      {
        for(j = p_ctx->dst_img.rect.x; j < p_ctx->dst_img.rect.w + p_ctx->dst_img.rect.x; j++)
        {
          if(j >  (p_ctx->dst_img.rect.x + 10))
          {
            printf("i : %d : j : %d bpp: %d\n", i,j, bpp);
                break;
          }
          for(k = 0; k < bpp; k++)
          {
            sw = p_ptr1[i * p_ctx->dst_img.pitch + j * bpp + k];
            hw = p_ptr2[i * p_ctx->dst_img.pitch + j * bpp + k]; 
            printf("sw %02x hw: %02x\n", sw, hw);
          }
        }
      }
  }//*/
  
  if(bpp >= 8)
  { 
      bpp = bpp / 8;
      for(i = p_ctx->dst_img.rect.y; i < p_ctx->dst_img.rect.h + p_ctx->dst_img.rect.y; i++)
      {
        for(j = p_ctx->dst_img.rect.x; j < p_ctx->dst_img.rect.w + p_ctx->dst_img.rect.x; j++)
        {
          for(k = 0; k < bpp; k++)
          {
            sw = p_ptr1[i * p_ctx->dst_img.pitch + j * bpp + k];
            hw = p_ptr2[i * p_ctx->dst_img.pitch + j * bpp + k];
            if(i * p_ctx->dst_img.pitch + j * bpp + k == 115932)
                printf("\r\n sw[115932]:0x%08x, hw[115932]:0x%08x", sw, hw);
            if(sw != hw)
            {
                if(count < 10)
                {
                    printf("\r\nsw ptr1[%d]=0x%0x, hw ptr2[%d]=0x%0x, x:%d, y:%d, k:%d, sw addr:0x%0"PRIx64", hw addr:0x%0"PRIx64, 
                        i * p_ctx->dst_img.pitch + j * bpp + k, sw, i * p_ctx->dst_img.pitch + j * bpp + k, hw, 
                        j - p_ctx->dst_img.rect.x, i - p_ctx->dst_img.rect.y, k, 
                        (ulong)&p_ptr1[i * p_ctx->dst_img.pitch + j * bpp + k], (ulong)&p_ptr2[i * p_ctx->dst_img.pitch + j * bpp + k]);
                }
                count++;
                diff = (sw > hw) ? (sw - hw) : (hw - sw);
                if(diff > diff_max)
                {
                    diff_max = diff;
                    max_i = i * p_ctx->dst_img.pitch + j * bpp + k;
                }
            }
          }
        }
      }
  }
  else
  {
      for(i = p_ctx->dst_img.rect.y; i < p_ctx->dst_img.rect.h + p_ctx->dst_img.rect.y; i++)
      {
        for(j = p_ctx->dst_img.rect.x; j < p_ctx->dst_img.rect.w + p_ctx->dst_img.rect.x; j += 8 / bpp)
        {
            sw = p_ptr1[i * p_ctx->dst_img.pitch + j * bpp / 8];
            hw = p_ptr2[i * p_ctx->dst_img.pitch + j * bpp / 8];
            if(sw != hw)
            {
                if(count < 10)
                {
                    printf("\r\nsw ptr1[%d]=0x%0x, hw ptr2[%d]=0x%0x\n", 
                        i * p_ctx->dst_img.pitch + j * bpp / 8, sw, i * p_ctx->dst_img.pitch + j * bpp / 8, hw);
                }
                count++;
                diff = (sw > hw) ? (sw - hw) : (hw - sw);
                if(diff > diff_max)
                {
                    diff_max = diff;
                    max_i = i * p_ctx->dst_img.pitch + j * bpp / 8;
                }
            }
        }
      }   
  }
  
  if(count == 0)
    printf("\r\n>>>>>>>>>>>>result of hw and sw gpe are the same>>>>>>>>>>>>>>>>>>>\n");
  else
    printf("\r\nxxxxxxxxxxxxxxxmax diff of hw and sw gpe is %d, max_i = %d, sw = %d, hw = %d\n", 
          diff_max, max_i, p_ptr1[max_i], p_ptr2[max_i]);

  mt_mmz_unmap((void *)p_ptr2);
  gpe_free(p_dst_buf2);
  p_dst_buf2 = NULL;
  if(count == 0)
    return TRUE;
  else
    return FALSE;
}
#endif

void gpe_core(aria_gpe_context_t *p_ctx, MT_BOOL sw_comp)
{
  gpe_hw_t gpe = {0};
  u32 size = 0;

  printf("\n========start to run software gpe========\n");
  load_param(p_ctx, &gpe);
  //printf("load_param ok!\n");
  
  load_img(p_ctx, &gpe, sw_comp);
 //printf("load_img ok!\n");
  
  src_format_stage(p_ctx, &gpe);
  //printf("src_format_stage ok!\n");
  
  scale_stage(p_ctx, &gpe);
  //printf("scale_stage ok!\n");
  
  rotate_stage(p_ctx, &gpe);
  //printf("rotate_stage ok!\n");

  gaussian_blur_stage(p_ctx, &gpe);
//  printf("gaussian_blur_stage ok!\n");
  
  paint_stage(p_ctx, &gpe);
 // printf("paint_stage ok!\n");
  
  xylc_stage(p_ctx, &gpe);
 // printf("xylc_stage ok!\n");
  
  composite_stage(p_ctx, &gpe);
 // printf("composite_stage ok!\n");
  
  dst_format_stage(p_ctx, &gpe);
 // printf("dst_format_stage ok!\n");
    
  free_gpe(&gpe);
//  printf("========software gpe finished========\n");
}

#pragma GCC diagnostic pop
#endif
