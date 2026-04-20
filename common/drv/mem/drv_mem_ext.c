#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <linux/fs.h>

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_drv_mem.h"
#include "drv_mmgr.h"
#include "drv_mem.h"

typedef struct tagKMEM_INFO_S
{
    mt_void* pMemAddr;
    mt_u32   u32Size;
}kmem_info_s;

typedef struct tagKMEM_ITEM_S
{
    kmem_info_s stMemInfo;

    struct tagKMEM_ITEM_S* pNext;
}kmem_item_s;

typedef struct tagKMEM_POOL_S
{
    mt_u32 u32Idle;

    kmem_item_s stMemItem;
}kmem_pool_s;

static kmem_pool_s* g_KMemBaseAddr = NULL;
static kmem_item_s* g_KMemHeader = NULL;
static mt_u32       g_u32KMemItemCount = 0;

static fnKMemStatCallback g_fnKMemCallback = NULL;

MT_DECLARE_MUTEX(g_KMemMutex);

#define kmem_lock(RET, func, param)                       \
do{                                                       \
    mt_s32 s32LockRet = down_interruptible(&g_KMemMutex); \
    if ( s32LockRet != 0 )                                \
    {                                                     \
        func(param);                                      \
        return RET;                                       \
    }                                                     \
}while(0)

#define kmem_lock_nonret()                                \
do{                                                       \
    mt_s32 s32LockRet = down_interruptible(&g_KMemMutex); \
    if ( s32LockRet != 0 )                                \
    {                                                     \
        return;                                           \
    }                                                     \
}while(0)


#define kmem_unlock() up(&g_KMemMutex)


mt_s32 kmem_pool_init(mt_u32 u32Count)
{
    if (NULL == g_KMemBaseAddr)
    {
        g_KMemBaseAddr = (kmem_pool_s*)kmalloc((u32Count+1) * sizeof(kmem_pool_s), GFP_KERNEL);

        if (NULL == g_KMemBaseAddr)
        {
            MT_ERR_MEM("<%s> malloc %d size failure!\n", __func__, u32Count * sizeof(kmem_pool_s));

            return MT_FAILURE;
        }

        memset(g_KMemBaseAddr, 0, (u32Count+1) * sizeof(kmem_pool_s));

        g_KMemHeader = &g_KMemBaseAddr[0].stMemItem;

        g_KMemBaseAddr[0].u32Idle = 1;

        g_u32KMemItemCount = u32Count+1;
    }

    return MT_SUCCESS;
}

mt_void kmem_pool_deinit(mt_void)
{
    if (NULL != g_KMemBaseAddr)
    {
        kfree(g_KMemBaseAddr);

        g_KMemHeader = NULL;

        g_KMemBaseAddr = NULL;

        g_u32KMemItemCount = 0;
    }
}

static mt_void* kmem_pool_malloc(mt_u32 u32Size)
{
    mt_u32 u32Index = 0;

    for (u32Index=1; u32Index<g_u32KMemItemCount; u32Index++)
    {
        if (g_KMemBaseAddr[u32Index].u32Idle == 0)
        {
            g_KMemBaseAddr[u32Index].u32Idle = 1;
            break;
        }
    }

    if (u32Index != g_u32KMemItemCount)
    {
        return &g_KMemBaseAddr[u32Index].stMemItem;
    }

    return NULL;
}

static mt_void kmem_pool_free(mt_void* pAddr)
{
    mt_u32 u32Index = 0;

    for (u32Index=1; u32Index<g_u32KMemItemCount; u32Index++)
    {
        if (&g_KMemBaseAddr[u32Index].stMemItem == pAddr)
        {
            g_KMemBaseAddr[u32Index].u32Idle = 0;

            memset(&g_KMemBaseAddr[u32Index].stMemItem, 0, sizeof(g_KMemBaseAddr[u32Index].stMemItem));
            break;
        }
    }

    return;
}

mt_s32 kmem_find_node(kmem_item_s* pNodeHeader, mt_void* pAddr, kmem_info_s** pstNode)
{
    kmem_item_s* pItrNode = NULL;

    if (NULL == pNodeHeader || pstNode == NULL)
    {
        MT_ERR_MEM("<%s>: find node failure, node header is NULL\n", __func__);

        return MT_FAILURE;
    }

    pItrNode = pNodeHeader->pNext;

    while ( NULL != pItrNode )
    {
        if (pItrNode->stMemInfo.pMemAddr == pAddr)
        {
            break;
        }

        pItrNode = pItrNode->pNext;
    }

    if (pItrNode != NULL)
    {
        *pstNode = &pItrNode->stMemInfo;

        return MT_SUCCESS;
    }

    return MT_FAILURE;

}

// Make one node and add it to the link tail
mt_s32 kmem_add_node(kmem_item_s* pNodeHeader, kmem_item_s* pstNode)
{
    kmem_item_s* pItrNode = NULL;
    kmem_item_s* pstAddNode = NULL;

    if (NULL == pNodeHeader || pstNode == NULL)
    {
        MT_ERR_MEM("<%s>: add node failure, node header is NULL\n", __func__);

        return MT_FAILURE;
    }

    pstAddNode = (kmem_item_s*)kmem_pool_malloc(sizeof(kmem_item_s));
    if ( NULL == pstAddNode)
    {
        MT_ERR_MEM("<%s>: add node failure, malloc node failure.\n", __func__);
        return MT_FAILURE;
    }

    memset(pstAddNode, 0, sizeof(kmem_item_s));

    pstAddNode->stMemInfo.pMemAddr = pstNode->stMemInfo.pMemAddr;
    pstAddNode->stMemInfo.u32Size  = pstNode->stMemInfo.u32Size;
    pstAddNode->pNext = NULL;

    pItrNode = pNodeHeader;

    while ( NULL != pItrNode->pNext )
    {
        pItrNode = pItrNode->pNext;
    }

    pItrNode->pNext = pstAddNode;

    return MT_SUCCESS;
}

// Delete the node from link
mt_s32 kmem_del_node(kmem_item_s* pNodeHeader, kmem_item_s* pstNode)
{
    kmem_item_s* pItrNode = NULL;
    kmem_item_s* pDelNode = NULL;

    if (NULL == pNodeHeader || pNodeHeader->pNext == NULL)
    {
        MT_ERR_MEM("<%s>:delete node failure, node header is NULL\n", __func__);

        return MT_FAILURE;
    }

    pItrNode = pNodeHeader;

    //if pstNode is NULL, delete all the node, exclusive the header node;
    if (NULL == pstNode)
    {
        pItrNode = pItrNode->pNext;

        while ( NULL != pItrNode )
        {
            pDelNode = pItrNode;
            pItrNode = pItrNode->pNext;

            MT_INFO_MEM("<%s>: delete module addr:%p\n", __func__, pDelNode->stMemInfo.pMemAddr);

            kmem_pool_free(pDelNode);
        }

        pNodeHeader->pNext = NULL;

        return MT_SUCCESS;
    }

    while ( NULL != pItrNode->pNext)
    {
        if (pItrNode->pNext->stMemInfo.pMemAddr == pstNode->stMemInfo.pMemAddr)
        {
            break;
        }
        pItrNode = pItrNode->pNext;
    }

    if (NULL != pItrNode && NULL != pItrNode->pNext)
    {
        //found out the next node to delete.
        pDelNode = pItrNode->pNext;
        pItrNode->pNext = pItrNode->pNext->pNext;

        MT_INFO_MEM("<%s>: delete module node:%p\n", __func__, pDelNode->stMemInfo.pMemAddr);

        kmem_pool_free(pDelNode);

        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

mt_void* mt_kmalloc(mt_u32 module_id, mt_u32 size, mt_s32 flags)
{
    void* pkAddr = kmalloc(size, flags);
    mt_s32 s32Ret = MT_FAILURE;

    if (NULL != pkAddr)
    {
        kmem_item_s stItem = {{0}};

        //TODO: add kmalloc info into the module manager.
        stItem.stMemInfo.pMemAddr = pkAddr;
        stItem.stMemInfo.u32Size = size;

        kmem_lock(NULL, kfree, pkAddr);

        //lookup the module info.
        s32Ret = g_fnKMemCallback(module_id, KMEM_TYPE_KMEM, 0);
        if (s32Ret != MT_SUCCESS)
        {
            kfree(pkAddr);

            kmem_unlock();

            return NULL;
        }

        s32Ret = kmem_add_node(g_KMemHeader, &stItem);

        kmem_unlock();

        if (s32Ret != MT_SUCCESS)
        {
            kfree(pkAddr);

            return NULL;
        }

        s32Ret = g_fnKMemCallback(module_id, KMEM_TYPE_KMEM, size);
    }

    return pkAddr;
}

mt_void mt_kfree(mt_u32 module_id, mt_void *ptr)
{
    if (NULL != ptr)
    {
        kmem_item_s stItem = {{0}};
        kmem_info_s *pNode = NULL;
        mt_s32      s32Ret = MT_FAILURE;
        mt_u32     u32Size = 0;

        stItem.stMemInfo.pMemAddr = ptr;

        kfree(ptr);

        //TODO: updae pAddr memory info into module manager.

        kmem_lock_nonret();

        s32Ret = kmem_find_node(g_KMemHeader, stItem.stMemInfo.pMemAddr, &pNode);

        if (s32Ret != MT_SUCCESS)
        {
            kmem_unlock();
            return;
        }

        u32Size = pNode->u32Size;

        kmem_del_node(g_KMemHeader, &stItem);

        kmem_unlock();

        g_fnKMemCallback(module_id, KMEM_TYPE_KMEM, u32Size*(-1) );
    }
}

mt_void* mt_vmalloc(mt_u32 module_id, mt_u32 size)
{
    void* pvAddr = vmalloc(size);
    mt_s32 s32Ret = MT_FAILURE;

    if (NULL != pvAddr)
    {
        kmem_item_s stItem = {{0}};

        //TODO: Add vmalloc memory info into the module manager.
        stItem.stMemInfo.pMemAddr = pvAddr;
        stItem.stMemInfo.u32Size  = size;

        //stItem.stMemInfo.u32ModuleID = u32ModuleID;

        //lookup the module info.
        s32Ret = g_fnKMemCallback(module_id, KMEM_TYPE_VMEM, 0);
        if (s32Ret != MT_SUCCESS)
        {
            vfree(pvAddr);

            return NULL;
        }

        kmem_lock(NULL, vfree, pvAddr);

        s32Ret = kmem_add_node(g_KMemHeader, &stItem);

        kmem_unlock();

        if (s32Ret != MT_SUCCESS)
        {
            vfree(pvAddr);

            return NULL;
        }

        g_fnKMemCallback(module_id, KMEM_TYPE_VMEM, size);
    }

    return pvAddr;
}

mt_void mt_vfree(mt_u32 module_id, mt_void *ptr)
{
    if (NULL != ptr)
    {
        kmem_item_s stItem = {{0}};
        kmem_info_s *pNode = NULL;
        mt_s32      s32Ret = MT_FAILURE;
        mt_u32      u32Size = 0;

        stItem.stMemInfo.pMemAddr = ptr;

        vfree(ptr);

        //TODO: update pAddr memory info into the module manager.

        kmem_lock_nonret();

        s32Ret = kmem_find_node(g_KMemHeader, stItem.stMemInfo.pMemAddr, &pNode);

        if (s32Ret != MT_SUCCESS)
        {
            kmem_unlock();
            return;
        }

        u32Size = pNode->u32Size;

        kmem_del_node(g_KMemHeader, &stItem);

        kmem_unlock();

        g_fnKMemCallback(module_id, KMEM_TYPE_VMEM, u32Size*(-1) );
    }
}

mt_s32 kmodule_mem_init(mt_u32 u32Count, fnKMemStatCallback fnCallback)
{
    mt_s32 s32Ret = kmem_pool_init(u32Count);

    g_fnKMemCallback = fnCallback;

    return s32Ret;
}

mt_s32 kmodule_mem_deinit(mt_void)
{
    kmem_pool_deinit();

    g_fnKMemCallback = NULL;

    return MT_SUCCESS;
}

EXPORT_SYMBOL(mt_kmalloc);
EXPORT_SYMBOL(mt_kfree);
EXPORT_SYMBOL(mt_vmalloc);
EXPORT_SYMBOL(mt_vfree);

