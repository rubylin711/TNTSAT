/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_demux.h"
#include "sample_ttx_data.h"


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

#define MAX_CHANNEL_BUF_SIZE (64 * 1024)
#define DATA_RECV_MAX_NUM (32)




static TTX_DATA_S       g_astTtxDataRecv[DATA_RECV_MAX_NUM];
static pthread_t        g_stTtxDataThreadID;
pthread_mutex_t         g_stTtxDataMutex;
static MT_BOOL          g_bTtxDataThreadReady = MT_FALSE;

static MT_BOOL         g_bInit = MT_FALSE;



/*
@brief Receives ttx data thread function
@param void
@retuen MT_SUCCESS
@return MT_FALSE
*/
static void Ttx_Data_Thread(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    TTX_DATA_S *pstSubtData = NULL;
    MT_UNF_ES_BUF_S  pEsBuf = { 0 };

    while(MT_TRUE == g_bTtxDataThreadReady)
    {
        pthread_mutex_lock(&g_stTtxDataMutex);

        pstSubtData = &g_astTtxDataRecv[0];
        if(pstSubtData->hChannelID == 0)
        {
            MT_USLEEP(10 * 1000);
            pthread_mutex_unlock(&g_stTtxDataMutex);
            continue;
        }

       ret = MT_UNF_DMX_AcquireEs(pstSubtData->hChannelID,  &pEsBuf);
       if(MT_SUCCESS != ret)
       {
            MT_USLEEP(10 * 1000);
            pthread_mutex_unlock(&g_stTtxDataMutex);
            continue;
       }


        if(pstSubtData->stInstallParam.pfnCallback)
        {
            (mt_void)pstSubtData->stInstallParam.pfnCallback(pstSubtData->stInstallParam.u32UserData, pEsBuf.pu8Buf, pEsBuf.u32BufLen);
        }

        if(pstSubtData == NULL)
        if(pstSubtData->stInstallParam.pfnCallback == NULL)

        MT_USLEEP(10*1000);
        (void)MT_UNF_DMX_ReleaseEs(pstSubtData->hChannelID, &pEsBuf);
        pthread_mutex_unlock(&g_stTtxDataMutex);
    }

    SAMPLE_TTX_INFO_PRINT("Data receive thread exit!!!\n");
}


/*
@brief Start the ttx data thread function
@param void
@retuen MT_SUCCESS
@return MT_FALSE
*/
mt_s32 Ttx_Data_Init(mt_void)
{
    if (MT_FALSE == g_bInit)
    {
        memset(g_astTtxDataRecv, 0, sizeof(g_astTtxDataRecv));

        (mt_void)pthread_mutex_init(&g_stTtxDataMutex, NULL);
        g_bTtxDataThreadReady = MT_TRUE;
        pthread_create(&g_stTtxDataThreadID, NULL, (void * (*)(void *))Ttx_Data_Thread, (void*)MT_NULL);
        g_bInit = MT_TRUE;
    }


    return MT_SUCCESS;
}



/*
@brief Close the ttx data thread function
@param void
@retuen MT_SUCCESS
@return MT_FALSE
*/
mt_s32 Ttx_Data_DeInit(mt_void)
{
    mt_u8 i = 0;
    TTX_DATA_S *pstSubtData = NULL;
    if (MT_TRUE == g_bInit)
    {
        for(i = 0; i < DATA_RECV_MAX_NUM; i++)
        {
            pstSubtData = &g_astTtxDataRecv[i];

            if(MT_TRUE == pstSubtData->bEnable)
            {
                (mt_void)Ttx_Data_Uninstall((MT_HANDLE)pstSubtData);
            }
        }

        memset(g_astTtxDataRecv, 0, sizeof(g_astTtxDataRecv));

        g_bTtxDataThreadReady = MT_FALSE;
        pthread_join(g_stTtxDataThreadID, NULL);
        pthread_mutex_destroy(&g_stTtxDataMutex);
        g_bInit = MT_FALSE;
    }

    return MT_SUCCESS;
}



/*
@brief In preparation for starting the ttx thread, create the filter channel
@param[in] pstInstallParam, Gets the parameters for ttx data initialization
@param[out] hData, Parameter handle to ttx data initialization
@retuen MT_SUCCESS
@return MT_FALSE
*/
mt_s32 Ttx_Data_Install(TTX_DATA_INSTALL_PARAM_S *pstInstallParam, MT_HANDLE *hData)
{
    MT_UNF_DMX_CHAN_ATTR_S tChAttr = { 0 };
    mt_s32 s32Ret = 0;
    mt_u8  i = 0;
    TTX_DATA_S *pstSubtData = NULL;
    if (MT_FALSE == g_bInit)
    {
        printf("not init...\n");

        return MT_FAILURE;
    }

    if(NULL == pstInstallParam || NULL == hData)
    {
        SAMPLE_TTX_ERR_PRINT("parameter is invalid...\n");
        return MT_FAILURE;
    }

    for(i = 0; i < DATA_RECV_MAX_NUM; i++)
    {
        pstSubtData = &g_astTtxDataRecv[i];
        if(MT_FALSE == pstSubtData->bEnable)
        {
            break;
        }
    }

    if(i >= DATA_RECV_MAX_NUM)
    {
        SAMPLE_TTX_ERR_PRINT("install too much, max is %d !\n", DATA_RECV_MAX_NUM);

        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_GetChannelDefaultAttr(&tChAttr);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("failed to MT_UNF_DMX_GetChannelDefaultAttr !\n");
        return MT_FAILURE;
    }

    tChAttr.u32BufSize = MAX_CHANNEL_BUF_SIZE;
    tChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_PES;
    tChAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    tChAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;

    s32Ret = MT_UNF_DMX_CreateChannel(pstInstallParam->u32DmxID, &tChAttr, &pstSubtData->hChannelID);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("failed to MT_UNF_DMX_CreateChannel !\n");
        return MT_FAILURE;
    }

    /* set channel PID for recving data */
    s32Ret = MT_UNF_DMX_SetChannelPID(pstSubtData->hChannelID, pstInstallParam->u16TtxPID);
    if(MT_SUCCESS != s32Ret)
    {
        (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);

        SAMPLE_TTX_ERR_PRINT("failed to MT_UNF_DMX_SetChannelPID !\n");
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_OpenChannel(pstSubtData->hChannelID);
    if(MT_SUCCESS != s32Ret)
    {
        (mt_void)MT_UNF_DMX_DetachFilter(pstSubtData->hFilterID, pstSubtData->hChannelID);

        (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);

        (mt_void)MT_UNF_DMX_DestroyFilter(pstSubtData->hFilterID);

        SAMPLE_TTX_ERR_PRINT("failed to MT_UNF_DMX_OpenChannel !\n");
        return MT_FAILURE;
    }
    pstSubtData->stInstallParam = *pstInstallParam;
    pstSubtData->bEnable = MT_TRUE;
    *hData = (MT_HANDLE)pstSubtData;
    return MT_SUCCESS;
}


/*
@brief Release the created ttx channel
@param[in] hData, Parameter handle to ttx data initialization
@retuen MT_SUCCESS
@return MT_FALSE
*/
mt_s32 Ttx_Data_Uninstall(MT_HANDLE hData)
{
    TTX_DATA_S *pstSubtData = (TTX_DATA_S *)hData;
    if (MT_FALSE == g_bInit)
    {
        printf("not init...\n");
        return MT_FAILURE;
    }


    if( MT_NULL == pstSubtData )
    {
        return MT_FAILURE;
    }

    (mt_void)MT_UNF_DMX_CloseChannel(pstSubtData->hChannelID);
    (mt_void)MT_UNF_DMX_DestroyChannel(pstSubtData->hChannelID);
    pstSubtData->hChannelID = 0;
    pstSubtData->hFilterID  = 0;
    pstSubtData->bEnable    = MT_FALSE;
    return MT_SUCCESS;
}


