/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/atomic.h>
#include <linux/miscdevice.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>
//#include <linux/mtmedia.h>
#include <linux/clk.h>

#include "mt_type.h"
//#include "mt_i2c.h"
//#include "drv_i2c_ext.h"
//#include "drv_gpioi2c_ext.h"
#include "mt_debug.h"

#include "mt_drv_frontend.h"
#include "drv_frontend_ext.h"
//#include "drv_demod.h"
#include "drv_frontend_ioctl.h"

//#include "mt_unf_i2c.h"

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_module.h"
//#include "common_dev.h"
//#include "common_proc.h"
//#include "common_mem.h"
//#include "common_stat.h"
//#include "mt_common_id.h"
//#include "common_module_drv.h"
#include "mt_common.h"

extern mt_s32 mt_fe_open(struct inode *inode, struct file *filp);
extern mt_s32 mt_fe_release(struct inode *inode, struct file *filp);
#ifdef CONFIG_NET
extern long mt_fe_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
extern mt_s32 fe_proc_read(struct seq_file *p, mt_void *v);
extern mt_s32 fe_proc_read_reg(struct seq_file *p, mt_void *v);
extern mt_s32 fe_resume(basedev_s *pdev);
extern mt_s32 fe_suspend(basedev_s *pdev, pm_message_t state);

static struct file_operations mt_fe_fops =
    {
        .owner = THIS_MODULE,
#ifdef CONFIG_NET
        .unlocked_ioctl = mt_fe_ioctl,
#endif
        .open = mt_fe_open,
        .release = mt_fe_release,
    };

static baseops_s fe_baseops =
{
        .probe = NULL,
        .remove = NULL,
        .shutdown = NULL,
        .prepare = NULL,
        .complete = NULL,
        .suspend = fe_suspend,
        .suspend_late = NULL,
        .resume_early = NULL,
        .resume = fe_resume
};

static fe_priv_data_s fe_priv_data_info;

static mt_device_s mt_fe_dev =
{
        .devfs_name = UMAP_DEVNAME_TUNER,
        .minor = UMAP_MIN_MINOR_TUNER,
        .owner = THIS_MODULE,
        .fops = &mt_fe_fops,
        .drvops = &fe_baseops,
        .priv = &fe_priv_data_info,
};

mt_s32 __init fe_drv_modinit(mt_void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_proc_entry_t *fe_proc = NULL;
    mt_proc_entry_t *fe_reg_proc = NULL;
    atomic_set(&fe_priv_data_info.atmOpenCnt, 0);

    fe_priv_data_info.mclk = clk_get(NULL, "mclk");
    fe_priv_data_info.xtalclk = clk_get(NULL, "xtalclk");
    fe_priv_data_info.drvclk = clk_get(NULL, "drvclk");
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.mclk)))
        clk_prepare_enable(fe_priv_data_info.mclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.xtalclk)))
        clk_prepare_enable(fe_priv_data_info.xtalclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.drvclk)))
        clk_prepare_enable(fe_priv_data_info.drvclk);

/* pre */
#if 1//ndef MT_MCE_SUPPORT
    ret = mt_drv_fe_init();
    if (MT_SUCCESS != ret) {
	MT_ERR_FE("mt_drv_fe_init err! \n");
	return ret;
    }
#endif

    /* misc register tuner */
    ret = mt_drv_dev_register(&mt_fe_dev);
    if (MT_SUCCESS != ret) {
	MT_INFO_FE("frontend register failed!\n");
	ret = -EFAULT;
	goto err0;
    }

    /*proc*/
    fe_proc = mt_drv_proc_add_module("fe", NULL, NULL);
    if (!fe_proc) {
	MT_INFO_FE("add proc module failed\n");
	ret = MT_FAILURE;
	goto err1;
    }
    fe_proc->read = fe_proc_read;

    fe_reg_proc = mt_drv_proc_add_module("fe_reg", NULL, NULL);
    if (!fe_reg_proc) {
	MT_INFO_FE("add proc module failed\n");
	ret = MT_FAILURE;
	goto err1;
    }
    fe_reg_proc->read = fe_proc_read_reg;

    if (!(IS_ERR_OR_NULL(fe_priv_data_info.mclk)))
        clk_disable_unprepare(fe_priv_data_info.mclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.xtalclk)))
        clk_disable_unprepare(fe_priv_data_info.xtalclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.drvclk)))
        clk_disable_unprepare(fe_priv_data_info.drvclk);

    return MT_SUCCESS;

err1:
    mt_drv_module_unregister(MT_ID_FRONTEND);
    mt_drv_dev_unregister(&mt_fe_dev);
err0:
#ifndef MT_MCE_SUPPORT
    mt_drv_fe_deinit();
#endif
    return ret;
}

mt_void __exit fe_drv_modexit(mt_void)
{
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.mclk)))
        clk_prepare_enable(fe_priv_data_info.mclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.xtalclk)))
        clk_prepare_enable(fe_priv_data_info.xtalclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.drvclk)))
        clk_prepare_enable(fe_priv_data_info.drvclk);

    mt_drv_proc_rm_module("fe");
    mt_drv_proc_rm_module("fe_reg");
    mt_drv_module_unregister(MT_ID_FRONTEND);
    mt_drv_dev_unregister(&mt_fe_dev);

#if 1//ndef MT_MCE_SUPPORT
    mt_drv_fe_deinit();
#endif

    if (!(IS_ERR_OR_NULL(fe_priv_data_info.mclk))) {
        clk_disable_unprepare(fe_priv_data_info.mclk);
        clk_put(fe_priv_data_info.mclk);
    }
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.xtalclk))) {
        clk_disable_unprepare(fe_priv_data_info.xtalclk);
        clk_put(fe_priv_data_info.xtalclk);
    }
    if (!(IS_ERR_OR_NULL(fe_priv_data_info.drvclk))) {
        clk_disable_unprepare(fe_priv_data_info.drvclk);
        clk_put(fe_priv_data_info.drvclk);
    }

    return;
}

