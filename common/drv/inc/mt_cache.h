/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_CACHE_H__
#define __MT_CACHE_H__

#ifdef __KERNEL__
#include <linux/types.h>
#endif

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#define CACHE_LINE_SIZE 64UL
#else /* CONFIG_MIPS */
#define CACHE_LINE_SIZE 32UL
#endif

#define CACHE_ALIGN __attribute__ ((aligned (CACHE_LINE_SIZE)))

/*
  * clean: sync data from cache to ddr
  * invalid: drop cache
  * flush: do clean, then do invalid
  */
#ifdef __KERNEL__
extern void mt_user_dcache_clean(void *vaddr, ulong offset, ulong bytes);
extern void mt_user_dcache_invalid(void *vaddr, ulong offset, ulong bytes);
extern void mt_user_dcache_flush(void * vaddr, ulong offset, ulong bytes);
extern void mt_dcache_line_size_init(void);
extern void mt_dcache_clean(void *vaddr, ulong bytes);
extern void mt_dcache_invalid(void *vaddr, ulong bytes);
extern void mt_dcache_flush(void *vaddr, ulong bytes);
extern void mt_dcache_flush_all(void);
extern void *mt_remap_mmz_k(phys_addr_t paddr, ulong size, bool cached);
extern void mt_unmap_mmz_k(void *vaddr);
extern pgprot_t mt_mmz_noncached_pgprot(pgprot_t prot);
#endif

#endif /* __MT_CACHE_H__ */
