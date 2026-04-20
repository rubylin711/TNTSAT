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
#include <linux/delay.h>

#include "mt_mach/irq.h"

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

#define RTC_TIMER_OFFSET 0x80
#define R_RTC_INIT_BASE 0x0
#define R_RTC_CAP_BASE 0x10
#define R_RTC_CW 0x20
#define MTTIMER_T01_CASC (16)
#define MTTIMER_T23_CASC (20)


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

#define MTIMER_WRITE32(addr,  v) writel(v, (void __iomem *)((ulong)((MTIMER_REG_BASE) + addr)))
#define MTIMER_READ32(addr) readl((void __iomem *)((ulong)((MTIMER_REG_BASE) + addr)))

#define R_RTC_ID_OFFSET 0x4


#define  MTIMER_INT_BIT       0x1

#define MTIMER_IRQ_NUM IRQ_T0_ID

/* debug */

static  mt_symp_timer_info  s_mt_timer_info[M_SYS_TIMER_MAX_NUM];


/* local var */
static DEFINE_SPINLOCK(mtimer_lock);

static unsigned long driver_open = 0;
static mt_s32 mtimer_pm_suspend (basedev_s *pdev, pm_message_t state);
static mt_s32 mtimer_pm_resume (basedev_s *pdev);

static struct fasync_struct *async;


static mt_s32 timer_fasync(mt_s32 fd, struct file *filp, mt_s32 mode)
{
     mt_s32 retval;
     retval = fasync_helper(fd, filp, mode,&async);
     if(retval < 0)
     {
         MT_INFO_TIMER("timer_fasync error\n");
     }

     return retval;
}

static irqreturn_t  mtimer_isr(int irq,void * para)
{
    mt_symp_timer_info *p_info = (mt_symp_timer_info *)para;
    int id = irq - MTIMER_IRQ_NUM;
    MT_INFO_TIMER(KERN_INFO "irq:%d id:%d\n",irq, id);
    if(p_info->p_fn)
    {
        p_info->p_fn(NULL);
    }
    if(async)
    {
        kill_fasync(&async, SIGIO, POLL_IN);
    }
    return IRQ_HANDLED;
}

static void mtimer_enable_irq(unsigned int mtimerindex)
{
    //unsigned long flags;
    int err;

    /* register isr */
    err = request_irq(MTIMER_IRQ_NUM + mtimerindex, mtimer_isr,  IRQF_SHARED, "mtimer", &s_mt_timer_info[mtimerindex]);
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


void mtimer_set_timeout(int id, unsigned int ms)
{
    //unsigned long flags;
    unsigned  int cnt = ms;

    unsigned  int mask = 1 << id;

    cnt = (MTIMER_CLK_HZ/1000 * ms);
    if (cnt >= ~(0UL))
    {
        cnt = ~0x0;
    }
    //cascade mode need to be impl
    MTIMER_WRITE32(R_RTC_INIT_BASE + R_RTC_ID_OFFSET * id, cnt);
    //MT_INFO_TIMER("%s %d  cnt:%d init:0x%x\n", __FUNCTION__, __LINE__, cnt, MTIMER_READ32(R_RTC_INIT_BASE + R_RTC_ID_OFFSET * id));
}



mt_s32  mt_drv_timer_start(int id, mt_timer_info *p_info)
{
    unsigned  int dtmp;

    dtmp = MTIMER_READ32(R_RTC_CW);

    dtmp &= ~(0x3 << id * R_RTC_ID_OFFSET);
    if(!p_info->cycled)
    dtmp |= (0x2 << id * R_RTC_ID_OFFSET);
    dtmp |= (0x1 << id * R_RTC_ID_OFFSET);
    MTIMER_WRITE32(R_RTC_CW, dtmp);
    return 0;
}

mt_s32  mt_drv_timer_stop(int id)
{
    unsigned  int dtmp;

    dtmp = MTIMER_READ32(R_RTC_CW);

    dtmp &= ~(0x3 << id * R_RTC_ID_OFFSET);
    MTIMER_WRITE32(R_RTC_CW, dtmp);
    return 0;
}
#endif
mt_u32  mt_drv_timer_read_cnt(int id)
{
    mt_u32 dtmp = 0;
    if (id >= M_SYS_TIMER_MAX_NUM)
    {
        return dtmp;
    }
    dtmp = MTIMER_READ32((R_RTC_CAP_BASE + R_RTC_ID_OFFSET * id));
    return dtmp;
}
EXPORT_SYMBOL(mt_drv_timer_read_cnt);


mt_s32  mt_drv_timer_request(int ms, timer_fn pfn, void *para, int cycled, char *name)
{
    int i = 0;
    int casc = 0;
    unsigned long long trig_cnt = 0;
    unsigned int dtmp = 0;
    unsigned long flags;

    spin_lock_irqsave(&mtimer_lock, flags);
    trig_cnt = (MTIMER_CLK_HZ/1000 * ms);

    if(trig_cnt > ~(0UL))
        casc = 1;
    for (i = 0; i < M_SYS_TIMER_MAX_NUM; i++)
    {
        if (!s_mt_timer_info[i].used)
        {
            break;
        }
    }
    if(casc)
    {
        if((s_mt_timer_info[0].used || s_mt_timer_info[1].used)
            && (s_mt_timer_info[2].used || s_mt_timer_info[3].used))
        {
            MT_ERR_TIMER(KERN_ERR "Error, no hw timer available.\n");
            return -EBUSY;
        }
        if(!(s_mt_timer_info[0].used || s_mt_timer_info[1].used))
            i = 1;
        else
            i = 3;
    }

    if ( i >= M_SYS_TIMER_MAX_NUM)
    {
		MT_ERR_TIMER(KERN_ERR "Error, no hw timer available.\n");
        spin_unlock_irqrestore(&mtimer_lock, flags);
        BUG();
        return -1;
    }

    s_mt_timer_info[i].used = 1;
    s_mt_timer_info[i].p_fn = pfn;
    s_mt_timer_info[i].p_para = para;
    s_mt_timer_info[i].cycled = cycled;
    s_mt_timer_info[i].name = name;
    s_mt_timer_info[i].timeout = ms;
    dtmp = MTIMER_READ32(R_RTC_CW);
    if(casc)
    {
        dtmp |= (0x1 << ((i==1)?MTTIMER_T01_CASC:MTTIMER_T23_CASC));
        MTIMER_WRITE32(R_RTC_INIT_BASE + R_RTC_ID_OFFSET * (i - 1), (unsigned int)trig_cnt);
        MTIMER_WRITE32(R_RTC_INIT_BASE + R_RTC_ID_OFFSET * i , (unsigned int)(trig_cnt >> 32));
    }
    else
    {
        MTIMER_WRITE32(R_RTC_INIT_BASE + R_RTC_ID_OFFSET * i, (unsigned int)trig_cnt);
    }

    dtmp &= ~(0x3 << (R_RTC_ID_OFFSET * i));
    if(!cycled)
        dtmp |= (0x2 << (R_RTC_ID_OFFSET * i));  //set cycle
    dtmp |= (0x1 << (R_RTC_ID_OFFSET * i)); //start timer
    MTIMER_WRITE32(R_RTC_CW,  dtmp);

    //mtimer_set_timeout(i, ms);
    //mt_drv_timer_start(i);
    spin_unlock_irqrestore(&mtimer_lock, flags);
    if(1)   //if (pfn)
    {
        enable_irq(i+ MTIMER_IRQ_NUM);
    }
    return i;
}
EXPORT_SYMBOL(mt_drv_timer_request);

mt_s32  mt_drv_timer_release(int id)
{
    int i = id;
    unsigned int dtmp = 0;
    unsigned long flags;
    if (id >=M_SYS_TIMER_MAX_NUM)
    {
        return -1;
    }
    spin_lock_irqsave(&mtimer_lock, flags);

    s_mt_timer_info[i].used = 0;
    //s_mt_timer_info[i].p_fn = NULL;
    s_mt_timer_info[i].p_para = NULL;
    s_mt_timer_info[i].cycled = 0;
    //mtimer_disable_irq(i);
    dtmp = MTIMER_READ32(R_RTC_CW);
    dtmp &= ~(0x3 << (R_RTC_ID_OFFSET * i)); //clear start & cycle bit
    if(i == 1 || i == 3) //clear casc bit
        dtmp &= ~(0x1 << ((i==1)?MTTIMER_T01_CASC:MTTIMER_T23_CASC));
    MTIMER_WRITE32(R_RTC_CW,  dtmp);
    //mt_drv_timer_stop(i);

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
    clear_bit(0, &driver_open);
    timer_fasync(-1, file, 0);
    return 0;
}

static long mtimer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    void __user *argp = (void __user *)arg;
    int __user *p = argp;

    switch (cmd)
    {
        case TIMERIOC_SET_START:
        {
            int index = 0;

            if (get_user(index, p))
            {
                return -EFAULT;
            }
            //no implement now;
        }
        return 0;

        case TIMERIOC_SET_STOP:
        {
            int index = 0;

            if (get_user(index, p))
            {
                return -EFAULT;
            }
            ////no implement now;
        }
        return 0;

        case TIMERIOC_SET_RELEASE:
        {
            int index = 0;

            if (get_user(index, p))
            {
                return -EFAULT;
            }

            if(mt_drv_timer_release(index) != 0)
                return MT_FAILURE;
            else
                return 0;
        }

        case TIMERIOC_GET_CNT:
        {
            timer_cnt_s gtOption;

            if (copy_from_user(&gtOption, (timer_cnt_s*)arg, sizeof(timer_cnt_s)))
            {
                return -EFAULT;
            }

             if (gtOption.u32TimerIndex  >= M_SYS_TIMER_MAX_NUM)
            {
                return MT_FAILURE;
            }
            gtOption.u32cnt = mt_drv_timer_read_cnt(gtOption.u32TimerIndex);
            return copy_to_user((timer_cnt_s*)arg, &gtOption, sizeof(timer_cnt_s));
        }

        case TIMERIOC_REQUEST:
        {
            mt_symp_timer_info stOption;
            timer_fn p_fn;
	     void *para;
	     int  cycled;
	     int ms;
	     char * name;
            int timer_id;


            if (copy_from_user(&stOption, (mt_symp_timer_info*)arg, sizeof(mt_symp_timer_info)))
            {
                return -EFAULT;
            }

            p_fn = NULL;
            para = stOption.p_para;
            cycled = stOption.cycled;
            ms = stOption.timeout;
            name = stOption.name;

            timer_id = mt_drv_timer_request(ms, p_fn, para, cycled, name);

            if(timer_id >= M_SYS_TIMER_MAX_NUM)
            {
                return MT_FAILURE;
            }
            else
            {
                stOption.timer_id = timer_id;
                return copy_to_user((mt_symp_timer_info*)arg, &stOption, sizeof(mt_symp_timer_info));
            }
        }


        default:
        return -ENOIOCTLCMD;
    }

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
    .fasync = timer_fasync,
};

static mt_s32 mtimer_pm_suspend (basedev_s *pdev, pm_message_t state)
{
    //unsigned int mtimerindex = 0;

    //mt_drv_timer_stop(mtimerindex);
    return 0;
}

static mt_s32 mtimer_pm_resume(basedev_s *pdev)
{
    //unsigned int mtimerindex = 0;


    //mt_drv_timer_start(mtimerindex);

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
        MT_INFO_TIMER(" cnt  =0x%x \n ", MTIMER_READ32(R_RTC_CAP_BASE  + R_RTC_ID_OFFSET * id));
        MT_INFO_TIMER(" triger   =0x%x \n ", MTIMER_READ32(R_RTC_INIT_BASE  + R_RTC_ID_OFFSET * id));
    }
    MT_INFO_TIMER(" cw   =0x%x \n ", MTIMER_READ32(R_RTC_CW));

    return 0;
}
#if 0
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
        printk("e1 %p %p %c \n", s, p, *p);
        return -1;
    }
    *p = '\0';

    /* seperate left from right vaule by '=' */


    t = ++p;
    /* find '=' */
    while(*p != '\0' && *p++ != '=');

    if(*--p != '=')
    {
        printk("e2 %p %p %c \n", s, p, *p);
        printk(" %s  \n",  *timerid);
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

#endif
mt_s32 mt_drv_timer_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
#if 0
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
        printk("Log module not init\n");
        goto out;
    }
    memset(d, 0, sizeof(d));
    mstrip_string_timer(m, d);
    /* echo help info to current terinmal */
    if (!mt_osal_strncasecmp("help", m, 4))
    {
#if 1
        printk("To modify the level, use command line in shell: \n");

        printk("Use 'echo   \"id:level = x\" > /proc/msp/timer' to change  printlevel.\n");
        printk("Use 'echo   \" id:scalar = x\" > /proc/msp/timer' to change scalar cnt.\n");
        printk("Use 'echo  \" id :triger = x\" > /proc/msp/timer' to change triger cnt.\n");

        goto out;
#endif
    }

#if 1
    if (seperate_string_timer(d, &timerid,&left, &right)){
        printk("string is unkown!\n");
        goto out;
    }
#endif

    id =  simple_strtol(timerid, NULL, 10);
    if ((id < 0) || (M_SYS_TIMER_MAX_NUM <=id))
    {
        printk(" error  id=%d \n", id);
        return len;
    }
    printk("  id=%d \n", id);
    level = simple_strtol(right, NULL, 10);
    printk("  option %s %s \n", left, right);

    if (!mt_osal_strncasecmp("level", left, strlen("level")+1))
    {
        printk("level = %d \n", level);
        s_timer_debug_level = level;
    }
    else if (!mt_osal_strncasecmp("scalar", left, strlen("scalar")+1))
    {
        printk("scalar = %d\n", level);
        MTIMER_WRITE32(R_RTC_PRESCALER + R_RTC_ID_OFFSET * id , level);
    }
    else if (!mt_osal_strncasecmp("triger", left, strlen("triger")+1))
    {
        printk("triger = %d\n", level);
        MTIMER_WRITE32(R_RTC_TRIGGER + R_RTC_ID_OFFSET * id, level);
        MTIMER_WRITE32(R_RTC_PRELOAD + R_RTC_ID_OFFSET * id , 0);
    }
    else
    {
        printk(" unknow option %s \n", m);
    }



    out:
    *ppos = len;
    return len;
#endif
    return 0;
}
static mt_device_s g_mtimer_register_data;
unsigned int  mt_get_pic0_base(void);

mt_s32 __init mtimer_drv_modinit(mt_void)
{
    int ret = 0;
    mt_proc_entry_t *item = NULL;
    int i = 0;

    MT_INFO_TIMER("\n ... mtimer init  ... \n");
    (mt_void)mt_drv_module_register(MT_ID_TIMER, "mtimer", NULL);
    sn_mtimer_base = mt_get_pic0_base() + RTC_TIMER_OFFSET;
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
        mtimer_enable_irq(i);
        disable_irq(MTIMER_IRQ_NUM+i);
    }

/* test sample */
#if 0
	extern void mt_drv_kn_timer_sample(void);

	mt_drv_kn_timer_sample();
#endif

    return ret;
}


mt_void __exit mtimer_drv_modexit(mt_void)
{
    mt_drv_module_unregister(MT_ID_TIMER);
    mt_drv_dev_unregister(&g_mtimer_register_data);
    //mtimer_set_timeout(0, 0);
}
