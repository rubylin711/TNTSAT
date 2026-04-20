#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "mt_cdlna.h"

static http_req_cb server_cb = NULL;
struct mt_svrthread {
	struct mt_svrthread* ms_next;
	pthread_t ms_thread;
};

struct mt_httpsvr {
	int	 mh_sock;
	int	 mh_port;
	unsigned	 mh_flags;
	struct mt_svrthread mh_main;
	struct mt_svrthread mh_list;
};

#define	MHF_STOP	(1 << 31)


#define MAX_RECVBUFLEN	(1 << 16)
static char g_recvbuf[MAX_RECVBUFLEN];

struct http_node {
	struct http_node* hn_next;
	char*	hn_name;
	char*	hn_value;
};

struct http_header {
	char*	hh_method;
	char*	hh_path;
	char* 	hh_version;
	struct http_node hh_list;
	struct http_node **hh_lastnode;
};


static struct http_node* __mt_allocnode(char* name, char* value)
{
	struct http_node *hn;

	if (!name || !value)
		return NULL;

	hn = malloc(sizeof(struct http_node));
	if (!hn)
		return NULL;

	hn->hn_name = malloc(strlen(name) + 1);
	if (!hn->hn_name) {
		free(hn);
		return NULL;
	}

	hn->hn_value = malloc(strlen(value) + 1);
	if (!hn->hn_value) {
		free(hn->hn_name);
		free(hn);
	}

	strcpy(hn->hn_name, name);
	strcpy(hn->hn_value, value);
	hn->hn_next = NULL;

	return hn;
}

static void __mt_freenode(struct http_node* hn)
{
	if (hn) {
		if (hn->hn_name)
			free(hn->hn_name);
		if (hn->hn_value)
			free(hn->hn_value);

		free(hn);
	}
}

static int __mt_nodeadd(struct http_header* hh, struct http_node* node)
{
	if (!hh || !node)
		return -1;

	*hh->hh_lastnode = node;
	hh->hh_lastnode = &node->hn_next;
	return 0;
}

static void __mt_freelist(struct http_header *hh)
{
	struct http_node *next, *tmp;

	if (hh) {
		next = hh->hh_list.hn_next;
		while (next) {
			tmp = next;
			next = next->hn_next;
			__mt_freenode(tmp);
		}

	}
}

#define __SKIP_VAL(p,v)	do{while(*(p) == (v)) ++(p);}while(0)
#define SKIP_SPACE(p)	__SKIP_VAL(p, ' ')

static int __mt_parse1stline(struct http_header* hh, char* buf)
{
	char* p, *p1;

	if (!hh || !buf)
		return -1;

	if (hh->hh_method || hh->hh_path || hh->hh_version)
		return -1;

	p = strstr(buf, "\r\n");
	if (!p)
		return -2;

	*p = 0;

	SKIP_SPACE(buf);
	p1 = strstr(buf, " ");
	if (!p1) {
		*p = '\r';
		return -2;
	}

	*p1 = 0;
	hh->hh_method = malloc(strlen(buf) + 1);
	if (!hh->hh_method) {
		*p = '\r';
		*p1 = ' ';
		return -3;
	}

	strcpy(hh->hh_method, buf);
	*p1 = ' ';

	buf = p1 + 1;
	SKIP_SPACE(buf);
	p1 = strstr(buf, " ");
	if (!p1) {
		*p = '\r';
		free(hh->hh_method);
		hh->hh_method = NULL;
		return -2;
	}

	*p1 = 0;
	hh->hh_path = malloc(strlen(buf) + 1);
	if (!hh->hh_path) {
		*p = '\r';
                *p1 = ' ';
		free(hh->hh_method);
                hh->hh_method = NULL;
                return -2;
	}

	strcpy(hh->hh_path, buf);

	buf = p1 + 1;
	*p1 = ' ';
	SKIP_SPACE(buf);

	hh->hh_version = malloc(strlen(buf) + 1);
	if (!hh->hh_version) {
		*p = '\r';
		free(hh->hh_method);
		hh->hh_method = NULL;
		free(hh->hh_path);
		hh->hh_path = NULL;
		return -2;
	}

	strcpy(hh->hh_version, buf);
	*p = '\r';

	return 0;
}


static int __mt_initheader(struct http_header* hh)
{
	if (!hh)
		return -1;

	memset(hh, 0, sizeof(struct http_header));
	hh->hh_lastnode = &hh->hh_list.hn_next;
	return 0;
}


static struct http_header* __mt_allocheader(void)
{
	struct http_header* hh;

	hh = malloc(sizeof(struct http_header));
	if (!hh)
		return NULL;

	__mt_initheader(hh);
	return hh;
}

static void __mt_freeheader(struct http_header* hh)
{
	if (hh) {
		if (hh->hh_method)
			free(hh->hh_method);
		if (hh->hh_path)
			free(hh->hh_path);
		if (hh->hh_version)
			free(hh->hh_version);
		__mt_freelist(hh);
	}
}

static int __mt_parseline(struct http_header* hh, char* buf)
{
	char* p_end, *p_tmp;
	char* name, *value;
	struct http_node* hnode;

	if (!hh || !buf)
		return -1;

	p_end = strstr(buf, "\r\n");
	if (!p_end)
		return -2;

	*p_end = 0;

	name = buf;
	SKIP_SPACE(name);
	p_tmp = strstr(name, ":");
	if (!p_tmp) {
		*p_end = '\r';
		return -2;
	}

	*p_tmp = 0;

	value = p_tmp + 1;
	SKIP_SPACE(value);

	hnode = __mt_allocnode(name, value);
	*p_tmp = ':';
	*p_end = '\r';

	if (!hnode)
		return -3;

	__mt_nodeadd(hh, hnode);
	return 0;
}
static int __mt_parsehead(struct http_header* hh, char* buf)
{
	char* p_end;
	int ret;

	ret = __mt_parse1stline(hh, buf);
	if (ret)
		return ret;

	p_end = strstr(buf, "\r\n");
	if (!p_end) {
		printf("\nSomething error found!!!!\n");
		return -2;
	}
	p_end += 2;
	while (1) {
		ret = __mt_parseline(hh, p_end);
		if (ret)
			break;

		p_end = strstr(p_end, "\r\n");
		if (!p_end)
			break;
		p_end += 2;
	}

	return 0;
}

/*
static int __mt_recvdata(int sk, char* buf, int len)
{
	int idx, rlen;

	idx = 0;
	while (idx < len) {
		rlen = recv(sk, buf + idx, len - idx, 0);
		if (rlen <= 0)
			return -1;

		idx += rlen;
	}

	return len;
}
*/

static int __mt_senddata(int sk, char*buf, int len)
{
	int idx, slen;

	idx = 0;
	while (idx < len) {
		slen = send(sk, buf + idx, len - idx, 0);
		if (slen <= 0)
			return -1;

		idx += slen;
	}

	return len;
}

static int __mt_recvheader(int sk, char* buf, int rlen)
{
	int idx, len, recvlen;
	char *ptr;

	idx = 0;

	while (idx < rlen) {
		len = recv(sk, buf + idx, rlen - idx, MSG_PEEK);
		if (len <= 0) {
			perror("Peek\t");
			return len;
		}

		ptr = strstr(buf, "\r\n\r\n");
		if (ptr)
			recvlen = ptr - buf - idx;
		else
			recvlen = len;
		len = recv(sk, buf + idx, recvlen, 0);
		if (len <= 0) {
			perror("Recv error:\t");
			return len;
		}

		idx += len;
		if (ptr)
			return idx;
	}

	return 0;
}

static struct mt_svrthread* __mt_allocthread(void)
{
	struct mt_svrthread* ms;

	ms = malloc(sizeof(struct mt_svrthread));
	if (ms)
		ms->ms_next = NULL;

	return ms;
}

static void __mt_addthread(struct mt_svrthread* head, struct mt_svrthread* mst)
{
	struct mt_svrthread** prve;

	if (!head || !mst)
		return;

	prve = &head->ms_next;
	while (*prve)
		prve = &((*prve)->ms_next);

	*prve = mst;
}

static void __mt_delthread(struct mt_svrthread* head)
{
	struct mt_svrthread* tmp, *p;

	if (!head)
		return;

	tmp = head->ms_next;

	while (tmp) {
		p = tmp->ms_next;
		free(tmp);
		tmp = p;
	}
}

static struct mt_httpsvr* __mt_getsvr(struct mt_httpsvr* mh)
{
	mh->mh_flags++;
	return mh;
}

static void __mt_putsvr(struct mt_httpsvr* mh)
{
	unsigned val;

	if (!mh)
		return;

	val = mh->mh_flags & ~MHF_STOP;
	if (val > 1) {
		val--;
		mh->mh_flags = (mh->mh_flags | MHF_STOP) | val;
		return;
	}

	free(mh);
}

/*
static int __open_file(int idx, char* path)
{
	DIR* pdir;
	struct dirent *ent;
	char file[512];
	int fd, index = 0;

	if (idx < 0 || !path)
		return -1;

	pdir = opendir(path);
	if (!pdir)
		return -1;

	while ((ent = readdir(pdir)) != NULL) {
		if (strcmp(ent->d_name, ".") == 0 ||
			strcmp(ent->d_name, "..") == 0)
			continue;
		if (!strstr(ent->d_name, ".mkv"))
			continue;

		if (index == idx) {
			sprintf(file, "%s/%s", path, ent->d_name);
			break;
		}

		++index;
	}

	closedir(pdir);
	if (index != idx)
		return -1;

	fd = open(file, O_RDONLY);
	return fd;

}
*/

static char g_dbuf[1024];
extern int mt_base64_decode(void * src, int len, void * dst);

static int __get_fd(char* path)
{
	int len;

	len = mt_base64_decode(path, strlen(path), g_dbuf);
	if (len < 0 || len > 1023)
		return -1;

	g_dbuf[len] = 0;

	return open(g_dbuf, O_RDONLY);;
}

static int __get_filelen(int fd)
{
	int ret;

	ret = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	return ret;
}

#define MAX_FBUFLEN	(1 << 16)

static void __send_file(int sk, int fd, int fsize)
{
	int len, rlen;
	char tmpbuf[MAX_FBUFLEN];

	while (fsize > 0) {
		len = fsize > MAX_FBUFLEN ? MAX_FBUFLEN : fsize;
		rlen = read(fd, tmpbuf, MAX_FBUFLEN);
		if (rlen < 0) {
			perror("\nRead File Error:\t");
			return;
		}
		len = __mt_senddata(sk, tmpbuf, rlen);
		if (len < 0) {
			perror("\nSend Error:\t");
			return ;
		}

		fsize -= rlen;
	}

	return ;
}
static void* __http_accept(void* arg)
{
	int sk = *(int*)arg;
	int rlen, fd;
	unsigned long long flen;
	//char buf[128];
	struct http_header *hh;

	//sprintf(buf, "The sk = %d", sk);
	//send(sk, buf, strlen(buf), 0);

	rlen = __mt_recvheader(sk, g_recvbuf, MAX_RECVBUFLEN - 1);
	if (rlen > 0) {
		g_recvbuf[rlen] = 0;
		printf("\nRecv Data:\n%s\n", g_recvbuf);
	}


	hh = __mt_allocheader();
	if (!hh) {
		close(sk);
		return NULL;
	}

	if(__mt_parsehead(hh, g_recvbuf)) {
		__mt_freeheader(hh);
		close(sk);
		return NULL;
	}
	if (strncmp(hh->hh_method,"GET",3) != 0) {
		sprintf(g_recvbuf, "HTTP/1.1 405 Method Not Allowed\r\nAllow: GET\r\n\r\n");
		send(sk, g_recvbuf, strlen(g_recvbuf), 0);
		close(sk);
		return NULL;
	}
	fd = __get_fd(hh->hh_path + 1);
	if (fd >= 0) {
		flen = __get_filelen(fd);
		sprintf(g_recvbuf, "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=\"utf-8\"\r\nContent-Length: %lld\r\n\r\n", flen);
		send(sk, g_recvbuf, strlen(g_recvbuf), 0);

		__send_file(sk, fd, flen);
		close(fd);
	} else {
		if (server_cb) {
			server_cb(sk,hh->hh_path + 1);
		}
	}
	close(sk);
	__mt_freeheader(hh);
	//printf("exit ts http clinet thread:%d\n",pthread_self());
	return NULL;
}


static void* __http_svr(void* arg)
{
	struct mt_httpsvr *mh = __mt_getsvr((struct mt_httpsvr*)arg);
	struct mt_svrthread* ms;
	struct sockaddr_in caddr;
	pthread_t thread, *p_th;
	int d_sk, len;
	fd_set rset;
	struct timeval timeout;
	int ret = 0;
	int maxfd  = 0;


	len = sizeof(caddr);
	while (!(mh->mh_flags & MHF_STOP)) {

		FD_ZERO(&rset);
		FD_SET(mh->mh_sock,&rset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		maxfd = mh->mh_sock + 1;

		ret = select(maxfd,&rset,NULL,NULL,&timeout);
		if (ret < 0) {
			perror("select error");
			break;
		}
		else if (ret == 0) {
			continue;
		}
		else {
			if (FD_ISSET(mh->mh_sock,&rset)) {
				d_sk = accept(mh->mh_sock, (struct sockaddr*)&caddr,(socklen_t*)&len);
				if (d_sk < 0) {
					continue;
				}
				ms = __mt_allocthread();
				if (!ms)
					p_th = &thread;
				else {
					__mt_addthread(&mh->mh_list, ms);
					p_th = &ms->ms_thread;
				}
				if (!pthread_create(p_th, NULL, __http_accept, (void*)&d_sk))
						pthread_detach(*p_th);
			}
		}

	}
	close(mh->mh_sock);
	__mt_delthread(&mh->mh_list);
	//__mt_putsvr(mh);
	printf("exit ts http server thread\n");
	return NULL;

}

/*
static int __mt_bind(int sk, struct sockaddr_in* addr, int port, int mxcnt)
{
	int i, ret;

	for (i = 0; i < mxcnt; ++i) {
		port += i;
		addr->sin_port = htons(port);
		ret = bind(sk, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
		if (!ret)
			return port;

		if (errno == EADDRINUSE)
			continue;

		return -1;
	}

	return -1;
}
*/

#define MT_SVRPORT_START	59600
#define MT_SVRPORT_MAXCNT	2000

void* mt_start_httpsvr(char* host,int port)
{
	int sk, ret;
	struct sockaddr_in addr;
	struct mt_httpsvr *mh;

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk < 0)
		return NULL;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (host)
		addr.sin_addr.s_addr = inet_addr(host);
	else
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	ret = bind(sk, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
	//printf("\nAddr = %X\n", addr.sin_addr.s_addr);
	//ret = __mt_bind(sk, &addr, MT_SVRPORT_START, MT_SVRPORT_MAXCNT);
	if (ret < 0) {
		perror("\nBind \t");
		printf("errno = %d\n", errno);
		close(sk);
		return NULL;
	}

	if (listen(sk, 5)) {
		printf("\nListen error!\n");
		close(sk);
		return NULL;
	}

	mh = malloc(sizeof(*mh));
	if (!mh)
		return NULL;

	memset(mh, 0, sizeof(*mh));
	mh->mh_port = port;
	mh->mh_sock = sk;

	ret = pthread_create(&mh->mh_main.ms_thread, NULL, __http_svr, (void*)mh);
	if (ret) {
		printf("\nCreate HttpSvr thread Faild!!!");
		close(sk);
		free(mh);
		return NULL;
	}

	//pthread_detach(mh->mh_main.ms_thread);

	return __mt_getsvr(mh);
}

int mt_get_svrport(void* handle)
{
	struct mt_httpsvr* mh = handle;

	if (!mh)
		return -1;

	return mh->mh_port;
}

void mt_stop_httpsvr(void* handle)
{
	struct mt_httpsvr * mh = handle;

	if (!mh)
		return;
	mh->mh_flags |= MHF_STOP;

	//pthread_cancel(mh->mh_main.ms_thread);
	pthread_join(mh->mh_main.ms_thread,NULL);

	mh->mh_sock = -1;
	__mt_putsvr(mh);
	printf("mt_stop_httpsvr done\n");
}

void mt_httpsvr_set_callback(http_req_cb callback)
{
	server_cb = callback;
}

#ifdef SVRTEST
int __mt_initsvr(char* ip, int port)
{
	int sk;
	struct sockaddr_in addr;
	pthread_t thread;

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk < 0)
		return 0;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
		printf("\n family = %d port = %d\n", addr.sin_family, port);

	if (bind(sk, (struct sockaddr*)&addr, sizeof(addr))) {
		perror("\nBind \t");
		printf("errno = %d\n", errno);
		close(sk);
		return -1;
	}
	return 0;
	if (listen(sk, 5)) {
		printf("\nListen error!\n");
		close(sk);
		return -1;
	}

	while (1) {
		struct sockaddr_in caddr;
		int d_sk, len;

		len = sizeof(caddr);
		d_sk = accept(sk, (struct sockaddr*)&caddr, &len);
		if (d_sk > 0) {
			printf("\nAccep Client ip:%s,port:%d\n",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));
			if (!pthread_create(&thread, NULL, __http_accept, (void*)&d_sk))
				pthread_detach(thread);

		}
	}

	close(sk);
	return 0;
}

int main(void)
{
	int port;
	void* handle;

	__mt_initsvr(NULL, 6886);

	handle = mt_start_httpsvr(NULL);
	if (handle) {
		port = mt_get_svrport(handle);
		printf("\nSvrPort = %d\n", port);
		while(1);
		mt_stop_httpsvr(handle);

	}
}

#endif


