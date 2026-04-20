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
#include "mt_adp_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DISEQC_DEBUG

#define MT_DISEQC_PRINT   printf
#else

#define MT_DISEQC_PRINT

#endif

#define SAMPLE_DISEQC_FUNCTION_ENTER()      MT_DISEQC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DISEQC_FUNCTION_EXIT()       MT_DISEQC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DISEQC_FATAL_PRINT(fmt...)       MT_DISEQC_PRINT(" [FATAL] " fmt)
#define SAMPLE_DISEQC_ERR_PRINT(fmt...)         MT_DISEQC_PRINT(" [ERROR] " fmt)
#define SAMPLE_DISEQC_WARN_PRINT(fmt...)        MT_DISEQC_PRINT(" [WARN] "  fmt)
#define SAMPLE_DISEQC_INFO_PRINT(fmt...)        MT_DISEQC_PRINT(" [INFO] "  fmt)
#define SAMPLE_DISEQC_DBG_PRINT(fmt...)         MT_DISEQC_PRINT(" [DEBUG] " fmt)


#define SAMPLE_DISEQC_PRINT  printf
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2


#define DMX_ID_0            0
#define MAX_TP_NUM           64

/*************************** Structure Definition ****************************/

typedef struct
{
    mt_u32 freq;
    mt_u32 symbol_rate;
    mt_u32 program_num;
    mt_u8 dvb_type;
    mt_u32 polar;
    PMT_COMPACT_PROG proginfo[64];
} mt_tpinfo_para_t;


typedef struct
{
    mt_u32 tuner_id;
    mt_u32 startfreq; /**<start Frequency, in kHz*/
    mt_u32 endfreq; /**<endfreq Frequency, in kHz*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar; /**<Polarization mode>*/
    mt_u32 bs_mode; /**<blind mode>*/
    mt_s32 low_lo;
    mt_s32 high_lo;
} mt_input_Blindscan_para_t;

typedef enum
{
    MT_BLINDSCAN_STATUS_INIT,
    MT_BLINDSCAN_STATUS_SCANNING,
    MT_BLINDSCAN_STATUS_FINISH,

} mt_blindscan_status_t;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
	mt_u32			   tpIndex;
} MT_Diseqc_RUN_INFO;


typedef struct
{
    mt_s32 total_num;
    mt_tpinfo_para_t tppara[MAX_TP_NUM];
    mt_unf_fe_sat_tpinfo_t satTPinfo[MAX_TP_NUM];
    mt_s32 s32TPNum;
    mt_blindscan_status_t bs_status;
    mt_input_Blindscan_para_t  sInputParam;
    MT_Diseqc_RUN_INFO stDiseqcRunInfo;
}mt_TPinfo_para_t;


/********************** Global Variable declaration **************************/
static mt_TPinfo_para_t g_sTPinfo;
static MT_BOOL    g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
MT_S32 MT_DisEqcMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


static mt_s32 MT_DisEqcCheckParam(mt_input_Blindscan_para_t *p_BlindScan_in)
{
    if(p_BlindScan_in->startfreq >= p_BlindScan_in->endfreq)
    {
        SAMPLE_DISEQC_ERR_PRINT("Frequency setting error\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DisEqcDmxInit(mt_u32 tuner_id)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    /** Initializes the demux module */
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return ret;
    }


    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        if(tuner_id == 0)
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
            SAMPLE_DISEQC_INFO_PRINT("Connect port 0!\n");
        }
        else if(tuner_id == 1)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            /** Bind Demux to tuner port 3 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_3);
            SAMPLE_DISEQC_INFO_PRINT("Connect port 3!\n");
#else
            /** Bind Demux to tuner port 1 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            SAMPLE_DISEQC_INFO_PRINT("Connect port 1!\n");
#endif
        }
        else
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
            SAMPLE_DISEQC_INFO_PRINT("Connect port 0!\n");
        }
    }
    else
    {
        /** Bind Demux to tuner port 0 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        SAMPLE_DISEQC_INFO_PRINT("Connect port 1!\n");
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_DisEqcDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}


/*!
@brief audio and video player initialization.
@param[out] phAvplay            Handle to AV player
@param[out] hWin                The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DisEqcAvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_DISEQC_FUNCTION_ENTER();

    if(NULL == phAvplay)
    {
        SAMPLE_DISEQC_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phWin)
    {
        SAMPLE_DISEQC_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phSoundTrack)
    {
        SAMPLE_DISEQC_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        return ret;

    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
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
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }
    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_DISEQC_FUNCTION_EXIT();

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



/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return void

*/
static void  MT_DisEqcAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
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

/*
 @brief Audio and video Type of decoding
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_DisEqcSetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };

    SAMPLE_DISEQC_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DISEQC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_DISEQC_ERR_PRINT("=====pProgInfo == NULL=====\n");
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

    SAMPLE_DISEQC_INFO_PRINT("VidPid=%x, Vidtype=0x%x, AudPid=%x, AudType=0x%x \n", VidPid, enVidType, AudPid, u32AudType);

    /** Get the audio properties of the AV player */

    if(VidPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
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
        VdecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.s32CtrlOptions = 0;
        VdecAttr.u32Priority = 3;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("Set video properties or video PID property failed.\n");
            return ret;
        }
    }


    if(AudPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }
        /* PCM decoding mode*/
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("Setting the decoding mode or audio PID property failed:%#x\n",ret);
            return ret;
        }
    }

    /* insert pts*/
    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /* Defines the attribute of low delay*/

        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            return ret;
        }
    }

    SAMPLE_DISEQC_FUNCTION_EXIT();


    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DisEqcStarToPlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_DISEQC_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_DISEQC_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DISEQC_ERR_PRINT("hAvplay is not exist\n");
        return ret;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_DisEqcSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcSetAvplayPidAndCodecType fail! \n");
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
        SAMPLE_DISEQC_INFO_PRINT("Has no audio stream!\n");
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_DISEQC_INFO_PRINT("Has no video stream!\n");
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
            SAMPLE_DISEQC_ERR_PRINT("Set frame to VO fail.\n");
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("Get avplay sync attr fail!\n");
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
            SAMPLE_DISEQC_ERR_PRINT("Set avplay sync attr fail!\n");
            return ret;
        }
    }

    /*start to play audio and video*/
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("MT_UNF_AVPLAY_Start fail!  ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_DISEQC_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static MT_S32 MT_DisEqcConnect_Dvbs(MT_U32 tuner_id, MT_U32 freq, MT_U32 sym_rate, MT_U32 onoff_22k, MT_U32 polar, MT_U32 u32LoopTimes)
{
      MT_S32 ret = MT_FAILURE;
      MT_U32 u32Loop = 0;
      MT_U32 u32Freq = 0;
      MT_U32 u32SymbolRate = 0;
      mt_unf_fe_status_t stTunerStatus = { 0 };
      mt_unf_fe_connect_para_t s_stConnectPara = { 0 };


      s_stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
      s_stConnectPara.connect_param.sat.freq = freq * 1000;
      s_stConnectPara.connect_param.sat.sym_rate = sym_rate;
      s_stConnectPara.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
      s_stConnectPara.connect_param.sat.onoff_22k = onoff_22k;
      s_stConnectPara.connect_param.sat.polarization = polar;
      s_stConnectPara.connect_param.sat.lnb_status = 1;

      ret = mt_unf_fe_connect(tuner_id, &s_stConnectPara, 0);
      u32Freq = s_stConnectPara.connect_param.sat.freq;
      u32SymbolRate = s_stConnectPara.connect_param.sat.sym_rate;

    if (MT_SUCCESS == ret)
    {
        if (u32LoopTimes == 0)
        {

            u32LoopTimes = s_stConnectPara.channel_set_info.lock_time / 10;
        }

        for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++)
        {
            ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
            if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
            {
                SAMPLE_DISEQC_INFO_PRINT("Tuner Lock freq %d symb %d polar%d Success!\n", u32Freq, u32SymbolRate, polar);
                SAMPLE_DISEQC_INFO_PRINT("SUCCESS end\n");
                return MT_SUCCESS;
            }
            else
            {
                MT_USLEEP(10000);

            }
        }
    }
    else
    {
        SAMPLE_DISEQC_ERR_PRINT("Tuner Lock freq %d symb %d polar%d mt_unf_fe_connect Fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, polar, ret);
    }

    if (u32Loop == u32LoopTimes)
    {
        SAMPLE_DISEQC_ERR_PRINT("Tuner Lock freq %d symb %d  polar%d Fail!\n", u32Freq, u32SymbolRate, polar);
    }

    SAMPLE_DISEQC_ERR_PRINT("FAIL end\n");

    return MT_FAILURE;
}

/*!
@brief stop AV playback into the stop state.
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DisEqcStopToPlay(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;

    SAMPLE_DISEQC_INFO_PRINT("stop live play ...\n");

    /*stop playing audio and video*/
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}

static MT_S32 MT_DiseqcPlay(MT_HANDLE     hAvplay, MT_BOOL onoff_22k)
{
    MT_S32 ret = 0;
    MT_S32 j = 0;
    MT_S32 i = 0;
    MT_S32 k = -1;
    PMT_COMPACT_TBL *progTbl = MT_NULL;
    PMT_COMPACT_PROG   *stCurrentProgInfo = { 0 };

    for(i = 0; i<g_sTPinfo.s32TPNum; i++)
    {
        ret = MT_DisEqcConnect_Dvbs(g_sTPinfo.sInputParam.tuner_id,  (g_sTPinfo.satTPinfo[i].freq)/1000+1, (g_sTPinfo.satTPinfo[i].symbol_rate), onoff_22k, g_sTPinfo.satTPinfo[i].polar, 1000);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcConnect_Dvbs failed.\n");
            continue;
        }

        /** Get the PMT table */
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
            continue;
        }

        k++;
        printf("k = %d \n", k);
        for(j = 0; j<progTbl->prog_num; j++)
        {
            stCurrentProgInfo = progTbl->proginfo + j;
            g_sTPinfo.tppara[k].freq = (g_sTPinfo.satTPinfo[i].freq)/1000+1;
            g_sTPinfo.tppara[k].symbol_rate = g_sTPinfo.satTPinfo[i].symbol_rate;
            g_sTPinfo.tppara[k].dvb_type = g_sTPinfo.satTPinfo[i].dvb_type;
            g_sTPinfo.tppara[k].polar = g_sTPinfo.satTPinfo[i].polar;
            g_sTPinfo.tppara[k].program_num = progTbl->prog_num;
            g_sTPinfo.tppara[k].proginfo[j].VElementNum = stCurrentProgInfo->VElementNum;
            g_sTPinfo.tppara[k].proginfo[j].AElementNum = stCurrentProgInfo->AElementNum;
            g_sTPinfo.tppara[k].proginfo[j].PcrPid = stCurrentProgInfo->PcrPid;
            g_sTPinfo.tppara[k].proginfo[j].VElementPid = stCurrentProgInfo->VElementPid;
            g_sTPinfo.tppara[k].proginfo[j].VideoType = stCurrentProgInfo->VideoType;
            g_sTPinfo.tppara[k].proginfo[j].AElementPid = stCurrentProgInfo->AElementPid;
            g_sTPinfo.tppara[k].proginfo[j].AudioType = stCurrentProgInfo->AudioType;
            g_sTPinfo.tppara[k].proginfo[j].PmtPid = stCurrentProgInfo->PmtPid;
            g_sTPinfo.tppara[k].proginfo[j].ProgID = stCurrentProgInfo->ProgID;

        }
        (MT_VOID)MTADP_Search_FreeAllPmt(progTbl);
        g_sTPinfo.total_num += g_sTPinfo.tppara[k].program_num;
    }
    g_sTPinfo.s32TPNum = k + 1;

    ret = MT_DisEqcConnect_Dvbs(g_sTPinfo.sInputParam.tuner_id,  g_sTPinfo.tppara[0].freq, (g_sTPinfo.tppara[0].symbol_rate), onoff_22k, g_sTPinfo.tppara[0].polar, 1000);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcConnect_Dvbs failed.\n");
        return MT_FAILURE;
    }
	g_sTPinfo.stDiseqcRunInfo.tpIndex = 0;
    ret = MT_DisEqcStarToPlay(hAvplay, (g_sTPinfo.tppara[0].proginfo));
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcStarToPlay failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*
@brief set tuner parameters
@param[in] tuner_id,Port of the tuner
@param[in] sig_type,Type of received signal
@param[in] tuner_dev_type,tuner Device type
@param[in] tuner_addr, The address of tuner
@param[in] demod_dev_type, Type of the demod device
@param[in] demod_addr, The address of demod
@param[in] out_put_mode, Output mode
@param[in] I2c_channel, i2c Channel mode
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_DisEqc_Set_Parameter(MT_U32 tuner_id)
{
    MT_S32 ret = 0;
    MT_U32 sig_type;
    MT_U32 tuner_dev_type;
    MT_U32 tuner_addr;
    MT_U32 demod_dev_type;
    MT_U32 demod_addr;
    MT_U32 out_put_mode;
    MT_U32 I2c_channel;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };
    mt_sys_version_s stSysChipInfo = { 0 };

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#ifdef MT_SYM4_DSS
    sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
    tuner_dev_type = MT_UNF_TUNER_TYPE_M88TS6011;
    tuner_addr = 0x58;
    demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88DS6113;
    demod_addr = 0xD2;
    out_put_mode = 1;
    I2c_channel = 1;
#else
    sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
    tuner_dev_type = MT_UNF_TUNER_TYPE_M88TS6011;
    tuner_addr = 0x58;
    demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800;
    demod_addr = 0x18;
    out_put_mode = 1;
    I2c_channel = 0;

#endif
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    if(tuner_id == 1)
    {
        sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
        tuner_dev_type = MT_UNF_TUNER_TYPE_M88RS6060;
        tuner_addr = 0x5A;
        demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88RS6060;
        demod_addr = 0xD2;
        out_put_mode = 4;
        I2c_channel = 1;
    }
    else
    {
        sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
        tuner_dev_type = MT_UNF_TUNER_TYPE_M88TS6011;
        tuner_addr = 0x58;
        demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800;
        demod_addr = 0x18;
        out_put_mode = 1;
        I2c_channel = 0;
    }
#endif

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("mt_sys_get_version err!\n");
        return ret;
    }

    SAMPLE_DISEQC_INFO_PRINT("MTCommand_Tuner_Operation:   chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id,&mtTunerAttr);
    if(SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("mt_unf_fe_get_default_attr err!\n");
        return ret;
    }


    mtTunerAttr.sig_type = sig_type;

    if (stSysChipInfo.enChipVersion < MT_CHIP_SYMPHONY2_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY2_A3)
    {
        if ((tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC3800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC6800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_MXL_608))
        {
            mtTunerAttr.fe_config.tun2_type = tuner_dev_type;
            mtTunerAttr.fe_config.tun2_addr = tuner_addr;
        }
        else
        {
            mtTunerAttr.tuner_type = tuner_dev_type;
            mtTunerAttr.tuner_addr = tuner_addr;
        }
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY4_A1)
    {
        SAMPLE_DISEQC_INFO_PRINT("[%s %d]tuner_dev_type = %d, tuner_addr = 0x%x\n", __FUNCTION__, __LINE__, tuner_dev_type, tuner_addr);
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY6_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#endif
    mtTunerAttr.demod_dev_type = demod_dev_type;
    mtTunerAttr.demod_addr = demod_addr;
    mtTunerAttr.demod_i2c_id = I2c_channel;
    mtTunerAttr.output_mode = out_put_mode;
    mtTunerAttr.tuner_i2c_id[0] = 0;
    mtTunerAttr.no_need_init = 0;

    SAMPLE_DISEQC_INFO_PRINT("tuner_id[%d] tuner_type[%d], tuner_addr[0x%x], demod_type[%d], demod_addr[0x%x], demod_id[%d], output_mode[%d] \n",
                            tuner_id, mtTunerAttr.tuner_type, mtTunerAttr.tuner_addr,
                            mtTunerAttr.demod_dev_type, mtTunerAttr.demod_addr,
                            mtTunerAttr.demod_i2c_id, mtTunerAttr.output_mode );

    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("mt_unf_fe_set_attr failed.ret is %d\n",ret);
        return ret;
    }

    SAMPLE_DISEQC_INFO_PRINT("output_mode = %d\n", out_put_mode);

    return MT_SUCCESS;
}


static MT_S32 MT_DisEqc_Notify(MT_U32 tuner_id, mt_unf_fe_blindscan_evt_t enEVT, mt_unf_fe_blindscan_notify_t *punNotify)
{
  mt_s32 i = 0;
  mt_s32 num = 0;
  mt_unf_fe_sat_tpinfo_t temp;



  switch (enEVT)
  {
    case MT_UNF_FE_BLINDSCAN_EVT_STATUS:
      if(MT_UNF_FE_BLINDSCAN_STATUS_FAIL == *(punNotify->status))
      {
        SAMPLE_DISEQC_ERR_PRINT("Scan fail.\n");
        return MT_FAILURE;
      }
      else if ((MT_UNF_FE_BLINDSCAN_STATUS_FINISH == *(punNotify->status)) || (MT_UNF_FE_BLINDSCAN_STATUS_QUIT == *(punNotify->status)))
      {
        SAMPLE_DISEQC_PRINT("100%%");
        SAMPLE_DISEQC_PRINT("done\n");
        SAMPLE_DISEQC_INFO_PRINT("Scan over, find %d TP.\n", g_sTPinfo.s32TPNum);
        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_FINISH;
        return MT_SUCCESS;


      }
      break;

    case MT_UNF_FE_BLINDSCAN_EVT_PROGRESS:

      SAMPLE_DISEQC_PRINT("%d%% \n", *(punNotify->progress_percent));
      break;

    case MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT:
      num++;
      temp = *(punNotify->result);
      SAMPLE_DISEQC_PRINT("\t%03d %7d %5d %d %d, %d \n", num, temp.freq, temp.symbol_rate, temp.polar, temp.code_rate, temp.DataTsNumber);

      for (i = 0; i < temp.DataTsNumber; i++)
      {
            SAMPLE_DISEQC_PRINT("TS[%d] ---- 0x%02x\n", i, temp.DataTsIdArray[i]);
            (MT_VOID)mt_unf_fe_set_s2_multi_stream_ts_id(tuner_id, temp.DataTsIdArray[i]);

            MT_USLEEP(200000);
      }


      g_sTPinfo.satTPinfo[g_sTPinfo.s32TPNum] = *(punNotify->result);
      g_sTPinfo.s32TPNum++;
      g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
      break;

    default:
      break;
  }

  return MT_SUCCESS;
}
static MT_VOID  MT_DisEqc_set(MT_U32 u32TunerId, mt_unf_fe_polar_t enPolar, mt_unf_fe_lnb_22k_t enLNB22K)
{
    SAMPLE_DISEQC_INFO_PRINT("enPolar = %d \n", enPolar);
    SAMPLE_DISEQC_INFO_PRINT("enLNB22K = %d \n", enLNB22K);
}

static MT_S32 MT_DisEqc_Set_Lnb(mt_u32 tuner_id, MT_U32 low_lo, MT_U32 high_lo)
{


    mt_unf_fe_lnb_config_t lnb_config = { 0 };
    mt_s32 ret = 0;


    lnb_config.lnb_type = (low_lo == high_lo) ? MT_UNF_FE_LNB_SINGLE_FREQUENCY : MT_UNF_FE_LNB_DUAL_FREQUENCY; //enType;



    lnb_config.low_lo = low_lo;
    lnb_config.high_lo = high_lo;

    lnb_config.lnb_band = (low_lo >= 7500) ? MT_UNF_FE_LNB_BAND_KU : MT_UNF_FE_LNB_BAND_C; //enBand;


    ret = mt_unf_fe_set_lnb_config(tuner_id, &lnb_config);
    if(MT_SUCCESS != ret)
    {
        printf("call mt_unf_fe_set_lnb_config failed.\n");
        return ret;
    }
    return MT_SUCCESS;
}

static MT_S32 MT_DisEqc_Blindscan(mt_u32 tuner_id, MT_U8 bs_mode, MT_U32 u32StartFreq, MT_U32 u32StopFreq, MT_U8 polar, MT_U8 onoff_22k)
{
    MT_S32 ret = 0;
    mt_unf_fe_blindscan_para_t stBlindScanPara = { 0 };

    g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;

    memset(&stBlindScanPara, 0, sizeof(mt_unf_fe_blindscan_para_t));

    if (0 == bs_mode)
    {
        /* Auto */
        stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_AUTO;
        /* If your diseqc device need config polarization and 22K, you need register the callback */
        stBlindScanPara.scan_para.sat.diseqc_set = MT_DisEqc_set;
        stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))MT_DisEqc_Notify;
    }
    else
    {
        stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_MANUAL;
        stBlindScanPara.scan_para.sat.polar = polar;
        stBlindScanPara.scan_para.sat.lnb_22k = onoff_22k;
        stBlindScanPara.scan_para.sat.start_freq = u32StartFreq * 1000;
        stBlindScanPara.scan_para.sat.stop_freq = u32StopFreq * 1000;
        stBlindScanPara.scan_para.sat.diseqc_set = MT_DisEqc_set;
        stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))MT_DisEqc_Notify;
    }


    ret = mt_unf_fe_blindscan_start(tuner_id, &stBlindScanPara);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("call mt_unf_fe_blindscan_start failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}



static MT_VOID MT_DisEqcPrintMenu(MT_S32 pronum)
{

    if(0 != pronum)
    {
        SAMPLE_DISEQC_PRINT("\n 1 - %d : select the program   \n", pronum);
        SAMPLE_DISEQC_PRINT("     p : print TP program info \n");
    }
    SAMPLE_DISEQC_PRINT("     s : Switching port (diseqc 1.0)\n");
    SAMPLE_DISEQC_PRINT("     w : Switching port (diseqc 1.1)\n");
    SAMPLE_DISEQC_PRINT("     o : on/off 22k \n");
    SAMPLE_DISEQC_PRINT("     r : Restart scan TP \n");
    SAMPLE_DISEQC_PRINT("     k : to play the program  \n");
    SAMPLE_DISEQC_PRINT("     h : help \n");
    SAMPLE_DISEQC_PRINT("     q : quit \n");
    SAMPLE_DISEQC_PRINT("Diseqc>> ");

}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_DisEqcPrint_help(char *name)
{
    SAMPLE_DISEQC_PRINT("Options:\n");
    SAMPLE_DISEQC_PRINT(" %s -t tuner_id -b mode -l low_lo -g high_lo -s start_freq -t stop_freq -k 22k -p polar \n", name);
    SAMPLE_DISEQC_PRINT(" %s -t 0 -b 1 -l 5150 -g 5150 -s 1000 -e 2000 -k 1 -p 0\n", name);
    SAMPLE_DISEQC_PRINT(" %s -t 0 -b 1 -l 5150 -g 5750 -s 1000 -e 2000 -k 1 -p 0\n", name);
}



static MT_VOID MT_DisEqcPrintProgramInfo( mt_tpinfo_para_t tppara[32])
{
    MT_S32  i = 0;
    MT_S32  j = 0;
    MT_S32  k = 0;
    MT_S32  n = 1;

    MT_CHAR dvb_type[g_sTPinfo.s32TPNum][10];
    for(i = 0;i < g_sTPinfo.s32TPNum; i++)
    {
        switch(tppara[i].dvb_type)
        {
            case 0:
                strcpy(dvb_type[i], "DVB-S");
                break;
            case 1:
                strcpy(dvb_type[i], "DVB-S2");
                break;
            default:
                break;
        }
    }


    for(k = 0; k < g_sTPinfo.s32TPNum; k++)
    {
        SAMPLE_DISEQC_PRINT("<Index>          <Freq>           <Sym Rate>         <Type>    <polar>\n");
        SAMPLE_DISEQC_PRINT(" %3d         %10dKHz     %8dKSs           %5s      %4d\n", k+1, tppara[k].freq, tppara[k].symbol_rate, dvb_type[k], tppara[k].polar);
    for(i =0; i<tppara[k].program_num;i++)
    {
        SAMPLE_DISEQC_PRINT("Channel Num = %d, Program ServiceID = %d PMT PID = %x\n", i+1, tppara[k].proginfo[i].ProgID,tppara[k].proginfo[i].PmtPid);

         for (j = 0; j < tppara[k].proginfo[i].VElementNum;j++)
         {
             SAMPLE_DISEQC_PRINT("\tVideo Stream PID   = 0x%x\n",tppara[k].proginfo[i].VElementPid);
             switch (tppara[k].proginfo[i].VideoType)
             {
                case MT_UNF_VCODEC_TYPE_H264:
                    SAMPLE_DISEQC_PRINT("\tVideo Stream Type H264\n");
                    break;
                case MT_UNF_VCODEC_TYPE_MPEG2:
                    SAMPLE_DISEQC_PRINT("\tVideo Stream Type MP2\n");
                    break;
                case MT_UNF_VCODEC_TYPE_MPEG4:
                    SAMPLE_DISEQC_PRINT("\tVideo Stream Type MP4\n");
                    break;
                case MT_UNF_VCODEC_TYPE_HEVC:
                    SAMPLE_DISEQC_PRINT("\tVideo Stream Type HEVC\n");
                    break;
                default:
                    SAMPLE_DISEQC_PRINT("\tVideo Stream Type error\n");
             }
         }
         for (j = 0; j < tppara[k].proginfo[i].AElementNum; j++)
         {
            SAMPLE_DISEQC_PRINT("\tAudio Stream PID   = 0x%x\n", tppara[k].proginfo[i].AElementPid);

            switch (tppara[k].proginfo[i].AudioType)
            {
                case HA_AUDIO_ID_MP3:
                    SAMPLE_DISEQC_PRINT("\tAudio Stream Type MP3\n");
                    break;
                case HA_AUDIO_ID_AAC:
                    SAMPLE_DISEQC_PRINT("\tAudio Stream Type AAC\n");
                    break;
                case HA_AUDIO_ID_DOLBY_PLUS:
                    SAMPLE_DISEQC_PRINT("\tAudio Stream Type AC3\n");
                    break;
                case HA_AUDIO_ID_DTSHD:
                    SAMPLE_DISEQC_PRINT("\tAudio Stream Type DTS\n");
                    break;
                default:
                    SAMPLE_DISEQC_PRINT("\tAudio Stream Type error\n");
             }
         }
         SAMPLE_DISEQC_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

    }
    for(i = 0; i < g_sTPinfo.s32TPNum; i++)
    {
        SAMPLE_DISEQC_PRINT("TP:%2d( %2d - %2d) freq:%4dKHZ symbol_rate:%5dKSs \n", i+1, n, tppara[i].program_num + n-1, tppara[i].freq, tppara[i].symbol_rate);
         n = n + tppara[i].program_num;
    }
}


static MT_S32 MT_DisEqcCmdTask(MT_HANDLE     hAvplay, mt_input_Blindscan_para_t pInutParam)
{
    MT_S32  ret = 0;
    MT_U32 s4port = 0;
    MT_U32 s16port = 0;
    MT_CHAR inputCmd[32] = { 0 };
    MT_BOOL onoff_22k = pInutParam.onoff_22k;
    mt_unf_fe_diseqc_switch4port_t st4Port = { 0 };
    mt_unf_fe_diseqc_switch16port_t st16Port = { 0 };
    MT_S32 i = 0;
    MT_S32 progNum = 0;
    MT_S32 s32ProgNum = 0;


    while(1)
    {
        (MT_VOID)MT_DisEqcPrintMenu(g_sTPinfo.total_num);
        /* get inputCmd*/
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_DISEQC_INFO_PRINT("prepare to exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }

        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            progNum = atoi(inputCmd);
            if(progNum <= g_sTPinfo.total_num)
            {
                for(i = 0; i< g_sTPinfo.s32TPNum; i++)
                {
                    s32ProgNum = progNum - (g_sTPinfo.tppara[i].program_num);
                    if(s32ProgNum > 0)
                    {
                        progNum = s32ProgNum;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                SAMPLE_DISEQC_INFO_PRINT("The biggest num is %d \n", g_sTPinfo.total_num);
                continue;
            }

            if(g_sTPinfo.stDiseqcRunInfo.tpIndex != i)
            {
                ret = MT_DisEqcConnect_Dvbs(pInutParam.tuner_id,  g_sTPinfo.tppara[i].freq, g_sTPinfo.tppara[i].symbol_rate, onoff_22k, g_sTPinfo.tppara[i].polar, 1000);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcConnect_Dvbs failed.\n");
                }
                g_sTPinfo.stDiseqcRunInfo.tpIndex = i;
            }
            ret = MT_DisEqcStopToPlay(hAvplay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcStopToPlay failed.\n");
            }

            // restore ac4    attr info.
            MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
            ret = MT_DisEqcStarToPlay(hAvplay, g_sTPinfo.tppara[i].proginfo + progNum - 1);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcStarToPlay failed.\n");
            }


        }
        else if('k' == inputCmd[0])
        {
            if(0 == g_sTPinfo.total_num)
            {
                ret = MT_DiseqcPlay(hAvplay, onoff_22k);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISEQC_ERR_PRINT("MT_DiseqcPlay failed.\n");
                }
            }

        }
        else if('p' == inputCmd[0])
        {
            SAMPLE_DISEQC_INFO_PRINT("Print program info \n");
            (MT_VOID)MT_DisEqcPrintProgramInfo(g_sTPinfo.tppara);
        }
        else if('r' == inputCmd[0])
        {
            if(0 != g_sTPinfo.total_num)
            {
                ret = MT_DisEqcStopToPlay(hAvplay);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcStopToPlay failed.\n");
                }
            }
            memset(g_sTPinfo.tppara, 0, sizeof(mt_tpinfo_para_t) * MAX_TP_NUM);
            memset(g_sTPinfo.satTPinfo, 0, sizeof(mt_unf_fe_sat_tpinfo_t) * MAX_TP_NUM);
            g_sTPinfo.total_num = 0;
            g_sTPinfo.s32TPNum = 0;

            ret = MT_DisEqc_Set_Lnb(pInutParam.tuner_id, pInutParam.low_lo, pInutParam.high_lo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISEQC_ERR_PRINT("MT_DisEqc_Set_Lnb failed.\n");
                continue;
            }

            ret = MT_DisEqc_Blindscan(pInutParam.tuner_id, pInutParam.bs_mode, pInutParam.startfreq, pInutParam.endfreq, pInutParam.polar, onoff_22k);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISEQC_ERR_PRINT("MT_Blindscan_Dvbs failed.\n");
            }
            while(g_sTPinfo.bs_status != MT_BLINDSCAN_STATUS_FINISH)
            {

                usleep(5000);
            }
        }
        else if('s' == inputCmd[0])
        {
            SAMPLE_DISEQC_INFO_PRINT("please input port(1-4) \n");
            scanf("%d", &s4port);
            if((s4port >  0) && (s4port <= 4))
            {
                SAMPLE_DISEQC_INFO_PRINT("port = %d", s4port);
                st4Port.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
                st4Port.port = (mt_unf_fe_diseqc_switch_port_t)s4port;
                st4Port.polar = pInutParam.polar;
                st4Port.lnb_22k = onoff_22k;
                ret = mt_unf_fe_set_lnb_power(pInutParam.tuner_id, MT_UNF_FE_LNB_POWER_ON);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_DISEQC_ERR_PRINT("switch port failed\n");
                }
                ret = mt_unf_fe_diseqc_switch4port(pInutParam.tuner_id, &st4Port);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_DISEQC_ERR_PRINT("switch port failed\n");
                }
            }
            else
            {
                SAMPLE_DISEQC_ERR_PRINT("input err \n");
            }
        }
        else if('w' == inputCmd[0])
        {
            SAMPLE_DISEQC_INFO_PRINT("please input port(1-16) \n");
            scanf("%d", &s16port);
            if((s16port >  0) && (s16port <= 16))
            {
                SAMPLE_DISEQC_INFO_PRINT("port = %d", s16port);
                st16Port.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
                st16Port.port = (mt_unf_fe_diseqc_switch_port_t)s16port;
                ret = mt_unf_fe_set_lnb_power(pInutParam.tuner_id, MT_UNF_FE_LNB_POWER_ON);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_DISEQC_ERR_PRINT("switch port failed\n");
                }
                ret = mt_unf_fe_diseqc_switch16port(pInutParam.tuner_id, &st16Port);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_DISEQC_ERR_PRINT("switch port failed\n");
                }
            }
            else
            {
                SAMPLE_DISEQC_ERR_PRINT("input err \n");
            }

        }
        else if('o' == inputCmd[0])
        {
            if(onoff_22k == 1)
            {
                onoff_22k = 0;
            }
            else
            {
                onoff_22k = 1;
            }
            SAMPLE_DISEQC_INFO_PRINT("onoff_22k = %d", onoff_22k);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DISEQC_INFO_PRINT("Print Help info \n");
        }
    }
    return MT_SUCCESS;
}

/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
static mt_s32 MT_DisEqcParase_args(int argc, char *argv[], mt_input_Blindscan_para_t *pInutParam)
{
    mt_s32 opt = 0;

    while((opt = MTADP_Getopt(argc, argv, ":?hHb:l:g:t:e:p:k:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_DisEqcPrint_help(argv[0]);
            return MT_FAILURE;

            case 't':
                  pInutParam->tuner_id = strtol(mt_optarg, 0, 0);
                  break;
            case 'b':
                  pInutParam->bs_mode = strtol(mt_optarg, 0, 0);
                  break;
            case 'l':
                  pInutParam->low_lo = strtol(mt_optarg, 0, 0);
                  break;
            case 'g':
                  pInutParam->high_lo = strtol(mt_optarg, 0, 0);
                  break;

            case 's':
                  pInutParam->startfreq = strtol(mt_optarg, 0, 0);
                  break;

            case 'k':
                  pInutParam->onoff_22k = strtol(mt_optarg, 0, 0);
                  break;

            case 'e':
                  pInutParam->endfreq = strtol(mt_optarg, 0, 0);
                  break;

            case 'p':
                  pInutParam->polar = strtol(mt_optarg, 0, 0);
                  break;


            default:
                (void)MT_DisEqcPrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DisEqcMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32  ret = 0;


    if(argc != 17 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DisEqcPrint_help(argv[0]);
        return MT_SUCCESS;
    }
    ret = MT_DisEqcParase_args(argc, argv, &g_sTPinfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DISEQC_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {

        ret = MT_DisEqcCheckParam(&g_sTPinfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcCheckParam failed.\n");
            return ret;
        }
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_SYS_Init failed.\n");
            return ret;
        }
#endif
        ret = MTADP_Fe_Init(g_sTPinfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MTADP_Fe_Init failed.\n");
            goto ERR1;
        }
        ret = MT_DisEqc_Set_Parameter(g_sTPinfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_DisEqc_Set_Parameter failed.\n");
            goto ERR2;
        }
#ifndef MT_SAMPLE_APP
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MTADP_HDMI_Init failed.\n");
            goto ERR2;
        }
        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MTADP_Disp_Init failed.\n");
            goto ERR3;

        }
#endif
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {

            SAMPLE_DISEQC_ERR_PRINT("MTADP_VO_Init failed.\n");
            goto ERR4;
        }
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MTADP_Snd_Init failed.\n");
            goto ERR5;
        }

        ret = MT_DisEqcDmxInit(g_sTPinfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcDmxInit failed.\n");
            goto ERR6;
        }

        (MT_VOID)MTADP_Search_Init();

        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_INIT;


        ret = MT_DisEqcAvplayInit(&g_sTPinfo.stDiseqcRunInfo.hAvPlay, &g_sTPinfo.stDiseqcRunInfo.hWin, &g_sTPinfo.stDiseqcRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcAvplayInit failed.\n");
            goto ERR8;
        }

        g_bTaskQuit = MT_FALSE;
    }

    (void)MT_DisEqcCmdTask(g_sTPinfo.stDiseqcRunInfo.hAvPlay, g_sTPinfo.sInputParam);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
    if(0 != g_sTPinfo.total_num)
    {
        ret = MT_DisEqcStopToPlay(g_sTPinfo.stDiseqcRunInfo.hAvPlay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISEQC_ERR_PRINT("MT_DisEqcStopToPlay failed, ret = 0x%x\n", ret);
        }
    }


    /** Audio and video player deinitialization */
    (MT_VOID)MT_DisEqcAvplayDeInit(g_sTPinfo.stDiseqcRunInfo.hAvPlay, g_sTPinfo.stDiseqcRunInfo.hWin, g_sTPinfo.stDiseqcRunInfo.hSoundTrack);
ERR8:

    (MT_VOID)MTADP_Search_DeInit();

    /** Demux module deinitialization */
    (MT_VOID)MT_DisEqcDmxDeInit();
ERR6:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

ERR5:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();

ERR4:
#ifndef MT_SAMPLE_APP
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
#endif
ERR2:
    (MT_VOID)MTADP_Fe_DeInit(g_sTPinfo.sInputParam.tuner_id);

ERR1:
#ifndef MT_SAMPLE_APP
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif

    memset(&g_sTPinfo, 0, sizeof(mt_TPinfo_para_t));

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}

