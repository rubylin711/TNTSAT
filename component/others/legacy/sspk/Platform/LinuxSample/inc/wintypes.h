///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

//----------------------------------------------
// Standard value types
//----------------------------------------------
#define    CONST            const

typedef int BOOL;
typedef unsigned char BOOLEAN;
typedef uint8_t BYTE;
typedef char CHAR;
typedef uint32_t DWORD;

typedef size_t ULONG_PTR;

typedef size_t DWORD_PTR;
typedef uint32_t DWORD32;
typedef uint64_t DWORD64;

typedef float FLOAT;
typedef double DOUBLE;

typedef int INT;
typedef size_t INT_PTR;

typedef uint16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;

typedef long LONG;
typedef size_t LONG_PTR;

typedef int32_t LONG32;
typedef int64_t LONG64;
typedef int16_t SHORT;

typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint32_t ULONG;

typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;

typedef void VOID;

typedef unsigned short int USHORT;
typedef uint16_t WORD;

typedef wchar_t WCHAR;

#ifdef UNICODE
typedef WCHAR TCHAR;
#else
typedef char TCHAR;
#endif

typedef unsigned UNSIGNED;

typedef size_t SIZE_T;

typedef size_t UINT_PTR, *PUINT_PTR;

//----------------------------------------------
// Standard Pointer types
//----------------------------------------------

typedef BOOL             *PBOOL;
typedef BYTE             *PBYTE;
typedef CHAR             *PCHAR;
typedef VOID             *PVOID;
typedef PVOID            HANDLE;
typedef DWORD            *PDWORD;
typedef DWORD_PTR        *PDWORD_PTR;
typedef DWORD32          *PDWORD32;
typedef DWORD64          *PDWORD64;
typedef FLOAT            *PFLOAT;
typedef DOUBLE           *PDOUBLE;
typedef HANDLE           *PHANDLE;
typedef INT              *PINT;
typedef INT_PTR          *PINT_PTR;
typedef INT16            *PINT16;
typedef INT32            *PINT32;
typedef INT64            *PINT64;
typedef LONG             *PLONG;
typedef LONGLONG         *PLONGLONG;
typedef LONG_PTR         *PLONG_PTR;
typedef LONG32           *PLONG32;
typedef LONG64           *PLONG64;
typedef SHORT            *PSHORT;
typedef TCHAR            *PTCHAR;
typedef UCHAR            *PUCHAR;
typedef UINT             *PUINT;
typedef UINT8            *PUINT8;
typedef UINT16           *PUINT16;
typedef UINT32           *PUINT32;
typedef UINT64           *PUINT64;
typedef ULONG            *PULONG;
typedef ULONGLONG        *PULONGLONG;
typedef USHORT           *PUSHORT;
typedef WCHAR            *PWCHAR;
typedef WORD             *PWORD;

typedef CONST CHAR       *PCCHAR;
typedef CONST TCHAR      *PCTCHAR;
typedef CONST WCHAR      *PCWCHAR;

typedef BOOL             *LPBOOL;
typedef BYTE             *LPBYTE;
typedef CHAR             *LPCHAR;
typedef VOID             *LPVOID;
typedef DWORD            *LPDWORD;
typedef DWORD_PTR        *LPDWORD_PTR;
typedef DWORD32          *LPDWORD32;
typedef DWORD64          *LPDWORD64;
typedef FLOAT            *LPFLOAT;
typedef DOUBLE           *LPDOUBLE;
typedef HANDLE           *LPHANDLE;
typedef INT              *LPINT;
typedef INT_PTR          *LPINT_PTR;
typedef INT16            *LPINT16;
typedef INT32            *LPINT32;
typedef INT64            *LPINT64;
typedef LONG             *LPLONG;
typedef LONGLONG         *LPLONGLONG;
typedef LONG_PTR         *LPLONG_PTR;
typedef LONG32           *LPLONG32;
typedef LONG64           *LPLONG64;
typedef SHORT            *LPSHORT;
typedef TCHAR            *LPTCHAR;
typedef UCHAR            *LPUCHAR;
typedef UINT             *LPUINT;
typedef UINT8            *LPUINT8;
typedef UINT32           *LPUINT32;
typedef UINT64           *LPUINT64;
typedef ULONG            *LPULONG;
typedef ULONGLONG        *LPULONGLONG;
typedef USHORT           *LPUSHORT;
typedef WCHAR            *LPWCHAR;
typedef WORD             *LPWORD;

typedef CHAR             *PSTR;
typedef WCHAR            *PWSTR;
typedef TCHAR            *PTSTR;

typedef CHAR             *LPSTR;
typedef WCHAR            *LPWSTR;
typedef TCHAR            *LPTSTR;

typedef CONST CHAR       *PCSTR;
typedef CONST WCHAR      *PCWSTR;
typedef CONST TCHAR      *PCTSTR;

typedef CONST CHAR       *LPCCHAR;
typedef CONST TCHAR      *LPCTCHAR;
typedef CONST WCHAR      *LPCWCHAR;

typedef CONST CHAR       *LPCSTR;
typedef CONST TCHAR      *LPCTSTR;
typedef CONST WCHAR      *LPCWSTR;

typedef CHAR             *PCH;

#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif


//----------------------------------------------
// Calling conventions
//----------------------------------------------

#ifndef __declspec
#define __declspec(param)
#endif

#ifndef __cdecl
#define __cdecl __attribute((stdcall))
#endif

#define API                 __attribute((stdcall))
#define STDAPICALLTYPE      __attribute((stdcall))
#define STDMETHODCALLTYPE   __attribute((stdcall))
#define CALLBACK            __attribute((stdcall))
#define DECLSPEC_SELECTANY
#define DECLSPEC_NOVTABLE

#define __override

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif


//----------------------------------------------
// Interface declarations
//----------------------------------------------
#define STDMETHOD(method)           virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method)     virtual type STDMETHODCALLTYPE method
#define PURE                        = 0
#define THIS_
#define THIS                        void

#define DECLARE_INTERFACE(iface)                interface DECLSPEC_NOVTABLE iface
#define DECLARE_INTERFACE_(iface, baseiface)    interface DECLSPEC_NOVTABLE iface : public baseiface

#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE

#define STDAPI                  PALEXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           PALEXTERN_C type STDAPICALLTYPE

#ifndef MAX_PATH
#define MAX_PATH  255
#endif

typedef struct  tagFILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;


typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } ;
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;


typedef union tagULARGE_INTEGER
{
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;

    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

//----------------------------------------------
// Pointers
//----------------------------------------------
#define FAR
#define NEAR

//----------------------------------------------
// Structure
//----------------------------------------------

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY FAR * Flink; //@field    The Pointer to next
    struct _LIST_ENTRY FAR * Blink; //@field    The Pointer to the previous
} LIST_ENTRY, *PLIST_ENTRY;

//----------------------------------------------
// Text
//----------------------------------------------
#define __T(x)      L ## x
#define _T(x)       __T(x)
#define _TEXT(x)    __T(x)

#define OPTIONAL
#define UNREFERENCED_PARAMETER(P)           (P)

#define IN
#define OUT

#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) || defined(_M_IA64) || defined(_M_AMD64)
#define UNALIGNED __unaligned
#if defined(_WIN64)
#define UNALIGNED64 __unaligned
#else
#define UNALIGNED64
#endif
#else
#define UNALIGNED
#define UNALIGNED64
#endif


//----------------------------------------------
// Error type
//----------------------------------------------
typedef pkRESULT HRESULT;

//----------------------------------------------
// Error check macros
//----------------------------------------------
#define     SUCCEEDED        pkSUCCEEDED
#define     FAILED            pkFAILED

#ifndef MAKE_HRESULT
#define SEVERITY_SUCCESS    0
#define SEVERITY_ERROR      1
#define MAKE_HRESULT(sev,fac,code) ((HRESULT)(((HRESULT)sev<<31)|((HRESULT)fac<<16)|((HRESULT)code)))
#endif


#define     HRESULT_FACILITY(hr) 0

//----------------------------------------------
// Success HRESULT codes
//----------------------------------------------
#define S_OK            ((HRESULT)0x00000000L)
#define S_FALSE         ((HRESULT)0x00000001L)

//----------------------------------------------
// Error HRESULT codes
//----------------------------------------------

#define E_FAIL          ((HRESULT)0x80004005L)
#define E_INVALIDARG    ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY   ((HRESULT)0x8007000EL)
#define E_POINTER       ((HRESULT)0x80004003L)
#define E_UNEXPECTED    ((HRESULT)0x8000FFFFL)
#define E_ACCESSDENIED  ((HRESULT)0x80070005L)
#define E_HANDLE        ((HRESULT)0x80070006L)
#define E_NOTIMPL       ((HRESULT)0x80004001L)
#define E_NOINTERFACE   ((HRESULT)0x80004002L)
#define E_ABORT         ((HRESULT)0x80004004L)

//----------------------------------------------
// Error MessageIDs (lower 16 bits)
//----------------------------------------------

#define ERROR_SUCCESS                    0L
#define ERROR_INVALID_DATA               13L
#define ERROR_REQUEST_OUT_OF_SEQUENCE    776L

//----------------------------------------------
// Standard Interface declarations
//----------------------------------------------
#define interface struct
#define WINAPI  __attribute((stdcall))

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

//----------------------------------------------
// miscellaneous declarations
//----------------------------------------------
typedef uint8_t         byte;

typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, *LPRECT;

#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define INFINITE 0xFFFFFFFF // same as EXEC_WAIT_INFINITE

// Emulate Windows thread priority levels - values must match those in platThreads.h

#define THREAD_PRIORITY_IDLE             254
#define THREAD_PRIORITY_LOWEST             254
#define THREAD_PRIORITY_BELOW_NORMAL     253
#define THREAD_PRIORITY_NORMAL             251
#define THREAD_PRIORITY_ABOVE_NORMAL     249
#define THREAD_PRIORITY_HIGHEST         248
#define THREAD_PRIORITY_TIME_CRITICAL    247

#define CHECK_ALLOC(_alloced_p) assert(NULL != _alloced_p)


// ================================================
// platExecutive.h included *after* Windows types are defined
// ================================================
#include "platExecutive.h"

