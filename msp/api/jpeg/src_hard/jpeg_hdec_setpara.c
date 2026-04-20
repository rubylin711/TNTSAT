/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_type.h"
#include "mt_jpeg_config.h"
#include "jpeg_hdec_api.h"
#include "jpeg_hdec_mem.h"
#include "jpeg_hdec_rwreg.h"
#include "jpeg_hdec_table.h"
#include "mt_drv_jpeg_reg.h"
#include "jpeg_hdec_adp.h"

#ifdef CONFIG_JPEG_FPGA_TEST_SET_DIFFERENT_OUTSTANDING_VALUE
#include "mt_jpeg_hdec_test.h"
#endif
/***************************** Macro Definition ******************************/
#ifdef CONFIG_MT_FPGA_GPE
#define RANDOM_DATA(a, b) ((rand() % (b - a + 1)) + a)
#endif
/*************************** Structure Definition ****************************/


/********************** Global Variable declaration **************************/

/******************************* API forward declarations *******************/
MT_VOID JPEG_HDEC_SetIdctBuf(const struct jpeg_decompress_struct *cinfo, MT_U32 comp_id);
phys_addr_t get_tar_addr(phys_addr_t base_addr, MT_U32 h, MT_U32 stride, MT_U32 w, MT_U32 k, MT_JPEG_ROT_MOD_E rot_mode);


/******************************* API realization *****************************/


static MT_S32 JPEG_HDEC_SetInfo(const struct jpeg_decompress_struct *cinfo)
{		
    JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
		
  MT_U32 huff_tab_fix = 0, iq_tab_fix = 0, mcu_blk_sel = 0;
  MT_U32 huff_tab_sel0 = 0, huff_tab_sel1 = 0, huff_tab_sel2 = 0, huff_tab_sel3 = 0;
  MT_U32 pic_in_type = 0;
  jpeg_component_info *compptr = cinfo->comp_info;
  MT_U32 dtmp = 0;
  MT_U32 pic_ds_mod = 0;
  MT_U8 bs_in_swap_mod = 0, bs_out_swap_mod = 0, ch_swap_mod = 0;
  
    dtmp = ((pJpegHandle->stJpegSofInfo.u32YMcuAlignHeight & 0xffff) << 16) 
        | pJpegHandle->stJpegSofInfo.u32YMcuAlignWidth;
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_SIZE, (MT_S32)dtmp);
      dtmp = cinfo->restart_interval;
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_RI_MCU, (MT_S32)dtmp);
      if(cinfo->comps_in_scan == 1)
      {
          if((cinfo->jpeg_color_space == JCS_YCbCr) || (cinfo->jpeg_color_space == JCS_GRAYSCALE))
            pic_in_type = YUV400;
          else if(cinfo->jpeg_color_space == JCS_RGB)
            pic_in_type = RGB400;      
      }
      else
        pic_in_type = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.pic_in_type;
#ifdef CONFIG_MT_FPGA_GPE
    if(cinfo->p_jpeg_dbg_info->stress_test_enable && cinfo->p_jpeg_dbg_info->pis_ds_en_rand == 1)
    {
            if(cinfo->p_jpeg_dbg_info->sw_dec)
            {
                cinfo->p_jpeg_dbg_info->ds_mode = RANDOM_DATA(0,1);
                pic_ds_mod = cinfo->p_jpeg_dbg_info->ds_mode;
            }
            else
                pic_ds_mod = cinfo->p_jpeg_dbg_info->ds_mode;
    }
    else 
            pic_ds_mod = 1;
    cinfo->p_jpeg_dbg_info->ds_mode = pic_ds_mod;
#endif
      dtmp = (pJpegHandle->stJpegSofInfo.sJpegOtherInfo.pic_ds << 20) 
        | (pJpegHandle->out_rot_mod << 12) 
        | (pJpegHandle->u32ScalRation << 8) 
        | (pic_in_type) 
        | (pic_ds_mod << 16);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE, (MT_S32)dtmp);

      
      bs_in_swap_mod = 0x0;
#ifdef CONFIG_MT_FPGA_GPE      
       if(cinfo->p_jpeg_dbg_info->jpeg_set_video_endian_en)
       {
         bs_out_swap_mod = 0x2;
         ch_swap_mod = 0x1; 
       }
       else
       {
         bs_out_swap_mod = 0x0;
         ch_swap_mod = 0x0;
       }
#endif       
       dtmp = (ch_swap_mod << 8) | (bs_out_swap_mod << 4) | bs_in_swap_mod;
     JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BS_SWAP_MOD, dtmp);
    dtmp = ((pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y & 0x3fff) << 16) |
        (pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x & 0x3fff);
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_START, (MT_S32)dtmp);
    dtmp = ((pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y & 0x3fff) << 16) |
        (pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x & 0x3fff);
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_END, (MT_S32)dtmp);

    if(pJpegHandle->bSoftHuffDec == MT_FALSE)
    {
          if((cinfo->jpeg_color_space == JCS_YCbCr) && (cinfo->comps_in_scan > 1))
            jcodec_set_tab(cinfo, TRUE);
          else
            jcodec_set_tab(cinfo, FALSE);

       if(cinfo->comps_in_scan > 1)
       {
           if(cinfo->jpeg_color_space == JCS_YCbCr)
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, 0xff003f00);
          else if(cinfo->jpeg_color_space == JCS_RGB)
          {
            huff_tab_fix = 0xf;
            if(compptr[0].quant_tbl_no == 0)
                iq_tab_fix = 0x8;
            else if(compptr[0].quant_tbl_no == 1)
                iq_tab_fix = 0x9;
            else
                iq_tab_fix = 0xa;    
            mcu_blk_sel = 0x8;
            dtmp = (0xff << 24) | (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);

            huff_tab_sel0 = (MT_U32)compptr[0].dc_tbl_no;
            huff_tab_sel1 = (MT_U32)compptr[1].dc_tbl_no;
            huff_tab_sel2 = (MT_U32)compptr[2].dc_tbl_no;
            
            dtmp = 0x111 | (huff_tab_sel0 << 3) | (huff_tab_sel1 << 7) | (huff_tab_sel2 << 11);
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG1, (MT_S32)dtmp);
          }
          else if((cinfo->jpeg_color_space == JCS_CMYK) || (cinfo->jpeg_color_space == JCS_YCCK))
          {
            huff_tab_fix = 0xf;
            if(compptr[0].quant_tbl_no == 0)
                iq_tab_fix = 0x8;
            else if(compptr[0].quant_tbl_no == 1)
                iq_tab_fix = 0x9;
            else
                iq_tab_fix = 0xa;    
            mcu_blk_sel = 0x8;
            dtmp = (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);

            huff_tab_sel0 = (MT_U32)compptr[0].dc_tbl_no;
            huff_tab_sel1 = (MT_U32)compptr[1].dc_tbl_no;
            huff_tab_sel2 = (MT_U32)compptr[2].dc_tbl_no;
            huff_tab_sel3 = (MT_U32)compptr[3].dc_tbl_no;
            
            dtmp = 0x1111 | (huff_tab_sel0 << 3) | (huff_tab_sel1 << 7) | 
                (huff_tab_sel2 << 11) | (huff_tab_sel3 << 15);
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG1, (MT_S32)dtmp);
          }
       }
       else
       {
          huff_tab_fix = 0xf;
          if(compptr[0].quant_tbl_no == 0)
              iq_tab_fix = 0x8;
          else if(compptr[0].quant_tbl_no == 1)
              iq_tab_fix = 0x9;
          else
              iq_tab_fix = 0xa;    
          mcu_blk_sel = 0;
          dtmp = (0xff << 24) | (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);
          
          huff_tab_sel0 = (mt_u32)(compptr[0].dc_tbl_no);
          dtmp = 0x1 | (huff_tab_sel0 << 3);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG1, (MT_S32)dtmp); 
       }
    }
	else
	{
//		MT_U32 mcu_blk_size;
		if(cinfo->jpeg_color_space == JCS_YCbCr)
		  jcodec_set_iq_tab(cinfo, TRUE);
		else
		  jcodec_set_iq_tab(cinfo, FALSE);

    	JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_HUFF_MCU_CFG0, 0xff000001 | ((pJpegHandle->swHuff.blk_size - 1) << 8));  
	}
    return MT_SUCCESS;

}



/*****************************************************************************
* func			: JPEG_HDEC_SetStreamBuf
* description	: set stream buffer message
				  CNcomment: 设置码流buffer寄存器 	   CNend\n
* param[in] 	: cinfo 		CNcomment: 解码对象    CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_SetStreamBuf(const struct jpeg_decompress_struct *cinfo)
{

		ulong u32Align       = 64;  /**< the stream buffer should 64 bytes align *//**<CNcomment:码流buffer需要64字节对齐 */
		MT_U32 u32Offset      = 0; /**< you can set >=0,insure include the save stream buffer *//**<CNcomment:大于等于0的值，保证能够包含存储码流buffer的区域 */
		MT_U64 u64StreamSize  = 0;
		phys_addr_t pStartStreamPhy  = 0;
		phys_addr_t pEndStreamPhy    = 0;
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		/**
		** if use user buffer, tmts buffer can not sure is 64 bytes align
		** CNcomment: 如果是使用用户的连续码流buffer，并不一定是64字节对齐
		**            硬件buffer只有有两个值即可，真正使用的存储码流的buffer，
		**            必须在这两个值之间 CNend\n
		**/
		if(MT_TRUE == pJpegHandle->stHDecDataBuf.bUserPhyMem && MT_FALSE == pJpegHandle->stHDecDataBuf.bNeedStreamReturn)
		{
		       if((phys_addr_t)pStartStreamPhy % 64)
		        {
    			u64StreamSize = pJpegHandle->stHDecDataBuf.u64DataSize + u32Offset;
    			/**
    			** tmts can insure the start buffer is before the stream address
    			** after align.
    			** CNcomment: 这个能确保对齐之后在码流地址之前 CNend\n
    			**/
    			pStartStreamPhy = pJpegHandle->stHDecDataBuf.pDataPhyBuf - u32Align;
    			pStartStreamPhy = ((pStartStreamPhy + u32Align - 1) & (~(u32Align - 1)));
    			pStartStreamPhy = pStartStreamPhy - u32Offset;
		        }
                      else
                        {
                            u64StreamSize = pJpegHandle->stHDecDataBuf.u64DataSize;
                            pStartStreamPhy = pJpegHandle->stHDecDataBuf.pDataPhyBuf;
                        }
			pEndStreamPhy   =pJpegHandle->stHDecDataBuf.pDataPhyBuf + u64StreamSize;
		}
		else if(MT_TRUE == pJpegHandle->stHDecDataBuf.bNeedStreamReturn)
		{/** 码流回绕 **/
			pStartStreamPhy = pJpegHandle->stHDecDataBuf.pStartBufPhy;
			pEndStreamPhy   = pStartStreamPhy + pJpegHandle->stHDecDataBuf.s32BufLen;
		}
		else
		{
			u64StreamSize   = JPGD_HARD_BUFFER;
			pStartStreamPhy = pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf;
			pEndStreamPhy   = pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u64StreamSize;
		}
		

		/**
		 ** the end address [0:d] invalid. just ok
		 ** CNcomment: 结束地址回读的时候[0:5]无效，为零，所以会看到和实际不一致，正常的 CNend\n
		 **/		
		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,    \
						    JCODEC_BUF_START_ADDR, 				  \
						    (phys_addr_t)pStartStreamPhy);
		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,    \
						    JCODEC_BUF_END_ADDR,				  \
						    (phys_addr_t)pEndStreamPhy - 1);

//        printf("\r\n buf start:0x%08x, buf_end:0x%08x", pStartStreamPhy, pEndStreamPhy - 1);
//        printf("\r\n return:%d, bufphy:0x%08x", pJpegHandle->stHDecDataBuf.bNeedStreamReturn, pJpegHandle->stHDecDataBuf.pDataPhyBuf);
		
}

MT_VOID JPEG_HDEC_SetIdctBuf(const struct jpeg_decompress_struct *cinfo, MT_U32 comp_id)
{

  JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,    \
  				    JCODEC_BS_START_ADDR, 				  \
  				    (phys_addr_t)pJpegHandle->swHuff.idct_buf_addr_phy[comp_id]);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,    \
  				    JCODEC_BS_END_ADDR,				  \
  				    (MT_S32)pJpegHandle->swHuff.idct_buf_addr_phy[comp_id] + (MT_S32)pJpegHandle->swHuff.idct_buf_size[comp_id] - 1);		 
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,    \
  				    JCODEC_BUF_START_ADDR, 				  \
  				    (MT_S32 )pJpegHandle->swHuff.idct_buf_addr_phy[comp_id]);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,    \
  				    JCODEC_BUF_END_ADDR,				  \
  				    (phys_addr_t)pJpegHandle->swHuff.idct_buf_addr_phy[comp_id] + (MT_S32)pJpegHandle->swHuff.idct_buf_size[comp_id] - 1);
}

phys_addr_t get_tar_addr(phys_addr_t base_addr, MT_U32 h, MT_U32 stride, MT_U32 w, MT_U32 k, MT_JPEG_ROT_MOD_E rot_mode)
{
    phys_addr_t tar_addr = 0;
    switch(rot_mode)
    {
      case ROT_NONE:
          tar_addr = base_addr;        
          break;
      case ROT_180:
          tar_addr = base_addr + (h - 1) * stride + (w - 1) * k;
          break;
      case ROT_90:
          tar_addr = base_addr + (h - 1) * stride;
          break;     
      case ROT_270:
          tar_addr = base_addr + (w - 1) * k;
          break;             
    }
    return tar_addr;
}

/*****************************************************************************
* func			: JPEG_HDEC_SetMidderBuf
* description	: set middle buffer register
				  CNcomment: 设置JPEG硬件解码输出寄存器   CNend\n
* param[in] 	: cinfo 		CNcomment: 解码对象       CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
static MT_VOID JPEG_HDEC_SetDstBuf(const struct jpeg_decompress_struct * cinfo)
{
    JPEG_HDEC_HANDLE_S_PTR	  pJpegHandle = NULL;
    pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
    MT_U32 dtmp = 0;

    //flush stMiddleSurface before set to hw
    if(pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem != MT_TRUE) {
        MT_GFX_Flush(pJpegHandle->stMiddleSurface.pMiddleVir[0], 0, 0);
        MT_GFX_Flush(pJpegHandle->stMiddleSurface.pMiddleVir[1], 0, 0);
    }
    
    if(cinfo->jpeg_color_space == JCS_YCbCr)
    {    

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_LU_STRIDE, 	
        	             (MT_S32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0]);
#ifdef CONFIG_MT_FPGA_GPE
			cinfo->p_jpeg_dbg_info->y_stride = pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0];
#endif
		}
		else
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_LU_STRIDE, 	
        	             (MT_S32)pJpegHandle->stJpegSofInfo.u32YOutStride);
#ifdef CONFIG_MT_FPGA_GPE
			cinfo->p_jpeg_dbg_info->y_stride = pJpegHandle->stJpegSofInfo.u32YOutStride;
#endif
		}


#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
                 JCODEC_PIC_CH_STRIDE, 	
                 (MT_S32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[1]);
#ifdef CONFIG_MT_FPGA_GPE
			cinfo->p_jpeg_dbg_info->uv_stride= pJpegHandle->stOutDesc.stOutSurface.u32OutStride[1];
#endif
		}
		else
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
                 JCODEC_PIC_CH_STRIDE, 	
                 (MT_S32)pJpegHandle->stJpegSofInfo.u32COutStride);
#ifdef CONFIG_MT_FPGA_GPE
			cinfo->p_jpeg_dbg_info->uv_stride= pJpegHandle->stJpegSofInfo.u32COutStride;
#endif
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{

        dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
            pJpegHandle->stJpegSofInfo.u32YOutHeight,
            (MT_U32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0],
            pJpegHandle->stJpegSofInfo.u32YOutWidth, 
            1,
            pJpegHandle->out_rot_mod);
		}
		else
		{

        dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
            pJpegHandle->stJpegSofInfo.u32YOutHeight,
            (MT_U32)pJpegHandle->stJpegSofInfo.u32YOutStride,
            pJpegHandle->stJpegSofInfo.u32YOutWidth, 
            1,
            pJpegHandle->out_rot_mod);		
		}

        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_LU_ADDR, (MT_S32)dtmp);
		
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{  
        dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[1], 
            pJpegHandle->stJpegSofInfo.u32COutHeight, 
            (MT_U32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[1],
            pJpegHandle->stJpegSofInfo.u32COutWidth, 
            2,
            pJpegHandle->out_rot_mod);
		}
		else
		{

        dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[1], 
            pJpegHandle->stJpegSofInfo.u32COutHeight, 
            (MT_U32)pJpegHandle->stJpegSofInfo.u32COutStride,
            pJpegHandle->stJpegSofInfo.u32COutWidth, 
            2,
            pJpegHandle->out_rot_mod);		
		}

        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CB_ADDR, (MT_S32)dtmp);
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CR_ADDR, (MT_S32)dtmp);
      }
      else if(cinfo->jpeg_color_space == JCS_RGB)
      {
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_LU_STRIDE, 				 
        	             (MT_S32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0]);   
#ifdef CONFIG_MT_FPGA_GPE	   
			cinfo->p_jpeg_dbg_info->y_stride = pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0];
#endif
		}
		else
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_LU_STRIDE, 				 
        	             (MT_S32)pJpegHandle->stJpegSofInfo.u32YOutStride);  
#ifdef CONFIG_MT_FPGA_GPE						 		
			cinfo->p_jpeg_dbg_info->y_stride = pJpegHandle->stJpegSofInfo.u32YOutStride;
#endif					
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{

        dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
            pJpegHandle->stJpegSofInfo.u32YOutHeight, 
            (MT_U32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0],
            pJpegHandle->stJpegSofInfo.u32YOutWidth, 
            4,
            pJpegHandle->out_rot_mod);
		}
		else
		{

		dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
			pJpegHandle->stJpegSofInfo.u32YOutHeight, 
			(MT_U32)pJpegHandle->stJpegSofInfo.u32YOutStride,
			pJpegHandle->stJpegSofInfo.u32YOutWidth, 
			4,
			pJpegHandle->out_rot_mod);

		}

        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_LU_ADDR, (MT_S32)dtmp);      
      }
      else if((cinfo->jpeg_color_space == JCS_CMYK) || 
            (cinfo->jpeg_color_space == JCS_YCCK))
      {
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_CH_STRIDE, 				 
        	             (MT_S32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0]);      
#ifdef CONFIG_MT_FPGA_GPE						 
                cinfo->p_jpeg_dbg_info->uv_stride=pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0];
#endif				
		}
		else
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_CH_STRIDE, 				 
        	             (MT_S32)pJpegHandle->stJpegSofInfo.u32YOutStride);    
#ifdef CONFIG_MT_FPGA_GPE						 
			cinfo->p_jpeg_dbg_info->uv_stride = pJpegHandle->stJpegSofInfo.u32YOutStride;
#endif					
		}
#ifdef CONFIG_MT_FPGA_GPE
		cinfo->p_jpeg_dbg_info->y_stride = cinfo->p_jpeg_dbg_info->uv_stride;
#endif
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{

        dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
            pJpegHandle->stJpegSofInfo.u32YOutHeight, 
            (MT_U32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0],
            pJpegHandle->stJpegSofInfo.u32YOutWidth, 
            2,
            pJpegHandle->out_rot_mod);
		}
		else
		{

		dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
			pJpegHandle->stJpegSofInfo.u32YOutHeight, 
			(MT_U32)pJpegHandle->stJpegSofInfo.u32YOutStride,
			pJpegHandle->stJpegSofInfo.u32YOutWidth, 
			2,
			pJpegHandle->out_rot_mod);

		}

        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CB_ADDR, (MT_S32)dtmp);          
      }
	  else if(cinfo->jpeg_color_space == JCS_GRAYSCALE)
	  {    
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
	  if(	(MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
		  &&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
	  if(	(MT_TRUE == pJpegHandle->bOutYCbCrSP)
		  &&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
	  {
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_LU_STRIDE, 	
        	             (MT_S32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0]);
#ifdef CONFIG_MT_FPGA_GPE						 
			cinfo->p_jpeg_dbg_info->y_stride = pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0];
#endif
		}
		else
		{
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
        	             JCODEC_PIC_LU_STRIDE, 	
        	             (MT_S32)pJpegHandle->stJpegSofInfo.u32YOutStride);
#ifdef CONFIG_MT_FPGA_GPE						 
			cinfo->p_jpeg_dbg_info->y_stride = pJpegHandle->stJpegSofInfo.u32YOutStride;
#endif				
		}

        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
                 JCODEC_PIC_CH_STRIDE, 	
                 0);
#ifdef CONFIG_MT_FPGA_GPE
       cinfo->p_jpeg_dbg_info->uv_stride = 0;
#endif
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{

        dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
            pJpegHandle->stJpegSofInfo.u32YOutHeight,
            (MT_U32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0],
            pJpegHandle->stJpegSofInfo.u32YOutWidth, 
            1,
            pJpegHandle->out_rot_mod);
		}
		else
		{

		dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], 
			pJpegHandle->stJpegSofInfo.u32YOutHeight,
			(MT_U32)pJpegHandle->stJpegSofInfo.u32YOutStride,
			pJpegHandle->stJpegSofInfo.u32YOutWidth, 
			1,
			pJpegHandle->out_rot_mod);

		}

        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_LU_ADDR, (MT_S32)dtmp);
    
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CB_ADDR, 0);
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CR_ADDR, 0);
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)) 
#endif
		{		
		memset(pJpegHandle->stMiddleSurface.pMiddleVir[1], 0x80, 
			(MT_U32)pJpegHandle->stOutDesc.stOutSurface.u32OutStride[1] * pJpegHandle->stJpegSofInfo.u32COutHeight);
		}
		else
		{

		memset(pJpegHandle->stMiddleSurface.pMiddleVir[1], 0x80, 
			(MT_U32)pJpegHandle->stJpegSofInfo.u32COutStride * pJpegHandle->stJpegSofInfo.u32COutHeight);

}

	  }	  
}


/*****************************************************************************
* func			: JPEG_HDEC_SetPara
* description	: set the parameter that hard decode need
				  CNcomment: 配置硬件解码需要的参数信息 	   CNend\n
* param[in] 	: cinfo 		CNcomment: 解码对象    CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	   CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_SetPara(const struct jpeg_decompress_struct *cinfo)
{
    JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    MT_S32 s32RetVal = MT_SUCCESS;

    /**
     ** turn on interrupt
     ** CNcomment: 打开中断 CNend\n
     **/
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_INT_EN,0x7F03);

          if((pJpegHandle->clip_value_max > 0) && (pJpegHandle->clip_value_max <= 255))
          {
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_DEC_CLIP, 
                ((1 << 8) | pJpegHandle->clip_value_max));
          }
          else
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_DEC_CLIP, 0);

    /**
     ** set dqt table register
     ** CNcomment: 配置量化表，21us CNend\n
     ** CNcomment: 配置哈夫曼表，100us ~ 120us CNend\n
     **/
    /**
     ** set sof message
     ** CNcomment: 设置SOF信息 CNend\n
     **/
    s32RetVal = JPEG_HDEC_SetInfo(cinfo);

    if(MT_SUCCESS != s32RetVal)
    {
       return MT_FAILURE;
    }
    
    if(pJpegHandle->bSoftHuffDec)
    {

    }
    else
    {
    /**
     ** set stream buffer message
     ** CNcomment: 设置码流buffer寄存器 CNend\n
     **/
    JPEG_HDEC_SetStreamBuf(cinfo);
    }
    /**
     ** set middle buffer register
     ** CNcomment: 设置JPEG硬件解码输出寄存器 CNend\n
     **/
    JPEG_HDEC_SetDstBuf(cinfo);	

    return MT_SUCCESS;

}


