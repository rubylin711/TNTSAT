/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <pthread.h>
#include <dlfcn.h>
#include <dirent.h>

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_module.h"
#include "mt_module_debug.h"
#include "mt_mpi_mem.h"
#include "mt_error_mpi.h"

#include "mt_unf_audio.h"
#include "mt_mpi_ao.h"
#include "mt_drv_ao.h"
#include "mt_mpi_aef.h"

static mt_void *s_apAefChn[AEF_MAX_INSTANCE_NUM] = {0};
/* aef authorize management */
static MT_AEF_AUTHORIZE_S *g_hFirstAefAuth = NULL;

#if defined(MT_HAEFFECT_BASE_SUPPORT)
extern MT_AEF_COMPONENT_S ha_base_effect_entry;
#endif

#if defined(MT_HAEFFECT_SRS_SUPPORT)
extern MT_AEF_COMPONENT_S srs_effect_entry;
#endif

static mt_u32 AEFHandle2ID(mt_handle hAef)
{
    return hAef & AO_AEF_CHNID_MASK;
}

static MT_AEF_AUTHORIZE_S *AEFFindHaEffectAuth(mt_u32 enEffectID)
{
    MT_AEF_AUTHORIZE_S *p;

    p = g_hFirstAefAuth;
    while (p)
    {
        if ((HA_GET_ID(p->enEffectID) == HA_GET_ID(enEffectID)))
        {

            return p;
        }

        p = p->pstNext;
    }

    MT_ERR_AO ("  AEFFindHaAffectAuth  effect(ID=0x%x) Fail \n", enEffectID);

    return NULL;
}

static mt_s32 AEFCheckHaEffectAuth(const MT_AEF_AUTHORIZE_S *pEntry)
{
    CHECK_AO_NULL_PTR(pEntry->GetAuthKey);

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_AEF_RegisterAuthLib(const mt_char *pAefLibFileName)
{
    mt_void * pDllModule;
    MT_AEF_AUTHORIZE_S **p;
    MT_AEF_AUTHORIZE_S *pEntry;

    CHECK_AO_NULL_PTR(pAefLibFileName);

    /* load the audio effect authorize lib and check for an error.  If filename is not an
     * absolute path (i.e., it does not  begin with a "/"), then the
     * file is searched for in the following locations:
     *
     *     The LD_LIBRARY_PATH environment variable locations
     *     The library cache, /etc/ld.so.cache.
     *     /lib
     *     /usr/lib
     *
     * If there is an error, we can't go on, so set the error code and exit */
    pDllModule = dlopen(pAefLibFileName, RTLD_LAZY | RTLD_GLOBAL);
    if (pDllModule == NULL)
    {
        MT_WARN_AO ( "  ****** Audio effect authorize lib %s failed because dlopen fail %s\n\n", pAefLibFileName, dlerror());
        return MT_FAILURE;
    }

    /* Get a entry pointer to the "ha_audio_decode_entry" .  If
     * there is an error, we can't go on, so set the error code and exit */
    pEntry = (MT_AEF_AUTHORIZE_S *)dlsym(pDllModule, "ha_audio_effect_auth_entry");
    if (pEntry == NULL)
    {
        MT_ERR_AO ( "  %s Failed because dlsym fail %s\n\n", pAefLibFileName, dlerror());
        dlclose(pDllModule);
        return MT_FAILURE;
    }

    if (MT_SUCCESS != AEFCheckHaEffectAuth(pEntry))
    {
        MT_ERR_AO ( " Register %s Failed \n", pAefLibFileName);
        dlclose(pDllModule);
        return MT_FAILURE;
    }

    p = &g_hFirstAefAuth;
    while (*p != NULL)
    {
        if (HA_GET_ID((*p)->enEffectID) == HA_GET_ID(pEntry->enEffectID))
        {
            MT_WARN_AO ( " Fail:Effect(ID=0x%x) had been Registered \n\n",
                           pEntry->enEffectID);
            dlclose(pDllModule);
            return MT_SUCCESS;
        }

        p = &(*p)->pstNext;
    }

    MT_INFO_AO ( "##### %s Effect Auth  Success #####\n\n", (mt_char*)(pEntry->szName));

    *p = pEntry;
    (*p)->pstNext = NULL;
    //(*p)->pDllModule = pDllModule;  //TODO if need?

    return MT_SUCCESS;
}

static MT_AEF_COMPONENT_S *AEFFindHaEffectComp(MT_UNF_SND_AEF_TYPE_E enEffectID)
{
    switch(enEffectID)
    {
#if defined(DOLBYDV258_SUPPORT)
        case MT_UNF_SND_AEF_TYPE_DOLBYDV258:
            return &ha_dolbydv258_entry;
#endif

#if defined(MT_HAEFFECT_SRS_SUPPORT)
        case MT_UNF_SND_AEF_TYPE_SRS3D:
            return &srs_effect_entry;
#endif

#if defined(MT_HAEFFECT_BASE_SUPPORT)
        case MT_UNF_SND_AEF_TYPE_BASE:
            return &ha_base_effect_entry;
#endif

        default:
            return MT_NULL;
    }
}


static mt_s32 AEFAllocChn(mt_u32 *pu32AefId)
{
    mt_u32 u32AefId;

    for(u32AefId = 0; u32AefId < AEF_MAX_INSTANCE_NUM; u32AefId++)
    {
        if(MT_NULL == s_apAefChn[u32AefId])
        {
            break;
        }
    }
    if(u32AefId == AEF_MAX_INSTANCE_NUM)
    {
        MT_ERR_AO(" no aef resource\n");
        return MT_FAILURE;
    }

    *pu32AefId = u32AefId;
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_AEF_Create(MT_UNF_SND_E enSound, MT_UNF_SND_AEF_TYPE_E enAefType, mt_void *pstAdvAttr, mt_handle *phAef)
{
    mt_s32 s32Ret;
    mt_u32 u32AefId;
    mt_handle hHaEffect = MT_INVALID_HANDLE;
    mt_u32    u32AfltId;
    MT_AEF_AUTHORIZE_S *pstAuthEntry = MT_NULL;
    MT_AEF_COMPONENT_S *pstEntry = MT_NULL;
    AEF_CHANNEL_S *pstAefChn = MT_NULL;
    mt_u32 u32AefProcAddr;
    AO_AEF_PROC_ITEM_S    *pstProcItem = MT_NULL;

    CHECK_AO_NULL_PTR(pstAdvAttr);
    CHECK_AO_NULL_PTR(phAef);

    s32Ret = AEFAllocChn(&u32AefId);
    if(MT_SUCCESS != s32Ret)
    {
        goto AEF_ERR_RETURE;
    }

    if(enAefType != MT_UNF_SND_AEF_TYPE_BASE)
    {
        //find effect authorize lib
        pstAuthEntry = AEFFindHaEffectAuth((mt_u32)enAefType);
        if (MT_NULL == pstAuthEntry)
        {
            MT_ERR_AO("  AEFFindHaEffectAuth fail u32EffectID(0x%x) ! \n", (mt_u32)enAefType);
            goto AEF_ERR_RETURE;
        }
    }

    //get authorize key
    //pstAuthEntry->GetAuthKey(pu32AuthKey);  //TODO return value

    //find effect component
    pstEntry = AEFFindHaEffectComp(enAefType);
    if(MT_NULL == pstEntry)
    {
        MT_ERR_AO(" can't find haeffect component %d\n", enAefType);
        goto AEF_ERR_RETURE;
    }

    //alloc aef channel
    pstAefChn = (AEF_CHANNEL_S *)mt_malloc(MT_ID_AO, sizeof(AEF_CHANNEL_S));
    if(MT_NULL == pstAefChn)
    {
        MT_ERR_AO(" Aef malloc channel failed\n");
        goto AEF_ERR_RETURE;
    }

    //create aef channe //TODO¡¡pstAdvAttr
    s32Ret = pstEntry->AefCreate(pstAuthEntry, pstAdvAttr, &hHaEffect);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef create failed\n");
        goto AEF_ERR_MALLOC;
    }

    s32Ret = pstEntry->AefGetConfig(hHaEffect, MT_AEF_GET_AFLTID_CMD, (mt_void *)(&u32AfltId));
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" AefGetConfig failed\n");
        goto AEF_ERR_MALLOC;
    }

    //attach aef channel sound
    s32Ret = MT_MPI_AO_SND_AttachAef(enSound, u32AfltId, &u32AefProcAddr);  //TODO AFLT_CHNID_MASK&hAef
    if(MT_SUCCESS != s32Ret || 0 == u32AefProcAddr)
    {
        MT_ERR_AO(" Aef[%d] attach sound[%d] failed\n", u32AefId, enSound);
        goto AEF_ERR_CREATE;
    }

    //pstProcItem = (AO_AEF_PROC_ITEM_S *)MT_MEM_Map(u32AefProcAddr, sizeof(AO_AEF_PROC_ITEM_S));
    if(MT_NULL == pstProcItem)
    {
        MT_ERR_AO(" MT_MEM_Map aef proc item failed\n");
        goto AEF_ERR_ATTACH;
    }

    //save proc info
    pstProcItem->enAefType = enAefType;
    pstProcItem->bEnable   = pstAefChn->bEnable;
    pstProcItem->u32AefId  = u32AefId;
    strncpy(pstProcItem->szName, pstEntry->szName, sizeof(pstProcItem->szName));
    if(enAefType != MT_UNF_SND_AEF_TYPE_BASE)
    {
        strncpy(pstProcItem->szDescription, pstAuthEntry->pszCustomerDescription, sizeof(pstProcItem->szDescription));
    }
    else
    {
        snprintf(pstProcItem->szDescription, sizeof(pstProcItem->szDescription), "None");
    }
    pstProcItem->szName[sizeof(pstProcItem->szName) - 1] = '\0';  //TQE
    //save info into aef channel
    pstAefChn->hHaEffect = hHaEffect;
    pstAefChn->hEntry = (mt_handle)pstEntry;
    pstAefChn->enSnd  = enSound;
    pstAefChn->bEnable = MT_FALSE;
    pstAefChn->pstProcItem = pstProcItem;

    s_apAefChn[u32AefId] = (mt_void *)pstAefChn;

 /*
  define of Aef Handle :
  bit31                                                           bit0
    |<----   16bit --------->|<---   8bit    --->|<---  8bit   --->|
    |--------------------------------------------------------------|
    |      MT_MOD_ID_E       |  sub_mod defined  |     chnID       |
    |--------------------------------------------------------------|
 */
    *phAef = (MT_ID_AO << 16) | (MT_ID_AEF << 8) | u32AefId;

    return MT_SUCCESS;

AEF_ERR_ATTACH:
    (mt_void)MT_MPI_AO_SND_DetachAef(enSound, u32AfltId);

AEF_ERR_CREATE:
    (mt_void)pstEntry->AefDestroy(hHaEffect);

AEF_ERR_MALLOC:
    //mt_free(MT_ID_AO, pstAefChn);

AEF_ERR_RETURE:
    return MT_FAILURE;
}

mt_s32 MT_MPI_AO_AEF_Destroy(mt_handle hAef)
{
    mt_s32 s32Ret;
    mt_u32 u32AefId;
    mt_u32 u32AfltId;
    MT_AEF_COMPONENT_S *pstEntry = MT_NULL;
    AEF_CHANNEL_S *pstAefChn     = MT_NULL;

    CHECK_AO_AEF_HANDLE(hAef);
    u32AefId = AEFHandle2ID(hAef);

    pstAefChn = (AEF_CHANNEL_S *)s_apAefChn[u32AefId];

    if(pstAefChn->pstProcItem)
    {
        //MT_MEM_Unmap((mt_void *)pstAefChn->pstProcItem);
        pstAefChn->pstProcItem = MT_NULL;
    }

    pstEntry = (MT_AEF_COMPONENT_S *)pstAefChn->hEntry;
    s32Ret = pstEntry->AefGetConfig(pstAefChn->hHaEffect, MT_AEF_GET_AFLTID_CMD, (mt_void *)(&u32AfltId));
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" AefGetConfig failed\n");
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_AO_SND_DetachAef(pstAefChn->enSnd, u32AfltId);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef[%d] detach sound[0x%x] failed\n", pstAefChn->enSnd, u32AefId);
        return MT_FAILURE;
    }

    s32Ret = pstEntry->AefDestroy(pstAefChn->hHaEffect);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef destory failed\n");
        return MT_FAILURE;
    }

    mt_free(MT_ID_AO, pstAefChn);
    s_apAefChn[u32AfltId] = MT_NULL;

    return MT_SUCCESS;

}

mt_s32 MT_MPI_AO_AEF_SetEnable(mt_handle hAef, MT_BOOL bEnable)
{
    mt_s32 s32Ret;
    mt_u32 u32AefId;
    MT_AEF_COMPONENT_S *pstEntry = MT_NULL;
    AEF_CHANNEL_S *pstAefChn     = MT_NULL;

    CHECK_AO_AEF_HANDLE(hAef);
    u32AefId = AEFHandle2ID(hAef);

    pstAefChn = (AEF_CHANNEL_S *)s_apAefChn[u32AefId];
    pstEntry = (MT_AEF_COMPONENT_S *)pstAefChn->hEntry;

    s32Ret = pstEntry->AefSetEnable(pstAefChn->hHaEffect, bEnable);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef setEnable failed\n");
        return MT_FAILURE;
    }
    pstAefChn->bEnable = bEnable;
    pstAefChn->pstProcItem->bEnable = bEnable;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_AEF_GetEnable(mt_handle hAef, MT_BOOL *pbEnable)
{
    mt_u32 u32AefId;
    AEF_CHANNEL_S *pstAefChn     = MT_NULL;

    CHECK_AO_NULL_PTR(pbEnable);
    CHECK_AO_AEF_HANDLE(hAef);
    u32AefId = AEFHandle2ID(hAef);

    pstAefChn = (AEF_CHANNEL_S *)s_apAefChn[u32AefId];

    *pbEnable = pstAefChn->bEnable;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_AEF_SetParams(mt_handle hAef, mt_u32 u32ParamType, const mt_void *pstParms)
{
    mt_s32 s32Ret;
    mt_u32 u32AefId;
    MT_AEF_COMPONENT_S *pstEntry = MT_NULL;
    AEF_CHANNEL_S *pstAefChn     = MT_NULL;

    CHECK_AO_NULL_PTR(pstParms);
    CHECK_AO_AEF_HANDLE(hAef);
    u32AefId = AEFHandle2ID(hAef);

    pstAefChn = (AEF_CHANNEL_S *)s_apAefChn[u32AefId];
    pstEntry = (MT_AEF_COMPONENT_S *)pstAefChn->hEntry;

    if(MT_TRUE == pstAefChn->bEnable)
    {
        MT_ERR_AO(" should stop aef before Set Aef Parameter\n");
        return MT_FAILURE;
    }

    s32Ret = pstEntry->AefSetParameter(pstAefChn->hHaEffect, u32ParamType, pstParms);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef SetParameter failed\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_AEF_GetParams(mt_handle hAef, mt_u32 u32ParamType, mt_void *pstParms)
{
    mt_s32 s32Ret;
    mt_u32 u32AefId;
    MT_AEF_COMPONENT_S *pstEntry = MT_NULL;
    AEF_CHANNEL_S *pstAefChn     = MT_NULL;

    CHECK_AO_NULL_PTR(pstParms);
    CHECK_AO_AEF_HANDLE(hAef);
    u32AefId = AEFHandle2ID(hAef);

    pstAefChn = (AEF_CHANNEL_S *)s_apAefChn[u32AefId];
    pstEntry = (MT_AEF_COMPONENT_S *)pstAefChn->hEntry;

    s32Ret = pstEntry->AefGetParameter(pstAefChn->hHaEffect, u32ParamType, pstParms);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef GetParameter failed\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_AEF_SetConfig(mt_handle hAef, mt_u32 u32CfgType, const mt_void *pstConfig)
{
    mt_s32 s32Ret;
    mt_u32 u32AefId;
    MT_AEF_COMPONENT_S *pstEntry = MT_NULL;
    AEF_CHANNEL_S *pstAefChn     = MT_NULL;

    CHECK_AO_NULL_PTR(pstConfig);
    CHECK_AO_AEF_HANDLE(hAef);
    u32AefId = AEFHandle2ID(hAef);

    pstAefChn = (AEF_CHANNEL_S *)s_apAefChn[u32AefId];
    pstEntry = (MT_AEF_COMPONENT_S *)pstAefChn->hEntry;

    s32Ret = pstEntry->AefSetConfig(pstAefChn->hHaEffect, u32CfgType, pstConfig);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef SetConfig failed\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_AEF_GetConfig(mt_handle hAef, mt_u32 u32CfgType, mt_void *pstConfig)
{
    mt_s32 s32Ret;
    mt_u32 u32AefId;
    MT_AEF_COMPONENT_S *pstEntry = MT_NULL;
    AEF_CHANNEL_S *pstAefChn     = MT_NULL;

    CHECK_AO_NULL_PTR(pstConfig);
    CHECK_AO_AEF_HANDLE(hAef);
    u32AefId = AEFHandle2ID(hAef);

    pstAefChn = (AEF_CHANNEL_S *)s_apAefChn[u32AefId];
    pstEntry = (MT_AEF_COMPONENT_S *)pstAefChn->hEntry;

    s32Ret = pstEntry->AefGetConfig(pstAefChn->hHaEffect, u32CfgType, pstConfig);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO(" Aef GetConfig failed\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
