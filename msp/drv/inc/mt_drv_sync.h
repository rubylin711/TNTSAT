/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : mt_drv_sync.h
  Version       : Initial Draft
  Author        : montage multimedia software group
  Created       : 2015/11/25
  Description   :
  History       :
  1.Date        : 2015/11/25
    Author      : 
    Modification: Created file
********************************************************************************************/
#ifndef __MT_DRV_SYNC_H__
#define __MT_DRV_SYNC_H__

#include "mt_unf_avplay.h"
#include "mt_error_mpi.h"
#include "mt_drv_video.h"

#ifdef __cplusplus
#if __cplusplus
	extern "C"{
#endif
#endif

#define MT_FATAL_SYNC(fmt...) \
            MT_FATAL_PRINT(MT_ID_SYNC, fmt)

#define MT_ERR_SYNC(fmt...) \
            MT_ERR_PRINT(MT_ID_SYNC, fmt)

#define MT_WARN_SYNC(fmt...) \
            MT_WARN_PRINT(MT_ID_SYNC, fmt)

#define MT_INFO_SYNC(fmt...) \
            MT_INFO_PRINT(MT_ID_SYNC, fmt)

#define MT_ERR_VSYNC(enChn, fmt...)\
        /*lint -save -e* */   \
        if (pSync->bPrint)\
        {\
            if (SYNC_CHAN_VID == enChn)\
            {\
                MT_ERR_PRINT(MT_ID_VSYNC, fmt);\
            }\
        }  \
        /*lint -restore */

#define MT_ERR_ASYNC(enChn, fmt...)\
        /*lint -save -e* */   \
        if (pSync->bPrint)\
        {\
            if (SYNC_CHAN_AUD == enChn)\
            {\
                MT_ERR_PRINT(MT_ID_ASYNC, fmt);\
            }\
        } \
        /*lint -restore*/

#define MT_INFO_VSYNC(enChn, fmt...)\
        /*lint -save -e* */   \
        if (pSync->bPrint)\
        {\
            if (SYNC_CHAN_VID == enChn)\
            {\
                MT_INFO_PRINT(MT_ID_VSYNC, fmt);\
            }\
        }  \
        /*lint -restore */

#define MT_INFO_ASYNC(enChn, fmt...)\
        /*lint -save -e* */   \
        if (pSync->bPrint)\
        {\
            if (SYNC_CHAN_AUD == enChn)\
            {\
                MT_INFO_PRINT(MT_ID_ASYNC, fmt);\
            }\
        } \
        /*lint -restore*/

#define MT_PTS_USE_64_US			1
/*!
  define avsync pts type
*/
#if MT_PTS_USE_64_US
typedef mt_s64 mt_spts;
typedef mt_u64 mt_upts;
#else
typedef mt_s32 mt_spts;
typedef mt_u32 mt_upts;
#endif

typedef enum mtSYNC_CHAN
{
    SYNC_CHAN_AUD,
    SYNC_CHAN_VID,
    SYNC_CHAN_PCR,
    SYNC_CHAN_SCR,
	SYNC_CHAN_EXT, 
	
    SYNC_CHAN_BUTT
}SYNC_CHAN_E;

typedef enum mtSYNC_STATUS_E
{
    SYNC_STATUS_STOP = 0,
    SYNC_STATUS_PLAY,
    SYNC_STATUS_TPLAY,
    SYNC_STATUS_PAUSE,

    SYNC_STATUS_BUTT
}SYNC_STATUS_E;

typedef enum mtSYNC_PROC_E
{
    SYNC_PROC_DISCARD,
    SYNC_PROC_REPEAT, 
    SYNC_PROC_PLAY,
    SYNC_PROC_QUICKOUTPUT,
    SYNC_PROC_TPLAY,
    SYNC_PROC_CONTINUE,

    SYNC_PROC_BUTT
}SYNC_PROC_E;

typedef enum mtSYNC_AUD_SPEED_ADJUST_E
{
    SYNC_AUD_SPEED_ADJUST_NORMAL,
    SYNC_AUD_SPEED_ADJUST_UP, 
    SYNC_AUD_SPEED_ADJUST_DOWN, 
    SYNC_AUD_SPEED_ADJUST_MUTE_REPEAT,

    SYNC_AUD_SPEED_ADJUST_BUTT
}SYNC_AUD_SPEED_ADJUST_E;


typedef struct mtSYNC_VID_INFO_S
{
    mt_upts   SrcPts;       /* original pts */
    mt_upts   Pts;          /* amendatory pts */
    mt_u32    FrameTime;      /* duration time of one video frame*/
    mt_upts    DispTime;
    MT_BOOL  bProgressive;
    mt_upts    DelayTime;        /* delay time from sync judge to display */
    mt_u32     DispRate;
}SYNC_VID_INFO_S;


typedef struct mtSYNC_VID_OPT_S
{
    SYNC_PROC_E     SyncProc;
    mt_u32          Repeat;
    mt_u32          Discard;
    mt_upts          VdecDiscardTime;
    MT_DRV_VIDEO_TB_ADJUST_E    enTBAdjust;
}SYNC_VID_OPT_S;

typedef struct mtSYNC_AUD_INFO_S
{
    mt_upts   SrcPts;            /* original pts */
    mt_upts   Pts;               /* amendatory pts */
    mt_u32   FrameTime;         /* duration time of one audio frame */
    mt_upts   BufTime;           /* duration time in ao buffer */
    mt_u32   FrameNum;          /* audio frame number in adec buffer */
}SYNC_AUD_INFO_S;

typedef struct mtSYNC_AUD_OPT_S
{
    SYNC_PROC_E               SyncProc;
    SYNC_AUD_SPEED_ADJUST_E   SpeedAdjust;
}SYNC_AUD_OPT_S;

typedef struct mtSYNC_VOUT_FRAME_INFO 
{
    unsigned int idx;       // frame index
    unsigned int apts;      // audio pts
    unsigned int vpts;      // video pts
    unsigned int out_crc;   // for video: the crc value of the video displaying frame
                            // for audio: the crc value of the audio rendering frame
}SYNC_VOUT_FRAME_INFO;

static inline mt_dvb_pts32 msecs_to_mt_dvb_pts32(mt_u32 msecs)
{
	//FIXME: might overflow!
	return (mt_dvb_pts32)(msecs * 45);
}

static inline mt_u32 mt_dvb_pts32_to_msecs(mt_dvb_pts32 pts)
{
	return (mt_u32)(pts / 45);
}

//#ifdef MT_MCE_SUPPORT
mt_s32 MT_DRV_SYNC_Init(mt_void);
mt_s32 MT_DRV_SYNC_DeInit(mt_void);
mt_s32 MT_DRV_SYNC_Create(MT_UNF_SYNC_ATTR_S *pstSyncAttr, mt_handle *phSync);
mt_s32 MT_DRV_SYNC_Destroy(mt_handle hSync);
mt_s32 MT_DRV_SYNC_Start(mt_handle hSync, SYNC_CHAN_E enChn);
mt_s32 MT_DRV_SYNC_Stop(mt_handle hSync, SYNC_CHAN_E enChn);
mt_s32 MT_DRV_SYNC_Play(mt_handle hSync);
mt_s32 MT_DRV_SYNC_AudJudge(mt_handle hSync, SYNC_AUD_INFO_S *pAudInfo, SYNC_AUD_OPT_S *pAudOpt);
mt_s32 MT_DRV_SYNC_VidJudge(mt_handle hSync, SYNC_VID_INFO_S *pVidInfo, SYNC_VID_OPT_S *pVidOpt);
mt_s32 MT_DRV_SYNC_GetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pstSyncAttr);
mt_s32 MT_DRV_SYNC_SetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pSyncAttr);
//#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
