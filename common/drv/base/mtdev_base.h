#ifndef _MTDEV_BASE_H_
#define _MTDEV_BASE_H_
//#include "../inc/mt_drv_dev.h"
typedef struct tagdevice_s  {
	mt_s32 minor;
	const mt_s8 *name;
	struct module *owner;
	const struct file_operations *app_ops;
	baseops_s *base_ops;
	struct list_head list;
	struct device *app_device;	/* for create device node in /dev by devtmpfs or mdev */
	basedev_s *base_device;		/* for most kernel api param, base_device->dev is the parent of app_device */
	basedrv_s *base_driver;
	void *priv;
}device_s;



// bus
int  mt_bus_init(void);
void mt_bus_exit(void);

// device
int  mt_device_register(basedev_s *pdev);
void mt_device_unregister(basedev_s *pdev);
int  mt_device_add(basedev_s *pdev);
void mt_device_del(basedev_s *pdev);
void mt_device_put(basedev_s *pdev);
basedev_s *mt_device_alloc(const char *name, int id, void *priv);

//driver
int  mt_driver_register(basedrv_s *drv);
void mt_driver_unregister(basedrv_s *drv);
void mt_driver_release(basedrv_s *drv);
basedrv_s *mt_driver_alloc(const char *name, struct module *owner,
								baseops_s *ops);


#endif
