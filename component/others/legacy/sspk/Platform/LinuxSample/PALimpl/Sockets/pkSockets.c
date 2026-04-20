///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  pkSockets.c
//
//  Implementation of the interface for network sockets
//
///////////////////////////////////////////////////////////////////////////////

#include <pkPAL.h>
#include <pkSockets.h>

#include <platPrivate.h>
#include <palPrint.h>

#undef s_addr

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <errno.h>

#include "pkDnsCache.h"


static int32_t _isExitNetworkByUser = 0;//add for network retry function

static void s_SetThreadLastSocketError(int32_t iLastError)
{
    pkRESULT pkRet = pkS_OK;
    switch (iLastError)
    {
    case SOCKET_FAILURE:
        switch (errno)
        {
        case EWOULDBLOCK:
        case EINPROGRESS:
            pkRet = pkE_SOCKET_WOULDBLOCK;
            break;

        case EADDRINUSE:
            pkRet = pkE_SOCKET_ADDRINUSE;
            break;

        case EINVAL:
        case EAFNOSUPPORT:
        case EPROTONOSUPPORT:
        case EOPNOTSUPP:
        case EBADF:
            pkRet = pkE_INVALIDARG;
            break;

        case ENOBUFS:
        case ENOMEM:
            pkRet = pkE_OUTOFMEMORY;
            break;

        case ENOTRECOVERABLE:
        case ENOTSOCK:
        case ENOTCONN: /* The specified socket is not connected. */
            pkRet = pkE_SOCKET_NOTSOCK;
            break;

        case ESHUTDOWN:
            pkRet = pkE_SOCKET_SHUTDOWN;
            break;

        default:
            PALPRINTMSG(PALPRINT_SOCKETS_VERBOSE, ("Received an invalid socket code with ERRNO: 0x%08X\n", errno));
            break;
        }
        break;

    case SOCKET_SUCCESS:
        break;

    default:
        PALPRINTMSG(PALPRINT_SOCKETS_VERBOSE, ("Received a socket error code: 0x%08X\n", iLastError));
        pkRet = pkE_FAIL;
        break;
    }

    SetThreadLastSocketError_priv(pkRet);
}

// Definitions of datatypes and constants for the socket PAL are provided
// by the platform layer in platSockets.h

int32_t pkAPI Socket_Startup(void)
{
    return 0;
}

int32_t pkAPI Socket_Cleanup(void)
{
    return 0;
}

int32_t pkAPI Socket_SetExitNetworkFlag(int32_t flag)
{
	_isExitNetworkByUser = flag;
    return 0;
}

int32_t pkAPI Socket_GetExitNetworkFlag()
{
	return _isExitNetworkByUser;
}



/*
 domain, type and protocol defined in /usr/include/bits/socket.h


 function prototype from /usr/include/sys/socket.h
 int socket (int __domain, int __type, int __protocol)
*/
SOCKET_HANDLE pkAPI Socket_Socket(int32_t af, int32_t type, int32_t protocol)
{
    int handle = (int)SOCKET_INVALID_HANDLE;

    handle = socket(af, type, protocol);
    
    if((int)SOCKET_INVALID_HANDLE==handle)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s call failed af(%d), type(%d), protocol(%d), errno: 0x%x. \n", 
            __FUNCTION__, af, type, protocol, errno));
        s_SetThreadLastSocketError(SOCKET_FAILURE);
    }
    else
    {
        s_SetThreadLastSocketError(SOCKET_SUCCESS);
    }

    return (SOCKET_HANDLE)handle;
}

/*
  function prototype from /usr/include/sys/socket.h
  int shutdown (int __fd, int __how)
 */

int32_t pkAPI Socket_Shutdown(SOCKET_HANDLE hSOCKET, int32_t how)
{
    int32_t retval = SOCKET_FAILURE;
    int32_t and_how = 0;
    bool_t invalidParam = FALSE;

    switch(how)
    {
        case SOCKET_D_RECEIVE: and_how = SHUT_RD;   break;
        case SOCKET_D_SEND:    and_how = SHUT_WR;   break;
        case SOCKET_D_BOTH:    and_how = SHUT_RDWR; break;
        default: invalidParam = TRUE;               break;
    }

    if((invalidParam ) || (SOCKET_INVALID_HANDLE == hSOCKET))
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), how(%d). \n",
            __FUNCTION__, hSOCKET, how));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = shutdown((int)hSOCKET, (int)and_how);
    if(retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), how(%d) errno: 0x%x. \n",
            __FUNCTION__, hSOCKET, how, errno));
        retval = SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

/*
 function prototype from /usr/include/sys/socket.h
 int connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
 */

int32_t pkAPI Socket_Connect(SOCKET_HANDLE hSOCKET, const struct SOCKET_SOCKADDR *name, int32_t namelen)
{
    int32_t retval = SOCKET_FAILURE;

    if((SOCKET_INVALID_HANDLE == hSOCKET) || (NULL == name))
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), name(0x%x). \n",
            __FUNCTION__, hSOCKET, name));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = connect((int)hSOCKET, (const struct sockaddr *)name, namelen);
    if(retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), name(0x%x) errno: 0x%x. \n",
            __FUNCTION__, hSOCKET, name, errno));
        retval = SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

/*
 function prototype from /usr/include/sys/socket.h
 ssize_t recv (int __fd, void *__buf, size_t __n, int __flags);
 */
int32_t pkAPI Socket_Recv(SOCKET_HANDLE hSocket, int8_t * buf, int32_t len, int32_t flags)
{
    int32_t retval = SOCKET_FAILURE;

    if ((SOCKET_INVALID_HANDLE == hSocket) || (NULL == buf) )
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), buf(0x%x). \n",
            __FUNCTION__, hSocket, buf));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = recv((int)hSocket, buf, len, flags);

    if(retval <= 0)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), buf(0x%x) errno: 0x%x. \n",
            __FUNCTION__, hSocket, buf, errno));

        retval = SOCKET_FAILURE;
        s_SetThreadLastSocketError(retval);
    }
    //else if(0 == retval)
    //{
    //    PALPRINTMSG(PALPRINT_ERROR, ("%s recv returned zero (peer orderly shutdown): socket(0x%x)\n",__FUNCTION__, hSocket));
    //    s_SetThreadLastSocketError(SOCKET_SUCCESS);
    //}
    else
    {
        s_SetThreadLastSocketError(SOCKET_SUCCESS);
    }
    return retval;
}

/*
 function prototype from /usr/include/sys/socket.h
 ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
                  int __flags, __SOCKADDR_ARG __addr,
                  socklen_t *__restrict __addr_len);
 */
int32_t pkAPI Socket_RecvFrom(SOCKET_HANDLE hSocket, int8_t * buf,
                                    int32_t len, int32_t flags,
                                    struct SOCKET_SOCKADDR *from, int32_t* fromlen)
{
    int32_t retval = SOCKET_FAILURE;

    if((SOCKET_INVALID_HANDLE == hSocket) || (NULL == buf) || (NULL == from) || (NULL == fromlen))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), buf(0x%x), from(0x%x), fromlen(0x%x). \n",
            __FUNCTION__, hSocket, buf, from, fromlen));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
     }

    retval = recvfrom((int)hSocket, buf, (size_t)len, (int)flags, (struct sockaddr *)from, (socklen_t*)fromlen);

    if (retval<0)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), buf(0x%x), from(0x%x), fromlen(0x%x) errno: 0x%x. \n",
            __FUNCTION__, hSocket, buf, from, fromlen, errno));

       s_SetThreadLastSocketError(retval);
        return SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(SOCKET_SUCCESS);
    return retval;
}


/*
 function prototype from /usr/include/sys/socket.h
 ssize_t send (int __fd, __const void *__buf, size_t __n, int __flags);
 */
int32_t pkAPI Socket_Send(SOCKET_HANDLE hSocket,const int8_t *buf, int32_t len, int32_t flags)
{
    int32_t retval = SOCKET_FAILURE;

    if((SOCKET_INVALID_HANDLE == hSocket) || (NULL == buf))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), buf(0x%x), len(%d), flags(%d). \n",
            __FUNCTION__, hSocket, buf, len, flags));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = send((int)hSocket, buf, len, (int)flags|MSG_NOSIGNAL);

    if (retval<0)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), buf(0x%x), len(%d), flags(%d) errno: 0x%x. \n",
            __FUNCTION__, hSocket, buf, len, flags, errno));

       s_SetThreadLastSocketError(retval);
        return SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(SOCKET_SUCCESS);
    return retval;
}

/*
 function prototype from /usr/include/sys/socket.h
 ssize_t sendto (int __fd, __const void *__buf, size_t __n,
               int __flags, __CONST_SOCKADDR_ARG __addr,
               socklen_t __addr_len);
 */

int32_t pkAPI Socket_SendTo(SOCKET_HANDLE hSocket, const int8_t *buf, int32_t len, int32_t flags, SOCKET_SOCKADDR *to, int32_t token)
{
    int32_t retval = SOCKET_FAILURE;

    if((SOCKET_INVALID_HANDLE == hSocket) || (NULL == buf) || (NULL == to))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), buf(0x%x), len(%d), flags(%d), to(0x%x), token(%d). \n",
            __FUNCTION__, hSocket, buf, len, flags, to, token));

       SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = sendto((int)hSocket, buf, len, flags, (const struct sockaddr *)to, token);
    
    if (retval<0)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), buf(0x%x), len(%d), flags(%d), to(0x%x), token(%d) errno: 0x%x. \n",
            __FUNCTION__, hSocket, buf, len, flags, to, token, errno));

        s_SetThreadLastSocketError(retval);
        return SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(SOCKET_SUCCESS);
    return retval;
}

/*
 function prototype from /usr/include/sys/socket.h
 int bind (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
 */

int32_t pkAPI Socket_Bind(SOCKET_HANDLE hSocket, const struct SOCKET_SOCKADDR *addr, int32_t addrlen)
{
    int32_t retval = SOCKET_FAILURE;

    if((SOCKET_INVALID_HANDLE == hSocket) || (NULL == addr))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), addr(0x%x), addrlen(%d). \n",
            __FUNCTION__, hSocket, addr, addrlen));

       SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = bind((int)hSocket, (const struct sockaddr *)addr, addrlen);

    if (retval<0)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), addr(0x%x), addrlen(%d) errno: 0x%x. \n",
            __FUNCTION__, hSocket, addr, addrlen, errno));

        s_SetThreadLastSocketError(retval);
        return SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(SOCKET_SUCCESS);
    return retval;
}

/*
 function prototype from /usr/include/sys/socket.h
 int listen (int __fd, int __n);
 */

int32_t pkAPI Socket_Listen (SOCKET_HANDLE hSocket, int32_t backlog)
{
    int32_t retval = SOCKET_FAILURE;

    if(SOCKET_INVALID_HANDLE == hSocket)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), backlog(%d). \n",
            __FUNCTION__, hSocket, backlog));

       SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }
    retval = listen((int)hSocket, backlog);
    if (retval<0)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), backlog(%d) errno: 0x%x. \n",
            __FUNCTION__, hSocket, backlog, errno));

        s_SetThreadLastSocketError(retval);
        return SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(SOCKET_SUCCESS);
    return retval;
}

/*
 function prototype from /usr/include/sys/socket.oh
 int accept (int __fd, __SOCKADDR_ARG __addr,
           socklen_t *__restrict __addr_len);
 */

SOCKET_HANDLE pkAPI Socket_Accept (SOCKET_HANDLE hSocket, struct SOCKET_SOCKADDR *addr, int32_t *addrlen)
{
    SOCKET_HANDLE handle = SOCKET_INVALID_HANDLE;

    if((SOCKET_INVALID_HANDLE == hSocket) || (NULL == addr) || (NULL == addrlen))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), addr(0x%x), addrlen(0x%x). \n",
            __FUNCTION__, hSocket, addr, addrlen));

        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return handle;
    }

    handle = (SOCKET_HANDLE)accept((int)hSocket, (struct sockaddr *) addr, (socklen_t*)addrlen);
        
    if(SOCKET_INVALID_HANDLE == handle)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), addr(0x%x), addrlen(0x%x) errno: 0x%x. \n",
            __FUNCTION__, hSocket, addr, addrlen, errno));
    }
    s_SetThreadLastSocketError((uint32_t)handle);
    return handle;
}

/*
 function prototype from /usr/include/sys/select.h

 int select (int __nfds, fd_set *__restrict __readfds,
           fd_set *__restrict __writefds,
           fd_set *__restrict __exceptfds,
           struct timeval *__restrict __timeout);
 */

int32_t pkAPI Socket_Select (int32_t nfds, SOCKET_FD_SET *readfds, SOCKET_FD_SET *writefds, SOCKET_FD_SET *exceptfds, const struct SOCKET_TIMEVAL *timeout)
{
    int32_t retval = SOCKET_FAILURE;
    int largest_fd = 0;
    //need to guard against NULL pointers.
    fd_set * read = NULL;
    fd_set * write = NULL;
    fd_set * xcept = NULL;

    if(NULL != readfds)
    {
        read = (fd_set *)&(readfds->fdset);
        largest_fd = readfds->maxfd;
    }

    if(NULL != writefds)
    {
        write = (fd_set *)&(writefds->fdset);
        if(largest_fd < writefds->maxfd)
        {
            largest_fd = writefds->maxfd;
        }
    }
    if(NULL != exceptfds)
    {
        xcept = (fd_set *)&(exceptfds->fdset);
        if(largest_fd < exceptfds->maxfd)
        {
            largest_fd = exceptfds->maxfd;
        }
    }

    retval = select(largest_fd+1,
                    read,
                    write,
                    xcept,
                    (struct timeval *)timeout);


    if (retval<0)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! nfds(%d), readfds(0x%x), writefds(0x%x), exceptfds(0x%x), timeout(0x%x), errno: %x \n",
            __FUNCTION__, nfds, readfds, writefds, exceptfds, timeout, errno));

        s_SetThreadLastSocketError(retval);
        return SOCKET_FAILURE;
    }

    s_SetThreadLastSocketError(SOCKET_SUCCESS);
    return retval;
}

/*
 function prototype from /usr/include/sys/select.h
 #define FD_ISSET(fd, fdsetp)    __FD_ISSET (fd, fdsetp)
 */
int32_t pkAPI Socket_FDIsSet(SOCKET_HANDLE hSocket, SOCKET_FD_SET* set)
{
    int32_t result = SOCKET_FAILURE;

    if((SOCKET_INVALID_HANDLE == hSocket) || (NULL == set))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), set(0x%x). \n",
            __FUNCTION__, hSocket, set));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return result;
    }
    
    result = FD_ISSET((int)hSocket, (fd_set *)&(set->fdset));
    
    if(0 != result)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), set(0x%x) errno: %x \n", __FUNCTION__, hSocket, set, errno));
    }
    s_SetThreadLastSocketError(result);
    return result;
}

/*
 /usr/include/asm/ioctls.h

 /usr/include/sys/ioctl.h

 int ioctl (int __fd, unsigned long int __request, ...)

 */
int32_t pkAPI Socket_GetBytesAvailable(SOCKET_HANDLE hSocket)
{
    int32_t retval= SOCKET_FAILURE;
    int bytes_available = 0;

     if(SOCKET_INVALID_HANDLE == hSocket)
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x). \n",
            __FUNCTION__, hSocket));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = ioctl((int)hSocket, FIONREAD, &bytes_available);

    if((int32_t)SOCKET_INVALID_HANDLE != retval)
    {
        s_SetThreadLastSocketError(SOCKET_SUCCESS);
        retval = bytes_available;
    }
    else
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed!  hSocket(0x%x) errno: %x \n", __FUNCTION__, hSocket, errno));
        s_SetThreadLastSocketError(SOCKET_FAILURE);
        return SOCKET_FAILURE;
    }

    return retval;
}


int32_t pkAPI Socket_SetSocketOpt(SOCKET_HANDLE hSocket, int32_t level, int32_t optname, const int8_t * optval, int32_t optlen)
{
    int32_t retval = SOCKET_FAILURE;
    bool_t invalidParam = FALSE;
    struct linger dont_linger;
    uint8_t ttl = 0;
    int32_t length = 0;
    SOCKET_IP_MREQ * pMreq = NULL;
    struct ip_mreq mreq;

    if(SOCKET_INVALID_HANDLE == hSocket)
    {
        invalidParam = TRUE;
        goto exit;
    }

    switch(level)
    {
        case SOCKET_L_SOCKET:
        {
            switch(optname)
            {
                case SOCKET_REUSEADDR:
                {
                    retval = setsockopt((int)hSocket, SOL_SOCKET, SO_REUSEADDR, optval, optlen);
                    break;
                }
                case SOCKET_DONTLINGER: //There is no 'Don't Linger' option, so just negate the Linger option
                {
                    dont_linger.l_onoff = 0; //non zero to linger on close
                    dont_linger.l_linger = 0;//time to linger before closing
                    retval = setsockopt((int)hSocket, SOL_SOCKET, SO_LINGER, &dont_linger, sizeof(dont_linger));
                    break;
                }
                case SOCKET_LINGER:
                {
                    retval = setsockopt((int)hSocket, SOL_SOCKET, SO_LINGER, optval, optlen);
                    break;
                }
                case SOCKET_KEEPALIVE:
                {
                    retval = setsockopt((int)hSocket, SOL_SOCKET, SO_KEEPALIVE, optval, optlen);
                    break;
                }
                case SOCKET_BROADCAST:
                {
                    retval = setsockopt((int)hSocket, SOL_SOCKET, SO_BROADCAST, optval, optlen);
                    break;
                }
                /*
                case SOCKET_DEBUG:

                case SOCKET_TCP_NODELAY:
                case SOCKET_MULTICAST_DROP_MEMBERSHIP:
                case SOCKET_MULTICAST_TTL:
                case SOCKET_MULTICAST_ADD_MEMBERSHIP:
                */
                default:
                {
                    invalidParam = TRUE;
                    break;
                }

            }
            break; //case SOCKET_L_SOCKET
        }

        case SOCKET_IPPROTO_TCP:
        {
            switch(optname)
            {
                case SOCKET_TCP_NODELAY:
                {
                    retval = setsockopt((int)hSocket, IPPROTO_TCP, TCP_NODELAY, optval, optlen);
                    break;
                }

                case SOCKET_DEBUG:
                {
                    retval = setsockopt((int)hSocket, IPPROTO_TCP, SO_DEBUG, optval, optlen);
                    break;
                }

                /*
                case SOCKET_SO_SNDBUF:
                case SOCKET_REUSEADDR:
                case SOCKET_DONTLINGER:
                case SOCKET_MULTICAST_DROP_MEMBERSHIP:
                case SOCKET_MULTICAST_TTL:
                case SOCKET_MULTICAST_ADD_MEMBERSHIP:
                case SOCKET_LINGER:
                */
                default:
                {
                    invalidParam = TRUE;
                    break; 
                }
            }

            break; //case SOCKET_IPPROTO_TCP
        }
        case SOCKET_IPPROTO_IP:
        {
            switch(optname)
            {

                case SOCKET_MULTICAST_DROP_MEMBERSHIP:
                {
                    pMreq = (SOCKET_IP_MREQ*)optval;
                    memset(&mreq, 0, sizeof(mreq));

                    if(pMreq == NULL)
                    {
                        retval = SOCKET_FAILURE;
                        break;
                    }

                    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                    mreq.imr_multiaddr.s_addr = pMreq->imr_multiaddr.S_un.S_addr;
                    retval = setsockopt((int)hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
                    break;
                }
                case SOCKET_MULTICAST_TTL:
                {
                    retval = setsockopt((int)hSocket, IPPROTO_IP, IP_MULTICAST_TTL, optval, optlen);
                    break;
                }
                case SOCKET_MULTICAST_ADD_MEMBERSHIP:
                {
                    pMreq = (SOCKET_IP_MREQ*)optval;
                    memset(&mreq, 0, sizeof(mreq));

                    if(pMreq == NULL)
                    {
                        retval = SOCKET_FAILURE;
                        break;
                    }

                    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                    mreq.imr_multiaddr.s_addr = pMreq->imr_multiaddr.S_un.S_addr;
                    retval = setsockopt((int)hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

                    break;
                }
                /*
                case SOCKET_DEBUG:
                case SOCKET_SO_SNDBUF:
                case SOCKET_TCP_NODELAY:
                case SOCKET_REUSEADDR:
                case SOCKET_DONTLINGER:
                case SOCKET_LINGER:
                */
                default:
                {
                    invalidParam = TRUE;
                    break;
                }

            }

            break; //case SOCKET_IPPROTO_IP
        }
        default:
        {
            invalidParam = TRUE;
            break;
        }
    }

exit:
    if(invalidParam)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter set: hSocket(0x%x), level(0x%x), optname(0x%x). \n",
            __FUNCTION__, hSocket, level, optname));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), level(0x%x), optname(0x%x) errno: 0x%x \n", 
            __FUNCTION__, hSocket, level, optname, errno));
    }
    s_SetThreadLastSocketError(retval);
    return retval;
}

/*
 int getsockname (int __fd, __SOCKADDR_ARG __addr,
            socklen_t *__restrict __len)
 */

int32_t pkAPI Socket_GetSocketName (SOCKET_HANDLE hSocket, struct SOCKET_SOCKADDR *name, int32_t *namelen)
{
    int32_t retval = SOCKET_FAILURE;
    
    if((SOCKET_INVALID_HANDLE == hSocket) || (NULL == name) || (NULL == namelen))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter set: hSocket(0x%x), name(0x%x) namelen(0x%x) \n",
            __FUNCTION__, hSocket, name, namelen));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = getsockname((int)hSocket, (struct sockaddr *)name, (socklen_t*)namelen);

    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), name(0x%x) namelen(0x%x) errno: 0x%x \n", 
            __FUNCTION__, hSocket, name, namelen, errno));
    }
    s_SetThreadLastSocketError(retval);
    return retval;
}

/*
 int getpeername (int __fd, __SOCKADDR_ARG __addr,
            socklen_t *__restrict __len)
 */

int32_t pkAPI Socket_GetPeerName (SOCKET_HANDLE hSocket, struct SOCKET_SOCKADDR *name, int32_t *namelen)
{
    int32_t retval = SOCKET_FAILURE;

    if((SOCKET_INVALID_HANDLE == hSocket)|| (NULL == name) || (NULL == namelen))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter set: hSocket(0x%x), name(0x%x), namelen(0x%x). \n",
            __FUNCTION__, hSocket, name, namelen));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = getpeername((int)hSocket, (struct sockaddr *)name, (socklen_t*)namelen);
    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), name(0x%x), namelen(0x%x) errno: 0x%x \n", 
            __FUNCTION__, hSocket, name, namelen, errno));
    }
    s_SetThreadLastSocketError(retval);
    return retval;
}

/*
 defined in /usr/include/netdb.h

 int getnameinfo (__const struct sockaddr *__restrict __sa,
            socklen_t __salen, char *__restrict __host,
            socklen_t __hostlen, char *__restrict __serv,
            socklen_t __servlen, unsigned int __flags)
 */
int32_t pkAPI Socket_GetNameInfo
    (
        const SOCKET_SOCKADDR *sa,
        int32_t salen,
        int8_t *host,
        uint32_t hostlen,
        int8_t *serv,
        uint32_t servlen,
        int32_t flags
    )
{
    int32_t retval = SOCKET_FAILURE;
    int new_flags = 0;
    bool_t invalidParam = FALSE;
    switch(flags)
    {
        case SOCKET_NI_NOFQDN:
        {
            new_flags = NI_NOFQDN;
            break;
        }
        case SOCKET_NI_NUMERICHOST:
        {
            new_flags = NI_NUMERICHOST;
            break;
        }
        case SOCKET_NI_NAMEREQD:
        {
            new_flags = NI_NAMEREQD;
            break;
        }
        case SOCKET_NI_NUMERICSERV:
        {
            new_flags = NI_NUMERICSERV;
            break;
        }
        case SOCKET_NI_DGRAM:
        {
            new_flags = NI_DGRAM;
            break;
        }
        default:
        {
            invalidParam = TRUE;
            break;
        }
    }

    // In getnameinfo, if the hostlen is 0, it means the caller doesn't want
    // the result, and the retval is 0 (SOCKET_SUCCESS). It is unexpected.    
    if ((hostlen <= 1) || (invalidParam))
    {
       PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: sa(0x%x), salen(%d), host(0x%x), hostlen(%d), serv(0x%x), servlen(%d), flags(%d). \n",
            __FUNCTION__, sa, host, hostlen, serv, servlen, flags));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    // On Fedora 6.0, there has buffer overflow bug in getnameinfo:
    // one byte off given host buffer will be written with '\0' if the hostlen is
    // equal to the length of host name not inlcuding terminating NULL character.
    // Here, decrease the hostlen to avoid this situation.
    hostlen -= 1;

    retval = getnameinfo((struct sockaddr *)sa, salen, (char *)host, hostlen, (char *)serv, servlen, new_flags);
    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! sa(0x%x), salen(%d), host(0x%x), hostlen(%d), serv(0x%x), servlen(%d), flags(%d) errno: 0x%x \n", 
            __FUNCTION__, sa, host, hostlen, serv, servlen, flags, errno));
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

int32_t pkAPI Socket_GetAddrInfo
    (
        const char *hostname,
        const char *servname,
        const struct SOCKET_ADDR_INFO *hints,
        struct SOCKET_ADDR_INFO **ai
    )
{
    //we may want to do more than this, there is concern that this call my block forever
    return getaddrinfo(hostname, servname, (struct addrinfo *) hints, (struct addrinfo **) ai);
}

void pkAPI Socket_FreeAddrInfo(struct SOCKET_ADDR_INFO *ai)
{
    freeaddrinfo((struct addrinfo *) ai);
}


int32_t pkAPI Socket_GetAddrInfo_DnsCache
    (
        const char *hostname,
        const char *servname,
        const struct SOCKET_ADDR_INFO *hints,
        struct SOCKET_ADDR_INFO **ai
    )
{
    //we may want to do more than this, there is concern that this call my block forever
    return mss_getaddrinfo((char *)hostname, (char *)servname, (struct addrinfo *) hints, (struct addrinfo **) ai);
}

void pkAPI Socket_FreeAddrInfo_DnsCache(char *hostname, char *port, struct SOCKET_ADDR_INFO *ai)
{
    mss_freeaddrinfo(hostname, port, (struct addrinfo *) ai);
}


pkRESULT pkAPI Socket_GetResultFromLastErr(void)
{
    return GetThreadLastSocketError_priv();
}

/*
  defined in /usr/include/fcntl.h and /usr/include/bits/fcntl.h
  int fcntl (int __fd, int __cmd, ...)
 */

int32_t pkAPI Socket_SetNonBlockingMode(SOCKET_HANDLE hSocket, bool_t state)
{
    int32_t retval = SOCKET_FAILURE;
    int original_flags = 0;
    int new_flag = state ? O_NONBLOCK : ~O_NONBLOCK;

    if(SOCKET_INVALID_HANDLE == hSocket)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), state(%s). \n",
            __FUNCTION__, hSocket, state ? "TRUE" : "FALSE"));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    original_flags = fcntl((int)hSocket, F_GETFL); //Don't lose any of the original settings

    retval = fcntl((int)hSocket, F_SETFL, original_flags | new_flag);
    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), state(%s) errno: 0x%x \n", 
            __FUNCTION__, hSocket, state ? "TRUE" : "FALSE", errno));
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

int32_t pkAPI Socket_SetRecvBuffer(SOCKET_HANDLE hSocket, int32_t iVal)
{
    int32_t retval = SOCKET_FAILURE;
    int32_t size = iVal;

    if(SOCKET_INVALID_HANDLE == hSocket)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), iVal(%d). \n",
            __FUNCTION__, hSocket, iVal));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }

    retval = setsockopt((int)hSocket, SOL_SOCKET, SO_RCVBUF, &iVal, sizeof(int32_t));
    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), iVal(%d) errno: 0x%x \n", 
            __FUNCTION__, hSocket, iVal, errno));
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

int32_t pkAPI Socket_SetSendBuffer(SOCKET_HANDLE hSocket, int32_t iVal)
{
    int32_t retval = SOCKET_FAILURE;
    int32_t size = iVal;

    if(SOCKET_INVALID_HANDLE == hSocket)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), iVal(%d). \n",
            __FUNCTION__, hSocket, iVal));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }
    retval = setsockopt((int)hSocket, SOL_SOCKET, SO_SNDBUF, &iVal, sizeof(int32_t));
    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), iVal(%d) errno: 0x%x \n", 
            __FUNCTION__, hSocket, iVal, errno));
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

int32_t pkAPI Socket_GetRecvBufferSize(SOCKET_HANDLE hSocket, int32_t *pSizeResult)
{
    int32_t retval = SOCKET_FAILURE;
    size_t nOptLen = sizeof(int32_t);

    if(SOCKET_INVALID_HANDLE == hSocket)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x), pSizeResult(0x%x). \n",
            __FUNCTION__, hSocket, pSizeResult));

        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }
    retval = getsockopt((int)hSocket, SOL_SOCKET, SO_RCVBUF, (char*)pSizeResult, &nOptLen);
    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x), pSizeResult(0x%x) errno: 0x%x \n", 
            __FUNCTION__, hSocket, pSizeResult, errno));
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

int32_t pkAPI Socket_CloseSocket(SOCKET_HANDLE hSocket)
{
    int32_t retval = SOCKET_FAILURE;
    
    if(SOCKET_INVALID_HANDLE == hSocket)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! Invalid input parameter: hSocket(0x%x). \n",
            __FUNCTION__, hSocket));
        SetThreadLastSocketError_priv(pkE_INVALIDARG);
        return retval;
    }
    retval = close((int)hSocket);
    if(0 != retval)
    {
        PALPRINTMSG(PALPRINT_ERROR, ("%s failed! hSocket(0x%x) errno: 0x%x \n", 
            __FUNCTION__, hSocket, errno));
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

/*
    inet_aton() is defined in /usr/include/arpa/inet.h
    int inet_aton (__const char *__cp, struct in_addr *__inp)
 */
uint32_t pkAPI Socket_INetAddr (const int8_t *cp)
{
    uint32_t retval = SOCKET_INADDR_NONE;
    struct in_addr Addr;

    if (0 != inet_aton((const char *)cp, &Addr))
    {
        retval = Addr.s_addr;
    }

    s_SetThreadLastSocketError(retval);
    return retval;
}

char* pkAPI Socket_INet_ntoa(struct SOCKET_IN_ADDR in)
{
    return inet_ntoa(*(struct in_addr *) &in);
}


void pkAPI Socket_FDZero(SOCKET_FD_SET * set)
{
    FD_ZERO((fd_set*)&(set->fdset));
    ((SOCKET_FD_SET *)(set))->maxfd = 0;

}

void pkAPI Socket_FDClr(SOCKET_HANDLE fd, SOCKET_FD_SET * set)
{
    FD_CLR((int)fd, (fd_set*)&(set->fdset));
}

void pkAPI Socket_FDSet(SOCKET_HANDLE fd, SOCKET_FD_SET * set)
{
    FD_SET((int)fd,(fd_set*)&(set->fdset));
    if((int)fd > ((SOCKET_FD_SET *)(set))->maxfd)
    {
        ((SOCKET_FD_SET *)set)->maxfd = (int)fd;
    }
}


uint32_t pkAPI Socket_htonl(uint32_t host32)
{
    return htonl(host32);
}

uint16_t pkAPI Socket_htons(uint16_t host16)
{
    return htons(host16);
}

uint32_t pkAPI Socket_ntohl(uint32_t net32)
{
    return ntohl(net32);
}

uint16_t pkAPI Socket_ntohs(uint16_t net16)
{
    return ntohs(net16);
}



