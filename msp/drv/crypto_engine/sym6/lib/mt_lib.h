/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _MT_LIB_H_
#define _MT_LIB_H_

#include "mt_type.h"
#include "mt_utils.h"

#ifdef LINUX
#include <linux/slab.h>
#include <linux/list.h>

#include "mt_cache.h"

static inline void flush_dcache_range(mt_u64 addr, mt_u32 size)
{
    mt_u64 residue = addr & (CACHE_LINE_SIZE - 1);
    addr &= ~residue;
    size = (size + residue + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
    mt_dcache_flush((void *)(ulong)addr, size);
}
#else
#include "stdlib.h"
#include "string.h"
#include "printk.h"
#include "hexdump.h"
#include "uart.h"
#include "cpu.h"
#include "memory/malloc.h"
#include "log.h"
#include "cmd.h"
#endif

#define HW_CE_SUCCESS               (0)
#define HW_CE_ERROR_GENERIC         (-1)
#define HW_CE_ERROR_BAD_PARAMETERS  (-2)
#define HW_CE_ERROR_OVERFLOW        (-3)
#define HW_CE_ERROR_BUSY            (-4)
#define HW_CE_ERROR_OUT_OF_MEMORY   (-5)
#define HW_CE_ERROR_BAD_STATE       (-6)
#define HW_CE_ERROR_NOT_SUPPORTED   (-7)
#define HW_CE_ERROR_SECURITY        (-8)

#define EMSG(fmt, args...) printk(fmt, ##args)

#define M2M_DEBUG
#ifdef M2M_DEBUG
#define DMSG(fmt, args...) printk(fmt, ##args)
#else
#define DMSG(fmt, args...)
#endif

#endif /* end of include guard: _MT_LIB_H_ */
