///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// emulation of Windows APIs

// These are implemented in PALimpl/Executive

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define CP_ACP  0 // ANSII code page

int MultiByteToWideChar(
    UINT CodePage,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar );

int WideCharToMultiByte(
    UINT CodePage,
    DWORD dwFlags,
    LPCWSTR lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cbMultiByte,
    LPCSTR lpDefaultChar,
    LPBOOL lpUsedDefaultChar );


void CoFreeUnusedLibraries( void );

void WINAPI Sleep ( DWORD milliseconds );

BOOL WINAPI GetThreadTimes(
    HANDLE hThread,
    FILETIME* lpCreationTime,
    FILETIME* lpExitTime,
    FILETIME* lpKernelTime,
    FILETIME* lpUserTime
);

DWORD GetIdleTime(void); // in milliseconds

HANDLE WINAPI GetCurrentThread(void);

LONG WINAPI InterlockedIncrement(LONG volatile *Addend);

LONG WINAPI InterlockedDecrement(LONG volatile *Addend);

LONG WINAPI InterlockedCompareExchange
(
    LONG volatile *Destination,
    LONG Exchange,
    LONG Comparand
);

LONG WINAPI InterlockedExchange
(
    LONG volatile *Target,
    LONG Value
);

LONG WINAPI InterlockedExchangeAdd
(
    LONG volatile *Target,
    LONG Value
);


void WINAPI GetSystemTimeAsFileTime(FILETIME *lpSystemTimeAsFileTime);

BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);

BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);



#ifdef __cplusplus
}
#endif


