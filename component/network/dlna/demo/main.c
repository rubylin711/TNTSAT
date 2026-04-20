/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <mt_cdlna.h>
#include <cybergarage/upnp/cdevice.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if 0
#define CG_UPNP_DEVICE_UDN "UDN"
#define CG_UPNP_DEVICE_DEVICE_TYPE "deviceType"
#define CG_UPNP_DEVICE_FRIENDLY_NAME "friendlyName"
#endif

#define SUPPORT_PLAYER
#ifdef SUPPORT_PLAYER
#define FUNC_PRFIX static
static int start_time;
static int cur_postion;
#define MAX_METALEN	64
static char cur_meta[MAX_METALEN + 1];
static char* g_metaptr;
static int g_mptrlen;
#define MAX_URLLEN	256
static char cur_url[MAX_URLLEN + 1];
static char* g_urlptr;
static int g_uptrlen;
static int g_mutevalue, g_volvalue;

FUNC_PRFIX int dlnac_setmeta(char* meta)
{
	int len;

	if (!meta)
		return -1;

	printf("\nSet meta = %s\n", meta);
	len = strlen(meta);
	if (len <= MAX_METALEN) {
		if (g_metaptr) {
			free(g_metaptr);
			g_metaptr = NULL;
		}
		strcpy(cur_meta, meta);
	} else {
		if (g_metaptr && len <= g_mptrlen) 
			strcpy(g_metaptr, meta);
		else {
			if(g_metaptr)
				free(g_metaptr);
			g_metaptr = malloc(len + 1);
			if (!g_metaptr)
				return -1;
			strcpy(g_metaptr, meta);
		}
	}

	g_mptrlen = len;
	
	return 0;
}


FUNC_PRFIX int dlnac_seturl(char* url)
{
	int len;

	if (!url)
		return -1;

	len = strlen(url);
	printf("\nSet url = %s <len = %d>\n", url, len);
	if (len <= MAX_URLLEN) {
		if (g_urlptr) {
			free(g_urlptr);
			g_urlptr = NULL;
		}
		strcpy(cur_url, url);
	} else {
		if (g_uptrlen >= len && g_urlptr)
			strcpy(g_urlptr, url);
		else {
			if (g_urlptr)
				free(g_urlptr);
			g_urlptr = malloc(len + 1);
			if (!g_urlptr)
				return -1;
			strcpy(g_urlptr, url);
		}
	}

	g_uptrlen = len;
	return 0;
}

FUNC_PRFIX int dlnac_play(void)
{
	start_time = cg_getcurrentsystemtime();
	cur_postion = 0;
	printf("\nPlay:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
		g_urlptr ? g_urlptr : cur_url);
}

FUNC_PRFIX int dlnac_stop(void)
{
	start_time = cur_postion = 0;
	printf("\nStop:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
		g_urlptr ? g_urlptr : cur_url);
}

FUNC_PRFIX int dlnac_pause(void)
{
	int cur_time = cg_getcurrentsystemtime();

	cur_postion += cur_time - start_time;
	start_time = 0;
	printf("\nPause:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
	g_urlptr ? g_urlptr : cur_url);

	return 0;
}

FUNC_PRFIX int dlnac_resume(void)
{
	start_time = cg_getcurrentsystemtime();
	printf("\nResume:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
		g_urlptr ? g_urlptr : cur_url);

	return 0;
}

FUNC_PRFIX int dlnac_seek(int s_time)
{

	printf("\nSeek to %d", s_time);
	start_time = cg_getcurrentsystemtime();

	cur_postion = s_time;

	return 0;
}

FUNC_PRFIX int dlnac_setvolume(int vol)
{
	
	g_volvalue = vol;
	if (vol > 0)
		g_mutevalue = 0;
	printf("\nSetvolume = %d\n", vol);
	return 0;
}

FUNC_PRFIX int dlnac_getvolume(void)
{
	printf("\nGetvolume = %d\n", g_volvalue);
	return g_volvalue;
}

FUNC_PRFIX int dlnac_setmute(void)
{
	g_mutevalue = 1;
	return 0;
}

FUNC_PRFIX int dlnac_getmute(void)
{
	return g_mutevalue;
}

FUNC_PRFIX int dlnac_getpostion(void)
{
	int pos;

	if (start_time) 
		pos = cg_getcurrentsystemtime() - start_time + cur_postion;
	else
		pos = cur_postion;

	return pos;
}


#endif


#define SKIP_TO_NEXT(p)	do{\
	if (*(p) != ' ')\
	while(*(p) && *(p) != ' ') (p)++;\
	while(*(p) && *(p) == ' ') (p)++;\
} while(0)		
	
void* mt_start_httpsvr(char* host);

int main(int argc, char* argv[])
{
	void *root;
	void *dmr;
	void *dms;
	void *httpsvr;
	char cmdbuf[128];
	char *ptr;
	
	
	root = MT_DLNA_Create_RootDev();
	if (!root) 
	{
		printf("\nCreate root Faild!!!\n");
		return -1;
	}

	httpsvr = mt_start_httpsvr(NULL);
	if (!httpsvr) {
		MT_DLNA_Delete_RootDev(root);
		printf("\nRunning httpSvr Faild!!!\n");
		return -1;
	}

	dms = MT_DLNA_Create_DMS(root, httpsvr);
	if(!dms)
	{
		printf("\nCreate dms device error!!!!\n\n");
		return -1;
	}
	

	dmr = MT_DLNA_Create_DMR(root);
	if(!dmr)
	{
		printf("\nCreate device error!!!!\n\n");
		return -1;
	}

#if 0
	MT_DLNA_Set_NodeValue(dmr, CG_UPNP_DEVICE_FRIENDLY_NAME, "DLNA_FLAX_DMR"); //set dmr's device name
	MT_DLNA_Set_NodeValue(dms, CG_UPNP_DEVICE_FRIENDLY_NAME, "DLNA_QY_DMS"); //set dmr's device name
	if (0 && argc == 3) {
		MT_DLNA_Set_NodeValue(root, CG_UPNP_DEVICE_UDN, argv[1]); //Set rootdev's udn
		MT_DLNA_Set_NodeValue(dmr, CG_UPNP_DEVICE_UDN, argv[1]); //Set dmr's udn
		MT_DLNA_Set_NodeValue(dmr, CG_UPNP_DEVICE_DEVICE_TYPE, argv[2]); //set dmr's device type
	}

	MT_DLNA_SetCB_SetMeta(dmr, dlnac_setmeta);
	MT_DLNA_SetCB_SetURL(dmr, dlnac_seturl);
	MT_DLNA_SetCB_Play(dmr, dlnac_play);
	MT_DLNA_SetCB_Stop(dmr, dlnac_stop);
	MT_DLNA_SetCB_Pause(dmr, dlnac_pause);
	MT_DLNA_SetCB_Resume(dmr, dlnac_resume);
	MT_DLNA_SetCB_Seek(dmr, dlnac_seek);
	MT_DLNA_SetCB_SetMute(dmr, dlnac_setmute);
	MT_DLNA_SetCB_GetMute(dmr, dlnac_getmute);
	MT_DLNA_SetCB_SetVolume(dmr, dlnac_setvolume);
	MT_DLNA_SetCB_GetVolume(dmr, dlnac_getvolume);
	MT_DLNA_SetCB_GetPostion(dmr, dlnac_getpostion);

	if (argc == 3)
		MT_DLNA_DMS_AddContent(dms, argv[1], argv[2]);
	else
		MT_DLNA_DMS_AddContent(dms, "/media/sda1", "MediaFile");
#endif

	MT_DLNA_Start(root);
	printf("\nStart DLNA...\n");
	while(1) {
		if (!fgets(cmdbuf, 127, stdin))
			break;

		if (strncmp(cmdbuf, "name", 4) == 0) {
			printf("\nDMR Name:\t%s\n", MT_DLNA_Get_NodeValue(dmr, CG_UPNP_DEVICE_FRIENDLY_NAME));
			printf("\nDMS Name:\t%s\n", MT_DLNA_Get_NodeValue(dms, CG_UPNP_DEVICE_FRIENDLY_NAME));
		}

		if (strncmp(cmdbuf, "setattr", 7) == 0) {
			char* type;
			void* dev;
			ptr = cmdbuf + 7;
			SKIP_TO_NEXT(ptr);
			type = ptr;
			SKIP_TO_NEXT(ptr);
			ptr[-1] = 0;
			if (strncmp(type, "dms", 3) == 0) 
				dev = dms;
			else
				dev = dmr;
		
			type = ptr;
			SKIP_TO_NEXT(ptr);
			ptr[-1] = 0;
			
			MT_DLNA_Set_NodeValue(dev, type, ptr);
		}

		if (strncmp(cmdbuf, "addcontent", 10) == 0) {
			char *path, *name;

			path = cmdbuf + 10;
			SKIP_TO_NEXT(path);

			name = path;
			SKIP_TO_NEXT(name);
			name[-1] = 0;
			printf("\nPath:\t%s\nName:\t%s\n", path, name);
			MT_DLNA_DMS_AddContent(dms, path, name);
		}

		if (strncmp(cmdbuf, "addfilter", 9) == 0) {
			char* postfix;
			cmdbuf[strlen(cmdbuf) - 1] = 0; //skip '\n'
			postfix = cmdbuf + 9;
			SKIP_TO_NEXT(postfix);

			dlna_add_filter(dms, postfix);
		}

		if (strncmp(cmdbuf, "dump", 4) == 0) {
			void dlna_dms_dump(void* dms);
			dlna_dms_dump(dms);
		}
		if (strncmp(cmdbuf, "exit", 4) == 0)
			break;

	}

	MT_DLNA_Delete_DMR(dmr);
	MT_DLNA_Delete_DMS(dms);
	MT_DLNA_Delete_RootDev(root);

	printf("\nto do eixt\n");
	while(1) {
		fgets(cmdbuf, 127, stdin);
		if (strstr(cmdbuf, "done"))
			break;
	}
	printf("\nDone!!!\n");
	return 0;
}

