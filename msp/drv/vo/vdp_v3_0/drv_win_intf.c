/******************************************************************************

  Copyright (C), 2017, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : drv_win_intf.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/01/20
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
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>

#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_kernel_adapt.h"


//#include "mpi_priv_vi.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"

#include "mt_drv_win.h"
#include "drv_disp.h"
#include "drv_disp_osal.h"

#include "drv_window.h"
#include "drv_win_ext.h"
#include "mt_osal.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

extern mt_s32 WIN_DRV_Open(struct inode *finode, struct file  *ffile);
extern mt_s32 WIN_DRV_Close(struct inode *finode, struct file  *ffile);
extern mt_s32 DRV_WIN_Ioctl(struct inode *inode, struct file  *file, unsigned int cmd, mt_void *arg);
extern mt_s32 DRV_WIN_Suspend(basedev_s *pdev, pm_message_t state);
extern mt_s32 DRV_WIN_Resume(basedev_s *pdev);
extern mt_s32 DRV_WIN_SetSftwareStage(mt_void);

static mt_device_s g_WinRegisterData;
static MT_DRV_WIN_PRIV_DATA win_priv_data_info;

static long VO_DRV_Ioctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    long Ret;

    Ret = (long)mt_drv_usercopy(ffile->f_path.dentry->d_inode,ffile, cmd, arg, DRV_WIN_Ioctl);

    return Ret;
}

static struct file_operations s_WinFileOps =
{
    .owner          = THIS_MODULE,
    .open           = WIN_DRV_Open,
    .unlocked_ioctl = VO_DRV_Ioctl,
    .release        = WIN_DRV_Close,
};

static baseops_s  s_WinBasicOps = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = DRV_WIN_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = DRV_WIN_Resume,
};

extern 	mt_s32 WIN_Para_Init(void);

mt_void WIN_ModExit(mt_void)
{
    if (!IS_ERR_OR_NULL(win_priv_data_info.hdclk))
        clk_prepare_enable(win_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(win_priv_data_info.hd_os_clk))
        clk_prepare_enable(win_priv_data_info.hd_os_clk);
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_108m))
        clk_prepare_enable(win_priv_data_info.sdclk_108m);
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_27m))
        clk_prepare_enable(win_priv_data_info.sdclk_27m);
    if (!IS_ERR_OR_NULL(win_priv_data_info.vbiclk))
        clk_prepare_enable(win_priv_data_info.vbiclk);

    DRV_WIN_UnRegister();

    mt_drv_dev_unregister(&g_WinRegisterData);

    if (!IS_ERR_OR_NULL(win_priv_data_info.hdclk)) {
        clk_disable_unprepare(win_priv_data_info.hdclk);
        clk_put(win_priv_data_info.hdclk);
    }
    if (!IS_ERR_OR_NULL(win_priv_data_info.hd_os_clk)) {
        clk_disable_unprepare(win_priv_data_info.hd_os_clk);
        clk_put(win_priv_data_info.hd_os_clk);
    }
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_108m)) {
        clk_disable_unprepare(win_priv_data_info.sdclk_108m);
        clk_put(win_priv_data_info.sdclk_108m);
    }
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_27m)) {
        clk_disable_unprepare(win_priv_data_info.sdclk_27m);
        clk_put(win_priv_data_info.sdclk_27m);
    }
    if (!IS_ERR_OR_NULL(win_priv_data_info.vbiclk)) {
        clk_disable_unprepare(win_priv_data_info.vbiclk);
        clk_put(win_priv_data_info.vbiclk);
    }

    return;
}

mt_s32 WIN_ModInit(mt_void)
{
    win_priv_data_info.hdclk = clk_get(NULL, "hdclk");
    win_priv_data_info.hd_os_clk = clk_get(NULL, "hd_os_clk");
    win_priv_data_info.sdclk_108m = clk_get(NULL, "sdclk_108m");
    win_priv_data_info.sdclk_27m = clk_get(NULL, "sdclk_27m");
    win_priv_data_info.vbiclk = clk_get(NULL, "vbiclk");
    if (!IS_ERR_OR_NULL(win_priv_data_info.hdclk))
        clk_prepare_enable(win_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(win_priv_data_info.hd_os_clk))
        clk_prepare_enable(win_priv_data_info.hd_os_clk);
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_108m))
        clk_prepare_enable(win_priv_data_info.sdclk_108m);
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_27m))
        clk_prepare_enable(win_priv_data_info.sdclk_27m);
    if (!IS_ERR_OR_NULL(win_priv_data_info.vbiclk))
        clk_prepare_enable(win_priv_data_info.vbiclk);

    mt_osal_snprintf(g_WinRegisterData.devfs_name, sizeof(g_WinRegisterData.devfs_name), "%s", UMAP_DEVNAME_VO);
    g_WinRegisterData.fops   = &s_WinFileOps;
    g_WinRegisterData.minor  = UMAP_MIN_MINOR_VO;
    g_WinRegisterData.owner  = THIS_MODULE;
    g_WinRegisterData.drvops = &s_WinBasicOps;
    g_WinRegisterData.priv = &win_priv_data_info;

    if (mt_drv_dev_register(&g_WinRegisterData) < 0)
    {
        MT_FATAL_WIN("register VO failed.\n");
        return MT_FAILURE;
    }

    if (MT_SUCCESS != DRV_WIN_Register())
    {
        mt_drv_dev_unregister(&g_WinRegisterData);
        MT_FATAL_WIN("DRV_WIN_Register failed.\n");
        return MT_FAILURE;
    }
    if (MT_SUCCESS != WIN_Para_Init())
    {
        mt_drv_dev_unregister(&g_WinRegisterData);
        MT_FATAL_WIN("DRV_WIN_Register failed.\n");
        return MT_FAILURE;
    }

    /*here we set the sftware to normal stage. not kernel boot stage.*/
    DRV_WIN_SetSftwareStage();

    if (!IS_ERR_OR_NULL(win_priv_data_info.hdclk))
        clk_disable_unprepare(win_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(win_priv_data_info.hd_os_clk))
        clk_disable_unprepare(win_priv_data_info.hd_os_clk);
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_108m))
        clk_disable_unprepare(win_priv_data_info.sdclk_108m);
    if (!IS_ERR_OR_NULL(win_priv_data_info.sdclk_27m))
        clk_disable_unprepare(win_priv_data_info.sdclk_27m);
    if (!IS_ERR_OR_NULL(win_priv_data_info.vbiclk))
        clk_disable_unprepare(win_priv_data_info.vbiclk);

    return  0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


