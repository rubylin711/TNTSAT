/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 File Name     : drv_demux_ext.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       : 2012/08/24
 Description   :
******************************************************************************/

#ifndef __DRV_DEMUX_EXT_H__
#define __DRV_DEMUX_EXT_H__

#include "mt_type.h"
#include "mt_drv_dev.h"
#include "mt_drv_demux.h"

typedef mt_s32  (*FN_DEMUX_AcquireEs)(mt_handle, DMX_Stream_S*);
typedef mt_s32  (*FN_DEMUX_ReleaseEs)(mt_handle, DMX_Stream_S*);
//typedef mt_s32  (*FN_DEMUX_Suspend)(basedev_s *himd, pm_message_t state);
typedef mt_s32  (*FN_DEMUX_Suspend)(basedev_s *himd, pm_message_t state);
//typedef mt_s32  (*FN_DEMUX_Resume)(basedev_s *himd);
typedef mt_s32  (*FN_DEMUX_Resume)(basedev_s *himd);

typedef struct
{
    FN_DEMUX_AcquireEs      pfnDmxAcquireEs;
    FN_DEMUX_ReleaseEs      pfnDmxReleaseEs;
    FN_DEMUX_Suspend        pfnDmxSuspend;
    FN_DEMUX_Resume         pfnDmxResume;
} DEMUX_EXPORT_FUNC_S;

mt_s32  DMX_DRV_ModInit(mt_void);
mt_void DMX_DRV_ModExit(mt_void);

#endif

