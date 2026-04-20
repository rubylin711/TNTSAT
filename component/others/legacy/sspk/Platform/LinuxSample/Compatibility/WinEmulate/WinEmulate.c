///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
**
**  WinEmulate.c
**
**  WinEmulate implementation.
**
*******************************************************************************/

#include "pkPAL.h"
#include "pkExecutive.h"

#include <sys/time.h>

int MultiByteToWideChar(
    UINT CodePage,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar )
{
    uint32_t uiWritten = 0;

    pkRESULT hr =
        Executive_MultiByteToWideChar
            (
                (uint32_t) CodePage,
                (uint32_t) dwFlags,
                (int8_t *) lpMultiByteStr,
                (int32_t) cbMultiByte,
                (wchar_t *) lpWideCharStr,
                (int32_t) cchWideChar,
                &uiWritten
            );

    return( pkSUCCEEDED(hr) ? (int) uiWritten : 0 );
}

int WideCharToMultiByte(
    UINT CodePage,
    DWORD dwFlags,
    LPCWSTR lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cbMultiByte,
    LPCSTR lpDefaultChar,
    LPBOOL lpUsedDefaultChar )
{
    uint32_t uiWritten = 0;

    pkRESULT hr =
        Executive_WideCharToMultiByte
            (
                (uint32_t) CodePage,
                (uint32_t) dwFlags,
                (const wchar_t *) lpWideCharStr,
                (int32_t) cchWideChar,
                (int8_t *) lpMultiByteStr,
                (int32_t) cbMultiByte,
                (int8_t *) lpDefaultChar,
                (bool_t *) lpUsedDefaultChar,
                &uiWritten
            );

    return( pkSUCCEEDED(hr) ? (int) uiWritten : 0 );
}


void CoFreeUnusedLibraries( void )
{
    // DO NOTHING
}

void WINAPI Sleep ( DWORD milliseconds )
{
    Executive_Sleep(milliseconds);
}

BOOL WINAPI GetThreadTimes(
    HANDLE hThread,
    FILETIME* lpCreationTime,
    FILETIME* lpExitTime,
    FILETIME* lpKernelTime,
    FILETIME* lpUserTime
)
{
    // TODO: NOT YET IMPLEMENTED
    return FALSE;
}

DWORD GetIdleTime(void) // in milliseconds
{
    // TODO: NOT YET IMPLEMENTED
    return 0;
}

HANDLE WINAPI GetCurrentThread(void)
{
    pkHANDLE pkHandle;

    pkRESULT hr = Executive_GetCurrentThread( &pkHandle );

    return( pkSUCCEEDED(hr) ? (HANDLE)pkHandle : NULL );
}

LONG WINAPI InterlockedIncrement(LONG volatile *Addend)
{

    return (LONG)Executive_InterlockedIncrement((int32_t volatile*) Addend);
}

LONG WINAPI InterlockedDecrement(LONG volatile *Addend)
{
    return (LONG)Executive_InterlockedDecrement((int32_t volatile*) Addend);
}

LONG WINAPI InterlockedCompareExchange
(
    LONG volatile *Destination,
    LONG Exchange,
    LONG Comparand
)
{
    return (LONG)Executive_InterlockedCompareExchange
        (
            (int32_t volatile*) Destination,
            (int32_t) Exchange,
            (int32_t) Comparand
        );
}

LONG WINAPI InterlockedExchange
(
    LONG volatile *Target,
    LONG Value
)
{
    return (LONG)Executive_InterlockedExchange((int32_t volatile*) Target, (int32_t) Value);
}

LONG WINAPI InterlockedExchangeAdd
(
    LONG volatile *Target,
    LONG Value
)
{
    return (LONG)Executive_InterlockedExchangeAdd((int32_t volatile*) Target, (int32_t) Value);
}


void WINAPI GetSystemTimeAsFileTime(FILETIME *pFileTime)
{
    static const uint64_t hns_FILETIME_1970 = 0x019db1ded53e8000LL; // 100 ns units
                            // 11644473600 seconds or 134774 days

    struct timeval tvNow;

    int rc = gettimeofday(&tvNow, NULL);

    if ( 0 == rc )
    {
        uint64_t hns =
            (10000000LL * (uint32_t)tvNow.tv_sec)
            + (10 * (uint32_t)tvNow.tv_usec)
            + hns_FILETIME_1970;

        pFileTime->dwHighDateTime = (DWORD)(hns >> 32);
        pFileTime->dwLowDateTime = (DWORD)hns;
    }
    else
    {
        pFileTime->dwHighDateTime = 0;
        pFileTime->dwLowDateTime = 0;
    }
}



#define kPERF_FREQUENCY 1000000LL // 1 MHz to match the microsecond resolution in the timeval

BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    lpPerformanceCount->QuadPart = (LONGLONG)((tv.tv_sec * kPERF_FREQUENCY) + tv.tv_usec);
    return TRUE;
}

BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
    lpFrequency->QuadPart = kPERF_FREQUENCY;
    return TRUE;
}



