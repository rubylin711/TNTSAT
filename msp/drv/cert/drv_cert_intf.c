/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#include "mt_drv_dev.h"
#include "mt_drv_module.h"
#include "drv_cert_ioctl.h"
#include "certip.h"

static struct semaphore g_cert_sem;
static mt_device_s g_cert_register_data;

static int cert_open(struct inode *inode, struct file *file)
{
    file->private_data = (void *)0;
    return 0;
}

static mt_s32 cert_ioctl(struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 retval = 0;
    CERT_EXCHANGE_S *p_exchange = NULL;
    CERT_EXPORT_KEY_S *p_export_key = NULL;

    //printk("cert ioctl %x arg %x, \n", cmd, (mt_u32)arg);
    switch (cmd) {
    case CERT_DRV_IOC_LOCK:
	retval = down_interruptible(&g_cert_sem);
	if (retval == 0) {
	    file->private_data = (void *)1;
	}
	break;
    case CERT_DRV_IOC_UNLOCK:
	file->private_data = (void *)0;
	up(&g_cert_sem);
	break;
    case CERT_DRV_IOC_EXCHANGE:
	p_exchange = (CERT_EXCHANGE_S *)arg;
	if (p_exchange->cmds_num == 0) {
	    retval = -1;
	    break;
	}
	retval = certip_exchange(p_exchange->cmds_num, (certip_command_s *)p_exchange->p_cmds,
                                    &p_exchange->processed_num, &p_exchange->cert_status);
	break;
    case CERT_DRV_IOC_EXPORT_KEY:
	p_export_key = (CERT_EXPORT_KEY_S *)arg;
	retval = certip_output_key(p_export_key->slot_id, p_export_key->ext_attr);
	break;
    case CERT_DRV_IOC_KEY_ACK:
	retval = certip_key_ack();
	break;
    case CERT_DRV_IOC_RESET:
	certip_reset();
	break;
    default:
	break;
    }
    return retval;
}

static long cert_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, cert_ioctl);
}

static int cert_close(struct inode *inode, struct file *file)
{
    if (file->private_data) {
	up(&g_cert_sem);
	printk("cert close without unlock\n");
    }
    return 0;
}

struct file_operations cert_fops = {
    .open = cert_open,
    .unlocked_ioctl = cert_drv_ioctl,
    .release = cert_close,
};

//int __init cert_drv_init(void)
int cert_drv_init(void)
{
    sema_init(&g_cert_sem, 1);

    g_cert_register_data.minor = UMAP_MIN_MINOR_CERT;
    g_cert_register_data.owner = THIS_MODULE;
    g_cert_register_data.fops = &cert_fops;

    g_cert_register_data.priv = NULL;
    sprintf(g_cert_register_data.devfs_name, UMAP_DEVNAME_CERT);
    if (mt_drv_dev_register(&g_cert_register_data) < 0) {
	MT_PRINT("register %s failed.\n", g_cert_register_data.devfs_name);
	return MT_FAILURE;
    }

    certip_reset();

    printk("cert init success\n");
    return MT_SUCCESS;
}

//void __exit cert_drv_cleanup(void)
void cert_drv_cleanup(void)
{
    sprintf(g_cert_register_data.devfs_name, UMAP_DEVNAME_CERT);
    mt_drv_dev_unregister(&g_cert_register_data);
}

//module_init(cert_drv_init);
//module_exit(cert_drv_cleanup);
//MODULE_LICENSE("Proprietary");
