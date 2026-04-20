/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_io.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT register io functions.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_IO_H__
#define __INC_MT_IO_H__

#ifdef __UBOOT__
#include <asm/io.h>
#elif defined(__KERNEL__)
#include <linux/io.h>
#else
/* RTOS */
#include <arm.h>
#include <sys_types.h>
#endif

#include "mt_log.h"

/* mt_io.h */
#ifdef __cplusplus
extern "C" {
#endif

/* invalid shift */
#define INV_SFT		0xFF

/* invalid width */
#define INV_WD		0

#define CHECK_SHIFT(s) 			((s >= 0) && (s <= 31))
#define CHECK_WIDTH(w) 			((w >= 1) && (w <= 32))

#ifdef __UBOOT__
#define readl_relaxed(a)		__raw_readl(a)
#define writel_relaxed(v, a)	__raw_writel(v, a)
#endif

extern u32 MASK32[];

static inline u32 set_bits(u32 org_val, u32 shift, u32 width, u32 set_val)
{
	if (CHECK_SHIFT(shift) && CHECK_WIDTH(width))
	{
		org_val &= ~(MASK32[width-1] << shift);

		org_val |= ((set_val & MASK32[width-1]) << shift);
	}

	return org_val;
}

static inline u32 mt_io_read32(volatile u32 *reg)
{
	if (reg == NULL)
		return 0;

	return readl(reg);
}

static inline int mt_io_write32(volatile u32 *reg, u32 val)
{
	if (reg == NULL)
		return (-22);

	writel(val, reg);

	return 0;
}

static inline u32 mt_io_get_bit(volatile u32 *reg, u8 shift)
{
	u32 val;

	if (reg == NULL || !CHECK_SHIFT(shift))
		return 0;

	val = mt_io_read32(reg);
	val = val >> shift;
	val = val & 0x01;

	return val;
}

static inline u32 mt_io_get_bits(volatile u32 *reg, u8 shift, u8 width)
{
	u32 val;

	if (reg == NULL || !CHECK_SHIFT(shift) || !CHECK_WIDTH(width))
		return 0;

	val = mt_io_read32(reg);
	val = val >> shift;
	val = val & MASK32[width-1];

	return val;
}

static inline int mt_io_set_bit(volatile u32 *reg, u8 shift, u32 val)
{
	u32 reg_val;

	if (reg == NULL || !CHECK_SHIFT(shift))
		return (-22);

	reg_val = mt_io_read32(reg);
	reg_val &= ~(0x01 << shift);
	reg_val |= ((val & 0x01) << shift);

	mt_io_write32(reg, reg_val);

	return 0;
}

static inline int mt_io_set_bits(volatile u32 *reg, u8 shift, u8 width, u32 val)
{
	u32 reg_val;

	if (reg == NULL || !CHECK_SHIFT(shift) || !CHECK_WIDTH(width))
		return (-22);

	reg_val = mt_io_read32(reg);
	reg_val &= ~(MASK32[width-1] << shift);
	reg_val |= ((val & MASK32[width-1]) << shift);

	mt_io_write32(reg, reg_val);

	return 0;
}

/* avoid compile warning */
#define MT_IO_READ32(c)			mt_io_read32((volatile u32*)(c))
#define MT_IO_WRITE32(c,v)		mt_io_write32((volatile u32*)(c), (u32)(v))
#define MT_GET_BIT(a, s)		mt_io_get_bit((volatile u32*)(a), (u8)(s))
#define MT_GET_BITS(a, s, w)	mt_io_get_bits((volatile u32*)(a), (u8)(s), (u8)(w))
#define MT_SET_BIT(a, s, v)		mt_io_set_bit((volatile u32*)(a), (u8)(s), (u32)v)
#define MT_SET_BITS(a, s, w, v)	mt_io_set_bits((volatile u32*)(a), (u8)(s), (u8)(w), (u32)(v))

//---------------------------------------------------------------------------//

/* register io object definition */
typedef struct
{
	void *reg;	/* register address */
	u8 shift;
	u8 width;

} mt_io_obj_t;

/* register io object with extended pointer */
typedef struct
{
	void *reg;	/* register address */
	u8 shift;
	u8 width;

	void *ex;	/* extended */

} mt_io_objex_t;

/* register io object with extended pointer, and SLOCK/LOCK */
typedef struct
{
	void *reg;	/* register address */
	u8 shift;
	u8 width;

	mt_io_obj_t slock;
	mt_io_obj_t lock;

	void *ex;	/* extended */

} mt_io_objlk_t;

#define MT_INIT_OBJ(a, s, w)		{ .reg = (void*)(a), \
							  	 	 .shift = (u8)(s), \
							  	 	 .width = (u8)(w) }

#define MT_INIT_OBJEX(a, s, w, e)	{ .reg = (void*)(a), \
							  	  	  .shift = (u8)(s), \
							  	  	  .width = (u8)(w),	\
							  	  	  .ex = (void*)(e) }

/*
 * a: reg address
 * s: shift
 * w: width
 * aslk: slock reg address
 * sslk: slock shift
 * wslk: slock width
 * alk: lock reg address
 * slk: lock shift
 * wlk: lock width
 * e: extended pointer(map table)
 */
#define MT_INIT_OBJLK(a, s, w, aslk, sslk, wslk, alk, slk, wlk, e) \
									{ .reg = (void*)(a), \
							  	  	  .shift = (u8)(s), \
							  	  	  .width = (u8)(w),	\
							  	  	  .slock = { .reg = (void*)(aslk), \
							  	  	             .shift = (u8)(sslk), \
							  	  	             .width = (u8)(wslk) \
							  	  	  		   }, \
									  .lock = { .reg = (void*)(alk), \
												.shift = (u8)(slk), \
												.width = (u8)(wlk) \
											  }, \
									  .ex = (void*)(e) \
							  	  	 }

#define MT_INIT_OBJLK_SPL(a, s, w, aslk, alk, slk) \
									MT_INIT_OBJLK(a, s, w, aslk, slk, 1, alk, slk, 1, NULL)

#if defined(__UBOOT__) || defined(__KERNEL__)
static inline bool VALID_IO_OBJ(mt_io_obj_t *obj)
#else
/* RTOS */
static inline BOOL VALID_IO_OBJ(mt_io_obj_t *obj)
#endif
{
	return (obj != NULL && obj->reg != NULL);
}

static inline u32 mt_read_obj(mt_io_obj_t *obj)
{
	if (!VALID_IO_OBJ(obj))
		return 0;

	return MT_GET_BITS(obj->reg, obj->shift, obj->width);
}

static inline int mt_write_obj(mt_io_obj_t *obj, u32 val)
{
	if (!VALID_IO_OBJ(obj))
		return (-22);

	return MT_SET_BITS(obj->reg, obj->shift, obj->width, val);
}

//---------------------------------------------------------------------------//
//
// Write slocked or locked IO Obj
//

#if defined(__UBOOT__) || defined(__KERNEL__)
static inline bool IS_LOCKED(mt_io_obj_t *obj)
{
	if (!VALID_IO_OBJ(obj))
		return false;

	return ((mt_read_obj(obj) != 0)?true:false);
}
#else
/* RTOS */
static inline BOOL IS_LOCKED(mt_io_obj_t *obj)
{
	if (!VALID_IO_OBJ(obj))
		return FALSE;

	return ((mt_read_obj(obj) != 0)?TRUE:FALSE);
}
#endif

static inline int mt_write_objlk(mt_io_objlk_t *obj, u32 val)
{
	if (!VALID_IO_OBJ((mt_io_obj_t*)obj))
	{
		MT_LOGE("[E]%s: invalid IO obj!\n", __FUNCTION__);
		return (-22);
	}

	if (IS_LOCKED(&obj->slock))
	{
		MT_LOGW("[W]%s: Reg %lX[%u] is slocked!\n",
				__FUNCTION__, (unsigned long)obj->reg, obj->shift);
		return (-1);
	}

	if (IS_LOCKED(&obj->lock))
	{
		MT_LOGW("[W]%s: Reg %lX[%u] is locked!\n",
				__FUNCTION__, (unsigned long)obj->reg, obj->shift);
		return (-1);
	}

	return MT_SET_BITS(obj->reg, obj->shift, obj->width, val);
}

/* register io parameter definition */
typedef struct
{
	mt_io_obj_t obj;
	u32         value;

} mt_io_param_t;

static inline int mt_io_batch_write(mt_io_param_t *para, unsigned int count)
{
	unsigned int i;
	u32 value;
	int ret = 0;

	if (para == NULL || count == 0)
	{
		MT_LOGE("[E]%s: invalid parameter!\n", __FUNCTION__);
		return (-22);
	}

	for (i=0; i<count; i++)
	{
		ret = mt_write_obj(&para[i].obj, para[i].value);
		if (ret != 0) {
			//FIXME: break or continue?
			return ret;
		}

		value = mt_read_obj(&para[i].obj);
		if (value != para[i].value)
		{
			MT_LOGW("[E]%s: %8lX[%u:%u] read (0x%X) != write (0x%X)\n", __FUNCTION__,
				(unsigned long)para[i].obj.reg,
				para[i].obj.shift+para[i].obj.width-1,
				para[i].obj.shift,
				value,
				para[i].value);
			return (-22);
		}
	}

	return ret;
}

/* register value definition */
typedef struct
{
	unsigned long reg;
	u32 value;

} mt_reg_value_t;

#ifdef __cplusplus
}
#endif

#endif

