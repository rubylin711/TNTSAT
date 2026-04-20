/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*****************************************************************************
 * Filename:        mt_fe_common.h
 *
 * Description:     Common definitions of Montage products.
 *
 * Current version:	1.02.00
 *
 * Description: Definitions for all Montage front-end projects common use.
 *
 * History:
 *  Description     Version     Date            Author
 *  --------------------------------------------------------------------------
 *  Create          1.00.00     2011.01.27      YZ.Huang
 *  Update          1.01.00     2013.07.12      YZ.Huang
 *  Update          1.02.00     2016.10.08      YZ.Huang
 *****************************************************************************/
#ifndef __MT_FE_COMMON_H__
#define __MT_FE_COMMON_H__


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
	MtFeType_Undef			 = 0,
	MtFeType_DVBC			 = 0x10,
	MtFeType_DVBT			 = 0x20,
	MtFeType_DVBT2			 = 0x21,
	MtFeType_DTMB			 = 0x30,
	MtFeType_CTTB			 = 0x31,
	MtFeType_DvbS			 = 0x40,
	MtFeType_DvbS2			 = 0x41,
	MtFeType_ABS			 = 0x50,
	MtFeType_TMS			 = 0x51,
	MtFeType_DvbS_S2		 = 0x80,
	MtFeType_DvbT_T2		 = 0x81,
	MtFeType_DTV_Unknown	 = 0xFE,
	MtFeType_DTV_Checked	 = 0xFF
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

typedef enum _MT_FE_SUPPORTED_DEMOD
{
	MT_FE_DEMOD_NOTSUPPORT	 = 0,
	MT_FE_DEMOD_M88DA3100	 = 0x10,
	MT_FE_DEMOD_M88DS3002	 = 0x20,
	MT_FE_DEMOD_M88DS3002B	 = 0x21,
	MT_FE_DEMOD_M88DS3103	 = 0x22,
	MT_FE_DEMOD_M88DS3103B	 = 0x23,
	MT_FE_DEMOD_M88DD2000	 = 0x40,
	MT_FE_DEMOD_M88DD3000	 = 0x41,
	MT_FE_DEMOD_M88DC2800	 = 0x50,
	MT_FE_DEMOD_M88RS2000	 = 0x61,
	MT_FE_DEMOD_M88RS6000	 = 0x62,
	MT_FE_DEMOD_JAZZ		 = 0x70,
	MT_FE_DEMOD_CS8000_SAT	 = 0x80,
	MT_FE_DEMOD_CS8000_CAB	 = 0x81,
	MT_FE_DEMOD_UNDEF		 = -1
} MT_FE_SUPPORTED_DEMOD;

/*****************************************************************************/


#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_COMMON_H__ */

