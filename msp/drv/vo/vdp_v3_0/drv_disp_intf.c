/******************************************************************************

  Copyright (C), 2017, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : drv_disp_intf.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/01/15
  Description   :
  History       :
  1.Date        : 2017/01/20
  Author        :
  Modification  : Created file

*******************************************************************************/
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
#include <linux/smp_lock.h>
#endif
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/clk.h>

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_kernel_adapt.h"
#include "mt_module_debug.h"

#include "mt_drv_video.h"
#include "mt_drv_disp.h"

#include "mt_drv_disp.h"
#include "drv_disp_ext.h"
#include "drv_display.h"
#include "drv_disp_debug.h"
#include "drv_disp.h"
#include "drv_disp_osal.h"
#include "mt_osal.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

int disp_dbg_enable = 0;
module_param_named(disp_dbg_enable, disp_dbg_enable, int, 0644);
EXPORT_SYMBOL(disp_dbg_enable);


extern mt_s32  DISP_FileOpen(struct inode *finode, struct file  *ffile);
extern mt_s32  DISP_FileClose(struct inode *finode, struct file  *ffile);
extern mt_s32  DRV_DISP_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg);

extern mt_s32  DRV_DISP_Init(mt_void);
extern mt_s32  DRV_DISP_DeInit(mt_void);
extern mt_s32  DRV_DISP_Suspend(basedev_s *pdev, pm_message_t state);
extern mt_s32  DRV_DISP_Resume(basedev_s *pdev);

extern mt_void DRV_WIN_UnRegister(mt_void);
extern mt_s32  DRV_WIN_Register(mt_void);
extern mt_void WIN_ModExit(mt_void);
extern mt_s32  WIN_ModInit(mt_void);

static mt_device_s g_DispRegisterData;
static disp_priv_data disp_priv_data_info;

static long DISP_FileIoctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    long Ret;

    Ret = (long)mt_drv_usercopy(ffile->f_path.dentry->d_inode,ffile, cmd, arg, DRV_DISP_Ioctl);

    return Ret;
}

static struct file_operations s_DispFileOps =
{
    .owner = THIS_MODULE,
    .open           = DISP_FileOpen,
    .unlocked_ioctl = DISP_FileIoctl,
    .release        = DISP_FileClose,
};

static baseops_s s_DispDasicOps =
{
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = DRV_DISP_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = DRV_DISP_Resume,
};

mt_void DISP_ModExit(mt_void)
{

    DRV_DISP_UnRegister();

    mt_drv_dev_unregister(&g_DispRegisterData);

    return;
}

mt_s32 DISP_ModInit(mt_void)
{
    /* Regist Disp device */
    mt_osal_snprintf(g_DispRegisterData.devfs_name,sizeof(g_DispRegisterData.devfs_name),"%s", UMAP_DEVNAME_DISP);
    g_DispRegisterData.fops   = &s_DispFileOps;
    g_DispRegisterData.minor  = UMAP_MIN_MINOR_DISP;
    g_DispRegisterData.owner  = THIS_MODULE;
    g_DispRegisterData.drvops = &s_DispDasicOps;
    g_DispRegisterData.priv = &disp_priv_data_info;

    if (mt_drv_dev_register(&g_DispRegisterData) < 0)
    {
        MT_FATAL_DISP("register DISP failed.\n");
        return MT_FAILURE;
    }

    if (MT_SUCCESS != DRV_DISP_Register())
    {
        //if DISP init failed, exit
        mt_drv_dev_unregister(&g_DispRegisterData);

        MT_FATAL_DISP("DRV_DISP_Init failed.\n");
        return MT_FAILURE;
    }


    return MT_SUCCESS;
}

extern mt_s32 DRV_DISP_Init2(mt_void);
extern mt_s32 DRV_DISP_DeInit2(mt_void);

mt_s32 __init VDP_DRV_ModInit(mt_void)
{
    MT_INFO_DISP("VDP_DRV_ModInit int\n");
    disp_priv_data_info.disclk = clk_get(NULL, "disclk");
    disp_priv_data_info.diclk = clk_get(NULL, "diclk");
    disp_priv_data_info.osdclk = clk_get(NULL, "osdclk");
    disp_priv_data_info.presclk = clk_get(NULL, "presclk");
    disp_priv_data_info.disaxiclk = clk_get(NULL, "disaxiclk");
    disp_priv_data_info.hdclk = clk_get(NULL, "hdclk");
    disp_priv_data_info.sdclk_27m = clk_get(NULL, "sdclk_27m");
    if (!IS_ERR_OR_NULL(disp_priv_data_info.disclk))
        clk_prepare_enable(disp_priv_data_info.disclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.diclk))
        clk_prepare_enable(disp_priv_data_info.diclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.osdclk))
        clk_prepare_enable(disp_priv_data_info.osdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.presclk))
        clk_prepare_enable(disp_priv_data_info.presclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.disaxiclk))
        clk_prepare_enable(disp_priv_data_info.disaxiclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.hdclk))
        clk_prepare_enable(disp_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.sdclk_27m))
        clk_prepare_enable(disp_priv_data_info.sdclk_27m);

    MT_INFO_DISP("display module init \n");
    if(MT_SUCCESS != DISP_ModInit())
    {
        MT_FATAL_DISP("DISP_ModInit failed.\n");
        return MT_FAILURE;
    }

    if(MT_SUCCESS != WIN_ModInit())
    {
        MT_FATAL_DISP("WIN_ModInit failed.\n");
        DISP_ModExit();
        return MT_FAILURE;
    }

    DRV_DISP_Init2();

    if (!IS_ERR_OR_NULL(disp_priv_data_info.disclk))
        clk_disable_unprepare(disp_priv_data_info.disclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.diclk))
        clk_disable_unprepare(disp_priv_data_info.diclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.osdclk))
        clk_disable_unprepare(disp_priv_data_info.osdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.presclk))
        clk_disable_unprepare(disp_priv_data_info.presclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.disaxiclk))
        clk_disable_unprepare(disp_priv_data_info.disaxiclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.hdclk))
        clk_disable_unprepare(disp_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.sdclk_27m))
        clk_disable_unprepare(disp_priv_data_info.sdclk_27m);

    MT_INFO_DISP("VDP_DRV_ModInit exit\n");
    return MT_SUCCESS;
}


mt_void __exit VDP_DRV_ModExit(mt_void)
{
    if (!IS_ERR_OR_NULL(disp_priv_data_info.disclk))
        clk_prepare_enable(disp_priv_data_info.disclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.diclk))
        clk_prepare_enable(disp_priv_data_info.diclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.osdclk))
        clk_prepare_enable(disp_priv_data_info.osdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.presclk))
        clk_prepare_enable(disp_priv_data_info.presclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.disaxiclk))
        clk_prepare_enable(disp_priv_data_info.disaxiclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.hdclk))
        clk_prepare_enable(disp_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data_info.sdclk_27m))
        clk_prepare_enable(disp_priv_data_info.sdclk_27m);

    DRV_DISP_DeInit2();

    WIN_ModExit();

    DISP_ModExit();

    if (!IS_ERR_OR_NULL(disp_priv_data_info.disclk)) {
        clk_disable_unprepare(disp_priv_data_info.disclk);
        clk_put(disp_priv_data_info.disclk);
    }
    if (!IS_ERR_OR_NULL(disp_priv_data_info.diclk)) {
        clk_disable_unprepare(disp_priv_data_info.diclk);
        clk_put(disp_priv_data_info.diclk);
    }
    if (!IS_ERR_OR_NULL(disp_priv_data_info.osdclk)) {
        clk_disable_unprepare(disp_priv_data_info.osdclk);
        clk_put(disp_priv_data_info.osdclk);
    }
    if (!IS_ERR_OR_NULL(disp_priv_data_info.presclk)) {
        clk_disable_unprepare(disp_priv_data_info.presclk);
        clk_put(disp_priv_data_info.presclk);
    }
    if (!IS_ERR_OR_NULL(disp_priv_data_info.disaxiclk)) {
        clk_disable_unprepare(disp_priv_data_info.disaxiclk);
        clk_put(disp_priv_data_info.disaxiclk);
    }
    if (!IS_ERR_OR_NULL(disp_priv_data_info.hdclk)) {
        clk_disable_unprepare(disp_priv_data_info.hdclk);
        clk_put(disp_priv_data_info.hdclk);
    }
    if (!IS_ERR_OR_NULL(disp_priv_data_info.sdclk_27m)) {
        clk_disable_unprepare(disp_priv_data_info.sdclk_27m);
        clk_put(disp_priv_data_info.sdclk_27m);
    }

    return;
}

// mzhu_fpga
//module_init(VDP_DRV_ModInit);
//module_exit(VDP_DRV_ModExit);

MODULE_AUTHOR("MONTAGE");
MODULE_LICENSE("GPL");

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
