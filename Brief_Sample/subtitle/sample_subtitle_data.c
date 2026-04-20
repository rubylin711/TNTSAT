/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_debug.h"

#include "mt_unf_demux.h"
#include "mt_unf_subtouput.h"
#include "mt_unf_subt.h"

#include "sample_subtitle_data.h"

#define MT_FATAL_SUBT(fmt...)      MT_FATAL_PRINT(MT_ID_SUBT, fmt)
#define MT_ERR_SUBT(fmt...)        MT_ERR_PRINT(MT_ID_SUBT, fmt)
#define MT_WARN_SUBT(fmt...)       MT_WARN_PRINT(MT_ID_SUBT, fmt)
#define MT_INFO_SUBT(fmt...)       MT_INFO_PRINT(MT_ID_SUBT, fmt)

#define SUBT_STREAM_KEYWORD (0xBD)
#define SCTE_STREAM_KEYWORD (0xC6)
#define MAX_CHANNEL_BUF_SIZE (64 * 1024)
#define DATA_RECV_MAX_NUM (32)

#ifdef SUBTITLE_USED_MT_FILTER
typedef struct tagSUBT_DATA_S
{
    MT_BOOL bEnable;
    SUBT_DATA_INSTALL_PARAM_S stInstallParam;
    mt_s32  s32FilterID;
}SUBT_DATA_S;

static SUBT_DATA_S s_astSubtDataRecv[DATA_RECV_MAX_NUM];


static SUBT_DATA_S* FindSubtData(mt_s32 s32Filterid)
{
    mt_u8 i = 0;
    SUBT_DATA_S *pstSubtData = NULL;

    for (i = 0; i < DATA_RECV_MAX_NUM; i++)
    {
        pstSubtData = &s_astSubtDataRecv[i];
        if ((MT_TRUE == pstSubtData->bEnable)/* && (s32Filterid == pstSubtData->s32FilterID)*/)
        {
            return pstSubtData;
        }
    }

    return (SUBT_DATA_S*)NULL;
}

static mt_s32 FilterDataCallBack(mt_s32 s32Filterid, MT_FILTER_CALLBACK_TYPE_E enCallbackType,
                   MT_UNF_DMX_DATA_TYPE_E eDataType, mt_u8 *pu8Buffer, mt_u32 u32BufferLength)
{
    if (MT_FILTER_CALLBACK_TYPE_TIMEOUT != enCallbackType
        && pu8Buffer
        && u32BufferLength)
    {
        SUBT_DATA_S *pstSubtData = FindSubtData(s32Filterid);

        if (pstSubtData)
        {
            (mt_void)pstSubtData->stInstallParam.pfnCallback(pstSubtData->stInstallParam.u32UserData, pu8Buffer, u32BufferLength);
        }

    }
    else if(MT_FILTER_CALLBACK_TYPE_TIMEOUT == enCallbackType)
    {

    }

    return 0;
}

mt_s32 SUBT_Data_Init()
{
    mt_s32 s32Ret = MT_SUCCESS;

    s32Ret = MT_FILTER_Init();
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to MT_FILTER_Init !\n");

        return MT_FAILURE;
    }

    memset(s_astSubtDataRecv, 0, sizeof(s_astSubtDataRecv));

    return MT_SUCCESS;
}

mt_s32 SUBT_Data_DeInit(void)
{
    mt_u8 i = 0;
    SUBT_DATA_S *pstSubtData = NULL;

    for (i = 0; i < DATA_RECV_MAX_NUM; i++)
    {
        pstSubtData = &s_astSubtDataRecv[i];

        if (MT_TRUE == pstSubtData->bEnable)
        {
            (mt_void)SUBT_Data_Uninstall((MT_HANDLE)pstSubtData);
        }
    }
    memset(s_astSubtDataRecv, 0, sizeof(s_astSubtDataRecv));

    //MT_FILTER_DeInit();

    return MT_SUCCESS;
}

mt_s32 SUBT_Data_Install(SUBT_DATA_INSTALL_PARAM_S *pstInstallParam, MT_HANDLE *hData)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MT_FILTER_ATTR_S stFilterAttr;
    mt_s32 s32FilterID;
    SUBT_DATA_S *pstSubtData = NULL;
    mt_u8 i = 0;

    if (NULL == pstInstallParam || NULL == hData)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    for (i = 0; i < DATA_RECV_MAX_NUM; i++)
    {
        pstSubtData = &s_astSubtDataRecv[i];

        if (MT_FALSE == pstSubtData->bEnable)
        {
            break;
        }
    }
    if (i >= DATA_RECV_MAX_NUM)
    {
        MT_ERR_SUBT("install too much, max is %d !\n", DATA_RECV_MAX_NUM);

        return MT_FAILURE;
    }
    stFilterAttr.u32DMXID = pstInstallParam->u32DmxID;
    stFilterAttr.funCallback = FilterDataCallBack;
    stFilterAttr.u32DirTransFlag = 1;
    stFilterAttr.u32PID = pstInstallParam->u16SubtPID;
    stFilterAttr.u32TimeOutMs = 2000;
    memset(stFilterAttr.u8Mask,0,DMX_FILTER_MAX_DEPTH);
    memset(stFilterAttr.u8Match,0,DMX_FILTER_MAX_DEPTH);
    memset(stFilterAttr.u8Negate,0,DMX_FILTER_MAX_DEPTH);
    stFilterAttr.u32FilterDepth = 1;

    switch(pstInstallParam->enDataType)
    {
        case MT_UNF_SUBT_SCTE:
            stFilterAttr.u32FilterType = 0;//section
            stFilterAttr.u32CrcFlag = 2;
            stFilterAttr.u8Match[0] = SCTE_STREAM_KEYWORD;
            break;
        case MT_UNF_SUBT_DVB:
        default:
            stFilterAttr.u32FilterType = 1;//PES
            stFilterAttr.u32CrcFlag = 0;
            stFilterAttr.u8Match[0] = SUBT_STREAM_KEYWORD;
            break;
    }

    s32Ret = MT_FILTER_Creat(&stFilterAttr,&s32FilterID);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to MT_FILTER_Creat !\n");

        return MT_FAILURE;
    }
    s32Ret = MT_FILTER_Start(s32FilterID);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to MT_FILTER_Start !\n");

        return MT_FAILURE;
    }

    pstSubtData->stInstallParam = *pstInstallParam;
    pstSubtData->s32FilterID = s32FilterID;
    pstSubtData->bEnable = MT_TRUE;

    *hData = (MT_HANDLE)pstSubtData;

    return MT_SUCCESS;
}

mt_s32 SUBT_Data_Uninstall(MT_HANDLE hData)
{
    mt_s32 s32Ret = MT_SUCCESS;
    SUBT_DATA_S *pstSubtData = (SUBT_DATA_S *)hData;
    mt_s32 s32FilterID = pstSubtData->s32FilterID;

    s32Ret = MT_FILTER_Stop(s32FilterID);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to MT_FILTER_Stop !\n");

        return MT_FAILURE;
    }

    s32Ret = MT_FILTER_Destroy(s32FilterID);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to MT_FILTER_Destroy !\n");

        return MT_FAILURE;
    }

    pstSubtData->s32FilterID = 0;
    pstSubtData->bEnable = MT_FALSE;

    return MT_SUCCESS;
}

#else

typedef struct tagSUBT_DATA_S
{
    MT_BOOL bEnable;
    SUBT_DATA_INSTALL_PARAM_S stInstallParam;
    MT_HANDLE hChannelID;
    MT_HANDLE hFilterID;
}SUBT_DATA_S;

static SUBT_DATA_S     s_astSubtDataRecv[DATA_RECV_MAX_NUM];
static pthread_t       s_stSubtDataThreadID;
static pthread_mutex_t s_stSubtDataMutex;
static MT_BOOL         s_bSubtDataThreadReady = MT_FALSE;
static MT_BOOL         s_bInit = MT_FALSE;

static void SUBT_Data_Thread(mt_void *args)
{
    MT_UNF_DMX_DATA_S stSection[5] = { {0} };
    mt_u32 u32SectionNum = 0;
    mt_u8 num = (mt_u8)(intptr_t)args;
    SUBT_DATA_S *pstSubtData = NULL;

    MT_UNF_ES_BUF_S  pEsBuf;

    while (MT_TRUE == s_bSubtDataThreadReady)
    {
        pthread_mutex_lock(&s_stSubtDataMutex);
        pstSubtData = &s_astSubtDataRecv[num];
        if(pstSubtData->hChannelID == 0)
        {
            pthread_mutex_unlock(&s_stSubtDataMutex);
            MT_USLEEP(10 * 1000);
            continue;
        }
#if 0
        dat_cnt = 12;
        if(MT_SUCCESS!=MT_UNF_DMX_GetDataHandle(dathandle,&dat_cnt,100)){
            MT_USLEEP(10 * 1000);
            pthread_mutex_unlock(&s_stSubtDataMutex);
            continue;
        }

        for(i=0; i<dat_cnt; i++)
        {
            if(dathandle[i] == pstSubtData->hChannelID)
            {
                switch(pstSubtData->stInstallParam.enDataType)
                {
                    case MT_UNF_SUBT_SCTE:
                        if (MT_SUCCESS != MT_UNF_DMX_AcquireBuf(dathandle[i], 1, &u32SectionNum, stSection, 0))
                        {
                            continue;
                        }
                        break;

                    case MT_UNF_SUBT_DVB:
                    default:
                        if (MT_SUCCESS != MT_UNF_DMX_AcquireEs(dathandle[i], &pEsBuf))
                        {
                            continue;
                        }
                        break;
                }

                if (pstSubtData->stInstallParam.pfnCallback)
                {
                    switch(pstSubtData->stInstallParam.enDataType)
                    {
                       case MT_UNF_SUBT_SCTE:
                            (mt_void)pstSubtData->stInstallParam.pfnCallback(pstSubtData->stInstallParam.u32UserData, stSection[0].pu8Data, stSection[0].u32Size);
                            break;
                        case MT_UNF_SUBT_DVB:
                        default:
                            (mt_void)pstSubtData->stInstallParam.pfnCallback(pstSubtData->stInstallParam.u32UserData, pEsBuf.pu8Buf, pEsBuf.u32BufLen);
                            break;
                    }
                }

                switch(pstSubtData->stInstallParam.enDataType)
                {
                    case MT_UNF_SUBT_SCTE:
                        MT_MPI_DMX_ReleaseBuf(pstSubtData->hChannelID, u32SectionNum, stSection);
                        break;
                    case MT_UNF_SUBT_DVB:
                    default:
                        MT_MPI_DMX_ReleaseEs(pstSubtData->hChannelID, &pEsBuf);
                        break;
                }
            }
        }
#else

        if(MT_SUCCESS!=MT_UNF_DMX_CheckDataHandle(pstSubtData->hChannelID,0)){
            pthread_mutex_unlock(&s_stSubtDataMutex);
            MT_USLEEP(10 * 1000);
            continue;
        }

        switch(pstSubtData->stInstallParam.enDataType)
        {
            case MT_UNF_SUBT_SCTE:
                if (MT_SUCCESS != MT_UNF_DMX_AcquireBuf(pstSubtData->hChannelID, 1, &u32SectionNum, stSection, 0))
                {
                    pthread_mutex_unlock(&s_stSubtDataMutex);
                    MT_USLEEP(10*1000);
                    continue;
                }
                break;

            case MT_UNF_SUBT_DVB:
            default:
                if (MT_SUCCESS != MT_UNF_DMX_AcquireEs(pstSubtData->hChannelID, &pEsBuf))
                {
                    pthread_mutex_unlock(&s_stSubtDataMutex);
                    MT_USLEEP(10*1000);
                    continue;
                }
                break;
        }

        if (pstSubtData->stInstallParam.pfnCallback)
        {
            switch(pstSubtData->stInstallParam.enDataType)
            {
               case MT_UNF_SUBT_SCTE:
                    (mt_void)pstSubtData->stInstallParam.pfnCallback(pstSubtData->stInstallParam.u32UserData, stSection[0].pu8Data, stSection[0].u32Size);
                    break;
                case MT_UNF_SUBT_DVB:
                default:
                    (mt_void)pstSubtData->stInstallParam.pfnCallback(pstSubtData->stInstallParam.u32UserData, pEsBuf.pu8Buf, pEsBuf.u32BufLen);
                    break;
            }
        }

        switch(pstSubtData->stInstallParam.enDataType)
        {
            case MT_UNF_SUBT_SCTE:
                MT_UNF_DMX_ReleaseBuf(pstSubtData->hChannelID, u32SectionNum, stSection);
                break;
            case MT_UNF_SUBT_DVB:
            default:
                MT_UNF_DMX_ReleaseEs(pstSubtData->hChannelID, &pEsBuf);
                break;
        }
#endif
        pthread_mutex_unlock(&s_stSubtDataMutex);
        MT_USLEEP(10*1000);
    }
    MT_INFO_SUBT("Data receive thread exit!!!\n");
}


mt_s32 SUBT_Data_Init(mt_u8 u8Num)
{
    if (MT_FALSE == s_bInit)
    {
        memset(s_astSubtDataRecv, 0, sizeof(s_astSubtDataRecv));
        (mt_void)pthread_mutex_init(&s_stSubtDataMutex, NULL);
        s_bSubtDataThreadReady = MT_TRUE;
        pthread_create(&s_stSubtDataThreadID, NULL, (void * (*)(void *))SUBT_Data_Thread, (void*)(intptr_t)u8Num);
        s_bInit = MT_TRUE;
    }
    return MT_SUCCESS;
}


mt_s32 SUBT_Data_DeInit(mt_void)
{
    mt_u8 i = 0;
    SUBT_DATA_S *pstSubtData = NULL;

    if (MT_TRUE == s_bInit)
    {
        for (i = 0; i < DATA_RECV_MAX_NUM; i++)
        {
            pstSubtData = &s_astSubtDataRecv[i];

            if (MT_TRUE == pstSubtData->bEnable)
            {
                (mt_void)SUBT_Data_Uninstall((MT_HANDLE)pstSubtData);
            }
        }
        memset(s_astSubtDataRecv, 0, sizeof(s_astSubtDataRecv));


        s_bSubtDataThreadReady = MT_FALSE;
        pthread_join(s_stSubtDataThreadID, NULL);
        pthread_mutex_destroy(&s_stSubtDataMutex);
        s_bInit = MT_FALSE;
    }

    return MT_SUCCESS;
}

mt_s32 SUBT_Data_Install(SUBT_DATA_INSTALL_PARAM_S *pstInstallParam, MT_HANDLE *hData)
{
    MT_UNF_DMX_CHAN_ATTR_S tChAttr = {0};
    MT_UNF_DMX_FILTER_ATTR_S tFilterAttr = {0};
    mt_s32 s32Ret = 0;
    mt_u8  i = 0;
    SUBT_DATA_S *pstSubtData = NULL;
    if (MT_FALSE == s_bInit)
    {
        MT_ERR_SUBT("not init...\n");

        return MT_FAILURE;
    }

    if (NULL == pstInstallParam || NULL == hData)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }
    for (i = 0; i < DATA_RECV_MAX_NUM; i++)
    {
        pstSubtData = &s_astSubtDataRecv[i];
        //printf("%d,  pstSubtData->bEnable = %d\n",i,pstSubtData->bEnable);
        if (MT_FALSE == pstSubtData->bEnable)
        {
            break;
        }
    }
    if (i >= DATA_RECV_MAX_NUM)
    {
        MT_ERR_SUBT("install too much, max is %d !\n", DATA_RECV_MAX_NUM);

        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_GetChannelDefaultAttr(&tChAttr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to MT_FILTER_Destroy !\n");
        return MT_FAILURE;
    }
     switch(pstInstallParam->enDataType)
    {
        case MT_UNF_SUBT_SCTE:
            tChAttr.u32BufSize = MAX_CHANNEL_BUF_SIZE;
            tChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
            tChAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
            tChAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
            break;
        case MT_UNF_SUBT_DVB:
        default:
            tChAttr.u32BufSize = MAX_CHANNEL_BUF_SIZE;
            tChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_PES;
            tChAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
            tChAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
            break;
    }

    s32Ret = MT_UNF_DMX_CreateChannel(pstInstallParam->u32DmxID, &tChAttr, &pstSubtData->hChannelID);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_SUBT("failed to MT_UNF_DMX_CreateChannel !\n");
        return MT_FAILURE;
    }
    //printf("pstSubtData->hChannelID = 0x%x\n",pstSubtData->hChannelID);
    /* set channel PID for recving data */
    s32Ret = MT_UNF_DMX_SetChannelPID(pstSubtData->hChannelID, pstInstallParam->u16SubtPID);
    if (s32Ret != MT_SUCCESS)
    {
        (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);

        MT_ERR_SUBT("failed to MT_UNF_DMX_SetChannelPID !\n");
        return MT_FAILURE;
    }
    #if 1 //add yuwu
    if(tChAttr.enChannelType == MT_UNF_DMX_CHAN_TYPE_SEC)
    {
        memset(tFilterAttr.au8Match, 0, DMX_FILTER_MAX_DEPTH);
        memset(tFilterAttr.au8Mask, 0, DMX_FILTER_MAX_DEPTH);
        memset(tFilterAttr.au8Negate, 0, DMX_FILTER_MAX_DEPTH);
        tFilterAttr.u32FilterDepth = 1;
        tFilterAttr.au8Match[0] = SCTE_STREAM_KEYWORD; /* Subtitle stream id is 0xbd */
        tFilterAttr.au8Mask[0] = 0xff;

        s32Ret = MT_UNF_DMX_CreateFilter(pstInstallParam->u32DmxID, &tFilterAttr, &pstSubtData->hFilterID);
        if (s32Ret != MT_SUCCESS)
        {
            (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);

            MT_ERR_SUBT("failed to MT_UNF_DMX_CreateFilter !\n");
            return MT_FAILURE;
        }

        s32Ret = MT_UNF_DMX_SetFilterAttr(pstSubtData->hFilterID, &tFilterAttr);
        if (s32Ret != MT_SUCCESS)
        {
            (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);

            (mt_void)MT_UNF_DMX_DestroyFilter(pstSubtData->hFilterID);

            MT_ERR_SUBT("failed to MT_UNF_DMX_SetFilterAttr !\n");
            return MT_FAILURE;
        }

        s32Ret = MT_UNF_DMX_AttachFilter(pstSubtData->hFilterID, pstSubtData->hChannelID);
        if (s32Ret != MT_SUCCESS)
        {
            (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);

            (mt_void)MT_UNF_DMX_DestroyFilter(pstSubtData->hFilterID);

            MT_ERR_SUBT("failed to MT_UNF_DMX_AttachFilter !\n");
            return MT_FAILURE;
        }
    }
    #endif
    s32Ret = MT_UNF_DMX_OpenChannel(pstSubtData->hChannelID);
    if (s32Ret != MT_SUCCESS)
    {
        (mt_void)MT_UNF_DMX_DetachFilter(pstSubtData->hFilterID, pstSubtData->hChannelID);

        (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);

        (mt_void)MT_UNF_DMX_DestroyFilter(pstSubtData->hFilterID);

        MT_ERR_SUBT("failed to MT_UNF_DMX_OpenChannel !\n");
        return MT_FAILURE;
    }
    pthread_mutex_lock(&s_stSubtDataMutex);
    pstSubtData->stInstallParam = *pstInstallParam;
    pstSubtData->bEnable = MT_TRUE;
    pthread_mutex_unlock(&s_stSubtDataMutex);
    *hData = (MT_HANDLE)pstSubtData;
    return MT_SUCCESS;
}

mt_s32 SUBT_Data_Uninstall(MT_HANDLE hData)
{
    SUBT_DATA_S *pstSubtData = (SUBT_DATA_S *)hData;

    if (MT_FALSE == s_bInit)
    {
        MT_ERR_SUBT("not init...\n");

        return MT_FAILURE;
    }

    if ( MT_NULL == pstSubtData )
    {
        return MT_FAILURE;
    }

    (mt_void)MT_UNF_DMX_CloseChannel(pstSubtData->hChannelID);
    //(mt_void)MT_UNF_DMX_DetachFilter(pstSubtData->hFilterID, pstSubtData->hChannelID);
    //(mt_void)MT_UNF_DMX_DestroyFilter(pstSubtData->hFilterID);
    (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);
    pthread_mutex_lock(&s_stSubtDataMutex);
    pstSubtData->hChannelID = 0;
    pstSubtData->hFilterID  = 0;
    pstSubtData->bEnable    = MT_FALSE;
    pthread_mutex_unlock(&s_stSubtDataMutex);

    return MT_SUCCESS;
}
#endif

