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
* file si_mod_tx_scdc_api.h
*
* brief HDCP - Tx SCDC API
*
*****************************************************************************/
#ifndef __SI_MOD_TX_SCDC_API_H__
#define __SI_MOD_TX_SCDC_API_H__

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "si_drv_tx_api.h"
#include "si_lib_time_api.h"
#include "si_lib_edid_api.h"

/***** public macro definitions **********************************************/
/***** public type definitions ***********************************************/
typedef uint32_t SiiModTxScdcEvent_t;
/**
* @brief TX HDMI SCDCS module event notification function pointer
*/
typedef void (*SiiModTxScdcsiNotifyEvent_t)(SiiInst_t, uint32_t);

//! Opcodes for Internal module variables.
typedef enum {
	SII_MOD_TX_SCDC_OPCODE__PEER_MANF_STATUS,
	SII_MOD_TX_SCDC_OPCODE__PEER_SCDC_STATUS,
	SII_MOD_TX_SCDC_OPCODE__SCRAMBLE_CLOCK_STATUS,
	SII_MOD_TX_SCDC_OPCODE__RESET_UPDATE_REGISTER,
	SII_MOD_TX_SCDC_OPCODE__ENABLE_AUTOPOLL_MODE,
	SII_MOD_TX_SCDC_OPCODE__ENABLE_READ_REQ_TEST,
	SII_MOD_TX_SCDC_OPCODE__SCDC_SCRAMBLE_ENABLE,
	SII_MOD_TX_SCDC_OPCODE__SCDC_SCRAMBLE_DISABLE,
	SII_MOD_TX_SCDC_OPCODE__SCDC_RESET_CAPABILITY,
	SII_MOD_TX_SCDC_OPCODE__SCDC_SINK_CAPABILITY,
	SII_MOD_TX_SCDC_OPCODE__SCDC_SRC_CR,
	SII_MOD_TX_SCDC_OPCODE__SCDC_SRC_STREAM_TYPE,
} SiiModTxScdcOpcode_t;

//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------
typedef struct {
	SiiDrvCraAddr_t      baseAddr;       //!< Base Addrress of SCDC-Tx register space
	bool_t               bReadReq;       //!< If true then use read-request if Sink support read request as well.
	uint8_t              srcVersion;     //!< Source version
	SiiInst_t			 instTxCra;
	SiiModTxScdcsiNotifyEvent_t cbFunc;  //!< SCDCS event notification call back
} SiiModTxScdcConfig_t;

/*****************************************************************************/
/**
* @Function:    SiiModTxScdcCreate
* @brief Tx SCDC driver constructor
*
* @param[in]  pNameStr   Name of instance
* @param[in]  parentInst Handle to parent instance
* @param[in]  pConfig    Static configuration parameters
*
* @retval                Handle to instance
*
*****************************************************************************/
SiiInst_t SiiModTxScdcCreate(char *pNameStr, SiiInst_t parentInst, SiiModTxScdcConfig_t *pConfig);

/*****************************************************************************/
/**
* @Function:    SiiModTxScdcDelete
* @brief Tx SCDC driver destructor
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiModTxScdcDelete(SiiInst_t inst);

/*****************************************************************************/
/**
* @Function:    SiiModTxScdcGet
*
* @brief Scdc Tx Property Get API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  outData	 out Data
*
*****************************************************************************/
bool_t SiiModTxScdcGet(SiiInst_t inst, SiiModTxScdcOpcode_t opcode, void *outData);

/*****************************************************************************/
/**
* @Function:    SiiModTxScdcSet
*
* @brief Scdc Tx Property Set API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  inData	 input Data
*
*****************************************************************************/
bool_t SiiModTxScdcSet(SiiInst_t inst, SiiModTxScdcOpcode_t opcode, void *indata);

/*****************************************************************************/
/**
* @Function:    SiiModTxScdcInterruptHandler
* @brief SCDC Interrupt Handler
*
* @param[in]  inst       Handle to instance
*
*****************************************************************************/
void SiiModTxScdcInterruptHandler(SiiInst_t inst);
/*****************************************************************************/

#endif // __SI_DRV_TX_SCDC_API_H__

/***** end of file ***********************************************************/
