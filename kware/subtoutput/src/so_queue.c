/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "linux_list.h"
#include "so_queue.h"

typedef struct hiSO_QUEUE_S
{
    mt_s32 num;    /* number of item in msg queue*/
    mt_u32 u32MaxNodeNum;
    mt_s32 s32BufferSize; /* size of buffer in msg queue */
    mt_u32 u32MaxBufferSize;
    struct list_head list;
    pthread_mutex_t lock;
    MT_HANDLE bufHandle;
} SO_QUEUE_S;

typedef struct hiSO_NODE_S
{
    struct list_head list;
    SO_INFO_S stInfo;
    mt_u8  addr[0];
} SO_NODE_S;

#define SO_ADD_EXT_BYTE_NUM   (4)
#define SO_NODE_SIZE          sizeof(SO_NODE_S)
#define SO_QUEUE_LOCK()       pthread_mutex_lock(&pSoQueue->lock);
#define SO_QUEUE_UNLOCK()     pthread_mutex_unlock(&pSoQueue->lock);

static mt_s32 _SO_GetBufLen(const SO_INFO_S *pstInfo)
{
    mt_s32 s32Len = 0;

    if (MT_UNF_SUBTITLE_BITMAP == pstInfo->eType)
    {
        s32Len = (mt_s32)pstInfo->unSubtitleParam.stGfx.u32Len;
    }
    else if (MT_UNF_SUBTITLE_TEXT == pstInfo->eType)
    {
        s32Len = (mt_s32)pstInfo->unSubtitleParam.stText.u32Len;
    }
    else if (MT_UNF_SUBTITLE_ASS == pstInfo->eType)
    {
        s32Len = (mt_s32)pstInfo->unSubtitleParam.stAss.u32FrameLen;
    }
    else
    {
        s32Len = 0;
    }

    return s32Len;
}

static mt_s32 _SO_InitNode(const SO_INFO_S *pstInfo, SO_NODE_S *pstNode)
{
    if (MT_UNF_SUBTITLE_BITMAP == pstInfo->eType)
    {
        pstNode->stInfo.eType = MT_UNF_SUBTITLE_BITMAP;
        SO_MEMCPY(pstNode->addr, pstInfo->unSubtitleParam.stGfx.pu8PixData, pstInfo->unSubtitleParam.stGfx.u32Len);
        pstNode->addr[pstInfo->unSubtitleParam.stGfx.u32Len] = '\0';
        SO_MEMCPY(&pstNode->stInfo.unSubtitleParam.stGfx, &pstInfo->unSubtitleParam.stGfx, sizeof(MT_UNF_SO_GFX_S));
    }
    else if (MT_UNF_SUBTITLE_TEXT == pstInfo->eType)
    {
        pstNode->stInfo.eType = MT_UNF_SUBTITLE_TEXT;
        SO_MEMCPY(pstNode->addr, pstInfo->unSubtitleParam.stText.pu8Data, pstInfo->unSubtitleParam.stText.u32Len);
        pstNode->addr[pstInfo->unSubtitleParam.stText.u32Len] = '\0';
        SO_MEMCPY(&pstNode->stInfo.unSubtitleParam.stText, &pstInfo->unSubtitleParam.stText, sizeof(MT_UNF_SO_TEXT_S));
    }
    else if (MT_UNF_SUBTITLE_ASS == pstInfo->eType)
    {
        pstNode->stInfo.eType = MT_UNF_SUBTITLE_ASS;
        SO_MEMCPY(pstNode->addr, pstInfo->unSubtitleParam.stAss.pu8EventData, pstInfo->unSubtitleParam.stAss.u32FrameLen);
        pstNode->addr[pstInfo->unSubtitleParam.stAss.u32FrameLen] = '\0';
        SO_MEMCPY(&pstNode->stInfo.unSubtitleParam.stAss, &pstInfo->unSubtitleParam.stAss, sizeof(MT_UNF_SO_ASS_S));
    }
    else
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 SO_QueueInit( mt_u32 bufsize, mt_u32 maxNodeNum, SO_QUEUE_HANDLE *handle )
{
    mt_s32 ret = MT_SUCCESS;
    SO_QUEUE_S *pSoQueue = NULL;

    SO_RETURN(bufsize < SO_NORMAL_BUFF_SIZE, MT_FAILURE, "### Queue size is less than 1024!\n");

    pSoQueue = (SO_QUEUE_S*)SO_MALLOC(sizeof(SO_QUEUE_S));
    SO_RETURN(NULL == pSoQueue, MT_FAILURE, "### No mem!\n");

    INIT_LIST_HEAD(&pSoQueue->list);
    pthread_mutex_init(&pSoQueue->lock, NULL);
    pSoQueue->num = 0;
    pSoQueue->s32BufferSize = 0;
    pSoQueue->u32MaxBufferSize = bufsize;
    pSoQueue->u32MaxNodeNum    = maxNodeNum;

    *handle = pSoQueue;
    return ret;
}

mt_s32 SO_QueueDeinit( SO_QUEUE_HANDLE handle )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;

    SO_RETURN(NULL == pSoQueue, MT_FAILURE, "");

    (mt_void)SO_QueueRemove(handle);

    pthread_mutex_destroy(&pSoQueue->lock);
    SO_FREE(pSoQueue);

    return MT_SUCCESS;
}

/*lint -e429*/
mt_s32 SO_QueuePut( SO_QUEUE_HANDLE handle, const SO_INFO_S *pstInfo )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;
    SO_NODE_S * pNode;
    mt_s32 s32Len = 0;

    SO_RETURN(NULL == pSoQueue, MT_FAILURE, "");
    SO_RETURN(NULL == pstInfo, MT_FAILURE, "");

    s32Len = _SO_GetBufLen(pstInfo);
    SO_RETURN(0 > s32Len, MT_FAILURE, "");

    SO_QUEUE_LOCK();
    SO_CALL_RETURN((mt_s32)pSoQueue->u32MaxBufferSize <= pSoQueue->s32BufferSize && pSoQueue->num >= (mt_s32)pSoQueue->u32MaxNodeNum,
        SO_QUEUE_UNLOCK(), MT_FAILURE);

    pNode = (SO_NODE_S *)SO_MALLOC(SO_NODE_SIZE + (mt_u32)s32Len + SO_ADD_EXT_BYTE_NUM);
    SO_CALL_RETURN(NULL == pNode, SO_QUEUE_UNLOCK(), MT_FAILURE);

    SO_MEMSET(&pNode->stInfo, 0, sizeof(pNode->stInfo));
    SO_MEMSET(pNode->addr, 0, (size_t)s32Len);

    /* copy node info, len(4bytes)+data(len bytes) */

    (mt_void)_SO_InitNode(pstInfo, pNode);

    list_add_tail(&pNode->list, &pSoQueue->list);
    pSoQueue->num++;
    pSoQueue->s32BufferSize += s32Len;

    SO_QUEUE_UNLOCK();

    return MT_SUCCESS;
}
/*lint +e429*/

mt_s32 SO_QueueGet( SO_QUEUE_HANDLE handle, SO_INFO_S *pstInfo )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;
    SO_NODE_S  *pNode = NULL;
    mt_s32 s32Len = 0;

    SO_RETURN(NULL == pSoQueue, MT_FAILURE, "");
    SO_RETURN(NULL == pstInfo, MT_FAILURE, "");

    SO_QUEUE_LOCK();

    if ( !list_empty(&pSoQueue->list) )
    {
        pNode = list_entry(pSoQueue->list.next, SO_NODE_S, list);
        list_del(&pNode->list);
        pSoQueue->num--;

        SO_MEMCPY(pstInfo, &pNode->stInfo, sizeof(SO_INFO_S));

        s32Len = _SO_GetBufLen(pstInfo);
        pSoQueue->s32BufferSize -= s32Len;

        if (MT_UNF_SUBTITLE_BITMAP == pNode->stInfo.eType)
        {
            pstInfo->unSubtitleParam.stGfx.pu8PixData = pNode->addr;
        }
        else if (MT_UNF_SUBTITLE_TEXT == pNode->stInfo.eType)
        {
            pstInfo->unSubtitleParam.stText.pu8Data = pNode->addr;
        }
        else if (MT_UNF_SUBTITLE_ASS == pNode->stInfo.eType)
        {
            pstInfo->unSubtitleParam.stAss.pu8EventData = pNode->addr;
        }

        SO_QUEUE_UNLOCK();

        return MT_SUCCESS;
    }

    SO_QUEUE_UNLOCK();

    return MT_FAILURE;
}

mt_s32 SO_QueueGetNodeInfoNotDel( SO_QUEUE_HANDLE handle, SO_INFO_S *pstInfo )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;
    SO_NODE_S  *pNode = NULL;

    SO_RETURN(NULL == pSoQueue, MT_FAILURE, "");
    SO_RETURN(NULL == pstInfo, MT_FAILURE, "");

    SO_QUEUE_LOCK();

    if ( !list_empty(&pSoQueue->list) )
    {
        pNode = list_entry(pSoQueue->list.next, SO_NODE_S, list);

        SO_MEMCPY(pstInfo, &pNode->stInfo, sizeof(SO_INFO_S));

        if (MT_UNF_SUBTITLE_BITMAP == pNode->stInfo.eType)
        {
            pstInfo->unSubtitleParam.stGfx.pu8PixData = pNode->addr;
        }
        else if (MT_UNF_SUBTITLE_TEXT == pNode->stInfo.eType)
        {
            pstInfo->unSubtitleParam.stText.pu8Data = pNode->addr;
        }
        else if (MT_UNF_SUBTITLE_ASS == pNode->stInfo.eType)
        {
            pstInfo->unSubtitleParam.stAss.pu8EventData = pNode->addr;
        }

        SO_QUEUE_UNLOCK();

        return MT_SUCCESS;
    }

    SO_QUEUE_UNLOCK();

    return MT_FAILURE;
}

mt_s32 SO_QueueFree( SO_QUEUE_HANDLE handle, SO_INFO_S *pstInfo )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;
    mt_u8 *pu8Data = NULL;

    SO_RETURN(NULL == pSoQueue || NULL == pstInfo, MT_FAILURE, "");

    if (MT_UNF_SUBTITLE_BITMAP == pstInfo->eType)
    {
        pu8Data = pstInfo->unSubtitleParam.stGfx.pu8PixData;
    }
    else if (MT_UNF_SUBTITLE_TEXT == pstInfo->eType)
    {
        pu8Data = pstInfo->unSubtitleParam.stText.pu8Data;
    }
    else if (MT_UNF_SUBTITLE_ASS == pstInfo->eType)
    {
        pu8Data = pstInfo->unSubtitleParam.stAss.pu8EventData;
    }

    SO_RETURN(NULL == pu8Data, MT_FAILURE, "");

    SO_QUEUE_LOCK();
    pu8Data = pu8Data - SO_NODE_SIZE;
    SO_FREE(pu8Data);
    SO_QUEUE_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 SO_QueueRemove( SO_QUEUE_HANDLE handle )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;
    SO_NODE_S  *pNode;
    SO_NODE_S  *n;

    SO_RETURN(pSoQueue == NULL, MT_FAILURE, "");

    SO_QUEUE_LOCK();

    list_for_each_entry_safe(pNode, n, &pSoQueue->list, list) /*lint !e10 !e26 !e40 !e48 !e64 !e530 !e666 !e1013 !e1055*/
    {
        list_del(&pNode->list);
        SO_FREE(pNode);
    }

    pSoQueue->num = 0;
    pSoQueue->s32BufferSize = 0;

    SO_QUEUE_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 SO_QueueReset( SO_QUEUE_HANDLE handle )
{
    return SO_QueueRemove(handle);
}

mt_s32 SO_QueueReset_ByPts( SO_QUEUE_HANDLE handle, mt_s64 s64Pts )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;
    SO_NODE_S  *pNode;
    SO_NODE_S  *n;

    SO_RETURN(pSoQueue == NULL, MT_FAILURE, "");

    SO_QUEUE_LOCK();

    list_for_each_entry_safe(pNode, n, &pSoQueue->list, list) /*lint !e10 !e26 !e40 !e48 !e64 !e530 !e666 !e1013 !e1055*/
    {
        if ((MT_UNF_SUBTITLE_BITMAP == pNode->stInfo.eType && pNode->stInfo.unSubtitleParam.stGfx.s64Pts < s64Pts)
            || (MT_UNF_SUBTITLE_TEXT == pNode->stInfo.eType && pNode->stInfo.unSubtitleParam.stText.s64Pts < s64Pts)
            || (MT_UNF_SUBTITLE_ASS == pNode->stInfo.eType && pNode->stInfo.unSubtitleParam.stAss.s64Pts < s64Pts))
        {
            pSoQueue->num --;
            if (MT_UNF_SUBTITLE_BITMAP == pNode->stInfo.eType && pNode->stInfo.unSubtitleParam.stGfx.s64Pts < s64Pts)
            {
                pSoQueue->s32BufferSize -= (mt_s32)pNode->stInfo.unSubtitleParam.stGfx.u32Len;
            }
            else if (MT_UNF_SUBTITLE_TEXT == pNode->stInfo.eType && pNode->stInfo.unSubtitleParam.stText.s64Pts < s64Pts)
            {
                pSoQueue->s32BufferSize -= (mt_s32)pNode->stInfo.unSubtitleParam.stText.u32Len;
            }
            else if (MT_UNF_SUBTITLE_ASS == pNode->stInfo.eType && pNode->stInfo.unSubtitleParam.stAss.s64Pts < s64Pts)
            {
                pSoQueue->s32BufferSize -= (mt_s32)pNode->stInfo.unSubtitleParam.stAss.u32FrameLen;
            }

            list_del(&pNode->list);
            SO_FREE(pNode);
        }
    }

    SO_QUEUE_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 SO_QueueNum( SO_QUEUE_HANDLE handle )
{
    SO_QUEUE_S *pSoQueue = (SO_QUEUE_S*)handle;
    mt_s32 num = 0;

    SO_RETURN(NULL == pSoQueue, MT_FAILURE, "");

    SO_QUEUE_LOCK();
    num = pSoQueue->num;
    SO_QUEUE_UNLOCK();

    return num;
}

