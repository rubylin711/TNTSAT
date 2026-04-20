/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/kernel.h>
//include memset
#include <linux/interrupt.h>

#include "drv_global.h"
#include "drv_disp_ext.h"

#include "mt_unf_audio.h"
#include "mt_unf_hdmi.h"
#include "mt_drv_hdmi.h"
#include "mt_drv_module.h"

//#include "si_hdmitx.h"
//#include "si_edid.h"

//#include "si_timer.h"

static HDMI_COMM_ATTR_S g_stHdmiCommParam;

static HDMI_CHN_ATTR_S  g_stHdmiChnParam[MT_UNF_HDMI_ID_BUTT];

static MT_UNF_EDID_BASE_INFO_S g_stEdidInfo[MT_UNF_HDMI_ID_BUTT];

static HDMI_PRIVATE_EDID_S g_stPriEdidInfo[MT_UNF_HDMI_ID_BUTT];

static MT_BOOL            bForceOutput = MT_FALSE;

//extern DISP_EXPORT_FUNC_S *disp_func_ops;

MT_DRV_HDMI_AUDIO_CAPABILITY_S g_stHdmiOldAudio;

mt_u32 g_u32DDCDelayCount = 0x1e;

//MT_U8 g_u8ExtEdid[EDID_SIZE] = {0};
//mt_u32 g_u32ExtEdidLengh = 0;
MT_BOOL g_bExtEdid[MT_UNF_HDMI_ID_BUTT] = {MT_FALSE};
HDMI_EDID_S g_ExtEdid[MT_UNF_HDMI_ID_BUTT];

MT_DRV_HDMI_AUDIO_CAPABILITY_S *DRV_Get_OldAudioCap()
{
	return &g_stHdmiOldAudio;
}

HDMI_COMM_ATTR_S *DRV_Get_CommAttr()
{
	COM_INFO("Get_CommAttr \n");
	return &g_stHdmiCommParam;
}

void DRV_PrintCommAttr()
{
	MT_PRINT("g_stHdmiCommParam \n"
			 "bOpenGreenChannel :%d \n"
			 "bOpenedInBoot :%d \n"
			 "kThreadTimerStop :%d \n"
			 "enVidInMode :%d \n",
			 g_stHdmiCommParam.bOpenMce2App,
			 g_stHdmiCommParam.bOpenedInBoot,
			 g_stHdmiCommParam.kThreadTimerStop,
			 g_stHdmiCommParam.enVidInMode);
}

mt_s32 DRV_Get_IsMce2App(mt_void)
{
	return g_stHdmiCommParam.bOpenMce2App;
}

void DRV_Set_Mce2App(MT_BOOL bSmooth)
{
	COM_INFO("Set_GreenChannel bGreen : %d \n", bSmooth);
	g_stHdmiCommParam.bOpenMce2App = bSmooth;
}

mt_s32 DRV_Get_IsOpenedInBoot(mt_void)
{
	return g_stHdmiCommParam.bOpenedInBoot;
}

void DRV_Set_OpenedInBoot(MT_BOOL bOpened)
{
	COM_INFO("Set_OpenedInBoot bOpend : %d \n", bOpened);
	g_stHdmiCommParam.bOpenedInBoot = bOpened;
}

mt_s32 DRV_Get_IsThreadStoped(mt_void)
{
	return g_stHdmiCommParam.kThreadTimerStop;
}

void DRV_Set_ThreadStop(MT_BOOL bStop)
{
	COM_INFO("Set_ThreadStatus bStop : %d \n", bStop);
	g_stHdmiCommParam.kThreadTimerStop = bStop;
}

MT_UNF_HDMI_VIDEO_MODE_E DRV_Get_VIDMode(mt_void)
{
	return g_stHdmiCommParam.enVidInMode;
}

void DRV_Set_VIDMode(MT_UNF_HDMI_VIDEO_MODE_E enVInMode)
{
	g_stHdmiCommParam.enVidInMode = enVInMode;
}

HDMI_CHN_ATTR_S *DRV_Get_ChnAttr()
{
	return g_stHdmiChnParam;
}

HDMI_ATTR_S *DRV_Get_HDMIAttr(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stHDMIAttr;
}

mt_u32 DRV_HDMI_SetDefaultAttr(mt_void)
{
	MT_DRV_DISP_FMT_E   enEncFmt = MT_DRV_DISP_FMT_1080i_50;
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(MT_UNF_HDMI_ID_0);
	HDMI_AUDIO_ATTR_S   *pstAudAttr = DRV_Get_AudioAttr(MT_UNF_HDMI_ID_0);
	HDMI_APP_ATTR_S     *pstAppAttr = DRV_Get_AppAttr(MT_UNF_HDMI_ID_0);
	hdmi_video_config_t	*pstMtAvParams = DRV_Get_Av_Params(MT_UNF_HDMI_ID_0);
	HDMI_APP_ATTRMT_S     *pstAppAttrMt = DRV_Get_AppAttrMt(MT_UNF_HDMI_ID_0);
	DISP_EXPORT_FUNC_S  *disp_func_ops = MT_NULL;
	#if HDMI_DISPLAY_READY
	mt_s32              ret = MT_SUCCESS;
	#endif

	memset(pstVidAttr, 0, sizeof(HDMI_VIDEO_ATTR_S));
	memset(pstAudAttr, 0, sizeof(HDMI_AUDIO_ATTR_S));
	memset(pstAppAttr, 0, sizeof(HDMI_APP_ATTR_S));
	memset(pstAppAttrMt, 0, sizeof(HDMI_APP_ATTRMT_S));
	memset(pstMtAvParams, 0, sizeof(hdmi_video_config_t));

	#if HDMI_DISPLAY_READY
	ret = mt_drv_module_getfunction(MT_ID_DISP, (mt_void**)&disp_func_ops);
	if ((NULL == disp_func_ops) || (ret != MT_SUCCESS)) {
		COM_FATAL("can't get disp funcs!\n");
		//return MT_FAILURE;
	}
	#endif

	// GetFormat need add 3d mode
	if (disp_func_ops && disp_func_ops->pfnDispGetFormat) {
		disp_func_ops->pfnDispGetFormat(MT_DRV_DISPLAY_1, &enEncFmt);
	}

	// if don't get disp fmt,then use default fmt
	pstVidAttr->enVideoFmt = enEncFmt;
	if ((enEncFmt < MT_DRV_DISP_FMT_861D_640X480_60) || DRV_Get_Is4KFmt(enEncFmt)) {
		pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
	} else {
		pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}

	pstVidAttr->b3DEnable = MT_FALSE;
	pstVidAttr->u83DParam = MT_UNF_EDID_3D_BUTT;

	pstAudAttr->enBitDepth = MT_UNF_BIT_DEPTH_16;
	pstAudAttr->enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
	pstAudAttr->enSampleRate = MT_UNF_SAMPLE_RATE_48K;
	pstAudAttr->u32Channels = 2;
	pstAudAttr->bIsMultiChannel = MT_FALSE;

	pstMtAvParams->input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
	pstMtAvParams->input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
	pstMtAvParams->input_v_cfg.std = SII_DRV_CONV_STD__BT_709;

	return MT_SUCCESS;
}

HDMI_APP_ATTR_S   *DRV_Get_AppAttr(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stHDMIAttr.stAppAttr;
}

HDMI_VIDEO_ATTR_S *DRV_Get_VideoAttr(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stHDMIAttr.stVideoAttr;
}

HDMI_AUDIO_ATTR_S *DRV_Get_AudioAttr(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stHDMIAttr.stAudioAttr;
}

MT_UNF_EDID_BASE_INFO_S *DRV_Get_SinkCap(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stEdidInfo[enHdmi];
}

MT_BOOL DRV_Get_IsNeedForceUpdate(MT_UNF_HDMI_ID_E enHdmi)
{
	COM_INFO("Get g_stHdmiChnParam[%d].ForceUpdateFlag %d\n", enHdmi, g_stHdmiChnParam[enHdmi].ForceUpdateFlag);
	return g_stHdmiChnParam[enHdmi].ForceUpdateFlag;
}

void DRV_Set_ForceUpdateFlag(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bupdate)
{
	COM_INFO("Set g_stHdmiChnParam[%d].ForceUpdateFlag %d\n", enHdmi, g_stHdmiChnParam[enHdmi].ForceUpdateFlag);
	g_stHdmiChnParam[enHdmi].ForceUpdateFlag = bupdate;
}

MT_BOOL DRV_Get_IsNeedPartUpdate(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].partUpdateFlag;
}

void DRV_Set_PartUpdateFlag(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bupdate)
{
	g_stHdmiChnParam[enHdmi].partUpdateFlag = bupdate;
}

HDMI_PROC_EVENT_S *DRV_Get_EventList(MT_UNF_HDMI_ID_E enHdmi)
{
	//eventlist is not associateed to channel,so force return  MT_UNF_HDMI_ID_0
	return g_stHdmiChnParam[MT_UNF_HDMI_ID_0].eventList;
}

MT_UNF_HDMI_CEC_STATUS_S *DRV_Get_CecStatus(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stCECStatus;
}

MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *DRV_Get_AviInfoFrm(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stAVIInfoFrame;
}

MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *DRV_Get_AudInfoFrm(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stAUDInfoFrame;
}

MT_BOOL DRV_Get_IsChnOpened(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].bOpen;
}

void DRV_Set_ChnOpen(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bChnOpen)
{
	g_stHdmiChnParam[enHdmi].bOpen = bChnOpen;
}

MT_BOOL DRV_Get_IsChnStart(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].bStart;
}

void DRV_Set_ChnStart(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bChnStart)
{
	g_stHdmiChnParam[enHdmi].bStart = bChnStart;
}

MT_BOOL DRV_Get_IsCECEnable(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].bCECEnable;
}

void DRV_Set_CECEnable(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bCecEnable)
{
	g_stHdmiChnParam[enHdmi].bCECEnable = bCecEnable;
}

MT_BOOL DRV_Get_IsCECStart(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].bCECStart;
}

void DRV_Set_CECStart(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bCecStart)
{
	g_stHdmiChnParam[enHdmi].bCECStart = bCecStart;
}

MT_BOOL DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].bValidSinkCap;
}

HDMI_APP_ATTRMT_S *DRV_Get_AppAttrMt(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].stAppAttrMt;
}

hdmi_video_config_t *DRV_Get_Av_Params(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stHdmiChnParam[enHdmi].mt_av_params;
}

void DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bSinkValid)
{
	g_stHdmiChnParam[enHdmi].bValidSinkCap = bSinkValid;
}

void DRV_Set_DefaultOutputMode(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEFAULT_ACTION_E enDefaultMode)
{
	g_stHdmiChnParam[enHdmi].enDefaultMode = enDefaultMode;
}

MT_UNF_HDMI_DEFAULT_ACTION_E DRV_Get_DefaultOutputMode(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].enDefaultMode;
}

hdmi_notify_info_t* DRV_Get_NotifyHdl(MT_UNF_HDMI_ID_E enHdmi)
{
	return &(g_stHdmiChnParam[enHdmi].notify_info[0]);
}

void DRV_Set_DDCSpeed(mt_u32 delayCount)
{
	g_u32DDCDelayCount = delayCount;
}

mt_u32 DRV_Get_DDCSpeed(void)
{
	return g_u32DDCDelayCount;
}

HDMI_EDID_S *DRV_Get_UserEdid(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_ExtEdid[enHdmi];
}

void DRV_Set_UserEdid(MT_UNF_HDMI_ID_E enHdmi, HDMI_EDID_S *pEDID)
{
	//only memset one ExtEdid,NOt All
	memset(&g_ExtEdid[enHdmi], 0, sizeof(HDMI_EDID_S));

	memcpy(&g_ExtEdid[enHdmi], pEDID, sizeof(HDMI_EDID_S));
}

MT_BOOL DRV_Get_IsUserEdid(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_bExtEdid[enHdmi];
}

void DRV_Set_UserEdidMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bUserEdid)
{
	g_bExtEdid[enHdmi] = bUserEdid;
}

VSDB_MODE_E DRV_Get_VSDBMode(MT_UNF_HDMI_ID_E enHdmi)
{
	return g_stHdmiChnParam[enHdmi].enVSDBMode;
}

void DRV_Set_VSDBMode(MT_UNF_HDMI_ID_E enHdmi, VSDB_MODE_E enVSDBMode)
{
	g_stHdmiChnParam[enHdmi].enVSDBMode = enVSDBMode;
}

//HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);
MT_BOOL DRV_Get_Is4KFmt(MT_DRV_DISP_FMT_E enFmt)
{
	if ((enFmt == MT_DRV_DISP_FMT_3840X2160_24)
			|| (enFmt == MT_DRV_DISP_FMT_3840X2160_25)
			|| (enFmt == MT_DRV_DISP_FMT_3840X2160_30)
			|| (enFmt == MT_DRV_DISP_FMT_4096X2160_24)) {
		return MT_TRUE;
	}
	return MT_FALSE;
}

// 640*480 is CEA861 Fmt,so not In this func
MT_BOOL DRV_Get_IsLCDFmt(MT_DRV_DISP_FMT_E enFmt)
{
	if ((enFmt == MT_DRV_DISP_FMT_VESA_800X600_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1024X768_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1280X720_60)

			|| (enFmt == MT_DRV_DISP_FMT_VESA_1280X800_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1280X1024_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1360X768_60)

			|| (enFmt == MT_DRV_DISP_FMT_VESA_1366X768_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1400X1050_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1440X900_60)

			|| (enFmt == MT_DRV_DISP_FMT_VESA_1440X900_60_RB)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1600X900_60_RB)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1600X1200_60)

			|| (enFmt == MT_DRV_DISP_FMT_VESA_1680X1050_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1680X1050_60_RB)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1920X1080_60)

			|| (enFmt == MT_DRV_DISP_FMT_VESA_1920X1200_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1920X1440_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_1680X1050_60_RB)

			|| (enFmt == MT_DRV_DISP_FMT_VESA_2048X1152_60)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_2560X1440_60_RB)
			|| (enFmt == MT_DRV_DISP_FMT_VESA_2560X1600_60_RB)

			|| (enFmt == MT_DRV_DISP_FMT_CUSTOM)) {
		return MT_TRUE;
	}
	return MT_FALSE;
}

MT_BOOL DRV_Get_IsPixelRepeatFmt(MT_DRV_DISP_FMT_E enFmt)
{
	if ((enFmt == MT_DRV_DISP_FMT_PAL)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_B)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_B1)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_D)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_D1)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_G)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_H)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_K)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_I)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_M)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_N)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_Nc)
			|| (enFmt == MT_DRV_DISP_FMT_PAL_60)
			|| (enFmt == MT_DRV_DISP_FMT_1440x576i_50)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_SIN)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_COS)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_L)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_B)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_G)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_D)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_K)
			|| (enFmt == MT_DRV_DISP_FMT_SECAM_H)
			|| (enFmt == MT_DRV_DISP_FMT_NTSC)
			|| (enFmt == MT_DRV_DISP_FMT_NTSC_J)
			|| (enFmt == MT_DRV_DISP_FMT_1440x480i_60)
			|| (enFmt == MT_DRV_DISP_FMT_NTSC_443)) {
		return MT_TRUE;
	}
	return MT_FALSE;
}

HDMI_PRIVATE_EDID_S *DRV_Get_PriSinkCap(MT_UNF_HDMI_ID_E enHdmi)
{
	return &g_stPriEdidInfo[enHdmi];
}

MT_BOOL DRV_Get_IsForceOutput(void)
{
	//return g_stHdmiChnParam[enHdmi].bForceOutput;
	return bForceOutput;
}

void DRV_Set_ForceOutputMode(MT_BOOL bForce)
{
	//g_stHdmiChnParam[enHdmi].bForceOutput = bForce;
	bForceOutput = bForce;
}

