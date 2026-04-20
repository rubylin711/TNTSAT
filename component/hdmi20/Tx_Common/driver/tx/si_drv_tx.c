/******************************************************************************
*
* Copyright 2013, Deco, Inc.  All rights reserved.
* No part of this work may be reproduced, modified, distributed, transmitted,
* transcribed, or translated into any language or computer format, in any form
* or by any means without written permission of
* Deco, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
*
*****************************************************************************/
/**
* @file si_drv_tx.c
*
* @brief Tx API
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "platform_api.h"
#include "sii_time.h"

#include "si_drv_tx_api.h"
#include "si_lib_log_api.h"
#include "si_lib_malloc_api.h"
#include "si_lib_seq_api.h"
#include "si_lib_time_api.h"
#include "si_drv_cra_api.h"
#include "si_mod_tx_scdc_api.h"
#include "si_drv_tx_regs.h"

/***** Register Module name **************************************************/
SII_LIB_OBJ_MODULE_DEF(drv_tx);

/***** local type definitions ************************************************/
#define TIMER_START__TX_INTR                150
#define TIMER_START__TX_INTR_PRI            255
#define TIMER_START__TX_INTR_REPEAT         100

typedef struct {
	SiiDrvTxConfig_t		*pConfig;
	SiiModTxVidPathEvent_t    vidPath_events;
	SiiInst_t               instIntrHandler;

	SiiDrvTxEvent_t         tx_events;
	TxcbFunc				txEventNotifyCbFunc;

	SiiDrvHdcpStatus_t		hdcpStatus;
	SiiInst_t				instHdmiTx;
	SiiInst_t				instHdcp;
	SiiInst_t				instVideoPath;

	SiiModTxHdmiConfig_t	hdmiConfig;
	SiiModTxHdcpConfig_t		hdcpConfig;
	SiiModTxVideoPathCfg_t	vidPathConfig;

	bool_t isHdmiConnected;
	bool_t isHpdForced;
} TxObj_t;

/***** local prototypes ******************************************************/
void SiiDrvTxHdmiModCallBack(SiiInst_t instTx, SiiDrvTxEvent_t eventFlags);
void SiiDrvTxHdcpModCallBack(SiiInst_t instTx, SiiDrvTxEvent_t eventFlags);

static void				sTxGroupInterruptHandler(SiiInst_t inst);
static SiiDrvCraAddr_t	sBaseAddrGet(TxObj_t *pObj);
static SiiInst_t		sCraInstGet(TxObj_t* pObj);
static void				sTxLog(uint8_t *pData, uint16_t len);

#if (__HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__)
	#include "mt4si/linux_k/si_drv_tx_mt.h"
#endif

/***** public functions ******************************************************/
//-------------------------------------------------------------------------------------------------
//! @brief      Initialize TX module
//-------------------------------------------------------------------------------------------------
SiiInst_t SiiDrvTxCreate(char *pNameStr, SiiDrvTxConfig_t *pConfig)
{
	TxObj_t *pObj = NULL;
	SiiDrvCraAddr_t         baseAddr = 0;
	SiiInst_t				instCra;

	/* Allocate memory for object */
	pObj = (TxObj_t*)SII_LIB_OBJ_CREATE(pNameStr, sizeof(TxObj_t));
	SII_PLATFORM_DEBUG_ASSERT(pObj);

	pObj->pConfig = (SiiDrvTxConfig_t*)SiiLibMallocCreate(sizeof(SiiDrvTxConfig_t));
	if ( pObj->pConfig ) {
		memcpy(pObj->pConfig, pConfig, sizeof(SiiDrvTxConfig_t));
	} else {
		SII_LIB_LOG_DEBUG2(("SiiDrvTxCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	baseAddr = sBaseAddrGet(pObj);
	instCra  = sCraInstGet(pObj);

	//Initialize
	pObj->hdcpStatus		= SII_DRV_HDCP_STATUS__OFF;
	pObj->isHdmiConnected	= false;

	// Create a timer to capture group interrupts
	pObj->instIntrHandler = SII_LIB_SEQ_TIMER_CREATE("Tx Interrupt Handler", sTxGroupInterruptHandler, SII_LIB_OBJ_INST(pObj), TIMER_START__TX_INTR_PRI);
	SII_PLATFORM_DEBUG_ASSERT(pObj->instIntrHandler);
	SiiLibSeqTimerStart(pObj->instIntrHandler, TIMER_START__TX_INTR, TIMER_START__TX_INTR_REPEAT);

	//Tx Soft Reset
	//SiiDrvCraSetBit8(instCra, baseAddr | REG_ADDR__PWD_SRST, BIT_MSK__PWD_SRST__REG_SW_RST);

	//Creating HDMI Tx
	pObj->hdmiConfig.baseAddrTx			= baseAddr;
	pObj->hdmiConfig.instTxCra				= instCra;
	pObj->hdmiConfig.instTx				= SII_LIB_OBJ_INST(pObj);
	pObj->hdmiConfig.cbFunc	= SiiDrvTxHdmiModCallBack;
	pObj->hdmiConfig.bScdcEn = pObj->pConfig->bScdcEn;
	pObj->hdmiConfig.bHdcpEn = pObj->pConfig->bHdcpEn;
	pObj->isHpdForced = false;
	pObj->instHdmiTx = SiiModTxHdmiCreate("tx_hdmi", &pObj->hdmiConfig);
	SII_PLATFORM_DEBUG_ASSERT(pObj->instHdmiTx);
	pConfig->scdcInst = pObj->hdmiConfig.scdcInst;

	// create HDCP object
	pObj->hdcpConfig.baseAddrTx	= baseAddr;
	pObj->hdcpConfig.bHdcpRepeat	= false;
	pObj->hdcpConfig.bHdcp2xEn		= pObj->pConfig->bHdcp2xEn;
	pObj->hdcpConfig.maxDsDev		= 127;
	pObj->hdcpConfig.instTxCra		= instCra;
	pObj->hdcpConfig.instTx		= SII_LIB_OBJ_INST(pObj);
	pObj->hdcpConfig.cbFunc		= SiiDrvTxHdcpModCallBack;
	pObj->instHdcp = SiiModTxHdcpCreate("tx_hdcp", &pObj->hdcpConfig);
	SII_PLATFORM_DEBUG_ASSERT(pObj->instHdcp);

	if (pObj->pConfig->bVidPathEn) {
		//Creating VideoPath
		pObj->vidPathConfig.instTxCra	= instCra;
		pObj->vidPathConfig.instTx		= SII_LIB_OBJ_INST(pObj);
		pObj->instVideoPath	= SiiModTxVideoPathCreate("VideoPath", &pObj->vidPathConfig);
		SII_PLATFORM_DEBUG_ASSERT(pObj->instVideoPath);
	}

	//clear reset
	//SiiDrvCraClrBit8(instCra, baseAddr | REG_ADDR__PWD_SRST, BIT_MSK__PWD_SRST__REG_SW_RST);

	return SII_LIB_OBJ_INST(pObj);
}

void SiiDrvTxDelete(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	/*--------------------------------*/
	/* Deallocate resources           */
	/* (reverse order)                */
	/*--------------------------------*/
	if (pObj->pConfig->bVidPathEn) {
		SiiModTxVideoPathDelete(pObj->instVideoPath);
	}

	SiiModTxHdcpDelete(pObj->instHdcp);
	SiiModTxHdmiDelete(pObj->instHdmiTx);
	SiiLibSeqTimerDelete(pObj->instIntrHandler);
	SiiLibMallocDelete(pObj->pConfig);
	SII_LIB_OBJ_DELETE(pObj);
}

//Register Application's callback function
void SiiDrvTxRegisterCallBack(SiiInst_t inst, TxcbFunc cbFunc)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	pObj->txEventNotifyCbFunc = cbFunc;
}

//Tx Driver's Callback function - called by internal modules to notify respective events.
void SiiDrvTxHdmiModCallBack(SiiInst_t instTx, SiiDrvTxEvent_t eventFlags)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(instTx);

	SiiModTxHdmiState_t hdmiState = SII_MOD_TX_HDMI_STATUS__TMDS_OFF;
	bool_t hdcpEnable = false;
	bool_t hotPlug	  = false;

	if (eventFlags & SII_DRV_TX_EVENT__SCDC_EVENT) {
		uint32_t scdc_events;
		SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_EVENTS, (void *)&scdc_events);
		//NOT SURE WHAT ELSE NEED TO DO
	}

	if (eventFlags & (SII_DRV_TX_EVENT__HOT_PLUG_CHNG | SII_DRV_TX_EVENT__RSEN_CHNG)) {
		SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__HOTPLUG, &hotPlug);
		if (hotPlug) {
			pObj->isHdmiConnected = true;
		} else {
			pObj->isHdmiConnected = false;
		}
		{
			SiiEdid_t edid = {0};
			SiiLibEdidPar_t parseEdid = {0};
			SiiTmdsMode_t tmdsMode = SII_TMDS_MODE__NONE;

			SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__EDID, &edid);
			SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__PARSED_EDID, &parseEdid);
			SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__TMDS_MODE, &tmdsMode);

		}

		//Set Hot_Plug status in HDCP module.
		SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__TX_HPD_STATUS, &hotPlug);
		{
			SiiDrvDsHdcpVersion_t hdcpver = SII_DRV_DS_HDCP_VER__NONE;
			SiiDrvTxHdcpCapGet(instTx, &hdcpver);
		}
	}
	if (eventFlags & SII_DRV_TX_EVENT__HDMI_STATE_CHNG) {
		//SiiModTxHdmiGet(pObj->pConfig->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__TMDS_STATUS, &tmdsStatus); //to read tmds_on/tmds_off
		//Get HDMI External State
		SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__HDMI_STATE, &hdmiState);

		if (hdmiState == SII_MOD_TX_HDMI_STATUS__HDCP_ON) {
			hdcpEnable	= true;
			SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_ENABLE, &hdcpEnable);
		} else if (hdmiState == SII_MOD_TX_HDMI_STATUS__HDCP_OFF) {
			hdcpEnable	= false;
			SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_ENABLE, &hdcpEnable);		//Disable HDCP
		}
	}

	//Notifing to application.
	if (pObj->txEventNotifyCbFunc) {
		pObj->txEventNotifyCbFunc(eventFlags);
	}
}

//Tx Driver's HDCP Callback function - called by HDCP module to notify events.
void SiiDrvTxHdcpModCallBack(SiiInst_t instTx, SiiDrvTxEvent_t eventFlags)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(instTx);

	if (eventFlags & SII_DRV_TX_EVENT__HDCP_STATE_CHNG) {
		SiiModTxHdcpGet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_STATUS, &pObj->hdcpStatus); //Get Hdcp Status from hdcp module

		SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__HDCP_STATUS, &pObj->hdcpStatus); //Update Tx Hdcp Status in hdmi module
	}

	//Notifing to application.
	if (pObj->txEventNotifyCbFunc) {
		pObj->txEventNotifyCbFunc(eventFlags);
	}
}

void SiiDrvTxEdidGet(SiiInst_t inst, SiiEdid_t* pEdid)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__EDID, pEdid);
}

void SiiDrvTxEdidParseGet(SiiInst_t inst, SiiLibEdidPar_t* pEdid)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__PARSED_EDID, pEdid);
}

void SiiDrvTxLipSyncInfoGet(SiiInst_t inst, SiiLipSyncInfo_t* lipSync)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__EDID_LIPSYNC, lipSync);
}

bool_t SiiDrvTxHotPlugStatusGet(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	bool_t hotPlug = false;
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__HOTPLUG, &hotPlug);

	return hotPlug;
}

bool_t SiiDrvTxRsenStatusGet(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	bool_t rsen = false;
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__RSEN, &rsen);

	return rsen;
}

void SiiDrvTxTmdsModeSet(SiiInst_t inst, SiiTmdsMode_t tmdsMode)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__TMDS_MODE, &tmdsMode);
}

SiiTmdsMode_t SiiDrvTxTmdsModeStatusGet(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiTmdsMode_t tmdsMode = SII_TMDS_MODE__NONE;
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__TMDS_MODE, &tmdsMode);

	return tmdsMode;
}

uint16_t SiiDrvTxCecPhysicalAddrGet(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	uint16_t cecAddr = 0;
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__CEC_PHY_ADDR, &cecAddr);

	return cecAddr;
}

mt_u32 SiiDrvTxHdcpSetProtectionDone(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	mt_u32 ret;

	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SM_UPDATE_STS, &ret);

	return ret;
}

void SiiDrvTxAvMuteSet(SiiInst_t inst, bool_t onOff)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__AVMUTE, &onOff);
}

void SiiDrvTxHdcpReauth(SiiInst_t inst, bool_t onOff)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_REAUTH, &onOff);
}

//hdcp off -> hdcp on, set true outside, self set false internal
//hdcp on -> hdcp off, set true outside, set false outside too
void SiiDrvTxHdcpMute(SiiInst_t inst, bool_t onOff)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_MUTE, &onOff);
}

void SiiDrvTxInfoframeSet(SiiInst_t inst, const SiiInfoFrame_t *pInfoFrame)
{
	TxObj_t*  pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SII_LIB_LOG_DEBUG1(pObj, ("SiiDrvTxInfoframeSet():: "));
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__INFOFRAME_TYPE, pInfoFrame);
}

void SiiDrvTxInfoframeOnOffSet(SiiInst_t inst, SiiInfoFrameId_t ifId, bool_t onOff)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	uint8_t inData[2] = {0};

	SII_LIB_LOG_DEBUG1(pObj, ("SiiDrvTxInfoframeOnOffSet():: ifId: %i, onOff: %s\n", ifId, onOff ? "ON" : "OFF"));
	inData[0] = ifId;
	inData[1] = onOff;
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__INFOFRAME_ONOFF, inData);
}

void SiiDrvTxInfoframeOnOffGet(SiiInst_t inst, uint8_t *OutD)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__INFOFRAME_ONOFF, OutD);
}

void SiiDrvTxChannelStatusSet(SiiInst_t inst, const SiiChannelStatus_t *pChannelStatus)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SII_LIB_LOG_DEBUG1(pObj, ("SiiDrvTxChannelStatusSet():: "));
	sTxLog((uint8_t*)pChannelStatus, sizeof(SiiChannelStatus_t));

	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__CHANNEL_STATUS, pChannelStatus);
}

void SiiDrvTxAudioFormatSet(SiiInst_t inst, const SiiAudioFormat_t *pAudioFormat)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SII_LIB_LOG_DEBUG1(pObj, ("SiiDrvTxAudioFormatSet():: "));
	sTxLog((uint8_t*)pAudioFormat, sizeof(SiiAudioFormat_t));

	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__AUDIO_FORMAT, pAudioFormat);
}

void SiiDrvTxAudioFormatStatusGet(SiiInst_t inst, SiiAudioFormat_t *audioFormat)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__AUDIO_FORMAT, audioFormat);
}

void SiiDrvTxOutputBitDepthSet(SiiInst_t inst, SiiDrvBitDepth_t bitDepth)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__OUTPUT_BIT_DEPTH, &bitDepth);
	SiiModVidpathSet(pObj->instVideoPath, SII_MOD_TX_VIDPATH_OPCODE__BIT_DEPTH, &bitDepth);
}

void SiiDrvTxHdcpProtectionSet(SiiInst_t inst, bool_t onOff)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	if (NULL == pObj) {
		return;
	}
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__HDCP_PROTECTION, &onOff);
}

bool_t SiiDrvTxHdcpProtectionGet(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	bool_t onOff;

	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__HDCP_PROTECTION, &onOff);
	return onOff;
}

void SiiDrvTxHdcpStateStatusGet(SiiInst_t inst, SiiDrvHdcpStatus_t *pHdcpStatus)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	if (pObj == NULL) {
		return;
	}
	*pHdcpStatus = pObj->hdcpStatus;
}

void SiiDrvTxHdcpKsvListGet(SiiInst_t inst, SiiDrvHdcpKsvList_t *pBksvList)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpGet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_BKSV_LIST, pBksvList);
}

void SiiDrvTxHdcpKsvListApprovalSet(SiiInst_t inst, bool_t bApproved)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_BKSV_LIST_APPROVAL, &bApproved);
}

void SiiDrvTxHdcpTopologyGet(SiiInst_t inst,  SiiDrvHdcpTopology_t *pTopology)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpGet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_TOPOLOGY, pTopology);
}

void SiiDrvTxHdcp2ContentTypeSet(SiiInst_t inst, SiiDrvHdcpContentType_t *pContentType)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpSet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_CONTENT_TYPE, pContentType);

}

void SiiDrvTxHdcp2xCupdStatusGet(SiiInst_t inst, SiiDrvHdcp2xCupdChkStat_t *pHdcp2xCupdStat)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpGet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP2X_CUPD_STAT, pHdcp2xCupdStat);
}

void SiiDrvTxHdcpCapGet(SiiInst_t inst, SiiDrvDsHdcpVersion_t *pHdcpCap)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpGet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_CAP, pHdcpCap);
}

void SiiDrvTxHdcpVerGet(SiiInst_t inst, SiiDrvDsHdcpVersion_t *pHdcpCap)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdcpGet(pObj->instHdcp, SII_MOD_TX_HDCP_OPCODE__HDCP_VERSION, pHdcpCap);
}

void SiiDrvTxReadEDIDFromSink(SiiInst_t inst, uint8_t *param)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__READ_EDID_FROM_SINK, param);
}

void SiiDrvTx_Get_Plug_Status(SiiInst_t inst, uint8_t *status)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__TPI_PLUG_STATUS, status);
}

void SiiDrvTxHvSyncPolaritySet(SiiInst_t inst, SiiHvSyncPol_t *hvSyncPol)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModVidpathSet(pObj->instVideoPath, SII_MOD_TX_VIDPATH_OPCODE__HV_SYNC_POLARITY, hvSyncPol);
}

void SiiDrvTxInputQuantSet(SiiInst_t inst, SiiQuantLevel_t *Quant)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModVidpathSet(pObj->instVideoPath, SII_MOD_TX_VIDPATH_OPCODE__QUANTIZATION, Quant);
}

void SiiDrvTxColorInfoConfig(SiiInst_t inst, SiiDrvTxColorInfoCfg_t *clrInfo)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModVidpathSet(pObj->instVideoPath, SII_MOD_TX_VIDPATH_OPCODE__COLOR_INFO_CONFIG, clrInfo);
}

void SiiDrvTxOutputColorSpaceSet(SiiInst_t inst, SiiDrvClrSpc_t *clrSpc)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModVidpathSet(pObj->instVideoPath, SII_MOD_TX_VIDPATH_OPCODE__OUTPUT_COLORSPACE, clrSpc);
}

void SiiDrvTxOutputColorimetrySet(SiiInst_t inst, SiiDrvConvStd_t *std)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModVidpathSet(pObj->instVideoPath, SII_MOD_TX_VIDPATH_OPCODE__COLORIMETRY, std);
}

void SiiDrvTxOutputCscMtxSet(SiiInst_t inst, void *mtx)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiModVidpathSet(pObj->instVideoPath, SII_MOD_TX_VIDPATH_OPCODE__CSC0_MATRIX, mtx);
}

void SiiDrvTxScdcManufacturerRegisterstatus(SiiInst_t inst, SiiDrvTxScdcManufacturerStatus_t *manfStatus)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_PEER_MANF_STATUS, manfStatus);
}

void SiiDrvTxScdcPeerRegisterstatus(SiiInst_t inst, SiiDrvTxScdcRegisterStatus_t *peerStatus)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_PEER_REG_STATUS, peerStatus);
}

void SiiDrvTxScdcScrambleAndClockstatus(SiiInst_t inst, SiiDrvTxScdcScrmbleclkStatus_t *scrambleclkStatus)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiGet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_CLOCK_STATUS, scrambleclkStatus);
}

void SiiDrvTxScdcResetUpdateRegisters(SiiInst_t inst)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_RESET_PEER_UPDATE_REG, NULL);

}

void SiiDrvTxScdcReadRequestTest(SiiInst_t inst, bool_t msdelay)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_ENABLE_READ_REQ_TEST, &msdelay);

}

void SiiDrvTxScdcSrcCrSet(SiiInst_t inst, bool_t cr)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_SRC_CR, &cr);

}

void SiiDrvTxScdcSrcStreamTypeSet(SiiInst_t inst, uint8_t st)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_SRC_STREAM_TYPE, &st);

}

void SiiDrvTxScdcScrambleenable(SiiInst_t inst, SiiDrvTxScdcSinKCaps_t *scramble_enable)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_ENABLE, scramble_enable);

}

void SiiDrvTxScdcScrambleDisable(SiiInst_t inst, SiiDrvTxScdcSinKCaps_t *scramble_enable)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_DISABLE, scramble_enable);

}

void SiiDrvTxScdcAutopollEnable(SiiInst_t inst, bool_t enable)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__SCDC_ENABLE_AUTOPOLL_MODE, &enable);

}

void SiiDrvTxTmdsMute(SiiInst_t inst, bool_t mute)
{
	TxObj_t* pObj = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiModTxHdmiSet(pObj->instHdmiTx, SII_MOD_TX_HDMI_OPCODE__TMDS_MUTE, &mute);
}

/***** local functions *******************************************************/
//-------------------------------------------------------------------------------------------------
//! @brief      TX Interrupt handler.
//!
//!             Check for TX related interrupts. If found, clear pending hardware interrupt
//!             bits and change the status to indicate pending interrupt.
//!
//!             This function is to be called from the Device Interrupt manager upon receiving
//!             a hardware interrupt from TX.
//!
//! @param[in]  inst  - instance of the notification addressee
//-------------------------------------------------------------------------------------------------
static void sTxGroupInterruptHandler(SiiInst_t inst)
{
	TxObj_t*        pObj        = (TxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiInst_t		craInst		= sCraInstGet(pObj);
	SiiDrvCraAddr_t	baseAddr	= sBaseAddrGet(pObj);

	if (SiiDrvCraIsInterruptRcvd(pObj->pConfig->instCra)) {
		uint8_t l1_intr_stat_0 = 0;
		uint8_t l1_intr_stat_1 = 0;
		//uint8_t status0 = 0;
		//Read All Group Interrupts here.
		l1_intr_stat_0 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__L1_INTR_STAT_0);
		l1_intr_stat_1 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__L1_INTR_STAT_1);

		//status0 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__SCDC_INTR0);

		//if(l1_intr_stat_0 || l1_intr_stat_1)
		//SII_LIB_LOG_DEBUG1(pObj, ("L1_INTR_STAT_0: %02x	L1_INTR_STAT_1: %02x\n", l1_intr_stat_0, l1_intr_stat_1));

		if ((l1_intr_stat_0 & BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B0) || pObj->isHpdForced) { //Check for INTR1 Interrupts
			SI_HDMI20_PRINT("\n%s_%d: 0x%x\n", __func__, __LINE__, (uint32_t)l1_intr_stat_0);
			SiiModTxHdmiTpiInterruptHandler(pObj->instHdmiTx);
			pObj->isHpdForced = 0;
		}
/* Mini Uboot supports HDCP, Lite Uboot NOT support HDCP */
#if !defined(CONFIG_TARGET_SYMPHONY6_LITE)
		if (l1_intr_stat_0 & (BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B4 | BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B7)) { //Check for Hdcp(TPI IP) or HDCP2x Group Interrupts
			SI_HDMI20_PRINT("\n%s_%d: 0x%x\n", __func__, __LINE__, (uint32_t)l1_intr_stat_0);
			SiiModTxHdcpInterruptHandler(pObj->instHdcp);
		}
#endif
		if (pObj->pConfig->bVidPathEn && l1_intr_stat_1 & BIT_MSK__L1_INTR_STAT_1__L1_INTR_STAT_B10) { //Check for VideoPath Interrupts
			SI_HDMI20_PRINT("\n%s_%d: 0x%x\n", __func__, __LINE__, (uint32_t)l1_intr_stat_1);
			SiiModVidPathInterruptHandler(pObj->instVideoPath);
		}
		if (pObj->pConfig->bCpiEn && l1_intr_stat_1 & BIT_MSK__L1_INTR_STAT_1__L1_INTR_STAT_B9) { //Check for CEC IP Interrupts
			SI_HDMI20_PRINT("\n%s_%d: 0x%x\n", __func__, __LINE__, (uint32_t)l1_intr_stat_1);
			pObj->txEventNotifyCbFunc(SII_DRV_TX_EVENT__CEC_CMD_RECEIVED);
		}
		if (pObj->pConfig->bScdcEn && (l1_intr_stat_0 & BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B6) ) {
			SI_HDMI20_PRINT("\n%s_%d: 0x%x\n", __func__, __LINE__, (uint32_t)l1_intr_stat_0);
			//if(pObj->pConfig->bScdcEn)
			SiiModTxScdcInterruptHandler(pObj->hdmiConfig.scdcInst);
		}
	}
}

static SiiDrvCraAddr_t sBaseAddrGet(TxObj_t* pObj)
{
	return pObj->pConfig->baseAddr;
}

static SiiInst_t sCraInstGet(TxObj_t* pObj)
{
	return pObj->pConfig->instCra;
}

static void sTxLog(uint8_t *pData, uint16_t len)
{
	int i = 0;
	SII_LIB_LOG_DEBUG2(("\n"));
	while (len--) {
		SII_LIB_LOG_DEBUG2((" %02X", *pData));
		pData++;
		if (++i == 0x10) {
			SII_LIB_LOG_DEBUG2(("\n"));
			i = 0;
		}

	}
	SII_LIB_LOG_DEBUG2(("\n"));
}

/***** end of file ***********************************************************/
