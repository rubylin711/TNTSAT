/******************************************************************************
 *
 * Copyright 2007-2013, Deco, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of
 * Deco, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * @file
 *
 * @brief Deco common types
 *
 *****************************************************************************/

#ifndef SI_DATATYPES_H
#define SI_DATATYPES_H

/***** #include statements ***************************************************/
#include "mt_hdmi20_cfg.h"
#if (1 == __HDMI_OS_RTOS__)
#include "sys_types.h"
#elif (1 == __HDMI_UBOOT__)
#else
#include "mt_type.h"
#endif

#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__ || __HDMI_OS_RTOS__)
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
#if __HDMI_OS_RTOS__
	#include "mtos_printk.h"
#endif
#else
	#include <linux/printk.h>
	#include <linux/string.h>
	#include <linux/kernel.h>
#endif
#if __HDMI_UBOOT__
	#include <vsprintf.h>
#elif (__HDMI_OS_LINUX__)
	#include <stdint.h>
#endif

/***** public macro definitions **********************************************/

#define MT_SDK_COMPILE_HDMI20      (1)

#ifndef ENABLE
#if __HDMI_OS_RTOS__
	/*****************************************************************************/
	/** @defgroup SII_ENABLE_DISABLE Enable/disable definitions
	* @brief Enable/disable definitions used in definitions
	*
	******************************************************************************/
	/* @{ */
	#define ENABLE      (1)
	#define DISABLE     (0)
	/* @} */
#endif
#endif // ENABLE

#define SII_MEMCPY(pdes, psrc, size)     memcpy(pdes, psrc, size)
#define SII_MEMCMP(pdes, psrc, size)     memcmp(pdes, psrc, size)
#define SII_MEMSET(pdes, value, size)    memset(pdes, value, size)
#define SII_STRCPY(pdes, psrc)           strcpy(pdes, psrc)
#define SII_STRCMP(pdes, psrc)           strcmp(pdes, psrc)
#define SII_STRLEN(pstr)                 strlen(pstr)
#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
	#define SII_VSPRINTF(dstr, fstr, arg)    vsprintf(dstr, fstr, arg)
	#define SII_SPRINTF(arg)                 sprintf arg                  // use macro as: SII_SPRINTF((dstr, "%d", i));
	#define SII_PRINTF(arg)                  printf arg                   // use macro as: SII_PRINTF(("%d", i));
#else
	#define SII_VSPRINTF(dstr, fstr, arg)    vsprintf(dstr, fstr, arg)
	#define SII_SPRINTF(arg)                 sprintf arg                  // use macro as: SII_SPRINTF((dstr, "%d", i));
#if __HDMI_OS_RTOS__
	#define SII_PRINTF(arg)                  mtos_printk arg                   // use macro as: SII_PRINTF(("%d", i));			//rtos need redo
#else
	#define SII_PRINTF(arg)                  printk arg                   // use macro as: SII_PRINTF(("%d", i));
#endif
#endif

#define SII_DEF_PI                       3.14159265358979323846f

#ifndef BIT0
	/*****************************************************************************/
	/** @defgroup SII_BIT_DEFINITIONS Generic bit definitions
	* @brief Generic bit definitions
	*
	******************************************************************************/
	/* @{ */
	#define SII_BIT0                         0x01
	#define SII_BIT1                         0x02
	#define SII_BIT2                         0x04
	#define SII_BIT3                         0x08
	#define SII_BIT4                         0x10
	#define SII_BIT5                         0x20
	#define SII_BIT6                         0x40
	#define SII_BIT7                         0x80
	#define SII_BIT8                         0x0100
	#define SII_BIT9                         0x0200
	#define SII_BIT10                        0x0400
	#define SII_BIT11                        0x0800
	#define SII_BIT12                        0x1000
	#define SII_BIT13                        0x2000
	#define SII_BIT14                        0x4000
	#define SII_BIT15                        0x8000

	#define SET_BITS    0xFF
	#define CLEAR_BITS  0x00

	/* @} */
#endif // BIT0

#ifndef NULL
	#define NULL       ((void*)0)
#endif // NULL

#ifndef INST_NULL
	#define INST_NULL  ((SiiInst_t)0)
#endif // INST_NULL

#ifndef PACKED
	//#define PACKED // 8 bit microcontroller version
	#define PACKED __attribute__  ((packed)) // GCC version
#endif // PACKED

#ifndef SII_INST2OBJ
	#define SII_INST2OBJ(inst)   ((void*)inst)
	#define SII_OBJ2INST(pObj)   ((SiiInst_t)pObj)
#endif // SII_INST2OBJ

#ifndef ON
	#define ON  true
	#define OFF false
#endif

#ifndef TRUE
	#define TRUE  true
	#define FALSE false
#endif

#define ROM const      // ROM memory
#define XDATA      // 8051 type of external memory

#define ABS_DIFF(A, B) ((A>B) ? (A-B) : (B-A))

// Test chain macro
// Note: "switch" operator must be used with caution as PASS_IF includes "break"
#define BEGIN_TEST do{
#define PASS_IF(a) {if(!(a)) break;}
#define FAIL_IF(a) {if(a) break;}
#define END_TEST }while(false);

/* Bit manipulation macros */
#define SII_SET_BIT(p, bit)     ( *(p) |= (1<<(bit)) )
#define SII_CLR_BIT(p, bit)     ( *(p) &= (~(1<<(bit))) )
#define SII_PUT_BIT(p, bit, b)  ( *(p) = (b) ? (*(p)|(1<<(bit))) : (*(p)&(~(1<<(bit)))) )

/***** public type definitions ***********************************************/
typedef signed int     int_t;
typedef unsigned int   uint_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int    int32_t;
typedef int            prefuint_t; // Platform-specific efficient integers

#if __HDMI_OS_RTOS__
#ifndef bool
typedef unsigned char bool;
#endif

#ifndef mt_void
#define mt_void                 void
#endif

#ifndef BOOL
typedef int                     BOOL;
#endif
#ifndef MT_BOOL
#define MT_BOOL                 BOOL
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#ifndef MT_SUCCESS
#define MT_SUCCESS      (0)
#endif

#ifndef mt_u32
#define mt_u32      uint32_t
#endif

#define MT_FALSE                0
#define MT_TRUE                 1

typedef signed char             mt_s8;
typedef unsigned char           mt_u8;
typedef char                    mt_char;
typedef unsigned char           mt_uchar;
typedef signed short            mt_s16;
typedef unsigned short          mt_u16;
typedef signed int              mt_s32;
//typedef unsigned int            mt_u32;
typedef unsigned long long      mt_u64;
typedef char*                   mt_pchar;
typedef unsigned char           MT_U8;
typedef signed int              MT_S32;
typedef unsigned int            MT_U32;
//#define MT_SUCCESS              0
//#define MT_FAILURE             (-1)
#if __HDMI_OS_RTOS__
#define MT_FAILURE             (-1)
#define EDID_INFO(fmt...)           //{printf(fmt);}
#define EDID_WARN(fmt...)           //{printf(fmt);}
#define EDID_ERR(fmt...)           //{printf(fmt);}
#endif
#endif

#ifdef __cplusplus
typedef MT_BOOL bool_t;
#else
#if (__HDMI_OS_LINUX__)
/**
* @brief C++ -like Boolean type
*/
typedef enum {
	false_t   = 0,
	true_t    = !(false_t)
} bool_t;
#elif (__HDMI_UBOOT__)
typedef enum {
	false_t   = 0,
	true_t    = !(false_t)
} bool_t;
#elif (__HDMI_OS_RTOS__)
typedef BOOL bool_t;
#else
typedef MT_BOOL bool_t;
#endif

#endif // __cplusplus

/**
* @brief Type to use with bitfields
*/
//typedef unsigned char bit_fld_t;
// gcc -pedantic does not allow bit field operation, so to avoid the warning, we need the following trick
typedef unsigned char   bit_fld_t;    // bit field type used in structures
typedef unsigned short  bit_fld16_t;
// #define bit_fld_t       __extension__ unsigned char
// #define bit_fld16_t     __extension__ unsigned short

/***** public functions ******************************************************/
#if ( __HDMI_OS_KERNEL__ )
#endif

#endif // SI_DATATYPES_H
