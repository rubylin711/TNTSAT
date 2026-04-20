/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************
  File Name     : mt_codec.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/3
  Description   : Functions implement of MT_CODEC.
  History       :
  1.Date        : 2015/12/3
    Author      :
    Modification: Created file

 *******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <pthread.h>
#include <dlfcn.h>
#include <string.h>

/* Unf headers */
#include "mt_video_codec.h"

/* Common headers */
#include "mt_mpi_mem.h"
#include "mt_module.h"
#include "mt_module_debug.h"

/* Mpi headers */
#include "mt_codec.h"

#include "list.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

/****************************** Macro Definition *****************************/

#if (MT_VDEC_REG_CODEC_SUPPORT == 1)
#define MAX_CODEC_LIB_LEN (256)
#endif

#define CODEC_MAKE_HANDLE(hCodec, hOriInst) ((hCodec<<8) | hOriInst)

#define CODEC_LOCK(Mutex) pthread_mutex_lock(&Mutex)
#define CODEC_UNLOCK(Mutex) pthread_mutex_unlock(&Mutex)

#define MT_MALLOC_CODEC(size) mt_malloc(MT_ID_VDEC, size)
#define MT_FREE_CODEC(addr) mt_free(MT_ID_VDEC, addr)

#define MT_ERR_CODEC(fmt...) \
    MT_ERR_PRINT(MT_ID_VDEC, fmt)
#define MT_WARN_CODEC(fmt...) \
    MT_WARN_PRINT(MT_ID_VDEC, fmt)
#define MT_INFO_CODEC(fmt...) \
    MT_INFO_PRINT(MT_ID_VDEC, fmt)

#define CODEC_FIND_CODEC_BY_STRUCT(pCodec, pCodecParam) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        CODEC_PARAM_S* pstTmp; \
        CODEC_LOCK(s_stParam.stMutex); \
        if (!list_empty(&s_stParam.stCodecHead)) \
        { \
            list_for_each_safe(pos, n, &s_stParam.stCodecHead) \
            { \
                pstTmp = list_entry(pos, CODEC_PARAM_S, stCodecNode); \
                if (pCodec == pstTmp->pstCodec) \
                { \
                    pCodecParam = pstTmp; \
                    break; \
                } \
            } \
        } \
        CODEC_UNLOCK(s_stParam.stMutex); \
    }


#define CODEC_FIND_CODEC_BY_LIBNAME(pszLib, pCodecParam) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        CODEC_PARAM_S* pstTmp; \
        CODEC_LOCK(s_stParam.stMutex); \
        if (!list_empty(&s_stParam.stCodecHead)) \
        { \
            list_for_each_safe(pos, n, &s_stParam.stCodecHead) \
            { \
                pstTmp = list_entry(pos, CODEC_PARAM_S, stCodecNode); \
                if ((MT_NULL != pstTmp->pszLibName) && (0 == strncmp(pszLib, pstTmp->pszLibName, strlen(pszLib)))) \
                { \
                    pCodecParam = pstTmp; \
                    break; \
                } \
            } \
        } \
        CODEC_UNLOCK(s_stParam.stMutex); \
    }

#define CODEC_FIND_CODEC_BY_INST(hInst, pCodecParam) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        mt_handle hCodec = MT_CODEC_LIB_HANDLE(hInst); \
        CODEC_PARAM_S* pstTmp; \
        CODEC_LOCK(s_stParam.stMutex); \
        list_for_each_safe(pos, n, &s_stParam.stCodecHead) \
        { \
            pstTmp = list_entry(pos, CODEC_PARAM_S, stCodecNode); \
            if (hCodec == pstTmp->hCodec) \
            { \
                pCodecParam = pstTmp; \
                break; \
            } \
        } \
        CODEC_UNLOCK(s_stParam.stMutex); \
    }

#define CODEC_FIND_INST(hInst, pCodecParam, pInst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        CODEC_INST_S* pstTmp; \
        list_for_each_safe(pos, n, &pCodecParam->stInstHead) \
        { \
            pstTmp = list_entry(pos, CODEC_INST_S, stInstNode); \
            if (hInst == pstTmp->hInst) \
            { \
                pInst = pstTmp; \
                break; \
            } \
        } \
    }

#define CODEC_FREE_INST_LIST(pCodecParam) \
    {  \
        struct list_head* pos; \
        struct list_head* n; \
        CODEC_INST_S* pstInst; \
        if (!list_empty(&pCodecParam->stInstHead)) \
        { \
            list_for_each_safe(pos, n, &pCodecParam->stInstHead) \
            { \
                pstInst = list_entry(pos, CODEC_INST_S, stInstNode); \
                (mt_void)pCodecParam->pstCodec->Destroy(MT_CODEC_INST_HANDLE(pstInst->hInst)); \
                list_del(pos); \
                MT_FREE_CODEC(pstInst); \
            } \
        } \
    }

#define CODEC_CHECK_INIT \
    if (!s_stParam.bInited) \
    { \
        MT_CODEC_Init(); \
    }

/************************ Static Structure Definition ************************/

/* Describe a codec instance */
typedef struct tagCODEC_INST_S
{
    mt_handle          hInst;          /* Codec instance handle, its value is ((hCodec<<16) | hOriInst) */
    struct list_head   stInstNode;     /* Codec instance node */
} CODEC_INST_S;

/* Describe a codec */
typedef struct tagCODEC_S
{
#if (MT_VDEC_REG_CODEC_SUPPORT == 1)
    mt_void*            pDllModule;     /* Handle of dlopen() */
    mt_char*            pszLibName;     /* Dll lib name(include path) */
#endif
    MT_CODEC_S*         pstCodec;       /* Code structure(MT_CODEC_S) */
    mt_handle           hCodec;         /* Handle */
    pthread_mutex_t     stMutex;        /* Codec mutex */
    MT_BOOL             bRegByLib;      /* Registered by lib */
    struct list_head    stInstHead;     /* Codec instance head */
    struct list_head    stCodecNode;    /* Codec list node */
} CODEC_PARAM_S;

/* Global parameter of this layer */
typedef struct tagCODEC_GLOBAL_S
{
    MT_BOOL             bCodecAlloc[MT_CODEC_MAX_NUMBER];
    MT_BOOL             bInited;            /* Had been inited or not */
    pthread_mutex_t     stMutex;            /* Global mutex */
    struct list_head    stCodecHead;        /* Codec list head */
    mt_u16              u16CodecNum;
} CODEC_GLOBAL_S;

/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/

static CODEC_GLOBAL_S s_stParam;

/*********************************** Code ************************************/

/* Check parameter */
static MT_BOOL CODEC_CheckMember(const MT_CODEC_S* pstCodec)
{
    if (MT_NULL == pstCodec)
    {
        return MT_FALSE;
    }

    if ((MT_NULL == pstCodec->GetCap)
       || (MT_NULL == pstCodec->Create)
       || (MT_NULL == pstCodec->Destroy))
    {
        return MT_FALSE;
    }

    return MT_TRUE;
}

/* Get first usable codec */
static CODEC_PARAM_S* CODEC_FindUsableCodec(MT_CODEC_TYPE_E enType, MT_CODEC_ID_E enID)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    MT_CODEC_CAP_S stCap;
    MT_CODEC_SUPPORT_S* pstSupport;
    struct list_head* pstPos = MT_NULL;
    struct list_head* pstN = MT_NULL;

    CODEC_LOCK(s_stParam.stMutex);
    if (!list_empty(&s_stParam.stCodecHead))
    {
        /* Check every registered codec */

        list_for_each_safe(pstPos, pstN, &s_stParam.stCodecHead)
        {

            pstCodecParam = list_entry(pstPos, CODEC_PARAM_S, stCodecNode);

            /* Get capability, if fail, pass this codec */
            if (MT_SUCCESS != pstCodecParam->pstCodec->GetCap(&stCap))
            {
                continue;
            }

            pstSupport = stCap.pstSupport;
            while (MT_NULL != pstSupport)
            {
                /* Yes, I get it! */
                if ((enID == pstSupport->enID) && (0 != (enType & pstSupport->u32Type)))
                {
                    CODEC_UNLOCK(s_stParam.stMutex);
                    return pstCodecParam;
                }

                pstSupport = pstSupport->pstNext;
            }
        }
    }

    CODEC_UNLOCK(s_stParam.stMutex);

    /* Fail */
    return MT_NULL;
}

static mt_void CODEC_UnRegister(CODEC_PARAM_S* pstCodecParam)
{
    /* Destroy all instance of this codec */
    CODEC_LOCK(pstCodecParam->stMutex);
    CODEC_FREE_INST_LIST(pstCodecParam);
    CODEC_UNLOCK(pstCodecParam->stMutex);

    /* Delete this codec from list */
    CODEC_LOCK(s_stParam.stMutex);
    list_del(&pstCodecParam->stCodecNode);
    s_stParam.bCodecAlloc[pstCodecParam->hCodec] = MT_FALSE;
    s_stParam.u16CodecNum--;
    CODEC_UNLOCK(s_stParam.stMutex);

#if (MT_VDEC_REG_CODEC_SUPPORT == 1)
    /* Free resource */
    if (pstCodecParam->bRegByLib)
    {
        if (pstCodecParam->pDllModule)
        {
            dlclose(pstCodecParam->pDllModule);
        }

        if (pstCodecParam->pszLibName)
        {
            MT_FREE_CODEC(pstCodecParam->pszLibName);
        }
    }
#endif

    (mt_void)pthread_mutex_destroy(&pstCodecParam->stMutex);

    /* Free parameter */
    MT_FREE_CODEC(pstCodecParam);
}

/* Init */
mt_s32 MT_CODEC_Init(mt_void)
{
    mt_s32 s32Ret;
    mt_s32 i;

    if (!s_stParam.bInited)
    {
        /* Init parameter */
        s_stParam.u16CodecNum = 0;
        for (i=0; i<MT_CODEC_MAX_NUMBER; i++)
        {
            s_stParam.bCodecAlloc[i] = MT_FALSE;
        }

        /* Init list */
        INIT_LIST_HEAD(&s_stParam.stCodecHead);

        /* Init mutex */
        s32Ret = pthread_mutex_init(&s_stParam.stMutex, MT_NULL);
        if (0 != s32Ret)
        {
            MT_WARN_CODEC("CODEC mutex init err.\n");
            return MT_FAILURE;
        }

        s_stParam.bInited = MT_TRUE;
    }

    MT_INFO_CODEC("MT_CODEC_Init OK\n");
    return MT_SUCCESS;
}

mt_s32 MT_CODEC_DeInit(mt_void)
{
#if 0
    mt_s32 s32Ret;
    struct list_head* pos;
    struct list_head* n;
    CODEC_PARAM_S* pstCodecParam;

    if (s_stParam.bInited)
    {
        CODEC_LOCK(s_stParam.stMutex);
        if (!list_empty(&s_stParam.stCodecHead))
        {
            list_for_each_safe(pos, n, &s_stParam.stCodecHead)
            {
                pstCodecParam = list_entry(pos, CODEC_PARAM_S, stCodecNode);
                CODEC_UNLOCK(s_stParam.stMutex);
                CODEC_UnRegister(pstCodecParam);
                CODEC_LOCK(s_stParam.stMutex);
            }
        }

        CODEC_UNLOCK(s_stParam.stMutex);

        /* DeInit mutex */
        s32Ret = pthread_mutex_destroy(&s_stParam.stMutex);
        if (0 != s32Ret)
        {
            MT_WARN_CODEC("CODEC mutex destroy err.\n");
            return MT_FAILURE;
        }

        /* DeInit parameter */
        s_stParam.u16CodecNum = 0;
        s_stParam.u16CodecHandle = 0;
    }

    s_stParam.bInited = MT_FALSE;
#endif
    MT_INFO_CODEC("MT_CODEC_DeInit OK\n");

    return MT_SUCCESS;
}

#if (MT_VDEC_REG_CODEC_SUPPORT == 1)
mt_s32 MT_CODEC_RegisterLib(const mt_char *pszCodecDllName)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_void* pDllModule = MT_NULL;
    MT_CODEC_S* pstCodec = MT_NULL;
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    mt_handle hCodecHandle = MT_INVALID_HANDLE;
    mt_u32 i;

    CODEC_CHECK_INIT;

    /* Check parameter */
    if (MT_NULL == pszCodecDllName)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    MT_INFO_CODEC("MT_CODEC_RegisterLib(%s) begin...\n", pszCodecDllName);

    /* Check registered */
    CODEC_FIND_CODEC_BY_LIBNAME(pszCodecDllName, pstCodecParam);
    if (MT_NULL != pstCodecParam)
    {
        MT_ERR_CODEC("Register %s fail:Had been registered.\n", pszCodecDllName);
        return MT_SUCCESS;
    }

    MT_INFO_CODEC("MT_CODEC_RegisterLib(%s) first time, go...\n", pszCodecDllName);

    /* 1. dlopen() lib; */
    // TODO: Test other open type
    pDllModule = dlopen(pszCodecDllName, RTLD_LAZY | RTLD_GLOBAL);
    if (MT_NULL == pDllModule)
    {
        MT_ERR_CODEC("Register %s fail:%s.\n", pszCodecDllName, dlerror());
        s32Ret = MT_ERR_CODEC_INVALIDPARAM;
        goto err0;
    }

    MT_INFO_CODEC("MT_CODEC_RegisterLib(%s) dlopen success, go...\n", pszCodecDllName);

    /* 2. Get entry symbol; */
    pstCodec = (MT_CODEC_S*)dlsym(pDllModule, "mt_codec_entry");
    if (MT_NULL == pstCodec)
    {
        MT_ERR_CODEC("Register %s fail:%s.\n", pszCodecDllName, dlerror());
        s32Ret = MT_ERR_CODEC_INVALIDPARAM;
        goto err1;
    }

    MT_INFO_CODEC("MT_CODEC_RegisterLib(%s) get mt_codec_entry success, go...\n", pszCodecDllName);

    /* 3. Check member methods */
    if (!CODEC_CheckMember(pstCodec))
    {
        MT_ERR_CODEC("Register %s fail: invalid method.\n", pszCodecDllName);
        s32Ret = MT_ERR_CODEC_INVALIDPARAM;
        goto err1;
    }

    MT_INFO_CODEC("MT_CODEC_RegisterLib(%s) check methods success, go...\n", pszCodecDllName);

    /* 4. Allocate resource, create CODEC_PARAM_S */
    pstCodecParam = (CODEC_PARAM_S*)MT_MALLOC_CODEC(sizeof(CODEC_PARAM_S));
    if (MT_NULL == pstCodecParam)
    {
        MT_ERR_CODEC("No memory.\n");
        s32Ret = MT_ERR_CODEC_NOENOUGHRES;
        goto err1;
    }

    /* 5. Add CODEC_PARAM_S to codec list */
    CODEC_LOCK(s_stParam.stMutex);

    /* Alloc codec handle */
    for (i=0; i<MT_CODEC_MAX_NUMBER; i++)
    {
        if (!(s_stParam.bCodecAlloc[i]))
        {
            hCodecHandle = i;
            break;
        }
    }

    if ((MT_INVALID_HANDLE == hCodecHandle) || (MT_CODEC_MAX_NUMBER == hCodecHandle))
    {
        CODEC_UNLOCK(s_stParam.stMutex);
        MT_FREE_CODEC(pstCodecParam);
        MT_ERR_CODEC("Too many codecs registered.\n");
        return MT_ERR_CODEC_NOENOUGHRES;
    }

    /* Init parameter of codec */
    pstCodecParam->pDllModule = pDllModule;
    pstCodecParam->pszLibName = (mt_char*)MT_MALLOC_CODEC(strlen(pszCodecDllName)+1);
    if (MT_NULL == pstCodecParam->pszLibName)
    {
        MT_ERR_CODEC("No memory.\n");
        s32Ret = MT_ERR_CODEC_NOENOUGHRES;
        goto err2;
    }
    strncpy(pstCodecParam->pszLibName, pszCodecDllName,strlen(pszCodecDllName)+1);
    pstCodecParam->pstCodec = pstCodec;
    pstCodecParam->hCodec = hCodecHandle;
    pstCodecParam->bRegByLib = MT_TRUE;
    if (0 != pthread_mutex_init(&pstCodecParam->stMutex, MT_NULL))
    {
        CODEC_UNLOCK(s_stParam.stMutex);
        s32Ret = MT_ERR_CODEC_NOENOUGHRES;
        goto err3;
    }

    INIT_LIST_HEAD(&pstCodecParam->stInstHead);

    /* Add this codec to list TAIL.(USE list_add_tail) */
    list_add_tail(&pstCodecParam->stCodecNode, &s_stParam.stCodecHead);
    s_stParam.u16CodecNum++;

    /* Set handle flag */
    s_stParam.bCodecAlloc[hCodecHandle] = MT_TRUE;

    CODEC_UNLOCK(s_stParam.stMutex);

    MT_INFO_CODEC("MT_CODEC_RegisterLib %s OK\n", pszCodecDllName);
    return MT_SUCCESS;

err3:
    MT_FREE_CODEC(pstCodecParam->pszLibName);
err2:
    MT_FREE_CODEC(pstCodecParam);
    CODEC_UNLOCK(s_stParam.stMutex);
err1:
    dlclose(pDllModule);
err0:
    return s32Ret;
}

mt_s32 MT_CODEC_UnRegisterLib(const mt_char *pszCodecDllName)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;

    CODEC_CHECK_INIT;

    /* Check parameter */
    if (MT_NULL == pszCodecDllName)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Find CODEC_PARAM_S* by pszCodecDllName */
    CODEC_FIND_CODEC_BY_LIBNAME(pszCodecDllName, pstCodecParam);
    if (MT_NULL == pstCodecParam)
    {
        MT_ERR_CODEC("UnRegister %s fail: can't find.\n", pszCodecDllName);
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* UnRegister */
    CODEC_UnRegister(pstCodecParam);

    return MT_SUCCESS;
}
#endif

/* Register codec using structure(MT_CODEC_S*) */
mt_s32 MT_CODEC_Register(MT_CODEC_S* pstCodec)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    mt_handle hCodecHandle = MT_INVALID_HANDLE;
    mt_u32 i;

    CODEC_CHECK_INIT;

    /* Check member property and method */
    if (!CODEC_CheckMember(pstCodec))
    {
        MT_ERR_CODEC("Invalid method.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Find codec */
    CODEC_FIND_CODEC_BY_STRUCT(pstCodec, pstCodecParam);
    if (MT_NULL != pstCodecParam)
    {
        MT_ERR_CODEC("Had been registered.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Allocate resource */
    pstCodecParam = (CODEC_PARAM_S*)MT_MALLOC_CODEC(sizeof(CODEC_PARAM_S));
    if (MT_NULL == pstCodecParam)
    {
        MT_ERR_CODEC("No memory.\n");
        return MT_ERR_CODEC_NOENOUGHRES;
    }

    CODEC_LOCK(s_stParam.stMutex);

    /* Alloc codec handle */
    for (i=0; i<MT_CODEC_MAX_NUMBER; i++)
    {
        if (!(s_stParam.bCodecAlloc[i]))
        {
            hCodecHandle = i;
            break;
        }
    }

    if ((MT_INVALID_HANDLE == hCodecHandle) || (MT_CODEC_MAX_NUMBER == hCodecHandle))
    {
        CODEC_UNLOCK(s_stParam.stMutex);
        MT_FREE_CODEC(pstCodecParam);
        MT_ERR_CODEC("Too many codecs registered.\n");
        return MT_ERR_CODEC_NOENOUGHRES;
    }

    /* Init parameter of codec */
#if (MT_VDEC_REG_CODEC_SUPPORT == 1)
    pstCodecParam->pDllModule = MT_NULL;
    pstCodecParam->pszLibName = MT_NULL;
#endif
    pstCodecParam->pstCodec = pstCodec;
    pstCodecParam->hCodec = hCodecHandle;
    pstCodecParam->bRegByLib = MT_FALSE;
    if (0 != pthread_mutex_init(&pstCodecParam->stMutex, MT_NULL))
    {
        CODEC_UNLOCK(s_stParam.stMutex);
        MT_FREE_CODEC(pstCodecParam);
        return MT_FAILURE;
    }

    INIT_LIST_HEAD(&pstCodecParam->stInstHead);

    /*
     * Add this codec to list HEAD.(USE list_add)
     * NOTE:
     *   CODEC REGISTERED BY THIS INTERFACE WILL HAVE THE HIGHEST PRIORITY.
     *   REGISTER VFMW WILL CALL THIS INTERFACE.
     */
    list_add(&pstCodecParam->stCodecNode, &s_stParam.stCodecHead);
    s_stParam.u16CodecNum++;

    /* Set handle flag */
    s_stParam.bCodecAlloc[hCodecHandle] = MT_TRUE;

    CODEC_UNLOCK(s_stParam.stMutex);
    MT_INFO_CODEC("MT_CODEC_Register OK\n");

    return MT_SUCCESS;
}

/* Unregister codec using structure(MT_CODEC_S*) */
mt_s32 MT_CODEC_UnRegister(const MT_CODEC_S* pstCodec)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;

    CODEC_CHECK_INIT;

    if (MT_NULL == pstCodec)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Find codec */
    CODEC_FIND_CODEC_BY_STRUCT(pstCodec, pstCodecParam);
    if (MT_NULL == pstCodecParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    CODEC_UnRegister(pstCodecParam);

    MT_INFO_CODEC("MT_CODEC_UnRegister OK\n");
    return MT_SUCCESS;
}

/* Create instance */
MT_CODEC_S* MT_CODEC_Create(mt_handle* phInst, const MT_CODEC_OPENPARAM_S * pstParam)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    CODEC_INST_S* pstInst;
    mt_s32 s32Ret;

    if (!s_stParam.bInited)
    {
        return MT_NULL;
    }

    if ((MT_NULL == phInst) || (MT_NULL == pstParam))
    {
        return MT_NULL;
    }

    if (MT_CODEC_TYPE_BUTT <= pstParam->enType)
    {
        return MT_NULL;
    }

    /* Find usable codec by type and ID */
    pstCodecParam = CODEC_FindUsableCodec(pstParam->enType, pstParam->enID);
    if (MT_NULL == pstCodecParam)
    {
        //MT_ERR_CODEC("No usable codec.\n");
        MT_ERR_VDEC("No usable codec.\n");
        return MT_NULL;
    }

    /* Create codec instance */
    s32Ret = pstCodecParam->pstCodec->Create(phInst, pstParam);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_NULL;
    }

    /* Alloc instance resource */
    pstInst = (CODEC_INST_S*)MT_MALLOC_CODEC(sizeof(CODEC_INST_S));
    if (MT_NULL == pstInst)
    {
        s32Ret = pstCodecParam->pstCodec->Destroy(*phInst);
        return MT_NULL;
    }

    /* Make instance handle */
    pstInst->hInst = CODEC_MAKE_HANDLE(pstCodecParam->hCodec, *phInst);

    /* Add instance to list */
    CODEC_LOCK(pstCodecParam->stMutex);
    list_add_tail(&pstInst->stInstNode, &pstCodecParam->stInstHead);
    CODEC_UNLOCK(pstCodecParam->stMutex);

    *phInst = pstInst->hInst;

    MT_INFO_CODEC("MT_CODEC_Create OK\n");
    return pstCodecParam->pstCodec;
}

/* Destroy instance */
mt_s32 MT_CODEC_Destory(mt_handle hInst)
{
    mt_s32 s32Ret = MT_SUCCESS;
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    CODEC_INST_S* pstInst = MT_NULL;

    CODEC_CHECK_INIT;

    /* Find codec */
    CODEC_FIND_CODEC_BY_INST(hInst, pstCodecParam);
    if (MT_NULL == pstCodecParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    CODEC_LOCK(pstCodecParam->stMutex);

    /* Find instance */
    CODEC_FIND_INST(hInst, pstCodecParam, pstInst);
    if (MT_NULL == pstInst)
    {
        CODEC_UNLOCK(pstCodecParam->stMutex);
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Delete instance from list */
    list_del(&pstInst->stInstNode);
    CODEC_UNLOCK(pstCodecParam->stMutex);

    /* Stop and Destroy instance */
    if (pstCodecParam->pstCodec->Stop)
    {
        s32Ret  = pstCodecParam->pstCodec->Stop(MT_CODEC_INST_HANDLE(hInst));
    }
    s32Ret |= pstCodecParam->pstCodec->Destroy(MT_CODEC_INST_HANDLE(hInst));

    /* Free resource */
    MT_FREE_CODEC(pstInst);

    MT_INFO_CODEC("MT_CODEC_Destory OK\n");
    return s32Ret;
}

MT_BOOL MT_CODEC_SupportDecode(const MT_CODEC_S* pstCodec, MT_CODEC_ID_E enID)
{
    MT_CODEC_CAP_S stCap;
    MT_CODEC_SUPPORT_S* pstSupport;

    if (!s_stParam.bInited)
    {
        return MT_FALSE;
    }

    if ((MT_NULL == pstCodec) || (enID >= MT_CODEC_ID_BUTT))
    {
        return MT_FALSE;
    }

    if (MT_SUCCESS != pstCodec->GetCap(&stCap))
    {
        return MT_FALSE;
    }

    pstSupport = stCap.pstSupport;
    while (MT_NULL != pstSupport)
    {
        /* Yes, I get it! */
        if ((enID == pstSupport->enID) && (0 != (MT_CODEC_TYPE_DEC & pstSupport->u32Type)))
        {
            return MT_TRUE;
        }

        pstSupport = pstSupport->pstNext;
    }

    return MT_FALSE;
}

/* Get codec name */
const mt_char* MT_CODEC_GetName(mt_handle hInst)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    CODEC_INST_S* pstInst = MT_NULL;

    if (!s_stParam.bInited)
    {
        return MT_NULL;
    }

    /* Find codec */
    CODEC_FIND_CODEC_BY_INST(hInst, pstCodecParam);
    if (pstCodecParam)
    {
        CODEC_FIND_INST(hInst, pstCodecParam, pstInst);
        if (pstInst)
        {
            MT_INFO_CODEC("MT_CODEC_GetName OK:%s.\n", pstCodecParam->pstCodec->pszName);
            return pstCodecParam->pstCodec->pszName;
        }
    }

    return MT_NULL;
}
#if 0
/* Get codec version */
MT_CODEC_VERSION_U MT_CODEC_GetVersion(mt_handle hInst)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    CODEC_INST_S* pstInst = MT_NULL;
    MT_CODEC_VERSION_U unInvalidVer = {.stVersion = {0, 0, 0, 0}};

    if (!s_stParam.bInited)
    {
        return unInvalidVer;
    }

    /* Find codec */
    CODEC_FIND_CODEC_BY_INST(hInst, pstCodecParam);
    if (pstCodecParam)
    {
        CODEC_FIND_INST(hInst, pstCodecParam, pstInst);
        if (pstInst)
        {
            MT_INFO_CODEC("MT_CODEC_GetVersion OK:%x.\n", pstCodecParam->pstCodec->unVersion.u32Version);
            return pstCodecParam->pstCodec->unVersion;
        }
    }

    return unInvalidVer;
}
#endif
/* Codec has frame buffer? */
MT_BOOL MT_CODEC_NeedFrameBuf(mt_handle hInst)
{
    CODEC_PARAM_S* pstCodecParam = MT_NULL;
    CODEC_INST_S* pstInst = MT_NULL;
    MT_CODEC_CAP_S stCap;

    if (!s_stParam.bInited)
    {
        return MT_FALSE;
    }

    /* Find codec */
    CODEC_FIND_CODEC_BY_INST(hInst, pstCodecParam);
    if (pstCodecParam)
    {
        CODEC_FIND_INST(hInst, pstCodecParam, pstInst);
        if (pstInst)
        {
            if (MT_SUCCESS == pstCodecParam->pstCodec->GetCap(&stCap))
            {
                MT_INFO_CODEC("MT_CODEC_NeedFrameBuf OK.\n");

                /* Don't need frame buffer if MT_CODEC_CAP_DRIVENSELF */
                return (MT_BOOL)(MT_CODEC_CAP_DRIVENSELF != (stCap.u32CapNumber & MT_CODEC_CAP_DRIVENSELF));
            }
        }
    }

    /* Default return false */
    return MT_FALSE;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
