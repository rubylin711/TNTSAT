/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_RWZB_H__
#define __VPSS_RWZB_H__

#include "vpss_alg_rwzb.h"
#include "mt_drv_video.h"
typedef struct hiVPSS_RWZB_S{
    mt_u32 u32Rwzb;
    MT_BOOL bInit;
    mt_u8 u8RwzbData[MAXSAMPNUM][PIX_NUM_IN_PATTERN];
    DET_INFO_S stDetInfo;
}VPSS_RWZB_S;

typedef struct hiVPSS_RWZB_IMG_S{
    mt_u32 u32Width;
    mt_u32 u32Height;
    MT_DRV_FIELD_MODE_E enFieldMode;
    MT_BOOL bProgressive;
}VPSS_RWZB_IMG_S;

mt_s32 VPSS_RWZB_Init(VPSS_RWZB_S *pstRwzb);
mt_s32 VPSS_RWZB_DeInit(VPSS_RWZB_S *pstRwzb);


mt_s32 VPSS_RWZB_GetRwzbData(VPSS_RWZB_S *pstRwzb,VPSS_RWZB_INFO_S *pstRwzbInfo);

mt_s32 VPSS_RWZB_GetRwzbInfo(VPSS_RWZB_S *pstRwzb,
                              VPSS_RWZB_INFO_S* pstRwzbInfo,
                              VPSS_RWZB_IMG_S* pstImage);

mt_s32 VPSS_RWZB_GetRwzbType(VPSS_RWZB_S *pstRwzb,mt_u32 *pu32Type);






#endif

