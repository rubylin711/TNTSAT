/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
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



#include "mt_common.h"
#include "mt_reg_common.h"

#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_sys.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_osal.h"
#include "drv_timer.h"
#include "mt_module_debug.h"

#define AON_MISC_OFFSET 0xc000

#define ALWAYS_TIMER_MAX_NUM    (1)
extern unsigned long plat_get_xtal_freq(void);
#if 0
def CONFIG_MT_CHIP_ARIA_F
#define MTIMER_CLK_HZ (13.5 * 1000 * 1000)  /* select crystal,freq fix:24M*/

#else
//#define MTIMER_CLK_HZ (27 * 1000 * 1000)  /* select crystal,freq fix:24M*/
#define MTIMER_CLK_HZ (plat_get_xtal_freq())

#endif

#define MS_CLK_PRELOAD  ((MTIMER_CLK_HZ/1000)-1)

static unsigned int sn_always_mtimer_base = 0;
static unsigned int s_timer_debug_level = 0;

#define ALWAYS_TIMER_REG_BASE (sn_always_mtimer_base)

#define ALWAYS_TIMER_WRITE32(addr,  v) writel(v, (void __iomem *)((ALWAYS_TIMER_REG_BASE) + addr))
#define ALWAYS_TIMER_READ32(addr) readl((void __iomem *)((ALWAYS_TIMER_REG_BASE) + addr))


#define R_RTC_PRESCALER 0x00
#define R_RTC_PRELOAD 0x04
#define R_RTC_TRIGGER 0x08
#define R_RTC_COUNT 0x0C

#define AON_RTC_INT 0x01c


#define  MTIMER_INT_BIT       0x1

#define MTIMER_IRQ_NUM 102
#if defined(CONFIG_MT_CHIP_ARIA)
typedef struct __tag_mt_timer_info
{
	timer_fn p_fn;
	void *p_para;
	int  cycled;
	int used;
}mt_timer_info;
#endif
/* debug */

static  mt_timer_info  s_always_timer_info[ALWAYS_TIMER_MAX_NUM];


/* local var */
static DEFINE_SPINLOCK(always_timer_lock);

static unsigned long driver_open = 0;
static mt_s32 always_timer_pm_suspend (basedev_s *pdev, pm_message_t state);
static mt_s32 always_timer_pm_resume (basedev_s *pdev);


static irqreturn_t  always_timer_isr(int irq,void * para)
{

	u32 dtmp;
	unsigned long flags;
	timer_fn p_fn = NULL;
	mt_timer_info *p_info = (mt_timer_info *)para;

	if (s_timer_debug_level)
	{
		MT_INFO_TIMER("always_timer isr \n");
	}
	spin_lock_irqsave(&always_timer_lock, flags);

	dtmp = ALWAYS_TIMER_READ32(AON_RTC_INT);
	ALWAYS_TIMER_WRITE32(AON_RTC_INT, MTIMER_INT_BIT); /*just for clear always_timer bit*/
	if (NULL !=  p_info )
	{
		p_fn = p_info->p_fn;
		if (0 == p_info->cycled)
		{
			ALWAYS_TIMER_WRITE32(R_RTC_TRIGGER, ~0);
			p_info->p_fn = NULL;
		}
	}


	spin_unlock_irqrestore(&always_timer_lock, flags);

	if (p_fn)
	{
		p_fn(p_info->p_para);
	}
	return IRQ_HANDLED;
}

static void always_timer_enable_irq(unsigned int mtimerindex)
{
    unsigned long flags;
   int err;

    ALWAYS_TIMER_WRITE32(AON_RTC_INT, MTIMER_INT_BIT);

    spin_lock_irqsave(&always_timer_lock, flags);


	/* register isr */

	err = request_irq(MTIMER_IRQ_NUM, always_timer_isr,  IRQF_TRIGGER_HIGH, "always_timer", &s_always_timer_info[0]);

    spin_unlock_irqrestore(&always_timer_lock, flags);



}

#if 0
static void always_timer_disable_irq(unsigned int mtimerindex)
{
    unsigned long flags;
    //u32 dtmp;

    spin_lock_irqsave(&always_timer_lock, flags);

    spin_unlock_irqrestore(&always_timer_lock, flags);
}
#endif

void always_timer_set_timeout(int always_timerindex,unsigned int ms)
{
    //unsigned long flags;
    unsigned  int cnt = ms;

   // spin_lock_irqsave(&always_timer_lock, flags);

    if (ms == 0)
    {
        cnt = ~0x0;
    }
	MT_INFO_TIMER("------%d \n", cnt);
	ALWAYS_TIMER_WRITE32(R_RTC_PRELOAD, 0);
	ALWAYS_TIMER_WRITE32(R_RTC_TRIGGER, cnt);

   //spin_unlock_irqrestore(&always_timer_lock, flags);
}



mt_s32  always_timer_start(int id)
{
	return 0;
}

mt_s32  always_timer_stop(int id)
{
	return 0;
}

mt_u32  always_timer_read_cnt(int id)
{
	mt_u32 dtmp;
	dtmp = ALWAYS_TIMER_READ32(R_RTC_COUNT);
	return dtmp;
}

mt_s32  always_timer_request(int ms, timer_fn pfn, void *para, int cycled, char *name)
{
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&always_timer_lock, flags);

	if (s_always_timer_info[i].used)
	{
		spin_unlock_irqrestore(&always_timer_lock, flags);
		return -1;
	}

	s_always_timer_info[i].used = 1;
	s_always_timer_info[i].p_fn = pfn;
	s_always_timer_info[i].p_para = para;
	s_always_timer_info[i].cycled = cycled;
	always_timer_set_timeout(i, ms);
	spin_unlock_irqrestore(&always_timer_lock, flags);
	return 0;
}

static int always_timer_open(struct inode *inode, struct file *file)
{
    int ret = 0;

    if (test_and_set_bit(0, &driver_open))
    {
        return -EBUSY;
    }



    return ret;
}

static int always_timer_release(struct inode *inode, struct file *file)
{
    unsigned int mtimerindex = 0;
    clear_bit(0, &driver_open);

        always_timer_stop(mtimerindex);
    return 0;
}

static long always_timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
#if 0
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

        }

        return 0;


    default:
        return -ENOIOCTLCMD;
    }
#endif
	 return -ENOIOCTLCMD;
}

/*
 *  Kernel Interfaces
 */

static struct file_operations always_timer_fops =
{
    .owner   = THIS_MODULE,
    .llseek  = no_llseek,

    //  .write      = always_timer_write,
    .unlocked_ioctl   = always_timer_ioctl,
    .open    = always_timer_open,
    .release = always_timer_release,
};

static mt_s32 always_timer_pm_suspend (basedev_s *pdev, pm_message_t state)
{
    unsigned int mtimerindex = 0;


     always_timer_stop(mtimerindex);


    return 0;
}

static mt_s32 always_timer_pm_resume(basedev_s *pdev)
{
    unsigned int mtimerindex = 0;


    always_timer_start(mtimerindex);

    //mtimer_value = hiwdt_readl(HIWDG_VALUE);
    //printk("> %s: [%d], 0x%x, mtimer_value=0x%x\n", __FUNCTION__, __LINE__, mtimer_load, mtimer_value);
    return 0;
}

static baseops_s always_timer_baseOps = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = always_timer_pm_suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = always_timer_pm_resume
};

mt_s32 always_timer_proc_read(struct seq_file *s, mt_void *arg)
{


	if(!sn_always_mtimer_base)
	{
		MT_INFO_TIMER("always_timer module not init \n");
		return 0;
	}

	MT_INFO_TIMER(" debug level  =%d \n ", s_timer_debug_level);
	MT_INFO_TIMER(" prescalar  =%d \n ", ALWAYS_TIMER_READ32(R_RTC_PRESCALER));
	MT_INFO_TIMER(" triger   =%d \n ", ALWAYS_TIMER_READ32(R_RTC_TRIGGER));
	MT_INFO_TIMER(" preload   =%d \n ", ALWAYS_TIMER_READ32(R_RTC_PRELOAD));
	MT_INFO_TIMER(" cnt   =%d \n ", ALWAYS_TIMER_READ32(R_RTC_COUNT));



	return 0;
}

#define MAX_FILENAME_LENTH 256

static int seperate_string(char *s, char **left, char **right)
{
	char *p = s;
    /* find '=' */
	while(*p != '\0' && *p++ != '=');

	if(*--p != '=')
		return -1;

    /* seperate left from right vaule by '=' */
	*p = '\0';
 	*left = s;
 	*right = p+1;

	return 0;
}
static char *mstrip_string(char *s, char *d)
{
	char *p = d;
	do{
		if (*s == '\n')
			*s = '\0';
		if (*s != ' ')
			*p++ = *s;
	}while(*s++ != '\0');

	return d;
}


mt_s32 always_timer_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char m[MAX_FILENAME_LENTH] = {0};
	char d[MAX_FILENAME_LENTH] = {0};
	size_t len = MAX_FILENAME_LENTH;
	char *left, *right;
	int  level;

	if (*ppos >= MAX_FILENAME_LENTH)
		return -EFBIG;

	len = min(len, count);
	if (copy_from_user(m, buf, len))
		return -EFAULT;


	if (!sn_always_mtimer_base)
	{
		MT_ERR_TIMER("Log module not init\n");
		goto out;
	}
	memset(d, 0, sizeof(d));
	mstrip_string(m, d);
    /* echo help info to current terinmal */
	if (!mt_osal_strncasecmp("help", m, 4))
	{
#if 1
		MT_INFO_TIMER("To modify the level, use command line in shell: \n");

	 MT_INFO_TIMER("Use 'echo \"level = x\" > /proc/msp/atimer' to change  printlevel.\n");
        MT_INFO_TIMER("Use 'echo \"scalar = x\" > /proc/msp/atimer' to change scalar cnt.\n");
	  MT_INFO_TIMER("Use 'echo \"triger = x\" > /proc/msp/atimer' to change triger cnt.\n");

     	goto out;
#endif
	}

#if 1
	if (seperate_string(d, &left, &right)){
        	MT_ERR_TIMER("string is unkown!\n");
		goto out;
	}
#endif
	level = simple_strtol(right, NULL, 10);
	MT_INFO_TIMER("  option %s %s \n", left, right);

	if (!mt_osal_strncasecmp("level", left, strlen("level")+1))
	{
		MT_INFO_TIMER("level = %d \n", level);
		s_timer_debug_level = (unsigned int)level;
	}
	else if (!mt_osal_strncasecmp("scalar", left, strlen("scalar")+1))
	{
		MT_INFO_TIMER("scalar = %d\n", level);
		ALWAYS_TIMER_WRITE32(R_RTC_PRESCALER, level);
	}
	else if (!mt_osal_strncasecmp("triger", left, strlen("triger")+1))
	{
		MT_INFO_TIMER("triger = %d\n", level);
		ALWAYS_TIMER_WRITE32(R_RTC_TRIGGER, level);
		ALWAYS_TIMER_WRITE32(R_RTC_PRELOAD, 0);
	}
	else
	{
		MT_ERR_TIMER(" unknow option %s \n", m);
	}



out:
	*ppos = len;
	return len;
}


static mt_device_s g_always_timer_register_data;
unsigned int  mt_get_alwayson_base(void);

mt_s32 __init always_timer_drv_modinit(mt_void)
{
    int ret = 0;
    mt_proc_entry_t *item = NULL;

	MT_INFO_TIMER("\n ... always init  ... \n");
    (mt_void)mt_drv_module_register(MT_ID_ATIMER, "atimer", NULL);
	sn_always_mtimer_base = mt_get_alwayson_base() + AON_MISC_OFFSET;
	MT_INFO_TIMER("\n ... always init  .sn_always_mtimer_base=%x.. \n", sn_always_mtimer_base);

	memset(&s_always_timer_info, 0, sizeof(s_always_timer_info));

    /* Check that the default_margin value is within it's range ; if not reset to the default */

    snprintf(g_always_timer_register_data.devfs_name, sizeof(g_always_timer_register_data.devfs_name), UMAP_DEVNAME_ATIMER);
    g_always_timer_register_data.minor = UMAP_MIN_MINOR_ATIMER;
    g_always_timer_register_data.owner = THIS_MODULE;
    g_always_timer_register_data.fops   = &always_timer_fops;
    g_always_timer_register_data.drvops = &always_timer_baseOps;
    if (mt_drv_dev_register(&g_always_timer_register_data) < 0)
    {
        MT_ERR_TIMER( "always  mt_drv_dev_register err (err=%d)\n", ret);
        return MT_FAILURE;
    }

	item = mt_drv_proc_add_module(MT_MOD_ATIMER, NULL, NULL);
    if(!item)
    {
		return -1;
	}

	item->read = always_timer_proc_read;
	item->write = always_timer_proc_write;

	ALWAYS_TIMER_WRITE32(R_RTC_PRESCALER, MS_CLK_PRELOAD);

	always_timer_set_timeout(0, 0);
	always_timer_enable_irq(0);

    return ret;
}


mt_void __exit always_timer_drv_modexit(mt_void)
{

    mt_drv_module_unregister(MT_ID_ATIMER);

    mt_drv_dev_unregister(&g_always_timer_register_data);


    always_timer_set_timeout(0, 0);
}
