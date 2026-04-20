/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************
  File Name     : mt_mpi_vdec_adapter.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/3
  Description   : Definitions for vdec driver
  History       :
  1.Date        : 2015/12/3
    Author      :
    Modification: Created file

*******************************************************************************/

#ifndef __MT_MPI_VDEC_ADAPTER_H__
#define __MT_MPI_VDEC_ADAPTER_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/******************************* Include Files *******************************/

#include "mt_type.h"
#include "mt_unf_avplay.h"
#include "mt_video_codec.h"
#include "mt_mpi_vdec.h"
#include "mt_drv_vdec.h"

/****************************** Macro Definition *****************************/
//#define VPU_MEM_MAP

/*************************** Structure Definition ****************************/

/* VFMW private attribute */
typedef struct
{
    MT_UNF_AVPLAY_OPEN_OPT_S        stOpenOpt;
}VFMW_PRIV_ATTR_S;

/* VFMW IFrame private parameter */
typedef struct
{
    MT_UNF_AVPLAY_I_FRAME_S*        pstIFrameStream;
    MT_DRV_VIDEO_FRAME_S*      pstVoFrameInfo;
    MT_BOOL                         bCapture;
}VFMW_IFRAME_PARAM_S;

/* VFMW stream buffer */
typedef struct
{
    mt_handle                       hStrmBuf;       /* ES buffer handle, if uses ES buffer, must set this param */
    mt_handle                       hDmxVidChn;     /* Demux handle, if play TS stream from demux, must set this param */
    mt_u32                          u32BufSize;
}VFMW_STREAMBUF_S;

/* VFMW User data */
typedef struct
{
    MT_UNF_VIDEO_USERDATA_TYPE_E*   penType;        /* User data type */
    MT_UNF_VIDEO_USERDATA_S*        pstData;        /* User data info */
}VFMW_USERDATA_S;

/*VFMW SeekPts parameter*/
typedef struct
{
    //mt_u32*                        pu32SeekPts;
    mt_u64*                        pu64SeekPts;
	mt_u32                         u32Gap;
}VFMW_SEEKPTS_PARAM_S;

/* VFMW extended command */
typedef enum
{
    VFMW_CMD_CHECKEVT = 0,      /**< The param is VDEC_EVENT_S* */
    VFMW_CMD_READNEWFRAME,      /**< The param is MT_UNF_VIDEO_FRAME_INFO_S* */
    VFMW_CMD_READUSRDATA,       /**< The param is MT_UNF_VIDEO_USERDATA_S* */
    VFMW_CMD_SETFRAMERATE,      /**< The param is MT_UNF_AVPLAY_FRMRATE_PARAM_S* */
    VFMW_CMD_GETFRAMERATE,      /**< The param is MT_UNF_AVPLAY_FRMRATE_PARAM_S* */
    VFMW_CMD_GETSTATUSINFO,     /**< The param is VDEC_STATUSINFO_S* */
    VFMW_CMD_SETEOSFLAG,        /**< No parameter */
    VFMW_CMD_IFRAMEDECODE,      /**< The param is VFMW_IFRAME_PARAM_S* */
    VFMW_CMD_IFRAMERELEASE,     /**< The param is MT_UNF_VIDEO_FRAME_INFO_S* */
    VFMW_CMD_RECEIVEFRAME,      /**< The param is MT_UNF_VIDEO_FRAME_INFO_S* */
    VFMW_CMD_RELEASEFRAME,      /**< The param is MT_UNF_VIDEO_FRAME_INFO_S* */
    VFMW_CMD_ATTACHBUF,         /**< The param is VFMW_STREAMBUF_S* */
    VFMW_CMD_DETACHBUF,         /**< No parameter */
    VFMW_CMD_DISCARDFRAME,      /**< The param is VDEC_DISCARD_FRAME_S* */
    VFMW_CMD_SETLOWDELAY,      /**< The param is MT_UNF_AVPLAY_LOW_DELAY_ATTR_S* */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    VFMW_CMD_ACQUSERDATA,       /**< The param is VFMW_USERDATA_S* */
    VFMW_CMD_RLSUSERDATA,       /**< The param is MT_UNF_VIDEO_USERDATA_S* */
    VFMW_CMD_RSTUSERDATABUF,    /**< No parameter */
#endif
    VFMW_CMD_DROPSTREAM,
    /* Commond can be invoked by AVPLAY directly */
    VFMW_CMD_GETINFO 			= 0x20,    /* Get vdec info, the param is MT_UNF_AVPLAY_VDEC_INFO_S* */
    VFMW_CMD_SETTPLAYOPT 		= 0x21,/* Set T play, the param is MT_UNF_AVPLAY_TPLAY_OPT_S* */
    VFMW_CMD_SETCTRLINFO 		= 0x22,/* Set control info, the param is MT_UNF_AVPLAY_CONTROL_INFO_S* */
    /**Defines commond to set video sample type, MT_BOOL *, MT_TRUE: Progressive, MT_FALSE: Interlance */
    /**CNcomment: 设置视频逐行信息, MT_TRUE: 逐行, MT_FALSE: 隔行*/
    VFMW_CMD_SET_PROGRESSIVE 	= 0x23,
    VFMW_CMD_SET_COLORSPACE 	=  0x24,

    VFMW_CMD_SET_DPBFULL_CTRL 	= 0x25,
    VFMW_CMD_SET_BUFCLEAR 		= 0x26,
	VFMW_CMD_GET_CAPABILITY 	= 0x27,
	VFMW_CMD_SET_HDRINFO 		= 0x28,	/* Deprecated, please use VFMW_CMD_SET_DISP_HDRINFO instead */
    VFMW_CMD_SET_DISP_HDRINFO   = 0x29,	/* set HDR information, The param is MT_UNF_VIDEO_DISP_HDR_INFO_S* */
	VFMW_CMD_GET_CRCBUF			= 0x30, /* get CRC buffer, The param is MT_UNF_AVPLAY_CRC_BUF_S* */
	VFMW_CMD_SETFRAMERATE_DECLEAR = 0x31,
    VFMW_CMD_BUTT
}VFMW_CMD_E;

//add by l00225186
typedef enum
{
    VPSS_CMD_RECEIVEFRAME = 0,
    VPSS_CMD_CREATEVPSS,
    VPSS_CMD_DESTORYVPSS,
	VPSS_CMD_CREATEPORT,
	VPSS_CMD_DESTORYPORT,
	VPSS_CMD_GETPORTPARAM,
	VPSS_CMD_ENABLEPORT,
	VPSS_CMD_DISABLEPORT,
	VPSS_CMD_RESETVPSS,
	VPSS_CMD_GETSTATUSINFO,
	VPSS_CMD_SETPORTTYPE,
	VPSS_CMD_CANCLEMAINPORT,
	VPSS_CMD_SETCHAN_FRMPACKTYPE,
	VPSS_CMD_GETCHAN_FRMPACKTYPE,
	VPSS_CMD_SENDEOS,
	VPSS_CMD_GETPORTSTATE,
	VPSS_CMD_GETPORTATTR,
	VPSS_CMD_SETPORTATTR,
	VPSS_CMD_SETEXTBUFFER,
	VPSS_CMD_SETBUFFERMODE,
	VPSS_CMD_CHECKANDDELBUFFER,
	VPSS_CMD_SETEXTBUFFERSTATE,
	VPSS_CMD_SETRESOLUTION

}VPSS_CMD_E;

/******************************* API declaration *****************************/

MT_CODEC_ID_E VDEC_UNF2CodecId(MT_UNF_VCODEC_TYPE_E enType);
MT_UNF_VCODEC_TYPE_E VDEC_CodecId2UNF(MT_CODEC_ID_E enCodecId);
MT_UNF_ENC_FMT_E VDEC_DisplayFmt2UNF(MT_CODEC_ENC_FMT_E enDisplayNorm);
MT_CODEC_ENC_FMT_E VDEC_UNFDisplayFmt2CODEC(MT_UNF_ENC_FMT_E enDisplayNorm);

mt_s32 VDEC_OpenDevFile(mt_void);
mt_s32 VDEC_CloseDevFile(mt_void);
MT_CODEC_S* VDEC_VFMW_Codec(mt_void);

mt_s32 VPSS_Control(mt_handle handle, mt_u32 u32CMD, mt_void * pParam);

mt_s32 VDEC_AllocHandle(mt_handle *phHandle);
mt_s32 VDEC_FreeHandle(mt_handle hHandle);

mt_s32 VDEC_CreateStreamBuf(mt_handle hVdec, mt_handle* phBuf,  phys_addr_t *u32PhyAddr, mt_u32 u32BufSize, mt_u32 pip_en);
mt_s32 VDEC_DestroyStreamBuf(mt_handle hBuf);
mt_s32 VDEC_GetStreamBuf(mt_handle hBuf, mt_u32 u32RequestSize, VDEC_ES_BUF_S *pstBuf);
mt_s32 VDEC_PutStreamBuf(mt_handle hBuf, VDEC_ES_BUF_S *pstBuf);
mt_s32 VDEC_ResetStreamBuf(mt_handle hBuf);
mt_s32 VDEC_GetStreamBufStatus(mt_handle hBuf, MT_DRV_VDEC_STREAMBUF_STATUS_S* pstStatus);
mt_s32 VDEC_GetCiTestInfo(mt_handle hInst, MT_UNF_AVPLAY_CI_TEST_INFO_S* pstInfo);
mt_s32 VPSS_ReleaseFrm(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pVideoFrame);
mt_s32 VDEC_SetDPBFullCtrl(mt_handle hInst, MT_BOOL* pParam);
mt_s32 VDEC_SetLowDelay(mt_handle hInst, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S* pParam);
mt_s32 VDEC_SetDecPause(mt_handle hInst, mt_u32* pstPauseFlag);
mt_s32 VDEC_SetColorSpace(mt_handle hInst,MT_UNF_COLOR_SPACE_E*pParam);
mt_s32 VDEC_SetDecFrmType(mt_handle hInst,MT_UNF_DEC_FRM_TYPE_E* pParam);
mt_s32 VDEC_SetTrickCfg(mt_handle hInst, MT_UNF_DEC_TRICK_PARAM_S* pParam);
mt_s32 VDEC_GetCrcBuf(mt_handle hInst, MT_UNF_AVPLAY_CRC_BUF_S* pstBuf);


mt_s32 VFMW_SetProgressive(mt_handle hInst, MT_BOOL* pParam);

mt_s32 VPSS_RecvFrm(mt_handle hVpss, MT_DRV_VIDEO_FRAME_PACKAGE_S* pstFrameInfo);

mt_s32 VPSS_CreateVpss(mt_handle hVdec,mt_handle* phVpss);
mt_s32 VPSS_DestoryVpss(mt_handle hVdec,mt_handle* phVpss);
mt_s32 VPSS_CreatePort(mt_handle hVpss, VDEC_PORT_CFG_S* psVdecPortCfg);
mt_s32 VPSS_DestoryPort(mt_handle hVpss, mt_handle* phPort);
mt_s32 VPSS_EnablePort(mt_handle hVpss, mt_handle* phPort);
mt_s32 VPSS_DisablePort(mt_handle hVpss, mt_handle* phPort);
mt_s32 VPSS_SetPortType(mt_handle hVpss, VDEC_PORT_TYPE_WITHPORT_S* pstPortTypeWithPortHandle);
mt_s32 VPSS_CancleMainPort(mt_handle hVpss, mt_handle* phPort);
mt_s32 VPSS_GetPortParam(mt_handle hVpss,VDEC_PORT_PARAM_WITHPORT_S *pstParam);
mt_s32 VPSS_SetChanFrmPackType(mt_handle hVpss,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E* pParam);
mt_s32 VPSS_GetChanFrmPackType(mt_handle hVpss,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E* pParam);
mt_s32 VPSS_SendEos(mt_handle hVdec);
mt_s32 VPSS_GetPortState(mt_handle hVdec,MT_BOOL* bAllPortComplete);
mt_s32 VPSS_ResetVpss(mt_handle hVpss);
mt_s32 VPSS_GetStatusInfo(mt_handle hVdec,VDEC_FRMSTATUSINFOWITHPORT_S* pstVdecFrmStatusInfo);
mt_s32 VPSS_GetPortAttr(mt_handle hVdec, VDEC_PORT_ATTR_WITHHANDLE_S *pstAttrWithHandle);
mt_s32 VPSS_SetPortAttr(mt_handle hVdec, VDEC_PORT_ATTR_WITHHANDLE_S *pstAttrWithHandle);
mt_s32 VPSS_SetExtBuffer(mt_handle handle,VDEC_BUFFER_ATTR_S *pstBufferAttr);
mt_s32 VPSS_SetBufferMode(mt_handle handle,VDEC_FRAMEBUFFER_MODE_E *penFrameBufferMode);
mt_s32 VPSS_CheckAndDelBuffer(mt_handle handle,VDEC_BUFFER_INFO_S *pstBufInfo);
mt_s32 VPSS_SetExtBufferState(mt_handle handle,VDEC_EXTBUFFER_STATE_E *pEnExtBufferState);
mt_s32 VPSS_SetResolution(mt_handle handle,VDEC_RESOLUTION_ATTR_S* pstResolution);

#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1)
mt_s32 VDEC_RecvStream(mt_handle hBuf, VDEC_ES_BUF_S *pstBuf);
mt_s32 VDEC_RlsStream(mt_handle hBuf, const VDEC_ES_BUF_S *pstBuf);
mt_s32 VDEC_CreateFrameBuf(mt_handle *phBuf);
mt_s32 VDEC_DestroyFrameBuf(mt_handle hBuf);
mt_s32 VDEC_GetFrameBuf(mt_handle hBuf, MT_DRV_VDEC_FRAME_BUF_S* pstBuf);
mt_s32 VDEC_PutFrameBuf(mt_handle hBuf, const MT_DRV_VDEC_USR_FRAME_S* pstBuf);
mt_s32 VDEC_RecvFrameBuf(mt_handle hBuf, MT_DRV_VDEC_USR_FRAME_S* pstBuf);
mt_s32 VDEC_RlsFrameBuf(mt_handle hBuf, const MT_DRV_VDEC_USR_FRAME_S* pstBuf);
mt_s32 VDEC_ResetFrameBuf(mt_handle hBuf);
mt_s32 VDEC_GetFrameBufStatus(mt_handle hBuf, MT_DRV_VDEC_FRAMEBUF_STATUS_S* pstStatus);
mt_s32 VDEC_GetNewFrm(mt_handle hBuf, MT_DRV_VIDEO_FRAME_S* pstFrm);
mt_s32 VDEC_SetFrmRate(mt_handle hBuf, const MT_UNF_AVPLAY_FRMRATE_PARAM_S* pstFrmRate);
mt_s32 VDEC_GetFrmRate(mt_handle hBuf, MT_UNF_AVPLAY_FRMRATE_PARAM_S* pstFrmRate);
#endif
#if (MT_VDEC_VPU_SUPPORT == 1)
mt_s32 VDEC_VPU_CreateFrameBuf(mt_mmz_buf_s *pStreamBuf);
mt_s32 VDEC_VPU_RevertFrameBuf(mt_u32 u32Phyaddr);
mt_s32 VDEC_VPU_CreateFrameList(mt_handle hVdec);
mt_s32 VDEC_VPU_ReleaseFrameList(mt_handle hVdec);
mt_s32 VDEC_VPU_PutFrame(mt_handle hVdec,MT_DRV_VDEC_USR_FRAME_S *pstBuf);
mt_s32 VDEC_VPU_CheckRlsFrameID(mt_handle hVdec,mt_s32 *pID, mt_s32 *ps32Count);
mt_s32 VDEC_VPU_Start(mt_handle hInst);
mt_s32 VDEC_VPU_Stop(mt_handle hInst);
mt_s32 VDEC_VPU_SetAttr(mt_handle hVdec,VDEC_VPU_ATTR_S *pstVPUAttr);

mt_s32 VDEC_VPU_PtsAlloc(mt_handle hInst, mt_handle vpuHandle); // modified by w00278582
mt_s32 VDEC_VPU_PtsFree(mt_handle hInst, mt_handle vpuHandle);  // modified by w00278582
mt_s32 VDEC_VPU_PtsStart(mt_handle hInst);
mt_s32 VDEC_VPU_PtsStop(mt_handle hInst);
mt_s32 VDEC_VPU_PtsReset(mt_handle hInst);

mt_s32 VDEC_VPU_GetFrameRateForNewFrm(mt_handle hHandle, mt_u32 *u32FrameRate);
mt_s32 VPU_GetVpssStatusInfo(mt_handle hVdec, MT_BOOL *bAllPortCompleteFrm);
mt_s32 VPU_SetFrmRate(mt_handle hVdec, const MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 VPU_GetFrmRate(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);

// add by w00278582
mt_s32 VDEC_VPU_ProcStatus(mt_handle hHandle, MT_DRV_VDEC_VPU_STATUS_S *stVPUStatus);

#endif

mt_s32 VDEC_SetHdrInfo(mt_handle hInst, MT_UNF_VIDEO_DISP_HDR_INFO_S *pstHdrInfo);

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
#include "ResidualDecoder.h"
#define MAX_FRAME 8

typedef enum
{
    VDEC_LCEVC_STOP = 0,
	VDEC_LCEVC_INIT,
	VDEC_LCEVC_PAUSE,
    VDEC_LCEVC_BUTT
}VDEC_LCEVC_STAT;

typedef struct pts_data_
{
	int displayed;
	int used;
	int64_t pts;
	phys_addr_t yuvDataPhy;
	ulong yuvDataVir;
	int   yuvsize;
	mt_u32 stride;
    mt_u32 width;                /**< Width of the plane in pixels. */
    mt_u32 height;               /**< Height of the plane in pixels. */
	mt_u32 packing;
	int frameIndex;
}pts_data;

typedef struct Lcevc_p_pipe_{
	size_t unpackedSize;
	uint16_t* unpackedData ;
	LCEVC_ResidualPlaneInfo lastResidualPlaneInfo;
	LCEVC_ResidualPlaneInfo residualPlaneInfo;
	LCEVC_ResidualPacking residualPacking;
    void *		 uVirLcevcAddr;
    phys_addr_t  LcevcPhyAddr ;
    ulong 		 LcevckVirAddr ;
    ulong		 Lcevcsize;
	phys_addr_t  LcecvYuv_phy;
	ulong 		 LcecvYuv_vir;
	mt_u32  	 LcecvYuv_size;
	LCEVC_DecoderHandle lcevcDecoder;
	Lcevc_SEIpipe *  pLcevc_SEIpipe;
	pts_data m_pts_data[MAX_FRAME];
	int m_pts_count;
	int m_total_fb_count;
	VDEC_LCEVC_STAT lcevc_stat;
	mt_u32 yuv_tvsysMatch;
	mt_u32 numReorderPics;
	mt_u32 u32LastPts;
}Lcevc_priv_pipe;

#define LCEVC_MAX_YUV_SIZE  0x4000000

mt_s32 VDEC_GetLcevcSEIData(mt_handle hInst, MT_VDEC_LCEVC_DATA_S * plecvc_data);
int LCEVC_SendSEIData(Lcevc_priv_pipe * pipe, int64_t timestamp,const uint8_t* data, size_t size);
mt_s32 pop_pts(Lcevc_priv_pipe * pipe, MT_DRV_VIDEO_FRAME_S * vframe);
void  LCEVC_INIT(Lcevc_priv_pipe  *pLecvc_priv);
int  LCEVC_DEINIT(Lcevc_priv_pipe  *pLecvc_priv);

#endif


/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_MPI_VDEC_ADAPTER_H__ */

