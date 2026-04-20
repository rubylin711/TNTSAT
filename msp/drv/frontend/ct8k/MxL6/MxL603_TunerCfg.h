/********************************************************************************************/
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*******************************************************************************
 *
 * FILE NAME          : MxL603_TunerCfg.h
 *
 * AUTHOR             : Dong Liu
 *
 * DATE CREATED       : 11/16/2011
 *
 * DESCRIPTION        : This file contains MxL603 common control register
 *                      definitions
 *
 *******************************************************************************
 *                Copyright (c) 2011, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MXL603_TUNER_CFG_H__
#define __MXL603_TUNER_CFG_H__

/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

#include "MaxLinearDataTypes.h"
#include "MxL603_TunerSpurTable.h"



/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/

typedef struct
{
  UINT8 regAddr;
  UINT8 mask;
  UINT8 data;
} MXL603_REG_CTRL_INFO_T, *PMXL603_REG_CTRL_INFO_T;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/

extern MXL603_REG_CTRL_INFO_T MxL603_OverwriteDefaults_ct8k[];
extern MXL603_REG_CTRL_INFO_T MxL603_DigitalDvbc_ct8k[];
extern MXL603_REG_CTRL_INFO_T MxL603_DigitalIsdbtAtsc_ct8k[];
extern MXL603_REG_CTRL_INFO_T MxL603_DigitalDvbt_ct8k[];

/******************************************************************************
    Prototypes
******************************************************************************/

// Functions for register write operation
MXL_STATUS MxL603_Ctrl_ProgramRegisters_ct8k(UINT8 devId, PMXL603_REG_CTRL_INFO_T ctrlRegInfoPtr);
//MXL_STATUS MxL603_Ctrl_WriteRegField(UINT8 devId, PMXL603_REG_CTRL_INFO_T ctrlRegInfoPtr);

MXL_STATUS Ctrl_SetRfFreqLutTblReg_ct8k(UINT8 devId, UINT32 FreqInHz, PMXL603_CHAN_DEPENDENT_FREQ_TABLE_T freqLutPtr);

#endif /* __MXL603_TUNER_CFG_H__*/




