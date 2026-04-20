#ifndef __SAMPLE_HDMI_COMMON_H__
#define __SAMPLE_HDMI_COMMON_H__

#include "mt_unf_hdmi.h"

typedef void (*User_HDMI_CallBack)(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData);

typedef enum {
    MT_HDMI_CEC_STANDBY_DEFAULT,
    MT_HDMI_CEC_STANDBY_FAKE,
    MT_HDMI_CEC_STANDBY_PMU,
    MT_HDMI_CEC_STANDBY_STR,
    MT_HDMI_CEC_STANDBY_BUTT,
} mt_hdmi_cec_standby_mode;

MT_UNF_ENC_FMT_E stringToUnfFmt(mt_char *pszFmt);
mt_s32 MTADP_HDMI_Init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt);
mt_s32 MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_E enHDMIId);
mt_void MTADP_HDMI_Set_HdcpEnable(mt_u32 enable);
MT_U32 MTADP_HDMI_Get_HdcpEnable(MT_VOID);
mt_void MTADP_HDMI_SetStandbyStatus(mt_s32 standby_flag);
mt_void MTADP_HDMI_GetStandbyStatus(mt_s32 *standby_flag);
//mt_s32 MTADP_HDMI_SetAdecAttr(MT_UNF_SND_INTERFACE_E enInterface, MT_UNF_SAMPLE_RATE_E enRate);
mt_void HDMI_PrintSinkCap(MT_UNF_EDID_BASE_INFO_S *pCapbility);
mt_void MTADP_HDMI_Set_Standby_mode(mt_hdmi_cec_standby_mode standby_mode);
mt_u32 MTADP_HDMI_Get_Standby_mode(mt_void);
mt_s32 MTADP_HDMI_ReadHdcpKey(MT_UNF_HDMI_ID_E enHDMIId, mt_u8 *p_key, mt_u32 key_len);

#endif /* #if pub_HDMI_H_ */

