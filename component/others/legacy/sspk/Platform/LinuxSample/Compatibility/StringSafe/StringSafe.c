///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
**
**  StringSafe.c
**
**  StringSafe implementation.
**
*******************************************************************************/

#include "pkPAL.h"
#include "pkExecutive.h"
#include <stdio.h> // vsnprintf
#include <stdarg.h>

// TODO: harrypy: Even though I see vswprintf in wchar.h, for some reason it isn't getting declared
#if 0
#include <wchar.h> // vswprintf
#else
extern int vswprintf(wchar_t *wcs, size_t maxlen, const wchar_t *format, va_list args);
#endif

#include "strsafe.h"
#include "FmtRewrite.h"

#define STRINGSAFE_WORKERAPI    STRINGSAFE_API


STRINGSAFE_WORKERAPI
StringCopyWorkerA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    size_t* pcchNewDestLength,
    STRINGSAFE_LPCSTR pszSrc,
    size_t cchToCopy)
{
    pkRESULT hr = pkS_OK;
    size_t cchNewDestLength = 0;

    while (cchDest && cchToCopy && (*pszSrc != '\0'))
    {
        *pszDest++ = *pszSrc++;
        cchDest--;
        cchToCopy--;

        cchNewDestLength++;
    }

    if (cchDest == 0)
    {
        // we are going to truncate pszDest
        pszDest--;
        cchNewDestLength--;

        hr = pkE_FAIL;
    }

    *pszDest= '\0';

    if (pcchNewDestLength)
    {
        *pcchNewDestLength = cchNewDestLength;
    }

    return hr;
}



STRINGSAFE_WORKERAPI
StringCopyWorkerW(
    wchar_t* pszDest,
    size_t cchDest,
    size_t* pcchNewDestLength,
    STRINGSAFE_LPCWSTR pszSrc,
    size_t cchToCopy)
{
    pkRESULT hr = pkS_OK;
    size_t cchNewDestLength = 0;

    // ASSERT(cchDest != 0);

    while (cchDest && cchToCopy && (*pszSrc != L'\0'))
    {
        *pszDest++ = *pszSrc++;
        cchDest--;
        cchToCopy--;

        cchNewDestLength++;
    }

    if (cchDest == 0)
    {
        // we are going to truncate pszDest
        pszDest--;
        cchNewDestLength--;

        hr = pkE_FAIL;
    }

    *pszDest= L'\0';

    if (pcchNewDestLength)
    {
        *pcchNewDestLength = cchNewDestLength;
    }

    return hr;
}



STRINGSAFE_WORKERAPI
StringLengthWorkerA(
    STRINGSAFE_LPCSTR psz,
    size_t cchMax,
    size_t* pcchLength)
{
    pkRESULT hr = pkS_OK;
    size_t cchOriginalMax = cchMax;

    while (cchMax && (*psz != '\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
        // the string is longer than cchMax
        hr = pkE_FAIL;
    }

    if (pcchLength)
    {
        if (pkSUCCEEDED(hr))
        {
            *pcchLength = cchOriginalMax - cchMax;
        }
        else
        {
            *pcchLength = 0;
        }
    }

    return hr;
}

STRINGSAFE_WORKERAPI
StringLengthWorkerW(
    STRINGSAFE_LPCWSTR psz,
    size_t cchMax,
    size_t* pcchLength)
{
    pkRESULT hr = pkS_OK;
    size_t cchOriginalMax = cchMax;

    while (cchMax && (*psz != L'\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
        // the string is longer than cchMax
        hr = pkE_FAIL;
    }

    if (pcchLength)
    {
        if (pkSUCCEEDED(hr))
        {
            *pcchLength = cchOriginalMax - cchMax;
        }
        else
        {
            *pcchLength = 0;
        }
    }

    return hr;
}



STRINGSAFE_API
StringCchLengthA(
    STRINGSAFE_LPCSTR psz,
    size_t cchMax,
    size_t* pcchLength)
{
    pkRESULT hr;

    if ((psz == NULL) || (cchMax > STRINGSAFE_MAX_CCH))
    {
        hr = pkE_FAIL;
    }
    else
    {
        hr = StringLengthWorkerA(psz, cchMax, pcchLength);
    }

    if (pkFAILED(hr) && pcchLength)
    {
        *pcchLength = 0;
    }

    return hr;
}

STRINGSAFE_API
StringCchLengthW(
    STRINGSAFE_LPCWSTR psz,
    size_t cchMax,
    size_t* pcchLength)
{
    pkRESULT hr;

    if ((psz == NULL) || (cchMax > STRINGSAFE_MAX_CCH))
    {
        hr = pkE_FAIL;
    }
    else
    {
        hr = StringLengthWorkerW(psz, cchMax, pcchLength);
    }

    if (pkFAILED(hr) && pcchLength)
    {
        *pcchLength = 0;
    }

    return hr;
}


STRINGSAFE_WORKERAPI
StringValidateDestA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    size_t* pcchDestLength,
    size_t cchMax)
{
    pkRESULT hr = pkS_OK;

    if ((cchDest == 0) || (cchDest > cchMax))
    {
        hr = pkE_FAIL;
    }

    if (pcchDestLength)
    {
        if (pkSUCCEEDED(hr))
        {
            hr = StringLengthWorkerA(pszDest, cchMax, pcchDestLength);
        }
        else
        {
            *pcchDestLength = 0;
        }
    }

    return hr;
}

STRINGSAFE_WORKERAPI
StringValidateDestW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    size_t* pcchDestLength,
    size_t cchMax)
{
    pkRESULT hr = pkS_OK;

    if ((cchDest == 0) || (cchDest > cchMax))
    {
        hr = pkE_FAIL;
    }

    if (pcchDestLength)
    {
        if (pkSUCCEEDED(hr))
        {
            hr = StringLengthWorkerW(pszDest, cchMax, pcchDestLength);
        }
        else
        {
            *pcchDestLength = 0;
        }
    }

    return hr;
}

STRINGSAFE_API
StringExValidateDestA(
    STRINGSAFE_LPSTR* ppszDest,
    size_t* pcchDest,
    size_t* pcchDestLength,
    size_t cchMax,
    uint32_t dwFlags)
{
    pkRESULT hr = pkS_OK;

    if (dwFlags & STRINGSAFE_IGNORE_NULLS)
    {
        if ((*ppszDest == NULL) && (*pcchDest != 0))
        {
            hr = pkE_FAIL;
        }

        if (pcchDestLength)
        {
            if (pkFAILED(hr) || (*pcchDest == 0))
            {
                *pcchDestLength = 0;
            }
            else
            {
                hr = StringLengthWorkerA(*ppszDest, cchMax, pcchDestLength);
            }
        }
    }
    else
    {
        hr = StringValidateDestA(*ppszDest, *pcchDest, pcchDestLength, cchMax);
    }

    return hr;
}

STRINGSAFE_API
StringCbCopyA(
    STRINGSAFE_LPSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCSTR pszSrc)
{
    pkRESULT hr;
    size_t cchDest = cbDest / sizeof(char);

    hr = StringValidateDestA(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerA(pszDest,
                               cchDest,
                               NULL,
                               pszSrc,
                               STRINGSAFE_MAX_LENGTH);
    }

    return hr;
}

STRINGSAFE_API
StringCbCopyW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCWSTR pszSrc)
{
    pkRESULT hr;
    size_t cchDest = cbDest / sizeof(wchar_t);

    hr = StringValidateDestW(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerW(pszDest,
                               cchDest,
                               NULL,
                               pszSrc,
                               STRINGSAFE_MAX_LENGTH);
    }

    return hr;
}


STRINGSAFE_WORKERAPI
StringVPrintfWorkerA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    size_t* pcchNewDestLength,
    STRINGSAFE_LPCSTR pszFormat,
    va_list argList)
{
    pkRESULT hr = pkS_OK;
    int iRet;
    size_t cchMax;
    size_t cchNewDestLength = 0;
#ifdef STRINGSAFE_NO_FORMAT_REWRITE
    LPCSTR fmt = pszFormat;
    LPSTR pszNewFormat = NULL;
#else // STRINGSAFE_NO_FORMAT_REWRITE is not set
    LPSTR pszNewFormat = RewriteFormatStringA(pszFormat);
    LPCSTR fmt = pszNewFormat?pszNewFormat:pszFormat;
#endif

    // leave the last space for the null terminator
    cchMax = cchDest - 1;


    iRet = vsnprintf(pszDest, cchDest, fmt, argList);
    if(iRet >= (int)cchDest)
    {
            iRet = -1;
    }

#ifndef STRINGSAFE_NO_FORMAT_REWRITE
    if(NULL != pszNewFormat)
    {
        FreeFormatStringA(pszNewFormat);
    }
#endif

    if ((iRet < 0) || (((size_t)iRet) > cchMax))
    {
        // need to null terminate the string
        pszDest += cchMax;
        *pszDest = '\0';

        cchNewDestLength = cchMax;

        // we have truncated pszDest
        hr = pkE_FAIL;
    }
    else if (((size_t)iRet) == cchMax)
    {
        // need to null terminate the string
        pszDest += cchMax;
        *pszDest = '\0';

        cchNewDestLength = cchMax;
    }
    else
    {
        cchNewDestLength = (size_t)iRet;
    }

    if (pcchNewDestLength)
    {
        *pcchNewDestLength = cchNewDestLength;
    }

    return hr;
}

STRINGSAFE_WORKERAPI
StringVPrintfWorkerW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    size_t* pcchNewDestLength,
    STRINGSAFE_LPCWSTR pszFormat,
    va_list argList)
{
    pkRESULT hr = pkS_OK;
    int iRet;
    size_t cchMax;
    size_t cchNewDestLength = 0;
#ifdef STRINGSAFE_NO_FORMAT_REWRITE
    LPCWSTR fmt = pszFormat;
    LPWSTR pszNewFormat = NULL;
#else // STRINGSAFE_NO_FORMAT_REWRITE is not set
    LPWSTR pszNewFormat = RewriteFormatStringW(pszFormat);
    LPCWSTR fmt = pszNewFormat?pszNewFormat:pszFormat;
#endif

    // leave the last space for the null terminator
    cchMax = cchDest - 1;

    iRet = vswprintf(pszDest, cchDest, fmt, argList);
    if(iRet >= (int)cchDest)
    {
            iRet = -1;
    }

#ifndef STRINGSAFE_NO_FORMAT_REWRITE
    if(NULL != pszNewFormat)
    {
        FreeFormatStringW(pszNewFormat);
    }
#endif

    // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

    if ((iRet < 0) || (((size_t)iRet) > cchMax))
    {
        // need to null terminate the string
        pszDest += cchMax;
        *pszDest = L'\0';

        cchNewDestLength = cchMax;

        // we have truncated pszDest
        hr = pkE_FAIL;
    }
    else if (((size_t)iRet) == cchMax)
    {
        // need to null terminate the string
        pszDest += cchMax;
        *pszDest = L'\0';

        cchNewDestLength = cchMax;
    }
    else
    {
        cchNewDestLength = (size_t)iRet;
    }

    if (pcchNewDestLength)
    {
        *pcchNewDestLength = cchNewDestLength;
    }

    return hr;
}


#ifdef UNICODE
#define StringCbPrintf  StringCbPrintfW
#else
#define StringCbPrintf  StringCbPrintfA
#endif // !UNICODE

STRINGSAFE_API
StringCbPrintfA(
    STRINGSAFE_LPSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCSTR pszFormat,
    ...)
{
    pkRESULT hr;
    size_t cchDest = cbDest / sizeof(char);

    hr = StringValidateDestA(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerA(pszDest,
                                  cchDest,
                                  NULL,
                                  pszFormat,
                                  argList);

        va_end(argList);
    }

    return hr;
}

STRINGSAFE_API
StringCbPrintfW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cbDest,
    STRINGSAFE_LPCWSTR pszFormat,
    ...)
{
    pkRESULT hr;
    size_t cchDest = cbDest / sizeof(wchar_t);

    hr = StringValidateDestW(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerW(pszDest,
                                  cchDest,
                                  NULL,
                                  pszFormat,
                                  argList);

        va_end(argList);
    }

    return hr;
}


STRINGSAFE_API
StringCchPrintfA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszFormat,
    ...)
{
    pkRESULT hr;

    hr = StringValidateDestA(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerA(pszDest,
                                  cchDest,
                                  NULL,
                                  pszFormat,
                                  argList);

        va_end(argList);
    }

    return hr;
}

STRINGSAFE_API
StringCchPrintfW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszFormat,
    ...)
{
    pkRESULT hr;

    hr = StringValidateDestW(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerW(pszDest,
                                  cchDest,
                                  NULL,
                                  pszFormat,
                                  argList);

        va_end(argList);
    }

    return hr;
}


STRINGSAFE_API
StringCchPrintfExA(
    __out_bcount(cchDest) STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPSTR *ppszDestEnd,
    size_t *pcchRemaining,
    DWORD dwFlags,
    STRINGSAFE_LPCSTR pszFormat,
    ...)
{
    pkRESULT hr;

    hr = StringValidateDestA(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        size_t cchNewDestLen = 0;

        va_list argList;

        va_start(argList, pszFormat);

        hr = StringVPrintfWorkerA(pszDest,
                                  cchDest,
                                  &cchNewDestLen,
                                  pszFormat,
                                  argList);

        va_end(argList);

        if (pkSUCCEEDED(hr))
        {
            if (NULL != ppszDestEnd)
            {
                *ppszDestEnd = &pszDest[cchNewDestLen];
            }
            if (NULL != pcchRemaining)
            {
                *pcchRemaining = cchDest - cchNewDestLen;
            }
        }
    }

    return hr;
}


STRINGSAFE_API
StringCchPrintfExW(
    __out_bcount(cchDest) STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPWSTR *ppszDestEnd,
    size_t *pcchRemaining,
    DWORD dwFlags,
    STRINGSAFE_LPCWSTR pszFormat,
    ...)
    {
        pkRESULT hr;

        hr = StringValidateDestW(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

        if (pkSUCCEEDED(hr))
        {
            size_t cchNewDestLen = 0;

            va_list argList;

            va_start(argList, pszFormat);

            hr = StringVPrintfWorkerW(pszDest,
                                      cchDest,
                                      &cchNewDestLen,
                                      pszFormat,
                                      argList);

            va_end(argList);

            if (pkSUCCEEDED(hr))
            {
                if (NULL != ppszDestEnd)
                {
                    *ppszDestEnd = &pszDest[cchNewDestLen];
                }
                if (NULL != pcchRemaining)
                {
                    *pcchRemaining = cchDest - cchNewDestLen;
                }
            }
        }

        return hr;
    }


STRINGSAFE_API
StringCchCopyA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszSrc)
{
    pkRESULT hr;

    hr = StringValidateDestA(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerA(pszDest,
                               cchDest,
                               NULL,
                               pszSrc,
                               STRINGSAFE_MAX_LENGTH);
    }

    return hr;
}

STRINGSAFE_API
StringCchCopyW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszSrc)
{
    pkRESULT hr;

    hr = StringValidateDestW(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerW(pszDest,
                               cchDest,
                               NULL,
                               pszSrc,
                               STRINGSAFE_MAX_LENGTH);
    }

    return hr;
}

STRINGSAFE_API
StringCchCopyNA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszSrc,
    size_t cchSrc)
{
    pkRESULT hr;

    hr = StringValidateDestA(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerA(pszDest,
                               cchDest,
                               NULL,
                               pszSrc,
                               cchSrc);
    }

    return hr;
}

STRINGSAFE_API
StringCchCopyNW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszSrc,
    size_t cchSrc)
{
    pkRESULT hr;

    hr = StringValidateDestW(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerW(pszDest,
                               cchDest,
                               NULL,
                               pszSrc,
                               cchSrc);
    }

    return hr;
}

STRINGSAFE_API
StringCchVPrintfA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszFormat,
    va_list argList)
{
    pkRESULT hr;

    hr = StringValidateDestA(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringVPrintfWorkerA(pszDest,
                                  cchDest,
                                  NULL,
                                  pszFormat,
                                  argList);
    }

    return hr;
}

STRINGSAFE_API
StringCchVPrintfW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszFormat,
    va_list argList)
{
    pkRESULT hr;

    hr = StringValidateDestW(pszDest, cchDest, NULL, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringVPrintfWorkerW(pszDest,
                                  cchDest,
                                  NULL,
                                  pszFormat,
                                  argList);
    }

    return hr;
}


STRINGSAFE_API
StringCchCatA(
    STRINGSAFE_LPSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCSTR pszSrc)
{
    pkRESULT hr;
    size_t cchDestLength;

    hr = StringValidateDestA(pszDest, cchDest, &cchDestLength, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerA(pszDest + cchDestLength,
                               cchDest - cchDestLength,
                               NULL,
                               pszSrc,
                               STRINGSAFE_MAX_CCH);
    }

    return hr;
}

STRINGSAFE_API
StringCchCatW(
    STRINGSAFE_LPWSTR pszDest,
    size_t cchDest,
    STRINGSAFE_LPCWSTR pszSrc)
{
    pkRESULT hr;
    size_t cchDestLength;

    hr = StringValidateDestW(pszDest, cchDest, &cchDestLength, STRINGSAFE_MAX_CCH);

    if (pkSUCCEEDED(hr))
    {
        hr = StringCopyWorkerW(pszDest + cchDestLength,
                               cchDest - cchDestLength,
                               NULL,
                               pszSrc,
                               STRINGSAFE_MAX_CCH);
    }

    return hr;
}


