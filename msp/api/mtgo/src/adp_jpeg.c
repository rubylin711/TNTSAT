/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
#include "mt_type.h"
#include "mtgo_common.h"
#include "mt_go_decoder.h"
#include "mt_jpeglib.h"
#include "mt_jpeg_api.h"
#include "mt_gfx_comm.h"
#include "mtgo_surface.h"
#include "mt_module_debug.h"
#include "../../jpeg/src_6b/jpegint.h"
#include "adp_jpeg.h"
#include "string.h"
#include "jpeg_hdec_api.h"
#include "mt_cache.h"
#ifdef CONFIG_MT_FPGA_GPE
#include <stdlib.h>
#endif
static struct jpeg_error_mgr jerr;

#ifdef CONFIG_MT_FPGA_GPE
#define RANDOM_DATA(a, b) ((rand() % (b - a + 1)) + a)
static u32 sw_width[4] = {0};
static u32 sw_height[4] = {0};
int *img_rot[4] = {NULL};
MT_JPEG_DBGCFG_S jpeg_dbg_info = {0};
#endif

/** a instance about jpeg decode*/
typedef struct _DEC_JPEGINSTANCE_S
{
	void *cinfo;
    mt_char *  pSrcVirBuf;/**  a pointer for the stream data buffer */
	phys_addr_t  pSrcPhyBuf;/**  a pointer for the stream data buffer */
	mt_s32 srcBufSize;
	MT_RECT activeSize; /*active rect of decoded picture*/
} DEC_JPEGINSTANCE_S;

#ifdef CONFIG_MT_FPGA_GPE
#include <sys/time.h>
static  struct timeval loop_test_start, loop_test_end;
extern ulong total_cost_time;
#endif


/*get the scaling rate*/
#define ADP_JPEGCALSCALE(num, dem, result) \
do { \
    MT_U32 ScaleTmp; \
    ScaleTmp = (MT_U32)((2*num  + dem)/(dem *2)); \
    if (8 <= ScaleTmp) \
    { \
        result = 3; \
    } \
    else if (4 <= ScaleTmp) \
    { \
        result = 2; \
    } \
    else if (2 <= ScaleTmp) \
    { \
        result = 1; \
    } \
    else \
    { \
        result = 0; \
    } \
}while(0)

static MT_BOOL check_if_hw_huffman_support(j_decompress_ptr p_cinfo)
{
    jpeg_component_info *compptr = p_cinfo->comp_info;

    if(p_cinfo->progressive_mode == MT_TRUE)
        return MT_FALSE;
    if(p_cinfo->jpeg_color_space == JCS_GRAYSCALE)//fix bug 117168
        return TRUE;
    if(p_cinfo->jpeg_color_space != JCS_YCbCr)
    {
        if(( p_cinfo->max_h_samp_factor > 1) || ( p_cinfo->max_v_samp_factor > 1))
            return MT_FALSE;
    }
    else
    {
      //hw huffman only support: Y 1x1,2x1,1x2,2x2, UV 1x1
      MT_INFO_MTGO("\r\n max_h_samp_factor:%d, comp:%d,%d,%d, max_v_samp_factor:%d, comp:%d,%d,%d",
      p_cinfo->max_h_samp_factor,compptr[0].h_samp_factor,compptr[1].h_samp_factor,compptr[2].h_samp_factor,
      p_cinfo->max_v_samp_factor,compptr[0].v_samp_factor,compptr[1].v_samp_factor,compptr[2].v_samp_factor); 
      if((compptr[1].h_samp_factor == 2 || compptr[1].v_samp_factor == 2 
        || compptr[2].h_samp_factor == 2 || compptr[2].v_samp_factor == 2))
        return MT_FALSE;
    }
    return MT_TRUE;
}

#define JPEG_ALIGN(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))

mt_s32 MTGO_ADP_JPGCreateDecoder(DEC_HANDLE *pJpegDec, const MTGO_DEC_ATTR_S *pSrcDesc)
{
	DEC_JPEGINSTANCE_S *pDecJpegInstance;
	MT_U32 Length = 0;
	phys_addr_t pSrcPhyBuf     = 0;
	mt_char *pSrcVirBuf     = NULL;
	j_decompress_ptr cinfo = NULL;

	/** check parameter*/
	if (MT_NULL_PTR == pSrcDesc)
	{
		MTGO_ERROR(MTGO_ERR_NULLPTR);
		return MTGO_ERR_NULLPTR;
	}

	/** create decode instance*/
	pDecJpegInstance = (DEC_JPEGINSTANCE_S *)MTGO_Malloc(sizeof(DEC_JPEGINSTANCE_S));
	if (MT_NULL_PTR == pDecJpegInstance)
	{
		MTGO_ERROR(MTGO_ERR_NOMEM);
		return MTGO_ERR_NOMEM;
	}

	MTGO_MemSet((MT_VOID *)pDecJpegInstance, 0x0, sizeof(DEC_JPEGINSTANCE_S));

	Length = JPEG_ALIGN(pSrcDesc->SrcInfo.MemInfo.Length, CACHE_LINE_SIZE);
	pSrcPhyBuf = MT_GFX_AllocMem(Length, 256, (mt_char*)"jpeg", (mt_char*)"JPEG");
	if(0 == pSrcPhyBuf)
	{
		MT_ERR_MTGO("MT_GFX_AllocMem failure\n");
		MTGO_Free(pDecJpegInstance);
		return MTGO_ERR_NOMEM;
	}
	pSrcVirBuf = (mt_char*)MT_GFX_MapCached(pSrcPhyBuf); 
	if(NULL == pSrcVirBuf)
	{
		MT_ERR_MTGO("MT_GFX_MapCached failure\n");
		MTGO_Free(pDecJpegInstance);
        return MTGO_ERR_DEPEND_MMZ;
	}

	memset(pSrcVirBuf,0,Length);
	MT_GFX_Flush((void*)pSrcVirBuf, 0, 0);
	memcpy(pSrcVirBuf, pSrcDesc->SrcInfo.MemInfo.pAddr, pSrcDesc->SrcInfo.MemInfo.Length);
	MT_GFX_Flush((void*)pSrcVirBuf, 0, 0);
	pDecJpegInstance->pSrcPhyBuf = pSrcPhyBuf;
	pDecJpegInstance->pSrcVirBuf = pSrcVirBuf;
	pDecJpegInstance->srcBufSize = (mt_s32)Length;


	/** allocate decode handle*/

	/**
	 ** use ourself error manage function
	 **/
	pDecJpegInstance->cinfo = (j_decompress_ptr)MTGO_Malloc(sizeof(struct jpeg_decompress_struct));
	if (MT_NULL_PTR == pDecJpegInstance->cinfo)
	{
		MTGO_ERROR(MTGO_ERR_NOMEM);
		MTGO_Free(pDecJpegInstance);
		return MTGO_ERR_NOMEM;
	}
	cinfo = pDecJpegInstance->cinfo;

	cinfo->err = jpeg_std_error(&jerr);

#ifdef CONFIG_MT_FPGA_GPE
    cinfo->p_jpeg_dbg_info = &jpeg_dbg_info;    
    cinfo->p_jpeg_dbg_info->sw_dec = pSrcDesc->dbg_cfg.sw_dec;
    cinfo->p_jpeg_dbg_info->reset_test_enable = pSrcDesc->dbg_cfg.reset_test_enable;
    cinfo->p_jpeg_dbg_info->reset_test_type = pSrcDesc->dbg_cfg.reset_test_type;
    cinfo->p_jpeg_dbg_info->reset_test = pSrcDesc->dbg_cfg.reset_test;
    cinfo->p_jpeg_dbg_info->stress_test_enable= pSrcDesc->dbg_cfg.stress_test_enable;
    cinfo->p_jpeg_dbg_info->stress_test_axi_wr_last_mod= pSrcDesc->dbg_cfg.stress_test_axi_wr_last_mod;  
    cinfo->p_jpeg_dbg_info->data_check = pSrcDesc->dbg_cfg.jpeg_data_check;
    cinfo->p_jpeg_dbg_info->jpeg_set_video_endian_en = pSrcDesc->dbg_cfg.jpeg_set_video_endian_en;
#endif
	/**
	 ** create decompress
	 **/
	jpeg_create_decompress(cinfo);
  if(cinfo->global_state == DSTATE_RUNERR)
  {		
		MTGO_Free(pDecJpegInstance->cinfo);
		MTGO_Free(pDecJpegInstance);
		if(cinfo->err_state == STATE_BUF_LACK)
		{
			MTGO_ERROR(MTGO_ERR_NOMEM);
			return MTGO_ERR_NOMEM;
		}
		else
		{
			MTGO_ERROR(MTGO_ERR_INTERNAL);
			return MTGO_ERR_INTERNAL;			
		}
  }

 //  printf("MTGO_ADP_JPGCreateDecoder : <sw_dec %d reset_test_enable %d %d reset_test %d stress_test_enable %d stress_test_axi_wr_last_mod %d\n", cinfo->p_jpeg_dbg_info->sw_dec,  cinfo->p_jpeg_dbg_info->reset_test_enable ,
 //         cinfo->p_jpeg_dbg_info->reset_test_type , cinfo->p_jpeg_dbg_info->reset_test, cinfo->p_jpeg_dbg_info->stress_test_enable,cinfo->p_jpeg_dbg_info->stress_test_axi_wr_last_mod);
	/**
	 ** set stream
	 **/		 
	MT_JPEG_SetStreamPhyMem(cinfo,pDecJpegInstance->pSrcPhyBuf);
	jpeg_mem_src(cinfo,(MT_UCHAR*)pDecJpegInstance->pSrcVirBuf, pDecJpegInstance->srcBufSize);
	/**
	 ** parse file
	 **/
	if(JPEG_HEADER_OK != jpeg_read_header(cinfo, TRUE) || cinfo->global_state == DSTATE_RUNERR)
	{
		MTGO_Free(pDecJpegInstance->cinfo);
		MTGO_Free(pDecJpegInstance);
		if(cinfo->err_state == STATE_BUF_LACK)
		{
			MTGO_ERROR(MTGO_ERR_NOMEM);
			return MTGO_ERR_NOMEM;
		}
		else
		{
			MTGO_ERROR(MTGO_ERR_INTERNAL);
			return MTGO_ERR_INTERNAL;			
		}		
	}
  //if progressive, use softhuffman decode
  if(MT_TRUE != check_if_hw_huffman_support(cinfo))
  {
  	MT_JPEG_SetSoftHuff(cinfo, MT_TRUE);
  }

#ifdef CONFIG_MT_FPGA_GPE
    if(!cinfo->p_jpeg_dbg_info->sw_dec)
    {
#endif
	cinfo = pDecJpegInstance->cinfo;
	if(cinfo->jpeg_color_space == JCS_YCbCr)
	{
//	  printf("\r\n colorspace:JCS_YCbCr:%d,%d", cinfo->max_h_samp_factor, cinfo->max_v_samp_factor);
#if 0
	  if (( cinfo->max_h_samp_factor == 2) && ( cinfo->max_v_samp_factor == 2))
	  {
		cinfo->out_color_space = JCS_YUV420_SP;
	  }
	  else if (( cinfo->max_h_samp_factor == 2) && ( cinfo->max_v_samp_factor == 1))
	  {
		cinfo->out_color_space = JCS_YUV422_SP_12;
	  }
	  else if (( cinfo->max_h_samp_factor == 1) && ( cinfo->max_v_samp_factor == 1))
	  {
		cinfo->out_color_space = JCS_YUV422_SP_12; //downsample
	  }
	  else
	  {
		cinfo->out_color_space = JCS_YUV422_SP_21;
	  }
#else
    jpeg_component_info *compptr = cinfo->comp_info;
    if ((compptr[0].h_samp_factor == 2 * compptr[1].h_samp_factor) 
      && (compptr[0].v_samp_factor == 2 * compptr[1].v_samp_factor))
    {
      cinfo->out_color_space = JCS_YUV420_SP;
    }
    else if ((compptr[0].h_samp_factor == 2 * compptr[1].h_samp_factor) 
      && (compptr[0].v_samp_factor == compptr[1].v_samp_factor))
    {
#ifdef CONFIG_MT_FPGA_GPE
        if(cinfo->p_jpeg_dbg_info->stress_test_enable)
        {
            cinfo->p_jpeg_dbg_info->pis_ds_en_rand = RANDOM_DATA(0,1);
        }
       if(cinfo->p_jpeg_dbg_info->stress_test_enable && (cinfo->p_jpeg_dbg_info->pis_ds_en_rand== 1))
       {
            cinfo->out_color_space = JCS_YUV420_SP;
            
            printf("<%s>  : <%d> : pis_ds_en_rand %d \n", __FUNCTION__, __LINE__,  cinfo->p_jpeg_dbg_info->pis_ds_en_rand );
       }
       else
#endif
        {
            cinfo->out_color_space = JCS_YUV422_SP_12;    //YUV422H2            
        }
    }
    else if ((compptr[0].h_samp_factor == compptr[1].h_samp_factor) 
      && (compptr[0].v_samp_factor == compptr[1].v_samp_factor))
    {
#ifdef CONFIG_MT_FPGA_GPE
      if(cinfo->p_jpeg_dbg_info->stress_test_enable)
       {
            cinfo->p_jpeg_dbg_info->pis_ds_en_rand = RANDOM_DATA(0,2);
       }
       if(cinfo->p_jpeg_dbg_info->stress_test_enable && (cinfo->p_jpeg_dbg_info->pis_ds_en_rand== 1))
       {
            cinfo->out_color_space = JCS_YUV420_SP;            
            printf("<%s>  : <%d> : pis_ds_en_rand %d \n", __FUNCTION__, __LINE__,  cinfo->p_jpeg_dbg_info->pis_ds_en_rand );
       }
       else  if(cinfo->p_jpeg_dbg_info->stress_test_enable && (cinfo->p_jpeg_dbg_info->pis_ds_en_rand== 2))
       {
            cinfo->out_color_space = JCS_YUV422_SP_21;            
            printf("<%s>  : <%d> : pis_ds_en_rand %d \n", __FUNCTION__, __LINE__,  cinfo->p_jpeg_dbg_info->pis_ds_en_rand );
       }
       else
#endif
       {
            cinfo->out_color_space = JCS_YUV422_SP_12; //downsample   //YUV444            
       }
    }
    else
    {
#ifdef CONFIG_MT_FPGA_GPE
        if(cinfo->p_jpeg_dbg_info->stress_test_enable)
        {
            cinfo->p_jpeg_dbg_info->pis_ds_en_rand = RANDOM_DATA(0,1);
        }
       if(cinfo->p_jpeg_dbg_info->stress_test_enable && (cinfo->p_jpeg_dbg_info->pis_ds_en_rand== 1))
       {
            cinfo->out_color_space = JCS_YUV420_SP;            
            printf("<%s>  : <%d> : pis_ds_en_rand %d \n", __FUNCTION__, __LINE__,  cinfo->p_jpeg_dbg_info->pis_ds_en_rand );
       }
       else
#endif
        {
            cinfo->out_color_space = JCS_YUV422_SP_21;   //YUV422V2            
        }
    }

#endif
	}
	else if(cinfo->jpeg_color_space == JCS_RGB)
	{	 
	  cinfo->out_color_space = JCS_ARGB_8888;
	}
	else if((cinfo->jpeg_color_space == JCS_CMYK) || (cinfo->jpeg_color_space == JCS_YCCK))
	{	 
//	  printf("\r\n colorspace:JCS_CMYK:%d", cinfo->jpeg_color_space); 
	  cinfo->out_color_space = JCS_CMYK;
	} 
	else if(cinfo->jpeg_color_space == JCS_GRAYSCALE)
	{
	  cinfo->out_color_space = JCS_YUV420_SP;
	}
	else
	{
		MT_ERR_MTGO("\n not support fmt");
		
		MTGO_Free(pDecJpegInstance->cinfo);
		MTGO_Free(pDecJpegInstance);
		return MTGO_ERR_UNSUPPORTED;
	}
#ifdef CONFIG_MT_FPGA_GPE
	}
#endif
	*pJpegDec = (DEC_HANDLE)pDecJpegInstance;

	return MT_SUCCESS;

}

mt_s32 MTGO_ADP_JPGDestroyDecoder(DEC_HANDLE JpegDec)
{
    DEC_JPEGINSTANCE_S *pDecJpegInstance;

    /** paramter check */
    pDecJpegInstance = (DEC_JPEGINSTANCE_S *)JpegDec;
	/**
	 ** destory decode
	 **/
	jpeg_destroy_decompress(pDecJpegInstance->cinfo);

	if(0 != pDecJpegInstance->pSrcPhyBuf)
	{
		MT_GFX_Unmap(pDecJpegInstance->pSrcVirBuf);	  
		MT_GFX_FreeMem(pDecJpegInstance->pSrcPhyBuf);	
		pDecJpegInstance->pSrcPhyBuf = 0;
	}
	MTGO_Free(pDecJpegInstance->cinfo);
    /** destroy decode instance*/
    MTGO_Free(pDecJpegInstance);

    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_JPGResetDecoder(DEC_HANDLE JpegDec)
{
    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_JPGDecCommInfo(DEC_HANDLE JpegDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo)
{
    DEC_JPEGINSTANCE_S *pDecJpegInstance;
	j_decompress_ptr cinfo = NULL;

    /** check parameter*/
    if (MT_NULL_PTR == pPrimaryInfo)
    {
        MTGO_ERROR((MT_U32)MT_NULL_PTR);
        return MT_NULL_PTR;
    }

    /** get decode instance*/
    pDecJpegInstance = (DEC_JPEGINSTANCE_S *)JpegDec;
	cinfo = pDecJpegInstance->cinfo;
	
	pPrimaryInfo->ImgType = MTGO_DEC_IMGTYPE_JPEG;
	pPrimaryInfo->Count = 1;
	pPrimaryInfo->BGColor = 0;
	pPrimaryInfo->IsHaveBGColor = MT_FALSE;
	pPrimaryInfo->ScrWidth = cinfo->image_width;
	pPrimaryInfo->ScrHeight = cinfo->image_height;

    return MT_SUCCESS;

}

mt_s32 MTGO_ADP_JPGDecImgInfo(DEC_HANDLE JpegDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo)
{
    DEC_JPEGINSTANCE_S *pDecJpegInstance;
	j_decompress_ptr cinfo = NULL;
	
    /** check parameter*/
    if (MT_NULL_PTR == pImgInfo)
    {
        MTGO_ERROR((MT_U32)MT_NULL_PTR);
        return MT_NULL_PTR;
    }

    /** get current decode instance*/
    pDecJpegInstance = (DEC_JPEGINSTANCE_S *)JpegDec;
	cinfo = pDecJpegInstance->cinfo;

	switch(cinfo->out_color_space)
	{
	case JCS_YUV420_SP:
		pImgInfo->Format = MTGO_PF_YUV420;
		break;
	case JCS_YUV422_SP_21:
		pImgInfo->Format = MTGO_PF_YUV422_V;
		break;
	case JCS_YUV422_SP_12:
		pImgInfo->Format = MTGO_PF_YUV422;
		break;
	case JCS_ARGB_8888:
		pImgInfo->Format = MTGO_PF_8888;
		break;
	case JCS_CMYK:
		pImgInfo->Format = MTGO_PF_SP_CMYK;
		break;
#ifdef CONFIG_MT_FPGA_GPE
	case JCS_RGB:
		pImgInfo->Format = MTGO_PF_8888;
        break;
#endif
	default:
		break;
	}	
	pImgInfo->Width = cinfo->image_width;
	pImgInfo->Height = cinfo->image_height;
	pImgInfo->OffSetX = 0;
	pImgInfo->OffSetY = 0;
	pImgInfo->DelayTime = 0;
	pImgInfo->DisposalMethod = 0;
	pImgInfo->IsHaveKey = MT_FALSE;
	pImgInfo->Key = 0;
	pImgInfo->Alpha = 255;

    return MT_SUCCESS;

}
#ifdef CONFIG_MT_FPGA_GPE
static void pic_chroma_ds(int *p_src, int *p_dst, int ds_vert, int ds_hori,
int scale_num, int ds_mode, int src_width, int dst_width, int dst_height)
{
    int i, j, temp;
    if ((ds_hori == 1) && (ds_vert== 1))
    {
        for (i=0;i<dst_height;i++)
        {
            for (j=0;j<dst_width;j++)
            {
                if ((ds_mode == 0) || (scale_num == 8))
                {
                    *(p_dst+i*dst_width+j) = *(p_src+i*2*src_width+j*2);
                }
                else
                {
                    temp=(*(p_src+i*2*src_width+j*2))+(*(p_src+i*2*src_width+j*2+1))+
                        (*(p_src+(i*2+1)*src_width+j*2))+(*(p_src+(i*2+1)*src_width+j*2+1));
                    *(p_dst+i*dst_width+j) = (temp>>2)+((temp>>1)&0x1);
                }
            }
        }
    }
    else if (ds_hori == 1)
    {
        for (i=0;i<dst_height;i++)
        {
            for (j=0;j<dst_width;j++)
            {
                if ((ds_mode == 0) || (scale_num == 8))
                {
                    *(p_dst+i*dst_width+j) = *(p_src+i*src_width+j*2);
                }
                else
                {
                    temp=(*(p_src+i*src_width+j*2))+(*(p_src+i*src_width+j*2+1));
                    *(p_dst+i*dst_width+j) = (temp>>1)+(temp&0x1);
                }
            }
        }
    }
    else if (ds_vert == 1)
    {
        for (i=0;i<dst_height;i++)
        {
            for (j=0;j<dst_width;j++)
            {
                if ((ds_mode == 0) || (scale_num == 8))
                {
                    *(p_dst+i*dst_width+j) = *(p_src+i*2*dst_width+j);
                }
                else
                {
                    temp=(*(p_src+(i*2)*dst_width+j))+(*(p_src+(i*2+1)*dst_width+j));
                    *(p_dst+i*dst_width+j) = (temp>>1)+(temp&0x1);
                }
            }
        }
    }
}

static void pic_rotate(int *p_src, int *p_dst, int rot_mode, int src_width, int src_height)
{
    int i, j;
    if (rot_mode==1)
    {
        for(i = 0; i < src_width; i++)
        {
            for(j = 0; j < src_height; j++)
            {
                p_dst[i * src_height + j] = p_src[j * src_width + src_width - 1 - i];
            }
        }
    }
    else if(rot_mode==2)
    {
        for(i = 0; i < src_height; i++)
        {
            for(j = 0; j < src_width; j++)
            {
                p_dst[i * src_width + j] =
                    p_src[(src_height - 1 - i) * src_width + src_width - 1 - j];
            }
        }
    }
    else if(rot_mode==3)
    {
        for(i = 0; i < src_width; i++)
        {
            for(j = 0; j < src_height; j++)
            {
                p_dst[i * src_height + j] = p_src[(src_height - 1 - j) * src_width + i];
            }
        }
    }
}

#define CLIP(x, max) ((x > max) ? (max) : (x))
static void pic_clip(u32 clip_x0, u32 clip_y0, u32 clip_x1, u32 clip_y1, u32 width, u32 height, u32 *p_buf)
{
    u32 i = 0, j = 0;
    MT_BOOL clip_value_en = 0;
    printf("\r\nclip_x0: %d, clip_y0: %d, clip_x1: %d, clip_y1: %d, width:%d, height:%d clip_value_en: %d",
        clip_x0, clip_y0, clip_x1, clip_y1, width, height, clip_value_en);
    if((clip_x0 != 0) || (clip_y0 != 0) || (clip_x1 != width - 1) || (clip_y1 != height - 1))
    {
        for(i = 0; i < height; i++)
        {
            for(j = 0; j < width; j++)
            {
                if((j < clip_x0) || (j > clip_x1)
                    || (i < clip_y0) || (i > clip_y1))
                {
                    *(p_buf + (i * width) + j) = 0;//CLIP_FILL_VAL;
                }
                else
                {
                    if(clip_value_en)
                        *(p_buf + (i * width) + j) =
                        CLIP(*(p_buf + (i * width) + j), 0); // usr_cfg.clip_value_max set to 0
                }
            }
        }
    }
}


static void pic_ref_comparison(DEC_JPEGINSTANCE_S *pDecJpegInstance, u32 stride_y, u32 stride_ch, J_COLOR_SPACE colorspace)
{
    j_decompress_ptr cinfo = NULL;
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = NULL;  
    mt_char *p_pic_dst_y = NULL;
    mt_char *p_pic_dst_uv = NULL;

  mt_char  hw_dec_data = 0, sw_dec_data = 0;
  u32 i = 0, j = 0;
  u32 error_cnt = 0;
  u32 sw_w_y = 0, sw_h_y = 0;
  u32 sw_w_uv = 0, sw_h_uv = 0;
  u32 *y_img_rot = (u32 *)img_rot[0];
  u32 *u_img_rot = (u32 *)img_rot[1];
  u32 *v_img_rot = (u32 *)img_rot[2];
  u32 *k_img_rot = (u32 *)img_rot[3];

  sw_w_y = sw_width[0];
  sw_h_y = sw_height[0];
  sw_w_uv = sw_width[1];
  sw_h_uv = sw_height[1];

   cinfo = pDecJpegInstance->cinfo;
    pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

  p_pic_dst_y = pJpegHandle->stMiddleSurface.pMiddleVir[0];
  p_pic_dst_uv =  pJpegHandle->stMiddleSurface.pMiddleVir[1];

  
  if(colorspace == JCS_YCbCr)
  {
      printf("\r\n sw_w_y:%d, sw_h_y:%d, sw_w_uv:%d, sw_h_uv:%d",
        sw_w_y, sw_h_y, sw_w_uv, sw_h_uv);
   

      for(i = 0; i < sw_h_y; i++)
      {
        for(j = 0; j < sw_w_y; j++)
        {

            hw_dec_data = p_pic_dst_y[i * stride_y + j];
            sw_dec_data = y_img_rot[i * sw_w_y + j];

            if (hw_dec_data != sw_dec_data)
            {                
                if(error_cnt == 1)
                {
                      printf("\r\nxxxxxxxxxxxxx Y mismatched, i: %d, j: %d, \
                        hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                        i, j, hw_dec_data, sw_dec_data);
                }
                error_cnt++;
            }
        }
      }
      if (error_cnt == 0)
      {
        printf("\r\n#################### Y matched OK\n");
      }
      else
      {
          printf("\r\n#################### Y mismatched wrong(%d)\n", error_cnt);
      }
      error_cnt = 0;
      for(i = 0; i < sw_h_uv; i++)
      {

        for(j = 0; j < sw_w_uv; j++)
        {
            hw_dec_data = p_pic_dst_uv[i * stride_ch + j * 2 + 1];
            sw_dec_data = u_img_rot[i * sw_w_uv + j];

            if (hw_dec_data != sw_dec_data)
            {
                if(error_cnt == 1)
                {
                  printf("\r\nxxxxxxxxxxxxx U mismatched, i: %d, j: %d, \
                    hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                    i, j, hw_dec_data, sw_dec_data);
                }
                error_cnt++;
            }
        }
      }
        if (error_cnt == 0)
      {
        printf("\r\n#################### U matched OK\n");
      }
      else
      {
          printf("\r\n#################### U mismatched wrong(%d)\n", error_cnt);
      }
      error_cnt = 0;
      for(i = 0; i < sw_h_uv; i++)
      {
        for(j = 0; j < sw_w_uv; j++)
        {
            hw_dec_data = p_pic_dst_uv[i * stride_ch + j * 2];
            sw_dec_data = v_img_rot[i * sw_w_uv + j];
            if (hw_dec_data != sw_dec_data)
            {                
               if(error_cnt == 1)
                {
                  printf("\r\nxxxxxxxxxxxxx V mismatched, i: %d, j: %d, \
                    hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                    i, j, hw_dec_data, sw_dec_data);
                }
              error_cnt++;
            }
        }
      }

      if (error_cnt == 0)
      {
        printf("\r\n#################### V matched OK\n");
      }
      else
          printf("\r\n#################### V mismatched wrong(%d)\n", error_cnt);
  }
  else if(colorspace == JCS_RGB)
  {
      error_cnt = 0;
      u32 hw_r = 0;
      u32 hw_g = 0;
      u32 hw_b = 0;

      for(i = 0; i < sw_h_y; i++)
      {
        for(j = 0; j < sw_w_y; j++)
        {
            hw_dec_data = p_pic_dst_y[i * stride_y + j * 4 + 3];
            if((hw_dec_data != 0xff) && (hw_dec_data != 0x80))
            {
                printf("\r\nxxxxxxxxxxxxx A mismatched:0x%02x\n", hw_dec_data);
                error_cnt++;
            }
          
                    hw_r = p_pic_dst_y[i * stride_y + j * 4 + 2];
                    hw_g = p_pic_dst_y[i * stride_y + j * 4 + 1];
                    hw_b = p_pic_dst_y[i * stride_y + j * 4 + 0];

            hw_dec_data = hw_r;
            sw_dec_data = y_img_rot[i * sw_w_y + j];
            if (hw_dec_data != sw_dec_data)
            {
                if(error_cnt == 1)
                {
                    printf("\r\nxxxxxxxxxxxxx R mismatched, i: %d, j: %d, \
                        hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                        i, j, hw_dec_data, sw_dec_data);
                }
              error_cnt++;
            }
            hw_dec_data = hw_g;
            sw_dec_data = u_img_rot[i * sw_w_y + j];
            if (hw_dec_data != sw_dec_data)
            {
                if(error_cnt == 1)
                {
                      printf("\r\nxxxxxxxxxxxxx G mismatched, i: %d, j: %d, \
                        hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                        i, j, hw_dec_data, sw_dec_data);
                }
              error_cnt++;
            }
            hw_dec_data = hw_b;
            sw_dec_data = v_img_rot[i * sw_w_y + j];
            if (hw_dec_data != sw_dec_data)
            {
                 if(error_cnt == 1)
                    {
                  printf("\r\nxxxxxxxxxxxxx B mismatched, i: %d, j: %d, \
                    hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                    i, j, hw_dec_data, sw_dec_data);
                    }
                  error_cnt++;
            }
        }
      }
      if (error_cnt == 0)
      {
        printf("\r\n#################### ARGB matched OK\n");
      }
      else
         printf("\r\n#################### ARGB mismatched wrong(%d)\n", error_cnt);
  }
else if((colorspace == JCS_CMYK) || (colorspace == JCS_YCCK))
  {
      error_cnt = 0;
      u32 hw_c = 0;
      u32 hw_m = 0;
      u32 hw_y = 0;
      u32 hw_k = 0;
       printf("<%s> : <%d> : sw_h_y: %d sw_w_y : %d\n", __FUNCTION__, __LINE__, sw_h_y, sw_w_y);
      for(i = 0; i < sw_h_y; i++)
      {
        for(j = 0; j < sw_w_y; j++)
        {
                    hw_c = p_pic_dst_y[i * stride_y + j * 2 + 1];
                    hw_m = p_pic_dst_y[i * stride_y + j * 2];
                    hw_y = p_pic_dst_uv[i * stride_ch + j * 2 + 1];
                    hw_k = p_pic_dst_uv[i * stride_ch + j * 2];

            hw_dec_data = hw_c;
            sw_dec_data = y_img_rot[i * sw_w_y + j];
            if (hw_dec_data != sw_dec_data)
            {
                if(error_cnt == 1)
                {
                  printf("\r\nxxxxxxxxxxxxx C mismatched, i: %d, j: %d, \
                    hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                    i, j, hw_dec_data, sw_dec_data);
                }
                error_cnt++;
            }
            hw_dec_data = hw_m;
            sw_dec_data = u_img_rot[i * sw_w_y + j];
            if (hw_dec_data != sw_dec_data)
            {
                 if(error_cnt == 1)
                {
                  printf("\r\nxxxxxxxxxxxxx M mismatched, i: %d, j: %d, \
                    hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                    i, j, hw_dec_data, sw_dec_data);
                }
                error_cnt++;
            }
            hw_dec_data = hw_y;
            sw_dec_data = v_img_rot[i * sw_w_y + j];
            if (hw_dec_data != sw_dec_data)
            {
                if(error_cnt == 1)
                {
                  printf("\r\nxxxxxxxxxxxxx Y mismatched, i: %d, j: %d, \
                    hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                    i, j, hw_dec_data, sw_dec_data);
                }
              error_cnt++;
            }
            hw_dec_data = hw_k;
            sw_dec_data = k_img_rot[i * sw_w_y + j];
            if (hw_dec_data != sw_dec_data)
            {
                if(error_cnt == 1)
                {
                    printf("\r\nxxxxxxxxxxxxx K mismatched, i: %d, j: %d, \
                        hw_dec_data: 0x%x != sw_dec_data: 0x%x\n",
                        i, j, hw_dec_data, sw_dec_data);
                }
                error_cnt++;
            }
        }
      }
      if (error_cnt == 0)
      {
        printf("\r\n#################### CMYK matched OK\n");
      }
      else
        printf("\r\n#################### CMYK mismatched Wrong<%d>\n", error_cnt);
  }
}
#endif
mt_s32 MTGO_ADP_JPGDecImgData(DEC_HANDLE JpegDec, mt_u32 Index, MTGO_SURFACE_S *pSurface)
{
    MT_S32 Ret = 0;
    DEC_JPEGINSTANCE_S *pDecJpegInstance;
    MT_PIXELDATA pData;
	MT_BOOL bHardDec;
	j_decompress_ptr cinfo = NULL;
	MT_JPEG_INFO_S stJpegInfo;
	MTGO_PF_E PixelFormat = 0;
	MT_JPEG_SURFACE_DESCRIPTION_S stSurfaceDesc;
	MT_U32 stride = 0;
	mt_s32 Width;
	mt_s32 Height;
	MT_U32 XScale, YScale;
	MTGO_DEC_ROTATE_E rot_mod;
       
#ifdef CONFIG_MT_FPGA_GPE
    u32 w_mcu_aligned[4] = {0};
    u32 h_mcu_aligned[4] = {0};
    u32 dst_width[4] = {0};
    u32 dst_height[4] = {0};
    u32 ds_width[4] = {0};
    u32 ds_height[4] = {0};
    u32 ci = 0, ci_num = 0;
    jpeg_component_info *compptr = NULL;
    int *img_ds[4] = {NULL};
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = NULL;    
    u32 clip_x0 = 0, clip_y0 = 0, clip_x1 = 0, clip_y1 = 0;
    JSAMPROW row_pointer[1];             // STEP 6: ReadScan 
    mt_char * ptr = NULL;
    phys_addr_t pPhyAddr = 0;
    mt_u32 WStride0 = 0, WStride1 = 0, HStride0 = 0, HStride1 = 0;
    mt_u32 Size = 0;    
    mt_char *pData_tmp = NULL;
#endif


	
	pDecJpegInstance = (DEC_JPEGINSTANCE_S *)JpegDec;
	cinfo = pDecJpegInstance->cinfo;
#ifdef CONFIG_MT_FPGA_GPE
    pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
#endif
    /**check parameter*/
    if (MT_NULL_PTR == pSurface)
    {
        MTGO_ERROR((MT_U32)MT_NULL_PTR);
        return MT_NULL_PTR;
    }
	if(cinfo->global_state == DSTATE_RUNERR)
	{	
		return MT_FAILURE;
	}
	MT_GO_GetSurfaceSize((mt_handle)pSurface, &Width, &Height);
	MT_JPEG_GetRotate(cinfo, (MT_JPEG_ROT_MOD_E *)(&rot_mod));

	if((rot_mod == MTGO_DEC_ROTATE_90) || (rot_mod == MTGO_DEC_ROTATE_270))	
	{
		//get down scale ratio
		ADP_JPEGCALSCALE((mt_s32)cinfo->image_height, Width, XScale);
		ADP_JPEGCALSCALE((mt_s32)cinfo->image_width, Height, YScale);

	}
	else
	{
		//get down scale ratio
		ADP_JPEGCALSCALE((mt_s32)cinfo->image_width, Width, XScale);
		ADP_JPEGCALSCALE((mt_s32)cinfo->image_height, Height, YScale);

	}

    if (YScale > XScale)
    {
        XScale = YScale;
    }

//	printf("\r\n surface %d,%d, image:%d,%d,xscale:%d,yscale:%d", Width, Height, cinfo->image_width, cinfo->image_height,XScale,YScale);
	cinfo->scale_num = 1;
	cinfo->scale_denom = (mt_u32)(1 << XScale);
	
	stJpegInfo.bOutInfo = MT_TRUE;
	MT_JPEG_GetJpegInfo(cinfo, &stJpegInfo);		
	memset(&stSurfaceDesc,0,sizeof(MT_JPEG_SURFACE_DESCRIPTION_S));	
	MT_GO_LockSurface((mt_handle)pSurface, pData,MT_TRUE);
	stride = pData[0].Pitch;

	if(stride < stJpegInfo.u32OutStride[0])
	{
		MT_ERR_MTGO("\n surface pitch less than min pitch: line : %d %d %d\n",__LINE__,  stride,  stJpegInfo.u32OutStride[0]);
		return MT_FAILURE;
	}
	stSurfaceDesc.stOutSurface.pOutPhy[0] = pData[0].pPhyData;
	stSurfaceDesc.stOutSurface.pOutVir[0] = pData[0].pData;
	memset(stSurfaceDesc.stOutSurface.pOutVir[0],0,stride * stJpegInfo.u32Height[0]);
//	printf("\r\n flush 0x%08x", stSurfaceDesc.stOutSurface.pOutPhy[0]);
//	MT_GFX_Flush((MT_U32)stSurfaceDesc.stOutSurface.pOutPhy[0], 0, 0);

	MT_GO_GetSurfacePixelFormat((mt_handle)pSurface, &PixelFormat);
	
	if((PixelFormat == MTGO_PF_YUV420) || (PixelFormat == MTGO_PF_YUV422_V)
		|| (PixelFormat == MTGO_PF_YUV422) || (PixelFormat == MTGO_PF_SP_CMYK))
	{
		stride = pData[1].Pitch;
		if(stride < stJpegInfo.u32OutStride[1])
		{
			MT_ERR_MTGO("\n surface pitch less than min pitch: line : %d %d %d\n",__LINE__,  stride, stJpegInfo.u32OutStride[1]);
			return MT_FAILURE;
		}

		stSurfaceDesc.stOutSurface.pOutPhy[1] = pData[1].pPhyData;
		stSurfaceDesc.stOutSurface.pOutVir[1] = pData[1].pData;	
		memset(stSurfaceDesc.stOutSurface.pOutVir[1],0,stride * stJpegInfo.u32Height[1]);
//		MT_GFX_Flush((MT_U32)stSurfaceDesc.stOutSurface.pOutPhy[1], 0, 0);
	}

#ifdef CONFIG_MT_FPGA_GPE
     compptr = cinfo->comp_info;
    if((cinfo->jpeg_color_space == JCS_YCbCr) || (cinfo->jpeg_color_space == JCS_RGB))
     {
        ci_num = 3;
     }
    else if((cinfo->jpeg_color_space == JCS_CMYK) ||(cinfo->jpeg_color_space == JCS_YCCK))
    {
        ci_num = 4;
    }
    else
        printf("\r\n xxxxcinfo->jpeg_color_space:%d not supported", cinfo->jpeg_color_space);

     if(cinfo->p_jpeg_dbg_info->sw_dec)
     {
         for(ci = 0; ci < 4; ci++)
         {
                if(img_rot[ci] != NULL)
                {
                    MTGO_Free(img_rot[ci]);
                    img_rot[ci] = NULL;            
                }
                 if(cinfo->p_jpeg_dbg_info->g_img[ci] != NULL)
                 {
                     printf("<%s>: <%d>  out of memory img_ds (%d %px)\n", __FUNCTION__, __LINE__, ci,   cinfo->p_jpeg_dbg_info->g_img[ci]);
                }
         }
        if(cinfo->comps_in_scan == 1) //non-interleaved
        {
            for(ci = 0; ci < ci_num; ci++)
            {
                if((cinfo->max_h_samp_factor == 2) && (compptr[ci].h_samp_factor == 1))
                   w_mcu_aligned[ci] = JPEG_ALIGN((cinfo->image_width + 1) / 2, 8);
                else
                   w_mcu_aligned[ci] =JPEG_ALIGN(cinfo->image_width, 8);

                if((cinfo->max_v_samp_factor == 2) && (compptr[ci].v_samp_factor == 1))
                   h_mcu_aligned[ci] =JPEG_ALIGN((cinfo->image_height + 1) / 2, 8);
                else
                   h_mcu_aligned[ci] = JPEG_ALIGN(cinfo->image_height, 8);
            }
        }
        else
        {
            for(ci = 0; ci < ci_num; ci++)
            {
                if(cinfo->max_h_samp_factor == 2)
                {
                   if(compptr[ci].h_samp_factor == 2)
                       w_mcu_aligned[ci] = JPEG_ALIGN(cinfo->image_width, 16);
                   else
                       w_mcu_aligned[ci] = JPEG_ALIGN((cinfo->image_width + 1) / 2, 8);
                }
                else
                   w_mcu_aligned[ci] = JPEG_ALIGN(cinfo->image_width, 8);

                if(cinfo->max_v_samp_factor == 2)
                {
                   if(compptr[ci].v_samp_factor == 2)
                       h_mcu_aligned[ci] = JPEG_ALIGN(cinfo->image_height, 16);
                   else
                       h_mcu_aligned[ci] = JPEG_ALIGN((cinfo->image_height + 1) / 2, 8);
                }
                else
                   h_mcu_aligned[ci] =JPEG_ALIGN(cinfo->image_height, 8);
            }
        }

        for(ci = 0; ci < ci_num; ci++)
        {
            dst_width[ci] = w_mcu_aligned[ci] * cinfo->scale_num / cinfo->scale_denom;
            dst_height[ci] = h_mcu_aligned[ci] * cinfo->scale_num / cinfo->scale_denom;
            printf("\r\n dst_width[%d]:%d, dst_height[%d]:%d", ci, dst_width[ci], ci, dst_height[ci]);
            
            cinfo->p_jpeg_dbg_info->g_output_width[ci] = dst_width[ci];
            cinfo->p_jpeg_dbg_info->g_output_height[ci] = dst_height[ci];         

            if(cinfo->p_jpeg_dbg_info->g_img[ci] != NULL)
            {
                    printf(" <%s> : out of memory(%d %px)\n", __FUNCTION__, ci, cinfo->p_jpeg_dbg_info->g_img[ci]);
            }
            cinfo->p_jpeg_dbg_info->g_img[ci] = MTGO_Malloc(sizeof(int)*(dst_width[ci] * dst_height[ci])); //MTGO_MMZ_Malloc(sizeof(int)*(dst_width[ci] * dst_height[ci], &cinfo->p_jpeg_dbg_info->g_img_phy[ci]); //(int *)malloc(sizeof(int)*(dst_width[ci] * dst_height[ci]));
            MT_ASSERT(cinfo->p_jpeg_dbg_info->g_img[ci] != NULL);
            cinfo->p_jpeg_dbg_info->g_idx[ci] = 0;
           memset(cinfo->p_jpeg_dbg_info->g_img[ci], 0x00, sizeof(int)*(dst_width[ci] * dst_height[ci]));  
        }
       }
#endif

	stSurfaceDesc.stOutSurface.bUserPhyMem = MT_TRUE;
	stSurfaceDesc.stOutSurface.u32OutStride[0] = pData[0].Pitch; 
	stSurfaceDesc.stOutSurface.u32OutStride[1] = pData[1].Pitch;
	Ret = MT_JPEG_SetOutDesc(cinfo, &stSurfaceDesc);
	if(Ret != MT_SUCCESS)
	{
		return MT_FAILURE;
	}
#ifdef CONFIG_MT_FPGA_GPE
          printf(" start decoding JPEG PIC\n");
          gettimeofday(&loop_test_start, NULL);   

#endif

	/**
	 ** start decode
	 **/
	Ret = jpeg_start_decompress(cinfo);
   if(cinfo->global_state == DSTATE_RUNERR || Ret == FALSE)
   {
		 jpeg_finish_decompress(cinfo);
		 return MT_FAILURE;
   }
	 
	MT_JPEG_IfHardDec(cinfo,&bHardDec);
	if(MT_TRUE == bHardDec)
	{
//		printf("=========================\n");
//		printf("hard dec success\n");
//		printf("=========================\n");
	}
	else
	{
	
//		printf("=========================\n");
//		printf("soft dec success\n");
//		printf("=========================\n");
	}

	if(jpeg_if_softhuff(cinfo))
	{
		 Ret = jpeg_start_soft_huff_decompress(cinfo);	
		 if(Ret  == FALSE)
         {
            jpeg_finish_decompress(cinfo);
            return MT_FAILURE;
         }
	}

	if(cinfo->global_state == DSTATE_RUNERR)
	{
		 jpeg_finish_decompress(cinfo);
		 return MT_FAILURE;
	}
	
	/**
	 ** output the decode data
	 **/
#ifdef CONFIG_MT_FPGA_GPE	
       if(cinfo->p_jpeg_dbg_info->sw_dec)
        {
            Surface_CalculateStride0(pSurface->PixelFormat, (mt_u32)pSurface->Width, (mt_u32)pSurface->Height, &WStride0, &HStride0);
            Surface_CalculateStride1(pSurface->PixelFormat, (mt_u32)pSurface->Width, (mt_u32)pSurface->Height, &WStride1, &HStride1);

            Size = (mt_u32)(HStride0 * WStride0 + HStride1 * WStride1) * cinfo->scale_denom;

            /** allocate memory  */
            ptr = MTGO_MMZ_Malloc(Size, &pPhyAddr);   
            if(ptr == NULL)
            {
                jpeg_finish_decompress(cinfo);                 
		 		return MT_FAILURE;
            }
            pData_tmp = ptr;
            memset(ptr, 0x00, Size);  
            MT_GFX_Flush((void *)ptr, 0, 0);
	}
#endif
	while (cinfo->output_scanline < cinfo->output_height) 
	{
#ifdef CONFIG_MT_FPGA_GPE
		if(cinfo->p_jpeg_dbg_info->sw_dec == 0)
			jpeg_read_scanlines(cinfo, NULL, 1);
		else
		{                
			row_pointer[0] = (JSAMPROW)&pData_tmp[(cinfo->output_height - cinfo->output_scanline - 1) * cinfo->image_width * cinfo->num_components];     
			jpeg_read_scanlines(cinfo, row_pointer, 1);    
		}
#else
		jpeg_read_scanlines(cinfo, NULL, 1);
#endif
	}


	/**
	 **这个地方是输出到连续物理内存，由用户决定是否刷cach,内部
	 **TDE转换直接转到物理内存，没有对虚拟内存做操作
	 **/
#ifdef CONFIG_MT_FPGA_GPE
	 if(cinfo->p_jpeg_dbg_info->sw_dec)
	{  
		MTGO_MMZ_Free(ptr);
		ptr = NULL;
	}
#endif
#if 0
{
	MT_U32 *ptr = pData[0].pData;
	int i;
	for(i = 0; i < 10; i++)
	{
		printf("\r\n ptr:0x%08x ", ptr[i]);
	}
}
#endif

    JPEG_GetJpegCropRect(cinfo, (MT_JPEG_RECT_S *)(&pDecJpegInstance->activeSize));

#ifdef CONFIG_MT_FPGA_GPE
    if(cinfo->p_jpeg_dbg_info->data_check && 
        (!cinfo->p_jpeg_dbg_info->reset_test_enable  
            || (cinfo->p_jpeg_dbg_info->reset_test_enable && img_rot[0] == NULL)))
   {        
        for(ci = 0; ci < ci_num; ci++)
        {
            u32 ds_hori_en = 0;
            u32 ds_vert_en = 0;

            if(cinfo->jpeg_color_space == JCS_YCbCr)
            {
                if(ci == 0)
                {
                     ds_hori_en = 0;
                     ds_vert_en = 0;
                }
                else
                {
                     ds_hori_en = (cinfo->p_jpeg_dbg_info->pic_ds & 0x1) ? 1 : 0;
                     ds_vert_en = (cinfo->p_jpeg_dbg_info->pic_ds & 0x2) ? 1 : 0;
                }
            }
            else
            {
                if((cinfo->p_jpeg_dbg_info->pic_ds & 0x1)&& (compptr[ci].h_samp_factor == 2))
                    ds_hori_en = 1;
                else
                    ds_hori_en = 0;
                 if((cinfo->p_jpeg_dbg_info->pic_ds & 0x2) && (compptr[ci].v_samp_factor == 2))
                    ds_vert_en = 1;
                else
                    ds_vert_en = 0;
            }

            if(ds_hori_en == 1)
                ds_width[ci] = ( cinfo->p_jpeg_dbg_info->g_output_width[ci] + 1) / 2;
            else
                ds_width[ci] =  cinfo->p_jpeg_dbg_info->g_output_width[ci];
            if(ds_vert_en == 1)
                ds_height[ci] = (cinfo->p_jpeg_dbg_info->g_output_height[ci] + 1) / 2;
            else
                ds_height[ci] = cinfo->p_jpeg_dbg_info->g_output_height[ci];
        
            printf("<%s> : <%d> ds_hori_en <%d %d>\n", __FUNCTION__, __LINE__,  ds_hori_en, ds_vert_en);
            if((ds_hori_en == 1) || (ds_vert_en == 1))
            {
                img_ds[ci] = (int *)MTGO_Malloc(ds_width[ci]*ds_height[ci]*sizeof(int));
                MT_ASSERT(img_ds[ci] != NULL);
                pic_chroma_ds(cinfo->p_jpeg_dbg_info->g_img[ci], img_ds[ci], ds_vert_en, ds_hori_en,
                    cinfo->scale_denom, cinfo->p_jpeg_dbg_info->ds_mode, cinfo->p_jpeg_dbg_info->g_output_width[ci], ds_width[ci], ds_height[ci]);
                MTGO_Free(cinfo->p_jpeg_dbg_info->g_img[ci]);
                cinfo->p_jpeg_dbg_info->g_img[ci] = NULL;
                
            }
            else
            {
                img_ds[ci] = cinfo->p_jpeg_dbg_info->g_img[ci];
                cinfo->p_jpeg_dbg_info->g_img[ci] = NULL;
            }
        }

        for(ci = 0; ci < ci_num; ci++)
        {
            if((ci == 0) && (cinfo->jpeg_color_space == JCS_YCbCr))
            {
                clip_x0 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x;
                clip_x1 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x;
                clip_y0 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y;
                clip_y1 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y;
            }
            else
            {
                if(((cinfo->p_jpeg_dbg_info->pic_ds & 0x1)) || ((compptr[ci].h_samp_factor == 1) && (cinfo->max_h_samp_factor == 2)))
                {
                    clip_x0 =  pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x / 2;
                    clip_x1 = (pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x + 1) / 2 - 1;
                }
                else
                {
                    clip_x0 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x;
                    clip_x1 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x;
                }
                if((cinfo->p_jpeg_dbg_info->pic_ds & 0x2) || ((compptr[ci].v_samp_factor == 1) && (cinfo->max_v_samp_factor == 2)))
                {
                    clip_y0 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y / 2;
                    clip_y1 = (pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y + 1) / 2 - 1;
                }
                else
                {
                    clip_y0 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y;
                    clip_y1 = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y;
                }
            }
            pic_clip(clip_x0, clip_y0, clip_x1, clip_y1, ds_width[ci], ds_height[ci], (u32 *)img_ds[ci]);
        }

        if(pJpegHandle->out_rot_mod ==  0)
        {
            for(ci = 0; ci < ci_num; ci++)
            {
                img_rot[ci] = img_ds[ci];
                img_ds[ci] = NULL;
                sw_width[ci] = ds_width[ci];
                sw_height[ci] = ds_height[ci];
            }            
        }
        else if (pJpegHandle->out_rot_mod <= 3)
        {
            for(ci = 0; ci < ci_num; ci++)
            {
                img_rot[ci] = (int *)MTGO_Malloc(ds_width[ci] * ds_height[ci] * sizeof(int));
                MT_ASSERT(img_rot[ci] != NULL);
                pic_rotate(img_ds[ci], img_rot[ci], pJpegHandle->out_rot_mod, ds_width[ci], ds_height[ci]);
                MTGO_Free(img_ds[ci]);
                img_ds[ci] = NULL;                
                if (pJpegHandle->out_rot_mod == 2)
                {
                    sw_width[ci] = ds_width[ci];
                    sw_height[ci] = ds_height[ci];
                }
                else if ((pJpegHandle->out_rot_mod == 1) || (pJpegHandle->out_rot_mod == 3))
                {
                    sw_width[ci] = ds_height[ci];
                    sw_height[ci] = ds_width[ci];
                }
            }            
        }        
        pic_ref_comparison(pDecJpegInstance, cinfo->p_jpeg_dbg_info->y_stride, cinfo->p_jpeg_dbg_info->uv_stride, cinfo->jpeg_color_space);

        if(!cinfo->p_jpeg_dbg_info->reset_test_enable)
        {
             for(ci = 0; ci < 4; ci++)
             {
                    if(img_rot[ci] != NULL)
                    {
                        MTGO_Free(img_rot[ci]);
                        img_rot[ci] = NULL;            
                    }
                    if(img_ds[ci] != NULL)
                    {                            
                            printf("<%s>: <%d>  out of memory img_ds (%d %px)\n", __FUNCTION__, __LINE__,  ci,   img_ds[ci]);
                    }
                     if(cinfo->p_jpeg_dbg_info->g_img[ci] != NULL)
                     {
                         printf("<%s>: <%d>  out of memory img_ds (%d %px)\n", __FUNCTION__, __LINE__, ci,   cinfo->p_jpeg_dbg_info->g_img[ci]);
                     }
             }
        }
    }
    else if(cinfo->p_jpeg_dbg_info->data_check && (cinfo->p_jpeg_dbg_info->reset_test_enable && img_rot[0] != NULL))
    {
        pic_ref_comparison(pDecJpegInstance, cinfo->p_jpeg_dbg_info->y_stride, cinfo->p_jpeg_dbg_info->uv_stride, cinfo->jpeg_color_space);            
    }
    else if(cinfo->p_jpeg_dbg_info->data_check )
     {
            printf("<%s> : <%d> : need check : Err\n", __FUNCTION__, __LINE__);
    }
#endif
    /**
     ** finish decode
     **/
    jpeg_finish_decompress(cinfo);
#ifdef CONFIG_MT_FPGA_GPE
        gettimeofday(&loop_test_end, NULL);
            total_cost_time += ((loop_test_end.tv_sec - loop_test_start.tv_sec) * 1000000 + loop_test_end.tv_usec) - loop_test_start.tv_usec;
       printf("JPEG decode finish : total_cost_time : %ld  : %ld %ld %ld %ld %ld\n",      total_cost_time,  loop_test_end.tv_sec,  loop_test_start.tv_sec, loop_test_end.tv_usec
        ,  loop_test_start.tv_usec, (1000000*(loop_test_end.tv_sec - loop_test_start.tv_sec) + loop_test_end.tv_usec) - loop_test_start.tv_usec);
#endif

	return MT_SUCCESS;
}


mt_s32 MTGO_ADP_JPGGetActualSize(DEC_HANDLE JpegDec, MT_S32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo)
{

    MTGO_DEC_IMGINFO_S ImgInfo;
    MT_U32 XScale, YScale;
    mt_s32 ret;
	MTGO_DEC_ROTATE_E rot_mod;
	j_decompress_ptr cinfo = NULL;
    DEC_JPEGINSTANCE_S *pDecJpegInstance;
	MT_JPEG_INFO_S stJpegInfo;
	
	pDecJpegInstance = (DEC_JPEGINSTANCE_S *)JpegDec;
	cinfo = pDecJpegInstance->cinfo;

    if(pSrcRect->w  == 0 || pSrcRect->h == 0)
    {
         MT_ERR_MTGO("\n pSrcRect fail : pSrcRect : wxh %d %d\n", pSrcRect->w, pSrcRect->h);
         ret =  MT_FAILURE;
         MTGO_ERROR(ret);
         return ret;
    }
	MT_JPEG_GetRotate(cinfo, (MT_JPEG_ROT_MOD_E *)(&rot_mod));
	
    ret = MTGO_ADP_JPGDecImgInfo(JpegDec, (MT_U32)Index, &ImgInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

	if((rot_mod == MTGO_DEC_ROTATE_90) || (rot_mod == MTGO_DEC_ROTATE_270))	
	{
	    /*scaling rate is the same both  for height and witdh, and the rate can be 2,4,8, send the parameter to decoder*/
	    ADP_JPEGCALSCALE(ImgInfo.Width, (mt_u32)pSrcRect->h,XScale);
	    ADP_JPEGCALSCALE(ImgInfo.Height, (mt_u32)pSrcRect->w,YScale);	
	}
	else
	{
	    /*scaling rate is the same both  for height and witdh, and the rate can be 2,4,8, send the parameter to decoder*/
	    ADP_JPEGCALSCALE(ImgInfo.Width, (mt_u32)pSrcRect->w,XScale);
	    ADP_JPEGCALSCALE(ImgInfo.Height, (mt_u32)pSrcRect->h,YScale);
	}
    if (YScale > XScale)
    {
        XScale = YScale;
    }

	cinfo->scale_num = 1;
	cinfo->scale_denom = (mt_u32)(1 << XScale);
	
	stJpegInfo.bOutInfo = MT_TRUE;
	MT_JPEG_GetJpegInfo(cinfo, &stJpegInfo);

	pSurInfo->Width = (mt_s32)stJpegInfo.u32Width[0];
	pSurInfo->Height = (mt_s32)stJpegInfo.u32Height[0];	
//    pSurInfo->Width = (MT_S32)((ImgInfo.Width  + (1 << XScale) - 1) >> XScale);
//    pSurInfo->Height = (MT_S32)((ImgInfo.Height + (1 << XScale) - 1) >> XScale);
    pSurInfo->PixelFormat = ImgInfo.Format;
    pSurInfo->Pitch[0] = 0;
    pSurInfo->Pitch[1] = 0;


	if((rot_mod == MTGO_DEC_ROTATE_90) || (rot_mod == MTGO_DEC_ROTATE_270))
	{
//	    pSurInfo->Width = (MT_S32)((ImgInfo.Height + (1 << XScale) - 1) >> XScale);		
//	    pSurInfo->Height = (MT_S32)((ImgInfo.Width  + (1 << XScale) - 1) >> XScale);	
		if(ImgInfo.Format == MTGO_PF_YUV422_V)
			pSurInfo->PixelFormat = MTGO_PF_YUV422;
		else if(ImgInfo.Format == MTGO_PF_YUV422)
			pSurInfo->PixelFormat = MTGO_PF_YUV422_V;
	}
//	printf("\r\n ~~~~~~~~~~~~~get actual size");
    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_JPGSetRotate(DEC_HANDLE JpegDec, MTGO_DEC_ROTATE_E rotate)
{
	mt_s32 ret;
	j_decompress_ptr cinfo = NULL;
    DEC_JPEGINSTANCE_S *pDecJpegInstance;
//	printf("[%s]: [%d]\n", __FUNCTION__, __LINE__);
	pDecJpegInstance = (DEC_JPEGINSTANCE_S *)JpegDec;
	cinfo = pDecJpegInstance->cinfo;	
//	printf("[%s]: [%d], cinfo:0x%08x, jpegDec:0x%08x\n", __FUNCTION__, __LINE__, cinfo, JpegDec);
	ret = MT_JPEG_SetRotate(cinfo, rotate);
	return ret;
}

mt_s32 MTGO_ADP_JPGGetCropRect(DEC_HANDLE JpegDec, MT_RECT *pCropRect)
{
    DEC_JPEGINSTANCE_S *pDecJpegInstance;
	
	pDecJpegInstance = (DEC_JPEGINSTANCE_S *)JpegDec;
	if(pDecJpegInstance == NULL)
		return MT_FAILURE;	

	pCropRect->x = pDecJpegInstance->activeSize.x;
	pCropRect->y = pDecJpegInstance->activeSize.y;
	pCropRect->w = pDecJpegInstance->activeSize.w;
	pCropRect->h = pDecJpegInstance->activeSize.h;
    return MT_SUCCESS;
}

