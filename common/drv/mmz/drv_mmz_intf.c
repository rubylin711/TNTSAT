/* kcom.c
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
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
//#include <linux/devfs_fs_kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
//#include <mach/hardware.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

#include <linux/string.h>
#include <linux/list.h>

#include <linux/time.h>

#include "mt_drv_mmz.h"
#include "drv_media_mem.h"
#include "drv_mmz.h"


// mmz over cma can't  be created runtime
#if 0
kcom_mmz_t *new_zone(const char *name, unsigned long phys_start, unsigned long size)
{
	mtl_mmz_t *mmz;

	mmz = mtl_mmz_create(name, 0, phys_start, size);
	if(mmz ==NULL)
		return NULL;
	if(mtl_mmz_register(mmz) !=0) {
		mtl_mmz_destroy(mmz);
		return NULL;
	}

	return (kcom_mmz_t *)mmz;
}

void delete_zone(kcom_mmz_t *zone)
{
	mtl_mmz_unregister((kcom_mmz_t *)zone);
	mtl_mmz_destroy((kcom_mmz_t *)zone);
}
#endif
phys_addr_t new_mmb(const char *name, ulong size, unsigned long align,
				   const char *zone_name,
				   struct mt_drv_mmz_security_attr_s *attr)
{
	mtl_mmb_t *mmb;

	mmb = mtl_mmb_alloc(name, size, align, 0, zone_name, attr);
	if (mmb == NULL)
		return MMB_ADDR_INVALID;

	return mtl_mmb_phys(mmb);
}

void delete_mmb(phys_addr_t addr)
{
	mtl_mmb_t *mmb;

	if (addr == 0 || addr == MMB_ADDR_INVALID)
		return;

	mmb = mtl_mmb_getby_phys(addr);
	if (mmb == NULL)
		return;

	mtl_mmb_free(mmb);
}

void *remap_mmb(phys_addr_t addr)
{
	mtl_mmb_t *mmb;

	if (addr == 0 || addr == MMB_ADDR_INVALID)
		return NULL;

	mmb = mtl_mmb_getby_phys(addr);
	if (mmb == NULL)
		return NULL;

	return mtl_mmb_map2kern(mmb);
}

void *remap_mmb_cached(phys_addr_t addr)
{
	mtl_mmb_t *mmb;

	if (addr == 0 || addr == MMB_ADDR_INVALID)
		return NULL;

	mmb = mtl_mmb_getby_phys(addr);
	if (mmb == NULL)
		return NULL;

	return mtl_mmb_map2kern_cached(mmb);
}

#if 0
void *	refer_mapped_mmb(void *mapped_addr)
{
	mtl_mmb_t *mmb;

	mmb = mtl_mmb_getby_kvirt(mapped_addr);
	if(mmb ==NULL)
		return NULL;

	if(mmb->flags & MTL_MMB_MAP2KERN_CACHED)
		return mtl_mmb_map2kern_cached(mmb);
	else
		return mtl_mmb_map2kern(mmb);
}
#endif

int unmap_mmb(void *mapped_addr)
{
	mtl_mmb_t *mmb;

	if (mapped_addr == NULL)
		return (-1);

	mmb = mtl_mmb_getby_kvirt(mapped_addr);
	if(mmb ==NULL)
		return (-1);

	return mtl_mmb_unmap(mmb);
}

#if 0
mmb_addr_t mapped_to_mmb(void *mapped_addr)
{
	mtl_mmb_t *mmb;

	mmb = mtl_mmb_getby_kvirt(mapped_addr);
	if(mmb ==NULL)
		return MMB_ADDR_INVALID;

	return mtl_mmb_phys(mmb);
}
#endif

int get_mmb(phys_addr_t addr)
{
	mtl_mmb_t *mmb;

	if (addr == 0 || addr == MMB_ADDR_INVALID)
		return (-1);

	mmb = mtl_mmb_getby_phys(addr);
	if (mmb == NULL)
		return (-1);

	return mtl_mmb_get(mmb);
}

int put_mmb(phys_addr_t addr)
{
	mtl_mmb_t *mmb;

	if (addr == 0 || addr == MMB_ADDR_INVALID)
		return (-1);

	mmb = mtl_mmb_getby_phys(addr);
	if (mmb == NULL)
		return (-1);

	return mtl_mmb_put(mmb);
}

/*
** Input is physaddr, is allocated by MMZ.
** Maybe this addr is not the original base physical address.
** return value is the real virsual addres
** the original base visual address can be get from VBaddr.
** *Outoffset is offset between VBaddr and
**User should use "viraddr + *Outoffset" as the real virsual address.
*/
void *remap_mmb_2(phys_addr_t phyaddr, ulong *VBaddr, phys_addr_t *Outoffset)
{
    void *viraddr = 0, *virrealaddr;
	mtl_mmb_t *mmb;

	if (phyaddr == 0 || phyaddr == MMB_ADDR_INVALID || Outoffset == NULL)
		return NULL;

    *Outoffset = 0;
	mmb = mtl_mmb_getby_phys_2(phyaddr, Outoffset);
	if (mmb == NULL)
		return NULL;

	viraddr = mtl_mmb_map2kern(mmb);
	if (viraddr == NULL)
		return NULL;

    *VBaddr      = (ulong)viraddr;
    virrealaddr = viraddr + *Outoffset;

    return virrealaddr;
}
/*
**mapped_addr may not be original base visual address.
**original base visual address should be:(mapped_VBaddr - offset)
*/
int unmap_mmb_2(void *mapped_addr, unsigned long offset)
{
	mtl_mmb_t *mmb;

    if (mapped_addr == NULL) {
        return (-1);
    }
    mmb = mtl_mmb_getby_kvirt((mapped_addr - offset));
    if (mmb == NULL) {
        return (-1);
    }

    return mtl_mmb_unmap(mmb);
}


#if 0
EXPORT_SYMBOL(new_zone);
EXPORT_SYMBOL(delete_zone);
#endif
EXPORT_SYMBOL(new_mmb);
EXPORT_SYMBOL(delete_mmb);
EXPORT_SYMBOL(remap_mmb);
EXPORT_SYMBOL(remap_mmb_cached);
/*EXPORT_SYMBOL(refer_mapped_mmb);*/
EXPORT_SYMBOL(unmap_mmb);

EXPORT_SYMBOL(get_mmb);
EXPORT_SYMBOL(put_mmb);

EXPORT_SYMBOL(remap_mmb_2);
EXPORT_SYMBOL(unmap_mmb_2);

