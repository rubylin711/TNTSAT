/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_DRV_JPEG_REG_H__
#define __MT_DRV_JPEG_REG_H__


/*********************************add include here******************************/
#include "mt_jpeg_config.h"
#include "mt_type.h"

/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */


    /***************************** Macro Definition ******************************/




#define JPGD_REG_LENGTH (0x300)

#define JCODEC_BASE_ADDR 0
#define JCODEC_START (JCODEC_BASE_ADDR + 0x0)
#define JCODEC_EN (JCODEC_BASE_ADDR + 0x4)
#define JCODEC_HUFF_MCU_CFG0 (JCODEC_BASE_ADDR + 0x8)
#define JCODEC_HUFF_MCU_CFG1 (JCODEC_BASE_ADDR + 0xc)
#define JCODEC_PIC_SIZE (JCODEC_BASE_ADDR + 0x10)
#define JCODEC_PIC_RI_MCU (JCODEC_BASE_ADDR + 0x14)
#define JCODEC_PIC_TYPE (JCODEC_BASE_ADDR + 0x18)
#define JCODEC_BS_SWAP_MOD (JCODEC_BASE_ADDR + 0x1c)
#define JCODEC_BUF_START_ADDR (JCODEC_BASE_ADDR + 0x20)
#define JCODEC_BUF_END_ADDR (JCODEC_BASE_ADDR + 0x24)
#define JCODEC_BUF_CUR_ADDR (JCODEC_BASE_ADDR + 0x28)
#define JCODEC_BS_START_ADDR (JCODEC_BASE_ADDR + 0x30)
#define JCODEC_BS_END_ADDR (JCODEC_BASE_ADDR + 0x34)
#define JCODEC_PIC_LU_ADDR (JCODEC_BASE_ADDR + 0x38)
#define JCODEC_PIC_LU_STRIDE (JCODEC_BASE_ADDR + 0x3c)
#define JCODEC_PIC_CB_ADDR (JCODEC_BASE_ADDR + 0x40)
#define JCODEC_PIC_CR_ADDR (JCODEC_BASE_ADDR + 0x48)
#define JCODEC_PIC_CH_STRIDE (JCODEC_BASE_ADDR + 0x4c)
#define JCODEC_STATE (JCODEC_BASE_ADDR + 0x50)
#define JCODEC_PIC_ERR (JCODEC_BASE_ADDR + 0x5c)
#define JCODEC_INT_STATE (JCODEC_BASE_ADDR + 0x60)
#define JCODEC_INT_EN (JCODEC_BASE_ADDR + 0x64)
#define JCODEC_CFG_EN (JCODEC_BASE_ADDR + 0x70)
#define JCODEC_CFG_PORT (JCODEC_BASE_ADDR + 0x74)
#define JCODEC_CLIP_START (JCODEC_BASE_ADDR + 0x80)
#define JCODEC_CLIP_END (JCODEC_BASE_ADDR + 0x84)
#define JCODEC_QUANT_COEF_CNT (JCODEC_BASE_ADDR + 0x88)
#define JCODEC_AXI_WR_LAST_MODE  (JCODEC_BASE_ADDR + 0x8c)
#define JCODEC_AXI_CTRL (JCODEC_BASE_ADDR + 0xe0)
#define JCODEC_AXI_STATUS (JCODEC_BASE_ADDR + 0xe4)
#define JCODEC_DEC_CLIP (JCODEC_BASE_ADDR + 0xe8)
#define JCODEC_CLOCK_GATED (JCODEC_BASE_ADDR + 0xf0)
#define JCODEC_LU_DC_MAX_CODE_BASE (JCODEC_BASE_ADDR + 0x100)
#define JCODEC_LU_AC_MAX_CODE_BASE (JCODEC_BASE_ADDR + 0x120)
#define JCODEC_CH_DC_MAX_CODE_BASE (JCODEC_BASE_ADDR + 0x140)
#define JCODEC_CH_AC_MAX_CODE_BASE (JCODEC_BASE_ADDR + 0x160)
#define JCODEC_LU_DC_MAX_ADDR_BASE (JCODEC_BASE_ADDR + 0x180)
#define JCODEC_LU_AC_MAX_ADDR_BASE (JCODEC_BASE_ADDR + 0x1a0)
#define JCODEC_CH_DC_MAX_ADDR_BASE (JCODEC_BASE_ADDR + 0x1c0)
#define JCODEC_CH_AC_MAX_ADDR_BASE (JCODEC_BASE_ADDR + 0x1e0)
#define JCODEC_DCT_COEF (JCODEC_BASE_ADDR + 0x200)
#define JCODEC_ADDR_MASK (0x1fffffff)

#define JCODEC_REG_NUM (40)

#define MAX_ERR_REC 128
#define MARKER_EOI 0xd9
#define MARKER_DHT 0xc4
#define MARKER_DQT 0xdb
#define MARKER_SOS 0xda

#define EOI_REACHED (1 << 0)
#define DHT_REACHED (1 << 1)
#define DQT_REACHED (1 << 2)
#define SOS_REACHED (1 << 3)

    /*************************** Structure Definition ****************************/

    /********************** Global Variable declaration **************************/


    /******************************* API declaration *****************************/
	/*****************************************************************************
	* func			  : JPGDRV_READ_REG
	* description	  : read register value
	* param[in] 	  : base
	* param[in] 	  : offset
	* retval		  : none
	* output		  : none
	* others:		  : notmtng
	*****************************************************************************/
	MT_U32 JPGDRV_READ_REG(ulong base,MT_U32 offset);
	/*****************************************************************************
	* func			  : JPGDRV_WRITE_REG
	* description	  : write register value
	* param[in] 	  : base
	* param[in] 	  : offset
	* param[in] 	  : value
	* retval		  : none
	* output		  : none
	* others:		  : notmtng
	*****************************************************************************/
	MT_VOID  JPGDRV_WRITE_REG(ulong base, MT_U32 offset, MT_U32 value);


#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MT_DRV_JPEG_REG_H__ */
