/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include "mt_type.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_unf_misc.h"
#include "mt_adp_pvr.h"
#include "mt_adp_demux.h"
#include "mt_cmdline.h"

/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_PVRPLAY_DEBUG
#define MT_PVRPLAY_PRINT   printf
#else
#define MT_PVRPLAY_PRINT
#endif

#define SAMPLE_PVRPLAY_FUNCTION_ENTER()             MT_PVRPLAY_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_PVRPLAY_FUNCTION_EXIT()              MT_PVRPLAY_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_PVRPLAY_FATAL_PRINT(fmt...)          MT_PVRPLAY_PRINT(" [FATAL] " fmt)
#define SAMPLE_PVRPLAY_ERR_PRINT(fmt...)            MT_PVRPLAY_PRINT(" [ERROR] " fmt)
#define SAMPLE_PVRPLAY_WARN_PRINT(fmt...)           MT_PVRPLAY_PRINT(" [WARN] "  fmt)
#define SAMPLE_PVRPLAY_INFO_PRINT(fmt...)           MT_PVRPLAY_PRINT(" [INFO] "  fmt)
#define SAMPLE_PVRPLAY_DBG_PRINT(fmt...)            MT_PVRPLAY_PRINT(" [DEBUG] " fmt)


#define SAMPLE_PVRPLAY_PRINT printf
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef struct tagCmdLinePara
{
    MT_CHAR chFileName[256];
}CMD_LINE_ST;
typedef enum mtPvrPlaySpeed
{
    MT_PVRPLAY_SPEED_NORMAL = 0,
    MT_PVRPLAY_SPEED_FAST_FORWORD_MODE = 1,
    MT_PVRPLAY_SPEED_FAST_FORWORD_2X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_4X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_8X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_16X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_32X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_MAX,

    MT_PVRPLAY_SPEED_FAST_BACKWORD_MODE = 10,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_2X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_4X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_8X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_16X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_32X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_MAX,

    MT_PVRPLAY_SPEED_SLOW_FORWORD_MODE = 20,
    MT_PVRPLAY_SPEED_SLOW_FORWORD_2X,
    MT_PVRPLAY_SPEED_SLOW_FORWORD_4X,
    MT_PVRPLAY_SPEED_SLOW_FORWORD_MAX,

    MT_PVRPLAY_SPEED_SLOW_BACKWORD_MODE = 30,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_2X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_4X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_8X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_16X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_32X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_MAX,
    MT_PVRPLAY_SPEED_MAX
}MT_PVRPLAY_SPEED_E;
typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    MT_U32             hPlayChn;
    pthread_t          stInjectTSThread;
} MT_PVR_PLAY_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
static MT_PVR_PLAY_RUN_INFO pvr_play_run_info;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_PvrPlayMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*!
@brief audio and video player initialization
@param[out] phAvplay            Handle to AV player
@param[out] phWin               The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   s32Ret = MT_FAILURE;
    MT_HANDLE                hAvplay = MT_INVALID_HANDLE;
    MT_HANDLE                hWin = MT_INVALID_HANDLE;
    MT_HANDLE                hSoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Audio decoder */
    s32Ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** AV player initialization */
    s32Ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_AVPLAY_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    s32Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Create AV player based on attributes */
    AvplayAttr.u32DemuxId = PVR_DMX_ID_REC;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    s32Ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_AVPLAY_Create failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    s32Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    s32Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_SND_CreateTrack failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    s32Ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_SND_Attach failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR5;
    }

    /** Create a window */
    s32Ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MTADP_VO_CreatWin failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    s32Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_VO_AttachWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR7;
    }

    /** Enable/disable windows */
    s32Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR8;
    }

    *phWin = hWin;
    *phSoundTrack = hSoundTrack;
    *phAvplay = hAvplay;

    return MT_SUCCESS;

ERROR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);
ERROR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);
ERROR6:
    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);
ERROR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);
ERROR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
ERROR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
ERROR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
ERROR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    return MT_FAILURE;
}


/*!
@brief audio and video player deinitialization
@param[in]  hAvplay             handle to AV player
@param[in]  hWin                Handle to window
@param[in]  hSoundTrack         Handle to sound track
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_VOID MT_PvrPlayModeAvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay || MT_INVALID_HANDLE == hWin || MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    /** Enable/disable windows */
    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);

    /** Unbind the window and AV player */
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

    /** Destroy window */
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

    /** Contact the binding of track and AV player */
    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);

    /** Destroy a Track */
    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);

    /** Turn off the video channel */
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    /** Turn off the audio channel */
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    /** Destroy the AV player */
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

    /** Deinitializes the AV player module */
    (MT_VOID)MT_UNF_AVPLAY_DeInit();
}


/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::Ret                 The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeDmxInit(MT_VOID)
{
    MT_S32 Ret = MT_FAILURE;

    Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return Ret;
    }

    Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        (MT_VOID)MT_UNF_DMX_DeInit();
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_PvrPlayModeDmxDeInit(MT_VOID)
{
    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();
}


/*!
@brief The progress of the playback
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModePlayProgress(MT_VOID *args)
{
    MT_S32                   Ret = MT_FAILURE;
    MT_U32                   *PlayChn = (MT_U32*)args;
    MT_UNF_PVR_FILE_ATTR_S   FileStatus = { 0 };
    MT_UNF_PVR_PLAY_STATUS_S PlaystStatus = { 0 };

    while(g_bTaskQuit != MT_TRUE)
    {
        Ret = MT_UNF_PVR_PlayGetStatus(*PlayChn, &PlaystStatus);
        Ret |=  MT_UNF_PVR_PlayGetFileAttr(*PlayChn, &FileStatus);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("No playback information is obtained\n");
            return Ret;
        }
        else
        {
            SAMPLE_PVRPLAY_INFO_PRINT("Playback progress = %d, %d\n", (PlaystStatus.u32CurPlayTimeInMs - FileStatus.u32StartTimeInMs) / 1000, (FileStatus.u32EndTimeInMs - FileStatus.u32StartTimeInMs) / 1000);
        }
        sleep(1);
    }
    return MT_SUCCESS;
}


/*!
@brief Fast forward to play
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeFastForwardTPlay(MT_U32 PlayChn, MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                  Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S  stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_FAST_FORWORD_2X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("2X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_4X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("4X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_8X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("8X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_16X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("16X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_32X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("32X_FAST_FORWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

static MT_S32 MT_PvrPlayModeFastForwardPlay(MT_U32 PlayChn, mt_s32 speed)
{
    MT_S32                  Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S  stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(speed)
    {
        case 2:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("2X_FAST_FORWARD\n");
            break;
        case 4:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("4X_FAST_FORWARD\n");
            break;
        case 8:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("8X_FAST_FORWARD\n");
            break;
        case 16:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("16X_FAST_FORWARD\n");
            break;
        case 32:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("32X_FAST_FORWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


static MT_S32 MT_PvrPlayModeFastBackwardPlay(MT_U32 PlayChn, mt_s32 speed)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(speed)
    {
        case 2:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("2X_FAST_BACKWARD\n");
            break;
        case 4:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("4X_FAST_BACKWARD\n");
            break;
        case 8:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("8X_FAST_BACKWARD\n");
            break;
        case 16:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("16X_FAST_BACKWARD\n");
            break;
        case 32:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("32X_FAST_BACKWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Rewind playback
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeFastBackwardTPlay(MT_U32 PlayChn, MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_2X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("2X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_4X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("4X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_8X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("8X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_16X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("16X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_32X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("32X_FAST_BACKWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Slow forward playback
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeSlowForwardTPlay(MT_U32 PlayChn, MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_2X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("2X_SLOW_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_4X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_SLOW_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("4X_SLOW_FORWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Slow rewind playback
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeSlowBackwardTPlay(MT_U32 PlayChn, MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_2X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_SLOW_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("2X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_4X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_SLOW_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("4X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_8X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_8X_SLOW_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("8X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_16X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_16X_SLOW_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("16X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_32X:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_32X_SLOW_BACKWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("32X_SLOW_BACKWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            SAMPLE_PVRPLAY_INFO_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Skip to the start position to play
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeSeekToStart(MT_U32 PlayChn)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = 0;
    stPos.s32Whence = SEEK_SET;
    Ret = MT_UNF_PVR_PlaySeek(PlayChn, &stPos);
    if(Ret != MT_SUCCESS)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}


/*!
@brief Skip to the end position to play
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeSeekToEnd(MT_U32 PlayChn)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = 0;
    stPos.s32Whence = SEEK_END;
    Ret = MT_UNF_PVR_PlaySeek(PlayChn, &stPos);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}


/*!
@brief Jump forward for 5 seconds
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeSeekForward(MT_U32 PlayChn)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = 5000;
    stPos.s32Whence = SEEK_CUR;
    Ret = MT_UNF_PVR_PlaySeek(PlayChn, &stPos);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}


/*!
@brief Jump back for 5 seconds
@param[in]  PlayChn             Play channel ID
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PvrPlayModeSeekBackward(MT_U32 PlayChn)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = -5000;
    stPos.s32Whence = SEEK_CUR;
    Ret = MT_UNF_PVR_PlaySeek(PlayChn, &stPos);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}


static MT_VOID MT_PvrPlayExit(void)
{
    g_bTaskQuit = MT_TRUE;

    pthread_join(pvr_play_run_info.stInjectTSThread, NULL);

    (MT_VOID)MTADP_PVR_StopPlayBack(pvr_play_run_info.hPlayChn);

    (MT_VOID)MT_UNF_PVR_PlayDeInit();

    /** Audio and video player deinitialization */
    (MT_VOID)MT_PvrPlayModeAvplayDeInit(pvr_play_run_info.hAvPlay, pvr_play_run_info.hWin, pvr_play_run_info.hSoundTrack);

    /** Demux module deinitialization */
    (MT_VOID)MT_PvrPlayModeDmxDeInit();

    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
    memset(&pvr_play_run_info, 0xff, sizeof(pvr_play_run_info));
}


static MT_VOID MT_PvrPlayModePrintMenu(MT_VOID)
{

    SAMPLE_PVRPLAY_PRINT("\n");
    SAMPLE_PVRPLAY_PRINT("    x: Fast forward(Manually enter multiples:2/4/8/16/32) \n");
    SAMPLE_PVRPLAY_PRINT("    c: Fast backward(Manually enter multiples:2/4/8/16/32) \n");
    SAMPLE_PVRPLAY_PRINT("    f: Fast forward\n");
    SAMPLE_PVRPLAY_PRINT("    r: Fast backward\n");
    SAMPLE_PVRPLAY_PRINT("    s: Slow forward\n");
    SAMPLE_PVRPLAY_PRINT("    g: Slow backward\n");
    SAMPLE_PVRPLAY_PRINT("    n: Normal play\n");
    SAMPLE_PVRPLAY_PRINT("    p: Pause\n");
    SAMPLE_PVRPLAY_PRINT("    k: Seek to start\n");
    SAMPLE_PVRPLAY_PRINT("    e: Seek to end\n");
    SAMPLE_PVRPLAY_PRINT("    d: Seek forward 5 second\n");
    SAMPLE_PVRPLAY_PRINT("    a: Seek rewwind 5 second\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_PVRPLAY_PRINT("    b : background run \n");
#endif
    SAMPLE_PVRPLAY_PRINT("    h : help \n");
    SAMPLE_PVRPLAY_PRINT("    q : quit \n");
    SAMPLE_PVRPLAY_PRINT("PVRPLAY>> ");

}


/*!
@brief The thread on which the command was entered
@param[in]  PlayChn             Play channel ID
@return::MT_VOID
@*/
static MT_VOID MT_PvrPlayModeCmdTask(MT_U32 PlayChn, mt_handle hAvplay)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_CHAR                    inputCmd[32] = { 0 };
    MT_BOOL                    NormalPlayStatue = MT_TRUE;
    MT_PVRPLAY_SPEED_E         PlaySpeed = MT_PVRPLAY_SPEED_NORMAL;
    mt_s32 speed = 0;


    while(1)
    {
        (MT_VOID)MT_PvrPlayModePrintMenu();
        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_PVRPLAY_INFO_PRINT("Program exits!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('x' == inputCmd[0])
        {
            SAMPLE_PVRPLAY_INFO_PRINT("Input fast forward speed (2/4/8/16/32)\n");
            scanf("%d", &speed);
            Ret = MT_PvrPlayModeFastForwardPlay(PlayChn, speed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }
            NormalPlayStatue = MT_FALSE;
        }
        else if('c' == inputCmd[0])
        {
            SAMPLE_PVRPLAY_INFO_PRINT("Input fast backward speed (2/4/8/16/32)\n");
            scanf("%d", &speed);
            Ret = MT_PvrPlayModeFastBackwardPlay(PlayChn, speed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }
            NormalPlayStatue = MT_FALSE;
        }
        else if('f' == inputCmd[0])
        {
            if(PlaySpeed > MT_PVRPLAY_SPEED_FAST_FORWORD_MODE && PlaySpeed < MT_PVRPLAY_SPEED_FAST_FORWORD_MAX - 1)
            {
                PlaySpeed++;
            }
            else
            {
                PlaySpeed = MT_PVRPLAY_SPEED_FAST_FORWORD_2X;
            }

            Ret = MT_PvrPlayModeFastForwardTPlay(PlayChn, PlaySpeed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            NormalPlayStatue = MT_FALSE;
            SAMPLE_PVRPLAY_INFO_PRINT("PVR Play fast forward to play now.\n");
            continue;
        }
        else if('r' == inputCmd[0])
        {
            if(PlaySpeed > MT_PVRPLAY_SPEED_FAST_BACKWORD_MODE && PlaySpeed < MT_PVRPLAY_SPEED_FAST_BACKWORD_MAX - 1)
            {
                PlaySpeed++;
            }
            else
            {
                PlaySpeed = MT_PVRPLAY_SPEED_FAST_BACKWORD_2X;
            }

            Ret = MT_PvrPlayModeFastBackwardTPlay(PlayChn, PlaySpeed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            NormalPlayStatue = MT_FALSE;
            SAMPLE_PVRPLAY_INFO_PRINT("PVR Play fast rewind playback now.\n");
            continue;
        }
        else if('s' == inputCmd[0])
        {
            if(PlaySpeed > MT_PVRPLAY_SPEED_SLOW_FORWORD_MODE && PlaySpeed < MT_PVRPLAY_SPEED_SLOW_FORWORD_MAX - 1)
            {
                PlaySpeed++;
            }
            else
            {
                PlaySpeed = MT_PVRPLAY_SPEED_SLOW_FORWORD_2X;
            }

            Ret = MT_PvrPlayModeSlowForwardTPlay(PlayChn, PlaySpeed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            NormalPlayStatue = MT_FALSE;
            SAMPLE_PVRPLAY_INFO_PRINT("PVR Play slow forward to play now.\n");
            continue;
        }
        else if('g' == inputCmd[0])
        {
            if(PlaySpeed > MT_PVRPLAY_SPEED_SLOW_BACKWORD_MODE && PlaySpeed < MT_PVRPLAY_SPEED_SLOW_BACKWORD_MAX - 1)
            {
                PlaySpeed++;
            }
            else
            {
                PlaySpeed = MT_PVRPLAY_SPEED_SLOW_BACKWORD_2X;
            }

            Ret = MT_PvrPlayModeSlowBackwardTPlay(PlayChn, PlaySpeed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            NormalPlayStatue = MT_FALSE;
            SAMPLE_PVRPLAY_INFO_PRINT("PVR Play slow rewind playback now.\n");
            continue;
        }
        else if('n' == inputCmd[0])
        {
            Ret = MT_UNF_PVR_PlayResumeChn(PlayChn);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                continue;
            }

            // restore ac4 play attr information.
            MTADP_PVR_RestoreAc4PlayAttrInfo(hAvplay);

            PlaySpeed = MT_PVRPLAY_SPEED_NORMAL;
            NormalPlayStatue = MT_TRUE;
            SAMPLE_PVRPLAY_INFO_PRINT("PVR Play normal play now.\n");
            continue;
        }
        else if('p' == inputCmd[0])
        {
            Ret = MT_UNF_PVR_PlayPauseChn(PlayChn);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                continue;
            }

            PlaySpeed = MT_PVRPLAY_SPEED_NORMAL;
            NormalPlayStatue = MT_FALSE;
            SAMPLE_PVRPLAY_INFO_PRINT("PVR pause now.\n");
            continue;
        }
        else if('k' == inputCmd[0])
        {
            if(MT_FALSE == NormalPlayStatue)
            {
                Ret = MT_UNF_PVR_PlayResumeChn(PlayChn);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                    continue;
                }
                NormalPlayStatue = MT_TRUE;
            }

            Ret = MT_PvrPlayModeSeekToStart(PlayChn);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            PlaySpeed = MT_PVRPLAY_SPEED_NORMAL;
            SAMPLE_PVRPLAY_INFO_PRINT("Play Seek to start.\n");
            continue;
        }
        else if('e' == inputCmd[0])
        {
            if(MT_FALSE == NormalPlayStatue)
            {
                Ret = MT_UNF_PVR_PlayResumeChn(PlayChn);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                    continue;
                }
                NormalPlayStatue = MT_TRUE;
            }

            Ret = MT_PvrPlayModeSeekToEnd(PlayChn);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            PlaySpeed = MT_PVRPLAY_SPEED_NORMAL;
            SAMPLE_PVRPLAY_INFO_PRINT("Play seek to end.\n");
            continue;
        }
        else if('d' == inputCmd[0])
        {
            if(MT_FALSE == NormalPlayStatue)
            {
                Ret = MT_UNF_PVR_PlayResumeChn(PlayChn);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                    continue;
                }
                NormalPlayStatue = MT_TRUE;
            }

            Ret = MT_PvrPlayModeSeekForward(PlayChn);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            PlaySpeed = MT_PVRPLAY_SPEED_NORMAL;
            SAMPLE_PVRPLAY_INFO_PRINT("seek forward 5 Second\n");
            continue;
        }
        else if('a' == inputCmd[0])
        {
            if(MT_FALSE == NormalPlayStatue)
            {
                Ret = MT_UNF_PVR_PlayResumeChn(PlayChn);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                    continue;
                }
                NormalPlayStatue = MT_TRUE;
            }

            Ret = MT_PvrPlayModeSeekBackward(PlayChn);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

            PlaySpeed = MT_PVRPLAY_SPEED_NORMAL;
            SAMPLE_PVRPLAY_INFO_PRINT("seek reward 5 Second\n");
            continue;
        }
    #ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_PVRPLAY_INFO_PRINT("Pvr play in back!\n");
            break;
        }
    #endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_PVRPLAY_INFO_PRINT("Print help info \n");
            continue;
        }

    }
}

/*!
@brief Help information
@param[in]  name     The executable name
@return::MT_VOID
@*/
static MT_VOID MT_PvrPlayModePrint_Help(MT_CHAR *name)
{
    SAMPLE_PVRPLAY_PRINT("Lack of parameters\n");
    SAMPLE_PVRPLAY_PRINT("\nUsage:\n");
    SAMPLE_PVRPLAY_PRINT("%s\n", name);
    SAMPLE_PVRPLAY_PRINT("    -f: File name\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_PVRPLAY_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_PVRPLAY_PRINT("example:\n");
    SAMPLE_PVRPLAY_PRINT("    %s -f ./test\n",name);
}


/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@param[out] pstCmd          Gets the address of the file path
@return::MT_VOID
@*/
static MT_S32 MT_PvrPlayModeParase_args(MT_S32 argc, MT_CHAR *argv[], CMD_LINE_ST *pstCmd)
{
    MT_S32 opt = 0;

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_PvrPlayModePrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_PvrPlayModePrint_Help(argv[0]);
                return MT_FAILURE;
            case 'f':
                strcpy(pstCmd->chFileName, argv[2]);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_PvrPlayExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_PvrPlayModePrint_Help(argv[0]);
                return MT_FAILURE;
                break;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_PvrPlayMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32                     Ret = MT_FAILURE;
    CMD_LINE_ST                cmd = { 0 };
    PMT_COMPACT_PROG *stProgInfoTmp = {0};
    PVR_PROG_INFO_S  fileInfo = {0};

    Ret = MT_PvrPlayModeParase_args(argc, argv, &cmd);
    if (MT_FAILURE == Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == Ret)
    {
        SAMPLE_PVRPLAY_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        SAMPLE_PVRPLAY_INFO_PRINT("checking idx file of '%s'...\n", cmd.chFileName);
        Ret = MTADP_PVR_checkIdx(cmd.chFileName);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("PVR_checkIdx is error\n");
            return Ret;
        }

#ifndef MT_SAMPLE_APP
        /** System initialization */
        Ret = mt_sys_init();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("failed to mt_sys_init\n");
            return Ret;
        }

        /** HDMI initialization */
        Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("failed to StartDmx\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }
#endif

        g_bTaskQuit = MT_FALSE;
        Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("call MTADP_VO_Init failed.\n");
            goto ERR2;
        }

        Ret = MTADP_Snd_Init();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("call MTADP_Snd_Init failed.\n");
            goto ERR3;
        }

        Ret = MT_PvrPlayModeDmxInit();
        if(Ret != MT_SUCCESS)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("call MT_PvrPlayModeDmxInit failed.\n");
            goto ERR4;
        }

        Ret = MT_PvrPlayModeAvplayInit(&pvr_play_run_info.hAvPlay, &pvr_play_run_info.hWin, &pvr_play_run_info.hSoundTrack);
        if(Ret != MT_SUCCESS)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("call MT_PvrPlayModeAvplayInit failed.\n");
            goto ERR5;
        }
#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = pvr_play_run_info.hAvPlay;
        avplayHandle.hWin = pvr_play_run_info.hWin;
        avplayHandle.hSoundTrack = pvr_play_run_info.hSoundTrack;
#endif

        Ret = MT_UNF_PVR_PlayInit();
        if(Ret != MT_SUCCESS)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("call MT_UNF_PVR_PlayInit failed.\n");
            goto ERR6;
        }

        Ret = MTADP_PVR_RegisterCallBacks(&pvr_play_run_info.hAvPlay);
        if(Ret != MT_SUCCESS)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("call PVR_RegisterCallBacks failed.\n");
            goto ERR7;
        }

        Ret = MTADP_PVR_StartPlayBack(cmd.chFileName, &pvr_play_run_info.hPlayChn, pvr_play_run_info.hAvPlay);
        if(Ret != MT_SUCCESS)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("call MT_PvrPlayModeStartPlayBack failed.\n");
            goto ERR7;
        }

        Ret = pthread_create(&pvr_play_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_PvrPlayModePlayProgress, &pvr_play_run_info.hPlayChn);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT("failed to pthread_create thread \n");
            goto ERR8;
        }
        SAMPLE_PVRPLAY_INFO_PRINT("Start playing...\n");

#ifdef MT_SAMPLE_APP
        stProgInfoTmp = (PMT_COMPACT_PROG*)malloc(sizeof(PMT_COMPACT_PROG));
        Ret = MTADP_PVR_GetPorgInfo(&fileInfo, cmd.chFileName);
        memcpy(stProgInfoTmp, &fileInfo.stProgInfo, sizeof(PMT_COMPACT_PROG));
        Ret = MTADP_Set_Current_Info(stProgInfoTmp);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_PVRPLAY_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
        }
#endif
    }

    (MT_VOID)MT_PvrPlayModeCmdTask(pvr_play_run_info.hPlayChn, pvr_play_run_info.hAvPlay);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifdef MT_SAMPLE_APP
    free(stProgInfoTmp);
    stProgInfoTmp = NULL;
#endif
    /** Wait for the thread to end */
    pthread_join(pvr_play_run_info.stInjectTSThread, NULL);
ERR8:
    (MT_VOID)MTADP_PVR_StopPlayBack(pvr_play_run_info.hPlayChn);

ERR7:
    (MT_VOID)MT_UNF_PVR_PlayDeInit();
ERR6:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_PvrPlayModeAvplayDeInit(pvr_play_run_info.hAvPlay, pvr_play_run_info.hWin, pvr_play_run_info.hSoundTrack);
ERR5:
    /** Demux module deinitialization */
    (MT_VOID)MT_PvrPlayModeDmxDeInit();
ERR4:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();
ERR3:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR2:
#ifndef MT_SAMPLE_APP
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&pvr_play_run_info, 0xff, sizeof(pvr_play_run_info));

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
#endif

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return MT_SUCCESS;
}

