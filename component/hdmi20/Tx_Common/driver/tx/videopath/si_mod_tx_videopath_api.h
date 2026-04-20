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
* @file
*
* @brief VideoPath API
*
*****************************************************************************/
#ifndef __SI_MOD_TX_VIDEOPATH_API_H__
#define __SI_MOD_TX_VIDEOPATH_API_H__

#include "si_datatypes.h"

/***** public type definitions ***********************************************/
typedef uint32_t SiiModTxVidPathEvent_t;

//! Opcodes for Internal module variables.
typedef enum {
	SII_MOD_TX_VIDPATH_OPCODE__COLOR_INFO_CONFIG,
	SII_MOD_TX_VIDPATH_OPCODE__OUTPUT_COLORSPACE,
	SII_MOD_TX_VIDPATH_OPCODE__BIT_DEPTH,
	SII_MOD_TX_VIDPATH_OPCODE__HV_SYNC_POLARITY,
	SII_MOD_TX_VIDPATH_OPCODE__QUANTIZATION,
	SII_MOD_TX_VIDPATH_OPCODE__COLORIMETRY,
	SII_MOD_TX_VIDPATH_OPCODE__CSC0_MATRIX,
} SiiModTxVidPathOpcode_t;

//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------
typedef struct {
	SiiInst_t instTxCra;
	SiiInst_t instTx;
} SiiModTxVideoPathCfg_t;

/*****************************************************************************/
/**
* @brief Video Path Module creation.
*
* @retval              Handle of instance created
*****************************************************************************/
SiiInst_t SiiModTxVideoPathCreate(char *pNameStr, SiiModTxVideoPathCfg_t *pConfig);
#if (__HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__)
	SiiInst_t SiiModTxVideoPathCreate_uboot2main(char *pNameStr, SiiModTxVideoPathCfg_t *pConfig);
#endif

/*****************************************************************************/
/**
* @brief Video Path Module instance deletion.
*
* @param[in]  inst     Handle to instance
*
*****************************************************************************/
void SiiModTxVideoPathDelete( SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Video Path Module Interrupt Handler
*
* @param[in]  inst     Handle to instance
*
*****************************************************************************/
void SiiModVidPathInterruptHandler(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Video Path Property Set API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  inData	 input Data
*
*****************************************************************************/
bool_t SiiModVidpathSet(SiiInst_t inst, SiiModTxVidPathOpcode_t opcode, void *inData);

/*****************************************************************************/
/**
* @brief Video Path Property Get API
*
* @param[in]  inst       Handle to instance
* @param[in]  opcode     Property's opcode
* @param[in]  outData	 out Data
*
*****************************************************************************/
bool_t SiiModVidpathGet(SiiInst_t inst, SiiModTxVidPathOpcode_t opcode, void *outData);

#endif // __SI_MOD_TX__VIDEOPATH_API_H__
