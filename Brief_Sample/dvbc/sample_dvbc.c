/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_cmdline.h"
#include "mt_adp_pvr.h"

/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_DVBC_DEBUG
#define MT_DVBC_PRINT   printf
#else
#define MT_DVBC_PRINT
#endif

#define SAMPLE_DVBC_FUNCTION_ENTER()            MT_DVBC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DVBC_FUNCTION_EXIT()             MT_DVBC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_DVBC_FATAL_PRINT(fmt...)         MT_DVBC_PRINT(" [FATAL] " fmt)
#define SAMPLE_DVBC_ERR_PRINT(fmt...)           MT_DVBC_PRINT(" [ERROR] " fmt)
#define SAMPLE_DVBC_WARN_PRINT(fmt...)          MT_DVBC_PRINT(" [WARN] "  fmt)
#define SAMPLE_DVBC_INFO_PRINT(fmt...)          MT_DVBC_PRINT(" [INFO] "  fmt)
#define SAMPLE_DVBC_DBG_PRINT(fmt...)           MT_DVBC_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DVBC_PRINT  printf
#define DMX_ID_0           0
#define TUNER_ID_4         4


#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2
/*************************** Structure Definition ****************************/

typedef struct
{
    mt_u32 freq; /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 mod_type; /**<QAM mode*/
} mt_input_cab_para_t;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    mt_input_cab_para_t sInputParam;
    PMT_COMPACT_TBL *pProgTbl;
} MT_DVBC_RUN_INFO;

/********************** Global Variable declaration **************************/

static MT_BOOL    g_bTaskQuit = MT_TRUE;
static MT_DVBC_RUN_INFO    g_stDvbcRunInfo;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbcMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif


/******************************* API declaration *****************************/
static MT_S32 MT_DvbcCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    SAMPLE_DVBC_FUNCTION_ENTER();

    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_DVBC_ERR_PRINT("The frequency[%d] is not in range\n", p_cab_in->freq);
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_DVBC_ERR_PRINT("The symbol rate [%d] is not in range\n", p_cab_in->sym_rate);
        return MT_FAILURE;
    }

    switch(p_cab_in->mod_type)
    {
        case 16:
            break;
        case 32:
            break;
        case 64:
            break;
        case 128:
            break;
        case 256:
            break;
        default:
            SAMPLE_DVBC_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    SAMPLE_DVBC_FUNCTION_EXIT();
    return MT_SUCCESS;
}


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbcDmxInit(MT_VOID)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    SAMPLE_DVBC_FUNCTION_ENTER();

    /** Initializes the demux module */
    if(play_resource.demux_use != MT_TRUE)
    {
        ret = MT_UNF_DMX_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MT_UNF_DMX_Init failed, ret = %x\n", ret);
            return ret;
        }
    }


    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, ret = %x\n", ret);
        MT_UNF_DMX_DeInit();
        if(ret != MT_SUCCESS)
        {
            SAMPLE_DVBC_ERR_PRINT("MT_UNF_DMX_DeInit failed, ret = %x\n", ret);
            return ret;
        }
        return ret;
    }

    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        /** Bind Demux to tuner port 1 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
    }
    else
    {
        /** Bind Demux to tuner port 0 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, ret = %x\n", ret);
        MT_UNF_DMX_DeInit();
        return ret;
    }

    SAMPLE_DVBC_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_VOID MT_DvbcDmxDeinit(MT_VOID)
{
    SAMPLE_DVBC_FUNCTION_ENTER();

    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();

    SAMPLE_DVBC_FUNCTION_EXIT();
}


/*!
@brief audio and video player initialization
@param[out] hWin                the input window handler
@param[out] phAvplay            Handle to AV player
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbcAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = 0;
    MT_HANDLE                hWin = 0;
    MT_HANDLE                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_DVBC_FUNCTION_ENTER();

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_DVBC_ERR_PRINT("The input address is empty!\n");
        return ret;
    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(ret != MT_SUCCESS)
    {
        SAMPLE_DVBC_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, ret = %x\n", ret);
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_Init failed, ret = %x\n", ret);
        return ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    /** Create AV player based on attributes */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_SND_CreateTrack failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_SND_Attach failed, ret = %x\n", ret);
        goto ERROR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", ret);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_VO_AttachWindow failed, ret = %x\n", ret);
        goto ERROR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, ret = %x\n", ret);
        goto ERROR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_DVBC_FUNCTION_EXIT();

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
@param[in]  phAvplay            handle to AV player
@param[in]  hWin                Handle to window
@param[in]  phSoundTrack        Handle to sound track
@return::MT_VOID
@*/
static MT_VOID MT_DvbcAvplayDeinit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    SAMPLE_DVBC_FUNCTION_ENTER();

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

    SAMPLE_DVBC_FUNCTION_EXIT();
}



/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_DvbcSetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
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

    SAMPLE_DVBC_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_DVBC_ERR_PRINT("The input address is empty\n");
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

    SAMPLE_DVBC_INFO_PRINT("VidPid = %#x, AudPid = %#x\n", VidPid, AudPid);

    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
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
            SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %x\n", ret);
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
            SAMPLE_DVBC_ERR_PRINT("Set video properties or video PID property failed, ret = %x\n", ret);
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
            SAMPLE_DVBC_ERR_PRINT("Setting the decoding mode or audio PID property failed, ret = %x\n", ret);
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
            SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, ret = %x\n", ret);
            return ret;
        }
    }

    SAMPLE_DVBC_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_DvbcStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_DVBC_FUNCTION_ENTER();

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_DvbcSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_DvbcSetAvplayPidAndCodecType fail! \n");
        return ret;
    }

    if(pProgInfo->AElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_DVBC_INFO_PRINT("has no audio info \n");
    }

    if(pProgInfo->VElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_DVBC_INFO_PRINT("has no vide0 info \n");
    }

    if((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("Set frame to VO is failed, ret = %x\n", ret);
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("Get avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.stSyncStartRegion.bSmoothPlay = MT_TRUE;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("Set avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  =MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_Start failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_DVBC_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbcStopToPlay(MT_HANDLE hAvplay, MT_UNF_AVPLAY_STOP_MODE_E enmode)
{
    MT_U32 ret = MT_FAILURE;

    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    SAMPLE_DVBC_FUNCTION_ENTER();

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = enmode;
    option.u32TimeoutMs = 0;
    MT_DVBC_PRINT("stop live play ...\n");

    ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_Stop failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_DVBC_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static MT_VOID MT_DvbcExit(MT_VOID)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    SAMPLE_DVBC_FUNCTION_ENTER();
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    (MT_VOID)MT_DvbcStopToPlay(g_stDvbcRunInfo.hAvPlay,1);

    (MT_VOID)MT_DvbcAvplayDeinit(g_stDvbcRunInfo.hAvPlay, g_stDvbcRunInfo.hWin, g_stDvbcRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbcRunInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)MT_DvbcDmxDeinit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_DVBC_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_DVBC_INFO_PRINT("PVR is recording now \n");
        SAMPLE_DVBC_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();
    if(play_resource.rec_status != MT_TRUE)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_4);
    }

    memset(&g_stDvbcRunInfo, 0xff, sizeof(g_stDvbcRunInfo));
    g_bTaskQuit = MT_TRUE;

    MTADP_Get_VcodeUnblank(&unblank);
    if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
    {
        MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
    }

    SAMPLE_DVBC_FUNCTION_EXIT();
}

/*!
@brief Help information
@param[in]  name     Enter the value
@return::MT_VOID
@*/
static MT_VOID MT_DvbcPrint_Help(MT_CHAR *name)
{
    SAMPLE_DVBC_PRINT("Lack of parameters\n");
    SAMPLE_DVBC_PRINT("\nUsage:\n");
    SAMPLE_DVBC_PRINT("%s\n", name);
    SAMPLE_DVBC_PRINT("    -f: set freq\n");
    SAMPLE_DVBC_PRINT("    -s: set symbol rate default:6875\n");
    SAMPLE_DVBC_PRINT("    -p: set qam default:64\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_DVBC_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_DVBC_PRINT("example:\n");
    SAMPLE_DVBC_PRINT("    %s -f 314 -s 6875 -p 64\n",name);
}


static MT_VOID MT_DvbcPrintMenu(MT_U32 prog_num)
{

    SAMPLE_DVBC_PRINT("\n 1 - %d : select the program \n", prog_num);

    SAMPLE_DVBC_PRINT("     h : help \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_DVBC_PRINT("     b : background run \n");
#endif
    SAMPLE_DVBC_PRINT("     p : pause \n");
    SAMPLE_DVBC_PRINT("     r : resume \n");
    SAMPLE_DVBC_PRINT("     z : Channel Switch Mode \n");
    SAMPLE_DVBC_PRINT("     s : signal strength \n");
    SAMPLE_DVBC_PRINT("     l : signal quality \n");
	SAMPLE_DVBC_PRINT("     i : get signal information \n");
    SAMPLE_DVBC_PRINT("     d : check if audio dolby mono \n");
    SAMPLE_DVBC_PRINT("     k : set unblank screen mode \n");
    SAMPLE_DVBC_PRINT("     g : get first video frame show and avsync done cost time \n");
    SAMPLE_DVBC_PRINT("     q : quit \n");
    SAMPLE_DVBC_PRINT("DVBC>> ");

}




/*!
@brief Task action commands
@param[in]  hAvPlay     handle to AV player
@return::MT_VOID
@*/
static MT_VOID MT_DvbcCmdTask(MT_HANDLE hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_S32             ret = MT_FAILURE;
    MT_U32             u32ProgNum = 1;
#ifdef MT_SAMPLE_APP
    play_resource.s32ProgNum = u32ProgNum;
#endif
    MT_CHAR            inputCmd[32] = { 0 };
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;
    MT_UNF_AVPLAY_STOP_MODE_E enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    mt_s32 accurate_snr = 0;
    mt_u32 strength = 0;
    mt_s32 agc = 0;
    mt_u32 quality = 0;
    mt_u32 snr = 0;
    mt_s32 unblank = MT_UNF_VCODEC_UNBLANK_STABLE;
    mt_s64 first_vid_frm_show_time, avsync_done_time;


    if(MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_DVBC_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    while(1)
    {
        (MT_VOID)MT_DvbcPrintMenu(pProgTbl->prog_num);

        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_DVBC_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
            {
                MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
            }
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_DVBC_INFO_PRINT("Dvbc play in back!\n");
            break;
        }

#endif
        else if('p' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Pause(hAvPlay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_Pause failed\n");
            }
        }
        else if('r' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Resume(hAvPlay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBC_ERR_PRINT("MT_UNF_AVPLAY_Resume failed\n");
            }
        }
        else if('z' == inputCmd[0])
        {
            if(enmode == MT_UNF_AVPLAY_STOP_MODE_BLACK)
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_STILL;
                SAMPLE_DVBC_INFO_PRINT("Set mode to Freeze \n");
            }
            else
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
                SAMPLE_DVBC_INFO_PRINT("Set mode to black \n");
            }

        }
        else if('s' == inputCmd[0])
        {
            memset(&strength, 0, sizeof(strength));
            memset(&agc, 0, sizeof(agc));
            ret = mt_unf_fe_get_signal_strength(TUNER_ID_4, &strength);
            ret = mt_unf_fe_get_agc(TUNER_ID_4, 0, &agc);

            SAMPLE_DVBC_PRINT("\t Signal strength = %d, agc = %d\n", strength, agc);
        }
        else if('l' == inputCmd[0])
        {
            memset(&snr, 0, sizeof(snr));
            memset(&quality, 0, sizeof(quality));
            memset(&accurate_snr, 0, sizeof(accurate_snr));
            ret = mt_unf_fe_get_signal_quality(TUNER_ID_4, &quality);
            ret = mt_unf_fe_get_snr(TUNER_ID_4, &snr);
            ret = mt_unf_fe_get_accurate_snr(TUNER_ID_4, &accurate_snr);

            SAMPLE_DVBC_PRINT("\t Signal quality = %d, snr = %d accurate_snr:%02d.%03d\n",
                quality, snr, accurate_snr/1000, accurate_snr%1000);
        }
		else if('i' == inputCmd[0])
		{
			ret = MTADP_Fe_Get_Signal_Info(TUNER_ID_4);
			if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBC_ERR_PRINT("MTADP_Fe_Get_Signal_Info failed\n");
            }
		}
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);

            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum - 1) % pProgTbl->prog_num);

                /** Stop AV playback into the stop state */
                ret = MT_DvbcStopToPlay(hAvPlay,enmode);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DVBC_ERR_PRINT("MT_DvbcStopToPlay failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
                }
                
                SAMPLE_DVBC_INFO_PRINT("===== Start play ProgNum: %d \n", u32ProgNum);
                // restore ac4    attr info.
                MTADP_AUD_RestoreAc4PlayAttrInfo(hAvPlay);

                /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
                ret = MT_DvbcStarToPlay(hAvPlay, pstCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DVBC_ERR_PRINT("Switching shows failed\n");
                }
                play_resource.s32ProgNum = u32ProgNum;
            }
            else
            {
                SAMPLE_DVBC_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }

#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(pstCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBC_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
        else if('d' == inputCmd[0])
        {
            MT_UNF_AUDIOTRACK_ATTR_S trackAttr;
            memset(&trackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
            ret = MT_UNF_SND_GetTrackAttr(g_stDvbcRunInfo.hSoundTrack, &trackAttr);
            if (MT_SUCCESS == ret && MT_FALSE != trackAttr.dolby_dd_ddp && MT_TRUE == trackAttr.dolby_dualmono)
            {
                SAMPLE_DVBC_INFO_PRINT("Audio dolby info: dolby[%d] Dual-Mono [1+1].\n", trackAttr.dolby_dd_ddp);
            }
        }
        else if ('k' == inputCmd[0])
        {
            SAMPLE_DVBC_PRINT("input unblank mode(0:fast 1:stable 2:sync):");
            scanf("%d", &unblank);
            getchar();
            SAMPLE_DVBC_PRINT("unblank: %d \n", unblank);
            unblank = unblank % MT_UNF_VCODEC_UNBLANK_BUTT;
            MTADP_Set_VcodeUnblank(unblank);
        }
        else if('g' == inputCmd[0])
        {
            (MT_VOID)MTADP_ReadPlayStat(&first_vid_frm_show_time, &avsync_done_time);
            SAMPLE_DVBC_PRINT("[time] first video frame showed cost time: %lldms \n", first_vid_frm_show_time);
            SAMPLE_DVBC_PRINT("[time] avsync done cost time: %lldms \n", avsync_done_time);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DVBC_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}
/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::MT_VOID
@*/
static MT_S32 MT_DvbcParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_cab_para_t *pInutParam)
{
    MT_S32 opt = 0;
    /** example: ./sample_dvbc -f 314 -s 6875 -p 64 */

    SAMPLE_DVBC_FUNCTION_ENTER();

    if(argc != 7 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DvbcPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:p:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_DvbcPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DvbcExit();
                }
                return MT_TASK_EXIT;

            case 'f':
                pInutParam->freq = strtol(mt_optarg, 0, 0);
                break;

            case 's':
                pInutParam->sym_rate = strtol(mt_optarg, 0, 0);
                break;

            case 'p':
                pInutParam->mod_type = strtol(mt_optarg, 0, 0);
                break;

            default:
                (MT_VOID)MT_DvbcPrint_Help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    SAMPLE_DVBC_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbcMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S8              count = 0;
    MT_S32             ret = MT_FAILURE;
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;


    SAMPLE_DVBC_FUNCTION_ENTER();

    /** Get the parameters */
    ret = MT_DvbcParase_args(argc, argv, &g_stDvbcRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DVBC_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DVBC_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    SAMPLE_DVBC_INFO_PRINT("DVBC: %d %d %d \n", g_stDvbcRunInfo.sInputParam.freq, g_stDvbcRunInfo.sInputParam.sym_rate, g_stDvbcRunInfo.sInputParam.mod_type);

    if(g_bTaskQuit == MT_TRUE)
    {
        ret = MT_DvbcCheckDvbcParam(&g_stDvbcRunInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MT_DvbcModeCheckDvbcParam failed.\n");
            return MT_FAILURE;
        }
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return MT_FAILURE;
        }

        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", ret);
            goto ERR0;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", ret);
            goto ERR1;
        }
#endif

        /** Tuner initialization, Set the default parameters for tuner */
        ret = MTADP_Fe_Init(TUNER_ID_4);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MTADP_Fe_Init failed, ret = %x\n", ret);
            goto ERR2;
        }


        /** Tuner DVBC locked, param[in] [1]Tuner ID [2]Tuner frequency [3]Tuner symbol rate [4]QAM modulation format */
        ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_4, g_stDvbcRunInfo.sInputParam.freq, g_stDvbcRunInfo.sInputParam.sym_rate, g_stDvbcRunInfo.sInputParam.mod_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MTADP_Fe_Connect failed\n");
            goto ERR2;
        }

        /** VO device initialization */
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MTADP_VO_Init failed, ret = %x\n", ret);
            goto ERR3;
        }

        /** Audio device initialization */
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MTADP_Snd_Init failed, ret = %x\n", ret);
            goto ERR5;
        }

        /** Demux initializes and retrieves the PMT and PAT tables in TS */
        ret = MT_DvbcDmxInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MT_DvbcDmxInit failed, ret = %x\n", ret);
            goto ERR6;
        }

        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        while(MT_SUCCESS != MTADP_Search_GetAllPmt(DMX_ID_0, &g_stDvbcRunInfo.pProgTbl))
        {
            count++;
            SAMPLE_DVBC_ERR_PRINT("MTADP_Search_GetAllPmt failed, ret = %x\n", ret);
            MT_USLEEP(100000);
            if(20 == count)
            {
                goto ERR8;
            }
        }

        /** audio and video player initialization */
        ret = MT_DvbcAvplayInit(&g_stDvbcRunInfo.hAvPlay, &g_stDvbcRunInfo.hWin, &g_stDvbcRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MT_DvbcAvplayInit failed, ret = %x\n", ret);
            goto ERR9;
        }

#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_stDvbcRunInfo.hAvPlay;
        avplayHandle.hSoundTrack = g_stDvbcRunInfo.hSoundTrack;
        avplayHandle.hWin = g_stDvbcRunInfo.hWin;
#endif
        pstCurrentProgInfo = g_stDvbcRunInfo.pProgTbl->proginfo;
        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        ret = MT_DvbcStarToPlay(g_stDvbcRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBC_ERR_PRINT("MT_DvbcStarToPlay failed, ret = %x\n", ret);
            goto ERR10;
        }

        g_bTaskQuit = MT_FALSE;
        play_resource.sig_type = 0;
    }

    (MT_VOID)MT_DvbcCmdTask(g_stDvbcRunInfo.hAvPlay, g_stDvbcRunInfo.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif

    /** Stop AV playback and enter the stop state */
    ret = MT_DvbcStopToPlay(g_stDvbcRunInfo.hAvPlay, MT_UNF_AVPLAY_STOP_MODE_BLACK);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBC_ERR_PRINT("MT_DvbcStopToPlay failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
    }

ERR10:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_DvbcAvplayDeinit(g_stDvbcRunInfo.hAvPlay, g_stDvbcRunInfo.hWin, g_stDvbcRunInfo.hSoundTrack);
ERR9:
    /** Release the PMT table */
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbcRunInfo.pProgTbl);
ERR8:
    (MT_VOID)MTADP_Search_DeInit();
    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)MT_DvbcDmxDeinit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_DVBC_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_DVBC_INFO_PRINT("PVR is recording now \n");
        SAMPLE_DVBC_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }
ERR6:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();
ERR5:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR3:
    if(play_resource.rec_status != MT_TRUE)
    {
        /** Disconnect the tuner lock */
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_4);
    }
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif

    g_bTaskQuit = MT_TRUE;
    memset(&g_stDvbcRunInfo, 0xff, sizeof(g_stDvbcRunInfo));

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}

