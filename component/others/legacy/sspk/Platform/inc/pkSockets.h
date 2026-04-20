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

#define SOCKET_INVALID_HANDLE         ((SOCKET_HANDLE)~0)
#define SOCKET_FAILURE                (-1)
#define SOCKET_SUCCESS                (0L)

// Definitions of datatypes and constants for the socket PAL are provided
// by the platform implementation in platSockets.h

int32_t pkAPI       Socket_Startup(void);
int32_t pkAPI       Socket_Cleanup(void);
SOCKET_HANDLE pkAPI Socket_Socket(int32_t af, int32_t type, int32_t protocol);
int32_t pkAPI       Socket_Shutdown(SOCKET_HANDLE hSOCKET, int32_t how);
int32_t pkAPI       Socket_Connect(SOCKET_HANDLE hSOCKET, const struct SOCKET_SOCKADDR *name, int32_t namelen);
int32_t pkAPI       Socket_Recv(SOCKET_HANDLE hSocket, int8_t * buf, int32_t len, int32_t flags);
int32_t pkAPI       Socket_RecvFrom(SOCKET_HANDLE hSocket, int8_t * buf, int32_t len, int32_t flags, struct SOCKET_SOCKADDR *from, int32_t* fromlen);
int32_t pkAPI       Socket_Send(SOCKET_HANDLE hSocket,const int8_t *buf, int32_t len, int32_t flags);
int32_t pkAPI       Socket_SendTo(SOCKET_HANDLE hSocket,const int8_t *buf, int32_t len, int32_t flags, SOCKET_SOCKADDR *to, int32_t token);
int32_t pkAPI       Socket_Bind(SOCKET_HANDLE hSocket, const struct SOCKET_SOCKADDR *addr, int32_t addrlen);
int32_t pkAPI       Socket_Listen (SOCKET_HANDLE hSocket, int32_t backlog);
SOCKET_HANDLE pkAPI Socket_Accept (SOCKET_HANDLE hSocket, struct SOCKET_SOCKADDR *addr, int32_t *addrlen);
int32_t pkAPI       Socket_Select (int32_t nfds, SOCKET_FD_SET *readfds, SOCKET_FD_SET *writefds, SOCKET_FD_SET *exceptfds, const struct SOCKET_TIMEVAL *timeout);
int32_t pkAPI       Socket_FDIsSet(SOCKET_HANDLE hSocket, SOCKET_FD_SET* set);
int32_t pkAPI       Socket_GetBytesAvailable(SOCKET_HANDLE hSocket);
int32_t pkAPI       Socket_SetSocketOpt(SOCKET_HANDLE hSocket, int32_t level, int32_t optname, const int8_t * optval, int32_t optlen);
int32_t pkAPI       Socket_GetSocketName (SOCKET_HANDLE hSocket, struct SOCKET_SOCKADDR *name, int32_t *namelen);
int32_t pkAPI       Socket_GetPeerName (SOCKET_HANDLE hSocket, struct SOCKET_SOCKADDR *name, int32_t *namelen);
int32_t pkAPI       Socket_GetNameInfo(const SOCKET_SOCKADDR *sa, int32_t salen, int8_t *host, uint32_t hostlen, int8_t *serv, uint32_t servlen, int32_t flags);
int32_t pkAPI       Socket_GetAddrInfo( const char *hostname, const char *servname, const struct SOCKET_ADDR_INFO *hints, struct SOCKET_ADDR_INFO **ai);
void    pkAPI       Socket_FreeAddrInfo(struct SOCKET_ADDR_INFO *ai);
pkRESULT pkAPI      Socket_GetResultFromLastErr(void);
int32_t pkAPI       Socket_SetNonBlockingMode(SOCKET_HANDLE hSocket, bool_t state);
int32_t pkAPI       Socket_SetRecvBuffer(SOCKET_HANDLE hSocket, int32_t iVal);
int32_t pkAPI       Socket_SetSendBuffer(SOCKET_HANDLE hSocket, int32_t iVal);
int32_t pkAPI       Socket_GetRecvBufferSize(SOCKET_HANDLE hSocket, int32_t *pSizeResult);
int32_t pkAPI       Socket_CloseSocket(SOCKET_HANDLE hSocket);
uint32_t pkAPI      Socket_INetAddr (const int8_t *cp);
char*   pkAPI       Socket_INet_ntoa(struct SOCKET_IN_ADDR in);

uint32_t pkAPI      Socket_htonl(uint32_t host32);
uint16_t pkAPI      Socket_htons(uint16_t host16);
uint32_t pkAPI      Socket_ntohl(uint32_t net32);
uint16_t pkAPI      Socket_ntohs(uint16_t net16);

int32_t pkAPI       Socket_SetExitNetworkFlag(int32_t flag);//add for network retry function
int32_t pkAPI       Socket_GetExitNetworkFlag();//add for network retry function

int32_t pkAPI       Socket_GetAddrInfo_DnsCache(const char *hostname,const char *servname,const struct SOCKET_ADDR_INFO *hints,struct SOCKET_ADDR_INFO **ai);
void    pkAPI       Socket_FreeAddrInfo_DnsCache(char *hostname, char *port, struct SOCKET_ADDR_INFO *ai);
#ifdef __cplusplus
}
#endif

