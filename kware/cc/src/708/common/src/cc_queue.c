/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include <errno.h>

#include "mt_type.h"
#include "cc_queue.h"
#include "cc_debug.h"

#define EnterCriticalSection(x)  \
    do{ \
        int lock_ret = pthread_mutex_lock(x); \
        if(lock_ret != 0){ \
            MT_ERR_CC("CC call pthread_mutex_lock(QUEUE) failure,lock_ret = 0x%x\n",lock_ret); \
        } \
    }while(0)

#define LeaveCriticalSection(x)  \
    do{ \
        int unlock_ret = pthread_mutex_unlock(x); \
        if(unlock_ret != 0){ \
            MT_ERR_CC("CC call pthread_mutex_unlock(QUEUE) failure,unlock_ret = 0x%x\n",unlock_ret); \
        } \
    }while(0)

#define InitializeCriticalSection(x)  \
    do{ \
        int init_ret = pthread_mutex_init(x,NULL); \
        if(init_ret != 0){ \
            MT_ERR_CC("CC call pthread_mutex_init(QUEUE) failure,init_ret = 0x%x\n",init_ret); \
        } \
    }while(0)

#define DeleteCriticalSection(x)  \
    do{ \
        int destroy_ret = pthread_mutex_destroy(x); \
        if(destroy_ret != 0){ \
            MT_ERR_CC("CC call pthread_mutex_destroy(QUEUE) failure,destroy_ret = 0x%x\n",destroy_ret); \
        } \
    }while(0)

void CCQueue_Init(TRI_QUEUE *queue,MT_U32 queue_size,MT_U32 element_size)
{
    queue->Size = queue_size;

    queue->Element_size = element_size;
    queue->Element = (void *)MT_MEMCC_MALLOC(element_size*queue_size);
    if (NULL == queue->Element)
    {
        MT_ERR_CC("malloc for cc queue failed!\n");
    }

    InitializeCriticalSection( &queue->mCriticalSection );
    EnterCriticalSection(&queue->mCriticalSection );
    queue->Head = 0;
    queue->Tail = 0;
    LeaveCriticalSection(&queue->mCriticalSection );
    pthread_cond_init(&queue->cond_insert,NULL);
    pthread_cond_init(&queue->cond_remove,NULL);
}

void CCQueue_Destroy(TRI_QUEUE *queue)
{
    if (queue->Element)
    {
        MT_MEMCC_FREE(queue->Element);
        queue->Element = NULL;
    }
    pthread_cond_destroy(&queue->cond_insert);
    pthread_cond_destroy(&queue->cond_remove);
    DeleteCriticalSection(&queue->mCriticalSection );
}

MT_BOOL CCQueue_Insert(TRI_QUEUE *queue, const void *element)
{
    MT_U32 n = 0;
    EnterCriticalSection( &queue->mCriticalSection );

    if (((queue->Head + 1) % queue->Size) == queue->Tail )
    {
        LeaveCriticalSection( &queue->mCriticalSection );
        return MT_FALSE;
    }
    n = queue->Head++;
    if (queue->Element)
    {
        memcpy((MT_U8 *)queue->Element+n*queue->Element_size,(MT_U8 *)element,queue->Element_size);
    }
    queue->Head %= queue->Size;
    // leave the critical section
    LeaveCriticalSection( &queue->mCriticalSection );
    (MT_VOID)pthread_cond_signal(&queue->cond_remove);
    return MT_TRUE;
}

MT_BOOL CCQueue_Remove(TRI_QUEUE *queue, void *element)
{
    MT_U32 n=0;
    EnterCriticalSection( &queue->mCriticalSection );
    if ( queue->Tail == queue->Head)
    {
        LeaveCriticalSection( &queue->mCriticalSection );
        return MT_FALSE;
    }

    n = queue->Tail++;
    if (queue->Element)
    {
        memcpy((MT_U8 *)element,(MT_U8 *)queue->Element+n*queue->Element_size,queue->Element_size);
    }
    queue->Tail%=queue->Size;
    // leave the critical section
    LeaveCriticalSection( &queue->mCriticalSection );
    (MT_VOID)pthread_cond_signal(&queue->cond_insert);
    return MT_TRUE;
}

MT_BOOL CCQueue_Get(TRI_QUEUE *queue, void *element)
{
    MT_U32 n;
    MT_BOOL ret;
    EnterCriticalSection( &queue->mCriticalSection );
    if ( queue->Tail == queue->Head )
    {
        ret = MT_FALSE;
    }
    else
    {
        ret = MT_TRUE;
        n=queue->Tail;
        if (queue->Element)
        {
            memcpy((MT_U8 *)element,(MT_U8 *)queue->Element+n*queue->Element_size,queue->Element_size);
        }
    }
    LeaveCriticalSection( &queue->mCriticalSection );
    return ret;
}

MT_BOOL CCQueue_IsFull(TRI_QUEUE *queue)
{
    MT_BOOL ret = MT_FALSE;
    // enter the critical section
    EnterCriticalSection( &queue->mCriticalSection );
    ret = (MT_BOOL)(((queue->Head+1)%queue->Size) == queue->Tail);
    // leave the critical section
    LeaveCriticalSection( &queue->mCriticalSection );
    return ret;
}

MT_BOOL CCQueue_IsEmpty(TRI_QUEUE *queue)
{
    MT_BOOL ret = MT_FALSE;

    // enter the critical section
    EnterCriticalSection( &queue->mCriticalSection );

    ret = (MT_BOOL)(queue->Tail == queue->Head);

    // leave the critical section
    LeaveCriticalSection( &queue->mCriticalSection );

    return ret;

}

MT_U32 CCQueue_Availability(TRI_QUEUE *queue)
{
    MT_U32 ret = 0;
    // enter the critical section
    EnterCriticalSection( &queue->mCriticalSection );
    if (queue->Head<queue->Tail)
        ret=queue->Tail-queue->Head;
    else
        ret = queue->Size - queue->Head + queue->Tail;
    // leave the critical section
    LeaveCriticalSection( &queue->mCriticalSection );
    return ret;

}

MT_U32 CCQueue_GetHeadPos(TRI_QUEUE *queue)
{
    return queue->Head;
}

MT_U32 CCQueue_TailPos(TRI_QUEUE *queue)
{
    MT_U32 n ;
    n = queue->Tail;
    return n;
}

void CCQueue_Flush(TRI_QUEUE *queue)
{
    // enter the critical section
    EnterCriticalSection( &queue->mCriticalSection );
    queue->Head = queue->Tail = 0;
    LeaveCriticalSection( &queue->mCriticalSection );
    (MT_VOID)pthread_cond_signal(&queue->cond_insert); //wake up the thread want to insert elements.
}

MT_U32 CCQueue_GetItemNum(TRI_QUEUE *queue)
{
    MT_U32 n;
    // enter the critical section
    EnterCriticalSection( &queue->mCriticalSection );
    if (queue->Head>=queue->Tail)
        n =  (queue->Head - queue->Tail);
    else
        n = queue->Size + queue->Head - queue->Tail;
    // leave the critical section
    LeaveCriticalSection( &queue->mCriticalSection );
    return n;
}

