/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mt_unf_hdmi.h"
//#include "mpi_hdmi.h"
#include "mt_mpi_hdmi.h"
#include "mt_module_debug.h"

static const mt_u8 s_szHDMIVersion[] __attribute__((used)) = "SDK_VERSION:[" MKMARCOTOSTR(SDK_VERSION) "] Build Time:[" __DATE__ ", " __TIME__ "]";

mt_s32 MT_UNF_HDMI_Init(void)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Init();

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_DeInit(void)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_DeInit();

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_OPEN_PARA_S *pstOpenPara)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Open(enHdmi, pstOpenPara);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Close(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_STATUS_S *pHdmiStatus)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_GetStatus(enHdmi, pHdmiStatus);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkAttr)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_GetSinkCapability(enHdmi, pstSinkAttr);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetAttr(enHdmi, pstAttr);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_GetAttr(enHdmi, pstAttr);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetCECCommand(enHdmi, pCECCmd);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd, mt_u32 timeout)
{
	mt_s32 s32Ret;

	if (timeout > 3000) {
		return -1;
	}

	s32Ret = MT_MPI_HDMI_GetCECCommand(enHdmi, pCECCmd, timeout);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_CECStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_STATUS_S *pStatus)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_CECStatus(enHdmi, pStatus);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_CEC_Enable(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_CEC_Enable(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_CEC_Disable(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_CEC_Disable(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_HDCP_Enable(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_HDCP_Enable(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_HDCP_Disable(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_HDCP_Disable(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetInfoFrame(enHdmi, pstInfoFrame);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_GetInfoFrame(enHdmi, enInfoFrameType, pstInfoFrame);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Start(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Stop(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Stop(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E enDeepColor)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetDeepColor(enHdmi, enDeepColor);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E *penDeepColor)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_GetDeepColor(enHdmi, penDeepColor);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E enColorSpace)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetColorSpace(enHdmi, enColorSpace);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E *penColorSpace)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_GetColorSpace(enHdmi, penColorSpace);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetxvYCCMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bEnable)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetxvYCCMode(enHdmi, bEnable);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetAVMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetAVMute(enHdmi, bAvMute);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Force_GetEDID(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *u8Edid, mt_u32 *u32EdidLength)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Force_GetEDID(enHdmi, u8Edid, u32EdidLength);

	return s32Ret;
}
mt_s32 MT_UNF_HDMI_ReadEDID(mt_u8 *u8Edid, mt_u32 *u32EdidLength)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_ReadEDID(u8Edid, u32EdidLength);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_RegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_RegCallbackFunc(enHdmi, pstCallbackFunc);

	return s32Ret;
}
mt_s32 MT_UNF_HDMI_UnRegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_UnRegCallbackFunc(enHdmi, pstCallbackFunc);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_LoadHDCPKey(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_LOAD_KEY_S *pstLoadKey)
{
	mt_s32 s32Ret = MT_SUCCESS;

	s32Ret = MT_MPI_HDMI_LoadHDCPKey(enHdmi, pstLoadKey);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL enable)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Output_Set(enHdmi, enable);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetDelay(enHdmi, pstDelay);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_GetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_GetDelay(enHdmi, pstDelay);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Registers_Dump(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Registers_Dump(enHdmi);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Register_Write(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 data)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Register_Write(enHdmi, addr, data);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Register_Read(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 *pstData)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_Register_Read(enHdmi, addr, pstData);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_ReadRawEDID(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *u8RawEdid)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_ReadRawEDID(u8RawEdid);

	return s32Ret;
}

mt_s32 MT_UNF_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E Fmt)
{
	mt_s32 s32Ret;
	s32Ret = MT_MPI_HDMI_SetFormat(enHdmi, Fmt);
	return s32Ret;
}

mt_s32 MT_UNF_HDMI_PreSetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E Fmt)
{
	mt_s32 s32Ret;
	s32Ret = MT_MPI_HDMI_PreSetFormat(enHdmi, Fmt);
	return s32Ret;
}

mt_s32 MT_UNF_HDMI_Set_Osdname(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *osdName)
{
	mt_s32 s32Ret;

	s32Ret = MT_MPI_HDMI_SetOsdName(enHdmi, osdName);

	return s32Ret;
}

/*------------------------------END--------------------------------*/
