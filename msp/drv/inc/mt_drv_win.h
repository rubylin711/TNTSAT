/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : mt_drv_win.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/

#ifndef __MT_DRV_WIN_H__
#define __MT_DRV_WIN_H__

#include "mt_drv_disp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


#ifndef MT_ADVCA_FUNCTION_RELEASE
#define MT_FATAL_WIN(fmt...) \
            MT_FATAL_PRINT(MT_ID_VO, fmt)

#define MT_ERR_WIN(fmt...) \
            MT_ERR_PRINT(MT_ID_VO, fmt)

#define MT_WARN_WIN(fmt...) \
            MT_WARN_PRINT(MT_ID_VO, fmt)

#define MT_INFO_WIN(fmt...) \
            MT_INFO_PRINT(MT_ID_VO, fmt)
#else

#define MT_FATAL_WIN(fmt...)
#define MT_ERR_WIN(fmt...)
#define MT_WARN_WIN(fmt...)
#define MT_INFO_WIN(fmt...)
#endif

#define DEF_MAX_WIN_NUM_ON_SINGLE_DISP 16
#define DEF_MAX_WIN_NUM_ON_VIRTUAL_DISP 16

/* window type */
typedef enum mtDRV_WIN_TYPE_E
{
    MT_DRV_WIN_ACTIVE_SINGLE = 0,
    MT_DRV_WIN_VITUAL_SINGLE,
    MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE,
    MT_DRV_WIN_ACTIVE_SLAVE,
    MT_DRV_WIN_BUTT
}MT_DRV_WIN_TYPE_E;


/* window swtich mode, in reset mode */
typedef enum mtDRV_WIN_SWITCH_E
{
    MT_DRV_WIN_SWITCH_LAST = 0,
    MT_DRV_WIN_SWITCH_BLACK = 1,
    MT_DRV_WIN_SWITCH_BUTT
} MT_DRV_WIN_SWITCH_E;

/* window attribute */
typedef struct mtDRV_WIN_ATTR_S
{
    MT_BOOL bVirtual;
    MT_BOOL bUseSubLayer;
    MT_BOOL bSetVideoBot;
    /* not change when window lives */
    MT_DRV_DISPLAY_E  enDisp;

    /* may change when window lives */
    MT_DRV_ASPECT_RATIO_S stCustmAR;
    MT_DRV_ASP_RAT_MODE_E enARCvrs;

    MT_BOOL bUseCropRect;
    mt_rect_s stInRect;
    MT_DRV_CROP_RECT_S stCropRect;

    mt_rect_s stOutRect;

    /* only for virtual window */
    MT_BOOL             bUserAllocBuffer;
    mt_u32              u32BufNumber; /* [1,16] */
    MT_DRV_PIX_FORMAT_E enDataFormat;
}MT_DRV_WIN_ATTR_S;

/* window information */
typedef struct mtDRV_WIN_INFO_S
{
    MT_DRV_WIN_TYPE_E eType;

    mt_handle hPrim;
    mt_handle hSec;
}MT_DRV_WIN_INFO_S;
/* window information */
typedef struct mtDRV_WIN_PARA_S
{
    mt_handle hwin;
    mt_u32              u32ctlcmd;
    mt_u64              send[10];
}MT_DRV_WIN_PARA_S;

typedef mt_s32 (*PFN_GET_FRAME_CALLBACK)(mt_handle hHd, MT_DRV_VIDEO_FRAME_S *pstFrm);
typedef mt_s32 (*PFN_PUT_FRAME_CALLBACK)(mt_handle hHd, MT_DRV_VIDEO_FRAME_S *pstFrm);
typedef mt_s32 (*PFN_GET_WIN_INFO_CALLBACK)(mt_handle hHd, MT_DRV_WIN_PRIV_INFO_S *pstWin);

/* source information.
   window will get / release frame or send private info to sourec
   by function pointer */
typedef struct mtDRV_WIN_SRC_INFO_S
{
    mt_handle hSrc;

    PFN_GET_FRAME_CALLBACK pfAcqFrame;
    PFN_PUT_FRAME_CALLBACK pfRlsFrame;
    PFN_GET_WIN_INFO_CALLBACK pfSendWinInfo;

	//Extend Freeze interfaces
    PFN_GET_FRAME_CALLBACK pfAcqFreezeFrame;
    PFN_PUT_FRAME_CALLBACK pfRlsFreezeFrame;

    mt_u32    u32Resrve0;
    mt_u32    u32Resrve1;
}MT_DRV_WIN_SRC_INFO_S;

/* window current play information, player gets it and adjust Audio and Video
   play rate */
typedef struct mtDRV_WIN_PLAY_INFO_S
{
    mt_u32    u32DelayTime; /* in ms */
    mt_u32    u32DispRate;  /* in 1/100 Hz */
    mt_u32    u32FrameNumInBufQn;
    MT_DRV_VIDEO_FRAME_S    newest_playframeinfo;
    MT_BOOL   bTBMatch;  /* for interlace frame display on interlace timing */
}MT_DRV_WIN_PLAY_INFO_S;

typedef struct mtDRV_WIN_INTF_S
{
    mt_void* pfAcqFrame;
    mt_void* pfRlsFrame;
    mt_void* pfSetWinAttr;

    mt_u32    u32Resrve0;
    mt_u32    u32Resrve1;
}MT_DRV_WIN_INTF_S;

typedef struct mtDRV_WIN_PRIV_DATA_S
{
    struct clk *hdclk;
    struct clk *hd_os_clk;
    struct clk *sdclk_108m;
    struct clk *sdclk_27m;
    struct clk *vbiclk;
}MT_DRV_WIN_PRIV_DATA;

mt_s32 MT_DRV_WIN_Create(MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWindow);
mt_s32 MT_DRV_WIN_Destroy(mt_handle hWindow);
mt_s32 MT_DRV_WIN_SetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr);
mt_s32 MT_DRV_WIN_GetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr);
mt_s32 MT_DRV_WIN_GetInfo(mt_handle hWindow, MT_DRV_WIN_INFO_S *pInfo);
mt_s32 MT_DRV_WIN_SetSource(mt_handle hWindow, MT_DRV_WIN_SRC_INFO_S *pstSrc);
mt_s32 MT_DRV_WIN_GetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc);
mt_s32 MT_DRV_WIN_SetEnable(mt_handle hWindow, MT_BOOL bEnable);
mt_s32 MT_DRV_WIN_GetEnable(mt_handle hWindow, MT_BOOL *pbEnable);
mt_s32 MT_DRV_WIN_QFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);
mt_s32 MT_DRV_WIN_QULSFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo);
mt_s32 MT_DRV_WIN_DQFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);
mt_s32 MT_DRV_WIN_GetPlayInfo(mt_handle hWindow, MT_DRV_WIN_PLAY_INFO_S *pInfo);
mt_s32 MT_DRV_WIN_SetZorder(mt_handle hWin, MT_DRV_DISP_ZORDER_E ZFlag);
mt_s32 MT_DRV_WIN_GetZorder(mt_handle hWin, mt_u32 *pu32Zorder);

mt_s32 MT_DRV_WIN_Reset(mt_handle hWindow, MT_DRV_WIN_SWITCH_E enMode);

mt_s32 MT_DRV_WIN_SendFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);
mt_s32 MT_DRV_WIN_Freeze(mt_handle hWin, MT_BOOL bEnable,  MT_DRV_WIN_SWITCH_E eRst);
mt_s32 MT_DRV_WIN_GetLatestFrameInfo(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *frame_info);
mt_s32 MT_DRV_WIN_Pause(mt_handle hWin, MT_BOOL bEnable);
mt_s32 MT_DRV_WIN_SetStepMode(mt_handle hWin, MT_BOOL bStepMode);
mt_s32 MT_DRV_WIN_SetStepPlay(mt_handle hWin);
mt_s32 MT_DRV_WIN_SetExtBuffer(mt_handle hWin, MT_DRV_VIDEO_BUFFER_POOL_S* pstBuf);
mt_s32 MT_DRV_WIN_AcquireFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
mt_s32 MT_DRV_WIN_ReleaseFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo);
mt_s32 MT_DRV_WIN_SetQuick(mt_handle hWin, MT_BOOL bEnable);
mt_s32 MT_DRV_WIN_CapturePicture(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);
mt_s32 MT_DRV_WIN_CapturePictureRelease(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic);
mt_s32 MT_DRV_WIN_SetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E enRotation);
mt_s32 MT_DRV_WIN_GetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E *penRotation);
mt_s32 MT_DRV_WIN_SetFlip(mt_handle hWin, MT_BOOL bHoriFlip, MT_BOOL bVertFlip);
mt_s32 MT_DRV_WIN_GetFlip(mt_handle hWin, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip);
mt_s32 MT_DRV_WIN_SetPARA(mt_handle hWin, MT_DRV_WIN_PARA_S * para);
mt_s32 MT_DRV_WIN_GetPARA(mt_handle hWin, MT_DRV_WIN_PARA_S * para);

mt_s32 MT_DRV_WIN_Init(mt_void);
mt_s32 MT_DRV_WIN_DeInit(mt_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MT_DRV_WIN_H__ */


