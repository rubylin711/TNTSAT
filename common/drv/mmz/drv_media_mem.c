/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/* media-mem.c
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
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/stacktrace.h>
//#include <mach/hardware.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/list.h>
#include <asm/cacheflush.h>
#include <linux/time.h>
#include <asm/setup.h>

#include <linux/dma-map-ops.h>

#include <linux/dma-mapping.h>
#include <asm/tlbflush.h>
#include <asm/pgtable.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>

#include "mt_drv_mmz.h"
#include "drv_media_mem.h"
#include "mt_kernel_adapt.h"
#include "mt_debug.h"
#include "mt_drv_memdev.h"
#include "mt_cache.h"
#include <linux/cma.h>
#include <linux/mt_mmz.h>

#undef DEBUG_MMZ
//#define DEBUG_MMZ

#ifdef CONFIG_STACKTRACE
#undef PRINT_MMAP_DEVICE
#endif

#define DEFAULT_ALLOC 			0
#define SLAB_ALLOC 				1
#define EQ_BLOCK_ALLOC 			2

#define LOW_TO_HIGH 			0
#define HIGH_TO_LOW 			1

#define MMZ_DBG_LEVEL 			0x0
#define mmz_trace(level, s, params...) do { if (level & MMZ_DBG_LEVEL) \
											PRINTK_CA(KERN_INFO "[%s, %d]: " s "\n", \
													__FUNCTION__, __LINE__, params);\
										} while (0)
#define mmz_trace_func() 		mmz_trace(0x02, "%s", __FILE__)

#define MMZ_GRAIN 				PAGE_SIZE
#define MMZ_ALIGN_ORDER64K 		4
#define mmz_bitmap_size(p) 		(mmz_align2(mmz_length2grain((p)->nbytes), 8) / 8)

#define mmz_get_bit(p, n) 		(((p)->bitmap[(n) / 8] >> ((n) & 0x7)) & 0x1)
#define mmz_set_bit(p, n) 		(p)->bitmap[(n) / 8] |= 1 << ((n) & 0x7)
#define mmz_clr_bit(p, n) 		(p)->bitmap[(n) / 8] &= ~(1 << ((n) & 0x7))

#define mmz_pos2phy_addr(p, n) 	((p)->phys_start + (n) * MMZ_GRAIN)
#define mmz_phy_addr2pos(p, a) 	(((a) - (p)->phys_start) / MMZ_GRAIN)

#define mmz_align2low(x, g) 	((x) & (~(((g) - 1))))
#define mmz_align2(x, g) 		(((x) + ((g) - 1)) & (~(((g) - 1))))
#define mmz_grain_align(x) 		mmz_align2(x, MMZ_GRAIN)
#define mmz_length2grain(len) 	(mmz_grain_align(len) / MMZ_GRAIN)

extern int dont_steal_av;
#define begin_list_for_each_mmz(p, gfp, mmz_name) \
	list_for_each_entry(p, &mmz_list, list) {\
		if (gfp == 0 ? 0 : (p)->gfp != (gfp)) \
			continue;\
		if (mmz_name != NULL) { \
			if ((*mmz_name != '\0') && strncmp(mmz_name, p->name, MTL_MMZ_NAME_LEN)) \
				continue;\
			if (dont_steal_av && (*mmz_name == '\0') && \
				(strncmp(MMZ_ZONE_DDR, p->name, MTL_MMZ_NAME_LEN))) \
				continue; \
		} \
		if ((mmz_name == NULL) && (anony == 1)) { \
			if (strncmp("anonymous", p->name, MTL_MMZ_NAME_LEN)) \
				continue;\
		} \
		if (mmz_name == NULL) { \
			if (dont_steal_av && \
				(strncmp(MMZ_ZONE_DDR, p->name, MTL_MMZ_NAME_LEN))) \
				continue; \
		} \
		mmz_trace(1, MTL_MMZ_FMT_S, mtl_mmz_fmt_arg(p));

#define end_list_for_each_mmz() }

static char line[COMMAND_LINE_SIZE];

static int mmz_total_size = 0;

static int zone_number = 0;
static int block_number = 0;

/* all mmz zone list */
static LIST_HEAD(mmz_list);

#ifdef PRINT_MMAP_DEVICE
static LIST_HEAD(memdev_mmap_info_list);
#endif
static MT_DECLARE_MUTEX(mmz_lock);

/* anonymous mmz zone */
static int anony = 0;

module_param(anony, int, S_IRUGO);

static int _mmb_free(mtl_mmb_t *mmb);

#ifdef DEBUG_MMZ
static void test_mmz_notify(int op,
							const char *zone_name,
							const char *name,
							unsigned long phys_addr, unsigned long length,
							mmz_security_attr_s *attr,
							void *priv);
#endif

/* call notifiers if needed */
static void mmz_run_callback(int op,	/* MT_MMZ_NOTIFY_ALLOC or MT_MMZ_NOTIFY_FREE */
							 const char *zone_name,
							 mtl_mmb_t *mmb)
{
	void *priv = NULL;
	mt_drv_mmz_notify_t notify;

	notify = mt_drv_mmz_get_notify(&priv);

	if (notify != NULL)
	{
#ifdef DEBUG_MMZ
		PRINTK_CA(KERN_DEBUG "%s: before call notifier %p.\r\n", __FUNCTION__,notify);
		PRINTK_CA(KERN_DEBUG "%s: op %d name \'%s\' addr 0x%lx len 0x%lx flags 0x%x type %d subtype %d.\r\n",
					__FUNCTION__,
					op,
					mmb->name,
					mmb->phys_addr,
					mmb->length,
					mmb->attr.flags,
					mmb->attr.type,
					mmb->attr.subtype);
#endif

		notify(op, zone_name,
			mmb->name,
			mmb->phys_addr,
			mmb->length,
			&mmb->attr,
			priv);

#ifdef DEBUG_MMZ
		PRINTK_CA(KERN_DEBUG "%s: after call notifier %p.\r\n", __FUNCTION__,notify);
#endif
	}
}

/* new one mmz zone struct */
mtl_mmz_t *mtl_mmz_create(const char *name, unsigned long gfp, unsigned long phys_start, unsigned long nbytes)
{
    mtl_mmz_t *p = NULL;

    mmz_trace_func();

    if (name == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: 'name' can not be zero!", __FUNCTION__);
        return NULL;
    }

    p = kzalloc(sizeof(mtl_mmz_t) + 1, GFP_KERNEL);
    if (p == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: kzalloc failed!\r\n", __FUNCTION__);
        return NULL;
    }

    memset(p, 0, sizeof(mtl_mmz_t) + 1);
    strlcpy(p->name, name, MTL_MMZ_NAME_LEN);
    p->gfp = gfp;
    p->phys_start = phys_start;
    p->nbytes = nbytes;

    INIT_LIST_HEAD(&p->list);
    INIT_LIST_HEAD(&p->mmb_list);

    p->destructor = kfree;

    return p;
}

/* destroy one mmz zone struct */
int mtl_mmz_destroy(mtl_mmz_t *zone)
{
    if (zone == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmz zone is null!\r\n", __FUNCTION__);
        return (-1);
    }

    if (zone->destructor != NULL)
    {
        zone->destructor(zone);
    }

    return 0;
}

/* register one mmz zone into the mmz zone list */
int mtl_mmz_register(mtl_mmz_t *zone)
{
    mmz_trace(1, MTL_MMZ_FMT_S, mtl_mmz_fmt_arg(zone));

    if (zone == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmz zone is null!\r\n", __FUNCTION__);
        return (-1);
    }

    down(&mmz_lock);

    INIT_LIST_HEAD(&zone->mmb_list);

    list_add(&zone->list, &mmz_list);

    up(&mmz_lock);

    return 0;
}

/* unregister one mmz zone from the mmz zone list, and destroy it(?) */
int mtl_mmz_unregister(mtl_mmz_t *zone)
{
    int losts = 0;
    mtl_mmb_t *p;

    if (zone == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmz zone is null!\r\n", __FUNCTION__);
        return -1;
    }

    mmz_trace_func();

    down(&mmz_lock);
    list_for_each_entry(p, &zone->mmb_list, list)
    {
        PRINTK_CA(KERN_WARNING "          MB Lost: " MTL_MMB_FMT_S "\n", mtl_mmb_fmt_arg(p));
        losts ++;
    }

    if (losts != 0)
    {
        PRINTK_CA(KERN_ERR "%d mmbs not free, mmz<%s> can not be deregistered!\n", losts, zone->name);
        up(&mmz_lock);
        return (-1);
    }

    list_del(&zone->list);

	//FIXME:
	//why not? memory leak!
    //mtl_mmz_destroy(zone);
    printk(KERN_ERR "%s: mmz zone %s might memory leak!\r\n", __FUNCTION__, zone->name);

    up(&mmz_lock);

    return 0;
}

/* sort and insert one mmz block(mmb) into the mmb list */
static int _do_mmb_alloc(mtl_mmb_t *mmb)
{
    mtl_mmb_t *p = NULL;

    mmz_trace_func();

    /* add mmb sorted */
    list_for_each_entry(p, &mmb->zone->mmb_list, list)
    {
        if (mmb->phys_addr < p->phys_addr)
        {
            break;
        }

        if (mmb->phys_addr == p->phys_addr)
        {
            PRINTK_CA(KERN_ERR "ERROR: media-mem allocator bad in %s! (%s, %d)",
                   mmb->zone->name, __FUNCTION__, __LINE__);
        }
    }
    list_add(&mmb->list, p->list.prev);

    mmz_trace(1, MTL_MMB_FMT_S, mtl_mmb_fmt_arg(mmb));

    return 0;
}

/* allocate mmz pages from dma, and mmz block(for manage mmz pages) */
static mtl_mmb_t *__mmb_alloc(const char *name, ulong size, unsigned long align,
							  unsigned long gfp, const char *mmz_name, mtl_mmz_t *_user_mmz,
							  struct mt_drv_mmz_security_attr_s *attr)
{
	mtl_mmz_t *mmz;
	mtl_mmb_t *mmb = NULL;
	unsigned int align_order;
	size_t count;
	struct page *page = NULL;
	unsigned long fixed_start = 0;
	//unsigned long fixed_len = -1;
	mtl_mmz_t *fixed_mmz = NULL;
	void *dma_virt_addr = NULL;
	dma_addr_t dma_handle = 0;
	void __iomem *ca;

	mmz_trace_func();

	if (size == 0) {
		printk(KERN_ERR "%s: invalid size 0x%lx!\r\n", __FUNCTION__, size);
		return NULL;
	}

	if (align == 0 || align > 0x400000UL) {
		//printk(KERN_WARNING "%s: invalid align 0x%lx!\r\n", __FUNCTION__, align);
		align = MMZ_GRAIN;
	}

	size = mmz_grain_align(size);
	count = (size >> PAGE_SHIFT);		/* page count */
	align_order = get_order(align);		/* page order */
	/* size >= 64KB, align 64K at least */
	if ((align_order < MMZ_ALIGN_ORDER64K) && (get_order(size) >= MMZ_ALIGN_ORDER64K)) {
		align_order = MMZ_ALIGN_ORDER64K;
	}

	//mmz_trace(1, "mmz_name=%s, size=%luKB, align=%lu",mmz_name,  size / SZ_1K, align);

	begin_list_for_each_mmz(mmz, gfp, mmz_name) {

		/* user specified mmz zone pointer */
		if ((_user_mmz != NULL) && (_user_mmz != mmz)) {
			continue;
		}
		//printk("====mmz===0x%x==\n", mmz);
		//note: dma_alloc_from_contiguous will reduce align_order, so it will not waste so much
		if (mmz->dev->cma_area && !mmz->dev->dma_mem) {
			page = dma_alloc_from_contiguous(mmz->dev, count, align_order, GFP_KERNEL & __GFP_NOWARN);
		} else if (mmz->dev->dma_mem && !mmz->dev->cma_area) {
			if (mt_dma_alloc_from_dev_coherent_align(mmz->dev, (ssize_t)size, &dma_handle, &dma_virt_addr, align_order)) {
				if (dma_handle) {
					page = phys_to_page(dma_handle);
				}
			}
		}
		if (page == NULL) {
			continue;
		}

		// printk("==222==mmz===0x%x==\n", mmz);
		fixed_mmz = mmz;
		fixed_start = page_to_phys(page);
		break;

	} end_list_for_each_mmz();
	//printk("==333==mmz===0x%x=fixed_mmz=0x%x=\n", mmz, fixed_mmz);
	if (fixed_mmz == NULL) {
		printk(" Alloc zone '%s', mmb '%s' failed, size %lu bytes.\n", mmz_name, name, size);
		dump_stack();
		return NULL;
	}

	mmb = kzalloc(sizeof(mtl_mmb_t), GFP_KERNEL);
	if (mmb == NULL) {
		printk(KERN_ERR "%s: kzalloc failed!\r\n", __FUNCTION__);
		return NULL;
	}

	mmb->zone = fixed_mmz;
	mmb->phys_addr = fixed_start;
	mmb->length = size;
	mmb->dma_virt = dma_virt_addr;
	if (name != NULL) {
		strlcpy(mmb->name, name, MTL_MMB_NAME_LEN);
	} else {
		strncpy(mmb->name, "<null>", sizeof(mmb->name)-1);
	}

	if (attr != NULL) {
		memcpy(&mmb->attr, attr, sizeof(mmb->attr));
	}

	if (_do_mmb_alloc(mmb) != 0) {
		printk(KERN_ERR "%s: _do_mmb_alloc failed!\r\n", __FUNCTION__);
		kfree(mmb);
		mmb = NULL;
	}

	mmz_run_callback(MT_MMZ_NOTIFY_ALLOC,
					mmz_name,
					mmb);

	/*
	* remove any dirty cache lines on the kernel alias
	* see __dma_clear_buffer in __alloc_from_contiguous
	* and arch_dma_prep_coherent in dma_direct_alloc
	*/
	if (mmz->dev->cma_area && !mmz->dev->dma_mem) {
		mt_dcache_flush(page_address(page), mmb->length);
	} else if (mmz->dev->dma_mem && !mmz->dev->cma_area) {
		/* dma pool is no map */
		ca = ioremap_cache(page_to_phys(page), mmb->length);
		mt_dcache_flush(ca, mmb->length);
		iounmap(ca);
	}

	return mmb;
}

/* allocate one mmz block(mmb) */
mtl_mmb_t *mtl_mmb_alloc(const char *name, ulong size, unsigned long align,
                         unsigned long gfp, const char *mmz_name,
                         struct mt_drv_mmz_security_attr_s *attr)
{
    mtl_mmb_t *mmb = NULL;
	int try_count = 0;

try_again:

    down(&mmz_lock);

    mmb = __mmb_alloc(name, size, align, gfp, mmz_name, NULL, attr);
    if (mmb != NULL)
    {
		mmb->kphy_ref ++;
	}

    up(&mmz_lock);

	if (mmb == NULL) {
		if (try_count < 2) {
			msleep_interruptible(50 * (try_count + 1));
			try_count++;
			pr_err("mtl_mmb_alloc try again\n");
			goto try_again;
		}
	}

    return mmb;
}

/* map mmz physical address to kernel virtual address with cachable or without cachable */
static void *_mmb_map2kern(mtl_mmb_t *mmb, int cached)
{
	/* already mapped */
    if ((mmb->flags & MTL_MMB_MAP2KERN) != 0)
    {
    	/* check cached flag */
        if ((cached * MTL_MMB_MAP2KERN_CACHED) != (mmb->flags & MTL_MMB_MAP2KERN_CACHED))
        {
            PRINTK_CA(KERN_ERR "mmb<%s> already kernel-mapped %s, can not be re-mapped as %s.",
                   mmb->name,
                   (mmb->flags & MTL_MMB_MAP2KERN_CACHED) ? "cached" : "non-cached",
                   (cached) ? "cached" : "non-cached" );
            return NULL;
        }

        mmb->map_ref ++;
        return mmb->kvirt;
    }

    if (cached != 0) {
	    mmb->flags |= MTL_MMB_MAP2KERN_CACHED;
    } else {
	    mmb->flags &= ~MTL_MMB_MAP2KERN_CACHED;
    }

    /* FIXME: invalid all the cache here? */
	mmb->kvirt = mt_remap_mmz_k(mmb->phys_addr, mmb->length, cached);

    if (mmb->kvirt != NULL)
    {
        mmb->flags |= MTL_MMB_MAP2KERN;
        mmb->map_ref ++;
    }

    return mmb->kvirt;
}

/* map mmz physical address to kernel virtual address without cachable */
void *mtl_mmb_map2kern(mtl_mmb_t *mmb)
{
    void *p;

    if (mmb == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmb is null!\r\n", __FUNCTION__);
        return NULL;
    }

    down(&mmz_lock);
    p = _mmb_map2kern(mmb, 0);
    up(&mmz_lock);

    return p;
}

/* map mmz physical address to kernel virtual address with cachable */
void *mtl_mmb_map2kern_cached(mtl_mmb_t *mmb)
{
    void *p;

    if (mmb == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmb is null!\r\n", __FUNCTION__);
        return NULL;
    }

    down(&mmz_lock);
    p = _mmb_map2kern(mmb, 1);
    up(&mmz_lock);

    return p;
}

/* user map, then call this function to update user mapped address */
int mtl_mmb_map2user(void *virt, unsigned long phys_addr, int cache)
{
	mtl_mmz_t *p = NULL;
	mtl_mmb_t *mmb = NULL;
	int ret = -EINVAL;

	if (virt == NULL || phys_addr == 0)
	{
		PRINTK_CA(KERN_ERR "%s: virt(%p) is null or phys_addr(%lx) is zero!\r\n", __FUNCTION__, virt, phys_addr);
		WARN_ON(1);
		return ret;
	}

	//Fix Bug 130319: multi thread issue!
	down(&mmz_lock);

	/* note: map register will can not found mmb */
	list_for_each_entry(p, &mmz_list, list) {
		list_for_each_entry(mmb, &p->mmb_list, list) {
			if (mmb->phys_addr == phys_addr) {
				if (mmb->uvirt != NULL) {
					mmb->flags |= MTL_MMB_MAP2USER_MULTI;
					if (((mmb->flags & MTL_MMB_MAP2USER_CACHED) != 0) && !cache) {
						mmb->flags |= MTL_MMB_MAP2USER_MULTI_INCORHERENT;
					} else if (!(mmb->flags & MTL_MMB_MAP2USER_CACHED) && cache) {
						mmb->flags |= MTL_MMB_MAP2USER_MULTI_INCORHERENT;
					}
				}
				/* FIXME: how about mult user processes mapping? */
				mmb->uvirt = virt;
				if (cache) {
					mmb->flags |= MTL_MMB_MAP2USER_CACHED;
				} else {
					mmb->flags &= ~MTL_MMB_MAP2USER_CACHED;
				}
				mmb->flags |= MTL_MMB_MAP2USER;
				ret = 0;
				break;
			}
		}
	}

#ifdef PRINT_MMAP_DEVICE
	if (ret != 0) {		/* save the buffer which is not alloc via mmz and mmap device register */
		struct memdev_mmap_info_t *memdev_mmap_info = kzalloc(sizeof(*memdev_mmap_info), GFP_KERNEL);

		if (memdev_mmap_info) {
			memdev_mmap_info->virt = virt;
			memdev_mmap_info->phys_addr = phys_addr;
			list_add(&memdev_mmap_info->list, &memdev_mmap_info_list);
			memdev_mmap_info->nr_entries = stack_trace_save(memdev_mmap_info->entries, ARRAY_SIZE(memdev_mmap_info->entries), 0);
		}
	}
#endif

	up(&mmz_lock);
	return ret;
}

/* unmap kernel virtual address */
int mtl_mmb_unmap(mtl_mmb_t *mmb)
{
    int ref;
    struct page* page = NULL;

    if (mmb == NULL || mmb->phys_addr == 0 || mmb->length == 0)
    {
        PRINTK_CA(KERN_ERR "%s: mmb is null or phys_addr/length is/are zero!\r\n", __FUNCTION__);
        return (-1);
    }

    page = phys_to_page(mmb->phys_addr);

    down(&mmz_lock);

    if ((mmb->flags & MTL_MMB_MAP2KERN) != 0)
    {
        ref = --mmb->map_ref;
        if (mmb->map_ref != 0)
        {
            up(&mmz_lock);
            return ref;
        }

        if (mmb->kvirt != NULL)
        {
			mt_unmap_mmz_k(mmb->kvirt);
        }
    }

    mmb->kvirt  = NULL;
    mmb->flags &= ~MTL_MMB_MAP2KERN;
    mmb->flags &= ~MTL_MMB_MAP2KERN_CACHED;

    if (((mmb->flags & MTL_MMB_RELEASED) != 0) && (mmb->phy_ref == 0))
    {
        _mmb_free(mmb);
    }
    up(&mmz_lock);

    return 0;
}

/* increase mmb physical reference counter */
int mtl_mmb_get(mtl_mmb_t *mmb)
{
    int ref;

    if (mmb == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmb is null!\r\n", __FUNCTION__);
        return (-1);
    }

    down(&mmz_lock);

    if ((mmb->flags & MTL_MMB_RELEASED) != 0)
    {
        PRINTK_CA(KERN_WARNING "mtl_mmb_get: amazing, mmb<%s> is released!\n", mmb->name);
    }

    ref = ++mmb->phy_ref;

    up(&mmz_lock);

    return ref;
}

/* free mmz block(mmb) */
static int _mmb_free(mtl_mmb_t *mmb)
{
    size_t count = (mmb->length >> PAGE_SHIFT); /* page count */
    struct page *page = phys_to_page(mmb->phys_addr);

    mtl_mmz_t *mmz = mmb->zone;

    if (mmb->kphy_ref == 0 || mmb->phys_addr == 0 || mmb->length == 0)
    {
        return 0;
    }

    mmb->kphy_ref --;

	BUG_ON(mmb->kphy_ref != 0);

	if (mmz->dev->cma_area) {
		dma_release_from_contiguous(mmz->dev, page, count);
	} else if (mmz->dev->dma_mem) {
		mt_dma_release_from_dev_coherent(mmz->dev, page, count);
	}

    list_del(&mmb->list);

	mmz_run_callback(MT_MMZ_NOTIFY_FREE,
					NULL,	/*unknown*/
					mmb);

    kfree(mmb);

    return 0;
}

/* decrease mmb physical reference counter */
int mtl_mmb_put(mtl_mmb_t *mmb)
{
    int ref;

    if (mmb == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmb is null!\r\n", __FUNCTION__);
        return (-1);
    }

    down(&mmz_lock);

    if ((mmb->flags & MTL_MMB_RELEASED) != 0)
    {
        PRINTK_CA(KERN_WARNING "mtl_mmb_put: amazing, mmb<%s> is released!\n", mmb->name);
    }

    ref = --mmb->phy_ref;

    if (((mmb->flags & MTL_MMB_RELEASED) != 0) && (mmb->phy_ref == 0) && (mmb->map_ref == 0))
    {
        _mmb_free(mmb);
    }

    up(&mmz_lock);

    return ref;
}

/* free mmz block(mmb) */
int mtl_mmb_free(mtl_mmb_t *mmb)
{
    mmz_trace_func();

    if (mmb == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: mmb is null!\r\n", __FUNCTION__);
        return (-1);
    }

    mmz_trace(1, MTL_MMB_FMT_S, mtl_mmb_fmt_arg(mmb));

    down(&mmz_lock);

    if ((mmb->flags & MTL_MMB_RELEASED) != 0)
    {
        PRINTK_CA(KERN_WARNING "mtl_mmb_free: amazing, mmb<%s> is released before, but still used!\n", mmb->name);

        up(&mmz_lock);

        return 0;
    }

    if (mmb->phy_ref > 0)
    {
        PRINTK_CA(KERN_WARNING "mtl_mmb_free: free mmb<%s> delayed for which ref-count is %d!\n",
               mmb->name, mmb->map_ref);
        mmb->flags |= MTL_MMB_RELEASED;
        up(&mmz_lock);

        return 0;
    }

    if ((mmb->flags & MTL_MMB_MAP2KERN) != 0)
    {
        PRINTK_CA(KERN_WARNING "mtl_mmb_free: free mmb<%s> delayed for which is kernel-mapped to 0x%p with map_ref %d!\n",
               mmb->name, mmb->kvirt, mmb->map_ref);
        mmb->flags |= MTL_MMB_RELEASED;
        up(&mmz_lock);

        return 0;
    }

    _mmb_free(mmb);

    up(&mmz_lock);

    return 0;
}

#define MACH_MMB(p, val, member) do {\
        mtl_mmz_t *__mach_mmb_zone__; \
        (p) = NULL; \
        list_for_each_entry(__mach_mmb_zone__, &mmz_list, list) { \
            mtl_mmb_t *__mach_mmb__; \
            list_for_each_entry(__mach_mmb__, &__mach_mmb_zone__->mmb_list, list) { \
                if (__mach_mmb__->member == (val)){ \
                    (p) = __mach_mmb__; \
                    break; \
                } \
            } \
            if (p) break;\
        } \
    } while (0)

/* get mmb by physical address */
mtl_mmb_t *mtl_mmb_getby_phys(phys_addr_t addr)
{
    mtl_mmb_t *p = NULL;

	if (addr == 0)
	{
        PRINTK_CA(KERN_ERR "%s: addr is zero!\r\n", __FUNCTION__);
		return NULL;
	}

    down(&mmz_lock);
    MACH_MMB(p, addr, phys_addr);
    up(&mmz_lock);

    return p;
}

/* get mmb by kernel virtual address */
mtl_mmb_t *mtl_mmb_getby_kvirt(void *virt)
{
    mtl_mmb_t *p = NULL;

    if (virt == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: virt is null!\r\n", __FUNCTION__);
        return NULL;
    }

    down(&mmz_lock);
    MACH_MMB(p, virt, kvirt);
    up(&mmz_lock);

    return p;
}

#define MACH_MMB_2(p, val, Outoffset) do {\
        mtl_mmz_t *__mach_mmb_zone__; \
        (p) = NULL; \
        list_for_each_entry(__mach_mmb_zone__, &mmz_list, list) { \
            mtl_mmb_t *__mach_mmb__; \
            list_for_each_entry(__mach_mmb__, &__mach_mmb_zone__->mmb_list, list) { \
                if ((__mach_mmb__->phys_addr <= (val)) && ((__mach_mmb__->length + __mach_mmb__->phys_addr) > (val))){ \
                    (p) = __mach_mmb__; \
                    Outoffset = val - __mach_mmb__->phys_addr; \
                    break; \
                } \
            } \
            if (p) break;\
        } \
    } while (0)

/* get mmb by physical address within [phys_addr, phys_addr+length) */
mtl_mmb_t *mtl_mmb_getby_phys_2(phys_addr_t addr, phys_addr_t *Outoffset)
{
    mtl_mmb_t *p = NULL;

    if (addr == 0 || Outoffset == NULL)
    {
        PRINTK_CA(KERN_ERR "%s: addr is zero, or Outoffset is null!\r\n", __FUNCTION__);
        return NULL;
    }

    down(&mmz_lock);
    MACH_MMB_2(p, addr, *Outoffset);
    up(&mmz_lock);
    return p;
}

/* get mmz zone struct pointer by mmz zone name */
mtl_mmz_t *mtl_mmz_find(unsigned long gfp, const char *mmz_name)
{
    mtl_mmz_t *p = NULL;

    down(&mmz_lock);
    begin_list_for_each_mmz(p, gfp, mmz_name) {
	    up(&mmz_lock);
	    return p;
    } end_list_for_each_mmz();
    up(&mmz_lock);

    return NULL;
}

/*
 * name,gfp,phys_start,nbytes,alloc_type,...
 *  e.g.
 *    "mmz=av,0,0,88M" => "av,0,0,88M"
 */
static int media_mem_parse_cmdline(char *s)
{
    mtl_mmz_t *zone = NULL;
    char *line;
    struct mmz_zone * mmz_zone;

    while ((line = strsep(&s, ":")) != NULL)
    {
        int i;
        char *argv[6];

        /*
         * FIXME: We got 4 args in "line", formated "argv[0],argv[1],argv[2],argv[3],argv[4]".
         * eg: "<mmz_name>,<gfp>,<phys_start_addr>,<size>,<alloc_type>"
         * For more convenient, "hard code" are used such as "arg[0]", i.e.
         */
        for (i = 0; (argv[i] = strsep(&line, ",")) != NULL;)
        {
            if (++i == ARRAY_SIZE(argv))
            {
                break;
            }
        }
		mmz_zone = mont_get_mmz_zone(argv[0]);

		if (mmz_zone == NULL)
		{
			PRINTK_CA(KERN_ERR"can't get cma zone info:%s\n",argv[0]);
			continue;
		}
		//printk("===cmdline===i=%d===start=0x%x====\n",i, mmz_zone->phys_start);
        if (i == 4)
        {
            zone = mtl_mmz_create("null", 0, 0, 0);
			//printk("===zone=0x%x=======\n",zone);
		    if(NULL == zone)
		    {
		    	continue;
		    }
            strlcpy(zone->name, argv[0], MTL_MMZ_NAME_LEN);
            //zone->gfp = _strtoul_ex(argv[1], NULL, 0);
            //zone->phys_start = _strtoul_ex(argv[2], NULL, 0);
            //zone->nbytes = _strtoul_ex(argv[3], NULL, 0);
			zone->gfp = mmz_zone->gfp;
			zone->phys_start = mmz_zone->phys_start;
			zone->nbytes = mmz_zone->nbytes;
			zone->dev = &mmz_zone->dev;
	    }
        else
        {
            PRINTK_CA(KERN_ERR "MMZ: your parameter num is not correct!\n");
            continue;
        }

        if (mtl_mmz_register(zone) != 0)
        {
            PRINTK_CA(KERN_WARNING "Add MMZ failed: " MTL_MMZ_FMT_S "\n", mtl_mmz_fmt_arg(zone));
            mtl_mmz_destroy(zone);
        }

        zone = NULL;
    }

    return 0;
}

#define MAX_MMZ_INFO_LEN (20*1024)

#define SPLIT_LINE "-------------------------------------------------------------------------------------------------------\n"

void dump_mmz_mem(void)
{
	mtl_mmz_t *p = NULL;

	int nZoneCnt = 0;
	unsigned int used_size = 0, free_size = 0;

	printk("dont_steal_av = %d\n", dont_steal_av);
	printk("|           PHYS           |  BLOCK COUNT  |   KVIRT   |    FLAGS    |  LENGTH(KB)  |       NAME        |\n");

	list_for_each_entry(p, &mmz_list, list) {
		mtl_mmb_t *mmb = NULL;
		unsigned int u32Number = 0;
		unsigned int used_size_zone = 0;

		list_for_each_entry(mmb, &p->mmb_list, list) {
			u32Number ++;
		}

		printk("|ZONE[%d]: (0x%016lx, 0x%016lx)   %d                 0x%08lx      %-10lu   \"%s%-14s|\n",nZoneCnt, \
			(ulong)((p)->phys_start), (ulong)((p)->phys_start+(p)->nbytes), u32Number, (p)->gfp,(p)->nbytes/SZ_1K,(p)->name, "\"");

		nZoneCnt ++;

		mmz_total_size += (p->nbytes / 1024);
		zone_number ++;

		list_for_each_entry(mmb, &p->mmb_list, list) {
			printk("|" MTL_MMB_FMT_S "|\n", mtl_mmb_fmt_arg(mmb));
			used_size += (mmb->length / 1024);
			used_size_zone += (mmb->length / 1024);
			block_number ++;
		}
		printk("|\"%s\" Used %d KB\n", p->name, used_size_zone);
	}

	printk(SPLIT_LINE);
	printk("|%-102s|\n", "Summary:");
	printk(SPLIT_LINE);
	printk("|  MMZ Total Size  |      Used      |     Idle        |  Zone Number  |   BLock Number                 |\n");
	printk(SPLIT_LINE);

	/*
	 * fix the wrong usage statistic
	 */
	if (zone_number == 1) {
		p = list_first_entry(&mmz_list, mtl_mmz_t, list);
		if (p->dev != NULL) {
			if (cma_count(p->dev)) {
				used_size = bitmap_weight(cma_bitmap(p->dev), cma_count(p->dev)) * 4;
			} else if (dma_count(p->dev)) {
				used_size = bitmap_weight(dma_bitmap(p->dev), dma_count(p->dev)) * 4;
			}
		}
	}

	if (0 != mmz_total_size) {
		free_size = mmz_total_size - used_size;
		printk("|       %d%-8s       %d%-8s      %d%-6s          %d                  %d                      |\n",
			 mmz_total_size/1024, "MB", used_size/1024, "MB", free_size/1024, "MB", zone_number, block_number);
		mmz_total_size = 0;
		zone_number = 0;
		block_number = 0;
	}
	printk(SPLIT_LINE);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
int mmz_read_proc(char *page, char **start, off_t off,
                  int count, int *eof, void *data)
{
	unsigned long ret = 0;
#if !(0 == MT_PROC_SUPPORT)
    int nZoneCnt = 0;
	mtl_mmz_t *p;
	unsigned int used_size = 0, free_size = 0;
	unsigned long len = 0;
	char * mmz_info_buf = (char*)__get_free_pages(GFP_TEMPORARY, get_order(MAX_MMZ_INFO_LEN));
	char * p_mmz_info_buf = mmz_info_buf;

	memset(mmz_info_buf, 0, MAX_MMZ_INFO_LEN);

	down(&mmz_lock);

	len = snprintf(page , count, SPLIT_LINE);
	memcpy(p_mmz_info_buf, page, len);
	p_mmz_info_buf += len;

	len = snprintf(page , count, "dont_steal_av = %d\n", dont_steal_av);
	memcpy(p_mmz_info_buf, page, len);
	p_mmz_info_buf += len;

	len = snprintf(page , count, SPLIT_LINE);
	memcpy(p_mmz_info_buf, page, len);
	p_mmz_info_buf += len;

    len = snprintf(page , count, "|           PHYS           |  BLOCK COUNT  |   KVIRT   |    FLAGS    |  LENGTH(KB)  |       NAME        |\n");
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;

    len = snprintf(page , count, SPLIT_LINE);
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;

	// Collect all mmb info into mmz_info_buff
	list_for_each_entry(p, &mmz_list, list){
		mtl_mmb_t *mmb;
		unsigned int u32Number = 0;
		unsigned int used_size_zone = 0;

        list_for_each_entry(mmb, &p->mmb_list, list){
			u32Number++;
		}

		len = snprintf(page , count, "|ZONE[%d]: (0x%016lx, 0x%016lx)   %d                 0x%08lx      %-10lu   \"%s%-14s|\n",nZoneCnt, \
                       (ulong)((p)->phys_start), (ulong)((p)->phys_start+(p)->nbytes), u32Number, (p)->gfp,(p)->nbytes/SZ_1K,(p)->name, "\"");
		memcpy(p_mmz_info_buf, page, len);
		p_mmz_info_buf += len;

        nZoneCnt++;

		mmz_total_size += p->nbytes / 1024;
		zone_number++;

		list_for_each_entry(mmb, &p->mmb_list, list){
			len = snprintf(page, count, "|" MTL_MMB_FMT_S "|\n", mtl_mmb_fmt_arg(mmb));
			memcpy(p_mmz_info_buf, page, len);
			p_mmz_info_buf += len;
			used_size += mmb->length/1024;
			used_size_zone += mmb->length/1024;
			block_number++;
		}
		snprintf(page , count, "|\"%s\" Used %d KB\n", p->name, used_size_zone);
	}

    len = snprintf(page , count, SPLIT_LINE);
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;

    len = snprintf(page , count, "|%-102s|\n", "Summary:");
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;

    len = snprintf(page , count, SPLIT_LINE);
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;


    len = snprintf(page , count, "|  MMZ Total Size  |      Used      |     Idle        |  Zone Number  |   BLock Number                 |\n");
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;

    len = snprintf(page , count, SPLIT_LINE);
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;

	if (zone_number == 1) {
		p = list_first_entry(&mmz_list, mtl_mmz_t, list);
		if (p->dev) {
			if (cma_count(p->dev)) {
				used_size = bitmap_weight(cma_bitmap(p->dev), cma_count(p->dev)) * 4;
			} else if (dma_count(p->dev)) {
				used_size = bitmap_weight(dma_bitmap(p->dev), dma_count(p->dev)) * 4;
			}
		}
	}

	if(0 != mmz_total_size){
		free_size = mmz_total_size - used_size;
	    len = snprintf(page, count, "|       %d%-8s       %d%-8s      %d%-6s          %d                  %d                      |\n",
				 mmz_total_size/1024, "MB", used_size/1024, "MB", free_size/1024, "MB", zone_number, block_number);
		memcpy(p_mmz_info_buf, page, len);
		p_mmz_info_buf += len;

		mmz_total_size = 0;
		zone_number = 0;
		block_number = 0;
	}

    len = snprintf(page , count, SPLIT_LINE);
    memcpy(p_mmz_info_buf, page, len);
    p_mmz_info_buf += len;

	// transfer info to proc buff page
	if (off +  count > p_mmz_info_buf - mmz_info_buf){
		memcpy(page, mmz_info_buf + off, p_mmz_info_buf - mmz_info_buf - off);
		*eof = 1;
		ret = p_mmz_info_buf - mmz_info_buf - off;
	}else{
		memcpy(page, mmz_info_buf + off, count);
		ret = count;
	}
	*start = page;

	up(&mmz_lock);

	free_pages((unsigned long)mmz_info_buf, get_order(MAX_MMZ_INFO_LEN) );
#endif

	return ret;
}
#else
int mmz_read_proc(struct seq_file *m, void *v) {
	int nZoneCnt = 0;
	mtl_mmz_t *p = NULL;
	unsigned int used_size = 0, free_size = 0;
#ifdef PRINT_MMAP_DEVICE
	struct memdev_mmap_info_t *memdev_mmap_info;
#endif

	#if !(0 == MT_PROC_SUPPORT)
	down(&mmz_lock);
	seq_puts(m,SPLIT_LINE);
	seq_printf(m , "dont_steal_av = %d\n", dont_steal_av);
	seq_puts(m,SPLIT_LINE);
	seq_puts(m, "|           PHYS           |  BLOCK COUNT  |   KVIRT   |    FLAGS    |  LENGTH(KB)  |       NAME        |\n");
	seq_puts(m,SPLIT_LINE);

	// Collect all mmb info into mmz_info_buff
	list_for_each_entry(p, &mmz_list, list) {
		mtl_mmb_t *mmb = NULL;
		unsigned int u32Number = 0;
		unsigned int used_size_zone = 0;

		list_for_each_entry(mmb, &p->mmb_list, list) {
			u32Number++;
		}

		seq_printf(m, "|ZONE[%d]: (0x%016lx, 0x%016lx)   %d                 0x%08lx      %-10lu   \"%s%-14s|\n",nZoneCnt, \
                       (ulong)((p)->phys_start), (ulong)((p)->phys_start+(p)->nbytes), u32Number, (p)->gfp,(p)->nbytes/SZ_1K,(p)->name, "\"");

		nZoneCnt ++;

		mmz_total_size += (p->nbytes / 1024);
		zone_number ++;

		list_for_each_entry(mmb, &p->mmb_list, list){
			seq_printf(m, "|" MTL_MMB_FMT_S "|\n", mtl_mmb_fmt_arg(mmb));
			used_size += (mmb->length / 1024);
			used_size_zone += (mmb->length / 1024);
			block_number ++;
		}
		seq_printf(m, "|\"%s\" Used %d KB\n", p->name, used_size_zone);
	}

	seq_puts(m, SPLIT_LINE);
	seq_printf(m, "|%-102s|\n", "Summary:");
	seq_puts(m, SPLIT_LINE);
	seq_puts(m, "|  MMZ Total Size  |      Used      |     Idle        |  Zone Number  |   BLock Number                 |\n");
	seq_puts(m, SPLIT_LINE);

	if (zone_number == 1) {
		p = list_first_entry(&mmz_list, mtl_mmz_t, list);
		if (p->dev != NULL) {
			if (cma_count(p->dev)) {
				used_size = bitmap_weight(cma_bitmap(p->dev), cma_count(p->dev)) * 4;
			} else if (dma_count(p->dev)) {
				used_size = bitmap_weight(dma_bitmap(p->dev), dma_count(p->dev)) * 4;
			}
		}
	}

	if (0 != mmz_total_size) {
		free_size = mmz_total_size - used_size;
		seq_printf(m, "|       %d%-8s       %d%-8s      %d%-6s          %d                  %d                      |\n",
				 mmz_total_size/1024, "MB", used_size/1024, "MB", free_size/1024, "MB", zone_number, block_number);

		mmz_total_size = 0;
		zone_number = 0;
		block_number = 0;
	}

	seq_puts(m, SPLIT_LINE);

#ifdef PRINT_MMAP_DEVICE
	seq_printf(m, "device mmap\n");
	list_for_each_entry(memdev_mmap_info, &memdev_mmap_info_list, list) {
		static char strace_buf[4096];
		seq_printf(m, "virt = 0x%lx, phys = 0x%lx\n", (ulong)memdev_mmap_info->virt, memdev_mmap_info->phys_addr);
		stack_trace_snprint(strace_buf, sizeof(strace_buf), memdev_mmap_info->entries, memdev_mmap_info->nr_entries, 1);
		seq_printf(m, "%s\n", strace_buf);
	}
	seq_puts(m, SPLIT_LINE);
#endif

	up(&mmz_lock);

	#endif
	return 0;
}
#endif

#undef TEST_MMZ_GAP
#ifdef TEST_MMZ_GAP
#define TEST_GAP_CNT 10

struct debug_device_t {
	unsigned int alloc_size[TEST_GAP_CNT];
	mmz_buffer_s p[TEST_GAP_CNT];
	bool needs_read_fill;
};

struct debug_device_t debug_gap_buffer;
struct dentry *debug_gap_dir;
struct dentry *debug_gap_alloc;
struct dentry *debug_gap_free;

static int debug_gap_open(struct inode *inode, struct file *file)
{
	struct debug_device_t *buffer;

	simple_open(inode, file);

	buffer = file->private_data;

	buffer->needs_read_fill = 1;

	return 0;
}

static ssize_t debug_gap_read(struct file *file, char __user *buf, size_t cnt, loff_t *postion)
{
	struct debug_device_t *buffer;
	int format_cnt = 0;
	unsigned long rest;
	int i;
	unsigned char format_buf[(TEST_GAP_CNT << 2) + 1 + 1];
	unsigned int size_buf[TEST_GAP_CNT];

	buffer = file->private_data;

	if (buffer->needs_read_fill == 0) {
		return 0;
	}

	buffer->needs_read_fill = 0;

	memcpy(size_buf, buffer->alloc_size, sizeof(buffer->alloc_size));

	for (i = 0; i < TEST_GAP_CNT; i++) {
		format_cnt += sprintf(format_buf + format_cnt, "%d ", size_buf[i]);
	}
	format_cnt += sprintf(format_buf + format_cnt, "\n");

	rest = copy_to_user(buf, (void *)format_buf, format_cnt);

	if (rest == -EFAULT)
		return -EFAULT;

	return format_cnt - rest;
}

static ssize_t debug_gap_alloc_write(struct file *file, const char __user *buf, size_t cnt, loff_t *postion)
{
	unsigned long rest;
	struct debug_device_t *buffer;
	unsigned long size;
	char *endp;
	unsigned char format_buf[(TEST_GAP_CNT << 2) + 1 + 1];
	unsigned int size_buf;
	unsigned int idx;
	char name[20];

	buffer = file->private_data;

	memset(format_buf, '\0', sizeof(format_buf));
	size = (unsigned long)cnt > (sizeof(format_buf) - 1) ? (sizeof(format_buf) - 1) : cnt;
	rest = copy_from_user((void *)format_buf, (void *)buf, size);
	if (rest == -EFAULT)
		return -EFAULT;

	endp = format_buf;
	idx = (unsigned int)simple_strtoul(endp, &endp, 0);
	endp++;
	size_buf = (unsigned int)simple_strtoul(endp, &endp, 0);

	sprintf(name, "gap_%d", idx);
	mt_drv_mmz_alloc_and_map(name, MT_NULL, size_buf, 0, &buffer->p[idx]);
	buffer->alloc_size[idx] = size_buf;

	buffer->needs_read_fill = 1;

	return cnt - rest;
}

static ssize_t debug_gap_free_write(struct file *file, const char __user *buf, size_t cnt, loff_t *postion)
{
	unsigned long rest;
	struct debug_device_t *buffer;
	unsigned long size;
	char *endp;
	unsigned char format_buf[(TEST_GAP_CNT << 2) + 1 + 1];
	unsigned int idx;

	buffer = file->private_data;

	memset(format_buf, '\0', sizeof(format_buf));
	size = (unsigned long)cnt > (sizeof(format_buf) - 1) ? (sizeof(format_buf) - 1) : cnt;
	rest = copy_from_user((void *)format_buf, (void *)buf, size);
	if (rest == -EFAULT)
		return -EFAULT;

	endp = format_buf;
	idx = (unsigned int)simple_strtoul(endp, &endp, 0);

	mt_drv_mmz_unmap_and_release(&buffer->p[idx]);
	buffer->alloc_size[idx] = 0;

	buffer->needs_read_fill = 1;

	return cnt - rest;
}

static struct file_operations debug_gap_alloc_fops = {
	.owner = THIS_MODULE,
	.open = debug_gap_open,
	.read = debug_gap_read,
	.write = debug_gap_alloc_write,
	.llseek = no_llseek,
};

static struct file_operations debug_gap_free_fops = {
	.owner = THIS_MODULE,
	.open = debug_gap_open,
	.read = debug_gap_read,
	.write = debug_gap_free_write,
	.llseek = no_llseek,
};
#endif

#define MMZ_SETUP_CMDLINE_LEN 	256
static char __initdata setup_zones[MMZ_SETUP_CMDLINE_LEN] = "ddr,0,0x8000000,120M";

static int init_done = 0;

/* mmz exit do mmz zone check and unregister */
static void mmz_exit_check(void)
{
    mtl_mmz_t *p;

    mmz_trace_func();

    /*
     * FIXME:
     *    mtl_mmz_find(0, NULL) could not find "av" zone etc,
     *    if dont_steal_av != 0.
     */
    for (p = mtl_mmz_find(0, NULL); p != NULL; p = mtl_mmz_find(0, NULL))
    {
        //PRINTK_CA(KERN_WARNING "MMZ force removed: " MTL_MMZ_FMT_S "\n", mtl_mmz_fmt_arg(p));
        mtl_mmz_unregister(p);
    }
}

int __init mt_drv_mmz_init(void)
{
    char *s;
    char *p = NULL;
    char *q;
    char *x;
    int i;

    //int len;
    if (init_done)
        return 0;

#ifdef TEST_MMZ_GAP
	debug_gap_dir = debugfs_create_dir("gap", NULL);
	debug_gap_alloc = debugfs_create_file("gap_alloc", S_IRWXU | S_IRUGO, debug_gap_dir, &debug_gap_buffer, &debug_gap_alloc_fops);
	debug_gap_free = debugfs_create_file("gap_free", S_IRWXU | S_IRUGO, debug_gap_dir, &debug_gap_buffer, &debug_gap_free_fops);
#endif

    //PRINTK_CA(KERN_INFO "Montage Media Memory Zone Manager state 0\n");

 //   len = strlen(setup_zones);
 //   if (len == 0)
    {
  //  #ifndef GRANT_COMPILE_TEST   //need add saved_command_line defined first
        strlcpy(line, saved_command_line, COMMAND_LINE_SIZE);
        x = line;

//		#endif
        for (i = 0; i < mmz_num_zones; i++) {
            q = strstr(x, "mmz=");
            if (q)
            {
                s = strsep(&q, "=");
                if (s)
                {
                    p = strsep(&q, " ");
                    x = q;
                }
                if (p)
                {
                    strlcpy(setup_zones, p, MMZ_SETUP_CMDLINE_LEN);
                    media_mem_parse_cmdline(setup_zones);
                }
            }
        }
    }
//printk("\n\n=11==MT_DRV_MMZ_Init====setup_zones===%s=====\n\n", setup_zones);

#ifdef DEBUG_MMZ
	mt_drv_mmz_register_notify(test_mmz_notify, NULL);
#endif

    init_done = 1;

    return 0;
}

void mt_drv_mmz_exit(void)
{
    mmz_exit_check();
    init_done = 0;
    return;
}

static struct
{
	mt_drv_mmz_notify_t notifier;
	void *priv;
} mmz_notifier = {NULL, NULL};

mt_s32 mt_drv_mmz_register_notify(mt_drv_mmz_notify_t notifier, void *priv)
{
	mmz_notifier.notifier = notifier;
	mmz_notifier.priv = priv;

	return MT_SUCCESS;
}

void mt_drv_mmz_unregister_notify(mt_drv_mmz_notify_t notifier, void *priv)
{
	mmz_notifier.notifier = NULL;
	mmz_notifier.priv = NULL;
}

mt_drv_mmz_notify_t mt_drv_mmz_get_notify(void **priv)
{
	*priv = mmz_notifier.priv;

	return mmz_notifier.notifier;
}

#ifdef DEBUG_MMZ
/* Test Notify */
static void test_mmz_notify(int op,	/* MT_MMZ_NOTIFY_ALLOC or MT_MMZ_NOTIFY_FREE */
							const char *zone_name,
							const char *name,
							unsigned long phys_addr, unsigned long length,
							mmz_security_attr_s *attr,
							void *priv)
{
	printk(KERN_INFO "%s: %d, %s, %s, %lx %lx, %x %d %d\r\n", __FUNCTION__,
		op, zone_name, name,
		phys_addr, length,
		attr->flags,
		attr->type,
		attr->subtype);
}
#endif

//EXPORT_SYMBOL(mtl_mmb_alloc);
//EXPORT_SYMBOL(mtl_mmb_free);
//EXPORT_SYMBOL(mtl_mmb_get);
//EXPORT_SYMBOL(mtl_mmb_put);
//EXPORT_SYMBOL(mmz_read_proc);
//EXPORT_SYMBOL(mmz_write_proc);

EXPORT_SYMBOL(mt_drv_mmz_register_notify);
EXPORT_SYMBOL(mt_drv_mmz_unregister_notify);

