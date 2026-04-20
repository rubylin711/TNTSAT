///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <platBase.h>
#include "pkSALDefaults.h"
#include <platGuids.h>
#include <platTypes.h>

#include <pkResult.h>
#ifndef MCX_BUILDOPTION_MEMORYTRACKING

#ifdef __cplusplus
#include <new>
#define NEW_NO_THROW new(std::nothrow)
#endif // __cplusplus

#else

#ifdef __cplusplus
extern void * operator new(size_t size, const char *file, const char *func, int line);
extern void * operator new[](size_t size, const char *file, const char *func, int line);
extern void operator delete(void *p);
extern void operator delete[](void *p);
#define NEW_NO_THROW new
#define new new(__FILE__, __FUNCTION__, __LINE__)
extern "C" {
#endif // __cplusplus

#define malloc(size) malloc_Tracked(size, __FILE__, __FUNCTION__, __LINE__)
#define free(pointer) free_Tracked(pointer, __FILE__, __FUNCTION__, __LINE__)

extern void * malloc_Tracked(size_t size, const char *szFile, const char *szFunc, int line);
extern void free_Tracked(void *p, const char *szFile, const char *szFunc, int line);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MCX_BUILDOPTION_MEMORYTRACKING

