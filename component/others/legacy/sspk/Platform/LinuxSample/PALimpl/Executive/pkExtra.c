///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  pkExtra.c
//
//  Implementation of the methods and data types for the set of functions that
//  make up a portable Executive.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include <pkPAL.h>

#include <stdarg.h> // valist
#include <stdio.h>

void pkAPI DebugPrint( const char *pFmt, ... )
{
    va_list ap;

    va_start(ap, pFmt);
    vfprintf(stderr, pFmt, ap);
    va_end(ap);

    fflush(stderr);
    return;
}


