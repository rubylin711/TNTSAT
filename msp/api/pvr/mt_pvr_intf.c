/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <pthread.h>
#include "mt_common.h"
#include "mt_pvr_priv.h"

#include "mt_module_debug.h"
#include "mt_pvr_debug.h"
#include "mt_pvr_play_ctrl.h"
#include "mt_pvr_rec_ctrl.h"
#include "mt_pvr_index.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PVR_MAX_EVENT_NUM (PVR_REC_MAX_CHN_NUM + PVR_REC_START_NUM)

#ifndef MT_SYS_SetLogLevel
#define MT_SYS_SetLogLevel mt_sys_set_log_level
#endif
typedef struct mtPVR_EVENT_CONTEXT_S
{
    MT_UNF_PVR_EVENT_E  enEventType;
    MT_U32              u32ChnId;
    MT_S32              s32EventValue;
}PVR_EVENT_CONTEXT_S;
/* callback function array                                                  */
static eventCallBack g_callBacks[MT_UNF_PVR_EVENT_BUTT];
static MT_VOID *g_callBackArgs[MT_UNF_PVR_EVENT_BUTT]; /* as parameter deliver it to callback  AI7D02612 */

static MT_BOOL   g_bPvrEventRunning = MT_FALSE;
static MT_S32    g_s32PvrEventInitTimes = 0;
static pthread_t g_threadPvrEvent;

/* 0: PLAY, 1~2: REC */
static volatile PVR_EVENT_CONTEXT_S g_stPvrEventTodo[PVR_MAX_EVENT_NUM];

/**
   report event to user about PVR event
*/
STATIC MT_VOID* PVRIntfEventRoutine(MT_VOID *args)
{
    MT_U32              i;
    MT_U32              eventDone;
    MT_UNF_PVR_EVENT_E  enEventType;
    mt_set_pthread_name(__FUNCTION__);
    while (MT_FALSE != g_bPvrEventRunning)
    {
        eventDone = 0;
        for (i = 0; i < PVR_MAX_EVENT_NUM; i++)
        {
            enEventType = g_stPvrEventTodo[i].enEventType;
            if (g_stPvrEventTodo[i].enEventType != MT_UNF_PVR_EVENT_BUTT)
            {
                MT_INFO_PVR("PVR: get a event: chn:%d, type:%#x, value:%#x\n",
                    g_stPvrEventTodo[i].u32ChnId,
                    enEventType,
                    g_stPvrEventTodo[i].s32EventValue);

                if (g_callBacks[enEventType])
                {
                    eventDone++;
                    g_callBacks[enEventType](g_stPvrEventTodo[i].u32ChnId, enEventType,
                                             g_stPvrEventTodo[i].s32EventValue,
                                             g_callBackArgs[enEventType]); /*as parameter deliver it to callback */
                }
                g_stPvrEventTodo[i].enEventType = MT_UNF_PVR_EVENT_BUTT;
            }
        }

        if (0 == eventDone)
        {
            MT_USLEEP(20000);
        }
    }

    return NULL;
}


MT_S32 PVRIntfInitEvent(MT_VOID)
{
    MT_U32          i;
    //pthread_attr_t  ThreadAttr;

    g_s32PvrEventInitTimes++;

    if (1 == g_s32PvrEventInitTimes)
    {
        MT_INFO_PVR("PVR: ===========> Event init.\n");

        for (i = 0; i < MT_UNF_PVR_EVENT_BUTT; i++)
        {
            g_callBacks[i] = MT_NULL;
            g_callBackArgs[i] = MT_NULL;
        }

        for (i = 0; i < PVR_MAX_EVENT_NUM; i++)
        {
            g_stPvrEventTodo[i].enEventType = MT_UNF_PVR_EVENT_BUTT;
            g_stPvrEventTodo[i].u32ChnId    = 0;
            g_stPvrEventTodo[i].s32EventValue = 0;
        }

        //pthread_attr_init(&ThreadAttr);
        //pthread_attr_setdetachstate(&ThreadAttr, PTHREAD_CREATE_DETACHED);
        g_bPvrEventRunning = MT_TRUE;
        //if(pthread_create(&g_threadPvrEvent, &ThreadAttr, PVRIntfEventRoutine, NULL))
        if(pthread_create(&g_threadPvrEvent, NULL, PVRIntfEventRoutine, NULL))
        {
            MT_ERR_PVR("PVR: can NOT Create thread for Event process.\n");
            return MT_ERR_PVR_NOT_INIT;
        }

        MT_INFO_PVR("PVR: ===========> Event init OK.\n");
    }

    return MT_SUCCESS;
}

MT_VOID PVRIntfDeInitEvent(MT_VOID)
{
    MT_U32      i;

    g_s32PvrEventInitTimes--;

    if (g_s32PvrEventInitTimes <= 0)
    {
        for (i = 0; i < PVR_MAX_EVENT_NUM; i++)
        {
            g_stPvrEventTodo[i].enEventType = MT_UNF_PVR_EVENT_BUTT;
            g_stPvrEventTodo[i].u32ChnId    = 0;
            g_stPvrEventTodo[i].s32EventValue = 0;
        }

        g_bPvrEventRunning = MT_FALSE;
        pthread_join(g_threadPvrEvent, NULL);
    }

    return ;
}

MT_S32 PVRIntfEventDirectCall(MT_U32 u32ChnId,MT_UNF_PVR_EVENT_E  enEventType,MT_S32 s32EventValue)
{
    //printf("%s %d u32ChnId=0x%x enEventType=0x%x s32EventValue=0x%x\n",
    //    __FUNCTION__,__LINE__,u32ChnId,enEventType,s32EventValue);
    
    if(enEventType >= MT_UNF_PVR_EVENT_BUTT)
    {
        MT_ERR_PVR("invalid event type:0x%x.\n", enEventType);
        return MT_ERR_PVR_INTF_EVENT_INVAL;
        
    }
    if (g_callBacks[enEventType])
    {
        g_callBacks[enEventType](u32ChnId, enEventType,s32EventValue,g_callBackArgs[enEventType]);
        //printf("%s %d call back done.\n",__FUNCTION__,__LINE__);
    }
    else
    {
        MT_WARN_PVR("event type:0x%x is not register!\n", enEventType);
        return MT_ERR_PVR_INTF_EVENT_NOREG;
    }
     
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_RegisterEvent
 Description     : register callback functions for event
 Input           : EventType  **
                   callBack   **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RegisterEvent(MT_UNF_PVR_EVENT_E enEventType, eventCallBack callBack, MT_VOID *args)
{
    if (MT_NULL == callBack)
    {
        MT_ERR_PVR("callback function is NULL.\n");
        return MT_ERR_PVR_NUL_PTR;
    }

    if (enEventType >= MT_UNF_PVR_EVENT_BUTT)
    {
        MT_ERR_PVR("invalid event type:%d.\n", enEventType);
        return MT_ERR_PVR_INTF_EVENT_INVAL;
    }

    if (MT_NULL != g_callBacks[enEventType])
    {
        MT_ERR_PVR("already register callback for this event:%d.\n", enEventType);
        return MT_ERR_PVR_ALREADY;
    }
    else
    {
        g_callBacks[enEventType] = callBack;
        g_callBackArgs[enEventType] = args;

        return MT_SUCCESS;
    }
}

/*****************************************************************************
 Prototype       : MT_PVR_UnRegisterEvent
 Description     : register callback functions for event
 Input           : EventType  **
                   callBack   **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_E enEventType)
{
    switch ( enEventType )
    {
        case MT_UNF_PVR_EVENT_PLAY_EOF      :
        case MT_UNF_PVR_EVENT_PLAY_SOF      :
        case MT_UNF_PVR_EVENT_PLAY_ERROR    :
        case MT_UNF_PVR_EVENT_PLAY_REACH_REC:
        case MT_UNF_PVR_EVENT_PLAY_RESV     :
        case MT_UNF_PVR_EVENT_REC_DISKFULL  :
        case MT_UNF_PVR_EVENT_REC_ERROR     :
        case MT_UNF_PVR_EVENT_REC_OVER_FIX  :
        case MT_UNF_PVR_EVENT_REC_REACH_PLAY:
        case MT_UNF_PVR_EVENT_REC_DISK_SLOW :
        case MT_UNF_PVR_EVENT_REC_RESV      :
            break;
        default:
            return MT_ERR_PVR_INTF_EVENT_INVAL;
    }

    if (!g_callBacks[enEventType])
    {
        MT_ERR_PVR("Not register event yet.\n");
        return MT_ERR_PVR_INTF_EVENT_NOREG;
    }
    else
    {
        g_callBacks[enEventType] = NULL;
        g_callBackArgs[enEventType] = NULL;

        return MT_SUCCESS;
    }
}

MT_VOID MT_PVR_ConfigDebugInfo(MT_LOG_LEVEL_E enDebugLevel)
{
    MT_SYS_SetLogLevel(MT_ID_PVR, enDebugLevel);
    return;
}

/*****************************************************************************
 Prototype       : MT_PVR_DoEvent
 Description     : the interface, inner module supply envent
 Input           : chn        **channel for suppling event
                   EventType  **event type
                   value      ** the additional parameters for event
 Output          : None
 Return Value    :
 Global Variable
    Read Only    : g_struPvrCallBacks
    Read & Write :
  History
  1.Date         : 2008/4/21
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_VOID PVR_Intf_DoEventCallback(MT_U32 u32ChnID, MT_UNF_PVR_EVENT_E enEventType, MT_S32 s32EnvetValue)
{
    MT_S32 retry = 0;

    /* discard the repeat event*/
    if ((enEventType == g_stPvrEventTodo[u32ChnID].enEventType)
        && (s32EnvetValue == g_stPvrEventTodo[u32ChnID].s32EventValue))
    {
        return ;
    }
    /*
    */

    while (retry < 10) /* send just only one msg in the same channel */
    {
        if (MT_UNF_PVR_EVENT_BUTT == g_stPvrEventTodo[u32ChnID].enEventType)
        {
            g_stPvrEventTodo[u32ChnID].u32ChnId = u32ChnID;
            g_stPvrEventTodo[u32ChnID].enEventType = enEventType;
            g_stPvrEventTodo[u32ChnID].s32EventValue = s32EnvetValue;
            break;
        }
        else
        {
            MT_USLEEP(20000);
            retry++;
        }
    }

    if (retry >= 10)
    {
        MT_ERR_PVR("PVR: lost event: chn:%d, type:%#x(Old:%#x), value:%#x\n",u32ChnID, enEventType, g_stPvrEventTodo[u32ChnID].enEventType, s32EnvetValue);
    }

    return ;
}

MT_S32 MT_PVR_CreateIdxFile(MT_U8* pstTsFileName, MT_U8* pstIdxFileName, MT_UNF_PVR_GEN_IDX_ATTR_S* pAttr)
{
    MT_ERR_PVR("Not support this api yet.\n");
    return MT_ERR_PVR_NOT_SUPPORT;
}

MT_VOID MT_PVR_RemoveFile(const MT_CHAR *pFileName)
{
    MT_CHAR  idxFileName[PVR_MAX_FILENAME_LEN + 5];

    if (pFileName == NULL)
    {
        MT_ERR_PVR("param pFileName is NULL\n");
        return;
    }

    if (strlen(pFileName) >= PVR_MAX_FILENAME_LEN)
    {
        MT_ERR_PVR("file name length too large! must < %d.\n", PVR_MAX_FILENAME_LEN);
        return;
    }
    PVR_REMOVE_FILE64(pFileName);
    snprintf(idxFileName, sizeof(idxFileName),"%s.idx", pFileName);
    remove(idxFileName);
}

MT_S32 MT_PVR_RegisterExtraCallback(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E eExtraCallbackType, ExtraCallBack fCallback, MT_VOID *args)
{
    if(MT_UNF_PVR_EXTRA_READ_CALLBACK == eExtraCallbackType)
    {
        return  MT_PVR_PlayRegisterReadCallBack(u32ChnID, (ExtraCallBack)fCallback);
    }
    else if(MT_UNF_PVR_EXTRA_WRITE_CALLBACK == eExtraCallbackType)
    {
        return  MT_PVR_RecRegisterWriteCallBack(u32ChnID, (ExtraCallBack)fCallback);
    }
    else
    {
        MT_ERR_PVR("invalid callback type!\n");
    }

    return MT_FAILURE;
}

MT_S32 MT_PVR_UnRegisterExtraCallBack(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E eExtraCallbackType)
{
    if(MT_UNF_PVR_EXTRA_READ_CALLBACK == eExtraCallbackType)
    {
        return  MT_PVR_PlayUnRegisterReadCallBack(u32ChnID);
    }
    else if(MT_UNF_PVR_EXTRA_WRITE_CALLBACK == eExtraCallbackType)
    {
        return  MT_PVR_RecUnRegisterWriteCallBack(u32ChnID);
    }
    else
    {
        MT_ERR_PVR("invalid callback type!\n");
    }

    return MT_FAILURE;
}

mt_u32 MT_PVR_SysGetTimeStampMs(mt_u32 *ms)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    // timespec to milliseconds
    *ms =  (mt_u32)(ts.tv_sec) * 1000 + (uint64_t)(ts.tv_nsec) / 1000000;
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

