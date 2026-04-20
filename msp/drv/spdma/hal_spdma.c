/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/kern_levels.h>
#include <linux/printk.h>
#include <linux/dma-direct.h>


#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#include <linux/clk.h>
#include <linux/clk/clk-conf.h>

/*
#include <linux/unistd.h>
#include <linux/time.h>
#include <linux/errno.h>
*/

#include <asm/cacheflush.h>

#include "mt_type.h"
#include "mt_osal.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "mt_common.h"

#include "mt_drv_spdma.h"
#include "hal_spdma_regs.h"
//#include "spdma_aria.h"
#include "mt_module_debug.h"
#include "mt_drv_proc.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/irq.h"
#include "mt_drv_clock.h"


/*
mt_u32 cmd;
mt_u32 shift_flag;
mt_u32 saturated_flag;
mt_u32 round_flag;
mt_u32 cut_flag;
mt_u32 burst_len;
mt_u32 transit_len;



#define SPDMA_DATA_IDX_TYPE						0
#define SPDMA_DATA_IDX_SHIFT					1
#define SPDMA_DATA_IDX_SATURATED				2
#define SPDMA_DATA_IDX_ROUND					3
#define SPDMA_DATA_IDX_CUT						4
#define SPDMA_DATA_IDX_BURST					5
#define SPDMA_DATA_IDX_TRANSIT					6
#define SPDMA_DATA_IDX_LINE						6

*/


#define SYM6_SRAM_ADDR		0x1e980000//0xbe980000//
#define SYM6_BRAM_ADDR		0x1e800000//0x1e800000


#define R_SYM6_SRAM_ADDR		SYMPHONY_IO_VA(0xbe980000)//0xbe980000//
#define R_SYM6_BRAM_ADDR		SYMPHONY_IO_VA(0xbe800000)//0x1e800000
#define R_SYM6_BOOT_ADDR		SYMPHONY_IO_VA(0xbf140010)

#define R_SYM6_BSRAM_ADDR		SYMPHONY_IO_VA(0xbe880000)//0xbe980000//

#define PROC_PARAM_MAXLEN (64)


//#define SPDMA_PRINTF(format, args...)	mt_drv_proc_echohelp(format, ##args)
#define SPDMA_PRINTF(fmt, ...) printk(KERN_INFO fmt, ##__VA_ARGS__)


struct mt_spdmac_dev
{
	struct device *dev;
	struct class *spdmac_class;
	ulong base;
	unsigned int irq;
	unsigned int open_cnt;
	unsigned int initflag;
	struct mutex mutex_spdmac;
};

struct spdmac_device {
	struct cdev cdev;
	struct mt_spdmac_dev spdmac_dev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

extern mt_s32 mt_drv_module_unregister(mt_u32 u32ModuleID);
extern mt_s32 mt_drv_module_register(mt_u32 u32ModuleID, const mt_u8* pu8ModuleName, mt_void* pFunc);


//extern int usleep(useconds_t usec);

//#define SPDMA_DELAY		1


static mt_s32 hal_spdma_open(struct inode *inode, struct file *filp)
{
    return SPDMA_SUCCESS;
}

static mt_s32 hal_spdma_close(struct inode *inode, struct file *filp)
{
    return SPDMA_SUCCESS;
}

static long hal_spdma_ioctl( struct file *filp,unsigned int cmd, unsigned long arg)
{
  //struct inode *inode = filp->f_path.dentry->d_inode;
  //hal_spdma_io_param_t spdma_param = {0,};

  mt_s32 err = 0;

  if(_IOC_TYPE(cmd) != SPDMA_IOC_MAGIC)
  {
    return -EINVAL;
  }

  if(_IOC_NR(cmd) > SPDMA_IOC_MAXNR)
  {
    return -EINVAL;
  }
/*
  if(_IOC_DIR(cmd) & _IOC_READ)
  {
    err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
  }
  else if(_IOC_DIR(cmd) & _IOC_WRITE)
  {
    err = !access_ok(VERIFY_READ,  (void *)arg, _IOC_SIZE(cmd));
  }

  if(err)
  {
    return -EFAULT;
  }
*/
  switch (cmd)
  {
    case SPDMA_IOC_START:
/*
        err = copy_from_user(&spdma_param, (void *)arg, sizeof(hal_spdma_io_param_t));
		if (err != 0)
		   break;
        MT_INFO_DMA("src_phy_addr = 0x%x, src_vir_addr = 0x%x, dst_phy_addr = 0x%x, dst_vir_addr = 0x%x\n",
                spdma_param.param.phy_src_addr, spdma_param.param.vir_src_addr,
                spdma_param.param.phy_dst_addr, spdma_param.param.vir_dst_addr);

		spdma_param.chn_id = hal_dma_get_free_channel(spdma_param.flags);
		if (spdma_param.chn_id >= 0 && spdma_param.chn_id < DMA_CHN_ID_MAX)
		{
	        err = hal_dma_start(spdma_param.chn_id, (void *)&spdma_param.param, (void *)spdma_param.p_notify);

			if (err == DMA_SUCCESS)
	        	err = copy_to_user((void __user *)arg, (void*)&spdma_param, sizeof(hal_dma_io_param_t));
	    }
	    else
	    {
	    	MT_ERR_DMA("dma get free channel failed!\n");
	    	err = DMA_ERR_NO_RSRC;
	    }
*/
        break;

    default:
        MT_INFO_DMA("-------------\n");
        break;
  }

  MT_INFO_DMA("err = %d\n",err);

  return err;
}

static struct file_operations spdma_fops =
{
	owner: THIS_MODULE,
	open: hal_spdma_open,
	unlocked_ioctl: hal_spdma_ioctl,
	release: hal_spdma_close,
};


static void spdma_prochelp(void)
{
    SPDMA_PRINTF("\n------------SPDma test help------------\n");
	//SPDMA_PRINTF("cat  /proc/msp/spdma	-- display spdma info\n");
	SPDMA_PRINTF("echo help > /proc/msp/spdma  -- display help\n");
	SPDMA_PRINTF("\n");
}


mt_s32 spdma_procread(struct seq_file *p, mt_void *v)
{
    SPDMA_PRINTF("\n------------SPDma------------\n");
	SPDMA_PRINTF("Spdma Status: %s\n",((HAL_GET_U32((volatile u32 *)(R_SPDMA_BUSY_STATUS)) & 0x01) == 1)?"Busy":"Free");
	//MT_PRINT("%s\n",((readl((void*)(R_SPDMA_BUSY_STATUS)) & 0x01) == 1)?"Busy":"Free");
	return MT_SUCCESS;
}



mt_s32 spdma_procwrite(struct file * file, const char __user * buf, size_t count, loff_t *ppos)
{
  mt_char ProcPara[PROC_PARAM_MAXLEN] = {0};
  mt_char *param = NULL;
  mt_u32 status = 0;

  if(count > PROC_PARAM_MAXLEN)
  {
    SPDMA_PRINTF("\nwrite data is too long!\n");//MT_PRINT
    return -EFAULT;
  }

  if(copy_from_user(ProcPara, buf, count))
  {
    SPDMA_PRINTF("\nwrite data is too long!\n");
    return -EFAULT;
  }
  ProcPara[PROC_PARAM_MAXLEN-1] = 0;
  param =ProcPara;

  SPDMA_PRINTF("SPdma cmd:%s\n",param);//MT_PRINT
  status = HAL_GET_U32((volatile u32 *)R_SPDMA_BUSY_STATUS);
  if(status)
  {
    SPDMA_PRINTF("SPDma is busy...");
    return SPDMA_ERR_BUSY;
  }

  if(strstr(param,"help"))
  {
    spdma_prochelp();
  }

  return count;
}


static ssize_t spdmac_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	// Code to read the attribute value and store it in the buf
	return 0;
}

static ssize_t spdmac_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	// Code to write the attribute value from buf
	return 0;
}

static DEVICE_ATTR(spdmac, 0664, spdmac_show, spdmac_store);

static int symphony_spdmac_probe(struct platform_device *pdev)
{

	int ret = 0;
	struct spdmac_device *spdmacdev;
	mt_proc_entry_t *pProcItem;
	struct resource *res;
	int irq;

	spdmacdev = kzalloc(sizeof(struct spdmac_device), GFP_KERNEL);
	if (NULL == spdmacdev) {
		ret = -ENOMEM;
		goto fail_spdmac_kzalloc;
	}

	spdmacdev->spdmac_dev.spdmac_class = class_create(UMAP_DEVNAME_SPDMA);
	if (IS_ERR(spdmacdev->spdmac_dev.spdmac_class)) {
		ret = PTR_ERR(spdmacdev->spdmac_dev.spdmac_class);
		goto fail_spdmac_class;
	}
	
	spdmacdev->minor = UMAP_MIN_MINOR_SPDMA;
	spdmacdev->minors = UMAP_DEV_NUM_SPDMA;
	spdmacdev->devt = MKDEV(MT_DEVICE_MAJOR, spdmacdev->minor);
	spdmacdev->major = MAJOR(spdmacdev->devt);

	spdmacdev->spdmac_dev.base = SYMPHONY_IO_VA(0xbe900000);
	spdmacdev->spdmac_dev.irq = IRQ_SPDMA_ID;
	spdmacdev->spdmac_dev.initflag = 0;
	spdmacdev->spdmac_dev.dev = NULL;
	
	cdev_init(&spdmacdev->cdev, &spdma_fops);
	spdmacdev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&spdmacdev->cdev, spdmacdev->devt, spdmacdev->minors);
	if (ret) {
		ret = -EINVAL;
		goto fail_spdmac_cdev;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq) {
		spdmacdev->spdmac_dev.irq = irq;
		printk("[%s_%d]0x%d\n", __func__, __LINE__, irq);
	}
	else {
		printk("[%s_%d]get irq number fail\n", __func__, __LINE__);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		spdmacdev->spdmac_dev.base = SYMPHONY_IO_VA(res->start);
		printk("[%s_%d]start = 0x%lx\n", __func__, __LINE__, (ulong)res->start);
		printk("[%s_%d]end = 0x%lx\n", __func__, __LINE__, (ulong)res->end);
	}

	spdmacdev->spdmac_dev.dev = device_create(spdmacdev->spdmac_dev.spdmac_class, NULL, spdmacdev->devt, NULL, UMAP_DEVNAME_SPDMA);
	if (IS_ERR(spdmacdev->spdmac_dev.dev)) {
			ret = PTR_ERR(spdmacdev->spdmac_dev.dev);
			printk("spdma device_create failed. ret = %d\n",ret);
			goto fail_spdmac_device;
		}

	if (device_create_file(&pdev->dev, &dev_attr_spdmac)) {
		ret = -ENOENT;
		printk("spdma device_create_file failed.\n");
		goto fail_spdmac_create_file;
	}

	pProcItem = mt_drv_proc_add_module(MT_MOD_SPDMA, NULL, NULL);
	if (!pProcItem)
	{
		printk("add spdma proc failed.\n");
		ret = -1;
		goto fail_spdmac_create_file;
	}

	pProcItem->read = spdma_procread;
	pProcItem->write = spdma_procwrite;

	platform_set_drvdata(pdev, spdmacdev);
	dev_set_drvdata(spdmacdev->spdmac_dev.dev, spdmacdev);

	return ret;
	
fail_spdmac_create_file:
	device_destroy(spdmacdev->spdmac_dev.spdmac_class, spdmacdev->devt);
fail_spdmac_device:
	cdev_del(&spdmacdev->cdev);
fail_spdmac_cdev:
	class_destroy(spdmacdev->spdmac_dev.spdmac_class);
fail_spdmac_class:
	kfree(spdmacdev);
fail_spdmac_kzalloc:
	return ret;	
	
}


static int symphony_spdmac_remove(struct platform_device *pdev)
{
	struct spdmac_device *spdmacdev = NULL;

	spdmacdev = platform_get_drvdata(pdev);
	if (spdmacdev) {
		if (spdmacdev->spdmac_dev.spdmac_class) {
			device_destroy(spdmacdev->spdmac_dev.spdmac_class, spdmacdev->devt);
		}
		cdev_del(&spdmacdev->cdev);
		if (spdmacdev->spdmac_dev.spdmac_class) {
			class_destroy(spdmacdev->spdmac_dev.spdmac_class);
		}
		kfree(spdmacdev);
	}

	return 0;
}

static int symphony_spdmac_suspend(struct platform_device *pdev, pm_message_t stState)
{
	return 0;
}

static int symphony_spdmac_resume(struct platform_device *pdev)
{
	return 0;
}


#if defined(CONFIG_OF)
	 static const struct of_device_id symphony_spdmac_of_match[] = {
		 { .compatible = "montage,spdmac" },
		 {},
	 };
MODULE_DEVICE_TABLE(of, symphony_spdmac_of_match);
#endif

static struct platform_driver symphony_spdmac_driver = {
	.probe 		= symphony_spdmac_probe,
	.remove		= symphony_spdmac_remove,
	.suspend	= symphony_spdmac_suspend,
	.resume		= symphony_spdmac_resume,
	.driver	= {
		.name = UMAP_DEVNAME_SPDMA,
		.of_match_table = of_match_ptr(symphony_spdmac_of_match),
	},
};


int __init spdma_drv_modinit(void)
{

	int  ret = 0;

	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf50820c),0x3f);		//release avcpu and spdma
	ret = platform_driver_register(&symphony_spdmac_driver);
	if (ret){
		return ret;
	}

	return 0;
}

void __exit spdma_drv_modexit(void)
{
	platform_driver_unregister(&symphony_spdmac_driver);
}

