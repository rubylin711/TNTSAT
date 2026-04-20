/*****************************************************************************
*             Copyright 2006 - 2014, Montage Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: tde_hal.h
* Description:TDE hal interface define
*
* History:
* Version   Date          Author        DefectNum       Description
*
*****************************************************************************/
//#include "tde_hal.h"
#include "tde_define.h"
#ifndef _TDE_ADP_H_
#define _TDE_ADP_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/
#if defined(CONFIG_GFX_STB_SDK)
#include "mt_drv_mmz.h"
#include "mt_drv_dev.h"
#elif defined(CONFIG_GFX_BVT_SDK)
#include <linux/sched.h>
#endif
#include "drv_tde_ext.h"


#if defined(CONFIG_GFX_STB_SDK)
#if defined(CONFIG_MT_CHIP_ARIA)
#define TDE_REG_BASEADDR  0xffd60000
#define TDE_INTNUM  (36+32)
#else
#define TDE_REG_BASEADDR  SYMPHONY_IO_PA(0xbf430000)
#define TDE_INTNUM  (IRQ_GRA_ID) //(IRQ_GRA_ENG_0_ID)
#endif
//#define TDE_REG_BASEADDR 0xf8c10000

#define CONFIG_TDE_USE_SDK_CRG_ENABLE
#define CONFIG_TDE_TDE_EXPORT_FUNC
//#define CONFIG_TDE_PM_ENABLE
#define MPP_LICENSE "GPL"
#define DESCRIPTION "Montage` TDE Device driver"
#define AUTHOR "Digital Media Team, Montage crop."
#define VERSION "V1.0.0.0"


#endif



#define TDE_NO_SCALE_STEP 0x1000
#define TDE_FLOAT_BITLEN 12
#define TDE_MAX_SLICE_WIDTH 256
#define TDE_MAX_SLICE_NUM 20
#define TDE_MAX_RECT_WIDTH 0xfff
#define TDE_MAX_RECT_HEIGHT 0xfff
#define TDE_MAX_SURFACE_PITCH 0xffff
#define TDE_MAX_ZOOM_OUT_STEP 8
#define TDE_MAX_RECT_WIDTH_EX 0x2000
#define TDE_MAX_RECT_HEIGHT_EX 0x2000


#define TDE_MAX_MINIFICATION_H  255
#define TDE_MAX_MINIFICATION_V  255



#define   ROP        (0x1)/*Rop*/
#define   ALPHABLEND     (0x1<<1) /* AlphaBlend */
#define   COLORIZE    (0x1<<2)/* Colorize */
#define   CLUT   (0x1<<3) /* Clut */
#define   COLORKEY  (0x1<<4) /* ColorKey */
#define   CLIP  (0x1<<5) /* Clip */
#define   DEFLICKER  (0x1<<6) /* Deflicker */
#define   RESIZE  (0x1<<7) /* Resize */
#define   MIRROR  (0x1<<8) /* Mirror */
#define   CSCCOVERT  (0x1<<9) /* CSC */
#define   QUICKCOPY  (0x1<<10)/*快速拷贝*/
#define   QUICKFILL  (0x1<<11)/*快速填充*/
#define   PATTERFILL  (0x1<<12)/*模式填充*/
#define   MASKROP  (0x1<<13)/*MaskRop*/
#define   MASKBLEND  (0x1<<14)/*MaskBlend*/

/*******************************************************************************
* Function:      TdeHalGetCapability
* Description:   Get the capability of TDE
* Input:         the pointer of the capability
* Output:        none
* Return:        success
* Others:        none
*******************************************************************************/
mt_s32 TdeHalGetCapability(mt_u32 *pstCapability);

#ifdef __cplusplus
#if __cplusplus
}
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

#endif  /*_TDE_ADP_H_*/


