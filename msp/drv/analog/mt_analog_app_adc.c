/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Montage LZ Co., Ltd.
 */
#if defined(__UBOOT__)
#include <common.h>
#include <command.h>

#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#include <asm/arch-symphony6/mt_analog_parameter.h>
#include <asm/arch-symphony6/mt_analog_app.h>

#include "mt_crm_clock_reg.h"

#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/timekeeping.h>

/* FIXME */
#include "mt_common.h"
#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "../crm/mt_crm_clock_reg.h"

#include "mt_drv_analog.h"
#include "analog/mt_analog_parameter.h"
#include "analog/mt_analog_app.h"

#include "mt_drv_clock.h"
#else
/* RTOS */
#include <sys_types.h>
#include <sys_define.h>

#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "../crm/mt_crm_clock_reg.h"

#include "mt_drv_analog.h"
#include "analog/mt_analog_parameter.h"
#include "analog/mt_analog_app.h"

#include "mt_drv_clock.h"
#endif

#include "mt_analog_reg.h"

#if defined(__UBOOT__)
//FIXME?
//#define SPIN_LOCK()					disable_interrupts()
//#define SPIN_UNLOCK()				enable_interrupts()
#define SPIN_LOCK()					do{}while(0)
#define SPIN_UNLOCK()				do{}while(0)
#elif defined(__KERNEL__)
static DEFINE_SPINLOCK(ana_app_lock);

#if 0

/* mt_hdmi_analog_config called in ISR! */

#if 1
/* multi core */
#define SPIN_LOCK()					spin_lock(&ana_app_lock)
#define SPIN_UNLOCK()				spin_unlock(&ana_app_lock)
#else
/* single core */
#define SPIN_LOCK()					do{}while(0)
#define SPIN_UNLOCK()				do{}while(0)
#endif

#else
/* mt_hdmi_analog_config NOT called in ISR! */

static unsigned long flags;
#define SPIN_LOCK()					spin_lock_irqsave(&ana_app_lock, flags)
#define SPIN_UNLOCK()				spin_unlock_irqrestore(&ana_app_lock, flags)
#endif

#else
/* RTOS */
/* TODO: */
#define SPIN_LOCK()					do{}while(0)
#define SPIN_UNLOCK()				do{}while(0)
#endif

/* for optimize size */
/*#define CFG_ANALOG_OPT_SIZE*/

#ifndef CFG_ANALOG_OPT_SIZE

static struct adc_param sym6_adc_params[] =
{
/*    .pd_sadc_1100M .pd_sdemod_270M .sadc_clk_sel  .pd_cadc_270M .pd_cdemod_81M .cadc_clk_sel .pd_drv0_clk .drv0_clk_sel .drv0_clk_mux .drv0_clk_81M_mux .pd_drv1_clk .drv1_clk_sel */
	/* S-demod(<45Msps)(960M) + C-demod(144M), S-demod(960M), C-demod(144M) */
	{{0,             0,              0,/*960M*/},  {0,            0,             0,/*144M*/}, {0,           0,/*57.6M*/   0,            0,},             {0,           0,/*192M*/}},

	/* S-demod(>45Msps)(1350M) + C-demod(144M) */
	{{0,             0,              1,/*1100M*/}, {0,            0,             0,/*144M*/}, {0,           0,/*57.6M*/   1,/*81M*/     0,},             {0,           0,/*192M*/}},

	/* S-demod(1350M/1080M) */
	{{0,             0,              1,/*1100M*/}, {0,            0,             0,/*144M*/}, {0,           0,/*57.6M*/   0,            0,},             {0,           0,/*192M*/}},

	/* J83.B-demod(270M) */
	{{0,             0,              1,/*1100M*/}, {0,            0,             1,/*270M*/}, {0,           0,/*57.6M*/   1,/*81M*/     0,},             {0,           0,/*192M*/}},
};

static void do_adc_analog_config(struct adc_param *param)
{
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_ADC, (void*)param);
}

/* Debug */
#if 0
static const char *get_adc_cfg_str(enum MT_ADC_ANALOG_CFG_E idx)
{
	static const char *str_cfg[] = {
							"S-demod(960M)/C-demod(144M)/S-demod(960M)+C-demod(144M)",
							"S-demod(1350M)+C-demod(144M)",
							"S-demod(1350M/1080M)",
							"J83.B-demod(270M)"
							};

	if (idx < ADC_ANALOG_CFG_SDEMOD_960 || idx >= ADC_ANALOG_CFG_MAX)
		return NULL;

	return str_cfg[idx];
}
#endif

int mt_adc_analog_config(enum MT_ADC_ANALOG_CFG_E idx)
{
	if (idx < ADC_ANALOG_CFG_SDEMOD_960 || idx >= ADC_ANALOG_CFG_MAX)
	{
		MT_LOGE("error: %s, invalid config(%d)!\r\n", __FUNCTION__, idx);
		return (-22);
	}

	MT_LOGI("%s: idx %d(%s)\n", __FUNCTION__, idx, get_adc_cfg_str(idx));

	SPIN_LOCK();

	do_adc_analog_config(&sym6_adc_params[idx]);

	SPIN_UNLOCK();

	return 0;
}

#if defined(__UBOOT__)
static int do_cmd_adc_analog(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	int index = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	index = simple_strtoul(argv[1], NULL, 0);

	ret = mt_adc_analog_config((enum MT_ADC_ANALOG_CFG_E)index);

	if (ret == 0) {
		return CMD_RET_SUCCESS;
	} else {
		return CMD_RET_FAILURE;
	}
}

U_BOOT_CMD(
	mt_adc_analog_cfg, 2, 0, do_cmd_adc_analog,
	"ADC Analog Configure",
	"\nmt_adc_analog_cfg index - adc analog configure\n"
	"                          index: configuration index\n"
	"                              0: S-demod(960M)/C-demod(144M)/S-demod(960M)+C-demod(144M)\n"
	"                              1: S-demod(1350M)+C-demod(144M)\n"
	"                              2: S-demod(1350M/1080M)\n"
	"                              3: J83.B-demod(270M)\n"
);
#endif

#endif //CFG_ANALOG_OPT_SIZE

