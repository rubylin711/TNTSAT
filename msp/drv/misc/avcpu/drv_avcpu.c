/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>

#include "mt_common.h"
#include "mt_mach/symphony_regs.h"

#include "mt_drv_clock.h"

#include "drv_avcpu.h"

#if !defined(CONFIG_TEE)

#define REG_AVCPU_BOOT_INSTR_BASE				SYMPHONY_IO_VA(0xBF120000)

/* avcpu boot config start*/
static u32 avcpu_lui_opcode_26 	= 0xf;
static u32 avcpu_lui_rt_16 		= 0;
static u32 avcpu_lui_imme_0 	= 0;

/* ORI instruction */
static u32 avcpu_ori_opcode_26 	= 0xd;
static u32 avcpu_ori_rs_21 		= 0;
static u32 avcpu_ori_rt_16 		= 0;
static u32 avcpu_ori_imme_0 	= 0;

/* JR instruction */
static u32 avcpu_jr_opcode_26 	= 0;
static u32 avcpu_jr_rs_21 		= 0;
static u32 avcpu_jr_hint_6 		= 0x00;

/* avcpu physical address */
static u32 avcpu_boot_addr = 0x0;

#if defined(CONFIG_MT_MSP_MODULE)

//TODO: build as ko, not support "early_param"

#else

static int __init early_avcpu_bootaddr(char *p)
{
	//printk("early_avcpu_bootaddr1=0x%x\n",avcpu_boot_addr);

	avcpu_boot_addr = memparse(p, &p);

	//printk("early_avcpu_bootaddr=0x%x\n",avcpu_boot_addr);

	return 0;
}
early_param("avcpu_bootaddr", early_avcpu_bootaddr);

#endif

static u32 get_avcpu_boot_addr(void)
{
	//early_avcpu_bootaddr();
	u32 tmp_bootaddr = 0;

	/* convert to MIPS address */
	tmp_bootaddr = avcpu_boot_addr | 0x80000000;

	//printk("get_avcpu_boot_addr=0x%x,bootaddr:0x%x\n",avcpu_boot_addr,tmp_bootaddr);

	return tmp_bootaddr;
}

static void config_avcpu_boot_addr(u32 avboot_addr)
{
	u32 instruct = 0;
	ulong addr = REG_AVCPU_BOOT_INSTR_BASE;

	printk("\nconfig_avcpu_boot_addr start baddr=0x%x\n", avboot_addr);

	/* lui v0, higher-16-bit of addr */
	avcpu_lui_rt_16 = 2;
	avcpu_lui_imme_0 = (avboot_addr >> 16) & 0xFFFF;

	instruct = 0;
	instruct = (avcpu_lui_opcode_26 << 26) | (avcpu_lui_rt_16 << 16) | (avcpu_lui_imme_0 << 0);

	*(volatile unsigned int *)addr = instruct;
	//  printf("addr(0x%x)=0x%x\n", addr, instruct, readl(addr));

	addr += 4;

	/* ori v0, v0, lower-16-bit of addr */
	avcpu_ori_rs_21 = 2;
	avcpu_ori_rt_16 = 2;
	avcpu_ori_imme_0 = avboot_addr & 0xFFFF;

	instruct = 0;
	instruct = (avcpu_ori_opcode_26 << 26) | (avcpu_ori_rs_21 << 21) | (avcpu_ori_rt_16 << 16) | (avcpu_ori_imme_0 << 0);

	*(volatile unsigned int *)addr = instruct;
	//  printf("addr(0x%x)=0x%x\n", addr, instruct, readl(addr));

	addr += 4;

	/* jr v0 instruction */
	instruct = 0;
	avcpu_jr_rs_21 = avcpu_lui_rt_16;
	//instruct = (jr_opcode_26 << 26) | (jr_rs_21 << 21) | (31 << 11) | (jr_hint_6 << 6) | (jr_JR_0 << 0);
	instruct = (avcpu_jr_opcode_26 << 26) | (avcpu_jr_rs_21 << 21) | (0 << 11) | (avcpu_jr_hint_6 << 6) | (8 << 0);

	*(volatile unsigned int *)addr = instruct; /* nop instruction */

	//  printf("addr(0x%x)=0x%x\n", addr, instruct, readl(addr));

	/* nop instruction */
	addr += 4;
	instruct = 0;
	*(volatile unsigned int *)addr = instruct;

	printk("\nconfig_avcpu_boot_addr end\n");
	//  printf("addr(0x%x)=0x%x\n", addr, instruct, readl(addr));

	return;
}
/* avcpu boot config end*/

//---------------------------------------------------------------------------//

void misc_avcpu_resume(void)
{
	u32 ret;

	//printk("==misc_avcpu_resume start==\n");

#if 0/*if spi drv str func not ok,must do those code for spi config*/
	u32 addr = SYMPHONY_SFLASH_VIRT_BASE+0x104;

	if(REG32(addr)!=spi_register)
	{
	printk("now_val:0x%x,bk_val:0x%x,spi reg err,need config sync before suspend\n",REG32(addr),spi_register);
	REG32(addr)=spi_register;
	}
#endif

	if (avcpu_boot_addr == 0)
	{
		pr_err("AVCPU boot address is zero! pls define \"avcpu_bootaddr=\" in kernel's command-line parameters!\n");
		return;
	}

	if (mt_avcpu_get_encrypt_status())
	{
		//printk("==avcpu decrypt start==\n");
		ret = mt_avcpu_decrypt(avcpu_boot_addr);
		if(!ret)
		{
			printk("==avcpu decrypt fail,resume avcpu stop==\n");
			return;
		}
	} else {
#ifdef CONFIG_DECOMPRESS_LZMA
		ret = avcpu_decompress(avcpu_boot_addr);
		if (!ret)
		{
			printk("==avcpu decompress fail,resume avcpu stop==\n");
			return;
		}
#else
		printk("==CONFIG_DECOMPRESS_LZMA not open,avcpu decompress fail==\n");
#endif
	}

	config_avcpu_boot_addr(get_avcpu_boot_addr());
	//	printk("==avcpu boot end==\n");

	mt_clk_reset(MT_CLK_AVCPU);
	//	printk("==misc_avcpu_resume end==\n");
}

#endif

