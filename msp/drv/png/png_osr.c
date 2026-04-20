/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <asm/atomic.h>
#include <asm/bitops.h>
#include <linux/uaccess.h>

#include "mt_drv_dev.h"
#include "mt_drv_sys.h"
#include "mt_drv_struct.h"
#include "mt_drv_module.h"
#include "mt_mach/irq.h"
#include "mt_mach/symphony_regs.h"
#include "mt_osal.h"

#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_io.h"
#endif

#ifdef CONFIG_MT_FPGA_GPE
#define PNG_INT_ENABLE
#endif
#define PNGNAME "MT_PNG"
#define PNG_INTNUM  (IRQ_PNG_ID)
#ifndef VM_RESERVED // for kernel up to 3.7.0 version
#define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif
#define PNG_REG_BASE 0xBF1C1000

/* Png device reference count*/
static atomic_t g_PNGCount = ATOMIC_INIT(0);

static void png_core_clk_high(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    volatile mt_u32 value = 0;
    value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF50A704));
    value &= ~(0x3);//clear bit[1:0]
    value |= (0x1 << 0);//bit[1:0] set 1
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF50A704), value);
    //printk("png clk high\n");
#endif
}

static void png_core_clk_low(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) 

    volatile mt_u32 value = 0;
    value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf50f818));
    value |= (0x1 << 12);//bit12 set 1
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf50f818), value);
    
    value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF50A704));
    value &= ~(0x3);//clear bit[1:0]
    value |= (0x2 << 0);//bit[1:0] set 2
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF50A704), value);
    //printk("png clk low\n");
#endif
}

static void png_enable_auto_gate(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY6) 

    volatile mt_u32 value = 0;
    value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF1C1010));
    value &= ~(0x7);
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF1C1010), value);
    value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF1C1010));
//	        printk("\r\n ~~~~~~~~~%s, %d 0xBF1C1010: %x\n", __FUNCTION__, __LINE__, HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xBF1C1010)));

#endif
}


static int png_map_to_register(struct file *fp, struct vm_area_struct *vm)
{
    unsigned long pfn;

    vm_flags_set(vm, VM_IO | VM_RESERVED);
    vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);
    pfn = PNG_REG_BASE >> PAGE_SHIFT;

    return remap_pfn_range(vm, vm->vm_start, pfn, vm->vm_end - vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

static int png_mmap(struct file *fp, struct vm_area_struct *vm)
{
    int ret = 0;

    if (vm->vm_pgoff)
    {
        return 0;
    }
    else
    {
        ret = png_map_to_register(fp, vm);
        return ret;
    }
}


static int png_open(struct inode *finode, struct file *ffile)
{
    if (1 == atomic_inc_return(&g_PNGCount))
    {
        png_core_clk_high();
    }

    return 0;
}

static int png_close(struct inode *finode, struct file *ffile)
{
    atomic_dec(&g_PNGCount);
    if (atomic_read(&g_PNGCount) < 0)
    {
        atomic_set(&g_PNGCount, 0);
    }
    if(atomic_read(&g_PNGCount) == 0)
        png_core_clk_low();
    return 0;
}

#ifdef PNG_INT_ENABLE

/* interrutp function */
static int png_isr(int irq, void *dev_id)
{
    /* read register and clear the int*/
	u32 dtmp = HAL_GET_U32((volatile u32 *)(SYMPHONY_IO_VA(0xbf1c10e4)));
	printk("\r\n int state:0x%08x, state:0x%08x", 
		dtmp, HAL_GET_U32((volatile u32 *)(SYMPHONY_IO_VA(0xbf1c10e0)))); //
	HAL_PUT_U32((volatile u32 *)(SYMPHONY_IO_VA(0xbf1c10f4)),dtmp); //PNG_INT_STATE_CLR
	printk("\r\n int state after clr:0x%08x", HAL_GET_U32((volatile u32 *)(SYMPHONY_IO_VA(0xbf1c10e4)))); //PNG_INT_STATE
    return IRQ_HANDLED;
}
#endif

static mt_device_s g_PngRegisterData;

static struct file_operations g_png_fops =
{
    .owner = THIS_MODULE,
    .open = png_open,
    .unlocked_ioctl = NULL,
    .release = png_close,
    .mmap = png_mmap,
};


/* module init */
int PNG_DRV_ModInit(void)
{

    MT_S32 Ret;

    png_core_clk_low();
printk("\r\n ~~~~~~~~~%s, %d", __FUNCTION__, __LINE__);
#ifdef PNG_INT_ENABLE
    /* register interrupt function*/
    if (0 != request_irq(PNG_INTNUM, (irq_handler_t)png_isr, IRQF_PROBE_SHARED, "mt_png_irq", NULL))
    {
    	return MT_FAILURE;
    }
#endif
  Ret = mt_drv_module_register(MT_ID_PNG, "MT_PNG", (mt_void *)MT_NULL);
  printk("\r\n ~~~~~~~~~%s, %d", __FUNCTION__, __LINE__);

  if (MT_SUCCESS != Ret)
  {
      free_irq(PNG_INTNUM, NULL);
      printk("register module failed!\n");
      return MT_FAILURE;
  }

  /* register device*/
    mt_osal_snprintf(g_PngRegisterData.devfs_name, sizeof(g_PngRegisterData.devfs_name), UMAP_DEVNAME_PNG);
    g_PngRegisterData.minor  = UMAP_MIN_MINOR_PNG;
    g_PngRegisterData.owner  = THIS_MODULE;
    g_PngRegisterData.drvops = NULL;
    g_PngRegisterData.fops = &g_png_fops;

  printk("\r\n ~~~~~~~~~%s, %d", __FUNCTION__, __LINE__);
    if (mt_drv_dev_register(&g_PngRegisterData) < 0)
    {
     mt_drv_module_unregister(MT_ID_PNG);
     free_irq(PNG_INTNUM, NULL);
      printk("register tde device failed.\n");
        return MT_FAILURE;
    }
	png_enable_auto_gate();
        printk("\r\n ~~~~~~~~~%s, %d", __FUNCTION__, __LINE__);
    return 0;

}

/* module deinit */

void PNG_DRV_ModExit(void)
{
    /* logoout device */
    mt_drv_dev_unregister(&g_PngRegisterData);
    mt_drv_module_unregister(MT_ID_PNG);

#ifdef PNG_INT_ENABLE

    /* release interrupt num*/
    free_irq(PNG_INTNUM, NULL);
#endif

    return;
}



/** 这两个函数要按此命名 **/
#ifdef MODULE
//module_init(PNG_DRV_ModInit);
//module_exit(PNG_DRV_ModExit);
#endif

#ifndef MT_ADVCA_FUNCTION_RELEASE
MODULE_DESCRIPTION("driver for the all png");
MODULE_AUTHOR("MONTAGE");
MODULE_LICENSE("GPL");
MODULE_VERSION("V1.0.0.0");
#else
MODULE_DESCRIPTION("");
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_VERSION("");
#endif
