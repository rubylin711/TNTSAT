/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_jpeglib.h"
#include "jpegint.h"
#include "mt_jerror.h"
#include "jdhuff.h"		/* Declarations shared with jdhuff.c */

#include "mt_type.h"
#include "jpeg_hdec_adp.h"
#include "jpeg_hdec_api.h"
#include "jpeg_hdec_rwreg.h"
#include "mt_drv_jpeg_reg.h"
#include "jpeg_hdec_table.h"

#define JCODEC_PRT printf

/***************************** Macro Definition ******************************/
void jcodec_set_iq_tab(j_decompress_ptr p_cinfo, MT_BOOL auto_flag)
{
  JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(p_cinfo->client_data);
  mt_u32 dtmp = 0;
  mt_u32 addr = 0;
  jpeg_component_info *compptr = p_cinfo->comp_info;

  if(auto_flag)
  {
//    JCODEC_PRT("\nIQ:\n");
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x10);

//    JCODEC_PRT("\r\n quant_tbl_no:%d,%d,%d", 
//        compptr[0].quant_tbl_no, compptr[1].quant_tbl_no, compptr[2].quant_tbl_no);
    // IQ: Y  
    if(p_cinfo->quant_tbl_ptrs[compptr[0].quant_tbl_no] != NULL)
    {
      for(addr = 0; addr < 64; addr ++)
      {
        dtmp = (addr << 16) | 
          p_cinfo->quant_tbl_ptrs[compptr[0].quant_tbl_no]->quantval[jpeg_natural_order[addr]];
  //      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
      }
    }
    // IQ: U
    if(p_cinfo->quant_tbl_ptrs[compptr[1].quant_tbl_no] != NULL)
    {    
      for (addr = 0; addr < 64; addr ++)
      {
        dtmp = ((addr + 64) << 16) | 
          p_cinfo->quant_tbl_ptrs[compptr[1].quant_tbl_no]->quantval[jpeg_natural_order[addr]];
  //      JCODEC_PRT("addr[%d]: 0x%x\n", addr + 64, dtmp);
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
      }
    }
    // IQ: V
    if(p_cinfo->quant_tbl_ptrs[compptr[2].quant_tbl_no] != NULL)
    {
      for (addr = 0; addr < 64; addr ++)
      {
        dtmp = ((addr + 128) << 16) | 
          p_cinfo->quant_tbl_ptrs[compptr[2].quant_tbl_no]->quantval[jpeg_natural_order[addr]];
  //      JCODEC_PRT("addr[%d]: 0x%x\n", addr + 128, dtmp);
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
      }
    }
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x100);
  }
  else
  {    
//    JCODEC_PRT("\nIQ:\n");
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x10);
    // IQ: Y /R  
    if(p_cinfo->quant_tbl_ptrs[0] != NULL)
    {
      for(addr = 0; addr < 64; addr ++)
      {
        dtmp = (addr << 16) | 
          p_cinfo->quant_tbl_ptrs[0]->quantval[jpeg_natural_order[addr]];
  //      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
      }
    }
    // IQ: U/G
    if(p_cinfo->quant_tbl_ptrs[1] != NULL)
    {
      for (addr = 0; addr < 64; addr ++)
      {
        dtmp = ((addr + 64) << 16) | 
            p_cinfo->quant_tbl_ptrs[1]->quantval[jpeg_natural_order[addr]];
//        JCODEC_PRT("addr[%d]: 0x%x\n", addr + 64, dtmp);
        JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
      }
    }
    // IQ: V/B
    if(p_cinfo->quant_tbl_ptrs[2] != NULL)
    {
        for (addr = 0; addr < 64; addr ++)
        {
          dtmp = ((addr + 128) << 16) | 
            p_cinfo->quant_tbl_ptrs[2]->quantval[jpeg_natural_order[addr]];
//          JCODEC_PRT("addr[%d]: 0x%x\n", addr + 128, dtmp);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
        }
    }  
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x100);
  }
}

void jcodec_set_tab(j_decompress_ptr p_cinfo, MT_BOOL auto_flag)
{
  JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(p_cinfo->client_data);
  mt_s32 dtmp = 0;
  mt_s32 addr = 0;
  mt_u32 i = 0;
  mt_u32 j = 0;
  huff_entropy_ptr entropy = (huff_entropy_ptr) p_cinfo->entropy;
  jpeg_component_info *compptr = p_cinfo->comp_info;

  if(auto_flag)
  {
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->dc_derived_tbls[compptr[0].dc_tbl_no]->maxcode[j + 1] & 0xFFFF) << 16) 
        | (entropy->dc_derived_tbls[compptr[0].dc_tbl_no]->maxcode[j] & 0xFFFF);
//      JCODEC_PRT("luma_dc[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_DC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->ac_derived_tbls[compptr[0].ac_tbl_no]->maxcode[j + 1] & 0xFFFF) << 16) 
        | (entropy->ac_derived_tbls[compptr[0].ac_tbl_no]->maxcode[j] & 0xFFFF);
//      JCODEC_PRT("luma_ac[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_AC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->dc_derived_tbls[compptr[1].dc_tbl_no]->maxcode[j + 1] & 0xFFFF) << 16) 
        | (entropy->dc_derived_tbls[compptr[1].dc_tbl_no]->maxcode[j] & 0xFFFF);
//      JCODEC_PRT("chroma_dc[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_DC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");      
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->ac_derived_tbls[compptr[1].ac_tbl_no]->maxcode[j + 1] & 0xFFFF) << 16) 
        | (entropy->ac_derived_tbls[compptr[1].ac_tbl_no]->maxcode[j] & 0xFFFF);
//      JCODEC_PRT("chroma_ac[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_AC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\nhuff maxaddr:\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->dc_derived_tbls[compptr[0].dc_tbl_no]->maxaddr[j + 1] & 0xFFFF) << 16) 
        | (entropy->dc_derived_tbls[compptr[0].dc_tbl_no]->maxaddr[j] & 0xFFFF);
//      JCODEC_PRT("luma_dc[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_DC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->ac_derived_tbls[compptr[0].ac_tbl_no]->maxaddr[j + 1] & 0xFFFF) << 16) 
        | (entropy->ac_derived_tbls[compptr[0].ac_tbl_no]->maxaddr[j] & 0xFFFF);
//      JCODEC_PRT("luma_ac[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_AC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->dc_derived_tbls[compptr[1].dc_tbl_no]->maxaddr[j + 1] & 0xFFFF) << 16) 
        | (entropy->dc_derived_tbls[compptr[1].dc_tbl_no]->maxaddr[j] & 0xFFFF);
//      JCODEC_PRT("chroma_dc[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_DC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");    
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->ac_derived_tbls[compptr[1].ac_tbl_no]->maxaddr[j + 1] & 0xFFFF) << 16) 
        | (entropy->ac_derived_tbls[compptr[1].ac_tbl_no]->maxaddr[j] & 0xFFFF);
//      JCODEC_PRT("chroma_ac[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_AC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\nhuff symbol:\n");
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x1);
    for (addr = 0; addr < 12; addr++)
    {
      dtmp = (addr << 16) | (mt_s32)(p_cinfo->dc_huff_tbl_ptrs[compptr[0].dc_tbl_no]->huffval[addr]);
//      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (addr = 12; addr < 174; addr++)
    {
      dtmp = (addr << 16) | (mt_s32)(p_cinfo->ac_huff_tbl_ptrs[compptr[0].ac_tbl_no]->huffval[addr - 12]);
//      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
  
    for (addr = 174; addr < 186; addr++)
    {
      dtmp = (addr << 16) | (mt_s32)(p_cinfo->dc_huff_tbl_ptrs[compptr[1].dc_tbl_no]->huffval[addr - 174]);
//      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n"); 
    for (addr = 186; addr < 348; addr++)
    {
      dtmp = (addr << 16) | (mt_s32)(p_cinfo->ac_huff_tbl_ptrs[compptr[1].ac_tbl_no]->huffval[addr - 186]);
//      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
    }
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x100);
  }
  else
  {
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->dc_derived_tbls[0]->maxcode[j + 1] & 0xFFFF) << 16) | 
        (entropy->dc_derived_tbls[0]->maxcode[j] & 0xFFFF);
//      JCODEC_PRT("luma_dc[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_DC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->ac_derived_tbls[0]->maxcode[j + 1] & 0xFFFF) << 16) | 
        (entropy->ac_derived_tbls[0]->maxcode[j] & 0xFFFF);
//      JCODEC_PRT("luma_ac[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_AC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    if(entropy->dc_derived_tbls[1] != NULL)
    {
        for (i = 0, j = 1; i < 8; i ++, j += 2)
        {
          dtmp = ((entropy->dc_derived_tbls[1]->maxcode[j + 1] & 0xFFFF) << 16) | 
            (entropy->dc_derived_tbls[1]->maxcode[j] & 0xFFFF);
//          JCODEC_PRT("chroma_dc[%d]: 0x%x\n", i, dtmp);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_DC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
        }
    }
//    JCODEC_PRT("\n");
    if(entropy->ac_derived_tbls[1] != NULL)
    {      
        for (i = 0, j = 1; i < 8; i ++, j += 2)
        {
          dtmp = ((entropy->ac_derived_tbls[1]->maxcode[j + 1] & 0xFFFF) << 16) | 
            (entropy->ac_derived_tbls[1]->maxcode[j] & 0xFFFF);
//          JCODEC_PRT("chroma_ac[%d]: 0x%x\n", i, dtmp);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_AC_MAX_CODE_BASE + i * 4, (MT_S32)dtmp);
        }
    }
//    JCODEC_PRT("\nhuff maxaddr:\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->dc_derived_tbls[0]->maxaddr[j + 1] & 0xFFFF) << 16) | 
        (entropy->dc_derived_tbls[0]->maxaddr[j] & 0xFFFF);
//      JCODEC_PRT("luma_dc[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_DC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (i = 0, j = 1; i < 8; i ++, j += 2)
    {
      dtmp = ((entropy->ac_derived_tbls[0]->maxaddr[j + 1] & 0xFFFF) << 16) | 
        (entropy->ac_derived_tbls[0]->maxaddr[j] & 0xFFFF);
//      JCODEC_PRT("luma_ac[%d]: 0x%x\n", i, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_AC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    if(entropy->dc_derived_tbls[1] != NULL)
    {
        for (i = 0, j = 1; i < 8; i ++, j += 2)
        {
          dtmp = ((entropy->dc_derived_tbls[1]->maxaddr[j + 1] & 0xFFFF) << 16) | 
            (entropy->dc_derived_tbls[1]->maxaddr[j] & 0xFFFF);
//          JCODEC_PRT("chroma_dc[%d]: 0x%x\n", i, dtmp);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_DC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
        }
    }
//    JCODEC_PRT("\n");
    if(entropy->ac_derived_tbls[1] != NULL)
    {      
        for (i = 0, j = 1; i < 8; i ++, j += 2)
        {
          dtmp = ((entropy->ac_derived_tbls[1]->maxaddr[j + 1] & 0xFFFF) << 16) | 
            (entropy->ac_derived_tbls[1]->maxaddr[j] & 0xFFFF);
 //         JCODEC_PRT("chroma_ac[%d]: 0x%x\n", i, dtmp);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_AC_MAX_ADDR_BASE + i * 4, (MT_S32)dtmp);
        }
    }
//    JCODEC_PRT("\nhuff symbol:\n");
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x1);
    for (addr = 0; addr < 12; addr++)
    {
      dtmp = (addr << 16) | (mt_s32)(p_cinfo->dc_huff_tbl_ptrs[0]->huffval[addr]);
//      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");
    for (addr = 12; addr < 174; addr++)
    {
      dtmp = (addr << 16) | (mt_s32)(p_cinfo->ac_huff_tbl_ptrs[0]->huffval[addr - 12]);
//      JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
    }
//    JCODEC_PRT("\n");

    if(p_cinfo->dc_huff_tbl_ptrs[1] != NULL)
    {
        for (addr = 174; addr < 186; addr++)
        {
          dtmp = (addr << 16) | (mt_s32)(p_cinfo->dc_huff_tbl_ptrs[1]->huffval[addr - 174]);
//          JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
        }
    }
 //   JCODEC_PRT("\n"); 
    if(p_cinfo->ac_huff_tbl_ptrs[1] != NULL)
    {
        for (addr = 186; addr < 348; addr++)
        {
          dtmp = (addr << 16) | (mt_s32)(p_cinfo->ac_huff_tbl_ptrs[1]->huffval[addr - 186]);
//          JCODEC_PRT("addr[%d]: 0x%x\n", addr, dtmp);
          JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT, (MT_S32)dtmp);
        }
    }
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN, 0x100);
  }
  jcodec_set_iq_tab(p_cinfo, auto_flag);  
}

