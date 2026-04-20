/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_AVPLAY_IOCTL_H__
#define __DRV_AVPLAY_IOCTL_H__

#include "mt_type.h"
#include "mt_unf_avplay.h"
#include "mt_drv_adec.h"
#include "mt_mpi_mem.h"
#include "mt_mpi_stat.h"
#include "mt_mpi_vdec.h"
#include "mt_mpi_adec.h"
#include "mt_mpi_demux.h"
#include "mt_mpi_sync.h"
#include "mt_mpi_win.h"
#include "mt_mpi_ao.h"
#include "mt_mpi_demux.h"
#include "mt_drv_avplay.h"
#include "mt_module_debug.h"
#ifndef __KERNEL__
#include <semaphore.h>
#endif
#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif

//#define  AVPLAY_VID_THREAD

#define  AVPLAY_MAX_NUM                 16
#define  AVPLAY_MAX_WIN                 6
#define  AVPLAY_MAX_DMX_AUD_CHAN_NUM    16
#define  AVPLAY_MAX_TRACK               6

#define  AVPLAY_MAX_PORT_NUM            3    //The max num of port
#define  AVPLAY_MAX_SLAVE_FRMCHAN       2    //The max num of slave port
#define  AVPLAY_MAX_VIR_FRMCHAN         2    //The max num of virtual port

#define    AVPLAY_DFT_VID_SIZE       (5*1024*1024)
#define    AVPLAY_MIN_VID_SIZE       (512*1024)
#define    AVPLAY_MAX_VID_SIZE       (128*1024*1024)

#define    AVPLAY_TS_DFT_AUD_SIZE    (384*1024)
#define    AVPLAY_ES_DFT_AUD_SIZE    (256*1024)
#define    AVPLAY_MIN_AUD_SIZE       (192*1024)
#define    AVPLAY_MAX_AUD_SIZE       (4*1024*1024)

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define    AVPLAY_ADEC_FRAME_NUM     (16)
#else
#define    AVPLAY_ADEC_FRAME_NUM     (8)
#endif

#define    AVPLAY_SYS_SLEEP_TIME     (10)

#define    APPLAY_EOS_BUF_MIN_LEN    (1024)
#define    AVPLAY_EOS_TIMEOUT        (2000)
#define    APPLAY_EOS_STREAM_THRESHOLD (2)


/* video buffer dither waterline */
/* CNcomment: 视频缓冲管理抖动水线的百分比，0-99 */
#define    AVPLAY_ES_VID_FULL_PERCENT    93
#define    AVPLAY_ES_VID_HIGH_PERCENT    70
#define    AVPLAY_ES_VID_LOW_PERCENT     30
#define    AVPLAY_ES_VID_EMPTY_PERCENT   10

/* audio buffer dither waterline */
/* CNcomment: 音频缓冲管理抖动水线的百分比，0-99 */
#define    AVPLAY_ES_AUD_FULL_PERCENT    98
#define    AVPLAY_ES_AUD_HIGH_PERCENT    85
#define    AVPLAY_ES_AUD_LOW_PERCENT     2
#define    AVPLAY_ES_AUD_EMPTY_PERCENT   1

/* max delay time of adec in buffer */
#define    AVPLAY_ADEC_MAX_DELAY        1200

#define    AVPLAY_THREAD_TIMEOUT        30

#define    AVPLAY_VDEC_SEEKPTS_THRESHOLD 5000


typedef    mt_s32 (*AVPLAY_EVT_CB_FN)(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E EvtMsg, ulong EvtPara);

typedef enum mtAVPLAY_PROC_ID_E
{
    AVPLAY_PROC_ADEC_AO,
    AVPLAY_PROC_ADEC_AO2,
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    AVPLAY_PROC_ADEC_AO3,
#endif	
    AVPLAY_PROC_DMX_ADEC,
    AVPLAY_PROC_VDEC_VO,
    AVPLAY_PROC_BUTT
}AVPLAY_PROC_ID_E;

typedef struct tagAVPLAY_VID_PORT_AND_WIN_S
{
    mt_handle   hWindow;
    mt_handle   hPort;
}AVPLAY_VID_PORT_AND_WIN_S;

typedef struct tagAVPLAY_FRC_CTRL_S
{
	/* frame rate conversion state for progressive frame : <0-drop; ==0-natrual play; >0-repeat time */
	mt_s32 s32FrmState;
}AVPLAY_FRC_CTRL_S;

typedef struct tagAVPLAY_FRC_CFG_S
{
	mt_u32  u32InRate;     /* unit: frame */
	mt_u32  u32OutRate;  /* fresh rate */
	mt_u32  u32PlayRate;  /* play rate*/
}AVPLAY_FRC_CFG_S;

typedef struct tagAVPLAY_ALG_FRC_S
{
	mt_u32  u32InRate;     /* unit: frame */
	mt_u32  u32OutRate;  /* fresh rate */
	mt_u32  u32PlayRate;  /* play rate*/
    mt_u32  u32CurID;     /* current insert or drop position in a FRC cycle*/
	mt_u32  u32InputCount; /* input counter */
}AVPLAY_ALG_FRC_S;

typedef struct mtAVPLAY_VIDFRM_STAT_S
{
    mt_u32      SendNum;
    mt_u32      PlayNum;
    mt_u32      RepeatNum;
    mt_u32      DiscardNum;
}AVPLAY_VIDFRM_STAT_S;

typedef struct mtAVPLAY_DEBUG_INFO_S
{
    mt_u32                     AcquireAudEsNum;
    mt_u32                     AcquiredAudEsNum;
    mt_u32                     SendAudEsNum;
    mt_u32                     SendedAudEsNum;

    mt_u32                     AcquireAudFrameNum;
    mt_u32                     AcquiredAudFrameNum;
    mt_u32                     SendAudFrameNum;
    mt_u32                     SendedAudFrameNum;

    mt_u32                     AcquireVidFrameNum;
    mt_u32                     AcquiredVidFrameNum;
    mt_u32                     LastAcquiredVidFrameNum;
    mt_u32                     NoNewFrameCnt;
    AVPLAY_VIDFRM_STAT_S       MasterVidStat;
    AVPLAY_VIDFRM_STAT_S       SlaveVidStat[AVPLAY_MAX_SLAVE_FRMCHAN];
    AVPLAY_VIDFRM_STAT_S       VirVidStat[AVPLAY_MAX_VIR_FRMCHAN];

    mt_u32                     VidOverflowNum;
    mt_u32                     AudOverflowNum;
    mt_u32                     VidUnderflowNum;
    mt_u32                     AudUnderflowNum;

    mt_u32                     ThreadBeginTime;
    mt_u32                     ThreadEndTime;
    mt_u32                     ThreadScheTimeOutCnt;
    mt_u32                     ThreadExeTimeOutCnt;
    mt_u32                     CpuFreqScheTimeCnt;
	mt_u32					   FirstVidShowd;
}AVPLAY_DEBUG_INFO_S;

typedef struct mtAVPLAY_DEBUG_INFO_EXT_S
{
	mt_u32                     u32AudCrcBufSize;
	ulong                     *pu32AudCrcBuf;
	mt_u32                     u32AudCrcBufWrPtr;
}AVPLAY_DEBUG_INFO_EXT_S;

typedef enum mtTHREAD_PRIO_E
{
    THREAD_PRIO_REALTIME,    /*Realtime thread, only 1 permitted*/
    THREAD_PRIO_HIGH,
    THREAD_PRIO_MID,
    THREAD_PRIO_LOW,
    THREAD_PRIO_BUTT
}THREAD_PRIO_E;

typedef enum mtTHREAD_NICE_E
{
    THREAD_NICE_HIGH = -20,
    THREAD_NICE_MID = 0,
    THREAD_NICE_LOW = 10,
    THREAD_NICE_BUTT
}THREAD_NICE_E;

#pragma pack(4)
typedef struct mtDMX_CHAN_INFO_S
{
  ulong dmx_buf_addr;
  ulong dmx_buf_phy_addr;
  ulong dmx_buf_len;
  mt_u32 dmx_buf_read_pnt;
}DMX_CHAN_INFO_S;

typedef struct mtAVPLAY_S
{
    MT_UNF_AVPLAY_ATTR_S            AvplayAttr;
    MT_UNF_VCODEC_ATTR_S            VdecAttr;
    MT_UNF_AVPLAY_LOW_DELAY_ATTR_S  LowDelayAttr;
    mt_u32                          AdecType;

    mt_handle                       hAvplay;
    mt_handle                       hVdec;
    mt_handle                       hAdec;
    #ifdef CONFIG_MT_AUDIO_AD
    mt_handle                       hAdec_AD;
    mt_handle                       hDmxAud_AD;
    DMX_CHAN_INFO_S                 DmxChanInfoAud_AD;
    mt_u32                          adectype_AD;
    #endif
    mt_handle                       hDmxVid;
    mt_handle                       hDmxAud[AVPLAY_MAX_DMX_AUD_CHAN_NUM];
    DMX_CHAN_INFO_S                 DmxChanInfoAud[AVPLAY_MAX_DMX_AUD_CHAN_NUM];
    mt_handle                       hDmxPcr;
    mt_handle                       hSync;

    mt_u32                          DmxVidPid;
    mt_u32                          DmxPcrPid;
    mt_u32                          DmxAudPid[AVPLAY_MAX_DMX_AUD_CHAN_NUM];

    /*multi audio demux channel*/
    mt_u32                          CurDmxAudChn;
    mt_u32                          DmxAudChnNum;
    MT_UNF_ACODEC_ATTR_S            *pstAcodecAttr;

    mt_handle                       hSharedOrgWin;  /*Original window of homologous*/

    /*multi video frame channel*/
    AVPLAY_VID_PORT_AND_WIN_S       MasterFrmChn;
    AVPLAY_VID_PORT_AND_WIN_S       SlaveFrmChn[AVPLAY_MAX_SLAVE_FRMCHAN];
    mt_u32                          SlaveChnNum;
    AVPLAY_VID_PORT_AND_WIN_S       VirFrmChn[AVPLAY_MAX_VIR_FRMCHAN];
    mt_u32                          VirChnNum;

    /*multi audio track channel*/
    mt_handle                       hSyncTrack;
    mt_handle                       hTrack[AVPLAY_MAX_TRACK];
    mt_u32                          TrackNum;

    /*frc parameters*/
    MT_BOOL                         bFrcEnable;
    AVPLAY_FRC_CFG_S                FrcParamCfg;        /* config frc param */ /*CNcomment: 配置的frc参数 */
    AVPLAY_ALG_FRC_S                FrcCalAlg;          /* frc used rate info */ /*CNcomment: frc正在使用的帧率信息 */
    AVPLAY_FRC_CTRL_S               FrcCtrlInfo;        /* frc control */ /*CNcomment: frc控制信息 */
    mt_u32                          FrcNeedPlayCnt;     /* this frame need to play time*/ /*CNcomment:该帧需要播几次 */
    mt_u32                          FrcCurPlayCnt;      /* this frame had played time*/   /*CNcomment:该帧实际播到第几次*/

    /*flush stream control*/
    MT_BOOL                         bSetEosFlag;
    MT_BOOL                         bSetAudEos;
    
    /* Eos event */
    MT_BOOL                         bVideoLastFrameDecoded;

    /*ddp test*/
    MT_BOOL                         AudDDPMode;
    //mt_u32                          LastAudPts;
    mt_u64                          LastAudPts;

    AVPLAY_EVT_CB_FN                EvtCbFunc[MT_UNF_AVPLAY_EVENT_BUTT];

    /*play control parameters*/
    MT_BOOL                         bSendedFrmToVirWin;          /*whether this frame has send to virtual window*/
    MT_BOOL                         VidEnable;
    MT_BOOL                         AudEnable;
    MT_BOOL                         bVidPreEnable;
    MT_BOOL                         bAudPreEnable;
	  MT_BOOL                         VidPreBufThreshhold;
    MT_BOOL                         AudPreBufThreshhold;
	  mt_u32							            VidPreSysTime;
	  mt_u32							            AudPreSysTime;
    MT_UNF_AVPLAY_STATUS_E          LstStatus;                   /* last avplay status */
    MT_UNF_AVPLAY_STATUS_E          CurStatus;                   /* current avplay status */
    MT_UNF_AVPLAY_OVERFLOW_E        OverflowProc;
    MT_BOOL                         AvplayDataPushPause;         /* flag for push av data */
    MT_BOOL                         AvplayProcContinue;          /*flag for thread continue*/
    MT_BOOL                         AvplayVidProcContinue;       /*flag for video thread continue*/

    MT_BOOL                         AvplayProcDataFlag[AVPLAY_PROC_BUTT];

    MT_UNF_STREAM_BUF_S             AvplayAudEsBuf;      /*adec buffer in es mode*/
    MT_UNF_ES_BUF_S                 AvplayDmxEsBuf;      /*audio denux buffer in ts mode*/
#if defined(CONFIG_MT_CHIP_SYMPHONY4) ||  defined(CONFIG_MT_CHIP_SYMPHONY6)
	void							*AudPtsDesc[2];		 /* audio pts descriptor from demux */
#endif
    MT_UNF_AO_FRAMEINFO_S           AvplayAudFrm;        /*audio frames get form adec*/
    MT_UNF_AO_FRAMEINFO_S           AvplayAudFrm2;        /*audio frames get form adec*/
#if defined(CONFIG_MT_CHIP_SYMPHONY4) ||  defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_UNF_AO_FRAMEINFO_S           AvplayAudFrm3;        /*audio frames get form adec*/
#endif	
    SYNC_AUD_INFO_S                 AudInfo;
    SYNC_AUD_OPT_S                  AudOpt;

    VDEC_ES_BUF_S                   AvplayVidEsBuf;      /*vdec buffer in es mode*/
    MT_DRV_VIDEO_FRAME_PACKAGE_S    CurFrmPack;
    MT_DRV_VIDEO_FRAME_PACKAGE_S    LstFrmPack;
    SYNC_VID_INFO_S                 VidInfo;
    SYNC_VID_OPT_S                  VidOpt;

    MT_DRV_VDEC_FRAME_S             stIFrame;

    MT_BOOL                         bStepMode;
    MT_BOOL                         bStepPlay;
    mt_u64                          TargetTime;
    MT_BOOL                         bTargetVideoRendered;
    MT_BOOL                         bTargetAudioRendered;

    AVPLAY_DEBUG_INFO_S             DebugInfo;
    AVPLAY_DEBUG_INFO_EXT_S         DebugInfoExt;

    mt_u32                          PreAudEsBuf;         /*audio es buffer size when EOS happens*/
    mt_u32                          PreVidEsBuf;         /*video es buffer size when EOS happens*/
    mt_u32                          PreSystime;          /*system time when EOS happens*/
    mt_u32                          PreVidEsBufWPtr;     /*position of the video es buffer write pointer*/
    mt_u32                          PreAudEsBufWPtr;     /*position of the audio es buffer write pointer*/
    mt_u32                          PreTscnt;            /*ts count when EOS happens*/
    MT_BOOL                         CurBufferEmptyState; /*current buffer state is empty or not*/

    MT_UNF_AVPLAY_BUF_STATE_E       PreVidBufState;     /*the status of video es buffer when CheckBuf*/
    MT_UNF_AVPLAY_BUF_STATE_E       PreAudBufState;     /*the status of audio es buffer when CheckBuf*/
    MT_BOOL                         VidDiscard;

    mt_u32                          EosStartTime;        /*EOS start time*/
    mt_u32                          EosDurationTime;     /*EOS duration time*/

    MT_BOOL                         bStandBy;            /*is standby or not*/

    //mt_u32                          AdecDelayMs;            /*How many mseconds in ADEC buffer*/
    mt_u64                          AdecDelayMs;            /*How many mseconds in ADEC buffer*/
    ADEC_SzNameINFO_S               AdecNameInfo;

    mt_u32                          u32DispOptimizeFlag;    /*this is for pvr smooth tplay*/

    mt_s32                          ThreadID;
    MT_BOOL                         AvplayThreadRun;
    THREAD_PRIO_E                   AvplayThreadPrio;    /*the priority level of avplay thread*/

/* Bug: for size of this structure, kernel not match with msp api.
 * it's better move to tail.
 */
#if 0
#ifndef __KERNEL__
    pthread_t                       AvplayDataThdInst;  /* run handle of avplay thread */
    pthread_t                       AvplayVidDataThdInst;  /* run handle of avplay thread */
    pthread_attr_t                  AvplayThreadAttr;   /*attribute of avplay thread*/
    pthread_mutex_t                 *pAvplayThreadMutex;     /*mutex for data safety use*/
    pthread_mutex_t                 *pAvplayVidThreadMutex;     /*mutex for data safety use*/
    pthread_mutex_t                 *pAvplayMutex;            /* mutex for interface safety use */

    pthread_t                       AvplayStatThdInst;    /* run handle of avplay thread */
#endif
#endif

    mt_u32                          u32AoUnloadTime;
    mt_u32                          u32WinUnloadTime;
    mt_u32                          u32ThreadScheTimeOutCnt;
    mt_u8                           ves_buffer_channel_id;
    ulong                          u32VidBufUsrVirAddr;
    ulong                          u32VidBufKerVirAddr;
    ulong                          u32VidBufSize;
    ulong                          u32VidBufKerVirDescAddr;
    mt_u32                          u32imgChangeFlag;
    ulong                          u32DescBuffSize;
    mt_u32                          u32PreFrameFinsh;
    mt_u32                          u32ScrapSize;

	mt_u32 							DmxAVSyncFlag;

/* Bug: for size of this structure, kernel not match with msp api.
 * move to tail, and add some reserved fileds.
 */
#ifndef __KERNEL__
    pthread_t                       AvplayDataThdInst;  /* run handle of avplay thread */
    pthread_t                       AvplayVidDataThdInst;  /* run handle of avplay thread */
    pthread_attr_t                  AvplayThreadAttr;   /*attribute of avplay thread*/
    pthread_mutex_t                 *pAvplayThreadMutex;     /*mutex for data safety use*/
    pthread_mutex_t                 *pAvplayVidThreadMutex;     /*mutex for data safety use*/
    pthread_mutex_t                 *pAvplayMutex;            /* mutex for interface safety use */

    pthread_t                       AvplayStatThdInst;    /* run handle of avplay thread */
	sem_t							*m_vidsem;
#else
	/*
	 3*(sizeof(pthread_t)=4)
	 + (sizeof(pthread_attr_t)=36)
	 + 3*(sizeof(pthread_mutex_t)=24)
	*/
	mt_u32 reserved[(3*4+36+3*24)/4];
	mt_u32 *m_vidsem;
#endif
    MT_UNF_SYNC_AV_INFO_S   AVSyncInfo;
    mt_u32                  PreDmxAudChn;
	DISP_TRICK_MODE_E 		trickmode;
	mt_u32					u32SpeedDecimal;
	mt_u64 					LastPlayFrameMs;
	mt_u32					enable_sw_adec;
	mt_u32					vdec_pip_chan;
}AVPLAY_S;
#pragma pack()

typedef struct mtAVPLAY_CREATE_S
{
    mt_u32     AvplayId;
    mt_u32     AvplayPhyAddr;
    MT_UNF_AVPLAY_STREAM_TYPE_E    AvplayStreamtype;
}AVPLAY_CREATE_S;

typedef struct mtAVPLAY_USR_ADDR_S
{
    mt_u32     AvplayId;
    ulong     AvplayUsrAddr;    /* AVPLAY address in user model */
}AVPLAY_USR_ADDR_S;

typedef struct mtAVPLAY_INFO_S
{
    AVPLAY_S   *pAvplay;         /* AVPLAY pointer in kernel model */
    phys_addr_t     AvplayPhyAddr;    /* AVPLAY physical address */
    ulong     File;             /*avplay file handle*//* CNcomment: AVPLAY所在进程句柄 */
    ulong     AvplayUsrAddr;    /* AVPLAY address in user model */
    MT_UNF_AVPLAY_STREAM_TYPE_E    AvplayStreamtype;	/* AVPlay Stream Type */
}AVPLAY_INFO_S;

typedef struct mtAVPLAY_GLOBAL_STATE_S
{
    AVPLAY_INFO_S  AvplayInfo[AVPLAY_MAX_NUM];
    mt_u32         AvplayCount;
}AVPLAY_GLOBAL_STATE_S;


typedef enum mtIOC_AVPLAY_E
{
    IOC_AVPLAY_GET_NUM = 0,

    IOC_AVPLAY_CREATE,
    IOC_AVPLAY_DESTROY,

    IOC_AVPLAY_CHECK_ID,
    IOC_AVPLAY_SET_USRADDR,
    IOC_AVPLAY_CHECK_NUM,
    IOC_AVPLAY_SET_CPUFREQ,
	IOC_AVPLAY_GET_PLAYERINFO,
    IOC_AVPLAY_SET_BUTT
}IOC_AVPLAY_E;


#define CMD_AVPLAY_CREATE            _IOWR(MT_ID_AVPLAY, IOC_AVPLAY_CREATE, AVPLAY_CREATE_S)
#define CMD_AVPLAY_DESTROY           _IOW(MT_ID_AVPLAY, IOC_AVPLAY_DESTROY, mt_u32)

#define CMD_AVPLAY_CHECK_ID       _IOWR(MT_ID_AVPLAY, IOC_AVPLAY_CHECK_ID, AVPLAY_USR_ADDR_S)
#define CMD_AVPLAY_SET_USRADDR    _IOW(MT_ID_AVPLAY, IOC_AVPLAY_SET_USRADDR, AVPLAY_USR_ADDR_S)
#define CMD_AVPLAY_CHECK_NUM      _IOWR(MT_ID_AVPLAY, IOC_AVPLAY_CHECK_NUM, mt_u32)
#define CMD_AVPLAY_SET_CPUFREQ    _IO(MT_ID_AVPLAY, IOC_AVPLAY_SET_CPUFREQ)
#define CMD_AVPLAY_GET_PLAYERINFO            _IOWR(MT_ID_AVPLAY, IOC_AVPLAY_GET_PLAYERINFO, MT_UNF_AVPLAY_PLAYERINFO_S)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


