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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"

#include "mtsu_svr_player.h"
#include "suplayer_internal.h"
#include "mt_cmdline.h"


#include <signal.h>




/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_SUPLAY_DEBUG

#define MT_SUPLAY_PRINT   printf
#else

#define MT_SUPLAY_PRINT

#endif

#define SAMPLE_SUPLAY_FUNCTION_ENTER()  MT_SUPLAY_PRINT("[SUPLAY][%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SUPLAY_FUNCTION_EXIT()       MT_SUPLAY_PRINT("[SUPLAY][%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_SUPLAY_FATAL_PRINT(fmt...)       MT_SUPLAY_PRINT(" [SUPLAY][FATAL] " fmt)
#define SAMPLE_SUPLAY_ERR_PRINT(fmt...)         MT_SUPLAY_PRINT(" [SUPLAY][ERROR] " fmt)
#define SAMPLE_SUPLAY_WARN_PRINT(fmt...)        MT_SUPLAY_PRINT(" [SUPLAY][WARN] "  fmt)
#define SAMPLE_SUPLAY_INFO_PRINT(fmt...)        MT_SUPLAY_PRINT(" [SUPLAY][INFO] "  fmt)
#define SAMPLE_SUPLAY_DBG_PRINT(fmt...)         MT_SUPLAY_PRINT(" [SUPLAY][DEBUG] " fmt)


#define SAMPLE_SUPLAY_PRINT  printf
#define DMX_ID_0 0
#define INVALID_TSPID (0x1fff)
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
#define FILENUM             64
/*************************** Structure Definition ****************************/
typedef struct tagSource_Param_T
{
    mt_u8 FileName[256];
    mt_u8 path[256];
    MT_BOOL iscycle;
}source_param_t;

typedef struct
{
    MT_HANDLE hAvPlay;
    MT_HANDLE hWin;
    MT_HANDLE hSoundTrack;
    MT_HANDLE hPlayer;
    MT_BOOL   iscycle;
    pthread_t stInjectTSThread;
    pthread_t stPlayTimeTSThread;
} MT_RATIO_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_BOOL g_bPrintQuit = MT_TRUE;
static MT_BOOL g_bPlayRunning = MT_FALSE;
static MT_RATIO_RUN_INFO suplay_run_info;
//static MT_S32 g_speed = 1;
MT_S32 g_bquit;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

/******************************* API declaration *****************************/
extern  void file_seq_avplay_handle_set(MT_HANDLE handle_avplay, int handle_track);
#ifdef MT_SAMPLE_APP
MT_S32 MT_SuPlayMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static MT_BOOL isDirectoryExists(const char *path)
{
    struct stat info;

    if(stat(path, &info) != 0)
    {
        //The folder does not exist
        return MT_FALSE;
    }
    else if
    (info.st_mode & S_IFDIR)
    {
        //It's a folder
        return MT_TRUE;
    }
    else
    {
        //Not a folder
        return MT_FALSE;
    }
}

static MT_U32 MT_Suplay_Event_STATE_CHANGED_Data(MT_HANDLE hPlayer, MT_SVR_PLAYER_STATE_E *pData)
{

    if (*pData == MT_SVR_PLAYER_STATE_STOP || *pData == MT_SVR_PLAYER_STATE_ES_TASK_EXIT) {
        g_bPlayRunning = MT_FALSE;
    }

    return MT_SUCCESS;
}

static MT_U32 MT_Suplay_Event_ERROR_Data(MT_HANDLE hPlayer, MT_SVR_PLAYER_ERROR_E *pData)
{

    if (*pData == MT_SVR_PLAYER_ERROR_UNKNOW) {
        g_bPlayRunning = MT_FALSE;
    }

    return MT_SUCCESS;
}


static MT_U32 MT_Suplay_Event_SEEK_FINISHED_Data(MT_HANDLE hPlayer, MT_SVR_PLAYER_STATE_E *pData)
{

    SAMPLE_SUPLAY_INFO_PRINT("MT_SVR_PLAYER_EVENT_SEEK_FINISHED date %d\n", *pData);
    return MT_SUCCESS;
}



static MT_U32  MT_Suplay_Event_Callback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent)
{

    MT_BOOL muteStatus = MT_FALSE;

    switch(pstruEvent->eEvent)
    {
        case MT_SVR_PLAYER_EVENT_STATE_CHANGED:

            MT_Suplay_Event_STATE_CHANGED_Data(hPlayer, (MT_SVR_PLAYER_STATE_E *)pstruEvent->pu8Data);
            break;
        case MT_SVR_PLAYER_EVENT_SOF:
            break;
        case MT_SVR_PLAYER_EVENT_EOF:
            SAMPLE_SUPLAY_INFO_PRINT("\n\n EOF\n");
            break;
        case MT_SVR_PLAYER_EVENT_SEEK_FINISHED: {
            MT_Suplay_Event_SEEK_FINISHED_Data(hPlayer, (MT_SVR_PLAYER_STATE_E *)pstruEvent->pu8Data);
            break;
        }
        case MT_SVR_PLAYER_EVENT_ERROR: {
            MT_Suplay_Event_ERROR_Data(hPlayer, (MT_SVR_PLAYER_ERROR_E *)pstruEvent->pu8Data);
            break;
        }
        case MT_SVR_PLAYER_EVENT_TRICKMODE_ENTER:
            MT_UNF_SND_GetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, &muteStatus);
            SAMPLE_SUPLAY_INFO_PRINT("current muteStatus:%d", muteStatus);
            MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, MT_TRUE);
            break;
        case MT_SVR_PLAYER_EVENT_TRICKMODE_LEAVE:
            SAMPLE_SUPLAY_INFO_PRINT("Recover to muteStatus:%d", muteStatus);
            MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, muteStatus);
            break;

        default :
            break;

    }

    return  0;
}

static void MT_Suplay_GetPlayerInfo(void *args)
{
    unsigned long  cur_hour = 0;
    unsigned long  cur_min  = 0;
    unsigned long  cur_sec = 0;
    MT_SVR_PLAYER_INFO_S struInfo;

    SAMPLE_SUPLAY_FUNCTION_ENTER();

    while ((g_bPrintQuit != MT_TRUE) && (suplay_run_info.hPlayer != MT_INVALID_HANDLE)  && (g_bTaskQuit != MT_TRUE)) {
        if(g_bPlayRunning == MT_FALSE)
        {
            SAMPLE_SUPLAY_WARN_PRINT("Player not playing...\n");
            usleep(500*1000);
            continue;
        }

        MT_SVR_PLAYER_GetPlayerInfo(suplay_run_info.hPlayer, &struInfo);
        cur_hour = (struInfo.u64TimePlayed / 1000) / 3600;
        cur_min = ((struInfo.u64TimePlayed / 1000) - (cur_hour * 3600)) / 60;
        cur_sec = (struInfo.u64TimePlayed / 1000) - (cur_hour * 3600) - cur_min * 60;
        SAMPLE_SUPLAY_INFO_PRINT("playing [%ld:%ld:%ld],  duraton %d ms\n", cur_hour, cur_min, cur_sec, (struInfo.film_duration));
        MT_USLEEP(1 * 1000 * 1000);
    }

    SAMPLE_SUPLAY_FUNCTION_EXIT();

}

/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_Suplay_AvplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    MT_S32      ret = MT_SUCCESS;
    mt_handle   hAvplay = 0;
    mt_handle   hWin = 0;
    mt_handle   hsoundTrack = 0;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_SUPLAY_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_SUPLAY_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_SUPLAY_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    SAMPLE_SUPLAY_INFO_PRINT("[%s] g_hAvplay: 0x%x \n",__func__, (mt_u32)hAvplay);

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen MEDIA_CHAN_AUD failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *p_hAvplay = hAvplay;
    *P_hWin = hWin;
    *p_hSoundTrack = hsoundTrack;

#ifdef MT_SAMPLE_APP
    avplayHandle.hAvPlay = hAvplay;
    avplayHandle.hWin = hWin;
    avplayHandle.hSoundTrack = hsoundTrack;
#endif

    return MT_SUCCESS;


ERR8:
    ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_VO_DetachWindow failed.\n");
    }
ERR7:
    ret = MT_UNF_VO_DestroyWindow(hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_VO_DestroyWindow failed.\n");
    }
ERR6:
    ret = MT_UNF_SND_Detach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_SND_Detach failed.\n");
    }
ERR5:
    ret = MT_UNF_SND_DestroyTrack(hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_SND_DestroyTrack failed.\n");
    }
ERR4:
    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose for AUD failed.\n");
    }
ERR3:
    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose for VID failed.\n");
    }
ERR2:
    ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Destroy failed.\n");
    }
ERR1:
    ret = MT_UNF_AVPLAY_DeInit();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_DeInit failed.\n");
    }

    return MT_FAILURE;

}



/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32  MT_Suplay_AvplayDeinit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    MT_S32 ret = 0;


    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
    }

    ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_VO_DetachWindow failed.\n");
    }

    ret = MT_UNF_VO_DestroyWindow(hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_VO_DestroyWindow failed.\n");
    }

    ret = MT_UNF_SND_Detach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_SND_Detach failed.\n");
    }

    ret = MT_UNF_SND_DestroyTrack(hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_SND_DestroyTrack failed.\n");
    }

    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Destroy failed.\n");
    }

    ret = MT_UNF_AVPLAY_DeInit();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT(" MT_UNF_AVPLAY_DeInit failed.\n");
        return ret;
    }

    return MT_SUCCESS;


}


static MT_S32 MT_Suplay_PlayerInit(mt_handle *phPlayer)
{
    MT_S32 ret = 0;
    MT_HANDLE hPlayer = 0;
    MT_SVR_PLAYER_PARAM_S s_stParam = {0};
    mtUNF_SUPLAYER_STATUS_S *pstatus;
    MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
    mtUNF_SUPLAYER_IN_ARG_S args;

    //av sync
    memset(&SyncAttr,0,sizeof(MT_UNF_SYNC_ATTR_S));
    ret = MT_UNF_AVPLAY_GetAttr(suplay_run_info.hAvPlay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
        return ret;
    }
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    ret = MT_UNF_AVPLAY_SetAttr(suplay_run_info.hAvPlay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
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
    s_stParam.u32VDecErrCover = 0;
    s_stParam.suplayer_status = pstatus;
    s_stParam.hAVPlayer = suplay_run_info.hAvPlay;
    ret = MT_SVR_PLAYER_Create(&s_stParam, &hPlayer);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Deinit(MT_NULL);
        return ret;
    }
    ret = MT_SVR_PLAYER_RegCallback(hPlayer, MT_Suplay_Event_Callback);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(hPlayer);
        return ret;
    }
    ret = MT_SVR_PLAYER_Set_T1xbase_Num(hPlayer, 1);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(hPlayer);
        return ret;
    }

    *phPlayer = hPlayer;
    return ret;
}

static MT_VOID MT_Suplay_PlayerDeinit(mt_handle phPlayer)
{
    (MT_VOID)MT_SVR_PLAYER_Destroy(phPlayer);

    (MT_VOID)MT_SVR_PLAYER_Deinit(phPlayer);
}

/*
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_S32 MT_Suplay_Avplay_Start(mt_handle hAvplay, mt_handle hTrack, MT_CHAR *fileName, mt_handle hPlayer)
{
    MT_S32 ret = 0;
    MT_FORMAT_FILE_INFO_S *pstFileInfo = NULL;
    MT_SVR_PLAYER_MEDIA_S stMedia;

    memset(&stMedia,0,sizeof(MT_SVR_PLAYER_MEDIA_S));
    sprintf(stMedia.aszUrl, "%s", fileName);
    ret = MT_SVR_PLAYER_SetMedia(hPlayer, 0, &stMedia);
    if(MT_SUCCESS != ret)
    {
        return ret;
    }
    ret = MT_SVR_PLAYER_LOADMEDIA_GetFileInfo(hPlayer, &pstFileInfo);
    if(MT_SUCCESS == ret)
    {

        file_seq_avplay_handle_set(hAvplay, hTrack);

        SAMPLE_SUPLAY_INFO_PRINT("[%s] g_hAvplay: %x hTrack: %x \n",__func__, (mt_u32)hAvplay, (mt_u32)hTrack);

    }

    ret = MT_SVR_PLAYER_Play(hPlayer, 0);
    if(MT_SUCCESS != ret)
    {
        return ret;
    }
    g_bPlayRunning = MT_TRUE;

    SAMPLE_SUPLAY_INFO_PRINT("\n%s %d \n", __FUNCTION__, __LINE__);


    return MT_SUCCESS;
}


/*
@brief fast to paly
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_Suplay_Fast(MT_HANDLE hPlayer, MT_S32 speed)
{
    MT_S32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_TPlay(hPlayer, speed);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MT_SVR_PLAYER_Pause fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief Seek
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_Suplay_Seek(MT_HANDLE hPlayer, MT_S64 times)
{
    mt_s32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Seek(hPlayer, times);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MT_SVR_PLAYER_Seek fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}


/*
@brief resume play
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_Suplay_Resume(MT_HANDLE hPlayer)
{
    MT_S32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Resume(hPlayer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MT_SVR_PLAYER_Pause fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief Pause play
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_Suplay_Pause(MT_HANDLE hPlayer)
{
    MT_S32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Pause(hPlayer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MT_SVR_PLAYER_Pause fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_Suplay_Stop(MT_HANDLE hPlayer)
{
    MT_S32     ret = MT_SUCCESS;

    SAMPLE_SUPLAY_FUNCTION_ENTER();
    ret = MT_SVR_PLAYER_Stop(hPlayer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MT_SVR_PLAYER_Stop fail. ret = 0x%x \n", ret);
        return ret;
    }

    SAMPLE_SUPLAY_INFO_PRINT("stop to play\n");

    SAMPLE_SUPLAY_FUNCTION_EXIT();
    return MT_SUCCESS;
}

static MT_S32 MT_playerTask(MT_VOID *args)
{
    MT_S32 s32Ret = MT_SUCCESS;
    source_param_t *pstParam = (source_param_t *)(args);

    while(suplay_run_info.hPlayer != MT_INVALID_HANDLE && g_bTaskQuit != MT_TRUE)
    {
        SAMPLE_SUPLAY_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->FileName);
        s32Ret = MT_Suplay_Avplay_Start(suplay_run_info.hAvPlay, suplay_run_info.hSoundTrack, (mt_char*)pstParam->FileName, suplay_run_info.hPlayer);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MT_Suplay_Avplay_Start\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
//        MT_Suplay_Fast(suplay_run_info.hPlayer, g_speed);
        s32Ret = MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_DAC0, MT_FALSE);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MT_UNF_SND_SetMute\n");
        }
        usleep(500*1000);

        while(g_bTaskQuit != MT_TRUE && suplay_run_info.hPlayer != MT_INVALID_HANDLE)
        {
            MT_USLEEP(1*1000*1000);
            if(g_bPlayRunning == MT_FALSE)
            {
                SAMPLE_SUPLAY_INFO_PRINT("loop stop, restart\n");
                break;
            }

//            MT_USLEEP(1*1000*1000);
        }

        MT_SVR_PLAYER_Stop(suplay_run_info.hPlayer);
        SAMPLE_SUPLAY_INFO_PRINT("loop stop, restart\n");
        usleep(500*1000);
    }

    s32Ret = MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_DAC0, MT_FALSE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("failed to MT_UNF_SND_SetMute\n");
    }

    return MT_SUCCESS;
}

static MT_VOID MT_SuplayExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

//    g_speed = 1;

    if(MT_FALSE == suplay_run_info.iscycle)
    {
        pthread_join(suplay_run_info.stInjectTSThread, NULL);
    }
    else
    {
        (void)MT_Suplay_Stop(suplay_run_info.hPlayer);
    }

    (MT_VOID)MT_Suplay_PlayerDeinit(suplay_run_info.hPlayer);

    (MT_VOID)MT_Suplay_AvplayDeinit(suplay_run_info.hAvPlay, suplay_run_info.hWin, suplay_run_info.hSoundTrack);

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    memset(&suplay_run_info, 0, sizeof(suplay_run_info));
}

static MT_S32 MT_SuplayReadFilename(MT_CHAR *path, MT_S32 *num, MT_CHAR folderName[][256])
{
    MT_S32 i = 0;

    if(isDirectoryExists(path))
    {
        SAMPLE_SUPLAY_INFO_PRINT("The folder exists\n");
    }
    else
    {
        SAMPLE_SUPLAY_ERR_PRINT("The folder does not exist\n");
        return MT_FAILURE;
    }

    DIR *dir = opendir(path);//打开目录文件
    struct dirent *entry;
    while((entry = readdir(dir))!=0)
    {
        if(strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0)
        {
            continue;
        }
        sprintf(folderName[i], "%s", entry->d_name);
        i++;
    }

    *num = i;

    (MT_VOID)closedir(dir);

    return MT_SUCCESS;
}


static void MT_SuplayPrintMenu(void)
{
    SAMPLE_SUPLAY_PRINT("commond: \n");
    SAMPLE_SUPLAY_PRINT("     f : fast forward (supports customize the speed)\n");
    SAMPLE_SUPLAY_PRINT("     k : fast back (supports customize the speed)\n");
    SAMPLE_SUPLAY_PRINT("     n : Return to normal speed\n");
    SAMPLE_SUPLAY_PRINT("     p : pause \n");
    SAMPLE_SUPLAY_PRINT("     r : resume \n");
    SAMPLE_SUPLAY_PRINT("     s : seek time \n");
    SAMPLE_SUPLAY_PRINT("     a : audio track \n");
    SAMPLE_SUPLAY_PRINT("     o : print current playback time \n");
    SAMPLE_SUPLAY_PRINT("     d : check if audio dolby mono \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_SUPLAY_PRINT("     b : background run \n");
#endif
    SAMPLE_SUPLAY_PRINT("     h : help \n");
    SAMPLE_SUPLAY_PRINT("     q : quit \n");
    SAMPLE_SUPLAY_PRINT("SUPLAY>> ");

}

static void when_sigusr1_suplay(int sig)
{

    if(g_bquit == 0)
    {

        printf("destroy suplay resource \n");
        MT_SuplayExit();
        g_bquit = 1;
    }
}

static mt_s32 MT_Suplay_RestoreAc4PlayAttrInfo(mt_handle hAvplay)
{
    mt_s32    ret = MT_SUCCESS;
    play_ac4_attr_info play_ac4_info;


    MTADP_AUD_GetAc4PlayAttrInfo(&play_ac4_info);
    if (!play_ac4_info.ac4_attr_enable)
    {
        SAMPLE_SUPLAY_INFO_PRINT("no restore ac4 play attr info \n");
        return ret;
    }

    usleep(500 * 1000);
    // restore ac4    attr info.
    ret = MT_UNF_AVPLAY_StopAudDec(hAvplay);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MT_UNF_AVPLAY_StopAudDec failed.\n");
        return ret;
    }

    ret = MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MTADP_AUD_RestoreAc4PlayAttrInfo failed.\n");
    }

    ret = MT_UNF_AVPLAY_StartAudDec(hAvplay);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_SUPLAY_ERR_PRINT("call MT_UNF_AVPLAY_StartAudDec failed.\n");
        return ret;
    }

    return ret;
}

static MT_VOID MT_SuplayCmdTask(MT_S32 fileNum, MT_U8 filePath[256], MT_CHAR folderName[][256])
{
    MT_S32   s32Ret = MT_SUCCESS;
    MT_S32   i = 0;
    MT_S32   time = 0;
    MT_S32   seek = 0;
    MT_S32   speed = 0;
    MT_CHAR  inputCmd[32] = { 0 };
    MT_CHAR  fileName[256] = { 0 };
    MT_S32   track_id = 0;
    MT_SVR_PLAYER_FILE_INFO_S info = {0};

    if(MT_INVALID_HANDLE == suplay_run_info.hAvPlay)
    {
        SAMPLE_SUPLAY_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    while(1)
    {
        memset(fileName, 0, sizeof(fileName));
        (MT_VOID)MT_SuplayPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_SUPLAY_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_SUPLAY_INFO_PRINT("suplay play in back!\n");
            break;
        }
#endif
        /* pause*/
        else if('p' == inputCmd[0])
        {
            (void)MT_Suplay_Pause(suplay_run_info.hPlayer);
            continue;
        }
        /* continue*/
        else if('r' == inputCmd[0])
        {
            (void)MT_Suplay_Resume(suplay_run_info.hPlayer);

            // restore ac4    attr info.
            MT_Suplay_RestoreAc4PlayAttrInfo(suplay_run_info.hAvPlay);

            continue;
        }
        /* fast*/
        else if('f' == inputCmd[0])
        {
            SAMPLE_SUPLAY_INFO_PRINT("Please enter the speed(2: 2x, 4: 4x, 8: 8x): ");
            scanf("%d", &speed);
            getchar();

            (void)MT_Suplay_Fast(suplay_run_info.hPlayer, speed);
            continue;
        }
        else if('k' == inputCmd[0])
        {
            SAMPLE_SUPLAY_INFO_PRINT("Please enter the speed(2: 2x, 4: 4x, 8: 8x): ");
            scanf("%d", &speed);
            getchar();
//            g_speed = speed;
            (void)MT_Suplay_Fast(suplay_run_info.hPlayer, 0 - speed);
            continue;
        }
        else if('n' == inputCmd[0])
        {
            (void)MT_Suplay_Fast(suplay_run_info.hPlayer, 1);

            // restore ac4    attr info.
            MT_Suplay_RestoreAc4PlayAttrInfo(suplay_run_info.hAvPlay);
//            g_speed = 1;
            continue;
        }
        else if('s' == inputCmd[0])
        {
            SAMPLE_SUPLAY_INFO_PRINT("Please enter the seek time: ");
            scanf("%d", &seek);
            getchar();
            seek = seek * 1000;
            (void)MT_Suplay_Seek(suplay_run_info.hPlayer, seek);

            // restore ac4    attr info.
            MT_Suplay_RestoreAc4PlayAttrInfo(suplay_run_info.hAvPlay);
            continue;
        }
        else if('a' == inputCmd[0])
        {
            (MT_VOID)MT_SVR_PLAYER_Get_Media_Info(suplay_run_info.hPlayer, &info);
            SAMPLE_SUPLAY_INFO_PRINT("audio_track_num = %d \n", info.film.audio_track_num);
            for(i = 0; i < info.film.audio_track_num; i++)
            {
                SAMPLE_SUPLAY_INFO_PRINT("audio_track[%d] lang is %s \n", i, info.aud_lang[i].lang);
            }

            SAMPLE_SUPLAY_INFO_PRINT("please input track_id(0 - %d)\n", info.film.audio_track_num - 1);
            scanf("%d", &track_id);
            if(track_id > info.film.audio_track_num - 1)
            {
                SAMPLE_SUPLAY_ERR_PRINT("input audio_track_num err \n");
            }
            else
            {
                (MT_VOID)MT_SVR_PLAYER_Set_Aud_Track(suplay_run_info.hPlayer, track_id);
            }

            // restore ac4    attr info.
            MT_Suplay_RestoreAc4PlayAttrInfo(suplay_run_info.hAvPlay);

            continue;
        }
        else if('t' == inputCmd[0])
        {
            (MT_VOID)MT_Suplay_Stop(suplay_run_info.hPlayer);
//            (MT_VOID)MT_SVR_PLAYER_Destroy(suplay_run_info.hPlayer);
//          (MT_VOID)MT_SVR_PLAYER_Deinit(suplay_run_info.hPlayer);
        }
        else if('o' == inputCmd[0])
        {
            if (g_bPrintQuit != MT_FALSE) {
                SAMPLE_SUPLAY_INFO_PRINT("Turn on playback duration printing !!! \n");
                g_bPrintQuit = MT_FALSE;
                s32Ret = pthread_create(&suplay_run_info.stPlayTimeTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_Suplay_GetPlayerInfo, NULL);
                if (MT_SUCCESS != s32Ret) {
                    SAMPLE_SUPLAY_ERR_PRINT("failed to pthread_create\n");
                }
            } else {
                SAMPLE_SUPLAY_INFO_PRINT("Turn off playback duration printing !!! \n");
                g_bPrintQuit = MT_TRUE;
                pthread_join(suplay_run_info.stPlayTimeTSThread, NULL);
            }
            continue;
        }
        else if('y' == inputCmd[0])
        {
            time++;
            if(fileNum == time)
            {
                time = 0;
            }
            snprintf(fileName, sizeof(fileName), "%s/%s", filePath, folderName[time]);

            s32Ret = MT_Suplay_Avplay_Start(suplay_run_info.hAvPlay, suplay_run_info.hSoundTrack, fileName, suplay_run_info.hPlayer);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUPLAY_ERR_PRINT("failed to MT_Suplay_Avplay_Start\n");
                continue;
            }

        }
        else if('d' == inputCmd[0])
        {
            MT_UNF_AUDIOTRACK_ATTR_S trackAttr;
            memset(&trackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
            s32Ret = MT_UNF_SND_GetTrackAttr(suplay_run_info.hSoundTrack, &trackAttr);
            if (MT_SUCCESS == s32Ret && MT_FALSE != trackAttr.dolby_dd_ddp && MT_TRUE == trackAttr.dolby_dualmono)
            {
                SAMPLE_SUPLAY_INFO_PRINT("Audio dolby info: dolby[%d] Dual-Mono [1+1].\n", trackAttr.dolby_dd_ddp);
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_SUPLAY_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}


/*
@brief help
@return void
*/
static void MT_Suplay_Print_help(MT_CHAR *name)
{
    SAMPLE_SUPLAY_PRINT("Lack of parameters\n");
    SAMPLE_SUPLAY_PRINT("\nUsage:\n");
    SAMPLE_SUPLAY_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_SUPLAY_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_SUPLAY_PRINT("    -f: File path\n");
    SAMPLE_SUPLAY_PRINT("    -t: Loop playback test\n");
    SAMPLE_SUPLAY_PRINT("example:\n");
    SAMPLE_SUPLAY_PRINT("    %s -f ./1.mp4\n", name);
    SAMPLE_SUPLAY_PRINT("    %s -t ./ts_video\n", name);
}


static MT_S32 MT_SuplayCheckIpStream(char *streamName)
{
    mt_char* ipstream_identifier_head_arr[] = {"http:", "https:", "rtmp:", "rtsp:", "rtp:", "udp:"};
    mt_char* ipstream_identifier = NULL;
    mt_char str[10];

    memset(str, 0, sizeof(str));
    strncpy(str, streamName, 8);
    for (mt_s32 i = 0; i < sizeof(ipstream_identifier_head_arr) / sizeof(char*); i++)
    {
        if (strstr(str, ipstream_identifier_head_arr[i]))
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}


static MT_S32 MT_SuplayParase_args(int argc, char *argv[], source_param_t *pInparam)
{
    int opt = 0;

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_Suplay_Print_help(argv[0]);
        return MT_FAILURE;
    }


    while((opt = MTADP_Getopt(argc, argv, ":?hHf:t:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_Suplay_Print_help(argv[0]);
                return MT_FAILURE;

            case 'f':
                MTADP_Strncpy((mt_char*)pInparam->FileName, argv[2], sizeof(pInparam->FileName));
                pInparam->iscycle = MT_FALSE;
                return MT_SUCCESS;
            case 't':
                MTADP_Strncpy((mt_char*)pInparam->path, argv[2], sizeof(pInparam->path));
                pInparam->iscycle = MT_TRUE;
                return MT_SUCCESS;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_SuplayExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_Suplay_Print_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    return MT_SUCCESS;
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_SuPlayMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32     ret = MT_SUCCESS;
    MT_CHAR    folderName[FILENUM][256] = { 0 };
    MT_S32     filenum = 0;
    static source_param_t stParam = { 0 };

    signal(SIGUSR1, when_sigusr1_suplay);


    ret = MT_SuplayParase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_SUPLAY_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    suplay_run_info.iscycle = stParam.iscycle;

    if(MT_TRUE == stParam.iscycle)
    {
        ret = MT_SuplayReadFilename((mt_char*)stParam.path, &filenum, folderName);
        if(MT_FAILURE == ret)
        {
            return MT_FAILURE;
        }
        snprintf((mt_char*)stParam.FileName, sizeof(stParam.FileName), "%s/%s", stParam.path, folderName[0]);
    }

    if (g_bTaskQuit == MT_TRUE)
    {
        if (!MT_SuplayCheckIpStream((mt_char*)stParam.FileName))
        {
            FILE* file = fopen((mt_char*)stParam.FileName, "r");
            if(file != NULL)
            {
                fclose(file);
                SAMPLE_SUPLAY_INFO_PRINT("File exists.\n");
            }
            else
            {
                SAMPLE_SUPLAY_ERR_PRINT("File does not exist.\n");
                return ret;
            }
        }
    }

    SAMPLE_SUPLAY_INFO_PRINT("Play file name: %s \n", stParam.FileName);
    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MT_SYS_Init\n");
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
        }

        sleep(1);

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }
#endif

        g_bTaskQuit = MT_FALSE;

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR4;
        }

        ret = MT_Suplay_AvplayInit(&suplay_run_info.hAvPlay, &suplay_run_info.hWin, &suplay_run_info.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MT_Suplay_AvplayInit\n");
            goto ERR5;
        }

        ret = MT_Suplay_PlayerInit(&suplay_run_info.hPlayer);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPLAY_ERR_PRINT("failed to MT_Suplay_PlayerInit\n");
            goto ERR6;
        }

        if(MT_FALSE == stParam.iscycle)
        {
            ret = pthread_create(&suplay_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_playerTask, &stParam);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPLAY_ERR_PRINT("failed to pthread_create\n");
                goto ERR7;
            }
            sleep(2);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR8;
            }
        }
        else
        {
            ret = MT_Suplay_Avplay_Start(suplay_run_info.hAvPlay, suplay_run_info.hSoundTrack, (mt_char*)stParam.FileName, suplay_run_info.hPlayer);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPLAY_ERR_PRINT("failed to MT_Suplay_Avplay_Start\n");
                goto ERR7;
            }
        }

    }

    (MT_VOID)MT_SuplayCmdTask(filenum, stParam.path, folderName);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    if(g_bquit != 0)
    {
        g_bquit = 0;
        memset(&suplay_run_info, 0, sizeof(suplay_run_info));
        return MT_SUCCESS;
    }

    if (g_bPrintQuit != MT_TRUE)
    {
        g_bPrintQuit = MT_TRUE;
        pthread_join(suplay_run_info.stPlayTimeTSThread, NULL);
    }

ERR8:
    if(MT_FALSE == stParam.iscycle)
    {
        pthread_join(suplay_run_info.stInjectTSThread, NULL);
    }
    else
    {
        (void)MT_Suplay_Stop(suplay_run_info.hPlayer);
    }

ERR7:
    (MT_VOID)MT_Suplay_PlayerDeinit(suplay_run_info.hPlayer);
ERR6:
    (MT_VOID)MT_Suplay_AvplayDeinit(suplay_run_info.hAvPlay, suplay_run_info.hWin, suplay_run_info.hSoundTrack);
ERR5:
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    (MT_VOID)MTADP_Snd_DeInit();

ERR3:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&suplay_run_info, 0, sizeof(suplay_run_info));

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
#endif

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    g_bquit = 0;
    return MT_SUCCESS;
}
