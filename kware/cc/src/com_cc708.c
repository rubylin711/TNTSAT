/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mt_type.h"

#include "cc608.h"
#include "cc708.h"
#include "cc608_obj.h"
#include "cc708_obj.h"
#include "mt_cc608_def.h"
#include "mt_cc708_def.h"
#include "cc_queue.h"
#include "cc708_dec.h"

//#include "aribcc.h"
#include "mt_unf_cc.h"
#include "cc_debug.h"
#include "cc_timer.h"
#include "com_cc708.h"

#define CC_MAX_MODULE_NUM 0x01
#define CC_HANDLE_BASE  ((MT_ID_CC << 16) & 0xFFFF0000)

static MT_VOID* s_ahCC708[CC_MAX_MODULE_NUM];

static MT_BOOL s_bCC708Init = MT_FALSE;

#define CheckCC708Handle(pstParam)\
    do{\
        MT_U8 i = 0; \
        for (i = 0; i < CC_MAX_MODULE_NUM; i++)\
        { \
            if ((s_ahCC708[i] == pstParam) && (MT_NULL != pstParam))\
            { \
                break; \
            } \
        } \
        if (i >= CC_MAX_MODULE_NUM)\
        { \
            MT_ERR_CC("handle invalid!\n"); \
            return MT_FAILURE; \
        } \
    } while (0)

#define CheckCC708Init() \
    if (MT_FALSE == s_bCC708Init)\
    {\
        MT_ERR_CC("Not Init\n");\
        return MT_FAILURE;\
    }
    
typedef struct tagCC708_LOCAL_PARAM_S
{
    MT_HANDLE h608CC;
    MT_HANDLE h708CC;
    MT_UNF_CC_DATA_TYPE_E enCCType;
}CC708_LOCAL_PARAM_S;

static MT_U16 _CC708_GetHandleIndex(MT_HANDLE hCC)
{
    MT_U16 u16HandleIndex = hCC & 0x0000FFFF;

    return u16HandleIndex;
}

static MT_HANDLE _CC708_GetNewHandle(MT_U16 u16HandleIndex)
{
    MT_HANDLE hCC = MT_NULL;

    hCC = CC_HANDLE_BASE | u16HandleIndex;

    return hCC;
}

static MT_VOID* _CC708_GetParamAddr(MT_HANDLE hCC)
{
    MT_U16 u16HandleIndex = _CC708_GetHandleIndex(hCC);

    if (u16HandleIndex >= CC_MAX_MODULE_NUM)
    {
        return MT_NULL;
    }

    if (CC_HANDLE_BASE == (hCC & 0xFFFF0000))
    {
        return s_ahCC708[u16HandleIndex];
    }
    else
    {
        return MT_NULL;
    }
}

static MT_S32 _CC708_GetFreeIndex(MT_U16* pu16FreeIndex)
{
    MT_U16  i = 0;

    for (i = 0; i < CC_MAX_MODULE_NUM; i++)
    {
        if (MT_NULL == s_ahCC708[i])
        {
            break;
        }
    }

    if (i >= CC_MAX_MODULE_NUM)
    {
        MT_ERR_CC("instance num > %d\n", CC_MAX_MODULE_NUM);

        return MT_FAILURE;
    }

    *pu16FreeIndex = i;

    return MT_SUCCESS;
}



MT_S32 Com_CC708_Init(MT_VOID)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U8 u8Index = 0;    
    if(MT_FALSE == s_bCC708Init)
    {
        s32Ret = CC708_Init();
        s32Ret |= CC608_Init();
        s32Ret |= CCTimer_Init();
        memset(s_ahCC708, 0, sizeof(s_ahCC708));
        for(u8Index = 0; u8Index < CC_MAX_MODULE_NUM; u8Index ++)
        {
            if (s_ahCC708[u8Index])
            {
                MT_HANDLE hCC = _CC708_GetNewHandle(u8Index);
                (MT_VOID)Com_CC708_Destroy(hCC);
            }
        }

        memset(s_ahCC708, 0, sizeof(s_ahCC708));

        s_bCC708Init = MT_TRUE;
    }
    return s32Ret;
}

MT_S32 Com_CC708_Deinit(MT_VOID)
{
    MT_S32 s32Ret = MT_SUCCESS;
    if( MT_TRUE == s_bCC708Init )
    {
        s32Ret = CC708_DeInit();
        s32Ret |= CC608_DeInit();
        s32Ret |= CCTimer_DeInit();
        s_bCC708Init = MT_FALSE;
    }
    return s32Ret;
}

MT_S32 Com_CC708_Create( MT_UNF_CC_PARAM_S *pstCCParam, MT_HANDLE *phCC)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U8 u8ModuleId = 0;
    CC708_LOCAL_PARAM_S *pstParam = MT_NULL;
    MT_U16 u16Index = 0;
    CCDISP_INIT_PARAM_S stDispInitParm;

    if (MT_NULL == phCC || MT_NULL == pstCCParam)
    {
        MT_ERR_CC("params invalid\n");
        return MT_FAILURE;
    }
    if (pstCCParam->stCCAttr.enCCDataType >= MT_UNF_CC_DATA_TYPE_BUTT)
    {
        MT_ERR_CC("enCCDataType : %d not support\n", pstCCParam->stCCAttr.enCCDataType);
        return MT_FAILURE;
    }

    CheckCC708Init();

    s32Ret = _CC708_GetFreeIndex(&u16Index);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    stDispInitParm.stOpt.pfnGetPts = pstCCParam->pfnCCGetPts;
    stDispInitParm.stOpt.pfnDisplay = pstCCParam->pfnCCDisplay;
    stDispInitParm.stOpt.pfnGetTextSize = pstCCParam->pfnCCGetTextSize;
    //stDispInitParm.stOpt.pfnGetTextSizeEx = pstCCParam->pfnCCGetTextSizeEx;
    stDispInitParm.stOpt.pfnBlit = pstCCParam->pfnBlit;
    stDispInitParm.stOpt.pfnVBIOutput = pstCCParam->pfnVBIOutput;
    stDispInitParm.stOpt.pfnXDSOutput = pstCCParam->pfnXDSOutput;
    stDispInitParm.stOpt.pUserData = (MT_VOID*)(pstCCParam->u32UserData);
    (MT_VOID)CCDISP_Init(&stDispInitParm);
    
    pstParam = (CC708_LOCAL_PARAM_S *)malloc(sizeof(CC708_LOCAL_PARAM_S));
    if(MT_NULL == pstParam)
    {
        MT_ERR_CC("Malloc error\n");
        return MT_FAILURE;
    }
    memset(pstParam,0,sizeof(CC708_LOCAL_PARAM_S));

    if (pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_608
        || pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_708)
    {
        s32Ret = CC708_Create(&pstParam->h708CC);
        u8ModuleId = (MT_U8)(pstParam->h708CC);

        if (pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_608)
        {
            s32Ret |= CC608_Create(u8ModuleId);
            pstParam->h608CC = u8ModuleId;
            pstParam->enCCType = MT_UNF_CC_DATA_TYPE_608;

            s32Ret |= CC608_Config(u8ModuleId,&pstCCParam->stCCAttr.unCCConfig.stCC608ConfigParam);
        }
        else
        {
            pstParam->enCCType = MT_UNF_CC_DATA_TYPE_708;
            s32Ret |= CC708_Config(u8ModuleId,&pstCCParam->stCCAttr.unCCConfig.stCC708ConfigParam);
        }
    }

    s_ahCC708[u16Index] = pstParam;
    *phCC = _CC708_GetNewHandle(u16Index);

    return s32Ret;
}

MT_S32 Com_CC708_Destroy(MT_HANDLE hCC)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U16 u16HandleIndex = _CC708_GetHandleIndex(hCC);
    CC708_LOCAL_PARAM_S *pstParam = (CC708_LOCAL_PARAM_S *)_CC708_GetParamAddr(hCC);

    CheckCC708Init();
    CheckCC708Handle(pstParam);

    if(MT_UNF_CC_DATA_TYPE_608 == pstParam->enCCType
        || MT_UNF_CC_DATA_TYPE_708 == pstParam->enCCType)
    {
        s32Ret = CC708_Destroy(pstParam->h708CC);
        s32Ret |= CC608_Destroy(pstParam->h608CC);
    }

    free((void *)pstParam);
    pstParam = MT_NULL;

    (MT_VOID)CCDISP_DeInit();

    s_ahCC708[u16HandleIndex] = MT_NULL;

    return s32Ret;
}

MT_S32 Com_CC708_Start(MT_HANDLE hCC)
{
    MT_S32 s32Ret = MT_SUCCESS;
    CC708_LOCAL_PARAM_S *pstParam = (CC708_LOCAL_PARAM_S *)_CC708_GetParamAddr(hCC);

    CheckCC708Handle(pstParam);

    switch(pstParam->enCCType)
    {
        case MT_UNF_CC_DATA_TYPE_608:
            s32Ret = CC608_Start(pstParam->h608CC);
            break;
        case MT_UNF_CC_DATA_TYPE_708:
            s32Ret = CC708_Start(pstParam->h708CC);
            break;
        default:
            s32Ret = MT_FAILURE;
            break;
    }

    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_CC("cc start fail\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

MT_S32 Com_CC708_Stop(MT_HANDLE hCC)
{
    MT_S32 s32Ret = MT_SUCCESS;
    CC708_LOCAL_PARAM_S *pstParam = (CC708_LOCAL_PARAM_S *)_CC708_GetParamAddr(hCC);

    CheckCC708Handle(pstParam);

    DEC_TRACE("MT_UNF_CC_Stop, begin, enCCType = 0x%x!\n", pstParam->enCCType);

    switch(pstParam->enCCType)
    {
        case MT_UNF_CC_DATA_TYPE_608:
            s32Ret = CC608_Stop(pstParam->h608CC);
            break;
        case MT_UNF_CC_DATA_TYPE_708:
            s32Ret = CC708_Stop(pstParam->h708CC);
            break;
        default:
            s32Ret = MT_FAILURE;
            break;
    }

    if(MT_SUCCESS != s32Ret)
    {
        DEC_TRACE("cc stop fail\n");
    }
    CC_SLEEP(10);
    DEC_TRACE("MT_UNF_CC_Stop, end!\n");

    return MT_SUCCESS;
}

MT_S32 Com_CC708_Reset(MT_HANDLE hCC)
{
    MT_S32 s32Ret = MT_SUCCESS;
    CC708_LOCAL_PARAM_S *pstParam = (CC708_LOCAL_PARAM_S *)_CC708_GetParamAddr(hCC);

    CheckCC708Handle(pstParam);

    if(MT_UNF_CC_DATA_TYPE_608 == pstParam->enCCType
        || MT_UNF_CC_DATA_TYPE_708 == pstParam->enCCType)
    {
        s32Ret = CC708_Reset(pstParam->h708CC);/*user data in 708 module*/
        s32Ret |= CC608_Reset(pstParam->h608CC);
    }
    else
    {
        s32Ret = MT_FAILURE;
    }

    return s32Ret;

}
MT_S32 Com_CC708_DtvCC_ParsePicUsrData(MT_U8 *pu8CCPicData, MT_U8 u8CCdataLength, MT_BOOL bCheckATSC608)
{
    return CC708_DtvCC_ParsePicUsrData(pu8CCPicData,u8CCdataLength,bCheckATSC608);
}

MT_S32 Com_CC708_ProcessData(MT_VOID)
{
    return CC708_ProcessData();
}