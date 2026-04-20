/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : video_decoder.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/06/17
 * Description    : Montage video decoder for OMX IL component.
 * History        :
 * 1.Date         : 2017/06/17
 *   Author       : 100613
 *   Modification : Created file
 *
 *****************************************************************************/
/*
 * 20170617:
 */
#include <pthread.h>
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_vo.h"
#include "libavcodec/avcodec.h"
#include "avplayal.h"

#undef NDEBUG

#ifdef ANDROID
#include <utils/Log.h>
#else
#define ALOGD printf
#define ALOGI printf
#define ALOGE printf
#endif

#undef LOG_TAG
#define LOG_TAG "video_decoder"

#define DEBUG_VDEC				0

#define VDEC_DEBUG 				ALOGD
#define VDEC_INFO 				ALOGI
#define VDEC_ERROR 				ALOGE

#define INVALID_HANDLE			0
#define OMX_BUFFERFLAG_EOS 0x00000001
#define TIME_OUT_MS_ES_PUSH       0//10
#define PUSH_DATA_TIME_COUNT (100)
//it's better to align 8 byte for hw vdec
#define ALIGNMENT   (8)       /* Worst case is requiring alignment to an 8 byte boundary */
#define ALIGN(_s_) ((_s_+(ALIGNMENT-1)) & ~(ALIGNMENT-1))


#define CHECK_ARG(expr, retval)		do {	\
										if ((expr)) {	\
											VDEC_ERROR("%s @%d: Error, Invalid arguments!\n",__FUNCTION__,__LINE__);	\
											return retval;	\
										}	\
									} while(0)

static FLUSH_OUTPUT_E flush_status = FLUSH_NO;
static short g_stop_flag = 0;
extern int g_check_task_stop;

static int g_vdec_opened 	   = 0;


static mt_video_dec_t g_v_dec;

#if 0
static pthread_mutex_t g_vdec_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK()				pthread_mutex_lock(&g_vdec_mutex)
#define MUTEX_UNLOCK()				pthread_mutex_unlock(&g_vdec_mutex)
#else
/*
 * audio & video decoders both use this mutex for
 * they have conflict calling AVPlay APIs.
 */
extern pthread_mutex_t g_avdec_mutex;
#define MUTEX_LOCK()				pthread_mutex_lock(&g_avdec_mutex)
#define MUTEX_UNLOCK()				pthread_mutex_unlock(&g_avdec_mutex)
#endif

static int Get_Vtype(enum AVCodecID video_codec_id)
{
    MT_UNF_VCODEC_TYPE_E VdecType = MT_UNF_VCODEC_TYPE_BUTT;

    VDEC_DEBUG("\n[%s]  vcodec_type:0x%x\n", __FUNCTION__, video_codec_id);

    switch(video_codec_id)
    	{
		case AV_CODEC_ID_MPEG2VIDEO:
			VdecType = MT_UNF_VCODEC_TYPE_MPEG2;
			break;

		case AV_CODEC_ID_H263:
        case AV_CODEC_ID_H263P:
			VdecType = MT_UNF_VCODEC_TYPE_H263;
			break;

		case AV_CODEC_ID_MPEG4:
			VdecType = MT_UNF_VCODEC_TYPE_MPEG4;
			break;

		case AV_CODEC_ID_WMV1:
			VdecType = MT_UNF_VCODEC_TYPE_WMV1;
			break;

		case AV_CODEC_ID_RV10:
			VdecType = MT_UNF_VCODEC_TYPE_RV10;
			break;

		case AV_CODEC_ID_H264:
			VdecType = MT_UNF_VCODEC_TYPE_H264;
			break;

		case AV_CODEC_ID_MJPEG:
			VdecType = MT_UNF_VCODEC_TYPE_MJPEG;
			break;

		case AV_CODEC_ID_VP6:
			VdecType = MT_UNF_VCODEC_TYPE_VP6;
			break;

		case AV_CODEC_ID_VP8:
			VdecType = MT_UNF_VCODEC_TYPE_VP8;
			break;

		case AV_CODEC_ID_HEVC:
			VdecType = MT_UNF_VCODEC_TYPE_HEVC;
			break;

        case AV_CODEC_ID_VC1:
            VdecType = MT_UNF_VCODEC_TYPE_VC1;
			break;

    	default:
			VdecType = MT_UNF_VCODEC_TYPE_BUTT;
			VDEC_ERROR("\n[%s]  unknow type: %d\n", __FUNCTION__, video_codec_id);
			break;
    	}
	VDEC_DEBUG("\n[%s]  VdecType:%d\n", __FUNCTION__, VdecType);
    return VdecType;
}

#define MT_DAC_CVBS 0 //Rock_hu
#define MT_DAC_YPBPR_Y 1
#define MT_DAC_YPBPR_PB 2
#define MT_DAC_YPBPR_PR 3

/* DAC */
#define DAC_CVBS MT_DAC_CVBS
#define DAC_YPBPR_Y MT_DAC_YPBPR_Y
#define DAC_YPBPR_PB MT_DAC_YPBPR_PB
#define DAC_YPBPR_PR MT_DAC_YPBPR_PR

MT_S32 MTADP_Disp_Init_decoder(MT_UNF_ENC_FMT_E enFormat)
{
    MT_S32                      Ret;
    MT_UNF_DISP_BG_COLOR_S      BgColor;
    MT_UNF_DISP_INTF_S          stIntf[2];
    MT_UNF_DISP_OFFSET_S        offset;

    Ret = MT_UNF_DISP_Init();
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_Init failed, Ret=%#x.\n", Ret);
        return Ret;
    }

    /* set display1 interface */
    stIntf[0].enIntfType                = MT_UNF_DISP_INTF_TYPE_YPBPR;
    stIntf[0].unIntf.stYPbPr.u8DacY     = DAC_YPBPR_Y;
    stIntf[0].unIntf.stYPbPr.u8DacPb    = DAC_YPBPR_PB;
    stIntf[0].unIntf.stYPbPr.u8DacPr    = DAC_YPBPR_PR;
    stIntf[1].enIntfType                = MT_UNF_DISP_INTF_TYPE_HDMI;
    stIntf[1].unIntf.enHdmi             = MT_UNF_HDMI_ID_0;
    Ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY1, &stIntf[0], 2);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    /* set display0 interface */
    stIntf[0].enIntfType            = MT_UNF_DISP_INTF_TYPE_CVBS;
    stIntf[0].unIntf.stCVBS.u8Dac   = DAC_CVBS;
    Ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY0, &stIntf[0], 1);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Attach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_Attach failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
    /* set display1 format*/
    Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, enFormat);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
    if ((MT_UNF_ENC_FMT_1080P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_1080i_60 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_30 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_24 == enFormat)
        ||(MT_UNF_ENC_FMT_720P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_480P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_NTSC == enFormat)
        ||(MT_UNF_ENC_FMT_4096X2160_24 == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_30 == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_24 == enFormat))
    {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_NTSC);
        if (MT_SUCCESS != Ret)
        {
            VDEC_INFO("call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret);
            MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
            MT_UNF_DISP_DeInit();
            return Ret;
        }
    }

    if ((MT_UNF_ENC_FMT_1080P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_1080i_50 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_25 == enFormat)
        ||(MT_UNF_ENC_FMT_720P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_576P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_PAL == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_25 == enFormat))
    {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_PAL);
        if (MT_SUCCESS != Ret)
        {
            VDEC_INFO("call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret);
            MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
            MT_UNF_DISP_DeInit();
            return Ret;
        }
    }

#ifndef ANDROID
    Ret = MT_UNF_DISP_SetVirtualScreen(MT_UNF_DISPLAY1, 1280, 720);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_SetVirtualScreen failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    offset.u32Left      = 0;
    offset.u32Top       = 0;
    offset.u32Right     = 0;
    offset.u32Bottom    = 0;
    /*set display1 screen offset*/
    Ret = MT_UNF_DISP_SetScreenOffset(MT_UNF_DISPLAY1, &offset);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    /*set display0 screen offset*/
    Ret = MT_UNF_DISP_SetScreenOffset(MT_UNF_DISPLAY0, &offset);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
#endif
    BgColor.u8Red   = 0;
    BgColor.u8Green = 0;
    BgColor.u8Blue  = 0;
    Ret = MT_UNF_DISP_SetBgColor(MT_UNF_DISPLAY1, &BgColor);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_Open DISPLAY1 failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY0);
    if (Ret != MT_SUCCESS)
    {
        VDEC_INFO("call MT_UNF_DISP_Open DISPLAY0 failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    //Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, enFormat);//Rock_hu
    if (MT_SUCCESS != Ret)
    {
        VDEC_INFO("call MTADP_HDMI_Init failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Close(MT_UNF_DISPLAY0);
        MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

#ifdef ANDROID
    MTADP_SURFACE_ATTR_S    stSurAttr;
    MT_UNF_PDM_DISP_PARAM_S stDispParam;

    MTADP_OSD_Init();

    Ret = MT_UNF_PDM_GetBaseParam(MT_UNF_PDM_BASEPARAM_DISP0, &stDispParam);
    if (MT_SUCCESS != Ret)
    {
	    stSurAttr.u32Width = 1280;
	    stSurAttr.u32Height = 720;
    }
    else
	{
        stSurAttr.u32Width = stDispParam.u32VirtScreenWidth;
        stSurAttr.u32Height = stDispParam.u32VirtScreenHeight;
	}

    stSurAttr.enPixelFormat = MTADP_PF_8888;
    Ret = MTADP_OSD_CreateSurface(&stSurAttr, &g_hSurface);
    if (MT_SUCCESS != Ret)
    {
        MT_UNF_DISP_Close(MT_UNF_DISPLAY0);
        MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        MTADP_OSD_DeInit();
        return Ret;
    }
#endif

    return MT_SUCCESS;
}


MT_S32 MT_AVPlay_SetVdecAttr(MT_HANDLE hAvplay,
					MT_UNF_VCODEC_TYPE_E enType,MT_UNF_VCODEC_MODE_E enMode)
{
    MT_S32 Ret;
    MT_UNF_VCODEC_ATTR_S        VdecAttr;

    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (MT_SUCCESS != Ret)
    {
        VDEC_ERROR("MT_UNF_AVPLAY_GetAttr failed:%#x\n",Ret);
        return Ret;
    }

    VdecAttr.enType = enType;
    VdecAttr.enMode = enMode;
    VdecAttr.u32ErrCover = 100;
    VdecAttr.u32Priority = 3;

    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (Ret != MT_SUCCESS)
    {
        VDEC_ERROR("call MT_UNF_AVPLAY_SetAttr failed.\n");
        return Ret;
    }

    return Ret;
}


MT_S32 video_get_outbuf(OMXVDEC_BUF_DESC *pstBuf)
{
    if (g_vdec_opened == 0)
        return MT_FAILURE;
    MT_S32 Ret = MT_SUCCESS;
#if 0
    //unsigned char *pbuf;
    MT_UNF_VIDEO_FRAME_INFO_S stVFrm = { 0 };
    FILE *pyuvFile;
    MT_S32 frm_len=0;
    //VDEC_INFO("video_get_outbuf: 0x%x, 0x%x pstBuf->buffer_len %d, stride %d height %d\n ",
    //    pstBuf, pstBuf->bufferaddr, pstBuf->buffer_len, pstBuf->out_frame.stride, pstBuf->out_frame.height);
    //memset(pstBuf->bufferaddr,0, pstBuf->buffer_len);
    //pbuf = pstBuf->bufferaddr;
    stVFrm.stLinearFrameAddr[0].u32YAddr = pstBuf->phyaddr;
    stVFrm.stLinearFrameAddr[0].u32CAddr = pstBuf->phyaddr + pstBuf->out_frame.stride*pstBuf->out_frame.height;
    stVFrm.stLinearFrameAddr[0].u32BufSize= pstBuf->buffer_len;

    Ret = MT_UNF_VO_AcquireFrame(g_v_dec.video_vir_win, &stVFrm, 0);
    if (MT_SUCCESS == Ret)
    {
        frm_len=stVFrm.stLinearFrameAddr[0].u32YStride*stVFrm.u32Height;
        pstBuf->data_len = (stVFrm.stLinearFrameAddr[0].u32YStride * stVFrm.u32Height * 3)>>1;
        pstBuf->timestamp = stVFrm.u64Pts;
        pstBuf->out_frame.width = stVFrm.u32Width;
        pstBuf->out_frame.height = stVFrm.u32Height;
        pstBuf->out_frame.stride = stVFrm.stLinearFrameAddr[0].u32YStride;
        pstBuf->out_frame.phyaddr_Y = stVFrm.stLinearFrameAddr[0].u32YAddr;
        pstBuf->out_frame.phyaddr_C = stVFrm.stLinearFrameAddr[0].u32CAddr;
        if(stVFrm.end_of_stream_flag == 1)
        {
            pstBuf->flags |= OMX_BUFFERFLAG_EOS;
        }
        VDEC_INFO("APP Yaddr %x,Caddr %x,ystride %d,cstride %d,w %d,h %d, data_len %d, pstBuf->timestamp[%lld] end_of_stream_flag[%d]", stVFrm.stLinearFrameAddr[0].u32YAddr,stVFrm.stLinearFrameAddr[0].u32CAddr,
            stVFrm.stLinearFrameAddr[0].u32YStride,stVFrm.stLinearFrameAddr[0].u32CStride,stVFrm.u32Width,stVFrm.u32Height, pstBuf->data_len
            ,pstBuf->timestamp, stVFrm.end_of_stream_flag);
        if (Ret == MT_SUCCESS) {
            Ret = MT_UNF_VO_ReleaseFrame(g_v_dec.video_vir_win, &stVFrm);
            if (Ret)
            {
                VDEC_ERROR("releas frm %d \n", Ret);/**/
                return MT_SUCCESS;
            }
        }
    }
#endif
    return Ret;
}

/**
 * open video decoder
 *
 * @param[in] VdecType video decoder codec type
 * @param[out] pHandle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_open(MT_HANDLE *pHandle, 
		MT_UNF_VCODEC_TYPE_E VdecType, MT_BOOL is_tunnel, MT_BOOL has_audio)
{
	MT_S32 Ret;
	MT_HANDLE hWin, hVirWin;
    MT_UNF_AVPLAY_OPEN_OPT_S stMaxCapbility;
	MT_HANDLE hAvplay;
    MT_UNF_WINDOW_ATTR_S WinAttr;
    MT_UNF_SYNC_ATTR_S AvSyncAttr;

	VDEC_INFO("Enter %s\n",__FUNCTION__);

	CHECK_ARG((pHandle == NULL), MT_FAILURE);

	VDEC_INFO("%s: VdecType=%d\n",__FUNCTION__,VdecType);
	CHECK_ARG((VdecType < 0 || VdecType >= MT_UNF_VCODEC_TYPE_BUTT), MT_FAILURE);

	MUTEX_LOCK();
	if (g_vdec_opened != 0)
	{
        VDEC_ERROR("%s: video decoder can support singleton only!\n",__FUNCTION__);
		MUTEX_UNLOCK();
		return MT_SUCCESS;
	}

    g_v_dec.is_tunnel = is_tunnel;
    g_v_dec.avplay_handle = INVALID_HANDLE;
    g_v_dec.video_win = INVALID_HANDLE;
    g_v_dec.video_vir_win = INVALID_HANDLE;

	if (g_vdec_opened == 0)
	{
        //mt_sys_init();
		Ret = avplayal_open(&hAvplay);
        if (Ret != MT_SUCCESS)
		{
			MUTEX_UNLOCK();
			return MT_FAILURE;
		}

        MT_UNF_ENC_FMT_E g_enDefaultFmt = MT_UNF_ENC_FMT_1080i_50;//MT_UNF_ENC_FMT_720P_50;
        Ret = MTADP_Disp_Init_decoder(g_enDefaultFmt);
		//if (is_tunnel && has_audio)
        if (is_tunnel)
		{
			Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);

			AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
			VDEC_DEBUG("%s: set sync=%d\n",__FUNCTION__, AvSyncAttr.enSyncRef);
			Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
			if (MT_SUCCESS != Ret) {
				VDEC_ERROR("call MT_UNF_AVPLAY_SetAttr failed.\n");
				MUTEX_UNLOCK();
				return MT_FAILURE;
			}
		}

		VDEC_DEBUG("V OPEN[+1]: %s\n","MT_UNF_VO_Init");
        Ret = MT_UNF_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if (Ret != MT_SUCCESS)
        {
            VDEC_ERROR("call MT_UNF_VO_Init failed.\n");
			MUTEX_UNLOCK();
			return MT_FAILURE;
        }

	    memset(&WinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
	    WinAttr.enDisp = MT_UNF_DISPLAY1;
	    WinAttr.bVirtual = MT_FALSE;
	    WinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
	    WinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
	    WinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
	    WinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
	    WinAttr.bUseCropRect = MT_FALSE;
	    WinAttr.stInputRect.s32X = 0;
	    WinAttr.stInputRect.s32Y = 0;
	    WinAttr.stInputRect.s32Width = 0;
	    WinAttr.stInputRect.s32Height = 0;

		VDEC_DEBUG("V OPEN[+2]: %s\n","MT_UNF_VO_CreateWindow");
        Ret = MT_UNF_VO_CreateWindow(&WinAttr, &hWin);
        if (Ret != MT_SUCCESS)
        {
            VDEC_ERROR("call MT_UNF_VO_CreateWindow failed.\n");
            goto VO_DEINIT;
        }
		VDEC_INFO("%s: vo win handle=0x%x\n",__FUNCTION__,hWin);

        if(!is_tunnel)
        {
            MT_UNF_WINDOW_ATTR_S stWinAttrVoVirtual;
            /*create virtual window */
            stWinAttrVoVirtual.enDisp = MT_UNF_DISPLAY0;
            stWinAttrVoVirtual.bVirtual = MT_TRUE;
            stWinAttrVoVirtual.enVideoFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_420;
            stWinAttrVoVirtual.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
            stWinAttrVoVirtual.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
            stWinAttrVoVirtual.bUseCropRect = MT_FALSE;
            memset(&stWinAttrVoVirtual.stInputRect, 0, sizeof(mt_rect_s));
            memset(&stWinAttrVoVirtual.stOutputRect, 0, sizeof(mt_rect_s));
            MT_UNF_VO_CreateWindow(&stWinAttrVoVirtual, &hVirWin);
            if (Ret != MT_SUCCESS)
            {
                VDEC_ERROR("call MT_UNF_VO_CreateWindow stWinAttrVoVirtual failed.\n");
                goto VO_DEINIT;
            }
    		VDEC_INFO("%s: vo hVirWin handle=0x%x\n",__FUNCTION__,hVirWin);
        }

        if (MT_UNF_VCODEC_TYPE_MVC == VdecType)
        {
            stMaxCapbility.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
            stMaxCapbility.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
            stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
        }
        else
        {
            stMaxCapbility.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
            stMaxCapbility.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
            stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
        }

		VDEC_DEBUG("V OPEN[+3]: %s\n","MT_UNF_AVPLAY_ChnOpen");
        Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stMaxCapbility);
        if (Ret != MT_SUCCESS)
        {
            VDEC_ERROR("call MT_UNF_AVPLAY_ChnOpen failed.\n");
            goto WIN_DESTROY;
        }


		VDEC_DEBUG("V OPEN[+4]: %s is_tunnel[%d]\n","MT_UNF_VO_AttachWindow ", is_tunnel);
        if(is_tunnel)
            Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
        else
            Ret = MT_UNF_VO_AttachWindow(hVirWin, hAvplay);

        if (Ret != MT_SUCCESS)
        {
            VDEC_ERROR("call MT_UNF_VO_AttachWindow failed.\n");
            goto VCHN_CLOSE;
        }

		VDEC_DEBUG("V OPEN[+5][END]: %s\n","MT_UNF_VO_SetWindowEnable");
        Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
        if (Ret != MT_SUCCESS)
        {
            VDEC_ERROR("call MT_UNF_VO_SetWindowEnable failed.\n");
            goto WIN_DETATCH;
        }

        if(!is_tunnel)
        {
            VDEC_DEBUG("no tunnel MT_UNF_VO_SetWindowEnable hVirWin");
            Ret = MT_UNF_VO_SetWindowEnable(hVirWin, MT_TRUE);
            if (Ret != MT_SUCCESS)
            {
                VDEC_ERROR("call MT_UNF_VO_SetWindowEnable hVirWin failed.\n");
                goto WIN_DETATCH;
            }
        }

		//FIXME: avplay al handle -> video decoder handle
		g_v_dec.avplay_handle = hAvplay;
        g_v_dec.video_win   = hWin;
        g_v_dec.vdecType = VdecType;
        if(!is_tunnel)
            g_v_dec.video_vir_win = hVirWin;
	}

	*pHandle = g_v_dec.avplay_handle;

    g_vdec_opened ++;

	VDEC_INFO("Leave %s: opened=%d, avplay_handle:0x%x\n",__FUNCTION__,g_vdec_opened, g_v_dec.avplay_handle);
	MUTEX_UNLOCK();
	return MT_SUCCESS;

WIN_DETATCH:
	VDEC_DEBUG("V OPEN[-1]: %s\n","MT_UNF_VO_DetachWindow");
    if(is_tunnel)
        MT_UNF_VO_DetachWindow(hWin, hAvplay);
    else
        MT_UNF_VO_DetachWindow(hVirWin, hAvplay);

VCHN_CLOSE:
	VDEC_DEBUG("V OPEN[-2]: %s\n","MT_UNF_AVPLAY_ChnClose");
	MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

WIN_DESTROY:
	VDEC_DEBUG("V OPEN[-3]: %s\n","MT_UNF_VO_DestroyWindow");
	MT_UNF_VO_DestroyWindow(hWin);
    if(!is_tunnel)
        MT_UNF_VO_DestroyWindow(hVirWin);

VO_DEINIT:
	VDEC_DEBUG("V OPEN[-4]: %s\n","MT_UNF_VO_DeInit");
	MT_UNF_VO_DeInit();

	MUTEX_UNLOCK();
	return MT_FAILURE;
}

/**
 * close video decoder
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_close(void *p1)
{
	VDEC_INFO("Enter %s\n",__FUNCTION__);
    mt_video_dec_t *p_handle = (mt_video_dec_t*)p1;
	CHECK_ARG((p_handle->avplay_handle == INVALID_HANDLE), MT_FAILURE);
	MUTEX_LOCK();

	if (g_vdec_opened > 0 && p_handle->avplay_handle== g_v_dec.avplay_handle)
	{
		g_vdec_opened --;

		if (g_vdec_opened == 0)
		{
			VDEC_DEBUG("V CLOSE[+1]: %s\n","MT_UNF_VO_SetWindowEnable");
			MT_UNF_VO_SetWindowEnable(g_v_dec.video_win, MT_FALSE);
            if(!g_v_dec.is_tunnel)
                MT_UNF_VO_SetWindowEnable(g_v_dec.video_vir_win, MT_FALSE);

			VDEC_DEBUG("V CLOSE[+2]: %s\n","MT_UNF_VO_DetachWindow");
            if(g_v_dec.is_tunnel)
			    MT_UNF_VO_DetachWindow(g_v_dec.video_win, p_handle->avplay_handle);
            else
                MT_UNF_VO_DetachWindow(g_v_dec.video_vir_win, p_handle->avplay_handle);

			VDEC_DEBUG("V CLOSE[+3]: %s\n","MT_UNF_AVPLAY_ChnClose");
			MT_UNF_AVPLAY_ChnClose(p_handle->avplay_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

			VDEC_DEBUG("V CLOSE[+4]: %s\n","MT_UNF_VO_DestroyWindow");
			MT_UNF_VO_DestroyWindow(g_v_dec.video_win);
            if(!g_v_dec.is_tunnel)
                MT_UNF_VO_DestroyWindow(g_v_dec.video_vir_win);

			VDEC_DEBUG("V CLOSE[+5]: %s\n","MT_UNF_VO_DeInit");
			MT_UNF_VO_DeInit();

			VDEC_INFO("%s: close handle(0x%x) success.\n",__FUNCTION__,p_handle->avplay_handle);

			//FIXME: video decoder handle -> avplay al handle
			avplayal_close(p_handle->avplay_handle);

			g_v_dec.avplay_handle = INVALID_HANDLE;
			g_v_dec.video_win   = INVALID_HANDLE;
            g_v_dec.video_vir_win = INVALID_HANDLE;
            g_v_dec.vdecType = MT_UNF_VCODEC_TYPE_BUTT;
		}

		VDEC_INFO("%s: opened=%d\n",__FUNCTION__,g_vdec_opened);
	}

	MUTEX_UNLOCK();
	VDEC_INFO("Leave %s\n",__FUNCTION__);
	return MT_SUCCESS;
}


/**
 * stop video decoder
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_stop(void)
{
	VDEC_INFO("%s,%d: %s\n", __FUNCTION__,__LINE__,"MT_UNF_AVPLAY_Stop");
    MT_UNF_AVPLAY_STOP_OPT_S stop;

    g_stop_flag = 1;
    g_check_task_stop = 1;
    stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;//
    stop.u32TimeoutMs = 0;
    int ret = MT_UNF_AVPLAY_Stop(g_v_dec.avplay_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID,&stop);

    mon_sync_deinit(OMX_OnlyVideo);
    VDEC_INFO("%s,%d\n", __func__,__LINE__);
	return ret;
}

/**
 * set video decoder framerate
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_set_framerate(uint32_t frame_rate)
{
   MT_S32 ret = 0;
   if(frame_rate == 0)
      return ret;
   MT_UNF_AVPLAY_FRMRATE_PARAM_S frame_rate_param;
   frame_rate_param.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_USER;
   frame_rate_param.stSetFrmRate.u32fpsInteger = frame_rate;
   frame_rate_param.stSetFrmRate.u32fpsDecimal = 0;

   VDEC_INFO("%s,%d, frame_rate:%d\n", __func__,__LINE__,frame_rate);
   ret = MT_UNF_AVPLAY_SetAttr(g_v_dec.avplay_handle, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, (mt_void *)(&frame_rate_param));

   return ret;
}

/**
 * start video decoder
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_start(MT_HANDLE handle, MT_UNF_VCODEC_TYPE_E VdecType)
{
    int ret = MT_FAILURE;
    MT_BOOL bAdvancedProfil = 1;
    MT_U32  u32CodecVersion = 8;
    g_stop_flag = 0;
    /*set compress attr*/
    g_v_dec.vdecType = VdecType;

    MT_UNF_VCODEC_ATTR_S VcodecAttr;
	VDEC_DEBUG("V OPEN[+6]: %s\n","MT_UNF_AVPLAY_GetAttr");
    ret = MT_UNF_AVPLAY_GetAttr(g_v_dec.avplay_handle, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);

    if (MT_UNF_VCODEC_TYPE_VC1 == VdecType)
    {
        VcodecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = bAdvancedProfil;
        VcodecAttr.unExtAttr.stVC1Attr.u32CodecVersion = u32CodecVersion;
    }

    if (MT_UNF_VCODEC_TYPE_VP6 == VdecType)
    {
        VcodecAttr.unExtAttr.stVP6Attr.bReversed = 0;
    }

    VcodecAttr.enType = VdecType;
    VcodecAttr.u32UseDescInfoFlag = 1;
    //VcodecAttr.u32ErrCover = 100;
    //VcodecAttr.u32Priority = 3;
	VDEC_DEBUG("V OPEN[+7]: %s\n","MT_UNF_AVPLAY_SetAttr ");
    ret |= MT_UNF_AVPLAY_SetAttr(g_v_dec.avplay_handle, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    if (MT_SUCCESS != ret)
    {
        VDEC_ERROR("call MT_UNF_AVPLAY_SetAttr failed.\n");
        return ret;
    }
	VDEC_INFO("%s: %s \n", __FUNCTION__,"MT_AVPlay_SetVdecAttr");
    ret = MT_AVPlay_SetVdecAttr(g_v_dec.avplay_handle, g_v_dec.vdecType, MT_UNF_VCODEC_MODE_NORMAL);

    if (ret != MT_SUCCESS) {
	    VDEC_ERROR("\n call MOHW_AVPlay_SetVdecAttr not success %s %d\n", __FUNCTION__, __LINE__);
        return ret;
    }

   VDEC_INFO("%s: %s, vdecType[%d]\n", __FUNCTION__,"MT_UNF_AVPLAY_Start", g_v_dec.vdecType);
   ret = MT_UNF_AVPLAY_Start(handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID,MT_NULL);
   if (ret != MT_SUCCESS)
   {
   	    VDEC_ERROR("%s,%d MT_UNF_AVPLAY_Start failed\n", __func__,__LINE__);
   }

    MT_UNF_AVPLAY_STATUS_INFO_S status;
    MT_UNF_AVPLAY_GetStatusInfo(g_v_dec.avplay_handle, &status);
    VDEC_INFO("%s,%d status.enRunStatus:%d\n", __func__,__LINE__,status.enRunStatus);

    mon_sync_init(OMX_BothAudioANDVideo);
    g_pts_info.type = OMX_BothAudioANDVideo;
	return ret;
}


/**
 * pause video decoder
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_pause(void)
{
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    MT_UNF_AVPLAY_GetStatusInfo(g_v_dec.avplay_handle,&status);
    int ret = MT_SUCCESS;
    VDEC_INFO("%s,%d status.enRunStatus:%d\n", __func__,__LINE__,status.enRunStatus);
    if(status.enRunStatus != MT_UNF_AVPLAY_STATUS_PAUSE)
    {
	   VDEC_INFO("%s,%d call MT_UNF_AVPLAY_Pause\n", __func__,__LINE__);//FP_DEBUG
       ret =  MT_UNF_AVPLAY_Pause(g_v_dec.avplay_handle, MT_NULL);
    }
	return ret;
}


/**
 * resume video decoder
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_resume(void)
{
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    int ret = MT_SUCCESS;
    MT_UNF_AVPLAY_GetStatusInfo(g_v_dec.avplay_handle,&status);
    VDEC_INFO("%s,%d status.enRunStatus:%d\n", __func__,__LINE__,status.enRunStatus);
    if(status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE)
    {
	   VDEC_INFO("%s,%d call MT_UNF_AVPLAY_Resume\n", __func__,__LINE__);//FP_DEBUG
       ret =  MT_UNF_AVPLAY_Resume(g_v_dec.avplay_handle, MT_NULL);
    }

	return ret;
}

/**
 * video_decoder_flush
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_flush(void)
{
    //flush both input and output port!!
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    MT_S32 s32Ret = -1;
    MT_UNF_AVPLAY_GetStatusInfo(g_v_dec.avplay_handle, &status);
    VDEC_DEBUG("[%s]  status.enRunStatus:%d use flush, avplay_handle[0x%x]\n",
        __FUNCTION__, status.enRunStatus, g_v_dec.avplay_handle);
    if((status.enRunStatus == MT_UNF_AVPLAY_STATUS_PLAY)||(status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE))
    {
       flush_status = FLUSH_START;
       //MT_USLEEP(3000);
       s32Ret = MT_UNF_AVPLAY_Flush(g_v_dec.avplay_handle);
    }
    mon_sync_init(OMX_OnlyVideo);
    return s32Ret;
}

MT_S32 video_decoder_flush_stop(void)
{
    flush_status = FLUSH_END;
    return MT_SUCCESS;
}

MT_S32 video_decoder_set_trick_mode(int playrate)
{
	mt_s32 ret = MT_SUCCESS;
	MT_UNF_AVPLAY_TPLAY_OPT_S pstTplayOpt;
//    MT_UNF_DEC_TRICK_PARAM_S stTrickParam;

    VDEC_DEBUG("%s,%d, playrate %d\n", __func__,__LINE__, playrate);
    if(playrate >= 0)
    {
	    pstTplayOpt.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
//        stTrickParam.trick_mode = MT_UNF_DEC_TM_FFWD;
    }
    else if(playrate < 0)
    {
	    pstTplayOpt.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD;
//        stTrickParam.trick_mode = MT_UNF_DEC_TM_FREV;
    }

    if(playrate== 0 || playrate==1)
    {
//        stTrickParam.is_incomplete_stream = 0;
//        stTrickParam.trick_mode = MT_UNF_DEC_TM_NORMAL;
	    pstTplayOpt.u32SpeedInteger = 1;
    }
    else
    {
        pstTplayOpt.u32SpeedInteger = 2;
//        stTrickParam.is_incomplete_stream = 1;
    }
	pstTplayOpt.u32SpeedDecimal = 0;

    VDEC_DEBUG("%s,%d, enTplayDirect %d, u32SpeedInteger %d\n",
         __func__,__LINE__, pstTplayOpt.enTplayDirect, pstTplayOpt.u32SpeedInteger);
       //pthread_mutex_lock(&VMutex);
	ret =  MT_UNF_AVPLAY_Tplay(g_v_dec.avplay_handle,  &pstTplayOpt);
	//pthread_mutex_unlock(&VMutex);


    //MT_MPI_VDEC_SetTrickCfg(p_file_seq->p_vdec_dev, &stTrickParam);
    return ret;
}




/**
 * video_decoder_check_buffer_empty
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_check_buffer_empty(MT_BOOL *p_empty)
{
	 //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;

	//MT_UNF_AVPLAY_IsBuffEmpty(g_v_dec.avplay_handle, p_empty);//audio and video should eos

     MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;
     MT_S32 ret = MT_SUCCESS;
     ret = MT_UNF_AVPLAY_GetStatusInfo(g_v_dec.avplay_handle, &statusInfo);
     if (ret == MT_SUCCESS )
     {
        if (statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream
            || statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize == 0)//only video
            *p_empty = MT_TRUE;//Video EOS Reached!
     }
    //VDEC_DEBUG("%s: p_empty[%d], u32UsedSize[%d] bEndOfStream[%d]\n", __FUNCTION__, *p_empty,
    //    statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize, statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream);
	return MT_SUCCESS;

}

/**
 * resume video decoder
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 video_decoder_push_es(OMXVDEC_BUF_DESC *puser_buf)
{
    int ret = MT_SUCCESS;
    MT_UNF_STREAM_BUF_S   StreamBuf;
    mt_u8 *p_addr = puser_buf->bufferaddr;
    int video_FrameLen = puser_buf->data_len;
    mt_s64 vpts = puser_buf->timestamp;
    MT_S32 timecount = 0;
    mt_u32 eos_flag = 0;
//    mt_u32 count = 0;


    if (puser_buf->flags & OMX_BUFFERFLAG_EOS)
    {
        eos_flag = 1;
        if (video_FrameLen == 0)
        {
        	video_FrameLen = 256;	//last eos packet, stuff zero, at least 16 bytes.
        }
        else
        {
        	//should not happen
        	VDEC_ERROR("%s: EOS with valid data length(%d)!\n",__FUNCTION__,video_FrameLen);
        }
    }

    /*while(mon_handle_video(vpts) == 0)
	{
        if(vpts == 0)
            break;
		count++;
		if(count >= 3000){	//3s
			VDEC_ERROR("%s: ERROR wait 3s video_pts[%lld] audio_pts[%lld]\n",
                __func__, g_pts_info.video_pts, g_pts_info.audio_pts);
			count = 0;
		}
        if(flush_status == FLUSH_START || g_stop_flag == 1)
            return MT_SUCCESS;
		MT_USLEEP(1000);	//10ms
	}*/

	if (DEBUG_VDEC)
	{
    	VDEC_INFO("timestamp:%lld video_FrameLen:%d, eos_flag:%d\n",
            puser_buf->timestamp,video_FrameLen, eos_flag);
    }
    while (video_FrameLen > 0)
    {
         //VDEC_INFO("%s,%d,size=%d\n", __func__,__LINE__,video_FrameLen);
         if(flush_status == FLUSH_START)
            return MT_SUCCESS;
         if(g_stop_flag == 1)
		   	return MT_SUCCESS;
         ret = MT_UNF_AVPLAY_GetBuf(g_v_dec.avplay_handle, MT_UNF_AVPLAY_BUF_ID_ES_VID, video_FrameLen
                               , &StreamBuf, TIME_OUT_MS_ES_PUSH);
         //VDEC_INFO("%s,%d,ret=0x%x,StreamBuf.u32Size=%d\n", __func__,__LINE__,ret,StreamBuf.u32Size);
         if(MT_SUCCESS != ret || StreamBuf.u32Size<= 0)
         {
              //VDEC_ERROR("[%s] %d  mem buf get failed: reqlen=%d,ret = %d\n",__func__, __LINE__,video_FrameLen,ret );
              MT_USLEEP(1000);
             //if( ++timecount > PUSH_DATA_TIME_COUNT)
             //   return MT_FAILURE;

             // mtos_task_sleep(10);
            if( ++timecount > PUSH_DATA_TIME_COUNT)
            {
	        	VDEC_ERROR("%s: reached PUSH_DATA_TIME_COUNT! usleep(100000)\n",__FUNCTION__);
                //return MT_FAILURE;
                timecount = 0;
                MT_USLEEP(100000);
            }
			continue;
         }

         timecount = 0;
         if(StreamBuf.u32Size >= video_FrameLen)
         {

            memcpy(StreamBuf.pu8Data, p_addr, StreamBuf.u32Size);
            //vpts/1000  gst ns, need us
            ret = MT_UNF_AVPLAY_PutBuf_V1(g_v_dec.avplay_handle, MT_UNF_AVPLAY_BUF_ID_ES_VID, StreamBuf.u32Size, vpts/1000, 1, 1, eos_flag);
            //printf("0000: %x, %x \n",StreamBuf.u32Size, video_FrameLen);
            if(eos_flag)
            {
                VDEC_INFO("timestamp:%lld video_FrameLen:%d, eos_flag:%d\n",
                    puser_buf->timestamp,video_FrameLen, eos_flag);
                MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S stFlushOpt;
                MT_UNF_AVPLAY_FlushStream(g_v_dec.avplay_handle, &stFlushOpt);
            }
         }
         else
         {
            if (eos_flag && puser_buf->data_len == 0)	//last eos packet, all stuff zero
            	memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);
            else
	            memcpy(StreamBuf.pu8Data, p_addr, StreamBuf.u32Size);

            ret = MT_UNF_AVPLAY_PutBuf_V1(g_v_dec.avplay_handle, MT_UNF_AVPLAY_BUF_ID_ES_VID, StreamBuf.u32Size, vpts/1000, 1, 0, 0);
            VDEC_INFO("1111: %x, %x \n",StreamBuf.u32Size, video_FrameLen);
         }
         /*push es data*/

         if(MT_SUCCESS != ret)
         {
           VDEC_ERROR("@@E:video es input failed: ret = %d@%s,line%d\n",ret ,__func__, __LINE__);
         }
         //else
     		//VDEC_INFO("[%s] push ves ok\n",__func__);
       	video_FrameLen -= StreamBuf.u32Size;
       	p_addr += StreamBuf.u32Size;
    }

	if (DEBUG_VDEC)
	{
	    //VDEC_INFO("timestamp:%lld video_FrameLen:%d, eos_flag:%d - done.\n", puser_buf->timestamp,video_FrameLen, eos_flag);
	}

    //mon_update_vpts(vpts);
    return ret;
}


MT_S32 video_decoder_set_avsync_none(void)
{
    MT_S32   ret = MT_SUCCESS;

    //only video track, no audio track
    mon_sync_init(OMX_OnlyVideo);

    MT_UNF_SYNC_ATTR_S AvSyncAttr;
    ret = MT_UNF_AVPLAY_GetAttr(g_v_dec.avplay_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);

	AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
	VDEC_INFO("%s: set sync=%d\n",__FUNCTION__, AvSyncAttr.enSyncRef);
	ret |= MT_UNF_AVPLAY_SetAttr(g_v_dec.avplay_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
	if (MT_SUCCESS != ret) {
		VDEC_ERROR("call MT_UNF_AVPLAY_SetAttr failed.\n");
	}
    g_pts_info.type = OMX_OnlyVideo;

    return ret;
}

MT_S32 video_decoder_init(void *p1)
{
   MT_S32   ret = MT_SUCCESS;
   if(p1 == NULL)
      return MT_FAILURE;
   mt_video_dec_t *p_dec_para = (mt_video_dec_t *)p1;
   g_v_dec.has_audio = p_dec_para->has_audio;
   g_v_dec.is_tunnel = p_dec_para->is_tunnel;
   g_v_dec.frame_rate = p_dec_para->frame_rate;
   g_v_dec.v_codec_id = p_dec_para->v_codec_id;
   //pthread_mutex_t* pVMutex = drv_ctx->pVMutex;
   MT_HANDLE hAvplay= g_v_dec.avplay_handle;
   MT_UNF_VCODEC_TYPE_E format;

   VDEC_INFO("%s,%d \n", __func__,__LINE__);

   format = Get_Vtype((enum AVCodecID)g_v_dec.v_codec_id);
   g_v_dec.vdecType = format;
   if(format == MT_UNF_VCODEC_TYPE_BUTT){
        VDEC_ERROR("\n unknow vdec type!! %s %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
     }

//Modified: 20170617
// all do here!
    ret = video_decoder_open(&hAvplay, format, g_v_dec.is_tunnel, g_v_dec.has_audio);
    if (ret != MT_SUCCESS) {
        return MT_FAILURE;
    }

   //pthread_mutex_lock(pVMutex);
   //ret = video_decoder_set_framerate(g_v_dec.frame_rate);
   //if (ret != MT_SUCCESS)
   //   goto V_START_FAILED;

   ret = video_decoder_start(hAvplay, format);
   if (ret != MT_SUCCESS)
      goto V_START_FAILED;

  //pthread_mutex_unlock(pVMutex);
  p_dec_para->avplay_handle = g_v_dec.avplay_handle;
  VDEC_INFO("%s,%d end end\n", __func__,__LINE__);
  return MT_SUCCESS;

V_START_FAILED:
    video_decoder_close((void*)hAvplay);
    //pthread_mutex_unlock(pVMutex);
    return MT_FAILURE;
}

void video_get_pts(int64_t *out_pts)
{
    MT_S32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;
    ret = MT_UNF_AVPLAY_GetStatusInfo(g_v_dec.avplay_handle, &statusInfo);
    if(ret != MT_SUCCESS)
    {
        VDEC_ERROR("@@E: get video info error @line %d,fun %s",__LINE__,__func__);
        *out_pts = 0;
        return;
    }

    *out_pts = (int64_t)statusInfo.stSyncStatus.u64LastVidPts * 1000;
    mon_update_playing_pts(*out_pts);
    //VDEC_DEBUG("%s: u64LastVidPts[%lld]\n",__func__, (long long)*out_pts);
    return;
}

void* video_decoder_get_handle(void)
{
    //VDEC_INFO("video_decoder_get_handle, avplay_handle:0x%x\n", g_v_dec.avplay_handle);
    return &g_v_dec;
}

MT_S32 video_decoder_check_waterlevel(MT_S32 *audio_percent, MT_S32 *video_percent)
{
    MT_UNF_AVPLAY_STATUS_INFO_S pstStatusInfo;
 	MT_UNF_AVPLAY_BUF_STATUS_S* pBufInfo;
//	int free_space = 0;
    MT_S32 percent = 100;
    MT_S32 percent2 = 100;
    if(flush_status == FLUSH_START || g_stop_flag == 1)
        return percent;
   	MT_S32 ret =  MT_UNF_AVPLAY_GetVideoStatusInfo(g_v_dec.avplay_handle,&pstStatusInfo);
	if (ret != MT_SUCCESS)
     {
        VDEC_ERROR(" MT_UNF_AVPLAY_GetStatusInfo fail\n");
	    //pthread_mutex_unlock(&VMutex);
	    return 0;
     }
  	pBufInfo = &(pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID]);
	//free_space = pVidBufInfo->u32BufSize - pVidBufInfo->u32UsedSize;
	if(pBufInfo->u32BufSize > 0)
	    percent = pBufInfo->u32UsedSize*100/pBufInfo->u32BufSize;

    ret =  MT_UNF_AVPLAY_GetAudioStatusInfo(g_v_dec.avplay_handle,&pstStatusInfo);
	if (ret != MT_SUCCESS)
    {
        VDEC_ERROR(" MT_UNF_AVPLAY_GetStatusInfo fail\n");
	    //pthread_mutex_unlock(&VMutex);
	    return ret;
    }
  	pBufInfo = &(pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD]);
	//free_space = pVidBufInfo->u32BufSize - pVidBufInfo->u32UsedSize;
	if(pBufInfo->u32BufSize > 0)
	    percent2 = pBufInfo->u32UsedSize*100/pBufInfo->u32BufSize;

    *audio_percent = percent2;
    *video_percent = percent;
    //VDEC_DEBUG("audio_percent[%d] video_percent[%d]\n", *audio_percent, *video_percent);
    return 0;
}



MT_S32 video_decoder_get_es_buf_space(uint32_t *video_size, uint32_t *video_total_size, uint32_t *audio_size, uint32_t *audio_total_size)
{
	MT_S32 ret;
	MT_UNF_AVPLAY_STATUS_INFO_S pstStatusInfo;
	MT_UNF_AVPLAY_BUF_STATUS_S* pBufInfo;
	int free_space = 0;

	ret =  MT_UNF_AVPLAY_GetVideoStatusInfo(g_v_dec.avplay_handle,&pstStatusInfo);
	if (ret != MT_SUCCESS)
	{
		VDEC_ERROR(" MT_UNF_AVPLAY_GetStatusInfo fail\n");
		return ret;
	}
	pBufInfo = &(pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID]);
	free_space = pBufInfo->u32BufSize - pBufInfo->u32UsedSize;
	if(free_space > 0)
		*video_size = free_space >>10;//Kbytes
	else{
		*video_size = 0;
		VDEC_ERROR("%s,%d no space size[%d]!\n", __func__,__LINE__,free_space);
	}
	*video_total_size = pBufInfo->u32BufSize >>10;//Kbytes


	ret =  MT_UNF_AVPLAY_GetAudioStatusInfo(g_v_dec.avplay_handle,&pstStatusInfo);
	if (ret != MT_SUCCESS)
	{
		VDEC_ERROR(" MT_UNF_AVPLAY_GetStatusInfo fail\n");
		return ret;
	}
	pBufInfo = &(pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD]);
	free_space = pBufInfo->u32BufSize - pBufInfo->u32UsedSize;
	if(free_space > 0)
		*audio_size = free_space >> 10;//Kbytes
	else{
		*audio_size = 0;
		VDEC_ERROR("%s,%d no space size[%d]!\n", __func__,__LINE__,free_space);
	}
	*audio_total_size = pBufInfo->u32BufSize >> 10;//Kbytes

	return MT_SUCCESS;
}
