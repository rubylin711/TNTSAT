/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __SAMPLE_HDMI_COMMON_H__
#define __SAMPLE_HDMI_COMMON_H__

#include "mt_unf_hdmi.h"

typedef void (*User_HDMI_CallBack)(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData);

MT_UNF_ENC_FMT_E stringToUnfFmt(mt_char *pszFmt);
mt_s32 MTADP_HDMI_Init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt);
mt_s32 MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_E enHDMIId);
//mt_s32 MTADP_HDMI_SetAdecAttr(MT_UNF_SND_INTERFACE_E enInterface, MT_UNF_SAMPLE_RATE_E enRate);
mt_void HDMI_PrintSinkCap(MT_UNF_EDID_BASE_INFO_S *pCapbility);
void HDMI_HotPlug_Proc_gst(mt_void *pPrivateData);
mt_void HDMI_UnPlug_Proc_gst(mt_void *pPrivateData);
mt_void HDMI_HdcpFail_Proc_gst(mt_void *pPrivateData);
mt_void HDMI_HdcpSuccess_Proc_gst(mt_void *pPrivateData);
mt_void HDMI_Event_Proc_gst(MT_UNF_HDMI_EVENT_TYPE_E event,
			mt_void *pPrivateData);

#endif /* #if pub_HDMI_H_ */

