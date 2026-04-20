/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef TDE_BOOT
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/fcntl.h>  /* O_ACCMODE */
#include <linux/cdev.h>
#include <linux/uaccess.h> /* copy_*_user */
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#include <linux/clk.h>
#include <asm/io.h>
#include "tde_proc.h"
#else
#include "tde_osilist.h"
#include "tde_adp.h"
#endif

#include "tde_config.h"
#include "mt_type.h"

#include "mt_module_debug.h"

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_mmz.h"
#include "mt_drv_avplay.h"
#include "mt_error_mpi.h"
#include "mt_drv_module.h"
#include "mt_module.h"
#include "mt_kernel_adapt.h"
#include "mt_osal.h"
#include "mt_debug.h"

//#define TDE_DEBUG_DISABLE_5
#if defined(TDE_DEBUG_DISABLE) || defined(TDE_DEBUG_DISABLE_5)
#define DUMP_LOG \
    do {         \
    } while (0)
#define TDE_FUN_IN DUMP_LOG
#define TDE_FUN_OUT DUMP_LOG
#define TDE_LOG(...) DUMP_LOG
#define TDE_LINE DUMP_LOG
#else
#define TDE_FUN_IN MT_INFO_TDE("-----------in----------\n")
#define TDE_FUN_OUT MT_INFO_TDE("----------out--------\n")
#define TDE_LOG printk
#define TDE_LINE printk("%s, LINE: %d\n", __FUNCTION__, __LINE__)
#endif

#ifndef MKSTR
#define MKSTR(exp) #exp
#endif
#define MKMARCOTOSTR(exp) MKSTR(exp)

TED_PRIV_DATA_S tde_priv_data_info;

char *g_pszTdeMmzName = NULL;
module_param(g_pszTdeMmzName, charp, S_IRUGO);
MODULE_PARM_DESC(g_pszTdeMmzName, "The mmz name for tde.");

mt_u32 g_u32TdeTmpBuf = 0;
module_param(g_u32TdeTmpBuf, uint, S_IRUGO);

MODULE_PARM_DESC(g_u32TdeTmpBuf, "TDE Tmp buffer.");

#ifndef TDE_BOOT
extern int tde_init_module_k(void);
extern void tde_cleanup_module_k(void);
extern int tde_open(struct inode *finode, struct file *ffile);
extern int tde_release(struct inode *finode, struct file *ffile);

extern long tde_ioctl(struct file *ffile, unsigned int cmd, unsigned long arg);
extern void mt_tde_device_unregister(void);

#ifdef CONFIG_TDE_PM_ENABLE
extern int tde_pm_suspend(basedev_s *pdev, pm_message_t state);
extern int tde_pm_resume(basedev_s *pdev);
//DECLARE_GFX_NODE("mt_tde",tde_open, tde_release, NULL, tde_ioctl, tde_pm_suspend, tde_pm_resume);
#endif

//DECLARE_GFX_NODE("mt_tde",tde_open, tde_release, NULL, tde_ioctl, NULL, NULL);
#endif

#if defined(CONFIG_MT_CHIP_ARIA)
static phys_addr_t s_tde_reg_phy_addr = 0xffd60000;
#else
static phys_addr_t s_tde_reg_phy_addr = SYMPHONY_IO_PA(0xbf430000);
#endif

#ifndef VM_RESERVED // for kernel up to 3.7.0 version
#define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif

static int tde_map_to_register(struct file *fp, struct vm_area_struct *vm)
{
    unsigned long pfn;

    vm_flags_set(vm, VM_IO | VM_RESERVED);
    vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);
    pfn = s_tde_reg_phy_addr >> PAGE_SHIFT;

    return remap_pfn_range(vm, vm->vm_start, pfn, vm->vm_end - vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

static int tde_mmap(struct file *fp, struct vm_area_struct *vm)
{
    int ret = 0;

    TDE_FUN_IN;

    if (vm->vm_pgoff)
    {
        TDE_FUN_OUT;
        return 0;
    }
    else
    {
        ret = tde_map_to_register(fp, vm);
        TDE_FUN_OUT;
        return ret;
    }
}

#ifdef TDE_BOOT
int tde_osr_init(void)
{
    TDE_FUN_IN;

    if (TdeHalInit(TDE_REG_BASEADDR) < 0)
    {
        TDE_FUN_OUT;
        return -1;
    }

    TdeOsiListInit();

    TDE_TRACE(TDE_KERN_DEBUG, "init tde successful!\n");
    TDE_FUN_OUT;
    return 0;
}

void tde_osr_deinit(void)
{
    TDE_FUN_IN;
    TdeOsiListTerm();

    TdeHalRelease();

    TDE_FUN_OUT;
    return;
}
#endif

#ifndef TDE_BOOT

#ifndef CONFIG_TDE_VERSION_DISABLE
static mt_void MT_GFX_ShowVersionK(MTGFX_MODE_ID_E ModID)
{
#if !defined(CONFIG_GFX_COMM_VERSION_DISABLE) && !defined(CONFIG_GFX_COMM_DEBUG_DISABLE)

    mt_char MouleName[7][10] = { "tde", "jpegdec", "jpegenc", "fb", "png", "mtgo", "gfx2d" };
    mt_char Version[160] = "SDK_VERSION:[" MKMARCOTOSTR(SDK_VERSION) "] Build Time:[" __DATE__ ", " __TIME__ "]";

    if (ModID >= MTGFX_BUTT_ID)
        return;

    if ((MTGFX_JPGDEC_ID == ModID) || (MTGFX_JPGENC_ID == ModID))
        GFX_Printk("Load mt_%s.ko success.\t(%s)\n", MouleName[ModID], Version);
    else
        GFX_Printk("Load mt_%s.ko success.\t\t(%s)\n", MouleName[ModID], Version);

    return;

#endif
}
#endif

mt_s32 __init TDE_DRV_ModInit(mt_void)
{
    int ret = 0;

    tde_priv_data_info.gxaclk = clk_get(NULL, "gxaclk");
    tde_priv_data_info.hdclk = clk_get(NULL, "hdclk");
    tde_priv_data_info.sdclk_27m = clk_get(NULL, "sdclk_27m");
    if (!IS_ERR_OR_NULL(tde_priv_data_info.gxaclk))
        clk_prepare_enable(tde_priv_data_info.gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data_info.hdclk))
        clk_prepare_enable(tde_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data_info.sdclk_27m))
        clk_prepare_enable(tde_priv_data_info.sdclk_27m);

    TDE_FUN_IN;

#ifndef MT_MCE_SUPPORT
    ret = tde_init_module_k();
    if (0 != ret)
    {
        TDE_FUN_OUT;
        return -1;
    }
#endif

#ifdef CONFIG_TDE_PM_ENABLE
    /* register tde device */
    ret = MT_GFX_PM_Register();
    if (0 != ret)
    {
       MT_INFO_ERR("register tde failed.\n");
        TDE_FUN_OUT;
        return MT_FAILURE;
    }
#endif

#ifndef CONFIG_TDE_PROC_DISABLE
    {
        TDE_Proc_init();
    }
#endif

#ifndef CONFIG_TDE_VERSION_DISABLE
    MT_GFX_ShowVersionK(MTGFX_TDE_ID);
#endif

    TDE_FUN_OUT;

    if (!IS_ERR_OR_NULL(tde_priv_data_info.gxaclk))
        clk_disable_unprepare(tde_priv_data_info.gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data_info.hdclk))
        clk_disable_unprepare(tde_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data_info.sdclk_27m))
        clk_disable_unprepare(tde_priv_data_info.sdclk_27m);

    return 0;
}

mt_void __exit TDE_DRV_ModExit(mt_void)
{
    if (!IS_ERR_OR_NULL(tde_priv_data_info.gxaclk))
        clk_prepare_enable(tde_priv_data_info.gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data_info.hdclk))
        clk_prepare_enable(tde_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data_info.sdclk_27m))
        clk_prepare_enable(tde_priv_data_info.sdclk_27m);

    TDE_FUN_IN;
#ifndef CONFIG_TDE_PROC_DISABLE
    TDE_Proc_Cleanup();
#endif

#ifndef MT_MCE_SUPPORT
    tde_cleanup_module_k();
#endif

    mt_tde_device_unregister();

#ifdef CONFIG_TDE_PM_ENABLE
    /* cleanup_module is never called if registering failed */
    MT_GFX_PM_UnRegister();
#endif

    TDE_FUN_OUT;

    if (!IS_ERR_OR_NULL(tde_priv_data_info.gxaclk)) {
        clk_disable_unprepare(tde_priv_data_info.gxaclk);
        clk_put(tde_priv_data_info.gxaclk);
    }
    if (!IS_ERR_OR_NULL(tde_priv_data_info.hdclk)) {
        clk_disable_unprepare(tde_priv_data_info.hdclk);
        clk_put(tde_priv_data_info.hdclk);
    }
    if (!IS_ERR_OR_NULL(tde_priv_data_info.sdclk_27m)) {
        clk_disable_unprepare(tde_priv_data_info.sdclk_27m);
        clk_put(tde_priv_data_info.sdclk_27m);
    }
}
#endif

static struct file_operations g_tde_fops =
    {
        .owner = THIS_MODULE,
        .open = tde_open,
        .unlocked_ioctl = tde_ioctl,
        .release = tde_release,
        .mmap = tde_mmap,
    };

static int tde_suspend(basedev_s *pdev, pm_message_t state)
{
    TED_PRIV_DATA_S *tde_priv_data = dev_get_platdata(&pdev->dev);

    TDE_FUN_IN;

    if (!IS_ERR_OR_NULL(tde_priv_data->gxaclk))
        clk_disable_unprepare(tde_priv_data->gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->hdclk))
        clk_disable_unprepare(tde_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->sdclk_27m))
        clk_disable_unprepare(tde_priv_data->sdclk_27m);

    return 0;
}

static int tde_resume(basedev_s *pdev)
{
    TED_PRIV_DATA_S *tde_priv_data = dev_get_platdata(&pdev->dev);

    if (!IS_ERR_OR_NULL(tde_priv_data->gxaclk))
        clk_prepare_enable(tde_priv_data->gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->hdclk))
        clk_prepare_enable(tde_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->sdclk_27m))
        clk_prepare_enable(tde_priv_data->sdclk_27m);

    TDE_FUN_IN;
    return 0;
}

static baseops_s g_tde_drvops =
    {
#ifdef CONFIG_TDE_PM_ENABLE
        .suspend = tde_pm_suspend,
        .resume = tde_pm_resume,
#else
        .suspend = tde_suspend,
        .resume = tde_resume,
#endif
    };

static mt_device_s g_tde_device;

void mt_tde_device_register(void)
{
    TDE_FUN_IN;
    mt_osal_snprintf(g_tde_device.devfs_name, sizeof(g_tde_device.devfs_name), UMAP_DEVNAME_TDE);
    g_tde_device.fops = &g_tde_fops;
    g_tde_device.minor = UMAP_MIN_MINOR_TDE;
    g_tde_device.owner = THIS_MODULE;
    g_tde_device.drvops = &g_tde_drvops;
    g_tde_device.priv = &tde_priv_data_info;

    if (mt_drv_dev_register(&g_tde_device) < 0) {
  MT_ERR_TDE("register tde device failed.\n");
  return;
    }

    TDE_FUN_OUT;
}

void mt_tde_device_unregister(void)
{
    mt_drv_dev_unregister(&g_tde_device);
}

