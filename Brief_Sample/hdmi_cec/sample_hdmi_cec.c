/*********************************************************************************************/
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
#include "mt_adp_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
#include "mt_unf_ir.h"
#include "mt_unf_pm.h"
#include "mt_cmdline.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_CEC_DEBUG

#define MT_HDMI_CEC_PRINT   printf
#else

#define MT_HDMI_CEC_PRINT

#endif

#define SAMPLE_CEC_FUNCTION_ENTER()     MT_HDMI_CEC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_CEC_FUNCTION_EXIT()      MT_HDMI_CEC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_CEC_FATAL_PRINT(fmt...)          MT_HDMI_CEC_PRINT(" [FATAL] " fmt)
#define SAMPLE_CEC_ERR_PRINT(fmt...)            MT_HDMI_CEC_PRINT(" [ERROR] " fmt)
#define SAMPLE_CEC_WARN_PRINT(fmt...)           MT_HDMI_CEC_PRINT(" [WARN] "  fmt)
#define SAMPLE_CEC_INFO_PRINT(fmt...)           MT_HDMI_CEC_PRINT(" [INFO] "  fmt)
#define SAMPLE_CEC_DBG_PRINT(fmt...)            MT_HDMI_CEC_PRINT(" [DEBUG] " fmt)

#define SAMPLE_CEC_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0 0
#define TUNER_ID_0 (0)

#define INVALID_TSPID (0x1fff)

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define KEY_STANDBY_OLD 0xf50a7f80
#define KEY_VOLINCREASE_OLD 0xa7587f80
#define KEY_VOLDECREASE_OLD 0xa45b7f80
#define KEY_VOLMUTE_OLD 0xf40b7f80

#define KEY_STANDBY_NEW 0xb748fd01
#define KEY_VOLINCREASE_NEW 0xe01ffd01
#define KEY_VOLDECREASE_NEW 0xbb44fd01
#define KEY_VOLMUTE_NEW 0xf609fd01

#define KEY_VOLINCREASE_NEW_2 0xe01ffd01
#define KEY_VOLDECREASE_NEW_2 0xbb44fd01

#elif defined CONFIG_MT_CHIP_SYMPHONY6
#define KEY_STANDBY_OLD 0x800a
#define KEY_VOLINCREASE_OLD 0x8058
#define KEY_VOLDECREASE_OLD 0x805b
#define KEY_VOLMUTE_OLD 0x800b

#define KEY_STANDBY_NEW 0x1fd48
#define KEY_VOLINCREASE_NEW 0x1fd1f
#define KEY_VOLDECREASE_NEW 0x1fd44
#define KEY_VOLMUTE_NEW 0x1fd09

#define KEY_VOLINCREASE_NEW_2 0x1fd0a
#define KEY_VOLDECREASE_NEW_2 0x1fd11

#endif
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
    pthread_t          key;
    PMT_COMPACT_TBL  *pProgTbl;
} MT_HdmiCec_RUN_INFO;

/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_HdmiCec_RUN_INFO    g_stHdmicecRunInfo;

extern cec_config_t g_cec_cfg;
extern MT_BOOL g_str_enable;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_HdmiCecMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

#ifndef MT_SAMPLE_APP
static mt_s32 MT_HdmiCecCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_CEC_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_CEC_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_CEC_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_HdmiCecCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_CEC_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_CEC_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}



/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_HdmiCecDmxInit(mt_input_para_t *pInputParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_CEC_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CEC_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (mt_void) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == pInputParam->sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
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
            SAMPLE_CEC_ERR_PRINT("failed to mt_sys_get_version\n");
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
            SAMPLE_CEC_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    SAMPLE_CEC_FUNCTION_EXIT();


    return MT_SUCCESS;
}
#endif

/*
 @brief DmxDeinit and detachTSPort
 @return MT_VOID
*/
static MT_VOID MT_HdmiCecDmxDeInit(MT_VOID)
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
static mt_s32 MT_HdmiCecAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = MT_INVALID_HANDLE;
    mt_handle   hWin = MT_INVALID_HANDLE;
    mt_handle   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_CEC_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_CEC_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_CEC_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
static void  MT_HdmiCecAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_CEC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_CEC_ERR_PRINT("=====hWin is INVALID_HANDLE ======\n");
        return ;
    }
    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_CEC_ERR_PRINT("=====hSoundTrack is INVALID_HANDLE ======\n");
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

#ifndef MT_SAMPLE_APP

/*
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_HdmiCecAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
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
        SAMPLE_CEC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == p_ProgInfo)
    {
        SAMPLE_CEC_ERR_PRINT("p_ProgInfo is NULL!\n");
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

    SAMPLE_CEC_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_CEC_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_CEC_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_CEC_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_CEC_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
            SAMPLE_CEC_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_CEC_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_CEC_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
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
static mt_s32 MT_HdmiCecInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_file_param_t *pstParam = (source_file_param_t *)(args);

    SAMPLE_CEC_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_CEC_ERR_PRINT( "\nfile %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_CEC_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        (mt_void)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT( "failed to MT_UNF_DMX_GetTSBuffer  ret= %x \n", ret);
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_CEC_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_CEC_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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

static void MT_HdmiCecPrintDvbcHelp(char *name)
{
    MT_HDMI_CEC_PRINT("\nDvbc Usage:\n");
    MT_HDMI_CEC_PRINT(" %s -c freq symrate qam\n", name);
    MT_HDMI_CEC_PRINT("    freq : \n");
    MT_HDMI_CEC_PRINT("    symrate: \n");
    MT_HDMI_CEC_PRINT("    qam: \n");
    MT_HDMI_CEC_PRINT("example:\n");
    MT_HDMI_CEC_PRINT("    %s -c 654 6875 64\n", name);
}

static void MT_HdmiCecPrintDvbsHelp(char *name)
{
    MT_HDMI_CEC_PRINT("\nDvbs Usage:\n");
    MT_HDMI_CEC_PRINT(" %s -s freq symrate onoff_22k polarization port_type\n", name);
    MT_HDMI_CEC_PRINT("    freq : \n");
    MT_HDMI_CEC_PRINT("    symrate: \n");
    MT_HDMI_CEC_PRINT("    onoff_22k: \n");
    MT_HDMI_CEC_PRINT("example:\n");
    MT_HDMI_CEC_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}

static void MT_HdmiCecPrintFileHelp(char *name)
{
    MT_HDMI_CEC_PRINT("\nDvbs Usage:\n");
    MT_HDMI_CEC_PRINT(" %s -f file\n", name);
    MT_HDMI_CEC_PRINT("example:\n");
    MT_HDMI_CEC_PRINT("    %s -f ./xxx.ts\n", name);
}
#endif

static void MT_HdmiCecPrint_help(char *name)
{
    MT_HDMI_CEC_PRINT("Lack of parameters\n");
    MT_HDMI_CEC_PRINT("\nUsage:\n");
    MT_HDMI_CEC_PRINT(" %s\n", name);
#ifndef MT_SAMPLE_APP
    MT_HDMI_CEC_PRINT("    -f: path of the subtitle stream file\n");
    MT_HDMI_CEC_PRINT("    -c: DVBC locks frequency\n");
    MT_HDMI_CEC_PRINT("    -s: DVBS locks frequency\n");
    MT_HDMI_CEC_PRINT("example:\n");
    MT_HDMI_CEC_PRINT("    %s -f ./sub.ts\n", name);
    MT_HDMI_CEC_PRINT("    %s -c 654 6875 64\n", name);
    MT_HDMI_CEC_PRINT("    %s -s 3840 27500 1 0 0\n", name);
#endif
    MT_HDMI_CEC_PRINT("    %s -q  <exit> \n", name);
}

static void MT_HdmiCecStandbyPrint_help(void)
{
    MT_HDMI_CEC_PRINT("===Choice standby mode===\n");
    MT_HDMI_CEC_PRINT("1. Fake standby\n");
    MT_HDMI_CEC_PRINT("2. pmu standby\n");
    MT_HDMI_CEC_PRINT("3. str standby\n");
    MT_HDMI_CEC_PRINT("=========================\n");
    MT_HDMI_CEC_PRINT("Set standby mode:");
}

/*
@brief stop to play
@param[in] avplay, Player handle
@return MT_SUCCESS
*/
static MT_S32 MT_HdmiCecStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if(MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_CEC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return MT_FAILURE;
    }
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    return MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}

// not used
#if 0
/*
@brief Assemble the cec command and send it
@param[in] destAddr, Destination address
@param[in] u8Opcode, Operation code
@param[in] data, Data content
@param[in] Datalength, Data length
@return::MT_SUCCESS
*/
static mt_u32 MT_HdmiCecsetcmd(mt_u8 destAddr, mt_u8 u8Opcode, mt_u8 *data, mt_u8 Datalength)
{
    mt_u32 RetError = MT_SUCCESS;
    MT_UNF_HDMI_CEC_CMD_S CECCmd = { 0 };

    MT_USLEEP(300 * 1000);
    memset(&CECCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
    CECCmd.enSrcAdd = MT_UNF_CEC_LOGICALADD_TUNER_1;
    CECCmd.enDstAdd = MT_UNF_CEC_LOGICALADD_TV;
    CECCmd.u8Opcode = u8Opcode;
    CECCmd.unOperand.stRawData.u8Length = Datalength;
    memcpy(&(CECCmd.unOperand.stRawData.u8Data), data, Datalength);

    RetError = MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &CECCmd);
    if(MT_SUCCESS != RetError)
    {
        SAMPLE_CEC_ERR_PRINT("MT_UNF_HDMI_SetCECCommand failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
    }
    return RetError;
}
#endif

static MT_VOID MT_HdmiCecPrintMenu(MT_U32 prog_num)
{
      SAMPLE_CEC_PRINT("\n");
#ifndef MT_SAMPLE_APP
      SAMPLE_CEC_PRINT(" '1-%d': Select a program \n", prog_num);
#endif

#ifdef MT_SAMPLE_APP
      SAMPLE_CEC_PRINT("    b: background run \n");
#endif
      SAMPLE_CEC_PRINT("    p: choice standby mode \n");
      SAMPLE_CEC_PRINT("    h: help \n");
      SAMPLE_CEC_PRINT("    q: quit \n");
      SAMPLE_CEC_PRINT("CEC>> ");
}

static mt_s32 MT_HdmiCecget_key(void)
{
    mt_s32  ret = MT_SUCCESS;
    MT_UNF_KEY_STATUS_E press_status = { 0 };
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };
    ir_wavefilter_config_s wavefiler = { 0 };
    MT_UNF_HDMI_CEC_CMD_S stCmd = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_U8 protocol = IRDA_NEC;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_U8 protocol = RC_PROTO_NEC_;
    irda_protocol_t tmpx[]={RC_PROTO_NEC_,RC_PROTO_NECX_,RC_PROTO_RCMM32_,RC_PROTO_RC5_};

#endif

    SAMPLE_CEC_INFO_PRINT("Please use the remote control button to control standby wake up, volume increase or decrease, mute\n");
    while(g_bTaskQuit != MT_TRUE)
    {
        if(g_str_enable == MT_TRUE)
        {
            usleep(5000);
            continue;
        }

        wavefiler.irda_wfilt_channel = 4;
        wavefiler.irda_protocol = protocol;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
        wavefiler.irda_protocol = IRDA_NEC;
        wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x7F800AF5;
        wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0xfd0148b7;
        wavefiler.irda_wfilt_channel_cfg[0].protocol = IRDA_NEC;
        wavefiler.irda_wfilt_channel_cfg[1].protocol = IRDA_NEC;
        (MT_VOID)MT_UNF_IR_SetWaveFilter(wavefiler);
        (MT_VOID)MT_UNF_IR_SetKeycode(0);
        (MT_VOID)MT_UNF_IR_SetUsercode(0);
#elif defined CONFIG_MT_CHIP_SYMPHONY6

        MT_UNF_IR_Config_Protocols_ByType(tmpx,4,1);
        wavefiler.irda_wfilt_channel_cfg[1].protocol = RC_PROTO_NECX_;
        wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0x1fd48;

        wavefiler.irda_wfilt_channel_cfg[2].protocol = RC_PROTO_RCMM32_;
        wavefiler.irda_wfilt_channel_cfg[2].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[2].wfilt_code = 0x29c0260c;

        wavefiler.irda_wfilt_channel_cfg[0].protocol = RC_PROTO_NEC_;
        wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x800a;
        (MT_VOID)MT_UNF_IR_SetWaveFilter(&wavefiler);
#endif

        ret = MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId, name, sizeof(name),3000);
        if(MT_SUCCESS == ret)
        {
            if(u64KeyId == KEY_STANDBY_OLD || u64KeyId == KEY_STANDBY_NEW)
            {
                mt_s32 standby_flag;
                MTADP_HDMI_GetStandbyStatus(&standby_flag);
                if (standby_flag != MT_TRUE)
                {
                    SAMPLE_CEC_INFO_PRINT("standby! \n");
                    //(void)MT_HdmiCecsetcmd(0x0, CEC_OPCODE_STANDBY, 0, 0x0);
                    memset(&stCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
                    stCmd.u8Opcode = CEC_OPCODE_STANDBY;
					stCmd.enDstAdd = 0xf;
                    ret = MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &stCmd);

                    MTADP_HDMI_SetStandbyStatus(MT_TRUE);
                }
                else
                {
                    SAMPLE_CEC_INFO_PRINT("wakeup! \n");
                    memset(&stCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
                    stCmd.u8Opcode = CEC_OPCODE_IMAGE_VIEW_ON;
                    ret = MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &stCmd);

                    MTADP_HDMI_SetStandbyStatus(MT_FALSE);

					//enable cvbs
					//MT_UNF_DISP_SetSdVideoEnable(MT_TRUE);
					//MT_UNF_DISP_SetHdVideoEnable(MT_TRUE);
					//MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_TRUE);
					//MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_TRUE);
					//MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_TRUE);
					//MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_TRUE);
                }
                continue;
            }

            else if((u64KeyId == KEY_VOLINCREASE_OLD) || (u64KeyId == KEY_VOLINCREASE_NEW) || (u64KeyId == KEY_VOLINCREASE_NEW_2))
            {
                SAMPLE_CEC_INFO_PRINT("VOLINCREASE \n");
                memset(&stCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
                stCmd.u8Opcode = CEC_OPCODE_USER_CONTROL_PRESSED;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
                stCmd.unOperand.stUIOpcode = MT_UNF_CEC_UICMD_VOLUME_UP;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
                stCmd.unOperand.stRawData.u8Data[0] = MT_UNF_CEC_UICMD_VOLUME_UP;
                stCmd.unOperand.stRawData.u8Length = 1;
#endif
                (void)MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &stCmd);
                continue;
            }
            else if((u64KeyId == KEY_VOLDECREASE_OLD) || (u64KeyId == KEY_VOLDECREASE_NEW) || (u64KeyId == KEY_VOLDECREASE_NEW_2))
            {
                SAMPLE_CEC_INFO_PRINT("VOLDECREASE \n");
                memset(&stCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
                stCmd.u8Opcode = CEC_OPCODE_USER_CONTROL_PRESSED;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
                stCmd.unOperand.stUIOpcode = MT_UNF_CEC_UICMD_VOLUME_DOWN;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
                stCmd.unOperand.stRawData.u8Data[0] = MT_UNF_CEC_UICMD_VOLUME_DOWN;
                stCmd.unOperand.stRawData.u8Length = 1;
#endif
                (void)MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &stCmd);
                continue;
            }
            else if((u64KeyId == KEY_VOLMUTE_OLD) || (u64KeyId == KEY_VOLMUTE_NEW))
            {
                //SAMPLE_CEC_INFO_PRINT("VOLMUTE \n");
                stCmd.u8Opcode = CEC_OPCODE_USER_CONTROL_PRESSED;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
                stCmd.unOperand.stUIOpcode = MT_UNF_CEC_UICMD_MUTE;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
                stCmd.unOperand.stRawData.u8Data[0] = MT_UNF_CEC_UICMD_MUTE;
                stCmd.unOperand.stRawData.u8Length = 1;
#endif
                (void)MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &stCmd);
                continue;
            }
        }
    }
    return MT_SUCCESS;
}


/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@param[in] ppProgTable,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static void MT_HdmiCecCmdTask( mt_handle hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
#ifndef MT_SAMPLE_APP
    mt_s32   ret = MT_SUCCESS;
    mt_u32   u32ProgNum = 0;
    PMT_COMPACT_PROG *stCurrentProgInfo = { 0 };
#endif
    MT_CHAR  *fgetret = NULL;
    MT_CHAR  inputCmd[32] = { 0 };
    MT_U32   standby_mode = 0;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_CEC_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ;
    }

    while (1)
    {
        (void)MT_HdmiCecPrintMenu(pProgTbl->prog_num);
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if('q' == inputCmd[0])
        {
            SAMPLE_CEC_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_CEC_INFO_PRINT("CEC in back!\n");
            break;
        }
#endif

#ifndef MT_SAMPLE_APP
        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                stCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                ret = MT_HdmiCecStopplay(hAvplay);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_CEC_ERR_PRINT(" MT_HdmiCecStopplay failed.\n");
                }
                SAMPLE_CEC_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);

                ret  = MT_HdmiCecAVPlay_Start(hAvplay, stCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_CEC_ERR_PRINT(" SwitchProg failed.\n");
                }
            }
            else
            {
                SAMPLE_CEC_INFO_PRINT("prog_num the biggest is %d\n", pProgTbl->prog_num);
                SAMPLE_CEC_INFO_PRINT("q: quit\n");
                continue;
            }
        }
#endif

         else if ('p' == inputCmd[0])
         {
             MT_HdmiCecStandbyPrint_help();
             scanf("%d", &standby_mode);
             getchar();
             SAMPLE_CEC_INFO_PRINT("standby_mode: %d \n", standby_mode);
             standby_mode = standby_mode % MT_HDMI_CEC_STANDBY_BUTT;
             MTADP_HDMI_Set_Standby_mode(standby_mode);
             continue;
         }
         else if('h' == inputCmd[0])
        {
            SAMPLE_CEC_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

static MT_VOID MT_HdmiCecExit(void)
{
    g_bTaskQuit = MT_TRUE;
    (void)pthread_join(g_stHdmicecRunInfo.key, NULL);

    (MT_VOID)MT_HdmiCecStopplay(g_stHdmicecRunInfo.hAvPlay);
    (MT_VOID)MT_HdmiCecAvplayDeInit(g_stHdmicecRunInfo.hAvPlay, g_stHdmicecRunInfo.hWin, g_stHdmicecRunInfo.hSoundTrack);
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stHdmicecRunInfo.pProgTbl);
    (MT_VOID)MTADP_Search_DeInit();
    if(MT_INPUT_SIG_TYPE_FILE == g_stHdmicecRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)pthread_join(g_stHdmicecRunInfo.htsThd, NULL);
    }
    (MT_VOID)MT_HdmiCecDmxDeInit();
    (MT_VOID)MTADP_VO_DeInit();
    (MT_VOID)MTADP_Snd_DeInit();
    if(MT_INPUT_SIG_TYPE_FILE != g_stHdmicecRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    }
    g_cec_cfg.cec_enable = 0;
    memset(&g_stHdmicecRunInfo, 0xff, sizeof(g_stHdmicecRunInfo));
}



/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_HdmiCecParase_args(int argc, char *argv[], mt_input_para_t *pInputParam)
{
        int opt = 0;
        while((opt = MTADP_Getopt(argc, argv, "h?Hf:c:s:q")) != -1)
        {
            switch(opt)
            {
                case 'h':
                case '?':
                case 'H':
                    (void)MT_HdmiCecPrint_help(argv[0]);
                return MT_FAILURE;

                case 'q':
                    if(g_bTaskQuit == MT_FALSE)
                    {
                        (MT_VOID)MT_HdmiCecExit();
                    }
                    return MT_TASK_EXIT;

#ifndef MT_SAMPLE_APP
                case 'f':
                    if(argc < 3)
                    {
                        (void)MT_HdmiCecPrintFileHelp(argv[0]);
                        return MT_FAILURE;
                    }
                    pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                    MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

                case 's':
                    if(argc < 6)
                    {
                        (void)MT_HdmiCecPrintDvbsHelp(argv[0]);
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
                        (void)MT_HdmiCecPrintDvbcHelp(argv[0]);
                        return MT_FAILURE;
                    }
                    pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                    pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                    pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                    pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
#endif

                default:
                    (void)MT_HdmiCecPrint_help(argv[0]);
                    return MT_FAILURE;
            }
        }


        return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_HdmiCecMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;
#ifndef MT_SAMPLE_APP
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#endif
    MT_UNF_KEY_STATUS_E press_status = MT_UNF_KEY_STATUS_BUTT;
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };


#ifndef MT_SAMPLE_APP
    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_HdmiCecPrint_help(argv[0]);
        return MT_SUCCESS;
    }
#endif

    ret = MT_HdmiCecParase_args(argc, argv, &g_stHdmicecRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_CEC_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_CEC_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        g_bTaskQuit = MT_FALSE;
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return ret;
        }
        if(MT_INPUT_SIG_TYPE_FILE != g_stHdmicecRunInfo.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_CEC_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            if(MT_INPUT_SIG_TYPE_CAB == g_stHdmicecRunInfo.sInputParam.sig_type)
            {     //dvbc
                ret = MT_HdmiCecCheckDvbcParam(&g_stHdmicecRunInfo.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_CEC_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        g_stHdmicecRunInfo.sInputParam.input_param.cab.freq,
                                        g_stHdmicecRunInfo.sInputParam.input_param.cab.sym_rate,
                                        g_stHdmicecRunInfo.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == g_stHdmicecRunInfo.sInputParam.sig_type)
            {
                ret = MT_HdmiCecCheckDvbsParam(&g_stHdmicecRunInfo.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_CEC_ERR_PRINT("Input sat parameter error!\n");
                    goto ERR2;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        g_stHdmicecRunInfo.sInputParam.input_param.sat.freq,
                                        g_stHdmicecRunInfo.sInputParam.input_param.sat.sym_rate,
                                        g_stHdmicecRunInfo.sInputParam.input_param.sat.onoff_22k,
                                        g_stHdmicecRunInfo.sInputParam.input_param.sat.polarization,
                                        g_stHdmicecRunInfo.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_CEC_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR2;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MTADP_Disp_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            goto ERR3;
        }

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR5;
        }

        ret = MT_HdmiCecDmxInit(&g_stHdmicecRunInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT( "failed to StartDmx\n");
            goto ERR6;
        }

        if(MT_INPUT_SIG_TYPE_FILE == g_stHdmicecRunInfo.sInputParam.sig_type)
        {
            ret = pthread_create(&g_stHdmicecRunInfo.htsThd, NULL, (void * (*)(void *))MT_HdmiCecInjectTsTask, &g_stHdmicecRunInfo.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_CEC_ERR_PRINT("failed to pthread_create stInjectTSThread\n");
                goto ERR7;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR7;
            }
        }

        (void)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stHdmicecRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR9;
        }

        ret = MT_HdmiCecAVplayInit(&g_stHdmicecRunInfo.hAvPlay, &g_stHdmicecRunInfo.hWin, &g_stHdmicecRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to MT_HdmiCecAVplayInit\n");
            goto ERR10;
        }

        /* Play the first program on the program list*/
        pstCurrentProgInfo = g_stHdmicecRunInfo.pProgTbl->proginfo;
        ret = MT_HdmiCecAVPlay_Start(g_stHdmicecRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to MT_HdmiCecAVPlay_Start\n");
            goto ERR11;
        }
#endif

        ret = MT_UNF_HDMI_CEC_Enable(MT_UNF_HDMI_ID_0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MT_UNF_HDMI_CEC_Enable failed ret = %d\n", ret);
#ifndef MT_SAMPLE_APP
            goto ERR11;
#else
            g_bTaskQuit = MT_TRUE;
            return ret;
#endif
        }

        g_cec_cfg.cec_enable = 1;

        ret = MT_UNF_IR_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MT_UNF_IR_Init failed ret = %d\n", ret);
            goto ERR12;
        }

        ret = MT_UNF_IR_SetRepKeyTimeoutAttr(500);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MT_UNF_IR_SetRepKeyTimeoutAttr failed ret = %d\n", ret);
            goto ERR13;
        }

        ret = MT_UNF_IR_EnableKeyUp(MT_FALSE);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MT_UNF_IR_EnableKeyUp failed ret = %d\n", ret);
            goto ERR13;
        }

        ret = MT_UNF_IR_EnableRepKey(MT_FALSE);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MT_UNF_IR_EnableRepKey failed ret = %d\n", ret);
            goto ERR13;
        }

        ret = MT_UNF_IR_SetFetchMode(0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MT_UNF_IR_SetFetchMode failed ret = %d\n", ret);
            goto ERR13;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        ret = MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        ret = MT_UNF_IR_Enable(MT_TRUE, RC_PROTO_NEC_);
#endif
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("MT_UNF_IR_Enable failed ret = %d\n", ret);
            goto ERR13;
        }
        while(MT_SUCCESS == MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId,name, sizeof(name),1000)) //Clear IR DataCache
        {
            MT_USLEEP(5000);
        }
        ret = pthread_create(&g_stHdmicecRunInfo.key, NULL, (void * (*)(void *))MT_HdmiCecget_key, NULL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CEC_ERR_PRINT("failed to pthread_create\n");
            goto ERR13;
        }
    }

#ifdef MT_SAMPLE_APP
            g_stHdmicecRunInfo.hAvPlay = avplayHandle.hAvPlay;
#endif

    (mt_void)MT_HdmiCecCmdTask(g_stHdmicecRunInfo.hAvPlay, g_stHdmicecRunInfo.pProgTbl);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifndef MT_SAMPLE_APP
    ret = MT_HdmiCecStopplay(g_stHdmicecRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CEC_ERR_PRINT("MT_HdmiCecStopplay  failed.\n");
        goto ERR14;
    }
    SAMPLE_CEC_INFO_PRINT("stop to play\n");
#endif

#ifndef MT_SAMPLE_APP
ERR14:
#endif
    g_bTaskQuit = MT_TRUE;
    (void)pthread_join(g_stHdmicecRunInfo.key, NULL);

ERR13:
    (MT_VOID)MT_UNF_IR_DeInit();

ERR12:
    (MT_VOID)MT_UNF_HDMI_CEC_Disable(MT_UNF_HDMI_ID_0);

#ifndef MT_SAMPLE_APP
ERR11:
    (MT_VOID)MT_HdmiCecAvplayDeInit(g_stHdmicecRunInfo.hAvPlay, g_stHdmicecRunInfo.hWin, g_stHdmicecRunInfo.hSoundTrack);

ERR10:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stHdmicecRunInfo.pProgTbl);

ERR9:
    (MT_VOID)MTADP_Search_DeInit();
ERR8:
    if(MT_INPUT_SIG_TYPE_FILE == g_stHdmicecRunInfo.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        (void)pthread_join(g_stHdmicecRunInfo.htsThd, NULL);
    }
ERR7:
    (MT_VOID)MT_HdmiCecDmxDeInit();

ERR6:
    (MT_VOID)MTADP_VO_DeInit();

ERR5:
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:
    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR2:
    if(MT_INPUT_SIG_TYPE_FILE != g_stHdmicecRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif

    g_bTaskQuit = MT_TRUE;
    g_cec_cfg.cec_enable = 0;
    memset(&g_stHdmicecRunInfo, 0xff, sizeof(g_stHdmicecRunInfo));

    return MT_SUCCESS;
}

