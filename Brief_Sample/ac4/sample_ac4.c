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
#ifdef  MT_SAMPLE_AC4_DEBUG
#define MT_AC4_PRINT   printf
#else
#define MT_AC4_PRINT
#endif

#define SAMPLE_AC4_FUNCTION_ENTER()   MT_AC4_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_AC4_FUNCTION_EXIT()        MT_AC4_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_AC4_FATAL_PRINT(fmt...)       MT_AC4_PRINT(" [FATAL] " fmt)
#define SAMPLE_AC4_ERR_PRINT(fmt...)         MT_AC4_PRINT(" [ERROR] " fmt)
#define SAMPLE_AC4_WARN_PRINT(fmt...)        MT_AC4_PRINT(" [WARN] "  fmt)
#define SAMPLE_AC4_INFO_PRINT(fmt...)        MT_AC4_PRINT(" [INFO] "  fmt)
#define SAMPLE_AC4_DBG_PRINT(fmt...)         MT_AC4_PRINT(" [DEBUG] " fmt)

#define SAMPLE_AC4_PRINT   printf

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
} MT_AC4_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
static MT_AC4_RUN_INFO g_ac4_run_info;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_AC4Main(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static void MT_AC4PrintAudioMessageDescriptor(SDT_TB sdt_tb)
{
    mt_s32 j,k;

    for (k = 0; k < sdt_tb.u32ProgNum; k++)
    {
        if (1 == sdt_tb.SdtInfo[k].message_descriptor_flag)
        {
            printf("\nmessage_id ISO_639_language_code labes \n");
            printf("%-10s %-21s %s\n", "----------", "--------------------", "----------------");
            for (j = 0; j < sdt_tb.SdtInfo[k].num_message_descriptor; j++)
            {
                printf("%-10d %-21s %s\n", sdt_tb.SdtInfo[k].message_descriptor[j].message_id,
                    sdt_tb.SdtInfo[k].message_descriptor[j].iso639LanguageCode,
                    sdt_tb.SdtInfo[k].message_descriptor[j].message);
            }
        }
    }

    return;
}

// 获取音频渲染指示的描述
static const char* MT_AC4GetAudioRenderingDescription(mt_u8 indication)
{
    switch (indication)
    {
        case 0: return "not indicated";
        case 1: return "main stereo";
        case 2: return "two-dimensional (e.g. 5.1 multi-channel)";
        case 3: return "three-dimensional";
        default: return "reserved";
    }
}

static void MT_AC4PrintAudioPreselectionDescriptor(const audio_preselection_descriptor_t* descriptor)
{
    printf("\n=== audio preselection descriptor ===\n");
    printf("descriptor_tag: 0x%02x\n", descriptor->descriptor_tag);
    printf("descriptor_length: 0x%x (%d) bytes\n", descriptor->descriptor_length, descriptor->descriptor_length);
    printf("descriptor_tag_extension: 0x%02x\n", descriptor->descriptor_tag_extension);
    printf("num_preselections: %d\n", descriptor->num_preselections);
    printf("has_aux_components: %d\n", descriptor->has_aux_components);
    for (int i = 0; i < descriptor->num_parsed_preselections; i++) {
        const audio_preselection_t* presel = &descriptor->preselections[i];

        printf("preselection %d:\n", i + 1);
        printf("  preselection_id: 0x%x (%d)\n", presel->preselection_id, presel->preselection_id);
        printf("  audio_rendering_indication: %d => %s\n",
               presel->audio_rendering_indication,
               MT_AC4GetAudioRenderingDescription(presel->audio_rendering_indication));

        printf("  audio_description: %d\n", presel->flags.audio_description);
        printf("  spoken_subtitles: %d\n", presel->flags.spoken_subtitles);
        printf("  dialogue_enhancement: %d\n", presel->flags.dialogue_enhancement);
        printf("  interactivity_enabled: %d\n", presel->flags.interactivity_enabled);
        printf("  language_code_present: %d\n", presel->flags.language_code_present);
        printf("  text_label_present: %d\n", presel->flags.text_label_present);
        printf("  multi_stream_info_present: %d\n", presel->flags.multi_stream_info_present);
        printf("  future_extension: %d\n", presel->flags.future_extension);

        if (presel->flags.language_code_present)
        {
            printf("  language_code: %s\n", presel->language_code);
        }

        if (presel->has_message_id)
        {
            printf("  message_id: %d\n", presel->message_id);
        }

        if (presel->has_aux_info)
        {
            printf("  num_aux_components: %d \n", presel->aux_info.num_aux_components);
            for (int j = 0; j < presel->aux_info.num_aux_components && j < 16; j++)
            {
                printf(" [%d]=0x%02x", j, presel->aux_info.component_tags[j]);
            }
            printf("\n");
        }
        printf("\n");
    }

    for (int i = 0; i < descriptor->num_parsed_preselections; i++) {
        const audio_preselection_t* presel = &descriptor->preselections[i];
        printf("preselection_id: %d, ", presel->preselection_id);

        if (presel->flags.language_code_present)
        {
            printf("%s ", presel->language_code);
        }

        if (presel->flags.dialogue_enhancement)
        {
            printf("+ %s ", "dialogue_enhancement");
        }

        if (presel->flags.spoken_subtitles)
        {
            printf("+ %s ", "spoken_subtitles");
        }

        if (presel->flags.audio_description)
        {
            printf("+ %s ", "audio_description");
        }

        if (presel->has_message_id)
        {
            printf("+ message_id: %d", presel->message_id);
        }
        printf("\n");
    }
    printf("\n");

}

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
    SAMPLE_AC4_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_AC4_ERR_PRINT("file %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_AC4_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_AC4_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer ret=0x%x \n", s32Ret);
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

            SAMPLE_AC4_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");

            /* wait for avplay end */
            rewind(pTsFile);
            continue;
        }

        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if (s32Ret != MT_SUCCESS)
        {
            SAMPLE_AC4_ERR_PRINT("failed to MT_UNF_DMX_PutTSBuffer ret=0x%x \n", s32Ret);
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


static mt_s32 MT_AC4CheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_AC4_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_AC4_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_AC4_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_AC4CheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{

    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_AC4_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_AC4_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_AC4DmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    MT_S32           s32Ret = MT_FAILURE;
    mt_sys_version_s stSysChipInfo;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = 0x%x\n", s32Ret);
        return s32Ret;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_AC4_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = 0x%x\n", s32Ret);
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
            SAMPLE_AC4_ERR_PRINT("failed to mt_sys_get_version\n");
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
            SAMPLE_AC4_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_AC4DmxDeInit(MT_VOID)
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
static MT_S32 MT_AC4AvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE* phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = MT_INVALID_HANDLE;
    MT_HANDLE                hWin = MT_INVALID_HANDLE;
    MT_HANDLE                hSoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == phAvplay)
    {
        SAMPLE_AC4_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }

    if(NULL == phWin)
    {
        SAMPLE_AC4_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }

    if(NULL == phSoundTrack)
    {
        SAMPLE_AC4_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MTADP_AVPlay_RegADecLib failed ret=0x%x \n", ret);
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_Init failed ret=0x%x \n", ret);
        return ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed ret=0x%x \n", ret);
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
        SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_Create failed ret=0x%x \n", ret);
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed ret=0x%x \n", ret);
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed ret=0x%x \n", ret);
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed ret=0x%x \n", ret);
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_SND_CreateTrack failed ret=0x%x \n", ret);
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_SND_Attach failed ret=0x%x \n", ret);
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MTADP_VO_CreatWin failed ret=0x%x \n", ret);
        goto ERR6;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_VO_AttachWindow failed ret=0x%x \n", ret);
        goto ERR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed ret=0x%x \n", ret);
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
static MT_S32 MT_AC4StopToPlay(MT_HANDLE hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AC4_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;
    SAMPLE_AC4_INFO_PRINT("stop live play ...\n");
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
static MT_VOID MT_AC4AvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AC4_ERR_PRINT("phAvplay is null.\n");

    }

    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_AC4_ERR_PRINT("phWin is null.\n");

    }

    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_AC4_ERR_PRINT("phSoundTrack is null.\n");

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
static MT_S32 MT_AC4SetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
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
        SAMPLE_AC4_ERR_PRINT("The input address is empty\n");
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

    SAMPLE_AC4_INFO_PRINT("VidPid = %#x, AudPid = %#x\n", VidPid, AudPid);

    if(VidPid != INVALID_TSPID)
    {
        /** Get the video properties of the AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed ret=0x%x \n", ret);
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
            SAMPLE_AC4_ERR_PRINT("Set video properties or video PID property failed\n");
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
            SAMPLE_AC4_ERR_PRINT("Setting the decoding mode or audio PID property failed\n");
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
            SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed ret=0x%x \n", ret);
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
static MT_S32 MT_AC4StarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AC4_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    if (NULL == pProgInfo)
    {
        SAMPLE_AC4_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return MT_FAILURE;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret=MT_AC4SetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_AC4SetAvplayPidAndCodecType fail! \n");
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
        SAMPLE_AC4_ERR_PRINT("Has no audio stream! ret=0x%x \n", ret);
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_AC4_ERR_PRINT("Has no video stream! ret=0x%x \n", ret);
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
            SAMPLE_AC4_ERR_PRINT("Set frame to VO is failed ret=0x%x \n", ret);
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("Get avplay sync attr is failed ret=0x%x \n", ret);
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
            SAMPLE_AC4_ERR_PRINT("Set avplay sync attr is failed ret=0x%x \n", ret);
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  =MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_UNF_AVPLAY_Start failed ret=0x%x \n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

#endif

static MT_VOID MT_AC4Exit(void)
{
    g_bTaskQuit = MT_TRUE;

#ifndef MT_SAMPLE_APP
    /** Stop playing the show */
    (MT_VOID)MT_AC4StopToPlay(g_ac4_run_info.hAvPlay);

    /** Audio and video player deinitialization */
    (MT_VOID)MT_AC4AvplayDeInit(g_ac4_run_info.hAvPlay, g_ac4_run_info.hWin, g_ac4_run_info.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_ac4_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == g_ac4_run_info.sInputParam.sig_type)
    {
        /** Wait for the thread to end */
        pthread_join(g_ac4_run_info.stInjectTSThread, NULL);
    }

    (MT_VOID)MT_AC4DmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != g_ac4_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
#endif
    memset(&g_ac4_run_info, 0xff, sizeof(g_ac4_run_info));
}


/*!
@brief Help information
@return::void
@*/
static void MT_AC4Print_Help(MT_CHAR *name)
{
#ifndef MT_SAMPLE_APP
    MT_AC4_PRINT("Lack of parameters\n");
    MT_AC4_PRINT("\nUsage:\n");
    MT_AC4_PRINT("%s\n", name);
    MT_AC4_PRINT("    -f: path of the stream file\n");
    MT_AC4_PRINT("    -c: DVBC locks frequency\n");
    MT_AC4_PRINT("    -s: DVBS locks frequency\n");
    MT_AC4_PRINT("example:\n");
    MT_AC4_PRINT("    %s -f ./677-CC-12.ts\n", name);
    MT_AC4_PRINT("    %s -c 314 6875 64\n", name);
    MT_AC4_PRINT("    %s -s 3840 27500 1 0 0\n", name);
#else
    MT_AC4_PRINT("    -q: Exit the background\n");
#endif
}




/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_AC4Parase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

#ifndef MT_SAMPLE_APP
    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_AC4Print_Help(argv[0]);
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
                (MT_VOID)MT_AC4Print_Help(argv[0]);
                return MT_FAILURE;
#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_AC4Print_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_AC4Print_Help(argv[0]);
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
                    (MT_VOID)MT_AC4Print_Help(argv[0]);
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
                    (MT_VOID)MT_AC4Exit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_AC4Print_Help(argv[0]);
                return MT_FAILURE;

        }
    }


    return MT_SUCCESS;

}


static MT_VOID MT_AC4PrintMenu(mt_s32 prognum)
{
    SAMPLE_AC4_PRINT("commond: \n");
#ifndef MT_SAMPLE_APP
    SAMPLE_AC4_PRINT("'1-%d': Select a program \n", prognum);
#endif
    SAMPLE_AC4_PRINT("     d: set AC4 downmix: (0: LtRt, 1: LoRo, 2:PCM_5_1, 3:PCM_RAW)\n");
    SAMPLE_AC4_PRINT("     e: set Dialogue Enhancement value: 0 ~ 12dB\n");
    SAMPLE_AC4_PRINT("     o: set DD/DDP/MAT encoder output type: (0:None 1:DD 2:DDP 3:mat 4:auto)\n");
    SAMPLE_AC4_PRINT("     a: set AD on/off: (0:disable 1:enable)\n");
    SAMPLE_AC4_PRINT("     v: set AD volume weight: -32 ~ +32dB, -32dB indicates main only\n");
    SAMPLE_AC4_PRINT("     f: ad Preferred config, 0: over lang, 1: over content type. \n");
    SAMPLE_AC4_PRINT("        ad content type: 0:undefined, 1:visually impaired, 2:hearing impaired\n");
    SAMPLE_AC4_PRINT("        ad content type: 3:commentary, 4:spoken subtitles, 5:ad with spoken sub\n");
    SAMPLE_AC4_PRINT("     l: sel language, 3 Letter ISO 639, e.g:chi, eng\n");
    SAMPLE_AC4_PRINT("     p: set presentation id: -1 ~ n, -1: auto, n: presentation id\n");
    SAMPLE_AC4_PRINT("     t: set DAP encoder output: (0:disable 1: DAP Speaker 2: DAP Headphone)\n");
    SAMPLE_AC4_PRINT("     s: show audio preselection descriptor information\n");
    SAMPLE_AC4_PRINT("     m: show timely audio message descriptor information\n");
    SAMPLE_AC4_PRINT("     n: show timely audio preselection descriptor information\n");
    SAMPLE_AC4_PRINT("     c: force ms12 decode dolby.\n");

#ifdef MT_SAMPLE_APP
    SAMPLE_AC4_PRINT("     b: background run \n");
#endif
    SAMPLE_AC4_PRINT("     q: quit \n");
    SAMPLE_AC4_PRINT("     h: help \n");
    SAMPLE_AC4_PRINT("AC4>> ");
}

/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_VOID MT_AC4CmdTask(MT_HANDLE hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_S32                 ret = MT_FAILURE;
    MT_CHAR                inPutCmd[32] = { 0 };
#ifdef MT_SAMPLE_APP
    PMT_COMPACT_PROG *stCurrentProgInfo = NULL;
#else
    mt_u32     u32ProgNum = 0;
    PMT_COMPACT_PROG *stCurrentProgInfo = pProgTbl->proginfo;
    struct timespec start, end;
    double elapsed_seconds;
#endif
    MT_S32  input_val;
    MT_S32  dolby_mode = 0;
    play_ac4_attr_info play_ac4_info;

#ifndef MT_SAMPLE_APP
    if(MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_AC4_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }
#endif

    while(1)
    {
        (void)MTADP_AUD_GetAc4PlayAttrInfo(&play_ac4_info);
        (MT_VOID)MT_AC4PrintMenu(pProgTbl->prog_num);
        fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);

        if('q' == inPutCmd[0])
        {
            SAMPLE_AC4_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inPutCmd[0])
        {
            SAMPLE_AC4_INFO_PRINT("AC4 play in back!\n");
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
            ret = MT_AC4StopToPlay(hAvPlay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_AC4_ERR_PRINT("MT_AC4StopToPlay failed ret=0x%x \n", ret);
            }
            MT_AC4_PRINT("===== Start play ProgNum: %d \n", u32ProgNum);

            // restore ac4    attr info.
            MTADP_AUD_RestoreAc4PlayAttrInfo(hAvPlay);

            /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
            ret = MT_AC4StarToPlay(hAvPlay, pstCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_AC4_ERR_PRINT("Switching shows failed\n");
            }
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            MT_AC4_PRINT("Switching time = %lf s\n", elapsed_seconds);
        }
#endif
        else if ('d' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input AC4 downmix type: 0: LtRt, 1: LoRo, 2:PCM_5_1, 3:PCM_RAW:\n");
            scanf("%d",&input_val);
            if(input_val >= 0 && input_val <= 3) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_DOWNMIX_MODE, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Set AC4 downmix mode:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1;
                play_ac4_info.downmix_type = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input downmix type value is out of range!!!!!!\n");
            }
        }
        else if ('e' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input Dialogue Enhancement value: 0 ~ 12dB:\n");
            scanf("%d",&input_val);
            if(input_val >= 0 && input_val <= 12) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_DIALOGUE_ENHANCEMENT, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Set AC4 Dialogue Enhancement:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 1;
                play_ac4_info.dialogue_enhancement_value = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input Dialogue Enhancement value is out of range!!!!!!\n");
            }
        }
        else if ('o' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input DD/DDP/MAT encoder output type: (0:None 1:DD 2:DDP 3:mat 4:auto):\n");
            scanf("%d",&input_val);
            if(input_val >= 0 && input_val <= 4) {
                MT_S32  mat_en = 0;
                if(input_val == 4) {
                    MT_S32  i = 0;
                    MT_S32  cap = 0;
                    MT_UNF_EDID_BASE_INFO_S stSinkCap;

                    ret = MT_UNF_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_0, &stSinkCap);
                    if(ret != MT_SUCCESS)
                    {
                        SAMPLE_AC4_INFO_PRINT("call MT_UNF_HDMI_GetSinkCapability failed.\n");
                        return;
                    }

                    for(i = stSinkCap.u32AudioInfoNum - 1; i >= 0; i--)
                    {
                        if(stSinkCap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT)
                        {
                            cap |= 1<<3;//bit 3
                            SAMPLE_AC4_INFO_PRINT("HDMI support MAT\n");
                        }
                        else if(stSinkCap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP)
                        {
                            cap |= 1<<2;//bit 2
                            SAMPLE_AC4_INFO_PRINT("HDMI support DD+\n");
                        }
                        else if(stSinkCap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3)
                        {
                            cap |= 1<<1;//bit 1
                            SAMPLE_AC4_INFO_PRINT("HDMI support DD\n");
                        }
                        else if(stSinkCap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM)
                        {
                            cap |= 1;//bit 0
                            SAMPLE_AC4_INFO_PRINT("HDMI support PCM\n");
                        }
                    }

                    if ( stSinkCap.bSupportDdMat48k ) {
                        cap |= 1<<3;//bit 3
                        SAMPLE_AC4_INFO_PRINT("HDMI support MAT 48K only\n");
                    }

                    if(cap & (1<<3)) {
                        mat_en = 1;
                        SAMPLE_AC4_INFO_PRINT("HDMI auto mat\n");
                    } else if(cap & (1<<2)) {
                        dolby_mode = 2;
                        SAMPLE_AC4_INFO_PRINT("HDMI auto DDP\n");
                    } else if(cap & (1<<1)) {
                        dolby_mode = 1;
                        SAMPLE_AC4_INFO_PRINT("HDMI auto DD\n");
                    } else if(cap & 1) {
                        dolby_mode = 0;
                        SAMPLE_AC4_INFO_PRINT("HDMI auto PCM\n");
                    }

                    if(mat_en == 1) {
                        ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_MAT, &mat_en);
                        if(MT_SUCCESS != ret){
                            SAMPLE_AC4_ERR_PRINT("Set AC4 MAT encoder enable failed ret=0x%x \n",ret);
                        }
                    } else {
                        ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_DD_DDP, &dolby_mode);
                        if(MT_SUCCESS != ret){
                            SAMPLE_AC4_ERR_PRINT("Set AC4 DD/DDP encoder output type:%d failed ret=0x%x \n",input_val, ret);
                        }
                    }
                } else if(input_val == 3) {
                    mat_en = 1;
                    ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_MAT, &mat_en);
                    if(MT_SUCCESS != ret){
                        SAMPLE_AC4_ERR_PRINT("Set AC4 MAT encoder enable failed ret=0x%x \n",ret);
                    }
                    SAMPLE_AC4_INFO_PRINT("HDMI support MAT\n");
                } else {
                    dolby_mode = input_val;
                    ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_DD_DDP, &dolby_mode);
                    if(MT_SUCCESS != ret){
                        SAMPLE_AC4_ERR_PRINT("Set AC4 DD/DDP encoder output type:%d failed ret=0x%x \n",input_val, ret);
                    }
                }

                play_ac4_info.ac4_attr_enable |= 1 << 2;
                play_ac4_info.encoder_output_type = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input DD/DDP encoder output type is out of range!!!!!!\n");
            }
        }
        else if ('a' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input AD on/off:(0:off 1:on):\n");
            scanf("%d",&input_val);
            if(input_val >= 0 && input_val <= 1) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_ONOFF, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Set AC4 AD on/off:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 4;
                play_ac4_info.ad_value = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input set AD on/off value is out of range!!!!!!\n");
            }
        }
        else if ('v' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input AD volume weight: -32 ~ +32dB, -32dB indicates main only:\n");
            scanf("%d",&input_val);
            if(input_val >= -32 && input_val <= 32) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_VOL_WEIGHT, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Set AC4 AD volume weight:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 5;
                play_ac4_info.ad_volume_value = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input set AD volume weight value is out of range!!!!!!\n");
            }
        }
        else if ('f' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input AC4 AD perferred config, 0: over lang, 1: over content type:\n");
            scanf("%d",&input_val);
            if(input_val == 0) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE_OVER_LANG, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Sel AC4 AD type over lang:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 6;
                play_ac4_info.ad_content_type_over_lang_value = input_val;
            } else {
                SAMPLE_AC4_PRINT("input AC4 AD content type:\n");
                scanf("%d",&input_val);
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Sel AC4 AD content type:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 7;
                play_ac4_info.ad_content_type = input_val;
            }
        }

        else if ('l' == inPutCmd[0])
        {
            MT_UNF_AVPLAY_AC4_LANG_S ac4_lang;
            memset(&ac4_lang, 0, sizeof(MT_UNF_AVPLAY_AC4_LANG_S));
            SAMPLE_AC4_PRINT("input AC4 1st language: e.g: chi, eng:\n");
            scanf("%3s",ac4_lang.ac4_1st_lang);

            SAMPLE_AC4_PRINT("input AC4 2nd language: e.g: chi, eng:\n");
            scanf("%3s",ac4_lang.ac4_2nd_lang);

            ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_LANG, &ac4_lang);
            if(MT_SUCCESS != ret){
                SAMPLE_AC4_ERR_PRINT("Set AC4 language failed ret=0x%x \n", ret);
            }

            play_ac4_info.ac4_attr_enable |= 1 << 8;
            play_ac4_info.ac4_lang = ac4_lang;
        }
        else if ('p' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input AC4 presentation id:\n");
            scanf("%d",&input_val);
            if(input_val >= -1) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_SET_PRES_ID, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Set AC4 presentation id:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 9;
                play_ac4_info.presentation_id = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input presentation id value is out of range!!!!!!\n");
            }
        }
        else if ('t' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("input DAP encoder output: (0:disable 1: DAP Speaker 2: DAP Headphone):\n");
            scanf("%d",&input_val);
            if(input_val >= 0 && input_val <= 2) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AC4_SET_ENCODE_DAP, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("Set AC4 DAP encoder output type:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 10;
                play_ac4_info.speaker_value = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input presentation id value is out of range!!!!!!\n");
            }
        }
        else if('s' == inPutCmd[0])
        {
            mt_u32 i = 0;
            mt_u32 cur_apid = 0x1fff;
            mt_s32 tracknum = 0;

            ret = MTADP_Get_Current_Info(&stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_AC4_ERR_PRINT("MTADP_Get_Current_Info ERR !!\n");
            }

            if (stCurrentProgInfo->AElementNum > 0)
            {
                ret = MT_UNF_AVPLAY_GetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &cur_apid);
                if (MT_SUCCESS == ret)
                {
                    for(i = 0; i< stCurrentProgInfo->AElementNum; i++)
                    {

                        if(cur_apid ==  stCurrentProgInfo->Audioinfo[i].u16AudioPid)
                        {
                            tracknum = i;
                            SAMPLE_AC4_INFO_PRINT("find current audio track[%d] pid=0x%x\n",i,cur_apid);
                            break;
                        }
                    }
                }
                else
                {
                    SAMPLE_AC4_ERR_PRINT("call get MT_UNF_AVPLAY_ATTR_ID_AUD_PID faild, ret = 0x%x \n", ret);
                }

                if (1 == stCurrentProgInfo->Audioinfo[tracknum].has_audio_preselection)
                {
                    MT_AC4PrintAudioPreselectionDescriptor(&stCurrentProgInfo->Audioinfo[tracknum].audio_preselection_descriptor);
                }
             }
        }
        else if ('c' == inPutCmd[0])
        {
            SAMPLE_AC4_PRINT("force ms12 decode dolby:(0:disable 1:enable)\n");
            scanf("%d",&input_val);
            if(input_val >= 0 && input_val <= 1) {
                ret = MT_UNF_AVPLAY_SetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_DOLBY_FORCE_MS12_DEC, &input_val);
                if(MT_SUCCESS != ret){
                    SAMPLE_AC4_ERR_PRINT("force ms12 decode dolby:%d failed ret=0x%x \n",input_val, ret);
                }

                play_ac4_info.ac4_attr_enable |= 1 << 11;
                play_ac4_info.ms12_decode_value = input_val;
            } else {
                SAMPLE_AC4_FATAL_PRINT("The input value is out of range!!!!!!\n");
            }
        }
        else if('m' == inPutCmd[0])
        {
            SDT_TB sdt_tb;

            memset(&sdt_tb, 0, sizeof(SDT_TB));
            ret = SRH_SDTRequest(DMX_ID_0, &sdt_tb);
            if (ret != MT_SUCCESS)
            {
                return MT_FAILURE;
            }

            MT_AC4PrintAudioMessageDescriptor(sdt_tb);
        }
        else if('n' == inPutCmd[0])
        {
            PMT_COMPACT_TBL *ProgTbl = NULL;
            mt_u32 i = 0;
            mt_u32 cur_apid = 0x1fff;
            mt_s32 tracknum = 0;
            PMT_TB pmt_tb;

            ret = MTADP_Get_Current_Info(&stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_AC4_ERR_PRINT("MTADP_Get_Current_Info ERR !!\n");
            }

            if (stCurrentProgInfo->AElementNum > 0)
            {
                ret = MT_UNF_AVPLAY_GetAttr(hAvPlay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &cur_apid);
                if (MT_SUCCESS == ret)
                {
                    for(i = 0; i< stCurrentProgInfo->AElementNum; i++)
                    {

                        if(cur_apid ==  stCurrentProgInfo->Audioinfo[i].u16AudioPid)
                        {
                            tracknum = i;
                            SAMPLE_AC4_INFO_PRINT("find current audio track[%d] pid=0x%x\n",i,cur_apid);
                            break;
                        }
                    }
                }
                else
                {
                    SAMPLE_AC4_ERR_PRINT("call get MT_UNF_AVPLAY_ATTR_ID_AUD_PID faild, ret = 0x%x \n", ret);
                }

                memset(&pmt_tb, 0, sizeof(PMT_TB));
                ret = SRH_PMTRequest(DMX_ID_0, &pmt_tb, stCurrentProgInfo->PmtPid, stCurrentProgInfo->ProgID);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_AC4_ERR_PRINT("failed to search PMT\n");
                }

                if (1 == pmt_tb.Audioinfo[tracknum].has_audio_preselection)
                {
                    MT_AC4PrintAudioPreselectionDescriptor(&pmt_tb.Audioinfo[tracknum].audio_preselection_descriptor);
                }
             }
        }
        else if('h' == inPutCmd[0])
        {
            SAMPLE_AC4_INFO_PRINT("Print help info\n");
        }

        (void)MTADP_AUD_SetAc4PlayAttrInfo(play_ac4_info);
    }


}


#ifdef MT_SAMPLE_APP
MT_S32 MT_AC4Main(MT_S32 argc, MT_CHAR *argv[])
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
    ret = MT_AC4Parase_args(argc, argv, &g_ac4_run_info.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_AC4_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_AC4_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
    #ifndef MT_SAMPLE_APP
        /** System initialization */
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("failed to mt_sys_init\n");
            return ret;
        }

        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("failed to StartDmx\n");
            goto ERR1;
        }

        sleep(1);

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }

        g_bTaskQuit = MT_FALSE;
        clock_gettime(CLOCK_MONOTONIC, &start);
        /** Tuner initialization, Set the default parameters for tuner */
        if(MT_INPUT_SIG_TYPE_FILE != g_ac4_run_info.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_AC4_ERR_PRINT("MTADP_Fe_Init failed ret=0x%x \n", ret);
                goto ERR3;
            }

            if(MT_INPUT_SIG_TYPE_CAB == g_ac4_run_info.sInputParam.sig_type)
            {     //dvbc
                ret = MT_AC4CheckDvbcParam(&g_ac4_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_AC4_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR4;
                }

                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            g_ac4_run_info.sInputParam.input_param.cab.freq,
                                            g_ac4_run_info.sInputParam.input_param.cab.sym_rate,
                                            g_ac4_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_ac4_run_info.sInputParam.sig_type)
            {
                ret = MT_AC4CheckDvbsParam(&g_ac4_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_AC4_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR4;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            g_ac4_run_info.sInputParam.input_param.sat.freq,
                                            g_ac4_run_info.sInputParam.input_param.sat.sym_rate,
                                            g_ac4_run_info.sInputParam.input_param.sat.onoff_22k,
                                            g_ac4_run_info.sInputParam.input_param.sat.polarization,
                                            g_ac4_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_AC4_ERR_PRINT("MTADP_Fe_Connect failed ret=0x%x \n", ret);
                goto ERR4;
            }
        }

        /** VO device initialization */
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("MTADP_VO_Init failed ret=0x%x \n", ret);
            goto ERR4;
        }

        /** Audio device initialization */
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("MTADP_Snd_Init failed ret=0x%x \n", ret);
            goto ERR5;
        }

        ret = MT_AC4DmxInit(g_ac4_run_info.sInputParam.sig_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("MT_AC4DmxInit failed ret=0x%x \n", ret);
            goto ERR6;
        }

        if(MT_INPUT_SIG_TYPE_FILE == g_ac4_run_info.sInputParam.sig_type)
        {
            ret = pthread_create(&g_ac4_run_info.stInjectTSThread, NULL, (void * (*)(void *))InjectTsTask, &g_ac4_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_AC4_ERR_PRINT("failed to pthread_create\n");
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
        while(MT_SUCCESS != MTADP_Search_GetAllPmt(DMX_ID_0, &g_ac4_run_info.pProgTbl))
        {
            count++;
            SAMPLE_AC4_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            MT_USLEEP(100000);
            if(20 == count)
            {
                goto ERR9;
            }
        }

        /** audio and video player initialization */
        ret = MT_AC4AvplayInit(&g_ac4_run_info.hAvPlay, &g_ac4_run_info.hWin, &g_ac4_run_info.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("MT_AC4AvplayInit failed ret=0x%x \n", ret);
            goto ERR10;
        }

        pstCurrentProgInfo = g_ac4_run_info.pProgTbl->proginfo;
        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        ret = MT_AC4StarToPlay(g_ac4_run_info.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("MT_AC4StarToPlay failed ret=0x%x \n", ret);
            goto ERR11;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        MT_AC4_PRINT("Search time = %lf s\n", elapsed_seconds);
    #endif
        g_bTaskQuit = MT_FALSE;
    }

#ifdef MT_SAMPLE_APP
    g_ac4_run_info.hAvPlay = avplayHandle.hAvPlay;
    g_ac4_run_info.hSoundTrack = avplayHandle.hSoundTrack;
#endif

    if (g_ac4_run_info.hSoundTrack <= 0)
    {
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("failed to MTADP_Snd_Init\n");
            return MT_FAILURE;
        }
    }

    if(MT_INVALID_HANDLE == g_ac4_run_info.hAvPlay || 0 == g_ac4_run_info.hAvPlay) {
        ret =MT_UNF_AVPLAY_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_Init error!\n", __func__, __LINE__);
            return MT_FAILURE;
        }

        memset(&AvplayAttr,0,sizeof(MT_UNF_AVPLAY_ATTR_S));
        ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
        ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &g_ac4_run_info.hAvPlay);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_Create error!\n", __func__, __LINE__);
            goto ERR0;
        }

        ret = MT_UNF_AVPLAY_ChnOpen(g_ac4_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_AC4_ERR_PRINT("<%s:%d> MT_UNF_AVPLAY_ChnOpen error!\n", __func__, __LINE__);
            goto ERR1;
        }

        (MT_VOID)MT_AC4CmdTask(g_ac4_run_info.hAvPlay, g_ac4_run_info.pProgTbl);

        (MT_VOID)MT_UNF_AVPLAY_ChnClose(g_ac4_run_info.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
        ERR1:
            (MT_VOID)MT_UNF_AVPLAY_Destroy(g_ac4_run_info.hAvPlay);
        ERR0:
            (MT_VOID)MT_UNF_AVPLAY_DeInit();
    } else {
        (MT_VOID)MT_AC4CmdTask(g_ac4_run_info.hAvPlay, g_ac4_run_info.pProgTbl);
    }

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifndef MT_SAMPLE_APP
    /** Stop AV playback and enter the stop state */
    ret = MT_AC4StopToPlay(g_ac4_run_info.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AC4_ERR_PRINT("MT_AC4StopToPlay failed ret=0x%x \n", ret);
    }

ERR11:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_AC4AvplayDeInit(g_ac4_run_info.hAvPlay, g_ac4_run_info.hWin, g_ac4_run_info.hSoundTrack);

ERR10:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_ac4_run_info.pProgTbl);
ERR9:
    (MT_VOID)MTADP_Search_DeInit();
ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == g_ac4_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(g_ac4_run_info.stInjectTSThread, NULL);
    }
ERR7:
    /** Demux module deinitialization */
    (MT_VOID)MT_AC4DmxDeInit();
ERR6:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

ERR5:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    /** Disconnect the tuner lock */
    if(MT_INPUT_SIG_TYPE_FILE != g_ac4_run_info.sInputParam.sig_type)
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
    memset(&g_ac4_run_info, 0xff, sizeof(g_ac4_run_info));

    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}
