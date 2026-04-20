/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_demux.h"
#include "mt_adp_pvr.h"
#include "mt_cmdline.h"


/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_PVRREC_DEBUG

#define MT_PVRREC_PRINT   printf
#else

#define MT_PVRREC_PRINT

#endif

#define SAMPLE_PVRREC_FUNCTION_ENTER()      MT_PVRREC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_PVRREC_FUNCTION_EXIT()       MT_PVRREC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_PVRREC_FATAL_PRINT(fmt...)       MT_PVRREC_PRINT(" [FATAL] " fmt)
#define SAMPLE_PVRREC_ERR_PRINT(fmt...)         MT_PVRREC_PRINT(" [ERROR] " fmt)
#define SAMPLE_PVRREC_WARN_PRINT(fmt...)        MT_PVRREC_PRINT(" [WARN] "  fmt)
#define SAMPLE_PVRREC_INFO_PRINT(fmt...)        MT_PVRREC_PRINT(" [INFO] "  fmt)
#define SAMPLE_PVRREC_DBG_PRINT(fmt...)         MT_PVRREC_PRINT(" [DEBUG] " fmt)

#define SAMPLE_PVRREC_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0 0
#define TUNER_ID_0 (0)
#define MAX_REC_FILE_SIZE                 (0) /* not set max size*/
#define TUNER_REC_USE   0
#define PVR_DMX_ID_REC  0


/*************************** Structure Definition ****************************/
typedef enum input_sig_type_t {
    MT_INPUT_SIG_TYPE_CAB = 1,
    /**<Cable signal*/ /**<CNcomment:DVB_C信号*/
    MT_INPUT_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/ /**<CNcomment:卫星信号*/
    MT_INPUT_SIG_TYPE_DVB_T = 3,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_INPUT_SIG_TYPE_FILE = 4,
    /**<local file */ /**<CNcomment:本地文件*/
}MT_INPUR_SIG_TYPE_T;

typedef struct
{
    mt_u32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    mt_u32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
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
    mt_u32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    mt_u32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
    mt_u8 port_type;
} mt_input_ter_para_t;


typedef struct tagInput_Param_T
{
    mt_u8 file_name[256];
}mt_rec_file_para_t;



typedef struct
{
    mt_s32 tuner_id;
    MT_INPUR_SIG_TYPE_T sig_type;
    mt_rec_file_para_t folder;
    union
    {
        mt_input_cab_para_t cab;
        mt_input_ter_para_t ter;
        mt_input_sat_para_t sat;
    } input_param;

} mt_input_para_t;

typedef struct
{
    mt_input_para_t sInputParam;
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    pthread_t          statusThd;
    PMT_COMPACT_TBL *pProgTbl;
    MT_U32  recChn1;
    MT_U32  recChn2;
    MT_U32  recChn3;
    MT_BOOL bIsRecStop;
} MT_PVRRec_RUN_INFO;


/********************** Global Variable declaration **************************/

static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_PVRRec_RUN_INFO    g_stPVRRecRunInfo;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

static mt_s32 MT_PVRRecCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_PVRREC_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_PVRREC_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_PVRREC_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_PVRRecCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_PVRREC_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_PVRREC_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_PVRRecDmxInit(MT_INPUR_SIG_TYPE_T sig_type, mt_s32 tuner_id)
{
    MT_S32   Ret = 0;
    mt_sys_version_s stSysChipInfo = { 0 };

    SAMPLE_PVRREC_FUNCTION_ENTER();

    Ret = MT_UNF_DMX_Init();
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_PVRREC_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return Ret;
    }

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    Ret = mt_sys_get_version(&stSysChipInfo);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_PVRREC_ERR_PRINT("failed to mt_sys_get_version\n");
        return MT_FAILURE;
    }

    if(MT_INPUT_SIG_TYPE_CAB == sig_type)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
        }
        else
        {
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_0);
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_0);
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_0);
        }
    }
    else if(MT_INPUT_SIG_TYPE_SAT == sig_type)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            if(tuner_id == 0)
            {
                MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, DMX_DVB_TSI_IN_PORT);
                MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, DMX_DVB_TSI_IN_PORT);
                MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, DMX_DVB_TSI_IN_PORT);
            }
            else
            {
                MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
                MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
                MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
            }

        }
        else
        {
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
        }
    }


    SAMPLE_PVRREC_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_VOID MT_PVRRecDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC+2);

    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_LIVE);

    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC);

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
static mt_s32 MT_PVRRecAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_PVRREC_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_PVRREC_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_PVRREC_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = PVR_DMX_ID_LIVE;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
static MT_VOID  MT_PVRRecAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_PVRREC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_PVRREC_ERR_PRINT("=====hWin is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_PVRREC_ERR_PRINT("=====hSoundTrack is INVALID_HANDLE ======\n");
        return ;
    }
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
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_PVRRecAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
{
    mt_u32 VidPid = 0;
    mt_u32 AudPid = 0;
    mt_u32 u32AudType = 0;
    mt_s32 ret = 0;
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0X00;

    if(NULL == p_ProgInfo)
    {
        SAMPLE_PVRREC_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }

    if(p_ProgInfo->VElementNum > 0 )
    {
        VidPid = p_ProgInfo->VElementPid;
        enVidType = p_ProgInfo->VideoType;
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if(p_ProgInfo->AElementNum > 0)
    {
        AudPid  = p_ProgInfo->AElementPid;
        u32AudType = p_ProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = MT_INVALID_HANDLE;
    }

    SAMPLE_PVRREC_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
            return ret;
        }
        /* The type of code stream supported by the decoder*/
        VcodecAttr.enType = enVidType;
        VcodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VcodecAttr.u32ErrCover = 100;
        VcodecAttr.u32Priority = 3;
        VcodecAttr.u32UseDescInfoFlag = 1;

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_PVRREC_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_PVRREC_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_PVRREC_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_PVRREC_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
        return ret;
    }


    //dmx sync
    if((INVALID_TSPID != VidPid) || (INVALID_TSPID != AudPid))
    {
        MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync = { 0 };
        memset(&DmxAvsync, 0, sizeof(DmxAvsync));
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay,MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC,(mt_void *)&DmxAvsync);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_PVRREC_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
            return ret;
        }
    }


    //av sync
    if((INVALID_TSPID != VidPid) || (INVALID_TSPID != AudPid))
    {
        MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
            return ret;
        }
        SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        SyncAttr.stSyncStartRegion.s32VidPlusTime = 20;
        SyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        SyncAttr.bQuickOutput = MT_TRUE;
        SyncAttr.stSyncStartRegion.bSmoothPlay  = MT_TRUE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
            return ret;
        }
    }

    return MT_SUCCESS;
}


/*
@brief stop to play
@param[in] avplay, Player handle
@return MT_SUCCESS
*/
static mt_s32 MT_PVRRecStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if(MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_PVRREC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}

/*!
@brief Help information
@param[in]  name     Enter the value
@return::void
@*/
static void MT_PVRRecPrint_help(char *name)
{

       MT_PVRREC_PRINT("Lack of parameters\n");
       MT_PVRREC_PRINT("\nUsage:\n");
       MT_PVRREC_PRINT(" %s -f folder -f ... \n", name);
#ifndef MT_SAMPLE_APP
       MT_PVRREC_PRINT("    -t: tuner_id \n");
       MT_PVRREC_PRINT("    -c: DVBC \n");
       MT_PVRREC_PRINT("        -c freq symrate qam\n");
       MT_PVRREC_PRINT("    -s: DVBS \n");
       MT_PVRREC_PRINT("        -s freq srate 22k sig_type polar type\n");
       MT_PVRREC_PRINT("example:\n");
       MT_PVRREC_PRINT("    %s -f ./pvr -t 1 -c 654 6875 64\n", name);
       MT_PVRREC_PRINT("    %s -f ./pvr -t 0 -s 3840 27500 1 0 0\n", name);
#else
       MT_PVRREC_PRINT("    %s -f ./pvr \n", name);
       MT_PVRREC_PRINT("    %s -q  <exit> \n", name);
#endif
}

static MT_VOID MT_PVRRecPrintMenu(MT_U32 prog_num)
{
    SAMPLE_PVRREC_PRINT("\n ");
    SAMPLE_PVRREC_PRINT(" 1 - %d : select the program to play\n", prog_num);
    SAMPLE_PVRREC_PRINT("    w0 : start record0 \n");
    SAMPLE_PVRREC_PRINT("    w1 : start record1 \n");
    SAMPLE_PVRREC_PRINT("    w2 : start record2 \n");
    SAMPLE_PVRREC_PRINT("    t0 : stop record0 \n");
    SAMPLE_PVRREC_PRINT("    t1 : stop record1 \n");
    SAMPLE_PVRREC_PRINT("    t2 : stop record2 \n");
    SAMPLE_PVRREC_PRINT("     i : Whether to record ttx/sub/cc/track(0 is close, 1 is open) \n");
    SAMPLE_PVRREC_PRINT("     y : set dio open/close(0 is close, 1 is open) \n");
    SAMPLE_PVRREC_PRINT("     m : Set whether to encrypt recording \n");
    SAMPLE_PVRREC_PRINT("     a : All stream recording \n");
    SAMPLE_PVRREC_PRINT("     d : Stop all stream recording \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_PVRREC_PRINT("     b : background run \n");
    SAMPLE_PVRREC_PRINT("     z : background run but save rec(only to tset)\n");
    SAMPLE_PVRREC_PRINT("     u : replay (only to tset)\n");
#endif
    SAMPLE_PVRREC_PRINT("     h : help \n");
    SAMPLE_PVRREC_PRINT("     q : quit \n");
    SAMPLE_PVRREC_PRINT("REC>> ");

}

static MT_S32 MT_PVRRecCheckkey(mt_pvr_rec_cipher_t rec_cipher)
{
    mt_s32 i = 0;
    mt_s32 j = 8;
    mt_s32 conunt = 0;
    for(i = 0; i < rec_cipher.u32KeyLen/2; i++)
    {
        if(rec_cipher.au8Key[i] == rec_cipher.au8Key[j])
        {
            conunt++;
        }
        j++;

    }
    if(conunt == 8)
    {
        SAMPLE_PVRREC_ERR_PRINT("the same key(The first 8b can't be the same as the last 8b) \n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static MT_S32 MT_PVRRecAllTsStart(char *path, MT_U32 u32DemuxID, MT_U32 freq, MT_U64 maxSize, MT_U32 *pRecChn, MT_BOOL bdio)
{
    MT_U32 recChn;
    MT_S32 ret = MT_SUCCESS;
    MT_UNF_PVR_REC_ATTR_S   attr = {0};
    PVR_PROG_INFO_S    fileInfo = {0};
    mt_s32 i = 0;
    MT_CHAR    szFileName[PVR_MAX_FILENAME_LEN];

    sprintf(szFileName, "rec_freq_%d.ts", freq);
    sprintf(attr.szFileName, "%s/", path);
    strcat(attr.szFileName, szFileName);
    printf("record file name:%s\n", attr.szFileName);
    attr.u32FileNameLen = strlen(attr.szFileName);
    attr.u32DemuxID    = u32DemuxID;
    attr.u32ScdBufSize = PVR_STUB_SC_BUF_SZIE;
    attr.u32DavBufSize = PVR_STUB_TSDATA_SIZE;
    attr.enStreamType  = MT_UNF_PVR_STREAM_TYPE_ALL_TS;
    attr.bRewind = MT_FALSE;
    attr.u64MaxFileSize= maxSize;//source;
    attr.bIsClearStream = MT_TRUE;
    attr.u32UsrDataInfoSize = 0;
    attr.stEncryptCfg.bDoCipher = MT_FALSE;
    attr.u32IndexPid   = 0x1fff;
    attr.enIndexType   = MT_UNF_PVR_REC_INDEX_TYPE_NONE;
    attr.enIndexVidType = MT_UNF_VCODEC_TYPE_MPEG2;
    attr.u32DIO = bdio;


    ret = MT_UNF_PVR_RecCreateChn(&recChn, &attr);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }
    if(attr.stEncryptCfg.bDoCipher)
    {
        MT_UNF_PVR_RegisterExtraCallback(recChn, MT_UNF_PVR_EXTRA_WRITE_CALLBACK, (ExtraCallBack)MTADP_PVR_Crypto_WriteCallback, NULL);
    }
    else
    {
        MT_UNF_PVR_RegisterExtraCallback(recChn, MT_UNF_PVR_EXTRA_WRITE_CALLBACK, (ExtraCallBack)MTADP_PVR_Normal_WriteCallback, NULL);
    }
    ret = MT_UNF_PVR_RecStartChn(recChn);
    if (MT_SUCCESS != ret)
    {
        MT_UNF_PVR_RecDestroyChn(recChn);
        return ret;
    }

    memcpy(&(fileInfo.stRecAttr), &attr, sizeof(fileInfo.stRecAttr));
    MTADP_PVR_SavePorgInfo(&fileInfo, attr.szFileName);
    *pRecChn = recChn;

    return MT_SUCCESS;
}

static void *MT_PVRRecStatus(MT_VOID *args)
{
    MT_S32 Ret = MT_SUCCESS;
    MT_UNF_PVR_REC_STATUS_S stRecStatus1 = { 0 };
    MT_UNF_PVR_REC_STATUS_S stRecStatus2 = { 0 };
    MT_UNF_PVR_REC_STATUS_S stRecStatus3 = { 0 };
    MT_UNF_PVR_REC_ATTR_S stRecAttr1 = { 0 };
    MT_UNF_PVR_REC_ATTR_S stRecAttr2 = { 0 };
    MT_UNF_PVR_FILE_ATTR_S FileStatus = { 0 };

    while(MT_FALSE == g_stPVRRecRunInfo.bIsRecStop)
    {
        memset(&stRecStatus1, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        memset(&stRecStatus2, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        memset(&stRecStatus3, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        sleep(1);
        if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1)
        {
            (void)MT_UNF_PVR_RecGetChn(g_stPVRRecRunInfo.recChn1, &stRecAttr1);
            Ret = MT_UNF_PVR_RecGetStatus(g_stPVRRecRunInfo.recChn1, &stRecStatus1);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_PVR_RecGetStatus failed.\n");
            }

            Ret = MT_UNF_PVR_GetFileAttrByFileName(stRecAttr1.szFileName, &FileStatus);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_PVR_GetFileAttrByFileName failed.\n");
            }

        }
        if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2)
        {
            (void)MT_UNF_PVR_RecGetChn(g_stPVRRecRunInfo.recChn2, &stRecAttr2);
            Ret = MT_UNF_PVR_RecGetStatus(g_stPVRRecRunInfo.recChn2, &stRecStatus2);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_PVR_RecGetStatus failed.\n");
            }

            Ret = MT_UNF_PVR_GetFileAttrByFileName(stRecAttr2.szFileName, &FileStatus);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_PVR_GetFileAttrByFileName failed.\n");
            }

        }
        if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3)
        {
            Ret = MT_UNF_PVR_RecGetStatus(g_stPVRRecRunInfo.recChn3, &stRecStatus3);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_PVR_RecGetStatus failed.\n");
            }


        }

        if((MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1)  || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3))
        {
            SAMPLE_PVRREC_INFO_PRINT("###rec1=%d,rec2=%d, rec3all=%d\n",(stRecStatus1.u32EndTimeInMs-stRecStatus1.u32StartTimeInMs)/1000,
                                                (stRecStatus2.u32EndTimeInMs-stRecStatus2.u32StartTimeInMs)/1000, stRecStatus3.u32CurTimeInMs/1000);
        }

    }

    return MT_SUCCESS;

}

static MT_VOID MT_PVRRecDestory(MT_VOID)
{

    play_resource.demux_use = MT_TRUE;

    (MT_VOID)MT_PVRRecStopplay(g_stPVRRecRunInfo.hAvPlay);

    (MT_VOID)MT_PVRRecAvplayDeInit(g_stPVRRecRunInfo.hAvPlay, g_stPVRRecRunInfo.hWin, g_stPVRRecRunInfo.hSoundTrack);

}

static MT_VOID MT_PVRRecReplay(MT_VOID)
{
    mt_s32 ret = 0;

    ret = MT_PVRRecAVplayInit(&g_stPVRRecRunInfo.hAvPlay, &g_stPVRRecRunInfo.hWin, &g_stPVRRecRunInfo.hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT("failed to MT_PVRRecAVplayInit\n");
    }
    ret = MT_PVRRecAVPlay_Start(g_stPVRRecRunInfo.hAvPlay, g_stPVRRecRunInfo.pProgTbl->proginfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PVRREC_ERR_PRINT("call PVR_RecStart failed.\n");
    }
}


/*!
@brief Thread exit
@param[in]  argc            The number of external input parameters
@return MT_FAILURE
@return MT_SUCCESS
@*/
static MT_S32  MT_PVRRecCmdTask(mt_handle hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_CHAR inputCmd[32]={0};
    char *fgetret=NULL;

    mt_u32   u32ProgNum = 1;
#ifdef MT_SAMPLE_APP
    u32ProgNum = play_resource.s32ProgNum;
#endif
    PMT_COMPACT_PROG *stCurrentProgInfo = pProgTbl->proginfo + (u32ProgNum -1);
    MT_S32   ret = MT_FAILURE;
    MT_BOOL  bDoCipher = MT_FALSE;
    MT_BOOL  bdio = MT_TRUE;
    MT_BOOL  binfo = MT_FALSE;
    MT_S32    rec1_num = 0;
    MT_S32    rec2_num = 0;
    MT_S32    rec3_num = 0;
    mt_pvr_rec_cipher_t rec_cipher = { 0 };
    MT_BOOL rec_all = MT_FALSE;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_PVRREC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }


    while (1)
    {
        (void)MT_PVRRecPrintMenu(pProgTbl->prog_num);

        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret=fgetret;
        if ('q' == inputCmd[0])
        {
            SAMPLE_PVRREC_INFO_PRINT("now exit!\n");
            if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1)
            {
                (void)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn1);
                g_stPVRRecRunInfo.recChn1 = MT_INVALID_HANDLE;
            }
            if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2)
            {
                (void)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn2);
                g_stPVRRecRunInfo.recChn2 = MT_INVALID_HANDLE;
            }
            if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3)
            {
                (void)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn3);
                g_stPVRRecRunInfo.recChn3 = MT_INVALID_HANDLE;
            }
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_PVRREC_INFO_PRINT("PVR_Rec in back!\n");
            break;
        }
        else if ('z' == inputCmd[0])
        {
            SAMPLE_PVRREC_INFO_PRINT("PVR_Rec in back save rec!\n");
            MT_PVRRecDestory();
            break;
        }
        else if ('u' == inputCmd[0])
        {
            (MT_VOID)MT_PVRRecReplay();
        }
#endif
        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                stCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                ret = MT_PVRRecStopplay(hAvplay);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PVRREC_ERR_PRINT(" MT_PVRRecStopplay failed.\n");
                }

                SAMPLE_PVRREC_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);

                ret = MT_PVRRecAVPlay_Start(hAvplay, stCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PVRREC_ERR_PRINT(" SwitchProg failed.\n");
                }
            }
            else
            {
                SAMPLE_PVRREC_INFO_PRINT("prog_num the biggest is %d\n", pProgTbl->prog_num);
                SAMPLE_PVRREC_INFO_PRINT("q: quit\n");
                continue;
            }
#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PVRREC_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
        else if('a' == inputCmd[0])
        {
            if((MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1))
            {
                SAMPLE_PVRREC_ERR_PRINT("Single program recording has been initiated \n");
                continue;
            }

            if(MT_INPUT_SIG_TYPE_SAT == g_stPVRRecRunInfo.sInputParam.sig_type)
            {

                ret = MT_PVRRecAllTsStart(g_stPVRRecRunInfo.sInputParam.folder.file_name, PVR_DMX_ID_REC, g_stPVRRecRunInfo.sInputParam.input_param.sat.freq, MAX_REC_FILE_SIZE, &g_stPVRRecRunInfo.recChn3, bdio);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PVRREC_ERR_PRINT(" MTADP_PVR_RecStart3 failed.\n");
                }
            }
            else
            {

                ret = MT_PVRRecAllTsStart(g_stPVRRecRunInfo.sInputParam.folder.file_name, PVR_DMX_ID_REC, g_stPVRRecRunInfo.sInputParam.input_param.cab.freq, MAX_REC_FILE_SIZE, &g_stPVRRecRunInfo.recChn3, bdio);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PVRREC_ERR_PRINT(" MTADP_PVR_RecStart3 failed.\n");
                }
            }
            rec_all = MT_TRUE;

        }
        else if('d' == inputCmd[0])
        {
             if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3)
             {
                 (MT_VOID)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn3);
                 g_stPVRRecRunInfo.recChn3 = MT_INVALID_HANDLE;
                 SAMPLE_PVRREC_INFO_PRINT("++stop rec 3 all\n");
                 rec_all = MT_FALSE;
              }
        }

        else if('w' == inputCmd[0])
        {
            if(rec_all != MT_FALSE)
            {
                SAMPLE_PVRREC_ERR_PRINT("Full-stream recording has been initiated \n");
                continue;
            }
            if('0' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1)
                {
                    SAMPLE_PVRREC_ERR_PRINT("rec 1 is start \n");
                    continue;
                }
                rec1_num = u32ProgNum;
                if((rec2_num == rec1_num) || (rec3_num == rec1_num))
                {
                    SAMPLE_PVRREC_ERR_PRINT("The same program \n");
                    rec1_num = 0;
                    continue;
                }
                ret = MTADP_PVR_RecStart(g_stPVRRecRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_LIVE, 0, bDoCipher, MAX_REC_FILE_SIZE, &g_stPVRRecRunInfo.recChn1, bdio, rec_cipher, binfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PVRREC_ERR_PRINT(" MTADP_PVR_RecStart1 failed.\n");
                }

            }
            else if('1' == inputCmd[1])
            {

                if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2)
                {
                    SAMPLE_PVRREC_ERR_PRINT("rec 2 is start \n");
                    continue;
                }
                rec2_num = u32ProgNum;

                if((rec2_num == rec1_num) || (rec2_num == rec3_num))
                {
                    SAMPLE_PVRREC_ERR_PRINT("The same program \n");
                    rec2_num = 0;
                    continue;
                }
                ret = MTADP_PVR_RecStart(g_stPVRRecRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC+2, 0, bDoCipher, MAX_REC_FILE_SIZE, &g_stPVRRecRunInfo.recChn2, bdio, rec_cipher, binfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PVRREC_ERR_PRINT(" MTADP_PVR_RecStart2 failed.\n");
                }
            }

             else if('2' == inputCmd[1])
             {
                if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3)
                {
                    SAMPLE_PVRREC_ERR_PRINT("rec 3 is start \n");
                    continue;
                }
                rec3_num = u32ProgNum;

                if((rec3_num == rec1_num) || (rec2_num == rec3_num))
                {
                    SAMPLE_PVRREC_ERR_PRINT("The same program \n");
                    rec3_num = 0;
                    continue;
                }
                ret = MTADP_PVR_RecStart(g_stPVRRecRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC, 0, bDoCipher, MAX_REC_FILE_SIZE, &g_stPVRRecRunInfo.recChn3, bdio, rec_cipher, binfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PVRREC_ERR_PRINT(" MTADP_PVR_RecStart2 failed.\n");
                }
             }

        }
        else if('i' == inputCmd[0])
        {

            if((MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1))
            {
                SAMPLE_PVRREC_ERR_PRINT("The program is recording. The binfo setting is invalid \n");
                continue;
            }
            if(binfo == MT_TRUE)
            {
                binfo = MT_FALSE;
                SAMPLE_PVRREC_INFO_PRINT("Close rec all info \n");
            }
            else
            {
                binfo = MT_TRUE;
                SAMPLE_PVRREC_INFO_PRINT("Open rec all info \n");
            }

        }

        else if('y' == inputCmd[0])
        {

            if((MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1))
            {
                SAMPLE_PVRREC_ERR_PRINT("The program is recording. The bDoCipher setting is invalid \n");
                continue;
            }
            if(bdio == MT_TRUE)
            {
                bdio = MT_FALSE;
                SAMPLE_PVRREC_INFO_PRINT("Close dio \n");
            }
            else
            {
                bdio = MT_TRUE;
                SAMPLE_PVRREC_INFO_PRINT("Open dio \n");
            }

        }
        else if('m' == inputCmd[0])
        {
            if((MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1))
            {
                SAMPLE_PVRREC_ERR_PRINT("The program is recording. The bDoCipher setting is invalid \n");
                continue;
            }
            if(bDoCipher > 0)
            {
                bDoCipher = 0;
                SAMPLE_PVRREC_INFO_PRINT("Close bDoCipher \n");
            }
            else
            {
                bDoCipher = 1;
                SAMPLE_PVRREC_INFO_PRINT("Open bDoCipher \n");
            }
            SAMPLE_PVRREC_INFO_PRINT("bDoCipher = %d\n", bDoCipher);
            if(0 == bDoCipher)
            {
                SAMPLE_PVRREC_INFO_PRINT("Please turn on the bDoCipher switch first \n");
                continue;
            }
            SAMPLE_PVRREC_INFO_PRINT("Whether to use your own keys and encryption , 0:MT_FALSE,1:MT_TRUE \n");
            fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
            rec_cipher.bkey = atoi(inputCmd);

            if(rec_cipher.bkey == MT_TRUE)
            {
                SAMPLE_PVRREC_INFO_PRINT("Please enter the encryption type \n");
                SAMPLE_PVRREC_INFO_PRINT("0:MT_CIPHER_ALG_DES 1:MT_CIPHER_ALG_TDES 2:MT_CIPHER_ALG_AES 3:MT_CIPHER_ALG_HMAC 4:MT_CIPHER_ALG_CSA2 5:MT_CIPHER_ALG_CSA3 \n");
                SAMPLE_PVRREC_INFO_PRINT("sym4 supports only the first three ciphers \n");
                scanf("%d", &rec_cipher.enType);
                if(rec_cipher.enType > 5)
                {
                    SAMPLE_PVRREC_ERR_PRINT("type err! \n");
                    memset(&rec_cipher, 0, sizeof(rec_cipher));
                    continue;
                }

                rec_cipher.u32KeyLen = 16;

                SAMPLE_PVRREC_INFO_PRINT("Please enter the encryption key(16b) \n");

                for(int i = 0; i<rec_cipher.u32KeyLen; i++)
                {
                    scanf("%d", &rec_cipher.au8Key[i]);
                }
                if(1 == rec_cipher.enType)
                {
                    ret = MT_PVRRecCheckkey(rec_cipher);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_PVRREC_ERR_PRINT("key err! \n");
                        memset(&rec_cipher, 0, sizeof(rec_cipher));
                    }
                }
            }


        }

        else if('t' == inputCmd[0])
        {

            if('0' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1)
                {
                    (MT_VOID)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn1);
                    g_stPVRRecRunInfo.recChn1 = MT_INVALID_HANDLE;
                    SAMPLE_PVRREC_INFO_PRINT("++stop rec 1\n");
                    rec1_num = 0;
                }
            }

            else if('1' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2)
                {
                    (MT_VOID)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn2);
                    g_stPVRRecRunInfo.recChn2 = MT_INVALID_HANDLE;
                    SAMPLE_PVRREC_INFO_PRINT("++stop rec 2\n");
                    rec2_num = 0;
                }
            }
            else if('2' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3)
                {
                    (MT_VOID)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn3);
                    g_stPVRRecRunInfo.recChn3 = MT_INVALID_HANDLE;
                    SAMPLE_PVRREC_INFO_PRINT("++stop rec 3\n");
                    rec3_num = 0;
                }
            }
        }

        else if('h' == inputCmd[0])
        {
            SAMPLE_PVRREC_INFO_PRINT("Print help info \n");
            continue;
        }



    }

    return MT_SUCCESS;
}
static MT_VOID MT_PVRRecExit(MT_VOID)
{
    if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn1)
    {
       (void)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn1);
        g_stPVRRecRunInfo.recChn1 = MT_INVALID_HANDLE;
    }
    if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn2)
    {
       (void)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn2);
        g_stPVRRecRunInfo.recChn2 = MT_INVALID_HANDLE;
    }
    if(MT_INVALID_HANDLE != g_stPVRRecRunInfo.recChn3)
    {
       (void)MTADP_PVR_RecStop(g_stPVRRecRunInfo.recChn3);
        g_stPVRRecRunInfo.recChn3 = MT_INVALID_HANDLE;
    }
    g_stPVRRecRunInfo.bIsRecStop = MT_TRUE;
    (MT_VOID)pthread_join(g_stPVRRecRunInfo.statusThd, MT_NULL);

    (MT_VOID)MT_PVRRecStopplay(g_stPVRRecRunInfo.hAvPlay);


    (MT_VOID)MT_UNF_PVR_PlayDeInit();


    (MT_VOID)MT_UNF_PVR_RecDeInit();


    (MT_VOID)MT_PVRRecAvplayDeInit(g_stPVRRecRunInfo.hAvPlay, g_stPVRRecRunInfo.hWin, g_stPVRRecRunInfo.hSoundTrack);


    (MT_VOID)MTADP_Search_FreeAllPmt(g_stPVRRecRunInfo.pProgTbl);


    (MT_VOID)MTADP_Search_DeInit();


    (MT_VOID)MT_PVRRecDmxDeInit();


    (MT_VOID)MTADP_VO_DeInit();


    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_Fe_DeInit(g_stPVRRecRunInfo.sInputParam.tuner_id);

    memset(&g_stPVRRecRunInfo, 0, sizeof(g_stPVRRecRunInfo));

    g_bTaskQuit = MT_TRUE;
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_PVRRecParaseargs(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:t:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_PVRRecPrint_help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_PVRRecExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                MTADP_Strncpy(pInputParam->folder.file_name, mt_optarg, sizeof(mt_rec_file_para_t));
                return MT_SUCCESS;
#ifndef MT_SAMPLE_APP
            case 't':

                pInputParam->tuner_id = strtol(mt_optarg, 0, 0);
            break;

            case 's':
                if(argc < 11)
                {
                    (void)MT_PVRRecPrint_help(argv[0]);
                    return MT_FAILURE;
                }

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_SAT;
                pInputParam->input_param.sat.freq = strtol(argv[6], 0, 0);
                pInputParam->input_param.sat.sym_rate = strtol(argv[7], 0, 0);
                pInputParam->input_param.sat.onoff_22k = strtol(argv[8], 0, 0);
                pInputParam->input_param.sat.polarization = strtol(argv[9], 0, 0);
                pInputParam->input_param.sat.port_type = strtol(argv[10], 0, 0);


            return MT_SUCCESS;

            case 'c':
                if(argc < 9)
                {
                    (void)MT_PVRRecPrint_help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;
                pInputParam->input_param.cab.freq = strtol(argv[6], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[7], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[8], 0, 0);
            return MT_SUCCESS;
#endif
        }
    }


    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_PVRRecMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = 0;
    PMT_COMPACT_PROG *pstCurrentProgInfo = NULL;



    if(argc < 3  && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_PVRRecPrint_help(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_PVRRecParaseargs(argc, argv, &g_stPVRRecRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_PVRREC_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_PVRREC_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }


        ret = MTADP_Fe_Init(g_stPVRRecRunInfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("MTADP_Fe_Init failed\n");
            goto ERR1;
        }

        if (MT_INPUT_SIG_TYPE_CAB == g_stPVRRecRunInfo.sInputParam.sig_type)
        {     //dvbc
            ret = MT_PVRRecCheckDvbcParam(&g_stPVRRecRunInfo.sInputParam.input_param.cab);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PVRREC_ERR_PRINT("Input sat parameter error!\n");
                goto ERR2;
            }
            ret = MTADP_Fe_Connect_Dvbc(g_stPVRRecRunInfo.sInputParam.tuner_id,
                                    g_stPVRRecRunInfo.sInputParam.input_param.cab.freq,
                                    g_stPVRRecRunInfo.sInputParam.input_param.cab.sym_rate,
                                    g_stPVRRecRunInfo.sInputParam.input_param.cab.mod_type);
        }
        else if(MT_INPUT_SIG_TYPE_SAT == g_stPVRRecRunInfo.sInputParam.sig_type)
        {
            ret = MT_PVRRecCheckDvbsParam(&g_stPVRRecRunInfo.sInputParam.input_param.sat);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PVRREC_ERR_PRINT("Input sat parameter error!\n");
                goto ERR2;
            }
            ret = MTADP_Fe_Connect_Dvbs(g_stPVRRecRunInfo.sInputParam.tuner_id,
                                    g_stPVRRecRunInfo.sInputParam.input_param.sat.freq,
                                    g_stPVRRecRunInfo.sInputParam.input_param.sat.sym_rate,
                                    g_stPVRRecRunInfo.sInputParam.input_param.sat.onoff_22k,
                                    g_stPVRRecRunInfo.sInputParam.input_param.sat.polarization,
                                    g_stPVRRecRunInfo.sInputParam.input_param.sat.port_type);
        }

        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR2;
        }


        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("MTADP_Fe_Connect failed\n");
            goto ERR2;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("call MTADP_Disp_Init failed.\n");
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_PVRRecDmxInit(g_stPVRRecRunInfo.sInputParam.sig_type, g_stPVRRecRunInfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT( "failed to MT_PVRRecDmxInit\n");
            goto ERR6;
        }

        (void)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(PVR_DMX_ID_LIVE, &g_stPVRRecRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR8;
        }

        ret = MT_PVRRecAVplayInit(&g_stPVRRecRunInfo.hAvPlay, &g_stPVRRecRunInfo.hWin, &g_stPVRRecRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("failed to MT_PVRRecAVplayInit\n");
            goto ERR9;
        }
#else
        g_stPVRRecRunInfo.hAvPlay = avplayHandle.hAvPlay;
        g_stPVRRecRunInfo.hSoundTrack = avplayHandle.hSoundTrack;
        g_stPVRRecRunInfo.hWin = avplayHandle.hWin;
#endif



        ret = MT_UNF_PVR_RecInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_PVR_RecInit failed.\n");
            goto ERR10;
        }

        ret = MT_UNF_PVR_PlayInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT(" MT_UNF_PVR_PlayInit failed.\n");
            goto ERR11;
        }

        ret = MTADP_PVR_RegisterCallBacks(&g_stPVRRecRunInfo.hAvPlay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("call PVR_RegisterCallBacks failed.\n");
            goto ERR12;
        }
#ifndef MT_SAMPLE_APP
        /* Play the first program on the program list*/
        pstCurrentProgInfo = g_stPVRRecRunInfo.pProgTbl->proginfo;
        ret = MT_PVRRecAVPlay_Start(g_stPVRRecRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("call PVR_RecStart failed.\n");
            goto ERR12;
        }
#else
        ret = MTADP_Search_get_proglist(&g_stPVRRecRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT("call PVR_RecStart failed.\n");
            goto ERR12;
        }
#endif
        g_stPVRRecRunInfo.recChn1 = MT_INVALID_HANDLE;
        g_stPVRRecRunInfo.recChn2 = MT_INVALID_HANDLE;
        g_stPVRRecRunInfo.recChn3 = MT_INVALID_HANDLE;
        g_stPVRRecRunInfo.bIsRecStop = MT_FALSE;
        ret = pthread_create(&g_stPVRRecRunInfo.statusThd, MT_NULL, MT_PVRRecStatus, MT_NULL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT(" pthread_create   failed.\n");
            goto ERR12;
        }
        g_bTaskQuit = MT_FALSE;
    }
    (void)MT_PVRRecCmdTask(g_stPVRRecRunInfo.hAvPlay, g_stPVRRecRunInfo.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
    g_stPVRRecRunInfo.bIsRecStop = MT_TRUE;
    (void)pthread_join(g_stPVRRecRunInfo.statusThd, MT_NULL);

    if(play_resource.demux_use != MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = MT_PVRRecStopplay(g_stPVRRecRunInfo.hAvPlay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PVRREC_ERR_PRINT(" MT_PVRRecStopplay   failed.\n");
        }

        SAMPLE_PVRREC_INFO_PRINT("stop to play\n");
#endif
ERR13:
    (MT_VOID)MTADP_PVR_UnRegisterCallBacks();
ERR12:
    (MT_VOID)MT_UNF_PVR_PlayDeInit();

ERR11:
    (MT_VOID)MT_UNF_PVR_RecDeInit();

ERR10:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_PVRRecAvplayDeInit(g_stPVRRecRunInfo.hAvPlay, g_stPVRRecRunInfo.hWin, g_stPVRRecRunInfo.hSoundTrack);

ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stPVRRecRunInfo.pProgTbl);

ERR8:
    (MT_VOID)MTADP_Search_DeInit();

ERR7:
    (MT_VOID)MT_PVRRecDmxDeInit();

ERR6:
    (MT_VOID)MTADP_VO_DeInit();

ERR5:
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:
    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR2:
    (MT_VOID)MTADP_Fe_DeInit(g_stPVRRecRunInfo.sInputParam.tuner_id);

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif

    memset(&g_stPVRRecRunInfo, 0, sizeof(g_stPVRRecRunInfo));

    }
    play_resource.demux_use = MT_FALSE;

    return MT_SUCCESS;

}

