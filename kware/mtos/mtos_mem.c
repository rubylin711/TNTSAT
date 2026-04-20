/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mt_type.h"

#include "mtos_mem.h"

static void *_align_malloc_alias(u32 size, u32 alignment)
{
    ulong *p_mem_ptr = NULL;
    ulong *p_tmp = NULL;
    ulong temp_len = 0;
    //u32 header_len = 0;
    //FILE *fp = NULL;

    temp_len = size + alignment;

    /*! Allocate the required size memory + alignment so we can realign the data if necessary */
    if ((p_tmp = (ulong *)malloc(temp_len)) != NULL) {
	/*! Align the tmp pointer */
	p_mem_ptr = (ulong *)(((ulong)p_tmp + alignment - 1) & (~(ulong)(alignment - 1)));

	/*!
    Special case where malloc have already satisfied the alignment
    We must add alignment to mem_ptr because we must store
    (mem_ptr - tmp) in *(mem_ptr-1)
    If we do not add alignment to mem_ptr then *(mem_ptr-1) points to a
    forbidden memory space
    */
	if (p_mem_ptr == p_tmp)
	    p_mem_ptr = (ulong *)((ulong)p_mem_ptr + alignment);

	/*!
      (mem_ptr - tmp) is stored in *(mem_ptr-1)
      so we are able to retrieve the real malloc block allocated
      and free it in mem_ptr-1
     */
	//header_len = *((p_mem_ptr - 1)) = (u32)(p_mem_ptr - p_tmp);
	*((p_mem_ptr - 1)) = ((ulong)p_mem_ptr -(ulong)p_tmp);

	return ((void *)p_mem_ptr);
    }
#if 0 /* drop cache*/
    sync();
    sync();
    sync();
    fp = fopen("/proc/sys/vm/drop_caches", "w+");
    if (fp == NULL)
	goto exit;
    fprintf(fp, "3");
    fclose(fp);
exit:
#endif
    return (NULL);
}

static void _align_free_alias(void *p_mem_ptr)
{
    ulong *p_ptr = NULL;
    ulong header_len = 0;

    if (p_mem_ptr == NULL)
	return;

    /*! Aligned pointer */
    p_ptr = (ulong *)p_mem_ptr;

    /*!
    (ptr - 1) holds the offset to the real allocated block
    we sub that offset os we free the real pointer
    */
    header_len = *(p_ptr - 1);
    //FIXME
    p_ptr = (ulong*)((ulong)p_ptr - header_len);

    free(p_ptr);

    return;
}

void mtos_mem_init(pmalloc m, pfree f)
{
    return;
}

void mtos_mem_user_debug(mem_user_dbg_info_t *p_dbg_info)
{
    return;
}

void mtos_mem_bound_check(void)
{
    return;
}

void mtos_mem_log_enable(MT_BOOL enable)
{
    return;
}

RET_CODE mtos_mem_leak_overflow_dbg_enable(mem_leak_overflow_dbg_cfg_t *p_cfg)
{
    return SUCCESS;
}

RET_CODE mtos_mem_leak_overflow_dbg_disable(void)
{
    return SUCCESS;
}

void mtos_malloc_actual_addr_get(ulong alloc_addr, u32 alloc_mode, ulong *p_act_addr)
{
    return;
}

#ifdef MTOS_MEM_DEBUG
void *mtos_malloc_dump(u32 size, char *p_file, char *p_func, int line)
{
    return NULL;
}

void mtos_free_dump(void *p_addr, char *p_file, char *p_func, int line)
{
    return;
}

void *mtos_calloc_dump(u32 n_elements, u32 elem_size, char *p_file, char *p_func, int line)
{
    return NULL;
}

void *mtos_realloc_dump(void *p_readdr, u32 size, char *p_file, char *p_func, int line)
{
    return NULL;
}

void *mtos_dma_malloc_dump(u32 size, char *p_file, char *p_func, int line)
{
    return NULL;
}

void mtos_dma_free_dump(void *p_addr, char *p_file, char *p_func, int line)
{
    return;
}

void *mtos_cache_align_malloc_dump(u32 size, char *p_file, char *p_func, int line)
{
    return NULL;
}

void mtos_cache_align_free_dump(void *p_ptr, char *p_file, char *p_func, int line)
{
    return;
}

void *mtos_align_malloc_dump(u32 size, u32 alignment, char *p_file, char *p_func, int line)
{
    return NULL;
}

void mtos_align_free_dump(void *p_mem_ptr, char *p_file, char *p_func, int line)
{
    return;
}

void *mtos_align_dma_malloc_dump(u32 size, u32 alignment, char *p_file, char *p_func, int line)
{
    return NULL;
}

void mtos_align_dma_free_dump(void *p_mem_ptr, char *p_file, char *p_func, int line)
{
    return;
}
#else
#if 0
void *mtos_malloc(u32 size)
{
    return malloc(size);
}

void mtos_free(void *p_addr)
{
    free(p_addr);
    return;
}

void *mtos_calloc(u32 n_elements, u32 elem_size)
{
    return calloc(n_elements, elem_size);
}

void *mtos_realloc(void *p_readdr, u32 size)
{
    return realloc(p_readdr, size);
}

#endif
void *mtos_malloc_alias(u32 size)
{
    return malloc(size);
}

void mtos_free_alias(void *p_addr)
{
    return free(p_addr);
}

void *mtos_calloc_alias(u32 n_elements, u32 elem_size)
{
    return calloc(n_elements, elem_size);
}

void *mtos_realloc_alias(void *p_readdr, u32 size)
{
    return realloc(p_readdr, size);
}

void *mtos_dma_malloc_alias(u32 size)
{
    return mtos_malloc(size);
}

void mtos_dma_free_alias(void *p_addr)
{
    return mtos_free(p_addr);
}

void *mtos_cache_align_malloc_alias(u32 size)
{
    return NULL;
}

void mtos_cache_align_free_alias(void *p_ptr)
{
    return;
}

void *mtos_align_malloc_alias(u32 size, u32 alignment)
{
    return _align_malloc_alias(size, alignment);
}

void mtos_align_free_alias(void *p_mem_ptr)
{
    _align_free_alias(p_mem_ptr);
    return;
}

void *mtos_align_dma_malloc_alias(u32 size, u32 alignment)
{
    return NULL;
}

void mtos_align_dma_free_alias(void *p_mem_ptr)
{
    return;
}

#endif

void dlmem_init(void *base, int size)
{
    return;
}

void *dl_malloc(u32 bytes)
{
    return NULL;
}

void dl_free(void *p_mem_ptr)
{
    return;
}

void *dl_calloc(u32 n_elements, u32 elem_size)
{
    return NULL;
}

void *dl_realloc(void *p_oldmem, u32 bytes)
{
    return NULL;
}
