/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC_QUEUE_H__
#define __CC_QUEUE_H__

#include <pthread.h>
#include <stdlib.h>

#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif


#define CRITICAL_SECTION pthread_mutex_t

typedef struct tag_TRI_QUEUE
{
    MT_U32 Head;  /**<queue head*/
    MT_U32 Tail;  /**<queue tail*/
    MT_U32 Size;  /**<queue size*/
    MT_U32 Element_size;/**<element size*/
    MT_VOID *Element;/**<the element*/
    pthread_cond_t cond_insert,cond_remove;/**< pthread condition for blocking insert and remove*/
    CRITICAL_SECTION mCriticalSection; /**<mutual exclusive critical section*/
} TRI_QUEUE;

#define TIMEOUT_RET_REASON_SUCCESS 0
#define TIMEOUT_RET_REASON_TIMEOUT 1
#define TIMEOUT_RET_REASON_NO_TIME_FUNCTION 2
#define TIMEOUT_RET_REASON_QUEUE_EMPTY 3
#define TIMEOUT_RET_REASON_QUEUE_FULL 4
#define TIMEOUT_RET_REASON_WAIT_FAIL 5


void CCQueue_Init(TRI_QUEUE *queue, MT_U32 queue_size,MT_U32 element_size);

void CCQueue_Destroy(TRI_QUEUE *queue);

MT_BOOL CCQueue_Insert(TRI_QUEUE *queue, const void *element);

MT_BOOL CCQueue_Remove(TRI_QUEUE *queue, void *element);

MT_BOOL CCQueue_Get(TRI_QUEUE *queue,void *element);

MT_BOOL CCQueue_IsFull(TRI_QUEUE *queue);

MT_BOOL CCQueue_IsEmpty(TRI_QUEUE *queue);

MT_U32 CCQueue_Availability(TRI_QUEUE *queue);

MT_U32 CCQueue_GetHeadPos(TRI_QUEUE *queue);

MT_U32 CCQueue_TailPos(TRI_QUEUE *queue);

void CCQueue_Flush(TRI_QUEUE *queue);

MT_U32 CCQueue_GetItemNum(TRI_QUEUE *queue);

#ifdef __cplusplus
}
#endif

#endif


