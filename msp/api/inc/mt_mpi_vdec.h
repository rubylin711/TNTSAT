/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name             :   mt_mpi_vdec.h
  Version               :   Initial Draft
  Author                :   Montage multimedia software group
  Created               :   2015/11/25
  Last Modified         :
  Description           :
  Function List         :
  History               :
  1.Date                :   2015/11/25
    Author              :
Modification            :   Created file
******************************************************************************/

/******************************* Include Files *******************************/

#ifndef  __MT_MPI_VDEC_H__
#define  __MT_MPI_VDEC_H__

#include "mt_unf_avplay.h"
#include "mt_drv_video.h"
#include "mt_drv_vpss.h"
#include "mt_video_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/****************************** Macro Definition *****************************/

#ifndef MT_VDEC_REG_CODEC_SUPPORT
#define MT_VDEC_REG_CODEC_SUPPORT (0)
#endif

#ifndef MT_VDEC_MJPEG_SUPPORT
#define MT_VDEC_MJPEG_SUPPORT (0)
#endif

#ifndef MT_VDEC_USERDATA_CC_SUPPORT
#define MT_VDEC_USERDATA_CC_SUPPORT (1)
#endif

#ifndef MT_VDEC_DFS_SUPPORT
#define MT_VDEC_DFS_SUPPORT (0)
#endif

#ifndef MT_VDEC_VPU_SUPPORT
#define MT_VDEC_VPU_SUPPORT (0)
#endif

#define MAX_VDEC_EXT_BUF_NUM 16
/*************************** Structure Definition ****************************/

typedef struct hiVDEC_ES_BUF_S
{
    mt_u8*  pu8Addr;      /*Virtual address of the buffer user.*/
    phys_addr_t  u32PhyAddr;   /*Physical address of the buffer */
    mt_u32  u32BufSize;   /*Buffer size, in the unit of byte.*/
    mt_u64  u64Pts;       /*PTS of the data filled in a buffer.*/
    MT_BOOL bEndOfFrame;  /* End of Frame flag */
    MT_BOOL bDiscontinuous;/* Stream continue or not */
    mt_u32  u32PtsValide;
    mt_u32  u32FrameFinsh;
    mt_u32  u32PreFrameFinsh;
    mt_u32  u32ScrapSize;
    mt_u32  u32EosFlag;

	//debug
    mt_u32  u32PutTime;

}VDEC_ES_BUF_S, *PTR_VDEC_ES_BUF_S;

typedef struct hiVDEC_STATUSINFO_S
{
    mt_u32  u32BufferSize;      /* Total buffer size, in the unit of byte.*/
    mt_u32  u32BufferAvailable; /* Available buffer, in the unit of byte.*/
    mt_u32  u32BufferUsed;      /* Used buffer, in the unit of byte.*/
    mt_u32  u32VfmwFrmNum;
    mt_u32  u32VfmwStrmSize;
    mt_u32  u32VfmwStrmNum;     /* The un-decoded stream seg num produced by SCD */
    mt_u32  u32VfmwTotalDispFrmNum; /* total display num ( plus extra_disp ) */
    mt_u32  u32FieldFlag;       /* 0:frame 1:field */
    MT_UNF_VCODEC_FRMRATE_S stVfmwFrameRate;/* vfmw frame rate */
    mt_u32  u32StrmInBps;
    mt_u32  u32TotalDecFrmNum;
    mt_u32  u32TotalErrFrmNum;
    mt_u32  u32TotalErrStrmNum;
    mt_u32  u32FrameBufNum;     /* frame num in buffer to display */
    MT_BOOL bEndOfStream;       /* There's no enough stream in buffer to decode a frame */
    MT_BOOL bAllPortCompleteFrm;

    mt_u32 u32BufRptr;  /*buffer read pointer *//*CNcomment:缓冲区读指针*/
    mt_u32 u32BufWptr;	/*buffer written pointer *//*CNcomment:缓冲区写指针*/
	mt_u32 u32VideoESFrameNumber;
}VDEC_STATUSINFO_S;

#if 1
typedef struct hiVDEC_FRMSTATUSINFO_S
{
    mt_u32  u32DecodedFrmNum;  /* decoded, but not ready to display.*/
    mt_u32  u32StrmSize;       /* the un-decoded stream seg produced by SCD */
    mt_u32  u32StrmInBps;      /*average Bps */
    mt_u32  u32OutBufFrmNum;   /*decoded, and ready to display*/
}VDEC_FRMSTATUSINFO_S;
typedef struct tagVDEC_FRMSTATUSINFOWITHPORT
{
    mt_handle hPort;
    VDEC_FRMSTATUSINFO_S stVdecFrmStatus;
}VDEC_FRMSTATUSINFOWITHPORT_S;
#endif

typedef struct hiVDEC_EVENT_S
{
    MT_BOOL  bNewFrame;
    MT_BOOL  bNewSeq;
    MT_BOOL  bNewUserData;

    MT_BOOL  bFirstValidPts;
    ulong   u32FirstValidPts;
    MT_BOOL  bSecondValidPts;
    ulong   u32SecondValidPts;

    MT_BOOL  bNormChange;
    MT_UNF_NORMCHANGE_PARAM_S stNormChangeParam;

    MT_BOOL  bFramePackingChange;
    MT_UNF_VIDEO_FRAME_PACKING_TYPE_E enFramePackingType;

    MT_BOOL  bIFrameErr;
    MT_BOOL  bUnSupportStream;
	MT_BOOL  bDecErr;
    MT_BOOL  bImageSizeChange;
    mt_u32   u32ErrRatio;
    MT_BOOL  bLastFrameDecoded;
    MT_BOOL  bLastFrameShowed;
    MT_BOOL  bSlHdrEnableChange;
    MT_BOOL  bSlHdrEnable;
} VDEC_EVENT_S;

typedef enum hiVDEC_DISCARD_MODE_E
{
    VDEC_DISCARD_STOP,      /* Don't discard frame, or stop when current status is discarding */
    VDEC_DISCARD_START,     /* Start to discard frame until mode switch to VDEC_DISCARD_STOP */
    VDEC_DISCARD_BY_NUM,    /* Discard appointed number frames */
    VDEC_DISCARD_BUTT
}VDEC_DISCARD_MODE_E;

typedef enum hiVDEC_PORT_ABILITY_E
{
    VDEC_PORT_HD,
    VDEC_PORT_SD,
    VDEC_PORT_STR,/*virtual window*/
    VDEC_PORT_BUTT
}VDEC_PORT_ABILITY_E;
typedef struct hiVDEC_PORT_CFG_S
{
    mt_handle *phPort;
    VDEC_PORT_ABILITY_E ePortAbility;
}VDEC_PORT_CFG_S;

typedef struct hiVDEC_DISCARD_FRAME_S
{
    VDEC_DISCARD_MODE_E enMode; /* Discard mode */
    mt_u32              u32Num; /* Discard number, usable when mode is VDEC_DISCARD_BY_NUM */
}VDEC_DISCARD_FRAME_S;

//add by l00225186
typedef int (*PFN_VDEC_Chan_VOAcqFrame)(mt_handle, MT_DRV_VIDEO_FRAME_S*);
typedef int (*PFN_VDEC_Chan_VORlsFrame)(mt_handle, MT_DRV_VIDEO_FRAME_S*);
typedef int (*PFN_VDEC_Chan_VOChangeWinInfo)(mt_handle,MT_DRV_WIN_PRIV_INFO_S*);
//add by l00225186
typedef struct tagVDEC_PORT_PARAM_S
{
   /*提供给VO,收帧，释放帧，处理窗口信息改变的函数*/
  PFN_VDEC_Chan_VOAcqFrame pfVOAcqFrame;
  PFN_VDEC_Chan_VORlsFrame pfVORlsFrame;
  PFN_VDEC_Chan_VOChangeWinInfo pfVOSendWinInfo;

  //Extend Freeze interfaces
  PFN_VDEC_Chan_VOAcqFrame pfAcqFreezeFrame;
  PFN_VDEC_Chan_VORlsFrame pfRlsFreezeFrame;

}VDEC_PORT_PARAM_S;
typedef struct tagVDEC_PORT_PARAM_WITHPORT_S
{
  mt_handle hPort;
  VDEC_PORT_PARAM_S stVdecPortParam;
}VDEC_PORT_PARAM_WITHPORT_S;
typedef enum hiVDEC_PORT_TYPE_E
{
    VDEC_PORT_TYPE_MASTER,
    VDEC_PORT_TYPE_SLAVE,
    VDEC_PORT_TYPE_VIRTUAL,
    VDEC_PORT_TYPE_BUTT
}VDEC_PORT_TYPE_E;

typedef struct tagVDEC_PORT_TYPE_WITHPORT_S
{
  mt_handle hPort;
  VDEC_PORT_TYPE_E enPortType;
}VDEC_PORT_TYPE_WITHPORT_S;

typedef struct tagVDEC_PORT_ATTR_WITHHANDLE_S
{
    mt_handle                   hPort;
    MT_DRV_VPSS_PORT_CFG_S      stPortCfg;
}VDEC_PORT_ATTR_WITHHANDLE_S;

typedef struct tagVDEC_PORT_ATTR_S
{
    MT_BOOL                   bVPU;
}VDEC_VPU_ATTR_S;
typedef struct mtUNF_VDEC_BUFF_ATTR_S
{
    ulong u32UsrVirAddr[MAX_VDEC_EXT_BUF_NUM];  /**<User virtual address *//**<CNcomment: 用户态虚拟地址*/
    phys_addr_t u32PhyAddr[MAX_VDEC_EXT_BUF_NUM];     /**<Physical address *//**<CNcomment: 物理地址*/
    mt_u32 u32BufNum;                          /**<Buffer number *//**<CNcomment: 缓冲区个数*/
	ulong u32BufSize;
    mt_u32 u32Stride;                          /**<Stride of external frame buffer *//**<CNcomment:外部帧存的stride*/
} VDEC_BUFFER_ATTR_S;
typedef enum hiVDEC_FRAMEBUFFER_MODE_E{
	VDEC_BUF_VPSS_ALLOC_MANAGE = 0,/**vpss buffer should alloc and manage by vpss*//**<CNcomment:vpss的buffer的分配和管理有vpss模块自己管理*/
    VDEC_BUF_USER_ALLOC_MANAGE,    /**vpss buffer should alloc and manage by user*//**<CNcomment:vpss的buffer分配由vdec的上层用户来分配，管理有vdec负责*/
    VDEC_BUF_TYPE_BUTT
}VDEC_FRAMEBUFFER_MODE_E;
typedef enum hiVDEC_FRAMEBUFFER_STATE_E{
	VDEC_BUF_STATE_EMPTY = 0,/**the frame buffer have no data*//**<CNcomment:要查询的buffer里面没有数据*/
    VDEC_BUF_STATE_FULL,     /**the frame buffer have data*//**<CNcomment:要查询的buffer里面已经被填充了有效数据*/
    VDEC_BUF_STATE_IN_USE,   /**the frame buffer is in using*//**<CNcomment:要查询的buffer正在被填充数据的过程中*/
    VDEC_BUF_STATE_BUTT
}VDEC_FRAMEBUFFER_STATE_E;
typedef struct mt_VDEC_BUFF_INFO_S
{
    phys_addr_t u32PhyAddr;
	VDEC_FRAMEBUFFER_STATE_E* penBufState;
} VDEC_BUFFER_INFO_S;

typedef enum hiVDEC_EXTBUFFER_STATE_E
{
    VDEC_EXTBUFFER_STATE_START,     /* control VPSS start use the extern buffer*/
    VDEC_EXTBUFFER_STATE_STOP,      /* control VPSS stop use the extern buffer*/
    VDEC_EXTBUFFER_STATE_BUTT
}VDEC_EXTBUFFER_STATE_E;

typedef struct tagVDEC_RESOLUTION_ATTR_S
{
    mt_s32                   s32Width;
	mt_s32                   s32Height;
}VDEC_RESOLUTION_ATTR_S;

/****************************** API Declaration ******************************/

mt_s32 MT_MPI_VDEC_Init(mt_void);
mt_s32 MT_MPI_VDEC_DeInit(mt_void);
mt_s32 MT_MPI_VDEC_RegisterVcodecLib(const mt_char *pszCodecDllName);
mt_s32 MT_MPI_VDEC_AllocChan(mt_handle *phHandle, const MT_UNF_AVPLAY_OPEN_OPT_S *pstMaxCapbility);
mt_s32 MT_MPI_VDEC_FreeChan(mt_handle hVdec);
mt_s32 MT_MPI_VDEC_SetChanAttr(mt_handle hVdec, const MT_UNF_VCODEC_ATTR_S *pstAttr);
mt_s32 MT_MPI_VDEC_GetChanAttr(mt_handle hVdec, MT_UNF_VCODEC_ATTR_S *pstAttr);
mt_s32 MT_MPI_VDEC_ChanBufferInit(mt_handle hVdec, mt_u32 u32BufSize, mt_handle hDmxVidChn, mt_u32 pip_en);
mt_s32 MT_MPI_VDEC_ChanBufferDeInit(mt_handle hVdec);
mt_s32 MT_MPI_VDEC_ResetChan(mt_handle hVdec, const MT_CODEC_RESETPARAM_S *pstParam);
mt_s32 MT_MPI_VDEC_ChanStart(mt_handle hVdec);
mt_s32 MT_MPI_VDEC_ChanStop(mt_handle hVdec);
mt_s32 MT_MPI_VDEC_ChanGetBuffer(mt_handle hVdec, mt_u32 u32RequestSize, VDEC_ES_BUF_S *pstBuf);
mt_s32 MT_MPI_VDEC_ChanPutBuffer(mt_handle hVdec, VDEC_ES_BUF_S *pstBuf);
mt_s32 MT_MPI_VDEC_ChanRecvFrm(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S* pstFrameInfo);
mt_s32 MT_MPI_VDEC_ChanRlsFrm(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S* pstFrameInfo);
mt_s32 MT_MPI_VDEC_ChanIFrameDecode(mt_handle hVdec, MT_UNF_AVPLAY_I_FRAME_S *pstIFrameStream,
                                    MT_DRV_VIDEO_FRAME_S *pstVoFrameInfo, MT_BOOL bCapture);
mt_s32 MT_MPI_VDEC_ChanIFrameRelease(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S *pstVoFrameInfo);
mt_s32 MT_MPI_VDEC_GetChanStatusInfo(mt_handle hVdec, VDEC_STATUSINFO_S *pstStatusInfo);
mt_s32 MT_MPI_VDEC_GetCiTestInfo(mt_handle hVdec, MT_UNF_AVPLAY_CI_TEST_INFO_S *pstInfo);
mt_s32 MT_MPI_VDEC_GetChanStreamInfo(mt_handle hVdec, MT_UNF_VCODEC_STREAMINFO_S *pstStreamInfo);
mt_s32 MT_MPI_VDEC_CheckNewEvent(mt_handle hVdec, VDEC_EVENT_S *pstNewEvent);
mt_s32 MT_MPI_VDEC_ReadNewFrame(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S *pstNewFrame);
mt_s32 MT_MPI_VDEC_ChanRecvUsrData(mt_handle hVdec, MT_UNF_VIDEO_USERDATA_S *pstUsrData);
mt_s32 MT_MPI_VDEC_SetChanFrmRate(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 MT_MPI_VDEC_SetChanFrmRateDeclear(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 MT_MPI_VDEC_GetChanFrmRate(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 MT_MPI_VDEC_SetEosFlag(mt_handle hVdec);
mt_s32 MT_MPI_VDEC_DiscardFrame(mt_handle hVdec, VDEC_DISCARD_FRAME_S* pstParam);
mt_s32 MT_MPI_VDEC_CreatePort(mt_handle hVdec, mt_handle *phPort, VDEC_PORT_ABILITY_E ePortAbility);
mt_s32 MT_MPI_VDEC_EnablePort(mt_handle hVdec,mt_handle hPort);
mt_s32 MT_MPI_VDEC_DisablePort(mt_handle hVdec,mt_handle hPort);
//mt_s32 MT_MPI_VDEC_SetMainPort(mt_handle hVdec,mt_handle hPort);
mt_s32 MT_MPI_VDEC_SetPortType(mt_handle hVdec, mt_handle hPort, VDEC_PORT_TYPE_E enPortType);
mt_s32 MT_MPI_VDEC_ChanPauseDecode(mt_handle hVdec, mt_u32 u32PauseFlag);

mt_s32 MT_MPI_VDEC_GetPortAttr(mt_handle hVdec, mt_handle hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg);

mt_s32 MT_MPI_VDEC_SetPortAttr(mt_handle hVdec, mt_handle hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg);

mt_s32 MT_MPI_VDEC_CancleMainPort(mt_handle hVdec,mt_handle hPort);
mt_s32 MT_MPI_VDEC_DestroyPort(mt_handle hVdec,mt_handle hPort);
mt_s32 MT_MPI_VDEC_GetPortParam(mt_handle hVdec, mt_handle hPort, VDEC_PORT_PARAM_S *pstParam);
mt_s32 MT_MPI_VDEC_ReceiveFrame(mt_handle hVdec, MT_DRV_VIDEO_FRAME_PACKAGE_S *pFrmPack);
mt_s32 MT_MPI_VDEC_GetChanFrmStatusInfo(mt_handle hVdec, mt_handle  hPort,VDEC_FRMSTATUSINFO_S *pstVdecFrmStatus);
mt_s32 MT_MPI_VDEC_SetChanFrmPackType(mt_handle hVdec, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType);
mt_s32 MT_MPI_VDEC_GetChanFrmPackType(mt_handle hVdec, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType);
mt_s32 MT_MPI_VDEC_AcqUserData(mt_handle hVdec,
                MT_UNF_VIDEO_USERDATA_S* pstUserData, MT_UNF_VIDEO_USERDATA_TYPE_E* penType);
mt_s32 MT_MPI_VDEC_RlsUserData(mt_handle hVdec, MT_UNF_VIDEO_USERDATA_S* pstUserData);
mt_s32 MT_MPI_VDEC_RstUserDataBuffer(mt_handle hVdec);

mt_s32 MT_MPI_VDEC_Invoke(mt_handle hVdec, MT_CODEC_VIDEO_CMD_S* pstParam);
mt_s32 MT_MPI_VDEC_GetChanOpenParam(mt_handle hVdec, MT_UNF_AVPLAY_OPEN_OPT_S *pstOpenPara);

mt_s32 MT_MPI_VDEC_ReleaseFrame(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pVideoFrame);
mt_s32 MT_MPI_VDEC_SetLowDelay(mt_handle hVdec, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *pstAttr);
mt_s32 MT_MPI_VDEC_ChanDropStream(mt_handle hVdec, mt_u64 *pSeekPts, mt_u32 u32Gap);
mt_s32 MT_MPI_VDEC_SetExternBuffer(mt_handle hVdec, VDEC_BUFFER_ATTR_S* pstBufAttr);
mt_s32 MT_MPI_VDEC_SetChanBufferMode(mt_handle hVdec,VDEC_FRAMEBUFFER_MODE_E enFrameBufferType);
mt_s32 MT_MPI_VDEC_CheckAndDeleteExtBuffer(mt_handle hVdec,mt_u32 u32PhyAddr,VDEC_FRAMEBUFFER_STATE_E* penBufState);
mt_s32 MT_MPI_VDEC_SetExternBufferState(mt_handle hVdec, VDEC_EXTBUFFER_STATE_E enExtBufferState);
mt_s32 MT_MPI_VDEC_SetResolution(mt_handle hVdec,VDEC_RESOLUTION_ATTR_S* pstResolution);

mt_s32 MT_MPI_VDEC_SetBuffClearnComp(mt_handle hVdec, MT_BOOL bVOClearnFlag);
mt_s32 MT_MPI_VDEC_DecFrmType(mt_handle hVdec, MT_UNF_DEC_FRM_TYPE_E eDecFrmType);
mt_s32 MT_MPI_VDEC_SetTrickCfg(mt_handle hVdec, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam);
mt_s32 MT_MPI_VDEC_ChanGetEsBuffer(mt_handle hVdec, phys_addr_t *u32PhyAddr, mt_u32 *u32BufSize);

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
mt_s32 MT_MPI_VDEC_GetLcevcStat(mt_handle hVdec);
void MT_MPI_VDEC_LcevcStart(mt_handle hVdec);
void MT_MPI_VDEC_LcevcStop(mt_handle hVdec);
void MT_MPI_VDEC_LcevcPause(mt_handle hVdec, mt_u32 u32PauseFlag);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif//__MT_MPI_VDEC_H__

