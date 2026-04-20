/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/dma-direct.h>
#include <linux/cdev.h>
#include "mt_drv_dev.h"
#include "mt_drv_mem.h"
#include "mtdev_base.h"
#include "mt_drv_log.h"
#include "mt_debug.h"


#include <linux/delay.h>

/*
 * Head entry for the doubly linked mt_dev_device list
 */
static LIST_HEAD(mt_dev_list);
static DEFINE_MUTEX(mt_dev_sem);

/*
 * Assigned numbers, used for dynamic minors
 */
#define DYNAMIC_MINORS 128 /* like dynamic majors */
static unsigned char mt_minors[DYNAMIC_MINORS / 8] = {0};
static void *mt_priv[DYNAMIC_MINORS] = {0};

extern void *mt_global_bus_dma_region_p;

extern int pmu_device_init(void);

#ifdef CONFIG_PROC_FS
static void *mt_dev_seq_start(struct seq_file *seq, loff_t *pos)
{
	mutex_lock(&mt_dev_sem);
	return seq_list_start(&mt_dev_list, *pos);
}

static void *mt_dev_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	return seq_list_next(v, &mt_dev_list, pos);
}

static void mt_dev_seq_stop(struct seq_file *seq, void *v)
{
	mutex_unlock(&mt_dev_sem);
}

static int mt_dev_seq_show(struct seq_file *seq, void *v)
{
//	const device_s *p = list_entry(v, device_s, list);
	//PROC_PRINT(seq, "%3i %s\n", p->minor, (char*)p->name ? (char*)p->name : "");
	return 0;
}


static struct seq_operations mt_dev_seq_ops = {
	.start = mt_dev_seq_start,
	.next  = mt_dev_seq_next,
	.stop  = mt_dev_seq_stop,
	.show  = mt_dev_seq_show,
};

static int mt_dev_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &mt_dev_seq_ops);
}

static struct proc_ops mt_dev_proc_fops = {
	.proc_open    = mt_dev_seq_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = seq_release,
};
#endif

static int mt_dev_open(struct inode * inode, struct file * file)
{
	int minor = iminor(inode);
	device_s *c;
	int err = -ENODEV;
	const struct file_operations *old_fops, *new_fops = NULL;

	mutex_lock(&mt_dev_sem);

	list_for_each_entry(c, &mt_dev_list, list) {
		if (c->minor == minor) {
			new_fops = fops_get(c->app_ops);
			break;
		}
	}

	if (!new_fops) {
#if 0
		mutex_unlock(&mt_dev_sem);
		request_module("char-major-%d-%d", MT_DEVICE_MAJOR, minor);
		mutex_lock(&mt_dev_sem);

		list_for_each_entry(c, &mt_dev_list, list) {
			if (c->minor == minor) {
				new_fops = fops_get(c->app_ops);
				break;
			}
		}
		if (!new_fops)
			goto fail;
#else
		goto fail;
#endif
	}

	err = 0;
	old_fops = file->f_op;
	file->f_op = new_fops;
	if (file->f_op->open) {
		err=file->f_op->open(inode,file);
		if (err) {
			fops_put(file->f_op);
			file->f_op = fops_get(old_fops);
		}
	}
	fops_put(old_fops);
fail:
	mutex_unlock(&mt_dev_sem);
	return err;
}

struct class *mt_class;

static struct file_operations mt_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= mt_dev_open,
};



/**
 *	mt_drv_register	-	register a mtdev device
 *	@mtdev: device structure
 *
 *	Register a mtdev device with the kernel. If the minor
 *	number is set to %MT_DYNAMIC_MINOR a minor number is assigned
 *	and placed in the minor field of the structure. For other cases
 *	the minor number requested is used.
 *
 *	The structure passed is linked into the kernel and may not be
 *	destroyed until it has been unregistered.
 *
 *	A zero is returned on success and a negative errno code for
 *	failure.
 */


mt_s32 mt_drv_register(device_s * mtdev)
{

	int ret;
	dev_t dev;
	device_s  *c = NULL;
	struct device *adev = NULL;
	basedev_s *bdev = NULL;
	basedrv_s *bdrv = NULL;
	u64 dma_start = 0;
	u64 dma_end = 0;

    //MT_INFO_LOG("--------------mt_drv_register  name  %s    mtdev->minor  %x \n",mtdev->name,mtdev->minor);

    if ( (mtdev == NULL) || (mtdev->name == NULL) || (mtdev->app_ops == NULL) )
    {
		//MT_INFO_LOG("%s args invalid \n", __FUNCTION__);
		return -EINVAL;
	}

	// 1
	mutex_lock(&mt_dev_sem);
	list_for_each_entry(c, &mt_dev_list, list) {
		if (c->minor == mtdev->minor) {
			ret = -EBUSY;
			goto out;
		}
	}
	if (mtdev->minor == MT_DYNAMIC_MINOR) {
		int i = DYNAMIC_MINORS;
		while (--i >= 0)
			if ( (mt_minors[i>>3] & (1 << (i&7))) == 0)
				break;
		if (i < 0) {
			ret = -EBUSY;
			goto out;
		}
		mtdev->minor = i;
	}

	if (mtdev->minor < DYNAMIC_MINORS) {
		mt_minors[mtdev->minor >> 3] |= 1 << (mtdev->minor & 7);
		mt_priv[mtdev->minor] = mtdev->priv;
	}

	// 2 base device, then class = NULL;
	bdev = mt_device_alloc(mtdev->name, -1, mtdev->priv);
	if (!bdev) {
		ret = -ENOMEM;
		goto err0;
	}
	// default dma_range_map and bus_dma_limit
	bdev->dev.dma_range_map = mt_global_bus_dma_region_p;
	{
		const struct bus_dma_region *r = bdev->dev.dma_range_map;
		if (r) {
			for (dma_start = ~0; r->size; r++) {
				/* Take lower and upper limits */
				if (r->dma_start < dma_start) {
					dma_start = r->dma_start;
				}
				if (r->dma_start + r->size > dma_end) {
					dma_end = r->dma_start + r->size;
				}
			}
		}
		bdev->dev.bus_dma_limit = dma_end - 1;
	}
	ret = mt_device_add(bdev);
	if (ret){
		mt_device_put(bdev);
		goto err0;
	}

	// 3 app class
	dev = MKDEV(MT_DEVICE_MAJOR, mtdev->minor);
	adev = device_create(mt_class, &(bdev->dev), dev, NULL,
						 "%s",  mtdev->name);
	if (IS_ERR(adev)) {
		ret = PTR_ERR(adev);
		goto err1;
	}

	// 4 base driver
	bdrv = mt_driver_alloc(mtdev->name, mtdev->owner, mtdev->base_ops);
	if (!bdrv) {
		ret = -ENOMEM;
		goto err2;
	}

	ret = mt_driver_register(bdrv);
	if(ret){
		mt_driver_release(bdrv);
		goto err2;
	}

	mtdev->app_device  = adev;
	mtdev->base_device = bdev;
	mtdev->base_driver = bdrv;

	/*
	 * Add it to the front, so that later devices can "override"
	 * earlier defaults
	 */
	INIT_LIST_HEAD(&mtdev->list);
	list_add(&mtdev->list, &mt_dev_list);
	goto out;


err2:
	device_destroy(mt_class, dev);
err1:
	mt_device_unregister(bdev);
err0:
	mt_minors[mtdev->minor >> 3] &= ~(1 << (mtdev->minor & 7));
	mt_priv[mtdev->minor] = NULL;
out:
	mutex_unlock(&mt_dev_sem);
	return ret;
}

/**
 *	mt_drv_unregister - unregister a mtdev device
 *	@mtdev: device to unregister
 *
 *	Unregister a mtdev device that was previously
 *	successfully registered with mt_drv_register(). Success
 *	is indicated by a zero return, a negative errno code
 *	indicates an error.
 */

mt_s32 mt_drv_unregister(device_s * mtdev)
{
	int i = 0;

    if (mt_class == NULL || mtdev == NULL || (mtdev->name == NULL)||(mtdev->owner == NULL) || (mtdev->app_ops == NULL))
    {
        return -EINVAL;
    }

    i = mtdev->minor;
	// 0
	if (list_empty(&mtdev->list))
		return -EINVAL;
	if ( (i >= DYNAMIC_MINORS) || (i < 0)) {
		//MT_INFO_LOG("%s mtdev->minor: %d  invalid \n", __FUNCTION__,  mtdev->minor);
		return -EINVAL;
	}
	// 1
	mutex_lock(&mt_dev_sem);
	// 1.0
	list_del(&mtdev->list);
	// 1.1
	if(mtdev->base_driver){
		mt_driver_unregister(mtdev->base_driver);
		mt_driver_release(mtdev->base_driver);
		mtdev->base_driver = NULL;
	}
	// 1.2
	if(mtdev->app_device){
		device_destroy(mt_class, MKDEV(MT_DEVICE_MAJOR, mtdev->minor));
		mtdev->app_device = NULL;
	}
	// 1.3
	if(mtdev->base_device){
		mt_device_unregister(mtdev->base_device);
		mtdev->base_device = NULL;
	}
	// 1.4
	mt_minors[i>>3] &= ~(1 << (i & 7));
	mutex_unlock(&mt_dev_sem);
	return 0;
}


mt_s32 drv_mod_init(mt_void)
{
	static int is_inited = 0;
	int ret;

	if(is_inited == 1)
	{
	  MT_INFO_LOG("--------#########---drv _mod inited  ---############----------\n");
	  return 0;
	}
	is_inited = 1;
	MT_INFO_LOG("-----------mt-bus-create-------------\n");
	// 0
#if !(0 == MT_PROC_SUPPORT)
//"mtd" is keywords in Android
//	proc_create("mtdev", 0, NULL, &mt_dev_proc_fops);
	proc_create("mt_dev", 0, NULL, &mt_dev_proc_fops);
#endif
	// 1
	ret = mt_bus_init();
	if(ret)
		goto err0;
	// 2

    MT_INFO_LOG("-----------mt-class-create-------------\n");
//"mtd" is keywords in Android
//	mt_class = class_create("mtdevClass");
	mt_class = class_create("mt_devClass");
	ret = PTR_ERR(mt_class);
	if (IS_ERR(mt_class))
		goto err1;
	// 3
	ret = -EIO;

    MT_INFO_LOG("-----------mt-char-create-------------\n");
//"mtd" is keywords in Android
//	if (register_chrdev(MT_DEVICE_MAJOR, "mtdevCharDev", &mt_dev_fops))
	if (register_chrdev(MT_DEVICE_MAJOR, "mt_devCharDev", &mt_dev_fops))
		goto err2;

	return 0;

err2:

	MT_INFO_LOG("!!! Module mtdev: unable to get major %d for mtdev devices\n", MT_DEVICE_MAJOR);

	class_destroy(mt_class);
err1:
	mt_bus_exit();
err0:
#if !(0 == MT_PROC_SUPPORT)
//"mtd" is keywords in Android
//	remove_proc_entry("mtdev", NULL);
	remove_proc_entry("mt_dev", NULL);
#endif
	return ret;
}

mt_void drv_mod_exit(mt_void)
{
	// 0
	if (list_empty(&mt_dev_list) == 0)
	{
		//MT_INFO_LOG("!!! Module mtdev: there module in list\n");
		return;
	}
	// 1
//"mtd" is keywords in Android
//	unregister_chrdev(MT_DEVICE_MAJOR, "mtdev");
	unregister_chrdev(MT_DEVICE_MAJOR, "mt_dev");
	// 2
	class_destroy(mt_class);
	// 3
	mt_bus_exit();
	// 4
#if !(0 == MT_PROC_SUPPORT)
//"mtd" is keywords in Android
//	remove_proc_entry("mtdev", NULL);
	remove_proc_entry("mt_dev", NULL);
#endif

	return;
}



mt_s32 mt_drv_usercopy(struct inode *inode, struct file *file,
           mt_u32 cmd, unsigned long arg,
           mt_s32 (*func)(struct inode *inode, struct file *file,
               mt_u32 cmd, mt_void *arg))
{
    //mt_char  sbuf[128];
	struct sbuf_usercopy {		/* use struct to keep 8 bytes align for compat 64bit cpu */
		mt_u64 sbuf_real[16];
	} sbuf;
    mt_void  *mbuf = NULL;
    mt_void  *parg = NULL;
    mt_s32   err  = -EINVAL;

    /*  copy arguments into temp kernel buffer  */
    switch (_IOC_DIR(cmd))
    {
        case _IOC_NONE:
            parg = NULL;
            break;
        case _IOC_READ:
        case _IOC_WRITE:
        case (_IOC_WRITE | _IOC_READ):
            if (_IOC_SIZE(cmd) <= sizeof(sbuf))
            {
                memset(&sbuf, 0 , sizeof(sbuf));
                parg = (void *)&sbuf;
            }
            else
            {
                mt_u32 buff_size = _IOC_SIZE(cmd);
                /* too big to allocate from stack */
                mbuf = kzalloc(buff_size, GFP_KERNEL);
                if (NULL == mbuf)
                {
                    //mt_fatal_dev("malloc cmd buffer failed\n");
                    return -ENOMEM;

                }
                parg = mbuf;
            }

            err = -EFAULT;
            if (_IOC_DIR(cmd) & _IOC_WRITE)
            {
                if (copy_from_user(parg, (void __user *)arg, _IOC_SIZE(cmd)))
                {
                   /* mt_fatal_dev("copy_from_user failed, when use ioctl, \
                            the para must be a address, cmd=0x%x\n", cmd); */
                    goto out;
                }
            }
            break;
    }

    /* call driver */
    err = func(inode, file, cmd, (parg));
    if (err == -ENOIOCTLCMD)
        err = -EINVAL;
    if (err < 0)
        goto out;

    /*  copy results into user buffer  */
    switch (_IOC_DIR(cmd))
    {
        case _IOC_READ:
        case (_IOC_WRITE | _IOC_READ):
            if (copy_to_user((void __user *)arg, parg, _IOC_SIZE(cmd)))
            {
              /*  mt_fatal_dev("copy_to_user failed, when use ioctl, \
                        the para must be a address, cmd=0x%x\n", cmd); */
                err = -EFAULT;
            }
        break;
    }

out:
    if (mbuf)
        MT_KFREE(MT_ID_MEM, mbuf);
    return err;
}

EXPORT_SYMBOL(mt_drv_usercopy);


static device_s s_umap_devs[UMAP_DEV_NUM_TOTAL];

/**
 *    MT_DRV_DEV_Register - register umap devices
 *    @umapd:  device structure we want to register
 */
mt_s32 mt_drv_dev_register(mt_device_s *umapd)
{
    mt_u32 i;
    mt_s32 ret;

    for (i = 0; i < UMAP_DEV_NUM_TOTAL; i++)
        if (0 == s_umap_devs[i].minor)
            break;

    if (i == UMAP_DEV_NUM_TOTAL)
    {
     MT_ERR_LOG("too many devices!\n");
     return -1;
    }

    s_umap_devs[i].minor = umapd->minor;
    s_umap_devs[i].name  = umapd->devfs_name;
    s_umap_devs[i].owner  = umapd->owner;
    s_umap_devs[i].app_ops  = umapd->fops;
    s_umap_devs[i].base_ops  = umapd->drvops;
	s_umap_devs[i].priv  = umapd->priv;

    //MT_INFO_CMPI("try register dev:'%s', minor=%d.\n", umapd->devfs_name, umapd->minor);
   // MT_INFO_LOG("--------------mt_drv_dev_register\n");

    ret = mt_drv_register(&s_umap_devs[i]);
    //MT_INFO_LOG("--------------mt_drv_register done,  ret: %d\n", ret);

    umapd->dev = &(s_umap_devs[i].base_device->dev);

    if (MT_SUCCESS != ret)
    {
       // MT_FATAL_DEV("failed register dev:'%s', minor=%d, ret=%d.\n", umapd->devfs_name, umapd->minor, ret);
    }

    return ret;
}

EXPORT_SYMBOL(mt_drv_dev_register);


/**
 *    mt_drv_dev_unregister - unregister a  device
 *    @umapd: the device to unregister
 *
 *    This unregisters the passed device and deassigns the minor
 *    number. Future open calls will be met with errors.
 */
mt_void mt_drv_dev_unregister(mt_device_s *umapd)
{
    mt_s32 i;

	if (0 == umapd->minor)
	{
//		MT_WARN_DEV("try unregister dev:'%s', but minor=%d is invalid.\n", umapd->devfs_name, umapd->minor);
		return ;
	}

    for (i = 0; i < UMAP_DEV_NUM_TOTAL; i++)
        if ( umapd->minor == s_umap_devs[i].minor)
            break;
    if (i == UMAP_DEV_NUM_TOTAL)
        return ;

    //MT_ERR_DEV("try unregister dev:'%s', minor=%d.\n", umapd->devfs_name, umapd->minor);
    MT_INFO_LOG("--------------mt_drv_dev_unregister\n");

    mt_drv_unregister(&s_umap_devs[i]);
    MT_INFO_LOG("--------------mt_drv_dev_unregister  done\n");

    s_umap_devs[i].minor = 0;
    umapd->dev = NULL;
    return ;
}

EXPORT_SYMBOL(mt_drv_dev_unregister);

mt_s32 mt_drv_dev_init(mt_void)
{
    mt_u32 i;

    for (i = 0; i < UMAP_DEV_NUM_TOTAL; i++)
    {
        s_umap_devs[i].minor = 0;
    }
    return MT_SUCCESS;
}

mt_void mt_drv_dev_exit(mt_void)
{
    return ;
}

 void __init mt_drv_env_init(mt_void)
{
	MT_INFO_LOG("====  drv env init=====1==\n");

	mt_drv_dev_init();
	drv_mod_init();
	return;
}

mt_void *get_mt_priv(mt_s32 idx)
{
	return mt_priv[idx];
}
EXPORT_SYMBOL(get_mt_priv);

MODULE_LICENSE("GPL");
