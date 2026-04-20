/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_demux.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_cmdline.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_DOLBY_MODE_DEBUG
#define MT_DOLBY_MODE_PRINT   printf
#else
#define MT_DOLBY_MODE_PRINT
#endif

#define SAMPLE_DOLBY_MODE_FUNCTION_ENTER()               MT_DOLBY_MODE_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DOLBY_MODE_FUNCTION_EXIT()                MT_DOLBY_MODE_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_DOLBY_MODE_FATAL_PRINT(fmt...)            MT_DOLBY_MODE_PRINT(" [FATAL] " fmt)
#define SAMPLE_DOLBY_MODE_ERR_PRINT(fmt...)              MT_DOLBY_MODE_PRINT(" [ERROR] " fmt)
#define SAMPLE_DOLBY_MODE_WARN_PRINT(fmt...)             MT_DOLBY_MODE_PRINT(" [WARN] "  fmt)
#define SAMPLE_DOLBY_MODE_INFO_PRINT(fmt...)             MT_DOLBY_MODE_PRINT(" [INFO] "  fmt)
#define SAMPLE_DOLBY_MODE_DBG_PRINT(fmt...)              MT_DOLBY_MODE_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DOLBY_MODE_PRINT   printf


#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef enum input_sig_type_t {
    MT_INPUT_SIG_TYPE_CAB = 1,
    /**<Cable signal*/
    MT_INPUT_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/
    MT_INPUT_SIG_TYPE_DVB_T = 3,
    /**<Terrestrial signal*/
    MT_INPUT_SIG_TYPE_FILE = 4,
    /**<local file */
}MT_INPUR_SIG_TYPE_T;

typedef struct
{
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
} mt_input_cab_para_t;

typedef struct
{
    MT_U32 freq; /* frequency kHz */
    MT_U32 sym_rate;
    MT_U8 port_type;     //!<differ DVBS/DVBS2/AUTO from eatchother
    MT_U8 onoff_22k;                     //!< 22K on/off
    MT_U8 polarization;                  //!< Polarization
} mt_input_sat_para_t;

typedef struct
{
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
    MT_U8 port_type;
} mt_input_ter_para_t;


typedef struct tagInput_Param_T
{
    MT_U8 file_name[256];
}mt_input_file_para_t;

typedef struct
{
    MT_INPUR_SIG_TYPE_T sig_type;
    union
    {
        mt_input_cab_para_t cab;
        mt_input_ter_para_t ter;
        mt_input_sat_para_t sat;
        mt_input_file_para_t file;
    } input_param;

} mt_input_para_t;
typedef struct
{
    MT_U8 file_name[256];
}source_file_param_t;
typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
} MT_DOLBY_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
static MT_DOLBY_RUN_INFO dolby_run_info;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_DolbyMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

#ifndef MT_SAMPLE_APP
/*!
@brief The thread that receives the file stream data
@param[in] args                 TS file name
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_DolbyModeInjectTsTask(MT_VOID *args)
{
    MT_S32              s32Ret = MT_SUCCESS;
    MT_U32              Readlen = 0;
    MT_HANDLE           hTsBuffer = MT_INVALID_HANDLE;
    MT_CHAR             *fileName = NULL;
    FILE                *pTsFile = NULL;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };

    fileName = (MT_CHAR*)args;
    pTsFile = fopen(fileName, "rb");
    if(NULL == pTsFile)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT( "file %s open error!!\n", fileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        fclose(pTsFile);
        return s32Ret;
    }

    s32Ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
        fclose(pTsFile);
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != s32Ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_DOLBY_MODE_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }

        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != s32Ret)
        {
           SAMPLE_DOLBY_MODE_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }
    (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    return MT_SUCCESS;
}


/*!
@brief Check if the QAM matches
@param[in]  mod_type            QAM
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static MT_S32 MT_DolbyModeCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_DOLBY_MODE_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Check if the DVBS param matches
@param[in]  p_sat_in            DVBS param
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static mt_s32 MT_DolbyModeCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_DOLBY_MODE_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error
@*/
static MT_S32 MT_DolbyModeSetAvplayPidAndCodecType(MT_HANDLE hAvplay      , const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           s32Ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };

    if(MT_INVALID_HANDLE == hAvplay || NULL == pProgInfo)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
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
        AudPid = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    SAMPLE_DOLBY_MODE_INFO_PRINT("VidPid = %#x, AudPid = %#x-------<%s> line: %d\n", VidPid, AudPid, __FUNCTION__, __LINE__);

    if(VidPid != INVALID_TSPID)
    {
        /** Get the video properties of the AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
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

        /** Set the video properties of the AV player */
        VdecAttr.enType = enVidType;
        VdecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.u32Priority = 3;
        VdecAttr.u32UseDescInfoFlag = 1;
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);

        /** Set the video PID properties of AV player */
        s32Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("Set video properties or video PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    if(AudPid != INVALID_TSPID)
    {
        /** Set audio decoder properties */
        s32Ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);

        /** Set the audio PID properties of AV player */
        s32Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("Setting the decoding mode or audio PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /** Set the audio and video synchronization properties of AV player */
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (MT_VOID *)&DmxAvsync);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error
@*/
static MT_S32 MT_DolbyModeStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        s32Ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    if(MT_INVALID_HANDLE == hAvplay || NULL == pProgInfo)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    /** Set the PID of the AV player and set the encoder type */
    MT_DolbyModeSetAvplayPidAndCodecType(hAvplay, pProgInfo);

    /** Get the audio PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("Has no audio stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    /** Get the video PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("Has no video stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    if((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("Set frame to VO is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Get synchronization properties of AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("Get avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 20;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.stSyncStartRegion.bSmoothPlay  = MT_TRUE;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_TRUE;
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("Set avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    s32Ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_Start failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state
@param[in] hAvplay              Handle to AV player
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_DolbyModeStopToPlay(MT_HANDLE hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;
    SAMPLE_DOLBY_MODE_INFO_PRINT("stop live play ...\n");
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}


/*!
@brief audio and video player initialization
@param[out] phAvplay            Handle to AV player
@param[out] phWin               The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_DolbyModeAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   s32Ret = MT_FAILURE;
    MT_HANDLE                hAvplay = MT_INVALID_HANDLE;
    MT_HANDLE                hWin = MT_INVALID_HANDLE;
    MT_HANDLE                hSoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Audio decoder */
    s32Ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** AV player initialization */
    s32Ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    s32Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Create AV player based on attributes */
    s32Ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_Create failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    s32Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }
    stTrackAttr.b_spdif_mod = MT_FALSE;
    /** Create a track based on the audio device model */
    s32Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_SND_CreateTrack failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    s32Ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_SND_Attach failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR5;
    }

    /** Create a window */
    s32Ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MTADP_VO_CreatWin failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    s32Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_VO_AttachWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR7;
    }

    /** Enable/disable windows */
    s32Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
static MT_VOID MT_DolbyModeAvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay || MT_INVALID_HANDLE == hWin || MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
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
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_DolbyModeDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
    {
        MT_S32           s32Ret = MT_FAILURE;
        mt_sys_version_s stSysChipInfo;

        /** Initializes the demux module */
        s32Ret = MT_UNF_DMX_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = 0x%x\n", s32Ret);
            return s32Ret;
        }

        if(MT_INPUT_SIG_TYPE_FILE == sig_type)
        {
            s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = 0x%x\n", s32Ret);
                (MT_VOID)MT_UNF_DMX_DeInit();
                return s32Ret;
            }
        }
        else
        {
            memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
            s32Ret = mt_sys_get_version(&stSysChipInfo);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("failed to mt_sys_get_version\n");
                return MT_FAILURE;
            }

            if(MT_INPUT_SIG_TYPE_CAB == sig_type)
            {
                if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
                {
                    s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
                }
                else
                {
                    s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
                }
            }
            else if(MT_INPUT_SIG_TYPE_SAT == sig_type)
            {
                if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
                {
                    s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
                }
                else
                {
                    s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
                }
            }
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
                return MT_FAILURE;
            }
        }

        return MT_SUCCESS;
    }

/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_DolbyModeDmxDeInit(MT_VOID)
{

    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();
    return;
}
#endif

static MT_VOID MT_DolbyExit(void)
{
    g_bTaskQuit = MT_TRUE;

#ifndef MT_SAMPLE_APP
    /** Stop playing the show */
    (MT_VOID)MT_DolbyModeStopToPlay(dolby_run_info.hAvPlay);

    /** Audio and video player deinitialization */
    (MT_VOID)MT_DolbyModeAvplayDeInit(dolby_run_info.hAvPlay, dolby_run_info.hWin, dolby_run_info.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(dolby_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == dolby_run_info.sInputParam.sig_type)
    {
        /** Wait for the thread to end */
        pthread_join(dolby_run_info.stInjectTSThread, NULL);
    }

    (MT_VOID)MT_DolbyModeDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != dolby_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
#endif

    memset(&dolby_run_info, 0xff, sizeof(dolby_run_info));
}


static MT_U8 *MT_DolbyHdmiModePrint(MT_UNF_SND_HDMI_MODE_E hdmi_mode)
{
    switch(hdmi_mode)
    {
        case MT_UNF_SND_HDMI_MODE_LPCM:
            return "LPCM_MODE";
        case MT_UNF_SND_HDMI_MODE_RAW:
            return "RAW_MODE";
        case MT_UNF_SND_HDMI_MODE_HBR2LBR:
            return "HBR2LBR_MODE";
        case MT_UNF_SND_HDMI_MODE_AUTO:
            return "AUTO_MODE";
        case MT_UNF_SND_HDMI_MODE_FORCE_DD:
            return "FORCE_DD_MODE";
        default:
            SAMPLE_DOLBY_MODE_ERR_PRINT("Error mode : %d \n", hdmi_mode);
            return MT_NULL;
    }
}

static MT_U8 *MT_DolbySpdifModePrint(MT_UNF_SND_SPDIF_MODE_E spdif_mode)
{
    switch(spdif_mode)
    {
        case MT_UNF_SND_SPDIF_MODE_LPCM:
            return "LPCM_MODE";
        case MT_UNF_SND_SPDIF_MODE_RAW:
            return "RAW_MODE";
        default:
            SAMPLE_DOLBY_MODE_ERR_PRINT("Error mode : %d \n", spdif_mode);
            return MT_NULL;
    }
}


static MT_S32 MT_DolbyHdmiSetMode(MT_UNF_SND_HDMI_MODE_E hdmi_mode)
{
    MT_S32 i = 0;
    MT_S32 s32Ret = MT_FAILURE;
    MT_UNF_SND_HDMI_MODE_E dolby_mode = MT_UNF_SND_HDMI_MODE_BUTT;
    MT_UNF_EDID_BASE_INFO_S stSinkCap;

    if(hdmi_mode == MT_UNF_SND_HDMI_MODE_AUTO)
    {
        s32Ret = MT_UNF_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_0, &stSinkCap);
        if(s32Ret != MT_SUCCESS)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("call MT_UNF_HDMI_GetSinkCapability failed.\n");
            return s32Ret;
        }

        for(i = stSinkCap.u32AudioInfoNum - 1; i >= 0; i--)
        {
            if(stSinkCap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP)
            {
                hdmi_mode = MT_UNF_SND_HDMI_MODE_RAW;
                SAMPLE_DOLBY_MODE_INFO_PRINT("HDMI support DD+\n");
                break;
            }
            else if(stSinkCap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3)
            {
                hdmi_mode = MT_UNF_SND_HDMI_MODE_FORCE_DD;
                SAMPLE_DOLBY_MODE_INFO_PRINT("HDMI support DD\n");
                break;
            }
            else if(stSinkCap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM)
            {
                hdmi_mode = MT_UNF_SND_HDMI_MODE_LPCM;
                SAMPLE_DOLBY_MODE_INFO_PRINT("HDMI support PCM\n");
                break;
            }
        }
    }

    (mt_void)MTADP_SND_SetHdmiMode(hdmi_mode);
    s32Ret = MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, hdmi_mode);
    if(s32Ret != MT_SUCCESS)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("call MT_UNF_SND_SetHdmiMode failed.\n");
        return s32Ret;
    }

    s32Ret = MT_UNF_SND_GetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, &dolby_mode);
    if(s32Ret != MT_SUCCESS)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("call MT_UNF_SND_GetHdmiMode failed.\n");
        return s32Ret;
    }

    SAMPLE_DOLBY_MODE_INFO_PRINT("Current HDMI DOBLY mode: %s \n", MT_DolbyHdmiModePrint(dolby_mode));

    return MT_SUCCESS;
}

static MT_S32 MT_DolbySpdifSetMode(MT_UNF_SND_SPDIF_MODE_E spdif_mode)
{
    MT_S32 s32Ret = MT_FAILURE;
    MT_UNF_SND_SPDIF_MODE_E dolby_mode = MT_UNF_SND_SPDIF_MODE_BUTT;

    (mt_void)MTADP_SND_SetSpidfMode(spdif_mode);
    s32Ret = MT_UNF_SND_SetSpdifMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_SPDIF0, spdif_mode);
    if(s32Ret != MT_SUCCESS)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("call MT_UNF_SND_SetSpdifMode failed.\n");
        return s32Ret;
    }

    s32Ret = MT_UNF_SND_GetSpdifMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_SPDIF0, &dolby_mode);
    if(s32Ret != MT_SUCCESS)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("call MT_UNF_SND_GetSpdifMode failed.\n");
        return s32Ret;
    }

    SAMPLE_DOLBY_MODE_INFO_PRINT("Current SPDIF DOBLY mode: %s \n", MT_DolbySpdifModePrint(dolby_mode));

    return MT_SUCCESS;
}


static MT_VOID MT_DolbyModePrintMenu(MT_U32 prog_num)
{
#ifndef MT_SAMPLE_APP
    SAMPLE_DOLBY_MODE_PRINT("\n 1 - %d : select the program \n", prog_num);
#endif
    SAMPLE_DOLBY_MODE_PRINT("     f : change hdmi mode\n");
    SAMPLE_DOLBY_MODE_PRINT("     c : change spdif mode\n");
	SAMPLE_DOLBY_MODE_PRINT("     d : dolby downmix mode, 0:raw mode, 1: downmix LtRt, 2:downmix LoRo(default)\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_DOLBY_MODE_PRINT("     b : background run \n");
#endif
    SAMPLE_DOLBY_MODE_PRINT("     h : help \n");
    SAMPLE_DOLBY_MODE_PRINT("     q : quit \n");
    SAMPLE_DOLBY_MODE_PRINT("DOLBY>> ");

}


/*!
@brief The thread on which the command was entered
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgTbl            The data structure of the PMT
@return::MT_VOID
@*/
static MT_VOID MT_DolbyModeCmdTask(MT_HANDLE hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_S32                  s32Ret = MT_SUCCESS;
    MT_S32                  mode = 0;
    MT_CHAR                 inputCmd[32] = { 0 };
#ifndef MT_SAMPLE_APP
    MT_U32                  u32ProgNum = 0;
    PMT_COMPACT_PROG        *pstCurrentProgInfo = MT_NULL;
#endif
    MT_UNF_SND_SPDIF_MODE_E spdifMode = MT_UNF_SND_SPDIF_MODE_LPCM;
    MT_UNF_SND_HDMI_MODE_E  hdmiMode = MT_UNF_SND_HDMI_MODE_LPCM;
	MT_U32                  downmixmode = 0;
	MT_HANDLE               hAvplay_temp = MT_INVALID_HANDLE;
#ifndef MT_SAMPLE_APP
    if(MT_INVALID_HANDLE == hAvplay || NULL == pProgTbl)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    while(1)
    {
        (MT_VOID)MT_DolbyModePrintMenu(pProgTbl->prog_num);

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if(inputCmd[0] == 'q')
        {
            SAMPLE_DOLBY_MODE_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_DOLBY_MODE_INFO_PRINT("Dolby play in back!\n");
            break;
        }
#endif
#ifndef MT_SAMPLE_APP
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);

            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1) % pProgTbl->prog_num);

                /** Stop AV playback into the stop state */
                s32Ret = MT_DolbyModeStopToPlay(hAvplay);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_DOLBY_MODE_ERR_PRINT("DOLBY_StopToPlay failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
                }
                SAMPLE_DOLBY_MODE_INFO_PRINT("===== Start play ProgNum: %d \n", u32ProgNum);

                /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
                s32Ret = MT_DolbyModeStarToPlay(hAvplay, pstCurrentProgInfo);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_DOLBY_MODE_ERR_PRINT("Switching shows failed\n");
                }
            }
            else
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
                continue;
            }
        }
#endif
        else if('f' == inputCmd[0])
        {
            SAMPLE_DOLBY_MODE_PRINT("Please select a mode\n"
                                "    0-HDMI LCPM2.0\n"
                                "    1-HDMI Pass-through\n"
                                "    2-HDMI Pass-through force high-bitrate to low-bitrate\n"
                                "    3-automatically match according to the EDID of HDMI\n"
                                "    4-HDMI output dolby digit format when dolby digit plus input\n"
                                ">> ");
            scanf("%d", &mode);
            getchar();
            hdmiMode = mode;
            SAMPLE_DOLBY_MODE_INFO_PRINT("Change DOLBY mode to [%s]. \n", MT_DolbyHdmiModePrint(hdmiMode));

            s32Ret = MT_DolbyHdmiSetMode(hdmiMode);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("Change mode fail!\n");
            }

        }
        else if('c' == inputCmd[0])
        {
            SAMPLE_DOLBY_MODE_PRINT("Please select a mode\n"
                                "    0-HDMI LCPM2.0\n"
                                "    1-HDMI Pass-through\n"
                                ">> ");
            scanf("%d", &mode);
            getchar();
            spdifMode = mode;
            SAMPLE_DOLBY_MODE_INFO_PRINT("Change DOLBY mode to [%s]. \n", MT_DolbySpdifModePrint(spdifMode));

            s32Ret = MT_DolbySpdifSetMode(spdifMode);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("Change mode fail!\n");
            }

        }
        else if('d' == inputCmd[0])
        {
            SAMPLE_DOLBY_MODE_PRINT("Please select a mode\n"
                                "    0-raw mode\n"
                                "    1-downmix LtRt\n"
                                "    2-downmix LoRo\n"
                                ">> ");
            scanf("%d", &mode);
            getchar();

			if(mode > 2) {
				SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> set dolby downmix mode para error!\n", __func__, __LINE__);
				return;
			}

			if(MT_INVALID_HANDLE == hAvplay || 0 == hAvplay) {
				MT_UNF_AVPLAY_ATTR_S AvplayAttr;

				SAMPLE_DOLBY_MODE_INFO_PRINT("create temp avplay handle...\n");
				s32Ret =MT_UNF_AVPLAY_Init();
				if (MT_SUCCESS != s32Ret)
				{
					SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_Init error!\n", __func__, __LINE__);
					return;
				}
				
				memset(&AvplayAttr,0,sizeof(MT_UNF_AVPLAY_ATTR_S));
				s32Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
				s32Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay_temp);
				if (MT_SUCCESS != s32Ret)
				{
					SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_Create error!\n", __func__, __LINE__);
					(MT_VOID)MT_UNF_AVPLAY_DeInit();
					return;
				}
				
				s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay_temp, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
				if (MT_SUCCESS != s32Ret)
				{
					SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_ChnOpen error!\n", __func__, __LINE__);
					(MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay_temp);
					(MT_VOID)MT_UNF_AVPLAY_DeInit();
				}
				hAvplay = hAvplay_temp;
			}

            switch(mode) 
			{
				case 0:
					SAMPLE_DOLBY_MODE_INFO_PRINT("set dolby raw mode\n");
					s32Ret = MT_UNF_AVPLAY_set_downmix_enable(hAvplay, 0);
					if (MT_SUCCESS != s32Ret)
					{
						SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> set dolby raw mode error!\n", __func__, __LINE__);
					}
					break;

				case 1:
				case 2:
					downmixmode = (1 == mode) ? 1 : 0;
					SAMPLE_DOLBY_MODE_INFO_PRINT("set dolby %s mode\n",(downmixmode==1) ? "LtRt":"LoRo");
					s32Ret = MT_UNF_AVPLAY_set_downmix_enable(hAvplay, 1);
					if (MT_SUCCESS != s32Ret)
					{
						SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> Set dolby downmix mode failed, s32Ret = %d\n", __func__, __LINE__, s32Ret);
						break;
					}
					s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DOLBY_DOWNMIX_MODE, &downmixmode);
					if(MT_SUCCESS != s32Ret)
					{
						SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> Set dolby downmix mode failed, s32Ret = %d\n", __func__, __LINE__, s32Ret);
						break;
					}
					break;

				default:
					SAMPLE_DOLBY_MODE_ERR_PRINT("<%s:%d> set dolby downmix mode para error!\n", __func__, __LINE__);
					break;
			}

			if(MT_INVALID_HANDLE != hAvplay_temp)
			{
				SAMPLE_DOLBY_MODE_INFO_PRINT("close temp avplay handle...\n");
				(MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay_temp, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
				(MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay_temp);
				(MT_VOID)MT_UNF_AVPLAY_DeInit();
			}
			else
			{
				SAMPLE_DOLBY_MODE_INFO_PRINT("reset audio to take effect...\n");
				MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };
				option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
				option.u32TimeoutMs = 0;
				s32Ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
				if(MT_SUCCESS != s32Ret)
				{
					SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_Stop failed ret=0x%x \n", s32Ret);
					return;
				}
				
				s32Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
				if(MT_SUCCESS != s32Ret)
				{
					SAMPLE_DOLBY_MODE_ERR_PRINT("MT_UNF_AVPLAY_Start failed ret=0x%x \n", s32Ret);
					return;
				}
			}
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DOLBY_MODE_INFO_PRINT("Print help info \n");
            continue;
        }
    }

}


static MT_VOID MT_DolbyModePrint_Help(MT_CHAR *name)
{
#ifndef MT_SAMPLE_APP
    SAMPLE_DOLBY_MODE_PRINT("Lack of parameters\n");
    SAMPLE_DOLBY_MODE_PRINT("\nUsage:\n");
    SAMPLE_DOLBY_MODE_PRINT("%s\n", name);
    SAMPLE_DOLBY_MODE_PRINT("    -f: path of the stream file\n");
    SAMPLE_DOLBY_MODE_PRINT("    -c: DVBC locks frequency\n");
    SAMPLE_DOLBY_MODE_PRINT("    -s: DVBS locks frequency\n");
    SAMPLE_DOLBY_MODE_PRINT("example:\n");
    SAMPLE_DOLBY_MODE_PRINT("    %s -f ./6ch_voices_id_7_dd_DVB_h264_25fps.trp\n",name);
    SAMPLE_DOLBY_MODE_PRINT("    %s -c 314 6875 64\n",name);
    SAMPLE_DOLBY_MODE_PRINT("    %s -s 3840 27500 1 0 0\n",name);
#endif

#ifdef MT_SAMPLE_APP
    SAMPLE_DOLBY_MODE_PRINT("    -q: Exit the background\n");
#endif
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_DolbyModeParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;
#ifndef MT_SAMPLE_APP
    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DolbyModePrint_Help(argv[0]);
        return MT_FAILURE;
    }
#endif
    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_DolbyModePrint_Help(argv[0]);
                return MT_FAILURE;
        #ifndef MT_SAMPLE_APP
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_DolbyModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_DolbyModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_SAT;

                pInputParam->input_param.sat.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.sat.sym_rate = strtol(argv[3], 0, 0);
                pInputParam->input_param.sat.onoff_22k = strtol(argv[4], 0, 0);
                pInputParam->input_param.sat.polarization = strtol(argv[5], 0, 0);
                pInputParam->input_param.sat.port_type = strtol(argv[6], 0, 0);

                return MT_SUCCESS;

            case 'c':
                if(argc != 5)
                {
                    (MT_VOID)MT_DolbyModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
        #endif
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DolbyExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_DolbyModePrint_Help(argv[0]);
                return MT_FAILURE;

        }
    }

    return MT_SUCCESS;
}



#ifdef MT_SAMPLE_APP
MT_S32 MT_DolbyMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32                 s32Ret = MT_SUCCESS;
#ifndef MT_SAMPLE_APP
    PMT_COMPACT_PROG       *pstCurrentProgInfo = MT_NULL;
#endif
    s32Ret = MT_DolbyModeParase_args(argc, argv, &dolby_run_info.sInputParam);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
    #ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }

        /** HDMI initialization */
        s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to StartDmx\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }

        g_bTaskQuit = MT_FALSE;
        if(MT_INPUT_SIG_TYPE_FILE != dolby_run_info.sInputParam.sig_type)
        {
            s32Ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }

            if (MT_INPUT_SIG_TYPE_CAB == dolby_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_DolbyModeCheckDvbcParam(&dolby_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_DOLBY_MODE_ERR_PRINT("MT_DolbyModeCheckDvbcParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR3;
                }

                s32Ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            dolby_run_info.sInputParam.input_param.cab.freq,
                                            dolby_run_info.sInputParam.input_param.cab.sym_rate,
                                            dolby_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == dolby_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_DolbyModeCheckDvbsParam(&dolby_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_DOLBY_MODE_ERR_PRINT("MT_CCModeCheckDvbsParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR3;
                }

                s32Ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            dolby_run_info.sInputParam.input_param.sat.freq,
                                            dolby_run_info.sInputParam.input_param.sat.sym_rate,
                                            dolby_run_info.sInputParam.input_param.sat.onoff_22k,
                                            dolby_run_info.sInputParam.input_param.sat.polarization,
                                            dolby_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR3;
            }
        }

        /** VO device initialization */
        s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR3;
        }

        /** Sound module initialization */
        s32Ret = MTADP_Snd_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        /** DMX module initialization */
        s32Ret = MT_DolbyModeDmxInit(dolby_run_info.sInputParam.sig_type);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to StartDmx\n");
            goto ERR5;
        }

        if(MT_INPUT_SIG_TYPE_FILE == dolby_run_info.sInputParam.sig_type)
        {
            s32Ret = pthread_create(&dolby_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_DolbyModeInjectTsTask, &dolby_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_DOLBY_MODE_ERR_PRINT("failed to pthread_create thread.\n");
                goto ERR6;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR6;
            }
        }

        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        s32Ret = MTADP_Search_GetAllPmt(DMX_ID_0, &dolby_run_info.pProgTbl);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");

            goto ERR8;
        }

        /** AVPLAY initialization */
        s32Ret = MT_DolbyModeAvplayInit(&dolby_run_info.hAvPlay, &dolby_run_info.hWin, &dolby_run_info.hSoundTrack);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to DOLBY_AvplayInit\n");
            goto ERR9;
        }

        /** Select the program */
        pstCurrentProgInfo = dolby_run_info.pProgTbl->proginfo;

        /** Start playing the show */
        s32Ret = MT_DolbyModeStarToPlay(dolby_run_info.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to DOLBY_StarToPlay\n");
            goto ERR10;
        }
    #endif
        g_bTaskQuit = MT_FALSE;
    }

#ifdef MT_SAMPLE_APP
    dolby_run_info.hAvPlay = avplayHandle.hAvPlay;
    dolby_run_info.hSoundTrack = avplayHandle.hSoundTrack;
#endif

    if (dolby_run_info.hSoundTrack <= 0)
    {
        s32Ret = MTADP_Snd_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DOLBY_MODE_ERR_PRINT("failed to MTADP_Snd_Init\n");
            return s32Ret;
        }
    }

    (MT_VOID)MT_DolbyModeCmdTask(dolby_run_info.hAvPlay, dolby_run_info.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifndef MT_SAMPLE_APP
    /** Stop playing the show */
    s32Ret = MT_DolbyModeStopToPlay(dolby_run_info.hAvPlay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DOLBY_MODE_ERR_PRINT("DOLBY_StopToPlay failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

ERR10:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_DolbyModeAvplayDeInit(dolby_run_info.hAvPlay, dolby_run_info.hWin, dolby_run_info.hSoundTrack);
ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(dolby_run_info.pProgTbl);
ERR8:
    (MT_VOID)MTADP_Search_DeInit();
ERR7:
    if(MT_INPUT_SIG_TYPE_FILE == dolby_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;

        /** Wait for the thread to end */
        (MT_VOID)pthread_join(dolby_run_info.stInjectTSThread, NULL);
    }
ERR6:
    /** Demux module deinitialization */
    (MT_VOID)MT_DolbyModeDmxDeInit();
ERR5:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();
ERR4:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR3:
    if(MT_INPUT_SIG_TYPE_FILE != dolby_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR2:
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif

    if (dolby_run_info.hSoundTrack <= 0)
    {
        (MT_VOID)MTADP_Snd_DeInit();
    }

    g_bTaskQuit = MT_TRUE;
    memset(&dolby_run_info, 0xff, sizeof(dolby_run_info));

    return s32Ret;
}
