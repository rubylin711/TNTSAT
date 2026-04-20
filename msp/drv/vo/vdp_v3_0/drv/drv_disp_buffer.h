
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_buffer.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_BUFFER_H__
#define __DRV_DISP_BUFFER_H__

#include "mt_type.h"
#include "drv_disp_com.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define WIN_BUFFER_MAX_NUMBER 16
#define WIN_BUFFER_MIN_NUMBER 1
#define WIN_BUFFER_DEFAULT_NUMBER 8

#define WIN_BUFFER_PRIV_DATA_SIZE 32
#define WIN_BUFFER_USER_DATA_SIZE 8

#define WIN_BUFFER_INDEX_PREFIX  ( ( ((mt_u32)'V')<<24 )|( ((mt_u32)'O')<<16 ) )
#define WIN_BUFFER_INDEX_PREFIX_MASK  0xFFFF0000ul
#define WIN_BUFFER_INDEX_SHIFT 8
#define WIN_BUFFER_INDEX_MASK  0x000000FFul
#define WIN_BUFFER_INDEX_INVALID  0xFFFFFFFFul

#define WIN_BUFFER_MAX_W 4096
#define WIN_BUFFER_MIN_W 64
#define WIN_BUFFER_MAX_H 2160
#define WIN_BUFFER_MIN_H 64

#define DISP_CAST_LOWDELAY_MEM_LENGTH   0x1000

typedef struct
{
    MT_BOOL bFbAllocMem;
    mt_u32 u32BufWidth;
    mt_u32 u32BufHeight;
    mt_u32 u32BufStride;
    mt_u32 u32BufSize;
    MT_DRV_PIX_FORMAT_E eDataFormat;

	// user supply mem
}BUF_ALLOC_S;

typedef struct
{
    mt_u32 u32UserData[WIN_BUFFER_USER_DATA_SIZE];
}BUF_USERDATA_S;

typedef enum
{
    BUF_STATE_EMPTY = 0,
    BUF_STATE_READING,
    BUF_STATE_WRITING,
    BUF_STATE_FULL,
    BUF_STATE_DONE,  /* wait to release */
    BUF_STATE_BUTT
}BUF_STATE_E;

typedef enum
{
    BUF_MEM_SRC_SUPPLY = 0,
    BUF_MEM_FB_SUPPLY,
    BUF_MEM_USER_SUPPLY,
    BUF_MEM_TYPE_BUTT
}BUF_MEM_TYPE_E;

typedef struct
{
    mt_u32 u32Index;
    BUF_STATE_E enState;
        
    MT_DRV_VIDEO_FRAME_S stFrame;
    BUF_USERDATA_S stUserData;
    mt_u32 u32PrivData[WIN_BUFFER_PRIV_DATA_SIZE];

    DISP_MMZ_BUF_S stMem;
}BUF_S;

typedef struct
{
    /* bit[31-16]=index prefix, bit[15-8]=buffer id, bit[7-0]=0 */
    mt_u32 u32Index;
    mt_u32 u32NodeStage; /*for cfg only.*/
}BUF_ID_S;

typedef struct
{
    /* number of frame in buffer currently  */
    mt_u32 u32EmptyDel;
    mt_u32 u32EmptyAdd;
    mt_u32 u32EmptyDoing;

    mt_u32 u32CfgDel;
    mt_u32 u32CfgAdd;
    mt_u32 u32CfgDoing;
    mt_u32 u32FullDel;
    mt_u32 u32FullAdd;
    mt_u32 u32FullDoing;
}BUF_STAT_S;

typedef struct
{
    mt_u32 u32BufNum;
    
    /*maybe it should not be located here, it's just for storing the
      setting passed by user. not real allocated information.*/
    mt_u32  u32BufSize;
    mt_u32  u32BufStride;
    
    //BUF_S stBufQueue[WIN_BUFFER_MAX_NUMBER];
    BUF_S *pstBufQueue;

    //BUF_ID_S stEmptyQueue[WIN_BUFFER_MAX_NUMBER];
    BUF_ID_S *pstEmptyQueue;
    mt_u32 u32EmptyRPtr;
    mt_u32 u32EmptyWPtr;

    //BUF_ID_S stFullQueue[WIN_BUFFER_MAX_NUMBER];
    BUF_ID_S *pstFullQueue;
    mt_u32 u32FullRPtr;
    mt_u32 u32FullWPtr;

    BUF_ID_S *pstCfgWritingQueue;
    mt_u32 u32CfgWritingRPtr;
    mt_u32 u32CfgWritingWPtr;
    MT_BOOL bAllocMemory;
    BUF_ALLOC_S stAlloc;
    BUF_MEM_TYPE_E enMemType;
    BUF_STAT_S stStatistic;
}BUF_POOL_S;

mt_s32 BP_Create(mt_u32 u32BufNum, BUF_ALLOC_S *pstAlloc, BUF_POOL_S *pstBP);
mt_s32 BP_Destroy(BUF_POOL_S *pstBP);
mt_s32 BP_Reset(BUF_POOL_S *pstBP);

// produce and consume empty buffer
mt_s32 BP_GetEmptyBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId);
mt_s32 BP_GetDoneBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId);
mt_s32 BP_DelEmptyBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId);
mt_s32 BP_AddEmptyBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId);
mt_s32 BP_SetBufEmpty(BUF_POOL_S *pstBP, mt_u32 u32BufId);

// produce and consume full buffer
mt_s32 BP_GetFullBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId);
mt_s32 BP_DelFullBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId);
mt_s32 BP_AddFullBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId);


// get and set frame to buffer
mt_s32 BP_GetFrame(BUF_POOL_S *pstBP, mt_u32 u32BufId, MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 BP_SetFrame(BUF_POOL_S *pstBP, mt_u32 u32BufId, MT_DRV_VIDEO_FRAME_S *pstFrame);
mt_s32 BP_ReAllocBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId);
mt_s32 BP_ReAllocAllBuf(BUF_POOL_S *pstBP);

// get and set user data to buffer
mt_s32 BP_SetUserData(BUF_POOL_S *pstBP, mt_u32 u32BufId, BUF_USERDATA_S *pstData);
mt_s32 BP_GetUserData(BUF_POOL_S *pstBP, mt_u32 u32BufId, BUF_USERDATA_S *pstData);

mt_s32 BP_GetFullBufNum(BUF_POOL_S *pstBP, mt_u32 *pu32BufNum);
mt_s32 BP_GetEmptyBufNum(BUF_POOL_S *pstBP, mt_u32 *pu32BufNum);

mt_s32 BP_GetCfgWritingBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId, mt_u32 u32CfgStage,mt_u32 *u32CfgStageAct);
mt_s32 BP_DelCfgWritingBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId);
mt_void BP_IncreaseAllCfgWritingState(BUF_POOL_S *pstBP, mt_u32 *pu32BufId);
mt_s32 BP_GetCfgWritingBuf_JustWriting(BUF_POOL_S *pstBP, mt_u32 *pu32BufId, mt_u32 u32Stage);
mt_s32 BP_AddCfgWritingBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId);
mt_s32 BP_SetBufReading(BUF_POOL_S *pstBP, mt_u32 u32BufId);

typedef struct
{
    mt_u32 u32BufNum;
    mt_u32 enMemType;
    mt_u32 enPixFmt;

    mt_u32 u32EmptyDel;
    mt_u32 u32EmptyAdd;
    mt_u32 u32EmptyDoing;

    mt_u32 u32FullDel;
    mt_u32 u32FullAdd;
    mt_u32 u32FullDoing;

    mt_u32 stBufState[WIN_BUFFER_MAX_NUMBER];

    mt_u32 stEmptyQueue[WIN_BUFFER_MAX_NUMBER];
    mt_u32 u32EmptyRPtr;
    mt_u32 u32EmptyWPtr;

    mt_u32 stFullQueue[WIN_BUFFER_MAX_NUMBER];
    mt_u32 u32FullRPtr;
    mt_u32 u32FullWPtr;

    MT_DRV_VIDEO_FRAME_S stFrame;
}BUF_STT_S;

mt_s32 BP_GetBufState(BUF_POOL_S *pstBP, BUF_STT_S *pstBufState);
mt_s32 BP_CreateSDWriteBackMem(bool sd_wrback_422, mt_u8 field_num);
mt_s32 BP_DestroySDWriteBackMem(mt_void);
DISP_MMZ_BUF_S *BP_GetSDWriteBackMemInfo(mt_void);
mt_s32 BP_CreateHDTestMem(mt_void);
mt_s32 BP_DestroyHDTestMem(mt_void);
DISP_MMZ_BUF_S *BP_GetHDTestMemInfo(mt_void);
mt_s32 BP_CreateDebugTestMem(mt_void);
mt_s32 BP_DestroyDebugTestMem(mt_void);
DISP_MMZ_BUF_S *BP_GetDebugTestMemInfo(mt_void);
mt_s32 BP_CreateBlackFrame(mt_void);
mt_s32 BP_DestroyBlackFrame(mt_void);
MT_DRV_VIDEO_FRAME_S *BP_GetBlackFrameInfo(mt_void);




#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_VO_BUFFER_H__  */


