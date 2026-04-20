/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//#include <linux/list.h>

#include <linux/kernel.h>

#include <linux/device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/poll.h>
//#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include "drv_compatibility.h"
#include "drv_global.h"
#include "drv_hdmi.h"
#include "mt_osal.h"
//#include "si_phy.h"
//#include "si_delay.h"
#include "drv_reg_proc.h"
#include "si_datatypes.h"
#include "si_misc.h"

// product delay

//Panasonic_TH-L32CH3C
HDMI_DELAY_TIME_S MEI_TH_L32CH3C = {
	"MEI", 49924, 600, 200, "MEI_TH_L32CH3C",
};

//Skyworth SKY_47E760A
HDMI_DELAY_TIME_S SKY_47E760A = {
	"SKY", 48, 100, 0, "SKY_47E760A",
};

//Skyworth 24E60HR
HDMI_DELAY_TIME_S SKW_24E60HR = {
	"SKW", 48, 1200, 0, "24E60HR",
};

//Sharp 24E60HR when close oe in preFormat ,blink green bg
HDMI_DELAY_TIME_S SHP_LCD_32Z100AS = {
	"SHP", 4149, 0, 0, "LCD-32Z100AS",
};

//Sumsung LA40B550K1F
HDMI_DELAY_TIME_S SAM_LA40B550K1F = {
	"SAM", 1289, 100, 0, "LA40B550K1F",
};

// Brand Delay
#define SKYWORTH_DELAY 600

HDMI_DELAY_TIME_S *WholeList[] = {
	&MEI_TH_L32CH3C, &SKY_47E760A, &SKW_24E60HR, &SHP_LCD_32Z100AS, &SAM_LA40B550K1F, NULL
};

static MT_BOOL Global_Fmt_Force = MT_FALSE;
static MT_BOOL Global_Mute_Force = MT_FALSE;

static mt_u32 Global_Fmt_Delay = 0;
static mt_u32 Global_Mute_Delay = 0;

mt_s32 GetFormatDelay(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *DelayTime)
{
	#if SI_COM_ALL_PASS
	mt_u32 i = 0;
	mt_u32 Delay = 0;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(enHdmi);

	if (DelayTime == NULL) {
		COM_ERR("Null Point Error \n");
		return MT_FAILURE;
	}

	if (IsForceFmtDelay()) {
		Delay = GetGlobalFmtDelay();
		COM_INFO("HDMI SetFormat Force Delay %dms \n", GetGlobalFmtDelay());
	} else {
		for (i = 0; WholeList[i] != NULL; i++) {
			//if sincap is invalid, the data can not euqal to compatibility list
			if ((pSinkCap->stMfrsInfo.u32ProductCode == WholeList[i]->u32IDProductCode) &&
					(!MT_OSAL_Strncmp(pSinkCap->stMfrsInfo.u8MfrsName, WholeList[i]->u8IDManufactureName, DEF_FILE_NAMELENGTH))) {
				Delay = WholeList[i]->u32DelayTimes;
				COM_INFO("Sink %s need Delay %d ms \n", WholeList[i]->u8ProductType, Delay);
			}
		}

		if (Delay == 0) {
			//In SKY_47E760A MfrsName SKY,And 24E60HR SKW
			if (!MT_OSAL_Strncmp(pSinkCap->stMfrsInfo.u8MfrsName, "SKY", DEF_FILE_NAMELENGTH) ||
					!MT_OSAL_Strncmp(pSinkCap->stMfrsInfo.u8MfrsName, "SKW", DEF_FILE_NAMELENGTH)) {
				Delay = SKYWORTH_DELAY;
				COM_INFO("Sink SKYWorth should Delay %d ms \n", Delay);
			}
		}

		if (Delay == 0) {
			Delay = GetGlobalFmtDelay();
			COM_INFO("HDMI SetFormat Global Delay %dms \n", GetGlobalFmtDelay());
		}
	}

	*DelayTime = Delay;
	#endif
	return MT_SUCCESS;
};

void SetGlobalFmtDelay(mt_u32 Delay)
{
	Global_Fmt_Delay = Delay;
}

mt_u32 GetGlobalFmtDelay(void)
{
	return Global_Fmt_Delay;
}

void SetForceDelayMode(MT_BOOL bforceFmtDelay, MT_BOOL bforceMuteDelay)
{
	Global_Fmt_Force = bforceFmtDelay;
	Global_Mute_Force = bforceMuteDelay;
}

MT_BOOL IsForceFmtDelay(void)
{
	return Global_Fmt_Force;
}

MT_BOOL IsForceMuteDelay(void)
{
	return Global_Mute_Force;
}

mt_s32 GetmuteDelay(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *DelayTime)
{
	#if SI_COM_ALL_PASS
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);

	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(enHdmi);
	mt_u32 MuteDelay = 0;
	mt_u32 i = 0;

	if (DelayTime == NULL) {
		COM_ERR("Null Point Error \n");
		return MT_FAILURE;
	}

	if (IsForceMuteDelay()) {
		MuteDelay = GetGlobalsMuteDelay();
		COM_INFO("HDMI SetFormat Force Delay %dms \n", GetGlobalFmtDelay());
	} else {
		for (i = 0; WholeList[i] != NULL; i++) {
			if ((pSinkCap->stMfrsInfo.u32ProductCode == WholeList[i]->u32IDProductCode) &&
					(!MT_OSAL_Strncmp(pSinkCap->stMfrsInfo.u8MfrsName, WholeList[i]->u8IDManufactureName, DEF_FILE_NAMELENGTH))) {
				MuteDelay = WholeList[i]->u32MuteDelay;
				COM_INFO("Sink %s need Avmute Delay %d ms \n", WholeList[i]->u8ProductType, MuteDelay);
			}
		}

		if (MuteDelay == 0) {
			if (pstVidAttr->enVideoFmt == MT_DRV_DISP_FMT_1080P_30 ||
					pstVidAttr->enVideoFmt == MT_DRV_DISP_FMT_1080P_25 ||
					pstVidAttr->enVideoFmt == MT_DRV_DISP_FMT_1080P_24) {
				//minimum requirement (2 frames + 50%) time needed in 24Hz timing
				//(1/24hz)*2 + 1/24Hz * 0.5 = 105ms
				MuteDelay = 120;
			} else {
				//minimum requirement (2 frames + 50%) time needed in 24Hz timing
				//(1/50hz)*2 + 1/50Hz * 0.5 = 50ms
				MuteDelay = 50;
			}
		}
	}

	*DelayTime = MuteDelay;
	#endif
	return MT_SUCCESS;
}

void SetGlobalMuteDelay(mt_u32 Delay)
{
	Global_Mute_Delay = Delay;
}

mt_u32 GetGlobalsMuteDelay(void)
{
	return Global_Mute_Delay;
}

typedef struct hiHDMI_Mfrs_INFO_S {
	MT_U8           u8IDManufactureName[4];   /**<Manufacture name*//**<CNcomment:设备厂商标识 */
	mt_u32          u32IDProductCode;         /**<Product code*//**<CNcomment:设备ID */
	MT_U8           u8ProductType[32];    /**<Product Type*//**<CNcomment:产品型号 */
} HDMI_Mfrs_INFO_S;

//Sony KLV-32J400A
HDMI_Mfrs_INFO_S SNY_32J400A = {
	"SNY", 23809, "KLV-32J400A",
};

HDMI_Mfrs_INFO_S *WholeOEList[] = {
	&SNY_32J400A, NULL
};

#if SI_COM_ALL_PASS
	static MT_BOOL bOeChanged = MT_FALSE;
#endif

void Check1stOE(MT_UNF_HDMI_ID_E enHdmi)
{
	#if SI_COM_ALL_PASS
	mt_u32 i = 0;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(enHdmi);

	if (bOeChanged) {
		return ;
	}

	bOeChanged = MT_TRUE;

	for (i = 0; WholeOEList[i] != NULL; i++) {
		//if sincap is invalid, the data can not euqal to compatibility list
		if ((pSinkCap->stMfrsInfo.u32ProductCode == WholeOEList[i]->u32IDProductCode) &&
				(!MT_OSAL_Strncmp(pSinkCap->stMfrsInfo.u8MfrsName, WholeOEList[i]->u8IDManufactureName, DEF_FILE_NAMELENGTH))) {
			COM_INFO("Sink %s need Reopen OE \n", WholeOEList[i]->u8ProductType);

			SI_TX_PHY_DisableHdmiOutput();
			DelayMS(100);
			SI_TX_PHY_EnableHdmiOutput();
			return ;
		}
	}

	if ((!MT_OSAL_Strncmp(pSinkCap->stMfrsInfo.u8MfrsName, "SNY", DEF_FILE_NAMELENGTH))) {
		if (pSinkCap->stMfrsInfo.u32Year == 2008 || pSinkCap->stMfrsInfo.u32Year == 2009) {
			COM_INFO("Sony 2008-2009 series need Reopen OE \n");

			SI_TX_PHY_DisableHdmiOutput();
			DelayMS(100);
			SI_TX_PHY_EnableHdmiOutput();
			return ;
		}
	}

	#endif

}

MT_BOOL Is3716Cv200ECVer(void)
{
	mt_u32 u32Reg = 0;
	//DRV_HDMI_ReadRegister(EC_3716CV200_VER,&u32Reg);

	u32Reg = (u32Reg & 0xff00) >> 8;
	COM_INFO("3716cv200 EC Version tag:0x%02x \n", u32Reg);

	if (u32Reg == 0x41) {
		COM_INFO("3716cv200 EC Version \n");
		return MT_TRUE;
	} else if (u32Reg == 0x40) {
		COM_INFO("3716cv200 original Version \n");
		return MT_FALSE;
	} else {
		COM_ERR("unKnown 3716cv200 version,force to 3716cv200 default \n");
		return MT_FALSE;
	}
}

