/*
*
* Copyright (c) 2006 Montage-tech Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*
*/

#ifndef __ASM_ARCH_MEDIA_MEM_H
#define __ASM_ARCH_MEDIA_MEM_H
#ifdef __cplusplus
extern "C" {
#endif

#include "mt_common.h"

#ifdef MMZ_V2_SUPPORT
#define MMB_SHARE_SUPPORT
#endif

#ifndef PRINTK_CA
#ifdef  CONFIG_SUPPORT_CA_RELEASE
#define PRINTK_CA(fmt, args...)
#else
#define PRINTK_CA(fmt, args...) do{\
        MT_PRINT("%s(%d): " fmt, __FILE__, __LINE__, ##args); \
} while (0)
#endif
#endif

struct mt_drv_mmz_security_attr_s;

/* just for inf */
struct mtl_media_memory_zone {
    char name[MTL_MMZ_NAME_LEN];
    unsigned long gfp;				/* ??? */
    phys_addr_t phys_start;
    unsigned long nbytes;
    struct list_head list;
  //  unsigned char *bitmap;
    struct device *dev;
    struct list_head mmb_list;
    unsigned int alloc_type;		/* ??? */
    unsigned long block_align;
    void (*destructor)(const void *);
};
typedef struct mtl_media_memory_zone mtl_mmz_t;

#define MTL_MMZ_FMT_S "PHYS(0x%016lx, 0x%016lx), GFP=%lu, nBYTES=%luKB,   NAME=\"%s\""
#define mtl_mmz_fmt_arg(p) (ulong)((p)->phys_start), (ulong)((p)->phys_start+(p)->nbytes), (p)->gfp, (p)->nbytes/SZ_1K, (p)->name

/* for inf & usr */
#define MTL_MMB_NAME_LEN 32
struct mtl_media_memory_block {
    char name[MTL_MMB_NAME_LEN];
    struct mtl_media_memory_zone *zone;
    struct list_head list;
    phys_addr_t phys_addr;
	void *dma_virt;
    void *kvirt;
    void *uvirt;
    unsigned long length;
    unsigned long flags;
    unsigned int order;
    int phy_ref;	/* access reference counter */
    int map_ref;	/* kernel map/unmap reference counter */
    int kphy_ref;	/* allloc/free reference counter */

    struct mt_drv_mmz_security_attr_s attr;	/* extended attributes for security etc. */
};
typedef struct mtl_media_memory_block mtl_mmb_t;

#define mtl_mmb_kvirt(p)    ({mtl_mmb_t *__mmb=(p); BUG_ON(__mmb==NULL); __mmb->kvirt;})
#define mtl_mmb_phys(p)     ({mtl_mmb_t *__mmb=(p); BUG_ON(__mmb==NULL); __mmb->phys_addr;})
#define mtl_mmb_length(p)   ({mtl_mmb_t *__mmb=(p); BUG_ON(__mmb==NULL); __mmb->length;})
#define mtl_mmb_name(p)     ({mtl_mmb_t *__mmb=(p); BUG_ON(__mmb==NULL); __mmb->name;})
#define mtl_mmb_zone(p)     ({mtl_mmb_t *__mmb=(p); BUG_ON(__mmb==NULL); __mmb->zone;})

#define MTL_MMB_MAP2KERN    				(1<<0)
#define MTL_MMB_MAP2KERN_CACHED 			(1<<1)
#define MTL_MMB_RELEASED    				(1<<2)
#define MTL_MMB_MAP2USER 					(1<<3)
#define MTL_MMB_MAP2USER_CACHED				(1<<4)
#define MTL_MMB_MAP2USER_MULTI				(1<<5)
#define MTL_MMB_MAP2USER_MULTI_INCORHERENT	(1<<6)

#define MTL_MMB_FMT_S "phys(0x%016lx, 0x%016lx), Kvirt=0x%016lx, Uvirt=0x%016lx, flags=0x%08lx, length=%luKB,	name=\"%s\""
#define mtl_mmb_fmt_arg(p) (ulong)((p)->phys_addr), (ulong)(mmz_grain_align((p)->phys_addr+(p)->length)), (ulong)((p)->kvirt), (ulong)((p)->uvirt), (p)->flags, (p)->length/SZ_1K, (p)->name



/********** API_0 for inf *********/
extern mtl_mmz_t *mtl_mmz_create(const char *name, unsigned long gfp, unsigned long phys_start,
            unsigned long nbytes);
extern mtl_mmz_t *mtl_mmz_create_v2(const char *name, unsigned long gfp, unsigned long phys_start,
            unsigned long nbytes,  unsigned int alloc_type, unsigned long block_align);
extern int mtl_mmz_destroy(mtl_mmz_t *zone);
extern int mtl_mmz_register(mtl_mmz_t *zone);
extern int mtl_mmz_unregister(mtl_mmz_t *zone);
extern mtl_mmb_t *mtl_mmb_getby_phys(phys_addr_t addr);
extern mtl_mmb_t *mtl_mmb_getby_phys_2(phys_addr_t addr, phys_addr_t *Outoffset);
extern mtl_mmb_t *mtl_mmb_getby_kvirt(void *virt);
extern void* mtl_mmb_map2kern(mtl_mmb_t *mmb);
extern void* mtl_mmb_map2kern_cached(mtl_mmb_t *mmb);
extern int mtl_mmb_unmap(mtl_mmb_t *mmb);
extern int mtl_mmb_map2user(void *virt, unsigned long phys_addr, int cache);


/********** API_1 for inf & usr *********/

extern void dump_mmz_mem(void);
extern mtl_mmb_t *mtl_mmb_alloc(const char *name, unsigned long size, unsigned long align,
        unsigned long gfp, const char *mmz_name,
        struct mt_drv_mmz_security_attr_s *attr);
extern int mtl_mmb_free(mtl_mmb_t *mmb);
extern mtl_mmb_t *mtl_mmb_alloc_v2(const char *name, unsigned long size, unsigned long align,
        unsigned long gfp, const char *mmz_name, unsigned int order);

extern int mtl_mmb_get(mtl_mmb_t *mmb);
extern int mtl_mmb_put(mtl_mmb_t *mmb);
#ifdef MMB_SHARE_SUPPORT
extern int mtl_mmb_force_put(mtl_mmb_t *mmb);
#endif
extern int get_mmz_info_phys_start(void);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
extern int mmz_read_proc(char *page, char **start, off_t off,
                            int count, int *eof, void *data);
#else
extern int mmz_read_proc(struct seq_file *m, void *v);
#endif

extern int mmz_write_proc(struct file *file, const char __user *buffer,
                                   unsigned long count, void *data);

int mt_drv_mmz_init(void);
void mt_drv_mmz_exit(void);

#ifdef __cplusplus
}
#endif

#endif
