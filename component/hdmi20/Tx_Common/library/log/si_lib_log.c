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
* @file si_lib_log.c
*
* @brief
*
*****************************************************************************/

/***** #include statements ***************************************************/

#include "mt_hdmi20_cfg.h"
#if ( __HDMI_OS_LINUX__ || __HDMI_UBOOT__)
	#include <stdio.h>
	#include <stdarg.h>
	#if ( __HDMI_UBOOT__ != 1 )
	#include <unistd.h>
#endif
#endif

#include "si_datatypes.h"
#include "si_lib_log_api.h"
#include "si_lib_time_api.h"
#include "platform_api.h"
#include "sii_time.h"
#include "si_lib_obj_api.h"
#if (__HDMI_OS_LINUX__)
	#include "conio.h"
#endif

#if __HDMI_OS_KERNEL__
#include "mt_kernel_adapt.h"
#include "drv_hdmi_ext.h"
#endif

/***** Register Module name **************************************************/

//SII_LIB_OBJ_MODULE_DEF(lib_log);

/***** local macro definitions ***********************************************/

#define LOG_LINE_WRAP              (80)
#if (MT_SDK_COMPILE_HDMI20)
	#define LOG_FILE_WRITE            (0)
#endif

#if __HDMI_OS_KERNEL__
MT_DECLARE_MUTEX(g_HDMISIMutex);
void HDMI_SI_LOCK(void)
{
	do{
		if (!in_atomic()){
			if (down_interruptible(&g_HDMISIMutex))
				;
		}
	}while(0);
}
void HDMI_SI_UNLOCK(void)
{
	if (!in_atomic()){
		do{
			up(&g_HDMISIMutex);
		}while(0);
	}
}
#elif __HDMI_OS_LINUX__
uint16_t  g_fake_mutex_si = 0;  //used only for print
void HDMI_SI_LOCK(void)
{
	uint16_t tout = 10000;
	if (g_fake_mutex_si==0) {
		g_fake_mutex_si = 1;
	} else {
		while(g_fake_mutex_si == 1 && tout > 0) {
			usleep(1);
			tout--;
		}
		g_fake_mutex_si = 1;
	}
}
void HDMI_SI_UNLOCK(void)
{
	g_fake_mutex_si = 0;
}
#else
void HDMI_SI_LOCK(void)
{
	;
}
void HDMI_SI_UNLOCK(void)
{
	;
}
#endif

/***** local type definitions ************************************************/

/***** local prototypes ******************************************************/

static uint_t sLogLimited( uint_t size, const char* pStr );
static void sLogPutString( char* pStr );

/***** local data objects ****************************************************/

static uint16_t sLinePos = 0;

static uint16_t vspfunc(char *buffer, char *format, ...)
{
   va_list aptr;
   uint16_t ret;

   va_start(aptr, format);
   ret = vsprintf(buffer, format, aptr);
   va_end(aptr);

   return(ret);
}

/***** public functions ******************************************************/
void SiiLibLogTimeStamp(const char* pClassStr, void* pObj)
{
	SiiLibTimeMilli_t mSec = SiiLibTimeMilliGet();
	uint16_t tot  = 0;
	char StrTmp[256];
	uint16_t len;
	uint16_t offset = 0;
	uint16_t offtime = 0;

	/* Print time stamp */
	memset(StrTmp,0,sizeof(StrTmp));
	//SII_PRINTF((SI_LOG_LEVEL_STRING "\n%ld.%03ld-", (ulong)mSec / 1000, (ulong)mSec % 1000));
	offtime = vspfunc(StrTmp,"\n%ld.%03ld-", (ulong)mSec / 1000, (ulong)mSec % 1000);

	/* Print module name */
	len = (uint16_t)sLogLimited(12, pClassStr);
	if (len) {
		memcpy(StrTmp+tot+offtime,pClassStr,len);
	}
	tot += len;

	/* If instance print instance name */
	if ( pObj ) {
		/* Separation character */
		//SII_PRINTF((SI_LOG_LEVEL_STRING "."));
		StrTmp[tot+offtime] = '.';
		tot++;

		//tot += (uint16_t)sLogLimited(12, SII_LIB_OBJ_NAME_INSTANCE(pObj));
		len = (uint16_t)sLogLimited(12, SII_LIB_OBJ_NAME_INSTANCE(pObj));
		if (len) {
			memcpy(StrTmp+tot+offtime,SII_LIB_OBJ_NAME_INSTANCE(pObj),len);
		}
		tot += len;
	}

	/* Print alignment space characters */
	offset = tot;
	tot = (24 < tot) ? (0) : (24 - tot);
	while ( tot-- ) {
		//SII_PRINTF((SI_LOG_LEVEL_STRING " "));
		StrTmp[offset+offtime] = ' ';
		offset++;
	}

	/* Print end of preamble */
	StrTmp[offset+offtime] = ':';
	offset++;
	StrTmp[offset+offtime] = ' ';
	offset++;
	//SII_PRINTF((SI_LOG_LEVEL_STRING ": "));
	SII_PRINTF((SI_LOG_LEVEL_STRING "%s",StrTmp));

	sLinePos = 29;
}

void SiiLibLogPrintf( char* pFrm, ...)
{
	va_list  arg;
	uint16_t chars = 0;
	char     str[160]; // CEC_LOGGER requires 160
	va_start(arg, pFrm);
	chars = (uint16_t)SII_VSPRINTF(str, pFrm, arg);
	va_end(arg);
	SII_PLATFORM_DEBUG_ASSERT(((int)sizeof(str)) > chars);
	sLogPutString(str);
}

/***** local functions *******************************************************/

static uint_t sLogLimited( uint_t size, const char* pStr )
{
	uint_t i;

	for ( i = 0; i < size; i++ ) {
		/* Check for end of string */
		if ( !(*pStr) ) {
			break;
		}

		//SII_PRINTF((SI_LOG_LEVEL_STRING "%c", *pStr));
		pStr++;
	}
	return i;
}

static void sLogPutString( char* pStr )
{
	char StrTmp[168];
	uint16_t offset = 0;
	memset(StrTmp,0,sizeof(StrTmp));
	while ( *pStr ) {
		if ( *pStr == '\n' ) {
			sLinePos = 0;
		} else {
			/* Apply indent */
			if ( 0 == sLinePos ) {
				SII_PRINTF((SI_LOG_LEVEL_STRING "\n    "));
			} else if ( LOG_LINE_WRAP < sLinePos ) {
				SII_PRINTF((SI_LOG_LEVEL_STRING "\n    %s",StrTmp));
				memset(StrTmp,0,sizeof(StrTmp));
				offset = 0;
				sLinePos = 0;
			}
			//SII_PRINTF((SI_LOG_LEVEL_STRING "%c", *pStr));
			StrTmp[offset++] = *pStr;
			sLinePos++;
		}
		pStr++;
	}
	SII_PRINTF((SI_LOG_LEVEL_STRING "%s",StrTmp));
}

/***** end of file ***********************************************************/
