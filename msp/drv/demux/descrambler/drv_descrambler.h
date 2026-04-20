/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 File Name     : drv_descrambler.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       : 2013/04/16
 Description   :
******************************************************************************/

#ifndef __DRV_DESCRAMBLER_H__
#define __DRV_DESCRAMBLER_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


typedef enum
{
    DMX_KEY_TYPE_EVEN   = 0,
    DMX_KEY_TYPE_ODD    = 1
} DMX_KEY_TYPE_E;

typedef enum
{
    DMX_KEY_CW = 0,
    DMX_KEY_IV = 1
} DMX_CW_TYPE;

mt_s32  DMXKeyIoctl(struct file *file, mt_u32 cmd, mt_void *arg);
mt_void DmxDestroyAllDescrambler(ulong file);

#ifdef MT_DEMUX_PROC_SUPPORT
mt_s32  DMXKeyProcRead(struct seq_file *p, mt_void *v);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __DRV_DESCRAMBLER_H__ */

