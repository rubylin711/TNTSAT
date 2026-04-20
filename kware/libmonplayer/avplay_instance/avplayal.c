/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : avplay_al.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/06/17
 * Description    : Montage AVPlay AL(Application Layer).
 * History        :
 * 1.Date         : 2017/06/17
 *   Author       : 100613
 *   Modification : Created file
 *
 *****************************************************************************/
/*
 * 20170617:
 */
#include <stdio.h>
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
#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>


#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_vo.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_adp_hdmi.h"
#include "avplayal.h"

#undef NDEBUG

#ifdef ANDROID
#include <utils/Log.h>
#else
#define ALOGD printf
#define ALOGE printf
#define ALOGI printf
#endif

#undef LOG_TAG
#define LOG_TAG "AVPLAY_AL"

#define AVPLAYAL_DEBUG 			ALOGD
#define AVPLAYAL_INFO 			ALOGI
#define AVPLAYAL_ERROR 			ALOGE

#define INVALID_HANDLE			0

#define CHECK_ARG(expr, retval)		do {	\
										if ((expr)) {	\
											AVPLAYAL_ERROR("%s @%d: Error, Invalid arguments!\n",__FUNCTION__,__LINE__);	\
											return retval;	\
										}	\
									} while(0)

//----------------------------------------------------------------------------//
static int g_system_init_once = 0;
static int g_avplayal_opened            = 0;
static MT_HANDLE g_avplayal_handle      = INVALID_HANDLE;
pthread_t g_checktaskThd = INVALID_HANDLE;


/*
 * since audio/video decoder already mutex protect avplay-al api,
 * no need mutex internal avplay-al.
 */
static pthread_mutex_t g_avplayal_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK()				    pthread_mutex_lock(&g_avplayal_mutex)
#define MUTEX_UNLOCK()					pthread_mutex_unlock(&g_avplayal_mutex)

//lock outside
//#define MUTEX_LOCK()
//#define MUTEX_UNLOCK()

typedef enum
{
  HDMI_PLUGIN,
  HDMI_PLUGOUT,
}displaysetting_evt_e;

typedef struct
{
    MT_UNF_HDMI_ID_E enHdmi;
    //void *pHandler;
} HDMI_ARGS_T;
HDMI_ARGS_T g_stHdmiArgs;
static unsigned int g_HDCPFailCount = 0;
//static MT_BOOL g_HDCPFlag = MT_FALSE;
static MT_UNF_HDMI_DEFAULT_ACTION_E g_enDefaultMode = MT_UNF_HDMI_DEFAULT_ACTION_HDMI;


static void displaysetting_HDMI_HotPlug_Proc(void *pPrivateData)
{
    mt_s32 ret = MT_SUCCESS;
    HDMI_ARGS_T *pArgs = (HDMI_ARGS_T *)pPrivateData;
    MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;
    MT_UNF_HDMI_ATTR_S stHdmiAttr;
    //MT_UNF_HDMI_INFOFRAME_S        stInfoFrame;
    MT_UNF_EDID_BASE_INFO_S stSinkCap;
    MT_UNF_HDMI_STATUS_S stHdmiStatus;
#ifdef MT_HDCP_SUPPORT
    static mt_u8 u8FirstTimeSetting = MT_TRUE;
#endif

    AVPLAYAL_ERROR("--- Get HDMI event: HOTPLUG. ---");

    MT_UNF_HDMI_GetStatus(hHdmi, &stHdmiStatus);
    if (MT_FALSE == stHdmiStatus.bConnected) {
	AVPLAYAL_ERROR("No Connect\n");
	return;
    }

    MT_UNF_HDMI_GetAttr(hHdmi, &stHdmiAttr);
    ret = MT_UNF_HDMI_GetSinkCapability(hHdmi, &stSinkCap);

    if (ret == MT_SUCCESS) {
	//stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
	if (MT_TRUE == stSinkCap.bSupportHdmi) {
	    stHdmiAttr.bEnableHdmi = MT_TRUE;
	} else {
	    stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	    //read real edid ok && sink not support hdmi,then we run in dvi mode
	    stHdmiAttr.bEnableHdmi = MT_FALSE;
	}
    } else {
	//when get capability fail,use default mode
	if (g_enDefaultMode != MT_UNF_HDMI_DEFAULT_ACTION_DVI)
	    stHdmiAttr.bEnableHdmi = MT_TRUE;
	else
	    stHdmiAttr.bEnableHdmi = MT_FALSE;
    }

    if (MT_TRUE == stHdmiAttr.bEnableHdmi) {
	stHdmiAttr.bEnableAudio = MT_TRUE;
	stHdmiAttr.bEnableVideo = MT_TRUE;
	stHdmiAttr.bEnableAudInfoFrame = MT_TRUE;
	stHdmiAttr.bEnableAviInfoFrame = MT_TRUE;
    stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_BUTT;// MT_UNF_HDMI_VIDEO_MODE_BUTT will auto output
    } else {
	stHdmiAttr.bEnableAudio = MT_FALSE;
	stHdmiAttr.bEnableVideo = MT_TRUE;
	stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
	stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
	stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
    }

#ifdef MT_HDCP_SUPPORT
    if (u8FirstTimeSetting == MT_TRUE) {
	u8FirstTimeSetting = MT_FALSE;
	if (g_HDCPFlag == MT_TRUE) {
	    stHdmiAttr.bHDCPEnable = MT_TRUE; //Enable HDCP
	} else {
	    stHdmiAttr.bHDCPEnable = MT_FALSE;
	}
    } else {
	//HDCP Enable use default setting!!
    }
#endif

    ret = MT_UNF_HDMI_SetAttr(hHdmi, &stHdmiAttr);

    /* MT_UNF_HDMI_SetAttr must before MT_UNF_HDMI_Start! */
    ret = MT_UNF_HDMI_Start(hHdmi);

    //HDMI_PrintAttr(&stHdmiAttr);

    //displaysetting_EventHandler(HDMI_PLUGIN, 0, 0, 0, 0); zhouxiang remove, dont process now.

    return;
}

static void displaysetting_HDMI_UnPlug_Proc(void *pPrivateData)
{
    HDMI_ARGS_T *pArgs = (HDMI_ARGS_T *)pPrivateData;
    MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;

    AVPLAYAL_ERROR("--- Get HDMI event: UnPlug. ---");
    MT_UNF_HDMI_Stop(hHdmi);


    //displaysetting_EventHandler(HDMI_PLUGOUT, 0, 0, 0, 0);  zhouxiang remove, dont process now.

    AVPLAYAL_ERROR("--- Get HDMI event: UnPlug. 2---");

    return;
}

static void displaysetting_HDMI_HdcpFail_Proc(void *pPrivateData)
{
    //MT_UNF_HDMI_ATTR_S             stHdmiAttr;
    AVPLAYAL_ERROR("--- Get HDMI event: HDCP_FAIL. ---");

    g_HDCPFailCount++;
    if (g_HDCPFailCount >= 50) {
	g_HDCPFailCount = 0;
	AVPLAYAL_ERROR("Warrning:Customer need to deal with HDCP Fail!!!!!!");
    }

#if 0
  MT_UNF_HDMI_GetAttr(0, &stHdmiAttr);

  stHdmiAttr.bHDCPEnable = MT_FALSE;

  MT_UNF_HDMI_SetAttr(0, &stHdmiAttr);
#endif
    return;
}

static void displaysetting_HDMI_HdcpSuccess_Proc(void *pPrivateData)
{
    AVPLAYAL_ERROR("--- Get HDMI event: HDCP_SUCCESS. ---");
    return;
}

static void displaysetting_HDMI_Event_Proc(MT_UNF_HDMI_EVENT_TYPE_E event, void *pPrivateData)
{
    switch (event) {
    case MT_UNF_HDMI_EVENT_HOTPLUG:
	displaysetting_HDMI_HotPlug_Proc(pPrivateData);
	break;
    case MT_UNF_HDMI_EVENT_NO_PLUG:
	displaysetting_HDMI_UnPlug_Proc(pPrivateData);
	break;
    case MT_UNF_HDMI_EVENT_EDID_FAIL:
	break;
    case MT_UNF_HDMI_EVENT_HDCP_FAIL:
	displaysetting_HDMI_HdcpFail_Proc(pPrivateData);
	break;
    case MT_UNF_HDMI_EVENT_HDCP_SUCCESS:
	displaysetting_HDMI_HdcpSuccess_Proc(pPrivateData);
	break;
    case MT_UNF_HDMI_EVENT_RSEN_CONNECT:
	break;
    case MT_UNF_HDMI_EVENT_RSEN_DISCONNECT:
	break;
    default:
	break;
    }

    return;
}

int displaysetting_init_disp(void)
{
    mt_s32 Ret;
    struct fb_var_screeninfo vinfo;
    int console_fd;
//    int osdW = 1080;
//    int osdH = 720;
    MT_UNF_DISP_BG_COLOR_S BgColor;
//    MT_UNF_DISP_INTF_S stIntf[2];
//    MT_UNF_DISP_OFFSET_S offset;
    //MT_UNF_ENC_FMT_E enDefaultFmt;
    MT_UNF_HDMI_DELAY_S stDelay;
    MT_UNF_HDMI_OPEN_PARA_S stOpenParam;
    MT_UNF_HDMI_ID_E enHDMIId;
    MT_UNF_HDMI_CALLBACK_FUNC_S stCallbackFunc;
    HDMI_ARGS_T *p_stHdmiArgs = &g_stHdmiArgs;
    MT_UNF_DISP_ASPECT_RATIO_S stDispAspectRatio = {MT_UNF_DISP_ASPECT_RATIO_AUTO};

    //enDefaultFmt = MT_UNF_ENC_FMT_1080i_50;
    enHDMIId = MT_UNF_HDMI_ID_0;

    //hdmi init.
    Ret = MT_UNF_HDMI_Init();
    if (MT_SUCCESS != Ret) {
	AVPLAYAL_ERROR( "MT_UNF_HDMI_Init failed:%#x\n", Ret);
	return MT_FAILURE;
    }

#ifdef MT_HDCP_SUPPORT
    Ret = MTADP_HDMI_SetHDCPKey(MT_UNF_HDMI_ID_0);
    if (MT_SUCCESS != Ret) {
	AVPLAYAL_ERROR( "Set hdcp erro:%#x\n", Ret);
	//return MT_FAILURE;
    }
#endif

    MT_UNF_HDMI_GetDelay(enHDMIId, &stDelay);
    stDelay.bForceFmtDelay = MT_TRUE;
    stDelay.bForceMuteDelay = MT_TRUE;
    stDelay.u32FmtDelay = 500;
    stDelay.u32MuteDelay = 120;
    MT_UNF_HDMI_SetDelay(enHDMIId, &stDelay);

    //p_stHdmiArgs->pHandler = (void *)g_stHdmiArgs;

    stCallbackFunc.pfnHdmiEventCallback = displaysetting_HDMI_Event_Proc;
    stCallbackFunc.pPrivateData = p_stHdmiArgs;

    Ret = MT_UNF_HDMI_RegCallbackFunc(MT_UNF_HDMI_ID_0, &stCallbackFunc);
    if (Ret != MT_SUCCESS) {
	AVPLAYAL_ERROR( "hdmi reg failed:%#x\n", Ret);
	MT_UNF_HDMI_DeInit();
	return MT_FAILURE;
    }

    stOpenParam.enDefaultMode = g_enDefaultMode;
    Ret = MT_UNF_HDMI_Open(enHDMIId, &stOpenParam);
    if (Ret != MT_SUCCESS) {
	AVPLAYAL_ERROR( "MT_UNF_HDMI_Open failed:%#x\n", Ret);
	MT_UNF_HDMI_DeInit();
	return MT_FAILURE;
    }

    //display init.
    Ret = MT_UNF_DISP_Init();
    if (Ret != MT_SUCCESS) {
	AVPLAYAL_ERROR( "displaysetting_init_disp -1\n");
	return -1;
    }

    Ret = MT_UNF_DISP_Attach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS) {
	AVPLAYAL_ERROR( "displaysetting_init_disp -4\n");
	MT_UNF_DISP_DeInit();
	return -4;
    }
    /* set display1 format*/
    Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, MT_UNF_ENC_FMT_1080i_50);
    if (Ret != MT_SUCCESS) {
	AVPLAYAL_ERROR( "displaysetting_init_disp -5\n");
	MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
	MT_UNF_DISP_DeInit();
	return -5;
    }

    Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_PAL);
    if (MT_SUCCESS != Ret) {
	AVPLAYAL_ERROR( "displaysetting_init_disp -6\n");
	MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
	MT_UNF_DISP_DeInit();
	return -6;
    }

    BgColor.u8Red = 0;
    BgColor.u8Green = 0;
    BgColor.u8Blue = 0;
    Ret = MT_UNF_DISP_SetBgColor(MT_UNF_DISPLAY1, &BgColor);
    if (Ret != MT_SUCCESS) {
    	AVPLAYAL_ERROR( "displaysetting_init_disp -7\n");
    	MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    	MT_UNF_DISP_DeInit();
    	return -7;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS) {
    	AVPLAYAL_ERROR( "displaysetting_init_disp -8\n");
    	MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    	MT_UNF_DISP_DeInit();
    	return -8;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY0);
    if (Ret != MT_SUCCESS) {
    	AVPLAYAL_ERROR("displaysetting_init_disp -9\n");
    	MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
    	MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    	MT_UNF_DISP_DeInit();
    	return -9;
    }

    console_fd = open("/dev/graphics/fb0", O_RDWR, 0);
    if (ioctl(console_fd, FBIOGET_VSCREENINFO, &vinfo) >= 0) {
//	osdW = vinfo.xres;
//	osdH = vinfo.yres;
	AVPLAYAL_ERROR(
	                    "[%s] FrameBuffer size %d * %d\n", __FUNCTION__,
	                    vinfo.xres, vinfo.yres);
    } else {
//	osdW = 1920;
//	osdH = 1080;
	AVPLAYAL_ERROR(
	                    "[%s] FrameBuffer size Error\n", __FUNCTION__);
    }
    if (console_fd > 0) {
	close(console_fd);
    }

#if 0
    //for android
    if (osdH == 1080) {
	property_set("persist.sys.resolution", "1080");
    } else {
	property_set("persist.sys.resolution", "720");
    }
#endif

    MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY0, &stDispAspectRatio);
    stDispAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_AUTO;
    MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY0, &stDispAspectRatio);

    MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY1, &stDispAspectRatio);
    stDispAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_AUTO;
    MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &stDispAspectRatio);

    AVPLAYAL_INFO("displaysetting_init_disp end end.\n");
    return 0;
}

/*
 * audio & video decoders both use this mutex for
 * they have conflict calling AVPlay APIs.
 */
pthread_mutex_t g_avdec_mutex = PTHREAD_MUTEX_INITIALIZER;

void mon_sync_init(OMX_TRACK_INFO type)
{
	//if(g_pts_info.is_started != 0)//gst audio and video separate,only judged by here
	//	return;
	if(type == OMX_OnlyAudio)
	{
        if(g_pts_info.video_pts == 0)//video is set
            g_pts_info.audio_pts = 0;
        else
        {
    		g_pts_info.video_pts = PTS_S64_MAX;
    		g_pts_info.audio_pts = 0;
        }
	}
	else if(type == OMX_OnlyVideo)
	{
		g_pts_info.video_pts = 0;
		g_pts_info.audio_pts = PTS_S64_MAX;
	}
	else//OMX_BothAudioANDVideo
	{
		g_pts_info.video_pts = 0;
		g_pts_info.audio_pts = 0;
	}
	g_pts_info.is_started = 1;
	g_pts_info.pkt_cnt =0;
	AVPLAYAL_DEBUG("[%s] g_pts_info: audio_pts[%lld] video_pts[%lld] is_started [%d] pkt_cnt[%d]\n", __func__, g_pts_info.audio_pts, g_pts_info.video_pts, g_pts_info.is_started, g_pts_info.pkt_cnt);

}

/**
 * mon_handle_video
 *
 * @param[in] for raw av sync
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
uint32_t mon_handle_video(int64_t v_pts)
{
    MUTEX_LOCK();
	if(v_pts <= g_pts_info.audio_pts || //(freespace >= threhold)
		((v_pts - g_pts_info.audio_pts) < PTS_DIFF_THREHOLD_MIN)
		|| ((v_pts - g_pts_info.audio_pts) > PTS_DIFF_THREHOLD_MAX)
		|| (g_pts_info.audio_pts == 0)	/* pass-through, audio do not go omx codec */
		|| g_pts_info.pkt_cnt < FIRST_NPKTS_NO_SYNC)//tmp
	{
        MUTEX_UNLOCK();
		return 1;
	}

    MUTEX_UNLOCK();
	//AVPLAYAL_DEBUG("v_pts[%lld], a_pts[%lld]\n", v_pts, g_pts_info.audio_pts);
	return 0;

}

void mon_update_vpts(int64_t v_pts)
{
    MUTEX_LOCK();
	g_pts_info.video_pts = v_pts;
	//AVPLAYAL_DEBUG("[%s] g_pts_info: video_pts[%lld]\n", __func__, g_pts_info.video_pts);
	g_pts_info.pkt_cnt ++;
    MUTEX_UNLOCK();

}

u32 mon_handle_audio(int64_t a_pts)
{
    MUTEX_LOCK();
	if (a_pts <= g_pts_info.video_pts
		|| //(freespace >= threhold)
		((a_pts - g_pts_info.video_pts) < PTS_DIFF_THREHOLD_MIN)
		|| ((a_pts - g_pts_info.video_pts) > PTS_DIFF_THREHOLD_MAX)
		|| (g_pts_info.pkt_cnt < FIRST_NPKTS_NO_SYNC)
		|| (g_pts_info.video_pts == 0)	/* for some audio file,
		                                 * extractor set it has both audio and video incorrectly,
		                                 * and never update video packet & pts,
		                                 * so need ignore v_pts which always is zero.
		                                 */
			)//tmp
	{
        MUTEX_UNLOCK();
		return 1;
	}

    MUTEX_UNLOCK();
	//AVPLAYAL_DEBUG("a_pts[%lld], v_pts[%lld]\n", a_pts, g_pts_info.video_pts);
	return 0;
}

void mon_update_apts(int64_t a_pts)
{
    MUTEX_LOCK();
	g_pts_info.audio_pts = a_pts;
	//AVPLAYAL_DEBUG("[%s] g_pts_info: audio_pts[%lld]\n", __func__, g_pts_info.audio_pts);
	g_pts_info.pkt_cnt ++;
    MUTEX_UNLOCK();

}

void mon_update_playing_pts(int64_t cur_pts)
{
    g_pts_info.cur_playing_pts = cur_pts;
}

int64_t mon_get_playing_pts(void)
{
    return g_pts_info.cur_playing_pts;
}

void mon_sync_deinit(OMX_TRACK_INFO type)
{
	if(type == OMX_OnlyAudio)
	{
		if(g_pts_info.video_pts == PTS_S64_MAX)
			g_pts_info.is_started = 0;
		g_pts_info.audio_pts = PTS_S64_MAX;
	}
	else if(type == OMX_OnlyVideo)
	{
		if(g_pts_info.audio_pts == PTS_S64_MAX)
			g_pts_info.is_started = 0;
		g_pts_info.video_pts = PTS_S64_MAX;
	}
	g_pts_info.pkt_cnt =0;
	AVPLAYAL_DEBUG("[%s] g_pts_info: audio_pts[%lld] video_pts[%lld] is_started [%d]\n", __func__, g_pts_info.audio_pts, g_pts_info.video_pts, g_pts_info.is_started);

}

//----------------------------------------------------------------------------//
/**
 * open avplay al
 *
 * @param[out] pHandle avplay al handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 avplayal_open(MT_HANDLE *pHandle)
{
    MT_S32 Ret;
    MT_HANDLE hAvplay;
    MT_UNF_AVPLAY_ATTR_S AvplayAttr;
    MT_UNF_SYNC_ATTR_S AvSyncAttr;

    MUTEX_LOCK();
	AVPLAYAL_INFO("Enter %s, g_avplayal_opened:%d, getpid()=%lu.\n",__FUNCTION__, g_avplayal_opened, getpid());
	CHECK_ARG((pHandle == NULL), MT_FAILURE);


    if(g_system_init_once == 0)
    {
        AVPLAYAL_DEBUG("mt_sys_init\n");
        mt_sys_init();
        //displaysetting_init_disp();  zhouxiang, copy from android startup
        Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_1080i_50);
        g_system_init_once = 1;
    }

	if (g_avplayal_opened == 0)
	{
		AVPLAYAL_DEBUG("AVAL OPEN[+1]: %s\n","MT_UNF_AVPLAY_Init");
		Ret = MT_UNF_AVPLAY_Init();
		if (Ret != MT_SUCCESS) {
			AVPLAYAL_ERROR("call MT_UNF_AVPLAY_Init failed.\n");
			goto Fail;
		}

		AVPLAYAL_DEBUG("AVAL OPEN[+2]: %s\n","MT_UNF_AVPLAY_GetDefaultConfig");
		Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);

		AVPLAYAL_DEBUG("AVAL OPEN[+3]: %s\n","MT_UNF_AVPLAY_Create");
		Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
		if (Ret != MT_SUCCESS) {
			AVPLAYAL_ERROR("call MT_UNF_AVPLAY_Create failed.\n");
			goto AVPLAY_DEINIT;
		}
		AVPLAYAL_INFO("%s: avplay handle=0x%x\n",__FUNCTION__, hAvplay);

		AVPLAYAL_DEBUG("AVAL OPEN[+4]: %s\n","MT_UNF_AVPLAY_GetAttr");
		Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);

		AVPLAYAL_DEBUG("AVAL OPEN[+5]: %s\n","MT_UNF_AVPLAY_SetAttr");
		AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
		AVPLAYAL_INFO("%s: set sync=%d\n",__FUNCTION__, AvSyncAttr.enSyncRef);
		Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
		if (MT_SUCCESS != Ret) {
			AVPLAYAL_ERROR("call MT_UNF_AVPLAY_SetAttr failed.\n");
			goto AVPLAY_DESTROY;
		}

		AVPLAYAL_DEBUG("AVAL OPEN[+6][END]: %s\n","MT_UNF_DISP_Init");
	    Ret = MT_UNF_DISP_Init();

	    if (Ret != MT_SUCCESS)
	    {
	        AVPLAYAL_ERROR("call MT_UNF_DISP_Init failed.\n");
	        goto AVPLAY_DESTROY;
	    }

		g_avplayal_handle = hAvplay;
        mon_sync_deinit(OMX_OnlyAudio);
        mon_sync_deinit(OMX_OnlyVideo);
        g_checktaskThd = INVALID_HANDLE;
	}

    g_avplayal_opened ++;
	*pHandle = g_avplayal_handle;

	AVPLAYAL_INFO("Leave %s: g_avplayal_opened=%d\n",__FUNCTION__,g_avplayal_opened);
	MUTEX_UNLOCK();
	return MT_SUCCESS;

AVPLAY_DESTROY:
	AVPLAYAL_DEBUG("AVAL OPEN[-1]: %s\n","MT_UNF_AVPLAY_Destroy");
	MT_UNF_AVPLAY_Destroy(hAvplay);

AVPLAY_DEINIT:
	AVPLAYAL_DEBUG("AVAL OPEN[-2]: %s\n","MT_UNF_AVPLAY_DeInit");
	MT_UNF_AVPLAY_DeInit();

Fail:
	MUTEX_UNLOCK();

	return MT_FAILURE;
}

/**
 * close avplay al
 *
 * @param[in] handle avplay al handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 avplayal_close(MT_HANDLE handle)
{
	AVPLAYAL_INFO("Enter %s\n",__FUNCTION__);
	CHECK_ARG((handle == INVALID_HANDLE), MT_FAILURE);
	MUTEX_LOCK();

	if (g_avplayal_opened > 0 && handle == g_avplayal_handle)
	{
		g_avplayal_opened --;

		if (g_avplayal_opened == 0)
		{
			AVPLAYAL_DEBUG("AVAL CLOSE[+1]: %s\n","MT_UNF_AVPLAY_Destroy");
		    MT_UNF_AVPLAY_Destroy(handle);

			AVPLAYAL_DEBUG("AVAL CLOSE[+2]: %s\n","MT_UNF_AVPLAY_DeInit");
		    MT_UNF_AVPLAY_DeInit();

			AVPLAYAL_DEBUG("AVAL CLOSE[+3]: %s\n","MT_UNF_DISP_DeInit");
		    MT_UNF_DISP_DeInit();

			AVPLAYAL_INFO("%s: close handle(0x%x) success.\n",__FUNCTION__,handle);

		    g_avplayal_handle = INVALID_HANDLE;
            g_checktaskThd = INVALID_HANDLE;
		}
		AVPLAYAL_INFO("%s: opened=%d\n",__FUNCTION__,g_avplayal_opened);
	}

	MUTEX_UNLOCK();
	AVPLAYAL_INFO("Leave %s\n",__FUNCTION__);
	return MT_SUCCESS;
}

#if 0       // compile warning, ignore it
static mt_s32 on_avplay_eos_event(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, mt_u32 u32Para)
{
	printf("\n%s: handle=%x, event=%d, param=%x\n",__FUNCTION__,
			hAvplay,enEvent,u32Para);

	if (enEvent == MT_UNF_AVPLAY_EVENT_EOS)
	{
		printf("\nSample EOS Reached!!!\n");
	}

	return 0;
}
#endif
int g_check_task_stop = 0;
mt_check_task_t g_in_args;
void check_task_finish_av_instance(void *args)
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_STATUS_INFO_S stStatusInfo;
    mt_set_pthread_name(__FUNCTION__);
    if (g_pts_info.type == OMX_OnlyAudio)
    {

    }
    else if (g_pts_info.type == OMX_BothAudioANDVideo || g_pts_info.type == OMX_OnlyVideo)
    {
        //AVPLAYAL_INFO("call MT_UNF_AVPLAY_GetStatusInfo, g_avplayal_handle 0x%x\n", g_avplayal_handle);
        do {
			if(g_check_task_stop)
                break;
			Ret = MT_UNF_AVPLAY_GetStatusInfo(g_avplayal_handle, &stStatusInfo);
            if (Ret != MT_SUCCESS)
            {
                AVPLAYAL_INFO("call MT_UNF_AVPLAY_GetStatusInfo failed.\n");
                break;
            }
            MT_USLEEP(30000);
		} while (!stStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream);

        AVPLAYAL_INFO("stStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream\n");/**/
        //MT_UNF_AVPLAY_RegisterEvent(g_avplayal_handle, MT_UNF_AVPLAY_EVENT_EOS, on_avplay_eos_event);
    }

    if(!g_check_task_stop)
    {
        if(g_in_args.v_event_cb)
            g_in_args.v_event_cb(g_in_args.vsink, g_in_args.vseqnum, 1);
        if(g_in_args.a_event_cb)
            g_in_args.a_event_cb(g_in_args.asink, g_in_args.aseqnum, 1);
    }
    AVPLAYAL_INFO("event_cb end end\n");
}

MT_S32 avplayer_check_task_finish(int codec_type, void* sink, int seqnum, gst_avplayer_eventcb_fun_t eventcb)
{
    if(codec_type == 1)
    {
        g_in_args.vsink = sink;
        g_in_args.v_event_cb = eventcb;
        g_in_args.vseqnum = seqnum;
    }
    else if(codec_type == 2)
    {
        g_in_args.asink = sink;
        g_in_args.a_event_cb = eventcb;
        g_in_args.aseqnum = seqnum;
    }

    if(g_checktaskThd == NULL)
    {
        g_check_task_stop = 0;
        /*int ret2 = */  pthread_create(&(g_checktaskThd), NULL, (void *)check_task_finish_av_instance, NULL);
    }
	return 0;
}
