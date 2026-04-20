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
#include "mt_drv_disp.h"
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
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
#include "mt_cmdline.h"

#include "Irdeto_ca_adt.h"
#include "ird_test_cmd.h"

/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_CA_IRD_DEBUG

#define MT_CA_IRD_PRINT   printf
#else

#define MT_CA_IRD_PRINT

#endif

#define SAMPLE_CA_IRD_FUNCTION_ENTER()     MT_CA_IRD_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_CA_IRD_FUNCTION_EXIT()      MT_CA_IRD_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_CA_IRD_FATAL_PRINT(fmt...)      MT_CA_IRD_PRINT(" [FATAL] " fmt)
#define SAMPLE_CA_IRD_ERR_PRINT(fmt...)        MT_CA_IRD_PRINT(" [ERROR] " fmt)
#define SAMPLE_CA_IRD_WARN_PRINT(fmt...)       MT_CA_IRD_PRINT(" [WARN] "  fmt)
#define SAMPLE_CA_IRD_INFO_PRINT(fmt...)       MT_CA_IRD_PRINT(" [INFO] "  fmt)
#define SAMPLE_CA_IRD_DBG_PRINT(fmt...)        MT_CA_IRD_PRINT(" [DEBUG] " fmt)

#define SAMPLE_CA_IRD_PRINT   printf


#define DMX_ID_0 0
#define TUNER_ID_0 (0)

#define INVALID_TSPID (0x1fff)

#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2


#define MT_IRD_TEST_MODE	1
#define MT_IRD_DESC_MODE	2

#define MT_IRD_TEST_ALL		1
#define MT_IRD_TEST_ONE		2


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
    MT_U32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    MT_U32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
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
    MT_U32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    MT_U32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
    MT_U8 port_type;
} mt_input_ter_para_t;


typedef struct tagInput_Param_T
{
    MT_U8 file_name[256];
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

	MT_U8 mode;

} mt_input_para_t;
typedef struct
{
    MT_U8 file_name[256];
}source_file_param_t;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          stInjectTSThread;
	pthread_t		   stIrdTestThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
} MT_IRD_RUN_INFO;

typedef struct
{
    MT_U8          TestMode;
    MT_U16		   TestNumber;
} MT_IRD_TEST_INFO;



#ifdef MT_SAMPLE_APP
extern mt_play_resource_t play_resource;
MT_S32 MT_CaIrdMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_IRD_RUN_INFO ird_run_info;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif



/******************************* API declaration *****************************/

#ifndef MT_SAMPLE_APP
static MT_S32 MT_IrdCakCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_CA_IRD_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_CA_IRD_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_CA_IRD_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static MT_S32 MT_IrdCakCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_CA_IRD_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_CA_IRD_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}



/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_IrdCakDmxInit(mt_input_para_t *pInputParam)
{
    MT_S32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_CA_IRD_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (MT_VOID) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == pInputParam->sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            (MT_VOID) MT_UNF_DMX_DeInit();
            return MT_FAILURE;
        }
    }
    else
    {
         memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to mt_sys_get_version\n");
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

    SAMPLE_CA_IRD_FUNCTION_EXIT();


    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_IrdCakDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
    (MT_VOID)MT_UNF_DMX_DeInit();
    return MT_SUCCESS;
}
#endif
#if 0
static MT_S32 MT_IrdCakGetDemuxData(MT_U16 pid, DEMUX_FILTER_S *pfilterInfo, mt_u8 *pu8DataBuf, mt_u32 *pBufSize)
{
	MT_S32      ret = MT_SUCCESS;
	MT_HANDLE channelHandle = MT_INVALID_HANDLE; 
	MT_HANDLE filterHandle = MT_INVALID_HANDLE; 
	
	mt_u32 u32BufSize[MAX_SECTION_NUM];
	mt_u32 u32AquiredNum = 0;
    
	
	ret = MTADP_DEMUX_OpenChannel(DMX_ID_0, pid, &channelHandle);
	if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
	

	ret = MT_Section_DEMUX_SetFilter(channelHandle, &filterHandle, &pfilterInfo);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
	/*Start demux.*/
	ret = MT_Sample_DEMUX_StartChannel(channelHandle);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }

	pu8DataBuf = malloc(MAX_SECTION_LEN * MAX_SECTION_NUM);
    if(NULL == pu8DataBuf){
        return MT_FAILURE;
    }
	memset(pu8DataBuf, 0, MAX_SECTION_LEN * MAX_SECTION_NUM);
    memset(u32BufSize, 0, sizeof(u32BufSize));

	ret = MTADP_DEMUX_DataRead(channelHandle, 2000, pu8DataBuf, &u32AquiredNum, pBufSize);
	if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
	
	ret = MTADP_DEMUX_CloseFilter(channelHandle, filterHandle);
	ret |= MTADP_DEMUX_StopChannel(channelHandle);
	ret |= MTADP_DEMUX_CloseChannel(channelHandle);
	if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
}

#endif

static MT_S32 MT_IrdCakSetPmtInfo(PMT_COMPACT_PROG   *pstProgInfo)
{
	MT_S32      ret = MT_SUCCESS;
	MT_HANDLE channelHandle = MT_INVALID_HANDLE; 
	MT_HANDLE filterHandle = MT_INVALID_HANDLE; 
	
	mt_u32 u32BufSize[MAX_SECTION_NUM];
	mt_u32 u32AquiredNum = 0;
	
	mt_u8 *pu8DataBuf = NULL;
	DEMUX_FILTER_S filterInfo= {0};
	mt_u8 retry = 0;

	SAMPLE_CA_IRD_FUNCTION_ENTER();

	ret = MTADP_DEMUX_OpenChannel(DMX_ID_0, pstProgInfo->PmtPid, &channelHandle);
	if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
	
	memset(&filterInfo, 0x0, sizeof(DEMUX_FILTER_S));

    /*Set up filter condition.*/
    filterInfo.u8Match[0] = PMT_TABLE_ID;
    filterInfo.u8Mask[0] = 0xFF;

    /* set service id*/
    filterInfo.u8Match[1] = (mt_u8)((pstProgInfo->ProgID >> 8) & 0xFF);
    filterInfo.u8Mask[1] = 0xFF;

    filterInfo.u8Match[2] = (mt_u8)(pstProgInfo->ProgID & 0xFF);
    filterInfo.u8Mask[2] = 0xFF;

	filterInfo.u16FilterDepth = 3;

	ret = MTADP_DEMUX_SetFilter(DMX_ID_0, channelHandle, &filterInfo, &filterHandle);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
	/*Start demux.*/
	ret = MTADP_DEMUX_StartChannel(channelHandle);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }

	pu8DataBuf = malloc(MAX_SECTION_LEN * MAX_SECTION_NUM);
    if(NULL == pu8DataBuf){
        return MT_FAILURE;
    }
	memset(pu8DataBuf, 0, MAX_SECTION_LEN * MAX_SECTION_NUM);
    memset(u32BufSize, 0, sizeof(u32BufSize));

	retry = 0;
	do{
		ret = MTADP_DEMUX_DataRead(channelHandle, 2000, pu8DataBuf, &u32AquiredNum, u32BufSize);
		if(MT_SUCCESS != ret)
	    {
	        break;
	    }

		ret = cas_irdeto_table_prase(CAS_TID_PMT, pu8DataBuf);
		SAMPLE_CA_IRD_INFO_PRINT("cas_irdeto_table_prase ret=%d \n", ret);
		
	}while(MT_SUCCESS != ret && retry++ < 3);
	
	ret = MTADP_DEMUX_CloseFilter(channelHandle, filterHandle);
	ret |= MTADP_DEMUX_StopChannel(channelHandle);
	ret |= MTADP_DEMUX_CloseChannel(channelHandle);
	if(MT_SUCCESS != ret)
	{
		return MT_FAILURE;
	}

	SAMPLE_CA_IRD_FUNCTION_EXIT();

	return MT_SUCCESS;
	
}


static MT_S32 MT_IrdCakSetCatInfo()
{
	MT_S32      ret = MT_SUCCESS;
	MT_HANDLE channelHandle = MT_INVALID_HANDLE; 
	MT_HANDLE filterHandle = MT_INVALID_HANDLE; 
	
	mt_u32 u32BufSize[MAX_SECTION_NUM];
	mt_u32 u32AquiredNum = 0;
	
	mt_u8 *pu8DataBuf = NULL;
	DEMUX_FILTER_S filterInfo= {0};
	mt_u8 retry = 0;

	SAMPLE_CA_IRD_FUNCTION_ENTER();

	ret = MTADP_DEMUX_OpenChannel(DMX_ID_0, CAT_TSPID, &channelHandle);
	if(MT_SUCCESS != ret)
	{
		return MT_FAILURE;
	}

	memset(&filterInfo, 0x0, sizeof(DEMUX_FILTER_S));

    /*Set up filter condition.*/
    filterInfo.u8Match[0] = CAT_TABLE_ID;
    filterInfo.u8Mask[0] = 0xFF;

	filterInfo.u16FilterDepth = 1;
	ret = MTADP_DEMUX_SetFilter(DMX_ID_0, channelHandle, &filterInfo, &filterHandle);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
	
	/*Start demux.*/
	ret = MTADP_DEMUX_StartChannel(channelHandle);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }

	pu8DataBuf = malloc(MAX_SECTION_LEN * MAX_SECTION_NUM);
    if(NULL == pu8DataBuf){
        return MT_FAILURE;
    }
	memset(pu8DataBuf, 0, MAX_SECTION_LEN * MAX_SECTION_NUM);
    memset(u32BufSize, 0, sizeof(u32BufSize));

	retry = 0;
	do{
		ret = MTADP_DEMUX_DataRead(channelHandle, 2000, pu8DataBuf, &u32AquiredNum, u32BufSize);
		if(MT_SUCCESS != ret)
	    {
	        break;
	    }
		
		ret = cas_irdeto_table_prase(CAS_TID_CAT, pu8DataBuf);

	}while(MT_SUCCESS != ret && retry++ < 3);

	free(pu8DataBuf);
	pu8DataBuf = NULL;
	
	ret = MTADP_DEMUX_CloseFilter(channelHandle, filterHandle);
	ret |= MTADP_DEMUX_StopChannel(channelHandle);
	ret |= MTADP_DEMUX_CloseChannel(channelHandle);
	if(MT_SUCCESS != ret)
	{
		return MT_FAILURE;
	}

	SAMPLE_CA_IRD_FUNCTION_EXIT();

	return MT_SUCCESS;

}
/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_IrdCakSetCasInfo(PMT_COMPACT_PROG   *pstProgInfo)
{
	MT_S32      ret = MT_SUCCESS;
	MT_HANDLE channelHandle = MT_INVALID_HANDLE; 
	MT_HANDLE filterHandle = MT_INVALID_HANDLE; 
	
	mt_u32 u32BufSize[MAX_SECTION_NUM];
	mt_u32 u32AquiredNum = 0;
	
	cas_channel_state_set channel_state;
	NIT_TB          nit_tb = { 0 };
	mt_u8 *pu8DataBuf = NULL;
	DEMUX_FILTER_S filterInfo= {0};
	mt_u8 retry = 0;

	SAMPLE_CA_IRD_FUNCTION_ENTER();
	
//#ifdef MT_SAMPLE_APP
//	SRH_NITRequest(play_resource.demux_id, &nit_tb);
//#else
	SRH_NITRequest(DMX_ID_0, &nit_tb);
//#endif

	cas_irdeto_io_ctrl(CAS_CMD_VIDEO_PID_SET,&pstProgInfo->VElementPid);	
	cas_irdeto_io_ctrl(CAS_CMD_AUDIO_PID_SET,&pstProgInfo->AElementPid);	

	memset(&channel_state, 0 ,sizeof(channel_state));
	channel_state.org_network_id = nit_tb.NitInfo[0].u16OrgNetID;
	channel_state.ts_id = nit_tb.NitInfo[0].u16TsId;
	channel_state.service_id = pstProgInfo->ProgID;
	channel_state.pmt_pid = pstProgInfo->PmtPid;
	
	cas_irdeto_io_ctrl(CAS_CMD_CHANNEL_STATE_SET,&channel_state);	
	cas_irdeto_io_ctrl(CAS_IOCMD_CRATE_TA_SLOT, NULL);

	ret = MT_IrdCakSetPmtInfo(pstProgInfo);
	ret != MT_IrdCakSetCatInfo();
	if(MT_SUCCESS != ret)
	{
		return MT_FAILURE;
	}

	SAMPLE_CA_IRD_FUNCTION_EXIT();

	return MT_SUCCESS;
}


void cas_on_resend_cat()
{
	MT_IrdCakSetCatInfo();
}


/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_IrdCakAVplayInit(MT_HANDLE *p_hAvplay, MT_HANDLE *P_hWin, MT_HANDLE *p_hSoundTrack)
{
    MT_S32      ret = MT_SUCCESS;
    MT_HANDLE   hAvplay = MT_INVALID_HANDLE;
    MT_HANDLE   hWin = MT_INVALID_HANDLE;
    MT_HANDLE   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_CA_IRD_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_CA_IRD_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_CA_IRD_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_VOID  MT_IrdCakAvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{


    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_CA_IRD_ERR_PRINT("phAvplay is null.\n");

    }

    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_CA_IRD_ERR_PRINT("phWin is null.\n");

    }

    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_CA_IRD_ERR_PRINT("phSoundTrack is null.\n");

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
static MT_S32 MT_IrdCakAVPlay_Start(MT_HANDLE hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
{
    MT_U32 VidPid = 0;
    MT_U32 AudPid = 0;
    MT_U32 u32AudType = 0;
    MT_S32 ret = 0;
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0X00;

    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_CA_IRD_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    if(NULL == p_ProgInfo)
    {
        SAMPLE_CA_IRD_ERR_PRINT("p_ProgInfo is NULL!\n");
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

    SAMPLE_CA_IRD_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_CA_IRD_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_CA_IRD_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_CA_IRD_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_CA_IRD_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay,MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC,(MT_VOID *)&DmxAvsync);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_CA_IRD_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
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
            SAMPLE_CA_IRD_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
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
#ifndef MT_SAMPLE_APP
static MT_S32 MT_IrdCakInjectTsTask(MT_VOID *args)
{
    MT_S32  ret = MT_SUCCESS;
    MT_U32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_file_param_t *pstParam = (source_file_param_t *)(args);

    SAMPLE_CA_IRD_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_CA_IRD_ERR_PRINT( "file %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_CA_IRD_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        g_bTaskQuit = MT_TRUE;
        SAMPLE_CA_IRD_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT( "failed to MT_UNF_DMX_GetTSBuffer  ret= %x \n", ret);
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_CA_IRD_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_CA_IRD_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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
#endif


/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
static MT_VOID MT_IrdCakStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if (MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_CA_IRD_ERR_PRINT("=====phAvplay == NULL=====\n");
    }

    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    (MT_VOID)MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}

static MT_VOID MT_IrdCakExit(void)
{
    g_bTaskQuit = MT_TRUE;
#ifndef MT_SAMPLE_APP
    /** Stop playing the show */
    (MT_VOID)MT_IrdCakStopplay(ird_run_info.hAvPlay);

    /** Audio and video player deinitialization */
    (MT_VOID)MT_IrdCakAvplayDeInit(ird_run_info.hAvPlay, ird_run_info.hWin, ird_run_info.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(ird_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == ird_run_info.sInputParam.sig_type)
    {
        /** Wait for the thread to end */
        pthread_join(ird_run_info.stInjectTSThread, NULL);
    }

    (MT_VOID)MT_IrdCakDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != ird_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
#endif
    memset(&ird_run_info, 0xff, sizeof(ird_run_info));
}

static MT_S32 MT_IrdTestTask(MT_VOID *args)
{
	MT_S32  ret = MT_SUCCESS;
	
	MT_IRD_TEST_INFO *pstIrdTest = (MT_IRD_TEST_INFO *)args;

	SAMPLE_CA_IRD_FUNCTION_ENTER();
	
	if(pstIrdTest->TestMode == MT_IRD_TEST_ONE)
	{
		IRD_Exec_Cmd(pstIrdTest->TestNumber);
	}
	else
	{
		IRD_BSP_Test_All();
	}

	SAMPLE_CA_IRD_FUNCTION_EXIT();

	return MT_SUCCESS;
}



#ifndef MT_SAMPLE_APP
static MT_VOID MT_IrdCakPrintDvbcHelp(MT_CHAR *name)
{
    SAMPLE_CA_IRD_PRINT("\nDvbc Usage:\n");
    SAMPLE_CA_IRD_PRINT("example:\n");
    SAMPLE_CA_IRD_PRINT("    %s -c 314 6875 64\n", name);
}

static MT_VOID MT_IrdCakPrintDvbsHelp(MT_CHAR *name)
{
    SAMPLE_CA_IRD_PRINT("\nDvbs Usage:\n");
    SAMPLE_CA_IRD_PRINT("example:\n");
    SAMPLE_CA_IRD_PRINT("    %s -s 3840 27500 1 0 0\n",name);
}

static MT_VOID MT_IrdCakPrintFileHelp(MT_CHAR *name)
{
    SAMPLE_CA_IRD_PRINT("\nFile Usage:\n");
    SAMPLE_CA_IRD_PRINT("example:\n");
    SAMPLE_CA_IRD_PRINT("    %s -f ./ttx.ts\n",name);
}
#endif

static MT_VOID MT_IrdCakPrint_help(MT_CHAR *name)
{
	SAMPLE_CA_IRD_PRINT("Lack of parameters\n");
    SAMPLE_CA_IRD_PRINT("\nUsage:\n");
    SAMPLE_CA_IRD_PRINT("%s\n", name);
	
#ifndef MT_SAMPLE_APP
    
    SAMPLE_CA_IRD_PRINT("    -f: path of the stream file\n");
    SAMPLE_CA_IRD_PRINT("    -c: DVBC locks frequency\n");
    SAMPLE_CA_IRD_PRINT("    -s: DVBS locks frequency\n");
#else
	SAMPLE_CA_IRD_PRINT("    -d: Irtedo Descrambler mode\n");
	SAMPLE_CA_IRD_PRINT("    -t: Irtedo Test case mode\n");
    SAMPLE_CA_IRD_PRINT("    -q: Exit the background\n");
#endif
	SAMPLE_CA_IRD_PRINT("example:\n");
#ifndef MT_SAMPLE_APP
    SAMPLE_CA_IRD_PRINT("    %s -d -f ./ttx.ts\n",name);
    SAMPLE_CA_IRD_PRINT("    %s -d -c 314 6875 64\n",name);
    SAMPLE_CA_IRD_PRINT("    %s -d -s 3840 27500 1 0 0\n",name);
#else
	SAMPLE_CA_IRD_PRINT("    %s -d \n",name);
#endif
	SAMPLE_CA_IRD_PRINT("    %s -t \n",name);
	

}

static MT_VOID MT_IrdCakMenuHelp(MT_U32 prog_num)
{
    SAMPLE_CA_IRD_PRINT("\n");
    SAMPLE_CA_IRD_PRINT(" 1 - %d : select the program \n", prog_num);
    SAMPLE_CA_IRD_PRINT("     e : enable ird cak\n");
	SAMPLE_CA_IRD_PRINT("     o : open monitor\n");
	SAMPLE_CA_IRD_PRINT("     n : close monitor\n");
	SAMPLE_CA_IRD_PRINT("     d : close ird menu\n");
	SAMPLE_CA_IRD_PRINT("     s : show client status menu\n");
	SAMPLE_CA_IRD_PRINT("     p : show product status menu\n");
	SAMPLE_CA_IRD_PRINT("     m : show emm service menu\n");
	SAMPLE_CA_IRD_PRINT("     a : show descramble service menu\n");
	SAMPLE_CA_IRD_PRINT("     c : cancel fsu\n");
	SAMPLE_CA_IRD_PRINT("     v : change secure core type\n");

#ifdef MT_SAMPLE_APP
    SAMPLE_CA_IRD_PRINT("     b : background run \n");
#endif
    SAMPLE_CA_IRD_PRINT("     h : help \n");
    SAMPLE_CA_IRD_PRINT("     q : quit \n");
    SAMPLE_CA_IRD_PRINT("=============================\n");
    SAMPLE_CA_IRD_PRINT("CA_IRD>> ");
}

static MT_VOID MT_IrdTestMenuHelp()
{
	SAMPLE_CA_IRD_PRINT("\n");
	
#ifdef MT_SAMPLE_APP
    SAMPLE_CA_IRD_PRINT("     b : background run \n");
#endif
	SAMPLE_CA_IRD_PRINT("     a : All test of ird \n");
	SAMPLE_CA_IRD_PRINT("     t : One case test of ird \n");


    SAMPLE_CA_IRD_PRINT("     h : help \n");
    SAMPLE_CA_IRD_PRINT("     q : quit \n");
    SAMPLE_CA_IRD_PRINT("=============================\n");
    SAMPLE_CA_IRD_PRINT("IRD_TEST>> ");
}
/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@param[in] ppProgTable,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_VOID MT_IrdCakCmdTask(MT_HANDLE hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_CHAR            *pfgetret = NULL;
    MT_CHAR            inputCmd[32] ={ 0 };
    MT_S32             ret = MT_FAILURE;
    MT_U32             u32ProgNum = 0;
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;

    if (MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_CA_IRD_ERR_PRINT("=====phAvplay == NULL=====\n");
    }

    if(NULL == pProgTbl)
    {
        SAMPLE_CA_IRD_ERR_PRINT("p_ProgInfo is NULL!\n");
    }

#ifdef MT_SAMPLE_APP
	u32ProgNum = play_resource.s32ProgNum;
	pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);
#endif

    while(1)
    {
        MT_IrdCakMenuHelp(pProgTbl->prog_num);

        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_CA_IRD_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
		else if('h' == inputCmd[0])
        {
            SAMPLE_CA_IRD_INFO_PRINT("Print help info \n");
            continue;
        }
		
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_CA_IRD_INFO_PRINT("set ird cak in back!\n");
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
                (MT_VOID)MT_IrdCakStopplay(hAvPlay);

                SAMPLE_CA_IRD_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);
                // restore ac4    attr info.
                MTADP_AUD_RestoreAc4PlayAttrInfo(hAvPlay);
                ret  = MT_IrdCakAVPlay_Start(hAvPlay, pstCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_CA_IRD_ERR_PRINT("Switching shows failed\n");
                    continue;
                }
#ifdef MT_SAMPLE_APP
                play_resource.s32ProgNum = u32ProgNum;
#endif
            }
            else
            {
                SAMPLE_CA_IRD_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }
        }

        else if ('e' == inputCmd[0])
        {
        	SAMPLE_CA_IRD_INFO_PRINT("Enable IRD Cas descramble...\n");

			MT_IrdCakSetCasInfo(pstCurrentProgInfo);

        }
		else if ('d' == inputCmd[0])
		{
			cas_irdeto_close_menu();
		}
		else if ('o' == inputCmd[0])
		{
			cas_irdeto_io_ctrl(CAS_IOCMD_OPEN_MONITOR,NULL); 
		}
		else if ('n' == inputCmd[0])
		{
			cas_irdeto_io_ctrl(CAS_IOCMD_CLOSE_MONITOR,NULL); 
		}		
		else if ('s' == inputCmd[0])
		{
			cas_irdeto_show_client_status();
		}
		else if ('p' == inputCmd[0])
		{		
			cas_irdeto_show_product_status();
		}
		else if ('m' == inputCmd[0])
		{
			cas_irdeto_show_emm_service();
		}
		else if ('a' == inputCmd[0])
		{
			cas_irdeto_show_descramble_service();
		}
		else if ('c' == inputCmd[0])  //cancel fsu
		{
			cas_irdeto_close_menu();
			cas_irdeto_cancel_fsu();
		}
		else if ('v' == inputCmd[0]) //change secorecore
		{
			cas_irdeto_change_securecore();
		}
		
    }
}


static MT_VOID MT_IrdTestCmdTask()
{
	MT_S32             ret = MT_FAILURE;
	MT_CHAR            *pfgetret = NULL;
    MT_CHAR            inputCmd[32] ={ 0 };
	MT_U32			   u32TestNum = 0;
	MT_IRD_TEST_INFO   stIrdTestCase = {0};

	SAMPLE_CA_IRD_FUNCTION_ENTER();

	
	while(1)
    {
        MT_IrdTestMenuHelp();

        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_CA_IRD_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
		else if('h' == inputCmd[0])
        {
            SAMPLE_CA_IRD_INFO_PRINT("Print help info \n");
            continue;
        }
		else if ('a' == inputCmd[0])
		{
			memset(&stIrdTestCase, 0, sizeof(MT_IRD_TEST_INFO));
			stIrdTestCase.TestMode = MT_IRD_TEST_ALL;
			ret = pthread_create(&ird_run_info.stIrdTestThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_IrdTestTask, &stIrdTestCase);
			if(MT_SUCCESS != ret)
            {
                SAMPLE_CA_IRD_ERR_PRINT("failed to pthread_create\n");
                continue;
            }
			pthread_join(ird_run_info.stIrdTestThread, NULL);
		}
		else if ('t' == inputCmd[0])
		{
			SAMPLE_CA_IRD_INFO_PRINT("Please input case number: \n");
			memset(&stIrdTestCase, 0, sizeof(MT_IRD_TEST_INFO));
			scanf("%d", &u32TestNum);
			stIrdTestCase.TestMode  = MT_IRD_TEST_ONE;
			stIrdTestCase.TestNumber = u32TestNum;
			ret = pthread_create(&ird_run_info.stIrdTestThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_IrdTestTask, &stIrdTestCase);
			if(MT_SUCCESS != ret)
            {
                SAMPLE_CA_IRD_ERR_PRINT("failed to pthread_create\n");
                continue;
            }
			pthread_join(ird_run_info.stIrdTestThread, NULL);
		}
	}

	SAMPLE_CA_IRD_FUNCTION_EXIT();

	return MT_SUCCESS;
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_IrdCakParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

#ifndef MT_SAMPLE_APP
    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_IrdCakPrint_help(argv[0]);
        return MT_FAILURE;
    }
#endif
    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s:tdq")) != -1)
    {
    	SAMPLE_CA_IRD_INFO_PRINT("%c\n", opt);
        switch(opt)
        {

            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_IrdCakPrint_help(argv[0]);
                return MT_FAILURE;
#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_IrdCakPrintFileHelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_IrdCakPrintDvbsHelp(argv[0]);
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
                    (MT_VOID)MT_IrdCakPrintDvbcHelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
#endif
			case 't':
				pInputParam->mode = MT_IRD_TEST_MODE;
				return MT_SUCCESS;
			case 'd':
				pInputParam->mode = MT_IRD_DESC_MODE;
				break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_IrdCakExit();
                }
                return MT_TASK_EXIT;

            default:
                (MT_VOID)MT_IrdCakPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_CaIrdMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif

{
    MT_S32     ret = MT_SUCCESS;
#ifndef MT_SAMPLE_APP
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#endif

    ret = MT_IrdCakParase_args(argc, argv,&ird_run_info.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_CA_IRD_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        g_bTaskQuit = MT_FALSE;
#ifndef MT_SAMPLE_APP
        /** System initialization */
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to mt_sys_init\n");
            return ret;
        }

        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to StartDmx\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }


        if(MT_INPUT_SIG_TYPE_FILE != ird_run_info.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_CA_IRD_ERR_PRINT("MTADP_Fe_Init failed, ret = %d\n", ret);
                goto ERR2;
            }

            if (MT_INPUT_SIG_TYPE_CAB == ird_run_info.sInputParam.sig_type)
            {     //dvbc
                ret = MT_IrdCakCheckDvbcParam(&ird_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_CA_IRD_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR3;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            ird_run_info.sInputParam.input_param.cab.freq,
                                            ird_run_info.sInputParam.input_param.cab.sym_rate,
                                            ird_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == ird_run_info.sInputParam.sig_type)
            {
                ret = MT_IrdCakCheckDvbsParam(&ird_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_CA_IRD_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR3;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            ird_run_info.sInputParam.input_param.sat.freq,
                                            ird_run_info.sInputParam.input_param.sat.sym_rate,
                                            ird_run_info.sInputParam.input_param.sat.onoff_22k,
                                            ird_run_info.sInputParam.input_param.sat.polarization,
                                            ird_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_CA_IRD_ERR_PRINT("MTADP_Fe_Connect failed, ret = %d\n", ret);
                goto ERR3;
            }
        }


        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR4;
        }

        ret = MT_IrdCakDmxInit(&ird_run_info.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT( "failed to StartDmx\n");
            goto ERR5;
        }

        if(MT_INPUT_SIG_TYPE_FILE == ird_run_info.sInputParam.sig_type)
        {
            ret = pthread_create(&ird_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_IrdCakInjectTsTask, &ird_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_CA_IRD_ERR_PRINT("failed to pthread_create\n");
                goto ERR6;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR6;
            }
        }

        (MT_VOID)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &ird_run_info.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR8;
        }

        ret = MT_IrdCakAVplayInit(&ird_run_info.hAvPlay, &ird_run_info.hWin, &ird_run_info.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to StartAVPlay\n");
            goto ERR9;
        }

        pstCurrentProgInfo = ird_run_info.pProgTbl->proginfo;
        ret = MT_IrdCakAVPlay_Start(ird_run_info.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CA_IRD_ERR_PRINT("failed to MTADP_AVPlay_PlayProg\n");
            goto ERR10;
        }
#endif

		if(MT_IRD_DESC_MODE == ird_run_info.sInputParam.mode)
		{
#ifdef MT_SAMPLE_APP
	        ird_run_info.hAvPlay = avplayHandle.hAvPlay;
			MTADP_Search_get_proglist(&ird_run_info.pProgTbl);
#endif

			cas_irdeto_init();
		}
		else if(MT_IRD_TEST_MODE == ird_run_info.sInputParam.mode)
		{
		}
    }

	if(MT_IRD_DESC_MODE == ird_run_info.sInputParam.mode)
	{
		/* Play the first program on the program list*/
	    (MT_VOID) MT_IrdCakCmdTask(ird_run_info.hAvPlay, ird_run_info.pProgTbl);

	    if(g_bTaskQuit != MT_TRUE)
	    {
	        return MT_TASK_RUN;
	    }
	
		cas_irdeto_io_ctrl(CAS_CMD_STOP, NULL);
		MT_IrdCakStopplay(ird_run_info.hAvPlay);
		cas_irdeto_io_ctrl(CAS_IOCMD_RELEASE_TA_SLOT, NULL);

		cas_irdeto_deinit();
	}
	else if(MT_IRD_TEST_MODE == ird_run_info.sInputParam.mode)
	{
		MT_IrdTestCmdTask();
	}
	
#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_IrdCakStopplay(ird_run_info.hAvPlay);
    SAMPLE_CA_IRD_INFO_PRINT("stop to play\n");

ERR10:
    (MT_VOID)MT_IrdCakAvplayDeInit(ird_run_info.hAvPlay, ird_run_info.hWin, ird_run_info.hSoundTrack);
ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(ird_run_info.pProgTbl);
ERR8:
    (MT_VOID)MTADP_Search_DeInit();
ERR7:
    if(MT_INPUT_SIG_TYPE_FILE == ird_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(ird_run_info.stInjectTSThread, NULL);
    }
ERR6:
    (MT_VOID)MT_IrdCakDmxDeInit();
ERR5:
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    (MT_VOID)MTADP_Snd_DeInit();

ERR3:
    if(MT_INPUT_SIG_TYPE_FILE != ird_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR2:
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&ird_run_info, 0xff, sizeof(ird_run_info));

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return MT_SUCCESS;
}
