/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "mt_mach/clock.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "drv_wdg_ioctl.h"
#include "drv_wdg.h"
#include "mt_common.h"
#include "mt_reg_common.h"

#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_sys.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_module_debug.h"

#include "drv_wdg_ext.h"

#define WDG_MAX_NUM    (1)

static void __iomem *ap_wdg_reg_base;
#define WDG_WRITE32(addr,  v) writel(v, (void  *)(ap_wdg_reg_base + (addr)))
#define WDG_READ32(addr) readl((void  *)(ap_wdg_reg_base + (addr)))

/* module param */
static unsigned long g_bWdgPmocing[WDG_MAX_NUM];

#define REG_ON_MAP(offset) (wdt_base + (offset))
#define wdog_TIMER_MARGIN (60000)  /*60 Second*/
static int default_margin = wdog_TIMER_MARGIN; /* in second */
module_param(default_margin, int, 0);

/* local var */
static DEFINE_SPINLOCK(wdog_lock);
static int cur_margin[WDG_MAX_NUM];
static int wdg_value[WDG_MAX_NUM];

static unsigned long driver_open = 0;
static int options[WDG_MAX_NUM]; //WDIOS_ENABLECARD;
static mt_s32 wdg_pm_suspend (basedev_s *pdev, pm_message_t state);
static mt_s32 wdg_pm_resume (basedev_s *pdev);

static WDG_EXT_FUNC_S g_stWdgExtFuncs =
{
    .pfnWdgSuspend      = wdg_pm_suspend,
    .pfnWdgResume       = wdg_pm_resume
};

#if 0
static void wdog_dump(unsigned int wdgindex)
{

}
#endif

static void wdog_set_timeout(unsigned int wdgindex,unsigned int nr)
{
    unsigned long flags = 0, freq = 0, max_ms = 0;
    unsigned  int cnt = nr;

    spin_lock_irqsave(&wdog_lock, flags);
    if (nr == 0)
    {
        cnt = ~0x0;
    }

    symphony_get_clock(HAL_OCP, (unsigned long *)&freq);
    MT_ASSERT(0 != freq);
    max_ms = 0xFFFFFFFF / (freq / 1000);
    //printk("%s %d  max_ms = %d freq = %d nr = %d\n", __FUNCTION__, __LINE__, max_ms,freq,nr);
    if(nr > max_ms)
    {
        cnt = max_ms;
    }
    WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x12345678);
    WDG_WRITE32(MCPU_WATCHDOG_CS_OFF, 0x0);
    WDG_WRITE32(MCPU_WATCHDOG_MPR_OFF, (freq / 1000) * cnt);
    WDG_WRITE32(MCPU_WATCHDOG_RST_DLY_OFF, 125);
    //WDG_WRITE3(MCPU_WATCHDOG_CS_OFF, 0x1);
    WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x0);
    spin_unlock_irqrestore(&wdog_lock, flags);
};

static void wdog_feed(unsigned int wdgindex)
{
    unsigned long flags;

    spin_lock_irqsave(&wdog_lock, flags);
    WDG_WRITE32(MCPU_WATCHDOG_FEED_OFF, 0x1);
    spin_unlock_irqrestore(&wdog_lock, flags);
};

static void wdog_reset_board(unsigned int wdgindex)
{
	unsigned long flags;
	//concerto_wdt_reset();
	spin_lock_irqsave(&wdog_lock, flags);
	/* 1. write protect register */
	WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x12345678);
	/* disable watchdog */
	WDG_WRITE32(MCPU_WATCHDOG_CS_OFF, 0x0);
	/* write protect register */
	WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x0);

	/* set timeout for reboot, should be as little as possible */
	WDG_WRITE32(MCPU_WATCHDOG_MPR_OFF, 0x10);
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	WDG_WRITE32(MCPU_WATCHDOG_RST_DLY_OFF, 125);
#endif

	/* write protect register */
	WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x12345678);
	/* enable watchdog */
	WDG_WRITE32(MCPU_WATCHDOG_CS_OFF, 0x1);
	/* write protect register */
	WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x0);

	spin_unlock_irqrestore(&wdog_lock, flags);
}

/* parameter : timeout time by ms*/
static int wdog_set_heartbeat(unsigned int wdgindex, int t)
{
    int ret = 0;
    wdog_set_timeout(wdgindex, t);

    wdog_feed(wdgindex);

    return ret;
}

static void wdog_start(unsigned int wdgindex)
{
    unsigned long flags;

    spin_lock_irqsave(&wdog_lock, flags);
    WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x12345678);
    WDG_WRITE32(MCPU_WATCHDOG_CS_OFF, 0x1);
    WDG_WRITE32(MCPU_WATCHDOG_RST_DLY_OFF, 125);
    WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x0);
    spin_unlock_irqrestore(&wdog_lock, flags);

    options[wdgindex] = WDIOS_ENABLECARD;
}

static void wdog_stop(unsigned int wdgindex)
{
    unsigned long flags;

    //  unsigned long t;
    spin_lock_irqsave(&wdog_lock, flags);

    WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x12345678);
    WDG_WRITE32(MCPU_WATCHDOG_CS_OFF, 0x0);
    WDG_WRITE32(MCPU_WATCHDOG_PROTECT_OFF, 0x0);
    spin_unlock_irqrestore(&wdog_lock, flags);
    //wdog_set_timeout(wdgindex, 0);
    options[wdgindex] = WDIOS_DISABLECARD;
}

static int wdog_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    unsigned int wdgindex = 0;

    if (test_and_set_bit(0, &driver_open))
    {
        return -EBUSY;
    }


    wdog_feed(wdgindex);

    return ret;
}

static int wdog_release(struct inode *inode, struct file *file)
{
    clear_bit(0, &driver_open);
    /* watchdog should not be controlled by device release api */
    //wdog_stop(wdgindex);
    return 0;
}

static long wdog_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int __user *p = argp;

    switch (cmd)
    {
        case WDIOC_KEEP_ALIVE:
            {
                int index = 0;

                if (get_user(index, p))
                {
                    return -EFAULT;
                }

                wdog_feed(index);
            }

            return 0;

        case WDIOC_SET_TIMEOUT:
            {
                wdg_timeout_s stTimeout;

                if (copy_from_user(&stTimeout, (wdg_timeout_s*)arg, sizeof(wdg_timeout_s)))
                {
                    return -EFAULT;
                }
                cur_margin[stTimeout.u32WdgIndex] = stTimeout.s32Timeout;

                if (wdog_set_heartbeat(stTimeout.u32WdgIndex, stTimeout.s32Timeout))
                {
                    return -EINVAL;
                }

                wdog_feed(stTimeout.u32WdgIndex);
            }

            return 0;

        case WDIOC_GET_TIMEOUT:
            {
                wdg_timeout_s stTimeout;

                if (copy_from_user(&stTimeout, (wdg_timeout_s*)arg, sizeof(wdg_timeout_s)))
                {
                    return -EFAULT;
                }

                if (stTimeout.u32WdgIndex >= WDG_MAX_NUM)
                {
                    return MT_FAILURE;
                }

                stTimeout.s32Timeout = cur_margin[stTimeout.u32WdgIndex];

                return copy_to_user((wdg_timeout_s*)arg, &stTimeout, sizeof(wdg_timeout_s));
            }

        case WDIOC_SET_OPTIONS:
            {
                wdg_option_s stOption;

                if (copy_from_user(&stOption, (wdg_option_s*)arg, sizeof(wdg_option_s)))
                {
                    return -EFAULT;
                }

                if (stOption.u32WdgIndex >= WDG_MAX_NUM)
                {
                    return MT_FAILURE;
                }

                if (stOption.s32Option == WDIOS_ENABLECARD)
                {
                    // wdog_set_heartbeat(stOption.u32WdgIndex, cur_margin[stOption.u32WdgIndex]);
                    wdog_start(stOption.u32WdgIndex);

                    return 0;
                }
                else if (stOption.s32Option == WDIOS_DISABLECARD)
                {
                    wdog_stop(stOption.u32WdgIndex);
                    return 0;
                }
                else if (stOption.s32Option == WDIOS_RESET_BOARD)
                {
                    if (options[stOption.u32WdgIndex] != WDIOS_ENABLECARD)
                    {
                        return MT_FAILURE;
                    }

                    wdog_reset_board(stOption.u32WdgIndex);
                    return 0;
                }
                else
                {
                    return -WDIOS_UNKNOWN;
                }
            }

        default:
            return -ENOIOCTLCMD;
    }
}

/*
 *  Kernel Interfaces
 */

static struct file_operations wdog_fops =
{
    .owner   = THIS_MODULE,
    .llseek  = no_llseek,

    //  .write      = wdog_write,
    .unlocked_ioctl   = wdog_ioctl,
    .open    = wdog_open,
    .release = wdog_release,
};

static mt_s32 wdg_pm_suspend (basedev_s *pdev, pm_message_t state)
{
    unsigned int wdgindex = 0;


    if (WDIOS_ENABLECARD == options[wdgindex])
    {

        options[wdgindex] = WDIOS_DISABLECARD; //WDIOS_ENABLECARD;
        wdog_stop(wdgindex);
        g_bWdgPmocing[wdgindex] = 1;
    }

    return 0;
}

static mt_s32 wdg_pm_resume(basedev_s *pdev)
{
    unsigned int wdgindex = 0;


    if ((WDIOS_DISABLECARD == options[wdgindex]) && (1 == g_bWdgPmocing[wdgindex]))
    {
        wdog_set_timeout(wdgindex, wdg_value[wdgindex]);
        wdog_feed(wdgindex);
        wdog_start(wdgindex);
    }

    //wdg_value = hiwdt_readl(HIWDG_VALUE);
    //printk("> %s: [%d], 0x%x, wdg_value=0x%x\n", __FUNCTION__, __LINE__, wdg_load, wdg_value);
    return 0;
}

static baseops_s wdg_baseOps = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = wdg_pm_suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = wdg_pm_resume
};


static mt_device_s g_WdgRegisterData;
static mt_s32 wdg_proc(struct seq_file *p, mt_void *v)
{
    unsigned long freq = 0;

    symphony_get_clock(HAL_OCP, &freq);
    //printk("freq:%d \n", freq);
    wdog_reset_board(0);
    MT_ASSERT(0 != freq);
    PROC_PRINT(p, "Enabled\t Period(ms)\n");
    PROC_PRINT(p, "%s\t %lu\t \n", (WDG_READ32(MCPU_WATCHDOG_CS_OFF) & 0x1)?"Yes":"No",
            (WDG_READ32(MCPU_WATCHDOG_MPR_OFF)/(freq/1000))/* ,
                                                              (WDG_READ32(R_M_WD_STATUS) & 0xFF)*/);

    return 0;
}
mt_s32 __init wdg_drv_modinit(mt_void)
{
    int ret = 0;
    unsigned int wdgindex = 0;
    mt_proc_entry_t *item = NULL;
    mt_drv_proc_t wdg_proc_ops;

    ap_wdg_reg_base = ioremap(MCPU_WATCHDOG_PHY_BASE_ADDR, 0x200);
    if (!ap_wdg_reg_base){
        pr_err("Cannot ioremap\n");
        return -ENOMEM;
    }
    MT_INFO_WDG("\n ... wdg init  ... \n");
    (mt_void)mt_drv_module_register(MT_ID_WDG, "wdg", (mt_void *)&g_stWdgExtFuncs);
    cur_margin[wdgindex] = default_margin;
    g_bWdgPmocing[wdgindex] = MT_FALSE;
    options[wdgindex] = WDIOS_DISABLECARD;

    /* Check that the default_margin value is within it's range ; if not reset to the default */
    wdog_set_heartbeat(wdgindex, default_margin);

    snprintf(g_WdgRegisterData.devfs_name, sizeof(g_WdgRegisterData.devfs_name), UMAP_DEVNAME_WDG);
    g_WdgRegisterData.minor = UMAP_MIN_MINOR_WDG;
    g_WdgRegisterData.owner = THIS_MODULE;
    g_WdgRegisterData.fops   = &wdog_fops;
    g_WdgRegisterData.drvops = &wdg_baseOps;
    if (mt_drv_dev_register(&g_WdgRegisterData) < 0)
    {
        //printk (KERN_ERR wdog_PFX "mt_drv_dev_register err (err=%d)\n", ret);
        return MT_FAILURE;
    }
    memset(&wdg_proc_ops, 0, sizeof(mt_drv_proc_t));
    wdg_proc_ops.fnRead = wdg_proc;
    item = mt_drv_proc_add_module(MT_MOD_WDG, &wdg_proc_ops, NULL);
    if(!item)
    {
        return -1;
    }
    return ret;
}


mt_void __exit wdg_drv_modexit(mt_void)
{
    mt_drv_module_unregister(MT_ID_WDG);
    mt_drv_dev_unregister(&g_WdgRegisterData);
    wdog_set_timeout(0, 0);
    iounmap(ap_wdg_reg_base);
}

