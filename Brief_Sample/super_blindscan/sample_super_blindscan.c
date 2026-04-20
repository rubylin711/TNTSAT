/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
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
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_SUPERBLINDSCAN_DEBUG

#define MT_SUPERBLINDSCAN_PRINT   printf
#else

#define MT_SUPERBLINDSCAN_PRINT

#endif

#define SAMPLE_SUPERBLINDSCAN_FUNCTION_ENTER()      MT_SUPERBLINDSCAN_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SUPERBLINDSCAN_FUNCTION_EXIT()       MT_SUPERBLINDSCAN_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_SUPERBLINDSCAN_FATAL_PRINT(fmt...)       MT_SUPERBLINDSCAN_PRINT(" [FATAL] " fmt)
#define SAMPLE_SUPERBLINDSCAN_ERR_PRINT(fmt...)         MT_SUPERBLINDSCAN_PRINT(" [ERROR] " fmt)
#define SAMPLE_SUPERBLINDSCAN_WARN_PRINT(fmt...)        MT_SUPERBLINDSCAN_PRINT(" [WARN] "  fmt)
#define SAMPLE_SUPERBLINDSCAN_INFO_PRINT(fmt...)        MT_SUPERBLINDSCAN_PRINT(" [INFO] "  fmt)
#define SAMPLE_SUPERBLINDSCAN_DBG_PRINT(fmt...)         MT_SUPERBLINDSCAN_PRINT(" [DEBUG] " fmt)


#define SAMPLE_SUPERBLINDSCAN_PRINT  printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MAX_TP_NUM          64

/*************************** Structure Definition ****************************/
typedef struct
{
    mt_u32 tuner_id;
    mt_u32 startfreq; /**<start Frequency, in kHz*/
    mt_u32 endfreq; /**<stopfreq Frequency, in kHz*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar; /**<Polarization mode>*/
    mt_u32 bs_mode; /**<blind mode>*/
} mt_input_Blindscan_para_t;

typedef enum
{
    MT_BLINDSCAN_STATUS_INIT,
    MT_BLINDSCAN_STATUS_SCANNING,
    MT_BLINDSCAN_STATUS_FINISH,

} mt_blindscan_status_t;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    mt_u32             tpIndex;
} MT_Super_RUN_INFO;


typedef struct
{
    mt_u32 freq;
    mt_u32 symbol_rate;
    mt_u32 program_num;
    mt_u8 dvb_type;
    PMT_COMPACT_PROG proginfo[64];
} mt_tpinfo_para_t;


typedef  struct
{
    mt_u32 PLSCodeAll;
} PlsDetail;


typedef struct
{
    mt_s32 total_num;
    mt_s32 s32TPNum;
    mt_unf_fe_sat_tpinfo_t satTPinfo[MAX_TP_NUM];
    mt_tpinfo_para_t tppara[MAX_TP_NUM];
    MT_Super_RUN_INFO stsuperRunInfo;
    mt_blindscan_status_t bs_status;
    PlsDetail plsdetail;
}mt_TPinfo_para_t;


/********************** Global Variable declaration **************************/
static mt_TPinfo_para_t g_sTPinfo;
static MT_BOOL    g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
MT_S32 MT_SuperBlindscanMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


static mt_s32 MT_SuperBlindscanCheckParam(mt_input_Blindscan_para_t *p_BlindScan_in)
{
    if(p_BlindScan_in->startfreq >= p_BlindScan_in->endfreq)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Frequency setting error\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_SuperBlindscanDmxInit(mt_input_Blindscan_para_t *pInutParam)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call mt_sys_get_version failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    /** Initializes the demux module */
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return ret;
    }


    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        if(pInutParam->tuner_id == 0)
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Connect port 0!\n");
        }
        else if(pInutParam->tuner_id == 1)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            /** Bind Demux to tuner port 3 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_3);
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Connect port 3!\n");
#else
            /** Bind Demux to tuner port 1 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Connect port 1!\n");
#endif
        }
        else
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Connect port 0!\n");
        }
    }
    else
    {
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_SuperBlindscanDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}



/*!
@brief audio and video player initialization.
@param[out] phAvplay            Handle to AV player
@param[out] hWin                The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_SuperBlindscanAvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_SUPERBLINDSCAN_FUNCTION_ENTER();

    if(NULL == phAvplay)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phWin)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phSoundTrack)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        return ret;

    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    /** Creates an AVPLAY */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }
    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_SUPERBLINDSCAN_FUNCTION_EXIT();

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




/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return void

*/
static void  MT_SuperBlindscanAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{

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
 @brief Audio and video Type of decoding
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_SuperBlindscanSetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };

    SAMPLE_SUPERBLINDSCAN_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return ret;
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

    SAMPLE_SUPERBLINDSCAN_INFO_PRINT("VidPid=%x, Vidtype=0x%x, AudPid=%x, AudType=0x%x \n", VidPid, enVidType, AudPid, u32AudType);

    /** Get the audio properties of the AV player */

    if(VidPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }
        /* The type of code stream supported by the decoder*/
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
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Set video properties or video PID property failed.\n");
            return ret;
        }
    }


    if(AudPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }
        /* PCM decoding mode*/
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Setting the decoding mode or audio PID property failed:%#x\n",ret);
            return ret;
        }
    }

    /* insert pts*/
    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /* Defines the attribute of low delay*/

        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            return ret;
        }
    }

    SAMPLE_SUPERBLINDSCAN_FUNCTION_EXIT();


    return MT_SUCCESS;
}



/*!
@brief start the AV playback into the start state.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_SuperBlindscanStarToPlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_SUPERBLINDSCAN_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("hAvplay is not exist\n");
        return ret;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_SuperBlindscanSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanSetAvplayPidAndCodecType fail! \n");
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
        SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Has no audio stream!\n");
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Has no video stream!\n");
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
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Set frame to VO fail.\n");
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Get avplay sync attr fail!\n");
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.stSyncStartRegion.bSmoothPlay = MT_TRUE;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Set avplay sync attr fail!\n");
            return ret;
        }
    }

    /*start to play audio and video*/
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_UNF_AVPLAY_Start fail!  ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_SUPERBLINDSCAN_FUNCTION_EXIT();

    return MT_SUCCESS;
}



/*!
@brief stop AV playback into the stop state.
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_SuperBlindscanStopToPlay(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;

    SAMPLE_SUPERBLINDSCAN_INFO_PRINT("stop live play ...\n");

    /*stop playing audio and video*/
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}


static MT_S32 MT_SuperBlindscanPlay(MT_HANDLE     hAvplay, mt_u32 tuner_id, MT_BOOL onoff_22k, MT_U32 polar)
{
    MT_S32 ret = 0;
    MT_S32 j = 0;
    MT_S32 i = 0;
    MT_S32 k = -1;
    PMT_COMPACT_TBL *progTbl = MT_NULL;
    PMT_COMPACT_PROG   *stCurrentProgInfo = { 0 };

    for(i = 0; i<g_sTPinfo.s32TPNum; i++)
    {
        ret = MTADP_Fe_Connect_Dvbs(tuner_id, (g_sTPinfo.satTPinfo[i].freq)/1000+1, (g_sTPinfo.satTPinfo[i].symbol_rate), onoff_22k, polar, 2);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Fe_Connect_Dvbs failed.\n");
            continue;
        }

        /** Get the PMT table */
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
            continue;
        }

        k++;
        SAMPLE_SUPERBLINDSCAN_INFO_PRINT("k = %d \n", k);


        for(j = 0; j<progTbl->prog_num; j++)
        {
            stCurrentProgInfo = progTbl->proginfo + j;
            g_sTPinfo.tppara[k].freq = (g_sTPinfo.satTPinfo[i].freq)/1000+1;
            g_sTPinfo.tppara[k].symbol_rate = g_sTPinfo.satTPinfo[i].symbol_rate;
            g_sTPinfo.tppara[k].dvb_type = g_sTPinfo.satTPinfo[i].dvb_type;
            g_sTPinfo.tppara[k].program_num = progTbl->prog_num;
            g_sTPinfo.tppara[k].proginfo[j].VElementNum = stCurrentProgInfo->VElementNum;
            g_sTPinfo.tppara[k].proginfo[j].AElementNum = stCurrentProgInfo->AElementNum;
            g_sTPinfo.tppara[k].proginfo[j].PcrPid = stCurrentProgInfo->PcrPid;
            g_sTPinfo.tppara[k].proginfo[j].VElementPid = stCurrentProgInfo->VElementPid;
            g_sTPinfo.tppara[k].proginfo[j].VideoType = stCurrentProgInfo->VideoType;
            g_sTPinfo.tppara[k].proginfo[j].AElementPid = stCurrentProgInfo->AElementPid;
            g_sTPinfo.tppara[k].proginfo[j].AudioType = stCurrentProgInfo->AudioType;
            g_sTPinfo.tppara[k].proginfo[j].PmtPid = stCurrentProgInfo->PmtPid;
            g_sTPinfo.tppara[k].proginfo[j].ProgID = stCurrentProgInfo->ProgID;
            for(MT_S32 n = 0;n < stCurrentProgInfo->AElementNum; n++)
            {
                g_sTPinfo.tppara[k].proginfo[j].Audioinfo[n].u16AudioPid = stCurrentProgInfo->Audioinfo[n].u16AudioPid;
                g_sTPinfo.tppara[k].proginfo[j].Audioinfo[n].u32AudioEncType = stCurrentProgInfo->Audioinfo[n].u32AudioEncType;
            }

        }
        (MT_VOID)MTADP_Search_FreeAllPmt(progTbl);
        g_sTPinfo.total_num += g_sTPinfo.tppara[k].program_num;
    }
    g_sTPinfo.s32TPNum = k + 1;

    ret = MTADP_Fe_Connect_Dvbs(tuner_id, g_sTPinfo.tppara[0].freq, (g_sTPinfo.tppara[0].symbol_rate), onoff_22k, polar, 2);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Fe_Connect_Dvbs failed.\n");
        return MT_FAILURE;
    }
    g_sTPinfo.stsuperRunInfo.tpIndex = 0;
    ret = MT_SuperBlindscanStarToPlay(hAvplay, (g_sTPinfo.tppara[0].proginfo));
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanStarToPlay failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*
@brief set tuner parameters
@param[in] tuner_id,Port of the tuner
@param[in] sig_type,Type of received signal
@param[in] tuner_dev_type,tuner Device type
@param[in] tuner_addr, The address of tuner
@param[in] demod_dev_type, Type of the demod device
@param[in] demod_addr, The address of demod
@param[in] out_put_mode, Output mode
@param[in] I2c_channel, i2c Channel mode
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_SuperBlindscan_Set_Parameter(MT_U32 tuner_id, MT_U32 sig_type, MT_U32 tuner_dev_type, MT_U32 tuner_addr,
             MT_U32 demod_dev_type, MT_U32 demod_addr, MT_U32 out_put_mode, MT_U32 I2c_channel)
{
    MT_S32           ret = 0;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };
    mt_sys_version_s stSysChipInfo = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("mt_sys_get_version err!\n");
        return ret;
    }

    SAMPLE_SUPERBLINDSCAN_INFO_PRINT("MTCommand_Tuner_Operation:   chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id,&mtTunerAttr);
    if(SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("mt_unf_fe_get_default_attr err!\n");
        return ret;
    }


    mtTunerAttr.sig_type = sig_type;

    if (stSysChipInfo.enChipVersion < MT_CHIP_SYMPHONY2_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY2_A3)
    {
        if ((tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC3800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC6800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_MXL_608))
        {
            mtTunerAttr.fe_config.tun2_type = tuner_dev_type;
            mtTunerAttr.fe_config.tun2_addr = tuner_addr;
        }
        else
        {
            mtTunerAttr.tuner_type = tuner_dev_type;
            mtTunerAttr.tuner_addr = tuner_addr;
        }
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY4_A1)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    else if(stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY6_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#endif
    else
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }

    mtTunerAttr.demod_dev_type = demod_dev_type;
    mtTunerAttr.demod_addr = demod_addr;
    mtTunerAttr.demod_i2c_id = I2c_channel;
    mtTunerAttr.output_mode = out_put_mode;
    mtTunerAttr.tuner_i2c_id[0] = 0;
    mtTunerAttr.no_need_init = 0;

    SAMPLE_SUPERBLINDSCAN_INFO_PRINT("tuner_type[%d], tuner_addr[0x%x], demod_type[%d], demod_addr[0x%x], demod_id[%d], output_mode[%d] \n",
                            mtTunerAttr.tuner_type, mtTunerAttr.tuner_addr,
                            mtTunerAttr.demod_dev_type, mtTunerAttr.demod_addr,
                            mtTunerAttr.demod_i2c_id, mtTunerAttr.output_mode );

    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("mt_unf_fe_set_attr failed.ret is %d\n",ret);
        return ret;
    }

    SAMPLE_SUPERBLINDSCAN_INFO_PRINT("output_mode = %d\n", out_put_mode);

    return MT_SUCCESS;
}


static MT_S32 MT_SuperBlindscan_Notify(MT_U32 tuner_id, mt_unf_fe_blindscan_evt_t enEVT, mt_unf_fe_blindscan_notify_t *punNotify)
{
  mt_s32 i = 0;
  mt_s32 num = 0;
  mt_unf_fe_sat_tpinfo_t temp;



  switch (enEVT)
  {
    case MT_UNF_FE_BLINDSCAN_EVT_STATUS:
      if(MT_UNF_FE_BLINDSCAN_STATUS_FAIL == *(punNotify->status))
      {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Scan fail.\n");
        return MT_FAILURE;
      }
      else if ((MT_UNF_FE_BLINDSCAN_STATUS_FINISH == *(punNotify->status)) || (MT_UNF_FE_BLINDSCAN_STATUS_QUIT == *(punNotify->status)))
      {
        SAMPLE_SUPERBLINDSCAN_PRINT("100%%");
        SAMPLE_SUPERBLINDSCAN_PRINT("done\n");
        SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Scan over, find %d TP.\n", g_sTPinfo.s32TPNum);
        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_FINISH;
        return MT_SUCCESS;


      }
      break;

    case MT_UNF_FE_BLINDSCAN_EVT_PROGRESS:

      SAMPLE_SUPERBLINDSCAN_PRINT("%d%% \n", *(punNotify->progress_percent));
      break;

    case MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT:
      num++;
      temp = *(punNotify->result);
      SAMPLE_SUPERBLINDSCAN_PRINT("\t%03d %7d %5d %d %d, %d \n", num, temp.freq, temp.symbol_rate, temp.polar, temp.code_rate, temp.DataTsNumber);

      for (i = 0; i < temp.DataTsNumber; i++)
      {
            SAMPLE_SUPERBLINDSCAN_PRINT("TS[%d] ---- 0x%02x\n", i, temp.DataTsIdArray[i]);
            (MT_VOID)mt_unf_fe_set_s2_multi_stream_ts_id(tuner_id, temp.DataTsIdArray[i]);

            MT_USLEEP(200000);
      }


      g_sTPinfo.satTPinfo[g_sTPinfo.s32TPNum] = *(punNotify->result);
      g_sTPinfo.s32TPNum++;
      g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
      break;

    default:
      break;
  }

  return MT_SUCCESS;
}


static MT_S32 MT_SuperBlindscan_Dvbs(mt_u32 tuner_id, MT_U8 bs_mode, MT_U32 u32StartFreq, MT_U32 u32StopFreq, MT_U8 polar, MT_U8 onoff_22k)
{
    MT_S32 ret = 0;
    mt_unf_fe_blindscan_para_t stBlindScanPara = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    ret = MT_SuperBlindscan_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 1, 0);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    if (tuner_id == 0)
    {
        ret = MT_SuperBlindscan_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 4, 0);
    }
    else
    {
        ret = MT_SuperBlindscan_Set_Parameter(tuner_id, 2048, 80, 90, 336, 210, 4, 1);
    }
#endif
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call MT_SuperBlindscan_Set_Parameter failed.\n");
        return MT_FAILURE;
    }

    ret = mt_unf_fe_switch_super_search_mode(tuner_id, 1);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call mt_unf_fe_switch_super_search_mode failed.\n");
        return MT_FAILURE;
    }
    memset(&stBlindScanPara, 0, sizeof(mt_unf_fe_blindscan_para_t));

    if (0 == bs_mode)
    {
        /* Auto */
        stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_AUTO;
        /* If your diseqc device need config polarization and 22K, you need register the callback */
        stBlindScanPara.scan_para.sat.diseqc_set = MT_NULL;
        stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))MT_SuperBlindscan_Notify;
    }
    else
    {
        stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_MANUAL;
        stBlindScanPara.scan_para.sat.polar = polar;
        stBlindScanPara.scan_para.sat.lnb_22k = onoff_22k;
        stBlindScanPara.scan_para.sat.start_freq = u32StartFreq * 1000;
        stBlindScanPara.scan_para.sat.stop_freq = u32StopFreq * 1000;
        stBlindScanPara.scan_para.sat.diseqc_set = MT_NULL;
        stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))MT_SuperBlindscan_Notify;
    }


    ret = mt_unf_fe_blindscan_start(tuner_id, &stBlindScanPara);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("call mt_unf_fe_blindscan_start failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

//not used
#if 0
static MT_S32 MT_SuperBlindscan_Lock_Dvbs(MT_U32 tuner_id, MT_U32 freq, MT_U32 sym_rate, MT_U32 onoff_22k, MT_U32 polar, MT_U32 port_type, MT_U32 u32LoopTimes)
{
    MT_S32                   ret = 0;
    MT_U32                   u32Loop = 0;
    MT_U32                   u32Freq = 0;
    MT_U32                   u32SymbolRate = 0;
    mt_unf_fe_status_t       stTunerStatus = { 0 };
    mt_unf_fe_connect_para_t stConnectPara = { 0 };
    MT_U32 connect_mode = 0;


    if (MT_UNF_PORT_TYPE_DVBS == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
        SAMPLE_SUPERBLINDSCAN_INFO_PRINT("dvbs\n");
    }
    else if (MT_UNF_PORT_TYPE_DVBS2 == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_SAT_2;
        SAMPLE_SUPERBLINDSCAN_INFO_PRINT("dvbs2\n");
    }
    else if(MT_UNF_PORT_TYPE_DVBS_AUTO == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
        SAMPLE_SUPERBLINDSCAN_INFO_PRINT("dvbs_auto\n");
    }

    stConnectPara.connect_param.sat.freq = freq * 1000;
    stConnectPara.connect_param.sat.sym_rate = sym_rate;
    stConnectPara.connect_param.sat.port_type = port_type;
    stConnectPara.connect_param.sat.onoff_22k = onoff_22k;
    stConnectPara.connect_param.sat.polarization = polar;

    stConnectPara.connect_param.sat.PLS.PLSCodeAll = g_sTPinfo.plsdetail.PLSCodeAll;

    stConnectPara.connect_mode = (mt_unf_fe_connect_mode_t)connect_mode;
    ret = mt_unf_fe_connect(tuner_id, &stConnectPara, 0);
    /* Get the actual lock frequency and rate*/
    u32Freq = stConnectPara.connect_param.sat.freq;
    u32SymbolRate = stConnectPara.connect_param.sat.sym_rate;

    if (MT_SUCCESS == ret)
    {
        for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++)
        {
            ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
            if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
            {
                SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Tuner Lock freq %d symb %d polar%d Success!\n", u32Freq, u32SymbolRate, polar);
                return MT_SUCCESS;
            }
            else
            {
                usleep(10000);
            }
        }
    }
    else
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Tuner Lock freq %d symb %d polar%d mt_unf_fe_connect Fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, polar, ret);
    }

    if (u32Loop == u32LoopTimes)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Tuner Lock freq %d symb %d  polar%d Fail!\n", u32Freq, u32SymbolRate, polar);
    }


    return MT_FAILURE;
}
#endif

static MT_VOID MT_SuperBlindscanPrintMenu(MT_S32 pronum)
{

    SAMPLE_SUPERBLINDSCAN_PRINT("\n 1 - %d : select the  number to play  \n", pronum);
    SAMPLE_SUPERBLINDSCAN_PRINT("     p : print program info \n");
    SAMPLE_SUPERBLINDSCAN_PRINT("     r : Restart scan TP \n");
    SAMPLE_SUPERBLINDSCAN_PRINT("     s : Set PLSN value \n");
    SAMPLE_SUPERBLINDSCAN_PRINT("     k : To play all tp \n");
    SAMPLE_SUPERBLINDSCAN_PRINT("     z : multistream to play other \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_SUPERBLINDSCAN_PRINT("     b : background run \n");
#endif
    SAMPLE_SUPERBLINDSCAN_PRINT("     h : help \n");
    SAMPLE_SUPERBLINDSCAN_PRINT("     q : quit \n");
    SAMPLE_SUPERBLINDSCAN_PRINT("SuperBlindscan>> ");

}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_SuperBlindscanPrint_help(char *name)
{

    SAMPLE_SUPERBLINDSCAN_PRINT("Options:\n");
    SAMPLE_SUPERBLINDSCAN_PRINT(" %s -t tuner_id -b mode -s start_freq -e stop_freq -k 22k -p polar \n", name);
    SAMPLE_SUPERBLINDSCAN_PRINT(" %s -t 0 -b 1 -s 1000 -e 1500 -k 0 -p 0\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_SUPERBLINDSCAN_PRINT(" %s -q  <exit> \n", name);
#endif

}



static MT_VOID MT_SuperBlindscanPrintProgramInfo(mt_tpinfo_para_t tppara[32])
{
    MT_S32  i = 0;
    MT_S32  j = 0;
    MT_S32  k = 0;
    MT_S32  n = 1;

    MT_CHAR dvb_type[g_sTPinfo.s32TPNum][10];
    for(i = 0;i < g_sTPinfo.s32TPNum; i++)
    {
        switch(tppara[i].dvb_type)
        {
            case 0:
                strcpy(dvb_type[i], "DVB-S");
                break;
            case 1:
                strcpy(dvb_type[i], "DVB-S2");
                break;
            default:
                break;
        }
    }


    for(k = 0; k < g_sTPinfo.s32TPNum; k++)
    {
        SAMPLE_SUPERBLINDSCAN_PRINT("<Index>          <Freq>           <Sym Rate>         <Type> \n");
        SAMPLE_SUPERBLINDSCAN_PRINT(" %3d         %10dKHz     %8dKSs           %5s\n", k+1, tppara[k].freq, tppara[k].symbol_rate, dvb_type[k]);
        for(i =0; i<tppara[k].program_num;i++)
        {
            SAMPLE_SUPERBLINDSCAN_PRINT("Channel Num = %d, Program ServiceID = %d PMT PID = %x\n", i+1, tppara[k].proginfo[i].ProgID,tppara[k].proginfo[i].PmtPid);

             for (j = 0; j < tppara[k].proginfo[i].VElementNum;j++)
             {
                 SAMPLE_SUPERBLINDSCAN_PRINT("\tVideo Stream PID   = 0x%x\n",tppara[k].proginfo[i].VElementPid);
                 switch (tppara[k].proginfo[i].VideoType)
                 {
                    case MT_UNF_VCODEC_TYPE_H264:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tVideo Stream Type H264\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG2:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tVideo Stream Type MP2\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG4:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tVideo Stream Type MP4\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_HEVC:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tVideo Stream Type HEVC\n");
                        break;
                    default:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tVideo Stream Type error\n");
                 }
             }
             for (j = 0; j < tppara[k].proginfo[i].AElementNum; j++)
             {
                SAMPLE_SUPERBLINDSCAN_PRINT("\tAudio Stream PID   = 0x%x\n", tppara[k].proginfo[i].Audioinfo[j].u16AudioPid);
                switch (tppara[k].proginfo[i].Audioinfo[j].u32AudioEncType)
                {
                    case HA_AUDIO_ID_MP3:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tAudio Stream Type MP3\n");
                        break;
                    case HA_AUDIO_ID_AAC:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tAudio Stream Type AAC\n");
                        break;
                    case HA_AUDIO_ID_DOLBY_PLUS:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tAudio Stream Type AC3\n");
                        break;
                    case HA_AUDIO_ID_DTSHD:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tAudio Stream Type DTS\n");
                        break;
                    default:
                        SAMPLE_SUPERBLINDSCAN_PRINT("\tAudio Stream Type error\n");
                 }
             }
             SAMPLE_SUPERBLINDSCAN_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        }
    }
    for(i = 0; i < g_sTPinfo.s32TPNum; i++)
    {
        SAMPLE_SUPERBLINDSCAN_PRINT("TP:%d( %d - %d)\n", i+1, n, tppara[i].program_num + n-1);


         n = n + tppara[i].program_num;
    }
}

static MT_S32 MT_SuperBlindscanCmdTask(mt_handle hAvplay, mt_input_Blindscan_para_t *pInutParam)
{
    MT_S32  ret = 0;
    MT_U32  plsCodeAll = 0;
    MT_CHAR inputCmd[32] = { 0 };
    MT_S32 currentTp = 0;
    MT_S32 i = 0;
    MT_S32 progNum = 0;
    MT_S32 s32ProgNum = 0;
    mt_unf_fe_connect_para_t connect_para;
    mt_u8 ts_id = 0;
    mt_s32 n = 0;
    mt_s32 j = 0;
    mt_s32 k = 0;
    PMT_COMPACT_TBL *progTbl = MT_NULL;
    PMT_COMPACT_PROG   *stCurrentProgInfo = { 0 };

    while(g_sTPinfo.bs_status != MT_BLINDSCAN_STATUS_FINISH)
    {

       usleep(5000);
    }

    currentTp = g_sTPinfo.stsuperRunInfo.tpIndex;

    while(1)
    {
        (MT_VOID)MT_SuperBlindscanPrintMenu(g_sTPinfo.total_num);
        /* get inputCmd*/
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("prepare to exit!\n");
            g_bTaskQuit  = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("superblindscan play in back!\n");
            g_sTPinfo.stsuperRunInfo.tpIndex = currentTp;
            break;
        }
#endif
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            progNum = atoi(inputCmd);
            if(progNum <= g_sTPinfo.total_num)
            {
                for(i = 0; i< g_sTPinfo.s32TPNum; i++)
                {
                    s32ProgNum = progNum - (g_sTPinfo.tppara[i].program_num);
                    if(s32ProgNum > 0)
                    {
                        progNum = s32ProgNum;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                SAMPLE_SUPERBLINDSCAN_INFO_PRINT("The biggest num is %d \n", g_sTPinfo.total_num);
                continue;
            }

            if(currentTp != i)
            {
                ret = MTADP_Fe_Connect_Dvbs(pInutParam->tuner_id,  g_sTPinfo.tppara[i].freq, g_sTPinfo.tppara[i].symbol_rate, pInutParam->onoff_22k, pInutParam->polar, 2);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Fe_Connect_Dvbs failed.\n");
                }
                currentTp = i;
            }
            ret = MT_SuperBlindscanStopToPlay(hAvplay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanStopToPlay failed.\n");
            }
            ret = MT_SuperBlindscanStarToPlay(hAvplay, g_sTPinfo.tppara[i].proginfo + progNum - 1);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanStarToPlay failed.\n");
            }


        }
        else if('k' == inputCmd[0])
        {
            if(0 == g_sTPinfo.total_num)
            {
                ret = MT_SuperBlindscanPlay(hAvplay, pInutParam->tuner_id, pInutParam->onoff_22k, pInutParam->polar);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanPlay failed.\n");
                }
            }

        }
        else if('p' == inputCmd[0])
        {
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Print program info \n");
            (MT_VOID)MT_SuperBlindscanPrintProgramInfo(g_sTPinfo.tppara);
        }
        else if('r' == inputCmd[0])
        {
            if(0 != g_sTPinfo.total_num)
            {
                ret = MT_SuperBlindscanStopToPlay(hAvplay);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanStopToPlay failed.\n");
                }
            }
            memset(&g_sTPinfo, 0, sizeof(g_sTPinfo));
            g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
            ret = MT_SuperBlindscan_Dvbs(pInutParam->tuner_id, pInutParam->bs_mode,pInutParam->startfreq, pInutParam->endfreq, pInutParam->polar, pInutParam->onoff_22k);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_Blindscan_Dvbs failed.\n");
            }
            while(g_sTPinfo.bs_status != MT_BLINDSCAN_STATUS_FINISH)
            {

                usleep(5000);
            }

        }
        else if('z' == inputCmd[0])
        {

            ret = MT_SuperBlindscanStopToPlay(hAvplay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT(" MT_DvbsStopToPlay failed.\n");
            }

            for(i = 0; i < g_sTPinfo.s32TPNum; i++)
            {

                SAMPLE_SUPERBLINDSCAN_PRINT("\t%03d %7d %5d %d %d, %d \n", i, g_sTPinfo.satTPinfo[i].freq,
                    g_sTPinfo.satTPinfo[i].symbol_rate, g_sTPinfo.satTPinfo[i].polar, g_sTPinfo.satTPinfo[i].code_rate,
                    g_sTPinfo.satTPinfo[i].DataTsNumber);
            }

            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("please input you want to play TP[0 - %d):", g_sTPinfo.s32TPNum);
            scanf("%d", &currentTp);
            getchar();

            if (currentTp < 0 || currentTp >= g_sTPinfo.s32TPNum)
            {
                SAMPLE_SUPERBLINDSCAN_WARN_PRINT("input TP is over range. \n");
            }

            ret = MTADP_Fe_Connect_Dvbs(pInutParam->tuner_id,  g_sTPinfo.satTPinfo[currentTp].freq/1000+1, g_sTPinfo.satTPinfo[currentTp].symbol_rate,
                                        pInutParam->onoff_22k, pInutParam->polar, 2);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Fe_Connect_Dvbs failed.\n");
            }

            ret = mt_unf_fe_get_s2_multi_stream_info(pInutParam->tuner_id, &connect_para);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT("mt_unf_fe_get_s2_multi_stream_info failed.\n");
            }

            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("total ts number id %d \n", connect_para.connect_param.sat.DataTsNumber);
            for (n = 0; n < connect_para.connect_param.sat.DataTsNumber; n++)
            {
                SAMPLE_SUPERBLINDSCAN_INFO_PRINT("\t\tTS[%d] ---- ts_id[%02x]\n", n, connect_para.connect_param.sat.DataTsIdArray[n]);
            }
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("please input you want to play ts_id:");
            scanf("%c", &ts_id);
            getchar();
            mt_unf_fe_set_s2_multi_stream_ts_id(pInutParam->tuner_id, ts_id);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT("mt_unf_fe_set_s2_multi_stream_ts_id failed.\n");
            }

            g_sTPinfo.total_num -= g_sTPinfo.tppara[currentTp].program_num;

            ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT(" MTADP_Search_GetAllPmt failed.\n");
            }

            for(j = 0; j<progTbl->prog_num; j++)
            {
                stCurrentProgInfo = progTbl->proginfo + j;

                g_sTPinfo.tppara[currentTp].program_num = progTbl->prog_num;
                g_sTPinfo.tppara[currentTp].proginfo[j].VElementNum = stCurrentProgInfo->VElementNum;
                g_sTPinfo.tppara[currentTp].proginfo[j].AElementNum = stCurrentProgInfo->AElementNum;
                g_sTPinfo.tppara[currentTp].proginfo[j].PcrPid = stCurrentProgInfo->PcrPid;
                g_sTPinfo.tppara[currentTp].proginfo[j].VElementPid = stCurrentProgInfo->VElementPid;
                g_sTPinfo.tppara[currentTp].proginfo[j].VideoType = stCurrentProgInfo->VideoType;
                g_sTPinfo.tppara[currentTp].proginfo[j].AElementPid = stCurrentProgInfo->AElementPid;
                g_sTPinfo.tppara[currentTp].proginfo[j].AudioType = stCurrentProgInfo->AudioType;
                g_sTPinfo.tppara[currentTp].proginfo[j].PmtPid = stCurrentProgInfo->PmtPid;
                g_sTPinfo.tppara[currentTp].proginfo[j].ProgID = stCurrentProgInfo->ProgID;
                for(k = 0; k < stCurrentProgInfo->AElementNum; k++)
                {
                    g_sTPinfo.tppara[currentTp].proginfo[j].Audioinfo[k].u16AudioPid = stCurrentProgInfo->Audioinfo[k].u16AudioPid;
                    g_sTPinfo.tppara[currentTp].proginfo[j].Audioinfo[k].u32AudioEncType = stCurrentProgInfo->Audioinfo[k].u32AudioEncType;
                }

            }
            (MT_VOID)MTADP_Search_FreeAllPmt(progTbl);
            g_sTPinfo.total_num += g_sTPinfo.tppara[currentTp].program_num;


            ret = MT_SuperBlindscanStarToPlay(hAvplay, g_sTPinfo.tppara[currentTp].proginfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SUPERBLINDSCAN_ERR_PRINT(" MT_DvbsStarToPlay failed.\n");
                return ret;
            }

        }
        else if('s' == inputCmd[0])
        {
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Please enter a PLSN value \n");

            scanf("%d", &plsCodeAll);
            g_sTPinfo.plsdetail.PLSCodeAll = (2 << 24) + (plsCodeAll & 0xFFFFFF);

        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_SUPERBLINDSCAN_INFO_PRINT("Print help info \n");
        }
    }
    return MT_SUCCESS;
}
static void MT_SuperBlindscanExit(mt_u32 tuner_id)
{
    (MT_VOID)mt_unf_fe_switch_super_search_mode(tuner_id, 0);
    (MT_VOID)MTADP_Fe_DeInit(tuner_id);
    memset(&g_sTPinfo, 0, sizeof(g_sTPinfo));
    g_bTaskQuit = MT_TRUE;
}

/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
static mt_s32 MT_SuperBlindscanParase_args(int argc, char *argv[], mt_input_Blindscan_para_t *pInutParam)
{
    mt_s32 opt = 0;

    while((opt = MTADP_Getopt(argc, argv, ":?hHb:e:p:k:s:t:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_SuperBlindscanPrint_help(argv[0]);
                return MT_FAILURE;
             case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_SuperBlindscanExit(pInutParam->tuner_id);
                }
                return MT_TASK_EXIT;

            case 'b':
                pInutParam->bs_mode = strtol(mt_optarg, 0, 0);
                break;

            case 's':
                pInutParam->startfreq = strtol(mt_optarg, 0, 0);
                break;

            case 'k':
                pInutParam->onoff_22k = strtol(mt_optarg, 0, 0);
                break;

            case 'e':
                pInutParam->endfreq = strtol(mt_optarg, 0, 0);
                break;

            case 'p':
                pInutParam->polar = strtol(mt_optarg, 0, 0);
                break;

            case 't':
                pInutParam->tuner_id = strtol(mt_optarg, 0, 0);
                break;

            default:
                (void)MT_SuperBlindscanPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_SuperBlindscanMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32  ret = 0;
    static mt_input_Blindscan_para_t    sInputParam = {0};

    if(argc != 13 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_SuperBlindscanPrint_help(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_SuperBlindscanParase_args(argc, argv, &sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {

        ret = MT_SuperBlindscanCheckParam(&sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanCheckParam failed.\n");
            return ret;
        }
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SYS_Init failed.\n");
            return ret;
        }
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_HDMI_Init failed.\n");
            goto ERR1;
        }
        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Disp_Init failed.\n");
            goto ERR2;

        }
#endif
        ret = MTADP_Fe_Init(sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Fe_Init failed.\n");
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {

            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_VO_Init failed.\n");
            goto ERR4;
        }
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MTADP_Snd_Init failed.\n");
            goto ERR5;
        }

        ret = MT_SuperBlindscanDmxInit(&sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanDmxInit failed.\n");
            goto ERR6;
        }

        (MT_VOID)MTADP_Search_Init();

        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_INIT;

        ret = MT_SuperBlindscan_Dvbs(sInputParam.tuner_id, sInputParam.bs_mode, sInputParam.startfreq, sInputParam.endfreq, sInputParam.polar, sInputParam.onoff_22k);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscan_Dvbs failed.\n");
            goto ERR8;
        }

        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
        ret = MT_SuperBlindscanAvplayInit(&g_sTPinfo.stsuperRunInfo.hAvPlay, &g_sTPinfo.stsuperRunInfo.hWin, &g_sTPinfo.stsuperRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SUPERBLINDSCAN_ERR_PRINT("MT_SuperBlindscanAvplayInit failed.\n");
            goto ERR8;
        }
        g_bTaskQuit = MT_FALSE;
    }

    (void)MT_SuperBlindscanCmdTask(g_sTPinfo.stsuperRunInfo.hAvPlay, &sInputParam);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    ret = MT_SuperBlindscanStopToPlay(g_sTPinfo.stsuperRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SUPERBLINDSCAN_ERR_PRINT(" MT_SuperBlindscanStopToPlay failed.\n");
    }

    /** Audio and video player deinitialization */
    (MT_VOID)MT_SuperBlindscanAvplayDeInit(g_sTPinfo.stsuperRunInfo.hAvPlay, g_sTPinfo.stsuperRunInfo.hWin, g_sTPinfo.stsuperRunInfo.hSoundTrack);
ERR8:

    (MT_VOID)MTADP_Search_DeInit();

    /** Demux module deinitialization */
    (MT_VOID)MT_SuperBlindscanDmxDeInit();
ERR6:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

ERR5:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();

ERR4:
    (MT_VOID)mt_unf_fe_switch_super_search_mode(sInputParam.tuner_id, 0);
    (MT_VOID)MTADP_Fe_DeInit(sInputParam.tuner_id);


ERR3:
#ifndef MT_SAMPLE_APP
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();

ERR2:

    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR1:

    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_sTPinfo, 0, sizeof(g_sTPinfo));
    return ret;


}

