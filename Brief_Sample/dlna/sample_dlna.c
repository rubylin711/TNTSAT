/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <fcntl.h>
#include <unistd.h>
#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_demux.h"
#include "mt_unf_descrambler.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecs.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_mpi_demux.h"

#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"

#include "file_playback_sequence.h"
#include "mtsu_svr_player.h"
#include "suplayer_internal.h"
#include "mt_cdlna.h"
#include "mt_cmdline.h"

#include <signal.h>


/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DLNA_DEBUG

#define MT_DLNA_PRINT   printf
#else

#define MT_DLNA_PRINT

#endif

#define SAMPLE_DLNA_FUNCTION_ENTER()    MT_DLNA_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DLNA_FUNCTION_EXIT()     MT_DLNA_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DLNA_FATAL_PRINT(fmt...)         MT_DLNA_PRINT(" [FATAL] " fmt)
#define SAMPLE_DLNA_ERR_PRINT(fmt...)           MT_DLNA_PRINT(" [ERROR] " fmt)
#define SAMPLE_DLNA_WARN_PRINT(fmt...)      MT_DLNA_PRINT(" [WARN] "  fmt)
#define SAMPLE_DLNA_INFO_PRINT(fmt...)      MT_DLNA_PRINT(" [INFO] "  fmt)
#define SAMPLE_DLNA_DBG_PRINT(fmt...)           MT_DLNA_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DLNA_PRINT      printf
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0 0
#define MT_DLAN_NODE  "friendlyName"
#define MT_DLAN_NAME  "MT_DLNA_Test"


/*************************** Structure Definition ****************************/


typedef struct
{
    MT_S32     candoplay;
    MT_CHAR*   url;
    MT_HANDLE  hPlayer;
    MT_HANDLE  hWin;
    MT_HANDLE  hAvplay;
    MT_HANDLE  hSoundTrack;
    void* rootdev;
    void* dmrdev;
}mt_sample_dlna_info;

extern  void file_seq_avplay_handle_set(int handle_avplay, int handle_track);

/********************** Global Variable declaration **************************/

static mt_sample_dlna_info g_dlna_info;
static MT_BOOL g_bTaskQuit = MT_TRUE;


#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

#ifdef MT_SAMPLE_APP
MT_S32 MT_DlnaMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static MT_U32 total_time_last = 0;

static void MT_Sample_DLNAPLAYER_GetPlayerInfo(void *args)
{
    mt_s32 ret = 0;
    unsigned long  cur_hour = 0;
    unsigned long  cur_min  = 0;
    unsigned long  cur_sec = 0;
    MT_SVR_PLAYER_INFO_S struInfo;
    while (g_dlna_info.hPlayer != 0)
    {

        ret = MT_SVR_PLAYER_GetPlayerInfo(g_dlna_info.hPlayer, &struInfo);
        if(MT_SUCCESS != ret || struInfo.eStatus != MT_SVR_PLAYER_STATE_PLAY)
        {
            continue;
        }

        cur_hour = (struInfo.u64TimePlayed / 1000) / 3600;
        cur_min = ((struInfo.u64TimePlayed / 1000) - (cur_hour * 3600)) / 60;
        cur_sec = (struInfo.u64TimePlayed / 1000) - (cur_hour * 3600) - cur_min * 60;
        SAMPLE_DLNA_PRINT("playing [%ld:%ld:%ld],  duraton %d ms\n", cur_hour, cur_min, cur_sec, (struInfo.film_duration));
        MT_USLEEP(1 * 1000 * 1000);
    }
}

static MT_U32  MT_Sample_DLNAEvent_Callback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent)
{
    MT_SVR_PLAYER_STATE_E *data = NULL;
    MT_UNF_WINDOW_ATTR_S winAttr;
    VIDEO_W_H_FPS *video_whfps = NULL;
    MT_UNF_DISP_ASPECT_RATIO_S stDispAspectRatio = {0};

    //SAMPLE_DLNA_INFO_PRINT("<%s> ,event=%d\n", __func__, pstruEvent->eEvent);

    switch (pstruEvent->eEvent) {
        case MT_SVR_PLAYER_EVENT_STATE_CHANGED:

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

            data = (MT_SVR_PLAYER_STATE_E *)pstruEvent->pu8Data;
            printf("MT_SVR_PLAYER_EVENT_SEEK_FINISHED date %d\n", *data);
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
        case MT_SVR_PLAYER_EVENT_NEW_SUBTITLE_RECEIVED:

            break;

        case MT_SVR_PLAYER_EVENT_FILE_INFO:
            video_whfps = (VIDEO_W_H_FPS *)pstruEvent->pu8Data;

            printf("video information: width*height=%d*%d \n", video_whfps->video_disp_w, video_whfps->video_disp_h);
            if (video_whfps->video_disp_w < video_whfps->video_disp_h)
            {
                stDispAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_16TO9;
                MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &stDispAspectRatio);

                memset(&winAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
                MT_UNF_VO_GetWindowAttr(g_dlna_info.hWin, &winAttr);

                winAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_LETTERBOX;
                MT_UNF_VO_SetWindowAttr(g_dlna_info.hWin, &winAttr);
            }

            break;

        default :
            break;
    }

    return  0;
}







/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] g_dlna_info.hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_Sample_DLNAAvPlayInit(MT_VOID)
{
    mt_s32      ret = MT_SUCCESS;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };


    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &g_dlna_info.hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    SAMPLE_DLNA_INFO_PRINT("[%s] g_dlna_info.hAvplay: 0x%x \n",__func__, (mt_u32)g_dlna_info.hAvplay);

    ret = MTADP_Snd_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT("failed to MTADP_Snd_Init\n");
        goto ERR2;
    }

    ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT("failed to MTADP_VO_Init\n");
        goto ERR3;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(g_dlna_info.hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(g_dlna_info.hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen MEDIA_CHAN_AUD failed.\n");
        goto ERR5;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &g_dlna_info.hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_SND_Attach(g_dlna_info.hSoundTrack, g_dlna_info.hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR7;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &g_dlna_info.hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR8;
    }

    ret = MT_UNF_VO_AttachWindow(g_dlna_info.hWin, g_dlna_info.hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR9;
    }

    ret = MT_UNF_VO_SetWindowEnable(g_dlna_info.hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR10;
    }


#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_dlna_info.hAvplay;
        avplayHandle.hWin = g_dlna_info.hWin;
        avplayHandle.hSoundTrack = g_dlna_info.hSoundTrack;
#endif

    return MT_SUCCESS;


ERR10:
    (MT_VOID)MT_UNF_VO_DetachWindow(g_dlna_info.hWin, g_dlna_info.hAvplay);

ERR9:
    (MT_VOID)MT_UNF_VO_DestroyWindow(g_dlna_info.hWin);

ERR8:
    (MT_VOID)MT_UNF_SND_Detach(g_dlna_info.hSoundTrack, g_dlna_info.hAvplay);

ERR7:
    (MT_VOID)MT_UNF_SND_DestroyTrack(g_dlna_info.hSoundTrack);

ERR6:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(g_dlna_info.hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

ERR5:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(g_dlna_info.hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

ERR4:
    (MT_VOID)MTADP_VO_DeInit();

ERR3:
    (MT_VOID)MTADP_Snd_DeInit();

ERR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(g_dlna_info.hAvplay);

ERR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
#endif

    return MT_FAILURE;
}



/*
 @brief Audio and video playback Deinit
 @param[in] g_dlna_info.hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32  MT_Sample_DLNAAVplayDeinit(MT_VOID)
{

    (MT_VOID)MT_UNF_VO_SetWindowEnable(g_dlna_info.hWin, MT_FALSE);

    (MT_VOID)MT_UNF_VO_DetachWindow(g_dlna_info.hWin, g_dlna_info.hAvplay);

    (MT_VOID)MT_UNF_VO_DestroyWindow(g_dlna_info.hWin);

    (MT_VOID)MT_UNF_SND_Detach(g_dlna_info.hSoundTrack, g_dlna_info.hAvplay);

    (MT_VOID)MT_UNF_SND_DestroyTrack(g_dlna_info.hSoundTrack);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(g_dlna_info.hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(g_dlna_info.hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MT_UNF_AVPLAY_Destroy(g_dlna_info.hAvplay);

    (MT_VOID)MT_UNF_AVPLAY_DeInit();

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
#endif

    return MT_SUCCESS;


}


/*
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_Sample_DLNAAvplay_Start(MT_VOID)
{
    mt_s32 ret = 0;
    MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
    mtUNF_SUPLAYER_IN_ARG_S args;
    mtUNF_SUPLAYER_STATUS_S *pstatus;
    MT_SVR_PLAYER_PARAM_S s_stParam = {0};

    MT_FORMAT_FILE_INFO_S *pstFileInfo = NULL;
    MT_SVR_PLAYER_MEDIA_S stMedia;
    pthread_t tid;
    pthread_attr_t attribs;


    //av sync
    memset(&SyncAttr,0,sizeof(MT_UNF_SYNC_ATTR_S));
    ret = MT_UNF_AVPLAY_GetAttr(g_dlna_info.hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
        return ret;
    }
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    ret = MT_UNF_AVPLAY_SetAttr(g_dlna_info.hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
        return ret;
    }

    args.ptype = MT_SUPLAYER_MPLAYER;
    args.pri = NULL;
    pstatus = (mtUNF_SUPLAYER_STATUS_S *)MT_SVR_PLAYER_Init(&args);
    if(pstatus == NULL)
    {
        printf("\nInit_player fail!!!\n");
        return MT_FAILURE;
    }

    memset(&s_stParam,0,sizeof(MT_SVR_PLAYER_PARAM_S));
    s_stParam.u32VDecErrCover = 100;
    s_stParam.suplayer_status = pstatus;
    s_stParam.hAVPlayer = g_dlna_info.hAvplay;

    ret = MT_SVR_PLAYER_Create(&s_stParam, &g_dlna_info.hPlayer);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Deinit(MT_NULL);
        return ret;
    }
    ret = MT_SVR_PLAYER_RegCallback(g_dlna_info.hPlayer, MT_Sample_DLNAEvent_Callback);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
        return ret;
    }
    ret = MT_SVR_PLAYER_Set_T1xbase_Num(g_dlna_info.hPlayer, 1);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
        return ret;
    }

    memset(&stMedia,0,sizeof(MT_SVR_PLAYER_MEDIA_S));
    sprintf(stMedia.aszUrl, "%s", g_dlna_info.url);
    ret = MT_SVR_PLAYER_SetMedia(g_dlna_info.hPlayer, 0, &stMedia);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
        return ret;
    }
    ret = MT_SVR_PLAYER_LOADMEDIA_GetFileInfo(g_dlna_info.hPlayer, &pstFileInfo);
    if(MT_SUCCESS != ret)
    {
        /* when app send not support picture, release player resource,
        and can play normally when sending a video */
        //(MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
        //(MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
        return ret;
    }
    (MT_VOID)file_seq_avplay_handle_set(g_dlna_info.hAvplay, g_dlna_info.hSoundTrack);


    ret = MT_SVR_PLAYER_Play(g_dlna_info.hPlayer, 0);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
        return ret;
    }


    pthread_attr_init(&attribs);
    pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&tid, &attribs, (void *)MT_Sample_DLNAPLAYER_GetPlayerInfo, NULL);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
        return ret;
    }

    return MT_SUCCESS;
}

static int MT_Sample_DLNAGetTransportInfo(void)
{
    int ret = 0;
    MT_SVR_PLAYER_INFO_S loc = { 0 };
    if(g_dlna_info.hPlayer == 0)
    {
        return -1;
    }
    ret = MT_SVR_PLAYER_GetPlayerInfo(g_dlna_info.hPlayer, &loc);
    if (ret < 0)
    {
        return 0;
    }
    if (loc.eStatus == MT_SVR_PLAYER_STATE_PLAY)
    {
        return 1;
    }
    else if(loc.eStatus == MT_SVR_PLAYER_STATE_PAUSE)
    {
        return 2;
    }
    else
    {
        return 0;
    }

}

static void when_sigusr1_void(int sig)
{

}


static MT_S32 MT_Sample_DLNAPlay(void)
{
    mt_s32 ret = 0;


    if (g_dlna_info.url)
    {


        if(g_dlna_info.candoplay == 0)
        {
            return -1;
        }
        if(g_dlna_info.hPlayer != 0)
        {

            (MT_VOID)MT_SVR_PLAYER_Stop(g_dlna_info.hPlayer);
            (MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
            (MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
            (MT_VOID)MT_Sample_DLNAAVplayDeinit();
            g_dlna_info.hPlayer = 0;
        }

        g_dlna_info.candoplay = 0;
        kill(getpid(), SIGUSR1);
        sleep(1);


        ret = MT_Sample_DLNAAvPlayInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("failed to MT_Sample_DLNAAvPlayInit\n");
        }

        ret = MT_Sample_DLNAAvplay_Start();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("failed to MT_Sample_DLNAAvplay_Start\n");
        }
    }

    return 0;
}

static int MT_Sample_DLNAGetTrackDuration(void)
{
    mt_s32 ret = 0;
    MT_SVR_PLAYER_FILE_INFO_S media_info = {0};
    MT_SVR_PLAYER_INFO_S loc = {0};
    MT_U32 total_time = 0;


    memset(&media_info,0,sizeof(media_info));
    if(g_dlna_info.hPlayer == 0)
    {
        return -1;
    }
    MT_SVR_PLAYER_GetPlayerInfo(g_dlna_info.hPlayer, &loc);
    ret = MT_SVR_PLAYER_Get_Media_Info(g_dlna_info.hPlayer,&media_info);
    total_time = media_info.film.film_duration/1000;
    if(loc.eStatus == MT_SVR_PLAYER_STATE_STOP)
    {
        total_time = total_time_last;
    }
    total_time_last = total_time;
    return total_time;
}

static MT_S32 MT_Sample_DLNASetMeta(MT_S8* meta)
{
    if (!meta)
        return -1;

    SAMPLE_DLNA_INFO_PRINT("\nSet meta = %s\n", meta);
    return 0;
}
static MT_S32 MT_Sample_DLNAStop(void)
{

    if(g_dlna_info.url)
    {
        SAMPLE_DLNA_INFO_PRINT("\n\nDLNA Stop\n\n");

        (MT_VOID)MT_SVR_PLAYER_Stop(g_dlna_info.hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Destroy(g_dlna_info.hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(g_dlna_info.hPlayer);
        (MT_VOID)MT_Sample_DLNAAVplayDeinit();
        g_dlna_info.url = NULL;
        g_dlna_info.hPlayer = 0;

    }
    g_dlna_info.candoplay = 1;
    return 0;
}

static MT_S32 MT_Sample_DLNAPause(void)
{
    SAMPLE_DLNA_INFO_PRINT("\n\nDLNA Pause!!!\n\n");
    if (g_dlna_info.url)
    {
        (MT_VOID)MT_SVR_PLAYER_Pause(g_dlna_info.hPlayer);
    }

    return 0;
}

static MT_S32 MT_Sample_DLNAResume(void)
{
    SAMPLE_DLNA_INFO_PRINT("\n\nDLNA Resume!!!\n\n");
    if (g_dlna_info.url)
    {
        (MT_VOID)MT_SVR_PLAYER_Resume(g_dlna_info.hPlayer);
    }

    return 0;
}

static MT_S32 MT_Sample_DLNASeek(MT_S32 s_time)
{
    SAMPLE_DLNA_INFO_PRINT("\n\nDLNA Seek at %ds!!!\n\n", s_time);
    if (g_dlna_info.url)
    {
        (MT_VOID)MT_SVR_PLAYER_Seek(g_dlna_info.hPlayer, s_time*1000);
    }
    return 0;
}

static MT_S32 MT_Sample_DLNASetVolume(int vol)
{
    MT_UNF_SND_GAIN_ATTR_S volume = {0};

    SAMPLE_DLNA_INFO_PRINT("\nSetvolume = %d\n", vol);
    volume.s32Gain = vol;
    (MT_VOID)MT_UNF_SND_SetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0 , &volume);
    return 0;
}

static MT_S32 MT_Sample_DLNAGetVolume(void)
{
    return 0;
}

static MT_S32 MT_Sample_DLNASetMute(void)
{
    SAMPLE_DLNA_INFO_PRINT("\nSet Mute\n");
    return 0;
}

static MT_S32 MT_Sample_DLNAGetMute(void) {
    SAMPLE_DLNA_INFO_PRINT("\nGet Mute\n");
    return 0;
}

static MT_S32 MT_Sample_DLNASetURL(MT_CHAR* url)
{
    SAMPLE_DLNA_INFO_PRINT("\nURL = %s\n", url);
    if (g_dlna_info.url)
    {
        g_dlna_info.url = NULL;
    }
    g_dlna_info.url = url;

    g_dlna_info.candoplay = 1;

    return 0;

}

static MT_S32 MT_Sample_DLNAGetPostion(void)
{
    MT_U32 last_pts = 0;
    MT_U32 last_pts_cnt = 0;
    MT_S32 ret = 0;
    MT_S32 current_time = 0;
    MT_SVR_PLAYER_INFO_S loc = { 0 };

    if(g_dlna_info.hPlayer == 0)
    {
        return MT_FAILURE;
    }


    ret = MT_SVR_PLAYER_GetPlayerInfo(g_dlna_info.hPlayer, &loc);
    current_time = loc.u64TimePlayed / 1000;
    if(loc.eStatus == MT_SVR_PLAYER_STATE_STOP)
    {
        current_time =  total_time_last;
    }
    else
    {
        last_pts = current_time;
    }

    return current_time;
}

static MT_S32 MT_Sample_DLNAInit(MT_VOID)
{


    g_dlna_info.rootdev = MT_DLNA_Create_RootDev();
    if(!g_dlna_info.rootdev)
    {
        SAMPLE_DLNA_ERR_PRINT("Create_RootDev failed! \n");
        return MT_FAILURE;
    }

    g_dlna_info.dmrdev = MT_DLNA_Create_DMR(g_dlna_info.rootdev);
    if(!g_dlna_info.dmrdev)
    {
        SAMPLE_DLNA_ERR_PRINT("MT_DLNA_Create_DMR failed! \n");
    }

    (MT_VOID)MT_DLNA_Set_NodeValue(g_dlna_info.dmrdev, MT_DLAN_NODE, MT_DLAN_NAME);
    (MT_VOID)MT_DLNA_SetCB_SetMeta(g_dlna_info.dmrdev, (void*)MT_Sample_DLNASetMeta);
    (MT_VOID)MT_DLNA_SetCB_SetURL(g_dlna_info.dmrdev, (void*)MT_Sample_DLNASetURL);
    (MT_VOID)MT_DLNA_SetCB_Play(g_dlna_info.dmrdev, MT_Sample_DLNAPlay);
    (MT_VOID)MT_DLNA_SetCB_Stop(g_dlna_info.dmrdev, MT_Sample_DLNAStop);
    (MT_VOID)MT_DLNA_SetCB_Pause(g_dlna_info.dmrdev, MT_Sample_DLNAPause);
    (MT_VOID)MT_DLNA_SetCB_Resume(g_dlna_info.dmrdev, MT_Sample_DLNAResume);
    (MT_VOID)MT_DLNA_SetCB_Seek(g_dlna_info.dmrdev, MT_Sample_DLNASeek);
    (MT_VOID)MT_DLNA_SetCB_SetMute(g_dlna_info.dmrdev, MT_Sample_DLNASetMute);
    (MT_VOID)MT_DLNA_SetCB_GetMute(g_dlna_info.dmrdev, MT_Sample_DLNAGetMute);
    (MT_VOID)MT_DLNA_SetCB_SetVolume(g_dlna_info.dmrdev, MT_Sample_DLNASetVolume);
    (MT_VOID)MT_DLNA_SetCB_GetVolume(g_dlna_info.dmrdev, MT_Sample_DLNAGetVolume);
    (MT_VOID)MT_DLNA_SetCB_GetPostion(g_dlna_info.dmrdev, MT_Sample_DLNAGetPostion);
    (MT_VOID)MT_DLNA_SetCB_GetTransportInfo(g_dlna_info.dmrdev, MT_Sample_DLNAGetTransportInfo);
    (MT_VOID)MT_DLNA_SetCB_GetTrackDuration(g_dlna_info.dmrdev, MT_Sample_DLNAGetTrackDuration);


    return MT_SUCCESS;

}

static MT_S32 MT_Sample_DLNADeinit(MT_VOID)
{

    (MT_VOID)MT_DLNA_Delete_DMR(g_dlna_info.dmrdev);
    g_dlna_info.dmrdev = NULL;
    (MT_VOID)MT_DLNA_Delete_RootDev(g_dlna_info.rootdev);
    g_dlna_info.rootdev = NULL;
    return MT_SUCCESS;
}


static MT_VOID MT_Sample_DLNAPrintMenu(MT_VOID)
{
#ifdef MT_SAMPLE_APP
    SAMPLE_DLNA_PRINT("      b: background run \n");
#endif
    SAMPLE_DLNA_PRINT("      a: Back off for 20 seconds \n");
    SAMPLE_DLNA_PRINT("      d: Advance for 20 seconds \n");
    SAMPLE_DLNA_PRINT("      p: pause \n");
    SAMPLE_DLNA_PRINT("      r: resume \n");
    SAMPLE_DLNA_PRINT("      h: help \n");
    SAMPLE_DLNA_PRINT("      q: quit \n");
    SAMPLE_DLNA_PRINT("Dlna>> ");
}


static void MT_Sample_DLNACmdTask(MT_VOID)
{
    MT_CHAR  inputCmd[32] = { 0 };
    MT_SVR_PLAYER_INFO_S struInfo = {0};


    while(1)
    {
        (MT_VOID)MT_Sample_DLNAPrintMenu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_DLNA_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_DLNA_INFO_PRINT("dlna play in back!\n");
            break;
        }
#endif
        else if('d' == inputCmd[0])
        {
            if (g_dlna_info.url)
            {
                memset(&struInfo, 0, sizeof(MT_SVR_PLAYER_INFO_S));
                MT_SVR_PLAYER_GetPlayerInfo(g_dlna_info.hPlayer, &struInfo);
                (MT_VOID)MT_SVR_PLAYER_Seek(g_dlna_info.hPlayer, struInfo.u64TimePlayed + 20*1000);
                SAMPLE_DLNA_INFO_PRINT("Advance for 20 seconds \n");
            }
        }
        else if('a' == inputCmd[0])
        {
            if (g_dlna_info.url)
            {
                memset(&struInfo, 0, sizeof(MT_SVR_PLAYER_INFO_S));
                MT_SVR_PLAYER_GetPlayerInfo(g_dlna_info.hPlayer, &struInfo);
                (MT_VOID)MT_SVR_PLAYER_Seek(g_dlna_info.hPlayer, (struInfo.u64TimePlayed - 20*1000));
                SAMPLE_DLNA_INFO_PRINT("Back off for 20 seconds \n");
            }
        }
        else if('p' == inputCmd[0])
        {

            if (g_dlna_info.url)
            {
                SAMPLE_DLNA_INFO_PRINT("DLNA Pause!!!\n");
                (MT_VOID)MT_SVR_PLAYER_Pause(g_dlna_info.hPlayer);
            }
        }
        else if('r' == inputCmd[0])
        {

            if (g_dlna_info.url)
            {
                SAMPLE_DLNA_INFO_PRINT("DLNA Resume!!!\n");
                (MT_VOID)MT_SVR_PLAYER_Resume(g_dlna_info.hPlayer);
            }
        }


        else if('h' == inputCmd[0])
        {
            SAMPLE_DLNA_INFO_PRINT("get help info! \n");
        }
        usleep(5000);
    }
}

static mt_s32 MT_DlnaExit(MT_VOID)
{
    (MT_VOID)MT_Sample_DLNAStop();
    (MT_VOID)MT_DLNA_Stop(g_dlna_info.rootdev);
    (MT_VOID)MT_Sample_DLNADeinit();
    g_bTaskQuit = MT_TRUE;
    memset(&g_dlna_info, 0, sizeof(mt_sample_dlna_info));

    return MT_SUCCESS;
}

static mt_s32 MT_DlnaParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_DLNA_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHq")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                   (MT_VOID)MT_DlnaExit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_DLNA_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DlnaMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    int ret = 0;


    signal(SIGUSR1, when_sigusr1_void);

    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }
    ret = MT_DlnaParase_args(argc, argv);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DLNA_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DLNA_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
    if(g_bTaskQuit == MT_TRUE)
    {
        ret = system("ping www.baidu.com -c 4");
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("Network connection failure \n");
            return ret;
        }
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("failed to MT_SYS_Init\n");
            return ret;
        }
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, 0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
            }
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }
#endif

        ret = MT_Sample_DLNAInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR3;
        }

        ret = MT_DLNA_Start(g_dlna_info.rootdev);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DLNA_ERR_PRINT("failed to MT_DLNA_Start\n");
            goto ERR4;
        }
        g_bTaskQuit = MT_FALSE;
    }

    (MT_VOID)MT_Sample_DLNACmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }


    (MT_VOID)MT_DLNA_Stop(g_dlna_info.rootdev);

    ret = MT_Sample_DLNAStop();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DLNA_ERR_PRINT("failed to MT_Sample_DLNAStop\n");
        goto ERR4;
    }



ERR4:
    (MT_VOID)MT_Sample_DLNADeinit();

ERR3:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_dlna_info, 0, sizeof(mt_sample_dlna_info));

    return MT_SUCCESS;
}
