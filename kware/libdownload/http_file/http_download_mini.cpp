/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
 * asynchronous http download.
 * Peacer Tsui
 */
#define _XOPEN_SOURCE 600

#include "mt_type.h"
//#include "osal_mtos.h"
#include "mtos_misc.h"

#include <stdlib.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/file.h>


#include <assert.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <ne_session.h>
#include <ne_request.h>
#include <ne_uri.h>
#include <ne_basic.h>
#include <ne_utils.h>
#include <ne_redirect.h>
#include <ne_socket.h>

#include "http_download_mini.h"
#include "md5.h"
#include <openssl/ssl.h>

#define mtos_printk  //printf	//peacer add for complication error
#define OS_PRINTF 	//printf


#define CLEAN_ARG_CONN       0
#define CLEAN_ARG_JOB        1
#define CLEAN_ARG_ALL        2

#define TMP_BUFFER_SIZE      4096  // multiple of pagesize

#define MAX_REDIRECT_CNT    6

#define  HTTP_DOWNLOAD_STACK_SIZE  (512*1024)


struct thread_dld {
    // md5 of the original URL - so we can find an existing job
    // given an URL
    u32 urlmd5[4];

    int http_connect_timeout; ///seconds
    int http_read_timeout;///seconds

    // current downloading URL and save filename
    ne_buffer * volatile url;
    ne_buffer * volatile fname;

    char * extraHeaders;         // any extra http headers to attach
    char * httpBody;            // http body to send (for post)
    unsigned int bodyLen;       // length of http body

    bool isPost;

    ssize_t volatile written;
    ssize_t volatile total;
    http_state volatile state;
    bool abort_by_user;

    bool redirect_flag;
    int redirect_cnt;

    int      fd;
    void * p_ufs_file;

    /* tmp buffer */
    char * http_buffer;
    ssize_t buf_offset;

    /* ne states */
    ne_session * volatile sess;
    ne_request * volatile req;

    /* return result */
    unsigned short volatile code;
    ne_buffer * volatile http_res;
    ne_buffer * volatile content_type;

    HTTP_rsp_header_t *http_rsp_header;
    char *cookie;
};

static void clean_arg(void * arg, int flag);
static int   http_begin_read(struct thread_dld * dld);
static int    http_begin_http(struct thread_dld * dld);

char *strdup1(const char *s)
{
  char *new_string = NULL;

  if (s == NULL)
  {
     return NULL;
  }


  new_string = (char*)malloc(strlen(s) + 1);

   if(new_string)
   {
      memset(new_string,0,strlen(s) + 1);
      strcpy(new_string, s);
   }

  return new_string;
}


/*
*
*
*
*
*
*/
static void compute_md5_url(void * result, const char * URL)
{
    MD5Context md5ctx;
    md5_init_ctx(&md5ctx);
    md5_process_bytes(URL, strlen(URL), &md5ctx);
    md5_finish_ctx(result, &md5ctx);
}
static void delete_file(struct thread_dld * dld)
{
    if (dld->fname) {
        OS_PRINTF("\n%s %d  %x\n", __func__, __LINE__, dld->fname->data);
        unlink(dld->fname->data);
    }
}

/*
*
*
*
*
*
*
*/
static void http_fill_res(struct thread_dld * dld, ne_session * sess)
{
    //OS_PRINTF("[%s] start start..\n", __func__);
    if (dld->http_res) {
        ne_buffer_destroy(dld->http_res);
    }

    dld->http_res = ne_buffer_ncreate(64);
    ne_buffer_zappend(dld->http_res, ne_get_error(sess));
    //OS_PRINTF("[%s] end end..\n", __func__);
    return;
}
/*
*
*Z:\drv_3.0\MainBranchDev_Drv\montage-tech
*
*
*
*
*/
static void http_fill_ctype(struct thread_dld * dld, const char * ctype)
{
    if (dld->content_type) {
        ne_buffer_destroy(dld->content_type);
    }

    dld->content_type = ne_buffer_create();
    ne_buffer_zappend(dld->content_type, ctype);
    return;
}
/*
*
*
*
*
*
*
*/
static void trimstr(char * buffer)
{
    int strl = 0;
    strl = strlen(buffer);
    strl --;

    while ((strl >= 0) && (buffer[strl] == ' ' || buffer[strl] == '\r' ||
                           buffer[strl] == '\n')) {
        buffer[strl] = '\0';
        strl -- ;
    }
}
static int copy_http_rsp_value(ne_request * req, char *key, char *buf, int buflen)
{
    const char * value;
    if(key && buf && buflen>0){
        value = ne_get_response_header(req, key);
        if(value){
            memset(buf,0,buflen);
            strncpy(buf,value,buflen);
        }
    }
    else
        return -1;

    return 0;
}
static int http_parse_rsp_header(struct thread_dld * dld)
{
    int i;
    const char * value;
    HTTP_rsp_header_t *header = dld->http_rsp_header;
    ne_request * req = dld->req;
    if(!header)
        return -1;

    for(i=0;i<4;i++)
        copy_http_rsp_value(req, header->key[i], header->value_buf[i], header->value_buflen[i]);

    return 0;
}
static int parse_cookie(struct thread_dld * dld)
{
    ne_request * req;
    if(!dld)
        return -1;
    req = dld->req;
    if(!req)
        return -1;
    const char *value = ne_get_response_header(req, "Set-Cookie");
    if(value)
    {
        dld->cookie = strdup1(value);
        return 0;
    }
    return -1;
}
/*
*
*
*
*  RETURN VALUE:  1 means success
*                          0 means failure
*
*/
static int http_accept(struct thread_dld * dld)
{
    const ne_uri * reuri;
    const char * value;
    const ne_status * st;
    ne_request * req;
    int state;
    req = dld->req;
    st = ne_get_status(req);
    //OS_PRINTF("[%s] start start... \n", __func__);

	///406 means ZHAOGE server will response 406 and http body, which we will use
    if (st->klass == 2 || st->code == 406) {
        //int oflags;
        //OS_PRINTF("[%s] st->klass is 2.. \n", __func__);
        value = ne_get_response_header(req, "Content-length");

        if (value != NULL) {
            dld->total = atoi(value);
        } else {
            dld->total = -1;
        }

        value = ne_get_response_header(req, "Content-Type");

        if (value) {
            //OS_PRINTF("[%s] fill ctype... \n", __func__);
            http_fill_ctype(dld, value);
        }

        if(dld->http_rsp_header)
            http_parse_rsp_header(dld);

        //OS_PRINTF("[%s]  Downloading Starts , 0x%x !!!\n", __func__, dld);
        if(dld->state == HTTP_ABORTED_BY_USER) {
            state = HTTP_ABORTED_BY_USER;
        } else {
            state = HTTP_DOWNLOADING;
        }

#if 0//del by libin ,move to downloader
        int nfd = ne_get_socket(dld->sess);

        if (strcmp(ne_get_scheme(dld->sess), "https") != 0) {
#if 0 //def __LINUX__//to be confirmed
            //if ((oflags = fcntl(nfd, F_GETFL, 0)) != -1){
            //fcntl(nfd, F_SETFL, oflags | O_NONBLOCK);
            //  }
#endif
        }

        if (ne_get_buffered(dld->sess) != 0) {
            int tmp = http_begin_read(dld);

            if (tmp != -1) {
                state = tmp;
            }
        }
#endif
        //OS_PRINTF("[%s] end end 1111... \n", __func__);
        return state;
    }

    if (st->klass == 3) {
        //OS_PRINTF("[%s] st->klass is 3 [redirect encountered] \n", __func__);
        dld->redirect_cnt++;
        if(dld->redirect_cnt >= MAX_REDIRECT_CNT)
        {
            OS_PRINTF("redirect max cnt %d, so abort\n", dld->redirect_cnt);
            http_fill_res(dld, dld->sess);
            dld->code = ne_end_request(req);
            return HTTP_ABORTED;
        }
        /* redirect encountered */
 
        dld->code = ne_end_request(req);
        ne_close_connection(dld->sess);
        reuri = ne_redirect_location(dld->sess);

        if (reuri == NULL) {
            OS_PRINTF("Server Error, redirect with incorrect redirect URL\n");
            goto aborted;
        }

        parse_cookie(dld);

        /* go for redirect */
        ne_buffer_destroy(dld->url);
        dld->url = ne_buffer_create();
        char * p = ne_uri_unparse(reuri);
        ne_buffer_zappend(dld->url, p);
        ne_free(p);
        ne_request_destroy(dld->req);
        ne_session_destroy(dld->sess);
        dld->req = NULL;
        dld->sess = NULL;
        dld->redirect_flag = true;
        OS_PRINTF("redirect dld  on %p, url %s\n", dld, dld->url->data);
        state = http_begin_http(dld);
        //OS_PRINTF("[%s] end end 222222... \n", __func__);
        return state;
    }
    OS_PRINTF("[%s]http dl aborted: %s\n", __func__, ne_get_error(dld->sess));
    http_fill_res(dld, dld->sess);
    dld->code = ne_end_request(req);
aborted:
//    clean_arg(dld, CLEAN_ARG_CONN);  yiyuan disable this 20150226 bug:57144

    if(dld->state == HTTP_ABORTED_BY_USER) {
        state = HTTP_ABORTED_BY_USER;
    } else {
        state = HTTP_ABORTED;
    }
    //OS_PRINTF("[%s] end end .. \n", __func__);
    return state;
}
static ssize_t write_fully(int fd, const char * buffer, ssize_t bufsize)
{
    ssize_t written = 0, ret;

    if (fd == -1) {
        return 0;
    }

    while (written < bufsize) {
        ret = write(fd, buffer + written, bufsize - written);

        if (ret > 0) {
            written += ret;
        } else {
            return ret;
        }
    }

    return written;
}
/*
*
*
*
*
*
*
*/
static int http_begin_read(struct thread_dld * dld)
{
    ssize_t len, ret;
    size_t bufed;
    size_t off;
    unsigned int prevTime = 0;
    int state;
    len = 0;
    ret = 0;
    bufed = 0;
    off = 0;
    prevTime = 0;
    //OS_PRINTF("[%s] start start ...\n", __func__);
    /*
     * calculate offset/len of tmp buffer to download
     */
    off = TMP_BUFFER_SIZE - dld->buf_offset;
    //yliu add:clr
    memset(dld->http_buffer + dld->buf_offset, 0, off);
    len = ne_read_response_block(dld->req, dld->http_buffer + dld->buf_offset, off);

    if (len > 0) {
        dld->buf_offset += len;

        if (dld->buf_offset == TMP_BUFFER_SIZE) {
            ret = write_fully(dld->fd, dld->http_buffer, TMP_BUFFER_SIZE);
            dld->written += TMP_BUFFER_SIZE;
            dld->buf_offset = 0;
        }
#if 0
        while ((bufed = ne_get_buffered(dld->sess)) != 0) {
#else
        while(1){
#endif
            off = TMP_BUFFER_SIZE - dld->buf_offset;
            len = ne_read_response_block(dld->req, dld->http_buffer + dld->buf_offset, off);

            if (len == 0 && dld->state != HTTP_ABORTED_BY_USER) {
                goto done;
            } else if(dld->state == HTTP_ABORTED_BY_USER) {
                goto user_abort;
            } else if(len < 0){
                goto bad_write;
            }

            dld->buf_offset += len;

            if (dld->buf_offset == TMP_BUFFER_SIZE) {
                ret = write_fully(dld->fd, dld->http_buffer, TMP_BUFFER_SIZE);
                dld->written += TMP_BUFFER_SIZE;
                dld->buf_offset = 0;
            }
        }

        if(dld->state == HTTP_ABORTED_BY_USER) {
            goto user_abort;
        }

        if (ne_request_remain(dld->req) == 0) {
            goto done;
        }

        return -1;
    } else if (len < 0) {
        mtos_printk("####$$$$ OUCH!, CONNECTION CLOSED!!!\n");
        goto bad_write;
    }

done:

    if (dld->buf_offset) {
        ret = write_fully(dld->fd, dld->http_buffer, dld->buf_offset);
    }

    dld->written += dld->buf_offset;
    OS_PRINTF("[%s]@@[HTTP_COMPLETE] total [%d] bytes!!@@@\n", __func__, dld->written);

    if(dld->state == HTTP_ABORTED_BY_USER) {
        goto user_abort;
    }
    state = HTTP_COMPLETE;
    dld->code = ne_end_request(dld->req);
    http_fill_res(dld, dld->sess);
    goto finished;
user_abort:
    OS_PRINTF("[%s] user abort encountered!\n", __func__);
    state = HTTP_ABORTED_BY_USER;
    dld->code = ne_end_request(dld->req);
    http_fill_res(dld, dld->sess);
    goto finished;
bad_write:
    OS_PRINTF("[%s] write error encountered!\n", __func__);
    if(dld->state == HTTP_ABORTED_BY_USER) {
        state = HTTP_ABORTED_BY_USER;
    } else {
        state = HTTP_ABORTED;
    }
    dld->code = ne_end_request(dld->req);
    http_fill_res(dld, dld->sess);
    goto finished;
finished:
    //OS_PRINTF("[%s] download done for %d, %s, %p\n", __func__,dld->descriptor, dld->url->data, dld);
//    clean_arg(dld, CLEAN_ARG_CONN);yiyuan disable this 20150226 bug:57144
    return state;
}

/*
*
*
*
*
*
*
*/
// find if "Content-Type" is already defined in extra headers
static int http_find_contenttype(char * extraHeaders)
{
        if(strstr(extraHeaders, "content-type") || strstr(extraHeaders, "Content-Type"))
        {
            return 1;
        }

    return 0;
}
/*
*
*
*
*
*
*
*/
static int my_verification(void * userdata, int failures, const ne_ssl_certificate * cert)
{
#ifdef  CONFIG_MT_ENABLE_OPEN_SSL
    const char * id = ne_ssl_cert_identity(cert);
    char * dn;

    if (failures & NE_SSL_UNTRUSTED) {
        OS_PRINTF("Untrusted certificate found!\n");
    }

    if (failures & NE_SSL_IDMISMATCH) {
        OS_PRINTF("Server certification mismatch!\n");
    }

    if (failures & NE_SSL_EXPIRED) {
        OS_PRINTF("Certification has expired!\n");
    }

    if (failures & NE_SSL_NOTYETVALID) {
        OS_PRINTF("Certification is not yet valid\n");
    }

    if (id) {
        OS_PRINTF("Certificate issed for %s\n", id);
    }

    dn = ne_ssl_readable_dname(ne_ssl_cert_subject(cert));

    if (dn) {
        OS_PRINTF("Subject: %s\n", dn);
        free(dn);
    }

    dn = ne_ssl_readable_dname(ne_ssl_cert_issuer(cert));

    if (dn) {
        OS_PRINTF("Issuer: %s\n", dn);
        free(dn);
    }

#endif
    return 0;
}


/*
 *
 *
 * initialize certificates for secure layer - load all security
 * certificates (.pem file)
 *
 */
static void http_initialize_certs(ne_session * sess)
{
    //peacer add
    //maybe in future, we will support ssl based on certificates !!!!!!
#if 0//peacer del, and please don't remove these code
    char * keydirs[2];
    char path[512];
    DIR * dir;
    struct dirent * entry;
    int i;
    struct stat info;
    // got to find a smarter way of doing this.
    keydirs[0] = "./Resource/";
    keydirs[1] = "./player/";

    for (i = 0; i < 2; i++) {
        if ((dir = opendir(keydirs[i])) == NULL) {
            OS_PRINTF("open dir fail %s\n", keydirs[i]);
            continue;
        }

        while ((entry = readdir(dir)) != NULL) {
            if ((strcmp(entry->d_name, ".") == 0) ||
                (strcmp(entry->d_name, "..") == 0)) {
                continue;
            }

            if (strcmp(entry->d_name + strlen(entry->d_name) - 4,
                       ".pem") != 0) {
                continue;
            }

            // got a file with .pem extension
            OS_PRINTF("trust cert: %s\n", entry->d_name);
            snprintf(path, 512, "%s%s",
                     keydirs[i], entry->d_name);

            if (stat(path, &info) != 0) {
                continue;
            } else if (S_ISDIR(info.st_mode)) {
                continue;
            } else {
                ne_ssl_certificate * ca = ne_ssl_cert_read(path);

                if (ca == NULL) {
                    OS_PRINTF("%s is not valid cert!\n", path);
                    continue;
                }

                ne_ssl_trust_cert(sess, ca);
                ne_ssl_cert_free(ca);
            }
        }

        closedir(dir);
    }
#else
	char* certfile = NULL ;
	char* certkey = NULL ;
	char* pw = NULL ;
//	int ret  = ne_ssl_trust_keypair(sess,certfile,certkey,pw) ;
	//int ret = ne_ssl_trust_keypair(sess,"./client.crt","./client.key","yiyuan") ;
//	OS_PRINTF(" ret = : %d\n", ret);

#endif
    ne_ssl_set_verify(sess, my_verification, NULL);
}
void check_extraheader(char *p)
{
    if(!p)
        return;

    unsigned len = strlen(p);
    if(p[len-2] == 0x0d && p[len-1] == 0x0a)//check "\r\n"
    {
        p[len-2] = 0;
        p[len-1] = 0;
    }

    return;
}
/*
*
*
*
*
*
*/
static int http_begin_http(struct thread_dld * dld)
{
    ne_uri uri ;
    ne_buffer * reqbuf = NULL;
    memset(&uri, 0, sizeof(ne_uri));
    //OS_PRINTF("[%s] start start ...\n", __func__);

    if (ne_uri_parse(dld->url->data, &uri)) {
        OS_PRINTF("[%s][ERROR] unable to parse dld url\n", __func__);
        goto aborted;
    }

    if (uri.scheme == NULL) {
        uri.scheme = ne_strndup("http", 4);
    }

    if (strcmp(uri.scheme, "https") == 0) {
        OS_PRINTF("[%s] >>this is https!!!<<\n", __func__);
#ifdef  CONFIG_MT_ENABLE_OPEN_SSL
        SSLeay_add_ssl_algorithms();
#endif
    }

    //OS_PRINTF("[%s] ==url->data:%s!!!\n", __func__, dld->url->data);

    if (uri.port == 0) {
        uri.port = ne_uri_defaultport(uri.scheme);
    }

    dld->sess = ne_session_create(uri.scheme, uri.host, uri.port);

    if (strcmp(uri.scheme, "https") == 0) {
        // secure layer HTTP
        ne_set_session_flag(dld->sess, NE_SESSFLAG_SSLv2, 1);
        http_initialize_certs(dld->sess);
    }

    //OS_PRINTF("[%s] start set network param ...\n", __func__);
    ne_set_connect_timeout(dld->sess, dld->http_connect_timeout);
    ne_set_read_timeout(dld->sess, dld->http_read_timeout);
    // ne_set_useragent(dld->sess, "MXXX_STB");
    /********************check extraheader, delete last "\r\n"*******************/
    check_extraheader(dld->extraHeaders);

    if(dld->extraHeaders && (strstr(dld->extraHeaders,"user-agent") || strstr(dld->extraHeaders,"User-Agent")))
        ;
    else
        ne_set_useragent(dld->sess, "Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1985.143 Safari/537.36");

    ne_set_session_flag(dld->sess, NE_SESSFLAG_NOBUFFER, 1);
    ne_redirect_register(dld->sess);

    //OS_PRINTF("[%s] end set network param ...\n", __func__);

    //OS_PRINTF("[%s] start create request header ...\n", __func__);
    if (uri.query == NULL) {
        if (dld->isPost) {
            //OS_PRINTF("[%s] current method is POST...\n", __func__);
            dld->req = ne_request_create(dld->sess, "POST", uri.path);

            ne_add_request_header(dld->req, "Accept", "*/*");
            if(dld->cookie)
                ne_add_request_header(dld->req, "Cookie", dld->cookie);
            if (dld->extraHeaders) {
                if (!http_find_contenttype(dld->extraHeaders)) {
                    // no content-type defined in POST, add default
                    ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
                }

                ne_add_request_headers(dld->req, dld->extraHeaders);
            } else {
                OS_PRINTF("[%s] application/x-www-form-urlencoded\n", __func__);
                ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
            }

            ne_set_request_flag(dld->req, NE_REQFLAG_IDEMPOTENT, 0);

            if (dld->httpBody) {
                ne_set_request_body_buffer(dld->req, dld->httpBody, dld->bodyLen);
            }
        } else {
            //OS_PRINTF("[%s] current method is GET...\n", __func__);
            dld->req = ne_request_create(dld->sess, "GET", uri.path);
            if(dld->cookie)
                ne_add_request_header(dld->req, "Cookie", dld->cookie);
            if (dld->extraHeaders) {
                ne_add_request_headers(dld->req, dld->extraHeaders);
            }
        }
    } else {
        //OS_PRINTF("[%s] uri.query is not NULL...\n", __func__);
        reqbuf = ne_buffer_create();
        ne_buffer_concat(reqbuf, uri.path, "?", NULL);
        ne_buffer_zappend(reqbuf, uri.query);

        if (dld->isPost) {
            //OS_PRINTF("[%s] method is  POST...\n", __func__);
            dld->req = ne_request_create(dld->sess, "POST", reqbuf->data);
            if(dld->cookie)
                ne_add_request_header(dld->req, "Cookie", dld->cookie);
            if (dld->extraHeaders) {
                if (!http_find_contenttype(dld->extraHeaders)) {
                    // no content-type defined in POST, add default
                    ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
                }

                ne_add_request_headers(dld->req, dld->extraHeaders);
            } else {
                ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
            }

            ne_set_request_flag(dld->req, NE_REQFLAG_IDEMPOTENT, 0);

            if (dld->httpBody) {
                ne_set_request_body_buffer(dld->req, dld->httpBody, dld->bodyLen);
            }
        } else {
            //OS_PRINTF("[%s] method is  GET...\n", __func__);
            dld->req = ne_request_create(dld->sess, "GET", reqbuf->data);
            if(dld->cookie)
                ne_add_request_header(dld->req, "Cookie", dld->cookie);
            if (dld->extraHeaders) {
                ne_add_request_headers(dld->req, dld->extraHeaders);
            }
        }

        ne_buffer_destroy(reqbuf);
    }

    //ne_add_request_header(dld->req, "Range", "bytes=0-");
    //OS_PRINTF("[%s] 2222...\n", __func__);
    //OS_PRINTF("[%s] finish to create request header ...\n", __func__);
    if (!dld->isPost) {
        ne_add_request_header(dld->req, "Accept", "*/*");
    }

    //OS_PRINTF("[%s] 33333...\n", __func__);
    //OS_PRINTF("[%s] start http request lalalala ...\n", __func__);
//    copy_priv_ne_request(dld->req,(void*)dld);
    dld->code = ne_begin_request(dld->req);
    if (dld->code == NE_OK) {
        //OS_PRINTF("[%s][%d]connected , ptr %p\n",__func__, dld->descriptor, dld);
        //OS_PRINTF("[%s] 5555..\n", __func__);
        int state;
        //OS_PRINTF("[%s] start accept..\n", __func__);
        state = http_accept(dld);
        //OS_PRINTF("[%s] end accept..\n", __func__);
        ne_uri_free(&uri);
        //OS_PRINTF("[%s] end end...\n", __func__);
        return state;
    } else if(dld->code == NE_TIMEOUT) {
        OS_PRINTF("[%s] fail to http request lallala ...timeout\n", __func__);
        ne_uri_free(&uri);
        if(dld->state == HTTP_ABORTED_BY_USER) {
            return HTTP_ABORTED_BY_USER;
        }
        //OS_PRINTF("[%s] abort 111...\n", __func__);
        return HTTP_TIMESOUT;
    }

aborted:
    OS_PRINTF("[%s] fail to http request lallala ...\n", __func__);
    ne_uri_free(&uri);
    if(dld->state == HTTP_ABORTED_BY_USER) {
        return HTTP_ABORTED_BY_USER;
    }
    //OS_PRINTF("[%s] abort 111...\n", __func__);
    return HTTP_ABORTED;
}


/*
*
*
*
*
*
*/
static int http_select(struct thread_dld * one, fd_set * rfds, fd_set * efds)
{
    int max_fd = 0, nfd = 0;
    //OS_PRINTF("[%s] start start \n",__func__);
    FD_ZERO(rfds);
    FD_ZERO(efds);
    //FD_SET(ctx->pfds[0], rfds);  //peacer del
    //FD_SET(ctx->pfds[0], efds);
    //count = ctx->pfds[0];
    //yliu modify for youx geturl hold
    max_fd = 0;

    if (one->sess != NULL) {
        nfd = ne_get_socket(one->sess);

        if (nfd >= 0) {
            FD_SET(nfd, rfds);
            FD_SET(nfd, efds);
            //tot++;
        }

        if (nfd > max_fd) {
            max_fd = nfd;
        }
    }

    //OS_PRINTF("[%s] end end \n",__func__);
    return max_fd;
}




/*
*
*
*
*
*
*
*/
static int http_all_req(struct thread_dld * dld,
                        fd_set * rfds, fd_set * efds)
{
    int fd;
    fd = ne_get_socket(dld->sess);

    if (fd < 0) {
        return -1;
    }

    if (FD_ISSET(fd, rfds) || FD_ISSET(fd, efds)) {
        return http_begin_read(dld);
    }
}

/*
*
*
*
*
*
*
*/
static void clean_arg(void * arg, int flag)
{
    struct thread_dld * dld = (struct thread_dld *) arg;
    /* first clean up http connection related state */
    //OS_PRINTF("[%s] start start ...\n", __func__);
    dld->buf_offset = 0;

    if (dld->fd != -1) {
        close(dld->fd);
        dld->fd = -1;
    }


    if (dld->fname) {

        if(dld->state == HTTP_ABORTED_BY_USER || dld->state == HTTP_ABORTED){
            if(dld->fname){
                unsigned short  path_tmp[256]={0};
            }
        }

        ne_buffer_destroy(dld->fname);
        dld->fname = NULL;
    }

    if (dld->req) {
        ne_request_destroy(dld->req);
        dld->req = NULL;
    }

    if (dld->sess) {
        //OS_PRINTF("[%s] destroy sess!!\n", __func__);
        ne_session_destroy(dld->sess);
        dld->sess = NULL;
    }

    if (flag == CLEAN_ARG_CONN) {
        //OS_PRINTF("[%s] CLEAN_ARG_CONN!!\n", __func__);
        //OS_PRINTF("[%s] end end 111...\n", __func__);
        return;
    }
    if (dld->http_rsp_header)
    {
      free(dld->http_rsp_header);
      dld->http_rsp_header = NULL;
    }
    if (dld->http_buffer) {
        //OS_PRINTF("[%s] ==444 http_buffer[0x%x]==\n", __func__, dld->http_buffer);
        free(dld->http_buffer);
        dld->http_buffer = NULL;
    }

    if (dld->http_res) {
        //OS_PRINTF("[%s] ==444 http_res[0x%x]==\n", __func__, dld->http_res);
        ne_buffer_destroy(dld->http_res);
        dld->http_res = NULL;
    }

    if (dld->extraHeaders) {
        //OS_PRINTF("[%s] ==444 extraHeaders[0x%x]==\n", __func__, dld->extraHeaders);
        free(dld->extraHeaders);
        dld->extraHeaders = NULL;
    }

    if (dld->httpBody && dld->bodyLen) {
        //OS_PRINTF("[%s]  free httpBody!!\n", __func__);
        free(dld->httpBody);
        dld->httpBody = NULL;
    }

    dld->httpBody = NULL;
    dld->bodyLen = 0;

    if (dld->url) {
        //OS_PRINTF("[%s]  destroy url!!\n", __func__);
        ne_buffer_destroy(dld->url);
        dld->url = NULL;
    }

    if (dld->content_type) {
        //OS_PRINTF("[%s] destroy content_type!!\n", __func__);
        ne_buffer_destroy(dld->content_type);
        dld->content_type = NULL;
    }

    if (dld->http_rsp_header) {
        //OS_PRINTF("[%s] destroy http_rsp_header!!\n", __func__);
        free(dld->http_rsp_header);
        dld->http_rsp_header = NULL;
    }

    if(dld->cookie)
    {
        free(dld->cookie);
        dld->cookie = NULL;
    }
    //OS_PRINTF("[%s] end end ...\n", __func__);
    return;
}

static int check_url(char *url)
{
    char *p = url;
    if(strncmp(url,"https",5) == 0){
        p+=8;
    }
    else if(strncmp(url,"http",4) == 0){
        p+=7;
    }
    else{
        OS_PRINTF("url is not http or https url!!!!\n");
        return -1;
    }

    char find_sprit = 0;
    while(*p != '?' && *p != '\0'){
        if(*p == '/')
            find_sprit = 1;
        p++;
    }

    if(!find_sprit && *p == '?'){
        memmove(p+1,p,strlen(p));
        *p = '/';
        OS_PRINTF("url find error, we set new:%s\n",url);
    }

    return 0;
}

extern "C" int get_user_abort_flag(void *dld)
{
    thread_dld* p_dld = (thread_dld*)dld;

    if(p_dld)
        return p_dld->abort_by_user;

    return 0;
}
/*
 *
 * constructor, here we allocate memory to be used
 * by this class, spawn the download task thread
 *
 */

http_download_mini::http_download_mini()
{
    //OS_PRINTF("[%s] start start ...\n", __func__);
    //OS_PRINTF("[%s] this is http_download_mini construtor ...!!!!!\n", __func__);
    struct thread_dld * dld = NULL;
    state = HTTP_ERR;
    dld = (struct thread_dld *) malloc(sizeof(struct thread_dld));

    if (!dld) {
        OS_PRINTF("[%s][ERROR] fail malloc...\n", __func__);
        return;
    }

    memset(dld, 0, sizeof(struct thread_dld));
    dld->state = HTTP_RECYCLE;
    dld->fd = -1;
    dld->fname = NULL;
    contex = dld;
    state = HTTP_INIT;
    dld->abort_by_user = false;
    dld->redirect_flag = false;
    dld->redirect_cnt = 0;
    dld->cookie = NULL;
    //OS_PRINTF("[%s] end end ...\n", __func__);
}
void http_download_mini::set_rsp_header(void* rsp_header)
{
	struct thread_dld * dld = (struct thread_dld *)contex;
    	dld->http_rsp_header =(HTTP_rsp_header_t *) rsp_header;
}
void http_download_mini::clean_data(void)
{
    struct thread_dld * dld;
    dld = (struct thread_dld *)contex;
    clean_arg(dld, CLEAN_ARG_JOB);
    free(dld);
}
/*
*
*
*
*
*
*
*/
http_download_mini::~http_download_mini(void)
{
    if (state == HTTP_ERR) {
        return;
    }

    clean_data();
}

/*
*
*
*
*
*
*
*/
void http_download_mini::downloader(unsigned int timeoutMS, unsigned char * abort_flag)
{
    struct thread_dld * dld = (struct thread_dld *) contex;
    int  count = 0;
    fd_set rfds, efds;
    int state = HTTP_ERR;

    if (dld->state != HTTP_CONNECTED
        &&  dld->state != HTTP_DOWNLOADING) {
        return;
    }

    u32 starttime = mtos_ticks_get();
    u32 currtime = 0;
    u32 reftime = timeoutMS / 10;
    bool abort;
    int  select_fail_num = 0;

    while (1) {
        struct timeval tv;
        int nfds, notimeout = 0;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        nfds = 0;
        notimeout = 0;
        currtime = mtos_ticks_get();

        if ((currtime - starttime) >= reftime) {
            dld->state = HTTP_ABORTED;
            return;
        }

        abort = ((abort_flag == NULL) ? false : (*abort_flag == 0 ? false : true));

        if (state == HTTP_COMPLETE) {
            dld->state = HTTP_COMPLETE;
            return;
        }
        if (state == HTTP_ABORTED || abort) {
            dld->state = HTTP_ABORTED;
            return;
        }
        if (state == HTTP_ABORTED_BY_USER) {
            dld->state = HTTP_ABORTED_BY_USER;
            return;
        }

        if(dld->abort_by_user == true){
            dld->state = HTTP_ABORTED_BY_USER;
            return;
        }
#if 0
        count = http_select(dld, &rfds, &efds);

        if (count >= 0) {
			///maybe useless , for http_all_req will do select for read !
            notimeout = nfds = select(count + 1, &rfds, NULL, &efds, &tv);

            if (nfds > 0) {
                state = http_all_req(dld, &rfds, &efds);

                if (state != -1) {
                    dld->state = (http_state)state;
                }
            } else if (nfds < 0) {
                OS_PRINTF("[%s]>>>[ERROR][ERROR][ERROR]select error !!!!!<<<\n", __func__);
                OS_PRINTF("[%s] >>>You Should Not Come Here!!!!!<<<<\n", __func__);
                mtos_task_sleep(50);
                continue;
            }
            else
            {
                if(ne_request_remain(dld->req) > 0){
                    state = http_begin_read(dld);
                        if (state != -1) {
                            dld->state = (http_state)state;
                        }
                }else{
                    state = HTTP_COMPLETE;
                }
             }
        } else {
            mtos_task_sleep(50);
        }
#else//changed by libin ,to fix can not download whole data in iptv
        state = http_begin_read(dld);
        if (state != -1) {
            dld->state = (http_state)state;
        }
#endif
    }
}
int http_download_mini::http_connect(char * url, unsigned int timeoutMS,
    bool isPost, const char * extraHeaders, const char * body, unsigned int bodyLen)
{
    //OS_PRINTF("[%s] start start ...\n", __func__);
    ne_buffer * furl = NULL;

    if (state == HTTP_ERR) {
        OS_PRINTF("[%s][ERROR] state == HTTP_ERR !!!!\n", __func__);
	  OS_PRINTF("[%s][ERROR] fail to download !!!!\n", __func__);
        return -1;
    }

    if (url == NULL) {
        OS_PRINTF("[%s][ERROR] url == NULL !!!!\n", __func__);
        return -1;
    }

    if (strncmp(url, "http", 4) != 0) {
        OS_PRINTF("[%s][ERROR] not find http !!!!\n", __func__);
        return -1;
    }

    check_url(url);
    furl = ne_buffer_create();
    /*
    *  find a free slot
    */
    //mtos_sem_take((os_sem_t *)(&(c_mutex)), 0);
    struct thread_dld * dld = (struct thread_dld *)contex;
    //OS_PRINTF("[%s] dld->state %d\n",__func__,dld->state);
    dld->http_buffer = (char *) malloc(TMP_BUFFER_SIZE);

    if (dld->http_buffer) {
        memset(dld->http_buffer, 0, TMP_BUFFER_SIZE);
    }else{
        OS_PRINTF("[%s][ERROR] http_buffer malloc failed !!!!\n", __func__);
        return -1;
    }

    compute_md5_url(&dld->urlmd5, url);

    if (dld->http_buffer == NULL) {
        ne_buffer_destroy(furl);
        //mtos_sem_give((os_sem_t *)(&c_mutex));
        OS_PRINTF("[%s][ERROR] sorry, memalign buffer failed!!!!!!!!\n", __func__);
        return -1;
    }

    
    if (extraHeaders) {
        dld->extraHeaders = strdup1(extraHeaders); // headers is string
    } else {
        dld->extraHeaders = NULL;
    }

    if (bodyLen && body) {
        dld->httpBody = (char *) malloc(bodyLen);

        if (!dld->httpBody) {
            ne_buffer_destroy(furl);
            OS_PRINTF("[%s][ERROR] sorry, no memory\n", __func__);
            return -1;
        }

        memcpy(dld->httpBody, body, bodyLen);
        dld->bodyLen = bodyLen;
    }



    //dld->extraHeaders = NULL;
    dld->buf_offset = 0;
    ne_buffer_zappend(furl, url);
    dld->isPost = isPost;
    dld->url = furl;
    dld->code = NE_OK;
    dld->total = 0;
    dld->written = 0;
    dld->http_connect_timeout = timeoutMS / 1000;
    dld->http_read_timeout = timeoutMS / 1000;


    dld->state = (http_state)http_begin_http(dld);

    const ne_status * st = ne_get_status(dld->req);
    if(st)
    {
        http_resp_code = st->code;
    }
    else
        http_resp_code = 0;

    if (dld->state  == HTTP_ABORTED || dld->state  == HTTP_ABORTED_BY_USER || dld->state  == HTTP_TIMESOUT) {
        if (dld->state  == HTTP_ABORTED) {
            OS_PRINTF("[%s][ERROR] --------------------------HTTP_ABORTED !!!\n", __func__);
            //ne_del_cached_host1(dld->sess);
        } else if (dld->state  == HTTP_ABORTED_BY_USER) {
            OS_PRINTF("[%s][ERROR] --------------------------HTTP_ABORTED_BY_USER !!!\n", __func__);
        } else {
            OS_PRINTF("[%s][ERROR] --------------------------HTTP_TIMEOUT !!!\n", __func__);
            //ne_del_cached_host1(dld->sess);
        }
        if (dld->sess) {
            http_fill_res(dld, dld->sess);
        }

        clean_arg(dld, CLEAN_ARG_JOB);
        return -1;
    }

    if(dld->total < 0)
        return 0;
    else
        return dld->total;
}
int http_download_mini::http_receive(unsigned char *buffer, unsigned int bufferlen, int *writelen)
{
#define ERR_RET       -2
#define DONE_RET    -1
#define OK_RET        0
    struct thread_dld * dld = (struct thread_dld *)contex;
    int second_read = 0;
    *writelen = 0;

retry:
    *writelen = ne_read_response_block(dld->req, (char*)buffer, bufferlen);
    if(*writelen > 0){
        dld->state = HTTP_DOWNLOADING;
        return OK_RET;
    }else if(*writelen == 0){
        //int socket_remain = ne_get_buffered(dld->sess);
        int chunk_remain = ne_request_remain(dld->req);
        //mtos_printk(" socket_remain %d,chunk_remain %d second_read %d\n",socket_remain,chunk_remain,second_read);
        if(chunk_remain  > 0)
        {
            dld->state = HTTP_DOWNLOADING;
            return OK_RET;
        }
        else//means no data in chunk
        {
            dld->state = HTTP_COMPLETE;
            return DONE_RET;
        }
    }else{
        dld->state = HTTP_ABORTED;
        //ne_del_cached_host1(dld->sess);
        return ERR_RET;
    }
}
void http_download_mini::http_close()
{
    if(!contex)
        return;
    struct thread_dld * dld = (struct thread_dld *)contex;
    dld->code = ne_end_request(dld->req);
    http_fill_res(dld, dld->sess);
    return;
}
void http_download_mini::abort()
{
    struct thread_dld * dld = (struct thread_dld *)contex;

    dld->abort_by_user = true;

///yiyuan add this 20141128 , for  bug : 52519,
///maybe in function:download, the user have cancel the task ;
   //dld->state = HTTP_ABORTED_BY_USER;//delete by libin, to avoid multi threads write dld->state
}
/*
*
*
*
*
*
*
*/
HttpDownloadMiniSpeed http_download_mini::download(char * url, unsigned int timeoutMS, char * writeFileName, unsigned char * abort_flag,
                                 bool isPost, const char * extraHeaders, const char * body, unsigned int bodyLen, HTTP_rsp_header_t *rsp_header)
{
    //OS_PRINTF("[%s] start start ...\n", __func__);
    ne_buffer * furl = NULL;
    HttpDownloadMiniSpeed result;
 struct thread_dld * dld = (struct thread_dld *)contex;
    result.state = HTTP_MINI_ERR;
    result.size = -1;
    result.connectMS= -1;
    result.downloadMS= -1;

    dld->http_rsp_header = rsp_header;
    if (state == HTTP_ERR) {
        OS_PRINTF("[%s][ERROR] state == HTTP_ERR !!!!\n", __func__);
	  OS_PRINTF("[%s][ERROR] fail to download !!!!\n", __func__);
	if(dld->abort_by_user == true)
	{
	    	result.state = HTTP_MINI_ABORT_BY_USER;
	}
        return result;
    }

    if (url == NULL) {
        OS_PRINTF("[%s][ERROR] url == NULL !!!!\n", __func__);
	if(dld->abort_by_user == true)
	{
	    	result.state = HTTP_MINI_ABORT_BY_USER;
	}
        return result;
    }

    if (strncmp(url, "http", 4) != 0) {
        OS_PRINTF("[%s][ERROR] not find http !!!!\n", __func__);
	if(dld->abort_by_user == true)
	{
	    	result.state = HTTP_MINI_ABORT_BY_USER;
	}
        return result;
    }

    check_url(url);
    furl = ne_buffer_create();
    /*
    *  find a free slot
    */
    //mtos_sem_take((os_sem_t *)(&(c_mutex)), 0);
//    struct thread_dld * dld = (struct thread_dld *)contex;
    //OS_PRINTF("[%s] dld->state %d\n",__func__,dld->state);
    dld->http_buffer = (char *) malloc(TMP_BUFFER_SIZE);

    if (dld->http_buffer) {
        memset(dld->http_buffer, 0, TMP_BUFFER_SIZE);
    }else{
        ne_buffer_destroy(furl);
        OS_PRINTF("[%s][ERROR] http_buffer malloc failed !!!!\n", __func__);
        return result;
    }

    compute_md5_url(&dld->urlmd5, url);

    if (dld->http_buffer == NULL) {
        ne_buffer_destroy(furl);
        //mtos_sem_give((os_sem_t *)(&c_mutex));
        OS_PRINTF("[%s][ERROR] sorry, memalign buffer failed!!!!!!!!\n", __func__);
	if(dld->abort_by_user == true)
	{
	    	result.state = HTTP_MINI_ABORT_BY_USER;
	}
        return result;
    }

    if (extraHeaders) {
        dld->extraHeaders = strdup1(extraHeaders); // headers is string
    } else {
        dld->extraHeaders = NULL;
    }

    if (bodyLen && body) {
        dld->httpBody = (char *) malloc(bodyLen+1);
        if(dld->httpBody){
            memset(dld->httpBody, 0 , bodyLen+1);
        }else{
           ne_buffer_destroy(furl);
            OS_PRINTF("[%s][ERROR] httpBody malloc failed !!!!\n", __func__);
            return result;
        }

        if (!dld->httpBody) {
            ne_buffer_destroy(furl);
            OS_PRINTF("[%s][ERROR] sorry, no memory\n", __func__);
		if(dld->abort_by_user == true)
		{
		    	result.state = HTTP_MINI_ABORT_BY_USER;
		}
            return result;
        }

        memcpy(dld->httpBody, body, bodyLen);
        dld->bodyLen = bodyLen;
    }

    if (writeFileName) {
        dld->fname = ne_buffer_create();
        ne_buffer_zappend(dld->fname, writeFileName);

        if ((dld->fd = open(dld->fname->data, O_RDWR | O_CREAT | O_TRUNC, 0666), S_IRWXU) < 0) {
            OS_PRINTF("[%s][ERROR] FAIL OPEN FILE!!!!\n", __func__);
            ne_buffer_destroy(dld->fname);
		if(dld->abort_by_user == true)
		{
			result.state = HTTP_MINI_ABORT_BY_USER;
		}
		ne_buffer_destroy(furl);
            return result;
        }



    }


    //dld->extraHeaders = NULL;
    dld->buf_offset = 0;
    ne_buffer_zappend(furl, url);
    dld->isPost = isPost;
    dld->url = furl;
    dld->code = NE_OK;
    dld->total = 0;
    dld->written = 0;
    dld->http_connect_timeout = timeoutMS / 1000;
    dld->http_read_timeout = timeoutMS / 1000;

    unsigned int start = mtos_ticks_get();

    dld->state = (http_state)http_begin_http(dld);

    unsigned int end = mtos_ticks_get();

    unsigned int cost = (end - start) * 10; //ms

    if (dld->state == HTTP_ABORTED ||dld->state == HTTP_ABORTED_BY_USER) {
        if(dld->state == HTTP_ABORTED)
            //ne_del_cached_host1(dld->sess);

        if (dld->sess) {
            http_fill_res(dld, dld->sess);
        }

        clean_arg(dld, CLEAN_ARG_JOB);
        result.connectMS= cost > timeoutMS? timeoutMS : cost;

        if(dld->abort_by_user == true){
        	result.state = HTTP_MINI_ABORT_BY_USER;
		return result ;
        }

        if(dld->state == HTTP_ABORTED_BY_USER) {
            OS_PRINTF("[%s][ERROR] --------------------------HTTP_ABORTED_BY_USER !!!\n", __func__);
            result.state = HTTP_MINI_ABORT_BY_USER;
        } else if (dld->state == HTTP_ABORTED) {
            OS_PRINTF("[%s][ERROR] --------------------------HTTP_ABORTED !!!\n", __func__);
            result.state = HTTP_MINI_ABORT;
        }
        return result;
    }

    if (timeoutMS <= cost || dld->state == HTTP_TIMESOUT) {
        OS_PRINTF("[%s] --------------------------HTTP_TIMEOUT !!!\n", __func__);
        result.state = HTTP_MINI_TIMEOUT;
        result.connectMS= timeoutMS;
        result.downloadMS= 0;
	if(dld->abort_by_user == true)
	{
	    	result.state = HTTP_MINI_ABORT_BY_USER;
	}
        //ne_del_cached_host1(dld->sess);
        return result;
    }

    //because written will non zero when connecting, that is not media data
    dld->written = 0;
    start = mtos_ticks_get();
    downloader(timeoutMS - cost, abort_flag);
    end = mtos_ticks_get();

	if(dld->abort_by_user == true)
	{
	    	dld->state = HTTP_ABORTED_BY_USER;
	}

    if(dld->state == HTTP_COMPLETE)
        result.state = HTTP_MINI_COMPLETE;
    else if(dld->state == HTTP_ABORTED)
        result.state = HTTP_MINI_ABORT;
    else if(dld->state ==HTTP_ABORTED_BY_USER)
        result.state = HTTP_MINI_ABORT_BY_USER;
    else
        result.state = HTTP_MINI_TIMEOUT;
    if(result.state == HTTP_MINI_TIMEOUT || result.state == HTTP_MINI_ABORT)
    {
        //ne_del_cached_host1(dld->sess);
    }

    result.size= dld->written;
    result.connectMS= cost;
    result.downloadMS= (end - start) * 10;


    return result;
}
/*
*
*
*
*
*
*
*/
const char * http_download_mini::get_redirect_url() {
	struct thread_dld *dld;

	if (state == HTTP_ERR)
		return NULL;

	dld = (struct thread_dld *) contex;

	if (dld->redirect_flag == true){
		return dld->url->data;
       }
	return NULL;
}
/*
*
*
*add by libin ,20140508
*
*
*
*/
const char * http_download_mini::get_direct_ts_url_from_m3u8(bool * isLive)
{
    struct thread_dld * dld;
    *isLive = true;

    //OS_PRINTF("[%s] start...state %d,fd %d\n",__func__,state,fd);
    if (state == HTTP_ERR) {
        return NULL;
    }

    dld = (struct thread_dld *) contex;

    if (dld->state != HTTP_COMPLETE &&
        dld->state != HTTP_ABORTED &&
        dld->state != HTTP_ABORTED_BY_USER) {
        /* download job not finished yet */
        return NULL;
    }

    if (dld->http_buffer == NULL) {
        return NULL;
    }

    //OS_PRINTF("[%s] dld->http_buffer:\n%s\n",__func__,dld->http_buffer);
    /********************file url***************************************/
    ////check if it is m3u8 index response
    if (strstr(dld->http_buffer, "#EXTM3U") == NULL
        && strstr(dld->http_buffer, "#EXTINF") == NULL
        && strstr(dld->http_buffer, "#EXT-X-STREAM-INF") == NULL) {
        return NULL;
    }

    //skip non live streaming
    if (strstr(dld->http_buffer, "#EXT-X-ENDLIST")) {
        *isLive = false;
    }

    char * p1 = strstr(dld->http_buffer, "#EXTINF");

    if (p1 == NULL) {
        p1 = strstr(dld->http_buffer, "#EXT-X-STREAM-INF");
    }

    if (p1 == NULL) {
        return NULL;
    }

    while (*p1 != 0x0a) { //find new line
        p1++;
    }

    p1++;//skip 0x0a,new line
    char * p_start_fileurl = p1;
    char * p_end_fileurl = strstr(p_start_fileurl, "#EXTINF"); //find next ts file

    if (p_end_fileurl == NULL) {
        p_end_fileurl = strstr(p_start_fileurl, "#EXT-X-STREAM-INF");
    }

    if (p_end_fileurl == NULL) { //if only one ts file
        p_end_fileurl = dld->http_buffer + strlen(dld->http_buffer);
        p_end_fileurl--; // skip "\0"
    } else {
        p_end_fileurl--;    //skip #
    }

    // skip 0x0d 0x0a or 0x0a
    while (1) {
        if (*p_end_fileurl != 0x0d && *p_end_fileurl != 0x0a) {
            break;
        }

        p_end_fileurl--;
    }

    int fileurl_size = (int)(((ulong)p_end_fileurl) - ((ulong)p_start_fileurl) + 1);
    bool absolute_addr = (strstr(p_start_fileurl, "http://") == p_start_fileurl); //if start with "http://" , means it is absolute address,so we can play the file url directly

    if (absolute_addr) {
        memset(dld->url->data, 0, dld->url->length);
        dld->url->used = 1;
    } else {
        bool root_directory = (p_start_fileurl[0] == 0x2f);// if start with '/', means it is root directory
        /***********************************prefix url************************/
        char * p2 = NULL;

        if (root_directory) { ///find root directory
            p2 = strstr(dld->url->data + 7, "/"); //7 means skip http://
            int valid_len = (p2 - dld->url->data + 1);
            memset(dld->url->data + valid_len, 0, dld->url->length - valid_len);
            dld->url->used = valid_len;
        } else { ///find lowest directory before xxx.m3u8
            p2 = strstr(dld->url->data, ".m3u8");
            if(!p2)
                p2 = strstr(dld->url->data, ".M3U8");
            if(!p2)
                p2 = strstr(dld->url->data, ".m3U8");
            if(!p2)
                p2 = strstr(dld->url->data, ".M3u8");
            if(!p2){
                OS_PRINTF("this is not m3u8 or M3U8 url !!!!!!\n");
                *isLive = false;
                return NULL;
            }

            //find first '/' before
            while (*p2 != 0x2f) {
                p2--;
            }

            int valid_len = (p2 - dld->url->data + 1);
            memset(dld->url->data + valid_len, 0, dld->url->length - valid_len);
            dld->url->used = valid_len + 1;
        }
    }

    ne_buffer_append(dld->url, p_start_fileurl, fileurl_size);
    //OS_PRINTF("len %d,url:%s\n",fileurl_size,dld->url->data);
    return dld->url->data;
}
const char * http_download_mini::get_final_url()
{
    struct thread_dld * dld;

    if (state == HTTP_ERR) {
        return NULL;
    }

    dld = (struct thread_dld *) contex;

    if (dld->state != HTTP_COMPLETE &&
        dld->state != HTTP_ABORTED &&
        dld->state != HTTP_ABORTED_BY_USER) {
        /* download job not finished yet */
        return NULL;
    }

    if(dld->url == NULL)
        return NULL;

    return dld->url->data;
}

const char *http_download_mini::get_ContentType_from_response()
{
    struct thread_dld * dld;

    if (state == HTTP_ERR) {
        return NULL;
    }

    dld = (struct thread_dld *) contex;

    if(dld->content_type)
        return dld->content_type->data;

    return NULL;
}
