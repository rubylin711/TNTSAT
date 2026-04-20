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
#include "mt_mpi_demux.h"
#include "sample_cc_data.h"
#include "sample_cc_common.h"
/***************************** Macro Definition ******************************/
#define CC_STREAM_KEYWORD (0xBD)
#define MAX_CHANNEL_BUF_SIZE (64 * 1024)
#define DATA_RECV_MAX_NUM (32)
/*************************** Structure Definition ****************************/
typedef struct tagCC_DATA_S
{
    MT_BOOL bEnable;
    CC_DATA_INSTALL_PARAM_S stInstallParam;
    MT_HANDLE hChannelID;
    MT_HANDLE hFilterID;
}CC_DATA_S;
/********************** Global Variable declaration **************************/
static CC_DATA_S       s_astCCDataRecv[DATA_RECV_MAX_NUM];
static pthread_t       s_stCCDataThreadID;
static pthread_mutex_t s_stCCDataMutex;
static MT_BOOL         s_bCCDataThreadReady = MT_FALSE;
static MT_BOOL         s_bInit = MT_FALSE;
/******************************* API declaration *****************************/
static void CC_Data_Thread(MT_VOID *args)
{
    mt_u32           dat_cnt = 0;
    mt_handle        dathandle[12] = { 0 };
    CC_DATA_S        *pstCCData = NULL;
    MT_UNF_ES_BUF_S  pPesBuf = { 0 };

    while(MT_TRUE == s_bCCDataThreadReady)
    {
        pthread_mutex_lock(&s_stCCDataMutex);

        pstCCData = &s_astCCDataRecv[0];
        if(MT_FALSE == pstCCData->bEnable)
        {
            MT_USLEEP(10 * 1000);
            pthread_mutex_unlock(&s_stCCDataMutex);
            continue;
        }

        dat_cnt = 12;
        if(MT_SUCCESS!=MT_UNF_DMX_GetDataHandle(dathandle, &dat_cnt, 100))
        {
            MT_USLEEP(10 * 1000);
            pthread_mutex_unlock(&s_stCCDataMutex);
            continue;
        }

        for(mt_u32 i = 0; i < dat_cnt; i++)
        {
            if(dathandle[i] == pstCCData->hChannelID)
            {
                if(MT_SUCCESS != MT_UNF_DMX_AcquireEs(dathandle[i], &pPesBuf))
                {
                    continue;
                }
                if(pstCCData->stInstallParam.pfnCallback)
                {
                    (MT_VOID)pstCCData->stInstallParam.pfnCallback(pstCCData->stInstallParam.u32UserData, pPesBuf.pu8Buf, pPesBuf.u32BufLen);
                }
                MT_MPI_DMX_ReleaseEs(pstCCData->hChannelID, &pPesBuf);
            }
        }

        pthread_mutex_unlock(&s_stCCDataMutex);
        MT_USLEEP(50 * 1000);
    }

    SAMPLE_CC_INFO_PRINT("Data receive thread exit!!!\n");
}

MT_S32 CC_Data_Init(MT_VOID)
{
    if (MT_FALSE == s_bInit)
    {
        memset(s_astCCDataRecv, 0, sizeof(s_astCCDataRecv));

        (MT_VOID)pthread_mutex_init(&s_stCCDataMutex, NULL);
        s_bCCDataThreadReady = MT_TRUE;
        pthread_create(&s_stCCDataThreadID, NULL, (void *(*)(void *))CC_Data_Thread, (void *)MT_NULL);
        s_bInit = MT_TRUE;
    }
    
    return MT_SUCCESS;
}

MT_S32 CC_Data_DeInit(MT_VOID)
{
    CC_DATA_S *pstCCData = NULL;

    if(MT_TRUE == s_bInit)
    {
        for(mt_u8 i = 0; i < DATA_RECV_MAX_NUM; i++)
        {
            pstCCData = &s_astCCDataRecv[i];

            if (MT_TRUE == pstCCData->bEnable)
            {
                (MT_VOID)CC_Data_Uninstall((MT_HANDLE)pstCCData);
            }
        }
        
        memset(s_astCCDataRecv, 0, sizeof(s_astCCDataRecv));

        s_bCCDataThreadReady = MT_FALSE;
        pthread_join(s_stCCDataThreadID, NULL);
        pthread_mutex_destroy(&s_stCCDataMutex);
        s_bInit = MT_FALSE;
    }

    return MT_SUCCESS;
}

MT_S32 CC_Data_Install(CC_DATA_INSTALL_PARAM_S *pstInstallParam, mt_handle *hData)
{
    mt_u8                    i = 0;
    MT_S32                   s32Ret = 0;
    CC_DATA_S                *pstCCData = NULL;
    MT_UNF_DMX_CHAN_ATTR_S   tChAttr = { 0 };

    if(MT_FALSE == s_bInit)
    {
        SAMPLE_CC_ERR_PRINT("not init...\n");
        return MT_FAILURE;
    }

    if(NULL == pstInstallParam || NULL == hData)
    {
        SAMPLE_CC_ERR_PRINT("parameter is invalid...\n");
        return MT_FAILURE;
    }

    for(i = 0; i < DATA_RECV_MAX_NUM; i++)
    {
        pstCCData = &s_astCCDataRecv[i];

        if(MT_FALSE == pstCCData->bEnable)
        {
            break;
        }
    }
    if(i >= DATA_RECV_MAX_NUM)
    {
        SAMPLE_CC_ERR_PRINT("install too much, max is %d !\n", DATA_RECV_MAX_NUM);
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_GetChannelDefaultAttr(&tChAttr);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to MT_FILTER_Destroy !\n");
        return MT_FAILURE;
    }

    tChAttr.u32BufSize = MAX_CHANNEL_BUF_SIZE;
    tChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_PES;
    tChAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    tChAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    s32Ret = MT_UNF_DMX_CreateChannel(pstInstallParam->u32DmxID, &tChAttr, &pstCCData->hChannelID);
    if (s32Ret != MT_SUCCESS)
    {
        SAMPLE_CC_ERR_PRINT("failed to MT_UNF_DMX_CreateChannel !\n");
        return MT_FAILURE;
    }

    /* set channel PID for recving data */
    s32Ret = MT_UNF_DMX_SetChannelPID(pstCCData->hChannelID, pstInstallParam->u16CCPID);
    if (s32Ret != MT_SUCCESS)
    {
        SAMPLE_CC_ERR_PRINT("failed to MT_UNF_DMX_SetChannelPID !\n");
        goto ERR0;
    }

    s32Ret = MT_UNF_DMX_OpenChannel(pstCCData->hChannelID);
    if(s32Ret != MT_SUCCESS)
    {
        SAMPLE_CC_ERR_PRINT("failed to MT_UNF_DMX_OpenChannel !\n");
        goto ERR0;
    }

    pthread_mutex_lock(&s_stCCDataMutex);
    pstCCData->stInstallParam = *pstInstallParam;
    pstCCData->bEnable = MT_TRUE;
    pthread_mutex_unlock(&s_stCCDataMutex);

    *hData = (MT_HANDLE)pstCCData;

    return MT_SUCCESS;

ERR0:
    (MT_VOID)MT_UNF_DMX_DestroyChannel(pstCCData->hChannelID);

    return MT_FAILURE;
}

MT_S32 CC_Data_Uninstall(mt_handle hData)
{
    CC_DATA_S *pstCCData = NULL;

    if(MT_FALSE == s_bInit)
    {
        SAMPLE_CC_ERR_PRINT("not init...\n");

        return MT_FAILURE;
    }

    pstCCData = (CC_DATA_S *)hData;
    if(MT_NULL == pstCCData)
    {
        return MT_FAILURE;
    }

    (MT_VOID)MT_UNF_DMX_CloseChannel(pstCCData->hChannelID);
    (MT_VOID)MT_UNF_DMX_DestroyChannel(pstCCData->hChannelID);

    pthread_mutex_lock(&s_stCCDataMutex);
    pstCCData->hChannelID = 0;
    pstCCData->hFilterID = 0;
    pstCCData->bEnable = MT_FALSE;
    pthread_mutex_unlock(&s_stCCDataMutex);

    return MT_SUCCESS;
}

