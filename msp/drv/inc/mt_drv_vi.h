/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : mt_drv_vi.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/

#ifndef __MT_DRV_VI_H__
#define __MT_DRV_VI_H__

#include "mt_debug.h"
#include "mt_module.h"
#include "drv_vi_ioctl.h"
#include "mt_unf_vi.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

#define MT_FATAL_VI(fmt...) MT_FATAL_PRINT(MT_ID_VI, fmt)
#define MT_ERR_VI(fmt...) MT_ERR_PRINT(MT_ID_VI, fmt)
#define MT_WARN_VI(fmt...) MT_WARN_PRINT(MT_ID_VI, fmt)
#define MT_INFO_VI(fmt...) MT_INFO_PRINT(MT_ID_VI, fmt)
#define MT_DEBUG_VI(fmt...) MT_DBG_PRINT(MT_ID_VI, fmt)

mt_s32         MT_DRV_VI_Create(VI_CREATE_S *pstCreate, mt_void *file);
mt_s32         MT_DRV_VI_Destroy(mt_handle hVi);
mt_s32         MT_DRV_VI_SetAttr(mt_handle hVi, MT_UNF_VI_ATTR_S *pstAttr);
mt_s32         MT_DRV_VI_SetExtBuf(MT_UNF_VI_E enViPort, MT_UNF_VI_BUFFER_ATTR_S *pstBufAttr);
mt_s32         MT_DRV_VI_CreateVpssPort(mt_handle hVi, VI_VPSS_PORT_S *pstVpssPort);
mt_s32         MT_DRV_VI_DestroyVpssPort(mt_handle hVi, VI_VPSS_PORT_S *pstVpssPort);
mt_s32         MT_DRV_VI_Suspend(mt_void);
mt_s32         MT_DRV_VI_Resume(mt_void);
mt_s32         MT_DRV_VI_Start(mt_handle hVi);
mt_s32         MT_DRV_VI_Stop(mt_handle hVi);
mt_s32         MT_DRV_VI_DequeueFrame(mt_handle hVi, MT_UNF_VIDEO_FRAME_INFO_S *pstFrame);
mt_s32         MT_DRV_VI_QueueFrame(mt_handle hVi, MT_UNF_VIDEO_FRAME_INFO_S *pstFrame);
mt_s32         MT_DRV_VI_UsrAcquireFrame(mt_handle hVi, MT_UNF_VIDEO_FRAME_INFO_S *pstFrame, MT_U32 u32TimeoutMs);
mt_s32         MT_DRV_VI_UsrReleaseFrame(mt_handle hVi, MT_UNF_VIDEO_FRAME_INFO_S *pstFrame);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif

#endif //__MT_DRV_VI_H__
