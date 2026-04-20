/*
 * HTTP Helper
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

#include "config.h"

#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#else
#include "mp_func_trans.h"
#endif
#include <string.h>
#include <unistd.h>

#if !HAVE_WINSOCK2_H
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "tcp.h"
#include "url.h"
#include "http.h"
#include "mp_msg.h"
#include "cookies.h"

#include "stream.h"
#include "libmpdemux/demuxer.h"
#include "network.h"
#ifndef __LINUX__
#include <limits.h>
#include "lib_unicode.h"
#include "mt_type.h"
#include "sys_define.h"
#include "ethernet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "ethernet_priv.h"
#endif
#include "help_mp.h"

#include "mp_strings.h"
#include "libavutil/base64.h"
#include "file_playback_sequence.h"

#define MAX_RECV_SIZE    1440
#define CHUNK_HEADER_PARSE 1
#define CHUNK_BODY_PARSE 0

#ifdef NAME_MAX
#undef NAME_MAX
#define NAME_MAX         255	/* # chars in a file name */
#endif
#ifdef PATH_MAX
#undef PATH_MAX
#define PATH_MAX         255	/* # chars in a path name including nul */
#endif

#ifndef NAME_MAX
#define NAME_MAX         255	/* # chars in a file name */
#endif
#ifndef PATH_MAX
#define PATH_MAX         255	/* # chars in a path name including nul */
#endif
#define MAX_DOMAIN_NAME_LEN 63
#define MAX_URL_LEN      2083
#ifdef __LINUX__
#define OS_PRINTF printf
#endif

#define HTTP_REQUEST_PORT1 80   // if you want change port num, you should check
#define HTTP_REQUEST_PORT2 8080 // weather conflict to mini_httpd server port defined in mini_httpd.h

typedef struct {
	unsigned metaint;
	unsigned metapos;
	int is_ultravox;
} scast_data_t;

////////////////////////////////////////////////////////////////////////////////////
//add for download file
typedef struct {
	URL_t *url;
	char *path;
	char *buffer_send;
	char *buffer_recv;
	char *save_path;
	char *file_name;
	char *location;
	int fd;
	long content_length;
	unsigned int status_code;
	int is_chunked;
} http_download_t;

typedef struct {
	char url[MAX_URL_LEN];
	char filename[PATH_MAX+NAME_MAX];
	int download_size;
	int recv_size;
} break_point_st_t;

/*
*for chunk recv.
*/
static unsigned int leftChunkLen = 0;
static unsigned int lastChunkLen = 0;

HTTP_proxy_t *g_httpproxy = NULL;

//end add for download file
////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief first read any data from sc->buffer then from fd
 * \param fd file descriptor to read data from
 * \param buffer buffer to read into
 * \param len how many bytes to read
 * \param sc streaming control containing buffer to read from first
 * \return len unless there is a read error or eof
 */
static unsigned my_read(int fd, char *buffer, int len, streaming_ctrl_t *sc) {
	unsigned pos = 0;
	unsigned cp_len = sc->buffer_size - sc->buffer_pos;
	if (cp_len > len)
		cp_len = len;
	memcpy(buffer, &sc->buffer[sc->buffer_pos], cp_len);
	sc->buffer_pos += cp_len;
	pos += cp_len;
	while (pos < len) {
		int ret = recv(fd, &buffer[pos], len - pos, 0);
		if (ret <= 0)
			break;
		pos += ret;
	}
	return pos;
}

/**
 * \brief read and process (i.e. discard *g*) a block of ultravox metadata
 * \param fd file descriptor to read from
 * \param sc streaming_ctrl_t whose buffer is consumed before reading from fd
 * \return number of real data before next metadata block starts or 0 on error
 *
 * You can use unsv://samples.mplayerhq.hu/V-codecs/VP5/vp5_artefacts.nsv to
 * test.
 */
static unsigned uvox_meta_read(int fd, streaming_ctrl_t *sc) {
	unsigned metaint;
	unsigned char info[6] = {0, 0, 0, 0, 0, 0};
	int info_read;
	do {
		info_read = my_read(fd, info, 1, sc);
		if (info[0] == 0x00)
			info_read = my_read(fd, info, 6, sc);
		else
			info_read += my_read(fd, &info[1], 5, sc);
		if (info_read != 6) // read error or eof
			return 0;
		// sync byte and reserved flags
		if (info[0] != 0x5a || (info[1] & 0xfc) != 0x00) {
			mp_msg(MSGT_DEMUXER, MSGL_ERR, "Invalid or unknown uvox metadata\n");
			return 0;
		}
		if (info[1] & 0x01)
			mp_msg(MSGT_DEMUXER, MSGL_WARN, "Encrypted ultravox data\n");
		metaint = info[4] << 8 | info[5];
		if ((info[3] & 0xf) < 0x07) { // discard any metadata nonsense
			char *metabuf = malloc33(metaint);
			my_read(fd, metabuf, metaint, sc);
			free33(metabuf);
		}
	} while ((info[3] & 0xf) < 0x07);
	return metaint;
}

/**
 * \brief read one scast meta data entry and print it
 * \param fd file descriptor to read from
 * \param sc streaming_ctrl_t whose buffer is consumed before reading from fd
 */
static void scast_meta_read(int fd, streaming_ctrl_t *sc) {
	unsigned char tmp = 0;
	unsigned metalen;
	my_read(fd, &tmp, 1, sc);
	metalen = tmp * 16;
	if (metalen > 0) {
		int i;
		uint8_t *info = malloc33(metalen + 1);
		unsigned nlen = my_read(fd, info, metalen, sc);
		// avoid breaking the user's terminal too much
		if (nlen > 256) nlen = 256;
		for (i = 0; i < nlen; i++)
			if (info[i] && info[i] < 32) info[i] = '?';
		info[nlen] = 0;
		mp_msg(MSGT_DEMUXER, MSGL_INFO, "\nICY Info: %s\n", info);
		free33(info);
	}
}

/**
 * \brief read data from scast/ultravox stream without any metadata
 * \param fd file descriptor to read from
 * \param buffer buffer to read data into
 * \param size number of bytes to read
 * \param sc streaming_ctrl_t whose buffer is consumed before reading from fd
 */
static int scast_streaming_read(int fd, char *buffer, int size,
		streaming_ctrl_t *sc) {
	scast_data_t *sd = (scast_data_t *)sc->data;
	unsigned block, ret;
	unsigned done = 0;

	// first read remaining data up to next metadata
	block = sd->metaint - sd->metapos;
	if (block > size)
		block = (unsigned)size;
	ret = my_read(fd, buffer, block, sc);
	sd->metapos += ret;
	done += ret;
	if (ret != block) // read problems or eof
		size = (unsigned)done;

	while (done < size) { // now comes the metadata
		if (sd->is_ultravox)
		{
			sd->metaint = uvox_meta_read(fd, sc);
			if (!sd->metaint)
				size = (unsigned)done;
		}
		else
			scast_meta_read(fd, sc); // read and display metadata
		sd->metapos = 0;
		block = size - done;
		if (block > sd->metaint)
			block = sd->metaint;
		ret = my_read(fd, &buffer[done], block, sc);
		sd->metapos += ret;
		done += ret;
		if (ret != block) // read problems or eof
			size = (unsigned)done;
	}
	return done;
}
/*
*
*
*
*/
static int scast_streaming_start(stream_t *stream) {

	OS_PRINTF("[%s] start start ...\n",__func__);

	int metaint;
	scast_data_t *scast_data;
    if(stream == NULL)
    {
        OS_PRINTF("[%s] ERROR1...\n",__func__);
        return -1;
    }
	HTTP_header_t *http_hdr = stream->streaming_ctrl->data;
	int is_ultravox = strcasecmp(stream->streaming_ctrl->url->protocol, "unsv") == 0;

	if (stream->fd < 0 || !http_hdr)
	{
		OS_PRINTF("[%s] ERROR1...\n",__func__);
		return -1;
	}

	if (is_ultravox)
	{
		metaint = 0;
	}
	else
	{
		metaint = atoi(http_get_field(http_hdr, "Icy-MetaInt"));
		if (metaint <= 0)
		{
			OS_PRINTF("[%s] metaint:%d...\n",__func__,metaint);
			return -1;
		}
	}
	stream->streaming_ctrl->buffer = malloc33(http_hdr->body_size);
	stream->streaming_ctrl->buffer_size = http_hdr->body_size;
	stream->streaming_ctrl->buffer_pos = 0;
	memcpy(stream->streaming_ctrl->buffer, http_hdr->body, http_hdr->body_size);
	scast_data = malloc33(sizeof(scast_data_t));
	scast_data->metaint = metaint;
	scast_data->metapos = 0;
	scast_data->is_ultravox = is_ultravox;
	http_free33(http_hdr);
	stream->streaming_ctrl->data = scast_data;
	stream->streaming_ctrl->streaming_read = scast_streaming_read;
	stream->streaming_ctrl->streaming_seek = NULL;
	stream->streaming_ctrl->prebuffer_size = 64 * 1024; // 64 KBytes
	stream->streaming_ctrl->buffering = 1;
	stream->streaming_ctrl->status = streaming_playing_e;


	OS_PRINTF("[%s] end end ...\n",__func__);
	return 0;


}



/*
*
*
*
*/
static int nop_streaming_start( stream_t *stream ) {

	//OS_PRINTF("[%s] start start ...\n",__func__);

	HTTP_header_t *http_hdr = NULL;
	char *next_url=NULL;
	URL_t *rd_url=NULL;
	int fd,ret;

	if( stream==NULL )
	{
		OS_PRINTF("[%s][ERROR] stream==NULL!!!!\n",__func__);
		return -1;
	}

	fd = stream->fd;
	if( fd < 0 ) {

		//OS_PRINTF("[%s] stream->fd:%d \n",__func__,stream->fd);

		fd = http_send_request( stream->streaming_ctrl->url, 0 );
		if( fd<0 )
		{
			return -1;
		}

		http_hdr = http_read_response( fd );

		if( http_hdr==NULL ) {
			closesocket(fd);
       		fd = -1;
			OS_PRINTF("[%s][ERROR] fail to read response !!!!!!!!\n",__func__);
			return -1;
		}

		switch( http_hdr->status_code ) {

			case 200: // OK
				mp_msg(MSGT_NETWORK,MSGL_V,"Content-Type: [%s]\n", http_get_field(http_hdr, "Content-Type") );
				mp_msg(MSGT_NETWORK,MSGL_V,"Content-Length: [%s]\n", http_get_field(http_hdr, "Content-Length") );
				if( http_hdr->body_size>0 ) {
					if( streaming_bufferize( stream->streaming_ctrl, http_hdr->body, http_hdr->body_size )<0 ) {
						http_free33( http_hdr );
						return -1;
					}
				}
				break;
				// Redirect
			case 301: // Permanently
			case 302: // Temporarily
			case 303: // See Other
			case 307: // Temporarily (since HTTP/1.1)
				ret=-1;
				next_url = http_get_field( http_hdr, "Location" );

				if (next_url != NULL)
					rd_url=url_new(next_url);

				if (next_url != NULL && rd_url != NULL) {
					mp_msg(MSGT_NETWORK,MSGL_STATUS,"Redirected: Using this url instead %s\n",next_url);
					stream->streaming_ctrl->url=check4proxies(rd_url);
					ret=nop_streaming_start(stream); //recursively get streaming started
				} else {
					mp_msg(MSGT_NETWORK,MSGL_ERR,"Redirection failed\n");
					closesocket( fd );
					fd = -1;
				}
				return ret;
				break;
			case 401: //Authorization required
			case 403: //Forbidden
			case 404: //Not found
			case 500: //Server Error
			default:
				mp_msg(MSGT_NETWORK,MSGL_ERR,"Server returned code %d: %s\n", http_hdr->status_code, http_hdr->reason_phrase );
				closesocket( fd );
				fd = -1;
				return -1;
				break;
		}

		stream->fd = fd;


	}
	else
	{
		OS_PRINTF("[%s] stream->fd:%d \n",__func__,stream->fd);

		http_hdr = (HTTP_header_t*)stream->streaming_ctrl->data;
		if( http_hdr->body_size > 0 ) {

			if( streaming_bufferize( stream->streaming_ctrl, http_hdr->body, http_hdr->body_size ) < 0 ) {
				http_free33( http_hdr );
				stream->streaming_ctrl->data = NULL;
				return -1;
			}
		}
	}

	if( http_hdr ) {

		http_free33( http_hdr );
		stream->streaming_ctrl->data = NULL;
	}

	stream->streaming_ctrl->streaming_read = nop_streaming_read;
	stream->streaming_ctrl->streaming_seek = nop_streaming_seek;
	stream->streaming_ctrl->prebuffer_size = 64*1024; // 64 KBytes
	stream->streaming_ctrl->buffering = 1;
	stream->streaming_ctrl->status = streaming_playing_e;


	//OS_PRINTF("[%s] prebuffer_size[%d]...\n",__func__,stream->streaming_ctrl->prebuffer_size);
	//OS_PRINTF("[%s] buffering[%d]...\n",__func__,stream->streaming_ctrl->buffering);
	//OS_PRINTF("[%s] end end ...\n",__func__);

	return 0;

}


/*
*
*
*
*/

HTTP_header_t *
mthttp_http_new_header(void) {
	HTTP_header_t *http_hdr;
#ifndef __LINUX__
	http_hdr = mthttp_calloc33(1, sizeof(*http_hdr));
#else
	http_hdr = calloc33(1, sizeof(*http_hdr));
#endif
	if( http_hdr==NULL ) return NULL;

	return http_hdr;
}
/*
*
*
*
*/
HTTP_header_t *
http_new_header(void) {
	HTTP_header_t *http_hdr;

	http_hdr = calloc33(1, sizeof(*http_hdr));
	if( http_hdr==NULL ) return NULL;
        http_hdr->chunksize = -1;

	return http_hdr;
}
/*
*
*
*
*/
void
mthttp_http_free33( HTTP_header_t *http_hdr ) {
    HTTP_field_t *field, *field2free;
    if( http_hdr==NULL ) return;
#ifndef __LINUX__
    if(http_hdr->protocol){
        mthttp_free33(http_hdr->protocol);
        http_hdr->protocol = NULL;
    }
    if(http_hdr->uri){
        mthttp_free33(http_hdr->uri);
        http_hdr->uri = NULL;
    }

    if(http_hdr->reason_phrase){
        mthttp_free33(http_hdr->reason_phrase);
        http_hdr->reason_phrase = NULL;
    }

    if(http_hdr->field_search){
        mthttp_free33(http_hdr->field_search);
        http_hdr->field_search = NULL;
    }

    if(http_hdr->method){
        mthttp_free33(http_hdr->method);
        http_hdr->method = NULL;
    }

    if(http_hdr->buffer){
        mthttp_free33(http_hdr->buffer);
        http_hdr->buffer = NULL;
    }

    field = http_hdr->first_field;
    while( field!=NULL ) {
        field2free = field;
        mthttp_free33(field->field_name);
        field = field->next;
        mthttp_free33( field2free );
    }
    mthttp_free33( http_hdr );
#else
    if(http_hdr->protocol){
        free33(http_hdr->protocol);
        http_hdr->protocol = NULL;
    }
    if(http_hdr->uri){
        free33(http_hdr->uri);
        http_hdr->uri = NULL;
    }

    if(http_hdr->reason_phrase){
        free33(http_hdr->reason_phrase);
        http_hdr->reason_phrase = NULL;
    }

    if(http_hdr->field_search){
        free33(http_hdr->field_search);
        http_hdr->field_search = NULL;
    }

    if(http_hdr->method){
        free33(http_hdr->method);
        http_hdr->method = NULL;
    }

    if(http_hdr->buffer){
        free33(http_hdr->buffer);
        http_hdr->buffer = NULL;
    }

    field = http_hdr->first_field;
    while( field!=NULL ) {
        field2free = field;
        free33(field->field_name);
        field = field->next;
        free33( field2free );
    }
    free33( http_hdr );
#endif
    http_hdr = NULL;
}
/*
*
*
*
*/
void  http_free33( HTTP_header_t *http_hdr ) {

	HTTP_field_t *field, *field2free;
	if( http_hdr==NULL )
		return;

	if(http_hdr->protocol)
	{
		free33(http_hdr->protocol);
		http_hdr->protocol = NULL;
	}

	if(http_hdr->uri)
	{
		free33(http_hdr->uri);
		http_hdr->uri = NULL;
	}

	if(http_hdr->reason_phrase)
	{
		free33(http_hdr->reason_phrase);
		http_hdr->reason_phrase = NULL;
	}

	if(http_hdr->field_search)
	{
		free33(http_hdr->field_search);
		http_hdr->field_search = NULL;
	}

	if(http_hdr->method)
	{
		free33(http_hdr->method);
		http_hdr->method = NULL;
	}

	if(http_hdr->buffer)
	{
		free33(http_hdr->buffer);
		http_hdr->buffer = NULL;
	}

	field = http_hdr->first_field;
	while( field!=NULL ) {
		field2free = field;
		free33(field->field_name);
		field = field->next;
		free33( field2free );
	}

	free33( http_hdr );
	http_hdr = NULL;

}
/*
*
*
*
*/
#define HTTP_BUF_INCREASE_SIZE (1024*512)
int
mthttp_http_response_append( HTTP_header_t *http_hdr, char *response, int length ) {
       char *buf_ptr = NULL;
	if( http_hdr==NULL || response==NULL || length<0 ) return -1;
	if( (unsigned)length > SIZE_MAX - http_hdr->buffer_size - 1) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,"Bad size in memory (re)allocation\n");
		return -1;
	}
	if(http_hdr->buffer_size + length <= http_hdr->content_length)
	{
	     memcpy(http_hdr->buffer + http_hdr->buffer_size, response, length);
	     http_hdr->buffer_size += length;
	     http_hdr->buffer[http_hdr->buffer_size]=0; // close the string!
	     return http_hdr->buffer_size;
	}
#ifndef __LINUX__
	buf_ptr = mthttp_realloc33( http_hdr->buffer, http_hdr->content_length + HTTP_BUF_INCREASE_SIZE);
#else
	buf_ptr = realloc33( http_hdr->buffer, http_hdr->content_length + HTTP_BUF_INCREASE_SIZE);
#endif
	if( buf_ptr ==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
	}
	http_hdr->buffer = buf_ptr;
	memcpy( http_hdr->buffer+http_hdr->buffer_size, response, length );
	http_hdr->content_length += HTTP_BUF_INCREASE_SIZE;
	http_hdr->buffer_size += length;
	http_hdr->buffer[http_hdr->buffer_size]=0; // close the string!
	return http_hdr->buffer_size;
}
/*
*
*
*
*/
int  http_response_append( HTTP_header_t *http_hdr, char *response, int length ) {

	if( http_hdr==NULL || response==NULL || length<0 )
		return -1;

	if( (unsigned)length > SIZE_MAX - http_hdr->buffer_size - 1) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,"Bad size in memory (re)allocation\n");
		return -1;
	}

	http_hdr->buffer = realloc33( http_hdr->buffer, http_hdr->buffer_size+length+1 );
	if( http_hdr->buffer==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
	}

	memcpy( http_hdr->buffer+http_hdr->buffer_size, response, length );
	http_hdr->buffer_size += length;
	http_hdr->buffer[http_hdr->buffer_size]=0; // close the string!

	return http_hdr->buffer_size;

}
/*
*
*
*
*/
int
http_is_header_entire( HTTP_header_t *http_hdr ) {

	if( http_hdr==NULL )
	{
		return -1;
	}

	if( http_hdr->buffer==NULL )
	{
		return 0; // empty
	}

	if( strstr(http_hdr->buffer, "\r\n\r\n") == NULL &&
			strstr(http_hdr->buffer, "\n\n") == NULL )
	{
			return 0;
	}

	return 1;

}
/*
 * Parse Hex Data
 * param:  *s
 *              *number
 * return: bits
 */
static unsigned char ParseHexUL(const char *s, unsigned int *number)
{
    unsigned char bits  = 0;
    unsigned int val = 0;

    while (*s != '\0')
    {
        if ((*s >= '0') && (*s <= '9'))
        {
            val = val * 16 + *s++ - '0';
        }
        else if ((*s >= 'a') && (*s <= 'f'))
        {
            val = val * 16 + *s++ - 'a' + 10;
        }
        else
            break;

        if (++bits >= 8)
            break;     // 8 Hex bits Max: u32 Para
    }

    *number = val;

    return (bits);
}

/*
Descrip:   Function was called when server response had the filed "Transfer-Encoding:chunked".
Param :   *pbuf: The server response.
                *len_in: server response data length,include \r\n and chunk_length lines.
                *len_out:real data length.
                header_flag: 1:header data; 0:body data.
Return :   success: 0  failed: -1
*/

/*

cookies process

*/



/*

cookies end

*/



/*
Descrip:   Function was called when server response had the filed "Transfer-Encoding:chunked".
Param :   *response_buf: The server response that don't include the header field.
                                         only the response body.
                *length: The sum of all chunk_size.
Return :   success: 0  failed: -1
*/
int mthttp_http_chunked_parse(char *response_buf, unsigned int *length)
{
    unsigned int nBytes=0, body_size=0;
    unsigned short chunk_len;
    char *pChunkStart=response_buf, *pHead=response_buf;
    char* pTemp;
    char str_len[9];   //the length of a chunk -- ASCII

    do{
            if(pChunkStart == NULL)
            {
                OS_PRINTF("%s %d: ERROR, HTTP header buffer is NULL\n", __func__, __LINE__);
                return -1;
            }

            pTemp =strstr(pChunkStart,"\r\n");
            if(NULL==pTemp)
            {
                OS_PRINTF("[%s][%d], cant get chunk size separator!\n", __func__,__LINE__);
                return -1;
            }
            /*get chunk size.(the string like "12ab" which means 0x12ab)*/
            chunk_len = pTemp - pChunkStart;
            strncpy(str_len, pChunkStart, chunk_len);
            str_len[chunk_len] = '\0';

            pChunkStart=pTemp+2;//skip "\r\n"(2 bytes)

            /*parse string to dec. And the string is like "12ab" which means 0x12ab.*/
            ParseHexUL(str_len, &nBytes);
            if(nBytes == 0)//the length of last chunk must be 0;
            {
                *length = body_size;
                return 0;
            }
            //OS_PRINTF("[%s][%d],nBytes=[%u]!\n", __func__,__LINE__, nBytes);
            body_size = body_size + nBytes;
            /*append again!don't need to realloc,just memcpy in the same buffer.*/
            memcpy(pHead, pChunkStart, nBytes);

            pHead = pHead + nBytes;
            pChunkStart=pChunkStart+nBytes+2; //nBytes && "\r\n"
    }while(nBytes > 0);
return 0;
}
/*
*
*
*
*/
int
mthttp_http_response_parse( HTTP_header_t *http_hdr ) {
	char *hdr_ptr, *ptr, *pchunk;
	char *field=NULL;
	int pos_hdr_sep, hdr_sep_len;
	size_t len, body_size = 0;
	if( http_hdr==NULL ) return -1;
	if( http_hdr->is_parsed ) return 0;

	if(http_hdr->buffer == NULL|| http_hdr->buffer_size == 0)
	{
	    OS_PRINTF("%s %d: ERROR, HTTP header buffer is NULL\n", __func__, __LINE__);
	    return -1;
	}
	// Get the protocol
	hdr_ptr = strstr( http_hdr->buffer, " " );
	if( hdr_ptr==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. No space separator found.\n");
		return -1;
	}
	len = hdr_ptr-http_hdr->buffer;
#ifndef __LINUX__
	http_hdr->protocol = mthttp_malloc33(len+1);
#else
	http_hdr->protocol = malloc33(len+1);
#endif
	if( http_hdr->protocol==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
	}
	strncpy( http_hdr->protocol, http_hdr->buffer, len );
	http_hdr->protocol[len]='\0';
	if( !strncasecmp( http_hdr->protocol, "HTTP", 4) ) {
		if( sscanf( http_hdr->protocol+5,"1.%d", &(http_hdr->http_minor_version) )!=1 ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get HTTP minor version.\n");
			return -1;
		}
	}

	// Get the status code
	if( sscanf( ++hdr_ptr, "%d", &(http_hdr->status_code) )!=1 ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get status code.\n");
		return -1;
	}
	hdr_ptr += 4;
	// Get the reason phrase
	if(hdr_ptr == NULL)
	    OS_PRINTF("%s %d: ERROR, HTTP header buffer is NULL\n", __func__, __LINE__);
	ptr = strstr( hdr_ptr, "\n" );
	if( ptr==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get the reason phrase.\n");
		return -1;
	}
	len = ptr-hdr_ptr;
#ifndef __LINUX__
	http_hdr->reason_phrase = mthttp_malloc33(len+1);
#else
	http_hdr->reason_phrase = malloc33(len+1);
#endif
	if( http_hdr->reason_phrase==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
	}
	strncpy( http_hdr->reason_phrase, hdr_ptr, len );
	if( http_hdr->reason_phrase[len-1]=='\r' ) {
		len--;
	}
	http_hdr->reason_phrase[len]='\0';

	// Set the position of the header separator: \r\n\r\n
	hdr_sep_len = 4;
	if(http_hdr->buffer == NULL)
	    OS_PRINTF("%s %d: ERROR, HTTP header buffer is NULL\n", __func__, __LINE__);
	ptr = strstr( http_hdr->buffer, "\r\n\r\n" );
	if( ptr==NULL ) {
		if(http_hdr->buffer == NULL)
		    OS_PRINTF("%s: ERROR, HTTP header buffer is NULL\n", __func__);
		ptr = strstr( http_hdr->buffer, "\n\n" );
		if( ptr==NULL ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,"Header may be incomplete. No CRLF CRLF found.\n");
			return -1;
		}
		hdr_sep_len = 2;
	}
	pos_hdr_sep = ptr-http_hdr->buffer;

	// Point to the first line after the method line.
	if(http_hdr->buffer == NULL)
	    OS_PRINTF("%s %d: ERROR, HTTP header buffer is NULL\n", __func__, __LINE__);
	hdr_ptr = strstr( http_hdr->buffer, "\n" )+1;
	do {
		ptr = hdr_ptr;
		while( *ptr!='\r' && *ptr!='\n' ) ptr++;
		len = ptr-hdr_ptr;
		if( len==0 ) break;
#ifndef __LINUX__
		field = mthttp_realloc33(field, len+1);
#else
		field = realloc33(field, len+1);
#endif
		if( field==NULL ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MemAllocFailed);
			return -1;
		}
		strncpy( field, hdr_ptr, len );
		field[len]='\0';
		mthttp_http_set_field( http_hdr, field );
		hdr_ptr = ptr+((*ptr=='\r')?2:1);
	} while( hdr_ptr<(http_hdr->buffer+pos_hdr_sep) );
#ifndef __LINUX__
	mthttp_free33(field);
#else
	free33(field);
#endif
        if(http_hdr->buffer == NULL)
            OS_PRINTF("%s %d: ERROR, HTTP header buffer is NULL\n", __func__, __LINE__);
        pchunk = strstr(http_hdr->buffer, "Transfer-Encoding");
	if(pchunk != NULL)
	{
	    OS_PRINTF("[%s][%d], have Transfer-Encoding\n", __func__,__LINE__);
            if(http_hdr->buffer == NULL)
                OS_PRINTF("%s %d: ERROR, HTTP header buffer is NULL\n", __func__, __LINE__);
            pchunk = strstr(http_hdr->buffer, "chunked");
            if(pchunk != NULL)
            {
                if(0 != mthttp_http_chunked_parse(http_hdr->buffer+pos_hdr_sep+hdr_sep_len, (unsigned int *)&body_size))
                {
                    OS_PRINTF("[%s][%d], chunked parse error!\n", __func__,__LINE__);
                    return -1;
                }
            }
            else
                OS_PRINTF("[%s][%d], NO CHUNKED!\n", __func__, __LINE__);
	}
	else
        	OS_PRINTF("[%s][%d], NO field : Transfer-Encoding!\n", __func__, __LINE__);

	if( pos_hdr_sep+hdr_sep_len<http_hdr->buffer_size )
        {
		// Response has data!
		if(body_size > 0) //chunked encoding
                {
                    //OS_PRINTF("[%s][%d], will realloc buffer! body_size=[%u]\n", __func__, __LINE__, body_size);
                    /*realloc, because the old buffer is larger than the real buffer.
                        Old buffer contains the "chunk_body" and the "body_size line".
                        And real buffer contains the "chunk_body" only!*/
                    #ifndef __LINUX__
                	http_hdr->buffer = mthttp_realloc33( http_hdr->buffer, pos_hdr_sep+hdr_sep_len+body_size +1 );
                    #else
                	http_hdr->buffer = realloc33( http_hdr->buffer, pos_hdr_sep+hdr_sep_len+body_size +1 );
                    #endif

                    http_hdr->body = http_hdr->buffer+pos_hdr_sep+hdr_sep_len;
		    http_hdr->body_size = (size_t)body_size;
                }
                else
                {
                    http_hdr->body = http_hdr->buffer+pos_hdr_sep+hdr_sep_len;
		    http_hdr->body_size = http_hdr->buffer_size-(pos_hdr_sep+hdr_sep_len);
                }
	}

	http_hdr->is_parsed = 1;
	return 0;

}
/*
*
*
*
*/
int   http_response_parse( HTTP_header_t *http_hdr ) {

	char *hdr_ptr, *ptr;
	char *field=NULL;
	int pos_hdr_sep, hdr_sep_len;
	size_t len;

	if( http_hdr==NULL )
		return -1;

	if( http_hdr->is_parsed )
		return 0;

	// Get the protocol
	hdr_ptr = strstr( http_hdr->buffer, " " );
	if( hdr_ptr==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. No space separator found.\n");
		return -1;
	}

	//OS_PRINTF("http_hdr->buffer[%s]\n", http_hdr->buffer);
	len = hdr_ptr-http_hdr->buffer;
	http_hdr->protocol = malloc33(len+1);
	if( http_hdr->protocol==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
	}

	strncpy( http_hdr->protocol, http_hdr->buffer, len );
	http_hdr->protocol[len]='\0';
	if( !strncasecmp( http_hdr->protocol, "HTTP", 4) ) {
		if( sscanf( http_hdr->protocol+5,"1.%d", &(http_hdr->http_minor_version) )!=1 ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get HTTP minor version.\n");
			return -1;
		}
	}

	// Get the status code
	if( sscanf( ++hdr_ptr, "%d", &(http_hdr->status_code) )!=1 ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get status code.\n");
		return -1;
	}
	hdr_ptr += 4;

	// Get the reason phrase
	ptr = strstr( hdr_ptr, "\n" );
	if( ptr==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get the reason phrase.\n");
		return -1;
	}
	len = ptr-hdr_ptr;

	http_hdr->reason_phrase = malloc33(len+1);
	if( http_hdr->reason_phrase==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
	}
	strncpy( http_hdr->reason_phrase, hdr_ptr, len );
	if( http_hdr->reason_phrase[len-1]=='\r' ) {
		len--;
	}
	http_hdr->reason_phrase[len]='\0';

	// Set the position of the header separator: \r\n\r\n
	hdr_sep_len = 4;
	ptr = strstr( http_hdr->buffer, "\r\n\r\n" );
	if( ptr==NULL ) {
		ptr = strstr( http_hdr->buffer, "\n\n" );
		if( ptr==NULL ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,"Header may be incomplete. No CRLF CRLF found.\n");
			return -1;
		}
		hdr_sep_len = 2;
	}
	pos_hdr_sep = ptr-http_hdr->buffer;

	// Point to the first line after the method line.
	hdr_ptr = strstr( http_hdr->buffer, "\n" )+1;
	do {
		ptr = hdr_ptr;
		while( *ptr!='\r' && *ptr!='\n' ) ptr++;
		len = ptr-hdr_ptr;
		if( len==0 ) break;
		field = realloc33(field, len+1);
		if( field==NULL ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MemAllocFailed);
			return -1;
		}
		strncpy( field, hdr_ptr, len );
		field[len]='\0';
		http_set_field( http_hdr, field );
		hdr_ptr = ptr+((*ptr=='\r')?2:1);
	} while( hdr_ptr<(http_hdr->buffer+pos_hdr_sep) );

	free33(field);
       ptr = strstr( http_hdr->buffer, "Transfer-Encoding" );
       if(ptr)
       {
            ptr = strstr(http_hdr->buffer, "chunked");
            if(ptr)
                http_hdr->chunksize = 0;
       }
	if( pos_hdr_sep+hdr_sep_len<http_hdr->buffer_size ) {
		// Response has data!
                    http_hdr->body = http_hdr->buffer+pos_hdr_sep+hdr_sep_len;
                    http_hdr->body_size = http_hdr->buffer_size-(pos_hdr_sep+hdr_sep_len);
	}

	http_hdr->is_parsed = 1;
	return 0;
}
/*
*
*
*
*/
char *
mthttp_http_build_request( HTTP_header_t *http_hdr, HTTP_spec_info_t *spec_info) {
	char *ptr, *uri=NULL;
	int len = 0;
	HTTP_field_t *field;
	if( http_hdr==NULL ) return NULL;

	if( http_hdr->method==NULL ) mthttp_http_set_method( http_hdr, "GET");
	if( http_hdr->uri==NULL ) mthttp_http_set_uri( http_hdr, "/");
	else {
#ifndef __LINUX__
		uri = mthttp_malloc33(strlen(http_hdr->uri) + 1);
#else
		uri = malloc33(strlen(http_hdr->uri) + 1);
#endif
		if( uri==NULL ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MemAllocFailed);
			return NULL;
		}
		strcpy(uri,http_hdr->uri);
	}

	//**** Compute the request length
	// Add the Method line
	    len = strlen(http_hdr->method)+strlen(uri)+14;
	// Add the fields
	field = http_hdr->first_field;
	while( field!=NULL ) {
		len += strlen(field->field_name)+2;
		field = field->next;
	}
	//Add the post value
	if(spec_info && spec_info->post_value_len!=NULL)
		len += spec_info->post_value_len + 2;
	// Add the body
	if( http_hdr->body!=NULL) {
		len += http_hdr->body_size;
	}
	// Free the buffer if it was previously used
	if( http_hdr->buffer!=NULL ) {
#ifndef __LINUX__
		mthttp_free33( http_hdr->buffer );
#else
		free33( http_hdr->buffer );
#endif
		http_hdr->buffer = NULL;
	}
#ifndef __LINUX__
	http_hdr->buffer = mthttp_malloc33(len+1);
#else
	http_hdr->buffer = malloc33(len+1);
#endif

	if( http_hdr->buffer==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MemAllocFailed);
		return NULL;
	}

	memset(http_hdr->buffer, 0, len+1);
	//http_hdr->buffer_size = len;

	//*** Building the request
	ptr = http_hdr->buffer;
	// Add the method line

	ptr += sprintf( ptr, "%s %s HTTP/1.%d\r\n", http_hdr->method, uri, http_hdr->http_minor_version );
	field = http_hdr->first_field;
	// Add the field
	while( field!=NULL ) {
		ptr += sprintf( ptr, "%s\r\n", field->field_name );
		field = field->next;
	}
	ptr +=sprintf( ptr, "\r\n" );
	// Add the Post Value
	if (spec_info && spec_info->post_value_len != 0)
	{
	      http_hdr->buffer_size = strlen(http_hdr->buffer);
	      memcpy(ptr, spec_info->p_post_value, spec_info->post_value_len );
	      ptr+= spec_info->post_value_len ;
	 }
	// Add the body
	if( http_hdr->body!=NULL )
	{
		memcpy( ptr, http_hdr->body, http_hdr->body_size );
	}
	if(spec_info && spec_info->post_value_len != 0)
     	       http_hdr->buffer_size = http_hdr->buffer_size + spec_info->post_value_len ;
       else
     	       http_hdr->buffer_size = strlen(http_hdr->buffer);

#ifndef __LINUX__
	mthttp_free33(uri);
       //mthttp_free33(p_post);
#else
	free33(uri);
#endif
	OS_PRINTF("request header is:\n%s\n", http_hdr->buffer);
	return http_hdr->buffer;
}
/*
*
*
*
*/
char *
http_build_request( HTTP_header_t *http_hdr ) {
	char *ptr, *uri=NULL;
	int len;
	HTTP_field_t *field;
	if( http_hdr==NULL ) return NULL;

	if( http_hdr->method==NULL ) http_set_method( http_hdr, "GET");
	if( http_hdr->uri==NULL ) http_set_uri( http_hdr, "/");
	else {
		uri = malloc33(strlen(http_hdr->uri) + 1);
		if( uri==NULL ) {
			mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MemAllocFailed);
			return NULL;
		}
		strcpy(uri,http_hdr->uri);
	}

	//**** Compute the request length
	// Add the Method line
	    len = strlen(http_hdr->method)+strlen(uri)+12;	// Add the fields
	field = http_hdr->first_field;
	while( field!=NULL ) {
		len += strlen(field->field_name)+2;
		field = field->next;
	}
	// Add the CRLF
	len += 2;
	// Add the body
	if( http_hdr->body!=NULL ) {
		len += http_hdr->body_size;
	}
	// Free the buffer if it was previously used
	if( http_hdr->buffer!=NULL ) {
		free33( http_hdr->buffer );
		http_hdr->buffer = NULL;
	}
	http_hdr->buffer = malloc33(len+1);
	if( http_hdr->buffer==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_ERR,MSGTR_MemAllocFailed);
		return NULL;
	}
	http_hdr->buffer_size = len;

	//*** Building the request
	ptr = http_hdr->buffer;
	// Add the method line
	ptr += sprintf( ptr, "%s %s HTTP/1.%d\r\n", http_hdr->method, uri, http_hdr->http_minor_version );
	field = http_hdr->first_field;
	// Add the field
	while( field!=NULL ) {
		ptr += sprintf( ptr, "%s\r\n", field->field_name );
		field = field->next;
	}
	ptr += sprintf( ptr, "\r\n" );
	// Add the body
	if( http_hdr->body!=NULL ) {
		memcpy( ptr, http_hdr->body, http_hdr->body_size );
	}

	free33(uri);
	return http_hdr->buffer;
}
/*
*
*
*
*/
char *
mthttp_http_get_field( HTTP_header_t *http_hdr, const char *field_name ) {
	if( http_hdr==NULL || field_name==NULL ) return NULL;
	http_hdr->field_search_pos = http_hdr->first_field;
#ifndef __LINUX__
	http_hdr->field_search = mthttp_realloc33( http_hdr->field_search, strlen(field_name)+1 );
#else
	http_hdr->field_search = realloc33( http_hdr->field_search, strlen(field_name)+1 );
#endif
	if( http_hdr->field_search==NULL ) {

		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return NULL;
	}
	memset(http_hdr->field_search, 0, strlen(field_name) + 1);
	strcpy( http_hdr->field_search, field_name );
	return http_get_next_field( http_hdr );
}
/*
*
*
*
*/
char *
http_get_field( HTTP_header_t *http_hdr, const char *field_name ) {
	if( http_hdr==NULL || field_name==NULL ) return NULL;
	http_hdr->field_search_pos = http_hdr->first_field;
	http_hdr->field_search = realloc33( http_hdr->field_search, strlen(field_name)+1 );
	if( http_hdr->field_search==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return NULL;
	}
	strcpy( http_hdr->field_search, field_name );
	return http_get_next_field( http_hdr );
}
/*
*
*
*
*/
char *
http_get_next_field( HTTP_header_t *http_hdr ) {
	char *ptr;
	HTTP_field_t *field;
	if( http_hdr==NULL ) return NULL;

	field = http_hdr->field_search_pos;
	while( field!=NULL ) {

		ptr = strstr( field->field_name, ":" );
		if( ptr==NULL ) return NULL;
		if( !strncasecmp( field->field_name, http_hdr->field_search, ptr-(field->field_name) ) ) {
			ptr++;	// Skip the column
			while( ptr[0]==' ' ) ptr++; // Skip the spaces if there is some
			http_hdr->field_search_pos = field->next;
			return ptr;	// return the value without the field name
		}

		field = field->next;
	}
	return NULL;
}
/*
*
*
*
*/
void
mthttp_http_set_field( HTTP_header_t *http_hdr, const char *field_name ) {
	HTTP_field_t *new_field;
	if( http_hdr==NULL || field_name==NULL ) return;
#ifndef __LINUX__
	new_field = mthttp_malloc33(sizeof(HTTP_field_t));
#else
	new_field = malloc33(sizeof(HTTP_field_t));
#endif
	if( new_field==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return;
	}
	new_field->next = NULL;
#ifndef __LINUX__
	new_field->field_name = mthttp_malloc33(strlen(field_name)+1);
#else
	new_field->field_name = malloc33(strlen(field_name)+1);
#endif
	if( new_field->field_name==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
#ifndef __LINUX__
		mthttp_free33(new_field);
#else
		free33(new_field);
#endif
		return;
	}
	strcpy( new_field->field_name, field_name );

	if( http_hdr->last_field==NULL ) {
		http_hdr->first_field = new_field;
	} else {
		http_hdr->last_field->next = new_field;
	}
	http_hdr->last_field = new_field;
	http_hdr->field_nb++;
}
/*
*
*
*
*/

void
http_set_field( HTTP_header_t *http_hdr, const char *field_name ) {
	HTTP_field_t *new_field;
	if( http_hdr==NULL || field_name==NULL ) return;

	new_field = malloc33(sizeof(HTTP_field_t));
	if( new_field==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return;
	}
	new_field->next = NULL;
	new_field->field_name = malloc33(strlen(field_name)+1);
	if( new_field->field_name==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		free33(new_field);
		return;
	}
	strcpy( new_field->field_name, field_name );

	if( http_hdr->last_field==NULL ) {
		http_hdr->first_field = new_field;
	} else {
		http_hdr->last_field->next = new_field;
	}
	http_hdr->last_field = new_field;
	http_hdr->field_nb++;
}
/*
*
*
*
*/
void
mthttp_http_set_method( HTTP_header_t *http_hdr, const char *method ) {
	if( http_hdr==NULL || method==NULL ) return;

#ifndef __LINUX__
	http_hdr->method = mthttp_malloc33(strlen(method)+1);
#else
	http_hdr->method = malloc33(strlen(method)+1);
#endif
	if( http_hdr->method==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return;
	}
	strcpy( http_hdr->method, method );
}
/*
*
*
*
*/
void
http_set_method( HTTP_header_t *http_hdr, const char *method ) {
	if( http_hdr==NULL || method==NULL ) return;

	http_hdr->method = malloc33(strlen(method)+1);
	if( http_hdr->method==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return;
	}
	strcpy( http_hdr->method, method );
}
/*
*
*
*
*/
void mthttp_http_set_uri( HTTP_header_t *http_hdr, const char *uri ) {
	if( http_hdr==NULL || uri==NULL ) return;
#ifndef __LINUX__
	http_hdr->uri = mthttp_malloc33(strlen(uri)+1);
#else
	http_hdr->uri = malloc33(strlen(uri)+1);
#endif
	if( http_hdr->uri==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return;
	}
	strcpy( http_hdr->uri, uri );
}
/*
*
*
*
*/
void
http_set_uri( HTTP_header_t *http_hdr, const char *uri ) {
	if( http_hdr==NULL || uri==NULL ) return;

	http_hdr->uri = malloc33(strlen(uri)+1);
	if( http_hdr->uri==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return;
	}
	strcpy( http_hdr->uri, uri );
}
/*
*
*
*
*/
static int
mthttp_http_add_authentication( HTTP_header_t *http_hdr, const char *username, const char *password, const char *auth_str ) {
	char *auth = NULL, *usr_pass = NULL, *b64_usr_pass = NULL;
	int encoded_len, pass_len=0;
	size_t auth_len, usr_pass_len;
	int res = -1;
	if( http_hdr==NULL || username==NULL ) return -1;

	if( password!=NULL ) {
		pass_len = strlen(password);
	}

	usr_pass_len = strlen(username) + 1 + pass_len;
#ifndef __LINUX__
	usr_pass = mthttp_malloc33(usr_pass_len + 1);
#else
	usr_pass = malloc33(usr_pass_len + 1);
#endif
	if( usr_pass==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		goto out;
	}

	sprintf( usr_pass, "%s:%s", username, (password==NULL)?"":password );

	encoded_len = AV_BASE64_SIZE(usr_pass_len);
#ifndef __LINUX__
	b64_usr_pass = mthttp_malloc33(encoded_len);
#else
	b64_usr_pass = malloc33(encoded_len);
#endif
	if( b64_usr_pass==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		goto out;
	}
	av_base64_encode(b64_usr_pass, encoded_len, usr_pass, usr_pass_len);

	auth_len = encoded_len + 100;
#ifndef __LINUX__
	auth = mthttp_malloc33(auth_len);
#else
	auth = malloc33(auth_len);
#endif
	if( auth==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		goto out;
	}

	snprintf(auth, auth_len, "%s: Basic %s", auth_str, b64_usr_pass);
	mthttp_http_set_field( http_hdr, auth );
	res = 0;

out:
#ifndef __LINUX__
	mthttp_free33( usr_pass );
	mthttp_free33( b64_usr_pass );
	mthttp_free33( auth );
#else
	free33( usr_pass );
	free33( b64_usr_pass );
	free33( auth );
#endif

	return res;
}

/*
*
*
*
*/
static int
http_add_authentication( HTTP_header_t *http_hdr, const char *username, const char *password, const char *auth_str ) {
	char *auth = NULL, *usr_pass = NULL, *b64_usr_pass = NULL;
	int encoded_len, pass_len=0;
	size_t auth_len, usr_pass_len;
	int res = -1;
	if( http_hdr==NULL || username==NULL ) return -1;

	if( password!=NULL ) {
		pass_len = strlen(password);
	}

	usr_pass_len = strlen(username) + 1 + pass_len;
	usr_pass = malloc33(usr_pass_len + 1);
	if( usr_pass==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		goto out;
	}

	sprintf( usr_pass, "%s:%s", username, (password==NULL)?"":password );

	encoded_len = AV_BASE64_SIZE(usr_pass_len);
	b64_usr_pass = malloc33(encoded_len);
	if( b64_usr_pass==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		goto out;
	}
	av_base64_encode(b64_usr_pass, encoded_len, usr_pass, usr_pass_len);

	auth_len = encoded_len + 100;
	auth = malloc33(auth_len);
	if( auth==NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		goto out;
	}

	snprintf(auth, auth_len, "%s: Basic %s", auth_str, b64_usr_pass);
	http_set_field( http_hdr, auth );
	res = 0;

out:
	free33( usr_pass );
	free33( b64_usr_pass );
	free33( auth );

	return res;
}

int
mthttp_http_add_basic_authentication( HTTP_header_t *http_hdr, const char *username, const char *password ) {
	return mthttp_http_add_authentication(http_hdr, username, password, "Authorization");
}

int
http_add_basic_authentication( HTTP_header_t *http_hdr, const char *username, const char *password ) {
	return http_add_authentication(http_hdr, username, password, "Authorization");
}


int
mthttp_http_add_basic_proxy_authentication( HTTP_header_t *http_hdr, const char *username, const char *password ) {
	return mthttp_http_add_authentication(http_hdr, username, password, "Proxy-Authorization");
}

int
http_add_basic_proxy_authentication( HTTP_header_t *http_hdr, const char *username, const char *password ) {
	return http_add_authentication(http_hdr, username, password, "Proxy-Authorization");
}

void
http_debug_hdr( HTTP_header_t *http_hdr ) {
	HTTP_field_t *field;
	int i = 0;
	if( http_hdr==NULL ) return;

	mp_msg(MSGT_NETWORK,MSGL_V,"--- HTTP DEBUG HEADER --- START ---\n");
	mp_msg(MSGT_NETWORK,MSGL_V,"protocol:           [%s]\n", http_hdr->protocol );
	mp_msg(MSGT_NETWORK,MSGL_V,"http minor version: [%d]\n", http_hdr->http_minor_version );
	mp_msg(MSGT_NETWORK,MSGL_V,"uri:                [%s]\n", http_hdr->uri );
	mp_msg(MSGT_NETWORK,MSGL_V,"method:             [%s]\n", http_hdr->method );
	mp_msg(MSGT_NETWORK,MSGL_V,"status code:        [%d]\n", http_hdr->status_code );
	mp_msg(MSGT_NETWORK,MSGL_V,"reason phrase:      [%s]\n", http_hdr->reason_phrase );
	mp_msg(MSGT_NETWORK,MSGL_V,"body size:          [%zu]\n", http_hdr->body_size );

	mp_msg(MSGT_NETWORK,MSGL_V,"Fields:\n");
	field = http_hdr->first_field;
	while( field!=NULL ) {
		mp_msg(MSGT_NETWORK,MSGL_V," %d - %s\n", i++, field->field_name );
		field = field->next;
	}
	mp_msg(MSGT_NETWORK,MSGL_V,"--- HTTP DEBUG HEADER --- END ---\n");
}
/*
*
*
*
*/
static void print_icy_metadata(HTTP_header_t *http_hdr) {
	const char *field_data;
	// note: I skip icy-notice1 and 2, as they contain html <BR>
	// and are IMHO useless info ::atmos
	if( (field_data = http_get_field(http_hdr, "icy-name")) != NULL )
	{
		mp_msg(MSGT_NETWORK,MSGL_INFO,"Name   : %s\n", field_data);
		OS_PRINTF("[%s] Name   : %s\n",__func__,field_data);
	}

	if( (field_data = http_get_field(http_hdr, "icy-genre")) != NULL )
	{
		mp_msg(MSGT_NETWORK,MSGL_INFO,"Genre  : %s\n", field_data);
		OS_PRINTF("[%s] Genre   : %s\n",__func__,field_data);
	}

	if( (field_data = http_get_field(http_hdr, "icy-url")) != NULL )
	{
		mp_msg(MSGT_NETWORK,MSGL_INFO,"Website: %s\n", field_data);
		OS_PRINTF("[%s] Website   : %s\n",__func__,field_data);
	}

	// XXX: does this really mean public server? ::atmos
	if( (field_data = http_get_field(http_hdr, "icy-pub")) != NULL )
	{
		mp_msg(MSGT_NETWORK,MSGL_INFO,"Public : %s\n", atoi(field_data)?"yes":"no");
		OS_PRINTF("[%s] Public   : %s\n",__func__,field_data);
	}

	if( (field_data = http_get_field(http_hdr, "icy-br")) != NULL )
	{
		mp_msg(MSGT_NETWORK,MSGL_INFO,"Bitrate: %skbit/s\n", field_data);
		OS_PRINTF("[%s] Bitrate: %skbit/s\n",__func__,field_data);
	}

}

//extern void print_long_url(u8 * filename);


/*
 *
 *
 *
 */
//! If this function succeeds you must closesocket stream->fd
static int http_streaming_start(stream_t *stream, int* file_format) {

	HTTP_header_t *http_hdr = NULL;
	int fd = stream->fd;
	int res = STREAM_UNSUPPORTED;
	int redirect = 0;
	int auth_retry=0;
	int seekable=0;
	char *content_type;
	const char *content_length;
	char *next_url;
	URL_t *url = stream->streaming_ctrl->url;

	//OS_PRINTF("[%s] start start ...\n",__func__);

	//print_long_url(stream->url);
	do
	{
		redirect = 0;
		//yliu add :for  fd = 0
#ifdef __LINUX__
		if (fd > 0)
#else
		if (fd >= 0)
#endif
		{
				closesocket(fd);
				fd  = -1;//peacer add
		}


		OS_PRINTF("[%s] start send request ...\n",__func__);
		fd = http_send_request( url, 0 );
		if( fd < 0 ) {

			OS_PRINTF("[%s]ERROR fail to send request !!!!!!!!!\n",__func__);
			goto err_out;
		}

		http_free33(http_hdr);


		//OS_PRINTF("[%s] end send request ...\n",__func__);

		//OS_PRINTF("[%s] start read response ...\n",__func__);

		http_hdr = http_read_response( fd );
		if( http_hdr==NULL ) {
			OS_PRINTF("[%s][ERROR] fail to response !!!!!!!!!!!!!\n",__func__);
			goto err_out;
		}


		if( mp_msg_test(MSGT_NETWORK,MSGL_V) ) {
			http_debug_hdr( http_hdr );
		}


		OS_PRINTF("[%s] end read response ...\n",__func__);
		// Check if we can make partial content requests and thus seek in http-streams
		if( http_hdr!=NULL && http_hdr->status_code==200 ) {

			const char *accept_ranges = http_get_field(http_hdr,"Accept-Ranges");
			const char *server = http_get_field(http_hdr, "Server");
			if (accept_ranges)
			{
				seekable = strncmp(accept_ranges,"bytes",5) == 0;
			}
			else if (server && (strcmp(server, "gvs 1.0") == 0 ||
						strncmp(server, "MakeMKV", 7) == 0))
			{
				// HACK for youtube and MakeMKV incorrectly claiming not to support seeking
				OS_PRINTF("Broken webserver, incorrectly claims to not support Accept-Ranges\n");
				seekable = 1;
			}
      seekable = 1;
		}

		print_icy_metadata(http_hdr);

		// Check if the response is an ICY status_code reason_phrase
		if( !strcasecmp(http_hdr->protocol, "ICY") ||  http_get_field(http_hdr, "Icy-MetaInt") ) {

			OS_PRINTF("[%s] find  ICY  Icy-MetaInt...!!!\n",__func__);

			switch( http_hdr->status_code ) {
				case 200: { // OK
						  char *field_data;
						  // If content-type == video/nsv we most likely have a winamp video stream
						  // otherwise it should be mp3. if there are more types consider adding mime type
						  // handling like later
						  if ( (field_data = http_get_field(http_hdr, "content-type")) != NULL && (!strcmp(field_data, "video/nsv") || !strcmp(field_data, "misc/ultravox")))
						  {
							  *file_format = DEMUXER_TYPE_NSV;
						  }
						  else if ( (field_data = http_get_field(http_hdr, "content-type")) != NULL && (!strcmp(field_data, "audio/aacp") || !strcmp(field_data, "audio/aac")))
						  {
							  *file_format = DEMUXER_TYPE_AAC;
						  }
						  else
						  {
							  *file_format = DEMUXER_TYPE_AUDIO;
						  }

						  res = STREAM_ERROR;
						  //OS_PRINTF("ERROR 33\n");
						  goto out;
					  }

				case 400: // Server Full
					  OS_PRINTF("ERROR Server Full!!!\n");
					  OS_PRINTF("Error: ICY-Server is full, skipping!\n");
					  goto err_out;

				case 401: // Service Unavailable
					  OS_PRINTF("ERROR Service Unavailable!!!\n");
					  OS_PRINTF("Error: ICY-Server return service unavailable, skipping!\n");
					  goto err_out;

				case 403: // Service Forbidden
					  OS_PRINTF("ERROR Service Forbidden!!!\n");
					  OS_PRINTF("Error: ICY-Server return 'Service Forbidden'\n");
					  goto err_out;

				case 404: // Resource Not Found
					  OS_PRINTF("ERROR Resource Not Found!!!\n");
					  OS_PRINTF("Error: ICY-Server couldn't find requested stream, skipping!\n");
					  goto err_out;

				default:
					  OS_PRINTF("ERROR 444!!!\n");
					  OS_PRINTF("Error: unhandled ICY-Errorcode, contact MPlayer developers!\n");
					  goto err_out;

			}

		}


		OS_PRINTF("[%s] Not Find  ICY  Icy-MetaInt...!!!\n",__func__);
		// Assume standard http if not ICY
		switch( http_hdr->status_code ) {

			case 200: // OK
				content_length = http_get_field(http_hdr, "Content-Length");
				if (content_length) {
					mp_msg(MSGT_NETWORK,MSGL_V,"Content-Length: [%s]\n", content_length);
					OS_PRINTF("[%s] Content-Length: [%s]\n",__func__,content_length);
					stream->end_pos = atoll(content_length);
					OS_PRINTF("[%s] stream->end_pos: [%ld]\n",__func__,stream->end_pos);
				}


				// Look if we can use the Content-Type
				content_type = http_get_field( http_hdr, "Content-Type" );
				if( content_type!=NULL ) {
					unsigned int i;

					OS_PRINTF("Content-Type: [%s]\n", content_type );
					// Check in the mime type table for a demuxer type
					for (i = 0; mime_type_table[i].mime_type != NULL; i++) {
						if( !strcasecmp( content_type, mime_type_table[i].mime_type ) ) {
							*file_format = mime_type_table[i].demuxer_type;
							res = seekable;
							OS_PRINTF("[%s] mime_type:%s\n",__func__,content_type);
							OS_PRINTF("[%s] demuxer_type:%d\n",__func__,mime_type_table[i].demuxer_type);
							goto out;
						}
					}
				}

				// Not found in the mime type table, don't fail,
				// we should try raw HTTP
				res = seekable;
				goto out;


				// Redirect
			case 301: // Permanently
			case 302: // Temporarily
			case 303: // See Other
			case 307: // Temporarily (since HTTP/1.1)

				OS_PRINTF("[%s] http_hdr->status_code:%d\n ",__func__,http_hdr->status_code );

				// TODO: RFC 2616, recommand to detect infinite redirection loops
				next_url = http_get_field( http_hdr, "Location" );
				if( next_url!=NULL ) {

					int is_ultravox = strcasecmp(stream->streaming_ctrl->url->protocol, "unsv") == 0;
					stream->streaming_ctrl->url = url_redirect( &url, next_url );

					if (url_is_protocol(url, "mms")) {
						res = STREAM_REDIRECTED;
						OS_PRINTF("[%s] ERROR mms!!!\n",__func__);
						goto err_out;
					}

					if (!url_is_protocol(url, "http")) {

						OS_PRINTF("[%s] Unsupported http %d redirect to %s protocol\n",__func__, http_hdr->status_code, url->protocol);
						goto err_out;
					}

					if (is_ultravox)
						url_set_protocol(url, "unsv");

					redirect = 1;

				}

				break;


			case 401: // Authentication required
				if( http_authenticate(http_hdr, url, &auth_retry)<0 )
				{
					OS_PRINTF("[%s][ERROR] Authentication required!!!\n",__func__);
					goto err_out;
				}
				redirect = 1;
				break;


			default:
				OS_PRINTF("[%s] Server returned %d: %s\n",__func__, http_hdr->status_code, http_hdr->reason_phrase );
				goto err_out;

		}

	} while( redirect );


err_out:
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (fd > 0)
	{
		closesocket( fd );
	}
#else
	if (fd >= 0)
	{
		closesocket( fd );
	}
#endif

	fd = -1;
	http_free33( http_hdr );
	http_hdr = NULL;
	OS_PRINTF("[%s] error 555!!!\n",__func__);


out:

	stream->streaming_ctrl->data = (void*)http_hdr;
	stream->fd = fd;

	OS_PRINTF("[%s], success return,   fd[%d]!!!!\n", __func__,fd);
	OS_PRINTF("[%s] end end ...\n",__func__);

	return res;

}
/*
*
*
*
*/
////////////////////////////////////////////////////////////////////////////////////
//add for download file
static void *memstr(void *src, unsigned int src_len, char *sub)
{
	if (NULL == src || NULL == sub || src_len < strlen(sub))
	{
		return NULL;
	}
	char *p = src;
	char *q = sub;
	unsigned int indx = src_len;
	unsigned int sub_len = strlen(sub);

	while (indx > 0)
	{
		int i = 0;
		while (i < sub_len)
		{
			char cp = *(p + i);
			char cq = *(q + i);// case ignore
			if (cq >= 'A' && cq <= 'Z')
			{
				cq |= 0x20;
			}
			if (cp >= 'A' && cp <= 'Z')
			{
				cp |= 0x20;
			}
			if (cq != cp)
			{
				break;
			}
			i++;
		}
		if (i == sub_len)
		{
			return p;
		}
		p++;
		indx--;
	}
	return NULL;
}
/*
*
*
*
*/
static void mthttp_filedownload_free33(http_download_t *p_http)
{
	if (NULL == p_http)
	{
#ifdef __LINUX__
		printf("p_http is NULL\n");
#else
		OS_PRINTF("p_http is NULL\n");
#endif
        return;     // for tsscan
	}
	mthttp_url_free33(p_http->url);
	p_http->url = NULL;
#ifndef __LINUX__
	if (p_http->save_path != NULL)
	{
		mthttp_free33(p_http->save_path);
		p_http->save_path = NULL;
	}
	if (p_http->buffer_send != NULL)
	{
		mthttp_free33(p_http->buffer_send);
		p_http->buffer_send = NULL;
	}
	if (p_http->buffer_recv != NULL)
	{
		mthttp_free33(p_http->buffer_recv);
		p_http->buffer_recv = NULL;
	}
	p_http->save_path = NULL;
	if (p_http->file_name != NULL)
	{
		mthttp_free33(p_http->file_name);
		p_http->file_name = NULL;
	}
	p_http->fd = -1;
	p_http->content_length = 0;
	p_http->status_code = 0;
	if (p_http->location != NULL)
	{
		mthttp_free33(p_http->location);
		p_http->location = NULL;
	}
#else
	if (p_http->save_path != NULL)
	{
		free33(p_http->save_path);
		p_http->save_path = NULL;
	}
	if (p_http->buffer_send != NULL)
	{
		free33(p_http->buffer_send);
		p_http->buffer_send = NULL;
	}
	if (p_http->buffer_recv != NULL)
	{
		free33(p_http->buffer_recv);
		p_http->buffer_recv = NULL;
	}
	p_http->save_path = NULL;
	if (p_http->file_name != NULL)
	{
		free33(p_http->file_name);
		p_http->file_name = NULL;
	}
	p_http->fd = -1;
	p_http->content_length = 0;
	p_http->status_code = 0;
	if (p_http->location != NULL)
	{
		free33(p_http->location);
		p_http->location = NULL;
	}
#endif
}
/*
*
*
*
*/
static void filedownload_free33(http_download_t *p_http)
{
	if (NULL == p_http)
	{
#ifdef __LINUX__
		printf("p_http is NULL\n");
#else
		OS_PRINTF("p_http is NULL\n");
#endif
        return ; // for tsscan
	}
	url_free33(p_http->url);
	p_http->url = NULL;
	if (p_http->save_path != NULL)
	{
		free33(p_http->save_path);
		p_http->save_path = NULL;
	}
	if (p_http->buffer_send != NULL)
	{
		free33(p_http->buffer_send);
		p_http->buffer_send = NULL;
	}
	if (p_http->buffer_recv != NULL)
	{
		free33(p_http->buffer_recv);
		p_http->buffer_recv = NULL;
	}
	p_http->save_path = NULL;
	if (p_http->file_name != NULL)
	{
		free33(p_http->file_name);
		p_http->file_name = NULL;
	}
	p_http->fd = -1;
	p_http->content_length = 0;
	p_http->status_code = 0;
	if (p_http->location != NULL)
	{
		free33(p_http->location);
		p_http->location = NULL;
	}
}
/*
*
*
*
*/
static int filedownload_get_filename(HTTP_header_t *http_hdr, char *url, char *filename)
{
	int len = 0;
	int i = 0;
	const char *content_disposition = NULL;
	char *filename_start = NULL;
	char *filename_end = NULL;

	OS_PRINTF("====>enter fun:%s\n", __func__);

	content_disposition = http_get_field(http_hdr, "Content-Disposition");
	//OS_PRINTF("content_disposition[%s]\n", content_disposition);
	if (content_disposition != NULL)
	{
		filename_start = strstr(content_disposition, "filename=") + 10;
		if ( NULL != filename_start)
		{
			filename_end = strstr(filename_start, "\"");
			if ( filename_end != NULL)
			{
				len =  strlen(filename_start) - strlen(filename_end);
				memcpy(filename, filename_start, len);
				filename[len] = '\0';
				//OS_PRINTF("========>filename[%s]\n", filename);
			}
		}
	}
	else
	{
		len = strlen(url);
		for (i = len - 1; i > 0; i--)
		{
			if (url[i] == '/')
			{
				break;
			}
		}
		if (i == 0) //error download address
		{
			OS_PRINTF("there is no '/' in url\n");
			return -1;
		}
		else
		{
			strcpy(filename, url + i + 1);
			OS_PRINTF("filename[%s]\n", filename);
			return 0;
		}
	}
return 0;
}
#if 0       // compile warning, ignore it
/*
*
*
*
*/
static int mthttp_filedownload_response_parse(char *buffer, http_download_t *p_http)
{
	char *p_tmp = NULL;
	char *ptr = NULL;
	int len = 0;

	if (buffer == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: Point param is NULL\n", __FUNCTION__, __LINE__);
        return -1;      // for tsscan
	}

	p_tmp = strstr(buffer, " ");
	if (p_tmp == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: Malformed answer. No space separator found.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}
	if (sscanf(++p_tmp, "%d", &p_http->status_code) != 1)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: Malformed answer. Unable to get status code.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	p_tmp = strstr(buffer, "Location");
	if (p_tmp != NULL)
	{
		ptr = strstr(p_tmp, ":");
		if (ptr != NULL)
		{
			ptr++;  // Skip the column
			while (ptr[0] == ' ') ptr++; // Skip the spaces if there is some
			p_tmp = ptr;
			while (*ptr != '\r' && *ptr != '\n') ptr++;
			len = ptr-p_tmp;
#ifndef __LINUX__
			p_http->location = mthttp_malloc33(len + 1);
#else
			p_http->location = malloc33(len + 1);
#endif
			if (p_http->location == NULL)
			{
				OS_PRINTF("[Fun:%s, Line:%d]: p_http->location malloc failed.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
			memset(p_http->location, 0, len + 1);
			strncpy(p_http->location, p_tmp, len);
		}
	}

	return 0;
}
#endif
/*
*
*
*
*/
static int filedownload_response_parse(char *buffer, http_download_t *p_http)
{
	char *p_tmp = NULL;
	char *ptr = NULL;
	int len = 0;

	if (buffer == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: Point param is NULL\n", __FUNCTION__, __LINE__);
        return -1;  // for tsscan
	}

	p_tmp = strstr(buffer, " ");
	if (p_tmp == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: Malformed answer. No space separator found.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}
	if (sscanf(++p_tmp, "%d", &p_http->status_code) != 1)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: Malformed answer. Unable to get status code.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	p_tmp = strstr(buffer, "Location");
	if (p_tmp != NULL)
	{
		ptr = strstr(p_tmp, ":");
		if (ptr != NULL)
		{
			ptr++;  // Skip the column
			while (ptr[0] == ' ') ptr++; // Skip the spaces if there is some
			p_tmp = ptr;
			while (*ptr != '\r' && *ptr != '\n') ptr++;
			len = ptr-p_tmp;
			p_http->location = malloc33(len + 1);
			if (p_http->location == NULL)
			{
				OS_PRINTF("[Fun:%s, Line:%d]: p_http->location malloc failed.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
			memset(p_http->location, 0, len + 1);
			strncpy(p_http->location, p_tmp, len);
		}
	}

	return 0;
}
#if 0       // compile warning, ignore it
/*
*
*
*
*/
static int mthttp_filedownload_save_data_trunk(http_download_t *p_http)
{
	int ret = -1;
	int finished = 0;
	unsigned int recvlen = 0;
	unsigned int bufferlen = 0;
//	unsigned int datalen = 0;


	unsigned long totaldatalen = 0;


	char *buffer_recv = NULL;
	char *data_start = NULL;


	//buffer_recv = mthttp_malloc33(BUFFER_SIZE);
	buffer_recv = p_http->buffer_recv;

	if (buffer_recv == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: buffer_recv malloc failed\n!", __FUNCTION__, __LINE__);
		return -1;
	}
	//memset(buffer_recv, 0, BUFFER_SIZE);
	while (1)
	{
		if (recvlen > p_http->content_length)
		{
			OS_PRINTF("%s:Error, receive buffer overflow.\n", __func__);
			return -1;
		}
		ret = recv(p_http->fd, buffer_recv + recvlen, bufferlen, 0);
		if (ret < 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: read data failed\n", __FUNCTION__, __LINE__);
			break;
		}
		else if (ret == 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: filedownload_read_response read 0 (i.e. EOF).\n",
					__FUNCTION__, __LINE__);
			finished = 1;
		}
		recvlen += ret;
		if (recvlen < BUFFER_SIZE)
		{
			bufferlen = BUFFER_SIZE - recvlen;
		}
		if ((recvlen == BUFFER_SIZE) || (finished == 1))
		{
			ret = mthttp_filedownload_response_parse(buffer_recv, p_http);
			if (p_http->status_code != 200)
			{
				OS_PRINTF("[Fun:%s, Line:%d]: p_http->status_code[%d]\n",
						__FUNCTION__, __LINE__, p_http->status_code);
				goto err_out;
			}
			if (totaldatalen == 0)
			{
				data_start = memstr((void *)buffer_recv, recvlen, "\r\n\r\n");
				if (data_start == NULL)
				{
					OS_PRINTF("[Fun:%s, Line:%d]: data return from server without header msg\n",
							__FUNCTION__, __LINE__);
					goto err_out;
				}
				else
				{
					data_start = data_start + 4;
				}
//				datalen = recvlen - (data_start - buffer_recv);
			}
			else
			{
				data_start = buffer_recv;
//				datalen = recvlen;
			}

		}
	}
	return 0;
err_out:
	return -1;
}
#endif
/*
*
*
*
*/
#ifdef __LINUX__
static int filedownload_save_data_trunk(int fd, http_download_t *p_http)
#else
static int filedownload_save_data_trunk(ufs_file_t *fd, http_download_t *p_http)
#endif
{
	int ret = -1;
	int finished = 0;
	unsigned int recvlen = 0;
	unsigned int bufferlen = 0;
	unsigned int datalen = 0;
//	unsigned int len_tmp = 0;
	unsigned int leave_len = 0;
	unsigned long totaldatalen = 0;
	unsigned int trunk_datalen = 0;	//every trunk data length
	unsigned int trunk_num = 0;	//record trunk num
#ifndef __LINUX__
	u32 write_len = 0;
#endif

	char *buffer_recv = NULL;
	char *data_start = NULL;
	char *buffer_tmp = NULL;

	buffer_recv = malloc33(BUFFER_SIZE);
	if (buffer_recv == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: buffer_recv malloc failed\n!", __FUNCTION__, __LINE__);
		return -1;
	}
	memset(buffer_recv, 0, BUFFER_SIZE);
	while (1)
	{
		if (recvlen == BUFFER_SIZE)
		{
			memset(buffer_recv, 0, BUFFER_SIZE);
			recvlen = 0;
			datalen = 0;
			buffer_tmp = NULL;
		}
		ret = recv(p_http->fd, buffer_recv + recvlen, bufferlen, 0);
		if (ret < 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: read data failed\n", __FUNCTION__, __LINE__);
			break;
		}
		else if (ret == 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: filedownload_read_response read 0 (i.e. EOF).\n",
					__FUNCTION__, __LINE__);
			finished = 1;
		}
		recvlen += ret;
		if (recvlen < BUFFER_SIZE)
		{
			bufferlen = BUFFER_SIZE - recvlen;
		}
		if ((recvlen == BUFFER_SIZE) || (finished == 1))
		{
			ret = filedownload_response_parse(buffer_recv, p_http);
			if (p_http->status_code != 200)
			{
				OS_PRINTF("[Fun:%s, Line:%d]: p_http->status_code[%d]\n",
						__FUNCTION__, __LINE__, p_http->status_code);
				goto err_out;
			}
			if (totaldatalen == 0)
			{
				data_start = memstr((void *)buffer_recv, recvlen, "\r\n\r\n");
				if (data_start == NULL)
				{
					OS_PRINTF("[Fun:%s, Line:%d]: data return from server without header msg\n",
							__FUNCTION__, __LINE__);
					goto err_out;
				}
				else
				{
					data_start = data_start + 4;
				}
				datalen = recvlen - (data_start - buffer_recv);
			}
			else
			{
				data_start = buffer_recv;
				datalen = recvlen;
			}
			while (1)
			{
				trunk_datalen = 0;
				if (leave_len != 0)
				{
#ifdef __LINUX__
					if (write(fd, data_start, leave_len) != datalen)
					{
						OS_PRINTF("[Fun:%s, Line:%d]: save file error\n",
								__FUNCTION__, __LINE__);
						goto err_out;
					}
#else
					ret = ufs_write(fd, data_start, leave_len, (u32 *)&write_len);
					if ((FR_OK != ret) || (leave_len != write_len))
					{
						OS_PRINTF("[Fun:%s, Line:%d]: save file error\n", __FUNCTION__, __LINE__);
						goto err_out;
					}
#endif
					data_start += (leave_len + 2);
					leave_len = 0;
				}

				buffer_tmp = data_start;
				sscanf(buffer_tmp, "%x", &trunk_datalen);
				trunk_num ++;
				OS_PRINTF("this trunk%d data len is %d\n", trunk_num, trunk_datalen);
				if (trunk_datalen == 0)
				{
					goto out;	// the last trunk data segment
				}
				data_start = memstr((void *)buffer_tmp, datalen, "\r\n");
				data_start += 2; //strlen("\r\n");
				datalen -= (data_start - buffer_tmp);
				if (trunk_datalen > datalen)
				{
//					len_tmp = datalen;
					leave_len = trunk_datalen - datalen;
				}
				else
				{
//					len_tmp = trunk_datalen;
				}
#ifdef __LINUX__
				if (write(fd, data_start, trunk_datalen) != datalen)
				{
					OS_PRINTF("[Fun:%s, Line:%d]: save file error\n",
							__FUNCTION__, __LINE__);
					goto err_out;
				}
#else
				ret = ufs_write(fd, data_start, trunk_datalen, (u32 *)&write_len);
				if ((FR_OK != ret) || (trunk_datalen != write_len))
				{
					OS_PRINTF("[Fun:%s, Line:%d]: save file error\n", __FUNCTION__, __LINE__);
					goto err_out;
				}
#endif
				totaldatalen += trunk_datalen;
				data_start += (trunk_datalen + 2);
				datalen -= trunk_datalen;
				if (datalen == 0)
				{
					break;
				}
			}
		}
	}

out:
#ifdef __LINUX__
	close(fd);
#else
	ufs_close(fd);
#endif
	return 0;

err_out:
#ifdef __LINUX__
	close(fd);
#else
	ufs_close(fd);
#endif
	return -1;
}
#if 0       // compile warning, ignore it
/*
*
*
*
*/
static int mthttp_filedownload_save_data(http_download_t *p_http)
{
	int ret = -1;
	int finished = 0;
	unsigned int recvlen = 0;
	unsigned int datalen = 0;
	unsigned int bufferlen = 0;

	char *buffer_recv = NULL;
	char *buffer_tmp = NULL;

	//buffer_recv = mthttp_malloc33(BUFFER_SIZE);
	buffer_recv = p_http->buffer_recv;
	if (buffer_recv == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: buffer_recv malloc failed\n!", __FUNCTION__, __LINE__);
		return -1;
	}
	bufferlen = p_http->content_length;

	while (1)
	{
		if (recvlen > p_http->content_length)
		{
			OS_PRINTF("%s:Error, receive buffer overflow.\n", __func__);
			return -1;
		}
		ret = recv(p_http->fd, buffer_recv + recvlen, bufferlen, 0);
		if (ret < 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: read data failed\n", __FUNCTION__, __LINE__);
			break;
		}
		else if (ret == 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: filedownload_read_response read 0 (i.e. EOF).\n",
					__FUNCTION__, __LINE__);
			finished = 1;
		}
		recvlen += ret;
		OS_PRINTF("[Fun:%s, Line:%d]: recv len this time is = %d\n", __FUNCTION__, __LINE__, ret);
		if (bufferlen == p_http->content_length)
		{
			ret = mthttp_filedownload_response_parse(buffer_recv, p_http);
			if (p_http->status_code != 200)
			{
				OS_PRINTF("[Fun:%s, Line:%d]: p_http->status_code[%d]\n",
						__FUNCTION__, __LINE__, p_http->status_code);
				goto err_out;
			}
			buffer_tmp = strstr(buffer_recv, "\r\n\r\n");
			if (buffer_tmp == NULL)
			{
				//buffer_tmp = buffer_recv;
				OS_PRINTF("[Fun:%s, Line:%d]: data return from server without header msg\n",
						__FUNCTION__, __LINE__);
				goto err_out;
			}
			else
			{
				buffer_tmp = buffer_tmp + 4;
			}
			datalen = recvlen - (buffer_tmp - buffer_recv);
			OS_PRINTF("%s:The first received len:%d valid data len:%d\n", __func__, recvlen, datalen);
			recvlen = datalen;
			memcpy(buffer_recv, buffer_tmp, datalen); //move valid data to the start pos of receive buffer

		}
		bufferlen = p_http->content_length - recvlen;
		if (recvlen >= p_http->content_length || finished == 1)
		{
			OS_PRINTF("%s:receive data finished.\n", __func__);
			break;
		}
	}
	if (recvlen == p_http->content_length)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: file download success\n", __FUNCTION__, __LINE__);
	}
	else
	{
		OS_PRINTF("[Fun:%s, Line:%d]: file download failed\n", __FUNCTION__, __LINE__);
	}

	return 0;
err_out:

	return -2;
}
#endif
/*
*
*
*
*/
#ifdef __LINUX__
static int filedownload_save_data(int fd, http_download_t *p_http)
#else
static int filedownload_save_data(ufs_file_t *fd, http_download_t *p_http)
#endif
{
	int ret = -1;
	int finished = 0;
	unsigned int recvlen = 0;
	unsigned int datalen = 0;
	unsigned int bufferlen = 0;
	unsigned long totaldatalen = 0;
#ifndef __LINUX__
	u32 write_len = 0;
#endif
	char *buffer_recv = NULL;
	char *buffer_tmp = NULL;

	buffer_recv = malloc33(BUFFER_SIZE);
	if (buffer_recv == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: buffer_recv malloc failed\n!", __FUNCTION__, __LINE__);
		return -1;
	}
	memset(buffer_recv, 0, BUFFER_SIZE);

	bufferlen = BUFFER_SIZE;

	while (1)
	{
		if (recvlen == BUFFER_SIZE)
		{
			memset(buffer_recv, 0, BUFFER_SIZE);
			recvlen = 0;
		}
		ret = recv(p_http->fd, buffer_recv + recvlen, bufferlen, 0);
		if (ret < 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: read data failed\n", __FUNCTION__, __LINE__);
			break;
		}
		else if (ret == 0)
		{
			OS_PRINTF("[Fun:%s, Line:%d]: filedownload_read_response read 0 (i.e. EOF).\n",
					__FUNCTION__, __LINE__);
			finished = 1;
		}
		OS_PRINTF("[Fun:%s, Line:%d]: recv len this time is = %d\n", __FUNCTION__, __LINE__, ret);
		recvlen += ret;
		if (recvlen < BUFFER_SIZE)
		{
			bufferlen = BUFFER_SIZE - recvlen;
		}
		if ((recvlen == BUFFER_SIZE) || (finished == 1))
		{
			if (totaldatalen == 0)
			{
				ret = filedownload_response_parse(buffer_recv, p_http);
				if (p_http->status_code != 200)
				{
					OS_PRINTF("[Fun:%s, Line:%d]: p_http->status_code[%d]\n",
							__FUNCTION__, __LINE__, p_http->status_code);
					goto err_out;
				}
				buffer_tmp = strstr(buffer_recv, "\r\n\r\n");
				if (buffer_tmp == NULL)
				{
					//buffer_tmp = buffer_recv;
					OS_PRINTF("[Fun:%s, Line:%d]: data return from server without header msg\n",
							__FUNCTION__, __LINE__);
					goto err_out;
				}
				else
				{
					buffer_tmp = buffer_tmp + 4;
				}
				datalen = recvlen - (buffer_tmp - buffer_recv);

			}
			else
			{
				buffer_tmp = buffer_recv;
				datalen = recvlen;
			}
			OS_PRINTF("[Fun:%s, Line:%d]: write do file [%d]\n", __FUNCTION__, __LINE__, datalen);

#ifdef __LINUX__
			if (write(fd, buffer_tmp, datalen) != datalen)
			{
				OS_PRINTF("[Fun:%s, Line:%d]: save file error\n",
						__FUNCTION__, __LINE__);
				break;
			}
#else
			ret = ufs_write(fd, buffer_tmp, datalen, (u32 *)&write_len);
			if ((FR_OK != ret) || (datalen != write_len))
			{
				OS_PRINTF("[Fun:%s, Line:%d]: save file error\n", __FUNCTION__, __LINE__);
				free33(buffer_recv);
				break;
			}
#endif

			totaldatalen += datalen;
			bufferlen = BUFFER_SIZE;
			if (finished == 1)
			{
				break;
			}
		}
	}
	if (totaldatalen == p_http->content_length)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: file download success\n", __FUNCTION__, __LINE__);
	}
	else
	{
		OS_PRINTF("[Fun:%s, Line:%d]: file download failed\n", __FUNCTION__, __LINE__);
	}

	if (buffer_recv != NULL)
	{
		free33(buffer_recv);
		buffer_recv = NULL;
	}
#ifdef __LINUX__
	close(fd);
#else
	ufs_close(fd);
#endif

	return 0;

err_out:
#ifdef __LINUX__
	close(fd);
#else
	ufs_close(fd);
#endif

	if (buffer_recv != NULL)
	{
		free33(buffer_recv);
		buffer_recv = NULL;
	}

	return -2;
}
#if 0       // compile warning, ignore it
/*
*
*
*
*/
static int mthttp_filedownload_read_response(http_download_t *p_http)
{
	int ret = -1;


	OS_PRINTF("[Fun:%s, Line:%d]: enter function\n", __FUNCTION__, __LINE__);
	if ( 0 )//p_http->is_chunked)
	{
		ret = mthttp_filedownload_save_data_trunk(p_http);
	}
	else
	{
		ret = mthttp_filedownload_save_data(p_http);
	}

	return ret;
}
#endif
/*
*
*
*
*/
static int filedownload_read_response(http_download_t *p_http)
{
	int ret = -1;
#ifdef __LINUX__
	int fd = -1;
#else
	tchar_t p_filename_uni[PATH_MAX + NAME_MAX] = {0};
	ufs_file_t *f = NULL;
#endif
	char *p_savefilename = NULL;
	char *path_tmp = NULL;
	char *charsplit = NULL;
	char *split = "/";

	OS_PRINTF("[Fun:%s, Line:%d]: enter function\n", __FUNCTION__, __LINE__);
	p_savefilename = malloc33(PATH_MAX + NAME_MAX);
	if (p_savefilename == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: p_savefilename malloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	memset(p_savefilename, 0, PATH_MAX + NAME_MAX);
	if (p_http->save_path != NULL)
	{
		path_tmp = p_http->save_path;
		if (path_tmp[strlen(path_tmp) - 1] != '/')
		{
			charsplit = split;
		}
		sprintf(p_savefilename, "%s%s%s", p_http->save_path, charsplit, p_http->file_name);
	}
	else
	{
		sprintf(p_savefilename, "%s", p_http->file_name);
	}
#ifdef __LINUX__
	fd = open(p_savefilename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: open %s error\n",
				__FUNCTION__, __LINE__, p_savefilename);
		return -1;
	}
	if (p_http->is_chunked)
	{
		ret = filedownload_save_data_trunk(fd, p_http);
	}
	else
	{
		ret = filedownload_save_data(fd, p_http);
	}
#else
	f = malloc33(sizeof(ufs_file_t));
	if (f == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: ufs_file malloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	memset(f, 0, sizeof(ufs_file_t));
	ufs_asc2uni(p_savefilename, p_filename_uni);
	free33(p_savefilename);
	if (ufs_open(f, p_filename_uni, UFS_CREATE_NEW_COVER | UFS_WRITE) != FR_OK)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: open %s error\n", p_savefilename,
				__FUNCTION__, __LINE__);
		free33(f);
		return -1;
	}
	if (p_http->is_chunked)
	{
		ret = filedownload_save_data_trunk(f, p_http);
	}
	else
	{
		ret = filedownload_save_data(f, p_http);
	}
#endif

	return ret;
}
/*
*
*
*
*/
extern const char *mplayer_version;
static int mthttp_filedownload_send_request(URL_t *url, off_t pos, char *method, HTTP_spec_info_t *http_spec_info)
{
	HTTP_header_t *http_hdr = NULL;
	URL_t *server_url = NULL;
	char str[256];
	int fd = -1;
	int ret;
	int proxy = 0;
// Boolean
	struct timeval to;

	OS_PRINTF("=====>enter [%s]\n",__func__);

	  if(url == NULL)
        {
            OS_PRINTF("%s: Error, invalid parameters.\n", __func__);
   	     goto err_out;
        }
	http_hdr = mthttp_http_new_header();if(http_hdr == NULL) goto err_out;
        if(pos == 1)
        {
            http_hdr->http_minor_version = 1;
            pos = 0;
        }

	if (!strcasecmp(url->protocol, "http_proxy"))
	{
		proxy = 1;
		server_url =mthttp_url_new((url->file) + 1);
		if (!server_url)
		{
			OS_PRINTF("Invalid URL '%s' to proxify\n", url->file + 1);
			goto err_out;
		}
		mthttp_http_set_uri(http_hdr, server_url->noauth_url);
	}
	else
	{
		server_url = url;
		mthttp_http_set_uri(http_hdr, server_url->file);
	}

	if (server_url->port && server_url->port != 80)
	{
		snprintf(str, sizeof(str), "Host: %s:%d", server_url->hostname, server_url->port );
	}
	else
	{
		snprintf(str, sizeof(str), "Host: %s", server_url->hostname );
	}
	OS_PRINTF("server_url->hostname[%s]\n", server_url->hostname);
	mthttp_http_set_field(http_hdr, str);
	if(http_spec_info != NULL)
	{
        	if(http_spec_info->cookie_len >0)
        	{
        	    snprintf (str, sizeof(str), "Cookie: %s", http_spec_info->p_cookie);
        	    mthttp_http_set_field(http_hdr, str);
        	}

        	if(http_spec_info->content_type_len)
        	{
        	   snprintf (str, sizeof(str), "Content-type: %s", http_spec_info->p_content_type);
        	    mthttp_http_set_field(http_hdr, str);
        	}
        	if(http_spec_info->x_request_len)
        	{
        	    snprintf (str, sizeof(str), "X-Requested-With: %s", http_spec_info->p_x_request);
        	    mthttp_http_set_field(http_hdr, str);
        	}
        	// Add the Content Length
        	if(http_spec_info->post_value_len != 0)
        	{
        	    snprintf(str, sizeof(str), "Accept: */*");
        	    mthttp_http_set_field(http_hdr, str);
        	}
        	if(http_spec_info->post_value_len != 0)
        	{
        	    snprintf (str, sizeof(str), "Content-Length: %d", http_spec_info->post_value_len);
        	    mthttp_http_set_field(http_hdr, str);
        	}
	}
       if(http_spec_info != NULL && http_spec_info->user_agent_len > 0)
       {
           snprintf(str, sizeof(str), "User-Agent: %s", http_spec_info->p_user_agent);
       }
	else if (network_useragent)
	{
		snprintf(str, sizeof(str), "User-Agent: %s", network_useragent);
	}
	else
	{
		snprintf(str, sizeof(str), "User-Agent: %s", mplayer_version);
	}
	mthttp_http_set_field(http_hdr, str);

	if (network_referrer)
	{
		char *referrer = NULL;
		size_t len = strlen(network_referrer) + 10;

		// Check len to ensure we don't do something really bad in case of an overflow
		if (len > 10)
		{
#ifndef __LINUX__
			referrer = mthttp_malloc33(len);
#else
			referrer = malloc33(len);
#endif
		}

		if (referrer == NULL)
		{
			OS_PRINTF("[Fun:%s Line:%d]:Memory allocation failed\n", __FUNCTION__, __LINE__);
		}
		else
		{
			snprintf(referrer, len, "Referer: %s", network_referrer);
			mthttp_http_set_field(http_hdr, referrer);
#ifndef __LINUX__
			mthttp_free33(referrer);
#else
			free33(referrer);
#endif
		}
	}

	if (strcasecmp(url->protocol, "noicyx"))
	{
		mthttp_http_set_field(http_hdr, "Icy-MetaData: 1");
	}

	if (pos > 0)
	{
		// Extend http_send_request with possibility to do partial content retrieval
		snprintf(str, sizeof(str), "Range: bytes=%"PRId64"-", (int64_t)pos);
		mthttp_http_set_field(http_hdr, str);
	}

	//if (network_cookies_enabled)
	//	cookies_set(http_hdr, server_url->hostname, server_url->url);

	if (network_http_header_fields)
	{
		int i=0;
		while (network_http_header_fields[i])
		{
			mthttp_http_set_field(http_hdr, network_http_header_fields[i++]);
		}
	}

	mthttp_http_set_field( http_hdr, "Connection: close");
        if(pos == 1)
            http_hdr->http_minor_version = 1; //HTTP1.1

	if (proxy)
	{
		mthttp_http_add_basic_proxy_authentication(http_hdr, url->username, url->password);
	}
	mthttp_http_add_basic_authentication(http_hdr, server_url->username, server_url->password);
	mthttp_http_set_method(http_hdr, method);
	if (mthttp_http_build_request(http_hdr, http_spec_info) == NULL)
	{
		OS_PRINTF("build request failed\n");
		goto err_out;
	}

	if (proxy)
	{
		if (url->port == 0)
		{
			url->port = HTTP_REQUEST_PORT2;           // Default port for the proxy server
		}
		fd = mt_connect2Server( url->hostname, url->port, 1);
		mthttp_url_free33(server_url);
		server_url = NULL;
	}
	else
	{
		if (server_url->port == 0)
		{
			server_url->port = HTTP_REQUEST_PORT1;  // Default port for the web server
		}
		OS_PRINTF("server_url->hostname[%s]\n", server_url->hostname);
		OS_PRINTF("server_url->port[%d]\n", server_url->port);
		fd = mt_connect2Server(server_url->hostname, server_url->port, 1);
	}
	if (fd < 0)
	{
		OS_PRINTF("connect to server failed\n");
		goto err_out;
	}
	if(http_spec_info != NULL && http_spec_info->timeout>0)
	{
	       to.tv_sec = http_spec_info->timeout;
	}
	else
	{
	       to.tv_sec = 5000;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
	//OS_PRINTF("Request: [%s]\n", http_hdr->buffer);

	ret = send(fd, http_hdr->buffer, http_hdr->buffer_size, DEFAULT_SEND_FLAGS);
	if (ret!=(int)http_hdr->buffer_size)
	{
		OS_PRINTF("Error while sending HTTP request: Didn't send all the request.\n");
		goto err_out;
	}
	mthttp_http_free33(http_hdr);

	return fd;

err_out:
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (fd > 0)
#else
		if (fd >= 0)
#endif
		{
			closesocket(fd);
		}
	mthttp_http_free33(http_hdr);
	if (proxy && server_url)
	{
		mthttp_url_free33(server_url);
	}

	return -1;
}
/*
*
*
*
*/
static int filedownload_send_request(URL_t *url, off_t pos, char *method)
{
	HTTP_header_t *http_hdr;
	URL_t *server_url = NULL;
	char str[256];
	int fd = -1;
	int ret;
	int proxy = 0;		// Boolean

	OS_PRINTF("=====>enter [%s]\n",__func__);

	http_hdr = http_new_header(); if(http_hdr == NULL) goto err_out;

	if (!strcasecmp(url->protocol, "http_proxy"))
	{
		proxy = 1;
		server_url = url_new((url->file) + 1);
		if (!server_url)
		{
			OS_PRINTF("Invalid URL '%s' to proxify\n", url->file + 1);
			goto err_out;
		}
		http_set_uri(http_hdr, server_url->noauth_url);
	}
	else
	{
		server_url = url;
		http_set_uri(http_hdr, server_url->file);
	}

	if (server_url->port && server_url->port != 80)
	{
		snprintf(str, sizeof(str), "Host: %s:%d", server_url->hostname, server_url->port );
	}
	else
	{
		snprintf(str, sizeof(str), "Host: %s", server_url->hostname );
	}
	OS_PRINTF("server_url->hostname[%s]\n", server_url->hostname);
	http_set_field(http_hdr, str);

	if (network_useragent)
	{
		snprintf(str, sizeof(str), "User-Agent: %s", network_useragent);
	}
	else
	{
		snprintf(str, sizeof(str), "User-Agent: %s", mplayer_version);
	}
	http_set_field(http_hdr, str);

	if (network_referrer)
	{
		char *referrer = NULL;
		size_t len = strlen(network_referrer) + 10;

		// Check len to ensure we don't do something really bad in case of an overflow
		if (len > 10)
		{
			referrer = malloc33(len);
		}

		if (referrer == NULL)
		{
			OS_PRINTF("[Fun:%s Line:%d]:Memory allocation failed\n", __FUNCTION__, __LINE__);
		}
		else
		{
			snprintf(referrer, len, "Referer: %s", network_referrer);
			http_set_field(http_hdr, referrer);
			free33(referrer);
		}
	}

	if (strcasecmp(url->protocol, "noicyx"))
	{
		http_set_field(http_hdr, "Icy-MetaData: 1");
	}

	if (pos > 0)
	{
		// Extend http_send_request with possibility to do partial content retrieval
		snprintf(str, sizeof(str), "Range: bytes=%"PRId64"-", (int64_t)pos);
		http_set_field(http_hdr, str);
	}

#ifdef __LINUX__
	if (network_cookies_enabled)
	{
		cookies_set( http_hdr, server_url->hostname, server_url->url);
	}
#endif

	if (network_http_header_fields)
	{
		int i=0;
		while (network_http_header_fields[i])
		{
			http_set_field(http_hdr, network_http_header_fields[i++]);
		}
	}

	http_set_field( http_hdr, "Connection: close");

	if (proxy)
	{
		http_add_basic_proxy_authentication(http_hdr, url->username, url->password);
	}
	http_add_basic_authentication(http_hdr, server_url->username, server_url->password);
	http_set_method(http_hdr, method);
	if (http_build_request(http_hdr) == NULL)
	{
		OS_PRINTF("build request failed\n");
		goto err_out;
	}

	if (proxy)
	{
		if (url->port == 0)
		{
			url->port = 8080;           // Default port for the proxy server
		}
		fd = mt_connect2Server( url->hostname, url->port, 1);
		url_free33(server_url);
		server_url = NULL;
	}
	else
	{
		if (server_url->port == 0)
		{
			server_url->port = 80;  // Default port for the web server
		}
		OS_PRINTF("server_url->hostname[%s]\n", server_url->hostname);
		OS_PRINTF("server_url->port[%d]\n", server_url->port);
		fd = mt_connect2Server(server_url->hostname, server_url->port, 1);
	}
	if (fd < 0)
	{
		OS_PRINTF("connect to server failed\n");
		goto err_out;
	}
	OS_PRINTF("Request: [%s]\n", http_hdr->buffer);

	ret = send(fd, http_hdr->buffer, http_hdr->buffer_size, DEFAULT_SEND_FLAGS);
	if (ret!=(int)http_hdr->buffer_size)
	{
		OS_PRINTF("Error while sending HTTP request: Didn't send all the request.\n");
		goto err_out;
	}
	http_free33(http_hdr);

	return fd;

err_out:
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (fd > 0)
#else
		if (fd >= 0)
#endif
		{
			closesocket(fd);
		}
	http_free33(http_hdr);
	if (proxy && server_url)
	{
		url_free33(server_url);
	}

	return -1;
}
#if 0       // compile warning, ignore it
/*
*
*
*
*/
static int mthttp_filedownload_save_file(http_download_t *http_download)
{
	char *next_url=NULL;
	URL_t *rd_url=NULL;
	int ret;

	OS_PRINTF("=====>enter filedownload_save_file\n");
	if (http_download == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: http_download is NULL\n", __FUNCTION__, __LINE__);
        return -1;  // for tsscan
	}

	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (http_download->fd  > 0)
#else
		if (http_download->fd  >= 0)
#endif
		{
			closesocket(http_download->fd);
		}
	http_download->fd = mthttp_filedownload_send_request(http_download->url, 0, "GET", NULL);
	if (http_download->fd < 0)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: send request failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = mthttp_filedownload_read_response(http_download);
	if (ret == -2)
	{
		switch(http_download->status_code)
		{
			// Redirect
			case 301: // Permanently
			case 302: // Temporarily
			case 303: // See Other
			case 307: // Temporarily (since HTTP/1.1)
				next_url = http_download->location;
				if (next_url != NULL)
				{
					rd_url = mthttp_url_new(next_url);
				}
				if ((next_url != NULL) && (rd_url != NULL))
				{
					OS_PRINTF("[Fun:%s, Line:%d]: Redirected: Using this url instead %s\n",
							__FUNCTION__, __LINE__, next_url);
					mthttp_url_free33(http_download->url);
					http_download->url = mthttp_check4proxies(rd_url);
					mthttp_url_free33(rd_url);
					ret = mthttp_filedownload_save_file(http_download); //recursively get save started
				}
				else
				{
					OS_PRINTF("[Fun:%s, Line:%d]: Redirection failed\n", __FUNCTION__, __LINE__);
					closesocket(http_download->fd);
					http_download->fd = -1;
				}
				return -1;
				break;
			case 401: //Authorization required
			case 403: //Forbidden
			case 404: //Not found
			case 500: //Server Error
			default:
				OS_PRINTF("Server returned code %d\n", http_download->status_code);
				closesocket(http_download->fd);
				http_download->fd = -1;
				return -1;
				break;
		}
	}

	return 0;
}
#endif
/*
*
*
*
*/
static int filedownload_save_file(http_download_t *http_download)
{
	char *next_url=NULL;
	URL_t *rd_url=NULL;
	int ret;

	OS_PRINTF("=====>enter filedownload_save_file\n");
	if (http_download == NULL)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: http_download is NULL\n", __FUNCTION__, __LINE__);
        return -1;  // for tsscan
	}
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (http_download->fd  > 0)
#else
		if (http_download->fd  >= 0)
#endif
		{
			closesocket(http_download->fd);
		}
	http_download->fd = filedownload_send_request(http_download->url, 0, "GET");
	if (http_download->fd < 0)
	{
		OS_PRINTF("[Fun:%s, Line:%d]: send request failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = filedownload_read_response(http_download);
	if (ret == -2)
	{
		switch(http_download->status_code)
		{
			// Redirect
			case 301: // Permanently
			case 302: // Temporarily
			case 303: // See Other
			case 307: // Temporarily (since HTTP/1.1)
				next_url = http_download->location;
				if (next_url != NULL)
				{
					rd_url = url_new(next_url);
				}
				if ((next_url != NULL) && (rd_url != NULL))
				{
					OS_PRINTF("[Fun:%s, Line:%d]: Redirected: Using this url instead %s\n",
							__FUNCTION__, __LINE__, next_url);
					url_free33(http_download->url);
					http_download->url = check4proxies(rd_url);
					url_free33(rd_url);
					ret = filedownload_save_file(http_download); //recursively get save started
				}
				else
				{
					OS_PRINTF("[Fun:%s, Line:%d]: Redirection failed\n", __FUNCTION__, __LINE__);
					closesocket(http_download->fd);
					http_download->fd = -1;
				}
				return -1;
				break;
			case 401: //Authorization required
			case 403: //Forbidden
			case 404: //Not found
			case 500: //Server Error
			default:
				OS_PRINTF("Server returned code %d\n", http_download->status_code);
				closesocket(http_download->fd);
				http_download->fd = -1;
				return -1;
				break;
		}
	}

	return 0;
}
#ifndef DOWNLOAD_HTTP_NEW
/*
*
*
*
*/
void chunkhttp_close(int fd)
{
    OS_PRINTF("[%s][%d], get in!!\n",__func__,__LINE__);
    lastChunkLen = 0;
    leftChunkLen = 0;

    closesocket(fd);
}
#endif
/*
*
*
*
*/
int chunkhttp_parse(int fd, char *buffer, unsigned int len)
{
        int readLen = 0;
//        unsigned char ret = 0;
        unsigned char tries = 0;
        unsigned int count = 0;
        unsigned int chunkLen = 0;
        //static unsigned int leftChunkLen = 0;
        //static unsigned int lastChunkLen = 0;

        char *chunkBody = buffer;
        char chunkHeader[MT_HTTP_READLINE_BUFSIZE];

        if (leftChunkLen <= len) {
                if (lastChunkLen == 0) {
                        /* Read chunk header */
        	        readLen = chunk_socket_readline(fd, chunkHeader, MT_HTTP_READLINE_BUFSIZE);
        	        if (readLen == -1) return -1;
					#ifndef __ANDRIOD__
					  	   chunkLen = mt_strhex2long(chunkHeader);
					#else
				chunkLen =	strtol(chunkHeader, NULL, 16);
					#endif
        	        OS_PRINTF("---DRV, 001 ChunkHeader, chunkLen=%u, readLen=%d\n", chunkLen, readLen);
        	        OS_PRINTF("---content: 0x%s", chunkHeader);
        	        if (chunkLen < 1) return 0;
                }
                else {
                        chunkLen = leftChunkLen;
                        //lastChunkLen = 0;
                }
        }
        else
                chunkLen = leftChunkLen;

        //OS_PRINTF("---DRV, 002 ChunkHeader, chunkLen=%u, len=%u\n", chunkLen, len);
        if (chunkLen > len) {
                leftChunkLen = chunkLen - len;

                count = 0;
                while (count < len && tries < MT_HTTP_SOCKREAD_RETRY)
                {
                        readLen = recv(fd, chunkBody+count, len-count, 0);
                        if (readLen < 0) return readLen;
                        else if (readLen == 0) tries++;
                        count += readLen;
                }
                lastChunkLen++;
                //OS_PRINTF("---DRV, 003 ChunkBody, count=%u, readLen=%d, lastChunkLen=%u\n", \
                                    //count, readLen, lastChunkLen);
        }
        else {
                count = 0;
	        /* Read chunkBody until chunkLen is reached, or tired of trying */
	        while (count < chunkLen && tries < MT_HTTP_SOCKREAD_RETRY)
	        {
	                readLen = recv(fd, chunkBody+count, chunkLen-count, 0);
	                if (readLen < 0) return readLen;
	                else if (readLen == 0) tries++;
		        count += readLen;
		       /// OS_PRINTF("---DRV, 003 For while, try=%d, count=%d, readLen=%d\n", tries, count, readLen);
	        }
	        if (count == chunkLen) {
                        /* Read CRLF bytes */
                        /*ret =*/ chunk_socket_readline(fd, chunkHeader, MT_HTTP_READLINE_BUFSIZE);
                        lastChunkLen = 0;
                        leftChunkLen = 0;
	        }
	        //OS_PRINTF("---DRV, 004 count=%u, readLen=%d, ret=%d\n", \
                                    //count, readLen, ret);
	}
	return count;
}
#ifndef DOWNLOAD_HTTP_NEW
/*
*
*
*
*/
int  chunkhttp_recv(int fd, char *response, unsigned int size, unsigned int isChunked)
{
        int /*i,*/ ret;
	int optlen;
	int readLen = 0;
	unsigned char tries = 0;
	unsigned int count = 0;
	char *content = response; //response packet have "Content-Length"
	char *chunkBuf = response; //response packet have "Chunked"
    //    OS_PRINTF("---DRV_BODY, chunkhttp_recv,msize=%u, isChunked=%d\n",
    //                            size, isChunked);
        /* Http packet is chunked */
	if (isChunked == 1) {
	        chunkBuf[0] = '\0';
	        count = 0;
	        do {
	                readLen = chunkhttp_parse(fd, chunkBuf+count, size-count);
	                if (readLen < 1) break;
	                count += readLen;
	        } while(count < size);
	        chunkBuf[count] = '\0';
	        //OS_PRINTF("---DRV_BODY, YES Chunk!LastReadLen=%d, count=%u\n", readLen, count);
	}
	else {
	        content[0] = '\0';
	        count = 0;
	        /* Read content until size is reached, or tired of trying */
	        while (count < size && tries < MT_HTTP_SOCKREAD_RETRY)
	        {
	                readLen = recv(fd, content+count, size-count, 0);
	                if (readLen < 0) break;
	                /* Fixed to increment the counter only when recv() doesn't read data */
		        if (readLen == 0)
		                tries++;
		        count += readLen;
	        }
    	        content[count] = '\0';
    	       // OS_PRINTF("\n---DRV_BODY, NO Chunk! LastReadLen=%d, \033[32count=%u\033[0m will return to MDL\n",
    	                                //readLen, count);
	}

	if (readLen < 0) {
	        mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ReadFailed);
	        return MTHTTP_UNKNOWN_ERROR;
	}
	else if (readLen == 0) {
	        //if (isChunked && (count != 0))
	                //return count;
	        optlen = sizeof(ret);
                getsockopt (fd, SOL_SOCKET, SO_ERROR, &ret, &optlen);
                //OS_PRINTF("---DRV_BODY, getsockopt=%d\n", ret);
                if ((ret != 0) && (ret !=108))
                        return MTHTTP_UNKNOWN_ERROR;

                mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_Read0CouldBeEOF);
	}
	return count;
}
#endif
/*
*
*
*
*/
int chunk_socket_readline(int fd, char *buffer, int bufferLen)
{
    int readCnt;
    int readLen;

    readCnt = 0;
    while (readCnt < (bufferLen-1)) {
	readLen = recv(fd, &buffer[readCnt], sizeof(char), 0);
	if (readLen < 0) return -1;
	else if (readLen == 0) return readCnt;
	readCnt++;
	if (buffer[readCnt-1] == '\n')
		break;
    }
    buffer[readCnt] = '\0';

    return readCnt;
}
/*
*
*
*
*/
int chunk_parse_statusline(char *buffer, HTTP_header_t *http_hdr)
{
    unsigned char len;
    char *p_pos;
    char *p_crlf;
    char *statusline = buffer;

    if (http_hdr->is_parsed) return 0;

    /*get protocol*/
    p_pos = strstr(statusline, MT_HTTP_SPACING);
    if (NULL == p_pos) {
        mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. No space separator found.\n");

        return -1;
    }
    len = p_pos - statusline;
    #ifndef __LINUX__
        http_hdr->protocol = mthttp_malloc33(len+1);
    #else
        http_hdr->protocol = malloc33(len+1);
    #endif
    //MT_ASSERT(NULL != http_hdr->protocol);
    if( NULL == http_hdr->protocol ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
    }

    strncpy( http_hdr->protocol, statusline, len );
    http_hdr->protocol[len] = '\0';
    if (!strncasecmp(http_hdr->protocol, "HTTP", 4)) {
	if (sscanf(http_hdr->protocol+5, "1.%d", &(http_hdr->http_minor_version)) !=1) {
	    mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get HTTP minor version.\n");
	    return -1;
	}
    }

    /*get status code*/
    if (sscanf(++p_pos, "%d", &(http_hdr->status_code)) != 1) {
	mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get status code.\n");
	return -1;
    }

    /*get reason phrase*/
    p_pos += (MT_HTTP_STATUS_CODE_LEN + MT_HTTP_SPACING_LEN);
    //MT_ASSERT(NULL != p_pos);
    if( NULL == p_pos ) return -1;

    p_crlf = strstr(p_pos, MT_HTTP_CRLF);
    if (p_crlf == NULL) {
        p_crlf = strstr(p_pos, MT_HTTP_LF);
        if (p_crlf == NULL) {
            mp_msg(MSGT_NETWORK,MSGL_ERR,"Malformed answer. Unable to get the reason phrase.\n");
            return -1;
        }
    }
    len = p_crlf - p_pos;
    #ifndef __LINUX__
        http_hdr->reason_phrase = mthttp_malloc33(len + 1);
    #else
        http_hdr->reason_phrase = malloc33(len + 1);
    #endif
    //MT_ASSERT(NULL != http_hdr->reason_phrase);
    if( NULL == http_hdr->reason_phrase ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
    }
    strncpy(http_hdr->reason_phrase, p_pos, len);
    http_hdr->reason_phrase[len] = '\0';

    return 0;
}

int mthttp_packet_ischunked(char *buffer)
{
    if (NULL != strstr(buffer, MT_HTTP_TRANSFER_ENCODING)) {
        if (NULL != strstr(buffer, MT_HTTP_CHUNKED))
            return 1;
    }
    return 0;
}

int chunkhttp_parse_header(char *buffer, HTTP_header_t *http_hdr)
{
    int ret = 0;
    unsigned int lineLen = 0;
    static unsigned char statusLine_flag = 0;

    char *field = NULL;
    char *p_pos = NULL;
    char *p_head = buffer;

    if (http_hdr->is_parsed) {
        //OS_PRINTF("\n   \033[41misParsed!%s\033[0m", p_head);
        statusLine_flag = 0;
        return 0;
    }

    if (statusLine_flag == 0) {
        statusLine_flag++;
        ret = chunk_parse_statusline(p_head, http_hdr);
        //OS_PRINTF("\n   %s", p_head);
        //OS_PRINTF("\n   ret=%d\n   protocol=%s\n   code=%d\n   reason=%s\n", \
         //                   ret, http_hdr->protocol, http_hdr->status_code, http_hdr->reason_phrase);
    }
    else {
        p_pos = p_head;
        while (*p_pos != '\r' && *p_pos != '\n') p_pos++;
        lineLen = p_pos - p_head;
        #ifndef __LINUX__
            field = mthttp_calloc33(1, lineLen+1);
        #else
            field = calloc33(1, lineLen+1);
        #endif
        //MT_ASSERT(NULL != field);
        if( NULL == field ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return -1;
        }
        strncpy(field, p_head, lineLen);
        field[lineLen] = '\0';
        mthttp_http_set_field(http_hdr, field);

        if (mthttp_packet_ischunked(field) == 1) {
            http_hdr->isChunked = 1;
        }
        OS_PRINTF("isChunked=%d\n", http_hdr->isChunked);
        //OS_PRINTF("   %s", field); //DRVshuai

        #ifndef __LINUX__
            mthttp_free33(field);
        #else
            free33(field);
        #endif
    }

    return ret;
}
/*
*
*
*
*/
HTTP_header_t *  chunkhttp_read_header(int fd)
{
    int readLen, ret;
    char header[MT_HTTP_READLINE_BUFSIZE];

    HTTP_header_t *http_hdr = mthttp_http_new_header();
    if( NULL == http_hdr ) {
		mp_msg(MSGT_NETWORK,MSGL_FATAL,MSGTR_MemAllocFailed);
		return NULL;
    }

    while(1) {
        readLen = chunk_socket_readline(fd, header, MT_HTTP_READLINE_BUFSIZE);
        //OS_PRINTF("---DRV, 001head_readLen=\033[41m%d\033[0m, ", readLen);
        if (readLen == -1) {
            mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_ReadFailed);
            mthttp_http_free33(http_hdr);
            return NULL;
        }
        else if (readLen <= 2)
        {
            http_hdr->is_parsed = 1;
            chunkhttp_parse_header(header, http_hdr);
            mp_msg(MSGT_NETWORK, MSGL_ERR, MSGTR_MPDEMUX_NW_Read0CouldBeEOF);
            //OS_PRINTF("\n---DRV, 002head_readLen = %d\n", readLen);
            break;
        }
        ret = chunkhttp_parse_header(header, http_hdr);
        if (ret == -1) {
            OS_PRINTF("---DRV, ret=-1, error!\n");
            mthttp_http_free33(http_hdr);
            return NULL;
        }
    }
    return http_hdr;
}

int http_direct_download_start(char *p_url)
{
    int fd = -1;
    URL_t *url = NULL;
    if(p_url == NULL )
    {
    	OS_PRINTF("%s: Error, invalid parameters.\n", __func__);
    	return -1;
    }
    url = mthttp_url_new(p_url);

    fd = mthttp_filedownload_send_request(url, 1, "GET", NULL);   //get file size
    if (fd < 0)
    {
        OS_PRINTF("ERROR:send request failed\n");
    }

    mthttp_url_free33(url);
    return fd;
}
#ifndef DOWNLOAD_HTTP_NEW
/*
*
*
*
*/
int chunkhttp_download_start(char *p_url, HTTP_header_t **p_outbuf)
{
    int fd = -1;
    int redirect = 0;
    int auth_retry = 0;
    const char *content_length = NULL;

    URL_t *url = NULL;
    char *next_url;
    http_download_t http_download = {0};
    HTTP_header_t *http_hdr = NULL;
    int ret = MTHTTP_UNKNOWN_ERROR;
    http_download.fd = -1;

    if(p_url == NULL )
    {
    	OS_PRINTF("%s: Error, invalid parameters.\n", __func__);
    	return MTHTTP_PARAM_INVALID;
    }

    url = mthttp_url_new(p_url);

    http_download.url = mthttp_check4proxies(url);
    mthttp_url_free33(url);if(http_download.url == NULL) return MTHTTP_PARAM_INVALID;

    url = http_download.url;
    fd = http_download.fd;

    do{
        redirect = 0;

    #ifdef __LINUX__
    	if (fd  > 0)
    #else
    	if (fd  >= 0)
    #endif
    	{
                closesocket(fd);
    	}

    	fd = mthttp_filedownload_send_request(http_download.url, 1, "GET", NULL);   //get file size
    	if (fd < 0)
    	{
            OS_PRINTF("ERROR:send request failed\n");
            ret = MTHTTP_SOCK_CREATE_FAILED;
	    goto err_out;
    	}

        mthttp_http_free33(http_hdr);
        http_hdr = chunkhttp_read_header(fd);
        if(http_hdr == NULL)
        {
            OS_PRINTF("ERROR read response failed\n");
            ret = MTHTTP_SESSION_TIMEOUT;
            goto err_out;
        }
        switch (http_hdr->status_code){
            case 200: // OK
                content_length = mthttp_http_get_field(http_hdr, "Content-Length");
//                OS_PRINTF("%s %d, status_code=200, content_length = %u\n",__func__,__LINE__,content_length);
                goto out;
            // Redirect
            case 301: // Permanently
            case 302: // Temporarily
            case 303: // See Other
            case 307: // Temporarily (since HTTP/1.1)
                next_url = mthttp_http_get_field(http_hdr, "Location");
                if (next_url != NULL)
                {
                    int is_ultravox = strcasecmp(http_download.url->protocol, "unsv") == 0;
                    http_download.url = mthttp_url_redirect(&url, next_url);
                    if (mthttp_url_is_protocol(url, "mms"))
                    {
                        OS_PRINTF("Unsupported mms!!!\n");
                        ret = MTHTTP_PROTOCOL_UNSUPPORT;
                        goto err_out;
                    }
                    if (!mthttp_url_is_protocol(url, "http"))
                    {
                        OS_PRINTF("Unsupported http %d redirect to %s protocol\n", http_hdr->status_code, url->protocol);
                        ret = MTHTTP_PROTOCOL_UNSUPPORT;
                        goto err_out;
                    }
                    if (is_ultravox)
                    {
                        mthttp_url_set_protocol(url, "unsv");
                    }
                    redirect = 1;
                }
                break;
            case 401: // Authentication required
                if (mthttp_http_authenticate(http_hdr, url, &auth_retry) < 0)
                {
                    OS_PRINTF("Authentication required!!!\n");
                    ret = MTHTTP_AHTHEN_FAILED;
                    goto err_out;
                }
                redirect = 1;
                break;
            default:
                OS_PRINTF("Server returned %d: %s\n", http_hdr->status_code, http_hdr->reason_phrase);
                goto err_out;
        }

    }while(redirect);


out:
       if(content_length)
       {
           http_hdr->content_length = atoll(content_length);
       }
	   else
           http_hdr->content_length = 0;
       OS_PRINTF("---DRV, 200 OK, \033[41mChunk=%d\033[0m, \033[32mcontent_length=%u\033[0m\n",
                            http_hdr->isChunked, http_hdr->content_length);
        *p_outbuf = http_hdr;
        //mthttp_http_free33(http_hdr); /*ap will free the buffer*/
	mthttp_filedownload_free33(&http_download);
	return fd;

err_out:
#ifdef __LINUX__
	if (fd  > 0)
#else
		if (fd  >= 0)
#endif
		{
			closesocket(fd);
			fd = -1;
		}
	mthttp_http_free33(http_hdr);
	mthttp_filedownload_free33(&http_download);
	return ret;

}
#endif
/*
*
*
*
*/
int mthttp_download_start(char *p_url, HTTP_header_t **p_outbuf, unsigned int timeout)
{
	http_download_t http_download = {0};

	int fd = 0;

	int redirect = 0;
//	int seekable = 0;
	int auth_retry = 0;
	int ret = MTHTTP_UNKNOWN_ERROR;


	URL_t *url = NULL;
	HTTP_header_t *http_hdr = NULL;

	const char *content_length = NULL;
//	const char *transfer_encoding;
	char *next_url;
	//yliu add:for lwip default fd = -1
    http_download.fd = -1;

       //sscanf ("%s", cookie);
	OS_PRINTF("====>enter fun:%s\n", __func__);
	if(p_url == NULL || p_outbuf == NULL)
	{
		OS_PRINTF("%s: Error, invalid parameters.\n", __func__);
		return MTHTTP_PARAM_INVALID;
	}

	url = mthttp_url_new(p_url);
	http_download.url = mthttp_check4proxies(url);
	mthttp_url_free33(url);if(http_download.url == NULL) return MTHTTP_PARAM_INVALID;

	url = http_download.url;
#if 0
	OS_PRINTF("====>http_download.url->url[%s]\n", http_download.url->url);
	OS_PRINTF("====>http_download.url->hostname[%s]\n", http_download.url->hostname);
	OS_PRINTF("====>http_download.url->port[%d]\n", http_download.url->port);
	OS_PRINTF("====>http_download.url->protocol[%s]\n", http_download.url->protocol);
	OS_PRINTF("====>http_download.url->noauth_url[%s]\n", http_download.url->noauth_url);
	OS_PRINTF("====>http_download.url->file[%s]\n", http_download.url->file);
	OS_PRINTF("====>http_download.url->username[%s]\n", http_download.url->username);
	OS_PRINTF("====>http_download.url->password[%s]\n", http_download.url->password);
#endif
      //yliu modify
      fd = http_download.fd;
	do
	{
		redirect = 0;


		//yliu add :for  fd = 0
#ifdef __LINUX__
		if (fd  > 0)
#else
			if (fd  >= 0)
#endif
			{
				closesocket(fd);
			}

		fd = mthttp_filedownload_send_request(http_download.url, 0, "GET", NULL);   //get file size
		if (fd < 0)
		{
			OS_PRINTF("ERROR:send request failed\n");
			ret = MTHTTP_SOCK_CREATE_FAILED;
			goto err_out;
		}

		mthttp_http_free33(http_hdr);
		http_hdr = mthttp_http_read_response(fd);
		if (http_hdr == NULL)
		{
			OS_PRINTF("ERROR read response failed\n");
			ret = MTHTTP_SESSION_TIMEOUT;
			goto err_out;
		}

#if 0
		////////////////////////////////////debug////////////////////////////////
		HTTP_field_t *field = NULL;
		int i = 0;
		OS_PRINTF("--- HTTP DEBUG HEADER --- START ---\n");
		OS_PRINTF("protocol:           [%s]\n", http_hdr->protocol);
		OS_PRINTF("http minor version: [%d]\n", http_hdr->http_minor_version);
		OS_PRINTF("uri:                [%s]\n", http_hdr->uri);
		OS_PRINTF("method:             [%s]\n", http_hdr->method);
		OS_PRINTF("status code:        [%d]\n", http_hdr->status_code);
		OS_PRINTF("reason phrase:      [%s]\n", http_hdr->reason_phrase);
		OS_PRINTF("body size:          [%zu]\n", http_hdr->body_size);

		OS_PRINTF("Fields:\n");
		field = http_hdr->first_field;
		while (field != NULL)
		{
			OS_PRINTF(" %d - %s\n", i++, field->field_name);
			field = field->next;
		}
		OS_PRINTF("--- HTTP DEBUG HEADER --- END ---\n");
		/////////////////////////////////////////////////////////////////////////////
#endif

		// Check if we can make partial content requests and thus seek in http-streams
		if ((http_hdr != NULL) && (http_hdr->status_code == 200))
		{
			const char *accept_ranges = mthttp_http_get_field(http_hdr,"Accept-Ranges");
			const char *server = mthttp_http_get_field(http_hdr, "Server");
			if (accept_ranges)
			{
				//seekable = (strncmp(accept_ranges, "bytes", 5) == 0);
			}
			else if (server &&
					(strcmp(server, "gvs 1.0") == 0
					 || strncmp(server, "MakeMKV", 7) == 0))
			{
				// HACK for youtube and MakeMKV incorrectly claiming not to support seeking
				OS_PRINTF("Broken webserver, incorrectly claims to not support Accept-Ranges\n");
				//seekable = 1;
			}
		}

		switch (http_hdr->status_code)
		{
			case 200: // OK

				content_length = mthttp_http_get_field(http_hdr, "Content-Length");
#if 0
				if (content_length)
				{
					http_download.content_length = atoll(content_length);
					OS_PRINTF("http_download.content_length[%d]\n", http_download.content_length);
#ifndef __LINUX__
					http_download.buffer_recv = mthttp_malloc33((int)http_download.content_length);
#else
					http_download.buffer_recv = malloc33((int)http_download.content_length);
#endif
					memset(http_download.buffer_recv, 0, http_download.content_length);
				}
				else
				{
					transfer_encoding = mthttp_http_get_field(http_hdr, "Transfer-Encoding");
					if (transfer_encoding)
					{
						if (strcasecmp(transfer_encoding, "chunked") == 0)
						{
							http_download.is_chunked = 1;
						}
					}
					else
					{
						OS_PRINTF("Response header return from server error\n");
						goto err_out;
					}
				}
#endif

				goto out;
				// Redirect
			case 301: // Permanently
			case 302: // Temporarily
			case 303: // See Other
			case 307: // Temporarily (since HTTP/1.1)
				next_url = mthttp_http_get_field(http_hdr, "Location");
				if (next_url != NULL)
				{
					int is_ultravox = strcasecmp(http_download.url->protocol, "unsv") == 0;
					http_download.url = mthttp_url_redirect(&url, next_url);
					if (mthttp_url_is_protocol(url, "mms"))
					{
						OS_PRINTF("Unsupported mms!!!\n");
						ret = MTHTTP_PROTOCOL_UNSUPPORT;
						goto err_out;
					}
					if (!mthttp_url_is_protocol(url, "http"))
					{
						OS_PRINTF("Unsupported http %d redirect to %s protocol\n", http_hdr->status_code, url->protocol);
						ret = MTHTTP_PROTOCOL_UNSUPPORT;
						goto err_out;
					}
					if (is_ultravox)
					{
						mthttp_url_set_protocol(url, "unsv");
					}
					redirect = 1;
				}
				break;
			case 401: // Authentication required
				if (mthttp_http_authenticate(http_hdr, url, &auth_retry) < 0)
				{
					OS_PRINTF("Authentication required!!!\n");
					ret = MTHTTP_AHTHEN_FAILED;
					goto err_out;
				}
				redirect = 1;
				break;
			default:
				OS_PRINTF("Server returned %d: %s\n", http_hdr->status_code, http_hdr->reason_phrase);
				goto err_out;
		}
	}while (redirect);

out:
               if(content_length)
               {
                    if(atoll(content_length) > http_hdr->body_size)
                    {
                          OS_PRINTF("Error: content len:%d body size:%d buffer size:%d \n", atoll(content_length), http_hdr->body_size, http_hdr->buffer_size);
                          ret = MTHTTP_SESSION_TIMEOUT;
                          goto err_out;
                    }
					http_hdr->content_length = atoll(content_length);
               }

	//ret = mthttp_filedownload_save_file(&http_download);
	//memcpy(p_outbuf, http_hdr->body, http_hdr->body_size);
	*p_outbuf = http_hdr;
	//mthttp_http_free33(http_hdr); //header must be release by caller
	mthttp_filedownload_free33(&http_download);
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (fd  > 0)
#else
		if (fd  >= 0)
#endif
		{
			closesocket(fd);
			fd = -1;
		}
	return 0;

err_out:
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (fd  > 0)
#else
		if (fd  >= 0)
#endif
		{
			closesocket(fd);
			fd = -1;
		}
	mthttp_http_free33(http_hdr);
	mthttp_filedownload_free33(&http_download);
	return ret;
}
/*
*
*
*
*/
void *mthttp_proxy_get(void) {
    return g_httpproxy;
}
#if 0
/*
*
*
*
*/
void *mthttp_proxy_config(HTTP_proxy_t *proxy) {
    HTTP_proxy_t *p_cfg = NULL;

    if (proxy == NULL)  return NULL;

    if (g_httpproxy)
        mthttp_url_free33(g_httpproxy);

    #ifndef __LINUX__
    p_cfg = mthttp_calloc33(1, sizeof(HTTP_proxy_t));   // for tsscan
    #else
//    p_cfg = calloc33(1, sizeof(*HTTP_proxy_t));//peacer del for complication error
    #endif
    if (p_cfg == NULL)  return NULL;

    /*get hostname*/
    if (proxy->hostname) {
        #ifndef __LINUX__
        p_cfg->hostname = mthttp_calloc33(1, strlen(proxy->hostname)+1);
        #else
        p_cfg->hostname = calloc33(1, strlen(proxy->hostname)+1);
        #endif
        if (p_cfg->hostname == NULL)  goto err_out;
        memset(p_cfg->hostname, 0, strlen(p_cfg->hostname)+1);
        strcpy(p_cfg->hostname, proxy->hostname);
    }
    /*get port*/
    p_cfg->port = proxy->port;

    /*get username*/
    if (proxy->username) {
        #ifndef __LINUX__
        p_cfg->username = mthttp_calloc33(1, strlen(proxy->username)+1);
        #else
        p_cfg->username = calloc33(1, strlen(proxy->username)+1);
        #endif
        if (p_cfg->username == NULL)  goto err_out;
        memset(p_cfg->username, 0, strlen(p_cfg->username)+1);
        strcpy(p_cfg->username, proxy->username);
    }
    /*get password*/
    if (proxy->password) {
        #ifndef __LINUX__
        p_cfg->password = mthttp_calloc33(1, strlen(proxy->password)+1);
        #else
        p_cfg->password = calloc33(1, strlen(proxy->password)+1);
        #endif
        if (p_cfg->password == NULL)  goto err_out;
        memset(p_cfg->password, 0, strlen(p_cfg->password)+1);
        strcpy(p_cfg->password, proxy->password);
    }
    g_httpproxy = p_cfg;

    OS_PRINTF("config  hostname=%s\n  port=%d\n  username=%s\n  passwd=%s\n",
                        p_cfg->hostname, p_cfg->port, p_cfg->username, p_cfg->password);
    return p_cfg;

err_out:
    if (p_cfg)  mthttp_url_free33(p_cfg);
    return NULL;
}
#endif
/*
*
*
*
*/
char *mthttp_proxyurl_build(const HTTP_proxy_t *proxy, const char *host_url)
{
    if (proxy->username)
        return mthttp_mp_asprintf("http_proxy://%s:%s@%s:%d/%s",
                           proxy->username,
                           proxy->password ? proxy->password : "",
                           proxy->hostname, proxy->port, host_url);
    else
        return mthttp_mp_asprintf("http_proxy://%s:%d/%s",
                           proxy->hostname, proxy->port, host_url);
}
#ifndef DOWNLOAD_HTTP_NEW
/*
*
*
*
*/
int chunkhttp_download_common_start(char *p_url, HTTP_header_t **p_outbuf,  HTTP_spec_info_t *p_http_spec_info)
{

    int fd = -1;
    int redirect = 0;
    int auth_retry = 0;
    const char *content_length = NULL;

    URL_t *url = NULL;
    HTTP_proxy_t *p_proxy = NULL;
    char *next_url;
    char *proxy_url;
    http_download_t http_download = {0};
    HTTP_header_t *http_hdr = NULL;
    int ret = MTHTTP_UNKNOWN_ERROR;


    http_download.fd = -1;

    if(p_url == NULL )
    {
    	OS_PRINTF("%s: Error, invalid parameters.\n", __func__);
    	return MTHTTP_PARAM_INVALID;
	}
    url = mthttp_url_new(p_url);if(url == NULL) return MTHTTP_PARAM_INVALID;

    OS_PRINTF("[%s][%d], \nurl=[%s]\n",__func__,__LINE__,url->url);
    OS_PRINTF("noauth_url=[%s]\n",url->noauth_url);
    OS_PRINTF("protocol=[%s]\n",url->protocol);
    OS_PRINTF("hostname=[%s]\n",url->hostname);
    OS_PRINTF("file=[%s]\n",url->file);
    OS_PRINTF("port=[%u]\n",url->port);

    if (p_proxy = (HTTP_proxy_t *)mthttp_proxy_get())
    {
        proxy_url = mthttp_proxyurl_build(p_proxy, (const char *)url->url);

		#ifndef __LINUX__
        url->url = mthttp_realloc33(url->url, strlen(proxy_url)+1);
		#else
        url->url = realloc33(url->url, strlen(proxy_url)+1);
		#endif
        if (NULL == url->url) return -1;
        memset(url->url, 0, strlen(proxy_url)+1);
        strcpy( url->url, proxy_url );

		#ifndef __LINUX__
        url->protocol = mthttp_realloc33(url->protocol, strlen("http_proxy")+1);
		#else
        url->protocol = realloc33(url->protocol, strlen("http_proxy")+1);
		#endif
        if (NULL == url->protocol) return -1;
        memset(url->protocol, 0, strlen("http_proxy")+1);
        strcpy( url->protocol, "http_proxy");

        /*The buffer has been allocated when call function mthttp_proxyurl_build()*/
        #ifndef __LINUX__
        mthttp_free33(proxy_url);
        #else
        free33(proxy_url);
        #endif
    }
    http_download.url = mthttp_check4proxies(url);
    mthttp_url_free33(url);if(http_download.url == NULL) goto err_out;

    url = http_download.url;
    fd = http_download.fd;

    do{
        redirect = 0;

    #ifdef __LINUX__
    	if (fd  > 0)
    #else
    	if (fd  >= 0)
    #endif
    	{
                closesocket(fd);
    	}
     //differ POST or GET
		if(p_http_spec_info && p_http_spec_info->post_value_len > 0)
		    fd = mthttp_filedownload_send_request(http_download.url, 0, "POST", p_http_spec_info);   //get file size
		else
		    fd = mthttp_filedownload_send_request(http_download.url, 0, "GET", p_http_spec_info);
    	if (fd < 0)
    	{
            OS_PRINTF("ERROR:send request failed\n");
            ret = MTHTTP_SOCK_CREATE_FAILED;
	    goto err_out;
    	}

        mthttp_http_free33(http_hdr);
        http_hdr = chunkhttp_read_header(fd);
        if(http_hdr == NULL)
        {
            OS_PRINTF("ERROR read response failed\n");
            ret = MTHTTP_SESSION_TIMEOUT;
            goto err_out;
        }
        switch (http_hdr->status_code){
            case 200: // OK
                content_length = mthttp_http_get_field(http_hdr, "Content-Length");
                OS_PRINTF("%s %d, status_code=200, content_length = %u\n",__func__,__LINE__,content_length);
                goto out;
            // Redirect
            case 301: // Permanently
            case 302: // Temporarily
            case 303: // See Other
            case 307: // Temporarily (since HTTP/1.1)
                next_url = mthttp_http_get_field(http_hdr, "Location");
                if (next_url != NULL)
                {
                    int is_ultravox = strcasecmp(http_download.url->protocol, "unsv") == 0;
                    http_download.url = mthttp_url_redirect(&url, next_url);
                    if (mthttp_url_is_protocol(url, "mms"))
                    {
                        OS_PRINTF("Unsupported mms!!!\n");
                        ret = MTHTTP_PROTOCOL_UNSUPPORT;
                        goto err_out;
                    }
                    if (!mthttp_url_is_protocol(url, "http"))
                    {
                        OS_PRINTF("Unsupported http %d redirect to %s protocol\n", http_hdr->status_code, url->protocol);
                        ret = MTHTTP_PROTOCOL_UNSUPPORT;
                        goto err_out;
                    }
                    if (is_ultravox)
                    {
                        mthttp_url_set_protocol(url, "unsv");
                    }
                    redirect = 1;
                }
                break;
            case 401: // Authentication required
                if (mthttp_http_authenticate(http_hdr, url, &auth_retry) < 0)
                {
                    OS_PRINTF("Authentication required!!!\n");
                    ret = MTHTTP_AHTHEN_FAILED;
                    goto err_out;
                }
                redirect = 1;
                break;
            default:
                OS_PRINTF("Server returned %d: %s\n", http_hdr->status_code, http_hdr->reason_phrase);
                goto err_out;
        }

    }while(redirect);


out:
       if(content_length)
       {
           http_hdr->content_length = atoll(content_length);
       }
	   else
           http_hdr->content_length = 0;
        *p_outbuf = http_hdr;
        //mthttp_http_free33(http_hdr); /*ap will free the buffer*/
	mthttp_filedownload_free33(&http_download);
	return fd;

err_out:
#ifdef __LINUX__
	if (fd  > 0)
#else
		if (fd  >= 0)
#endif
		{
			closesocket(fd);
			fd = -1;
		}
	mthttp_http_free33(http_hdr);
	mthttp_filedownload_free33(&http_download);
	return ret;

}
#endif
#ifndef DOWNLOAD_HTTP_NEW
/*
*
*
*
*/

int mthttp_download_common_start(char *p_url, HTTP_header_t **p_outbuf, HTTP_spec_info_t *p_http_spec_info)
{
	http_download_t http_download = {0};

	int fd = 0;

	int redirect = 0;
//	int seekable = 0;
	int auth_retry = 0;
	int ret = MTHTTP_UNKNOWN_ERROR;


	URL_t *url = NULL;
	HTTP_header_t *http_hdr = NULL;

	const char *content_length = NULL;
//	const char *transfer_encoding;
	char *next_url;
	//yliu add:for lwip default fd = -1

	http_download.fd = -1;

       //sscanf ("%s", cookie);
	OS_PRINTF("====>enter fun:%s\n", __func__);
	if(p_url == NULL || p_outbuf == NULL)
	{
		OS_PRINTF("%s: Error, invalid parameters.\n", __func__);
		return MTHTTP_PARAM_INVALID;
	}
	url = mthttp_url_new(p_url);
	http_download.url = mthttp_check4proxies(url);
	mthttp_url_free33(url);if(http_download.url == NULL) goto err_out;

	url = http_download.url;
#if 0
	OS_PRINTF("====>http_download.url->url[%s]\n", http_download.url->url);
	OS_PRINTF("====>http_download.url->hostname[%s]\n", http_download.url->hostname);
	OS_PRINTF("====>http_download.url->port[%d]\n", http_download.url->port);
	OS_PRINTF("====>http_download.url->protocol[%s]\n", http_download.url->protocol);
	OS_PRINTF("====>http_download.url->noauth_url[%s]\n", http_download.url->noauth_url);
	OS_PRINTF("====>http_download.url->file[%s]\n", http_download.url->file);
	OS_PRINTF("====>http_download.url->username[%s]\n", http_download.url->username);
	OS_PRINTF("====>http_download.url->password[%s]\n", http_download.url->password);
#endif
      //yliu modify
      fd = http_download.fd;
	do
	{
		redirect = 0;


		//yliu add :for  fd = 0
#ifdef __LINUX__
		if (fd  > 0)
#else
			if (fd  >= 0)
#endif
			{
				closesocket(fd);
			}
              //differ POST or GET
        if(p_http_spec_info && p_http_spec_info->post_value_len > 0)
		    fd = mthttp_filedownload_send_request(http_download.url, 0, "POST", p_http_spec_info);   //get file size
		else
		    fd = mthttp_filedownload_send_request(http_download.url, 0, "GET", p_http_spec_info);
		if (fd < 0)
		{
			OS_PRINTF("ERROR:send request failed\n");
			ret = MTHTTP_SOCK_CREATE_FAILED;
			goto err_out;
		}
		mthttp_http_free33(http_hdr);
		http_hdr = mthttp_http_read_response(fd);
		if (http_hdr == NULL)
		{
			OS_PRINTF("ERROR read response failed\n");
			ret = MTHTTP_SESSION_TIMEOUT;
			goto err_out;
		}


#if 0
		////////////////////////////////////debug////////////////////////////////
		HTTP_field_t *field = NULL;
		int i = 0;
		OS_PRINTF("--- HTTP DEBUG HEADER --- START ---\n");
		OS_PRINTF("protocol:           [%s]\n", http_hdr->protocol);
		OS_PRINTF("http minor version: [%d]\n", http_hdr->http_minor_version);
		OS_PRINTF("uri:                [%s]\n", http_hdr->uri);
		OS_PRINTF("method:             [%s]\n", http_hdr->method);
		OS_PRINTF("status code:        [%d]\n", http_hdr->status_code);
		OS_PRINTF("reason phrase:      [%s]\n", http_hdr->reason_phrase);
		OS_PRINTF("body size:          [%zu]\n", http_hdr->body_size);

		OS_PRINTF("Fields:\n");
		field = http_hdr->first_field;
		while (field != NULL)
		{
			OS_PRINTF(" %d - %s\n", i++, field->field_name);
			field = field->next;
		}
		OS_PRINTF("--- HTTP DEBUG HEADER --- END ---\n");
		/////////////////////////////////////////////////////////////////////////////
#endif

		// Check if we can make partial content requests and thus seek in http-streams
		if ((http_hdr != NULL) && (http_hdr->status_code == 200))
		{
			const char *accept_ranges = mthttp_http_get_field(http_hdr,"Accept-Ranges");
			const char *server = mthttp_http_get_field(http_hdr, "Server");
			if (accept_ranges)
			{
//				seekable = (strncmp(accept_ranges, "bytes", 5) == 0);
			}
			else if (server &&
					(strcmp(server, "gvs 1.0") == 0
					 || strncmp(server, "MakeMKV", 7) == 0))
			{
				// HACK for youtube and MakeMKV incorrectly claiming not to support seeking
				OS_PRINTF("Broken webserver, incorrectly claims to not support Accept-Ranges\n");
//				seekable = 1;
			}
		}


		switch (http_hdr->status_code)
		{
			case 200: // OK

				content_length = mthttp_http_get_field(http_hdr, "Content-Length");
#if 0
				if (content_length)
				{
					http_download.content_length = atoll(content_length);
					OS_PRINTF("http_download.content_length[%d]\n", http_download.content_length);
#ifndef __LINUX__
					http_download.buffer_recv = mthttp_malloc33((int)http_download.content_length);
#else
					http_download.buffer_recv = malloc33((int)http_download.content_length);
#endif
					memset(http_download.buffer_recv, 0, http_download.content_length);
				}
				else
				{
					transfer_encoding = mthttp_http_get_field(http_hdr, "Transfer-Encoding");
					if (transfer_encoding)
					{
						if (strcasecmp(transfer_encoding, "chunked") == 0)
						{
							http_download.is_chunked = 1;
						}
					}
					else
					{
						OS_PRINTF("Response header return from server error\n");
						goto err_out;
					}
				}
#endif

				goto out;
				// Redirect
			case 301: // Permanently
			case 302: // Temporarily
			case 303: // See Other
			case 307: // Temporarily (since HTTP/1.1)

				next_url = mthttp_http_get_field(http_hdr, "Location");
				if (next_url != NULL)
				{
					int is_ultravox = strcasecmp(http_download.url->protocol, "unsv") == 0;
					http_download.url = mthttp_url_redirect(&url, next_url);
					if (mthttp_url_is_protocol(url, "mms"))
					{
						OS_PRINTF("Unsupported mms!!!\n");
						ret = MTHTTP_PROTOCOL_UNSUPPORT;
						goto err_out;
					}
					if (!mthttp_url_is_protocol(url, "http"))
					{
						OS_PRINTF("Unsupported http %d redirect to %s protocol\n", http_hdr->status_code, url->protocol);
						ret = MTHTTP_PROTOCOL_UNSUPPORT;
						goto err_out;
					}
					if (is_ultravox)
					{
						mthttp_url_set_protocol(url, "unsv");
					}
					redirect = 1;
				}
				break;
			case 401: // Authentication required
				if (mthttp_http_authenticate(http_hdr, url, &auth_retry) < 0)
				{
					OS_PRINTF("Authentication required!!!\n");
					ret = MTHTTP_AHTHEN_FAILED;
					goto err_out;
				}
				redirect = 1;
				break;
			default:
				OS_PRINTF("Server returned %d: %s\n", http_hdr->status_code, http_hdr->reason_phrase);
				goto err_out;
		}
	}while (redirect);

out:
               if(content_length)
               {
                    if(atoll(content_length) > http_hdr->body_size)
                    {
                          OS_PRINTF("Error: content len:%d body size:%d buffer size:%d \n", atoll(content_length), http_hdr->body_size, http_hdr->buffer_size);
                          ret = MTHTTP_SESSION_TIMEOUT;
                          goto err_out;
                    }
					http_hdr->content_length = atoll(content_length);
               }

	//ret = mthttp_filedownload_save_file(&http_download);
	//memcpy(p_outbuf, http_hdr->body, http_hdr->body_size);
	*p_outbuf = http_hdr;
	//mthttp_http_free33(http_hdr); //header must be release by caller
	mthttp_filedownload_free33(&http_download);
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (fd  > 0)
#else
		if (fd  >= 0)
#endif
		{
			closesocket(fd);
			fd = -1;
		}
	return 0;

err_out:
	//yliu add :for  fd = 0
#ifdef __LINUX__
	if (fd  > 0)
#else
		if (fd  >= 0)
#endif
		{
			closesocket(fd);
			fd = -1;
		}
	mthttp_http_free33(http_hdr);
	mthttp_filedownload_free33(&http_download);

	return ret;
}
#endif
/*!
 *
 *return: -1 memory limit
 *       -2 error website or port format
 */
int filedownload_start(char *p_url, char *save_path, char *file_name)
{
	http_download_t http_download = {0};

	int fd = 0;
	int ret = 0;

	int redirect = 0;
//	int seekable = 0;
	int auth_retry = 0;
	int save_path_len = 0;
	int file_name_len = 0;

	URL_t *url = NULL;
	HTTP_header_t *http_hdr = NULL;

	const char *content_length;
	const char *transfer_encoding;
	char *next_url;


#ifndef __LINUX__
	tchar_t *unichar = NULL;
#endif

	OS_PRINTF("====>enter fun:%s\n", __func__);

	url = url_new(p_url);
	http_download.url = check4proxies(url);
	url_free33(url);if(http_download.url == NULL)return -1;

	url = http_download.url;

	OS_PRINTF("====>http_download.url->url[%s]\n", http_download.url->url);
	OS_PRINTF("====>http_download.url->hostname[%s]\n", http_download.url->hostname);
	OS_PRINTF("====>http_download.url->port[%d]\n", http_download.url->port);
	OS_PRINTF("====>http_download.url->protocol[%s]\n", http_download.url->protocol);
	OS_PRINTF("====>http_download.url->noauth_url[%s]\n", http_download.url->noauth_url);
	OS_PRINTF("====>http_download.url->file[%s]\n", http_download.url->file);
	OS_PRINTF("====>http_download.url->username[%s]\n", http_download.url->username);
	OS_PRINTF("====>http_download.url->password[%s]\n", http_download.url->password);

	if (save_path != NULL)
	{
		save_path_len = strlen(save_path);
		http_download.save_path = malloc33(save_path_len + 1);
		if (http_download.save_path == NULL)
		{
			OS_PRINTF("http_download.save_path malloc failed\n");
			return -1;
		}
		memset(http_download.save_path, 0, save_path_len + 1);
		strncpy(http_download.save_path, save_path, save_path_len);
		http_download.save_path[save_path_len + 1] = '\0';
		OS_PRINTF("======>http_download.save_path[%s]\n", http_download.save_path);
	}
	http_download.file_name = malloc33(NAME_MAX);
	if (http_download.file_name == NULL)
	{
		OS_PRINTF("http_download.file_name malloc failed\n");
		return -1;
	}
	if (file_name != NULL)
	{
		file_name_len = strlen(file_name);
		strncpy(http_download.file_name, file_name, file_name_len);
		http_download.file_name[file_name_len] = '\0';
		OS_PRINTF("======>http_download.file_name[%s]\n", http_download.file_name);
	}

	do
	{
		redirect = 0;

		fd = http_download.fd;
		//yliu add :for  fd = 0
#ifdef __LINUX__
		if (fd > 0)
#else
			if (fd >= 0)
#endif
			{
				closesocket(fd);
			}

		fd = filedownload_send_request(http_download.url, 0, "HEAD");   //get file size
		if (fd < 0)
		{
			OS_PRINTF("ERROR:send request failed\n");
			goto err_out;
		}

		http_free33(http_hdr);
		http_hdr = http_read_response(fd);
		if (http_hdr == NULL)
		{
			closesocket(fd);
       		fd = -1;
			OS_PRINTF("ERROR read response failed\n");
			goto err_out;
		}

#if 1
		////////////////////////////////////debug////////////////////////////////
		HTTP_field_t *field = NULL;
		int i = 0;
		OS_PRINTF("--- HTTP DEBUG HEADER --- START ---\n");
		OS_PRINTF("protocol:           [%s]\n", http_hdr->protocol);
		OS_PRINTF("http minor version: [%d]\n", http_hdr->http_minor_version);
		OS_PRINTF("uri:                [%s]\n", http_hdr->uri);
		OS_PRINTF("method:             [%s]\n", http_hdr->method);
		OS_PRINTF("status code:        [%d]\n", http_hdr->status_code);
		OS_PRINTF("reason phrase:      [%s]\n", http_hdr->reason_phrase);
		OS_PRINTF("body size:          [%zu]\n", http_hdr->body_size);

		OS_PRINTF("Fields:\n");
		field = http_hdr->first_field;
		while (field != NULL)
		{
			OS_PRINTF(" %d - %s\n", i++, field->field_name);
			field = field->next;
		}
		OS_PRINTF("--- HTTP DEBUG HEADER --- END ---\n");
		/////////////////////////////////////////////////////////////////////////////
#endif

		// Check if we can make partial content requests and thus seek in http-streams
		if ((http_hdr != NULL) && (http_hdr->status_code == 200))
		{
			const char *accept_ranges = http_get_field(http_hdr,"Accept-Ranges");
			const char *server = http_get_field(http_hdr, "Server");
			if (accept_ranges)
			{
//				seekable = (strncmp(accept_ranges, "bytes", 5) == 0);
			}
			else if (server &&
					(strcmp(server, "gvs 1.0") == 0
					 || strncmp(server, "MakeMKV", 7) == 0))
			{
				// HACK for youtube and MakeMKV incorrectly claiming not to support seeking
				OS_PRINTF("Broken webserver, incorrectly claims to not support Accept-Ranges\n");
//				seekable = 1;
			}
		}

		switch (http_hdr->status_code)
		{
			case 200: // OK
				content_length = http_get_field(http_hdr, "Content-Length");
				if (content_length)
				{
					//OS_PRINTF("Content-Length: [%s]\n", content_length);
					http_download.content_length = atoll(content_length);
					//OS_PRINTF("http_download.content_length[%d]\n", http_download.content_length);
				}
				else
				{
					transfer_encoding = http_get_field(http_hdr, "Transfer-Encoding");
					if (transfer_encoding)
					{
						if (strcasecmp(transfer_encoding, "chunked") == 0)
						{
							http_download.is_chunked = 1;
						}
					}
					else
					{
						OS_PRINTF("Response header return from server error\n");
						goto err_out;
					}
				}

				if (file_name == NULL)
				{
					memset(http_download.file_name, 0, NAME_MAX);
					filedownload_get_filename(http_hdr, http_download.url->url, http_download.file_name);
				}

				goto out;
				// Redirect
			case 301: // Permanently
			case 302: // Temporarily
			case 303: // See Other
			case 307: // Temporarily (since HTTP/1.1)
				next_url = http_get_field(http_hdr, "Location");
				if (next_url != NULL)
				{
					int is_ultravox = strcasecmp(http_download.url->protocol, "unsv") == 0;
					http_download.url = url_redirect(&url, next_url);
					if (url_is_protocol(url, "mms"))
					{
						OS_PRINTF("Unsupported mms!!!\n");
						goto err_out;
					}
					if (!url_is_protocol(url, "http"))
					{
						OS_PRINTF("Unsupported http %d redirect to %s protocol\n", http_hdr->status_code, url->protocol);
						goto err_out;
					}
					if (is_ultravox)
					{
						url_set_protocol(url, "unsv");
					}
					redirect = 1;
				}
				break;
			case 401: // Authentication required
				if (http_authenticate(http_hdr, url, &auth_retry) < 0)
				{
					OS_PRINTF("Authentication required!!!\n");
					goto err_out;
				}
				redirect = 1;
				break;
			default:
				OS_PRINTF("Server returned %d: %s\n", http_hdr->status_code, http_hdr->reason_phrase);
				goto err_out;
		}
	}while (redirect);

out:
	http_free33(http_hdr);
	ret = filedownload_save_file(&http_download);
	filedownload_free33(&http_download);
	return ret;

err_out:
	http_free33(http_hdr);
	filedownload_free33(&http_download);
	return ret;
}

//end add for download file
///////////////////////////////////////////////////////////////////////////////////
/*
 *
 *
 *
 *
 */
static int fixup_open(stream_t *stream,int seekable) {

	OS_PRINTF("[%s] start start ...\n",__func__);

	HTTP_header_t *http_hdr = stream->streaming_ctrl->data;
	int is_icy = http_hdr && http_get_field(http_hdr, "Icy-MetaInt");
	int is_ultravox = strcasecmp(stream->streaming_ctrl->url->protocol, "unsv") == 0;

	stream->type = STREAMTYPE_STREAM;
	if(!is_icy && !is_ultravox && seekable)
	{
		stream->flags |= MP_STREAM_SEEK;
		stream->seek = http_seek;
	}

	stream->streaming_ctrl->bandwidth = network_bandwidth;

	OS_PRINTF("[%s] is_icy:%d\n",__func__,is_icy);
	OS_PRINTF("[%s] is_ultravox:%d\n",__func__,is_ultravox);

	if ((!is_icy && !is_ultravox) || scast_streaming_start(stream))
	{
		if(nop_streaming_start( stream )) {

			mp_msg(MSGT_NETWORK,MSGL_ERR,"nop_streaming_start failed\n");
			if (stream->fd >= 0)
			{
				closesocket(stream->fd);
			}

			stream->fd = -1;
			streaming_ctrl_free33(stream->streaming_ctrl);
			stream->streaming_ctrl = NULL;
			OS_PRINTF("[%s][ERROR] fail to nop_streaming_start ...\n",__func__);

			return STREAM_UNSUPPORTED;
		}
	}

	OS_PRINTF("[%s] end end ...\n",__func__);

	return STREAM_OK;

}
/*
*
*
*
*/
static int open_s1(stream_t *stream,int mode, void* opts, int* file_format) {

	int seekable=0;
	URL_t *url;

	OS_PRINTF("[%s] start start ...\n",__func__);

	stream->streaming_ctrl = streaming_ctrl_new();
	if( stream->streaming_ctrl==NULL ) {
		return STREAM_ERROR;
	}
	stream->streaming_ctrl->bandwidth = network_bandwidth;

	url = url_new(stream->url);
	stream->streaming_ctrl->url = check4proxies(url);
	url_free33(url);

	OS_PRINTF("[%s] STREAM_HTTP(1), URL: %s\n",__func__, stream->url);

	seekable = http_streaming_start(stream, file_format);

	if((seekable < 0) || (*file_format == DEMUXER_TYPE_ASF)) {

		if (stream->fd >= 0)
		{
			closesocket(stream->fd);
		}

		stream->fd = -1;
		if (seekable == STREAM_REDIRECTED)
		{
			OS_PRINTF("[%s] STREAM_REDIRECTED !!!\n",__func__);
			return seekable;
		}

		streaming_ctrl_free33(stream->streaming_ctrl);
		stream->streaming_ctrl = NULL;
		OS_PRINTF("[%s] [STREAM_UNSUPPORTED]---error 111111111\n",__func__);
		return STREAM_UNSUPPORTED;
	}

	OS_PRINTF("[%s] end end ...\n",__func__);

	return fixup_open(stream, seekable);


}
/*
*
*
*
*/
static int open_s2(stream_t *stream,int mode, void* opts, int* file_format) {
	int seekable=0;
	URL_t *url;

	stream->streaming_ctrl = streaming_ctrl_new();
	if( stream->streaming_ctrl==NULL ) {
		return STREAM_ERROR;
	}
	stream->streaming_ctrl->bandwidth = network_bandwidth;
	url = url_new(stream->url);
	stream->streaming_ctrl->url = check4proxies(url);
	url_free33(url);

	mp_msg(MSGT_OPEN, MSGL_V, "STREAM_HTTP(2), URL: %s\n", stream->url);
	seekable = http_streaming_start(stream, file_format);
	if(seekable < 0) {
		if (stream->fd >= 0)
			closesocket(stream->fd);
		stream->fd = -1;
		streaming_ctrl_free33(stream->streaming_ctrl);
		stream->streaming_ctrl = NULL;
		return STREAM_UNSUPPORTED;
	}

	return fixup_open(stream, seekable);
}
/*
*
*
*
*/
const stream_info_t stream_info_http1 = {
	"http streaming",
	"null",
	"Bertrand, Albeau, Reimar Doeffinger, Arpi?",
	"plain http",
	open_s1,
	{"http", "http_proxy", "unsv", "icyx", "noicyx", NULL},
	NULL,
	0 // Urls are an option string
};
/*
*
*
*
*/
const stream_info_t stream_info_http2 = {
	"http streaming",
	"null",
	"Bertrand, Albeu, Arpi? who?",
	"plain http, also used as fallback for many other protocols",
	open_s2,
	{"http", "http_proxy", "pnm", "mms", "mmsu", "mmst", "rtsp", NULL},	//all the others as fallback
	NULL,
	0 // Urls are an option string
};
