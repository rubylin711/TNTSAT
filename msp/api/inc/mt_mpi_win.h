/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_win.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*********************************************************************************************/


#ifndef __MT_MPI_WIN_H__
#define __MT_MPI_WIN_H__

#include "mt_drv_win.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

mt_s32 MT_MPI_WIN_Init(mt_void);
mt_s32 MT_MPI_WIN_DeInit(mt_void);

mt_s32 MT_MPI_WIN_Create(const MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWin);
mt_s32 MT_MPI_WIN_Create_Ext(const MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWindow, MT_BOOL bVirtScreen);
mt_s32 MT_MPI_WIN_Destroy(mt_handle hWin);

mt_s32 MT_MPI_WIN_SetAttr(mt_handle hWin, const MT_DRV_WIN_ATTR_S *pWinAttr);
mt_s32 MT_MPI_WIN_GetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr);

mt_s32 MT_MPI_WIN_DequeueFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);
mt_s32 MT_MPI_WIN_QueueFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);
mt_s32 MT_MPI_WIN_GetFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);

mt_s32 MT_MPI_WIN_VO_MAPFrame(mt_handle hWin,mt_u32 *pFrame);

mt_s32 MT_MPI_WIN_QueueUselessFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);

//get info for source
mt_s32 MT_MPI_WIN_GetInfo(mt_handle hWin, MT_DRV_WIN_INFO_S * pstInfo);

mt_s32 MT_MPI_WIN_SetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc);
mt_s32 MT_MPI_WIN_GetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc);

mt_s32 MT_MPI_WIN_SetEnable(mt_handle hWin, MT_BOOL bEnable);
mt_s32 MT_MPI_WIN_GetEnable(mt_handle hWin, MT_BOOL *pbEnable);

mt_s32 MT_MPI_WIN_SetZorder(mt_handle hWin, MT_DRV_DISP_ZORDER_E enZFlag);
mt_s32 MT_MPI_WIN_GetZorder(mt_handle hWin, mt_u32 *pu32Zorder);

mt_s32 MT_MPI_WIN_Freeze(mt_handle hWin, MT_BOOL bEnable, MT_DRV_WIN_SWITCH_E eRst);
mt_s32 MT_MPI_WIN_GetFreezeStat(mt_handle hWindow, MT_BOOL *bEnable, MT_DRV_WIN_SWITCH_E *enWinFreezeMode);

mt_s32 MT_MPI_WIN_Reset(mt_handle hWin, MT_DRV_WIN_SWITCH_E eRst);

mt_s32 MT_MPI_WIN_Pause(mt_handle hWin, MT_BOOL bEnable);

mt_s32 MT_MPI_WIN_GetPlayInfo(mt_handle hWin, MT_DRV_WIN_PLAY_INFO_S *pstInfo);

mt_s32 MT_MPI_WIN_SetStepMode(mt_handle hWin, MT_BOOL bStepMode);
mt_s32 MT_MPI_WIN_SetStepPlay(mt_handle hWin);

/* only for virtual window */
mt_s32 MT_MPI_WIN_SetExtBuffer(mt_handle hWin, MT_DRV_VIDEO_BUFFER_POOL_S* pstBuf);
mt_s32 MT_MPI_WIN_AcquireFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
mt_s32 MT_MPI_WIN_ReleaseFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);

//todo
mt_s32 MT_MPI_WIN_SetQuickOutput(mt_handle hWin, MT_BOOL bEnable);
mt_s32 MT_MPI_WIN_GetQuickOutputStatus(mt_handle hWindow, MT_BOOL *bQuickOutputEnable);
mt_s32 MT_MPI_WIN_GetQuickOutput(mt_handle hWin, MT_BOOL *pbEnable);

mt_s32 MT_MPI_WIN_CapturePicture(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);
mt_s32 MT_MPI_WIN_CapturePictureRelease(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);

mt_s32 MT_MPI_WIN_SetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E enRotation);
mt_s32 MT_MPI_WIN_GetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E *penRotation);

mt_s32 MT_MPI_WIN_SetFlip(mt_handle hWin, MT_BOOL bHoriFlip, MT_BOOL bVertFlip);
mt_s32 MT_MPI_WIN_GetFlip(mt_handle hWin, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip);

mt_s32 MT_MPI_WIN_SendFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
mt_s32 MT_MPI_WIN_SET_PARA(mt_handle hWin,MT_DRV_WIN_PARA_S *para);
mt_s32 MT_MPI_WIN_CleanAllFrm(mt_handle hWin, MT_DRV_WIN_FLUSH_TYPE_E eType);

mt_s32 MT_MPI_WIN_Suspend(mt_void);
mt_s32 MT_MPI_WIN_Resume(mt_void);

//mt_s32 MT_MPI_WIN_GetHandle(WIN_GET_HANDLE_S *pstWinHandle);


mt_s32 MT_MPI_WIN_GetWinParam(mt_handle hWin, MT_DRV_WIN_INTF_S *pstWinIntf);

mt_s32 MT_MPI_WIN_AttachWinSink(mt_handle hWin, mt_handle hSink);

mt_s32 MT_MPI_WIN_DetachWinSink(mt_handle hWin, mt_handle hSink);

mt_s32 MT_MPI_WIN_GetLatestFrameInfo(mt_handle hWin, MT_DRV_VIDEO_FRAME_S  *frame_info);

mt_s32 MT_MPI_WIN_GetUnloadTimes(mt_handle hWin, mt_u32 *pu32Time);

mt_s32 MT_MPI_VO_GetWindowDelay(mt_handle hWindow, MT_DRV_WIN_PLAY_INFO_S *pDelay);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

