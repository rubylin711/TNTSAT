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
#include "mt_mpi_demux.h"
#include "mt_adp_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"


/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DVB_PLAY_DEBUG

#define MT_DVB_PLAY_PRINT   printf
#else

#define MT_DVB_PLAY_PRINT

#endif

#define SAMPLE_DVB_PLAY_FUNCTION_ENTER()    MT_DVB_PLAY_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DVB_PLAY_FUNCTION_EXIT()     MT_DVB_PLAY_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DVB_PLAY_FATAL_PRINT(fmt...)         MT_DVB_PLAY_PRINT(" [FATAL] " fmt)
#define SAMPLE_DVB_PLAY_ERR_PRINT(fmt...)           MT_DVB_PLAY_PRINT(" [ERROR] " fmt)
#define SAMPLE_DVB_PLAY_WARN_PRINT(fmt...)          MT_DVB_PLAY_PRINT(" [WARN] "  fmt)
#define SAMPLE_DVB_PLAY_INFO_PRINT(fmt...)          MT_DVB_PLAY_PRINT(" [INFO] "  fmt)
#define SAMPLE_DVB_PLAY_DBG_PRINT(fmt...)           MT_DVB_PLAY_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DVB_PLAY_PRINT   printf

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

typedef struct
{
    mt_u32 VidPid;
    MT_UNF_VCODEC_TYPE_E VdecType;
    mt_u32 AudPid;
    mt_s32 AdecType;
    mt_u32 PcrPid;
}mt_dvb_play_t;



typedef struct
{
    MT_INPUR_SIG_TYPE_T sig_type;
    mt_dvb_play_t dvb_play;
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
} MT_Dvbplay_RUN_INFO;





/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_Dvbplay_RUN_INFO    g_stDvbplayRunInfo;




/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbplayMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_DvbplayCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_DVB_PLAY_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_DvbplayCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_DVB_PLAY_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


static MT_U32 MT_Dvbplay_Parse_Type(MT_S32 mtatype)
{
    MT_U32 atype = 0;
    switch (mtatype)
    {
    case 0:
        atype = HA_AUDIO_ID_PCM;
        break;
    case 1:
        atype = HA_AUDIO_ID_MP2;
        break;
    case 2:
        atype = HA_AUDIO_ID_MP2;
        break;
    case 3:
        atype = HA_AUDIO_ID_MP3;
        break;
    case 4:
        atype = HA_AUDIO_ID_DOLBY_TRUEHD;
        break;
    case 5:
        atype = HA_AUDIO_ID_DOLBY_PLUS;
        break;
    case 6:
        atype = HA_AUDIO_ID_AAC;
        break;
    case 8:
        atype = HA_AUDIO_ID_DRA;
        break;
    case 9:
        atype = HA_AUDIO_ID_OGG;
        break;
    case 10:
        atype = HA_AUDIO_ID_OPUS;
        break;
    case 11:
        atype = HA_AUDIO_ID_FLAC;
        break;
    case 12:
        atype = HA_AUDIO_ID_APE;
        break;
    case 13:
        atype = HA_AUDIO_ID_VORBIS;
        break;
    case 107:
        atype = HA_AUDIO_ID_EAC3PASSTHROUGH;
        break;
    case 108:
        atype = HA_AUDIO_ID_DOLBY_CONVERT;
        break;
    case 109:
        atype = HA_AUDIO_ID_AC3PASSTHROUGH;
        break;
    case 110:
        atype = HA_AUDIO_ID_EAC3PASSTHROUGH;
        break;
    case 111:
        atype = HA_AUDIO_ID_DTSPASSTHROUGH;
        break;
    default:
        MT_DVB_PLAY_PRINT("wrong params or unsupport audio type\n");
        break;
    }
    return atype;
}


/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_DvbplayDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_DVB_PLAY_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }



    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    s32Ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("failed to mt_sys_get_version\n");
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
        SAMPLE_DVB_PLAY_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        return MT_FAILURE;
    }


    SAMPLE_DVB_PLAY_FUNCTION_EXIT();


    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static MT_VOID MT_DvbplayDmxDeInit(MT_VOID)
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
static mt_s32 MT_DvbplayAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
static void  MT_DvbplayAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("=====hWin is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("=====hSoundTrack is INVALID_HANDLE ======\n");
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



static mt_s32 MT_Dvbplay_Bypid(mt_handle hAvplay, mt_u32 VidPid, MT_UNF_VCODEC_TYPE_E VdecType, mt_u32 AudPid, mt_s32 AdecType, mt_u32 PcrPid)
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_MEDIA_CHAN_E chan_media = 0;
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync = {0};
    MT_UNF_VCODEC_ATTR_S VdecAttr = {0};
    MT_UNF_SYNC_ATTR_S   stSyncAttr = { 0 };
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };

    AdecType = MT_Dvbplay_Parse_Type(AdecType);

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        DmxAvsync.VdecType = VdecType;
        DmxAvsync.AdecType = (mt_u32)AdecType;
        DmxAvsync.AvsyncFlage = 1;
        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
            return MT_FAILURE;
        }
    }

    if(VidPid != INVALID_TSPID)
    {
        Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed:%#x\n", Ret);
            return Ret;
        }

        VdecAttr.enType = VdecType;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.u32Priority = 3;

        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
            return Ret;
        }
        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("call MTADP_AVPlay_SetVdecAttr failed.\n");
            return Ret;
        }

        chan_media |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;

        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr PcrPid failed.\n");
            return Ret;
        }

    }

    if(AudPid != INVALID_TSPID)
    {
        Ret = MTADP_AVPlay_SetAdecAttr(hAvplay, AdecType, HD_DEC_MODE_RAWPCM, 1);
        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n", Ret);
            return Ret;
        }

        chan_media |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;

    }

    if((chan_media & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (chan_media & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("Set frame to VO is failed, ret = %x\n", Ret);
            return Ret;
        }

        /** Get synchronization properties of AV player */
        Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("Get avplay sync attr is failed, ret = %x\n", Ret);
            return Ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.stSyncStartRegion.bSmoothPlay = MT_TRUE;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("Set avplay sync attr is failed, ret = %x\n", Ret);
            return Ret;
        }
    }


    Ret = MT_UNF_AVPLAY_Start(hAvplay, chan_media, MT_NULL);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("call MT_UNF_AVPLAY_Start failed.\n");
        return Ret;
    }
    SAMPLE_DVB_PLAY_INFO_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    SAMPLE_DVB_PLAY_INFO_PRINT("--------------Play start---------------------------\n");
    SAMPLE_DVB_PLAY_INFO_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    return MT_SUCCESS;
}





static void MT_DvbplayPrint_help(char *name)
{
    MT_DVB_PLAY_PRINT("Lack of parameters\n");
    MT_DVB_PLAY_PRINT("\nUsage:\n");
    MT_DVB_PLAY_PRINT("sample_disp_zoom\n");
    MT_DVB_PLAY_PRINT("    -c: DVBC locks frequency\n");
    MT_DVB_PLAY_PRINT("        -c freq symrate qam vpid vtype,apid,atype,pcrpid \n");
    MT_DVB_PLAY_PRINT("    -s: DVBS locks frequency\n");
    MT_DVB_PLAY_PRINT("        -s freq symrate onoff_22k polarization port_type vpid vtype,apid,atype,pcrpid \n");
    MT_DVB_PLAY_PRINT("example:\n");
    MT_DVB_PLAY_PRINT("    %s -c 314 6875 64 82 0 83 3 82 \n", name);
    MT_DVB_PLAY_PRINT("    %s -s 3840 27500 1 0 0 514 0 670 3 514 \n", name);
    MT_DVB_PLAY_PRINT("    %s -q  <exit> \n", name);
}

static void MT_DvbplayPrintMenu(void)
{

#ifdef MT_SAMPLE_APP
    SAMPLE_DVB_PLAY_PRINT("     b : background run \n");
#endif
    SAMPLE_DVB_PLAY_PRINT("     h : help \n");
    SAMPLE_DVB_PLAY_PRINT("     q : quit \n");
    SAMPLE_DVB_PLAY_PRINT("Pid_play>> ");

}


/*
@brief stop to play
@param[in] avplay, Player handle
@return MT_SUCCESS
*/
static MT_S32 MT_DvbplayStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if(MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}


/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@param[in] ppProgTable,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static void MT_DvbplayCmdTask(mt_handle hAvplay, mt_handle hWin)
{
    MT_CHAR    inputCmd[32] = { 0 };
    MT_CHAR    *pfgetret = NULL;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }

    while(1)
    {
        (void)MT_DvbplayPrintMenu();

        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_DVB_PLAY_INFO_PRINT("exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_DVB_PLAY_INFO_PRINT("disp_zoom in back!\n");
            break;
        }
#endif

        else if('h' == inputCmd[0])
        {
            SAMPLE_DVB_PLAY_INFO_PRINT("Print help info \n");
            continue;
        }
    }

}

static MT_VOID MT_DvbplayExit(void)
{
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)MT_DvbplayStopplay(g_stDvbplayRunInfo.hAvPlay);
    (MT_VOID)MT_DvbplayAvplayDeInit(g_stDvbplayRunInfo.hAvPlay, g_stDvbplayRunInfo.hWin, g_stDvbplayRunInfo.hSoundTrack);


    (MT_VOID)MT_DvbplayDmxDeInit();
    (MT_VOID)MTADP_VO_DeInit();
    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);


    memset(&g_stDvbplayRunInfo, 0, sizeof(g_stDvbplayRunInfo));

}



/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_DvbplayParase_args(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;
    while((opt = MTADP_Getopt(argc, argv, "h?H:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_DvbplayPrint_help(argv[0]);
            return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DvbplayExit();
                }
                return MT_TASK_EXIT;


            case 's':
                if(argc != 12)
                {
                    (void)MT_DvbplayPrint_help(argv[0]);
                    return MT_FAILURE;
                }

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_SAT;

                pInputParam->input_param.sat.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.sat.sym_rate = strtol(argv[3], 0, 0);
                pInputParam->input_param.sat.onoff_22k = strtol(argv[4], 0, 0);
                pInputParam->input_param.sat.polarization = strtol(argv[5], 0, 0);
                pInputParam->input_param.sat.port_type = strtol(argv[6], 0, 0);

                pInputParam->dvb_play.VidPid = strtol(argv[7], 0, 0);
                pInputParam->dvb_play.VdecType = strtol(argv[8], 0, 0);
                pInputParam->dvb_play.AudPid = strtol(argv[9], 0, 0);
                pInputParam->dvb_play.AdecType = strtol(argv[10], 0, 0);
                pInputParam->dvb_play.PcrPid = strtol(argv[11], 0, 0);

            return MT_SUCCESS;

            case 'c':
                if(argc != 10)
                {
                    (void)MT_DvbplayPrint_help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                pInputParam->dvb_play.VidPid = strtol(argv[5], 0, 0);
                pInputParam->dvb_play.VdecType = strtol(argv[6], 0, 0);
                pInputParam->dvb_play.AudPid = strtol(argv[7], 0, 0);
                pInputParam->dvb_play.AdecType = strtol(argv[8], 0, 0);
                pInputParam->dvb_play.PcrPid = strtol(argv[9], 0, 0);
            return MT_SUCCESS;


            default:
                (void)MT_DvbplayPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbplayMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;

    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DvbplayPrint_help(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_DvbplayParase_args(argc, argv, &g_stDvbplayRunInfo.sInputParam);
    if(MT_FAILURE == ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }
#endif
        g_bTaskQuit = MT_FALSE;

        ret = MTADP_Fe_Init(TUNER_ID_0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR1;
        }

        if(MT_INPUT_SIG_TYPE_CAB == g_stDvbplayRunInfo.sInputParam.sig_type)
        {     //dvbc
            ret = MT_DvbplayCheckDvbcParam(&g_stDvbplayRunInfo.sInputParam.input_param.cab);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVB_PLAY_ERR_PRINT("Input cab parameter error!\n");
                goto ERR2;
            }
            ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        g_stDvbplayRunInfo.sInputParam.input_param.cab.freq,
                                        g_stDvbplayRunInfo.sInputParam.input_param.cab.sym_rate,
                                        g_stDvbplayRunInfo.sInputParam.input_param.cab.mod_type);
        }
        else if(MT_INPUT_SIG_TYPE_SAT == g_stDvbplayRunInfo.sInputParam.sig_type)
        {
            ret = MT_DvbplayCheckDvbsParam(&g_stDvbplayRunInfo.sInputParam.input_param.sat);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVB_PLAY_ERR_PRINT("Input sat parameter error!\n");
                goto ERR2;
            }
            ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        g_stDvbplayRunInfo.sInputParam.input_param.sat.freq,
                                        g_stDvbplayRunInfo.sInputParam.input_param.sat.sym_rate,
                                        g_stDvbplayRunInfo.sInputParam.input_param.sat.onoff_22k,
                                        g_stDvbplayRunInfo.sInputParam.input_param.sat.polarization,
                                        g_stDvbplayRunInfo.sInputParam.input_param.sat.port_type);
        }

        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR2;
        }


#ifndef MT_SAMPLE_APP
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_1080i_50);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_1080i_50);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("MTADP_Disp_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            goto ERR3;
        }
#endif
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_DvbplayDmxInit(g_stDvbplayRunInfo.sInputParam.sig_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT( "failed to MT_DvbplayDmxInit\n");
            goto ERR6;
        }

        ret = MT_DvbplayAVplayInit(&g_stDvbplayRunInfo.hAvPlay, &g_stDvbplayRunInfo.hWin, &g_stDvbplayRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("failed to MT_DvbplayAVplayInit\n");
            goto ERR7;
        }

        ret = MT_Dvbplay_Bypid(g_stDvbplayRunInfo.hAvPlay, g_stDvbplayRunInfo.sInputParam.dvb_play.VidPid, g_stDvbplayRunInfo.sInputParam.dvb_play.VdecType, g_stDvbplayRunInfo.sInputParam.dvb_play.AudPid, g_stDvbplayRunInfo.sInputParam.dvb_play.AdecType, g_stDvbplayRunInfo.sInputParam.dvb_play.PcrPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVB_PLAY_ERR_PRINT("failed to MT_Dvbplay_Bypid\n");
            goto ERR8;
        }

    }
    (mt_void)MT_DvbplayCmdTask(g_stDvbplayRunInfo.hAvPlay, g_stDvbplayRunInfo.hWin);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
    ret = MT_DvbplayStopplay(g_stDvbplayRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVB_PLAY_ERR_PRINT("MT_DvbplayStopplay failed.\n");
    }
    SAMPLE_DVB_PLAY_INFO_PRINT("stop to play\n");

ERR8:
    (MT_VOID)MT_DvbplayAvplayDeInit(g_stDvbplayRunInfo.hAvPlay, g_stDvbplayRunInfo.hWin, g_stDvbplayRunInfo.hSoundTrack);

ERR7:
    (MT_VOID)MT_DvbplayDmxDeInit();


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

    (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
ERR1:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_stDvbplayRunInfo, 0, sizeof(g_stDvbplayRunInfo));

    return MT_SUCCESS;
}
