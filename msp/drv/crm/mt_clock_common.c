/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_clock_common.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT clock common functions.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __UBOOT__
#include <common.h>
#include <command.h>
#include <asm/arch-symphony6/mt_common.h>

#define MUTEX_LOCK()			do{}while(0)
#define MUTEX_UNLOCK()			do{}while(0)

#elif defined(__KERNEL__)

#include <linux/types.h>
#include <linux/string.h>
#include <linux/mutex.h>

#include <mach/chipinfo.h>

#include "mt_log.h"
#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_drv_clock.h"

DEFINE_MUTEX(clk_mutex);
#define MUTEX_LOCK()			mutex_lock(&clk_mutex);
#define MUTEX_UNLOCK()			mutex_unlock(&clk_mutex);

#else
/* RTOS */
#include <stdio.h>
#include <string.h>

#include <sys_types.h>
#include <sys_define.h>

#include "mtos_mutex.h"
#include "hal_misc.h"

#include "mt_log.h"
#include "mt_io.h"
#include "mt_drv_clock.h"

static void *clk_mutex = NULL;
#define MUTEX_LOCK()			do{ \
									if(clk_mutex == NULL) { \
										clk_mutex = mtos_mutex_create(1); \
									} \
									mtos_mutex_take(clk_mutex); \
								}while(0)
#define MUTEX_UNLOCK()			do{ mtos_mutex_give(clk_mutex); }while(0)

#endif

#include "mt_clock.h"
#include "mt_mod_reset.h"

#define ALL_ONE_U32				(~0U)

/******************************************************************************
 *
 * <CNComment>
 *
 *  CRM: Clock Reset Module
 *    (1) Clk EN/SEL	(已实现)
 *    (2) Soft Reset	(由mt_mod_reset支持实现)
 *    (3) Slock/Lock	(已实现)
 *    (4) TOP Control	(由mt_clock_top实现)
 *    (5) xtal_sel (选择xtal时钟,省电模式) (由mt_clock_common+mt_clock_top实现)
 *
 *    CRM模块Clock的配置较为复杂,
 *    mt_clock只实现了基本的gate/rate/mux/divider功能.
 *    某些CLKSEL较为特殊,另外添加一个attr用于配置该CLKSEL
 *    配置这种特殊的CLKSEL(attr),请使用以下两个接口:
 *		mt_clk_get_attr,
 *		mt_clk_set_attr
 *
 * 无毛刺时钟切换可以在模块工作过程中动态切换时钟频率;
 * <Caution!>有毛刺切换要求模块切换时钟后执行模块复位.<Caution!>
 *
 * AXI Clk和APCPU Backup Clk通过流程保证是无毛刺切换.
 *
 * APCPU时钟切换:
 *    1. ARMPLL -> apbackup
 *       (1) ARMPLL -> apbackup(near)
 *       (2) apbackup(near) -> apbackup(target)
 *           e.g.
 *           mt_clk_set_rate(MT_CLK_APBACKUP, xxx);
 *           mt_clk_set_mux(MT_CLK_APCPU, 0);
 *           mt_clk_set_rate(MT_CLK_APBACKUP, xxx);
 *           ...
 *    2. apbackup -> ARMPLL
 *       (1) apbackup(current) -> apbackup(near)
 *       (2) apbackup(near) -> ARMPLL
 *           e.g.
 *           mt_clk_set_rate(MT_CLK_APBACKUP, xxx);
 *           ...
 *           mt_pll_set_freq(MT_ARMPLL, xxx);
 *           mt_pll_reset(MT_ARMPLL);
 *           mdelay(10);
 *           mt_clk_set_mux(MT_CLK_APCPU, 1);
 *    3. ARMPLL <-> ARMPLL
 *       (1) ARMPLL -> apbackup(near)
 *       (2) apbackup(near) -> apbackup(target)
 *       (3) apbackup(target) -> ARMPLL
 *    4. apbackup <-> apbackup
 *       step by step
 *       e.g.
 *         240M<->480M<->576M<->720M
 *         mt_clk_set_rate(MT_CLK_APBACKUP, 240M);
 *         mt_clk_set_rate(MT_CLK_APBACKUP, 480M);
 *         mt_clk_set_rate(MT_CLK_APBACKUP, 576M);
 *         mt_clk_set_rate(MT_CLK_APBACKUP, 720M);
 *
 * GMAC时钟切换:
 *    1. disable
 *    2. clock select
 *    3. phase select
 *    4. reset
 *    5. enable
 *
 * XTAL时钟切换
 *   XTAL时钟切换是有毛刺时钟切换,需要通过流程保证模块在时钟切换过程中
 *   没有时钟毛刺产生.
 *
 * 隐含选择晶振时钟
 *  1.切XTAL时钟
 *    (1)配置模块时钟为非"隐含选择晶振"的时钟;
 *    (2)配置TOP对应的比特为1;
 *    (3)配置模块时钟为"隐含选择晶振"的时钟.
 *    e.g.
 *      mt_clk_set_mux(TOP_AVCPU_XTAL_MUX, 1);
 *      mt_clk_set_mux(MT_CLK_AVCPU, 7);
 *
 *  2.切高频时钟
 *    (1)配置模块时钟为非"隐含选择晶振"的时钟;
 *    (2)配置TOP对应的比特为0;
 *    (3)配置模块时钟为"隐含选择晶振"的时钟,切换到高频时钟.
 *    e.g.
 *      mt_clk_set_rate(MT_CLK_AVCPU, 360000000);
 *      mt_clk_set_mux(TOP_AVCPU_XTAL_MUX, 0);
 *
 *****************************************************************************/

//---------------------------------------------------------------------------//

/* clock group structure */
struct mt_clk_group
{
	struct mt_clk_in *pclk_tab;
	u32				 *pcount;
};

/* clock groups */
static struct mt_clk_group g_clk_groups[] =
{
	{mt_clk_table, 			&mt_clk_table_size},
	{mt_top_clk_table, 		&mt_top_clk_table_size},
	{mt_analog_clk_table,	&mt_analog_clk_table_size},

	//TODO: add your clock table here:

	{NULL, 0},
};

int mt_clk_get_extra(mt_clk_t *clk, u32 *value);
int mt_clk_set_extra(mt_clk_t *clk, u32 value);

static struct mt_clk_in *get_clk_by_name(const char *name)
{
	u32 i, j;
	struct mt_clk_in *pclk_tab;
	u32 count;

	if (name == NULL || strlen(name) == 0)
	{
		MT_LOGE("[E]%s: invalid name!\n", __FUNCTION__);
		return NULL;
	}

#if 0
	for (i=0; i<mt_clk_table_size; i++)
	{
		if (strcmp(name, mt_clk_table[i].name) == 0)
		{
			MT_LOGD("%s: found %s -> %u\n", __FUNCTION__, name, i);
			return &mt_clk_table[i];
		}
	}
#endif

	for (i=0; ; i++)
	{
		pclk_tab = g_clk_groups[i].pclk_tab;

		if (pclk_tab == NULL)
			break;

		count = *g_clk_groups[i].pcount;

		for (j=0; j<count; j++)
		{
			if (strcmp(name, pclk_tab[j].name) == 0)
			{
				MT_LOGD("%s: found %s -> [%u][%u]\n", __FUNCTION__, name, i, j);
				return &pclk_tab[j];
			}
		}
	}

	MT_LOGW("%s: not found %s!\n", __FUNCTION__, name);
	return NULL;
}

#define DEF_GET_CHK_CLK(name)	struct mt_clk_in *pClk; \
								do { pClk = get_clk_by_name(name); \
									if (pClk == NULL) \
										return (-1); \
								} while(0)

#if defined(__UBOOT__) || defined(__KERNEL__)
static bool HAS_GATE(struct mt_clk_in *clk)
#else
/* RTOS */
static BOOL HAS_GATE(struct mt_clk_in *clk)
#endif
{
	return VALID_IO_OBJ((mt_io_obj_t*)&clk->gate);
}

#if defined(__UBOOT__) || defined(__KERNEL__)
static bool HAS_MUX(struct mt_clk_in *clk)
#else
/* RTOS */
static BOOL HAS_MUX(struct mt_clk_in *clk)
#endif
{
	return (((clk->flags & FLAG_CLK_MUX) != 0)
			&& VALID_IO_OBJ((mt_io_obj_t*)&clk->comb));
}

#if defined(__UBOOT__) || defined(__KERNEL__)
static bool HAS_RATE(struct mt_clk_in *clk)
#else
/* RTOS */
static BOOL HAS_RATE(struct mt_clk_in *clk)
#endif
{
	return ((clk->flags & FLAG_CLK_RATE) != 0
			&& VALID_IO_OBJ((mt_io_obj_t*)&clk->comb)
			&& clk->comb.ex != NULL);
}

#if defined(__UBOOT__) || defined(__KERNEL__)
static bool HAS_DIV(struct mt_clk_in *clk)
#else
/* RTOS */
static BOOL HAS_DIV(struct mt_clk_in *clk)
#endif
{
	return ((clk->flags & FLAG_CLK_DIV) != 0
			&& VALID_IO_OBJ((mt_io_obj_t*)&clk->comb)
			&& clk->comb.ex != NULL);
}

#if defined(__UBOOT__) || defined(__KERNEL__)
static bool HAS_ATTR(struct mt_clk_in *clk)
#else
/* RTOS */
static BOOL HAS_ATTR(struct mt_clk_in *clk)
#endif
{
	return (((clk->flags & FLAG_CLK_ATTR) != 0)
			&& VALID_IO_OBJ((mt_io_obj_t*)&clk->attr));
}

#if defined(__UBOOT__) || defined(__KERNEL__)
static bool IS_FIXED_RATE(struct mt_clk_in *clk)
#else
/* RTOS */
static BOOL IS_FIXED_RATE(struct mt_clk_in *clk)
#endif
{
	return ((clk->flags & FLAG_CLK_FIXED_RATE) != 0);
}

#if defined(__UBOOT__) || defined(__KERNEL__)
static bool IS_FIXED_DIV(struct mt_clk_in *clk)
#else
/* RTOS */
static BOOL IS_FIXED_DIV(struct mt_clk_in *clk)
#endif
{
	return ((clk->flags & FLAG_CLK_FIXED_DIV) != 0);
}

/* get mapped value from key */
static int get_map_value(struct mt_clk_map_table *map, u32 key, unsigned long *value, char **parent)
{
	int i;

	for (i=0;;i++)
	{
		if (map[i].key == -1 && map[i].value == -1)
			break;	//error

		if (map[i].key == key)
		{
			if (value)
				*value = map[i].value;

			if (parent)
				*parent = (char*)map[i].parent;

			return 0;
		}
	}

	//error
	MT_LOGW("%s: not found %u!\n", __FUNCTION__, key);
	return (-1);
}

/* get mapped key from value */
static int get_map_key(struct mt_clk_map_table *map, unsigned long value, u32 *key)
{
	int i;

	if (map == NULL)
	{
		MT_LOGW("%s: map is null!\n", __FUNCTION__);
		return (-1);
	}

	for (i=0;;i++)
	{
		if (map[i].key == -1 && map[i].value == -1)
			break;	//error

		if (map[i].value == value)
		{
			*key = map[i].key;
			return 0;
		}
	}

	//error
	MT_LOGW("%s: not found %lu!\n", __FUNCTION__, value);
	return (-1);
}

//---------------------------------------------------------------------------//

static inline u32 get_status(struct mt_clk_in *pClk)
{
	u32 status;

	status = mt_read_obj((mt_io_obj_t*)&pClk->gate);

	if ((pClk->flags & FLAG_CLK_GATE_ACTIVE_LOW) != 0) {
		status = (status==0)?1:0;
	}

	return status;
}

int mt_clk_get_status(mt_clk_t *clk, u32 *status)
{
	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);
	CHECK_NULL_PTR(status);

	if (!HAS_GATE(pClk))
	{
		MT_LOGW("%s: %s has no gate!\n", __FUNCTION__, clk);
		return (-1);
	}

	MUTEX_LOCK();
	*status = get_status(pClk);
	MUTEX_UNLOCK();

	MT_LOGD("%s: %s - %s\n", __FUNCTION__,
			clk,
			*status?"enable":"disable");

	return 0;
}

int mt_clk_enable(mt_clk_t *clk)
{
	int ret;

	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);

	if (!HAS_GATE(pClk))
	{
		MT_LOGW("%s: %s has no gate!\n", __FUNCTION__, clk);
		return (-1);
	}

	MT_LOGD("%s: %s\n", __FUNCTION__, clk);

	MUTEX_LOCK();

	if ((pClk->flags & FLAG_CLK_GATE_ACTIVE_LOW) == 0) {
		ret = mt_write_objlk(&pClk->gate, ALL_ONE_U32);
	} else {
		ret = mt_write_objlk(&pClk->gate, 0);
	}

	patch_clk_enable(clk);

	MUTEX_UNLOCK();

	return ret;
}

int mt_clk_disable(mt_clk_t *clk)
{
	int ret;

	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);

	if (!HAS_GATE(pClk))
	{
		MT_LOGW("%s: %s has no gate!\n", __FUNCTION__, clk);
		return (-1);
	}

	MT_LOGD("%s: %s\n", __FUNCTION__, clk);

	MUTEX_LOCK();

	patch_clk_disable(clk);

	if ((pClk->flags & FLAG_CLK_GATE_ACTIVE_LOW) == 0) {
		ret = mt_write_objlk(&pClk->gate, 0);
	} else {
		ret = mt_write_objlk(&pClk->gate, ALL_ONE_U32);
	}
	MUTEX_UNLOCK();

	return ret;
}

static inline u32 get_comb(struct mt_clk_in *pClk, unsigned long *value, char **parent)
{
	u32 key;

	key = mt_read_obj((mt_io_obj_t*)&pClk->comb);

	MT_LOGD("%s: %s - reg val %u\n", __FUNCTION__, pClk->name, key);

	if (pClk->comb.ex)
	{
		if (value || parent)
		{
			if (0 != get_map_value((struct mt_clk_map_table*)pClk->comb.ex, key, value, parent))
			{
				//not found!
				MT_LOGE("%s: %s - not found map value of reg val %u\n", __FUNCTION__, pClk->name, key);
				return key;
			}
		}
	}

	return key;
}

int mt_clk_get_rate(mt_clk_t *clk, unsigned long *rate)
{
#ifdef DEBUG
	u32 raw = 0;
#endif
	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);
	CHECK_NULL_PTR(rate);

	if (IS_FIXED_RATE(pClk))
	{
		*rate = pClk->fixed_rate;
		goto done;
	}

	if (!HAS_RATE(pClk))
	{
		MT_LOGW("%s: %s has no rate!\n", __FUNCTION__, clk);
		return (-1);
	}

	MUTEX_LOCK();
#ifdef DEBUG
	raw = get_comb(pClk, rate, NULL);
#else
	(void)get_comb(pClk, rate, NULL);
#endif
	MUTEX_UNLOCK();

	MT_LOGD("%s: %s - rate %lu, raw %x\n", __FUNCTION__,
			clk, *rate, raw);

done:
	return (*rate == 0)?(-1):0;
}

int mt_clk_set_rate(mt_clk_t *clk, unsigned long rate)
{
	int ret;
	u32 key;

	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);

	if (rate == 0)
	{
		MT_LOGE("%s: invalid rate!\n", __FUNCTION__);
		return (-1);
	}

	if (!HAS_RATE(pClk))
	{
		MT_LOGW("%s: %s has no rate!\n", __FUNCTION__, clk);
		return (-1);
	}

	//first search by rate
	if (get_map_key((struct mt_clk_map_table*)pClk->comb.ex, rate, &key) != 0)
	{
		MT_LOGE("%s: %s invalid rate %lu!\n", __FUNCTION__, clk, rate);
		return (-1);
	}

	MUTEX_LOCK();
	ret = mt_write_objlk(&pClk->comb, key);
	MUTEX_UNLOCK();

	if (ret != 0)
		return ret;

	MT_LOGD("%s: %s - rate %lu -> reg val %u\n", __FUNCTION__, clk, rate, key);

	return ret;
}

int mt_clk_get_mux(mt_clk_t *clk, u32 *mux)
{
	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);
	CHECK_NULL_PTR(mux);

	if (!HAS_MUX(pClk) && !HAS_RATE(pClk) && !HAS_DIV(pClk))
	{
		MT_LOGW("%s: %s has no mux/rate/div!\n", __FUNCTION__, clk);
		return (-1);
	}

	MUTEX_LOCK();
	*mux = get_comb(pClk, NULL, NULL);
	MUTEX_UNLOCK();

	MT_LOGD("%s: %s - mux %u\n", __FUNCTION__, clk, *mux);

	return 0;
}

int mt_clk_set_mux(mt_clk_t *clk, u32 mux)
{
	int ret;

	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);

	if (!HAS_MUX(pClk) && !HAS_RATE(pClk) && !HAS_DIV(pClk))
	{
		MT_LOGW("%s: %s has no mux/rate/div!\n", __FUNCTION__, clk);
		return (-1);
	}

	MT_LOGD("%s: %s - mux %u\n", __FUNCTION__, clk, mux);

	MUTEX_LOCK();
	ret = mt_write_objlk(&pClk->comb, mux);
	MUTEX_UNLOCK();

	return ret;
}

int mt_clk_get_div(mt_clk_t *clk, u32 *div)
{
#ifdef DEBUG
	u32 raw = 0;
#endif
	unsigned long ldiv = 0;

	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);
	CHECK_NULL_PTR(div);

	if (IS_FIXED_DIV(pClk))
	{
		*div = pClk->fixed_div.value;
		goto done;
	}

	if (!HAS_DIV(pClk))
	{
		MT_LOGW("%s: %s has no divider!\n", __FUNCTION__, clk);
		return (-1);
	}

	MUTEX_LOCK();
#ifdef DEBUG
	raw = get_comb(pClk, &ldiv, NULL);
#else
	(void)get_comb(pClk, &ldiv, NULL);
#endif
	MUTEX_UNLOCK();

	*div = (u32)ldiv;

	MT_LOGD("%s: %s - div(*100) %u, raw %x\n", __FUNCTION__, clk, *div, raw);

done:
	return *div==0?(-1):0;
}

int mt_clk_set_div(mt_clk_t *clk, u32 div)
{
	int ret;
	u32 key;

	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);

	if (div == 0)
	{
		MT_LOGE("%s: invalid divider!\n", __FUNCTION__);
		return (-1);
	}

	if (!HAS_DIV(pClk))
	{
		MT_LOGW("%s: %s has no divider!\n", __FUNCTION__, clk);
		return (-1);
	}

	if (0 != get_map_key((struct mt_clk_map_table*)pClk->comb.ex, div, &key))
	{
		MT_LOGE("%s: %s - div %u not supported!\n", __FUNCTION__, clk, div);
		return (-1);
	}

	MT_LOGD("%s: %s - div(*100) %u -> reg div %u\n", __FUNCTION__, clk, div, key);

	MUTEX_LOCK();
	ret = mt_write_objlk(&pClk->comb, key);
	MUTEX_UNLOCK();

	return ret;
}

static inline u32 get_attr(struct mt_clk_in *pClk)
{
	return mt_read_obj((mt_io_obj_t*)&pClk->attr);
}

int mt_clk_get_extra(mt_clk_t *clk, u32 *value)
{
	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);
	CHECK_NULL_PTR(value);

	if (!HAS_ATTR(pClk))
	{
		MT_LOGW("%s: %s has no extra attr!\n", __FUNCTION__, clk);
		return (-1);
	}

	MUTEX_LOCK();
	*value = get_attr(pClk);
	MUTEX_UNLOCK();

	MT_LOGD("%s: %s - extra attr %x\n", __FUNCTION__, clk, *value);

	return 0;
}

int mt_clk_get_attr(mt_clk_t *clk, u32 *value) __attribute((alias("mt_clk_get_extra")));

int mt_clk_set_extra(mt_clk_t *clk, u32 value)
{
	int ret;

	DEF_GET_CHK_CLK(clk);

	CHECK_NULL_PTR(clk);

	if (!HAS_ATTR(pClk))
	{
		MT_LOGW("%s: %s has no extra attr!\n", __FUNCTION__, clk);
		return (-1);
	}

	MT_LOGD("%s: %s - extra attr %x\n", __FUNCTION__, clk, value);

	MUTEX_LOCK();
	ret = mt_write_objlk(&pClk->attr, value);
	MUTEX_UNLOCK();

	return ret;
}

int mt_clk_set_attr(mt_clk_t *clk, u32 value) __attribute((alias("mt_clk_set_extra")));

int mt_clk_reset(mt_clk_t *clk)
{
	int ret;

	CHECK_NULL_PTR(clk);

	MUTEX_LOCK();
	ret = mt_mod_reset((mt_mod_t*)clk);
	MUTEX_UNLOCK();

	return ret;
}

int mt_clk_reset_reset(mt_clk_t *clk)
{
	int ret;

	CHECK_NULL_PTR(clk);

	MUTEX_LOCK();
	ret = mt_mod_rst_reset((mt_mod_t*)clk);
	MUTEX_UNLOCK();

	return ret;
}

int mt_clk_reset_release(mt_clk_t *clk)
{
	int ret;

	CHECK_NULL_PTR(clk);

	MUTEX_LOCK();
	ret = mt_mod_rst_release((mt_mod_t*)clk);
	MUTEX_UNLOCK();

	return ret;
}

int mt_clk_autogate_enable(void)
{
	int ret;

	MUTEX_LOCK();

#ifdef __UBOOT__
	if (mt_get_chip_id() == MT_CHIP_SYMPHONY6_A0)
		ret = mt_io_batch_write(mt_autogate_params_a0, mt_autogate_params_size_a0);
	else
		ret = mt_io_batch_write(mt_autogate_params, mt_autogate_params_size);
#elif defined(__KERNEL__)
	if (symphony_get_chip_rev() == CHIP_SYMPHONY6_A0)
		ret = mt_io_batch_write(mt_autogate_params_a0, mt_autogate_params_size_a0);
	else
		ret = mt_io_batch_write(mt_autogate_params, mt_autogate_params_size);
#else
/* RTOS */
	if (hal_get_chip_rev() == CHIP_SYMPHONY6_A0)
		ret = mt_io_batch_write(mt_autogate_params_a0, mt_autogate_params_size_a0);
	else
		ret = mt_io_batch_write(mt_autogate_params, mt_autogate_params_size);
#endif

	MUTEX_UNLOCK();

	return ret;
}

#if defined(__KERNEL__) && !defined(__UBOOT__)

void mt_clk_suspend(void)
{
	int i;
	int count = (int)suspend_clock_regs_size;

	MT_LOGD("%s: Enter\n", __FUNCTION__);

	for (i=0; i<count; i++)
	{
		suspend_clock_regs[i].value = MT_IO_READ32(suspend_clock_regs[i].reg);
		MT_LOGD("%lX: %08X\n", suspend_clock_regs[i].reg, suspend_clock_regs[i].value);
	}
}

void mt_clk_resume(void)
{
	int i;
	int count = (int)suspend_clock_regs_size;

	MT_LOGD("%s: Enter\n", __FUNCTION__);

	MUTEX_LOCK();

	mt_top_clk_enable_all();

	for (i=0; i<count; i++)
	{
		MT_LOGD("%lX: %08X\n", suspend_clock_regs[i].reg, suspend_clock_regs[i].value);
		MT_IO_WRITE32(suspend_clock_regs[i].reg, suspend_clock_regs[i].value);
	}

	MUTEX_UNLOCK();

	mt_clk_autogate_enable();
}

#endif

int mt_clk_init(void)
{
	//TODO

	return 0;
}

#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)

#if defined(__UBOOT__)
static void dump_map(struct mt_clk_map_table *map)
#elif defined(__KERNEL__)
static void dump_map(struct seq_file *s, struct mt_clk_map_table *map)
#else
/* RTOS */
static void dump_map(int (*DP_LOG)(const char *fmt, ...), struct mt_clk_map_table *map)
#endif
{
	for (;;)
	{
		if (map->key == -1 && map->value == -1)
			break;

		if (map->parent) {
			DP_LOG("\t\t\t\t%x - %lu(%s)\n", map->key, map->value, map->parent);
		} else {
			DP_LOG("\t\t\t\t%x - %lu\n", map->key, map->value);
		}

		map ++;
	}
}

#if defined(__UBOOT__)
static void dump_clk(struct mt_clk_in *pClk)
#elif defined(__KERNEL__)
static void dump_clk(struct seq_file *s, struct mt_clk_in *pClk)
#else
/* RTOS */
static void dump_clk(int (*DP_LOG)(const char *fmt, ...), struct mt_clk_in *pClk)
#endif
{
	u32 gate = 0xFF;
	u32 mux = 0;
	unsigned long rate = 0;
	u32 raw_rate = 0;
	unsigned long div = 0;
	u32 raw_div = 0;
	u32 attr;
	int rst_status;

	char szNA[] = "x";
	char szDash[] = "-";
	char szSpace[] = " ";
	char *parent = NULL;

	char clk_gate[16];
	char clk_union[32];
	char clk_parent[32];
	char clk_attr[16];
	char str_rst_stat[16];

	struct mt_clk_map_table *map = NULL;

	if (HAS_GATE(pClk))
	{
		gate = get_status(pClk);
		snprintf(clk_gate, sizeof(clk_gate), "%x", gate);
	}
	else
	{
		snprintf(clk_gate, sizeof(clk_gate), "%s", szDash);
	}

	if (HAS_MUX(pClk))
	{
		mux = get_comb(pClk, NULL, &parent);
		map = pClk->comb.ex;
		snprintf(clk_union, sizeof(clk_union), "M%11u[%8x]", mux, mux);
	}
	else if (IS_FIXED_RATE(pClk))
	{
		rate = pClk->fixed_rate;
		snprintf(clk_union, sizeof(clk_union), "R%11lu[%8x]", rate, raw_rate);
	}
	else if (HAS_RATE(pClk))
	{
		raw_rate = get_comb(pClk, &rate, &parent);
		map = pClk->comb.ex;
		snprintf(clk_union, sizeof(clk_union), "R%11lu[%8x]", rate, raw_rate);
	}
	else if (IS_FIXED_DIV(pClk))
	{
		div = pClk->fixed_div.value;
		parent = (char*)pClk->fixed_div.parent;
		snprintf(clk_union, sizeof(clk_union), "D%11lu[%8x]", div, raw_div);
	}
	else if (HAS_DIV(pClk))
	{
		raw_div = get_comb(pClk, &div, &parent);
		map = pClk->comb.ex;
		snprintf(clk_union, sizeof(clk_union), "D%11lu[%8x]", div, raw_div);
	}
	else
	{
		snprintf(clk_union, sizeof(clk_union), "N%11s[%8s]", szNA, szDash);
	}

	if (parent)
	{
		snprintf(clk_parent, sizeof(clk_parent), "%s", parent);
	}
	else
	{
		snprintf(clk_parent, sizeof(clk_parent), "%s", szSpace);
	}

	if (HAS_ATTR(pClk))
	{
		attr = get_attr(pClk);
		snprintf(clk_attr, sizeof(clk_attr), "%x", attr);
	}
	else
	{
		snprintf(clk_attr, sizeof(clk_attr), "%s", szDash);
	}

	mt_clk_get_reset_status((mt_clk_t*)pClk->name, &rst_status);
	memset(str_rst_stat, 0, sizeof(str_rst_stat));
	if (rst_status == 0) {
		strncpy(str_rst_stat, "Release", sizeof(str_rst_stat)-1);
	} else if (rst_status > 0) {
		strncpy(str_rst_stat, "Reset", sizeof(str_rst_stat)-1);
	} else {
		strncpy(str_rst_stat, szNA, sizeof(str_rst_stat)-1);
	}

	DP_LOG("%23s: %4s %s %19s %8s %7s\n", pClk->name, clk_gate, clk_union, clk_parent, clk_attr, str_rst_stat);

	if (map) {
#if defined(__UBOOT__)
		dump_map(map);
#elif defined(__KERNEL__)
		dump_map(s, map);
#else
/* RTOS */
		dump_map(DP_LOG, map);
#endif
	}
}

/* autogate parameter */
static struct
{
	const char *name;
	mt_io_param_t *pio;

} autogate_params[] =
{
	{"SPDMA(SW)", 	&mt_autogate_params[0]},
	{"SPDMA(HW)",	&mt_autogate_params[1]},
	{"FW_REG", 		&mt_autogate_params[2]},
	{"PKA",			&mt_autogate_params[3]},
	{"EDMA", 		&mt_autogate_params[4]},
	{"TSI_DS",		&mt_autogate_params[5]},
	{"M2M",			&mt_autogate_params[6]},
	{"LEDKB", 		&mt_autogate_params[7]},
	{"SPI_FP",		&mt_autogate_params[8]},
	{"VDEC",		&mt_autogate_params[9]},
	{"TSI",			&mt_autogate_params[10]},
	{"GRA", 		&mt_autogate_params[11]},
	{"HD_VENC", 	&mt_autogate_params[12]},
	{"SD_VENC", 	&mt_autogate_params[13]},
	{"SMC", 		&mt_autogate_params[14]},
	{"NFLASH", 		&mt_autogate_params[15]},
	{"SFLASH",		&mt_autogate_params[16]},
	{"OTP",			&mt_autogate_params[17]},
	{"KLE", 		&mt_autogate_params[18]},
#if 0
	{"IFCP_CRYPTO", &mt_autogate_params[?]},
#endif
	{"JPEG", 		&mt_autogate_params[19]},
	{"PNG", 		&mt_autogate_params[20]},
	/* A1+ */
	{"AHB", 		&mt_autogate_params[21]},

/* move to btinit/auxcode, read only */
	{"OIC", 		&mt_autogate_params_rd[0]},
};

#if defined(__UBOOT__)
static void dump_autogate(void)
#elif defined(__KERNEL__)
static void dump_autogate(struct seq_file *s)
#else
/* RTOS */
static void dump_autogate(int (*DP_LOG)(const char *fmt, ...))
#endif
{
	int i;
	int count = sizeof(autogate_params) / sizeof(autogate_params[0]);

	DP_LOG("------------------------------------------Auto Gate------------------------------------------\n");

	for (i=0; i<count; i++)
	{
		DP_LOG("%12s[%lX]: %08X, [%u:%u]=%x\n",
			autogate_params[i].name,
			(unsigned long)autogate_params[i].pio->obj.reg,
			MT_IO_READ32(autogate_params[i].pio->obj.reg),
			autogate_params[i].pio->obj.shift + autogate_params[i].pio->obj.width - 1,
			autogate_params[i].pio->obj.shift,
			MT_GET_BITS(autogate_params[i].pio->obj.reg, autogate_params[i].pio->obj.shift, autogate_params[i].pio->obj.width));
	}
}

#if defined(__UBOOT__)
void mt_clk_dump_autogate(void)
#elif defined(__KERNEL__)
void mt_clk_dump_autogate(struct seq_file *s)
#else
/* RTOS */
void mt_clk_dump_autogate(int (*misc_printf)(const char *fmt, ...))
#endif
{
#if defined(__UBOOT__)
	dump_autogate();
#elif defined(__KERNEL__)
	dump_autogate(s);
#else
	dump_autogate(misc_printf);
#endif
}

#if defined(__UBOOT__)
void mt_clk_dump_state(void)
#elif defined(__KERNEL__)
void mt_clk_dump_state(struct seq_file *s)
#else
/* RTOS */
void mt_clk_dump_state(int (*DP_LOG)(const char *fmt, ...))
#endif
{
	u32 i, j;
	struct mt_clk_in *pclk_tab;
	u32 count;

	DP_LOG("--------------------------------------------CLOCK--------------------------------------------\n");
	DP_LOG("%24s %4s %22s %19s %8s %7s\n", "Name", "Gate", "Mux/Rate/Divider", "Parent", "Attr", "Reset");

#if 0
	for (i=0; i<mt_clk_table_size; i++)
	{
		dump_clk(&mt_clk_table[i]);
	}
#endif

	for (i=0; ; i++)
	{
		pclk_tab = g_clk_groups[i].pclk_tab;

		if (pclk_tab == NULL)
			break;

		count = *g_clk_groups[i].pcount;

		for (j=0; j<count; j++)
		{
#if defined(__UBOOT__)
			dump_clk(&pclk_tab[j]);
#elif defined(__KERNEL__)
			dump_clk(s, &pclk_tab[j]);
#else
/* RTOS */
			dump_clk(DP_LOG, &pclk_tab[j]);
#endif
		}
	}

#if defined(__UBOOT__)
	dump_autogate();
#elif defined(__KERNEL__)
	dump_autogate(s);
#else
	dump_autogate(DP_LOG);
#endif

	DP_LOG("---------------------------------------------------------------------------------------------\n");
}
#endif

//---------------------------------------------------------------------------//

#if defined(__UBOOT__)

#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)

static int do_cmd_clk_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	mt_clk_dump_state();

	return 0;
}

U_BOOT_CMD(
	clk_info, 1, 0,	do_cmd_clk_info,
	"print Clock information",
	""
);

static int do_cmd_clock(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	unsigned long value = 0;

	if (argc < 2)
	{
		return CMD_RET_USAGE;
	}

	if (strcmp(argv[1], "autogate") == 0)
	{
		ret = mt_clk_autogate_enable();
		if (ret == 0)
		{
			printf("enable clock autogate - success.\r\n");
		}
		else
		{
			printf("enable clock autogate - failed!\r\n");
		}

		return ret;
	}

	if (argc < 3)
	{
		return CMD_RET_USAGE;
	}

	if (strcmp(argv[1], "enable") == 0)
	{
		ret = mt_clk_enable((mt_clk_t*)argv[2]);
		if (ret == 0)
		{
			printf("enable clock %s - success.\r\n", argv[2]);
		}
		else
		{
			printf("enable clock %s - failed!\r\n", argv[2]);
		}
	}
	else if (strcmp(argv[1], "disable") == 0)
	{
		ret = mt_clk_disable((mt_clk_t*)argv[2]);
		if (ret == 0)
		{
			printf("disable clock %s - success.\r\n", argv[2]);
		}
		else
		{
			printf("disable clock %s - failed!\r\n", argv[2]);
		}
	}
	else if (strcmp(argv[1], "reset") == 0)
	{
		ret = mt_clk_reset((mt_clk_t*)argv[2]);
		if (ret == 0)
		{
			printf("reset clock %s - success.\r\n", argv[2]);
		}
		else
		{
			printf("reset clock %s - failed!\r\n", argv[2]);
		}
	}
	else
	{
		if (argc < 4)
		{
			return CMD_RET_USAGE;
		}

		value = simple_strtoul(argv[3], NULL, 0);

		if (strcmp(argv[1], "set_rate") == 0)
		{
			ret = mt_clk_set_rate((mt_clk_t*)argv[2], value);
			if (ret == 0)
			{
				printf("set clock %s rate %lu - success.\r\n", argv[2], value);
			}
			else
			{
				printf("set clock %s rate %lu - failed!\r\n", argv[2], value);
			}
		}
		else if (strcmp(argv[1], "set_mux") == 0)
		{
			ret = mt_clk_set_mux((mt_clk_t*)argv[2], (u32)value);
			if (ret == 0)
			{
				printf("set clock %s mux %lu - success.\r\n", argv[2], value);
			}
			else
			{
				printf("set clock %s mux %lu - failed!\r\n", argv[2], value);
			}
		}
		else if (strcmp(argv[1], "set_div") == 0)
		{
			ret = mt_clk_set_div((mt_clk_t*)argv[2], (u32)value);
			if (ret == 0)
			{
				printf("set clock %s div %lu - success.\r\n", argv[2], value);
			}
			else
			{
				printf("set clock %s div %lu - failed!\r\n", argv[2], value);
			}
		}
		else if (strcmp(argv[1], "set_attr") == 0)
		{
			ret = mt_clk_set_attr((mt_clk_t*)argv[2], (u32)value);
			if (ret == 0)
			{
				printf("set clock %s attr %lx - success.\r\n", argv[2], value);
			}
			else
			{
				printf("set clock %s attr %lx - failed!\r\n", argv[2], value);
			}
		}
	}

	return ret;
}

U_BOOT_CMD(
	mt_clock, 4, 0,	do_cmd_clock,
	"Clock sub-system",
	"\n\tmt_clock enable name - enable clock module"
	"\n\tmt_clock disable name - disable clock module"
	"\n\tmt_clock reset name - reset clock module"
	"\n\tmt_clock set_rate name rate - set clock rate"
	"\n\tmt_clock set_mux name mux - set clock mux"
	"\n\tmt_clock set_div name div - set clock divider(multed 100, e.g. 2.75 => 275)"
	"\n\tmt_clock set_attr name attr - set clock extra attribute"
	"\n\tmt_clock autogate - enable clock autogate"
	"\r\n"
);
#endif

#elif defined(__KERNEL__)

u32 MASK32[] =
{
	0x1, 0x3, 0x7, 0xf,
	0x1f, 0x3f, 0x7f, 0xff,
	0x1ff, 0x3ff, 0x7ff, 0xfff,
	0x1fff, 0x3fff, 0x7fff, 0xffff,
	0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
	0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
	0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
	0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

EXPORT_SYMBOL(mt_clk_reset);
EXPORT_SYMBOL(mt_clk_reset_reset);
EXPORT_SYMBOL(mt_clk_reset_release);
EXPORT_SYMBOL(mt_clk_get_status);
EXPORT_SYMBOL(mt_clk_enable);
EXPORT_SYMBOL(mt_clk_disable);
EXPORT_SYMBOL(mt_clk_get_rate);
EXPORT_SYMBOL(mt_clk_set_rate);
EXPORT_SYMBOL(mt_clk_get_mux);
EXPORT_SYMBOL(mt_clk_set_mux);
EXPORT_SYMBOL(mt_clk_get_div);
EXPORT_SYMBOL(mt_clk_set_div);
EXPORT_SYMBOL(mt_clk_get_attr);
EXPORT_SYMBOL(mt_clk_set_attr);

#else
/* RTOS */
u32 MASK32[] =
{
	0x1, 0x3, 0x7, 0xf,
	0x1f, 0x3f, 0x7f, 0xff,
	0x1ff, 0x3ff, 0x7ff, 0xfff,
	0x1fff, 0x3fff, 0x7fff, 0xffff,
	0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
	0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
	0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
	0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

#endif

