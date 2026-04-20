/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "cc_debug.h"
#include "cc_timer.h"

#define MAX_CC_TIMER ((MT_S32)CCTIMER_ID_BUTT)
#define CC_USEC_PER_TICK  (1000)

typedef struct tagCCTIMER_HOOK_S
{
    MT_BOOL bStart;
    MT_VOID (*pfnHook)(MT_U32);
    MT_U32 u32Args;
    MT_U32 u32Interval;
    MT_U32 u32Escape;
    CCTIMER_MODE_E enMode;
} CCTIMER_HOOK_S;

typedef struct tagCCTIMER_ATTR_S
{
    pthread_t thread;
    pthread_mutex_t mutex;
    MT_BOOL bRun;
    CCTIMER_HOOK_S astHook[MAX_CC_TIMER];
} CCTIMER_ATTR_S;

static CCTIMER_ATTR_S *s_pstCCTimer = MT_NULL;

static MT_VOID _cctimer_trigger(CCTIMER_HOOK_S *pstHook, MT_U32 escape)
{
    if (MT_TRUE == pstHook->bStart)
    {
        pstHook->u32Escape += escape;
        if(pstHook->u32Escape >= pstHook->u32Interval)
        {
            if (MT_NULL != pstHook->pfnHook)
            {
                pstHook->pfnHook(pstHook->u32Args);
            }
            if (TIMER_MODE_ONE_SHOOT == pstHook->enMode)
            {
                pstHook->bStart = MT_FALSE;
            }
            pstHook->u32Escape = 0;
        }
    }
}

static MT_VOID * _cctimer_task(MT_VOID *pvArgs)
{
    CCTIMER_ATTR_S *pstTimer = (CCTIMER_ATTR_S *)pvArgs;
    struct timeval tickval;
    MT_S32 i = 0;

    if (MT_NULL == pstTimer)
    {
        MT_ERR_CC("param is error\n");
        return NULL;
    }

    while (MT_TRUE == pstTimer->bRun)
    {
        tickval.tv_sec = 0;
        tickval.tv_usec = CC_USEC_PER_TICK;

        (MT_VOID)select(0, NULL, NULL, NULL, &tickval); /* I don't care if it is fail */

        for (i = 0; i < MAX_CC_TIMER; i++)
        {
            _cctimer_trigger(pstTimer->astHook + i, 1);
        }
    }

    return NULL;
}

MT_S32 CCTimer_Init(void)
{
    if (MT_NULL == s_pstCCTimer)
    {
        s_pstCCTimer = (CCTIMER_ATTR_S *)MT_MEMCC_MALLOC(sizeof(CCTIMER_ATTR_S));
        if (MT_NULL == s_pstCCTimer)
        {
            MT_ERR_CC("alloc failed\n");
            return MT_FAILURE;
        }
        (MT_VOID)memset(s_pstCCTimer, 0, sizeof(CCTIMER_ATTR_S));

        s_pstCCTimer->bRun = MT_TRUE;

        (MT_VOID)pthread_mutex_init(&s_pstCCTimer->mutex, NULL);

        (MT_VOID)pthread_create(&s_pstCCTimer->thread, MT_NULL, (void * (*)(void *))_cctimer_task, (MT_VOID *)s_pstCCTimer);
    }

    return MT_SUCCESS;
}

MT_S32 CCTimer_DeInit(void)
{
    if (MT_NULL != s_pstCCTimer)
    {
        s_pstCCTimer->bRun = MT_FALSE;

        (MT_VOID)pthread_join(s_pstCCTimer->thread, MT_NULL);

        (MT_VOID)pthread_mutex_destroy(&s_pstCCTimer->mutex);

        MT_MEMCC_FREE(s_pstCCTimer);
        s_pstCCTimer = MT_NULL;
    }

    return MT_SUCCESS;
}

MT_S32 CCTimer_Open(CCTIMER_ID_E enTimerID, MT_VOID (*pfnHook)(MT_U32), MT_U32 u32Args)
{
    MT_S32 s32TimerID = enTimerID;
    CCTIMER_HOOK_S *pstHook = MT_NULL;

    if (s32TimerID >= MAX_CC_TIMER)
    {
        MT_ERR_CC("TimerID : %d is invalid\n", s32TimerID);
        return MT_FAILURE;
    }
    if (MT_NULL == s_pstCCTimer)
    {
        MT_ERR_CC("cctimer not init yet\n");
        return MT_FAILURE;
    }

    pstHook = s_pstCCTimer->astHook + s32TimerID;

    pstHook->pfnHook = pfnHook;
    pstHook->u32Args = u32Args;
    pstHook->bStart = MT_FALSE;

    return MT_SUCCESS;
}

MT_S32 CCTimer_Close(CCTIMER_ID_E enTimerID)
{
    return CCTimer_Stop(enTimerID);
}

MT_S32 CCTimer_Start(CCTIMER_ID_E enTimerID, MT_U32 u32Msec, CCTIMER_MODE_E enMode)
{
    MT_S32 s32TimerID = enTimerID;
    CCTIMER_HOOK_S *pstHook = MT_NULL;

    if (s32TimerID >= MAX_CC_TIMER)
    {
        MT_ERR_CC("TimerID : %d is invalid\n", s32TimerID);
        return MT_FAILURE;
    }
    if (MT_NULL == s_pstCCTimer)
    {
        MT_ERR_CC("cctimer not init yet\n");
        return MT_FAILURE;
    }

    pstHook = s_pstCCTimer->astHook + s32TimerID;

    pstHook->bStart = MT_TRUE;
    pstHook->u32Interval = u32Msec;
    pstHook->u32Escape = 0;
    pstHook->enMode = enMode;

    return MT_SUCCESS;
}

MT_S32 CCTimer_Stop(CCTIMER_ID_E enTimerID)
{
    MT_S32 s32TimerID = enTimerID;
    CCTIMER_HOOK_S *pstHook = MT_NULL;

    if (s32TimerID >= MAX_CC_TIMER)
    {
        MT_ERR_CC("TimerID : %d is invalid\n", s32TimerID);
        return MT_FAILURE;
    }
    if (MT_NULL == s_pstCCTimer)
    {
        MT_ERR_CC("cctimer not init yet\n");
        return MT_FAILURE;
    }

    pstHook = s_pstCCTimer->astHook + s32TimerID;

    pstHook->bStart = MT_FALSE;

    return MT_SUCCESS;
}

