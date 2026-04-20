/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "sample_teletext_msg.h"

#ifdef  MT_SAMPLE_TTX_DEBUG

#define MT_TTX_PRINT   printf
#else

#define MT_TTX_PRINT

#endif

#define SAMPLE_TTX_FUNCTION_ENTER() MT_TTX_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TTX_FUNCTION_EXIT()      MT_TTX_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_TTX_FATAL_PRINT(fmt...)          MT_TTX_PRINT(" [FATAL] " fmt)
#define SAMPLE_TTX_ERR_PRINT(fmt...)            MT_TTX_PRINT(" [ERROR] " fmt)
#define SAMPLE_TTX_WARN_PRINT(fmt...)           MT_TTX_PRINT(" [WARN] "  fmt)
#define SAMPLE_TTX_INFO_PRINT(fmt...)           MT_TTX_PRINT(" [INFO] "  fmt)
#define SAMPLE_TTX_DBG_PRINT(fmt...)            MT_TTX_PRINT(" [DEBUG] " fmt)

#define MIN_GAP (64)

static MSG_QUEUE_S s_stMsgQueue = {0};



static mt_s32 IsEmptyQueue(MSG_QUEUE_S * pstMsgQueue)
{
    if(pstMsgQueue->pstMsgRear != pstMsgQueue->pstMsgFront)
    {
        SAMPLE_TTX_ERR_PRINT("write and read failed!\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 IsFullQueue(MSG_QUEUE_S * pstMsgQueue)
{
    mt_u32 u32Step = sizeof(MESSAGE_S) + pstMsgQueue->pstMsgRear->u16MsgLength;

    ulong u32RemainSpace = 0;

    if(pstMsgQueue->pstMsgRear >= pstMsgQueue->pstMsgFront)
    {
        u32RemainSpace = ((ulong)pstMsgQueue->pu8BaseAddr + pstMsgQueue->u32MaxLen) - (ulong)pstMsgQueue->pstMsgRear;

        if((ulong)pstMsgQueue->pstMsgFront > (ulong)pstMsgQueue->pu8BaseAddr)
        {
            u32RemainSpace += ((ulong)pstMsgQueue->pstMsgFront - MIN_GAP - (ulong)pstMsgQueue->pu8BaseAddr);
        }
        else
        {
            u32RemainSpace -= MIN_GAP;
        }
    }
    else
    {
        u32RemainSpace = (ulong)pstMsgQueue->pstMsgFront - MIN_GAP - (ulong)pstMsgQueue->pstMsgRear;
    }

    if(u32Step > u32RemainSpace)
    {
        /* Queue  full*/
        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

mt_s32 MsgQueue_Init(mt_void)
{
    mt_u8* pszBufAddr = MT_NULL;
    mt_u32 u32BufLen = 1024;

    pszBufAddr = (mt_u8 *) malloc(u32BufLen * sizeof(mt_u8));
    if(pszBufAddr == MT_NULL)
    {
        SAMPLE_TTX_ERR_PRINT("malloc pszBufAddr failed!\n");
        return MT_FAILURE;
    }

    if(0 != pthread_mutex_init(&s_stMsgQueue.Queue_lock, NULL));

    s_stMsgQueue.pu8BaseAddr = pszBufAddr;
    s_stMsgQueue.u32MaxLen = u32BufLen;

    s_stMsgQueue.pstMsgFront = (MESSAGE_S*)pszBufAddr;
    s_stMsgQueue.pstMsgFront->pu8MsgData = MT_NULL;
    s_stMsgQueue.pstMsgFront->pstNext = MT_NULL;

    s_stMsgQueue.pstMsgRear = (MESSAGE_S*)pszBufAddr;
    s_stMsgQueue.pstMsgRear->pu8MsgData = MT_NULL;
    s_stMsgQueue.pstMsgRear->pstNext = MT_NULL;

    return MT_SUCCESS;
}

mt_s32  MsgQueue_DeInit(mt_void)
{
    if(0 != pthread_mutex_destroy(&(s_stMsgQueue.Queue_lock)));

    if(s_stMsgQueue.pu8BaseAddr != MT_NULL)
    {
        free(s_stMsgQueue.pu8BaseAddr);
    }

    s_stMsgQueue.pu8BaseAddr = MT_NULL;
    s_stMsgQueue.u32MaxLen   = 0;
    s_stMsgQueue.pstMsgFront = MT_NULL;
    s_stMsgQueue.pstMsgRear  = MT_NULL;

    return MT_SUCCESS;
}

static mt_s32  MsgQueueEn(MESSAGE_S * pstMsg)
{
    mt_u32 u32DataTypeSize = sizeof(MESSAGE_S);
    mt_u32 u32ItemSize = 0;
    MESSAGE_S * pstNextMsgAddr = MT_NULL;

    if((pstMsg == MT_NULL) || (s_stMsgQueue.pu8BaseAddr == MT_NULL))
    {
        SAMPLE_TTX_ERR_PRINT("pstMsg is null!\n");
        return MT_FAILURE;
    }

    s_stMsgQueue.pstMsgRear->u16MsgLength = pstMsg->u16MsgLength;
    if(IsFullQueue(&s_stMsgQueue ) == MT_SUCCESS)
    {
        return MT_FAILURE;
    }

    u32ItemSize = u32DataTypeSize + pstMsg->u16MsgLength;

    if(((ulong)s_stMsgQueue.pstMsgRear + u32ItemSize) > ((ulong)s_stMsgQueue.pu8BaseAddr + s_stMsgQueue.u32MaxLen))
    {
        s_stMsgQueue.pstMsgRear->pu8MsgData = (mt_u8*)s_stMsgQueue.pu8BaseAddr + u32DataTypeSize;
    }
    else
    {
        s_stMsgQueue.pstMsgRear->pu8MsgData = (mt_u8*)(s_stMsgQueue.pstMsgRear + u32DataTypeSize);
    }

    s_stMsgQueue.pstMsgRear->u16MsgLength = pstMsg->u16MsgLength;
    memcpy(s_stMsgQueue.pstMsgRear->pu8MsgData, pstMsg->pu8MsgData, pstMsg->u16MsgLength);

    pstNextMsgAddr = (MESSAGE_S*)((ulong)s_stMsgQueue.pstMsgRear + u32ItemSize);
    if(((ulong)pstNextMsgAddr + u32DataTypeSize) > ((ulong)s_stMsgQueue.pu8BaseAddr + s_stMsgQueue.u32MaxLen))
    {
        pstNextMsgAddr = (MESSAGE_S *)(mt_void *)s_stMsgQueue.pu8BaseAddr;
    }

    pstNextMsgAddr->pu8MsgData = MT_NULL;
    pstNextMsgAddr->pstNext = MT_NULL;
    s_stMsgQueue.pstMsgRear->pstNext = pstNextMsgAddr;
    s_stMsgQueue.pstMsgRear = pstNextMsgAddr;
    s_stMsgQueue.pstMsgRear->u16MsgLength = 0;
    if(IsFullQueue(&s_stMsgQueue) == MT_SUCCESS)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 MsgQueue_En(MESSAGE_S * pstMsg)
{
    mt_s32 s32Ret = MT_SUCCESS;

    if((pstMsg == MT_NULL) || (s_stMsgQueue.pu8BaseAddr == MT_NULL))
    {
        SAMPLE_TTX_ERR_PRINT("pstMsg is null!\n");
        return MT_FAILURE;
    }

    pthread_mutex_lock(&s_stMsgQueue.Queue_lock);
    s32Ret = MsgQueueEn(pstMsg);
    pthread_mutex_unlock(&s_stMsgQueue.Queue_lock);

    return s32Ret;
}

static mt_s32  MsgQueueDe(MESSAGE_S * pstMsg)
{
    if((pstMsg == MT_NULL))
    {
        return MT_FAILURE;
    }

    if(IsEmptyQueue(&s_stMsgQueue) == MT_SUCCESS)
    {
        return MT_FAILURE;
    }

    pstMsg->u16MsgLength = s_stMsgQueue.pstMsgFront->u16MsgLength;
    pstMsg->pu8MsgData = s_stMsgQueue.pstMsgFront->pu8MsgData;
    pstMsg->pstNext = MT_NULL;
    s_stMsgQueue.pstMsgFront = s_stMsgQueue.pstMsgFront->pstNext;

    return MT_SUCCESS;
}

mt_s32 MsgQueue_De(MESSAGE_S * pstMsg)
{
    mt_s32 s32Ret = MT_SUCCESS;

    if(pstMsg == MT_NULL)
    {
        SAMPLE_TTX_ERR_PRINT("pstMsg is null!\n");
        return MT_FAILURE;
    }

    pthread_mutex_lock(&s_stMsgQueue.Queue_lock);
    s32Ret = MsgQueueDe(pstMsg);
    pthread_mutex_unlock(&s_stMsgQueue.Queue_lock);

    return s32Ret;
}
