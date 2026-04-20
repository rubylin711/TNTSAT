/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/***********************************************************************************/
/*  Copyright (c) 2002-2006, Silicon Image, Inc.  All rights reserved.             */
/*  No part of this work may be reproduced, modified, distributed, transmitted,    */
/*  transcribed, or translated into any language or computer format, in any form   */
/*  or by any means without written permission of: Silicon Image, Inc.,            */
/*  1060 East Arques Avenue, Sunnyvale, California 94085                           */
/***********************************************************************************/
#ifndef _SI_MISC_
#define _SI_MISC_

#include "drv_global.h"

#define CABLE_UNPLUG_ 0
#define CABLE_PLUGIN_CHECK_EDID 1
#define CABLE_PLUGIN_CHECK_EDID_OK 2
#define CABLE_PLUGIN_CHECK_EDID_CORRUPTED 3
#define CABLE_PLUGIN_HDMI_OUT  4
#define CABLE_PLUGIN_DVI_OUT   5

#define siiIsTClockStable()		(SiiDrvCraRdReg8((ulong)NULL, REG_ADDR__SYS_STAT) & BIT_MSK__SYS_STAT__P_STABLE)

typedef enum HDMI_TMDS_CLK_RATE_E {
	HDMI_TMDS_CLK_IDLE,
	HDMI_TMDS_CLK_27MHZ_1X,
	HDMI_TMDS_CLK_27MHZ_1D25X,
	HDMI_TMDS_CLK_27MHZ_1D5X,
	HDMI_TMDS_CLK_74MHZ_1X,
	HDMI_TMDS_CLK_74MHZ_1D25X,
	HDMI_TMDS_CLK_74MHZ_1D5X,
	HDMI_TMDS_CLK_148MHZ_1X,
	HDMI_TMDS_CLK_148MHZ_1D25X,
	HDMI_TMDS_CLK_148MHZ_1D5X,
	HDMI_TMDS_CLK_297MHZ_1X,
	HDMI_TMDS_CLK_297MHZ_1D25X,
	HDMI_TMDS_CLK_297MHZ_1D5X,
	HDMI_TMDS_CLK_594MHZ_1X,
	HDMI_TMDS_CLK_594MHZ_1D25X,
	HDMI_TMDS_CLK_594MHZ_1D5X,
	HDMI_TMDS_CLK_27_1P001MHZ_1X,
	HDMI_TMDS_CLK_27_1P001MHZ_1D25X,
	HDMI_TMDS_CLK_27_1P001MHZ_1D5X,
	HDMI_TMDS_CLK_74_1P001MHZ_1X,
	HDMI_TMDS_CLK_74_1P001MHZ_1D25X,
	HDMI_TMDS_CLK_74_1P001MHZ_1D5X,
	HDMI_TMDS_CLK_148_1P001MHZ_1X,
	HDMI_TMDS_CLK_148_1P001MHZ_1D25X,
	HDMI_TMDS_CLK_148_1P001MHZ_1D5X,
	HDMI_TMDS_CLK_297_1P001MHZ_1X,
	HDMI_TMDS_CLK_297_1P001MHZ_1D25X,
	HDMI_TMDS_CLK_297_1P001MHZ_1D5X,
	HDMI_TMDS_CLK_594_1P001MHZ_1X,
	HDMI_TMDS_CLK_594_1P001MHZ_1D25X,
	HDMI_TMDS_CLK_594_1P001MHZ_1D5X,
	HDMI_TMDS_CLK_BUFF,
} HDMI_TMDS_CLK_RATE_T;

typedef struct HDMI_UBOOT_AVINFO_S {
	SiiTmdsMode_t	hdmi_mode;
	mt_u32	vic;
	MT_DRV_DISP_FMT_E enFmt;
	MT_UNF_HDMI_VIDEO_MODE_E enVidOutMode;
	MT_UNF_HDMI_DEEP_COLOR_E enDeepColorMode;
	MT_BOOL b3DEnable;
	mt_u8 u83DParam;
	MT_BOOL bHdcpEn;
	mt_u32 hdcp_mode;
	mt_u32 hdcp_sts;
	MT_BOOL bcecEn;
	mt_u8 std;
	MT_BOOL bLimitedRange;
	HDMI_AUDIO_ATTR_S aud;
} HDMI_UBOOT_AVINFO_T;

void SI_TX_PHY_INIT(void);
MT_S32 SI_TX_PHY_GetOutPutEnable(void);
MT_U32 SI_OpenHdmiDevice(void);
void SI_DisableHdmiDevice(void);
void SI_EnableHdmiDevice(void);
void SI_PoweDownHdmiDevice(void);
void SI_EnableInterrupts(void);
void SI_CloseHdmiDevice(void);
MT_S32 SI_GetHdmiSinkCaps(MT_UNF_EDID_BASE_INFO_S *pCapability);
MT_U32 SI_timer_count(void);
MT_U32 SI_RSEN_Status( void );
MT_S32 SI_TX_PHY_PowerDown(MT_BOOL bPwdown);
MT_U32 SI_HPD_Status( void );
void SiiPlatformDebugAssert( const char *pFileName, uint32_t lineNumber,
							 uint32_t expressionEvaluation, const char *pConditionText );
MT_U32 SI_HPD_SetHPDUserCallbackCount( void );
MT_U32 SI_Is_HPDKernelCallback_DetectHPD( void );
void SI_SendCP_Packet(MT_U8 bMuteAv);
void SI_SetEncryption(MT_U8 OnOff);
void SI_WriteByteEEPROM(MT_U8 RegAddr, MT_U8 RegData);
MT_U8 SI_GetAVIInfoFrameVID(void);
MT_VOID SI_timer_stop(void);
MT_S32 SI_TX_PHY_EnableHdmiOutput(void);
MT_U32 SI_CEC_Close(void);
MT_U32 SI_CEC_SendCommand(MT_UNF_HDMI_CEC_CMD_S  *pCECCmd);
mt_void mt_init_keep_out_win(mt_u32 win_max, mt_u32 win_min, mt_u8 pix4x, mt_u8 en);
mt_void drv_aud_hdmi_acr_cfg(MT_UNF_HDMI_ID_E enHdmi);
HDMI_TMDS_CLK_RATE_T get_hdmi_tmds_clk(MT_UNF_HDMI_ID_E enHdmi);
mt_void SI_CEC_SetUp(void);
MT_U32 SI_CEC_Open(void);
MT_U32 SI_CEC_AudioPing(MT_U32 *pu32Status);
void SI_Uboot2mainSmooth ( uint32_t smooth);
void SI_GetHdmiHalParams (HDMI_UBOOT_AVINFO_T *avinfo);
mt_void hdmi_soft_rst(mt_u32);
mt_void hdmi_rcfg_info(SiiInst_t inst);
mt_void SI_wait_hdcp_off(mt_void);
mt_void SI_hdcp_en(mt_u8 onOff, mt_u8 flg);
mt_void DRV_HDMI_Phy_Rst(mt_u8 rst);
mt_void DRV_HDMI_Phy_Drv_Cfg(mt_void);
void DRV_HDMI_Aip_Rst(mt_u32 rst);
void SI_GetHdmiHalDc(uint8_t *dc);
MT_U32 DRV_HDMI_Set_Csc_Mtx(mt_u8 idx, void* mtx);
volatile mt_void mt_SiiCecSetSourceActive(bool_t act);
#endif
