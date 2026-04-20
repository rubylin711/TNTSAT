/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HDMI_EXT_H__
#define __HDMI_EXT_H__
//#include "mt_unf_hdmi.h"
#include "mt_drv_hdmi.h"
#include "mt_drv_disp.h"
#include "mt_drv_dev.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

mt_s32 HDMI_DRV_ModInit(mt_void);
mt_void HDMI_DRV_ModExit(mt_void);

typedef mt_s32 (*FN_HDMI_Init)(mt_void);
typedef mt_void (*FN_HDMI_Deinit)(mt_void);
typedef mt_s32 (*FN_HDMI_Open)(MT_UNF_HDMI_ID_E enHdmi);
typedef mt_s32 (*FN_HDMI_Close)(MT_UNF_HDMI_ID_E enHdmi);

typedef mt_s32 (*FN_HDMI_GetPlayStus)(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32Stutus);
typedef mt_s32 (*FN_HDMI_GetAoAttr)(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);
typedef mt_s32 (*FN_HDMI_GetSinkCapability)(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkCap);
typedef mt_s32 (*FN_HDMI_GetAudioCapability)(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_HDMI_AUDIO_CAPABILITY_S *pstAudCap);
typedef mt_s32 (*FN_HDMI_SetAudioMute)(MT_UNF_HDMI_ID_E enHdmi);
typedef mt_s32 (*FN_HDMI_SetAudioUnMute)(MT_UNF_HDMI_ID_E enHdmi);

typedef mt_s32 (*FN_HDMI_AudioChange)(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);

typedef mt_s32 (*FN_HDMI_PreFormat)(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enEncodingFormat);
typedef mt_s32 (*FN_HDMI_SetFormat)(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo);

typedef mt_s32 (*FN_HDMI_Detach)(MT_UNF_HDMI_ID_E enHdmi);
typedef mt_s32 (*FN_HDMI_Attach)(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo);

typedef mt_s32 (*FN_HDMI_Suspend)(basedev_s *pdev, pm_message_t state);
typedef mt_s32 (*FN_HDMI_Resume)(basedev_s *pdev);

typedef mt_s32 (*FN_HDMI_SoftResume)(MT_DRV_DISP_FMT_E enHdmi, MT_DRV_DISP_STEREO_E enStereo);

typedef mt_s32 (*FN_HDMI_ConfigVideo)(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
typedef mt_s32 (*FN_HDMI_NotifyRegister)(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
typedef mt_s32 (*FN_HDMI_AvMute)(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
typedef mt_s32 (*FN_HDMI_Clock_Config)(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
typedef mt_s32 (*FN_HDMI_EMP_Config)(MT_UNF_HDMI_ID_E enHdmi, void *p_param, mt_u32 len);
typedef mt_s32 (*FN_HDMI_Vcfg_Check)(MT_UNF_HDMI_ID_E enHdmi, void *param_in, void *param_out);

typedef struct {
	FN_HDMI_Init pfnHdmiInit;
	FN_HDMI_Deinit pfnHdmiDeinit;
	FN_HDMI_Open pfnHdmiOpen;
	FN_HDMI_Close pfnHdmiClose;
	FN_HDMI_GetPlayStus pfnHdmiGetPlayStus;
	FN_HDMI_GetAoAttr pfnHdmiGetAoAttr;
	FN_HDMI_GetSinkCapability pfnHdmiGetSinkCapability;
	FN_HDMI_GetAudioCapability pfnHdmiGetAudioCapability;
	FN_HDMI_SetAudioMute pfnHdmiSetAudioMute;
	FN_HDMI_SetAudioUnMute pfnHdmiSetAudioUnMute;
	FN_HDMI_AudioChange pfnHdmiAudioChange;
	FN_HDMI_PreFormat pfnHdmiPreFormat;
	FN_HDMI_SetFormat pfnHdmiSetFormat;
	FN_HDMI_Detach pfnHdmiDetach;
	FN_HDMI_Attach pfnHdmiAttach;
	FN_HDMI_Resume pfnHdmiResume;
	FN_HDMI_Suspend pfnHdmiSuspend;
	FN_HDMI_SoftResume pfnHdmiSoftResume;
	FN_HDMI_ConfigVideo pfnHdmiConfigVid;
	FN_HDMI_NotifyRegister pfnHdmiNotifyRegister;
	FN_HDMI_AvMute pfnHdmiAvMute;
	FN_HDMI_Clock_Config pfnHdmiClkCfg;
	FN_HDMI_EMP_Config pfnHdmiEmpCfg;
	FN_HDMI_Vcfg_Check pfnHdmiVcfgCheck;
} HDMI_EXPORT_FUNC_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HDMI_EXT_H__ */
