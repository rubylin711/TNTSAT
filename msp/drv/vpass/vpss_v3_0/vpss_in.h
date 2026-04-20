/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_IN_H__
#define __VPSS_IN_H__
#include "vpss_common.h"
#include "drv_vdec_ext.h"
#include "vpss_src.h"
#include "vpss_wbc.h"
#include "vpss_sttinf.h"
#include "vpss_img.h"
#include "vpss_his.h"
#include "vpss_rwzb.h"

#define DEF_TOPFIRST_BUTT 0xffffffff
#define DEF_TOPFIRST_PROG 0xfffffffe

typedef mt_s32 (*PFN_IN_RlsCallback)(mt_handle hSrc,MT_DRV_VIDEO_FRAME_S *pstImage);

typedef enum
{
    VPSS_IN_MODE_VPSSACTIVE = 0,
    VPSS_IN_MODE_USRACTIVE,
    VPSS_IN_MODE_BUTT
}VPSS_IN_MODE_E;

typedef struct hiVPSS_IN_STREAM_INFO_S{
    mt_u32  u32IsNewImage;

    mt_u32 u32InRate;
    mt_u32 u32RealTopFirst;/*defines the stream top first info.
                                  *init:DEF_TOPFIRST_BUTT
                                  *Progressive:DEF_TOPFIRST_PROG
                                  *Interlace:DEF_TOPFIRST_BOTTOM or DEF_TOPFIRST_BOTTOM*/
    mt_u32 u32Rwzb;
    mt_u32 u32StreamInRate;
    MT_BOOL u32StreamTopFirst;
    MT_BOOL u32StreamProg;
    mt_u32 u32StreamH;
    mt_u32 u32StreamW;
    MT_DRV_FRAME_TYPE_E  eStreamFrmType;
}VPSS_IN_STREAM_INFO_S;

typedef enum
{
    VPSS_IN_INFO_SRC,
    VPSS_IN_INFO_WBC,
    VPSS_IN_INFO_REF,
    VPSS_IN_INFO_STREAM,
    VPSS_IN_INFO_WBCREG_VA,
    VPSS_IN_INFO_WBCREG_PA,
    VPSS_IN_INFO_NR,
    VPSS_IN_INFO_DIE,
    VPSS_IN_INFO_CCREF,
    VPSS_IN_INFO_WBCMODE,
    VPSS_IN_INFO_CCCL,
    VPSS_IN_INFO_FIELD,
    VPSS_IN_INFO_MT_ADDR,
    VPSS_IN_INFO_CORRECT_FIELD,
    VPSS_IN_INFO_BUTT
}VPSS_IN_INFO_TYPE_E;

typedef struct
{
    VPSS_VERSION_E enVersion;

    VPSS_IN_MODE_E enMode;
    PFN_IN_RlsCallback pfnRlsCallback;
    PFN_IN_RlsCallback pfnAcqCallback;
    mt_handle hSource;

    MT_DRV_VPSS_PRODETECT_E enProgInfo;
    MT_BOOL bProgRevise;
    MT_BOOL bAlwaysFlushSrc;
    MT_DRV_COLOR_SPACE_E enSrcCS;


    VPSS_IN_STREAM_INFO_S stStreamInfo;
    MT_DRV_VIDEO_ORIGINAL_INFO_S stOriInfo;
    mt_u32                u32ScenceChgCnt;

    /*spinlock for VPSS_SRC_S*/
    VPSS_OSAL_SPIN stSrcSpin;

    /*VPSS_VERSION_V2_0:
       VPSS_SRC_S
       VPSS_WBC_S[2]
       VPSS_STTWBC_S[2]
       VPSS_DIESTINFO_S[2]
       VPSS_CCCLCNTINFO_S[2]
       VPSS_NRMADINFO_S[2]
     */
    VPSS_SRC_S *pstSrc;
    VPSS_WBC_S *pstWbcInfo[2];
    VPSS_STTWBC_S *pstSttWbc[2];
    VPSS_DIESTINFO_S *pstDieStInfo[2];
    VPSS_CCCLCNTINFO_S *pstCcclCntInfo[2];
    VPSS_NRMADINFO_S *pstNrMadInfo[2];

    /*VPSS_VERSION_V1_0:
       VPSS_IMAGELIST_INFO_S
       VPSS_MT_INFO_S
       VPSS_RWZB_S
     */
    VPSS_IMAGELIST_INFO_S *pstSrcImagesList;
    VPSS_MT_INFO_S *pstMtInfo;

}VPSS_IN_ENTITY_S;

typedef mt_s32 (*PFN_IN_RefreshList)(VPSS_IN_ENTITY_S *pstEntity);

typedef mt_s32 (*PFN_IN_GetProcessImage)(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S **ppstFrame);

typedef mt_s32 (*PFN_IN_CompleteImage)(VPSS_IN_ENTITY_S *pstEntity);

typedef mt_s32 (*PFN_IN_GetInfo)(VPSS_IN_ENTITY_S *pstEntity,
                                    VPSS_IN_INFO_TYPE_E enType,
                                    MT_DRV_BUF_ADDR_E enLR,
                                    mt_void* pstInfo);

typedef mt_s32 (*PFN_IN_Reset)(VPSS_IN_ENTITY_S *pstEntity);

typedef mt_s32 (*PFN_IN_PutImage)(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S *pstFrame);
typedef mt_s32 (*PFN_IN_GetImage)(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S *pstFrame);


typedef struct
{
    PFN_IN_GetProcessImage pfnGetProcessImage;
    PFN_IN_CompleteImage   pfnCompleteImage;
    PFN_IN_Reset           pfnReset;
    PFN_IN_GetInfo         pfnGetInfo;
    PFN_IN_RefreshList     pfnRefresh;
}VPSS_IN_INTF_S;

typedef struct
{
    VPSS_VERSION_E enVersion;
}VPSS_IN_ENV_S;

typedef struct
{
    MT_DRV_VPSS_PRODETECT_E enProgInfo;
    MT_BOOL bProgRevise;
    MT_BOOL bAlwaysFlushSrc;
    MT_DRV_COLOR_SPACE_E enSrcCS;
}VPSS_IN_ATTR_S;

typedef struct
{
    VPSS_IN_MODE_E enMode;
    mt_handle hSource;
    PFN_IN_RlsCallback pfnRlsCallback;
    PFN_IN_RlsCallback pfnAcqCallback;
}VPSS_IN_SOURCE_S;

mt_s32 VPSS_IN_Init(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_ENV_S stEnv);

mt_s32 VPSS_IN_GetIntf(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_INTF_S *pstIntf);

mt_s32 VPSS_IN_SetAttr(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_ATTR_S stAttr);

mt_s32 VPSS_IN_GetAttr(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_ATTR_S *pstAttr);

mt_s32 VPSS_IN_SetSrcMode(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_SOURCE_S stMode);

mt_s32 VPSS_IN_DeInit(VPSS_IN_ENTITY_S *pstEntity);

#endif
