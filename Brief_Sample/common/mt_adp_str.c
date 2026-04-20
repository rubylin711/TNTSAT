/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_unf_pm.h"
#include "mt_unf_ir.h"
#include "mt_adp_config.h"
#include "mt_cmdline.h"
#include "mt_adp_str.h"
/***************************** Macro Definition ******************************/
#ifdef MTADP_STR_DEBUG
#define MTADP_STR_PRINT   printf
#else
#define MTADP_STR_PRINT
#endif

#define MTADP_STR_FUNCTION_ENTER()  MTADP_STR_PRINT("[MTADP_STR][%s]: Enter ==>> \n", __FUNCTION__)
#define MTADP_STR_FUNCTION_EXIT()   MTADP_STR_PRINT("[MTADP_STR][%s]: Exit ==<< \n", __FUNCTION__)

#define MTADP_STR_FATAL_PRINT(fmt...)       MTADP_STR_PRINT(" [MTADP_STR][FATAL] " fmt)
#define MTADP_STR_ERR_PRINT(fmt...)         MTADP_STR_PRINT(" [MTADP_STR][ERROR] " fmt)
#define MTADP_STR_WARN_PRINT(fmt...)        MTADP_STR_PRINT(" [MTADP_STR][WARN] "  fmt)
#define MTADP_STR_INFO_PRINT(fmt...)        MTADP_STR_PRINT(" [MTADP_STR][INFO] "  fmt)
#define MTADP_STR_MT_DBG_PRINT(fmt...)      MTADP_STR_PRINT(" [MTADP_STR][DEBUG] " fmt)
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
mt_nim_play_info g_nimPlayInfo;
static mt_u32 g_sample_use_Tunerid[5] = {0xff};
static mt_str_play_status g_strPlayStatus;

/******************************* API declaration *****************************/
/*!
@brief audio and video player deinitialization
@param[in]  phAvplay            handle to AV player
@param[in]  hWin                Handle to window
@param[in]  phSoundTrack        Handle to sound track
@return::MT_VOID
@*/
static MT_VOID MTADP_STR_AvplayDeinit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    MTADP_STR_FUNCTION_ENTER();

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

    MTADP_STR_FUNCTION_EXIT();
}

/*!
@brief stop AV playback into the stop state
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MTADP_STR_StopToPlay(MT_HANDLE hAvplay, MT_UNF_AVPLAY_STOP_MODE_E enmode)
{
    MT_U32 ret = MT_FAILURE;
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    MTADP_STR_FUNCTION_ENTER();

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    MTADP_STR_PRINT("stop live play ...\n");
    option.enMode = enmode;
    option.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
    if (MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_Stop failed, ret = %x\n", ret);
        return ret;
    }

    MTADP_STR_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static mt_s32 MTADP_STR_SavePlayStatus(void)
{
    mt_s32 s32Ret = 0;
    MT_UNF_ENC_FMT_E fmt;
    MT_UNF_HDMI_ATTR_S hdmi_attr;
    mt_str_play_status play_status;

    s32Ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &fmt);
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_DISP_GetFormat failed\n");
    }

    s32Ret = MT_UNF_HDMI_GetAttr(MT_UNF_HDMI_ID_0, &hdmi_attr);
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_HDMI_GetAttr failed\n");
    }

    MTADP_STR_GetPlayStatus(&play_status);
    play_status.fmt = fmt;
    play_status.hdcp_enable = hdmi_attr.bHDCPEnable;
    MTADP_STR_SetPlayStatus(play_status);

    return s32Ret;
}

static void MTADP_STR_ReleasePlayResource(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    (MT_VOID)MTADP_STR_SavePlayStatus();

    /** Stop AV playback and enter the stop state */
    (MT_VOID)MTADP_STR_StopToPlay(hAvplay, MT_UNF_AVPLAY_STOP_MODE_BLACK);

    MTADP_STR_AvplayDeinit(hAvplay, hWin, hSoundTrack);

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Disp_DeInit();

    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

    (MT_VOID)mt_sys_deinit();

    return;
}

static mt_s32 MTADP_STR_StrConnectCurNim(void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_nim_config_info nim_info;
    mt_nim_type nim_type;

    MTADP_Str_GetNimInfo(&nim_info);
    nim_type = nim_info.cur_nim_use;
    if (MT_NIM_TYPE_DVBC == nim_type)
    {
        mt_nim_dvbc_info dvbc = nim_info.dvbc;

        ret = MTADP_Fe_Connect_Dvbc(dvbc.tuner_id, dvbc.freq, dvbc.sym_rate, dvbc.mod_type);
        if (MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_J83B == nim_type)
    {
        mt_nim_j83b_info j83b = nim_info.j83b;

        ret = MTADP_Fe_Connect_J83b(j83b.tuner_id, j83b.freq, j83b.sym_rate, j83b.mod_type);
        if (MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("MTADP_Fe_Connect_J83b failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_DVBT == nim_type)
    {
        mt_nim_dvbt_info dvbt = nim_info.dvbt;

        ret = MTADP_Fe_Connect_Dvbt(dvbt.tuner_id, dvbt.freq, dvbt.bandwidth);
        if (MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("MTADP_Fe_Connect_Dvbt failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_DVBS_IN == nim_type)
    {
        mt_nim_dvbs_info dvbs_in = nim_info.dvbs_in;

        ret = MTADP_Fe_Connect_Dvbs(dvbs_in.tuner_id, dvbs_in.freq, dvbs_in.sym_rate, dvbs_in.onoff_22k, dvbs_in.polar, dvbs_in.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("MTADP_Fe_Connect_Dvbs_In failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_DVBS_OUT == nim_type)
    {
        mt_nim_dvbs_info dvbs_out = nim_info.dvbs_out;

        ret = MTADP_Fe_Connect_Dvbs(dvbs_out.tuner_id, dvbs_out.freq, dvbs_out.sym_rate, dvbs_out.onoff_22k, dvbs_out.polar, dvbs_out.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("MTADP_Fe_Connect_Dvbs_Out failed.\n");
            return ret;
        }
    }

    return MT_SUCCESS;
}

/*!
@brief audio and video player initialization
@param[out] hWin                the input window handler
@param[out] phAvplay            Handle to AV player
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MTADP_STR_AvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = 0;
    MT_HANDLE                hWin = 0;
    MT_HANDLE                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    MTADP_STR_FUNCTION_ENTER();

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        MTADP_STR_ERR_PRINT("The input address is empty!\n");
        return ret;
    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(ret != MT_SUCCESS)
    {
        MTADP_STR_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, ret = %x\n", ret);
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_Init failed, ret = %x\n", ret);
        return ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_SND_CreateTrack failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_SND_Attach failed, ret = %x\n", ret);
        goto ERROR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", ret);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_VO_AttachWindow failed, ret = %x\n", ret);
        goto ERROR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, ret = %x\n", ret);
        goto ERROR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    MTADP_STR_FUNCTION_EXIT();

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
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MTADP_STR_SetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
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

    MTADP_STR_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        MTADP_STR_ERR_PRINT("The input address is empty\n");
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

    if (u32AudType == HA_AUDIO_ID_PCM)
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

    MTADP_STR_INFO_PRINT("VidPid = %#x, AudPid = %#x\n", VidPid, AudPid);

    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
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
            MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %x\n", ret);
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
            MTADP_STR_ERR_PRINT("Set video properties or video PID property failed, ret = %x\n", ret);
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
            MTADP_STR_ERR_PRINT("Setting the decoding mode or audio PID property failed, ret = %x\n", ret);
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
            MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, ret = %x\n", ret);
            return ret;
        }
    }

    MTADP_STR_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief start the AV playback into the start state
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MTADP_STR_StarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    MTADP_STR_FUNCTION_ENTER();

    /** Set the PID of the AV player and set the encoder type */
    ret = MTADP_STR_SetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if (MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_NimPlaySetAvplayPidAndCodecType fail! \n");
        return ret;
    }

    if (pProgInfo->AElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        MTADP_STR_INFO_PRINT("has no audio info \n");
    }

    if (pProgInfo->VElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        MTADP_STR_INFO_PRINT("has no vide0 info \n");
    }

    if ((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if(MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("Set frame to VO is failed, ret = %x\n", ret);
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("Get avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            MTADP_STR_ERR_PRINT("Set avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if (MT_SUCCESS != ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_AVPLAY_Start failed, ret = %x\n", ret);
        return ret;
    }

    MTADP_STR_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static mt_s32 MTADP_STR_RequestPlayResource(MT_HANDLE *hAvplay, MT_HANDLE *hWin, MT_HANDLE *hSoundTrack)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_str_play_status play_status;
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;

    s32Ret = mt_sys_init();
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("mt_sys_init error. ret=0x%x \n", s32Ret);
        return s32Ret;
    }

    (mt_void)MTADP_STR_GetPlayStatus(&play_status);
    (mt_void)MTADP_HDMI_Set_HdcpEnable(play_status.hdcp_enable);
    s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, play_status.fmt);
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", s32Ret);
        goto ERR0;
    }

    /** Display initialization */
    s32Ret = MTADP_Disp_Init(play_status.fmt);
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", s32Ret);
        goto ERR1;
    }

    s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MTADP_VO_Init failed.\n");
        goto ERR2;
    }

    s32Ret = MTADP_Snd_Init();
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MTADP_Snd_Init failed.\n");
        goto ERR3;
    }

    s32Ret = MTADP_STR_StrConnectCurNim();
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_NimPlayStrConnectCurNim failed.\n");
        goto ERR4;
    }

    /** audio and video player initialization */
    s32Ret = MTADP_STR_AvplayInit(hAvplay, hWin, hSoundTrack);
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_NimPlayAvplayInit failed, ret = %x\n", s32Ret);
        goto ERR4;
    }

    s32Ret = MTADP_Get_Current_Info(&pstCurrentProgInfo);
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MTADP_Get_Current_Info ERR !!\n");
        goto ERR5;
    }

    /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
    s32Ret = MTADP_STR_StarToPlay(*hAvplay, pstCurrentProgInfo);
    if (MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_NimPlayStarToPlay failed, ret = %x\n", s32Ret);
        goto ERR5;
    }

    return s32Ret;

ERR5:
    (MT_VOID)MTADP_STR_AvplayDeinit(*hAvplay, *hWin, *hSoundTrack);
ERR4:
    (MT_VOID)MTADP_Snd_DeInit();
ERR3:
    (MT_VOID)MTADP_VO_DeInit();
ERR2:
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();

    return s32Ret;
}

void MTADP_STR_StartSigStrStandby(void)
{
    mt_s32 s32Ret = 0;
    sty_wakeup_conf_t wconfig = { 0 };
    MT_UNF_HDMI_CEC_CMD_S stCmd = { 0 };

    s32Ret = MT_UNF_PMOC_Init();
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
        return;
    }

    s32Ret = MT_UNF_PMOC_SetCecConfig(g_cec_cfg);
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_PMOC_SetCecConfig failed\n");
    }

    wconfig.w_key.fp_wkey = 0x1;
    s32Ret = MT_UNF_PMOC_SetWakeUpAttr(wconfig);
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr  failed\n");
    }

    /*First close hdmi/disp/sys module, otherwise can not enter str suspend.*/
    MTADP_STR_ReleasePlayResource(g_nimPlayInfo.hAvPlay, g_nimPlayInfo.hWin, g_nimPlayInfo.hSoundTrack);
    /*Check whether the udhcpc process exists, and kill it if it does.*/
    s32Ret = system("pidof udhcpc");
    if(MT_SUCCESS == s32Ret)
    {
        s32Ret = system("kill $(pidof udhcpc)");
        if(MT_SUCCESS != s32Ret)
        {
            MTADP_STR_ERR_PRINT("kill udhcpc failed\n");
        }
    }

    s32Ret = system("killall -SIGUSR1 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("killall -SIGUSR1 audio_ta_service failed\n");
    }

    s32Ret = system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
    }

    mt_msleep(200);
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    s32Ret = system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }
#else
    //symphony4
    s32Ret = system("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }
#endif

    MTADP_STR_INFO_PRINT("enter system suspend \n");
    s32Ret = system("echo mem > /sys/power/state");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("echo mem > /sys/power/state failed\n");
    }

    MTADP_STR_INFO_PRINT("exit system suspend \n");
    s32Ret = system("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin failed\n");
    }

    s32Ret = system("killall -SIGUSR2 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("killall -SIGUSR2 audio_ta_service failed\n");
    }

	//fix bug31220
	s32Ret = system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq");
	s32Ret |= system("echo 1200000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq");
	s32Ret = system("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
    }

    s32Ret = MTADP_STR_RequestPlayResource(&g_nimPlayInfo.hAvPlay, &g_nimPlayInfo.hWin, &g_nimPlayInfo.hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        MTADP_STR_ERR_PRINT("call MT_NimPlayRequestPlayResource failed\n");
    }

    usleep(2000*1000);
    memset(&stCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
    stCmd.u8Opcode = CEC_OPCODE_IMAGE_VIEW_ON;
    s32Ret = MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &stCmd);
    MTADP_HDMI_SetStandbyStatus(MT_FALSE);

    MTADP_STR_INFO_PRINT("%s: system resume success.\n", __FUNCTION__);
}

static mt_s32 MTADP_STR_SetUseStandbyTunerid(mt_u32 id)
{
    mt_s32 i = 0;

    for (i = 0;i < 5; i++)
    {
        if(g_sample_use_Tunerid[i]==0xff)
            break;
    }

    if (i==5)
    {
        return -1;
    }

    g_sample_use_Tunerid[i] = id;

    return 0;
}

void MTADP_STR_FrontEnterStandby(void)
{
    mt_u32 tuner_id = 0;
    mt_unf_fe_attr_t fe_attr;
    mt_u8 cs8800_standby_enable_flag = 0;/*0:no enter standby;1:enter standby*/

    memset(g_sample_use_Tunerid, 0xff, sizeof(g_sample_use_Tunerid));
    for (tuner_id = 0;tuner_id < 5;tuner_id++)
    {
        memset(&fe_attr, 0x00, sizeof(mt_unf_fe_attr_t));
        mt_unf_fe_get_attr(tuner_id,&fe_attr);
        switch (fe_attr.demod_dev_type)
        {
            case MT_UNF_DEMOD_DEV_TYPE_M88CS8800:
                /* The cs8800 may have two tuner id for DVBS and DVBC,
                *  but it only needs to enter the standby once
                */
                if(cs8800_standby_enable_flag == 0)
                {
                    mt_unf_fe_standby(tuner_id);
                    MTADP_STR_SetUseStandbyTunerid(tuner_id);
                    cs8800_standby_enable_flag = 1;
                    MTADP_STR_INFO_PRINT("Tuner_id:%d demod_type:%d enter standby\n",tuner_id,fe_attr.demod_dev_type);
                }
                break;
            case MT_UNF_DEMOD_DEV_TYPE_M88RS6060:
            case MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856:
                mt_unf_fe_standby(tuner_id);
                MTADP_STR_SetUseStandbyTunerid(tuner_id);
                MTADP_STR_INFO_PRINT("Tuner_id:%d demod_type:%d enter standby\n",tuner_id,fe_attr.demod_dev_type);
                break;
            default:
                break;
        }
    }

    return;
}

mt_s32 MTADP_STR_FrontExitStandby(void)
{
    mt_s32 i = 0,ret = 0;

    for (i = 0;i < 5; i++)
    {
        if (g_sample_use_Tunerid[i]!=0xff)
        {
            ret = mt_unf_fe_wakeup(g_sample_use_Tunerid[i]);
            if (MT_SUCCESS != ret)
            {
                MTADP_STR_ERR_PRINT("call mt_unf_fe_wakeup failed\n");
                return ret;
            }
        }
    }

    return ret;
}

mt_void MTADP_STR_SetPlayStatus(mt_str_play_status play_status)
{
    g_strPlayStatus = play_status;
}

mt_void MTADP_STR_GetPlayStatus(mt_str_play_status *play_status)
{
    *play_status = g_strPlayStatus;
}

