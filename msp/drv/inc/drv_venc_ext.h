/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_VENC_EXT_H__
#define __DRV_VENC_EXT_H__

#include "mt_drv_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* source information.
   venc will get / release frame or send private info to sourec by function pointer */
typedef mt_s32  (*FN_VENC_GET_FRAME)(mt_handle handle, MT_DRV_VIDEO_FRAME_S* pstImage);
typedef mt_s32  (*FN_VENC_PUT_FRAME)(mt_handle handle, MT_DRV_VIDEO_FRAME_S* pstImage);
typedef mt_s32  (*FN_VENC_CHANGE_INFO)(mt_handle handle, mt_u32 u32Width, mt_u32 u32Height);

typedef struct hiDRV_VENC_SRC_INFO_S
{
    mt_handle hSrc;
    FN_VENC_GET_FRAME   pfAcqFrame;
    FN_VENC_PUT_FRAME   pfRlsFrame;
    FN_VENC_CHANGE_INFO pfChangeInfo;

    mt_u32    u32Resrve0;
    mt_u32    u32Resrve1;
} MT_DRV_VENC_SRC_INFO_S;

//typedef mt_s32 (*FN_VENC_EncodeFrame)(mt_void);    //del
typedef mt_s32  (*FN_VENC_QueueFrame)(mt_handle handle, MT_DRV_VIDEO_FRAME_S* pstFrame);
typedef mt_void (*FN_VENC_WakeUpThread)(mt_void);

typedef mt_s32  (*FN_VENC_Resume)(mt_void);
typedef mt_s32 (*FN_VENC_Suspend)(mt_void);

/* just for VENC attach vi temp!*/
typedef mt_s32  (*FN_VENC_SetSrcInfo)(mt_handle hVencChn, MT_DRV_VENC_SRC_INFO_S* pstSrcInfo);

typedef struct
{
    //FN_VENC_EncodeFrame pfnVencEncodeFrame;
    FN_VENC_QueueFrame   pfnVencQueueFrame;
    FN_VENC_WakeUpThread pfnVencWakeUpThread;
    FN_VENC_Resume       pfnVencResume;
    FN_VENC_Suspend      pfnVencSuspend;
    FN_VENC_SetSrcInfo   pfnSetSrcInfo;
} VENC_EXPORT_FUNC_S;

mt_s32 VENC_DRV_ModInit(mt_void);
mt_void VENC_DRV_ModExit(mt_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif //__DRV_VENC_EXT_H__
