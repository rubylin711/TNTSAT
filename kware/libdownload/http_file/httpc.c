/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
 * a library to download files via HTTP1.1 using neon C library
 */


#if 0

#ifndef __LINUX__
#include "mp_func_trans.h"
#else
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#endif

#include <ne_session.h>
#include <ne_uri.h>
#include <ne_basic.h>
#include <ne_utils.h>
#include <ne_redirect.h>

#include "httpc.h"

#define HTTP_MAX_RETRY     3
#define HTTP_MAX_FILELEN   100

#define HTTP_CONNECT_TIMEOUT   15
#define HTTP_READ_TIMEOUT      15

#define METHOD_GET             0
#define METHOD_POST            1

void http_free_file_res(struct http_filesave_res * res)
{
  if (res == NULL) 
    return;

  if (res->filename) 
    free(res->filename);

  if (res->httprsp) 
    free(res->httprsp);

  if (res->ctype) 
    free(res->ctype);

  free(res);
}

static void http_copy_str(char** ptr, const char* data) {
  char *cp;
  int l = strlen(data);

  if (!data) {
    *ptr = NULL;
    return;
  }
  *ptr = cp = malloc(l+1);
  if (cp == NULL) 
    return;

  memcpy(cp, data, l);
  cp[l] = '\0';
}


static int http_open_file(struct http_filesave_res * res, 
			  char* dpath, char* filen, const ne_uri *uri,
			  const char *cdis)
{
  int len, dlen;
  /* 
   * determine the best filename to use to save the http response,
   * open the file, and return the fd.
   */
  dlen = (dpath==NULL)? 0:strlen(dpath);
  if (filen != NULL) {
    len = dlen + strlen(filen);
    res->filename = malloc(len + 2);
    if (!res->filename) 
      return -1;
    if (dpath != NULL) {
        if (dpath[dlen-1] == '/') 
            snprintf(res->filename, len+2, "%s%s", dpath, filen);
        else 
            snprintf(res->filename, len+2, "%s/%s", dpath, filen);
    } else {
        snprintf(res->filename, len+2, "%s", filen);
    }
    #ifdef __LINUX__
    return open(res->filename, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    #else
    ufs_file_t *file_handle;
    u8 ret;
    file_handle = malloc(sizeof(ufs_file_t));
    memset(file_handle, 0, sizeof(ufs_file_t));
    ret = ufs_open(file_handle, "myfile", UFS_READ | UFS_WRITE | UFS_CREATE_NEW_COVER);
    if(ret){
      mtos_free(file_handle);
      return 0;
    }
    return file_handle;
    #endif
  }
  return -1;
}

static void http_request_save(struct http_filesave_res* res,
			      ne_uri * uri, char* dpath, char* filen,
                              int operation, const char *buffer)
{
  ne_session *sess;
  ne_request *req;
  const ne_status * st;
  int setry = 0, fd, ret;
  const ne_uri *reuri;
  const char *value;
  ne_buffer *reqbuf;

  while (setry < HTTP_MAX_RETRY) {
    printf("open ses: %s, %s, %d, %s, %s\n", uri->scheme, uri->host, uri->port, uri->path, uri->query);
    sess = ne_session_create(uri->scheme, uri->host, uri->port);
    ne_set_useragent(sess, "RealtekVOD");
    ne_set_connect_timeout(sess, HTTP_CONNECT_TIMEOUT);
    ne_set_read_timeout(sess, HTTP_READ_TIMEOUT);
    ne_redirect_register(sess);
    
    /*
     * construct the http request 
     */
    if (uri->query == NULL) {
        if (operation == METHOD_GET)
            req = ne_request_create(sess, "GET", uri->path);
        else {
            printf("warning! this URL looks like a GET, but set to POST\n");
            req = ne_request_create(sess, "POST", uri->path);
            ne_add_request_header(req, "Content-Type", "application/x-www-form-urlencoded");
            ne_set_request_flag(req, NE_REQFLAG_IDEMPOTENT, 0);
            ne_set_request_body_buffer(req, buffer, strlen(buffer));
        }
    }
    else {
        reqbuf = ne_buffer_create();
	ne_buffer_concat(reqbuf, uri->path, "?", NULL);
	ne_buffer_zappend(reqbuf, uri->query);

        if (operation == METHOD_GET)
            req = ne_request_create(sess, "GET", reqbuf->data);
        else {
            req = ne_request_create(sess, "POST", reqbuf->data);
            ne_set_request_flag(req, NE_REQFLAG_IDEMPOTENT, 0);
            ne_set_request_body_buffer(req, buffer, strlen(buffer));
        }
	ne_buffer_destroy(reqbuf);
    }
    ne_add_request_header(req, "Accept", "*/*");
    st = ne_get_status(req);
    res->code = ret = ne_begin_request(req);

    printf("begin request, code=%d\n", ret);

    if (ret != NE_OK) {
      http_copy_str(&res->httprsp, ne_get_error(sess));
      ne_request_destroy(req);
      ne_session_destroy(sess);
      return;
    }

    value = ne_get_response_header(req, "Content-Type");
    if (value != NULL) {
      http_copy_str(&res->ctype, value);
      value = ne_get_response_header(req, "Content-Disposition");
      fd = http_open_file(res, dpath, filen, uri, value); 
      #ifdef __LINUX__
      if (fd < 0) {
      #else
      if (fd == 0) {
      #endif
	ne_request_destroy(req);
	ne_session_destroy(sess);
	res->code = HTTP_FILE;
	return;
      }
	  if (st->klass == 2) {
		res->code = ret = ne_read_response_to_fd(req, fd);
	  } else {
		res->code = ret = ne_end_request(req);
		printf("discard response\n");
		if (ret == NE_REDIRECT) {
		  printf("go for redirect\n");
		  goto redirect;
		}
                printf("error: %s\n", ne_get_error(sess));
		break;
 	  }
      #ifdef __LINUX__
      close(fd);
      #else
      ufs_close(fd);
      #endif
      http_copy_str(&res->httprsp, ne_get_error(sess));
      if (ret == NE_OK) ret = ne_end_request(req);
      break;
    }
    /* content-type is NULL, check! */
    if (ret == NE_OK) ret = ne_end_request(req);
    if (ret == NE_REDIRECT) {
 redirect:
	  printf("redirect on url\n");
      reuri =  ne_redirect_location(sess);
      ne_uri_free(uri);
      ne_uri_copy(uri, reuri);
      if (uri->scheme == NULL) 
	uri->scheme = ne_strndup("http", 4);
      if (uri->port == 0)
	uri->port = ne_uri_defaultport(uri->scheme);
      ne_request_destroy(req);
      ne_session_destroy(sess);
      setry ++;
      continue;
    } else {
      res->code = ret;
      http_copy_str(&res->httprsp, ne_get_error(sess));
      break;
    }

  } /* while */

  ne_request_destroy(req);
  ne_session_destroy(sess);
}

struct http_filesave_res*
http_post_savefile (char *url, char *dpath, char *filen, const char* postbuffer) 
{
    ne_uri uri;
    struct http_filesave_res *res;
    if (filen == NULL)
        return NULL;
    
    res = malloc(sizeof(struct http_filesave_res));
    if (!res) 
        return NULL;
    memset(res, 0, sizeof(struct http_filesave_res));
    if (ne_sock_init()) {
        free(res);
        return NULL;
    }
    if (ne_uri_parse(url, &uri) || uri.host == NULL || 
        uri.path == NULL || (strcasecmp(uri.scheme, "https") == 0)) {
        ne_uri_free(&uri);
        free(res);
        ne_sock_exit();
        return NULL;
    }

    if (uri.scheme == NULL) 
        uri.scheme = ne_strndup("http", 4);
    if (uri.port == 0)
        uri.port = ne_uri_defaultport(uri.scheme);

    http_request_save(res, &uri, NULL, filen, METHOD_POST, postbuffer);
    ne_uri_free(&uri);
    ne_sock_exit();
    return res;
}

  
    
struct http_filesave_res*
http_req_savefile (char *url, char *dpath, char *filen) {
  DIR *dir;
  ne_uri uri;
  struct http_filesave_res *res;

  if (filen == NULL) {
      return NULL;
  }
  
  res = malloc(sizeof(struct http_filesave_res));
  if (!res) 
    return NULL;

  memset(res, 0, sizeof(struct http_filesave_res));
#ifdef __LINUX__
  if (dpath != NULL) {
      if ((dir = opendir(dpath)) == NULL) {
          free(res);
          return NULL;
      }
      closedir(dir);
  }
#endif  
  if (ne_sock_init()) {
    free(res);
    return NULL;
  }

  /* don't handle https yet */
  if (ne_uri_parse(url, &uri) || uri.host == NULL || 
      uri.path == NULL || (strcasecmp(uri.scheme, "https") == 0)) {
    ne_uri_free(&uri);
    free(res);
    ne_sock_exit();
    return NULL;
  }

  if (uri.scheme == NULL) 
    uri.scheme = ne_strndup("http", 4);
  if (uri.port == 0)
    uri.port = ne_uri_defaultport(uri.scheme);
  
  http_request_save(res, &uri, dpath, filen, METHOD_GET, NULL);
  ne_uri_free(&uri);
  ne_sock_exit();
  return res;
}

#endif
