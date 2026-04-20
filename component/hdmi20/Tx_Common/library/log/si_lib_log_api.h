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
* @file si_lib_log_api.h
*
* @brief
*
*****************************************************************************/

#ifndef __SI_LIB_LOG_API_H__
#define __SI_LIB_LOG_API_H__

/***** #include statements ***************************************************/

#include "si_datatypes.h"

#if (__HDMI_UBOOT__ != 1 )
#define SII_DEBUG
#endif

extern uint32_t g_Hdmi_CommonDebug;
#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#define HDMI_DEBUG_TRACE(fmt, ...) {if(g_Hdmi_CommonDebug) {printk(KERN_EMERG fmt, ##__VA_ARGS__);}}
#else
#include <stdio.h>
#define HDMI_DEBUG_TRACE(fmt, ...) {if(g_Hdmi_CommonDebug) {printf(fmt, ##__VA_ARGS__);}}
#endif

/***** public macro definitions **********************************************/

#ifdef SII_DEBUG
	extern uint8_t		g_edid_print_en;
	#define SII_LIB_LOG_DEBUG1(obj, str)      { if ( (g_edid_print_en & 0x2) == 2 ) {HDMI_SI_LOCK();SiiLibLogTimeStamp(sSiiLibObjClassStr, obj); SiiLibLogPrintf str;HDMI_SI_UNLOCK(); }}
	#define SII_LIB_LOG_DEBUG2(str)           { if ( (g_edid_print_en & 0x4) == 4 ) {HDMI_SI_LOCK();SiiLibLogPrintf str; HDMI_SI_UNLOCK();}}
#else
	#define SII_LIB_LOG_DEBUG1(obj, str)
	#define SII_LIB_LOG_DEBUG2(str)
#endif

#define SII_LIB_LOG_PRINT1(obj, str)      { HDMI_SI_LOCK(); SiiLibLogTimeStamp(sSiiLibObjClassStr, obj); SiiLibLogPrintf str; HDMI_SI_UNLOCK(); }

#ifdef SII_DEBUG
#define SII_LIB_LOG_PRINT2(str)           { HDMI_SI_LOCK(); SiiLibLogPrintf str; HDMI_SI_UNLOCK();}
#else
#define SII_LIB_LOG_PRINT2(...)           do{}while(0)//{ char *testp = "testMod";SiiLibLogTimeStamp(testp, NULL); SiiLibLogPrintf str; }
#endif

/***** public type definitions ***********************************************/

/***** public functions ******************************************************/

void SiiLibLogTimeStamp(const char* pClassStr, void* pObj);
void SiiLibLogPrintf( char* pFrm, ...);
void HDMI_SI_LOCK(void);
void HDMI_SI_UNLOCK(void);

#endif /* __SI_LIB_LOG_API_H__ */

/***** end of file ***********************************************************/
