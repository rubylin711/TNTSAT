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
#include "mt_cmdline.h"
#include "mt_adp_pvr.h"

/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DVBS_T2MI_DEBUG

#define MT_DVBS_T2MI_PRINT   printf
#else

#define MT_DVBS_T2MI_PRINT

#endif

#define SAMPLE_DVBS_T2MI_FUNCTION_ENTER()   MT_DVBS_T2MI_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DVBS_T2MI_FUNCTION_EXIT()        MT_DVBS_T2MI_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DVBS_T2MI_FATAL_PRINT(fmt...)        MT_DVBS_T2MI_PRINT(" [FATAL] " fmt)
#define SAMPLE_DVBS_T2MI_ERR_PRINT(fmt...)          MT_DVBS_T2MI_PRINT(" [ERROR] " fmt)
#define SAMPLE_DVBS_T2MI_WARN_PRINT(fmt...)         MT_DVBS_T2MI_PRINT(" [WARN] "  fmt)
#define SAMPLE_DVBS_T2MI_INFO_PRINT(fmt...)         MT_DVBS_T2MI_PRINT(" [INFO] "  fmt)
#define SAMPLE_DVBS_T2MI_DBG_PRINT(fmt...)          MT_DVBS_T2MI_PRINT(" [DEBUG] " fmt)


#define SAMPLE_DVBS_T2MI_PRINT  printf


#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

/*************************** Structure Definition ****************************/
typedef struct
{
    mt_u32 tuner_id;
    mt_u32 freq; /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar; /**<Polarization mode>*/
    mt_u32 pid;
    mt_u32 plpid;
} mt_input_Dvbs_para_t;

typedef struct hiPMT_COMPACT_INFO_S
{
    mt_u32 ProgID;          /* program ID */
    mt_u32 PmtPid;          /*program PMT PID*/
    mt_u32 PcrPid;          /*program PCR PID*/

    mt_u32   VideoType;
    mt_u16               VElementNum;        /* video stream number */
    mt_u16               VElementPid;        /* the first video stream PID*/

    mt_u32   AudioType;
    mt_u16               AElementNum;        /* audio stream number */
    mt_u16               AElementPid;        /* the first audio stream PID*/

} PMT_COMPACT_info_t;
typedef struct
{
    mt_u32 program_num;
    PMT_COMPACT_info_t proginfo[256];
} mt_plpid_para_t;


typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    mt_plpid_para_t    plp[32];
    mt_s32 total_num;
    mt_s32 plp_num;
} MT_DVBS2T2MI_RUN_INFO;


/********************** Global Variable declaration **************************/

static MT_BOOL    g_bTaskQuit = MT_TRUE;
static MT_DVBS2T2MI_RUN_INFO    g_stDvbsT2miRunInfo = {MT_INVALID_HANDLE};

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

#ifdef MT_SAMPLE_APP
MT_S32 MT_Dvbs_t2miMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_DvbsT2MICheckParam(mt_input_Dvbs_para_t *p_dvbs_in)
{
    if((p_dvbs_in->freq) > 4200 || (p_dvbs_in->freq) < 3000)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        SAMPLE_DVBS_T2MI_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbsT2MIDmxInit(mt_u32 tuner_id)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("mt_sys_get_version failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    /** Initializes the demux module */
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return ret;
    }

    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        if(tuner_id == 0)
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_DVBS_T2MI_INFO_PRINT("Connect port 0!\n");
            play_resource.sig_type = 1;
        }
        else if(tuner_id == 1)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            /** Bind Demux to tuner port 3 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_3);
            SAMPLE_DVBS_T2MI_INFO_PRINT("Connect port 3!\n");
            play_resource.sig_type = 3;
#else
            /** Bind Demux to tuner port 1 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            SAMPLE_DVBS_T2MI_INFO_PRINT("Connect port 1!\n");
            play_resource.sig_type = 0;
#endif
        }
        else
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_DVBS_T2MI_INFO_PRINT("Connect port 0!\n");
            play_resource.sig_type = 1;
        }

    }
    else
    {
        /** Bind Demux to tuner port 1 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        SAMPLE_DVBS_T2MI_INFO_PRINT("Connect port 1!\n");
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_DvbsT2MIDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_T2MIEnable(MT_FALSE);

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
static MT_S32 MT_DvbsT2MIAvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_DVBS_T2MI_FUNCTION_ENTER();

    if(NULL == phAvplay)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phWin)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phSoundTrack)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        return ret;

    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
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
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }
    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_DVBS_T2MI_FUNCTION_EXIT();

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
static void  MT_DvbsT2MIAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
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
static MT_S32 MT_DvbsT2MISetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_info_t *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };
    SAMPLE_DVBS_T2MI_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("=====pProgInfo == NULL=====\n");
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

    SAMPLE_DVBS_T2MI_INFO_PRINT("VidPid=%x, Vidtype=0x%x, AudPid=%x, AudType=0x%x \n", VidPid, enVidType, AudPid, u32AudType);

    /** Get the audio properties of the AV player */

    if(VidPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
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
            SAMPLE_DVBS_T2MI_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("Set video properties or video PID property failed.\n");
            return ret;
        }
    }


    if(AudPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }
        /* PCM decoding mode*/
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("Setting the decoding mode or audio PID property failed:%#x\n",ret);
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
            SAMPLE_DVBS_T2MI_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            return ret;
        }
    }

    SAMPLE_DVBS_T2MI_FUNCTION_EXIT();


    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbsT2MIStarToPlay(mt_handle hAvplay, const PMT_COMPACT_info_t *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_DVBS_T2MI_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("hAvplay is not exist\n");
        return ret;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_DvbsT2MISetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("MT_DvbsT2MISetAvplayPidAndCodecType fail! \n");
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
        SAMPLE_DVBS_T2MI_INFO_PRINT("Has no audio stream!\n");
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_DVBS_T2MI_INFO_PRINT("Has no video stream!\n");
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
            SAMPLE_DVBS_T2MI_ERR_PRINT("Set frame to VO fail.\n");
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("Get avplay sync attr fail!\n");
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
            SAMPLE_DVBS_T2MI_ERR_PRINT("Set avplay sync attr fail!\n");
            return ret;
        }
    }

    /*start to play audio and video*/
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("MT_UNF_AVPLAY_Start fail!  ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_DVBS_T2MI_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state.
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbsT2MIStopToPlay(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    option.u32TimeoutMs = 0;

    SAMPLE_DVBS_T2MI_INFO_PRINT("stop live play ...\n");

    /*stop playing audio and video*/
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}

static MT_VOID MT_DvbsT2MIPrintMenu(MT_U32 prog_num)
{

    SAMPLE_DVBS_T2MI_PRINT("\n 1 - %d : select the program \n", prog_num);
#ifdef MT_SAMPLE_APP
    SAMPLE_DVBS_T2MI_PRINT("     b : background run \n");
#endif
    SAMPLE_DVBS_T2MI_PRINT("     h : help \n");
    SAMPLE_DVBS_T2MI_PRINT("     q : quit \n");
    SAMPLE_DVBS_T2MI_PRINT("DVBS_T2MI>> ");

}

static MT_VOID MT_DvbsT2MIExit(void)
{
    SAMPLE_DVBS_T2MI_FUNCTION_ENTER();

    (MT_VOID)MT_DvbsT2MIStopToPlay(g_stDvbsT2miRunInfo.hAvPlay);


    (MT_VOID)MT_DvbsT2MIAvplayDeInit(g_stDvbsT2miRunInfo.hAvPlay, g_stDvbsT2miRunInfo.hWin, g_stDvbsT2miRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_DeInit();

    (MT_VOID)MT_DvbsT2MIDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    memset(&g_stDvbsT2miRunInfo, 0, sizeof(g_stDvbsT2miRunInfo));


    g_bTaskQuit = MT_TRUE;

    SAMPLE_DVBS_T2MI_FUNCTION_EXIT();
}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_DvbsT2MIPrint_help(char *name)
{
    SAMPLE_DVBS_T2MI_PRINT(" [ options ]...\n"
       "\n"
       "Options:\n"
       " ?/-h/-H        print this help\n"
       " -f <freq.M>    set freq  \n"
       " -s <srate.K>   set srate default 27500\n"
       " -k <22k>       set 22k on/off:0 is off, 1 is on\n"
       " -p <polar>     0/1:0 is the horizontal polarization,1 is vertically polarized\n"
       " -d <pid>       Set the pid of the t2mi stream\n"
       " -l <plpid>     Set how many PLPS there are\n");
    SAMPLE_DVBS_T2MI_PRINT("example: %s -t 0 -f 3000 -s 27500 -k 0 -p 0 -d 4096 -l 1\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_DVBS_T2MI_PRINT("         %s -q  <exit> \n", name);
#endif
}

static void MT_DvbsT2MICmdTask(MT_HANDLE      hAvplay, mt_plpid_para_t  plp[32], MT_S32 total_num)
{
    MT_S32                  ret = 0;
    MT_S32                  i = 0;
    MT_S32                  s32ProgNum = 0;

    MT_S32                  progNum = 1;
    MT_S32                  input_progNum = 1;
#ifdef MT_SAMPLE_APP
    play_resource.s32ProgNum = progNum;
#endif
    MT_CHAR                 inputCmd[32] = { 0 };


    while(1)
    {
        (MT_VOID)MT_DvbsT2MIPrintMenu(total_num);
        /* get inputCmd*/
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_DVBS_T2MI_INFO_PRINT("prepare to exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {

            progNum = atoi(inputCmd);
            input_progNum = progNum;

            if(progNum <= total_num)
            {
                for(i = 0; i< g_stDvbsT2miRunInfo.plp_num; i++)
                {
                    s32ProgNum = progNum - (plp[i].program_num);
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
                SAMPLE_DVBS_T2MI_ERR_PRINT(" prog_num the biggest is %d \n\n", total_num);
            }
            (MT_VOID)MT_UNF_DMX_T2MISetPlpid(i);
            /** Stop AV playback into the stop state */
            ret = MT_DvbsT2MIStopToPlay(hAvplay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_T2MI_ERR_PRINT(" MT_DvbsT2MIStopToPlay failed.\n");
            }
            SAMPLE_DVBS_T2MI_INFO_PRINT("Start play ProgNum: %d \n", progNum);
            // restore ac4    attr info.
            MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);

            /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
            ret = MT_DvbsT2MIStarToPlay(hAvplay, plp[i].proginfo + progNum - 1);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_T2MI_ERR_PRINT(" SwitchProg failed.\n");
                return;
            }
#ifdef MT_SAMPLE_APP
            play_resource.s32ProgNum = input_progNum;
#endif
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_DVBS_T2MI_INFO_PRINT("Dvbs play in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_DVBS_T2MI_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
static mt_s32 MT_DvbsT2MIParase_args(int argc, char *argv[], mt_input_Dvbs_para_t *pInutParam)
{
    int opt = 0;
    /** example: ./sample_dvbs -f 3840 -s 27500 -k 1 -p 0 -d 4096 -l 0 */

    SAMPLE_DVBS_T2MI_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, "?hHf:s:k:p:d:l:t:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_DvbsT2MIPrint_help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DvbsT2MIExit();
                }
                return MT_TASK_EXIT;

            case 'f':
                pInutParam->freq = strtol(mt_optarg, 0, 0);
                break;

            case 's':
                pInutParam->sym_rate = strtol(mt_optarg, 0, 0);
                break;

            case 'k':
                pInutParam->onoff_22k = strtol(mt_optarg, 0, 0);
                break;

            case 'p':
                pInutParam->polar = strtol(mt_optarg, 0, 0);
                break;

            case 'd':
                pInutParam->pid = strtol(mt_optarg, 0, 0);
                break;
            case 'l':
                pInutParam->plpid = strtol(mt_optarg, 0, 0);
                break;
            case 't':
                pInutParam->tuner_id = strtol(mt_optarg, 0, 0);
                break;

            default:
                (void)MT_DvbsT2MIPrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_DVBS_T2MI_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_Dvbs_t2miMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = 0;
    MT_S32 j = 0;
    MT_S32 i = 0;
    MT_S32 k = -1;
    static mt_input_Dvbs_para_t    sInputParam = {0};
    PMT_COMPACT_TBL *progTbl =  { 0 };
    PMT_COMPACT_PROG   *stCurrentProgInfo = { 0 };
    MT_UNF_DMX_PORT_E portId;

    SAMPLE_DVBS_T2MI_FUNCTION_ENTER();

    if(argc != 15 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DvbsT2MIPrint_help(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_DvbsT2MIParase_args(argc, argv, &sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        ret = MT_DvbsT2MICheckParam(&sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MT_DvbsT2MICheckParam failed.\n");
            return MT_FAILURE;
        }

#ifndef MT_SAMPLE_APP

        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("mt_sys_init error. ret=0x%x \n", ret);
            return MT_FAILURE;
        }
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", ret);
            return MT_FAILURE;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", ret);
            goto ERR1;
        }
#endif

        /** Tuner initialization, Set the default parameters for tuner */
        ret = MTADP_Fe_Init(sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_Fe_Init failed, ret = %x\n", ret);
            goto ERR2;
        }

        ret = MTADP_Fe_Connect_Dvbs(sInputParam.tuner_id, sInputParam.freq, sInputParam.sym_rate, sInputParam.onoff_22k, sInputParam.polar, MT_UNF_PORT_TYPE_DVBS2);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_Fe_Connect_Dvbs error\n");
            goto ERR2;
        }


        /** VO device initialization */
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_VO_Init failed, ret = %x\n", ret);
            goto ERR3;
        }

        /** Audio device initialization */
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_Snd_Init failed, ret = %x\n", ret);
            goto ERR4;
        }

        /** Demux initializes and retrieves the PMT and PAT tables in TS */
        ret = MT_DvbsT2MIDmxInit(sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MT_DvbcDmxInit failed, ret = %x\n", ret);
            goto ERR5;
        }

        (MT_VOID)MT_UNF_DMX_T2MISoftReset();

        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        mt_sys_version_s       stSysChipInfo = { 0 };

        /** Obtain the chip model */
        memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        ret = mt_sys_get_version(&stSysChipInfo);

        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            if(sInputParam.tuner_id == 0)
            {
                portId = DMX_DVB_TSI_IN_PORT;
            }
            else if(sInputParam.tuner_id == 1)
            {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
                portId = MT_UNF_DMX_PORT_TSI_3;
#else
                portId = MT_UNF_DMX_PORT_TSI_1;
#endif
            }
            else
            {
                portId = DMX_DVB_TSI_IN_PORT;
            }
        }
        else
        {
            portId = MT_UNF_DMX_PORT_TSI_1;
        }

        (MT_VOID)MT_UNF_DMX_T2MIEnable(MT_TRUE);
        MT_UNF_DMX_T2MISetInCh(portId);
        MT_UNF_DMX_T2MISetOutCh(portId);
        MT_UNF_DMX_T2MISetPid(sInputParam.pid);


        SAMPLE_DVBS_T2MI_INFO_PRINT("set pid is %d\n ", sInputParam.pid);

        for(i = 0; i<sInputParam.plpid; i++)
        {
            MT_UNF_DMX_T2MISetPlpid(i);


            SAMPLE_DVBS_T2MI_INFO_PRINT("set plpid is %d\n ", i);
            /** Get the PMT table */
            ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);
            if(MT_SUCCESS != ret)
            {

                SAMPLE_DVBS_T2MI_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
                continue;
            }
            k += 1;
            g_stDvbsT2miRunInfo.plp[k].program_num = progTbl->prog_num;
            for(j = 0; j<progTbl->prog_num; j++)
            {
                stCurrentProgInfo = progTbl->proginfo + j;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].VElementNum = stCurrentProgInfo->VElementNum;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].AElementNum = stCurrentProgInfo->AElementNum;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].PcrPid = stCurrentProgInfo->PcrPid;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].VElementPid = stCurrentProgInfo->VElementPid;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].VideoType = stCurrentProgInfo->VideoType;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].AElementPid = stCurrentProgInfo->AElementPid;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].AudioType = stCurrentProgInfo->AudioType;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].PmtPid = stCurrentProgInfo->PmtPid;
                g_stDvbsT2miRunInfo.plp[k].proginfo[j].ProgID = stCurrentProgInfo->ProgID;
            }


            (MT_VOID)MTADP_Search_FreeAllPmt(progTbl);
            g_stDvbsT2miRunInfo.total_num += g_stDvbsT2miRunInfo.plp[k].program_num;

        }

        g_stDvbsT2miRunInfo.plp_num = k + 1;

        (MT_VOID)MT_UNF_DMX_T2MISetPlpid(0);

        ret = MT_DvbsT2MIAvplayInit(&g_stDvbsT2miRunInfo.hAvPlay, &g_stDvbsT2miRunInfo.hWin, &g_stDvbsT2miRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {

            SAMPLE_DVBS_T2MI_ERR_PRINT("MT_DvbsT2MIAvplayInit failed.\n");
            goto ERR7;
        }

#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_stDvbsT2miRunInfo.hAvPlay;
        avplayHandle.hSoundTrack = g_stDvbsT2miRunInfo.hSoundTrack;
        avplayHandle.hWin = g_stDvbsT2miRunInfo.hWin;
#endif

        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        ret = MT_DvbsT2MIStarToPlay(g_stDvbsT2miRunInfo.hAvPlay, g_stDvbsT2miRunInfo.plp[0].proginfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_T2MI_ERR_PRINT("MT_DvbsT2MIStarToPlay failed.\n");
            goto ERR8;
        }
        g_bTaskQuit = MT_FALSE;
    }


    (void)MT_DvbsT2MICmdTask(g_stDvbsT2miRunInfo.hAvPlay, g_stDvbsT2miRunInfo.plp, g_stDvbsT2miRunInfo.total_num);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
    /** Stop AV playback and enter the stop state */
    ret = MT_DvbsT2MIStopToPlay(g_stDvbsT2miRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_T2MI_ERR_PRINT("MT_DvbsT2MIStopToPlay failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
    }


ERR8:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_DvbsT2MIAvplayDeInit(g_stDvbsT2miRunInfo.hAvPlay, g_stDvbsT2miRunInfo.hWin, g_stDvbsT2miRunInfo.hSoundTrack);


ERR7:

    (MT_VOID)MTADP_Search_DeInit();
    /** Demux module deinitialization */
    (MT_VOID)MT_DvbsT2MIDmxDeInit();
ERR5:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR3:
    /** Disconnect the tuner lock */
    (MT_VOID)MTADP_Fe_DeInit(sInputParam.tuner_id);

ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
#endif

    memset(&g_stDvbsT2miRunInfo, 0, sizeof(g_stDvbsT2miRunInfo));

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
#endif

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}

