/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2014                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_common.h
*
* Current version:	00.01
*
* Description: Definitions for all Montage front-end projects common use.
*
* Log:
* Description			Version			Date			Author
*---------------------------------------------------------------------
* Create				00.00			2011.08.18		BJ.Wang
* Modify				00.01			2013.12.05		YZ.Huang
*****************************************************************************************************/
#ifndef __MT_FE_COMMON_DM6K_H__
#define __MT_FE_COMMON_DM6K_H__


#include <linux/printk.h>



#ifdef __cplusplus
extern "C" {
#endif

#include "mt_type.h"
/*****************************************************************************/


#define MT_FE_DEBUG							0		/*	0 off, 1 on*/
#if (MT_FE_DEBUG == 1)
	#define mt_fe_print(str)				printf str;
	#define mt_fe_assert(bool,msg)
#else
	#define mt_fe_print(str)
	#define mt_fe_assert(bool,msg)
#endif



#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x)		((void)(x))
#endif

/************************ TS OUTPUT DEFINES *****************************/

typedef enum _MT_FE_TS_OUT_MODE
{
	MtFeTsOutMode_Unknown = 0,
	MtFeTsOutMode_Serial1,
	MtFeTsOutMode_Serial2,
	MtFeTsOutMode_Serial,
	MtFeTsOutMode_Parallel,
	MtFeTsOutMode_Common
} MT_FE_TS_OUT_MODE;

typedef enum _MT_FE_TS_OUT_MAX_CLOCK
{
	MtFeTSOut_Max_Clock_Unknown = 0,
	MtFeTSOut_Max_Clock_12_MHz,						// for CI & Parallel mode
	MtFeTSOut_Max_Clock_16_MHz,						// for Parallel mode only
	MtFeTSOut_Max_Clock_19_p_2_MHz,					// for Parallel mode only
	MtFeTSOut_Max_Clock_24_MHz,						// for CI & Parallel mode
	MtFeTSOut_Max_Clock_9_p_6_MHz,
	MtFeTSOut_Max_Clock_10_p_7_MHz,
	MtFeTSOut_Max_Clock_Userdefine,
	MtFeTSOut_Max_Clock_Auto
} MT_FE_TS_OUT_MAX_CLOCK;

/*****************************************************************************/

typedef enum _MT_FE_BOOL
{
	MtFe_False = 0,
	MtFe_True
} MT_FE_BOOL;

typedef enum _MT_FE_ON_OFF
{
	MtFe_Off = 0,
	MtFe_On
} MT_FE_ON_OFF;

typedef enum _MT_FE_RET
{
	MtFeErr_Ok					 = 0,
	MtFeErr_Undef				 = -1,
	MtFeErr_Uninit				 = -2,
	MtFeErr_Param				 = -3,
	MtFeErr_NoSupportFunc		 = -4,
	MtFeErr_NoSupportTuner		 = -5,
	MtFeErr_NoSupportDemod		 = -6,
	MtFeErr_UnLock				 = -7,
	MtFeErr_I2cErr				 = -8,
	MtFeErr_DiseqcBusy			 = -9,
	MtFeErr_NoMemory			 = -10,
	MtFeErr_NullPointer			 = -11,
	MtFeErr_TimeOut				 = -12,
	MtFeErr_Fail				 = -13,
	MtFeErr_NoMatch				 = -14,
	MtFeErr_FirmwareErr			 = -15,
	MtFeErr_S2Block				 = -16,		/* DVB-S2 module is running, can't open this module(T2) */
	MtFeErr_T2Block				 = -17		/* DVB-T2 module is running, can't open this module(S2) */
} MT_FE_RET;

typedef enum _MT_FE_TYPE
{
	MtFeType_Undef = 0,
	MtFeType_DVBC,
	MtFeType_DVBT,
	MtFeType_CTTB,
	MtFeType_DVBS,
	MtFeType_DVBS2,
	MtFeType_ABS,
	MtFeType_TMS,
	MtFeType_DVBT2,
	MtFeType_DVBS_S2,
	MtFeType_DVBT_T2,
	MtFeType_DTV_Unknown	= 0xFE,
	MtFeType_DTV_Checked	= 0xFF
} MT_FE_TYPE;

typedef enum _MT_FE_MOD_MODE
{
	MtFeModMode_Undef = 0,
	MtFeModMode_4Qam,
	MtFeModMode_4QamNr,
	MtFeModMode_16Qam,
	MtFeModMode_32Qam,
	MtFeModMode_64Qam,
	MtFeModMode_128Qam,
	MtFeModMode_256Qam,
	MtFeModMode_Qpsk,
	MtFeModMode_8psk,
	MtFeModMode_16Apsk,
	MtFeModMode_32Apsk,
	MtFeModMode_Auto
} MT_FE_MOD_MODE;

typedef enum _MT_FE_LOCK_STATE
{
	MtFeLockState_Undef = 0,
	MtFeLockState_Unlocked,
	MtFeLockState_Locked,
	MtFeLockState_Waiting
} MT_FE_LOCK_STATE;

typedef enum _MT_FE_BANDWIDTH
{
	MtFeBandwidth_Undef = 0,
	MtFeBandwidth_10M,
	MtFeBandwidth_8M,
	MtFeBandwidth_7M,
	MtFeBandwidth_6M,
	MtFeBandwidth_5M,
	MtFeBandwidth_1P7M
} MT_FE_BANDWIDTH;

typedef enum _MT_FE_CODE_RATE
{
	MtFeCodeRate_Undef = 0,
	MtFeCodeRate_1_4,
	MtFeCodeRate_1_3,
	MtFeCodeRate_2_5,
	MtFeCodeRate_1_2,
	MtFeCodeRate_3_5,
	MtFeCodeRate_2_3,
	MtFeCodeRate_3_4,
	MtFeCodeRate_4_5,
	MtFeCodeRate_5_6,
	MtFeCodeRate_7_8,
	MtFeCodeRate_8_9,
	MtFeCodeRate_9_10
} MT_FE_CODE_RATE;

typedef enum _MT_FE_FFT
{
	MtFeFFTMode_Undef = 0,
	MtFeFFTMode_1K,
	MtFeFFTMode_2K,
	MtFeFFTMode_4K,
	MtFeFFTMode_8K,
	MtFeFFTMode_16K,
	MtFeFFTMode_32K,
	MtFeFFTMode_8E,
	MtFeFFTMode_16E,
	MtFeFFTMode_32E
} MT_FE_FFT;

typedef enum _MT_FE_XTAL
{
	MtFeXTALMode_24M = 0,
	MtFeXTALMode_27M
} MT_FE_XTAL;

typedef enum MT_FE_SUPPORTED_DEMOD
{
	MT_FE_DEMOD_NOTSUPPORT,
	MT_FE_DEMOD_M88DA3100,
	MT_FE_DEMOD_M88DS3002,
	MT_FE_DEMOD_M88DD3000,
	MT_FE_DEMOD_M88DC2800,
	MT_FE_DEMOD_UNDEF
} MT_FE_SUPPORTED_DEMOD;

/*****************************************************************************/


#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_COMMON_H__ */

