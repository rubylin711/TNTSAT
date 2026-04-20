#include <linux/slab.h>

#include "mt_type.h"
#include "drv_mutils.h"
#include "drv_module_ioctl.h"
#include "mt_drv_mem.h"

#ifdef USING_KMEM
#include "common_mem_mgr.h"
#include "mem_mgr_drv.h"
#include "common_mem_base.h"
#endif

#ifdef LOG_PRIVATE
#include "common_log_base.h"
#endif

#define KERNEL_MODE "kernel"

static mt_u8* g_szPoolName[] = {"Module_Mem_Pool", "Adp_Mem_Pool", "Usr_Mem_Pool", "MMZ_Pool", "UnknowType"};


mt_handle kmem_utils_init(mt_u32 u32Count, kmem_utils_s stMem)
{
    kmem_utils_s* pUtils = NULL;
    mt_u32 u32ItemSize = 0;
    mt_handle hResult = 0;
    
    pUtils = (kmem_utils_s*)kmalloc(sizeof(kmem_utils_s), GFP_KERNEL);

    if (NULL == pUtils)
    {
        MT_ERR_MEM("<%s:%s> malloc %d size failure!\n", KERNEL_MODE, __func__, u32Count * u32ItemSize);

        return 0;
    }

    memset(pUtils, 0, sizeof(kmem_utils_s));
    pUtils->enType = stMem.enType;
    pUtils->u32ItemCount = u32Count;

    switch(stMem.enType)
    {
        case KMEM_POOL_TYPE_MODULE:
            u32ItemSize = sizeof(module_pool_s)*u32Count;            
        break;

#ifdef USING_KMEM
        case KMEM_POOL_TYPE_MODULE_MEMORY:
            u32ItemSize = sizeof(module_mem_pool_s)*u32Count;
        break;
        case KMEM_POOL_TYPE_USR_MEMORY:
            u32ItemSize = sizeof(usr_mem_pool_s)*u32Count;
        break;
#endif

#ifdef LOG_PRIVATE
        case KMEM_POOL_TYPE_LOG:
            u32ItemSize = sizeof(module_log_pool_s)*u32Count;
        break;
#endif
        default:
            pUtils->enType = KMEM_POOL_TYPE_BUTT;
        break;;
    }

    pUtils->pMemBaseAddr = kmalloc(u32ItemSize, GFP_KERNEL);
    if (NULL == pUtils->pMemBaseAddr)
    {
        MT_ERR_MEM("<%s:%s> , failed to request %s memory size %d\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], u32ItemSize);
        kfree(pUtils);
    }
    else
    {
        memset(pUtils->pMemBaseAddr, 0, u32ItemSize);
        hResult = (mt_handle)pUtils;
        
        MT_INFO_MEM("<%s:%s>  successfully, request %s, and base address is %p\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], pUtils->pMemBaseAddr);
    }
    
    return hResult;
}

mt_void kmem_utils_deinit(mt_handle hUtils)
{
    kmem_utils_s* pUtils = (kmem_utils_s*)hUtils;
    
    if (NULL != pUtils)
    {
        if (NULL != pUtils->pMemBaseAddr)
        {
            MT_INFO_MEM("<%s:%s>  free %s memory, and address is %p\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], pUtils->pMemBaseAddr);
            kfree(pUtils->pMemBaseAddr);
        }
        
        kfree((mt_void*)hUtils);
    }
}

#if 0
MT_U32 KMem_Utils_GetItemNo(mt_handle hUtils)
{
    KMEM_UTILS_S* pUtils = (KMEM_UTILS_S*)hUtils;
    
    if (NULL != pUtils)
    {
        return pUtils->u32HasCount;
    }

    return 0;
}
#endif


mt_void* kmem_utils_malloc(mt_handle hUtils)
{
    mt_u32 u32Index = 0;

    kmem_utils_s* pUtils = (kmem_utils_s*)hUtils;
    mt_void* pResult = NULL;

    module_pool_s * pModuleBase = NULL;
    
#ifdef USING_KMEM
    module_mem_pool_s*     pMemAdpBase = NULL;
    usr_mem_pool_s*     pMemBase    = NULL;
#endif

#ifdef LOG_PRIVATE
    module_log_pool_s*  pLogBase    = NULL;
#endif

    if (NULL != pUtils && pUtils->pMemBaseAddr != NULL)
    {
        MT_INFO_MEM("pool type:%d, maxsize:%u, size:%u, membase:0x%x.\n", pUtils->enType, pUtils->u32ItemCount, pUtils->u32HasCount,  pUtils->pMemBaseAddr);
                
        switch(pUtils->enType)
        {
            case KMEM_POOL_TYPE_MODULE:
            {
                pModuleBase = (module_pool_s*)pUtils->pMemBaseAddr;
            }
            break;

#ifdef USING_KMEM
            case KMEM_POOL_TYPE_MODULE_MEMORY:
            {
                pMemAdpBase = (module_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            case KMEM_POOL_TYPE_USR_MEMORY:
            {
                pMemBase = (usr_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
#endif

#ifdef LOG_PRIVATE
            case KMEM_POOL_TYPE_LOG:
            {
                pLogBase = (module_log_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
#endif
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
                
                MT_INFO_MEM("<%s:%s> ... request %s and address[%d] is %p!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], u32Index, pResult);
                break;
            }
            
#ifdef USING_KMEM

            if (NULL != pMemAdpBase && pMemAdpBase[u32Index].u32Idle == 0)
            {
                pMemAdpBase[u32Index].u32Idle = 1;

                pResult = &pMemAdpBase[u32Index];

                pUtils->u32HasCount++;
                
                MT_INFO_MEM("<%s:%s> ... request %s and address[%d] is %p!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], u32Index, pResult);
                break;
            }

            if (NULL != pMemBase && pMemBase[u32Index].u32Idle == 0)
            {
                pMemBase[u32Index].u32Idle = 1;

                pResult = &pMemBase[u32Index];

                pUtils->u32HasCount++;
                
                MT_INFO_MEM("<%s:%s> ... request %s and address[%d] is %p!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], u32Index, pResult);
                break;
            }
#endif

#ifdef LOG_PRIVATE
            if (NULL != pLogBase && pLogBase[u32Index].u32Idle == 0)
            {
                pLogBase[u32Index].u32Idle = 1;

                pResult = &pLogBase[u32Index];

                pUtils->u32HasCount++;
                
                MT_INFO_MEM("<%s:%s> ... request %s and address[%d] is %p!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], u32Index, pResult);
                break;
            }
#endif
        }

        return pResult;
    }


    MT_ERR_MEM("<%s:%s> ... param is invalid!\n",KERNEL_MODE, __func__);
    
    return NULL;
}

mt_void kmem_utils_free(mt_handle hUtils, mt_void* pAddr)
{
    kmem_utils_s* pUtils = (kmem_utils_s*)hUtils;
    mt_u32 u32Index = 0;

    module_pool_s* pModuleBase = NULL;

#ifdef USING_KMEM
    module_mem_pool_s*    pMemAdpBase = NULL;
    usr_mem_pool_s*    pMemBase    = NULL;
#endif

#ifdef LOG_PRIVATE
    module_log_pool_s* pLogBase    = NULL;
#endif

    if (NULL != pUtils && pUtils->pMemBaseAddr != NULL)
    {
        MT_INFO_MEM("pool type:%d, maxsize:%u, size:%u, addr:0x%x.\n", pUtils->enType, pUtils->u32ItemCount, pUtils->u32HasCount,  pAddr);
        
        switch(pUtils->enType)
        {
            case KMEM_POOL_TYPE_MODULE:
            {
                pModuleBase = (module_pool_s*)pUtils->pMemBaseAddr;
            }
            break;

#ifdef USING_KMEM
            case KMEM_POOL_TYPE_MODULE_MEMORY:
            {
                pMemAdpBase = (module_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
            case KMEM_POOL_TYPE_USR_MEMORY:
            {
                pMemBase = (usr_mem_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
#endif

#ifdef LOG_PRIVATE
            case KMEM_POOL_TYPE_LOG:
            {
                pLogBase = (module_log_pool_s*)pUtils->pMemBaseAddr;
            }
            break;
#endif
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
                
                MT_INFO_MEM("<%s:%s>... idle %s and address is %p, successfully!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], pAddr);
                break;
            }

#ifdef USING_KMEM
            if (NULL != pMemAdpBase && (&pMemAdpBase[u32Index] == pAddr) )
            {
                pMemAdpBase[u32Index].u32Idle = 0;

                memset(&pMemAdpBase[u32Index], 0, sizeof(pMemAdpBase[u32Index]));

                pUtils->u32HasCount--;
                
                MT_INFO_MEM("<%s:%s>... idle %s and address is %p, successfully!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], pAddr);
                break;
            }

            if (NULL != pMemBase && (&pMemBase[u32Index] == pAddr) )
            {
                pMemBase[u32Index].u32Idle = 0;

                memset(&pMemBase[u32Index], 0, sizeof(pMemBase[u32Index]));

                pUtils->u32HasCount--;
                
                MT_INFO_MEM("<%s:%s>... idle %s and address is %p, successfully!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], pAddr);
                break;
            }
#endif

#ifdef LOG_PRIVATE
            if (NULL != pLogBase && (&pLogBase[u32Index] == pAddr) )
            {
                pLogBase[u32Index].u32Idle = 0;

                memset(&pLogBase[u32Index], 0, sizeof(pLogBase[u32Index]));

                pUtils->u32HasCount--;
                
                MT_INFO_MEM("<%s:%s>... idle %s and address is %p, successfully!\n", KERNEL_MODE, __func__, g_szPoolName[pUtils->enType], pAddr);
                break;
            }
#endif
        }

        return ;
    }


    MT_ERR_MEM("<%s:%s>... idle %s and address is %p, failure!!!\n", KERNEL_MODE, __func__, pUtils ? g_szPoolName[pUtils->enType] : g_szPoolName[KMEM_POOL_TYPE_BUTT], pAddr);
    
    return;

}

