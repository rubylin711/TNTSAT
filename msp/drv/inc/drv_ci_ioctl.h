/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _CI_DRV_H
#define _CI_DRV_H

#include "mt_type.h"
#include <linux/ioctl.h>

struct mtci_memrw
{
	u32  addr;				/* Attribute memory address */
	u32  size;				/* Number of bytes of buffer */
	u8  *buffer;				/* Pointer of the data buffer */
	u32 rsize;				/* Number of bytes actually in data buffer */
};

struct mtci_iorw
{
	u32  reg;				/* CI register */
	u8  *buffer;				/* CI register value */
};

struct mtci_pid_ram
{
	u32 index;
	u32 val;
};





#define CI_IOC_BASE 'i'

#define CI_IOC_CIRESET		_IO(CI_IOC_BASE,0)
#define CI_IOC_MEMREAD		_IOR(CI_IOC_BASE,1,struct mtci_memrw)
#define CI_IOC_MEMWRITE		_IOW(CI_IOC_BASE,2,struct mtci_memrw)
#define CI_IOC_IOREAD		_IOR(CI_IOC_BASE,3,struct mtci_iorw)
#define CI_IOC_IOWRITE		_IOW(CI_IOC_BASE,4,struct mtci_iorw)
#define CI_IOC_CAMSTATUS	_IOR(CI_IOC_BASE,5,unsigned int)
#define CI_IOC_TSENABLE		_IOR(CI_IOC_BASE,6,unsigned int)
#define CI_IOC_IOMEMSWITCH	_IOW(CI_IOC_BASE,7,unsigned int)
#define CI_IOC_TS0CTRL		_IOW(CI_IOC_BASE,8,unsigned int)
#define CI_IOC_IRQCTL		_IOW(CI_IOC_BASE,9,unsigned int)
#define CI_IOC_TSINCLK		_IOW(CI_IOC_BASE,10,unsigned int)
#define CI_IOC_CDCFG		_IOW(CI_IOC_BASE,11,unsigned int)
#define CI_IOC_SETTS1CTRL	_IOW(CI_IOC_BASE,12,unsigned int)
#define CI_IOC_GETTS1CTRL	_IOW(CI_IOC_BASE,13,unsigned int)
#define CI_IOC_GETPIDRAM	_IOW(CI_IOC_BASE,14,struct mtci_pid_ram)
#define CI_IOC_SETPIDRAM	_IOW(CI_IOC_BASE,15,struct mtci_pid_ram)
#define CI_IOC_CAMPWR		_IOW(CI_IOC_BASE,16,unsigned int)
#define CI_IOC_TSSERIALEN	_IOW(CI_IOC_BASE,17,unsigned int)
#define CI_IOC_FILTEREN		_IOW(CI_IOC_BASE,18,unsigned int)
#define CI_IOC_DUMPREG		_IO(CI_IOC_BASE,0xfe)

#endif

