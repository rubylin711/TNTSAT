/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _DRV_SYNC_IOCTL_H_
#define _DRV_SYNC_IOCTL_H_

#include "mt_type.h"
#include "mt_drv_sync.h"
#include "mt_error_mpi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if (MT_PTS_USE_64_US == 0)
#define  SYNC_MAX_NUM                   16

#define  SYS_TIME_MAX                   0xFFFFFFFFU  
#define  PCR_TIME_MAX                   95443718

#define  PTS_SERIES_COUNT               2
#define  PCR_MAX_DELTA                  100

#define  PCR_LEAD_ADJUST_THRESHOLD      200    
#define  PCR_LAG_ADJUST_THRESHOLD       200

#define  PCR_LEAD_STOP_THRESHOLD        100    
#define  PCR_LAG_STOP_THRESHOLD         100

#define  PCR_TIMEOUTMS                  50
#define  AUD_TIMEOUTMS                  200

#define  PRE_SYNC_MIN_TIME              300
#define  BUF_FUND_TIMEOUT               200

#define  VID_LEAD_DISCARD_THRESHOLD     10000
#define  VID_LAG_DISCARD_THRESHOLD      10000

#define  AUD_RESYNC_TIMEOUT             2000  /* Temporarily reduce the timeout value
                                                 of audio resynchronization to 1000ms. 10000 */
#define  AUD_RESYNC_ADJUST_THRESHOLD    10000 /* aud resync adjust threshold */

#define  PCR_DISCARD_THRESHOLD          2000
#define  PCR_ADJUST_THRESHOLD           500

#define  SCR_DISCARD_THRESHOLD          10000

#define VID_PTS_GAP                     300
#define AUD_PTS_GAP                     200

#define VID_SMOOTH_DISCARD_INTERVAL     2     /*discard one frame in every interval frame*/
#define VID_SMOOTH_REPEAT_INTERVAL      2     /*repeat  one frame in every interval frames, including repeated frame*/

#define SYNC_FRAME_VID_EMPTY_BUFNUM     4     /* vo bufnum waterline */
#define SYNC_ES_VID_EMPTY_PERCENT       2     /* vid es buf waterline */

#define SYNC_PTS_JUMP_FRM_NUM           2

#define PTS_LOOPBACK_TIMEOUT            2000
#else
#define  SYNC_MAX_NUM                   16

#define  SYS_TIME_MAX                   0xFFFFFFFFFFFFFFFFULL  
#define  PCR_TIME_MAX                   (95443718ULL * 1000)

#define  PTS_SERIES_COUNT               2
#define  PCR_MAX_DELTA                  (100*1000)

#define  PCR_LEAD_ADJUST_THRESHOLD      (200*1000)    
#define  PCR_LAG_ADJUST_THRESHOLD       (200*1000)

#define  PCR_LEAD_STOP_THRESHOLD        (100*1000)    
#define  PCR_LAG_STOP_THRESHOLD         (100*1000)

#define  PCR_TIMEOUTMS                  (50*1000)
#define  AUD_TIMEOUTMS                  (200*1000)

#define  PRE_SYNC_MIN_TIME              (300*1000)
#define  BUF_FUND_TIMEOUT               (200*1000)

#define  VID_LEAD_DISCARD_THRESHOLD     (10000*1000)
#define  VID_LAG_DISCARD_THRESHOLD      (10000*1000)

#define  AUD_RESYNC_TIMEOUT             (2000*1000)  /* Temporarily reduce the timeout value
                                                 of audio resynchronization to 1000ms. 10000 */
#define  AUD_RESYNC_ADJUST_THRESHOLD    (10000*1000) /* aud resync adjust threshold */

#define  PCR_DISCARD_THRESHOLD          (2000*1000)
#define  PCR_ADJUST_THRESHOLD           (500*1000)

#define  SCR_DISCARD_THRESHOLD          (10000*1000)

#define VID_PTS_GAP                     (300*1000)
#define AUD_PTS_GAP                     (200*1000)

#define VID_SMOOTH_DISCARD_INTERVAL     2     /*discard one frame in every interval frame*/
#define VID_SMOOTH_REPEAT_INTERVAL      2     /*repeat  one frame in every interval frames, including repeated frame*/

#define SYNC_FRAME_VID_EMPTY_BUFNUM     4     /* vo bufnum waterline */
#define SYNC_ES_VID_EMPTY_PERCENT       2     /* vid es buf waterline */

#define SYNC_PTS_JUMP_FRM_NUM           2

#define PTS_LOOPBACK_TIMEOUT            (2000*1000)

#endif

typedef enum mtSYNC_PCR_ADJUST_E
{
    SYNC_PCR_ADJUST_SCR,
    SYNC_AUD_ADJUST_SCR, 

    SYNC_SCR_ADJUST_BUTT
}SYNC_PCR_ADJUST_E;

typedef struct mtSYNC_PCR_INFO_S
{
    MT_BOOL                    PcrFirstCome;         /* arrive flag of the first pcr */
    MT_BOOL                    PcrAudSyncOK;
    MT_BOOL                    PcrVidSyncOK;
    MT_BOOL                    PcrAdjustDeltaOK;
    mt_upts                     PcrFirstSysTime;      /* arrive time of the first pcr */
    mt_u32                     PcrFirst;             /* first pcr value */
    mt_u32                     PcrLast;              /* last  pcr value*/
    mt_upts                     PcrLastSysTime;       /* last system time of setting pcr localtime*/
    mt_upts                     PcrLastLocalTime;     /* last pcr localtime */
    mt_upts                     PcrPauseLocalTime;    /* pcr localtime when pause */
    mt_u32                     PcrSeriesCnt;         /* pcr successive count*/
    mt_upts                     PcrSyncStartSysTime;  /* start time of pcr synchronization,set when sync start */
    MT_BOOL                    PcrLocalTimeFlag;     /* valid flag of pcr localtime */
    MT_BOOL                    PcrAdjustDeltaFlag;
    SYNC_PCR_ADJUST_E          enPcrAdjust;          /*which way to adjust scr*/
    mt_spts                     PcrDelta;             /*adjust pcr value*/

    mt_spts                     AudPcrDiff;           /* difference value between audio localtime and pcr localtime */
    mt_spts                     VidPcrDiff;           /* difference value between video localtime and pcr localtime */
    mt_spts                     LastVidPcrDiff;
    mt_spts                     LastAudPcrDiff;

    MT_BOOL                    PcrLoopBack;
    mt_upts                     PcrGradient;         /*gradient of pcr and system time*//**<CNcomment: PCR和系统时间比例斜率(*100)*/
    
}SYNC_PCRINFO_S;

typedef enum mtSYNC_BUF_STATE_E
{
    SYNC_BUF_STATE_EMPTY = 0,   /**<The buffer is idle.*//**<CNcomment: 缓冲区空闲*/
    SYNC_BUF_STATE_LOW,         /**<The buffer usage is too low.*//**<CNcomment: 缓冲区占用率过低*/
    SYNC_BUF_STATE_NORMAL,      /**<The buffer works normally.*//**<CNcomment: 缓冲区使用正常*/
    SYNC_BUF_STATE_HIGH,        /**<The buffer usage is too high.*//**<CNcomment: 缓冲区占用率过高*/
    SYNC_BUF_STATE_FULL,        /**<The buffer is full.*//**<CNcomment: 缓冲区已满*/

    SYNC_BUF_STATE_BUTT
}SYNC_BUF_STATE_E;

typedef struct mtSYNC_BUF_STATUS_S
{
    mt_u32    VidBufPercent;
    mt_u32    AudBufPercent;
    SYNC_BUF_STATE_E VidBufState;
    SYNC_BUF_STATE_E AudBufState;
    MT_BOOL   bOverflowDiscFrm;

}SYNC_BUF_STATUS_S;

/* sync event struct */
typedef struct tagSYNC_EVENT_S
{
    MT_BOOL                         bVidPtsJump;
    MT_BOOL                         bAudPtsJump;

    MT_BOOL                         bStatChange;
    MT_BOOL                         bEos_back;

    MT_UNF_SYNC_PTSJUMP_PARAM_S     VidPtsJumpParam;
    MT_UNF_SYNC_PTSJUMP_PARAM_S     AudPtsJumpParam;
    
    MT_UNF_SYNC_STAT_PARAM_S        StatParam;
    
}SYNC_EVENT_S;

typedef enum tagSYNC_EXT_INFO_E
{
    SYNC_EXT_INFO_FIRST_PTS,     /*mt_u32*/
    SYNC_EXT_INFO_SECOND_PTS,    /*mt_u32*/

    SYNC_EXT_INFO_STOP_REGION,   /*MT_BOOL*/
    
    SYNC_EXT_INFO_BUTT
}SYNC_EXT_INFO_E;

/* sync region status enum */
typedef enum tagSYNC_REGION_STAT_E
{
    SYNC_REGION_STAT_IN_STOP,
    SYNC_REGION_STAT_IN_START,
    SYNC_REGION_STAT_IN_NOVEL,
    SYNC_REGION_STAT_IN_DISCARD,
    SYNC_REGION_STAT_OUT_DISCARD,
    
    SYNC_REGION_STAT_BUTT
}SYNC_REGION_STAT_E;

typedef struct mtSYNC_S
{
    MT_UNF_SYNC_ATTR_S         SyncAttr;             /* sync attributes set by user*/

    MT_BOOL 				   VidEnable;            /* video enable flag, set when start video*/
	MT_BOOL 				   AudEnable;            /* audio enable flag, set when start audio*/
    MT_BOOL                    AudDDPMode;           /* for DDP test only */
    
	/* AV common information. reset only when both of av are stoped */
    SYNC_STATUS_E              CrtStatus;            /* current status */

    SYNC_BUF_STATUS_S          CrtBufStatus;         /* current buf status*/

    SYNC_EVENT_S               SyncEvent;
    mt_upts                     LoopBackTime;
    MT_BOOL                    LoopBackFlag;

    mt_upts                     VidFirstDecPts;
    mt_upts                     VidSecondDecPts;
        
    mt_upts                     PreSyncStartSysTime;  /* start time of presynchronization,set when presync start */
    mt_upts                     PreSyncEndSysTime;    /* end time of presynchronization,set when presync end */
    MT_BOOL                    PreSyncFinish;        /* finish flag of presynchronization */
    SYNC_CHAN_E                PreSyncTarget;        /* presynchronization target*/
    mt_upts                     PreSyncTargetTime;    /* target time of presynchronization */
    MT_BOOL                    PreSyncTargetInit;    /* initialize flag of presynchronization target */
    MT_BOOL                    BufFundFinish;        /* finish flag of audio and video data cumulation */
    mt_upts                     BufFundEndSysTime;    /* end time of audio and video data cumulation  */
    mt_upts                     ExtPreSyncTagetTime;
    MT_BOOL                    UseExtPreSyncTaget;

    SYNC_PCRINFO_S             PcrSyncInfo;          /*some information while sync reference set MT_UNF_SYNC_REF_PCR */
    
    /* video statistics, reset when stop*/
    MT_BOOL                    VidFirstCome;         /* arrive flag of the first video frame */ 
    mt_upts                     VidFirstSysTime;      /* arrive time of the first video frame */
    mt_upts                     VidFirstPts;          /* PTS of the first video frame */
    MT_BOOL                    VidFirstValidCome;
    mt_upts                     VidFirstValidPts;
    mt_upts                     VidLastPts;           /* PTS of the last video frame*/
    mt_upts                     VidLastSrcPts;
    MT_BOOL                    VidPreSyncTargetInit; /* initialize flag of video presync target */
    mt_upts                     VidPreSyncTargetTime; /* video presync target time */
    MT_BOOL                    VidLocalTimeFlag;     /* valid flag of video localtime */
    mt_upts                     VidLastSysTime;       /* last system time of setting video localtime*/
    mt_upts                     VidLastLocalTime;     /* last video localtime */
    mt_upts                     VidPauseLocalTime;    /* video localtime when pause */
    mt_u32                     VidPtsSeriesCnt;      /* video pts successive count */
    MT_BOOL                    VidSyndAdjust;        /* adjust flag */
    // TODO: x57522  the usage of the  following three members
    mt_u32                     VidDisPlayCnt;        /* video play count when discard*/
    mt_u32                     VidDiscardCnt;        /* video discard count */
    mt_u32                     VidRepPlayCnt;        /* video play count when repeat*/
    mt_u32                     VidRepeatCnt;         /* video repeat count */
    SYNC_VID_INFO_S            VidInfo;              /* video channel information,set by vo and used by sync */
    SYNC_VID_OPT_S             VidOpt;               /* video adjust mode,set by sync */
    MT_BOOL                    VidFirstPlay;         /* played flag of the first video frame */
    mt_upts                     VidFirstPlayTime;     /* time of playing the first video frame */
    mt_spts                     VidAudDiff;           /* difference value between video localtime and audio localtime */
    mt_spts                     LastVidAudDiff;
    MT_BOOL                    VidPtsLoopBack;
    
    /* audio statistics,reset when stop */
    MT_BOOL                    AudFirstCome;         /* arrive flag of the first audio frame */
    mt_upts                     AudFirstSysTime;      /* arrive time of the first audio frame */
    mt_upts                     AudFirstPts;          /* pts of the first audio frame */
    MT_BOOL                    AudFirstValidCome;
    mt_upts                     AudFirstValidPts;
    mt_upts                     AudLastPts;           /* pts of the last audio frame */
    mt_upts                     AudLastSrcPts;
	mt_upts                     AudLastBufTime;       /* buftime value when last audio pts arrived */
    MT_BOOL                    AudPreSyncTargetInit; /* initialize flag of audio presync target */
    mt_upts                     AudPreSyncTargetTime; /* audio presync target time */
    MT_BOOL                    AudLocalTimeFlag;     /* valid flag of audio localtime */
    mt_upts                     AudLastSysTime;       /* last system time of setting audio localtime */
    mt_upts                     AudLastLocalTime;     /* last audio localtime */
    mt_upts                     AudPauseLocalTime;    /* audio localtime when pause */
    mt_u32                     AudPtsSeriesCnt;      /* audio pts successive count*/
    MT_BOOL                    AudReSync;            /* audio resync flag */
    MT_BOOL                    AudReBufFund;         /* audio recumulate flag */
    mt_u32                     AudPlayCnt;           /* audio play count */
    mt_u32                     AudRepeatCnt;         /* audio repeat count */
    mt_u32                     AudDiscardCnt;        /* audio discard count */
    SYNC_AUD_INFO_S            AudInfo;              /* audio channel information,set by adec and used by sync */
    SYNC_AUD_OPT_S             AudOpt;               /* audio adjust mode,set by sync */
    MT_BOOL                    AudFirstPlay;         /* played flag of the first audio frame */
    mt_upts                     AudFirstPlayTime;     /* time of playing the first audio frame */
    MT_BOOL                    AudPtsLoopBack;
    
    mt_upts                     ScrFirstSysTime;      /* last scr system time */
    mt_upts                     ScrFirstLocalTime;    /* last scr local time */
    mt_upts                     ScrLastSysTime;       /* last scr system time */
    mt_upts                     ScrLastLocalTime;     /* last scr local time */
    mt_upts                     ScrPauseLocalTime;    /* scr localtime when pause */
    mt_spts                     AudScrDiff;           /* difference time between audio localtime and scr localtime */
    mt_spts                     VidScrDiff;           /* difference time between vidio localtime and scr localtime */
    MT_BOOL                    ScrInitFlag;          /* scr be inited or not */
    
    MT_BOOL                    bPrint;

    ulong                     bUseStopRegion;       /*use stop region or not*/
    
	// Debug AV Render
	// 0: none
	// 1: audio render play
	// 2: audio render pause
	// 3: audio render discard
	// 4: video render play
	// 5: video render repeat
	// 6: video render discard
	mt_u32						DebugRender;
	mt_u32						DebugParam;			  /* extended debug parameter */

#ifndef __KERNEL__
    pthread_mutex_t           *pSyncMutex;            /* mutex used to protect sync interface */
#endif

}SYNC_S;

typedef struct mtSYNC_CREATE_S
{
    mt_u32     SyncId;
    phys_addr_t     SyncPhyAddr;
}SYNC_CREATE_S;

typedef struct mtSYNC_USR_ADDR_S
{
    mt_u32     SyncId;
    ulong     SyncUsrAddr;    /* SYNC user space address */ 
}SYNC_USR_ADDR_S;

typedef struct mtSYNC_AUD_JUDGE_S
{
    mt_handle         hSync;
    SYNC_AUD_INFO_S   AudInfo;
    SYNC_AUD_OPT_S    AudOpt;
}SYNC_AUD_JUDGE_S;

typedef struct mtSYNC_VID_JUDGE_S
{
    mt_handle           hSync;
    SYNC_VID_INFO_S     VidInfo;
    SYNC_VID_OPT_S      VidOpt;
}SYNC_VID_JUDGE_S;

typedef struct mtSYNC_GET_TIME_S
{
    mt_u32             SyncId;
    mt_upts             LocalTime;
    mt_upts             PlayTime;
}SYNC_GET_TIME_S;


typedef struct mtSYNC_GET_TIME_INFO_S
{
    mt_u32              SyncId;
    mt_upts             LocalTime;
    mt_upts             PlayTime;
    
    mt_upts             AudFirstPts; 
    mt_upts             VidFirstPts; 
    mt_upts             AudLastPts;  
    mt_upts             VidLastPts;

    mt_s64              VidAudDiff; 
     
}SYNC_GET_TIME_INFO_S;


typedef struct mtSYNC_SET_AVSYNC_MODE_S
{
    mt_u32              SyncId;
    MT_UNF_SYNC_REF_E   sync_ref_mode;

}SYNC_SET_AVSYNC_MODE_S;

typedef struct mtSYNC_PUSH_APTS_S
{
    mt_upts             pts;
    MT_BOOL             valid;
    mt_u32              id;
    mt_upts             step;
    mt_dvb_pts32        pts_u32;
    mt_dvb_pts32        step_u32;
}SYNC_PUSH_APTS_S;

typedef struct mtSYNC_PUSH_VPTS_S
{
    mt_upts              pts;
    MT_BOOL            valid;
    mt_u32              id;
    mt_upts             step;

    mt_u32              pts_u32;
    mt_u32              step_u32;
}SYNC_PUSH_VPTS_S;


typedef struct mtSYNC_INFO_S
{
    SYNC_S     *pSync;           /* SYNC kernel space pointer */
    ulong     SyncPhyAddr;      /* SYNC physical address*/ 
    ulong     File;             /* SYNC process handle */
    ulong     SyncUsrAddr;      /* SYNC usr space address*/
}SYNC_INFO_S;

typedef  mt_s32 ( *SyncManage)(mt_handle *hSyncID);

typedef struct mtSYNC_GLOBAL_STATE_S
{
    SYNC_INFO_S   SyncInfo[SYNC_MAX_NUM];
    mt_u32        SyncCount;
	SyncManage    AddSyncIns;
	SyncManage    DelSyncIns;	
}SYNC_GLOBAL_STATE_S;

typedef struct mtSYNC_GET_AVSYNC_INFO_S
{
    mt_u32    cur_apts;
    mt_u32    cur_vpts;
     
}SYNC_GET_AVSYNC_INFO_S;

typedef enum mtSYNC_AVSYNC_EVENT_E
{
    SYNC_AVSYNC_EVENT_APTS_JUMP,
    SYNC_AVSYNC_EVENT_VPTS_JUMP,
    SYNC_AVSYNC_EVENT_STA_CHANGE,
    SYNC_AVSYNC_EVENT_EOS,
    SYNC_AVSYNC_EVENT_AUD_ROLLBACK,
    SYNC_AVSYNC_EVENT_VID_ROLLBACK,
    SYNC_AVSYNC_EVENT_BUTT,

}SYNC_AVSYNC_EVENT_E;


typedef enum mtIOC_SYNC_E
{
    IOC_SYNC_GET_NUM = 0,
    IOC_SYNC_CREATE,
    IOC_SYNC_DESTROY,
    IOC_SYNC_CHECK_ID,
    IOC_SYNC_SET_USRADDR,
    IOC_SYNC_CHECK_NUM,
    IOC_SYNC_START_SYNC,
    IOC_SYNC_AUD_JUDGE,
    IOC_SYNC_VID_JUDGE,
    IOC_SYNC_PAUSE_SYNC,
    IOC_SYNC_RESUME_SYNC,
    IOC_SYNC_GET_TIME,
    IOC_SYNC_GET_TIME_INFO,
    IOC_SYNC_PUSH_VPTS,
    IOC_SYNC_PUSH_APTS,
    IOC_SYNC_INIT_AUD,
    IOC_SYNC_INIT_VID,
    IOC_SYNC_DEINIT_AUD,
    IOC_SYNC_DEINIT_VID,
    IOC_SYNC_ADEC_PTS,
    IOC_SYNC_APTS_ADJUST,
    IOC_SYNC_GET_VFRM_CNT,
    IOC_SYNC_AUD_TRACK,
    IOC_SYNC_GET_AVSYNC_INFO,
	IOC_SYNC_VFRM_INFO_CAP_ENABLE,
    IOC_SYNC_VFRM_INFO_CAP_READ,
    IOC_SYNC_CFG_PLAY_INFO,
    IOC_SYNC_SET_AVSYNC_REF_MODE,
    IOC_SYNC_SET_AVSYNC_DATA_SOURCE,
    IOC_SYNC_SET_BUTT
}IOC_SYNC_E;

typedef struct mtSYNC_PLAY_INFO_S {
    mt_u32 a_pid;
    mt_u32 v_pid;
    mt_u32 a_type;
    mt_u32 v_type;
    mt_u32 tvformat;
	mt_u32 is_hdmi_connected;
}SYNC_PLAY_INFO_S;

typedef enum mtSYNC_DATA_SOURCE_E
{
    SYNC_DATA_SOURCE_TUNER,
    SYNC_DATA_SOURCE_FILE,
    SYNC_DATA_SOURCE_PVR,
    SYNC_DATA_SOURCE_NET,
    SYNC_DATA_SOURCE_BUTT,
} MT_SYNC_DATA_SOURCE_E;

struct  avsync_async_info_t
{
    mt_u32 apts;                      //!< PTS of audio frame.
    mt_u32 apts_id;                   //!< Id of audio frame.
    mt_u32 apts_step;                 //!< Audio frame duration, the same unit with pts.
    mt_u32 inbuf_es_size;             //!< Remained audio ES data size.
    mt_u32 inbuf_size;                //!< Audio ES inbuffer size.
    mt_u32 ao_data_time;              //!< Play time in AO buffer, the same unit with pts.
    mt_u32 ao_data_size;              //!< AO buffer data size.
    mt_u32 pcm_temp_time;             //!< pcm data time, the same unit with pts.
    mt_u8  dolby_type;				  //1:dd, 2:ddp
    MT_BOOL audio_bypass;			  //1:audio bypass, 0:pcm
};

struct  avsync_vsync_info_t
{
    mt_u32 vpts;                      //!< PTS of video frame.
    mt_u32 vpts_id;                   //!< PTS of video frame.
    mt_u32 vpts_step;                 //!< video frame duration.
    mt_u32 remain_es_size;            //!< Remained video ES data size.
    mt_u32 esbuf_size;                //!< video ES buffer size.
    mt_u32 end_of_stream_flag;        //!< End of stream.
    mt_u32 dolby_avsync;              //!< Dolby avsync stream flag.
    mt_u32 trick_state;               //!< trick mode.
    mt_u32 trick_speed;               //!< trick speed.
    mt_u32 screen_mode;               //!< screen on mode.
};

typedef enum
{
	AVSYNC_FRAME_PLAY = 0,	          //!< Play  frame
	AVSYNC_FRAME_SKIP,                //!< Skip  frame
	AVSYNC_FRAME_PAUSE,               //!< Pause frmae
	AVSYNC_FRAME_FREE,                //!< Free  frame
	AVSYNC_FRAME_SKIP_AOUT,			 //!< Skip  all ao frame
}AVSYNC_FRAME_SYNCFLAG_E;

#define CMD_SYNC_CREATE  	                   _IOWR(MT_ID_SYNC, IOC_SYNC_CREATE, SYNC_CREATE_S)
#define CMD_SYNC_DESTROY 	                   _IOW(MT_ID_SYNC, IOC_SYNC_DESTROY, mt_u32)
#define CMD_SYNC_CHECK_ID                      _IOWR(MT_ID_SYNC, IOC_SYNC_CHECK_ID, SYNC_USR_ADDR_S)
#define CMD_SYNC_SET_USRADDR                   _IOW(MT_ID_SYNC, IOC_SYNC_SET_USRADDR, SYNC_USR_ADDR_S)
#define CMD_SYNC_CHECK_NUM                     _IOWR(MT_ID_SYNC, IOC_SYNC_CHECK_NUM, mt_u32)
#define CMD_SYNC_START_SYNC                    _IOW(MT_ID_SYNC, IOC_SYNC_START_SYNC, mt_u32)
#define CMD_SYNC_AUD_JUDGE                     _IOWR(MT_ID_SYNC, IOC_SYNC_AUD_JUDGE, SYNC_AUD_JUDGE_S)
#define CMD_SYNC_VID_JUDGE                     _IOWR(MT_ID_SYNC, IOC_SYNC_VID_JUDGE, SYNC_VID_JUDGE_S)
#define CMD_SYNC_PAUSE_SYNC                    _IOW(MT_ID_SYNC, IOC_SYNC_PAUSE_SYNC, mt_u32)
#define CMD_SYNC_RESUME_SYNC                   _IOW(MT_ID_SYNC, IOC_SYNC_RESUME_SYNC, mt_u32)
#define CMD_SYNC_GET_TIME                      _IOWR(MT_ID_SYNC, IOC_SYNC_GET_TIME, SYNC_GET_TIME_S)
#define CMD_SYNC_PUSH_VPTS                     _IOWR(MT_ID_SYNC, IOC_SYNC_PUSH_VPTS, SYNC_PUSH_VPTS_S)
#define CMD_SYNC_PUSH_APTS                     _IOWR(MT_ID_SYNC, IOC_SYNC_PUSH_APTS, SYNC_PUSH_APTS_S)
#define CMD_SYNC_INIT_AUD                      _IOWR(MT_ID_SYNC, IOC_SYNC_INIT_AUD, mt_u32)
#define CMD_SYNC_INIT_VID                      _IOWR(MT_ID_SYNC, IOC_SYNC_INIT_VID, mt_u32)
#define CMD_SYNC_DEINIT_AUD                    _IOW(MT_ID_SYNC, IOC_SYNC_DEINIT_AUD, mt_u32)
#define CMD_SYNC_DEINIT_VID                    _IOW(MT_ID_SYNC, IOC_SYNC_DEINIT_VID, mt_u32)
#define CMD_SYNC_ADEC_PTS                      _IOWR(MT_ID_SYNC, IOC_SYNC_ADEC_PTS, SYNC_PUSH_APTS_S)
#define CMD_SYNC_GET_TIME_INFO                 _IOWR(MT_ID_SYNC, IOC_SYNC_GET_TIME_INFO, SYNC_GET_TIME_INFO_S)
#define CMD_SYNC_APTS_ADJUST 	               _IOW(MT_ID_SYNC, IOC_SYNC_APTS_ADJUST, mt_u32)
#define CMD_SYNC_GET_VFRM_CNT                  _IOWR(MT_ID_SYNC, IOC_SYNC_GET_VFRM_CNT, mt_u32)
#define CMD_SYNC_AUD_TRACK                     _IOW(MT_ID_SYNC, IOC_SYNC_AUD_TRACK, mt_u32)
#define CMD_SYNC_GET_AVSYNC_INFO               _IOWR(MT_ID_SYNC, IOC_SYNC_GET_AVSYNC_INFO, MT_UNF_SYNC_AV_INFO_S)
#define CMD_SYNC_VFRM_INFO_CAP_ENABLE          _IOW(MT_ID_SYNC, IOC_SYNC_VFRM_INFO_CAP_ENABLE, mt_u32)
#define CMD_SYNC_VFRM_INFO_CAP_READ            _IOR(MT_ID_SYNC, IOC_SYNC_VFRM_INFO_CAP_READ, SYNC_VOUT_FRAME_INFO)
#define CMD_SYNC_CFG_PLAY_INFO                 _IOW(MT_ID_SYNC, IOC_SYNC_CFG_PLAY_INFO, SYNC_PLAY_INFO_S)
#define CMD_SYNC_SET_AVSYNC_REF_MODE           _IOWR(MT_ID_SYNC, IOC_SYNC_SET_AVSYNC_REF_MODE, SYNC_SET_AVSYNC_MODE_S)
#define CMD_SYNC_SET_AVSYNC_DATA_SOURCE        _IOWR(MT_ID_SYNC, IOC_SYNC_SET_AVSYNC_DATA_SOURCE, MT_SYNC_DATA_SOURCE_E)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

