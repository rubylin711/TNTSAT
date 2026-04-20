#ifndef __DRV_VIRTUAL_H__
#define __DRV_VIRTUAL_H__

#include "mt_type.h"
#include "mt_common.h"
#include "mt_drv_video.h"
#include "mt_drv_win.h"
#include "drv_disp_osal.h"
#include "drv_window.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DEF_VIR_BUFFER_LENGTH 32

#define WinCheckVirWindow(hWin, pstWin)                                \
    \
{                                                                      \
    pstWin = WinGetVirWindow(hWin);                                    \
    if (!pstWin) {                                                     \
        WIN_ERROR("WIN is not exist!%s %d\n", __FUNCTION__, __LINE__); \
        return MT_ERR_VO_WIN_NOT_EXIST;                                \
    }                                                                  \
    \
}
typedef enum tagMUTUAL_TYPE_E {
    MUTUAL_TYPE_SRCACTIVE = 0,
    MUTUAL_TYPE_SINKACTIVE,
    MUTUAL_TYPE_BUTT
} MUTUAL_TYPE_E;

typedef struct tagVIR_BUFFER_S
{
    MUTUAL_TYPE_E enType;
    MT_DRV_VIDEO_FRAME_S stBufArray[DEF_VIR_BUFFER_LENGTH];
    mt_u32 u32Head;
    mt_u32 u32Tail;
} VIR_BUFFER_S;

typedef struct tagWIN_VIRTUAL_STAT_S
{
    mt_u32 u32SrcQTry;
    mt_u32 u32SrcQOK;
    mt_u32 u32SrcDQTry;
    mt_u32 u32SrcDQOK;

    mt_u32 u32SinkAcqTry;
    mt_u32 u32SinkAcqOK;
    mt_u32 u32SinkRlsTry;
    mt_u32 u32SinkRlsOK;
} WIN_VIRTUAL_STAT_S;

typedef struct tagVIRTUAL_S
{
    mt_u32 u32Index;

    /* state */
    MT_BOOL bEnable;
    MT_BOOL bMasked;

    MT_DRV_WIN_TYPE_E enType;

    MUTUAL_TYPE_E enBufType;
    VIR_BUFFER_S stBuffer;

    /*as display window ,store usrset attr*/
    MT_DRV_WIN_ATTR_S stAttrBuf;
    atomic_t bNewAttrFlag;

    /*sink module can change it*/
    MT_DRV_PIX_FORMAT_E ePixFormat;
    mt_u32 u32Height;
    mt_u32 u32Width;

    MT_DRV_ROT_ANGLE_E enRotation;
    MT_BOOL bHoriFlip;
    MT_BOOL bVertFlip;

    mt_handle hSink;
    mt_void *pfnQueueFrm;
    mt_void *pfnDequeueFrame;

    MT_DRV_WIN_SRC_INFO_S stSrcInfo;

    WIN_VIRTUAL_STAT_S stFrameStat;
} VIRTUAL_S;

mt_s32 VIR_BUFFER_Init(VIR_BUFFER_S *pstBuffer);
mt_s32 VIR_BUFFER_DeInit(VIR_BUFFER_S *pstBuffer);
mt_s32 VIR_BUFFER_Reset(VIR_BUFFER_S *pstBuffer);
/*consumer*/
mt_s32 VIR_BUFFER_GetFrm(VIR_BUFFER_S *pstBuffer, MT_DRV_VIDEO_FRAME_S *pstFrm);
/*productor*/
mt_s32 VIR_BUFFER_AddFrm(VIR_BUFFER_S *pstBuffer, MT_DRV_VIDEO_FRAME_S *pstFrm);

mt_s32 WIN_VIR_Create(MT_DRV_WIN_ATTR_S *pWinAttr, VIRTUAL_S **ppstVirWin);
mt_s32 WIN_VIR_Destroy(VIRTUAL_S *pstVirWin);
mt_s32 WIN_VIR_Reset(VIRTUAL_S *pstVirWin);

mt_s32 WIN_VIR_GetFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm);
mt_s32 WIN_VIR_RelFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm);

mt_s32 WIN_VIR_AddNewFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm);
mt_s32 WIN_VIR_AddUlsFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm);

mt_s32 WIN_VIR_SendAttrToSource(VIRTUAL_S *pstVirWin);
mt_s32 WIN_VIR_SetSize(VIRTUAL_S *pstVirWin, mt_u32 u32Width, mt_u32 u32Height);
mt_s32 WIN_VIR_SetAttr(VIRTUAL_S *pstVirWin, MT_DRV_WIN_ATTR_S *pWinAttr);

mt_s32 WIN_VIR_AttachSink(VIRTUAL_S *pstVirWin, mt_handle hSink);
mt_s32 WIN_VIR_DetachSink(VIRTUAL_S *pstVirWin, mt_handle hSink);

MT_BOOL WinCheckVirtual(mt_u32 u32WinIndex);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*  __DRV_WINDOW_H__  */