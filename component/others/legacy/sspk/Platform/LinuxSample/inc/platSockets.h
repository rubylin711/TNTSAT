///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __platSockets_h__
#define __platSockets_h__


/* socket type */
#define MAX_SOCKETS 256

#define SOCKET_MAX_INTERFACE_NAME_LEN 256

#define S_TCP_CHANNEL_SHUTDOWN  0x000B0001
#define S_UDP_CHANNEL_SHUTDOWN  0x000B0002


typedef pkHANDLE                SOCKET_HANDLE;
typedef pkHANDLE                SOCKET_EVENT;

typedef struct SOCKET_SOCKADDR
{
        uint16_t                sa_family;              /* address family */
        int8_t                  sa_data[14];            /* up to 14 bytes of direct address */
}SOCKET_SOCKADDR;


#define SOCKET_FD_SETSIZE      1024
#define NUMFDBITS (sizeof(unsigned char) * sizeof(unsigned long))
#define FD_SET_LONGS (SOCKET_FD_SETSIZE / NUMFDBITS)

/* The fd_set member is required to be an array of longs.  */
typedef long int socket_fd_mask;
typedef struct
{
    socket_fd_mask __fds_bits [FD_SET_LONGS];
}socket_fd_set;

typedef struct SOCKET_FD_SET
{
    int maxfd;
    socket_fd_set fdset;
}SOCKET_FD_SET;

#ifdef __cplusplus
extern "C" {
#endif

extern void pkAPI Socket_FDZero(SOCKET_FD_SET * set);
extern void pkAPI Socket_FDClr(SOCKET_HANDLE fd, SOCKET_FD_SET * set);
extern void pkAPI Socket_FDSet(SOCKET_HANDLE fd, SOCKET_FD_SET * set);
#ifdef __cplusplus
}
#endif

#define SOCKET_FD_CLR(fd, set)   Socket_FDClr((SOCKET_HANDLE)(fd), (SOCKET_FD_SET *)(set))
#define SOCKET_FD_ZERO(set)      Socket_FDZero((SOCKET_FD_SET *)(set))
#define SOCKET_FD_SET(fd, set)   Socket_FDSet((SOCKET_HANDLE)(fd), (SOCKET_FD_SET *)(set))
#define SOCKET_FD_ISSET(fd, set) Socket_FDIsSet((SOCKET_HANDLE)(fd), (SOCKET_FD_SET *)(set))

struct SOCKET_TIMEVAL {
        int32_t                 tv_sec;         /* seconds */
        int32_t                 tv_usec;        /* and microseconds */
};

#define _SOCKET_MAXSIZE 128                     /* Maximum size. */
#define _SOCKET_ALIGNSIZE (sizeof(uint64_t))    /* Desired alignment. */


/* Definitions used for SOCKET_ADDR_STORAGE structure paddings design. */
#define _SOCKET_PAD1SIZE (_SOCKET_ALIGNSIZE - sizeof (uint16_t))
#define _SOCKET_PAD2SIZE (_SOCKET_MAXSIZE - (sizeof (uint16_t) + _SOCKET_PAD1SIZE + _SOCKET_ALIGNSIZE))

typedef struct SOCKET_ADDR_STORAGE
{
    int16_t ss_family;                   /* Address family. */
    int8_t __ss_pad1[_SOCKET_PAD1SIZE];  /* 6 byte pad for alignment */
    int64_t __ss_align;                  /* Field to force desired structure. */
    int8_t  __ss_pad2[_SOCKET_PAD2SIZE]; /* 112 byte pad to achieve desired size; */
}SOCKET_ADDR_STORAGE;

typedef struct SOCKET_IN_ADDR
{
    union {
        union {
                struct { uint8_t s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { uint16_t s_w1,s_w2; } S_un_w;
                uint32_t S_addr;
        } S_un;
        uint32_t s_addr;
    };

//#define s_addr  S_un.S_addr
}SOCKET_IN_ADDR;

#define s_host  S_un.S_un_b.s_b2
#define s_net   S_un.S_un_b.s_b1
#define s_imp   S_un.S_un_w.s_w2
#define s_impno S_un.S_un_b.s_b4
#define s_lh    S_un.S_un_b.s_b3

/* Socket address, internet style. */
typedef struct SOCKET_SOCKADDR_IN
{
        int16_t                 sin_family;
        uint16_t                sin_port;
        struct  SOCKET_IN_ADDR  sin_addr;
        int8_t                  sin_zero[8];
}SOCKET_SOCKADDR_IN, *PSOCKET_SOCKADDR_IN;

typedef struct SOCKET_IP_MREQ
{
    struct SOCKET_IN_ADDR imr_multiaddr;
    struct SOCKET_IN_ADDR imr_interface;
} SOCKET_IP_MREQ;


/* getaddrinfo list element */
typedef struct SOCKET_ADDR_INFO
{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    size_t  ai_addrlen;
    struct SOCKET_SOCKADDR *ai_addr;
    char   *ai_canonname;
    struct SOCKET_ADDR_INFO *ai_next;
} SOCKET_ADDR_INFO;


/* Address and protocol Families */
#define SOCKET_FAMILY_UNSPEC      0
#define SOCKET_FAMILY_INET        2
#define SOCKET_FAMILY_INET6       10
#define SOCKET_FAMILY_PACKET      17
#define SOCKET_PROTO_LLTD         0x88d9


/* Socket Types */
#define SOCKET_STREAM     1               /* stream socket */
#define SOCKET_DGRAM      2               /* datagram socket */
#define SOCKET_RAW        3               /* raw-protocol interface */

/* inet addr constants */
#define SOCKET_INADDR_ANY              (uint32_t)0x00000000
#define SOCKET_INADDR_LOOPBACK         0x7f000001
#define SOCKET_INADDR_BROADCAST        (uint32_t)0xffffffff
#define SOCKET_INADDR_NONE             0xffffffff

/* ProtoTypes */
#define SOCKET_IPPROTO_IP           0               /* dummy for IP */
#define SOCKET_IPPROTO_TCP          6               /* tcp */
#define SOCKET_IPPROTO_UDP          17              /* user datagram protocol */

/* TCP Options */
#define SOCKET_TCP_NODELAY          0x1000
#define SOCKET_TCP_BSDURGENT        0x7000

/* UDP Options */
#define SOCKET_MULTICAST_TTL    10
#define SOCKET_MULTICAST_ADD_MEMBERSHIP 12
#define SOCKET_MULTICAST_DROP_MEMBERSHIP 13

#define SOCKET_L_SOCKET             0xffff          /* options for socket level */

/* manifest constants for sockets_shutdown() */
#define SOCKET_D_RECEIVE      0x00
#define SOCKET_D_SEND         0x01
#define SOCKET_D_BOTH         0x02


/* Option flags per-socket. */
#define SOCKET_DEBUG        0x0001          /* turn on debugging info recording */
#define SOCKET_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SOCKET_REUSEADDR    0x0004          /* allow local address reuse */
#define SOCKET_KEEPALIVE    0x0008          /* keep connections alive */
#define SOCKET_DONTROUTE    0x0010          /* just use interface addresses */
#define SOCKET_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SOCKET_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SOCKET_LINGER       0x0080          /* linger on close if data present */
#define SOCKET_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SOCKET_DONTLINGER   (uint32_t)(~SOCKET_LINGER)

#define SOCKET_NI_MAXHOST  1025             /* Max size of a fully-qualified domain name */
#define SOCKET_NI_MAXSERV    32             /* Max size of a service name */

/* Flags for getnameinfo() */
#define SOCKET_NI_NOFQDN       0x01  /* Only return nodename portion for local hosts */
#define SOCKET_NI_NUMERICHOST  0x02  /* Return numeric form of the host's address */
#define SOCKET_NI_NAMEREQD     0x04  /* Error if the host's name not in DNS */
#define SOCKET_NI_NUMERICSERV  0x08  /* Return numeric form of the service (port #) */
#define SOCKET_NI_DGRAM        0x10  /* Service is a datagram service */

/*  Define flags to be used with the WSAAsyncSelect() call. */
#define SOCKET_FD_READ         0x01
#define SOCKET_FD_WRITE        0x02
#define SOCKET_FD_OOB          0x04
#define SOCKET_FD_ACCEPT       0x08
#define SOCKET_FD_CONNECT      0x10
#define SOCKET_FD_CLOSE        0x20

/* Maximum queue length specifiable by listen. */
#define SOCKET_SOMAXCONN       5

#endif /*__platSockets_h__*/
