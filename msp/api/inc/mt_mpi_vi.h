/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_vi.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/

#ifndef __MT_MPI_VI_H__
#define __MT_MPI_VI_H__

#include "drv_vi_ioctl.h"
#include "mt_error_mpi.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C"
{
 #endif
#endif

#define MIN_VI_FB_NUM 4
#define MAX_VI_FB_NUM 16

mt_s32         MT_MPI_VI_Init(mt_void);
mt_s32         MT_MPI_VI_DeInit(mt_void);
mt_s32         MT_MPI_VI_Create(MT_UNF_VI_E enViPort, MT_UNF_VI_ATTR_S *pstAttr, mt_handle *phVi);
mt_s32         MT_MPI_VI_Destroy(mt_handle handle);
mt_s32         MT_MPI_VI_SetAttr(mt_handle handle, MT_UNF_VI_ATTR_S *pstAttr);
mt_s32         MT_MPI_VI_GetAttr(mt_handle handle, MT_UNF_VI_ATTR_S *pstAttr);
mt_s32         MT_MPI_VI_Attach(mt_handle hVi, mt_handle hDst);
mt_s32         MT_MPI_VI_Detach(mt_handle hVi, mt_handle hDst);
mt_s32         MT_MPI_VI_SetExternBuffer(mt_handle handle, MT_UNF_VI_BUFFER_ATTR_S* pstBufAttr);
mt_s32         MT_MPI_VI_Start(mt_handle handle);
mt_s32         MT_MPI_VI_Stop(mt_handle handle);
mt_s32         MT_MPI_VI_QueueFrame(mt_handle hVI, MT_UNF_VIDEO_FRAME_INFO_S *pFrameInfo);
mt_s32         MT_MPI_VI_DequeueFrame(mt_handle hVI, MT_UNF_VIDEO_FRAME_INFO_S *pFrameInfo);
mt_s32         MT_MPI_VI_AcquireFrame(mt_handle handle, MT_UNF_VIDEO_FRAME_INFO_S *pFrameInfo, mt_u32 u32TimeoutMs);
mt_s32         MT_MPI_VI_ReleaseFrame(mt_handle handle, const MT_UNF_VIDEO_FRAME_INFO_S *pFrameInfo);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif

#endif //__MT_MPI_VI_H__
