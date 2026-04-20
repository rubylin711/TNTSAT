/*
 * Network layer for MPlayer
 *
 * Copyright (C) 2001 Bertrand Baudet <bertrand_baudet@yahoo.com>
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

#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#else
#include "mp_func_trans.h"
#endif
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <ctype.h>

#include "config.h"

#include "mp_msg.h"
#include "help_mp.h"

#if HAVE_WINSOCK2_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "stream.h"
#include "libmpdemux/demuxer.h"
#include "m_config.h"
#include "mpcommon.h"
#include "network.h"
#ifndef __ANDRIOD__
#include "mt_type.h"
#include "sys_define.h"
#else
#include "mt_type.h"
#endif
#ifndef __LINUX__
#include "ethernet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "ethernet_priv.h"
#endif
#include "tcp.h"
#include "http.h"
#include "cookies.h"
#include "url.h"
#include "file_playback_sequence.h"
#include "mtos_printk.h"

/* Variables for the command line option -user, -passwd, -bandwidth,
   -user-agent and -nocookies */
#define HTTP_CHUNK_BUF_SIZE (32 * 1024)

char *network_username = NULL;
char *network_password = NULL;
int network_bandwidth = 0;
int network_cookies_enabled = 0;
char *network_useragent = NULL;
char *network_referrer = NULL;
char **network_http_header_fields = NULL;

/* IPv6 options */
int network_ipv4_only_proxy = 0;

const mime_struct_t mime_type_table[] = {
#ifdef CONFIG_FFMPEG_MP
    // Flash Video
    { "video/x-flv", DEMUXER_TYPE_LAVF_PREFERRED },
    // do not force any demuxer in this case!
    // we want the lavf demuxer to be tried first (happens automatically anyway),
    // but for mov reference files to work we must also try
    // the native demuxer if lavf fails.
    { "video/quicktime", 0 },
#endif
    // MP3 streaming, some MP3 streaming server answer with audio/mpeg
    { "audio/mpeg", DEMUXER_TYPE_AUDIO },
    // MPEG streaming
    { "video/mpeg", DEMUXER_TYPE_UNKNOWN },
    { "video/x-mpeg", DEMUXER_TYPE_UNKNOWN },
    { "video/x-mpeg2", DEMUXER_TYPE_UNKNOWN },
    // AVI ??? => video/x-msvideo
    { "video/x-msvideo", DEMUXER_TYPE_AVI },
    // MOV => video/quicktime
    { "video/quicktime", DEMUXER_TYPE_MOV },
    // ASF
    { "audio/x-ms-wax", DEMUXER_TYPE_ASF },
    { "audio/x-ms-wma", DEMUXER_TYPE_ASF },
    { "video/x-ms-asf", DEMUXER_TYPE_ASF },
    { "video/x-ms-afs", DEMUXER_TYPE_ASF },
    { "video/x-ms-wmv", DEMUXER_TYPE_ASF },
    { "video/x-ms-wma", DEMUXER_TYPE_ASF },
    { "application/x-mms-framed", DEMUXER_TYPE_ASF },
    { "application/vnd.ms.wms-hdr.asfv1", DEMUXER_TYPE_ASF },
    { "application/octet-stream", DEMUXER_TYPE_UNKNOWN },
    // Playlists
    { "video/x-ms-wmx", DEMUXER_TYPE_PLAYLIST },
    { "video/x-ms-wvx", DEMUXER_TYPE_PLAYLIST },
    { "audio/x-scpls", DEMUXER_TYPE_PLAYLIST },
    { "audio/x-mpegurl", DEMUXER_TYPE_PLAYLIST },
    { "audio/x-pls", DEMUXER_TYPE_PLAYLIST },
    // Real Media
    //  { "audio/x-pn-realaudio", DEMUXER_TYPE_REAL },
    // OGG Streaming
    { "application/ogg", DEMUXER_TYPE_OGG },
    { "application/x-ogg", DEMUXER_TYPE_OGG },
    // NullSoft Streaming Video
    { "video/nsv", DEMUXER_TYPE_NSV },
    { "misc/ultravox", DEMUXER_TYPE_NSV },
    { NULL, DEMUXER_TYPE_UNKNOWN },
};
#if 0       // compile warning, ignore it
/*
*
*
*
*/
static int net_select(int fd, fd_set *sel_fd)
{
    struct timeval timeoutVal;
    FD_ZERO(sel_fd);
    FD_SET(fd, sel_fd);
    timeoutVal.tv_sec = 1;
    timeoutVal.tv_usec = 0;
//OS_PRINTF("\n %s %d\n", __func__, __LINE__);
#ifndef __LINUX__
    return select(fd + 1, sel_fd, NULL, NULL, &timeoutVal);
#else
    return 1;
#endif
}
#endif
/*
*
*
*
*/
streaming_ctrl_t *streaming_ctrl_new(void)
{
    streaming_ctrl_t *streaming_ctrl = calloc33(1, sizeof(*streaming_ctrl));

    if (streaming_ctrl == NULL) {
	mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
	return NULL;
    }
    streaming_ctrl->hc_buf_size = HTTP_CHUNK_BUF_SIZE;
    streaming_ctrl->hc_buf = malloc33(streaming_ctrl->hc_buf_size);
    if (streaming_ctrl->hc_buf == NULL) {
	free33(streaming_ctrl);
	return NULL;
    }
    streaming_ctrl->hc_buf_ptr = streaming_ctrl->hc_buf;
    streaming_ctrl->hc_buf_end = streaming_ctrl->hc_buf;

    return streaming_ctrl;
}

/*
*
*
*
*/
void streaming_ctrl_free33(streaming_ctrl_t *streaming_ctrl)
{
    if (streaming_ctrl == NULL)
	return;

    if (streaming_ctrl->url)
	url_free33(streaming_ctrl->url);
    if (streaming_ctrl->hc_buf) {
	free33(streaming_ctrl->hc_buf);
	streaming_ctrl->hc_buf = NULL;
	streaming_ctrl->hc_buf_ptr = NULL;
	streaming_ctrl->hc_buf_end = NULL;
    }

    free33(streaming_ctrl->buffer);
    free33(streaming_ctrl->data);
    free33(streaming_ctrl);
}

/*
*
*
*
*/
URL_t *mthttp_check4proxies(URL_t *url)
{
    URL_t *url_out = NULL;

    if (url == NULL)
	return NULL;

    url_out = mthttp_url_new(url->url);

    if (!strcasecmp(url->protocol, "http_proxy")) {
	mp_msg(MSGT_NETWORK, MSGL_V, "Using HTTP proxy: http://%s:%d\n", url->hostname, url->port);
	return url_out;
    }

    // Check if the http_proxy environment variable is set.
    if (!strcasecmp(url->protocol, "http")) {
	char *proxy;
	proxy = getenv("http_proxy");

	if (proxy != NULL) {
	    // We got a proxy, build the URL to use it
	    char *new_url;
	    URL_t *tmp_url;
	    URL_t *proxy_url = mthttp_url_new(proxy);

	    if (proxy_url == NULL) {
		mp_msg(MSGT_NETWORK, MSGL_WARN,
		       MSGTR_MPDEMUX_NW_InvalidProxySettingTryingWithout);
		return url_out;
	    }

#ifdef HAVE_AF_INET6

	    if (network_ipv4_only_proxy && (gethostbyname(url->hostname) == NULL)) {
		mp_msg(MSGT_NETWORK, MSGL_WARN,
		       MSGTR_MPDEMUX_NW_CantResolvTryingWithoutProxy);
		mthttp_url_free33(proxy_url);
		return url_out;
	    }

#endif
	    mp_msg(MSGT_NETWORK, MSGL_V, "Using HTTP proxy: %s\n", proxy_url->url);
	    new_url = get_http_proxy_url(proxy_url, url->url);

	    if (new_url == NULL) {
		mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
		mthttp_url_free33(proxy_url);
		return url_out;
	    }

	    tmp_url = mthttp_url_new(new_url);

	    if (tmp_url == NULL) {
#ifndef __LINUX__
		mthttp_free33(new_url);
#else
		free33(new_url);
#endif
		mthttp_url_free33(proxy_url);
		return url_out;
	    }

	    mthttp_url_free33(url_out);
	    url_out = tmp_url;
#ifndef __LINUX__
	    mthttp_free33(new_url);
#else
	    free33(new_url);
#endif
	    mthttp_url_free33(proxy_url);
	}
    }

    return url_out;
}

/*
*
*
*
*/
URL_t *check4proxies(URL_t *url)
{
    URL_t *url_out = NULL;

    if (url == NULL)
	return NULL;

    url_out = url_new(url->url);

    if (!strcasecmp(url->protocol, "http_proxy")) {
	mp_msg(MSGT_NETWORK, MSGL_V, "Using HTTP proxy: http://%s:%d\n", url->hostname, url->port);
	return url_out;
    }

    // Check if the http_proxy environment variable is set.
    if (!strcasecmp(url->protocol, "http")) {
	char *proxy;
	proxy = getenv("http_proxy");

	if (proxy != NULL) {
	    // We got a proxy, build the URL to use it
	    char *new_url;
	    URL_t *tmp_url;
	    URL_t *proxy_url = url_new(proxy);

	    if (proxy_url == NULL) {
		mp_msg(MSGT_NETWORK, MSGL_WARN,
		       MSGTR_MPDEMUX_NW_InvalidProxySettingTryingWithout);
		return url_out;
	    }

#ifdef HAVE_AF_INET6

	    if (network_ipv4_only_proxy && (gethostbyname(url->hostname) == NULL)) {
		mp_msg(MSGT_NETWORK, MSGL_WARN,
		       MSGTR_MPDEMUX_NW_CantResolvTryingWithoutProxy);
		url_free33(proxy_url);
		return url_out;
	    }

#endif
	    mp_msg(MSGT_NETWORK, MSGL_V, "Using HTTP proxy: %s\n", proxy_url->url);
	    new_url = get_http_proxy_url(proxy_url, url->url);

	    if (new_url == NULL) {
		mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
		url_free33(proxy_url);
		return url_out;
	    }

	    tmp_url = url_new(new_url);

	    if (tmp_url == NULL) {
		free33(new_url);
		url_free33(proxy_url);
		return url_out;
	    }

	    url_free33(url_out);
	    url_out = tmp_url;
	    free33(new_url);
	    url_free33(proxy_url);
	}
    }

    return url_out;
}

URL_t *url_new_with_proxy(const char *urlstr)
{
    URL_t *url = url_new(urlstr);
    URL_t *url_with_proxy = check4proxies(url);
    url_free33(url);
    return url_with_proxy;
}

/*
*
*
*
*/
int http_send_request(URL_t *url, off_t pos)
{
    HTTP_header_t *http_hdr;
    URL_t *server_url;
    char str[256];
    int fd = -1;
    int ret;
    int proxy = 0; // Boolean
    http_hdr = http_new_header(); if(http_hdr == NULL) return -1;

    if (!strcasecmp(url->protocol, "http_proxy")) {
	proxy = 1;
	server_url = url_new((url->file) + 1);

	if (!server_url) {
	    mp_msg(MSGT_NETWORK, MSGL_ERR, "Invalid URL '%s' to proxify\n", url->file + 1);
	    goto err_out;
	}

	http_set_uri(http_hdr, server_url->noauth_url);

    } else {
	server_url = url;
	http_set_uri(http_hdr, server_url->file);
    }

    if (server_url->port && server_url->port != 80)
	snprintf(str, sizeof(str), "Host: %s:%d", server_url->hostname, server_url->port);

    else
	snprintf(str, sizeof(str), "Host: %s", server_url->hostname);

    http_set_field(http_hdr, str);

    if (network_useragent)
	snprintf(str, sizeof(str), "User-Agent: %s", network_useragent);

    else {
	snprintf(str, sizeof(str), "User-Agent: PPlayer");
	// snprintf(str, sizeof(str), "User-Agent: %s", mplayer_version);
    }

    http_set_field(http_hdr, str);

    if (network_referrer) {
	char *referrer = NULL;
	size_t len = strlen(network_referrer) + 10;

	// Check len to ensure we don't do something really bad in case of an overflow
	if (len > 10)
	    referrer = malloc33(len);

	if (referrer == NULL) {
	    mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);

	} else {
	    snprintf(referrer, len, "Referer: %s", network_referrer);
	    http_set_field(http_hdr, referrer);
	    free33(referrer);
	}
    }

    if (strcasecmp(url->protocol, "noicyx"))
	http_set_field(http_hdr, "Icy-MetaData: 1");

    if (pos > 0) {
	// Extend http_send_request with possibility to do partial content retrieval
	snprintf(str, sizeof(str), "Range: bytes=%" PRId64 "-", (int64_t)pos);
	http_set_field(http_hdr, str);
    }

#ifdef __LINUX__

    if (network_cookies_enabled)
	cookies_set(http_hdr, server_url->hostname, server_url->url);

#endif

    if (network_http_header_fields) {
	int i = 0;

	while (network_http_header_fields[i])
	    http_set_field(http_hdr, network_http_header_fields[i++]);
    }

    http_set_field(http_hdr, "Connection: close");

    if (proxy)
	http_add_basic_proxy_authentication(http_hdr, url->username, url->password);

    http_add_basic_authentication(http_hdr, server_url->username, server_url->password);

    if (http_build_request(http_hdr) == NULL) {
	goto err_out;
    }

    if (proxy) {

	if (url->port == 0)
	    url->port = 8080; // Default port for the proxy server

	fd = connect2Server(url->hostname, url->port, 1);
	url_free33(server_url);
	server_url = NULL;

    } else {

	if (server_url->port == 0)
	    server_url->port = 80; // Default port for the web server

	fd = connect2Server(server_url->hostname, server_url->port, 1);
    }

    if (fd < 0) {
	goto err_out;
    }

    mp_msg(MSGT_NETWORK, MSGL_DBG2, "Request: [%s]\n", http_hdr->buffer);
    OS_PRINTF("\n %s fd[%d] %d\n", __func__, fd, __LINE__);
    ret = send(fd, http_hdr->buffer, http_hdr->buffer_size, DEFAULT_SEND_FLAGS);
    OS_PRINTF("\n %s %d\n", __func__, __LINE__);

    if (ret != (int)http_hdr->buffer_size) {
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ErrSendingHTTPRequest);
	goto err_out;
    }

    http_free33(http_hdr);
    return fd;

err_out:

//yliu add :for  fd = 0
#ifdef __LINUX__
    if (fd > 0) {
	closesocket(fd);
    }
#else
    if (fd >= 0) {
	closesocket(fd);
    }
#endif

    fd = -1;
    http_free33(http_hdr);

    if (proxy && server_url) {
	url_free33(server_url);
    }

    return -1;
}
/*
*
*
*
*/
#define MTHTTP_BUFFER_SIZE (1024 * 64)
char glb_response[MTHTTP_BUFFER_SIZE];
HTTP_header_t *mthttp_http_read_response(int fd)
{
    HTTP_header_t *http_hdr;
    int i;
    int ret;
    int err = 0;
    socklen_t optlen;
    http_hdr = mthttp_http_new_header();

    if (http_hdr == NULL) {
	return NULL;
    }

    do {
	i = recv(fd, glb_response, MTHTTP_BUFFER_SIZE, 0);

	if (i < 0) {
	    mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ReadFailed);
	    optlen = sizeof(err);
	    getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &optlen);
	    if (err == 108)
		break;
	    mthttp_http_free33(http_hdr);
	    return NULL;
	}

	if (i == 0) {
	    mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_Read0CouldBeEOF);
	    break;
	}

	ret = mthttp_http_response_append(http_hdr, glb_response, i);
	if (ret < 0) {
	    OS_PRINTF("%s: allocate buffer for http response failed.\n", __func__);
	    break;
	}
    } while (i > 0);

    if (mthttp_http_response_parse(http_hdr) < 0) {
	mthttp_http_free33(http_hdr);
	return NULL;
    }

    return http_hdr;
}
/*
*
*
*
*/
HTTP_header_t *http_read_response(int fd)
{

    HTTP_header_t *http_hdr = NULL;
    char response[BUFFER_SIZE];
    int i= -1;
    http_hdr = http_new_header();

    if (http_hdr == NULL) {
	return NULL;
    }

    memset(response, 0, BUFFER_SIZE);

    do {

	if (is_file_seq_exit()) {
	    //yliu add:for exit
	    http_free33(http_hdr);

	    OS_PRINTF("\n%s %d stream check :%d\n", __func__, __LINE__, i);
	    return NULL;
	}

	i = recv(fd, response, BUFFER_SIZE, 0);
	if (i < 0) {
	    mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ReadFailed);
	    http_free33(http_hdr);
	    return NULL;
	}

	if (i == 0) {
	    mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_Read0CouldBeEOF);
	    http_free33(http_hdr);
	    return NULL;
	}

	http_response_append(http_hdr, response, i);

    } while (!http_is_header_entire(http_hdr));

    if (http_response_parse(http_hdr) < 0) {
	http_free33(http_hdr);
	return NULL;
    }

    return http_hdr;
}
/*
*
*
*
*/
HTTP_header_t *http_read_response_with_content(int fd)
{
    HTTP_header_t *http_hdr;
    char response[BUFFER_SIZE];
    int i;
    http_hdr = http_new_header();

    if (http_hdr == NULL) {
	return NULL;
    }

    do {
	i = recv(fd, response, BUFFER_SIZE, 0);

	if (i < 0) {
	    mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ReadFailed);
	    break;
	}

	if (i == 0) {
	    mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_Read0CouldBeEOF);
	    break;
	}

	http_response_append(http_hdr, response, i);
    } while (1);

    if ((http_hdr->buffer_size == 0) || http_response_parse(http_hdr) < 0) {
	http_free33(http_hdr);
	return NULL;
    }

    return http_hdr;
}

/*
*
*
*
*/
int
mthttp_http_authenticate(HTTP_header_t *http_hdr, URL_t *url, int *auth_retry)
{
    char *aut;

    if (*auth_retry == 1) {
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_AuthFailed);
	return -1;
    }

    if (*auth_retry > 0) {
#ifndef __LINUX__
	mthttp_free33(url->username);
	url->username = NULL;
	mthttp_free33(url->password);
#else
	free33(url->username);
	url->username = NULL;
	free33(url->password);
#endif
	url->password = NULL;
    }

    aut = mthttp_http_get_field(http_hdr, "WWW-Authenticate");

    if (aut != NULL) {
	char *aut_space;
	aut_space = strstr(aut, "realm=");

	if (aut_space != NULL)
	    aut_space += 6;

	mp_msg(MSGT_NETWORK, MSGL_INFO, MSGTR_MPDEMUX_NW_AuthRequiredFor, aut_space);

    } else {
	mp_msg(MSGT_NETWORK, MSGL_INFO, MSGTR_MPDEMUX_NW_AuthRequired);
    }

    if (network_username) {
#ifndef __LINUX__
	url->username = mtstrdup33(network_username);
#else
	url->username = strdup33(network_username);
#endif

	if (url->username == NULL) {
	    mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
	    return -1;
	}

    } else {
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_AuthFailed);
	return -1;
    }

    if (network_password) {
#ifndef __LINUX__
	url->password = mtstrdup33(network_password);
#else
	url->password = strdup33(network_password);
#endif

	if (url->password == NULL) {
	    mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
	    return -1;
	}

    } else {
	mp_msg(MSGT_NETWORK, MSGL_INFO, MSGTR_MPDEMUX_NW_NoPasswdProvidedTryingBlank);
    }

    (*auth_retry)++;
    return 0;
}
/*
*
*
*
*/
int http_authenticate(HTTP_header_t *http_hdr, URL_t *url, int *auth_retry)
{
    char *aut;

    if (*auth_retry == 1) {
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_AuthFailed);
	return -1;
    }

    if (*auth_retry > 0) {
	free33(url->username);
	url->username = NULL;
	free33(url->password);
	url->password = NULL;
    }

    aut = http_get_field(http_hdr, "WWW-Authenticate");

    if (aut != NULL) {
	char *aut_space;
	aut_space = strstr(aut, "realm=");

	if (aut_space != NULL)
	    aut_space += 6;

	mp_msg(MSGT_NETWORK, MSGL_INFO, MSGTR_MPDEMUX_NW_AuthRequiredFor, aut_space);

    } else {
	mp_msg(MSGT_NETWORK, MSGL_INFO, MSGTR_MPDEMUX_NW_AuthRequired);
    }

    if (network_username) {
	url->username = strdup33(network_username);

	if (url->username == NULL) {
	    mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
	    return -1;
	}

    } else {
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_AuthFailed);
	return -1;
    }

    if (network_password) {
	url->password = strdup33(network_password);

	if (url->password == NULL) {
	    mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
	    return -1;
	}

    } else {
	mp_msg(MSGT_NETWORK, MSGL_INFO, MSGTR_MPDEMUX_NW_NoPasswdProvidedTryingBlank);
    }

    (*auth_retry)++;
    return 0;
}
/*
*
*
*
*/
int http_seek(stream_t *stream, off_t pos)
{

    HTTP_header_t *http_hdr = NULL;
    int fd = -1;

    if (stream == NULL) {
	OS_PRINTF("[%s][ERROR] stream == NULL!!!!\n", __func__);
	return 0;
    }

//This if removed for network play issue fix, Gavin 2013-09-09
//if (stream->fd > 0) closesocket(stream->fd); // need to reconnect to seek in http-stream
//yliu add :for  fd = 0
#ifdef __LINUX__
    if (stream->fd > 0) {
	closesocket(stream->fd); // need to reconnect to seek in http-stream
    }
#else
    if (stream->fd >= 0) {
	closesocket(stream->fd);
	stream->fd = -1;
    }
#endif

    if (is_file_seq_exit()) {

	return -1;
    }

    fd = http_send_request(stream->streaming_ctrl->url, pos);

    if (fd < 0) {
	OS_PRINTF("[%s][ERROR] fail to send request  fd[%d]!!!!!!!!\n", __func__, fd);
	return 0;
    }

    http_hdr = http_read_response(fd);

    if (http_hdr == NULL) {
	//yliu add:
	closesocket(fd);
	fd = -1;
	OS_PRINTF("[%s][ERROR] fail to read response !!!!!!!!\n", __func__);
	return 0;
    }

    stream->streaming_ctrl->chunksize = http_hdr->chunksize;
    if (mp_msg_test(MSGT_NETWORK, MSGL_V)) {
	http_debug_hdr(http_hdr);
    }

    switch (http_hdr->status_code) {
    case 200:
    case 206: // OK
	//mp_msg(MSGT_NETWORK, MSGL_V, "Content-Type: [%s]\n", http_get_field(http_hdr, "Content-Type"));
	//mp_msg(MSGT_NETWORK, MSGL_V, "Content-Length: [%s]\n", http_get_field(http_hdr, "Content-Length"));
	OS_PRINTF("[%s] Content-Type: [%s]\n", __func__, http_get_field(http_hdr, "Content-Type"));
	OS_PRINTF("[%s] Content-Length: [%s]\n", __func__, http_get_field(http_hdr, "Content-Length"));
	//  yliu add :for reconnect
	//if(pos == 0)
	{
	    if (http_get_field(http_hdr, "Content-Length"))
		stream->end_pos = atoll(http_get_field(http_hdr, "Content-Length")) + pos;
	    else
		stream->end_pos = 0;
	}
	OS_PRINTF("[%s] stream->end_pos: [%ld]\n", __func__, stream->end_pos);

	if (http_hdr->body_size > 0) {
	    if (streaming_bufferize(stream->streaming_ctrl, http_hdr->body, http_hdr->body_size) < 0) {
		http_free33(http_hdr);
		closesocket(fd);
		fd = -1;
		return -1;
	    }
	}

	break;

    default:
	mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ErrServerReturned, http_hdr->status_code, http_hdr->reason_phrase);
	closesocket(fd);
	fd = -1;
    }
//add macro for consistency in Linux Version, yliu 2013-09-10
#if 0
	//This if added for network play issue fix, Gavin 2013-09-09
	if(stream->fd >= 0)
	{
		closesocket(stream->fd); // need to reconnect to seek in http-stream
		stream->fd = -1;
	}
#endif

    stream->fd = fd;

    if (http_hdr) {

	http_free33(http_hdr);
	stream->streaming_ctrl->data = NULL;
    }

    stream->pos = pos;
    return 1;
}

/*
*
*
*
*/
int streaming_bufferize(streaming_ctrl_t *streaming_ctrl, char *buffer, int size)
{
    //printf("streaming_bufferize\n");
    OS_PRINTF("[%s] start start ...\n", __func__);
    OS_PRINTF("[%s] size[%d]...\n", __func__, size);

    streaming_ctrl->buffer = malloc33(size);

    if (streaming_ctrl->buffer == NULL) {
	mp_msg(MSGT_NETWORK, MSGL_FATAL, MSGTR_MemAllocFailed);
	return -1;
    }

    memcpy(streaming_ctrl->buffer, buffer, size);
    streaming_ctrl->buffer_size = size;

    OS_PRINTF("[%s] end end ...\n", __func__);
    return size;
}

int nop_chunk_rcv(int fd, int size, streaming_ctrl_t *stream_ctrl)
{
    int ret;
    //OS_PRINTF("[%s]line[%d],chunksize[%lld]\n",__func__,__LINE__,phc->chunksize);
    ret = recv(fd, stream_ctrl->hc_buf, size, 0);
    if (ret < 0) {
	mp_msg(MSGT_NETWORK, MSGL_ERR, "nop_streaming_read error : %s\n", strerror(errno));
	ret = 0;
    } else if (ret == 0)
	stream_ctrl->status = streaming_stopped_e;
#ifndef __LINUX__
    if (!ret)
	OS_PRINTF("[%s]rev000[0]\n", __func__);
#endif
    return ret;
}
static int nop_chunk_getc(int fd, streaming_ctrl_t *stream_ctrl)
{
    int len;
    if (stream_ctrl->hc_buf_ptr >= stream_ctrl->hc_buf_end) {
	len = nop_chunk_rcv(fd, stream_ctrl->hc_buf_size, stream_ctrl);
	//OS_PRINTF("[%s]rcv[%d]\n",__func__,len);
	if (len < 0) {
	    return len;
	} else if (len == 0) {
	    return -1;
	} else {
	    stream_ctrl->hc_buf_ptr = stream_ctrl->hc_buf;
	    stream_ctrl->hc_buf_end = stream_ctrl->hc_buf + len;
	}
    }
    return *stream_ctrl->hc_buf_ptr++;
}

int nop_chunk_read(int fd, char *buffer, int size,
                   streaming_ctrl_t *stream_ctrl)
{
    int len = 0;
    int buffer_len = 0;

    if (is_file_seq_exit()) {
	OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	return TCP_ERROR_TIMEOUT;
    }

    if (stream_ctrl->buffer_size != 0) {
	buffer_len = stream_ctrl->buffer_size - stream_ctrl->buffer_pos;
	len = (size < buffer_len) ? size : buffer_len;
	if (buffer_len > stream_ctrl->hc_buf_size) {
#ifndef __LINUX__
	    mtos_printk("[%s]buffer_len000>hc_buf_size\n", __func__);
#endif
	    return -1;
	}
	memcpy(stream_ctrl->hc_buf, (stream_ctrl->buffer) + (stream_ctrl->buffer_pos), buffer_len);
	free33(stream_ctrl->buffer);
	stream_ctrl->buffer = NULL;
	stream_ctrl->buffer_size = 0;
	stream_ctrl->buffer_pos = 0;
	stream_ctrl->hc_buf_ptr = stream_ctrl->hc_buf;
	stream_ctrl->hc_buf_end = stream_ctrl->hc_buf_ptr + buffer_len;
	stream_ctrl->chunksize = 0;
    }
    if (!stream_ctrl->chunksize) {
	char line[32];
	int ch;
	char *q;
	for (;;) {
	    do {
		q = line;

		if (is_file_seq_exit()) {
		    OS_PRINTF("\n%s %d \n", __func__, __LINE__);
		    return TCP_ERROR_TIMEOUT;
		}

		while ((q - line) < 32 - 1) {
		    ch = nop_chunk_getc(fd, stream_ctrl);
		    if (ch < 0)
			return ch;
		    if (ch == '\n') {
			// process line
			if (q > line && q[-1] == '\r')
			    q--;
			*q = '\0';
			break;
		    } else {
			if ((q - line) < 32 - 1)
			    *q++ = ch;
		    }
		}
	    } while (!*line); // skip CR LF from last chunk

	    stream_ctrl->chunksize = strtoll(line, NULL, 16);
	    if (!stream_ctrl->chunksize)
		return 0;
	    break;
	}
    }
    buffer_len = stream_ctrl->hc_buf_end - stream_ctrl->hc_buf_ptr;
    if (stream_ctrl->chunksize && !buffer_len) {
	buffer_len = nop_chunk_rcv(fd, stream_ctrl->hc_buf_size, stream_ctrl);
	stream_ctrl->hc_buf_ptr = stream_ctrl->hc_buf;
	stream_ctrl->hc_buf_end = stream_ctrl->hc_buf + buffer_len;
    }
    if (stream_ctrl->chunksize >= buffer_len)
	len = buffer_len;
    else
	len = stream_ctrl->chunksize;
    if (len >= size)
	len = size;
    memcpy(buffer, stream_ctrl->hc_buf_ptr, len);
    stream_ctrl->hc_buf_ptr += len;
    stream_ctrl->chunksize -= len;
    return len;
}
/*
*
*
*
*/
extern void network_listen_start(void *p_param, void *p_task_par, int task_par_len);
extern int network_listen_stop(void *p_param, int size);
extern NETWORK_LISTEN_T network_listen;
int nop_streaming_read(int fd, char *buffer, int size, streaming_ctrl_t *stream_ctrl)
{

    int len = 0;

    if (is_file_seq_exit()) {
	OS_PRINTF("\n%s %d \n", __func__, __LINE__);
	return TCP_ERROR_TIMEOUT;
    }

    // OS_PRINTF("\n%s %d \n",__func__,__LINE__);
    //printf("nop_streaming_read\n");
    if (stream_ctrl->chunksize < 0) {
	if (stream_ctrl->buffer_size != 0) {
	    int buffer_len = stream_ctrl->buffer_size - stream_ctrl->buffer_pos;
	    //printf("%d bytes in buffer\n", stream_ctrl->buffer_size);
	    len = (size < buffer_len) ? size : buffer_len;
	    memcpy(buffer, (stream_ctrl->buffer) + (stream_ctrl->buffer_pos), len);
	    stream_ctrl->buffer_pos += len;
	    //printf("buffer_pos = %d\n", stream_ctrl->buffer_pos );
	    if (stream_ctrl->buffer_pos >= stream_ctrl->buffer_size) {
		free33(stream_ctrl->buffer);
		stream_ctrl->buffer = NULL;
		stream_ctrl->buffer_size = 0;
		stream_ctrl->buffer_pos = 0;
		//printf("buffer cleaned\n");
	    }
	    //printf("read %d bytes from buffer\n", len );
	}
	//OS_PRINTF("\n%s %d \n",__func__,__LINE__);
	if (len < size) {
	    int ret;

#if 0
    {
        CONNECT_PARAM_T  connect_para;
        connect_para.fd = fd;
        connect_para.size = size-len;
        connect_para.p_buf = buffer+len;
        network_listen.task_type =  NET_TYPE_RECV;
        network_listen_start(&network_listen, &connect_para,0);
        ret = network_listen_stop(&network_listen, 0);
    }

#else
	    ret = recv(fd, buffer + len, size - len, 0);
#endif
	    if (ret < 0) {
		mp_msg(MSGT_NETWORK, MSGL_ERR, "nop_streaming_read error : %s\n", strerror(errno));
		ret = 0;
	    } else if (ret == 0)
		stream_ctrl->status = streaming_stopped_e;
	    len += ret;
	    //printf("read %d bytes from network\n", len );
	}
    } else {
	len = nop_chunk_read(fd, buffer, size, stream_ctrl);
    }
    //OS_PRINTF("\n%s %d \n",__func__,__LINE__);
    return len;
}
/*
*
*
*
*/
int nop_streaming_seek(int fd, off_t pos, streaming_ctrl_t *stream_ctrl)
{
    return -1;
}

