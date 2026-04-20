///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include <pkPAL.h>

LPSTR RewriteFormatStringA(LPCSTR pszFormat);
LPWSTR RewriteFormatStringW(LPCWSTR pszFormat);

void FreeFormatStringA(char *pBuf);
void FreeFormatStringW(WCHAR *pBuf);
