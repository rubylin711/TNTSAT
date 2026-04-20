/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_cmdline.h"
#include "mt_adp_pvr.h"
#include "mt_unf_pm.h"
#include "mt_unf_ir.h"
#include "mt_adp_config.h"
#include "mt_cmdline.h"
#include "mt_wfd.h"

/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_MIRACAST_DEBUG
#define MT_MIRACAST_PRINT   printf
#else
#define MT_MIRACAST_PRINT
#endif

#define SAMPLE_MIRACAST_FUNCTION_ENTER()            MT_MIRACAST_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_MIRACAST_FUNCTION_EXIT()             MT_MIRACAST_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_MIRACAST_FATAL_PRINT(fmt...)         MT_MIRACAST_PRINT(" [FATAL] " fmt)
#define SAMPLE_MIRACAST_ERR_PRINT(fmt...)           MT_MIRACAST_PRINT(" [ERROR] " fmt)
#define SAMPLE_MIRACAST_WARN_PRINT(fmt...)          MT_MIRACAST_PRINT(" [WARN] "  fmt)
#define SAMPLE_MIRACAST_INFO_PRINT(fmt...)          MT_MIRACAST_PRINT(" [INFO] "  fmt)
#define SAMPLE_MIRACAST_DBG_PRINT(fmt...)           MT_MIRACAST_PRINT(" [DEBUG] " fmt)

#define SAMPLE_MIRACAST_PRINT  printf

#define DMX_ID_0           0
#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2

#define SUPPORT_AV_STATUS_MONITOR
#define DEFAULT_VIDEO_TYPE 2  /*Default 1080p@30fps*/

#ifdef SUPPORT_AV_STATUS_MONITOR
#define AV_UNSYNC_PTS 500000 //500ms
#define AV_DELAY_RATIO 10  //10%
#define AV_ADJUST_TIMEOUT 15  //15s
#endif

#define WFD_MAC_STR "%02X:%02X:%02X:%02X:%02X:%02X"
#define WFD2MAC_STR(x) (x)[0],(x)[1],(x)[2],(x)[3],(x)[4],(x)[5]
/*************************** Structure Definition ****************************/
typedef struct
{
    mt_u8 mirName[256];
    mt_s32 mirMode;
    mt_s32 mirChannel;
    mt_s32 mirVideoType;
} source_param_t;

typedef struct
{
    MT_HANDLE avplay;
    MT_HANDLE track;
    MT_HANDLE win;
    MT_HANDLE tsBuffer;
    PMT_COMPACT_TBL *progTbl;
    source_param_t param;
} mt_miracast_run_info;

typedef struct
{
    const char *video_size;
    MT_MIRACAST_VIDEO_TYPE vtype;
} mira_video_t;

#ifdef SUPPORT_AV_STATUS_MONITOR
typedef enum
{
    MS_CHECKING,
    MS_ADJUSTING
} monitor_state_e;

typedef enum
{
    AR_NONE,
    AR_AV_UNSYNC,
    AR_AV_DELAY
} adjust_reason_e;

typedef struct
{
  monitor_state_e state;
  adjust_reason_e reason;
  mt_s32 adjust_timeout; //s
}av_monitor_t;
#endif /*SUPPORT_AV_STATUS_MONITOR*/

/********************** Global Variable declaration **************************/
static MT_BOOL    g_bTaskQuit = MT_TRUE;
static mt_miracast_run_info  g_mirRunInfo = {MT_INVALID_HANDLE};

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

static mira_video_t g_mira_video_type[] =
{
    {"720P@30FPS",MIRA_VIDEO_CEA_720P_30FPS},
    {"720P@60FPS",MIRA_VIDEO_CEA_720P_60FPS},
    {"1080P@30FPS",MIRA_VIDEO_VESA_CEA_1080P_30FPS},
    {"1080P@60FPS",MIRA_VIDEO_CEA_1080P_60FPS},
#if 0
    {"2K@30FPS",MIRACAST_VIDEO_2K_30FPS},
    {"2K@60FPS",MIRACAST_VIDEO_2K_60FPS},
    {"4K@30FPS",MIRACAST_VIDEO_4K_30FPS},
    {"4K@60FPS",MIRACAST_VIDEO_4K_60FPS}
#endif
};

#ifdef SUPPORT_AV_STATUS_MONITOR
static mt_s32 g_av_state_monitor_ext;
static pthread_t g_av_state_monitor;
#endif

static MT_UNF_STREAM_BUF_S g_StreamBuf;

#ifdef MT_SAMPLE_APP
MT_S32 MT_MiracastMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/******************************* API declaration *****************************/

/*
@brief help
@return void
*/
static void MT_MiracastPrintHelp(MT_CHAR *name)
{
    SAMPLE_MIRACAST_PRINT("Lack of parameters\n");
    SAMPLE_MIRACAST_PRINT("\nUsage:\n");
    SAMPLE_MIRACAST_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_MIRACAST_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_MIRACAST_PRINT("    -n: input mircast name(ex: Montage-STB)\n");
    SAMPLE_MIRACAST_PRINT("    -m: input mircast mode(ex: 0:go 1:gc)\n");
    SAMPLE_MIRACAST_PRINT("    -c: input mircast channel(ex: 1/6/11)\n");
    SAMPLE_MIRACAST_PRINT("    -v: input mircast video type(ex: 0:720P@30FPS 1:720P@60FPS 2:1080P@30FPS 3:1080P@60FPS)\n");
    SAMPLE_MIRACAST_PRINT("example:\n");
    SAMPLE_MIRACAST_PRINT("    %s -n Montage-STB -m 0 -c 6 -v 2\n", name);
}

static MT_VOID MT_MiracastPrintMenu(void)
{
#ifdef MT_SAMPLE_APP
    SAMPLE_MIRACAST_PRINT("     b : background run \n");
#endif
    SAMPLE_MIRACAST_PRINT("     h : help \n");
    SAMPLE_MIRACAST_PRINT("     q : quit \n");
    SAMPLE_MIRACAST_PRINT("miracast>> ");
}

/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_MiracastDmxInit(MT_VOID)
{
    mt_s32  ret = MT_SUCCESS;

    SAMPLE_MIRACAST_FUNCTION_ENTER();

    ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        return ret;
    }

    ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        (MT_VOID)MT_UNF_DMX_DeInit();
        return ret;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x80000, &g_mirRunInfo.tsBuffer);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        (MT_VOID)MT_UNF_DMX_DeInit();
        return ret;
    }

    SAMPLE_MIRACAST_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static void MT_MiracastDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(g_mirRunInfo.tsBuffer);
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
    (MT_VOID)MT_UNF_DMX_DeInit();
}

/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_MiracastAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = 0;
    mt_handle   hWin = 0;
    mt_handle   hsoundTrack = 0;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if (NULL == p_hAvplay)
    {
        SAMPLE_MIRACAST_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }

    if (NULL == P_hWin)
    {
        SAMPLE_MIRACAST_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }

    if (NULL == p_hSoundTrack)
    {
        SAMPLE_MIRACAST_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *p_hAvplay = hAvplay;
    *P_hWin = hWin;
    *p_hSoundTrack = hsoundTrack;

    return MT_SUCCESS;

ERR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);
ERR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);
ERR6:
    (MT_VOID)MT_UNF_SND_Detach(hsoundTrack, hAvplay);
ERR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hsoundTrack);
ERR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
ERR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
ERR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
ERR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    return MT_FAILURE;
}

/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return void

*/
static void  MT_MiracastAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{

    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);

    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);

    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

    (MT_VOID)MT_UNF_AVPLAY_DeInit();
}

/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_MiracastSetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_U32                           PcrPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };
    MT_UNF_VCODEC_UNBLANK_E          unblank;

    SAMPLE_MIRACAST_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_MIRACAST_ERR_PRINT("The input address is empty\n");
        return MT_FAILURE;
    }

    if(pProgInfo->VElementNum > 0)
    {
        VidPid = pProgInfo->VElementPid;
        enVidType = pProgInfo->VideoType;
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if(pProgInfo->AElementNum > 0)
    {
        AudPid  = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    if (u32AudType == HA_AUDIO_ID_PCM || u32AudType == HA_AUDIO_ID_BLYRAYLPCM)
    {
        mt_s32 i = 0;
        pcm_info_t pcm = { 0 };
        for (i = 0; i < pProgInfo->AElementNum; i++)
        {
            if (pProgInfo->Audioinfo[i].u16AudioPid == AudPid) {
                break;
            }
        }

        pcm = pProgInfo->Audioinfo[i].pcm;
        MTADP_Set_AudPcmInfo(pcm);
    }

    PcrPid = pProgInfo->PcrPid;
    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
            return MT_FAILURE;
        }
    }

    if(VidPid != INVALID_TSPID)
    {
        MTADP_Get_VcodeUnblank(&unblank);
        /** Get the video properties of the AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %x\n", ret);
            return ret;
        }

        if (MT_UNF_VCODEC_TYPE_VC1 == enVidType)
        {
            VdecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = 1;
            VdecAttr.unExtAttr.stVC1Attr.u32CodecVersion = 8;
        }

        if (MT_UNF_VCODEC_TYPE_VP6 == enVidType)
        {
            VdecAttr.unExtAttr.stVP6Attr.bReversed = 0;
        }

        VdecAttr.enType = enVidType;
        VdecAttr.enUnBlank = unblank;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.s32CtrlOptions = 0;
        VdecAttr.u32Priority = 3;

        /** Set the video properties of the AV player */
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);

        /** Set the video PID properties of AV player */
        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("Set video properties or video PID property failed, ret = %x\n", ret);
            return ret;
        }
    }

    if(AudPid != INVALID_TSPID)
    {
        /** Set audio decoder properties */
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);

        /** Set the audio PID properties of AV player */
        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("Setting the decoding mode or audio PID property failed, ret = %x\n", ret);
            return ret;
        }
    }

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /** Set the audio and video synchronization properties of AV player */
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (MT_VOID *)&DmxAvsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, ret = %x\n", ret);
            return ret;
        }
    }

    SAMPLE_MIRACAST_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief start the AV playback into the start state
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_MiracastStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_MIRACAST_FUNCTION_ENTER();

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_MiracastSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("MT_MiracastSetAvplayPidAndCodecType fail! \n");
        return ret;
    }

    if (pProgInfo->AElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_MIRACAST_INFO_PRINT("has no audio info \n");
    }

    if (pProgInfo->VElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_MIRACAST_INFO_PRINT("has no vide0 info \n");
    }

    if ((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("Set frame to VO is failed, ret = %x\n", ret);
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("Get avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MIRACAST_ERR_PRINT("Set avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  =MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("MT_UNF_AVPLAY_Start failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_MIRACAST_FUNCTION_EXIT();

    return MT_SUCCESS;
}

#ifdef SUPPORT_AV_STATUS_MONITOR
static void *av_state_monitor_proc(void *arg)
{
    mt_s32 ret = 0;
    MT_UNF_AVPLAY_STATUS_INFO_S av_status;
    static av_monitor_t av_monitor;
    static const char *adjust_reason[] = {"none","AV UNSYNC","AV DELAY"};
    mt_handle handle = (mt_handle)arg;

    /* Force issue an adjust for the play start */
    memset(&av_monitor, 0x00, sizeof(av_monitor));
    av_monitor.state = MS_ADJUSTING;
    av_monitor.adjust_timeout = AV_ADJUST_TIMEOUT;
    av_monitor.reason = AR_AV_UNSYNC;

    //Play started, no start auto adjust
    while (!g_av_state_monitor_ext)
    {
        //Get av buffer info and av run state
        ret = MT_UNF_AVPLAY_GetSyncStatusInfo(handle, &av_status);
        ret |= MT_UNF_AVPLAY_GetVideoStatusInfo(handle, &av_status);
        if (ret == MT_ERR_AVPLAY_DEV_NO_INIT){
            SAMPLE_MIRACAST_ERR_PRINT("AV Not Initialized, exit!\n");
            break;
        }

        if(av_status.enRunStatus != MT_UNF_AVPLAY_STATUS_PLAY){
            usleep(1000000);
            continue;
        }

        //Currently in adjust state
        if (av_monitor.state == MS_ADJUSTING){
            if (av_monitor.adjust_timeout-- <= 0){
                //Back to checking state
                av_monitor.state = MS_CHECKING;
                SAMPLE_MIRACAST_INFO_PRINT("AV STATE MONITOR: Adjust for [%s] done\n", adjust_reason[av_monitor.reason]);
            }
            else
            {
                usleep(1000000);
            }
            continue;
        }

        //Currently in checking state
#if 0
        //2. Check AV sync
        if((av_status.stSyncStatus.s64DiffAvPlayTime >= AV_UNSYNC_PTS) ||
        (av_status.stSyncStatus.s64DiffAvPlayTime <= 0-AV_UNSYNC_PTS)){
        //Trigger an adjust since av sync pts out of range
        av_monitor.state = MS_ADJUSTING;
        av_monitor.adjust_timeout = AV_ADJUST_TIMEOUT;
        av_monitor.reason = AR_AV_UNSYNC;
        printf("AV STATE MONITOR: AV PTS:%lld,Trigger Adjust By AV UNSYNC\n",av_status.stSyncStatus.s64DiffAvPlayTime);

        MT_UNF_AVPLAY_Reset(g_hAvplay, NULL);
        continue;
        }
#endif
        //1. If av sync is ok, then check delay
        if (av_status.stBufStatus[0].u32UsedSize > av_status.stBufStatus[0].u32BufSize/AV_DELAY_RATIO)
        {
            //Trigger an adjust since video buffered AV_DELAY_RATIO% data
            av_monitor.state = MS_ADJUSTING;
            av_monitor.adjust_timeout = AV_ADJUST_TIMEOUT;
            av_monitor.reason = AR_AV_DELAY;
            SAMPLE_MIRACAST_INFO_PRINT("AV STATE MONITOR: VIDEO BUFFERED DATA:%d,Trigger Adjust By AV DELAY\n",av_status.stBufStatus[0].u32UsedSize);

            MT_UNF_AVPLAY_Reset(handle, NULL);
            continue;
        }
        usleep(1000000);
    }

    return NULL;
}
#endif

static void MT_MiracastUserCallback(MT_MIRACAST_EVENT_E event, void* arg)
{
    switch (event)
    {
        case MIRA_EVT_REMOVED:
            SAMPLE_MIRACAST_INFO_PRINT("P2P Device Removed!\n");
            MT_Miracast_Stop();
            MT_Miracast_Deinit();
            break;
        case MIRA_EVT_REQEUST_CONNECT:
            {
                arg_evt_req_to_connect_t *req_connect = (arg_evt_req_to_connect_t *)arg;
                SAMPLE_MIRACAST_INFO_PRINT("Peer %s Request to connect!\n", req_connect->name);
            }
            break;
        case MIRA_EVT_CONNECTED:
            {
            arg_evt_connected_t *connected = (arg_evt_connected_t *)arg;
            SAMPLE_MIRACAST_INFO_PRINT("Peer "WFD_MAC_STR" Connected,IP:%d.%d.%d.%d!\n", WFD2MAC_STR(connected->mac),
                connected->ip[0], connected->ip[1], connected->ip[2], connected->ip[3]);

#if defined(SUPPORT_AV_STATUS_MONITOR)
            g_av_state_monitor_ext = 0;
            pthread_create(&g_av_state_monitor, NULL, av_state_monitor_proc, (void *)g_mirRunInfo.avplay);
#endif
            }
            break;
        case MIRA_EVT_DISCONNECTED:
            {
            SAMPLE_MIRACAST_INFO_PRINT("Peer Disconnected\n");
#if defined(SUPPORT_AV_STATUS_MONITOR)
            g_av_state_monitor_ext = 1;
#endif
            }
            break;
        default:
            break;
    }
}

/*!
@brief audio and video player initialization
@param[out] hWin                the input window handler
@param[out] phAvplay            Handle to AV player
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_MIRACAST_AV_ERR_CODE_E symphony_linux_av_init(int max_res,int max_fr)
{
    mt_s32 ret = 0;

#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MT_SYS_Init\n");
        return ret;
    }

    ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MTADP_HDMI_Init\n");
        goto ERR1;
    }

    ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MTADP_Disp_Init\n");
        goto ERR2;
    }
#endif

    ret = MTADP_Snd_Init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MTADP_Snd_Init\n");
        goto ERR3;
    }

    ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MTADP_VO_Init\n");
        goto ERR4;
    }

    ret = MT_MiracastDmxInit();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MT_MiracastDmxInit\n");
        goto ERR5;
    }

    ret = MT_MiracastAVplayInit(&g_mirRunInfo.avplay, &g_mirRunInfo.win, &g_mirRunInfo.track);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to StartAVPlay\n");
        goto ERR6;
    }

#ifdef MT_SAMPLE_APP
    avplayHandle.hAvPlay = g_mirRunInfo.avplay;
    avplayHandle.hSoundTrack = g_mirRunInfo.track;
    avplayHandle.hWin = g_mirRunInfo.win;
#endif

    return MIRA_AV_SUCCESS;

ERR6:
    (MT_VOID)MT_MiracastDmxDeInit();
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

    return MIRA_AV_ERR_OTHER;
}

static MT_MIRACAST_AV_ERR_CODE_E symphony_linux_av_deinit(void)
{
    MT_MiracastAvplayDeInit(g_mirRunInfo.avplay, g_mirRunInfo.win, g_mirRunInfo.track);
    (MT_VOID)MTADP_Search_FreeAllPmt(g_mirRunInfo.progTbl);
    (MT_VOID)MTADP_Search_DeInit();
    (MT_VOID)MT_MiracastDmxDeInit();
    (MT_VOID)MTADP_VO_DeInit();
    (MT_VOID)MTADP_Snd_DeInit();
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
    (MT_VOID)mt_sys_deinit();
#endif

    return MIRA_AV_SUCCESS;
}

static MT_MIRACAST_AV_ERR_CODE_E symphony_linux_av_start(void)
{
    mt_s32 ret = 0;
    PMT_COMPACT_PROG *currentProgInfo = { 0 };

    (void)MTADP_Search_Init();
    ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_mirRunInfo.progTbl);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
        goto ERR1;
    }

    /* Play the first program on the program list*/
    currentProgInfo = g_mirRunInfo.progTbl->proginfo;
    ret = MT_MiracastStarToPlay(g_mirRunInfo.avplay, currentProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
        goto ERR2;
    }

    return MIRA_AV_SUCCESS;

ERR2:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_mirRunInfo.progTbl);
ERR1:
    (MT_VOID)MTADP_Search_DeInit();

    return MIRA_AV_ERR_OTHER;
}

static MT_MIRACAST_AV_ERR_CODE_E symphony_linux_av_stop(void)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    ret = MT_UNF_DMX_ResetTSBuffer(g_mirRunInfo.tsBuffer);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MT_UNF_DMX_ResetTSBuffer\n");
    }

    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(g_mirRunInfo.avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MIRACAST_ERR_PRINT("failed to MT_UNF_AVPLAY_Stop\n");
        return MT_FAILURE;
    }

    return MIRA_AV_SUCCESS;
}


static MT_MIRACAST_AV_ERR_CODE_E symphony_linux_av_get_stream(int need_len, mt_mira_stream_t *mira_stream)
{
    mt_s32 Ret = 0;

    if (mira_stream->platform_priv == NULL)
    {
        mira_stream->platform_priv = (void *)&g_StreamBuf;
    }

    Ret = MT_UNF_DMX_GetTSBuffer(g_mirRunInfo.tsBuffer, need_len, &g_StreamBuf, 50);
    if (Ret == MT_ERR_DMX_NOAVAILABLE_BUF) {
        need_len >>= 1;
        return MIRA_AV_ERR_NOMEM;
    } else if (Ret == MT_ERR_DMX_NOT_INIT) {
        return MIRA_AV_ERR_OTHER;
    } else if (Ret == MT_ERR_DMX_TIMEOUT || Ret){
        return MIRA_AV_ERR_TIMEOUT;
    }

    mira_stream->data = (CHAR *)g_StreamBuf.pu8Data;

    return MIRA_AV_SUCCESS;
}

static MT_MIRACAST_AV_ERR_CODE_E symphony_linux_av_put_stream(mt_mira_stream_t *mira_stream)
{
    mt_s32 ret = 0;

    ret = MT_UNF_DMX_PutTSBuffer(g_mirRunInfo.tsBuffer, mira_stream->data_len);
    if (ret != MT_SUCCESS ){
        SAMPLE_MIRACAST_ERR_PRINT("call MT_UNF_DMX_PutTSBuffer failed.\n");
        return MIRA_AV_ERR_OTHER;
    }

    return MIRA_AV_SUCCESS;
}


static MT_S32 MT_MiracastInit(void)
{
    int ret = 0;

    mt_miracast_av_api_t av_api =
    {
        .enable_internal_demux = 0, //Enable Miracast internal TS demuxer
        .platform_av_init = &symphony_linux_av_init, //&symphony_linux_av_init, /*Miracast doesn't want to control AV init/deinit*/
        .platform_av_start = &symphony_linux_av_start,
        .platform_av_get_stream = &symphony_linux_av_get_stream,
        .platform_av_put_stream = &symphony_linux_av_put_stream,
        .platform_av_push_es = NULL,
        .platform_av_stop = &symphony_linux_av_stop,
        .platform_av_deinit = &symphony_linux_av_deinit,//symphony_linux_av_deinit, /*Miracast doesn't want to control AV init/deinit*/
    };

    mt_miracast_config_t mira_param =
    {
        .edid_file = NULL,
        .rtp_port = 1028,
        .hdcp2_port = 1030,
        .p2p_go = P2P_MODE_GO,
        .video_size = g_mira_video_type[DEFAULT_VIDEO_TYPE].vtype,
        .wps_method = 0,
        .channel = 1,
        .ssid = "Montage-STB",
        .passwd = "12345678",
        .devicename = "Montage-STB",
        .log_level = 7
    };

    mira_param.p2p_go = g_mirRunInfo.param.mirMode;
    mira_param.channel = g_mirRunInfo.param.mirChannel;
    mira_param.video_size = g_mira_video_type[g_mirRunInfo.param.mirVideoType].vtype;

    if (strlen((char*)g_mirRunInfo.param.mirName)){
        memset(mira_param.ssid, 0x00, sizeof(mira_param.ssid));
        memset(mira_param.devicename, 0x00, sizeof(mira_param.devicename));
        strncpy((char*)mira_param.ssid, (char*)g_mirRunInfo.param.mirName, strlen((char*)g_mirRunInfo.param.mirName));
        strncpy((char*)mira_param.devicename, (char*)g_mirRunInfo.param.mirName, strlen((char*)g_mirRunInfo.param.mirName));
    }

    ret = MT_Miracast_Init(&av_api, &mira_param, MT_MiracastUserCallback);
    if (ret != MIRA_SUCCESS)
    {
        SAMPLE_MIRACAST_ERR_PRINT("call MT_Miracast_Init failed.\n");
        return ret;
    }

    return MIRA_SUCCESS;
}

static MT_S32 MT_MiracastStart(void)
{
    mt_s32 ret = 0;

    ret = MT_Miracast_Start();
    if (ret != MIRA_SUCCESS)
    {
        SAMPLE_MIRACAST_ERR_PRINT("call MT_Miracast_Start failed.\n");
        return ret;
    }

    return MIRA_SUCCESS;
}

static MT_S32 MT_MiracastStop(void)
{
    mt_s32 ret = 0;

#ifdef SUPPORT_AV_STATUS_MONITOR
    g_av_state_monitor_ext = 1;
#endif

    ret = MT_Miracast_Stop();
    if (ret != MIRA_SUCCESS)
    {
        SAMPLE_MIRACAST_ERR_PRINT("call MT_Miracast_Stop failed.\n");
        return ret;
    }

    return MIRA_SUCCESS;
}

static MT_S32 MT_MiracastDeinit(void)
{
    mt_s32 ret = 0;

    ret = MT_Miracast_Deinit();
    if (ret != MIRA_SUCCESS)
    {
        SAMPLE_MIRACAST_ERR_PRINT("call MT_Miracast_Deinit failed.\n");
        return ret;
    }

    return MIRA_SUCCESS;
}

static MT_VOID MT_MiracastExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    MT_MiracastStop();
    MT_MiracastDeinit();
}

static MT_S32 MT_MiracastParase_args(int argc, char *argv[], source_param_t *pInparam)
{
    int opt = 0;

    SAMPLE_MIRACAST_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHn:m:c:v:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_MiracastPrintHelp(argv[0]);
                return MT_FAILURE;
            case 'n':
                MTADP_Strncpy((mt_char*)pInparam->mirName, argv[2], sizeof(pInparam->mirName));
                break;
            case 'm':
                pInparam->mirMode = strtol(mt_optarg, 0, 0);
                break;
            case 'c':
                pInparam->mirChannel = strtol(mt_optarg, 0, 0);
                break;
            case 'v':
                pInparam->mirVideoType = strtol(mt_optarg, 0, 0);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MiracastExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_MiracastPrintHelp(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_MIRACAST_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*
@brief quit and function
@return void
*/
static void MT_MiracastCmdTask(void)
{
    mt_s8 inputCmd[32] = { 0 };

    while (1)
    {
        (void)MT_MiracastPrintMenu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        if ('q' == inputCmd[0])
        {
            SAMPLE_MIRACAST_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_MIRACAST_INFO_PRINT("miracast in back!\n");
            break;
        }
#endif
        else if ('h' == inputCmd[0])
        {
            SAMPLE_MIRACAST_INFO_PRINT("help info\n");
        }
    }
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_MiracastMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    mt_s32                ret = MT_SUCCESS;

    if (argc != 9 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_MiracastPrintHelp(argv[0]);
        return MT_FAILURE;
    }

    ret = MT_MiracastParase_args(argc, argv, &g_mirRunInfo.param);
    if (ret == MT_FAILURE)
    {
        SAMPLE_MIRACAST_ERR_PRINT("Parase args err. stop window.\n");
        return MT_FAILURE;
    }
    else if (ret == MT_TASK_EXIT)
    {
        SAMPLE_MIRACAST_ERR_PRINT("Recv stop command. stop window.\n");
        return MT_SUCCESS;
    }

    if (g_bTaskQuit == MT_TRUE)
    {
        ret = MT_MiracastInit();
        if (ret != MIRA_SUCCESS)
        {
            SAMPLE_MIRACAST_ERR_PRINT("call MT_MiracastInit failed.\n");
            return ret;
        }

        ret = MT_MiracastStart();
        if (ret != MIRA_SUCCESS)
        {
            SAMPLE_MIRACAST_ERR_PRINT("call MT_MiracastStart failed.\n");
            goto err1;
        }

        g_bTaskQuit = MT_FALSE;
    }

    MT_MiracastCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif

    MT_MiracastStop();

err1:
    MT_MiracastDeinit();

    memset(&g_mirRunInfo, 0x00, sizeof(g_mirRunInfo));
    return MT_SUCCESS;
}
