/*
 * Arasan MMC/SD/SDIO driver
 *
 *  This is the driver for the Arasan MMC/SD/SDIO host controller
 *  integrated in the STMicroelectronics platforms
 *
 * Author: Giuseppe Cavallaro <peppe.cavallaro@xxxxxx>
 * Copyright (C) 2010 STMicroelectronics Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/mbus.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/irq.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/mmc/host.h>
#include <linux/mmc/arasan_plat.h>

#include <asm/sizes.h>
#include <asm/unaligned.h>

#include <mach/hardware.h>

#include "arasan.h"

//#define STOP_CMD_and_CLK		//
static int value = 0x54 ;
module_param(value,int,S_IRUSR);

static struct resource arasan2_resource[] = {
	[0] = {
		.name  = "sdio2_addr",
		.start = SDIO2_BASE,
		.end   = SDIO2_BASE + 0x10000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "sdio2_irq",
		.start = SDIO2_IRQ,
		.end   = SDIO2_IRQ,
		.flags = IORESOURCE_IRQ,
	}

};

struct resource *port0_r;
unsigned int port0_addr;

static void arasan_release(struct device *device)
{
	return;
}

#ifndef GPIO_BASE
	#define GPIO_BASE (0x10020000)
#endif
#ifndef GPIO6_MULT_USE_EN
	#define GPIO6_MULT_USE_EN (0x18)
#endif



void gpio6_reuse_sdio2(void)
{
	unsigned int gpio_6_reuse,gpio_6_reuse_value;
	void __iomem *gpio_6_resue_addr = NULL;
	int ret = 0; 

	gpio_6_reuse = GPIO_BASE+GPIO6_MULT_USE_EN;
	if (!request_mem_region(gpio_6_reuse,4,"gpio6_sdio2")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)gpio_6_reuse);
		return;
	}
	gpio_6_resue_addr = ioremap(gpio_6_reuse,4);
	if (!gpio_6_resue_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}
	/* 分配给sdio2 */
	gpio_6_reuse_value = readl(gpio_6_resue_addr);
	gpio_6_reuse_value |= (1<<0 | 1<<1 | 1<<2 | 1<<3);
	writel(gpio_6_reuse_value, gpio_6_resue_addr);
	gpio_6_reuse_value = readl(gpio_6_resue_addr);
	//printk("value=%02x\n", gpio_6_reuse_value);

	iounmap(gpio_6_resue_addr);

out_release_region:
	release_mem_region(gpio_6_reuse, 4);
	return;

}

void gpio6_46_outlow(void)
{
	unsigned int gpio_6,gpio_6_value;
	void __iomem *gpio_6_addr = NULL;
	int ret = 0; 

	gpio_6 = GPIO_BASE+0xA0;
	if (!request_mem_region(gpio_6,12,"gpio6")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)gpio_6);
		return;
	}
	gpio_6_addr = ioremap(gpio_6,12);
	if (!gpio_6_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}

	/* gpio_6_4, gpio_6_6输出使能 */
	gpio_6_value = readl(gpio_6_addr+4);
	gpio_6_value |= 0x5<<4;
	writel(gpio_6_value, gpio_6_addr+4);

	/* gpio_6_4低, gpio_6_6输出高 */
	gpio_6_value = readl(gpio_6_addr);
	//gpio_6_value &= ~(0x5<<4);
	gpio_6_value &= ~(0x1<<6);
	//gpio_6_value &= ~(0x1<<4);
	writel(gpio_6_value, gpio_6_addr);
    mdelay(100);
	gpio_6_value |= 0x1<<6;
	writel(gpio_6_value, gpio_6_addr);
    mdelay(100);

	iounmap(gpio_6_addr);

out_release_region:
	release_mem_region(gpio_6, 12);
	return;
}
void gpio5_6_input(void)
{
	unsigned int gpio_5,gpio_5_value;
	void __iomem *gpio_5_addr = NULL;
	int ret = 0; 

	gpio_5 = GPIO_BASE+0x90;
	if (!request_mem_region(gpio_5,12,"gpio5")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)gpio_5);
		return;
	}
	gpio_5_addr = ioremap(gpio_5,12);
	if (!gpio_5_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}

	/* gpio_5_6输入使能 */
    gpio_5_value = 0;
	gpio_5_value = readl(gpio_5_addr+4);
	gpio_5_value &= ~(0x1<<6);
	writel(gpio_5_value, gpio_5_addr+4);

	iounmap(gpio_5_addr);

out_release_region:
	release_mem_region(gpio_5, 12);
	return;
}

int wifi_powerdown(unsigned int value)
{
	unsigned int PortIndex = (value&0xF0)>>4; 
	unsigned int BitIndex = value&0x0F;
	unsigned int gpio_dir_value = 0;
	unsigned int gpio_value = 0;
	unsigned int gpio_bitdata_value = 0;

	//printk("PortIndex = %x BitIndex = %x\n",PortIndex,BitIndex);
	
	if(PortIndex > 6)
	{
		printk("error : PortIndex = %d\n",PortIndex);
		return -1;
	}
	//复用
	gpio_dir_value = readl(IO_ADDRESS(GPIO_BASE + PortIndex*0x04));
	//printk("@@@@gpio_dir_value = %x@@@\n",gpio_dir_value);
	gpio_dir_value &= ~(1 << BitIndex | 1 << (8 +BitIndex) |1 << (16 + BitIndex));
	//printk("!!!!gpio_dir_value = %x!!! \n",gpio_dir_value);
	writel(gpio_dir_value,IO_ADDRESS(GPIO_BASE + PortIndex*0x04));
	//使能
	gpio_bitdata_value = readl(IO_ADDRESS(GPIO_BASE + 0x44 +PortIndex*0x10));
	gpio_bitdata_value |=(1 << BitIndex); 
	writel(gpio_bitdata_value,IO_ADDRESS(GPIO_BASE + 0x44 +PortIndex*0x10));
	
//	gpio_value = readl(IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));
//	gpio_value |=~(1 << BitIndex);
//	writel(gpio_value,IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));

	gpio_value = readl(IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));
	gpio_value &=~(1 << BitIndex);
	writel(gpio_value,IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));
	
	return 0;
}

int wifi_powerup(unsigned int value)
{
	unsigned int PortIndex = (value&0xF0)>>4; 
	unsigned int BitIndex = value&0x0F;
	unsigned int gpio_dir_value = 0;
	unsigned int gpio_value = 0;
	unsigned int gpio_bitdata_value = 0;

	//printk("PortIndex = %x BitIndex = %x\n",PortIndex,BitIndex);
	
	if(PortIndex > 6)
	{
		printk("error : PortIndex = %d\n",PortIndex);
		return -1;
	}
	//复用
	gpio_dir_value = readl(IO_ADDRESS(GPIO_BASE + PortIndex*0x04));
	//printk("@@@@gpio_dir_value = %x@@@\n",gpio_dir_value);
	gpio_dir_value &= ~(1 << BitIndex | 1 << (8 +BitIndex) |1 << (16 + BitIndex));
	//printk("!!!!gpio_dir_value = %x!!! \n",gpio_dir_value);
	writel(gpio_dir_value,IO_ADDRESS(GPIO_BASE + PortIndex*0x04));
	//使能
	gpio_bitdata_value = readl(IO_ADDRESS(GPIO_BASE + 0x44 +PortIndex*0x10));
	gpio_bitdata_value |=(1 << BitIndex); 
	writel(gpio_bitdata_value,IO_ADDRESS(GPIO_BASE + 0x44 +PortIndex*0x10));
	//配置//先低后高
	gpio_value = readl(IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));

	if(gpio_value &(0x1<<BitIndex)){
		pr_err("PDN pin is High \n");
		gpio_value &= ~(0x1<<BitIndex);
		writel(gpio_value,IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));
		pr_err("ouput low and delay 7 sec　for softreset @ -10 degree \n");
		schedule_timeout_interruptible(msecs_to_jiffies(7 * 1000));
	}else{
		pr_err("Uboot already keep PDN pin low :delay 2 sec.\n");
		schedule_timeout_interruptible(msecs_to_jiffies(2 * 1000));
	}
#if 0
	gpio_value &=~(1 << BitIndex);
	writel(gpio_value,IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));
	pr_err("ouput low and delay 7000ms\n");
	mdelay(7000);
#endif
	gpio_value = readl(IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));
	gpio_value |=(1 << BitIndex);
	writel(gpio_value,IO_ADDRESS(GPIO_BASE + 0x40 +PortIndex*0x10 ));
	pr_err("ouput high and delay 2000ms\n");
	schedule_timeout_interruptible(msecs_to_jiffies(2 * 1000));
	
	return 0;
}
#if 0
void wifi_powerdown(void)
{
	unsigned int gpio_5,gpio_5_value;
	void __iomem *gpio_5_addr = NULL;
	int ret = 0; 

	gpio_5 = GPIO_BASE+0x90;
	if (!request_mem_region(gpio_5,12,"gpio5")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)gpio_5);
		return;
	}
	gpio_5_addr = ioremap(gpio_5,12);
	if (!gpio_5_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}
    //pr_err("start delay 500ms\n");
    //mdelay(500);

	/* gpio_5_4输出使能 */
	gpio_5_value = readl(gpio_5_addr+4);
	gpio_5_value |= (0x1<<4);
	writel(gpio_5_value, gpio_5_addr+4);
    //pr_err("start delay 500ms\n");
    //mdelay(500);

    /* 先低后高 */
	gpio_5_value = readl(gpio_5_addr);
	gpio_5_value &= ~(0x1<<4);
	writel(gpio_5_value, gpio_5_addr);
	pr_err("ouput low\n");

	iounmap(gpio_5_addr);

out_release_region:
	release_mem_region(gpio_5, 12);
	return;
}
#endif

#if 0
void wifi_powerup(void)
{
	unsigned int gpio_5,gpio_5_value;
	void __iomem *gpio_5_addr = NULL;
	int ret = 0; 

	gpio_5 = GPIO_BASE+0x90;
	if (!request_mem_region(gpio_5,12,"gpio5")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)gpio_5);
		return;
	}
	gpio_5_addr = ioremap(gpio_5,12);
	if (!gpio_5_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}
    //pr_err("start delay 500ms\n");
    //mdelay(500);

	/* gpio_5_4输出使能 */
	gpio_5_value = readl(gpio_5_addr+4);
	gpio_5_value |= (0x1<<4);
	writel(gpio_5_value, gpio_5_addr+4);
    //pr_err("start delay 500ms\n");
    //mdelay(500);

    /* 先低后高 */
	gpio_5_value = readl(gpio_5_addr);
	if(gpio_5_value &(0x1<<4)){
		pr_err("PDN pin is High \n");
		gpio_5_value &= ~(0x1<<4);
		writel(gpio_5_value, gpio_5_addr);
		pr_err("ouput low and delay 7 sec　for softreset @ -10 degree \n");
		mdelay(7*1000);
	}else{
		pr_err("Uboot already keep PDN pin low : Skip 7 sec.\n");
	}


	gpio_5_value = readl(gpio_5_addr);
	gpio_5_value |= 0x1<<4;
	writel(gpio_5_value, gpio_5_addr);
	pr_err("output high and delay 2000ms\n");
	mdelay(2000);


	iounmap(gpio_5_addr);

out_release_region:
	release_mem_region(gpio_5, 12);
	return;
}
#endif
/* GPIO复用到sdio2 */

void gpio6_ouput_low(void)
{
	unsigned int gpio_6,gpio_6_value;
	void __iomem *gpio_6_addr = NULL;
	int ret = 0; 

	gpio_6 = GPIO_BASE+0xA0;
	if (!request_mem_region(gpio_6,12,"gpio6")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)gpio_6);
		return;
	}
	gpio_6_addr = ioremap(gpio_6,12);
	if (!gpio_6_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}
	gpio_6_value = readl(gpio_6_addr+4);
	gpio_6_value |= (0x1<<0|0x01<<1|0x01<<2|0x01<<3);
	writel(gpio_6_value, gpio_6_addr+4);

	gpio_6_value = readl(gpio_6_addr);
	gpio_6_value  &= ~(0x1<<0|0x01<<1|0x01<<2|0x01<<3);
	writel(gpio_6_value, gpio_6_addr);
	
	mdelay(100);
	iounmap(gpio_6_addr);

out_release_region:
	release_mem_region(gpio_6, 12);
	return;
}

#ifdef STOP_CMD_and_CLK
void ctrl_sdio2_clk(int ctrl)
{
	unsigned int sdio_clk,sdio_clk_value;
	void __iomem *clk_value_addr = NULL;
	int ret = 0; 

	sdio_clk = 0x5001002c;
	if (!request_mem_region(sdio_clk,4,"sdio_clk")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)sdio_clk);
		return;
	}

	clk_value_addr = ioremap(sdio_clk,4);
	if (!clk_value_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}

	//mdelay(10000);
	sdio_clk_value = readl(clk_value_addr);
	printk("clk_value %x\n",sdio_clk_value);
	if(!ctrl){
		sdio_clk_value &= ~(1<<2);
	}else{	
		sdio_clk_value |= (1<<2);
	}
		writel(sdio_clk_value, clk_value_addr);
	printk("clk_value %x\n",sdio_clk_value);

	iounmap(clk_value_addr);

out_release_region:
	release_mem_region(sdio_clk, 4);
	return;
}
	
void ctrl_sdio2_cmd(void)
{
	unsigned int sdio_cmd;
	unsigned volatile int sdio_cmd_value;
	void __iomem *cmd_value_addr = NULL;
	int ret = 0; 
	struct timeval tv1;
	struct timeval tv2;
	int count = 1<<24;
	
	sdio_cmd = 0x5001002c;
	if (!request_mem_region(sdio_cmd,4,"sdio_cmd")) {
		pr_err("%s: ERROR: memory allocation failed"
				"cannot get the I/O addr 0x%x\n",
				__func__, (unsigned int)sdio_cmd);
		return;
	}

	cmd_value_addr = ioremap(sdio_cmd,4);
	if (!cmd_value_addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}
//	do_gettimeofday(&tv1);
//	do_gettimeofday(&tv2);
//	while((tv2.tv_sec-tv1.tv_sec)<2)
	while(count--)
	{
//		do_gettimeofday(&tv2);
		sdio_cmd_value = readl(cmd_value_addr);
//		printk("cmd_value %x\n",sdio_cmd_value);
		sdio_cmd_value |= (1<<25);
		writel(sdio_cmd_value, cmd_value_addr);
//		printk("cmd_value %x\n",sdio_cmd_value);
	}
	iounmap(cmd_value_addr);

out_release_region:
	release_mem_region(sdio_cmd, 4);
	return;
}

#endif
/* GPIO复用到sdio2 */
void arasan2_powerup(void)
{
//	wifi_powerdown(value);
#ifdef STOP_CMD_and_CLK
	ctrl_sdio2_clk(0);
	ctrl_sdio2_cmd();
#endif
	gpio6_ouput_low();
	mdelay(100);
	wifi_powerup(value);
#ifdef STOP_CMD_and_CLK
	ctrl_sdio2_clk(1);
#endif
	mdelay(100);
	gpio6_reuse_sdio2();
}

struct arasan_platform_data arasan2_platform_data = {
	.need_poll = 0,
	.need_detect = 0,
	.use_pio = 0,
	.auto_cmd12 = 0,
	.card_irq = 1,
	.sdio_4bit_data = 0,

	.p_powerup = arasan2_powerup,
};
/* sdio 2 */
static struct platform_device arasan2_device = {
	//.id = -1,
	.name = ARASAN_DRIVER_NAME,
	.num_resources = ARRAY_SIZE(arasan2_resource),
	.resource = arasan2_resource,
	.dev = {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &arasan2_platform_data,
		.release = arasan_release,
	}
};

static int __init arasan2_init(void)
{
	platform_device_register(&arasan2_device);
	return 0;
}

static void __exit arasan2_exit(void)
{
	printk("arasan2_exit.\n");
	platform_device_unregister(&arasan2_device);
}

module_init(arasan2_init);
module_exit(arasan2_exit);

MODULE_LICENSE("GPL");
