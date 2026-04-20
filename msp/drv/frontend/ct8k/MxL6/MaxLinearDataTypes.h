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
 * FILE NAME          : MaxLinearDataTypes.h
 *
 * AUTHOR             : Brenndon Lee
 *                      Dong Liu
 *
 * DATE CREATED       : Jul/31, 2006
 *                      Jan/23, 2010
 *
 * DESCRIPTION        : This file contains MaxLinear-defined data types.
 *                      Instead of using ANSI C data type directly in source code
 *                      All module should include this header file.
 *                      And conditional compilation switch is also declared
 *                      here.
 *
 *******************************************************************************
 *                Copyright (c) 2010, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MAXLINEAR_DATA_TYPES_H__
#define __MAXLINEAR_DATA_TYPES_H__

/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

/******************************************************************************
    Macros
******************************************************************************/

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#ifdef _ANSI_C_SOURCE
#define false                1
#define true                 0

#define TRUE                 1
#define FALSE                0
#endif

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/
typedef unsigned char        UINT8;
typedef unsigned short       UINT16;
typedef unsigned int         UINT32;
//typedef unsigned long long   UINT64;
typedef char                 SINT8;
typedef short                SINT16;
typedef int                  SINT32;
typedef float                REAL32;
typedef double               REAL64;
typedef unsigned long        ULONG_32;
typedef unsigned int         MS_BOOL;
typedef unsigned int         MS_U32;
typedef unsigned char        MS_U8;

#ifdef _ANSI_C_SOURCE
typedef unsigned char        bool;
#endif

/******************************************************************************
    Macros
******************************************************************************/

#define AIC_RESET_REG                  0xFF // For MxL603 Tuner

#define PAGE_CHANGE_REG                0x00 // Page change, can configured as page 0 or page 1
#define XTAL_CAP_CTRL_REG              0x01 // Xtal frequency and CAP register
#define XTAL_ENABLE_DIV_REG            0x02 // Xtal enable and frequency div 4 register
#define XTAL_CALI_SET_REG              0x03 // Xtal calibration enable register enable register
#define IF_FREQ_SEL_REG                0x04 // IF frequency selection and manual set bypass register
#define IF_PATH_GAIN_REG               0x05 // IF gain level and path selection register
#define IF_FCW_LOW_REG                 0x06 // Low register of IF FCW set when manual program IF frequency
#define IF_FCW_HIGH_REG                0x07 // High register of IF FCW set when manual program IF frequency
#define AGC_CONFIG_REG                 0x08 // AGC configuration, include AGC select and AGC type
#define AGC_SET_POINT_REG              0x09
#define AGC_FLIP_REG                   0x5E
#define AGC_SLOPE_REG                  0xB5
#define AGC_OFFSET_REG                 0xB4
#define GPO_SETTING_REG                0x0A // GPO set and inquiring register
#define TUNER_ENABLE_REG               0x0B // Power up register, bit<0>
#define TUNE_MODE_REG                  0x0D
#define MAIN_REG_AMP                   0x0E
#define CHAN_TUNE_BW_REG               0x0F // Band width register
#define CHAN_TUNE_LOW_REG              0x10 // Tune frequency set low byte
#define CHAN_TUNE_HI_REG               0x11 // Tune frequency set high byte
#define START_TUNE_REG                 0x12 // sequencer setting register
#define FINE_TUNE_SET_REG              0x13 // Fine tune operation register
#define FINE_TUNE_CTRL_REG_0           0x13 // Fine tune operation register
#define FINE_TUNE_CTRL_REG_1           0x14 // Fine tune operation register

#define FINE_TUNE_OFFSET_LOW_REG       0x14 // Fine tune frequency offset low byte
#define FINE_TUNE_OFFSET_HIGH_REG      0x15 // Fine tune frequency offset high byte
#define CHIP_ID_REQ_REG                0x18 // Tuner Chip ID register
#define CHIP_VERSION_REQ_REG           0x1A // Tuner Chip Revision register

#define RFPIN_RB_LOW_REG               0x1D // RF power low 8 bit register
#define RFPIN_RB_HIGH_REG              0x1E // RF power high 8 bit register
#define SIGNAL_TYPE_REG                0x1E // Signal type

#define DFE_CTRL_ACCUM_LOW_REG         0x24 // Bit<7:0>
#define DFE_CTRL_ACCUM_MID_REG         0x25 // Bit<7:0>
#define DFE_CTRL_ACCUM_HI_REG          0x26 // Bit<1:0>

#define DFE_CTRL_TRIG_REG              0xA0 // Bit<3>
#define DFE_CTRL_RB_HI_REG             0x7B // Bit<7:0>
#define DFE_CTRL_RB_LOW_REG            0x7A // Bit<1:0>

#define RF_REF_STATUS_REG              0x2B // RF/REF lock status register

#define AGC_SAGCLOCK_STATUS_REG        0x2C

#define DFE_DACIF_BYP_GAIN_REG         0x43
#define DIG_ANA_RFRSSI_REG             0x57

#define DFE_SEQ_TUNE_RF1_BO_REG        0x60
#define DFE_SEQ_DIGANA_LT_GAIN_REG     0x62
#define DFE_SEQ_DIGANA_LT_ATTN_REG     0x63
#define XTAL_EXT_BIAS_REG              0x6D

#define RSSI_RESET_REG                 0x78
#define DIG_ANA_GINJO_LT_REG           0x96
#define FINE_TUNE_INIT1_REG            0xA9
#define FINE_TUNE_INIT2_REG            0xAA

#define DFE_AGC_CEIL1_REG              0xB0

#define DFE_RFLUT_BYP_REG              0xDB  // Dec: 220, bit<7>
#define DFE_RFLUT_DIV_MOD_REG          0xDB  // Dec: 221

#define DFE_RFLUT_SWP1_REG             0x49

#define DFE_RFSX_FRAC_MOD1_REG         0xDF
#define DFE_RFSX_FRAC_MOD2_REG         0xE0
#define DFE_RFSX_FRAC_MOD3_REG         0xE1
#define DFE_RFSX_FRAC_MOD4_REG         0xE2

#define DFE_REFLUT_BYP_REG             0xEA  // Dec: 240, bit<6>
#define DFE_REFSX_INT_MOD_REG          0xEB  // Dec: 241

#define APP_MODE_FREQ_HZ_THRESHOLD_1   358000000
#define APP_MODE_FREQ_HZ_THRESHOLD_2   625000000
#define APP_MODE_FREQ_HZ_THRESHOLD_3   700000000

#define MXL603_SPUR_SHIFT_FREQ_WINDOW  500000  // +- 0.5MHz
#define SPUR_SHIFT_FREQ_WINDOW         500000  // +- 0.5MHz

#define IF_GAIN_SET_POINT1             10
#define IF_GAIN_SET_POINT2             11
#define IF_GAIN_SET_POINT3             12

#define DIG_ANA_IF_CFG_0              0x5A
#define DIG_ANA_IF_CFG_1              0x5B
#define DIG_ANA_IF_PWR                0x5C

#define DFE_CSF_SS_SEL                0xEA
#define DFE_DACIF_GAIN                0xDC

#define FINE_TUNE_FREQ_INCREASE       0x01
#define FINE_TUNE_FREQ_DECREASE       0x02

#define RF_SX_FRAC_N_RANGE            0xDD

#define HIGH_IF_35250_KHZ             35250

#define SPUR_SHIFT_CLOCK_ADJUST_MIN    205
#define SPUR_SHIFT_CLOCK_ADJUST_MAX    227


typedef enum
{
  MXL_TRUE = 0,
  MXL_FALSE = 1,
  MXL_SUCCESS = 0,
  MXL_FAILED,
  MXL_BUSY,
  MXL_NULL_PTR,
  MXL_INVALID_PARAMETER,
  MXL_NOT_INITIALIZED,
  MXL_ALREADY_INITIALIZED,
  MXL_BUFFER_TOO_SMALL,
  MXL_NOT_SUPPORTED,
  MXL_TIMEOUT
} MXL_STATUS;

typedef enum{
	MxL_OK					        =  0x0,
	MxL_ERR_INIT			      =  0x1,
	MxL_ERR_RFTUNE			    =  0x2,
	MxL_ERR_SET_REG			    =  0x3,
	MxL_ERR_GET_REG			    =  0x4,
	MxL_ERR_MODE			      =  0x10,
	MxL_ERR_IF_FREQ			    =  0x11,
	MxL_ERR_XTAL_FREQ		    =  0x12,
	MxL_ERR_BANDWIDTH		    =  0x13,
	MxL_GET_ID_FAIL			    =  0x14,
	MxL_ERR_DEMOD_LOCK		  =  0x20,
	MxL_NOREADY_DEMOD_LOCK	=  0x21,
	MxL_ERR_OTHERS			    =  0x0A
}MxL_ERR_MSG;

typedef enum
{
  MXL_DISABLE = 0,
  MXL_ENABLE,

  MXL_UNLOCKED = 0,
  MXL_LOCKED,

  MXL_INVALID = 0,
  MXL_VALID,

  MXL_PORT_LOW = 0,
  MXL_PORT_HIGH,

  MXL_START = 0,
  MXL_FINISH,

  MXL_ABORT_TUNE = 0,
  MXL_START_TUNE,

  MXL_FINE_TUNE_STEP_DOWN = 0,
  MXL_FINE_TUNE_STEP_UP

} MXL_BOOL;

typedef enum
{
  IFX_SUCCESS = 0,
  IFX_FAILED,
  IFX_BUSY,
  IFX_NULL_PTR,
  IFX_INVALID_PARAMETER,
  IFX_BUFFER_TOO_SMALL,
  IFX_TIMEOUT
} IFX_STATUS;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/

/******************************************************************************
    Prototypes
******************************************************************************/

#endif /* __MAXLINEAR_DATA_TYPES_H__ */

