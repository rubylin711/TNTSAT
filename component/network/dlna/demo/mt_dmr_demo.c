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

#if defined(__GST_PLAYER__)
#include "mt_type.h"
#include "mtsu_svr_player.h"
#endif

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

typedef enum PLAY_STATE
{
    STATE_VOID_PENDING        = 0,
    STATE_NULL                = 1,
    STATE_READY               = 2,
    STATE_PAUSED              = 3,
    STATE_PLAYING             = 4
}PLAY_STATE;

PLAY_STATE fp_state = STATE_VOID_PENDING;

#if defined(__GST_PLAYER__)

MT_HANDLE g_hAvplay = 0;
MT_HANDLE hWin;
MT_HANDLE hTrack;
/*!
  Success return
  */
#define SUCCESS ((mt_s32)0)


MT_U32  mp_event_callback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent)
{
    unsigned long vpts = 0;
    unsigned long  cur_hour = 0;
    unsigned long  cur_min  = 0;
    unsigned long  cur_sec = 0;

    printf("&&&&@@@@@!!!1%s %d ,event=%d\n", __func__, __LINE__,pstruEvent->eEvent);

    switch (pstruEvent->eEvent) {
        case MT_SVR_PLAYER_EVENT_STATE_CHANGED:
            if(1){
                //printf("\n\n &&&&@@@@@!!!1%s %d ,event=%d\n", __func__, __LINE__,event);
                MT_SVR_PLAYER_STATE_E *data;
                data = (MT_SVR_PLAYER_STATE_E *)pstruEvent->pu8Data;
                if(*data == MT_SVR_PLAYER_STATE_STOP){
                    fp_state = STATE_NULL;
                }
            }
            break;
        case MT_SVR_PLAYER_EVENT_SOF:
            break;

        case MT_SVR_PLAYER_EVENT_EOF:
            break;

        case MT_SVR_PLAYER_EVENT_PROGRESS:
            break;

        case MT_SVR_PLAYER_EVENT_STREAMID_CHANGED:
            break;
        case MT_SVR_PLAYER_EVENT_SEEK_FINISHED:
            break;
        case MT_SVR_PLAYER_EVENT_CODETYPE_CHANGED:
            break;
        case MT_SVR_PLAYER_EVENT_DOWNLOAD_PROGRESS:
            break;
        case MT_SVR_PLAYER_EVENT_BUFFER_STATE:
            break;
        case MT_SVR_PLAYER_EVENT_FIRST_FRAME_TIME:
            break;
        case MT_SVR_PLAYER_EVENT_ERROR:
            if(1){
                MT_SVR_PLAYER_ERROR_E *data;
                data = (MT_SVR_PLAYER_ERROR_E *)pstruEvent->pu8Data;
                if(*data == MT_SVR_PLAYER_ERROR_UNKNOW){
                    fp_state = STATE_NULL;
                }
            }
            break;
        case MT_SVR_PLAYER_EVENT_NETWORK_INFO:
            break;
        case MT_SVR_PLAYER_EVENT_DOWNLOAD_FINISH:
            break;
        case MT_SVR_PLAYER_EVENT_ASYNC_SETMEDIA_FINISH:
            break;
        case MT_SVR_PLAYER_EVENT_UPDATE_FILE_INFO:
            break;
        case MT_SVR_PLAYER_EVENT_STREAM_NOT_AVAIABLE:
            break;
        case MT_SVR_PLAYER_EVENT_NETWORKBRANDWIDTH:
            break;
        case MT_SVR_PLAYER_EVENT_USER_PRIVATE:
            break;

        default :
            break;
    }

    return  0;
}

#endif

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

    if(fp_state == STATE_PLAYING)
    {
#if defined(__GST_PLAYER__)
       MT_SVR_PLAYER_Stop(g_hAvplay);
#endif
    }

#if defined(__GST_PLAYER__)

    MT_SVR_PLAYER_MEDIA_S stMedia;
    MT_FORMAT_FILE_INFO_S *pstFileInfo = NULL;
    if(g_uptrlen < MAX_URLLEN)
        sprintf(stMedia.aszUrl, "%s",cur_url);
    else
        sprintf(stMedia.aszUrl, "%s",g_urlptr);
    MT_SVR_PLAYER_SetMedia(g_hAvplay, 0, &stMedia);
    int ret = MT_SVR_PLAYER_LOADMEDIA_GetFileInfo(g_hAvplay, &pstFileInfo);
    if(ret != SUCCESS)
    {
        printf("MT_SVR_PLAYER_LOADMEDIA_GetFileInfo failed!!\n");
        return;
    }


#endif
	return 0;
}

FUNC_PRFIX int dlnac_play(void)
{
	start_time = cg_getcurrentsystemtime();
	cur_postion = 0;
	printf("\nPlay:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
		g_urlptr ? g_urlptr : cur_url);

#if defined(__GST_PLAYER__)
    MT_SVR_PLAYER_Play(g_hAvplay, 0);
    fp_state = STATE_PLAYING;
#endif
	return 0;
}

FUNC_PRFIX int dlnac_stop(void)
{
	start_time = cur_postion = 0;
	printf("\nStop:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
		g_urlptr ? g_urlptr : cur_url);
#if defined(__GST_PLAYER__)
    MT_SVR_PLAYER_Stop(g_hAvplay);
#endif
    fp_state = STATE_NULL;
	return 0;
}

FUNC_PRFIX int dlnac_pause(void)
{
	int cur_time = cg_getcurrentsystemtime();

	cur_postion += cur_time - start_time;
	start_time = 0;
	printf("\nPause:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
	g_urlptr ? g_urlptr : cur_url);
#if defined(__GST_PLAYER__)
    MT_SVR_PLAYER_Pause(g_hAvplay);
#endif
    fp_state = STATE_PAUSED;
	return 0;
}

FUNC_PRFIX int dlnac_resume(void)
{
	start_time = cg_getcurrentsystemtime();
	printf("\nResume:\t%s\nURL:\t%s\n", g_metaptr ? g_metaptr : cur_meta,
		g_urlptr ? g_urlptr : cur_url);
    if(fp_state == STATE_PAUSED)
    {
#if defined(__GST_PLAYER__)
        MT_SVR_PLAYER_Resume(g_hAvplay);
#endif
        fp_state = STATE_PLAYING;
    }
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

int main(int argc, char* argv[])
{
	void *root;
	void *dmr;
	char cmdbuf[128];
	char *ptr;

#if defined(__GST_PLAYER__)

    mtUNF_SUPLAYER_IN_ARG_S args;
    mtUNF_SUPLAYER_STATUS_S *pstatus;
    static MT_HANDLE hPlayer = (MT_HANDLE)NULL;
    //MT_PLAYBACK_INTERNAL_T *p_MonPlayer;
    //mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_SVR_PLAYER_PARAM_S s_stParam = {
        0,
        3,
        0,
        0,
        0,
        0,
        100,
        0,
        0,
        0,
        100,
        0,
        0,
        0,
        0,
        0
    };
    args.ptype = MT_SUPLAYER_GSTREAMER;
	args.pri = NULL;
    pstatus = MT_SVR_PLAYER_Init(&args);
    if(pstatus == NULL){
        printf("\nInit_player fail!!!\n");
        return;
    }
    s_stParam.suplayer_status = pstatus;
    int ret = MT_SVR_PLAYER_Create(&s_stParam, &hPlayer);
    if(ret != SUCCESS)
    {
        printf("MT_SVR_PLAYER_Create failed!!\n");
        return;
    }

    ret = MT_SVR_PLAYER_RegCallback(hPlayer, mp_event_callback);

    g_hAvplay = hPlayer;
#endif

	root = MT_DLNA_Create_RootDev();
	if (!root) 
	{
		printf("\nCreate root Faild!!!\n");
		return -1;
	}


	dmr = MT_DLNA_Create_DMR(root);
	if(!dmr)
	{
		printf("\nCreate device error!!!!\n\n");
		return -1;
	}

	MT_DLNA_Set_NodeValue(dmr, CG_UPNP_DEVICE_FRIENDLY_NAME, "DLNA_FLAX_DMR"); //set dmr's device name
	if (0 && argc == 3) { // you can set node
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


	MT_DLNA_Start(root);
	printf("\nStart DLNA...\n");
	while(1)
		sleep(5);
}


