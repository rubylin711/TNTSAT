///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
**
**  strsafe.h
**
**  Definition of the StringSafe APIs.
**
*******************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h> // valist

// TODO: convert from STRINGSAFE_ to STRSAFE and eliminate duplicates?

typedef  char* STRINGSAFE_LPSTR;
typedef  const char* STRINGSAFE_LPCSTR;

typedef  wchar_t* STRINGSAFE_LPWSTR;
typedef  const wchar_t* STRINGSAFE_LPCWSTR;

#define STRINGSAFE_API    HRESULT

#define STRINGSAFE_IGNORE_NULLS                            0x00000100  // treat null string pointers as TEXT("") -- don't fault on NULL buffers
#define STRINGSAFE_MAX_CCH  2147483647                                 // max # of characters we support (same as INT_MAX)
#define STRINGSAFE_MAX_LENGTH  (STRINGSAFE_MAX_CCH - 1)                // max buffer length, in characters, that we support

// ==============================================================

// The user may override STRSAFE_MAX_CCH, but it must always be less than INT_MAX
#ifndef STRSAFE_MAX_CCH
#define STRSAFE_MAX_CCH     2147483647  // max buffer size, in characters, that we support (same as INT_MAX)
#endif

#define STRSAFE_MAX_LENGTH  (STRSAFE_MAX_CCH - 1)   // max buffer length, in characters, that we support


// Flags for controling the Ex functions
//
//      STRSAFE_FILL_BYTE(0xFF)                         0x000000FF  // bottom byte specifies fill pattern
#define STRSAFE_IGNORE_NULLS                            0x00000100  // treat null string pointers as TEXT("") -- don't fault on NULL buffers
#define STRSAFE_FILL_BEHIND_NULL                        0x00000200  // on success, fill in extra space behind the null terminator with fill pattern
#define STRSAFE_FILL_ON_FAILURE                         0x00000400  // on failure, overwrite pszDest with fill pattern and null terminate it
#define STRSAFE_NULL_ON_FAILURE                         0x00000800  // on failure, set *pszDest = TEXT('\0')
#define STRSAFE_NO_TRUNCATION                           0x00001000  // instead of returning a truncated result, copy/append nothing to pszDest and null terminate it

#define STRSAFE_VALID_FLAGS                     (0x000000FF | STRSAFE_IGNORE_NULLS | STRSAFE_FILL_BEHIND_NULL | STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION)

// helper macro to set the fill character and specify buffer filling
#define STRSAFE_FILL_BYTE(x)                    ((DWORD)((x & 0x000000FF) | STRSAFE_FILL_BEHIND_NULL))
#define STRSAFE_FAILURE_BYTE(x)                 ((DWORD)((x & 0x000000FF) | STRSAFE_FILL_ON_FAILURE))

#define STRSAFE_GET_FILL_PATTERN(dwFlags)       ((int)(dwFlags & 0x000000FF))


// error return codes
#define STRSAFE_E_INSUFFICIENT_BUFFER           ((HRESULT)0x8007007AL)  // 0x7A = 122L = ERROR_INSUFFICIENT_BUFFER
#define STRSAFE_E_INVALID_PARAMETER             ((HRESULT)0x80070057L)  // 0x57 =  87L = ERROR_INVALID_PARAMETER
#define STRSAFE_E_END_OF_FILE                   ((HRESULT)0x80070026L)  // 0x26 =  38L = ERROR_HANDLE_EOF

//
// These typedefs are used in places where the string is guaranteed to
// be null terminated.
//
typedef char* STRSAFE_LPSTR;
typedef const char* STRSAFE_LPCSTR;
typedef wchar_t* STRSAFE_LPWSTR;
typedef const wchar_t* STRSAFE_LPCWSTR;

// ==============================================================



#ifdef UNICODE
#define StringCchLength  StringCchLengthW
#else
#define StringCchLength  StringCchLengthA
#endif // !UNICODE



STRINGSAFE_API
StringCchLengthA(
    __in_ecount(cchMax) STRINGSAFE_LPCSTR psz,
    size_t cchMax,
    __out size_t* pcchLength
);

STRINGSAFE_API
StringCchLengthW(
    __in_ecount(cchMax) STRINGSAFE_LPCWSTR psz,
    size_t cchMax,
    __out size_t* pcchLength
);

#ifndef COUNTOF
#define COUNTOF( x )        ( sizeof(x) / sizeof( (x)[0] ) )
#endif

#ifdef UNICODE
#define StringCchCopy  StringCchCopyW
#define StringCchCopyN  StringCchCopyNW
#else
#define StringCchCopy  StringCchCopyA
#define StringCchCopyN  StringCchCopyNA
#endif // !UNICODE

STRINGSAFE_API StringCchCopyA(
    __out_ecount(cchDest) STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszSrc
);

STRINGSAFE_API StringCchCopyW(
    __out_ecount(cchDest) STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszSrc
);

STRINGSAFE_API StringCchCopyNA(
    __out_ecount(cchDest) STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    __in_ecount(cchSrc) STRINGSAFE_LPCSTR pszSrc,
    size_t cchSrc
);

STRINGSAFE_API StringCchCopyNW(
    __out_ecount(cchDest) STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    __in_ecount(cchSrc) STRINGSAFE_LPCWSTR pszSrc,
    size_t cchSrc
);

#ifdef UNICODE
#define StringCbCopy  StringCbCopyW
#else
#define StringCbCopy  StringCbCopyA
#endif // !UNICODE

STRINGSAFE_API
StringCbCopyA(
    __out_bcount(cbDest) STRINGSAFE_LPSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCSTR pszSrc
);

STRINGSAFE_API
StringCbCopyW(
    __out_bcount(cbDest) STRINGSAFE_LPWSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCWSTR pszSrc
);


#ifdef UNICODE
#define StringCchPrintf  StringCchPrintfW
#else
#define StringCchPrintf  StringCchPrintfA
#endif // !UNICODE


STRINGSAFE_API StringCchPrintfA(
    __out_bcount(cchDest) STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszFormat,
     ...
);

STRINGSAFE_API StringCchPrintfW(
    __out_ecount(cchDest) STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszFormat,
     ...
);


#ifdef UNICODE
#define StringCchPrintfEx  StringCchPrintfW
#else
#define StringCchPrintfEx  StringCchPrintfA
#endif // !UNICODE


STRINGSAFE_API
StringCchPrintfExA(
    __out_bcount(cchDest) STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPSTR *ppszDestEnd,
    size_t *pcchRemaining,
    DWORD dwFlags,
    STRINGSAFE_LPCSTR pszFormat,
    ...
);

STRINGSAFE_API
StringCchPrintfExW(
    __out_bcount(cchDest) STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPWSTR *ppszDestEnd,
    size_t *pcchRemaining,
    DWORD dwFlags,
    STRINGSAFE_LPCWSTR pszFormat,
    ...
);



#ifdef UNICODE
#define StringCbPrintf  StringCbPrintfW
#else
#define StringCbPrintf  StringCbPrintfA
#endif // !UNICODE

STRINGSAFE_API
StringCbPrintfA(
    __out_bcount(cbDest) STRINGSAFE_LPSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCSTR pszFormat,
    ...);

STRINGSAFE_API
StringCbPrintfW(
    __out_bcount(cbDest) STRINGSAFE_LPWSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCWSTR pszFormat,
    ...);


#ifdef UNICODE
#define StringCbPrintfEx  StringCbPrintfExW
#else
#define StringCbPrintfEx  StringCbPrintfExA
#endif // !UNICODE

STRINGSAFE_API
StringCbPrintfExA(
     __out_bcount(cbDest) STRSAFE_LPSTR pszDest,
     size_t cbDest,
     STRSAFE_LPSTR* ppszDestEnd,
     size_t* pcbRemaining,
     DWORD dwFlags,
     STRSAFE_LPCSTR pszFormat,
     ...);

STRINGSAFE_API
StringCbPrintfExW(
      __out_bcount(cbDest) STRSAFE_LPWSTR pszDest,
      size_t cbDest,
      STRSAFE_LPWSTR* ppszDestEnd,
      size_t* pcbRemaining,
      DWORD dwFlags,
      STRSAFE_LPCWSTR pszFormat,
      ...);


#ifdef UNICODE
#define StringCchVPrintf  StringCchVPrintfW
#else
#define StringCchVPrintf  StringCchVPrintfA
#endif // !UNICODE

STRINGSAFE_API
StringCchVPrintfA(
    __out_ecount(cchDest) STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszFormat,
    va_list argList);


STRINGSAFE_API
StringCchVPrintfW(
    __out_ecount(cchDest) STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszFormat,
    va_list argList);


#ifdef UNICODE
#define StringCbVPrintfEx  StringCbVPrintfExW
#else
#define StringCbVPrintfEx  StringCbVPrintfExA
#endif // !UNICODE

STRINGSAFE_API
StringCbVPrintfExA(
    __out_bcount(cbDest) STRSAFE_LPSTR pszDest,
    size_t cbDest,
    STRSAFE_LPSTR* ppszDestEnd,
    size_t* pcbRemaining,
    DWORD dwFlags,
    STRSAFE_LPCSTR pszFormat,
    va_list argList);

STRINGSAFE_API
StringCbVPrintfExW(
    __out_bcount(cbDest) STRSAFE_LPWSTR pszDest,
    size_t cbDest,
    STRSAFE_LPWSTR* ppszDestEnd,
    size_t* pcbRemaining,
    DWORD dwFlags,
    STRSAFE_LPCWSTR pszFormat,
    va_list argList);


#ifdef UNICODE
#define StringCchCat  StringCchCatW
#else
#define StringCchCat  StringCchCatA
#endif // !UNICODE

STRINGSAFE_API
StringCchCatA(
        __out_ecount(cchDest) STRINGSAFE_LPSTR pszDest,
        size_t cchDest,
        STRINGSAFE_LPCSTR pszSrc);

STRINGSAFE_API
StringCchCatW(
        __out_ecount(cchDest) STRINGSAFE_LPWSTR pszDest,
        size_t cchDest,
        STRINGSAFE_LPCWSTR pszSrc);


#ifdef UNICODE
#define StringCbCat  StringCbCatW
#else
#define StringCbCat  StringCbCatA
#endif // !UNICODE

STRINGSAFE_API
StringCbCatA(
    __inout_bcount(cbDest) STRSAFE_LPSTR pszDest,
    size_t cbDest,
    STRSAFE_LPCSTR pszSrc);

STRINGSAFE_API
StringCbCatW(
    __inout_bcount(cbDest) STRSAFE_LPWSTR pszDest,
    size_t cbDest,
    STRSAFE_LPCWSTR pszSrc);


#ifdef UNICODE
#define StringCbCatEx  StringCbCatExW
#else
#define StringCbCatEx  StringCbCatExA
#endif // !UNICODE

STRINGSAFE_API
StringCbCatExA(
    __inout_bcount(cbDest) STRSAFE_LPSTR pszDest,
    size_t cbDest,
    STRSAFE_LPCSTR pszSrc,
    STRSAFE_LPSTR* ppszDestEnd,
    size_t* pcbRemaining,
    DWORD dwFlags);


STRINGSAFE_API
StringCbCatExW(
    __inout_bcount(cbDest) STRSAFE_LPWSTR pszDest,
    size_t cbDest,
    STRSAFE_LPCWSTR pszSrc,
    STRSAFE_LPWSTR* ppszDestEnd,
    size_t* pcbRemaining,
    DWORD dwFlags);

#ifdef __cplusplus
};
#endif


