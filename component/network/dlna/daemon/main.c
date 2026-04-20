/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <cybergarage/util/clist.h>
#include <mt_cdlna_command.h>
#include <cybergarage/upnp/std/av/cdms_filesys.h>
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
#include "cybergarage/util/clog.h"
#include "satip_player.h"
#include "mt_dlna_daemon.h"

#undef OS_PRINTF
#ifdef MTDLNA_DEBUG
#define	OS_PRINTF	printf
#else
#define OS_PRINTF(...)     do{}while(0)
#endif
#define	OS_ERROR	printf

extern void* __MT_DLNA_Create_RootDev(void);
extern void* __MT_DLNA_Create_DMR(void* rt);
static struct dlna_device* g_dlna_root;
static struct dlna_device* g_dlna_dmr;
static struct dlna_device* g_dlna_dms;

static int g_msg_socket;

static satip_pg_list pg_list;

static pthread_mutex_t msgmutex;


static int __dlna_cb_meta(char *p_uri)
{
	struct mtdlna_cmdhdr chdr;
	
	pthread_mutex_lock(&msgmutex);
	OS_PRINTF("In %s Meta = %s chdr = %d\n", __func__, p_uri, sizeof(chdr));
//	return 0;
	chdr.d_cmd = MTDLNAM_META;
	chdr.d_len = strlen(p_uri) + 1;
	write(g_msg_socket, &chdr, sizeof(chdr));
	write(g_msg_socket, p_uri, chdr.d_len);
	
	pthread_mutex_unlock(&msgmutex);
	
	return 0;
}

static int __dlan_cb_seturl(char *url)
{
	struct mtdlna_cmdhdr chdr;
	
	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_URL;
	chdr.d_len = strlen(url) + 1;
	OS_PRINTF("In %s url = %s len = %d [%d]\n", __func__, url, chdr.d_len, strlen(url));
//	return 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	write(g_msg_socket, url, chdr.d_len);	
	pthread_mutex_unlock(&msgmutex);
	return 0;
}

static int __dlna_cb_play(void)
{
	struct mtdlna_cmdhdr chdr;
	
	pthread_mutex_lock(&msgmutex);
	chdr.d_cmd = MTDLNAM_PLAY;
	chdr.d_len = 0;

	OS_PRINTF("In %s do Play\n", __func__);
//	return 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	pthread_mutex_unlock(&msgmutex);
	return 0;
}

static int __dlna_cb_stop(void)
{
	struct mtdlna_cmdhdr chdr;

	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_STOP;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	
	pthread_mutex_unlock(&msgmutex);
	
	return 0;
}

static int __dlna_cb_pause(void)
{
	struct mtdlna_cmdhdr chdr;
	
	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_PAUSE;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	
	pthread_mutex_unlock(&msgmutex);
	
	return 0;
}

static int __dlna_cb_resume(void)
{
	struct mtdlna_cmdhdr chdr;
	
	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_RESUME;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	
	pthread_mutex_unlock(&msgmutex);
	
	return 0;
}

static int __dlna_cb_seek(int position)
{
	struct mtdlna_cmdhdr chdr;

	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_SEEK_POS;
	chdr.d_len = sizeof(position);
	write(g_msg_socket, &chdr, sizeof(chdr));
	write(g_msg_socket, &position, sizeof(position));
	
	pthread_mutex_unlock(&msgmutex);
	
	return 0;
}

static int __dlna_cb_getpos(void)
{
	int pos = 0;
	struct mtdlna_cmdhdr chdr;	

	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_GET_POS;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	read(g_msg_socket, &chdr, sizeof(chdr));	
	if (chdr.d_cmd == MTDLNAM_ACK) {
		read(g_msg_socket, &pos, sizeof(pos));
	}
	
	pthread_mutex_unlock(&msgmutex);
	return pos;
}

static int __dlna_cb_setvol(int volume)
{
	struct mtdlna_cmdhdr chdr;
	
	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_SET_VOL;
	chdr.d_len = sizeof(volume);
	write(g_msg_socket, &chdr, sizeof(chdr));
	write(g_msg_socket, &volume, sizeof(volume));
	
	pthread_mutex_unlock(&msgmutex);
	return 0;
}

static int __dlna_cb_getvol(void)
{
	int volume = 0;
	struct mtdlna_cmdhdr chdr;

	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_GET_VOL;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	read(g_msg_socket, &chdr, sizeof(chdr));
	if (chdr.d_cmd == MTDLNAM_ACK) {
		read(g_msg_socket, &volume, sizeof(volume));
	}
	
	pthread_mutex_unlock(&msgmutex);
	return volume;
}

static int __dlna_cb_setmute(void)
{
	struct mtdlna_cmdhdr chdr;

	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_SET_MUTE;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));

	pthread_mutex_unlock(&msgmutex);
	return 0;

}

static int __dlna_cb_getmute(void)
{
	int mute = 0;
	struct mtdlna_cmdhdr chdr;

	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_GET_MUTE;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	read(g_msg_socket, &chdr, sizeof(chdr));
	if (chdr.d_cmd == MTDLNAM_ACK) {
		read(g_msg_socket, &mute, sizeof(mute));
	}
	
	pthread_mutex_unlock(&msgmutex);
	return mute;
}

static int __dlna_cb_gettransport_info(char *state)
{
	int transport_info = 0;
	struct mtdlna_cmdhdr chdr;
	int ret = 0;
	
	pthread_mutex_lock(&msgmutex);
	
	chdr.d_cmd = MTDLNAM_GET_TRAN;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));
	
	read(g_msg_socket, &chdr, sizeof(chdr));
	if (chdr.d_cmd == MTDLNAM_GET_TRAN_ACK) {
		read(g_msg_socket, &transport_info, sizeof(transport_info));		
		if (transport_info == 1) {			
			memcpy(state,"PLAYING",strlen("PLAYING"));
		} else if (transport_info == 2) {
			memcpy(state,"PAUSED_PLAYBACK",strlen("PAUSED_PLAYBACK"));
		} else {			
			memcpy(state,"STOPPED",strlen("STOPPED"));
	  	}
		ret = 0;
	} else {
		ret = -1;
	}
	
	pthread_mutex_unlock(&msgmutex);	
	return ret;
}
static int __dlna_cb_gettrackduration(void)
{
	int pos = 0;
	struct mtdlna_cmdhdr chdr;

	pthread_mutex_lock(&msgmutex);

	chdr.d_cmd = MTDLNAM_GET_TRADK_DUR;
	chdr.d_len = 0;
	write(g_msg_socket, &chdr, sizeof(chdr));

	read(g_msg_socket, &chdr, sizeof(chdr));	
	if (chdr.d_cmd == MTDLNAM_GET_TRADK_ACK) {
		read(g_msg_socket, &pos, sizeof(pos));
	}
	pthread_mutex_unlock(&msgmutex);

	return pos;
}


static void __dlna_set_cb(void)
{
	int sock_len, cli_fd;
	struct sockaddr_un server_address;

	if (!g_dlna_dmr)
		return;

	g_dlna_dmr->dd_setmeta = (void*)__dlna_cb_meta;
	g_dlna_dmr->dd_seturl = (void*)__dlan_cb_seturl;
	g_dlna_dmr->dd_play = (void*)__dlna_cb_play;
	g_dlna_dmr->dd_stop = (void*)__dlna_cb_stop;
	g_dlna_dmr->dd_pause = (void*)__dlna_cb_pause;
	g_dlna_dmr->dd_resume = (void*)__dlna_cb_resume;
	g_dlna_dmr->dd_seek = (void*)__dlna_cb_seek;
	g_dlna_dmr->dd_getpostion = (void*)__dlna_cb_getpos;
	g_dlna_dmr->dd_setvolume = (void*)__dlna_cb_setvol;
	g_dlna_dmr->dd_getvolume = (void*)__dlna_cb_getvol;
	g_dlna_dmr->dd_setmute = (void*)__dlna_cb_setmute;
	g_dlna_dmr->dd_getmute = (void*)__dlna_cb_getmute;
	g_dlna_dmr->dd_gettransport_info = (void*)__dlna_cb_gettransport_info;
	g_dlna_dmr->dd_gettrackduration = (void*)__dlna_cb_gettrackduration;
}


static int __dlna_get_socket(char* path)
{
	int sk, len, d_sk, ret;
	struct sockaddr_un server_address;

	sk = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sk < 0) {
		OS_PRINTF("\nIn %s create socket error!\n", __func__);
		exit(-1);
	}

	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
	server_address.sun_family = AF_UNIX;
	strcpy (server_address.sun_path, path);
	ret = bind(sk, &server_address, sizeof(server_address));
	if (ret < 0) {
		OS_PRINTF("Bind Unix socket at %s Faild\n", path);
		exit(ret);
	}

	listen(sk, 5);
	len = sizeof(server_address);
	ret = accept(sk, &server_address, &len);
	close(sk);

	return ret;

}

static char buf[4096];
static void __dlna_cmd_proc(int msg_sk, int sk)
{
	int ret;
	void* handle;
	unsigned char cmd;
	struct mtdlna_cmdhdr chdr;
	static struct mtdlna_satip_cfg satipcfg;
	char *pmsgbuf = NULL;
	while (1) {
		ret = read(sk, &chdr, sizeof(chdr));
		if (ret != sizeof(chdr))
			continue;

		OS_PRINTF("Recv cmd = %d\n", chdr.d_cmd);
		cmd = MTDLNAC_NAK;
		switch(chdr.d_cmd) {
		case MTDLNAC_START:
			g_msg_socket = msg_sk;
			__dlna_set_cb();
			ret = __MT_DLNA_Start(g_dlna_root);
			cmd = MTDLNAC_ACK;
			OS_PRINTF(" DLNA Start ret = %d\n", ret);
			break;
		case MTDLNAC_STOP:
			OS_PRINTF(" DLNA Stop\n");
			__MT_DLNA_Stop(g_dlna_root);
			cmd = MTDLNAC_ACK;
			break;
		case MTDLNAC_EXIT:
			OS_PRINTF("DLNA Exit\n");
			chdr.d_cmd = MTDLNAC_ACK;
			write(sk, &chdr, sizeof(chdr));
			chdr.d_cmd = MTDLNAC_EXIT;
			write(g_msg_socket, &chdr, sizeof(chdr));
			sleep(1);
			exit(0);
			break;
		case MTDLNAC_CREATE_ROOT:
			OS_PRINTF("DLNA Create Root\n");
			g_dlna_root = __MT_DLNA_Create_RootDev();
			cmd = MTDLNAC_ACK;
			break;
		case MTDLNAC_CREATE_DMR:
			g_dlna_dmr = __MT_DLNA_Create_DMR(g_dlna_root);
			OS_PRINTF("DLNA Create DMR\n");
			cmd = MTDLNAC_ACK;
			break;
		case MTDLNAC_CREATE_DMS:
			OS_PRINTF("DLNA Create DMS\n");
			g_dlna_dms = __MT_DLNA_Create_DMS(g_dlna_root,NULL);			
			cmd = MTDLNAC_ACK;
			break;
		case MTDLNAC_SETNODE_ROOT:
		case MTDLNAC_SETNODE_DMR:
		case MTDLNAC_SETNODE_DMS:			
			if (chdr.d_len > 4096)
				chdr.d_len = 4096;
			ret = read(sk, buf, chdr.d_len);
			if (ret == chdr.d_len)
				OS_PRINTF("%s is set to %s\n", buf, buf + strlen(buf) + 1);
			if (chdr.d_cmd == MTDLNAC_SETNODE_ROOT)
				handle = g_dlna_root;
			else if (chdr.d_cmd == MTDLNAC_SETNODE_DMR)
				handle = g_dlna_dmr;
			else
				handle = g_dlna_dms;
			__MT_DLNA_Set_NodeValue(handle, buf, buf + strlen(buf) + 1);
			cmd = MTDLNAC_ACK;
			break;
		case NTDLNAC_SATIP_INIT:
			ret = read(sk,&satipcfg,sizeof(satipcfg));
			g_dlna_dms = __MT_DLNA_Create_DMS(g_dlna_root,&satipcfg);
			cmd = MTDLNAC_ACK;
			break;
		case NTDLNAC_SATIP_SETPROGLIST:
			cmd = MTDLNAC_ACK;
			cg_log_debug_l4("chdr.d_len:%d\n",chdr.d_len);
			pmsgbuf = malloc(chdr.d_len);
			if (!pmsgbuf)
				break;
			ret = read(sk,pmsgbuf,chdr.d_len);
			MT_DLNA_Satip_Addprogramlist(g_dlna_dms,pmsgbuf,chdr.d_len);	
			free(pmsgbuf);
			break;
				
		}

		chdr.d_cmd = cmd;
		write(sk, &chdr, sizeof(chdr));

	}
}

int main(int argc,char **argv)
{
	int status;
	int sk_cmd, sk_msg;

	pthread_mutex_init(&msgmutex,NULL);
	pg_list.pg_cnt = 0;
	pg_list.pglist = NULL;
	sk_msg = __dlna_get_socket(MTDLNA_MSGSOCK);
	if (sk_msg < 0) {
		fprintf(stderr,"In %s Get CMD Socket Faild\n", __func__);
		exit(0);
	}

	sk_cmd = __dlna_get_socket(MTDLNA_CMDSOCK);
	if (sk_cmd < 0) {
		fprintf(stderr,"In %s Get CMD Socket Faild\n", __func__);
		exit(0);
	}
	__dlna_cmd_proc(sk_msg, sk_cmd);
	return 0;
}



