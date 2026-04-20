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
//#ifdef CONFIG_MT_CHIP_ARIA_F
//#define WDG_CLK_HZ (13.5 * 1000 * 1000)  /* select crystal,freq fix:24M*/
//#else
#define WDG_CLK_HZ (24 * 1000 * 1000)  /* select crystal,freq fix:24M*/

//#endif
#if defined(CONFIG_MT_CHIP_ARIA)
static unsigned int sn_wdg_base = 0;

#define WDG_REG_BASE (sn_wdg_base)


#define WDG_WRITE32(addr,  v) writel(v, (void __iomem *)((WDG_REG_BASE) + addr))
#define WDG_READ32(addr) readl((void __iomem *)((WDG_REG_BASE) + addr))


#define R_WATCHDOG_CTRL 0x008
#define R_WATCHDOG_DLY 0x00C

#define  WATCHDOG_CTRL_ENABLE       0x1
#define  WATCHDOG_CTRL_DISABLE      0x2
#define  WATCHDOG_CTRL_KNOCKER      0x8
#define  WATCHDOG_CTRL_IRQMODE      0x4

#define WDG_IRQ_NUM 99
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)

#include "mt_mach/clock.h"

#define WDG_WRITE32(addr,  v) writel(v, (void  *)( addr))
#define WDG_READ32(addr) readl((void  *)(addr))

/************************************************************
 * WatchDog Register definition
 ************************************************************/
/*!
  comments
  */
#define R_WD_REG_BASE                        0xBF100100
/*!
  comments
  */
#define R_M_WD_MPR                    (R_WD_REG_BASE + 0x0)
/*!
  comments
  */
#define R_M_WD_PROTECT                    (R_WD_REG_BASE + 0x4)
/*!
  comments
  */
#define R_M_WD_EN                                (R_WD_REG_BASE + 0x8)
/*!
  comments
  */
#define R_M_WD_FEED                            (R_WD_REG_BASE + 0xc)
/*!
  comments
  */
#define R_M_WD_STA_CLR                    (R_WD_REG_BASE + 0x10)
/*!
  comments
  */
#define R_M_WD_STATUS                        (R_WD_REG_BASE + 0x14)


#endif

static unsigned long g_bWdgPmocing[WDG_MAX_NUM];

/* debug */

//#define wdog_PFX "wdog: "

/* module param */
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

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
static void wdog_dump(unsigned int wdgindex)
{

}
#endif
#if defined(CONFIG_MT_CHIP_ARIA)
static irqreturn_t  wdg_isr(int irq,void * punused)
{
#if defined(CONFIG_MT_CHIP_ARIA)
	u32 dtmp;
	unsigned long flags;
	MT_INFO_WDG("wdg isr \n");
	    spin_lock_irqsave(&wdog_lock, flags);

	dtmp = WDG_READ32(R_WATCHDOG_CTRL);

	dtmp |= WATCHDOG_CTRL_DISABLE;
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);

	dtmp = WDG_READ32(R_WATCHDOG_CTRL);
	dtmp &= (~WATCHDOG_CTRL_ENABLE);
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);

	dtmp = WDG_READ32(R_WATCHDOG_CTRL);
	dtmp &= (~WATCHDOG_CTRL_DISABLE);
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);

       dtmp = WDG_READ32(R_WATCHDOG_CTRL);

	WDG_WRITE32(R_WATCHDOG_CTRL,
      dtmp | WATCHDOG_CTRL_ENABLE | WATCHDOG_CTRL_KNOCKER);

    spin_unlock_irqrestore(&wdog_lock, flags);
#endif
	 return IRQ_HANDLED;
}

static void wdog_enable_irq(unsigned int wdgindex)
{
#if defined(CONFIG_MT_CHIP_ARIA)
    unsigned long flags;
    u32 data = 0;
   int err;

    spin_lock_irqsave(&wdog_lock, flags);



	data = WDG_READ32(R_WATCHDOG_CTRL);
	data |= WATCHDOG_CTRL_IRQMODE;

	WDG_WRITE32(R_WATCHDOG_CTRL, data);

	/* register isr */

	err = request_irq(WDG_IRQ_NUM, wdg_isr,  IRQF_TRIGGER_HIGH, "wdg", NULL);

    spin_unlock_irqrestore(&wdog_lock, flags);

#endif

};

static void wdog_disable_irq(unsigned int wdgindex)
{
#if defined(CONFIG_MT_CHIP_ARIA)
    unsigned long flags;
    u32 dtmp;

    spin_lock_irqsave(&wdog_lock, flags);
	dtmp = WDG_READ32(R_WATCHDOG_CTRL);

	dtmp &= ~WATCHDOG_CTRL_IRQMODE;
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);

    spin_unlock_irqrestore(&wdog_lock, flags);
#endif
};
#endif

static void wdog_set_timeout(unsigned int wdgindex,unsigned int nr)
{
	unsigned long flags;

#if defined(CONFIG_MT_CHIP_ARIA) || defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	unsigned  int cnt = nr;
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	unsigned  int freq = 0;
	unsigned  int max_ms = 0;
#endif

	spin_lock_irqsave(&wdog_lock, flags);
#if defined(CONFIG_MT_CHIP_ARIA)
	if (nr == 0)
	{
	    cnt = ~0x0;
	}

    	WDG_WRITE32(R_WATCHDOG_DLY, cnt);

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	symphony_get_clock(HAL_OCP, (unsigned long *)&freq);
	MT_ASSERT(0 != freq);
	max_ms = 0xFFFFFFFF / (freq / 1000);
	//printk("%s %d  max_ms = %d freq = %d nr = %d\n", __FUNCTION__, __LINE__, max_ms,freq,nr);
	if(nr > max_ms)
	{
		cnt = max_ms;
	}
    WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x12345678);
    WDG_WRITE32((u32 *)R_M_WD_EN, 0x0);
	WDG_WRITE32((u32 *)R_M_WD_MPR, (freq / 1000) * cnt);
	//WDG_WRITE32((u32 *)R_M_WD_EN, 0x1);
	WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x0);
#endif
    	spin_unlock_irqrestore(&wdog_lock, flags);
};

static void wdog_feed(unsigned int wdgindex)
{
    unsigned long flags;
#if defined(CONFIG_MT_CHIP_ARIA)
    u32 dtmp;
#endif
	//printk(" wfeed\n");

    spin_lock_irqsave(&wdog_lock, flags);
#if defined(CONFIG_MT_CHIP_ARIA)
	dtmp = WDG_READ32(R_WATCHDOG_CTRL);

	dtmp |= WATCHDOG_CTRL_KNOCKER;
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	WDG_WRITE32((u32 *)R_M_WD_FEED, 0x1);
#endif
    spin_unlock_irqrestore(&wdog_lock, flags);
};

static void wdog_reset_board(unsigned int wdgindex)
{
    unsigned long flags;
	/*
	 * Fix Bug: Mantis 0000595: 重启后uboot有概率挂死
	 *		    等待系统处理完成
	 */
	//printk("w reset\n");
	mdelay(100);	//for safe wdg reset
	//MT_INFO_WDG("w reset\n");
      //mdelay(100);
    spin_lock_irqsave(&wdog_lock, flags);
	//printk("out spin lock\n");
	//mdelay(100);
#if defined(CONFIG_MT_CHIP_ARIA)
	WDG_WRITE32((u32 *)R_WATCHDOG_DLY, WDG_CLK_HZ / 1000 * 200);
	WDG_WRITE32((u32 *)R_WATCHDOG_CTRL,
				WATCHDOG_CTRL_ENABLE | WATCHDOG_CTRL_KNOCKER);

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x12345678);
    WDG_WRITE32((u32 *)R_M_WD_EN, 0x0);
	WDG_WRITE32((u32 *)R_M_WD_MPR, 0);
	WDG_WRITE32((u32 *)R_M_WD_EN, 0x1);
	WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x0);
#endif
	//printk("into spin unclock\n");
	//mdelay(100);
    spin_unlock_irqrestore(&wdog_lock, flags);
    //printk("out w reset\n");
    //mdelay(100);
	/*
	 * Fix Bug: Mantis 0000595: 重启后uboot有概率挂死
	 *		    挂住系统,不再处理任何任务
	 */
    while(1);
	//printk("while\n");
}

/* parameter : timeout time by ms*/
static int wdog_set_heartbeat(unsigned int wdgindex, int t)
{
    int ret = 0;
#if defined(CONFIG_MT_CHIP_ARIA)
    unsigned int u32WdgLoad;
	unsigned int max_ms;
	unsigned int freq = WDG_CLK_HZ;

	max_ms = 0xFFFFFFFF / (freq / 1000);
	if(t > max_ms)
	{
	  //printk("max time out=%d \n", max_ms);
	  t = max_ms;

	}
	u32WdgLoad =  (freq / 1000)*t;
	//printk("max time out=%d   f=%d   u32WdgLoad=%d \n", t, freq, u32WdgLoad);

	wdog_set_timeout(wdgindex, u32WdgLoad);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	wdog_set_timeout(wdgindex, t);
#endif

    wdog_feed(wdgindex);

    return ret;
}

static void wdog_start(unsigned int wdgindex)
{
    unsigned long flags;
#if defined(CONFIG_MT_CHIP_ARIA)
    u32 data;
#endif

    //  unsigned long t;

    spin_lock_irqsave(&wdog_lock, flags);
#if defined(CONFIG_MT_CHIP_ARIA)
    data = WDG_READ32(R_WATCHDOG_CTRL);

    WDG_WRITE32(R_WATCHDOG_CTRL,
    data | WATCHDOG_CTRL_ENABLE | WATCHDOG_CTRL_KNOCKER);

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x12345678);
	WDG_WRITE32((u32 *)R_M_WD_EN, 0x1);
	WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x0);

#endif
    spin_unlock_irqrestore(&wdog_lock, flags);

    options[wdgindex] = WDIOS_ENABLECARD;
}

static void wdog_stop(unsigned int wdgindex)
{
    unsigned long flags;
#if defined(CONFIG_MT_CHIP_ARIA)
   u32 dtmp;
#endif

    //  unsigned long t;
    spin_lock_irqsave(&wdog_lock, flags);

#if defined(CONFIG_MT_CHIP_ARIA)
	dtmp = WDG_READ32(R_WATCHDOG_CTRL);

	dtmp |= WATCHDOG_CTRL_DISABLE;
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);

	dtmp = WDG_READ32(R_WATCHDOG_CTRL);
	dtmp &= (~WATCHDOG_CTRL_ENABLE);
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);

	dtmp = WDG_READ32(R_WATCHDOG_CTRL);
	dtmp &= (~WATCHDOG_CTRL_DISABLE);
	WDG_WRITE32(R_WATCHDOG_CTRL, dtmp);

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x12345678);
	WDG_WRITE32((u32 *)R_M_WD_EN, 0x0);
	WDG_WRITE32((u32 *)R_M_WD_PROTECT, 0x0);
#endif

    spin_unlock_irqrestore(&wdog_lock, flags);

    wdog_set_timeout(wdgindex, 0);

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
   // unsigned int wdgindex = 0;
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
#if defined(CONFIG_MT_CHIP_ARIA)
		else if (stOption.s32Option == WDIOS_ENABLECARD_IRQ)
            {
                wdog_enable_irq(stOption.u32WdgIndex);
                return 0;
            }
	     else if (stOption.s32Option == WDIOS_DISABLECARD_IRQ)
            {
                if (options[stOption.u32WdgIndex] != WDIOS_ENABLECARD)
                {
                   // return MT_FAILURE;
                }

                wdog_disable_irq(stOption.u32WdgIndex);
                return 0;
            }
#endif
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
#if defined(CONFIG_MT_CHIP_ARIA)
unsigned int mt_get_sys_ctrl_base(void);
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
static mt_s32 wdg_proc(struct seq_file *p, mt_void *v)
{
    unsigned long freq = 0;
    
    symphony_get_clock(HAL_OCP, &freq);
    MT_ASSERT(0 != freq);
    PROC_PRINT(p, "Enabled\t Period(ms)\n");
    PROC_PRINT(p, "%s\t %lu\t \n", (WDG_READ32((u32 *)R_M_WD_EN) & 0x1)?"Yes":"No", 
            (WDG_READ32((u32 *)R_M_WD_MPR)/(freq/1000))/* , 
           (WDG_READ32((u32 *)R_M_WD_STATUS) & 0xFF)*/);

    return 0;
}
#endif
mt_s32 __init wdg_drv_modinit(mt_void)
{
    int ret = 0;
    unsigned int wdgindex = 0;
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    mt_proc_entry_t *item = NULL;
    mt_drv_proc_t wdg_proc_ops;
#endif
    
	MT_INFO_WDG("\n ... wdg init  ... \n");
    (mt_void)mt_drv_module_register(MT_ID_WDG, "wdg", (mt_void *)&g_stWdgExtFuncs);
#if defined(CONFIG_MT_CHIP_ARIA)
	sn_wdg_base = mt_get_sys_ctrl_base();
	MT_INFO_WDG("\n ... wdg init  .sn_wdg_base=%x.. \n", sn_wdg_base);
#endif


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
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    memset(&wdg_proc_ops, 0, sizeof(mt_drv_proc_t));
    wdg_proc_ops.fnRead = wdg_proc;
    item = mt_drv_proc_add_module(MT_MOD_WDG, &wdg_proc_ops, NULL);
    if(!item)
    {
        return -1;
    }
#endif
    return ret;
}


mt_void __exit wdg_drv_modexit(mt_void)
{

    mt_drv_module_unregister(MT_ID_WDG);

    mt_drv_dev_unregister(&g_WdgRegisterData);


    wdog_set_timeout(0, 0);
}

