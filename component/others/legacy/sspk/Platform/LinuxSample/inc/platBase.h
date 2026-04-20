///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "platTypes.h"

#ifndef MSPK_IN_PAL_IMPL // PAL implmentations use only OS headers and standard C libs
#include "wintypes.h"
#include "stdCrtNeeded.h"
#include "platSafeStdLib.h" // Always included even though normally only part of std C libs
#include <zlib.h>
#include <stdlib.h>
#endif

#include <assert.h>

#define _INTPTR_T_DEFINED
#define pkAPI

#define pkHIBYTE(x) (uint8_t)(((uint16_t)x) >> 8)
#define pkLOBYTE(x) (uint8_t)(((uint16_t)x) & 0xFF)

/* defines */
#define pkASSERT assert

#ifdef PRINTMSG_ENABLED
#define TV2INTERNAL 1
#endif

#include "platVersion.h"

#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#define MSPK_PLATFORM_SUPPORTSDECLTYPE  1   // GCC supports decltype starting from version 4.3 even though __cplusplus <= 199711L. Need to specify -std=c++0x
#endif
