/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_UDPPLAY_DEBUG
#define MT_UDPPLAY_PRINT   printf
#else
#define MT_UDPPLAY_PRINT
#endif

#define SAMPLE_UDPPLAY_FUNCTION_ENTER()               MT_UDPPLAY_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_UDPPLAY_FUNCTION_EXIT()                MT_UDPPLAY_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_UDPPLAY_FATAL_PRINT(fmt...)            MT_UDPPLAY_PRINT(" [FATAL] " fmt)
#define SAMPLE_UDPPLAY_ERR_PRINT(fmt...)              MT_UDPPLAY_PRINT(" [ERROR] " fmt)
#define SAMPLE_UDPPLAY_WARN_PRINT(fmt...)             MT_UDPPLAY_PRINT(" [WARN] "  fmt)
#define SAMPLE_UDPPLAY_INFO_PRINT(fmt...)             MT_UDPPLAY_PRINT(" [INFO] "  fmt)
#define SAMPLE_UDPPLAY_DBG_PRINT(fmt...)              MT_UDPPLAY_PRINT(" [DEBUG] " fmt)

#define SAMPLE_UDPPLAY_PRINT   printf

#define DMX_ID_0            0
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef struct tagCmdLinePara
{
    MT_U16   u16Port;
    MT_CHAR  chMcastIP[20];
}CMD_LINE_ST;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
} MT_UDPLAY_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
static MT_UDPLAY_RUN_INFO udplay_run_info;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_UdPlayMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*!
@brief The thread that receives UDP data
@param[in] args                 Multicast IP and local IP
@return::MT_VOID
@*/
static MT_VOID MT_UdpInjectTsTask(MT_VOID *args)
{
    MT_S32               s32Ret = MT_FAILURE;
    MT_HANDLE            hTsBuffer = MT_INVALID_HANDLE;
    MT_S32               SocketFd = 0;
    MT_S32               max_buf_len = 0;
    MT_U32               ReadLen = 0;
    MT_U32               ReceiveCount = 0;
    MT_U32               AddrLen = 0;
    MT_U32               datalen = 0;
    struct ip_mreq       Mreq = { 0 };
    struct sockaddr_in   ServerAddr = { 0 };
    MT_UNF_STREAM_BUF_S  StreamBuf = { 0 };
    CMD_LINE_ST          *cmd = (CMD_LINE_ST *)args;

    SocketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(SocketFd < 0)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("Create socket error.\n");
        g_bTaskQuit = MT_TRUE;
        return;
    }

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(cmd->chMcastIP);
    ServerAddr.sin_port = htons(cmd->u16Port);
    if(bind(SocketFd, (struct sockaddr *)(&ServerAddr), sizeof(struct sockaddr_in)) < 0)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("Socket bind error.\n");
        close(SocketFd);
        g_bTaskQuit = MT_TRUE;
        return;
    }

    Mreq.imr_multiaddr.s_addr = inet_addr(cmd->chMcastIP);
    Mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(SocketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &Mreq, sizeof(struct ip_mreq)))
    {
        SAMPLE_UDPPLAY_ERR_PRINT("Failed to join multicast.\n");
        close(SocketFd);
        g_bTaskQuit = MT_TRUE;
        return;
    }

    AddrLen = sizeof(ServerAddr);
    max_buf_len = 188*448;

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        close(SocketFd);
        return ;
    }

    s32Ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
        close(SocketFd);
        return ;
    }

    while(!g_bTaskQuit)
    {
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*500, &StreamBuf, 0);
        if(MT_SUCCESS != s32Ret)
        {
            continue;
        }

        datalen = 0;
        while(datalen < max_buf_len)
        {
            /** Receives UDP multicast data and blocks if there is no data transmission */
            ReadLen = recvfrom(SocketFd, StreamBuf.pu8Data + datalen, StreamBuf.u32Size, 0, (struct sockaddr *)&ServerAddr, &AddrLen);
            if (ReadLen <= 0)
            {
                ReceiveCount++;
                if (ReceiveCount >= 50)
                {
                    SAMPLE_UDPPLAY_ERR_PRINT("########## TS come too slow or net error! #########\n");
                    ReceiveCount = 0;
                }
            }
            else
            {
                ReceiveCount=0;
                datalen += ReadLen;
                if(datalen >= max_buf_len)
                {
                    s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, datalen);
                    if (s32Ret != MT_SUCCESS)
                    {
                        SAMPLE_UDPPLAY_ERR_PRINT("call MT_UNF_DMX_PutTSBuffer failed.\n");
                    }
                }
            }
        }
    }

    close(SocketFd);

    (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
    return;
}


/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error
@*/
static MT_S32 MT_UdPlaySetAvplayPidAndCodecType(MT_HANDLE hAvplay      , const PMT_COMPACT_PROG *pProgInfo)
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
        SAMPLE_UDPPLAY_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
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

    SAMPLE_UDPPLAY_INFO_PRINT("VidPid = %#x, AudPid = %#x-------<%s> line: %d\n", VidPid, AudPid, __FUNCTION__, __LINE__);

    if(VidPid != INVALID_TSPID)
    {
        /** Get the video properties of the AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_UDPPLAY_ERR_PRINT("Set video properties or video PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_UDPPLAY_ERR_PRINT("Setting the decoding mode or audio PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
static MT_S32 MT_UdPlayStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        s32Ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    /** Set the PID of the AV player and set the encoder type */
    MT_UdPlaySetAvplayPidAndCodecType(hAvplay, pProgInfo);

    /** Get the audio PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_UDPPLAY_ERR_PRINT("Has no audio stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    /** Get the video PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_UDPPLAY_ERR_PRINT("Has no video stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_UDPPLAY_ERR_PRINT("Set frame to VO is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Get synchronization properties of AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("Get avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_UDPPLAY_ERR_PRINT("Set avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    s32Ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_Start failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
static MT_S32 MT_UdPlayStopToPlay(MT_HANDLE hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;
    SAMPLE_UDPPLAY_INFO_PRINT("stop live play ...\n");
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
static MT_S32 MT_UdPlayAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   s32Ret = MT_FAILURE;
    MT_HANDLE                hAvplay = 0;
    MT_HANDLE                hWin = 0;
    MT_HANDLE                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Audio decoder */
    s32Ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** AV player initialization */
    s32Ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    s32Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Create AV player based on attributes */
    s32Ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_Create failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    s32Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    s32Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_SND_CreateTrack failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    s32Ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_SND_Attach failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR5;
    }

    /** Create a window */
    s32Ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MTADP_VO_CreatWin failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    s32Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_VO_AttachWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR7;
    }

    /** Enable/disable windows */
    s32Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
static MT_VOID MT_UdPlayAvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
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
static MT_S32 MT_UdPlayDmxInit(MT_VOID)
{
    MT_S32           s32Ret = MT_FAILURE;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        (MT_VOID)MT_UNF_DMX_DeInit();
        return s32Ret;
    }

    s32Ret = MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_RAM_0);
    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_UdPlayDmxDeInit(MT_VOID)
{
    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();
}


static MT_VOID MT_UdPlayExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

    /** Stop playing the show */
    (MT_VOID)MT_UdPlayStopToPlay(udplay_run_info.hAvPlay);

    /** Audio and video player deinitialization */
    (MT_VOID)MT_UdPlayAvplayDeInit(udplay_run_info.hAvPlay, udplay_run_info.hWin, udplay_run_info.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(udplay_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    /** Wait for the thread to end */
    pthread_join(udplay_run_info.stInjectTSThread, NULL);


    (MT_VOID)MT_UdPlayDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    memset(&udplay_run_info, 0xff, sizeof(udplay_run_info));
}


static MT_VOID MT_UdPlayPrintMenu(MT_U32 prog_num)
{
    SAMPLE_UDPPLAY_PRINT("\n");
    SAMPLE_UDPPLAY_PRINT(" 1 - %d : select the program \n", prog_num);
#ifdef MT_SAMPLE_APP
    SAMPLE_UDPPLAY_PRINT("     b : background run \n");
#endif
    SAMPLE_UDPPLAY_PRINT("     h : help \n");
    SAMPLE_UDPPLAY_PRINT("     q : quit \n");
    SAMPLE_UDPPLAY_PRINT("UDPLAY>> ");

}


/*!
@brief The thread on which the command was entered
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgTbl            The data structure of the PMT
@return::MT_VOID
@*/
static MT_VOID MT_UdPlayCmdTask(MT_HANDLE hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_S32                 s32Ret = MT_SUCCESS;
    MT_U32                 u32ProgNum = 0;
    MT_CHAR                inputCmd[32] ={ 0 };
    PMT_COMPACT_PROG       *pstCurrentProgInfo = MT_NULL;

    if(MT_INVALID_HANDLE == hAvplay || NULL == pProgTbl)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    while(1)
    {
         (MT_VOID)MT_UdPlayPrintMenu(pProgTbl->prog_num);

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if(inputCmd[0] == 'q')
        {
            SAMPLE_UDPPLAY_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_UDPPLAY_INFO_PRINT("tplay play in back!\n");
            break;
        }
    #endif
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);

            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1) % pProgTbl->prog_num);

                /** Stop AV playback into the stop state */
                s32Ret = MT_UdPlayStopToPlay(hAvplay);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_UDPPLAY_ERR_PRINT("MT_UdpplayModeStopToPlay failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
                }
                SAMPLE_UDPPLAY_INFO_PRINT("===== Start play ProgNum: %d \n", u32ProgNum);

                /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
                s32Ret = MT_UdPlayStarToPlay(hAvplay, pstCurrentProgInfo);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_UDPPLAY_ERR_PRINT("Switching shows failed\n");
                }
            }
            else
            {
                SAMPLE_UDPPLAY_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
                continue;
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_UDPPLAY_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}


/*!
@brief Help information
@param[in]  name     The executable name
@return::MT_VOID
@*/
static MT_VOID MT_UdPlayPrint_Help(MT_CHAR *name)
{
    MT_UDPPLAY_PRINT("Lack of parameters\n");
    MT_UDPPLAY_PRINT("\nUsage:\n");
    MT_UDPPLAY_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    MT_UDPPLAY_PRINT("    -q: Exit the background\n");
#endif
    MT_UDPPLAY_PRINT("    -m: Multicast IP\n");
    MT_UDPPLAY_PRINT("    -p: IP port\n");
    MT_UDPPLAY_PRINT("example:\n");
    MT_UDPPLAY_PRINT("    %s -m 224.1.1.1 -p 8080\n", name);
}


/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@param[out] pstCmd          UDP connection parameters
@return::MT_VOID
@*/
static MT_S32 MT_UdPlayParase_args(MT_S32 argc, MT_CHAR *argv[], CMD_LINE_ST *pstCmd)
{
    MT_S32 opt = 0;
    MT_S32 tmpvalue = 0;

    if(argc != 5 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_UdPlayPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, "?hHm:p:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_UdPlayPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'm':
                strcpy(pstCmd->chMcastIP, argv[2]);
                break;
            case 'p':
                tmpvalue = strtol(mt_optarg, 0, 0);
                pstCmd->u16Port = (MT_U16)tmpvalue;
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_UdPlayExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_UdPlayPrint_Help(argv[0]);
                return MT_FAILURE;
                break;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_UdPlayMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_U8                  count = 0;
    MT_S32                 s32Ret = MT_SUCCESS;
    CMD_LINE_ST            cmd = { 0 };
    PMT_COMPACT_PROG       *pstCurrentProgInfo = MT_NULL;

    s32Ret = MT_UdPlayParase_args(argc, argv, &cmd);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }


    if(g_bTaskQuit == MT_TRUE)
    {
    #ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }

        /** HDMI initialization */
        s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to StartDmx\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }
    #endif

        g_bTaskQuit = MT_FALSE;

        /** VO device initialization */
        s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR2;
        }

        /** Sound module initialization */
        s32Ret = MTADP_Snd_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR3;
        }

        /** DMX module initialization */
        s32Ret = MT_UdPlayDmxInit();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to MT_UdPlayDmxInit\n");
            goto ERR4;
        }

        /** Create a thread to read the TS stream file */
        s32Ret = pthread_create(&udplay_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_UdpInjectTsTask, &cmd);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to pthread_create\n");
            goto ERR5;
        }

        sleep(1);
        if(MT_FALSE != g_bTaskQuit)
        {
            goto ERR6;
        }

        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        while(MT_SUCCESS != MTADP_Search_GetAllPmt(DMX_ID_0, &udplay_run_info.pProgTbl))
        {
            count++;
            SAMPLE_UDPPLAY_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            MT_USLEEP(100000);
            if(20 == count)
            {
                goto ERR7;
            }
        }

        /** AVPLAY initialization */
        s32Ret = MT_UdPlayAvplayInit(&udplay_run_info.hAvPlay, &udplay_run_info.hWin, &udplay_run_info.hSoundTrack);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to MT_UdPlayAvplayInit\n");
            goto ERR8;
        }

        /** Select the program */
        pstCurrentProgInfo = udplay_run_info.pProgTbl->proginfo;

        /** Start playing the show */
        s32Ret = MT_UdPlayStarToPlay(udplay_run_info.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_UDPPLAY_ERR_PRINT("failed to MT_UdPlayStarToPlay\n");
            goto ERR9;
        }
    }

    (MT_VOID)MT_UdPlayCmdTask(udplay_run_info.hAvPlay, udplay_run_info.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    /** Stop playing the show */
    s32Ret = MT_UdPlayStopToPlay(udplay_run_info.hAvPlay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_UDPPLAY_ERR_PRINT("MT_UdPlayStopToPlay failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

ERR9:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_UdPlayAvplayDeInit(udplay_run_info.hAvPlay, udplay_run_info.hWin, udplay_run_info.hSoundTrack);
ERR8:
    (MT_VOID)MTADP_Search_FreeAllPmt(udplay_run_info.pProgTbl);
ERR7:
    (MT_VOID)MTADP_Search_DeInit();
ERR6:
    g_bTaskQuit = MT_TRUE;
    /** Wait for the thread to end */
    pthread_join(udplay_run_info.stInjectTSThread, NULL);
ERR5:
    /** Demux module deinitialization */
    (MT_VOID)MT_UdPlayDmxDeInit();
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
    memset(&udplay_run_info, 0xff, sizeof(udplay_run_info));

    return s32Ret;
}
