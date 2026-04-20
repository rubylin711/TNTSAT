#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "mt_type.h"
#include "mt_mpi_mem.h"

#ifdef CMN_MMGR_SUPPORT
#include "mt_drv_mem.h"
#include "drv_module_ioctl.h"
#include "mpi_mem_base.h"
#include "mpi_mutils.h"


#define HLEN sizeof(struct head)

static struct head *first = NULL;
static struct head *last = NULL;

//!! Reserved
mt_u8* mem_file_name = NULL;
mt_u32 mem_line_number = 0;

//!! Global variable
static pthread_mutex_t g_MemMutex;
static fnModuleInfoCBK g_fnModuleCallback = NULL;
static MT_BOOL g_bMemInitFlag = MT_FALSE;


#define mem_lock(x) pthread_mutex_lock(x)
#define mem_unlock(x) pthread_mutex_unlock(x)


mt_handle g_hMemMgr = 0;

mt_s32 mem_pool_init(mt_u32 u32Count, fnModuleInfoCBK fnCallback)
{
    mem_utils_s stMemMgrPool = {0};
    
    if (g_bMemInitFlag == MT_TRUE)
    {
        MT_INFO_MEM("has been initialized!\n");
        return MT_SUCCESS;
    }

    MT_INFO_MEM("count is %u, and malloc request size is %d\n", u32Count, u32Count * sizeof(usr_mem_pool_s));
        
    stMemMgrPool.enType = MEM_POOL_TYPE_USR_MEMORY;

    g_hMemMgr = mem_utils_init(u32Count, stMemMgrPool);

    if (0 == g_hMemMgr)
    {
        return MT_FAILURE;
    }

    g_bMemInitFlag = MT_TRUE;

    g_fnModuleCallback = fnCallback;
        
    pthread_mutex_init(&g_MemMutex, NULL);

    return MT_SUCCESS;
}

mt_void mem_pool_deinit(mt_void)
{
    if (g_bMemInitFlag == MT_FALSE)
    {
        MT_INFO_MEM("has been deinitialized!\n");
        return;
    }
    
    mem_utils_deinit(g_hMemMgr);

    g_bMemInitFlag = MT_FALSE;

    g_fnModuleCallback = NULL;
        
    pthread_mutex_destroy(&g_MemMutex);
}

static mt_void* mem_pool_malloc(mt_u32 u32Size)
{
    return mem_utils_malloc(g_hMemMgr);
}

static mt_void mem_pool_free(mt_void* pAddr)
{
    mem_utils_free(g_hMemMgr, pAddr);

    return;
}

static struct head *mem_add(void *buf, size_t s)
{
    struct head *p = NULL;
    
    p = mem_pool_malloc(HLEN);
    if(p)
    {
        p->addr = buf;
        p->size = s;
        p->prev = last;
        p->next = NULL;

        if(last)
            last->next = p;
        else
            first = p;

        last = p;
    }

    return p;
}

static void mem_del(struct head *p)
{
    struct head *prev, *next;

    prev = p->prev;
    next = p->next;


    if(prev)
        prev->next = next;
    else
        first = next;

    if(next)
        next->prev = prev;
    else
        last = prev;

    mem_pool_free(p);
}

static void mem_replace(struct head *p, void *buf, size_t s)
{
    p->addr = buf;
    p->size = s;
}

static struct head *mem_find(void *addr)
{
    struct head *p;
    
    /* start search from lately allocated blocks */
    for(p = last; p; p = p->prev)
    {
        if(p->addr == addr)
            return p;
    }

    return NULL;
}
#endif

mt_void* mt_malloc(mt_u32 u32ModuleID, mt_u32 u32Size)
{
    mt_void* pMemAddr = NULL;

    pMemAddr = malloc(u32Size);
   
#ifdef CMN_MMGR_SUPPORT
    if (NULL != pMemAddr && g_fnModuleCallback)
    { 
        struct head* pHead = NULL;
        mt_s32 s32MallocSize = 0;
        mt_s32 s32Ret = 0;

        mem_lock(&g_MemMutex);

        //lookup the module info.
        s32Ret = g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, 0);
        if(s32Ret != MT_SUCCESS)
        {
            mem_unlock(&g_MemMutex);

            free(pMemAddr);
            
    	    return NULL;
        }

        pHead = mem_add(pMemAddr, u32Size);
        
        if(NULL != pHead)
        {
            // Add memory info to MODULE MGR
            s32MallocSize = (mt_s32)u32Size;
            
            g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, s32MallocSize);

            mem_unlock(&g_MemMutex);
            
    	    return pMemAddr;
        }
        else
        {
            mem_unlock(&g_MemMutex);

    	    free(pMemAddr);

            return NULL;
        }
    }
#endif

    return pMemAddr;
}

mt_void mt_free(mt_u32 u32ModuleID, mt_void* pMemAddr)
{
#ifdef CMN_MMGR_SUPPORT
    if (NULL != pMemAddr && g_fnModuleCallback)
    {
        struct head *p = NULL;
        mt_s32 s32MallocSize = 0;

        mem_lock(&g_MemMutex);

        p = mem_find(pMemAddr);

        if ( NULL != p )
        {
            // Update memory info for MODULE MGR 
            s32MallocSize = (mt_s32)p->size;
            s32MallocSize *= -1;
            
            g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, s32MallocSize);
            
            mem_del(p);
        }

        mem_unlock(&g_MemMutex);      
    }
#endif
 
    free(pMemAddr);   
    return;
}

mt_void* mt_calloc(mt_u32 u32ModuleID, mt_u32 u32MemBlock, mt_u32 u32Size)
{
    mt_void* pMemAddr = NULL;

    pMemAddr = calloc(u32MemBlock, u32Size);
    
#ifdef CMN_MMGR_SUPPORT
    if (NULL != pMemAddr && g_fnModuleCallback)
    {
        struct head* pHead = NULL;
        mt_s32 s32MallocSize = 0;
        mt_s32 s32Ret = MT_FAILURE;

        mem_lock(&g_MemMutex);

        //lookup the module info.
        s32Ret = g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, 0);
        if(s32Ret != MT_SUCCESS)
        {
            mem_unlock(&g_MemMutex);

            free(pMemAddr);
            
    	    return NULL;
        }

        pHead = mem_add(pMemAddr, u32Size*u32MemBlock);
        
        if(NULL != pHead)
        {
            // Add memory info to MODULE MGR
            s32MallocSize = (mt_s32)pHead->size;
            g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, s32MallocSize);

            mem_unlock(&g_MemMutex);
            
    	    return pMemAddr;
        }
        else
        {
            mem_unlock(&g_MemMutex);
            
    	    free(pMemAddr);

            return NULL;
        }
    }
#endif

    return pMemAddr;
}

mt_void* mt_realloc(mt_u32 u32ModuleID, mt_void *pMemAddr, mt_u32 u32Size)
{
    mt_void* pNewMemAddr = NULL;

#ifdef CMN_MMGR_SUPPORT
    struct head *p;
    
    if (NULL != pMemAddr && g_fnModuleCallback)/* when pMemAddr is NULL do malloc.*/
    {
        // Add memory info to MODULE MGR
        if (0 == u32Size)/* when s = 0 realloc() acts like free(). */
        {
            mt_free(u32ModuleID, pMemAddr);
        }
        else
        {
            mt_s32 s32Ret = MT_FAILURE;
            
            mem_lock(&g_MemMutex);

            //lookup the module info.
            s32Ret = g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, 0);
            if(s32Ret != MT_SUCCESS)
            {
                mem_unlock(&g_MemMutex);
                
        	    return NULL;
            }
            
            p = mem_find(pMemAddr);
            if (NULL != p)
            {
                pNewMemAddr = realloc(pMemAddr, u32Size);
                if ( NULL != pNewMemAddr)
                {
                    g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, (-1)*(p->size));

                    mem_replace(p, pNewMemAddr, u32Size);
                    
                    g_fnModuleCallback(u32ModuleID, MEM_TYPE_USR, u32Size);

                    mem_unlock(&g_MemMutex);
                    
	                return pNewMemAddr;
                }
            }

            mem_unlock(&g_MemMutex);
        }
    }
    else
    {
        return mt_malloc(u32ModuleID, u32Size);
    }
#else
    pNewMemAddr = realloc(pMemAddr, u32Size);
#endif

    return pNewMemAddr;

}

