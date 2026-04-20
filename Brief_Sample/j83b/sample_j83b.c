/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
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
#include "mt_unf_frontend.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_cmdline.h"
#include "mt_adp_frontend.h"
#include "mt_adp_pvr.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_J83B_DEBUG

#define MT_J83B_PRINT   printf
#else

#define MT_J83B_PRINT

#endif

#define SAMPLE_J83B_FUNCTION_ENTER()    MT_J83B_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_J83B_FUNCTION_EXIT()     MT_J83B_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_J83B_FATAL_PRINT(fmt...)         MT_J83B_PRINT(" [FATAL] " fmt)
#define SAMPLE_J83B_ERR_PRINT(fmt...)           MT_J83B_PRINT(" [ERROR] " fmt)
#define SAMPLE_J83B_WARN_PRINT(fmt...)          MT_J83B_PRINT(" [WARN] "  fmt)
#define SAMPLE_J83B_INFO_PRINT(fmt...)          MT_J83B_PRINT(" [INFO] "  fmt)
#define SAMPLE_J83B_DBG_PRINT(fmt...)           MT_J83B_PRINT(" [DEBUG] " fmt)

#define SAMPLE_J83B_PRINT printf

#define DMX_ID_0            0
#define TUNER_ID_4          4
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef struct
{
    mt_u32 tuner_id;
    mt_u32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    mt_u32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
} mt_input_j83b_para_t;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    mt_input_j83b_para_t para;
} MT_J83B_RUN_INFO;
/********************** Global Variable declaration **************************/
static PMT_COMPACT_TBL *g_pProgTbl = MT_NULL;
static MT_BOOL    g_bTaskQuit = MT_TRUE;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif
static MT_J83B_RUN_INFO    g_stJ83bRunInfo = {MT_INVALID_HANDLE};
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_J83bMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_J83bCheckParam(mt_input_j83b_para_t *p_j83b_in)
{
    if(p_j83b_in->freq < 45 || p_j83b_in->freq > 862)
    {
        SAMPLE_J83B_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_j83b_in->sym_rate < 5057 || p_j83b_in->sym_rate > 7560)
    {
        SAMPLE_J83B_ERR_PRINT("srate error. srate = %d \n", p_j83b_in->sym_rate);
        SAMPLE_J83B_ERR_PRINT("The symbol rate is out of range.\n");
        return MT_FAILURE;
    }

    if(p_j83b_in->mod_type != 256 && p_j83b_in->mod_type != 64)
    {
        SAMPLE_J83B_ERR_PRINT("QAM error. QAM = %d \n", p_j83b_in->mod_type);
        SAMPLE_J83B_ERR_PRINT("QAM must be set 256 or 64.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_J83bDmxInit(MT_VOID)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }
    if(play_resource.demux_use != MT_TRUE)
    {
        /** Initializes the demux module */
        ret = MT_UNF_DMX_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
            return ret;
        }
    }


    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        /** Bind Demux to tuner port 1 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        SAMPLE_J83B_INFO_PRINT("Connect port 1!\n");
    }
    else
    {
        /** Bind Demux to tuner port 0 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
        SAMPLE_J83B_INFO_PRINT("Connect port 0!\n");
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*!
@brief Demux module deinitialization.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_J83bDmxDeinit(MT_VOID)
{
    MT_S32 ret = MT_FAILURE;
    MT_S32 flag = MT_SUCCESS;

    /** Unbind demux from the port */
    ret = MT_UNF_DMX_DetachTSPort(DMX_ID_0);
    if(MT_SUCCESS != ret)
    {

        SAMPLE_J83B_ERR_PRINT("call MT_UNF_DMX_DetachTSPort failed.\n");
        flag = MT_FAILURE;
    }

    /** Deinitializes the demux module */
    ret = MT_UNF_DMX_DeInit();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_DMX_DeInit failed.\n");
        flag = MT_FAILURE;

    }

    return flag;
}

/*!
@brief audio and video player initialization.
@param[out] phAvplay            Handle to AV player
@param[out] hWin                The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_J83bAvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_J83B_FUNCTION_ENTER();

    if(NULL == phAvplay)
    {
        SAMPLE_J83B_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phWin)
    {
        SAMPLE_J83B_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phSoundTrack)
    {
        SAMPLE_J83B_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        return ret;

    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    /** Creates an AVPLAY */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }
    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_J83B_FUNCTION_EXIT();

    return MT_SUCCESS;

ERR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

ERR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

ERR6:
    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);

ERR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);

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



/*!
@brief audio and video player deinitialization.
@param[in]  phAvplay            handle to AV player
@param[in]  hWin                Handle to window
@param[in]  phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_J83bAvplayDeinit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_S32                   flag = MT_SUCCESS;

    SAMPLE_J83B_FUNCTION_ENTER();

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
    }

    /** Unbind the window and AV player */
    ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_VO_DetachWindow failed.\n");
    }

    /** Destroy window */
    ret = MT_UNF_VO_DestroyWindow(hWin);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_VO_DestroyWindow failed.\n");
    }

    /** Contact the binding of track and AV player */
    ret = MT_UNF_SND_Detach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_SND_Detach failed.\n");
    }

    /** Destroy a Track */
    ret = MT_UNF_SND_DestroyTrack(hSoundTrack);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_SND_DestroyTrack failed.\n");
    }

    /** Turn off the video channel */
    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    /** Turn off the audio channel */
    ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    /** Destroy the AV player */
    ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_Destroy failed.\n");
    }

    /** Deinitializes the AV player module */
    ret = MT_UNF_AVPLAY_DeInit();
    if(MT_SUCCESS != ret )
    {
        flag = MT_FAILURE;
        SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_DeInit failed.\n");
    }


    SAMPLE_J83B_FUNCTION_EXIT();

    return flag;
}



/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_J83bSetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
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

    SAMPLE_J83B_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_J83B_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_J83B_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_J83B_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return ret;
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

    SAMPLE_J83B_INFO_PRINT("VidPid=%x, Vidtype=0x%x, AudPid=%x, AudType=0x%x \n", VidPid, enVidType, AudPid, u32AudType);

    /** Get the audio properties of the AV player */

    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
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
            SAMPLE_J83B_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            return ret;
        }

        /* The type of code stream supported by the decoder*/
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
            SAMPLE_J83B_ERR_PRINT("Set video properties or video PID property failed.\n");
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
            SAMPLE_J83B_ERR_PRINT("Setting the decoding mode or audio PID property failed:%#x\n",ret);
            return ret;
        }
    }

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /** Set the audio and video synchronization properties of AV player */
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            return ret;
        }
    }

    SAMPLE_J83B_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_J83bStarToPlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_J83B_FUNCTION_ENTER();

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_J83bSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("MT_J83bSetAvplayPidAndCodecType fail! \n");
        return ret;
    }
    /** Get the audio PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_J83B_INFO_PRINT("Has no audio stream!\n");
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_J83B_INFO_PRINT("Has no video stream!\n");
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
            SAMPLE_J83B_ERR_PRINT("Set frame to VO fail.\n");
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("Get avplay sync attr fail!\n");
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
            SAMPLE_J83B_ERR_PRINT("Set avplay sync attr fail!\n");
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  =MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("MT_UNF_AVPLAY_Start fail!  ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_J83B_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state.
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_J83bStopToPlay(mt_handle hAvplay,MT_UNF_AVPLAY_STOP_MODE_E enmode)
{
    MT_U32 ret = MT_FAILURE;

    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    SAMPLE_J83B_FUNCTION_ENTER();

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = enmode;
    option.u32TimeoutMs = 0;
    SAMPLE_J83B_INFO_PRINT("stop live play ...\n");
    ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("MT_UNF_AVPLAY_Stop failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_J83B_FUNCTION_EXIT();

    return MT_SUCCESS;
}


static MT_VOID MT_J83bExit(MT_VOID)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    SAMPLE_J83B_FUNCTION_ENTER();
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    (MT_VOID)MT_J83bStopToPlay(g_stJ83bRunInfo.hAvPlay,1);

    (MT_VOID)MT_J83bAvplayDeinit(g_stJ83bRunInfo.hAvPlay, g_stJ83bRunInfo.hWin, g_stJ83bRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();
    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)(MT_VOID)MT_J83bDmxDeinit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_J83B_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_J83B_INFO_PRINT("PVR is recording now \n");
        SAMPLE_J83B_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();
    if(play_resource.rec_status != MT_TRUE)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_4);
    }

    memset(&g_stJ83bRunInfo, 0xff, sizeof(g_stJ83bRunInfo));
    g_bTaskQuit = MT_TRUE;

    MTADP_Get_VcodeUnblank(&unblank);
    if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
    {
        MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
    }

    SAMPLE_J83B_FUNCTION_ENTER();
}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static MT_VOID MT_J83bPrint_help(MT_CHAR *name)
{
    SAMPLE_J83B_PRINT("Lack of parameters\n");
    SAMPLE_J83B_PRINT("\nUsage:\n");
    SAMPLE_J83B_PRINT("%s\n", name);
    SAMPLE_J83B_PRINT("    -f: set freq(50~900)\n");
    SAMPLE_J83B_PRINT("    -s: set symbol rate default:5361\n");
    SAMPLE_J83B_PRINT("    -p: set qam default:256\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_J83B_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_J83B_PRINT("example:\n");
    SAMPLE_J83B_PRINT("    %s -f 474 -s 5361 -p 256\n",name);
}

/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@param[out] pInutParam      Analyze input parameter values
@return::void
@*/
static mt_s32 MT_J83bParase_args(int argc, char *argv[], mt_input_j83b_para_t *pOutParam)
{
    int opt = 0;

    SAMPLE_J83B_FUNCTION_ENTER();

    if(argc != 7 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_J83bPrint_help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:p:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_J83bPrint_help(argv[0]);
                return MT_FAILURE;
            case 'f':
                pOutParam->freq = strtol(mt_optarg, 0, 0);
                break;
            case 's':
                pOutParam->sym_rate = strtol(mt_optarg, 0, 0);
                break;
            case 'p':
                pOutParam->mod_type = strtol(mt_optarg, 0, 0);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_J83bExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_J83bPrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    SAMPLE_J83B_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static void MT_J83bPrintMenu(MT_U32 prog_num)
{
    SAMPLE_J83B_PRINT("\n 1 - %d : select the program \n", prog_num);
    SAMPLE_J83B_PRINT("     h : help \n");
    SAMPLE_J83B_PRINT("     p : pause \n");
    SAMPLE_J83B_PRINT("     r : resume \n");
    SAMPLE_J83B_PRINT("     z : Channel Switch Mode \n");
    SAMPLE_J83B_PRINT("     s : signal strength \n");
    SAMPLE_J83B_PRINT("     l : signal quality \n");
    SAMPLE_J83B_PRINT("     d : check if audio dolby mono \n");
    SAMPLE_J83B_PRINT("     k : set unblank screen mode\n");
    SAMPLE_J83B_PRINT("     g : get first video frame show and avsync done cost time \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_J83B_PRINT("     b : background run \n");
#endif
    SAMPLE_J83B_PRINT("     q : quit \n");
    SAMPLE_J83B_PRINT("J83B>> ");
}

/*!
@brief Task action commands
@param[in]  hAvPlay     handle to AV player
@return::MT_VOID
@*/
static MT_VOID MT_J83bCmdTask(MT_HANDLE hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    mt_u32             strength = 0;
    mt_s32             agc = 0;
    mt_u32             quality = 0;
    mt_s32             accurate_snr = 0;
    mt_u32             snr = 0;
    MT_S32             ret = MT_FAILURE;
    MT_U32             u32ProgNum = 1;
#ifdef MT_SAMPLE_APP
    play_resource.s32ProgNum = u32ProgNum;
#endif
    MT_CHAR            inputCmd[32] = { 0 };
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;
    MT_UNF_AVPLAY_STOP_MODE_E enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    mt_s32 unblank = MT_UNF_VCODEC_UNBLANK_STABLE;
    mt_s64 first_vid_frm_show_time, avsync_done_time;

    while(1)
    {
        MT_J83bPrintMenu(pProgTbl->prog_num);

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if(inputCmd[0] == 'q')
        {
            SAMPLE_J83B_INFO_PRINT("exit!\n");
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
            SAMPLE_J83B_INFO_PRINT("J83b play in back!\n");
            break;
        }
    #endif
        else if('p' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Pause(hAvplay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_J83B_INFO_PRINT("MT_UNF_AVPLAY_Pause failed\n");
            }
        }
        else if('r' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Resume(hAvplay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_J83B_INFO_PRINT("MT_UNF_AVPLAY_Resume failed\n");
            }
        }
        else if('z' == inputCmd[0])
        {
            if(enmode == MT_UNF_AVPLAY_STOP_MODE_BLACK)
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_STILL;
                SAMPLE_J83B_INFO_PRINT("Set mode to Freeze \n");
            }
            else
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
                SAMPLE_J83B_INFO_PRINT("Set mode to black \n");
            }

        }
        else if('s' == inputCmd[0])
        {
            memset(&strength, 0, sizeof(strength));
            memset(&agc, 0, sizeof(agc));
            ret = mt_unf_fe_get_signal_strength(TUNER_ID_4, &strength);
            ret = mt_unf_fe_get_agc(TUNER_ID_4, 0, &agc);

            SAMPLE_J83B_PRINT("\t Signal strength = %d, agc = %d\n", strength, agc);
        }
        else if('l' == inputCmd[0])
        {
            memset(&snr, 0, sizeof(snr));
            memset(&quality, 0, sizeof(quality));
            memset(&accurate_snr, 0, sizeof(accurate_snr));
            ret = mt_unf_fe_get_signal_quality(TUNER_ID_4, &quality);
            ret = mt_unf_fe_get_snr(TUNER_ID_4, &snr);
            ret = mt_unf_fe_get_accurate_snr(TUNER_ID_4, &accurate_snr);

            SAMPLE_J83B_PRINT("\t Signal quality = %d, snr = %d accurate_snr:%02d.%03d\n",
                quality, snr, accurate_snr/1000, accurate_snr%1000);
        }
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);

            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                /** Stop AV playback into the stop state */
                ret = MT_J83bStopToPlay(hAvplay,enmode);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_J83B_ERR_PRINT(" MT_J83bStopToPlay failed.\n");
                }
                SAMPLE_J83B_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);
                // restore ac4    attr info.
                MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);

                /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
                ret = MT_J83bStarToPlay(hAvplay, pstCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_J83B_ERR_PRINT("Switching shows failed\n");
                }
                play_resource.s32ProgNum = u32ProgNum;
            }
            else
            {
               SAMPLE_J83B_ERR_PRINT("prog_num the biggest is %d\n", pProgTbl->prog_num);
               continue;
            }
#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(pstCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_J83B_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
        else if('d' == inputCmd[0])
        {
            MT_UNF_AUDIOTRACK_ATTR_S trackAttr;
            memset(&trackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
            ret = MT_UNF_SND_GetTrackAttr(g_stJ83bRunInfo.hSoundTrack, &trackAttr);
            if (MT_SUCCESS == ret && MT_FALSE != trackAttr.dolby_dd_ddp && MT_TRUE == trackAttr.dolby_dualmono)
            {
                SAMPLE_J83B_INFO_PRINT("Audio dolby info: dolby[%d] Dual-Mono [1+1].\n", trackAttr.dolby_dd_ddp);
            }
        }
        else if ('k' == inputCmd[0])
        {
            SAMPLE_J83B_PRINT("input unblank mode(0:fast 1:stable 2:sync):");
            scanf("%d", &unblank);
            getchar();
            SAMPLE_J83B_PRINT("unblank: %d \n", unblank);
            unblank = unblank % MT_UNF_VCODEC_UNBLANK_BUTT;
            MTADP_Set_VcodeUnblank(unblank);
        }
        else if('g' == inputCmd[0])
        {
            (MT_VOID)MTADP_ReadPlayStat(&first_vid_frm_show_time, &avsync_done_time);
            SAMPLE_J83B_PRINT("[time] first video frame showed cost time: %lldms \n", first_vid_frm_show_time);
            SAMPLE_J83B_PRINT("[time] avsync done cost time: %lldms \n", avsync_done_time);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_J83B_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_J83bMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    MT_S32                  ret = 0;
    PMT_COMPACT_PROG        *pstCurrentProgInfo= { 0 };

    ret = MT_J83bParase_args(argc, argv, &g_stJ83bRunInfo.para);
    if (MT_FAILURE == ret)
    {
        SAMPLE_J83B_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_J83B_INFO_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    SAMPLE_J83B_INFO_PRINT("J83b: %d %d %d \n", g_stJ83bRunInfo.para.freq, g_stJ83bRunInfo.para.sym_rate, g_stJ83bRunInfo.para.mod_type);

    if(g_bTaskQuit == MT_TRUE)
    {
        ret = MT_J83bCheckParam(&g_stJ83bRunInfo.para);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("MT_SampleCheckJ83bParam failed.\n");
            (MT_VOID)MT_J83bPrint_help(argv[0]);
            return ret;
        }

    #ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("MT_SYS_Init failed.\n");
            return ret;
        }

        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {

           SAMPLE_J83B_ERR_PRINT("MTADP_HDMI_Init failed.\n");
           goto ERR1;

        }
        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {

           SAMPLE_J83B_ERR_PRINT("MTADP_Disp_Init failed.\n");
           goto ERR2;
        }
    #endif


        ret = MTADP_Fe_Init(TUNER_ID_4);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("MTADP_Fe_Init failed.\n");
            goto ERR3;
        }

        ret = MTADP_Fe_Connect_J83b(TUNER_ID_4, g_stJ83bRunInfo.para.freq, g_stJ83bRunInfo.para.sym_rate, g_stJ83bRunInfo.para.mod_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_J83B_ERR_PRINT("MTADP_Fe_Connect_J83b error\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {

           SAMPLE_J83B_ERR_PRINT("MTADP_VO_Init failed.\n");
           goto ERR4;
        }
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
           SAMPLE_J83B_ERR_PRINT("MTADP_Snd_Init failed.\n");
           goto ERR5;

        }
        ret = MT_J83bDmxInit();
        if(MT_SUCCESS != ret)
        {

           SAMPLE_J83B_ERR_PRINT("MT_J83bDmxInitAndSearch failed.\n");
           goto ERR6;

        }
        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_pProgTbl);
        if(MT_SUCCESS != ret)
        {

           SAMPLE_J83B_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
           goto ERR8;

        }

        ret = MT_J83bAvplayInit(&g_stJ83bRunInfo.hAvPlay, &g_stJ83bRunInfo.hWin, &g_stJ83bRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {

           SAMPLE_J83B_ERR_PRINT("MT_J83bAvplayInit failed.\n");
           goto ERR9;

        }
#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_stJ83bRunInfo.hAvPlay;
        avplayHandle.hSoundTrack = g_stJ83bRunInfo.hSoundTrack;
        avplayHandle.hWin = g_stJ83bRunInfo.hWin;

#endif
        SAMPLE_J83B_INFO_PRINT("please input the number of program to play:\n");

        /* Play the first program on the program list*/
        pstCurrentProgInfo = g_pProgTbl->proginfo;
        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        ret = MT_J83bStarToPlay(g_stJ83bRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_J83B_ERR_PRINT("MT_J83bStarToPlay failed.\n");
           goto ERR10;
        }
        g_bTaskQuit = MT_FALSE;
        play_resource.sig_type = 0;
    }

    (MT_VOID)MT_J83bCmdTask(g_stJ83bRunInfo.hAvPlay, g_pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    /** Stop AV playback and enter the stop state */
    ret = MT_J83bStopToPlay(g_stJ83bRunInfo.hAvPlay,1);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_J83B_ERR_PRINT("MT_J83bStopToPlay failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
    }


ERR10:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_J83bAvplayDeinit(g_stJ83bRunInfo.hAvPlay, g_stJ83bRunInfo.hWin, g_stJ83bRunInfo.hSoundTrack);
ERR9:
    /** Release the PMT table */
    (MT_VOID)MTADP_Search_FreeAllPmt(g_pProgTbl);
ERR8:
    (MT_VOID)MTADP_Search_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)MT_J83bDmxDeinit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_J83B_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_J83B_INFO_PRINT("PVR is recording now \n");
        SAMPLE_J83B_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }
    /** Demux module deinitialization */

ERR6:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();
ERR5:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    if(play_resource.rec_status != MT_TRUE)
    {
        /** Disconnect the tuner lock */
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_4);
    }

ERR3:
#ifndef MT_SAMPLE_APP
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR2:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_stJ83bRunInfo, 0xff, sizeof(g_stJ83bRunInfo));

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}



