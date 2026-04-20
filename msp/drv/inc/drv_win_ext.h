/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : drv_win_ext.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/
#ifndef __DRV_WIN_EXT_H__
#define __DRV_WIN_EXT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include "mt_type.h"
#include "mt_drv_video.h"
#include "mt_drv_disp.h"
#include "mt_drv_win.h"
#include "mt_drv_dev.h"

typedef struct
{
    mt_handle hWin;
    mt_handle hSlvWin;
}WIN_PRIV_STATE_S;
typedef mt_s32 (* FN_WIN_Init)(mt_void);
typedef mt_s32 (* FN_WIN_DeInit)(mt_void);
typedef mt_s32 (* FN_WIN_Create)(MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWin);
typedef mt_s32 (* FN_WIN_Destroy)(mt_handle hWin);
typedef mt_s32 (* FN_WIN_SetAttr)(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr);
typedef mt_s32 (* FN_WIN_GetAttr)(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr);

//get info for source
typedef mt_s32 (* FN_WIN_GetInfo)(mt_handle hWin, MT_DRV_WIN_INFO_S * pstInfo);
typedef mt_s32 (* FN_WIN_SetSource)(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc);
typedef mt_s32 (* FN_WIN_GetSource)(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc);
typedef mt_s32 (* FN_WIN_SetEnable)(mt_handle hWin, MT_BOOL bEnable);
typedef mt_s32 (* FN_WIN_GetEnable)(mt_handle hWin, MT_BOOL *pbEnable);
typedef mt_s32 (* FN_WIN_QueueFrame)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo);
typedef mt_s32 (* FN_WIN_QueueUselessFrame)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo);
typedef mt_s32 (* FN_WIN_DequeueFrame)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo);


typedef mt_s32 (* FN_WIN_GetPlayInfo)(mt_handle hWin, MT_DRV_WIN_PLAY_INFO_S *pstInfo);

typedef mt_s32 (* FN_WIN_SetZorder)(mt_handle hWin, MT_DRV_DISP_ZORDER_E enZFlag);
typedef mt_s32 (* FN_WIN_GetZorder)(mt_handle hWin, mt_u32 *pu32Zorder);
typedef mt_s32 (* FN_WIN_Freeze)(mt_handle hWin, MT_BOOL bEnable,  MT_DRV_WIN_SWITCH_E eRst);
typedef mt_s32 (* FN_WIN_Reset)(mt_handle hWin, MT_DRV_WIN_SWITCH_E eRst);
typedef mt_s32 (* FN_WIN_Pause)(mt_handle hWin, MT_BOOL bEnable);

typedef mt_s32 (* FN_WIN_SetStepMode)(mt_handle hWin, MT_BOOL bStepMode);
typedef mt_s32 (* FN_WIN_SetStepPlay)(mt_handle hWin);
/* only for virtual window */
typedef mt_s32 (* FN_WIN_SetExtBuffer)(mt_handle hWin, MT_DRV_VIDEO_BUFFER_POOL_S* pstBuf);
typedef mt_s32 (* FN_WIN_AcquireFrame)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
typedef mt_s32 (* FN_WIN_ReleaseFrame)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
//todo
typedef mt_s32 (* FN_WIN_Set3DMode)(mt_handle hWin, MT_BOOL b3DEnable,MT_DRV_DISP_STEREO_E eMode);

typedef mt_s32 (* FN_WIN_SetQuick)(mt_handle hWin, MT_BOOL bEnable);

typedef mt_s32 (* FN_WIN_CapturePicture)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);
typedef mt_s32 (* FN_WIN_CapturePictureRelease)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);

typedef mt_s32 (* FN_WIN_SetRotation)(mt_handle hWin, MT_DRV_ROT_ANGLE_E enRotation);
typedef mt_s32 (* FN_WIN_GetRotation)(mt_handle hWin, MT_DRV_ROT_ANGLE_E *penRotation);
typedef mt_s32 (* FN_WIN_SetFlip)(mt_handle hWin, MT_BOOL bHoriFlip, MT_BOOL bVertFlip);
typedef mt_s32 (* FN_WIN_GetFlip)(mt_handle hWin, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip);

typedef mt_s32 (* FN_WIN_SendFrame)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
typedef mt_s32 (* FN_WIN_GetLatestFrameInfo)(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *frame_info);
typedef mt_s32 (*FN_WIN_UpdatePqData)(mt_u32 u32UpdateType,/*PQ_PARAM_S */ mt_void * pstPqParam);
typedef mt_s32 (*FN_WIN_Suspend)(basedev_s *pdev, pm_message_t state);
typedef mt_s32 (*FN_WIN_Resume)(basedev_s *pdev);
typedef struct
{
    FN_WIN_Init           pfnWinInit;
    FN_WIN_DeInit         pfnWinDeInit;
    FN_WIN_Create         pfnWinCreate;
    FN_WIN_Destroy        pfnWinDestory;
    FN_WIN_SetAttr        pfnWinSetAttr;
    FN_WIN_GetAttr        pfnWinGetAttr;
    FN_WIN_GetInfo        pfnWinGetInfo;
    FN_WIN_SetSource      pfnWinSetSrc;
    FN_WIN_GetSource      pfnWinGetSrc;
    FN_WIN_SetEnable      pfnWinSetEnable;
    FN_WIN_GetEnable      pfnWinGetEnable;
    FN_WIN_QueueFrame     pfnWinQueueFrm;
    FN_WIN_QueueUselessFrame   pWinQueueUselessFrm;
    FN_WIN_DequeueFrame   pfnWinDequeueFrm;
    FN_WIN_GetPlayInfo    pfnWinGetPlayInfo;
    FN_WIN_SetZorder      pfnWinSetZorder;
    FN_WIN_GetZorder      pfnWinGetZorder;
    FN_WIN_Freeze         pfnWinFreeze;
    FN_WIN_Reset          pfnWinReset;
    FN_WIN_Pause          pfnWinPause;
    FN_WIN_SetStepMode    pfnWinSetStepMode;
    FN_WIN_SetStepPlay    pfnWinSetStepPlay;
    FN_WIN_SetExtBuffer   pfnWinSetExtBuffer;
    FN_WIN_AcquireFrame   pfnWinAcquireFrm;
    FN_WIN_ReleaseFrame   pfnWinRlsFrm;
    FN_WIN_Set3DMode      pfnWin3DMode;
    FN_WIN_SetQuick       pfnWinSetQuik;
    FN_WIN_CapturePicture pfnWinCapturePic;
    FN_WIN_CapturePictureRelease pfnWinCapturePicRls;
    FN_WIN_SetRotation    pfnWinSetRotation;
    FN_WIN_GetRotation    pfnWinGetRotation;
    FN_WIN_SetFlip        pfnWinSetFlip;
    FN_WIN_GetFlip        pfnWinGetFlip;
    FN_WIN_SendFrame      pfnWinSendFrm;
    FN_WIN_GetLatestFrameInfo pfnWinGetLatestFrameInfo;
    FN_WIN_UpdatePqData        pfnWinUpdatePqData;	
    FN_WIN_Resume         pfnWinResume;
    FN_WIN_Suspend        pfnWinSuspend;
}WIN_EXPORT_FUNC_S;

mt_s32  DRV_WIN_Register(mt_void);
mt_void DRV_WIN_UnRegister(mt_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif  /* __VO_EXT_H__ */

