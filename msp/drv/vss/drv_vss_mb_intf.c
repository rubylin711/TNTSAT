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
#include <asm/siginfo.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/irq.h"

#include "mt_drv_dev.h"
#include "mt_debug.h"
#include "hw_mailbox.h"


#define MT_VSS_SIG_NOTIFY   0x83
#define MT_VSS_SIG_TEE      MT_VSS_SIG_NOTIFY

mt_u32 vss_app_pid = 0;

static void vss_send_signal(int sig_no)
{
    int ret;
    struct siginfo info;
    struct task_struct *app_task = NULL;

    if (0 == vss_app_pid){
        /* no_pid, please ret */
        printk("pid %d error\n", vss_app_pid);
        return;
    }

    memset(&info, 0, sizeof(struct siginfo));
    info.si_signo = sig_no;
    info.si_errno = MT_VSS_SIG_NOTIFY;
    info.si_code  = MT_VSS_SIG_TEE;

    rcu_read_lock();
    app_task = pid_task(find_vpid(vss_app_pid), PIDTYPE_PID);
    rcu_read_unlock();

    if (app_task == NULL)
    {
        MT_PRINT("Get pid_task failed! \n");
        return;
    }

    ret = send_sig_info(sig_no, (struct kernel_siginfo *)&info, app_task);
    if (ret < 0)    {
        MT_PRINT("Send signal failed! \n");
    }
}


static irqreturn_t vss_mb_tee_isr(int irq, void *dev_id)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)dev_id;


    hw_mailbox_clear_notice_interrupt(handle);

    vss_send_signal(SIGUSR1);

    return IRQ_HANDLED;
}


static irqreturn_t vss_mb_isr(int irq, void *dev_id)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)dev_id;
    //hw_mailbox_clear_interrupt(handle);
    int32_t state;

    state = hw_mailbox_check_state(handle);
    if ((state & HW_STATE_READY_FOR_READ) == HW_STATE_READY_FOR_READ) {
        /*MT_PRINT("rd int\n");*/
        hw_mailbox_clear_read_interrupt(handle);
        hw_mailbox_onoff_read_int_mask(handle, 1);
        hw_mailbox_read_block(handle);
        //handle->block_rd_notice = 1;
        wake_up_interruptible(&handle->rq);
    }

    if ((state & HW_STATE_READY_FOR_WRITE) == HW_STATE_READY_FOR_WRITE) {
        /*MT_PRINT("wr int\n");*/
        hw_mailbox_clear_write_interrupt(handle);
        handle->block_wr_echo = 1;
        wake_up_interruptible(&handle->wq);
    }

    return IRQ_HANDLED;
}

static int vss_mb_open(struct inode *inode, struct file *filp)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)get_mt_priv(iminor(inode));
    filp->private_data = handle;
    return 0;
}

static int vss_mb_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static unsigned int vss_mb_poll(struct file *filp, poll_table *wait)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)filp->private_data;
    unsigned int mask = 0;

    mutex_lock(&handle->read_write_mutex);

    poll_wait(filp, &handle->wq, wait);
    poll_wait(filp, &handle->rq, wait);

    if (handle->block_rd_len)
        mask |= POLLIN | POLLRDNORM;

    if (handle->block_wr_echo)
        mask |= POLLOUT | POLLWRNORM;

    mutex_unlock(&handle->read_write_mutex);

    return mask;
}

static ssize_t vss_mb_read(struct file *filp, char __user *buf, size_t size, loff_t *pos)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)filp->private_data;
    ssize_t size_be_read;
    int timeout = 1000; // for example 1K jiffies

    if (wait_event_interruptible_timeout(handle->rq, handle->block_rd_len, timeout) <= 1) {
        MT_PRINT("Timeout or Interrupted while reading\n");
        return -EAGAIN;
    }

    mutex_lock(&handle->read_write_mutex);
    size_be_read = size < handle->block_rd_len ? size : handle->block_rd_len;
    if (copy_to_user(buf, (char *)(handle->block_rd_buf), size_be_read)) {
        MT_PRINT("Unable to copy\n");
        size_be_read = -EFAULT;
    }
    handle->block_rd_len = 0;
    //handle->block_rd_notice = 0;
    hw_mailbox_onoff_read_int_mask(handle, 0);
    mutex_unlock(&handle->read_write_mutex);

    return size_be_read;
}

static ssize_t vss_mb_write(struct file *filp, const char __user *buf, size_t size, loff_t *pos)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)filp->private_data;
    ssize_t size_be_write;
    int timeout = 1000; // for example 1K jiffies

    handle->block_wr_len = size < MAILBOX_FIFO_DEPTH ? size: MAILBOX_FIFO_DEPTH;
    if (copy_from_user(handle->block_wr_buf, (void __user *)buf, handle->block_wr_len)>0)
        return -EFAULT;

    handle->block_wr_echo = 0;
    size_be_write = hw_mailbox_write_block(handle);
    if (size_be_write > 0) {
        if (wait_event_interruptible_timeout(handle->wq, handle->block_wr_echo, timeout) <= 1) {
            MT_PRINT("Timeout or Interrupted while writing\n");
            return -EAGAIN;
        }
    }

    return size_be_write;
}

static mt_device_s vss_mb_dev;

static struct file_operations vss_mb_fops = {
    .owner = THIS_MODULE,
    .read = vss_mb_read,
    .write = vss_mb_write,
    .open = vss_mb_open,
    .release = vss_mb_release,
    .poll = vss_mb_poll,
};

int vss_mb_init(void)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)kmalloc(sizeof(mailbox_handle_t), GFP_KERNEL);
    int ret;

    memset(handle, 0, sizeof(mailbox_handle_t));

    init_waitqueue_head(&handle->wq);
    init_waitqueue_head(&handle->rq);
    mutex_init(&handle->read_write_mutex);

    handle->base = SYMPHONY_VSS_MB_REGISTER_VIRT_BASE;
    handle->irq = IRQ_VSS_MB_ID;
    ret = request_irq(handle->irq, vss_mb_isr, IRQF_TRIGGER_HIGH, "mt_vss_mb", handle);
    if (ret < 0)
        goto fail;  

    handle->tee_base = SYMPHONY_VSS_MB_TEE_REGISTER_VIRT_BASE;
    handle->tee_irq = IRQ_VSS_TEE_ID;
    ret = request_irq(handle->tee_irq, vss_mb_tee_isr, IRQF_TRIGGER_HIGH, "mt_vss_mb", handle);
    if (ret < 0){
        MT_PRINT("request irq ret = %d failed.\n", ret);
        goto fail;
    }

    vss_mb_dev.minor = UMAP_MIN_MINOR_VSS_MB;
    vss_mb_dev.owner= THIS_MODULE;
    vss_mb_dev.fops = &vss_mb_fops;
    vss_mb_dev.priv = (void *)handle;
    sprintf(vss_mb_dev.devfs_name, UMAP_DEVNAME_VSS_MB);

    ret = mt_drv_dev_register(&vss_mb_dev);
    if (ret < 0) {
        MT_PRINT("register %s failed.\n", vss_mb_dev.devfs_name);
        goto fail;
    }

    MT_PRINT("vss mb init success\n");
    return 0;

fail:
    kfree(handle);
    return ret;
}

void vss_mb_exit(void)
{
    mailbox_handle_t *handle = (mailbox_handle_t *)vss_mb_dev.priv;

    free_irq(handle->irq, handle);
    handle->irq = 0;

    kfree(handle);

    mt_drv_dev_unregister(&vss_mb_dev);

    MT_PRINT("vss mb exit success\n");
}
