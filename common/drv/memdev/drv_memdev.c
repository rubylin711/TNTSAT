/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/******************************* Include Files *******************************/

/* Sys headers */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/pagewalk.h>
#include <linux/rmap.h>
#include <linux/hw_breakpoint.h>
#include <linux/mt_mmz.h>
#include <asm/pgtable.h>
#include "mt_cache.h"

/* Unf headers */

/* Drv headers */

/* Local headers */
#include "mt_drv_module.h"
#include "mt_drv_memdev.h"
#include "mt_drv_mmz.h"
#include "drv_media_mem.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/


/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/

static mt_s32 memdev_drv_open(struct inode *inode, struct file *file);
static mt_s32 memdev_drv_close(struct inode *inode, struct file *file);
static mt_s32 memdev_drv_mmap(struct file *file, struct vm_area_struct *vm);
static long memdev_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#ifdef CONFIG_COMPAT
static long compat_memdev_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif

static bool walk_watch_once = false;
static ulong mt_user_vaddr;
ulong mt_check_molva = 0;	/* memory overflow linear virt addr */
u64 mt_check_molva_rv;		/* right value should be */
u32 mt_check_molva_sz;

static struct file_operations g_stMemdevFops =
{
	.open = memdev_drv_open,
	.release = memdev_drv_close,
	.mmap = memdev_drv_mmap,
	.unlocked_ioctl = memdev_drv_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_memdev_drv_ioctl,
#endif
};

static struct miscdevice g_stMemdevDevice =
{
    MISC_DYNAMIC_MINOR,
    UMAP_DEVNAME_MEMDEV,
    &g_stMemdevFops
};

#ifdef CONFIG_MT_DETECT_LINEAR_OVERFLOW_WATCHPOINT
static struct perf_event * __percpu *mt_hbp1;
static struct perf_event *mt_hbp2;

static void mt_hbp_handler1(struct perf_event *bp,
			       struct perf_sample_data *data,
			       struct pt_regs *regs)
{
	printk(KERN_INFO "0x%lx value is changed\n", mt_check_molva);
	dump_stack();
	printk(KERN_INFO "Dump stack from mt_hbp_handler1\n");
}

static void mt_hbp_handler2(struct perf_event *bp,
			       struct perf_sample_data *data,
			       struct pt_regs *regs)
{
	printk(KERN_INFO "0x%lx value is changed\n", mt_user_vaddr);
	dump_stack();
	printk(KERN_INFO "Dump stack from mt_hbp_handler2\n");
}
#endif

/* copy from arch/arm/mm/mmu.c */
/*
* we hope use writecombine to improve performance,
* but default phys_mem_access_prot in drivers/char/mem.c no support writecombine.
* arm/arm64's pfn_valid false on no-map memory in old kernel:
* no-map have page and SECTION_IS_EARLY, early_section true, so will check memblock_is_map_memory.
* back porting pfn_valid from new kernel, the no-map's pfn_valid is true
*/
static pgprot_t mt_phys_mem_access_prot(struct file *file, unsigned long pfn,
						pgprot_t vma_prot, bool *is_ddr)
{
	*is_ddr = true;
	if (!pfn_valid(pfn)) {	/* mmz and isolate_mmz pfn_valid must be true, only device pfn_valid false */
		*is_ddr = false;
		if (in_isolate_mmz(pfn_to_page(pfn))) {
			panic("pfn %lx is isolate_mmz, but pfn_valid false\n", pfn);
		} else if (in_mmz(pfn_to_page(pfn))) {
			panic("pfn %lx is mmz, but pfn_valid false\n", pfn);
		} else {
			if (file->f_flags & O_SYNC) {
				return pgprot_noncached(vma_prot);
			}
		}
	} else if (file->f_flags & O_SYNC) {
		return pgprot_writecombine(vma_prot);
	}
	return vma_prot;
}

/*********************************** Code ************************************/


bool mt_rmap_one(struct folio *folio, struct vm_area_struct *vma, unsigned long addr, void *arg)
{
	pr_info("mt_rmap_one		pfn: 0x%lx, user_vaddr = 0x%lx\n", folio_pfn(folio), addr);
#ifdef CONFIG_MEMCG
	pr_info("mt_rmap_one		tgid: %d, comm: %s\n", vma->vm_mm->owner->tgid, vma->vm_mm->owner->comm);
#endif
	return true;
}

static int mt_watch_pte_entry(pte_t *pte, unsigned long addr, unsigned long next, struct mm_walk *walk)
{
	ulong pfn = pte_pfn(*pte);
	ulong linear_vaddr_align = (ulong)(__va((pfn) << PAGE_SHIFT));
	ulong linear_vaddr = ((linear_vaddr_align & PAGE_MASK) | (addr & ~PAGE_MASK));
	struct folio *folio = pfn_folio(pfn);
	struct rmap_walk_control rwc = {
		.rmap_one = mt_rmap_one,
	};
#ifdef CONFIG_MT_DETECT_LINEAR_OVERFLOW_WATCHPOINT
	int ret;
	struct perf_event_attr attr1;
	struct perf_event_attr attr2;
#endif

	pr_info("pfn: 0x%lx, linear_vaddr_align: 0x%lx, linear_vaddr: 0x%lx, user_vaddr: 0x%lx\n", pfn, linear_vaddr_align, linear_vaddr, addr);
	mt_check_molva = linear_vaddr;
	pr_info("mt_check_molva = 0x%lx, mt_check_molva_rv = 0x%llx, mt_check_molva_sz = %d\n", mt_check_molva, mt_check_molva_rv, mt_check_molva_sz);

#ifdef CONFIG_MT_DETECT_LINEAR_OVERFLOW_WATCHPOINT
	hw_breakpoint_init(&attr1);
	attr1.bp_addr = mt_check_molva;
	if (mt_check_molva_sz == sizeof(u64)) {
		attr1.bp_len = HW_BREAKPOINT_LEN_8;
	} else if (mt_check_molva_sz == sizeof(u32)) {
		attr1.bp_len = HW_BREAKPOINT_LEN_4;
	} else if (mt_check_molva_sz == sizeof(u16)) {
		attr1.bp_len = HW_BREAKPOINT_LEN_2;
	} else {
		attr1.bp_len = HW_BREAKPOINT_LEN_1;
	}
	attr1.bp_type = HW_BREAKPOINT_W;
	mt_hbp1 = register_wide_hw_breakpoint(&attr1, mt_hbp_handler1, NULL);
	if (IS_ERR((void __force *)mt_hbp1)) {
		ret = PTR_ERR((void __force *)mt_hbp1);
		pr_info("Watchpoint mt_hbp1 registration failed: %d\n", ret);
		return ret;
	}
	pr_info("Watchpoint mt_hbp1 registration: 0x%llx\n", attr1.bp_addr);

	hw_breakpoint_init(&attr2);
	attr2.bp_addr = addr;
	if (mt_check_molva_sz == sizeof(u64)) {
		attr2.bp_len = HW_BREAKPOINT_LEN_8;
	} else if (mt_check_molva_sz == sizeof(u32)) {
		attr2.bp_len = HW_BREAKPOINT_LEN_4;
	} else if (mt_check_molva_sz == sizeof(u16)) {
		attr2.bp_len = HW_BREAKPOINT_LEN_2;
	} else {
		attr2.bp_len = HW_BREAKPOINT_LEN_1;
	}
	attr2.bp_type = HW_BREAKPOINT_W;
	mt_hbp2 = register_user_hw_breakpoint(&attr2, mt_hbp_handler2, NULL, current);
	if (IS_ERR((void __force *)mt_hbp2)) {
		ret = PTR_ERR((void __force *)mt_hbp2);
		pr_info("Watchpoint mt_hbp2 registration failed: %d\n", ret);
		return ret;
	}
	pr_info("Watchpoint mt_hbp2 registration: 0x%llx\n", attr2.bp_addr);
#endif

	rmap_walk(folio, &rwc);
	return 0;
}

static int mt_get_pa_pte_entry(pte_t *pte, unsigned long addr, unsigned long next, struct mm_walk *walk)
{
	ulong pfn = pte_pfn(*pte);

	*((phys_addr_t *)(walk->private)) = __pfn_to_phys(pfn) + (addr & ~PAGE_MASK);
	return 0;
}

static int mt_test_walk(unsigned long addr, unsigned long next, struct mm_walk *walk)
{
	return 0;
}

static mt_s32 memdev_drv_open(struct inode *inode, struct file *file)
{
    MT_INFO_MEMDEV("Open mt_mem:%s,%d\n", current->comm, current->pid);
    return MT_SUCCESS;
}

static mt_s32 memdev_drv_close(struct inode *inode, struct file *file)
{
    MT_INFO_MEMDEV("Close mt_mem:%s,%d\n", current->comm, current->pid);
    return MT_SUCCESS;
}

static mt_s32 memdev_drv_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long i;
	int err;
	bool is_ddr;

	vma->vm_page_prot = mt_phys_mem_access_prot(file, vma->vm_pgoff, vma->vm_page_prot, &is_ddr);

	if (is_ddr) {
		for (i = 0; i < ((vma->vm_end - vma->vm_start - (2 * PAGE_SIZE)) / PAGE_SIZE); i++) {
			/* mmz and isolate_mmz have page */
			err = vm_insert_page(vma,
						vma->vm_start + PAGE_SIZE + (PAGE_SIZE * i),
						pfn_to_page(vma->vm_pgoff + i));
			if (err) {
				pr_err("memdev_drv_mmap ddr vm_insert_page failed\n");
				return err;
			}
		}
	} else {
		if (remap_pfn_range(vma,
							vma->vm_start + PAGE_SIZE,
							vma->vm_pgoff,
							vma->vm_end - vma->vm_start - (2 * PAGE_SIZE),
							vma->vm_page_prot)) {
			pr_err("memdev_drv_mmap dev remap_pfn_range fail.\n");
			return -EAGAIN;
		}
	}

	mtl_mmb_map2user((void *)(vma->vm_start + PAGE_SIZE), vma->vm_pgoff << PAGE_SHIFT, !(file->f_flags & O_SYNC));

	return MT_SUCCESS;
}

static long memdev_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct cache_op_mem_area coa;
	struct get_mem_pageinfo info;
	struct mm_walk_ops mt_walk_ops = {
		.test_walk = mt_test_walk,
	};
	long ret = 0;
	struct page *pages[2];
	struct walk_watch ww;
	struct walk_get_phys_addr get_pa;
	struct mm_struct *mm;

	switch (cmd) {
	case MT_MEM_FLUSH_DCACHE:
	case MT_MEM_INV_DCACHE:
		if (copy_from_user(&coa, (void *)arg, sizeof(coa))) {
	        PRINTK_CA("\nmemdev_drv_ioctl: copy_from_user error.\n");
	        ret = -EFAULT;
	        goto __error_exit;
		}
		if (cmd == MT_MEM_FLUSH_DCACHE) {
			if (coa.virtaddr == NULL) {
				mt_dcache_flush_all();
			} else {
				mt_user_dcache_flush(coa.virtaddr, 0, coa.size);
			}
		} else {
			mt_user_dcache_invalid(coa.virtaddr, 0, coa.size);
		}
		break;
	case MT_MEM_GET_PAGEINFO:
		info.page_block_order = (mt_u32)pageblock_order;
		info.pages_per_block = (mt_u32)pageblock_nr_pages;
		if (copy_to_user((struct get_mem_pageinfo *)((void *)arg), &info, sizeof(struct get_mem_pageinfo))) {
			ret = -EFAULT;
			goto __error_exit;
		}
		break;
	case MT_MEM_WALK_WATCH:
		if (!walk_watch_once) {
			mt_walk_ops.pte_entry = mt_watch_pte_entry;
			walk_watch_once = true;
			if (copy_from_user(&ww, (void *)arg, sizeof(ww))) {
				ret = -EFAULT;
			}
			if (ww.size <= sizeof(u64)) {
				mt_check_molva_sz = ww.size;
				mt_check_molva_rv = ww.right_value;
				mt_user_vaddr = ww.user_vaddr;
				get_user_pages_fast(ww.user_vaddr, ARRAY_SIZE(pages), 1, pages);
				mm = current->mm;
				mmap_read_lock(mm);
				walk_page_range(current->mm, ww.user_vaddr, ww.user_vaddr + ww.size, &mt_walk_ops, NULL);
				mmap_read_unlock(mm);
			}
		}
		break;
	case MT_MEM_WALK_GET_PHYS_ADDR:
		mt_walk_ops.pte_entry = mt_get_pa_pte_entry;
		if (copy_from_user(&get_pa, (void *)arg, sizeof(get_pa))) {
			ret = -EFAULT;
		}
		mm = current->mm;
		mmap_read_lock(mm);
		walk_page_range(current->mm, get_pa.user_vaddr, get_pa.user_vaddr + 1, &mt_walk_ops, &get_pa.phys_addr);
		mmap_read_unlock(mm);
		if (copy_to_user((void *)arg, &get_pa, sizeof(get_pa))) {
			ret = -EFAULT;
		}
		break;
	default:
		break;
	}

__error_exit:
	return ret;
}

#ifdef CONFIG_COMPAT
static long compat_memdev_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct compat_cache_op_mem_area coa;
	struct compat_get_mem_pageinfo info;
	struct mm_walk_ops mt_walk_ops = {
		.test_walk = mt_test_walk,
	};
	long ret = 0;
	struct page *pages[2];
	struct walk_watch ww;
	struct walk_get_phys_addr get_pa;
	struct compat_walk_watch compat_ww;
	struct compat_walk_get_phys_addr compat_get_pa;
	struct mm_struct *mm;

	switch (cmd) {
	case COMPAT_MT_MEM_FLUSH_DCACHE:
	case COMPAT_MT_MEM_INV_DCACHE:
		if (copy_from_user(&coa, (void *)arg, sizeof(coa))) {
	        PRINTK_CA("\nmemdev_drv_ioctl: copy_from_user error.\n");
	        ret = -EFAULT;
	        goto __error_exit;
		}
		if (cmd == COMPAT_MT_MEM_FLUSH_DCACHE) {
			if ((void *)((ulong)coa.virtaddr) == NULL) {
				mt_dcache_flush_all();
			} else {
				mt_user_dcache_flush((void *)((ulong)coa.virtaddr), 0, (ulong)coa.size);
			}
		} else {
			mt_user_dcache_invalid((void *)((ulong)coa.virtaddr), 0, (ulong)coa.size);
		}
		break;
	case COMPAT_MT_MEM_GET_PAGEINFO:
		info.page_block_order = (mt_u32)pageblock_order;
		info.pages_per_block = (mt_u32)pageblock_nr_pages;
		if (copy_to_user((struct compat_get_mem_pageinfo *)((void *)arg), &info, sizeof(struct compat_get_mem_pageinfo))) {
			ret = -EFAULT;
			goto __error_exit;
		}
		break;
	case COMPAT_MT_MEM_WALK_WATCH:
		if (!walk_watch_once) {
			mt_walk_ops.pte_entry = mt_watch_pte_entry;
			walk_watch_once = true;
			if (copy_from_user(&compat_ww, (void *)arg, sizeof(compat_ww))) {
				ret = -EFAULT;
			}
			ww.user_vaddr = compat_ww.user_vaddr;
			ww.right_value = compat_ww.right_value;
			ww.size = compat_ww.size;
			if (ww.size <= sizeof(u64)) {
				mt_check_molva_sz = ww.size;
				mt_check_molva_rv = ww.right_value;
				mt_user_vaddr = ww.user_vaddr;
				get_user_pages_fast(ww.user_vaddr, ARRAY_SIZE(pages), 1, pages);
				mm = current->mm;
				mmap_read_lock(mm);
				walk_page_range(current->mm, ww.user_vaddr, ww.user_vaddr + ww.size, &mt_walk_ops, NULL);
				mmap_read_unlock(mm);
			}
		}
		break;
	case COMPAT_MT_MEM_WALK_GET_PHYS_ADDR:
		mt_walk_ops.pte_entry = mt_get_pa_pte_entry;
		if (copy_from_user(&compat_get_pa, (void *)arg, sizeof(compat_get_pa))) {
			ret = -EFAULT;
		}
		get_pa.user_vaddr = compat_get_pa.user_vaddr;
		get_pa.phys_addr = compat_get_pa.phys_addr;
		mm = current->mm;
		mmap_read_lock(mm);
		walk_page_range(current->mm, get_pa.user_vaddr, get_pa.user_vaddr + 1, &mt_walk_ops, &get_pa.phys_addr);
		mmap_read_unlock(mm);
		compat_get_pa.phys_addr = get_pa.phys_addr;
		if (copy_to_user((void *)arg, &compat_get_pa, sizeof(compat_get_pa))) {
			ret = -EFAULT;
		}
		break;
	default:
		break;
	}

__error_exit:
	return ret;
}
#endif

mt_s32 memdev_drv_modinit(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = mt_drv_module_register(MT_ID_MEMDEV, "MT_MEMDEV", MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        MT_FATAL_MEMDEV("register memdev modules failed.\n");
        goto out1;
    }

    s32Ret = misc_register(&g_stMemdevDevice);
    if (s32Ret)
    {
        MT_FATAL_MEMDEV("mt_memdevice_dev device register failed\n");
        goto out2;
    }

    return MT_SUCCESS;

out2:
    mt_drv_module_unregister(MT_ID_MEMDEV);
out1:
    return s32Ret;
}

mt_s32 memdev_drv_modexit(mt_void)
{
    misc_deregister(&g_stMemdevDevice);
    mt_drv_module_unregister(MT_ID_MEMDEV);
    return MT_SUCCESS;
}

MODULE_AUTHOR("Montage");
MODULE_DESCRIPTION("Montage Mem Device Driver");
MODULE_LICENSE("GPL");

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
