/**
 \file
 \brief subtitle output
 \author 
 \version 1.0
 \author
 \date 
 */

#include <pthread.h>
#include "so_queue.h"
#include "mt_unf_subtouput.h"
#include "mt_common.h"

#define SO_NORMAL_MAX_DISPLAY_TIME        (10 * 1000)  /**< 10s */
#define SO_NORMAL_MIN_DISPLAY_TIME        (2 * 1000)  /**< ss */
#define SO_PLAY_TIME_JUMP_JUDGE           (5 * 1000)  /**< 5s */
#define SO_TRHEAD_SLEEP_TIME          (10)
#define SO_TIME_OFFSET                (10)
#define SO_QUEUE_RESET_CHECK_TIME     (100)
#define SO_QUEUE_MAX_CLEAR_NODE_NUM   (512)
#define SO_SEND_OFFSET                (20)  /**< 20ms */
#define SO_CALLBACK_LOCK()            pthread_mutex_lock(&pstMember->stMutex);
#define SO_CALLBACK_UNLOCK()          pthread_mutex_unlock(&pstMember->stMutex);
#define SO_QUEUE_RESET_LOCK()         pthread_mutex_lock(&pstMember->stQueueResetMutex);
#define SO_QUEUE_RESET_UNLOCK()       pthread_mutex_unlock(&pstMember->stQueueResetMutex);

static MT_BOOL s_bSoInit = MT_FALSE;



typedef struct tagSO_CLEAR_NODE_S
{
    MT_UNF_SO_CLEAR_PARAM_S stClearParam;
    mt_u32 u32Duration;
    mt_s64 s64NodePts;
} SO_CLEAR_NODE_S;

/************************ Structure Definition **************************/
/** so module members */
typedef struct tagSO_MEMBER_S
{
    MT_UNF_SO_GETPTS_FN  pfnGetPts;    /**< get local time */
    MT_UNF_SO_ONDRAW_FN  pfnOnDraw;    /**< draw subtitle */
    MT_UNF_SO_ONCLEAR_FN pfnOnClear;   /**< clear subtitle, called after pfnOnDraw */
    MT_HANDLE hSurface;                /**< surface for subtitle output */
    MT_BOOL   bThreadExit;             /**< flag of so exit */
    MT_BOOL   bQueueReset;             /**< reset subtitle queue */
    mt_s64    s64ResetPts;             /**< reset subtitle queue by pts */
    ulong    u32PtsUserData;          /**< user data of pfnGetPts */
    ulong    u32DrawUserData;         /**< user data of pfnOnDraw and pfnOnClear */
    ulong    u32Font;                 /**< font of subtitle output */
    mt_s64    s64PtsOffset;
    mt_u32    u32Color;                /**< color of subtitle output */
    mt_u32    x, y;                    /**< position of subtitle output */
    SO_QUEUE_HANDLE queuehdl;          /**< subtitle queue handle */
    pthread_t hTaskId;                 /**< main task handle */
    pthread_mutex_t stMutex;           /**< main task mutex */
    pthread_mutex_t stQueueResetMutex; /**< mutex of bQueueReset */
    pthread_attr_t  struAttr;          /**< main task attribute */
    mt_u32 u32ReadIndx;                /**< read pos of node list */
    mt_u32 u32WriteIndx;               /**< write pos of node list */
    mt_s64 s64LastPts;                 /**< Last subt frame pts */
    SO_CLEAR_NODE_S astClearNodeList[SO_QUEUE_MAX_CLEAR_NODE_NUM];
} SO_MEMBER_S;

static mt_s32 SO_DrawSubtitle(const MT_UNF_SO_SUBTITLE_INFO_S *pstInfo)
{
    return MT_SUCCESS;
}

static mt_s32 SO_GetNodeClearInfo(const SO_INFO_S *pstInfo, MT_UNF_SO_CLEAR_PARAM_S *pstClearParam)
{
    if (MT_UNF_SUBTITLE_BITMAP == pstInfo->eType)
    {
        pstClearParam->x = pstInfo->unSubtitleParam.stGfx.x;
        pstClearParam->y = pstInfo->unSubtitleParam.stGfx.y;
        pstClearParam->w = pstInfo->unSubtitleParam.stGfx.w;
        pstClearParam->h = pstInfo->unSubtitleParam.stGfx.h;
    }
    else if (MT_UNF_SUBTITLE_TEXT == pstInfo->eType)
    {
        pstClearParam->x = pstInfo->unSubtitleParam.stText.x;
        pstClearParam->y = pstInfo->unSubtitleParam.stText.y;
        pstClearParam->w = pstInfo->unSubtitleParam.stText.w;
        pstClearParam->h = pstInfo->unSubtitleParam.stText.h;
    }
/*    else if (MT_UNF_SUBTITLE_ASS == pstInfo->eType)
    {
        pstClearParam->x = 0;
        pstClearParam->y = 0;
        pstClearParam->w = 0;
        pstClearParam->h = 0;
    }*/
    else
    {
        pstClearParam->x = 0;
        pstClearParam->y = 0;
        pstClearParam->w = 0;
        pstClearParam->h = 0;
    }

    return MT_SUCCESS;
}

static mt_s32 SO_GetNodePts(const SO_INFO_S *pstInfo, mt_s64 *ps64Pts, mt_u32 *pu32Duration)
{
    if (MT_UNF_SUBTITLE_BITMAP == pstInfo->eType)
    {
        *ps64Pts = pstInfo->unSubtitleParam.stGfx.s64Pts;
        *pu32Duration = pstInfo->unSubtitleParam.stGfx.u32Duration;
    }
    else if (MT_UNF_SUBTITLE_TEXT == pstInfo->eType)
    {
        *ps64Pts = pstInfo->unSubtitleParam.stText.s64Pts;
        *pu32Duration = pstInfo->unSubtitleParam.stText.u32Duration;
    }
    else if (MT_UNF_SUBTITLE_ASS == pstInfo->eType)
    {
        *ps64Pts = pstInfo->unSubtitleParam.stAss.s64Pts;
        *pu32Duration = pstInfo->unSubtitleParam.stAss.u32Duration;
    }
    else
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 SO_InsertToClearList(SO_MEMBER_S *pstMember, const SO_INFO_S *pstSoInfo, mt_s64 s64NodePts, mt_u32 u32Duration)
{
    mt_u32 u32NodeNum = 0;

    if (pstMember->u32WriteIndx < SO_QUEUE_MAX_CLEAR_NODE_NUM)
    {
        pstMember->astClearNodeList[pstMember->u32WriteIndx].s64NodePts  = s64NodePts;
        pstMember->astClearNodeList[pstMember->u32WriteIndx].u32Duration = u32Duration;
        (mt_void)SO_GetNodeClearInfo(pstSoInfo, &pstMember->astClearNodeList[pstMember->u32WriteIndx].stClearParam);
        pstMember->u32WriteIndx++;
    }
    else
    {
        if (pstMember->u32ReadIndx < pstMember->u32WriteIndx)
        {
            u32NodeNum = pstMember->u32WriteIndx - pstMember->u32ReadIndx;

            SO_MEMMOVE(&pstMember->astClearNodeList[0], &pstMember->astClearNodeList[pstMember->u32ReadIndx], u32NodeNum);
            pstMember->u32ReadIndx  = 0;
            pstMember->u32WriteIndx = pstMember->u32ReadIndx + u32NodeNum;
        }
        else
        {
            pstMember->u32ReadIndx  = 0;
            pstMember->u32WriteIndx = 0;
            SO_MEMSET(pstMember->astClearNodeList, 0, sizeof(SO_CLEAR_NODE_S) * SO_QUEUE_MAX_CLEAR_NODE_NUM);
        }

        pstMember->astClearNodeList[pstMember->u32WriteIndx].s64NodePts  = s64NodePts;
        pstMember->astClearNodeList[pstMember->u32WriteIndx].u32Duration = u32Duration;
        (mt_void)SO_GetNodeClearInfo(pstSoInfo, &pstMember->astClearNodeList[pstMember->u32WriteIndx].stClearParam);
        pstMember->u32WriteIndx++;
        
    }

    return MT_SUCCESS;
}

static mt_s32 SO_ClearNode(SO_MEMBER_S *pstMember, MT_UNF_SO_GETPTS_FN pfnGetPts, MT_UNF_SO_ONCLEAR_FN pfnOnClear)
{
    mt_s64 s64NodePts = 0, s64CurPts = 0;
    mt_u32 u32Duration = 0;

    if (NULL == pfnGetPts || NULL == pfnOnClear)
    {
        return MT_FAILURE;
    }

    if (pstMember->u32WriteIndx > pstMember->u32ReadIndx)
    {
        u32Duration = pstMember->astClearNodeList[pstMember->u32ReadIndx].u32Duration;
        s64NodePts  = pstMember->astClearNodeList[pstMember->u32ReadIndx].s64NodePts;

        (mt_void)pfnGetPts(pstMember->u32PtsUserData, &s64CurPts);

        if (MT_UNF_SO_NO_PTS != (mt_s32)s64CurPts
            && ((s64CurPts - s64NodePts) >= (u32Duration - SO_TIME_OFFSET) || u32Duration < SO_TIME_OFFSET))
        {
            // clear this subtitle
            // l00192899 add current time in stClearParam,check it while clear pgs sub.
            pstMember->astClearNodeList[pstMember->u32ReadIndx].stClearParam.s64ClearTime = s64CurPts;
            pstMember->astClearNodeList[pstMember->u32ReadIndx].stClearParam.s64NodePts = s64NodePts;
            pstMember->astClearNodeList[pstMember->u32ReadIndx].stClearParam.u32Duration = u32Duration;
            if (NULL != pfnOnClear)
            {
                (mt_void)pfnOnClear(pstMember->u32DrawUserData,
                    (mt_void*)&pstMember->astClearNodeList[pstMember->u32ReadIndx].stClearParam);
            }

            pstMember->u32ReadIndx++;
        }
    }

    return MT_SUCCESS;
}

static mt_s32 SO_SyncOutput(SO_MEMBER_S *pstMember, MT_UNF_SO_GETPTS_FN pfnGetPts,
                                   MT_UNF_SO_ONDRAW_FN  pfnOnDraw,
                                   MT_UNF_SO_ONCLEAR_FN pfnOnClear,
                                   const SO_INFO_S *pstSoInfo)
{
    mt_s32  s32Ret = 0;
    mt_s64  s64CurPts = 0, s64NodePts = 0, s64CurNodePts = 0;
    mt_u32  u32Duration = 0, u32NodeDuration = 0;

    SO_INFO_S stNextSubInfo;

    for (;;)
    {
        if (MT_TRUE == pstMember->bThreadExit || MT_TRUE == pstMember->bQueueReset)
        {
            //pstMember->bQueueReset = MT_FALSE;
            return MT_SUCCESS;
        }
        (mt_void)SO_ClearNode(pstMember, pfnGetPts, pstMember->pfnOnClear);
        (mt_void)pfnGetPts(pstMember->u32PtsUserData, &s64CurPts);
        (mt_void)SO_GetNodePts(pstSoInfo, &s64CurNodePts, &u32NodeDuration);

        s64CurNodePts += pstMember->s64PtsOffset;

        /* CurPts less than LastPts, need to reset SO */
        if (((s64CurPts + SO_PLAY_TIME_JUMP_JUDGE) < pstMember->s64LastPts) && (MT_UNF_SO_NO_PTS != s64CurPts))
        {
            pstMember->bQueueReset = MT_TRUE;
            pstMember->s64LastPts = 0;
            return MT_SUCCESS;
        }
        pstMember->s64LastPts = s64CurPts;

        /* local time is invalid, do not output subtitle */

        if (MT_UNF_SO_NO_PTS == (mt_s32)s64CurPts)
        {
            SO_SLEEP(SO_TIME_OFFSET);
            continue;
        }
        
        if ((s64CurPts < s64CurNodePts) /*&& (s64CurNodePts - s64CurPts > SO_SEND_OFFSET)*/)
        {
            if((s64CurNodePts - s64CurPts) > SO_NORMAL_MAX_DISPLAY_TIME)
            {
                /* pts of subtitle is larger than local time */
                break;//abnormal subtitle pts,don't wait and display right now.
            }            
            else if((s64CurNodePts - s64CurPts) > SO_SEND_OFFSET)
            {
                /* pts of subtitle is larger than local time */
                SO_SLEEP(SO_TIME_OFFSET);
                continue;
            }
            else
            {
                break;//display time is got
            }
        }
        else
        {
			//local time is more than node pts,don't wait and display right now.
            break;
        }
    }

    /* pts of subtitle is less than or equal to local time */

    if (0 != u32NodeDuration)
    {
        /* local time is larger than (pts + duration), skip this subtitle */

        //SO_RETURN((s64CurPts > s64CurNodePts + u32NodeDuration), MT_SUCCESS, NULL); //don't jump any subtitle,still display this subtitle.//gavin.s change
        u32Duration = (mt_u32)((s64CurNodePts + u32NodeDuration) - s64CurPts);
        u32Duration = u32Duration > u32NodeDuration ? u32NodeDuration : u32Duration;
    }
    else
    {
        /* this subtitle has no duration, get pts of next subtitle in queue */

        SO_MEMSET(&stNextSubInfo, 0, sizeof(stNextSubInfo));
        s32Ret = SO_QueueGetNodeInfoNotDel(pstMember->queuehdl, &stNextSubInfo);

        if (MT_SUCCESS != s32Ret)
        {
            /* default duration */

            u32Duration = SO_NORMAL_MIN_DISPLAY_TIME;
        }
        else
        {
            (mt_void)SO_GetNodePts(&stNextSubInfo, &s64NodePts, &u32NodeDuration);
            s64NodePts += pstMember->s64PtsOffset;

            /* local time is larger than this subtitle, skip last subtitle */

            //SO_RETURN((s64CurPts + SO_TIME_OFFSET >= s64NodePts), MT_SUCCESS, NULL);//don't jump any subtitle,still display this subtitle. //gavin.s change
            u32Duration = (mt_u32)(s64NodePts - s64CurPts - SO_TIME_OFFSET);
            u32Duration = u32Duration > SO_NORMAL_MAX_DISPLAY_TIME ? SO_NORMAL_MAX_DISPLAY_TIME : u32Duration;
        }
    }
#if 0
    /* clear last subtitle */

    if (NULL != pfnOnClear)
    {
        (mt_void)pfnOnClear(pstMember->u32DrawUserData);
    }
#endif

    if (MT_UNF_SUBTITLE_BITMAP == pstSoInfo->eType && MT_UNF_SO_DISP_MSG_ERASE == pstSoInfo->unSubtitleParam.stGfx.enMsgType)
    {
        pstMember->u32ReadIndx = 0;
        pstMember->u32WriteIndx = 0;
        SO_MEMSET(pstMember->astClearNodeList, 0, sizeof(SO_CLEAR_NODE_S) * SO_QUEUE_MAX_CLEAR_NODE_NUM);

        /* if reset clear list, insert a empty node */

        stNextSubInfo.eType = MT_UNF_SUBTITLE_BUTT;
        SO_InsertToClearList(pstMember, &stNextSubInfo, 0, SO_TIME_OFFSET);
    }

    if (MT_UNF_SUBTITLE_BITMAP == pstSoInfo->eType && MT_UNF_SO_DISP_MSG_NORM == pstSoInfo->unSubtitleParam.stGfx.enMsgType)
    {
        /* if no pix data in normal display message, just need to clear OSD only */
        if (0 == pstSoInfo->unSubtitleParam.stGfx.x &&
            0 == pstSoInfo->unSubtitleParam.stGfx.y &&
            0 == pstSoInfo->unSubtitleParam.stGfx.w &&
            0 == pstSoInfo->unSubtitleParam.stGfx.h )
        {
            pstMember->u32ReadIndx = 0;
            pstMember->u32WriteIndx = 0;
            SO_MEMSET(pstMember->astClearNodeList, 0, sizeof(SO_CLEAR_NODE_S) * SO_QUEUE_MAX_CLEAR_NODE_NUM);

            /* if reset clear list, insert a empty node */

            stNextSubInfo.eType = MT_UNF_SUBTITLE_BUTT;
            SO_InsertToClearList(pstMember, &stNextSubInfo, 0, SO_TIME_OFFSET);
            (mt_void)SO_ClearNode(pstMember, pfnGetPts, pstMember->pfnOnClear);
            return MT_SUCCESS;
        }
    }
    (mt_void)SO_ClearNode(pstMember, pfnGetPts, pstMember->pfnOnClear);
    if (NULL != pfnOnDraw)
    {
        /* use pfnOnDraw first */
        (mt_void)pfnOnDraw(pstMember->u32DrawUserData, pstSoInfo, NULL);
    }
    else
    {
        /* not support */
        (mt_void)SO_DrawSubtitle(pstSoInfo);
    }

    /* insert node into clear list */
    SO_InsertToClearList(pstMember, pstSoInfo, /*s64CurNodePts*/s64CurPts, u32Duration);

#if 0
    /* clear node from list */
    if (pstMember->u32WriteIndx > pstMember->u32ReadIndx)
    {
        u32Duration = pstMember->astClearNodeList[pstMember->u32ReadIndx].u32Duration;
        s64NodePts  = pstMember->astClearNodeList[pstMember->u32ReadIndx].s64NodePts;

        (mt_void)pfnGetPts(pstMember->u32PtsUserData, &s64CurPts);

        if (MT_UNF_SO_NO_PTS != (mt_u32)s64CurPts
            && (s64CurPts - s64NodePts >= u32Duration - SO_TIME_OFFSET || u32Duration < SO_TIME_OFFSET))
        {
            /* clear this subtitle */

            if (NULL != pfnOnClear)
            {
                (mt_void)pfnOnClear(pstMember->u32DrawUserData,
                    (mt_void*)&pstMember->astClearNodeList[pstMember->u32ReadIndx].stClearPos);
            }

            pstMember->u32ReadIndx++;
        }
    }
#endif
#if 0
    /* clear node from list */
    s32DisTotalTime = 0;

    while ((u32Duration >= SO_TIME_OFFSET) && ((mt_u32)s32DisTotalTime < (u32Duration - SO_TIME_OFFSET)))
    {
        if (MT_TRUE == pstMember->bThreadExit || MT_TRUE == pstMember->bQueueReset)
        {
            //pstMember->bQueueReset = MT_FALSE;
            break;
        }

        /* check the time of outputting next subtitle */

        (mt_void)pfnGetPts(pstMember->u32PtsUserData, &s64CurPts);
        s32Ret = SO_QueueGetNodeInfoNotDel(pstMember->queuehdl, &stNextSubInfo);
        (mt_void)SO_GetNodePts(&stNextSubInfo, &s64NodePts, &u32NodeDuration);
        s64NodePts += pstMember->s64PtsOffset;

        if ((MT_SUCCESS == s32Ret) && (s64NodePts - s64CurPts <= SO_TIME_OFFSET))
        {
            break;
        }

        SO_SLEEP(SO_TIME_OFFSET);
        s32DisTotalTime += SO_TIME_OFFSET;
    }

    if (NULL != pfnOnClear)
    {
        (mt_void)pfnOnClear(pstMember->u32DrawUserData);
    }
#endif

    return MT_SUCCESS;
}

static mt_void* SO_ThreadMainFunction(mt_void *pArg)
{
    mt_s32  s32Ret = MT_SUCCESS;
    MT_UNF_SO_GETPTS_FN  pfnGetPts = NULL;
    //MT_UNF_SO_ONDRAW_FN  pfnOnDraw = NULL;
    MT_UNF_SO_ONCLEAR_FN pfnOnClear = NULL;

    SO_INFO_S stSubInfo;
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)pArg;

    if (NULL == pstMember)
    {
        return NULL;
    }
    mt_set_pthread_name(__FUNCTION__);
    for (;;)
    {
        if (MT_TRUE == pstMember->bThreadExit)
        {
            break;
        }

        SO_QUEUE_RESET_LOCK();

        if (MT_TRUE == pstMember->bQueueReset)
        {
            if (pstMember->s64ResetPts)
            {
                s32Ret = SO_QueueReset_ByPts(pstMember->queuehdl, pstMember->s64ResetPts);
                pstMember->s64ResetPts = 0;
            }
            else
            {
                s32Ret = SO_QueueReset(pstMember->queuehdl);
            }
            pstMember->bQueueReset = MT_FALSE;
            pstMember->u32ReadIndx = 0;
            pstMember->u32WriteIndx = 0;
            SO_MEMSET(pstMember->astClearNodeList, 0, sizeof(SO_CLEAR_NODE_S) * SO_QUEUE_MAX_CLEAR_NODE_NUM);

            /* if reset clear list, insert a empty node */

            stSubInfo.eType = MT_UNF_SUBTITLE_BUTT;
            SO_InsertToClearList(pstMember, &stSubInfo, 0, SO_TIME_OFFSET);
        }

        SO_QUEUE_RESET_UNLOCK();

        SO_CALLBACK_LOCK();
        pfnGetPts  = pstMember->pfnGetPts;
        //pfnOnDraw  = pstMember->pfnOnDraw;
        pfnOnClear = pstMember->pfnOnClear;
        SO_CALLBACK_UNLOCK();
        (mt_void)SO_ClearNode(pstMember, pfnGetPts, pfnOnClear);

        SO_MEMSET(&stSubInfo, 0, sizeof(stSubInfo));
        s32Ret = SO_QueueGet(pstMember->queuehdl, &stSubInfo);

        if (MT_SUCCESS != s32Ret)
        {
            SO_SLEEP(SO_TRHEAD_SLEEP_TIME);
            continue;
        }
        #if 0
        /* if not set pfnGetPts, skip all subtitle */
        if (NULL == pfnGetPts)
        {
            (mt_void)SO_QueueFree(pstMember->queuehdl, &stSubInfo);
            SO_SLEEP(SO_TRHEAD_SLEEP_TIME);
            continue;
        }
        #endif
        SO_QUEUE_RESET_LOCK();
        s32Ret = SO_SyncOutput(pstMember, pfnGetPts, pstMember->pfnOnDraw, pfnOnClear, &stSubInfo);
        (mt_void)SO_QueueFree(pstMember->queuehdl, &stSubInfo);
        SO_QUEUE_RESET_UNLOCK();
        SO_SLEEP(SO_TRHEAD_SLEEP_TIME);
    }

    return NULL;
}

mt_s32 MT_UNF_SO_Init(mt_void)
{
    if (MT_TRUE == s_bSoInit)
    {
        return MT_SUCCESS;
    }

    s_bSoInit = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_DeInit(mt_void)
{
    if (MT_FALSE == s_bSoInit)
    {
        return MT_SUCCESS;
    }

    s_bSoInit = MT_FALSE;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_Create(MT_HANDLE *phdl)
{
    mt_s32 s32Ret = 0;
    SO_MEMBER_S *pstMember = NULL;

    SO_RETURN(MT_FALSE == s_bSoInit, MT_FAILURE, "");
    SO_RETURN(NULL == phdl, MT_FAILURE, "");

    pstMember = (SO_MEMBER_S*)SO_MALLOC(sizeof(SO_MEMBER_S));
    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    SO_MEMSET(pstMember, 0, sizeof(SO_MEMBER_S));

    /* create the subtitle queue */

    s32Ret = SO_QueueInit(MT_UNF_SO_MAX_BUFFER_SIZE, MT_UNF_SO_MAX_NODE_NUM, &pstMember->queuehdl);
    SO_CALL_RETURN(MT_SUCCESS != s32Ret, SO_FREE(pstMember), MT_FAILURE);

    pstMember->bThreadExit = MT_FALSE;
    pstMember->bQueueReset = MT_FALSE;
    pstMember->s64ResetPts = 0;
    pstMember->s64PtsOffset = 0;
    pstMember->u32ReadIndx  = 0;
    pstMember->u32WriteIndx = 0;
    (mt_void)pthread_mutex_init(&pstMember->stQueueResetMutex, NULL);
    (mt_void)pthread_mutex_init(&pstMember->stMutex, NULL);
    (mt_void)pthread_attr_init(&pstMember->struAttr);

    /* create the main task */
    pthread_attr_setdetachstate(&pstMember->struAttr, PTHREAD_CREATE_JOINABLE);
    s32Ret = pthread_create(&pstMember->hTaskId, &pstMember->struAttr,
                            SO_ThreadMainFunction, (mt_void*)pstMember);

    if (MT_SUCCESS != s32Ret)
    {
        (mt_void)SO_QueueDeinit(pstMember->queuehdl);
        pthread_attr_destroy(&pstMember->struAttr);
        pthread_mutex_destroy(&pstMember->stMutex);
        pthread_mutex_destroy(&pstMember->stQueueResetMutex);
        SO_FREE(pstMember);
        return MT_FAILURE;
    }

    *phdl = (MT_HANDLE)pstMember;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_Destroy(MT_HANDLE handle)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    pstMember->bQueueReset = MT_TRUE;
    pstMember->bThreadExit = MT_TRUE;
    pthread_join(pstMember->hTaskId, NULL);

    pthread_attr_destroy(&pstMember->struAttr);
    pthread_mutex_destroy(&pstMember->stMutex);
    pthread_mutex_destroy(&pstMember->stQueueResetMutex);

    (mt_void)SO_QueueDeinit(pstMember->queuehdl);
    SO_FREE(pstMember);

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_RegGetPtsCb(MT_HANDLE handle, MT_UNF_SO_GETPTS_FN pfnCallback, ulong u32UserData)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN((NULL == pstMember || NULL == pfnCallback), MT_FAILURE, "");

    SO_CALLBACK_LOCK();
    pstMember->pfnGetPts      = pfnCallback;
    pstMember->u32PtsUserData = u32UserData;
    SO_CALLBACK_UNLOCK();;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_RegOnDrawCb(MT_HANDLE handle, MT_UNF_SO_ONDRAW_FN pfnOnDraw,
                                          MT_UNF_SO_ONCLEAR_FN pfnOnClear, ulong u32UserData)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN((NULL == pstMember || NULL == pfnOnDraw), MT_FAILURE, "");

    SO_CALLBACK_LOCK();
    pstMember->pfnOnDraw       = pfnOnDraw;
    pstMember->pfnOnClear      = pfnOnClear;
    pstMember->u32DrawUserData = u32UserData;
    SO_CALLBACK_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_SetDrawSurface(MT_HANDLE handle, MT_HANDLE hSurfaceHandle)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN((NULL == pstMember || 0 == hSurfaceHandle), MT_FAILURE, "");

    SO_CALLBACK_LOCK();
    pstMember->hSurface = hSurfaceHandle;
    SO_CALLBACK_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_SetFont(MT_HANDLE handle, MT_HANDLE hFont)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    pstMember->u32Font = (MT_HANDLE)hFont;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_SetOffset(MT_HANDLE handle, mt_s64 s64OffsetMs)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    pstMember->s64PtsOffset = s64OffsetMs;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_SetColor(MT_HANDLE handle, mt_u32 u32Color)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    pstMember->u32Color = u32Color;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_SetPos(MT_HANDLE handle, mt_u32 u32x, mt_u32 u32y)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    pstMember->x = u32x;
    pstMember->y = u32y;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_GetSubNumInBuff(MT_HANDLE handle, mt_u32 *pu32SubNum)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN(NULL == pstMember || NULL == pu32SubNum, MT_FAILURE, "");

    *pu32SubNum = (mt_u32)SO_QueueNum(pstMember->queuehdl);

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SO_ResetSubBuf(MT_HANDLE handle)
{
    mt_s32 s32Ret = 0;
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;
    mt_u32 i = 0;

    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    //SO_QUEUE_RESET_LOCK();
    pstMember->bQueueReset = MT_TRUE;
    pstMember->s64LastPts = 0;
    //SO_QUEUE_RESET_UNLOCK();

    while (i < SO_QUEUE_RESET_CHECK_TIME)
    {
        if (MT_FALSE == pstMember->bQueueReset)
        {
            break;
        }

        i++;
        SO_SLEEP(SO_TIME_OFFSET);
    }

    return s32Ret;
}

mt_s32 MT_UNF_SO_ResetSubBuf_ByPts(MT_HANDLE handle, mt_s64 s64Pts)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN(NULL == pstMember, MT_FAILURE, "");

    pstMember->s64ResetPts = s64Pts;
    return MT_UNF_SO_ResetSubBuf(handle);
}

mt_s32 MT_UNF_SO_SendData(MT_HANDLE handle, const MT_UNF_SO_SUBTITLE_INFO_S *pstSubInfo, mt_u32 u32TimeOut)
{
    SO_MEMBER_S *pstMember = (SO_MEMBER_S*)handle;

    SO_RETURN((NULL == pstMember || NULL == pstSubInfo), MT_FAILURE, "");

    return SO_QueuePut(pstMember->queuehdl, (const SO_INFO_S*)pstSubInfo);
}

