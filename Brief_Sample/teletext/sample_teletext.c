/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mt_debug.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_unf_avplay.h"
#include "mt_unf_common.h"
#include "mt_unf_disp.h"
#include "mt_unf_demux.h"
#include "mt_unf_sound.h"
#include "mt_type.h"
#include "sample_teletext_out.h"
#include "sample_teletext_msg.h"
#include "sample_ttx_data.h"
#include "mt_adp_frontend.h"
#include "sample_common.h"
#include "mt_cmdline.h"


/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_TTX_DEBUG

#define MT_TELETEXT_PRINT   printf
#else

#define MT_TELETEXT_PRINT

#endif

#define SAMPLE_TELETEXT_FUNCTION_ENTER()     MT_TELETEXT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TELETEXT_FUNCTION_EXIT()      MT_TELETEXT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_TELETEXT_FATAL_PRINT(fmt...)          MT_TELETEXT_PRINT(" [FATAL] " fmt)
#define SAMPLE_TELETEXT_ERR_PRINT(fmt...)            MT_TELETEXT_PRINT(" [ERROR] " fmt)
#define SAMPLE_TELETEXT_WARN_PRINT(fmt...)           MT_TELETEXT_PRINT(" [WARN] "  fmt)
#define SAMPLE_TELETEXT_INFO_PRINT(fmt...)           MT_TELETEXT_PRINT(" [INFO] "  fmt)
#define SAMPLE_TELETEXT_DBG_PRINT(fmt...)            MT_TELETEXT_PRINT(" [DEBUG] " fmt)

#define SAMPLE_TELETEXT_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2


#define DMX_ID_0 0
#define TUNER_ID_0 (0)

#define INVALID_TSPID (0x1fff)


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
    mt_u32 freq; /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 mod_type; /**<QAM mode*/
    mt_u8 port_type;
} mt_input_ter_para_t;


typedef struct tagInput_Param_T
{
    mt_u8 file_name[256];
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
    mt_u8 file_name[256];
}source_file_param_t;

typedef struct
{
    mt_input_para_t sInputParam;
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    pthread_t          htsThd;
    PMT_COMPACT_TBL *pProgTbl;
    MT_HANDLE hSubtData;
    MT_HANDLE hTTX;
} MT_Teletext_RUN_INFO;




/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_Teletext_RUN_INFO    g_stTeletextRunInfo;

extern pthread_mutex_t g_stTtxDataMutex;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_TeletextMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

#ifndef MT_SAMPLE_APP
static mt_s32 MT_TeletextCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_TELETEXT_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_TELETEXT_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_TELETEXT_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_TeletextCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_TELETEXT_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_TELETEXT_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}



/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_TeletextDmxInit(mt_input_para_t *pInputParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_TELETEXT_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == pInputParam->sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            (mt_void) MT_UNF_DMX_DeInit();
            return MT_FAILURE;
        }
    }
    else
    {
         memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to mt_sys_get_version\n");
            return MT_FAILURE;
        }

        if(MT_INPUT_SIG_TYPE_CAB == pInputParam->sig_type)
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
        else if(MT_INPUT_SIG_TYPE_SAT == pInputParam->sig_type)
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
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    SAMPLE_TELETEXT_FUNCTION_EXIT();


    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return MT_VOID
*/
static MT_VOID MT_TeletextDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

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
static mt_s32 MT_TeletextAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_TELETEXT_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_TELETEXT_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_TELETEXT_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
 @return MT_VOID
*/
static MT_VOID  MT_TeletextAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_TELETEXT_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_TELETEXT_ERR_PRINT("=====hWin is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_TELETEXT_ERR_PRINT("=====hSoundTrack is INVALID_HANDLE ======\n");
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
static mt_s32 MT_TeletextAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
{
    mt_u32 VidPid = 0;
    mt_u32 AudPid = 0;
    mt_u32 u32AudType = 0;
    mt_s32 ret = 0;
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0X00;


    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_TELETEXT_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == p_ProgInfo)
    {
        SAMPLE_TELETEXT_ERR_PRINT("p_ProgInfo is NULL!\n");
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
        u32AudType = 0xffffffff;
    }

    SAMPLE_TELETEXT_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_TELETEXT_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_TELETEXT_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_TELETEXT_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_TELETEXT_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_TELETEXT_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
            return ret;
        }
    }

    return MT_SUCCESS;
}

/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_TeletextInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_file_param_t *pstParam = (source_file_param_t *)(args);

    SAMPLE_TELETEXT_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_TELETEXT_ERR_PRINT( "\nfile %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        (mt_void)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_TELETEXT_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_TELETEXT_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    return MT_SUCCESS;
}

/*
@brief help
@return void
*/

static void MT_TeletextPrintDvbcHelp(char *name)
{
    MT_TELETEXT_PRINT("\nDvbc Usage:\n");
    MT_TELETEXT_PRINT("%s -c freq symrate qam\n", name);
    MT_TELETEXT_PRINT("    freq : \n");
    MT_TELETEXT_PRINT("    symrate: \n");
    MT_TELETEXT_PRINT("    qam: \n");
    MT_TELETEXT_PRINT("example:\n");
    MT_TELETEXT_PRINT("    %s -c 654 6875 64\n", name);
}

static void MT_TeletextPrintDvbsHelp(char *name)
{
    MT_TELETEXT_PRINT("\nDvbs Usage:\n");
    MT_TELETEXT_PRINT("%s -s freq symrate onoff_22k polarization port_type\n", name);
    MT_TELETEXT_PRINT("    freq : \n");
    MT_TELETEXT_PRINT("    symrate: \n");
    MT_TELETEXT_PRINT("    onoff_22k: \n");
    MT_TELETEXT_PRINT("example:\n");
    MT_TELETEXT_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}

static void MT_TeletextPrintFileHelp(char *name)
{
    MT_TELETEXT_PRINT("\nDvbs Usage:\n");
    MT_TELETEXT_PRINT(" %s -f file\n", name);
    MT_TELETEXT_PRINT("example:\n");
    MT_TELETEXT_PRINT("    %s -f ./xxx.ts\n", name);
}
#endif

static void MT_TeletextPrint_help(char *name)
{
    MT_TELETEXT_PRINT("Lack of parameters\n");
    MT_TELETEXT_PRINT("\nUsage:\n");
    MT_TELETEXT_PRINT(" %s\n", name);
    MT_TELETEXT_PRINT("    -f: path of the subtitle stream file\n");
    MT_TELETEXT_PRINT("    -c: DVBC locks frequency\n");
    MT_TELETEXT_PRINT("    -s: DVBS locks frequency\n");
    MT_TELETEXT_PRINT("example:\n");
    MT_TELETEXT_PRINT("    %s -f ./sub.ts\n", name);
    MT_TELETEXT_PRINT("    %s -c 654 6875 64\n", name);
    MT_TELETEXT_PRINT("    %s -s 3840 27500 1 0 0\n", name);
    MT_TELETEXT_PRINT("    %s -q  <exit> \n", name);
}

/*
@brief stop to play
@param[in] avplay, Player handle
@return MT_SUCCESS
*/
static mt_s32 MT_TeletextStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if(MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_TELETEXT_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}

/*
@brief A callback function that filters ttx data
@param[in] u32UserData, pid of ttx data
@param[in] pu8Data, ttx data
@param[in] u32DataLength,ttx data length
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_TeletextFilterDataCallback(mt_u32 u32UserData, mt_u8 *pu8Data, mt_u32 u32DataLength)
{
    mt_s32 s32Ret = MT_SUCCESS;

    mt_u32 u32TtxPID = u32UserData;

    if(u32TtxPID)
    {
        s32Ret = MT_UNF_TTX_InjectData(g_stTeletextRunInfo.hTTX, u32TtxPID, pu8Data, u32DataLength);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_TTX_InjectData, u32TtxPID=0x%x\n", u32TtxPID);
        }
    }


    return s32Ret;
}

/*
@brief Get dmx channel data
@param[in] u32ProgNum,Select the program number to play
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_S32 MT_TeletextStartDataFilter(PMT_COMPACT_PROG *stCurrentProgInfo)
{
    MT_S32 s32Ret = MT_SUCCESS;
    mt_u8 u8Index = 0;
    TTX_DATA_INSTALL_PARAM_S stInstallParam = { 0 };

    s32Ret = Ttx_Data_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("failed to Ttx_Data_Init\n");
        return MT_FAILURE;
    }

    stInstallParam.u32DmxID = 0;
    stInstallParam.pfnCallback = MT_TeletextFilterDataCallback;
    if(stCurrentProgInfo->u16TtxNum > 0)
    {
        u8Index = 0;
        stInstallParam.u16TtxPID = stCurrentProgInfo->stTtxInfo[u8Index].u16TtxPID;
        stInstallParam.u32UserData = (mt_u32)stCurrentProgInfo->stTtxInfo[u8Index].u16TtxPID;
    }

    s32Ret = Ttx_Data_Install(&stInstallParam, &g_stTeletextRunInfo.hSubtData);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("failed to Ttx_Data_Install\n");
        (mt_void) Ttx_Data_DeInit();
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/*
@brief Defines the parameter of subtitle instance
@param[in] pstProginfo, pmt program information
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_S32 MT_TeletextCreateDVBTtx(PMT_COMPACT_PROG *pstProginfo)
{
    mt_u8 u8Index;
    MT_S32 s32Ret = MT_SUCCESS;
    MT_UNF_TTX_PARAM_S stTtxParam;
    MT_HANDLE hUnfSO = 0;
    if(MT_NULL == pstProginfo)
    {
        return MT_FAILURE;
    }
    memset(&stTtxParam, 0, sizeof(stTtxParam));

    stTtxParam.u32UserData = (mt_u32)hUnfSO;
    stTtxParam.u8TtxItemNum = pstProginfo->u16TtxNum;
    for (u8Index = 0; u8Index < stTtxParam.u8TtxItemNum; u8Index++)
    {
        stTtxParam.astItems[u8Index].u32TtxPID = pstProginfo->stTtxInfo[u8Index].u16TtxPID;
    }
    s32Ret = MT_UNF_TTX_DataRecv_Create(&stTtxParam, &g_stTeletextRunInfo.hTTX);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("failed to MT_UNF_TTX_DataRecv_Create\n");
    }
    return s32Ret;
}


static mt_u8 MT_TeletextSelectTTXDescriptor(PMT_COMPACT_PROG *pstProginfo)
{
    mt_u8 u8DesNo = 0xff; /* for Descriptor No. */
    mt_u8 i;
    mt_u8 u8Descriptor;
    mt_u8 u8MaxDesNum = pstProginfo->stTtxInfo[0].u8DesInfoCnt;
    mt_u8 u8FirstSupportDes = 0;
    mt_u8 u8SupportDesCnt = 0;
    MT_BOOL bHasSupportDes = MT_FALSE;

    MT_TELETEXT_PRINT("\nTeletext Descriptor number is %d, ", u8MaxDesNum);
    if(u8MaxDesNum > 0)
    {
        for(i = 0; i < u8MaxDesNum; i++)
        {
            u8Descriptor = pstProginfo->stTtxInfo[0].stTtxDes[i].u8TtxType;
            if((1 == u8Descriptor) ||(2 == u8Descriptor))
            {
                bHasSupportDes = MT_TRUE;
                u8FirstSupportDes = i;
                break;
            }
        }
        if(MT_TRUE == bHasSupportDes)
        {
            MT_TELETEXT_PRINT("we support the descriptor: \n");
            for(i = 0; i < u8MaxDesNum; i++)
            {
                u8Descriptor = pstProginfo->stTtxInfo[0].stTtxDes[i].u8TtxType;
                if (1 == u8Descriptor)
                {
                    u8SupportDesCnt++;
                    MT_TELETEXT_PRINT("%d-------%s \n",i,"initial Teletext page");

                }
                else if (2 == u8Descriptor)
                {
                    u8SupportDesCnt++;
                    MT_TELETEXT_PRINT("%d-------%s \n",i,"Teletext subtitle page");
                }
            }
        }
        else
        {
            MT_TELETEXT_PRINT("but have not descriptor we can support ");
        }

    }
    MT_TELETEXT_PRINT("\n");

    if (bHasSupportDes)
    {
        mt_u8  au8DesNo[32];

        if(u8SupportDesCnt > 1)
        {
            MT_TELETEXT_PRINT("Please input a teletext Descriptor number:");


            fgets((char *)(au8DesNo), (sizeof(au8DesNo) - 1), stdin);
            u8DesNo = atoi((char*)au8DesNo);

            /*check if the user input is valid or not*/
            if (u8DesNo < u8MaxDesNum)
            {
                u8Descriptor = pstProginfo->stTtxInfo[0].stTtxDes[u8DesNo].u8TtxType;
                if((1 != u8Descriptor) &&(2 != u8Descriptor))
                {
                    MT_TELETEXT_PRINT("\nTeletext Descriptor number %d is invalid, use the first support number %d\n", u8DesNo, u8FirstSupportDes);
                    u8DesNo = u8FirstSupportDes;
                }
            }
            else
            {
                MT_TELETEXT_PRINT("\nTeletext Descriptor number %d is invalid, use the first support number %d\n", u8DesNo, u8FirstSupportDes);
                u8DesNo = u8FirstSupportDes;
            }
        }
        else
        {
            u8DesNo = u8FirstSupportDes;
        }

    }

    return u8DesNo;
}


/*
@brief Get the TTX data
@param[in] u32ProgNum, Select the program number to play
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_S32 MT_TeletextStartTtx(PMT_COMPACT_PROG *stCurrentProgInfo)
{
    MT_S32 s32Ret = MT_SUCCESS;
    mt_u8 u8TTXNum = 0;
    mt_u8 u8PageNum = 0;
    mt_u8 u8DescriptorNum = 0;
    MT_UNF_TTX_FONT_SRC_S font = { 0 };
    MT_UNF_TTX_CONTENT_PARA_S stContentParam = {0};
    MT_UNF_TTX_INIT_PARA_S stInitParam = { 0 };


    font.p_pal_font = wstfont2_bits_pal_vsb_test;
    font.p_ntsl_font = wstfont2_bits_ntsc_vsb_test;
    font.p_small_font = wstfont2_bits_small_vsb_test;
    font.p_hd_font = wstfont2_bits_hd_vsb;


    /* Create subtitle module instance */
    s32Ret = MT_TeletextCreateDVBTtx(stCurrentProgInfo);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("failed to MT_TeletextCreateDVBTtx\n");
        g_stTeletextRunInfo.hTTX = 0;
        return MT_FAILURE;
    }
    s32Ret = Mtgo_Teletext_Init();
    if(MT_SUCCESS != s32Ret)
    {

        (void)MT_UNF_TTX_DataRecv_Destroy(g_stTeletextRunInfo.hTTX);
        SAMPLE_TELETEXT_ERR_PRINT("call Mtgo_Teletext_Init failed !\n");
        g_stTeletextRunInfo.hTTX = 0;
        return MT_FAILURE;
    }
    s32Ret = MT_UNF_TTX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("call MT_UNF_TTX_Init failed !\n");
        (void)MT_UNF_TTX_DataRecv_Destroy(g_stTeletextRunInfo.hTTX);
        (void)Mtgo_Teletext_DeInit();
        g_stTeletextRunInfo.hTTX = 0;
        return MT_FAILURE;
    }

    stInitParam.pfnCB = TTX_SampleCallBack;
    stInitParam.p_font = &font;
    stInitParam.bNavigation = MT_TRUE;
    stInitParam.bFlash = MT_TRUE;
    s32Ret = MT_UNF_TTX_Create(&stInitParam, &g_stTeletextRunInfo.hTTX);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("call MT_UNF_TTX_Create failed !\n");
        (void)MT_UNF_TTX_DataRecv_Destroy(g_stTeletextRunInfo.hTTX);
        (void)Mtgo_Teletext_DeInit();
        (void)MT_UNF_TTX_DeInit();
        g_stTeletextRunInfo.hTTX = 0;
        return MT_FAILURE;
    }

    u8DescriptorNum = MT_TeletextSelectTTXDescriptor(stCurrentProgInfo);

    stContentParam.enType = stCurrentProgInfo->stTtxInfo[u8TTXNum].stTtxDes[u8DescriptorNum].u8TtxType;
    stContentParam.u32ISO639LanCode = stCurrentProgInfo->stTtxInfo[u8TTXNum].stTtxDes[u8DescriptorNum].u32ISO639LanguageCode;

    u8PageNum = stCurrentProgInfo->stTtxInfo[u8TTXNum].stTtxDes[u8DescriptorNum].u8TtxPageNumber;
    stContentParam.stInitPgAddr.u8MagazineNum = stCurrentProgInfo->stTtxInfo[u8TTXNum].stTtxDes[u8DescriptorNum].u8TtxMagazineNumber;
    //stContentParam.stInitPgAddr.u8PageNum = ((u8PageNum >> 4)*10) + (u8PageNum & 0xf);
    stContentParam.stInitPgAddr.u8PageNum = u8PageNum;

    stContentParam.u16MaxPageNum = 800;
    stContentParam.u16MaxSubPageNum = 100;

    s32Ret = MT_UNF_TTX_SwitchContent(g_stTeletextRunInfo.hTTX, &stContentParam);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT(" MT_UNF_TTX_SwitchContent failed !\n");
        (void)MT_UNF_TTX_Destroy(g_stTeletextRunInfo.hTTX);
        (void)MT_UNF_TTX_DataRecv_Destroy(g_stTeletextRunInfo.hTTX);
        (void)Mtgo_Teletext_DeInit();
        (void)MT_UNF_TTX_DeInit();
        g_stTeletextRunInfo.hTTX = 0;
        return MT_FAILURE;
    }


    s32Ret = MT_UNF_TTX_Output(g_stTeletextRunInfo.hTTX,MT_UNF_TTX_OSD_OUTPUT,MT_FALSE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("call MT_UNF_TTX_Output failed !\n");
        (void)MT_UNF_TTX_Destroy(g_stTeletextRunInfo.hTTX);
        (void)MT_UNF_TTX_DataRecv_Destroy(g_stTeletextRunInfo.hTTX);
        (void)Mtgo_Teletext_DeInit();
        (void)MT_UNF_TTX_DeInit();
        g_stTeletextRunInfo.hTTX = 0;
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}





/*
@brief help information
@return void
*/
static mt_void MT_TeletextPrintMenu(void)
{
    SAMPLE_TELETEXT_PRINT("      s: Next page \n");
    SAMPLE_TELETEXT_PRINT("      w: Previous page \n");
    SAMPLE_TELETEXT_PRINT("      a: Previous subpage \n");
    SAMPLE_TELETEXT_PRINT("      d: Next subpage \n");
    SAMPLE_TELETEXT_PRINT("      z: Last magazine \n");
    SAMPLE_TELETEXT_PRINT("      x: Next magazine \n");
    SAMPLE_TELETEXT_PRINT("    0-9: Enter three numbers to select a page. The first digit is the magazine number and the next two digits are the page number \n");
    SAMPLE_TELETEXT_PRINT("      c: x/27The first link in the package \n");
    SAMPLE_TELETEXT_PRINT("      v: x/27The second link in the package \n");
    SAMPLE_TELETEXT_PRINT("      r: x/27The third link in the package \n");
    SAMPLE_TELETEXT_PRINT("      n: x/27The fourth link in the package \n");
    SAMPLE_TELETEXT_PRINT("      m: Start page \n");
    SAMPLE_TELETEXT_PRINT("      u: Update current page \n");
    SAMPLE_TELETEXT_PRINT("      #: teletext content type is teletext \n");
    SAMPLE_TELETEXT_PRINT("      @: teletext content type is subtitle \n");
    SAMPLE_TELETEXT_PRINT("      f: open teletext \n");
    SAMPLE_TELETEXT_PRINT("      g: close teletext  \n");
#ifndef MT_SAMPLE_APP
    SAMPLE_TELETEXT_PRINT("      p: switch program number \n");
#else
    SAMPLE_TELETEXT_PRINT("      b: background run \n");
#endif
    SAMPLE_TELETEXT_PRINT("      h: Help information \n");
    SAMPLE_TELETEXT_PRINT("      q: exit teltext \n");
    SAMPLE_TELETEXT_PRINT("Teletext>> ");

}



/*
@brief A function on the ttx operation command
@param[in] pInputParam, Input param
@param[in] p_stCurrentProgInfo, Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static void MT_TeletextCmdTask(MT_HANDLE  hAvplay, PMT_COMPACT_TBL *pProgTbl, mt_handle httx)
{
#ifndef MT_SAMPLE_APP
    mt_u32   ret = 0;
    mt_u32     u32ProgNum = 0;
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#endif
    MT_UNF_TTX_CONTENT_PARA_S scontentParam;
    MT_UNF_TTX_CMD_E enCMD = MT_UNF_TTX_CMD_KEY;
    MT_UNF_TTX_KEY_E enKey = { 0 };
    MT_CHAR   InputCmd = 0;
    MT_BOOL   bQuit = MT_FALSE;
    MT_BOOL   ttx_start = MT_FALSE;


    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_TELETEXT_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }

    while(bQuit == MT_FALSE)
    {
        (void)MT_TeletextPrintMenu();
        scanf(" %c",&InputCmd);
        switch (InputCmd)
        {
        case '0':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_0;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }

            break;
        case '1':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_1;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }

            break;
        case '2':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_2;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case '3':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_3;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case '4':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_4;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case '5':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_5;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case '6':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_6;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case '7':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_7;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case '8':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_8;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case '9':
            if(ttx_start == MT_TRUE)
            {
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_9;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'w':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->w      Previous page \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_PREVIOUS_PAGE;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }

            break;

        case 's':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->s      Next page \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_NEXT_PAGE;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'a':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->a      Previous subpage \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_PREVIOUS_SUBPAGE;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'd':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->d      Next subpage \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_NEXT_SUBPAGE;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;
        case 'z':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->z      Last magazine \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_PREVIOUS_MAGAZINE;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'x':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->x      Next magazine \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_NEXT_MAGAZINE;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'c':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->c      x/27The first link in the package \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_RED;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'v':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->v      x/27The second link in the package \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_GREEN;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'r':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->b      x/27The third link in the package \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_YELLOW;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'n':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->n      x/27The fourth link in the package \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_CYAN;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'm':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->m      Start page \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_INDEX;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case 'u':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->u      Update current page \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                enCMD  = MT_UNF_TTX_CMD_KEY;
                enKey  = MT_UNF_TTX_KEY_UPDATE;
                (void)MT_UNF_TTX_ExecCmd(httx, enCMD, (mt_void*)&enKey);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;


        case 'q':
            SAMPLE_TELETEXT_INFO_PRINT("--->q      exit teltext \n");
            bQuit = MT_TRUE;
            g_bTaskQuit = MT_TRUE;

            break;
#ifdef MT_SAMPLE_APP
        case 'b':
        {
            SAMPLE_TELETEXT_INFO_PRINT("teletext in back!\n");
            bQuit = MT_TRUE;
            break;
        }
#endif
        case 'f':
        {
            pthread_mutex_lock(&g_stTtxDataMutex);
            if(g_stTeletextRunInfo.hTTX != 0)
            {
                (void)MT_UNF_TTX_Output(httx, MT_UNF_TTX_OSD_OUTPUT, MT_TRUE);
                ttx_start = MT_TRUE;
            }
            pthread_mutex_unlock(&g_stTtxDataMutex);
        }
        break;
        case 'g':
        {
            pthread_mutex_lock(&g_stTtxDataMutex);
            if(g_stTeletextRunInfo.hTTX != 0)
            {
                (void)MT_UNF_TTX_Output(httx, MT_UNF_TTX_OSD_OUTPUT, MT_FALSE);
                ttx_start = MT_FALSE;
            }
            pthread_mutex_unlock(&g_stTtxDataMutex);

            break;
        }

        break;
#ifndef MT_SAMPLE_APP
        case 'p':
            SAMPLE_TELETEXT_INFO_PRINT("--->p      Switch program \n");

            SAMPLE_TELETEXT_INFO_PRINT("Please enter the program you want to play 0-%d\n", pProgTbl->prog_num - 1);

            scanf("%d", &u32ProgNum);
            if(u32ProgNum < pProgTbl->prog_num)
            {
                ttx_start = MT_FALSE;
                if(g_stTeletextRunInfo.hTTX != 0)
                {
                    (MT_VOID)Ttx_Data_Uninstall(g_stTeletextRunInfo.hSubtData);
                    (MT_VOID)Ttx_Data_DeInit();
                    (MT_VOID)MT_UNF_TTX_Destroy(httx);
                    (MT_VOID)MT_UNF_TTX_DataRecv_Destroy(httx);
                    (MT_VOID)Mtgo_Teletext_DeInit();
                    (MT_VOID)MT_UNF_TTX_DeInit();
                }

                (void)MT_TeletextStopplay(hAvplay);
                pstCurrentProgInfo = pProgTbl->proginfo + (u32ProgNum % (pProgTbl->prog_num));
                (void)MT_TeletextAVPlay_Start(hAvplay, pstCurrentProgInfo);
                ret = MT_TeletextStartTtx(pstCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TELETEXT_ERR_PRINT("This show has no ttx information  \n");
                    break;
                }
                (void)MT_TeletextStartDataFilter(pstCurrentProgInfo);
            }
            else
            {
                SAMPLE_TELETEXT_ERR_PRINT("input err  \n");
            }

            break;
#endif
       case '@':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->@      teletext content type is subtitle \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                scontentParam.enType = MT_UNF_TTX_TTXSUBT;
                scontentParam.stInitPgAddr.u8MagazineNum = 0;
                scontentParam.stInitPgAddr.u8PageNum = 1;
                (void)MT_UNF_TTX_ResetData(httx);
                (void)MT_UNF_TTX_Output(httx, MT_UNF_TTX_OSD_OUTPUT, MT_FALSE);
                (void)MT_UNF_TTX_Output(httx, MT_UNF_TTX_OSD_OUTPUT, MT_TRUE);
                (void)MT_UNF_TTX_SwitchContent(httx, &scontentParam);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case '#':
            if(ttx_start == MT_TRUE)
            {
                SAMPLE_TELETEXT_INFO_PRINT("--->#      teletext content type is teletext \n");
                pthread_mutex_lock(&g_stTtxDataMutex);
                scontentParam.enType = MT_UNF_TTX_INITTTX;
                scontentParam.stInitPgAddr.u8MagazineNum = 0xff;
                scontentParam.stInitPgAddr.u8PageNum = 0xff;
                (void)MT_UNF_TTX_ResetData(httx);
                (void)MT_UNF_TTX_Output(httx, MT_UNF_TTX_OSD_OUTPUT, MT_FALSE);
                (void)MT_UNF_TTX_Output(httx, MT_UNF_TTX_OSD_OUTPUT, MT_TRUE);
                (void)MT_UNF_TTX_SwitchContent(httx, &scontentParam);
                pthread_mutex_unlock(&g_stTtxDataMutex);
            }
            break;

        case '\n':
            break;
        case 'h':
            (void)MT_TeletextPrintMenu();
            break;

        default:
            (void)MT_TeletextPrintMenu();
            break;
        }
    }


}

static MT_VOID MT_TeletextExit(void)
{


    (MT_VOID)MT_TeletextStopplay(g_stTeletextRunInfo.hAvPlay);
    if(g_stTeletextRunInfo.hTTX != 0)
    {
        (MT_VOID)Ttx_Data_Uninstall(g_stTeletextRunInfo.hSubtData);

        (MT_VOID)Ttx_Data_DeInit();

        (MT_VOID)MT_UNF_TTX_Destroy(g_stTeletextRunInfo.hTTX);

        (MT_VOID)MT_UNF_TTX_DataRecv_Destroy(g_stTeletextRunInfo.hTTX);

        (MT_VOID)Mtgo_Teletext_DeInit();

        (MT_VOID)MT_UNF_TTX_DeInit();
    }

#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_TeletextAvplayDeInit(g_stTeletextRunInfo.hAvPlay, g_stTeletextRunInfo.hWin, g_stTeletextRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_stTeletextRunInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == g_stTeletextRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        (void)pthread_join(g_stTeletextRunInfo.htsThd, NULL);
    }

    (MT_VOID)MT_TeletextDmxDeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != g_stTeletextRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }
#endif
    memset(&g_stTeletextRunInfo, 0xff, sizeof(g_stTeletextRunInfo));

    g_bTaskQuit = MT_TRUE;
}



/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_TeletextParase_args(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;
#ifndef MT_SAMPLE_APP
    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_TeletextPrint_help(argv[0]);
        return MT_FAILURE;
    }
#endif
        while((opt = MTADP_Getopt(argc, argv, "h?Hf:c:s:q")) != -1)
        {
            switch(opt)
            {
                case 'h':
                case '?':
                case 'H':
                    (void)MT_TeletextPrint_help(argv[0]);
                return MT_FAILURE;
                case 'q':
                    if(g_bTaskQuit == MT_FALSE)
                    {
                        (MT_VOID)MT_TeletextExit();
                    }
                    return MT_TASK_EXIT;
#ifndef MT_SAMPLE_APP
                case 'f':
                    if(argc < 3)
                    {
                        (void)MT_TeletextPrintFileHelp(argv[0]);
                        return MT_FAILURE;
                    }
                    pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                    MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

                case 's':
                    if(argc < 6)
                    {
                        (void)MT_TeletextPrintDvbsHelp(argv[0]);
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
                    if(argc < 4)
                    {
                        (void)MT_TeletextPrintDvbcHelp(argv[0]);
                        return MT_FAILURE;
                    }
                    pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                    pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                    pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                    pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
#endif
                default:
                    (void)MT_TeletextPrint_help(argv[0]);
                    return MT_FAILURE;
            }
        }


        return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_TeletextMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#ifndef MT_SAMPLE_APP
    mt_u32     u32ProgNum = 0;
#endif



    ret = MT_TeletextParase_args(argc, argv, &g_stTeletextRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_TELETEXT_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }


    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }

        g_bTaskQuit = MT_FALSE;
        if(MT_INPUT_SIG_TYPE_FILE != g_stTeletextRunInfo.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TELETEXT_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            if(MT_INPUT_SIG_TYPE_CAB == g_stTeletextRunInfo.sInputParam.sig_type)
            {     //dvbc
                ret = MT_TeletextCheckDvbcParam(&g_stTeletextRunInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TELETEXT_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        g_stTeletextRunInfo.sInputParam.input_param.cab.freq,
                                        g_stTeletextRunInfo.sInputParam.input_param.cab.sym_rate,
                                        g_stTeletextRunInfo.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_stTeletextRunInfo.sInputParam.sig_type)
            {
                ret = MT_TeletextCheckDvbsParam(&g_stTeletextRunInfo.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TELETEXT_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        g_stTeletextRunInfo.sInputParam.input_param.sat.freq,
                                        g_stTeletextRunInfo.sInputParam.input_param.sat.sym_rate,
                                        g_stTeletextRunInfo.sInputParam.input_param.sat.onoff_22k,
                                        g_stTeletextRunInfo.sInputParam.input_param.sat.polarization,
                                        g_stTeletextRunInfo.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_TELETEXT_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }
        }


        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("MTADP_Disp_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_TeletextDmxInit(&g_stTeletextRunInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT( "failed to StartDmx\n");
            goto ERR6;
        }

        if(MT_INPUT_SIG_TYPE_FILE == g_stTeletextRunInfo.sInputParam.sig_type)
        {

            ret = pthread_create(&g_stTeletextRunInfo.htsThd, NULL, (void * (*)(void *))MT_TeletextInjectTsTask, &g_stTeletextRunInfo.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TELETEXT_ERR_PRINT("failed to pthread_create\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }

        (void)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stTeletextRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR9;
        }

        ret = MT_TeletextAVplayInit(&g_stTeletextRunInfo.hAvPlay, &g_stTeletextRunInfo.hWin, &g_stTeletextRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_TeletextAVplayInit\n");
            goto ERR10;
        }

        for(u32ProgNum = 0; u32ProgNum < g_stTeletextRunInfo.pProgTbl->prog_num; u32ProgNum++)
        {
            SAMPLE_TELETEXT_INFO_PRINT("The number of teletext of the program[%d]: %d\n", u32ProgNum, g_stTeletextRunInfo.pProgTbl->proginfo[u32ProgNum].u16TtxNum);
        }
        for(u32ProgNum = 0; u32ProgNum < g_stTeletextRunInfo.pProgTbl->prog_num; u32ProgNum++)
        {
            if(0 != g_stTeletextRunInfo.pProgTbl->proginfo[u32ProgNum].u16TtxNum)
            {
                SAMPLE_TELETEXT_INFO_PRINT("The current video has a ttx, ttx number: %d\n", g_stTeletextRunInfo.pProgTbl->proginfo[u32ProgNum].u16TtxNum);
                break;
            }
        }

        if(0 == g_stTeletextRunInfo.pProgTbl->proginfo[u32ProgNum].u16TtxNum)
        {
            SAMPLE_TELETEXT_ERR_PRINT("There is no ttx for the video stream\n");
            goto ERR11;
        }

        pstCurrentProgInfo = g_stTeletextRunInfo.pProgTbl->proginfo + (u32ProgNum % (g_stTeletextRunInfo.pProgTbl->prog_num));



        ret = MT_TeletextAVPlay_Start(g_stTeletextRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_TeletextAVPlay_Start\n");
            goto ERR11;
        }
#endif

#ifdef MT_SAMPLE_APP
        g_stTeletextRunInfo.hAvPlay = avplayHandle.hAvPlay;


        ret = MTADP_Get_Current_Info(&pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("MTADP_Get_Current_Info ERR !!\n");
            goto ERR12;
        }
#endif

        ret = MT_TeletextStartTtx(pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("There is no ttx for the video stream\n");
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_TeletextStartTtx\n");
            goto ERR12;
        }
        ret = MT_TeletextStartDataFilter(pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TELETEXT_ERR_PRINT("failed to MT_TeletextStartDataFilter\n");
            goto ERR13;
        }
        g_bTaskQuit = MT_FALSE;
    }
    (mt_void)MT_TeletextCmdTask(g_stTeletextRunInfo.hAvPlay, g_stTeletextRunInfo.pProgTbl, g_stTeletextRunInfo.hTTX);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    if(g_stTeletextRunInfo.hTTX != 0)
    {

        (MT_VOID)Ttx_Data_Uninstall(g_stTeletextRunInfo.hSubtData);

        (MT_VOID)Ttx_Data_DeInit();
    }

ERR13:
    if(g_stTeletextRunInfo.hTTX != 0)
    {
        (MT_VOID)MT_UNF_TTX_Output(g_stTeletextRunInfo.hTTX,MT_UNF_TTX_DUAL_OUTPUT,MT_FALSE);

        (MT_VOID)MT_UNF_TTX_Destroy(g_stTeletextRunInfo.hTTX);

        (MT_VOID)MT_UNF_TTX_DataRecv_Destroy(g_stTeletextRunInfo.hTTX);

        (MT_VOID)Mtgo_Teletext_DeInit();

        (MT_VOID)MT_UNF_TTX_DeInit();
    }

ERR12:
#ifndef MT_SAMPLE_APP
     (MT_VOID)MT_TeletextStopplay(g_stTeletextRunInfo.hAvPlay);

ERR11:
    (MT_VOID)MT_TeletextAvplayDeInit(g_stTeletextRunInfo.hAvPlay, g_stTeletextRunInfo.hWin, g_stTeletextRunInfo.hSoundTrack);

ERR10:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stTeletextRunInfo.pProgTbl);

ERR9:
    (MT_VOID)MTADP_Search_DeInit();

ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == g_stTeletextRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit  = MT_TRUE;
        (void)pthread_join(g_stTeletextRunInfo.htsThd, NULL);
    }
ERR7:
    (MT_VOID)MT_TeletextDmxDeInit();

ERR6:
    (MT_VOID)MTADP_VO_DeInit();

ERR5:
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:

    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR2:
    if(MT_INPUT_SIG_TYPE_FILE != g_stTeletextRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR1:

    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_stTeletextRunInfo, 0xff, sizeof(g_stTeletextRunInfo));

    return MT_SUCCESS;
}




