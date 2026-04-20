/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MPI_HDMI_H__
#define __MPI_HDMI_H__

#include "mt_unf_hdmi.h"
#include "mt_unf_disp.h"
#include "mt_drv_hdmi.h"
#include "mt_drv_disp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

mt_s32 MT_MPI_HDMI_Init(void);
mt_s32 MT_MPI_HDMI_DeInit(void);
mt_s32 MT_MPI_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_OPEN_PARA_S *pstOpenPara);
mt_s32 MT_MPI_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_MPI_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkCap);
mt_s32 MT_MPI_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr);
mt_s32 MT_MPI_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr);
mt_s32 MT_MPI_HDMI_SetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd);
mt_s32 MT_MPI_HDMI_GetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd, mt_u32 timeout);
mt_s32 MT_MPI_HDMI_CECStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_STATUS_S *pStatus);
mt_s32 MT_MPI_HDMI_CEC_Enable(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_MPI_HDMI_CEC_Disable(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_MPI_HDMI_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);
mt_s32 MT_MPI_HDMI_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);
mt_s32 MT_MPI_HDMI_Start(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_MPI_HDMI_Stop(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_MPI_HDMI_SetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E enDeepColor);
mt_s32 MT_MPI_HDMI_GetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E *penDeepColor);
mt_s32 MT_MPI_HDMI_SetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E enColorSpace);
mt_s32 MT_MPI_HDMI_GetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E *enColorSpace);
mt_s32 MT_MPI_HDMI_SetxvYCCMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bEnalbe);
mt_s32 MT_MPI_HDMI_SetAVMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute);

mt_s32 MT_MPI_HDMI_AVMute(void);
mt_s32 MT_MPI_HDMI_AVUnMute(void);
mt_s32 MT_MPI_HDMI_PreSetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E pstFmt);
mt_s32 MT_MPI_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E pstFmt);
//mt_s32 MT_MPI_HDMI_AudioChange(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);
//mt_s32 MT_MPI_HDMI_PlayStus(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32Stutus);
mt_s32 MT_MPI_HDMI_Force_GetEDID(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *u8Edid, mt_u32 *u32EdidLength);
mt_s32 MT_MPI_HDMI_ReadEDID(mt_u8 *u8Edid, mt_u32 *u32EdidLength);
mt_s32 MT_MPI_HDMI_RegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc);
mt_s32 MT_MPI_HDMI_UnRegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc);
mt_s32 MT_MPI_HDMI_LoadHDCPKey(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_LOAD_KEY_S *pstLoadKey);
mt_s32 MT_MPI_HDMI_Output_Set(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL enable);

mt_s32 MT_MPI_HDMI_GetStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_STATUS_S *pHdmiStatus);

mt_s32 MT_MPI_HDMI_GetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay);
mt_s32 MT_MPI_HDMI_SetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay);

mt_s32 MT_MPI_HDMI_Registers_Dump(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_MPI_HDMI_Register_Write(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 data);
mt_s32 MT_MPI_HDMI_Register_Read(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 *pData);
mt_s32 MT_MPI_HDMI_SetOsdName(MT_UNF_HDMI_ID_E enHdmi,mt_u8 *osdName);

mt_s32 MT_MPI_HDMI_Docmd(MT_UNF_HDMI_ID_E enHdmi, hdmi_io_cmd_t cmd, mt_u32 data1, mt_u32 data2);

//mt_s32 MT_MPI_AO_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);
//mt_s32 MT_MPI_AO_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);
mt_s32 MT_MPI_HDMI_ReadRawEDID(mt_u8 *u8RawEdid);
mt_s32 MT_MPI_HDMI_HDCP_Enable(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_MPI_HDMI_HDCP_Disable(MT_UNF_HDMI_ID_E enHdmi);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __MPI_HDMI_H__ */
