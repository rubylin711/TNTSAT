/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MTFB_CONFIG_H__
#define __MTFB_CONFIG_H__


/*********************************add include here******************************/

#include "mt_type.h"



/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */



/***************************** Macro Definition ******************************/

#define CONFIG_MTFB_STRIDE_16ALIGN                         16

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/
static inline mt_void MT_MTFB_GetStride(mt_u32 u32SrcSize, mt_u32 *pu32Stride,mt_u32 u32Align)
{
    *pu32Stride = (u32SrcSize + u32Align - 1) & (~(u32Align - 1));
}


#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MTFB_CONFIG_H__ */


