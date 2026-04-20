
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_win_priv.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_WIN_PRIV_H__
#define __DRV_WIN_PRIV_H__

#include "drv_disp_com.h"
#include "drv_win_hal.h"
#include "drv_win_prc.h"
#include "drv_win_buffer.h"
//#include "MT_DF_disp.h"
#include "drv_disp_buffer.h"
#include "drv_virtual.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/* window state */
#define WIN_DEVICE_STATE_CLOSE   0
#define WIN_DEVICE_STATE_OPEN    1
#define WIN_DEVICE_STATE_SUSPEND 2

#define WIN_DISPLAY_MAX_NUMBER 17
#define WIN_VIRTAUL_MAX_NUMBER 17

#define WIN_INDEX_PREFIX_MASK 0xFFFF0000ul
#define WIN_INDEX_PREFIX_SHIFT_NUMBER 16
#define WIN_INDEX_PREFIX      ((mt_u32)MT_ID_VO << WIN_INDEX_PREFIX_SHIFT_NUMBER)

/*VIRTUAL PREFIX*/
//#define WIN_INDEX_VIRTUAL_PREFIX  ((((mt_u32)MT_ID_VO << WIN_INDEX_PREFIX_SHIFT_NUMBER))|0x5a000000)
#define WIN_INDEX_VIRTUAL_CHANNEL ((mt_u32)0xF)

#define WIN_INDEX_DISPID_SHIFT_NUMBER 8
#define WIN_INDEX_DISPID_MASK 0x0000000Ful
#define WIN_INDEX_MASK        0x000000FFul

#define WIN_INDEX_SLAVE_PREFIX  0x00000080ul

#define WIN_FRAME_MIN_WIDTH  64
#define WIN_FRAME_MIN_HEIGHT 64
#define WIN_FRAME_MAX_WIDTH  7680
#define WIN_FRAME_MAX_HEIGHT 4320

#define WIN_MAX_ASPECT_RATIO 16

#define WIN_INRECT_MIN_WIDTH   64
#define WIN_INRECT_MAX_WIDTH   7680
#define WIN_INRECT_MIN_HEIGHT  64
#define WIN_INRECT_MAX_HEIGHT  4320

#define WIN_OUTRECT_MIN_WIDTH  64
#define WIN_OUTRECT_MAX_WIDTH  4096
#define WIN_OUTRECT_MIN_HEIGHT 64
#define WIN_OUTRECT_MAX_HEIGHT 2160

#define WIN_CROPRECT_MAX_OFFSET_TOP     128
#define WIN_CROPRECT_MAX_OFFSET_LEFT    128
#define WIN_CROPRECT_MAX_OFFSET_BOTTOM  128
#define WIN_CROPRECT_MAX_OFFSET_RIGHT   128


/* in 1/100 Hz */
#define WIN_MAX_FRAME_RATE   12000
#define WIN_TRANSFER_CODE_MAX_FRAME_RATE 3000

#define WIN_MAX_FRAME_PLAY_TIME  1

#define WIN_IN_FB_DEFAULT_NUMBER 32//by qiuying's requirement
#define WIN_USING_FB_MAX_NUMBER 2

#define WIN_USELESS_FRAME_MAX_NUMBER  16

typedef struct tagWIN_CONFIG_S
{
    MT_DRV_WIN_ATTR_S stAttr;

    MT_DRV_DISP_STEREO_E eDispMode;
    MT_BOOL bRightEyeFirst;

    MT_DRV_WIN_ATTR_S stAttrBuf;
    atomic_t bNewAttrFlag;

    MT_DRV_WIN_SRC_INFO_S stSource;

    /* may change when window lives */
    MT_BOOL bQuickOutput;

    /*  */
    MT_DRV_COLOR_SPACE_E enFrameCS;
    mt_u32 u32Fidelity;
    MT_DRV_COLOR_SPACE_E enOutCS;
}WIN_CONFIG_S;

typedef struct tagWIN_STATISTIC_S
{
    mt_u32 u32Reserved;

}WIN_STATISTIC_S;

typedef struct tagWIN_DEBUG_S
{
    mt_u32 u32Reserved;

}WIN_DEBUG_S;

typedef enum tagWIN_FRAME_TYPE_E
{
    WIN_FRAME_NORMAL = 0,
    WIN_FRAME_BLACK,
    WIN_FRAME_FREEZE,
    WIN_FRAME_TYPE_BUTT
}WIN_FRAME_TYPE_E;

typedef struct tagWIN_BUF_NODE_S
{
    MT_DRV_VIDEO_FRAME_S stFrame;
    mt_u32 u32BufId;
    MT_BOOL bIdle;
    WIN_FRAME_TYPE_E enType;
}WIN_BUF_NODE_S;

#define WIN_INVALID_BUFFER_INDEX 0xfffffffful
#define WIN_BLACK_FRAME_INDEX   0xf0000001ul


typedef struct tagWIN_BUFFER_S
{
    mt_handle stDispBP;
    WB_POOL_S stWinBP;

    /* for  frame buffer not used ,should moved to wb_pool_s next.*/
    MT_DRV_VIDEO_FRAME_S stUselessFrame[WIN_USELESS_FRAME_MAX_NUMBER];
    mt_u32 u32ULSRdPtr;
    mt_u32 u32ULSWtPtr;
    mt_u32 u32ULSIn;
    mt_u32 u32ULSOut;


    mt_u32 u32UnderLoad;
}WIN_BUFFER_S;

typedef struct tagWIN_FREEZE_PRIV_S
{
    MT_DRV_WIN_SWITCH_E enFreezeMode;
    mt_u32 u32Reserve;
}WIN_FREEZE_PRIV_S;

typedef struct tagWIN_RESET_PRIV_S
{
    MT_DRV_WIN_SWITCH_E enResetMode;
    mt_u32 u32Reserve;
}WIN_RESET_PRIV_S;

typedef enum tagWIN_STATE_E
{
    WIN_STATE_WORK = 0,
    WIN_STATE_PAUSE,
    WIN_STATE_RESUME,
    WIN_STATE_FREEZE,
    WIN_STATE_UNFREEZE,
    WIN_STATE_BUTT
}WIN_STATE_E;

typedef struct tagWIN_DELAY_INFO_S
{
    mt_u32 u32DispRate;  /* in 1/100 Hz */
    mt_u32 T;  /* in ms */

//    volatile mt_u32 u32FrameNumber;
    volatile mt_u32 u32DisplayTime;
    volatile mt_u32 u32CfgTime;
    MT_BOOL bInterlace;
    volatile MT_BOOL bTBMatch;  /* for interlace frame display on interlace timing */
}WIN_DELAY_INFO_S;


typedef struct tagWIN_DISP_INFO_S
{
    mt_u32 u32RefreshRate;
    MT_BOOL bIsInterlace;
    MT_BOOL bIsBtm;
    mt_rect_s stWinInitialFmt;
    mt_rect_s stWinCurrentFmt;
}WIN_DISP_INFO_S;

typedef enum tagWINLAYER_OPT_TYPE_E {
    LAYER_OPT_ZORDER_ADJUST = 0,
    LAYER_OPT_BUTT
}WINLAYER_OPT_TYPE_E;

typedef struct tagWIN_LAYER_OPT_S {
    WINLAYER_OPT_TYPE_E layerOptType;
    MT_BOOL             bEffective;
    mt_void             *pParam;
}WIN_LAYER_OPT_S;

typedef struct hiDRV_VIDEO_TMP_INFO_S
{
    mt_u32 u32WinInWidth;
    mt_u32 u32WinInHeight;
    mt_u32 u32WinOutWidth;
    mt_u32 u32WinOutHeight;

    MT_DRV_PIX_FORMAT_E  enPixFormat;
    mt_u32 u32FrameWidth;
    mt_u32 u32FrameHeight;

    mt_u32 u32DispWidth;
    mt_u32 u32DispHeight;
}MT_DRV_VIDEO_TMP_INFO_S;

typedef struct hiDRV_VIDEO_MISC_INFO_S
{
    mt_rect_s stFrameOriginalRect;
    mt_rect_s stWinOutRect;
    MT_BOOL  bWinSrEnableCurrent;
    MT_BOOL  bWinSrEnableLast;

 /*   MT_PQ_DCI_WIN_S  stWinDciLastConfig;*/
    MT_BOOL  bWinDciEnableLastConfig;


}MT_DRV_VIDEO_MISC_INFO_S;

typedef struct tagWINDOW_S
{
    mt_u32 u32Index;

    /* state */
    MT_BOOL bEnable;
    MT_BOOL bMasked;

    MT_DRV_DISPLAY_E  enDisp;
    MT_DRV_WIN_TYPE_E enType;

    /*the location*/
    mt_u32  u32VideoLayer;
    mt_u32  u32VideoLayerNew;

    mt_u32  u32VideoRegionNo;
    mt_u32  u32VideoRegionNoNew;

    /* window config */
    WIN_CONFIG_S stCfg;
    MT_BOOL bDispInfoChange;

    /* private attribute */
    MT_DRV_WIN_ATTR_S stUsingAttr;

    volatile MT_BOOL bUpState;
    volatile WIN_STATE_E enState;
    volatile WIN_STATE_E enStateNew;

    //reset flag
    volatile MT_BOOL bReset;
    WIN_RESET_PRIV_S stRst;

    /*resume flag.*/
    volatile MT_BOOL bNeedResume;

    // freeze flag
    WIN_FREEZE_PRIV_S stFrz;

    // quickout flag
    MT_BOOL bQuickMode;

    // stepmode flag
    MT_BOOL bStepMode;
    MT_BOOL bVirtScreenMode;

    /* play info */
    volatile WIN_DELAY_INFO_S stDelayInfo;
    volatile MT_BOOL bInInterrupt;

    /* lowdelay report idx*/
    mt_u32 u32LastInLowDelayIdx;
    mt_u32 u32LastOutLowDelayIdx;

    /* statistic info */
    WIN_STATISTIC_S stStatistic;

    /* debug info */
    MT_BOOL     bDebugEn;
    WIN_DEBUG_S stDebug;

    /*  buffer */
    WIN_BUFFER_S stBuffer;
    MT_BOOL bConfigedBlackFrame;

    /* slave window for HD&SD display the same content at the same time */
    mt_handle hSlvWin;
    mt_handle pstMstWin;

    WIN_DISP_INFO_S stDispInfo;
    mt_u32 u32TBTestCount;
    mt_u32 u32TBNotMatchCount;

    MT_DRV_ROT_ANGLE_E enRotation;
    MT_BOOL bVertFlip;
    MT_BOOL bHoriFlip;
    /* video surface function */
    VIDEO_LAYER_FUNCTIONG_S stVLayerFunc;

    /*the lastest frame may be a member of window.
     * this may be a temporary modify,should be discussed with others.
     */
    MT_DRV_VIDEO_FRAME_S  latest_display_frame;
    mt_u32                latest_display_frame_valid;
    DISP_MMZ_BUF_S        stWinCaptureMMZ;
    mt_u32                u32WinCapMMZvalid;
    MT_BOOL               bRestoreFlag;
    mt_u32                u32Zorder;

    WIN_LAYER_OPT_S       stWinLayerOpt;
    MT_DRV_VIDEO_TMP_INFO_S stVideoTmpInfo;

    /*only for developer or mainteiner.*/
    MT_WIN_RROC_FOR_DEVELOPER_S stWinInfoForDeveloper;

    /*misc information, difficult to classify.*/
    MT_DRV_VIDEO_MISC_INFO_S stMiscInfor;
}WINDOW_S;



#define MAX_RELEASE_NO  16

typedef  struct task_struct*    WIN_THREAD;
typedef   wait_queue_head_t    WAIT_QUEUE_HAEAD;

typedef enum tagTHREAT_EVENT_E
{
    EVENT_NOTHING= 0,
    EVENT_RELEASE,
    EVENT_BUTT
}THREAT_EVENT_E;

typedef struct hiWIN_RELEASE_FRM_S
{
    WAIT_QUEUE_HAEAD  stWaitQueHead;
    THREAT_EVENT_E  enThreadEvent;
    WIN_THREAD  hThread;

     MT_DRV_VIDEO_FRAME_S *pstNeedRelFrmNode[MAX_RELEASE_NO];
} WIN_RELEASE_FRM_S;

typedef struct tagDISPLAY_WINDOW_S
{
    WINDOW_S *pstWinArray[MT_DRV_DISPLAY_BUTT][WINDOW_MAX_NUMBER];
    mt_u32    u32WinNumber[MT_DRV_DISPLAY_BUTT];
    VIDEO_LAYER_FUNCTIONG_S stVSurfFunc;
    WIN_RELEASE_FRM_S stWinRelFrame;
    MT_BOOL  bWinManageStatus;

    /*if window system changed, this is a event.*/
    atomic_t   bWindowSytemUpdate[MT_DRV_DISPLAY_BUTT];
}DISPLAY_WINDOW_S;


typedef struct tagVIRTUAL_WINDOW_S
{
    mt_u32    u32WinNumber;
    VIRTUAL_S *pstWinArray[WIN_VIRTAUL_MAX_NUMBER];
}VIRTUAL_WINDOW_S;



#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_VO_PRIV_H__  */










