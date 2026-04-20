///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PALPRINT_ERROR
#define PALPRINT_ERROR                  1
#endif

#ifndef PALPRINT_WARNING
#define PALPRINT_WARNING                1
#endif


#ifndef PALPRINT_AVRENDERER
#define PALPRINT_AVRENDERER             1
#endif

#ifndef PALPRINT_AVRENDERER_VERBOSE
#define PALPRINT_AVRENDERER_VERBOSE     0
#endif

#ifndef PALPRINT_AVRENDERER_BUFFERPOOL
#define PALPRINT_AVRENDERER_BUFFERPOOL  0
#endif

#ifndef PALPRINT_AVRENDERER_BUFFERPOOL_VERBOSE
#define PALPRINT_AVRENDERER_BUFFERPOOL_VERBOSE  0
#endif


#ifndef PALPRINT_EXECUTIVE
#define PALPRINT_EXECUTIVE              1
#endif

#ifndef PALPRINT_EXECUTIVE_VERBOSE
#define PALPRINT_EXECUTIVE_VERBOSE      0
#endif


#ifndef PALPRINT_SOCKETS
#define PALPRINT_SOCKETS                1
#endif

#ifndef PALPRINT_SOCKETS_VERBOSE
#define PALPRINT_SOCKETS_VERBOSE        0
#endif


extern void pkAPI DebugPrint( const char *pFmt, ... );

// base PALPRINTMSG on the same condition as PRINTMSG
#ifdef PRINTMSG_ENABLED

void    pkAPI   Executive_DebugPrintf( const char *pFmt, ... );

#define PALPRINTMSG( cond, printf_exp ) \
    ( ( cond ) ? ( Executive_DebugPrintf printf_exp ), 1 : 0 ) // TODO: was DebugPrint

#else // PRINTMSG_ENABLED

#define PALPRINTMSG( cond, printf_exp )   ( void ) 0

#endif // PRINTMSG_ENABLED

#ifdef __cplusplus
}
#endif

