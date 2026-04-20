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
#include <sys/wait.h>
#include "mt_cdlna.h"

#undef OS_PRINTF

#ifdef MTDLNA_DEBUG
#define	OS_PRINTF	printf
#else
#define OS_PRINTF(...)     do{}while(0)
#endif
#define	OS_ERROR	printf

static struct mtdlna_device g_devroot;
static struct mtdlna_device g_devdmr;
static struct mtdlna_device g_devdms;

static int g_dlan_inited = 0;
static int g_dlna_isrunn = 0;

static int g_dlna_cmdsock = -1;
static int g_dlna_msgsock = -1;

static unsigned int g_setnode_buflen = 0;
static char* g_setnode_buf = NULL;

static int(*g_setmeta)(char*);
static int(*g_seturl)(char*);
static int(*g_play)(void);
static int(*g_stop)(void);
static int(*g_pause)(void);
static int(*g_resume)(void);
static int(*g_seek_pos)(long);
static int(*g_get_transport_info)(void);
static int(*g_get_pos)(void);
static int(*g_set_vol)(int);
static int(*g_get_vol)(void);
static int(*g_set_mute)(void);
static int(*g_get_mute)(void);
static int(*g_get_track_duration)(void);

static char msg_buffer[4096];

static int dlna_daemon_started = 0;

static void __dlna_msg_proc(void* unused)
{
	int ret, len = 0;
	struct mtdlna_cmdhdr chdr;
	pthread_detach(pthread_self());

	while (1) {
		len = 0;
		ret = read(g_dlna_msgsock, &chdr, sizeof(chdr));
		if (ret != sizeof(chdr))
			continue;
		switch(chdr.d_cmd) {
		case MTDLNAM_META:
			len = chdr.d_len;
			ret = read(g_dlna_msgsock, msg_buffer, len);
			OS_PRINTF("Rcve Meta = [%s]\n", msg_buffer);
			g_setmeta(msg_buffer);
			break;
		case MTDLNAM_URL:
			len = chdr.d_len;
			ret = read(g_dlna_msgsock, msg_buffer, len);
			OS_PRINTF("Rcve url = [%s] len = %d ret = %d\n", msg_buffer, len, ret);
			g_seturl(msg_buffer);
			break;

		case MTDLNAM_PLAY:
			OS_PRINTF("Rcve Msg play\n");
			if (g_play)
				g_play();
			break;
		case MTDLNAM_STOP:
			OS_PRINTF("Recv Msg Stop\n");
			if (g_stop)
				g_stop();
			break;
		case MTDLNAM_PAUSE:
			OS_PRINTF("Recv Msg Pause\n");
			if (g_pause)
				g_pause();
			break;
		case MTDLNAM_RESUME:
			OS_PRINTF("Recv Msg Resume\n");
			if (g_resume)
				g_resume();
			break;
		case MTDLNAM_SEEK_POS:
			OS_PRINTF("Recv Msg Seek\n");
			read(g_dlna_msgsock, &len, chdr.d_len);
			if (g_seek_pos)
				g_seek_pos(len);
			break;
		case MTDLNAM_GET_POS:
			OS_PRINTF("Recv Msg Get Pos\n");
			if (g_get_pos)
				len = g_get_pos();
			chdr.d_cmd = MTDLNAM_ACK;
			write(g_dlna_msgsock, &chdr, sizeof(chdr));
			write(g_dlna_msgsock, &len, sizeof(len));
			break;
		case MTDLNAM_SET_VOL:
			OS_PRINTF("Recv Msg Set Vol\n");
			read(g_dlna_msgsock, &len, chdr.d_len);
			if (g_set_vol)
				g_set_vol(len);
			break;
		case MTDLNAM_GET_VOL:
			OS_PRINTF("Recv Msg Get Vol\n");
			if (g_get_vol)
				len = g_get_vol();
			chdr.d_cmd = MTDLNAM_ACK;
			write(g_dlna_msgsock, &chdr, sizeof(chdr));
			write(g_dlna_msgsock, &len, sizeof(len));
			break;
		case MTDLNAM_SET_MUTE:
			OS_PRINTF("Recv Msg Set Mute\n");
			if (g_set_mute)
				g_set_mute();
			break;
		case MTDLNAM_GET_MUTE:
			OS_PRINTF("Recv Msg Get Mute\n");
			if (g_get_mute)
				len = g_get_mute();
			chdr.d_cmd = MTDLNAM_ACK;
			write(g_dlna_msgsock, &chdr, sizeof(chdr));
			write(g_dlna_msgsock, &len, sizeof(len));
			break;
		case MTDLNAM_GET_TRAN://MTDLNAM_GET_TRANSPORT_INFO
			if (g_get_transport_info)
				len = g_get_transport_info();
			chdr.d_cmd = MTDLNAM_GET_TRAN_ACK;
			write(g_dlna_msgsock, &chdr, sizeof(chdr));
			write(g_dlna_msgsock, &len, sizeof(len));
			break;
		case MTDLNAM_GET_TRADK_DUR:
			//OS_PRINTF("Recv Msg Get Pos\n");
			if(g_get_track_duration)
				len = g_get_track_duration();
			chdr.d_cmd = MTDLNAM_GET_TRADK_ACK;
			write(g_dlna_msgsock, &chdr, sizeof(chdr));
			write(g_dlna_msgsock, &len, sizeof(len));

			break;
		case MTDLNAC_EXIT:
			OS_PRINTF("Recv Msg Exit:\n");
			pthread_exit(0);
			break;
		}
	}

	pthread_exit(0) ;
}


static int __dlna_send_cmd(unsigned short cmd, int vlen, void* buffer)
{
	int ret;
	struct mtdlna_cmdhdr chdr;

	chdr.d_cmd = cmd;

	if (vlen == 0 || !buffer)
		vlen = 0;
	chdr.d_len = (unsigned short)vlen;
	ret = write(g_dlna_cmdsock, &chdr, sizeof(chdr));
	if (buffer)
		write(g_dlna_cmdsock, buffer, vlen);

	read(g_dlna_cmdsock, &chdr, sizeof(chdr));
	ret = 0;
	if (chdr.d_cmd != MTDLNAC_ACK) {
		OS_PRINTF("\nNot ACK CMD <%d>\n", cmd);
		ret = -1;
	}

	return ret;
}

static void* daemon_thread(void* param)
{
	volatile int ret = 0;
	char *cmd = (char*)param;
	pid_t pid;
	int status;
	pthread_detach(pthread_self());

	pid = vfork();
	if (pid < 0) {
		return NULL;
	} else if (pid == 0) {
		ret = execlp(cmd,"",NULL);
		exit(127);
	}
	usleep(10000);
	if (ret == 0)
		dlna_daemon_started = 1;
	else
		dlna_daemon_started = 0;
	waitpid(pid,&status,WUNTRACED | WCONTINUED);
	return NULL;
}

static int __start_dlna_daemon(char* cmd)
{
	pthread_t thread1;
	pthread_create(&thread1,NULL,(void *)&daemon_thread,(void *)cmd);
	sleep(1);
	return 0;
}
void MT_DLNA_Set_NodeValue(void* handle, char* name, char* value)
{
	unsigned int nlen, vlen;
	struct mtdlna_cmdhdr *chdr;
	struct mtdlna_device* mtdev = handle;
	if (!mtdev || !name || !value)
		return;

	nlen = strlen(name);
	vlen = strlen(value);
	if (g_setnode_buflen < nlen + vlen + sizeof(*chdr)) {
		if (g_setnode_buf)
			free(g_setnode_buf);
		g_setnode_buflen = nlen + vlen + sizeof(*chdr) + 2;
		g_setnode_buf = malloc(g_setnode_buflen);
		if (!g_setnode_buf) {
			OS_ERROR("\nIn %s alloc %d Bytes faild\n", __func__, g_setnode_buflen);
			g_setnode_buflen = 0;
			return;
		}
	}
	chdr = (struct mtdlna_cmdhdr*)g_setnode_buf;
	if (mtdev->dd_type == DDT_ROOT)
		chdr->d_cmd = MTDLNAC_SETNODE_ROOT;
	else if (mtdev->dd_type == DDT_DMR)
		chdr->d_cmd = MTDLNAC_SETNODE_DMR;
	else if (mtdev->dd_type == DDT_DMS)
		chdr->d_cmd = MTDLNAC_SETNODE_DMS;
	else
		return;

	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);

	chdr->d_len = nlen + vlen + 2;
	memcpy(g_setnode_buf + sizeof(*chdr), name, nlen + 1);
	memcpy(g_setnode_buf + sizeof(*chdr) + nlen + 1, value, vlen + 1);
	write(g_dlna_cmdsock, g_setnode_buf, chdr->d_len + sizeof(*chdr));

	read(g_dlna_cmdsock, chdr, sizeof(*chdr));
	if (chdr->d_cmd != MTDLNAC_ACK)
		OS_PRINTF("Not Recv CMD ACK\n");

}

void* MT_DLNA_Create_DMR(void* rt)
{
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);

	memset(&g_devdmr, 0, sizeof(g_devdmr));
	g_devdmr.dd_type = DDT_DMR;

	__dlna_send_cmd(MTDLNAC_CREATE_DMR, 0, NULL);
	return &g_devdmr;
}

void MT_DLNA_Delete_DMR(void* dmr)
{
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
	if (!dmr || dmr != &g_devdmr)
		return;

	g_devdmr.dd_type = 0;
}

int MT_DLNA_Start(void* dev)
{
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
	if (g_dlna_isrunn) {
		OS_PRINTF("Already Running\n");
		return -1;
	}


	OS_PRINTF("\n\ndo dlna Start\n");
	{
		pthread_t thread1;
		pthread_create(&thread1,NULL,(void *)&__dlna_msg_proc,(void *)0);
	}

	__dlna_send_cmd(MTDLNAC_START, 0, NULL);
	g_dlna_isrunn = 1;

	return 0;
}

void MT_DLNA_Stop(void* dev)
{
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
	if (!g_dlna_isrunn) {
		OS_PRINTF("DLNA Not Running\n");
		return;
	}
	g_dlna_isrunn = 0;
	__dlna_send_cmd(MTDLNAC_STOP, 0, NULL);
	OS_PRINTF("\n\ndo dlna Stop\n");
	return;
}

int MT_DLNA_SetCB_SetMeta(void* dmr, int(*setmeta)(char*))
{
	g_setmeta = (void*)setmeta;
	return 0;
}

int MT_DLNA_SetCB_SetURL(void* dmr, int(*seturl)(char*))
{
	g_seturl = (void*)seturl;
	return 0;
}

int MT_DLNA_SetCB_Play(void* dmr, int(*play)(void))
{
	g_play = (void*)play;
	return 0;
}

int MT_DLNA_SetCB_Stop(void* dmr, int(*stop)(void))
{
	g_stop = (void*)stop;
	return 0;
}

int MT_DLNA_SetCB_Pause(void* dmr, int(*pause)(void))
{
	g_pause = (void*)pause;
	return 0;
}

int MT_DLNA_SetCB_Resume(void* dmr, int(*resume)(void))
{
	g_resume = (void*)resume;
	return 0;
}

int MT_DLNA_SetCB_GetTransportInfo(void* dmr, int(*trans)(void))
{
	g_get_transport_info = (void*)trans;
	return 0;
}

int MT_DLNA_SetCB_Seek(void* dmr, int(*seek)(int))
{
	g_seek_pos = (void*)seek;
	return 0;
}

int MT_DLNA_SetCB_GetPostion(void* dmr, int(*getpostion)(void))
{
	g_get_pos = (void*)getpostion;
	return 0;
}

int MT_DLNA_SetCB_SetVolume(void* dmr, int(*setvolume)(int))
{
	g_set_vol = (void*)setvolume;
	return 0;
}

int MT_DLNA_SetCB_GetVolume(void* dmr, int(*getvolume)(void))
{
	g_get_vol = (void*)getvolume;
	return 0;
}

int MT_DLNA_SetCB_SetMute(void* dmr, int(*setmute)(void))
{
	g_set_mute = (void*)setmute;
	return 0;
}

int MT_DLNA_SetCB_GetMute(void* dmr, int(*getmute)(void))
{
	g_get_mute = (void*)getmute;
	return 0;
}

int MT_DLNA_SetCB_GetTrackDuration(void* dmr, int(*gettrackduration)(void))
{
	g_get_track_duration = gettrackduration;
	return 0;

}

int MT_DLNA_Init(void)
{
	struct sockaddr_un address;

	if (g_dlan_inited)
		return 0;

	unlink(MTDLNA_CMDSOCK);
	unlink(MTDLNA_MSGSOCK);
	__start_dlna_daemon("dlna_daemon");
	if (dlna_daemon_started == 0) {
		OS_ERROR("start dlna daemon failed\n");
		return -1;
	}

	//OS_PRINTF("DLNA Daemon pid is:\t%d\n", g_dlna_daemon);

	sleep(1);

	g_dlna_msgsock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (g_dlna_msgsock < 0) {
		OS_PRINTF("In %s open %s Error!\n", __func__, MTDLNA_MSGSOCK);
		system("killall dlna_daemon");
		return -1;
	}

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, MTDLNA_MSGSOCK);

	if (connect(g_dlna_msgsock, &address, sizeof(address)) < 0) {
		{
			char tmpbuf[128];
			OS_PRINTF("Show /tmp fds\n");
			sprintf(tmpbuf, "ls /tmp -la");
			system(tmpbuf);
		}

		OS_PRINTF("Connect %s error!\n", MTDLNA_MSGSOCK);
		system("killall dlna_daemon");
		close(g_dlna_msgsock);
		return -1;
	}

	sleep(1);

	g_dlna_cmdsock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (g_dlna_cmdsock < 0) {
		OS_PRINTF("In %s open %s Error!\n", __func__, MTDLNA_CMDSOCK);
		system("killall dlna_daemon");
		close(g_dlna_msgsock);
		return -1;
	}

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, MTDLNA_CMDSOCK);

	if (connect(g_dlna_cmdsock, &address, sizeof(address)) < 0) {
		{
			char tmpbuf[128];
			OS_PRINTF("Show /tmp fds\n");
			sprintf(tmpbuf, "ls /tmp -la");
			system(tmpbuf);
		}

		OS_PRINTF("Connect %s error!\n", MTDLNA_CMDSOCK);
		system("killall dlna_daemon");
		close(g_dlna_msgsock);
		close(g_dlna_cmdsock);
		return -1;
	}
	g_dlan_inited = 1;

	return 0;
}


void MT_DLNA_DeInit(void)
{
	if (!g_dlan_inited)
		return;

	//kill(g_dlna_daemon, 9);
	//wait(&status);
	close(g_dlna_cmdsock);
	close(g_dlna_msgsock);
	g_dlna_cmdsock = -1;
	g_dlan_inited = 0;
	dlna_daemon_started = 0;
	if (g_setnode_buf) {
		free(g_setnode_buf);
		g_setnode_buf = NULL;
		g_setnode_buflen = 0;
	}
}

void* MT_DLNA_Create_RootDev(void)
{
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);

	if (MT_DLNA_Init())
		return NULL;

	g_devroot.dd_type = DDT_ROOT;
	__dlna_send_cmd(MTDLNAC_CREATE_ROOT, 0, NULL);
	return &g_devroot;
}

void MT_DLNA_Delete_RootDev(void* rt)
{
	if (!rt || (rt != &g_devroot))
		return;

	OS_PRINTF("\n[DLNA_FLOW]In %s at %d sk = %d\n", __func__, __LINE__, g_dlna_cmdsock);
	g_devroot.dd_type = 0;
	if (g_dlna_cmdsock > 0)
		__dlna_send_cmd(MTDLNAC_EXIT, 0, NULL);

	MT_DLNA_DeInit();
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
}

void* MT_DLNA_Create_DMS(void *rt)
{
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);

	memset(&g_devdms, 0, sizeof(g_devdms));
	g_devdms.dd_type = DDT_DMS;

	__dlna_send_cmd(MTDLNAC_CREATE_DMS, 0, NULL);
	return &g_devdms;
}

void MT_DLNA_Delete_DMS(void* dms)
{
	OS_PRINTF("\n[DLNA_FLOW]In %s at %d\n", __func__, __LINE__);
	if (!dms || dms != &g_devdms)
		return;
	g_devdms.dd_type = 0;
}

int  MT_DLNA_Send_Cmd(int cmd,void *para,int len)
{
	__dlna_send_cmd(cmd, len, para);
	return 0;
}


