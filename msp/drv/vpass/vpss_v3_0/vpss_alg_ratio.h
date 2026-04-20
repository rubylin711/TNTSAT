/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_ALG_RATIO_H__
#define __VPSS_ALG_RATIO_H__
#include"vpss_common.h"
#include"mt_drv_video.h"

#define ALG_RATIO_PIX_RATIO1 1024
typedef struct tagALG_RATIO_USR_ASPECTRATIO_S
{
	MT_BOOL bUserDefAspectRatio;
	mt_u32 u32UserAspectWidth;
	mt_u32 u32UserAspectHeight;
}ALG_RATIO_USR_ASPECTRATIO_S;


typedef struct{
    mt_u32 AspectWidth;
    mt_u32 AspectHeight;
    mt_u32 DeviceWidth;
    mt_u32 DeviceHeight;
    MT_DRV_ASP_RAT_MODE_E eAspMode;
    ALG_RATIO_USR_ASPECTRATIO_S stUsrAsp;
    mt_rect_s stInWnd;
    mt_rect_s stOutWnd;
    mt_rect_s stScreen;
}ALG_RATIO_DRV_PARA_S;


typedef struct{
    mt_rect_s stCropWnd;
    mt_rect_s stOutWnd;
    mt_rect_s stOutScreen;
    mt_u32 u32ZmeH;
    mt_u32 u32ZmeW;
    MT_BOOL bEnAsp;
    MT_BOOL bEnCrop;
    mt_u32 u32BgColor;
    mt_u32 u32BgAlpha;
}ALG_RATIO_OUT_PARA_S;


mt_s32 ALG_RATIO_RatioProcess(ALG_RATIO_DRV_PARA_S *pstDrvPara,ALG_RATIO_OUT_PARA_S *pstOutPara);

mt_void ALG_RATIO_LetterBox(mt_u32 AspectRatioW, mt_u32 AspectRatioH,mt_rect_s *pOutWnd,mt_s32 pixr1_out);
mt_void ALG_RATIO_CropedAspect(mt_rect_s *pInWnd,mt_rect_s *pCropedWnd,mt_u32 *AspectRatioW, mt_u32 *AspectRatioH);
mt_void ALG_RATIO_CorrectAspectRatioW_H(mt_u32 *pw, mt_u32 *ph);

mt_s32 VPSS_ALG_GetAspCfg(ALG_RATIO_DRV_PARA_S *pstAspDrvPara,
                        MT_DRV_ASP_RAT_MODE_E eAspMode,mt_rect_s *pstScreen,
                        ALG_RATIO_OUT_PARA_S *pstAspCfg);

#endif /*__ALG_CSC_H__*/
