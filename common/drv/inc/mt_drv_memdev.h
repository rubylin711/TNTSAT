
#ifndef __MT_DRV_MEMDEV_H__
#define __MT_DRV_MEMDEV_H__

/******************************* Include Files *******************************/
/* add include here */
#include "mt_type.h"
#include "mt_module.h"
#include "mt_debug.h"
#include "mt_drv_struct.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

#define UMAP_DEVNAME_MEMDEV           UMAP_NAME MT_MOD_MEM

#define MT_FATAL_MEMDEV(fmt...)   MT_FATAL_PRINT(MT_ID_MEMDEV, fmt)
#define MT_ERR_MEMDEV(fmt...)     MT_ERR_PRINT(MT_ID_MEMDEV, fmt)
#define MT_WARN_MEMDEV(fmt...)    MT_WARN_PRINT(MT_ID_MEMDEV, fmt)
#define MT_INFO_MEMDEV(fmt...)    MT_INFO_PRINT(MT_ID_MEMDEV, fmt)

#define MT_MEM_FLUSH_DCACHE		_IO('c', 40)
#define MT_MEM_INV_DCACHE		_IO('c', 41)
#define MT_MEM_GET_PAGEINFO		_IOWR('p', 42, struct get_mem_pageinfo)
#define MT_MEM_WALK_WATCH		_IOW('w', 43, struct walk_watch)
#define MT_MEM_WALK_GET_PHYS_ADDR	_IOW('g', 44, struct walk_get_phys_addr)


/*************************** Structure Definition ****************************/
#ifdef __KERNEL__
struct memdev_mmap_info_t {
	struct list_head list;
	void *virt;
	unsigned long phys_addr;
#ifdef CONFIG_STACKTRACE
	unsigned long entries[64];
	unsigned int nr_entries;
#endif
};

#ifdef CONFIG_COMPAT

struct compat_cache_op_mem_area {
	compat_uptr_t virtaddr;
	compat_ulong_t size;
};

struct compat_get_mem_pageinfo {
	mt_u32 page_block_order;
	mt_u32 pages_per_block;
};

struct compat_walk_watch {
	compat_ulong_t user_vaddr;
	u64 right_value;
	u32 size;
};

struct compat_walk_get_phys_addr {
	compat_ulong_t user_vaddr;
	phys_addr_t phys_addr;
};


#define COMPAT_MT_MEM_FLUSH_DCACHE		_IO('c', 40)
#define COMPAT_MT_MEM_INV_DCACHE		_IO('c', 41)
#define COMPAT_MT_MEM_GET_PAGEINFO		_IOWR('p', 42, struct compat_get_mem_pageinfo)
#define COMPAT_MT_MEM_WALK_WATCH		_IOW('w', 43, struct compat_walk_watch)
#define COMPAT_MT_MEM_WALK_GET_PHYS_ADDR	_IOW('g', 44, struct compat_walk_get_phys_addr)

#endif
#endif

struct cache_op_mem_area {
	void *virtaddr;
	ulong size;
};

struct get_mem_pageinfo {
	mt_u32 page_block_order;
	mt_u32 pages_per_block;
};

struct walk_watch {
	ulong user_vaddr;
	u64 right_value;
	u32 size;
};

struct walk_get_phys_addr {
	ulong user_vaddr;
	phys_addr_t phys_addr;
};

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

mt_s32 memdev_drv_modinit(mt_void);
mt_s32 memdev_drv_modexit(mt_void);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_DRV_MEMDEV_H__ */
