
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_win_buffer.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_WIN_BUFFER_H__
#define __DRV_WIN_BUFFER_H__

#include"vfmw.h"
#include "drv_disp_com.h"
#include "mt_drv_win.h"
//#include "MT_Dec.h"
#include "drv_disp_bufcore.h"
//#include "MT_DF_disp.h"
//#include "MT_DF_video.h"
//#include "MT_DF_disp.h"
//#include "MT_DF_FRC.h"
#if defined(CONFIG_MT_CHIP_ARIA)
#include "MT_DF_Aria_reg.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#include "drv_disp_Symphony_reg.h"
#endif

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

typedef struct tagWIN_BUF_ALLOC_PARA_S
{
    MT_BOOL bFbAllocMem;
    mt_u32 u32BufWidth;
    mt_u32 u32BufHeight;
    mt_u32 u32BufStride;
    MT_DRV_PIX_FORMAT_E eDataFormat;

    // user supply mem
}WIN_BUF_ALLOC_PARA_S;

typedef struct tagWB_SOURCE_INFO_S
{
    mt_handle hSrc;
    PFN_GET_FRAME_CALLBACK pfAcqFrame;
    PFN_PUT_FRAME_CALLBACK pfRlsFrame;

    //Extend Freeze interfaces
    PFN_GET_FRAME_CALLBACK pfAcqFreezeFrame;
    PFN_PUT_FRAME_CALLBACK pfRlsFreezeFrame;

}WB_SOURCE_INFO_S;


#define WB_BUFFER_DEBUG_FRAME_RECORD_NUMBER 32
typedef struct tagWB_DEBUG_INFO_S
{
    mt_u32 u32RecordNumber;

    mt_u32 u32InputFrameID[WB_BUFFER_DEBUG_FRAME_RECORD_NUMBER];
    mt_u32 u32InputPos;
    mt_u32 u32Input;

    mt_u32 u32CfgFrameID[WB_BUFFER_DEBUG_FRAME_RECORD_NUMBER];
    mt_u32 u32CfgPos;
    mt_u32 u32Config;
    
    mt_u32 u32RlsFrameID[WB_BUFFER_DEBUG_FRAME_RECORD_NUMBER];
    mt_u32 u32RlsPos;
    mt_u32 u32Release;

    mt_u32 u32TryQueueFrame;
    mt_u32 u32QueueFrame;
    mt_u32 u32Underload;
    mt_u32 u32Disacard;
}WB_DEBUG_INFO_S;

typedef struct tagWB_POOL_S
{
    mt_u32 u32BufNumber;
    
    mt_u32 u32MemType;
    WIN_BUF_ALLOC_PARA_S stAlloc;
    DISP_BUF_S stBuffer;
    // source info
    WB_SOURCE_INFO_S stSrcInfo;

    //lastest display and config.//
    DISP_BUF_NODE_S *pstDisplay;
    DISP_BUF_NODE_S *pstConfig;
    DISP_BUF_NODE_S *pstCapture;
    
    WB_DEBUG_INFO_S *pstDebugInfo;
}WB_POOL_S;

#define WIN_BUF_MEM_SRC_SUPPLY  0
#define WIN_BUF_MEM_FB_SUPPLY   1
#define WIN_BUF_MEM_USER_SUPPLY 2

#define WIN_CHECK_NULL_RETURN(p) \
do{ if (!p)    \
    {WIN_FATAL("FUNC %s input NULL Pointer!\n", __FUNCTION__); return MT_FAILURE;} \
}while(0)

#define WIN_CHECK_NULL_RETURN_NULL(p) \
do{ if (!p)    \
    {WIN_FATAL("FUNC %s input NULL Pointer!\n", __FUNCTION__); return MT_NULL;} \
}while(0)

mt_s32 WinBuf_Create(mt_u32 u32BufNum, mt_u32 u32MemType, WIN_BUF_ALLOC_PARA_S *pstAlloc, WB_POOL_S *pstWinBP);
mt_s32 WinBuf_Destroy(WB_POOL_S *pstWinBP);
mt_s32 WinBuf_Reset(WB_POOL_S *pstWinBP);

mt_s32 WinBuf_SetSource(WB_POOL_S *pstWinBP, WB_SOURCE_INFO_S *pstSrc);

// put a new frame into buffer
mt_s32 WinBuf_PutNewFrame(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstFrame);

// release frame that has been displayed and set configed frame as displayed frame.
mt_s32 WinBuf_RlsAndUpdateUsingFrame(WB_POOL_S *pstWinBP);
mt_s32 WinBuf_RepeatDisplayedFrame(WB_POOL_S *pstWinBP);
mt_s32 WinBuf_DiscardDisplayedFrame(WB_POOL_S *pstWinBP);
MT_DRV_VIDEO_FRAME_S *WinBuf_GetDisplayedFrame(WB_POOL_S *pstWinBP);
MT_DRV_VIDEO_FRAME_S *WinBuf_GetConfigedFrame(WB_POOL_S *pstWinBP);

MT_DRV_VIDEO_FRAME_S *WinBuf_GetConfigFrame(WB_POOL_S *pstWinBP);
mt_s32 WinBuf_SetCaptureFrame(WB_POOL_S *pstWinBP, mt_u32 u32InvalidFlag);
mt_s32 WinBuf_ReleaseCaptureFrame(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstFrame, MT_BOOL bForceFlag);
MT_DRV_VIDEO_FRAME_S *WinBuf_GetCapturedFrame(WB_POOL_S *pstWinBP);




mt_s32 WinBuf_ForceReleaseFrame(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 WinBuf_ReleaseOneFrame(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstPreFrame);
mt_s32 WinBuf_FlushWaitingFrame(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstPreFrame);
MT_DRV_VIDEO_FRAME_S *WinBuf_GetFrameByMaxID(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstRefFrame,mt_u32 u32RefID, MT_DRV_FIELD_MODE_E enDstField);
MT_DRV_VIDEO_FRAME_S * WinBuf_GetFrameByDstFrame(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstDstFrame, MT_DRV_VIDEO_FRAME_S *pstRefFrame);
MT_DRV_VIDEO_FRAME_S *WinBuf_GetFrameByDisplayInfo(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstRefFrame, mt_u32 u32RefRate, MT_DRV_FIELD_MODE_E enDstField);
MT_DRV_VIDEO_FRAME_S * WinBuf_GetNewestFrame(WB_POOL_S *pstWinBP, MT_DRV_VIDEO_FRAME_S *pstRefFrame);
mt_s32 WinBuf_GetFullBufNum(WB_POOL_S *pstBP, mt_u32 *pu32BufNum);

// todo
typedef struct tagWB_STATE_S
{
    mt_u32 u32Number;

    mt_u32 u32EmptyRPtr;
    mt_u32 u32EmptyWPtr;

    mt_u32 u32FullRPtr;
    mt_u32 u32FullWPtr;
    
    struct {
        mt_u32 u32State;
        mt_u32 u32Empty;
        mt_u32 u32Full;
        mt_u32 u32FrameIndex;
    }stNode[DISP_BUF_NODE_MAX_NUMBER];

    WB_DEBUG_INFO_S stRecord;

    mt_u32 u32EmptyArray[DISP_BUF_NODE_MAX_NUMBER];
    mt_u32 u32FullArray[DISP_BUF_NODE_MAX_NUMBER];

    MT_DRV_VIDEO_FRAME_S stCurrentFrame;
}WB_STATE_S;

mt_s32 WinBuf_GetStateInfo(WB_POOL_S *pstWinBP, WB_STATE_S *pstWinBufState);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_wIN_BUFFER_H__  */


