/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_IMG_H__
#define __VPSS_IMG_H__

#include "mt_drv_video.h"
#include "vpss_common.h"
#include "vpss_osal.h"

#define VPSS_SOURCE_MAX_NUMB 6

typedef mt_s32 (*PFN_IMG_FUNC)(mt_handle hSrc,MT_DRV_VIDEO_FRAME_S *pstImage);

typedef struct hiVPSS_IMAGE_NODE_S{
    MT_DRV_VIDEO_FRAME_S stSrcImage;
    LIST node;
}VPSS_IMAGE_NODE_S;

typedef struct hiVPSS_IMAGELIST_STATE_S{
    mt_u32 u32TotalNumb;
    mt_u32 u32FulListNumb;
    mt_u32 u32EmptyListNumb;
    mt_u32 u32GetUsrTotal;
    mt_u32 u32GetUsrSuccess;

    mt_u32 u32RelUsrTotal;
    mt_u32 u32RelUsrSuccess;

    mt_u32 u32Target;
    mt_u32 u32FulList[VPSS_SOURCE_MAX_NUMB];
    mt_u32 u32EmptyList[VPSS_SOURCE_MAX_NUMB];
    mt_u32 u32List[VPSS_SOURCE_MAX_NUMB][2];
}VPSS_IMAGELIST_STATE_S;

typedef struct hiVPSS_IMAGELIST_INFO_S{
    VPSS_OSAL_LOCK stEmptyListLock;
    VPSS_OSAL_LOCK stFulListLock;
    mt_u32 u32GetUsrTotal;
    mt_u32 u32GetUsrSuccess;

    mt_u32 u32RelUsrTotal;
    mt_u32 u32RelUsrSuccess;

    LIST stEmptyImageList;
    LIST stFulImageList;
    LIST* pstTarget_1;

    mt_handle hSrc;
    PFN_IMG_FUNC pfnRlsImage;
    PFN_IMG_FUNC pfnAcqImage;
}VPSS_IMAGELIST_INFO_S;

typedef struct hiVPSS_IMG_CALLBACK_S{
    mt_handle hSrc;
    PFN_IMG_FUNC pfnRlsImage;
    PFN_IMG_FUNC pfnAcqImage;
}VPSS_IMG_CALLBACK_S;

mt_s32 VPSS_IMG_Init(VPSS_IMAGELIST_INFO_S *pstImgInfo);

mt_s32 VPSS_IMG_DeInit(VPSS_IMAGELIST_INFO_S *pstImgInfo);

MT_BOOL VPSS_IMG_CheckImageList(VPSS_IMAGELIST_INFO_S *pstImgInfo);
MT_BOOL VPSS_IMG_CheckEmptyNode(VPSS_IMAGELIST_INFO_S *pstImgInfo);

mt_s32 VPSS_IMG_GetProcessImg(VPSS_IMAGELIST_INFO_S *pstImgInfo,
                                   MT_DRV_VIDEO_FRAME_S **ppstImg);

mt_s32 VPSS_IMG_AddNewImg(VPSS_IMAGELIST_INFO_S *pstImgInfo,
                            MT_DRV_VIDEO_FRAME_S *pstNewImg);

mt_s32 VPSS_IMG_GetFieldAddr(VPSS_IMAGELIST_INFO_S *pstImgInfo,
                            MT_DRV_VID_FRAME_ADDR_S *pstFieldAddr,
                            MT_DRV_BUF_ADDR_E eLReye);

mt_s32 VPSS_IMG_Complete(VPSS_IMAGELIST_INFO_S *pstImgInfo);


mt_s32 VPSS_IMG_Regist(VPSS_IMAGELIST_INFO_S *pstImgInfo,
                    VPSS_IMG_CALLBACK_S stCallback);

mt_s32 VPSS_IMG_CorrectListOrder(VPSS_IMAGELIST_INFO_S *pstImgInfo,
                    MT_BOOL bTopFirst);

mt_s32 VPSS_IMG_Reset(VPSS_IMAGELIST_INFO_S *pstImgInfo);
#endif
