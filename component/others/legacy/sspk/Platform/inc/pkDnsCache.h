///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  pkSockets.h
//
//  PAL definition of Sockets networking PAL
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pkPAL.h>
#include <platSockets.h>

#ifdef __cplusplus
extern "C"
{
#endif

int uninitDnsCache();
int initDnsCache(int cnt);

int mss_freeaddrinfo(char *hostname, char *port, struct  addrinfo *ai);
int mss_getaddrinfo(char * hostname, char * portstr,  struct addrinfo* hints,  struct addrinfo** ai);

#ifdef __cplusplus
}
#endif

