/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include <net/netlink.h>
#include <net/sock.h>

#include <asm/io.h>
#include <mt_mach/symphony_regs.h>

#include "mt_drv_dev.h"
#include "mt_debug.h"
#include "hw_opc.h"
#include <mt_drv_opc.h>

static opc_handle_t *g_opc_handle = NULL;
struct sock *nl_sk = NULL;

static void opc_nl_send(void *data, int len)
{
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh_out;

	skb_out = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC);
	if (!skb_out) {
		printk(KERN_ERR "OPC : Failed to allocate new skb\n");
		return;
	}

	nlh_out = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, len, 0);
	memcpy(nlmsg_data(nlh_out), data, len);

	netlink_broadcast(nl_sk, skb_out, 0, 1, GFP_KERNEL);
}

static void opc_nl_send_status(void *hdl)
{
	opc_handle_t *handle = (opc_handle_t *)hdl;

	opc_nl_send(&handle->opc_status, sizeof(handle->opc_status));
}

static void opc_nl_recv(struct sk_buff *skb) {
	opc_handle_t *handle = (opc_handle_t *)g_opc_handle;
	struct nlmsghdr *nlh;
	char *umsg = NULL;

	nlh = nlmsg_hdr(skb);
	umsg = NLMSG_DATA(nlh);
	if(umsg){
		printk("kernel recv from user: %s, pid=0x%x\n", umsg, NETLINK_CB(skb).portid);
	}

	opc_nl_send_status(handle);
}

static int opc_netlink_init(void) {
	struct netlink_kernel_cfg cfg = {
		.input = opc_nl_recv,
	};

	nl_sk = netlink_kernel_create(&init_net, OPC_NETLINK_ID, &cfg);
	if (!nl_sk) {
		printk(KERN_ALERT "Error creating socket.\n");
		return -ENOMEM;
	}

	return 0;
}

static void mt_opc_irq_work(struct work_struct *work)
{
	opc_handle_t *handle = (opc_handle_t *)g_opc_handle;

	wake_up_interruptible(&handle->opcq);

	/* Broadcast the status to application if connected */
	opc_nl_send_status(handle);


	/* delay 3s to enable opc irq again */
	schedule_delayed_work(&handle->delay_work, msecs_to_jiffies(handle->interval_ticks));
}


static irqreturn_t mt_opc_isr(int irq, void *dev_id)
{
	opc_handle_t *handle = (opc_handle_t *)dev_id;

	/* mask and get status, and unmask after 1 second later */
	hw_opc_onoff_int_mask(handle, OPC_IRQ_OFF);
	hw_opc_clear_interrupt(handle);
	handle->opc_status = hw_opc_get_status(handle);

	schedule_work(&handle->work);

    return IRQ_HANDLED;
}

static void mt_opc_delay_work(struct work_struct *work)
{
	opc_handle_t *handle = (opc_handle_t *)g_opc_handle;

	/* Enable the opc irq ree */
	mutex_lock(&handle->opc_mutex);
	handle->opc_status = hw_opc_get_status(handle);
	hw_opc_onoff_int_mask(handle, OPC_IRQ_ON);
	mutex_unlock(&handle->opc_mutex);
}

static int mt_opc_open(struct inode *inode, struct file *filp)
{
	opc_handle_t *handle = (opc_handle_t *)get_mt_priv(iminor(inode));
	filp->private_data   = handle;

	return 0;
}

static int mt_opc_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t mt_opc_read(struct file *filp, char __user *buf, size_t size, loff_t *pos)
{
	opc_handle_t *handle = (opc_handle_t *)filp->private_data;
	ssize_t size_be_read;
	int timeout = 1000; // for example 1K jiffies

	if (wait_event_interruptible_timeout(handle->opcq, handle->opc_status, timeout) <= 1) {
		MT_PRINT("Timeout or Interrupted while get opc status\n");
		return -EAGAIN;
	}

	mutex_lock(&handle->opc_mutex);

	size_be_read = sizeof(handle->opc_status);
	if (copy_to_user(buf, (char *)(handle->opc_status), size_be_read)) {
		MT_PRINT("Get opc status issue, user copy issue.\n");
		size_be_read = -EFAULT;
	}

	mutex_unlock(&handle->opc_mutex);

	return size_be_read;
}


static struct file_operations mt_opc_fops = {
	.owner   = THIS_MODULE,
	.read    = mt_opc_read,
	.open    = mt_opc_open,
	.release = mt_opc_release,
};

static int mt_opc_init(void)
{
	opc_handle_t *handle = (opc_handle_t *)kmalloc(sizeof(opc_handle_t), GFP_KERNEL);
	int ret;


	handle->base = SYMPHONY_DISPLAY_REGISTER_VIRT_BASE;
	hw_opc_onoff_int_mask(handle, OPC_IRQ_OFF);

	memset(handle, 0, sizeof(opc_handle_t));
	opc_netlink_init();

	g_opc_handle = handle;

	handle->interval_ticks = 800; 

	mutex_init(&handle->opc_mutex);

	INIT_WORK(&handle->work, mt_opc_irq_work);
	INIT_DELAYED_WORK(&handle->delay_work, mt_opc_delay_work);
	init_waitqueue_head(&handle->opcq);

	handle->base = SYMPHONY_DISPLAY_REGISTER_VIRT_BASE;
	handle->irq  = IRQ_DISP_OP_REE_ID;
	ret = request_irq(handle->irq, mt_opc_isr, IRQF_TRIGGER_HIGH, "mt_opc", handle);
	if (ret < 0)
		goto fail;

	handle->workqueue = create_workqueue("opc_workqueue");
	if (!handle->workqueue) {
		MT_PRINT("Failed to create workqueue\n");
		goto fail;
	}

	handle->dev.minor = UMAP_MIN_MINOR_OPC;
	handle->dev.owner = THIS_MODULE;
	handle->dev.fops  = &mt_opc_fops;
	handle->dev.priv  = (void *)handle;
	sprintf(handle->dev.devfs_name, UMAP_DEVNAME_OPC);

	ret = mt_drv_dev_register(&handle->dev);
	if (ret < 0) {
		MT_PRINT("register %s failed.\n", handle->dev.devfs_name);
		goto fail;
	}

	hw_opc_onoff_int_mask(handle, OPC_IRQ_ON);

	MT_PRINT("Display Output Policy Init Success\n");
	return 0;

fail:
	free_irq(handle->irq, handle);
	if(handle->workqueue){
		flush_workqueue(handle->workqueue);
		destroy_workqueue(handle->workqueue);
	}
	kfree(handle);
	return ret;
}

static void mt_opc_exit(void)
{
	opc_handle_t *handle = (opc_handle_t *)g_opc_handle;

	flush_workqueue(handle->workqueue);
	destroy_workqueue(handle->workqueue);

	free_irq(handle->irq, handle);
	handle->irq = 0;

	kfree(handle);

	mt_drv_dev_unregister(&handle->dev);

	MT_PRINT("Display Output Policy Exit Success\n");
}

int mt_opc_setup(void)
{
	mt_opc_init();

	return MT_SUCCESS;
}

void mt_opc_cleanup(void)
{
	mt_opc_exit();
}
