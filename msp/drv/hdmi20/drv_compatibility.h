/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_drv_hdmi.h"

typedef struct hiHDMI_DELAY_TIME_S {
	MT_U8           u8IDManufactureName[4];   /**<Manufacture name*//**<CNcomment:设备厂商标识 */
	mt_u32          u32IDProductCode;         /**<Product code*//**<CNcomment:设备ID */
	mt_u32          u32DelayTimes;
	mt_u32          u32MuteDelay;
	MT_U8           u8ProductType[32];    /**<Product Type*//**<CNcomment:产品型号 */
} HDMI_DELAY_TIME_S;

mt_s32 GetFormatDelay(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *DelayTime);
mt_s32 GetmuteDelay(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *DelayTime);

void SetGlobalFmtDelay(mt_u32 Delay);
mt_u32 GetGlobalFmtDelay(void);
void SetForceDelayMode(MT_BOOL bforceFmtDelay, MT_BOOL bforceMuteDelay);
MT_BOOL IsForceFmtDelay(void);
MT_BOOL IsForceMuteDelay(void);

void SetGlobalMuteDelay(mt_u32 Delay);
mt_u32 GetGlobalsMuteDelay(void);

void Check1stOE(MT_UNF_HDMI_ID_E enHdmi);

MT_BOOL Is3716Cv200ECVer(void);

