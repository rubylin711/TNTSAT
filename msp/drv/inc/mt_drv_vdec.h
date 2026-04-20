/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : mt_drv_vdec.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/04
  Description   :
  History       :
  1.Date        : 2015/12/04
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __MT_DRV_VDEC_H__
#define __MT_DRV_VDEC_H__

#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_mpi_vdec.h"
#include "mt_debug.h"
//add by l00225186
#include "mt_drv_video.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define MT_MALLOC_VDEC(size)    mt_malloc(MT_ID_VDEC, size)
#define MT_FREE_VDEC(addr)      mt_free(MT_ID_VDEC, addr)

#define VDEC_LOCK(Mutex)        (mt_void)pthread_mutex_lock(&Mutex)
#define VDEC_UNLOCK(Mutex)      (mt_void)pthread_mutex_unlock(&Mutex)

#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
#define MT_VDEC_MAX_INSTANCE_NEW (2)
#else
#define MT_VDEC_MAX_INSTANCE_NEW (1)
#endif

/** max user data length*/
#define MAX_USER_DATA_LEN       256UL

#define MT_VDEC_MAX_VPU_FRAME_NUM (31)
typedef enum tagVDEC_RESET_TYPE_E
{
    VDEC_RESET_TYPE_ALL = 0,
    VDEC_RESET_TYPE_IDLE,
    VDEC_RESET_TYPE_INVALID
}MT_DRV_VDEC_RESET_TYPE_E;

typedef struct mtVDEC_CRC_BUF_CRC_S
{
    phys_addr_t              u32PhyAddr;
    ulong              u32Size;
}MT_DRV_VDEC_CRC_BUF_S;


typedef struct mtVDEC_BTL_S
{
    MT_DRV_VIDEO_FRAME_S *pstInFrame;
    MT_DRV_VIDEO_FRAME_S *pstOutFrame;
    phys_addr_t u32PhyAddr;
    mt_u32 u32Size;
    mt_u32 u32TimeOutMs;
}MT_DRV_VDEC_BTL_S;

typedef struct mtVDEC_USR_FRAME_S
{
    MT_BOOL             bFrameValid;
    MT_BOOL             bEndOfStream;
    MT_UNF_VIDEO_FORMAT_E enFormat;           /* Color format */
    mt_u32              u32Pts;
    mt_s32              s32YWidth;
    mt_s32              s32YHeight;
    mt_s32              s32LumaPhyAddr;
    mt_s32              s32LumaStride;
    mt_s32              s32CbPhyAddr;    /* ChromePhyAddr if bSemiPlanar is true;CbPhyAddr if bSemiPlanar is false */
    mt_s32              s32CrPhyAddr;    /* invalid if bSemiPlanar is true; CrPhyAddr if bSemiPlanar is false */
    mt_s32              s32ChromStride;
    mt_s32              s32ChromCrStride;
	mt_s32              s32FrameID;
}MT_DRV_VDEC_USR_FRAME_S;

typedef struct mtVDEC_FRAME_BUF_S
{
    phys_addr_t u32PhyAddr;
    mt_u32 u32Size;
}MT_DRV_VDEC_FRAME_BUF_S;

typedef struct mtVDEC_STREAM_BUF_S
{
    mt_handle           hVdec;    /* vdec handle [in] */
    mt_u32              u32Size;    /* Buffer size[in] */
    mt_handle           hHandle;    /* Stream buffer handle [out] */
    phys_addr_t              u32PhyAddr; /* Buffer phy address [out] */
	mt_u32 			    pip_en;
}MT_DRV_VDEC_STREAM_BUF_S;

/** Stream buffer status */
typedef struct
{
    mt_u32  u32Size;            /**< Total buffer size, in the unit of byte.*/
    mt_u32  u32Available;       /**< Available buffer, in the unit of byte.*/
    mt_u32  u32Used;            /**< Used buffer, in the unit of byte.*/
    mt_u32  u32DataNum;         /**< For stream mode, it is undecoded packet number.
                                     For frame mode, it is undecoded frame number, support BUFMNG_NOT_END_FRAME_BIT.*/
}MT_DRV_VDEC_STREAMBUF_STATUS_S;

/** Frame buffer status */
typedef struct
{
    mt_u32  u32TotalDecFrameNum;
    mt_u32  u32FrameBufNum;     /**< Frame num in buffer to display */
    MT_BOOL bAllPortCompleteFrm;
}MT_DRV_VDEC_FRAMEBUF_STATUS_S;

typedef struct mtVDEC_USERDATABUF_S
{
    mt_u32  u32Size;            /* Buffer size[in] */
    phys_addr_t  u32PhyAddr;         /* Buffer phy address [out] */
}MT_DRV_VDEC_USERDATABUF_S;

typedef enum
{
    VIDDEC_EVT_SPEED_MISMATCH,         /*!< decoder speed does not match requested speed                  */
    VIDDEC_EVT_BUFFER_ALMOST_FULL,     /*!< decoder buffer is almost full (pacing for inject operations)  */
    VIDDEC_EVT_BUFFER_ALMOST_EMPTY,    /*!< decoder buffer is almost empty (pacing for inject operations) */
    VIDDEC_EVT_UNSUPPORTED_STREAM_TYPE,/*!< stream type is unsupported                                    */
    VIDDEC_EVT_1ST_IFRAME_DECODED,     /*!< the first IFRAME has been decoded                             */
    VIDDEC_EVT_STREAM_INFO_CHANGE,     /*!< Change in the stream characteristics                          */
    VIDDEC_EVT_CC_DATA,                /*!< Closed caption data event                                     */
    VIDDEC_EVT_DATA_ERROR,             /*!< stream data error                                             */
    VIDDEC_EVT_ENDDEF                  /* enum terminator */
}VIDDEC_EVNT_TYPE_E;

typedef struct mtVDEC_VPU_FRAME_STATUS_S
{
    mt_u32 u32FrmPhyAddr;
    mt_u32 u32IsPutVdecQueue;
}MT_DRV_VDEC_VPU_FRAME_STATUS_S;


typedef struct mtVDEC_VPU_STATUS_S
{
    mt_u32  u32Version;

    /* vpu decoder status*/
    mt_u32 u32DecodeStatus;
    mt_u32 u32DecodeMode;
    mt_u32 u32InstanceMode;

    /* vpu bitstream buf information*/
    mt_u32 u32PhyAddr;
    mt_u32 u32BsBufSize;
    mt_u32 u32BsBuFUsedSize;
    mt_u32 u32BsBufPercent;
    mt_u32 u32BsBufReadPtr;
    mt_u32 u32BsBufWritePtr;
    mt_u32 u32Profile;
    mt_u32 u32LumaBitdepth;
    mt_u32 u32ChromaBitdepth;

    /* vpu information*/
    mt_u32 u32VedioStandard;
    mt_u32 u32DecWidth;
    mt_u32 u32DecHeight;
    mt_u32 u32DispWidth;
    mt_u32 u32DispHeight;
    mt_u32 u32ErrRatio;
    mt_u32 u32NumOfErrMBs;
    mt_u32 u32SeqChangeCount;
    mt_s32 s32indexFrameDisplay;
    mt_s32 s32indexFrameDecoded;
    mt_u32 u32OldWidth[10];
    mt_u32 u32OldHeight[10];
    mt_u32 u32OldActualFrmBufNum[10];

    /* vpu frame buf information*/
    MT_DRV_VDEC_VPU_FRAME_STATUS_S stFrmStatus[31];
    mt_u32 u32ActualFrmBufNum;
    mt_u32 u32OldFrmBufNum;

} MT_DRV_VDEC_VPU_STATUS_S;

typedef  mt_s32 ( *EventCallBack)(mt_handle vHandle,mt_void *pEvent, VIDDEC_EVNT_TYPE_E eType);
typedef  mt_s32 ( *GetDmxHdlCallBack)(mt_handle dmxID,mt_u32 chanType, mt_handle *pDmxHdl);



mt_s32 MT_DRV_VDEC_Init(mt_void);
mt_void MT_DRV_VDEC_DeInit(mt_void);
mt_s32 MT_DRV_VDEC_Open(mt_void);
mt_s32 MT_DRV_VDEC_Close(mt_void);
mt_s32 MT_DRV_VDEC_AllocChan(mt_handle *phHandle, MT_UNF_AVPLAY_OPEN_OPT_S *pstCapParam);
mt_s32 MT_DRV_VDEC_FreeChan(mt_handle hHandle);
mt_s32 MT_DRV_VDEC_SetChanAttr(mt_handle hHandle, MT_UNF_VCODEC_ATTR_S *pstCfgParam);
mt_s32 MT_DRV_VDEC_GetChanAttr(mt_handle hHandle, MT_UNF_VCODEC_ATTR_S *pstCfgParam);
mt_s32 MT_DRV_VDEC_ChanBufferInit(mt_handle hHandle, mt_u32 u32BufSize, mt_handle hDmxVidChn);
mt_s32 MT_DRV_VDEC_ChanBufferDeInit(mt_handle hHandle);
mt_s32 MT_DRV_VDEC_ResetChan(mt_handle hHandle);
mt_s32 MT_DRV_VDEC_ChanStart(mt_handle hHandle);
mt_s32 MT_DRV_VDEC_ChanStop(mt_handle hHandle);
mt_s32 MT_DRV_VDEC_GetChanStatusInfo(mt_handle hHandle, VDEC_STATUSINFO_S* pstStatus);
mt_s32 MT_DRV_VDEC_GetCiTestInfo(mt_handle hHandle, MT_UNF_AVPLAY_CI_TEST_INFO_S *pstInfo);
mt_s32 MT_DRV_VDEC_GetChanStreamInfo(mt_handle hHandle, MT_UNF_VCODEC_STREAMINFO_S *pstStreamInfo);
mt_s32 MT_DRV_VDEC_CheckNewEvent(mt_handle hHandle, VDEC_EVENT_S *pstEvent);
mt_s32 MT_DRV_VDEC_GetUsrData(mt_handle hHandle, MT_UNF_VIDEO_USERDATA_S *pstUsrData);
mt_s32 MT_DRV_VDEC_SetTrickMode(mt_handle hHandle, MT_UNF_AVPLAY_TPLAY_OPT_S* pstOpt);
mt_s32 MT_DRV_VDEC_SetCtrlInfo(mt_handle hHandle, MT_UNF_AVPLAY_CONTROL_INFO_S* pstCtrlInfo);
mt_s32 MT_DRV_VDEC_DecodeIFrame(mt_handle hHandle, MT_UNF_AVPLAY_I_FRAME_S *pstStreamInfo,
                          MT_DRV_VIDEO_FRAME_S *pstFrameInfo, MT_BOOL bCapture, MT_BOOL bUserSpace);

mt_s32 MT_DRV_VDEC_ReleaseIFrame(mt_handle hHandle, MT_DRV_VIDEO_FRAME_S *pstFrameInfo);

mt_s32 MT_DRV_VDEC_SetEosFlag(mt_handle hHandle);
mt_s32 MT_DRV_VDEC_DiscardFrm(mt_handle hHandle, VDEC_DISCARD_FRAME_S* pstParam);
mt_s32 MT_DRV_VDEC_GetFrmBuf(mt_handle hHandle, MT_DRV_VDEC_FRAME_BUF_S* pstFrm);
mt_s32 MT_DRV_VDEC_PutFrmBuf(mt_handle hHandle, MT_DRV_VDEC_USR_FRAME_S* pstFrm);
mt_s32 MT_DRV_VDEC_CreateStrmBuf(mt_handle hVdec, MT_DRV_VDEC_STREAM_BUF_S *pstBuf);
mt_s32 MT_DRV_VDEC_RecvFrmBuf(mt_handle s32Handle, MT_DRV_VIDEO_FRAME_S * pstFrameInfo);
mt_s32 MT_DRV_VDEC_RlsFrmBuf(mt_handle s32Handle, MT_DRV_VIDEO_FRAME_S * pstFrameInfo);
mt_s32 MT_DRV_VDEC_RlsFrmBufWithoutHandle(MT_DRV_VIDEO_FRAME_S * pstFrameInfo);
mt_s32 MT_DRV_VDEC_BlockToLine(mt_s32 s32Handle, MT_DRV_VDEC_BTL_S * pstBTLInfo);
mt_s32 MT_DRV_VDEC_GetEsBuf(mt_handle s32Handle,  VDEC_ES_BUF_S * pstBuf);
mt_s32 MT_DRV_VDEC_PutEsBuf(mt_handle s32Handle,  VDEC_ES_BUF_S * pstBuf);
mt_s32 MT_DRV_VDEC_SetPortType(mt_handle hVpss,mt_handle hPort, VDEC_PORT_TYPE_E enPortType );
mt_s32 MT_DRV_VDEC_EnablePort(mt_handle hVdec,mt_handle hPort);
mt_s32 MT_DRV_VDEC_CreatePort(mt_handle hVdec,mt_handle* phPort, VDEC_PORT_ABILITY_E ePortAbility);
mt_s32 MT_DRV_VDEC_DestroyPort(mt_handle hVdec,mt_handle hPort);
mt_s32 MT_DRV_VDEC_GetPortParam(mt_handle hVdec,mt_handle hPort,VDEC_PORT_PARAM_S* pstPortParam);
mt_s32 MT_DRV_VDEC_SetChanFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 MT_DRV_VDEC_GetChanFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 MT_DRV_VDEC_Chan_RecvVpssFrmBuf(mt_handle hVdec, MT_DRV_VIDEO_FRAME_PACKAGE_S* pstFrm);
mt_s32 MT_DRV_VDEC_SetProgressive(mt_handle hHandle, MT_BOOL pProgressive);
mt_s32 MT_DRV_VDEC_SetFrmPackingType(mt_handle hVdec,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E eFramePackType);
mt_s32 MT_DRV_VDEC_GetFrmPackingType(mt_handle hVdec,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *penFramePackType);
mt_s32 MT_DRV_VDEC_GetVideoFrameInfo(mt_handle hVdec, MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S *pstVideoFrameInfo);
mt_void MT_DRV_VDEC_GetVcmpFlag(MT_BOOL *pbVcmpFlag);

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
#define LCEVC_RING_SIZE 1024
typedef struct Lcevc_pipe_ {
        char *buffer, *end;                /* begin of buf, end of buf */
		int freesize;
        char *rp;                     /* where to read, where to write */
		mt_s32 push_cnt;
		mt_s32 pop_cnt;
		char *wp;
		char ring[LCEVC_RING_SIZE];
		int64_t m_pop_max_pts;
		int64_t m_last_display_pts;
		mt_s32 m_frame_id;
		mt_u32 numReorderPics;
}Lcevc_SEIpipe;
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MT_DRV_VDEC_H__ */
