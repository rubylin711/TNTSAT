/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include "config.h"
#ifdef __LINUX__
#include <stdlib.h>
#include <string.h>
#else
#include "mp_func_trans.h"
#endif
#include "mp_msg.h"
#include "network.h"
#include "stream.h"
#include "url.h"
#include "udp.h"
#include "srt.h"

#define false 0
#define true 1

/* libsrt defines default packet size as 1316 internally
 * so srt module takes same value. */
#define SRT_DEFAULT_CHUNK_SIZE 1316
/* The default timeout is -1 (infinite) */
#define SRT_DEFAULT_POLL_TIMEOUT -1
/* The default latency is 125
 * which uses srt library internally */
#define SRT_DEFAULT_LATENCY 125
/* Crypto key length in bytes. */


static int srt_schedule_reconnect(stream_t *stream,URL_t *url)
{
	printf("[%s:%s:%d] ----davis  start\n",__FILE__, __func__,__LINE__);

    int         i_latency;
    int         stat;
    //char        *psz_passphrase = NULL;
     int failed = 0;
   /*
    struct addrinfo hints = {
        .ai_socktype = SOCK_DGRAM,
    }, *res = NULL;
   */
   struct addrinfo hints= { 0 }, *ai;
   memset(&hints, 0, sizeof(hints));
   hints.ai_socktype=SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;
	char portstr[10];
	 snprintf(portstr, sizeof(portstr), "%d", url->port);
   stat = getaddrinfo(url->hostname? url->hostname : NULL, portstr, &hints, &ai);
   if(stat != 0){
	 printf("---------getaddrinfo failed, errno = %d", stat);
	 failed = 1;
	 goto out;
   }
   printf("[%s:%s:%d] ----davis  \n",__FILE__, __func__,__LINE__);
   // stream_sys_t *p_sys = p_stream->p_sys;

 /*
    stat = vlc_getaddrinfo( p_sys->psz_host, p_sys->i_port, &hints, &res );
    if ( stat )
    {
        msg_Err( p_stream, "Cannot resolve [%s]:%d (reason: %s)",
                 p_sys->psz_host,
                 p_sys->i_port,
                 gai_strerror( stat ) );

        failed = 1;
        goto out;
    }
*/
    /* Always start with a fresh socket */
    if (stream!= SRT_INVALID_SOCK)
    {
        srt_epoll_remove_usock( stream->poll_id, stream->fd);
        srt_close( stream->fd );
    }

    stream->fd = srt_socket( ai->ai_family, SOCK_DGRAM, 0 );
    if ( stream->fd == SRT_INVALID_SOCK )
    {
    	 printf("[%s:%s:%d] Failed to open socket.\n",__FILE__, __func__,__LINE__);

        failed = 1;
        goto out;
    }
	printf("[%s:%s:%d] ----davis  \n",__FILE__, __func__,__LINE__);
    /* Make SRT non-blocking */
    srt_setsockopt(stream->fd, 0, SRTO_SNDSYN,
        &(int) { false }, sizeof( int ) );
    srt_setsockopt( stream->fd, 0, SRTO_RCVSYN,
        &(int) { false }, sizeof( int ) );

    /* Make sure TSBPD mode is enable (SRT mode) */
    srt_setsockopt(stream->fd, 0, SRTO_TSBPDMODE,
        &(int) { 1 }, sizeof( int ) );

    /* This is an access module so it is always a receiver */
    srt_setsockopt( stream->fd, 0, SRTO_SENDER,
        &(int) { 0 }, sizeof( int ) );

    /* Set latency */
    //i_latency = var_InheritInteger( p_stream, "latency" );
    i_latency = SRT_DEFAULT_LATENCY;
    srt_setsockopt(stream->fd, 0, SRTO_TSBPDDELAY,
        &i_latency, sizeof( int ) );
   /*
    psz_passphrase = var_InheritString( p_stream, "passphrase" );
    if ( psz_passphrase != NULL && psz_passphrase[0] != '\0')
    {
        int i_key_length = var_InheritInteger( p_stream, "key-length" );
        srt_setsockopt( stream->fd, 0, SRTO_PASSPHRASE,
            psz_passphrase, strlen( psz_passphrase ) );
        srt_setsockopt( stream->fd, 0, SRTO_PBKEYLEN,
            &i_key_length, sizeof( int ) );
    }*/

    srt_epoll_add_usock( stream->poll_id, stream->fd,
        &(int) { SRT_EPOLL_ERR | SRT_EPOLL_IN });

    /* Schedule a connect */
   // msg_Dbg( p_stream, "Schedule SRT connect (dest addresss: %s, port: %d).",
   //     p_sys->psz_host, p_sys->i_port);

    stat = srt_connect( stream->fd, ai->ai_addr, ai->ai_addrlen);
   printf("[%s:%s:%d] ----davis  stat:%d\n",__FILE__, __func__,__LINE__,stat);
    if ( stat == SRT_ERROR )
    {
	 printf("[%s:%s:%d] Failed to connect to server (reason: %s)\n",__FILE__, __func__,__LINE__,srt_getlasterror_str() );
        failed = 1;
    }

out:
    if (failed && stream->fd != SRT_INVALID_SOCK)
    {
        srt_epoll_remove_usock( stream->poll_id, stream->fd );
        srt_close(stream->fd);
        stream->fd = SRT_INVALID_SOCK;
    }

    freeaddrinfo( ai );
    //free( psz_passphrase );
    printf("[%s:%s:%d] ----davis  end %d\n",__FILE__, __func__,__LINE__,!failed);
    return !failed;
}


static int
srt_streaming_read (int fd, char *buffer,
                     int size, streaming_ctrl_t *stream_ctrl)
{
   //printf("[%s:%s:%d] ----davis  start\n",__FILE__, __func__,__LINE__);
    stream_t *stream = (stream_t*)(stream_ctrl->i_stream);
    int stat = 0;
    SRTSOCKET ready[1];
    int readycnt = 1;
    while ( srt_epoll_wait( stream->poll_id,
    ready, &readycnt, 0, 0,
    SRT_DEFAULT_POLL_TIMEOUT, NULL, 0, NULL, 0 ) >= 0)
    {
    if ( readycnt < 0  || ready[0] != fd )
    {
        /* should never happen, force recovery */
        srt_close(fd);
        stream->fd= SRT_INVALID_SOCK;
    }

    switch( srt_getsockstate(fd) )
    {
        case SRTS_CONNECTED:
            /* Good to go */
            break;
        case SRTS_BROKEN:
        case SRTS_NONEXIST:
        case SRTS_CLOSED:
            /* Failed. Schedule recovery. */
            if ( !srt_schedule_reconnect( stream,stream_ctrl->url) )
                printf("[%s:%s:%d] ----davis  Failed to schedule connect\n",__FILE__, __func__,__LINE__);
            /* Fall-through */
        default:
            /* Not ready */
            continue;
    }

    stat = srt_recvmsg( fd,
        buffer, SRT_DEFAULT_CHUNK_SIZE );
    if ( stat > 0 )
    {
        goto out;
    }

     printf("[%s:%s:%d] ----davis  failed to receive packet, set EOS (reason: %s)\n",__FILE__, __func__,__LINE__,srt_getlasterror_str());
    break;
    }

    out:
   	 return stat;
}

static int
srt_streaming_start (stream_t *stream,URL_t *url)
{
  streaming_ctrl_t *streaming_ctrl;
  int fd;
	printf("[%s:%s:%d] ----davis  start\n",__FILE__, __func__,__LINE__);
  if (!stream)
    return -1;
  srt_startup();

  stream->poll_id = srt_epoll_create();
  if(stream->poll_id == -1){
  	printf("[%s:%s:%d] ----davis  failed to create poll id for SRT socket.\n",__FILE__, __func__,__LINE__);
  	return -1;
  }
	printf("[%s:%s:%d] ----davis  poll_id:%d\n",__FILE__, __func__,__LINE__,stream->poll_id);
   if ( !srt_schedule_reconnect( stream,url) ){
   	printf("[%s:%s:%d] ----davis  Failed to schedule connect\n",__FILE__, __func__,__LINE__);
	return -1;
   }
  streaming_ctrl = stream->streaming_ctrl;
  /*
  fd = stream->fd;
  if (fd < 0)
  {
    fd = srt_socket (streaming_ctrl->url);
    if (fd < 0)
      return -1;
    stream->fd = fd;
  }*/
	printf("[%s:%s:%d] ----davis  \n",__FILE__, __func__,__LINE__);
  streaming_ctrl->streaming_read = srt_streaming_read;
  streaming_ctrl->streaming_seek = NULL;
  streaming_ctrl->prebuffer_size = 16 * 1024; /* 4KBytes */
  streaming_ctrl->buffering = 0;
  streaming_ctrl->status = streaming_playing_e;
  //streaming_ctrl->i_poll_id = stream->poll_id;
  streaming_ctrl->i_stream=stream;
	printf("[%s:%s:%d] ----davis  end\n",__FILE__, __func__,__LINE__);
  return 0;
}

static int Control(stream_t *s, int cmd, void *arg)
{
    return 0;
}

static void
srt_streaming_close (struct stream *s)
{
	  srt_epoll_remove_usock( s->poll_id, s->fd );
        srt_close( s->fd);
        srt_epoll_release( s->poll_id );
     srt_cleanup();
}

static int
srt_stream_open (stream_t *stream, int mode, void *opts, int *file_format)
{
  URL_t *url;
	printf("[%s:%s:%d] ----davis  start\n",__FILE__, __func__,__LINE__);
  mp_msg (MSGT_OPEN, MSGL_INFO, "STREAM_SRT, URL: %s\n", stream->url);
  stream->streaming_ctrl = streaming_ctrl_new ();
  if (!stream->streaming_ctrl)
    return STREAM_ERROR;
printf("[%s:%s:%d] ----davis  stream->url:%s,network_bandwidth:%d\n",__FILE__, __func__,__LINE__,stream->url,network_bandwidth);
  stream->streaming_ctrl->bandwidth = network_bandwidth;
  url = url_new (stream->url);
  stream->streaming_ctrl->url = check4proxies (url);
 // stream->streaming_ctrl->url = url_new(stream->url);

printf("[%s:%s:%d] ----davis  \n",__FILE__, __func__,__LINE__);
  if (url->port == 0)
  {
    mp_msg (MSGT_NETWORK, MSGL_ERR,
            "You must enter a port number for SRT streams!\n");
    streaming_ctrl_free33(stream->streaming_ctrl);
    stream->streaming_ctrl = NULL;
    printf("[%s:%s:%d] ----davis  STREAM_UNSUPPORTED\n",__FILE__, __func__,__LINE__);
    return STREAM_UNSUPPORTED;
  }
  printf("[%s:%s:%d] ----davis url->hostname:%s,url->port:%d,ulr->protocol:%s\n",__FILE__, __func__,__LINE__,url->hostname,url->port,url->protocol);
  if (srt_streaming_start (stream,url) < 0)
  {
    mp_msg (MSGT_NETWORK, MSGL_ERR, "srt_streaming_start failed\n");
    streaming_ctrl_free33(stream->streaming_ctrl);
    stream->streaming_ctrl = NULL;

    return STREAM_UNSUPPORTED;
  }
  printf("[%s:%s:%d] ----davis  \n",__FILE__, __func__,__LINE__);
  stream->type = STREAMTYPE_STREAM;
  stream->close = srt_streaming_close;
  stream->control=Control;
  printf("[%s:%s:%d] ----davis  end ok\n",__FILE__, __func__,__LINE__);
  return STREAM_OK;
}

const stream_info_t stream_info_srt = {
  "SRT streaming",
  "srt",
  "srt",
  "native srt support",
  srt_stream_open,
  { "srt", NULL},
  NULL,
  0 // Urls are an option string
};
