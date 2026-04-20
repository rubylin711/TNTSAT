/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : drv_win_ioctl.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/
#ifndef __DRV_WIN_IOCTL_H__
#define __DRV_WIN_IOCTL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include "mt_type.h"
#include "mt_drv_video.h"
#include "mt_drv_disp.h"
#include "mt_drv_win.h"


typedef struct mtWIN_CREATE_S
{
    mt_handle          hWindow;
    MT_DRV_WIN_ATTR_S  WinAttr;
    MT_BOOL            bVirtScreen;
}WIN_CREATE_S;

typedef struct mtWIN_ENABLE_S
{
    mt_handle hWindow;
    MT_BOOL   bEnable;
}WIN_ENABLE_S;

typedef struct mtWIN_SOURCE_S
{
    mt_handle hWindow;
    MT_DRV_WIN_SRC_INFO_S stSrc;
}WIN_SOURCE_S;


typedef struct mtWIN_PRIV_INFO_S
{
    mt_handle hWindow;
    MT_DRV_WIN_INFO_S stPrivInfo;
}WIN_PRIV_INFO_S;

typedef struct mtWIN_PLAY_INFO_S
{
    mt_handle hWindow;
    MT_DRV_WIN_PLAY_INFO_S stPlayInfo;
}WIN_PLAY_INFO_S;

typedef struct mtWIN_ZORDER_S
{
    mt_handle            hWindow;
    MT_DRV_DISP_ZORDER_E eZFlag;
}WIN_ZORDER_S;

typedef struct mtWIN_ORDER_S
{
    mt_handle    hWindow;
    mt_u32       Order;
}WIN_ORDER_S;

typedef struct mtWIN_FREEZE_S
{
    mt_handle hWindow;
    MT_BOOL   bEnable;
    MT_DRV_WIN_SWITCH_E eMode;
}WIN_FREEZE_S;

typedef struct mtWIN_RESET_S
{
    mt_handle hWindow;
    MT_DRV_WIN_SWITCH_E eMode;
}WIN_RESET_S;

typedef struct mtWIN_FRAME_S
{
    mt_handle hWindow;
    MT_DRV_VIDEO_FRAME_S stFrame;
}WIN_FRAME_S;

typedef struct mtWIN_CMD_FRAME_S
{
    mt_handle hWindow;
    MT_DRV_VIDEO_FRAME_S *pstFrame;
}WIN_CMD_FRAME_S;

typedef struct mtWIN_UNLOAD_S
{
    mt_handle hWindow;
    mt_u32 u32Times;
}WIN_UNLOAD_S;

typedef struct mtWIN_CAPTURE_DRIVER_SUPPLY_ADDR_S
{
    mt_u32        startPhyAddr;
    mt_u32        length;    
}WIN_CAPTURE_DRIVER_SUPPLY_ADDR_S;

typedef struct mtVO_WIN_CAPTURE_FREE_S
{
    mt_handle                        hWindow;
    MT_DRV_VIDEO_FRAME_S             CapPicture; 
    WIN_CAPTURE_DRIVER_SUPPLY_ADDR_S driver_supply_addr;
}WIN_CAPTURE_S;

typedef struct mtWIN_PAUSE_S
{
    mt_handle hWindow;
    MT_BOOL   bEnable;
}WIN_PAUSE_S;

typedef struct mtWIN_STEP_S
{
    mt_handle hWindow;
    MT_BOOL   bStep;
}WIN_STEP_MODE_S;

typedef struct mtWIN_STATE_S
{
    mt_handle hWin[MT_DRV_DISPLAY_BUTT][DEF_MAX_WIN_NUM_ON_SINGLE_DISP];
    mt_handle hVirtualWin[DEF_MAX_WIN_NUM_ON_VIRTUAL_DISP];
    mt_handle hCapture[MT_DRV_DISPLAY_BUTT][DEF_MAX_WIN_NUM_ON_SINGLE_DISP];
}WIN_STATE_S;

typedef struct mtWIN_ROTATION_S
{
    mt_handle hWindow;
    MT_DRV_ROT_ANGLE_E enRotation;
}WIN_ROTATION_S;
typedef struct mtWIN_FLIP_S
{
    mt_handle hWindow;
    MT_BOOL bVertFlip;
    MT_BOOL bHoriFlip;
}WIN_FLIP_S;
/*
typedef struct mtWIN_GLOBAL_STATE_S
{
    mt_u32 bDisp0Num;
    mt_u32 bDisp1Num;    
    mt_u32 WinCount[MT_DRV_DISPLAY_BUTT];
}WIN_GLOBAL_STATE_S;
*/
typedef struct mtWIN_PRIV_PARA_S
{
    mt_handle hWindow;
    MT_DRV_WIN_INFO_S stPrivInfo;
}WIN_PRIV_PARA_S;

typedef struct mtWIN_BUF_POOL_S
{
    mt_handle hwin;
    MT_DRV_VIDEO_BUFFER_POOL_S stBufPool;
}WIN_BUF_POOL_S;

typedef struct mtWIN_SET_QUICK_S
{
    mt_handle hWindow;
    MT_BOOL   bQuickEnable;
}WIN_SET_QUICK_S;

typedef struct mtWIN_GET_HANDLE_S
{
    MT_DRV_DISPLAY_E enDisp;
    mt_u32 u32WinNumber;
    mt_handle ahWinHandle[DEF_MAX_WIN_NUM_ON_SINGLE_DISP];
}WIN_GET_HANDLE_S;

typedef enum mtWIN_ATTACH_TYPE_E
{
    ATTACH_TYPE_SRC = 0,
    ATTACH_TYPE_SINK,
    ATTACH_TYPE_BUTT
}WIN_ATTACH_TYPE_E;

typedef struct mtWIN_ATTACH_S
{
    WIN_ATTACH_TYPE_E enType;
    mt_handle hWindow;
    mt_handle hMutual;
}WIN_ATTACH_S;

typedef struct mtWIN_INTF_S
{
    mt_void* pfAcqFrame;
    mt_void* pfRlsFrame;
    mt_void* pfSetWinAttr;
}WIN_INTF_S;

typedef enum mtIOC_VO_E
{
    IOC_WIN_CREATE = 0,
    IOC_WIN_DESTROY,

    IOC_WIN_SET_ENABLE,
    IOC_WIN_GET_ENABLE,

    IOC_WIN_SET_ATTR,
    IOC_WIN_GET_ATTR,

    IOC_WIN_SET_SOURCE,
    IOC_WIN_GET_SOURCE,

    IOC_WIN_GET_INFO,
    IOC_WIN_GET_PLAY_INFO,

    IOC_WIN_QU_FRAME,
    IOC_WIN_QU_ULSFRAME,
    IOC_WIN_DQ_FRAME,

    IOC_WIN_FREEZE,
    IOC_WIN_GET_FREEZE_STATUS,
    IOC_WIN_RESET,
    IOC_WIN_PAUSE,
    IOC_WIN_SET_QUICK,
    IOC_WIN_GET_QUICK,
    
    IOC_WIN_SET_ZORDER,
    IOC_WIN_GET_ORDER,

    IOC_WIN_CAP_FRAME,
    IOC_WIN_CAP_REL_FRAME,
    
    IOC_WIN_SEND_FRAME,

    IOC_WIN_STEP_MODE,
    IOC_WIN_STEP_PLAY,

    IOC_WIN_VIR_ACQUIRE,
    IOC_WIN_VIR_RELEASE,
    IOC_WIN_VIR_EXTERNBUF,

    IOC_WIN_SUSPEND,
    IOC_WIN_RESUME,
    
    IOC_WIN_ATTACH,
    IOC_WIN_DETACH,
    
    IOC_WIN_GET_INTF,
    IOC_WIN_GET_LATESTFRAME_INFO,
    IOC_VO_WIN_CAPTURE_START,
    IOC_VO_WIN_CAPTURE_RELEASE,
    IOC_VO_WIN_CAPTURE_FREE,
    IOC_WIN_SET_ROTATION,
    IOC_WIN_GET_ROTATION,
    IOC_WIN_SET_FLIP,
    IOC_WIN_GET_FLIP,
    IOC_WIN_GET_UNLOAD,
    IOC_WIN_DEBUG_GET_HANDLE,
    IOC_WIN_SET_PARA,
    IOC_WIN_GET_PARA,
    IOC_WIN_CLEAN_ALLFRAME,

    IOC_WIN_BUTT
} IOC_WIN_E;


#define CMD_WIN_CREATE             _IOWR(MT_ID_VO, IOC_WIN_CREATE, WIN_CREATE_S)
#define CMD_WIN_DESTROY            _IOW(MT_ID_VO, IOC_WIN_DESTROY, mt_handle)

#define CMD_WIN_SET_ENABLE         _IOW(MT_ID_VO, IOC_WIN_SET_ENABLE, WIN_ENABLE_S)
#define CMD_WIN_GET_ENABLE         _IOWR(MT_ID_VO, IOC_WIN_GET_ENABLE, WIN_ENABLE_S)

#define CMD_WIN_SET_ATTR           _IOW(MT_ID_VO, IOC_WIN_SET_ATTR, WIN_CREATE_S)
#define CMD_WIN_GET_ATTR           _IOWR(MT_ID_VO, IOC_WIN_GET_ATTR, WIN_CREATE_S)

#define CMD_WIN_SET_SOURCE         _IOW(MT_ID_VO, IOC_WIN_SET_SOURCE, WIN_SOURCE_S)
#define CMD_WIN_GET_SOURCE         _IOW(MT_ID_VO, IOC_WIN_GET_SOURCE, WIN_SOURCE_S)

#define CMD_WIN_GET_INFO          _IOWR(MT_ID_VO, IOC_WIN_GET_INFO, WIN_PRIV_INFO_S)
#define CMD_WIN_GET_PLAY_INFO      _IOWR(MT_ID_VO, IOC_WIN_GET_PLAY_INFO, WIN_PLAY_INFO_S)

//#define CMD_WIN_QU_FRAME           _IOW(MT_ID_VO, IOC_WIN_QU_FRAME, WIN_FRAME_S)
#define CMD_WIN_QU_FRAME           _IOW(MT_ID_VO, IOC_WIN_QU_FRAME, WIN_CMD_FRAME_S)
#define CMD_WIN_QU_ULSFRAME        _IOW(MT_ID_VO, IOC_WIN_QU_ULSFRAME, WIN_FRAME_S)
#define CMD_WIN_DQ_FRAME           _IOWR(MT_ID_VO, IOC_WIN_DQ_FRAME, WIN_FRAME_S)

#define CMD_WIN_FREEZE             _IOW(MT_ID_VO,  IOC_WIN_FREEZE, WIN_FREEZE_S)
#define CMD_WIN_GET_FREEZE_STATUS  _IOWR(MT_ID_VO, IOC_WIN_GET_FREEZE_STATUS, WIN_FREEZE_S)
#define CMD_WIN_RESET              _IOW(MT_ID_VO, IOC_WIN_RESET, WIN_RESET_S)
#define CMD_WIN_PAUSE              _IOW(MT_ID_VO, IOC_WIN_PAUSE, WIN_PAUSE_S)

#define CMD_WIN_SET_QUICK          _IOW(MT_ID_VO,IOC_WIN_SET_QUICK,WIN_SET_QUICK_S)
#define CMD_WIN_GET_QUICK          _IOWR(MT_ID_VO,IOC_WIN_GET_QUICK,WIN_SET_QUICK_S)
#define CMD_WIN_SET_ZORDER         _IOW(MT_ID_VO, IOC_WIN_SET_ZORDER, WIN_ZORDER_S)
#define CMD_WIN_GET_ORDER          _IOWR(MT_ID_VO, IOC_WIN_GET_ORDER, WIN_ORDER_S)


#define CMD_WIN_CAPTURE            _IOWR(MT_ID_VO, IOC_WIN_CAP_FRAME, WIN_CAPTURE_S)
#define CMD_WIN_CAP_RELEASE        _IOWR(MT_ID_VO, IOC_WIN_CAP_REL_FRAME, WIN_CAPTURE_S)

#define CMD_WIN_SEND_FRAME         _IOW(MT_ID_VO, IOC_WIN_SEND_FRAME, WIN_FRAME_S)

#define CMD_WIN_STEP_MODE          _IOW(MT_ID_VO, IOC_WIN_STEP_MODE, WIN_STEP_MODE_S)
#define CMD_WIN_STEP_PLAY          _IOW(MT_ID_VO, IOC_WIN_STEP_PLAY, mt_handle)

#define CMD_WIN_VIR_ACQUIRE        _IOWR(MT_ID_VO,IOC_WIN_VIR_ACQUIRE,WIN_FRAME_S)
#define CMD_WIN_VIR_RELEASE        _IOWR(MT_ID_VO,IOC_WIN_VIR_RELEASE,WIN_FRAME_S)
#define CMD_WIN_VIR_EXTERNBUF      _IOW(MT_ID_VO,IOC_WIN_VIR_EXTERNBUF,WIN_BUF_POOL_S)


#define CMD_WIN_SUSPEND            _IOW(MT_ID_VO,IOC_WIN_SUSPEND, ulong)
#define CMD_WIN_RESUM              _IOW(MT_ID_VO,IOC_WIN_RESUME, ulong)

#define CMD_WIN_GET_HANDLE         _IOWR(MT_ID_VO,IOC_WIN_DEBUG_GET_HANDLE, WIN_GET_HANDLE_S)

#define CMD_WIN_ATTACH             _IOWR(MT_ID_VO,IOC_WIN_ATTACH, WIN_ATTACH_S)
#define CMD_WIN_DETACH             _IOWR(MT_ID_VO,IOC_WIN_DETACH, WIN_ATTACH_S)

#define CMD_WIN_GET_INTF             _IOWR(MT_ID_VO,IOC_WIN_GET_INTF, WIN_INTF_S)

#define CMD_WIN_GET_LATESTFRAME_INFO _IOWR(MT_ID_VO,IOC_WIN_GET_LATESTFRAME_INFO, WIN_FRAME_S)
#define CMD_VO_WIN_CAPTURE_START     _IOWR(MT_ID_VO,IOC_VO_WIN_CAPTURE_START, WIN_CAPTURE_S)
#define CMD_VO_WIN_CAPTURE_RELEASE   _IOWR(MT_ID_VO,IOC_VO_WIN_CAPTURE_RELEASE, WIN_CAPTURE_S)
#define CMD_VO_WIN_CAPTURE_FREE      _IOWR(MT_ID_VO,IOC_VO_WIN_CAPTURE_FREE, WIN_CAPTURE_S)

#define CMD_WIN_SET_ROTATION      _IOWR(MT_ID_VO,IOC_WIN_SET_ROTATION, WIN_ROTATION_S)
#define CMD_WIN_GET_ROTATION      _IOWR(MT_ID_VO,IOC_WIN_GET_ROTATION, WIN_ROTATION_S)
#define CMD_WIN_SET_FLIP      _IOWR(MT_ID_VO,IOC_WIN_SET_FLIP, WIN_FLIP_S)
#define CMD_WIN_GET_FLIP      _IOWR(MT_ID_VO,IOC_WIN_GET_FLIP, WIN_FLIP_S)

#define CMD_WIN_GET_UNLOAD _IOWR(MT_ID_VO,IOC_WIN_GET_UNLOAD, WIN_UNLOAD_S)
#define CMD_WIN_SET_PARA          _IOW(MT_ID_VO, IOC_WIN_SET_PARA, WIN_PRIV_PARA_S)
#define CMD_WIN_GET_PARA          _IOWR(MT_ID_VO, IOC_WIN_GET_PARA, WIN_PRIV_PARA_S)
#define CMD_WIN_CLEAN_ALLFRAME          _IOW(MT_ID_VO, IOC_WIN_CLEAN_ALLFRAME , WIN_PRIV_PARA_S)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif  /* __DRV_WIN_IOCTL_H__ */

