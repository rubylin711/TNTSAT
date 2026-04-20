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
#include "mt_adp_hdmi.h"
#include "mt_adp_demux.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_UNICABLE_DEBUG

#define MT_UNICABLE_PRINT   printf
#else

#define MT_UNICABLE_PRINT

#endif

#define SAMPLE_UNICABLE_FUNCTION_ENTER()    MT_UNICABLE_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_UNICABLE_FUNCTION_EXIT()     MT_UNICABLE_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_UNICABLE_FATAL_PRINT(fmt...)         MT_UNICABLE_PRINT(" [FATAL] " fmt)
#define SAMPLE_UNICABLE_ERR_PRINT(fmt...)           MT_UNICABLE_PRINT(" [ERROR] " fmt)
#define SAMPLE_UNICABLE_WAEN_PRINT(fmt...)          MT_UNICABLE_PRINT(" [WARN] "  fmt)
#define SAMPLE_UNICABLE_INFO_PRINT(fmt...)          MT_UNICABLE_PRINT(" [INFO] "  fmt)
#define SAMPLE_UNICABLE_DBG_PRINT(fmt...)           MT_UNICABLE_PRINT(" [DEBUG] " fmt)


#define  SAMPLE_UNICABLE_PRINT  printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MAX_TP_NUM          64

/*************************** Structure Definition ****************************/
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
    mt_u32 freq;
    mt_u32 symbol_rate;
    mt_u32 program_num;
    PMT_COMPACT_info_t proginfo[64];
    mt_u8 dvb_type;
    mt_u32 polar;

} mt_tpinfo_para_t;

typedef struct
{
    mt_u32 tuner_id;
    mt_u32 ub_freq;
    mt_s32 unicable_version;
    mt_s32 scrnum;
    mt_u32 onoff22k;
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
	mt_u32			   tpIndex;
} MT_Split_RUN_INFO;


typedef struct
{
    mt_s32 s32TPNum;
    mt_s32 total_num;
    mt_input_Blindscan_para_t  sInputParam;
    mt_unf_fe_sat_tpinfo_t satTPinfo[MAX_TP_NUM];
    mt_blindscan_status_t bs_status;
    MT_Split_RUN_INFO stSplitRunInfo;
    mt_tpinfo_para_t tppara[MAX_TP_NUM];
}mt_TP_info_para_t;


/********************** Global Variable declaration **************************/
static MT_BOOL    g_bTaskQuit = MT_TRUE;
static mt_TP_info_para_t g_sTPinfo;

#ifdef MT_SAMPLE_APP
    MT_S32 MT_UnicableMain(MT_S32 argc, MT_CHAR *argv[]);
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_UnicableDmxInit(mt_u32 tuner_id)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    /** Initializes the demux module */
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return ret;
    }

    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        if(tuner_id == 0)
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
            SAMPLE_UNICABLE_INFO_PRINT("Connect port 0!\n");
        }
        else if(tuner_id == 1)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            /** Bind Demux to tuner port 3 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_3);
            SAMPLE_UNICABLE_INFO_PRINT("Connect port 3!\n");
#else
            /** Bind Demux to tuner port 1 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            SAMPLE_UNICABLE_INFO_PRINT("Connect port 1!\n");
#endif
        }
        else
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_UNICABLE_INFO_PRINT("Connect port 0!\n");
        }
    }
    else
    {
        /** Bind Demux to tuner port 0 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        SAMPLE_UNICABLE_INFO_PRINT("Connect port 1!\n");
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_UnicableDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}



static MT_S32 MT_Unicable_Notify(MT_U32 tuner_id, mt_unf_fe_blindscan_evt_t enEVT, mt_unf_fe_blindscan_notify_t *punNotify)
{
  mt_s32 i = 0;
  mt_s32 num = 0;
  mt_unf_fe_sat_tpinfo_t temp = { 0 };



  switch (enEVT)
  {
    case MT_UNF_FE_BLINDSCAN_EVT_STATUS:
        if(MT_UNF_FE_BLINDSCAN_STATUS_FAIL == *(punNotify->status))
        {
            SAMPLE_UNICABLE_ERR_PRINT("Scan fail.\n");
            return MT_FAILURE;
        }
        else if ((MT_UNF_FE_BLINDSCAN_STATUS_FINISH == *(punNotify->status)) || (MT_UNF_FE_BLINDSCAN_STATUS_QUIT == *(punNotify->status)))
        {
            SAMPLE_UNICABLE_PRINT("100%%");
            SAMPLE_UNICABLE_PRINT("done\n");
            SAMPLE_UNICABLE_INFO_PRINT("Scan over, find %d TP.\n", g_sTPinfo.s32TPNum);
            g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_FINISH;

            return MT_SUCCESS;

        }
        break;

    case MT_UNF_FE_BLINDSCAN_EVT_PROGRESS:

        SAMPLE_UNICABLE_PRINT("%d%% \n", *(punNotify->progress_percent));
        break;

    case MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT:
        num++;
        temp = *(punNotify->result);
        SAMPLE_UNICABLE_PRINT("\t%03d %7d %5d %d %d, %d TS\n", num, temp.freq, temp.symbol_rate, temp.polar, temp.code_rate, temp.DataTsNumber);

        for (i = 0; i < temp.DataTsNumber; i++)
        {
            SAMPLE_UNICABLE_PRINT("TS[%d] ---- 0x%02x\n", i, temp.DataTsIdArray[i]);
            (MT_VOID)mt_unf_fe_set_s2_multi_stream_ts_id(tuner_id, temp.DataTsIdArray[i]);

            MT_USLEEP(200000);
        }

        g_sTPinfo.satTPinfo[g_sTPinfo.s32TPNum] = *(punNotify->result);
        g_sTPinfo.s32TPNum++;
        break;

    default:
        break;
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
static mt_s32 MT_Unicable_Set_Parameter(MT_U32 tuner_id, MT_U32 sig_type, MT_U32 tuner_dev_type, MT_U32 tuner_addr,
             MT_U32 demod_dev_type, MT_U32 demod_addr, MT_U32 out_put_mode, MT_U32 I2c_channel)
{
    MT_S32           ret = 0;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };
    mt_sys_version_s stSysChipInfo = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("mt_sys_get_version err!\n");
        return ret;
    }

    SAMPLE_UNICABLE_INFO_PRINT("MTCommand_Tuner_Operation:   chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id,&mtTunerAttr);
    if(SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("mt_unf_fe_get_default_attr err!\n");
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

    SAMPLE_UNICABLE_INFO_PRINT("tuner_type[%d], tuner_addr[0x%x], demod_type[%d], demod_addr[0x%x], demod_id[%d], output_mode[%d] \n",
                            mtTunerAttr.tuner_type, mtTunerAttr.tuner_addr,
                            mtTunerAttr.demod_dev_type, mtTunerAttr.demod_addr,
                            mtTunerAttr.demod_i2c_id, mtTunerAttr.output_mode );

    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("mt_unf_fe_set_attr failed.ret is %d\n",ret);
        return ret;
    }

    SAMPLE_UNICABLE_INFO_PRINT("output_mode = %d\n", out_put_mode);

    return MT_SUCCESS;
}


static MT_S32 MT_Unicable_Blindscan(mt_u32 tuner_id, MT_U8 polar)
{
    MT_S32 ret = 0;
    mt_unf_fe_blindscan_para_t stBlindScanPara = { 0 };
    g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
#ifdef MT_SYM4_DSS
    ret = MT_Unicable_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88TS6011, 0x58, MT_UNF_DEMOD_DEV_TYPE_M88DS6113, 0xD2, 1, 1);
#else
    ret = MT_Unicable_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 1, 0);
#endif
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    if (tuner_id == 0)
    {
        ret = MT_Unicable_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 4, 0);
    }
    else
    {
        ret = MT_Unicable_Set_Parameter(tuner_id, 2048, 80, 90, 336, 210, 4, 1);
    }
#endif
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("MT_Unicable_Set_Parameter err \n");
        return ret;
    }

    memset(&stBlindScanPara, 0, sizeof(mt_unf_fe_blindscan_para_t));
    stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_MANUAL;
    stBlindScanPara.scan_para.sat.polar = polar;
    stBlindScanPara.scan_para.sat.lnb_22k = g_sTPinfo.sInputParam.onoff22k;
    stBlindScanPara.scan_para.sat.start_freq = 950 * 1000;
    stBlindScanPara.scan_para.sat.stop_freq = 2150 * 1000;
    stBlindScanPara.scan_para.sat.diseqc_set = MT_NULL;
    stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))MT_Unicable_Notify;
    stBlindScanPara.scan_para.sat.uc_param.use_uc = 1;
    stBlindScanPara.scan_para.sat.uc_param.ub_freq_mhz = g_sTPinfo.sInputParam.ub_freq;
    stBlindScanPara.scan_para.sat.uc_param.ub_ver = g_sTPinfo.sInputParam.unicable_version;
    ret = mt_unf_fe_blindscan_start(tuner_id, &stBlindScanPara);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call mt_unf_fe_blindscan_start failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}



/*!
@brief audio and video player initialization.
@param[out] phAvplay            Handle to AV player
@param[out] hWin                The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_UnicableAvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_UNICABLE_FUNCTION_ENTER();

    if(NULL == phAvplay)
    {
        SAMPLE_UNICABLE_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phWin)
    {
        SAMPLE_UNICABLE_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phSoundTrack)
    {
        SAMPLE_UNICABLE_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        return ret;

    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
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
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }
    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_UNICABLE_FUNCTION_EXIT();

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
static void  MT_UnicableAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
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
static MT_S32 MT_UnicableSetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_info_t *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };
    SAMPLE_UNICABLE_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_UNICABLE_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_UNICABLE_ERR_PRINT("=====pProgInfo == NULL=====\n");
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

    SAMPLE_UNICABLE_INFO_PRINT("VidPid=%x, Vidtype=0x%x, AudPid=%x, AudType=0x%x \n", VidPid, enVidType, AudPid, u32AudType);

    /** Get the audio properties of the AV player */

    if(VidPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
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
            SAMPLE_UNICABLE_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("Set video properties or video PID property failed.\n");
            return ret;
        }
    }


    if(AudPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }
        /* PCM decoding mode*/
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("Setting the decoding mode or audio PID property failed:%#x\n",ret);
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
            SAMPLE_UNICABLE_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            return ret;
        }
    }

    SAMPLE_UNICABLE_FUNCTION_EXIT();


    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_UnicableStarToPlay(mt_handle hAvplay, const PMT_COMPACT_info_t *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_UNICABLE_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_UNICABLE_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_UNICABLE_ERR_PRINT("hAvplay is not exist\n");
        return ret;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_UnicableSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableSetAvplayPidAndCodecType fail! \n");
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
        SAMPLE_UNICABLE_INFO_PRINT("Has no audio stream!\n");
    }

    /** Get the video PID properties of AV player */
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_UNICABLE_INFO_PRINT("Has no video stream!\n");
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
            SAMPLE_UNICABLE_ERR_PRINT("Set frame to VO fail.\n");
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("Get avplay sync attr fail!\n");
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
            SAMPLE_UNICABLE_ERR_PRINT("Set avplay sync attr fail!\n");
            return ret;
        }
    }

    /*start to play audio and video*/
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("MT_UNF_AVPLAY_Start fail!  ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_UNICABLE_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state.
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_UnicableStopToPlay(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;

    SAMPLE_UNICABLE_INFO_PRINT("stop live play ...\n");

    /*stop playing audio and video*/
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}


static MT_VOID MT_UnicablePrintMenu(MT_U32 prog_num)
{

    SAMPLE_UNICABLE_PRINT("\n 1 - %d : select the program \n", prog_num);
    SAMPLE_UNICABLE_PRINT("     p : print all tp program info \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_UNICABLE_PRINT("     b : background run \n");
#endif
    SAMPLE_UNICABLE_PRINT("     h : help \n");
    SAMPLE_UNICABLE_PRINT("     q : quit \n");
    SAMPLE_UNICABLE_PRINT("Unicable>> ");

}
static MT_VOID MT_UnicablePrintProgramInfo( mt_tpinfo_para_t tppara[32])
{
    MT_S32  i = 0;
    MT_S32  j = 0;
    MT_S32  k = 0;

    int n = 1;
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


    for(k = 0;k<g_sTPinfo.s32TPNum;k++)
    {
        SAMPLE_UNICABLE_PRINT("<Index>          <Freq>           <Sym Rate>         <Type>      <Polar>\n");
        SAMPLE_UNICABLE_PRINT(" %3d         %8dKHz     %10dKSs           %5s         %d\n", k+1, tppara[k].freq, tppara[k].symbol_rate, dvb_type[k], tppara[k].polar);
        for(i = 0; i<tppara[k].program_num;i++)
        {
            SAMPLE_UNICABLE_PRINT("Channel Num = %d, Program ServiceID = %d PMT PID = %x\n", i+1, tppara[k].proginfo[i].ProgID,tppara[k].proginfo[i].PmtPid);

             for (j = 0; j < tppara[k].proginfo[i].VElementNum;j++)
             {
                 SAMPLE_UNICABLE_PRINT("\tVideo Stream PID   = 0x%x\n",tppara[k].proginfo[i].VElementPid);
                 switch (tppara[k].proginfo[i].VideoType)
                 {
                    case MT_UNF_VCODEC_TYPE_H264:
                        SAMPLE_UNICABLE_PRINT("\tVideo Stream Type H264\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG2:
                        SAMPLE_UNICABLE_PRINT("\tVideo Stream Type MP2\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG4:
                        SAMPLE_UNICABLE_PRINT("\tVideo Stream Type MP4\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_HEVC:
                        SAMPLE_UNICABLE_PRINT("\tVideo Stream Type HEVC\n");
                        break;
                    default:
                        SAMPLE_UNICABLE_PRINT("\tVideo Stream Type error\n");
                 }
             }
             for (j = 0; j < tppara[k].proginfo[i].AElementNum; j++)
             {
                SAMPLE_UNICABLE_PRINT("\tAudio Stream PID   = 0x%x\n", tppara[k].proginfo[i].AElementPid);

                switch (tppara[k].proginfo[i].AudioType)
                {
                    case HA_AUDIO_ID_MP3:
                        SAMPLE_UNICABLE_PRINT("\tAudio Stream Type MP3\n");
                        break;
                    case HA_AUDIO_ID_AAC:
                        SAMPLE_UNICABLE_PRINT("\tAudio Stream Type AAC\n");
                        break;
                    case HA_AUDIO_ID_DOLBY_PLUS:
                        SAMPLE_UNICABLE_PRINT("\tAudio Stream Type AC3\n");
                        break;
                    case HA_AUDIO_ID_DTSHD:
                        SAMPLE_UNICABLE_PRINT("\tAudio Stream Type DTS\n");
                        break;
                    default:
                        SAMPLE_UNICABLE_PRINT("\tAudio Stream Type error\n");
                 }
             }
             SAMPLE_UNICABLE_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        }

    }
    for(i = 0; i<g_sTPinfo.s32TPNum; i++)
    {
        SAMPLE_UNICABLE_PRINT("TP:%2d ( %2d - %2d) freq:%4dKHZ symbol_rate:%5dKSs \n", i+1, n, tppara[i].program_num + n-1, tppara[i].freq, tppara[i].symbol_rate);

         n = n + tppara[i].program_num;
    }
}

static MT_VOID MT_Unicable_Set_Lnb(mt_u32 tuner_id, MT_U8 unicable_port_no, MT_U8 unicable_scr_no, MT_U32 unicable_if_freq_mhz, MT_U8 unicable_version)
{
    mt_unf_fe_lnb_config_t lnb_config = { 0 };
    mt_s32 ret = 0;

    lnb_config.lnb_type = MT_UNF_FE_LNB_UNICABLE; //enType;
    if (g_sTPinfo.sInputParam.onoff22k == 1)
    {
        lnb_config.low_lo = 9750;   //u32LowLOFreq;
        lnb_config.high_lo = 10600; //u32HighLOFreq;
    }
    else
    {
        lnb_config.low_lo = 9750;   //u32LowLOFreq;
        lnb_config.high_lo = 9750; //u32HighLOFreq;
    }

    lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_KU;
    lnb_config.unicable_scr_no = unicable_scr_no;           //u32SCRNO;
    lnb_config.unicable_if_freq_mhz = unicable_if_freq_mhz; //u32IFCenterFreq_MHz;
    lnb_config.unicable_port_no = unicable_port_no;         //enSatPosn;
    lnb_config.unicable_ver_no = unicable_version;
    lnb_config.lnb_agent_mode = 0;
    lnb_config.lnb_agent_id = 0;
    ret = mt_unf_fe_set_lnb_config(tuner_id, &lnb_config);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("call mt_unf_fe_set_lnb_config failed.\n");
    }
}


static MT_S32 MT_UnicableConnect_Dvbs_Unicable(MT_U32 tuner_id,
                                            MT_U32 freq,
                                            MT_U32 sym_rate,
                                            MT_U32 onoff_22k,
                                            MT_U32 polar,
                                            MT_U32 unicable_port_no,
                                            MT_U32 user_band,
                                            MT_U32 ub_freq_mhz,
                                            MT_U32 u32LoopTimes,
                                            MT_U32 ub_ver)
{
      MT_S32 ret = MT_FAILURE;
      MT_U32 u32Loop = 0;
      MT_U32 u32Freq = 0;
      MT_U32 u32SymbolRate = 0;
      mt_unf_fe_status_t stTunerStatus = { 0 };
      mt_unf_fe_connect_para_t s_stConnectPara = { 0 };


      s_stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
      s_stConnectPara.connect_param.sat.freq = freq * 1000;
      s_stConnectPara.connect_param.sat.sym_rate = sym_rate;
      s_stConnectPara.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
      s_stConnectPara.connect_param.sat.onoff_22k = onoff_22k;
      s_stConnectPara.connect_param.sat.polarization = polar;

      s_stConnectPara.connect_param.sat.uc_param.use_uc = 1;

      if (MT_UNF_FE_SATPOSN_A == unicable_port_no)
      {
            if (MT_UNF_FE_POLARIZATION_V == polar)
            {
                s_stConnectPara.connect_param.sat.uc_param.bank = onoff_22k ? 1 : 0;
            }
            else
            {
                s_stConnectPara.connect_param.sat.uc_param.bank = onoff_22k ? 3 : 2;
            }
      }
      else
      {
            if (MT_UNF_FE_POLARIZATION_V == polar)
            {
                s_stConnectPara.connect_param.sat.uc_param.bank = onoff_22k ? 5 : 4;
            }
            else
            {
                s_stConnectPara.connect_param.sat.uc_param.bank = onoff_22k ? 7 : 6;
            }
      }

      s_stConnectPara.connect_param.sat.uc_param.ub_freq_mhz = ub_freq_mhz;
      s_stConnectPara.connect_param.sat.uc_param.user_band = user_band;
      s_stConnectPara.connect_param.sat.uc_param.ub_ver = ub_ver;


      ret = mt_unf_fe_connect(tuner_id, &s_stConnectPara, 2000);
      u32Freq = s_stConnectPara.connect_param.sat.freq;
      u32SymbolRate = s_stConnectPara.connect_param.sat.sym_rate;

    if (MT_SUCCESS == ret)
    {
        if (u32LoopTimes == 0)
        {

            u32LoopTimes = s_stConnectPara.channel_set_info.lock_time / 10;
        }

        for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++)
        {
            ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
            if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
            {
                SAMPLE_UNICABLE_INFO_PRINT("Tuner Lock freq %d symb %d polar%d Success!\n", u32Freq, u32SymbolRate, polar);
                SAMPLE_UNICABLE_INFO_PRINT("SUCCESS end\n");
                return MT_SUCCESS;
            }
            else
            {
                MT_USLEEP(10000);

            }
        }
    }
    else
    {
        SAMPLE_UNICABLE_ERR_PRINT("Tuner Lock freq %d symb %d polar%d mt_unf_fe_connect Fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, polar, ret);
    }

    if (u32Loop == u32LoopTimes)
    {
        SAMPLE_UNICABLE_ERR_PRINT("Tuner Lock freq %d symb %d  polar%d Fail!\n", u32Freq, u32SymbolRate, polar);
    }

    SAMPLE_UNICABLE_ERR_PRINT("FAIL end\n");

    return MT_FAILURE;
}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_UnicablePrint_help(char *name)
{
    SAMPLE_UNICABLE_PRINT("Options:\n");
    SAMPLE_UNICABLE_PRINT(" %s -t tuner_id -u ub_freq -i scr -v version(1/2) -k 22k\n", name);
    SAMPLE_UNICABLE_PRINT(" %s -t 0 -u 1210 -i 0 -v 1 -k 1\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_UNICABLE_PRINT(" %s -q  <exit> \n", name);
#endif

}

static MT_VOID MT_UnicableExit(void)
{
    (MT_VOID)MT_UnicableStopToPlay(g_sTPinfo.stSplitRunInfo.hAvPlay);

    (MT_VOID)MT_UnicableAvplayDeInit(g_sTPinfo.stSplitRunInfo.hAvPlay, g_sTPinfo.stSplitRunInfo.hWin, g_sTPinfo.stSplitRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_DeInit();

    (MT_VOID)MT_UnicableDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Fe_DeInit(g_sTPinfo.sInputParam.tuner_id);

    memset(&g_sTPinfo, 0, sizeof(mt_TP_info_para_t));
    g_bTaskQuit = MT_TRUE;
}


static void MT_UnicableCmdTask(MT_HANDLE        hAvplay, MT_S32 total_num, mt_tpinfo_para_t tppara[32], mt_input_Blindscan_para_t *pInutParam)
{
    MT_S32 ret = 0;
    MT_S32 i = 0;
    MT_S32 progNum = 0;
    MT_S32 s32ProgNum = 0;
    MT_CHAR inputCmd[32] = { 0 };

    while(1)
    {
        (MT_VOID)MT_UnicablePrintMenu(total_num);
        /* get inputCmd*/
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_UNICABLE_INFO_PRINT("prepare to exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_UNICABLE_INFO_PRINT("Unicable play in back!\n");
            break;
        }
#endif
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            progNum = atoi(inputCmd);
            if(progNum <= total_num)
            {
                for(i = 0; i < g_sTPinfo.s32TPNum; i++)
                {
                    s32ProgNum = progNum - (tppara[i].program_num);
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
                SAMPLE_UNICABLE_INFO_PRINT("The biggest num is %d \n", total_num);
                continue;
            }

            if(g_sTPinfo.stSplitRunInfo.tpIndex != i)
            {
                ret = MT_UnicableConnect_Dvbs_Unicable(pInutParam->tuner_id, g_sTPinfo.tppara[i].freq, g_sTPinfo.tppara[i].symbol_rate,
                    g_sTPinfo.sInputParam.onoff22k, g_sTPinfo.tppara[i].polar, MT_UNF_FE_SATPOSN_A, g_sTPinfo.sInputParam.scrnum,
                    g_sTPinfo.sInputParam.ub_freq, 100, g_sTPinfo.sInputParam.unicable_version);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableConnect_Dvbs_Unicable failed.\n");
                }
                g_sTPinfo.stSplitRunInfo.tpIndex = i;
            }
            ret = MT_UnicableStopToPlay(hAvplay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableStopToPlay failed.\n");
            }
            ret = MT_UnicableStarToPlay(hAvplay, tppara[i].proginfo + progNum - 1);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableStarToPlay failed.\n");
            }


        }
        else if('p' == inputCmd[0])
        {
            (MT_VOID)MT_UnicablePrintProgramInfo(tppara);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_UNICABLE_INFO_PRINT("Print Help info \n");
        }

    }
}


/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
    static mt_s32 MT_UnicableParase_args(int argc, char *argv[], mt_input_Blindscan_para_t *pInutParam)
    {
        int opt = 0;

        while((opt = MTADP_Getopt(argc, argv, ":?hH:u:i:v:k:t:q")) != -1)
        {
            switch(opt)
            {
                case 'h':
                case '?':
                case 'H':
                    (void)MT_UnicablePrint_help(argv[0]);
                    return MT_FAILURE;
                case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_UnicableExit();
                }
                return MT_TASK_EXIT;

                case 't':
                    pInutParam->tuner_id = strtol(mt_optarg, 0, 0);
                    break;
                case 'u':
                    pInutParam->ub_freq = strtol(mt_optarg, 0, 0);
                    break;
                case 'i':
                    pInutParam->scrnum = strtol(mt_optarg, 0, 0);
                    break;
                case 'v':
                    pInutParam->unicable_version = strtol(mt_optarg, 0, 0);
                    break;
                case 'k':
                    pInutParam->onoff22k = strtol(mt_optarg, 0, 0);
                    break;
                default:
                    (void)MT_UnicablePrint_help(argv[0]);
                    return MT_FAILURE;
                break;
            }
        }
        return MT_SUCCESS;
    }



#ifdef MT_SAMPLE_APP
    MT_S32 MT_UnicableMain(MT_S32 argc, MT_CHAR *argv[])
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = 0;
    MT_S32 j = 0;
    MT_S32 i = 0;
    MT_S32 k = -1;
    MT_S32 n = 0;
    PMT_COMPACT_TBL *progTbl = MT_NULL;
    PMT_COMPACT_PROG   *stCurrentProgInfo = { 0 };


    if(argc != 11 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_UnicablePrint_help(argv[0]);
        return MT_SUCCESS;
    }
    ret = MT_UnicableParase_args(argc, argv, &g_sTPinfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }


    if(g_bTaskQuit == MT_TRUE)
    {

#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MT_SYS_Init failed.\n");
            return ret;
        }
#endif
        ret = MTADP_Fe_Init(g_sTPinfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MTADP_Fe_Init failed.\n");
            goto ERR1;
        }

        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_INIT;

#ifndef MT_SAMPLE_APP
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MTADP_HDMI_Init failed.\n");
            goto ERR2;
        }
        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MTADP_Disp_Init failed.\n");
            goto ERR3;

        }
#endif
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {

            SAMPLE_UNICABLE_ERR_PRINT("MTADP_VO_Init failed.\n");
            goto ERR4;
        }
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MTADP_Snd_Init failed.\n");
            goto ERR5;
        }

        ret = MT_UnicableDmxInit(g_sTPinfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableDmxInit failed.\n");
            goto ERR6;
        }

        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        (void)MT_Unicable_Set_Lnb(g_sTPinfo.sInputParam.tuner_id, MT_UNF_FE_SATPOSN_A, g_sTPinfo.sInputParam.scrnum, g_sTPinfo.sInputParam.ub_freq, g_sTPinfo.sInputParam.unicable_version);

        for(n = 0; n < 2; n++)
        {
            ret = MT_Unicable_Blindscan(g_sTPinfo.sInputParam.tuner_id, n);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_UNICABLE_ERR_PRINT("MT_Unicable_Blindscan failed.\n");
                goto ERR6;
            }
            while(MT_BLINDSCAN_STATUS_SCANNING == g_sTPinfo.bs_status)
            {
                usleep(5000);
            }

            for(i = 0; i < g_sTPinfo.s32TPNum; i++)
            {
                ret = MT_UnicableConnect_Dvbs_Unicable(g_sTPinfo.sInputParam.tuner_id, (g_sTPinfo.satTPinfo[i].freq)/1000,
                    (g_sTPinfo.satTPinfo[i].symbol_rate), g_sTPinfo.sInputParam.onoff22k, g_sTPinfo.satTPinfo[i].polar, MT_UNF_FE_SATPOSN_A,
                     g_sTPinfo.sInputParam.scrnum, g_sTPinfo.sInputParam.ub_freq, 1000, g_sTPinfo.sInputParam.unicable_version);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableConnect_Dvbs_Unicable failed.\n");
                    continue;
                }

                /** Get the PMT table */
                ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);

                if(MT_SUCCESS != ret)
                {
                    SAMPLE_UNICABLE_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
                    continue;
                }
                k++;
                printf("k = %d \n", k);
                for(j = 0; j<progTbl->prog_num; j++)
                {
                    stCurrentProgInfo = progTbl->proginfo + j;
                    g_sTPinfo.tppara[k].freq = (g_sTPinfo.satTPinfo[i].freq)/1000;
                    g_sTPinfo.tppara[k].symbol_rate = (g_sTPinfo.satTPinfo[i].symbol_rate);
                    g_sTPinfo.tppara[k].dvb_type = g_sTPinfo.satTPinfo[i].dvb_type;
                    g_sTPinfo.tppara[k].polar = g_sTPinfo.satTPinfo[i].polar;
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

                }

                (MT_VOID)MTADP_Search_FreeAllPmt(progTbl);
                g_sTPinfo.total_num += g_sTPinfo.tppara[k].program_num;
            }

            g_sTPinfo.s32TPNum = 0;
            memset(g_sTPinfo.satTPinfo, 0, sizeof(mt_unf_fe_sat_tpinfo_t)*MAX_TP_NUM);
        }

        g_sTPinfo.s32TPNum = k + 1;
        ret = MT_UnicableConnect_Dvbs_Unicable(g_sTPinfo.sInputParam.tuner_id, g_sTPinfo.tppara[0].freq, g_sTPinfo.tppara[0].symbol_rate, g_sTPinfo.sInputParam.onoff22k,
            g_sTPinfo.tppara[0].polar, MT_UNF_FE_SATPOSN_A, g_sTPinfo.sInputParam.scrnum , g_sTPinfo.sInputParam.ub_freq, 1000,
            g_sTPinfo.sInputParam.unicable_version);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableConnect_Dvbs_Unicable failed.\n");
            goto ERR8;
        }

        ret = MT_UnicableAvplayInit(&g_sTPinfo.stSplitRunInfo.hAvPlay, &g_sTPinfo.stSplitRunInfo.hWin, &g_sTPinfo.stSplitRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableAvplayInit failed.\n");
            goto ERR8;
        }

		g_sTPinfo.stSplitRunInfo.tpIndex = 0;
        ret = MT_UnicableStarToPlay(g_sTPinfo.stSplitRunInfo.hAvPlay, (g_sTPinfo.tppara[0].proginfo));
        if(MT_SUCCESS != ret)
        {

            SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableStarToPlay failed.\n");
            goto ERR9;
        }
        g_bTaskQuit = MT_FALSE;
    }

    (void)MT_UnicableCmdTask(g_sTPinfo.stSplitRunInfo.hAvPlay, g_sTPinfo.total_num, g_sTPinfo.tppara, &g_sTPinfo.sInputParam);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    /** Stop AV playback and enter the stop state */
    ret = MT_UnicableStopToPlay(g_sTPinfo.stSplitRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_UNICABLE_ERR_PRINT("MT_UnicableStopToPlay failed, ret = %d\n", ret);
    }


ERR9:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_UnicableAvplayDeInit(g_sTPinfo.stSplitRunInfo.hAvPlay, g_sTPinfo.stSplitRunInfo.hWin, g_sTPinfo.stSplitRunInfo.hSoundTrack);
ERR8:

    (MT_VOID)MTADP_Search_DeInit();

    /** Demux module deinitialization */
    (MT_VOID)MT_UnicableDmxDeInit();
ERR6:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

ERR5:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();

ERR4:
#ifndef MT_SAMPLE_APP
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();

ERR3:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
#endif

#ifndef MT_SAMPLE_APP
ERR2:
#endif
    /** Disconnect the tuner lock */
    (MT_VOID)MTADP_Fe_DeInit(g_sTPinfo.sInputParam.tuner_id);

ERR1:
#ifndef MT_SAMPLE_APP
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif

    memset(&g_sTPinfo, 0, sizeof(mt_TP_info_para_t));
    return ret;


}

