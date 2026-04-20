/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : drv_crm_intf.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT CRM driver.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/printk.h>

#include "mt_drv_proc.h"
#include "mt_drv_clock.h"

extern void mt_mod_reset_help(struct seq_file *s);

static mt_void crm_proc_print_help(mt_void)
{
    mt_drv_proc_echohelp("echo reset mod > /proc/msp/crm, reset clock module\n");
    //mt_drv_proc_echohelp("echo clk=?,gate=[0|1],rate=?,mux=?,div=?,attr=? > /proc/msp/crm, batch config clock\n");
}

static mt_s32 crm_proc_read(struct seq_file *p, mt_void *v)
{
	mt_mod_reset_help(p);
	mt_clk_dump_state(p);

	return MT_SUCCESS;
}

static mt_s32 crm_proc_write(struct file *file,
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

		mt_clk_reset(mod);
	}
#if 0
	else if (strncmp(ProcPara, "clk=", 4) == 0)
	{
		mt_clk_cfg_cmdline(ProcPara);
	}
#endif
	else
	{
		crm_proc_print_help();
	}

    return count;
}

static int mt_drv_crm_init(void)
{
	int err = 0;
    mt_proc_entry_t *pProcItem;

	err = mt_clk_init();
	if (err != 0)
	{
		printk(KERN_ERR "%s: mt_clk_init() failed!\n", __FUNCTION__);
		return -ENODEV;
	}

	pProcItem = mt_drv_proc_add_module("crm", MT_NULL, MT_NULL);
	if (pProcItem == NULL)
	{
		printk(KERN_ERR "%s: mt_drv_proc_add_module(%s) failed!\n", __FUNCTION__, "crm");
		return -ENOENT;
	}

    pProcItem->read  = crm_proc_read;
    pProcItem->write = crm_proc_write;

	printk("mt_drv_crm_init success.\n");

	return err;
}

static void mt_drv_crm_deinit(void)
{
	mt_drv_proc_rm_module("crm");
}

static struct platform_driver mt_crm_driver = {
	.driver = {
		.name 	= "mt-crm",
		.owner	= THIS_MODULE,
	},
};

int __init mt_crm_mod_init(void)
{
	mt_drv_crm_init();

	return platform_driver_register(&mt_crm_driver);
}

void __exit mt_crm_mod_exit(void)
{
	mt_drv_crm_deinit();

	platform_driver_unregister(&mt_crm_driver);
}

