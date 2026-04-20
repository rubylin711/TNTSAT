/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_reg_io.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/05/14
 * Description    : Montage Register IO function.
 * History        :
 * 1.Date         : 2021/05/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_REG_IO_H__
#define __INC_MT_REG_IO_H__

#ifdef __UBOOT__
#include <asm/io.h>

static inline u32 HAL_GET_U32(volatile u32 *reg)
{
	return readl(reg);
}

static inline void HAL_PUT_U32(volatile u32 *reg, u32 value)
{
	writel(value, reg);
}

#else

#ifndef MT_ASSERT
#define MT_ASSERT(expr)		if (!(expr)) { \
								printk(KERN_ALERT "[ASSERT]@%s, %d!\n", __FUNCTION__,__LINE__); \
							}
#endif

#endif

static mt_u32 MASK[] =
{
	0x01, 0x03, 0x07, 0x0F,
	0x1F, 0x3F, 0x7F, 0xFF,
	0x1FF, 0x3FF, 0x7FF, 0xFFF,
	0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
	0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF,
	0x1FFFFF, 0x3FFFFF, 0x7FFFFF, 0xFFFFFF,
	0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};

static inline mt_u32 mt_reg32_read(volatile mt_u32 *reg)
{
	MT_ASSERT(reg != NULL);

	return (mt_u32)HAL_GET_U32((volatile u32*)reg);
}

static inline void mt_reg32_write(volatile mt_u32 *reg, mt_u32 value)
{
	MT_ASSERT(reg != NULL);

	HAL_PUT_U32((volatile u32*)reg, (u32)value);
}

static inline mt_u32 mt_reg32_get_bit(volatile mt_u32 *reg, mt_u32 bit)
{
	mt_u32 val;

	MT_ASSERT(reg != NULL);
	MT_ASSERT(bit < 32);

	val = mt_reg32_read(reg);

	val = (val >> bit) & 0x01;

	return val;
}

static inline void mt_reg32_set_bit(volatile mt_u32 *reg, mt_u32 bit)
{
	mt_u32 val;

	MT_ASSERT(reg != NULL);
	MT_ASSERT(bit < 32);

	val = mt_reg32_read(reg);
	val |= (0x01 << bit);

	mt_reg32_write(reg, val);
}

static inline void mt_reg32_clear_bit(volatile mt_u32 *reg, mt_u32 bit)
{
	mt_u32 val;

	MT_ASSERT(reg != NULL);
	MT_ASSERT(bit < 32);

	val = mt_reg32_read(reg);
	val &= ~(0x01 << bit);

	mt_reg32_write(reg, val);
}

static inline mt_u32 mt_reg32_get_bits(volatile mt_u32 *reg, mt_u32 shift, mt_u32 width)
{
	mt_u32 val;

	MT_ASSERT(reg != NULL);
	MT_ASSERT(shift < 32);
	MT_ASSERT(width < 32);

	val = mt_reg32_read(reg);

	val = (val >> shift) & MASK[width-1];

	return val;
}

static inline void mt_reg32_set_bits(volatile mt_u32 *reg, mt_u32 shift, mt_u32 width, mt_u32 value)
{
	mt_u32 val;

	MT_ASSERT(reg != NULL);
	MT_ASSERT(shift < 32);
	MT_ASSERT(width < 32);

	val = mt_reg32_read(reg);
	val &= ~(MASK[width-1] << shift);
	val |= ((value & MASK[width-1]) << shift);

	mt_reg32_write(reg, val);
}

#endif

