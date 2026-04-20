/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2020                                                      */
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
****************************************************************************/
#ifndef __MT_FE_COMMON_CS8800_H__
#define __MT_FE_COMMON_CS8800_H__



#ifdef __cplusplus
extern "C" {
#endif
#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
#include "mt_type.h"
#else //defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
/*	VARIABLE TYPE DEFINES*/
#if 1
#define	U8	unsigned char					/* 8bit unsigned	*/
#define	S8	signed char						/* 8bit unsigned	*/
#define	U16	unsigned short					/* 16bit unsigned	*/
#define	S16	signed short					/* 16bit unsigned	*/
#define	U32	unsigned int					/* 32bit unsigned	*/
#define	S32	signed int						/* 16bit unsigned	*/
#define	DB	double
typedef int BOOL;
#else
typedef	unsigned char	U8;					/* 8bit unsigned	*/
typedef	unsigned char	S8;					/* 8bit unsigned	*/
typedef	unsigned short	U16;				/* 16bit unsigned	*/
typedef	signed short	S16;				/* 16bit unsigned	*/
typedef	unsigned int	U32;				/* 32bit unsigned	*/
typedef	signed int		S32;				/* 16bit unsigned	*/
typedef	double			DB;
#endif


#ifndef NULL
#define NULL	0
#endif

#ifndef BOOL
//#define BOOL	boolean
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#endif //defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
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
	MtFeType_J83B			 = 0x11,
	MtFeType_DVBT			 = 0x20,
	MtFeType_DVBT2			 = 0x21,
	MtFeType_DVBT_T2		 = 0x28,
	MtFeType_DTMB			 = 0x30,
	MtFeType_CTTB			 = 0x31,
	MtFeType_DVBS			 = 0x40,
	MtFeType_DVBS2			 = 0x41,
	MtFeType_DVBS2X			 = 0x42,
	MtFeType_DVBS_S2		 = 0x48,
	MtFeType_ABS			 = 0x50,
	MtFeType_TMS			 = 0x51,
	MtFeType_DIRECTV		 = 0x60,
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
	MtFeModMode_64Apsk,
	MtFeModMode_128Apsk,
	MtFeModMode_256Apsk,
	MtFeModMode_8Apsk_L,
	MtFeModMode_16Apsk_L,
	MtFeModMode_32Apsk_L,
	MtFeModMode_64Apsk_L,
	MtFeModMode_128Apsk_L,
	MtFeModMode_256Apsk_L,
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
    MtFeCodeRate_Undef = 0
    ,MtFeCodeRate_1_4
    ,MtFeCodeRate_1_3
    ,MtFeCodeRate_2_5
    ,MtFeCodeRate_1_2
    ,MtFeCodeRate_3_5
    ,MtFeCodeRate_2_3
    ,MtFeCodeRate_3_4
    ,MtFeCodeRate_4_5
    ,MtFeCodeRate_5_6
    ,MtFeCodeRate_7_8
    ,MtFeCodeRate_8_9
    ,MtFeCodeRate_9_10
    //,MtFeCodeRate_1_2L
    //,MtFeCodeRate_3_5L
    //,MtFeCodeRate_2_3L
    ,MtFeCodeRate_5_9
    //,MtFeCodeRate_5_9L
    ,MtFeCodeRate_7_9
    ,MtFeCodeRate_4_15
    ,MtFeCodeRate_7_15
    ,MtFeCodeRate_8_15
    //,MtFeCodeRate_8_15L
    ,MtFeCodeRate_11_15
    //,MtFeCodeRate_11_15L
    ,MtFeCodeRate_13_18
    ,MtFeCodeRate_9_20
    ,MtFeCodeRate_11_20
    ,MtFeCodeRate_23_36
    ,MtFeCodeRate_25_36
    ,MtFeCodeRate_11_45
    ,MtFeCodeRate_13_45
    ,MtFeCodeRate_14_45
    ,MtFeCodeRate_26_45
    //,MtFeCodeRate_26_45L
    ,MtFeCodeRate_28_45
    ,MtFeCodeRate_29_45
    //,MtFeCodeRate_29_45L
    ,MtFeCodeRate_31_45
    //,MtFeCodeRate_31_45L
    ,MtFeCodeRate_32_45
    //,MtFeCodeRate_32_45L
    ,MtFeCodeRate_77_90
    ,MtFeCodeRate_6_7
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
	MT_FE_DEMOD_NOTSUPPORT	 = 0,
	MT_FE_DEMOD_M88DA3100	 = 0x10,
	MT_FE_DEMOD_M88DS3002	 = 0x20,
	MT_FE_DEMOD_M88DS3002B	 = 0x21,
	MT_FE_DEMOD_M88DS3103	 = 0x22,
	MT_FE_DEMOD_M88DS3103B	 = 0x23,
	MT_FE_DEMOD_M88DS3103C	 = 0x24,
	MT_FE_DEMOD_M88DS6103	 = 0x25,
	MT_FE_DEMOD_M88DD2000	 = 0x40,
	MT_FE_DEMOD_M88DD3000	 = 0x41,
	MT_FE_DEMOD_M88DC2800	 = 0x50,
	MT_FE_DEMOD_M88RS2000	 = 0x61,
	MT_FE_DEMOD_M88RS6000	 = 0x62,
	MT_FE_DEMOD_M88RS6060	 = 0x63,
	MT_FE_DEMOD_M88RS6000B	 = 0x64,
	MT_FE_DEMOD_JAZZ		 = 0x70,
	MT_FE_DEMOD_CONCERTO	 = 0x71,
	MT_FE_DEMOD_CS8000_SAT	 = 0x80,
	MT_FE_DEMOD_CS8000_CAB	 = 0x81,
	MT_FE_DEMOD_UNDEF		 = -1
} MT_FE_SUPPORTED_DEMOD;

/*****************************************************************************/


#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_COMMON_CS8800_H__ */

