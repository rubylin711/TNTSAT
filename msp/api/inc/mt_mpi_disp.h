/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_disp.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*********************************************************************************************/


#ifndef __MPI_DISP_H__
#define __MPI_DISP_H__

#include "mt_drv_disp.h"
#include "mt_error_mpi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

#define CHECK_DISP_INIT()\
do{\
    MT_DISP_LOCK();\
    if (g_DispDevFd < 0)\
    {\
        MT_ERR_DISP("DISP is not init.\n");\
        MT_DISP_UNLOCK();\
        return MT_ERR_DISP_NO_INIT;\
    }\
    MT_DISP_UNLOCK();\
}while(0)


mt_s32 MT_MPI_DISP_Init(mt_void);
mt_s32 MT_MPI_DISP_DeInit(mt_void);

mt_s32 MT_MPI_DISP_Attach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave);
mt_s32 MT_MPI_DISP_Detach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave);
mt_s32 MT_MPI_DISP_SetSdEncPqParam(MT_DRV_DISPLAY_E enDisp, DISP_SD_ENC_PQ_PARA_S *pPara);

mt_s32 MT_MPI_DISP_SetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E enStereo, MT_DRV_DISP_FMT_E enFormat);
mt_s32 MT_MPI_DISP_GetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E *penStereo,MT_DRV_DISP_FMT_E *penFormat);

mt_s32 MT_MPI_DISP_SetRightEyeFirst(MT_DRV_DISPLAY_E enDisp, MT_BOOL bRFirst);
mt_s32 MT_MPI_DISP_SetVirtualScreen(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Width, mt_u32 u32Height);
mt_s32 MT_MPI_DISP_GetVirtualScreen(MT_DRV_DISPLAY_E enDisp, mt_u32 *u32Width, mt_u32 *u32Height);
mt_s32 MT_MPI_DISP_SetSmallWindow(MT_DRV_DISPLAY_E enDisp, mt_s32 xstart, mt_s32 ystart,mt_u32 u32Width, mt_u32 u32Height);
mt_s32 MT_MPI_DISP_SetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstOffset);
mt_s32 MT_MPI_DISP_GetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstOffset);

mt_s32 MT_MPI_DISP_SetTiming(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_TIMING_S *pstTiming);
mt_s32 MT_MPI_DISP_GetTiming(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_TIMING_S *pstTiming);

mt_s32 MT_MPI_DISP_AddIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);
mt_s32 MT_MPI_DISP_DelIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);

mt_s32 MT_MPI_DISP_AddVDAC(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf, mt_u32 u32Number);
mt_s32 MT_MPI_DISP_DelVDAC(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);

mt_s32 MT_MPI_DISP_Open(MT_DRV_DISPLAY_E enDisp);
mt_s32 MT_MPI_DISP_Close(MT_DRV_DISPLAY_E enDisp);

mt_s32 MT_MPI_DISP_SetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_GetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbEnable);

mt_s32 MT_MPI_DISP_SetBGColor(MT_DRV_DISPLAY_E eDisp, MT_DRV_DISP_COLOR_S *pstBGColor);
mt_s32 MT_MPI_DISP_GetBGColor(MT_DRV_DISPLAY_E eDisp, MT_DRV_DISP_COLOR_S *pstBGColor);

//set color
mt_s32 MT_MPI_DISP_SetColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS);
mt_s32 MT_MPI_DISP_GetColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS);

//set aspect ratio
mt_s32 MT_MPI_DISP_SetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Ratio_h, mt_u32 u32Ratio_v);
mt_s32 MT_MPI_DISP_GetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Ratio_h, mt_u32 *pu32Ratio_v);

mt_s32 MT_MPI_DISP_SetLayerZorder(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ZORDER_ABS_E enZFlag);
mt_s32 MT_MPI_DISP_GetLayerZorder(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Zorder);

//miracast
mt_s32 MT_MPI_DISP_CreateCast(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CAST_CFG_S *pstCfg, mt_handle *phCast);
mt_s32 MT_MPI_DISP_DestroyCast(mt_handle hCast);
mt_s32 MT_MPI_DISP_SetCastEnable(mt_handle hCast, MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable);

mt_s32 MT_MPI_DISP_AcquireCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 MT_MPI_DISP_ReleaseCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 MT_MPI_DISP_ExtAttach(mt_handle hCast, mt_handle hSink);
mt_s32 MT_MPI_DISP_ExtDeAttach(mt_handle hCast, mt_handle hSink);

//snapshot
mt_s32 MT_MPI_DISP_Snapshot_Acquire(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S * pstFrame);
mt_s32 MT_MPI_DISP_Snapshot_Release(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S * pstFrame);

//Macrovision
mt_s32 MT_MPI_DISP_TestMacrovisionSupport(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbSupport);
mt_s32 MT_MPI_DISP_SetMacrovisionCustomer(MT_DRV_DISPLAY_E enDisp, mt_void *pData);
mt_s32 MT_MPI_DISP_SetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E enMode);
mt_s32 MT_MPI_DISP_GetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E *penMode);

//cgms-a
mt_s32 MT_MPI_DISP_SetCGMS_A(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CGMSA_CFG_S *pstCfg);

//vbi
mt_s32 MT_MPI_DISP_CreateVBI(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_VBI_CFG_S *pstCfg, mt_handle *phVbi);
mt_s32 MT_MPI_DISP_DestroyVBI(mt_handle hVbi);
mt_s32 MT_MPI_DISP_SendVBIData(mt_handle hVbi, const MT_DRV_DISP_VBI_DATA_S *pstVbiData);
mt_s32 MT_MPI_DISP_SetWss(MT_DRV_DISPLAY_E enDisp, const MT_DRV_DISP_WSS_DATA_S *pstWssData);

//may be deleted
//mt_s32 MT_MPI_DISP_SetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg);
//mt_s32 MT_MPI_DISP_GetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg);


//setting
//mt_s32 MT_MPI_DISP_SetSetting(MT_DRV_DISPLAY_E enDisp, DISP_SETTING_S *pstSetting);
//mt_s32 MT_MPI_DISP_GetSetting(MT_DRV_DISPLAY_E enDisp, DISP_SETTING_S *pstSetting);
//mt_s32 MT_MPI_DISP_ApplySetting(MT_DRV_DISPLAY_E enDisp);

mt_s32 MT_MPI_DISP_SetColorbar(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetOutputEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);

mt_s32 MT_MPI_DISP_SetBrightness(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Brightness);
mt_s32 MT_MPI_DISP_GetBrightness(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Brightness);
mt_s32 MT_MPI_DISP_SetContrast(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Contrast);
mt_s32 MT_MPI_DISP_GetContrast(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Contrast);
mt_s32 MT_MPI_DISP_SetSaturation(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Saturation);
mt_s32 MT_MPI_DISP_GetSaturation(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Saturation);
mt_s32 MT_MPI_DISP_SetHuePlus(MT_DRV_DISPLAY_E enDisp, mt_u32 u32HuePlus);
mt_s32 MT_MPI_DISP_GetHuePlus(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32HuePlus);

mt_s32 MT_MPI_DISP_Suspend(mt_void);
mt_s32 MT_MPI_DISP_Resume(mt_void);
mt_s32 MT_MPI_DISP_SetDacOutputEnable(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetLayerShow(MT_DRV_DISPLAY_E enDisp,MT_DRV_DISP_LAYER_ID_E elayer,MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetPPMode(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_PPMODE_E enMode);
mt_s32 MT_MPI_DISP_VidLayerShow(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_GetVidLayerEnable(MT_BOOL *pbEnable);
mt_s32 MT_MPI_DISP_GetVideoSize(mt_u32 *pWidth, mt_u32 *pHeight);
mt_s32 MT_MPI_DISP_SetAlpha(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Alpha);
mt_s32 MT_MPI_DISP_GetAlpha(MT_DRV_DISPLAY_E enDisp, mt_u32 *u32Alpha);
mt_s32 MT_MPI_DISP_SetCscEnable(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetDenoiseEnable(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetAfdEnable(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetHdVideoEnable(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetSdVideoEnable(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetVdacOnOff(dac_index_t eDacId, MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetTrickMode(DISP_TRICK_MODE_E trickmode);
mt_s32 MT_MPI_DISP_SetUnblankMode(MT_DRV_DISP_UNBLANK_MODE_E unblank_mode);

mt_s32 MT_MPI_DISP_SetAlgCfg(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ALG_CFG_S *pstAlg);
mt_s32 MT_MPI_DISP_GetAlgCfg(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ALG_CFG_S *pstAlg);
mt_s32 MT_MPI_DISP_SetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg);
mt_s32 MT_MPI_DISP_GetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg);
mt_s32 MT_MPI_DISP_SetCgms(MT_DRV_DISPLAY_E enDisp, const MT_DRV_DISP_CGMSA_CFG_S *pstCgmsCgf);
mt_s32 MT_MPI_DISP_SetLowDelayEnable(mt_handle hCast, MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetDiOnOff(MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_ForceShowDS(mt_bool bStillLayer,MT_BOOL bEnable);
mt_s32 MT_MPI_DISP_SetTvCapability(MT_DRV_DISP_HDMI_MODE_E enTvCap);

mt_s32 MT_MPI_DISP_ResetHardware(MT_BOOL bHighSpeed);
mt_s32 MT_MPI_DISP_SetSdScalerEnable(mt_u32 bState);
mt_s32 MT_MPI_DISP_DumpScaler2OSD(MT_DRV_DISP_DUMP_SCALER_PARA_S *pstParam);
mt_s32 MT_MPI_DISP_Set_Sl_Hdr(MT_DRV_DISPLAY_E enDisp, mt_u32 transparent_mode, mt_u32 display_Brightness, mt_u32 tuning_level, mt_u32 display_OETF);
mt_s32 MT_MPI_DISP_GetSlHdrVersion(mt_u32 *pVer);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
