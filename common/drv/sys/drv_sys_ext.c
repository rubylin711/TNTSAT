#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
//#include <mach/hardware.h>
#include "mt_type.h"
#include "mt_osal.h"
#include "mt_reg_common.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_reg.h"
#include "drv_sys_ioctl.h"
#include "mt_drv_analog.h"


static mt_char s_szSdkKoVersion[] __attribute__((used)) = "SDK_VERSION:["\
    MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
    __DATE__", "__TIME__"]";

typedef struct himtxxxx_soc_s
{
    struct semaphore stSem;
    mt_sys_conf_s stChipConf;
}mtxxxx_soc_s;

static mtxxxx_soc_s s_stSocData;

extern const char * get_sdkversion(void);
#ifdef CONFIG_MT_ADEC
extern mt_s32 ADEC_DRV_SYSDATE_SET(mt_sysdate_t *date);
extern mt_s32 ADEC_DRV_SYSDATE_GET(mt_sysdate_t *date);
#endif

mt_s32 SYS_GetBootVersion(mt_char *pVersion,mt_u32 u32VersionLen)
{
#if 0
    const mt_u8* pu8BootVer = get_sdkversion();

    if (MT_NULL == pVersion || u32VersionLen == 0)
    {
        MT_WARN_SYS("SYS_GetBootVersion failure line:%d\n", __LINE__);
        return MT_FAILURE;
    }

    if (pu8BootVer != NULL)
    {
        if (u32VersionLen > strlen(pu8BootVer))
        {
            u32VersionLen = strlen(pu8BootVer);
        }

        memcpy(pVersion, pu8BootVer, u32VersionLen);
        pVersion[u32VersionLen] = '\0';

        return MT_SUCCESS;
    }

    return MT_FAILURE;
	#else
	MT_WARN_SYS("SYS_GetBootVersion failure line:%d\n", __LINE__);
        return MT_FAILURE;
	#endif
}

mt_s32 SysSetConfig(mt_sys_conf_s *pstConf)
{
    memcpy(&s_stSocData.stChipConf, pstConf, sizeof(*pstConf));
    return 0;
}

mt_s32 SysGetConfig(mt_sys_conf_s *pstConf)
{
    memcpy(pstConf, &s_stSocData.stChipConf, sizeof(*pstConf));
    return 0;
}


mt_s32 mt_drv_sys_getdate(mt_sysdate_t *date)
{
  mt_s32 Ret=MT_FAILURE;
#ifndef CONFIG_MT_CHIP_SYMPHONY6
#ifdef CONFIG_MT_ADEC
  Ret=ADEC_DRV_SYSDATE_GET(date);
#endif
#endif
  return Ret;
}
mt_s32 mt_drv_sys_setdate(mt_sysdate_t *date)
{
	mt_s32 Ret=MT_FAILURE;
#ifndef CONFIG_MT_CHIP_SYMPHONY6
#ifdef CONFIG_MT_ADEC
	Ret=ADEC_DRV_SYSDATE_SET(date);
#endif
#endif
    return Ret;
}

extern void (*p_mt_analog_get_temperature_async)(int *temp_int, int *temp_dec);

static int get_temperature(void)
{
	int temp_int = 0;
	int temp_dec = 0;

	if (p_mt_analog_get_temperature_async)
		p_mt_analog_get_temperature_async(&temp_int, &temp_dec);
	else
		return 0;

	return (temp_int * 1000 + temp_dec * 10);
}

static mt_s32 SYS_Ioctl(struct inode *pInode, struct file *pFile, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = -ENOIOCTLCMD;
    mt_sys_version_s* chiptype;

    down(&s_stSocData.stSem);
    switch (cmd)
    {
        case SYS_SET_CONFIG_CTRL :
            ret = SysSetConfig((mt_sys_conf_s*)arg);
            break;

        case SYS_GET_CONFIG_CTRL :
            ret = SysGetConfig((mt_sys_conf_s*)arg);
            break;

        case SYS_GET_SYS_VERSION :
            chiptype = (mt_sys_version_s*)arg;
            ret = mt_drv_sys_getchipversion(&chiptype->enChipTypeHardWare, &chiptype->enChipVersion);
            SYS_GetBootVersion(chiptype->BootVersion, sizeof(chiptype->BootVersion));
            break;

        case SYS_GET_TIMESTAMPMS :
            ret = mt_drv_sys_gettimestampms((mt_u32*)arg);
            break;

        case SYS_GET_DOLBYSUPPORT:
            ret = mt_drv_sys_getdolbysupport((mt_u32*)arg);
            break;
        case SYS_GET_DTSSUPPORT:
            ret = mt_drv_sys_getdtssupport((mt_u32*)arg);
            break;
        case SYS_GET_ADVCASUPPORT:
            ret = mt_drv_sys_getadvcasupport((mt_u32*)arg);
            break;
        case SYS_GET_MACROVISIONSUPPORT:
            ret = mt_drv_sys_getrovisupport((mt_u32*)arg);
            break;
        case SYS_GET_DDRCONFIG:
            ret = mt_drv_sys_getmemconfig((mt_sys_mem_config_s*)arg);
            break;
        case SYS_SET_AVDATE:
			ret=mt_drv_sys_setdate((mt_sysdate_t*)arg);
			break;
		case SYS_GET_AVDATE:
			ret=mt_drv_sys_getdate((mt_sysdate_t*)arg);
			break;
		case SYS_GET_TEMP:		/* get temperature */
			*(int *)arg = get_temperature();
			ret = 0;
			break;
        default :
            MT_WARN_SYS("ioctl cmd %d nonexist!\n", cmd);
    }
    up(&s_stSocData.stSem);
    return ret;
}

static mt_s32 SysProcShow(struct seq_file *s, mt_void *pArg)
{
    MT_CHIP_TYPE_E      ChipType    = 0;//MT_CHIP_TYPE_BUTT;
    MT_CHIP_VERSION_E   ChipVersion = 0;
    mt_char            *ChipName;
    mt_u32 u32DolbySupport ;
    mt_u32 u32DtsSupport ;
    mt_u32 u32RoviSupport ;
    mt_u32 u32AdvcaSupport;

    mt_drv_sys_getchipversion(&ChipType, &ChipVersion);
    PROC_PRINT(s, "%s\n", s_szSdkKoVersion);
/*
    switch (ChipType)
    {

            ChipName = "UNKNOWN";
    }
*/
    ChipName = "symphony";
    PROC_PRINT(s, "CHIP_VERSION: %s(0x%x)_v%x\n", ChipName, ChipType, ChipVersion);

    if (MT_SUCCESS == mt_drv_sys_getdolbysupport(&u32DolbySupport))
    {
        PROC_PRINT(s, "DOLBY: %s\n", (u32DolbySupport) ? "YES" : "NO");
    }

    if (MT_SUCCESS == mt_drv_sys_getdtssupport(&u32DtsSupport))
    {
        PROC_PRINT(s, "DTS: %s\n", (u32DtsSupport) ? "YES" : "NO");
    }

    if (MT_SUCCESS == mt_drv_sys_getadvcasupport(&u32AdvcaSupport))
    {
        PROC_PRINT(s, "ADVCA: %s\n", (u32AdvcaSupport) ? "YES" : "NO");
    }

    if (MT_SUCCESS == mt_drv_sys_getrovisupport(&u32RoviSupport))
    {
        PROC_PRINT(s, "ROVI(Macrovision): %s\n", (u32RoviSupport) ? "YES" : "NO");
    }

    return 0;
}

static long CMPI_SYS_Ioctl(struct file *file, mt_u32 cmd, unsigned long arg)
{
    long ret;
    ret=(long)mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, SYS_Ioctl);
    return ret;
}

static mt_s32 CMPI_SYS_Open(struct inode * vinode, struct file * vfile)
{
    return 0;
}

static mt_s32 CMPI_SYS_Close(struct inode * vinode, struct file * vfile)
{
    return 0;
}


static struct file_operations stFileOp =
{
     .owner       = THIS_MODULE,
     .open        = CMPI_SYS_Open,
     .unlocked_ioctl  = CMPI_SYS_Ioctl,
     .release     = CMPI_SYS_Close
};




#if 0
static mt_device_s s_stDevice;

mt_s32 mt_drv_sys_init(mt_void)
{
    mt_drv_proc_t stFnOpt =
    {
         .fnRead = SysProcShow,
    };

    sema_init(&s_stSocData.stSem, 1);

    mt_osal_snprintf(s_stDevice.devfs_name, sizeof(s_stDevice.devfs_name), UMAP_DEVNAME_SYS);
    s_stDevice.fops = &stFileOp;
    s_stDevice.minor = UMAP_MIN_MINOR_SYS;
    s_stDevice.owner  = THIS_MODULE;
    s_stDevice.drvops = NULL;
    if (mt_drv_dev_register(&s_stDevice))
    {
        MT_ERR_SYS("Register system device failed!\n");
        goto OUT;
    }

    mt_drv_proc_add_module(MT_MOD_SYS, &stFnOpt, 0);

    return 0;

OUT:
    MT_WARN_SYS("load sys ...FAILED!\n");
    return MT_FAILURE;
}

mt_void mt_drv_sys_exit(mt_void)
{
    mt_drv_proc_rm_module(MT_MOD_SYS);
    mt_drv_dev_unregister(&s_stDevice);
}
#endif






struct mt_drv_sys_device {
	struct device *dev;
	void *platdata;
	dev_t devt;
	int id;
};

struct mt_drv_sys_driver {
	struct cdev cdev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct mt_drv_sys_driver *mt_drv_sys_drv;

static int mt_drv_sys_probe(struct platform_device *pdev)
{
	mt_proc_entry_t *item = NULL;
	struct mt_drv_sys_device *mt_drv_sys_dev = NULL;
	int ret = 0;
    mt_drv_proc_t stFnOpt = {
         .fnRead = SysProcShow,
    };

    sema_init(&s_stSocData.stSem, 1);

	mt_drv_sys_dev = kzalloc(sizeof(struct mt_drv_sys_device), GFP_KERNEL);
	if (!mt_drv_sys_dev) {
		pr_err("Error kzalloc mt_drv_sys_dev\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mt_drv_sys_dev;
	}
	mt_drv_sys_dev->id = pdev->id;
	mt_drv_sys_dev->devt = mt_drv_sys_drv->devt + mt_drv_sys_dev->id;
	mt_drv_sys_dev->platdata = dev_get_platdata(&pdev->dev);

#if 0
	mt_drv_sys_dev->dev = device_create(mt_class, &pdev->dev, mt_drv_sys_dev->devt, NULL, UMAP_DEVNAME_SYS);
#else
	mt_drv_sys_dev->dev = device_create(mt_class, NULL, mt_drv_sys_dev->devt, NULL, UMAP_DEVNAME_SYS);
#endif
	if (IS_ERR(mt_drv_sys_dev->dev)) {
		pr_err("Error device_create\n\n");
		ret = PTR_ERR(mt_drv_sys_dev->dev);
		goto fail_device_create;
	}

	platform_set_drvdata(pdev, mt_drv_sys_dev);
	dev_set_drvdata(mt_drv_sys_dev->dev, mt_drv_sys_dev);

	item = mt_drv_proc_add_module(MT_MOD_SYS, &stFnOpt, 0);
    if (!item) {
        goto fail_mt_drv_proc_add_module;
    }

	return 0;

fail_mt_drv_proc_add_module:
	device_destroy(mt_class, mt_drv_sys_dev->devt);
fail_device_create:
	kfree(mt_drv_sys_dev);
	mt_drv_sys_dev = NULL;
fail_kzalloc_mt_drv_sys_dev:
	return ret;
}

static int mt_drv_sys_remove(struct platform_device *pdev)
{
	struct mt_drv_sys_device *mt_drv_sys_dev = platform_get_drvdata(pdev);

	mt_drv_proc_rm_module(MT_MOD_SYS);

	device_destroy(mt_class, mt_drv_sys_dev->devt);
	mt_drv_sys_dev->dev = NULL;

	platform_set_drvdata(pdev, NULL);

	if (mt_drv_sys_dev) {
		kfree(mt_drv_sys_dev);
		mt_drv_sys_dev = NULL;
	}

	return 0;
}

static int mt_drv_sys_suspend(struct platform_device *pdev, pm_message_t stState)
{
	return 0;
}

static int mt_drv_sys_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver mt_drv_sys_platform_driver = {
	.probe = mt_drv_sys_probe,
	.remove = mt_drv_sys_remove,
	.suspend = mt_drv_sys_suspend,
	.resume = mt_drv_sys_resume,
	.driver = {
		.name = UMAP_DEVNAME_SYS,
		.owner = THIS_MODULE,
	}
};

static void mt_drv_sys_release_device(struct device *pdev) {  }

static struct platform_device mt_drv_sys_platform_device = {
	.name = UMAP_DEVNAME_SYS,
	.id = 0,
	.dev= {
		.platform_data = NULL,
		.release = mt_drv_sys_release_device,
	},
};

int __init mt_drv_sys_init(void)
{
	int ret;

	mt_drv_sys_drv = kzalloc(sizeof(struct mt_drv_sys_driver), GFP_KERNEL);
	if (!mt_drv_sys_drv) {
		pr_err("Error kzalloc mt_drv_sys_drv\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_mt_drv_sys_drv;
	}

	mt_drv_sys_drv->major = MT_DEVICE_MAJOR;
	mt_drv_sys_drv->minor = UMAP_MIN_MINOR_SYS;
	mt_drv_sys_drv->minors = UMAP_DEV_NUM_SYS;
	mt_drv_sys_drv->devt = MKDEV(mt_drv_sys_drv->major, mt_drv_sys_drv->minor);
	cdev_init(&mt_drv_sys_drv->cdev, &stFileOp);
	mt_drv_sys_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&mt_drv_sys_drv->cdev, mt_drv_sys_drv->devt, mt_drv_sys_drv->minors);
	if (ret) {
		pr_err("Error cdev_add\n\n");
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&mt_drv_sys_platform_driver);
	if (ret) {
		pr_err("Error platform_driver_register\n\n");
		goto fail_platform_driver_register;
	}

	ret = platform_device_register(&mt_drv_sys_platform_device);
	if (ret) {
		pr_err("Error platform_device_register\n\n");
		goto fail_platform_device_register;
	}

	return 0;

fail_platform_device_register:
	platform_driver_unregister(&mt_drv_sys_platform_driver);
fail_platform_driver_register:
	cdev_del(&mt_drv_sys_drv->cdev);
fail_cdev_add:
	kfree(mt_drv_sys_drv);
	mt_drv_sys_drv = NULL;
fail_kzalloc_mt_drv_sys_drv:
	return ret;
}

void mt_drv_sys_exit(void)
{
	platform_driver_unregister(&mt_drv_sys_platform_driver);
	platform_device_unregister(&mt_drv_sys_platform_device);
	kfree(mt_drv_sys_drv);
	mt_drv_sys_drv = NULL;
}

