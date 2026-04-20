/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "mt_common.h"
#include "mt_module.h"
#include "mt_mpi_mem.h"
#include "mt_drv_memdev.h"
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_sanitize.h"

#define OS_LINUX

//#define WRITE_LOG_ERROR(fmt...) MT_ERR_PRINT(MT_ID_MEM, fmt)
//#define WRITE_LOG_INFO(fmt...)  MT_INFO_PRINT(MT_ID_MEM, fmt)
#define WRITE_LOG_ERROR MT_ERR_LOG
#define WRITE_LOG_INFO  printf

#ifdef OS_LINUX
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct tag_MMAP_Node
{
    phys_addr_t Start_P;
    void *Start_V;
    size_t length;
    unsigned int refcount;  /*the count of the memory after mapped*//*CNcomment: map后的空间段的引用计数 */
    struct tag_MMAP_Node * next;
}mmap_node_t;

mmap_node_t * pMMAPNode = NULL;

#define mmmp_dev "/dev/"UMAP_DEVNAME_MEMDEV

#endif

static pthread_mutex_t   g_MemmapMutex = PTHREAD_MUTEX_INITIALIZER;
#define MT_MEMMAP_LOCK()  	 (void)pthread_mutex_lock(&g_MemmapMutex);
#define MT_MEMMAP_UNLOCK()   (void)pthread_mutex_unlock(&g_MemmapMutex);

static mt_s32 g_s32_cache_fd = -1;
static mt_s32 g_s32_uncache_fd = -1;
static mt_s32 memdevicecheckopen(mt_void)
{
	MT_MEMMAP_LOCK();

	if(g_s32_uncache_fd < 0)
	{
		int flag = O_RDWR | O_NONBLOCK | O_SYNC | O_CLOEXEC;	/*without cache*/

		/* dev not opened yet, so open it */
		g_s32_uncache_fd = open(mmmp_dev, flag);
		if (g_s32_uncache_fd < 0)
			{
			perror("memmap uncache open\n");
			MT_MEMMAP_UNLOCK() ;
			WRITE_LOG_ERROR("memmap():uncache open %s error!\n", mmmp_dev);
			return MT_FAILURE;
		}
	}

	if(g_s32_cache_fd < 0)
	{
		int flag = O_RDWR | O_NONBLOCK | O_CLOEXEC;		/*with cache*/

		/* dev not opened yet, so open it */
		g_s32_cache_fd = open(mmmp_dev, flag);
		if (g_s32_cache_fd < 0)
		{
			perror("memmap cache open\n");
			MT_MEMMAP_UNLOCK() ;
			WRITE_LOG_ERROR("memmap():cache open %s error!\n", mmmp_dev);
			return MT_FAILURE;
		}
	}

	MT_MEMMAP_UNLOCK();

	return MT_SUCCESS;
}

/*below is the macro used frequently*/
#define CHECK_MEM_OPEN_STATE()    \
	do {  \
		if (MT_SUCCESS != memdevicecheckopen()) return MT_FAILURE; \
	} while(0)

#define CHECK_MEM_OPEN_STATE2()    \
	do {  \
		if (MT_SUCCESS != memdevicecheckopen()) return MT_NULL; \
	} while(0)



/* no need considering page_size of 4K */
static mt_void *mt_mmap_base(phys_addr_t phy_addr, ulong size, mt_u8 cached)
{
	int mmmp_fd;
#ifndef OS_LINUX
    return (void *)phy_addr;
#else
    phys_addr_t phy_addr_in_page;
    phys_addr_t page_diff;

    size_t size_align_page;

    mmap_node_t * pTmp;
    mmap_node_t * pNew;

    void *addr = NULL;

	CHECK_MEM_OPEN_STATE2();
	mmmp_fd = cached ? g_s32_cache_fd : g_s32_uncache_fd;

    if(size == 0 || phy_addr == 0)
    {
        WRITE_LOG_ERROR("memmap():size or addr can't be zero!\n");
        mt_backtrace();
        return NULL;
    }

    MT_MEMMAP_LOCK() ;

    /* addr align in page_size(4K) */
    phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
    page_diff = phy_addr - phy_addr_in_page;

    /* size in page_size */
    size_align_page = ((size + page_diff - 1) & PAGE_SIZE_MASK) + MEM_PAGE_SIZE;

#ifdef ANDROID
	addr = mt_mmap_alias(NULL, size_align_page + (2 * PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mmmp_fd, (off_t)phy_addr_in_page);
#else
	addr = mmap(NULL, size_align_page + (2 * PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mmmp_fd, (off_t)phy_addr_in_page);
#endif
    if (addr == MAP_FAILED)
    {
        perror("memmap error\n");
        MT_MEMMAP_UNLOCK() ;
        WRITE_LOG_ERROR("memmap():mmap @ 0x%x error!\n", phy_addr_in_page);
        mt_backtrace();

        return NULL;
    }

    /* add this mmap to MMAP Node */
    pNew = (mmap_node_t *)mt_malloc(MT_ID_MEM, sizeof(mmap_node_t));
    if(NULL == pNew)
    {
		munmap(addr, size_align_page + (2 * PAGE_SIZE));
        MT_MEMMAP_UNLOCK() ;
        WRITE_LOG_ERROR("memmap():malloc new node failed!\n");
        return NULL;
    }
	addr += PAGE_SIZE;
    pNew->Start_P = phy_addr_in_page;
    pNew->Start_V = addr;
    pNew->length = size_align_page;
    pNew->refcount = 1;
    pNew->next = NULL;

    if(pMMAPNode == NULL)
    {
        pMMAPNode = pNew;
    }
    else
    {
        pTmp = pMMAPNode;
        while(pTmp->next != NULL)
        {
            pTmp = pTmp->next;
        }

        pTmp->next = pNew;
    }

    //printf("\nmemmap %p to %p\n", (void *)phy_addr, (void*)(addr+page_diff));

	MT_MEMMAP_UNLOCK() ;

#ifdef CONFIG_MT_SANITIZE_TAG
	return __tag_set(addr, mt_hwasan_tag_memory(addr, size_align_page)) + page_diff;
#else
    return addr + page_diff;
#endif
#endif
}

mt_void *mt_mmap(phys_addr_t phy_addr, ulong size)
{
	return mt_mmap_base(phy_addr, size, 0);	/*without cache*/
}

mt_void *mt_mmap_cache(phys_addr_t phy_addr, ulong size)
{
	return mt_mmap_base(phy_addr, size, 1);	/*with cache*/
}

/*****************************************************************************
 Prototype    : memunmap
 Description  :
 Input        : void * addr_mapped
 Output       : None
 Return Value : On success, returns 0, on failure -1
 Calls        :
 Called By    :

  History        :
  1.Date         : 2005/12/21
    Author       : Z42136
    Modification : Created function

*****************************************************************************/
mt_s32 mt_munmap(void *addr_mapped)
{
    mmap_node_t * pPre;
    mmap_node_t * pTmp;

    if(pMMAPNode == NULL)
    {
        WRITE_LOG_ERROR("memunmap(): address have not been mmaped!\n");
        return -1;
    }

    MT_MEMMAP_LOCK() ;

    /* check if the physical memory space have been mmaped */
    pTmp = pMMAPNode;
    pPre = pMMAPNode;

#ifdef CONFIG_MT_SANITIZE_TAG
	addr_mapped = untagged_addr(addr_mapped);
#endif

    do
    {
        if( (addr_mapped >= pTmp->Start_V) &&
            (addr_mapped < (pTmp->Start_V + pTmp->length)))/*modify by jianglei(40671), should be '<' instead of '<=' */
        {
            pTmp->refcount--;   /* referrence count decrease by 1  */
            if(0 == pTmp->refcount)
            {
            	/*when the count is 0 the mapped memory is not in use, use memunmap to reclaim*/
                /*CNcomment:引用计数变为0, 被map的内存空间不再使用,此时需要进行munmap回收 */

                //WRITE_LOG_INFO("memunmap(): map node will be remove:0x%x!\n", pTmp);

                /* delete this map node from pMMAPNode */
                if(pTmp == pMMAPNode)
                {
                    pMMAPNode = pTmp->next;
                }
                else
                {
                    pPre->next = pTmp->next;
                }

                /* munmap */
                if(munmap((void *)(pTmp->Start_V - PAGE_SIZE), pTmp->length + (2 * PAGE_SIZE)) != 0 )
                {
                    /* Don't call LOG print between MT_MEMMAP_LOCK and MT_MEMMAP_UNLOCK */
                    //WRITE_LOG_INFO("memunmap(): munmap failed!\n");
                }
                else
                {
#ifdef CONFIG_MT_SANITIZE_TAG
					mt_hwasan_tag_clean(pTmp->Start_V, pTmp->length);
#endif
                }

                mt_free(MT_ID_MEM, pTmp);
            }

		    MT_MEMMAP_UNLOCK() ;
            return 0;
        }

        pPre = pTmp;
        pTmp = pTmp->next;
    }while(pTmp != NULL);

    MT_MEMMAP_UNLOCK() ;
    WRITE_LOG_ERROR("memunmap(): address have not been mmaped!\n");
    return -1;
}

#ifdef ANDROID
/*
 * android bionic mmap()'s "off_t offset" argument is signed long,
 * if (unsigned)offset >= 0x80000000, that's (signed)offset <0, mmap() return MAP_FAILED!
 * so shall call mmap64() instead.
 *
 * Note: call mt_mmap_alias() instead of mmap()!
 */
void* mt_mmap_alias(void* addr, size_t size, int prot, int flags, int fd, off_t offset)
{
  off64_t offset64 = 0;
  if (((unsigned long)offset & 0x80000000UL) != 0) {
    offset64 = offset & 0x7FFFFFFFUL;
    offset64 |= 0x0000000080000000UL;
  } else {
    offset64 = offset;
  }
  return mmap64(addr, size, prot, flags, fd, offset64);
}
#endif

mt_s32 mt_flush(void *virtaddr, ulong size)
{
	struct cache_op_mem_area coa;

#ifdef CONFIG_MT_SANITIZE_TAG
	virtaddr = untagged_addr(virtaddr);
#endif

	CHECK_MEM_OPEN_STATE();

	coa.virtaddr = virtaddr;
	coa.size = size;
	return ioctl(g_s32_cache_fd, MT_MEM_FLUSH_DCACHE, &coa);
}

mt_s32 mt_invalidate(void *virtaddr, ulong size)
{
	struct cache_op_mem_area coa;

#ifdef CONFIG_MT_SANITIZE_TAG
	virtaddr = untagged_addr(virtaddr);
#endif

	CHECK_MEM_OPEN_STATE();

	coa.virtaddr = virtaddr;
	coa.size = size;
	return ioctl(g_s32_cache_fd, MT_MEM_INV_DCACHE, &coa);
}

mt_s32 mt_get_pageinfo(mt_u32 *page_block_order, mt_u32 *pages_per_block)
{
	static struct get_mem_pageinfo info;
	static int first_fetch = 1;

	if (first_fetch == 1) {
		CHECK_MEM_OPEN_STATE();
		ioctl(g_s32_cache_fd, MT_MEM_GET_PAGEINFO, &info);
		first_fetch = 0;
	}
	if (page_block_order) {
		*page_block_order = info.page_block_order;
	}
	if (pages_per_block) {
		*pages_per_block = info.pages_per_block;
	}

	return 0;
}

int mt_get_phys_addr(void *vaddr, phys_addr_t *phy_addr)
{
	struct walk_get_phys_addr get_pa = {
#ifdef CONFIG_MT_SANITIZE_TAG
		.user_vaddr = (ulong)untagged_addr(vaddr),
#else
		.user_vaddr = (ulong)vaddr,
#endif
		.phys_addr = -1,
	};

	CHECK_MEM_OPEN_STATE();

	if (get_pa.user_vaddr) {
		ioctl(g_s32_cache_fd, MT_MEM_WALK_GET_PHYS_ADDR, &get_pa);
	}
	*phy_addr = get_pa.phys_addr;
	return 0;
}

int mt_walk_watch(void *user_addr, u32 size, u64 right_value)
{
	struct walk_watch ww = {
#ifdef CONFIG_MT_SANITIZE_TAG
		.user_vaddr = (ulong)untagged_addr(user_addr),
#else
		.user_vaddr = (ulong)user_addr,
#endif
		.right_value = right_value,
		.size = size,
	};

	CHECK_MEM_OPEN_STATE();

	if (size <= sizeof(u64)) {
		ioctl(g_s32_cache_fd, MT_MEM_WALK_WATCH, &ww);
	}
	return 0;
}
