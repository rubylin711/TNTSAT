/* userdev.c
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
#include <linux/mman.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/mt_mmz.h>
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "drv_media_mem.h"
#include "drv_mmz_ioctl.h"
#include "mt_kernel_adapt.h"
#include "mt_cache.h"
#include "drv_mmz.h"

#define mmz_error(s...) 	do { PRINTK_CA(KERN_ERR "mmz_userdev:%s: ", __FUNCTION__); PRINTK_CA(s); } while(0)
#define mmz_warning(s...) 	do { PRINTK_CA(KERN_WARNING "mmz_userdev:%s: ", __FUNCTION__); PRINTK_CA(s); } while(0)


/****************************fops*********************************/

struct mmz_userdev_info {
	pid_t pid;
	struct semaphore sem;
	struct list_head list;
};

static DEFINE_SPINLOCK(cache_lock);

static int mmz_flush_dcache_mmb(struct kmmb_info *pmi, ulong offset, ulong size)
{
	mtl_mmb_t *mmb = NULL;
	unsigned long flags;

	if (pmi == NULL)
		return -EINVAL;

	mmb = pmi->mmb;
	if (mmb == NULL || pmi->map_cached == 0) {
		PRINTK_CA(KERN_DEBUG "%s->%d,error!\n", __func__,__LINE__);
		return -EINVAL;
	}

	if (pmi->mapped == NULL || pmi->phys_addr == 0 || pmi->size == 0)
	{
		return -EINVAL;
	}

	spin_lock_irqsave(&cache_lock, flags);

	if ((size != 0) && (offset + size > pmi->size)) {
		BUG_ON(1);
	}

	mt_user_dcache_flush(pmi->mapped,
						 size == 0 ? 0 : offset,
						 size == 0 ? pmi->size : size);

	spin_unlock_irqrestore(&cache_lock, flags);

	return 0;
}

static int mmz_inv_dcache_mmb(struct kmmb_info *pmi, ulong offset, ulong size)
{
    mtl_mmb_t *mmb = NULL;
    unsigned long flags;

    if (pmi == NULL)
        return -EINVAL;

    mmb = pmi->mmb;
    if (mmb == NULL || pmi->map_cached == 0) {
        PRINTK_CA(KERN_DEBUG "%s->%d,error!\n", __func__,__LINE__);
        return -EINVAL;
    }

	if (pmi->mapped == NULL || pmi->phys_addr == 0 || pmi->size == 0)
	{
		return -EINVAL;
	}

    spin_lock_irqsave(&cache_lock, flags);

	if ((size != 0) && (offset + size > pmi->size)) {
		BUG_ON(1);
	}

    mt_user_dcache_invalid(pmi->mapped,
    					   size == 0 ? 0 : offset,
						   size == 0 ? pmi->size : size);

    spin_unlock_irqrestore(&cache_lock, flags);

    return 0;
}

static int mmz_flush_dcache_all(void)
{
	mt_dcache_flush_all();
	return 0;
}

int mmz_userdev_open(struct inode *inode, struct file *file)
{
	struct mmz_userdev_info *pmu = NULL;

	pmu = kzalloc(sizeof(*pmu), GFP_KERNEL);
	if (pmu == NULL) {
		mmz_error("alloc mmz_userdev_info failed!\n");
		return -ENOMEM;
	}
	pmu->pid = current->tgid;
	MT_INIT_MUTEX(&pmu->sem);
	INIT_LIST_HEAD(&pmu->list);

	file->private_data = (void*)pmu;

	return 0;
}

static int ioctl_mmb_alloc(struct file *file, unsigned int iocmd, struct kmmb_info *pmi)
{
	struct mmz_userdev_info *pmu = file->private_data;
	struct kmmb_info *new_mmbinfo = NULL;
	mtl_mmb_t *mmb = NULL;
	struct mt_drv_mmz_security_attr_s attr = {0};

    if (pmi == NULL)
    {
        return -EINVAL;
    }

	/* NOTE: add all fields here: */
    attr.flags   = pmi->attr.flags;
    attr.type    = pmi->attr.type;
    attr.subtype = pmi->attr.subtype;
	memcpy(attr.reserved, pmi->attr.reserved, sizeof(attr.reserved));

	mmb = mtl_mmb_alloc(pmi->mmb_name, pmi->size, pmi->align, pmi->gfp, pmi->mmz_name, &attr);
	if (mmb == NULL) {
		dump_mmz_mem();
		return -ENOMEM;
	}

	new_mmbinfo = kzalloc(sizeof(*new_mmbinfo), GFP_KERNEL);
	if (new_mmbinfo == NULL) {
		mtl_mmb_free(mmb);
		mmz_error("alloc mmb_info failed!\n");
		return -ENOMEM;
	}

	memcpy(new_mmbinfo, pmi, sizeof(*new_mmbinfo));
	new_mmbinfo->phys_addr = mtl_mmb_phys(mmb);
	new_mmbinfo->size = mtl_mmb_length(mmb);
	new_mmbinfo->mmb = mmb;
	new_mmbinfo->pid = current->tgid;
	list_add_tail(&new_mmbinfo->list, &pmu->list);

	pmi->phys_addr = new_mmbinfo->phys_addr;
	pmi->size = new_mmbinfo->size;

	mtl_mmb_get(mmb);

	return 0;
}

/* get mmb info by virtual address */
static struct kmmb_info* get_mmbinfo_vaddr(void *addr, struct mmz_userdev_info *pmu, bool range)
{
	struct kmmb_info *p;
	struct kmmb_info *temp;

	if (addr == NULL)
		return NULL;

	list_for_each_entry_safe(p, temp, &pmu->list, list) {
		if (!range) {
			if (p->mapped == addr) {
				break;
			}
		} else {
			if (p->mapped <= addr && p->mapped + p->size > addr) {
				break;
			}
		}
	}

	if (&p->list == &pmu->list) {
		return NULL;
	}

	return p;
}

static struct kmmb_info* get_mmbinfo_vbase(void *addr, struct mmz_userdev_info *pmu)
{
	struct kmmb_info *p;

	p = get_mmbinfo_vaddr(addr, pmu, 0);

	if (p == NULL) {
		mmz_error("mmb(0x%016lx) not found!\n", (ulong)addr);
		return NULL;
	}

	return p;
}

static struct kmmb_info* get_mmbinfo_vrange(void *addr, struct mmz_userdev_info *pmu)
{
	struct kmmb_info *p;

	p = get_mmbinfo_vaddr(addr, pmu, 1);

	if (p == NULL) {
		mmz_error("mmb(0x%016lx) not found!\n", (ulong)addr);
		return NULL;
	}

	return p;
}

/* get mmb info by phys address */
static struct kmmb_info* get_mmbinfo_paddr(phys_addr_t paddr, struct mmz_userdev_info *pmu, bool range)
{
	struct kmmb_info *p;
	struct kmmb_info *temp;

	if (paddr == 0)
		return NULL;

	list_for_each_entry_safe(p, temp, &pmu->list, list) {
		if (!range) {
			if (p->phys_addr == paddr) {
				break;
			}
		} else {
			if (p->phys_addr <= paddr && p->phys_addr + p->size > paddr) {
				break;
			}
		}
	}

	if (&p->list == &pmu->list) {
		return NULL;
	}

	return p;
}

static struct kmmb_info* get_mmbinfo_pbase(phys_addr_t paddr, struct mmz_userdev_info *pmu)
{
	struct kmmb_info *p;

	p = get_mmbinfo_paddr(paddr, pmu, 0);

	if (p == NULL) {
		mmz_error("mmb(0x%016lx) not found!\n", (ulong)paddr);
		return NULL;
	}

	return p;
}

__used static struct kmmb_info* get_mmbinfo_prange(phys_addr_t paddr, struct mmz_userdev_info *pmu)
{
	struct kmmb_info *p;

	p = get_mmbinfo_paddr(paddr, pmu, 1);

	if (p == NULL) {
		mmz_error("mmb(0x%016lx) not found!\n", (ulong)paddr);
		return NULL;
	}

	return p;
}

static int _usrdev_mmb_free(struct kmmb_info *p)
{
	int ret = 0;

	if (p == NULL || p->mmb == NULL)
		return (-1);

	list_del(&p->list);
	mtl_mmb_put(p->mmb);
	ret = mtl_mmb_free(p->mmb);
	kfree(p);

	return ret;
}

static int ioctl_mmb_free(struct file *file, unsigned int iocmd, struct kmmb_info *pmi)
{
	int ret = 0;
	struct mmz_userdev_info *pmu = file->private_data;
	struct kmmb_info *p;

	if (pmi == NULL || pmi->phys_addr == 0)
		return -EINVAL;

	if ((p = get_mmbinfo_pbase(pmi->phys_addr, pmu)) == NULL) {
		return -EPERM;
	}

	if (p->delayed_free) {
		mmz_warning("mmb<%s> is delayed_free, can not free again!\n", p->mmb->name);
		return -EBUSY;
	}

	if (p->map_ref > 0 || p->mmb_ref > 0) {
		mmz_warning("mmb<%s> is still used!\n", p->mmb->name);
		p->delayed_free = 1;
		return -EBUSY;
	}

	ret = _usrdev_mmb_free(p);

	return ret;
}

/* get mmb info by physical address */
static int ioctl_mmb_attr(struct file *file, unsigned int iocmd, struct kmmb_info *pmi)
{
	struct mmz_userdev_info *pmu = file->private_data;
	struct kmmb_info *p;

	if (pmi == NULL || pmi->phys_addr == 0)
		return -EINVAL;

	if ((p = get_mmbinfo_vbase(pmi->mapped, pmu)) == NULL)
		return -EPERM;

	memcpy(pmi, p, sizeof(*pmi));
	return 0;
}

static int ioctl_mmb_user_remap(struct file *file, unsigned int iocmd, struct kmmb_info *pmi, int cached)
{
	struct mmz_userdev_info *pmu = file->private_data;
	struct kmmb_info *p;

	if (pmi == NULL || pmi->phys_addr == 0)
		return -EINVAL;

	pmi->mapped = NULL;

	if ((p = get_mmbinfo_pbase(pmi->phys_addr, pmu)) == NULL) {
		return -EPERM;
	}
	if (p->mapped && p->map_ref > 0) {
		p->map_ref++;
		p->mmb_ref++;
		pmi->mapped = p->mapped;
		mtl_mmb_get(p->mmb);
		if (p->map_cached != cached) {
			mmz_error("mmb<%s> cached not match\n", p->mmb->name);
		}
		return 0;
	}

	pmi->prot = p->prot;
	pmi->flags = p->flags;
	pmi->size = p->size;
	pmi->map_cached = p->map_cached = cached;

	p->map_ref++;
	p->mmb_ref++;
	mtl_mmb_get(p->mmb);

	return 0;
}

static int ioctl_mmb_user_unmap(struct file *file, unsigned int iocmd, struct kmmb_info *pmi)
{
	struct kmmb_info *p;
	struct mmz_userdev_info *pmu = file->private_data;

	if (pmi == NULL)
		return -EINVAL;

	if ((p = get_mmbinfo_vbase(pmi->mapped, pmu)) == NULL) {
		return -EPERM;
	}

	if (!p->mapped) {
		PRINTK_CA(KERN_WARNING "mmb(0x%016lx) have'nt been user-mapped yet!\n", (ulong)p->phys_addr);
		return -EIO;
	}

	if (!(p->map_ref > 0 && p->mmb_ref > 0)) {
		mmz_error("mmb<%s> has invalid refer: map_ref = %d, mmb_ref = %d.\n", p->mmb->name, p->map_ref, p->mmb_ref);
		return -EIO;
	}

	p->map_ref--;
	p->mmb_ref--;
	mtl_mmb_put(p->mmb);

	if (p->map_ref > 0) {
		return 0;
	}

	pmi->size = p->size;
	pmi->phys_addr = p->phys_addr;
	p->mapped = NULL;

	if (p->delayed_free && p->mmb_ref == 0) {
		_usrdev_mmb_free(p);
	}

	return 0;
}


// find mmbinfo by use addr
static struct kmmb_info* get_mmbinfo_byusraddr(void *addr, struct mmz_userdev_info *pmu)
{
	struct kmmb_info *p;
	struct kmmb_info *temp;

	if (addr == 0)
		return NULL;

	list_for_each_entry_safe(p, temp, &pmu->list, list) {
		if ((p->mapped <= addr) &&
			(p->mapped + p->size > addr) &&
			(p->pid == current->tgid)) {
				break;
		}
	}
	if (&p->list == &pmu->list) {
		return NULL;
	}

	return p;
}

/*To make sure ref get and release, both get and put interface shoude be exist,
	but customers make sure theirselves, will not release in using */
/*CNcomment: ? get/put???ref??????y??????в?*/
static int ioctl_mmb_user_getphyaddr(struct file *file, unsigned int iocmd, struct kmmb_info *pmi)
{
	struct kmmb_info *p;
	struct mmz_userdev_info *pmu = file->private_data;

	if (pmi == NULL || pmi->mapped == NULL)
		return -EINVAL;

	p = get_mmbinfo_byusraddr(pmi->mapped, pmu);
	if (p == NULL) {
		return -EPERM;
	}

	if (!(p->map_ref > 0 && p->mmb_ref > 0)) {
		mmz_error("mmb<%s> has invalid refer: map_ref = %d, mmb_ref = %d.\n", p->mmb->name, p->map_ref, p->mmb_ref);
		return -EIO;
	}

	pmi->phys_addr = p->phys_addr + ((unsigned long)pmi->mapped - (unsigned long)p->mapped);
	pmi->size = p->size - ((unsigned long)pmi->mapped - (unsigned long)p->mapped);
	return 0;
}

static int ioctl_mmb_user_test(struct file *file, unsigned int iocmd, struct kmmb_info *pmi)
{
	//struct mmz_userdev_info *pmu = file->private_data;
	int i = 0;
    unsigned long test_addr = (ulong)remap_mmb(pmi->phys_addr);

	//remap_mmb
	MT_INFO_LOG("\n=@@@@===test mmz===phys_addr=0x%lx=test_addr=0x%p\n", test_addr, pmi->mapped);
	for (i=0; i<0x100; i++)
	{
        MT_INFO_LOG("0x%x ",*(mt_u8*)(test_addr + 32 + i));
	}
    MT_INFO_LOG("\n");
    memset((void*)(test_addr+16), 0x31, 0x8000);
	for (i=0; i<100; i++)
	{
        MT_INFO_LOG("0x%x ",*(mt_u8*)(test_addr+i));
	}
	MT_INFO_LOG("\n");
	return 0;
}

static int ioctl_mmz_get_start_size(struct kmmz_start_size *kmmz_start_size)
{
	int i;

	for (i = 0; i < mmz_num_zones; i++) {
		if (!strcmp(kmmz_start_size->mmz_name, mont_zone[i].name)) {
			kmmz_start_size->phys_start = mont_zone[i].phys_start;
			kmmz_start_size->nbytes = mont_zone[i].nbytes;
			break;
		}
	}
	if (i >= mmz_num_zones) {
		kmmz_start_size->phys_start = 0;
		kmmz_start_size->nbytes = 0;
	}

	return 0;
}

static int mmz_userdev_ioctl_m(struct inode *inode, struct file *file, unsigned int cmd, struct kmmb_info *pmi)
{
	int ret = 0;

	switch (_IOC_NR(cmd)) {
		case _IOC_NR(IOC_MMB_ALLOC):
			ret = ioctl_mmb_alloc(file, cmd, pmi);
			break;
		case _IOC_NR(IOC_MMB_ATTR):
			ret = ioctl_mmb_attr(file, cmd, pmi);
			break;
		case _IOC_NR(IOC_MMB_FREE):
			ret = ioctl_mmb_free(file, cmd, pmi);
			break;

		case _IOC_NR(IOC_MMB_USER_REMAP):
			ret = ioctl_mmb_user_remap(file, cmd, pmi, 0);
			break;
		case _IOC_NR(IOC_MMB_USER_REMAP_CACHED):
			ret = ioctl_mmb_user_remap(file, cmd, pmi, 1);
			break;
		case _IOC_NR(IOC_MMB_USER_UNMAP):
			ret = ioctl_mmb_user_unmap(file, cmd, pmi);
			break;
		case _IOC_NR(IOC_MMB_USER_GETPHYADDR):
			ret = ioctl_mmb_user_getphyaddr(file, cmd, pmi);
			break;
		case _IOC_NR(IOC_MMB_USER_TEST_S):
			ret = ioctl_mmb_user_test(file, cmd, pmi);
			break;
		default:
			mmz_error("invalid ioctl cmd = %08X\n", cmd);
			ret = -EINVAL;
			break;
	}

	return ret;
}

static int mmz_userdev_ioctl_r(struct inode *inode, struct file *file, unsigned int cmd, struct kmmb_info *pmi)
{
	switch(_IOC_NR(cmd)) {
		case _IOC_NR(IOC_MMB_ADD_REF):
			pmi->mmb_ref++;
			mtl_mmb_get(pmi->mmb);
			break;
		case _IOC_NR(IOC_MMB_DEC_REF):
			if(pmi->mmb_ref <=0) {
				mmz_error("mmb<%s> mmb_ref is %d!\n", pmi->mmb->name, pmi->mmb_ref);
				return -EPERM;
			}
			pmi->mmb_ref--;
			mtl_mmb_put(pmi->mmb);
			if(pmi->delayed_free && pmi->mmb_ref==0) {
				_usrdev_mmb_free(pmi);
			}
			break;
		default:
			return -EINVAL;
			break;
	}

	return 0;
}

/* just for test */
static int mmz_userdev_ioctl_t(struct inode *inode, struct file *file, unsigned int cmd, struct kmmb_info *pmi)
{
	return 0;
}

static void set_kmmb(struct kmmb_info *kmi, struct mmb_info *mi)
{
	kmi->phys_addr = mi->phys_addr;
	kmi->align = mi->align;
	kmi->size = mi->size;
	kmi->order = mi->order;
	kmi->mapped = mi->mapped;
	kmi->prot = mi->prot;
	kmi->flags = mi->flags;
	strncpy(kmi->mmb_name, mi->mmb_name, MTL_MMB_NAME_LEN);
	strncpy(kmi->mmz_name, mi->mmz_name, MTL_MMZ_NAME_LEN);
	kmi->gfp = mi->gfp;
	kmi->pid = mi->pid;
	kmi->attr = mi->attr;
}

static void set_mmb(struct mmb_info *mi, struct kmmb_info *kmi)
{
	mi->phys_addr = kmi->phys_addr;
	mi->align = kmi->align;
	mi->size = kmi->size;
	mi->order = kmi->order;
	mi->mapped = kmi->mapped;
	mi->prot = kmi->prot;
	mi->flags = kmi->flags;
	strncpy(mi->mmb_name, kmi->mmb_name, MTL_MMB_NAME_LEN);
	strncpy(mi->mmz_name, kmi->mmz_name, MTL_MMZ_NAME_LEN);
	mi->gfp = kmi->gfp;
	mi->pid = kmi->pid;
	mi->attr = kmi->attr;
}

static void set_kmmz_start_size(struct kmmz_start_size *kmmz_start_size, struct mmz_start_size *mmz_start_size)
{
	strncpy(kmmz_start_size->mmz_name, mmz_start_size->mmz_name, MTL_MMZ_NAME_LEN);
}

static void set_mmz_start_size(struct mmz_start_size *mmz_start_size, struct kmmz_start_size *kmmz_start_size)
{
	mmz_start_size->phys_start = kmmz_start_size->phys_start;
	mmz_start_size->nbytes = kmmz_start_size->nbytes;
}

long mmz_userdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct mmz_userdev_info *pmu = file->private_data;

	down(&pmu->sem);

	if (_IOC_TYPE(cmd) == 'm') {
		struct mmb_info mi;
		struct kmmb_info kmi = {0};

		if (_IOC_SIZE(cmd) > sizeof(mi) || arg == 0) {
			mmz_error("_IOC_SIZE(cmd)=%d, arg==0x%08lX\n", _IOC_SIZE(cmd), arg);
			ret = -EINVAL;
			goto __error_exit;
		}
		memset(&mi, 0, sizeof(mi));
		if (copy_from_user(&mi, (void*)arg, _IOC_SIZE(cmd))) {
            PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
			goto __error_exit;
		}
		set_kmmb(&kmi, &mi);

		ret = mmz_userdev_ioctl_m(file->f_path.dentry->d_inode, file, cmd, &kmi);

		if (!ret && (cmd & IOC_OUT)) {
			set_mmb(&mi, &kmi);
			if (copy_to_user((void*)arg, &mi, _IOC_SIZE(cmd))) {
                PRINTK_CA("\nmmz_userdev_ioctl: copy_to_user error.\n");
                ret = -EFAULT;
			    goto __error_exit;
		    }
		}

	} else if (_IOC_TYPE(cmd) == 'r') {
		struct kmmb_info *pmi;
		if ((pmi = get_mmbinfo_vbase((void *)arg, pmu)) == NULL) {
			ret = -EPERM;
			goto __error_exit;
		}

		ret = mmz_userdev_ioctl_r(file->f_path.dentry->d_inode, file, cmd, pmi);

	} else if (_IOC_TYPE(cmd) == 'c') {
		struct cache_op_mmz_area coa;
		struct kmmb_info *pmi;

		if ((void *)arg == NULL) {
			mmz_flush_dcache_all();
			goto __error_exit;
		}

		if (copy_from_user(&coa, (void *)arg, sizeof(coa))) {
            PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
		}

		if ((pmi = get_mmbinfo_vrange(coa.vbase, pmu)) == NULL) {
			ret = -EPERM;
			goto __error_exit;
		}

		switch (_IOC_NR(cmd)) {
		case _IOC_NR(IOC_MMB_FLUSH_DCACHE):
			mmz_flush_dcache_mmb(pmi, (ulong)(coa.vbase - pmi->mapped) + coa.offset, coa.size);
			break;
		case _IOC_NR(IOC_MMB_INV_DCACHE):
			mmz_inv_dcache_mmb(pmi, (ulong)(coa.vbase - pmi->mapped) + coa.offset, coa.size);
			break;
		default:
			ret = -EINVAL;
			break;
		}

	} else if (_IOC_TYPE(cmd) == 't') {
		struct mmb_info mi;
		struct kmmb_info *pmi;

		if (_IOC_SIZE(cmd) > sizeof(mi) || arg == 0) {
			mmz_error("_IOC_SIZE(cmd)=%d, arg==0x%08lX\n", _IOC_SIZE(cmd), arg);
			ret = -EINVAL;
			goto __error_exit;
		}

		memset(&mi, 0, sizeof(mi));
		if (copy_from_user(&mi, (void*)arg, _IOC_SIZE(cmd))) {
            PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
		}

		if ((pmi = get_mmbinfo_vbase(mi.mapped, pmu)) == NULL) {
			ret = -EPERM;
			goto __error_exit;
		}

		ret = mmz_userdev_ioctl_t(file->f_path.dentry->d_inode, file, cmd, pmi);
	} else if (_IOC_TYPE(cmd) == 'z') {
		struct mmz_start_size mmz_start_size;
		struct kmmz_start_size kmmz_start_size;

		if (copy_from_user(&mmz_start_size, (void*)arg, _IOC_SIZE(cmd))) {
			PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
			ret = -EFAULT;
			goto __error_exit;
		}

		switch(_IOC_NR(cmd)) {
		case _IOC_NR(IOC_MMZ_GET_START_SIZE):
			set_kmmz_start_size(&kmmz_start_size, &mmz_start_size);
			ret = ioctl_mmz_get_start_size(&kmmz_start_size);
			set_mmz_start_size(&mmz_start_size, &kmmz_start_size);
			break;
		default:
			ret = -EINVAL;
			break;
		}

		if (!ret && (cmd & IOC_OUT)) {
			if (copy_to_user((void*)arg, &mmz_start_size, _IOC_SIZE(cmd))) {
				PRINTK_CA("\nmmz_userdev_ioctl: copy_to_user error.\n");
				ret = -EFAULT;
				goto __error_exit;
		    }
		}
	} else {
		ret = -EINVAL;
	}

__error_exit:

	up(&pmu->sem);

	return ret;
}

#ifdef CONFIG_COMPAT
static void compat_set_kmmb(struct kmmb_info *kmi, struct compat_mmb_info *mi)
{
	kmi->phys_addr = mi->phys_addr;
	kmi->align = mi->align;
	kmi->size = mi->size;
	kmi->order = mi->order;
	kmi->mapped = (void *)((ulong)mi->mapped);
	kmi->prot = mi->prot;
	kmi->flags = mi->flags;
	strncpy(kmi->mmb_name, mi->mmb_name, MTL_MMB_NAME_LEN);
	strncpy(kmi->mmz_name, mi->mmz_name, MTL_MMZ_NAME_LEN);
	kmi->gfp = mi->gfp;
	kmi->pid = mi->pid;
	kmi->attr = mi->attr;
}

static void compat_set_mmb(struct compat_mmb_info *mi, struct kmmb_info *kmi)
{
	mi->phys_addr = kmi->phys_addr;
	mi->align = kmi->align;
	mi->size = kmi->size;
	mi->order = kmi->order;
	mi->mapped = (compat_uptr_t)((ulong)kmi->mapped);
	mi->prot = kmi->prot;
	mi->flags = kmi->flags;
	strncpy(mi->mmb_name, kmi->mmb_name, MTL_MMB_NAME_LEN);
	strncpy(mi->mmz_name, kmi->mmz_name, MTL_MMZ_NAME_LEN);
	mi->gfp = kmi->gfp;
	mi->pid = kmi->pid;
	mi->attr = kmi->attr;
}

static void compat_set_kmmz_start_size(struct kmmz_start_size *kmmz_start_size, struct compat_mmz_start_size *compat_mmz_start_size)
{
	strncpy(kmmz_start_size->mmz_name, compat_mmz_start_size->mmz_name, MTL_MMZ_NAME_LEN);
}

static void compat_set_mmz_start_size(struct compat_mmz_start_size *compat_mmz_start_size, struct kmmz_start_size *kmmz_start_size)
{
	compat_mmz_start_size->phys_start = kmmz_start_size->phys_start;
	compat_mmz_start_size->nbytes = kmmz_start_size->nbytes;
}

long compat_mmz_userdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct mmz_userdev_info *pmu = file->private_data;
	unsigned int cmd64;

	down(&pmu->sem);

	if (_IOC_TYPE(cmd) == 'm') {
		struct compat_mmb_info mi;
		struct kmmb_info kmi = {0};

		if (_IOC_SIZE(cmd) > sizeof(mi) || arg == 0) {
			mmz_error("_IOC_SIZE(cmd)=%d, arg==0x%08lX\n", _IOC_SIZE(cmd), arg);
			ret = -EINVAL;
			goto __error_exit;
		}
		memset(&mi, 0, sizeof(mi));
		if (copy_from_user(&mi, (void*)arg, _IOC_SIZE(cmd)))
		{
            PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
			goto __error_exit;
		}
		compat_set_kmmb(&kmi, &mi);

		switch (_IOC_NR(cmd)) {
		case _IOC_NR(COMPAT_IOC_MMB_ALLOC):
			cmd64 = IOC_MMB_ALLOC;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_ATTR):
			cmd64 = IOC_MMB_ATTR;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_FREE):
			cmd64 = IOC_MMB_FREE;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_USER_REMAP):
			cmd64 = IOC_MMB_USER_REMAP;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_USER_REMAP_CACHED):
			cmd64 = IOC_MMB_USER_REMAP_CACHED;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_USER_UNMAP):
			cmd64 = IOC_MMB_USER_UNMAP;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_USER_GETPHYADDR):
			cmd64 = IOC_MMB_USER_GETPHYADDR;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_USER_TEST_S):
			cmd64 = IOC_MMB_USER_TEST_S;
			break;
		default:
			mmz_error("invalid ioctl cmd = %08X\n", cmd);
			return -EINVAL;
		}

		ret = mmz_userdev_ioctl_m(file->f_path.dentry->d_inode, file, cmd64, &kmi);

		if (!ret && (cmd & IOC_OUT)) {
			compat_set_mmb(&mi, &kmi);
			if (copy_to_user((void*)arg, &mi, _IOC_SIZE(cmd))) {
                PRINTK_CA("\nmmz_userdev_ioctl: copy_to_user error.\n");
                ret = -EFAULT;
			    goto __error_exit;
		    }
		}

	} else if (_IOC_TYPE(cmd) == 'r') {
		struct kmmb_info *pmi;
		if ((pmi = get_mmbinfo_vbase((void *)arg, pmu)) == NULL) {
			ret = -EPERM;
			goto __error_exit;
		}

		switch (_IOC_NR(cmd)) {
		case _IOC_NR(COMPAT_IOC_MMB_ADD_REF):
			cmd64 = IOC_MMB_ADD_REF;
			break;
		case _IOC_NR(COMPAT_IOC_MMB_DEC_REF):
			cmd64 = IOC_MMB_DEC_REF;
			break;
		default:
			return -EINVAL;
		}

		ret = mmz_userdev_ioctl_r(file->f_path.dentry->d_inode, file, cmd64, pmi);

	} else if (_IOC_TYPE(cmd) == 'c') {
		struct compat_cache_op_mmz_area coa;
		struct kmmb_info *pmi;

		if ((void *)arg == NULL) {
			mmz_flush_dcache_all();
			goto __error_exit;
		}

		if (copy_from_user(&coa, (void *)arg, sizeof(coa))) {
            PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
		}

		if ((pmi = get_mmbinfo_vrange((void *)((ulong)coa.vbase), pmu)) == NULL) {
			ret = -EPERM;
			goto __error_exit;
		}

		switch (_IOC_NR(cmd)) {
		case _IOC_NR(COMPAT_IOC_MMB_FLUSH_DCACHE):
			mmz_flush_dcache_mmb(pmi, (ulong)((void *)((ulong)coa.vbase) - pmi->mapped) + coa.offset, (ulong)coa.size);
			break;
		case _IOC_NR(COMPAT_IOC_MMB_INV_DCACHE):
			mmz_inv_dcache_mmb(pmi, (ulong)((void *)((ulong)coa.vbase) - pmi->mapped) + coa.offset, (ulong)coa.size);
			break;
		default:
			ret = -EINVAL;
			break;
		}

	} else if (_IOC_TYPE(cmd) == 't') {
		struct compat_mmb_info mi;
		struct kmmb_info *pmi;

		if (_IOC_SIZE(cmd) > sizeof(mi) || arg == 0) {
			mmz_error("_IOC_SIZE(cmd)=%d, arg==0x%08lX\n", _IOC_SIZE(cmd), arg);
			ret = -EINVAL;
			goto __error_exit;
		}

		memset(&mi, 0, sizeof(mi));
		if (copy_from_user(&mi, (void*)arg, _IOC_SIZE(cmd))) {
            PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
		}

		if ((pmi = get_mmbinfo_vbase((void *)((ulong)mi.mapped), pmu)) == NULL) {
			ret = -EPERM;
			goto __error_exit;
		}

		ret = mmz_userdev_ioctl_t(file->f_path.dentry->d_inode, file, cmd, pmi);
	} else if (_IOC_TYPE(cmd) == 'z') {
		struct compat_mmz_start_size compat_mmz_start_size;
		struct kmmz_start_size kmmz_start_size;

		if (copy_from_user(&compat_mmz_start_size, (void*)arg, _IOC_SIZE(cmd))) {
			PRINTK_CA("\nmmz_userdev_ioctl: copy_from_user error.\n");
			ret = -EFAULT;
			goto __error_exit;
		}

		switch(_IOC_NR(cmd)) {
		case _IOC_NR(COMPAT_IOC_MMZ_GET_START_SIZE):
			compat_set_kmmz_start_size(&kmmz_start_size, &compat_mmz_start_size);
			ret = ioctl_mmz_get_start_size(&kmmz_start_size);
			compat_set_mmz_start_size(&compat_mmz_start_size, &kmmz_start_size);
			break;
		default:
			ret = -EINVAL;
			break;
		}

		if (!ret && (cmd & IOC_OUT)) {
			if (copy_to_user((void*)arg, &compat_mmz_start_size, _IOC_SIZE(cmd))) {
				PRINTK_CA("\nmmz_userdev_ioctl: copy_to_user error.\n");
				ret = -EFAULT;
				goto __error_exit;
		    }
		}
	} else {
		ret = -EINVAL;
	}

__error_exit:

	up(&pmu->sem);

	return ret;
}
#endif

int mmz_userdev_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct kmmb_info *p;
	struct mmz_userdev_info *pmu = file->private_data;
	unsigned long offset = (vma->vm_pgoff << PAGE_SHIFT);
	unsigned long i;
	int err;

	p = get_mmbinfo_pbase(offset, pmu);
	if (p == NULL) {
		mmz_error("I'm confused, mmb(0x%016lx) not found?!\n", offset);
		return -EPERM;
	}
	if (p->mapped) {
		mmz_error("I'm confused, mmb(0x%016lx) have been mapped yet?!\n", offset);
		return -EIO;
	}

	if (file->f_flags & O_SYNC) {
		if (p->map_cached) {
			PRINTK_CA("%s->%d,error!: O_SYNC conflict with IOC_MMB_USER_REMAP_CACHED\n", __func__,__LINE__);
			return -EPERM;
		}
		p->map_cached = 0;
		vma->vm_page_prot = mt_mmz_noncached_pgprot(vma->vm_page_prot);
	} else {
		if (p->map_cached) {
			/* cached remap, do nothing */
		} else {
			vma->vm_page_prot = mt_mmz_noncached_pgprot(vma->vm_page_prot);
		}
	}

	p->mapped = (void *)(vma->vm_start + PAGE_SIZE);

	for (i = 0; i < ((vma->vm_end - vma->vm_start - (2 * PAGE_SIZE)) / PAGE_SIZE); i++) {
		/* mmz and isolate_mmz have page */
		err = vm_insert_page(vma,
					vma->vm_start + PAGE_SIZE + (PAGE_SIZE * i),
					pfn_to_page(vma->vm_pgoff + i));
		if (err) {
			pr_err("mmz_userdev_mmap vm_insert_page failed\n");
			return err;
		}
	}

	mtl_mmb_map2user((void *)(vma->vm_start + PAGE_SIZE), vma->vm_pgoff << PAGE_SHIFT, p->map_cached);

	return 0;
}

int mmz_userdev_release(struct inode *inode, struct file *file)
{
	struct mmz_userdev_info *pmu = file->private_data;
	struct kmmb_info *p, *n;

	list_for_each_entry_safe(p, n, &pmu->list, list) {
	    /*
		PRINTK_CA(KERN_ERR "MMB LEAK(pid=%d): 0x%016lx, %lu bytes, '%s'\n", \
				pmu->pid, mtl_mmb_phys(p->mmb), \
				mtl_mmb_length(p->mmb),
				mtl_mmb_name(p->mmb));
		*/
		/* we do not need to release mapped-area here, system will do it for us */
		/*
		if(p->mapped)
			PRINTK_CA(KERN_WARNING "mmz_userdev_release: mmb<0x%016lx> mapped to userspace 0x%p will be force unmaped!\n", (ulong)p->phys_addr, p->mapped);
		*/
		for (; p->mmb_ref > 0; p->mmb_ref--) {
			mtl_mmb_put(p->mmb);
		}
		_usrdev_mmb_free(p);
	}

	file->private_data = NULL;
	kfree(pmu);

	return 0;
}

static struct file_operations mmz_userdev_fops = {
	.owner = THIS_MODULE,
	.open = mmz_userdev_open,
	.release = mmz_userdev_release,
	.unlocked_ioctl = mmz_userdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_mmz_userdev_ioctl,
#endif
	.mmap = mmz_userdev_mmap,
};

static int proc_mmz_open(struct inode *inode, struct file *file)
{
	return single_open(file, mmz_read_proc, pde_data(inode));
}

static struct proc_ops proc_mmz_fops = {
	.proc_open = proc_mmz_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write = NULL,
	.proc_release = single_release,
};

/****************************proc**********************************/
#define MEDIA_MEM_NAME  "media-mem"
static int media_mem_proc_init(void)
{
#if !(0 == MT_PROC_SUPPORT)
	struct proc_dir_entry *p;
	p = proc_create(MEDIA_MEM_NAME, 0444, NULL, &proc_mmz_fops);
	if (p == NULL)
		return -1;
#endif
    return 0;
}


static void media_mem_proc_exit(void)
{
#if !(0 == MT_PROC_SUPPORT)
	remove_proc_entry(MEDIA_MEM_NAME, NULL);
#endif
}

/********************init**************************/

struct mmz_user_device {
	struct device *dev;
	void *platdata;
	dev_t devt;
	int id;
};

struct mmz_user_driver {
	struct cdev cdev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct mmz_user_driver *mmz_user_drv;

static int mmz_user_probe(struct platform_device *pdev)
{
	struct mmz_user_device *mmz_user_dev = NULL;
	int ret = 0;

	mmz_user_dev = kzalloc(sizeof(struct mmz_user_device), GFP_KERNEL);
	if (!mmz_user_dev) {
		pr_err("Error kzalloc mmz_user_dev\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mmz_user_dev;
	}
	mmz_user_dev->id = pdev->id;
	mmz_user_dev->devt = mmz_user_drv->devt + mmz_user_dev->id;
	mmz_user_dev->platdata = dev_get_platdata(&pdev->dev);

#if 0
	mmz_user_dev->dev = device_create(mt_class, &pdev->dev, mmz_user_dev->devt, NULL, UMAP_DEVNAME_MMZ);
#else
	mmz_user_dev->dev = device_create(mt_class, NULL, mmz_user_dev->devt, NULL, UMAP_DEVNAME_MMZ);
#endif
	if (IS_ERR(mmz_user_dev->dev)) {
		pr_err("Error device_create\n\n");
		ret = PTR_ERR(mmz_user_dev->dev);
		goto fail_device_create;
	}

	platform_set_drvdata(pdev, mmz_user_dev);
	dev_set_drvdata(mmz_user_dev->dev, mmz_user_dev);

	return 0;

fail_device_create:
	kfree(mmz_user_dev);
	mmz_user_dev = NULL;
fail_kzalloc_mmz_user_dev:
	return ret;
}

static int mmz_user_remove(struct platform_device *pdev)
{
	struct mmz_user_device *mmz_user_dev = platform_get_drvdata(pdev);

	device_destroy(mt_class, mmz_user_dev->devt);
	mmz_user_dev->dev = NULL;

	platform_set_drvdata(pdev, NULL);

	if (mmz_user_dev) {
		kfree(mmz_user_dev);
		mmz_user_dev = NULL;
	}

	return 0;
}

static int mmz_user_suspend(struct platform_device *pdev, pm_message_t stState)
{
	return 0;
}

static int mmz_user_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver mmz_user_platform_driver = {
	.probe = mmz_user_probe,
	.remove = mmz_user_remove,
	.suspend = mmz_user_suspend,
	.resume = mmz_user_resume,
	.driver = {
		.name = UMAP_DEVNAME_MMZ,
		.owner = THIS_MODULE,
	}
};

static void mmz_user_release_device(struct device *pdev) {  }

static struct platform_device mmz_user_platform_device = {
	.name = UMAP_DEVNAME_MMZ,
	.id = 0,
	.dev= {
		.platform_data = NULL,
		.release = mmz_user_release_device,
	},
};

int __init drv_mmz_modinit(void)
{
	int ret;

	MT_INFO_LOG("===drv_mmz_modinit===00===\n");
    mt_drv_mmz_init();
	MT_INFO_LOG("===drv_mmz_modinit===11===\n");
    media_mem_proc_init();
	MT_INFO_LOG("===drv_mmz_modinit===22===\n");

	mmz_user_drv = kzalloc(sizeof(struct mmz_user_driver), GFP_KERNEL);
	if (!mmz_user_drv) {
		pr_err("Error kzalloc mmz_user_drv\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mmz_user_drv;
	}

	mmz_user_drv->major = MT_DEVICE_MAJOR;
	mmz_user_drv->minor = UMAP_MIN_MINOR_MMZ;
	mmz_user_drv->minors = UMAP_DEV_NUM_MMZ;
	mmz_user_drv->devt = MKDEV(mmz_user_drv->major, mmz_user_drv->minor);
	cdev_init(&mmz_user_drv->cdev, &mmz_userdev_fops);
	mmz_user_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&mmz_user_drv->cdev, mmz_user_drv->devt, mmz_user_drv->minors);
	if (ret) {
		pr_err("Error cdev_add\n\n");
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&mmz_user_platform_driver);
	if (ret) {
		pr_err("Error platform_driver_register\n\n");
		goto fail_platform_driver_register;
	}

	ret = platform_device_register(&mmz_user_platform_device);
	if (ret) {
		pr_err("Error platform_device_register\n\n");
		goto fail_platform_device_register;
	}

	MT_INFO_LOG("===drv_mmz_modinit===33===\n");
	return 0;

fail_platform_device_register:
	platform_driver_unregister(&mmz_user_platform_driver);
fail_platform_driver_register:
	cdev_del(&mmz_user_drv->cdev);
fail_cdev_add:
	kfree(mmz_user_drv);
	mmz_user_drv = NULL;
fail_kzalloc_mmz_user_drv:
    media_mem_proc_exit();
    mt_drv_mmz_exit();
	return ret;
}

void drv_mmz_modexit(void)
{
	platform_driver_unregister(&mmz_user_platform_driver);
	platform_device_unregister(&mmz_user_platform_device);
	kfree(mmz_user_drv);
	mmz_user_drv = NULL;

	media_mem_proc_exit();
	mt_drv_mmz_exit();
}

