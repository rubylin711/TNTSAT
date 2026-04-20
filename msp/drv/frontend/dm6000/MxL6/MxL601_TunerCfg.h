/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*******************************************************************************
 *
 * FILE NAME          : MxL601_TunerCfg.h
 *
 * AUTHOR             : Dong Liu
 *
 * DATE CREATED       : 11/16/2011
 *
 * DESCRIPTION        : This file contains MxL601 common control register
 *                      definitions
 *
 *******************************************************************************
 *                Copyright (c) 2011, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MXL601_TUNER_CFG_H__
#define __MXL601_TUNER_CFG_H__

/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

#include "MxL601_TunerSpurTable.h"

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/

typedef struct
{
  UINT8 regAddr;
  UINT8 mask;
  UINT8 data;
} MXL601_REG_CTRL_INFO_T, **PMXL601_REG_CTRL_INFO_T;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/
extern MXL601_REG_CTRL_INFO_T MxL601_OverwriteDefaults[];
extern MXL601_REG_CTRL_INFO_T MxL601_AnalogNtsc[];
extern MXL601_REG_CTRL_INFO_T MxL601_AnalogPal[];
extern MXL601_REG_CTRL_INFO_T MxL601_AnalogSecam[];
extern MXL601_REG_CTRL_INFO_T MxL601_DigitalDvbc[];
extern MXL601_REG_CTRL_INFO_T MxL601_DigitalIsdbtAtsc[];
extern MXL601_REG_CTRL_INFO_T MxL601_DigitalDvbt[];

extern MXL601_REG_CTRL_INFO_T MxL601_Ntsc_RfLutSwpHIF[];
extern MXL601_REG_CTRL_INFO_T MxL601_Ntsc_16MHzRfLutSwpLIF[];
extern MXL601_REG_CTRL_INFO_T MxL601_Ntsc_24MHzRfLutSwpLIF[];

extern MXL601_REG_CTRL_INFO_T MxL601_Pal_RfLutSwpLIF[];
extern MXL601_REG_CTRL_INFO_T MxL601_PalD_RfLutSwpHIF[];
extern MXL601_REG_CTRL_INFO_T MxL601_PalI_RfLutSwpHIF[];
extern MXL601_REG_CTRL_INFO_T MxL601_PalBG_8MHzBW_RfLutSwpHIF[];
extern MXL601_REG_CTRL_INFO_T MxL601_PalBG_7MHzBW_RfLutSwpHIF[];
extern MXL601_REG_CTRL_INFO_T MxL601_Ntsc_HRCRfLutSwpLIF[];


/******************************************************************************
    Prototypes
******************************************************************************/

// Functions for register write operation
//MXL_STATUS Ctrl_ProgramRegisters(UINT8 I2cAddr, PREG_CTRL_INFO_T ctrlRegInfoPtr);
//MXL_STATUS Ctrl_WriteRegField(UINT8 I2cAddr, PREG_CTRL_INFO_T ctrlRegInfoPtr);

// Functions called by MxLWare API
//MXL_STATUS Ctrl_SetRfFreqLutReg(UINT8 i2cAddress, UINT32 FreqInHz, PCHAN_DEPENDENT_FREQ_TABLE_T freqLutPtr);

#endif /* __MXL601_TUNER_CFG_H__*/




