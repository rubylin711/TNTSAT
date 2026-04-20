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

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#include <linux/clk.h>
#include <linux/clk/clk-conf.h>

#include <asm/cacheflush.h>
#include <asm/io.h>

//#include <../../../kernel/linux-x.y.z/include/linux/time.h>
#include <linux/time.h>
//#include <sys/time.h>
//#include <time.h>
#include <mt_cache.h>


#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "mt_type.h"
#include "mt_osal.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dma.h"
#include "hal_dma_regs.h"
#include "dma_aria.h"
#include "mt_module_debug.h"
#include "mt_drv_proc.h"
#include "sys_define.h"
#include <linux/dma-direct.h>

#define PROC_PARAM_MAXLEN (64 * 2)

#define APP_ACTIVE_CH	(0xaa)

#define DMA_XTAL_CFG_FLAG	(0x40)

#define SYM6_BRAM_ADDR		SYMPHONY_BSRAM_RAM_PHYS_BASE

#define DMA_SRCDATA_INIT_0X4000	(1)
#define DMA_SRCDATA_INIT_SIZE	(2)
#define DMA_DESTDATA_NOT_INIT	(4)
#define DMA_SECURE_OPERATION	(8)
#define DMA_LOOPCHECK_MODE		(16)
#define DMA_NOPRINT_DEBUG			(32)
#define DMA_MEM_MMZ			(64)
#define DMA_MEM_DELMMZ			(128)

#define DEFAULT_DMA_FREQ		262000000

struct dmac_device {
	struct cdev cdev;
	struct mt_dmac_dev dmac_dev;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

enum
{
	NO_USE = 0,
	USED   = 1,
};

typedef struct
{
	u8 dma_num;
	u8 used; //0:no use 1:used
	u8 flag;
}dma_state;
static dma_state dma_cap[DMA_CHN_ID_MAX];


static hal_dma_op_t g_hal_dma_op = {0};

static struct dmac_device *g_dmac_drv = NULL;


static DEFINE_SPINLOCK(g_hal_dma_lock);

extern mt_s32 mt_drv_module_unregister(mt_u32 u32ModuleID);
extern mt_s32 mt_drv_module_register(mt_u32 u32ModuleID, const mt_u8* pu8ModuleName, mt_void* pFunc);

extern mt_u32  mt_drv_timer_read_cnt(int id);

extern int dma_aria_init(hal_dma_op_t *p_dma,struct mt_dmac_dev dev);
extern int dma_aria_deinit(hal_dma_op_t *p_dma);

static hal_dma_op_t *hal_get_dma_hdl(mt_void)
{
  return &g_hal_dma_op;
}

mt_s32 hal_dma_capacity_get(hal_dma_capacity_t *p_capacity)
{

  mt_s32 ret = -1;
  hal_dma_op_t *p_dma;

  p_dma = hal_get_dma_hdl();

  if(p_dma->capacity_get == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }
  ret= p_dma->capacity_get((mt_u32 *)p_capacity, (mt_u32 *)p_dma);

  return ret;
}
EXPORT_SYMBOL(hal_dma_capacity_get);

mt_s32  hal_dma_get_free_channel(unsigned int ch_flags)
{
	mt_s32 chn_id = 0;
	unsigned long flags;

	/*
	 * display interrupt isr will call dma api,
	 * and audio will call dma api too,
	 * so use spinlock and disable interrupt while allocate dma channel.
	 */
	spin_lock_irqsave(&g_hal_dma_lock, flags);

	for(chn_id = 0; chn_id < DMA_CHN_ID_MAX; chn_id++)
	{
	  if (ch_flags && ((ch_flags & (0x01 << chn_id)) == 0))
	  {
	  	continue;
	  }

	  if ((NO_USE == dma_cap[chn_id].used)&& (!hal_dma_check(chn_id)))
	  {
		dma_cap[chn_id].used = USED;
		spin_unlock_irqrestore(&g_hal_dma_lock, flags);
		//printk("%s: ch_flags %x, chn_id %d\n",__FUNCTION__,ch_flags,chn_id);
		return chn_id;
	  }
	}

	spin_unlock_irqrestore(&g_hal_dma_lock, flags);
	return DMA_CHN_ID_MAX;
}
EXPORT_SYMBOL(hal_dma_get_free_channel);

mt_s32 hal_dma_start(mt_s32 id, mt_void *param, mt_void *p_notify)
{
  hal_dma_op_t *p_dma;
  mt_s32 ret;

  p_dma = hal_get_dma_hdl();

  if(p_dma->start == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }

  ret = p_dma->start(id, (u32 *)param ,(u32 *)p_notify, (u32 *)p_dma);

  return ret;
}
EXPORT_SYMBOL(hal_dma_start);

mt_s32 hal_dma_check(mt_s32 id)
{
  hal_dma_op_t *p_dma;
  mt_s32  ret = -1;

  p_dma = hal_get_dma_hdl();

  if(p_dma->check == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }

  ret =p_dma->check(id);

  MT_INFO_DMA("ret=%d\n",ret);

  return ret;
}
EXPORT_SYMBOL(hal_dma_check);

mt_s32 hal_dma_pause(mt_s32 id)
{
  hal_dma_op_t *p_dma;
  mt_s32  ret = -1;

  p_dma = hal_get_dma_hdl();

  if(p_dma->pause == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }

  ret = p_dma->pause(id);

  return ret;
}
EXPORT_SYMBOL(hal_dma_pause);

mt_s32 hal_dma_resume(mt_s32 id)
{
  hal_dma_op_t *p_dma;
  mt_s32  ret = -1;

  p_dma = hal_get_dma_hdl();

  if(p_dma->resume == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }
  ret = p_dma->resume(id);

  return ret;
}
EXPORT_SYMBOL(hal_dma_resume);

mt_s32 hal_dma_stop(mt_s32 id)
{
  hal_dma_op_t *p_dma = hal_get_dma_hdl();
  mt_s32 ret = 0;
  unsigned long flags;

  if(p_dma->stop == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }

  ret = p_dma->stop(id);

  if(dma_cap[id].flag != APP_ACTIVE_CH)
  {
	  spin_lock_irqsave(&g_hal_dma_lock, flags);
	  dma_cap[id].used = NO_USE;
	  spin_unlock_irqrestore(&g_hal_dma_lock, flags);
  }
  return ret;
}
EXPORT_SYMBOL(hal_dma_stop);

mt_s32 hal_dma_reset(mt_void)
{
  hal_dma_op_t *p_dma;
  mt_s32 ret = -1;

  p_dma = hal_get_dma_hdl();

  if(p_dma->reset == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }
  ret = p_dma->reset();

  return ret;
}
EXPORT_SYMBOL(hal_dma_reset);

mt_s32 hal_dma_soft_reset(mt_s32 id)
{
  hal_dma_op_t *p_dma;
  mt_s32 ret = -1;

  p_dma = hal_get_dma_hdl();

  if(p_dma->soft_reset == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }
  ret = p_dma->soft_reset(id);

  return ret;
}
EXPORT_SYMBOL(hal_dma_soft_reset);

mt_s32 hal_dma_push(mt_s32 id, mt_u32 length)
{
  hal_dma_op_t *p_dma;
  mt_s32 ret = -1;

  p_dma = hal_get_dma_hdl();

  if(p_dma->push == NULL)
  {
    return DMA_ERR_NOFEATURE;
  }
  ret = p_dma->push(id, length);

  return ret;
}
EXPORT_SYMBOL(hal_dma_push);

mt_s32 hal_dma_init(void)
{
	mt_s32 ret=0;
	if (NULL == g_dmac_drv){
		return DMA_ERR_FAILURE;
	}
	ret=dma_aria_init(hal_get_dma_hdl(),g_dmac_drv->dmac_dev);
	if (ret){
		printk("dma_aria_init err ret = %d. \n",ret);
		return ret;
	}
	return DMA_SUCCESS;
}

EXPORT_SYMBOL(hal_dma_init);

mt_s32 hal_dma_deinit(void)
{

  return dma_aria_deinit(hal_get_dma_hdl());
}
EXPORT_SYMBOL(hal_dma_deinit);

mt_s32 hal_dma_active_channel(mt_s32 id)
{
	unsigned long flags;

	if((id < 0) || (id >= DMA_CHN_ID_MAX))
	{
		return DMA_ERR_PARAM;
	}

	spin_lock_irqsave(&g_hal_dma_lock, flags);

	if ((NO_USE == dma_cap[id].used)&& (!hal_dma_check(id)))
	{
		dma_cap[id].used = USED;
		dma_cap[id].flag = APP_ACTIVE_CH;
		spin_unlock_irqrestore(&g_hal_dma_lock, flags);
		return id;
	}

	spin_unlock_irqrestore(&g_hal_dma_lock, flags);
	return DMA_ERR_BUSY;
}
EXPORT_SYMBOL(hal_dma_active_channel);

mt_s32 hal_dma_deactive_channel(mt_s32 id)
{
	unsigned long flags;
	if((id < 0) || (id >= DMA_CHN_ID_MAX))
	{
		return DMA_ERR_PARAM;
	}
	spin_lock_irqsave(&g_hal_dma_lock, flags);
	if ((dma_cap[id].flag == APP_ACTIVE_CH) && (USED == dma_cap[id].used)&& (0 == hal_dma_check(id)))
	{
		dma_cap[id].used = NO_USE;
		dma_cap[id].flag = 0;
	}
	else
	{
		spin_unlock_irqrestore(&g_hal_dma_lock, flags);
		return DMA_ERR_FAILURE;
	}
	spin_unlock_irqrestore(&g_hal_dma_lock, flags);
	return 0;
}
EXPORT_SYMBOL(hal_dma_deactive_channel);

static mt_s32 hal_dma_open(struct inode *inode, struct file *filp)
{
    return DMA_SUCCESS;
}

static mt_s32 hal_dma_close(struct inode *inode, struct file *filp)
{
    return DMA_SUCCESS;
}

static long hal_dma_ioctl( struct file *filp,unsigned int cmd, unsigned long arg)
{
  //struct inode *inode = filp->f_path.dentry->d_inode;
  hal_dma_io_param_t dma_param = {0,};
  dma_addr_priv_t dma_addinfo = {0};
  mt_s32 chn_id = 0;
  mt_s32 err = 0;

  if(_IOC_TYPE(cmd) != DMA_IOC_MAGIC)
  {
    return -EINVAL;
  }

  if(_IOC_NR(cmd) > DMA_IOC_MAXNR)
  {
    return -EINVAL;
  }

  if(_IOC_DIR(cmd) & _IOC_READ)
  {
    err = !access_ok((void *)arg, _IOC_SIZE(cmd));
  }
  else if(_IOC_DIR(cmd) & _IOC_WRITE)
  {
    err = !access_ok((void *)arg, _IOC_SIZE(cmd));
  }

  if(err)
  {
    return -EFAULT;
  }

  switch (cmd)
  {
    case DMA_IOC_START:
        err = copy_from_user(&dma_param, (mt_void *)arg, sizeof(hal_dma_io_param_t));
		if (err != 0)
		   break;
        MT_INFO_DMA("src_phy_addr = 0x%x, src_vir_addr = 0x%x, dst_phy_addr = 0x%x, dst_vir_addr = 0x%x\n",
                dma_param.param.phy_src_addr, dma_param.param.vir_src_addr,
                dma_param.param.phy_dst_addr, dma_param.param.vir_dst_addr);

		dma_param.chn_id = hal_dma_get_free_channel(dma_param.flags);
		if (dma_param.chn_id >= 0 && dma_param.chn_id < DMA_CHN_ID_MAX)
		{
	        err = hal_dma_start(dma_param.chn_id, (mt_void *)&dma_param.param, (mt_void *)dma_param.p_notify);

			if (err == DMA_SUCCESS)
	        	err = copy_to_user((void __user *)arg, (void*)&dma_param, sizeof(hal_dma_io_param_t));
	    }
	    else
	    {
	    	MT_ERR_DMA("dma get free channel failed!\n");
	    	err = DMA_ERR_NO_RSRC;
	    }

        break;
    case DMA_IOC_START_WITH_CH:
        err = copy_from_user(&dma_param, (mt_void *)arg, sizeof(hal_dma_io_param_t));
		if (err != 0)
		   break;
        MT_INFO_DMA("src_phy_addr = 0x%x, src_vir_addr = 0x%x, dst_phy_addr = 0x%x, dst_vir_addr = 0x%x\n",
                dma_param.param.phy_src_addr, dma_param.param.vir_src_addr,
                dma_param.param.phy_dst_addr, dma_param.param.vir_dst_addr);

		if (dma_param.chn_id >= 0 && dma_param.chn_id < DMA_CHN_ID_MAX)
		{
	        err = hal_dma_start(dma_param.chn_id, (mt_void *)&dma_param.param, (mt_void *)dma_param.p_notify);

			if (err == DMA_SUCCESS)
				err = copy_to_user((void __user *)arg, (void*)&dma_param, sizeof(hal_dma_io_param_t));
	    }
	    else
	    {
			MT_ERR_DMA("dma channel id error!\n");
			err = DMA_ERR_PARAM;
	    }

        break;

    case DMA_IOC_ACTIVE_CH:
        err = copy_from_user(&chn_id, (mt_s32 *)arg, sizeof(mt_s32));
		if (err != 0)
		   break;
        err = hal_dma_active_channel(chn_id);
        break;

    case DMA_IOC_DEACTIVE_CH:
        err = copy_from_user(&chn_id, (mt_s32 *)arg, sizeof(mt_s32));
		if (err != 0)
		   break;
        err = hal_dma_deactive_channel(chn_id);
        break;

    case DMA_IOC_CHECK:
        err = copy_from_user(&chn_id, (mt_s32 *)arg, sizeof(mt_s32));
		if (err != 0)
		   break;
        err = hal_dma_check(chn_id);
        break;

    case DMA_IOC_PAUSE:
        err = copy_from_user(&chn_id, (mt_s32 *)arg, sizeof(mt_s32));
		if (err != 0)
		   break;
        err = hal_dma_pause(chn_id);
        break;

    case DMA_IOC_STOP:
        err = copy_from_user(&chn_id, (mt_s32 *)arg, sizeof(mt_s32));
		if (err != 0)
		   break;
        err = hal_dma_stop(chn_id);
        break;
    case DMA_IOC_RESUME:
        err = copy_from_user(&chn_id, (mt_s32 *)arg, sizeof(mt_s32));
		if (err != 0)
		   break;
        err = hal_dma_resume(chn_id);
        break;

    case DMA_IOC_CAPACITY_GET:
        break;

	case DMA_IOC_ADDR_TRANSLATION:
		err = copy_from_user(&dma_addinfo, (mt_void *)arg, sizeof(dma_addr_priv_t));
		if (err != 0)
		   break;
		if (NULL == g_dmac_drv)
		{
			err = -1;
			break;
		}
		if (1 == dma_addinfo.src_flag)
		{
			dma_addinfo.dma_src_addr = phys_to_dma(g_dmac_drv->dmac_dev.dev, dma_addinfo.cpu_src);
		}
		if (1 == dma_addinfo.dst_flag)
		{
			dma_addinfo.dma_dst_addr = phys_to_dma(g_dmac_drv->dmac_dev.dev, dma_addinfo.cpu_dst);
		}
		err = copy_to_user((void __user *)arg, (void*)&dma_addinfo, sizeof(dma_addr_priv_t));
		break;

    default:
        MT_INFO_DMA("-------------\n");
        break;
  }

  MT_INFO_DMA("err = %d\n",err);

  return err;
}

static struct file_operations dmac_fops =
{
	.owner = THIS_MODULE,
	.open = hal_dma_open,
	.unlocked_ioctl = hal_dma_ioctl,
	.release = hal_dma_close,
};

mt_s32 dma_procread(struct seq_file *p, mt_void *v)
{
	MT_PRINT("0x0054 = 0x%x\n",readl((void*)(R_DMA_PLUS_BASE_ADDR+0x54)));
	MT_PRINT("Channel Status:\n");
	MT_PRINT("0 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4)) & 0x01) == 1)?"Busy":"Free");
	MT_PRINT("1 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4+0x40)) & 0x01) == 1)?"Busy":"Free");
	MT_PRINT("2 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4+0x80)) & 0x01) == 1)?"Busy":"Free");
	MT_PRINT("3 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4+0xC0)) & 0x01) == 1)?"Busy":"Free");
	MT_PRINT("4 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4+0x100)) & 0x01) == 1)?"Busy":"Free");
	MT_PRINT("5 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4+0x140)) & 0x01) == 1)?"Busy":"Free");
	MT_PRINT("6 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4+0x180)) & 0x01) == 1)?"Busy":"Free");
	MT_PRINT("7 : %s\n",((readl((void*)(R_DMA_PLUS_BASE_ADDR+0xa4+0x1C0)) & 0x01) == 1)?"Busy":"Free");
	return MT_SUCCESS;
}
extern __kernel_size_t strlen(const char *);
mt_s32 dma_procwrite(struct file * file,
                     const char __user * buf, size_t count, loff_t *ppos)

{
	mt_char ProcPara[PROC_PARAM_MAXLEN] = {0};
	mt_char *param = NULL;
	hal_dma_op_t *p_dma_1;

	p_dma_1 = hal_get_dma_hdl();

	if(count > PROC_PARAM_MAXLEN)
	{
		MT_ERR_DMA("write data is too long!\n");
		return -EFAULT;
	}

	if(copy_from_user(ProcPara, buf, count))
	{
		MT_ERR_DMA("write data is too long!\n");
		return -EFAULT;
	}
	ProcPara[PROC_PARAM_MAXLEN-1] = 0;
	param =ProcPara;
	MT_PRINT("[%s]%d param : %s len : %d\n",__FUNCTION__,__LINE__,param,(int)strlen((const char *)param));

	return count;
}

static ssize_t dmac_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	// Code to read the attribute value and store it in the buf
	return 0;
}

static ssize_t dmac_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	// Code to write the attribute value from buf
	return 0;
}

static DEVICE_ATTR(dmac, 0664, dmac_show, dmac_store);

static int symphony_dmac_probe(struct platform_device *pdev)
{

	int ret = 0;
	struct dmac_device *dmacdev;
	mt_proc_entry_t *pProcItem;
	struct resource *res;
	int irq;
	unsigned int tmp_val = 0;

	dmacdev = kzalloc(sizeof(struct dmac_device), GFP_KERNEL);
	if (NULL == dmacdev) {
		ret = -ENOMEM;
		goto fail_dmac_kzalloc;
	}

	dmacdev->dmac_dev.dmac_class = class_create(UMAP_DEVNAME_DMA);
	if (IS_ERR(dmacdev->dmac_dev.dmac_class)) {
		ret = PTR_ERR(dmacdev->dmac_dev.dmac_class);
		goto fail_dmac_class;
	}

	dmacdev->minor = UMAP_MIN_MINOR_DMA;
	dmacdev->minors = UMAP_DEV_NUM_DMA;
	dmacdev->devt = MKDEV(MT_DEVICE_MAJOR, dmacdev->minor);
	dmacdev->major = MAJOR(dmacdev->devt);

	dmacdev->dmac_dev.base = SYMPHONY_IO_VA(0xbf400000);
	dmacdev->dmac_dev.clk = NULL;
	dmacdev->dmac_dev.dmaclk = 24;
	dmacdev->dmac_dev.irq = IRQ_DMA_ID;
	dmacdev->dmac_dev.initflag = 0;
	dmacdev->dmac_dev.dev = NULL;

	cdev_init(&dmacdev->cdev, &dmac_fops);
	dmacdev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&dmacdev->cdev, dmacdev->devt, dmacdev->minors);
	if (ret) {
		ret = -EINVAL;
		goto fail_dmac_cdev;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq) {
		dmacdev->dmac_dev.irq = irq;
		printk("[%s_%d]0x%d\n", __func__, __LINE__, irq);
	}
	else {
		printk("[%s_%d]get irq number fail\n", __func__, __LINE__);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		dmacdev->dmac_dev.base = SYMPHONY_IO_VA(res->start);
		printk("[%s_%d]start = 0x%lx\n", __func__, __LINE__, (ulong)res->start);
		printk("[%s_%d]end = 0x%lx\n", __func__, __LINE__, (ulong)res->end);
	}

	ret = of_property_read_u32(pdev->dev.of_node, "clock-frequency", &tmp_val);
	if (ret) {
		printk("[%s_%d]do not config clock-frequency in dts. use default : uboot cfg!\n", __func__, __LINE__);
	}
	else {
		printk("[%s_%d]config clock-frequency = %d\n", __func__, __LINE__, tmp_val);
	}

	switch(tmp_val) {
		case 262000000:
		case 411500000:
			break;
		default:
			tmp_val = 411500000;
			break;
	}

	dmacdev->dmac_dev.clk = devm_clk_get(&pdev->dev, "dmac_clk");
	if(dmacdev->dmac_dev.clk){
		clk_prepare_enable(dmacdev->dmac_dev.clk);
		dmacdev->dmac_dev.dmaclk = clk_get_rate(dmacdev->dmac_dev.clk);
		if(tmp_val != dmacdev->dmac_dev.dmaclk) {
			dmacdev->dmac_dev.dmaclk = tmp_val;
			ret = clk_set_rate(dmacdev->dmac_dev.clk,dmacdev->dmac_dev.dmaclk);
			if (ret) {
				printk("[%s_%d]clk_set_rate error\n", __func__, __LINE__);
			}
		}
		printk("dma clk =%d\n",dmacdev->dmac_dev.dmaclk);
	}
	else {
		printk("[%s_%d]devm_clk_get error\n", __func__, __LINE__);
	}

	dmacdev->dmac_dev.dev = device_create(dmacdev->dmac_dev.dmac_class, NULL, dmacdev->devt, NULL, UMAP_DEVNAME_DMA);
	if (IS_ERR(dmacdev->dmac_dev.dev)) {
			ret = PTR_ERR(dmacdev->dmac_dev.dev);
			printk("dma device_create failed. ret = %d\n",ret);
			goto fail_dmac_device;
		}

	if (device_create_file(&pdev->dev, &dev_attr_dmac)) {
		ret = -ENOENT;
		printk("dma device_create_file failed.\n");
		goto fail_dmac_create_file;
	}

	g_dmac_drv = dmacdev;
	ret = hal_dma_init();
	pProcItem = mt_drv_proc_add_module(MT_MOD_DMA, NULL, NULL);
	if (!pProcItem)
	{
		printk("add dma proc failed.\n");
		ret = -1;
		g_dmac_drv = NULL;
		goto fail_dmac_create_file;
	}

	pProcItem->read = dma_procread;
	pProcItem->write = dma_procwrite;

	platform_set_drvdata(pdev, dmacdev);
	dev_set_drvdata(dmacdev->dmac_dev.dev, dmacdev);

	return ret;

fail_dmac_create_file:
	device_destroy(dmacdev->dmac_dev.dmac_class, dmacdev->devt);
fail_dmac_device:
	cdev_del(&dmacdev->cdev);
fail_dmac_cdev:
	class_destroy(dmacdev->dmac_dev.dmac_class);
fail_dmac_class:
	kfree(dmacdev);
fail_dmac_kzalloc:
	return ret;

}


static int symphony_dmac_remove(struct platform_device *pdev)
{
	struct dmac_device *dmacdev = NULL;

	dmacdev = platform_get_drvdata(pdev);
	if (dmacdev) {
		if (dmacdev->dmac_dev.dmac_class) {
			device_destroy(dmacdev->dmac_dev.dmac_class, dmacdev->devt);
		}
		cdev_del(&dmacdev->cdev);
		if (dmacdev->dmac_dev.dmac_class) {
			class_destroy(dmacdev->dmac_dev.dmac_class);
		}
		kfree(dmacdev);
	}
	hal_dma_deinit();
	return 0;
}

static int symphony_dmac_suspend(struct platform_device *pdev, pm_message_t stState)
{
	struct dmac_device *dmacdev = NULL;
	int ret = 0;

	dmacdev = platform_get_drvdata(pdev);
	if (dmacdev && dmacdev->dmac_dev.clk) {
		ret = clk_set_rate(dmacdev->dmac_dev.clk, DEFAULT_DMA_FREQ);
		if (ret) {
			printk("[%s_%d]clk_set_rate error\n", __func__, __LINE__);
		}

		clk_disable_unprepare(dmacdev->dmac_dev.clk);
	}
	else {
		printk("dmacdev or clk is null!\n");
	}
	return 0;
}

static int symphony_dmac_resume(struct platform_device *pdev)
{
	struct dmac_device *dmacdev = NULL;
	int ret = 0;

	dmacdev = platform_get_drvdata(pdev);
	if (dmacdev) {
		switch(dmacdev->dmac_dev.dmaclk){
			case 262000000:
			case 411500000:
				break;
			default:
				printk("[%s_%d]warn dmaclk = %d error\n", __func__, __LINE__,dmacdev->dmac_dev.dmaclk);
				dmacdev->dmac_dev.dmaclk = 411500000;
				break;
		}
		if(dmacdev->dmac_dev.clk){

			ret = clk_prepare_enable(dmacdev->dmac_dev.clk);
			if (ret) {
				printk("[%s_%d]clk_prepare_enable error\n", __func__, __LINE__);
			}

			ret = clk_set_rate(dmacdev->dmac_dev.clk,dmacdev->dmac_dev.dmaclk);
			if (ret) {
				printk("[%s_%d]clk_set_rate error\n", __func__, __LINE__);
			}
		}
		else {
			printk("dmacdev->dmac_dev.clk is null!\n");
		}
	}
	else {
		printk("dmacdev is null!\n");
	}
	return 0;
}


#if defined(CONFIG_OF)
	 static const struct of_device_id symphony_dmac_of_match[] = {
		 { .compatible = "montage,dmac" },
		 {},
	 };
MODULE_DEVICE_TABLE(of, symphony_dmac_of_match);
#endif

static struct platform_driver symphony_dmac_driver = {
	.probe 		= symphony_dmac_probe,
	.remove		= symphony_dmac_remove,
	.suspend	= symphony_dmac_suspend,
	.resume		= symphony_dmac_resume,
	.driver	= {
		.name = UMAP_DEVNAME_DMA,
		.of_match_table = of_match_ptr(symphony_dmac_of_match),
	},
};

int __init dmac_modinit(void)
{
	int  ret = 0;

	ret = platform_driver_register(&symphony_dmac_driver);
	if (ret){
		return ret;
	}

	return 0;
}

void __exit dmac_modexit(void)
{
	platform_driver_unregister(&symphony_dmac_driver);
}

