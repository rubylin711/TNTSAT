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
* @file si_lib_malloc.c
*
* @brief Dynamic memory allocation from static memory pool
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "platform_api.h"
#include "si_lib_obj_api.h"

/***** Register Module name **************************************************/

//SII_LIB_OBJ_MODULE_DEF(lib_malloc);

/***** local macro definitions ***********************************************/
#if 1
	//#define MEMPOOL_BITSIZE_IN_BYTES (sizeof(ulong))
	#if defined (CONFIG_MT_CHIP_SYMPHONY6)
		#ifdef __KERNEL__
			#ifdef CONFIG_MT_64BIT_KMODE
				#define MEMPOOL_BITSIZE_IN_BYTES (8)
			#else
				#define MEMPOOL_BITSIZE_IN_BYTES (4)
			#endif
		#elif __HDMI_OS_RTOS__
			#define MEMPOOL_BITSIZE_IN_BYTES (4)
		#else
			#ifdef CONFIG_MT_64BIT_MODE
				#define MEMPOOL_BITSIZE_IN_BYTES (8)
			#else
				#define MEMPOOL_BITSIZE_IN_BYTES (4)
			#endif
		#endif
	#elif defined (CONFIG_MT_CHIP_ETUDE2)
		#define MEMPOOL_BITSIZE_IN_BYTES (4)
	#else
		#define MEMPOOL_BITSIZE_IN_BYTES (4)
	#endif
#endif
#define MEMPOOL_BITSIZE_REMAINDER	(MEMPOOL_BITSIZE_IN_BYTES - 1)
#if (MEMPOOL_BITSIZE_IN_BYTES == 8)
	#define MEMPOOL_BITSIZE_SHIFT_BITS	(3)
#elif (MEMPOOL_BITSIZE_IN_BYTES == 4)
	#define MEMPOOL_BITSIZE_SHIFT_BITS	(2)
#else
	#define MEMPOOL_BITSIZE_SHIFT_BITS	(2)
#endif
#define MEMPOOL_SIZE_IN_BYTES   (0x2000)
#define MEMPOOL_SIZE_IN_WORDS   ((MEMPOOL_SIZE_IN_BYTES/MEMPOOL_BITSIZE_IN_BYTES)+MEMPOOL_BITSIZE_IN_BYTES/2)

/***** local prototypes ******************************************************/

static void sMemoryClear( uint_t size, ulong ptr );
void* SiiLibMallocCreate( uint_t size );
void SiiLibMallocDelete( void* p );

/***** local data objects ****************************************************/

static ulong  sMemPool[MEMPOOL_SIZE_IN_WORDS]; /* Ensure memory pool location at 4-byte boundary */
static ulong	sPtr     = (ulong)NULL;                    /* Pointer to next avaialble memory location      */
#if SII_ENV_BUILD_ASSERT
	static bool_t    sbLock   = false;
#endif

/***** public functions ******************************************************/

void* SiiLibMallocCreate( uint_t size )
{
	uint_t words = (size & MEMPOOL_BITSIZE_REMAINDER) ? ((size >> MEMPOOL_BITSIZE_SHIFT_BITS) + 1) : (size >> MEMPOOL_BITSIZE_SHIFT_BITS); /* Round up to nearest number of words (1 word = 8 bytes) */

	/* Check if memory pool is locked */
	SII_PLATFORM_DEBUG_ASSERT(!sbLock);

	if ( MEMPOOL_SIZE_IN_WORDS > (sPtr + words) ) {
		ulong ptr = sPtr;

		/* Clear memory */
		sMemoryClear(words, sPtr);

		/* Increase pointer to next available memory location */
		sPtr += words;

		SI_HDMI20_PRINT("\n[%s_%d]size:%d,words=%d,ptr=0x%p,tail=0x%p,sulong:%d\n", __func__, __LINE__, size, words, (void *)ptr, (void *)sPtr,(int)sizeof(ulong));
		return (void *)(sMemPool + ptr);
	} else {
		SII_PLATFORM_DEBUG_ASSERT(0);
		SI_HDMI20_PRINT("\n[%s_%d]alloc size:%d,words=%d fail >_<\n", __func__, __LINE__, size, words);
		return NULL;
	}
}

#if 0 // not used
uint_t SiiLibMallocBytesAllocatedGet( void )
{
	/* Return total amound of bytes allocated */
	return (uint_t)(sPtr << 2);
}
#endif

void SiiLibMallocDelete( void* p )
{
	/* Make sure that delete pointer is in allocated memory space */
	SII_PLATFORM_DEBUG_ASSERT((ulong*)p >= &sMemPool[0]);
	SII_PLATFORM_DEBUG_ASSERT((ulong*)p <  &sMemPool[sPtr]);

	/* Make sure that delete pointer is on a even 8 byte boundary */
	SII_PLATFORM_DEBUG_ASSERT(!((((uint8_t*)p) - ((uint8_t*)&sMemPool[0])) % MEMPOOL_BITSIZE_IN_BYTES));

	sPtr = (ulong)((((uint8_t*)p) - ((uint8_t*)&sMemPool[0])) >> MEMPOOL_BITSIZE_SHIFT_BITS);
	SI_HDMI20_PRINT("\n[%s_%d]delete:0x%p,tail=0x%p\n", __func__, __LINE__, p, (void *)sPtr);
}

#if 0 // not used
void SiiLibMallocLock( void )
{
	sbLock = true;
}

void SiiLibMallocDeleteAll( void )
{
	sbLock = false;
	sPtr   = 0;
}
#endif

/***** local functions *******************************************************/

static void sMemoryClear( uint_t size, ulong ptr )
{
	ulong* pData = &sMemPool[ptr];

	while ( size-- ) {
		*pData = 0UL;
		pData++;
	}
}

/***** end of file ***********************************************************/
