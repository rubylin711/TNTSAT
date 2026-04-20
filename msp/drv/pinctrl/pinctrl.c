/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2020, Montage-LZ Technology Co., Ltd.
 *
 * File Name      : pinctrl.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2020/9/22
 * Description    : PinCtrol Core functions.
 * History        :
 * 1.Date         : 2020/9/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#if defined(__UBOOT__)
#include <common.h>
#include <command.h>
#include <asm/arch-symphony6/mt_common.h>

#define MUTEX_LOCK()		do{}while(0)
#define MUTEX_UNLOCK()		do{}while(0)

#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/mutex.h>
#include "mt_drv_pinctrl.h"

/* FIXME */
#include "../crm/mt_io.h"

DEFINE_MUTEX(pin_mutex);
#define MUTEX_LOCK()		mutex_lock(&pin_mutex)
#define MUTEX_UNLOCK()		mutex_unlock(&pin_mutex)

#else
/* user space */
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "mt_common.h"
#include "mt_drv_pinctrl.h"
#include "pinctrl_user.h"

#define PRINTF				printf
#define DP_LOG				printf

static pthread_mutex_t pin_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MUTEX_LOCK()		pthread_mutex_lock(&pin_mutex)
#define MUTEX_UNLOCK()		pthread_mutex_unlock(&pin_mutex)
#endif

#include "pinctrl.h"

#if defined(__UBOOT__)
#define ASSERT(expr)	do { \
							if (!(expr)) { \
								printf("!!! [ASSERT@]%s:%d !!!\n", __FUNCTION__, __LINE__); \
								hang(); \
							} \
						} while(0)
#elif defined(__KERNEL__)
#define ASSERT(expr)	do { \
							if (!(expr)) { \
								printk(KERN_CRIT "!!! [ASSERT@]%s:%d !!!\n", __FUNCTION__, __LINE__); \
							} \
						} while(0)
#else
#define ASSERT			assert
#endif

unsigned int pinctrl_get_io_ctrl(PINMUX_INDEX_E index);
void pinctrl_set_io_ctrl(PINMUX_INDEX_E index, unsigned int val);

unsigned int pinctrl_get_sel(PINMUX_INDEX_E index);
void pinctrl_set_sel(PINMUX_INDEX_E index, unsigned int val);

static struct pinmux_in *get_pin_obj(PINMUX_INDEX_E index, unsigned long *reg_base)
{
	unsigned int i, j, count;
	struct pinmux_in *tab;

	*reg_base = 0;

	for (i=0; i<g_pin_group_count; i++)
	{
		tab = g_pin_group[i].pin_tab;
		count = g_pin_group[i].pin_count;

		for (j=0; j<count; j++)
		{
			if (tab[j].index == index)
			{
				*reg_base = g_pin_group[i].reg_mapped;
				return &tab[j];
			}
		}
	}

	MT_LOGE("error: %s pin %d not found!\r\n", __FUNCTION__, index);
	return NULL;
}

#define DEF_PIN()			unsigned long reg_base; struct pinmux_in *pin
#define GET_PIN(index)		pin = get_pin_obj(index, &reg_base)

unsigned int pinctrl_get_io_ctrl(PINMUX_INDEX_E index)
{
	unsigned int val;

	DEF_PIN();
	ASSERT(index >= 0 && index < INDEX_PINMUX_MAX);
	GET_PIN(index);

	if (!pin) return 0;

	MUTEX_LOCK();
	val = MT_GET_BITS(reg_base + pin->offset,
						pin->io_ctrl.shift,
						pin->io_ctrl.width);
	MUTEX_UNLOCK();

	return val;
}
unsigned int mt_pinctrl_get_io_ctrl(PINMUX_INDEX_E index) __attribute((alias("pinctrl_get_io_ctrl")));

void pinctrl_set_io_ctrl(PINMUX_INDEX_E index, unsigned int val)
{
	DEF_PIN();
	ASSERT(index >= 0 && index < INDEX_PINMUX_MAX);
	GET_PIN(index);

	if (!pin) return;

	MUTEX_LOCK();
	MT_SET_BITS(reg_base + pin->offset,
				pin->io_ctrl.shift,
				pin->io_ctrl.width,
				val);
	MUTEX_UNLOCK();
}
void mt_pinctrl_set_io_ctrl(PINMUX_INDEX_E index, unsigned int val) __attribute((alias("pinctrl_set_io_ctrl")));

unsigned int pinctrl_get_sel(PINMUX_INDEX_E index)
{
	unsigned int val;

	DEF_PIN();
	ASSERT(index >= 0 && index < INDEX_PINMUX_MAX);
	GET_PIN(index);

	if (!pin) return 0;

	MUTEX_LOCK();
	val = MT_GET_BITS(reg_base + pin->offset,
						pin->sel.shift,
						pin->sel.width);
	MUTEX_UNLOCK();

	return val;
}
unsigned int mt_pinctrl_get_function(PINMUX_INDEX_E index) __attribute((alias("pinctrl_get_sel")));

void pinctrl_set_sel(PINMUX_INDEX_E index, unsigned int val)
{
	DEF_PIN();
	ASSERT(index >= 0 && index < INDEX_PINMUX_MAX);
	GET_PIN(index);

	if (!pin) return;

	MUTEX_LOCK();
	MT_SET_BITS(reg_base + pin->offset,
				pin->sel.shift,
				pin->sel.width,
				val);
	MUTEX_UNLOCK();
}
void mt_pinctrl_set_function(PINMUX_INDEX_E index, unsigned int val) __attribute((alias("pinctrl_set_sel")));

#if defined(__KERNEL__) && !defined(__UBOOT__)
static void dump_fmux(struct seq_file *s, const char *fmux_name[])
#else
static void dump_fmux(const char *fmux_name[])
#endif
{
	int i;

	if (fmux_name == NULL || fmux_name[0] == NULL)
		return;

	for (i=0; i<CFG_MAX_FMUX_COUNT; i++)
	{
		if (fmux_name[i] == NULL)
			break;

		DP_LOG(" %19s", fmux_name[i]);
	}
}

/* @Hide */
#if defined(__KERNEL__) && !defined(__UBOOT__)
void pinctrl_dump_state(struct seq_file *s)
#else
void pinctrl_dump_state(void)
#endif
{
	int i;
	DEF_PIN();

	DP_LOG("===================================================PINMUX"
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"\n");
	DP_LOG("PIN         NAME  OFF      VAL IO_CTRL VAL FUN_SEL VAL"
			"        FUNCTION - 0"
			"                   1"
			"                   2"
			"                   3"
			"                   4"
			"                   5"
			"                   6"
			"                   7\r\n"
			);
	for (i=0; i<INDEX_PINMUX_MAX; i++)
	{
		/* Patch */
		if (i >= INDEX_SW_PIN_CTRL_IN_MAX && i < INDEX_AO_PIN_CTRL0)
			continue;

		GET_PIN(i);

		if (pin)
		{
			DP_LOG("%3d %12s %04X %08X [%2d:%2d]%4X [%2d:%2d]%4X",
				pin->index,
				pin->name+4, /* ignore 'PAD_' */
				pin->offset,
				MT_IO_READ32(reg_base + pin->offset),
				CHECK_WIDTH(pin->io_ctrl.width)?(pin->io_ctrl.shift + pin->io_ctrl.width - 1):-1,
				CHECK_SHIFT(pin->io_ctrl.shift)?pin->io_ctrl.shift:-1,
				pinctrl_get_io_ctrl(i),
				CHECK_WIDTH(pin->sel.width)?(pin->sel.shift + pin->sel.width - 1):-1,
				CHECK_SHIFT(pin->sel.shift)?pin->sel.shift:-1,
				pinctrl_get_sel(i));

#if defined(__KERNEL__) && !defined(__UBOOT__)
			dump_fmux(s, pin->fmux_name);
#else
			dump_fmux(pin->fmux_name);
#endif
			DP_LOG("\r\n");
		}
	}
	DP_LOG("========================================================="
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"===================="
			"\n");
}

//---------------------------------------------------------------------------//

#if defined(__UBOOT__)

static int do_cmd_pinctrl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int pin_index;
	unsigned int val;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "set_io_ctrl") == 0)
	{
		if (argc < 4)
			return CMD_RET_USAGE;

		pin_index = (int)simple_strtol(argv[2], NULL, 0);
		val = (unsigned int)simple_strtoul(argv[3], NULL, 0);

		pinctrl_set_io_ctrl((PINMUX_INDEX_E)pin_index, val);
		printf("Write PIN_INDEX[%d] IO_CTRL: 0x%X\n", pin_index, val);
	}
	else if (strcmp(argv[1], "set_pin_sel") == 0
			|| strcmp(argv[1], "set_pin_fun") == 0)
	{
		if (argc < 4)
			return CMD_RET_USAGE;

		pin_index = (int)simple_strtol(argv[2], NULL, 0);
		val = (unsigned int)simple_strtoul(argv[3], NULL, 0);

		mt_pinctrl_set_function((PINMUX_INDEX_E)pin_index, val);
		printf("Write PIN_INDEX[%d] PIN_SEL: 0x%X\n", pin_index, val);
	}
	else if (strcmp(argv[1], "dump") == 0)
	{
		pinctrl_dump_state();
	}
	else
	{
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	pinctrl, 4, 0,	do_cmd_pinctrl,
	"PINMUX sub-system",
	"\npinctrl dump - dump pinmux state\n"
	"pinctrl set_io_ctrl pin_index value - set pin io control value\n"
	/*"pinctrl set_pin_sel pin_index value - set pin function mux value\n"*/
	"pinctrl set_pin_fun pin_index value - set pin function mux value\n"
);

#endif

