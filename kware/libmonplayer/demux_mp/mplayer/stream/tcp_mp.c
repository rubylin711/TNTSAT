/*
 * Network layer for MPlayer
 *
 * Copyright (C) 2001 Bertrand BAUDET <bertrand_baudet@yahoo.com>
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "config.h"

#include "mp_msg.h"
#include "help_mp.h"
#ifdef __LINUX__
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include "mt_type.h"
#include "sys_define.h"
#include "mtos_task.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_mem.h"
#include "mtos_msg.h"
#include "mtos_timer.h"
#include "mtos_misc.h"
#include "ethernet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "ethernet_priv.h"
#endif

#ifdef __LINUX__
#define closesocket close
#define OS_PRINTF printf
#endif

#include "tcp.h"
#include "libavutil/avstring.h"
#include "demux_mp_misc.h"
#include "file_playback_sequence.h"

typedef enum {
    NET_TASK_INVALID,
    NET_TASK_RUNNING,
    NET_TASK_READY,
    NET_TASK_SUCCESS,
    NET_TASK_EXIT,
    NET_TASK_DELETE,
    NET_TASK_STATUS_MAX,
} NET_TASK_STATUS;

extern int stream_check_interrupt(int time);
/* IPv6 options */
int network_prefer_ipv4 = 0;
extern int g_is_live_broadcast;
// Converts an address family constant to a string
#if 0
static int net_task_status = NET_TASK_INVALID;
static int net_task_res = 0;
static int net_task_prio = 128;
static void * p_net_task_mem = NULL;
#endif
#ifndef __LINUX__
static char *
i2a(unsigned i, char *a, unsigned r)
{
    if (i / r > 0)
	a = i2a(i / r, a, r);
    *a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % r];
    return a + 1;
}

/**
 Transforms integer i into an ascii string and stores the result in a;
 string is encoded in the base indicated by r.
 @param i Number to be converted
 @param a String result
 @param r Base of value; must be in the range 2 - 36
 @return Returns a
*/
static char *
_itoa(int i, char *a, int r)
{
    r = ((r < 2) || (r > 36)) ? 10 : r;
    if (i < 0) {
	*a = '-';
	*i2a(-i, a + 1, r) = 0;
    } else
	*i2a(i, a, r) = 0;
    return a;
}

static int2ipchar(char *ip_str, char *our_s_addr)
{
    int i = 0;
    for (i = 0; i < 4; i++) {
	u8 ip_int = *(our_s_addr++);

	_itoa(ip_int, ip_str, 10);

	ip_str = ip_str + strlen(ip_str);
	*ip_str = '.';
	ip_str++;
    }
    ip_str--;
    *ip_str = '\0';
    // OS_PRINTF("\n %s %d %s\n",__func__,__LINE__,ip_str_bak);
}
#endif
NETWORK_LISTEN_T network_listen = { 0, 0, 0, 0, 0, 0 };
static const char *af2String(int af)
{
    switch (af) {
    case AF_INET:
	return "AF_INET";
#ifdef HAVE_AF_INET6

    case AF_INET6:
	return "AF_INET6";
#endif

    default:
	return "Unknown address family!";
    }
}
#if NET_TASK_LISTEN
static void network_call_task(void *p_param)
{
    NETWORK_LISTEN_T *network_task_listen = (NETWORK_LISTEN_T *)p_param;
    network_task_listen->task_status = NET_TASK_RUNNING;
    //OS_PRINTF("[%s]  %d...\n ", __func__, __LINE__);
    mt_set_pthread_name(__FUNCTION__);
    switch (network_task_listen->task_type) {
    case NET_TYPE_GETHOSTBYNAME: {
	void *host_name = malloc33(strlen(network_task_listen->p_par) + 1);

	if (host_name == NULL) {
	    network_task_listen->task_res = (int)NULL;
	    break;
	}

	memset(host_name, 0, strlen(network_task_listen->p_par) + 1);
	memcpy(host_name, network_task_listen->p_par, strlen(network_task_listen->p_par));
	network_task_listen->task_status = NET_TASK_READY;
	network_task_listen->task_res = (int)gethostbyname(host_name);
	free33(host_name);
    } break;

    case NET_TYPE_CONNECT: {
	CONNECT_PARAM_T *p_connect = (CONNECT_PARAM_T *)(network_task_listen->p_par);
	struct sockaddr *server_address = malloc33(sizeof(struct sockaddr));
	int fd = p_connect->fd;
	int size = p_connect->size;
	if (server_address == NULL) {
	    network_task_listen->task_res = -1;
	    break;
	}

	memset(server_address, 0, sizeof(struct sockaddr));
	//OS_PRINTF("[%s]  %d...\n ", __func__, __LINE__);
	memcpy(server_address, p_connect->p_buf, sizeof(struct sockaddr));
	network_task_listen->task_status = NET_TASK_READY;
	network_task_listen->task_res = (int)connect(fd, server_address, size);
	free33(server_address);
	break;
    }
    case NET_TYPE_RECV: {
	CONNECT_PARAM_T *p_connect = (CONNECT_PARAM_T *)(network_task_listen->p_par);
	int fd = p_connect->fd;
	int size = p_connect->size;
	void *recv_buf = malloc33(size);
	if (recv_buf == NULL) {
	    network_task_listen->task_res = 0;
	    break;
	}
	network_task_listen->task_status = NET_TASK_READY;
	network_task_listen->task_res = recv(fd, recv_buf, size, 0);
	memcpy(p_connect->p_buf, recv_buf, size);
	free33(recv_buf);
	break;
    }

    default:
	break;
    }

    //  while(1)
    {
	if (network_task_listen->task_status != NET_TASK_EXIT) {
	    network_task_listen->task_status = NET_TASK_SUCCESS;
	}

	//YOUTUBE_DEBUG("[%s]  to be killed\n ", __func__);
    }

    while (network_task_listen->task_status != NET_TASK_EXIT) {
	//OS_PRINTF("[%s]  %d. %d..\n ", __func__, __LINE__,network_task_listen->task_status);
	mtos_task_sleep(50);
    }

    //free33(p_net_task_mem);
    network_task_listen->task_status = NET_TASK_DELETE;
    //OS_PRINTF("[%s]  %d. %d..\n ", __func__, __LINE__,network_task_listen->task_status);
    while (1) {
	//OS_PRINTF("[%s]  %d...\n ", __func__,__LINE__);

	mtos_task_sleep(50);
    }
}
void network_listen_start(void *p_param, void *p_task_par, int task_par_len)

{
    int ret;
    NETWORK_LISTEN_T *network_task_listen = (NETWORK_LISTEN_T *)p_param;

    int p_net_task_mem_size = 128 * 1024;
    network_task_listen->task_prio = p_seq->net_task_prio;
    network_task_listen->p_par = p_task_par;

    if (network_task_listen->task_status == NET_TASK_INVALID) {
	network_task_listen->p_task_mem = malloc33(p_net_task_mem_size);
    } else {
	while (network_task_listen->task_status != NET_TASK_DELETE) {
	    mtos_task_sleep(50);
	    //OS_PRINTF("[%s]  %d...\n ", __func__, __LINE__);
	}

	mtos_task_delete(network_task_listen->task_prio);
    }

    //OS_PRINTF("[%s]!!!!@@@@  %d...%d\n ", __func__, __LINE__, network_task_listen->task_prio);
    ret = mtos_task_create((u8 *)"network_call_task", network_call_task,
                           network_task_listen, network_task_listen->task_prio,
                           (u32 *)(network_task_listen->p_task_mem),
                           p_net_task_mem_size);
}

int network_listen_stop(void *p_param, int size)

{
    int p_res = -1;
    NETWORK_LISTEN_T *network_task_listen = (NETWORK_LISTEN_T *)p_param;
    //OS_PRINTF("[%s]  %d. %d..\n ", __func__, __LINE__,network_task_listen->task_status);
    while (network_task_listen->task_status != NET_TASK_SUCCESS) {
	mtos_task_sleep(50);
	{

	    if ((is_file_seq_exit()) && NET_TASK_READY == network_task_listen->task_status) {
		//OS_PRINTF("\n%s %d \n", __func__, __LINE__);
		network_task_listen->task_status = NET_TASK_EXIT;

		switch (network_task_listen->task_type) {
		case NET_TYPE_GETHOSTBYNAME:
		    p_res = (int)NULL;
		    break;

		case NET_TYPE_CONNECT:
		    p_res = -1;
		    break;
		case NET_TYPE_RECV:
		    p_res = 0;
		    break;
		default:
		    break;
		}

		return p_res;
	    }
	}
	//OS_PRINTF("[%s]  %d. %d..\n ", __func__, __LINE__,network_task_listen->task_status);
    }

    //OS_PRINTF("[%s]  %d. %d..\n ", __func__, __LINE__,network_task_listen->task_status);

    switch (network_task_listen->task_type) {
    case NET_TYPE_GETHOSTBYNAME:
	if (network_task_listen->task_res == (int)NULL) {
	    p_res = (int)NULL;
	} else {
	    //OS_PRINTF("[%s]  %d..%d.\n ", __func__, __LINE__, size);
	    p_res = (int)malloc33(size);
	    //OS_PRINTF("[%s]  %d...\n ", __func__, __LINE__);
	    memcpy((void *)p_res, (void *)network_task_listen->task_res, size);
	    // OS_PRINTF("[%s]  %d...\n ", __func__, __LINE__);
	    //hp=gethostbyname( host );
	}

	break;

    case NET_TYPE_CONNECT:
	p_res = network_task_listen->task_res;
	break;
    case NET_TYPE_RECV:
	p_res = network_task_listen->task_res;
	break;

    default:
	break;
    }
    //OS_PRINTF("[%s]  %d. %d..\n ", __func__, __LINE__,network_task_listen->task_status);
    network_task_listen->task_status = NET_TASK_EXIT;
    return p_res;
}
#endif
// Connect to a server using a TCP connection, with specified address family
// return -2 for fatal error, like unable to resolve name, connection timeout...
// return -1 is unable to connect to a particular port

static int mt_connect2Server_with_af(char *host, int port, int af, int verb)
{
    int socket_server_fd;
    int err;
    socklen_t err_len;
    int ret;
//    unsigned long val;
    int dns_num = 0, dns_idx = 0;

#ifdef __LINUX__

    int count = 0;
    fd_set set;
    struct timeval tv;
#endif
    union
    {
	struct sockaddr_in four;
#ifdef HAVE_AF_INET6
	struct sockaddr_in6 six;
#endif
    } server_address;
    size_t server_address_size;
    void *our_s_addr; // Pointer to sin_addr or sin6_addr
    struct hostent *hp = NULL;
//char buf[255];
#if 0 //HAVE_WINSOCK2_H  peacer del
    unsigned long val;
    int to;
#else
    struct timeval to;
#endif
#if 0 //HAVE_WINSOCK2_H && defined(HAVE_AF_INET6)  peacer del

    // our winsock name resolution code can not handle IPv6
    if (af == AF_INET6) {
        mp_msg(MSGT_NETWORK, MSGL_WARN, "IPv6 not supported for winsock2\n");
        return TCP_ERROR_FATAL;
    }

#endif
    socket_server_fd = socket(af, SOCK_STREAM, 0);

    if (socket_server_fd == -1) {
	OS_PRINTF("[%s][ERROR]Failed to create %s socket:\n", __func__, af2String(af));
	//mp_msg(MSGT_NETWORK,MSGL_ERR,"Failed to create %s socket:\n", af2String(af));
	return TCP_ERROR_FATAL;
    }

//#if   1//defined(SO_RCVTIMEO) && defined(SO_SNDTIMEO)  peacer del
#ifdef __LINUX__
    to.tv_sec = 30;
#else
    {
    }
#endif
    to.tv_usec = 0;
    setsockopt(socket_server_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
    setsockopt(socket_server_fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to));

    //#endif

    switch (af) {
    case AF_INET:
	our_s_addr = &server_address.four.sin_addr;
	break;
#ifdef HAVE_AF_INET6

    case AF_INET6:
	our_s_addr = &server_address.six.sin6_addr;
	break;
#endif

    default:
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_UnknownAF, af);
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

    memset(&server_address, 0, sizeof(server_address));
    /*
    #if  HAVE_INET_PTON
        if (inet_pton(af, host, our_s_addr)!=1)
    #elif HAVE_INET_ATON
        if (inet_aton(host, our_s_addr)!=1)
    #elif HAVE_WINSOCK2_H
        if ( inet_addr(host)==INADDR_NONE )
    #endif
    */ //peacre del
    {
	//    OS_PRINTF("[%s] verb:%d\n", __func__, verb);

	if (verb) {
	    mp_msg(MSGT_NETWORK, MSGL_STATUS, MSGTR_MPDEMUX_NW_ResolvingHostForAF, host, af2String(af));
	}

#ifndef __LINUX__

#endif
//   OS_PRINTF("\n@@@@@@@@@@@########%s %d \n", __func__, __LINE__);
//mtos_task_sleep(5000);
//   OS_PRINTF("\n@@@@@@@@@@@########%s %d \n", __func__, __LINE__);
#ifdef HAVE_GETHOSTBYNAME2
	hp = gethostbyname2(host, af);
#else
#if 0
        network_listen.task_type =  NET_TYPE_GETHOSTBYNAME;
        network_listen_start(&network_listen, host,0);
        hp = (struct hostent *) network_listen_stop(&network_listen, (sizeof(struct hostent)));
#else
	hp = gethostbyname(host);
	dns_num = 0;
	while (hp != NULL && hp->h_addr_list[dns_num] != NULL) {
	    OS_PRINTF("Gavin: %s %d idx:%d, ipaddr:0x%x\n", __func__, __LINE__, dns_num, *((unsigned int *)(hp->h_addr_list[dns_num])));
	    dns_num++;
	}
#endif
#endif

	if (hp == NULL) {
	    if (verb) {
		OS_PRINTF("[%s] [ERROR] fail to gethostbyname!!!!!!!!!\n", __func__);
		//mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MPDEMUX_NW_CantResolv, af2String(af), host);
	    }

	    OS_PRINTF("[%s] [ERROR] fail to gethostbyname!!!!!!!!!\n", __func__);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_FATAL;
	}

	memcpy(our_s_addr, hp->h_addr_list[0], hp->h_length);
#if 0
        free33(hp);
#endif
    }
#if 0 // HAVE_WINSOCK2_H  peacer del
    else {
        unsigned long addr = inet_addr(host);
        memcpy(our_s_addr, &addr, sizeof(addr));
    }

#endif

    switch (af) {
    case AF_INET:
	server_address.four.sin_family = af;
	server_address.four.sin_port = htons(port);
	server_address_size = sizeof(server_address.four);
	break;
#ifdef HAVE_AF_INET6

    case AF_INET6:
	server_address.six.sin6_family = af;
	server_address.six.sin6_port = htons(port);
	server_address_size = sizeof(server_address.six);
	break;
#endif

    default:
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_UnknownAF, af);
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

	/*
    #if HAVE_INET_PTON
        inet_ntop(af, our_s_addr, buf, 255);
    #elif HAVE_INET_ATON || defined(HAVE_WINSOCK2_H)
        av_strlcpy( buf, inet_ntoa( *((struct in_addr*)our_s_addr) ), 255);
    #endif*/ //peacer del
           // if(verb)
           // {
           //mp_msg(MSGT_NETWORK,MSGL_STATUS,MSGTR_MPDEMUX_NW_ConnectingToServer, host, buf , port );
           // }
           // Turn the socket as non blocking so we can timeout on the connection
#if !HAVE_WINSOCK2_H
//yliu remove
//fcntl( socket_server_fd, F_SETFL, fcntl(socket_server_fd, F_GETFL) | O_NONBLOCK );
#else
#if 0
#else
    val = 1;
    ioctlsocket(socket_server_fd, FIONBIO, &val);
#endif
#endif
//OS_PRINTF("[%s] %d\n",__func__,__LINE__);
//   OS_PRINTF("[%s] start connect lalalal!!!!!!!!!!\n", __func__);
#ifndef __LINUX__
    {
    }
#endif
//    OS_PRINTF("\n@@@@@!!!!!!!!@@@@@@########%s %d \n", __func__, __LINE__);
//mtos_task_sleep(5000);
//    OS_PRINTF("\n@@@@@!!!!!!!@@@@@@########%s %d \n", __func__, __LINE__);
#if 0
    {
        CONNECT_PARAM_T  connect_para;
        connect_para.fd = socket_server_fd;
        connect_para.address_size = server_address_size;
        connect_para.server_address = (struct sockaddr *)&server_address;
        network_listen.task_type =  NET_TYPE_CONNECT;
        network_listen_start(&network_listen, &connect_para,0);
        ret = network_listen_stop(&network_listen, 0);
    }
#else
    dns_idx = 0;
    do {

	//OS_PRINTF("Gavin: %s dns_num:%d addr:0x%x fd:%d\n", __func__, dns_num, server_address.four.sin_addr, socket_server_fd);
	ret = connect(socket_server_fd, (struct sockaddr *)&server_address, server_address_size);
	if (ret >= 0) {
	    break;
	}
	closesocket(socket_server_fd);
	dns_idx++;
	if (hp->h_addr_list[dns_idx] != NULL) {
	    OS_PRINTF("XXX: %s dns_idx:%d addr:0x%x\n", __func__, dns_idx, *((unsigned int *)(hp->h_addr_list[dns_idx])));
	    memcpy(our_s_addr, hp->h_addr_list[dns_idx], hp->h_length);
	}
	socket_server_fd = socket(af, SOCK_STREAM, 0);
	if (socket_server_fd == -1) {
	    OS_PRINTF("[%s][ERROR]Failed to create %s socket:\n", __func__, af2String(af));
	    return TCP_ERROR_FATAL;
	}
	setsockopt(socket_server_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
	setsockopt(socket_server_fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to));
	switch (af) {
	case AF_INET:
	    server_address.four.sin_family = af;
	    server_address.four.sin_port = htons(port);
	    server_address_size = sizeof(server_address.four);
	    break;
#ifdef HAVE_AF_INET6

	case AF_INET6:
	    server_address.six.sin6_family = af;
	    server_address.six.sin6_port = htons(port);
	    server_address_size = sizeof(server_address.six);
	    break;
#endif

	default:
	    mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_UnknownAF, af);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_FATAL;
	}
#if !HAVE_WINSOCK2_H
//yliu remove
//fcntl( socket_server_fd, F_SETFL, fcntl(socket_server_fd, F_GETFL) | O_NONBLOCK );
#else
	val = 1;
	ioctlsocket(socket_server_fd, FIONBIO, &val);
#endif
    } while (dns_idx < dns_num);
#endif

    if (ret == -1) {
	OS_PRINTF("[%s] %d\n", __func__, __LINE__);
#ifdef __LINUX__

	//#if !HAVE_WINSOCK2_H
	if (errno != EINPROGRESS) {
	    //#else
	    //      if( (WSAGetLastError() != WSAEINPROGRESS) && (WSAGetLastError() != WSAEWOULDBLOCK) )
	    //         {
	    //#endif
	    if (verb) {
		OS_PRINTF("[%s] can't connect !!!!\n", __func__);
		mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_CantConnect2Server, af2String(af));
	    }

	    OS_PRINTF("[%s][ERROR] can't connect !!!!\n", __func__);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_PORT;
	}

#else
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
#endif
    }

//OS_PRINTF("[%s] success connect!!!!!!!!!!\n", __func__);
// OS_PRINTF("[%s] start select lalala !!!!!!!!!!\n", __func__);
//yliu add: non block not use in lwip connect
#ifdef __LINUX__
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    FD_ZERO(&set);
    FD_SET(socket_server_fd, &set);
#ifndef __LINUX__
    {
    }
#endif

    // When the connection will be made, we will have a writeable fd
    while ((ret = select(socket_server_fd + 1, NULL, &set, NULL, &tv)) == 0) {
#ifndef __LINUX__

#endif

	if (count > 30 || stream_check_interrupt(500)) {
	    if (count > 30) {
		OS_PRINTF("[%s] :%s!!!\n", __func__, MSGTR_MPDEMUX_NW_ConnTimeout);
		//mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MPDEMUX_NW_ConnTimeout);
	    } else {
		OS_PRINTF("[%s] :%s!!!\n", __func__, "Connection interrupted by user\n");
		//mp_msg(MSGT_NETWORK,MSGL_V,"Connection interrupted by user\n");
	    }
	    closesocket(socket_server_fd);
	    return TCP_ERROR_TIMEOUT;
	}

	count++;
	FD_ZERO(&set);
	FD_SET(socket_server_fd, &set);
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
    }

    if (ret < 0) {
	OS_PRINTF("[%s][ERROR] :%s!!!\n", __func__, MSGTR_MPDEMUX_NW_SelectFailed);
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_SelectFailed);
    }

#endif
// Turn back the socket as blocking
#ifdef __LINUX__ //gjq for compile error
    //yliu remove
    //peacer add,linux need it
    fcntl(socket_server_fd, F_SETFL, fcntl(socket_server_fd, F_GETFL) & ~O_NONBLOCK);
#else
    val = 0;
    ioctlsocket(socket_server_fd, FIONBIO, &val);
#endif

#ifndef __LINUX__
    {
    }
#endif
    // Check if there were any errors
    err_len = sizeof(int);
    ret = getsockopt(socket_server_fd, SOL_SOCKET, SO_ERROR, &err, &err_len);

    if (ret < 0) {
	OS_PRINTF("[%s][ERROR] :%s!!!\n", __func__, "getsockopt failed:");
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_GetSockOptFailed, strerror(errno));
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

    if (err > 0) {
	OS_PRINTF("[%s][ERROR] :%s!!!\n", __func__, "connect error:");
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ConnectError, strerror(err));
	closesocket(socket_server_fd);
	return TCP_ERROR_PORT;
    }

    return socket_server_fd;
}
static int connect2Server_with_af(char *host, int port, int af, int verb)
{
    int socket_server_fd;
    int err;
    socklen_t err_len;
    int ret;
#ifdef __LINUX__
    int count = 0;
    fd_set set;
    struct timeval tv;
#endif
    union
    {
	struct sockaddr_in four;
#ifdef HAVE_AF_INET6
	struct sockaddr_in6 six;
#endif
    } server_address;
    size_t server_address_size;
    void *our_s_addr; // Pointer to sin_addr or sin6_addr
    struct hostent *hp = NULL;
//char buf[255];
#if 0 //HAVE_WINSOCK2_H  peacer del
    unsigned long val;
    int to;
#else
//    unsigned long val;
    struct timeval to;
#endif
#if 0 //HAVE_WINSOCK2_H && defined(HAVE_AF_INET6)  peacer del

    // our winsock name resolution code can not handle IPv6
    if (af == AF_INET6) {
        mp_msg(MSGT_NETWORK, MSGL_WARN, "IPv6 not supported for winsock2\n");
        return TCP_ERROR_FATAL;
    }

#endif
    socket_server_fd = socket(af, SOCK_STREAM, 0);
    OS_PRINTF("\n@@@@%s %d fd:%d\n", __func__, __LINE__, socket_server_fd);
    if (socket_server_fd == -1) {
	OS_PRINTF("[%s][ERROR]Failed to create %s socket:\n", __func__, af2String(af));
	//mp_msg(MSGT_NETWORK,MSGL_ERR,"Failed to create %s socket:\n", af2String(af));
	return TCP_ERROR_FATAL;
    }

//#if   1//defined(SO_RCVTIMEO) && defined(SO_SNDTIMEO)  peacer del
#ifdef __LINUX__
    to.tv_sec = 30;
#else
    {

	if (is_file_seq_exit()) {
	    OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	    to.tv_sec = g_is_live_broadcast ? 10000 : 10000;
	    //return TCP_ERROR_TIMEOUT;
	} else {
	    to.tv_sec = 1000;
	}
    }
#endif
    to.tv_usec = 0;
    setsockopt(socket_server_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
    setsockopt(socket_server_fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to));

    //#endif

    switch (af) {
    case AF_INET:
	our_s_addr = &server_address.four.sin_addr;
	break;
#ifdef HAVE_AF_INET6

    case AF_INET6:
	our_s_addr = &server_address.six.sin6_addr;
	break;
#endif

    default:
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_UnknownAF, af);
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

    memset(&server_address, 0, sizeof(server_address));
    /*
    #if  HAVE_INET_PTON
        if (inet_pton(af, host, our_s_addr)!=1)
    #elif HAVE_INET_ATON
        if (inet_aton(host, our_s_addr)!=1)
    #elif HAVE_WINSOCK2_H
        if ( inet_addr(host)==INADDR_NONE )
    #endif
    */ //peacre del
    {
	OS_PRINTF("[%s] verb:%d\n", __func__, verb);

	if (verb) {
	    mp_msg(MSGT_NETWORK, MSGL_STATUS, MSGTR_MPDEMUX_NW_ResolvingHostForAF, host, af2String(af));
	}

#ifndef __LINUX__

	if (is_file_seq_exit()) {
	    OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_FATAL;
	}

#endif
	OS_PRINTF("\n@@@@@@@@@@@########%s %d \n", __func__, __LINE__);
	//mtos_task_sleep(5000);
	OS_PRINTF("\n@@@@@@@@@@@########%s %d \n", __func__, __LINE__);
#ifdef HAVE_GETHOSTBYNAME2
	hp = gethostbyname2(host, af);
#else
#if NET_TASK_LISTEN
	network_listen.task_type = NET_TYPE_GETHOSTBYNAME;
	network_listen_start(&network_listen, host, 0);
	hp = (struct hostent *)network_listen_stop(&network_listen, (sizeof(struct hostent)));
#else
	hp = gethostbyname(host);
#endif
#endif

	if (hp == NULL) {
	    if (verb) {
		OS_PRINTF("[%s] [ERROR] fail to gethostbyname!!!!!!!!!\n", __func__);
		//mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MPDEMUX_NW_CantResolv, af2String(af), host);
	    }

	    OS_PRINTF("[%s] [ERROR] fail to gethostbyname!!!!!!!!!\n", __func__);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_FATAL;
	}

	memcpy(our_s_addr, hp->h_addr_list[0], hp->h_length);
#ifndef __LINUX__
	{

	    char server_ip[32] = { 0 };
	    char local_ip[32] = { 0 };
	    char server_country[8] = { 0 };
	    char local_country[8] = { 0 };
	    u8 chip_type;

	    chip_type = net_init();

	    if (chip_type == 0) {

		get_local_ip(local_ip);

		get_ip_country(local_ip, local_country);

		if (strstr(local_country, "CN")) {
		    int2ipchar(server_ip, (char *)our_s_addr);
		    get_ip_country(server_ip, server_country);
		    if (strstr(server_country, "CN")) {
			OS_PRINTF("\n %s %d %s %s\n", __func__, __LINE__, server_ip, server_country);
			OS_PRINTF("\n %s %d %s %s\n", __func__, __LINE__, local_ip, local_country);
			OS_PRINTF("\n %s %d %d\n", __func__, __LINE__, chip_type);
			OS_PRINTF("[%s] %d region restrict!!\n", __func__, __LINE__);
			closesocket(socket_server_fd);
			return TCP_ERROR_FATAL;
		    }
		}
	    }
	    //get_local_ip();
	    //free33(ip_str_bak);
	}
#endif
#if NET_TASK_LISTEN
	free33(hp);
#endif
    }
#if 0 // HAVE_WINSOCK2_H  peacer del
    else {
        unsigned long addr = inet_addr(host);
        memcpy(our_s_addr, &addr, sizeof(addr));
    }

#endif

    switch (af) {
    case AF_INET:
	server_address.four.sin_family = af;
	server_address.four.sin_port = htons(port);
	server_address_size = sizeof(server_address.four);
	break;
#ifdef HAVE_AF_INET6

    case AF_INET6:
	server_address.six.sin6_family = af;
	server_address.six.sin6_port = htons(port);
	server_address_size = sizeof(server_address.six);
	break;
#endif

    default:
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_UnknownAF, af);
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

	/*
    #if HAVE_INET_PTON
        inet_ntop(af, our_s_addr, buf, 255);
    #elif HAVE_INET_ATON || defined(HAVE_WINSOCK2_H)
        av_strlcpy( buf, inet_ntoa( *((struct in_addr*)our_s_addr) ), 255);
    #endif*/ //peacer del
           // if(verb)
           // {
           //mp_msg(MSGT_NETWORK,MSGL_STATUS,MSGTR_MPDEMUX_NW_ConnectingToServer, host, buf , port );
           // }
           // Turn the socket as non blocking so we can timeout on the connection
#if !HAVE_WINSOCK2_H
//yliu remove
//fcntl( socket_server_fd, F_SETFL, fcntl(socket_server_fd, F_GETFL) | O_NONBLOCK );
#else

#ifndef __LINUX__
    val = 0;
    ioctlsocket(socket_server_fd, FIONBIO, &val);
#endif

#endif
    //OS_PRINTF("[%s] %d\n",__func__,__LINE__);
    OS_PRINTF("[%s] start connect lalalal!!!!!!!!!!\n", __func__);
#ifndef __LINUX__
    {

	if (is_file_seq_exit()) {
	    OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_FATAL;
	}
    }
#endif
//OS_PRINTF("\n@@@@@!!!!!!!!@@@@@@########%s %d \n", __func__, __LINE__);
//mtos_task_sleep(5000);
//OS_PRINTF("\n@@@@@!!!!!!!@@@@@@########%s %d \n", __func__, __LINE__);

#if NET_TASK_LISTEN
    {
	CONNECT_PARAM_T connect_para;
	connect_para.fd = socket_server_fd;
	connect_para.size = server_address_size;
	connect_para.p_buf = (struct sockaddr *)&server_address;
	network_listen.task_type = NET_TYPE_CONNECT;
	network_listen_start(&network_listen, &connect_para, 0);
	ret = network_listen_stop(&network_listen, 0);
    }
#else
    ret = connect(socket_server_fd, (struct sockaddr *)&server_address, server_address_size);
#endif

    if (ret == -1) {
	OS_PRINTF("[%s] %d\n", __func__, __LINE__);
#ifdef __LINUX__

	//#if !HAVE_WINSOCK2_H
	if (errno != EINPROGRESS) {
	    //#else
	    //      if( (WSAGetLastError() != WSAEINPROGRESS) && (WSAGetLastError() != WSAEWOULDBLOCK) )
	    //         {
	    //#endif
	    if (verb) {
		OS_PRINTF("[%s] can't connect !!!!\n", __func__);
		mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_CantConnect2Server, af2String(af));
	    }

	    OS_PRINTF("[%s][ERROR] can't connect !!!!\n", __func__);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_PORT;
	}

#else
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
#endif
    }

    OS_PRINTF("[%s] success connect!!!!!!!!!!\n", __func__);
//OS_PRINTF("[%s] start select lalala !!!!!!!!!!\n", __func__);
//yliu add: non block not use in lwip connect
#ifdef __LINUX__
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    FD_ZERO(&set);
    FD_SET(socket_server_fd, &set);

    if (is_file_seq_exit()) {
	OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

    // When the connection will be made, we will have a writeable fd
    while ((ret = select(socket_server_fd + 1, NULL, &set, NULL, &tv)) == 0) {

	if (is_file_seq_exit()) {
	    OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	    closesocket(socket_server_fd);
	    return TCP_ERROR_TIMEOUT;
	}

	if (count > 30 || stream_check_interrupt(500)) {
	    if (count > 30) {
		OS_PRINTF("[%s] :%s!!!\n", __func__, MSGTR_MPDEMUX_NW_ConnTimeout);
		//mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MPDEMUX_NW_ConnTimeout);
	    } else {
		OS_PRINTF("[%s] :%s!!!\n", __func__, "Connection interrupted by user\n");
		//mp_msg(MSGT_NETWORK,MSGL_V,"Connection interrupted by user\n");
	    }
	    closesocket(socket_server_fd);
	    return TCP_ERROR_TIMEOUT;
	}

	count++;
	FD_ZERO(&set);
	FD_SET(socket_server_fd, &set);
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
    }

    if (ret < 0) {
	OS_PRINTF("[%s][ERROR] :%s!!!\n", __func__, MSGTR_MPDEMUX_NW_SelectFailed);
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_SelectFailed);
    }

#endif
// Turn back the socket as blocking
#ifdef __LINUX__
    //yliu remove
    //peacer add,linux need it
    fcntl(socket_server_fd, F_SETFL, fcntl(socket_server_fd, F_GETFL) & ~O_NONBLOCK);
#else
    val = 0;
    ioctlsocket(socket_server_fd, FIONBIO, &val);
#endif

    if (is_file_seq_exit()) {
	OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

    // Check if there were any errors
    err_len = sizeof(int);
    ret = getsockopt(socket_server_fd, SOL_SOCKET, SO_ERROR, &err, &err_len);

    if (ret < 0) {
	OS_PRINTF("[%s][ERROR] :%s!!!\n", __func__, "getsockopt failed:");
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_GetSockOptFailed, strerror(errno));
	closesocket(socket_server_fd);
	return TCP_ERROR_FATAL;
    }

    if (err > 0) {
	OS_PRINTF("[%s][ERROR] :%s!!!\n", __func__, "connect error:");
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ConnectError, strerror(err));
	closesocket(socket_server_fd);
	return TCP_ERROR_PORT;
    }
//OS_PRINTF("\n%s %d \n", __func__, __LINE__);
#ifndef __LINUX__
    val = 0;
    ioctlsocket(socket_server_fd, FIONBIO, &val);
#endif
    OS_PRINTF("\n%s %d \n", __func__, __LINE__);
    return socket_server_fd;
}

// Connect to a server using a TCP connection
// return -2 for fatal error, like unable to resolve name, connection timeout...
// return -1 is unable to connect to a particular port

int
mt_connect2Server(char *host, int port, int verb)
{
#ifdef HAVE_AF_INET6
    int r;
    int s = TCP_ERROR_FATAL;
    r = mt_connect2Server_with_af(host, port, network_prefer_ipv4 ? AF_INET : AF_INET6, verb);

    if (r >= 0) {
	return r;
    }

    s = mt_connect2Server_with_af(host, port, network_prefer_ipv4 ? AF_INET6 : AF_INET, verb);

    if (s == TCP_ERROR_FATAL) {
	return r;
    }

    return s;
#else
    return mt_connect2Server_with_af(host, port, AF_INET, verb);
#endif
}

int
connect2Server(char *host, int port, int verb)
{
#ifdef HAVE_AF_INET6
    int r;
    int s = TCP_ERROR_FATAL;
    r = connect2Server_with_af(host, port, network_prefer_ipv4 ? AF_INET : AF_INET6, verb);

    if (r >= 0) {
	return r;
    }

    s = connect2Server_with_af(host, port, network_prefer_ipv4 ? AF_INET6 : AF_INET, verb);

    if (s == TCP_ERROR_FATAL) {
	return r;
    }

    return s;
#else
    return connect2Server_with_af(host, port, AF_INET, verb);
#endif
}
