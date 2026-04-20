/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pm_runtime.h>

#include "mt_drv_dev.h"
#include "mtdev_base.h"



/*********************************************************************/
/**** mtmedia bus  ****/
/*********************************************************************/


static void mt_bus_release(struct device *dev)
{
	return;
}

struct device mt_bus = {
	.init_name	= "mtmediaBusDev",
	.release    = mt_bus_release
};

static ssize_t modalias_show(struct device *dev, struct device_attribute *a,
							char *buf)
{
	basedev_s *pdev = TO_BASEDEV(dev);
	int len = snprintf(buf, PAGE_SIZE, "mtmedia:%s\n", (char*)pdev->name);

	return (len >= PAGE_SIZE) ? (PAGE_SIZE - 1) : len;
}

static DEVICE_ATTR_RO(modalias);

static struct attribute *mt_dev_attrs[] = {
	&dev_attr_modalias.attr,
	NULL,
};
ATTRIBUTE_GROUPS(mt_dev);

static int mt_match(struct device *dev, struct device_driver *drv)
{
	basedev_s *pdev = TO_BASEDEV(dev);
	return (strncmp(pdev->name, drv->name, MT_DEVICE_NAME_MAX_LEN+8) == 0);
}

static int mt_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
	basedev_s	*pdev = TO_BASEDEV(dev);
	add_uevent_var(env, "MODALIAS=mtmedia:%s", pdev->name);
	return 0;
}

#if 1
//#ifdef CONFIG_SLEEP

static int mt_legacy_suspend(struct device *dev, pm_message_t mesg)
{
	int ret = 0;
	basedev_s *pdev = TO_BASEDEV(dev);
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);

	//printk(KERN_INFO "mt_legacy_suspend: %p\n", dev);

	if (dev->driver && pdrv->suspend)
		ret = pdrv->suspend(pdev, mesg);
	else if (dev->driver && dev->driver->suspend)
		ret = dev->driver->suspend(dev, mesg);

	return ret;
}


static int mt_legacy_suspend_late(struct device *dev, pm_message_t mesg)
{
	int ret = 0;
	basedev_s *pdev = TO_BASEDEV(dev);
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);

	if (dev->driver && pdrv->suspend_late)
		ret = pdrv->suspend_late(pdev, mesg);

	return ret;
}

static int mt_legacy_resume_early(struct device *dev)
{
	int ret = 0;
	basedev_s *pdev = TO_BASEDEV(dev);
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);

	if (dev->driver && pdrv->resume_early)
		ret = pdrv->resume_early(pdev);

	return ret;
}

static int mt_legacy_resume(struct device *dev)
{
	int ret = 0;
	basedev_s *pdev = TO_BASEDEV(dev);
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);

	//printk(KERN_INFO "mt_legacy_resume: %p\n", dev);

	if (dev->driver && pdrv->resume)
		ret = pdrv->resume(pdev);
	else if (dev->driver && dev->driver->resume)
		ret = dev->driver->resume(dev);

	return ret;
}

static int mt_pm_prepare(struct device *dev)
{
	struct device_driver *drv = dev->driver;
	int ret = 0;

	if (drv && drv->pm && drv->pm->prepare)
		ret = drv->pm->prepare(dev);

	return ret;
}

static void mt_pm_complete(struct device *dev)
{
	struct device_driver *drv = dev->driver;

	if (drv && drv->pm && drv->pm->complete)
		drv->pm->complete(dev);
}

//#ifdef CONFIG_SUSPEND
#if 1

static int mt_pm_suspend(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	//printk(KERN_INFO "mt_pm_suspend: %p\n", dev);

	if (!drv)
		return 0;

	if (drv->pm && drv->pm->suspend) {
		ret = drv->pm->suspend(dev);
	} else {
		ret = mt_legacy_suspend(dev, PMSG_SUSPEND);
	}

	return ret;
}

static int mt_pm_suspend_noirq(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->suspend_noirq)
			ret = drv->pm->suspend_noirq(dev);
	} else {
		ret = mt_legacy_suspend_late(dev, PMSG_SUSPEND);
	}

	return ret;
}

static int mt_pm_resume(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

//	printk("%s %s %d \n", __FILE__, __FUNCTION__, __LINE__);

	//printk(KERN_INFO "mt_pm_resume: %p\n", dev);

	if (!drv)
		return 0;

	if (drv->pm && drv->pm->resume) {
		ret = drv->pm->resume(dev);
	} else {
		ret = mt_legacy_resume(dev);
	}

	return ret;
}

static int mt_pm_resume_noirq(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

//	printk("111 %s %s %d \n", __FILE__, __FUNCTION__, __LINE__);


	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->resume_noirq)
			ret = drv->pm->resume_noirq(dev);
	} else {
		ret = mt_legacy_resume_early(dev);
	}

	return ret;
}

#else /* !CONFIG_SUSPEND */

#define mt_pm_suspend		    NULL
#define mt_pm_resume		    NULL
#define mt_pm_suspend_noirq	NULL
#define mt_pm_resume_noirq	    NULL

#endif /* !CONFIG_SUSPEND */

//#ifdef  CONFIG_HIBERNATION
#ifdef CONFIG_HIBERNATE
static int mt_pm_freeze(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->freeze)
			ret = drv->pm->freeze(dev);
	} else {
		ret = mt_legacy_suspend(dev, PMSG_FREEZE);
	}

	return ret;
}

static int mt_pm_freeze_noirq(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->freeze_noirq)
			ret = drv->pm->freeze_noirq(dev);
	} else {
		ret = mt_legacy_suspend_late(dev, PMSG_FREEZE);
	}

	return ret;
}

static int mt_pm_thaw(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->thaw)
			ret = drv->pm->thaw(dev);
	} else {
		ret = mt_legacy_resume(dev);
	}

	return ret;
}

static int mt_pm_thaw_noirq(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->thaw_noirq)
			ret = drv->pm->thaw_noirq(dev);
	} else {
		ret = mt_legacy_resume_early(dev);
	}

	return ret;
}

static int mt_pm_poweroff(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->poweroff)
			ret = drv->pm->poweroff(dev);
	} else {
		ret = mt_legacy_suspend(dev, PMSG_HIBERNATE);
	}

	return ret;
}

static int mt_pm_poweroff_noirq(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->poweroff_noirq)
			ret = drv->pm->poweroff_noirq(dev);
	} else {
		ret = mt_legacy_suspend_late(dev, PMSG_HIBERNATE);
	}

	return ret;
}

static int mt_pm_restore(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->restore)
			ret = drv->pm->restore(dev);
	} else {
		ret = mt_legacy_resume(dev);
	}

	return ret;
}

static int mt_pm_restore_noirq(struct device *dev)
{
	int ret = 0;
	struct device_driver *drv = dev->driver;

	if (!drv)
		return 0;

	if (drv->pm) {
		if (drv->pm->restore_noirq)
			ret = drv->pm->restore_noirq(dev);
	} else {
		ret = mt_legacy_resume_early(dev);
	}

	return ret;
}

#else /* !CONFIG_HIBERNATION */

#define mt_pm_freeze		    NULL
#define mt_pm_thaw		        NULL
#define mt_pm_poweroff		    NULL
#define mt_pm_restore		    NULL
#define mt_pm_freeze_noirq	    NULL
#define mt_pm_thaw_noirq		NULL
#define mt_pm_poweroff_noirq	NULL
#define mt_pm_restore_noirq	NULL

#endif /* !CONFIG_HIBERNATION */

static struct dev_pm_ops mt_dev_pm_ops = {
	.prepare        = mt_pm_prepare,
	.complete       = mt_pm_complete,
	.suspend        = mt_pm_suspend,
	.resume         = mt_pm_resume,
	.freeze         = mt_pm_freeze,
	.thaw           = mt_pm_thaw,
	.poweroff       = mt_pm_poweroff,
	.restore        = mt_pm_restore,
	.suspend_noirq  = mt_pm_suspend_noirq,
	.resume_noirq   = mt_pm_resume_noirq,
	.freeze_noirq   = mt_pm_freeze_noirq,
	.thaw_noirq     = mt_pm_thaw_noirq,
	.poweroff_noirq = mt_pm_poweroff_noirq,
	.restore_noirq  = mt_pm_restore_noirq,
	.runtime_suspend = pm_generic_runtime_suspend,
	.runtime_resume = pm_generic_runtime_resume,
};

#define MT_OPS_PTR	(&mt_dev_pm_ops)

#else /* !CONFIG_SLEEP */

#define MT_OPS_PTR	NULL

#endif /* !CONFIG_SLEEP */


struct bus_type mt_bus_type = {
	.name		= "mtmediaBus",
	.dev_groups	= mt_dev_groups,
	.match		= mt_match,
	.uevent		= mt_uevent,
	.pm			= MT_OPS_PTR,
};
// EXPORT_SYMBOL_GPL(mt_bus_type);


int mt_bus_init(void)
{
	int ret;
	ret = device_register(&mt_bus);
	if (ret)
		return ret;
	ret =  bus_register(&mt_bus_type);
	if (ret)
		device_unregister(&mt_bus);
	return ret;
}

void mt_bus_exit(void)
{
	bus_unregister(&mt_bus_type);
	device_unregister(&mt_bus);
	return;
}


/*********************************************************************/
/***  himedia  base  device  ***/
/*********************************************************************/
int mt_device_add(basedev_s *pdev)
{
	// 0
	if (!pdev)
		return -EINVAL;
	// 1
	if (!pdev->dev.parent)
		pdev->dev.parent = &mt_bus;
	pdev->dev.bus = &mt_bus_type;

	if (pdev->id != -1)
		dev_set_name(&pdev->dev, "%s.%d", pdev->name,  pdev->id);
	else
		dev_set_name(&pdev->dev, "%s", pdev->name);

#if 0
	printk("Registering himedia device '%s'. Parent at %s\n",
		dev_name(&pdev->dev), dev_name(pdev->dev.parent));
#endif
	return device_add(&pdev->dev);
}
//EXPORT_SYMBOL_GPL(mt_device_add);

void mt_device_del(basedev_s *pdev)
{
	if (pdev) {
		device_del(&pdev->dev);
	}
	return;
}
//EXPORT_SYMBOL_GPL(mt_device_del);


void mt_device_put(basedev_s *pdev)
{
	if (pdev)
		put_device(&pdev->dev);
}
//EXPORT_SYMBOL_GPL(mt_device_put);


struct mt_devobj {
	basedev_s pdev;
	char name[1];
};


static void mt_device_release(struct device *dev)
{
	struct mt_devobj *pa = container_of(dev, struct mt_devobj,
								pdev.dev);
	kfree(pa);
	return;
}

static void setup_mtdev_dma_masks(struct tagbasedev_s *pdev)
{
	pdev->dev.dma_parms = &pdev->dma_parms;

	if (!pdev->dev.coherent_dma_mask)
		pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	if (!pdev->dev.dma_mask) {
		pdev->mt_dma_mask = DMA_BIT_MASK(32);
		pdev->dev.dma_mask = &pdev->mt_dma_mask;
	}
};

basedev_s *mt_device_alloc(const char *name, int id, void *priv)
{
	int size ;
	struct mt_devobj *pa;
	size = strlen(name) + 8;
	pa = kzalloc(sizeof(struct mt_devobj) + size, GFP_KERNEL);
	if (pa) {
		snprintf(pa->name, size, "%s-base", name);
		//strcpy(pa->name, name);
		pa->pdev.name = pa->name;
		pa->pdev.id   = id;
		device_initialize(&pa->pdev.dev);
		pa->pdev.dev.release = mt_device_release;
		pa->pdev.dev.platform_data = priv;
		setup_mtdev_dma_masks(&pa->pdev);
	}
	return pa ? &pa->pdev : NULL;
}
//EXPORT_SYMBOL_GPL(mt_device_alloc);


int mt_device_register(basedev_s *pdev)
{
	device_initialize(&pdev->dev);
	return mt_device_add(pdev);
}

void mt_device_unregister(basedev_s *pdev)
{
	mt_device_del(pdev);
	mt_device_put(pdev);
	return;
}





/*********************************************************************/
/***  himedia  base  driver  ***/
/*********************************************************************/

static int mt_drv_probe(struct device *dev)
{
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);
	basedev_s *pdev = TO_BASEDEV(dev);

	return pdrv->probe(pdev);
}

static int mt_drv_remove(struct device *dev)
{
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);
	basedev_s *pdev = TO_BASEDEV(dev);

	return pdrv->remove(pdev);
}

static void mt_drv_shutdown(struct device *dev)
{
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);
	basedev_s *pdev = TO_BASEDEV(dev);

	pdrv->shutdown(pdev);
	return;
}

static int mt_drv_suspend(struct device *dev, pm_message_t state)
{
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);
	basedev_s *pdev = TO_BASEDEV(dev);

	//printk(KERN_INFO "mt_drv_suspend: %p\n",dev);

	return pdrv->suspend(pdev, state);
}

static int mt_drv_resume(struct device *dev)
{
	basedrv_s *pdrv = to_mt_basedrv(dev->driver);
	basedev_s *pdev = TO_BASEDEV(dev);

	//printk(KERN_INFO "mt_drv_resume: %p\n",dev);

	return pdrv->resume(pdev);
}


int mt_driver_register(basedrv_s *drv)
{
	drv->driver.bus = &mt_bus_type;
	if (drv->probe)
		drv->driver.probe    = mt_drv_probe;
	if (drv->remove)
		drv->driver.remove   = mt_drv_remove;
	if (drv->shutdown)
		drv->driver.shutdown = mt_drv_shutdown;
	if (drv->suspend)
		drv->driver.suspend  = mt_drv_suspend;
	if (drv->resume)
		drv->driver.resume   = mt_drv_resume;
	return driver_register(&drv->driver);
}
//EXPORT_SYMBOL_GPL(mt_driver_register0);


void mt_driver_unregister(basedrv_s *drv)
{
	driver_unregister(&drv->driver);
}
//EXPORT_SYMBOL_GPL(mt_driver_unregister0);


struct mt_drvobj {
	basedrv_s pdrv;
	char name[1];
};



basedrv_s *mt_driver_alloc(const char *name, struct module *owner,
								baseops_s *ops)
{
	int size;
	struct mt_drvobj *pa;
	struct dev_pm_ops *pm;

	size = strlen(name) + 8;
	pa = kzalloc(sizeof(struct mt_drvobj) + size, GFP_KERNEL);
	if (pa) {
		// 0
		snprintf(pa->name, size, "%s-base", name);
		//strcpy(pa->name, name);
		pa->pdrv.driver.name  = pa->name;
		pa->pdrv.driver.owner = owner;
		// 1
		if (ops && ops->probe) {
			pa->pdrv.probe = ops->probe;
		}else{
			pa->pdrv.probe = NULL;
		}

		if (ops && ops->remove) {
			pa->pdrv.remove = ops->remove;
		}else{
			pa->pdrv.remove = NULL;
		}

		if (ops && ops->shutdown) {
			pa->pdrv.shutdown = ops->shutdown;
		}else {
			pa->pdrv.shutdown = NULL;
		}

		if (ops && ops->suspend) {
			pa->pdrv.suspend = ops->suspend;
		}else{
			pa->pdrv.suspend = NULL;
		}

		if (ops && ops->resume) {
			pa->pdrv.resume = ops->resume;
		}else{
			pa->pdrv.resume = NULL;
		}

		if (ops && ops->suspend_late){
			pa->pdrv.suspend_late = ops->suspend_late;
		}else{
			pa->pdrv.suspend_late = NULL;
		}

		if (ops && ops->resume_early){
			pa->pdrv.resume_early = ops->resume_early;
		}else{
			pa->pdrv.resume_early = NULL;
		}

		pm = kzalloc(sizeof(struct dev_pm_ops), GFP_KERNEL);
		if (pm) {
			if (ops && ops->runtime_suspend) {
				pm->runtime_suspend = ops->runtime_suspend;
			}
			if (ops && ops->runtime_resume) {
				pm->runtime_resume = ops->runtime_resume;
			}
			pa->pdrv.driver.pm = pm;
		}
	}
	return pa ? &pa->pdrv : NULL;
}

void mt_driver_release(basedrv_s *drv)
{
	struct mt_drvobj *pa = container_of(drv, struct mt_drvobj, pdrv);
	kfree(pa);
	return;
}




