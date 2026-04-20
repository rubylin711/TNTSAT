///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// This implementation assume the use of GCC

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#include <stdint.h> // sized integer types for 8, 16, 32, and 64 bits

#include <stddef.h> // includes wchar_t

// Boolean

typedef unsigned long bool_t;

#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

// Generic handle

typedef void* pkHANDLE;

// Generic result

typedef int32_t pkRESULT;
