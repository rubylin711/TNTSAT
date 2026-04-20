/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifdef HAVE_FTIME
#include <sys/timeb.h>
#endif
#include <pthread.h>

#include "mt_type.h"
#include "mt_common.h"
#include "drv_mmz_ioctl.h"
#include "mt_debug.h"
#include "mt_drv_struct.h"
#include "drv_mem_ioctl.h"
#include "mt_mpi_mem.h"
#include "mt_sanitize.h"

#define MMZ_DEVNAME "/dev/" UMAP_DEVNAME_MMZ
static mt_s32 g_s32fd = -1; /*mem device not open*/

static pthread_mutex_t g_mem_mutex = PTHREAD_MUTEX_INITIALIZER;

mt_s32 mt_mpi_mmz_test(mt_mmz_buf_s *pstBuf);

static mt_s32 mmzdevicecheckopen(mt_void)
{
    MEM_LOCK(&g_mem_mutex);

    if (-1 == g_s32fd)
    {
        g_s32fd = open(MMZ_DEVNAME, O_RDWR | O_CLOEXEC);
        if (-1 == g_s32fd)
        {
            MT_FATAL_MEM("Open mem device failed!\n");
            MEM_UNLOCK(&g_mem_mutex);
            return  MT_FAILURE;
        }
    }

    MEM_UNLOCK(&g_mem_mutex);

    return MT_SUCCESS;
}

#if 0
static mt_s32 mmzdevicecheckclose(mt_void)
{
    MEM_LOCK(&g_mem_mutex);

    if ( g_s32fd != -1 )
    {
        (mt_void)close(g_s32fd);
        g_s32fd = -1;
    }

    MEM_UNLOCK(&g_mem_mutex);
    MEM_LOCK_DESTROY(&g_mem_mutex);

    return MT_SUCCESS;
}
#endif

/*below is the macro used frequently*/
#define CHECK_MMZ_OPEN_STATE()    \
    do{  \
        if (MT_SUCCESS != mmzdevicecheckopen()) return MT_FAILURE; \
    }while(0)

#define CHECK_MMZ_OPEN_STATE2()    \
    do{  \
        if (MT_SUCCESS != mmzdevicecheckopen()) return NULL; \
    }while(0)

#define CHECK_MMZ_OPEN_STATE3()    \
    do{  \
        if (MT_SUCCESS != mmzdevicecheckopen()) return (phys_addr_t)(0); \
    }while(0)

/*****************************************************************************
 Prototype    : MT_MPI_MMZ_Malloc
 Description  : ...
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2006/10/9
    Author       : g45345
    Modification : Created function
*****************************************************************************/
mt_s32 mt_mpi_mmz_malloc(mt_mmz_buf_s *pstBuf)
{
	mt_s32 l_return = 0;
	struct mmb_info mmi = {0};

	CHECK_MMZ_OPEN_STATE();

	/*parameters check*/
	if (NULL == pstBuf || pstBuf->bufsize == 0 || pstBuf->bufsize > 0x40000000UL)
	{
		MT_ERR_MEM("%s:pBuf is NULL pointer!\n", __FUNCTION__);
		return MT_FAILURE;
	}

	if (strlen(pstBuf->bufname) >= MAX_BUFFER_NAME_SIZE)
	{
		MT_ERR_MEM("%s:the buffer name len is overflow!\n", __FUNCTION__);
		return MT_FAILURE;
	}

	mmi.size = pstBuf->bufsize;
	mmi.align = 0x1000;
	mmi.prot = PROT_READ | PROT_WRITE;
	mmi.flags = MAP_SHARED;
	strncpy(mmi.mmb_name, pstBuf->bufname, MTL_MMB_NAME_LEN - 1);

	/* FIXME: no mmz zone name! */

	MEM_LOCK(&g_mem_mutex);

	/*call ioctl to malloc*/
	l_return = ioctl(g_s32fd, IOC_MMB_ALLOC, &mmi);
	if (l_return != 0)
	{
		MEM_UNLOCK(&g_mem_mutex);
		return MT_FAILURE;
	}

	/*call ioctl to get info for mmap*/
	l_return = ioctl(g_s32fd, IOC_MMB_USER_REMAP, &mmi);
	if (l_return != 0)
	{
		ioctl(g_s32fd, IOC_MMB_FREE, &mmi);
		MEM_UNLOCK(&g_mem_mutex);
		return MT_FAILURE;
	}

	/* call mmap to user addr remap */
	/* map 2 more pages for overwrite debug */
	mmi.mapped = mmap(NULL, (size_t)mmi.size + (2 * PAGE_SIZE), mmi.prot, mmi.flags, g_s32fd, (off_t)mmi.phys_addr);
	if (mmi.mapped == MAP_FAILED)
	{
		MEM_UNLOCK(&g_mem_mutex);
		MT_ERR_MEM("%s: map (0x%lx, 0x%lx) failed!\n", __FUNCTION__,
					mmi.phys_addr, mmi.size);
		return MT_FAILURE;
	}
	mmi.mapped += PAGE_SIZE;

	pstBuf->phyaddr = mmi.phys_addr;
#ifdef CONFIG_MT_SANITIZE_TAG
	pstBuf->user_viraddr = __tag_set(mmi.mapped, mt_hwasan_tag_memory(mmi.mapped, mmi.size));
#else
	pstBuf->user_viraddr = mmi.mapped;
#endif
	pstBuf->overflow_threshold = 100;
	pstBuf->underflow_threshold = 0;
	pstBuf->kernel_viraddr = NULL;

	MEM_UNLOCK(&g_mem_mutex);
	return MT_SUCCESS;
}

/*****************************************************************************
 Prototype    : MT_MPI_MMZ_Free
 Description  : ...
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2006/10/9
    Author       : g45345
    Modification : Created function
*****************************************************************************/
mt_s32 mt_mpi_mmz_free(mt_mmz_buf_s *pstBuf)
{
    mt_s32 l_return = 0;
    struct mmb_info mmi = {0};

    CHECK_MMZ_OPEN_STATE();

    /*parameters check*/
    if (NULL == pstBuf || pstBuf->phyaddr == 0)
    {
        MT_ERR_MEM("pBuf is NULL pointer!\n");
        return MT_FAILURE;
    }

#ifdef CONFIG_MT_SANITIZE_TAG
	mmi.mapped = untagged_addr(pstBuf->user_viraddr);
#else
    mmi.mapped = pstBuf->user_viraddr;
#endif

    MEM_LOCK(&g_mem_mutex);

    ioctl(g_s32fd, IOC_MMB_USER_UNMAP, &mmi);

	/* mapped 2 more pages for overwrite debug */
	if (mmi.mapped != NULL && mmi.size != 0)
	{
		munmap(mmi.mapped - PAGE_SIZE, (size_t)mmi.size + (2 * PAGE_SIZE));
#ifdef CONFIG_MT_SANITIZE_TAG
		mt_hwasan_tag_clean(mmi.mapped, mmi.size);
#endif
	}

    /*call ioctl to free*/
    l_return = ioctl(g_s32fd, IOC_MMB_FREE, &mmi);
    if (l_return == MT_SUCCESS)
    {
        MEM_UNLOCK(&g_mem_mutex);
        return MT_SUCCESS;
    }
    else
    {
        MEM_UNLOCK(&g_mem_mutex);
        return MT_FAILURE;
    }
}

mt_s32 mt_mpi_mmz_test(mt_mmz_buf_s *pstBuf)
{
    mt_s32 l_return = 0;
    struct mmb_info mmi = {0};

    CHECK_MMZ_OPEN_STATE();

    /*parameters check*/
    if (NULL == pstBuf || pstBuf->phyaddr == 0)
    {
        MT_ERR_MEM("pBuf is NULL pointer!\n");
        return MT_FAILURE;
    }

    mmi.phys_addr = pstBuf->phyaddr;
#ifdef CONFIG_MT_SANITIZE_TAG
	mmi.mapped = untagged_addr(pstBuf->user_viraddr);
#else
    mmi.mapped = (void *)pstBuf->user_viraddr;
#endif

    // (mt_u8)pstBuf->kernel_viraddr = (mt_u32)remap_mmb (mmi.phys_addr);

    MEM_LOCK(&g_mem_mutex);
	//printf("====test mt_mpi_mmz_test===kernel_viraddr=0x%x=\n", (mt_u8)pstBuf->kernel_viraddr);
    /*test mmz*/
    l_return = ioctl(g_s32fd, IOC_MMB_USER_TEST_S, &mmi);
    if (l_return == MT_SUCCESS)
    {
        MEM_UNLOCK(&g_mem_mutex);
        return MT_SUCCESS;
    }
    else
    {
        MEM_UNLOCK(&g_mem_mutex);
        return MT_FAILURE;
    }
}

phys_addr_t mt_mpi_mmz_new(ulong size , mt_u32 u32Align, const mt_char *ps8mmzname, const mt_char *ps8mmbname,
						mt_mmz_security_attr_s *attr)
{
    struct mmb_info mmi = {0};
    mt_s32 ret = 0;

    CHECK_MMZ_OPEN_STATE3();

    if (size == 0)
    {
        MT_ERR_MEM("invalid size(0x%lx)!\n", size);
    	return 0;
    }

    mmi.size = size;
    mmi.align = u32Align;
	mmi.prot = PROT_READ | PROT_WRITE;
	mmi.flags = MAP_SHARED;
    if (ps8mmbname != NULL ) strncpy(mmi.mmb_name, ps8mmbname, MTL_MMB_NAME_LEN - 1);
    if (ps8mmzname != NULL ) strncpy(mmi.mmz_name, ps8mmzname, MTL_MMB_NAME_LEN - 1);

	if (attr != NULL)
	{
		/* NOTE: add all fields here: */
		mmi.attr.flags   = attr->flags;
		mmi.attr.type    = attr->type;
		mmi.attr.subtype = attr->subtype;
		memcpy(mmi.attr.reserved, attr->reserved, sizeof(mmi.attr.reserved));
	}

	MEM_LOCK(&g_mem_mutex);

    ret = ioctl(g_s32fd, IOC_MMB_ALLOC, &mmi);
    if (ret != 0)
	{
		MEM_UNLOCK(&g_mem_mutex);
        return 0;
	}
    else
    {
    	MEM_UNLOCK(&g_mem_mutex);
        return mmi.phys_addr;
	}
}

mt_s32 mt_mpi_mmz_delete(phys_addr_t physaddr)
{
    struct mmb_info mmi = {0};
	mt_s32 ret = 0;

    CHECK_MMZ_OPEN_STATE();

    if (physaddr == 0)
    {
        MT_ERR_MEM("invalid phys addr(0x%x)!\n", physaddr);
    	return MT_FAILURE;
    }

    mmi.phys_addr = physaddr;

	MEM_LOCK(&g_mem_mutex);

    ret = ioctl(g_s32fd, IOC_MMB_FREE, &mmi);

	MEM_UNLOCK(&g_mem_mutex);
	return ret;

}

/* get physical address and size by user virtual address */
mt_s32 mt_mpi_mmz_getphyaddr(void *pRefAddr, phys_addr_t *phyaddr, ulong *size)
{
    int ret = 0;
    struct mmb_info mmi = {0};

#ifdef CONFIG_MT_SANITIZE_TAG
	pRefAddr = untagged_addr(pRefAddr);
#endif

    CHECK_MMZ_OPEN_STATE();

    if (pRefAddr == NULL)
    {
        MT_ERR_MEM("user addr is null!\n");
    	return MT_FAILURE;
    }

    mmi.mapped = pRefAddr;

	MEM_LOCK(&g_mem_mutex);
    ret = ioctl(g_s32fd, IOC_MMB_USER_GETPHYADDR, &mmi);
	MEM_UNLOCK(&g_mem_mutex);
    if (ret)
    {
        return MT_FAILURE;
    }
    if (phyaddr)
    {
        *phyaddr = mmi.phys_addr;
    }
    if (size)
    {
        *size = mmi.size;
    }
    return MT_SUCCESS;
}

mt_void *mt_mpi_mmz_map(phys_addr_t physaddr, mt_u32 u32cached)
{
	struct mmb_info mmi = {0};
	mt_s32 ret = 0;

	CHECK_MMZ_OPEN_STATE2();

	MT_INFO_MEM("map addr:0x%x\n", physaddr);

	if (physaddr == 0)
	{
		MT_ERR_MEM("phys addr is null!\n");
		return NULL;
	}

	if (u32cached != 0 && u32cached != 1)
	{
		MT_ERR_MEM("invalid cache %u!\n", u32cached);
		return NULL;
	}

	mmi.phys_addr = physaddr;

	MEM_LOCK(&g_mem_mutex);

	if (u32cached)
	{
		ret = ioctl(g_s32fd, IOC_MMB_USER_REMAP_CACHED, &mmi);
		if (ret != 0)
		{
			MT_FATAL_MEM("IOC_MMB_USER_REMAP_CACHED failed, phyaddr:0x%x\n", mmi.phys_addr);
			MEM_UNLOCK(&g_mem_mutex);
			return NULL;
		}
	}
	else
	{
		ret = ioctl(g_s32fd, IOC_MMB_USER_REMAP, &mmi);
		if (ret != 0)
		{
			MT_FATAL_MEM("IOC_MMB_USER_REMAP failed, phyaddr:0x%x\n", mmi.phys_addr);
			MEM_UNLOCK(&g_mem_mutex);
			return NULL;
		}
	}

	if (mmi.mapped == NULL && mmi.phys_addr != 0 && mmi.size != 0) {
		/* map 2 more pages for overwrite debug */
		mmi.mapped = mmap(NULL, (size_t)mmi.size + (2 * PAGE_SIZE), mmi.prot, mmi.flags, g_s32fd, (off_t)mmi.phys_addr);
		if (mmi.mapped == MAP_FAILED)
		{
			MT_FATAL_MEM("IOC_MMB_USER_REMAP mmap failed, phyaddr:0x%x\n", mmi.phys_addr);
			MEM_UNLOCK(&g_mem_mutex);
			return NULL;
		}
		mmi.mapped += PAGE_SIZE;
	}

	MEM_UNLOCK(&g_mem_mutex);

	if (mmi.mapped == NULL)
	{
		MT_ERR_MEM("something is wrong: (0x%x, 0x%x)!\n", mmi.phys_addr, mmi.size);
	}

#ifdef CONFIG_MT_SANITIZE_TAG
	return __tag_set(mmi.mapped, mt_hwasan_tag_memory(mmi.mapped, mmi.size));
#else
	return mmi.mapped;
#endif
}

mt_s32 mt_mpi_mmz_unmap(void *vaddr)
{
    struct mmb_info mmi = {0};
	int ret = 0;

#ifdef CONFIG_MT_SANITIZE_TAG
	vaddr = untagged_addr(vaddr);
#endif

    CHECK_MMZ_OPEN_STATE();

	if (vaddr == 0)
	{
        MT_ERR_MEM("%s: phys addr is null!\n", __FUNCTION__);
    	return MT_FAILURE;
	}

    mmi.mapped = vaddr;

	MEM_LOCK(&g_mem_mutex);

    ioctl(g_s32fd, IOC_MMB_USER_UNMAP, &mmi);

	if (mmi.mapped == NULL || mmi.size == 0)
	{
		MEM_UNLOCK(&g_mem_mutex);
		return MT_SUCCESS;
	}
	ret = munmap(mmi.mapped - PAGE_SIZE, (size_t)mmi.size + (2 * PAGE_SIZE));
#ifdef CONFIG_MT_SANITIZE_TAG
	mt_hwasan_tag_clean(mmi.mapped, mmi.size);
#endif

	MEM_UNLOCK(&g_mem_mutex);

	return ret;
}

mt_s32 mt_mpi_mmz_flush(void *vaddr, ulong offset, ulong size)
{
	struct cache_op_mmz_area coa = {0};
	int ret = 0;

#ifdef CONFIG_MT_SANITIZE_TAG
	vaddr = untagged_addr(vaddr);
#endif

    CHECK_MMZ_OPEN_STATE();

    if (vaddr == NULL)
    {
    	MEM_LOCK(&g_mem_mutex);
        ret = ioctl(g_s32fd, IOC_MMB_FLUSH_DCACHE, NULL);
		MEM_UNLOCK(&g_mem_mutex);
		return ret;
    }
    else
    {
    	coa.vbase = vaddr;
    	coa.offset = offset;
		coa.size = size;
		MEM_LOCK(&g_mem_mutex);
        ret = ioctl(g_s32fd, IOC_MMB_FLUSH_DCACHE, &coa);
		MEM_UNLOCK(&g_mem_mutex);
		return ret;
    }
}

mt_s32 mt_mpi_mmz_invalidate(void *vaddr, ulong offset, ulong size)
{
	struct cache_op_mmz_area coa = {0};
	int ret = 0;

#ifdef CONFIG_MT_SANITIZE_TAG
	vaddr = untagged_addr(vaddr);
#endif

    CHECK_MMZ_OPEN_STATE();

    if (vaddr == NULL)
    {
        MT_FATAL_MEM("can't invalidate all");
        return MT_FAILURE;
    }
    else
    {
    	coa.vbase = vaddr;
    	coa.offset = offset;
		coa.size = size;
		MEM_LOCK(&g_mem_mutex);
        ret = ioctl(g_s32fd, IOC_MMB_INV_DCACHE, &coa);
		MEM_UNLOCK(&g_mem_mutex);
		return ret;
    }
}

mt_s32 mt_mpi_mmz_get_start_size(const char *mmz_name, phys_addr_t *phys_start, ulong *size)
{
	int ret;
	struct mmz_start_size mmz_start_size;

	CHECK_MMZ_OPEN_STATE();

	if (mmz_name) {
		strncpy(mmz_start_size.mmz_name, mmz_name, sizeof(mmz_start_size.mmz_name));
	} else {
		strncpy(mmz_start_size.mmz_name, MMZ_ZONE_DDR, sizeof(mmz_start_size.mmz_name));
	}

	MEM_LOCK(&g_mem_mutex);
	ret = ioctl(g_s32fd, IOC_MMZ_GET_START_SIZE, &mmz_start_size);
	MEM_UNLOCK(&g_mem_mutex);

	if ((ret != MT_SUCCESS) || (mmz_start_size.nbytes == 0) || (mmz_start_size.phys_start == 0)) {
		*phys_start = 0;
		*size = 0;
		return MT_FAILURE;
	}
	*phys_start = mmz_start_size.phys_start;
	*size = mmz_start_size.nbytes;

	return MT_SUCCESS;
}
