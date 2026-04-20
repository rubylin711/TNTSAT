/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_demux.h"
#include "mt_unf_descrambler.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecs.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_pvr.h"
#include "mt_mpi_demux.h"

#include "mt_adp_hdmi.h"
#include "mt_adp_demux.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
#include "mt_cmdline.h"



/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_TRACK_DEBUG

#define MT_TRACK_PRINT   printf
#else

#define MT_TRACK_PRINT

#endif

#define SAMPLE_TRACK_FUNCTION_ENTER()       MT_TRACK_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TRACK_FUNCTION_EXIT()        MT_TRACK_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_TRACK_FATAL_PRINT(fmt...)    MT_TRACK_PRINT(" [FATAL] " fmt)
#define SAMPLE_TRACK_ERR_PRINT(fmt...)      MT_TRACK_PRINT(" [ERROR] " fmt)
#define SAMPLE_TRACK_WARN_PRINT(fmt...)     MT_TRACK_PRINT(" [WARN] "  fmt)
#define SAMPLE_TRACK_INFO_PRINT(fmt...)     MT_TRACK_PRINT(" [INFO] "  fmt)
#define SAMPLE_TRACK_DBG_PRINT(fmt...)      MT_TRACK_PRINT(" [DEBUG] " fmt)


#define SAMPLE_TRACK_PRINT   printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0 0
#define TUNER_ID_0 (0)

#define INVALID_TSPID (0x1fff)

#define SWITCH_PVR_MAX_AUDIO_PID_NUM    (12)

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
    mt_u32 multi_audio_flag;
    mt_u32 apid_first_part;
    mt_u32 apid_num;
    mt_u32 cur_apid_indx;
} mt_multi_audio_set;

typedef struct
{
    mt_input_para_t sInputParam;
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    pthread_t          htsThd;
    PMT_COMPACT_TBL  *pProgTbl;
} MT_SwitchTrack_RUN_INFO;



/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static mt_multi_audio_set g_multiAudAttr = { 0 };

static MT_SwitchTrack_RUN_INFO    g_stSwitchTrackRunInfo;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
extern mt_handle      g_hPvrPlayChn;
#endif

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_SwitchTrackMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static void MT_SwitchTrackSetMultiAudAttr(mt_multi_audio_set muti_aud_attr)
{
    g_multiAudAttr = muti_aud_attr;

    return;
}

static void MT_SwitchTrackGetMultiAudAttr(mt_multi_audio_set *muti_aud_attr)
{
    *muti_aud_attr = g_multiAudAttr;

    return;
}

#ifndef MT_SAMPLE_APP
static mt_s32 MT_SwitchTrackCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_TRACK_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_TRACK_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_TRACK_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_SwitchTrackCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_TRACK_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_TRACK_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_SwitchTrackDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_TRACK_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TRACK_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TRACK_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
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
            SAMPLE_TRACK_ERR_PRINT("failed to mt_sys_get_version\n");
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
            SAMPLE_TRACK_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    SAMPLE_TRACK_FUNCTION_EXIT();


    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static MT_VOID MT_SwitchTrackDmxDeInit(MT_VOID)
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
static mt_s32 MT_SwitchTrackAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_TRACK_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_TRACK_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_TRACK_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
static MT_VOID  MT_SwitchTrackAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_TRACK_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_TRACK_ERR_PRINT("=====hWin is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_TRACK_ERR_PRINT("=====hSoundTrack is INVALID_HANDLE ======\n");
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
static mt_s32 MT_SwitchTrackAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo, mt_u32 tracknum)
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
        SAMPLE_TRACK_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == p_ProgInfo)
    {
        SAMPLE_TRACK_ERR_PRINT("p_ProgInfo is NULL!\n");
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
        AudPid  = p_ProgInfo->Audioinfo[tracknum].u16AudioPid;
        u32AudType = p_ProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    SAMPLE_TRACK_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        SAMPLE_TRACK_INFO_PRINT("u32VidType = %#x \n",enVidType);
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_TRACK_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_TRACK_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_TRACK_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_TRACK_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
            SAMPLE_TRACK_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_TRACK_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_TRACK_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
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
static mt_s32 MT_SwitchTrackInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_file_param_t *pstParam = (source_file_param_t *)(args);

    SAMPLE_TRACK_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_TRACK_ERR_PRINT( "\nfile %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_TRACK_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        fclose(pTsFile);
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
        fclose(pTsFile);
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
            SAMPLE_TRACK_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_TRACK_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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

/*
@brief help
@return void
*/

static void MT_SwitchTrackPrintDvbcHelp(char *name)
{
    MT_TRACK_PRINT("\nDvbc Usage:\n");
    MT_TRACK_PRINT("%s -c freq symrate qam\n", name);
    MT_TRACK_PRINT("    freq : \n");
    MT_TRACK_PRINT("    symrate: \n");
    MT_TRACK_PRINT("    qam: \n");
    MT_TRACK_PRINT("example:\n");
    MT_TRACK_PRINT("    %s -c 654 6875 64\n", name);
}

static void MT_SwitchTrackPrintDvbsHelp(char *name)
{
    MT_TRACK_PRINT("\nDvbs Usage:\n");
    MT_TRACK_PRINT("%s -s freq symrate onoff_22k polarization port_type\n", name);
    MT_TRACK_PRINT("    freq : \n");
    MT_TRACK_PRINT("    symrate: \n");
    MT_TRACK_PRINT("    onoff_22k: \n");
    MT_TRACK_PRINT("example:\n");
    MT_TRACK_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}

static void MT_SwitchTrackPrintFileHelp(char *name)
{
    MT_TRACK_PRINT("\nDvbs Usage:\n");
    MT_TRACK_PRINT(" %s -f file\n", name);
    MT_TRACK_PRINT("example:\n");
    MT_TRACK_PRINT("    %s -f ./xxx.ts\n", name);
}
#endif

static void MT_SwitchTrackPrint_help(char *name)
{
#ifndef MT_SAMPLE_APP
    MT_TRACK_PRINT("Lack of parameters\n");
    MT_TRACK_PRINT("\nUsage:\n");
    MT_TRACK_PRINT(" %s\n", name);
    MT_TRACK_PRINT("    -f: path of the subtitle stream file\n");
    MT_TRACK_PRINT("    ./sample_switch_track -f file\n");
    MT_TRACK_PRINT("    -c: DVBC locks frequency\n");
    MT_TRACK_PRINT("    ./sample_switch_track -c freq symrate qam\n");
    MT_TRACK_PRINT("    -s: DVBS locks frequency\n");
    MT_TRACK_PRINT("    %s -s freq symrate onoff_22k polarization port_type\n", name);
    MT_TRACK_PRINT("example:\n");
    MT_TRACK_PRINT("    %s -f ./sub.ts\n", name);
    MT_TRACK_PRINT("    %s -c 654 6875 64\n", name);
    MT_TRACK_PRINT("    %s -s 3840 27500 1 0 0\n", name);
#else
    MT_TRACK_PRINT("    %s -q  <exit> \n", name);
#endif
}

#ifndef MT_SAMPLE_APP
/*
@brief stop to play
@param[in] avplay, Player handle
@return MT_SUCCESS
@return MT_FAlSE
*/
static MT_S32 MT_SwitchTrackStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if(MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_TRACK_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }

    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}
#endif

static mt_s32  MT_SwitchSetMutiAudPidNumAndGropInfo(const PMT_COMPACT_PROG *pProgInfo, mt_u32 new_apid)
{
    mt_u32 loopi = 0;
    mt_multi_audio_set muti_aud_attr;

    for (loopi = 0; loopi < pProgInfo->AElementNum; loopi ++)
    {
        if (pProgInfo->Audioinfo[loopi].u16AudioPid == new_apid){
            break;
        }
    }

    if (loopi == pProgInfo->AElementNum){
        SAMPLE_TRACK_ERR_PRINT("[%s %d], not found cur audio pid=%d, loopi=%d\n", __FUNCTION__, __LINE__, new_apid, loopi);
        return MT_FAILURE;
    }

    MT_SwitchTrackGetMultiAudAttr(&muti_aud_attr);
    if (pProgInfo->AElementNum <= SWITCH_PVR_MAX_AUDIO_PID_NUM)
    {
        muti_aud_attr.cur_apid_indx = 0;
        muti_aud_attr.apid_num = pProgInfo->AElementNum;
        muti_aud_attr.apid_first_part = 1;
    }
    else if (pProgInfo->AElementNum == (SWITCH_PVR_MAX_AUDIO_PID_NUM + 1))
    {
        if (loopi < (SWITCH_PVR_MAX_AUDIO_PID_NUM - 1))
        {
            muti_aud_attr.cur_apid_indx = 0;
            muti_aud_attr.apid_num = (SWITCH_PVR_MAX_AUDIO_PID_NUM - 1);
            muti_aud_attr.apid_first_part = 1;
        }
        else
        {
            muti_aud_attr.cur_apid_indx = (SWITCH_PVR_MAX_AUDIO_PID_NUM - 1);
            muti_aud_attr.apid_num = pProgInfo->AElementNum - (SWITCH_PVR_MAX_AUDIO_PID_NUM - 1);
            muti_aud_attr.apid_first_part = 0;
        }
    }
    else
    {
        if (loopi < (SWITCH_PVR_MAX_AUDIO_PID_NUM))
        {
            muti_aud_attr.cur_apid_indx = 0;
            muti_aud_attr.apid_num = (SWITCH_PVR_MAX_AUDIO_PID_NUM);
            muti_aud_attr.apid_first_part = 1;
        }
        else
        {
            muti_aud_attr.cur_apid_indx = (SWITCH_PVR_MAX_AUDIO_PID_NUM);
            muti_aud_attr.apid_num = pProgInfo->AElementNum - (SWITCH_PVR_MAX_AUDIO_PID_NUM);
            muti_aud_attr.apid_first_part = 0;
        }
    }

    MT_SwitchTrackSetMultiAudAttr(muti_aud_attr);

    return MT_SUCCESS;
}

static mt_s32 MT_SwitchSetAvplayMultiAudio(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    mt_s32 Ret = 0;
    mt_u32                  j;
    MT_UNF_AVPLAY_MULTIAUD_ATTR_S   stMultiAudAttr = {0};
    mt_u32                      AudPids[32] = {0};
    MT_UNF_ACODEC_ATTR_S        stAdecAttr[32] = {0};
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync = {0};
    MT_UNF_AVPLAY_DMX_BUF_FULL_CARE_S  buf_full_set = {0};
    mt_u32 loopi = 0, apid_num = 0;
    mt_multi_audio_set muti_aud_attr;

    Ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(Ret != MT_SUCCESS)
    {
        SAMPLE_TRACK_ERR_PRINT("call MT_UNF_AVPLAY_Stop failed.\n");
        return Ret;
    }

    MT_SwitchTrackGetMultiAudAttr(&muti_aud_attr);
    loopi = muti_aud_attr.cur_apid_indx;
    apid_num = muti_aud_attr.apid_num;
    for(j = 0; j < apid_num; j++)
    {
        stAdecAttr[j].enType = pProgInfo->Audioinfo[loopi%pProgInfo->AElementNum].u32AudioEncType;
        Ret = MTADP_AVPlay_GetAdecAttr(hAvplay,stAdecAttr[j].enType,&stAdecAttr[j]);
        if (Ret != MT_SUCCESS)
        {
            SAMPLE_TRACK_ERR_PRINT("call MTADP_AVPlay_GetAdecAttr failed.\n");
            return Ret;
        }

        AudPids[j] = (mt_u32)(pProgInfo->Audioinfo[loopi%pProgInfo->AElementNum].u16AudioPid);
        SAMPLE_TRACK_INFO_PRINT("%u, AudPids[%u] : %u\n", pProgInfo->AElementNum, j,  AudPids[j]);
        loopi++;
    }

    memset(&stMultiAudAttr, 0, sizeof(MT_UNF_AVPLAY_MULTIAUD_ATTR_S));
    stMultiAudAttr.u32PidNum = apid_num;
    stMultiAudAttr.pu32AudPid = AudPids;
    stMultiAudAttr.pstAcodecAttr = stAdecAttr;
    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_MULTIAUD, &stMultiAudAttr);
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_TRACK_ERR_PRINT("call MT_UNF_AVPLAY_ATTR_ID_MULTIAUD failed.\n");
        return Ret;
    }

    /*
    set all the apid need inserting apts in audio es data, if not, av will not sync when do auido trick
    */
    memset(&DmxAvsync, 0, sizeof(MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S));
    DmxAvsync.AvsyncFlage = 1;
    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_MULTIAUDSYNC, (mt_void*)&DmxAvsync);
    if (MT_SUCCESS != Ret) {
        SAMPLE_TRACK_ERR_PRINT("%s, %d, MT_UNF_AVPLAY_ATTR_ID_DMX_MULTIAUDSYNC failed!!!!, %d\n", __func__, __LINE__, Ret);
    }else{
        SAMPLE_TRACK_INFO_PRINT("%s, %d, MT_UNF_AVPLAY_ATTR_ID_DMX_MULTIAUDSYNC OK\n", __func__, __LINE__);
    }

    /*
    first set all the auido es buf and descriptor buf to buf full not care mode, if not, after the background
    auido buf full, ts data could not put to dmx hw.
    */
    for (j = 0; j < stMultiAudAttr.u32PidNum; j++)
    {
        memset(&buf_full_set, 0, sizeof(buf_full_set));
        buf_full_set.pid = stMultiAudAttr.pu32AudPid[j];
        buf_full_set.es_buf_full = 0;
        buf_full_set.dsc_buf_full = 0;
        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE, &buf_full_set);
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_TRACK_ERR_PRINT("\n##%s %d MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE failed[0x%x].\n", __FUNCTION__, __LINE__, Ret);
            return Ret;
        }
    }

    Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(Ret != MT_SUCCESS)
    {
        SAMPLE_TRACK_ERR_PRINT("call MT_UNF_AVPLAY_Start failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

static MT_S32 MT_SwitchTrackStartTrack(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo, mt_u32 tracknum, mt_bool pvr_playback, mt_handle pvr_playchan)
{
    mt_s32 ret = 0;
    mt_u32 AudPid = 0;
    mt_u32 u32AudType = 0;
    mt_u32 cur_apid = 0x1fff;
    MT_UNF_AVPLAY_DMX_BUF_FULL_CARE_S buf_full_set;
    mt_multi_audio_set multiAudSet;
    mt_u32 cur_apid_first_part;

    if(tracknum < p_ProgInfo->AElementNum )
    {
        AudPid  = p_ProgInfo->Audioinfo[tracknum].u16AudioPid;
        u32AudType = p_ProgInfo->Audioinfo[tracknum].u32AudioEncType;
    }
    else
    {
        AudPid  = p_ProgInfo->AElementPid;
        u32AudType = p_ProgInfo->AudioType;
    }

    if (u32AudType == HA_AUDIO_ID_PCM)
    {
        MTADP_Set_AudPcmInfo(p_ProgInfo->Audioinfo[tracknum].pcm);
    }

    SAMPLE_TRACK_INFO_PRINT("Start AudPid=%x u32AudType = 0x%x pvr_playback=%d\n",  AudPid, u32AudType, pvr_playback);
    if (MT_TRUE ==pvr_playback)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &cur_apid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("call MT_UNF_AVPLAY_ATTR_ID_AUD_PID faild, ret = 0x%x \n", ret);
            return ret;
        }

        memset(&buf_full_set, 0, sizeof(buf_full_set));
        buf_full_set.pid = cur_apid;
        buf_full_set.es_buf_full = 0;
        buf_full_set.dsc_buf_full = 0;
        SAMPLE_TRACK_INFO_PRINT("----set cur_apid=0x%x(%d) buf full not care mode\n",cur_apid, cur_apid);
        /* set current apid channel buf to buf full not care mode */
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE, &buf_full_set);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("config MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE faild, ret = 0x%x \n", ret);
            return ret;
        }

        MT_SwitchTrackGetMultiAudAttr(&multiAudSet);
        cur_apid_first_part = multiAudSet.apid_first_part;

        MT_SwitchSetMutiAudPidNumAndGropInfo(p_ProgInfo, AudPid);
        MT_SwitchTrackGetMultiAudAttr(&multiAudSet);
        if (cur_apid_first_part != multiAudSet.apid_first_part) {
            MT_SwitchSetAvplayMultiAudio(hAvplay, p_ProgInfo);
        }

        ret = MT_UNF_AVPLAY_AudioTrack(hAvplay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("MT_UNF_AVPLAY_AudioTrack!ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_PVR_PlayChangeAudioPid(pvr_playchan,MT_TRUE,u32AudType,AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("MT_UNF_PVR_PlayChangeAudioPid failed!ret = %#x\n",ret);
            return ret;
        }

        // restore ac4    attr info.
        (mt_void)MT_UNF_AVPLAY_StopAudDec(hAvplay);
        MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
        (mt_void)MT_UNF_AVPLAY_StartAudDec(hAvplay);

        /*set the new apid channel es buf to buf full care mode*/
        SAMPLE_TRACK_INFO_PRINT("----set AudPid=0x%x(%d) buf full care mode\n",AudPid,AudPid);
        memset(&buf_full_set, 0, sizeof(buf_full_set));
        buf_full_set.pid = AudPid;
        buf_full_set.es_buf_full = 1;
        buf_full_set.dsc_buf_full = 0;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE, &buf_full_set);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("config MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE faild, ret = 0x%x \n", ret);
            return ret;
        }
    }
    else
    {
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_AudioTrack(hAvplay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("MT_UNF_AVPLAY_AudioTrack!ret = %#x\n",ret);
            return ret;
        }

        // restore ac4    attr info.
        MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
        ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
            return ret;
        }

        //av sync
        //if((INVALID_TSPID != VidPid) || (INVALID_TSPID != AudPid))
        {
        #if 0
            MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
            ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
                SAMPLE_TRACK_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
                return ret;
            }
        #endif
        }
    }

    return MT_SUCCESS;

}


static MT_S32 MT_SwitchTrackStopTrack(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

   if(MT_INVALID_HANDLE == avplay)
   {
       SAMPLE_TRACK_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
       return MT_FAILURE;
   }

   stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
   stopopt.u32TimeoutMs = 0;
   return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);

}

static MT_VOID MT_SwitchTrackPrintMenu(MT_U32 prog_num)
{
#ifndef MT_SAMPLE_APP
    SAMPLE_TRACK_PRINT("\n 1 - %d : select the program \n", prog_num);
#endif
    SAMPLE_TRACK_PRINT("     p : print all track \n");
    SAMPLE_TRACK_PRINT("     s : switch track \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_TRACK_PRINT("     b : background run \n");
#endif
    SAMPLE_TRACK_PRINT("     h : help \n");
    SAMPLE_TRACK_PRINT("     q : quit \n");
    SAMPLE_TRACK_PRINT("Switch_Track>> ");

}


static MT_VOID MT_SwitchTrackPrintAllTrack(PMT_COMPACT_PROG *p_ProgInfo)
{
    mt_u32 i = 0;
    mt_u32 AudPid = 0;
    mt_u32 u32AudType = 0;

    if(NULL == p_ProgInfo)
    {
        SAMPLE_TRACK_ERR_PRINT("p_ProgInfo is NULL!\n");
        return ;
    }

    SAMPLE_TRACK_PRINT(" ========================================\n");

    for (i = 0; i < p_ProgInfo->AElementNum; i++)
    {
        AudPid  = p_ProgInfo->Audioinfo[i].u16AudioPid;
        u32AudType = p_ProgInfo->Audioinfo[i].u32AudioEncType;
        SAMPLE_TRACK_PRINT("Audio Track [%d] Info: \n", i);
        SAMPLE_TRACK_PRINT("\tAudio Stream PID     = 0x%x\n", AudPid);
        switch (u32AudType)
        {
            case HA_AUDIO_ID_PCM:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type PCM\n");
                break;
            case HA_AUDIO_ID_MP2:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type MP2\n");
                break;
            case HA_AUDIO_ID_MP3:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type MP3\n");
                break;
            case HA_AUDIO_ID_AAC:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type AAC\n");
                break;
            case HA_AUDIO_ID_DRA:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type DRA\n");
                break;
            case HA_AUDIO_ID_WMA9STD:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type WMA9STD\n");
                break;
            case HA_AUDIO_ID_DOLBY_PLUS:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type DOLBY_PLUS\n");
                break;
            case HA_AUDIO_ID_DOLBY_TRUEHD:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type DOLBY_TRUEHD\n");
                break;
            case HA_AUDIO_ID_DOLBY_CONVERT:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type DOLBY_CONVERT\n");
                break;
            case HA_AUDIO_ID_DTSHD:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type DTSHD\n");
                break;
            case HA_AUDIO_ID_AC3PASSTHROUGH:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type AC3PASSTHROUGH\n");
                break;
            case HA_AUDIO_ID_EAC3PASSTHROUGH:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type EAC3PASSTHROUGH\n");
                break;
            case HA_AUDIO_ID_DTSPASSTHROUGH:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type DTSPASSTHROUGH\n");
                break;
            case HA_AUDIO_ID_DOLBY_AC4:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type AC4\n");
                break;
            default:
                SAMPLE_TRACK_PRINT("\tAudio Stream Type error\n");
        }
		SAMPLE_TRACK_PRINT("\tLanguage Code        = %s\n", p_ProgInfo->Audioinfo[i].language_code);
		
    }

    SAMPLE_TRACK_PRINT(" ========================================\n");
}



static MT_S32 MT_SwitchTrackAudioType(mt_u32 u32AudType)
{
    switch (u32AudType)
    {
        case HA_AUDIO_ID_PCM:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type PCM\n");
            return 1;
        case HA_AUDIO_ID_MP2:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type MP2\n");
            return 1;
        case HA_AUDIO_ID_MP3:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type MP3\n");
            return 1;
        case HA_AUDIO_ID_AAC:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type AAC\n");
            return 1;
        case HA_AUDIO_ID_DRA:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type DRA\n");
            return 1;
        case HA_AUDIO_ID_WMA9STD:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type WMA9STD\n");
            return 1;
        case HA_AUDIO_ID_DOLBY_PLUS:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type DOLBY_PLUS\n");
            return 0;
        case HA_AUDIO_ID_DOLBY_TRUEHD:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type DOLBY_TRUEHD\n");
            return 0;
        case HA_AUDIO_ID_DOLBY_CONVERT:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type DOLBY_CONVERT\n");
            return 0;
        case HA_AUDIO_ID_DTSHD:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type DTSHD\n");
            return 0;
        case HA_AUDIO_ID_AC3PASSTHROUGH:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type AC3PASSTHROUGH\n");
            return 0;
        case HA_AUDIO_ID_EAC3PASSTHROUGH:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type EAC3PASSTHROUGH\n");
            return 0;
        case HA_AUDIO_ID_DTSPASSTHROUGH:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type DTSPASSTHROUGH\n");
            return 0;
        case HA_AUDIO_ID_DTSM6:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type DTSM6\n");
            return 0;
        case HA_AUDIO_ID_DOLBY_AC4:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type AC4\n");
            return 0;
        default:
            SAMPLE_TRACK_PRINT("\tAudio Stream Type error\n");
        }

    return MT_SUCCESS;
}

static mt_s32 MT_SwitchTrackSetHdmiMode(mt_u32 u32AudType)
{
    mt_s32     ret = MT_SUCCESS;
    MT_UNF_SND_HDMI_MODE_E  hdmiMode;
    MT_UNF_SND_SPDIF_MODE_E  spdifMode;


    if(MT_SwitchTrackAudioType(u32AudType))
    {
        ret = MTADP_SND_GetHdmiMode(&hdmiMode);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_TRACK_ERR_PRINT("get hdmi mode fail.\n");
        }
    
        if (hdmiMode != MT_UNF_SND_HDMI_MODE_LPCM)
        {
            ret = MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, MT_UNF_SND_HDMI_MODE_LPCM);
            if(ret != MT_SUCCESS)
            {
                SAMPLE_TRACK_ERR_PRINT("call MT_UNF_SND_SetHdmiMode failed.\n");
            }
        }
    }
    else
    {
        ret = MTADP_SND_GetHdmiMode(&hdmiMode);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_TRACK_ERR_PRINT("get hdmi mode fail.\n");
        }

        if (hdmiMode != MT_UNF_SND_HDMI_MODE_LPCM)
        {
            ret = MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, MT_UNF_SND_HDMI_MODE_RAW);
            if(ret != MT_SUCCESS)
            {
                SAMPLE_TRACK_ERR_PRINT("call MT_UNF_SND_SetHdmiMode failed.\n");
            }
        }
    }

    if(MT_SwitchTrackAudioType(u32AudType))
    {
        ret = MTADP_SND_GetSpidfMode(&spdifMode);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_TRACK_ERR_PRINT("get spdif mode fail.\n");
        }

        if (spdifMode != MT_UNF_SND_SPDIF_MODE_LPCM)
        {
            ret = MT_UNF_SND_SetSpdifMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_SPDIF0, MT_UNF_SND_SPDIF_MODE_LPCM);
            if(ret != MT_SUCCESS)
            {
                SAMPLE_TRACK_ERR_PRINT("call MT_UNF_SND_SetSpdifMode failed.\n");
            }
        }
    }
    else
    {
        ret = MTADP_SND_GetSpidfMode(&spdifMode);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_TRACK_ERR_PRINT("get spdif mode fail.\n");
        }

        if (spdifMode != MT_UNF_SND_SPDIF_MODE_LPCM)
        {
            ret = MT_UNF_SND_SetSpdifMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_SPDIF0, MT_UNF_SND_SPDIF_MODE_RAW);
            if(ret != MT_SUCCESS)
            {
                SAMPLE_TRACK_ERR_PRINT("call MT_UNF_SND_SetSpdifMode failed.\n");
            }
        }
    }

    return ret;
}
/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@param[in] ppProgTable,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static void MT_SwitchTrackCmdTask(mt_handle hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    mt_s32     ret = MT_SUCCESS;
    MT_CHAR    inputCmd[32] = { 0 };
    mt_s32     tracknum = 0;
#ifdef MT_SAMPLE_APP
    PMT_COMPACT_PROG *stCurrentProgInfo = NULL;
#else
    mt_u32     u32ProgNum = 1;
    PMT_COMPACT_PROG *stCurrentProgInfo = pProgTbl->proginfo;
#endif
    mt_u32 i = 0;
    mt_u32 cur_apid = 0x1fff;


    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_TRACK_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }

    while (1)
    {
        (void)MT_SwitchTrackPrintMenu(pProgTbl->prog_num);
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_TRACK_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_TRACK_INFO_PRINT("switch_track in back!\n");
            break;
        }
#endif
        /* switch track*/
        else if('s' == inputCmd[0])
        {
            MT_BOOL pvr_playback = MT_FALSE;
            mt_handle pvr_playchan = 0;
#ifdef MT_SAMPLE_APP
            ret = MTADP_Get_Current_Info(&stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT("MTADP_Get_Current_Info ERR !!\n");
                continue;
            }

            if(stCurrentProgInfo == NULL)
            {
                    SAMPLE_TRACK_ERR_PRINT("stCurrentProgInfo is NULL!!\n");
                    continue;
            }

            if(g_hPvrPlayChn != MT_INVALID_HANDLE && stCurrentProgInfo->AElementNum > 1 ) //just for test ,need to get pvr playback status
            {
                pvr_playback = MT_TRUE;
                pvr_playchan = g_hPvrPlayChn;

            }
#endif

            //if multi audio get current play pid index
            if(stCurrentProgInfo->AElementNum > 1)
            {
                ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &cur_apid);
                if (MT_SUCCESS == ret)
                {
                    for(i = 0; i< stCurrentProgInfo->AElementNum; i++)
                    {
                        if(cur_apid ==  stCurrentProgInfo->Audioinfo[i].u16AudioPid)
                        {
                            tracknum = i;
                            SAMPLE_TRACK_INFO_PRINT("find current audio track[%d] pid=0x%x\n",i,cur_apid);
                            break;
                        }
                    }
                }
                else
                {
                    SAMPLE_TRACK_ERR_PRINT("call get MT_UNF_AVPLAY_ATTR_ID_AUD_PID faild, ret = 0x%x \n", ret);
                }
             }

            if(pvr_playback == MT_FALSE)
            {
                (void)MT_SwitchTrackStopTrack(hAvplay);
            }

            SAMPLE_TRACK_INFO_PRINT("Switch track-------\n");
            tracknum++;
            if(tracknum >= (stCurrentProgInfo->AElementNum))
            {
                SAMPLE_TRACK_INFO_PRINT("Audio track is max, use first audio track. \n");
                tracknum = 0;
            }

            ret = MT_SwitchTrackSetHdmiMode(stCurrentProgInfo->Audioinfo[tracknum].u32AudioEncType);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT(" MT_SwitchTrackSetHdmiMode failed.\n");
            }

            ret  = MT_SwitchTrackStartTrack(hAvplay, stCurrentProgInfo, tracknum, pvr_playback,pvr_playchan);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT(" Switch failed.\n");
            }
        }
#ifndef MT_SAMPLE_APP
        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                stCurrentProgInfo = pProgTbl->proginfo +((u32ProgNum-1)% pProgTbl->prog_num);

                (void)MT_SwitchTrackStopplay(hAvplay);

                SAMPLE_TRACK_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);

                ret  = MT_SwitchTrackAVPlay_Start(hAvplay, stCurrentProgInfo, 0);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TRACK_ERR_PRINT(" SwitchProg failed.\n");
                }

                (MT_VOID)MT_SwitchTrackPrintAllTrack(stCurrentProgInfo);
            }
            else
            {
                SAMPLE_TRACK_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
                continue;
            }
        }
#endif
        else if('p' == inputCmd[0])
        {
#ifdef MT_SAMPLE_APP
            ret = MTADP_Get_Current_Info(&stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT("MTADP_Get_Current_Info ERR !!\n");
                continue;
            }
#endif
            SAMPLE_TRACK_INFO_PRINT("Print All Track Info: \n");
            (MT_VOID)MT_SwitchTrackPrintAllTrack(stCurrentProgInfo);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_TRACK_INFO_PRINT("Print help info \n");
            continue;
        }

    }


}


static MT_VOID MT_SwitchTrackExit(void)
{
#ifndef MT_SAMPLE_APP
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)MT_SwitchTrackStopplay(g_stSwitchTrackRunInfo.hAvPlay);
    (MT_VOID)MT_SwitchTrackAvplayDeInit(g_stSwitchTrackRunInfo.hAvPlay, g_stSwitchTrackRunInfo.hWin, g_stSwitchTrackRunInfo.hSoundTrack);
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stSwitchTrackRunInfo.pProgTbl);
    (MT_VOID)MTADP_Search_DeInit();
    if(MT_INPUT_SIG_TYPE_FILE == g_stSwitchTrackRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)pthread_join(g_stSwitchTrackRunInfo.htsThd, NULL);
    }
    (MT_VOID)MT_SwitchTrackDmxDeInit();
    (MT_VOID)MTADP_VO_DeInit();
    (MT_VOID)MTADP_Snd_DeInit();
    if(MT_INPUT_SIG_TYPE_FILE != g_stSwitchTrackRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }
#endif
    memset(&g_stSwitchTrackRunInfo, 0xff, sizeof(g_stSwitchTrackRunInfo));
    g_bTaskQuit = MT_TRUE;
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_SwitchTrackParase_args(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;

#ifndef MT_SAMPLE_APP
    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_SwitchTrackPrint_help(argv[0]);
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
                (void)MT_SwitchTrackPrint_help(argv[0]);
            return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_SwitchTrackExit();
                }
                return MT_TASK_EXIT;
#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc < 3)
                {
                    (void)MT_SwitchTrackPrintFileHelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
            return MT_SUCCESS;

            case 's':
                if(argc < 6)
                {
                    (void)MT_SwitchTrackPrintDvbsHelp(argv[0]);
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
                    (void)MT_SwitchTrackPrintDvbcHelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
            return MT_SUCCESS;
#endif
            default:
                (void)MT_SwitchTrackPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}

#ifdef MT_SAMPLE_APP
MT_S32 MT_SwitchTrackMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;
    mt_multi_audio_set multiAudAttr;
#ifndef MT_SAMPLE_APP
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#endif


    ret = MT_SwitchTrackParase_args(argc, argv, &g_stSwitchTrackRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_TRACK_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_TRACK_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        g_bTaskQuit = MT_FALSE;
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }

        if(MT_INPUT_SIG_TYPE_FILE != g_stSwitchTrackRunInfo.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            if(MT_INPUT_SIG_TYPE_CAB == g_stSwitchTrackRunInfo.sInputParam.sig_type)
            {     //dvbc
                ret = MT_SwitchTrackCheckDvbcParam(&g_stSwitchTrackRunInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TRACK_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.cab.freq,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.cab.sym_rate,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_stSwitchTrackRunInfo.sInputParam.sig_type)
            {
                ret = MT_SwitchTrackCheckDvbsParam(&g_stSwitchTrackRunInfo.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TRACK_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.sat.freq,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.sat.sym_rate,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.sat.onoff_22k,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.sat.polarization,
                                        g_stSwitchTrackRunInfo.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }
        }


        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("MTADP_Disp_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_SwitchTrackDmxInit(g_stSwitchTrackRunInfo.sInputParam.sig_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT( "failed to StartDmx\n");
            goto ERR6;
        }


        if(MT_INPUT_SIG_TYPE_FILE == g_stSwitchTrackRunInfo.sInputParam.sig_type)
        {
            ret = pthread_create(&g_stSwitchTrackRunInfo.htsThd, NULL, (void * (*)(void *))MT_SwitchTrackInjectTsTask, &g_stSwitchTrackRunInfo.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TRACK_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }

        (void)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stSwitchTrackRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR9;
        }

        ret = MT_SwitchTrackAVplayInit(&g_stSwitchTrackRunInfo.hAvPlay, &g_stSwitchTrackRunInfo.hWin, &g_stSwitchTrackRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("failed to MT_SwitchTrackAVplayInit\n");
            goto ERR10;
        }

        /* Play the first program on the program list*/
        pstCurrentProgInfo = g_stSwitchTrackRunInfo.pProgTbl->proginfo;
        ret = MT_SwitchTrackAVPlay_Start(g_stSwitchTrackRunInfo.hAvPlay, pstCurrentProgInfo, 0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TRACK_ERR_PRINT("failed to MT_SwitchTrackAVPlay_Start\n");
            goto ERR11;
        }
#endif
    }
#ifdef MT_SAMPLE_APP
    g_stSwitchTrackRunInfo.hAvPlay = avplayHandle.hAvPlay;
#endif

    memset(&multiAudAttr, 0, sizeof(mt_multi_audio_set));
    multiAudAttr.apid_first_part = 0;
    MT_SwitchTrackSetMultiAudAttr(multiAudAttr);
    (mt_void)MT_SwitchTrackCmdTask( g_stSwitchTrackRunInfo.hAvPlay, g_stSwitchTrackRunInfo.pProgTbl);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifndef MT_SAMPLE_APP
    ret = MT_SwitchTrackStopplay(g_stSwitchTrackRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TRACK_ERR_PRINT("MT_SwitchTrackStopplay failed.\n");
    }
    SAMPLE_TRACK_INFO_PRINT("stop to play\n");

ERR11:
    (MT_VOID)MT_SwitchTrackAvplayDeInit(g_stSwitchTrackRunInfo.hAvPlay, g_stSwitchTrackRunInfo.hWin, g_stSwitchTrackRunInfo.hSoundTrack);

ERR10:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stSwitchTrackRunInfo.pProgTbl);

ERR9:
    (MT_VOID)MTADP_Search_DeInit();
ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == g_stSwitchTrackRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        (void)pthread_join(g_stSwitchTrackRunInfo.htsThd, NULL);
    }

ERR7:
    (MT_VOID)MT_SwitchTrackDmxDeInit();

ERR6:
    (MT_VOID)MTADP_VO_DeInit();

ERR5:
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
#endif

ERR2:
    if(MT_INPUT_SIG_TYPE_FILE != g_stSwitchTrackRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }

ERR1:

    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_stSwitchTrackRunInfo, 0, sizeof(g_stSwitchTrackRunInfo));

    return MT_SUCCESS;
}






