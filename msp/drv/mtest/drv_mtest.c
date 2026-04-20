#include <linux/version.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/sched.h>

#include "mt_type.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "mt_drv_log.h"
#include "mt_debug.h"

#include "drv_mtest_ioctl.h"
#include "drv_timer.h"

static mt_device_s g_XXX_Dev;
#define XXX_name "mtest"
#define XXX_name_minor_id 100

//static u32 mtest_paddr = 0;
static u8 *mtest_vaddr = 0;

static mmz_buffer_s MtestBuf;

mt_s32 _mtest_open(struct inode *inode, struct file *filp)
{
    MT_INFO_LOG("----------------\n");
    return 0;
}

mt_s32 _mtest_close(struct inode *inode, struct file *filp)
{
    MT_INFO_LOG("----------------\n");
    return 0;
}

mt_s32 _mtest_check_pattern(mtest_option_s *p_info)
{
    u32 len = p_info->len;
    u32 i = 0;
    u32 j = 0;
    u8 *p_buf;
    u8 pattern;
    //printk("-------%s--offset=%d--p=%d  len=%d-----\n",__FUNCTION__, p_info->offset, p_info->pattern, p_info->len);
    if (mtest_vaddr == NULL) {
	MT_ERR_LOG(" mtest invalid vaddr \n");
	return -1;
    }
    p_buf = mtest_vaddr + p_info->offset;
    pattern = p_info->pattern;
    // printk("----p_buf=0x%x	 p=%d	\n",   p_buf, pattern);

    for (i = 0; i < len; i++) {
	if (p_buf[i] != pattern) {
	    //printk("----p_buf[%d]=%d   pa=%d  \n", i,  p_buf[i], pattern);
	    // break;
	    if (0 == j)
		MT_ERR_LOG("----p_buf[%d]=%d   pa=%d  \n", i, p_buf[i], pattern);
	    j++;
	}
    }

    if (j)
	MT_INFO_LG("----p_buf=0x%x	 p=%d	error cnt=%d \n", (unsigned int)p_buf, pattern, j);
    return 0;
}

static long _mtest_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    // void __user *argp = (void __user *)arg;
    // int __user *p = argp;
    mtest_option_s soption;

    switch (cmd) {

    case MTEST_SET_CHECK_PATTERN: {

	if (copy_from_user(&soption, (mtest_option_s *)arg, sizeof(mtest_option_s))) {
	    return -EFAULT;
	}
	_mtest_check_pattern(&soption);

    } break;
    case MTEST_SET_OPTIONS: {

	if (copy_from_user(&soption, (mtest_option_s *)arg, sizeof(mtest_option_s))) {
	    return -EFAULT;
	}
	MT_INFO_LOG("---mtest set 0x%x---len=%d---\n", soption.phyaddr, soption.len);

	if (soption.phyaddr) {
	    mt_drv_mmz_unmap_and_release(&MtestBuf);
	    MtestBuf.u32StartPhyAddr = 0;
	} else {
	    mtest_vaddr = NULL;
	}

	if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("mtest_PoolBuf", NULL, soption.len, 0, &MtestBuf)) {
	    MT_ERR_LOG("memory allocate failed\n");

	    return -1;
	}

	mtest_vaddr = (u8 *)MtestBuf.u32StartVirAddr;

	soption.len = MtestBuf.u32Size;
	soption.phyaddr = MtestBuf.u32StartPhyAddr;
	MT_INFO_LOG("memory allocate va=0x%x  pa=0x%x len=%d \n", MtestBuf.u32StartVirAddr, MtestBuf.u32StartPhyAddr, soption.len);

	if (copy_to_user((mtest_option_s *)arg, &soption, sizeof(mtest_option_s))) {
	    return -EFAULT;
	}
    }

    default:
	break;
    }
    return 0;
}

static struct file_operations mtest_fops =
    {
        .read = NULL,
        .write = NULL,
        .open = _mtest_open,
        .release = _mtest_close,
        .unlocked_ioctl = _mtest_ioctl,
    };

static baseops_s mtest_drv_fops =
    {
        .probe = NULL,
        .remove = NULL,
        .shutdown = NULL,
        .prepare = NULL,
        .complete = NULL,
        .suspend = NULL,
        .resume = NULL,
    };

#define MAX_TEST_BUFF_SIZE (1 * 1024 * 1024)
#define MAX_TEST_LOOP 200

void mtest_speed(void)
{
    int mhz = HZ;
    int start;
    int stop;
    int i;
    dma_addr_t p_dma1;
    dma_addr_t p_dma2;
    void *p_start1;
    void *p_start2;
    p_start1 = dma_alloc_coherent(NULL, MAX_TEST_BUFF_SIZE,
                                  &p_dma1, GFP_KERNEL);
    p_start2 = dma_alloc_coherent(NULL, MAX_TEST_BUFF_SIZE,
                                  &p_dma2, GFP_KERNEL);

    if ((NULL == p_start1) || (NULL == p_start2)) {
	MT_ERR_LOG(" mtest malloc failed");
	return;
    }
    start = jiffies;
    for (i = 0; i < MAX_TEST_LOOP; i++) {
	memcpy(p_start1, p_start2, MAX_TEST_BUFF_SIZE);
    }
    stop = jiffies;

    MT_INFO_LOG("\n mtest start=%d stop=%d %d HZ=%d \n", start, stop, stop - start, mhz);
    dma_free_coherent(NULL, MAX_TEST_BUFF_SIZE, p_start1, p_dma1);
    dma_free_coherent(NULL, MAX_TEST_BUFF_SIZE, p_start2, p_dma2);
}

void mtest_speed2(void)
{
    int mhz = HZ;
    int start;
    int stop;
    int i;
    void *p_start1;
    void *p_start2;
    p_start1 = kmalloc(MAX_TEST_BUFF_SIZE, GFP_KERNEL);
    p_start2 = kmalloc(MAX_TEST_BUFF_SIZE, GFP_KERNEL);

    if ((NULL == p_start1) || (NULL == p_start2)) {
	MT_ERR_LOG(" mtest kmalloc failed");
	return;
    }
    start = jiffies;
    for (i = 0; i < MAX_TEST_LOOP; i++) {
	memcpy(p_start1, p_start2, MAX_TEST_BUFF_SIZE);
    }
    stop = jiffies;

    MT_INFO_LOG(" kmalloc mtest start=%d stop=%d   %d HZ=%d \n", start, stop, stop - start, mhz);
    kfree(p_start1);
    kfree(p_start2);
}

static int kernel_test_task(void *unused)
{
    unsigned int cpu = smp_processor_id();
    unsigned int c2;
    unsigned int c3;
    MT_INFO_LOG(" ..... task run in cpu =%d \n", cpu);

    while (1) {
	//u = smp_processor_id();
	cpu = mt_drv_timer_read_cnt(0);
	MT_INFO_LOG("task sleep  \n");
	c3 = msleep_interruptible(30);
	clear_tsk_thread_flag(current, TIF_SIGPENDING);
	c2 = mt_drv_timer_read_cnt(0);
	MT_INFO_LOG("task  r =%d  t=%d  %d \n", c3, c2 - cpu, c2);
    }

    return 0;
}

static pid_t test_pid;
static void timer_test(void *para)
{
    int rc;
    rc = wake_up_process((struct task_struct *)para);
    set_tsk_thread_flag(current, TIF_SIGPENDING);
    set_tsk_need_resched((struct task_struct *)para);
    //printk(" rc=%d \n", rc);
}

int __init mtest_drv_init(void)
{
    struct cpumask cpumask;
    long rc;
    snprintf(g_XXX_Dev.devfs_name, sizeof(g_XXX_Dev.devfs_name), XXX_name);
    g_XXX_Dev.minor = XXX_name_minor_id;
    // g_XXX_Dev.owner = THIS_MOUDLE;
    g_XXX_Dev.fops = &mtest_fops;
    g_XXX_Dev.drvops = &mtest_drv_fops;
    MtestBuf.u32StartPhyAddr = 0;
    MT_INFO_LOG("\n  ...mtest_drv_init  !!! \n");
    	mtest_speed();
     	mtest_speed2();

    return 0;

    if (mt_drv_dev_register(&g_XXX_Dev) < 0) {

	MT_ERR_LOG("mt_drv_dev_register err!!!");
	return -1;
    }

#ifdef TEST_TASK_SCHED
    //memset(&cpumask, 0, sizeof(cpumask));
    //cpumask_set_cpu(1, &cpumask);
    kthread_run
    test_pid = kernel_thread(kernel_test_task, NULL, CLONE_FS);

//rc = sched_setaffinity(test_pid, &cpumask);

#else

    MT_INFO_LOG("  ...mtest_drv_init   22!!!");
    struct task_struct *t = kthread_run(kernel_test_task, NULL, "mtest");
    mt_drv_timer_request(3, timer_test, t, 1, "mtest");
#endif
    msleep(100);

    MT_INFO_LOG(" test_pid =%d  rc=%d  \n", (int)test_pid, (int)rc);

    return 0;
}

void __exit mtest_drv_exit(void)
{
    mt_drv_dev_unregister(&g_XXX_Dev);
}

