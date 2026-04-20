/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_frontend.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_demux.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"


/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_PIP_PLAY_DEBUG

#define MT_PIP_PLAY_PRINT   printf
#else

#define MT_PIP_PLAY_PRINT

#endif

#define SAMPLE_PIP_PLAY_FUNCTION_ENTER()        MT_PIP_PLAY_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_PIP_PLAY_FUNCTION_EXIT()           MT_PIP_PLAY_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_PIP_PLAY_FATAL_PRINT(fmt...)       MT_PIP_PLAY_PRINT(" [FATAL] " fmt)
#define SAMPLE_PIP_PLAY_ERR_PRINT(fmt...)         MT_PIP_PLAY_PRINT(" [ERROR] " fmt)
#define SAMPLE_PIP_PLAY_WARN_PRINT(fmt...)        MT_PIP_PLAY_PRINT(" [WARN] "  fmt)
#define SAMPLE_PIP_PLAY_INFO_PRINT(fmt...)        MT_PIP_PLAY_PRINT(" [INFO] "  fmt)
#define SAMPLE_PIP_PLAY_DBG_PRINT(fmt...)         MT_PIP_PLAY_PRINT(" [DEBUG] " fmt)

#define SAMPLE_PIP_PLAY_PRINT   printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0 0
#define DMX_ID_1 1

#define TUNER_ID_0 (0)
#define INVALID_TSPID (0x1fff)
#define DEFAULT_PIP_PLAY        80

#define SAMPLE_PIP_PLAY_DEFAULTT_X   40
#define SAMPLE_PIP_PLAY_DEFAULTT_Y   20
#define SAMPLE_PIP_PLAY_DEFAULTT_W   480
#define SAMPLE_PIP_PLAY_DEFAULTT_H   360





/*************************** Structure Definition ****************************/
typedef enum input_sig_type_t {
    MT_INPUT_SIG_TYPE_CAB = 1,
    /**<Cable signal*/ /**<CNcomment:DVB_C信号*/
    MT_INPUT_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/ /**<CNcomment:卫星信号*/
    MT_INPUT_SIG_TYPE_DVB_T = 3,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_INPUT_SIG_TYPE_J83B = 4,
    /**<Terrestrial signal*/ /**<CNcomment:J83B信号*/
    MT_INPUT_SIG_TYPE_FILE = 5,
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
    mt_s32  tuner_id;
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
    mt_input_para_t sInputParam;
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    pthread_t          htsThd;
    PMT_COMPACT_TBL  *pProgTbl;

} MT_PipPlay_RUN_INFO;

typedef struct
{
    MT_HANDLE hAvPlay;
    MT_HANDLE hWin;

    mt_input_para_t sInputParam;
    pthread_t          htsPipThd;
    PMT_COMPACT_TBL  *pProgTbl;
    MT_U32 progBase ;
} mt_pip_play_info;


/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_BOOL g_bPipTaskQuit = MT_TRUE;

static mt_pip_play_info g_pipPlayInfo;


static MT_PipPlay_RUN_INFO    g_stPipPlayMainRunInfo;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_PipPlayMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


static mt_s32 MT_PipPlayCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_PIP_PLAY_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_PipPlayCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_PIP_PLAY_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_PipPlayJ83bCheckParam(mt_input_cab_para_t *p_j83b)
{
    if (p_j83b->freq < 45 || p_j83b->freq > 862)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if (p_j83b->sym_rate < 5057 || p_j83b->sym_rate > 7560)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("srate error. srate = %d \n", p_j83b->sym_rate);
        SAMPLE_PIP_PLAY_ERR_PRINT("The symbol rate is out of range.\n");
        return MT_FAILURE;
    }

    if (p_j83b->mod_type != 256 && p_j83b->mod_type != 64)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("QAM error. QAM = %d \n", p_j83b->mod_type);
        SAMPLE_PIP_PLAY_ERR_PRINT("QAM must be set 256 or 64.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_PipPlayDvbtCheckParam(mt_input_ter_para_t *p_dvbt)
{
    if ((p_dvbt->freq) > 900 || (p_dvbt->freq) < 50)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("freq error. freq = %d \n", p_dvbt->freq);
        SAMPLE_PIP_PLAY_ERR_PRINT("freq must be more than 50 and less than 900.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_PipPlayDmxInit(MT_U32 dmx_id, mt_input_para_t *pInputParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();
#ifndef MT_SAMPLE_APP

    s32Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }
#endif

    if(MT_INPUT_SIG_TYPE_FILE == pInputParam->sig_type)
    {
    /*
        s32Ret = MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_RAM_0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            (mt_void) MT_UNF_DMX_DeInit();
            return MT_FAILURE;
        }
    */
    }
    else
    {
         memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to mt_sys_get_version\n");
            return MT_FAILURE;
        }

        if(MT_INPUT_SIG_TYPE_CAB == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_1);
            }
            else
            {
                MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_0);
            }
        }
        else if(MT_INPUT_SIG_TYPE_SAT == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                if(pInputParam->tuner_id == 0)
                {
                    /** Bind Demux to tuner port 0 */
                    s32Ret = MT_UNF_DMX_AttachTSPort(dmx_id, DMX_DVB_TSI_IN_PORT);

                    SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 0!\n");
                }
                else if(pInputParam->tuner_id == 1)
                {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
                    /** Bind Demux to tuner port 3 */
                    s32Ret = MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_3);
                    SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 3!\n");
#else
                    /** Bind Demux to tuner port 1 */
                    ret = MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_1);
                    SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 1!\n");
#endif
                }
                else
                {
                    /** Bind Demux to tuner port 0 */
                    s32Ret = MT_UNF_DMX_AttachTSPort(dmx_id, DMX_DVB_TSI_IN_PORT);

                    SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 0!\n");
                }

            }
            else
            {
                MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_1);
            }
        }
        else if(MT_INPUT_SIG_TYPE_DVB_T == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_2);
                SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 2!\n");
            }
            else
            {
                MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_0);
                SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 0!\n");
            }
        }
        else if(MT_INPUT_SIG_TYPE_J83B == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                /** Bind Demux to tuner port 1 */
                MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_1);
                SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 1!\n");
            }
            else
            {
                /** Bind Demux to tuner port 0 */
                MT_UNF_DMX_AttachTSPort(dmx_id, MT_UNF_DMX_PORT_TSI_0);
                SAMPLE_PIP_PLAY_INFO_PRINT("Connect port 0!\n");
            }
        }
    }

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();


    return MT_SUCCESS;
}

static MT_VOID MT_PipPlayDmxSearchDeInit(MT_U32 dmx_id)
{
    (MT_VOID)MT_UNF_DMX_DetachTSPort(dmx_id);
}

/*
 @brief DmxDeinit and detachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_VOID MT_PipPlayDmxDeInit(MT_VOID)
{
#ifndef MT_SAMPLE_APP

   (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

   (MT_VOID)MT_UNF_DMX_DeInit();
#endif
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
static mt_s32 MT_PipPlayAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }

    if(NULL == p_hSoundTrack)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
 @return ::(MT_VOID)
*/
static MT_VOID  MT_PipPlayMainPlayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("phAvplay is null.\n");

    }

    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("phWin is null.\n");

    }

    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("phSoundTrack is null.\n");

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
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_PipPlayInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    mt_input_file_para_t *pstParam = (mt_input_file_para_t *)(args);

    SAMPLE_PIP_PLAY_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT( " file %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        fclose(pTsFile);
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
        fclose(pTsFile);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*448, &StreamBuf, 1000);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT( "failed to MT_UNF_DMX_GetTSBuffer  ret= %x \n", ret);
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_PIP_PLAY_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_PIP_PLAY_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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


static mt_void *MT_PipPlayEsTthread(char * file_name)
{
    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_s32 Readlen; //read() may return < 0
    mt_u32 Readlen8;
    mt_s32 Ret;
    FILE *m_pVidFile = fopen(file_name, "rb");

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    SAMPLE_PIP_PLAY_INFO_PRINT("Opne file : %s \n", file_name);

    if(NULL == m_pVidFile)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("open file [%s]  failed return \n", file_name);
        return NULL;
    }

    while (!g_bPipTaskQuit)
    {

        Ret = MT_UNF_AVPLAY_GetBuf(g_pipPlayInfo.hAvPlay, MT_UNF_AVPLAY_BUF_ID_ES_VID, 0x10000, &StreamBuf, 0);
        if (MT_SUCCESS == Ret
            && StreamBuf.u32Size > 0
            && StreamBuf.pu8Data != NULL)
        {
            memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);
            Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, m_pVidFile);
            //printf("EsTthread: Readlen %d\n", Readlen);
            if (Readlen > 0)
            {
                //FIXME: fread segment fault!
                //buffer must aligned with 16k and stuff with 0xff?
                //must put 16k buffer each time! NOT Readlen!
                //Ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, Readlen, 0);

                //it's better to align 8 byte for hw vdec
                //推荐8字节对齐
                //如果不是完整的一帧数据,添加填充数据反而会出马赛克
                Readlen8 = StreamBuf.u32Size;

                //stuff zero
                if (Readlen < Readlen8)
                {
                    memset(StreamBuf.pu8Data+Readlen, 0, Readlen8-Readlen);
                }

                Ret = MT_UNF_AVPLAY_PutBuf(g_pipPlayInfo.hAvPlay, MT_UNF_AVPLAY_BUF_ID_ES_VID, Readlen8, 0);
                if (Ret != MT_SUCCESS)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT("call MT_UNF_AVPLAY_PutBuf failed.\n");
                }
            }
            else
            {
                rewind(m_pVidFile);
            }
        }

        MT_USLEEP(1000 * 10);
    }

    if (m_pVidFile != MT_NULL)
    {
        fclose(m_pVidFile);
        m_pVidFile = MT_NULL;
    }

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();
}

static MT_S32 MT_PipPlayReSetMainWindow(MT_HANDLE hWin, mt_u32 scalerMode, MT_BOOL bEnable)
{

    MT_UNF_DISP_SetSdScalerEnable((MT_UNF_DISP_SCALER_MODE_E)scalerMode);
    MT_UNF_WINDOW_ATTR_S pMainWinAttr;
    MT_UNF_VO_GetWindowAttr(hWin, &pMainWinAttr);
    pMainWinAttr.bSetVideoBot = bEnable;
    MT_UNF_VO_SetWindowAttr(hWin, &pMainWinAttr);

    return MT_SUCCESS;
}

static MT_S32 MT_PipPlaySetWindowLocation(MT_HANDLE hWin, MT_S32 x, MT_S32 y, MT_U32 w, MT_U32 h)
{
    MT_S32                   ret = MT_FAILURE;

    MT_UNF_WINDOW_ATTR_S stWinAttr = {0};

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    ret = MT_UNF_VO_GetWindowAttr(hWin, &stWinAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_VO_GetWindowAttr failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_PIP_PLAY_INFO_PRINT("play window Input rect(%d, %d, %d, %d)...\n", stWinAttr.stInputRect.s32X, stWinAttr.stInputRect.s32Y, stWinAttr.stInputRect.s32Width, stWinAttr.stInputRect.s32Height);

    SAMPLE_PIP_PLAY_INFO_PRINT("Set pip play window location(%d, %d, %d, %d)...\n", x, y, w, h);

    stWinAttr.stOutputRect.s32X = x;
    stWinAttr.stOutputRect.s32Y = y;
    stWinAttr.stOutputRect.s32Width = w;
    stWinAttr.stOutputRect.s32Height = h;
    stWinAttr.bUseSubLayer = 1;
    stWinAttr.bSetVideoBot = 1;

    ret = MT_UNF_VO_SetWindowAttr(hWin, &stWinAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_VO_SetWindowAttr failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();

    return ret;
}

static MT_S32 MT_PipPlayAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = 0;
    MT_HANDLE                hWin = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    if(NULL == phAvplay || NULL == phWin )
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("The input address is empty!\n");
        return ret;
    }


    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_1;
    AvplayAttr.stStreamAttr.u32VidBufSize = 0x400000;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.vdec_pip_chan = 1;
    /** Create AV player based on attributes */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Create a window */
    ret = MTADP_VO_CreatPipWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_VO_AttachWindow failed, ret = %x\n", ret);
        goto ERROR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, ret = %x\n", ret);
        goto ERROR8;
    }

    MT_UNF_WINDOW_ATTR_S pWinAttr;
    MT_UNF_VO_GetWindowAttr(hWin, &pWinAttr);

    pWinAttr.stInputRect.s32X = 0;
    pWinAttr.stInputRect.s32Y = 0;
    pWinAttr.stInputRect.s32Width = 0;
    pWinAttr.stInputRect.s32Height = 0;
    pWinAttr.stOutputRect.s32X = 40;
    pWinAttr.stOutputRect.s32Y = 20;
    pWinAttr.stOutputRect.s32Width = 480;
    pWinAttr.stOutputRect.s32Height = 360;
    pWinAttr.bUseSubLayer = 1;
    pWinAttr.bSetVideoBot = 1;
    MT_UNF_VO_SetWindowAttr(hWin, &pWinAttr);

    *phAvplay = hAvplay;
    *phWin = hWin;

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;

ERROR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);
ERROR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);
ERROR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
ERROR1:

    return MT_FAILURE;
}


static MT_S32 MT_PipPlayAvplayESInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = 0;
    MT_HANDLE                hWin = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    if(NULL == phAvplay || NULL == phWin )
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("The input address is empty!\n");
        return ret;
    }


    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, ret = %x\n", ret);
        goto ERROR1;
    }

    AvplayAttr.stStreamAttr.u32VidBufSize = 0x800000;
    AvplayAttr.stStreamAttr.vdec_pip_chan = 1;
    /** Create AV player based on attributes */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Create a window */
    ret = MTADP_VO_CreatPipWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_VO_AttachWindow failed, ret = %x\n", ret);
        goto ERROR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, ret = %x\n", ret);
        goto ERROR8;
    }

    MT_UNF_WINDOW_ATTR_S pWinAttr;
    MT_UNF_VO_GetWindowAttr(hWin, &pWinAttr);

    pWinAttr.stInputRect.s32X = 0;
    pWinAttr.stInputRect.s32Y = 0;
    pWinAttr.stInputRect.s32Width = 0;
    pWinAttr.stInputRect.s32Height = 0;
    pWinAttr.stOutputRect.s32X = 40;
    pWinAttr.stOutputRect.s32Y = 20;
    pWinAttr.stOutputRect.s32Width = 480;
    pWinAttr.stOutputRect.s32Height = 360;
    pWinAttr.bUseSubLayer = 1;
    pWinAttr.bSetVideoBot = 1;
    MT_UNF_VO_SetWindowAttr(hWin, &pWinAttr);

    *phAvplay = hAvplay;
    *phWin = hWin;

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;

ERROR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);
ERROR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);
ERROR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
ERROR1:

    return MT_FAILURE;
}


static MT_VOID MT_PipPlayAvplayDeinit(MT_HANDLE hAvplay, MT_HANDLE hWin)
{
    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    /** Enable/disable windows */
    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);

    /** Unbind the window and AV player */
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

    /** Destroy window */
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);


    /** Turn off the video channel */
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);


    /** Destroy the AV player */
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

    /** Deinitializes the AV player module */
//    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();
}


/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_PipPlaySetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           VidPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_VCODEC_UNBLANK_E          unblank;

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("The input address is empty\n");
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

    SAMPLE_PIP_PLAY_INFO_PRINT("VidPid = %#x \n", VidPid);


    if(VidPid != INVALID_TSPID)
    {
        MTADP_Get_VcodeUnblank(&unblank);
        /** Get the video properties of the AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %x\n", ret);
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
        VdecAttr.enUnBlank = unblank;
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
            SAMPLE_PIP_PLAY_ERR_PRINT("Set video properties or video PID property failed, ret = %x\n", ret);
            return ret;
        }
    }

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_PipPlayStart(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();
    /** Set the PID of the AV player and set the encoder type */
    ret = MT_PipPlaySetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_PipPlaySetAvplayPidAndCodecType fail! \n");
        return ret;
    }

    if(pProgInfo->VElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_PIP_PLAY_INFO_PRINT("has no vide0 info \n");
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Start failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static MT_VOID MT_PipPlayStop(MT_HANDLE hAvplay)
{
    mt_s32            ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    /*File play*/


    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    SAMPLE_PIP_PLAY_INFO_PRINT("stop live play ...\n");
    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &option);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Stop failed, ret = %x\n", ret);
        return ret;
    }

/*
    MT_UNF_DISP_SetSdScalerEnable(MT_UNF_DISP_SCALER_MODE_FORCE_AUTO);
    MT_UNF_WINDOW_ATTR_S pMainWinAttr;
    MT_UNF_VO_GetWindowAttr(g_nimPlayInfo.hWin, &pMainWinAttr);
    pMainWinAttr.bSetVideoBot = 0;
    MT_UNF_VO_SetWindowAttr(g_nimPlayInfo.hWin, &pMainWinAttr);
*/
    SAMPLE_PIP_PLAY_FUNCTION_EXIT();

    return ret;
}


static MT_S32 MT_PipPlayStartES(MT_HANDLE hAvplay, MT_UNF_VCODEC_TYPE_E VdecType)
{
    mt_s32            ret = MT_SUCCESS;

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();


    /*set compress attr*/
    MT_UNF_VCODEC_ATTR_S VcodecAttr;
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    VcodecAttr.enType = VdecType;
    VcodecAttr.u32UseDescInfoFlag = 0;
    ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
     if (MT_SUCCESS != ret)
     {
         SAMPLE_PIP_PLAY_ERR_PRINT("MT_NimPlayStarToPlay failed, ret = %x\n", ret);
     }

     SAMPLE_PIP_PLAY_FUNCTION_EXIT();


     return ret;

}



/*!
@brief Help information
@param[in]  name     Enter the value
@return::MT_VOID
@*/
static MT_VOID MT_PipPlayPrinthelp(MT_CHAR *name)
{
    SAMPLE_PIP_PLAY_PRINT("Lack of parameters\n");
    SAMPLE_PIP_PLAY_PRINT("\nUsage:\n");
    SAMPLE_PIP_PLAY_PRINT("%s\n", name);
    SAMPLE_PIP_PLAY_PRINT("    -i: input tuner id\n");
    SAMPLE_PIP_PLAY_PRINT("    -c: input dvbc info(freq symbol_rate qam)\n");
    SAMPLE_PIP_PLAY_PRINT("    -j: input j83b info(freq symbol_rate qam)\n");
    SAMPLE_PIP_PLAY_PRINT("    -s: input dvbs info(freq symbol_rate 22k polar sig_type)\n");
    SAMPLE_PIP_PLAY_PRINT("    -t: input dvbt info(freq band_width)\n");
    SAMPLE_PIP_PLAY_PRINT("    -f: input esfile\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_PIP_PLAY_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_PIP_PLAY_PRINT("example:\n");
    SAMPLE_PIP_PLAY_PRINT("    %s -c 314 6875 64\n",name);
    SAMPLE_PIP_PLAY_PRINT("    %s -j 474 5361 256\n",name);
    SAMPLE_PIP_PLAY_PRINT("    %s -i 0 -s 3840 27500 1 0 2\n",name);
    SAMPLE_PIP_PLAY_PRINT("    %s -t 585 8\n",name);
    SAMPLE_PIP_PLAY_PRINT("    %s -f ./pipes.mpg\n",name);
}


static MT_VOID MT_PipPlayPrintMenu(MT_U32 prog_num)
{

    SAMPLE_PIP_PLAY_PRINT("\n 1 - %d : select the program \n", prog_num);

    SAMPLE_PIP_PLAY_PRINT("     h : help \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_PIP_PLAY_PRINT("     b : background run \n");
#endif
    SAMPLE_PIP_PLAY_PRINT("     l : Set pip play window location.\n");
    SAMPLE_PIP_PLAY_PRINT("     q : quit \n");
    SAMPLE_PIP_PLAY_PRINT("PIPLAY>> ");

}

#ifndef MT_SAMPLE_APP

/*
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_S32 MT_PipPlayStartMainPlay(MT_HANDLE hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
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
        SAMPLE_PIP_PLAY_ERR_PRINT("p_ProgInfo is NULL!\n");
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

    SAMPLE_PIP_PLAY_INFO_PRINT("vidpid = %x AudPid=%x \n", VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_PIP_PLAY_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_PIP_PLAY_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_PIP_PLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
            return ret;
        }
    }

    return MT_SUCCESS;
}


/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
static void MT_PipPlayStopMainPlay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    (void)MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}
#endif

/*
@brief Achieve volume addition and subtraction and mute
@return ::void
*/
static void MT_PipPlayCmdTask(MT_HANDLE hAvPlay, MT_HANDLE hWin, PMT_COMPACT_TBL *pProgTbl)
{

    MT_CHAR    inputCmd[32] = { 0 };
    MT_S32     ret = SUCCESS;
    MT_U32     u32ProgNum = 0;
    PMT_COMPACT_PROG *pstCurrentProgInfo = NULL;
    MT_U32     x = SAMPLE_PIP_PLAY_DEFAULTT_X;
    MT_U32     y = SAMPLE_PIP_PLAY_DEFAULTT_Y;
    MT_U32     w = SAMPLE_PIP_PLAY_DEFAULTT_W;
    MT_U32     h = SAMPLE_PIP_PLAY_DEFAULTT_H;

    while (1)
    {

        (MT_VOID)MT_PipPlayPrintMenu(pProgTbl->prog_num);
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_PIP_PLAY_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_PIP_PLAY_INFO_PRINT("pipplay in back!\n");
            break;
        }
#endif
        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                if(pstCurrentProgInfo->VElementNum == 0)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT(" SwitchProg failed, prog is not vedio.\n");
                    continue;
                }
                if(pstCurrentProgInfo->VElementPid == g_stPipPlayMainRunInfo.pProgTbl->currentprog->VElementPid)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT(" SwitchProg failed, Not play same with main prog.\n");
                    continue;
                }

                (void)MT_PipPlayStop(hAvPlay);

                SAMPLE_PIP_PLAY_INFO_PRINT("Start pip play ProgNum: %d \n", u32ProgNum);

                ret  = MT_PipPlayStart(hAvPlay, pstCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT(" SwitchProg failed.\n");
                }
            }
            else
            {
                SAMPLE_PIP_PLAY_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }
        }
        else if ('l' == inputCmd[0])
        {
            SAMPLE_PIP_PLAY_INFO_PRINT("Set pip play window location...\n");
            SAMPLE_PIP_PLAY_PRINT("Please input location Coordinate(x,y,w,h). \n");
            scanf("%d %d %d %d", &x, &y, &w, &h);

            ret = MT_PipPlaySetWindowLocation(hWin, x, y, w, h);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT(" MT_PipPlaySetWindowLocation failed.\n");
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_PIP_PLAY_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}


static MT_VOID MT_PipPlayExit(MT_VOID)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    SAMPLE_PIP_PLAY_FUNCTION_ENTER();

    (MT_VOID)MT_PipPlayStop(g_pipPlayInfo.hAvPlay);

    (MT_VOID)MT_PipPlayAvplayDeinit(g_pipPlayInfo.hAvPlay, g_pipPlayInfo.hWin);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_pipPlayInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();


    if(MT_INPUT_SIG_TYPE_FILE == g_pipPlayInfo.sInputParam.sig_type)
    {
        g_bPipTaskQuit = MT_TRUE;
        pthread_join(g_pipPlayInfo.htsPipThd, NULL);
    }
    else
    {
        (MT_VOID)MTADP_Fe_DeInit(g_pipPlayInfo.sInputParam.tuner_id);
    }


    (MT_VOID)MT_PipPlayDmxDeInit();

    g_bTaskQuit = MT_TRUE;

    SAMPLE_PIP_PLAY_FUNCTION_EXIT();
}


static MT_VOID MT_PipPlayMainExit(void)
{



#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_PipPlayStopplay(g_stPipPlayMainRunInfo.hAvPlay);

    (MT_VOID)MT_PipPlayAVplayDeInit(g_stPipPlayMainRunInfo.hAvPlay, g_stPipPlayMainRunInfo.hWin, g_stPipPlayMainRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_stPipPlayMainRunInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == g_stPipPlayMainRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(g_stPipPlayMainRunInfo.htsThd, NULL);
    }

    (MT_VOID)MT_PipPlayDmxDeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != g_stPipPlayMainRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }
#endif
    memset(&g_stPipPlayMainRunInfo, 0xff, sizeof(g_stPipPlayMainRunInfo));
    g_bTaskQuit = MT_TRUE;
}

/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_PipPlayParaseargs(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;
    mt_u32 idx = 0;

//#ifndef MT_SAMPLE_APP
    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_PipPlayPrinthelp(argv[0]);
        return MT_FAILURE;
    }
//#endif
    while((opt = MTADP_Getopt(argc, argv, "h?Hf:t:c:s:j:i:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_PipPlayPrinthelp(argv[0]);
            return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_PipPlayExit();
                }
                return MT_TASK_EXIT;
            case 'i':
                pInputParam->tuner_id = strtol(mt_optarg, 0, 0);
                break;
            case 'f':
                if(argc < 3)
                {
                    (void)MT_PipPlayPrinthelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                return MT_SUCCESS;

            case 's':
                if(argc < 6)
                {
                    (void)MT_PipPlayPrinthelp(argv[0]);
                    return MT_FAILURE;
                }

                for (idx = 0; idx < argc; idx++) {
                    if (!strcmp("-s", argv[idx])){
                        break;
                    }
                }

                SAMPLE_PIP_PLAY_INFO_PRINT("DVBS parameter idx: %d \n", idx);

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_SAT;

                pInputParam->input_param.sat.freq = strtol(argv[++idx], 0, 0);
                pInputParam->input_param.sat.sym_rate = strtol(argv[++idx], 0, 0);
                pInputParam->input_param.sat.onoff_22k = strtol(argv[++idx], 0, 0);
                pInputParam->input_param.sat.polarization = strtol(argv[++idx], 0, 0);
                pInputParam->input_param.sat.port_type = strtol(argv[++idx], 0, 0);

                return MT_SUCCESS;

            case 'c':
                if(argc < 4)
                {
                    (void)MT_PipPlayPrinthelp(argv[0]);
                    return MT_FAILURE;
                }

                for (idx = 0; idx < argc; idx++) {
                    if (!strcmp("-c", argv[idx])){
                        break;
                    }
                }

                SAMPLE_PIP_PLAY_INFO_PRINT("DVBC parameter idx: %d \n", idx);

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[++idx], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[++idx], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[++idx], 0, 0);
                return MT_SUCCESS;
             case 't':
                if(argc < 3)
                {
                    (void)MT_PipPlayPrinthelp(argv[0]);
                    return MT_FAILURE;
                }

                for (idx = 0; idx < argc; idx++) {
                    if (!strcmp("-t", argv[idx])){
                        break;
                    }
                }

                SAMPLE_PIP_PLAY_INFO_PRINT("DVBT parameter idx: %d \n", idx);

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_DVB_T;

                pInputParam->input_param.ter.freq = strtol(argv[++idx], 0, 0);
                pInputParam->input_param.ter.sym_rate= strtol(argv[++idx], 0, 0);
                return MT_SUCCESS;
            case 'j':
                if(argc < 4)
                {
                    (void)MT_PipPlayPrinthelp(argv[0]);
                    return MT_FAILURE;
                }

                for (idx = 0; idx < argc; idx++) {
                    if (!strcmp("-j", argv[idx])){
                        break;
                    }
                }
                SAMPLE_PIP_PLAY_INFO_PRINT("DVBT parameter idx: %d \n", idx);

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_J83B;
                pInputParam->input_param.cab.freq = strtol(argv[++idx], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[++idx], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[++idx], 0, 0);
                return MT_SUCCESS;
            default:
                (void)MT_PipPlayPrinthelp(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_PipPlayMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;
#ifndef MT_SAMPLE_APP
    MT_UNF_SND_GAIN_ATTR_S stGainVolume = {0};
    PMT_COMPACT_PROG *p_stCurrentProgInfo = { 0 };
#endif
    PMT_COMPACT_PROG *pstPipProgInfo = NULL;
    mt_s32 i = 0;


    ret = MT_PipPlayParaseargs(argc, argv, &g_pipPlayInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_PIP_PLAY_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }


        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_PipPlayDmxInit(&g_stPipPlayMainRunInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT( "failed to StartDmx\n");
            goto ERR6;
        }


        if(MT_INPUT_SIG_TYPE_FILE == g_stPipPlayMainRunInfo.sInputParam.sig_type)
        {
            ret = pthread_create(&g_stPipPlayMainRunInfo.htsThd, NULL, (void * (*)(void *))MT_PipPlayInjectTsTask, &g_stPipPlayMainRunInfo.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("failed to pthread_create\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }
        else
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            if (MT_INPUT_SIG_TYPE_CAB == g_stPipPlayMainRunInfo.sInputParam.sig_type)
            {     //dvbc
                ret = MT_PipPlayCheckDvbcParam(&g_stPipPlayMainRunInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.cab.freq,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.cab.sym_rate,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_stPipPlayMainRunInfo.sInputParam.sig_type)
            {
                ret = MT_PipPlayCheckDvbsParam(&g_stPipPlayMainRunInfo.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.sat.freq,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.sat.sym_rate,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.sat.onoff_22k,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.sat.polarization,
                                        g_stPipPlayMainRunInfo.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }
        }

        (void)MTADP_Search_Init();

        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stPipPlayMainRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR9;
        }

        (MT_VOID)DVB_ListProg();

        ret = MT_PipPlayAVplayInit(&g_stPipPlayMainRunInfo.hAvPlay, &g_stPipPlayMainRunInfo.hWin, &g_stPipPlayMainRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MT_PipPlayAVplayInit\n");
            goto ERR10;
        }

        /* Play the first program on the program list*/
        p_stCurrentProgInfo = g_stPipPlayMainRunInfo.pProgTbl->proginfo;

        ret = MT_PipPlayStartMainPlay(g_stPipPlayMainRunInfo.hAvPlay, p_stCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT("failed to MT_PipPlayStartMainPlay\n");
            goto ERR11;
        }
#endif
#ifdef MT_SAMPLE_APP
        g_stPipPlayMainRunInfo.hAvPlay = avplayHandle.hAvPlay;
        g_stPipPlayMainRunInfo.hWin = avplayHandle.hWin;
        g_stPipPlayMainRunInfo.pProgTbl = (PMT_COMPACT_TBL*)malloc(sizeof(PMT_COMPACT_TBL));
        g_stPipPlayMainRunInfo.pProgTbl->prog_num = 1;

        MTADP_Get_Current_Info(&g_stPipPlayMainRunInfo.pProgTbl->currentprog);

        SAMPLE_PIP_PLAY_INFO_PRINT("pstPipProgInfo: vedio: %#x audio: %#x \n", g_stPipPlayMainRunInfo.pProgTbl->currentprog->VElementPid, g_stPipPlayMainRunInfo.pProgTbl->currentprog->AElementPid);

#endif
        SAMPLE_PIP_PLAY_INFO_PRINT( "MainRunInfo Avplay[%#x] win[%#x] \n", g_stPipPlayMainRunInfo.hAvPlay, g_stPipPlayMainRunInfo.hWin);

        (MT_VOID)MT_PipPlayReSetMainWindow(g_stPipPlayMainRunInfo.hWin, MT_UNF_DISP_SCALER_MODE_FORCE_SD_DISABLE, MT_TRUE);

        ret = MT_PipPlayDmxInit(DMX_ID_1, &g_pipPlayInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PIP_PLAY_ERR_PRINT( "failed to StartDmx\n");
            goto ERR0;
        }
        if(MT_INPUT_SIG_TYPE_FILE != g_pipPlayInfo.sInputParam.sig_type)
        {

            ret = MT_PipPlayAvplayInit(&g_pipPlayInfo.hAvPlay, &g_pipPlayInfo.hWin);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MT_PipPlayAvplayInit failed, ret = %x\n", ret);
                goto ERR12;
            }

            /** Tuner initialization, Set the default parameters for tuner */
            ret = MTADP_Fe_Init(g_pipPlayInfo.sInputParam.tuner_id);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MTADP_Fe_Init failed, ret = %x\n", ret);
                goto ERR13;
            }

            if (MT_INPUT_SIG_TYPE_CAB == g_pipPlayInfo.sInputParam.sig_type)
            {     //dvbc
                ret = MT_PipPlayCheckDvbcParam(&g_pipPlayInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR14;
                }
                ret = MTADP_Fe_Connect_Dvbc(g_pipPlayInfo.sInputParam.tuner_id,
                                            g_pipPlayInfo.sInputParam.input_param.cab.freq,
                                            g_pipPlayInfo.sInputParam.input_param.cab.sym_rate,
                                            g_pipPlayInfo.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_pipPlayInfo.sInputParam.sig_type)
            {
                ret = MT_PipPlayCheckDvbsParam(&g_pipPlayInfo.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR14;
                }
                ret = MTADP_Fe_Connect_Dvbs(g_pipPlayInfo.sInputParam.tuner_id,
                                            g_pipPlayInfo.sInputParam.input_param.sat.freq,
                                            g_pipPlayInfo.sInputParam.input_param.sat.sym_rate,
                                            g_pipPlayInfo.sInputParam.input_param.sat.onoff_22k,
                                            g_pipPlayInfo.sInputParam.input_param.sat.polarization,
                                            g_pipPlayInfo.sInputParam.input_param.sat.port_type);
            }
            else if(MT_INPUT_SIG_TYPE_DVB_T == g_pipPlayInfo.sInputParam.sig_type)
            {

                ret = MT_PipPlayDvbtCheckParam(&g_pipPlayInfo.sInputParam.input_param.ter);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR14;
                }
                ret = MTADP_Fe_Connect_Dvbt(g_pipPlayInfo.sInputParam.tuner_id,
                                            g_pipPlayInfo.sInputParam.input_param.ter.freq,
                                            g_pipPlayInfo.sInputParam.input_param.ter.sym_rate);
            }
            else if(MT_INPUT_SIG_TYPE_J83B == g_pipPlayInfo.sInputParam.sig_type)
            {

                ret = MT_PipPlayJ83bCheckParam(&g_pipPlayInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PIP_PLAY_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR14;
                }
                ret = MTADP_Fe_Connect_J83b(g_pipPlayInfo.sInputParam.tuner_id,
                                            g_pipPlayInfo.sInputParam.input_param.cab.freq,
                                            g_pipPlayInfo.sInputParam.input_param.cab.sym_rate,
                                            g_pipPlayInfo.sInputParam.input_param.cab.mod_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MTADP_Fe_Connect failed, ret = %d\n", ret);
                goto ERR14;
            }

            (MT_VOID)MTADP_Search_Init();
            ret = MTADP_Search_GetAllPmt(DMX_ID_1, &g_pipPlayInfo.pProgTbl);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
                goto ERR14;
            }

            while(MT_TRUE)
            {
                pstPipProgInfo = g_pipPlayInfo.pProgTbl->proginfo + i;

                if((pstPipProgInfo->VElementNum > 0) && (pstPipProgInfo->VElementPid != g_stPipPlayMainRunInfo.pProgTbl->currentprog->VElementPid))
                {
                    break;
                }

                i++;
            }

            ret = MT_PipPlayStart(g_pipPlayInfo.hAvPlay, pstPipProgInfo);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MT_PipPlayStarToPlay failed, ret = %x\n", ret);
                goto ERR15;
            }

        }
        else if(MT_INPUT_SIG_TYPE_FILE == g_pipPlayInfo.sInputParam.sig_type)
        {
            ret = MT_PipPlayAvplayESInit(&g_pipPlayInfo.hAvPlay, &g_pipPlayInfo.hWin);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MT_PipPlayAvplayESInit failed, ret = %x\n", ret);
                goto ERR12;
            }
            ret = MT_PipPlayStartES(g_pipPlayInfo.hAvPlay, MT_UNF_VCODEC_TYPE_MPEG2);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("MT_PipPlayStartES failed, ret = %x\n", ret);
                goto ERR13;
            }

            g_bPipTaskQuit = MT_FALSE;
            ret = pthread_create(&g_pipPlayInfo.htsPipThd, NULL, (MT_VOID * (*)(MT_VOID *))MT_PipPlayEsTthread, &g_pipPlayInfo.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_PIP_PLAY_ERR_PRINT("failed to pthread_create\n");
                goto ERR14;
            }
            sleep(1);

             g_pipPlayInfo.pProgTbl = (PMT_COMPACT_TBL*)malloc(sizeof(PMT_COMPACT_TBL));
             g_pipPlayInfo.pProgTbl->prog_num = 1;
             g_pipPlayInfo.pProgTbl->proginfo = (PMT_COMPACT_PROG*)malloc(sizeof(PMT_COMPACT_PROG));
        }

        g_bTaskQuit = MT_FALSE;
    }


    (void) MT_PipPlayCmdTask(g_pipPlayInfo.hAvPlay, g_pipPlayInfo.hWin, g_pipPlayInfo.pProgTbl);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    (MT_VOID)MT_PipPlayStop(g_pipPlayInfo.hAvPlay);

ERR15:

    (MT_VOID)MTADP_Search_FreeAllPmt(g_pipPlayInfo.pProgTbl);
    (MT_VOID)MTADP_Search_DeInit();


ERR14:
    if(MT_INPUT_SIG_TYPE_FILE == g_pipPlayInfo.sInputParam.sig_type)
    {
        g_bPipTaskQuit = MT_TRUE;
        pthread_join(g_pipPlayInfo.htsPipThd, NULL);
    }
    else
    {
        (MT_VOID)MTADP_Fe_DeInit(g_pipPlayInfo.sInputParam.tuner_id);
    }

ERR13:

    (MT_VOID)MT_PipPlayAvplayDeinit(g_pipPlayInfo.hAvPlay, g_pipPlayInfo.hWin);


ERR12:
    (MT_VOID)MT_PipPlayDmxSearchDeInit(DMX_ID_1);

    (MT_VOID)MT_PipPlayReSetMainWindow(g_stPipPlayMainRunInfo.hWin, MT_UNF_DISP_SCALER_MODE_FORCE_AUTO, MT_TRUE);

ERR0:
#ifdef MT_SAMPLE_APP
    free(g_stPipPlayMainRunInfo.pProgTbl);
#endif

#ifndef MT_SAMPLE_APP
ERR11:
    (MT_VOID)MT_PipPlayStopMainPlay(g_stPipPlayMainRunInfo.hAvPlay);
    SAMPLE_PIP_PLAY_INFO_PRINT("stop to play\n");

ERR10:
    (MT_VOID)MT_PipPlayMainPlayDeInit(g_stPipPlayMainRunInfo.hAvPlay, g_stPipPlayMainRunInfo.hWin, g_stPipPlayMainRunInfo.hSoundTrack);
ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stPipPlayMainRunInfo.pProgTbl);
ERR8:
    (MT_VOID)MTADP_Search_DeInit();
ERR7:
    if(MT_INPUT_SIG_TYPE_FILE == g_stPipPlayMainRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(g_stPipPlayMainRunInfo.htsThd, NULL);
    }
    else
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR6:
    (MT_VOID)MT_PipPlayDmxDeInit();
ERR5:
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    (MT_VOID)MTADP_Snd_DeInit();
ERR3:
    (MT_VOID)MTADP_Disp_DeInit();
ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    (MT_VOID)mt_sys_deinit();
#endif

    memset(&g_stPipPlayMainRunInfo, 0xff, sizeof(g_stPipPlayMainRunInfo));
    g_bTaskQuit = MT_TRUE;
    return MT_SUCCESS;
}

