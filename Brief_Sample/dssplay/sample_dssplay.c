/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <fcntl.h>
#include <unistd.h>
#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_demux.h"
#include "mt_unf_descrambler.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecs.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_mpi_demux.h"
#include "mt_adp_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_demux.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
#include "mt_cmdline.h"




/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DSSPLAY_DEBUG

#define MT_DSSPLAY_PRINT   printf
#else

#define MT_DSSPLAY_PRINT

#endif

#define SAMPLE_DSSPLAY_FUNCTION_ENTER()     MT_DSSPLAY_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DSSPLAY_FUNCTION_EXIT()      MT_DSSPLAY_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DSSPLAY_FATAL_PRINT(fmt...)      MT_DSSPLAY_PRINT(" [FATAL] " fmt)
#define SAMPLE_DSSPLAY_ERR_PRINT(fmt...)            MT_DSSPLAY_PRINT(" [ERROR] " fmt)
#define SAMPLE_DSSPLAY_WARN_PRINT(fmt...)       MT_DSSPLAY_PRINT(" [WARN] "  fmt)
#define SAMPLE_DSSPLAY_INFO_PRINT(fmt...)       MT_DSSPLAY_PRINT(" [INFO] "  fmt)
#define SAMPLE_DSSPLAY_DBG_PRINT(fmt...)            MT_DSSPLAY_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DSSPLAY_PRINT printf

#define TUNER_ID_0             0
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2
#define DMX_ID_0               0
#define INVALID_TSPID (0x1fff)

#define DSS_TS_PACKET_BUF_MAX       (130 * 5000)
#define READ_DSS_DATA_LEN           130*5
#define AUDIO_ES_1_READ_LEN         0x10000
#define V_ES_BUFFER_SIZE            0x100000
#define VIDEO_ES_MAX (120 * 5000)
#define AUDIO_ES_MAX (100 * 5000)
static pthread_mutex_t g_ESLock = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK()            do{pthread_mutex_lock(&g_ESLock);}while(0)
#define MUTEX_UNLOCK()          do{pthread_mutex_unlock(&g_ESLock);}while(0)
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
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
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
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
    MT_U8 port_type;
} mt_input_ter_para_t;
typedef struct
{
    MT_U8 file_name[256];
}mt_input_file_para_t;

typedef struct
{
    MT_INPUR_SIG_TYPE_T sig_type;
    MT_S32 data_source;
    union
    {
        mt_input_cab_para_t cab;
        mt_input_ter_para_t ter;
        mt_input_sat_para_t sat;
        mt_input_file_para_t file;
    } input_param;
    MT_U16             vidpid;
    MT_U16             audpid;
} mt_input_para_t;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hDmx;
    pthread_t          htsThd;
    mt_input_para_t    sInputParam;
    PMT_COMPACT_TBL    *pProgTbl;
    PMT_COMPACT_PROG   *pProgInfo;
} MT_DSS_PLAY_INFO;
typedef enum
{
    DSS_NULL_PACKET,
    DSS_RANGING_PACKET,
    DSS_AUXILIARY_DATA_PACKET,
    DSS_VIDEO_SERVICE_PACKET,
    DSS_AUDIO_SERVICE_PACKET,
    DSS_PROGRAM_GUIDE_PACKET,
    DSS_CONDITIONAL_ACCESS_PACKET,
    DSS_DATA_APPLICATION_PACKET,
    DSS_PACKET_UNKNOWN,
}dss_packet_type_t;

typedef enum
{
    DSS_ADG_RTS = 0x00,
    DSS_ADG_CWP = 0x01,
    DSS_ADG_RTS_CWP = 0x03,
    DSS_ADG_GOP_MAP_DATA = 0x04,
    DSS_ADG_SVP = 0x0B,/*001000b to 001011b SVP control data*/
    DSS_ADG_BROADBAND_VIDEO_DATA = 0x0C,
    DSS_ADG_RESERVED = 0x0D
}dss_auxiliary_data_group_type_t;struct dss_prefix_data
{
    u8 pf;/*Packet Framing 1bit This bit toggles between 0 and 1 with each packet*/
    u8 bb;/*Bundle Boundary 1bit 1:a sequence header or a picture header*/
    u8 cf;/*Control Flag 0: scrambled 1: not scrambled*/
    u8 cs;/*Control sync*/
    u16 scid;/*Service Channel ID 0x000 NULL packet,0xFEF to 0xFFF reserved*/
    u8 cc;/*Continuity Counter*/
    u8 hd;/*Header Designator*/
    dss_packet_type_t packet_type;
};

struct dss_auxiliary_data_prefix
{
    u8 mf;/*Modifiable Flag 1bit this bit is always set to 1*/
    u8 cff;/*Current field flag 1bit 0: not a vaild ADG,1: a vaild ADG*/
    u8 afid;/*Aux Field ID*/
    u8 afs;/*Auxiliary Field Size*/
};
struct dss_decoder
{
    struct dss_prefix_data prefix_data;
    struct dss_auxiliary_data_prefix aux_prefix_data;
    u16 packet_type[8];
    u8 payload[127];
    u16 hd[16];
    u16 video_scid_filter;
    u16 audio_scid_filter;
    u8 need_copy_length;
    u32 had_copy_video_data_length;
    u32 had_copy_auido_data_length;
};
typedef struct tag_dss_packet_heaer
{
  //packet_framing
  u32 pf : 1;
  //bundle_bundary
  u32 bb : 1;
  //control_flag, cf=1,packet not scrambled;cf=0,packet scrambled;
  u32 cf :1;
  //control_sync
  u32 cs : 1;
  //service_channel_id
  u32 scid : 12;
  //continuity_counter
  u32 cc : 4;
  //header_designator
  u32 hd : 4;
  //dss_packet_type_t
  u32 packet_type : 8;
}dss_packet_heaer_t;

typedef struct tag_local_buffer
{
  u32 max_len;
  u32 write_pos;
  u32 read_pos;
  u32 hold_len;
  u8 *p_buf;
}local_buffer_t;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static u8 g_video_pes_packet_start = 0;
static local_buffer_t g_audio_es = { 0 };
static local_buffer_t g_video_es = { 0 };
static mt_s32 g_run =0;
static local_buffer_t g_dss_ts_packet = { 0 };
static MT_DSS_PLAY_INFO    g_stDssPlayRunInfo = { 0 };

static u8 null_ranging_packet_payload[128] =
{
    4,9,180,6,149,240,167,88,169,6,
    78,175,172,129,134,185,162,181,137,118,
    8,149,57,198,147,97,2,83,64,38,
    41,20,48,124,121,26,179,128,88,113,
    223,82,75,112,18,242,249,172,112,199,
    /*214,50,93,159,218,180,223,65,141,123,*/ /*In spec payload[55]=180,but in fact is 189*/
    214,50,93,159,218,189,223,65,141,123,
    64,184,0,54,38,137,99,57,113,146,
    191,245,71,194,159,212,55,154,235,227,
    129,200,197,13,230,112,19,246,86,128,
    182,122,127,197,176,233,125,137,212,61,
    /*187,96,192,141,69,15,108,80,184,106,*/ /*In spec payload[107]=80,but in fact is 89*/
    187,96,192,141,69,15,108,89,184,106,
    159,231,224,157,197,198,57,60,134,61,
    11,218,100,50,214,95,53,184
};


/*!
@brief Check if the QAM matches
@param[in]  mod_type            QAM
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static MT_S32 MT_DssPlayCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_DSSPLAY_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Check if the DVBS param matches
@param[in]  p_sat_in            DVBS param
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static mt_s32 MT_DssPlayCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_DSSPLAY_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_DssPlayDmxInit(MT_VOID)
{
    mt_s32  ret = MT_SUCCESS;
    MT_UNF_DMX_CHAN_ATTR_S      stChnAttr;
    mt_sys_version_s stSysChipInfo;
    u32 tspes_pid = 0xa0;

    SAMPLE_DSSPLAY_FUNCTION_ENTER();

    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        return MT_FAILURE;
    }

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to mt_sys_get_version\n");
        return MT_FAILURE;
    }

    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
    }
    else
    {
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        (MT_VOID)MT_UNF_DMX_DeInit();
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_GetChannelDefaultAttr(&stChnAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_UNF_DMX_GetChannelDefaultAttr\n");
        return MT_FAILURE;
    }

    stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_DSS;

    stChnAttr.u32BufSize = (4*1024*1024);
    stChnAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    stChnAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    ret = MT_UNF_DMX_CreateChannel(DMX_ID_0, &stChnAttr, &g_stDssPlayRunInfo.hDmx);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_UNF_DMX_CreateChannel\n");
        return MT_FAILURE;
    }
    ret = MT_UNF_DMX_SetChannelPID(g_stDssPlayRunInfo.hDmx, tspes_pid);
    ret |= MT_UNF_DMX_OpenChannel(g_stDssPlayRunInfo.hDmx);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_UNF_DMX_OpenChannel\n");
        return MT_FAILURE;
    }

    SAMPLE_DSSPLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static void MT_DssPlayDmxDeInit(MT_VOID)
{
    (MT_VOID)MT_UNF_DMX_CloseChannel(g_stDssPlayRunInfo.hDmx);
    (MT_VOID)MT_UNF_DMX_DestroyChannel(g_stDssPlayRunInfo.hDmx);
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
static mt_s32 MT_DssPlayAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = 0;
    mt_handle   hWin = 0;
    mt_handle   hsoundTrack = 0;
    MT_UNF_VCODEC_TYPE_E VdecType = MT_UNF_VCODEC_TYPE_MPEG2;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };
    MT_UNF_AVPLAY_OPEN_OPT_S stMaxCapbility;

    if(NULL == p_hAvplay)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.stStreamAttr.u32VidBufSize = 0x600000;
    AvplayAttr.stStreamAttr.u32AudBufSize = 0x60000;
    AvplayAttr.stStreamAttr.u32MultiAudBufSize = 0x30000;
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stMaxCapbility);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_AVPLAY_Enable_AudioHEAAC(hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Enable_AudioHEAAC failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
 @return void

*/
static void  MT_DssPlayAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
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
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_DssPlayAVPlay_Start(mt_handle hAvplay)
{
    mt_u32 VidPid = 0;
    mt_u32 AudPid = 0;
    mt_u32 u32AudType = 0;
    mt_s32 ret = 0;
    MT_BOOL bAdvancedProfil = 1;
    MT_UNF_VCODEC_TYPE_E VdecType = MT_UNF_VCODEC_TYPE_MPEG2;
    mt_u32  u32CodecVersion = 8;
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0X00;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
        return ret;
    }

    VcodecAttr.enType = VdecType;
    VcodecAttr.u32ErrCover = 100;
    VcodecAttr.u32Priority = 3;
    VcodecAttr.u32UseDescInfoFlag = 0;
    VcodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
//    VcodecAttr.u32UseDescInfoFlag = 0;
    VcodecAttr.bDynamicResSupport = MT_TRUE;

    ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
        return ret;
    }

    ret = MTADP_AVPlay_SetVdecAttr(hAvplay, VdecType, MT_UNF_VCODEC_MODE_NORMAL);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MTADP_AVPlay_SetVdecAttr failed.ret = %#x\n",ret);
        return ret;
    }

    ret = MTADP_AVPlay_SetAdecAttr(hAvplay, HA_AUDIO_ID_MP3, HD_DEC_MODE_RAWPCM, 0);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
        return ret;
    }

    MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
        return ret;
    }
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
        return ret;
    }

    return MT_SUCCESS;
}

static MT_VOID dss_dump_data(char *p_name, const unsigned char *p_data, int length)
{
    unsigned int i;
    printf("\n");
    printf("begin dump [%s] data, length = [%d]: \n", p_name, length);
    for (i = 0; i < length; i++)
    {
        printf("%02x ", p_data[i]);
        if((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n\n");
}

static int find_start_code_index(const unsigned char *p_in)
{
    u8 i = 0;
    u8 data_start_index = 0;
    u8 stream_id = 0;
    do
    {
        if(i > 127)
        {
            break;
        }
        if((p_in[i] == 0x00) && (p_in[i + 1] == 0x00) && (p_in[i + 2] == 0x01))
        {
            stream_id = p_in[i + 3];
            if(stream_id < 0xBC)
            {
                data_start_index = i;
                break;
            }
            else
            {
                i += 3;
                data_start_index = i;
            }
        }
        else
        {
            i++;
        }
    }while(1);
    return data_start_index;
}

static MT_S32 is_null_or_ranging_packet(unsigned char *p_in)
{
    MT_S32 i = 0;
    for(i = 0; i < 128; i++)
    {
        if(null_ranging_packet_payload[i] == p_in[i])
        {
            continue;
        }
        else
        {
            printf("i:%d,%d != %d \n",i,null_ranging_packet_payload[i],p_in[i]);
            return 0;
        }
    }
    return 1;
}

static int find_value_in_array_u16(u16 *p_in,u16 array_size,u16 value)
{
    u16 i = 0;
    int ret = 0;
    for(i = 0; i < array_size; i++)
    {
        if(p_in[i] == value)
        {
            ret = 1;
            break;
        }
    }
    return ret;
}

static int parse_dss_auxiliary_packet(const unsigned char *p_in,struct dss_decoder *p_decoder)
{
    struct dss_auxiliary_data_prefix *p_aux_prefix_data = NULL;
    u8 tmp = 0;

    p_aux_prefix_data = (struct dss_auxiliary_data_prefix *)&p_decoder->aux_prefix_data;
    if(p_aux_prefix_data == NULL)
    {
        printf("[%s %d]p_aux_prefix_data is NULL,Please check \n",__FUNCTION__,__LINE__);
        return MT_FAILURE;
    }

    p_aux_prefix_data->mf = (p_in[0]&0x80) >> 7;

    p_aux_prefix_data->cff = (p_in[0]&0x40) >> 6;
    if(p_aux_prefix_data->cff == 0)
    {
        printf("[%s %d]Not a vaild ADG cff is %d,Please check \n", __FUNCTION__, __LINE__, p_aux_prefix_data->cff);
        return MT_FAILURE;
    }

    p_aux_prefix_data->cff = p_in[1];
    tmp = p_in[0] & 0x3F;
    switch(tmp)
    {
        case 0x00:
            p_aux_prefix_data->afid = DSS_ADG_RTS;
            break;
        case 0x01:
            p_aux_prefix_data->afid = DSS_ADG_CWP;
            break;
        case 0x02:
            p_aux_prefix_data->afid = DSS_ADG_RESERVED;
            break;
        case 0x03:
            p_aux_prefix_data->afid = DSS_ADG_RTS_CWP;
            break;
        case 0x04:
            p_aux_prefix_data->afid = DSS_ADG_GOP_MAP_DATA;
            break;
        case 0x05:
        case 0x06:
        case 0x07:
            p_aux_prefix_data->afid = DSS_ADG_RESERVED;
            break;
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
            p_aux_prefix_data->afid = DSS_ADG_SVP;
            break;
        case 0x0C:
            p_aux_prefix_data->afid = DSS_ADG_BROADBAND_VIDEO_DATA;
            break;
        default:
            p_aux_prefix_data->afid = DSS_ADG_RESERVED;
            break;
    }

}

static void parse_dss_prefix(const unsigned char *p_in,struct dss_prefix_data *p_prefix_data)
{
    memset(p_prefix_data, 0, sizeof(struct dss_prefix_data));

    p_prefix_data->pf = (p_in[0]&0x80)>>7;
    p_prefix_data->bb = (p_in[0]&0x40)>>6;
    p_prefix_data->cf = (p_in[0]&0x20)>>5;
    p_prefix_data->cs = (p_in[0]&0x10)>>4;
    p_prefix_data->scid = ((p_in[0]&0x0F)<<8)|p_in[1];
    p_prefix_data->cc = ((p_in[2]&0xF0)>>4);
    p_prefix_data->hd = p_in[2]&0x0F;
}

static int extract_vaild_data(const unsigned char *p_in,struct dss_decoder *p_decoder)
{
    struct dss_prefix_data *p_prefix_data = NULL;
    u8 start_copy_index = 0;

    memset(p_decoder->payload, 0, sizeof(u8)*127);
    p_decoder->need_copy_length = 0;

    p_prefix_data = (struct dss_prefix_data *)&p_decoder->prefix_data;
    if(p_prefix_data == NULL)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("p_prefix_data is NULL,PLS check \n");
        return MT_FAILURE;
    }

    if(p_prefix_data->hd == 0x00)
    {
        p_prefix_data->packet_type = DSS_AUXILIARY_DATA_PACKET;
        parse_dss_auxiliary_packet(p_in,p_decoder);
    }
    else
    {
        if(p_prefix_data->bb == 0)
        {
            p_decoder->need_copy_length = 127;
            memcpy(p_decoder->payload, p_in, p_decoder->need_copy_length);
            if(p_prefix_data->hd == 0x04)
            {
                if(find_value_in_array_u16(p_decoder->hd, 16, 0x06)== 0)
                    p_prefix_data->packet_type = DSS_AUDIO_SERVICE_PACKET;
                else
                    p_prefix_data->packet_type = DSS_VIDEO_SERVICE_PACKET;
            }
        }
        else
        {
            start_copy_index = find_start_code_index(p_in);
            if(start_copy_index > 127)
            {
                p_decoder->need_copy_length = 0;
            }
            else
            {
                p_decoder->need_copy_length = 127 - start_copy_index;
            }

            if(p_decoder->need_copy_length > 0)
            {
                memcpy(p_decoder->payload, &p_in[start_copy_index], p_decoder->need_copy_length);
            }

            p_prefix_data->packet_type = DSS_VIDEO_SERVICE_PACKET;
        }
    }
}


static int _local_buf_reset(local_buffer_t *p_buf)
{
  p_buf->hold_len = 0;
  p_buf->write_pos = 0;
  p_buf->read_pos = 0;

  return 0;
}

static int _local_buf_write(local_buffer_t *p_buf, u8 *p_data, u32 len)
{
    u32 write_len = 0;
    u32 avaible_len = len;

    if(avaible_len > (p_buf->max_len - p_buf->hold_len))
    {
//        printf("\n##buf_write[max_len=%d, hold_len=%d, write_pos=%d] in[%d]\n", p_buf->max_len, p_buf->hold_len, p_buf->write_pos, len);
//        printf("\n##buf_write: ovweflow [%ld]!!!", len - (p_buf->max_len - p_buf->hold_len));
        avaible_len = (p_buf->max_len - p_buf->hold_len);
    }

    if((p_buf->write_pos + avaible_len) > p_buf->max_len)
    {
        write_len = p_buf->max_len - p_buf->write_pos;
        memcpy(p_buf->p_buf + p_buf->write_pos, p_data, write_len);
        memcpy(p_buf->p_buf, p_data + write_len, avaible_len - write_len);
        p_buf->write_pos = avaible_len - write_len;
    }
    else
    {
        memcpy(p_buf->p_buf + p_buf->write_pos, p_data, avaible_len);
        p_buf->write_pos += avaible_len;
        if (p_buf->write_pos == p_buf->max_len)
        {
            p_buf->write_pos = 0;
        }
//        printf("\n##buf_write: write_pos [%ld]!!!", p_buf->write_pos);
    }
    p_buf->hold_len += avaible_len;
//    printf("\n##buf_write: hold_len [%d]!!!", p_buf->hold_len);

    return avaible_len;
}

static int _local_buf_read(local_buffer_t *p_buf, u8 *p_data, u32 len)
{
    mt_u32 read_len = 0;

    if((p_buf->read_pos + len) > p_buf->max_len)
    {
        read_len = p_buf->max_len - p_buf->read_pos;
        memcpy(p_data, p_buf->p_buf + p_buf->read_pos, read_len);
        memcpy(p_data + read_len, p_buf->p_buf, len - read_len);
        p_buf->read_pos = len - read_len;
    }
    else
    {
        memcpy(p_data, p_buf->p_buf + p_buf->read_pos, len);
        p_buf->read_pos += len;
        if (p_buf->read_pos == p_buf->max_len)
        {
            p_buf->read_pos = 0;
        }
    }
    p_buf->hold_len -= len;

    return len;
}


static int _acquire_es(void)
{
    mt_u32 write_len = 0;
    MT_UNF_ES_BUF_S esinfo = {0};
    int ret = 0;

    ret = MT_UNF_DMX_CheckDataHandle(g_stDssPlayRunInfo.hDmx, 0);
    if(MT_SUCCESS != ret)
    {
//        printf("\n##%s_%d MT_UNF_DMX_CheckDataHandle fail[%x]!\n", __FUNCTION__, __LINE__, ret);
        return 0;
    }

    ret = MT_UNF_DMX_AcquireEs(g_stDssPlayRunInfo.hDmx, &esinfo);
    if(MT_SUCCESS != ret)
    {
//        printf("\n##%s_%d MT_UNF_DMX_AcquireEs fail[%x]!\n", __FUNCTION__, __LINE__, ret);
        return 0;
    }

    if(esinfo.u32BufLen > 0)
    {
        write_len = _local_buf_write(&g_dss_ts_packet, esinfo.pu8Buf, esinfo.u32BufLen);
        if(write_len < esinfo.u32BufLen)
        {
            _local_buf_reset(&g_dss_ts_packet);
            write_len = _local_buf_write(&g_dss_ts_packet, esinfo.pu8Buf, esinfo.u32BufLen);
        }
    }
    else
    {
        printf("\n##_acquire_es esinfo.u32BufLen = %d\n",esinfo.u32BufLen);
    }
     MT_UNF_DMX_ReleaseEs(g_stDssPlayRunInfo.hDmx,&esinfo);
    return 1;
}

static inline void parse_dss_header(const unsigned char *p_in, dss_packet_heaer_t *p_header)
{
    p_header->pf = (p_in[0]&0x80)>>7;
    p_header->bb = (p_in[0]&0x40)>>6;
    p_header->cf = (p_in[0]&0x20)>>5;
    p_header->cs = (p_in[0]&0x10)>>4;
    p_header->scid = ((p_in[0]&0x0F)<<8)|p_in[1];
    p_header->cc = ((p_in[2]&0xF0)>>4);
    p_header->hd = p_in[2]&0x0F;
}


static int _parse_es(void)
{
    u32 dss_packet_cnt = 0;
    int loopi = 0;
    dss_packet_heaer_t dss_header;
    int av_es_payload = 0;
    u8 *p_dss_ts_start = NULL;

    if(0 == g_dss_ts_packet.hold_len)
    {
        return 0;
    }

    dss_packet_cnt = g_dss_ts_packet.hold_len / 130;

    while(loopi < dss_packet_cnt)
    {
        if(g_bTaskQuit == MT_TRUE)
        {
            break;
        }

        if(g_dss_ts_packet.read_pos >= (g_dss_ts_packet.max_len))
        {
            g_dss_ts_packet.read_pos = 0;
        }

        p_dss_ts_start = g_dss_ts_packet.p_buf + g_dss_ts_packet.read_pos;
        parse_dss_header(p_dss_ts_start, &dss_header);
        p_dss_ts_start += 3;

        //null packet
        if(dss_header.scid == 0x00)
        {
            if ((dss_header.bb != 0) || (dss_header.cf != 1) || (dss_header.cs != 0))
            {
                //OS_PRINTF("\n##dss null packet err!\n");
            }
            loopi ++;
            g_dss_ts_packet.read_pos += 130;
            g_dss_ts_packet.hold_len -= 130;
            continue;
        }

        if((dss_header.scid == g_stDssPlayRunInfo.sInputParam.vidpid) || (dss_header.scid == g_stDssPlayRunInfo.sInputParam.audpid))
        {
            if(dss_header.hd == 0x0)
            {
                dss_header.packet_type = DSS_AUXILIARY_DATA_PACKET;
                loopi ++;
                g_dss_ts_packet.read_pos += 130;
                g_dss_ts_packet.hold_len -= 130;
                continue;
            }

            if(dss_header.scid == g_stDssPlayRunInfo.sInputParam.vidpid)
            {

                if ((dss_header.hd == 0x4) || (dss_header.hd == 0x6))
                {
                    dss_header.packet_type = DSS_VIDEO_SERVICE_PACKET;
                }
                else
                {
                    dss_header.packet_type = DSS_PACKET_UNKNOWN;
                    //OS_PRINTF("\n##warning, reserved video ts packet or error,hd[%x]!\n", dss_header.hd);
                }
                if (dss_header.cf == 0)
                {
                    //OS_PRINTF("\n##scrambled video packet, not support!\n");
                    av_es_payload = 0;
                }
                else
                {
                    if ((1 == dss_header.bb) && 0 == g_video_pes_packet_start)
                    {
                        g_video_pes_packet_start = 1;
                    }

                    if ((g_video_pes_packet_start) && (dss_header.packet_type = DSS_VIDEO_SERVICE_PACKET))
                    {
                        av_es_payload = 127;
                        _local_buf_write(&g_video_es, p_dss_ts_start, av_es_payload);
                    }
                }
            }
            else
            {
                if ((dss_header.hd == 0x4) && (0 == dss_header.bb))
                {
                    dss_header.packet_type = DSS_AUDIO_SERVICE_PACKET;
                }
                else
                {
                  dss_header.packet_type = DSS_PACKET_UNKNOWN;
                  //OS_PRINTF("\n##warning, audio ts packet error hd[%x]!\n", dss_header.hd);
                }
                if (dss_header.cf == 0)
                {
                    //OS_PRINTF("\n##scrambled audio packet, not support!\n");
                    av_es_payload = 0;
                }
                else
                {
                    if (dss_header.packet_type = DSS_AUDIO_SERVICE_PACKET)
                    {
                        av_es_payload = 127;
                        _local_buf_write(&g_audio_es, p_dss_ts_start, av_es_payload);
                    }
                }
            }
        }

        loopi ++;
        g_dss_ts_packet.read_pos += 130;
        g_dss_ts_packet.hold_len -= 130;
    }
    return 1;
}

static int _push_av_es(void)
{
    mt_handle av_handle = 0;
    MT_UNF_STREAM_BUF_S StreamBuf;
    u32 put_len = 0;
    mt_u64 aud_pts_fake = 0;
    int ret = 0;

    if((g_video_es.hold_len + g_audio_es.hold_len) < 7000)
    {
        return 0;
    }

    av_handle = g_stDssPlayRunInfo.hAvPlay;

    if(g_video_es.hold_len)
    {
        ret = MT_UNF_AVPLAY_GetBuf(av_handle, MT_UNF_AVPLAY_BUF_ID_ES_VID, g_video_es.hold_len, &StreamBuf, 0);

        if (MT_SUCCESS == ret && StreamBuf.u32Size > 0 && StreamBuf.pu8Data != NULL)
        {
            put_len = _local_buf_read(&g_video_es, StreamBuf.pu8Data, StreamBuf.u32Size);
        }
        ret = MT_UNF_AVPLAY_PutBuf(av_handle, MT_UNF_AVPLAY_BUF_ID_ES_VID, put_len, 0);
    }

    if(g_audio_es.hold_len)
    {
        ret = MT_UNF_AVPLAY_GetBuf(av_handle, MT_UNF_AVPLAY_BUF_ID_ES_AUD, g_audio_es.hold_len, &StreamBuf, 0);
        if(MT_SUCCESS == ret && StreamBuf.u32Size > 0 && StreamBuf.pu8Data != NULL)
        {
            put_len = _local_buf_read(&g_audio_es, StreamBuf.pu8Data, StreamBuf.u32Size);
        }
        ret = MT_UNF_AVPLAY_PutBuf64(av_handle,  MT_UNF_AVPLAY_BUF_ID_ES_AUD, put_len, (171 * aud_pts_fake++), NULL);
    }
    return 1;
}


/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_DssPlayInjectTsTask(mt_void *args)
{
    mt_s32  Ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_BOOL bVidPlay = MT_TRUE;
    MT_BOOL bAudPlay = MT_TRUE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    MT_CHAR *pstParam = (MT_CHAR*)(args);
    FILE *pTsFile = NULL;
    mt_handle hAvplay;
    mt_u32 Readlen8;
    static u8 *video_buff = NULL;
    static u8 *audio_buff = NULL;
    MT_BOOL b_start_video_play = MT_FALSE;
    MT_BOOL b_start_audio_play = MT_FALSE;
    struct dss_prefix_data *p_prefix_data = NULL;
    mt_u64 aud_pts_fake = 0;
    unsigned char *dss_one_pkt = NULL;
    unsigned char *dss_pkt = NULL;
    mt_u32 data_len = 0;
    mt_u32 video_buff_copy_index = 0;
    mt_u32 packet_count = 0;
    int index = 0;
    FILE *EsFile = NULL;
    struct dss_decoder g_decoder = { 0 };

    hAvplay = g_stDssPlayRunInfo.hAvPlay;

    if(g_stDssPlayRunInfo.sInputParam.data_source == 0)
    {
        memset(&g_dss_ts_packet, 0, sizeof(g_dss_ts_packet));
        memset(&g_video_es, 0, sizeof(g_video_es));
        memset(&g_audio_es, 0, sizeof(g_audio_es));

        g_dss_ts_packet.p_buf = malloc(DSS_TS_PACKET_BUF_MAX);
        if(g_dss_ts_packet.p_buf == NULL)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("\n##%s_%d mtos_malloc fail[%x]!\n", __FUNCTION__, __LINE__, Ret);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }
        g_dss_ts_packet.max_len = DSS_TS_PACKET_BUF_MAX;
        memset(g_dss_ts_packet.p_buf, 0, g_dss_ts_packet.max_len);

        g_video_es.p_buf = malloc(VIDEO_ES_MAX);
        if(g_video_es.p_buf == NULL)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("\n##%s_%d mtos_malloc fail[%x]!\n", __FUNCTION__, __LINE__, Ret);
            free(g_dss_ts_packet.p_buf);
            g_bTaskQuit = MT_TRUE;
            return ERR_FAILURE;
        }
        g_video_es.max_len = VIDEO_ES_MAX;
        memset(g_video_es.p_buf, 0, g_video_es.max_len);

        g_audio_es.p_buf = malloc(AUDIO_ES_MAX);
        if(g_audio_es.p_buf == NULL)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("\n##%s_%d mtos_malloc fail[%x]!\n", __FUNCTION__, __LINE__, Ret);
            free(g_dss_ts_packet.p_buf);
            free(g_video_es.p_buf);
            g_bTaskQuit = MT_TRUE;
            return ERR_FAILURE;
        }
        g_audio_es.max_len = AUDIO_ES_MAX;
        memset(g_audio_es.p_buf, 0, g_audio_es.max_len);
    }
    else if(g_stDssPlayRunInfo.sInputParam.data_source == 1)
    {
        memset(g_decoder.hd, 0xFFFF, sizeof(u16)*16);
        memset(g_decoder.packet_type, 0xFFFF, sizeof(u16)*8);
        g_decoder.audio_scid_filter = 0x11;
        g_decoder.video_scid_filter = 0x10;

        dss_one_pkt = (u8 *)malloc(READ_DSS_DATA_LEN);
        if(dss_one_pkt == NULL)
        {
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }
        memset(dss_one_pkt, 0, READ_DSS_DATA_LEN);

        SAMPLE_DSSPLAY_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam);

        /* Open a binary file. The file must exist. Read only*/
        pTsFile = fopen(pstParam, "rb");
        if(pTsFile == NULL)
        {
            SAMPLE_DSSPLAY_ERR_PRINT( "\nfile %s open error!!\n", pstParam);
            free(dss_one_pkt);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }

        video_buff = (u8 *)malloc(V_ES_BUFFER_SIZE);
        if(video_buff == NULL)
        {
            fclose(pTsFile);
            pTsFile = NULL;
            free(dss_one_pkt);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }
        memset(video_buff, 0, V_ES_BUFFER_SIZE);

        audio_buff = (u8 *)malloc(AUDIO_ES_1_READ_LEN);
        if(audio_buff == NULL)
        {
            free(video_buff);
            fclose(pTsFile);
            pTsFile = NULL;
            free(dss_one_pkt);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }
        memset(audio_buff, 0, AUDIO_ES_1_READ_LEN);

        p_prefix_data = &g_decoder.prefix_data;
        if(p_prefix_data == NULL)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("p_prefix_data is NULL,PLS check \n");
            free(audio_buff);
            free(video_buff);
            fclose(pTsFile);
            pTsFile = NULL;
            free(dss_one_pkt);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }
    }

    while(!g_bTaskQuit)
    {
        if(g_stDssPlayRunInfo.sInputParam.data_source == 0)
        {
            if(g_run == 0)
            {
                mtos_task_sleep(50);
                continue;
            }
            Ret = _acquire_es();
            Ret |= _parse_es();
            Ret |= _push_av_es();
            if(!Ret)
            {
                mtos_task_sleep(80);
            }
        }
        else if(g_stDssPlayRunInfo.sInputParam.data_source == 1)
        {
            packet_count = 0;

            memset(dss_one_pkt, 0, READ_DSS_DATA_LEN);
            data_len = fread(dss_one_pkt, 1, READ_DSS_DATA_LEN, pTsFile);
            packet_count = data_len / 130;
            if(data_len < READ_DSS_DATA_LEN)
            {
                SAMPLE_DSSPLAY_INFO_PRINT("read  file end and rewind!\n");
                rewind(pTsFile);
                memset(g_decoder.hd, 0xFFFF, sizeof(u16)*16);
                memset(g_decoder.packet_type, 0xFFFF, sizeof(u16)*8);
                continue;
            }
            dss_pkt = &dss_one_pkt[0];

            index = 0;
            while(packet_count > index)
            {
                index ++;
                if(data_len >= 130)
                {
                    parse_dss_prefix(&dss_pkt[0], p_prefix_data);
                    if(p_prefix_data->scid == 0x00)
                    {
                        p_prefix_data->packet_type = DSS_NULL_PACKET;
                        if(is_null_or_ranging_packet(&dss_pkt[2]))
                        {
                            dss_pkt += 130;
                            continue;
                        }
                        else
                        {
                            SAMPLE_DSSPLAY_ERR_PRINT("ERR: scid is 0,but payload no match,pls check!!\n");
                            dss_dump_data("payload", &dss_pkt[2], 128);
                        }

                    }
                    else
                    {
                        if(p_prefix_data->cf == 1)
                        {
                            if((p_prefix_data->scid == g_decoder.audio_scid_filter)||(p_prefix_data->scid == g_decoder.video_scid_filter))
                            {
                                extract_vaild_data(&dss_pkt[3], &g_decoder);
                                if(g_decoder.need_copy_length > 0)
                                {
                                    if(p_prefix_data->scid == g_decoder.audio_scid_filter)
                                    {
                                        memcpy(&audio_buff[g_decoder.had_copy_auido_data_length], &g_decoder.payload, g_decoder.need_copy_length);
                                        g_decoder.had_copy_auido_data_length += g_decoder.need_copy_length;
                                        if((AUDIO_ES_1_READ_LEN - g_decoder.had_copy_auido_data_length) < 127)
                                        {
                                            b_start_audio_play = MT_TRUE;
                                        }
                                    }
                                    else if(p_prefix_data->scid == g_decoder.video_scid_filter)
                                    {
                                        memcpy(&video_buff[g_decoder.had_copy_video_data_length], &g_decoder.payload, g_decoder.need_copy_length);
                                        g_decoder.had_copy_video_data_length += g_decoder.need_copy_length;
                                        if((V_ES_BUFFER_SIZE - g_decoder.had_copy_video_data_length) < 127)
                                        {
                                            b_start_video_play = MT_TRUE;
                                        }
                                    }
                                }

                            }
                        }
                        else
                        {
                            dss_pkt += 130;
                            continue;
                        }
                    }

                }
                dss_pkt += 130;
            }

            if (b_start_video_play & bVidPlay)
            {
                video_buff_copy_index = 0;
                do
                {
                    MUTEX_LOCK();
                    Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, V_ES_BUFFER_SIZE, &StreamBuf, 0);
                    if(MT_SUCCESS == Ret && StreamBuf.u32Size > 0 && StreamBuf.pu8Data != NULL)
                    {
                        memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);
                        if(g_decoder.had_copy_video_data_length > StreamBuf.u32Size)
                        {
                            g_decoder.had_copy_video_data_length -= StreamBuf.u32Size;
                            Readlen = StreamBuf.u32Size;
                        }
                        else
                        {
                            Readlen = g_decoder.had_copy_video_data_length;
                            g_decoder.had_copy_video_data_length = 0;
                        }
                        memcpy(StreamBuf.pu8Data, &video_buff[video_buff_copy_index], Readlen);
                        video_buff_copy_index += Readlen;
                        if(Readlen > 0)
                        {
                            Readlen8 = Readlen;
                            //stuff zero
                            if (Readlen < Readlen8)
                            {
                                memset(StreamBuf.pu8Data+Readlen, 0, Readlen8-Readlen);
                            }

                            Ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, Readlen8, 0);
                            if (Ret != MT_SUCCESS)
                            {
                                SAMPLE_DSSPLAY_ERR_PRINT("call MT_UNF_AVPLAY_PutBuf failed.\n");
                            }
                        }
                    }

                    MT_USLEEP(30000);
                    MUTEX_UNLOCK();
                }while(g_decoder.had_copy_video_data_length != 0);

                b_start_video_play = MT_FALSE;
                memset(video_buff, 0, V_ES_BUFFER_SIZE);
            }

            if (b_start_audio_play & bAudPlay)
            {
                do
                {
                    MUTEX_LOCK();

                    Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, AUDIO_ES_1_READ_LEN, &StreamBuf, 0);
                    if(MT_SUCCESS == Ret && StreamBuf.pu8Data != NULL && StreamBuf.u32Size == AUDIO_ES_1_READ_LEN)
                    {
                        if(g_decoder.had_copy_auido_data_length > StreamBuf.u32Size)
                        {
                            g_decoder.had_copy_auido_data_length -= StreamBuf.u32Size;
                            Readlen = StreamBuf.u32Size;
                        }
                        else
                        {
                            Readlen = g_decoder.had_copy_auido_data_length;
                            g_decoder.had_copy_auido_data_length = 0;
                        }
                        memcpy(StreamBuf.pu8Data, audio_buff, Readlen);
                        b_start_audio_play = MT_FALSE;
                        memset(audio_buff, 0, AUDIO_ES_1_READ_LEN);
                        if(Readlen > 0)
                        {
                            Ret = MT_UNF_AVPLAY_PutBuf64(hAvplay,  MT_UNF_AVPLAY_BUF_ID_ES_AUD, Readlen, (171 * aud_pts_fake++), NULL);
                            if(Ret != MT_SUCCESS)
                            {
                                SAMPLE_DSSPLAY_ERR_PRINT("call MT_UNF_AVPLAY_PutBuf failed.\n");
                            }
                        }
                        MUTEX_UNLOCK();
                        break;
                    }
                    else if(Ret != MT_SUCCESS)
                    {
                        MT_USLEEP(30000);
                    }
                    MUTEX_UNLOCK();
                }while(1);
            }
        }
    }

    if(g_stDssPlayRunInfo.sInputParam.data_source == 0)
    {
        free(g_dss_ts_packet.p_buf);
        free(g_audio_es.p_buf);
        free(g_video_es.p_buf);
    }
    else if(g_stDssPlayRunInfo.sInputParam.data_source == 1)
    {

        if(bVidPlay)
        {
            do
            {
                MT_UNF_AVPLAY_PUTBUFEX_OPT_S PutOpt;

                SAMPLE_DSSPLAY_INFO_PRINT("send video eos flag\n");
                Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, 32, &StreamBuf, 0);
                if(Ret == MT_SUCCESS && StreamBuf.u32Size >= 32)
                {
                    memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);

                    PutOpt.bEndOfFrm        = MT_TRUE;
                    PutOpt.bContinue        = MT_TRUE;
                    PutOpt.u32PtsValide     = 1;
                    PutOpt.u32FrameFinsh    = 1;
                    PutOpt.u32EosFlag       = 1;

                    Ret = MT_UNF_AVPLAY_PutBuf64(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, StreamBuf.u32Size, 0, &PutOpt);
                    if(Ret != MT_SUCCESS)
                    {
                        SAMPLE_DSSPLAY_ERR_PRINT("error: MT_UNF_AVPLAY_PutBuf64 {VID} failed, return %d!\n", Ret);
                    }
                    bVidPlay = MT_FALSE;
                    break;
                }
                MT_USLEEP(30000);
            } while (1);
        }

        if(bAudPlay)
        {
            do
            {
                MT_UNF_AVPLAY_PUTBUFEX_OPT_S PutOpt;

                SAMPLE_DSSPLAY_INFO_PRINT("send audio eos flag\n");
                Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, 32, &StreamBuf, 0);
                if(Ret == MT_SUCCESS && StreamBuf.u32Size >= 32)
                {
                    memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);

                    PutOpt.bEndOfFrm        = MT_TRUE;
                    PutOpt.bContinue        = MT_TRUE;
                    PutOpt.u32PtsValide     = 1;
                    PutOpt.u32FrameFinsh    = 1;
                    PutOpt.u32EosFlag       = 1;

                    Ret = MT_UNF_AVPLAY_PutBuf64(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, StreamBuf.u32Size, 0, &PutOpt);
                    if(Ret != MT_SUCCESS)
                    {
                        SAMPLE_DSSPLAY_ERR_PRINT("error: MT_UNF_AVPLAY_PutBuf64 {Aud} failed, return %d!\n",Ret);
                    }
                    bAudPlay = MT_FALSE;
                    break;
                }
                MT_USLEEP(30000);
            } while (1);
        }

        free(audio_buff);
        free(video_buff);
        fclose(pTsFile);
        pTsFile = NULL;
        free(dss_one_pkt);
    }

    return MT_SUCCESS;
}

static mt_s32 MT_DssPlayStopplay(MT_HANDLE avplay)
{
    mt_s32     ret = MT_SUCCESS;

    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_Stop\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*
@brief help
@return void
*/
static void MT_DssPlayPrint_Help(char *name)
{
    MT_DSSPLAY_PRINT("Lack of parameters\n");
    MT_DSSPLAY_PRINT("\nUsage:\n");
    MT_DSSPLAY_PRINT("%s\n", name);
    MT_DSSPLAY_PRINT("    -f: path of the stream file\n");
    MT_DSSPLAY_PRINT("    -s: DVBS locks frequency\n");
    MT_DSSPLAY_PRINT("example:\n");
    MT_DSSPLAY_PRINT("    %s -f ./test.trp\n",name);
    MT_DSSPLAY_PRINT("    %s -s 3840 27500 1 0 0 16 17\n",name);
    MT_DSSPLAY_PRINT("    -q: Exit the background\n");
}


static MT_VOID MT_DssPlayPrintMenu(MT_VOID)
{
#ifdef MT_SAMPLE_APP
    SAMPLE_DSSPLAY_PRINT("     b : background run \n");
#endif
    SAMPLE_DSSPLAY_PRINT("     h : help \n");
    SAMPLE_DSSPLAY_PRINT("     q : quit \n");
    SAMPLE_DSSPLAY_PRINT("DssPlay>> ");

}

static MT_VOID MT_DssPlayExit()
{
    SAMPLE_DSSPLAY_FUNCTION_ENTER();

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stDssPlayRunInfo.htsThd, NULL);

    (MT_VOID)MT_DssPlayStopplay(g_stDssPlayRunInfo.hAvPlay);

    (MT_VOID)MT_DssPlayAvplayDeInit(g_stDssPlayRunInfo.hAvPlay, g_stDssPlayRunInfo.hWin, g_stDssPlayRunInfo.hSoundTrack);

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MT_DssPlayDmxDeInit();

    if(MT_INPUT_SIG_TYPE_FILE != g_stDssPlayRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
    memset(&g_stDssPlayRunInfo, 0xff, sizeof(g_stDssPlayRunInfo));

    SAMPLE_DSSPLAY_FUNCTION_EXIT();
}


/*
@brief quit and function
@return void
*/
static void MT_DssPlayCmdTask(mt_handle hAvplay)
{
    mt_s32     ret = MT_SUCCESS;
    mt_u32     u32ProgNum = 0;
    MT_CHAR    inputCmd[32] = { 0 };

    struct timespec start, end;
    double elapsed_seconds;

    while (1)
    {
        (void)MT_DssPlayPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_DSSPLAY_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_DSSPLAY_INFO_PRINT("tsplay play in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            continue;
        }

    }
}

/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::MT_VOID
@*/
static MT_S32 MT_DssPlayParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInutParam)
{
    MT_S32 opt = 0;
    SAMPLE_DSSPLAY_FUNCTION_ENTER();
    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DssPlayPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_DssPlayPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_DssPlayPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInutParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                pInutParam->data_source = 1;
                MTADP_Strncpy(pInutParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 9)
                {
                    (MT_VOID)MT_DssPlayPrint_Help(argv[0]);
                    return MT_FAILURE;
                }

                pInutParam->sig_type = MT_INPUT_SIG_TYPE_SAT;
                pInutParam->data_source = 0;
                pInutParam->input_param.sat.freq = strtol(argv[2], 0, 0);
                pInutParam->input_param.sat.sym_rate = strtol(argv[3], 0, 0);
                pInutParam->input_param.sat.onoff_22k = strtol(argv[4], 0, 0);
                pInutParam->input_param.sat.polarization = strtol(argv[5], 0, 0);
                pInutParam->input_param.sat.port_type = strtol(argv[6], 0, 0);
                pInutParam->vidpid = strtol(argv[7], 0, 0);
                pInutParam->audpid = strtol(argv[8], 0, 0);

                return MT_SUCCESS;

            case 'c':
                if(argc != 5)
                {
                    (MT_VOID)MT_DssPlayPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInutParam->sig_type = MT_INPUT_SIG_TYPE_CAB;
                pInutParam->data_source = 0;
                pInutParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInutParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInutParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DssPlayExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_DssPlayPrint_Help(argv[0]);
                return MT_FAILURE;

        }
    }
    SAMPLE_DSSPLAY_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DssPlayMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    mt_s32     ret = MT_SUCCESS;

    /** Get the parameters */
    ret = MT_DssPlayParase_args(argc, argv, &g_stDssPlayRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP

        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_SYS_Init\n");
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
        }

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }
#endif
        if(0 == g_stDssPlayRunInfo.sInputParam.data_source)
        {
            if(MT_INPUT_SIG_TYPE_FILE != g_stDssPlayRunInfo.sInputParam.sig_type)
            {
                ret = MTADP_Fe_Init(TUNER_ID_0);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DSSPLAY_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR3;
                }

                if (MT_INPUT_SIG_TYPE_CAB == g_stDssPlayRunInfo.sInputParam.sig_type)
                {
                    ret = MT_DssPlayCheckDvbcParam(&g_stDssPlayRunInfo.sInputParam.input_param.cab);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_DSSPLAY_ERR_PRINT("MT_DssPlayCheckDvbcParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                        goto ERR4;
                    }

                    ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.cab.freq,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.cab.sym_rate,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.cab.mod_type);
                }
                else if(MT_INPUT_SIG_TYPE_SAT == g_stDssPlayRunInfo.sInputParam.sig_type)
                {
                    ret = MT_DssPlayCheckDvbsParam(&g_stDssPlayRunInfo.sInputParam.input_param.sat);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_DSSPLAY_ERR_PRINT("MT_CCModeCheckDvbsParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                        goto ERR4;
                    }

                    ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.sat.freq,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.sat.sym_rate,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.sat.onoff_22k,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.sat.polarization,
                                                    g_stDssPlayRunInfo.sInputParam.input_param.sat.port_type);
                }

                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DSSPLAY_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR4;
                }
            }
        }



        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR5;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR6;
        }

        ret = MT_DssPlayAVplayInit(&g_stDssPlayRunInfo.hAvPlay, &g_stDssPlayRunInfo.hWin, &g_stDssPlayRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to StartAVPlay\n");
            goto ERR7;
        }

        ret = MT_DssPlayAVPlay_Start(g_stDssPlayRunInfo.hAvPlay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to MTADP_AVPlay_PlayProg\n");
            goto ERR8;
        }

        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stDssPlayRunInfo.htsThd, NULL, (void * (*)(void *))MT_DssPlayInjectTsTask, &g_stDssPlayRunInfo.sInputParam.input_param.file);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to pthread_create\n");
            goto ERR9;
        }

        ret = MT_DssPlayDmxInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DSSPLAY_ERR_PRINT("failed to MT_DssPlayDmxInit\n");
            goto ERR10;
        }

        sleep(1);
        g_run = 1;

    }

    (void)MT_DssPlayCmdTask(g_stDssPlayRunInfo.hAvPlay);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
ERR10:
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stDssPlayRunInfo.htsThd, NULL);
ERR9:
    ret = MT_DssPlayStopplay(g_stDssPlayRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DSSPLAY_ERR_PRINT("failed to stop\n");
    }
ERR8:
    (MT_VOID)MT_DssPlayAvplayDeInit(g_stDssPlayRunInfo.hAvPlay, g_stDssPlayRunInfo.hWin, g_stDssPlayRunInfo.hSoundTrack);
ERR7:
    (MT_VOID)MTADP_VO_DeInit();

ERR6:
    (MT_VOID)MTADP_Snd_DeInit();
ERR5:
    (MT_VOID)MT_DssPlayDmxDeInit();
ERR4:
    if(MT_INPUT_SIG_TYPE_FILE != g_stDssPlayRunInfo.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR3:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_stDssPlayRunInfo, 0xff, sizeof(g_stDssPlayRunInfo));
    return MT_SUCCESS;
}
