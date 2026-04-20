/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "mt_type.h"
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_adp_demux.h"
#include "mt_adp_frontend.h"
#include "mt_unf_subtouput.h"
#include "mt_adp_mpi.h"
#include "mt_unf_sound.h"
#include "mt_unf_subt.h"
#include "sample_subtitle_out.h"
#include "sample_subtitle_data.h"
#include "mt_adp_hdmi.h"
#include "mt_cmdline.h"
#include "mt_adp_mpi.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_SUBT_DEBUG

#define MT_SUBT_PRINT   printf
#else

#define MT_SUBT_PRINT

#endif


#define SAMPLE_SUBT_FUNCTION_ENTER()    MT_SUBT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SUBT_FUNCTION_EXIT()     MT_SUBT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_SUBT_FATAL_PRINT(fmt...)             MT_SUBT_PRINT(" [FATAL] " fmt)
#define SAMPLE_SUBT_ERR_PRINT(fmt...)           MT_SUBT_PRINT(" [ERROR] " fmt)
#define SAMPLE_SUBT_WARN_PRINT(fmt...)          MT_SUBT_PRINT(" [WARN] "  fmt)
#define SAMPLE_SUBT_INFO_PRINT(fmt...)          MT_SUBT_PRINT(" [INFO] "  fmt)
#define SAMPLE_SUBT_DBG_PRINT(fmt...)           MT_SUBT_PRINT(" [DEBUG] " fmt)

#define DMX_ID_0 0
#define TUNER_ID_0 (0)
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2

#define PAGE_TIMEOUT_OUT (5000) //5s

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
    MT_U32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    MT_U32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
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
    MT_U32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    MT_U32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
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
    MT_HANDLE          hUnfSubt;
    MT_HANDLE          hUnfSO;
    MT_HANDLE          hSubtOut;
    pthread_t          usrRecvThread;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
} MT_CC_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_HANDLE g_hUnfSubt;
static MT_HANDLE g_hUnfSO;
static MT_HANDLE g_hSubtOut;

static MT_BOOL g_bTaskQuit = MT_TRUE;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

static MT_CC_RUN_INFO subt_run_info;

static MT_UNF_SUBT_DATA_TYPE_E g_enSubtType = MT_UNF_SUBT_BUTT;
/* ref iso-8859-1 subtitle language */
static MT_U8 g_LangChTable[16][16] =
{
        /* 0    1    2    3    4    5    6    7     8    9    A    B    C    D    E    F */
        /*0*/ { ' ', ' ', ' ', '0', '@', 'P', '`', 'p', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*1*/ { ' ', ' ', '!', '1', 'A', 'Q', 'a', 'q', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*2*/ { ' ', ' ', '"', '2', 'B', 'R', 'b', 'r', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*3*/ { ' ', ' ', '#', '3', 'C', 'S', 'c', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*4*/ { ' ', ' ', '$', '4', 'D', 'T', 'd', 't', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*5*/ { ' ', ' ', '%', '5', 'E', 'U', 'e', 'u', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*6*/ { ' ', ' ', '&', '6', 'F', 'V', 'f', 'v', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*7*/ { ' ', ' ', '\'', '7', 'G', 'W', 'g', 'w', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*8*/ { ' ', ' ', '(', '8', 'H', 'X', 'h', 'x', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*9*/ { ' ', ' ', ')', '9', 'I', 'Y', 'i', 'y', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*A*/ { ' ', ' ', '*', ':', 'J', 'Z', 'j', 'z', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*B*/ { ' ', ' ', '+', ';', 'K', '[', 'k', '{', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*C*/ { ' ', ' ', ',', '<', 'L', '\\', 'l', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*D*/ { ' ', ' ', '-', '=', 'M', ']', 'm', '}', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*E*/ { ' ', ' ', '.', '>', 'N', '^', 'n', '~', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

        /*F*/ { ' ', ' ', '/', '?', 'O', '_', 'o', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }
    };

/******************************* API declaration *****************************/
static MT_VOID MT_SubtExit(void);
#ifdef MT_SAMPLE_APP
MT_S32 MT_SubtMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

#ifndef MT_SAMPLE_APP
static MT_S32 MT_SubtCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_SUBT_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_SUBT_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_SUBT_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static MT_S32 MT_SubtCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_SUBT_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_SUBT_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_SubtDmxInit(mt_input_para_t *pInputParam)
{
    MT_S32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_SUBT_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (MT_VOID) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == pInputParam->sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            (MT_VOID) MT_UNF_DMX_DeInit();
            return MT_FAILURE;
        }
    }
    else
    {
         memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to mt_sys_get_version\n");
            return MT_FAILURE;
        }

        if(MT_INPUT_SIG_TYPE_CAB == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
            else
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
            }
        }
        else if(MT_INPUT_SIG_TYPE_SAT == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
            }
            else
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
        }
    }

    SAMPLE_SUBT_FUNCTION_EXIT();


    return MT_SUCCESS;
}


/*****************************************************************************
 @brief DmxDeinit and detachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*****************************************************************************/
static MT_VOID MT_SubtDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
    (MT_VOID)MT_UNF_DMX_DeInit();
}

/*****************************************************************************
 @brief Audio and video playback init
 @param[out] hTrack Pointer to the outgoing AVPLAY handle
 @param[out] hWindow Pointer to the outgoing Window handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*****************************************************************************/
static MT_S32 MT_SubtAvPlayInit(MT_HANDLE *hTrack, MT_HANDLE *hWindow)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_HANDLE   hWin = MT_INVALID_HANDLE;
    MT_HANDLE   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AVPLAY_ATTR_S stAvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == hTrack)
    {
        SAMPLE_SUBT_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == hWindow)
    {
        SAMPLE_SUBT_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }

    s32Ret = MTADP_AVPlay_RegADecLib();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MTADP_AVPlay_RegADecLib\n");
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_AVPLAY_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_AVPLAY_Init\n");
        goto ERR0;
    }

    memset(&stAvplayAttr, 0 ,sizeof(stAvplayAttr));
    s32Ret = MT_UNF_AVPLAY_GetDefaultConfig(&stAvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_AVPLAY_GetDefaultConfig\n");
        goto ERR0;
    }

    stAvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    stAvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    s32Ret = MT_UNF_AVPLAY_Create(&stAvplayAttr, &subt_run_info.hAvPlay);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_AVPLAY_Create\n");
        goto ERR1;
    }

    s32Ret = MT_UNF_AVPLAY_ChnOpen(subt_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_AVPLAY_ChnOpen\n");
        goto ERR1;
    }

    s32Ret = MT_UNF_AVPLAY_ChnOpen(subt_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_AVPLAY_ChnOpen\n");
        goto ERR1;
    }

    s32Ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MTADP_VO_CreatWin\n");
        goto ERR1;
    }

    s32Ret = MT_UNF_VO_AttachWindow(hWin, subt_run_info.hAvPlay);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_VO_AttachWindow\n");
        goto ERR2;
    }
    s32Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_VO_SetWindowEnable\n");
        goto ERR2;
    }

    s32Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR2;
    }
    s32Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hsoundTrack);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SND_CreateTrack\n");
        goto ERR3;
    }

    s32Ret = MT_UNF_SND_Attach(hsoundTrack, subt_run_info.hAvPlay);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SND_Attach\n");
        goto ERR3;
    }

    *hWindow = hWin;
    *hTrack = hsoundTrack;

    return MT_SUCCESS;

ERR3:
    (MT_VOID) MT_UNF_AVPLAY_DeInit();
ERR2:
    (MT_VOID) MT_UNF_VO_DestroyWindow(hWin);
ERR1:
    (MT_VOID) MTADP_VO_DeInit();
ERR0:
    (MT_VOID) MT_UNF_AVPLAY_Destroy(subt_run_info.hAvPlay);

    return MT_FAILURE;
}

/*****************************************************************************
 @brief Audio and video playback Deinit
 @param[in] hTrack  A pointer to the Window handle passed in
 @param[in] hWin A pointer to the SoundTrack handle passed in
 @return ::(MT_VOID)
*****************************************************************************/
static MT_VOID  MT_SubtAvPlayDeInit(MT_HANDLE hTrack, MT_HANDLE hWindow)
{

    if(MT_INVALID_HANDLE == hTrack)
    {
        SAMPLE_SUBT_ERR_PRINT("phWin is null.\n");

    }

    if(MT_INVALID_HANDLE == hWindow)
    {
        SAMPLE_SUBT_ERR_PRINT("phSoundTrack is null.\n");

    }

    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWindow, MT_FALSE);
    (MT_VOID)MT_UNF_VO_DetachWindow(hWindow, subt_run_info.hAvPlay);
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWindow);
    (MT_VOID)MT_UNF_SND_Detach(hTrack, subt_run_info.hAvPlay);
    (MT_VOID)MT_UNF_SND_DestroyTrack(hTrack);
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(subt_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(subt_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    (MT_VOID)MT_UNF_AVPLAY_Destroy(subt_run_info.hAvPlay);
    (MT_VOID)MT_UNF_AVPLAY_DeInit();
}



/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error
@*/
static MT_S32 MT_SubtSetAvplayPidAndCodecType(MT_HANDLE hAvplay      , const PMT_COMPACT_PROG *pProgInfo)
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
        SAMPLE_SUBT_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
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

    SAMPLE_SUBT_INFO_PRINT("VidPid = %#x, AudPid = %#x-------<%s> line: %d\n", VidPid, AudPid, __FUNCTION__, __LINE__);

    if(VidPid != INVALID_TSPID)
    {
        /** Get the video properties of the AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_SUBT_ERR_PRINT("Set video properties or video PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_SUBT_ERR_PRINT("Setting the decoding mode or audio PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_SUBT_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
static MT_S32 MT_SubtStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        s32Ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    if(MT_INVALID_HANDLE == hAvplay || NULL == pProgInfo)
    {
        SAMPLE_SUBT_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    /** Set the PID of the AV player and set the encoder type */
    MT_SubtSetAvplayPidAndCodecType(hAvplay, pProgInfo);

    /** Get the audio PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_SUBT_ERR_PRINT("Has no audio stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    /** Get the video PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_SUBT_ERR_PRINT("Has no video stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_SUBT_ERR_PRINT("Set frame to VO is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Get synchronization properties of AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("Get avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_SUBT_ERR_PRINT("Set avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    s32Ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("MT_UNF_AVPLAY_Start failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    return MT_SUCCESS;
}


/**********************************************************************************
@brief stop to play
@return MT_VOID
**********************************************************************************/
static MT_VOID MT_SubtStopplay(MT_VOID)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    (MT_VOID)MT_UNF_AVPLAY_Stop(subt_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);

}
#endif

/**********************************************************************************
@brief Get language by subtitle code, per language composed of three subtitle codec
@param[in] u32LangCode The LangCode
@param[in] pu8Buf  The Buf
@param[in] u8BufLen The BufLen
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtGetLangStr(MT_U32 u32LangCode, MT_U8 *pu8Buf, MT_U8 u8BufLen)
{
    MT_U8 u8FirstNibble = 0;
    MT_U8 u8SecondNibble = 0;

    if (pu8Buf == MT_NULL || u8BufLen < 3)
    {
        return MT_FAILURE;
    }

    u8FirstNibble = ((u32LangCode >> 16)&0xf0) >> 4;
    u8SecondNibble = (u32LangCode >> 16)&0x0f;

    if (u8FirstNibble > 0xf || u8SecondNibble > 0xf)
    {
        return MT_FAILURE;
    }

    pu8Buf[0] = g_LangChTable[u8SecondNibble][u8FirstNibble];

    u8FirstNibble = ((u32LangCode >> 8)&0xf0) >> 4;
    u8SecondNibble = (u32LangCode >> 8)&0x0f;

    if (u8FirstNibble > 0xf || u8SecondNibble > 0xf)
    {
        return MT_FAILURE;
    }

    pu8Buf[1] = g_LangChTable[u8SecondNibble][u8FirstNibble];

    u8FirstNibble = ((u32LangCode)&0xf0) >> 4;
    u8SecondNibble = (u32LangCode)&0x0f;

    if (u8FirstNibble > 0xf || u8SecondNibble > 0xf)
    {
        return MT_FAILURE;
    }

    pu8Buf[2] = g_LangChTable[u8SecondNibble][u8FirstNibble];

    return MT_SUCCESS;
}

/**********************************************************************************
@brief Gets the current playback time
@param[in] u32UserData UserData
@param[out] ps64CurrentPts pre time
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
#ifdef CONFIG_MT_CHIP_SYMPHONY4
static MT_S32 MT_SubtGetCurPts(mt_u32 u32UserData, mt_s64 *ps64CurrentPts)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static MT_S32 MT_SubtGetCurPts(ulong u32UserData, mt_s64 *ps64CurrentPts)
#endif
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STATUS_INFO_S stStatusInfo = { { 0 } };
    MT_HANDLE hAvplayer = (MT_HANDLE)u32UserData;
    static MT_U32 u32PrePtsTime = 0;

    s32Ret = MT_UNF_AVPLAY_GetStatusInfo(hAvplayer, &stStatusInfo);
    if (s32Ret != MT_SUCCESS)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_AVPLAY_GetStatusInfo\n");
    }

    *ps64CurrentPts = stStatusInfo.stSyncStatus.u64LastVidPts / 1000;
    if (*ps64CurrentPts < u32PrePtsTime)
    {
        SAMPLE_SUBT_INFO_PRINT("Get PTS:%llu, pre time is %u\n", *ps64CurrentPts, u32PrePtsTime);

        /* reset the output */
        if (g_hUnfSO)
        {
            (MT_VOID) MT_UNF_SO_ResetSubBuf(g_hUnfSO);
        }
    }

    u32PrePtsTime = *ps64CurrentPts;

    return MT_SUCCESS;
}

/**********************************************************************************
@brief FilterData callback function
@param[in] u32UserData User Data
@param[in] pu8Data Obtained data
@param[in] u32DataLength Length of data
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
#ifdef CONFIG_MT_CHIP_SYMPHONY4
static MT_S32 MT_SubtFilterDataCallback(mt_u32 u32UserData, MT_U8 *pu8Data, MT_U32 u32DataLength)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static MT_S32 MT_SubtFilterDataCallback(ulong u32UserData, MT_U8 *pu8Data, MT_U32 u32DataLength)
#endif
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U32 u32SubtPID = u32UserData;

    if (g_hUnfSubt)
    {
        //SAMPLE_SUBT_INFO_PRINT("u32SubtPID=0x%x, pu8Data = 0x%x, u32DataLength=%d\n", u32SubtPID, pu8Data, u32DataLength);
        if(u32SubtPID)
        {
            s32Ret = MT_UNF_SUBT_InjectData(g_hUnfSubt, u32SubtPID, pu8Data, u32DataLength);
            if (s32Ret != MT_SUCCESS)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SUBT_InjectData, u32SubtPID=0x%x\n", u32SubtPID);
            }
        }
    }

    return s32Ret;
}


/**********************************************************************************
@brief A callback function that sends data
@param[in] u32UserData User Data
@param[in] pu8Data Obtained data
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
#ifdef CONFIG_MT_CHIP_SYMPHONY4
static MT_S32 MT_SubtUnfSubtCallback(mt_u32 u32UserData, MT_UNF_SUBT_DATA_S *pstData)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static MT_S32 MT_SubtUnfSubtCallback(ulong u32UserData, MT_UNF_SUBT_DATA_S *pstData)
#endif
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_HANDLE hSO = (MT_HANDLE)u32UserData;
    MT_UNF_SO_SUBTITLE_INFO_S stSubtitleOut;

    if (NULL != pstData)
    {
        /* send subtitle data to so */
        memset(&stSubtitleOut, 0, sizeof(stSubtitleOut));

        stSubtitleOut.eType = (MT_UNF_SO_SUBTITLE_TYPE_E)pstData->enDataType;
        if (pstData->enPageState == MT_UNF_SUBT_PAGE_NORMAL_CASE)
        {
            stSubtitleOut.unSubtitleParam.stGfx.enMsgType = MT_UNF_SO_DISP_MSG_NORM;
        }
        else
        {
            stSubtitleOut.unSubtitleParam.stGfx.enMsgType = MT_UNF_SO_DISP_MSG_ERASE;
        }

        stSubtitleOut.unSubtitleParam.stGfx.x = pstData->u32x;
        stSubtitleOut.unSubtitleParam.stGfx.y = pstData->u32y;
        stSubtitleOut.unSubtitleParam.stGfx.w = pstData->u32w;
        stSubtitleOut.unSubtitleParam.stGfx.h = pstData->u32h;

        stSubtitleOut.unSubtitleParam.stGfx.s32BitWidth = pstData->u32BitWidth;
        if(pstData->pvPalette && pstData->u32PaletteItem)
        {
            memcpy(stSubtitleOut.unSubtitleParam.stGfx.stPalette, pstData->pvPalette, pstData->u32PaletteItem);
        }

        stSubtitleOut.unSubtitleParam.stGfx.s64Pts = pstData->u32PTS;
        stSubtitleOut.unSubtitleParam.stGfx.u32Duration = (pstData->u32Duration>PAGE_TIMEOUT_OUT)?PAGE_TIMEOUT_OUT:pstData->u32Duration;

        stSubtitleOut.unSubtitleParam.stGfx.u32Len = pstData->u32DataLen;
        stSubtitleOut.unSubtitleParam.stGfx.pu8PixData = pstData->pu8SubtData;
        stSubtitleOut.unSubtitleParam.stGfx.u32CanvasWidth = pstData->u32DisplayWidth;
        stSubtitleOut.unSubtitleParam.stGfx.u32CanvasHeight = pstData->u32DisplayHeight;

        if (hSO)
        {
            s32Ret = MT_UNF_SO_SendData(hSO, &stSubtitleOut, 1000);
            if (s32Ret != MT_SUCCESS)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SO_SendData\n");
            }
        }
    }

    return MT_SUCCESS;
}

#ifdef MT_SAMPLE_APP
static MT_S32 MT_RecvTask(MT_VOID *args)
{
    MT_S32 s32Ret = MT_SUCCESS;

    while(g_bTaskQuit != MT_TRUE)
    {
        usleep(50);
        subt_run_info.hAvPlay = avplayHandle.hAvPlay;
        if(0 == subt_run_info.hAvPlay)
        {
            g_bTaskQuit = MT_TRUE;
            (mt_void)MT_SubtExit();
        }
    }

    return s32Ret;
}
#endif
/**********************************************************************************
@brief Select the subtitle number
@param[in] pstProginfo  Program information
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_U8 MT_SubtSelectSubtitle(PMT_COMPACT_PROG *pstProginfo)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U8 u8Index = 0;
    MT_U32 u32LangCode = 0;
    MT_U8 au8Buf[32] = { 0 };
    MT_U8 u8SubtNo = 0; /* for subtitle No. */
    char *fgetret=NULL;


    if (MT_UNF_SUBT_DVB == g_enSubtType)
    {
        SAMPLE_SUBT_INFO_PRINT("DVB subtitle number is %d\n", pstProginfo->u16SubtitlingNum);
        /*for multi-subtitle,need to select someone */
        if (pstProginfo->u16SubtitlingNum > 1)
        {
            for(u8Index=0; u8Index < pstProginfo->u16SubtitlingNum; u8Index++)
            {
                u32LangCode = pstProginfo->SubtitingInfo[u8Index].DesInfo[0].u32LangCode;
        
                memset(au8Buf, 0, sizeof(au8Buf));
                s32Ret = MT_SubtGetLangStr(u32LangCode, au8Buf, sizeof(au8Buf));
                if (s32Ret != MT_SUCCESS)
                {
                    SAMPLE_SUBT_ERR_PRINT("failed to GetLangStr\n");
                }
                SAMPLE_SUBT_INFO_PRINT("%d is the %s language\n", u8Index, au8Buf);
            }
        
            SAMPLE_SUBT_INFO_PRINT("Please input a number from 0 to %d for subtitle:\n", pstProginfo->u16SubtitlingNum-1);
            fgetret=fgets((char *)(au8Buf), (sizeof(au8Buf) - 1), stdin);
            fgetret=fgetret;
            u8SubtNo = atoi((char*)au8Buf);
            if (u8SubtNo >= pstProginfo->u16SubtitlingNum)
            {
                SAMPLE_SUBT_INFO_PRINT("subtitle %d is not exist!! usd default subtitle 0\n", u8SubtNo);
                u8SubtNo = 0;
            }
        }
        else
        {
            SAMPLE_SUBT_INFO_PRINT("usd default subtitle 0\n");
            u8SubtNo = 0;
        }
    }
    else if(MT_UNF_SUBT_SCTE == g_enSubtType)
    {
        SAMPLE_SUBT_INFO_PRINT("SCTE subtitle number is %d\n", pstProginfo->u16SCTESubtNum);
        if (pstProginfo->u16SCTESubtNum > 1)
        {
            /*for multi-subtitle,need to select someone */
            if (pstProginfo->u16SCTESubtNum > 1)
            {
                for(u8Index=0; u8Index < pstProginfo->u16SCTESubtNum; u8Index++)
                {
                    SAMPLE_SUBT_INFO_PRINT("%d is the %d subtitle pid\n", u8Index, pstProginfo->stSCTESubtInfo[u8Index].u16SCTESubtPID);
                }
            
                SAMPLE_SUBT_INFO_PRINT("Please input a number from 0 to %d for subtitle:\n", pstProginfo->u16SCTESubtNum-1);
                memset(au8Buf, 0, sizeof(au8Buf));
                fgetret=fgets((char *)(au8Buf), (sizeof(au8Buf) - 1), stdin);
                fgetret=fgetret;
                u8SubtNo = atoi((char*)au8Buf);
                if (u8SubtNo >= pstProginfo->u16SCTESubtNum)
                {
                    SAMPLE_SUBT_INFO_PRINT("subtitle %d is not exist!! usd default subtitle 0\n", u8SubtNo);
                    u8SubtNo = 0;
                }
            }
            else
            {
                SAMPLE_SUBT_WARN_PRINT("usd default subtitle 0\n");
                u8SubtNo = 0;
            }
        }
    }
    else
    {
        SAMPLE_SUBT_WARN_PRINT("do not find subtitle type \n");
        u8SubtNo = 0;
    }

    return u8SubtNo;
}

/**********************************************************************************
@brief Create DVB subtitle
@param[in] pstProginfo  Program information
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtCreateDVBSubt(PMT_COMPACT_PROG *pstProginfo)
{
    MT_U8 u8Index;
    MT_S32 s32Ret = MT_SUCCESS;
    MT_UNF_SUBT_PARAM_S stSubtParam;

    if (MT_NULL == pstProginfo)
    {
        SAMPLE_SUBT_ERR_PRINT("Param is NULL!\n");
        return MT_FAILURE;
    }
    memset(&stSubtParam, 0, sizeof(stSubtParam));
    stSubtParam.enDataType = MT_UNF_SUBT_DVB;

    stSubtParam.pfnCallback = MT_SubtUnfSubtCallback;
    stSubtParam.u32UserData = g_hUnfSO;
    stSubtParam.u8SubtItemNum = pstProginfo->u16SubtitlingNum;
    for (u8Index = 0; u8Index < stSubtParam.u8SubtItemNum; u8Index++)
    {
        stSubtParam.astItems[u8Index].u32SubtPID = pstProginfo->SubtitingInfo[u8Index].u16SubtitlingPID;
        stSubtParam.astItems[u8Index].u16PageID = pstProginfo->SubtitingInfo[u8Index].DesInfo[0].u16PageID;
        stSubtParam.astItems[u8Index].u16AncillaryID = pstProginfo->SubtitingInfo[u8Index].DesInfo[0].u16AncillaryPageID;
    }
    s32Ret = MT_UNF_SUBT_Init();
    s32Ret |= MT_UNF_SUBT_Create(&stSubtParam, &g_hUnfSubt);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SUBT_Create\n");
    }

    return s32Ret;

}

/**********************************************************************************
@brief Create SCTE subtitle
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtCreateSCTESubt(MT_VOID)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_UNF_SUBT_PARAM_S stSubtParam;

    memset(&stSubtParam, 0, sizeof(stSubtParam));
    stSubtParam.enDataType = MT_UNF_SUBT_SCTE;

    stSubtParam.pfnCallback = MT_SubtUnfSubtCallback;
    stSubtParam.u32UserData = g_hUnfSO;

    s32Ret = MT_UNF_SUBT_Init();
    s32Ret |= MT_UNF_SUBT_Create(&stSubtParam, &g_hUnfSubt);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SUBT_Create\n");
    }
    s32Ret = MT_UNF_SUBT_RegGetPtsCb(g_hUnfSubt, MT_SubtGetCurPts, subt_run_info.hAvPlay);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SCTESUBT_RegGetPtsCb\n");
    }
    return s32Ret;
}

/**********************************************************************************
@brief Update DVB subtitle
@param[in] pstProginfo  Program information
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtUpdateDVBSubt(PMT_COMPACT_PROG *pstProginfo)
{
    MT_U8 u8Index = 0;
    MT_S32 s32Ret = MT_SUCCESS;
    MT_UNF_SUBT_PARAM_S stSubtParam;

    if (MT_NULL == pstProginfo)
    {
        SAMPLE_SUBT_ERR_PRINT("Param is NULL!\n");
        return MT_FAILURE;
    }

    memset(&stSubtParam, 0, sizeof(stSubtParam));
    stSubtParam.pfnCallback = MT_SubtUnfSubtCallback;
    stSubtParam.u32UserData = g_hUnfSO;
    stSubtParam.enDataType = MT_UNF_SUBT_DVB;
    stSubtParam.u8SubtItemNum = pstProginfo->u16SubtitlingNum;
    for (u8Index = 0; u8Index < stSubtParam.u8SubtItemNum; u8Index++)
    {
        stSubtParam.astItems[u8Index].u32SubtPID = pstProginfo->SubtitingInfo[u8Index].u16SubtitlingPID;
        stSubtParam.astItems[u8Index].u16PageID = pstProginfo->SubtitingInfo[u8Index].DesInfo[0].u16PageID;
        stSubtParam.astItems[u8Index].u16AncillaryID = pstProginfo->SubtitingInfo[u8Index].DesInfo[0].u16AncillaryPageID;
        SAMPLE_SUBT_INFO_PRINT("update subtitle[0x%x,0x%x,0x%x]\n",stSubtParam.astItems[u8Index].u32SubtPID,
                        stSubtParam.astItems[u8Index].u16PageID,stSubtParam.astItems[u8Index].u16AncillaryID);
    }

    s32Ret = MT_UNF_SUBT_Reset(g_hUnfSubt);
    s32Ret |= MT_UNF_SUBT_Update(g_hUnfSubt, &stSubtParam);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SUBT_Update\n");
    }
    return s32Ret;
}

/**********************************************************************************
@brief SwitchDecoder
@param[in] type  subtitle type
@param[in] pstProginfo  Program information
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtSwitchDecoder(MT_U32 type, PMT_COMPACT_PROG *pstProginfo)
{
    MT_S32 s32Ret = MT_SUCCESS;
    switch(type)
    {
        case 0:     /**DVB*/
            if (g_enSubtType == MT_UNF_SUBT_DVB)
            {
                s32Ret = MT_SubtUpdateDVBSubt(pstProginfo);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_SUBT_ERR_PRINT("failed to MT_SubtUpdateDVBSubt\n");
                }

            }
            else if (g_enSubtType == MT_UNF_SUBT_SCTE)
            {

                s32Ret = MT_UNF_SUBT_Destroy(g_hUnfSubt);
                s32Ret |= MT_SubtCreateDVBSubt(pstProginfo);
            }
            else
            {

                s32Ret |= MT_SubtCreateDVBSubt(pstProginfo);
            }
            g_enSubtType = MT_UNF_SUBT_DVB;
            break;
        case 1:         /** SCTE */
            SAMPLE_SUBT_INFO_PRINT("SCTE subtitle\n");
            if (g_enSubtType == MT_UNF_SUBT_DVB)
            {
                s32Ret = MT_UNF_SUBT_Destroy(g_hUnfSubt);
                s32Ret |= MT_SubtCreateSCTESubt();
            }
            else if (g_enSubtType == MT_UNF_SUBT_SCTE)
            {
                s32Ret = MT_UNF_SUBT_Destroy(g_hUnfSubt);
                s32Ret |= MT_SubtCreateSCTESubt();
            }
            else
            {
                s32Ret |= MT_SubtCreateSCTESubt();
            }
            g_enSubtType = MT_UNF_SUBT_SCTE;
            break;
        default:
            SAMPLE_SUBT_INFO_PRINT("Input error!No subtitle!\n");
            g_enSubtType = MT_UNF_SUBT_BUTT;
            if(g_hUnfSubt)
            {
                s32Ret = MT_UNF_SUBT_Destroy(g_hUnfSubt);
                g_hUnfSubt = 0;
            }
            break;
    }
    return s32Ret;
}

/**********************************************************************************
@brief Decode according to different subtitle types
@param[in] pstProginfo  Program information
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtDecoderByProgram(PMT_COMPACT_PROG *pstProginfo)
{
    MT_S32 s32Ret = MT_SUCCESS;
    char *fgetret=NULL;

    if (pstProginfo->SubtType == (MT_UNF_SUBT_DVB | MT_UNF_SUBT_SCTE))
    {
        MT_U32 type = 0;
        MT_U8 au8Buf[32];
        SAMPLE_SUBT_INFO_PRINT("The program  has two kinds of subtitles:\n");
        SAMPLE_SUBT_INFO_PRINT("\t0 ---  choose DVB subt\n");
        SAMPLE_SUBT_INFO_PRINT("\t1 ---  choose SCTE subt\n");
        SAMPLE_SUBT_INFO_PRINT("Input choice:");

        fgetret=fgets((char *)(au8Buf), (sizeof(au8Buf) - 1), stdin);
        fgetret=fgetret;
        type = atoi((char*)au8Buf);
        s32Ret = MT_SubtSwitchDecoder(type, pstProginfo);

    }
    else if (pstProginfo->SubtType == MT_UNF_SUBT_DVB)
    {
        s32Ret = MT_SubtSwitchDecoder(0, pstProginfo);
    }
    else if (pstProginfo->SubtType == MT_UNF_SUBT_SCTE)
    {
        s32Ret = MT_SubtSwitchDecoder(1, pstProginfo);
    }
    else
    {
        s32Ret = MT_SubtSwitchDecoder(2, pstProginfo);
    }
    return s32Ret;
}


/**********************************************************************************
@brief Decode according to different subtitle types
@param[in] pSubTitleProgTbl  GET PMT
@param[in] u8ProgNo  Current program
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtStartSubtitle(const PMT_COMPACT_PROG *pProgInfo, MT_U8 *u8Num)
{
    MT_S32 s32Ret = MT_SUCCESS;
    static MT_U8 u8CurProgNo = 0;

    PMT_COMPACT_PROG *pstProginfo = pProgInfo;

    /* Create SO instance, so that the subtitle data can be processed with this instance */
    if(0 == g_hSubtOut)
    {
        s32Ret = Subt_Output_Init(&g_hSubtOut);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to Subt_Output_Init\n");
            goto ERR1;
        }
    }

    if(0 == g_hUnfSO)
    {
        s32Ret = MT_UNF_SO_Init();
        s32Ret |= MT_UNF_SO_Create(&g_hUnfSO);

        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to SUBT_DataRecv_BindParsing\n");

            return MT_FAILURE;
        }
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SO_Create\n");
            goto ERR2;
        }

        s32Ret = MT_UNF_SO_RegGetPtsCb(g_hUnfSO, MT_SubtGetCurPts, subt_run_info.hAvPlay);
        s32Ret |= MT_UNF_SO_RegOnDrawCb(g_hUnfSO, Subt_Output_OnDraw, Subt_Output_OnClear, g_hSubtOut);

        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SO_RegOnDrawCb\n");
            goto ERR2;
        }

    }

    /* Create subtitle module instance */
    if (0 == g_hUnfSubt)
    {
        s32Ret = MT_SubtDecoderByProgram(pstProginfo);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MT_SubtDecoderByProgram\n");
            goto ERR2;
        }
        u8CurProgNo = pstProginfo->ProgID;
    }

    if (g_hUnfSubt)
    {

        MT_U8 u8SubtNo = 0;
        MT_UNF_SUBT_ITEM_S stSubtItem;

        if (g_hUnfSO)
        {

            MT_UNF_SO_CLEAR_PARAM_S stClearParam;

            memset(&stClearParam, 0, sizeof(MT_UNF_SO_CLEAR_PARAM_S));
            (MT_VOID) Subt_Output_OnClear(0, (MT_VOID *)&stClearParam);
            (MT_VOID) MT_UNF_SO_ResetSubBuf(g_hUnfSO);
        }

        if (u8CurProgNo != pstProginfo->ProgID)
        {
            s32Ret = MT_SubtDecoderByProgram(pstProginfo);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to MT_SubtDecoderByProgram\n");
                goto ERR3;
            }
            u8CurProgNo = pstProginfo->ProgID;
        }

        if (MT_UNF_SUBT_DVB == g_enSubtType)
        {
            u8SubtNo = MT_SubtSelectSubtitle(pstProginfo);
            *u8Num = u8SubtNo;
            memset(&stSubtItem, 0, sizeof(stSubtItem));
            stSubtItem.u32SubtPID = pstProginfo->SubtitingInfo[u8SubtNo].u16SubtitlingPID;
            stSubtItem.u16PageID = pstProginfo->SubtitingInfo[u8SubtNo].DesInfo[0].u16PageID;
            stSubtItem.u16AncillaryID = pstProginfo->SubtitingInfo[u8SubtNo].DesInfo[0].u16AncillaryPageID;
            SAMPLE_SUBT_INFO_PRINT("select subtitle[%d,%d,%d]\n",stSubtItem.u32SubtPID,stSubtItem.u16PageID,stSubtItem.u16AncillaryID);
            s32Ret = MT_UNF_SUBT_SwitchContent(g_hUnfSubt, &stSubtItem);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SUBT_SwitchContent\n");
            }
        }
        else if (MT_UNF_SUBT_SCTE == g_enSubtType)
        {
            u8SubtNo = MT_SubtSelectSubtitle(pstProginfo);
            *u8Num = u8SubtNo;
            memset(&stSubtItem, 0, sizeof(stSubtItem));
            stSubtItem.u32SubtPID = pstProginfo->stSCTESubtInfo[u8SubtNo].u16SCTESubtPID;
            SAMPLE_SUBT_INFO_PRINT("select subtitle[%d,%d,%d]\n",stSubtItem.u32SubtPID,stSubtItem.u16PageID,stSubtItem.u16AncillaryID);
            s32Ret = MT_UNF_SUBT_SwitchContent(g_hUnfSubt, &stSubtItem);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_SUBT_SwitchContent\n");
            }
        }
    }

    return MT_SUCCESS;

ERR3:
    if(g_hUnfSubt)
    {
        (MT_VOID)MT_UNF_SUBT_Destroy(g_hUnfSubt);
        (MT_VOID)MT_UNF_SUBT_DeInit();
        g_hUnfSubt = 0;
    }
ERR2:
    if(g_hUnfSO)
    {
        (MT_VOID)MT_UNF_SO_Destroy(g_hUnfSO);
        (MT_VOID)MT_UNF_SO_DeInit();
        g_hUnfSO = 0;
    }
ERR1:
    if(g_hSubtOut)
    {
        (MT_VOID)Subt_Output_DeInit(g_hSubtOut);
        g_hSubtOut = 0;
    }

    return MT_FAILURE;
}


/**********************************************************************************
@brief StopSubtitle
@return::MT_VOID
**********************************************************************************/
static MT_S32 MT_SubtStopSubtitle(MT_VOID)
{
    if(g_hUnfSubt)
    {
        (MT_VOID) MT_UNF_SUBT_Destroy(g_hUnfSubt);
        (MT_VOID) MT_UNF_SUBT_DeInit();
        g_hUnfSubt = 0;
    }
    if(g_hUnfSO)
    {
        (MT_VOID)MT_UNF_SO_Destroy(g_hUnfSO);
        (MT_VOID)MT_UNF_SO_DeInit();
        g_hUnfSO = 0;
    }
    if(g_hSubtOut)
    {
        (MT_VOID)Subt_Output_DeInit(g_hSubtOut);
        g_hSubtOut = 0;
    }
    (MT_VOID)SUBT_Data_DeInit();
    return 0;
}

/**********************************************************************************
@brief Start Filter subtitle data
@param[in] pSubTitleProgTbl  GET PMT
@param[in] u8ProgNo  Current program
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtStartDataFilter(const PMT_COMPACT_PROG *pProgInfo, MT_U8 u8Num)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U8 u8Index = 0;
    MT_HANDLE hSubtData = MT_INVALID_HANDLE;
    PMT_COMPACT_PROG *pstProginfo = pProgInfo;
    SUBT_DATA_INSTALL_PARAM_S stInstallParam;

    s32Ret = SUBT_Data_DeInit();
    s32Ret |= SUBT_Data_Init(u8Num);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to SUBT_Data_Init\n");

        return MT_FAILURE;
    }
    stInstallParam.u32DmxID = 0;
    stInstallParam.pfnCallback = MT_SubtFilterDataCallback;

    if (MT_UNF_SUBT_DVB == g_enSubtType) /*DVB*/
    {
        for (u8Index = 0; u8Index < pstProginfo->u16SubtitlingNum; u8Index++)
        {
            stInstallParam.u16SubtPID = pstProginfo->SubtitingInfo[u8Index].u16SubtitlingPID;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
            stInstallParam.u32UserData = (mt_u32)pstProginfo->SubtitingInfo[u8Index].u16SubtitlingPID;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
            stInstallParam.u32UserData = (ulong)pstProginfo->SubtitingInfo[u8Index].u16SubtitlingPID;
#endif

            stInstallParam.enDataType = MT_UNF_SUBT_DVB;
            s32Ret |= SUBT_Data_Install(&stInstallParam, &hSubtData);
        }
    }
    else if (MT_UNF_SUBT_SCTE == g_enSubtType) /*SCTE*/
    {
        for (u8Index = 0; u8Index < pstProginfo->u16SCTESubtNum; u8Index++)
        {
            stInstallParam.u16SubtPID = pstProginfo->stSCTESubtInfo[u8Index].u16SCTESubtPID;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
            stInstallParam.u32UserData = (mt_u32)pstProginfo->stSCTESubtInfo[u8Index].u16SCTESubtPID;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
            stInstallParam.u32UserData = (ulong)pstProginfo->stSCTESubtInfo[u8Index].u16SCTESubtPID;
#endif

            stInstallParam.enDataType = MT_UNF_SUBT_SCTE;
            s32Ret |= SUBT_Data_Install(&stInstallParam, &hSubtData);
        }
    }
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to SUBT_Data_Init\n");

        (MT_VOID) SUBT_Data_DeInit();

        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

#ifndef MT_SAMPLE_APP
/**********************************************************************************
@brief Read USB resource
@param[in] args, Structure of file
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtInjectTsTask(MT_VOID *args)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U32 Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf;
    FILE *pTsFile = NULL;

    source_file_param_t *pstParam = (source_file_param_t *)(args);
    SAMPLE_SUBT_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_SUBT_ERR_PRINT("file %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    if(MT_INPUT_SIG_TYPE_FILE == subt_run_info.sInputParam.sig_type)
    {
        s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
            g_bTaskQuit = MT_TRUE;
            fclose(pTsFile);
            pTsFile = NULL;

            return MT_FAILURE;
        }
    }
    s32Ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188 * 200, &StreamBuf, 1000);
        if (s32Ret != MT_SUCCESS)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {

            SAMPLE_SUBT_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");

            /* wait for avplay end */
            rewind(pTsFile);
            continue;
        }

        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if (s32Ret != MT_SUCCESS)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(MT_INPUT_SIG_TYPE_FILE == subt_run_info.sInputParam.sig_type)
    {
        /** Destroys an existing TS buffer */
        (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
    }

    if (pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    return MT_SUCCESS;
}
#endif

static MT_VOID MT_SubtExit()
{
    g_bTaskQuit = MT_TRUE;
    g_enSubtType = MT_UNF_SUBT_BUTT;
    (MT_VOID)MT_SubtStopSubtitle();
    memset(&subt_run_info, 0, sizeof(subt_run_info));
}


static MT_VOID MT_SubtModePrintMenu(PMT_COMPACT_TBL *pSubTitleProgTbl)
{
    MT_SUBT_PRINT("commond: \n");
    MT_SUBT_PRINT("     s: choose subtitle\n");
    MT_SUBT_PRINT("     c: turn off subtitles\n");
    MT_SUBT_PRINT("     r: turn on subtitles\n");
#ifdef MT_SAMPLE_APP
    MT_SUBT_PRINT("     b: background run \n");
#endif
    MT_SUBT_PRINT("     h: help \n");
    MT_SUBT_PRINT("     q: quit \n");
#ifndef MT_SAMPLE_APP
    MT_SUBT_PRINT("'1-%d': Select a program \n", pSubTitleProgTbl->prog_num);
#endif
    MT_SUBT_PRINT("SUBTITLE>> ");
}


/**********************************************************************************
@brief Gets the state of the key
@param[in] pSubTitleProgTbl  GET PMT
@param[in] u8ProgNo  Current program
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static MT_S32 MT_SubtCmdTask(PMT_COMPACT_TBL *pSubTitleProgTbl, PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U8 num = 0;
#ifndef MT_SAMPLE_APP
    MT_U32 u32ProgNum = 1;
#endif
    MT_CHAR inputCmd[32];
    PMT_COMPACT_PROG *p_stCurrentProgInfo = pProgInfo;
#ifdef MT_SAMPLE_APP
    s32Ret = MTADP_Get_Current_Info(&p_stCurrentProgInfo);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("failed to MTADP_Get_Current_Info\n");
        SAMPLE_SUBT_ERR_PRINT("now exit!\n");
        g_bTaskQuit = MT_TRUE;
    }
#endif

    while (1)
    {
        (MT_VOID)MT_SubtModePrintMenu(pSubTitleProgTbl);

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if ('q' == inputCmd[0])
        {
            SAMPLE_SUBT_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_SUBT_INFO_PRINT("Subtitle play in back!\n");
            break;
        }
#endif
        else if ('s' == inputCmd[0])
        {
            if (g_hUnfSO)
            {
                (MT_VOID) MT_UNF_SO_ResetSubBuf(g_hUnfSO);
            }

            s32Ret = MT_SubtDecoderByProgram(p_stCurrentProgInfo);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to MT_SubtDecoderByProgram\n");
            }

            s32Ret = MT_SubtStartSubtitle(p_stCurrentProgInfo, &num);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartSubtitle\n");
            }

            s32Ret = MT_SubtStartDataFilter(p_stCurrentProgInfo, num);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartDataFilter\n");
            }
        }
        else if ('c' == inputCmd[0])
        {
            s32Ret = MT_SubtStopSubtitle();
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartSubtitle\n");
            }
            g_enSubtType = MT_UNF_SUBT_BUTT;
        }
        else if ('r' == inputCmd[0])
        {
            s32Ret = MT_SubtStartSubtitle(p_stCurrentProgInfo, &num);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartSubtitle\n");
            }

            s32Ret = MT_SubtStartDataFilter(p_stCurrentProgInfo, num);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartDataFilter\n");
            }
        }
#ifndef MT_SAMPLE_APP
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pSubTitleProgTbl->prog_num)
            {
                p_stCurrentProgInfo = pSubTitleProgTbl->proginfo + ((u32ProgNum - 1) % (pSubTitleProgTbl->prog_num));
                (MT_VOID)MT_SubtStopplay();
                s32Ret = MT_SubtStarToPlay(subt_run_info.hAvPlay, p_stCurrentProgInfo);
                s32Ret = MT_SubtStartSubtitle(p_stCurrentProgInfo, &num);
                if (s32Ret != MT_SUCCESS)
                {
                    SAMPLE_SUBT_ERR_PRINT("Switching shows failed\n");
                }

                s32Ret = MT_SubtStartDataFilter(p_stCurrentProgInfo, num);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_SUBT_ERR_PRINT("failed to StartDataFilter\n");
                }
            }
            else
            {
                SAMPLE_SUBT_ERR_PRINT(" prog_num the biggest is %d \n\n", pSubTitleProgTbl->prog_num);
            }
        }
#endif
        else if ('h' == inputCmd[0])
        {
            SAMPLE_SUBT_INFO_PRINT("Print help info \n");
        }
    }

    return MT_SUCCESS;
}

#ifndef MT_SAMPLE_APP
/**********************************************************************************
@brief help
@return MT_VOID
**********************************************************************************/
static MT_VOID MT_SubtPrint_Help(MT_CHAR *name)
{
    MT_SUBT_PRINT("Lack of parameters\n");
    MT_SUBT_PRINT("\nUsage:\n");
    MT_SUBT_PRINT("%s\n", name);
    MT_SUBT_PRINT("    -f: path of the stream file\n");
    MT_SUBT_PRINT("    -c: DVBC locks frequency\n");
    MT_SUBT_PRINT("    -s: DVBS locks frequency\n");
#ifdef MT_SAMPLE_APP
    MT_SUBT_PRINT("    -q: Exit the background\n");
#endif
    MT_SUBT_PRINT("example:\n");
    MT_SUBT_PRINT("    %s -f ./ttx.ts\n", name);
    MT_SUBT_PRINT("    %s -c 314 6875 64\n", name);
    MT_SUBT_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}


/**********************************************************************************
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
**********************************************************************************/
static MT_S32 MT_SubtParase_args(MT_S32 argc, char *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_SubtPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_SubtPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'f':
                if(argc < 3)
                {
                    (MT_VOID)MT_SubtPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy((mt_char*)pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;
            case 's':
                if(argc < 7)
                {
                    (MT_VOID)MT_SubtPrint_Help(argv[0]);
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
                if(argc < 5)
                {
                    (MT_VOID)MT_SubtPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_SubtExit();
                }
                return MT_TASK_EXIT;
        }
    }


    return MT_SUCCESS;

}
#endif

#ifdef MT_SAMPLE_APP
MT_S32 MT_SubtMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U8 num = 0;
    PMT_COMPACT_PROG *p_stCurrentProgInfo = { 0 };
#ifndef MT_SAMPLE_APP
    s32Ret = MT_SubtParase_args(argc, argv, &subt_run_info.sInputParam);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_SUBT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_SUBT_INFO_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
#endif
    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }

        s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
        }

        s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }

        g_bTaskQuit = MT_FALSE;
        (MT_VOID) MTADP_MCE_Exit();

        if(MT_INPUT_SIG_TYPE_FILE != subt_run_info.sInputParam.sig_type)
        {
            s32Ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("MTADP_Fe_Init failed\n");
                goto ERR3;
            }

            if (MT_INPUT_SIG_TYPE_CAB == subt_run_info.sInputParam.sig_type)
            {     //dvbc
                s32Ret = MT_SubtCheckDvbcParam(&subt_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_SUBT_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR4;
                }
                s32Ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            subt_run_info.sInputParam.input_param.cab.freq,
                                            subt_run_info.sInputParam.input_param.cab.sym_rate,
                                            subt_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == subt_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_SubtCheckDvbsParam(&subt_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_SUBT_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR4;
                }
                s32Ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            subt_run_info.sInputParam.input_param.sat.freq,
                                            subt_run_info.sInputParam.input_param.sat.sym_rate,
                                            subt_run_info.sInputParam.input_param.sat.onoff_22k,
                                            subt_run_info.sInputParam.input_param.sat.polarization,
                                            subt_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("MTADP_Fe_Connect failed\n");
                goto ERR4;
            }
        }

        s32Ret = MTADP_Snd_Init();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;

        }

        s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        s32Ret = MT_SubtDmxInit(&subt_run_info.sInputParam);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to StartDmx\n");
            goto ERR6;
        }

        if(MT_INPUT_SIG_TYPE_FILE == subt_run_info.sInputParam.sig_type)
        {
            s32Ret = pthread_create(&subt_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_SubtInjectTsTask, &subt_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to pthread_create\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }

        s32Ret = MT_SubtAvPlayInit(&subt_run_info.hSoundTrack, &subt_run_info.hWin);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to StartAVPlay\n");
            goto ERR8;
        }
        /* Search table, including subtitle informations */
        (MT_VOID) MTADP_Search_Init();
        while (MT_SUCCESS != MTADP_Search_GetAllPmt(0, &subt_run_info.pProgTbl))
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            MT_USLEEP(100000);
            goto ERR10;
        }
#endif
        g_bTaskQuit = MT_FALSE;

#ifdef MT_SAMPLE_APP
        subt_run_info.hAvPlay = avplayHandle.hAvPlay;
        if(0 == subt_run_info.hAvPlay)
        {
            SAMPLE_SUBT_ERR_PRINT("Didn't get avplay handle\n");
            goto ERR12;
        }
        s32Ret = MTADP_Get_Current_Info(&p_stCurrentProgInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to MTADP_Get_Current_Info\n");
            goto ERR12;
        }
#else
        p_stCurrentProgInfo = subt_run_info.pProgTbl->proginfo + (0 % (subt_run_info.pProgTbl->prog_num));
#endif
        if(0 == p_stCurrentProgInfo->u16SubtitlingNum && 0 == p_stCurrentProgInfo->u16SCTESubtNum)
        {
            SAMPLE_SUBT_INFO_PRINT("There is no subtitle for the video stream\n");
#ifndef MT_SAMPLE_APP
            s32Ret = MT_SubtStarToPlay(subt_run_info.hAvPlay, p_stCurrentProgInfo);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartAVPlay\n");
                goto ERR11;
            }
#else
            goto ERR12;
#endif
        }
        else
        {
#ifndef MT_SAMPLE_APP
            s32Ret = MT_SubtStarToPlay(subt_run_info.hAvPlay, p_stCurrentProgInfo);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartAVPlay\n");
                goto ERR12;
            }
#endif
            s32Ret = MT_SubtStartSubtitle(p_stCurrentProgInfo, &num);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartSubtitle\n");
                goto ERR12;
            }

            s32Ret = MT_SubtStartDataFilter(p_stCurrentProgInfo, num);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SUBT_ERR_PRINT("failed to StartDataFilter\n");
                goto ERR13;
            }
        }
#ifdef MT_SAMPLE_APP
        s32Ret = pthread_create(&subt_run_info.usrRecvThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_RecvTask, NULL);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SUBT_ERR_PRINT("failed to pthread_create\n");
            goto ERR13;
        }
#endif
    }

    (MT_VOID) MT_SubtCmdTask(subt_run_info.pProgTbl, p_stCurrentProgInfo);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifdef MT_SAMPLE_APP
    pthread_join(subt_run_info.usrRecvThread, NULL);
#endif
ERR13:
    g_enSubtType = MT_UNF_SUBT_BUTT;
    (MT_VOID)MT_SubtStopSubtitle();
ERR12:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_SubtStopplay();
ERR11:
    (MT_VOID)MTADP_Search_FreeAllPmt(subt_run_info.pProgTbl);
ERR10:
    (void)MTADP_Search_DeInit();
ERR9:
    (MT_VOID)MT_SubtAvPlayDeInit(subt_run_info.hSoundTrack, subt_run_info.hWin);
ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == subt_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(subt_run_info.stInjectTSThread, NULL);
    }
ERR7:
    (MT_VOID)MT_SubtDmxDeInit();
ERR6:
    (MT_VOID)MTADP_VO_DeInit();
ERR5:
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:
    if(MT_INPUT_SIG_TYPE_FILE != subt_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR3:
    (MT_VOID)MTADP_Disp_DeInit();
ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&subt_run_info, 0xff, sizeof(subt_run_info));

    return MT_SUCCESS;
}


