/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef CONFIG_DMA_SHARED_BUFFER
#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/highmem.h>
#include <linux/memblock.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include "mtfb_comm.h"

MODULE_IMPORT_NS(DMA_BUF);

struct mtfb_memblock_pdata {
	phys_addr_t base;
};

static unsigned long get_page_count( size_t length)
{
    unsigned long count = 0;
    count = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    return count;
}


static struct sg_table *mtfb_memblock_map(struct dma_buf_attachment *attach,
		enum dma_data_direction direction)
{
	struct mtfb_memblock_pdata *pdata = attach->dmabuf->priv;
	struct sg_table *table;
	int ret;
        size_t nr_pages = 0;
       table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret < 0)
		goto err;
    nr_pages = get_page_count(attach->dmabuf->size);
    sg_dma_len(table->sgl) = nr_pages* PAGE_SIZE;
    sg_set_page(table->sgl, pfn_to_page(PFN_DOWN(pdata->base)), nr_pages* PAGE_SIZE, 0);
    sg_dma_address(table->sgl) = pdata->base;
	return table;

err:
	kfree(table);
	return ERR_PTR(ret);
}

static void mtfb_memblock_unmap(struct dma_buf_attachment *attach,
		struct sg_table *table, enum dma_data_direction direction)
{
	sg_free_table(table);
	kfree(table);
}

static void __init_memblock mtfb_memblock_release(struct dma_buf *buf)
{
	struct mtfb_memblock_pdata *pdata = buf->priv;
	/*int err = memblock_free(pdata->base, buf->size);*/
	kfree(pdata);
	buf->priv = NULL;
}
#if 0

static void *mtfb_memblock_do_kmap(struct dma_buf *buf, unsigned long pgoffset,
		bool atomic)
{
	struct mtfb_memblock_pdata *pdata = buf->priv;
	unsigned long pfn = PFN_DOWN(pdata->base) + pgoffset;
	struct page *page = pfn_to_page(pfn);

	if (atomic)
		return kmap_atomic(page);
	else
		return kmap(page);
}

static void *mtfb_memblock_kmap_atomic(struct dma_buf *buf,
		unsigned long pgoffset)
{
	return mtfb_memblock_do_kmap(buf, pgoffset, true);
}

static void mtfb_memblock_kunmap_atomic(struct dma_buf *buf,
		unsigned long pgoffset, void *vaddr)
{
	kunmap_atomic(vaddr);
}

static void *mtfb_memblock_kmap(struct dma_buf *buf, unsigned long pgoffset)
{
	return mtfb_memblock_do_kmap(buf, pgoffset, false);
}

static void mtfb_memblock_kunmap(struct dma_buf *buf, unsigned long pgoffset,
		void *vaddr)
{
	kunmap(vaddr);
}
#endif

static int mtfb_memblock_mmap(struct dma_buf *buf, struct vm_area_struct *vma)
{
	struct mtfb_memblock_pdata *pdata = buf->priv;

	vma->vm_page_prot =  pgprot_writecombine(vma->vm_page_prot);
	return remap_pfn_range(vma, vma->vm_start, PFN_DOWN(pdata->base),
			vma->vm_end - vma->vm_start, vma->vm_page_prot);
}


struct dma_buf_ops mtfb_memblock_ops = {
	.map_dma_buf = mtfb_memblock_map,
	.unmap_dma_buf = mtfb_memblock_unmap,
	.release = mtfb_memblock_release,
	//.map_atomic = mtfb_memblock_kmap_atomic,
	//.unmap_atomic = mtfb_memblock_kunmap_atomic,        //sym6
	//.map = mtfb_memblock_kmap,
	//.unmap = mtfb_memblock_kunmap,
	.mmap = mtfb_memblock_mmap,
};

/**
 * mtfb_memblock_export - export a memblock reserved area as a dma-buf
 *
 * @base: base physical address
 * @size: memblock size
 * @flags: mode flags for the dma-buf's file
 *
 * @base and @size must be page-aligned.
 *
 * Returns a dma-buf on success or ERR_PTR(-errno) on failure.
 */
struct dma_buf *mtfb_memblock_export(phys_addr_t base, size_t size, int flags)
{
	struct mtfb_memblock_pdata *pdata;
	struct dma_buf *buf;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
#endif

	if (PAGE_ALIGN(base) != base || PAGE_ALIGN(size) != size)
		return ERR_PTR(-EINVAL);

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);

	if (!pdata)
		return ERR_PTR(-ENOMEM);

	pdata->base = base;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
	exp_info.ops = &mtfb_memblock_ops;
	exp_info.size = size;
	exp_info.flags = flags;
	exp_info.priv = pdata;
	buf = dma_buf_export(&exp_info);
#else
	buf = dma_buf_export(pdata, &mtfb_memblock_ops, size, flags, NULL);
#endif
	if (IS_ERR(buf))
		kfree(pdata);

	return buf;
}
#endif

