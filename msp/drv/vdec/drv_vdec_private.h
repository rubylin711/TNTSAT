/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_VDEC_PRIVATE_H__
#define __MT_VDEC_PRIVATE_H__

#include "mt_type.h"
#include "mt_module.h"
#include "mt_drv_sys.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "mt_drv_mem.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_module.h"
#include "drv_vdec_buf_mng.h"
#include "mt_unf_avplay.h"
#include "mt_drv_video.h"
#include "mt_drv_vdec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/* VDEC VERSION */
#define VDEC_VERSION                (2018020100)

#define VDEC_DBG_MODULE_VDI         (0x0)
#define VDEC_DEBUG                  (0)

#define VDEC_KUD_MAX_NUM 4      /*max user data num of each frame*/
#define VDEC_KUD_MAX_LEN 256UL  /*max user data length*/
#define VDEC_UDC_MAX_NUM 16
//add by l00225186
#define VDEC_MAX_PROC_ARGS_SIZE 30
#define VDEC_MAX_PORT_NUM 3
//Fake VPASS no need VPSS frames
//#define VDEC_MAX_PORT_FRAME 20
#define VDEC_MAX_PORT_FRAME 1
#define MT_KMALLOC_VDEC(size)           MT_KMALLOC(MT_ID_VDEC, size, GFP_KERNEL)
#define MT_KMALLOC_ATOMIC_VDEC(size)    MT_KMALLOC(MT_ID_VDEC, size, GFP_ATOMIC)
#define MT_KFREE_VDEC(addr)             MT_KFREE(MT_ID_VDEC, addr)
#define MT_VMALLOC_VDEC(size)           MT_VMALLOC(MT_ID_VDEC, size)
#define MT_VFREE_VDEC(addr)             MT_VFREE(MT_ID_VDEC, addr)

typedef enum tagVDEC_CHAN_STATE_E{
    VDEC_CHAN_STATE_STOP = 0,
    VDEC_CHAN_STATE_RUN,
    VDEC_CHAN_STATE_INVALID
}VDEC_CHAN_STATE_E;

typedef enum tagVDEC_FRAME_FORMAT_E{
    VDEC_I_FRAME = 0,
    VDEC_P_FRAME,
    VDEC_B_FRAME,
    VDEC_FRAME_BUTT
}VDEC_FRAME_FORMAT_E;

typedef enum hiDRV_VDEC_FRAMEBUFFER_TYPE_E{
    MT_DRV_VDEC_BUF_VPSS_ALLOC_MANAGE = 0,
    MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE,
    MT_DRV_VDEC_BUF_VDEC_ALLOC_MANAGE,
    MT_DRV_VDEC_BUF_USER_ALLOC_VPSS_MANAGE,
    MT_DRV_VDEC_BUF_TYPE_BUTT
}MT_DRV_VDEC_FRAMEBUFFER_TYPE_E;
typedef enum hiDRV_VDEC_FRAMEBUFFER_STATE_E{
    MT_DRV_VDEC_BUF_STATE_IN_VDEC_EMPTY = 0,
    MT_DRV_VDEC_BUF_STATE_IN_VDEC_FULL,
    MT_DRV_VDEC_BUF_STATE_IN_VPSS,
    MT_DRV_VDEC_BUF_STATE_IN_USER,
    MT_DRV_VDEC_BUF_STATE_BUTT
}MT_DRV_VDEC_FRAMEBUFFER_STATE_E;
typedef struct tagVDEC_CHAN_STATINFO_S{
    mt_u32 u32TotalVdecOutFrame;    /*the number of total output frames*/
    mt_u32 u32TotalVdecParseIFrame; /*the number of total I frames decoded from stream*/
    mt_u32 u32TotalVdecInByte;      /*total bytes of the input stream*/
    mt_u32 u32TotalVdecHoldByte;    /*bytes of stream in VDEC_Frimware buffer*/
    mt_u32 u32TotalVdecTime;        /*total run time of the vdec channel*/
    mt_u32 u32CalcBpsVdecTime;      /*run time of the vdec channel, can be reset, used by calculate bps*/
    mt_u32 u32AvrgVdecFps;          /*the integer part of average output frame rate*/
    mt_u32 u32AvrgVdecFpsLittle;    /*the decimal part of average output frame rate*/
    mt_u32 u32AvrgVdecInBps;        /*the average stream input bit rate(bps)*/
    mt_u32 u32TotalStreamErrNum;    /*the totoal error number of stream*/

    mt_u32 u32VdecRcvFrameTry;      /*the frame number try to acquire*/
    mt_u32 u32VdecRcvFrameOK;       /*the frame number acquire success*/
    mt_u32 u32VdecRlsFrameTry;      /*the frame number try to release*/
    mt_u32 u32VdecRlsFrameOK;       /*the frame number release success*/
    mt_u32 u32VdecRlsFrameFail;     /*the frame number release fail*/
    mt_u32 u32VdecErrFrame;         /*the error frame number*/
    mt_u32 u32VdecDecErrFrame;         /*the number of Frame which pErrRatio is not 0*/

    mt_u32 u32VdecAcqBufTry;
    mt_u32 u32VdecAcqBufOK;
    mt_u32 u32VdecRlsBufTry;
    mt_u32 u32VdecRlsBufOK;

    mt_u32 u32UserAcqFrameTry;
    mt_u32 u32UserAcqFrameOK;
    mt_u32 u32UserRlsFrameTry;
    mt_u32 u32UserRlsFrameOK;

    mt_u32 u32AvplayRcvFrameTry;
    mt_u32 u32AvplayRcvFrameOK;
    mt_u32 u32AvplayRlsFrameTry;
    mt_u32 u32AvplayRlsFrameOK;

    mt_u32 u32FrameType[2];
}VDEC_CHAN_STATINFO_S;

typedef struct
{
    mmz_buffer_s    stMMZBuf;
    mmz_buffer_s    st2dBuf;
    mt_u32          u32ReadTimes;
}VDEC_IFRAME_PARAM_S;

typedef struct
{
    mt_u32                  u32ReadID;
    mt_u32                  u32WriteID;
    mt_u8                   au8Buf[VDEC_UDC_MAX_NUM][MAX_USER_DATA_LEN];
    MT_UNF_VIDEO_USERDATA_S stAttr[VDEC_UDC_MAX_NUM];
}VDEC_USRDATA_PARAM_S;

typedef struct
{
    atomic_t                atmWorking;
    wait_queue_head_t       stWaitQue;
    MT_DRV_VIDEO_FRAME_S*  pstFrame;
}VDEC_BTL_PARAM_S;
//add by l00225186
typedef struct BUFMNG_VPSS_LOCK_S
{
    spinlock_t     irq_lock;
    unsigned long  irq_lockflags;
    int            isInit;
} BUFMNG_VPSS_IRQ_LOCK_S;
typedef struct
{
    MT_DRV_VIDEO_FRAME_S stVpssOutFrame;
    MT_BOOL bBufUsed;
    struct list_head node;
    mmz_buffer_s stMMZBuf;
    MT_DRV_VDEC_FRAMEBUFFER_STATE_E enFrameBufferState;
}BUFMNG_VPSS_NODE_S;
typedef struct
{
    struct list_head stVpssBufAvailableList;
    struct list_head stVpssBufUnAvailableList;
    struct list_head* pstUnAvailableListPos;
    mt_u32 u32BufNum;
    mt_u32 u32BufSize;
    BUFMNG_VPSS_IRQ_LOCK_S stAvailableListLock;
    BUFMNG_VPSS_IRQ_LOCK_S stUnAvailableListLock;
    VDEC_BUFFER_ATTR_S stBufferAttr;
    MT_DRV_VDEC_FRAMEBUFFER_TYPE_E enFrameBuffer;
    mt_u32 u32AvaiableFrameCnt;
    VDEC_EXTBUFFER_STATE_E enExtBufferState;
}BUFMNG_VPSS_INST_S;

typedef struct tagVDEC_PORT_FRAME_LIST_NODE_S
{
    MT_DRV_VIDEO_FRAME_S stPortOutFrame;
    struct list_head node;
}VDEC_PORT_FRAME_LIST_NODE_S;

typedef struct tagVDEC_PORT_FRAME_LIST_LOCK_S
{
    spinlock_t     irq_lock;
    unsigned long  irq_lockflags;
    int            isInit;
} VDEC_PORT_FRAME_LIST_LOCK_S;

typedef struct tagVDEC_PORT_LIST_S
{
    struct list_head stVdecPortFrameList;
    VDEC_PORT_FRAME_LIST_LOCK_S stPortFrameListLock;
}VDEC_PORT_LIST_S;

typedef struct
{
    mt_handle           hPort;
    MT_BOOL             bMainPort; /*0:SlavePort 1:MainPort*/
    VDEC_PORT_TYPE_E    enPortType;
    MT_BOOL             bEnable;/*0:enable 1:disable*/
    BUFMNG_VPSS_INST_S  stBufVpssInst;
    MT_DRV_VPSS_BUFFER_TYPE_E bufferType;
    VDEC_PORT_LIST_S    stPortList;
    VDEC_PORT_FRAME_LIST_NODE_S astPortTmpList[VDEC_MAX_PORT_FRAME];
    mt_s32 s32PortTmpListPos;
    mt_s32 s32GetFirstVpssFrameFlag;
    mt_s32 s32RecvNewFrame;
    mt_s32 s32PortLastFrameGopNum;
    mt_u32 u32LastFrameIndex;
}VDEC_VPSS_PORT_PARAM_S;

typedef struct tagVDEC_VPU_FRAME_LIST_NODE_S
{
    MT_DRV_VIDEO_FRAME_S stPortOutFrame;
    struct list_head node;
}VDEC_VPU_FRAME_LIST_NODE_S;
typedef struct tagVDEC_VPU_LIST_S
{
    struct list_head stVdecVPUFrameList;
    BUFMNG_VPSS_IRQ_LOCK_S stVPUFrameListLock;
}VDEC_VPU_LIST_S;
typedef struct tagVDEC_VPU_RLS_PARAM_S
{
    mt_s32 RlsFrameIDArray[MT_VDEC_MAX_VPU_FRAME_NUM];
    mt_s32 s32AvailableNum;
    BUFMNG_VPSS_IRQ_LOCK_S stVPURlsFrameListLock;
}VDEC_VPU_RLS_PARAM_S;
typedef struct
{
    VDEC_VPU_LIST_S stVPUFrameList;
    VDEC_VPU_RLS_PARAM_S stVPURlsParam;
}VDEC_VPU_PARAM_S;

typedef struct tagVDEC_CHANNEL_S
{
    /* For configurate */
    MT_UNF_VCODEC_ATTR_S    stCurCfg;       /* Configurate */
    MT_UNF_AVPLAY_OPEN_OPT_S stUserCfgCap;

    /* State */
    VDEC_CHAN_STATE_E       enCurState;

    /* Parameters from or to VFMW */
    VDEC_CHAN_CAP_LEVEL_E   enCapToFmw;
    DETAIL_MEM_SIZE         stMemSize;
    VDEC_CHAN_OPTION_S      stOption;
    mmz_buffer_s            stSCDMMZBuf;
    mmz_buffer_s            stVDHMMZBuf;
    STREAM_INTF_S           stStrmIntf ;
    IMAGE_INTF_S            stImageIntf;
    MT_VDEC_USRDAT_S*          pu8UsrDataForWaterMark[4];

    /* Save last frame */
   // MT_UNF_VIDEO_FRAME_INFO_S stLastFrm;
   //chang by l00225186
    MT_DRV_VIDEO_FRAME_S    stLastFrm;
    //MT_DRV_VIDEO_FRAME_S    stLastVpssFrm;
    mt_u32                  u32UserSetAspectWidth;
    mt_u32                  u32UserSetAspectHeight;
    mt_u32                  u32DecodeAspectWidth;
    mt_u32                  u32DecodeAspectHeight;
    mt_u32                  u32LastFrmId;
    mt_u32                  u32LastFrmTryTimes;
    mt_u32                  u32EndFrmFlag;
    
    MT_BOOL                 bIsLastFrame;
    MT_BOOL                 bEndOfStrm;
    MT_UNF_ENC_FMT_E        enDisplayNorm;
    MT_UNF_VIDEO_FRAME_PACKING_TYPE_E eFramePackType;
    /* Last display frame info */
    //MT_DRV_VIDEO_FRAME_S stLastDispFrameInfo;////////123

    mt_u32                  u32BitRate;
    mt_u32                  u32Profile;
    mt_u32                  u32LastLumaBitdepth;     // proc 10bit信息
    mt_u32                  u32LastChromaBitdepth;   // proc 10bit信息
    mt_u32                  u32Level;

    /* Channel status */
    VDEC_CHAN_STATINFO_S    stStatInfo;

    /* For stream buffer */
    mt_handle               hChan;          /* VFMW handle */
    //mt_handle               hVpss;          /* VPSS handle */
    mt_handle               hVdec;          /*VDEC handle*/
    mt_handle               hDmxVidChn;     /* Dmx handle */
    mt_u32                  u32DmxID;
    mt_u32                  u32DmxBufSize;  /* Dmx buffer size */
    mt_handle               hStrmBuf;       /* Stream buffer handle */
    mt_u32                  u32StrmBufSize; /* Stream buffer size */

    /* For event */
    MT_BOOL                             bNormChange;
    MT_UNF_NORMCHANGE_PARAM_S           stNormChangeParam;
    MT_BOOL                             bFramePackingChange;
    MT_UNF_VIDEO_FRAME_PACKING_TYPE_E   enFramePackingType;
    MT_BOOL                             bNewSeq;
    MT_BOOL                 bNewFrame;      /* use tlastFrm */
    MT_BOOL                 bNewUserData;
    MT_BOOL                 bIFrameErr;
    MT_BOOL                 bImageSizeChange;
    MT_BOOL                 bNoGetImage;

    /*0: 1st and 2nd not get; 1: 1st get 2nd not get; 2: 1st and 2nd both get*/
    mt_u32                  u32ValidPtsFlag;

    MT_BOOL                 bFirstValidPts;
    mt_u32                  u32FirstValidPts;
    MT_BOOL                 bSecondValidPts;
    mt_u32                  u32SecondValidPts;

    /* For I frame decode */
    VDEC_IFRAME_PARAM_S     stIFrame;

    /* For user data */
    VDEC_USRDATA_PARAM_S*   pstUsrData;

    /* For capture BTL */
    VDEC_BTL_PARAM_S        stBTL;

    /* For frame rate */
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrameRateParam;

    /* For stream discontinue */
    mt_u32                  u32DiscontinueCount;

    //add by l00225186
    //VDEC_VPSS_PORT_PARAM_S       stPort[VDEC_MAX_PORT_NUM];

    /* For trick mode */
    mt_s32                  s32Speed;

    /* For resolution change */
    mt_u8                   u8ResolutionChange;

    MT_BOOL                 bIsIFrameDec;
    MT_BOOL                 bUnSupportStream;

    MT_BOOL                 bLowdelay;
    MT_BOOL                 bProcRegister;
    MT_BOOL                 bVPUProcRegister;
    mt_handle               u32VPUhandle;
    VDEC_VPU_PARAM_S        stVPUParam;
    MT_BOOL                 bVPU;
    mt_u32                  u32FrameCnt;
    mt_u32                  u32ErrRatio;
    MT_DRV_VDEC_VPU_STATUS_S  stVdecVpuStatus;
    mt_u32                  u32FrameSize;//l00273086
    mt_u32                  u32RefFrameNum; //l00273086
    MT_BOOL                 bNeedAlloc;//l00273086
    MT_BOOL                 bDPBFullCtrl;

	//Patch:
	//when flush, VPSS may store some frames processed or not processed,
	//and can not do release internally(can only reset internally),
	//so need to receive/release them all.
	MT_DRV_VIDEO_FRAME_PACKAGE_S	stVpssFramePack;

	//Freeze Frame Buffer
	MT_DIS_FRAME_SLOT_INFO_T stFreezeFrame;

	/* avoid stack overflow, get image from VFMW, but may not thread safe! */
    IMAGE stImage;

	mmz_buffer_s			stCrcBuf;
	mt_u32 last_recv_time;
}VDEC_CHANNEL_S;

typedef struct tagVDEC_CONTROLINFO_S{
    mt_s32  u32BackwardOptimizeFlag;  /*Backward optimize flag, 1 means optimize the backward fast play performance*/
    mt_s32  u32DispOptimizeFlag;      /*Display optimize flag, 1 means optimize the VO display performance*/
}VDEC_CONTROLINFO_S;

typedef struct tagVDEC_VPSSCHANNEL_S
{
    mt_handle hVdec;
    mt_handle hVpss;
    MT_BOOL bUsed;
    MT_UNF_VIDEO_FRAME_PACKING_TYPE_E eFramePackType;
    VDEC_VPSS_PORT_PARAM_S       stPort[VDEC_MAX_PORT_NUM];
    VDEC_CONTROLINFO_S stControlInfo;
    mt_s32 s32Speed;
    mt_s32 s32GetFirstIFrameFlag;
    mt_s32 s32ImageDistance;
    mt_s32 s32GetFirstVpssFrameFlag;
    VDEC_FRAME_FORMAT_E eLastFrameFormat;
    mt_s32 s32LastFrameGopNum;
    MT_DRV_VDEC_FRAMEBUFFER_TYPE_E enFrameBuffer;
    VDEC_BUFFER_ATTR_S stBufferAttr;
}VDEC_VPSSCHANNEL_S;

typedef struct tagVDEC_REGISTER_PARAM_S{
    mt_proc_read_func  pfnCtrlReadProc;
    mt_drv_proc_write_func pfnCtrlWriteProc;
    mt_proc_read_func  pfnReadProc;
    mt_drv_proc_write_func pfnWriteProc;
    mt_proc_read_func  pfnVpuReadProc;
    mt_drv_proc_write_func pfnVpuWriteProc;
}VDEC_REGISTER_PARAM_S;

typedef struct tagVDEC_PREMMZ_NODE_S
{
    void * u32StartVirAddr;
    phys_addr_t  u32StartPhyAddr;
    ulong  u32Size;
    mt_u32  u32NodeState;/*0:have MMZ not used,1:have MMZ but used 2:invalid*/
}VDEC_PREMMZ_NODE_S;

typedef int (*FN_VDEC_Watermark)(MT_DRV_VIDEO_FRAME_S  *, MT_VDEC_USRDAT_S* pu8UsrDataForWaterMark[4]);

mt_s32 VDEC_DRV_RegWatermarkFunc(FN_VDEC_Watermark pfnFunc);
mt_void VDEC_DRV_UnRegWatermarkFunc(mt_void);

mt_s32 VDEC_DRV_RegisterProc(VDEC_REGISTER_PARAM_S *pstParam);
mt_void VDEC_DRV_UnregisterProc(mt_void);
mt_s32 VDEC_DRV_Suspend(basedev_s *pdev, pm_message_t state);
mt_s32 VDEC_DRV_Resume(basedev_s *pdev);
mt_s32 VDEC_DRV_DebugCtrl(mt_u32 u32Para1, mt_u32 u32Para2);
VDEC_CHANNEL_S * VDEC_DRV_GetChan(mt_handle hHandle);

mt_s32 VDEC_DRV_Init(mt_void);
mt_void VDEC_DRV_Exit(mt_void);
mt_s32 VDEC_DRV_Open(struct inode *inode,  struct file  *filp);
mt_s32 VDEC_DRV_Release(struct inode *inode,  struct file  *filp);
mt_s32 VDEC_Ioctl(struct inode *inode,  struct file  *filp,  unsigned int  cmd,  void *arg);
mt_s32 VDEC_FindVpssHandleByVdecHandle(mt_handle hVdec, mt_handle *phVpss);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_VDEC_KER_TEST_H__ */

