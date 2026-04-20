/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#include <asm/cache.h>
#include <asm/cacheflush.h>
#include <linux/smp.h>
#include "mt_drv_mmz.h"
#include "drv_mmz.h"
#include "drv_media_mem.h"
#include "drv_mem_ioctl.h"
#include "mt_module_debug.h"
#include "mt_cache.h"

/*
* for CA solution, if "ddr" mmz zone memory not enough,
* should NOT allocate from other "av" mmz zone!
*/
#ifdef CONFIG_MT_DONT_STEAL_AV_MMZ
int dont_steal_av = 1;
#else
int dont_steal_av = 0;
#endif

mt_s32 mt_drv_mmz_alloc_and_map(const char *bufname, char *zone_name, ulong size, int align, mmz_buffer_s *psMBuf)
{
	phys_addr_t phyaddr;
	char *mmz_name =  zone_name;

	BUG_ON(bufname == NULL);
	BUG_ON(size <= 0);
	BUG_ON(align < 0);
	BUG_ON(psMBuf == NULL);

	phyaddr = new_mmb(bufname, size, align, mmz_name, NULL);
	if (phyaddr == MMB_ADDR_INVALID)
	{
		if ((dont_steal_av && (zone_name != NULL) && (strncmp(zone_name, MMZ_ZONE_DDR, MTL_MMZ_NAME_LEN))) ||
			((zone_name == NULL) || (!strncmp(zone_name, MMZ_ZONE_DDR, MTL_MMZ_NAME_LEN))))
		{
			//dont_steal_av, av/audio zone, no retry
			psMBuf->size =  0;
			psMBuf->startPhyAddr = MMB_ADDR_INVALID;
			psMBuf->startVirAddr = NULL;

			MT_ERR_LOG("Alloc %s failed! \n", bufname);
			dump_mmz_mem();
			return MT_FAILURE;
		}
		else
		{
			//mmz_name = MMZ_OTHERS;
			/*alloc once again from others buffer*/
			/*CNcomment:再从others 缓冲区分配一次*/
			phyaddr = new_mmb(bufname, size, align, MMZ_OTHERS, NULL);
			if (phyaddr == MMB_ADDR_INVALID)
			{
				psMBuf->size =  0;
				psMBuf->startPhyAddr = MMB_ADDR_INVALID;
				psMBuf->startVirAddr = NULL;

				MT_ERR_LOG("Alloc %s failed! \n", bufname);
				dump_mmz_mem();
				return MT_FAILURE;
			}
		}
	}
	psMBuf->size = size;

	psMBuf->startVirAddr = remap_mmb(phyaddr);
	if (MT_NULL == psMBuf->startVirAddr)
	{
		delete_mmb(phyaddr);
		psMBuf->size =  0;
		psMBuf->startPhyAddr = MMB_ADDR_INVALID;
		psMBuf->startVirAddr = NULL;

		MT_ERR_LOG("Remap %s failed! \n", bufname);

		return MT_FAILURE;
	}
	psMBuf->startPhyAddr = phyaddr;

	return MT_SUCCESS;
}

mt_void mt_drv_mmz_unmap_and_release(mmz_buffer_s *psMBuf)
{
	phys_addr_t phyaddr;

	BUG_ON(psMBuf == NULL);

	phyaddr = psMBuf->startPhyAddr;

	if (MMB_ADDR_INVALID != phyaddr && 0 != phyaddr)
	{
		if (psMBuf->startVirAddr != NULL)
		{
			unmap_mmb(psMBuf->startVirAddr);

			//TODO?
			psMBuf->startVirAddr = NULL;
		}
		delete_mmb(phyaddr);

		//TODO?
		psMBuf->startPhyAddr = MMB_ADDR_INVALID;
		psMBuf->size = 0;
	}
}

mt_s32 mt_drv_mmz_alloc(const char *bufname, char *zone_name, ulong size, int align, mmz_buffer_s *psMBuf)
{
	return mt_drv_mmz_alloc_secure(bufname, zone_name, size, align, psMBuf, NULL);
}

mt_s32 mt_drv_mmz_alloc_secure(const char *bufname, char *zone_name, ulong size, int align, mmz_buffer_s *psMBuf,
												mmz_security_attr_s *attr)
{
	phys_addr_t phyaddr;
	char *mmz_name = zone_name;

	BUG_ON(bufname == NULL);
	BUG_ON(size <= 0);
	BUG_ON(align < 0);
	BUG_ON(psMBuf == NULL);

	phyaddr = new_mmb(bufname, size, align, mmz_name, attr);
	if (phyaddr == MMB_ADDR_INVALID)
	{
		if ((dont_steal_av && (zone_name != NULL) && (strncmp(zone_name, MMZ_ZONE_DDR, MTL_MMZ_NAME_LEN))) ||
			((zone_name == NULL) || (!strncmp(zone_name, MMZ_ZONE_DDR, MTL_MMZ_NAME_LEN))))
		{
			//dont_steal_av, av/audio zone, no retry
			psMBuf->size = 0;
			psMBuf->startPhyAddr = MMB_ADDR_INVALID;
			psMBuf->startVirAddr = NULL;

			MT_ERR_LOG("Alloc %s failed! \n", bufname);
			dump_mmz_mem();
			return MT_FAILURE;
		}
		else
		{
			//mmz_name = MMZ_OTHERS;
			/*alloc once again from others buffer*/
			/*CNcomment:再从others 缓冲区分配一次*/
			phyaddr = new_mmb(bufname, size, align, MMZ_OTHERS, attr);
			if (phyaddr == MMB_ADDR_INVALID)
			{
				psMBuf->size = 0;
				psMBuf->startPhyAddr = MMB_ADDR_INVALID;
				psMBuf->startVirAddr = NULL;

				MT_ERR_LOG("Alloc %s failed! \n", bufname);
				dump_mmz_mem();
				return MT_FAILURE;
			}
		}
	}

	psMBuf->size = size;

	psMBuf->startPhyAddr = phyaddr;
	psMBuf->startVirAddr = NULL;
	return MT_SUCCESS;
}

mt_s32 mt_drv_mmz_map_cache(mmz_buffer_s *psMBuf)
{
	phys_addr_t phyaddr;

	BUG_ON(psMBuf == NULL);

	phyaddr = psMBuf->startPhyAddr;
	BUG_ON(phyaddr == 0);
	BUG_ON(phyaddr == MMB_ADDR_INVALID);

	psMBuf->startVirAddr = remap_mmb_cached(phyaddr);
	if (NULL == psMBuf->startVirAddr)
	{
		MT_ERR_LOG("Remap buf(0x%lx) failed! \n", phyaddr);

		return MT_FAILURE;
	}
	return MT_SUCCESS;
}

mt_s32 mt_drv_mmz_flush(mmz_buffer_s *psMBuf)
{
	void *viraddr;
	ulong len;

	BUG_ON(psMBuf == NULL);

	viraddr = psMBuf->startVirAddr;
	len = psMBuf->size;

	BUG_ON(viraddr == NULL);
	BUG_ON(len <= 0);

	mt_dcache_flush(viraddr, len);

	return MT_SUCCESS;
}

mt_s32 mt_drv_mmz_invalid(mmz_buffer_s *psMBuf)
{
	void * viraddr;
	ulong len;

	BUG_ON(psMBuf == NULL);

	viraddr = psMBuf->startVirAddr;
	len = psMBuf->size;

	BUG_ON(viraddr == NULL);
	BUG_ON(len <= 0);

	mt_dcache_invalid(viraddr, len);

	return MT_SUCCESS;
}

mt_s32 mt_drv_mmz_map(mmz_buffer_s *psMBuf)
{
	phys_addr_t phyaddr;

	BUG_ON(psMBuf == NULL);

	phyaddr = psMBuf->startPhyAddr;
	BUG_ON(phyaddr == 0);
	BUG_ON(phyaddr == MMB_ADDR_INVALID);

	psMBuf->startVirAddr = remap_mmb(phyaddr);
	if (NULL == psMBuf->startVirAddr)
	{
		MT_ERR_LOG("Remap buf(0x%lx) failed! \n", phyaddr);

		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

mt_void mt_drv_mmz_unmap(mmz_buffer_s *psMBuf)
{
	BUG_ON(psMBuf == NULL);

	if (psMBuf->startVirAddr != NULL)
	{
		unmap_mmb(psMBuf->startVirAddr);

		//TODO?
		psMBuf->startVirAddr = NULL;
	}
}

mt_void mt_drv_mmz_release(mmz_buffer_s *psMBuf)
{
	phys_addr_t phyaddr;

	BUG_ON(psMBuf == NULL);

	phyaddr = psMBuf->startPhyAddr;

	if (MMB_ADDR_INVALID != phyaddr && phyaddr != 0)
	{
		//FIXME: delete_mmb might failed!
		delete_mmb(phyaddr);

		//TODO?
		psMBuf->startPhyAddr = MMB_ADDR_INVALID;
		psMBuf->size = 0;
	}
}

void *mmz_va(mmz_buffer_s *psMBuf, phys_addr_t pa)
{
	if ((pa < psMBuf->startPhyAddr) || (pa >= psMBuf->startPhyAddr + psMBuf->size)) {
		BUG_ON(1);
	}
	return ((pa - psMBuf->startPhyAddr) + psMBuf->startVirAddr);
}

phys_addr_t mmz_pa(mmz_buffer_s *psMBuf, void *va)
{
	if ((va < psMBuf->startVirAddr) || (va >= psMBuf->startVirAddr + psMBuf->size)) {
		BUG_ON(1);
	}
	return ((va - psMBuf->startVirAddr) + psMBuf->startPhyAddr);
}

EXPORT_SYMBOL(mt_drv_mmz_alloc_and_map);
EXPORT_SYMBOL(mt_drv_mmz_unmap_and_release);
EXPORT_SYMBOL(mt_drv_mmz_alloc);
EXPORT_SYMBOL(mt_drv_mmz_alloc_secure);
EXPORT_SYMBOL(mt_drv_mmz_map_cache);
EXPORT_SYMBOL(mt_drv_mmz_flush);
EXPORT_SYMBOL(mt_drv_mmz_map);
EXPORT_SYMBOL(mt_drv_mmz_unmap);
EXPORT_SYMBOL(mt_drv_mmz_release);
EXPORT_SYMBOL(mmz_va);
EXPORT_SYMBOL(mmz_pa);
