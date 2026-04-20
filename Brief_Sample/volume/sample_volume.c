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
#ifdef MT_SAMPLE_VOLUME_DEBUG

#define MT_VOLUME_PRINT   printf
#else

#define MT_VOLUME_PRINT

#endif

#define SAMPLE_VOLUME_FUNCTION_ENTER()  MT_VOLUME_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_VOLUME_FUNCTION_EXIT()       MT_VOLUME_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_VOLUME_FATAL_PRINT(fmt...)       MT_VOLUME_PRINT(" [FATAL] " fmt)
#define SAMPLE_VOLUME_ERR_PRINT(fmt...)         MT_VOLUME_PRINT(" [ERROR] " fmt)
#define SAMPLE_VOLUME_WARN_PRINT(fmt...)        MT_VOLUME_PRINT(" [WARN] "  fmt)
#define SAMPLE_VOLUME_INFO_PRINT(fmt...)        MT_VOLUME_PRINT(" [INFO] "  fmt)
#define SAMPLE_VOLUME_DBG_PRINT(fmt...)         MT_VOLUME_PRINT(" [DEBUG] " fmt)

#define SAMPLE_VOLUME_PRINT   printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0 0
#define TUNER_ID_0 (0)
#define INVALID_TSPID (0x1fff)
#define DEFAULT_VOLUME        80

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
    mt_input_para_t sInputParam;
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    pthread_t          htsThd;
    PMT_COMPACT_TBL  *pProgTbl;

} MT_Volume_RUN_INFO;


/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;



static MT_Volume_RUN_INFO    g_stVolumeRunInfo;

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_VolumeMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

#ifndef MT_SAMPLE_APP
static mt_s32 MT_VolumeCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_VOLUME_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_VOLUME_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_VOLUME_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_VolumeCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_VOLUME_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_VOLUME_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_VolumeDmxInit(mt_input_para_t *pInputParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_VOLUME_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == pInputParam->sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            (mt_void) MT_UNF_DMX_DeInit();
            return MT_FAILURE;
        }
    }
    else
    {
         memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to mt_sys_get_version\n");
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

    SAMPLE_VOLUME_FUNCTION_EXIT();


    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_VOID MT_VolumeDmxDeInit(MT_VOID)
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
static mt_s32 MT_VolumeAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_VOLUME_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_VOLUME_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }

    if(NULL == p_hSoundTrack)
    {
        SAMPLE_VOLUME_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
static MT_VOID  MT_VolumeAVplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_VOLUME_ERR_PRINT("phAvplay is null.\n");

    }

    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_VOLUME_ERR_PRINT("phWin is null.\n");

    }

    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_VOLUME_ERR_PRINT("phSoundTrack is null.\n");

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
static mt_s32 MT_VolumeAVPlayStart(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
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
        SAMPLE_VOLUME_ERR_PRINT("p_ProgInfo is NULL!\n");
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

    SAMPLE_VOLUME_INFO_PRINT("vidpid = %x AudPid=%x \n", VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_VOLUME_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_VOLUME_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_VOLUME_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_VOLUME_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
            SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_VOLUME_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
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
static mt_s32 MT_VolumeInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    mt_input_file_para_t *pstParam = (mt_input_file_para_t *)(args);

    SAMPLE_VOLUME_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_VOLUME_ERR_PRINT( " file %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        fclose(pTsFile);
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_VOLUME_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
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
            SAMPLE_VOLUME_ERR_PRINT( "failed to MT_UNF_DMX_GetTSBuffer  ret= %x \n", ret);
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_VOLUME_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_VOLUME_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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

/*
@brief help
@return void
*/
static void MT_VolumePrinthelp(char *name)
{
#ifndef MT_SAMPLE_APP
    SAMPLE_VOLUME_PRINT("Lack of parameters\n");
    SAMPLE_VOLUME_PRINT("\nUsage:\n");
    SAMPLE_VOLUME_PRINT(" %s\n", name);
    SAMPLE_VOLUME_PRINT("    -f: path of the subtitle stream file\n");
    SAMPLE_VOLUME_PRINT("        -f path\n");
    SAMPLE_VOLUME_PRINT("    -c: DVBC locks frequency\n");
    SAMPLE_VOLUME_PRINT("        -c freq symrate qam\n");
    SAMPLE_VOLUME_PRINT("    -s: DVBS locks frequency\n");
    SAMPLE_VOLUME_PRINT("        -s freq symrate onoff_22k polar dvb_type");
    SAMPLE_VOLUME_PRINT("example:\n");
    SAMPLE_VOLUME_PRINT("    %s -f ./sub.ts\n", name);
    SAMPLE_VOLUME_PRINT("    %s -c 654 6875 64\n", name);
    SAMPLE_VOLUME_PRINT("    %s -s 3840 27500 1 0 0\n", name);
#else
    SAMPLE_VOLUME_PRINT("    %s -q  <exit> \n", name);
#endif
}

static void MT_VolumeMenuHelp(MT_U32 prog_num)
{
#ifndef MT_SAMPLE_APP
    SAMPLE_VOLUME_PRINT("\n 1 - %d : select the program \n", prog_num);
#endif
    SAMPLE_VOLUME_PRINT("     u : turn up volume \n");
    SAMPLE_VOLUME_PRINT("     d : turn down volume \n");
    SAMPLE_VOLUME_PRINT("     g : get volume \n");
    SAMPLE_VOLUME_PRINT("     m : mute/unmute \n");
    SAMPLE_VOLUME_PRINT("     s : set volume \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_VOLUME_PRINT("     b : background run \n");
#endif
    SAMPLE_VOLUME_PRINT("     h : help \n");
    SAMPLE_VOLUME_PRINT("     q : quit \n");
    SAMPLE_VOLUME_PRINT("Volume>> ");

}

#ifndef MT_SAMPLE_APP

/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
static void MT_VolumeStopplay(MT_HANDLE avplay)
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
static void MT_VolumeCmdTask(MT_HANDLE hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{

    MT_CHAR    inputCmd[32] = { 0 };
    MT_S32     volume = 0;
    MT_S32     Ret = SUCCESS;
    MT_BOOL    mute = MT_FALSE;
#ifndef MT_SAMPLE_APP
    MT_U32     u32ProgNum = 0;
    PMT_COMPACT_PROG *pstCurrentProgInfo = NULL;
#endif
    MT_UNF_SND_GAIN_ATTR_S stGainVolume = { 0 };



    while (1)
    {

        (MT_VOID)MT_VolumeMenuHelp(pProgTbl->prog_num);
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_VOLUME_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_VOLUME_INFO_PRINT("volume in back!\n");
            break;
        }
#else
        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                (void)MT_VolumeStopplay(hAvPlay);

                SAMPLE_VOLUME_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);

                Ret  = MT_VolumeAVPlayStart(hAvPlay, pstCurrentProgInfo);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_VOLUME_ERR_PRINT(" SwitchProg failed.\n");
                }
            }
            else
            {
                SAMPLE_VOLUME_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }
        }
#endif
        else if('s' == inputCmd[0])
        {
            Ret = MT_UNF_SND_GetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_GetVolume\n");
                continue;
            }
            volume = stGainVolume.s32Gain;
            SAMPLE_VOLUME_INFO_PRINT("Please enter the volume you want to set 0-100\n");
            scanf("%d",&volume);
            if(volume >= 0 && volume <= 100)
            {
                stGainVolume.s32Gain = volume;
                SAMPLE_VOLUME_INFO_PRINT(" Set volume to %d \n",  volume);
                MT_UNF_SND_SetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            }
            else
            {
                SAMPLE_VOLUME_ERR_PRINT("The input volume is out of range!!!!!!\n");
            }
        }

        /* turn up volume */
        else if('u' == inputCmd[0])
        {
            Ret = MT_UNF_SND_GetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_GetVolume\n");
                continue;
            }
            volume = stGainVolume.s32Gain;
            SAMPLE_VOLUME_INFO_PRINT("Turn up the volume.\n");

            volume += 5;
            if(100 < volume)
            {
                SAMPLE_VOLUME_INFO_PRINT("The volume has reached its maximum 100.\n");
                volume = 100;
            }

            SAMPLE_VOLUME_INFO_PRINT(" Set volume to %d \n",  volume);
            stGainVolume.s32Gain = volume;
            Ret = MT_UNF_SND_SetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            Ret |= MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_DAC0, MT_FALSE);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_SetVolume\n");
                continue;
            }
            mute = MT_FALSE;

        }
        /* turn down volume */
        else if('d' == inputCmd[0])
        {
            Ret = MT_UNF_SND_GetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_GetVolume\n");
                continue;
            }
            volume = stGainVolume.s32Gain;

            volume -= 5;
            if(volume < 0)
            {
                SAMPLE_VOLUME_INFO_PRINT("The volume has reached its minimum 0.\n");
                volume = 0;
            }

            SAMPLE_VOLUME_INFO_PRINT(" Set volume to %d \n",  volume);
            stGainVolume.s32Gain = volume;
            Ret = MT_UNF_SND_SetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            Ret |= MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_DAC0, MT_FALSE);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_SetVolume\n");
                continue;
            }
            mute = MT_FALSE;
        }
        /* mute/unmute */
        else if('m' == inputCmd[0])
        {
            Ret = MT_UNF_SND_GetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_ALL, &mute);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_GetMute\n");
                continue;
            }

            Ret = MT_UNF_SND_GetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_GetVolume\n");
                continue;
            }

            volume = stGainVolume.s32Gain;

            mute = (mute == MT_TRUE)? MT_FALSE : MT_TRUE;
            if(MT_FALSE == mute)
            {
                SAMPLE_VOLUME_INFO_PRINT(" Set UNMUTE \n");
                Ret = MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_ALL, MT_FALSE);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_SetMute\n");
                }
                stGainVolume.s32Gain = volume;
                Ret = MT_UNF_SND_SetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, &stGainVolume);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_SetVolume\n");
                }
            }
            else
            {
                SAMPLE_VOLUME_INFO_PRINT(" Set MUTE \n");
                Ret = MT_UNF_SND_SetMute(MT_UNF_SND_0,  MT_UNF_SND_OUTPUTPORT_ALL, MT_TRUE);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_SetMute\n");
                }
            }
        }
        else if('g' == inputCmd[0])
        {
            if(MT_TRUE == mute)
            {
                SAMPLE_VOLUME_INFO_PRINT("Now Volume is Mute \n");
            }
            else
            {
                memset(&stGainVolume, 0, sizeof(MT_UNF_SND_GAIN_ATTR_S));
                Ret = MT_UNF_SND_GetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_GetVolume\n");
                }

                SAMPLE_VOLUME_INFO_PRINT("Now play volume : %d \n", stGainVolume.s32Gain);
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_VOLUME_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

static MT_VOID MT_VolumeExit(void)
{
#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_VolumeStopplay(g_stVolumeRunInfo.hAvPlay);

    (MT_VOID)MT_VolumeAVplayDeInit(g_stVolumeRunInfo.hAvPlay, g_stVolumeRunInfo.hWin, g_stVolumeRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_stVolumeRunInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == g_stVolumeRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(g_stVolumeRunInfo.htsThd, NULL);
    }

    (MT_VOID)MT_VolumeDmxDeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != g_stVolumeRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }
#endif
    memset(&g_stVolumeRunInfo, 0xff, sizeof(g_stVolumeRunInfo));
    g_bTaskQuit = MT_TRUE;
}

/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_VolumeParaseargs(int argc, char *argv[], mt_input_para_t *pInputParam)
{
    int opt = 0;

#ifndef MT_SAMPLE_APP
    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_VolumePrinthelp(argv[0]);
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
                (void)MT_VolumePrinthelp(argv[0]);
            return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_VolumeExit();
                }
                return MT_TASK_EXIT;
#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc < 3)
                {
                    (void)MT_VolumePrinthelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                return MT_SUCCESS;

            case 's':
                if(argc < 6)
                {
                    (void)MT_VolumePrinthelp(argv[0]);
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
                    (void)MT_VolumePrinthelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
#endif
            default:
                (void)MT_VolumePrinthelp(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_VolumeMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;
#ifndef MT_SAMPLE_APP
    MT_UNF_SND_GAIN_ATTR_S stGainVolume = {0};
    PMT_COMPACT_PROG *p_stCurrentProgInfo = { 0 };
#endif


    ret = MT_VolumeParaseargs(argc, argv, &g_stVolumeRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_VOLUME_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_VOLUME_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
    if(g_bTaskQuit == MT_TRUE)
    {
        g_bTaskQuit = MT_FALSE;
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }


        if(MT_INPUT_SIG_TYPE_FILE != g_stVolumeRunInfo.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            if (MT_INPUT_SIG_TYPE_CAB == g_stVolumeRunInfo.sInputParam.sig_type)
            {     //dvbc
                ret = MT_VolumeCheckDvbcParam(&g_stVolumeRunInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_VOLUME_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        g_stVolumeRunInfo.sInputParam.input_param.cab.freq,
                                        g_stVolumeRunInfo.sInputParam.input_param.cab.sym_rate,
                                        g_stVolumeRunInfo.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_stVolumeRunInfo.sInputParam.sig_type)
            {
                ret = MT_VolumeCheckDvbsParam(&g_stVolumeRunInfo.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_VOLUME_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        g_stVolumeRunInfo.sInputParam.input_param.sat.freq,
                                        g_stVolumeRunInfo.sInputParam.input_param.sat.sym_rate,
                                        g_stVolumeRunInfo.sInputParam.input_param.sat.onoff_22k,
                                        g_stVolumeRunInfo.sInputParam.input_param.sat.polarization,
                                        g_stVolumeRunInfo.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_VolumeDmxInit(&g_stVolumeRunInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT( "failed to StartDmx\n");
            goto ERR6;
        }



        if(MT_INPUT_SIG_TYPE_FILE == g_stVolumeRunInfo.sInputParam.sig_type)
        {
            ret = pthread_create(&g_stVolumeRunInfo.htsThd, NULL, (void * (*)(void *))MT_VolumeInjectTsTask, &g_stVolumeRunInfo.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_VOLUME_ERR_PRINT("failed to pthread_create\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }

        (void)MTADP_Search_Init();

        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stVolumeRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR9;
        }

        (MT_VOID)DVB_ListProg();

        ret = MT_VolumeAVplayInit(&g_stVolumeRunInfo.hAvPlay, &g_stVolumeRunInfo.hWin, &g_stVolumeRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MT_VolumeAVplayInit\n");
            goto ERR10;
        }

        memset(&stGainVolume, 0, sizeof(MT_UNF_SND_GAIN_ATTR_S));
        stGainVolume.s32Gain = DEFAULT_VOLUME;
        ret = MT_UNF_SND_SetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MT_UNF_SND_SetVolume\n");
            goto ERR11;
        }

        /* Play the first program on the program list*/
        p_stCurrentProgInfo = g_stVolumeRunInfo.pProgTbl->proginfo;

        ret = MT_VolumeAVPlayStart(g_stVolumeRunInfo.hAvPlay, p_stCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_VOLUME_ERR_PRINT("failed to MT_VolumeAVPlayStart\n");
            goto ERR11;
        }
#endif
    }

    (void) MT_VolumeCmdTask(g_stVolumeRunInfo.hAvPlay, g_stVolumeRunInfo.pProgTbl);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_VolumeStopplay(g_stVolumeRunInfo.hAvPlay);
    SAMPLE_VOLUME_INFO_PRINT("stop to play\n");

ERR11:
    (MT_VOID)MT_VolumeAVplayDeInit(g_stVolumeRunInfo.hAvPlay, g_stVolumeRunInfo.hWin, g_stVolumeRunInfo.hSoundTrack);
ERR10:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stVolumeRunInfo.pProgTbl);
ERR9:
    (MT_VOID)MTADP_Search_DeInit();
ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == g_stVolumeRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(g_stVolumeRunInfo.htsThd, NULL);
    }
ERR7:
    (MT_VOID)MT_VolumeDmxDeInit();
ERR6:
    (MT_VOID)MTADP_VO_DeInit();
ERR5:
    (MT_VOID)MTADP_Snd_DeInit();
ERR4:
    (MT_VOID)MTADP_Disp_DeInit();
ERR3:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR2:
    if(MT_INPUT_SIG_TYPE_FILE != g_stVolumeRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_stVolumeRunInfo, 0xff, sizeof(g_stVolumeRunInfo));
    return MT_SUCCESS;
}
