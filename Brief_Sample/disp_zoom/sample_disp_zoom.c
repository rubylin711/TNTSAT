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
#include "mt_cmdline.h"


/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DISP_ZOOM_DEBUG

#define MT_DISP_ZOOM_PRINT   printf
#else

#define MT_DISP_ZOOM_PRINT

#endif

#define SAMPLE_DISP_ZOOM_FUNCTION_ENTER()    MT_DISP_ZOOM_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DISP_ZOOM_FUNCTION_EXIT()     MT_DISP_ZOOM_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DISP_ZOOM_FATAL_PRINT(fmt...)         MT_DISP_ZOOM_PRINT(" [FATAL] " fmt)
#define SAMPLE_DISP_ZOOM_ERR_PRINT(fmt...)           MT_DISP_ZOOM_PRINT(" [ERROR] " fmt)
#define SAMPLE_DISP_ZOOM_WARN_PRINT(fmt...)          MT_DISP_ZOOM_PRINT(" [WARN] "  fmt)
#define SAMPLE_DISP_ZOOM_INFO_PRINT(fmt...)          MT_DISP_ZOOM_PRINT(" [INFO] "  fmt)
#define SAMPLE_DISP_ZOOM_DBG_PRINT(fmt...)           MT_DISP_ZOOM_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DISP_ZOOM_PRINT   printf

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
    PMT_COMPACT_TBL  *pProgTbl;
} MT_DispZoom_RUN_INFO;


typedef enum
{
    SAMPLE_DISP_ZOOM_1X = 0,
    SAMPLE_DISP_ZOOM_1_2X,
    SAMPLE_DISP_ZOOM_1_4X,
    SAMPLE_DISP_ZOOM_BUTT,
}SAMPLE_DISP_ZOOM_TYPE;

typedef enum
{
    SAMPLE_DISP_MOVE_NONE = 0,
    SAMPLE_DISP_MOVE_LEFT_TOP,
    SAMPLE_DISP_MOVE_RIGHT_TOP,
    SAMPLE_DISP_MOVE_CENTER,
    SAMPLE_DISP_MOVE_LEFT_BOTTOM,
    SAMPLE_DISP_MOVE_RIGHT_BOTTOM,
    SAMPLE_DISP_MOVE_BUTT,
}SAMPLE_DISP_MOVE_TYPE;


/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_DispZoom_RUN_INFO    g_stDispzoomRunInfo;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif



/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_DispZoomMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

#ifndef MT_SAMPLE_APP
static mt_s32 MT_DispZoomCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_DISP_ZOOM_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_DispZoomCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_DISP_ZOOM_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_DispZoomDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_DISP_ZOOM_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
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
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to mt_sys_get_version\n");
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
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    SAMPLE_DISP_ZOOM_FUNCTION_EXIT();


    return MT_SUCCESS;
}
#endif

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static MT_VOID MT_DispZoomDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}


#ifndef MT_SAMPLE_APP

/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_DispZoomAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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

#endif

/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return void
*/
static void  MT_DispZoomAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("=====hWin is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("=====hSoundTrack is INVALID_HANDLE ======\n");
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
static mt_s32 MT_DispZoomAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
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
        SAMPLE_DISP_ZOOM_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }
    if(NULL == p_ProgInfo)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("p_ProgInfo is NULL!\n");
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

    SAMPLE_DISP_ZOOM_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_DISP_ZOOM_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_DISP_ZOOM_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_DISP_ZOOM_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
            return ret;
        }
    }

    return MT_SUCCESS;
}

#ifndef MT_SAMPLE_APP

/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_DispZoomInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_file_param_t *pstParam = (source_file_param_t *)(args);

    SAMPLE_DISP_ZOOM_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT( "\nfile %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        fclose(pTsFile);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
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
            SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_UNF_DMX_GetTSBuffer  ret= %x \n", ret);
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_DISP_ZOOM_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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
#endif

static mt_s32 MT_DispZoomSetSmallWindow(mt_handle hWin, mt_u32 x, mt_u32 y, mt_u32 w, mt_u32 h)
{
    mt_s32  ret = MT_SUCCESS;
    MT_UNF_WINDOW_ATTR_S pWinAttr;

    ret = MT_UNF_VO_GetWindowAttr(hWin, &pWinAttr);
    if(MT_SUCCESS != ret)
    {
       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_UNF_VO_GetWindowAttr\n");
       return ret;
    }

    pWinAttr.enDisp = MT_UNF_DISPLAY1;
    pWinAttr.stInputRect.s32X = 0;
    pWinAttr.stInputRect.s32Y = 0;
    pWinAttr.stInputRect.s32Width = 0;
    pWinAttr.stInputRect.s32Height = 0;
    pWinAttr.stOutputRect.s32X = x;
    pWinAttr.stOutputRect.s32Y = y;
    pWinAttr.stOutputRect.s32Width = w;
    pWinAttr.stOutputRect.s32Height = h;
    ret = MT_UNF_VO_SetWindowAttr(hWin, &pWinAttr);
    if(MT_SUCCESS != ret)
    {
       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_UNF_VO_SetWindowAttr\n");
       return ret;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_DispZoomSetBigWindow(mt_handle hWin, mt_u32 in_x, mt_u32 in_y, mt_u32 in_w, mt_u32 in_h,
    mt_u32 out_x, mt_u32 out_y, mt_u32 out_w, mt_u32 out_h)
{
     mt_s32  ret = MT_SUCCESS;
    MT_UNF_WINDOW_ATTR_S pWinAttr;

    ret = MT_UNF_VO_GetWindowAttr(hWin, &pWinAttr);
    if(MT_SUCCESS != ret)
    {
       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_UNF_VO_GetWindowAttr\n");
       return ret;
    }

    pWinAttr.enDisp = MT_UNF_DISPLAY1;
    pWinAttr.stInputRect.s32X = in_x;
    pWinAttr.stInputRect.s32Y = in_y;
    pWinAttr.stInputRect.s32Width = in_w;
    pWinAttr.stInputRect.s32Height = in_h;
    pWinAttr.stOutputRect.s32X = out_x;
    pWinAttr.stOutputRect.s32Y = out_y;
    pWinAttr.stOutputRect.s32Width = out_w;
    pWinAttr.stOutputRect.s32Height = out_h;

    ret = MT_UNF_VO_SetWindowAttr(hWin, &pWinAttr);
    if(MT_SUCCESS != ret)
    {
       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_UNF_VO_SetWindowAttr\n");
       return ret;
    }

    return MT_SUCCESS;
}

#ifndef MT_SAMPLE_APP

/*
@brief help
@return void
*/

static void MT_DispZoomPrintDvbcHelp(char *name)
{
    MT_DISP_ZOOM_PRINT("\nDvbc Usage:\n");
    MT_DISP_ZOOM_PRINT(" %s -c freq symrate qam\n", name);
    MT_DISP_ZOOM_PRINT("    freq : \n");
    MT_DISP_ZOOM_PRINT("    symrate: \n");
    MT_DISP_ZOOM_PRINT("    qam: \n");
    MT_DISP_ZOOM_PRINT("example:\n");
    MT_DISP_ZOOM_PRINT("    %s -c 654 6875 64\n", name);
}

static void MT_DispZoomPrintDvbsHelp(char *name)
{
    MT_DISP_ZOOM_PRINT("\nDvbs Usage:\n");
    MT_DISP_ZOOM_PRINT(" %s -s freq symrate onoff_22k polarization port_type\n", name);
    MT_DISP_ZOOM_PRINT("    freq : \n");
    MT_DISP_ZOOM_PRINT("    symrate: \n");
    MT_DISP_ZOOM_PRINT("    onoff_22k: \n");
    MT_DISP_ZOOM_PRINT("example:\n");
    MT_DISP_ZOOM_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}

static void MT_DispZoomPrintFileHelp(char *name)
{
    MT_DISP_ZOOM_PRINT("\nDvbs Usage:\n");
    MT_DISP_ZOOM_PRINT(" %s -f file\n", name);
    MT_DISP_ZOOM_PRINT("example:\n");
    MT_DISP_ZOOM_PRINT("    %s -f ./xxx.ts\n", name);
}
#endif

static void MT_DispZoomPrint_help(char *name)
{
    MT_DISP_ZOOM_PRINT("Lack of parameters\n");
    MT_DISP_ZOOM_PRINT("\nUsage:\n");
    MT_DISP_ZOOM_PRINT("sample_disp_zoom\n");
    MT_DISP_ZOOM_PRINT("    -f: path of the subtitle stream file\n");
    MT_DISP_ZOOM_PRINT("    -c: DVBC locks frequency\n");
    MT_DISP_ZOOM_PRINT("        -c freq symrate qam\n");
    MT_DISP_ZOOM_PRINT("    -s: DVBS locks frequency\n");
    MT_DISP_ZOOM_PRINT("        -s freq symrate onoff_22k polarization port_type\n");
    MT_DISP_ZOOM_PRINT("example:\n");
    MT_DISP_ZOOM_PRINT("    %s -f ./sub.ts\n", name);
    MT_DISP_ZOOM_PRINT("    %s -c 654 6875 64\n", name);
    MT_DISP_ZOOM_PRINT("    %s -s 3840 27500 1 0 0\n", name);
    MT_DISP_ZOOM_PRINT("    %s -q  <exit> \n", name);
}

static void MT_DispZoomPrintMenu(MT_U32 prog_num)
{
#ifndef MT_SAMPLE_APP
    SAMPLE_DISP_ZOOM_PRINT("\n 1 - %d : select the program \n", prog_num);
#endif
    SAMPLE_DISP_ZOOM_PRINT("     s: scale \n");
    SAMPLE_DISP_ZOOM_PRINT("     r: Restore full screen \n");
    SAMPLE_DISP_ZOOM_PRINT("     z: Zoom in to double \n");
    SAMPLE_DISP_ZOOM_PRINT("     m: move the small window \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_DISP_ZOOM_PRINT("     b : background run \n");
#endif
    SAMPLE_DISP_ZOOM_PRINT("     h : help \n");
    SAMPLE_DISP_ZOOM_PRINT("     q : quit \n");
    SAMPLE_DISP_ZOOM_PRINT("Disp_Zoom>> ");

}


/*
@brief stop to play
@param[in] avplay, Player handle
@return MT_SUCCESS
*/
static MT_S32 MT_DispZoomStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if(MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}

static MT_S32 MT_DispZoomGetGreenSize(MT_U32 *width, MT_U32 *height)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_ENC_FMT_E fmt;

    ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &fmt);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("call MT_UNF_DISP_GetFormat fail, ret=0x%x !!! \n", ret);
        return ret;
    }

    switch (fmt)
    {
        case MT_UNF_ENC_FMT_1080P_60 ... MT_UNF_ENC_FMT_1080i_50:
            *width = 1920;
            *height = 1080;
            break;

        case MT_UNF_ENC_FMT_720P_60 ... MT_UNF_ENC_FMT_720P_50:
            *width = 1280;
            *height = 720;
            break;

        case MT_UNF_ENC_FMT_576P_50:
            *width = 720;
            *height = 576;
            break;

        case MT_UNF_ENC_FMT_480P_60:
            *width = 720;
            *height = 480;
            break;

        case MT_UNF_ENC_FMT_PAL ... MT_UNF_ENC_FMT_PAL_Nc:
            *width = 720;
            *height = 576;
            break;

        case MT_UNF_ENC_FMT_NTSC ... MT_UNF_ENC_FMT_NTSC_443:
            *width = 720;
            *height = 480;
            break;

        case MT_UNF_ENC_FMT_SECAM_SIN ... MT_UNF_ENC_FMT_SECAM_COS:
            *width = 720;
            *height = 576;
            break;

        case MT_UNF_ENC_FMT_3840X2160_24 ... MT_UNF_ENC_FMT_3840X2160_60:
            *width = 3840;
            *height = 2160;
            break;

        case MT_UNF_ENC_FMT_4096X2160_24 ... MT_UNF_ENC_FMT_4096X2160_60:
            *width = 4096;
            *height = 2160;
            break;

        default:
            SAMPLE_DISP_ZOOM_INFO_PRINT("Don't find fmt, defualt: w: 0 h: 0 !\n");
            *width = 0;
            *height = 0;
            break;
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
static void MT_DispZoomCmdTask(mt_handle hAvplay, mt_handle hWin, PMT_COMPACT_TBL *pProgTbl)
{
    mt_s32     ret = MT_SUCCESS;
    MT_CHAR    inputCmd[32] = { 0 };
    mt_u32     u32ProgNum = 0;
    MT_CHAR    *pfgetret = NULL;
    SAMPLE_DISP_ZOOM_TYPE   zoomType = SAMPLE_DISP_ZOOM_1X;
    SAMPLE_DISP_MOVE_TYPE   moveType = SAMPLE_DISP_MOVE_NONE;
    PMT_COMPACT_PROG *stCurrentProgInfo = { 0 };
    MT_U32 s_width, s_height;
    MT_U32 x,y,w,h;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return;
    }

    ret = MT_DispZoomGetGreenSize(&s_width, &s_height);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("call MT_DispZoomGetGreenSize fail, ret=0x%x !!!\n", ret);
        return;
    }

    while(1)
    {
        (void)MT_DispZoomPrintMenu(pProgTbl->prog_num);

        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_DISP_ZOOM_INFO_PRINT("exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_DISP_ZOOM_INFO_PRINT("disp_zoom in back!\n");
            break;
        }
#endif
        else if('s' == inputCmd[0])
        {
            zoomType++;
            switch(zoomType)
            {
                case SAMPLE_DISP_ZOOM_1_2X:
                    x = (s_width - s_width / 2) / 2;
                    y = (s_height - s_height / 2) / 2;
                    w = s_width / 2;
                    h = s_height / 2;
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Let's zoom out by 1/2\n");
                    ret = MT_DispZoomSetSmallWindow(hWin, x, y, w, h);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to 1/2\n");
                       break ;
                    }
                    break;
                case SAMPLE_DISP_ZOOM_1_4X:
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Let's zoom out by 1/4\n");

                    x = (s_width - s_width / 4) / 2;
                    y = (s_height - s_height / 4) / 2;
                    w = s_width / 4;
                    h = s_height / 4;
                    ret = MT_DispZoomSetSmallWindow(hWin, x, y, w, h);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to 1/4\n");
                       break ;
                    }
                    break;
                default:
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Display window is too small, reset nomal window.\n");
                    zoomType = SAMPLE_DISP_ZOOM_1X;
                    ret = MT_DispZoomSetSmallWindow(hWin, 0, 0, s_width, s_height);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to 1x\n");
                       break;
                    }
                    break;
            }
        }
        else if('z' == inputCmd[0])
        {
            x = (s_width - s_width / 2) / 2;
            y = (s_height - s_height / 2) / 2;
            w = s_width / 2;
            h = s_height / 2;
            SAMPLE_DISP_ZOOM_INFO_PRINT("Display screen enlarged by 2x\n");

            ret = MT_DispZoomSetBigWindow(hWin, x, y, w, h, 0, 0, 0, 0);
            if(MT_SUCCESS != ret)
            {
               SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetBigWindow\n");
            }
        }
        else if('r' == inputCmd[0])
        {
            SAMPLE_DISP_ZOOM_INFO_PRINT("Restore full screen\n");
            zoomType = SAMPLE_DISP_ZOOM_1X;
            ret = MT_DispZoomSetSmallWindow(hWin, 0, 0, s_width, s_height);
            if(MT_SUCCESS != ret)
            {
               SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to 1x\n");
            }
        }
        else if('m' == inputCmd[0])
        {
            moveType++;
            switch(moveType)
            {
                case SAMPLE_DISP_MOVE_LEFT_TOP:
                    x = 0;
                    y = 0;
                    w = s_width / 4;
                    h = s_height / 4;
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Move the window to the top left corner[x,y,w,h]: %d, %d, %d, %d\n", x, y, w, h);
                    ret = MT_DispZoomSetSmallWindow(hWin, x, y, w, h);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to the top left corner\n");
                       break ;
                    }
                    break;

                case SAMPLE_DISP_MOVE_RIGHT_TOP:
                    x = s_width - s_width / 2;
                    y = 0;
                    w = s_width / 2;
                    h = s_height / 2;
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Move the window to the top right corner[x,y,w,h]: %d, %d, %d, %d\n", x, y, w, h);
                    ret = MT_DispZoomSetSmallWindow(hWin, x, y, w, h);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to the top right corner\n");
                       break ;
                    }
                    break;

                case SAMPLE_DISP_MOVE_CENTER:
                    x = (s_width - s_width / 2) / 2;
                    y = (s_height - s_height / 2) / 2;
                    w = s_width / 2;
                    h = s_height / 2;
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Move the window to the center corner[x,y,w,h]: %d, %d, %d, %d\n", x, y, w, h);
                    ret = MT_DispZoomSetSmallWindow(hWin, x, y, w, h);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to the center corner\n");
                       break ;
                    }
                    break;

                case SAMPLE_DISP_MOVE_LEFT_BOTTOM:
                    x = 0;
                    y = s_height - s_height / 4;
                    w = s_width / 4;
                    h = s_height / 4;
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Move the window to the bottom left corner[x,y,w,h]: %d, %d, %d, %d\n", x, y, w, h);
                    ret = MT_DispZoomSetSmallWindow(hWin, x, y, w, h);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to the bottom left corner\n");
                       break ;
                    }
                    break;

                case SAMPLE_DISP_MOVE_RIGHT_BOTTOM:
                    x = s_width - s_width / 2;
                    y = s_height - s_height / 2;
                    w = s_width / 2;
                    h = s_height / 2;
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Move the window to the bottom right corner[x,y,w,h]: %d, %d, %d, %d\n", x, y, w, h);
                    ret = MT_DispZoomSetSmallWindow(hWin, x, y, w, h);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to the bottom right corner\n");
                       break ;
                    }
                    break;

                default:
                    SAMPLE_DISP_ZOOM_INFO_PRINT("Now reset nomal window.\n");
                    moveType = SAMPLE_DISP_MOVE_NONE;
                    ret = MT_DispZoomSetSmallWindow(hWin, 0, 0, s_width, s_height);
                    if(MT_SUCCESS != ret)
                    {
                       SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomSetSmallWindow to move none\n");
                       break;
                    }
                    break;
            }
        }
        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                stCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                ret = MT_DispZoomStopplay(hAvplay);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISP_ZOOM_ERR_PRINT(" MT_DispZoomStopplay failed.\n");
                }

                SAMPLE_DISP_ZOOM_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);

                ret  = MT_DispZoomAVPlay_Start(hAvplay, stCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISP_ZOOM_ERR_PRINT(" SwitchProg failed.\n");
                }
            }
            else
            {
                SAMPLE_DISP_ZOOM_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DISP_ZOOM_INFO_PRINT("Print help info \n");
            continue;
        }
    }

}

static MT_VOID MT_DispZoomExit(void)
{
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)MT_DispZoomStopplay(g_stDispzoomRunInfo.hAvPlay);
    (MT_VOID)MT_DispZoomAvplayDeInit(g_stDispzoomRunInfo.hAvPlay, g_stDispzoomRunInfo.hWin, g_stDispzoomRunInfo.hSoundTrack);
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDispzoomRunInfo.pProgTbl);
    (MT_VOID)MTADP_Search_DeInit();
    if(MT_INPUT_SIG_TYPE_FILE == g_stDispzoomRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)pthread_join(g_stDispzoomRunInfo.htsThd, NULL);
    }
    (MT_VOID)MT_DispZoomDmxDeInit();
    (MT_VOID)MTADP_VO_DeInit();
    (MT_VOID)MTADP_Snd_DeInit();
    if(MT_INPUT_SIG_TYPE_FILE != g_stDispzoomRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }
    memset(&g_stDispzoomRunInfo, 0xff, sizeof(g_stDispzoomRunInfo));

}



/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_DispZoomParase_args(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;
    while((opt = MTADP_Getopt(argc, argv, "h?Hf:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_DispZoomPrint_help(argv[0]);
            return MT_FAILURE;

#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc < 3)
                {
                    (void)MT_DispZoomPrintFileHelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
            break;

            case 's':
                if(argc < 6)
                {
                    (void)MT_DispZoomPrintDvbsHelp(argv[0]);
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
                    (void)MT_DispZoomPrintDvbcHelp(argv[0]);
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
                    (MT_VOID)MT_DispZoomExit();
                }
                return MT_TASK_EXIT;

            default:
                (void)MT_DispZoomPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DispZoomMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;
#ifndef MT_SAMPLE_APP
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#endif


#ifndef MT_SAMPLE_APP
    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DispZoomPrint_help(argv[0]);
        return MT_SUCCESS;
    }
#endif

    ret = MT_DispZoomParase_args(argc, argv, &g_stDispzoomRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        g_bTaskQuit = MT_FALSE;
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }

        if(MT_INPUT_SIG_TYPE_FILE != g_stDispzoomRunInfo.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_ZOOM_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            if(MT_INPUT_SIG_TYPE_CAB == g_stDispzoomRunInfo.sInputParam.sig_type)
            {     //dvbc
                ret = MT_DispZoomCheckDvbcParam(&g_stDispzoomRunInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISP_ZOOM_ERR_PRINT("Input cab parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            g_stDispzoomRunInfo.sInputParam.input_param.cab.freq,
                                            g_stDispzoomRunInfo.sInputParam.input_param.cab.sym_rate,
                                            g_stDispzoomRunInfo.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_stDispzoomRunInfo.sInputParam.sig_type)
            {
                ret = MT_DispZoomCheckDvbsParam(&g_stDispzoomRunInfo.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISP_ZOOM_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            g_stDispzoomRunInfo.sInputParam.input_param.sat.freq,
                                            g_stDispzoomRunInfo.sInputParam.input_param.sat.sym_rate,
                                            g_stDispzoomRunInfo.sInputParam.input_param.sat.onoff_22k,
                                            g_stDispzoomRunInfo.sInputParam.input_param.sat.polarization,
                                            g_stDispzoomRunInfo.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_ZOOM_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("MTADP_Disp_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_DispZoomDmxInit(g_stDispzoomRunInfo.sInputParam.sig_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT( "failed to MT_DispZoomDmxInit\n");
            goto ERR6;
        }

        if(MT_INPUT_SIG_TYPE_FILE == g_stDispzoomRunInfo.sInputParam.sig_type)
        {
            ret = pthread_create(&g_stDispzoomRunInfo.htsThd, NULL, (void * (*)(void *))MT_DispZoomInjectTsTask, &g_stDispzoomRunInfo.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_ZOOM_ERR_PRINT("failed to pthread_create\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }

        (void)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stDispzoomRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR9;
        }

        ret = MT_DispZoomAVplayInit(&g_stDispzoomRunInfo.hAvPlay, &g_stDispzoomRunInfo.hWin, &g_stDispzoomRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_DispZoomAVplayInit\n");
            goto ERR10;
        }

        /* Play the first program on the program list*/
        pstCurrentProgInfo = g_stDispzoomRunInfo.pProgTbl->proginfo;
        ret = MT_DispZoomAVPlay_Start(g_stDispzoomRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_ZOOM_ERR_PRINT("failed to MT_DispZoomAVPlay_Start\n");
            goto ERR11;
        }
#endif

#ifdef MT_SAMPLE_APP
        g_stDispzoomRunInfo.hAvPlay = avplayHandle.hAvPlay;
        g_stDispzoomRunInfo.hWin = avplayHandle.hWin;
#endif
    }

    (mt_void)MT_DispZoomCmdTask(g_stDispzoomRunInfo.hAvPlay, g_stDispzoomRunInfo.hWin, g_stDispzoomRunInfo.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifndef MT_SAMPLE_APP
    ret = MT_DispZoomStopplay(g_stDispzoomRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_ZOOM_ERR_PRINT("MT_DispZoomStopplay failed.\n");
    }
    SAMPLE_DISP_ZOOM_INFO_PRINT("stop to play\n");

ERR11:
    (MT_VOID)MT_DispZoomAvplayDeInit(g_stDispzoomRunInfo.hAvPlay, g_stDispzoomRunInfo.hWin, g_stDispzoomRunInfo.hSoundTrack);


ERR10:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDispzoomRunInfo.pProgTbl);


ERR9:
    (MT_VOID)MTADP_Search_DeInit();

ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == g_stDispzoomRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)pthread_join(g_stDispzoomRunInfo.htsThd, NULL);
    }

ERR7:
    (MT_VOID)MT_DispZoomDmxDeInit();

ERR6:
   (MT_VOID)MTADP_VO_DeInit();

ERR5:
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:
    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR2:
    if(MT_INPUT_SIG_TYPE_FILE != g_stDispzoomRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif

    g_bTaskQuit = MT_TRUE;
    memset(&g_stDispzoomRunInfo, 0xff, sizeof(g_stDispzoomRunInfo));
    return MT_SUCCESS;
}
