#include "apollo.h"
#include "wsm.h"#include <linux/cdev.h>

#ifdef CONFIG_SUPPORT_MDNS_OFFLOAD
static dev_t mdns_offload_devid;
static struct class *mdns_offload_char_class;
static struct cdev mdns_offload_char_dev;

static int __match_devt(struct device *dev, const void *data)
{
	const dev_t *devt = data;

	return dev->devt == *devt;
}

struct device *device_find_by_devt(dev_t devt)
{
	struct device *dev;

	dev = class_find_device(mdns_offload_char_class, NULL, &devt, __match_devt);

	return dev;
}

static int mdns_offload_chr_open(struct inode *inode, struct file *file_p)
{
	struct device *dev = NULL;
	if (imajor(inode) == MAJOR(mdns_offload_devid)){
		dev = device_find_by_devt(inode->i_rdev);
	}
	if (!dev)
		goto err;

	file_p->private_data = dev_set_drvdata(dev);
	if(WARN_ON(!file_p->private_data))
		goto err;
	return 0;
err:
	return -1;
}

static ssize_t mdns_offload_chr_read(struct file *file_p,
        char __user *buf_p,
        size_t count,
        loff_t *pos_p)
{
	struct atbm_common *hw_priv;
	u32 ret;

	if ((hw_priv = file_p->private_data) == NULL)
		return -ENODEV;

	ret = hw_priv->tmp_count;

	if(copy_to_user(buf_p, &ret, 4)){
	    return -EINVAL;
	}

	return 4;
}


static ssize_t mdns_offload_chr_write(struct file *file_p,
        const char __user *buf_p,
        size_t count,
        loff_t *pos_p)
{
    struct atbm_common *hw_priv;
	u8 buf[256];
    int ret;

    if(count > 256)
		return -1;

	if ((hw_priv = file_p->private_data) == NULL)
		return -ENODEV;

	if(copy_from_user(buf, buf_p, count)){
		ret =  -EINVAL;
		return ret;
	}

	ret = wsm_set_mdns_offload((void*)hw_priv, buf, count, 0);
	if(ret < 0)
		return ret;

	return count;
}

static struct file_operations mdns_offload_chrdev_ops  = {
	open    :    mdns_offload_chr_open,
    read    :    mdns_offload_chr_read,
    write    :    mdns_offload_chr_write,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
static char *atbm_mdns_offload_devnode(struct device *dev, umode_t *mode)
#else
static char *atbm_mdns_offload_devnode(struct device *dev, mode_t *mode)
#endif
{
    if (!mode)
        return NULL;
    *mode = 0666;
    return NULL;
}

int mdns_offload_chr_init(struct atbm_common *hw_priv)
{
    int res = 0;
    struct device *dev;
	char *name = "AtbmVendorDev";

    atbm_printk_init("Register usb char device interface for BT driver");
    mdns_offload_char_class = class_create(THIS_MODULE, name);
    if (IS_ERR(mdns_offload_char_class)) {
        atbm_printk_err("Failed to create bt char class");
        return PTR_ERR(mdns_offload_char_class);
    }

    res = alloc_chrdev_region(&mdns_offload_devid, 0, 1, name);
    if (res < 0) {
        atbm_printk_err("Failed to allocate mdns offload char device");
        goto err_alloc;
    }

	mdns_offload_char_class->devnode = atbm_mdns_offload_devnode;

    dev = device_create(mdns_offload_char_class, NULL, mdns_offload_devid, hw_priv, name);
    if (IS_ERR(dev)) {
        atbm_printk_err("Failed to create mdns offload char device");
        res = PTR_ERR(dev);
        goto err_create;
    }

    cdev_init(&mdns_offload_char_dev, &mdns_offload_chrdev_ops);
    res = cdev_add(&mdns_offload_char_dev, mdns_offload_devid, 1);
    if (res < 0) {
        atbm_printk_err("Failed to add mdns offload char device");
        goto err_add;
    }

    return 0;

err_add:
    device_destroy(mdns_offload_char_class, mdns_offload_devid);
err_create:
    unregister_chrdev_region(mdns_offload_devid, 1);
err_alloc:
    class_destroy(mdns_offload_char_class);
    return res;
}


void mdns_offload_chr_exit(void)
{
    atbm_printk_err("Unregister usb char device interface for mdns offload driver");

    device_destroy(mdns_offload_char_class, mdns_offload_devid);
    cdev_del(&mdns_offload_char_dev);
    unregister_chrdev_region(mdns_offload_devid, 1);
    class_destroy(mdns_offload_char_class);

    return;
}
#endif
