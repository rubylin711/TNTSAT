/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MEDIA_MEM_INTF_H__
#define __MEDIA_MEM_INTF_H__

typedef void kcom_mmz_t;

#define MMB_ADDR_INVALID (~0UL)

phys_addr_t new_mmb(const char *name, ulong size, unsigned long align, const char *zone_name,
					struct mt_drv_mmz_security_attr_s *attr);
void delete_mmb(phys_addr_t addr);

void *remap_mmb(phys_addr_t addr);
void *remap_mmb_cached(phys_addr_t addr);
void *refer_mapped_mmb(void *mapped_addr);
int unmap_mmb(void *mapped_addr);

int	get_mmb(phys_addr_t addr);
int	put_mmb(phys_addr_t addr);

void *remap_mmb_2(phys_addr_t phyaddr, ulong *VBaddr, phys_addr_t *Outoffset);
int unmap_mmb_2(void *mapped_addr, unsigned long offset);

#endif

