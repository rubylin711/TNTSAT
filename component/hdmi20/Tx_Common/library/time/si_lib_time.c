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
* @file si_lib_time.c
*
* @brief Time library
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "platform_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_time_api.h"

/***** Register Module name **************************************************/

//SII_LIB_OBJ_MODULE_DEF(lib_time);

/***** local macro definitions ***********************************************/

#define MILLI_TO_MAX          (((SiiLibTimeMilli_t)~0)>>1)  /* Maximum milli out must be set to less than half the range of SiiSysTimeMilli_t */

/***** local type definitions ************************************************/

/***** local prototypes ******************************************************/

/***** local data objects ****************************************************/

/***** local functions ******************************************************/
static void sTimeOutMilliSet( SiiLibTimeMilli_t* pMilliTO, SiiLibTimeMilli_t timeOut );
static bool_t sTimeOutMilliIs( const SiiLibTimeMilli_t* pMilliTO );
static bool_t sTimeOutIsExpired( const SiiLibTimeMilli_t* pMilliTO, SiiLibTimeMilli_t timeout );

SiiLibTimeMilli_t SiiLibTimeMilliGet( void )
{
	return (SiiLibTimeMilli_t)SiiPlatformTimeMilliGet();
}

void SiiLibTimeMilliDelay( uint32_t millDelay )
{
	SiiPlatformTimeMilliDelay(millDelay);
}

void SiiLibTimeOutMilliSet( SiiLibTimeMilli_t* pMilliTO, SiiLibTimeMilli_t timeOut )
{
	sTimeOutMilliSet(pMilliTO, timeOut);
}

bool_t SiiLibTimeOutMilliIs( const SiiLibTimeMilli_t* pMilliTO )
{
	return sTimeOutMilliIs(pMilliTO);
}

static void sTimeOutMilliSet( SiiLibTimeMilli_t* pMilliTO, SiiLibTimeMilli_t timeOut )
{
	SII_PLATFORM_DEBUG_ASSERT(MILLI_TO_MAX > timeOut);
	*pMilliTO = SiiLibTimeMilliGet() + timeOut;
}

static bool_t sTimeOutMilliIs( const SiiLibTimeMilli_t* pMilliTO )
{
	SiiLibTimeMilli_t milliNew = SiiLibTimeMilliGet();
	SiiLibTimeMilli_t milliDif = (*pMilliTO > milliNew) ? (*pMilliTO - milliNew) : (milliNew - *pMilliTO);

	if ( MILLI_TO_MAX < milliDif ) {
		return (*pMilliTO >  milliNew) ? (true) : (false);
	} else {
		return (*pMilliTO <= milliNew) ? (true) : (false);
	}
}

bool_t SiiLibTimeOutIsExpired(const SiiLibTimeMilli_t* pMilliTO, SiiLibTimeMilli_t timeout)
{
	return sTimeOutIsExpired(pMilliTO, timeout);
}

static bool_t sTimeOutIsExpired(const SiiLibTimeMilli_t* pMilliTO, SiiLibTimeMilli_t timeout)
{
	SiiLibTimeMilli_t milliNew = SiiLibTimeMilliGet();
	SiiLibTimeMilli_t milliDif = (*pMilliTO > milliNew) ? (*pMilliTO - milliNew) : (milliNew - *pMilliTO);

	//Todo. Check if it is working fine.
	//if( MILLI_TO_MAX < milliDif )
	return (milliDif < timeout) ? (false) : (true);
}

void SiiLibTimeMicroDelay( uint32_t microDelay )
{
	SiiPlatformTimeMicroDelay(microDelay);
}
/***** end of file ***********************************************************/
