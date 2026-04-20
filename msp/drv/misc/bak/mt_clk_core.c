/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk_core.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Montage Clock driver.
 * History        :
 * 1.Date         : 2021/02/08
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/string.h>
#include <linux/mutex.h>

#include "mt_drv_proc.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_reg_io.h"
#include "mt_clk.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
#include "mt_clk_sym4.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
#include "mt_clk_sym2.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY1)
#include "mt_clk_sym1.h"
#endif

#define LEVEL_ERR			"[E]"

//#define DEBUG
#undef DEBUG

#ifdef DEBUG
#define CLK_DEBUG			printk
#else
#define CLK_DEBUG(...)		do{}while(0)
#endif

#define CLK_ERROR(...)		printk(KERN_ERR __VA_ARGS__)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) sizeof(a)/sizeof((a)[0])
#endif

#define CHECK_NULL_RETURN(arg, ret)	\
			do {					\
				if ((void*)(arg) == NULL) {	\
					CLK_ERROR(LEVEL_ERR "%s@%d: param is null or zero!\n", __FUNCTION__, __LINE__);	\
					return ret;	\
				}	\
			} while (0)

/* check if this clk is locked */
#define CHECK_LOCK_RETURN(clk, bit_idx, ret)	\
			do {					\
				if (is_locked(clk, bit_idx)) { \
					CLK_ERROR(LEVEL_ERR "%s@%d: locked!\n", __FUNCTION__, __LINE__); \
					return ret; \
				}	\
			} while (0)

//---------------------------------------------------------------------------//

DEFINE_MUTEX(mt_clk_mutex);

#define MUTEX_LOCK()		do{mutex_lock(&mt_clk_mutex);}while(0)
#define MUTEX_UNLOCK()		do{mutex_unlock(&mt_clk_mutex);}while(0)

static struct mt_clk *mt_clk_get_nb(void *dev, const char *id);
static mt_u32 mt_clk_get_rate_nb(struct mt_clk *clk);

/* general index/value map table structure */
struct table_t
{
	mt_u32 index;
	mt_u32 value;
};

static mt_u32 index_to_value(struct table_t *tab, mt_u32 index)
{
	int i = 0;

	while (tab[i].index != 0 || tab[i].value != 0)
	{
		if (index == tab[i].index)
			return tab[i].value;

		i ++;
	}

	return 0;
}

static mt_u32 value_to_index(struct table_t *tab, mt_u32 value)
{
	int i = 0;

	while (tab[i].index != 0 || tab[i].value != 0)
	{
		if (value == tab[i].value)
			return tab[i].index;

		i ++;
	}

	return (mt_u32)(-1);
}

static MT_BOOL _is_locked(mt_u32 reg_addr, mt_u32 bit_idx)
{
	if (reg_addr != 0
		&& bit_idx != NA_LOCK_BIT_IDX
		&& mt_reg32_get_bit((volatile mt_u32 *)reg_addr, bit_idx) != 0)
		return MT_TRUE;

	return MT_FALSE;
}

/* check if this clk some bit is locked */
static MT_BOOL is_locked(struct mt_clk *clk, mt_u32 bit_idx)
{
	if (_is_locked(clk->reg_slock, bit_idx)
		|| _is_locked(clk->reg_lock, bit_idx))
		return MT_TRUE;

	return MT_FALSE;
}

static mt_u32 local_clk_get_rate(const char *clk_name)
{
	struct mt_clk *clk;

	clk = mt_clk_get_nb(MT_CLK_DEV, clk_name);
	if (clk != NULL)
	{
		return mt_clk_get_rate_nb(clk);
	}

	return 0;
}

//patch for convert get rate
static mt_u32 patch_get_rate(struct mt_clk *clk, mt_u32 rate)
{
	mt_u32 xtal_clk = 27000;
	mt_u32 venc_osclk = 148500;	//148.5
	mt_u32 rate_ret = rate;

	//patch for VAR_XTAL_CLK_HALF
	if (rate == VAR_XTAL_CLK_HALF
		&& (strcmp(clk->name, MT_SMC0_CLK) == 0
		   || strcmp(clk->name, MT_SPI0_CLK) == 0
		   || strcmp(clk->name, MT_SPI1_CLK) == 0))
	{
		xtal_clk = local_clk_get_rate(MT_XTAL_CLK);

		rate_ret = xtal_clk / 2;
	}

	//patch for VAR_XTAL_CLK
	if (rate == VAR_XTAL_CLK)
	{
		if (strcmp(clk->name, MT_SPI0_CLK) == 0
			|| strcmp(clk->name, MT_SPI1_CLK) == 0
			|| strcmp(clk->name, MT_PNAND_CLK) == 0
			|| strcmp(clk->name, MT_UART0_CLK) == 0
			|| strcmp(clk->name, MT_UART1_CLK) == 0)
		{
			xtal_clk = local_clk_get_rate(MT_XTAL_CLK);

			if (xtal_clk == 40000)
				rate_ret = xtal_clk / 2;
			else
				rate_ret = xtal_clk;
		}
	}

	//patch for VAR_VENC_OSCLK
	if (rate == VAR_VENC_OSCLK
		&& strcmp(clk->name, "lcd2x_clk") == 0)
	{
		venc_osclk = local_clk_get_rate("venc_osclk");

		rate_ret = venc_osclk;
	}

	return rate_ret;
}

//patch for convert set rate
static mt_u32 patch_set_rate(struct mt_clk *clk, mt_u32 rate)
{
	mt_u32 xtal_clk = 27000;
	mt_u32 venc_osclk = 148500;	//148.5
	mt_u32 rate_ret = rate;

	xtal_clk = local_clk_get_rate(MT_XTAL_CLK);

	//patch for VAR_XTAL_CLK_HALF
	if (rate == (xtal_clk / 2)
		&& (strcmp(clk->name, MT_SMC0_CLK) == 0
			|| strcmp(clk->name, MT_SPI0_CLK) == 0
			|| strcmp(clk->name, MT_SPI1_CLK) == 0))
	{
		rate_ret = VAR_XTAL_CLK_HALF;
	}

	//patch for VAR_XTAL_CLK
	if (strcmp(clk->name, MT_SPI0_CLK) == 0
		|| strcmp(clk->name, MT_SPI1_CLK) == 0
		|| strcmp(clk->name, MT_PNAND_CLK) == 0
		|| strcmp(clk->name, MT_UART0_CLK) == 0
		|| strcmp(clk->name, MT_UART1_CLK) == 0)
	{
		if (rate == xtal_clk
			|| (xtal_clk == 40000
				&& rate == (xtal_clk / 2)))
		{
			rate_ret = VAR_XTAL_CLK;
		}
	}

	//patch for VAR_VENC_OSCLK
	if (strcmp(clk->name, "lcd2x_clk") == 0)
	{
		venc_osclk = local_clk_get_rate("venc_osclk");
		if (rate == venc_osclk)
			rate_ret = VAR_VENC_OSCLK;
	}

	return rate_ret;
}

//---------------------------------------------------------------------------//

static struct mt_clk *mt_clk_get_nb(void *dev, const char *id)
{
	int i;
	int count = ARRAY_SIZE(mt_clk_table);

	CHECK_NULL_RETURN(id, NULL);

	for (i=0; i<count; i++)
	{
		if (mt_clk_table[i].name != NULL
			&& strcmp(id, mt_clk_table[i].name) == 0)
		{
			CLK_DEBUG("%s: find id(%s) clk(%d).\n", __FUNCTION__, id, i);

			return &mt_clk_table[i];
		}
	}

	CLK_ERROR(LEVEL_ERR "%s: get clk(%s) failed!\n", __FUNCTION__, id);
	return NULL;
}

struct mt_clk *mt_clk_get(void *dev, const char *id)
{
	struct mt_clk *clk;

	MUTEX_LOCK();
	clk = mt_clk_get_nb(dev, id);
	MUTEX_UNLOCK();

	return clk;
}

mt_s32 mt_clk_enable(struct mt_clk *clk)
{
	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_GATE), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->gate.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();

	clk->ref_count ++;

	CLK_DEBUG("%s: clk %s ref_count %u.\n", __FUNCTION__, clk->name, clk->ref_count);

	//enable multi times
	if (clk->ref_count != 1)
	{
		MUTEX_UNLOCK();
		return MT_SUCCESS;
	}

	mt_reg32_set_bit((volatile mt_u32 *)clk->gate.reg_addr, clk->gate.bit_idx);

	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s enabled.\n", __FUNCTION__, clk->name);

	return MT_SUCCESS;
}

void mt_clk_disable(struct mt_clk *clk)
{
	if (clk == NULL || clk->name == NULL) {
		CLK_ERROR(LEVEL_ERR "%s@%d: param is null!\n", __FUNCTION__, __LINE__);
		return;
	}

	if ((clk->flags & FLAG_CLK_GATE) == 0) {
		CLK_ERROR(LEVEL_ERR "%s@%d: clk is not gate!\n", __FUNCTION__, __LINE__);
		return;
	}

	if (is_locked(clk, clk->gate.bit_idx_lock))
	{
		CLK_ERROR(LEVEL_ERR "%s: locked!\n", __FUNCTION__);
		return;
	}

	MUTEX_LOCK();

	if (clk->ref_count == 0)
	{
		CLK_ERROR(LEVEL_ERR "%s: ref_count is zero!\n", __FUNCTION__);
		MUTEX_UNLOCK();
		return;
	}

	clk->ref_count --;

	CLK_DEBUG("%s: clk %s ref_count %u.\n", __FUNCTION__, clk->name, clk->ref_count);

	//disable multi times
	if (clk->ref_count != 0)
	{
		MUTEX_UNLOCK();
		return;
	}

	mt_reg32_clear_bit((volatile mt_u32 *)clk->gate.reg_addr, clk->gate.bit_idx);

	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s disabled.\n", __FUNCTION__, clk->name);
}

mt_u32 mt_clk_get_enable(struct mt_clk *clk)
{
	mt_u32 status = 0;

	CHECK_NULL_RETURN(clk, 0);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_GATE), 0);

	MUTEX_LOCK();

	status = mt_reg32_get_bit((volatile mt_u32 *)clk->gate.reg_addr, clk->gate.bit_idx);

	MUTEX_UNLOCK();

	return status;
}

static mt_u32 mt_clk_get_rate_nb(struct mt_clk *clk)
{
	mt_u32 value;
	mt_u32 rate;

	CHECK_NULL_RETURN(clk, 0);
	CHECK_NULL_RETURN(clk->name, 0);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_RATE), 0);
	CHECK_NULL_RETURN(clk->rate.ptable, 0);

	value = mt_reg32_get_bits((volatile mt_u32 *)clk->rate.reg_addr, clk->rate.shift, clk->rate.width);

	rate = index_to_value((struct table_t*)(clk->rate.ptable), value);

	if (rate == 0)
	{
		CLK_ERROR(LEVEL_ERR "%s: clk %s value %u rate not found!\n", __FUNCTION__, clk->name, value);
	}
/*	else
	{
		//Patch: VAR_XTAL_CLK to xtal clk
		rate = patch_get_rate(clk, rate);

		CLK_DEBUG("%s: clk %s value %u -> rate %u\n", __FUNCTION__, clk->name, value, rate);
	}
*/

	return rate;
}

mt_u32 mt_clk_get_rate(struct mt_clk *clk)
{
	mt_u32 rate = 0;

	MUTEX_LOCK();

	rate = mt_clk_get_rate_nb(clk);

	//Patch: VAR_XTAL_CLK to xtal clk, etc.
	rate = patch_get_rate(clk, rate);

	MUTEX_UNLOCK();

	return rate;
}

mt_s32 mt_clk_set_rate(struct mt_clk *clk, mt_u32 rate)
{
	mt_u32 value;

	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_RATE), MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->rate.ptable, MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->rate.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();

	//Patch: xtal clk to VAR_XTAL_CLK
	rate = patch_set_rate(clk, rate);

	value = value_to_index((struct table_t*)(clk->rate.ptable), rate);

	if (value != (mt_u32)(-1))
	{
		mt_reg32_set_bits((volatile mt_u32 *)clk->rate.reg_addr, clk->rate.shift, clk->rate.width, value);

		MUTEX_UNLOCK();
		CLK_DEBUG("%s: clk %s rate %u -> %u\n", __FUNCTION__, clk->name, rate, value);
		return MT_SUCCESS;
	}
	else
	{
		MUTEX_UNLOCK();
		CLK_ERROR(LEVEL_ERR "%s: clk %s rate %u not found!\n", __FUNCTION__, clk->name, rate);
		return MT_FAILURE;
	}
}

mt_u32 mt_clk_get_div(struct mt_clk *clk)
{
	mt_u32 value;
	mt_u32 div;

	CHECK_NULL_RETURN(clk, 0);
	CHECK_NULL_RETURN(clk->name, 0);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_DIVIDER), 0);
	CHECK_NULL_RETURN(clk->divider.ptable, 0);

	MUTEX_LOCK();

	value = mt_reg32_get_bits((volatile mt_u32 *)clk->divider.reg_addr, clk->divider.shift, clk->divider.width);

	div = index_to_value((struct table_t*)(clk->divider.ptable), value);

	if (div == 0)
	{
		CLK_ERROR(LEVEL_ERR "%s: clk %s value %u div not found!\n", __FUNCTION__, clk->name, value);
	}
	else
	{
		CLK_DEBUG("%s: clk %s value %u -> div %u\n", __FUNCTION__, clk->name, value, div);
	}

	MUTEX_UNLOCK();

	return div;
}

mt_s32 mt_clk_set_div(struct mt_clk *clk, mt_u32 div)
{
	mt_u32 value;

	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_DIVIDER), MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->divider.ptable, MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->divider.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();

	value = value_to_index((struct table_t*)(clk->divider.ptable), div);

	if (value != (mt_u32)(-1))
	{
		mt_reg32_set_bits((volatile mt_u32 *)clk->divider.reg_addr, clk->divider.shift, clk->divider.width, value);

		MUTEX_UNLOCK();
		CLK_DEBUG("%s: clk %s div %u -> %u\n", __FUNCTION__, clk->name, div, value);
		return MT_SUCCESS;
	}
	else
	{
		MUTEX_UNLOCK();
		CLK_ERROR(LEVEL_ERR "%s: clk %s div %u not found!\n", __FUNCTION__, clk->name, div);
		return MT_FAILURE;
	}
}

mt_u32 mt_clk_get_mux(struct mt_clk *clk)
{
	mt_u32 mux;

	CHECK_NULL_RETURN(clk, 0);
	CHECK_NULL_RETURN(clk->name, 0);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_MUX), 0);

	MUTEX_LOCK();
	mux = mt_reg32_get_bits((volatile mt_u32 *)clk->mux.reg_addr, clk->mux.shift, clk->mux.width);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s mux %u\n", __FUNCTION__, clk->name, mux);

	return mux;
}

mt_s32 mt_clk_set_mux(struct mt_clk *clk, mt_u32 mux)
{
	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_MUX), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->mux.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();
	mt_reg32_set_bits((volatile mt_u32 *)clk->mux.reg_addr, clk->mux.shift, clk->mux.width, mux);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s mux %u\n", __FUNCTION__, clk->name, mux);

	return MT_SUCCESS;
}

mt_u32 mt_clk_get_edge(struct mt_clk *clk)
{
	mt_u32 edge;

	CHECK_NULL_RETURN(clk, 0);
	CHECK_NULL_RETURN(clk->name, 0);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_EDGE), 0);

	MUTEX_LOCK();
	edge = mt_reg32_get_bits((volatile mt_u32 *)clk->edge.reg_addr, clk->edge.shift, clk->edge.width);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s edge %u\n", __FUNCTION__, clk->name, edge);

	return edge;
}

mt_s32 mt_clk_set_edge(struct mt_clk *clk, mt_u32 edge)
{
	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_EDGE), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->edge.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();
	mt_reg32_set_bits((volatile mt_u32 *)clk->edge.reg_addr, clk->edge.shift, clk->edge.width, edge);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s edge %u\n", __FUNCTION__, clk->name, edge);

	return MT_SUCCESS;
}

mt_u32 mt_clk_get_attr(struct mt_clk *clk)
{
	mt_u32 attr;

	CHECK_NULL_RETURN(clk, 0);
	CHECK_NULL_RETURN(clk->name, 0);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_ATTR), 0);

	MUTEX_LOCK();
	attr = mt_reg32_get_bits((volatile mt_u32 *)clk->attr.reg_addr, clk->attr.shift, clk->attr.width);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s attr %u\n", __FUNCTION__, clk->name, attr);

	return attr;
}

mt_s32 mt_clk_set_attr(struct mt_clk *clk, mt_u32 attr)
{
	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_ATTR), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->attr.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();
	mt_reg32_set_bits((volatile mt_u32 *)clk->attr.reg_addr, clk->attr.shift, clk->attr.width, attr);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s attr %u\n", __FUNCTION__, clk->name, attr);

	return MT_SUCCESS;
}

mt_s32 mt_clk_select_xtal(struct mt_clk *clk)
{
	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_XTAL_SEL), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->xtal_sel.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();
	mt_reg32_set_bit((volatile mt_u32 *)clk->xtal_sel.reg_addr, clk->xtal_sel.bit_idx);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s xtal selected.\n", __FUNCTION__, clk->name);

	return MT_SUCCESS;
}

mt_s32 mt_clk_unselect_xtal(struct mt_clk *clk)
{
	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_XTAL_SEL), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(clk, clk->xtal_sel.bit_idx_lock, MT_FAILURE);

	MUTEX_LOCK();
	mt_reg32_clear_bit((volatile mt_u32 *)clk->xtal_sel.reg_addr, clk->xtal_sel.bit_idx);
	MUTEX_UNLOCK();

	CLK_DEBUG("%s: clk %s xtal unselected.\n", __FUNCTION__, clk->name);

	return MT_SUCCESS;
}

mt_u32 mt_clk_get_xtal_sel(struct mt_clk *clk)
{
	mt_u32 sel = 0;

	CHECK_NULL_RETURN(clk, 0);
	CHECK_NULL_RETURN((clk->flags & FLAG_CLK_XTAL_SEL), 0);

	MUTEX_LOCK();
	sel = mt_reg32_get_bit((volatile mt_u32 *)clk->xtal_sel.reg_addr, clk->xtal_sel.bit_idx);
	MUTEX_UNLOCK();

	return sel;
}

void mt_clk_dump_state(struct seq_file *p)
{
	int i;
	int count;
	struct mt_clk *clk;
	struct mt_clk *p_clk_table;

	char str_gate[16];
	char str_rate[16];
	char str_mux[8];
	char str_div[16];
	char str_edge[8];
	char str_attr[8];
	char str_xtal_sel[8];

	//printk(KERN_INFO "======================================== DUMP CLK ========================================\r\n");
	//printk(KERN_INFO "[Clock]             \t[Gate] \t[Rate(KHz)]\t[Mux]\t[Div]\t[Edge]\t[Attr]\t[XTAL Sel]\r\n");
	PROC_PRINT(p, "======================================== DUMP CLK ========================================\r\n");
 	PROC_PRINT(p, "[Clock]             \t[Gate] \t[Rate(KHz)]\t[Mux]\t[Div]\t[Edge]\t[Attr]\t[XTAL Sel]\r\n");

	p_clk_table = mt_clk_table;
	count = ARRAY_SIZE(mt_clk_table);

	for (i=0; i<count; i++)
	{
		clk = &p_clk_table[i];

		//printk(KERN_INFO "CLK[%d]: %s\n", i, clk->name);
		//printk(KERN_INFO "\tSLOCK: %X LOCK: %X\n", clk->reg_slock, clk->reg_lock);

		if ((clk->flags & FLAG_CLK_GATE) != 0)
		{
			/*printk(KERN_INFO "\tGATE[%X][%u]: %u (0: OFF, 1: ON)\r\n",
				clk->gate.reg_addr, clk->gate.bit_idx,
				mt_clk_get_enable(clk));*/
			snprintf(str_gate, sizeof(str_gate), "%u", mt_clk_get_enable(clk));
		}
		else
		{
			snprintf(str_gate, sizeof(str_gate), "NA");
		}

		if ((clk->flags & FLAG_CLK_RATE) != 0)
		{
			/*printk(KERN_INFO "\tRATE[%X][%u:%u]: %u (KHz)\n",
				clk->rate.reg_addr, clk->rate.shift+clk->rate.width-1,
				clk->rate.shift,
				mt_clk_get_rate(clk));*/
			snprintf(str_rate, sizeof(str_rate), "%u", mt_clk_get_rate(clk));
		}
		else
		{
			snprintf(str_rate, sizeof(str_rate), "NA");
		}

		if ((clk->flags & FLAG_CLK_MUX) != 0)
		{
			/*printk(KERN_INFO "\tMUX[%X][%u:%u]: %u\n",
				clk->mux.reg_addr, clk->mux.shift+clk->mux.width-1,
				clk->mux.shift,
				mt_clk_get_mux(clk));*/
			snprintf(str_mux, sizeof(str_mux), "%u", mt_clk_get_mux(clk));
		}
		else
		{
			snprintf(str_mux, sizeof(str_mux), "NA");
		}

		if ((clk->flags & FLAG_CLK_DIVIDER) != 0)
		{
			mt_u32 div_int;
			mt_u32 div_dec;

			div_int = mt_clk_get_div(clk);
			div_dec = div_int % 100;
			div_int = div_int / 100;

			/*printk(KERN_INFO "\tDIV[%X][%u:%u]: %u.%02u\n",
				clk->divider.reg_addr, clk->divider.shift+clk->divider.width-1,
				clk->divider.shift,
				div_int, div_dec);*/
			snprintf(str_div, sizeof(str_div), "%u.%02u", div_int, div_dec);
		}
		else
		{
			snprintf(str_div, sizeof(str_div), "NA");
		}

		if ((clk->flags & FLAG_CLK_EDGE) != 0)
		{
			/*printk(KERN_INFO "\tEDGE[%X][%u:%u]: %u\n",
				clk->edge.reg_addr, clk->edge.shift+clk->edge.width-1,
				clk->edge.shift,
				mt_clk_get_edge(clk));*/
			snprintf(str_edge, sizeof(str_edge), "%u", mt_clk_get_edge(clk));
		}
		else
		{
			snprintf(str_edge, sizeof(str_edge), "NA");
		}

		if ((clk->flags & FLAG_CLK_ATTR) != 0)
		{
			/*printk(KERN_INFO "\tATTR[%X][%u:%u]: %u\n",
				clk->attr.reg_addr, clk->attr.shift+clk->attr.width-1,
				clk->attr.shift,
				mt_clk_get_attr(clk));*/
			snprintf(str_attr, sizeof(str_attr), "%u", mt_clk_get_attr(clk));
		}
		else
		{
			snprintf(str_attr, sizeof(str_attr), "NA");
		}

		if ((clk->flags & FLAG_CLK_XTAL_SEL) != 0)
		{
			/*printk(KERN_INFO "\tXTAL_SEL[%X][%u]: %u (0: NO, 1: YES)\n",
				clk->xtal_sel.reg_addr, clk->xtal_sel.bit_idx,
				mt_clk_get_xtal_sel(clk));*/
			snprintf(str_xtal_sel, sizeof(str_xtal_sel), "%u", mt_clk_get_xtal_sel(clk));
		}
		else
		{
			snprintf(str_xtal_sel, sizeof(str_xtal_sel), "NA");
		}

		//printk(KERN_INFO "%-19s\t%-7s\t%-11s\t%s\t%s\t%s\t%s\t%s\r\n",
		//	clk->name, str_gate, str_rate, str_mux, str_div, str_edge, str_attr, str_xtal_sel);
		PROC_PRINT(p, "%-19s\t%-7s\t%-11s\t%s\t%s\t%s\t%s\t%s\r\n",
			clk->name, str_gate, str_rate, str_mux, str_div, str_edge, str_attr, str_xtal_sel);
	}

	//printk(KERN_INFO "==========================================================================================\r\n");
	PROC_PRINT(p, "==========================================================================================\r\n");
}

