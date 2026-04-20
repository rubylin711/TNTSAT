
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_cast.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_CAST_H__
#define __DRV_DISP_CAST_H__

//#include "drv_disp_com.h"
#include "vo_fw.h"

#include "drv_disp_hal.h"
#include "drv_disp_buffer.h"
#include "drv_disp_isr.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


#define DISP_CAST_MIN_W 320
#define DISP_CAST_MAX_W 1920
#define DISP_CAST_MIN_H 240
#define DISP_CAST_MAX_H 1080

#define DISP_CAST_BUFFER_MIN_NUMBER 3
#define DISP_CAST_BUFFER_MAX_NUMBER 16

#define DISP_CAST_LOWDLY_THRESHOLD_NUMERATOR  50
#define DISP_CAST_LOWDLY_THRESHOLD_DENOMINATOR 100
#define DISP_CAST_LOWDLY_LINENUM_INTERVAL     10
/* in ms */
#define DISP_CAST_MAX_FRAME_RATE 3000

#define CAST_BUFFER_STATE_IN_CFGLIST_WRITING      1
#define CAST_BUFFER_STATE_IN_CFGLIST_WRITE_FINISH 2



typedef enum hiCAST_RETRIVE_LIST_NODE_STATUS_E
{
    CAST_RETRIVE_NODE_EMPTY = 0,
    CAST_RETRIVE_NODE_REALLOCATE,
    CAST_RETRIVE_NODE_READY_TO_RETURN,
    CAST_RETRIVE_NODE_BUTT,
} CAST_RETRIVE_LIST_NODE_STATUS_E;

typedef struct tagDISP_CAST_ATTR_S
{
    MT_DRV_PIX_FORMAT_E eFormat; /* Support ... */

    mt_rect_s stIn;
    MT_BOOL bInterlace;
    mt_u32 u32InRate;
    MT_DRV_COLOR_SPACE_E eInColorSpace;

    mt_rect_s stOut;
    mt_u32 u32OutRate;
    MT_DRV_COLOR_SPACE_E eOutColorSpace;

    // store output informaiton
    MT_DRV_VIDEO_FRAME_S stFrameDemo;
}DISP_CAST_ATTR_S;

typedef struct tagDISP_CAST_PRIV_FRAME_S
{
    MT_DRV_VIDEO_PRIVATE_S stPrivInfo;

    mt_handle cast_ptr;
    mt_u32    u32Pts0;  /* create PTS */
    mt_u32    u32Pts1;  /* acquire PTS */

}DISP_CAST_PRIV_FRAME_S;

typedef struct tagDISP_SNAPSHOT_PRIV_FRAME_S
{
    MT_DRV_VIDEO_PRIVATE_S stPrivInfo;
    mt_u32 u32Magic;
    mt_u32 u32BPAddr;
}DISP_SNAPSHOT_PRIV_FRAME_S;

typedef enum
{
    MIRA_SET_CREATE_PTS = 0,
    MIRA_SET_AQUIRE_PTS,
    MIRA_FLAG_BUTT
}MIRA_GET_PTS_E;

typedef struct tagDISP_Attach_PAIR_S
{
    /*push mode, get the func ptr from back mode.*/
    mt_handle hSink;
    mt_void* pfnQueueFrm;
    mt_void* pfnDequeueFrame;
}DISP_Attach_PAIR_S;

typedef enum mtCAST_OUTBUF_STATUS_E
{
    CAST_OUTBUF_EMPTY = 0,
    CAST_OUTBUF_BE_WROTE,
    CAST_OUTBUF_BE_READ,
} CAST_OUTBUF_STATUS_E;

typedef struct mtDISP_CAST_OUTFRM_S
{
    mt_u32 u32RWStatus;
    MT_DRV_VID_FRAME_ADDR_S stBufAddr[MT_DRV_BUF_ADDR_MAX];
    PRESCALE_ORDERINFO stPreSclInfo;
    DISP_MMZ_BUF_S stMem;
}MT_DISP_CAST_OUTFRM_S;

#define DISP_CAST_OUTFRM_CNT_MAX 2
#define DISP_CAST_OUTFRM_BUFSZ (1920 * 1080)

#define DISPLAY_ATTACH_CNT_MAX   3

#define USE_PRE_LINEAR


//prescale input size, gra_scale0_output size
#define GRA_SCALE0_OUTSZ_W 1920
#define GRA_SCALE0_OUTSZ_H 1080


typedef struct tagDISP_CAST_S
{
    //state
    MT_BOOL bOpen;
    MT_BOOL bEnable;
    MT_BOOL bMasked;
    atomic_t bBufBusy;
    MT_BOOL bAttached;
  mt_u32 u32Ref;

    MT_BOOL bLowDelay;
  /* wbc controll between cast and snapshot. */
  /* Should cast shedule wbc? default no.  */
  volatile MT_BOOL  bScheduleWbc;
  volatile MT_BOOL  bScheduleWbcStatus;
    // cfg
    MT_DRV_DISP_CAST_CFG_S stConfig;

    // disp info
    MT_DRV_DISPLAY_E eDisp;
    //MT_BOOL bDispSet;
    //MT_BOOL bDispUpdate;
    MT_DISP_DISPLAY_INFO_S stDispInfo;
    MT_BOOL bToGetDispInfo;

    // private attr
    DISP_WBC_E eWBC;
    //MT_BOOL bAttrUpdate;
    DISP_CAST_ATTR_S stAttr;

    mt_u32 u32Periods;
    mt_u32 u32TaskCount;

    //mirrorcast
    //DISP_MIRACAST_S stMrCt;

    //algrithm operation

    // buffer
    BUF_POOL_S stBP;
    mt_u32 u32FrameCnt;
    mt_u32 u32LastCfgBufId;
    mt_u32 u32LastFrameBufId;

    //component operation
    DISP_INTF_OPERATION_S stIntfOpt;

    DISP_Attach_PAIR_S attach_pairs[DISPLAY_ATTACH_CNT_MAX];
    struct task_struct  *kThreadReleaseFrame;

    /*for cast proc infor stastics.*/
    mt_u32 u32CastAcquireTryCnt;
    mt_u32 u32CastAcquireOkCnt;
    mt_u32 u32CastReleaseTryCnt;
    mt_u32 u32CastReleaseOkCnt;

    mt_u32 u32CastIntrCnt;

    MT_DISP_CAST_OUTFRM_S stCastOutFrm[DISP_CAST_OUTFRM_CNT_MAX];
}DISP_CAST_S;


typedef struct tagDISP_SNAPSHOT_S
{
    MT_BOOL bWork;
    BUF_POOL_S stBP;
}DISP_SNAPSHOT_S;


typedef struct tagCAST_RELEASE_PTR_S
{
    atomic_t  atReleaseNodeStatus;
    mt_u32   u32BufID;
    MT_BOOL  bInternalRelease;
}CAST_RELEASE_PTR_S;

mt_s32 DISP_CastCreate(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstInfo, MT_DRV_DISP_CAST_CFG_S *pstCfg, mt_handle *cast_ptr);
mt_s32 DISP_CastDestroy(mt_handle hCast);
mt_s32 DISP_CastSetEnable(mt_handle hCast, MT_BOOL bEnable);
mt_s32 DISP_CastGetEnable(mt_handle hCast, MT_BOOL *pbEnable);
mt_s32 DISP_CastAcquireFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 DISP_CastReleaseFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 DISP_Cast_AttachSink(mt_handle cast_ptr, mt_handle hSink);
mt_s32 DISP_Cast_DeAttachSink(mt_handle cast_ptr, mt_handle hSink);

mt_s32 DISP_Cast_SetAttr(mt_handle cast_ptr, MT_DRV_DISP_Cast_Attr_S *castAttr);
mt_s32 DISP_Cast_GetAttr(mt_handle cast_ptr, MT_DRV_DISP_Cast_Attr_S *castAttr);

mt_s32 DISP_Acquire_Snapshot(MT_DRV_DISPLAY_E enDisp, mt_handle *snapshotHandle, MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 DISP_Release_Snapshot(MT_DRV_DISPLAY_E enDisp, mt_handle snapshotHandle, MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 DISP_SnapshotDestroy(mt_handle snapshot_ptr);


mt_void DISP_CastCBSetDispMode(mt_handle hCast, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);
mt_void DISP_CastCBWork(mt_handle hCast, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);

mt_void DISP_CastCB_GenarateFrame(mt_handle cast_ptr, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);
mt_void DISP_CastPushFrame(mt_handle cast_ptr, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);
mt_void DISP_CastGetDlyStatus(mt_handle cast_ptr, MT_BOOL *pbLowDly);
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_X_H__  */










