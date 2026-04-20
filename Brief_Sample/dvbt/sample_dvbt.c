/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_search.h"
#include "mt_cmdline.h"
#include "mt_adp_pvr.h"
#include "mt_unf_gpio.h"
#include "mt_unf_misc.h"

/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DVBT_DEBUG

#define MT_DVBT_PRINT   printf
#else

#define MT_DVBT_PRINT

#endif

#define SAMPLE_DVBT_FUNCTION_ENTER()    MT_DVBT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DVBT_FUNCTION_EXIT()     MT_DVBT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DVBT_FATAL_PRINT(fmt...)         MT_DVBT_PRINT(" [FATAL] " fmt)
#define SAMPLE_DVBT_ERR_PRINT(fmt...)           MT_DVBT_PRINT(" [ERROR] " fmt)
#define SAMPLE_DVBT_WARN_PRINT(fmt...)          MT_DVBT_PRINT(" [WARN] "  fmt)
#define SAMPLE_DVBT_INFO_PRINT(fmt...)          MT_DVBT_PRINT(" [INFO] "  fmt)
#define SAMPLE_DVBT_DBG_PRINT(fmt...)           MT_DVBT_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DVBT_PRINT   printf

#define DMX_ID_0    0
#define TUNER_ID_2    2
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2
/*************************** Structure Definition ****************************/
typedef struct
{
    mt_u32 tuner_id;
    mt_u32 freq; /**<Frequency, in kHz*/
    mt_u32 bandwidth; /**<Symbol rate, in bit/s*/
} mt_input_dvbt_para_t;
typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    mt_input_dvbt_para_t para;
    PMT_COMPACT_TBL *pProgTbl;
} MT_DVBT_RUN_INFO;

typedef enum ANTENNA_TEST_TYPE{
    ANTENNA_POWER_OFF_TEST = 0,
    ANTENNA_POWER_ON_TEST = 1,
    ANTENNA_SHORT_PROTECT_TEST = 2,
    ANTENNA_AUTO_PROTECT_TEST = 3
} ANTENNA_TEST_TYPE;

/********************** Global Variable declaration **************************/

static MT_DVBT_RUN_INFO    g_stDvbtRunInfo = {MT_INVALID_HANDLE};
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_BOOL g_stop_antenna_thread = MT_FALSE;
static MT_BOOL g_antenna_thread_end = MT_FALSE;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif
/******************************* API declaration *****************************/

#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbtMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*****************************************************************************
 *brief init and Search
 *return ::MT_SUCCESS  success.
 *return ::other  FAILURE.
*****************************************************************************/
static mt_s32 MT_DvbtCheckParam(mt_input_dvbt_para_t *p_dvbt_in)
{
    if((p_dvbt_in->freq) > 900 || (p_dvbt_in->freq) < 50)
    {
        SAMPLE_DVBT_ERR_PRINT("freq error. freq = %d \n", p_dvbt_in->freq);
        SAMPLE_DVBT_ERR_PRINT("freq must be more than 50 and less than 900.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static MT_S32 MT_DvbtDmxInit(MT_VOID)
{
    MT_S32 ret = 0;
    mt_sys_version_s stSysChipInfo ={ 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_DVBT_ERR_PRINT("mt_sys_get_version err.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }
    if(play_resource.demux_use != MT_TRUE)
    {
        ret = MT_UNF_DMX_Init();
        if (ret != MT_SUCCESS)
        {
            SAMPLE_DVBT_ERR_PRINT("MT_UNF_DMX_Init err.\n");
            return ret;
        }
    }


    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_2);
        SAMPLE_DVBT_INFO_PRINT("Connect port 2!\n");
    }
    else
    {
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
        SAMPLE_DVBT_INFO_PRINT("Connect port 0!\n");
    }


    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("MT_UNF_DMX_AttachTSPort for REC err.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}


/*****************************************************************************
 *brief Deinit
 *return ::MT_SUCCESS  success.
 *return ::MT_FAILURE  FAILURE
*****************************************************************************/
static MT_VOID MT_DvbtDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();
}


/*****************************************************************************
 *brief Audio and video playback init
 *param[out] hWin, Pointer to the outgoing Window handle
 *param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 *param[out] phSoundTrack   Pointer to the handle of the created Track
 *return ::MT_SUCCESS  success.
 *return ::MT_FAILURE  FAILURE.
*****************************************************************************/
static mt_s32 MT_DvbtAvplayInit(mt_handle *phAvplay, mt_handle *hWin, mt_handle* phSoundTrack)
{
    mt_s32 ret = 0;
    mt_handle hAvplay = 0;
    mt_handle hwin = 0;
    mt_handle hsoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S avplayAttr= { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == hWin)
    {
        SAMPLE_DVBT_ERR_PRINT("=====hWin == NULL=====\n");
        return MT_FAILURE;
    }

    if (NULL == phAvplay)
    {
        SAMPLE_DVBT_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    if (NULL == phSoundTrack)
    {
        SAMPLE_DVBT_ERR_PRINT("=====phSoundTrack == NULL=====\n");
        return MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_Init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_Init err.\n");
        return ret;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MTADP_AVPlay_RegADecLib err.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_GetDefaultConfig(&avplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig err.\n");
        goto ERR1;
    }

    avplayAttr.u32DemuxId = DMX_ID_0;
    avplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    avplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&avplayAttr, &hAvplay);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_Create err.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen err.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen err.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hsoundTrack);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hwin);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hwin, hAvplay);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hwin, MT_TRUE);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *hWin = hwin;
    *phSoundTrack = hsoundTrack;
    return MT_SUCCESS;
ERR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hwin, hAvplay);
ERR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hwin);
ERR6:
    (MT_VOID)MT_UNF_SND_Detach(*phSoundTrack, hAvplay);
ERR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(*phSoundTrack);
ERR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
ERR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
ERR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
ERR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();
    return ret;
}


/*****************************************************************************
 *brief Audio and video playback Deinit
 *param[in] hWin, A pointer to the Window handle passed in
 *param[in] phAvplay,A pointer to the Avplay handle passed in
 *param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 *return ::MT_SUCCESS  success.
 *return ::MT_FAILURE  FAILURE.
*****************************************************************************/
static mt_void  MT_DvbtAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
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


/*****************************************************************************
 *brief Audio and video Type of decoding
 *param[in] phAvplay:: Pointer to the handle of a created AVPLAY
 *param[in] pProgInfo  Data type corresponding to an attribute ID CNcomment
 *return ::MT_SUCCESS  success.
 *return ::MT_FAILURE  FAILURE
*****************************************************************************/
static mt_s32 MT_DvbtSetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    mt_u32 u32AudType = 0;
    mt_u32 vidPid = 0;
    mt_u32 audPid = 0;
    MT_U32 PcrPid = 0;
    mt_s32 ret = 0;
    MT_UNF_VCODEC_ATTR_S vdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S adecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_UNBLANK_E unblank;


    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVBT_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    if (NULL == pProgInfo)
    {
        SAMPLE_DVBT_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return MT_FAILURE;
    }

    if (pProgInfo->VElementNum > 0 )
    {
        vidPid = pProgInfo->VElementPid;
        enVidType = pProgInfo->VideoType;
    }
    else
    {
        vidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if (pProgInfo->AElementNum > 0)
    {
        audPid  = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        audPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    if (u32AudType == HA_AUDIO_ID_PCM)
    {
        int i = 0;
        pcm_info_t pcm = { 0 };
        for (i = 0; i < pProgInfo->AElementNum; i++)
        {
            if (pProgInfo->Audioinfo[i].u16AudioPid == audPid) {
                break;
            }
        }

        pcm = pProgInfo->Audioinfo[i].pcm;
        MTADP_Set_AudPcmInfo(pcm);
    }

    PcrPid = pProgInfo->PcrPid;

    SAMPLE_DVBT_INFO_PRINT("VidPid=%x, AudPid=%x\n",vidPid, audPid);

    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
            return MT_FAILURE;
        }
    }

    if (vidPid != INVALID_TSPID)
    {
        MTADP_Get_VcodeUnblank(&unblank);
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vdecAttr);
        if (ret != MT_SUCCESS )
        {
            SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_GetAttr failed.\n");
        }

        /* The type of code stream supported by the decoder*/
        if (MT_UNF_VCODEC_TYPE_VC1 == enVidType)
        {
            vdecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = 1;
            vdecAttr.unExtAttr.stVC1Attr.u32CodecVersion = 8;
        }

        if (MT_UNF_VCODEC_TYPE_VP6 == enVidType)
        {
            vdecAttr.unExtAttr.stVP6Attr.bReversed = 0;
        }

        vdecAttr.enType = enVidType;
        vdecAttr.enUnBlank = unblank;
        vdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        vdecAttr.u32ErrCover = 100;
        vdecAttr.s32CtrlOptions = 0;
        vdecAttr.u32Priority = 3;

        vdecAttr.enType = enVidType;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vdecAttr);
        if (ret != MT_SUCCESS)
        {
            SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &vidPid);
        if (ret != MT_SUCCESS)
        {
            SAMPLE_DVBT_ERR_PRINT("call MTADP_AVPlay_SetVdecAttr failed.\n");
            return ret;
        }
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &adecAttr);
    if (ret != MT_SUCCESS )
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_UNF_AVPLAY_GetAttr failed.\n");
    }

    if (audPid != INVALID_TSPID)
    {
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &audPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",ret);
            return ret;
        }
    }

    if ((vidPid != INVALID_TSPID) || (audPid != INVALID_TSPID))
    {
        MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync = { 0 };
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
    }

    return MT_SUCCESS;
}

/*****************************************************************************
 *brief Audio and video decoding
 *param[in]  pProgInfo  Data type corresponding to an attribute ID CNcomment
 *param[out] pAvplay:: Pointer to the handle of a created AVPLAY
 *return ::MT_SUCCESS  success.
 *return ::MT_FAILURE  FAILURE
*****************************************************************************/
static mt_s32 MT_DvbtStarToPlay( mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    mt_u32 ret = MT_SUCCESS;
    mt_u32 pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S stSyncAttr = { 0 };


    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVBT_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    if (NULL == pProgInfo)
    {
        SAMPLE_DVBT_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return MT_FAILURE;
    }

    ret = MT_DvbtSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("MT_DvbtSetAvplayPidAndCodecType error.\n:%#x\n",ret);
        return ret;
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if ((MT_SUCCESS != ret) || (0x1fff == pid))
    {
        SAMPLE_DVBT_ERR_PRINT("has no audio stream!\n");
    }
    else
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if ((MT_SUCCESS != ret) || (0x1fff == pid))
    {
        SAMPLE_DVBT_ERR_PRINT("has no video stream!\n");
    }
    else
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }

    if ((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /* enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr))
        {
            SAMPLE_DVBT_ERR_PRINT("MT_UNF_AVPLAY_SetAttr error.\n");
            return MT_FAILURE;
        }

        /* enable avplay A/V sync */
        if (MT_SUCCESS != MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
        {
            SAMPLE_DVBT_ERR_PRINT("MT_UNF_AVPLAY_GetAttr error.\n");
            return MT_FAILURE;
        }

        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.stSyncStartRegion.bSmoothPlay = MT_TRUE;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
        {
            SAMPLE_DVBT_ERR_PRINT("MT_UNF_AVPLAY_SetAttr error.\n");
            return MT_FAILURE;
        }
    }

    /* start to play audio and video */
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("MT_UNF_AVPLAY_Start error.  ret=0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}


/*****************************************************************************
 *brief Stops an AVPLAY
 *param[out] phAvplay:: Pointer to the handle of a created AVPLAY
 *return :: ret.
*****************************************************************************/
static mt_s32 MT_DvbtStopToPlay(mt_handle hAvplay,MT_UNF_AVPLAY_STOP_MODE_E enmode)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    option.enMode = enmode;
    option.u32TimeoutMs = 0;

    SAMPLE_DVBT_INFO_PRINT("stop live play ...\n");

    /* stop playing audio and video */
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}

static MT_VOID MT_DvbtExit(MT_VOID)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    SAMPLE_DVBT_FUNCTION_ENTER();
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    (MT_VOID)MT_DvbtStopToPlay(g_stDvbtRunInfo.hAvPlay,1);

    (MT_VOID)MT_DvbtAvplayDeInit(g_stDvbtRunInfo.hAvPlay, g_stDvbtRunInfo.hWin, g_stDvbtRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbtRunInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)MT_DvbtDmxDeInit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_DVBT_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_DVBT_INFO_PRINT("PVR is recording now \n");
        SAMPLE_DVBT_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_2);
    }

    memset(&g_stDvbtRunInfo, 0xff, sizeof(g_stDvbtRunInfo));
    g_bTaskQuit = MT_TRUE;

    MTADP_Get_VcodeUnblank(&unblank);
    if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
    {
        MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
    }

    SAMPLE_DVBT_FUNCTION_EXIT();
}

static MT_VOID MT_DvbtPrintMenu(MT_U32 prog_num)
{

    SAMPLE_DVBT_PRINT("\n 1 - %d : select the program \n", prog_num);
#ifdef MT_SAMPLE_APP
    SAMPLE_DVBT_PRINT("     b : background run \n");
#endif

    SAMPLE_DVBT_PRINT("     p : pause \n");
    SAMPLE_DVBT_PRINT("     r : resume \n");
    SAMPLE_DVBT_PRINT("     z : Channel Switch Mode \n");
    SAMPLE_DVBT_PRINT("     k : multistream to play \n");
    SAMPLE_DVBT_PRINT("     s : signal strength \n");
    SAMPLE_DVBT_PRINT("     l : signal quality \n");
	SAMPLE_DVBT_PRINT("     i : get signal information \n");
    SAMPLE_DVBT_PRINT("     d : check if audio dolby mono \n");
    SAMPLE_DVBT_PRINT("     a : antenna short protection \n");
    SAMPLE_DVBT_PRINT("     m : set unblank screen mode\n");
    SAMPLE_DVBT_PRINT("     g : get first video frame show and avsync done cost time \n");
    SAMPLE_DVBT_PRINT("     h : help \n");
    SAMPLE_DVBT_PRINT("     q : quit \n");
    SAMPLE_DVBT_PRINT("DVBT>> ");

}

static MT_VOID MT_DvbtAntennaTestMenu(void)
{
    SAMPLE_DVBT_PRINT("****************************************\n");
    SAMPLE_DVBT_PRINT("******  Dvbt Antenna Test Menu  ********\n");
    SAMPLE_DVBT_PRINT("****************************************\n");
    SAMPLE_DVBT_PRINT("     0 : power off \n");
    SAMPLE_DVBT_PRINT("     1 : power on \n");
    SAMPLE_DVBT_PRINT("     2 : antenna short protect \n");
    SAMPLE_DVBT_PRINT("     3 : antenna auto short protect \n");
    SAMPLE_DVBT_PRINT("Antenna>> ");
}

/*****************************************************************************
 *brief Printthe info of reminding
 *param[in] name: argv[0]
 *return ::void.
*****************************************************************************/
static void MT_DvbtPrintHelp(char *name)
{
    SAMPLE_DVBT_PRINT("Options:\n"
        " ?/-h/-H               print this help\n"
        " -f <freq>             set freq (50~900) \n"
        " -p <band width>       set band width\n");
    SAMPLE_DVBT_PRINT("example: %s -f 585 -p 8 \n", name);
    SAMPLE_DVBT_PRINT("         %s -q  <exit> \n", name);
}

/*****************************************************************************
 *brief  input message
 *param[in]  argc            The number of external input parameters
 *param[in]  argv            External input parameter values
 *return ::void.
*****************************************************************************/
static MT_S32 MT_Dvbt_ParaseArgs(int argc, char *argv[], mt_input_dvbt_para_t *pInutParam)
{
    int opt = 0;
    while((opt = MTADP_Getopt(argc, argv, ":?hHf:p:q")) != -1)
    {
        switch (opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_DvbtPrintHelp(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DvbtExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                pInutParam->freq = strtol(mt_optarg, 0, 0);
                break;
            case 'p':
                pInutParam->bandwidth = strtol(mt_optarg, 0, 0);
                break;
            default:
                (MT_VOID)MT_DvbtPrintHelp(argv[0]);
                return MT_FAILURE;
        }
    }
    return MT_SUCCESS;
}

mt_void *MT_DvbtAntennaAutoProtectThread(mt_void *args)
{
    MT_UNF_GPIO_LIST_E gpio_pin = *((MT_UNF_GPIO_LIST_E *)args);
    MT_BOOL bHighVolt = MT_FALSE;
    MT_U8 times = 0;

    SAMPLE_DVBT_INFO_PRINT("Enter %s GPIO_%d \n",__FUNCTION__,gpio_pin);
    MT_UNF_GPIO_Init();
    /*Set Output Mode*/
    MT_UNF_GPIO_SetDirBit(gpio_pin,MT_FALSE);
    /*OutPut High*/
    MT_UNF_GPIO_WriteBit(gpio_pin,MT_TRUE);
    MT_USLEEP(5 * 1000);
    /*Set Intput Mode*/
    MT_UNF_GPIO_SetDirBit(gpio_pin,MT_TRUE);

    while (!g_stop_antenna_thread)
    {
        MT_UNF_GPIO_SetDirBit(gpio_pin,MT_TRUE);
        MT_UNF_GPIO_ReadBit(gpio_pin,&bHighVolt);
        if(bHighVolt == MT_FALSE)
        {
            times++;
            if(times == 3)
            {
                SAMPLE_DVBT_INFO_PRINT("Warning:ANTENNA_PROTECT happen %d times power off the antenna!!!\n",times);
                /*Set Output Mode*/
                MT_UNF_GPIO_SetDirBit(gpio_pin,MT_FALSE);
                /*OutPut Low*/
                MT_UNF_GPIO_WriteBit(gpio_pin,MT_FALSE);
                SAMPLE_DVBT_INFO_PRINT("Warning:Pls check the voltage become 0V \n");
                break;
            }
            /*Set Output Mode*/
            MT_UNF_GPIO_SetDirBit(gpio_pin,MT_FALSE);
            /*OutPut High*/
            MT_UNF_GPIO_WriteBit(gpio_pin,MT_TRUE);
            MT_USLEEP(5 * 1000);
            SAMPLE_DVBT_INFO_PRINT("ANTENNA_PROTECT happen %d times rechecking now!!\n",times);
            MT_USLEEP(1*1000*1000);
        }
        else
        {
            times = 0;
            MT_USLEEP(3*1000*1000);
        }

    }
    g_antenna_thread_end = MT_TRUE;

    return NULL;
}


/*****************************************************************************
@brief Gets the value of the key
*param[out] phAvplay,Pointer to the outgoing AVPLAY handle
@return ::void
*****************************************************************************/
static void MT_DvbtCmdTask(MT_HANDLE     hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_U8 plpNum = 0;
    mt_s32 plpId = 0;
    MT_U32 ret = 0;
    MT_CHAR inputCmd[32] ={ 0 };
    MT_U32 u32ProgNum = 1;
#ifdef MT_SAMPLE_APP
    play_resource.s32ProgNum = u32ProgNum;
#endif
    MT_CHAR *pfgetret = NULL;
    mt_u32 strength = 0;
    mt_s32 agc = 0;
    mt_u32 quality = 0;
    mt_s32 accurate_snr = 0;
    mt_u32 snr = 0;
    PMT_COMPACT_PROG *pstCurrentProgInfo = { 0 };
    MT_UNF_AVPLAY_STOP_MODE_E enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    mt_s32 unblank = MT_UNF_VCODEC_UNBLANK_STABLE;
    mt_s64 first_vid_frm_show_time, avsync_done_time;

    while(1)
    {

        (MT_VOID)MT_DvbtPrintMenu(pProgTbl->prog_num);

        pfgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret=pfgetret;

        if ('q' == inputCmd[0])
        {

            g_bTaskQuit = MT_TRUE;
            SAMPLE_DVBT_ERR_PRINT("prepare to exit!\n");
            if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
            {
                MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
            }
            break;
        }
        else if('p' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Pause(hAvPlay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBT_ERR_PRINT("MT_UNF_AVPLAY_Pause failed\n");
            }
        }
        else if('r' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Resume(hAvPlay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBT_ERR_PRINT("MT_UNF_AVPLAY_Resume failed\n");
            }
        }
        else if('s' == inputCmd[0])
        {
            memset(&strength, 0, sizeof(strength));
            memset(&agc, 0, sizeof(agc));
            ret = mt_unf_fe_get_signal_strength(TUNER_ID_2, &strength);
            ret = mt_unf_fe_get_agc(TUNER_ID_2, 0, &agc);

            SAMPLE_DVBT_PRINT("\t Signal strength = %d, agc = %d\n", strength, agc);
        }
        else if('z' == inputCmd[0])
        {
            if(enmode == MT_UNF_AVPLAY_STOP_MODE_BLACK)
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_STILL;
                SAMPLE_DVBT_INFO_PRINT("Set mode to Freeze \n");
            }
            else
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
                SAMPLE_DVBT_INFO_PRINT("Set mode to black \n");
            }

        }
        else if('l' == inputCmd[0])
        {
            memset(&snr, 0, sizeof(snr));
            memset(&quality, 0, sizeof(quality));
            memset(&accurate_snr, 0, sizeof(accurate_snr));
            ret = mt_unf_fe_get_signal_quality(TUNER_ID_2, &quality);
            ret = mt_unf_fe_get_snr(TUNER_ID_2, &snr);
            ret = mt_unf_fe_get_accurate_snr(TUNER_ID_2, &accurate_snr);

            SAMPLE_DVBT_PRINT("\t Signal quality = %d, snr = %d accurate_snr:%02d.%03d\n",
                quality, snr, accurate_snr/1000, accurate_snr%1000);
        }
		else if('i' == inputCmd[0])
		{
			ret = MTADP_Fe_Get_Signal_Info(TUNER_ID_2);
			if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBT_ERR_PRINT("MTADP_Fe_Get_Signal_Info failed\n");
            }
		}
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);


            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                ret = MT_DvbtStopToPlay(hAvPlay,enmode);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DVBT_ERR_PRINT(" MT_Dvbt_StopToPlay failed.\n");
                    continue;
                }
                SAMPLE_DVBT_INFO_PRINT("===== Start play ProgNum: %d \n", u32ProgNum);
                // restore ac4    attr info.
                MTADP_AUD_RestoreAc4PlayAttrInfo(hAvPlay);

                ret = MT_DvbtStarToPlay(hAvPlay, pstCurrentProgInfo);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_DVBT_ERR_PRINT("call SwitchProg failed.\n");
                    continue;
                }
                play_resource.s32ProgNum = u32ProgNum;
            }
            else
            {
                SAMPLE_DVBT_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }

#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(pstCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBT_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_DVBT_INFO_PRINT("Dvbt play in back!\n");
            break;
        }
#endif
        else if('k' == inputCmd[0])
        {

            ret = MT_DvbtStopToPlay(hAvPlay,enmode);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBT_ERR_PRINT(" MT_DvbsStopToPlay failed.\n");
                continue;
            }
            (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbtRunInfo.pProgTbl);
            (MT_VOID)MTADP_Search_destory_proglist();

            mt_unf_fe_get_plpnum(TUNER_ID_2, &plpNum);

            SAMPLE_DVBT_INFO_PRINT("please input you want to play plp Id(0 - %d)\n", plpNum - 1);
            scanf("%d", &plpId);
            mt_unf_fe_set_plpid(TUNER_ID_2, plpId);

            ret = MTADP_Search_GetAllPmt(DMX_ID_0, &pProgTbl);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBT_ERR_PRINT(" MTADP_Search_GetAllPmt failed.\n");
                continue;
            }

            pstCurrentProgInfo = pProgTbl->proginfo;

            ret = MT_DvbtStarToPlay(hAvPlay, pstCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBT_ERR_PRINT(" MT_DvbsStarToPlay failed.\n");
                continue;
            }

        }
        else if('d' == inputCmd[0])
        {
            MT_UNF_AUDIOTRACK_ATTR_S trackAttr;
            memset(&trackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
            ret = MT_UNF_SND_GetTrackAttr(g_stDvbtRunInfo.hSoundTrack, &trackAttr);
            if (MT_SUCCESS == ret && MT_FALSE != trackAttr.dolby_dd_ddp && MT_TRUE == trackAttr.dolby_dualmono)
            {
                SAMPLE_DVBT_INFO_PRINT("Audio dolby info: dolby[%d] Dual-Mono [1+1].\n", trackAttr.dolby_dd_ddp);
            }
        }
        else if('a' == inputCmd[0])
        {
            ANTENNA_TEST_TYPE test_type = ANTENNA_POWER_OFF_TEST;
            mt_sys_version_s stSysChipInfo;
            mt_char InputCmd[32];
            MT_UNF_GPIO_LIST_E gpio_pin = MT_UNF_GPIO_18;
            MT_BOOL bHighVolt = MT_FALSE;
            pthread_t antenna_thread = -1;

            MT_DvbtAntennaTestMenu();
            scanf("%d", (mt_s32*)&test_type);
            getchar();

            memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
            mt_sys_get_version(&stSysChipInfo);

            if ((MT_CHIP_SYMPHONY6_A0 <= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY6_MAX >= stSysChipInfo.enChipVersion))
                gpio_pin = MT_UNF_GPIO_18;

            mt_unf_misc_pinmux_set(18,0,0,1);
            switch(test_type)
            {
                case ANTENNA_POWER_OFF_TEST:
                    MT_UNF_GPIO_Init();
                    /*Set Output Mode*/
                    MT_UNF_GPIO_SetDirBit(gpio_pin,MT_FALSE);
                    /*OutPut Low*/
                    MT_UNF_GPIO_WriteBit(gpio_pin,MT_FALSE);
                    SAMPLE_DVBT_INFO_PRINT("ANTENNA_POWER_OFF_TEST Set GPIO_%d finished,Pls check the voltage become 0V\n",gpio_pin);
                    break;
                case ANTENNA_SHORT_PROTECT_TEST:
                    MT_UNF_GPIO_Init();
                    MT_UNF_GPIO_SetDirBit(gpio_pin,MT_TRUE);
                    MT_UNF_GPIO_ReadBit(gpio_pin,&bHighVolt);
                    if(bHighVolt == MT_FALSE)
                    {
                        SAMPLE_DVBT_INFO_PRINT("ANTENNA_PROTECT happen,power off the antenna!!!\n");
                        /*Set Output Mode*/
                        MT_UNF_GPIO_SetDirBit(gpio_pin,MT_FALSE);
                        /*OutPut Low*/
                        MT_UNF_GPIO_WriteBit(gpio_pin,MT_FALSE);
                        SAMPLE_DVBT_INFO_PRINT("Pls check the voltage become 0V \n");
                    }
                    else
                    {
                        SAMPLE_DVBT_INFO_PRINT("ANTENNA_PROTECT unhappen!!!\n");
                    }
                    break;
                case ANTENNA_AUTO_PROTECT_TEST:
                    g_stop_antenna_thread = MT_FALSE;
                    g_antenna_thread_end = MT_FALSE;
                    ret = pthread_create(&antenna_thread, MT_NULL, MT_DvbtAntennaAutoProtectThread, (mt_void *)&gpio_pin);
                    if (0 != ret)
                    {
                        SAMPLE_DVBT_INFO_PRINT(" pthread_create antenna_auto_protect_thread error \n");
                    }
                    while (1)
                    {
                        SAMPLE_DVBT_INFO_PRINT("****************************************\n");
                        SAMPLE_DVBT_INFO_PRINT("****  please input the q to quit!  *****\n");
                        SAMPLE_DVBT_INFO_PRINT("****************************************\n");
                        fgets(InputCmd, 30, stdin);
                        if ('q' == InputCmd[0])
                        {
                            g_stop_antenna_thread = 1;
                            SAMPLE_DVBT_INFO_PRINT("prepare to quit wait thread finish!\n");
                            while(g_antenna_thread_end != MT_TRUE)
                            {
                                MT_USLEEP(10 * 1000);
                            }
                            break;
                        }
                        MT_USLEEP(10 * 1000);
                        if(g_antenna_thread_end == MT_TRUE)
                            break;
                    }
                    SAMPLE_DVBT_INFO_PRINT("Leave Auto ANTENNA PROTECT TEST \n");
                    break;
                case ANTENNA_POWER_ON_TEST:
                default:
                    MT_UNF_GPIO_Init();
                    /*Set Output Mode*/
                    MT_UNF_GPIO_SetDirBit(gpio_pin,MT_FALSE);
                    /*OutPut High*/
                    MT_UNF_GPIO_WriteBit(gpio_pin,MT_TRUE);
                    MT_USLEEP(5 * 1000);
                    /*Set Intput Mode*/
                    MT_UNF_GPIO_SetDirBit(gpio_pin,MT_TRUE);
                    SAMPLE_DVBT_INFO_PRINT("ANTENNA_POWER_ON_TEST Set GPIO_%d finished,Pls check the voltage become 5V\n",gpio_pin);
                    break;
            }
        }
        else if ('m' == inputCmd[0])
        {
            SAMPLE_DVBT_PRINT("input unblank mode(0:fast 1:stable 2:sync):");
            scanf("%d", &unblank);
            getchar();
            SAMPLE_DVBT_PRINT("unblank: %d \n", unblank);
            unblank = unblank % MT_UNF_VCODEC_UNBLANK_BUTT;
            MTADP_Set_VcodeUnblank(unblank);
        }
        else if('g' == inputCmd[0])
        {
            (MT_VOID)MTADP_ReadPlayStat(&first_vid_frm_show_time, &avsync_done_time);
            SAMPLE_DVBT_PRINT("[time] first video frame showed cost time: %lldms \n", first_vid_frm_show_time);
            SAMPLE_DVBT_PRINT("[time] avsync done cost time: %lldms \n", avsync_done_time);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DVBT_INFO_PRINT("Print help info \n");
            continue;
        }
    }



}

#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbtMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{

    MT_S32 ret = 0;
    PMT_COMPACT_PROG *pstCurrentProgInfo = { 0 };

    if(argc != 5 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DvbtPrintHelp(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_Dvbt_ParaseArgs(argc, argv, &g_stDvbtRunInfo.para);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DVBT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DVBT_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        ret = MT_DvbtCheckParam(&g_stDvbtRunInfo.para);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MT_DvbsCheckParam failed.\n");
            return MT_FAILURE;
        }
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("mt_sys_init error. ret=0x%x \n", ret);
            return MT_FAILURE;
        }
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", ret);
            goto ERR0;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", ret);
            goto ERR1;
        }
#endif
        SAMPLE_DVBT_INFO_PRINT("---------DVBT USE TUNER_ID_2\n");
        ret = MTADP_Fe_Init(TUNER_ID_2);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("mtadp_fe_init error\n");
            goto ERR2;
        }

        ret = MTADP_Fe_Connect_Dvbt(TUNER_ID_2, g_stDvbtRunInfo.para.freq, g_stDvbtRunInfo.para.bandwidth);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("mtadp_fe_connect_dvbtauto error\n");
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MTADP_VO_Init error\n");
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("failed to DVB_Disp_Init err\n");
            goto ERR4;
        }

        ret = MT_DvbtDmxInit();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MT_DvbtDmxInit error.\n");
            goto ERR5;
        }

        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stDvbtRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
            goto ERR7;
        }

        ret = MT_DvbtAvplayInit(&g_stDvbtRunInfo.hAvPlay, &g_stDvbtRunInfo.hWin, &g_stDvbtRunInfo.hSoundTrack);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MT_DvbtAvplayInit error.\n");
            goto ERR8;
        }

#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_stDvbtRunInfo.hAvPlay;
        avplayHandle.hSoundTrack = g_stDvbtRunInfo.hSoundTrack;
        avplayHandle.hWin = g_stDvbtRunInfo.hWin;
#endif
        pstCurrentProgInfo = g_stDvbtRunInfo.pProgTbl->proginfo;    //Play the first program
        ret = MT_DvbtStarToPlay(g_stDvbtRunInfo.hAvPlay, pstCurrentProgInfo);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBT_ERR_PRINT("MT_Dvbt_StarToPlay error.\n");
            goto ERR9;
        }
        g_bTaskQuit = MT_FALSE;
        play_resource.sig_type = 2;
    }

    (MT_VOID)MT_DvbtCmdTask(g_stDvbtRunInfo.hAvPlay, g_stDvbtRunInfo.pProgTbl);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    ret = MT_DvbtStopToPlay(g_stDvbtRunInfo.hAvPlay,1);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBT_ERR_PRINT("call MT_Dvbt_StopToPlay failed.\n");
    }

ERR9:
    (MT_VOID)MT_DvbtAvplayDeInit(g_stDvbtRunInfo.hAvPlay, g_stDvbtRunInfo.hWin, g_stDvbtRunInfo.hSoundTrack);
ERR8:
    /** Release the PMT table */
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbtRunInfo.pProgTbl);
ERR7:
    (MT_VOID)MTADP_Search_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)MT_DvbtDmxDeInit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_DVBT_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_DVBT_INFO_PRINT("PVR is recording now \n");
        SAMPLE_DVBT_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

ERR5:
    (MT_VOID)MTADP_Snd_DeInit();
ERR4:
    (MT_VOID)MTADP_VO_DeInit();
ERR3:
    (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_2);
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_stDvbtRunInfo, 0xff, sizeof(g_stDvbtRunInfo));
    g_bTaskQuit = MT_TRUE;

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return 0;
}
