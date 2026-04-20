/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Montage LZ Co., Ltd.
 */
#ifndef __INC_MT_LOG_INTERNAL_H__
#define __INC_MT_LOG_INTERNAL_H__

#if defined(__UBOOT__)
//#define DUMP_LOG					printf
#define DUMP_LOG(...)

//#define TRACE_LOG					printf
#define TRACE_LOG(...)

#elif defined(__KERNEL__)
//#define DUMP_LOG					printk
#define DUMP_LOG(...)

//#define TRACE_LOG					printk
#define TRACE_LOG(...)
#else
/* RTOS */
//#define DUMP_LOG					OS_PRINTF
#define DUMP_LOG(...)

//#define TRACE_LOG					OS_PRINTF
#define TRACE_LOG(...)
#endif

static int CHECK_REGISTER_DEBUG(unsigned long reg, u8 shift, u8 width, u32 val, const char *func, int line)
{
	u32 rd_val;

	rd_val = MT_IO_READ32(reg);
	rd_val = (rd_val >> shift) & MASK32[width-1];

	if (rd_val != val)
	{
		MT_BUG("\n[BUG]%s@%d: Check Register[%lX] Bit %u-%u Value %X != %X!\n",
				func, line,
				reg,
				shift,
				shift+width-1,
				rd_val,
				val);

		return (-250);
	}
	else
	{
		return 0;
	}
}

#if 1
#define CHECK_REGISTER(r, s, w, v)			CHECK_REGISTER_DEBUG(r, s, w, v, __FUNCTION__, __LINE__)
#else
#define CHECK_REGISTER(...)
#endif

static void dump_reg(unsigned long reg)
{
	if (reg == 0)
		return;

	DUMP_LOG("Reg[%08lX]: %08X\n", reg, MT_IO_READ32(reg));
}

#endif

