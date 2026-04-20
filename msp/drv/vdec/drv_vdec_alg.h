/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
File Name     : drv_vdec_alg.h
Version       : Initial Draft
Author        : Montage MA-SW
Created       : 2016/01/06
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __OPTM_ALG_H__
#define __OPTM_ALG_H__

#include "mt_type.h"

#define OPTM_FRD_IN_PTS_SEQUENCE_LENGTH         100

#define OPTM_FRD_INFRAME_RATE_JUMP_THRESHOLD  10
#define OPTM_FRD_INFRAME_RATE_WAVE_THRESHOLD  2

#define OPTM_ALG_ABS(x)     (((x) < 0) ? -(x) : (x))
#define OPTM_ALG_ROUND(x)   (((x % 10) > 4) ? (x / 10 + 1) * 10 : x)

/*****************************************************************
 *              New frame rate detect algorithm                 *
 *****************************************************************/
typedef struct tagOPTM_ALG_FRD_S
{
    mt_u32                       InPTSSqn[OPTM_FRD_IN_PTS_SEQUENCE_LENGTH]; /* past PTS information  */
    mt_u32                       nowPTSPtr; /* pointer of past PTS information, pointing to the oldest frame rate in record */
    mt_u32                       length;
    mt_u32                       u320_Pts;
    mt_u32                       u32120_Pts;
    mt_u32                       u32QueCnt;
    mt_u32                       u32QueRate;
    mt_u32                       u32QueStable;

    mt_u32                       unableTime;

    mt_u32                       InFrameRateLast;     /*  last input frame rate */
    mt_u32                       StableThreshold;
    mt_u32                       InFrameRateEqueTime;/*  counter of stable frame rate, to avoid display shake caused by shake of frame rate */

    mt_u32                       InFrameRate;
}OPTM_ALG_FRD_S;

mt_void OPTM_ALG_FrdInfo_Reset(OPTM_ALG_FRD_S *pPtsInfo, mt_u32 ptsNum);
mt_u32 OPTM_ALG_FrameRateDetect(OPTM_ALG_FRD_S *pPtsInfo,mt_u32 Pts);
mt_u32 OPTM_ALG_InPTSSqn_CalNowRate(OPTM_ALG_FRD_S *pPtsInfo, mt_u32 env);
mt_void OPTM_ALG_InPTSSqn_ChangeInFrameRate(OPTM_ALG_FRD_S *pPtsInfo, mt_u32 nowRate);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __OPTM_ALG_H__ */

