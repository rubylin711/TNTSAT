/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/
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
#ifdef  MT_SAMPLE_Audio_VIVID_DEBUG
#define MT_Audio_VIVID_PRINT   printf
#else
#define MT_Audio_VIVID_PRINT
#endif

#define SAMPLE_Audio_VIVID_FUNCTION_ENTER()   MT_Audio_VIVID_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_Audio_VIVID_FUNCTION_EXIT()        MT_Audio_VIVID_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_Audio_VIVID_FATAL_PRINT(fmt...)       MT_Audio_VIVID_PRINT(" [FATAL] " fmt)
#define SAMPLE_Audio_VIVID_ERR_PRINT(fmt...)         MT_Audio_VIVID_PRINT(" [ERROR] " fmt)
#define SAMPLE_Audio_VIVID_WARN_PRINT(fmt...)        MT_Audio_VIVID_PRINT(" [WARN] "  fmt)
#define SAMPLE_Audio_VIVID_INFO_PRINT(fmt...)        MT_Audio_VIVID_PRINT(" [INFO] "  fmt)
#define SAMPLE_Audio_VIVID_DBG_PRINT(fmt...)         MT_Audio_VIVID_PRINT(" [DEBUG] " fmt)

#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
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
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
} MT_Audio_VIVID_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
static MT_Audio_VIVID_RUN_INFO g_audio_vivid_run_info;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_AudioVIVIDMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

#ifndef MT_SAMPLE_APP
/**********************************************************************************
@brief Read USB resource
@param[in] args, Structure of file
@return::MT_SUCCESS Success.
@return::MT_FAILURE Failed.
**********************************************************************************/
static mt_s32 InjectTsTask(mt_void *args)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u32 Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf;
    FILE *pTsFile = NULL;

    source_file_param_t *pstParam = (source_file_param_t *)(args);
    SAMPLE_Audio_VIVID_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("file %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer ret=0x%x \n", s32Ret);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /* loop in inject data */
    while (g_bTaskQuit == MT_FALSE)
    {
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188 * 200, &StreamBuf, 1000);
        if (s32Ret != MT_SUCCESS)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {

            SAMPLE_Audio_VIVID_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");

            /* wait for avplay end */
            rewind(pTsFile);
            continue;
        }

        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if (s32Ret != MT_SUCCESS)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("failed to MT_UNF_DMX_PutTSBuffer ret=0x%x \n", s32Ret);
        }
    }

    (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    if (pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    return MT_SUCCESS;
}


static mt_s32 MT_Audio_VIVIDCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_Audio_VIVID_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_Audio_VIVIDCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{

    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_Audio_VIVID_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_Audio_VIVIDDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    MT_S32           s32Ret = MT_FAILURE;
    mt_sys_version_s stSysChipInfo;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = 0x%x\n", s32Ret);
        return s32Ret;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = 0x%x\n", s32Ret);
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
            SAMPLE_Audio_VIVID_ERR_PRINT("failed to mt_sys_get_version\n");
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
            SAMPLE_Audio_VIVID_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_Audio_VIVIDDmxDeInit(MT_VOID)
{
    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();
}

/*!
@brief audio and video player initialization
@param[out] hWin                the input window handler
@param[out] phAvplay            Handle to AV player
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_Audio_VIVIDAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE* phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = MT_INVALID_HANDLE;
    MT_HANDLE                hWin = MT_INVALID_HANDLE;
    MT_HANDLE                hSoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == phAvplay)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }

    if(NULL == phWin)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }

    if(NULL == phSoundTrack)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MTADP_AVPlay_RegADecLib failed ret=0x%x \n", ret);
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_Init failed ret=0x%x \n", ret);
        return ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed ret=0x%x \n", ret);
        goto ERR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    /** Create AV player based on attributes */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_Create failed ret=0x%x \n", ret);
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed ret=0x%x \n", ret);
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed ret=0x%x \n", ret);
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed ret=0x%x \n", ret);
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_SND_CreateTrack failed ret=0x%x \n", ret);
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_SND_Attach failed ret=0x%x \n", ret);
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MTADP_VO_CreatWin failed ret=0x%x \n", ret);
        goto ERR6;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_VO_AttachWindow failed ret=0x%x \n", ret);
        goto ERR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed ret=0x%x \n", ret);
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

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


/*!
@brief stop AV playback into the stop state
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_Audio_VIVIDStopToPlay(MT_HANDLE hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;
    SAMPLE_Audio_VIVID_INFO_PRINT("stop live play ...\n");
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}

/*!
@brief audio and video player deinitialization
@param[in]  phAvplay            handle to AV player
@param[in]  hWin                Handle to window
@param[in]  phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_VOID MT_Audio_VIVIDAvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("phAvplay is null.\n");

    }

    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("phWin is null.\n");

    }

    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("phSoundTrack is null.\n");

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
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_Audio_VIVIDSetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };

    if(NULL == pProgInfo)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("The input address is empty\n");
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
        AudPid  = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    SAMPLE_Audio_VIVID_INFO_PRINT("VidPid = %#x, AudPid = %#x\n", VidPid, AudPid);

    if(VidPid != INVALID_TSPID)
    {
        /** Get the video properties of the AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed ret=0x%x \n", ret);
            return ret;
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

        VdecAttr.enType = enVidType;
        VdecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.s32CtrlOptions = 0;
        VdecAttr.u32Priority = 3;

        /** Set the video properties of the AV player */
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);

        /** Set the video PID properties of AV player */
        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("Set video properties or video PID property failed\n");
            return ret;
        }
    }

    if(AudPid != INVALID_TSPID)
    {
        /** Set audio decoder properties */
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);

        /** Set the audio PID properties of AV player */
        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("Setting the decoding mode or audio PID property failed\n");
            return ret;
        }
    }

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /** Set the audio and video synchronization properties of AV player */
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed ret=0x%x \n", ret);
            return ret;
        }
    }

    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_Audio_VIVIDStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    if (NULL == pProgInfo)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return MT_FAILURE;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret=MT_Audio_VIVIDSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_Audio_VIVIDSetAvplayPidAndCodecType fail! \n");
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
        SAMPLE_Audio_VIVID_ERR_PRINT("Has no audio stream! ret=0x%x \n", ret);
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("Has no video stream! ret=0x%x \n", ret);
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
            SAMPLE_Audio_VIVID_ERR_PRINT("Set frame to VO is failed ret=0x%x \n", ret);
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("Get avplay sync attr is failed ret=0x%x \n", ret);
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("Set avplay sync attr is failed ret=0x%x \n", ret);
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  =MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_UNF_AVPLAY_Start failed ret=0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

#endif

static MT_VOID MT_Audio_VIVIDExit(void)
{
    g_bTaskQuit = MT_TRUE;

#ifndef MT_SAMPLE_APP
    /** Stop playing the show */
    (MT_VOID)MT_Audio_VIVIDStopToPlay(g_audio_vivid_run_info.hAvPlay);

    /** Audio and video player deinitialization */
    (MT_VOID)MT_Audio_VIVIDAvplayDeInit(g_audio_vivid_run_info.hAvPlay, g_audio_vivid_run_info.hWin, g_audio_vivid_run_info.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_audio_vivid_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == g_audio_vivid_run_info.sInputParam.sig_type)
    {
        /** Wait for the thread to end */
        pthread_join(g_audio_vivid_run_info.stInjectTSThread, NULL);
    }

    (MT_VOID)MT_Audio_VIVIDDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != g_audio_vivid_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
#endif
    memset(&g_audio_vivid_run_info, 0xff, sizeof(g_audio_vivid_run_info));
}


/*!
@brief Help information
@return::void
@*/
static void MT_Audio_VIVIDPrint_Help(MT_CHAR *name)
{
#ifndef MT_SAMPLE_APP
    MT_Audio_VIVID_PRINT("Lack of parameters\n");
    MT_Audio_VIVID_PRINT("\nUsage:\n");
    MT_Audio_VIVID_PRINT("%s\n", name);
    MT_Audio_VIVID_PRINT("    -f: path of the stream file\n");
    MT_Audio_VIVID_PRINT("    -c: DVBC locks frequency\n");
    MT_Audio_VIVID_PRINT("    -s: DVBS locks frequency\n");
    MT_Audio_VIVID_PRINT("example:\n");
    MT_Audio_VIVID_PRINT("    %s -f ./677-CC-12.ts\n", name);
    MT_Audio_VIVID_PRINT("    %s -c 314 6875 64\n", name);
    MT_Audio_VIVID_PRINT("    %s -s 3840 27500 1 0 0\n", name);
#else
    MT_Audio_VIVID_PRINT("    -q: Exit the background\n");
#endif
}




/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_Audio_VIVIDParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

	SAMPLE_Audio_VIVID_FUNCTION_ENTER();
	
#ifndef MT_SAMPLE_APP
    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_Audio_VIVIDPrint_Help(argv[0]);
        return MT_FAILURE;
    }
#endif

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s:q")) != -1)
    {
    	MT_Audio_VIVID_PRINT(" opt[%c]\n", opt);
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_Audio_VIVIDPrint_Help(argv[0]);
                return MT_FAILURE;
#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_Audio_VIVIDPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_Audio_VIVIDPrint_Help(argv[0]);
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
                    (MT_VOID)MT_Audio_VIVIDPrint_Help(argv[0]);
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
                    (MT_VOID)MT_Audio_VIVIDExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_Audio_VIVIDPrint_Help(argv[0]);
                return MT_FAILURE;

        }
    }

	SAMPLE_Audio_VIVID_FUNCTION_EXIT();

    return MT_SUCCESS;

}


static MT_VOID MT_Audio_VIVIDPrintMenu(mt_s32 prognum)
{
    MT_Audio_VIVID_PRINT("commond: \n");
#ifndef MT_SAMPLE_APP
    MT_Audio_VIVID_PRINT("'1-%d': Select a program \n", prognum);
#endif
    MT_Audio_VIVID_PRINT("     d: show vivid meta info\n");
	MT_Audio_VIVID_PRINT("     o: select obj id\n");
	MT_Audio_VIVID_PRINT("     p: set postion\n");
#ifdef MT_SAMPLE_APP
    MT_Audio_VIVID_PRINT("     b: background run \n");
#endif
    MT_Audio_VIVID_PRINT("     q: quit \n");
    MT_Audio_VIVID_PRINT("     h: help \n");
    MT_Audio_VIVID_PRINT("VIVID>> ");
}

/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_VOID MT_Audio_VIVIDCmdTask(MT_HANDLE hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_S32                 ret = MT_FAILURE;
    MT_CHAR                inPutCmd[32] = { 0 };
#ifndef MT_SAMPLE_APP
    MT_U32                 u32ProgNum = 0;
    PMT_COMPACT_PROG       *pstCurrentProgInfo = NULL;
    struct timespec start, end;
    double elapsed_seconds;
#endif
	MT_S32	input_val;
	MT_U32	i, j, k;
	float x,y,z;
	MT_S32	obj_id, obj_on = 0;
	MT_UNF_AVPLAY_METARINFO_S meta_info;

#ifndef MT_SAMPLE_APP
    if(MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }
#endif

    while(1)
    {
        (MT_VOID)MT_Audio_VIVIDPrintMenu(pProgTbl->prog_num);
        fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);

        if('q' == inPutCmd[0])
        {
            SAMPLE_Audio_VIVID_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inPutCmd[0])
        {
            SAMPLE_Audio_VIVID_INFO_PRINT("VIVID play in back!\n");
            break;
        }
    #endif
#ifndef MT_SAMPLE_APP
        u32ProgNum = atoi(inPutCmd);
        if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1) % pProgTbl->prog_num);

            /** Stop AV playback into the stop state */
            ret = MT_Audio_VIVIDStopToPlay(hAvPlay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_Audio_VIVID_ERR_PRINT("MT_Audio_VIVIDStopToPlay failed ret=0x%x \n", ret);
            }
            MT_Audio_VIVID_PRINT("===== Start play ProgNum: %d \n", u32ProgNum);

            /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
            ret = MT_Audio_VIVIDStarToPlay(hAvPlay, pstCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_Audio_VIVID_ERR_PRINT("Switching shows failed\n");
            }
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            MT_Audio_VIVID_PRINT("Switching time = %lf s\n", elapsed_seconds);
        }
#endif
        else if ('d' == inPutCmd[0])
        {
            SAMPLE_Audio_VIVID_INFO_PRINT("show vivid meta info\n");
			memset(&meta_info, 0, sizeof(MT_UNF_AVPLAY_METARINFO_S));
			ret = MT_UNF_AVPLAY_GetMetaInfo(hAvPlay, &meta_info);
			if(MT_SUCCESS != ret){
				SAMPLE_Audio_VIVID_ERR_PRINT("get vivid meta info failed ret=0x%x \n", ret);
			} else {
				printf("obj num:%d\n",meta_info.obj_num);
				for(i=0; i<meta_info.obj_num; i++) {
					printf("obj id:%d\n",meta_info.obj_info[i].obj_id);
					printf("obj name:%s\n",meta_info.obj_info[i].obj_name);
					printf("obj interact:%d\n",meta_info.obj_info[i].interact);
				}

				for(j=0; j<sizeof(meta_info.complementary_object_num)/sizeof(meta_info.complementary_object_num[0]); j++) {
					if(meta_info.complementary_object_num[j] > 0) {
						printf("obj complementary num:%d\n",meta_info.complementary_object_num[j]);
						for(k=0; k<meta_info.complementary_object_num[j]; k++)
							printf("obj complementary id:%d\n",meta_info.complementary_object_id[j][k]);
					}
				}
			}
        }
        else if ('o' == inPutCmd[0])
        {
			printf("input Obj id and state(1:on 0:off):\n");
			scanf("%d %d",&obj_id, &obj_on);
			if(meta_info.obj_num == 0)
			{
				ret = MT_UNF_AVPLAY_GetMetaInfo(hAvPlay, &meta_info);
				if(MT_SUCCESS != ret)
				{
					SAMPLE_Audio_VIVID_ERR_PRINT("get vivid meta info failed ret=0x%x \n", ret);
					continue;
				}
				
			}

			if(obj_id >= 0 && obj_id < meta_info.obj_num) {
				ret = MT_UNF_AVPLAY_SelObj(hAvPlay, obj_id, obj_on);
				if(MT_SUCCESS != ret){
					SAMPLE_Audio_VIVID_ERR_PRINT("sel obj id:%d failed ret=0x%x \n",input_val, ret);
				}
            } else {
                SAMPLE_Audio_VIVID_FATAL_PRINT("The obj id value is out of range!!!!!!\n");
            }
        }
        else if ('p' == inPutCmd[0])
        {
            printf("input position x:\n");
			scanf("%f",&x);
			printf("input position y:\n");
			scanf("%f",&y);
			printf("input position z:\n");
			scanf("%f",&z);
			if((x >= -10 && x <= 10) && (y >= -10 && y <= 10) && (z >= -10 && z <= 10)) {
			ret = MT_UNF_AVPLAY_SetPos(hAvPlay, x, y, z);
			if(MT_SUCCESS != ret){
				SAMPLE_Audio_VIVID_ERR_PRINT("set postion:(%f,%f,%f) failed ret=0x%x \n",x,y,z, ret);
			}
            } else {
                SAMPLE_Audio_VIVID_FATAL_PRINT("The position value is out of range, should be [-10,10]!!!!!!\n");
            }
        }

        else if('h' == inPutCmd[0])
        {
            SAMPLE_Audio_VIVID_INFO_PRINT("Print help info\n");
        }
    }


}


#ifdef MT_SAMPLE_APP
MT_S32 MT_AudioVIVIDMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32             ret = MT_FAILURE;
#ifndef MT_SAMPLE_APP
    MT_S32             count = 0;
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;
    struct timespec start, end;
    double elapsed_seconds;
#endif
	MT_UNF_AVPLAY_ATTR_S AvplayAttr;

    /** Get the parameters */
    ret = MT_Audio_VIVIDParase_args(argc, argv, &g_audio_vivid_run_info.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
    #ifndef MT_SAMPLE_APP
        /** System initialization */
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("failed to mt_sys_init\n");
            return ret;
        }

        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("failed to StartDmx\n");
            goto ERR1;
        }

        sleep(1);

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }

        g_bTaskQuit = MT_FALSE;
        clock_gettime(CLOCK_MONOTONIC, &start);
        /** Tuner initialization, Set the default parameters for tuner */
        if(MT_INPUT_SIG_TYPE_FILE != g_audio_vivid_run_info.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_Audio_VIVID_ERR_PRINT("MTADP_Fe_Init failed ret=0x%x \n", ret);
                goto ERR3;
            }

            if(MT_INPUT_SIG_TYPE_CAB == g_audio_vivid_run_info.sInputParam.sig_type)
            {     //dvbc
                ret = MT_Audio_VIVIDCheckDvbcParam(&g_audio_vivid_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_Audio_VIVID_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR4;
                }

                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            g_audio_vivid_run_info.sInputParam.input_param.cab.freq,
                                            g_audio_vivid_run_info.sInputParam.input_param.cab.sym_rate,
                                            g_audio_vivid_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_audio_vivid_run_info.sInputParam.sig_type)
            {
                ret = MT_Audio_VIVIDCheckDvbsParam(&g_audio_vivid_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_Audio_VIVID_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR4;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            g_audio_vivid_run_info.sInputParam.input_param.sat.freq,
                                            g_audio_vivid_run_info.sInputParam.input_param.sat.sym_rate,
                                            g_audio_vivid_run_info.sInputParam.input_param.sat.onoff_22k,
                                            g_audio_vivid_run_info.sInputParam.input_param.sat.polarization,
                                            g_audio_vivid_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_Audio_VIVID_ERR_PRINT("MTADP_Fe_Connect failed ret=0x%x \n", ret);
                goto ERR4;
            }
        }

        /** VO device initialization */
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MTADP_VO_Init failed ret=0x%x \n", ret);
            goto ERR4;
        }

        /** Audio device initialization */
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MTADP_Snd_Init failed ret=0x%x \n", ret);
            goto ERR5;
        }

        ret = MT_Audio_VIVIDDmxInit(g_audio_vivid_run_info.sInputParam.sig_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MT_Audio_VIVIDDmxInit failed ret=0x%x \n", ret);
            goto ERR6;
        }

        if(MT_INPUT_SIG_TYPE_FILE == g_audio_vivid_run_info.sInputParam.sig_type)
        {
            ret = pthread_create(&g_audio_vivid_run_info.stInjectTSThread, NULL, (void * (*)(void *))InjectTsTask, &g_audio_vivid_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_Audio_VIVID_ERR_PRINT("failed to pthread_create\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }

        /** Demux initializes and retrieves the PMT and PAT tables in TS */
        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        while(MT_SUCCESS != MTADP_Search_GetAllPmt(DMX_ID_0, &g_audio_vivid_run_info.pProgTbl))
        {
            count++;
            SAMPLE_Audio_VIVID_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            MT_USLEEP(100000);
            if(20 == count)
            {
                goto ERR9;
            }
        }

        /** audio and video player initialization */
        ret = MT_Audio_VIVIDAvplayInit(&g_audio_vivid_run_info.hAvPlay, &g_audio_vivid_run_info.hWin, &g_audio_vivid_run_info.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MT_Audio_VIVIDAvplayInit failed ret=0x%x \n", ret);
            goto ERR10;
        }

        pstCurrentProgInfo = g_audio_vivid_run_info.pProgTbl->proginfo;
        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        ret = MT_Audio_VIVIDStarToPlay(g_audio_vivid_run_info.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_Audio_VIVID_ERR_PRINT("MT_Audio_VIVIDStarToPlay failed ret=0x%x \n", ret);
            goto ERR11;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        MT_Audio_VIVID_PRINT("Search time = %lf s\n", elapsed_seconds);
    #endif
        g_bTaskQuit = MT_FALSE;
    }

#ifdef MT_SAMPLE_APP
    g_audio_vivid_run_info.hAvPlay = avplayHandle.hAvPlay;
#endif

	if(MT_INVALID_HANDLE == g_audio_vivid_run_info.hAvPlay || 0 == g_audio_vivid_run_info.hAvPlay) 
	{
		ret =MT_UNF_AVPLAY_Init();
		if (MT_SUCCESS != ret)
		{
			SAMPLE_Audio_VIVID_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_Init error!\n", __func__, __LINE__);
			return MT_FAILURE;
		}
	
		memset(&AvplayAttr,0,sizeof(MT_UNF_AVPLAY_ATTR_S));
		ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
		ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &g_audio_vivid_run_info.hAvPlay);
		if (MT_SUCCESS != ret)
		{
			SAMPLE_Audio_VIVID_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_Create error!\n", __func__, __LINE__);
			goto ERR0;
		}
	
		ret = MT_UNF_AVPLAY_ChnOpen(g_audio_vivid_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
		if (MT_SUCCESS != ret)
		{
			SAMPLE_Audio_VIVID_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_ChnOpen error!\n", __func__, __LINE__);
			goto ERR1;
		}
	
	    (MT_VOID)MT_Audio_VIVIDCmdTask(g_audio_vivid_run_info.hAvPlay, g_audio_vivid_run_info.pProgTbl);

		(MT_VOID)MT_UNF_AVPLAY_ChnClose(g_audio_vivid_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
		ERR1:
			(MT_VOID)MT_UNF_AVPLAY_Destroy(g_audio_vivid_run_info.hAvPlay);
		ERR0:
			(MT_VOID)MT_UNF_AVPLAY_DeInit();
	} else {
		(MT_VOID)MT_Audio_VIVIDCmdTask(g_audio_vivid_run_info.hAvPlay, g_audio_vivid_run_info.pProgTbl);
	}

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifndef MT_SAMPLE_APP
    /** Stop AV playback and enter the stop state */
    ret = MT_Audio_VIVIDStopToPlay(g_audio_vivid_run_info.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_Audio_VIVID_ERR_PRINT("MT_Audio_VIVIDStopToPlay failed ret=0x%x \n", ret);
    }

ERR11:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_Audio_VIVIDAvplayDeInit(g_audio_vivid_run_info.hAvPlay, g_audio_vivid_run_info.hWin, g_audio_vivid_run_info.hSoundTrack);

ERR10:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_audio_vivid_run_info.pProgTbl);
ERR9:
    (MT_VOID)MTADP_Search_DeInit();
ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == g_audio_vivid_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(g_audio_vivid_run_info.stInjectTSThread, NULL);
    }
ERR7:
    /** Demux module deinitialization */
    (MT_VOID)MT_Audio_VIVIDDmxDeInit();
ERR6:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

ERR5:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    /** Disconnect the tuner lock */
    if(MT_INPUT_SIG_TYPE_FILE != g_audio_vivid_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR3:
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR2:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_audio_vivid_run_info, 0xff, sizeof(g_audio_vivid_run_info));

    return ret;
}
