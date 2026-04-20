/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>

#include "mt_type.h"
#include "mt_jpeg_config.h"
#include "jpeg_hdec_adp.h"
#include "jpeg_hdec_api.h"
#include "jpeg_hdec_mem.h"
#include "jpegint.h"

/***************************** Macro Definition ******************************/


/** calculate the size according to the scale */
/** CNcomment:根据缩放比例计算大小 */
#define JPEG_ALIGNED_SCALE(x, i)  (((x) + (1 << (i)) - 1) >> (i))

/** calculate the size according to the scale */
/** CNcomment:根据缩放比例计算大小 */
#define JPEG_ROUND_UP(a,b)          ( ((a) + (b) - (1L)) / (b) )

#define RANDOM_DATA(a, b) ((rand() % (b - a + 1)) + a)



/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/


/******************************* API forward declarations *******************/


/******************************* API realization *****************************/


/*****************************************************************************
* func          : JPEG_HDEC_GetOutSize
* description	: get the out size
                  CNcomment:  获取输出大小 CNend\n
* param[in]  	: s32Ration      CNcomment: 缩放比例 CNend\n
* param[in]  	: u32InWidth     CNcomment: 输入宽度 CNend\n
* param[in]  	: u32InHeight    CNcomment: 输入高度 CNend\n
* param[out] 	: pu32OutWidth   CNcomment: 输出宽度 CNend\n
* param[out]	: pu32OutHeight  CNcomment: 输出高度 CNend\n
* retval     	: NA
* others:	 	: NA
*****************************************************************************/
static MT_VOID JPEG_HDEC_GetOutSize(const MT_U32 u32Ration,    \
                                                   const MT_U32 u32InWidth,   \
                                                   const MT_U32 u32InHeight,  \
                                                   MT_U32 *pu32OutWidth,      \
                                                   MT_U32 *pu32OutHeight)
{

       switch(u32Ration)
	   {
		     case 0:
		         *pu32OutWidth  = u32InWidth;
		         *pu32OutHeight = u32InHeight;
				 break;
			 case 1:
		         *pu32OutWidth  = (JDIMENSION)JPEG_ROUND_UP((long) u32InWidth, 2L);
		         *pu32OutHeight = (JDIMENSION)JPEG_ROUND_UP((long) u32InHeight, 2L);
				  break;
			 case 2:
				*pu32OutWidth = (JDIMENSION)JPEG_ROUND_UP((long) u32InWidth, 4L);
		         *pu32OutHeight = (JDIMENSION)JPEG_ROUND_UP((long) u32InHeight, 4L);
				 break;
	         case 3:
			 	  *pu32OutWidth = (JDIMENSION)JPEG_ROUND_UP((long) u32InWidth, 8L);
		          *pu32OutHeight = (JDIMENSION)JPEG_ROUND_UP((long) u32InHeight, 8L);  
				  break;
			 default:
			 	  break;

       }

}


/*****************************************************************************
* func			: JPEG_HDEC_GetScale
* description	: get the jpeg decode scale
				  CNcomment:  获取jpeg解码要缩放的比例 CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象	CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
static MT_VOID JPEG_HDEC_GetScale(j_decompress_ptr cinfo)
{


		MT_U32 u32Ration      = 0;
		MT_U32 u32TmpWidth    = 0;
		MT_U32 u32TmpHeight   = 0;
		MT_U32 u32TdeInWidth  = 0;
		MT_U32 u32TdeInHeight = 0;


		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		u32TmpWidth  = cinfo->image_width;
		u32TmpHeight = cinfo->image_height;

		/**
		 ** the user set scale
		 ** CNcomment: 用户设置需要的缩放比例 CNend\n
		 **/
		if(cinfo->scale_num * 8 <= cinfo->scale_denom)
		{
		    u32Ration = JPEG_SCALEDOWN_8;
		}
		else if(cinfo->scale_num * 4 <= cinfo->scale_denom)
		{
		    u32Ration = JPEG_SCALEDOWN_4;
		}
		else if(cinfo->scale_num * 2 <= cinfo->scale_denom)
		{
			u32Ration = JPEG_SCALEDOWN_2;
		}
		else if(cinfo->scale_num == cinfo->scale_denom)
		{
		     u32Ration = JPEG_SCALEDOWN_1;
		}
		else
		{
		     u32Ration = JPEG_SCALEDOWN_BUTT;
		}

#ifdef CONFIG_JPEG_HARDDEC2ARGB

              if(cinfo->jpeg_color_space == JCS_RGB)
		{
		    pJpegHandle->bDecARGB	=  MT_TRUE;
		}
#endif

              JPEG_HDEC_GetOutSize(u32Ration,u32TmpWidth,u32TmpHeight,&u32TdeInWidth,&u32TdeInHeight);
		/**
		** the output size
		** CNcomment: 输出大小 CNend\n
		**/
		cinfo->output_width  = u32TdeInWidth;
		cinfo->output_height = u32TdeInHeight;              
		/**
		** set to hard register scale value
		** CNcomment: 配给硬件的缩放比例大小 CNend\n
		**/
		pJpegHandle->u32ScalRation = u32Ration;
				
}



static mt_u32 mcu_aligned(mt_u32 size, mt_u32 align_size)
{
    mt_u32 size_aligned = (size + align_size - 1) & (~(align_size - 1));
    return size_aligned;
}

static void get_mcu_aligned_size(mt_u32 *w_mcu_aligned, mt_u32 *h_mcu_aligned, mt_u32 ci, j_decompress_ptr p_cinfo)
{
  jpeg_component_info *compptr = p_cinfo->comp_info;

  if((p_cinfo->jpeg_color_space == JCS_CMYK) || (p_cinfo->jpeg_color_space == JCS_YCCK))
  {
    if(p_cinfo->max_h_samp_factor == 2)
        *w_mcu_aligned = mcu_aligned(p_cinfo->image_width, 16);  
    else
        *w_mcu_aligned = mcu_aligned(p_cinfo->image_width, 8);  
    if(p_cinfo->max_v_samp_factor == 2)
        *h_mcu_aligned = mcu_aligned(p_cinfo->image_height, 16);  
    else
        *h_mcu_aligned = mcu_aligned(p_cinfo->image_height, 8);            
  }
  else
  {
	  if(p_cinfo->comps_in_scan == 1) //non-interleaved
	  {
	     if((p_cinfo->max_h_samp_factor == 2) && (compptr[ci].h_samp_factor == 1))
	         *w_mcu_aligned = mcu_aligned((p_cinfo->image_width + 1) / 2, 8); 
	     else
	         *w_mcu_aligned = mcu_aligned(p_cinfo->image_width, 8);  
	  
	     if((p_cinfo->max_v_samp_factor == 2) && (compptr[ci].v_samp_factor == 1))
	         *h_mcu_aligned = mcu_aligned((p_cinfo->image_height + 1) / 2, 8); 
	     else
	         *h_mcu_aligned = mcu_aligned(p_cinfo->image_height, 8);                                  
	  }
	  else
	  {
	     if(p_cinfo->max_h_samp_factor == 2) 
	     {
	         if(compptr[ci].h_samp_factor == 2)
	             *w_mcu_aligned = mcu_aligned(p_cinfo->image_width, 16);   
	         else
	             *w_mcu_aligned = mcu_aligned((p_cinfo->image_width + 1) / 2, 8); 
	     }
	     else
	         *w_mcu_aligned = mcu_aligned(p_cinfo->image_width, 8);  
	  
	     if(p_cinfo->max_v_samp_factor == 2) 
	     {
	         if(compptr[ci].v_samp_factor == 2)
	             *h_mcu_aligned = mcu_aligned(p_cinfo->image_height, 16);   
	         else
	             *h_mcu_aligned = mcu_aligned((p_cinfo->image_height + 1) / 2, 8); 
	     }
	     else
	         *h_mcu_aligned = mcu_aligned(p_cinfo->image_height, 8);                              
	  }        
  }
}

/*****************************************************************************
* func			: JPEG_HDEC_GetImagInfo
* description	: get jpeg picture information
				  CNcomment:  获取图片信息 CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象	CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_GetImagInfo(j_decompress_ptr cinfo)
{

    jpeg_component_info *compptr = cinfo->comp_info;
    MT_U32 dst_width = 0, dst_height = 0;
    MT_U32 dst_width_uv = 0, dst_height_uv = 0;  
    MT_U32 active_width = 0, active_height = 0;
    MT_U32 w_mcu_aligned[4] = {0};
    MT_U32 h_mcu_aligned[4] = {0};  
    MT_U8 pic_in_type = 0, pic_out_type = 0, pic_out_scale = 0;
    MT_U8 pic_out_rot = 0, pic_ds = 0;
    MT_U32 stride_y_min = 0, stride_uv_min = 0;
    MT_U32 i = 0;
		
    JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    if(MT_TRUE == pJpegHandle->stJpegSofInfo.bCalcSize)
    {
        return;
    }
    /**
     **  if out yuvsp,the output mem has two types,one from user, the other from inner. 
     **  if out others, we should alloc the yuvsp mem.
     ** CNcomment: 要是输出yuvsp 和 硬件解码输出ARGB，是否需要分配硬件解码的中间buffer有两种情况，
     ** 		   要是输出其它格式则需要分配中间buffer  CNend\n
     **/
    if(  (JCS_YUV400_SP	   == cinfo->out_color_space)
       ||(JCS_YUV420_SP    == cinfo->out_color_space)
       ||(JCS_YUV422_SP_12 == cinfo->out_color_space)
       ||(JCS_YUV422_SP_21 == cinfo->out_color_space)
       ||(JCS_YUV444_SP	   == cinfo->out_color_space)
       ||(JCS_CMYK	   == cinfo->out_color_space))
    {
    	pJpegHandle->bOutYCbCrSP   = MT_TRUE;
    }
    else
    {
    	pJpegHandle->bOutYCbCrSP    = MT_FALSE;
    }

    /**
     ** this function call should after check the bOutYCbCrSP
     ** CNcomment:这个要在上面bOutYCbCrSP判断之后调用 CNend\n
     **/
    JPEG_HDEC_GetScale(cinfo);

      for(i = 0; i < (mt_u32)cinfo->num_components; i++)
      {
      	  if(pJpegHandle->bSoftHuffDec == MT_TRUE)
    	  {
    		  w_mcu_aligned[i] = (mt_u32)(jround_up((long) compptr[i].width_in_blocks,
    					(long) compptr[i].h_samp_factor) * 8);
    		  h_mcu_aligned[i] = (mt_u32)(jround_up((long) compptr[i].height_in_blocks,
    			(long) compptr[i].v_samp_factor) * 8);
    	  }  
    	  else
    	  {
    		get_mcu_aligned_size(&w_mcu_aligned[i], &h_mcu_aligned[i], i, cinfo);

    	  }
    //			  printf("\r\n~~~~~~~~~~%d,%d,%d, %d,w_mcu_aligned[%d]:%d, h_mcu_aligned[%d]:%d", compptr[i].width_in_blocks,
    //			  	compptr[i].height_in_blocks, compptr[i].h_samp_factor, compptr[i].v_samp_factor,i,w_mcu_aligned[i], i, h_mcu_aligned[i]);
      }

    //fix bug 99132
    if((cinfo->jpeg_color_space == JCS_GRAYSCALE))
    {
      w_mcu_aligned[1] = w_mcu_aligned[0] / 2;
      h_mcu_aligned[1] = h_mcu_aligned[0] / 2;
    }

        
        //pic_in_type
      if(cinfo->jpeg_color_space == JCS_YCbCr)
      {
#if 0      
        if (( cinfo->max_h_samp_factor == 2) && ( cinfo->max_v_samp_factor == 2))
        {
          pic_in_type = YUV420;
        }
        else if (( cinfo->max_h_samp_factor == 2) && ( cinfo->max_v_samp_factor == 1))
        {
          pic_in_type = YUV422H2;
        }
        else if (( cinfo->max_h_samp_factor == 1) && ( cinfo->max_v_samp_factor == 1))
        {
          pic_in_type = YUV444;
        }
        else
        {
          pic_in_type = YUV422V2;
        }
#else
        if ((compptr[0].h_samp_factor == 2 * compptr[1].h_samp_factor) 
          && (compptr[0].v_samp_factor == 2 * compptr[1].v_samp_factor))
        {
          pic_in_type = YUV420;
        }
        else if ((compptr[0].h_samp_factor == 2 * compptr[1].h_samp_factor) 
          && (compptr[0].v_samp_factor == compptr[1].v_samp_factor))
        {
          pic_in_type = YUV422H2;
        }
        else if ((compptr[0].h_samp_factor == compptr[1].h_samp_factor) 
          && (compptr[0].v_samp_factor == compptr[1].v_samp_factor))
        {
          pic_in_type = YUV444;
        }
        else
        {
          pic_in_type = YUV422V2;
        }
#endif
      }
      else if(cinfo->jpeg_color_space == JCS_RGB)
      {    
        pic_in_type = RGB_R;
      }
      else if((cinfo->jpeg_color_space == JCS_CMYK) || (cinfo->jpeg_color_space == JCS_YCCK))
      {    
        pic_in_type = CMYK_C;
      }  
	  else if(cinfo->jpeg_color_space == JCS_GRAYSCALE)
	  {
		pic_in_type = YUV400;
	  }

      //pic_out_type, pic_ds_en
      pic_out_rot = pJpegHandle->out_rot_mod;
      if (pic_in_type == YUV444)
      {
#ifdef CONFIG_JPEG_OUTPUT_YUV420SP      
        if(pJpegHandle->bOutYUV420SP)
        {
            pic_out_type = JPEG_SP_YUV420;
            pic_ds = 0x3;
        }
        else
#endif            
        {
#ifdef CONFIG_MT_FPGA_GPE
           if(cinfo->p_jpeg_dbg_info->stress_test_enable && (cinfo->p_jpeg_dbg_info->pis_ds_en_rand== 1))
           {
               pic_ds = 0x3;
               pic_out_type = JPEG_SP_YUV420;
           }
           else if (cinfo->p_jpeg_dbg_info->stress_test_enable && (cinfo->p_jpeg_dbg_info->pis_ds_en_rand == 2))
           {
                pic_ds = 0x2;
                if((pic_out_rot == ROT_180) || (pic_out_rot == ROT_NONE)) 
                 {                   
                    pic_out_type = JPEG_SP_YUV422V2;
                 }
                else
                {
                    pic_out_type = JPEG_SP_YUV422H2;    
                }
           }
            else
#endif
           {
                pic_ds = 0x1;
                if((pic_out_rot == ROT_90) || (pic_out_rot == ROT_270)) 
                    pic_out_type = JPEG_SP_YUV422V2;
                else
                    pic_out_type = JPEG_SP_YUV422H2;    
             }
        }
      }
      else if (pic_in_type == YUV422V2)
      {
#ifdef CONFIG_JPEG_OUTPUT_YUV420SP         
        if(pJpegHandle->bOutYUV420SP)
        {     
            pic_ds = 0x1;
            pic_out_type = JPEG_SP_YUV420;
        }     
        else
#endif               
        {
#ifdef CONFIG_MT_FPGA_GPE
            if(cinfo->p_jpeg_dbg_info->stress_test_enable && ( cinfo->p_jpeg_dbg_info->pis_ds_en_rand == 1))
            {
                pic_ds = 0x1;
                pic_out_type = JPEG_SP_YUV420;
            }
            else
#endif
            {                
                pic_ds = 0x0;
                if((pic_out_rot == ROT_90) || (pic_out_rot == ROT_270)) 
                    pic_out_type = JPEG_SP_YUV422H2;
                else
                    pic_out_type = JPEG_SP_YUV422V2;    
            }
        }
      }
      else if (pic_in_type == YUV422H2)
      {
#ifdef CONFIG_JPEG_OUTPUT_YUV420SP         
        if(pJpegHandle->bOutYUV420SP)
        {    
            pic_ds = 0x2;
            pic_out_type = JPEG_SP_YUV420; 
        }
        else
#endif            
        {
#ifdef CONFIG_MT_FPGA_GPE
            if(cinfo->p_jpeg_dbg_info->stress_test_enable && (cinfo->p_jpeg_dbg_info->pis_ds_en_rand == 1))
            {
               pic_ds = 0x2;
               pic_out_type = JPEG_SP_YUV420;
            }
            else
#endif
            {
                pic_ds = 0x0;
                if((pic_out_rot == ROT_90) || (pic_out_rot == ROT_270)) 
                    pic_out_type = JPEG_SP_YUV422V2;
                else
                    pic_out_type = JPEG_SP_YUV422H2;     
            }
        }
      }
      else if (pic_in_type == YUV420)
      {
        pic_ds = 0x0;
        pic_out_type = JPEG_SP_YUV420;      
      }
      else if (pic_in_type == RGB_R) 
      {
        pic_ds = 0;
        pic_out_type = JPEG_ARGB8888;
      }  
      else if((pic_in_type == CMYK_C) || (pic_in_type == CMYK_M) || 
        (pic_in_type == CMYK_Y) || (pic_in_type == CMYK_K))
      {
        pic_ds = 0;
        pic_out_type = JPEG_SP_CMYK;
      }   
    	else if(pic_in_type == YUV400)
    	{
    	pic_ds = 0;
    	pic_out_type = JPEG_GRAY_8;
    	}

#ifdef CONFIG_MT_FPGA_GPE
    cinfo->p_jpeg_dbg_info->pic_ds  = pic_ds;
#endif
  pic_out_scale = (mt_u8)pJpegHandle->u32ScalRation;
  if(( cinfo->max_h_samp_factor == 2) || ((pic_ds & 0x1) == 0x1))
    active_width = (mt_u32)((cinfo->image_width >> pic_out_scale) & (~0x1));
  else
    active_width = (mt_u32)(cinfo->image_width >> pic_out_scale);
  if(( cinfo->max_v_samp_factor == 2) || ((pic_ds & 0x2) == 0x2))
    active_height = (mt_u32)((cinfo->image_height >> pic_out_scale) & (~0x1));
  else
 	 active_height = (cinfo->image_height >> pic_out_scale) > 0 ?  (cinfo->image_height >> pic_out_scale) :  1;  //bug 14646

  if(pJpegHandle->stOutDesc.bCrop == MT_FALSE)
  {
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x = 0;
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y = 0;
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x = active_width - 1;
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y = active_height - 1;
  }
  else
  {
    //暂时不考虑旋转的情况
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x = (mt_u32)pJpegHandle->stOutDesc.stCropRect.x;
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y = (mt_u32)pJpegHandle->stOutDesc.stCropRect.y;
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x = 
        (mt_u32)(pJpegHandle->stOutDesc.stCropRect.x + pJpegHandle->stOutDesc.stCropRect.w - 1);
    pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y = 
        (mt_u32)(pJpegHandle->stOutDesc.stCropRect.y + pJpegHandle->stOutDesc.stCropRect.h - 1);
  }
  
//硬件最终输出的大小
  if ((pic_out_rot == ROT_NONE) || (pic_out_rot == ROT_180))
   {
	 dst_width = w_mcu_aligned[0] >> pic_out_scale;
	 dst_height = h_mcu_aligned[0] >> pic_out_scale;
	 if(pic_ds == 0)
	 {
		 dst_width_uv = w_mcu_aligned[1] >> pic_out_scale;
		 dst_height_uv = h_mcu_aligned[1] >> pic_out_scale;
	 }
	 else if(pic_ds == 1)
	 {
		 dst_width_uv = ((w_mcu_aligned[1] >> pic_out_scale) + 1) / 2;		 
		 dst_height_uv = h_mcu_aligned[1] >> pic_out_scale;
	 }
	 else if(pic_ds == 2)
	 {
		 dst_width_uv = w_mcu_aligned[1] >> pic_out_scale;		 
		 dst_height_uv = ((h_mcu_aligned[1] >> pic_out_scale) + 1) / 2;    
	 }
	 else
	 {
		 dst_width_uv = ((w_mcu_aligned[1] >> pic_out_scale) + 1) / 2; 
		 dst_height_uv = ((h_mcu_aligned[1] >> pic_out_scale) + 1) / 2;  
	 }	 
   }
   else
   {
	 dst_width = h_mcu_aligned[0] >> pic_out_scale;
	 dst_height = w_mcu_aligned[0] >> pic_out_scale;
	 if(pic_ds == 0)
	 {
		 dst_width_uv = h_mcu_aligned[1] >> pic_out_scale;
		 dst_height_uv = w_mcu_aligned[1] >> pic_out_scale;
	 }
	 else if(pic_ds == 1)
	 {
		 dst_width_uv = h_mcu_aligned[1] >> pic_out_scale;		 
		 dst_height_uv = ((w_mcu_aligned[1] >> pic_out_scale) + 1) / 2;
	 }
	 else if(pic_ds == 2)
	 {
		 dst_width_uv = ((h_mcu_aligned[1] >> pic_out_scale) + 1) / 2;		
		 dst_height_uv = w_mcu_aligned[1] >> pic_out_scale;  
	 }
	 else
	 {
		 dst_width_uv = ((h_mcu_aligned[1] >> pic_out_scale) + 1) /2;  
		 dst_height_uv = ((w_mcu_aligned[1] >> pic_out_scale) + 1) / 2;
	 }	 
   }


  pJpegHandle->stJpegSofInfo.u32YOutWidth = dst_width;
  pJpegHandle->stJpegSofInfo.u32YOutHeight = dst_height;
  pJpegHandle->stJpegSofInfo.u32COutWidth = dst_width_uv; 
  pJpegHandle->stJpegSofInfo.u32COutHeight = dst_height_uv;

  
  //有效显示区域大小(旋转后)
  if(pJpegHandle->stOutDesc.bCrop == MT_FALSE)
  {
      if(pic_out_rot == ROT_NONE)
      {
        pJpegHandle->stOutDesc.stCropRect.w = (mt_s32)active_width;
        pJpegHandle->stOutDesc.stCropRect.h = (mt_s32)active_height;     
        pJpegHandle->stOutDesc.stCropRect.x = 0;
        pJpegHandle->stOutDesc.stCropRect.y = 0;
      }
      else if(pic_out_rot == ROT_90)
      {
        pJpegHandle->stOutDesc.stCropRect.w = (mt_s32)active_height;
        pJpegHandle->stOutDesc.stCropRect.h = (mt_s32)active_width;   
        pJpegHandle->stOutDesc.stCropRect.x = 0;
        pJpegHandle->stOutDesc.stCropRect.y = (mt_s32)dst_height - pJpegHandle->stOutDesc.stCropRect.h;
      }
      else if(pic_out_rot == ROT_180)
      {
        pJpegHandle->stOutDesc.stCropRect.w = (mt_s32)active_width;
        pJpegHandle->stOutDesc.stCropRect.h = (mt_s32)active_height;    
        pJpegHandle->stOutDesc.stCropRect.x = (mt_s32)dst_width - pJpegHandle->stOutDesc.stCropRect.w;
        pJpegHandle->stOutDesc.stCropRect.y = (mt_s32)dst_height - pJpegHandle->stOutDesc.stCropRect.h;
      }
      else
      {
        pJpegHandle->stOutDesc.stCropRect.w = (mt_s32)active_height;
        pJpegHandle->stOutDesc.stCropRect.h = (mt_s32)active_width;    
        pJpegHandle->stOutDesc.stCropRect.x = (mt_s32)dst_width - pJpegHandle->stOutDesc.stCropRect.w;
        pJpegHandle->stOutDesc.stCropRect.y = 0;
      }
  }
    
   if((cinfo->jpeg_color_space == JCS_YCbCr) || (cinfo->jpeg_color_space == JCS_GRAYSCALE))
  {
    stride_y_min = (dst_width + 31) & (~0x1F);
	stride_uv_min = ((dst_width_uv << 1) + 31) & (~0x1F);  
  }
  else if(cinfo->jpeg_color_space == JCS_RGB)
  {
    stride_y_min = (dst_width * 4 + 31) & (~0x1F);
  }
  else if((cinfo->jpeg_color_space == JCS_CMYK) || 
    (cinfo->jpeg_color_space == JCS_YCCK))
  {
    stride_y_min = (dst_width * 2 + 31) & (~0x1F);  
    stride_uv_min =  stride_y_min;
  }
  //jpeg解码要求y和uv的stride为32byte对齐
  pJpegHandle->stJpegSofInfo.u32YOutStride = stride_y_min;
  pJpegHandle->stJpegSofInfo.u32COutStride = stride_uv_min;  
    
  //osd显示要求y和uv的stride成倍数
  if(pic_out_type == JPEG_SP_YUV422V2)
    pJpegHandle->stJpegSofInfo.u32COutStride = 2 * stride_y_min;


  
  switch(cinfo->out_color_space)
  {
	case JCS_ARGB_8888:
		if(cinfo->jpeg_color_space != JCS_RGB)
			MT_GFX_GetStride((MT_U32)pJpegHandle->stJpegSofInfo.u32YOutWidth * 4,&(pJpegHandle->stJpegSofInfo.u32DisplayStride),32); 
		else
			pJpegHandle->stJpegSofInfo.u32DisplayStride = pJpegHandle->stJpegSofInfo.u32YOutStride;
		break;
	case JCS_ABGR_8888:
		MT_GFX_GetStride((MT_U32)pJpegHandle->stJpegSofInfo.u32YOutWidth * 4,&(pJpegHandle->stJpegSofInfo.u32DisplayStride),32); 
		break;
	  case JCS_ARGB_1555:
	  case JCS_ABGR_1555:
	  case JCS_RGB_565:
	  case JCS_BGR_565:
		   MT_GFX_GetStride((MT_U32)pJpegHandle->stJpegSofInfo.u32YOutWidth * 2,&(pJpegHandle->stJpegSofInfo.u32DisplayStride),32); 
		break;	
	default:
		pJpegHandle->stJpegSofInfo.u32DisplayStride = pJpegHandle->stJpegSofInfo.u32YOutStride;
		break;
		
  }
  


#ifdef CONFIG_JPEG_HARDDEC2ARGB
  if(MT_TRUE == pJpegHandle->bDecARGB)
  {
	  pJpegHandle->stJpegSofInfo.u32YSize	= pJpegHandle->stJpegSofInfo.u32DisplayStride * pJpegHandle->stJpegSofInfo.u32YOutHeight;
	  pJpegHandle->stJpegSofInfo.u32CSize	= 0;
  }
  else
#endif
  {
	  pJpegHandle->stJpegSofInfo.u32YSize = pJpegHandle->stJpegSofInfo.u32YOutStride * pJpegHandle->stJpegSofInfo.u32YOutHeight;
	  pJpegHandle->stJpegSofInfo.u32CSize = pJpegHandle->stJpegSofInfo.u32COutStride * pJpegHandle->stJpegSofInfo.u32COutHeight;
  }

  pJpegHandle->stJpegSofInfo.u32YMcuAlignWidth = w_mcu_aligned[0];
  pJpegHandle->stJpegSofInfo.u32YMcuAlignHeight = h_mcu_aligned[0];
  pJpegHandle->stJpegSofInfo.sJpegOtherInfo.pic_in_type = pic_in_type;
  pJpegHandle->stJpegSofInfo.sJpegOtherInfo.pic_ds = pic_ds;
  pJpegHandle->stJpegSofInfo.u32CMcuAlignWidth = w_mcu_aligned[1];
  pJpegHandle->stJpegSofInfo.u32CMcuAlignHeight = h_mcu_aligned[1];

		/**
		 ** the jpeg size info has calculated
		 ** CNcomment:jpeg大小已经计算完了，不需要重新计算了 CNend\n
		 **/
    pJpegHandle->stJpegSofInfo.bCalcSize       =  MT_TRUE;        

}

