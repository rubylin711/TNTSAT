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
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"

#include "mtsu_svr_player.h"
#include "suplayer_internal.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_IPSTREAM_DEBUG

#define MT_IPSTREAM_PRINT   printf
#else

#define MT_IPSTREAM_PRINT

#endif

#define SAMPLE_IPSTREAM_FUNCTION_ENTER()           MT_IPSTREAM_PRINT("[IPSTREAM][%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_IPSTREAM_FUNCTION_EXIT()            MT_IPSTREAM_PRINT("[IPSTREAM][%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_IPSTREAM_FATAL_PRINT(fmt...)        MT_IPSTREAM_PRINT(" [IPSTREAM][FATAL] " fmt)
#define SAMPLE_IPSTREAM_ERR_PRINT(fmt...)          MT_IPSTREAM_PRINT(" [IPSTREAM][ERROR] " fmt)
#define SAMPLE_IPSTREAM_WARN_PRINT(fmt...)         MT_IPSTREAM_PRINT(" [IPSTREAM][WARN] "  fmt)
#define SAMPLE_IPSTREAM_INFO_PRINT(fmt...)         MT_IPSTREAM_PRINT(" [IPSTREAM][INFO] "  fmt)
#define SAMPLE_IPSTREAM_DBG_PRINT(fmt...)          MT_IPSTREAM_PRINT(" [IPSTREAM][DEBUG] " fmt)



#define DMX_ID_0 0
#define INVALID_TSPID (0x1fff)
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef struct tagSource_Param_T
{
    mt_u8 FileName[256];
    mt_u8 Avsync;
    mt_u8 LicenseUrl[256];//for DRM stream test
}source_param_t;

typedef struct
{
    MT_HANDLE hAvPlay;
    MT_HANDLE hWin;
    MT_HANDLE hSoundTrack;
    MT_HANDLE hPlayer;
} MT_IPSTREAM_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_IPSTREAM_RUN_INFO ipstream_run_info;
/******************************* API declaration *****************************/
extern void file_seq_avplay_handle_set(MT_HANDLE handle_avplay, int handle_track);
#ifdef MT_SAMPLE_APP
MT_S32 MT_IpstreamMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static void *MT_IpstreamEventStopAsync(void *args)
{
    //(void) args;
    //SAMPLE_IPSTREAM_INFO_PRINT("args=%p\n",args);
    (void) MT_SVR_PLAYER_Stop(ipstream_run_info.hPlayer);
    SAMPLE_IPSTREAM_INFO_PRINT("Stop player async\n");
    return NULL;
}

static void MT_IpstreamEventDecryptFail(
    MT_HANDLE hPlayer, MT_SVR_PLAYER_ERROR_E *data)
{
    if (*data != MT_SVR_PALYE_DRV_DECRYPT_FAIL) {
        return;
    }

    pthread_t tid;
    //int err = -1;
    pthread_attr_t attribs;
    pthread_attr_init(&attribs);
    pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
    //SAMPLE_IPSTREAM_INFO_PRINT("hPlayer=%p\n",(void *)hPlayer);
    (void) pthread_create(&tid, &attribs, MT_IpstreamEventStopAsync, (void *)hPlayer);
}

static MT_U32  MT_IpstreamEventCallback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent)
{
//    SAMPLE_IPSTREAM_INFO_PRINT("<%s> ,event=%d\n", __func__, pstruEvent->eEvent);

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
        case MT_SVR_PLAYER_EVENT_DRM: {
            MT_SVR_PLAYER_ERROR_E data = *pstruEvent->pu8Data;
            SAMPLE_IPSTREAM_INFO_PRINT("MT_SVR_PLAYER_EVENT_DRM:%d\n",data);
            MT_IpstreamEventDecryptFail(hPlayer, &data);
            break;
        }    
        default :
            break;
    }

    return  0;
}


/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_IpstreamAvplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = 0;
    mt_handle   hWin = 0;
    mt_handle   hsoundTrack = 0;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32DebugAudCrcBufSize = DEBUG_AUDIO_CRC_BUF_SIZE;
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    SAMPLE_IPSTREAM_INFO_PRINT("[%s] g_hAvplay: 0x%x \n",__func__, (mt_u32)hAvplay);

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen MEDIA_CHAN_AUD failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *p_hAvplay = hAvplay;
    *P_hWin = hWin;
    *p_hSoundTrack = hsoundTrack;

    return MT_SUCCESS;


ERR8:
    ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_VO_DetachWindow failed.\n");
    }
ERR7:
    ret = MT_UNF_VO_DestroyWindow(hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_VO_DestroyWindow failed.\n");
    }
ERR6:
    ret = MT_UNF_SND_Detach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_SND_Detach failed.\n");
    }
ERR5:
    ret = MT_UNF_SND_DestroyTrack(hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_SND_DestroyTrack failed.\n");
    }
ERR4:
    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose for AUD failed.\n");
    }
ERR3:
    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose for VID failed.\n");
    }
ERR2:
    ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_Destroy failed.\n");
    }
ERR1:
    ret = MT_UNF_AVPLAY_DeInit();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_DeInit failed.\n");
    }

    return MT_FAILURE;

}


static void TASK_SVR_PLAYER_GetPlayerInfo(void *args)
{
    unsigned long  cur_hour = 0;
    unsigned long  cur_min  = 0;
    unsigned long  cur_sec = 0;
    MT_SVR_PLAYER_INFO_S struInfo;
    while(g_bTaskQuit == MT_FALSE)
    {
        MT_SVR_PLAYER_GetPlayerInfo(ipstream_run_info.hPlayer, &struInfo);
        cur_hour = (struInfo.u64TimePlayed / 1000) / 3600;
        cur_min = ((struInfo.u64TimePlayed / 1000) - (cur_hour * 3600)) / 60;
        cur_sec = (struInfo.u64TimePlayed / 1000) - (cur_hour * 3600) - cur_min * 60;
        SAMPLE_IPSTREAM_INFO_PRINT("playing [%ld:%ld:%ld],  duraton %d ms\n", cur_hour, cur_min, cur_sec, (struInfo.film_duration));
        MT_USLEEP(1 * 1000 * 1000);
    }
}



/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32  MT_IpstreamAvplayDeinit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    mt_s32 ret = 0;


    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
    }

    ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_VO_DetachWindow failed.\n");
    }

    ret = MT_UNF_VO_DestroyWindow(hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_VO_DestroyWindow failed.\n");
    }

    ret = MT_UNF_SND_Detach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_SND_Detach failed.\n");
    }

    ret = MT_UNF_SND_DestroyTrack(hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_SND_DestroyTrack failed.\n");
    }

    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_Destroy failed.\n");
    }

    ret = MT_UNF_AVPLAY_DeInit();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT(" MT_UNF_AVPLAY_DeInit failed.\n");
        return ret;
    }

    return MT_SUCCESS;


}

/*
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_IpstreamAvplayStart(mt_handle hAvplay, mt_handle hTrack, source_param_t *pstParam, mt_handle *phPlayer)
{
    mt_s32 ret = 0;
    MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
    mtUNF_SUPLAYER_IN_ARG_S args;
    mtUNF_SUPLAYER_STATUS_S *pstatus;
    MT_SVR_PLAYER_PARAM_S s_stParam = {0};
    MT_HANDLE hPlayer = 0;
    MT_FORMAT_FILE_INFO_S *pstFileInfo = NULL;
    MT_SVR_PLAYER_MEDIA_S stMedia;


    //av sync
    memset(&SyncAttr,0,sizeof(MT_UNF_SYNC_ATTR_S));
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
        return ret;
    }
    //SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    SyncAttr.enSyncRef = pstParam->Avsync;
    ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
        return ret;
    }

    args.ptype = MT_SUPLAYER_MPLAYER;
    args.pri = NULL;
    pstatus = (mtUNF_SUPLAYER_STATUS_S *)MT_SVR_PLAYER_Init(&args);
    if(pstatus == NULL){
        printf("\nInit_player fail!!!\n");
        return MT_FAILURE;
    }

    memset(&s_stParam,0,sizeof(MT_SVR_PLAYER_PARAM_S));
    s_stParam.u32DmxId = DMX_ID_0;
    s_stParam.u32PortId = 3;
    s_stParam.u32MixHeight = 100;
    s_stParam.u32SndPort = 100;
    s_stParam.u32VDecErrCover = 0;
    s_stParam.suplayer_status = pstatus;
    s_stParam.audio_output = 0;
    s_stParam.pb_heap_mem_start = MT_NULL;
    s_stParam.heapMemSize = 15*1024*1024;
    ret = MT_SVR_PLAYER_Create(&s_stParam, &hPlayer);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Deinit(MT_NULL);
        return ret;
    }
    ret = MT_SVR_PLAYER_RegCallback(hPlayer, MT_IpstreamEventCallback);
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

    memset(&stMedia,0,sizeof(MT_SVR_PLAYER_MEDIA_S));
    sprintf(stMedia.aszUrl, "%s", pstParam->FileName);
    ret = MT_SVR_PLAYER_SetMedia(hPlayer, 0, &stMedia);
    if(MT_SUCCESS != ret)
    {
        (MT_VOID)MT_SVR_PLAYER_Destroy(hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(hPlayer);
        return ret;
    }
    ret = MT_SVR_PLAYER_LOADMEDIA_GetFileInfo(hPlayer, &pstFileInfo);
    if(MT_SUCCESS == ret) {

        file_seq_avplay_handle_set(hAvplay, hTrack);

        SAMPLE_IPSTREAM_INFO_PRINT("[%s] g_hAvplay: %x hTrack: %x \n",__func__, (mt_u32)hAvplay, (mt_u32)hTrack);
    }

    ret = MT_SVR_PLAYER_Play(hPlayer, 0);
    if(MT_SUCCESS != ret)
    {
        ret = MT_SVR_PLAYER_Stop(hPlayer);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("call MT_SVR_PLAYER_Stop fail. ret = 0x%x \n", ret);
        }
        (MT_VOID)MT_SVR_PLAYER_Destroy(hPlayer);
        (MT_VOID)MT_SVR_PLAYER_Deinit(hPlayer);
        return MT_FAILURE;
    }

    *phPlayer = hPlayer;

    pthread_t tid;
    pthread_attr_t attribs, attribs1;
    pthread_attr_init(&attribs);
    pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
    pthread_attr_init(&attribs1);
    pthread_attr_setdetachstate(&attribs1, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&tid, &attribs, (void *)TASK_SVR_PLAYER_GetPlayerInfo, NULL);
    if(ret != 0)
    {
        printf("can't file_seq_debug_monitor thread!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }

    SAMPLE_IPSTREAM_INFO_PRINT("\n%s %d \n", __FUNCTION__, __LINE__);


    return MT_SUCCESS;
}


/*
@brief fast to paly
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_IpstreamFast(MT_HANDLE hPlayer, mt_s32 speed)
{
    mt_s32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_TPlay(hPlayer, speed);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("call MT_SVR_PLAYER_Pause fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief resume play
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_IpstreamResume(MT_HANDLE hPlayer)
{
    mt_s32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Resume(hPlayer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("call MT_SVR_PLAYER_Pause fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief Pause play
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_IpstreamPause(MT_HANDLE hPlayer)
{
    mt_s32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Pause(hPlayer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("call MT_SVR_PLAYER_Pause fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief Seek
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_IpstreamSeek(MT_HANDLE hPlayer, MT_S64 times)
{
    mt_s32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Seek(hPlayer, times);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("call MT_SVR_PLAYER_Seek fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief Seek
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_IpstreamAudioTrack(MT_HANDLE hPlayer, MT_S32 trackId)
{
    mt_s32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Set_Aud_Track(hPlayer, trackId);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("call MT_SVR_PLAYER_Set_Aud_Track fail. ret = 0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
static MT_S32 MT_IpstreamStop(MT_HANDLE hPlayer)
{
    mt_s32     ret = MT_SUCCESS;

    ret = MT_SVR_PLAYER_Stop(hPlayer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("call MT_SVR_PLAYER_Stop fail. ret = 0x%x \n", ret);
        return ret;
    }

    SAMPLE_IPSTREAM_INFO_PRINT("stop to play\n");
    return MT_SUCCESS;
}


static MT_VOID MT_IpstreamExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

    (MT_VOID)MT_IpstreamStop(ipstream_run_info.hPlayer);

    (MT_VOID)MT_SVR_PLAYER_Destroy(ipstream_run_info.hPlayer);

    (MT_VOID)MT_SVR_PLAYER_Deinit(ipstream_run_info.hPlayer);

    (MT_VOID)MT_IpstreamAvplayDeinit(ipstream_run_info.hAvPlay, ipstream_run_info.hWin, ipstream_run_info.hSoundTrack);

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    memset(&ipstream_run_info, 0xff, sizeof(ipstream_run_info));
}


static void MT_IpstreamPrintMenu(void)
{
    MT_IPSTREAM_PRINT("commond: \n");
    MT_IPSTREAM_PRINT("     f : fast forward (supports customize the speed)\n");
    MT_IPSTREAM_PRINT("     k : fast back (supports customize the speed)\n");
    MT_IPSTREAM_PRINT("     n : Return to normal speed\n");
    MT_IPSTREAM_PRINT("     p : pause \n");
    MT_IPSTREAM_PRINT("     r : resume \n");
    MT_IPSTREAM_PRINT("     s : seek time \n");
    MT_IPSTREAM_PRINT("     a : audio track \n");
#ifdef MT_SAMPLE_APP
    MT_IPSTREAM_PRINT("     b : background run \n");
#endif
    MT_IPSTREAM_PRINT("     h : help \n");
    MT_IPSTREAM_PRINT("     q : quit \n");
    MT_IPSTREAM_PRINT("IPSTREAM>> ");

}


static MT_VOID MT_IpstreamCmdTask(MT_HANDLE hAvPlay, MT_HANDLE hWin, MT_HANDLE hPlayer)
{
    MT_S32  s32Ret = MT_SUCCESS;
    MT_S32  trackId = 0;
    MT_S32  speed = 0;
    MT_S64  times = 0;
    MT_CHAR inputCmd[32] = { 0 };

    if(MT_INVALID_HANDLE == hAvPlay || MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }



    while(1)
    {
        (MT_VOID)MT_IpstreamPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_IPSTREAM_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_IPSTREAM_INFO_PRINT("ipstream play in back!\n");
            break;
        }
    #endif
        /* pause*/
        else if('p' == inputCmd[0])
        {
            (void)MT_IpstreamPause(hPlayer);
            continue;
        }
        /* continue*/
        else if('r' == inputCmd[0])
        {
            (void)MT_IpstreamResume(hPlayer);
            continue;
        }
        /* fast*/
        else if('f' == inputCmd[0])
        {
            MT_IPSTREAM_PRINT("Please enter the speed(speed > 0): ");
            scanf("%d", &speed);
            getchar();
            s32Ret = MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_DAC0, MT_TRUE);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_IPSTREAM_ERR_PRINT("failed to MT_UNF_SND_SetMute\n");
            }
            (void)MT_IpstreamFast(hPlayer, speed);
            continue;
        }
        else if('k' == inputCmd[0])
        {
            MT_IPSTREAM_PRINT("Please enter the speed(speed < 0): ");
            scanf("%d", &speed);
            getchar();
            (void)MT_IpstreamFast(hPlayer, speed);
            continue;
        }
        else if('n' == inputCmd[0])
        {
            s32Ret = MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_DAC0, MT_FALSE);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_IPSTREAM_ERR_PRINT("failed to MT_UNF_SND_SetMute\n");
            }
            (void)MT_IpstreamFast(hPlayer, 1);
            continue;
        }
        else if('s' == inputCmd[0])
        {
            MT_IPSTREAM_PRINT("Please enter the seek time: ");
            scanf("%lld", &times);
            getchar();
            times = times * 1000;
            (void)MT_IpstreamSeek(hPlayer, times);
            continue;
        }
        else if('a' == inputCmd[0])
        {
            MT_IPSTREAM_PRINT("Please enter the audio track you want to play: ");
            scanf("%d", &trackId);
            getchar();
            (void)MT_IpstreamAudioTrack(hPlayer, trackId);
            continue;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_IPSTREAM_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}


/*
@brief help
@return void
*/
static void MT_IpstreamPrint_Help(MT_CHAR *name)
{
    MT_IPSTREAM_PRINT("Lack of parameters\n");
    MT_IPSTREAM_PRINT("\nUsage:\n");
    MT_IPSTREAM_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    MT_IPSTREAM_PRINT("    -q: Exit the background\n");
#endif
    MT_IPSTREAM_PRINT("    -f: Url path -s: avsync(1 or 0) -l: License url for DRM\n");
    MT_IPSTREAM_PRINT("example:\n");
    MT_IPSTREAM_PRINT("    %s -f http://10.48.144.87:8100/HLS_VOD/2M/H264_AAC_2M.m3u8\n", name);
    MT_IPSTREAM_PRINT("    %s -f http://10.48.144.87:8100/HLS_VOD/2M/H264_AAC_2M.m3u8 -s 1\n", name);
    MT_IPSTREAM_PRINT("    %s -f https://xxxx/xxx.mpd -s 1 -l https://xxx/xxx_xxxx\n", name);
}



static mt_s32 MT_IpstreamParase_args(int argc, char *argv[], source_param_t *pInparam)
{
    int opt = 0;

    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_IpstreamPrint_Help(argv[0]);
        return MT_FAILURE;
    }
    
    while((opt = MTADP_Getopt(argc, argv, ":?hHf:s:l:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_IpstreamPrint_Help(argv[0]);
                return MT_FAILURE;

            case 'f':
                MTADP_Strncpy((mt_char*)pInparam->FileName, mt_optarg, sizeof(pInparam->FileName));
                break;
            case 's':
                pInparam->Avsync = (mt_u8)strtol(mt_optarg, 0, 0);
                SAMPLE_IPSTREAM_INFO_PRINT("set play Avsync %d \n", pInparam->Avsync);
                if(pInparam->Avsync >= MT_UNF_AVPLAY_SYNC_REF_BUTT)
                {
                    SAMPLE_IPSTREAM_ERR_PRINT("input avsync=%d error!reset to audio master\n",pInparam->Avsync);
                    pInparam->Avsync = MT_UNF_SYNC_REF_AUDIO;
                }                
                break;
            case 'l':
                MTADP_Strncpy((mt_char*)pInparam->LicenseUrl, mt_optarg, sizeof(pInparam->LicenseUrl));
                SAMPLE_IPSTREAM_INFO_PRINT("set play LicenseUrl %s \n", pInparam->LicenseUrl);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_IpstreamExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_IpstreamPrint_Help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_IpstreamMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    mt_s32     ret = MT_SUCCESS;
    struct timespec start, end;
    double elapsed_seconds;
    source_param_t stParam = { 0 };
    mt_u32 lurl_len = 0;
    
    memset(&stParam,0,sizeof(source_param_t));
    stParam.Avsync = MT_UNF_SYNC_REF_AUDIO; //default audio master
    
    ret = MT_IpstreamParase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_IPSTREAM_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    lurl_len = (mt_u32)strlen((char *)stParam.LicenseUrl);
    SAMPLE_IPSTREAM_INFO_PRINT("play FileName %s \n", stParam.FileName);
    SAMPLE_IPSTREAM_INFO_PRINT("play Avsync %d \n", stParam.Avsync);
    SAMPLE_IPSTREAM_INFO_PRINT("play lurl_len %d sizeof LicenseUrl=%d\n", lurl_len,(mt_u32)sizeof(stParam.LicenseUrl));
    if(lurl_len)
    {
        SAMPLE_IPSTREAM_INFO_PRINT("play LicenseUrl %s \n", stParam.LicenseUrl);
        setenv("LICENSE_URL", (char *)stParam.LicenseUrl, 1);
        SAMPLE_IPSTREAM_INFO_PRINT("License URL:%s \n", getenv("LICENSE_URL"));
    }
    else
    {
        unsetenv("LICENSE_URL");
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("failed to MT_SYS_Init\n");
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
        }

        sleep(1);

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }
#endif
        clock_gettime(CLOCK_MONOTONIC, &start);
        g_bTaskQuit = MT_FALSE;

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR4;
        }

        ret = MT_IpstreamAvplayInit(&ipstream_run_info.hAvPlay, &ipstream_run_info.hWin, &ipstream_run_info.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("failed to MT_IpstreamAvplayInit\n");
            goto ERR5;
        }

        ret = MT_IpstreamAvplayStart(ipstream_run_info.hAvPlay, ipstream_run_info.hSoundTrack, &stParam, &ipstream_run_info.hPlayer);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_IPSTREAM_ERR_PRINT("failed to MT_IpstreamAvplayStart\n");
            goto ERR6;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        MT_IPSTREAM_PRINT("Search time = %lf s\n", elapsed_seconds);
    }

    (MT_VOID)MT_IpstreamCmdTask(ipstream_run_info.hAvPlay, ipstream_run_info.hWin, ipstream_run_info.hPlayer);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    (void)MT_IpstreamStop(ipstream_run_info.hPlayer);



    (MT_VOID)MT_SVR_PLAYER_Destroy(ipstream_run_info.hPlayer);
    (MT_VOID)MT_SVR_PLAYER_Deinit(ipstream_run_info.hPlayer);
ERR6:
    (MT_VOID)MT_IpstreamAvplayDeinit(ipstream_run_info.hAvPlay, ipstream_run_info.hWin, ipstream_run_info.hSoundTrack);
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
    memset(&ipstream_run_info, 0xff, sizeof(ipstream_run_info));

    return MT_SUCCESS;
}
