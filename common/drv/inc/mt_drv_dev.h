#ifndef __MT_DRV_DEV_1_H__
#define __MT_DRV_DEV_1_H__

#include <linux/module.h>
#include <linux/device.h>
#include <linux/major.h>
#include <asm/types.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_drv_struct.h"

#ifdef __cplusplus
extern "C"
{
#endif /* End of #ifdef __cplusplus */

/** @addtogroup H_DEV */
/** @{ */
#define MT_DEVICE_NAME_MAX_LEN 64

#define MT_DEVICE_MAJOR 218
#define MT_DYNAMIC_MINOR 255
//#define UMAP_DEV_NUM_TOTAL 100

extern struct class *mt_class;

typedef struct tagbasedev_s{
	mt_s32		id;
	const mt_s8	* name;
	struct device	dev;
	struct device_dma_parameters dma_parms;
	u64		mt_dma_mask;
}basedev_s;

#define TO_BASEDEV(x) container_of((x), basedev_s, dev)

typedef struct tagbaseops_s {
	mt_s32  (*probe)(basedev_s *);
	mt_s32  (*remove)(basedev_s *);
	mt_void (*shutdown)(basedev_s *);
	mt_s32  (*prepare)(basedev_s *);
	mt_void (*complete)(basedev_s *);
	mt_s32  (*suspend)(basedev_s *, pm_message_t state);
	mt_s32  (*suspend_late)(basedev_s *, pm_message_t state);
	mt_s32  (*resume_early)(basedev_s *);
	mt_s32  (*resume)(basedev_s *);
	int (*runtime_suspend)(struct device *dev);
	int (*runtime_resume)(struct device *dev);
}baseops_s;


typedef struct tagbasedrv_s {
	mt_s32  (*probe) (basedev_s *);
	mt_s32  (*remove)(basedev_s *);
	mt_void (*shutdown)(basedev_s *);
	mt_s32  (*suspend)(basedev_s *, pm_message_t state);
	mt_s32  (*suspend_late)(basedev_s *, pm_message_t state);
	mt_s32  (*resume_early)(basedev_s *);
	mt_s32  (*resume)(basedev_s *);
	struct device_driver driver;
}basedrv_s;

#define to_mt_basedrv(drv)	container_of((drv), basedrv_s, driver)

typedef struct tagPM_DEVICE_S  {
	mt_s32 minor;
	const mt_s8 *name;
	struct module *owner;
	const struct file_operations *app_ops;
	basedrv_s *base_ops;
	struct list_head list;
	struct device *app_device;
	basedrv_s *base_device;
	basedrv_s *base_driver;
}PM_DEVICE_S;


typedef struct _mt_device_s
{
	mt_char devfs_name[MT_DEVICE_NAME_MAX_LEN];     /* devfs */
	mt_s32  minor;
	struct module *owner;
	struct file_operations *fops;
	baseops_s *drvops;
	struct device *dev;
	void *priv;
}mt_device_s, *p_mt_device_s;

mt_s32  mt_drv_dev_register(mt_device_s *umapd);
mt_void mt_drv_dev_unregister(mt_device_s *umapd);
mt_s32 mt_drv_dev_init(mt_void);
mt_void mt_drv_dev_exit(mt_void);
mt_void *get_mt_priv(mt_s32 idx);

//mt_s32 mt_drv_register(PM_DEVICE_S *);
//mt_s32 mt_drv_unregister(PM_DEVICE_S *);


extern mt_s32 mt_drv_usercopy(struct inode *inode, struct file *file,
           mt_u32 cmd, unsigned long arg,
           mt_s32 (*func)(struct inode *inode, struct file *file,
               mt_u32 cmd, mt_void *arg));

/** @} */

#define MT_FATAL_DEV(fmt...) \
            MT_TRACE(MT_LOG_LEVEL_FATAL, MT_ID_LOG, fmt)
#define MT_ERR_DEV(fmt...) \
            MT_TRACE(MT_LOG_LEVEL_ERROR, MT_ID_LOG, fmt)
#define MT_WARN_DEV(fmt...) \
            MT_TRACE(MT_LOG_LEVEL_WARNING, MT_ID_LOG, fmt)
#define MT_INFO_DEV(fmt...) \
            MT_TRACE(MT_LOG_LEVEL_INFO, MT_ID_LOG, fmt)

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */


#endif /* __MT_DRV_DEV_H__ */

