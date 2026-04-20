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
#include "drv_adp.h"
#include "mtos_mem.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_task.h"

//#include "../HTTPC/config.h"


#define printf_debug  
#define mtos_printk  

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
#define mtos_printk  printf//peacer add for complication error
#define OS_PRINTF 

#include <ne_session.h>
#include <ne_request.h>
#include <ne_uri.h>
#include <ne_basic.h>
#include <ne_utils.h>
#include <ne_redirect.h>
#include <ne_socket.h>

#include "http_download.h"
#include "md5.h"
#include <openssl/ssl.h>


#undef MT_ASSERT
#define MT_ASSERT(__x)	\
	do{	\
		if(!(__x)) {	\
			printf("%s(%s, %d ): ASSERT(%s) failed\n", __FUNCTION__, __FILE__, __LINE__, #__x);	\
			while(1);	\
		}	\
	}while(0)

#define  CMD_SIZE   (4)
#define DOWNLOAD_BUFFER      512

#define MAX_DOWNLOADS        64
#define INTERNAL_CMD_FIFO_LEN  128

#define CLEAN_ARG_CONN       0
#define CLEAN_ARG_JOB        1
#define CLEAN_ARG_ALL        2

#define TMP_BUFFER_SIZE      4096  // multiple of pagesize

#define HTTP_CONNECT_TIMEOUT     30//peacer modify 60
#define HTTP_READ_TIMEOUT        30//peacer modify 60

// list of reserved flags used by http_download
#define HTTP_RESERVED_FLAGS      0x00ffffff
#define HTTP_DOWNLOAD_MULTI      0x01000000
#define HTTP_DOWNLOAD_ABORT      0x02000000
#define HTTP_DOWNLOAD_ABORTCLEAN      0x04000000

#define GFILE_COUNT_LIMIT       10

#define  HTTP_DOWNLOAD_STACK_SIZE  (512*1024)

static http_download * board_http_man = NULL;


#ifndef __LINUX__
extern "C" unsigned short  *   Convert_Utf8_To_Unicode(unsigned char * putf8,unsigned short * out) ;
#endif



struct thread_dld {
	// md5 of the original URL - so we can find an existing job
	// given an URL
	u32 urlmd5[4];

	// current downloading URL and save filename
	ne_buffer * volatile url;
	ne_buffer * volatile fname;

	// in case of multi file downloading, m3u files
	ne_buffer * urllist;
	ne_buffer * savelist;
	int fcount;

	unsigned long volatile flag;
	char * extraHeaders;         // any extra http headers to attach
	char * httpBody;            // http body to send (for post)
	unsigned int bodyLen;       // length of http body

	//pthread_mutex_t done_notify;
	u32  done_notify;


	ssize_t volatile written;
	ssize_t volatile total;
	http_state volatile state;

	/* file handling related */
	int      fd;
	void * p_ufs_file;
	//    gfile_t* filed; peacer del
	int64_t  timestamp;

	unsigned short volatile descriptor;

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

	/* status report */
	IDownloadEventSink * volatile report;
	void * volatile arg;

	/* mutex */
	//pthread_mutex_t lock;
	u32 lock;

};

typedef enum command_to_dld {
	COMMAND_ADD,
	COMMAND_KILL,
	COMMAND_ABORT,
	COMMAND_ABORT_CLEAN
} dld_command;

struct thread_ctx {
	struct thread_dld * array[MAX_DOWNLOADS];
	int pfds[2];

	//peacer add, use cmd fifo replace the origin pipe
	char cmd_fifo[INTERNAL_CMD_FIFO_LEN];
	u32  fifo_rd_pos;
	u32  fifo_wr_pos;

	unsigned int t_order;
};


static int  push_internal_cmd(void * param, u32 * p_cmd);
static int  pop_internal_cmd(void * param, u32 * p_cmd);
static void clean_arg(void *arg, int flag);
static void delete_file(struct thread_dld *dld);
static int   http_begin_read(struct thread_dld *dld);
static int    http_begin_http(struct thread_dld *dld);
/*
*
*
*
*
*
*/
static unsigned long combo_to_command(dld_command cmd, unsigned long index) {
	return (cmd << 16) | ((index << 16) >> 16);
}
/*
*
*
*
*
*
*/
static void command_to_combo(unsigned long cmd, dld_command *dcmd,
		unsigned long *idx) 
{
	*dcmd = (dld_command)(cmd >> 16);
	*idx = cmd & 0x0000ffff;
}
/*
*
*
*
*
*
*/
static void compute_md5_url(void * result, const char * URL) {
	MD5Context md5ctx;
	md5_init_ctx(&md5ctx);
	md5_process_bytes(URL, strlen(URL), &md5ctx);
	md5_finish_ctx(result, &md5ctx);
}
/*
*
*
*
*
*
*/
static void delete_file(struct thread_dld *dld) 
{
	if (!(dld->flag & HTTP_DOWNLOAD_ABORT ||
				dld->flag & HTTP_DOWNLOAD_ABORTCLEAN)) {
		//pthread_mutex_lock(&dld->lock);
		mtos_sem_take((os_sem_t *)(&(dld->lock)), 0);

		if (dld->fname) 
		{
			OS_PRINTF("\n%s %d  %x\n",__func__,__LINE__,dld->fname->data);
#ifdef  __LINUX__
			unlink(dld->fname->data);
#else
			if(dld->p_ufs_file)//peacer add 20130913
			{
				mtos_free(dld->p_ufs_file);
				dld->p_ufs_file = NULL;
			}

#endif
		}

		mtos_sem_give((os_sem_t *)(&(dld->lock)));
		//pthread_mutex_unlock(&dld->lock);
	}
}

/*
 *
 *
 *   the board should only have 1 http manager, that will keep track of
 *   all the http downloads on the board.
 *
 *
 */
http_download * http_download::get_http_manager(void) 
{
	if (board_http_man == NULL) 
	{
		//board_http_man = new http_download();
		OS_PRINTF("[%s] [ERROR] YOU SHOULD CALL NW_DOWNLOAD_INIT FIRSTLY!!!\n",__func__);
		return NULL;
	}

	return board_http_man;
}
/*
*
*
*
*
*
*/
http_download * http_download::get_http_manager(int prio,unsigned char *p_mem,int stack_len) 
{
	if (board_http_man == NULL) 
	{
		board_http_man = new http_download(prio,p_mem,stack_len);
	}

	return board_http_man;
}
/*
*
*
*
*
*
*/
http_download * http_download::clear_http_manager(void) 
{
	OS_PRINTF("[%s]  line%d\n ", __func__, __LINE__);
	//board_http_man->~http_download();
	//if (board_http_man == NULL) {
	//board_http_man = new http_download();
	//}
	delete board_http_man;
	board_http_man = NULL;
	return NULL ;
	//return board_http_man;
}



/*
 *
 * constructor, here we allocate memory to be used
 * by this class, spawn the download task thread
 *
 */

http_download::http_download(int task_prio, unsigned char  * p_stack_mem, int stack_size) 
{
	OS_PRINTF("[%s] start start ...\n", __func__);

	OS_PRINTF("[%s] this is http_download construtor ...!!!!!\n", __func__);
	
	struct thread_ctx *ctx = NULL;
	int i = 0, j = 0;
	struct thread_dld * dld = NULL;
	struct thread_dld *darray = NULL;
	state = HTTP_ERR;
	descriptor = 0;
	MT_BOOL  ret  = mtos_sem_create((os_sem_t *)(&(this->c_mutex)), TRUE);
	MT_ASSERT(ret == TRUE);
	darray = (struct thread_dld*) mtos_malloc(MAX_DOWNLOADS * sizeof(struct thread_dld));

	if (!darray) {
		mtos_printk("[%s][ERROR] fail malloc...\n", __func__);
		return;
	}

	memset(darray, 0, MAX_DOWNLOADS*sizeof(struct thread_dld));

	for (i = 0; i < MAX_DOWNLOADS; i++) 
	{
	
		dld = &darray[i];
		dld->state = HTTP_RECYCLE;
#ifdef __LINUX__
		dld->fd = -1;
#else
		dld->p_ufs_file = NULL;
#endif
		ret  = mtos_sem_create((os_sem_t *)(&(dld->done_notify)), TRUE);
		MT_ASSERT(ret == TRUE);
		
		ret  = mtos_sem_create((os_sem_t *)(&(dld->lock)), TRUE);
		MT_ASSERT(ret == TRUE);
	}
	

	//we have only one download thread
	for (i = 0; i < HTTP_N_THREADS; i++) 
	{
		ctx = (struct thread_ctx*) mtos_malloc(sizeof(struct thread_ctx));

		if (ctx == NULL) 
		{
			mtos_free(darray);
			return;
		}

		for (j = 0; j < MAX_DOWNLOADS; j++) {
			ctx->array[j] = &darray[j];
		}

		memset(ctx->cmd_fifo, 0, INTERNAL_CMD_FIFO_LEN);
		ctx->fifo_rd_pos = (u32)(ctx->cmd_fifo);
		ctx->fifo_wr_pos = (u32)(ctx->cmd_fifo);
		contex[i] = ctx;
		ctx->t_order = i;
		
#ifdef __LINUX__

		if (mtos_task_create((u8 *)"download daemon", downloader, (void *)ctx, 0, 0, 0) == 0) 
		{

			mtos_sem_destroy((os_sem_t *)(&(c_mutex)), 0);
			for (i = 0; i < MAX_DOWNLOADS; i++) 
			{
			
				dld = &darray[i];
				os_sem_t * p_tmp = NULL;
				p_tmp = (os_sem_t *)(&(dld->done_notify));
				if(p_tmp)
				{
					mtos_sem_destroy((os_sem_t *)(p_tmp), 0);
				}
				
				//p_tmp = &(dld->lock);
				p_tmp = (os_sem_t *)(&(dld->lock));
				if(p_tmp)
				{
					mtos_sem_destroy((os_sem_t *)(p_tmp), 0);
				}

			}
			mtos_free(darray);
			mtos_free(ctx);
			
			return;
		}

#else

		u8 * p_stack = NULL;

		if(p_stack_mem == NULL)
		{
			p_stack = (u8*)mtos_malloc(HTTP_DOWNLOAD_STACK_SIZE);
			memset(p_stack, 0, HTTP_DOWNLOAD_STACK_SIZE);
			m_stack_start = p_stack;
			m_stack_size = HTTP_DOWNLOAD_STACK_SIZE;
			m_use_ext_heap = 0;
		}
		else
		{
			p_stack = p_stack_mem;
			m_stack_start = p_stack;
			m_use_ext_heap = 1;
			m_stack_size = stack_size;
		}

		m_task_prio = task_prio;
		
		//http_task_pstack = p_stack;
		ret = mtos_task_create((u8 *)"download daemon", downloader, (void *)ctx, m_task_prio, (u32 *)m_stack_start, m_stack_size);

		if (ret == FALSE) 
		{
			mtos_sem_destroy((os_sem_t *)(&(c_mutex)), 0);
			
			for (i = 0; i < MAX_DOWNLOADS; i++) 
			{
			
				dld = &darray[i];
				os_sem_t * p_tmp = NULL;
				p_tmp = (os_sem_t *)(&(dld->done_notify));
				if(p_tmp)
				{
					mtos_sem_destroy((os_sem_t *)(p_tmp), 0);
				}
				
				p_tmp = (os_sem_t *)(&(dld->lock));
				if(p_tmp)
				{
					mtos_sem_destroy((os_sem_t *)(p_tmp), 0);
				}

			}
			
			mtos_free(darray);
			mtos_free(ctx);
			mtos_free(p_stack);
			p_stack  = NULL;
			ctx = NULL;
			darray = NULL;
			return;
			
		}

#endif
	}

	state = HTTP_INIT;
	
	OS_PRINTF("[%s] end end ...\n", __func__);

	
}

void http_download::clean_data(void) 
{
	struct thread_ctx *ctx = NULL;
	struct thread_dld *dld[MAX_DOWNLOADS];
	void * token = NULL;
	int i = 0, j = 0;
	unsigned long cmd = 0;

	for (i = 0; i < HTTP_N_THREADS; i++) 
	{
		cmd = combo_to_command(COMMAND_KILL, 0);
		ctx = (struct thread_ctx *) contex[i];
		
		//#ifdef __LINUX__
		//write(ctx->pfds[1], &cmd, sizeof(cmd));
		push_internal_cmd((void*)ctx, (u32*)&cmd);
		
		//#endif
#ifdef __LINUX__
		pthread_join(thread_id[i], &token);
#endif

		if (i == 0) 
		{
			for (j = 0; j < MAX_DOWNLOADS; j++)
			{
				dld[j] = ctx->array[j];
			}
		}

		//#ifdef __LINUX__
		//close(ctx->pfds[1]); peacer del
		memset(ctx->cmd_fifo, 0, INTERNAL_CMD_FIFO_LEN);
		ctx->fifo_rd_pos = (u32)(ctx->cmd_fifo);
		ctx->fifo_wr_pos = (u32)(ctx->cmd_fifo);
		//#endif
		mtos_free(ctx);
		
	}

	for (i = 0; i < MAX_DOWNLOADS; i++)
	{
		if (dld[i] == NULL)
		{
			continue;
		}

		mtos_sem_take((os_sem_t *)(&(dld[i]->done_notify)), 0);
		clean_arg(dld[i], CLEAN_ARG_JOB);
		//pthread_mutex_destroy(&dld[i]->done_notify);
		mtos_sem_destroy((os_sem_t *)(&(dld[i]->done_notify)), 0);
		mtos_free(dld[i]);
	}

	mtos_sem_take((os_sem_t *)(&c_mutex), 0);
	
#ifndef __LINUX__	
////20151028, yiyuan remove this , for linux , mtos_task_delete is not use anymore 
///if want to use this file , should change thread exit info !
	mtos_task_delete(m_task_prio);
#endif
	if(m_stack_start && m_use_ext_heap == 0)
	{
		mtos_free(m_stack_start);
		m_stack_start = NULL;
		m_use_ext_heap = 0;
	}
	
	//
	mtos_sem_give((os_sem_t *)(&c_mutex));
	
	mtos_sem_destroy((os_sem_t *)(&(c_mutex)), 0);

	
}
/*
*
*
*
*
*
*
*/
http_download::~http_download(void) 
{
	if (state == HTTP_ERR)
	{
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
void http_download::prepareUnmount(void) {
	if (state == HTTP_ERR)
		return;

	state = HTTP_ERR;
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
static void http_fill_res(struct thread_dld *dld, ne_session *sess) 
{
	//OS_PRINTF("[%s] start start..\n", __func__);

	if (dld->http_res)
	{
		ne_buffer_destroy(dld->http_res);
	}

	dld->http_res = ne_buffer_ncreate(64);
	
	ne_buffer_zappend(dld->http_res, ne_get_error(sess));
	
	//OS_PRINTF("[%s] end end..\n", __func__);
	
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
static void http_fill_ctype(struct thread_dld *dld, const char *ctype) 
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
static void trimstr(char* buffer) {
	int strl = 0;
	strl = strlen(buffer);
	strl --;

	while ((strl >= 0) && (buffer[strl] == ' ' || buffer[strl] == '\r' ||
				buffer[strl] == '\n')) {
		buffer[strl] = '\0';
		strl -- ;
	}
}

/*
*
*
* 
*  RETURN VALUE:  1 means success
*                          0 means failure
*
*/
static int http_accept(struct thread_dld *dld) 
{
	const ne_uri *reuri;
	const char * value;
	const ne_status * st;
	ne_request *req;
	req = dld->req;
	st = ne_get_status(req);
	
	printf_debug("[%s] start start... \n", __func__);
	
	if (st->klass == 2) 
	{
		//int oflags;
		//OS_PRINTF("[%s] st->klass is 2.. \n", __func__);
		
		value = ne_get_response_header(req, "Content-length");

		if (value != NULL)
		{
			dld->total = atoi(value);
		}
		else
		{
			dld->total = -1;
		}

		value = ne_get_response_header(req, "Content-Type");
		if (value) 
		{
			//OS_PRINTF("[%s] fill ctype... \n", __func__);
			http_fill_ctype(dld, value);
		}

		if (dld->flag & HTTP_HEAD_ONLY) 
		{
			dld->state = HTTP_COMPLETE;
			clean_arg(dld, CLEAN_ARG_CONN);

			if (dld->report)
			{
				dld->report->onDownloadFinished(dld->arg);
			}

			return 0;
		}

		//OS_PRINTF("[%s] [%d] Downloading Starts , 0x%x !!!\n", __func__, dld->descriptor, dld);
		
		dld->state = HTTP_DOWNLOADING;
		int nfd = ne_get_socket(dld->sess);

		if (strcmp(ne_get_scheme(dld->sess), "https") != 0) 
		{
#if 0 //def __LINUX__//to be confirmed
			//if ((oflags = fcntl(nfd, F_GETFL, 0)) != -1){
			//fcntl(nfd, F_SETFL, oflags | O_NONBLOCK);
			//  }
#endif
		}

		if (ne_get_buffered(dld->sess) != 0) 
		{
			http_begin_read(dld);
		}

		printf_debug("[%s] end end 1111... \n", __func__);
		
		return 1;
		
	}
	

	if (st->klass == 3)
	{
		//OS_PRINTF("[%s] st->klass is 3 [redirect encountered] \n", __func__);
		/* redirect encountered */
		dld->code = ne_end_request(req);
		reuri = ne_redirect_location(dld->sess);

		if (reuri == NULL) {
			mtos_printk("Server Error, redirect with incorrect redirect URL\n");
			goto aborted;
		}

		if (dld->flag & HTTP_NO_AUTOREDIRECT) {
			http_fill_res(dld, dld->sess);
			dld->state = HTTP_COMPLETE;

			char *p = ne_uri_unparse(reuri);
			http_fill_ctype(dld, p);
			ne_free(p);
	
			OS_PRINTF("dld redirect with no-autoredirect on %p\n", dld);
			clean_arg(dld, CLEAN_ARG_CONN);

			if (dld->report)
				dld->report->onDownloadFinished(dld->arg);

			//pthread_mutex_unlock(&dld->done_notify);
			mtos_sem_give((os_sem_t *)(&(dld->done_notify)));
			return 0;

		} else {
			/* go for redirect */
			ne_buffer_destroy(dld->url);
			dld->url = ne_buffer_create();

			char *p = ne_uri_unparse(reuri);
			ne_buffer_zappend(dld->url, p);
			ne_free(p);
			
			ne_request_destroy(dld->req);
			ne_session_destroy(dld->sess);
			dld->req = NULL;
			dld->sess = NULL;
			OS_PRINTF("redirect dld  on %p, url %s\n", dld, dld->url->data);
			http_begin_http(dld);
		}

		if ((dld->state == HTTP_ABORTED ||
					dld->state == HTTP_COMPLETE) &&
				(dld->flag & HTTP_AUTO_CLEANUP)) {
			/* time to clean it up */
			clean_arg(dld, CLEAN_ARG_JOB);
			dld->state = HTTP_RECYCLE;
		}

		//OS_PRINTF("[%s] end end 222222... \n", __func__);
		return 1;
	}

	mtos_printk("[%s]http dl aborted: %s\n",__func__, ne_get_error(dld->sess));
	http_fill_res(dld, dld->sess);
	dld->code = ne_end_request(req);

	
aborted:
	clean_arg(dld, CLEAN_ARG_CONN);
	delete_file(dld);
	dld->state = HTTP_ABORTED;

	if (dld->report) {
		dld->report->onDownloadAborted(dld->arg);
	}

	mtos_sem_give((os_sem_t *)(&(dld->done_notify)));
	
	//OS_PRINTF("[%s] end end .. \n", __func__);
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

#ifdef __LINUX__

static ssize_t write_fully(int fd, const char *buffer, ssize_t bufsize) {
	ssize_t written = 0, ret;

	printf_debug("[%s:%s:%d] ssl test reveive buf is %s \n", __FILE__,__func__,__LINE__,buffer);

	while (written < bufsize) {
		ret = write(fd, buffer + written, bufsize - written);

		if (ret > 0)
			written += ret;

		else
			return ret;
	}

	return written;
}

#else
static ssize_t write_fully(void * fp, const char *buffer, ssize_t bufsize) {
	//OS_PRINTF("[%s] fp[0x%x] bufsize:%d\n", __func__, fp, bufsize);
	ssize_t written = 0, ret;
#if 1

	while (written < bufsize) {
		if (ufs_write((ufs_file_t*)fp, (void *)(buffer + written), (bufsize - written), (u32 *)&ret) != FR_OK)
			return -1;

		if (ret > 0)
			written += ret;

		else
			return ret;
	}

#endif
	//OS_PRINTF("[%s] written:%d\n", __func__, written);
	return written;
}

#endif

/*
*
*
*
*
*
*
*/
static int http_begin_read(struct thread_dld *dld) 
{
	ssize_t len, ret;
	size_t bufed;
	size_t off;
	int64_t timestamp;
	unsigned int prevTime = 0;

	len = 0;
	ret = 0;
	bufed = 0;
	off = 0;
	timestamp = 0;
	prevTime = 0;
	
	printf_debug("[%s] start start ...\n", __func__);
	
	/*
	 * calculate offset/len of tmp buffer to download
	 */
	off = TMP_BUFFER_SIZE - dld->buf_offset;
	//yliu add:clr
	memset(dld->http_buffer + dld->buf_offset, 0, off);
	len = ne_read_response_block(dld->req, dld->http_buffer + dld->buf_offset, off);
	if (len > 0) 
	{
		dld->buf_offset += len;
		if (dld->buf_offset == TMP_BUFFER_SIZE) 
		{
		
#ifdef __LINUX__
			ret = write_fully(dld->fd, dld->http_buffer, TMP_BUFFER_SIZE);
#else
			ret = write_fully(dld->p_ufs_file, dld->http_buffer, TMP_BUFFER_SIZE);
#endif
			dld->buf_offset = 0;

			if (ret < 0) 
			{
				mtos_printk("[%s][ERROR] bad write to %p, size 4096\n", __func__,dld->http_buffer);
				goto bad_write;
			}


			//OS_PRINTF("[%s] 11 write block len[%d] \n", __func__, ret);
			dld->written += ret;
		}

		while ((bufed = ne_get_buffered(dld->sess)) != 0)
		{
			off = TMP_BUFFER_SIZE - dld->buf_offset;
			len = ne_read_response_block(dld->req, dld->http_buffer + dld->buf_offset, off);

			if (len == 0) 
			{
				mtos_printk("[%s] read response len==0, done %d\n", __func__,dld->descriptor);
				goto done;
			}

			dld->buf_offset += len;

			if (dld->buf_offset == TMP_BUFFER_SIZE) 
			{
#ifdef __LINUX__
				ret = write_fully(dld->fd, dld->http_buffer, TMP_BUFFER_SIZE);
#else
				ret = write_fully(dld->p_ufs_file, dld->http_buffer, TMP_BUFFER_SIZE);
#endif
				dld->buf_offset = 0;

				if (ret < 0)
				{
					mtos_printk("[%s][ERROR]fail write to %p, size 4096\n", __func__,dld->http_buffer);
					goto bad_write;
				}
				
				OS_PRINTF("[%s] 22 write block len[%d] \n", __func__, ret);
				dld->written += ret;
			}
		}

		if (ne_request_remain(dld->req) == 0) {
			goto done;
		}

		return 0;

	} else if (len < 0) {
		mtos_printk("####$$$$ OUCH!, CONNECTION CLOSED!!!\n");
		goto bad_write;
	}

done:

	if (dld->buf_offset) 
	{
#ifdef __LINUX__
		bufed = write_fully(dld->fd, dld->http_buffer, TMP_BUFFER_SIZE);
#else
		bufed = write_fully(dld->p_ufs_file, dld->http_buffer, TMP_BUFFER_SIZE);
#endif

		if (bufed < 0) 
		{
			mtos_printk("[%s][ERROR]WRITE fail fail fail!!!\n",__func__);
		}
	}

#ifdef __LINUX__
	truncate(dld->fname->data, dld->written + dld->buf_offset);
#endif


	if (dld->fd != -1 || dld->p_ufs_file) 
	{
		
#ifdef __LINUX__
		close(dld->fd);
		dld->fd = -1;
#else
		ufs_close((ufs_file_t*)dld->p_ufs_file);
		mtos_free(dld->p_ufs_file);
		dld->p_ufs_file = NULL;

#endif

	}

	dld->written += dld->buf_offset;
	OS_PRINTF("[%s]@@[HTTP_COMPLETE] total [%d] bytes!!@@@\n",__func__,dld->written);
	dld->state = HTTP_COMPLETE;
	dld->code = ne_end_request(dld->req);
	http_fill_res(dld, dld->sess);
	goto finished;

	
bad_write:
	mtos_printk("[%s] write error encountered!\n",__func__);
	dld->state = HTTP_ABORTED;
	dld->code = ne_end_request(dld->req);
	http_fill_res(dld, dld->sess);
	goto finished;

	
finished:
	
	//OS_PRINTF("[%s] download done for %d, %s, %p\n", __func__,dld->descriptor, dld->url->data, dld);
	clean_arg(dld, CLEAN_ARG_CONN);

	if (dld->state == HTTP_COMPLETE && dld->report)
	{
		dld->report->onDownloadFinished(dld->arg);
	}

	else if (dld->state == HTTP_ABORTED) 
	{
		delete_file(dld);
		if (dld->report)
		{
			dld->report->onDownloadAborted(dld->arg);
		}
	}

	//pthread_mutex_unlock(&dld->done_notify);
	mtos_sem_give((os_sem_t *)(&(dld->done_notify)));

	if (dld->flag & HTTP_AUTO_CLEANUP) 
	{
		/* time to clean it up */
		clean_arg(dld, CLEAN_ARG_JOB);
		dld->state = HTTP_RECYCLE;
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
// find if "Content-Type" is already defined in extra headers
static int http_find_contenttype(char * extraHeaders) {
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
static int my_verification(void *userdata, int failures, const ne_ssl_certificate *cert) {
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

	if (id)
		OS_PRINTF("Certificate issed for %s\n", id);

	dn = ne_ssl_readable_dname(ne_ssl_cert_subject(cert));

	if (dn) {
		OS_PRINTF("Subject: %s\n", dn);
		mtos_free(dn);
	}

	dn = ne_ssl_readable_dname(ne_ssl_cert_issuer(cert));

	if (dn) {
		OS_PRINTF("Issuer: %s\n", dn);
		mtos_free(dn);
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
static void http_initialize_certs(ne_session *sess) {
	//peacer add
	//maybe in future, we will support ssl based on certificates !!!!!!
#if 0//peacer del, and please don't remove these code
	char * keydirs[2];
	char path[512];
	DIR *dir;
	struct dirent *entry;
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
				ne_ssl_certificate *ca = ne_ssl_cert_read(path);

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
#ifdef  __LINUX__
  //ne_ssl_set_verify(sess, ne_ssl_verify_cb, NULL);
	//int ret = ne_ssl_trust_keypair(sess,"./client.crt","./client.key","yiyuan") ;
#else
	int ret  = ne_ssl_trust_keypair(sess,certfile,certkey,pw);
 //int ret  = ne_ssl_trust_keypair(sess,"client.crt","client.key","yiyuan") ;
#endif	
	OS_PRINTF(" ret = : %d\n", 0);
	
#endif
	ne_ssl_set_verify(sess, my_verification, NULL);
}

/*
*
*
*
*
*
*/
static int http_begin_http(struct thread_dld *dld) 
{
	ne_uri uri ;
	ne_buffer * reqbuf = NULL;
	memset(&uri,0,sizeof(ne_uri));
	
	printf_debug("[%s] start start ...\n", __func__);

	if (ne_uri_parse(dld->url->data, &uri)) 
	{
		mtos_printk("[%s][ERROR] unable to parse dld url\n",__func__);
		goto aborted;
	}

	if (uri.scheme == NULL)
	{
		uri.scheme = ne_strndup("http", 4);
	}

	if (strcmp(uri.scheme, "https") == 0) 
	{
		OS_PRINTF("[%s] >>this is https!!!<<\n", __func__);
		#ifdef  CONFIG_MT_ENABLE_OPEN_SSL
		SSLeay_add_ssl_algorithms();
		#endif
	}

	printf_debug("[%s] ==url->data:%s!!!\n", __func__, dld->url->data);

	if (uri.port == 0)
	{
		uri.port = ne_uri_defaultport(uri.scheme);
	}

	dld->sess = ne_session_create(uri.scheme, uri.host, uri.port);

	if (strcmp(uri.scheme, "https") == 0) 
	{
		// secure layer HTTP
		ne_set_session_flag(dld->sess, NE_SESSFLAG_SSLv2, 1);
		http_initialize_certs(dld->sess);
	}

	printf_debug("[%s] start set network param ...\n", __func__);
	
	ne_set_connect_timeout(dld->sess, HTTP_CONNECT_TIMEOUT);
	ne_set_read_timeout(dld->sess, HTTP_READ_TIMEOUT);
	ne_set_useragent(dld->sess, "MXXX_STB");
	ne_set_session_flag(dld->sess, NE_SESSFLAG_NOBUFFER, 1);
	ne_redirect_register(dld->sess);
	
	printf_debug("[%s] end set network param ...\n", __func__);

	//OS_PRINTF("[%s] start create request header ...\n", __func__);
	if (uri.query == NULL) 
	{
		//OS_PRINTF("[%s] uri.query is NULL...\n", __func__);
		
		if (dld->flag & HTTP_HEAD_ONLY)
		{
			dld->req = ne_request_create(dld->sess, "HEAD", uri.path);
		}
		else
		{
			if (dld->flag & HTTP_POST_METHOD) 
			{
				printf_debug("[%s] current method is POST...\n", __func__);
				dld->req = ne_request_create(dld->sess, "POST", uri.path);

				if (dld->extraHeaders) 
				{
					if (!http_find_contenttype(dld->extraHeaders)) 
					{
						// no content-type defined in POST, add default
						ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
					}

					ne_add_request_headers(dld->req, dld->extraHeaders);

				} 
				else 
				{
					OS_PRINTF("[%s] application/x-www-form-urlencoded\n",__func__);
					ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
				}

				ne_set_request_flag(dld->req, NE_REQFLAG_IDEMPOTENT, 0);

				if (dld->httpBody)
					ne_set_request_body_buffer(dld->req, dld->httpBody, dld->bodyLen);

			} 
			else 
			{
				printf_debug("[%s] current method is GET...\n", __func__);
				dld->req = ne_request_create(dld->sess, "GET", uri.path);

				if (dld->extraHeaders) 
				{
					ne_add_request_headers(dld->req, dld->extraHeaders);
				}
			}
		}

	}
	else
	{

		printf_debug("[%s] uri.query is not NULL...\n", __func__);
		reqbuf = ne_buffer_create();
		ne_buffer_concat(reqbuf, uri.path, "?", NULL);
		ne_buffer_zappend(reqbuf, uri.query);

		if (dld->flag & HTTP_HEAD_ONLY)
		{
			OS_PRINTF("[%s] method is  HEAD...\n", __func__);
			dld->req = ne_request_create(dld->sess, "HEAD", reqbuf->data);
		}
		else if (dld->flag & HTTP_POST_METHOD) 
		{
			OS_PRINTF("[%s] method is  POST...\n", __func__);
			dld->req = ne_request_create(dld->sess, "POST", reqbuf->data);

			if (dld->extraHeaders) 
			{
				if (!http_find_contenttype(dld->extraHeaders)) {
					// no content-type defined in POST, add default
					ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
				}

				ne_add_request_headers(dld->req, dld->extraHeaders);

			}
			else 
			{
				ne_add_request_header(dld->req, "Content-Type", "application/x-www-form-urlencoded");
			}

			ne_set_request_flag(dld->req, NE_REQFLAG_IDEMPOTENT, 0);

			if (dld->httpBody)
			{
				ne_set_request_body_buffer(dld->req, dld->httpBody, dld->bodyLen);
			}

		}
		else
		{
			printf_debug("[%s] method is  GET...\n", __func__);
			dld->req = ne_request_create(dld->sess, "GET", reqbuf->data);

			if (dld->extraHeaders)
			{
				ne_add_request_headers(dld->req, dld->extraHeaders);
			}
		}

		ne_buffer_destroy(reqbuf);
		
	}

	//OS_PRINTF("[%s] 2222...\n", __func__);
	printf_debug("[%s] finish to create request header ...\n", __func__);
	
	ne_add_request_header(dld->req, "Accept", "*/*");
	
	printf_debug("[%s] 33333...\n", __func__);
	printf_debug("[%s:%s:%d] start http request lalalala ...\n", __FILE__,__func__,__LINE__);
	if ((dld->code = ne_begin_request(dld->req)) == NE_OK) 
	{
		printf_debug("[%s][%d]connected , ptr %p\n",__func__, dld->descriptor, dld);
		//OS_PRINTF("[%s] 5555..\n", __func__);

		if (dld->flag & HTTP_DOWNLOAD_ABORT || dld->flag & HTTP_DOWNLOAD_ABORTCLEAN)
		{
			mtos_printk("[%s] HTTP_DOWNLOAD_ABORT 111!!!\n",__func__);
			//OS_PRINTF("[%s] 6666..dld->flag[%d]\n", __func__, dld->flag);
			goto aborted;
		}

		dld->state = HTTP_CONNECTED;
		
		printf_debug("[%s] start accept..\n", __func__);
		
		http_accept(dld);
		
		printf_debug("[%s] end accept..\n", __func__);

		if (dld->flag & HTTP_DOWNLOAD_ABORT || dld->flag & HTTP_DOWNLOAD_ABORTCLEAN) 
		{
			mtos_printk("[%s] HTTP_DOWNLOAD_ABORT 222!!!\n",__func__);
			goto aborted;
		}

		//OS_PRINTF("[%s] end http  request lalalala ...\n", __func__);
		
		ne_uri_free(&uri);
		//OS_PRINTF("[%s] end end...\n", __func__);
		
		return 0;
	}
	else
	{
		printf_debug("[%s:%s:%d] yiyuan test dld->code = %d \n", __FILE__,__func__,__LINE__,dld->code);
	}
aborted:

	mtos_printk("[%s] fail to http request lallala ...\n", __func__);
	
	ne_uri_free(&uri);

	if (dld->sess)
	{
		mtos_printk("[%s]abort download for %d, %s, error: %s\n",__func__, dld->descriptor, dld->url->data, ne_get_error(dld->sess));
	}
	
	mtos_printk("[%s] abort 111...\n", __func__);
	
	dld->state = HTTP_ABORTED;

	if (dld->sess)
	{
		http_fill_res(dld, dld->sess);
	}

	//OS_PRINTF("[%s] abort 222...\n", __func__);
	clean_arg(dld, CLEAN_ARG_CONN);
	delete_file(dld);

	if (dld->report) 
	{
		dld->report->onDownloadAborted(dld->arg);
	}

	//pthread_mutex_unlock(&dld->done_notify);
	mtos_sem_give((os_sem_t *)(&(dld->done_notify)));
	
	OS_PRINTF("[%s] end end.\n", __func__);
	return 1;
	
}


/*
*
*
*   
*
*
*/
static int http_select(struct thread_ctx *ctx, fd_set *rfds, fd_set *efds) 
{
	struct thread_dld * one = NULL;
	int max_fd = 0, nfd = 0;
	unsigned int i = 0;
	//OS_PRINTF("[%s] start start \n",__func__);
	
	FD_ZERO(rfds);
	FD_ZERO(efds);
	
	//FD_SET(ctx->pfds[0], rfds);  //peacer del
	//FD_SET(ctx->pfds[0], efds);
	//count = ctx->pfds[0];
	//yliu modify for youx geturl hold
	max_fd = 0;

	for (i = 0; i < MAX_DOWNLOADS; i++) 
	{
		
		if ((i % HTTP_N_THREADS) != ctx->t_order)
		{
			continue;
		}

		one = ctx->array[i];

		if (one == NULL)
		{
			continue;
		}

		if ((one->sess != NULL) &&  
			 (one->state == HTTP_CONNECTED
			 ||  one->state == HTTP_DOWNLOADING)) 
		{
				 
			nfd = ne_get_socket(one->sess);

			if (nfd >= 0)
			{
				FD_SET(nfd, rfds);
				FD_SET(nfd, efds);
				//tot++;
			}

			if (nfd > max_fd)
			{
				max_fd = nfd;
			}
		}
		
	}

	//OS_PRINTF("[%s] end end \n",__func__);
	return max_fd;
	
}

/*
*
*
*
*   FUNCTION:   read the command socket for data 
*
*
*/
static int http_do_command(struct thread_ctx *ctx, u32 cmd_param) 
{
	unsigned long command = 0;
	dld_command cmd;
	unsigned long index = 0;
	unsigned long i = 0;
	struct thread_dld *dld = NULL;

	command = cmd_param;

	command_to_combo(command, &cmd, &index);

	if (index >= MAX_DOWNLOADS)
	{
		mtos_printk("[%s][ERROR] index >= MAX_DOWNLOADS!!!!!\n",__func__);
		return 0;
	}

	/*
	*    <1>   add a new download task
	*/
	if (cmd == COMMAND_ADD) 
	{
		if ((index % HTTP_N_THREADS) != ctx->t_order)
		{
			mtos_printk("[%s] ouch!!!  thread order %d do no handle index %d\n", __func__,ctx->t_order,
					ctx->t_order);
		}

		dld = ctx->array[index];

		if (dld == NULL)
		{
			mtos_printk("[%s][ERROR] dld == NULL!!\n",__func__);
			return 0;
		}

		if (dld->state != HTTP_START)
		{
			mtos_printk("[%s][ERROR] dld->state != HTTP_START !!!\n",__func__);
			return 0;
		}

		if (dld->flag & HTTP_DOWNLOAD_ABORT || dld->flag & HTTP_DOWNLOAD_ABORTCLEAN) 
		{
			mtos_printk("[%s][WARNNING] job %d is aborted before starting!!\n",__func__, dld->descriptor);

			if (dld->report)
			{
				dld->report->onDownloadAborted(dld->arg);
			}

			clean_arg(dld, CLEAN_ARG_JOB);
			//pthread_mutex_unlock(&dld->done_notify);
			mtos_sem_give((os_sem_t *)(&(dld->done_notify)));

			if (dld->flag & HTTP_DOWNLOAD_ABORTCLEAN)
			{
				dld->state = HTTP_RECYCLE;
			}

			return 0;
		}

		if (dld->report)
		{
			dld->report->onDownloadStarted(dld->arg);
		}

		OS_PRINTF("[%s][OK] begin http download job !!!!!!!\n",__func__);
		
		http_begin_http(dld);

		/* if the request returns abort, cleanup */
		if ((dld->state == HTTP_ABORTED || dld->state == HTTP_COMPLETE) &&
				(dld->flag & HTTP_AUTO_CLEANUP)) 
		{
			/* time to clean it up */
			clean_arg(dld, CLEAN_ARG_JOB);
			dld->state = HTTP_RECYCLE;
		}

		return 0;
		
	}


	/*
	* <2>   abort a download task
	*/
	if (cmd == COMMAND_ABORT || cmd == COMMAND_ABORT_CLEAN) 
	{
		OS_PRINTF("[%s] abort command on %ld\n", __func__,index);

		if ((index % HTTP_N_THREADS) != ctx->t_order)
		{
			mtos_printk("[%s] ouch!!!  thread order %d do no handle index %ld\n",
					__func__,ctx->t_order, index);
		}

		dld = ctx->array[index];

		if (index != dld->descriptor)  
		{
			mtos_printk("[%s] abort command on %lu no effect\n",__func__, index);
			return 0;
		}

		if (dld->state == HTTP_RECYCLE ||
				dld->state == HTTP_ABORTED ||
				dld->state == HTTP_COMPLETE) 
		{
			if (cmd == COMMAND_ABORT)
			{
				return 0;
			}

			if (dld->state !=  HTTP_RECYCLE) 
			{
				clean_arg(dld, CLEAN_ARG_JOB);
				dld->state = HTTP_RECYCLE;
			}

			return 0;
		}

		dld->state = HTTP_ABORTED;

		if (dld->report) 
		{
			dld->report->onDownloadAborted(dld->arg);
		}

		//pthread_mutex_unlock(&dld->done_notify);
		mtos_sem_give((os_sem_t *)(&(dld->done_notify)));

		if (dld->flag & HTTP_AUTO_CLEANUP || cmd == COMMAND_ABORT_CLEAN) 
		{
			clean_arg(dld, CLEAN_ARG_JOB);
			dld->state = HTTP_RECYCLE;
		}

		return 0;
		
	}


	/*
	* <3>   kill a download  task
	*/
	if (cmd == COMMAND_KILL) 
	{
		OS_PRINTF("[%s] thread %d receive KILL\n",__func__, ctx->t_order);

		for (i = 0; i < MAX_DOWNLOADS; i++) 
		{
			if ((i % HTTP_N_THREADS) != ctx->t_order)
			{
				continue;
			}

			dld = ctx->array[i];

			if ((dld->state != HTTP_COMPLETE) &&
					(dld->state != HTTP_ABORTED) &&
					(dld->state != HTTP_RECYCLE)) 
			{
				OS_PRINTF("[%s] thread %d report abort %d\n",
					__func__,ctx->t_order, dld->descriptor);
				
				dld->state = HTTP_ABORTED;

				if (dld->report)
				{
					dld->report->onDownloadAborted(dld->arg);
				}

				clean_arg(dld, CLEAN_ARG_JOB);
				
				//pthread_mutex_unlock(&dld->done_notify);
				mtos_sem_give((os_sem_t *)(&(dld->done_notify)));
			}
		}

		return 1;
		
	}



	return 0;
	
}

/*
*
*
*  abort all active connections (due to timeout)
*
*
*
*/
static void http_all_abort(struct thread_ctx *ctx) 
{
	unsigned int i;
	struct thread_dld *dld = NULL;

	for (i = 0; i < MAX_DOWNLOADS; i++) 
	{
		if ((i % HTTP_N_THREADS) != ctx->t_order)
		{
			continue;
		}

		dld = ctx->array[i];

		if (dld->state == HTTP_RECYCLE ||
				dld->state == HTTP_COMPLETE ||
				dld->state == HTTP_ABORTED || dld->state == HTTP_START)
		{
			continue;
		}

		OS_PRINTF("[%s]aborted dl job %d due to timeout!!!!\n", __func__,dld->descriptor);
		
		dld->code = ne_end_request(dld->req);
		clean_arg(dld, CLEAN_ARG_CONN);
		delete_file(dld);
		dld->state = HTTP_ABORTED;

		if (dld->report) 
		{
			dld->report->onDownloadAborted(dld->arg);
		}

		//pthread_mutex_unlock(&dld->done_notify);
		mtos_sem_give((os_sem_t *)(&(dld->done_notify)));

		if (dld->flag & HTTP_AUTO_CLEANUP)
		{
			/* time to clean it up */
			clean_arg(dld, CLEAN_ARG_JOB);
			dld->state = HTTP_RECYCLE;
		}
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
static void http_all_req(struct thread_ctx *ctx,
		fd_set *rfds, fd_set *efds) 
{
	unsigned int i;
	int fd;
	struct thread_dld *dld;

	i=0;
	fd = -1;
	dld = NULL;

	for (i = 0; i < MAX_DOWNLOADS; i++) 
	{
		if ((i % HTTP_N_THREADS) != ctx->t_order)
		{
			continue;
		}

		dld = ctx->array[i];

		if (dld->state == HTTP_RECYCLE ||
				dld->state == HTTP_COMPLETE ||
				dld->state == HTTP_ABORTED || dld->state == HTTP_START)
			continue;

		fd = ne_get_socket(dld->sess);

		if (fd < 0)
			continue;

		if (FD_ISSET(fd, rfds) || FD_ISSET(fd, efds)) 
		{
			http_begin_read(dld);
		}
	}
}



/*
 *
 *      RETURN VALUE: 0 means sucess; -1 means fail
 *
 *
 *
 *
 *    NOTICE:  please don't add any mutex or lock to the two follwwing
 *                  functions: pop_internal_cmd() and push_internal_cmd()
 *                  for better efficency
 *
 *                  But the condition is:
 *                       <1>  don't update  wr_pos in pop function !!!!
 *                       <2>  don't update  rd_pos  in push function !!!!
 *                       <3>  rd_pos is never equal to wr_pos except the first time
 *
 *                                                            peacer 2013-07-30
 */
static int  pop_internal_cmd(void * param, u32 * p_cmd) 
{
	struct thread_ctx *ctx = (thread_ctx *)param;

	if (ctx->fifo_rd_pos != ctx->fifo_wr_pos) {
		if ((ctx->fifo_rd_pos + CMD_SIZE) <= ((u32)(ctx->cmd_fifo) + INTERNAL_CMD_FIFO_LEN )) {
			memcpy(p_cmd, (char *)(ctx->fifo_rd_pos), CMD_SIZE);
			OS_PRINTF("[%s] 11 get cmd[0x%x] OK !!!\n", __func__, *p_cmd);
			ctx->fifo_rd_pos += CMD_SIZE;
			return 0;

		} else { // (ctx->fifo_rd_pos + CMD_SIZE) ==  ((u32)(ctx->cmd_fifo) + INTERNAL_CMD_FIFO_LEN - 1))
			memcpy(p_cmd, ctx->cmd_fifo, CMD_SIZE);
			OS_PRINTF("[%s] 22 get cmd[0x%x] OK!!!\n", __func__, *p_cmd);
			ctx->fifo_rd_pos = (u32)ctx->cmd_fifo;
			return 0 ;
		}
	}

	return -1;
}

/*
*
*   RETURN VALUE:   0 means success; -1 means fail
*
*
*
*
*/
static int  push_internal_cmd(void * param, u32 * p_cmd) {
	OS_PRINTF("[%s] =start start !!!\n", __func__);
	struct thread_ctx *ctx = (thread_ctx *)param;
	 if ((ctx->fifo_wr_pos + CMD_SIZE) > ((u32)ctx->cmd_fifo) + INTERNAL_CMD_FIFO_LEN )
	 	{
	 	
	 	memcpy(ctx->cmd_fifo, p_cmd, CMD_SIZE);
				ctx->fifo_wr_pos = (u32)ctx->cmd_fifo ;
				OS_PRINTF("\n %s %d %x %x\n",__func__,__LINE__,ctx->fifo_rd_pos,ctx->fifo_wr_pos);
	 	}
	 else
	 	{
		      //OS_PRINTF("\n %s %d %d %d %d\n",__func__,__LINE__,ctx->fifo_rd_pos,ctx->fifo_wr_pos,p_cmd);
		
			memcpy((char *)(ctx->fifo_wr_pos), p_cmd, CMD_SIZE);
			ctx->fifo_wr_pos += CMD_SIZE;
			OS_PRINTF("\n %s %d %x %x %x\n",__func__,__LINE__,ctx->fifo_rd_pos,ctx->fifo_wr_pos,p_cmd);
		}

	 return 0;

}
extern "C"  void update_BPS();
/*
*
*
*
*
*
*
*/
void http_download::downloader(void* arg) 
{
	struct thread_ctx *ctx = NULL;
	struct thread_dld **dld = NULL;
	int  count = 0;
	fd_set rfds, efds;
	ctx = (struct thread_ctx*) arg;
	dld = ctx->array;
	OS_PRINTF("[%s] Start Downloader Task   thread_ctx[0x%x]!!!!\n",__func__,ctx);
	OS_PRINTF("[%s] downloader  t_order:%d,   array:0x%x\n",__func__, ctx->t_order, ctx->array);
	u32 cmd = 0;
	
	mt_set_pthread_name(__FUNCTION__);
	while (1) 
	{
		
		struct timeval tv;
		int nfds, notimeout = 0;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		static int g_continueTimeoutNum = 0;
		nfds = 0;
		notimeout = 0;

		count = http_select(ctx, &rfds, &efds);
		if(count >= 0)
		{
			notimeout = nfds = select(count + 1, &rfds, NULL, &efds, &tv);
			if (nfds > 0) 
			{
				http_all_req(ctx, &rfds, &efds);
			}
			else if (nfds < 0) 
			{
				mtos_printk("[%s]>>>[ERROR][ERROR][ERROR]select error !!!!!<<<\n",__func__);
				mtos_printk("[%s] >>>You Should Not Come Here!!!!!<<<<\n",__func__);
				mtos_task_sleep(50);
				continue;
			}
		}
		else
		{
			//	TODO:	
			//hack for update bsp more frequently
			    //and maybe we should remove the 'update_BPS' in feture !!!!!
			     //  peacer 20140218 !!!!!!!!!!!!!!!!!!!!!!!

		
			mtos_task_sleep(100);
		}
		

		if (!notimeout) 
		{
			g_continueTimeoutNum ++;
		}
		else 
		{
			g_continueTimeoutNum = 0;
		}


		if (g_continueTimeoutNum == 60) 
		{//peacer add
			// 60s has passed without activity.  If there are active
			// net connections, we have to mannually abort them.
			// TCP will not abort idle connection for us.
			g_continueTimeoutNum = 0;
			http_all_abort(ctx);
		}
		

		/*
		*  In one second, we can't sniff any data coming in
		*  and we should try to fetch the internal command from fifo 
		*/
		cmd = 0;
		if (pop_internal_cmd(ctx, &cmd) == 0) 
		{
			http_do_command(ctx, cmd);
		}


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
static void clean_arg(void *arg, int flag) 
{
	struct thread_dld *dld = (struct thread_dld *) arg;
	/* first clean up http connection related state */
	//OS_PRINTF("[%s] start start ...\n", __func__);

	if (dld->fd != -1 || dld->p_ufs_file) 
	{
#ifdef __LINUX__
		close(dld->fd);
		dld->fd = -1;
#else
		if (dld->p_ufs_file) 
		{
			ufs_close((ufs_file_t*)dld->p_ufs_file);
			mtos_free(dld->p_ufs_file);
			dld->p_ufs_file = NULL;
		}
#endif
	}

	dld->buf_offset = 0;
	if (dld->req) 
	{
		ne_request_destroy(dld->req);
		dld->req = NULL;
	}

	if (dld->sess) {
		//OS_PRINTF("[%s] destroy sess!!\n", __func__);
		ne_session_destroy(dld->sess);
		dld->sess = NULL;
	}

	if (flag == CLEAN_ARG_CONN)
	{
		//OS_PRINTF("[%s] CLEAN_ARG_CONN!!\n", __func__);
		//OS_PRINTF("[%s] end end 111...\n", __func__);
		return;
	}

	if (dld->http_buffer) 
	{
		//OS_PRINTF("[%s] ==444 http_buffer[0x%x]==\n", __func__, dld->http_buffer);
		mtos_free(dld->http_buffer);
		dld->http_buffer = NULL;
	}

	if (dld->http_res) 
	{
		//OS_PRINTF("[%s] ==444 http_res[0x%x]==\n", __func__, dld->http_res);
		ne_buffer_destroy(dld->http_res);
		dld->http_res = NULL;
	}

	if (dld->extraHeaders) 
	{
		//OS_PRINTF("[%s] ==444 extraHeaders[0x%x]==\n", __func__, dld->extraHeaders);
		mtos_free(dld->extraHeaders);
		dld->extraHeaders = NULL;
	}

	if (dld->httpBody && dld->bodyLen) 
	{
		//OS_PRINTF("[%s]  free httpBody!!\n", __func__);
		mtos_free(dld->httpBody);
		dld->httpBody = NULL;
	}

	dld->httpBody = NULL;
	dld->bodyLen = 0;

	if (dld->url) 
	{
		//OS_PRINTF("[%s]  destroy url!!\n", __func__);
		ne_buffer_destroy(dld->url);
		dld->url = NULL;
	}

	//pthread_mutex_lock(&dld->lock);
	mtos_sem_take((os_sem_t *)(&(dld->lock)), 0);

	if (dld->fname) {
		//
		//OS_PRINTF("[%s] destroy fname!!\n", __func__);
		ne_buffer_destroy(dld->fname);
		dld->fname = NULL;
	}

	//pthread_mutex_unlock(&dld->lock);
	mtos_sem_give((os_sem_t *)(&(dld->lock)));

	if (dld->content_type) {
		//OS_PRINTF("[%s] destroy content_type!!\n", __func__);
		ne_buffer_destroy(dld->content_type);
		dld->content_type = NULL;
	}

	//OS_PRINTF("[%s] end end ...\n", __func__);
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
int http_download::lockin_slot(void) {
	int i;
	struct thread_dld *dld;
	struct thread_ctx *ctx;
	ctx = (struct thread_ctx *) contex[0];
	/* find a mtos_free slot */
	mtos_sem_take((os_sem_t *)(&c_mutex), 0);

	for (i = 0; i < MAX_DOWNLOADS; i++) {
		descriptor ++;
		dld = (struct thread_dld*)ctx->array[descriptor % MAX_DOWNLOADS];

		if (dld->state == HTTP_RECYCLE)
			break;
	}

	if (i >= MAX_DOWNLOADS) {
		mtos_sem_give((os_sem_t *)(&c_mutex));
		mtos_printk("sorry, maxed out jobs, cannot handle more download\n");
		return -1;
	}

	dld->state = HTTP_START;
	dld->descriptor = descriptor;
	mtos_sem_give((os_sem_t *)(&c_mutex));
	return descriptor;
}

/*
*
*
*
*
*
*
*/
int http_download::download(char * url, char* dir, char *filen,
			unsigned long flag, void * response,
			void *arg, const char *extraHeaders,
			const char *body, unsigned int bodyLen) 
{

		OS_PRINTF("[%s] start start ...\n", __func__);
		
		struct thread_dld *dld = NULL;
		struct thread_ctx *ctx = NULL;
		
#ifndef __LINUX__
		ufs_file_t  *ufs_fp = NULL;
#else
		int fd = -1;
#endif

		ne_buffer * fname = NULL;
		ne_buffer * furl = NULL;
		int  i = 0;
		unsigned long cmd = 0;

		if (state == HTTP_ERR) 
		{
			mtos_printk("[%s] state == HTTP_ERR !!!!\n", __func__);
			return -1;
		}

		if (url == NULL) 
		{
			mtos_printk("[%s][ERROR] url == NULL !!!!\n", __func__);
			return -3;
		}

		if (strncmp(url, "http", 4) != 0) 
		{
			mtos_printk("[%s][ERROR] not find http !!!!\n", __func__);
			return -3;
		}

		ctx = (struct thread_ctx *) contex[0];

		#if 0
		for (i = 0; i < MAX_DOWNLOADS; i++) //?????????????????????? DO WHAT ????????
		{
			dld = (struct thread_dld*)ctx->array[i];

			if (dld == NULL)
			{
				continue;
			}
		}
		#endif

		flag = flag & HTTP_RESERVED_FLAGS;

		if (!(flag & HTTP_HEAD_ONLY) && filen == NULL) 
		{
			mtos_printk("[%s][ERROR] filen == NULL !!!!\n", __func__);
			return -4;
		}

		if (!(flag & HTTP_HEAD_ONLY)) 
		{
			/* head request does not save contend to file */
			fname = ne_buffer_create();

			if (dir == NULL) 
			{
				ne_buffer_zappend(fname, filen);

			} else {
				ne_buffer_concat(fname, dir, "/", filen, NULL);
			}

#ifdef  __LINUX__

			if ((fd = open(fname->data, O_RDWR | O_CREAT | O_TRUNC), S_IRWXU) < 0) 
			{
				mtos_printk("[%s][ERROR] FAIL OPEN FILE!!!!\n", __func__);
				ne_buffer_destroy(fname);
				return -4;
			}

#else
			ufs_fp = (ufs_file_t*)mtos_malloc(sizeof(ufs_file_t));//= stream->fd;

			if (ufs_fp) {
				memset(ufs_fp, 0, sizeof(ufs_file_t));
			}

			unsigned short  path_tmp[256]={0};
			u16 * p_ufs_filename = Convert_Utf8_To_Unicode((unsigned char *)fname->data,path_tmp);
			u8 ufs_ret = 0;
			ufs_ret = ufs_open(ufs_fp, p_ufs_filename, (op_mode_t)(UFS_WRITE | UFS_CREATE_NEW_COVER));
			if((ufs_ret == FR_EXIST)||(ufs_ret == FR_WRITE_PROTECTED))
			{
				OS_PRINTF("[%s] %d ret%d\n", __func__,__LINE__,ufs_ret);
				ufs_close(ufs_fp);
				ufs_ret = ufs_open(ufs_fp, p_ufs_filename, (op_mode_t)(UFS_WRITE | UFS_CREATE_NEW_COVER));	
			}
			
			OS_PRINTF("[%s] %d ret%d\n", __func__,__LINE__,ufs_ret);
			
			if (ufs_ret != FR_OK) 
			{
				mtos_printk("[%s] fail to %s !!!\n", __func__, fname->data);
				mtos_printk("[%s] fail to open local file !!!\n", __func__);
				return 0;
			}

#endif
		}

		furl = ne_buffer_create();

		
		/*
		*  find a free slot
		*/
		mtos_sem_take((os_sem_t *)(&(c_mutex)), 0);

		for (i = 0; i < MAX_DOWNLOADS; i++) 
		{
			
			descriptor ++;
			dld = (struct thread_dld*)ctx->array[descriptor % MAX_DOWNLOADS];

			if (dld->state == HTTP_RECYCLE) 
			{
				break;
			}
		}



		if (i >= MAX_DOWNLOADS) 
		{
			mtos_sem_give((os_sem_t *)(&c_mutex));

			if (fname) 
			{
			
#ifdef  __LINUX__
				unlink(fname->data);
#endif

				ne_buffer_destroy(fname);

#ifdef __LINUX__
				close(fd);
#else
				if(ufs_fp)
				{
					ufs_close(ufs_fp);
					mtos_free(ufs_fp);
					ufs_fp = NULL;
				}
#endif

			}

			ne_buffer_destroy(furl);
			mtos_printk("[%s][ERROR] sorry, max concurrent jobs\n", __func__);
			return -1;
		}
		

		OS_PRINTF("[%s] picked dld descriptor[%d]\n", __func__, descriptor);
		
		dld->http_buffer = (char*) mtos_malloc(TMP_BUFFER_SIZE);

		if (dld->http_buffer) 
		{
			memset(dld->http_buffer, 0, TMP_BUFFER_SIZE);
		}

		compute_md5_url(&dld->urlmd5, url);

		if (dld->http_buffer == NULL) 
		{
			if (fname)
			{
#ifdef  __LINUX__
				unlink(fname->data);
#endif

				ne_buffer_destroy(fname);

#ifdef __LINUX__
				close(fd);
#else
				ufs_close(ufs_fp);
				mtos_free(ufs_fp);
				ufs_fp = NULL;
#endif
			}

			ne_buffer_destroy(furl);
			mtos_sem_give((os_sem_t *)(&c_mutex));
			mtos_printk("[%s][ERROR] sorry, memalign buffer failed\n", __func__);
			return -1;
		}

		

		if (extraHeaders) 
		{
			dld->extraHeaders = strdup(extraHeaders); // headers is string

		}
		else 
		{
			dld->extraHeaders = NULL;
		}

		if (bodyLen && body) 
		{
			dld->httpBody = (char*) mtos_malloc(bodyLen);

			if (!dld->httpBody) 
			{
				if (fname) 
				{
#ifdef  __LINUX__
					unlink(fname->data);
#endif

					ne_buffer_destroy(fname);

#ifdef __LINUX__
					close(fd);
#else
					ufs_close(ufs_fp);
					mtos_free(ufs_fp);
					ufs_fp = NULL;
#endif
				}

				ne_buffer_destroy(furl);
				mtos_sem_give((os_sem_t *)(&c_mutex));
				mtos_printk("[%s][ERROR] sorry, no memory\n",__func__);
				return -1;
			}

			memcpy(dld->httpBody, body, bodyLen);
			dld->bodyLen = bodyLen;
			
		}

		dld->buf_offset = 0;
		dld->state = HTTP_START;
		dld->descriptor = descriptor;
		
		mtos_sem_give((os_sem_t *)(&c_mutex));
		
		ne_buffer_zappend(furl, url);
		dld->url = furl;
		dld->fname = fname;
		dld->urllist = NULL;
		dld->savelist = NULL;
		dld->flag = flag;
		dld->code = NE_OK;
		dld->total = 0;
		dld->written = 0;
		
#ifdef __LINUX__
		dld->fd = fd;
#else
		dld->p_ufs_file = ufs_fp;
		OS_PRINTF("[%s] ufs ufs_fp[0x%x]\n", __func__, ufs_fp);
#endif

		dld->report = (IDownloadEventSink *)response;
		dld->arg = arg;
		//pthread_mutex_lock(&dld->done_notify);
		
		mtos_sem_take((os_sem_t *)(&(dld->done_notify)), 0);
		
		cmd = combo_to_command(COMMAND_ADD, dld->descriptor % MAX_DOWNLOADS);
		ctx = (struct thread_ctx *) contex[(dld->descriptor % MAX_DOWNLOADS) % HTTP_N_THREADS];
		//#ifdef __LINUX__
		//write(ctx->pfds[1], &cmd, sizeof(unsigned long));
		push_internal_cmd(ctx, &cmd);
		//#endif
		OS_PRINTF("[%s] end end ...\n", __func__);
		
		return dld->descriptor;
		
}
/*
*
*
*
*
*
*
*/
	int http_download::busy(int fd) {
		struct thread_dld *dld;
		struct thread_ctx *ctx;

		if (state == HTTP_ERR || (fd < 0))
			return 0;

		ctx = (struct thread_ctx *) contex[0];
		dld = ctx->array[fd % MAX_DOWNLOADS];

		if (dld->state == HTTP_RECYCLE ||
				dld->descriptor != fd ||
				dld->state == HTTP_COMPLETE ||
				dld->state == HTTP_ABORTED ||
				dld->state == HTTP_INIT) {
			return 0;
		}

		return 1;
	}
/*
*
*
*
*
*
*
*/
	const char * http_download::get_errString(int fd) {
		struct thread_dld *dld;
		struct thread_ctx *ctx;

		if (state == HTTP_ERR || (fd < 0))
			return "system error";

		ctx = (struct thread_ctx *) contex[0];
		dld = ctx->array[fd % MAX_DOWNLOADS];

		if (dld->state == HTTP_RECYCLE)
			return "download finished";

		if (dld->descriptor == fd) {
			if (dld->http_res) {
				return dld->http_res->data;
			}

			return "http download error";
		}

		return "system error";
	}
/*
*
*
*
*
*
*
*/
http_state http_download::get_status(int fd)
{
	struct thread_dld *dld;
	struct thread_ctx *ctx;

	if (state == HTTP_ERR || (fd < 0))
		return HTTP_ERR;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	if (dld->state == HTTP_RECYCLE)
		return HTTP_ERR;

	if (dld->descriptor == fd)
		return dld->state;

	return HTTP_ERR;
}
/*
*
*
*
*
*
*
*/
int http_download::finish(int fd) 
{
	struct thread_dld *dld;
	struct thread_ctx *ctx;
	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	
	if (dld->fd != -1 || dld->p_ufs_file) 
	{
	
#ifdef __LINUX__
		close(dld->fd);
		dld->fd = -1;
#else

		if (dld->p_ufs_file) 
		{
			ufs_close((ufs_file_t*)dld->p_ufs_file);
			//mtos_free(dld->p_ufs_file);//peacer del 20130913
			//dld->p_ufs_file = NULL;
		}

#endif

	}
	
	if (state == HTTP_ERR || (fd < 0))
		return -1;



	if (dld->descriptor != fd)
		return -1;

	if (dld->state == HTTP_RECYCLE) {
		return -1;
	}

	/* clean up download job */
	if (dld->state == HTTP_ABORTED ||
			dld->state == HTTP_COMPLETE) {
		OS_PRINTF("clean up fd %d\n", fd);
		clean_arg(dld, CLEAN_ARG_JOB);
		dld->state = HTTP_RECYCLE;
		return 0;
	}

	return -1;
}
/*
*
*
*
*
*
*
*/
void http_download::abort(int fd, int cleanup) {
	struct thread_dld *dld;
	struct thread_ctx *ctx;
	unsigned long cmd;

	if (state == HTTP_ERR || (fd < 0))
		return;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd%MAX_DOWNLOADS];

	if (!cleanup) {
		if (dld->state == HTTP_RECYCLE ||
				dld->state == HTTP_ABORTED ||
				dld->state == HTTP_COMPLETE ||
				dld->descriptor != fd) {
			return;
		}

		delete_file(dld);
		dld->flag |= HTTP_DOWNLOAD_ABORT;
		cmd = combo_to_command(COMMAND_ABORT, fd);

	} else {
		if (dld->state == HTTP_RECYCLE ||
				dld->descriptor != fd)
			return;

		delete_file(dld);
		dld->flag |= HTTP_DOWNLOAD_ABORTCLEAN;
		cmd = combo_to_command(COMMAND_ABORT_CLEAN, fd);
	}

	OS_PRINTF("[%s] attempt to abort download %d\n", __func__,fd);
	ctx = (struct thread_ctx *) contex[(fd%MAX_DOWNLOADS)%HTTP_N_THREADS];
	//#ifdef __LINUX__
	//write(ctx->pfds[1], &cmd, sizeof(cmd));
	push_internal_cmd(ctx, &cmd);
	//#endif

	if (!cleanup) {
		//pthread_mutex_lock(&dld->done_notify);
		//pthread_mutex_unlock(&dld->done_notify);
		mtos_sem_take((os_sem_t *)(&(dld->done_notify)), 0);
		mtos_sem_give((os_sem_t *)(&(dld->done_notify)));
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
int http_download::wait(int fd, unsigned int msec) {
	struct thread_ctx *ctx;
	struct thread_dld *dld;
	int code = 0;

	if (state == HTTP_ERR || (fd < 0))
		return -1;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	if (dld->state == HTTP_RECYCLE ||
			dld->descriptor != fd) {
		return 0;
	}

	if (msec == 0) {
		//code = pthread_mutex_lock(&dld->done_notify);
		//pthread_mutex_unlock(&dld->done_notify);
		mtos_sem_take((os_sem_t *)(&(dld->done_notify)), 0);
		mtos_sem_give((os_sem_t *)(&(dld->done_notify)));
		return 0;
		//return code;
	}

	code = mtos_sem_take((os_sem_t *)(&(dld->done_notify)), msec);
	OS_PRINTF("[%s] code:%d  msec:%d\n", __func__, code, msec);

	if (code == TRUE) {
		//pthread_mutex_unlock(&dld->done_notify);
		mtos_sem_give((os_sem_t *)(&(dld->done_notify)));
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
ssize_t http_download::get_downloaded(int fd) {
	struct thread_ctx *ctx;
	struct thread_dld *dld;

	if (state == HTTP_ERR || (fd < 0))
		return 0;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	if (dld->state == HTTP_RECYCLE ||
			dld->descriptor != fd) {
		return 0;
	}

	return dld->written;
}
/*
*
*
*
*
*
*
*/
ssize_t http_download::get_content_len(int fd) {
	struct thread_ctx *ctx;
	struct thread_dld *dld;

	if (state == HTTP_ERR || (fd < 0))
		return 0;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	if (dld->state == HTTP_RECYCLE ||
			dld->descriptor != fd) {
		return 0;
	}

	return dld->total;
}
/*
*
*
*
*
*
*
*/
http_response http_download::get_http_code(int fd) {
	struct thread_ctx *ctx;
	struct thread_dld *dld;

	if (state == HTTP_ERR || (fd < 0))
		return HTTP_INVALID;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	if (dld->descriptor != fd)
		return HTTP_INVALID;

	if (dld->state != HTTP_COMPLETE &&
			dld->state != HTTP_ABORTED) {
		/* download job not finished yet */
		return HTTP_INVALID;
	}

	if (dld->code == NE_OK)
		return HTTP_DL_OK;

	if (dld->code == NE_CONNECT)
		return HTTP_CONNECT;

	if (dld->code == NE_REDIRECT)
		return HTTP_REDIRECT;

	if (dld->code == NE_TIMEOUT)
		return HTTP_TIMEOUT;

	return HTTP_INVALID;
}
/*
*
*
*
*
*
*
*/
const char * http_download::get_redirect_url(int fd) {
	struct thread_ctx *ctx;
	struct thread_dld *dld;

	if (state == HTTP_ERR || (fd < 0))
		return NULL;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	if (dld->descriptor != fd)
		return NULL;

	if (dld->state != HTTP_COMPLETE &&
			dld->state != HTTP_ABORTED) {
		/* download job not finished yet */
		return NULL;
	}

	if (dld->code == NE_REDIRECT && dld->content_type != NULL)
		return dld->content_type->data;

	return NULL;
}
/*
*
*
*
*
*
*
*/
int http_download::get_descriptor_fromURL(const char * URL) {
	int i;
	struct thread_dld *dld;
	struct thread_ctx *ctx;
	ctx = (struct thread_ctx *) contex[0];
	u32 mymd5[4];
	compute_md5_url(&mymd5, URL);
	mtos_sem_take((os_sem_t *)(&(c_mutex)), 0);

	for (i = 0; i < MAX_DOWNLOADS; i++) {
		dld = (struct thread_dld*)ctx->array[i];

		if (dld->state == HTTP_RECYCLE || dld->state == HTTP_COMPLETE ||
				dld->state == HTTP_ABORTED)
			continue;

		if (dld->flag & HTTP_DOWNLOAD_ABORT ||
				dld->flag & HTTP_DOWNLOAD_ABORTCLEAN)
			continue;

		// this dld is not in recycle state... check its URL?
		if (memcmp(&mymd5, &dld->urlmd5, sizeof(mymd5)) == 0) {
			// match!
			mtos_sem_give((os_sem_t *)(&c_mutex));
			return dld->descriptor;
		}
	}

	mtos_sem_give((os_sem_t *)(&c_mutex));
	return -1;
}
/*
*
*
*
*
*
*
*/
const char * http_download::get_content_type(int fd) {
	struct thread_ctx *ctx;
	struct thread_dld *dld;

	if (state == HTTP_ERR || (fd < 0))
		return NULL;

	ctx = (struct thread_ctx *) contex[0];
	dld = ctx->array[fd % MAX_DOWNLOADS];

	if (dld->descriptor != fd)
		return NULL;

	if (dld->state != HTTP_COMPLETE &&
			dld->state != HTTP_ABORTED) {
		/* download job not finished yet */
		return NULL;
	}

	if (dld->code != NE_REDIRECT && dld->content_type != NULL)
		return dld->content_type->data;

	return NULL;
}
