/*
 * network helpers for TCP connections
 * (originally borrowed from network.c,
 * by Bertrand BAUDET <bertrand_baudet@yahoo.com>)
 *
 * Copyright (C) 2001 Bertrand BAUDET, 2006 Benjamin Zores
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

#ifndef MPLAYER_TCP_H
#define MPLAYER_TCP_H

extern int network_prefer_ipv4;
typedef enum {
    NET_TYPE_GETHOSTBYNAME,
    NET_TYPE_CONNECT,
    NET_TYPE_RECV,
    NET_TYPE_MAX,
} NET_TASK_TYPE;

typedef struct {
    int task_status;
    void * p_task_mem;
    int task_prio;
    int task_res;
    void * p_par;
    int p_par_len;
    int task_type;
} NETWORK_LISTEN_T;
typedef struct {
    int fd;
    void * p_buf;
    int size;
} CONNECT_PARAM_T;
/* Connect to a server using a TCP connection */
int connect2Server (char *host, int port, int verb);
int mt_connect2Server(char *host, int port, int verb);

#define TCP_ERROR_TIMEOUT -3     /* connection timeout */
#define TCP_ERROR_FATAL   -2     /* unable to resolve name */
#define TCP_ERROR_PORT    -1     /* unable to connect to a particular port */

#endif /* MPLAYER_TCP_H */
