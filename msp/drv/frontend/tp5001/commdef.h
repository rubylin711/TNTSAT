/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifdef _USE_TP5001_CHIP_
/******************************************************
	Sample source code for Evaluation Board
******************************************************/

/******************************************************
commdef.h						
----------------------------------------------------
common global definitions for TUNER MODULE CONTROLER				

<Revision History>					
'09/07/02 : OKAMOTO	[Floreal] V9.2
'09/06/30 : OKAMOTO	[Floreal] V9.1.0
'09/02/25 : OKAMOTO	Add PRESET_PAL_I
'05/09/26 : AZUMA	First Version
----------------------------------------------------
Copyright(C) 2005 SHARP CORPORATION			
******************************************************/

#ifndef _SH_COMMDEF_
#define	_SH_COMMDEF_

//=========================================================================
// INCLUDES
//=========================================================================
//#include <limits.h>
//#include < float.h >


//=========================================================================
// COMPILE SWITCH
//=========================================================================
#define _COMMDEF_PRESET_ENABLE	/* Preset listbox enable */
#define _MN88471_BER_ENABLE
//#define _MN88471_DEBUG
//#define _SI2165_DEBUG
//#define _MN88471_BER_ENABLE

// add by azuma from here +++
typedef enum _LPT_PORT_ADDR_ENUM {
	LPT_PORT_ADDR_378 = 0,
	LPT_PORT_ADDR_3BC,
	LPT_PORT_ADDR_278,
	LPT_PORT_ADDR_MAX,
} LPT_PORT_ADDR_ENUM, *PLPT_PORT_ADDR_ENUM;

typedef enum _I2C_SPEED_ENUM {
	I2C_SPEED_125K = 0,
	I2C_SPEED_63K,
	I2C_SPEED_31K,
	I2C_SPEED_400K,
	I2C_SPEED_50K,
	I2C_SPEED_ENUM_MAX,
} I2C_SPEED_ENUM, *PI2C_SPEED_ENUM;

typedef enum _COMMDEF_PRESET_ENUM {
#ifdef _COMMDEF_PRESET_ENABLE
	COMMDEF_PRESET_NTSC = 0,
	COMMDEF_PRESET_PAL_B,
	COMMDEF_PRESET_PAL_G_H,
	COMMDEF_PRESET_PAL_I,
	COMMDEF_PRESET_PAL_D,
	COMMDEF_PRESET_SECAM_L,
	COMMDEF_PRESET_SECAM_L_ACCENT,
	COMMDEF_PRESET_ISDB_T,
	COMMDEF_PRESET_DVB_T,
#endif /* #ifdef _COMMDEF_PRESET_ENABLE */
	COMMDEF_PRESET_DEFAULT,
	COMMDEF_PRESET_MAX,
} COMMDEF_PRESET_ENUM, *PCOMMDEF_PRESET_ENUM;

//=========================================================================
// DEFINITIONS
//=========================================================================

/* Common definition */
#ifndef UINT8
#define	UINT8		unsigned char
 #ifndef	UINT8_MAX
 #define UINT8_MAX	UCHAR_MAX
 #endif
#define UINT8_MIN	0
#endif

#ifndef UINT16
#define	UINT16		unsigned short
 #ifndef	UINT16_MAX
 #define UINT16_MAX	USHRT_MAX
 #endif
#define UINT16_MIN	0
#endif

#ifndef UINT32
#define UINT32		unsigned long
 #ifndef	UINT32_MAX
 #define UINT32_MAX	ULONG_MAX
 #endif
#define UINT32_MIN_TP5001	0
#endif

#ifndef SINT8
#define	SINT8		signed char
#define SINT8_MAX	SCHAR_MAX
#define SINT8_MIN	SCHAR_MIN
#endif

#ifndef SINT16
#define	SINT16		signed short
#define SINT16_MAX	SHRT_MAX
#define SINT16_MIN	SHRT_MIN
#endif

#ifndef SINT32
#define SINT32		signed long
#define SINT32_MAX	LONG_MAX
#define SINT32_MIN	LONG_MIN
#endif

#ifndef DOUBLE
#define DOUBLE		double
#define DOUBLE_MAX	DBL_MAX
#define DOUBLE_MIN	DBL_MIN
#endif

//=========================================================================
// CONSTANTS
//=========================================================================

#endif /* _SH_COMMDEF_ */
#endif // _USE_TP5001_CHIP_
