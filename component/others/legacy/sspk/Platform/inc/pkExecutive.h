///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  pkExecutive.h
//
//  Definition of the methods and data types for the set of functions that
//  make up a portable Executive.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pkPAL.h>
#include <platThreads.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ASSERT
#define ASSERT pkASSERT
#endif

#ifdef NDEBUG
#ifndef pkCT_ASSERT
#define pkCT_ASSERT(expr) ((void)0)
#endif

#ifndef pkCT_ASSERT_TYPEISUNSIGNED
#define pkCT_ASSERT_TYPEISUNSIGNED(var) ((void)0)
#endif

#else // NDEBUG

#ifndef pkCT_ASSERT
#define pkCT_ASSERT(expr)\
    switch(0){case 0:case (expr):;} \

#endif

// To have a true compile-time assert for type (signed/unsigned), we need to use C++ 11's decltype(). The "official"
// platform-independent way to detect C++ 11 is __cplusplus > 199711L. However, VS2010 and GCC 4.3 already support
// decltype even though they are not C++ 11 compliant (__cplusplus <= 199711L). So we use a platform-defined
// macro, MSPK_PLATFORM_SUPPORTSDECLTYPE, for platform to inform us whether decltype is available for use.
// If decltype is not available (older C++ or C compiler), then we will resort to using a runtime assert so that
// at least there is *some* coverage.
#ifndef pkCT_ASSERT_TYPEISUNSIGNED
#if defined(__cplusplus) && (__cplusplus > 199711L || MSPK_PLATFORM_SUPPORTSDECLTYPE)
#define pkCT_ASSERT_TYPEISUNSIGNED(var)\
    switch(0){case 0:case ((decltype(var))-1 > 0):;} \

#else
#define pkCT_ASSERT_TYPEISUNSIGNED(var) pkASSERT(var - var - 1 > 0); // Convert to runtime assert
#endif
#endif

#endif // NDEBUG


typedef enum
{
    THREAD_CATEGORY_DEFAULT = 0,
    THREAD_CATEGORY_RTSP,
    THREAD_CATEGORY_AVCLIENT,
    THREAD_CATEGORY_TIMER,
} THREAD_CATEGORY;

void pkAPI Executive_EnableDebugOutput( bool_t bEnable );
void pkAPI Executive_DebugBreak( void );

//-----------------------------------------------------------------------------
//  Module startup/shutdown
//-----------------------------------------------------------------------------
pkRESULT Executive_Startup();

pkRESULT Executive_Shutdown();

//-----------------------------------------------------------------------------
//  Threads
//-----------------------------------------------------------------------------
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
#define Executive_CreateThread(aa,bb,cc,dd) Executive_CreateThread_Tracked(aa,bb,cc,dd,__FILE__,__FUNCTION__,__LINE__)
#define Executive_CreateThreadEx(aa,bb,cc,dd,ee) Executive_CreateThreadEx_Tracked(aa,bb,cc,dd,ee,__FILE__,__FUNCTION__,__LINE__)
pkRESULT pkAPI Executive_CreateThread_Tracked
(
    THREAD_ENTRY,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle,
    const char *szFile,
    const char *szFunc,
    int line
);

pkRESULT pkAPI Executive_CreateThreadEx_Tracked
(
    THREAD_ENTRY,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle,
    THREAD_CATEGORY threadCategory,
    const char *szFile,
    const char *szFunc,
    int line
);
#else // MCX_BUILDOPTION_MEMORYTRACKING
pkRESULT pkAPI Executive_CreateThread
(
    THREAD_ENTRY,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle
);

pkRESULT pkAPI Executive_CreateThreadEx
(
    THREAD_ENTRY,
    void *pParam,
    uint32_t stackSize,
    pkHANDLE *pHandle,
    THREAD_CATEGORY threadCategory
);
#endif // MCX_BUILDOPTION_MEMORYTRACKING

pkRESULT pkAPI Executive_SetThreadPriority( pkHANDLE hHandle, int priority );
pkRESULT pkAPI Executive_GetThreadPriority( pkHANDLE hHandle, int *pPriority );
pkRESULT pkAPI Executive_CloseThread( pkHANDLE hHandle );
pkRESULT pkAPI Executive_TerminateThread( pkHANDLE hHandle, int32_t exitCode );
pkRESULT pkAPI Executive_GetCurrentThread( pkHANDLE *phHandle );
pkRESULT pkAPI Executive_WaitForThread( pkHANDLE hHandle, uint32_t timeOutMs );
pkRESULT pkAPI Executive_GetCurrentThreadId( uint32_t *pThreadId );
pkRESULT pkAPI Executive_GetThreadId( pkHANDLE hHandle, uint32_t *pThreadId );
//-----------------------------------------------------------------------------
//  Locks
//-----------------------------------------------------------------------------
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
#define Executive_CreateLock(aa) Executive_CreateLock_Tracked(aa,__FILE__,__FUNCTION__,__LINE__);
pkRESULT pkAPI Executive_CreateLock_Tracked( pkHANDLE *pHandle, const char *szFile, const char *szFunc, int line );
#else
pkRESULT pkAPI Executive_CreateLock( pkHANDLE *pHandle );
#endif
pkRESULT pkAPI Executive_TryEnterLock( pkHANDLE hHandle );
pkRESULT pkAPI Executive_EnterLock( pkHANDLE hHandle );
pkRESULT pkAPI Executive_ExitLock( pkHANDLE hHandle );
pkRESULT pkAPI Executive_DeleteLock( pkHANDLE hHandle );
bool_t   pkAPI Executive_IsLockedByCurrentThread( pkHANDLE hHandle );

//-----------------------------------------------------------------------------
//  Semaphores
//-----------------------------------------------------------------------------
#define EXEC_WAIT_INFINITE              0xFFFFFFFF

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
#define Executive_CreateSemaphore(aa,bb,cc,dd) Executive_CreateSemaphore_Tracked(aa,bb,cc,dd,__FILE__,__FUNCTION__,__LINE__)
pkRESULT pkAPI Executive_CreateSemaphore_Tracked
(
    const char *pszIgnored,
    int32_t initCount,
    int32_t maxCount,
    pkHANDLE *pHandle,
    const char *szFile,
    const char *szFunc,
    int line
);
#else
pkRESULT pkAPI Executive_CreateSemaphore
(
    const char *pszIgnored,
    int32_t initCount,
    int32_t maxCount,
    pkHANDLE *pHandle
);
#endif
pkRESULT pkAPI Executive_WaitForSemaphore( pkHANDLE hHandle, uint32_t timeOutMs );
pkRESULT pkAPI Executive_ReleaseSemaphore( pkHANDLE hHandle, int32_t *pPrevCount );
pkRESULT pkAPI Executive_CloseSemaphore( pkHANDLE hHandle );


//-----------------------------------------------------------------------------
//  Events
//-----------------------------------------------------------------------------
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
#define Executive_CreateEvent(aa,bb,cc,dd) Executive_CreateEvent_Tracked(aa,bb,cc,dd, __FILE__, __FUNCTION__, __LINE__)
pkRESULT pkAPI Executive_CreateEvent_Tracked
(
    const char *pszIgnored,
    bool_t isManualReset,
    bool_t isSignaled,
    pkHANDLE *pHandle,
    const char *szFile,
    const char *szFunc,
    int line
);
#else
pkRESULT pkAPI Executive_CreateEvent
(
    const char *pszIgnored,
    bool_t isManualReset,
    bool_t isSignaled,
    pkHANDLE *pHandle
);
#endif
pkRESULT pkAPI Executive_SetEvent( pkHANDLE hHandle );
pkRESULT pkAPI Executive_ResetEvent( pkHANDLE hHandle );
pkRESULT pkAPI Executive_WaitForEvent( pkHANDLE hHandle, uint32_t timeOutMs );

pkRESULT pkAPI Executive_CloseEvent( pkHANDLE hHandle );

//-----------------------------------------------------------------------------
//  Time functionality
//-----------------------------------------------------------------------------
uint32_t pkAPI Executive_GetTickCount();
void     pkAPI Executive_Sleep( uint32_t msecs );
uint64_t pkAPI Executive_GetPerformanceCounter(void);
uint64_t pkAPI Executive_GetPerformanceFrequency(void);


//-----------------------------------------------------------------------------
// InterLocked Operations
//-----------------------------------------------------------------------------
int32_t pkAPI Executive_InterlockedIncrement(int32_t volatile* lpAddend);
int32_t pkAPI Executive_InterlockedDecrement(int32_t volatile* lpAddend);
int32_t pkAPI Executive_InterlockedExchange(int32_t volatile* Target, int32_t Value);
int32_t pkAPI Executive_InterlockedExchangeAdd(int32_t volatile* Addend, int32_t Value);
int32_t pkAPI Executive_InterlockedCompareExchange(int32_t volatile* Destination, int32_t Exchange, int32_t Comperand);
void *  pkAPI Executive_InterlockedExchangePointer(void * volatile* pTarget, void *pValue);
void *  pkAPI Executive_InterlockedCompareExchangePointer(void * volatile* Destination, void * Exchange, void * Comperand);


//-----------------------------------------------------------------------------
//  C Run-Time functions
//-----------------------------------------------------------------------------
#ifdef MCX_BUILDOPTION_MEMORYTRACKING
#define Executive_Alloc(aaParam,bbParam) Executive_Alloc_Tracked(aaParam,bbParam,__FILE__,__FUNCTION__,__LINE__)
#define Executive_ReAlloc(aaParam,bbParam,ccParam) Executive_ReAlloc_Tracked(aaParam,bbParam,ccParam,__FILE__,__FUNCTION__,__LINE__)
void *  pkAPI   Executive_Alloc_Tracked(uint32_t cb,  bool_t fZeroInit, const char * szFile, const char *szFunc, int line);
void *  pkAPI   Executive_ReAlloc_Tracked(void *pv, uint32_t cb, bool_t fZeroInit, const char * szFile, const char *szFunc, int line);
#else
void *  pkAPI   Executive_Alloc(uint32_t cb,  bool_t fZeroInit);
void *  pkAPI   Executive_ReAlloc(void *pv, uint32_t cb, bool_t fZeroInit);
#endif
void    pkAPI   Executive_Free(void *pv);
void    pkAPI   Executive_DebugPrintf( const char *pFmt, ... );
void    pkAPI   Executive_DebugOut( const wchar_t *pFmt);


//-----------------------------------------------------------------------------
//  Multi-byte conversion definitions and functionality
//-----------------------------------------------------------------------------
#define EXECUTIVE_CODEPAGE_ANSII     0

pkRESULT pkAPI    Executive_MultiByteToWideChar(
            uint32_t codePage,
            uint32_t flags,
            int8_t *pMultiByteStr,
            int32_t cbMultiByte,
            wchar_t *pWideCharStr,
            int32_t cchWideChar,
            uint32_t *pWritten);
pkRESULT pkAPI    Executive_WideCharToMultiByte(
            uint32_t codePage,
            uint32_t flags,
            const wchar_t *pWideCharStr,
            int32_t cchWideChar,
            int8_t *pMultiByteStr,
            int32_t cbMultiByte,
            int8_t *pDefaultChar,
            bool_t *pUsedDefaultChar,
            uint32_t *pWritten);

#ifdef __cplusplus
}
#endif

