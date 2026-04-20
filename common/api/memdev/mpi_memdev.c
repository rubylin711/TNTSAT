

/******************************* Include Files *******************************/

/* Sys headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include "list.h"

/* Unf headers */
#include "mt_common.h"

/* Drv headers */
#include "mt_drv_memdev.h"

/* Local headers */
#include "mpi_memdev.h"
#include "mt_mpi_mem.h"
#include "mt_sanitize.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

#define MEMDEV_LOCK(Mutex)        pthread_mutex_lock(&Mutex)
#define MEMDEV_UNLOCK(Mutex)      pthread_mutex_unlock(&Mutex)

#define MEMDEV_MALLOC(size)       mt_malloc(MT_ID_MEMDEV, size)
#define MEMDEV_FREE(addr)         mt_free(MT_ID_MEMDEV, addr)

#define MEMDEV_FIND_NODE_BY_ADDR(pAddr, pMapNode) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        memdev_map_node_s* pstTmp; \
        if (!list_empty(&g_stMemdevParam.stList)) \
        { \
            list_for_each_safe(pos, n, &g_stMemdevParam.stList) \
            { \
                pstTmp = list_entry(pos, memdev_map_node_s, stNode); \
                if (pAddr == pstTmp->pVirAddr) \
                { \
                    pMapNode = pstTmp; \
                    break; \
                } \
            } \
        } \
    }


/*************************** Structure Definition ****************************/

typedef struct tagMEMDEV_MAP_NODE_S
{
    mt_void* pVirAddr;
    mt_void* pActualVirAddr;
    size_t Len;
    struct list_head stNode;
}memdev_map_node_s;

typedef struct tagMEMDEV_PARAM_S
{
    mt_s32 s32Fd;
    pthread_mutex_t stMutex;
    struct list_head stList;
}memdev_param_s;

/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/

static memdev_param_s g_stMemdevParam =
{
    .s32Fd = -1,
    .stMutex = PTHREAD_MUTEX_INITIALIZER,
    .stList = {&g_stMemdevParam.stList,&g_stMemdevParam.stList}
};

/*********************************** Code ************************************/

mt_s32 mpi_memdev_init(mt_void)
{
    MEMDEV_LOCK(g_stMemdevParam.stMutex);

    if (-1 == g_stMemdevParam.s32Fd)
    {
        g_stMemdevParam.s32Fd = open("/dev/"UMAP_DEVNAME_MEMDEV, O_RDWR | O_NONBLOCK | O_SYNC | O_CLOEXEC);
        if (-1 == g_stMemdevParam.s32Fd)
        {
            MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
            MT_FATAL_MEMDEV("Open %s err!\n", UMAP_DEVNAME_MEMDEV);
            return MT_FAILURE;
        }
    }

    MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
    return MT_SUCCESS;
}

mt_s32 mpi_memdev_deinit(mt_void)
{
    MEMDEV_LOCK(g_stMemdevParam.stMutex);

    if (-1 != g_stMemdevParam.s32Fd)
    {
        close(g_stMemdevParam.s32Fd);
        g_stMemdevParam.s32Fd = -1;
    }

    MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
    return MT_SUCCESS;
}

mt_s32 mpi_memdev_write_register(phys_addr_t RegAddr, mt_u32 u32Value)
{
    mt_s32 s32Ret;
    mt_void *pVirAddr;

    s32Ret = mpi_memdev_map_register(RegAddr, 4, &pVirAddr);
    if (MT_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

	mpi_write_reg32(pVirAddr, u32Value);
    mpi_memdev_unmap_register(pVirAddr);
    return MT_SUCCESS;
}

mt_s32 mpi_memdev_read_register(phys_addr_t RegAddr, mt_u32 *pu32Value)
{
    mt_s32 s32Ret;
    mt_void *pVirAddr;

    if (MT_NULL == pu32Value)
    {
        return MT_FAILURE;
    }

    s32Ret = mpi_memdev_map_register(RegAddr, 4, &pVirAddr);
    if (MT_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

	*pu32Value = mpi_read_reg32(pVirAddr);
    mpi_memdev_unmap_register(pVirAddr);
    return MT_SUCCESS;
}

mt_s32 mpi_memdev_map_register(phys_addr_t RegAddr, mt_u32 u32Length, mt_void **pVirAddr)
{
    memdev_map_node_s* pstMapNode = MT_NULL;
    phys_addr_t PageAddr = RegAddr & (~MEMDEV_PAGE_ALIGN_MASK);
    size_t SubAddr = RegAddr & MEMDEV_PAGE_ALIGN_MASK;
    mt_void *pvMapAddr = MT_NULL;

    MT_INFO_MEMDEV("PageAddr = %#x, SubAddr = %#x\n", PageAddr, SubAddr);

    /* Check param */
    if (MT_NULL == pVirAddr)
    {
        MT_ERR_MEMDEV("Invalid param!\n");
        return MT_FAILURE;
    }

    /* Check init */
    MEMDEV_LOCK(g_stMemdevParam.stMutex);
    if (g_stMemdevParam.s32Fd == -1)
    {
        MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
        MT_ERR_MEMDEV("MEMDEV not init!\n");
        return MT_FAILURE;
    }

    /* Alloc node */
    pstMapNode = (memdev_map_node_s*)MEMDEV_MALLOC(sizeof(*pstMapNode));
    if (MT_NULL == pstMapNode)
    {
        MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
        MT_ERR_MEMDEV("Malloc %dB fail!\n", sizeof(*pstMapNode));
        return MT_FAILURE;
    }

    /* map */
#ifdef ANDROID
    pvMapAddr = mt_mmap_alias(MT_NULL_PTR, ((u32Length + SubAddr + MEMDEV_PAGE_ALIGN_MASK) & ~MEMDEV_PAGE_ALIGN_MASK) + (2 * PAGE_SIZE),
            PROT_READ|PROT_WRITE, MAP_SHARED, g_stMemdevParam.s32Fd, (off_t)PageAddr);
#else
    pvMapAddr = mmap(MT_NULL_PTR, ((u32Length + SubAddr + MEMDEV_PAGE_ALIGN_MASK) & ~MEMDEV_PAGE_ALIGN_MASK) + (2 * PAGE_SIZE),
            PROT_READ|PROT_WRITE, MAP_SHARED, g_stMemdevParam.s32Fd, (off_t)PageAddr);
#endif
    if (pvMapAddr == MAP_FAILED)
    {
        MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
        MEMDEV_FREE(pstMapNode);
        MT_ERR_MEMDEV("Map 0x%08x failed!\n", PageAddr);
        return MT_FAILURE;
    }
	pvMapAddr += PAGE_SIZE;

    /* Save address and length to node */
    pstMapNode->pActualVirAddr = pvMapAddr;
    pstMapNode->Len = ((u32Length + SubAddr + MEMDEV_PAGE_ALIGN_MASK) & ~MEMDEV_PAGE_ALIGN_MASK);
#ifdef CONFIG_MT_SANITIZE_TAG
	/* addr and size of parameter of __hwasan_tag_memory must keep align with 16B */
	*pVirAddr = __tag_set(pvMapAddr, mt_hwasan_tag_memory(pvMapAddr, pstMapNode->Len)) + SubAddr;
#else
    *pVirAddr = pvMapAddr + SubAddr;
#endif
    pstMapNode->pVirAddr = pvMapAddr + SubAddr;

    /* Add node to list */
    list_add_tail(&(pstMapNode->stNode), &(g_stMemdevParam.stList));
    MEMDEV_UNLOCK(g_stMemdevParam.stMutex);

    MT_INFO_MEMDEV("mmap %#x(%#x) to %p(%p), len %d(%d)\n", RegAddr, PageAddr,
        pstMapNode->pVirAddr, pstMapNode->pActualVirAddr, u32Length, pstMapNode->Len);

    return MT_SUCCESS;
}

mt_s32 mpi_memdev_unmap_register(mt_void *pVirAddr)
{
    memdev_map_node_s* pstMapNode = MT_NULL;

    /* Check init */
    MEMDEV_LOCK(g_stMemdevParam.stMutex);
    if (g_stMemdevParam.s32Fd == -1)
    {
        MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
        MT_ERR_MEMDEV("MEMDEV not init!\n");
        return MT_FAILURE;
    }

#ifdef CONFIG_MT_SANITIZE_TAG
	pVirAddr = untagged_addr(pVirAddr);
#endif

    /* Find node */
    MEMDEV_FIND_NODE_BY_ADDR(pVirAddr, pstMapNode);
    if (MT_NULL == pstMapNode)
    {
        MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
        MT_ERR_MEMDEV("Invalid addr:%p!\n", pVirAddr);
        return MT_FAILURE;
    }

    /* munmap */
    munmap(pstMapNode->pActualVirAddr - PAGE_SIZE, pstMapNode->Len + (2 * PAGE_SIZE));
#ifdef CONFIG_MT_SANITIZE_TAG
	mt_hwasan_tag_clean(pstMapNode->pActualVirAddr, pstMapNode->Len);
#endif

    /* Delete node */
    list_del(&pstMapNode->stNode);

    /* Free memory */
    MEMDEV_FREE(pstMapNode);

    MEMDEV_UNLOCK(g_stMemdevParam.stMutex);
    return MT_SUCCESS;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

