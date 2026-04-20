
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_window.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_WINDOW_H__
#define __DRV_WINDOW_H__

#include "mt_type.h"
#include "mt_common.h"
#include "mt_drv_video.h"
#include "mt_drv_win.h"
#include "drv_win_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/******************************************************************************
    local function and macro
******************************************************************************/
#define WinGetType(pstWin) (pstWin->enType)
#define WinGetLayerID(pstWin) (pstWin->eLayer)
#define WinGetDispID(pstWin) (pstWin->enDisp)

#define WinCheckDeviceOpen()                                                    \
    {                                                                           \
        if (WIN_DEVICE_STATE_OPEN != s_s32WindowGlobalFlag) {                   \
            WIN_ERROR("WIN is not inited or suspended in %s!\n", __FUNCTION__); \
            return MT_ERR_VO_NO_INIT;                                           \
        }                                                                       \
    }

#define WinCheckNullPointer(ptr)                                        \
    {                                                                   \
        if (!ptr) {                                                     \
            WIN_ERROR("WIN Input null pointer in %s!\n", __FUNCTION__); \
            return MT_ERR_VO_NULL_PTR;                                  \
        }                                                               \
    }

#define WinCheckWindow(hWin, pstWin)                                       \
    {                                                                      \
        pstWin = WinGetWindow(hWin);                                       \
        if (!pstWin) {                                                     \
            WIN_ERROR("WIN is not exist!%s %d\n", __FUNCTION__, __LINE__); \
            return MT_ERR_VO_WIN_NOT_EXIST;                                \
        }                                                                  \
    }

#define WinCheckSlaveWindow(pstWin)                                          \
    {                                                                        \
        if (MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == WinGetType(pstWin)) {        \
            if (!pstWin->hSlvWin) {                                          \
          WIN_ERROR("WIN Slave window is lost in %s\n", __FUNCTION__); \
          return MT_ERR_VO_SLAVE_WIN_LOST;                             \
            }                                                                \
        }                                                                    \
    }

#define WINDOW_INVALID_ID 0xFFFFFFFFul

#define WINDOW_MAX_NUMBER 2

#define WINDOW_INDEX_MASK 0x00000FFFl
#define WINDOW_INDEX_NUMBER_MASK 0x000000FFl

mt_s32 WIN_Init(mt_void);
mt_s32 WIN_DeInit(mt_void);

mt_s32 WIN_Suspend(mt_void);
mt_s32 WIN_Resume(mt_void);

//mt_s32 WIN_SetMode(MT_DRV_VO_MODE_E enDevMode);
//mt_s32 WIN_GetMode(MT_DRV_VO_MODE_E *enDevMode);

mt_s32 WIN_Create(MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWin, MT_BOOL bVirtScreen);
mt_s32 WIN_Destroy(mt_handle hWin);
mt_s32 WIN_CheckAttachState(mt_handle hWin, MT_BOOL *pbSrcAttached, MT_BOOL *pbSinkAttached);

mt_s32 WIN_SetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr);
mt_s32 WIN_GetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr);

//get info for source
mt_s32 WIN_GetInfo(mt_handle hWin, MT_DRV_WIN_INFO_S *pstInfo);

mt_s32 WIN_SetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc);
mt_s32 WIN_GetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc);

mt_s32 WIN_SetEnable(mt_handle hWin, MT_BOOL bEnable);
mt_s32 WIN_GetEnable(mt_handle hWin, MT_BOOL *pbEnable);

mt_s32 WIN_QueueFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo);
mt_s32 WIN_QueueUselessFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo);
mt_s32 WIN_DequeueFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo);

mt_s32 WIN_GetPlayInfo(mt_handle hWin, MT_DRV_WIN_PLAY_INFO_S *pstInfo);

mt_s32 WIN_SetZorder(mt_handle hWin, MT_DRV_DISP_ZORDER_E enZFlag);
mt_s32 WIN_GetZorder(mt_handle hWin, mt_u32 *pu32Zorder);

mt_s32 WIN_Freeze(mt_handle hWin, MT_BOOL bEnable, MT_DRV_WIN_SWITCH_E eRst);
mt_s32 WIN_GetFreezeStatus(mt_handle hWin, MT_BOOL *pbEnable, MT_DRV_WIN_SWITCH_E *penFrz);

mt_s32 WIN_Reset(mt_handle hWin, MT_DRV_WIN_SWITCH_E eRst);

mt_s32 WIN_Pause(mt_handle hWin, MT_BOOL bEnable);

mt_s32 WIN_SetStepMode(mt_handle hWin, MT_BOOL bStepMode);
mt_s32 WIN_SetStepPlay(mt_handle hWin);

/* only for virtual window */
mt_s32 WIN_SetExtBuffer(mt_handle hWin, MT_DRV_VIDEO_BUFFER_POOL_S *pstBuf);
mt_s32 WIN_AcquireFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
mt_s32 WIN_ReleaseFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
mt_s32 WIN_AttachSink(mt_handle hWin, mt_handle hSink);
mt_s32 WIN_DetachSink(mt_handle hWin, mt_handle hSink);
mt_s32 WIN_SetVirtualAttr(mt_handle hWin, mt_u32 u32Width, mt_u32 u32Height);

mt_s32 WIN_SetQuick(mt_handle hWin, MT_BOOL bEnable);
mt_s32 WIN_GetQuick(mt_handle hWin, MT_BOOL *pbEnable);

mt_s32 WIN_CapturePicture(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);
mt_s32 WIN_CapturePictureRelease(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);

mt_s32 WIN_SetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E enRotation);
mt_s32 WIN_GetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E *penRotation);

mt_s32 WIN_SetFlip(mt_handle hWin, MT_BOOL bHoriFlip, MT_BOOL bVertFlip);
mt_s32 WIN_GetFlip(mt_handle hWin, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip);

mt_s32 WIN_GetUnload(mt_handle hWin, mt_u32 *pu32Times);
mt_s32 WIN_SetPara(MT_DRV_WIN_PARA_S *arg);
mt_s32 WIN_GetPara(MT_DRV_WIN_PARA_S *arg);
mt_s32 WIN_ClearAllFrame(MT_DRV_WIN_PARA_S *arg);

mt_s32 WIN_SendFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
mt_s32 WIN_DestroyStillFrame(MT_DRV_VIDEO_FRAME_S *pStillFrameinfo);
mt_u32 WinGetIndex(mt_handle hWin, MT_DRV_DISPLAY_E *enDisp, mt_u32 *u32WinIndex);
mt_s32 WinForceClearCapture(mt_handle hWin);
mt_s32 WIN_Test(mt_handle hWin);
typedef struct tagWIN_HANDLE_ARRAY_S
{
    mt_u32 u32WinNumber;
    mt_handle ahWinHandle[DEF_MAX_WIN_NUM_ON_SINGLE_DISP];
} WIN_HANDLE_ARRAY_S;

mt_s32 Win_DebugGetHandle(MT_DRV_DISPLAY_E enDisp, WIN_HANDLE_ARRAY_S *pstWin);

mt_u32 WinGetPrefix(mt_u32 u32WinIndex);
mt_u32 WinGetDispId(mt_u32 u32WinIndex);
mt_u32 WinGetId(mt_u32 u32WinIndex);
mt_u32 Win_GetVideoWinIndex(mt_void);
mt_u32 Win_GetStillWinIndex(mt_void);

#define WIN_PROC_BUFFER_MAX_NUMBER 32
#define WIN_PROC_DEBUG_FRAME_RECORD_NUMBER 32
typedef struct tagWINBUF_STATE_S
{
    mt_u32 u32Number;

    mt_u32 u32EmptyRPtr;
    mt_u32 u32EmptyWPtr;

    mt_u32 u32FullRPtr;
    mt_u32 u32FullWPtr;

    struct
    {
        mt_u32 u32State;
        mt_u32 u32Empty;
        mt_u32 u32Full;
        mt_u32 u32FrameIndex;
    } stNode[WIN_PROC_BUFFER_MAX_NUMBER];

    struct
    {
        mt_u32 u32RecordNumber;
        
        mt_u32 u32InputFrameID[WIN_PROC_DEBUG_FRAME_RECORD_NUMBER];
        mt_u32 u32InputPos;
        mt_u32 u32Input;
        
        mt_u32 u32CfgFrameID[WIN_PROC_DEBUG_FRAME_RECORD_NUMBER];
        mt_u32 u32CfgPos;
        mt_u32 u32Config;
        
        mt_u32 u32RlsFrameID[WIN_PROC_DEBUG_FRAME_RECORD_NUMBER];
        mt_u32 u32RlsPos;
        mt_u32 u32Release;
        
        mt_u32 u32TryQueueFrame;
        mt_u32 u32QueueFrame;
        mt_u32 u32Underload;
        mt_u32 u32Disacard;
    } stRecord;

    mt_u32 u32EmptyArray[WIN_PROC_BUFFER_MAX_NUMBER];
    mt_u32 u32FullArray[WIN_PROC_BUFFER_MAX_NUMBER];

    MT_DRV_VIDEO_FRAME_S stCurrentFrame;
} WINBUF_STATE_S;

#if 1
typedef struct mtWIN_RROC_FOR_DEVELOPER_S
{
    /*sr dci location in layer, for debug use.*/
    VIDEO_LAYER_PROC_S stSrDciPhysicalInfo;
    /*the current win num, if > 2, sr not open.*/
    mt_u32 u32WinNum;

    /*only in 4k, sr can be opened now.*/
    mt_rect_s eCurrentFmt;
    /*when scaled down ,sr should not be opened.*/
    MT_BOOL bExistScaleDown_WhenRatioRevise;
    /*whether in 3d or not.*/
    MT_BOOL bIn3DMode;

    /*sr effect activated or not;*/
    MT_BOOL bHorSrOpenInPreProcess;
    MT_BOOL bVerSrOpenInPreProcess;

    MT_BOOL bHorSrOpenInPostProcess;
    MT_BOOL bVerSrOpenInPostProcess;

    /*frame size.*/
    mt_rect_s stOringinFrameSize;
    mt_rect_s stFinalWinOutputSize;
    mt_rect_s stOutputSizeOfV0;

    mt_rect_s stVdpRequire;
    mt_rect_s stVpssGive;

    mt_rect_s stSrOutputSize;

    /*the information of dci.*/
    MT_BOOL bDciOpen;
    mt_rect_s stOriginDCIPositionInFrame;
    mt_rect_s stDciFrameSize;

    /*dci input size, the finale position should give a revise.*/
    mt_rect_s stDciEffecttiveContentInputSize;
    mt_rect_s stWinFinalPosition;

} MT_WIN_RROC_FOR_DEVELOPER_S;
#endif

typedef struct tagWIN_PROC_INFO_S
{
    MT_DRV_WIN_TYPE_E enType;
    mt_u32 u32Index;
    mt_u32 u32Zorder;
    mt_u32 u32LayerId;
    mt_u32 u32LayerRegionNo;

    MT_BOOL bEnable;
    MT_BOOL bMasked;
    mt_u32 u32WinState;

    MT_BOOL bReset;
    MT_DRV_WIN_SWITCH_E enResetMode;
    MT_DRV_WIN_SWITCH_E enFreezeMode;

    MT_BOOL bQuickMode;
    MT_BOOL bStepMode;
    MT_BOOL bVirtualCoordinate;

    /* not change when window lives */
    /* source info */
    mt_u32 hSrc;
    ulong pfAcqFrame;
    ulong pfRlsFrame;
    ulong pfSendWinInfo;

    /* attribute */
    //MT_DISP_DISPLAY_INFO_S stRefDispInfo;
    MT_DRV_WIN_ATTR_S stAttr;
    MT_DRV_DISP_STEREO_E eDispMode;
    MT_BOOL bRightEyeFirst;

    /* slave window for HD&SD display the same content at the same time */
    mt_handle hSlvWin;

    /* debug info */
    MT_BOOL bDebugEn;
    mt_u32 u32TBNotMatchCount;

    /* statistic info */
    //WIN_STATISTIC_S stStatistic;
    mt_u32 u32ULSIn;
    mt_u32 u32ULSOut;
    mt_u32 u32UnderLoad;
    /*  buffer state */
    WINBUF_STATE_S stBufState;

    /*only for developer or mainteiner.*/
    MT_WIN_RROC_FOR_DEVELOPER_S stWinInfoForDeveloper;
} WIN_PROC_INFO_S;

mt_s32 WinGetProcIndex(mt_handle hWin, mt_u32 *p32Index);
mt_s32 WinGetProcInfo(mt_handle hWin, WIN_PROC_INFO_S *pstInfo);

mt_s32 WinGetCurrentImg(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 WinCaptureFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstFrame, mt_u32 *stMMZPhyAddr, mt_u32 *stMMZlen);
mt_s32 WinReleaseCaptureFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 WinFreeCaptureMMZBuf(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *cap_frame);
mt_s32 WinCapturePause(mt_handle hWin, MT_BOOL bCaptureStart);
mt_s32 WinReleaseStillFrame(MT_DRV_VIDEO_FRAME_S *pStillFrameInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*  __DRV_WINDOW_H__  */
