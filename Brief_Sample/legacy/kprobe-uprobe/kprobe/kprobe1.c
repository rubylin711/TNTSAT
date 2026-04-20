/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/delay.h>
#include "kprobe.h"
#include "mt_ftrace.h"

#define MT_KPROBE_TEST_NAME "mt_kprobe_test"

struct kprobe_device {
	struct device *dev;
	void *platdata;
};

struct kprobe_driver {
	struct cdev cdev;
	struct class *class;
	struct kprobe_device *kprobe_dev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct kprobe_driver *kprobe_drv;

static int kprobe_open(struct inode *inode, struct file *file)
{
	struct kprobe_device *kprobe_dev;

	kprobe_dev = kprobe_drv->kprobe_dev;
	file->private_data = kprobe_dev;

	return 0;
}

static int kprobe_release(struct inode *inode, struct file *file)
{
	//struct kprobe_device *kprobe_dev = file->private_data;

	return 0;
}

static long kprobe_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	//struct kprobe_device *kprobe_dev = file->private_data;
	unsigned long delay = 3000;

	switch (cmd) {
	case MT_KPROBE_TEST_DELAY:
		printk("%s, %d\n", __FUNCTION__, __LINE__);
		printk("mdelay(%ld)\n", delay);
		mdelay(delay);
#if 0
		mt_ftrace_k_mark2();
		mt_ftrace_k_stop();
#endif
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

static struct file_operations kprobe_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = kprobe_ioctl,
	.open = kprobe_open,
	.release = kprobe_release,
};

static int kprobe_probe(struct platform_device *pdev)
{
	struct kprobe_device *kprobe_dev = NULL;
	int ret = 0;

	kprobe_dev = kzalloc(sizeof(struct kprobe_device), GFP_KERNEL);
	if (!kprobe_dev) {
		ret = -ENOMEM;
		goto fail_kzalloc_kprobe_dev;
	}

	kprobe_drv->kprobe_dev = kprobe_dev;

	kprobe_dev->platdata = dev_get_platdata(&pdev->dev);

	kprobe_dev->dev = device_create(kprobe_drv->class, &pdev->dev, kprobe_drv->devt, NULL, MT_KPROBE_TEST_NAME);
	if (IS_ERR(kprobe_dev->dev)) {
		ret = PTR_ERR(kprobe_dev->dev);
		goto fail_device_create;
	}

	platform_set_drvdata(pdev, kprobe_dev);
	dev_set_drvdata(kprobe_dev->dev, kprobe_dev);

	return 0;

fail_device_create:
	kprobe_drv->kprobe_dev = NULL;
	kfree(kprobe_dev);
	kprobe_dev = NULL;
fail_kzalloc_kprobe_dev:
	return ret;
}

static int kprobe_remove(struct platform_device *pdev)
{
	struct kprobe_device *kprobe_dev = platform_get_drvdata(pdev);

	printk(KERN_EMERG "%s\n", __FUNCTION__);


	platform_set_drvdata(pdev, NULL);

	device_destroy(kprobe_drv->class, kprobe_drv->devt);

	if (kprobe_dev) {
		kprobe_dev->platdata = NULL;
		kprobe_dev->dev = NULL;
		kfree(kprobe_dev);
		kprobe_dev = NULL;
	}

	return 0;
}

static int kprobe_suspend(struct platform_device *pdev, pm_message_t stState)
{
	//struct kprobe_device *kprobe_dev = platform_get_drvdata(pdev);

	return 0;
}

static int kprobe_resume(struct platform_device *pdev)
{
	//struct kprobe_device *kprobe_dev = platform_get_drvdata(pdev);

	return 0;
}

static struct platform_driver kprobe_platform_driver = {
	.probe = kprobe_probe,
	.remove = kprobe_remove,
	.suspend = kprobe_suspend,
	.resume = kprobe_resume,
	.driver = {
		.name = MT_KPROBE_TEST_NAME,
		.owner = THIS_MODULE,
	}
};

static void kprobe_release_device(struct device *pdev)
{
	printk(KERN_EMERG "%s\n", __FUNCTION__);
}

static struct platform_device kprobe_platform_device = {
	.name = MT_KPROBE_TEST_NAME,
	.id = -1,
	.dev= {
		.release = kprobe_release_device,
	},
};

static int __init kprobe_module_init(void)
{
	int ret;

	printk(KERN_EMERG "%s\n", __FUNCTION__);

	kprobe_drv = kzalloc(sizeof(struct kprobe_driver), GFP_KERNEL);
	if (!kprobe_drv) {
		ret = -ENOMEM;
		goto fail_kzalloc_kprobe_drv;
	}

	kprobe_drv->class = class_create(MT_KPROBE_TEST_NAME);
	if (IS_ERR(kprobe_drv->class)) {
		ret = PTR_ERR(kprobe_drv->class);
		goto fail_class_create;
	}

	kprobe_drv->minor = 0;
	kprobe_drv->minors = 1;
	ret = alloc_chrdev_region(&kprobe_drv->devt, kprobe_drv->minor, kprobe_drv->minors, MT_KPROBE_TEST_NAME);
	if (ret) {
		ret = -EINVAL;
		goto fail_alloc_chrdev_region;
	}
	kprobe_drv->major = MAJOR(kprobe_drv->devt);

	cdev_init(&kprobe_drv->cdev, &kprobe_fops);
	kprobe_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&kprobe_drv->cdev, kprobe_drv->devt, kprobe_drv->minors);
	if (ret) {
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&kprobe_platform_driver);
	if (ret)
		goto fail_platform_driver_register;

	ret = platform_device_register(&kprobe_platform_device);
	if (ret)
		goto fail_platform_device_register;

	return 0;

fail_platform_device_register:
	platform_driver_unregister(&kprobe_platform_driver);
fail_platform_driver_register:
	cdev_del(&kprobe_drv->cdev);
fail_cdev_add:
	unregister_chrdev_region(kprobe_drv->devt, kprobe_drv->minors);
fail_alloc_chrdev_region:
	class_destroy(kprobe_drv->class);
	kprobe_drv->class = NULL;
fail_class_create:
	kfree(kprobe_drv);
	kprobe_drv = NULL;
fail_kzalloc_kprobe_drv:
	return ret;
}


static void __exit kprobe_module_exit(void)
{
	printk(KERN_EMERG "%s\n", __FUNCTION__);

	platform_device_unregister(&kprobe_platform_device);
	platform_driver_unregister(&kprobe_platform_driver);

	cdev_del(&kprobe_drv->cdev);
	unregister_chrdev_region(kprobe_drv->devt, kprobe_drv->minors);

	class_destroy(kprobe_drv->class);
	kprobe_drv->class = NULL;

	kfree(kprobe_drv);
	kprobe_drv = NULL;
}

module_init(kprobe_module_init);
module_exit(kprobe_module_exit);

MODULE_AUTHOR("MONTAGE");
MODULE_LICENSE("GPL");

