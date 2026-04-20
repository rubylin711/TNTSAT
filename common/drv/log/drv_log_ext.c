#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/seq_file.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_osal.h"
#include "mt_drv_dev.h"
#include "drv_log.h"
#include "mt_drv_proc.h"
#include "mt_drv_log.h"
#include "drv_log_ioctl.h"

static mt_s32 cmpi_log_ioctl(struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg)
{
	switch(cmd)
	{
		case UMAP_CMPI_LOG_INIT:
		{
			ulong *phyaddr = (ulong *)arg;
            /*pass the address of config information to user-state to share config information*/
			mt_drv_log_config_bufaddr(phyaddr);
			return MT_SUCCESS;
		}
		case UMAP_CMPI_LOG_EXIT:
		{
			return MT_SUCCESS;
		}
#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
		case UMAP_CMPI_LOG_READ_LOG:
		{
			log_buf_read_s *msg;
			msg =(log_buf_read_s *)arg;
			return mt_drv_log_buffer_read(msg->buf, msg->size, &msg->len, MT_FALSE);
		}
		case UMAP_CMPI_LOG_WRITE_LOG:
		{
			log_buf_write_s *msg;
			msg =(log_buf_write_s *)arg;
			return mt_drv_log_buffer_write(msg->buf, msg->len, MSG_FROM_USER);
		}
#endif
		case UMAP_CMPI_LOG_SET_PATH:
		{
			log_path_s *path = (log_path_s *)arg;
			return mt_drv_log_set_path(path);
		}
		case UMAP_CMPI_LOG_SET_STORE_PATH:
		{
			store_path_s *path = (store_path_s *)arg;
			return mt_drv_log_set_storepath(path);
		}
		default:
			return MT_FAILURE;
	}

	UNUSED(file);
	UNUSED(inode);
}

static mt_length_t log_drv_ioctl(struct file *file, mt_u32 cmd, unsigned long arg)
{
	mt_length_t ret;
	ret = (mt_length_t)mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, cmpi_log_ioctl);
	return ret;
}

static mt_s32 log_drv_open(struct inode * inode, struct file *file)
{
	return 0;
}

static mt_s32 log_drv_release(struct inode * inode, struct file *file)
{
	return 0;
}

static struct file_operations drv_log_fops =
{
	.owner 			= THIS_MODULE,
	.open  			= log_drv_open,
	.unlocked_ioctl = log_drv_ioctl,
	.release		= log_drv_release,
};



#if 0
static mt_device_s mt_log_dev;

mt_s32 mt_drv_log_init(mt_void)
{
	mt_proc_entry_t *item = NULL;
	mt_osal_snprintf(mt_log_dev.devfs_name, sizeof(mt_log_dev.devfs_name), "%s", UMAP_DEVNAME_LOG);
	mt_log_dev.fops = &drv_log_fops;
	mt_log_dev.minor = UMAP_MIN_MINOR_LOG;
	mt_log_dev.owner = THIS_MODULE;
	mt_log_dev.drvops = NULL;
	if(mt_drv_dev_register(&mt_log_dev)<0)
	{
		MT_ERR_LOG("Unable to register dbg dev\n");
		return -1;
	}

	item = mt_drv_proc_add_module(MT_MOD_LOG, NULL, NULL);
    if(!item)
    {
		mt_drv_dev_unregister(&mt_log_dev);
		return -1;
	}

	item->read = mt_drv_log_proc_read;
	item->write = mt_drv_log_proc_write;

	return 0;
}


mt_void mt_drv_log_exit(mt_void)
{
	mt_drv_proc_rm_module(MT_MOD_LOG);
	mt_drv_dev_unregister(&mt_log_dev);
	return;
}

//module_param(log_bufsize, int, S_IRUGO);
#endif





struct mt_drv_log_device {
	struct device *dev;
	void *platdata;
	dev_t devt;
	int id;
};

struct mt_drv_log_driver {
	struct cdev cdev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct mt_drv_log_driver *mt_drv_log_drv;

static int mt_drv_log_probe(struct platform_device *pdev)
{
	mt_proc_entry_t *item = NULL;
	struct mt_drv_log_device *mt_drv_log_dev = NULL;
	int ret = 0;

	mt_drv_log_dev = kzalloc(sizeof(struct mt_drv_log_device), GFP_KERNEL);
	if (!mt_drv_log_dev) {
		pr_err("Error kzalloc mt_drv_log_dev\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mt_drv_log_dev;
	}
	mt_drv_log_dev->id = pdev->id;
	mt_drv_log_dev->devt = mt_drv_log_drv->devt + mt_drv_log_dev->id;
	mt_drv_log_dev->platdata = dev_get_platdata(&pdev->dev);

#if 0
	mt_drv_log_dev->dev = device_create(mt_class, &pdev->dev, mt_drv_log_dev->devt, NULL, UMAP_DEVNAME_LOG);
#else
	mt_drv_log_dev->dev = device_create(mt_class, NULL, mt_drv_log_dev->devt, NULL, UMAP_DEVNAME_LOG);
#endif
	if (IS_ERR(mt_drv_log_dev->dev)) {
		pr_err("Error device_create\n\n");
		ret = PTR_ERR(mt_drv_log_dev->dev);
		goto fail_device_create;
	}

	platform_set_drvdata(pdev, mt_drv_log_dev);
	dev_set_drvdata(mt_drv_log_dev->dev, mt_drv_log_dev);

	item = mt_drv_proc_add_module(MT_MOD_LOG, NULL, NULL);
    if (!item) {
		goto fail_mt_drv_proc_add_module;
	}

	item->read = mt_drv_log_proc_read;
	item->write = mt_drv_log_proc_write;

	return 0;

fail_mt_drv_proc_add_module:
	device_destroy(mt_class, mt_drv_log_dev->devt);
fail_device_create:
	kfree(mt_drv_log_dev);
	mt_drv_log_dev = NULL;
fail_kzalloc_mt_drv_log_dev:
	return ret;
}

static int mt_drv_log_remove(struct platform_device *pdev)
{
	struct mt_drv_log_device *mt_drv_log_dev = platform_get_drvdata(pdev);

	mt_drv_proc_rm_module(MT_MOD_LOG);

	device_destroy(mt_class, mt_drv_log_dev->devt);
	mt_drv_log_dev->dev = NULL;

	platform_set_drvdata(pdev, NULL);

	if (mt_drv_log_dev) {
		kfree(mt_drv_log_dev);
		mt_drv_log_dev = NULL;
	}

	return 0;
}

static int mt_drv_log_suspend(struct platform_device *pdev, pm_message_t stState)
{
	return 0;
}

static int mt_drv_log_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver mt_drv_log_platform_driver = {
	.probe = mt_drv_log_probe,
	.remove = mt_drv_log_remove,
	.suspend = mt_drv_log_suspend,
	.resume = mt_drv_log_resume,
	.driver = {
		.name = UMAP_DEVNAME_LOG,
		.owner = THIS_MODULE,
	}
};

static void mt_drv_log_release_device(struct device *pdev) {  }

static struct platform_device mt_drv_log_platform_device = {
	.name = UMAP_DEVNAME_LOG,
	.id = 0,
	.dev= {
		.platform_data = NULL,
		.release = mt_drv_log_release_device,
	},
};

int __init mt_drv_log_init(void)
{
	int ret;

	mt_drv_log_drv = kzalloc(sizeof(struct mt_drv_log_driver), GFP_KERNEL);
	if (!mt_drv_log_drv) {
		pr_err("Error kzalloc mt_drv_log_drv\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mt_drv_log_drv;
	}

	mt_drv_log_drv->major = MT_DEVICE_MAJOR;
	mt_drv_log_drv->minor = UMAP_MIN_MINOR_LOG;
	mt_drv_log_drv->minors = UMAP_DEV_NUM_LOG;
	mt_drv_log_drv->devt = MKDEV(mt_drv_log_drv->major, mt_drv_log_drv->minor);
	cdev_init(&mt_drv_log_drv->cdev, &drv_log_fops);
	mt_drv_log_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&mt_drv_log_drv->cdev, mt_drv_log_drv->devt, mt_drv_log_drv->minors);
	if (ret) {
		pr_err("Error cdev_add\n\n");
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&mt_drv_log_platform_driver);
	if (ret) {
		pr_err("Error platform_driver_register\n\n");
		goto fail_platform_driver_register;
	}

	ret = platform_device_register(&mt_drv_log_platform_device);
	if (ret) {
		pr_err("Error platform_device_register\n\n");
		goto fail_platform_device_register;
	}

	return 0;

fail_platform_device_register:
	platform_driver_unregister(&mt_drv_log_platform_driver);
fail_platform_driver_register:
	cdev_del(&mt_drv_log_drv->cdev);
fail_cdev_add:
	kfree(mt_drv_log_drv);
	mt_drv_log_drv = NULL;
fail_kzalloc_mt_drv_log_drv:
	return ret;
}

void mt_drv_log_exit(void)
{
	platform_driver_unregister(&mt_drv_log_platform_driver);
	platform_device_unregister(&mt_drv_log_platform_device);
	kfree(mt_drv_log_drv);
	mt_drv_log_drv = NULL;
}

