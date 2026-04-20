
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_bufcore.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_BUFCORE_H__
#define __DRV_DISP_BUFCORE_H__

#include "mt_type.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define DISP_BUF_NODE_MAX_NUMBER  32
//#define DISP_BUF_DATA_SIZE        8192
#define DISP_BUF_DATA_SIZE        512
#define DISP_BUF_USER_DATA_SIZE   32

#define DISP_BUF_INVALID_ID 0xffffffffUL

#define DISP_MAKE_ID(bufID, index) ( (((bufID) & 0xffffUL) << 16) | ((index) & 0xffffUL))
#define DISP_GET_BUF_ID(ID)  (((ID) >> 16) & 0xffffUL)
#define DISP_GET_NODE_ID(ID) (((ID) >> 0) & 0xffffUL)

#define DISP_BUFN_STATE_INVALID 0UL
#define DISP_BUFN_STATE_EMPTY   1UL
#define DISP_BUFN_STATE_WRITING 2UL
#define DISP_BUFN_STATE_FULL    3UL
#define DISP_BUFN_STATE_READING 4UL
#define DISP_BUFN_STATE_USED    5UL

#define DBC_CHECK_NULL_RETURN(p) \
do{ if (!p)    \
    {DISP_FATAL("FUNC %s input NULL Pointer!\n", __FUNCTION__); return MT_FAILURE;} \
}while(0)

#define DBC_CHECK_NULL_RETURN_NULL(p) \
do{ if (!p)    \
    {DISP_FATAL("FUNC %s input NULL Pointer!\n", __FUNCTION__); return MT_NULL;} \
}while(0)


typedef struct tagDISP_BUF_NODE_S
{
    /* bit[31~16] resevered, zero; bit[15~8], buf id; bit[7~0], node id */
    volatile mt_u32 u32ID;
    volatile mt_u32 u32State;

    ulong u32Data[DISP_BUF_DATA_SIZE];				/* see: MT_DRV_VIDEO_FRAME_S */
    ulong u32UserData[DISP_BUF_USER_DATA_SIZE];

    mt_u32 u32EmptyCount;
    mt_u32 u32FullCount;
}DISP_BUF_NODE_S;

typedef struct tagDISP_BUF_S
{
    mt_u32 u32BufID;

    mt_u32 u32Number;
    DISP_BUF_NODE_S *pstBufArray[DISP_BUF_NODE_MAX_NUMBER];

    mt_u32 u32EmptyArray[DISP_BUF_NODE_MAX_NUMBER];
    volatile mt_u32 u32EmptyReadPos;
    volatile mt_u32 u32EmptyWritePos;

    mt_u32 u32FullArray[DISP_BUF_NODE_MAX_NUMBER];
    volatile mt_u32 u32FullReaddPos;
    volatile mt_u32 u32FullWritePos;

    // todo
    mt_u32 u32UsedArray[DISP_BUF_NODE_MAX_NUMBER];
    volatile mt_u32 u32UsedReaddPos;
    volatile mt_u32 u32UsedWritePos;
}DISP_BUF_S;

mt_s32 DispBuf_Create(DISP_BUF_S *pstBuffer, mt_u32 number);
mt_s32 DispBuf_Destoy(DISP_BUF_S *pstBuffer);
mt_s32 DispBuf_Reset(DISP_BUF_S *pstBuffer);

// create a new node
mt_s32 DispBuf_GetNodeContent(DISP_BUF_S *pstBuffer, mt_s32 s32Index, DISP_BUF_NODE_S **ppstNode);

// get first empty node and not take it away from empty array
mt_s32 DispBuf_GetEmptyNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S **ppstNode);
// take node away from empty array and set Writing State
mt_s32 DispBuf_DelEmptyNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S *pstNode);
// put node into empry array and set empty state
mt_s32 DispBuf_AddEmptyNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S *pstNode);

// get first full node and not take it away from full array
mt_s32 DispBuf_GetFullNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S **ppstNode);
// get second full node and not take it away from full array
mt_s32 DispBuf_GetNextFullNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S **ppstNode);

mt_s32 DispBuf_GetFullNodeByIndex(DISP_BUF_S *pstBuffer, mt_u32 u32Index, DISP_BUF_NODE_S **ppstNode);

// take node away from full array and set reading state
mt_s32 DispBuf_DelFullNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S *pstNode);
// put node into full array and set full state
mt_s32 DispBuf_AddFullNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S *pstNode);
// test whether the node is last one in full array
//mt_s32 DispBuf_IsLastFullNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S *pstNode);


mt_s32 DispBuf_GetFullNodeNumber(DISP_BUF_S *pstBuffer, mt_u32 *pu32Num);
mt_s32 DispBuf_PrintFullState(DISP_BUF_S *pstBuffer);

MT_BOOL DispBuf_FullListHasSameNode(DISP_BUF_S *pstBuffer, DISP_BUF_NODE_S *pstNode);
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_BUFCORE_H__  */



