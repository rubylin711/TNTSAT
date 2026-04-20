/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "mt_debug.h"
#include "mt_drv_struct.h"
#include "mpi_module.h"
#include "drv_module_ioctl.h"
#include "mpi_mem_base.h"
#include "mt_common.h"

#ifdef CMN_MMGR_SUPPORT
#include "mpi_mmgr.h"
#endif

#define MODULE_BASE_ID (MT_DEBUG_ID_BUTT)

///////////////////////////////////////////////////////////////////////////////
//// global variable definition
///////////////////////////////////////////////////////////////////////////////
static pthread_mutex_t g_ModuleMutex = PTHREAD_MUTEX_INITIALIZER;

#define MODULE_LOCK(x) pthread_mutex_lock(x)
#define MODULE_UNLOCK(x) pthread_mutex_unlock(x)

static mt_s32 g_s32ModuleFd = -1;

#define MODULE_OPEN_FILE(fd, name)                     \
do{                                                    \
    if (-1 == fd)                                      \
    {                                                  \
        fd = open("/dev/"name, O_RDWR | O_CLOEXEC);                \
        if (fd < 0)                                    \
        {                                              \
            MODULE_UNLOCK(&g_ModuleMutex);             \
            MT_ERR_PRINT(MT_ID_MODULE, "open %s failure\n", name); \
            return MT_FAILURE;                         \
        }                                              \
    }                                                  \
}while(0)

#define MODULE_CLOSE_FILE(fd)                          \
do{                                                    \
    if (-1 != fd)                                      \
    {                                                  \
        close(fd);                                     \
    }                                                  \
}while(0)

mt_u8* mt_module_get_module_name(mt_u32 u32ModuleID);

mt_s32 mt_module_init(mt_void)
{
	MODULE_LOCK(&g_ModuleMutex);

    if (g_s32ModuleFd != -1)
    {
		MODULE_UNLOCK(&g_ModuleMutex);
        return MT_SUCCESS;
    }

    //!! NOTES: the following MACRO maybe fail to return, in advance.
    MODULE_OPEN_FILE(g_s32ModuleFd, UMAP_DEVNAME_MODULE);

#ifdef CMN_MMGR_SUPPORT
    module_mem_adp_init(MODULE_COUNT_MAX);
#endif

    MODULE_UNLOCK(&g_ModuleMutex);

    MT_INFO_PRINT(MT_ID_MODULE, "success and malloc module header the heap memory.\n");

    return MT_SUCCESS;
}

mt_s32 mt_module_deinit(mt_void)
{
    MODULE_LOCK(&g_ModuleMutex);

#ifdef CMN_MMGR_SUPPORT
    module_mem_adp_deinit();
#endif

    MODULE_CLOSE_FILE(g_s32ModuleFd);

    g_s32ModuleFd = -1;

    MODULE_UNLOCK(&g_ModuleMutex);

    MT_INFO_PRINT(MT_ID_MODULE, "has been called and free the heap memory,successfully.\n");

    return MT_SUCCESS;
}

mt_s32 mt_module_register(mt_u32 u32ModuleID, const mt_char * pszModuleName)
{
    mt_s32 s32Ret = MT_FAILURE;
    module_info_s stModule = {0};

	MODULE_LOCK(&g_ModuleMutex);

    if (g_s32ModuleFd == -1 || NULL == pszModuleName)
    {
        MODULE_UNLOCK(&g_ModuleMutex);
        MT_ERR_PRINT(MT_ID_MODULE, "param invalid!\n");
        return MT_FAILURE;
    }

    //memcpy(stModule.u8ModuleName, pszModuleName, sizeof(stModule.u8ModuleName));
    strncpy((char *)stModule.u8ModuleName, (const char *)pszModuleName, sizeof(stModule.u8ModuleName)-1);
    stModule.u32ModuleID = u32ModuleID;

    s32Ret = ioctl(g_s32ModuleFd,  CMD_ADD_MODULE_INFO, &stModule);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_PRINT(MT_ID_MODULE, "Add module fail:%#x!\n", s32Ret);
        MODULE_UNLOCK(&g_ModuleMutex);
        return MT_FAILURE;
    }

    MODULE_UNLOCK(&g_ModuleMutex);
    MT_INFO_PRINT(MT_ID_MODULE, "stModule %s and module ID = 0x%08x\n",stModule.u8ModuleName, stModule.u32ModuleID);

#ifdef CMN_MMGR_SUPPORT
    s32Ret = module_mem_add_module_info(&stModule);

    if (s32Ret != MT_SUCCESS)
    {
        return MT_FAILURE;
    }

#endif

    return MT_SUCCESS;
}

mt_s32 mt_module_register_by_name(const mt_char * pszModuleName, mt_u32* pu32ModuleID)
{
    mt_s32 s32Ret = MT_FAILURE;
    module_alloc_s stModule = {0};

    if ((NULL == pszModuleName) || (strlen(pszModuleName)==0) || (strlen(pszModuleName)>MAX_MODULE_NAME-1) || (NULL == pu32ModuleID))
    {
        MT_ERR_PRINT(MT_ID_MODULE, "name invalid!\n");
        return MT_FAILURE;
    }

    memcpy(stModule.u8ModuleName, pszModuleName, sizeof(stModule.u8ModuleName));

    if (g_s32ModuleFd == -1)
    {
        (mt_void)mt_module_init();
    }

    MODULE_LOCK(&g_ModuleMutex);
    s32Ret = ioctl(g_s32ModuleFd, CMD_ALLOC_MODULE_ID, &stModule);
    MODULE_UNLOCK(&g_ModuleMutex);

    if (MT_SUCCESS == s32Ret)
    {
        if (1 == stModule.s32Status)
        {
            *pu32ModuleID = stModule.u32ModuleID;
            MT_INFO_PRINT(MT_ID_MODULE, "HAD_REGISTERED %s %d\n", stModule.u8ModuleName, *pu32ModuleID);
            return MT_SUCCESS;
        }
        else if (0 == stModule.s32Status)
        {
            s32Ret = mt_module_register(stModule.u32ModuleID, (mt_char*)stModule.u8ModuleName);
            if (MT_SUCCESS == s32Ret)
            {
                *pu32ModuleID = stModule.u32ModuleID;
                MT_INFO_PRINT(MT_ID_MODULE, "FIRST REGISTER %s %d\n", stModule.u8ModuleName, *pu32ModuleID);
            }
        }
    }

    return s32Ret;
}

mt_s32 mt_module_unregister(mt_u32 u32ModuleID)
{
    mt_s32 s32Ret = MT_FAILURE;
    module_info_s stModule = {0};

	MODULE_LOCK(&g_ModuleMutex);

    if (g_s32ModuleFd == -1)
    {
		MODULE_UNLOCK(&g_ModuleMutex);
        return MT_FAILURE;
    }

    stModule.u32ModuleID = u32ModuleID;

    s32Ret = ioctl(g_s32ModuleFd, CMD_GET_MODULE_INFO, &stModule);

    if (s32Ret != MT_SUCCESS)
    {
        MODULE_UNLOCK(&g_ModuleMutex);
        return s32Ret;
    }

#ifdef CMN_MMGR_SUPPORT
    if (stModule.s32RegCount == 1)
    {
        s32Ret = module_mem_del_module_info(&stModule);

    }
#endif

    s32Ret = ioctl(g_s32ModuleFd, CMD_DEL_MODULE_INFO, &stModule);

    MODULE_UNLOCK(&g_ModuleMutex);

    return s32Ret;
}

mt_s32 mt_module_get_module_id(const mt_u8* pu8ModuleName)
{
    mt_s32 s32Ret = MT_FAILURE;
    module_info_s stModule = {0};

	MODULE_LOCK(&g_ModuleMutex);

    if (g_s32ModuleFd == -1 || pu8ModuleName == NULL)
    {
		MODULE_UNLOCK(&g_ModuleMutex);
		MT_ERR_PRINT(MT_ID_MODULE, "param invalid!\n");
        return MT_FAILURE;
    }

    memcpy(stModule.u8ModuleName, pu8ModuleName, sizeof(stModule.u8ModuleName));

    s32Ret = ioctl(g_s32ModuleFd, CMD_GET_MODULE_INFO, &stModule);
    MODULE_UNLOCK(&g_ModuleMutex);

    if (MT_SUCCESS == s32Ret)
    {
        return (mt_s32)stModule.u32ModuleID;
    }

    return (mt_s32)MT_INVALID_MODULE_ID;
}

mt_u8* mt_module_get_module_name(mt_u32 u32ModuleID)
{
    mt_s32 s32Ret = MT_FAILURE;

    static module_info_s stModule = {0};

	MODULE_LOCK(&g_ModuleMutex);

    if (g_s32ModuleFd == -1)
    {
		MODULE_UNLOCK(&g_ModuleMutex);

		MT_ERR_PRINT(MT_ID_MODULE, "deveice not open!\n");

        return NULL;
    }

    memset(&stModule, 0, sizeof(stModule));

    stModule.u32ModuleID = u32ModuleID;


    s32Ret = ioctl(g_s32ModuleFd, CMD_GET_MODULE_INFO, &stModule);
    MODULE_UNLOCK(&g_ModuleMutex);

    if (MT_SUCCESS == s32Ret)
    {
        return stModule.u8ModuleName;
    }

    return NULL;
}

