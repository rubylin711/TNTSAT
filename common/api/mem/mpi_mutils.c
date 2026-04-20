#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mt_type.h"
#include "mt_drv_mem.h"
#include "mpi_mutils.h"
#include "drv_module_ioctl.h"
#include "mpi_mmgr.h"
#include "mpi_mem_base.h"


const char* g_szPoolName[] = {"Module_Mem_Pool", "Adp_Mem_Pool", "Usr_Mem_Pool", "MMZ_Pool", "LOG_Pool", "UnknowType"};

mt_handle mem_utils_init(mt_u32 u32Count, mem_utils_s stMem)
{
    mem_utils_s* pUtils = NULL;
    mt_u32 u32ItemSize = 0;
    mt_handle hResult = 0;
    
    pUtils = (mem_utils_s*)malloc(sizeof(mem_utils_s));

    if (NULL == pUtils)
    {
       MT_ERR_MEM("malloc %d size failure!\n", u32Count * u32ItemSize);

        return 0;
    }

    memset(pUtils, 0, sizeof(mem_utils_s));
    pUtils->enType = stMem.enType;
    pUtils->u32ItemCount = u32Count;

    switch(stMem.enType)
    {
        case MEM_POOL_TYPE_MODULE:
            u32ItemSize = sizeof(module_mem_pool_s)*u32Count;            
        break;
        case MEM_POOL_TYPE_MODULE_MEMORY:
            u32ItemSize = sizeof(module_mem_pool_s)*u32Count;
        break;
        case MEM_POOL_TYPE_USR_MEMORY:
            u32ItemSize = sizeof(usr_mem_pool_s)*u32Count;
        break;
        default:
            pUtils->enType = MEM_POOL_TYPE_BUTT;
        break;
    }

    pUtils->pMemBaseAddr = malloc(u32ItemSize);
    if (NULL == pUtils->pMemBaseAddr)
    {
        MT_ERR_MEM("failed to request %s memory size %d\n", g_szPoolName[pUtils->enType], u32ItemSize);
        free(pUtils);
    }
    else
    {
        memset(pUtils->pMemBaseAddr, 0, u32ItemSize);
        hResult = (mt_handle)pUtils;
        
        MT_INFO_MEM("successfully, request %s, and base address is 0x%08x\n", g_szPoolName[pUtils->enType], pUtils->pMemBaseAddr);
    }
    
    return hResult;
}

mt_void mem_utils_deinit(mt_handle hUtils)
{
    mem_utils_s* pUtils = (mem_utils_s*)hUtils;
    
    if (NULL != pUtils)
    {
        if (NULL != pUtils->pMemBaseAddr)
        {
            MT_INFO_MEM("free %s memory, and address is 0x%08x\n", g_szPoolName[pUtils->enType], pUtils->pMemBaseAddr);
            free(pUtils->pMemBaseAddr);
        }
        
        free((mt_void*)hUtils);
    }
}

mt_u32 mem_utils_get_item_no(mt_handle hUtils)
{
    mem_utils_s* pUtils = (mem_utils_s*)hUtils;
    
    if (NULL != pUtils)
    {
        return pUtils->u32HasCount;
    }

    return 0;
}


mt_void* mem_utils_malloc(mt_handle hUtils)
{
    mt_u32 u32Index = 0;

    mem_utils_s* pUtils = (mem_utils_s*)hUtils;
    mt_void* pResult = NULL;

    module_pool_s *     pModuleBase = NULL;
    module_mem_pool_s*  pMemAdpBase = NULL;
    usr_mem_pool_s*     pMemBase    = NULL;
    
    if (NULL != pUtils && pUtils->pMemBaseAddr != NULL)
    {
        switch(pUtils->enType)
        {
            case MEM_POOL_TYPE_MODULE:
            {
                pModuleBase = (module_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            case MEM_POOL_TYPE_MODULE_MEMORY:
            {
                pMemAdpBase = (module_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            case MEM_POOL_TYPE_USR_MEMORY:
            {
                pMemBase = (usr_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            default:
            break;
        }

        
        for (u32Index=0; u32Index<pUtils->u32ItemCount; u32Index++)
        {            
            if (NULL != pModuleBase && pModuleBase[u32Index].u32Idle == 0)
            {
                pModuleBase[u32Index].u32Idle = 1;

                pResult = &pModuleBase[u32Index];

                pUtils->u32HasCount++;
                
                MT_INFO_MEM("request %s and address[%d] is 0x%08x!\n", g_szPoolName[pUtils->enType], u32Index, pResult);
                break;
            }

            if (NULL != pMemAdpBase && pMemAdpBase[u32Index].u32Idle == 0)
            {
                pMemAdpBase[u32Index].u32Idle = 1;

                pResult = &pMemAdpBase[u32Index];

                pUtils->u32HasCount++;
                
                MT_INFO_MEM("request %s and address[%d] is 0x%08x!\n", g_szPoolName[pUtils->enType], u32Index, pResult);
                break;
            }

            if (NULL != pMemBase && pMemBase[u32Index].u32Idle == 0)
            {
                pMemBase[u32Index].u32Idle = 1;

                pResult = &pMemBase[u32Index];

                pUtils->u32HasCount++;
                
                MT_INFO_MEM("request %s and address[%d] is 0x%08x!\n", g_szPoolName[pUtils->enType], u32Index, pResult);
                break;
            }
        }

        return pResult;
    }


    MT_ERR_MEM("param is invalid!\n");
    
    return NULL;
}

mt_void mem_utils_free(mt_handle hUtils, mt_void* pAddr)
{
    mem_utils_s* pUtils = (mem_utils_s*)hUtils;
    mt_u32 u32Index = 0;

    module_pool_s*     pModuleBase = NULL;
    module_mem_pool_s* pMemAdpBase = NULL;
    usr_mem_pool_s*    pMemBase    = NULL;

    if (NULL != pUtils && pUtils->pMemBaseAddr != NULL)
    {
        switch(pUtils->enType)
        {
            case MEM_POOL_TYPE_MODULE:
            {
                pModuleBase = (module_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            case MEM_POOL_TYPE_MODULE_MEMORY:
            {
                pMemAdpBase = (module_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            case MEM_POOL_TYPE_USR_MEMORY:
            {
                pMemBase = (usr_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            default:
            break;
        }

        
        for (u32Index=0; u32Index<pUtils->u32ItemCount; u32Index++)
        {            
            if (NULL != pModuleBase && (&pModuleBase[u32Index] == pAddr) )
            {
                pModuleBase[u32Index].u32Idle = 0;

                memset(&pModuleBase[u32Index], 0, sizeof(pModuleBase[u32Index]));

                pUtils->u32HasCount--;
                
                MT_INFO_MEM("idle %s and address is 0x%08x, successfully!\n", g_szPoolName[pUtils->enType], pAddr);
                break;
            }

            if (NULL != pMemAdpBase && (&pMemAdpBase[u32Index] == pAddr) )
            {
                pMemAdpBase[u32Index].u32Idle = 0;

                memset(&pMemAdpBase[u32Index], 0, sizeof(pMemAdpBase[u32Index]));

                pUtils->u32HasCount--;
                
                MT_INFO_MEM("idle %s and address is 0x%08x, successfully!\n", g_szPoolName[pUtils->enType], pAddr);
                break;
            }

            if (NULL != pMemBase && (&pMemBase[u32Index] == pAddr) )
            {
                pMemBase[u32Index].u32Idle = 0;

                memset(&pMemBase[u32Index], 0, sizeof(pMemBase[u32Index]));

                pUtils->u32HasCount--;
                
                MT_INFO_MEM("idle %s and address is 0x%08x, successfully!\n", g_szPoolName[pUtils->enType], pAddr);
                break;
            }

        }

        return ;
    }


    MT_ERR_MEM("idle %s and address is 0x%08x, failure!!!\n", pUtils ? g_szPoolName[pUtils->enType] : g_szPoolName[MEM_POOL_TYPE_BUTT], pAddr);
    
    return;
}

