/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <mt_mach/symphony_regs.h>
#include "mt_mach/symphony_io.h"

#include "drv_vss_ioctl.h"
#include "mt_drv_dev.h"
#include "mt_debug.h"

#define VSS_RESET_REG (0xbf500034)
extern mt_u32 vss_app_pid;

static int vss_base_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int vss_base_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static int vss_base_reset(void)
{
    static int    reset_1st        = 0;
    static int    one_sec          = 1 * HZ;
    static long   last_time        = 0;
    static long   current_time     = 0;
    long          interval_seconds = 0;


    current_time     = jiffies;

    interval_seconds = current_time - last_time;

    if((reset_1st == 0) || (interval_seconds >= one_sec))
    {
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(VSS_RESET_REG),0);
        mdelay(100);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(VSS_RESET_REG),1);
        mdelay(100);

        reset_1st = 0xa5;
        last_time = jiffies;
    }else{
        printk(KERN_ERR "Next reset must bigger than 1 second!\n");
        return -1;
    }

    return 0;
}

static mt_s32 vss_base_ioctl_core(struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg)
{
    CMD_VSS_SET_PID_S *app_pid = (CMD_VSS_SET_PID_S *)arg;

	switch(cmd) {
		case VSS_IOC_RESET:
			return vss_base_reset();

        case VSS_IOC_SET_PID:
            vss_app_pid = app_pid->pid;
            break;

		default:
			break;
	}
	return 0;
}

static long vss_base_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return mt_drv_usercopy(filp->f_path.dentry->d_inode, filp, cmd, arg, vss_base_ioctl_core);
}

static mt_device_s vss_base_dev;

static struct file_operations vss_base_fops = {
    .owner = THIS_MODULE,
    .open = vss_base_open,
    .release = vss_base_release,
    .unlocked_ioctl = vss_base_ioctl,
};

int vss_base_init(void)
{
    int ret;

    memset(&vss_base_dev, 0, sizeof(vss_base_dev));
    vss_base_dev.minor = UMAP_MIN_MINOR_VSS_BASE;
    vss_base_dev.owner= THIS_MODULE;
    vss_base_dev.fops = &vss_base_fops;
    sprintf(vss_base_dev.devfs_name, UMAP_DEVNAME_VSS_BASE);

    ret = mt_drv_dev_register(&vss_base_dev);
    if (ret < 0) {
        MT_PRINT("register %s failed.\n", vss_base_dev.devfs_name);
        goto out;
    }

    MT_PRINT("vss base init success\n");
out:
    return ret;
}

void vss_base_exit(void)
{
    MT_PRINT("vss base exit success\n");
}
