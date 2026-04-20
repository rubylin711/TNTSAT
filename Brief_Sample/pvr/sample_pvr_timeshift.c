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
#include <sys/stat.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_frontend.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
#include "mt_adp_pvr.h"
#include "mt_cmdline.h"






/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_TIMESHIFT_DEBUG

#define MT_TIMESHIFT_PRINT   printf
#else

#define MT_TIMESHIFT_PRINT

#endif

#define SAMPLE_TIMESHIFT_FUNCTION_ENTER()     MT_TIMESHIFT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TIMESHIFT_FUNCTION_EXIT()      MT_TIMESHIFT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_TIMESHIFT_FATAL_PRINT(fmt...)          MT_TIMESHIFT_PRINT(" [FATAL] " fmt)
#define SAMPLE_TIMESHIFT_ERR_PRINT(fmt...)            MT_TIMESHIFT_PRINT(" [ERROR] " fmt)
#define SAMPLE_TIMESHIFT_WARN_PRINT(fmt...)           MT_TIMESHIFT_PRINT(" [WARN] "  fmt)
#define SAMPLE_TIMESHIFT_INFO_PRINT(fmt...)           MT_TIMESHIFT_PRINT(" [INFO] "  fmt)
#define SAMPLE_TIMESHIFT_DBG_PRINT(fmt...)            MT_TIMESHIFT_PRINT(" [DEBUG] " fmt)

#define SAMPLE_TIMESHIFT_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define INVALID_TSPID (0x1fff)
#define MAX_TIMESHIFT_REC_FILE_SIZE     (4 *1024 *1024*1024LLU) /*4G*/





/*************************** Structure Definition ****************************/
typedef enum input_sig_type_t {
    MT_INPUT_SIG_TYPE_CAB = 1,
    /**<Cable signal*/
    MT_INPUT_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/
    MT_INPUT_SIG_TYPE_DVB_T = 3,
    /**<Terrestrial signal*/
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
    MT_BOOL bLIve;
    MT_BOOL bPAuse;
    PMT_COMPACT_TBL  *pProgTbl;
    MT_U32  recChn1 ;
    MT_U32  recChn2 ;
    MT_U32  recChn3 ;
    MT_U32  recChn4 ;
    MT_U32  playChn ;
    MT_BOOL bIsPlayStop;
    MT_U32  firstRecTime;
} MT_Timeshift_RUN_INFO;

typedef enum mtPvrPlaySpeed
{
    MT_PVRPLAY_SPEED_NORMAL = 0,
    MT_PVRPLAY_SPEED_FAST_FORWORD_MODE = 1,
    MT_PVRPLAY_SPEED_FAST_FORWORD_2X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_4X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_8X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_16X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_32X,
    //MT_PVRPLAY_SPEED_FAST_FORWORD_64X,
    MT_PVRPLAY_SPEED_FAST_FORWORD_MAX,

    MT_PVRPLAY_SPEED_FAST_BACKWORD_MODE = 10,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_2X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_4X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_8X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_16X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_32X,
    //MT_PVRPLAY_SPEED_FAST_BACKWORD_64X,
    MT_PVRPLAY_SPEED_FAST_BACKWORD_MAX,

    MT_PVRPLAY_SPEED_SLOW_FORWORD_MODE = 20,
    MT_PVRPLAY_SPEED_SLOW_FORWORD_2X,
    MT_PVRPLAY_SPEED_SLOW_FORWORD_4X,
    //MT_PVRPLAY_SPEED_SLOW_FORWORD_8X, //SF only support 2X 4X
    //MT_PVRPLAY_SPEED_SLOW_FORWORD_16X,
    //MT_PVRPLAY_SPEED_SLOW_FORWORD_32X,
    //MT_PVRPLAY_SPEED_SLOW_FORWORD_64X,
    //MT_PVRPLAY_SPEED_SLOW_FORWORD_43,
    MT_PVRPLAY_SPEED_SLOW_FORWORD_MAX,

    MT_PVRPLAY_SPEED_SLOW_BACKWORD_MODE = 30,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_2X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_4X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_8X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_16X,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_32X,
    //MT_PVRPLAY_SPEED_SLOW_BACKWORD_64X,
    //MT_PVRPLAY_SPEED_SLOW_BACKWORD_43,
    MT_PVRPLAY_SPEED_SLOW_BACKWORD_MAX,
    MT_PVRPLAY_SPEED_MAX
}MT_PVRPLAY_SPEED_E;

/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_Timeshift_RUN_INFO    g_stTimeshiftRunInfo = {0};
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif


/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_PVRTimeshiftMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

#ifndef MT_SAMPLE_APP
static mt_s32 MT_PVRTimeshiftCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_TIMESHIFT_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_PVRTimeshiftCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_TIMESHIFT_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}



/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_PVRTimeshiftDmxInit(mt_input_para_t *pInputParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_TIMESHIFT_FUNCTION_ENTER();
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    s32Ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("failed to mt_sys_get_version\n");
        return MT_FAILURE;
    }



    if(MT_INPUT_SIG_TYPE_CAB == pInputParam->sig_type)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
        }
        else
        {
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_0);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_0);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_0);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_0);
        }
    }
    else if(MT_INPUT_SIG_TYPE_SAT == pInputParam->sig_type)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            if(pInputParam->tuner_id == 0)
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, DMX_DVB_TSI_IN_PORT);
                s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, DMX_DVB_TSI_IN_PORT);
                s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, DMX_DVB_TSI_IN_PORT);
                s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, DMX_DVB_TSI_IN_PORT);
            }
            else
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
                s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
                s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
            }
        }
        else
        {
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
            s32Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
        }
    }
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort PVR_DMX_ID_REC/PVR_DMX_ID_REC+2\n");
        return MT_FAILURE;
    }


    SAMPLE_TIMESHIFT_FUNCTION_EXIT();


    return MT_SUCCESS;
}
#endif

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static MT_VOID MT_PVRTimeshiftDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_LIVE);
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC);
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC+2);
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC+3);
    (MT_VOID)MT_UNF_DMX_DeInit();

}


/*
@brief State thread function
@return NULL;
*/
static MT_VOID *MT_PVRTimeshiftstatuThread(MT_VOID *args)
{
    MT_S32 Ret = 0;
    MT_U32 rectime1 = 0;
    MT_U32 rectime2 = 0;
    MT_U32 rectime3 = 0;
    MT_U32 playtime = 0;
    MT_UNF_PVR_REC_ATTR_S stRecAttr1 = { 0 };
    MT_UNF_PVR_REC_ATTR_S stRecAttr2 = { 0 };
    MT_UNF_PVR_REC_ATTR_S stRecAttr3 = { 0 };
    MT_UNF_PVR_REC_STATUS_S RecstStatus1 = { 0 };
    MT_UNF_PVR_REC_STATUS_S RecstStatus2 = { 0 };
    MT_UNF_PVR_REC_STATUS_S RecstStatus3 = { 0 };
    MT_UNF_PVR_REC_STATUS_S RecstStatus4 = { 0 };
    MT_UNF_PVR_PLAY_STATUS_S PlaystStatus = { 0 };
    MT_UNF_PVR_FILE_ATTR_S FileStatus = { 0 };


    while(MT_FALSE == g_stTimeshiftRunInfo.bIsPlayStop)
    {
        memset(&RecstStatus1, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        memset(&RecstStatus2, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        memset(&RecstStatus3, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        memset(&RecstStatus4, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        memset(&PlaystStatus, 0, sizeof(MT_UNF_PVR_PLAY_STATUS_S));
        playtime = 0;
        rectime1 = 0;
        rectime2 = 0;
        rectime3 = 0;
        sleep(1);

        if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1)
        {
            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn1, &stRecAttr1);
            Ret = MT_UNF_PVR_RecGetStatus(g_stTimeshiftRunInfo.recChn1, &RecstStatus1);
            if(MT_SUCCESS != Ret)
            {
                //SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_RecGetStatus failed.\n");
                continue;
            }
            rectime1 = RecstStatus1.u32EndTimeInMs;

            Ret = MT_UNF_PVR_GetFileAttrByFileName(stRecAttr1.szFileName, &FileStatus);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_GetFileAttrByFileName failed.\n");
                continue;
            }
        }
        if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2)
        {
            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn2, &stRecAttr2);
            Ret = MT_UNF_PVR_RecGetStatus(g_stTimeshiftRunInfo.recChn2, &RecstStatus2);
            if(MT_SUCCESS != Ret)
            {
                //SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_RecGetStatus failed.\n");
                continue;
            }
            rectime2 = RecstStatus2.u32EndTimeInMs;

            Ret = MT_UNF_PVR_GetFileAttrByFileName(stRecAttr2.szFileName, &FileStatus);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_GetFileAttrByFileName failed.\n");
                continue;
            }
        }
        if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3)
        {
            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn3, &stRecAttr3);
            Ret = MT_UNF_PVR_RecGetStatus(g_stTimeshiftRunInfo.recChn3, &RecstStatus3);
            if(MT_SUCCESS != Ret)
            {
                //SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_RecGetStatus failed.\n");
                continue;
            }
            rectime3 = RecstStatus3.u32EndTimeInMs;

            Ret = MT_UNF_PVR_GetFileAttrByFileName(stRecAttr3.szFileName, &FileStatus);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_GetFileAttrByFileName failed.\n");
                continue;
            }
        }

        if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn4)
        {
            Ret = MT_UNF_PVR_RecGetStatus(g_stTimeshiftRunInfo.recChn4, &RecstStatus4);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }

        }

        if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.playChn)
        {
            Ret = MT_UNF_PVR_PlayGetStatus(g_stTimeshiftRunInfo.playChn, &PlaystStatus);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_PlayGetStatus failed.\n");
                continue;
            }
            playtime = PlaystStatus.u32CurPlayTimeInMs;

            Ret =  MT_UNF_PVR_PlayGetFileAttr(g_stTimeshiftRunInfo.playChn, &FileStatus);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_PlayGetFileAttr failed.\n");
                continue;
            }
            if(g_stTimeshiftRunInfo.firstRecTime == MT_INVALID_HANDLE)
            {
                g_stTimeshiftRunInfo.firstRecTime = FileStatus.u32StartTimeInMs;
                SAMPLE_TIMESHIFT_INFO_PRINT("first rec time=%d\n",g_stTimeshiftRunInfo.firstRecTime);
            }
        }


        if(playtime && ((MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn4)))
        {
            SAMPLE_TIMESHIFT_INFO_PRINT("###rec1=%d,rec2=%d,rec3=%d,rec4=%d,ply=%d(%d)\n",(rectime1-RecstStatus1.u32StartTimeInMs)/1000,
                                                (rectime2-RecstStatus2.u32StartTimeInMs)/1000,
                                                (rectime3-RecstStatus3.u32StartTimeInMs)/1000,
                                                RecstStatus4.u32CurTimeInMs/1000,
                                                (playtime-g_stTimeshiftRunInfo.firstRecTime)/1000,//not rewind time display
                                                (playtime-FileStatus.u32StartTimeInMs)/1000);
            play_resource.rec_status = MT_TRUE;
        }
        else if((MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn4))
        {
            SAMPLE_TIMESHIFT_INFO_PRINT("###rec1=%d,rec2=%d,rec3=%d,rec4all=%d\n",(rectime1-RecstStatus1.u32StartTimeInMs)/1000,
                                                (rectime2-RecstStatus2.u32StartTimeInMs)/1000,
                                                (rectime3-RecstStatus3.u32StartTimeInMs)/1000,
                                                RecstStatus4.u32CurTimeInMs/1000);
            play_resource.rec_status = MT_TRUE;
        }
        else
        {
            play_resource.rec_status = MT_FALSE;
        }



    }
    return NULL;
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
static mt_s32 MT_PVRTimeshiftAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = PVR_DMX_ID_REC;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
static MT_VOID  MT_PVRTimeshiftAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("=====hWin is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("=====hSoundTrack is INVALID_HANDLE ======\n");
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
static mt_s32 MT_PVRTimeshiftAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
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
        SAMPLE_TIMESHIFT_ERR_PRINT("p_ProgInfo is NULL!\n");
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

    SAMPLE_TIMESHIFT_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
            return ret;
        }
        /* The type of code stream supported by the decoder*/
        VcodecAttr.enType = enVidType;
        VcodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VcodecAttr.u32ErrCover = 100;
        VcodecAttr.u32Priority = 3;
        VcodecAttr.u32UseDescInfoFlag = 1;

        //VcodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_USER;

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_TIMESHIFT_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_TIMESHIFT_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


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
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_TIMESHIFT_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
            return ret;
        }
    }

    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
        return ret;
    }


    return MT_SUCCESS;
}


/*
@brief stop to play
@param[in] avplay, Player handle
@return MT_SUCCESS
*/
static mt_s32 MT_PVRTimeshiftStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if(MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    stopopt.u32TimeoutMs = 0;
    return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}


/*
@brief start playback
@param[in] pszFileName, Recorded file name
@param[in] pu32PlayChn, The recorded channel name
@param[in] hAvplay, Player handle
@return::MT_SUCCESS
@return::MT_MT_FAILURE
*/
static MT_S32 MT_PVRTimeshiftSwitchToShiftPlay(const MT_CHAR *pszFileName, MT_U32 *pu32PlayChn, MT_HANDLE hAvplay)
{
    MT_S32    ret = MT_SUCCESS;

    ret = MT_PVRTimeshiftStopplay(hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("PVR_StopPlayLive failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("MT_UNF_DMX_DetachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC,MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }

    // restore ac4    attr info.
    MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);

    ret = MTADP_PVR_StartPlayBack(pszFileName, pu32PlayChn, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("MTADP_PVR_StartPlayBack failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }

    return MT_SUCCESS;
}



/*
@brief Switch to live mode
@param[in] u32PlayChn, The recorded channel name
@param[in] hAvplay, Player handle
@param[in] pProgInfo, Broadcast program
@return::MT_SUCCESS
@return::MT_MT_FAILURE
*/
static MT_S32 MT_PVRTimeshiftSwitchToLivePlay(MT_U32 u32PlayChn, MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo, mt_input_para_t *pInputParam)
{
    MT_S32    ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo={0};
    mt_sys_get_version(&stSysChipInfo);

    (void)MTADP_PVR_StopPlayBack(u32PlayChn);

    SAMPLE_TIMESHIFT_INFO_PRINT("%s AElementNum=%d\n",__FUNCTION__,pProgInfo->AElementNum);
    if(pProgInfo->AElementNum > 1){

        //after pvr playback multi audio, need to reopen auido channel
         (MT_VOID)MT_UNF_AVPLAY_ChnClose(g_stTimeshiftRunInfo.hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

        ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("switch to live play, MT_UNF_AVPLAY_ChnOpen failed ret=0x%x\n",ret);
            return ret;
        }
        SAMPLE_TIMESHIFT_PRINT("%s reopen audio chn ok\n",__FUNCTION__);
    }

    ret = MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("MT_UNF_DMX_DetachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }
#ifndef MT_SAMPLE_APP
    if(MT_INPUT_SIG_TYPE_CAB == pInputParam->sig_type)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
        }
        else
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_0);
        }
    }
    else if(MT_INPUT_SIG_TYPE_SAT == pInputParam->sig_type)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, DMX_DVB_TSI_IN_PORT);
        }
        else
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
        }
    }
#else
    if(play_resource.sig_type == 0)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
        }
        else
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_0);
        }
    }
    else if(play_resource.sig_type == 1)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, DMX_DVB_TSI_IN_PORT);
        }
        else
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
        }
    }
    else if(play_resource.sig_type == 2)
    {
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_2);
        }
        else
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_0);
        }
    }
    else if(play_resource.sig_type == 3)
    {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
        ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_3);
        ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_3);
        ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_3);
        ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_3);
#else
        ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
        ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC, MT_UNF_DMX_PORT_TSI_1);
        ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
        ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
#endif

    }
#endif
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }

    // restore ac4    attr info.
    MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
    ret = MT_PVRTimeshiftAVPlay_Start(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("MT_PVRTimeshiftAVPlay_Start failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return MT_SUCCESS;
}

#ifndef MT_SAMPLE_APP
/*
@brief help
@return void
*/
static void MT_PVRTimeshiftPrintDvbcHelp(char *name)
{
    SAMPLE_TIMESHIFT_PRINT("\nDvbc Usage:\n");
    SAMPLE_TIMESHIFT_PRINT("%s -f folder -c freq symrate qam \n", name);
    SAMPLE_TIMESHIFT_PRINT("    freq : \n");
    SAMPLE_TIMESHIFT_PRINT("    symrate: \n");
    SAMPLE_TIMESHIFT_PRINT("    qam: \n");
    SAMPLE_TIMESHIFT_PRINT("example:\n");
    SAMPLE_TIMESHIFT_PRINT("    %s -f ./pvr -c 654 6875 64 \n", name);
}

static void MT_PVRTimeshiftPrintDvbsHelp(char *name)
{
    SAMPLE_TIMESHIFT_PRINT("\nDvbs Usage:\n");
    SAMPLE_TIMESHIFT_PRINT("%s -f folder -s freq symrate onoff_22k polarization port_type \n", name);
    SAMPLE_TIMESHIFT_PRINT("    freq : \n");
    SAMPLE_TIMESHIFT_PRINT("    symrate: \n");
    SAMPLE_TIMESHIFT_PRINT("    onoff_22k: \n");
    SAMPLE_TIMESHIFT_PRINT("example:\n");
    SAMPLE_TIMESHIFT_PRINT("    %s -f ./pvr -s 3840 27500 1 0 0 \n", name);
}
#endif

static void MT_PVRTimeshiftPrint_help(char *name)
{
    SAMPLE_TIMESHIFT_PRINT("Lack of parameters\n");
    SAMPLE_TIMESHIFT_PRINT("\nUsage:\n");
    SAMPLE_TIMESHIFT_PRINT(" %s\n", name);
    SAMPLE_TIMESHIFT_PRINT("    -f: Record file save folder\n");
#ifndef MT_SAMPLE_APP
    SAMPLE_TIMESHIFT_PRINT("    -t: tuner id \n");
    SAMPLE_TIMESHIFT_PRINT("    -c: DVBC locks frequency\n");
    SAMPLE_TIMESHIFT_PRINT("        -c freq symrate qam \n");
    SAMPLE_TIMESHIFT_PRINT("    -s: DVBS locks frequency\n");
    SAMPLE_TIMESHIFT_PRINT("        -s freq symrate onoff_22k polarization port_type path\n");
    SAMPLE_TIMESHIFT_PRINT("example:\n");
    SAMPLE_TIMESHIFT_PRINT("    %s -f ./pvr -t 1 -c 314 6875 64 \n", name);
    SAMPLE_TIMESHIFT_PRINT("    %s -f ./pvr -t 0 -s 3840 27500 1 0 0 \n", name);
#else
    SAMPLE_TIMESHIFT_PRINT("    %s -f ./pvr  \n", name);
    SAMPLE_TIMESHIFT_PRINT("    %s -q  <exit> \n", name);

#endif

}

static mt_s32 MT_PVRTimeshiftRestoreAc4PlayAttrInfo(mt_handle hAvplay)
{
    mt_s32    ret = MT_SUCCESS;
    play_ac4_attr_info play_ac4_info;


    MTADP_AUD_GetAc4PlayAttrInfo(&play_ac4_info);
    if (!play_ac4_info.ac4_attr_enable)
    {
        SAMPLE_TIMESHIFT_INFO_PRINT("no restore ac4 play attr info \n");
        return ret;
    }

    // restore ac4    attr info.
    ret = MT_UNF_AVPLAY_StopAudDec(hAvplay);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_AVPLAY_StopAudDec failed.\n");
        return ret;
    }

    ret = MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MTADP_AUD_RestoreAc4PlayAttrInfo failed.\n");
    }

    ret = MT_UNF_AVPLAY_StartAudDec(hAvplay);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_AVPLAY_StartAudDec failed.\n");
        return ret;
    }

    return ret;
}

/*!
@brief Fast forward to play
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeFastForwardTPlay(MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                  Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S  stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E speed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_FAST_FORWORD_2X:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("2X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_4X:
            speed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("4X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_8X:
            speed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("8X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_16X:
            speed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("16X_FAST_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_FORWORD_32X:
            speed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("32X_FAST_FORWARD\n");
            break;
        #if 0
        case MT_PVRPLAY_SPEED_FAST_FORWORD_64X:
            speed = MT_UNF_PVR_PLAY_SPEED_64X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("64X_FAST_FORWARD\n");
            break;
        #endif
        default:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("Use default speed (2x speed)\n");
    }

    stTrickMode.enSpeed = speed;
    Ret = MT_UNF_PVR_PlayTPlay(g_stTimeshiftRunInfo.playChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


static MT_S32 MT_PVRTimeshiftModeFastForwardPlay(MT_U32 PlayChn, mt_s32 speed)
{
    MT_S32                  Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S  stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(speed)
    {
        case 2:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("2X_FAST_FORWARD\n");
            break;
        case 4:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("4X_FAST_FORWARD\n");
            break;
        case 8:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("8X_FAST_FORWARD\n");
            break;
        case 16:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("16X_FAST_FORWARD\n");
            break;
        case 32:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("32X_FAST_FORWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


static MT_S32 MT_PVRTimeshiftModeFastBackwardPlay(MT_U32 PlayChn, mt_s32 speed)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E PlaySpeed = 0;
    switch(speed)
    {
        case 2:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("2X_FAST_BACKWARD\n");
            break;
        case 4:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("4X_FAST_BACKWARD\n");
            break;
        case 8:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("8X_FAST_BACKWARD\n");
            break;
        case 16:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("16X_FAST_BACKWARD\n");
            break;
        case 32:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("32X_FAST_BACKWARD\n");
            break;
        default:
            PlaySpeed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("Use default PlaySpeed (2x PlaySpeed)\n");
    }

    stTrickMode.enSpeed = PlaySpeed;
    Ret = MT_UNF_PVR_PlayTPlay(PlayChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}




/*!
@brief Rewind playback
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeFastBackwardTPlay(MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E speed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_2X:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("2X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_4X:
            speed = MT_UNF_PVR_PLAY_SPEED_4X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("4X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_8X:
            speed = MT_UNF_PVR_PLAY_SPEED_8X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("8X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_16X:
            speed = MT_UNF_PVR_PLAY_SPEED_16X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("16X_FAST_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_FAST_BACKWORD_32X:
            speed = MT_UNF_PVR_PLAY_SPEED_32X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("32X_FAST_BACKWARD\n");
            break;
         #if 0
         case MT_PVRPLAY_SPEED_FAST_BACKWORD_64X:
            speed = MT_UNF_PVR_PLAY_SPEED_64X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("64X_FAST_BACKWARD\n");
            break;
         #endif
        default:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD;
            MT_TIMESHIFT_PRINT("Use default speed (2x speed)\n");
    }

    stTrickMode.enSpeed = speed;
    Ret = MT_UNF_PVR_PlayTPlay(g_stTimeshiftRunInfo.playChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Slow forward playback
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeSlowForwardTPlay(MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E speed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_2X:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("2X_SLOW_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_4X:
            speed = MT_UNF_PVR_PLAY_SPEED_4X_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("4X_SLOW_FORWARD\n");
            break;
        #if 0
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_8X:
            speed = MT_UNF_PVR_PLAY_SPEED_8X_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("8X_SLOW_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_16X:
            speed = MT_UNF_PVR_PLAY_SPEED_16X_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("16X_SLOW_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_32X:
            speed = MT_UNF_PVR_PLAY_SPEED_32X_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("32X_SLOW_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_64X:
            speed = MT_UNF_PVR_PLAY_SPEED_64X_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("64X_SLOW_FORWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_FORWORD_43:
            speed = MT_UNF_PVR_PLAY_SPEED_43_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("3/4_SLOW_FORWARD\n");
            break;
        #endif
        default:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD;
            MT_TIMESHIFT_PRINT("Use default speed (2x speed)\n");
    }

    stTrickMode.enSpeed = speed;
    Ret = MT_UNF_PVR_PlayTPlay(g_stTimeshiftRunInfo.playChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Slow rewind playback
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeSlowBackwardTPlay(MT_PVRPLAY_SPEED_E mode)
{
    MT_S32                 Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
    MT_UNF_PVR_PLAY_SPEED_E speed = 0;
    switch(mode)
    {
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_2X:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_SLOW_BACKWARD;
            MT_TIMESHIFT_PRINT("2X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_4X:
            speed = MT_UNF_PVR_PLAY_SPEED_4X_SLOW_BACKWARD;
            MT_TIMESHIFT_PRINT("4X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_8X:
            speed = MT_UNF_PVR_PLAY_SPEED_8X_SLOW_BACKWARD;
            MT_TIMESHIFT_PRINT("8X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_16X:
            speed = MT_UNF_PVR_PLAY_SPEED_16X_SLOW_BACKWARD;
            MT_TIMESHIFT_PRINT("16X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_32X:
            speed = MT_UNF_PVR_PLAY_SPEED_32X_SLOW_BACKWARD;
            MT_TIMESHIFT_PRINT("32X_SLOW_BACKWARD\n");
            break;
        #if 0
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_64X:
            speed = MT_UNF_PVR_PLAY_SPEED_64X_SLOW_BACKWARD;
            MT_TIMESHIFT_PRINT("64X_SLOW_BACKWARD\n");
            break;
        case MT_PVRPLAY_SPEED_SLOW_BACKWORD_43:
            speed = MT_UNF_PVR_PLAY_SPEED_43_SLOW_BACKWARD;
            MT_TIMESHIFT_PRINT("3/4_SLOW_BACKWARD\n");
            break;
        #endif
        default:
            speed = MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD;
            MT_TIMESHIFT_PRINT("Use default speed (2x speed)\n");
    }

    stTrickMode.enSpeed = speed;
    Ret = MT_UNF_PVR_PlayTPlay(g_stTimeshiftRunInfo.playChn, &stTrickMode);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayTPlay failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Skip to the start position to play
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeSeekToStart(void)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = 0;
    stPos.s32Whence = SEEK_SET;
    Ret = MT_UNF_PVR_PlaySeek(g_stTimeshiftRunInfo.playChn, &stPos);
    if(Ret != MT_SUCCESS)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}


/*!
@brief Skip to the end position to play
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeSeekToEnd(void)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = 0;
    stPos.s32Whence = SEEK_END;
    Ret = MT_UNF_PVR_PlaySeek(g_stTimeshiftRunInfo.playChn, &stPos);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}


/*!
@brief Jump forward for 5 seconds
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeSeekForward(void)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = 5000;
    stPos.s32Whence = SEEK_CUR;
    Ret = MT_UNF_PVR_PlaySeek(g_stTimeshiftRunInfo.playChn, &stPos);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}


/*!
@brief Jump back for 5 seconds
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_PVRTimeshiftModeSeekBackward(mt_s32 time)
{
    MT_S32                     Ret = MT_FAILURE;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };

    stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
    stPos.s64Offset = -time;
    stPos.s32Whence = SEEK_CUR;
    Ret = MT_UNF_PVR_PlaySeek(g_stTimeshiftRunInfo.playChn, &stPos);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlaySeek failed.\n");
    }

    return MT_SUCCESS;
}

static MT_S32 MT_PVRTimeshiftAllTsStart(char *path, MT_U32 u32DemuxID, MT_U64 maxSize, MT_U32 *pRecChn, MT_BOOL bdio)
{
    MT_U32 recChn;
    MT_S32 ret = MT_SUCCESS;
    MT_UNF_PVR_REC_ATTR_S   attr = {0};
    PVR_PROG_INFO_S    fileInfo = {0};
    MT_CHAR    szFileName[PVR_MAX_FILENAME_LEN];

    sprintf(szFileName, "rec_all.ts");
    sprintf(attr.szFileName, "%s", path);
    strcat(attr.szFileName, "/");
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
    attr.enTsRecMode = MT_UNF_PVR_REC_FULL_TS_WITHOUT_NULL_PACKET;
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


static MT_S32 MT_PVRTimeshiftCheckkey(mt_pvr_rec_cipher_t rec_cipher)
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
        SAMPLE_TIMESHIFT_ERR_PRINT("the same key(The first 8b can't be the same as the last 8b) \n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


static MT_VOID MT_PVRTimeshiftModePrintMenu(MT_U32 prog_num)
{

    SAMPLE_TIMESHIFT_PRINT("\n");
    SAMPLE_TIMESHIFT_PRINT(" 1 - %d : select the program to play\n", prog_num);
    SAMPLE_TIMESHIFT_PRINT("    w0 : rec 1 \n");
    SAMPLE_TIMESHIFT_PRINT("    w1 : rec 2 \n");
    SAMPLE_TIMESHIFT_PRINT("    w2 : rec 3 \n");
    SAMPLE_TIMESHIFT_PRINT("     l : live play \n");
    SAMPLE_TIMESHIFT_PRINT("     x : Fast forward(Manually enter multiples:2/4/8/16/32) \n");
    SAMPLE_TIMESHIFT_PRINT("     c : Fast backward(Manually enter multiples:2/4/8/16/32) \n");
    SAMPLE_TIMESHIFT_PRINT("     f : Fast forward \n");
    SAMPLE_TIMESHIFT_PRINT("     r : Fast backward \n");
    SAMPLE_TIMESHIFT_PRINT("     s : Slow forward \n");
    SAMPLE_TIMESHIFT_PRINT("     g : Slow backward \n");
    SAMPLE_TIMESHIFT_PRINT("     i : Whether to record ttx/sub/cc/track \n");
    SAMPLE_TIMESHIFT_PRINT("     y : set dio open/close \n");
    SAMPLE_TIMESHIFT_PRINT("     m : Set whether to encrypt recording \n");
    SAMPLE_TIMESHIFT_PRINT("     n : Normal play \n");
    SAMPLE_TIMESHIFT_PRINT("     p : Pause and play,If on live will enter timeshift \n");
    SAMPLE_TIMESHIFT_PRINT("    t0 : stop record1 \n");
    SAMPLE_TIMESHIFT_PRINT("    t1 : stop record2 \n");
    SAMPLE_TIMESHIFT_PRINT("    t2 : stop record3 \n");
    SAMPLE_TIMESHIFT_PRINT("    t3 : stop timeshift \n");
    SAMPLE_TIMESHIFT_PRINT("     k : Seek to start \n");
    SAMPLE_TIMESHIFT_PRINT("     e : Seek to end \n");
    SAMPLE_TIMESHIFT_PRINT("     d : Seek forward 5 second \n");
    SAMPLE_TIMESHIFT_PRINT("     a : Seek rewwind 5 second \n");
    SAMPLE_TIMESHIFT_PRINT("     o : All stream recording \n");
    SAMPLE_TIMESHIFT_PRINT("     j : Stop all stream recording \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_TIMESHIFT_PRINT("     b : background run \n");
#endif

    SAMPLE_TIMESHIFT_PRINT("     h : help \n");
    SAMPLE_TIMESHIFT_PRINT("     q : quit \n");
    SAMPLE_TIMESHIFT_PRINT("PVR>> ");

}


/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@param[in] ppProgTable,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static  mt_s32 MT_PVRTimeshiftCmdTask(  mt_handle hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_CHAR   inputCmd[32] = { 0 };
    MT_S32    ret = MT_FAILURE;
    MT_S32    Ret = MT_FAILURE;
    MT_UNF_PVR_REC_ATTR_S   RecAttr = { 0 };
    MT_BOOL   NormalPlayStatue = MT_TRUE;
    MT_S32    speed = 0;
    mt_u32    u32ProgNum = 1;
#ifdef MT_SAMPLE_APP
    u32ProgNum = play_resource.s32ProgNum;
#endif
    MT_S32    rec1_num = 0;
    MT_S32    rec2_num = 0;
    MT_S32    rec3_num = 0;
    MT_BOOL   bDoCipher = 0;
    MT_BOOL   bdio = MT_TRUE;
    MT_BOOL   binfo = MT_TRUE;
    PMT_COMPACT_PROG *stCurrentProgInfo = pProgTbl->proginfo + (u32ProgNum -1);
    mt_pvr_rec_cipher_t rec_cipher = {0};

    if(MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }


    while(1)
    {
        (void)MT_PVRTimeshiftModePrintMenu(pProgTbl->prog_num);

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            g_bTaskQuit = MT_TRUE;
            if(g_stTimeshiftRunInfo.bLIve == MT_FALSE)
            {
               SAMPLE_TIMESHIFT_INFO_PRINT("Switch back to live\n");
               (void)MT_PVRTimeshiftSwitchToLivePlay(g_stTimeshiftRunInfo.playChn, hAvPlay, stCurrentProgInfo, &g_stTimeshiftRunInfo.sInputParam);
               g_stTimeshiftRunInfo.bLIve = MT_TRUE;
               g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
               g_stTimeshiftRunInfo.playChn = MT_INVALID_HANDLE;
               speed = MT_PVRPLAY_SPEED_NORMAL;
            }
            SAMPLE_TIMESHIFT_INFO_PRINT("prepare to exit!\n");
            break;

        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_TIMESHIFT_INFO_PRINT("PVR_timeshift in back!\n");
            break;
        }
#endif
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {   //switch pg
            if(g_stTimeshiftRunInfo.bLIve)
            {
                u32ProgNum = atoi(inputCmd);
                if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
                {
                    stCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                    ret = MT_PVRTimeshiftStopplay(hAvPlay);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_DispZoom_Stopplay failed.\n");

                    }
                    
                    SAMPLE_TIMESHIFT_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);
                    // restore ac4    attr info.
                    MTADP_AUD_RestoreAc4PlayAttrInfo(hAvPlay);
                    ret  = MT_PVRTimeshiftAVPlay_Start(hAvPlay, stCurrentProgInfo);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT(" SwitchProg failed.\n");
                    }
                }
                else
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
                }
            }
            else
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("play is live!\n");
            }
#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
        else if('t' == inputCmd[0])
        {
            if('0' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1)
                {
                    (MT_VOID)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn1);
                    g_stTimeshiftRunInfo.recChn1 = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec 0\n");
                    rec1_num = 0;
                }
            }

            else if('1' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2)
                {
                    (MT_VOID)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn2);
                    g_stTimeshiftRunInfo.recChn2 = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec 1\n");
                    rec2_num = 0;
                }
            }
            else if('2' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3)
                {
                    (MT_VOID)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn3);
                    g_stTimeshiftRunInfo.recChn3 = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec 2\n");
                    rec3_num = 0;
                }
            }
            else if('3' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.playChn)
                {
                    (MT_VOID)MT_PVRTimeshiftSwitchToLivePlay(g_stTimeshiftRunInfo.playChn, hAvPlay, stCurrentProgInfo, &g_stTimeshiftRunInfo.sInputParam);
                    g_stTimeshiftRunInfo.playChn = MT_INVALID_HANDLE;
                    g_stTimeshiftRunInfo.firstRecTime = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_INFO_PRINT("++switch to live\n");
                    g_stTimeshiftRunInfo.bLIve = MT_TRUE;
                    g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
                    speed = MT_PVRPLAY_SPEED_NORMAL;
                }

                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn4)
                {
                    (MT_VOID)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn4);
                    g_stTimeshiftRunInfo.recChn4 = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec 3\n");
                }
            }
        }
        else if('w' == inputCmd[0])
        {
            if(g_stTimeshiftRunInfo.bLIve == MT_FALSE)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("is not live \n");
                continue;
            }
            if('0' == inputCmd[1])
            {
                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1)
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("rec 0 is start \n");
                    continue;
                }
                rec1_num = u32ProgNum;
                if((rec2_num == rec1_num) || (rec3_num == rec1_num))
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("The same program \n");
                    rec1_num = 0;
                    continue;
                }
                ret = MTADP_PVR_RecStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC, 0, bDoCipher, 0, &g_stTimeshiftRunInfo.recChn1, bdio, rec_cipher, binfo);
                if(MT_SUCCESS != ret)
                {
                    g_stTimeshiftRunInfo.recChn1 = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_PVR_RecStart failed.\n");
                }


            }
            else if('1' == inputCmd[1])
            {

                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2)
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("rec 1 is start \n");
                    continue;
                }
                rec2_num = u32ProgNum;

                if((rec2_num == rec1_num) || (rec2_num == rec3_num))
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("The same program \n");
                    rec2_num = 0;
                    continue;
                }
                ret = MTADP_PVR_RecStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC+2, 0, bDoCipher, 0, &g_stTimeshiftRunInfo.recChn2, bdio, rec_cipher, binfo);
                if(MT_SUCCESS != ret)
                {
                    g_stTimeshiftRunInfo.recChn2 = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_PVR_RecStart failed.\n");
                }
            }
            else if('2' == inputCmd[1])
            {

                if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3)
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("rec 2 is start \n");
                    continue;
                }
                rec3_num = u32ProgNum;

                if((rec3_num == rec1_num) || (rec3_num == rec2_num))
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("The same program \n");
                    rec3_num = 0;
                    continue;
                }
                ret = MTADP_PVR_RecStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_LIVE, 0, bDoCipher, 0, &g_stTimeshiftRunInfo.recChn3, bdio, rec_cipher, binfo);
                if(MT_SUCCESS != ret)
                {
                    g_stTimeshiftRunInfo.recChn3 = MT_INVALID_HANDLE;
                    SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_PVR_RecStart failed.\n");
                }
            }


        }

        else if('p' == inputCmd[0])
        {
            if(MT_TRUE == g_stTimeshiftRunInfo.bPAuse)
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("PVR resume test.\n");
                ret = MT_UNF_PVR_PlayResumeChn(g_stTimeshiftRunInfo.playChn);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                    continue;
                }

                g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
            }
            else
            {
                if(MT_INVALID_HANDLE == g_stTimeshiftRunInfo.recChn4)
                {
                    if((u32ProgNum == rec1_num) ||(u32ProgNum == rec2_num) ||(u32ProgNum == rec3_num))
                    {
                        mt_u32 copy_chn = MT_INVALID_HANDLE;
                        if(u32ProgNum == rec1_num)
                        {
                            copy_chn = g_stTimeshiftRunInfo.recChn1;
                        }
                        else if(u32ProgNum == rec2_num)
                        {
                            copy_chn = g_stTimeshiftRunInfo.recChn2;
                        }
                        else if(u32ProgNum == rec3_num)
                        {
                            copy_chn = g_stTimeshiftRunInfo.recChn3;
                        }
                        ret = MTADP_PVR_RecCopy((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC+3, 1, bDoCipher, MAX_TIMESHIFT_REC_FILE_SIZE, &g_stTimeshiftRunInfo.recChn4, copy_chn, bdio);
                        if(MT_SUCCESS != ret)
                        {
                            g_stTimeshiftRunInfo.recChn4 = MT_INVALID_HANDLE;
                            SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_PVR_RecCopy failed.\n");
                        }
                         SAMPLE_TIMESHIFT_INFO_PRINT("switch to timeshift...\n");

                        memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                        (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn4, &RecAttr);

                        ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                        if(MT_SUCCESS != ret)
                        {
                            SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                            continue;
                        }
                        SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);
                        g_stTimeshiftRunInfo.bLIve = MT_FALSE;

                        /* when shift, just to pause*/
                        SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now test.\n");
                        ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                        if(MT_SUCCESS != ret)
                        {
                            SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                            continue;
                        }
                        g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                    }
                    else
                    {
                        ret = MTADP_PVR_RecStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC+3, 1, bDoCipher, MAX_TIMESHIFT_REC_FILE_SIZE, &g_stTimeshiftRunInfo.recChn4, bdio, rec_cipher, binfo);
                        if(MT_SUCCESS != ret)
                        {
                            g_stTimeshiftRunInfo.recChn4 = MT_INVALID_HANDLE;
                            SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_PVR_RecCopy failed.\n");
                        }
                         SAMPLE_TIMESHIFT_INFO_PRINT("switch to timeshift...\n");

                        memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                        (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn4, &RecAttr);

                        ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                        if(MT_SUCCESS != ret)
                        {
                            SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                            continue;
                        }
                        SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);
                        g_stTimeshiftRunInfo.bLIve = MT_FALSE;

                        /* when shift, just to pause*/
                        SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now test.\n");
                        ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                        if(MT_SUCCESS != ret)
                        {
                            SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                            continue;
                        }
                        g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                    }
                }
                else if(g_stTimeshiftRunInfo.bLIve)
                {
                     SAMPLE_TIMESHIFT_INFO_PRINT("switch to timeshift...\n");

                    memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                    (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn4, &RecAttr);

                    ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                        continue;
                    }
                    SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);
                    g_stTimeshiftRunInfo.bLIve = MT_FALSE;

                    /* when shift, just to pause*/
                    SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now test.\n");
                    ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                        continue;
                    }
                    g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                }
                else
                {
                    SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now test1.\n");
                    ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                        continue;
                    }
                    g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                }
            }

        }
        else if('l' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {

               SAMPLE_TIMESHIFT_INFO_PRINT("Switch back to live\n");
               (void)MT_PVRTimeshiftSwitchToLivePlay(g_stTimeshiftRunInfo.playChn, hAvPlay, stCurrentProgInfo, &g_stTimeshiftRunInfo.sInputParam);
               g_stTimeshiftRunInfo.bLIve = MT_TRUE;
               g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
               g_stTimeshiftRunInfo.playChn = MT_INVALID_HANDLE;
            }

        }
        else if('i' == inputCmd[0])
        {

            if((MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3))
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("The program is recording. The binfo setting is invalid \n");
                continue;
            }
            if(binfo == MT_TRUE)
            {
                binfo = MT_FALSE;
                SAMPLE_TIMESHIFT_INFO_PRINT("Close rec all info \n");
            }
            else
            {
                binfo = MT_TRUE;
                SAMPLE_TIMESHIFT_INFO_PRINT("Open rec all info \n");
            }
            SAMPLE_TIMESHIFT_INFO_PRINT("binfo = %d\n", binfo);
        }
        else if('y' == inputCmd[0])
        {

            if((MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3))
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("The program is recording. The bDoCipher setting is invalid \n");
                continue;
            }
            if(bdio == MT_TRUE)
            {
                bdio = MT_FALSE;
                SAMPLE_TIMESHIFT_INFO_PRINT("Close dio \n");
            }
            else
            {
                bdio = MT_TRUE;
                SAMPLE_TIMESHIFT_INFO_PRINT("Open dio \n");
            }
            SAMPLE_TIMESHIFT_INFO_PRINT("bdio = %d\n", bdio);
        }
        else if('m' == inputCmd[0])
        {
            if((MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3))
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("The program is recording. The bDoCipher setting is invalid \n");
                continue;
            }
            if(bDoCipher > 0)
            {
                bDoCipher = 0;
                SAMPLE_TIMESHIFT_INFO_PRINT("Turn off encryption \n");
            }
            else
            {
                bDoCipher = 1;
                SAMPLE_TIMESHIFT_INFO_PRINT("Turning on encryption \n");
            }
            SAMPLE_TIMESHIFT_INFO_PRINT("bDoCipher = %d\n", bDoCipher);

            if(0 == bDoCipher)
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("Please turn on the bDoCipher switch first \n");
                continue;
            }
            SAMPLE_TIMESHIFT_INFO_PRINT("Whether to use your own keys and encryption , 0:MT_FALSE,1:MT_TRUE \n");
            fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
            rec_cipher.bkey = atoi(inputCmd);

            if(rec_cipher.bkey == MT_TRUE)
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("Please enter the encryption type \n");
                SAMPLE_TIMESHIFT_INFO_PRINT("0:MT_CIPHER_ALG_DES 1:MT_CIPHER_ALG_TDES 2:MT_CIPHER_ALG_AES \n");
                SAMPLE_TIMESHIFT_INFO_PRINT("pvr supports only the  three ciphers \n");
                scanf("%d", (mt_s32*)&rec_cipher.enType);
                if(rec_cipher.enType > 2)
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("type err! \n");
                    memset(&rec_cipher, 0, sizeof(rec_cipher));
                    continue;
                }


                rec_cipher.u32KeyLen = 16;

                SAMPLE_TIMESHIFT_INFO_PRINT("Please enter the encryption key(16b) \n");

                for(int i = 0; i<rec_cipher.u32KeyLen; i++)
                {
                    scanf("%c", &rec_cipher.au8Key[i]);
                }
                if(rec_cipher.enType == 1)
                {
                    ret = MT_PVRTimeshiftCheckkey(rec_cipher);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("key err! \n");
                        memset(&rec_cipher, 0, sizeof(rec_cipher));
                    }
                }

            }
        }

        else if('v' == inputCmd[0])
        {
            /* when pause ,to shift play */
            if(g_stTimeshiftRunInfo.bPAuse)
            {
                /* when shift, just to resume*/

                SAMPLE_TIMESHIFT_INFO_PRINT("PVR resume now.\n");
                ret = MT_UNF_PVR_PlayResumeChn(g_stTimeshiftRunInfo.playChn);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                    continue;
                }

                g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
            }
            /* to pause */
            else
            {
                if(g_stTimeshiftRunInfo.bLIve)
                {
                    if(MT_INVALID_HANDLE == g_stTimeshiftRunInfo.recChn1)
                    {
                        if(u32ProgNum == rec2_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn2, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 6.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");

                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else if(u32ProgNum == rec3_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn3, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 7.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");

                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else
                        {
                            rec1_num = u32ProgNum;
                            SAMPLE_TIMESHIFT_INFO_PRINT("start rec1 ,is live\n");
                            ret = MTADP_PVR_RecStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC, 1, bDoCipher, MAX_TIMESHIFT_REC_FILE_SIZE, &g_stTimeshiftRunInfo.recChn1, bdio, rec_cipher, binfo);
                            if(MT_SUCCESS != ret)
                            {
                                g_stTimeshiftRunInfo.recChn1 = MT_INVALID_HANDLE;
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MTADP_PVR_RecStart failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("switch to timeshift...\n");

                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn1, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;

                            /* when shift, just to pause*/
                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 5.\n");
                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                    }
                    else if(MT_INVALID_HANDLE == g_stTimeshiftRunInfo.recChn2)
                    {
                        if(u32ProgNum == rec1_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn1, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 4.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                continue;
                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else if(u32ProgNum == rec3_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn3, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 8.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                continue;
                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else
                        {
                            rec2_num = u32ProgNum;

                            SAMPLE_TIMESHIFT_INFO_PRINT("start rec2 ,is live\n");
                            ret = MTADP_PVR_RecStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_REC+2, 1, bDoCipher, MAX_TIMESHIFT_REC_FILE_SIZE, &g_stTimeshiftRunInfo.recChn2, bdio, rec_cipher, binfo);
                            if(MT_SUCCESS != ret)
                            {
                                g_stTimeshiftRunInfo.recChn2 = MT_INVALID_HANDLE;
                                SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_PVR_RecStart failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("switch to timeshift...\n");

                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn2, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;

                            /* when shift, just to pause*/
                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 0.\n");
                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                    }
                    else if(MT_INVALID_HANDLE == g_stTimeshiftRunInfo.recChn3)
                    {
                        if(u32ProgNum == rec2_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn2, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 9.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");

                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else if(u32ProgNum == rec1_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn1, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 10.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");

                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else
                        {
                            rec3_num = u32ProgNum;
                            SAMPLE_TIMESHIFT_INFO_PRINT("start rec3 ,is live\n");
                            ret = MTADP_PVR_RecStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, stCurrentProgInfo, PVR_DMX_ID_LIVE, 1, bDoCipher, MAX_TIMESHIFT_REC_FILE_SIZE, &g_stTimeshiftRunInfo.recChn3, bdio, rec_cipher, binfo);
                            if(MT_SUCCESS != ret)
                            {
                                g_stTimeshiftRunInfo.recChn3 = MT_INVALID_HANDLE;
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MTADP_PVR_RecStart failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("switch to timeshift...\n");

                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn3, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;

                            /* when shift, just to pause*/
                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 11.\n");
                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                    }


                    else
                    {
                        if(u32ProgNum == rec1_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn1, &RecAttr);

                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 1.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftModeSeekToEnd failed.\n");
                                continue;
                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                        }
                        else if(u32ProgNum == rec2_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn2, &RecAttr);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 2.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                continue;
                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else if(u32ProgNum == rec3_num)
                        {
                            memset(&RecAttr, 0, sizeof(MT_UNF_PVR_REC_ATTR_S));
                            (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn3, &RecAttr);
                            g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                            g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
                            ret = MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_PVRTimeshiftSwitchToShiftPlay failed.\n");
                                continue;
                            }
                            SAMPLE_TIMESHIFT_INFO_PRINT("Create play chan[%x] \n", g_stTimeshiftRunInfo.playChn);

                            SAMPLE_TIMESHIFT_INFO_PRINT("PVR pause now 3.\n");

                            ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                            if(MT_SUCCESS != ret)
                            {
                                SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                                continue;
                            }
                            Ret = MT_PVRTimeshiftModeSeekToEnd();
                            if(MT_SUCCESS != Ret)
                            {
                                continue;
                            }
                            (void)MT_PVRTimeshiftModeSeekBackward(500);
                            g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                        }
                        else
                        {
                            SAMPLE_TIMESHIFT_ERR_PRINT("this play is live, can not pause \n");
                            SAMPLE_TIMESHIFT_ERR_PRINT("rec num is max ! \n");
                            SAMPLE_TIMESHIFT_ERR_PRINT("If you need to record, stop the first two \n");
                        }

                    }

                }
                else
                {
                    SAMPLE_TIMESHIFT_INFO_PRINT("PVR Play pause now 3 playChn[%x].\n",g_stTimeshiftRunInfo.playChn);
                    ret = MT_UNF_PVR_PlayPauseChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayPauseChn failed.\n");
                        continue;
                    }
                    g_stTimeshiftRunInfo.bPAuse = MT_TRUE;
                }

            }
        }
        else if('x' == inputCmd[0])
        {
            SAMPLE_TIMESHIFT_INFO_PRINT("Input fast forward speed (2/4/8/16/32)\n");
            scanf("%d", &speed);
            Ret = MT_PVRTimeshiftModeFastForwardPlay(g_stTimeshiftRunInfo.playChn, speed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }
            NormalPlayStatue = MT_FALSE;
        }
        else if('c' == inputCmd[0])
        {
            SAMPLE_TIMESHIFT_INFO_PRINT("Input fast backward speed (2/4/8/16/32)\n");
            scanf("%d", &speed);
            Ret = MT_PVRTimeshiftModeFastBackwardPlay(g_stTimeshiftRunInfo.playChn, speed);
            if(MT_SUCCESS != Ret)
            {
                continue;
            }
            NormalPlayStatue = MT_FALSE;
        }
        else if('f' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(speed > MT_PVRPLAY_SPEED_FAST_FORWORD_MODE && speed < MT_PVRPLAY_SPEED_FAST_FORWORD_MAX - 1)
                {
                    speed++;
                }
                else
                {
                    speed = MT_PVRPLAY_SPEED_FAST_FORWORD_2X;
                }

                Ret = MT_PVRTimeshiftModeFastForwardTPlay(speed);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_TIMESHIFT_ERR_PRINT("fast err \n");
                    continue;
                }

                NormalPlayStatue = MT_FALSE;
                SAMPLE_TIMESHIFT_INFO_PRINT("PVR fast forward to play now.\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }
        else if('r' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(speed > MT_PVRPLAY_SPEED_FAST_BACKWORD_MODE && speed < MT_PVRPLAY_SPEED_FAST_BACKWORD_MAX - 1)
                {
                    speed++;
                }
                else
                {
                    speed = MT_PVRPLAY_SPEED_FAST_BACKWORD_2X;
                }

                Ret = MT_PVRTimeshiftModeFastBackwardTPlay(speed);
                if(MT_SUCCESS != Ret)
                {
                    continue;
                }

                NormalPlayStatue = MT_FALSE;
                SAMPLE_TIMESHIFT_INFO_PRINT("PVR fast rewind playback now.\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }
        else if('s' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(speed > MT_PVRPLAY_SPEED_SLOW_FORWORD_MODE && speed < MT_PVRPLAY_SPEED_SLOW_FORWORD_MAX - 1)
                {
                    speed++;
                }
                else
                {
                    speed = MT_PVRPLAY_SPEED_SLOW_FORWORD_2X;
                }

                Ret = MT_PVRTimeshiftModeSlowForwardTPlay(speed);
                if(MT_SUCCESS != Ret)
                {
                    continue;
                }

                NormalPlayStatue = MT_FALSE;
                SAMPLE_TIMESHIFT_INFO_PRINT("PVR slow forward to play now.\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }
        else if('g' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(speed > MT_PVRPLAY_SPEED_SLOW_BACKWORD_MODE && speed < MT_PVRPLAY_SPEED_SLOW_BACKWORD_MAX - 1)
                {
                    speed++;
                }
                else
                {
                    speed = MT_PVRPLAY_SPEED_SLOW_BACKWORD_2X;
                }

                Ret = MT_PVRTimeshiftModeSlowBackwardTPlay(speed);
                if(MT_SUCCESS != Ret)
                {
                    continue;
                }

                NormalPlayStatue = MT_FALSE;
                SAMPLE_TIMESHIFT_INFO_PRINT("PVR slow rewind playback now.\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }

        else if('k' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(MT_FALSE == NormalPlayStatue)
                {
                    Ret = MT_UNF_PVR_PlayResumeChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != Ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                        continue;
                    }
                    NormalPlayStatue = MT_TRUE;
                }

                Ret = MT_PVRTimeshiftModeSeekToStart();
                if(MT_SUCCESS != Ret)
                {
                    continue;
                }

                // restore ac4    attr info.
                MT_PVRTimeshiftRestoreAc4PlayAttrInfo(hAvPlay);

                speed = MT_PVRPLAY_SPEED_NORMAL;
                SAMPLE_TIMESHIFT_INFO_PRINT("Play from the starting position.\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }

        }
        else if('e' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(MT_FALSE == NormalPlayStatue)
                {
                    Ret = MT_UNF_PVR_PlayResumeChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != Ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                        continue;
                    }
                    NormalPlayStatue = MT_TRUE;
                }

                Ret = MT_PVRTimeshiftModeSeekToEnd();
                if(MT_SUCCESS != Ret)
                {
                    continue;
                }

                speed = MT_PVRPLAY_SPEED_NORMAL;
                SAMPLE_TIMESHIFT_INFO_PRINT("Play to the end.\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }
        else if('d' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(MT_FALSE == NormalPlayStatue)
                {
                    Ret = MT_UNF_PVR_PlayResumeChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != Ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                        continue;
                    }
                    NormalPlayStatue = MT_TRUE;
                }

                Ret = MT_PVRTimeshiftModeSeekForward();
                if(MT_SUCCESS != Ret)
                {
                    continue;
                }

                // restore ac4    attr info.
                MT_PVRTimeshiftRestoreAc4PlayAttrInfo(hAvPlay);

                speed = MT_PVRPLAY_SPEED_NORMAL;
                SAMPLE_TIMESHIFT_INFO_PRINT("seek forward 5 Second\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }
        else if('a' == inputCmd[0])
        {
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                if(MT_FALSE == NormalPlayStatue)
                {
                    Ret = MT_UNF_PVR_PlayResumeChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != Ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                        continue;
                    }
                    NormalPlayStatue = MT_TRUE;
                }

                Ret = MT_PVRTimeshiftModeSeekBackward(5000);
                if(MT_SUCCESS != Ret)
                {
                    continue;
                }

                // restore ac4    attr info.
                MT_PVRTimeshiftRestoreAc4PlayAttrInfo(hAvPlay);

                speed = MT_PVRPLAY_SPEED_NORMAL;
                SAMPLE_TIMESHIFT_INFO_PRINT("seek reward 5 Second\n");
                continue;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }

        else if('n' == inputCmd[0])
        {
            MT_UNF_PVR_PLAY_MODE_S stTrickMode = { 0 };
            if(!g_stTimeshiftRunInfo.bLIve)
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("PVR normal play now.\n");

                if(MT_INVALID_HANDLE == g_stTimeshiftRunInfo.playChn)
                {
                    (void)MT_UNF_PVR_RecGetChn(g_stTimeshiftRunInfo.recChn1,&RecAttr);
                    SAMPLE_TIMESHIFT_INFO_PRINT("switch to timeshift:%s\n", RecAttr.szFileName);
                    (void)MT_PVRTimeshiftSwitchToShiftPlay(RecAttr.szFileName, &g_stTimeshiftRunInfo.playChn, hAvPlay);
                    stTrickMode.enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
                    (void)MT_UNF_PVR_PlayTPlay(g_stTimeshiftRunInfo.playChn, &stTrickMode);
                    SAMPLE_TIMESHIFT_INFO_PRINT("PlayChn ============= %d\n", g_stTimeshiftRunInfo.playChn);
                    speed = MT_PVRPLAY_SPEED_NORMAL;
                    g_stTimeshiftRunInfo.bLIve = MT_FALSE;
                }
                else
                {
                    stTrickMode.enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
                    (void)MT_UNF_PVR_PlayTPlay(g_stTimeshiftRunInfo.playChn, &stTrickMode);
                    ret = MT_UNF_PVR_PlayResumeChn(g_stTimeshiftRunInfo.playChn);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_TIMESHIFT_ERR_PRINT("call MT_UNF_PVR_PlayResumeChn failed.\n");
                        return ret;
                    }
                    speed = MT_PVRPLAY_SPEED_NORMAL;
                }

                // restore ac4    attr info.
                MT_PVRTimeshiftRestoreAc4PlayAttrInfo(hAvPlay);
                
                g_stTimeshiftRunInfo.bPAuse = MT_FALSE;
            }
            else
            {
                SAMPLE_TIMESHIFT_INFO_PRINT("play is live\n");
            }
        }
        else if('o' == inputCmd[0])
        {
            if((MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1) || (MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn4))
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("Single program recording has been initiated \n");
                continue;
            }

            ret = MT_PVRTimeshiftAllTsStart((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, PVR_DMX_ID_REC+3,  0, &g_stTimeshiftRunInfo.recChn4, bdio);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT(" MTADP_PVR_RecStart3 failed.\n");
            }


        }
        else if('j' == inputCmd[0])
        {
             if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn4)
             {
                 (MT_VOID)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn4);
                 g_stTimeshiftRunInfo.recChn4 = MT_INVALID_HANDLE;
                 SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec all\n");
              }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_TIMESHIFT_INFO_PRINT("Print help info \n");
            continue;
        }
    }

    return MT_SUCCESS;


}

static MT_VOID MT_PVRTimeshiftExit(void)
{
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1)
    {
        (void)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn1);
        g_stTimeshiftRunInfo.recChn1 = MT_INVALID_HANDLE;
        SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec 1\n");
    }
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2)
    {
        (void)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn2);
        g_stTimeshiftRunInfo.recChn2 = MT_INVALID_HANDLE;
        SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec 2\n");
    }
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3)
    {
        (void)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn3);
        g_stTimeshiftRunInfo.recChn3 = MT_INVALID_HANDLE;
        SAMPLE_TIMESHIFT_INFO_PRINT("++stop rec 3\n");
    }
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.playChn)
    {
        (void)MTADP_PVR_StopPlayBack(g_stTimeshiftRunInfo.playChn);
        g_stTimeshiftRunInfo.playChn = MT_INVALID_HANDLE;
        g_stTimeshiftRunInfo.firstRecTime = MT_INVALID_HANDLE;
        SAMPLE_TIMESHIFT_INFO_PRINT("++stop to live\n");
    }

    g_stTimeshiftRunInfo.bIsPlayStop = MT_TRUE;
    (MT_VOID)pthread_join(g_stTimeshiftRunInfo.statusThd, MT_NULL);


    (MT_VOID)MT_PVRTimeshiftStopplay(g_stTimeshiftRunInfo.hAvPlay);


    (MT_VOID)MT_UNF_PVR_PlayDeInit();


    (MT_VOID)MT_UNF_PVR_RecDeInit();


    (MT_VOID)MT_PVRTimeshiftAvplayDeInit(g_stTimeshiftRunInfo.hAvPlay, g_stTimeshiftRunInfo.hWin, g_stTimeshiftRunInfo.hSoundTrack);


    (MT_VOID)MTADP_Search_FreeAllPmt(g_stTimeshiftRunInfo.pProgTbl);


    (MT_VOID)MTADP_Search_DeInit();


    (MT_VOID)MT_PVRTimeshiftDmxDeInit();


    (MT_VOID)MTADP_VO_DeInit();


    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_Fe_DeInit(g_stTimeshiftRunInfo.sInputParam.tuner_id);

    memset(&g_stTimeshiftRunInfo, 0xff, sizeof(g_stTimeshiftRunInfo));
    g_bTaskQuit = MT_TRUE;
    play_resource.demux_use = MT_FALSE;
}



/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_PVRTimeshiftParase_args(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;

    SAMPLE_TIMESHIFT_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:t:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_PVRTimeshiftPrint_help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_PVRTimeshiftExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                MTADP_Strncpy((mt_char*)pInputParam->folder.file_name, mt_optarg, sizeof(mt_rec_file_para_t));
                break;
#ifndef MT_SAMPLE_APP
            case 't':
                pInputParam->tuner_id = strtol(mt_optarg, 0, 0);

                return MT_SUCCESS;
            case 's':
                if(argc != 11)
                {
                    (void)MT_PVRTimeshiftPrintDvbsHelp(argv[0]);
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
                if(argc != 9)
                {
                    (void)MT_PVRTimeshiftPrintDvbcHelp(argv[0]);
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
    SAMPLE_TIMESHIFT_FUNCTION_EXIT();

    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_PVRTimeshiftMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 ret = MT_SUCCESS;
    struct stat statbuf;
    mt_s32 exists;

#ifndef MT_SAMPLE_APP
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#endif

    if(argc < 3  && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_PVRTimeshiftPrint_help(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_PVRTimeshiftParase_args(argc, argv, &g_stTimeshiftRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    // check whether the pvr directory exists.
    exists = stat((mt_char*)g_stTimeshiftRunInfo.sInputParam.folder.file_name, &statbuf);
    if (exists == 0 && S_ISDIR(statbuf.st_mode)) {
        SAMPLE_TIMESHIFT_INFO_PRINT("[%s] exists \n",
            g_stTimeshiftRunInfo.sInputParam.folder.file_name);
    } else {
        SAMPLE_TIMESHIFT_ERR_PRINT("[%s] does not exist, please check again.\n",
            g_stTimeshiftRunInfo.sInputParam.folder.file_name);
        return MT_FAILURE;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }


        ret = MTADP_Fe_Init(g_stTimeshiftRunInfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR1;
        }

        if(MT_INPUT_SIG_TYPE_CAB == g_stTimeshiftRunInfo.sInputParam.sig_type)
        {     //dvbc
            ret = MT_PVRTimeshiftCheckDvbcParam(&g_stTimeshiftRunInfo.sInputParam.input_param.cab);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("Input sat parameter error!\n");
                goto ERR2;
            }
            ret = MTADP_Fe_Connect_Dvbc(g_stTimeshiftRunInfo.sInputParam.tuner_id,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.cab.freq,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.cab.sym_rate,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.cab.mod_type);
        }
        else if(MT_INPUT_SIG_TYPE_SAT == g_stTimeshiftRunInfo.sInputParam.sig_type)
        {
            ret = MT_PVRTimeshiftCheckDvbsParam(&g_stTimeshiftRunInfo.sInputParam.input_param.sat);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TIMESHIFT_ERR_PRINT("Input sat parameter error!\n");
                goto ERR2;
            }
            ret = MTADP_Fe_Connect_Dvbs(g_stTimeshiftRunInfo.sInputParam.tuner_id,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.sat.freq,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.sat.sym_rate,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.sat.onoff_22k,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.sat.polarization,
                                    g_stTimeshiftRunInfo.sInputParam.input_param.sat.port_type);
        }

        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR2;
        }



        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("MTADP_Disp_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            goto ERR3;
        }


        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_PVRTimeshiftDmxInit(&g_stTimeshiftRunInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT( "failed to StartDmx\n");
            goto ERR6;
        }

        (void)MTADP_Search_Init();

        ret = MTADP_Search_GetAllPmt(PVR_DMX_ID_REC, &g_stTimeshiftRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR8;
        }

        ret = MT_PVRTimeshiftAVplayInit(&g_stTimeshiftRunInfo.hAvPlay, &g_stTimeshiftRunInfo.hWin, &g_stTimeshiftRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MT_PVRTimeshiftAVplayInit\n");
            goto ERR9;
        }
#else
        if(0 == avplayHandle.hAvPlay)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("No play chanl. stop window.\n");

            return MT_FAILURE;
        }
        g_stTimeshiftRunInfo.hAvPlay = avplayHandle.hAvPlay;
        g_stTimeshiftRunInfo.hSoundTrack = avplayHandle.hSoundTrack;
        g_stTimeshiftRunInfo.hWin = avplayHandle.hWin;


#endif

        ret = MT_UNF_PVR_RecInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_RecInit failed.\n");
            goto ERR10;
        }


        ret = MT_UNF_PVR_PlayInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_PVR_PlayInit failed.\n");
            goto ERR11;
        }

        ret = MTADP_PVR_RegisterCallBacks(&g_stTimeshiftRunInfo.hAvPlay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" PVR_RegisterCallBacks failed.\n");
            goto ERR12;
        }

        g_stTimeshiftRunInfo.bLIve = MT_TRUE;
        g_stTimeshiftRunInfo.bPAuse = MT_FALSE;

#ifndef MT_SAMPLE_APP
        /* Play the first program on the program list*/
        pstCurrentProgInfo = g_stTimeshiftRunInfo.pProgTbl->proginfo;
        ret = MT_PVRTimeshiftAVPlay_Start(g_stTimeshiftRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("failed to MT_PVRTimeshiftAVPlay_Start\n");
            goto ERR13;
        }
#else
        ret = MTADP_Search_get_proglist(&g_stTimeshiftRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT("call MTADP_Search_get_proglist failed.\n");
            goto ERR13;
        }

#endif


        g_stTimeshiftRunInfo.recChn1 = MT_INVALID_HANDLE;
        g_stTimeshiftRunInfo.recChn2 = MT_INVALID_HANDLE;
        g_stTimeshiftRunInfo.recChn3 = MT_INVALID_HANDLE;
        g_stTimeshiftRunInfo.recChn4 = MT_INVALID_HANDLE;
        g_stTimeshiftRunInfo.playChn = MT_INVALID_HANDLE;
        g_stTimeshiftRunInfo.firstRecTime = MT_INVALID_HANDLE;
        g_stTimeshiftRunInfo.bIsPlayStop = MT_FALSE;
        g_bTaskQuit = MT_FALSE;

        ret = pthread_create(&g_stTimeshiftRunInfo.statusThd, MT_NULL, MT_PVRTimeshiftstatuThread, MT_NULL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" pthread_create  timeThread failed.\n");
            goto ERR13;
        }

        if(play_resource.sig_type == 0)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
        }
        else if(play_resource.sig_type == 1)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, DMX_DVB_TSI_IN_PORT);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, DMX_DVB_TSI_IN_PORT);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, DMX_DVB_TSI_IN_PORT);
        }
        else if(play_resource.sig_type == 2)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_2);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_2);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_2);
        }
        else if(play_resource.sig_type == 3)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_3);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_3);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_3);
#else
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
#endif

        }
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TIMESHIFT_ERR_PRINT(" MT_UNF_DMX_AttachTSPort failed.\n");
            goto ERR14;
        }



    }
#ifdef MT_SAMPLE_APP
    if(play_resource.demux_use == MT_TRUE)
    {

        g_stTimeshiftRunInfo.hAvPlay = avplayHandle.hAvPlay;
        g_stTimeshiftRunInfo.hSoundTrack = avplayHandle.hSoundTrack;
        g_stTimeshiftRunInfo.hWin = avplayHandle.hWin;

        g_stTimeshiftRunInfo.bLIve = MT_TRUE;
        g_stTimeshiftRunInfo.bPAuse = MT_FALSE;

        (MT_VOID)MTADP_Search_get_proglist(&g_stTimeshiftRunInfo.pProgTbl);

        if(play_resource.sig_type == 0)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
        }
        else if(play_resource.sig_type == 1)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, DMX_DVB_TSI_IN_PORT);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, DMX_DVB_TSI_IN_PORT);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, DMX_DVB_TSI_IN_PORT);
        }
        else if(play_resource.sig_type == 2)
        {
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_2);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_2);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_2);
        }
        else if(play_resource.sig_type == 3)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_3);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_3);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_3);
#else
            ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+2, MT_UNF_DMX_PORT_TSI_1);
            ret |= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_REC+3, MT_UNF_DMX_PORT_TSI_1);
#endif

        }

    }
#endif
    (void)MT_PVRTimeshiftCmdTask(g_stTimeshiftRunInfo.hAvPlay, g_stTimeshiftRunInfo.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_stTimeshiftRunInfo.bIsPlayStop = MT_TRUE;


    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn1)
    {
        (void)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn1);
    }
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn2)
    {
        (void)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn2);
    }
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn3)
    {
        (void)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn3);
    }
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.recChn4)
    {
        (void)MTADP_PVR_RecStop(g_stTimeshiftRunInfo.recChn4);
    }
    if(MT_INVALID_HANDLE != g_stTimeshiftRunInfo.playChn)
    {
        (void)MTADP_PVR_StopPlayBack(g_stTimeshiftRunInfo.playChn);
    }

    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_LIVE);
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC+2);
    (MT_VOID)MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_REC+3);
    play_resource.rec_status = MT_FALSE;

#ifndef MT_SAMPLE_APP
    ret = MT_PVRTimeshiftStopplay(g_stTimeshiftRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TIMESHIFT_ERR_PRINT(" MT_PVRTimeshiftStopplay   failed.\n");
    }
    SAMPLE_TIMESHIFT_INFO_PRINT("stop to play\n");
#endif
ERR14:
    (void)pthread_join(g_stTimeshiftRunInfo.statusThd, MT_NULL);

ERR13:
    (MT_VOID)MTADP_PVR_UnRegisterCallBacks();
ERR12:
    (MT_VOID)MT_UNF_PVR_PlayDeInit();

ERR11:
    (MT_VOID)MT_UNF_PVR_RecDeInit();

ERR10:

#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_PVRTimeshiftAvplayDeInit(g_stTimeshiftRunInfo.hAvPlay, g_stTimeshiftRunInfo.hWin, g_stTimeshiftRunInfo.hSoundTrack);

ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stTimeshiftRunInfo.pProgTbl);

ERR8:
    (MT_VOID)MTADP_Search_DeInit();

ERR7:
    (MT_VOID)MT_PVRTimeshiftDmxDeInit();

ERR6:
    (MT_VOID)MTADP_VO_DeInit();

ERR5:
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:

    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR2:
    (MT_VOID)MTADP_Fe_DeInit(g_stTimeshiftRunInfo.sInputParam.tuner_id);

ERR1:

    (MT_VOID)mt_sys_deinit();
#endif

    g_bTaskQuit = MT_TRUE;
    memset(&g_stTimeshiftRunInfo, 0xff, sizeof(g_stTimeshiftRunInfo));
    play_resource.demux_use = MT_FALSE;

    return MT_SUCCESS;
}

