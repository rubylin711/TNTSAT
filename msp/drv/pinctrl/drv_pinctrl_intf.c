/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage LZ Technology Group Limited and its affiliated companies      */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : drv_pinctrl_intf.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/07/04
 * Description    : MT Pinctrl driver.
 * History        :
 * 1.Date         : 2022/07/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/printk.h>

#include "mach/symphony_reg_base_addr.h"

#include "mt_drv_proc.h"
#include "mt_drv_pinctrl.h"
#include "pinctrl.h"

extern int parse_cmdline(int *argc, char **argv, const char *cmd_line);

static mt_void pinctrl_proc_print_help(mt_void)
{
    mt_drv_proc_echohelp("echo set_io_ctrl pin value > /proc/msp/pinctrl, set pin io control value\n");
    mt_drv_proc_echohelp("echo set_pin_fun pin value > /proc/msp/pinctrl, set pin function select value\n");
}

static mt_s32 pinctrl_proc_read(struct seq_file *p, mt_void *v)
{
	pinctrl_dump_state(p);

	return MT_SUCCESS;
}

static mt_s32 pinctrl_proc_write(struct file *file,
    							 const char __user *buf, size_t count, loff_t *ppos)
{
	int index = 0;
	unsigned int value = 0;
    char ProcPara[128] = {0};
    int argc;
    char *argv[64];

    if (count >= sizeof(ProcPara) || copy_from_user(ProcPara, buf, count))
    {
        return -EFAULT;
    }

	strreplace(ProcPara, '\r', '\0');
	strreplace(ProcPara, '\n', '\0');

	if (parse_cmdline(&argc, &argv[0], ProcPara) < 0)
        return -EFAULT;

	if (argc >= 2)
	{
		index = (int)simple_strtol(argv[1], NULL, 0);
	}

	if (argc >= 3)
	{
		value = (unsigned int)simple_strtoul(argv[2], NULL, 0);
	}

	if (strcmp(argv[0], "set_io_ctrl") == 0)
	{
		if (argc < 3)
			return -EFAULT;

		mt_pinctrl_set_io_ctrl((PINMUX_INDEX_E)index, value);
		printk("Write PIN_INDEX[%d] IO_CTRL: 0x%X\n", index, value);
	}
	else if ((strcmp(argv[0], "set_pin_sel") == 0)
		 || (strcmp(argv[0], "set_pin_fun") == 0))
	{
		if (argc < 3)
			return -EFAULT;

		mt_pinctrl_set_function((PINMUX_INDEX_E)index, value);
		printk("Write PIN_INDEX[%d] PIN_FUN: 0x%X\n", index, value);
	}
	else
	{
		pinctrl_proc_print_help();
	}

    return count;
}

static int mt_drv_pinctrl_init(void)
{
	int err = 0;
    mt_proc_entry_t *pProcItem;

	pProcItem = mt_drv_proc_add_module("pinctrl", MT_NULL, MT_NULL);
	if (pProcItem == NULL)
	{
		printk(KERN_ERR "%s: mt_drv_proc_add_module(%s) failed!\n", __FUNCTION__, "pinctrl");
		return -ENOENT;
	}

    pProcItem->read  = pinctrl_proc_read;
    pProcItem->write = pinctrl_proc_write;

	printk("mt_drv_pinctrl_init success.\n");

	return err;
}

static void mt_drv_pinctrl_deinit(void)
{
	mt_drv_proc_rm_module("pinctrl");
}

static struct platform_driver mt_pinctrl_driver = {
	.driver = {
		.name 	= "mt-pinctrl",
		.owner	= THIS_MODULE,
	},
};

int __init mt_pinctrl_mod_init(void)
{
	mt_drv_pinctrl_init();

	return platform_driver_register(&mt_pinctrl_driver);
}

void __exit mt_pinctrl_mod_exit(void)
{
	mt_drv_pinctrl_deinit();

	platform_driver_unregister(&mt_pinctrl_driver);
}

