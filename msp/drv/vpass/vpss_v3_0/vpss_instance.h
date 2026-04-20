/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_INSTANCE_H__
#define __VPSS_INSTANCE_H__

#include "mt_type.h"
#include"drv_vpss_ext.h"
#include "mt_drv_mmz.h"
#include "vpss_fb.h"
#include "linux/list.h"
#include "vpss_osal.h"
#include "vpss_in.h"
#include "vpss_hal_3798m.h"
#include "vpss_info.h"
#include "drv_vdec_ext.h"
#include "mt_unf_video.h"
#include "drv_pq_ext.h"
#include "vpss_alg_scd.h"

#define VPSS_PORT_MAX_NUMB 4


typedef struct hiVPSS_PORT_PRC_S{
    mt_s32 s32PortId;
    MT_BOOL bEnble;

    VPSS_FB_STATE_S stFbPrc;
    MT_DRV_VPSS_BUFLIST_CFG_S stBufListCfg;
    MT_DRV_PIX_FORMAT_E eFormat;
    mt_s32  s32OutputWidth;
    mt_s32  s32OutputHeight;
    MT_DRV_COLOR_SPACE_E eDstCS;

    MT_DRV_ASPECT_RATIO_S stDispPixAR;
    MT_DRV_ASP_RAT_MODE_E eAspMode;
    MT_DRV_ASPECT_RATIO_S stCustmAR;
    MT_BOOL b3Dsupport;
    MT_BOOL   bInterlaced;
    mt_rect_s stScreen;
    mt_u32 u32MaxFrameRate;
    mt_u32 u32OutCount;

    MT_DRV_VPSS_PORT_PROCESS_S stProcCtrl;

    MT_BOOL  bTunnelEnable;
    mt_s32  s32SafeThr;

    MT_DRV_VPSS_ROTATION_E enRotation;
    MT_BOOL bHoriFlip;
    MT_BOOL bVertFlip;
}VPSS_PORT_PRC_S;

typedef enum hiVPSS_INSTANCE_STATE_E{
    INSTANCE_STATE_STOP = 0,
    INSTANCE_STATE_WORING,
    INSTANCE_STATE_BUTT
}VPSS_INSTANCE_STATE_E;

typedef struct hiVPSS_PORT_S{
    mt_s32 s32PortId;
    MT_BOOL bEnble;

    VPSS_FB_INFO_S stFrmInfo;

    MT_DRV_PIX_FORMAT_E eFormat;
    mt_s32  s32OutputWidth;
    mt_s32  s32OutputHeight;
    MT_DRV_COLOR_SPACE_E eDstCS;

    MT_DRV_ASPECT_RATIO_S stDispPixAR;
    MT_DRV_ASP_RAT_MODE_E eAspMode;
    MT_DRV_ASPECT_RATIO_S stCustmAR;

    MT_BOOL b3Dsupport;

    MT_BOOL   bInterlaced;
    mt_rect_s stScreen;

    mt_u32 u32MaxFrameRate;
    mt_u32 u32OutCount;

    MT_DRV_VPSS_PORT_PROCESS_S stProcCtrl;

    MT_BOOL  bTunnelEnable;
    mt_s32  s32SafeThr;

    MT_BOOL bOnlyKeyFrame;      /* 配合硬件FRC工作，电影模式源需要输出非重复帧，TV独有 */
    MT_BOOL bLBDCropEn;         /* 动态CROP已检测到的黑边，TV独有 */
    mt_rect_s stVideoRect;      /* TV LBX需求 */

    mt_rect_s stInRect;
    MT_BOOL   bUseCropRect;
    MT_DRV_CROP_RECT_S stCropRect;

    MT_DRV_PIXEL_BITWIDTH_E  enOutBitWidth;

    MT_DRV_VPSS_ROTATION_E enRotation;
    MT_BOOL bHoriFlip;
    MT_BOOL bVertFlip;

}VPSS_PORT_S;

typedef struct hiVPSS_STREAM_ORIGINFO_S{
    mt_u32 u32StreamInRate;
    MT_BOOL u32StreamTopFirst;
    MT_BOOL u32StreamProg;
    MT_DRV_PIX_FORMAT_E ePixFormat;
}VPSS_STREAM_ORIGINFO_S;

#define VPSS_LBX_DET_NODE_NUM 32
typedef struct hiVPSS_LBX_DET_S
{
    mt_u32 m_top[VPSS_LBX_DET_NODE_NUM];
    mt_u32 m_bot[VPSS_LBX_DET_NODE_NUM];
    mt_u32 m_left[VPSS_LBX_DET_NODE_NUM];
    mt_u32 m_right[VPSS_LBX_DET_NODE_NUM];

    mt_u32 u32NodeIndex;

    mt_u32 u32valid_top;
    mt_u32 u32valid_bot;
    mt_u32 u32valid_left;
    mt_u32 u32valid_right;
}VPSS_LBX_DET_S;


typedef struct hiVPSS_INSTANCE_S
{
    mt_s32  ID;                       //实例ID
    mt_s32  CtrlID;                //实例隶属的CTRL实体ID
    VPSS_INSTANCE_STATE_E enState;  //实例状态 WORK/STOP
    VPSS_OSAL_LOCK stInstLock;      //实例资源锁，线程与接口读写保护
    VPSS_OSAL_SPIN stUsrSetSpin;
    VPSS_REG_S stPqRegData;             //PQ初始化数据
    MT_PQ_VPSS_MODULE_S stPQModule;
    MT_PQ_IFMD_PLAYBACK_S stIfmdRls;    //IFMD数据
    MT_PQ_PFMD_PLAYBACK_S stPfmdRls;    //PFMD数据
    MT_PQ_MOTION_INFO_S stGlbMotionRls;	//GlobalMotion数据
    MT_PQ_DB_WEIGHT_S stDBRls;          //DB数据
    SCDRls   stSCDRls;                  //SCD数据
    VPSS_LBX_DET_S stLbxDet;

    MT_DRV_VIDEO_FRAME_S stSrcImage;
    /*用户接口配置*/
    MT_BOOL bCfgNew;           //用户配置更新
    MT_DRV_VPSS_CFG_S stUsrInstCfg;    //用户配置数据
    MT_DRV_VPSS_PORT_CFG_S  stUsrPortCfg[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER];
    MT_DRV_VPSS_PROCESS_S stProcCtrl; //看是否保留

    /*低延时总处理最新一帧*/
    MT_BOOL bAlwaysFlushSrc;

    VPSS_SRCIN_S stSrcIn;

    /*debug proc 信息*/
    VPSS_DBG_S stDbgCtrl;

    VPSS_RWZB_S stRwzbInfo;

    VPSS_IN_ENTITY_S stInEntity;

    VPSS_PORT_S stPort[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER];

    MT_BOOL abNodeVaild[VPSS_HAL_TASK_NODE_BUTT];

    /*用户注册回调*/
    mt_handle hDst;
    PFN_VPSS_CALLBACK pfUserCallBack;

    /*与前级交互模式*/
    MT_DRV_VPSS_SOURCE_MODE_E eSrcImgMode;
    MT_DRV_VPSS_SOURCE_FUNC_S stSrcFuncs;

	mt_u32 u32UhdLevelW;
	mt_u32 u32UhdLevelH;

    mt_u32 u32CheckRate;
    mt_u32 u32CheckSucRate;
    mt_u32 u32CheckCnt;
    mt_u32 u32CheckSucCnt;
    mt_u32 u32LastCheckTime;

    mt_u32 u32ImgRate;
    mt_u32 u32ImgSucRate;
    mt_u32 u32ImgCnt;
    mt_u32 u32ImgSucCnt;
    mt_u32 u32ImgLastCnt;
    mt_u32 u32ImgLastSucCnt;

    mt_u32 u32SrcRate;
    mt_u32 u32SrcSucRate;
    mt_u32 u32SrcCnt;
    mt_u32 u32SrcSucCnt;

    mt_u32 u32BufRate;
    mt_u32 u32BufSucRate;
    mt_u32 u32BufCnt;
    mt_u32 u32BufSucCnt;

    mt_u32 u32ScenceChgCnt;
}VPSS_INSTANCE_S;


/* FUNCTIONS FOR CTRL */
mt_s32 VPSS_INST_SyncUsrCfg(VPSS_INSTANCE_S *pstInstance);
mt_s32 VPSS_INST_CheckInstAvailable(VPSS_INSTANCE_S *pstInstance);
mt_s32 VPSS_INST_CompleteImage(VPSS_INSTANCE_S *pstInstance);
mt_s32 VPSS_INST_GetFrmBuffer(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE hPort,
                    MT_DRV_VPSS_BUFLIST_CFG_S* pstBufCfg,VPSS_BUFFER_S *pstBuffer,
                    mt_u32 u32StoreH,mt_u32 u32StoreW);

mt_s32 VPSS_INST_RelFrmBuffer(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE  hPort,
                                MT_DRV_VPSS_BUFLIST_CFG_S   *pstBufCfg,
                                mmz_buffer_s *pstMMZBuf);
mt_s32 VPSS_INST_ReportNewFrm(VPSS_INSTANCE_S* pstInstance,
                                VPSS_HANDLE  hPort,MT_DRV_VIDEO_FRAME_S *pstFrm);
MT_BOOL VPSS_INST_CheckIsDropped(VPSS_INSTANCE_S *pstInstance,mt_u32 u32OutRate,mt_u32 u32OutCount);

VPSS_HAL_NODE_TYPE_E VPSS_INST_Check2DNodeType(VPSS_INSTANCE_S* pstInst);
VPSS_HAL_NODE_TYPE_E VPSS_INST_Check3DNodeType(VPSS_INSTANCE_S* pstInst);
mt_void VPSS_INST_SetHalFrameInfo(MT_DRV_VIDEO_FRAME_S *pstFrame,
    VPSS_HAL_FRAME_S *pstHalFrm, MT_DRV_BUF_ADDR_E enBufLR);
mt_void VPSS_INST_SetOutFrameInfo(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, VPSS_BUFFER_S *pstBuf,
    MT_DRV_VIDEO_FRAME_S *pstFrm, MT_DRV_BUF_ADDR_E enBufLR);
mt_void VPSS_INST_SetRotationOutFrameInfo(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, VPSS_BUFFER_S* pstBuf,
                                  MT_DRV_VIDEO_FRAME_S* pstFrm, MT_DRV_BUF_ADDR_E enBufLR);

mt_void VPSS_INST_GetInCrop(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, mt_rect_s *pstInCropRect);
mt_void VPSS_INST_GetVideoRect(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, mt_rect_s *pstInCropRect, mt_rect_s *pstVideoRect);
mt_void VPSS_INST_GetLbxInfo(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, mt_rect_s *pstLbx);
mt_s32 VPSS_INST_GetPortPrc(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE hPort,VPSS_PORT_PRC_S *pstPortPrc);
mt_void VPSS_INST_GetRotate(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, VPSS_HAL_PORT_INFO_S *pstHalPortInfo, MT_DRV_VIDEO_FRAME_S *pstFrm);


/* FUNCTIONS FOR INTF */
mt_s32 VPSS_INST_Init(VPSS_INSTANCE_S *pstInstance,MT_DRV_VPSS_CFG_S *pstVpssCfg);
mt_s32 VPSS_INST_DelInit(VPSS_INSTANCE_S *pstInstance);
mt_s32 VPSS_INST_GetDefInstCfg(MT_DRV_VPSS_CFG_S *pstVpssCfg);
mt_s32 VPSS_INST_SetInstCfg(VPSS_INSTANCE_S *pstInstance,MT_DRV_VPSS_CFG_S *pstVpssCfg);
mt_u32 VPSS_INST_GetInstCfg(VPSS_INSTANCE_S *pstInstance,MT_DRV_VPSS_CFG_S *pstVpssCfg);

mt_s32 VPSS_INST_CreatePort(VPSS_INSTANCE_S *pstInstance,MT_DRV_VPSS_PORT_CFG_S *pstPortCfg,VPSS_HANDLE *phPort);
mt_s32 VPSS_INST_DestoryPort(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort);
mt_u32 VPSS_INST_GetDefPortCfg(MT_DRV_VPSS_PORT_CFG_S *pstPortCfg);
mt_s32 VPSS_INST_CheckPortCfg(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort,MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg);
mt_s32 VPSS_INST_GetPortCfg(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg);
mt_s32 VPSS_INST_SetPortCfg(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg);

mt_s32 VPSS_INST_RelPortFrame(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort,MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 VPSS_INST_GetPortFrame(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort,MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 VPSS_INST_EnablePort(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort,MT_BOOL bEnPort);

mt_s32 VPSS_INST_SetUserActiveMode(VPSS_INSTANCE_S * pstInstance);

mt_s32 VPSS_INST_ReplyUserCommand(VPSS_INSTANCE_S * pstInstance,
                                    MT_DRV_VPSS_USER_COMMAND_E eCommand,
                                    mt_void *pArgs);

mt_s32 VPSS_INST_SetCallBack(VPSS_INSTANCE_S *pstInstance,mt_handle hDst, PFN_VPSS_CALLBACK pfVpssCallback);

mt_s32 VPSS_INST_GetPortListState(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE hPort,MT_DRV_VPSS_PORT_BUFLIST_STATE_S *pstListState);

mt_s32 VPSS_INST_UpdatePqInfo(VPSS_INSTANCE_S *pstInstance,mt_u32 u32Width,mt_u32 u32Height);

mt_s32 VPSS_INST_GetSrcListState(VPSS_INSTANCE_S* pstInstance,VPSS_IMAGELIST_STATE_S *pstListState);

#endif
