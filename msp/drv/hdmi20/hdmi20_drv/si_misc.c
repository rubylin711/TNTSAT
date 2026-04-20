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
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
//#include "si_globdefs.h"
//#include "si_cpldefs.h"
//#include "si_defstx.h"
//#include "si_datatypes.h"
#include "si_hdmitx.h"
#include "si_lib_time_api.h"
#include "si_misc.h"
#include "si_drv_tx_api.h"
#include "drv_global.h"
#ifdef CEC_SUPPORT
	#include "si_app_cec.h"
#endif
#include "mt_drv_analog.h"
#include "analog/mt_analog_parameter.h"
#include "analog/mt_analog_app.h"

extern SiiInst_t DRV_HDMI_Get_TxInst(void);
extern MT_BOOL get_current_rgb_mode(MT_UNF_HDMI_ID_E enHdmi);
extern void SI_SetBpssVidVal(uint8_t c0, uint8_t c1, uint8_t c2);

void SI_TX_PHY_INIT(void)
{
}

MT_S32 SI_TX_PHY_GetOutPutEnable(void)
{
	MT_S32 sRet = 1;
	#if (!defined(CONFIG_MT_FPGA))
	struct hdmiphy_param param;
	memset(&param, 0, sizeof(struct hdmiphy_param));
	mt_analog_get_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
	sRet = (param.mute==0);
	#endif
	return sRet;
}

MT_U32 SI_OpenHdmiDevice(void)
{
	return MT_SUCCESS;
}

MT_S32 SI_TX_PHY_DisableHdmiOutput(void)
{
	#if (!defined(CONFIG_MT_FPGA))
	//SiiDrvTxTmdsMute(DRV_HDMI_Get_TxInst(), 1);
	struct hdmiphy_param param;
	mt_analog_get_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
	param.mute = 1;
	mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
	#endif
	return 0;
}

void SI_DisableHdmiDevice(void)
{
	SI_TX_PHY_DisableHdmiOutput();
	return;
}

void SI_CloseHdmiDevice(void)
{
}

MT_S32 SI_TX_PHY_EnableHdmiOutput(void)
{
	#if (!defined(CONFIG_MT_FPGA))
	//SiiDrvTxTmdsMute(DRV_HDMI_Get_TxInst(), 0);
	struct hdmiphy_param param;
	mt_analog_get_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
	param.mute = 0;
	mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
	#endif

	return 1;
}

void SI_EnableHdmiDevice(void)
{
	SI_TX_PHY_EnableHdmiOutput();
	return;
}

void SI_PoweDownHdmiDevice(void)
{
	//SI_TX_PHY_PowerDown(MT_TRUE);  //0x1f157000
#if (!defined(CONFIG_MT_FPGA))
	//SiiDrvTxTmdsMute(DRV_HDMI_Get_TxInst(), 0);
	struct hdmiphy_param param;
	mt_analog_get_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
	param.mute = 1;
	mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
#endif
}

void SI_EnableInterrupts(void)
{
}

MT_S32 SI_GetHdmiSinkCaps(MT_UNF_EDID_BASE_INFO_S *pCapability)
{
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);

	if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		memcpy(pCapability, pSinkCap, sizeof(MT_UNF_EDID_BASE_INFO_S));
		return MT_SUCCESS;
	} else {
		return MT_FAILURE;
	}
}

MT_U32 SI_timer_count(void)
{
	return 1;
}

MT_U32 SI_RSEN_Status( void )
{
	if (DRV_Get_IsForceOutput()) {
		return MT_TRUE;
	} else {
		return (GetRsenStat()) ? MT_TRUE : MT_FALSE;
	}
}

MT_S32 SI_TX_PHY_PowerDown(MT_BOOL bPwdown)
{
	MT_S32 ret = 0;
	if ( bPwdown ) {
		ret = (MT_S32)mt_analog_disable(MT_ANA_HDMITX);
	} else {
		ret = (MT_S32)mt_analog_enable(MT_ANA_HDMITX);
	}
	return ret;
}

MT_U32 SI_HPD_Status( void )
{
	if (DRV_Get_IsForceOutput()) {
		return MT_TRUE;
	} else {
		return GetSysStat() ? MT_TRUE : MT_FALSE;
	}
}

void SiiPlatformDebugAssert( const char *pFileName, uint32_t lineNumber,
							 uint32_t expressionEvaluation, const char *pConditionText )
{
}

MT_U32 SI_HPD_SetHPDUserCallbackCount( void )
{
	return 0;
}

MT_U32 SI_Is_HPDKernelCallback_DetectHPD( void )
{
	return 0;
}

void SI_SendCP_Packet(MT_U8 bMuteAv)
{
	MT_U8 BlankValue[3];

	COM_INFO("MUTE=%u.\n", bMuteAv);

	if (get_current_rgb_mode(MT_UNF_HDMI_ID_0)) {
		BlankValue[0] = 0;
		BlankValue[1] = 0;
		BlankValue[2] = 0;
	} else {
		BlankValue[0] = 16;
		BlankValue[1] = 128;
		BlankValue[2] = 128;
	}

	/* also mute av in DVI mode  */
	if (SI_IsTXInHDMIMode()) {			// Send CP packets only if in HDMI mode
		SI_SetBpssVidVal((uint8_t)BlankValue[0], (uint8_t)BlankValue[1], (uint8_t)BlankValue[2]);
		if (bMuteAv) {
			SI_SetHdmiVideo(MT_FALSE);
		} else {
			SI_SetHdmiVideo(MT_TRUE);
		}
		SiiDrvTxAvMuteSet(DRV_HDMI_Get_TxInst(), bMuteAv);
	}
	#if defined (DVI_SUPPORT)
	else { /* DVI Mode */
		SI_SetBpssVidVal((uint8_t)BlankValue[0], (uint8_t)BlankValue[1], (uint8_t)BlankValue[2]);
		if (bMuteAv) {
			SI_SetHdmiVideo(MT_FALSE);
		} else {
			SI_SetHdmiVideo(MT_TRUE);
		}
		SiiDrvTxAvMuteSet(DRV_HDMI_Get_TxInst(), bMuteAv);
	}
	#endif
}

void SI_SetEncryption(MT_U8 OnOff)
{
	SiiDrvTxHdcpProtectionSet(DRV_HDMI_Get_TxInst(), (bool_t)OnOff);
}

void SI_WriteByteEEPROM(MT_U8 RegAddr, MT_U8 RegData)
{
	;
}

MT_VOID SI_timer_stop(void)
{
}

MT_U32 SI_Is_HPDUserCallback_DetectHPD( void )
{
	return MT_FALSE;
}

MT_U8 SI_GetAVIInfoFrameVID(void)
{
	MT_U8 u8Value;
	//Read VIC form AVI Inforframe
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, 0x00);
	SiiDrvCraBlockRead8((ulong)NULL, REG_ADDR__TPI_INFO_B7, (uint8_t*)(&u8Value), 1);
	return u8Value;
}

#ifdef CEC_SUPPORT
extern void SiiCecDisable(void);
extern void SiiCecEnable(void);
extern mt_void Si_CEC_Start(bool_t en);
extern void SI_CEC_Enum( bool_t en, uint8_t hpd);
mt_void SI_CEC_SetUp(void)
{
	;//SiiDrvCraWrReg8((ulong)NULL,REG_ADDR__CEC_CONFIG_CPI,BIT_MSK__CEC_CONFIG_CPI__CEC_REG_FORCE_NON_CALIB);
}

MT_U32 SI_CEC_AudioPing(MT_U32 *pu32Status)
{
	MT_U32 status, timeout = 0;
	MT_U32 Error = MT_SUCCESS;

	/* CEC Set Up Register */
	SiiDrvCraWrReg8((SiiInst_t)NULL, REG_ADDR__CEC_AUTO_DISCOVERY, BIT_MSK__CEC_AUTO_DISCOVERY__CEC_AUTO_PING_CLEAR);
	SiiDrvCraWrReg8((SiiInst_t)NULL, REG_ADDR__CEC_AUTO_DISCOVERY, BIT_MSK__CEC_AUTO_DISCOVERY__CEC_AUTO_PING_START);
	while (timeout ++ < 10) {
		SiiLibTimeMilliDelay(100);
		status = SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__CEC_AUTO_DISCOVERY);
		if (0x80 == (status & 0x80)) {
			CEC_INFO("REG__CEC_AUTO_PING_CTRL:0x%x\n", status);
			break;
		}
	}

	if (timeout >= 100) {
		CEC_ERR("AutoPing timeout\n");
		return MT_FAILURE;
	}

	status = SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__CEC_AUTODISC_MAP0);

	//CEC_INFO("AUTO_PING_MAP0:0x%x\n",status);
	status = SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__CEC_AUTODISC_MAP1) + status; //0xcc 0xe2

	CEC_INFO("CEC Auto Ping Result:0x%x\n", status);
	SiiDrvCraWrReg8((SiiInst_t)NULL, REG_ADDR__CEC_AUTO_DISCOVERY, 0x00);

	*pu32Status = status;

	return Error;
}

mt_void mt_SiiCecSetSourceActive(bool_t act)
{
	extern uint8_t GetSourceRealPowerSts(void);
	if ( GetSourceRealPowerSts() == 0 ) {
		SiiCecSetSourceActive( act );
	}
}

MT_U32 SI_CEC_Open(void)
{
	MT_U32 ret;
	SiiCecPowerstatus_t sts;
	if (DRV_Get_IsCECEnable(MT_UNF_HDMI_ID_0)) {
		sts = SiiCecGetPowerState();
		SiiCecEnable();
		//if ( sts == CEC_POWERSTATUS_ON ) {
		//	mt_SiiCecSetSourceActive( true );
		//}
		SiiCecUpdatePowerState(sts);
		ret = MT_SUCCESS;
	} else {
		ret = MT_FAILURE;
	}
	return ret;
}

MT_U32 SI_CEC_Close(void)
{
	MT_U32 ret;
	SI_CEC_Enum((bool_t)0, 1);
	SiiCecDisable();
	Si_CEC_Start(0);
	DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_FALSE);
	ret = MT_SUCCESS;
	return ret;
}

extern bool_t SiiCecSendMsgAck ( SiI_CEC_t *cec_frame );
MT_U32 SI_CEC_SendCommand(MT_UNF_HDMI_CEC_CMD_S  *pCECCmd)
{
	SiI_CEC_t cec_frame;
	bool_t ret;

	memset(&cec_frame, 0, sizeof(SiI_CEC_t));

	cec_frame.bOpcode         = pCECCmd->u8Opcode;
	cec_frame.bDestOrRXHeader = pCECCmd->enDstAdd;
	if ( pCECCmd->unOperand.stRawData.u8Length > sizeof(pCECCmd->unOperand.stRawData.u8Data)) {
		HDMI20_MISC_PRINTK("\nerror operand len:%d\n", pCECCmd->unOperand.stRawData.u8Length);
		return MT_FAILURE;
	}
	cec_frame.bCount          = pCECCmd->unOperand.stRawData.u8Length;
	memcpy((cec_frame.bOperand), (pCECCmd->unOperand.stRawData.u8Data), pCECCmd->unOperand.stRawData.u8Length);
	#if 0
	{
		int i;
		if ( cec_frame.bCount ) {
			HDMI20_MISC_PRINTK("\noperand list:");
		}
		for (i = 0; i < cec_frame.bCount; i++) {
			HDMI20_MISC_PRINTK("0x%x ", cec_frame.bOperand[i]);
		}
		if ( cec_frame.bCount ) {
			HDMI20_MISC_PRINTK("\n");
		}
	}
	#endif

	ret = SiiCecSendMsgAck( &cec_frame );
	if ( ret == true ) {
		ret = MT_SUCCESS;
	} else {
		ret = MT_FAILURE;
	}

	return (MT_U32)ret;
}
#endif

mt_void mt_init_keep_out_win(mt_u32 win_max, mt_u32 win_min, mt_u8 pix4x, mt_u8 en)
{
	/*+++++ CONFIG KEEP OUT WINDOW ++++++*/
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__KEEP_OUT_WIN_SIZE_0_MT, BIT_MSK__KEEP_OUT_WIN_MAX_B12_B8, (mt_u8)((win_max >> 5)&BIT_MSK__KEEP_OUT_WIN_MAX_B12_B8));
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__KEEP_OUT_WIN_SIZE_0_MT, BIT_MSK__KEEP_OUT_WIN_MIN_B8, (mt_u8)((win_min >> 6)&BIT_MSK__KEEP_OUT_WIN_MIN_B8));
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__KEEP_OUT_WIN_CTRL_MT, BIT_MSK__PIX_X4_MODE, (mt_u8)((pix4x << 7)&BIT_MSK__PIX_X4_MODE));
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__KEEP_OUT_WIN_CTRL_MT, BIT_MSK__KEEP_OUT_WIN_EN_REQ_SEL, (mt_u8)((en << 6)&BIT_MSK__KEEP_OUT_WIN_EN_REQ_SEL));
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__KEEP_OUT_WIN_MAX_LOW_MT, BIT_MSK__KEEP_OUT_WIN_MAX_B7_B0, (mt_u8)(win_max & BIT_MSK__KEEP_OUT_WIN_MAX_B7_B0));
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__KEEP_OUT_WIN_MIN_LOW_MT, BIT_MSK__KEEP_OUT_WIN_MIN_B7_B0, (mt_u8)(win_min & BIT_MSK__KEEP_OUT_WIN_MIN_B7_B0));
	/*------- CONFIG KEEP OUT WINDOW --------*/
}

HDMI_TMDS_CLK_RATE_T get_hdmi_tmds_clk(MT_UNF_HDMI_ID_E enHdmi)
{
	HDMI_TMDS_CLK_RATE_T tmds_clk;
	HDMI_APP_ATTRMT_S     *pstAppAttrMt = DRV_Get_AppAttrMt(enHdmi);
	mt_u32 clk1p001 = 0;

	switch ( pstAppAttrMt->tmds_clk ) {
		case 594000000:
			tmds_clk = HDMI_TMDS_CLK_594MHZ_1X;
			break;
		case 445500000:
			tmds_clk = HDMI_TMDS_CLK_297MHZ_1D5X;
			break;
		case 371250000:
			tmds_clk = HDMI_TMDS_CLK_297MHZ_1D25X;
			break;
		case 297000000:
			tmds_clk = HDMI_TMDS_CLK_297MHZ_1X;
			break;
		case 222750000:
			tmds_clk = HDMI_TMDS_CLK_148MHZ_1D5X;
			break;
		case 185625000:
			tmds_clk = HDMI_TMDS_CLK_148MHZ_1D25X;
			break;
		case 148500000:
			tmds_clk = HDMI_TMDS_CLK_148MHZ_1X;
			break;
		case 111375000:
			tmds_clk = HDMI_TMDS_CLK_74MHZ_1D5X;
			break;
		case 92812500:
			tmds_clk = HDMI_TMDS_CLK_74MHZ_1D25X;
			break;
		case 74250000:
			tmds_clk = HDMI_TMDS_CLK_74MHZ_1X;
			break;
		case 40500000:
			tmds_clk = HDMI_TMDS_CLK_27MHZ_1D5X;
			break;
		case 33750000:
			tmds_clk = HDMI_TMDS_CLK_27MHZ_1D25X;
			break;
		case 27000000:
			tmds_clk = HDMI_TMDS_CLK_27MHZ_1X;
			break;
		default:
			tmds_clk = HDMI_TMDS_CLK_74MHZ_1X;
			break;
	}
	if ( clk1p001 ) {
		tmds_clk += HDMI_TMDS_CLK_594MHZ_1D5X - HDMI_TMDS_CLK_IDLE;
	}
	return tmds_clk;
}

mt_void drv_aud_hdmi_acr_cfg(MT_UNF_HDMI_ID_E enHdmi)
{
	HDMI_AUDIO_ATTR_S *pstAudAttr = DRV_Get_AudioAttr(enHdmi);
	mt_u32 acr_n = 6144;
	HDMI_TMDS_CLK_RATE_T tmds_clk;

	/*++++ TMDS cr <= 340Mcsc , CTS=(fTmds_clock*N)/(128*fs) ++++*/
	/*++++ TMDS cr > 340Mcsc , CTS=(4*fTmds_clock*N)/(128*fs) ++++*/
	tmds_clk = get_hdmi_tmds_clk(enHdmi);
	switch ( pstAudAttr->enSampleRate ) {
		case MT_UNF_SAMPLE_RATE_32K:
		case MT_UNF_SAMPLE_RATE_64K:
		case MT_UNF_SAMPLE_RATE_128K:
			switch (tmds_clk) {
				case HDMI_TMDS_CLK_27MHZ_1X:
				case HDMI_TMDS_CLK_27_1P001MHZ_1X:
					acr_n = 4096;//cts=27000/27027
					break;
				case HDMI_TMDS_CLK_74MHZ_1X:
					acr_n = 4096;//cts=74250
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1X:
					acr_n = 11648;//cts=210937~210938
					break;
				case HDMI_TMDS_CLK_148MHZ_1X:
					acr_n = 4096;//cts=148500
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1X:
					acr_n = 11648;//cts=421875
					break;
				case HDMI_TMDS_CLK_297MHZ_1X:
					acr_n = 3072;//cts=222750
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1X:
					acr_n = 5824;//cts=421875
					break;
				case HDMI_TMDS_CLK_594MHZ_1X:
					acr_n = 3072;//cts=445500
					break;
				case HDMI_TMDS_CLK_594_1P001MHZ_1X:
					acr_n = 5824;//cts=843750
					break;
				case HDMI_TMDS_CLK_27MHZ_1D25X:
					acr_n = 4096;//cts=33750
					break;
				case HDMI_TMDS_CLK_27_1P001MHZ_1D25X:
					acr_n = 8192;//cts=67567-67568
					break;
				case HDMI_TMDS_CLK_74MHZ_1D25X:
					acr_n = 8192;//cts=185625
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1D25X:
					acr_n = 11648;//cts=263671-263672
					break;
				case HDMI_TMDS_CLK_148MHZ_1D25X:
					acr_n = 4096;//cts=185625
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1D25X:
					acr_n = 11648;//cts=527343-527344
					break;
				case HDMI_TMDS_CLK_297MHZ_1D25X:
					acr_n = 6144;//cts=556875
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1D25X:
					acr_n = 5824;//cts=527343-527344
					break;
				case HDMI_TMDS_CLK_27MHZ_1D5X:
					acr_n = 4096;//cts=40500
					break;
				case HDMI_TMDS_CLK_27_1P001MHZ_1D5X:
					acr_n = 8192;//cts=81081
					break;
				case HDMI_TMDS_CLK_74MHZ_1D5X:
					acr_n = 4096;//cts=111375
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1D5X:
					acr_n = 11648;//cts=316406-316407
					break;
				case HDMI_TMDS_CLK_148MHZ_1D5X:
					acr_n = 4096;//cts=222750
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1D5X:
					acr_n = 11648;//cts=316406-316407
					break;
				case HDMI_TMDS_CLK_297MHZ_1D5X:
					acr_n = 4096;//cts=445500
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1D5X:
					acr_n = 5824;//cts=632812-632813
					break;
				default:
					acr_n = 4096;//cts=74250
					break;
			}
			break;
		case MT_UNF_SAMPLE_RATE_48K://48K
		case MT_UNF_SAMPLE_RATE_96K://48K
		case MT_UNF_SAMPLE_RATE_192K://48K
			switch (tmds_clk) {
				case HDMI_TMDS_CLK_27MHZ_1X:
				case HDMI_TMDS_CLK_27_1P001MHZ_1X:
					acr_n = 6144;//cts=27000/27027
					break;
				case HDMI_TMDS_CLK_74MHZ_1X:
					acr_n = 6144;//cts=74250
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1X:
					acr_n = 11648;//cts=140625
					break;
				case HDMI_TMDS_CLK_148MHZ_1X:
					acr_n = 6144;//cts=148500
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1X:
					acr_n = 5824;//cts=140625
					break;
				case HDMI_TMDS_CLK_297MHZ_1X:
					acr_n = 5120;//cts=222750
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1X:
					acr_n = 5824;//cts=281250
					break;
				case HDMI_TMDS_CLK_594MHZ_1X:
					acr_n = 6144;//cts=445500
					break;
				case HDMI_TMDS_CLK_594_1P001MHZ_1X:
					acr_n = 5824;//cts=562500
					break;
				case HDMI_TMDS_CLK_27MHZ_1D25X:
					acr_n = 6144;//cts=33750
					break;
				case HDMI_TMDS_CLK_27_1P001MHZ_1D25X:
					acr_n = 8192;//cts=45045
					break;
				case HDMI_TMDS_CLK_74MHZ_1D25X:
					acr_n = 12288;//cts=185625
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1D25X:
					acr_n = 11648;//cts=175781-175782
					break;
				case HDMI_TMDS_CLK_148MHZ_1D25X:
					acr_n = 6144;//cts=185625
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1D25X:
					acr_n = 11648;//cts=351562-351563
					break;
				case HDMI_TMDS_CLK_297MHZ_1D25X:
					acr_n = 5120;//cts=309375
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1D25X:
					acr_n = 11648;//cts=703125
					break;
				case HDMI_TMDS_CLK_27MHZ_1D5X:
					acr_n = 6144;//cts=40500
					break;
				case HDMI_TMDS_CLK_27_1P001MHZ_1D5X:
					acr_n = 8192;//cts=54054
					break;
				case HDMI_TMDS_CLK_74MHZ_1D5X:
					acr_n = 6144;//cts=111375
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1D5X:
					acr_n = 11648;//cts=210937-210938
					break;
				case HDMI_TMDS_CLK_148MHZ_1D5X:
					acr_n = 6144;//cts=222750
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1D5X:
					acr_n = 11648;//cts=421875
					break;
				case HDMI_TMDS_CLK_297MHZ_1D5X:
					acr_n = 5120;//cts=445500
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1D5X:
					acr_n = 5824;//cts=421875
					break;
				default:
					acr_n = 6144;//cts=74250
					break;
			}
			break;
		case MT_UNF_SAMPLE_RATE_44K://44.1K
		case MT_UNF_SAMPLE_RATE_88K://44.1K
		case MT_UNF_SAMPLE_RATE_176K://44.1K
			switch (tmds_clk) {
				case HDMI_TMDS_CLK_27MHZ_1X:
				case HDMI_TMDS_CLK_27_1P001MHZ_1X:
					acr_n = 6272;//cts=30000/30030
					break;
				case HDMI_TMDS_CLK_74MHZ_1X:
					acr_n = 6272;//cts=82500
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1X:
					acr_n = 17836;//cts=234375
					break;
				case HDMI_TMDS_CLK_148MHZ_1X:
					acr_n = 6272;//cts=165000
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1X:
					acr_n = 8918;//cts=234375
					break;
				case HDMI_TMDS_CLK_297MHZ_1X:
					acr_n = 4704;//cts=247500
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1X:
					acr_n = 4459;//cts=234375
					break;
				case HDMI_TMDS_CLK_594MHZ_1X:
					acr_n = 9408;//cts=990000
					break;
				case HDMI_TMDS_CLK_594_1P001MHZ_1X:
					acr_n = 8918;//cts=937500
					break;
				case HDMI_TMDS_CLK_27MHZ_1D25X:
					acr_n = 6272;//cts=37500
					break;
				case HDMI_TMDS_CLK_27_1P001MHZ_1D25X:
					acr_n = 12544;//cts=75075
					break;
				case HDMI_TMDS_CLK_74MHZ_1D25X:
					acr_n = 6272;//cts=103125
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1D25X:
					acr_n = 17836;//cts=292968-292969
					break;
				case HDMI_TMDS_CLK_148MHZ_1D25X:
					acr_n = 6272;//cts=206250
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1D25X:
					acr_n = 17836;//cts=585937-585938
					break;
				case HDMI_TMDS_CLK_297MHZ_1D25X:
					acr_n = 4704;//cts=309375
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1D25X:
					acr_n = 8918;//cts=585937-585938
					break;
				case HDMI_TMDS_CLK_27MHZ_1D5X:
				case HDMI_TMDS_CLK_27_1P001MHZ_1D5X:
					acr_n = 6272;//cts=45000
					break;
				case HDMI_TMDS_CLK_74MHZ_1D5X:
					acr_n = 6272;//cts=123750
					break;
				case HDMI_TMDS_CLK_74_1P001MHZ_1D5X:
					acr_n = 17836;//cts=351562-351563
					break;
				case HDMI_TMDS_CLK_148MHZ_1D5X:
					acr_n = 6272;//cts=247500
					break;
				case HDMI_TMDS_CLK_148_1P001MHZ_1D5X:
					acr_n = 17836;//cts=703125
					break;
				case HDMI_TMDS_CLK_297MHZ_1D5X:
					acr_n = 4704;//cts=371250
					break;
				case HDMI_TMDS_CLK_297_1P001MHZ_1D5X:
					acr_n = 8918;//cts=703125
					break;
				default:
					acr_n = 6272;//cts=74250
					break;
			}
			break;
		default:
			break;
	}

	switch ( pstAudAttr->enSampleRate ) {
		case MT_UNF_SAMPLE_RATE_64K:
		case MT_UNF_SAMPLE_RATE_96K:
		case MT_UNF_SAMPLE_RATE_88K:
			acr_n <<= 1;
			break;
		case MT_UNF_SAMPLE_RATE_128K:
		case MT_UNF_SAMPLE_RATE_192K:
		case MT_UNF_SAMPLE_RATE_176K:
			acr_n <<= 2;
			break;
		default:
			break;
	}
	if ( pstAudAttr->enSoundIntf == HDMI_AUDIO_INTERFACE_HBR ) {
		//acr_n <<= 2;
	}
	HDMI20_MISC_PRINTK("\nacr: tmds:%d,fs=%d,N=0x%x\n", tmds_clk, pstAudAttr->enSampleRate, acr_n);
	SiiDrvCraClrBit8((ulong)NULL, REG_ADDR__TPI_DOWN_SMPL_CTRL, BIT_MSK__TPI_DOWN_SMPL_CTRL__REG_TPI_AUDIO_LOOKUP_EN);
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__N_SVAL1, BIT_MSK__N_SVAL1__REG_N_VAL_SW1, (uint8_t)(acr_n & 0xff));
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__N_SVAL2, BIT_MSK__N_SVAL2__REG_N_VAL_SW2, (uint8_t)((acr_n >> 8) & 0xff));
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__N_SVAL3, BIT_MSK__N_SVAL3__REG_N_VAL_SW3, (uint8_t)((acr_n >> 16) & 0x0f));
}

SiiTmdsMode_t SI_TX_HDMImode(void)
{
	uint8_t u8Value = 0;
	uint8_t u8Value1 = 0;
	SiiTmdsMode_t ret;
	uint8_t tmdsOn = 1;

	tmdsOn = SI_TX_PHY_GetOutPutEnable();
	u8Value = SiiDrvCraRdReg8((ulong)NULL, REG_ADDR__TPI_SC);
	u8Value1 = SiiDrvCraRdReg8((ulong)NULL, REG_ADDR__SCRCTL);

	if ( (u8Value & BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B0) == 0) {
		ret = SII_TMDS_MODE__DVI;
	} else if ( (u8Value1 & BIT_MSK__SCRCTL__REG_HDMI2_ON) == 0) {
		ret = SII_TMDS_MODE__HDMI1;
	} else {
		ret = SII_TMDS_MODE__HDMI2;
	}

	if ( tmdsOn == 0 ) {
		ret = SII_TMDS_MODE__NONE;
	}
	return ret;
}

void SI_GetVSInfoFrame3DInfo(MT_BOOL *b3dEn, mt_u8 *u83DParam)
{
	MT_U8 u8Value[9];
	//Read 3D info from VSD Inforframe
	SiiDrvCraPutBit8((ulong)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, 0x05);
	SiiDrvCraBlockRead8((ulong)NULL, REG_ADDR__TPI_INFO_B0, (uint8_t*)(&u8Value), 9);
	if (u8Value[0] == 0x81 && u8Value[1] == 0x01 && u8Value[4] == 0x03) {
		if ( (u8Value[7] & 0xe0) == 0x40 ) {
			*b3dEn = TRUE;
			*u83DParam = (u8Value[8] & 0xF0) >> 4;
		}
	}
}

mt_u8 SI_HalGetHdcpInfo(void)
{
	mt_u8 reghdcp1x;
	mt_u8 reghdcp2x;
	mt_u8 hdcp_mode = 0;
	reghdcp1x = SiiDrvCraRdReg8((ulong)NULL, REG_ADDR__TPI_COPP_DATA2);
	reghdcp2x = SiiDrvCraRdReg8((ulong)NULL, REG_ADDR__HDCP2X_CTL_0);
	if ((reghdcp1x & 0x17) == 0x17) {
		hdcp_mode = 1;
	} else if ( (reghdcp2x & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN) == BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN ) {
		hdcp_mode = 2;
	}
	return hdcp_mode;
}

void SI_GetHdmiHalCs(uint8_t *cs, uint8_t *std, uint8_t *pc)
{
	uint16_t cms0 = 0;
	uint16_t cms1 = 0;
	uint16_t cs_fmt = 0;
	uint8_t cms_csc1_444_422 = 0;
	uint8_t cms_csc1_422_420 = 0;

	cms0 = (uint16_t)SiiDrvCraRdReg8((SiiInst_t)NULL, 0xD20);
	cms0 |= ((uint16_t)SiiDrvCraRdReg8((SiiInst_t)NULL, 0xD20 + 1)) << 8;

	cms1 = (uint16_t)SiiDrvCraRdReg8((SiiInst_t)NULL, 0xDA0);
	cms1 |= ((uint16_t)SiiDrvCraRdReg8((SiiInst_t)NULL, 0xDA0 + 1)) << 8;
	cs_fmt = (uint16_t)SiiDrvCraRdReg8((SiiInst_t)NULL, 0xB48);
	cs_fmt |= ((uint16_t)SiiDrvCraRdReg8((SiiInst_t)NULL, 0xB48 + 1)) << 8;

	cms_csc1_444_422 = SiiDrvCraRdReg8((SiiInst_t)NULL, 0xDC0);
	cms_csc1_422_420 = SiiDrvCraRdReg8((SiiInst_t)NULL, 0xDC2);

	if ( cms1 & 0x03 ) {
		if ( cms1 & 0x20 ) {
			*cs = 0;
		} else {
			if ( cs_fmt & 0x400 ) {
				*cs = 3;
			} else if ( (cms_csc1_444_422 & 0x1) && (cms_csc1_422_420 & 0x1) == 0) {
				*cs = 1;
			} else if ( (cms_csc1_444_422 & 0x1) == 0 && (cms_csc1_422_420 & 0x1) == 0) {
				*cs = 2;
			} else {
				*cs = 3;
			}
		}
		*std = (cms1 & 0x0C) >> 2;
		*pc = (cms1 & 0x10) >> 4;
	} else if ( cms0 & 0x03 ) {
		if ( cms0 & 0x20 ) {
			*cs = 0;
		} else {
			if ( cs_fmt & 0x400 ) {
				*cs = 3;
			} else if ( (cms_csc1_444_422 & 0x1) && (cms_csc1_422_420 & 0x1) == 0) {
				*cs = 1;
			} else if ( (cms_csc1_444_422 & 0x1) == 0 && (cms_csc1_422_420 & 0x1) == 0) {
				*cs = 2;
			} else {
				*cs = 3;
			}
		}
		*std = (cms0 & 0x0C) >> 2;
		*pc = (cms0 & 0x10) >> 4;
	} else {
		//*cs = 2;  //bypass
		if ( cs_fmt & 0x400 ) {
			*cs = 3;
		} else if ( (cms_csc1_444_422 & 0x1) && (cms_csc1_422_420 & 0x1) == 0) {
			*cs = 1;
		} else if ( (cms_csc1_444_422 & 0x1) == 0 && (cms_csc1_422_420 & 0x1) == 0) {
			*cs = 2;
		} else {
			*cs = 3;
		}
		*std = (cms0 & 0x0C) >> 2;
		*pc = (cms0 & 0x10) >> 4;
	}
}

void SI_GetHdmiHalDc(uint8_t *dc)
{
	uint8_t p2t = 0;
	*dc = MT_UNF_HDMI_DEEP_COLOR_24BIT;

	p2t = (uint16_t)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__P2T_CTRL);
	if ( p2t & BIT_MSK__P2T_CTRL__REG_DC_PKT_EN) {
		switch ( p2t & BIT_MSK__P2T_CTRL__REG_PACK_MODE ) {
			case 1:
				*dc = MT_UNF_HDMI_DEEP_COLOR_30BIT;
				break;
			case 2:
				*dc = MT_UNF_HDMI_DEEP_COLOR_36BIT;
				break;
			case 3:
				*dc = MT_UNF_HDMI_DEEP_COLOR_48BIT;
				break;
			default:
				*dc = MT_UNF_HDMI_DEEP_COLOR_24BIT;
				break;
		}
	}
}

void SI_GetHdmiHalParams (HDMI_UBOOT_AVINFO_T *avinfo)
{
	uint8_t cs = 0xff;
	uint8_t std = 0xff;
	uint8_t pc = 0xff;
	uint8_t dc = 0xff;

	//Get uboot hdmi Params
	//HDMI (DVI/HDMI1/HDMI2?)
	avinfo->hdmi_mode = SI_TX_HDMImode();
	//TV format (format,3D..VIC?)
	avinfo->vic = SI_GetAVIInfoFrameVID();
	SI_GetVSInfoFrame3DInfo(&(avinfo->b3DEnable), &(avinfo->u83DParam));

	//Color Space (RGB/YUV422/YUV444/YUV420 Full/Limited?)
	SI_GetHdmiHalCs(&cs, &std, &pc);
	avinfo->enVidOutMode = (MT_UNF_HDMI_VIDEO_MODE_E)cs;
	if ( cs == 0 ) {
		avinfo->bLimitedRange = 0;
	} else {
		avinfo->bLimitedRange = (pc == 0);
	}

	//Colorimetry (BT601/BT709/BT2020/BT2100?)
	avinfo->std = std;

	//Deep Color (8/10/12bits? HDR/HLG/HDR10+?)
	SI_GetHdmiHalDc(&dc);
	avinfo->enDeepColorMode = (MT_UNF_HDMI_DEEP_COLOR_E)dc;

	//Audio ( Sample Rate/Channel/Mode...?)
	avinfo->aud.u32Channels = 2;
	avinfo->aud.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
	avinfo->aud.enBitDepth = 16;

	//HDCP (HDCP1x/HDCP2x?)(Authenticated Success?)
	avinfo->bHdcpEn = (SI_HalGetHdcpInfo() != 0);
	avinfo->hdcp_mode = SI_HalGetHdcpInfo();

	//CEC (Open/Start?)
	avinfo->bcecEn = 0;// From boot to main, init it to 0;
}

void SI_Uboot2mainSmooth ( uint32_t smooth)
{
	HDMI_UBOOT_AVINFO_T avinfo = {0};
	SI_GetHdmiHalParams(&avinfo);

	HDMI20_MISC_PRINTK("hdmi:%d,vic:%d,fmt:%d,vid[%d %d %d %d],3d[%d,%d],hdcp[%d,%d,%d],cec:%d,aud[%d,%d,%d,%d]\n", \
					   avinfo.hdmi_mode, avinfo.vic, avinfo.enFmt, \
					   avinfo.enVidOutMode, avinfo.enDeepColorMode, avinfo.bLimitedRange, avinfo.std, avinfo.b3DEnable, avinfo.u83DParam, \
					   avinfo.bHdcpEn, avinfo.hdcp_mode, avinfo.hdcp_sts, \
					   avinfo.bcecEn,
					   avinfo.aud.enSampleRate, avinfo.aud.u32Channels, avinfo.aud.enBitDepth, avinfo.aud.enAudioCode);
}

mt_void hdmi_soft_rst(mt_u32 rst)
{
	SiiDrvCraAddr_t reg_addr = 0x10;

	if ( rst ) {
		SiiDrvCraSetBit8((SiiInst_t)NULL, reg_addr, BIT_MSK__PWD_SRST__REG_SW_RST);
	} else {
		SiiLibTimeMilliDelay(1);
		SiiDrvCraClrBit8((SiiInst_t)NULL, reg_addr, BIT_MSK__PWD_SRST__REG_SW_RST);
		SiiLibTimeMilliDelay(2);
	}
	//printf("\n soft reset done!!!\n");
}

mt_void hdmi_rcfg_info(SiiInst_t inst)
{
	#define HDMI20_INFOFRAME_CH_MAX (9)
	mt_u32 j;
	uint8_t infoOn[HDMI20_INFOFRAME_CH_MAX];

	SiiDrvTxInfoframeOnOffGet(inst,infoOn);
	for (j=0;j<HDMI20_INFOFRAME_CH_MAX;j++) {
		SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL,j);
		if ( infoOn[j]) {
			SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_EN, BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_EN | BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_RPT);
		} else {
			SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_EN, BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_RPT);
			SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_EN, BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_EN);
		}
	}
}

mt_void SI_wait_hdcp_off(mt_void)
{
	mt_u8 hdcp1x_ctrl = 0;
	mt_u8 hdcp2x_ctrl0 = 0;
	mt_u8 hdcp2x_ctrl1 = 0;
	mt_s32 cnt = 0;
	SiiDrvHdcpStatus_t sts = SII_DRV_HDCP_STATUS__OFF;

	SiiDrvTxHdcpStateStatusGet(DRV_HDMI_Get_TxInst(), &sts);
	if ( 1 ) {//( sts != SII_DRV_HDCP_STATUS__OFF ) {
		do {
			hdcp1x_ctrl = SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INTR_EN);
			hdcp2x_ctrl0 = SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__HDCP2X_INTR0_MASK);
			hdcp2x_ctrl1 = SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__HDCP2X_INTR1_MASK);
			SiiLibTimeMilliDelay(10);
			cnt++;
		} while ((hdcp1x_ctrl || hdcp2x_ctrl0 || hdcp2x_ctrl1 || SiiDrvTxHdcpSetProtectionDone(DRV_HDMI_Get_TxInst()) != 1) && cnt < 200);
	}
	HDMI20_MISC_PRINTK("\nhdcp disabled time :%d(ms), 0x%x,0x%x, 0x%x\n", cnt*10, hdcp1x_ctrl,hdcp2x_ctrl0,hdcp2x_ctrl1);
}

mt_void SI_hdcp_en(mt_u8 onOff, mt_u8 flg)
{
	SiiDrvHdcpStatus_t sts = SII_DRV_HDCP_STATUS__OFF;
	SiiDrvTxHdcpStateStatusGet(DRV_HDMI_Get_TxInst(), &sts);
	if ( onOff ) {
		if ( flg ) {
			SI_wait_hdcp_off();
		}
		SiiDrvTxHdcpStateStatusGet(DRV_HDMI_Get_TxInst(), &sts);
		if ( sts == SII_DRV_HDCP_STATUS__SUCCESS_1X || sts == SII_DRV_HDCP_STATUS__SUCCESS_22 ) {
			DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_FALSE);
			HDMI20_MISC_PRINTK("\nhdcp sts:%d\n", sts);
		} else {
			SiiDrvTxHdcpProtectionSet(DRV_HDMI_Get_TxInst(), 1);
		}
	} else {
		SiiDrvTxHdcpProtectionSet(DRV_HDMI_Get_TxInst(), 0);
		if ( flg ) {
			SI_wait_hdcp_off();
		}
	}
}

mt_void DRV_HDMI_Phy_Rst(mt_u8 rst)
{
	if ( rst ) {
		mt_analog_reset(MT_ANA_HDMITX);
		SiiLibTimeMilliDelay(2);
	} else {
		mt_analog_release(MT_ANA_HDMITX);
		//SiiLibTimeMilliDelay(1);
	}
}

mt_void DRV_HDMI_Phy_Drv_Cfg(mt_void)
{
	HDMI_APP_ATTRMT_S     *pstAppAttrMt = DRV_Get_AppAttrMt(MT_UNF_HDMI_ID_0);
	enum MT_HDMI_ANALOG_CFG_E cfg_idx = HDMI_ANALOG_CFG_MAX;
	cfg_idx = TMDS_TO_IDX(pstAppAttrMt->tmds_clk, pstAppAttrMt->os_clk);
	(void)mt_hdmi_analog_config_post(MT_UNF_HDMI_ID_0, cfg_idx);
}

void DRV_HDMI_Aip_Rst(mt_u32 rst)
{
	if ( rst ) {
		SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__AIP_RST, BIT_MSK__AIP_RST__REG_RST4AUDIO_FIFO | BIT_MSK__AIP_RST__REG_RST4AUDIO);
	} else {
		SiiLibTimeMilliDelay(1);
		SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__AIP_RST, BIT_MSK__AIP_RST__REG_RST4AUDIO_FIFO | BIT_MSK__AIP_RST__REG_RST4AUDIO);
	}
}

MT_U32 DRV_HDMI_Set_Csc_Mtx(mt_u8 idx, void* mtx)
{
	MT_U32 ret = MT_SUCCESS;
	if ( DRV_HDMI_Get_TxInst() ) {
		switch (idx) {
			case 0:
				SiiDrvTxOutputCscMtxSet(DRV_HDMI_Get_TxInst(), mtx);
				break;
			default:
				break;
		}
	} else {
		ret = MT_FAILURE;
	}
	return ret;
}

