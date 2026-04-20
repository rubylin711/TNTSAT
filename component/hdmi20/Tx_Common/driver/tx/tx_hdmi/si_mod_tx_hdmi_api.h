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
* @file si_drv_hdmi_tx_internal.h
*
* @brief HDMI Tx Internal API
*
*****************************************************************************/
#ifndef __SI_MOD_TX_HDMI_API_H__
#define __SI_MOD_TX_HDMI_API_H__

#include "si_drv_tx_api.h"

/***** public type definitions ***********************************************/
typedef uint32_t SiiModTxHdmiEvent_t;
typedef void (*HdmiTxEventNotifyCallBack)(SiiInst_t, SiiModTxHdmiEvent_t);

//! Opcodes for Internal module variables.
typedef enum {
	SII_MOD_TX_HDMI_OPCODE__HOTPLUG,
	SII_MOD_TX_HDMI_OPCODE__RSEN,
	SII_MOD_TX_HDMI_OPCODE__EDID,
	SII_MOD_TX_HDMI_OPCODE__PARSED_EDID,
	SII_MOD_TX_HDMI_OPCODE__HDMI_STATE,
	SII_MOD_TX_HDMI_OPCODE__TMDS_MODE,
	SII_MOD_TX_HDMI_OPCODE__INFOFRAME_TYPE,
	SII_MOD_TX_HDMI_OPCODE__INFOFRAME_ONOFF,
	SII_MOD_TX_HDMI_OPCODE__CHANNEL_STATUS,
	SII_MOD_TX_HDMI_OPCODE__AUDIO_FORMAT,
	SII_MOD_TX_HDMI_OPCODE__OUTPUT_BIT_DEPTH,
	SII_MOD_TX_HDMI_OPCODE__CEC_PHY_ADDR,
	SII_MOD_TX_HDMI_OPCODE__HDCP_PROTECTION,
	SII_MOD_TX_HDMI_OPCODE__HDCP_STATUS,
	SII_MOD_TX_HDMI_OPCODE__EDID_LIPSYNC,
	SII_MOD_TX_HDMI_OPCODE__HW_UPDATE_START,
	SII_MOD_TX_HDMI_OPCODE__SCDC_PEER_MANF_STATUS,
	SII_MOD_TX_HDMI_OPCODE__SCDC_PEER_REG_STATUS,
	SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_CLOCK_STATUS,
	SII_MOD_TX_HDMI_OPCODE__SCDC_RESET_PEER_UPDATE_REG,
	SII_MOD_TX_HDMI_OPCODE__SCDC_ENABLE_AUTOPOLL_MODE,
	SII_MOD_TX_HDMI_OPCODE__SCDC_ENABLE_READ_REQ_TEST,
	SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_ENABLE,
	SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_DISABLE,
	SII_MOD_TX_HDMI_OPCODE__SCDC_EVENTS,
	SII_MOD_TX_HDMI_OPCODE__SM_UPDATE_STS,
	SII_MOD_TX_HDMI_OPCODE__TMDS_MUTE,
	SII_MOD_TX_HDMI_OPCODE__SCDC_SRC_CR,
	SII_MOD_TX_HDMI_OPCODE__SCDC_SRC_STREAM_TYPE,
	SII_MOD_TX_HDMI_OPCODE__READ_EDID_FROM_SINK,
	SII_MOD_TX_HDMI_OPCODE__TPI_PLUG_STATUS,
} SiiModTxHdmiOpcode_t;

/**
* @brief hdmi external states
*/
typedef enum {
	SII_MOD_TX_HDMI_STATUS__TMDS_OFF,
	SII_MOD_TX_HDMI_STATUS__TMDS_ON,
	SII_MOD_TX_HDMI_STATUS__HDCP_OFF,
	SII_MOD_TX_HDMI_STATUS__HDCP_ON,
} SiiModTxHdmiState_t;

typedef struct {
	SiiDrvCraAddr_t     baseAddrTx;    //!< Base Addrress of Tx register space
	SiiInst_t			instTxCra;
	SiiInst_t			instTx;		//!< Tx Controller's instance.
	void				(*cbFunc)(SiiInst_t, SiiModTxHdmiEvent_t);
	SiiInst_t			scdcInst;
	bool_t              bScdcEn;
	bool_t              bHdcpEn;
} SiiModTxHdmiConfig_t;

/***** HDMI Tx public functions ******************************************************/

/*****************************************************************************/
/**
* @Function:    SiiModTxHdmiCreate
*
* @brief HDMI Tx driver constructor
*
* @param[in]  pNameStr   Name of instance
* @param[in]  pConfig    Static configuration parameters
*
* @retval                Handle to instance
*
*****************************************************************************/
SiiInst_t SiiModTxHdmiCreate(char *pNameStr, SiiModTxHdmiConfig_t *pConfig);
#if (__HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__)
	SiiInst_t SiiModTxHdmiCreate_uboot2main(char *pNameStr, SiiModTxHdmiConfig_t *pConfig);
#endif

/*****************************************************************************/
/**
* @Function:    SiiModTxHdmiDelete
*
* @brief HDMI Tx driver destructor
*
* @param[in]  inst       Handle to instance
*
*****************************************************************************/
void SiiModTxHdmiDelete(SiiInst_t inst);

/*****************************************************************************/
/**
* @Function:    SiiModTxHdmiTpiInterruptHandler
*
* @brief HDMI Tx TPI Interrupt Handler
*
* @param[in]  inst       Handle to instance
*
*****************************************************************************/
void SiiModTxHdmiTpiInterruptHandler(SiiInst_t inst);

/*****************************************************************************/
/**
* @Function:    SiiModTxHdmiSet
*
* @brief HDMI Tx Property Set API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  inData	 input Data
*
*****************************************************************************/
bool_t SiiModTxHdmiSet(SiiInst_t inst, SiiModTxHdmiOpcode_t opcode, const void *inData);

/*****************************************************************************/
/**
* @Function:    SiiModTxHdmiGet
*
* @brief HDMI Tx Property Get API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  outData	 out Data
*
*****************************************************************************/
bool_t SiiModTxHdmiGet(SiiInst_t inst, SiiModTxHdmiOpcode_t opcode, void *outData);

#endif //__SI_DRV_HDMI_TX_INTERNAL_H__
