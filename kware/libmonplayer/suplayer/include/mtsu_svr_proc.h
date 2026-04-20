/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_SVR_PROC_H__
#define __MT_SVR_PROC_H__

#include "hi_type.h"


typedef struct mtSVR_PLAYER_PROC_SEEKINFO_S
{
//trace seek
    MT_U32      u32DoReadSeek;
    MT_U32      u32ReadSeekDone;
    MT_U32      u32DoSeekFrameBinary;  //ff_seek_frame_binary
    MT_U32      u32SeekFrameBinaryDone;
    MT_U32      u32DoSeekFrameGeneric;
    MT_U32      u32SeekFrameGenericDone;
//trace switch-pg
    MT_U32      u32DoInitInput;
    MT_U32      u32DoAvioOpenH;
    MT_U32      u32AvioOpenHDone;
    MT_U32      u32DoAvProbeInputBuffer;
    MT_U32      u32AvProbeInputBufferDone;
    MT_U32      u32FindEndPtsStart;
    MT_U32      u32FindEndPtsDone;
    MT_U32      u32EstimateDurationStart;
    MT_U32      u32EstimateDurationDone;
    MT_U32      u32DoPAdaptStart;
    MT_U32      u32DoPAdaptStartDone;
    MT_U32      u32FirstVidFrameRead;
    MT_U32      u32FirstAudFrameRead;
    MT_U32      u32FirstVidFrameSent;
    MT_U32      u32FirstAudFrameSent;

//for hi_svr_format
    MT_U32      u32DoHiSvrFormatSeekPts;
    MT_U32      u32CmdSeek;
    MT_U32      u32DoAvformatOpenInput;
    MT_U32      u32AvformatOpenInputDone;
    MT_U32      u32DoSvrFormatFindStream;
    MT_U32      u32SvrFormatFindStreamDone;
    MT_U32      u32DoSvrFormatGetFileInfo;
    MT_U32      u32SvrFormatGetFileInfoDone;

} MT_SVR_PLAYER_PROC_SEEKINFO_S;


typedef enum mtSVR_PLAYER_PROC_SWITCHPG_INFOTYPE_E
{
    MT_SVR_PLAYER_PROC_DO_STOP=1,
    MT_SVR_PLAYER_PROC_HIMEDIAPLAYER_CONSTRUCT,
    MT_SVR_PLAYER_PROC_SETDATASOURCE,
    MT_SVR_PLAYER_PROC_UNF_AVPLAY_CREATE,
    MT_SVR_PLAYER_PROC_DO_PREPARE,
    MT_SVR_PLAYER_PROC_PREPARE_ASYNC_COMPLETE,
    MT_SVR_PLAYER_PROC_DO_START_ENTER,
    MT_SVR_PLAYER_PROC_PLAYER_STATE_PLAY,
    MT_SVR_PLAYER_PROC_MEDIA_INFO_FIRST_FRAME_TIME,
    MT_SVR_PLAYER_PROC_DO_RESET,
    MT_SVR_PLAYER_PROC_DO_DESTRUCTOR,

} MT_SVR_PLAYER_PROC_SWITCHPG_INFOTYPE_E;

typedef struct mtSVR_PLAYER_PROC_SWITCHPG_S
{
    MT_SVR_PLAYER_PROC_SWITCHPG_INFOTYPE_E eType;
    MT_U32 u32DoStop;       //1
    MT_U32 u32HiMediaPlayerConstruct; //2
    MT_U32 u32SetDataSource;        //3
    MT_U32 u32DoCreateAVPlay;  //4
    MT_U32 u32DoPrepare;            //5
    MT_U32 u32prepareAsyncComplete; //6
    MT_U32 u32DoStartEnter;         //7
    MT_U32 u32PlayedEvent;  //8
    MT_U32 u32FirstFrameTime;  //9
    MT_U32 u32DoReset;              //10
    MT_U32 u32DoDestructor;         //11

} MT_SVR_PLAYER_PROC_SWITCHPG_S;



#endif /* __MT_SVR_PROC_H__ */
