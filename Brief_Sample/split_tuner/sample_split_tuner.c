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
#include "mt_adp_demux.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_SPLIT_TUNER_DEBUG

#define MT_SPLIT_TUNER_PRINT   printf
#else

#define MT_SPLIT_TUNER_PRINT

#endif

#define SAMPLE_SPLIT_TUNER_FUNCTION_ENTER() MT_SPLIT_TUNER_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SPLIT_TUNER_FUNCTION_EXIT()      MT_SPLIT_TUNER_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_SPLIT_TUNER_FATAL_PRINT(fmt...)      MT_SPLIT_TUNER_PRINT(" [FATAL] " fmt)
#define SAMPLE_SPLIT_TUNER_ERR_PRINT(fmt...)            MT_SPLIT_TUNER_PRINT(" [ERROR] " fmt)
#define SAMPLE_SPLIT_TUNER_WARN_PRINT(fmt...)           MT_SPLIT_TUNER_PRINT(" [WARN] "  fmt)
#define SAMPLE_SPLIT_TUNER_INFO_PRINT(fmt...)           MT_SPLIT_TUNER_PRINT(" [INFO] "  fmt)
#define SAMPLE_SPLIT_TUNER_DBG_PRINT(fmt...)            MT_SPLIT_TUNER_PRINT(" [DEBUG] " fmt)


#define  SAMPLE_SPLIT_TUNER_PRINT  printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MAX_TP_NUM          32

/*************************** Structure Definition ****************************/
typedef struct hiPMT_COMPACT_INFO_S
{
    mt_u32 ProgID;          /* program ID */
    mt_u32 PmtPid;          /*program PMT PID*/
    mt_u32 PcrPid;          /*program PCR PID*/

    mt_u32   VideoType;
    mt_u16               VElementNum;        /* video stream number */
    mt_u16               VElementPid;        /* the first video stream PID*/

    mt_u32   AudioType;
    mt_u16               AElementNum;        /* audio stream number */
    mt_u16               AElementPid;        /* the first audio stream PID*/

} PMT_COMPACT_info_t;



typedef struct
{
    mt_s32 sig_type;
    mt_u32 program_num;
    PMT_COMPACT_info_t proginfo[32];
} mt_tpinfo_para_t;

typedef struct
{
    mt_u32 freq; /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 mod_type; /**<QAM mode*/
} mt_input_cab_para_t;


typedef struct
{
    mt_u32 freq; /* frequency kHz */
    mt_u32 sym_rate;
    mt_u8 port_type;     //!<differ DVBS/DVBS2/AUTO from eatchother
    mt_u8 onoff_22k;                     //!< 22K on/off
    mt_u8 polarization;                  //!< Polarization
} mt_input_sat_para_t;

typedef struct
{

    mt_input_cab_para_t cab;
    mt_input_sat_para_t sat;

} mt_input_para_t;


typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
	mt_u32			   tpIndex;
} MT_Split_RUN_INFO;


typedef struct
{
    mt_s32 total_num;
    mt_input_para_t sInputParam;
    MT_Split_RUN_INFO stSplitRunInfo;
    mt_tpinfo_para_t tppara[MAX_TP_NUM];
}mt_TP_info_para_t;


/********************** Global Variable declaration **************************/
static MT_BOOL    g_bTaskQuit = MT_TRUE;
static mt_TP_info_para_t g_sTPinfo;


#ifdef MT_SAMPLE_APP
    MT_S32 MT_SplitTunerMain(MT_S32 argc, MT_CHAR *argv[]);
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif



/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_Split_TunerDmxInit(MT_VOID)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    /** Initializes the demux module */
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return ret;
    }



    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_Split_TunerDmxDeInit(MT_VOID)
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
static MT_S32 MT_Split_TunerAvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_SPLIT_TUNER_FUNCTION_ENTER();

    if(NULL == phAvplay)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phWin)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phSoundTrack)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        return ret;

    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
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
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }
    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_SPLIT_TUNER_FUNCTION_EXIT();

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
static void  MT_Split_TunerAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
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
static MT_S32 MT_Split_TunerSetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_info_t *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };
    MT_UNF_VCODEC_UNBLANK_E          unblank;


    SAMPLE_SPLIT_TUNER_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("=====pProgInfo == NULL=====\n");
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

    SAMPLE_SPLIT_TUNER_INFO_PRINT("VidPid=%x, Vidtype=0x%x, AudPid=%x, AudType=0x%x \n", VidPid, enVidType, AudPid, u32AudType);

    /** Get the audio properties of the AV player */

    if(VidPid != INVALID_TSPID)
    {
        MTADP_Get_VcodeUnblank(&unblank);
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
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
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("Set video properties or video PID property failed.\n");
            return ret;
        }
    }


    if(AudPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }
        /* PCM decoding mode*/
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("Setting the decoding mode or audio PID property failed:%#x\n",ret);
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
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            return ret;
        }
    }

    SAMPLE_SPLIT_TUNER_FUNCTION_EXIT();


    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_Split_TunerStarToPlay(mt_handle hAvplay, const PMT_COMPACT_info_t *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_SPLIT_TUNER_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("hAvplay is not exist\n");
        return ret;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_Split_TunerSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_Split_TunerSetAvplayPidAndCodecType fail! \n");
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
        SAMPLE_SPLIT_TUNER_INFO_PRINT("Has no audio stream!\n");
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_SPLIT_TUNER_INFO_PRINT("Has no video stream!\n");
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
            SAMPLE_SPLIT_TUNER_ERR_PRINT("Set frame to VO fail.\n");
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("Get avplay sync attr fail!\n");
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
            SAMPLE_SPLIT_TUNER_ERR_PRINT("Set avplay sync attr fail!\n");
            return ret;
        }
    }

    /*start to play audio and video*/
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_UNF_AVPLAY_Start fail!  ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_SPLIT_TUNER_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state.
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_Split_TunerStopToPlay(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;

    SAMPLE_SPLIT_TUNER_INFO_PRINT("stop live play ...\n");

    /*stop playing audio and video*/
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}

static MT_VOID MT_Split_TunerPrintProgramInfo( mt_tpinfo_para_t tppara[32])
{
    MT_S32  i = 0;
    MT_S32  j = 0;
    MT_S32  k = 0;

    int n = 1;

    for(i = 0; i < 2; i++)
    {
        SAMPLE_SPLIT_TUNER_PRINT("TP:%d( %d - %d) \n", i+1, n, tppara[i].program_num + n-1);


         n = n + tppara[i].program_num;
    }
    for(k = 0; k < 2;k++)
    {
        SAMPLE_SPLIT_TUNER_PRINT("<Index>\n");
        SAMPLE_SPLIT_TUNER_PRINT("%3d \n", k+1);
        for(i = 0; i<tppara[k].program_num;i++)
        {
            SAMPLE_SPLIT_TUNER_PRINT("Channel Num = %d, Program ServiceID = %d PMT PID = %x\n", i+1, tppara[k].proginfo[i].ProgID,tppara[k].proginfo[i].PmtPid);

             for (j = 0; j < tppara[k].proginfo[i].VElementNum;j++)
             {
                 SAMPLE_SPLIT_TUNER_PRINT("\tVideo Stream PID   = 0x%x\n",tppara[k].proginfo[i].VElementPid);
                 switch (tppara[k].proginfo[i].VideoType)
                 {
                    case MT_UNF_VCODEC_TYPE_H264:
                        SAMPLE_SPLIT_TUNER_PRINT("\tVideo Stream Type H264\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG2:
                        SAMPLE_SPLIT_TUNER_PRINT("\tVideo Stream Type MP2\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG4:
                        SAMPLE_SPLIT_TUNER_PRINT("\tVideo Stream Type MP4\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_HEVC:
                        SAMPLE_SPLIT_TUNER_PRINT("\tVideo Stream Type HEVC\n");
                        break;
                    default:
                        SAMPLE_SPLIT_TUNER_PRINT("\tVideo Stream Type error\n");
                 }
             }
             for (j = 0; j < tppara[k].proginfo[i].AElementNum; j++)
             {
                SAMPLE_SPLIT_TUNER_PRINT("\tAudio Stream PID   = 0x%x\n", tppara[k].proginfo[i].AElementPid);

                switch (tppara[k].proginfo[i].AudioType)
                {
                    case HA_AUDIO_ID_MP3:
                        SAMPLE_SPLIT_TUNER_PRINT("\tAudio Stream Type MP3\n");
                        break;
                    case HA_AUDIO_ID_AAC:
                        SAMPLE_SPLIT_TUNER_PRINT("\tAudio Stream Type AAC\n");
                        break;
                    case HA_AUDIO_ID_DOLBY_PLUS:
                        SAMPLE_SPLIT_TUNER_PRINT("\tAudio Stream Type AC3\n");
                        break;
                    case HA_AUDIO_ID_DTSHD:
                        SAMPLE_SPLIT_TUNER_PRINT("\tAudio Stream Type DTS\n");
                        break;
                    default:
                        SAMPLE_SPLIT_TUNER_PRINT("\tAudio Stream Type error\n");
                 }
             }
             SAMPLE_SPLIT_TUNER_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        }
    }
}


static MT_VOID MT_Split_TunerPrintMenu(MT_U32 prog_num)
{

    SAMPLE_SPLIT_TUNER_PRINT("\n 1 - %d : select the program \n", prog_num);
    SAMPLE_SPLIT_TUNER_PRINT("     p : print all tp program info \n");
    SAMPLE_SPLIT_TUNER_PRINT("     k : set unblank screen mode\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_SPLIT_TUNER_PRINT("     b : background run \n");
#endif
    SAMPLE_SPLIT_TUNER_PRINT("     h : help \n");
    SAMPLE_SPLIT_TUNER_PRINT("     q : quit \n");
    SAMPLE_SPLIT_TUNER_PRINT("Split_tuner>> ");

}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_Split_TunerPrint_help(char *name)
{
    SAMPLE_SPLIT_TUNER_PRINT("Options:\n");
    SAMPLE_SPLIT_TUNER_PRINT(" %s -m s_tuner -c freq sym_rate qam -s freq sym_rate 22k polar port_type \n", name);
    SAMPLE_SPLIT_TUNER_PRINT(" %s -m 0 -c 314 6875 64 -s 3840 27500 1 0 0 \n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_SPLIT_TUNER_PRINT(" %s -q  <exit> \n", name);
#endif

}

static MT_VOID MT_Split_TunerExit(void)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    (MT_VOID)MT_Split_TunerStopToPlay(g_sTPinfo.stSplitRunInfo.hAvPlay);

    (MT_VOID)MT_Split_TunerAvplayDeInit(g_sTPinfo.stSplitRunInfo.hAvPlay, g_sTPinfo.stSplitRunInfo.hWin, g_sTPinfo.stSplitRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_DeInit();

    (MT_VOID)MT_Split_TunerDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    memset(&g_sTPinfo, 0, sizeof(mt_TP_info_para_t));
    g_bTaskQuit = MT_TRUE;

    MTADP_Get_VcodeUnblank(&unblank);
    if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
    {
        MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
    }
}


static void MT_Split_TunerCmdTask(MT_HANDLE        hAvplay, MT_S32 total_num, mt_tpinfo_para_t tppara[32], mt_input_para_t *pInutParam)
{
    MT_S32 ret = 0;
    MT_S32 i = 0;
    MT_S32 progNum = 0;
    MT_S32 s32ProgNum = 0;
    MT_CHAR inputCmd[32] = { 0 };
    mt_s32 unblank = MT_UNF_VCODEC_UNBLANK_STABLE;


    while(1)
    {
        (MT_VOID)MT_Split_TunerPrintMenu(total_num);
        /* get inputCmd*/
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_SPLIT_TUNER_INFO_PRINT("prepare to exit!\n");
            if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
            {
                MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
            }
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_SPLIT_TUNER_INFO_PRINT("split_search play in back!\n");
            break;
        }
#endif
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            progNum = atoi(inputCmd);
            if(progNum <= total_num)
            {
                for(i = 0; i< 2; i++)
                {
                    s32ProgNum = progNum - (tppara[i].program_num);
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
                SAMPLE_SPLIT_TUNER_INFO_PRINT("The biggest num is %d \n", total_num);
                continue;
            }

            if(g_sTPinfo.stSplitRunInfo.tpIndex != i)
            {
                (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
                g_sTPinfo.stSplitRunInfo.tpIndex = i;
                if(2 == tppara[i].sig_type)
                {
                    (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
                    ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0, pInutParam->sat.freq, pInutParam->sat.sym_rate, pInutParam->sat.onoff_22k, pInutParam->sat.polarization, pInutParam->sat.port_type);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_Dvbs failed.\n");
                        continue;
                    }
                }
                else if(0 == tppara[i].sig_type)
                {
                    (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
                    ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0, pInutParam->cab.freq, pInutParam->cab.sym_rate, pInutParam->cab.mod_type);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
                        continue;
                    }
                }
                else if(1 == tppara[i].sig_type)
                {
                    (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
                    ret = MTADP_Fe_Connect_J83b(TUNER_ID_0, pInutParam->cab.freq, pInutParam->cab.sym_rate, pInutParam->cab.mod_type);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
                        continue;
                    }
                }



            }
            ret = MT_Split_TunerStopToPlay(hAvplay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_Split_TunerStopToPlay failed.\n");
            }

            // restore ac4    attr info.
            MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
            
            ret = MT_Split_TunerStarToPlay(hAvplay, tppara[i].proginfo + progNum - 1);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_Split_TunerStarToPlay failed.\n");
            }


        }
        else if('p' == inputCmd[0])
        {
            (MT_VOID)MT_Split_TunerPrintProgramInfo(tppara);
        }
        else if ('k' == inputCmd[0])
        {
            SAMPLE_SPLIT_TUNER_PRINT("input unblank mode(0:fast 1:stable 2:sync):");
            scanf("%d", &unblank);
            getchar();
            SAMPLE_SPLIT_TUNER_PRINT("unblank: %d \n", unblank);
            unblank = unblank % MT_UNF_VCODEC_UNBLANK_BUTT;
            MTADP_Set_VcodeUnblank(unblank);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_SPLIT_TUNER_INFO_PRINT("Print Help info \n");
        }

    }
}


/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
static mt_s32 MT_Split_TunerParase_args(int argc, char *argv[], mt_input_para_t *pInputParam)
{
        int opt = 0;
        //splite_tuner -c 314 6875 64 -s 3840 27500 1 0 0

        while((opt = MTADP_Getopt(argc, argv, ":?hH:m:q:")) != -1)
        {
            switch(opt)
            {
                case 'h':
                case '?':
                case 'H':
                    (void)MT_Split_TunerPrint_help(argv[0]);
                    return MT_FAILURE;

                case 'q':
                    if(g_bTaskQuit == MT_FALSE)
                    {
                        (MT_VOID)MT_Split_TunerExit();
                    }
                    return MT_TASK_EXIT;

                case 'm':
                    g_sTPinfo.tppara[0].sig_type = strtol(argv[2], 0, 0);
                    pInputParam->cab.freq = strtol(argv[4], 0, 0);
                    pInputParam->cab.sym_rate= strtol(argv[5], 0, 0);
                    pInputParam->cab.mod_type= strtol(argv[6], 0, 0);

                    pInputParam->sat.freq = strtol(argv[8], 0, 0);
                    pInputParam->sat.sym_rate = strtol(argv[9], 0, 0);
                    pInputParam->sat.onoff_22k = strtol(argv[10], 0, 0);
                    pInputParam->sat.polarization = strtol(argv[11], 0, 0);
                    pInputParam->sat.port_type = strtol(argv[12], 0, 0);
                    g_sTPinfo.tppara[1].sig_type = 2;
                    return MT_SUCCESS;
                default:
                    (void)MT_Split_TunerPrint_help(argv[0]);
                    return MT_FAILURE;
                break;
            }
        }
        return MT_SUCCESS;
    }



#ifdef MT_SAMPLE_APP
    MT_S32 MT_SplitTunerMain(MT_S32 argc, MT_CHAR *argv[])
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = 0;
    MT_S32 j = 0;
    MT_S32 i = 0;
    MT_S32 k = -1;
    PMT_COMPACT_TBL *progTbl = MT_NULL;
    PMT_COMPACT_PROG   *stCurrentProgInfo = { 0 };


    if(argc != 13 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_Split_TunerPrint_help(argv[0]);
        return MT_SUCCESS;
    }
    ret = MT_Split_TunerParase_args(argc, argv, &g_sTPinfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {

#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_SYS_Init failed.\n");
            return ret;
        }
#endif
        ret = MTADP_Fe_Init(TUNER_ID_0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Init failed.\n");
            goto ERR1;
        }


#ifndef MT_SAMPLE_APP
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_HDMI_Init failed.\n");
            goto ERR2;
        }
        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Disp_Init failed.\n");
            goto ERR3;

        }
#endif
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {

            SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_VO_Init failed.\n");
            goto ERR4;
        }
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Snd_Init failed.\n");
            goto ERR5;
        }

        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        ret = MT_Split_TunerDmxInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_Split_TunerDmxInit failed.\n");
            goto ERR7;
        }

        for(i = 0; i < 2; i++)
        {
            if(0 == i)
            {
                (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
                if(g_sTPinfo.tppara[0].sig_type == 0)
                {
                    ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0, g_sTPinfo.sInputParam.cab.freq, g_sTPinfo.sInputParam.cab.sym_rate, g_sTPinfo.sInputParam.cab.mod_type);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
                        goto ERR8;
                    }
                }
                else
                {
                    ret = MTADP_Fe_Connect_J83b(TUNER_ID_0, g_sTPinfo.sInputParam.cab.freq, g_sTPinfo.sInputParam.cab.sym_rate, g_sTPinfo.sInputParam.cab.mod_type);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_J83b failed.\n");
                        goto ERR8;
                    }
                }

            }
            else
            {
                (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0, g_sTPinfo.sInputParam.sat.freq, g_sTPinfo.sInputParam.sat.sym_rate, g_sTPinfo.sInputParam.sat.onoff_22k, g_sTPinfo.sInputParam.sat.polarization, g_sTPinfo.sInputParam.sat.port_type);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_Dvbs failed.\n");
                    goto ERR8;
                }
            }


            /** Get the PMT table */
            ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);

            if(MT_SUCCESS != ret)
            {
                SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
                goto ERR8;
            }

            k++;
            printf("k = %d \n", k);


            for(j = 0; j<progTbl->prog_num; j++)
            {
                stCurrentProgInfo = progTbl->proginfo + j;
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
            (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        }


        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        if(g_sTPinfo.tppara[0].sig_type == 0)
        {
            ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0, g_sTPinfo.sInputParam.cab.freq, g_sTPinfo.sInputParam.cab.sym_rate, g_sTPinfo.sInputParam.cab.mod_type);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
                goto ERR8;
            }
        }
        else
        {
            ret = MTADP_Fe_Connect_J83b(TUNER_ID_0, g_sTPinfo.sInputParam.cab.freq, g_sTPinfo.sInputParam.cab.sym_rate, g_sTPinfo.sInputParam.cab.mod_type);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SPLIT_TUNER_ERR_PRINT("MTADP_Fe_Connect_J83b failed.\n");
                goto ERR8;
            }
        }


        ret = MT_Split_TunerAvplayInit(&g_sTPinfo.stSplitRunInfo.hAvPlay, &g_sTPinfo.stSplitRunInfo.hWin, &g_sTPinfo.stSplitRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_Split_TunerAvplayInit failed.\n");
            goto ERR8;
        }

        g_sTPinfo.stSplitRunInfo.tpIndex = 0;
        ret = MT_Split_TunerStarToPlay(g_sTPinfo.stSplitRunInfo.hAvPlay, (g_sTPinfo.tppara[0].proginfo));
        if(MT_SUCCESS != ret)
        {

            SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_Split_TunerStarToPlay failed.\n");
            goto ERR9;
        }
        g_bTaskQuit = MT_FALSE;
    }

    (void)MT_Split_TunerCmdTask(g_sTPinfo.stSplitRunInfo.hAvPlay, g_sTPinfo.total_num, g_sTPinfo.tppara, &g_sTPinfo.sInputParam);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    /** Stop AV playback and enter the stop state */
    ret = MT_Split_TunerStopToPlay(g_sTPinfo.stSplitRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SPLIT_TUNER_ERR_PRINT("MT_Split_TunerStopToPlay failed, ret = %d\n", ret);
    }


ERR9:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_Split_TunerAvplayDeInit(g_sTPinfo.stSplitRunInfo.hAvPlay, g_sTPinfo.stSplitRunInfo.hWin, g_sTPinfo.stSplitRunInfo.hSoundTrack);
ERR8:

    /** Demux module deinitialization */
    (MT_VOID)MT_Split_TunerDmxDeInit();

ERR7:
    (MT_VOID)MTADP_Search_DeInit();

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

#ifndef MT_SAMPLE_APP
ERR2:
#endif
    /** Disconnect the tuner lock */
    (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

ERR1:
#ifndef MT_SAMPLE_APP
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif

    memset(&g_sTPinfo, 0, sizeof(mt_TP_info_para_t));

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}

