/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
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

#define RTC_TIMER_OFFSET 0x0

#define M_SYS_TIMER_MAX_NUM    (4)
extern unsigned long plat_get_xtal_freq(void);

#if 0
def CONFIG_MT_CHIP_ARIA_F
#define MTIMER_CLK_HZ (13.5 * 1000 * 1000)  /* select crystal,freq fix:24M*/

#else
//#define MTIMER_CLK_HZ (27 * 1000 * 1000)  /* select crystal,freq fix:24M*/
#define MTIMER_CLK_HZ (plat_get_xtal_freq())

#endif

#define MS_CLK_PRELOAD  ((MTIMER_CLK_HZ/1000)-1)

static unsigned int sn_mtimer_base = 0;
static unsigned int s_timer_debug_level = 0;

#define MTIMER_REG_BASE (sn_mtimer_base)

#define MTIMER_WRITE32(addr,  v) writel(v, (void __iomem *)((MTIMER_REG_BASE) + addr))
#define MTIMER_READ32(addr) readl((void __iomem *)((MTIMER_REG_BASE) + addr))

#define R_RTC_ID_OFFSET 0x10

#define R_RTC_PRESCALER 0x160
#define R_RTC_PRELOAD 0x164
#define R_RTC_TRIGGER 0x168
#define R_RTC_COUNT 0x16C

#define SYS_RTC_INT 0x15c


#define  MTIMER_INT_BIT       0x1

#define MTIMER_IRQ_NUM 109

typedef struct __tag_mt_timer_info
{
	timer_fn p_fn;
	void *p_para;
	int  cycled;
	int timeout;
	int used;
	char * name;
}mt_timer_info;

/* debug */

static  mt_timer_info  s_mt_timer_info[M_SYS_TIMER_MAX_NUM];


/* local var */
static DEFINE_SPINLOCK(mtimer_lock);

static unsigned long driver_open = 0;
static mt_s32 mtimer_pm_suspend (basedev_s *pdev, pm_message_t state);
static mt_s32 mtimer_pm_resume (basedev_s *pdev);


static irqreturn_t  mtimer_isr(int irq,void * para)
{

	u32 dtmp;
	unsigned long flags;
	timer_fn p_fn = NULL;
	mt_timer_info *p_info = (mt_timer_info *)para;
	u32 id = irq - MTIMER_IRQ_NUM;
	u32 mask  = 1 << id;
	if (s_timer_debug_level)
	{
		MT_INFO_TIMER("mtimer isr %d  \n", id);
	}
	spin_lock_irqsave(&mtimer_lock, flags);

	dtmp = MTIMER_READ32(SYS_RTC_INT);
	if (!(dtmp && mask))
	{

		MT_INFO_TIMER("unkown irq=%d  0x%x \n", irq, dtmp);
		spin_unlock_irqrestore(&mtimer_lock, flags);
		return IRQ_HANDLED;
	}
	MTIMER_WRITE32(SYS_RTC_INT  , mask); /*just for clear mtimer bit*/
	if (NULL !=  p_info )
	{
		p_fn = p_info->p_fn;
		if (0 == p_info->cycled)
		{
			MTIMER_WRITE32(R_RTC_TRIGGER + R_RTC_ID_OFFSET * id , ~0);
			p_info->p_fn = NULL;
		}
	}


	spin_unlock_irqrestore(&mtimer_lock, flags);

	if (p_fn)
	{
		p_fn(p_info->p_para);
		if (0 != p_info->cycled)
		{
			MTIMER_WRITE32(R_RTC_PRELOAD + R_RTC_ID_OFFSET * id, 0);
			MTIMER_WRITE32(R_RTC_TRIGGER + R_RTC_ID_OFFSET * id, p_info->timeout);
		}
	}
	return IRQ_HANDLED;
}

static void mtimer_enable_irq(unsigned int mtimerindex)
{
    unsigned long flags;
   int err;

    spin_lock_irqsave(&mtimer_lock, flags);


	/* register isr */

	err = request_irq(MTIMER_IRQ_NUM + mtimerindex, mtimer_isr,  IRQF_TRIGGER_HIGH, "mtimer", &s_mt_timer_info[mtimerindex]);

    spin_unlock_irqrestore(&mtimer_lock, flags);



}
#if 0
static void mtimer_disable_irq(unsigned int mtimerindex)
{
    //unsigned long flags;
    //u32 dtmp;

    //spin_lock_irqsave(&mtimer_lock, flags);
	free_irq(MTIMER_IRQ_NUM + mtimerindex, NULL);
    //spin_unlock_irqrestore(&mtimer_lock, flags);
}
#endif
void mtimer_set_timeout(int id, unsigned int ms)
{
    //unsigned long flags;
    unsigned  int cnt = ms;
    unsigned  int dtmp;
    unsigned  int mask = 1 << id;


   // spin_lock_irqsave(&mtimer_lock, flags);

    if (ms == 0)
    {
        cnt = ~0x0;
    }
	MT_INFO_TIMER("-----%d \n",cnt);
	MTIMER_WRITE32(R_RTC_PRESCALER  + R_RTC_ID_OFFSET * id,  MS_CLK_PRELOAD);
	MTIMER_WRITE32(R_RTC_PRELOAD + R_RTC_ID_OFFSET * id, 0);
	MTIMER_WRITE32(R_RTC_TRIGGER + R_RTC_ID_OFFSET * id, cnt);
	dtmp = MTIMER_READ32(SYS_RTC_INT);

	if (dtmp & mask)
	{
		MTIMER_WRITE32(SYS_RTC_INT, mask);
	}

   //spin_unlock_irqrestore(&mtimer_lock, flags);
}



mt_s32  mt_drv_timer_start(int id)
{
	return 0;
}

mt_s32  mt_drv_timer_stop(int id)
{
	return 0;
}

mt_u32  mt_drv_timer_read_cnt(int id)
{
	mt_u32 dtmp = 0;
	if (id >= M_SYS_TIMER_MAX_NUM)
	{
		return dtmp;
	}
	dtmp = MTIMER_READ32((R_RTC_COUNT + R_RTC_ID_OFFSET * id));
	return dtmp;
}
EXPORT_SYMBOL(mt_drv_timer_read_cnt);

mt_s32  mt_drv_timer_request(int ms, timer_fn pfn, void *para, int cycled, char *name)
{
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&mtimer_lock, flags);

	for (i = 0; i < M_SYS_TIMER_MAX_NUM; i++)
	{
		if (!s_mt_timer_info[i].used)
		{
			break;
		}
	}

	if ( i >= M_SYS_TIMER_MAX_NUM)
	{
		spin_unlock_irqrestore(&mtimer_lock, flags);
		return -1;
	}


	s_mt_timer_info[i].used = 1;
	s_mt_timer_info[i].p_fn = pfn;
	s_mt_timer_info[i].p_para = para;
	s_mt_timer_info[i].cycled = cycled;
	s_mt_timer_info[i].name = name;
	s_mt_timer_info[i].timeout = ms;
	mtimer_set_timeout(i, ms);
	spin_unlock_irqrestore(&mtimer_lock, flags);
	if (pfn)
	{
		enable_irq(i+ MTIMER_IRQ_NUM);
	}
	return i;
}
EXPORT_SYMBOL(mt_drv_timer_request);

mt_s32  mt_drv_timer_release(int id)
{
	int i = id;
	unsigned long flags;
	 if (id >=M_SYS_TIMER_MAX_NUM)
	 {
	 	return -1;
	 }
	spin_lock_irqsave(&mtimer_lock, flags);


	if ( i >= M_SYS_TIMER_MAX_NUM)
	{
		spin_unlock_irqrestore(&mtimer_lock, flags);
		return -1;
	}

	s_mt_timer_info[i].used = 0;
	//s_mt_timer_info[i].p_fn = NULL;
	s_mt_timer_info[i].p_para = NULL;
	s_mt_timer_info[i].cycled = 0;
	//mtimer_disable_irq(i);
  disable_irq_nosync(MTIMER_IRQ_NUM + i);
	spin_unlock_irqrestore(&mtimer_lock, flags);


	return 0;
}
EXPORT_SYMBOL(mt_drv_timer_release);

static int mtimer_open(struct inode *inode, struct file *file)
{
    int ret = 0;

    if (test_and_set_bit(0, &driver_open))
    {
        return -EBUSY;
    }



    return ret;
}

static int mtimer_release(struct inode *inode, struct file *file)
{
    unsigned int mtimerindex = 0;
    clear_bit(0, &driver_open);

        mt_drv_timer_stop(mtimerindex);
    return 0;
}

static long mtimer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
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

static struct file_operations mtimer_fops =
{
    .owner   = THIS_MODULE,
    .llseek  = no_llseek,

    //  .write      = mtimer_write,
    .unlocked_ioctl   = mtimer_ioctl,
    .open    = mtimer_open,
    .release = mtimer_release,
};

static mt_s32 mtimer_pm_suspend (basedev_s *pdev, pm_message_t state)
{
    unsigned int mtimerindex = 0;


     mt_drv_timer_stop(mtimerindex);


    return 0;
}

static mt_s32 mtimer_pm_resume(basedev_s *pdev)
{
    unsigned int mtimerindex = 0;


    mt_drv_timer_start(mtimerindex);

    //mtimer_value = hiwdt_readl(HIWDG_VALUE);
    //printk("> %s: [%d], 0x%x, mtimer_value=0x%x\n", __FUNCTION__, __LINE__, mtimer_load, mtimer_value);
    return 0;
}

static baseops_s mtimer_baseOps = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = mtimer_pm_suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = mtimer_pm_resume
};

mt_s32 mt_drv_timer_proc_read(struct seq_file *s, mt_void *arg)
{

	int id;
	if(!sn_mtimer_base)
	{
		MT_INFO_TIMER("mtimer module not init \n");
		return 0;
	}
	MT_INFO_TIMER(" debug level  =%d \n ", s_timer_debug_level);

	for (id = 0; id < M_SYS_TIMER_MAX_NUM; id ++)
	{
	MT_INFO_TIMER("timer id %d \n", id);
	MT_INFO_TIMER(" prescalar  =%d \n ", MTIMER_READ32(R_RTC_PRESCALER +  R_RTC_ID_OFFSET * id));
	MT_INFO_TIMER(" triger   =%d \n ", MTIMER_READ32(R_RTC_TRIGGER  + R_RTC_ID_OFFSET * id));
	MT_INFO_TIMER(" preload   =%d \n ", MTIMER_READ32(R_RTC_PRELOAD + R_RTC_ID_OFFSET * id));
	MT_INFO_TIMER(" cnt   =%d \n ", MTIMER_READ32(R_RTC_COUNT + R_RTC_ID_OFFSET * id));

	}

	return 0;
}

#define MAX_FILENAME_LENTH 256

static int seperate_string_timer(char *s, char **timerid,  char **left, char **right)
{
	char *p = s;
	char *t;
	//printk(" %s  \n",  s);

	   /* skip '=' */
	while(*p == ' '){p ++;}

	*timerid  = p;

    /* find ' ' */
	while(*p != '\0' && *p++ != ':');

	if(*--p != ':')
	{
		MT_ERR_TIMER("e1 %p %p %c \n", s, p, *p);
		return -1;
	}
	*p = '\0';

    /* seperate left from right vaule by '=' */


	t = ++p;
	  /* find '=' */
	while(*p != '\0' && *p++ != '=');

	if(*--p != '=')
	{
		MT_ERR_TIMER("e2 %p %p %c \n", s, p, *p);
		MT_ERR_TIMER(" %s  \n",  *timerid);
		return -1;
	}
	*p = '\0';
 	*left = t;
 	*right = p+1;

	return 0;
}

static char *mstrip_string_timer(char *s, char *d)
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


mt_s32 mt_drv_timer_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char m[MAX_FILENAME_LENTH] = {0};
	char d[MAX_FILENAME_LENTH] = {0};
	size_t len = MAX_FILENAME_LENTH;
	char *left,  *right;
	char *timerid;
	int  level;
	int id = 0;
	if (*ppos >= MAX_FILENAME_LENTH)
		return -EFBIG;

	len = min(len, count);
	if (copy_from_user(m, buf, len))
		return -EFAULT;


	if (!sn_mtimer_base)
	{
		MT_ERR_TIMER("Log module not init\n");
		goto out;
	}
	memset(d, 0, sizeof(d));
	mstrip_string_timer(m, d);
    /* echo help info to current terinmal */
	if (!mt_osal_strncasecmp("help", m, 4))
	{
#if 1
		MT_INFO_TIMER("To modify the level, use command line in shell: \n");

	 MT_INFO_TIMER("Use 'echo   \"id:level = x\" > /proc/msp/timer' to change  printlevel.\n");
        MT_INFO_TIMER("Use 'echo   \" id:scalar = x\" > /proc/msp/timer' to change scalar cnt.\n");
	  MT_INFO_TIMER("Use 'echo  \" id :triger = x\" > /proc/msp/timer' to change triger cnt.\n");

     	goto out;
#endif
	}

#if 1
	if (seperate_string_timer(d, &timerid,&left, &right)){
        	MT_ERR_TIMER("string is unkown!\n");
		goto out;
	}
#endif

	id =  simple_strtol(timerid, NULL, 10);
	if ((id < 0) || (M_SYS_TIMER_MAX_NUM <=id))
	{
		MT_ERR_TIMER(" error  id=%d \n", id);
		return len;
	}
	MT_INFO_TIMER("  id=%d \n", id);
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
		MTIMER_WRITE32(R_RTC_PRESCALER + R_RTC_ID_OFFSET * id , level);
	}
	else if (!mt_osal_strncasecmp("triger", left, strlen("triger")+1))
	{
		MT_INFO_TIMER("triger = %d\n", level);
		MTIMER_WRITE32(R_RTC_TRIGGER + R_RTC_ID_OFFSET * id, level);
		MTIMER_WRITE32(R_RTC_PRELOAD + R_RTC_ID_OFFSET * id , 0);
	}
	else
	{
		MT_ERR_TIMER(" unknow option %s \n", m);
	}



out:
	*ppos = len;
	return len;
}


static mt_device_s g_mtimer_register_data;
unsigned int  mt_get_sys_ctrl_base(void);

mt_s32 __init mtimer_drv_modinit(mt_void)
{
    int ret = 0;
    mt_proc_entry_t *item = NULL;
    int i = 0;

	MT_INFO_TIMER("\n ... mtimer init  ... \n");
    (mt_void)mt_drv_module_register(MT_ID_TIMER, "mtimer", NULL);
	sn_mtimer_base = mt_get_sys_ctrl_base() + RTC_TIMER_OFFSET;
	MT_INFO_TIMER("\n ... mtimer init  .sn_mtimer_base=%x.. \n", sn_mtimer_base);

	memset(&s_mt_timer_info, 0, sizeof(s_mt_timer_info));

    /* Check that the default_margin value is within it's range ; if not reset to the default */

    snprintf(g_mtimer_register_data.devfs_name, sizeof(g_mtimer_register_data.devfs_name), UMAP_DEVNAME_TIMER);
    g_mtimer_register_data.minor = UMAP_MIN_MINOR_TIMER;
    g_mtimer_register_data.owner = THIS_MODULE;
    g_mtimer_register_data.fops   = &mtimer_fops;
    g_mtimer_register_data.drvops = &mtimer_baseOps;
    if (mt_drv_dev_register(&g_mtimer_register_data) < 0)
    {
        MT_ERR_TIMER(" mtimer mt_drv_dev_register err (err=%d)\n", ret);
        return MT_FAILURE;
    }

	item = mt_drv_proc_add_module(MT_MOD_TIMER, NULL, NULL);
    if(!item)
    {
		return -1;
	}

	item->read = mt_drv_timer_proc_read;
	item->write = mt_drv_timer_proc_write;



	for (i = 0; i< M_SYS_TIMER_MAX_NUM; i++)
	{
	  s_mt_timer_info[i].used = 0;
	  s_mt_timer_info[i].p_fn = NULL;
		mtimer_set_timeout(i, 0);
#if defined(CONFIG_MT_CHIP_ARIA)		//symphony linux 上影响到display，临时删除
		mtimer_enable_irq(i);
#endif
    disable_irq(MTIMER_IRQ_NUM+i);
	}

    return ret;
}


mt_void __exit mtimer_drv_modexit(mt_void)
{

    mt_drv_module_unregister(MT_ID_TIMER);

    mt_drv_dev_unregister(&g_mtimer_register_data);


    mtimer_set_timeout(0, 0);
}
