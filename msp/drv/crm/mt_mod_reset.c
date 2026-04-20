/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_mod_reset.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT clock module reset functions.
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
#include <asm/arch-symphony6/mt_drv_analog.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_log.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"
#else
/* RTOS */
#include <string.h>

#include <sys_types.h>
#include <sys_define.h>

#include "mtos_misc.h"

#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_log.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"
#endif

#include "mt_mod_reset.h"

//TODO: delay or sleep?
#if defined(__UBOOT__) || defined(__KERNEL__)
#define RST_UDELAY						udelay
#else
/* RTOS */
#define RST_UDELAY						mtos_task_delay_us
#endif

#ifdef __UBOOT__
//TODO: disable interrupts? disable task scheduler?
#define RESET_ENTER_CRITICAL()			do{}while(0)
#define RESET_LEAVE_CRITICAL()			do{}while(0)
#elif defined(__KERNEL__)

#include <linux/spinlock.h>

static DEFINE_SPINLOCK(rst_lock);
static unsigned long flags;

#define RESET_ENTER_CRITICAL()			spin_lock_irqsave(&rst_lock, flags)
#define RESET_LEAVE_CRITICAL()			spin_unlock_irqrestore(&rst_lock, flags)
#else
/* RTOS */
static u32 flags;
#define RESET_ENTER_CRITICAL()			do{mtos_critical_enter(&flags);}while(0)
#define RESET_LEAVE_CRITICAL()			do{mtos_critical_exit(flags);}while(0)
#endif

/* reset device */
struct mt_reset_dev
{
	void (*get_status)(void *pMod, int *status);
	void (*reset)(void *pMod);
	void (*release)(void *pMod);

	/* reset and release process */
	void (*reset_proc)(void *pMod, unsigned long usec_hold);

	/* module ptr */
	void *pMod;
};

static struct mt_mod_rst_simple *get_module_by_name_simple(const char *name)
{
	u32 i;
	u32 count = mt_rst_modules_count;

	for (i=0; i<count; i++)
	{
		if (strcmp(name, mt_rst_modules[i].name) == 0)
		{
			MT_LOGD("%s: found %s -> %d\n", __FUNCTION__, name, i);
			return &mt_rst_modules[i];
		}
	}

	//MT_LOGE("%s: not found %s!\n", __FUNCTION__, name);
	return NULL;
}

static struct mt_mod_rst_composite *get_module_by_name_composite(const char *name)
{
	u32 i;
	u32 count = mt_com_rst_modules_count;

	for (i=0; i<count; i++)
	{
		if (strcmp(name, mt_com_rst_modules[i].name) == 0)
		{
			MT_LOGD("%s: found %s -> %d\n", __FUNCTION__, name, i);
			return &mt_com_rst_modules[i];
		}
	}

	//MT_LOGE("%s: not found %s!\n", __FUNCTION__, name);
	return NULL;
}

static void __mod_reset_simple_get_status(void *para, int *status)
{
	struct mt_mod_rst_simple *pMod = (struct mt_mod_rst_simple *)para;

	if (VALID_IO_OBJ((mt_io_obj_t*)&pMod->core))
	{
		*status = (int)mt_read_obj((mt_io_obj_t*)&pMod->core);

		if (pMod->active_low)
			*status = (*status==0)?1:0;
	}

	return;
}

//static void __mod_reset_simple(struct mt_mod_rst_simple *pMod)
static void __mod_reset_simple(void *para)
{
	u32 rst_val;
	struct mt_mod_rst_simple *pMod = (struct mt_mod_rst_simple *)para;

	if (pMod->name) {
		MT_LOGV("%s: reset mod %s, do NOT access any more!\n", __FUNCTION__, pMod->name);
	}

	if (pMod->active_low)
		rst_val = 0;
	else
		rst_val = 1;

	if (VALID_IO_OBJ((mt_io_obj_t*)&pMod->core))
		mt_write_objlk(&pMod->core, rst_val);

	return;
}

static void __mod_reset_composite_get_status(void *para, int *status)
{
	int stat = 0;
	struct mt_mod_rst_composite *pMod = (struct mt_mod_rst_composite *)para;

	*status = 0;

	if (pMod->core) {
		stat = 0;
		__mod_reset_simple_get_status(pMod->core, &stat);
		*status |= stat;
	}

	if (pMod->ahb) {
		stat = 0;
		__mod_reset_simple_get_status(pMod->ahb, &stat);
		*status |= stat;
	}

	if (pMod->axi) {
		stat = 0;
		__mod_reset_simple_get_status(pMod->axi, &stat);
		*status |= stat;
	}

	return;
}

//static void __mod_reset_composite(struct mt_mod_rst_composite *pMod)
static void __mod_reset_composite(void *para)
{
	struct mt_mod_rst_composite *pMod = (struct mt_mod_rst_composite *)para;

	if (pMod->name) {
		MT_LOGV("%s: reset mod %s, do NOT access any more!\n", __FUNCTION__, pMod->name);
	}

	/* ahb/axi first */
	if (pMod->ahb)
		__mod_reset_simple(pMod->ahb);
	if (pMod->axi)
		__mod_reset_simple(pMod->axi);

	/* core last */
	if (pMod->core)
		__mod_reset_simple(pMod->core);

	/* reg/phy/ibus/ext1 */
	if (pMod->ext1)
		__mod_reset_simple(pMod->ext1);

	if (pMod->ext2)
		__mod_reset_simple(pMod->ext2);

	return;
}

//static void __mod_release_simple(struct mt_mod_rst_simple *pMod)
static void __mod_release_simple(void *para)
{
	u32 clr_val;
	struct mt_mod_rst_simple *pMod = (struct mt_mod_rst_simple *)para;

	if (pMod->active_low)
		clr_val = 1;
	else
		clr_val = 0;

	if (VALID_IO_OBJ((mt_io_obj_t*)&pMod->core))
		mt_write_objlk(&pMod->core, clr_val);

	/* wait reset complete and stable(10 cycle at least) */
	RST_UDELAY(CFG_MOD_SFT_RST_WAIT_US);

	if (pMod->name) {
		MT_LOGV("%s: release complete, you can access %s now!\n", __FUNCTION__, pMod->name);
	}

	return;
}

//static void __mod_release_composite(struct mt_mod_rst_composite *pMod)
static void __mod_release_composite(void *para)
{
	struct mt_mod_rst_composite *pMod = (struct mt_mod_rst_composite *)para;

	if (pMod->ext2)
		__mod_release_simple(pMod->ext2);

	/* reg/phy/ibus/ext1 */
	if (pMod->ext1)
		__mod_release_simple(pMod->ext1);

	/* core fist */
	if (pMod->core)
		__mod_release_simple(pMod->core);

	/* ahb/axi last */
	if (pMod->ahb)
		__mod_release_simple(pMod->ahb);
	if (pMod->axi)
		__mod_release_simple(pMod->axi);

	if (pMod->name) {
		MT_LOGV("%s: release complete, you can access %s now!\n", __FUNCTION__, pMod->name);
	}

	return;
}

//static void __mod_reset_proc_simple(struct mt_mod_rst_simple *pMod, unsigned long usec_hold)
static void __mod_reset_proc_simple(void *para, unsigned long usec_hold)
{
	struct mt_mod_rst_simple *pMod = (struct mt_mod_rst_simple *)para;

	__mod_reset_simple(pMod);

	/* hold reset */
	if (usec_hold == 0)
		usec_hold = pMod->delay_us;

	if (usec_hold == 0)
		usec_hold = CFG_MOD_SFT_RST_HOLD_US;

	if (usec_hold != 0)
		RST_UDELAY(usec_hold);

	__mod_release_simple(pMod);
}

//static void __mod_reset_proc_composite(struct mt_mod_rst_composite *pMod, unsigned long usec_hold)
static void __mod_reset_proc_composite(void *para, unsigned long usec_hold)
{
	struct mt_mod_rst_composite *pMod = (struct mt_mod_rst_composite *)para;

	__mod_reset_composite(pMod);

	/* hold reset */
	if (usec_hold == 0 && pMod->core)
		usec_hold = pMod->core->delay_us;

	if (usec_hold == 0)
		usec_hold = CFG_MOD_SFT_RST_HOLD_US;

	if (usec_hold != 0)
		RST_UDELAY(usec_hold);

	__mod_release_composite(pMod);
}

/* Simple Reset Device */
static struct mt_reset_dev simple_reset_dev =
{
	.get_status = __mod_reset_simple_get_status,
	.reset      = __mod_reset_simple,
	.release    = __mod_release_simple,
	.reset_proc = __mod_reset_proc_simple,

	.pMod       = NULL
};

/* Composite Reset Device */
static struct mt_reset_dev composite_reset_dev =
{
	.get_status = __mod_reset_composite_get_status,
	.reset      = __mod_reset_composite,
	.release    = __mod_release_composite,
	.reset_proc = __mod_reset_proc_composite,

	.pMod       = NULL
};

static struct mt_reset_dev *get_dev_by_name(const char *name)
{
	void *pMod = NULL;

	pMod = get_module_by_name_simple(name);
	if (pMod) {
		/* FIXME: single simple reset device instance, NOT support reentry!!! */
		simple_reset_dev.pMod = pMod;
		return &simple_reset_dev;
	}

	pMod = get_module_by_name_composite(name);
	if (pMod) {
		/* FIXME: single composite reset device instance, NOT support reentry!!! */
		composite_reset_dev.pMod = pMod;
		return &composite_reset_dev;
	}

	//MT_LOGE("%s: not found %s!\n", __FUNCTION__, name);
	return NULL;
}

int mt_mod_get_reset_status(mt_mod_t *mod, int *status)
{
	int ret = 0;
	struct mt_reset_dev *dev = NULL;

	if (status == NULL) {
		return (-22);
	}

	*status = -1;

	/*
	 * FIXME: disable interrupts, disable task scheduler?
	 */
	RESET_ENTER_CRITICAL();

	dev = get_dev_by_name(mod);
	if (dev == NULL) {
		ret = -1;
		goto RET;
	}

	dev->get_status(dev->pMod, status);

RET:
	RESET_LEAVE_CRITICAL();

	return ret;
}

int mt_clk_get_reset_status(mt_clk_t *clk, int *status) __attribute__((alias("mt_mod_get_reset_status")));
int mt_analog_get_reset_status(mt_ana_t *ana, int *status) __attribute__((alias("mt_mod_get_reset_status")));

int mt_mod_rst_reset(mt_mod_t *mod)
{
	int ret = 0;
	struct mt_reset_dev *dev = NULL;

	/*
	 * FIXME: disable interrupts, disable task scheduler?
	 */
	RESET_ENTER_CRITICAL();

	dev = get_dev_by_name(mod);
	if (dev == NULL) {
		ret = -1;
		goto RET;
	}

	dev->reset(dev->pMod);

RET:
	RESET_LEAVE_CRITICAL();

	return ret;
}

int mt_mod_rst_release(mt_mod_t *mod)
{
	int ret = 0;
	struct mt_reset_dev *dev = NULL;

	/*
	 * FIXME: disable interrupts, disable task scheduler?
	 */
	RESET_ENTER_CRITICAL();

	dev = get_dev_by_name(mod);
	if (dev == NULL) {
		ret = -1;
		goto RET;
	}

	dev->release(dev->pMod);

RET:
	RESET_LEAVE_CRITICAL();

	return ret;
}

int mt_mod_reset(mt_mod_t *mod)
{
	int ret = 0;
	struct mt_reset_dev *dev = NULL;

	/*
	 * FIXME: disable interrupts, disable task scheduler?
	 */
	RESET_ENTER_CRITICAL();

	dev = get_dev_by_name(mod);
	if (dev == NULL) {
		ret = -1;
		goto RET;
	}

	dev->reset_proc(dev->pMod, 0);

RET:
	RESET_LEAVE_CRITICAL();

	return ret;
}

/* Module Soft Reset when Exception(Abnormal) */
int mt_mod_reset_exp(mt_mod_t *mod)
{
	int ret = 0;
	struct mt_reset_dev *dev = NULL;

	/*
	 * FIXME: disable interrupts, disable task scheduler?
	 */
	RESET_ENTER_CRITICAL();

	dev = get_dev_by_name(mod);
	if (dev == NULL) {
		ret = -1;
		goto RET;
	}

	dev->reset_proc(dev->pMod, CFG_MOD_EXP_SFT_RST_HOLD_US);

RET:
	RESET_LEAVE_CRITICAL();

	return ret;
}

/*@Hide*/
#if defined(__UBOOT__)
void mt_mod_reset_help(void)
{
	int i;
	int status;

	printf("-----------RESET-----------\n");
	printf("%15s  %7s\n", "Module", "Status");

	for (i=0; i<mt_rst_modules_count; i++)
	{
		mt_mod_get_reset_status((mt_mod_t*)mt_rst_modules[i].name, &status);

		printf("%15s  %7s\n", mt_rst_modules[i].name, status?"Reset":"Release");
	}
	for (i=0; i<mt_com_rst_modules_count; i++)
	{
		mt_mod_get_reset_status((mt_mod_t*)mt_com_rst_modules[i].name, &status);

		printf("%15s  %7s\n", mt_com_rst_modules[i].name, status?"Reset":"Release");
	}
}
#elif defined(__KERNEL__)
void mt_mod_reset_help(struct seq_file *s)
{
	int i;
	int status;

	seq_printf(s, "-----------RESET-----------\n");
	seq_printf(s, "%15s  %7s\n", "Module", "Status");

	for (i=0; i<mt_rst_modules_count; i++)
	{
		mt_mod_get_reset_status((mt_mod_t*)mt_rst_modules[i].name, &status);

		seq_printf(s, "%15s  %7s\n", mt_rst_modules[i].name, status?"Reset":"Release");
	}
	for (i=0; i<mt_com_rst_modules_count; i++)
	{
		mt_mod_get_reset_status((mt_mod_t*)mt_com_rst_modules[i].name, &status);

		seq_printf(s, "%15s  %7s\n", mt_com_rst_modules[i].name, status?"Reset":"Release");
	}
}
#endif

//---------------------------------------------------------------------------//

#if defined(__UBOOT__)

static int do_cmd_mod_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;

	if (argc < 2
		|| strcmp(argv[1], "help") == 0
		|| strcmp(argv[1], "--help") == 0
		|| strcmp(argv[1], "-h") == 0)
	{
		printf("Usage:\n");
		printf("mt_reset module - reset module by name\n");

		mt_mod_reset_help();
	}
	else
	{
		ret = mt_mod_reset(argv[1]);
		printf("mt_reset %s - %s\n", argv[1], ret==0?"Success.":"Failed!");
	}

	if (ret == 0)
		return CMD_RET_SUCCESS;
	else
		return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	mt_reset, 2, 0,	do_cmd_mod_reset,
	"reset Analog/PLL/Clock module",
	"\nmt_reset module - reset module by name\n"
	"mt_reset -h|--help|help - help\n"
);

#endif

