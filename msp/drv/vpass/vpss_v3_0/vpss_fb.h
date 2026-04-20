/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_BUFFER_H__
#define __VPSS_BUFFER_H__


#include <linux/list.h>
#include "mt_drv_mmz.h"
#include"drv_vpss_ext.h"
#include "vpss_osal.h"
#include "vpss_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

typedef struct hiVPSS_BUFFER_S{
    mmz_buffer_s stMMZBuf;
    mt_u32 u32Stride;
}VPSS_BUFFER_S;

typedef struct hiVPSS_FRAME_NODE_S{
    MT_DRV_VIDEO_FRAME_S stOutFrame;
    VPSS_BUFFER_S stBuffer;
    LIST node;
}VPSS_FB_NODE_S;

typedef struct hiVPSS_FB_INFO_S{
    MT_DRV_VPSS_BUFLIST_CFG_S  stBufListCfg;

    mt_u32 u32ExtNumb;
    mt_u32 u32ExtCnt;

    mt_u32 u32GetTotal;
    mt_u32 u32GetSuccess;

    mt_u32 u32RelTotal;
    mt_u32 u32RelSuccess;

    unsigned long ulStart;
    mt_u32 u32GetHZ;
    mt_u32 u32GetLast;

    mt_u32 u32ListFul;
    VPSS_OSAL_SPIN stFulBufSpin;
    VPSS_OSAL_SPIN stEmptyBufSpin;
    LIST stEmptyFrmList;
    LIST stFulFrmList;


    VPSS_OSAL_SPIN stExtBufSpin;
    LIST stExtFrmList;

    LIST* pstTarget_1;
}VPSS_FB_INFO_S;

typedef struct hiVPSS_FB_STATE_S{
    mt_u32 u32TotalNumb;
    mt_u32 u32EmptyListNumb;
    mt_u32 u32FulListNumb;
    mt_u32 u32ExtListNumb;

    mt_u32 u32GetTotal;
    mt_u32 u32GetSuccess;
    mt_u32 u32RelTotal;
    mt_u32 u32RelSuccess;
    mt_u32 u32GetHZ;

    mt_u32 u32ListFul;
    mt_u32 u32Target_1;
    mt_u32 u32OutRate;
    #if FB_DBG
    mt_u32 u32List[DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER][2];
    mt_u32 u32FulList[DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER];
    mt_u32 u32EmptyList[DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER];
    #endif
}VPSS_FB_STATE_S;
typedef enum {
    VPSS_FB_TYPE_NORMAL = 0,
    VPSS_FB_TYPE_EXTERN,
    VPSS_FB_TYPE_BUTT
}VPSS_FB_TYPE_E;

mt_s32 VPSS_FB_Init(VPSS_FB_INFO_S *pstFrameList,MT_DRV_VPSS_BUFLIST_CFG_S *pstBufListCfg);
mt_s32 VPSS_FB_DelInit(VPSS_FB_INFO_S *pstFrameList);

/*Consumer:Port*/
mt_s32 VPSS_FB_GetFulFrmBuf(VPSS_FB_INFO_S *pstFrameList,MT_DRV_VIDEO_FRAME_S *pstFrame,mt_char* pchFile);
mt_s32 VPSS_FB_RelFulFrmBuf(VPSS_FB_INFO_S *pstFrameList,MT_DRV_VIDEO_FRAME_S *pstFrame);

/*Producers:TASK*/
VPSS_FB_NODE_S * VPSS_FB_GetEmptyFrmBuf(VPSS_FB_INFO_S *pstFrameList,
                            mt_u32 u32Height,mt_u32 u32Width,
                            MT_DRV_PIX_FORMAT_E ePixFormat,MT_DRV_PIXEL_BITWIDTH_E  enOutBitWidth);
mt_s32 VPSS_FB_AddFulFrmBuf(VPSS_FB_INFO_S *pstFrameList,VPSS_FB_NODE_S *pstFBNode);
mt_s32 VPSS_FB_AddEmptyFrmBuf(VPSS_FB_INFO_S *pstFrameList,
                                VPSS_FB_NODE_S *pstFBNode,
                                VPSS_FB_TYPE_E enType);

MT_BOOL VPSS_FB_CheckIsAvailable(VPSS_FB_INFO_S *pstFrameList);


mt_s32 VPSS_FB_GetState(VPSS_FB_INFO_S *pstFrameList,VPSS_FB_STATE_S *pstFbState);


mt_s32 VPSS_FB_Reset(VPSS_FB_INFO_S *pstFrameList);


mt_s32 VPSS_FB_AllocExtBuffer(VPSS_FB_INFO_S *pstFrameList,mt_u32 u32ExtNumb);
mt_s32 VPSS_FB_RlsExtBuffer(VPSS_FB_INFO_S *pstFrameList);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif  /* __VO_EXT_H__ */
