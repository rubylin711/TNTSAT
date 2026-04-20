#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/types.h>


#include "../../inc/mt_type.h"
#include "../../inc/mt_drv_dev.h"
#include "mt_drv_log.h"
#include "mt_debug.h"


static mt_device_s g_XXX_Dev;
#define XXX_name "demo_test"
#define XXX_name_minor_id 100

mt_s32 _xxx_open(struct inode *inode,struct file *filp)
{
    MT_INFO_LOG("----------------\n");
    return 0;
}

mt_s32 _xxx_close(struct inode *inode,struct file *filp)
{
    MT_INFO_LOG("----------------\n");
    return 0;
}

mt_s32  _xxx_ioctl(struct file *filp,mt_u32 cmd,unsigned long arg)
{
     return 0;
}
static struct file_operations xxx_fops=
{
	.read = NULL,
	.write = NULL,
	.open = _xxx_open,
	.release = _xxx_close,
	.unlocked_ioctl = _xxx_ioctl,
};

static baseops_s xxx_drv_fops=
{
    .probe = NULL,
    .remove = NULL,
    .shutdown = NULL,
    .prepare = NULL,
    .complete = NULL,
    .suspend = NULL,
    .resume = NULL,

};

static int  xxx_drv_init()
{
    snprintf(g_XXX_Dev.devfs_name,sizeof(g_XXX_Dev.devfs_name),XXX_name);
    g_XXX_Dev.minor = XXX_name_minor_id;
    // g_XXX_Dev.owner = THIS_MOUDLE;
    g_XXX_Dev.fops = &xxx_fops;
    g_XXX_Dev.drvops = &xxx_drv_fops;

    if(mt_drv_dev_register(&g_XXX_Dev) <0){

        MT_ERR_LOG("mt_drv_dev_register err!!!");
        return -1;
    }

    return 0;
 }

void xxx_drv_exit()
{
    mt_drv_dev_unregister(&g_XXX_Dev);
}

