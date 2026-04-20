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
* @file si_mod_tx_hdcp_api.h
*
* @brief HDCP - Tx HDCP API
*
*****************************************************************************/
#ifndef __SI_MOD_TX_HDCP_API_H__
#define __SI_MOD_TX_HDCP_API_H__

/***** #include statements ***************************************************/
#include "si_drv_tx_api.h"

/***** public macro definitions **********************************************/

/***** public type definitions ***********************************************/

typedef uint32_t SiiModTxHdcpEvent_t;
typedef void (*HdcpEventNotifyCallBack)(SiiInst_t, SiiModTxHdcpEvent_t);

//! Opcodes for Internal module variables.
typedef enum {
	SII_MOD_TX_HDCP_OPCODE__HDCP_PROTECTION,
	SII_MOD_TX_HDCP_OPCODE__HDCP_ENABLE,
	SII_MOD_TX_HDCP_OPCODE__HDCP_STATUS,
	SII_MOD_TX_HDCP_OPCODE__HDCP_VERSION,
	SII_MOD_TX_HDCP_OPCODE__HDCP_BKSV_LIST,
	SII_MOD_TX_HDCP_OPCODE__HDCP_TOPOLOGY,
	SII_MOD_TX_HDCP_OPCODE__HDCP_CONTENT_TYPE,
	SII_MOD_TX_HDCP_OPCODE__HDCP2X_CUPD_STAT,
	SII_MOD_TX_HDCP_OPCODE__HDCP_BKSV_LIST_APPROVAL,
	SII_MOD_TX_HDCP_OPCODE__TX_HPD_STATUS,
	SII_MOD_TX_HDCP_OPCODE__AVMUTE,
	SII_MOD_TX_HDCP_OPCODE__HDCP_REAUTH,
	SII_MOD_TX_HDCP_OPCODE__DS_MHL_VERSION,
	SII_MOD_TX_HDCP_OPCODE__HDCP_CAP,
	SII_MOD_TX_HDCP_OPCODE__HDCP_MUTE,
} SiiModTxHdcpOpcode_t;

//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

typedef struct {
	SiiDrvCraAddr_t      baseAddrTx;     //!< Base Addrress of Tx register space
	bool_t               bHdcpRepeat;    //!< true if enabled as HDCP repeater.
	bool_t				 bHdcp2xEn;		 //!< true if HDCP2.x is enabled
	uint8_t              maxDsDev;       //!< Maximum number down stream devices. Define 0 if KSV interrogation is not desired for transmitter mode.
	SiiInst_t            instTx;		 //!< tx driver instance
	SiiInst_t			 instTxCra;      //!< cra driver instance
	void				(*cbFunc)(SiiInst_t, SiiModTxHdcpEvent_t);		//!< hdcpEvents Notification callback funtion.
} SiiModTxHdcpConfig_t;

/***** public functions ******************************************************/

/*****************************************************************************/
/**
* @Function:    SiiModTxHdcpCreate
*
* @brief Tx HDCP driver constructor
*
* @param[in]  pNameStr   Name of instance
* @param[in]  pConfig    Static configuration parameters
*
* @retval                Handle to instance
*
*****************************************************************************/
SiiInst_t SiiModTxHdcpCreate(char *pNameStr, SiiModTxHdcpConfig_t *pConfig);
#if (__HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__)
	SiiInst_t SiiModTxHdcpCreate_uboot2main(char *pNameStr, SiiModTxHdcpConfig_t *pConfig);
#endif

/*****************************************************************************/
/**
* @Function:    SiiModTxHdcpDelete
*
* @brief Tx driver destructor
*
* @param[in]  inst       Handle to instance
*
*****************************************************************************/
void SiiModTxHdcpDelete(SiiInst_t inst);

/*****************************************************************************/
/**
* @Function:    SiiModTxHdcpInterruptHandler
*
* @brief HDCP Interrupt Handler
*
* @param[in]  inst       Handle to instance
*
*****************************************************************************/
void SiiModTxHdcpInterruptHandler(SiiInst_t inst);

/*****************************************************************************/
/**
* @Function:    SiiModTxHdcpSet
*
* @brief HDCP Property Set API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  inData	 input Data
*
*****************************************************************************/
#if (MT_SDK_COMPILE_HDMI20 == 0)
	bool_t SiiModTxHdcpSet(SiiInst_t inst, SiiModTxHdcpOpcode_t opcode, const void *inData);
#else
	bool_t SiiModTxHdcpSet(SiiInst_t inst, SiiModTxHdcpOpcode_t opcode, void *inData);
#endif

/*****************************************************************************/
/**
* @Function:    SiiModTxHdcpGet
*
* @brief HDCP Property Get API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  outData	 out Data
*
*****************************************************************************/
bool_t SiiModTxHdcpGet(SiiInst_t inst, SiiModTxHdcpOpcode_t opcode, void *outData);
bool_t SiiModTxHdcpLoadKey(SiiInst_t inst, uint8_t *key, uint32_t len);

#endif // __SI_DRV_TX_HDCP_API_H__

/***** end of file ***********************************************************/
