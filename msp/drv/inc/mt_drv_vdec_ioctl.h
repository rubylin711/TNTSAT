/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : mt_drv_vdec_ioctl.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/04
  Description   :
  History       :
  1.Date        : 2015/12/04
    Author      :
    Modification: Created file
******************************************************************************/
#ifndef __MT_DRV_VDEC_IOCTL_H__
#define __MT_DRV_VDEC_IOCTL_H__

#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_mpi_vdec.h"
#include "mt_debug.h"
#include "mt_drv_vdec.h"
#include "mt_drv_mmz.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

typedef MT_DRV_VDEC_STREAM_BUF_S VDEC_CMD_CREATEBUF_S;

typedef struct mtVDEC_CMD_BUF_CRC_S
{
	mt_handle				hHandle;
	MT_DRV_VDEC_CRC_BUF_S	stCrcBuf;
}VDEC_CMD_BUF_CRC_S;

typedef struct mtVDEC_CMD_BUF_S
{
    mt_handle           hHandle;
    VDEC_ES_BUF_S       stBuf;
}VDEC_CMD_BUF_S;

typedef struct mtVDEC_CMD_BUF_USERADDR_S
{
    mt_handle           hHandle;
    ulong              u32UserAddr;
}VDEC_CMD_BUF_USERADDR_S;

typedef struct mtVDEC_CMD_BUF_STATUS_S
{
    mt_handle               hHandle;
    MT_DRV_VDEC_STREAMBUF_STATUS_S stStatus;
}VDEC_CMD_BUF_STATUS_S;

typedef struct mtVDEC_CMD_ALLOC_S
{
    mt_handle                   hHandle;
    MT_UNF_AVPLAY_OPEN_OPT_S    stOpenOpt;
    mt_u32                      u32DFSEnable;//l00273086
}VDEC_CMD_ALLOC_S;

typedef struct mtVDEC_CMD_RESET_S
{
    mt_handle           hHandle;
    MT_DRV_VDEC_RESET_TYPE_E   enType;
    MT_BOOL             resetDQ;    // reset vdec display queue or not; 
}VDEC_CMD_RESET_S;

typedef struct mtVDEC_CMD_ATTR_S
{
    mt_handle               hHandle;
    MT_UNF_VCODEC_ATTR_S    stAttr;
}VDEC_CMD_ATTR_S;

#if 0
typedef struct
{
  mt_u32 u32Width;
  mt_u32 u32Height;
  mt_u32 u32FrameRate;
  mt_u32 u32VdecCapability;  //decoding rate that vdec can surppot
}FW_VDEC_CAPABILITY_INFO_S;
#endif

typedef struct mtVDEC_CMD_CAPABILITY_S
{
    mt_handle               hHandle;
    FW_VDEC_CAPABILITY_INFO_S    stCapability;
}VDEC_CMD_CAPABILITY_S;

typedef struct mtVDEC_CMD_ATTACH_BUF_S
{
    mt_handle               hHandle;
    mt_u32                  u32BufSize;
    mt_handle               hDmxVidChn;
    mt_handle               hStrmBuf;
}VDEC_CMD_ATTACH_BUF_S;

typedef struct mtVDEC_CMD_USERDATA_S
{
    mt_handle               hHandle;
    MT_UNF_VIDEO_USERDATA_S stUserData;
}VDEC_CMD_USERDATA_S;

typedef struct mtVDEC_CMD_USERDATA_ACQMODE_S
{
    mt_handle               hHandle;
    MT_UNF_VIDEO_USERDATA_S stUserData;
    MT_UNF_VIDEO_USERDATA_TYPE_E enType;
}VDEC_CMD_USERDATA_ACQMODE_S;

typedef struct mtVDEC_CMD_STATUS_S
{
    mt_handle               hHandle;
    VDEC_STATUSINFO_S       stStatus;
}VDEC_CMD_STATUS_S;

typedef struct mtVDEC_CMD_CI_TEST_INFO_S
{
	mt_handle						hHandle;
	MT_UNF_AVPLAY_CI_TEST_INFO_S	stInfo;
}VDEC_CMD_CI_TEST_INFO_S;

typedef struct mtVDEC_CMD_STREAM_INFO_S
{
    mt_handle                   hHandle;
    MT_UNF_VCODEC_STREAMINFO_S  stInfo;
}VDEC_CMD_STREAM_INFO_S;

typedef struct mtVDEC_CMD_EVENT_S
{
    mt_handle               hHandle;
    VDEC_EVENT_S            stEvent;
} VDEC_CMD_EVENT_S;

typedef struct mtVDEC_CMD_FRAME_S
{
    mt_handle               hHandle;
    MT_DRV_VIDEO_FRAME_S stFrame;
} VDEC_CMD_FRAME_S;

typedef struct mtVDEC_CMD_BUFCLEAR_S
{
    mt_handle               hHandle;
    MT_BOOL stBuffClearFlag;
} VDEC_CMD_BUFCLEAR_S;

typedef struct mtVDEC_CMD_IFRAME_DEC_S
{
    mt_handle               hHandle;
    MT_UNF_AVPLAY_I_FRAME_S stIFrame;
    MT_DRV_VIDEO_FRAME_S stVoFrameInfo;
    MT_BOOL                 bCapture;
}VDEC_CMD_IFRAME_DEC_S;

typedef struct mtVDEC_CMD_IFRAME_RLS_S
{
    mt_handle               hHandle;
    MT_DRV_VIDEO_FRAME_S stVoFrameInfo;
}VDEC_CMD_IFRAME_RLS_S;

typedef struct mtVDEC_CMD_FRAME_RATE_S
{
    mt_handle               hHandle;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrameRate;
}VDEC_CMD_FRAME_RATE_S;

typedef struct mtVDEC_CMD_GET_FRAME_S
{
    mt_handle               hHandle;
    MT_DRV_VDEC_FRAME_BUF_S        stFrame;
}VDEC_CMD_GET_FRAME_S;

typedef struct mtVDEC_CMD_PUT_FRAME_S
{
    mt_handle               hHandle;
    MT_DRV_VDEC_USR_FRAME_S        stFrame;
}VDEC_CMD_PUT_FRAME_S;

typedef struct mtVDEC_CMD_VO_FRAME_S
{
    mt_handle               hHandle;
    MT_DRV_VIDEO_FRAME_S stFrame;
}VDEC_CMD_VO_FRAME_S;

typedef struct mtVDEC_CMD_DISCARD_FRAME_S
{
    mt_handle               hHandle;
    VDEC_DISCARD_FRAME_S    stDiscardOpt;
}VDEC_CMD_DISCARD_FRAME_S;

typedef struct mtVDEC_CMD_ATTACHHDL_S
{
    mt_handle               hHandle;
    mt_handle               hVdec;
}VDEC_CMD_ATTACHHDL_S;

typedef struct mtVDEC_CMD_USERDATABUF_S
{
    mt_handle           hHandle;
    MT_DRV_VDEC_USERDATABUF_S  stBuf;
}VDEC_CMD_USERDATABUF_S;

typedef struct mtVDEC_CMD_SET_TPLAY_OPT_S
{
    mt_handle               hHandle;
    MT_UNF_AVPLAY_TPLAY_OPT_S stTPlayOpt;
}VDEC_CMD_TRICKMODE_OPT_S;

typedef struct mtVDEC_CMD_SET_CTRL_INFO_S
{
    mt_handle               hHandle;
    MT_UNF_AVPLAY_CONTROL_INFO_S stCtrlInfo;
}VDEC_CMD_SET_CTRL_INFO_S;

typedef struct mtVDEC_CMD_SET_PROGRESSIVE_S
{
    mt_handle  hHandle;
	MT_BOOL    bProgressive;
}VDEC_CMD_SET_PROGRESSIVE_S;

typedef struct mtVDEC_CMD_SET_DPBFULL_CTRL_S
{
    mt_handle  hHandle;
	MT_BOOL    bDPBFullCtrl;
}VDEC_CMD_SET_DPBFULL_CTRL_S;

typedef struct mtVDEC_CMD_SET_COLORSPACE_S
{
    mt_handle  hHandle;
    mt_u32    u32ColorSpace;
}VDEC_CMD_SET_COLORSPACE_S;

typedef struct mtVDEC_CMD_SET_LOWDELAY_S
{
    mt_handle  hHandle;
	MT_BOOL    bLowdelay;
}VDEC_CMD_SET_LOWDELAY_S;

typedef struct mtVDEC_CMD_SET_PAUSE_S
{
  mt_handle  hHandle;
	MT_BOOL    bPauseFlag;
}VDEC_CMD_SET_PAUSE_S;

typedef struct mtVDEC_CMD_SET_DECFRMTYPE_S
{
  mt_handle  hHandle;
  mt_u32    decFrmType;  //0: all frames;1 : only I and P frames; 2: only I frames
}VDEC_CMD_SET_DECFRMTYPE_S;

typedef struct mtVDEC_CMD_SET_TRICKCFG_S
{
  mt_handle  hHandle;
  MT_BOOL is_incomplete_stream;
  MT_UNF_DEC_TRICK_MODE_E trick_mode;
}VDEC_CMD_SET_TRICKCFG_S;

//add by l00225186
typedef struct tagVDEC_VPSS_PARAM_S
{
    mt_handle* phVpss;
}VDEC_VPSS_PARAM_S;


typedef struct mtVDEC_CMD_VPSS_FRAME_S
{
    mt_handle                       hHandle;
    VDEC_VPSS_PARAM_S               stVpssParam;
	mt_handle                       hPort;
	VDEC_PORT_TYPE_E                enPortType;
    //MT_DRV_VIDEO_FRAME_PACKAGE_S    stFrame;
    MT_DRV_VIDEO_FRAME_PACKAGE_S    *pstFrame;
	VDEC_PORT_PARAM_S               stPortParam;
	VDEC_FRMSTATUSINFO_S            stVdecFrmStatusInfo;
	MT_UNF_VIDEO_FRAME_PACKING_TYPE_E   eFramePackType;
	VDEC_PORT_ABILITY_E             ePortAbility;
	MT_BOOL                         bAllPortComplete;
	MT_DRV_VPSS_PORT_CFG_S          stPortCfg;
    MT_DRV_VIDEO_FRAME_S            *pVideoFrame;
	VDEC_BUFFER_ATTR_S              stBufferAttr;
}VDEC_CMD_VPSS_FRAME_S;

typedef struct mtVDEC_CMD_SEEKPTS_S
{
    mt_handle                       hHandle;
	//mt_u32*                         pu32SeekPts;
	mt_u64*                         pu64SeekPts;
	mt_u32                          u32Gap;
}VDEC_CMD_SEEK_PTS_S;

typedef struct mtVDEC_CMD_VPU_BUF_CREATE_S
{
	ulong u32StartVirAddr;
    phys_addr_t u32StartPhyAddr;
    mt_u32 u32Size;
}VDEC_CMD_VPU_BUF_CREATE_S;
typedef struct mtVDEC_CMD_VPU_GET_FRAME_S
{
    mt_handle                   hHandle;
    MT_DRV_VDEC_FRAME_BUF_S     stFrame;
}VDEC_CMD_VPU_GET_FRAME_S;
typedef struct mtVDEC_CMD_VPU_PUT_FRAME_S
{
    mt_handle                      hHandle;
    MT_DRV_VDEC_USR_FRAME_S        stFrame;
}VDEC_CMD_VPU_PUT_FRAME_S;
typedef struct mtVDEC_CMD_CREATE_FRAME_LIST_S
{
    mt_handle                      hHandle;
}VDEC_CMD_CREATE_FRAME_LIST_S;
typedef struct mtVDEC_CMD_VPU_GET_VPSS_STATUSINFO_S
{
	mt_handle					   hHandle;
	MT_BOOL                        bAllPortCompleteFrm;
}VDEC_CMD_VPU_GET_VPSS_STATUSINFO_S;
typedef struct mtVDEC_CMD_RELEASE_FRAME_LIST_S
{
    mt_handle                      hHandle;
}VDEC_CMD_RELEASE_FRAME_LIST_S;
typedef struct mtVDEC_CMD_VPU_ATTR_S
{
    mt_handle                      hHandle;
    VDEC_VPU_ATTR_S                stVPUAttr;
}VDEC_CMD_VPU_ATTR_S;
typedef struct mtVDEC_CMD_VPU_CHECK_RLSFRAME_S
{
    mt_handle                      hHandle;
    mt_s32                         as32FrameID[MT_VDEC_MAX_VPU_FRAME_NUM];
	mt_s32                         s32Count;
}VDEC_CMD_VPU_CHECK_RLSFRAME_S;
typedef struct mtVDEC_CMD_SET_BUFFERMODE_S
{
    mt_handle                       hHandle;
	VDEC_FRAMEBUFFER_MODE_E         enFrameBufferMode;
}VDEC_CMD_SET_BUFFERMODE_S;
typedef struct mtVDEC_CMD_CHECKANDDELBUFFER_S
{
    mt_handle                       hHandle;
	VDEC_BUFFER_INFO_S              stBufInfo;
}VDEC_CMD_CHECKANDDELBUFFER_S;
typedef struct mtVDEC_CMD_SETEXTBUFFERSTATE_S
{
    mt_handle                       hHandle;
	VDEC_EXTBUFFER_STATE_E          enExtBufferState;
}VDEC_CMD_SETEXTBUFFERTATE_S;

typedef struct mtVDEC_CMD_SETRESOLUTION_S
{
    mt_handle                       hHandle;
	VDEC_RESOLUTION_ATTR_S          stResolution;
}VDEC_CMD_SETRESOLUTION_S;
typedef struct mtVDEC_CMD_VPU_PROC_HANDLE_S
{
    mt_handle                      hVdecHandle;
    mt_handle                      hVpuHandle;
}VDEC_CMD_VPU_PROC_HANDLE_S;

typedef struct mtVDEC_CMD_VPU_PROC_STATUS_S
{
    mt_handle                      hVdecHandle;
    MT_DRV_VDEC_VPU_STATUS_S       stVPUStatus;
}VDEC_CMD_VPU_PROC_STATUS_S;

typedef struct mtVDEC_CMD_SET_HDRINFO_S
{
    mt_handle                      hHandle;
	MT_UNF_VIDEO_DISP_HDR_INFO_S   stHdrInfo;
}VDEC_CMD_SET_HDRINFO_S;

typedef struct mtVDEC_CMD_GET_LEVCDATA_S
{
    mt_handle                      hHandle;
	MT_VDEC_LCEVC_DATA_S           lcevcData;
}VDEC_CMD_GET_LEVCDATA_S;

typedef struct mtVDEC_CMD_GET_LEVCMEM_S
{
    mt_handle                      hHandle;
	mmz_buffer_s          		   mmz_buf;
}VDEC_CMD_GET_LEVCMMZ_S;

/* 0x00 - 0x1F: For vdec global */
/* 0x20 - 0x3F: For stream buffer */
/* 0x40 - 0x5F: Reserve for frame buffer */
/* 0x60 - 0x7F: For vfmw codec instance basic operation */
/* 0x80 - 0x9F: For vfmw codec instance special operation */
/* 0xA0 - 0xFF: Reserve for use */
#define UMAPC_VDEC_GETCAP               _IOR (MT_ID_VDEC, 0x00, VDEC_CAP_S)
#define UMAPC_VDEC_ALLOCHANDLE          _IOR (MT_ID_VDEC, 0x01, ulong)
#define UMAPC_VDEC_FREEHANDLE           _IOW (MT_ID_VDEC, 0x02, ulong)

#define UMAPC_VDEC_CREATE_ESBUF         _IOWR(MT_ID_VDEC, 0x20, VDEC_CMD_CREATEBUF_S)
#define UMAPC_VDEC_DESTROY_ESBUF        _IOW (MT_ID_VDEC, 0x21, ulong)
#define UMAPC_VDEC_GETBUF 	            _IOWR(MT_ID_VDEC, 0x22, VDEC_CMD_BUF_S)
#define UMAPC_VDEC_PUTBUF 	            _IOWR (MT_ID_VDEC, 0x23, VDEC_CMD_BUF_S)
#define UMAPC_VDEC_SETUSERADDR          _IOW (MT_ID_VDEC, 0x24, VDEC_CMD_BUF_USERADDR_S)
#define UMAPC_VDEC_RCVBUF               _IOWR(MT_ID_VDEC, 0x25, VDEC_CMD_BUF_S)
#define UMAPC_VDEC_RLSBUF               _IOWR(MT_ID_VDEC, 0x26, VDEC_CMD_BUF_S)
#define UMAPC_VDEC_RESET_ESBUF          _IOWR(MT_ID_VDEC, 0x27, ulong)
#define UMAPC_VDEC_GET_ESBUF_STATUS     _IOWR(MT_ID_VDEC, 0x28, VDEC_CMD_BUF_STATUS_S)
#define UMAPC_VDEC_GET_CI_TEST_INFO     _IOWR(MT_ID_VDEC, 0x29, VDEC_CMD_CI_TEST_INFO_S)
#define UMAPC_VDEC_GET_LCEVC_DATA      _IOWR(MT_ID_VDEC, 0x2a, VDEC_CMD_GET_LEVCDATA_S)

#define UMAPC_VDEC_CHAN_ALLOC  	        _IOWR(MT_ID_VDEC, 0x60, VDEC_CMD_ALLOC_S)
#define UMAPC_VDEC_CHAN_FREE  	        _IOW (MT_ID_VDEC, 0x61, ulong)
#define UMAPC_VDEC_CHAN_START           _IOW (MT_ID_VDEC, 0x62, ulong)
#define UMAPC_VDEC_CHAN_STOP            _IOW (MT_ID_VDEC, 0x63, ulong)
#define UMAPC_VDEC_CHAN_RESET           _IOW (MT_ID_VDEC, 0x64, VDEC_CMD_RESET_S)
#define UMAPC_VDEC_CHAN_SETATTR         _IOW (MT_ID_VDEC, 0x65, VDEC_CMD_ATTR_S)
#define UMAPC_VDEC_CHAN_GETATTR         _IOWR(MT_ID_VDEC, 0x66, VDEC_CMD_ATTR_S)
#define UMAPC_VDEC_CHAN_ATTACHBUF       _IOW (MT_ID_VDEC, 0x67, VDEC_CMD_ATTACH_BUF_S)
#define UMAPC_VDEC_CHAN_DETACHBUF       _IOW (MT_ID_VDEC, 0x68, ulong)
#define UMAPC_VDEC_CHAN_SETEOSFLAG      _IOWR(MT_ID_VDEC, 0x69, ulong)
#define UMAPC_VDEC_CHAN_DISCARDFRM      _IOWR(MT_ID_VDEC, 0x6a, VDEC_CMD_DISCARD_FRAME_S)
#define UMAPC_VDEC_CHAN_PAUSE           _IOW (MT_ID_VDEC, 0x6b, VDEC_CMD_SET_PAUSE_S)
#define UMAPC_VDEC_CHAN_SETDECFRMTYPE   _IOW (MT_ID_VDEC, 0x6c, VDEC_CMD_SET_DECFRMTYPE_S)
#define UMAPC_VDEC_CHAN_SETTRICKCFG     _IOW (MT_ID_VDEC, 0x6d, VDEC_CMD_SET_TRICKCFG_S)


#define UMAPC_VDEC_CHAN_USRDATA 	    _IOWR(MT_ID_VDEC, 0x80, VDEC_CMD_USERDATA_S)
#define UMAPC_VDEC_CHAN_STATUSINFO      _IOWR(MT_ID_VDEC, 0x81, VDEC_CMD_STATUS_S)
#define UMAPC_VDEC_CHAN_STREAMINFO      _IOWR(MT_ID_VDEC, 0x82, VDEC_CMD_STREAM_INFO_S)
#define UMAPC_VDEC_CHAN_CHECKEVT        _IOWR(MT_ID_VDEC, 0x83, VDEC_CMD_EVENT_S)
#define UMAPC_VDEC_CHAN_EVNET_NEWFRAME  _IOWR(MT_ID_VDEC, 0x84, VDEC_CMD_FRAME_S)
#define UMAPC_VDEC_CHAN_IFRMDECODE      _IOWR(MT_ID_VDEC, 0x85, VDEC_CMD_IFRAME_DEC_S)
#define UMAPC_VDEC_CHAN_IFRMRELEASE     _IOW (MT_ID_VDEC, 0x8c, VDEC_CMD_IFRAME_RLS_S)

#define UMAPC_VDEC_CHAN_SETFRMRATE      _IOW (MT_ID_VDEC, 0x86, VDEC_CMD_FRAME_RATE_S)
#define UMAPC_VDEC_CHAN_GETFRMRATE      _IOWR(MT_ID_VDEC, 0x87, VDEC_CMD_FRAME_RATE_S)
#define UMAPC_VDEC_CHAN_GETFRM          _IOWR(MT_ID_VDEC, 0x88, VDEC_CMD_GET_FRAME_S)
#define UMAPC_VDEC_CHAN_PUTFRM          _IOWR(MT_ID_VDEC, 0x89, VDEC_CMD_PUT_FRAME_S)
#define UMAPC_VDEC_CHAN_RLSFRM 	        _IOW (MT_ID_VDEC, 0x8a, VDEC_CMD_VO_FRAME_S)
#define UMAPC_VDEC_CHAN_RCVFRM 	        _IOWR(MT_ID_VDEC, 0x8b, VDEC_CMD_VO_FRAME_S)
/* 0x8c is UMAPC_VDEC_CHAN_IFRMRELEASE */
#define UMAPC_VDEC_CHAN_SETTRICKMODE    _IOW (MT_ID_VDEC, 0x8d, VDEC_CMD_TRICKMODE_OPT_S)

/* For CC acquire user data mode */
#define UMAPC_VDEC_CHAN_USERDATAINITBUF _IOWR(MT_ID_VDEC, 0x8e, VDEC_CMD_USERDATABUF_S)
#define UMAPC_VDEC_CHAN_USERDATASETBUFADDR  _IOW (MT_ID_VDEC, 0x8f, VDEC_CMD_BUF_USERADDR_S)
#define UMAPC_VDEC_CHAN_ACQUSERDATA     _IOWR(MT_ID_VDEC, 0x90, VDEC_CMD_USERDATA_ACQMODE_S)
#define UMAPC_VDEC_CHAN_RLSUSERDATA     _IOW (MT_ID_VDEC, 0x91, VDEC_CMD_USERDATA_S)

#define UMAPC_VDEC_CHAN_SETCTRLINFO     _IOW (MT_ID_VDEC, 0x92, VDEC_CMD_SET_CTRL_INFO_S)
#define UMAPC_VDEC_CHAN_PROGRSSIVE      _IOW (MT_ID_VDEC, 0x93, VDEC_CMD_SET_PROGRESSIVE_S)
#define UMAPC_VDEC_CHAN_DPBFULL			_IOWR (MT_ID_VDEC, 0x94, VDEC_CMD_SET_DPBFULL_CTRL_S)
#define UMAPC_VDEC_CHAN_SETCOLORSPACE	_IOWR (MT_ID_VDEC, 0x95, VDEC_CMD_SET_COLORSPACE_S)

/* For CRC */
#define UMAPC_VDEC_CHAN_GETCRCBUF		_IOWR (MT_ID_VDEC, 0x96, VDEC_CMD_BUF_CRC_S)

#define UMAPC_VDEC_CHAN_RSTUSERDATABUF  _IOW (MT_ID_VDEC, 0x97, VDEC_CMD_USERDATA_S)

#define UMAPC_VDEC_CHAN_SETFRMRATE_DECLEAR      _IOW (MT_ID_VDEC, 0x98, VDEC_CMD_FRAME_RATE_S)


//add by l00225186
#define UMAPC_VDEC_ALLOCCHANENTITY      _IOWR(MT_ID_VDEC, 0xa0, ulong)
#define UMAPC_VDEC_CHAN_RCVVPSSFRM      _IOWR(MT_ID_VDEC, 0xa1, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_CREATEPORT      _IOWR(MT_ID_VDEC, 0xa2, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_DESTROYPORT     _IOWR(MT_ID_VDEC, 0xa3, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_GETPORTPARAM    _IOWR(MT_ID_VDEC, 0xa4, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_CREATEVPSS      _IOWR(MT_ID_VDEC, 0xa5, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_ENABLEPORT      _IOWR(MT_ID_VDEC, 0xa6, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_DISABLEPORT     _IOWR(MT_ID_VDEC, 0xa7, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_RESETVPSS       _IOWR(MT_ID_VDEC, 0xa8, ulong)
#define UMAPC_VDEC_CHAN_GETFRMSTATUSINFO  _IOWR(MT_ID_VDEC, 0xa9, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_SETPORTTYPE      _IOWR(MT_ID_VDEC, 0xaa, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_CANCLEMAINPORT     _IOWR(MT_ID_VDEC, 0xab, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_SETFRMPACKTYPE    _IOWR(MT_ID_VDEC, 0xac, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_GETFRMPACKTYPE    _IOWR(MT_ID_VDEC, 0xad, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_SENDEOS            _IOWR(MT_ID_VDEC, 0xae, ulong)
#define UMAPC_VDEC_CHAN_GETPORTSTATE      _IOWR(MT_ID_VDEC, 0xaf, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_DESTORYVPSS      _IOWR(MT_ID_VDEC, 0xb0, VDEC_CMD_VPSS_FRAME_S)

#define UMAPC_VDEC_CHAN_GETPORTATTR      _IOWR(MT_ID_VDEC, 0xb1, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_SETPORTATTR      _IOWR(MT_ID_VDEC, 0xb2, VDEC_CMD_VPSS_FRAME_S)

#define UMAPC_VDEC_CHAN_RLSPORTFRM      _IOWR(MT_ID_VDEC, 0xb3, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_LOWDELAY      _IOW (MT_ID_VDEC, 0xb4, VDEC_CMD_SET_LOWDELAY_S)
#define UMAPC_VDEC_CHAN_SEEKPTS       _IOWR (MT_ID_VDEC, 0xb5, VDEC_CMD_SEEK_PTS_S)
#define UMAPC_VDEC_CHAN_CREATE_VPU_BUF   _IOWR (MT_ID_VDEC, 0xb6, VDEC_CMD_VPU_BUF_CREATE_S)
#define UMAPC_VDEC_CHAN_VPU_REVERT_FRAME_BUF   _IOWR(MT_ID_VDEC, 0xb7, ulong)
#define UMAPC_VDEC_CHAN_VPU_CREATE_FRAMELIST   _IOWR(MT_ID_VDEC, 0xb8, VDEC_CMD_CREATE_FRAME_LIST_S)
#define UMAPC_VDEC_CHAN_VPU_RELEASE_FRAMELIST   _IOWR(MT_ID_VDEC, 0xb9, VDEC_CMD_RELEASE_FRAME_LIST_S)
#define UMAPC_VDEC_CHAN_VPU_PUT_FRAME   _IOWR(MT_ID_VDEC, 0xba, VDEC_CMD_VPU_PUT_FRAME_S)
#define UMAPC_VDEC_CHAN_VPU_ATTR   _IOWR(MT_ID_VDEC, 0xbb, VDEC_CMD_VPU_ATTR_S)
#define UMAPC_VDEC_CHAN_VPU_CHECK_RLSFRM   _IOWR(MT_ID_VDEC, 0xbc, VDEC_CMD_VPU_CHECK_RLSFRAME_S)

#define UMAPC_VDEC_CHAN_VPU_START           _IOW (MT_ID_VDEC, 0xbd, ulong)
#define UMAPC_VDEC_CHAN_VPU_STOP            _IOW (MT_ID_VDEC, 0xbe, ulong)
#define UMAPC_VDEC_VPU_PTS_Alloc           _IOW (MT_ID_VDEC, 0xbf, VDEC_CMD_VPU_PROC_HANDLE_S) // modified by w00278582
#define UMAPC_VDEC_VPU_PTS_Free            _IOW (MT_ID_VDEC, 0xc0, VDEC_CMD_VPU_PROC_HANDLE_S) // modified by w00278582
#define UMAPC_VDEC_VPU_PTS_Start           _IOW (MT_ID_VDEC, 0xc1, ulong)
#define UMAPC_VDEC_VPU_PTS_Stop            _IOW (MT_ID_VDEC, 0xc2, ulong)
#define UMAPC_VDEC_VPU_PTS_Reset           _IOW (MT_ID_VDEC, 0xc3, ulong)
#define UMAPC_VDEC_VPU_GET_FRAME_RATE      _IOW (MT_ID_VDEC, 0xc4, VDEC_CMD_VO_FRAME_S)
#define UMAPC_VDEC_VPU_GET_VPSS_STATUSINFO    _IOWR (MT_ID_VDEC, 0xc5, VDEC_CMD_VPU_GET_VPSS_STATUSINFO_S)
// add by w00278582
#define UMAPC_VDEC_VPU_PROC                 _IOWR(MT_ID_VDEC, 0xc6, VDEC_CMD_VPU_PROC_STATUS_S)

#define UMAPC_VDEC_CHAN_SETEXTBUFFER      _IOWR(MT_ID_VDEC, 0xd0, VDEC_CMD_VPSS_FRAME_S)
#define UMAPC_VDEC_CHAN_SETBUFFERMODE     _IOWR(MT_ID_VDEC, 0xd1, VDEC_CMD_SET_BUFFERMODE_S)
#define UMAPC_VDEC_CHAN_CHECKANDDELBUFFER     _IOWR(MT_ID_VDEC, 0xd2, VDEC_CMD_CHECKANDDELBUFFER_S)
#define UMAPC_VDEC_CHAN_SETEXTBUFFERSTATE     _IOWR(MT_ID_VDEC, 0xd3, VDEC_CMD_SETEXTBUFFERTATE_S)
#define UMAPC_VDEC_CHAN_SETRESOLUTION     _IOWR(MT_ID_VDEC, 0xd4, VDEC_CMD_SETRESOLUTION_S)

#define UMAPC_VDEC_CHAN_SETVOBUFCLEARFLAE    _IOW(MT_ID_VDEC, 0xd5, VDEC_CMD_BUFCLEAR_S)

#define UMAPC_VDEC_GET_DECODING_CAPABILITY   _IOR(MT_ID_VDEC, 0xd6, VDEC_CMD_CAPABILITY_S)

#define UMAPC_VDEC_CHAN_SETHDRINFO    _IOW(MT_ID_VDEC, 0xd7, VDEC_CMD_SET_HDRINFO_S)

#define UMAPC_VDEC_GETFB_MEM               _IOR (MT_ID_VDEC, 0xE0, mmz_buffer_s)
#define UMAPC_VDEC_GETLCEVC_MEM               _IOR (MT_ID_VDEC, 0xE1, VDEC_CMD_GET_LEVCMMZ_S)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __DRV_VDEC_IOCTL_H__ */
