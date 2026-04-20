/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
/***************************** Macro Definition ******************************/
#define MT_SAMPLE_BOX_DEBUG
#ifdef  MT_SAMPLE_BOX_DEBUG
#define MT_BOX_PRINT   printf
#else
#define MT_BOX_PRINT
#endif

#define SAMPLE_BOX_FUNCTION_ENTER()             MT_BOX_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_BOX_FUNCTION_EXIT()              MT_BOX_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_BOX_FATAL_PRINT(fmt...)          MT_BOX_PRINT(" [FATAL] " fmt)
#define SAMPLE_BOX_ERR_PRINT(fmt...)            MT_BOX_PRINT(" [ERROR] " fmt)
#define SAMPLE_BOX_WARN_PRINT(fmt...)           MT_BOX_PRINT(" [WARN] "  fmt)
#define SAMPLE_BOX_INFO_PRINT(fmt...)           MT_BOX_PRINT(" [INFO] "  fmt)
#define SAMPLE_BOX_DBG_PRINT(fmt...)            MT_BOX_PRINT(" [DEBUG] " fmt)

#define DMXID               0
/*************************** Structure Definition ****************************/
typedef struct tagCmdLinePara
{
    mt_u32  u32TunerFreq;
    mt_u32  u32TunerSymbolRate;
    mt_u32  u32TunerQam;
    mt_char chFileName[100];
}CMD_LINE_ST;
/********************** Global Variable declaration **************************/
MT_BOOL   g_bTaskQuit = MT_FALSE;
mt_handle g_Avplay = 0;
mt_handle hTsBuffer = 0;
/******************************* API declaration *****************************/
/*!
@brief The thread that receives the file stream data
@param[in] args                 TS file name
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static mt_s32 InjectTsTask(mt_void *args)
{
    mt_s32                 s32Ret = MT_SUCCESS;
    mt_u32                 Readlen = 0;
    mt_char                *fileName = NULL;
    FILE                   *pTsFile = NULL;
    MT_UNF_STREAM_BUF_S    StreamBuf = { 0 };

    fileName = (mt_char*)args;
    pTsFile = fopen(fileName, "rb");
    if(NULL == pTsFile)
    {
        SAMPLE_BOX_ERR_PRINT( "\nfile %s open error!!\n", fileName);
        return MT_FAILURE;
    }

    /** loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        /** Obtains a TS buffer to input data */
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*200, &StreamBuf, 1000);
        if(MT_SUCCESS != s32Ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            if(g_Avplay)
            {
                /** Resets an AVPLAY. In this case */
                (mt_void)MT_UNF_AVPLAY_Reset(g_Avplay, NULL);
            }

            /** Resets a TS buffer to clear its data */
            (mt_void)MT_UNF_DMX_ResetTSBuffer(hTsBuffer);

            SAMPLE_BOX_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");
            rewind(pTsFile);
            continue;
        }

        /** Updates the write pointer of a TS buffer after the TS data is input */
        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != s32Ret)
        {
           SAMPLE_BOX_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
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
static mt_s32 BOX_SetAvplayPidAndCodecType(mt_handle hAvplay      , const PMT_COMPACT_PROG *pProgInfo)
{
    mt_s32                           s32Ret = MT_FAILURE;
    mt_u32                           u32AudType = 0;
    mt_u32                           VidPid = 0;
    mt_u32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_STOP_OPT_S         Stop = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };

    if(NULL == hAvplay || NULL == pProgInfo)
    {
        SAMPLE_BOX_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
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

    SAMPLE_BOX_INFO_PRINT("VidPid = %#x, AudPid = %#x-------<%s> line: %d\n", VidPid, AudPid, __FUNCTION__, __LINE__);

    if(VidPid != INVALID_TSPID)
    {
        /** Get the video properties of the AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Set the video properties of the AV player */
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
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);

        /** Set the video PID properties of AV player */
        s32Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_BOX_ERR_PRINT("Set video properties or video PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_BOX_ERR_PRINT("Setting the decoding mode or audio PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /** Set the audio and video synchronization properties of AV player */
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
static mt_s32 BOX_StarToPlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    mt_u32                        s32Ret = MT_FAILURE;
    mt_u32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    /** Set the PID of the AV player and set the encoder type */
    BOX_SetAvplayPidAndCodecType(hAvplay, pProgInfo);

    /** Get the audio PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_BOX_ERR_PRINT("Has no audio stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    /** Get the video PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_BOX_ERR_PRINT("Has no video stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_BOX_ERR_PRINT("Set frame to VO is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Get synchronization properties of AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_BOX_ERR_PRINT("Get avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_BOX_ERR_PRINT("Set avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    s32Ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_Start failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
static mt_s32 BOX_StopToPlay(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;
    SAMPLE_BOX_INFO_PRINT("stop live play ...\n");
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
static mt_s32 BOX_AvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    mt_s32                   s32Ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_BOX_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Audio decoder */
    s32Ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** AV player initialization */
    s32Ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    s32Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Create AV player based on attributes */
    s32Ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_Create failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    s32Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    s32Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_SND_CreateTrack failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    s32Ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_SND_Attach failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR5;
    }

    /** Create a window */
    s32Ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MTADP_VO_CreatWin failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    s32Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_VO_AttachWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR7;
    }

    /** Enable/disable windows */
    s32Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR8;
    }

    *phWin = hWin;
    *phSoundTrack = hSoundTrack;
    *phAvplay = hAvplay;

    return MT_SUCCESS;

ERROR8:
    s32Ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_VO_DestroyWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERROR7:
    s32Ret = MT_UNF_VO_DestroyWindow(hWin);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_VO_DestroyWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERROR6:
    s32Ret = MT_UNF_SND_Detach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_SND_Detach failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERROR5:
    s32Ret = MT_UNF_SND_DestroyTrack(hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_SND_DestroyTrack failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERROR4:
    s32Ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_ChnClose failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERROR3:
    s32Ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_ChnClose failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERROR2:
    s32Ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_Destroy failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERROR1:
    s32Ret = MT_UNF_AVPLAY_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_DeInit failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

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
static mt_s32 BOX_AvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_s32 flag = MT_SUCCESS;

    /** Enable/disable windows */
    s32Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Unbind the window and AV player */
    s32Ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_VO_DetachWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Destroy window */
    s32Ret = MT_UNF_VO_DestroyWindow(hWin);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_VO_DestroyWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Contact the binding of track and AV player */
    s32Ret = MT_UNF_SND_Detach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_SND_Detach failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Destroy a Track */
    s32Ret = MT_UNF_SND_DestroyTrack(hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_SND_DestroyTrack failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Turn off the video channel */
    s32Ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_ChnClose failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Turn off the audio channel */
    s32Ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_ChnClose failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Destroy the AV player */
    s32Ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_Destroy failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    /** Deinitializes the AV player module */
    s32Ret = MT_UNF_AVPLAY_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_AVPLAY_DeInit failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        flag = MT_FAILURE;
    }

    if(MT_SUCCESS != flag)
    {
        return flag;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static mt_s32 BOX_DmxInit()
{
    mt_s32           s32Ret = MT_FAILURE;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    s32Ret = MT_UNF_DMX_AttachTSPort(DMXID, MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        (mt_void)MT_UNF_DMX_DeInit();
        return s32Ret;
    }

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        (mt_void)MT_UNF_DMX_DetachTSPort(DMXID);
        (mt_void)MT_UNF_DMX_DeInit();
        return s32Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::void
@*/
static mt_void BOX_DmxDeInit()
{
    mt_s32 s32Ret = MT_FAILURE;

    /** Destroys an existing TS buffer */
    s32Ret = MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_DMX_DestroyTSBuffer failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
    /** Unbind demux from the port */
    s32Ret = MT_UNF_DMX_DetachTSPort(DMXID);
    if(MT_SUCCESS != s32Ret)
    {

        SAMPLE_BOX_ERR_PRINT("MT_UNF_DMX_DetachTSPort failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    /** Deinitializes the DEMUX module */
    s32Ret = MT_UNF_DMX_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MT_UNF_DMX_DeInit failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
    return;
}


/*!
@brief Switch the aspect ratio each time you enter this function
@param[in]  hWin                Handle to window
@return::void
@*/
static mt_void BOX_SetAspectRatio(mt_handle hWin)
{
    MT_UNF_WINDOW_ATTR_S getWindowAttr = { 0 };
    MT_UNF_DISP_ASPECT_RATIO_S setDispAspectRatio = { 0 };
    MT_UNF_DISP_ASPECT_RATIO_S getDispAspectRatio = { 0 };
    static mt_u8 s_status = 0;

    switch(s_status)
    {
        case 0:
            SAMPLE_BOX_INFO_PRINT("Set to 16TO9 mode\n");
            setDispAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_16TO9;
            MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &setDispAspectRatio);
            s_status++;
            break;
        case 1:
            SAMPLE_BOX_INFO_PRINT("Set to 4TO3 mode\n");
            setDispAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_4TO3;
            MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &setDispAspectRatio);

            MT_UNF_VO_GetWindowAttr(hWin, &getWindowAttr);
            getWindowAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_LETTERBOX;
            MT_UNF_VO_SetWindowAttr(hWin, &getWindowAttr);
            s_status++;
            break;
        case 2:
            SAMPLE_BOX_INFO_PRINT("Set to 1TO1 mode\n");
            setDispAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_1TO1;
            MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &setDispAspectRatio);

            MT_UNF_VO_GetWindowAttr(hWin, &getWindowAttr);
            getWindowAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_PAN_SCAN;
            MT_UNF_VO_SetWindowAttr(hWin, &getWindowAttr);
            s_status++;
            break;
        case 3:
            SAMPLE_BOX_INFO_PRINT("Set to adaptive mode\n");
            setDispAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_AUTO;
            MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &setDispAspectRatio);

            MT_UNF_VO_GetWindowAttr(hWin, &getWindowAttr);
            getWindowAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_PAN_SCAN;
            MT_UNF_VO_SetWindowAttr(hWin, &getWindowAttr);
            s_status = 0;
            break;
        default:
            s_status = 0;
            break;
    }

    MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY1, &getDispAspectRatio);
    SAMPLE_BOX_INFO_PRINT("enDispAspectRatio: %d\n", getDispAspectRatio.enDispAspectRatio);
}


/*!
@brief Help information
@param[in]  name     The executable name
@return::void
@*/
static mt_void show_help(char *name)
{
    MT_BOX_PRINT("%s[ options ]...\n"
               "Options:\n"
               " -f<file>     File path playback\n"
               "example: ./sample_box -f ./677-CC-12.ts\n",
               name);
}


/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@param[out] pstCmd          Gets the address of the file path
@return::void
@*/
static mt_void parseCmdParam(int argc, char *argv[], CMD_LINE_ST *pstCmd)
{
    int opt = 0;
    int tmpvalue = 0;
    while((opt = getopt(argc, argv, ":?hHf:")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (mt_void)show_help(argv[0]);
                exit(0);
            case 'f':
                strcpy(pstCmd->chFileName, argv[2]);
                break;
            default:
                (mt_void)show_help(argv[0]);
                exit(0);
                break;
        }
    }
}


mt_s32 main(mt_s32 argc, mt_char *argv[])
{
    mt_s32                 s32Ret = MT_SUCCESS;
    mt_u32                 u32ProgNum = 0;
    mt_char                inPutCmd[32] ={ 0 };
    mt_handle              hSoundTrack = 0;
    mt_handle              hWin = 0;
    mt_handle              hAvPlay = 0;
    pthread_t              stInjectTSThread = 0;
    CMD_LINE_ST            cmd = { 0 };
    PMT_COMPACT_TBL        *pProgTbl = MT_NULL;
    PMT_COMPACT_PROG       *pstCurrentProgInfo = MT_NULL;

    if(argc < 3)
    {
        (mt_void)show_help(argv[0]);
        exit(0);
    }

    (mt_void)parseCmdParam(argc, argv, &cmd);

    /** System initialization */
    s32Ret = mt_sys_init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to mt_sys_init\n");
        return s32Ret;
    }

    /** HDMI initialization */
    s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_NTSC);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to StartDmx\n");
        goto ERR0;
    }

    /** Display initialization */
    s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_NTSC);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to MTADP_Disp_Init\n");
        goto ERR1;
    }

    /** VO device initialization */
    s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to MTADP_Snd_Init\n");
        goto ERR2;
    }

    /** Sound module initialization */
    s32Ret = MTADP_Snd_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to MTADP_Snd_Init\n");
        goto ERR3;
    }

    /** DMX module initialization */
    s32Ret = BOX_DmxInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to StartDmx\n");
        goto ERR4;
    }

    /** Create a thread to read the TS stream file */
    pthread_create(&stInjectTSThread, NULL, (mt_void * (*)(mt_void *))InjectTsTask, cmd.chFileName);

    (mt_void)MTADP_Search_Init();

    /** Get the PMT table */
    while(MT_SUCCESS != MTADP_Search_GetAllPmt(DMXID, &pProgTbl))
    {
        SAMPLE_BOX_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
        MT_USLEEP(100000);
    }

    /** AVPLAY initialization */
    s32Ret = BOX_AvplayInit(&hAvPlay, &hWin, &hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to StartAVPlay\n");
        g_bTaskQuit = MT_TRUE;
        goto ERR5;
    }

    g_Avplay = hAvPlay;

    /** Select the program */
    pstCurrentProgInfo = pProgTbl->proginfo;

    /** Start playing the show */
    s32Ret = BOX_StarToPlay(hAvPlay, pstCurrentProgInfo);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("failed to BOX_StarToPlay\n");
        g_bTaskQuit = MT_TRUE;
        goto ERR6;
    }

    while(1)
    {
        MT_BOX_PRINT("\nPlease enter 1-%d to select the program\n", pProgTbl->prog_num);
        MT_BOX_PRINT("   input the 'q' to quit\n"
                     "   input the 'a' to switch the aspect ratio\n"
                      "CMD>> ");

        scanf("%s", inPutCmd);

        if(inPutCmd[0] == 'q')
        {
            g_bTaskQuit = MT_TRUE;
            break;
        }

        u32ProgNum = atoi(inPutCmd);

        if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
        {
            pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1) % pProgTbl->prog_num);

            /** Stop AV playback into the stop state */
            s32Ret = BOX_StopToPlay(hAvPlay);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_BOX_ERR_PRINT("DVB_StopToPlay failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
                g_bTaskQuit = MT_TRUE;
                goto ERR6;
            }
            MT_BOX_PRINT("===== Start play ProgNum: %d \n", u32ProgNum);

            /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
            s32Ret = BOX_StarToPlay(hAvPlay, pstCurrentProgInfo);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_BOX_ERR_PRINT("DVB_StarToPlay failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
                g_bTaskQuit = MT_TRUE;
                goto ERR7;
            }
        }
        else if(inPutCmd[0] == 'a')
        {
            /** Switch the aspect ratio each time you enter this function */
            (mt_void)BOX_SetAspectRatio(hWin);
        }
        else
        {
            MT_BOX_PRINT("\nprog_num must less %d\n", pProgTbl->prog_num);
        }
    }

    /** Wait for the thread to end */
    pthread_join(stInjectTSThread, NULL);

ERR7:
    /** Stop playing the show */
    s32Ret = BOX_StopToPlay(hAvPlay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("BOX_StopToPlay failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR6:
    /** Audio and video player deinitialization */
    s32Ret = BOX_AvplayDeInit(hAvPlay, hWin, hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("BOX_AvplayDeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR5:
    (mt_void)MTADP_Search_FreeAllPmt(pProgTbl);
    (mt_void)MTADP_Search_DeInit();

    /** Demux module deinitialization */
    (mt_void)BOX_DmxDeInit();
ERR4:
    /** Audio output is deinitialized */
    s32Ret = MTADP_Snd_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MTADP_Snd_DeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR3:
    /** VO device deinitialization */
    s32Ret = MTADP_VO_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MTADP_VO_DeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR2:
    /** Display deinitialization */
    s32Ret = MTADP_Disp_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MTADP_Disp_DeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR1:
    /** HDMI deinitialization */
    s32Ret = MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_BOX_ERR_PRINT("MTADP_HDMI_DeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR0:
    /** system deinitialized */
    (mt_void)mt_sys_deinit();

    return s32Ret;
}
