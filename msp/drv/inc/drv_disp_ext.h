/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : drv_disp_ext.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/

#ifndef __DRV_DISP_EXT_H__
#define __DRV_DISP_EXT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_drv_video.h"
#include "mt_drv_disp.h"
#include "mt_drv_dev.h"


typedef mt_s32  (* FN_DISP_Init)(mt_void);
typedef mt_s32  (* FN_DISP_DeInit)(mt_void);
typedef mt_s32  (* FN_DISP_Attach)(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave);
typedef mt_s32  (* FN_DISP_Detach)(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave);
typedef mt_s32  (* FN_DISP_SetFormat)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E enEnFormat);
typedef mt_s32  (* FN_DISP_GetFormat)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E *penEnFormat);

typedef mt_s32  (* FN_DISP_SetCustomTiming)(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_TIMING_S *pstTiming);
typedef mt_s32  (* FN_DISP_GetCustomTiming)(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_TIMING_S *pstTiming);
typedef mt_s32  (* FN_DISP_AddIntf)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);
typedef mt_s32  (* FN_DISP_DelIntf)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);
typedef mt_s32  (* FN_DISP_Open)(MT_DRV_DISPLAY_E enDisp);
typedef mt_s32  (* FN_DISP_Close)(MT_DRV_DISPLAY_E enDisp);
typedef mt_s32  (* FN_DISP_SetEnable)(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
typedef mt_s32  (* FN_DISP_GetEnable)(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbEnable);
typedef mt_s32  (* FN_DISP_SetRightEyeFirst)(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
typedef mt_s32  (* FN_DISP_SetBgColor)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor);
typedef mt_s32  (* FN_DISP_GetBgColor)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor);
typedef mt_s32  (* FN_DISP_SetColor)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS);
typedef mt_s32  (* FN_DISP_GetColor)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS);

typedef mt_s32  (* FN_DISP_SetAspectRatio)(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Ratio_h, mt_u32 u32Ratio_v);
typedef mt_s32  (* FN_DISP_GetAspectRatio)(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Ratio_h, mt_u32 *pu32Ratio_v);
typedef mt_s32  (* FN_DISP_SetLayerZorder)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ZORDER_ABS_E enZFlag);
typedef mt_s32  (* FN_DISP_GetLayerZorder)(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Zorder);

typedef mt_s32  (* FN_DISP_CreateCast) (MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CAST_CFG_S * pstCfg, mt_handle *phCast);
typedef mt_s32  (* FN_DISP_DestroyCast)(mt_handle hCast);
typedef mt_s32  (* FN_DISP_SetCastEnable)(mt_handle hCast, MT_BOOL bEnable);
typedef mt_s32  (* FN_DISP_GetCastEnable)(mt_handle hCast, MT_BOOL *pbEnable);
typedef mt_s32  (* FN_DISP_AcquireCastFrame)(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
typedef mt_s32  (* FN_DISP_ReleaseCastFrame)(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);


typedef mt_s32  (* FN_DISP_GetInitFlag)(MT_BOOL *pbInited);
typedef mt_s32  (* FN_DISP_GetVersion)(MT_DRV_DISP_VERSION_S *pstVersion);
typedef MT_BOOL (* FN_DISP_IsOpened)(MT_DRV_DISPLAY_E enDisp);
typedef mt_s32  (* FN_DISP_GetSlave)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penSlave);
typedef mt_s32  (* FN_DISP_GetMaster)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penMaster);
typedef mt_s32  (* FN_DISP_GetDisplayInfo)(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstInfo);

typedef mt_s32 (* FN_DISP_Ioctl)(mt_u32 cmd, mt_void *arg);
typedef mt_s32 (* FN_DISP_RegCallback)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                                   MT_DRV_DISP_CALLBACK_S *pstCallback);

typedef mt_s32 (* FN_DISP_UnRegCallback)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                                     MT_DRV_DISP_CALLBACK_S *pstCallback);
typedef mt_s32 (* FN_DISP_ExtAttach)( mt_handle hCast, mt_handle hSink);
typedef mt_s32 (* FN_DISP_ExtDeAttach)( mt_handle hCast, mt_handle hSink);

typedef mt_s32 (* FN_DISP_SetCastAttr)( mt_handle hCast, mt_u32 u32Width,mt_u32 u32Height);
typedef mt_s32 (* FN_DISP_GetCastAttr)( mt_handle hCast, MT_DRV_DISP_Cast_Attr_S *pstCastAttr);
typedef mt_s32 (*FN_DISP_UpdatePqData)(mt_u32 u32UpdateType,/*PQ_PARAM_S*/ mt_void * pstPqParam);
typedef mt_s32 (*FN_DISP_Suspend)(basedev_s *pdev, pm_message_t state);
typedef mt_s32 (*FN_DISP_Resume)(basedev_s *pdev);
typedef struct
{
    FN_DISP_Init                 pfnDispInit;
    FN_DISP_DeInit               pfnDispDeInit;
    FN_DISP_Attach               pfnDispAttach;
    FN_DISP_Detach               pfnDispDetach;
    FN_DISP_SetFormat            pfnDispSetFormat;
    FN_DISP_GetFormat            pfnDispGetFormat;
    FN_DISP_SetCustomTiming      pfnDispSetCustomTiming;
    FN_DISP_GetCustomTiming      pfnDispGetCustomTiming;
    FN_DISP_AddIntf              pfnDispAddIntf;
    FN_DISP_DelIntf              pfnDispDeIntf;
    FN_DISP_Open                 pfnDispOpen;
    FN_DISP_Close                pfnDispClose;
    FN_DISP_SetEnable            pfnDispSetEnable;
    FN_DISP_GetEnable            pfnDispGetEnable;
    FN_DISP_SetRightEyeFirst     pfnDispSetRightEyeFirst;
    FN_DISP_SetBgColor           pfnDispSetBgColor;
    FN_DISP_GetBgColor           pfnDispGetBgColor;
    FN_DISP_SetColor             pfnDispSetColor;
    FN_DISP_GetColor             pfnDispGetColor;
    FN_DISP_SetAspectRatio       pfnDispSetAspectRatio;
    FN_DISP_GetAspectRatio       pfnDispGetAspectRatio;
    FN_DISP_SetLayerZorder       pfnDispSetLayerZorder;
    FN_DISP_GetLayerZorder       pfnDispGetLayerZorder;
    FN_DISP_CreateCast           pfnDispCreatCast;
    FN_DISP_DestroyCast          pfnDispDestoryCast;
    FN_DISP_SetCastEnable        pfnDispSetCastEnable;
    FN_DISP_GetCastEnable        pfnDispGetCastEnable;
    FN_DISP_AcquireCastFrame     pfnDispAcquireCastFrm;
    FN_DISP_ReleaseCastFrame     pfnDispRlsCastFrm;
    FN_DISP_SetCastAttr          pfnDispSetCastAttr;
    FN_DISP_GetCastAttr          pfnDispGetCastAttr;
    FN_DISP_ExtAttach            pfnDispExtAttach;
    FN_DISP_ExtAttach            pfnDispExtDeAttach;
    FN_DISP_GetInitFlag          pfnDispGetInitFlag;
    FN_DISP_GetVersion           pfnDispGetVersion;
    FN_DISP_IsOpened             pfnDispIsOpen;
    FN_DISP_GetSlave             pfnDispGetSlave;
    FN_DISP_GetMaster            pfnDispGetMaster;
    FN_DISP_GetDisplayInfo       pfnDispGetDispInfo;
    FN_DISP_Ioctl                pfnDispIoctl;
    FN_DISP_RegCallback          pfnDispRegCallback;
    FN_DISP_UnRegCallback        pfnDispUnRegCallback;
    FN_DISP_UpdatePqData         pfnDispUpdatePqData;
    FN_DISP_Suspend pfnDispSuspend;
    FN_DISP_Resume pfnDispResume;

	mt_s32 (*disp_set_hd_video_onoff)(MT_BOOL bEnable);
	mt_s32 (*disp_get_dce_percent)(mt_s32 ChanID, mt_u32 *pDce);

	//MT_RET (*DF_AvsyncGetVptsInfo)(DISP_POOL_S * pstDispBP, DF_AVSYNC_PTS_S *pInfo);
	mt_s32 (*DRV_DISP_AvsyncGetVptsInfo)(void *pstDispBP, void *pInfo);

	//MT_RET (*DF_AvsyncGetInputRate)(DISP_POOL_S * pstDispBP, MT_U32 *pInputRate);
	mt_s32 (*DRV_DISP_AvsyncGetInputRate)(void *pstDispBP, MT_U32 *pInputRate);

	//MT_RET (*DF_AvsyncSetSkipFrame)(MT_U32 skip_num);
	mt_s32 (*DRV_DISP_AvsyncSetSkipFrame)(MT_U32 skip_num);

	//MT_RET (*DF_AvsyncSetRepeatFrame)(MT_VOID);
	mt_s32 (*DRV_DISP_AvsyncSetRepeatFrame)(void);

	mt_u8 (*reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode)(void);
	mt_u8 (*reg_aria_hd_encoder_get_basic_cfg_interlace_mode)(void);
    mt_s32 (*pfnDispStillScalerUpdate)(MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height, MT_BOOL bProgressive);
    mt_s32 (*pfnDispTvsysForceUpdate)(void);
}DISP_EXPORT_FUNC_S;

mt_s32  DRV_DISP_Register(mt_void);
mt_void DRV_DISP_UnRegister(mt_void);

mt_s32  VDP_DRV_ModInit(mt_void);
mt_void VDP_DRV_ModExit(mt_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DRV_DISP_EXT_H__ */

