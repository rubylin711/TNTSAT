
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_win_hal.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_WIN_HAL_H__
#define __DRV_WIN_HAL_H__


#include "mt_type.h"
#include "drv_disp_com.h"
//#include "vdp_define.h"
//#include "vdp_driver.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


#define MT_WIN_IN_RECT_X_ALIGN      0xFFFFFFFEul
#define MT_WIN_IN_RECT_WIDTH_ALIGN  0xFFFFFFF8ul
#define MT_WIN_IN_RECT_Y_ALIGN      0xFFFFFFFEul
#define MT_WIN_IN_RECT_HEIGHT_ALIGN 0xFFFFFFFEul

#define MT_WIN_OUT_RECT_X_ALIGN      0xFFFFFFFEul
#define MT_WIN_OUT_RECT_WIDTH_ALIGN  0xFFFFFFF8ul
#define MT_WIN_OUT_RECT_Y_ALIGN      0xFFFFFFFEul
#define MT_WIN_OUT_RECT_HEIGHT_ALIGN 0xFFFFFFFEul


#define VIDEO_ZOOM_IN_VERTICAL_MAX   4
#define VIDEO_ZOOM_IN_HORIZONTAL_MAX 4

#define DEF_VIDEO_LAYER_MAX_NUMBER 6
#define DEF_VIDEO_LAYER_INVALID_ID 0xff

#define VIDEO_LAYER_SUPPORT_MIN_WIDTH   720
#define VIDEO_LAYER_SUPPORT_MIN_HEIGHT  576

typedef enum tagVIDEO_LAYER_ACC_MODE_E
{
    VIDEO_LAYER_ACC_DISABLE = 0,
    VIDEO_LAYER_ACC_AUTO,
    VIDEO_LAYER_ACC_MODE_BUTT
}VIDEO_LAYER_ACC_MODE_E;


typedef enum tagVIDEO_LAYER_ACM_MODE_E
{
    VIDEO_LAYER_ACM_DISABLE = 0,
    VIDEO_LAYER_ACM_AUTO,
    VIDEO_LAYER_ACM_MODE_BUTT
}VIDEO_LAYER_ACM_MODE_E;


typedef enum tagVIDEO_ALG_LOCATION
{
    ALG_LOCATION_IN_V0 = 0,
    ALG_LOCATION_IN_VP0,
    ALG_LOCATION_IN_BUTT
}VIDEO_ALG_LOCATION_E;

typedef struct tagVIDEO_LAYER_PROC_S
{
    MT_BOOL bDci;
    MT_BOOL bSR;
    VIDEO_ALG_LOCATION_E eSrLocation;
    VIDEO_ALG_LOCATION_E eDciLocation;
    MT_BOOL bSrBehindDci;
}VIDEO_LAYER_PROC_S;


typedef struct tagVIDEO_LAYER_CAPABILITY_S
{
    MT_BOOL bSupport;
    mt_u32  eId;
    MT_BOOL bZme;
    MT_BOOL bACC;
    MT_BOOL bACM;
    MT_BOOL bLTICTI;
    MT_BOOL bDcmp;

    /*although  DCI and SR may be  in SR, but it's mainly for window on V0
     *so we make it a spec of video layer.
     */
    MT_BOOL bDci;
    MT_BOOL bSR;

    MT_BOOL bHDIn;
    MT_BOOL bHDOut;

    /*the bitwidth of the layer. used for bg color setting.*/
    mt_u32 u32BitWidth;
    mt_u32 u32LayerWidthMax;
    mt_u32 u32LayerHeightMax;

    VIDEO_LAYER_PROC_S stLayerProcInfo;
}VIDEO_LAYER_CAPABILITY_S;

typedef struct tagVIDEO_LAYER_S
{
    MT_BOOL bWorking;
    MT_BOOL bInitial;
}VIDEO_LAYER_S;

typedef struct tagVIDEO_LAYER_FRAME_PARA_S
{
    MT_DRV_FRAME_TYPE_E  eFrmType;
    MT_DRV_PIX_FORMAT_E  eVideoFormat;

    MT_BOOL bInterlaced;
    MT_BOOL bTopFirst;

    mt_rect_s stIn;
    mt_rect_s stDisp;
    mt_rect_s stVideo;

    MT_DRV_COLOR_SPACE_E eSrcCS;
    MT_DRV_COLOR_SPACE_E eDstCs;

    mt_u32 u32AddrNumber;
    MT_DRV_VID_FRAME_ADDR_S stAddr[2];

}VIDEO_LAYER_FRAME_PARA_S;


typedef struct tagWIN_HAL_PARA_S
{
    MT_DRV_DISP_STEREO_E en3Dmode;
    MT_BOOL bRightEyeFirst;

    MT_DRV_VIDEO_FRAME_S *pstFrame;

    MT_BOOL bZmeUpdate;
    MT_BOOL bZmeSupport;

    /* since some platform use complete width ,
     * some use cropped width,so we add a original one.
     */
    mt_rect_s stInOrigin;
    mt_rect_s stIn;
    mt_rect_s stVideo;
    mt_rect_s stDisp;
    mt_rect_s stOriRect;/*Vpss InRect,for ZME*/


    /*for chiptype which has a second zme module such as SR.*/
    mt_rect_s stFinalZmeRect;
    /*to judge the second zme module action.*/
    MT_BOOL bSecondHorZmeEnable;
    MT_BOOL bSecondVerZmeEnable;

    mt_u32 u32Fidelity;
    MT_DISP_DISPLAY_INFO_S *pstDispInfo;
    MT_DRV_DISP_FIELD_FLAG_E eField;
    MT_BOOL bRegionMute;
    mt_u32  u32RegionNum;
}WIN_HAL_PARA_S;


typedef struct tagVIDEO_LAYER_FUNCTIONG_S
{
    mt_s32 (*PF_GetCapability)(mt_u32 u32Layer, VIDEO_LAYER_CAPABILITY_S *pstSurf);

    mt_s32 (*PF_AcquireLayerByDisplay)(MT_DRV_DISPLAY_E eDisp, mt_u32 *pu32Layer);
    mt_s32 (*PF_ReleaseLayer)(mt_u32 u32Layer);

    mt_s32 (*PF_SetEnable)(mt_u32 u32Layer, mt_u32 u32RegionNum, MT_BOOL bEnable);

    mt_s32 (*PF_VP0ParaUpd)(mt_u32 u32Layer);
    mt_s32 (*PF_Update)(mt_u32 u32Layer);

    mt_s32 (*PF_SetDefault)(mt_u32 u32Layer);
    mt_s32 (*PF_ChckLayerInit)(mt_u32 u32Layer);

    mt_s32 (*PF_SetAllLayerDefault)(mt_void);

    mt_s32 (*PF_SetDispMode)(mt_u32 u32Layer, MT_DRV_DISP_STEREO_MODE_E eMode);
    mt_s32 (*PF_SetColor)(mt_u32 u32Layer, MT_DRV_DISP_COLOR_SETTING_S *pstColor);

    mt_s32 (*PF_MovUp)(mt_u32 u32Layer);
    mt_s32 (*PF_MovTop)(mt_u32 u32Layer);
    mt_s32 (*PF_MovDown)(mt_u32 u32Layer);
    mt_s32 (*PF_MovBottom)(mt_u32 u32Layer);

    mt_s32 (*PF_GetZorder)(mt_u32 u32Layer, mt_u32 *pZOrder);
    mt_s32 (*PF_SetDebug)(mt_u32 u32Layer, MT_BOOL bEnable);

    mt_s32 (*PF_SetFramePara)(mt_u32 u32Layer, WIN_HAL_PARA_S *pstPara);
    mt_s32 (*PF_Get3DOutRect)(MT_DRV_DISP_STEREO_E en3DMode, mt_rect_s *pstOutRect, mt_rect_s *pstReviseOutRect);
    mt_s32 (*PF_GetCSCReg)(mt_u32 u32Data, mt_u32 *pdata);
    mt_s32 (*PF_SetCSCReg)(mt_u32 u32Data, mt_u32 *pdata);
    mt_s32 (*PF_SetZMEPhase)(mt_u32 u32Data, mt_s32 s32PhaseL, mt_s32 s32PhaseC);
}VIDEO_LAYER_FUNCTIONG_S;

mt_s32 VideoLayer_Init(mt_void);
mt_s32 VideoLayer_DeInit(mt_void);

mt_s32 VideoLayer_GetFunction(VIDEO_LAYER_FUNCTIONG_S *pstFunc);

VIDEO_LAYER_FUNCTIONG_S *VideoLayer_GetFunctionPtr(mt_void);



#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_VO_HAL_H__  */











