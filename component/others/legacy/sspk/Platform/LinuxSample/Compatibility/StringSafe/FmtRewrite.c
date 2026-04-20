///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
**
**  FmtRewrite.c
**
**  Since many of our modules use StringSafe in a way that assumes it understands
**  windows-style printf format strings (e.g. %s means %ls if you're using the -W
**  version of a function), but this functionality does not exist in the underlying
**  string calls on most OSes, this implementation of StringSafe will
**  rewrite format strings on the fly.
**
**  The rewrites this performs are:
**     In the ANSI versions of the printf functions:
**            %S becomes %ls
**            %C becomes %lc
**     In the WCHAR versions of the printf functions:
**            %s becomes %ls
**            %c becomes %ls
**            %S becomes %s
**            %C becomes %c
**
**  Flags, options, width, and precision are preserved when performing these
**  rewrites, e.g. "%10s" will become "%10ls"
**
**  This functionality can be disabled by defining STRINGSAFE_NO_FORMAT_REWRITE
**
*******************************************************************************/

#include "pkPAL.h"
#include "pkExecutive.h"
#include "strsafe.h"
#include "FmtRewrite.h"

// ===============================================================================
#ifdef STRINGSAFE_NO_FORMAT_REWRITE

LPSTR RewriteFormatStringA(LPCSTR pszFormat)
{
    return NULL;
}

LPWSTR RewriteFormatStringW(LPCWSTR pszFormat)
{
    return NULL;
}

void FreeFormatStringA(char *pBuf)
{
}

void FreeFormatStringW(WCHAR *pBuf)
{
}


// ===============================================================================
#else // !STRINGSAFE_NO_FORMAT_REWRITE

// -----------------------------
// The internal worker functions
// -----------------------------

//outchar is used for a character that
//should be moved into the output buffer
//if there is one.  If there isn't one,
//it still decrements the remaining count
//so that at the end of this all, we have
//an accurate count of how many characters
//would have been needed
#define outchar(c)\
     {\
        if(cchOutRemaining > 0)\
        {\
            *outp++ = c;\
        }\
        cchOutRemaining--;\
     }\

//for debugging of format string parsing
//#define FRDBG(y) Executive_DebugPrintf y
#define FRDBG(y)


// ANSI character version

#ifdef FMT_CHAR_TYPE
#error FMT_CHAR_TYPE already defined
#endif
#define FMT_CHAR_TYPE(x) x

static pkRESULT RewriteFormatStringWorkerA(
    LPCSTR pszFormat,
    LPSTR pszNewFormat,
    int *pcchNewFormatLen
    )
{
    LPSTR pszResult = NULL;
    int cchOutRemaining = 0;
    char *outp = pszNewFormat;
    pkRESULT hr = S_FALSE;

    char c;
    const char *p = pszFormat;

    //flags tracking our state in the conversion spec
    bool_t bInSpec = FALSE;    //we are between a % and the end of a conversion spec
    bool_t bLHWPrefix = FALSE; //this conversion has a 'l', 'h' or 'w' prefix

    if(!pcchNewFormatLen)
    {
        hr = pkE_POINTER;
        goto bail;
    }

    if(NULL == pszFormat || (*pcchNewFormatLen > 0 && NULL == pszNewFormat))
    {
        hr = pkE_POINTER;
        goto bail;
    }

    cchOutRemaining = *pcchNewFormatLen;

    while(*p)
    {
        c = *p;
        FRDBG(("----> %c\n", c));
        if(bInSpec)
        {
            bool_t looping = TRUE;
            do
            {
                //skip over flags
                switch(c)
                {
                    case FMT_CHAR_TYPE('-'):
                    case FMT_CHAR_TYPE('+'):
                    case FMT_CHAR_TYPE('0'):
                    case FMT_CHAR_TYPE(' '):
                    case FMT_CHAR_TYPE('#'):
                        FRDBG(("  flag\n"));
                        outchar(c);
                        c = *(++p);
                        FRDBG(("----> %c\n", c));
                        break;
                    default:
                        FRDBG(("    past flags\n"));
                        looping = FALSE;
                        break;
                }

            } while(looping && c);

            if(!c)
            {
                break;
            }

            looping = TRUE;
            do
            {
                //skip over width
                switch(c)
                {
                    case FMT_CHAR_TYPE('*'):
                    case FMT_CHAR_TYPE('0'):
                    case FMT_CHAR_TYPE('1'):
                    case FMT_CHAR_TYPE('2'):
                    case FMT_CHAR_TYPE('3'):
                    case FMT_CHAR_TYPE('4'):
                    case FMT_CHAR_TYPE('5'):
                    case FMT_CHAR_TYPE('6'):
                    case FMT_CHAR_TYPE('7'):
                    case FMT_CHAR_TYPE('8'):
                    case FMT_CHAR_TYPE('9'):
                        FRDBG(("  width char\n"));
                        outchar(c);
                        c = *(++p);
                        FRDBG(("----> %c\n", c));
                        break;
                    default:
                        FRDBG(("    past width\n"));
                        looping = FALSE;
                        break;
                }
            } while(looping && c);

            if(!c)
            {
                break;
            }

             if(c == '.')
             {
                 FRDBG(("precision\n"));
                 outchar(c);
                 c = *(++p);
                 FRDBG(("----> %c\n", c));
                 looping = TRUE;
                 do
                 {
                     //skip over precision
                     switch(c)
                     {
                         case FMT_CHAR_TYPE('0'):
                         case FMT_CHAR_TYPE('1'):
                         case FMT_CHAR_TYPE('2'):
                         case FMT_CHAR_TYPE('3'):
                         case FMT_CHAR_TYPE('4'):
                         case FMT_CHAR_TYPE('5'):
                         case FMT_CHAR_TYPE('6'):
                         case FMT_CHAR_TYPE('7'):
                         case FMT_CHAR_TYPE('8'):
                         case FMT_CHAR_TYPE('9'):
                             FRDBG(("  precision digit\n"));
                             outchar(c);
                             c = *(++p);
                            FRDBG(("----> %c\n", c));
                             break;
                         default:
                            FRDBG(("    past precision\n"));
                             looping = FALSE;
                             break;
                     }
                 } while(looping && c);

                 if(!c)
                 {
                     break;
                 }
            }

            // track then skip 'l', 'h', 'w'
            // convert 'I' & 'I32' to l (el), and 'I64' to ll (el el)
            switch(c)
            {
                case FMT_CHAR_TYPE('l'):
                case FMT_CHAR_TYPE('h'):
                case FMT_CHAR_TYPE('w'):
                    // keep track that we have seen an l, h, or w prefix for this conversion
                    bLHWPrefix = TRUE;
                    FRDBG(("  prefix lhw\n"));
                    outchar(c);
                    c = *(++p);
                    FRDBG(("----> %c\n", c));
                    break;
                case FMT_CHAR_TYPE('I'):
                    FRDBG(("  prefix I\n"));
                    outchar(FMT_CHAR_TYPE('l')); // replace I with l (el)
                    c = *(++p);
                    FRDBG(("----> %c\n", c));
                    if(c == FMT_CHAR_TYPE('3'))
                    {
                        FRDBG(("    prefix I32\n"));
                        ++p; // skip the 2
                        c = *(++p); // get the format character
                        FRDBG(("----> %c\n", c));
                    }
                    else if(c == FMT_CHAR_TYPE('6'))
                    {
                        FRDBG(("    prefix I64\n"));
                        outchar(FMT_CHAR_TYPE('l')); // replace 6 with another l so now we have ll prefix
                        ++p; // skip the 4
                        c = *(++p); // get the format character
                        FRDBG(("----> %c\n", c));
                    }
                    else
                    {
                        FRDBG(("    prefix I\n"));
                        FRDBG(("----> %c\n", c));
                    }
                    hr = pkS_OK;
                    break;
            }
            FRDBG(("    past prefix\n"));

            //ok, now we should be staring at the format character
            // for the ANSI version of this function,
            // we will convert %S to %ls and %C to %lc

            switch(c)
            {
                case FMT_CHAR_TYPE('S'):
                    FRDBG(("found S conversion\n"));
                    // if there isn't an l,h or w already, add an l
                    if(!bLHWPrefix)
                    {
                        outchar(FMT_CHAR_TYPE('l'));
                        outchar(FMT_CHAR_TYPE('s'));
                    }
                    else
                    {
                        outchar(c);
                    }
                    hr = pkS_OK;
                    break;
                case FMT_CHAR_TYPE('C'):
                    FRDBG(("found C conversion\n"));
                    // if there isn't an l,h or w already, add an l
                    if(!bLHWPrefix)
                    {
                        outchar(FMT_CHAR_TYPE('l'));
                        outchar(FMT_CHAR_TYPE('c'));
                    }
                    else
                    {
                        outchar(c);
                    }
                    hr = pkS_OK;
                    break;
                default:
                    outchar(c);
            }

            FRDBG(("leaving spec\n"));
            bInSpec = FALSE;
            bLHWPrefix = FALSE;
            ++p;
        }
        else // !bInSpec
        {
            if(c == FMT_CHAR_TYPE('%'))
            {
                // start of a conversion spec, unless we see another % immediately
                outchar(c);
                c = *(++p);
                if(c == FMT_CHAR_TYPE('%'))
                {
                    FRDBG(("%%\n"));
                    outchar(c);
                    p++;
                }
                else
                {
                    FRDBG(("entering spec\n"));
                    bInSpec = TRUE;
                }
            }
            else
            {
                // normal character
                outchar(c);
                ++p;
            }
        }
    }

    outchar(FMT_CHAR_TYPE('\0'));

    // if hr == pkS_FALSE here, no rewrite was necessary,
    // so we need not worry about the output buffer.
    if(hr == pkS_OK)
    {
        if(cchOutRemaining < 0)
        {
            //if we ran out of buffer, return pkE_INSUFFICIENT_BUFFER,
            //but also make sure the output buffer is null-terminated
            hr = pkE_INSUFFICIENT_BUFFER;
            if(*pcchNewFormatLen > 0)
            {
                pszNewFormat[(*pcchNewFormatLen) - 1] = FMT_CHAR_TYPE('\0');
            }
        }

        //report to the caller how much output buffer was necessary.
        //if we ran out, cchOutRemaining will be negative by the
        //extra amount needed
        *pcchNewFormatLen -= cchOutRemaining;
    }

bail:
    return hr;
}


// Wide character version

#undef FMT_CHAR_TYPE
#define FMT_CHAR_TYPE(x) L##x

static pkRESULT RewriteFormatStringWorkerW(
    LPCWSTR pszFormat,
    LPWSTR pszNewFormat,
    int *pcchNewFormatLen
    )
{
    LPWSTR pszResult = NULL;
    int cchOutRemaining = 0;
    wchar_t *outp = pszNewFormat;
    pkRESULT hr = S_FALSE;

    wchar_t c;
    const wchar_t *p = pszFormat;

    //flags tracking our state in the conversion spec
    bool_t bInSpec = FALSE;    //we are between a % and the end of a spec
    bool_t bLHWPrefix = FALSE; //this conversion has a 'l', 'h' or 'w' prefix

    if(!pcchNewFormatLen)
    {
        hr = pkE_POINTER;
        goto bail;
    }

    if(NULL == pszFormat || (*pcchNewFormatLen > 0 && NULL == pszNewFormat))
    {
        hr = pkE_POINTER;
        goto bail;
    }

    cchOutRemaining = *pcchNewFormatLen;

    while(*p)
    {
        c = *p;
        FRDBG(("----> %c\n", c));
        if(bInSpec)
        {
            bool_t looping = TRUE;
            do
            {
                //skip over flags
                switch(c)
                {
                    case FMT_CHAR_TYPE('-'):
                    case FMT_CHAR_TYPE('+'):
                    case FMT_CHAR_TYPE('0'):
                    case FMT_CHAR_TYPE(' '):
                    case FMT_CHAR_TYPE('#'):
                        FRDBG(("  flag\n"));
                        outchar(c);
                        c = *(++p);
                        FRDBG(("----> %c\n", c));
                        break;
                    default:
                        FRDBG(("    past flags\n"));
                        looping = FALSE;
                        break;
                }

            } while(looping && c);

            if(!c)
            {
                break;
            }

            looping = TRUE;
            do
            {
                //skip over width
                switch(c)
                {
                    case FMT_CHAR_TYPE('*'):
                    case FMT_CHAR_TYPE('0'):
                    case FMT_CHAR_TYPE('1'):
                    case FMT_CHAR_TYPE('2'):
                    case FMT_CHAR_TYPE('3'):
                    case FMT_CHAR_TYPE('4'):
                    case FMT_CHAR_TYPE('5'):
                    case FMT_CHAR_TYPE('6'):
                    case FMT_CHAR_TYPE('7'):
                    case FMT_CHAR_TYPE('8'):
                    case FMT_CHAR_TYPE('9'):
                        FRDBG(("  width char\n"));
                        outchar(c);
                        c = *(++p);
                        FRDBG(("----> %c\n", c));
                        break;
                    default:
                        FRDBG(("    past width\n"));
                        looping = FALSE;
                        break;
                }
            } while(looping && c);

            if(!c)
            {
                break;
            }

             if(c == '.')
             {
                 FRDBG(("precision\n"));
                 outchar(c);
                 c = *(++p);
                 FRDBG(("----> %c\n", c));
                 looping = TRUE;
                 do
                 {
                     //skip over precision
                     switch(c)
                     {
                         case FMT_CHAR_TYPE('0'):
                         case FMT_CHAR_TYPE('1'):
                         case FMT_CHAR_TYPE('2'):
                         case FMT_CHAR_TYPE('3'):
                         case FMT_CHAR_TYPE('4'):
                         case FMT_CHAR_TYPE('5'):
                         case FMT_CHAR_TYPE('6'):
                         case FMT_CHAR_TYPE('7'):
                         case FMT_CHAR_TYPE('8'):
                         case FMT_CHAR_TYPE('9'):
                             FRDBG(("  precision digit\n"));
                             outchar(c);
                             c = *(++p);
                            FRDBG(("----> %c\n", c));
                             break;
                         default:
                            FRDBG(("    past precision\n"));
                             looping = FALSE;
                             break;
                     }
                 } while(looping && c);

                 if(!c)
                 {
                     break;
                 }
            }

            // track then skip 'l', 'h', 'w'
            // convert 'I' & 'I32' to l (el), and 'I64' to ll (el el)
            switch(c)
            {
                case FMT_CHAR_TYPE('l'):
                case FMT_CHAR_TYPE('h'):
                case FMT_CHAR_TYPE('w'):
                    // keep track that we have seen an l, h, or w prefix for this conversion
                    bLHWPrefix = TRUE;
                    FRDBG(("  prefix lhw\n"));
                    outchar(c);
                    c = *(++p);
                    FRDBG(("----> %c\n", c));
                    break;
                case FMT_CHAR_TYPE('I'):
                    FRDBG(("  prefix I\n"));
                    outchar(FMT_CHAR_TYPE('l')); // replace I with l (el)
                    c = *(++p);
                    FRDBG(("----> %c\n", c));
                    if(c == FMT_CHAR_TYPE('3'))
                    {
                        FRDBG(("    prefix I32\n"));
                        ++p; // skip the 2
                        c = *(++p); // get the format character
                        FRDBG(("----> %c\n", c));
                    }
                    else if(c == FMT_CHAR_TYPE('6'))
                    {
                        FRDBG(("    prefix I64\n"));
                        outchar(FMT_CHAR_TYPE('l')); // replace 6 with another l so now we have ll prefix
                        ++p; // skip the 4
                        c = *(++p); // get the format character
                        FRDBG(("----> %c\n", c));
                    }
                    else
                    {
                        FRDBG(("    prefix I\n"));
                        FRDBG(("----> %c\n", c));
                    }
                    hr = pkS_OK;
                    break;
            }
            FRDBG(("    past prefix\n"));

            //ok, now we should be staring at the format character

            // for the WCHAR version of this function,
            // we will convert:
            //      %s to %ls
            //      %c to %lc
            //      %S to %s
            //      %C to %c

            switch(c)
            {
                case FMT_CHAR_TYPE('s'):
                    FRDBG(("found s conversion\n"));
                    // if there isn't an l,h or w already, add an l
                    if(!bLHWPrefix)
                    {
                        outchar(FMT_CHAR_TYPE('l'));
                        outchar(FMT_CHAR_TYPE('s'));
                    }
                    else
                    {
                        outchar(c);
                    }
                    hr = pkS_OK;
                    break;
                case FMT_CHAR_TYPE('S'):
                    FRDBG(("found S conversion\n"));
                    outchar(FMT_CHAR_TYPE('s'));
                    hr = pkS_OK;
                    break;
                case FMT_CHAR_TYPE('c'):
                    FRDBG(("found c conversion\n"));
                    // if there isn't an l,h or w already, add an l
                    if(!bLHWPrefix)
                    {
                        outchar(FMT_CHAR_TYPE('l'));
                        outchar(FMT_CHAR_TYPE('c'));
                    }
                    else
                    {
                        outchar(c);
                    }
                    hr = pkS_OK;
                    break;
                case FMT_CHAR_TYPE('C'):
                    FRDBG(("found C conversion\n"));
                    outchar(FMT_CHAR_TYPE('c'));
                    hr = pkS_OK;
                    break;
                default:
                    outchar(c);
            }

            FRDBG(("leaving spec\n"));
            bInSpec = FALSE;
            bLHWPrefix = FALSE;
            ++p;
        }
        else // !bInSpec
        {
            if(c == FMT_CHAR_TYPE('%'))
            {
                // start of a conversion spec, unless we see another % immediately
                outchar(c);
                c = *(++p);
                if(c == FMT_CHAR_TYPE('%'))
                {
                    FRDBG(("%%\n"));
                    outchar(c);
                    p++;
                }
                else
                {
                    FRDBG(("entering spec\n"));
                    bInSpec = TRUE;
                }
            }
            else
            {
                // normal character
                outchar(c);
                ++p;
            }
        }
    }

    outchar(FMT_CHAR_TYPE('\0'));

    // if hr == pkS_FALSE here, no rewrite was necessary,
    // so we need not worry about the output buffer.
    if(hr == pkS_OK)
    {
        if(cchOutRemaining < 0)
        {
            //if we ran out of buffer, return pkE_INSUFFICIENT_BUFFER,
            //but also make sure the output buffer is null-terminated
            hr = pkE_INSUFFICIENT_BUFFER;
            if(*pcchNewFormatLen > 0)
            {
                pszNewFormat[(*pcchNewFormatLen) - 1] = FMT_CHAR_TYPE('\0');
            }
        }

        //report to the caller how much output buffer was necessary.
        //if we ran out, cchOutRemaining will be negative by the
        //extra amount needed
        *pcchNewFormatLen -= cchOutRemaining;
    }

bail:
    return hr;
}

// ----------------------
// The external functions
// ----------------------

LPSTR RewriteFormatStringA(LPCSTR pszFormat)
{
    int cchBuf = 0;
    LPSTR pszBuf = NULL;

    pkRESULT hr = RewriteFormatStringWorkerA(pszFormat, pszBuf, &cchBuf);
    if(hr == pkE_INSUFFICIENT_BUFFER)
    {
        pszBuf = Executive_Alloc((uint32_t)(sizeof(char) * cchBuf), FALSE);
        if(NULL != pszBuf)
        {
            hr = RewriteFormatStringWorkerA(pszFormat, pszBuf, &cchBuf);
            if(pkFAILED(hr))
            {
                Executive_Free(pszBuf);
                pszBuf = NULL;
            }
        }
    }

    return pszBuf;
}

LPWSTR RewriteFormatStringW(LPCWSTR pszFormat)
{
    int cchBuf = 0;
    LPWSTR pszBuf = NULL;

    pkRESULT hr = RewriteFormatStringWorkerW(pszFormat, pszBuf, &cchBuf);
    if(hr == pkE_INSUFFICIENT_BUFFER)
    {
        pszBuf = Executive_Alloc((uint32_t)(sizeof(wchar_t) * cchBuf), FALSE);
        if(NULL != pszBuf)
        {
            hr = RewriteFormatStringWorkerW(pszFormat, pszBuf, &cchBuf);
            if(pkFAILED(hr))
            {
                Executive_Free(pszBuf);
                pszBuf = NULL;
            }
        }
    }

    return pszBuf;
}

void FreeFormatStringA(char *pBuf)
{
    if(pBuf)
    {
        Executive_Free(pBuf);
    }
}

void FreeFormatStringW(WCHAR *pBuf)
{
    if(pBuf)
    {
        Executive_Free(pBuf);
    }
}

#endif // !STRINGSAFE_NO_FORMAT_REWRITE
