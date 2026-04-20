/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <mt_cdlna_command.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "mt_cdlna.h"
#include "mt_type.h"
#include "satip_player.h"
#include <errno.h>

#ifdef MTDLNA_DEBUG
#define SATIP_DEBUG(fmt,arg...) printf(fmt,##arg)
#else
#define SATIP_DEBUG(fmt,arg...)
#endif
#define SATIP_WARN(fmt,arg...) printf(fmt,##arg)
#define SATIP_ERROR(fmt,arg...) printf(fmt,##arg)

#define MAX_CLIENT	4
#define MAX_RECORD	4
#define PVR_CACHE_LEN	(188*1024*10)
typedef struct __satip_dev
{
	void *proot;
	void *phttpsvr;
}satip_dev;

typedef struct __dmp_info
{
	int sockfd;
	unsigned int ip;
	unsigned char *pbuf;
	unsigned int wptr;
	unsigned int rptr;
	unsigned int len;
	pthread_mutex_t mutex;
	struct __dmp_info *pnext;
}dmp_info_st;

typedef struct __prog_rec_list
{
	pthread_t threadid;
	unsigned int prog_id;
	unsigned long rec_handle;
	int run;
	dmp_info_st head;
}prog_rec_list;

static satip_dvb_op_t satip_op_func;
static unsigned char satip_start = 0;

static prog_rec_list reclist[MAX_RECORD];
static pthread_mutex_t gmutex;

#define HTTP404_HEAD "HTTP/1.0 404 Not found\r\n"\
"Content-Length: %d\r\n"\
"Content-Type: text/html\r\n"\
"Connection: close\r\n\r\n"

static char* http_404_body = "<?xml version=\"1.0\" encoding=\"ascii\" ?>\n"\
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml10/DTD/xhtml10strict.dtd\">\n"\
"<html lang=\"en\">\n"\
"<head>\n"
"<title>Not found</title>\n"\
"</head>\n"\
"<body>\n"\
"<h1>404 Not found</h1>\n"\
"<hr />\n"\
"<a href=\"http://www.videolan.org\">VideoLAN</a>\n"\
"</body>\n"\
"</html>\n";
static dmp_info_st* dmp_info_init(void)
{
	dmp_info_st *p = (dmp_info_st*)malloc(sizeof(dmp_info_st));
	if (!p)
		return NULL;
	p->pbuf = (unsigned char*)malloc(PVR_CACHE_LEN);
	if (!p->pbuf) {
		free(p);
		return NULL;
	}
	p->pnext = NULL;
	p->wptr = p->rptr = 0;
	p->len = 0;
	pthread_mutex_init(&p->mutex,NULL);
	return p;
}

static void dmp_info_deinit(dmp_info_st *dmp)
{
	free(dmp->pbuf);
	free(dmp);
}

static int dmp_push_data(dmp_info_st *dmp,unsigned char* data,unsigned int len)
{
	int i = 0;

	for(i = 0; i < 5;i++) {
		if ( (dmp->len + len) < PVR_CACHE_LEN) {
			break;
		}
		usleep(20000);
	}
	if (i == 5) {
		SATIP_DEBUG("client buffer no spaced left\n");
		return 0;
	}
	pthread_mutex_lock(&dmp->mutex);
	if (dmp->wptr + len <= PVR_CACHE_LEN) {
			memcpy(dmp->pbuf + dmp->wptr,data,len);
			dmp->wptr = (dmp->wptr+len)%PVR_CACHE_LEN;
	} else {
			memcpy(dmp->pbuf + dmp->wptr,data,PVR_CACHE_LEN-dmp->wptr);
			memcpy(dmp->pbuf,data + (PVR_CACHE_LEN-dmp->wptr),len-(PVR_CACHE_LEN-dmp->wptr));
			dmp->wptr = len-(PVR_CACHE_LEN-dmp->wptr);
	}
	dmp->len += len;
	pthread_mutex_unlock(&dmp->mutex);
	return len;
}

static int dmp_pull_data(dmp_info_st *dmp,unsigned char* pbuf,unsigned int len)
{
	int i = 0;
	for(i = 0; i < 10;i++) {
		if ( dmp->len != 0) {
			break;
		}
		usleep(20000);
	}
	if (i == 10) {
		SATIP_DEBUG("client buffer empty\n");
		return 0;
	}
	pthread_mutex_lock(&dmp->mutex);
	if (len > dmp->len)
		len = dmp->len;
	if (dmp->rptr + len <= PVR_CACHE_LEN) {
		memcpy(pbuf,dmp->pbuf + dmp->rptr,len);
		dmp->rptr = (dmp->rptr+len)%PVR_CACHE_LEN;
	} else {
		memcpy(pbuf,dmp->pbuf+dmp->rptr,PVR_CACHE_LEN-dmp->rptr);
		memcpy(pbuf+(PVR_CACHE_LEN-dmp->rptr),dmp->pbuf,len-(PVR_CACHE_LEN-dmp->rptr));
		dmp->rptr = len-(PVR_CACHE_LEN-dmp->rptr);
	}
	dmp->len -= len;
	pthread_mutex_unlock(&dmp->mutex);
	//SATIP_DEBUG("data length:%d,return len:%d,rptr:%d\n",dmp->len,len,dmp->rptr);
	return len;
}

static int add_dmp_to_rec(prog_rec_list *prec,dmp_info_st *dmp)
{
	dmp_info_st *pnode = &prec->head;

	pthread_mutex_lock(&gmutex);
	while(pnode->pnext) {
		pnode = pnode->pnext;
	}
	pnode->pnext = dmp;
	pthread_mutex_unlock(&gmutex);
	return 0;
}

static int remove_dmp_from_rec(prog_rec_list *prec,dmp_info_st *dmp)
{
	dmp_info_st *pnode = &prec->head;

	pthread_mutex_lock(&gmutex);
	while(pnode->pnext) {
		if (pnode->pnext == dmp) {
			pnode->pnext = dmp->pnext;
			break;
		}
		pnode = pnode->pnext;
	}
	pthread_mutex_unlock(&gmutex);
	return 0;
}

static int is_dmp_exist(prog_rec_list *prec,unsigned int dmp_ip)
{
	dmp_info_st *pnode = prec->head.pnext;

	pthread_mutex_lock(&gmutex);
	while(pnode) {
		if(pnode->ip == dmp_ip) {
			pthread_mutex_unlock(&gmutex);
			return 1;
		}
		pnode = pnode->pnext;
	}
	pthread_mutex_unlock(&gmutex);
	return 0;
}

static void* record_thread(void * param)
{
	int ret = 0;
	unsigned char *pbufaddr = NULL;
	unsigned int len = 0;
	int retry = 0;
	dmp_info_st *dmp = NULL;
	prog_rec_list *prec = (prog_rec_list*)param;

	while(prec->run) {
		ret = satip_op_func.rec_acquire_buf(prec->rec_handle,&pbufaddr,&len);
		if (ret < 0 ) {
			break;
		}
		if (ret == 0) {
			usleep(100000);
			retry++;
			if (retry > 100)
				break;
			continue;
		}
		retry = 0;
		dmp = prec->head.pnext;
		while (dmp) {
			dmp_push_data(dmp,pbufaddr,len);
			dmp = dmp->pnext;
		}
        ret = satip_op_func.rec_release_buf(prec->rec_handle,pbufaddr);
		usleep(10000);
	}
	SATIP_DEBUG("record_thread exit\n");
	return NULL;
}

static prog_rec_list* get_program_record_info(satip_pg_info_t *prog)
{

	int i = 0;
	int isrecord = 0;

	pthread_mutex_lock(&gmutex);
	for(i = 0; i < MAX_RECORD;i++) {
		if (reclist[i].prog_id == prog->pg_id) {
			isrecord = 1;
			break;
		}
	}

	if (isrecord) {
		pthread_mutex_unlock(&gmutex);
		SATIP_WARN("prog id %d already record\n",prog->pg_id);
		return &reclist[i];

	}
	pthread_mutex_unlock(&gmutex);
	return NULL;
}

static void* start_recored(int sockfd,satip_pg_info_t *prog)
{
	int i = 0;
	//int isrecord = 0;
	unsigned long rechandle = (unsigned long)-1;
	int k = 0;
	pthread_t threadid;

	pthread_mutex_lock(&gmutex);

	for(i = 0; i < MAX_RECORD; i++) {
		if (reclist[i].prog_id == 0)
			break;
	}

	if (i == MAX_RECORD) {
		SATIP_ERROR("no free record resource\n");
		pthread_mutex_unlock(&gmutex);
		return NULL;
	}

	satip_op_func.nim_lock(prog);
	k = 0;
	while(k++ < 4) {
		rechandle = satip_op_func.start_rec(prog);
		if(rechandle != (unsigned long)-1) {
			break;
		}
		sleep(1);
	}
	if (rechandle == (unsigned long)-1) {
		SATIP_ERROR("start ts record failed\n");
		pthread_mutex_unlock(&gmutex);
		return NULL;
	}
	SATIP_DEBUG("start record thread\n");
	reclist[i].prog_id = prog->pg_id;
	reclist[i].rec_handle = rechandle;
	reclist[i].run = 1;
	pthread_create(&threadid, NULL, record_thread, (void*)&reclist[i]);
	reclist[i].threadid = threadid;
	pthread_mutex_unlock(&gmutex);
	return &reclist[i];

}

static void stop_record(prog_rec_list *prec)
{
	pthread_mutex_lock(&gmutex);
	if (prec->head.pnext != NULL) {
		SATIP_WARN("thers is other dmps\n");
		pthread_mutex_unlock(&gmutex);
		return;

	}
	prec->run = 0;
	prec->prog_id = 0;
	prec->head.pnext = NULL;
	pthread_join(prec->threadid,NULL);
	satip_op_func.stop_rec(prec->rec_handle);
	prec->rec_handle = (unsigned long)-1;
	pthread_mutex_unlock(&gmutex);

}
static int net_senddata(int sk, unsigned char *buf, unsigned int len)
{
	int idx, slen;
	fd_set wset;
	struct timeval timeout;
	int ret = 0;
	int maxfd  = 0;

	idx = 0;
	while ((idx < len) && satip_start) {

		FD_ZERO(&wset);
		FD_SET(sk,&wset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		maxfd = sk + 1;

		ret = select(maxfd,NULL,&wset,NULL,&timeout);
		if (ret < 0) {
			perror("select error");
			break;
		}
		else if (ret == 0) {
			continue;
		} else {
			if (FD_ISSET(sk,&wset)) {
				slen = send(sk, buf + idx, len - idx, MSG_NOSIGNAL);
				if (slen <= 0) {
					perror("net send error");
					return -1;
				}
				idx += slen;
			}
		}
	}
	return idx;
}


static int rec_send_ts(int sockfd,satip_pg_info_t *prog)
{
	#define BUF_LEN (188*1024)
	int ret = 0;
	unsigned int len = 0;
	unsigned int retry = 0;
	struct timeval to;
	static unsigned char buf[BUF_LEN];
	dmp_info_st *dmp;
	prog_rec_list *prec = NULL;
	struct sockaddr_in caddr;
	int sock_len;

	to.tv_sec = 1;
    to.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) < 0) {
		return -1;
	}

	sock_len = sizeof(caddr);
	if (getpeername(sockfd,(struct sockaddr*)&caddr,(socklen_t*)&sock_len) < 0) {
		return -1;
	}

	dmp = dmp_info_init();
	dmp->sockfd = sockfd;
	dmp->ip = caddr.sin_addr.s_addr;

	prec = start_recored(sockfd,prog);
	if (!prec) {
		SATIP_ERROR("start record failed\n");
		dmp_info_deinit(dmp);
		return -1;
	}

	if(is_dmp_exist(prec,dmp->ip) == 0) {
		SATIP_DEBUG("add  to dmp:%p,rec:%p\n",dmp,prec);
		add_dmp_to_rec(prec,dmp);
	}

	while(satip_start) {
		len = dmp_pull_data(dmp,buf,BUF_LEN);
		if (len == 0) {
			retry++;
			if (retry > 1000) {
				SATIP_ERROR("dmp_pull_data failed\n");
				break;
			}
			usleep(10000);
			continue;
		}
		retry = 0;
		//SATIP_DEBUG("send data len:%d\n",len);
		ret = net_senddata(sockfd,buf,len);
		if (ret <= 0) {
			SATIP_ERROR("send ts data to clinet failed\n");
			break;
		}
	}

	remove_dmp_from_rec(prec,dmp);
	dmp_info_deinit(dmp);
	stop_record(prec);

	SATIP_DEBUG("func:%s,line:%d\n",__FUNCTION__,__LINE__);
	return 0;
}
static int http_server_callback(int sockfd,char * reqpath)
{

#define SPLIT_STR(dst,src,chr) \
	do {\
		int len = 0;\
		char *p = strchr(src,chr);\
		if (p) {\
			len = p - src; \
			strncpy(dst,src,len);\
			dst[len] = '\0';\
			src = src + len +1;\
			}\
	}while(0)

	satip_pg_info_t prog_info;
	char buf[128];
	char http_response[1024] = {0};
	struct sockaddr_in caddr;
	int sock_len;

	SATIP_DEBUG("reqinfo:%s,sockfd:%d\n",reqpath,sockfd);   //51_666000_6875_0_0_201_202_51_201_0.ts

	memset(buf,0,128);
	memset(http_response,0,1024);

	if (strchr(reqpath,'_') == NULL) {
		goto HTTP404;
	}

	SPLIT_STR(buf,reqpath,'_');
	prog_info.pg_id = atoi(buf);
	SATIP_DEBUG("program_id:%d\n",prog_info.pg_id);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.freq = atoi(buf);
	SATIP_DEBUG("freq:%d\n",prog_info.freq);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.symb_rate = atoi(buf);
	SATIP_DEBUG("symb_rate:%d\n",prog_info.symb_rate);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.onoff_22k = atoi(buf);
	SATIP_DEBUG("onoff_22k:%d\n",prog_info.onoff_22k);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.polar = atoi(buf);
	SATIP_DEBUG("polar:%d\n",prog_info.polar);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.v_pid = atoi(buf);
	SATIP_DEBUG("v_pid:%d\n",prog_info.v_pid);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.a_pid = atoi(buf);
	SATIP_DEBUG("a_pid:%d\n",prog_info.a_pid);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.pmt_pid = atoi(buf);
	SATIP_DEBUG("pmt_pid:%d\n",prog_info.pmt_pid);

	SPLIT_STR(buf,reqpath,'_');
	prog_info.pcr_pid = atoi(buf);
	SATIP_DEBUG("pcr_pid:%d\n",prog_info.pcr_pid);

	SPLIT_STR(buf,reqpath,'.');
	prog_info.is_des = atoi(buf);
	SATIP_DEBUG("is_des:%d\n",prog_info.is_des);
	if(strncmp(reqpath,"ts",2)) {
		goto HTTP404;
	}

	sock_len = sizeof(caddr);
	if (getpeername(sockfd,(struct sockaddr*)&caddr,(socklen_t*)&sock_len) < 0) {
		return -1;
	}

	prog_rec_list *rec_info = get_program_record_info(&prog_info);
	if(rec_info) {
		if(is_dmp_exist(rec_info,caddr.sin_addr.s_addr)) {
			goto HTTP404;
		}
	}

	sprintf(http_response, "HTTP/1.1 200 OK\r\nContent-Type: video/mp2t\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n");
	send(sockfd, http_response, strlen(http_response), 0);
	rec_send_ts(sockfd,&prog_info);
	return 0;
HTTP404:
	sprintf(http_response,HTTP404_HEAD,(int)strlen(http_404_body));
	strcat(http_response,http_404_body);
	send(sockfd, http_response, strlen(http_response), 0);
	return -1;


}
void* mt_satip_init(char *ip,int port,satip_dvb_op_t *psatip_callback)
{
	SATIP_DEBUG("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
	struct mtdlna_satip_cfg cfg;
	if (ip == NULL || psatip_callback == NULL)
		return NULL;
	satip_dev *pdev = malloc(sizeof(satip_dev));
	if (!pdev)
		return NULL;
	pdev->proot = MT_DLNA_Create_RootDev();
	if (!pdev->proot) {
		SATIP_ERROR("crate dlna rootdev failed\n");
		free(pdev);
		return NULL;
	}
	memset(reclist,0,sizeof(reclist));
	satip_op_func = *psatip_callback;
	mt_httpsvr_set_callback(http_server_callback);
	pdev->phttpsvr = NULL;
	while ((pdev->phttpsvr = mt_start_httpsvr(ip,port)) == NULL) {
			port += 1;
	}
	pthread_mutex_init(&gmutex,NULL);
	SATIP_DEBUG("ts http server port:%d\n",port);
	cfg.tsserver_ip = inet_addr(ip);
	cfg.port = port;
	MT_DLNA_Send_Cmd(NTDLNAC_SATIP_INIT,&cfg,sizeof(cfg));
	return pdev;
}

void  mt_satip_setproglist(void *psatipdev,satip_pg_list *plist)
{
	SATIP_DEBUG("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
	MT_DLNA_Send_Cmd(NTDLNAC_SATIP_SETPROGLIST,plist->pglist,plist->pg_cnt*sizeof(satip_pg_info_t));
}

void mt_satip_deinit(void *pdevsatip)
{
	satip_dev *pdev = (satip_dev*)pdevsatip;
	if (pdev == NULL)
		return;
	mt_stop_httpsvr(pdev->phttpsvr);
	MT_DLNA_Delete_RootDev(pdev->proot);
	free(pdev);
}

int mt_satip_start(void *pdevsatip)
{
	satip_dev *pdev = (satip_dev*)pdevsatip;
	if (pdev == NULL)
		return -1;
	satip_start = 1;
	MT_DLNA_Start(pdev->proot);
	return 0;
}

int mt_satip_stop(void *pdevsatip)
{
	satip_dev *pdev = (satip_dev*)pdevsatip;
	if (pdev == NULL)
		return -1;
	satip_start = 0;
	MT_DLNA_Stop(pdev->proot);
	return 0;
}

void  mt_satip_set_friendlyname(void *pdevsatip,char *name ,int len)
{
	struct mtdlna_device device;
	device.dd_type = DDT_DMS;
	if(pdevsatip == NULL)
		return;

	MT_DLNA_Set_NodeValue(&device,"friendlyName",name);
}

