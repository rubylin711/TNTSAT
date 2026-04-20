/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mt_common.h"
#include "mt_unf_hdmi.h"
#include "mt_mpi_hdmi.h"
#include "mt_drv_struct.h"
#include "mt_drv_disp.h"
#include "mt_module_debug.h"

#include "mt_mpi_mem.h"
#include "mt_mpi_hdmi_fmt.h"

#include "drv_hdmi_ioctl.h"

#include "mt_list.h"
//#include "mt_drv_disp.h"

//#define HDMI_MPI_NOT_ARRANGED

//10s
#define MAX_DELAY_TIME_MS 10000

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	#define HDMI_SUPPORT_CEC 0
#else
	#define HDMI_SUPPORT_CEC 1 // sym2 sym4 enable cec
#endif


//change by zhouxiang, callback only can register one.
//#define HDMI_CALLBACK_SUPPORT_LIST 1

#define MT_HDMI_FORCE_PRINT   printf

#ifdef HDMI_CALLBACK_SUPPORT_LIST
typedef struct list_head List;

typedef struct mt_HDMI_CALLBACK_NODE_S {
	List list;
	MT_UNF_HDMI_CALLBACK_FUNC_S stCallbackfunc;
} HDMI_CALLBACK_NODE_S;

HDMI_CALLBACK_NODE_S g_pstHDMICallBackList;
#else

MT_UNF_HDMI_CALLBACK_FUNC_S *g_stCallbackfunc = NULL;
#endif

typedef struct {
	MT_BOOL bOpen;
	MT_BOOL bStart;
	MT_UNF_HDMI_ATTR_S stAttr;
	MT_UNF_HDMI_AVI_INFOFRAME_VER2_S stAVIInfoFrame;
	MT_UNF_HDMI_AUD_INFOFRAME_VER1_S stAUDInfoFrame;
} HDMI_CHN_USER_ATTR_S;

typedef struct {
	MT_BOOL bHdmiInit;
	MT_BOOL bEnableTimer;  /* Timer thread Flag */
	pthread_t tEventTimer; /* Timer thread ID */
	MT_BOOL bHdmiExit;     /* HDMI Exit Flag */
} HDMI_COMM_USER_ATTR_S;

typedef struct {
	MT_UNF_HDMI_ID_E enHdmi;
	MT_BOOL bCECEnable;
	pthread_t CECTread;
	MT_UNF_HDMI_CECCALLBACK pCECCallback;
} HDMI_CEC_ATTR_S;

static HDMI_CEC_ATTR_S g_stCECAttr;

/************************ for mta_hdmi hdmi driver  ****************************************/

/*!
  hdmi nofity information param
  */
typedef struct tag_hdmi_notify_param {
	/*!
	HDMI event ids this notify should supported, see hdmi_event_id_t
	*/
	mt_u32 event;
	/*!
	the param to be transfered back by calling event notify function
	*/
	mt_u32 param;
	/*!
	the context to be transfered back by calling event notify function
	*/
	mt_u32 context;
} hdmi_notify_param_t;
/*************************************************************************************/
//typedef struct tagMPI_HDMI_CALLBACK_S
//{
//mt_u32 u32CallbackAddr;
//MT_UNF_HDMI_CALLBACK_FUNC_S stCallbackFunc;
//}MPI_HDMI_CALLBACK_S;
//static MPI_HDMI_CALLBACK_S g_stCallback;

//static HDMI_AUDIO_ATTR_S    g_stHDMIAudAttr;

static mt_s32 g_HDMIDevFd = -1;
static const mt_char g_HDMIDevName[] = "/dev/" UMAP_DEVNAME_HDMI20;
static mt_u32 g_HDMIProcID = MT_INVALID_HANDLE; //defalult value: MT_INVALID_HANDLE == 0xffffffff

static HDMI_COMM_USER_ATTR_S g_stHdmiCommUserParam;

static HDMI_CHN_USER_ATTR_S g_stHdmiChnUserParam[MT_UNF_HDMI_ID_BUTT];

static pthread_mutex_t g_HDMIMutex = PTHREAD_MUTEX_INITIALIZER;

//static mt_u32 hdmi_Mutex_Count = 0;

#define MT_HDMI_LOCK()                           \
	do {                                         \
		(void) pthread_mutex_lock(&g_HDMIMutex); \
	} while (0)

#define MT_HDMI_UNLOCK()                           \
	do {                                           \
		(void) pthread_mutex_unlock(&g_HDMIMutex); \
	} while (0)

#define HDMI_CHECK_NULL_PTR(ptr)          \
	do {                                  \
		if (NULL == (ptr)) {              \
			MT_ERR_HDMI("poiner is NULL!!\n"); \
			return MT_ERR_HDMI_NUL_PTR;   \
		}                                 \
	} while (0)

#define HDMI_CHECK_ID(l_enHdmi)                         \
	do {                                                \
		if ((l_enHdmi) >= MT_UNF_HDMI_ID_BUTT) {        \
			MT_ERR_HDMI("enHdmi %d is invalid\n", l_enHdmi); \
			return MT_ERR_HDMI_INVALID_PARA;            \
		}                                               \
	} while (0)

#define HDMI_CheckChnOpen(l_HdmiID)                      \
	do {                                                 \
		if (MT_TRUE != g_stHdmiChnUserParam[0].bOpen) {  \
			MT_ERR_HDMI("enHdmi:%d do NOT open\n", l_HdmiID); \
			return MT_ERR_HDMI_DEV_NOT_OPEN;             \
		}                                                \
	} while (0)

static mt_void MT_MPI_UNF_HDMI_GenEvent(MT_UNF_HDMI_EVENT_TYPE_E event);
mt_void *Hdmi_Poll_Event_Thread(void *pParam);
mt_void Hdmi_Unf2DrvAttr(HDMI_APP_ATTR_S *pstDestAttr, MT_UNF_HDMI_ATTR_S *pstSrcAttr);
mt_void Hdmi_Drv2UnfAttr(MT_UNF_HDMI_ATTR_S *pstDestAttr, HDMI_APP_ATTR_S *pstSrcAttr);
mt_void *Hdmi_CEC_Event_Thread(mt_void *pParam);

#if 0
static mt_s32 MT_MPI_HDMI_INIT_INPUT_VIDEO_CFG(mt_void)
{
	#if 0 //def HDMI_MPI_NOT_ARRANGED
	hdmi_video_config_t v_config;
	mt_u32 ret = 0;

	v_config.input_v_cfg.fmt = 0;
	v_config.input_v_cfg.csc = 2;
	v_config.input_v_cfg.ddr_edge = 1;
	v_config.input_v_cfg.pixel_rpt = 1;

	v_config.output_v_cfg.resolution = 6;  //1080i
	v_config.output_v_cfg.shape = 2;   //16X9
	v_config.output_v_cfg.standard = 2;   //ntsc
	v_config.output_v_cfg.color_space = 3; //auto
	v_config.hdcp_on_off = 0;
	printf("MT_MPI_HDMI_INIT_INPUT_VIDEO_CFG ..............\n");
	ret = ioctl(g_HDMIDevFd, HDMI_VIDEO_CONFIG, &v_config);
	if ( ret < 0 ) {
		printf("ioctl failure %s, MT_MPI_HDMI_INIT_INPUT_VIDEO_CFG ERROR\n", mt_mta_hdmi_test);
		return MT_FAILURE;
	}
	#endif
	mt_u32 param = 0;
	mt_s32 ret = 0;
	MT_INFO_HDMI("MT_MPI_HDMI_INIT_INPUT_VIDEO_CFG ..............\n");

	ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_DEFAULT_VIDEO, (mt_void *)&param);
	if (ret < 0) {
		MT_ERR_HDMI(" MT_MPI_HDMI_INIT_INPUT_VIDEO_CFG ERROR\n");
		return MT_FAILURE;
	}
	return MT_SUCCESS;
}

static mt_s32 MT_MPI_HDMI_INIT_INPUT_AUDIO_CFG(mt_void)
{
	#if 0
	hdmi_input_audio_cfg_t  hdmi_acfg;
	mt_u32 ret = 0;

	hdmi_acfg.type = HDMI_AUDIO_I2S;
	hdmi_acfg.down_sample = AUDIO_NO_DOWN_SAMPLE;
	hdmi_acfg.i2s_cfg.sample_rate = HDMI_AUDIO_SR_48;   //default audio sample rate: 48 KHz
	hdmi_acfg.i2s_cfg.sample_size = HDMI_AUDIO_SAMPLE_SIZE_16;  //HDMI_AUDIO_SAMPLE_SIZE_16
	hdmi_acfg.i2s_cfg.dir_ctrl = 0;
	hdmi_acfg.i2s_cfg.justify_ctrl = 0;
	hdmi_acfg.i2s_cfg.shift_ctrl = 0;
	hdmi_acfg.i2s_cfg.ws_pol = 0;
	hdmi_acfg.i2s_cfg.channel_0_en = 1;
	hdmi_acfg.i2s_cfg.channel_1_en = 1;
	hdmi_acfg.i2s_cfg.channel_2_en = 1;
	hdmi_acfg.i2s_cfg.channel_3_en = 1;
	hdmi_acfg.i2s_cfg.mclk_fs_relation = 1;
	printf("MT_MPI_HDMI_INIT_INPUT_AUDIO_CFG ..............\n");
	ret = ioctl(g_HDMIDevFd, HDMI_AUDIO_CONFIG, &hdmi_acfg);
	if ( ret < 0 ) {
		printf("ioctl failure %s, MT_MPI_HDMI_INIT_INPUT_AUDIO_CFG ERROR !\n", mt_mta_hdmi_test);
		return MT_FAILURE;
	}
	#endif
	mt_u32 param = 0;
	mt_s32 ret = 0;
	MT_INFO_HDMI("MT_MPI_HDMI_INIT_INPUT_AUDIO_CFG ..............\n");
	ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_DEFAULT_AUDIO, (mt_void *)&param);
	if (ret < 0) {
		MT_ERR_HDMI("MT_MPI_HDMI_INIT_INPUT_AUDIO_CFG ERROR !\n");
		return MT_FAILURE;
	}
	return MT_SUCCESS;
}
#endif

mt_void *Hdmi_Poll_Event_Thread(void *pParam)
{
	mt_s32 Ret;
	HDMI_POLL_EVENT_S stPollEvent = { 0 };
	//hdmi_notify_param_t param = { 0 };
	mt_set_pthread_name(__FUNCTION__);
	//pParam = pParam; //just to move compile warring

	while (MT_FALSE == g_stHdmiCommUserParam.bHdmiExit) {
		if (MT_FALSE == g_stHdmiChnUserParam[0].bOpen) {
			MT_USLEEP(100 * 1000);
			continue;
		}

		memset(&stPollEvent, 0, sizeof(HDMI_POLL_EVENT_S));

		stPollEvent.enHdmi = MT_UNF_HDMI_ID_0;
		stPollEvent.u32ProcID = g_HDMIProcID;
		Ret = ioctl(g_HDMIDevFd, CMD_HDMI_POLL_EVENT, &stPollEvent);
		#ifdef HDMI_MPI_NOT_ARRANGED
		Ret = ioctl(g_HDMIDevFd, HDMI_NOTIFY_GET_EVENTS, &param);
		#endif
		if (Ret == MT_SUCCESS) {
			#ifdef HDMI_MPI_NOT_ARRANGED
			stPollEvent.Event = param.event;
			#endif
			if (stPollEvent.Event) {
				MT_INFO_HDMI("Get HDMI new event:%d\n", stPollEvent.Event);
				MT_MPI_UNF_HDMI_GenEvent(stPollEvent.Event);
			}
		}
		MT_USLEEP(80 * 1000);
	}

	return (void *)0;
}

mt_void Hdmi_Unf2DrvAttr(HDMI_APP_ATTR_S *pstDestAttr, MT_UNF_HDMI_ATTR_S *pstSrcAttr)
{
	pstDestAttr->bEnableHdmi = pstSrcAttr->bEnableHdmi;
	pstDestAttr->bEnableVideo = pstSrcAttr->bEnableVideo;
	pstDestAttr->bEnableAudio = pstSrcAttr->bEnableAudio;

	pstDestAttr->enVidOutMode = pstSrcAttr->enVidOutMode;
	pstDestAttr->enDeepColorMode = pstSrcAttr->enDeepColorMode;
	pstDestAttr->bxvYCCMode = pstSrcAttr->bxvYCCMode;
	pstDestAttr->bEnableAviInfoFrame = pstSrcAttr->bEnableAviInfoFrame;
	pstDestAttr->bEnableAudInfoFrame = pstSrcAttr->bEnableAudInfoFrame;
	pstDestAttr->bEnableSpdInfoFrame = pstSrcAttr->bEnableSpdInfoFrame;
	pstDestAttr->bEnableMpegInfoFrame = pstSrcAttr->bEnableMpegInfoFrame;

	//pstDestAttr->bDebugFlag           = pstSrcAttr->bDebugFlag;
	pstDestAttr->bHDCPEnable = pstSrcAttr->bHDCPEnable;
}

mt_void Hdmi_Drv2UnfAttr(MT_UNF_HDMI_ATTR_S *pstDestAttr, HDMI_APP_ATTR_S *pstSrcAttr)
{
	pstDestAttr->bEnableHdmi = pstSrcAttr->bEnableHdmi;
	pstDestAttr->bEnableVideo = pstSrcAttr->bEnableVideo;
	pstDestAttr->bEnableAudio = pstSrcAttr->bEnableAudio;

	pstDestAttr->enVidOutMode = pstSrcAttr->enVidOutMode;
	pstDestAttr->enDeepColorMode = pstSrcAttr->enDeepColorMode;
	pstDestAttr->bxvYCCMode = pstSrcAttr->bxvYCCMode;
	pstDestAttr->bEnableAviInfoFrame = pstSrcAttr->bEnableAviInfoFrame;
	pstDestAttr->bEnableSpdInfoFrame = pstSrcAttr->bEnableSpdInfoFrame;
	pstDestAttr->bEnableMpegInfoFrame = pstSrcAttr->bEnableMpegInfoFrame;
	pstDestAttr->bEnableAudInfoFrame = pstSrcAttr->bEnableAudInfoFrame;

	//pstDestAttr->bDebugFlag           = pstSrcAttr->bDebugFlag;
	pstDestAttr->bHDCPEnable = pstSrcAttr->bHDCPEnable;
}

/**
\brief
\attention \n
\param
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_Init(void)
{
	mt_s32 Ret = 0;
	HDMI_INIT_S stHDMIInit;
	HDMI_GET_PROCID_S stHDMIProcID;
	mt_u32 taskreturn;
	if ((g_HDMIDevFd != 0) && (g_HDMIDevFd != -1)) {
		/* HDMI have been initialized */
		MT_ERR_HDMI("HDMI_DEV have been initialized\n");
		return MT_SUCCESS;
	}

	#ifdef HDMI_CALLBACK_SUPPORT_LIST
	INIT_LIST_HEAD(&g_pstHDMICallBackList.list);
	memset(&g_stHdmiCommUserParam, 0, sizeof(HDMI_COMM_USER_ATTR_S));
	MT_INFO_HDMI("Add HDMI Lock to deal with mutex\n");
	#else
	g_stCallbackfunc = NULL;
	#endif
	//g_stHDMIAudAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;

	g_HDMIDevFd = open(g_HDMIDevName, O_RDWR | O_CLOEXEC);
	if (g_HDMIDevFd <= 0) {
		MT_ERR_HDMI("[%s_%d]open HDMI20 err: %s\n", __func__, __LINE__, g_HDMIDevName);
		//Ret = pthread_mutex_destroy(&g_HDMIMutex);
		return MT_ERR_HDMI_DEV_NOT_OPEN;
	}

	MT_INFO_HDMI("\n\n\n g_HDMIDevName[%s] have been opened \n\n\n", g_HDMIDevName);
	Ret = pthread_mutex_init(&g_HDMIMutex, NULL);
	MT_HDMI_LOCK();
	#if 1
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_INIT, &stHDMIInit);
	if ((Ret == MT_SUCCESS) || (Ret == MT_ERR_HDMI_CALLBACK_ALREADY)) {
		if (Ret == MT_ERR_HDMI_CALLBACK_ALREADY) {
			MT_INFO_HDMI("HDMI has been inited already \n");
		}

		/*if hdmi alreadyinit , hdmi have already created thread*/
		if (MT_FALSE == g_stHdmiCommUserParam.bHdmiInit) {
			Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_PROCID, &stHDMIProcID);
			if (Ret == MT_SUCCESS) {
				g_HDMIProcID = stHDMIProcID.u32ProcID;
				/* create hdmi task */
				taskreturn = pthread_create(&g_stHdmiCommUserParam.tEventTimer, MT_NULL,
											Hdmi_Poll_Event_Thread, MT_NULL);
				g_stHdmiCommUserParam.bEnableTimer = MT_TRUE;
				MT_INFO_HDMI("timer task return:0x%x\n", taskreturn);
			} else {
				MT_ERR_HDMI("Error:HDMI Process is full,can't get process ID\n");
				MT_ERR_HDMI("Error:HDMI Ret:%d,%d,%d\n", Ret, (mt_u32)stHDMIProcID.enHdmi, (mt_u32)stHDMIProcID.u32ProcID);
			}
		}
	} else {
		MT_ERR_HDMI("HDMI Init Error:0x%x\n", Ret);
		MT_HDMI_UNLOCK();
		Ret = pthread_mutex_destroy(&g_HDMIMutex);
		return Ret;
	}
	#else

	//MT_MPI_HDMI_INIT_INPUT_VIDEO_CFG();
	//MT_MPI_HDMI_INIT_INPUT_AUDIO_CFG();
	/* create hdmi task */
	taskreturn = (mt_u32)pthread_create(&g_stHdmiCommUserParam.tEventTimer, MT_NULL, Hdmi_Poll_Event_Thread, MT_NULL);
	g_stHdmiCommUserParam.bEnableTimer = MT_TRUE;
	MT_INFO_HDMI("timer task return:0x%x\n", taskreturn);
	g_HDMIProcID = 0;
	#endif

	g_stHdmiCommUserParam.bHdmiInit = MT_TRUE;
	g_stHdmiCommUserParam.bHdmiExit = MT_FALSE;
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief
\attention \n
\param
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_DeInit(void)
{
	mt_s32 Ret;
	HDMI_GET_PROCID_S stHDMIProcID;

	if (((g_HDMIDevFd == 0) || (g_HDMIDevFd == -1))) {
		/* HDMI have no initialized */
		return MT_SUCCESS;
	}

	if (MT_FALSE == g_stHdmiCommUserParam.bHdmiInit) {
		return MT_SUCCESS;
	}

	//del procID
	#if 1
	//reserved
	//stHDMIProcID.enHdmi = xxx;
	stHDMIProcID.u32ProcID = g_HDMIProcID;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_RELEASE_PROCID, &stHDMIProcID);
	if (Ret != MT_SUCCESS) {
		MT_ERR_HDMI("release hdmi procID failed\n");
		return Ret;
	}
	#endif
	g_HDMIProcID = MT_INVALID_HANDLE;

	/*exit thread*/
	g_stHdmiCommUserParam.bHdmiExit = MT_TRUE;

	if (g_stHdmiCommUserParam.bEnableTimer == MT_TRUE) {
		MT_INFO_HDMI("stop hdmi task\n");

		Ret = pthread_join(g_stHdmiCommUserParam.tEventTimer, NULL);
		g_stHdmiCommUserParam.bEnableTimer = MT_FALSE;
	}

	Ret = close(g_HDMIDevFd);
	g_HDMIDevFd = -1;
	memset(&g_stHdmiCommUserParam, 0, sizeof(HDMI_COMM_USER_ATTR_S)); //clean

	/*destory mutex */
	Ret = pthread_mutex_destroy(&g_HDMIMutex);

	return Ret;
}

/**
\brief
\attention \n
\param[in] enHdmi
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_OPEN_PARA_S *pstOpenPara)
{
	mt_s32 Ret = 0;
	HDMI_OPEN_S stHDMIOpen;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CHECK_NULL_PTR(pstOpenPara);

	if (g_stHdmiChnUserParam[enHdmi].bOpen) {
		return MT_SUCCESS;
	}

	MT_HDMI_LOCK();
	memset(&stHDMIOpen, 0, sizeof(HDMI_OPEN_S));
	stHDMIOpen.enDefaultMode = pstOpenPara->enDefaultMode;
	stHDMIOpen.enHdmi = enHdmi;
	stHDMIOpen.u32ProcID = g_HDMIProcID;
	stHDMIOpen.initParam = pstOpenPara->u32InitParam;
	#if 1
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_OPEN, &stHDMIOpen);
	if (MT_SUCCESS != Ret) {
		MT_INFO_HDMI("hdmi open err:%d\n", Ret);
		MT_HDMI_UNLOCK();
		return Ret;
	}
	#endif
	MT_HDMI_UNLOCK();
	g_stHdmiChnUserParam[enHdmi].bOpen = MT_TRUE; //Enable HDMI thread
	return MT_SUCCESS;
}

/**
\brief
\attention \n
\param[in] hHdmi
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 Ret = 0;
	HDMI_STOP_S stHDMIStop;
	HDMI_CLOSE_S stHDMIClose;

	HDMI_CHECK_ID(enHdmi);
	if (g_stHdmiChnUserParam[enHdmi].bOpen == MT_FALSE) {
		return MT_SUCCESS;
	}

	if (MT_TRUE == g_stHdmiChnUserParam[enHdmi].bStart) {
		MT_HDMI_LOCK();
		memset(&stHDMIStop, 0, sizeof(HDMI_STOP_S));
		stHDMIStop.enHdmi = enHdmi;
		Ret = ioctl(g_HDMIDevFd, CMD_HDMI_STOP, &stHDMIStop);
		g_stHdmiChnUserParam[enHdmi].bStart = MT_FALSE;
		MT_HDMI_UNLOCK();
	}

	if (MT_TRUE == g_stHdmiChnUserParam[enHdmi].bOpen) {
		MT_HDMI_LOCK();
		memset(&stHDMIClose, 0, sizeof(HDMI_CLOSE_S));
		#if 1
		stHDMIClose.enHdmi = enHdmi;
		Ret = ioctl(g_HDMIDevFd, CMD_HDMI_CLOSE, &stHDMIClose);
		if (Ret != MT_SUCCESS) {
			MT_HDMI_UNLOCK();
			return Ret;
		}
		#endif
		g_stHdmiChnUserParam[enHdmi].bOpen = MT_FALSE;
		MT_HDMI_UNLOCK();
	}

	return MT_SUCCESS;
}

/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pCapability
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkCap)
{
	mt_s32 Ret = 0;
	HDMI_SINK_CAPABILITY_S stSinkCap;

	HDMI_CHECK_NULL_PTR(pstSinkCap);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stSinkCap, 0, sizeof(HDMI_SINK_CAPABILITY_S));
	stSinkCap.enHdmi = enHdmi;
	#if 1
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SINK_CAPABILITY, &stSinkCap);
	if (Ret != MT_SUCCESS) {
		memset(pstSinkCap, 0, sizeof(MT_UNF_EDID_BASE_INFO_S));
		MT_HDMI_UNLOCK();
		return Ret;
	}
	#else
	//stSinkCap.SinkCap.bSupportAudioSpeaker = 0;
	stSinkCap.SinkCap.bSupportHdmi = MT_TRUE;

	#endif
	memcpy(pstSinkCap, &(stSinkCap.SinkCap), sizeof(MT_UNF_EDID_BASE_INFO_S));
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pstAttr
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr)
{
	mt_s32 s32Ret = 0;
	HDMI_PORT_ATTR_S stPortAttr;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstAttr);

	MT_HDMI_LOCK();
	memset(&stPortAttr, 0, sizeof(HDMI_PORT_ATTR_S));
	stPortAttr.enHdmi = enHdmi;
	//s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_ATTR, &stPortAttr);
	//stPortAttr.stHDMIAttr.stAttr= *pstAttr;
	Hdmi_Unf2DrvAttr(&stPortAttr.stHdmiAppAttr, pstAttr);
	////set auido mode in kernel. not in mpi or unf
	//stPortAttr.stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_BUTT;
	//g_stHDMIAudAttr.enSampleRate = pstAttr->enSampleRate;

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_ATTR, &stPortAttr);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}
	MT_HDMI_UNLOCK();

	#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	/* Warring: Below is just for test!!!! */
	if (pstAttr->bDebugFlag == 0x3fc) {
		//create HPD Event!!
		MT_MPI_UNF_HDMI_GenEvent(MT_UNF_HDMI_EVENT_HOTPLUG);
	}
	#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	return s32Ret;
}

/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pstAttr
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr)
{
	mt_s32 s32Ret = 0;
	HDMI_PORT_ATTR_S stPortAttr;

	HDMI_CHECK_NULL_PTR(pstAttr);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stPortAttr, 0, sizeof(HDMI_PORT_ATTR_S));
	memset(pstAttr, 0, sizeof(MT_UNF_HDMI_ATTR_S));
	stPortAttr.enHdmi = enHdmi;
	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_ATTR, &stPortAttr);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}
	Hdmi_Drv2UnfAttr(pstAttr, &stPortAttr.stHdmiAppAttr);
	//*pstAttr = stPortAttr.stHDMIAttr.stAttr;

	MT_HDMI_UNLOCK();
	return s32Ret;
}
/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pCECCmd
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_SetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd)
{
	#if HDMI_SUPPORT_CEC
	mt_s32 Ret = 0;
	HDMI_CEC_S stCEC;

	HDMI_CHECK_NULL_PTR(pCECCmd);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stCEC, 0, sizeof(HDMI_CEC_S));
	stCEC.enHdmi = enHdmi;
	memcpy(&(stCEC.CECCmd), pCECCmd, sizeof(MT_UNF_HDMI_CEC_CMD_S));
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_CEC, &stCEC);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	#endif
	return MT_SUCCESS;
}

mt_s32 MT_MPI_HDMI_GetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd, mt_u32 timeout)
{
	#if HDMI_SUPPORT_CEC
	mt_s32 Ret = 0;
	HDMI_CEC_S stCEC;

	HDMI_CHECK_NULL_PTR(pCECCmd);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stCEC, 0, sizeof(HDMI_CEC_S));
	stCEC.enHdmi = enHdmi;
	stCEC.timeout = timeout;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_CEC, &stCEC);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	memcpy(pCECCmd, &(stCEC.CECCmd), sizeof(MT_UNF_HDMI_CEC_CMD_S));
	MT_HDMI_UNLOCK();
	#endif

	return MT_SUCCESS;
}

mt_void *Hdmi_CEC_Event_Thread(mt_void *pParam)
{
	#if HDMI_SUPPORT_CEC
	//mt_s32 Ret;
	//MT_UNF_HDMI_CEC_CMD_S stCECCmd;
	mt_set_pthread_name(__FUNCTION__);
	while (MT_TRUE == g_stCECAttr.bCECEnable) {
		/*Ret = MT_MPI_HDMI_GetCECCommand(g_stCECAttr.enHdmi, &stCECCmd, 0);
		if (Ret == MT_SUCCESS)
		{
		    g_stCECAttr.pCECCallback(g_stCECAttr.enHdmi, &stCECCmd, MT_NULL);
		}*/
		#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
		else {
			MT_INFO_HDMI("Get CEC cmd failed\n");
		}
		#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
		MT_USLEEP(80 * 1000);
	}
	#endif

	return MT_NULL;
}

mt_s32 MT_UNF_HDMI_RegCECCallBackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CECCALLBACK pCECCallback)
{
	mt_s32 Ret = 0;
	#if HDMI_SUPPORT_CEC
	HDMI_CheckChnOpen(enHdmi);

	memset(&g_stCECAttr, 0, sizeof(HDMI_CEC_ATTR_S));
	g_stCECAttr.enHdmi = enHdmi;
	g_stCECAttr.pCECCallback = pCECCallback;

	Ret = pthread_create(&g_stCECAttr.CECTread, MT_NULL,
						 Hdmi_CEC_Event_Thread, MT_NULL);

	if (Ret == MT_SUCCESS) {
		g_stCECAttr.bCECEnable = MT_TRUE;
	}
	#endif

	return Ret;
}

mt_s32 MT_UNF_HDMI_UnRegCECCallBackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CECCALLBACK pCECCallback)
{
	#if HDMI_SUPPORT_CEC
	HDMI_CHECK_ID(enHdmi);

	if (pCECCallback != g_stCECAttr.pCECCallback) {
		MT_ERR_HDMI("hand err :0x%x != 0x%x\n", pCECCallback, g_stCECAttr.pCECCallback);
		return MT_FAILURE;
	}

	g_stCECAttr.bCECEnable = MT_FALSE;
	pthread_join(g_stCECAttr.CECTread, NULL);
	memset(&g_stCECAttr, 0, sizeof(HDMI_CEC_ATTR_S));
	#endif

	return MT_SUCCESS;
}

mt_s32 MT_MPI_HDMI_CECStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_STATUS_S *pStatus)
{
	#if HDMI_SUPPORT_CEC
	mt_s32 Ret = 0;
	HDMI_CEC_STATUS CECStatus;

	HDMI_CHECK_NULL_PTR(pStatus);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&CECStatus, 0, sizeof(HDMI_CEC_STATUS));
	CECStatus.enHdmi = enHdmi;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_CECSTATUS, &CECStatus);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	memcpy(pStatus, &(CECStatus.stStatus), sizeof(MT_UNF_HDMI_CEC_STATUS_S));
	MT_HDMI_UNLOCK();
	#endif

	return MT_SUCCESS;
}

mt_s32 MT_MPI_HDMI_CEC_Enable(MT_UNF_HDMI_ID_E enHdmi)
{
	#if HDMI_SUPPORT_CEC
	mt_s32 Ret = 0;
	mt_u32 u32CECFlag;
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	u32CECFlag = 1;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_CEC_ENABLE, &u32CECFlag);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	#endif

	return MT_SUCCESS;
}

mt_s32 MT_MPI_HDMI_CEC_Disable(MT_UNF_HDMI_ID_E enHdmi)
{
	#if HDMI_SUPPORT_CEC
	mt_s32 Ret = 0;
	mt_u32 u32CECFlag;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	u32CECFlag = 0;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_CEC_DISABLE, &u32CECFlag);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	#endif

	return MT_SUCCESS;
}

mt_s32 MT_MPI_HDMI_HDCP_Enable(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 Ret = 0;
	mt_u32 u32HDCPFlag;
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	u32HDCPFlag = 1;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_HDCP_ENABLE, &u32HDCPFlag);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();

	return MT_SUCCESS;
}

mt_s32 MT_MPI_HDMI_HDCP_Disable(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 Ret = 0;
	mt_u32 u32HDCPFlag;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	u32HDCPFlag = 0;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_HDCP_DISABLE, &u32HDCPFlag);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();

	return MT_SUCCESS;
}

#if 0
static mt_void MT_MPI_HDMI_SetInfoFrameForVideoResolution(MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame, mt_u8 *resolution, mt_u8 *standard)
{
	#if 0 //def HDMI_MPI_NOT_ARRANGED //should do it in drv
	switch (pstInfoFrame->unInforUnit.stAVIInfoFrame.enTimingMode) {
		case  MT_UNF_ENC_FMT_NTSC:
		case MT_UNF_ENC_FMT_NTSC_J:
			*resolution = HDMI_480I;
			*standard = HDMI_NTSC;
			break;
		case MT_UNF_ENC_FMT_NTSC_PAL_M:
			*resolution = HDMI_480I;
			*standard = HDMI_PAL;
			break;
		case  MT_UNF_ENC_FMT_PAL:
		case MT_UNF_ENC_FMT_PAL_N:
		case MT_UNF_ENC_FMT_PAL_Nc:
			*resolution =  HDMI_576I;
			*standard = HDMI_PAL;
			break;
		case  MT_UNF_ENC_FMT_480P_60:
			*resolution =  HDMI_480P;
			*standard = HDMI_NTSC;
			break;
		case  MT_UNF_ENC_FMT_576P_50:
			*resolution =  HDMI_576P;
			*standard = HDMI_PAL;
			break;

		case  MT_UNF_ENC_FMT_720P_50:
			*resolution =  HDMI_720P;
			*standard = HDMI_PAL;
			break;
		case  MT_UNF_ENC_FMT_720P_60:
			*resolution =  HDMI_720P;
			*standard = HDMI_NTSC;
			break;

		case MT_UNF_ENC_FMT_1080i_60:        /**<1080i 60 Hz*/
			*standard = HDMI_NTSC;
			*resolution =  HDMI_1080I;
			break;

		case MT_UNF_ENC_FMT_1080i_50:
			*resolution =  HDMI_1080I;
			*standard = HDMI_PAL;
			break;

		case  MT_UNF_ENC_FMT_1080P_60:     /**<1080p 60 Hz*/
			*standard = HDMI_NTSC;
			*resolution =   HDMI_1080P;
		case  MT_UNF_ENC_FMT_1080P_50:         /**<1080p 50 Hz*/
		case  MT_UNF_ENC_FMT_1080P_30:         /**<1080p 30 Hz*/
		case  MT_UNF_ENC_FMT_1080P_25:         /**<1080p 25 Hz*/
		case  MT_UNF_ENC_FMT_1080P_24:
			*resolution =   HDMI_1080P;
			*standard = HDMI_PAL;
			break;
		default:
			*resolution =   HDMI_OUTPUT_RESOLUTION_DEF;
			break;
	}
	#endif
	return;
}
#endif
/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pstInfoFrame
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_s32 Ret = 0;
	HDMI_INFORFRAME_S stInfoFrame;

	HDMI_CHECK_NULL_PTR(pstInfoFrame);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	#if 1
	memset(&stInfoFrame, 0, sizeof(HDMI_INFORFRAME_S));
	stInfoFrame.enHdmi = enHdmi;
	stInfoFrame.enInfoFrameType = pstInfoFrame->enInfoFrameType;
	memcpy(&(stInfoFrame.InfoFrame), pstInfoFrame, sizeof(MT_UNF_HDMI_INFOFRAME_S));
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_INFORFRAME, &stInfoFrame);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	#else
	if (pstInfoFrame->enInfoFrameType == MT_INFOFRAME_TYPE_AVI) {
		#if 0 //def HDMI_MPI_NOT_ARRANGED//should do it in drv_layer
		hdmi_video_config_t v_config;
		mt_u32 ret = 0;
		mt_u8 resolution = 0;
		mt_u8 standard = 0;

		v_config.input_v_cfg.fmt = 0;
		v_config.input_v_cfg.csc = 2;
		v_config.input_v_cfg.ddr_edge = 1;
		v_config.input_v_cfg.pixel_rpt = 1;

		MT_MPI_HDMI_SetInfoFrameForVideoResolution(pstInfoFrame, &resolution, &standard);
		v_config.output_v_cfg.resolution = resolution;
		v_config.output_v_cfg.standard = standard;

		if (pstInfoFrame->unInforUnit.stAVIInfoFrame.enActiveAspectRatio >= 0
				&& pstInfoFrame->unInforUnit.stAVIInfoFrame.enActiveAspectRatio <= 2) {
			v_config.output_v_cfg.shape = pstInfoFrame->unInforUnit.stAVIInfoFrame.enActiveAspectRatio;
		} else {
			v_config.output_v_cfg.shape = 2;
		}
		/* HDMI_COLOR_RGB  = 0,
		    HDMI_COLOR_YUV422,
		    HDMI_COLOR_YUV444,*/
		v_config.output_v_cfg.color_space = pstInfoFrame->unInforUnit.stAVIInfoFrame.enOutputType;
		v_config.hdcp_on_off = 0;

		ret = ioctl(g_HDMIDevFd, HDMI_VIDEO_CONFIG, &v_config);
		if ( ret < 0 ) {
			printf("ioctl failure %s, MT_MPI_HDMI_INIT_INPUT_VIDEO_CFG ERROR\n", mt_mta_hdmi_test);
			MT_HDMI_UNLOCK();
			return MT_FAILURE;
		}
		#endif
	} else if (pstInfoFrame->enInfoFrameType == MT_INFOFRAME_TYPE_AUDIO) {
		#ifdef HDMI_MPI_NOT_ARRANGED
		hdmi_input_audio_cfg_t hdmi_acfg;
		mt_u32 ret = 0;

		if (pstInfoFrame->unInforUnit.stAUDInfoFrame.enCodingType == MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM) {
			hdmi_acfg.type = HDMI_AUDIO_SPDIF;
		} else {
			hdmi_acfg.type = HDMI_AUDIO_I2S;
		}
		hdmi_acfg.down_sample = AUDIO_NO_DOWN_SAMPLE;
		hdmi_acfg.i2s_cfg.sample_rate = pstInfoFrame->unInforUnit.stAUDInfoFrame.u32SampleSize;
		hdmi_acfg.i2s_cfg.sample_size = pstInfoFrame->unInforUnit.stAUDInfoFrame.u32SamplingFrequency;
		hdmi_acfg.i2s_cfg.dir_ctrl = 0;
		hdmi_acfg.i2s_cfg.justify_ctrl = 0;
		hdmi_acfg.i2s_cfg.shift_ctrl = 0;
		hdmi_acfg.i2s_cfg.ws_pol = 0;
		hdmi_acfg.i2s_cfg.channel_0_en = 1;
		hdmi_acfg.i2s_cfg.channel_1_en = 1;
		hdmi_acfg.i2s_cfg.channel_2_en = 1;
		hdmi_acfg.i2s_cfg.channel_3_en = 1;
		hdmi_acfg.i2s_cfg.mclk_fs_relation = 1;

		printf("%s %d\n", __FUNCTION__, __LINE__);
		ret = ioctl(g_HDMIDevFd, HDMI_AUDIO_CONFIG, &hdmi_acfg);
		if (ret < 0) {
			printf("ioctl failure %s, MT_MPI_HDMI_INIT_INPUT_AUDIO_CFG ERROR !\n", mt_mta_hdmi_test);
			MT_HDMI_UNLOCK();
			return MT_FAILURE;
		}
		#endif

	} else {
		printf("enInfoFrameType ERROR !  enInfoFrameType = %d\n", pstInfoFrame->enInfoFrameType);
	}
	#endif
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pstInfoFrame
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_s32 Ret = 0;
	HDMI_INFORFRAME_S stInfoFrame;

	HDMI_CHECK_NULL_PTR(pstInfoFrame);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stInfoFrame, 0, sizeof(HDMI_INFORFRAME_S));
	stInfoFrame.enHdmi = enHdmi;
	stInfoFrame.enInfoFrameType = enInfoFrameType;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_INFORFRAME, &stInfoFrame);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	memcpy(pstInfoFrame, &(stInfoFrame.InfoFrame), sizeof(MT_UNF_HDMI_INFOFRAME_S));
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief HOT-PLUG Callback function, it will invoke Customer callback function.
\attention Customer callback function is register in MT_UNF_HDMI_Init
\param[in] none
\retval none
\see \n
*/
static void MT_MPI_UNF_HDMI_GenEvent(MT_UNF_HDMI_EVENT_TYPE_E event)
{
	#ifdef HDMI_CALLBACK_SUPPORT_LIST
	HDMI_CALLBACK_NODE_S *tmp;
	List *pos;

	MT_INFO_HDMI("HDMI EVENT TYPE:0x%x\n", event);
	list_for_each(pos, &g_pstHDMICallBackList.list) {
		tmp = list_entry(pos, HDMI_CALLBACK_NODE_S, list);
		tmp->stCallbackfunc.pfnHdmiEventCallback(event, tmp->stCallbackfunc.pPrivateData);
	}
	#else
	MT_INFO_HDMI("HDMI EVENT TYPE:0x%x\n", event);
	printf("zx HDMI EVENT TYPE:0x%x\n", event);
	if (g_stCallbackfunc) {
		g_stCallbackfunc->pfnHdmiEventCallback(event, g_stCallbackfunc->pPrivateData);
	} else {
		MT_ERR_HDMI("HDMI EVENT TYPE:0x%x, no register callback\n", event);
	}
	#endif
}

/**
\brief Start HDMI output. It should be invoked within User callback.
\attention \n
\param[in] hHdmi
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_Start(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 Ret = 0;
	HDMI_START_S stHDMIStart;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	if (g_stHdmiChnUserParam[enHdmi].bStart == MT_TRUE) {
		MT_INFO_HDMI("MT_MPI_HDMI_Start Already Start before!\n");
	}

	MT_HDMI_LOCK();
	memset(&stHDMIStart, 0, sizeof(HDMI_START_S));
	stHDMIStart.enHdmi = enHdmi;
	#if 1
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_START, &stHDMIStart);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	#endif
	g_stHdmiChnUserParam[enHdmi].bStart = MT_TRUE;
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief Stop HDMI output
\attention \n
\param[in] hHdmi
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_Stop(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 Ret = 0;
	HDMI_STOP_S stHDMIStop;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	if (g_stHdmiChnUserParam[enHdmi].bStart == MT_FALSE) {
		return MT_SUCCESS;
	}

	MT_HDMI_LOCK();
	if (MT_TRUE != g_stHdmiChnUserParam[enHdmi].bStart) {
		MT_HDMI_UNLOCK();
		return MT_SUCCESS;
	}
	memset(&stHDMIStop, 0, sizeof(HDMI_STOP_S));
	stHDMIStop.enHdmi = enHdmi;
	#if 1
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_STOP, &stHDMIStop);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	#endif
	g_stHdmiChnUserParam[enHdmi].bStart = MT_FALSE;
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief Set DeepColor mode
\attention \n
\param[in] enHdmi
\param[in] enDeepColor, please refer to:MT_UNF_HDMI_DEEP_COLOR_E
\retval MT_SUCCESS
\see \n
*/
mt_s32 MT_MPI_HDMI_SetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E enDeepColor)
{
	mt_s32 Ret = 0;
	HDMI_DEEPCOLORC_S stHDMIDeepcolor;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stHDMIDeepcolor, 0, sizeof(HDMI_DEEPCOLORC_S));
	stHDMIDeepcolor.enHdmi = enHdmi;
	stHDMIDeepcolor.enDeepColor = enDeepColor;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_DEEPCOLOR, &stHDMIDeepcolor);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief Get DeepColor mode
\attention \n
\param[in] enHdmi
\param[in] enDeepColor, please refer to:MT_UNF_HDMI_DEEP_COLOR_E
\retval MT_SUCCESS
\see \n
*/
mt_s32 MT_MPI_HDMI_GetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E *penDeepColor)
{
	mt_s32 Ret = 0;
	HDMI_DEEPCOLORC_S stHDMIDeepcolor;

	HDMI_CHECK_NULL_PTR(penDeepColor);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stHDMIDeepcolor, 0, sizeof(HDMI_DEEPCOLORC_S));
	stHDMIDeepcolor.enHdmi = enHdmi;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_DEEPCOLOR, &stHDMIDeepcolor);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	*penDeepColor = stHDMIDeepcolor.enDeepColor;

	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief Set hdmi color space mode
\attention \n
\param[in] enHdmi
\param[in] enColorSpace, please refer to:MT_UNF_HDMI_VIDEO_MODE_E
\retval MT_SUCCESS
\see \n
*/
mt_s32 MT_MPI_HDMI_SetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E enColorSpace)
{
	mt_s32 Ret = 0;
	HDMI_COLORSPACE_S stHDMIColorSpace;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stHDMIColorSpace, 0, sizeof(HDMI_COLORSPACE_S));
	stHDMIColorSpace.enHdmi = enHdmi;
	stHDMIColorSpace.enColorSapce = enColorSpace;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_COLORSPACE, &stHDMIColorSpace);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief Get HDMI color space mode
\attention \n
\param[in] enHdmi
\param[in] enColorSpace, please refer to:MT_UNF_HDMI_VIDEO_MODE_E
\retval MT_SUCCESS
\see \n
*/
mt_s32 MT_MPI_HDMI_GetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E *enColorSpace)
{
	mt_s32 Ret = 0;
	HDMI_COLORSPACE_S stHDMIColorSpace;

	HDMI_CHECK_NULL_PTR(enColorSpace);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stHDMIColorSpace, 0, sizeof(HDMI_COLORSPACE_S));
	stHDMIColorSpace.enHdmi = enHdmi;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_COLORSPACE, &stHDMIColorSpace);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	*enColorSpace = stHDMIColorSpace.enColorSapce;

	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief set video xvYCC mode
\attention \n
\param[in] enHdmi
\param[in] bEnable   enable xvYCC mode flag.
\retval MT_SUCCESS
\see \n
*/
mt_s32 MT_MPI_HDMI_SetxvYCCMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bEnable)
{
	mt_s32 Ret = 0;
	HDMI_SET_XVYCC_S stxvYCC;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stxvYCC, 0, sizeof(HDMI_SET_XVYCC_S));
	stxvYCC.enHdmi = enHdmi;
	stxvYCC.xvYCCEnable = bEnable;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_XVYCC, &stxvYCC);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

/**
\brief set HDMI AV mute
\attention \n
\param[in] enHdmi
\param[in] bAvMute   avmute flag
\retval MT_SUCCESS
\see \n
*/
mt_s32 MT_MPI_HDMI_SetAVMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute)
{
	mt_s32 Ret = 0;
	HDMI_AVMUTE_S stAVMute;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	#if 1
	memset(&stAVMute, 0, sizeof(HDMI_AVMUTE_S));
	stAVMute.enHdmi = enHdmi;
	stAVMute.AVMuteEnable = bAvMute;

	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_AVMUTE, &stAVMute);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}

	#endif
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 MT_MPI_HDMI_Pre_SetFormat(MT_DRV_DISP_FMT_E enEncodingFormat)
{
	mt_s32 Ret = 0;
	HDMI_PREVIDEOTIMING_S stPreVideoTiming;
	MT_UNF_HDMI_ID_E  enHdmi = MT_UNF_HDMI_ID_0;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stPreVideoTiming, 0, sizeof(HDMI_PREVIDEOTIMING_S));
	stPreVideoTiming.enHdmi          = MT_UNF_HDMI_ID_0;
	stPreVideoTiming.VideoTiming     = enEncodingFormat;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_PREVTIMING, &stPreVideoTiming);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

mt_s32 MT_MPI_HDMI_AVMute(void)
{
	mt_s32 Ret = 0;

	Ret = MT_MPI_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);

	return Ret;
}

mt_s32 MT_MPI_HDMI_AVUnMute(void)
{
	mt_s32 Ret = 0;

	Ret = MT_MPI_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_FALSE);

	return Ret;
}

#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 MT_MPI_HDMI_SetFormat(MT_DRV_DISP_FMT_E enEncodingFormat)
{
	mt_s32 Ret = 0;
	HDMI_VIDEOTIMING_S stVideoTiming;
	MT_UNF_HDMI_ID_E  enHdmi = MT_UNF_HDMI_ID_0;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	memset(&stVideoTiming, 0, sizeof(HDMI_VIDEOTIMING_S));
	stVideoTiming.enHdmi          = MT_UNF_HDMI_ID_0;
	stVideoTiming.VideoTiming     = enEncodingFormat;
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_VIDEO_TIMING, &stVideoTiming);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 MT_MPI_HDMI_PlayStus(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32Stutus)
{
	mt_s32 Ret = 0;
	HDMI_PLAYSTAUS_S PlayStaus;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	if (g_stHdmiChnUserParam[enHdmi].bOpen == MT_TRUE) {
		memset(&PlayStaus, 0, sizeof(HDMI_PLAYSTAUS_S));
		PlayStaus.enHdmi = enHdmi;
		Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_HDMI_PLAYSTAUS, &PlayStaus);
		if (Ret != MT_SUCCESS) {
			*pu32Stutus = MT_FALSE;
			return Ret;
		}
		*pu32Stutus = PlayStaus.u32PlayStaus;
	} else {
		*pu32Stutus = 0;
	}

	return MT_SUCCESS;
}
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

mt_s32 MT_MPI_HDMI_Force_GetEDID(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *u8Edid, mt_u32 *u32EdidLength)
{
	mt_s32 Ret = 0;
	HDMI_EDID_S EDIDData;
	MT_INFO_HDMI("==================\n");

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(u8Edid);
	HDMI_CHECK_NULL_PTR(u32EdidLength);

	*u32EdidLength = 0;

	MT_INFO_HDMI("==================\n");

	MT_HDMI_LOCK();
	memset(&EDIDData, 0, sizeof(HDMI_EDID_S));
	EDIDData.enHdmi = enHdmi;

	MT_INFO_HDMI("==================\n");
	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_FORCE_GET_EDID, &EDIDData);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		MT_INFO_HDMI("==================\n");
		return Ret;
	}
	MT_HDMI_UNLOCK();

	if (EDIDData.u8EdidValid == MT_TRUE) {
		memcpy(u8Edid, &(EDIDData.stEdidParsed), sizeof(mt_unf_hdmi_edid_t));
	}

	return Ret;
}

mt_s32 MT_MPI_HDMI_ReadEDID(mt_u8 *u8Edid, mt_u32 *u32EdidLength)
{
	mt_s32 ret = 0;
	HDMI_EDID_S EdidData;
	MT_INFO_HDMI("==================\n");

	HDMI_CHECK_NULL_PTR(u8Edid);
	HDMI_CHECK_NULL_PTR(u32EdidLength);

	MT_INFO_HDMI("==================\n");

	#if 1
	EdidData.enHdmi = MT_UNF_HDMI_ID_0;
	memset(&EdidData, 0, sizeof(HDMI_EDID_S));
	ret = MT_MPI_HDMI_Force_GetEDID(EdidData.enHdmi, (mt_u8*)&EdidData.stEdidParsed, &EdidData.u32Edidlength);

	if (ret != MT_SUCCESS) {
		MT_ERR_HDMI("forec get edid fail!\n");
		return ret;
	}

	memcpy(u8Edid, &(EdidData.stEdidParsed), sizeof(mt_unf_hdmi_edid_t));
	#else
	#if 0
	hdmi_edid_result_t  edid;
	memset(&edid, 0, sizeof(edid));
	ret = ioctl(g_HDMIDevFd, HDMI_EDID_GET, &edid);
	if ( ret < 0 ) {
		printf("get edid fail ioctl failure %s, error !\n", mt_mta_hdmi_test);
		return -1;
	}
	printf("%s mode\n", edid.is_hdmi ? "HDMI" : "DVI");
	printf("YCbCr422   is %ssupported\n", edid.ycbcr422_supported ? "" : "not ");
	printf("YCbCr444   is %ssupported\n", edid.ycbcr444_supported ? "" : "not ");
	printf("RGB30bit        is %ssupported\n", edid.rgb30bit ? "" : "not ");
	printf("RGB36bit        is %ssupported\n", edid.rgb36bit ? "" : "not ");
	printf("RGB48bit        is %ssupported\n", edid.rgb48bit ? "" : "not ");
	printf("DC_YUV444       is %ssupported\n", edid.dc_y444 ? "" : "not ");
	printf("1920x1080P@60Hz is %ssupported\n", edid.supported_1080p_60Hz ? "" : "not ");
	printf("1920x1080P@50Hz is %ssupported\n", edid.supported_1080p_50Hz ? "" : "not ");
	printf("1920x1080I@60Hz is %ssupported\n", edid.supported_1080i_60Hz ? "" : "not ");
	printf("1920x1080I@50Hz is %ssupported\n", edid.supported_1080i_50Hz ? "" : "not ");
	printf("1280x720P@60Hz  is %ssupported\n", edid.supported_720p_60Hz ? "" : "not ");
	printf("1280x720P@50Hz  is %ssupported\n", edid.supported_720p_50Hz ? "" : "not ");
	printf("640x480P@60Hz   is %ssupported\n", edid.supported_640x480p_60Hz ? "" : "not ");
	printf("720x480P@60Hz   is %ssupported\n", edid.supported_720x480p_60Hz ? "" : "not ");
	printf("720x480I@60Hz   is %ssupported\n", edid.supported_720x480i_60Hz ? "" : "not ");
	printf("720x576P@50Hz   is %ssupported\n", edid.supported_576p_50Hz ? "" : "not ");
	printf("720x576I@50Hz   is %ssupported\n", edid.supported_576i_50Hz ? "" : "not ");
	*u32EdidLength  = sizeof(hdmi_edid_result_t);
	memcpy(u8Edid,  (mt_u8 *)&edid, sizeof(hdmi_edid_result_t));
	#endif
	#endif
	return ret;
}

mt_s32 MT_MPI_HDMI_ReadRawEDID(mt_u8 *u8RawEdid)
{
	mt_s32 ret = 0;
	HDMI_RAW_EDID_INFO_S RawEdidInfo;

	HDMI_CHECK_NULL_PTR(u8RawEdid);

	RawEdidInfo.enHdmi = MT_UNF_HDMI_ID_0;
	memset(&RawEdidInfo, 0, sizeof(HDMI_RAW_EDID_INFO_S));

	ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_RAW_EDID_INFO, &RawEdidInfo);

	if (ret != MT_SUCCESS) {
		MT_ERR_HDMI(" get edid fail!\n");
		return ret;
	}
	memcpy(u8RawEdid, RawEdidInfo.rawEdidInfo, sizeof(RawEdidInfo.rawEdidInfo));
	return ret;
}

mt_s32 MT_MPI_HDMI_RegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc)
{
	#ifdef HDMI_CALLBACK_SUPPORT_LIST
	mt_s32 s32Ret = MT_SUCCESS;
	//HDMI_REGCALLBACKFUNC_S stRegCallbackFunc;
	HDMI_CALLBACK_NODE_S *tmp = NULL;
	//List *pos, *q;
	//unsigned int i;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CHECK_NULL_PTR(pstCallbackFunc);

	MT_INFO_HDMI("enHdmi %d,procID %d----\n", enHdmi, g_HDMIProcID);
	MT_HDMI_LOCK();
	if (NULL != pstCallbackFunc->pfnHdmiEventCallback) {
		tmp = (HDMI_CALLBACK_NODE_S *)mt_malloc(MT_ID_HDMI, sizeof(HDMI_CALLBACK_NODE_S));
		if (NULL == tmp) {
			MT_ERR_HDMI("Callback malloc failed\n");
			MT_HDMI_UNLOCK();
			return MT_FAILURE;
		}

		tmp->stCallbackfunc = *pstCallbackFunc;
	} else {
		MT_ERR_HDMI("Callbackaddr is NULL!\n");
		MT_HDMI_UNLOCK();
		return MT_FAILURE;
	}

	list_add_tail(&(tmp->list), &(g_pstHDMICallBackList.list));
	MT_HDMI_UNLOCK();

	return s32Ret;
	#else
	MT_HDMI_FORCE_PRINT("g_stCallbackfunc 0x%p, pstCallbackFunc 0x%p\n", g_stCallbackfunc, pstCallbackFunc);
	if (g_stCallbackfunc == NULL) {
		if (NULL != pstCallbackFunc->pfnHdmiEventCallback) {
			g_stCallbackfunc = pstCallbackFunc;
			return MT_SUCCESS;
		} else {
			//MT_ERR_HDMI("Callbackaddr is NULL!\n");
			MT_HDMI_FORCE_PRINT("Callbackaddr is NULL!\n");
			return MT_FAILURE;
		}
	} else {
		MT_HDMI_FORCE_PRINT("ERROR !!!g_stCallbackfunc already registered! Only support one!! \n");
		return MT_SUCCESS;
	}
	#endif
}

mt_s32 MT_MPI_HDMI_UnRegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc)
{
	#ifdef HDMI_CALLBACK_SUPPORT_LIST
	HDMI_CALLBACK_NODE_S *tmp;
	List *pos, *q;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CHECK_NULL_PTR(pstCallbackFunc);

	list_for_each_safe(pos, q, &g_pstHDMICallBackList.list) {
		tmp = list_entry(pos, HDMI_CALLBACK_NODE_S, list);
		if (tmp->stCallbackfunc.pfnHdmiEventCallback == pstCallbackFunc->pfnHdmiEventCallback) {
			list_del(pos);
			mt_free(MT_ID_HDMI, tmp);
			tmp = NULL;
			return MT_SUCCESS;
		}
	}

	MT_ERR_HDMI("CallbackFunc:No this Callbackfunc\n");
	return MT_FAILURE;
	#else
	if (g_stCallbackfunc->pfnHdmiEventCallback == pstCallbackFunc->pfnHdmiEventCallback) {
		g_stCallbackfunc = NULL;
		return MT_SUCCESS;
	} else {
		MT_HDMI_FORCE_PRINT("CallbackFunc:No this Callbackfunc\n");
		return MT_FAILURE;
	}
	#endif
}

mt_s32 MT_MPI_HDMI_LoadHDCPKey(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_LOAD_KEY_S *pstLoadKey)
{
	mt_s32 s32Ret = 0;
	HDMI_LOADKEY_S stLoadKey;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CHECK_NULL_PTR(pstLoadKey);

	MT_HDMI_LOCK();
	stLoadKey.enHdmi = enHdmi;
	stLoadKey.stLoadKey = *pstLoadKey;

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_LOADKEY, &stLoadKey);
	if (s32Ret != MT_SUCCESS) {
		MT_ERR_HDMI(" Ioctrl Load Key failed!\n");
		MT_HDMI_UNLOCK();
		return s32Ret;
	}
	MT_HDMI_UNLOCK();
	return s32Ret;
}

mt_s32 MT_MPI_HDMI_GetStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_STATUS_S *pHdmiStatus)
{
	mt_s32 s32Ret = 0;
	HDMI_STATUS_S stHdmiStatus;

	HDMI_CHECK_ID(enHdmi);
	//HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pHdmiStatus);
	#if 1
	MT_HDMI_LOCK();
	memset(&stHdmiStatus, 0, sizeof(HDMI_STATUS_S));
	stHdmiStatus.enHdmi = enHdmi;
	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_STATUS, &stHdmiStatus);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}

	memcpy(pHdmiStatus, &(stHdmiStatus.stStatus), sizeof(MT_UNF_HDMI_STATUS_S));
	MT_HDMI_UNLOCK();
	#else
	stHdmiStatus.stStatus.bAuthed = MT_TRUE;
	stHdmiStatus.stStatus.bConnected = MT_TRUE;
	stHdmiStatus.stStatus.bSinkPowerOn = MT_TRUE;
	memcpy(pHdmiStatus, &(stHdmiStatus.stStatus), sizeof(MT_UNF_HDMI_STATUS_S));
	#endif
	return s32Ret;
}

mt_s32 MT_MPI_HDMI_GetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay)
{
	mt_s32 s32Ret = 0;
	//HDMI_DELAY_S stHdmiDelay;

	HDMI_CHECK_ID(enHdmi);
	//HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstDelay);
	#if 0
	MT_HDMI_LOCK();
	memset(&stHdmiDelay, 0, sizeof(HDMI_DELAY_S));
	stHdmiDelay.enHdmi = enHdmi;

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_GET_DELAY, &stHdmiDelay);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}

	memcpy(pstDelay, &stHdmiDelay.stDelay, sizeof(MT_UNF_HDMI_DELAY_S));

	MT_HDMI_UNLOCK();
	#endif
	return s32Ret;
}

mt_s32 MT_MPI_HDMI_SetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay)
{
	mt_s32 s32Ret = 0;
	//HDMI_DELAY_S stHdmiDelay;
	HDMI_CHECK_ID(enHdmi);
	//HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstDelay);

	if ((pstDelay->u32FmtDelay > MAX_DELAY_TIME_MS) || (pstDelay->u32MuteDelay > MAX_DELAY_TIME_MS)) {
		MT_ERR_HDMI("Delay Time fmt:%d Mute:%d Over Range:%d \n", pstDelay->u32FmtDelay, pstDelay->u32MuteDelay, MAX_DELAY_TIME_MS);
		return MT_ERR_HDMI_INVALID_PARA;
	}

	MT_HDMI_LOCK();
	#if 0
	stHdmiDelay.enHdmi = enHdmi;
	memcpy(&stHdmiDelay.stDelay, pstDelay, sizeof(MT_UNF_HDMI_DELAY_S));

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_DELAY, &stHdmiDelay);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}
	#endif
	MT_HDMI_UNLOCK();
	return s32Ret;
}

mt_s32 MT_MPI_HDMI_Output_Set(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL enable)
{
	mt_s32 Ret = 0;
	HDMI_OUTPUT_S stOutput;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();

	#if 1
	memset(&stOutput, 0, sizeof(HDMI_OUTPUT_S));
	stOutput.enHdmi = enHdmi;
	stOutput.outEnable = enable;

	Ret = ioctl(g_HDMIDevFd, CMD_HDMI_OUTPUT_SET, &stOutput);
	if (Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return Ret;
	}
	#endif

	MT_HDMI_UNLOCK();
	return MT_SUCCESS;
}

mt_s32 MT_MPI_HDMI_Registers_Dump(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret = 0;
	HDMI_REG_DUMP_S stHdmiDump;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();

	memset(&stHdmiDump, 0, sizeof(HDMI_REG_DUMP_S));
	stHdmiDump.enHdmi = enHdmi;

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_REGISTERS_DUMP, &stHdmiDump);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}

	MT_HDMI_UNLOCK();

	return s32Ret;
}


mt_s32 MT_MPI_HDMI_Register_Write(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 data)
{
	mt_s32 s32Ret = 0;
	HDMI_REG_WRITE_S stHdmiWrite;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();

	memset(&stHdmiWrite, 0, sizeof(HDMI_REG_WRITE_S));
	stHdmiWrite.enHdmi = enHdmi;
	stHdmiWrite.addr = addr;
	stHdmiWrite.data = data;

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_REGISTER_WRITE, &stHdmiWrite);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}

	MT_HDMI_UNLOCK();

	return s32Ret;
}


mt_s32 MT_MPI_HDMI_Register_Read(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 *pData)
{
	mt_s32 s32Ret = 0;
	HDMI_REG_READ_S stHdmiRead;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();

	memset(&stHdmiRead, 0, sizeof(HDMI_REG_READ_S));
	stHdmiRead.enHdmi = enHdmi;
	stHdmiRead.addr = addr;
	stHdmiRead.data = pData;

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_REGISTER_READ, &stHdmiRead);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}

	MT_HDMI_UNLOCK();

	return s32Ret;
}

/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pstAttr
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E pstFmt)
{
	mt_s32 s32Ret = 0;
	HDMI_VIDEOTIMING_S stPortFmt;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	//HDMI_CHECK_NULL_PTR(pstFmt);

	MT_HDMI_LOCK();
	memset(&stPortFmt, 0, sizeof(HDMI_VIDEOTIMING_S));
	stPortFmt.enHdmi = enHdmi;
	stPortFmt.VideoTiming = (mt_u32)pstFmt;
	//Hdmi_Unf2DrvAttr(&stPortAttr.stHdmiAppAttr, pstAttr);

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_VIDEO_TIMING, &stPortFmt);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}
	MT_HDMI_UNLOCK();

	#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	/* Warring: Below is just for test!!!! */
	if (pstAttr->bDebugFlag == 0x3fc) {
		//create HPD Event!!
		MT_MPI_UNF_HDMI_GenEvent(MT_UNF_HDMI_EVENT_HOTPLUG);
	}
	#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	return s32Ret;
}

/**
\brief
\attention \n
\param[in] hHdmi
\param[in] pstAttr
\retval none
\see \n
*/
mt_s32 MT_MPI_HDMI_PreSetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E pstFmt)
{
	mt_s32 s32Ret = 0;
	HDMI_PREVIDEOTIMING_S stPortFmt;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	//HDMI_CHECK_NULL_PTR(pstFmt);

	MT_HDMI_LOCK();
	memset(&stPortFmt, 0, sizeof(HDMI_PREVIDEOTIMING_S));
	stPortFmt.enHdmi = enHdmi;
	stPortFmt.VideoTiming = (mt_u32)pstFmt;

	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_PREVTIMING, &stPortFmt);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}
	MT_HDMI_UNLOCK();

	return s32Ret;
}

mt_s32 MT_MPI_HDMI_SetOsdName(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *osdName)
{
	mt_s32 s32Ret = 0;
	HDMI_OSD_NAME_S stHdmiOsd;
	HDMI_CHECK_ID(enHdmi);
	//HDMI_CheckChnOpen(enHdmi);

	MT_HDMI_LOCK();
	if (NULL == osdName) {
		return MT_FAILURE;
	}

	memset(&stHdmiOsd, 0, sizeof(HDMI_OSD_NAME_S));
	stHdmiOsd.enHdmi = enHdmi;

	strncpy((char *)stHdmiOsd.osdName, (char *)osdName, 14);
	s32Ret = ioctl(g_HDMIDevFd, CMD_HDMI_SET_OSDNAME, &stHdmiOsd);
	if (s32Ret != MT_SUCCESS) {
		MT_HDMI_UNLOCK();
		return s32Ret;
	}

	MT_HDMI_UNLOCK();

	return s32Ret;
}

/*------------------------------END--------------------------------*/
