/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
  system
  */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
//
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/proc_fs.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/compiler.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/version.h>

#include <asm/io.h>
#include <asm/cache.h>
#include <asm/cacheflush.h>
#include "mt_cache.h"
/*!
 * ko
 */
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "drv_ipcs_ioctl.h"
#include "ipcs_symphony.h"
#include "mt_module_debug.h"


/*!
 ***********************************************************************
 **  description : copy data from or to user space according to _IOC_DIR
 **  return      : int
 ***********************************************************************
 */
static int ipcs_usercopy(struct inode *inode, struct file *file,
        unsigned int cmd, unsigned long arg,
        int (*func)(struct inode *inode, struct file *file,
            unsigned int cmd, void *arg))
{
    char    sbuf[128];
    void    *mbuf = NULL;
    void    *parg = NULL;
    int     err  = -EINVAL;

    /*  Copy arguments into temp kernel buffer  */
    switch (_IOC_DIR(cmd)) {
        case _IOC_NONE:
            /*
             * For this command, the pointer is actually an integer
             * argument.
             */
            parg = (void *) arg;
            break;
        case _IOC_READ: /* some v4l ioctls are marked wrong ... */
        case _IOC_WRITE:
        case (_IOC_WRITE | _IOC_READ):
            if (_IOC_SIZE(cmd) <= sizeof(sbuf)) {
                parg = sbuf;
            } else {
                /* too big to allocate from stack */
                mbuf = kmalloc(_IOC_SIZE(cmd),GFP_DMA);
                if (NULL == mbuf)
                    return -ENOMEM;
                parg = mbuf;
            }

            err = -EFAULT;
            if (copy_from_user(parg, (void __user *)arg, _IOC_SIZE(cmd)))
                goto out;
            break;
    }

    /* call driver */
    if ((err = func(inode, file, cmd, parg)) == -ENOIOCTLCMD)
        err = -EINVAL;

    if (err < 0)
        goto out;

    /*  Copy results into user buffer  */
    switch (_IOC_DIR(cmd))
    {
        case _IOC_READ:
        case (_IOC_WRITE | _IOC_READ):
            if (copy_to_user((void __user *)arg, parg, _IOC_SIZE(cmd)))
            err = -EFAULT;
            break;
    }

out:
    kfree(mbuf);
    return err;
}

/*
 * IO CONCTROL for secure
 */
static  int ipcs_ioctl(struct inode *inode, struct file *file, unsigned int cmd, void *arg)
{
    int ret = MT_FAILURE;

    if (arg == NULL) {
        MT_ERR_CIPHER("Bad cipher parametes\n");
        return ret;
    }

    switch(cmd)
    {
        case CMD_IPCS_SEND_MSG:
            {
                struct ipcs_send_msg_priv *priv = (struct ipcs_send_msg_priv *)arg;
                unsigned int channel_id = priv->channel_id;
                unsigned int wait_ack = priv->ack;
                unsigned int msg_id = priv->msg_id;
                unsigned int param1 = priv->param1;
                unsigned int param2 = priv->param2;

                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};

                if (channel_id >= MAX_CHANNEL_INDEX) {
                    ret = MT_FAILURE;
                }

                msg.msg_id = msg_id;
                msg.param1 = param1;
                msg.param2 = param2;

                ipcs_send_mbx_msg(channel_id, &msg);
                if (wait_ack) {
                    ret = ipcs_recv_mbx_msg(channel_id, &recv_msg);
                    if (msg.msg_id == recv_msg.msg_id && ret == 0) {
                        ret = MT_SUCCESS;
                    } else {
                        ret = MT_FAILURE;
                    }
                } else {
                    //I do not want response
                    ret = MT_SUCCESS;
                }
            }
            break;
        case CMD_IPCS_RECV_MSG:
            {
                struct ipcs_recv_msg_priv *priv = (struct ipcs_recv_msg_priv *)arg;
                unsigned int  channel_id = priv->channel_id;
                unsigned int  send_ack = priv->ack;
                unsigned int  *p_msg_id = priv->p_msg_id;
                unsigned int  *p_param1 = priv->p_param1;
                unsigned int  *p_param2 = priv->p_param2;

                ipc_msg_t msg = {0};

                if (channel_id >= MAX_CHANNEL_INDEX) {
                    return MT_FAILURE;
                }

                if (p_msg_id == NULL || p_param1 == NULL || p_param2 == NULL) {
                    return MT_FAILURE;
                }

                ret = ipcs_recv_mbx_msg((channel_id), &msg);
                if (ret == 0) {
                    *p_msg_id = msg.msg_id;
                    *p_param1 = msg.param1;
                    *p_param2 = msg.param2;
                    if (send_ack) {
                        ipcs_send_mbx_msg(channel_id, &msg);
                        ret = MT_SUCCESS;
                        break;
                    }
                }
            };
            break;
        case CMD_IPCS_CHECK_PEER:
        {
            struct ipcs_peer_status_priv *priv = (struct ipcs_peer_status_priv *)arg;
            priv->peer_status = ipcs_check_seccpu_status();
            ret = MT_SUCCESS;
        }
        break;
        case CMD_IPCS_SET_LOCAL:
        {
            struct ipcs_peer_status_priv *priv = (struct ipcs_peer_status_priv *)arg;
            ipcs_set_local_status(priv->local_status);
            ret = MT_SUCCESS;
        }
        break;
        default:
            break;
    }

    return ret;
}

static  long symphony_ipcs_ioctl( struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct inode *inode;
    inode = filp->f_path.dentry->d_inode;
    return ipcs_usercopy(inode, filp, cmd, arg, ipcs_ioctl);
}

static int ipcs_open(struct inode *inode, struct file *filp)
{
    return MT_SUCCESS;
}

static int ipcs_release(struct inode *inode, struct file *filp)
{
    return MT_SUCCESS;
}

static struct file_operations ipcs_fops = {
    .owner   = THIS_MODULE,
    .open    = ipcs_open,
    .unlocked_ioctl   = symphony_ipcs_ioctl,
    .release = ipcs_release,
};


static mt_device_s  IPCSDev;

int ipcs_setup(void)
{
    strncpy(IPCSDev.devfs_name, UMAP_DEVNAME_IPCS, sizeof(IPCSDev.devfs_name) - 1);
    IPCSDev.fops   = &ipcs_fops;
    IPCSDev.minor  = UMAP_MIN_MINOR_IPCS;
    IPCSDev.owner  = THIS_MODULE;
    IPCSDev.drvops = NULL;

    if (mt_drv_dev_register(&IPCSDev) < 0)
    {
        return MT_FAILURE;
    }

    ipcs_mbx_init();

    return MT_SUCCESS;
}

void ipcs_cleanup(void)
{
    ipcs_mbx_deinit();

    mt_drv_dev_unregister(&IPCSDev);
}

