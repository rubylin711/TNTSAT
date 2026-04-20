/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/kthread.h>

#include "mt_common.h"

#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "drv_timer.h"
#include "mt_module_debug.h"

/* local var */

static unsigned long driver_open = 0;
static int user_layer_timer_id = -1;
static mt_s32 mtimer_pm_suspend(basedev_s *pdev, pm_message_t state);
static mt_s32 mtimer_pm_resume(basedev_s *pdev);

extern void mt_drv_timer_init(void);
extern void mt_drv_timer_dump_state(void);

extern void mt_drv_timer_reg_suspend(void);
extern void mt_drv_timer_reg_resume(void);

static struct fasync_struct *async;

static mt_s32 timer_fasync(mt_s32 fd, struct file *filp, mt_s32 mode)
{
     mt_s32 retval = 0;

     retval = fasync_helper(fd, filp, mode, &async);

     if(retval < 0)
     {
         MT_INFO_TIMER("timer_fasync error\n");
     }

     return retval;
}

irqreturn_t mtimer_hsr(int irq, void *para)
{
    if (async)
    {
        kill_fasync(&async, SIGIO, POLL_IN);
    }

    return IRQ_HANDLED;
}

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
	     {
                return MT_FAILURE;
            }
	     else
	     {
		  user_layer_timer_id = -1;
		  return 0;
            }
        }

        case TIMERIOC_GET_CNT:
        {
            timer_cnt_s gtOption;

            if (copy_from_user(&gtOption, (timer_cnt_s*)arg, sizeof(timer_cnt_s)))
            {
                return -EFAULT;
            }

            /*if (gtOption.u32TimerIndex >= M_SYS_TIMER_MAX_NUM)
            {
                return MT_FAILURE;
            }*/
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
	     if(user_layer_timer_id  != -1)   //only one hardware_timer be used by user_layer api
	     {
		  return -EFAULT;
	     }
            p_fn = NULL;
            para = stOption.p_para;
            cycled = stOption.cycled;
            ms = stOption.timeout;
            name = stOption.name;
            timer_id = mt_drv_timer_request(ms, p_fn, para, cycled, name);
	     
            if (timer_id < 0)
            {
                return MT_FAILURE;
            }
            else
            {
                stOption.timer_id = timer_id;
		  user_layer_timer_id = timer_id;
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
    .unlocked_ioctl = mtimer_ioctl,
    .open           = mtimer_open,
    .release        = mtimer_release,
    .fasync         = timer_fasync,
};

static mt_s32 mtimer_pm_suspend (basedev_s *pdev, pm_message_t state)
{
	mt_drv_timer_reg_suspend();
	MT_PRINT("TIMER suspend OK\n");
    return 0;
}

static mt_s32 mtimer_pm_resume(basedev_s *pdev)
{
	mt_drv_timer_reg_resume();
	MT_PRINT("TIMER resume OK\n");
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
	mt_drv_timer_dump_state();

    return 0;
}

mt_s32 mt_drv_timer_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static mt_device_s g_mtimer_register_data;

mt_s32 __init mtimer_drv_modinit(mt_void)
{
	mt_s32 ret;
    mt_proc_entry_t *item = NULL;

    MT_INFO_TIMER("\n ... mtimer init  ... \n");
    (mt_void)mt_drv_module_register(MT_ID_TIMER, "mtimer", NULL);

    mt_drv_timer_init();

    /* Check that the default_margin value is within it's range ; if not reset to the default */

    snprintf(g_mtimer_register_data.devfs_name, sizeof(g_mtimer_register_data.devfs_name), UMAP_DEVNAME_TIMER);
    g_mtimer_register_data.minor  = UMAP_MIN_MINOR_TIMER;
    g_mtimer_register_data.owner  = THIS_MODULE;
    g_mtimer_register_data.fops   = &mtimer_fops;
    g_mtimer_register_data.drvops = &mtimer_baseOps;
    if ((ret=mt_drv_dev_register(&g_mtimer_register_data)) < 0)
    {
        MT_ERR_TIMER(" mtimer mt_drv_dev_register err (err=%d)\n", ret);
        return MT_FAILURE;
    }

    item = mt_drv_proc_add_module(MT_MOD_TIMER, NULL, NULL);
    if (item == NULL)
    {
    	MT_ERR_TIMER(" mt_drv_proc_add_module failed!\n");
        return MT_FAILURE;
    }

    item->read = mt_drv_timer_proc_read;
    item->write = mt_drv_timer_proc_write;

/* test sample */
#if 0
	extern void mt_drv_kn_timer_sample(void);

	mt_drv_kn_timer_sample();
#endif

/* test case */
#if 0
	extern void testcase_timer_non_cascade(void);
	extern void testcase_timer_cascade(void);

	testcase_timer_non_cascade();

	testcase_timer_cascade();
#endif

    return 0;
}

mt_void __exit mtimer_drv_modexit(mt_void)
{
    mt_drv_module_unregister(MT_ID_TIMER);
    mt_drv_dev_unregister(&g_mtimer_register_data);
}

