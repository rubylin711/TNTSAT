///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pkExecutive.h"

#include <memory.h>
#include <string.h>
#include <string>

#include "Trace.h"
#include "StringUtils.h"

using namespace std;

//================================================================
//================================================================
Tracer::Tracer(const char* file, int line, int flags)
    : _file(file)
    , _line(line)
    , _flags(flags)
{
}

void Tracer::Write(const char* str)
{
    Executive_DebugPrintf("%s\n", str);
}

void Tracer::operator()(const std::string& str)
{
    Executive_DebugPrintf("%s\n", str.c_str());
}

void Tracer::operator()(const char* format, ...)
{
    pkRESULT pkResult;

    char szFormatted[ 1024 ];
    char* pszPrefixEnd = szFormatted;
    size_t cchPostfix = 0;

    // If file and line given, output that first
        if ((0 != (_flags & Flag_FileAndLine)) && (NULL != _file))
    {
        size_t cchFileLen;
        pkResult = StringCchLengthA(_file, 250, &cchFileLen);
        if (pkSUCCEEDED(pkResult) && cchFileLen > 0)
        {
            const char* szFileName = &_file[cchFileLen];
            while (szFileName != _file && szFileName[-1] != '/' && szFileName[-1] != '\\')
            {
                szFileName--;
            }

            StringCchPrintfExA(szFormatted, sizeof(szFormatted), &pszPrefixEnd, &cchPostfix, 0,
                "%s(%i) %s", szFileName, _line, (0 != (_flags & Flag_Error)) ? "# " : "");
        }
    }

    // Append the specified formatted data
    {
        va_list args;
        va_start(args, format);

        // Note: Using strsafe API gives the platform a chance to convert MS format string extensions (e.g. %I64)
        pkResult = StringCchVPrintfA( pszPrefixEnd, cchPostfix, format, args );

        va_end(args);

        if (pkFAILED(pkResult)) // assume truncated
        {
            szFormatted[sizeof(szFormatted)-1] = '\0';
            szFormatted[sizeof(szFormatted)-2] = '.';
            szFormatted[sizeof(szFormatted)-3] = '.';
            szFormatted[sizeof(szFormatted)-4] = '.';
            szFormatted[sizeof(szFormatted)-5] = '*';
        }

        Executive_DebugPrintf("%s\n", szFormatted);
    }
}

void Tracer::AssertFailed(const char* how)
{
    (*this)("Assert Failed - %s",how);

    Executive_DebugBreak();
}

