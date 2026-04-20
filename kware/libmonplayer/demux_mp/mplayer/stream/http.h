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

#ifndef MPLAYER_HTTP_H
#define MPLAYER_HTTP_H

#include <sys/types.h>

#define MT_HTTP_SPACING_LEN 1
#define MT_HTTP_STATUS_CODE_LEN 3
#define MT_HTTP_SOCKREAD_RETRY 5
#define MT_HTTP_READLINE_BUFSIZE 512

#define MT_HTTP_CR "\r"
#define MT_HTTP_LF "\n"
#define MT_HTTP_CRLF "\r\n"
#define MT_HTTP_SPACING " "
#define MT_HTTP_CHUNKED "chunked"
#define MT_HTTP_TRANSFER_ENCODING "Transfer-Encoding"

#define mt_strhex2long(value) (value ? strtol(value, NULL, 16) : 0)
/*!
  This part define mthttp_download_start return values
  */
enum{
 /*!
   Unknown error
   */
 MTHTTP_UNKNOWN_ERROR = -1,
 /*!
   invalid download parameter
   */
 MTHTTP_PARAM_INVALID = -2,
 /*!
   Download session timeout
   */
 MTHTTP_SESSION_TIMEOUT = -3,
 /*!
   Download socket create failed
   */
 MTHTTP_SOCK_CREATE_FAILED = -4,
 /*!
   Authentication failed
   */
 MTHTTP_AHTHEN_FAILED = -5,
 /*!
   Request or redirect protocol un-support
   */
 MTHTTP_PROTOCOL_UNSUPPORT = -6,
};
typedef struct HTTP_field_type {
	char *field_name;
	struct HTTP_field_type *next;
} HTTP_field_t;

typedef struct {
	char *protocol;
	char *method;
	char *uri;
	unsigned int status_code;
	char *reason_phrase;
	unsigned int http_minor_version;
	// Field variables
	HTTP_field_t *first_field;
	HTTP_field_t *last_field;
	unsigned int field_nb;
	char *field_search;
	HTTP_field_t *field_search_pos;
	// Body variables
	char *body;
	size_t body_size;
	char *buffer;
	size_t buffer_size;
	size_t content_length;
	unsigned int is_parsed;
       long long chunksize;
        unsigned int isChunked;
} HTTP_header_t;
/*!
  for special info setting
  */
typedef struct {
    /*!
      user specified cookie length
      */
    size_t cookie_len;
    /*!
      user specified cookie content
      */
    char *p_cookie;
    /*!
      user specified user agent string length
      */
     size_t user_agent_len;
     /*!
      user specified user agent content
      */
     char *p_user_agent;
    /*!
      user specified content type length
      */
     size_t content_type_len;
    /*!
      user specified content type 
      */
     char *p_content_type;
    /*!
      user specified X_Request
      */
     char *p_x_request;
    /*!
      user specified X_Request content length
      */
     size_t x_request_len;
    /*!
      user specified post string
     */
     char *p_post_value;
    /*
    post string length
   */
   size_t post_value_len; 
   /*
   timeout
   */
   long  timeout; 
}HTTP_spec_info_t;

typedef struct {
    char *hostname;
    unsigned int port;
    char *username;
    char *password;
} HTTP_proxy_t;


HTTP_header_t*	http_new_header(void);
void		http_free33( HTTP_header_t *http_hdr );
int		http_response_append( HTTP_header_t *http_hdr, char *data, int length );
int		http_response_parse( HTTP_header_t *http_hdr );
int		http_is_header_entire( HTTP_header_t *http_hdr );
char* 		http_build_request( HTTP_header_t *http_hdr );
char* 		http_get_field( HTTP_header_t *http_hdr, const char *field_name );
char*		http_get_next_field( HTTP_header_t *http_hdr );
void		http_set_field( HTTP_header_t *http_hdr, const char *field_name );
void		http_set_method( HTTP_header_t *http_hdr, const char *method );
void		http_set_uri( HTTP_header_t *http_hdr, const char *uri );
int		http_add_basic_authentication( HTTP_header_t *http_hdr, const char *username, const char *password );
int		http_add_basic_proxy_authentication( HTTP_header_t *http_hdr, const char *username, const char *password );

void		http_debug_hdr( HTTP_header_t *http_hdr );

HTTP_header_t*	mthttp_http_new_header(void);
void		mthttp_http_free33( HTTP_header_t *http_hdr );
int		mthttp_http_response_append( HTTP_header_t *http_hdr, char *data, int length );
int            mthttp_http_chunked_parse(char *response_buf, unsigned int *length);
int		mthttp_http_response_parse( HTTP_header_t *http_hdr );
char *     mthttp_http_build_request( HTTP_header_t *http_hdr, HTTP_spec_info_t *spec_info);
char* 		mthttp_http_get_field( HTTP_header_t *http_hdr, const char *field_name );
void		mthttp_http_set_field( HTTP_header_t *http_hdr, const char *field_name );
void		mthttp_http_set_method( HTTP_header_t *http_hdr, const char *method );
void		mthttp_http_set_uri( HTTP_header_t *http_hdr, const char *uri );
int		mthttp_http_add_basic_authentication( HTTP_header_t *http_hdr, const char *username, const char *password );
int		mthttp_http_add_basic_proxy_authentication( HTTP_header_t *http_hdr, const char *username, const char *password );

int mthttp_download_start(char *p_url, HTTP_header_t **p_outbuf, unsigned int flag);
int mthttp_download_start(char *p_url, HTTP_header_t **p_outbuf, unsigned int flag);
int http_direct_download_start(char *p_url);
int chunkhttp_download_start(char *p_url, HTTP_header_t **p_outbuf);
int chunkhttp_download_common_start(char *p_url, HTTP_header_t **p_outbuf,  HTTP_spec_info_t *spec_info);

int mthttp_download_common_start(char *p_url, HTTP_header_t **p_outbuf, HTTP_spec_info_t *spec_info);

HTTP_header_t *  chunkhttp_read_header(int fd);
int chunkhttp_header_parse(HTTP_header_t *http_hdr);
int  chunkhttp_recv(int fd, char *response, unsigned int size, unsigned int isChunked);
void chunkhttp_close(int fd);
int chunkhttp_parse(int fd, char *buffer, unsigned int len);
int chunk_socket_readline(int fd, char *buffer, int bufferLen);
int chunk_parse_statusline(char *buffer, HTTP_header_t *http_hdr);
int mthttp_packet_ischunked(char *buffer);
int chunkhttp_parse_header(char *buffer, HTTP_header_t *http_hdr);
void *mthttp_proxy_config(HTTP_proxy_t *proxy);
void *mthttp_proxy_get(void);
char *mthttp_proxyurl_build(const HTTP_proxy_t *proxy, 
                            const char *host_url);
int filedownload_start(char *p_url, char *save_path, char *file_name);

#endif /* MPLAYER_HTTP_H */
