/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage LZ Technology Group Limited and its affiliated companies      */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : drv_analog_intf.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/08/11
 * Description    : MT Analog driver.
 * History        :
 * 1.Date         : 2022/08/11
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/delay.h>

#include "mt_drv_proc.h"
#include "mt_drv_analog.h"

extern int opt_temp_log;
extern int opt_temp_hack_degree;
extern int opt_hdmi_hack_27M;

extern void print_temperature(struct seq_file *s);

static mt_void analog_proc_print_help(mt_void)
{
    mt_drv_proc_echohelp("echo reset mod > /proc/msp/analog, reset analog module\n");
    mt_drv_proc_echohelp("echo enable mod > /proc/msp/analog, enable analog module\n");
    mt_drv_proc_echohelp("echo disable mod > /proc/msp/analog, disable analog module\n");
    mt_drv_proc_echohelp("echo temp_log on|off > /proc/msp/analog, temperature log on|off\n");
    mt_drv_proc_echohelp("echo temp_hack degree > /proc/msp/analog, hack temperature return degree\n");
    mt_drv_proc_echohelp("echo hdmi_hack_27M 0|54|108 > /proc/msp/analog, hack hdmi 27M to 54M, 108M\n");
}

static mt_s32 analog_proc_read(struct seq_file *p, mt_void *v)
{
	seq_printf(p, "opt:\n");
	seq_printf(p, "  temp_log - %d\n", opt_temp_log);
	seq_printf(p, "  temp_hack_degree - %d\n", opt_temp_hack_degree);
	seq_printf(p, "  hdmi_hack_27M - %d\n", opt_hdmi_hack_27M);

	print_temperature(p);

	mt_analog_dump_status(p);
	mt_analog_dump_state(p);

	return MT_SUCCESS;
}

static mt_s32 analog_proc_write(struct file *file,
    						const char __user *buf, size_t count, loff_t *ppos)
{
    char ProcPara[64] = {0};

    if (count >= sizeof(ProcPara) || copy_from_user(ProcPara, buf, count))
    {
        return -EFAULT;
    }

	strreplace(ProcPara, '\r', '\0');
	strreplace(ProcPara, '\n', '\0');

	if (strncmp(ProcPara, "reset", 5) == 0)
	{
		char *mod = &ProcPara[6];

		mt_analog_reset(mod);

		if (strcmp(mod, MT_ANA_TSENSOR) == 0)
			udelay(CFG_MT_ANA_RESET_UDELAY_MAX);
		else
			udelay(CFG_MT_ANA_RESET_UDELAY_MIN);

		mt_analog_release(mod);
	}
	else if (strncmp(ProcPara, "enable", 6) == 0)
	{
		char *mod = &ProcPara[7];

		mt_analog_enable(mod);
	}
	else if (strncmp(ProcPara, "disable", 7) == 0)
	{
		char *mod = &ProcPara[8];

		mt_analog_disable(mod);
	}
	else if (strncmp(ProcPara, "temp_log", 8) == 0)
	{
		char *onoff = &ProcPara[9];

		if (strncmp(onoff, "on", 2) == 0)
		{
			opt_temp_log = 1;
		}
		else if (strncmp(onoff, "off", 3) == 0)
		{
			opt_temp_log = 0;
		}
	}
	else if (strncmp(ProcPara, "temp_hack", 9) == 0)
	{
		char *degree = &ProcPara[10];
		long res = 0;

		if (0 == kstrtol(degree, 0, &res))
			opt_temp_hack_degree = (int)res;
	}
	else if (strncmp(ProcPara, "hdmi_hack_27M", 13) == 0)
	{
		char *sz_hack = &ProcPara[14];
		long res = 0;

		if (0 == kstrtol(sz_hack, 0, &res))
			opt_hdmi_hack_27M = (int)res;
	}
	else
	{
		analog_proc_print_help();
	}

    return count;
}

static int mt_drv_analog_init(void)
{
	int err = 0;
    mt_proc_entry_t *pProcItem;

	pProcItem = mt_drv_proc_add_module("analog", MT_NULL, MT_NULL);
	if (pProcItem == NULL)
	{
		printk(KERN_ERR "%s: mt_drv_proc_add_module(%s) failed!\n", __FUNCTION__, "analog");
		return -ENOENT;
	}

    pProcItem->read  = analog_proc_read;
    pProcItem->write = analog_proc_write;

	p_mt_analog_get_temperature = mt_analog_get_temperature;
	p_mt_analog_get_temperature_async = mt_analog_get_temperature_async;

	mt_tsensor_init();

	printk("mt_drv_analog_init success.\n");

	return err;
}

static void mt_drv_analog_deinit(void)
{
	mt_tsensor_deinit();

	mt_drv_proc_rm_module("analog");
}

static struct platform_driver mt_analog_driver = {
	.driver = {
		.name 	= "mt-analog",
		.owner	= THIS_MODULE,
	},
};

int __init mt_analog_mod_init(void)
{
	mt_drv_analog_init();

	return platform_driver_register(&mt_analog_driver);
}

void __exit mt_analog_mod_exit(void)
{
	mt_drv_analog_deinit();

	platform_driver_unregister(&mt_analog_driver);
}

