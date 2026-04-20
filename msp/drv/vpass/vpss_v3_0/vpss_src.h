/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_SRC_H__
#define __VPSS_SRC_H__
#include "vpss_common.h"
#include "mt_drv_vpss.h"
#include "vpss_osal.h"
#include "drv_vdec_ext.h"
#define VPSS_LIST_DATA_LENTH 128

typedef mt_s32 (*PFN_SRC_FUNC)(mt_handle hSrc,MT_DRV_VIDEO_FRAME_S *pstImage);
typedef MT_DRV_VIDEO_FRAME_S VPSS_SRC_DATA_S;
#define DEF_SRC_MODE_FRAME_NUMB 1
#define DEF_SRC_MODE_FIELD_NUMB 2
#define DEF_SRC_MODE_NTSC_NUMB  6
#define DEF_SRC_MODE_PAL_NUMB   10
#define DEF_SRC_INVALID_POINT 0xffffffff
typedef enum hiVPSS_SRC_MODE_E{
    SRC_MODE_FRAME = 1,
    SRC_MODE_FIELD = 2,
    SRC_MODE_NTSC = 6,
    SRC_MODE_PAL = 10,
    SRC_MODE_BUTT
}VPSS_SRC_MODE_E;
typedef struct hiVPSS_SRC_NODE_S{
    VPSS_SRC_DATA_S stSrcData;
    LIST node;
}VPSS_SRC_NODE_S;
typedef struct hiVPSS_SRC_S{
    MT_BOOL bInit;
    mt_u32 u32PutSrcCount;
    mt_u32 u32CompleteSrcCount;
    mt_u32 u32ReleaseSrcCount;
    VPSS_SRC_DATA_S* pstDataList;

    mt_u32 u32ListLenth;
    LIST stEmptySrcList;
    LIST stFulSrcList;
    LIST* pstTarget_1;

    VPSS_SRC_MODE_E enMode;
    mt_handle    hSrcModule;
    PFN_SRC_FUNC pfnRlsImage;
}VPSS_SRC_S;

typedef struct hiVPSS_SRC_ATTR_S{
    VPSS_SRC_MODE_E enMode;
    mt_handle    hSrcModule;
    PFN_SRC_FUNC pfnRlsImage;
}VPSS_SRC_ATTR_S;


#define SRCIN_NODE_NUM 6

typedef struct hiVPSS_SRCIN_S
{
    MT_BOOL bInit;

    VPSS_OSAL_SPIN stSrcInLock;

    VPSS_SRC_NODE_S *pstNodeUsed;

    struct list_head   stFreeList;
    struct list_head   stBusyList;
    struct list_head   stReleaseList;

    VPSS_SRC_NODE_S    astSrcNode[SRCIN_NODE_NUM];
}VPSS_SRCIN_S;


mt_s32 VPSS_SRC_Init(VPSS_SRC_S *pstSrc,VPSS_SRC_ATTR_S stAttr);
mt_s32 VPSS_SRC_DeInit(VPSS_SRC_S* pstSrc);
mt_s32 VPSS_SRC_Reset(VPSS_SRC_S* pstSrc);

mt_s32 VPSS_SRC_PutImage(VPSS_SRC_S *pstSrc,VPSS_SRC_DATA_S *pstData);
mt_s32 VPSS_SRC_GetProcessImage(VPSS_SRC_S *pstSrc,VPSS_SRC_DATA_S **pstData);
mt_s32 VPSS_SRC_GetPreImgInfo(VPSS_SRC_S *pstSrc,
                               VPSS_SRC_DATA_S **pPtrPreData,
                               VPSS_SRC_DATA_S **pPtrPpreData
                               );
mt_s32 VPSS_SRC_CompleteImage(VPSS_SRC_S* pstSrc,mt_void *arg);

mt_s32 VPSS_SRCIN_Init(VPSS_SRCIN_S *pSrcIn);
mt_s32 VPSS_SRCIN_DeInit(VPSS_SRCIN_S *pSrcIn);
mt_s32 VPSS_SRCIN_SendImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData);
mt_s32 VPSS_SRCIN_CallImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData);
mt_s32 VPSS_SRCIN_GetImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData);
mt_s32 VPSS_SRCIN_RelImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData);


#endif
