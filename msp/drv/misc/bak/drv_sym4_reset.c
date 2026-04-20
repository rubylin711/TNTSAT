/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "mt_common.h"

#include "drv_sys_misc.h"

#include "mt_mach/clock.h"

#include "drv_sym4_reset.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"

extern int otp_open(struct inode * inode, struct file *file);
extern int sys_otp_read(u32 bit_addr, u8 len, u32 *p_result);
#ifndef REG32
#define REG32(addr) (*(volatile unsigned int *)(addr))
#endif
/* avcpu boot config start*/
static u32 avcpu_lui_opcode_26 = 0xf;
static u32 avcpu_lui_rt_16 = 0;
static u32 avcpu_lui_imme_0 = 0;

/* ORI instruction */
static u32 avcpu_ori_opcode_26 = 0xd;
static u32 avcpu_ori_rs_21 = 0;
static u32 avcpu_ori_rt_16 = 0;
static u32 avcpu_ori_imme_0 = 0;

/* JR instruction */
static u32 avcpu_jr_opcode_26 = 0;
static u32 avcpu_jr_rs_21 = 0;
//unsigned int jr_hint_6 = 0x10;
static u32 avcpu_jr_hint_6 = 0x00;

//#if defined(CONFIG_MT_DDR_SIZE_512)
//static u32 avcpu_boot_addr=0x9de10000;
//#else
static u32 avcpu_boot_addr=0x0;
//#endif

//static u32 spi_register=0x0;
extern u32 avcpu_decompress(u32 ddr_addr);
extern MT_BOOL mt_avcpu_get_encry_status(void);
extern u32 mt_avcpu_decrypt(u32 ddr_addr);

static int __init crm_early_avcpu_bootaddr(char *p)
{
	//printk("early_avcpu_bootaddr1=0x%x\n",avcpu_boot_addr);
	avcpu_boot_addr = memparse(p, &p);
	//printk("early_avcpu_bootaddr=0x%x\n",avcpu_boot_addr);
	return 0;
}
early_param("avcpu_bootaddr", crm_early_avcpu_bootaddr);

static u32 crm_get_avcpu_boot_addr(void)
{
	//early_avcpu_bootaddr();
	u32 tmp_bootaddr=0;
	tmp_bootaddr=avcpu_boot_addr|0x80000000;
	//printk("get_avcpu_boot_addr=0x%x,bootaddr:0x%x\n",avcpu_boot_addr,tmp_bootaddr);
	return tmp_bootaddr;
}

static void crm_config_avcpu_boot_addr(u32 avboot_addr)
{
  u32 instruct = 0;
  u32 addr = SYMPHONY_IO_VA(0xbf120000);

  printk("\nconfig_avcpu_boot_addr start baddr=0x%x\n",avboot_addr);

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
static unsigned long crm_get_u32(u32 reg)
{
    u32 val = 0;
    val = readl((volatile void __iomem *)reg);
    return val;
}
static void crm_set_u32(u32 reg, u32 val)
{
    return writel(val, (volatile void __iomem *)reg);
}

static int crm_reg_set_valid_bit(u32 reg, u8 sbit, u8 size, u32 val)
{
    u32 tmp = 0, vbit = 0, cur_val = 0;

    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = readl((volatile void __iomem *)reg);
    cur_val = (tmp >> sbit) & vbit; //get old value from register
    if(val == cur_val)
        return 0;
    tmp &= ~(vbit << sbit);
    tmp |= (val << sbit);
    writel(tmp, (volatile void __iomem *)reg);

    return 0;
}

static unsigned long crm_reg_get_valid_bit(unsigned int reg, unsigned int sbit, unsigned int size, unsigned long *val)
{
    unsigned long tmp = 0, vbit = 0;

    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = readl((volatile void __iomem *)reg);
    *val = (tmp >> sbit) & vbit; //get old value from register
    return 0;
}
//Unified CRM sub module reset interface
static void crm_reset(unsigned int addr, unsigned int bit_mask)
{
	u32 val = 0;

	val = crm_get_u32(addr);
	val &= ~(bit_mask);
	crm_set_u32(addr, val);
      udelay(100);
	//TODO: need read again?
	val = crm_get_u32(addr);
	val |= (bit_mask);
	crm_set_u32(addr, val);
}

//Unified AO_CRM sub module reset interface
static void crm_ao_reset(unsigned int addr)
{
	crm_set_u32(addr, (u32)BIT_AO_RESET);
	crm_set_u32(addr, (u32)BIT_AO_RESET_RELEASE);
}

static MT_BOOL crm_feature_is_locked(mt_u32 reg, mt_u32 mask)
{
    if ((crm_get_u32(reg) & mask) != 0)
        return MT_TRUE;
    return MT_FALSE;
}
static int crm_xtal_clk_get(void)
{
    unsigned long val = 0, clk = 0;

    crm_reg_get_valid_bit(R_CHIP_BOOT_CFG, 30, 2, &val);
    if(val == 0)
        clk = 27;
    else if(val == 1)
        clk = 24;
    else
        clk = 40;
    return clk;
}
//mac module
static MT_BOOL crm_mac_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(MAC_SLOCK_REG, mask)
        ||crm_feature_is_locked(MAC_LOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}
static int crm_mac_clk_enabled(void)
{
    return ((crm_get_u32(MAC_CLKEN_REG) >> 4) & 0x1);
}
static MT_BOOL crm_mac_reset(void)
{
    if(crm_mac_locked(BIT_MAC_RST_LOCK))
        return 0;
    crm_reset(MAC_SRSTN_REG, BIT_MAC_RST_MASK);
    return 1;
}

static MT_BOOL crm_mac_clk_en(MT_BOOL onoff)
{
    if(crm_mac_locked(BIT_MAC_CLK_EN_LOCK))
        return 0;

    crm_reg_set_valid_bit(MAC_CLKSEL_REG, 4, 1, onoff);
    return 1;
}

static MT_BOOL crm_rmii_clk_en(MT_BOOL onoff)
{
    if(crm_mac_locked(BIT_RMII_CLK_EN_LOCK))
        return 0;

    crm_reg_set_valid_bit(MAC_CLKSEL_REG, 5, 1, onoff);
    return 1;
}
static int crm_rmii_clk_enabled(void)
{
    return ((crm_get_u32(MAC_CLKEN_REG) >> 5) & 0x1);
}

static MT_BOOL crm_mac_clk_sel(mac_rmii_clk_mode_e type)
{
    if(crm_mac_locked(BIT_MAC_CLK_SEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(MAC_CLKSEL_REG, 0, 2, type);
    return 1;
}
#if 0 //only support rmii
static MT_BOOL crm_mac_clk_mode(mac_clk_mode_e type)
{
    if(crm_mac_locked(BIT_MAC_CLK_MODE_LOCK))
        return 0;
    crm_reg_set_valid_bit(MAC_CLKSEL_REG, 8, 1, type);
    return 1;
}
#endif
static u32 crm_mac_clk_get(u32 m_id)
{
    u32 freq = 0;
    unsigned long val  = 0;

    crm_reg_get_valid_bit(MAC_CLKSEL_REG, 8, 1, &val);
    if(val)  //rmii
         freq = 50;
    else
         freq = 25;
    return freq;
}

//bus
static MT_BOOL crm_bus_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(BUS_SLOCK_REG, mask)
        ||crm_feature_is_locked(BUS_LOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_bus_axi_reg_clk_en(MT_BOOL onoff)
{
     if(crm_bus_locked(BIT_BUS_AXIREG_CLKEN_LOCK))
        return 0;
     crm_reg_set_valid_bit(BUS_CLKEN_REG, 2, 1, onoff);
     return 1;
}
static int crm_bus_axi_reg_clk_enabled(void)
{
     return (crm_get_u32(BUS_CLKEN_REG) >> 2) & 0x1;
}

static int crm_bus_axi_debug_clk_en(MT_BOOL onoff)
{
     if(crm_bus_locked(BIT_BUS_AXIDBG_CLKEN_LOCK))
        return 0;
     crm_reg_set_valid_bit(BUS_CLKEN_REG, 1, 1, onoff);
     return 1;
}

static int crm_bus_axi_debug_clk_enabled(void)
{
     return (crm_get_u32(BUS_CLKEN_REG) >> 1) & 0x1;
}

static int crm_bus_axi_clk_en(MT_BOOL onoff)
{
     if(crm_bus_locked(BIT_BUS_AXI_CLKEN_LOCK))
        return 0;
     crm_reg_set_valid_bit(BUS_CLKEN_REG, 0, 1, onoff);
     return 1;
}
static int crm_bus_axi_clk_enabled(void)
{
     return (crm_get_u32(BUS_CLKEN_REG) >> 0) & 0x1;
}

int crm_bus_axi_clk_sel(bus_axi_clk_mode_e type,axi_clk_sel_e clk_or_ddr)
{
    if(crm_bus_locked(BIT_BUS_AXICLK_SEL_LOCK))
        return 0;
    if(type== BUS_AXI_CLOCK_1){
       crm_reg_set_valid_bit(BUS_CLKSEL_REG, 11, 3, clk_or_ddr & 0x7);
       crm_reg_set_valid_bit(BUS_CLKSEL_REG, 14, 1, 1);
    }
   else {
       crm_reg_set_valid_bit(BUS_CLKSEL_REG, 8, 3, clk_or_ddr);
       crm_reg_set_valid_bit(BUS_CLKSEL_REG, 14, 1, 0);
   }
   return 1;
}

int crm_bus_apb_clk_sel(apb_clk_sel_e type )
{
    if(crm_bus_locked(BIT_BUS_APBCLK_SEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL0_REG, 16, 4, 0xf);
    crm_reg_set_valid_bit(BUS_CLKSEL_REG, 6, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL0_REG, 16, 4, 0x1 << type);
    return 1;
}

static int crm_bus_apb_clk_get(void)
{
    unsigned long freq = 0;

    crm_reg_get_valid_bit(BUS_CLKSEL_REG, 6, 2, &freq);
    switch(freq)
    {
        case APB_CLK_80M:
          freq = 80;
          break;
        case APB_CLK_64M:
          freq = 64;
          break;
        case APB_CLK_90M:
          freq = 90;
          break;
        case APB_CLK_45M:
          freq = 45;
          break;
         default:
          printk(KERN_WARNING "%s:invalid clock config.\n", __func__);
    }
    return freq;
}

int crm_bus_ahb_clk_sel(ahb_clk_sel_e type )
{
    if(crm_bus_locked(BIT_BUS_AHBCLK_SEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL0_REG, 8, 6, 0x3f);
    crm_reg_set_valid_bit(BUS_CLKSEL_REG, 3, 3, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL0_REG, 8, 6, 0x1 << type);
    return 1;
}

static int crm_bus_ahb_clk_get(void)
{
    unsigned long val = 0, freq = 0;

    crm_reg_get_valid_bit(BUS_CLKSEL_REG, 3, 3, &val);
    switch(val)
    {
      case AHB_CLK_206M:
          freq = 206;
          break;
      case AHB_CLK_144M:
          freq = 144;
          break;
      case AHB_CLK_90M:
          freq = 90;
          break;
      case AHB_CLK_60M:
          freq = 60;
          break;
      case AHB_CLK_30M:
          freq = 30;
          break;
      default:
          freq = 240;
          printk(KERN_WARNING "%s:invalid clock config.\n", __func__);
    }
    return freq;

}

static int crm_bus_get_sdram_info(void)
{
    unsigned int ddr_info = 0;
    void __iomem * reg = 0;
    int phy_ver = 0, freq = 0, size = 0;

    reg =  ioremap(0xbf090000, 8);
    ddr_info = crm_get_u32((unsigned int)reg);
    iounmap(reg);

    phy_ver = ddr_info & 0xff;
    switch((ddr_info >> 8) & 0xf)
    {
        /* 0 DDR3_1066 (533Mhz)*/
        case 0:
            freq = 1066;
            break;
        /* 1 DDR3_1333 (666Mhz)*/
        case 1:
            freq = 1333;
            break;
        /* 2 DDR3_1600 (800Mhz)*/
        case 2:
            freq = 1600;
            break;
        /* 3 DDR3_1600K(800Mhz)*/
        case 3:
            freq = 1600K;
            break;
        /* 4 DDR3_1866 (933Mhz)*/
        case 4:
            freq = 1866;
            break;
        /* 5 DDR3_2040 (1020Mhz)*/
        case 5:
            freq = 2040;
            break;
        /* 6 DDR3_800  (400Mhz)*/
        case 6:
            freq = 2080;
            break;
        /* 7 DDR3_2133 (1066Mhz)*/
        case 7:
            freq = 2133;
            break;
        /* 8 DDR3_2666 (1333Mhz)*/
        case 8:
            freq = 2666;
            break;
        default:
            freq = 0;
        break;
    }

    switch((ddr_info >> 13) & 0x7)
    {
        /* 1 64MB */
        case 1:
            size = 64;
            break;
       /* 2 128MB */
        case 2:
            size = 128;
            break;
        /* 3 256MB */
        case 3:
            size = 256;
            break;
        /* 4 512MB */
        case 4:
            size = 512;
            break;
        /* 5 1024MB */
        case 5:
            size = 1024;
            break;
        /* 6 2048MB */
        case 6:
            size = 2048;
            break;
        /* 7 4096MB */
        case 7:
            size = 4096;
            break;
        default:
            size = 128;
             break;
    }
    printk(KERN_INFO "DDR3 PHY VER:0x%x, %dMHz %s, %dMBytes \n", phy_ver, freq, (ddr_info >> 12 & 0x1)?"SIP":"NOSIP", size);

    return freq;
}
static int crm_bus_axi_clk_get(void)
{
    u32 val = 0, freq = 0, phy_clk = 0;;


    val = crm_get_u32(BUS_CLKSEL_REG);
    if((val >> POS_AXI_CLK_SRC_SEL) & 0x1)
    {
        val = (val >> 11) & 0x7;
        val += AXI_CLK_RESV; //redirect to macro of AXI1
     }
    else
    {
        val = (val >> 8) & 0x7;
        phy_clk = crm_bus_get_sdram_info() >> 1; //get ddrphy clock
    }
    switch(val)
    {
          case AXI_CLK_DPHY_DIV_2:
              freq = phy_clk/2;
              break;
          case AXI_CLK_DPHY_DIV_2D5:
              freq = phy_clk*2/5;
              break;
          case AXI_CLK_DPHY_DIV_2D75:
              freq = phy_clk * 100/275;
              break;
          case AXI_CLK_DPHY_DIV_3:
              freq = phy_clk/3;
              break;
          case AXI_CLK_DPHY_DIV_3D5:
              freq = phy_clk * 2 /7;
              break;
          case AXI_CLK_DPHY_DIV_8:
              freq = phy_clk/8;
              break;
          case AXI_CLK_240M:
              freq = 240;
              break;
          case AXI_CLK_120M:
              freq = 120;
              break;
          case AXI_CLK_60M:
              freq = 60;
              break;
          case AXI_CLK_30M:
              freq = 30;
              break;
           case AXI_CLK_444M:
              freq = 444;
              break;
          default:
              printk(KERN_WARNING "%s:invalid clock config.\n", __func__);
              break;
    }

    return freq;
}


static MT_BOOL crm_avcpu_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(AVCPU_LOCK_REG, mask)
        ||crm_feature_is_locked(AVCPU_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_avcpu_clken(MT_BOOL onoff)
{
    if(crm_avcpu_locked(BIT_APCPU_CLKEN_LOCK))
        return 0;

    crm_reg_set_valid_bit(AVCPU_CLKEN_REG, 0, 1, onoff);
    return 1;
}

static int crm_avcpu_clk_enabled(void)
{
    return crm_get_u32(AVCPU_CLKEN_REG) & 0x1;
}
static int crm_avcpu_clk_get(void)
{
    unsigned long  val= 0, freq = 0;

    crm_reg_get_valid_bit(AVCPU_CLKSEL_REG, 0, 3, &val);
    switch(val)
    {
      case AVCPU_CLKSEL_667M:
          freq = 667;
          break;
      case AVCPU_CLKSEL_576M:
          freq = 576;
          break;
      case AVCPU_CLKSEL_480M:
          freq = 480;
          break;
      case AVCPU_CLKSEL_360M:
          freq = 360;
          break;
      case AVCPU_CLKSEL_288M:
          freq = 288;
          break;
      case AVCPU_CLKSEL_180M:
          freq = 180;
          break;
      case AVCPU_CLKSEL_90M:
          freq = 90;
          break;
    }
    return freq;
}

static int crm_avcpu_clk_sel(avcpu_clksel_e type)
{
    unsigned long  cur_clk = 0;
    int idx = 0, dir = 0;;

    if(crm_avcpu_locked(BIT_AVCPU_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 0, 8, 0xff);
    crm_reg_get_valid_bit(AVCPU_CLKSEL_REG, 0, 3, &cur_clk);
    if(cur_clk > type)
        dir = -1;
    else
        dir = 1;
    idx = cur_clk;
    while(idx != type) //clock switch step by step
    {
        idx += dir;
        crm_reg_set_valid_bit(AVCPU_CLKSEL_REG, 0, 3, idx);
    }
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 0, 8, 0x1 << type);
    return 1;
}

static int crm_avcpu_clk_reset(avcpu_reset_e type)
{
    if(crm_avcpu_locked(BIT_AVCPU_LOCK_DEC_RSTN+type))
        return 0;
    if((crm_get_u32(AVCPU_CLKEN_REG) & 0x1) == 0)
        return 0;
    crm_reset(AVCPU_SRSTN_REG, (BIT_AVCPU_RSTN_EN<<type));

    return 1;
}

int crm_avcpu_clk_switch_mask( int type)
{
    crm_reg_set_valid_bit(AVCPU_CLKEN_REG, 3, 1, type);
    return 1;
}




static MT_BOOL crm_sys_locked(mt_u32 offset)
{
    if(crm_feature_is_locked(SYS_LOCK_REG, 0x1<< offset)
        ||crm_feature_is_locked(SYS_SLOCK_REG, 0x1<< offset))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_sys_clken(sys_clken_e type,MT_BOOL onoff)
{
    if(crm_sys_locked(type))
        return 0;

    crm_reg_set_valid_bit(SYS_CLKEN_REG, type, 1, onoff);
    return 1;
}

static int crm_sys_dma_clksel(sys_dma_clksel_e type)
{
    if(crm_sys_locked(BIT_SYS_DMA_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL4_REG, 12, 4, 0xf);
    if(type == DMA_CLKESEL_XTAL || type == DMA_CLKESEL_131M)
        crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 6, 1, DMA_CLKESEL_XTAL == type?1:0);
    crm_reg_set_valid_bit(SYS_CLKSEL_REG, 0, 2, type & 0x3);
    crm_reg_set_valid_bit(TOPCLK_CTRL4_REG, 12, 4, 0x1 << type);
    return 1;
}

static int crm_sys_dma_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(SYS_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
      case DMA_CLKESEL_262M:
          freq = 262;
          break;
      case DMA_CLKESEL_288M:
          freq = 288;
          break;
      case DMA_CLKESEL_131M:
          if((crm_get_u32(TOPCLK_CTRL6_REG) >> 6) & 0x1)
              freq = crm_xtal_clk_get();
          else
              freq = 131;
          break;
      case DMA_CLKESEL_206M:
          freq = 206;
          break;
    }
    return freq;
}
static int crm_sys_reset(sys_rstn_sel_e type)
{
    if(crm_sys_locked(BIT_SYS_LOCK_DEC_RSTN+type))
        return 0;

    crm_reset(SYS_SRSTN_REG, (BIT_SYS_SRST_EN<<type));

    return 1;
}


static MT_BOOL crm_intf_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(INTF_SLOCK_REG, mask)
        ||crm_feature_is_locked(INTF_LOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static MT_BOOL crm_intf_locked_rstreg(mt_u32 mask)
{
    if(crm_feature_is_locked(INTF_LOCK_RSTREG, mask)
        ||crm_feature_is_locked(INTF_SLOCK_RSTREG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_intf_clken(intf_clken_e type , intf_clken_status_e status)
{
    if(crm_intf_locked(type))
        return 0;
    crm_reg_set_valid_bit(INTF_CLKEN_REG, type, 1, status);
    return 1;
}


static int crm_intf_smc_phyclksel(intf_smc_phyclksel_e type)
{
    if(crm_intf_locked(BIT_INTF_SMC_PHYCLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 8, 4, 0x3);
    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 0, 1, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 8, 4, 0x1 << type);
    return 1;
}
static int crm_intf_smc_phyclk_get(void)
{
    unsigned long  val= 0, freq = 0;

    val = crm_get_u32(INTF_CLKSEL_REG);
    if(val & 0x1) //90M
    {
        freq = crm_xtal_clk_get() /2;
    }
    else
        freq = 90;
    return freq;
}

static int crm_intf_uart1phy_clksel(intf_uartphy_clksel_e type)
{
    if(crm_intf_locked(BIT_INTF_UART1PHY_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 10, 1, type);
    return 1;
}

static int crm_intf_uart0phy_clksel(intf_uartphy_clksel_e type)
{
    if(crm_intf_locked(BIT_INTF_UART0PHY_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 9, 1, type);

    return 1;
}

static int crm_intf_uart_clk_get(int id)
{
    unsigned long  freq = 0;

    if(id == 0)//uart0
        crm_reg_get_valid_bit(INTF_CLKSEL_REG, 9, 1, &freq);
    else
        crm_reg_get_valid_bit(INTF_CLKSEL_REG, 10, 1, &freq);
    switch(freq)
    {
        case UARTPHY_CLKSEL_XTAL:
            freq = crm_xtal_clk_get();
            if(freq == 40)
                freq = 20;
            break;
        case UARTPHY_CLKSEL_120M:
            freq = 120;
            break;
    }
    return freq;
}

static int crm_intf_sdio1_clksel(intf_sdio_clksel_e type)
{
    if(crm_intf_locked(BIT_INTF_SDIO1_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 22, 2, 0x3);
    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 8, 1, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 22, 2, 0x1 << type);
    return 1;
}

static int crm_intf_sdio0_clksel(intf_sdio_clksel_e type)
{
    if(crm_intf_locked(BIT_INTF_SDIO0_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 20, 2, 0x3);
    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 7, 1, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 20, 2, 0x1 << type);
    return 1;
}
static int crm_intf_sdio_clk_get(int id)
{
    unsigned long  freq = 0;

    if(id == 0)//sdio0
        crm_reg_get_valid_bit(INTF_CLKSEL_REG, 7, 1, &freq);
    else
        crm_reg_get_valid_bit(INTF_CLKSEL_REG, 8, 1, &freq);
    switch(freq)
    {
        case SDIO_CLKSEL_100M:
            freq = 100;
            break;
        case SDIO_CLKSEL_90M:
            freq = 90;
            break;
    }
    return freq;

}
static int crm_intf_pnand_clksel(intf_pnand_clksel_e type)
{
    if(crm_intf_locked(BIT_INTF_PNAND_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 16, 4, 0xf);
    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 5, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 16, 4, 0x1 << type);
    return 1;
}
static int crm_intf_pnand_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(INTF_CLKSEL_REG, 5, 2, &freq);
    switch(freq)
    {
        case PNAND_CLKSEL_XTAL:
            freq = crm_xtal_clk_get();
            break;
        case PNAND_CLKSEL_288M:
            freq = 288;
            break;
        case PNAND_CLKSEL_262M:
            freq = 262;
            break;
         case PNAND_CLKSEL_200M:
            freq = 200;
            break;
    }
    return freq;
}

static int crm_intf_spi1_clksel(intf_spi_clksel_e type)
{
    unsigned long  spi0_freq = 0;
    if(crm_intf_locked(BIT_INTF_SPI1_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 12, 4, 0xf);
    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 3, 2, type);
    crm_reg_get_valid_bit(INTF_CLKSEL_REG, 1, 2, &spi0_freq);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 12, 4, 0x1 << type | 0x1 << spi0_freq);
    return 1;
}

static int crm_intf_spi0_clksel(intf_spi_clksel_e type)
{
    unsigned long  spi1_freq = 0;
    if(crm_intf_locked(BIT_INTF_SPI0_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 12, 4, 0xf);
    crm_reg_set_valid_bit(INTF_CLKSEL_REG, 1, 2, type);
    crm_reg_get_valid_bit(INTF_CLKSEL_REG, 3, 2, &spi1_freq);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 12, 4, 0x1 << type | 0x1 << spi1_freq);
    return 1;
}
static int crm_intf_spi_clk_get(int id)
{
    unsigned long  freq = 0;

    if(id == 0)//spi0
        crm_reg_get_valid_bit(INTF_CLKSEL_REG, 1, 2, &freq);
    else
        crm_reg_get_valid_bit(INTF_CLKSEL_REG, 3, 2, &freq);

    switch(freq)
    {
        case SPI_CLKSEL_XTAL:
            freq = crm_xtal_clk_get();
            break;
        case SPI_CLKSEL_360M:
            freq = 360;
            break;
        case SPI_CLKSEL_288M:
            freq = 288;
            break;
         case SPI_CLKSEL_400M:
            freq = 400;
            break;
    }
    return freq;
}

static int crm_intf_clk_enabled(intf_clken_e id)
{
    return ((crm_get_u32(INTF_CLKEN_REG) >> id) & 0x1);
}

static int crm_intf_reset(intf_srstn_e type)
{
    mt_u32 val1;
    val1=BIT_INTF_SRSTN_EN<<type;
    if(crm_intf_locked_rstreg(val1))
        return 0;
    crm_reset(INTF_SRSTN_REG, val1);
    return 1;
}

static MT_BOOL crm_gra_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(GRA_LOCK_REG, mask)
        ||crm_feature_is_locked(GRA_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_gra_clken(crm_clken_status_e status)
{
    if(crm_gra_locked(BIT_GRA_CLKEN_LOCK))
        return 0;
    crm_set_u32(GRA_CLKEN_REG, status);
    return 1;
}

static int crm_gra_clk_enabled(void)
{
    return (crm_get_u32(GRA_CLKEN_REG)& 0x1);
}

static int crm_gra_clksel(gra_clksel_e type)
{
    if(crm_gra_locked(BIT_GRA_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 24, 4, 0xf);
    if(type == GRA_CLKSEL_XTAL || type == GRA_CLKSEL_320M)
        crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 7, 1, GRA_CLKSEL_XTAL == type?1:0);
    crm_reg_set_valid_bit(GRA_CLKSEL_REG, 0, 2, type & 0x3);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 24, 4, 0x1 << type);
    return 1;
}

static int crm_gra_reset(void)
{
    if(crm_gra_locked(BIT_GRA_AXI_LOCK))
        return 0;

    crm_reset(GRA_SRSTN_REG, BIT_GRA_AXI_MASK|BIT_GRA_AHB_MASK);
    crm_reset(GRA_SRSTN_REG, BIT_GRA_CORE_MASK);

    return 1;
}



static int crm_gra_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(GRA_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
        case GRA_CLKSEL_262M:
            freq = 262;
            break;
        case GRA_CLKSEL_360M:
            freq = 360;
            break;
         case GRA_CLKSEL_320M:
            if((crm_get_u32(TOPCLK_CTRL6_REG) >> 7) & 0x1)
                freq = crm_xtal_clk_get();
            else
                freq = 320;
            break;
    }
    return freq;
}
static MT_BOOL crm_jpg_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(JPG_LOCK_REG, mask)
        ||crm_feature_is_locked(JPG_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_jpg_clken(crm_clken_status_e status)
{
    if(crm_jpg_locked(BIT_JPG_CLKEN_LOCK))
        return 0;
    crm_set_u32(JPG_CLKEN_REG, status);
    return 1;
}

static int crm_jpg_clk_enabled(void)
{
    return (crm_get_u32(JPG_CLKEN_REG)& 0x1);
}

static int crm_jpg_clksel(jpg_clksel_e type)
{
    if(crm_jpg_locked(BIT_JPG_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 28, 4, 0xf);
    if(type == JPG_CLKSEL_XTAL || type == JPG_CLKSEL_144M)
        crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 8, 1, JPG_CLKSEL_XTAL == type?1:0);
    crm_set_u32(JPG_CLKSEL_REG, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL1_REG, 28, 4, 0x1 << type);
    return 1;
}

static int crm_jpg_core_reset(void)
{
    if(crm_jpg_locked(BIT_JPG_CORE_LOCK))
        return 0;

    crm_reset(JPG_SRSTN_REG, BIT_JPG_CORE_MASK);

    return 1;
}


static int crm_jpg_axi_reset(void)
{
    if(crm_jpg_locked(BIT_JPG_AXI_LOCK))
        return 0;

    crm_reset(JPG_SRSTN_REG, BIT_JPG_AXI_MASK);

    return 1;
}


static int crm_jpg_ahb_reset(void)
{
    if(crm_jpg_locked(BIT_JPG_AHB_LOCK))
        return 0;

    crm_reset(JPG_SRSTN_REG, BIT_JPG_AHB_MASK);

    return 1;
}
static int crm_jpg_clk_get(void)
{
    unsigned long  freq = 0;

    if(!(crm_get_u32(JPG_CLKEN_REG)  & 0x1))
        return 0;
    crm_reg_get_valid_bit(JPG_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
        case JPG_CLKSEL_206M:
            freq = 206;
            break;
        case JPG_CLKSEL_144M:
            if((crm_get_u32(TOPCLK_CTRL6_REG) >> 8) & 0x1)
                freq = crm_xtal_clk_get();
            else
                freq = 144;
            break;
         default:
            freq = 206;
            break;
    }
    return freq;
}
static MT_BOOL crm_disp_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(DISP_LOCK_REG, mask)
        ||crm_feature_is_locked(DISP_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_disp_clken(crm_clken_status_e status)
{
    if(crm_disp_locked(BIT_DISP_CLKEN_LOCK))
        return 0;
    crm_set_u32(DISP_CLKEN_REG, status);
    return 1;
}

static int crm_disp_clk_enabled(void)
{
    return (crm_get_u32(DISP_CLKEN_REG)& 0x1);
}

static int crm_disp_osdc_clksel(disposdc_clksel_e type)
{
    if(crm_disp_locked(BIT_DISPOSDC_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(DISP_CLKSEL_REG, 6, 3, type);
    return 1;
}

static int crm_disp_di_clksel(dispdi_clksel_e type)
{
    if(crm_disp_locked(BIT_DISPDI_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(DISP_CLKSEL_REG, 3, 3, type);
    return 1;
}

static int crm_disp_core_clksel(dispcore_clksel_e type)
{
    if(crm_disp_locked(BIT_DISPCORE_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(DISP_CLKSEL_REG, 0, 3, type);

    return 1;
}

static int crm_disp_osdc_reset(void)
{
    if(crm_disp_locked(BIT_DISPOSDC_LOCK))
        return 0;

    crm_reset(DISP_SRSTN_REG, BIT_DISPOSDC_MASK);

    return 1;
}

static int crm_disp_di_reset(void)
{
    if(crm_disp_locked(BIT_DISPDI_LOCK))
        return 0;

    crm_reset(DISP_SRSTN_REG, BIT_DISPDI_MASK);

    return 1;
}

static int crm_disp_core_reset(void)
{
    if(crm_disp_locked(BIT_DISPCORE_LOCK))
        return 0;
    crm_reset(DISP_SRSTN_REG, BIT_DISPAHB_MASK|BIT_DISPAXI_MASK);
    crm_reset(DISP_SRSTN_REG, BIT_DISPCORE_MASK);

    return 1;
}

static int crm_disp_core_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(DISP_CLKSEL_REG, 0, 3, &freq);
    switch(freq)
    {
        case DISPCORE_CLKSEL_RESV:
        case DISPCORE_CLKSEL_206M:
            freq = 206;
            break;
        case DISPCORE_CLKSEL_180M:
            freq = 180;
            break;
         case DISPCORE_CLKSEL_131M:
            freq = 131;
            break;
         case DISPCORE_CLKSEL_40M:
            freq = 40;
            break;
         case DISPCORE_CLKSEL_60M:
            freq = 60;
            break;
    }
    return freq;
}
static int crm_disp_di_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(DISP_CLKSEL_REG, 3, 3, &freq);
    switch(freq)
    {
        case DISPDI_CLKSEL_288M:
            freq = 288;
            break;
        case DISPDI_CLKSEL_262M:
            freq = 262;
            break;
        case DISPDI_CLKSEL_160M:
            freq = 160;
            break;
         case DISPDI_CLKSEL_131M:
            freq = 131;
            break;
         case DISPDI_CLKSEL_80M:
            freq = 80;
            break;
         case DISPDI_CLKSEL_60M:
            freq = 60;
            break;
    }
    return freq;
}
static int crm_disp_osdc_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(DISP_CLKSEL_REG, 6, 3, &freq);
    switch(freq)
    {
        case DISPOSDC_CLKSEL_206M:
            freq = 206;
            break;
        case DISPOSDC_CLKSEL_250M:
            freq = 250;
            break;
         case DISPOSDC_CLKSEL_288M:
            freq = 288;
            break;
         case DISPOSDC_CLKSEL_144M:
            freq = 144;
            break;
         case DISPOSDC_CLKSEL_80M:
            freq = 80;
            break;
         case DISPOSDC_CLKSEL_60M:
            freq = 60;
            break;
    }
    return freq;
}
static MT_BOOL crm_vdec_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(VDEC_LOCK_REG, mask)
        ||crm_feature_is_locked(VDEC_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_vdec_clken(crm_clken_status_e status)
{
    if(crm_vdec_locked(BIT_VDEC_CLKEN_LOCK))
        return 0;
    crm_set_u32(VDEC_CLKEN_REG, status);
    return 1;
}

static int crm_vdec_clk_enabled(void)
{
    return (crm_get_u32(VDEC_CLKEN_REG)& 0x1);
}
static int crm_vdec_clksel(vdec_clksel_e type)
{
    if(crm_vdec_locked(BIT_VDEC_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 4, 4, 0xf);
    crm_reg_set_valid_bit(VDEC_CLKSEL_REG, 0, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 4, 4, 0x1 << type);
    return 1;
}

static int crm_vdec_core_core_reset(void)
{
    if(crm_vdec_locked(BIT_VDECCORE_CORE_LOCK))
        return 0;

    crm_reset(VDEC_SRSTN_REG, BIT_VDECCORE_CORE_MASK);

    return 1;
}

static int crm_vdec_core_axi_reset(void)
{
    if(crm_vdec_locked(BIT_VDECCORE_AXI_LOCK))
        return 0;

    crm_reset(VDEC_SRSTN_REG, BIT_VDECCORE_AXI_MASK);

    return 1;
}

static int crm_vdec_reset(void)
{
    if(crm_vdec_locked(BIT_VDECCORE_CORE_LOCK))
        return 0;
    crm_reset(VDEC_SRSTN_REG, BIT_VDECAHB_MASK | BIT_VDECAXI_MASK);
    crm_reset(VDEC_SRSTN_REG, BIT_VDECCORE_MASK);

    return 1;
}

static int crm_vdec_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(VDEC_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
        case VDEC_CLKSEL_262M:
            freq = 262;
            break;
        case VDEC_CLKSEL_206M:
            freq = 206;
            break;
        case VDEC_CLKSEL_131M:
            freq = 131;
            break;
         case VDEC_CLKSEL_45M:
            freq = 45;
            break;
    }
    return freq;
}

static MT_BOOL crm_aout_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(AOUT_LOCK_REG, mask)
        ||crm_feature_is_locked(AOUT_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_aout_spdf_clken(crm_clken_status_e status)
{
    if(crm_aout_locked(BIT_AOUT_SPDF_CLKEN_LOCK))
        return 0;
    crm_reg_set_valid_bit(AOUT_CLKEN_REG, 0, 1,status);
    return 1;
}

static int crm_aout_spdf_clk_enabled(void)
{
    return (crm_get_u32(AOUT_CLKEN_REG)& 0x1);
}

static int crm_aout_clken(crm_clken_status_e status)
{
    if(crm_aout_locked(BIT_AOUT_CLKEN_LOCK))
        return 0;
    crm_reg_set_valid_bit(AOUT_CLKEN_REG, 1, 1,status);
    return 1;
}

static int crm_aout_clk_enabled(void)
{
    return ((crm_get_u32(AOUT_CLKEN_REG) >> 1)& 0x1);
}

static int crm_aout_clksel(aout_clksel_e type)
{
    if(crm_aout_locked(BIT_AOUT_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 8, 4, 0xf);
    if(type == AOUT_CLKSEL_XTAL || type == AOUT_CLKSEL_131M)
        crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 11, 1, type == AOUT_CLKSEL_XTAL?1:0);
    crm_reg_set_valid_bit(AOUT_CLKSEL_REG, 0, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 8, 4, 0x1 << type);
    return 1;
}

static int crm_aout_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(AOUT_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
        case AOUT_CLKSEL_288M:
            freq = 288;
            break;
        case AOUT_CLKSEL_262M:
            freq = 262;
            break;
        case AOUT_CLKSEL_206M:
            freq = 206;
            break;
         case AOUT_CLKSEL_131M:
            freq = 131;
            break;
    }
    return freq;
}

static int crm_aout_mclk_clksel(aout_mclk_clksel_e type)
{
    if(crm_aout_locked(BIT_AOUT_MCLK_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(AOUT_CLKSEL_REG, 2, 1, type);
    return 1;
}

static int crm_aout_mclk_get(void)
{
    return (crm_get_u32(AOUT_CLKSEL_REG) >> 2)& 0x1;
}

static int crm_aout_adac_reset(void)
{
    if(crm_aout_locked(BIT_AOUT_ADAC_LOCK))
        return 0;
    crm_reset(AOUT_SRSTN_REG, BIT_AOUT_ADAC_MASK);
    return 1;
}

static int crm_aout_mclk_reset(void)
{
    if(crm_aout_locked(BIT_AOUT_MCLK_LOCK))
        return 0;

    crm_reset(AOUT_SRSTN_REG, BIT_AOUT_MCLK_MASK);

    return 1;
}

static int crm_aout_core_reset(void)
{
    if(crm_aout_locked(BIT_AOUTCORE_LOCK|BIT_AOUTAXI_LOCK|BIT_AOUTAHB_LOCK))
        return 0;
    crm_reset(AOUT_SRSTN_REG, BIT_AOUTAXI_MASK | BIT_AOUTAHB_MASK);
    crm_reset(AOUT_SRSTN_REG, BIT_AOUTCORE_MASK);

    return 1;
}




static MT_BOOL crm_vout_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(VOUT_LOCK_REG, mask)
        ||crm_feature_is_locked(VOUT_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}



static int crm_vout_vbi_clken(crm_clken_status_e status)
{
    if(crm_vout_locked(BIT_VOUT_VBI_CLKEN_LOCK))
        return 0;

    crm_reg_set_valid_bit(VOUT_CLKEN_REG, 1, 1, status);
    return 1;
}

static int crm_vout_hdmi_clken(crm_clken_status_e status)
{
    if(crm_vout_locked(BIT_VOUT_HDMI_CLKEN_LOCK))
        return 0;
    crm_reg_set_valid_bit(VOUT_CLKEN_REG, 4, 1, status);
    return 1;
}

static int crm_vout_lcd_clken(crm_clken_status_e status)
{
    if(crm_vout_locked(BIT_VOUT_LCD_CLKEN_LOCK))
        return 0;
    crm_reg_set_valid_bit(VOUT_CLKEN_REG, 3, 1, status);
    return 1;
}

static int crm_vout_hdvenc_clken(crm_clken_status_e status)
{
    if(crm_vout_locked(BIT_VOUT_HDVENC_CLKEN_LOCK))
        return 0;

    crm_reg_set_valid_bit(VOUT_CLKEN_REG, 2, 1, status);
    return 1;
}

static int crm_vout_sdvenc_clken(crm_clken_status_e status)
{
    if(crm_vout_locked(BIT_VOUT_SDVENC_CLKEN_LOCK))
        return 0;
    crm_reg_set_valid_bit(VOUT_CLKEN_REG, 0, 1, status);
    return 1;
}

static int crm_vout_lcdc_clksel(vout_lcdc_clksel_e type)
{
    if(crm_vout_locked(BIT_VOUT_LCDC_CLK_DIVCFG_LOCK))
        return 0;
    crm_reg_set_valid_bit(VOUT_CLKSEL_REG, 7, 3, type);
    return 1;
}

static int crm_vout_lcdhd_clksel(vout_lcdhd_clksel_e type)
{
    if(crm_vout_locked(BIT_VOUT_LCDC_CLK_DIVCFG_LOCK))
        return 0;
    crm_reg_set_valid_bit(VOUT_CLKSEL_REG, 6, 1, type);
    return 1;
}

static int crm_vout_lcd2x_clksel(vout_lcd2x_clksel_e type)
{
    if(crm_vout_locked(BIT_VOUT_LCD_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 12, 5, 0x1f);
    crm_reg_set_valid_bit(VOUT_CLKSEL_REG, 3, 3, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 12, 5, 0x1 << type);
    return 1;
}

static int crm_vout_hdvenc_clksel(vout_hdvenc_clksel_e type)
{
    if(crm_vout_locked(BIT_VOUT_HDVENC_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(VOUT_CLKSEL_REG, 0, 3, type);
    return 1;
}

static int crm_vout_reset(vout_srstn_sel_e type)
{
    if(crm_vout_locked(type+BIT_VOUT_SRSTN_MAP_LOCK))
        return 0;
    crm_reset(VOUT_SRSTN_REG, 0x1 << type);
    return 1;
}
static int crm_vout_venc_oscclk_get(void)
{
    //refer reg 0xbf5d005c
    //TBD
    return 1;
}
static int crm_vout_sdvenc_clk_enabled(void)
{
    return (crm_get_u32(VOUT_CLKEN_REG) >> 0) & 0x1;
}
static int crm_vout_vbi_clk_enabled(void)
{
    return (crm_get_u32(VOUT_CLKEN_REG) >> 1) & 0x1;
}
static int crm_vout_hdvenc_clk_enabled(void)
{
    return (crm_get_u32(VOUT_CLKEN_REG) >> 2) & 0x1;

}
static int crm_vout_lcd_clk_enabled(void)
{
    return (crm_get_u32(VOUT_CLKEN_REG) >> 3) & 0x1;
}
static int crm_vout_hdmi_clk_enabled(void)
{
    return (crm_get_u32(VOUT_CLKEN_REG) >> 4)  & 0x1;
}

static int crm_vout_lcd2x_clk_get(void)
{
    unsigned long freq = 0;

    crm_reg_get_valid_bit(VOUT_CLKSEL_REG, 3, 3, &freq);
    switch(freq)
    {
        case VOUT_LCD_CLKSEL_64M:
            freq = 64;
            break;
        case VOUT_LCD_CLKSEL_72M:
            freq = 72;
            break;
        case VOUT_LCD_CLKSEL_80M:
            freq = 80;
            break;
        case VOUT_LCD_CLKSEL_131M:
            freq = 131;
            break;
        case VOUT_LCD_CLKSEL_138M:
            freq = 138;
            break;
        case VOUT_LCD_CLKSEL_54M:
            freq = 54;
            break;
        case VOUT_LCD_CLKSEL_VENC_OSCLK:
            freq = crm_vout_venc_oscclk_get();
            break;
    }
    return freq;
}
static int crm_vout_lcdc_clk_get(void)
{
    unsigned long  val= 0, freq = 0;

    crm_reg_get_valid_bit(DISP_CLKSEL_REG, 7, 3, &freq);
    val = crm_vout_lcd2x_clk_get();
    switch(freq)
    {
        case VOUT_LCDC_CLKSEL_LCD2XCLK_DIV1:
            freq = val;
            break;
        case VOUT_LCDC_CLKSEL_LCD2XCLK_DIV2:
            freq = val/2;
            break;
        case VOUT_LCDC_CLKSEL_LCD2XCLK_DIV4:
            freq = val /4;
            break;
        case VOUT_LCDC_CLKSEL_LCD2XCLK_DIV6:
            freq = val /6;
            break;
    }
    return freq;
}

static int crm_vout_hdvenc_clk_get(void)
{
    unsigned long  val = 0, freq = 0;

    crm_reg_get_valid_bit(DISP_CLKSEL_REG, 0, 3, &freq);
    val = crm_vout_venc_oscclk_get();
    switch(freq)
    {
        case VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV2:
            freq = val /2;
            break;
        case VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV4:
            freq = val / 4;
            break;
        case VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV6:
            freq = val / 6;
            break;
        case VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV8:
            freq = val / 8;
            break;
        default:
            freq = val;
    }
    return freq;
}

static int crm_vout_lcdhd_clk_get(void)
{
    unsigned long val = 0, freq = 0;

    crm_reg_get_valid_bit(DISP_CLKSEL_REG, 6, 1, &freq);
    val = crm_vout_venc_oscclk_get();
    switch(freq)
    {
        case VOUT_LCDHD_CLKSEL_HDVENC_EN:
            freq = val;
            break;
        case VOUT_LCDHD_CLKSEL_LCDC_EN:
            freq = val /2;
            break;
    }
    return freq;
}

static MT_BOOL crm_png_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(PNG_LOCK_REG, mask)
        ||crm_feature_is_locked(PNG_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_png_clken(crm_clken_status_e status)
{
    if(crm_png_locked(BIT_PNG_CLKEN_LOCK))
        return 0;
    crm_set_u32(PNG_CLKEN_REG, status);
    return 1;
}

static int crm_png_clk_enabled(void)
{
    return (crm_get_u32(PNG_CLKEN_REG) & 0x1);
}

static int crm_png_clksel(png_clksel_e type)
{
    if(crm_png_locked(BIT_PNG_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 20, 4, 0xf);
    if(type == PNG_CLKSEL_XTAL || type == PNG_CLKSEL_144M)
        crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 12, 1, PNG_CLKSEL_XTAL == type?1:0);
    crm_reg_set_valid_bit(PNG_CLKSEL_REG, 0, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 20, 4, 0x1 << type);
    return 1;
}

static int crm_png_core_reset(void)
{
    if(crm_png_locked(BIT_PNGCORE_LOCK))
        return 0;
    crm_reset(PNG_SRSTN_REG, BIT_PNGAXI_MASK | BIT_PNGAHB_MASK);
    crm_reset(PNG_SRSTN_REG, BIT_PNGCORE_MASK);

    return 1;
}

static int crm_png_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(PNG_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
        case PNG_CLKSEL_RESV0:
            freq = 262;
            break;
        case PNG_CLKSEL_206M:
            freq = 206;
            break;
        case PNG_CLKSEL_144M:
            if((crm_get_u32(TOPCLK_CTRL6_REG) >> 12) & 0x1)
                freq = crm_xtal_clk_get();
            else
                freq = 144;
            break;
        case PNG_CLKSEL_RESV1:
            freq = 144;
            break;
    }
    return freq;
}
static MT_BOOL crm_dai_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(DAI_LOCK_REG, mask)
        ||crm_feature_is_locked(DAI_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_dai_clken(crm_clken_status_e status)
{
    if(crm_dai_locked(BIT_DAI_CLKEN_LOCK))
        return 0;
    crm_set_u32(DAI_CLKEN_REG, status);
    return 1;
}

static int crm_dai_clk_enabled(void)
{
    return (crm_get_u32(DAI_CLKEN_REG) & 0x1);
}

static int crm_dai_reset(dai_srstn_e type)
{
    if(crm_dai_locked(type+BIT_DAI_LOCK_MAP_SRSTN))
        return 0;

    crm_reset(DAI_SRSTN_REG, (BIT_DAI_SRSTN_MASK<<type));

    return 1;
}

static MT_BOOL crm_tsi_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(TSI_LOCK_REG, mask)
        ||crm_feature_is_locked(TSI_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_tsi_clken(crm_clken_status_e status)
{
    if(crm_tsi_locked(BIT_TSI_CLKEN_LOCK))
        return 0;
    crm_set_u32(TSI_CLKEN_REG, status);
    return 1;
}

static int crm_tsi_clk_enabled(void)
{
    return (crm_get_u32(TSI_CLKEN_REG) & 0x1);
}
static int crm_tsi_clksel(tsi_clksel_e type)
{
    if(crm_tsi_locked(BIT_TSI_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 28, 4, 0xf);
    crm_reg_set_valid_bit(TSI_CLKSEL_REG, 0, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL2_REG, 28, 4, 0x1 << type);
    return 1;
}

static int crm_tsi_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(TSI_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
        case TSI_CLKSEL_RESV: //reserve
            freq = 288;
            break;
        case TSI_CLKSEL_262M:
            freq = 262;
            break;
        case TSI_CLKSEL_206M:
            freq = 206;
            break;
         case TSI_CLKSEL_131M:
            freq = 131;
            break;
    }
    return freq;
}

static int crm_tsi_ts0_clksel(ts0_clksel_mode_e type)
{
    if(crm_tsi_locked(BIT_TS0_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(TSI_CLKSEL_REG, 2, 1, type);
    return 1;
}

static int crm_tsi_ts0_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(TSI_CLKSEL_REG, 2, 1, &freq);
    return freq;
}

static int crm_tsi_ts1_clksel(ts1_clksel_mode_e type)
{
    if(crm_tsi_locked(BIT_TS1_CLKSEL_LOCK))
        return 0;

    crm_reg_set_valid_bit(TSI_CLKSEL_REG, 3, 2, type);
    return 1;
}

static int crm_tsi_ts1_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(TSI_CLKSEL_REG, 3, 2, &freq);
    return freq;
}


static int crm_tsi_ts2_clksel(ts2_clksel_mode_e type)
{
    if(crm_tsi_locked(BIT_TS2_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TSI_CLKSEL_REG, 5, 2, type);

    return 1;
}

static int crm_tsi_ts2_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(TSI_CLKSEL_REG, 5, 2, &freq);
    return freq;
}

static int crm_tsi_ts3_clksel(ts3_clksel_mode_e type)
{
    if(crm_tsi_locked(BIT_TS3_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TSI_CLKSEL_REG, 7, 2, type);

    return 1;
}

static int crm_tsi_ts3_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(TSI_CLKSEL_REG, 7, 2, &freq);
    return freq;
}

static int crm_tsi_reset(tsi_srstn_e type)
{
    if(crm_tsi_locked(type+BIT_TSI_SRSTN_MAP_LOCK))
        return 0;

    crm_reset(TSI_SRSTN_REG, (BIT_TSI_SRSTN_MASK<<type));

    return 1;
}

static MT_BOOL crm_ci_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(CI_LOCK_REG, mask)
        ||crm_feature_is_locked(CI_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_ci_clken(crm_clken_status_e status)
{
    if(crm_tsi_locked(BIT_CI_CLKEN_LOCK))
        return 0;
    crm_set_u32(CI_CLKEN_REG, status);
    return 1;
}

static inline int crm_ci_clk_enabled(void)
{
    return crm_get_u32(CI_CLKEN_REG) & 0x1;
}

static int crm_ci_clksel(ci_clksel_e type)
{
    if(crm_ci_locked(BIT_CI_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL3_REG, 12, 2, 0x3);
    crm_reg_set_valid_bit(TSI_CLKSEL_REG, 0, 1, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL3_REG, 12, 2, 0x1 << type);
    return 1;
}

static int crm_ci_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(TSI_CLKSEL_REG, 0, 1, &freq);
    switch(freq)
    {
        case CI_CLKSEL_80M: //reserve
            freq = 80;
            break;
        case CI_CLKSEL_120M:
            freq = 120;
    }
    return freq;
}

static int crm_ci_tsin_clksel(citsin_clksel_mode_e type)
{
    if(crm_ci_locked(BIT_CI_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TSI_CLKSEL_REG, 1, 3, type);
    return 1;
}

static int crm_ci_tsin_clk_get(void)
{
    unsigned long  freq = 0;

    crm_reg_get_valid_bit(TSI_CLKSEL_REG, 1, 3, &freq);
    return freq;
}

static int crm_ci_core_reset(void)
{
    if(crm_ci_locked(BIT_CI_CORE_LOCK))
        return 0;

    crm_reset(CI_SRSTN_REG, BIT_CI_CORE_MASK);

    return 1;
}

static int crm_ci_ahb_reset(void)
{
    if(crm_ci_locked(BIT_CI_AHB_LOCK))
        return 0;

    crm_reset(CI_SRSTN_REG, BIT_CI_AHB_MASK);

    return 1;
}

static MT_BOOL crm_secure_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(SECURE_LOCK_REG, mask)
        ||crm_feature_is_locked(SECURE_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_secure_clken(secure_clken_e type,crm_clken_status_e status)
{
    if(crm_secure_locked(type))
        return 0;

    crm_reg_set_valid_bit(SECURE_CLKEN_REG, type, 1, status);
    return 1;
}

static inline int crm_secure_clk_enabled(secure_clken_e type)
{
    return (crm_get_u32(SECURE_CLKEN_REG) >> type) & 0x1;
}

static int crm_secure_m2m_cipher_clksel(secure_m2m_chiper_clksel_e type)
{
    if(crm_secure_locked(BIT_SECURE_M2M_CIPHER_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL4_REG, 8, 4, 0xf);
    crm_reg_set_valid_bit(SECURE_CLKSEL_REG, 2, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL4_REG, 8, 4, 0x1 << type);
    return 1;
}

static int crm_secure_m2m_cipher_clk_get(void)
{
    unsigned long  freq = 0;
    crm_reg_get_valid_bit(SECURE_CLKSEL_REG, 2, 2, &freq);
    switch(freq)
    {
        case SECURE_M2M_CIPHER_CLKSEL_262M:  //reserve
            freq = 262;
            break;
        case SECURE_M2M_CIPHER_CLKSEL_240M:
            freq = 240;
            break;
        case SECURE_M2M_CIPHER_CLKSEL_206M:
            freq = 206;
            break;
        case SECURE_M2M_CIPHER_CLKSEL_131M:
            freq = 131;
            break;
    }
    return freq;
}

static int crm_secure_clksel(secure_clksel_e type)
{
    if(crm_secure_locked(BIT_SECURE_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL4_REG, 4, 4, 0xf);
    if(type == SECURE_CLKSEL_XTAL || type == SECURE_CLKSEL_131M)
        crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 16, 1, type == SECURE_CLKSEL_XTAL?1:0);
    crm_reg_set_valid_bit(SECURE_CLKSEL_REG, 0, 2, type & 0x3);
    crm_reg_set_valid_bit(TOPCLK_CTRL4_REG, 4, 4, 0x1 << type);

    return 1;
}

static int crm_secure_clk_get(void)
{
    unsigned long  freq = 0;
    crm_reg_get_valid_bit(SECURE_CLKSEL_REG, 0, 2, &freq);
    switch(freq)
    {
        case SECURE_CLKSEL_262M:  //reserve
            freq = 262;
            break;
        case SECURE_CLKSEL_206M:
            freq = 206;
            break;
        case SECURE_CLKSEL_240M:
            freq = 240;
            break;
        case SECURE_CLKSEL_131M:
            if((crm_get_u32(TOPCLK_CTRL6_REG) >> 16) & 0x1)
                freq = crm_xtal_clk_get();
            else
                freq = 131;
            break;
    }
    return freq;
}

static int crm_secure_reset(secure_srstn_e type)
{
    if(crm_secure_locked(type+BIT_SECURE_SRSTN_MAP_LOCK))
        return 0;

    crm_reset(SECURE_CLKSEL_REG, (BIT_SECURE_SRSTN_MASK<<type));

    return 1;
}
static MT_BOOL crm_ifcp_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(IFCP_LOCK_REG, mask)
        ||crm_feature_is_locked(IFCP_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_ifcp_klm_clken(crm_clken_status_e status)
{
    if(crm_ifcp_locked(BIT_IFCP_KLM_CLKEN_LOCK))
        return 0;

    crm_reg_set_valid_bit(IFCP_CLKEN_REG, 1, 1, status);
    return 1;
}

static int crm_ifcp_klm_clk_enabled(void)
{
    return (crm_get_u32(IFCP_CLKEN_REG) >> 1) & 0x1;
}

static int crm_ifcp_crypto_clken(crm_clken_status_e status)
{
    if(crm_ifcp_locked(BIT_IFCP_CRYPTO_CLKEN_LOCK))
        return 0;

    crm_reg_set_valid_bit(IFCP_CLKEN_REG, 0, 1, status);
    return 1;
}

static int crm_ifcp_crypto_clk_enabled(void)
{
    return crm_get_u32(IFCP_CLKEN_REG) & 0x1;
}

static int crm_ifcp_crypto_clksel(ifcp_crypto_clksel_e type)
{
    if(crm_ifcp_locked(BIT_IFCP_CRYPTO_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL3_REG, 8, 4, 0xf);
    crm_reg_set_valid_bit(IFCP_CLKSEL_REG, 3, 2, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL3_REG, 8, 4, 0x1 << type);
    return 1;
}


static int crm_ifcp_crypto_clk_get(void)
{
    unsigned long  freq = 0;
    crm_reg_get_valid_bit(IFCP_CLKSEL_REG, 3, 2, &freq);
    switch(freq)
    {
        case IFCP_CRYPTO_CLKSEL_206M:  //reserve
            freq = 206;
            break;
        case IFCP_CRYPTO_CLKSEL_120M:
            freq = 120;
            break;
        case IFCP_CRYPTO_CLKSEL_240M:
            freq = 240;
            break;
        default:
            freq = 262;
            break;
    }
    return freq;
}


static int crm_ifcp_sys_clksel(ifcp_sys_clksel_e type)
{
    if(crm_ifcp_locked(BIT_IFCP_SYS_CLKSEL_LOCK))
        return 0;
    crm_reg_set_valid_bit(TOPCLK_CTRL3_REG, 0, 8, 0xff);
    crm_reg_set_valid_bit(IFCP_CLKSEL_REG, 0, 3, type);
    crm_reg_set_valid_bit(TOPCLK_CTRL3_REG, 0, 8, 0x1 << type);
    return 1;
}

static int crm_ifcp_sys_clk_get(void)
{
    unsigned long  freq = 0;
    crm_reg_get_valid_bit(IFCP_CLKSEL_REG, 0, 3, &freq);
    switch(freq)
    {
        case IFCP_SYS_CLKSEL_288M:  //reserve
            freq = 288;
            break;
        case IFCP_SYS_CLKSEL_240M:
            freq = 240;
            break;
        case IFCP_SYS_CLKSEL_180M:
            freq = 180;
            break;
        case IFCP_SYS_CLKSEL_90M:
            freq = 90;
            break;
        case IFCP_SYS_CLKSEL_24M:
            freq = 24;
            break;
        default:
            freq = 288;
            break;
    }
    return freq;
}

#if 0
static int crm_ifcp_reset(void)
{
    if(crm_get_u32(SHAREREG_VSCPU_RST_CTRL_REG)&BIT_IFCP_SRSTN_LOCK)
        return 0;

    crm_reset(IFCP_GLB_SRSTN_REG, BIT_IFCP_SYS_CLKSEL_MASK);

    return 1;
}
#endif

static MT_BOOL crm_aocrm_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(AOCRM_LOCK_REG, mask)
        ||crm_feature_is_locked(AOCRM_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

static int crm_ao_lpm_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_LPM_LOCK))
     return 0;
  crm_set_u32(AO_LPMCLK_REG, status);
  return 1;
}

static int crm_ao_lpm_clk_enabled(void)
{
    return (crm_get_u32(AO_LPMCLK_REG) & 0x1);
}


static int crm_ao_lpm_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_LPM_LOCK))
     return 0;

  crm_ao_reset(AO_LPMRST_REG);

  return 1;
}


static int crm_ao_irda_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_IRDA_LOCK))
     return 0;
  crm_set_u32(AO_IRDACLK_REG, status);
  return 1;
}

static int crm_ao_irda_clk_enabled(void)
{
    return (crm_get_u32(AO_IRDACLK_REG) & 0x1);
}


static int crm_ao_irda_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_IRDA_LOCK))
     return 0;

  crm_ao_reset(AO_IRDARST_REG);

  return 1;
}

static int crm_ao_rtc_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_RTC_LOCK))
     return 0;

  crm_ao_reset(AO_RTCRST_REG);

  return 1;
}

static int crm_ao_ledkb_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_LEDKB_LOCK))
     return 0;
  crm_set_u32(AO_LEDKBCLK_REG, status);
  return 1;
}

static int crm_ao_ledkb_clk_enabled(void)
{
    return (crm_get_u32(AO_LEDKBCLK_REG) & 0x1);
}

static int crm_ao_ledkb_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_LEDKB_LOCK))
     return 0;

  crm_ao_reset(AO_LEDKBRST_REG);

  return 1;
}

static int crm_ao_gpio_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_GPIO_LOCK))
     return 0;
  crm_set_u32(AO_GPIOCLK_REG, status);
  return 1;
}

static int crm_ao_gpio_clk_enabled(void)
{
    return (crm_get_u32(AO_GPIOCLK_REG) & 0x1);
}

static int crm_ao_gpio_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_GPIO_LOCK))
     return 0;

  crm_ao_reset(AO_GPIORST_REG);

  return 1;
}

static int crm_ao_kadc_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_KADC_LOCK))
     return 0;
  crm_set_u32(AO_KADCCLK_REG, status);
  return 1;
}

static int crm_ao_kadc_clk_enabled(void)
{
    return (crm_get_u32(AO_KADCCLK_REG) & 0x1);
}

static int crm_ao_kadc_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_KADC_LOCK))
     return 0;

  crm_ao_reset(AO_KADCRST_REG);

  return 1;
}

static int crm_ao_ana_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_ANA_LOCK))
     return 0;
  crm_set_u32(AO_ANACLK_REG, status);
  return 1;
}

static int crm_ao_ana_clk_enabled(void)
{
    return (crm_get_u32(AO_ANACLK_REG) & 0x1);
}

static int crm_ao_fpi2c_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_FPI2C_LOCK))
     return 0;
  crm_set_u32(AO_FPI2CCLK_REG, status);
  return 1;
}

static int crm_ao_fpi2c_clk_enabled(void)
{
    return (crm_get_u32(AO_FPI2CCLK_REG) & 0x1);
}

static int crm_ao_fpi2c_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_FPI2C_LOCK))
     return 0;

  crm_ao_reset(AO_FPI2CRST_REG);

  return 1;
}


static int crm_ao_fpspi_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_FPSPI_LOCK))
     return 0;
  crm_set_u32(AO_FPSPICLK_REG, status);
  return 1;
}

static int crm_ao_fpspi_clk_enabled(void)
{
    return (crm_get_u32(AO_FPSPICLK_REG) & 0x1);
}

static int crm_ao_fpspi_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_FPSPI_LOCK))
     return 0;

  //???
  REG32(AO_FPSPIRST_REG) &=~BIT_AO_FPSPI_SRSTN_MASK;
  REG32(AO_FPSPIRST_REG) |=BIT_AO_RESET_RELEASE;

  return 1;
}

static int crm_ao_fpspireg_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_FPSPI_LOCK))
     return 0;

  //???
  REG32(AO_FPSPIRST_REG) &=~BIT_AO_FPSPIREG_SRSTN_MASK;
  REG32(AO_FPSPIRST_REG) |=BIT_AO_RESET_RELEASE;
  return 1;
}


static int crm_ao_recram_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_RECRAM_LOCK))
     return 0;
  crm_set_u32(AO_RECRAMCLK_REG, status);
  return 1;
}

static int crm_ao_recram_clk_enabled(void)
{
    return (crm_get_u32(AO_RECRAMCLK_REG) & 0x1);
}

static int crm_ao_pinmux_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_PINMUX_LOCK))
     return 0;
  crm_set_u32(AO_PINMUXCLK_REG, status);
  return 1;
}

static int crm_ao_pinmux_clk_enabled(void)
{
    return (crm_get_u32(AO_PINMUXCLK_REG) & 0x1);
}

static int crm_ao_pinmux_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_PINMUX_LOCK))
     return 0;

  crm_ao_reset(AO_PINMUXRST_REG);

  return 1;
}

static int crm_ao_timer_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_TIMER_LOCK))
     return 0;
  crm_set_u32(AO_TIMERCLK_REG, status);
  return 1;
}

static int crm_ao_timer_clk_enabled(void)
{
    return (crm_get_u32(AO_TIMERCLK_REG) & 0x1);
}

static int crm_ao_timer_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_TIMER_LOCK))
     return 0;

  crm_ao_reset(AO_TIMERRST_REG);

  return 1;
}

static int crm_ao_mntsosc_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_MAILBOX_LOCK))
     return 0;

  crm_reset(AO_TIMERRST_REG, BIT_AO_MNTSOSC_SRSTN_MASK);

  return 1;
}

static int crm_ao_mntxtal_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_MNT_LOCK))
     return 0;

  crm_reset(AO_TIMERRST_REG, BIT_AO_MNTXTAL_SRSTN_MASK);

  return 1;
}

static int crm_ao_mailbox_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_MAILBOX_LOCK))
     return 0;
  crm_set_u32(AO_MAILBOXCLK_REG, status);
  return 1;
}

static int crm_ao_mailbox_clk_enabled(void)
{
    return (crm_get_u32(AO_MAILBOXCLK_REG) & 0x1);
}

static int crm_ao_mailbox_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_MAILBOX_LOCK))
     return 0;

  crm_ao_reset(AO_MAILBOXRST_REG);

  return 1;
}

static int crm_ao_cec_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_CEC_LOCK))
     return 0;
  crm_set_u32(AO_CECCLK_REG, status);
  return 1;
}

static int crm_ao_cec_clk_enabled(void)
{
    return (crm_get_u32(AO_CECCLK_REG) & 0x1);
}

static int crm_ao_cec_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_CEC_LOCK))
     return 0;

  crm_ao_reset(AO_CECRST_REG);

  return 1;
}

static int crm_ao_avs_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_AVS_LOCK))
     return 0;
  crm_set_u32(AO_AVSCLK_REG, status);
  return 1;
}

static int crm_ao_avs_clk_enabled(void)
{
    return (crm_get_u32(AO_AVSCLK_REG) & 0x1);
}

static int crm_ao_avs_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_AVS_LOCK))
     return 0;

  crm_ao_reset(AO_AVSRST_REG);

  return 1;
}

static int crm_ao_agtimer_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_AGTIMER_LOCK))
     return 0;
  crm_set_u32(AO_AGTIMERCLK_REG, status);
  return 1;
}

static int crm_ao_agtimer_clk_enabled(void)
{
    return (crm_get_u32(AO_AGTIMERCLK_REG) & 0x1);
}

static int crm_ao_agtimer_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_AGTIMER_LOCK))
     return 0;
  crm_set_u32(AO_AGTIMERRST_REG, BIT_AO_RESET);
  //FIXME: AO_WDOGRST_REG???
  crm_set_u32(AO_WDOGRST_REG, BIT_AO_RESET_RELEASE);
  return 1;
}

static int crm_ao_aomcu_clken(crm_clken_status_e status)
{
  if(crm_aocrm_locked(BIT_AO_AOMCU_LOCK))
     return 0;
  crm_set_u32(AO_AOMCUCLK_REG, status);
  return 1;
}

static int crm_ao_aomcu_clk_enabled(void)
{
    return (crm_get_u32(AO_AOMCUCLK_REG) & 0x1);
}

static int crm_ao_wdog_reset(void)
{
  if(crm_aocrm_locked(BIT_AO_WDOG_LOCK))
     return 0;

  crm_ao_reset(AO_WDOGRST_REG);

  return 1;
}

static int crm_ao_xtal_clksel(crm_xtal_sel_e type)
{
  if(crm_aocrm_locked(BIT_AO_XTALSEL_LOCK))
     return 0;

  crm_reg_set_valid_bit(XTAL_CLKSEL_REG, 0, 1, type);
  return 1;
}

static int crm_ao_xtal_clk_mode_get(void)
{
    return crm_get_u32(XTAL_CLKSEL_REG) & 0x1;
}

static int crm_panther2_clken(crm_clken_status_e status)
{
  if(status == CLKEN_OFF)
  {
      crm_reg_set_valid_bit(REG_CLK_ANALOG, 4, 2, 3); //pd panther2 aadc
      crm_reg_set_valid_bit(REG_CLK_ANALOG1, 4, 1, 1); //pd panther2 aadc mic
      crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf5d0120), 11, 1, 1);  //pd panther2 clock-gen
      crm_reg_set_valid_bit(REG_CLK_ANALOG, 22, 1, 1); //pd panther2 madc
  }
  else
  {
      crm_reg_set_valid_bit(REG_CLK_ANALOG, 4, 2, 0); //en panther2 aadc
      crm_reg_set_valid_bit(REG_CLK_ANALOG1, 4, 1, 0); //enpanther2 aadc mic
      crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf5d0120), 11, 1, 0);  //en panther2 clock-gen
      crm_reg_set_valid_bit(REG_CLK_ANALOG, 22, 1, 0); //en panther2 madc
  }
  return 1;
}
static int crm_panther2_clk_enabled(void)
{
   return !(crm_get_u32(SYMPHONY_IO_VA(0xbf5d0120)) >> 11) & 0x1;
}
#if 0
static MT_BOOL apcpu_crm_locked(mt_u32 mask)
{
    if(crm_feature_is_locked(APCPU_LOCK_REG, mask)
        ||crm_feature_is_locked(APCPU_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

int apcpu_crm_clk_en(void)
{
    if(apcpu_crm_locked(BIT_APCPU_CLKEN_LOCK))
        return 0;
    REG32(APCPU_CLKEN_REG) |= BIT_APCPU_CLKEN;
    return 1;
}

int apcpu_crm_pclk_en(void)
{
    if(apcpu_crm_locked(BIT_APCPU_PCLKEN_LOCK))
        return 0;
    REG32(APCPU_CLKEN_REG) |= BIT_APCPU_PCLKEN;
    return 1;
}

int apcpu_crm_clk_switch_mask(int type )
{
	crm_clk_sel(APCPU_CLKSEL0_REG, BIT_APCPU_CLK_SWITCHMASK, (type<<POS_APCPU_CLK_SWITCHMASK));

    return 1;
}

int apcpu_crm_backup_armpll_clksel(int type)
{
    if(apcpu_crm_locked(BIT_ARMPLL_CFG_LOCK))
        return 0;

	crm_clk_sel(APCPU_CLKSEL0_REG, BIT_BACKUP_ARMPLL_CLKSEL_MASK, (type<<POS_APCPU_BACKUP_ARMPLL_CLKSEL));

    return 1;
}

int apcpu_crm_atclk_cfg_freq_update(void )
{
    REG32(APCPU_CLKSEL0_REG) |=  BIT_APCPU_ATCLK_CFG_FREQ_UPDATE;
    return 1;
}

int apcpu_crm_atclk_cfg_div_sel (apcpu_debug_clk_sel_e type )
{
    if(apcpu_crm_locked(BIT_ATCLK_CFG_LOCK))
        return 0;

	crm_clk_sel(APCPU_CLKSEL0_REG, BIT_ATCLK_CFG_DIV_SEL_MASK, (type<<POS_APCPU_ATCLK_CFG_DIV));

    return 1;
}

int apcpu_crm_apbackup_clksel (apcpu_clk_sel0_e type )
{
    if(apcpu_crm_locked(BIT_APBACKUP_CLKSEL_LOCK))
        return 0;

	crm_clk_sel(APCPU_CLKSEL0_REG, BIT_APBACKUP_CLKSEL_MASK, (type<<POS_APCPU_APBACKUP_CLK));

    return 1;
}

MT_BOOL apcpu_crm_clksel1_vco_sel(apcpu_vco_sel_e type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_2BIT_MASK << POS_APCPU_CLKSEL1_VCO_SEL),
				(type<<POS_APCPU_CLKSEL1_VCO_SEL));

    return 1;
}

int apcpu_crm_clksel1_vco_ext(void )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_2BIT_MASK<<POS_APCPU_CLKSEL1_VCO_EXT),
				(BIT_APCPU_CLKSEL1_VCO_EXT<<POS_APCPU_CLKSEL1_VCO_EXT));

    return 1;
}


int apcpu_crm_clksel1_cpvr_sel(int type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_2BIT_MASK << POS_APCPU_CLKSEL1_CPVR_SEL),
				(type<<POS_APCPU_CLKSEL1_DC_TEST_SEL));

    return 1;
}

int apcpu_crm_clksel1_div_fb(int type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_DIV_FB_MASK << POS_APCPU_CLKSEL1_CPVR_SEL),
				(type<<POS_APCPU_CLKSEL1_DIV_FB));

    return 1;
}

int apcpu_crm_clksel1_ctr_dly(int type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_1BIT_MASK << POS_APCPU_CLKSEL1_CTR_DLY),
				(type<<POS_APCPU_CLKSEL1_CTR_DLY));

    return 1;
}

int apcpu_crm_clksel1_rst_soft(int type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_1BIT_MASK<<POS_APCPU_CLKSEL1_RST_SOFT),
				(type<<POS_APCPU_CLKSEL1_RST_SOFT));

    return 1;
}

int apcpu_crm_clksel1_div_front(int type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_4BIT_MASK<<POS_APCPU_CLKSEL1_DIV_FRONT),
				(type<<POS_APCPU_CLKSEL1_DIV_FRONT));

    return 1;
}


int apcpu_crm_clksel1_clkcpu_sel(apcpu_clk_sel1_e type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_4BIT_MASK<<POS_APCPU_CLKSEL1_CLK_CPU_SEL),
				(type<<POS_APCPU_CLKSEL1_CLK_CPU_SEL));

    return 1;
}

int apcpu_crm_clksel1_cal_en(int  type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_1BIT_MASK<<POS_APCPU_CLKSEL1_CAL_EN),
				(type<<POS_APCPU_CLKSEL1_CAL_EN));

    return 1;
}

int apcpu_crm_clksel1_ctr_i(int  type )
{
	crm_clk_sel(APCPU_CLKSEL1_REG, (BIT_APCPU_CLKSEL1_2BIT_MASK<<POS_APCPU_CLKSEL1_CTR_I),
				(type<<POS_APCPU_CLKSEL1_CTR_I));

    return 1;
}

int apcpu_crm_reset(apcpu_reset_e  type )
{
    if(apcpu_crm_locked(type+BIT_APCPU_REST_ADD_LOCK))
        return 0;

	crm_reset(APCPU_SRSTN_REG, (BIT_APCPU_RESET_MASK<<type));

    return 1;
}
//=======================================//


/*================DFT====================*/
void dft_crm_apcpu_mbist_en( dft_apcpu_mbist_en_e status)
{
   REG32(DFT_CFG_REG) &=BIT_DFT_APCPU_MBIST_EN_MASK;
   REG32(DFT_CFG_REG) |=status;
}

void dft_crm_mbist_mode( dft_mbist_mode_e status)
{
  REG32(DFT_CFG_REG) &=BIT_DFT_MBIST_MODE_MASK;
  REG32(DFT_CFG_REG) |=status;
}
//=======================================//


/*================Others====================*/
int vscpu_crm_clk_switch_mask(vscpu_clk_switch_mask_e status)
{
    REG32(IFCP_CLKSEL_REG) &=~(BIT_VSCPU_CLK_SWITCHMASK_MASK);
    REG32(IFCP_CLKSEL_REG)  |=(status<<POS_VSCPU_CLK_SWITCH);
    return 1;
}

int vscpu_sec_crm_reset(ifcp_srstn_e type)
{
    if ((crm_get_u32(SHAREREG_VSCPU_RST_CTRL_REG) & BIT_IFCP_SRSTN_LOCK) != 0)
        return 0;

    crm_reset(SHAREREG_VSCPU_RST_CTRL_REG, (BIT_SHAREREG_VSCPU_RST_CTRL_REG_MASK<<type));

    return 1;
}

 int ifcp_crm_reset_lock(crm_clken_status_e status)
{
    REG32(SHAREREG_VSCPU_RST_CTRL_REG) &=BIT_IFCP_SRSTN_MASK;
    REG32(SHAREREG_VSCPU_RST_CTRL_REG) |=(status<<POS_IFCP_SRSTN);
    return 1;
}

int exprst_crm_cfg_req(reg_rst_cfg_req_e status)
{
    crm_set_u32(EXPRST_CFG_REG, status);
    return 1;
}

int expwdog0_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG0_RSTTIMERS);
}

int expwdog1_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG1_RSTTIMERS);
}

int expwdog2_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG2_RSTTIMERS);
}

int expwdog3_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG3_RSTTIMERS);
}

int expwdog4_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG4_RSTTIMERS);
}

int expwdog5_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG5_RSTTIMERS);
}

int expwdog6_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG6_RSTTIMERS);
}

int expwdog7_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG7_RSTTIMERS);
}

int expwdog8_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG8_RSTTIMERS);
}

int expwdog9_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG9_RSTTIMERS);
}

int expwdog10_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG10_RSTTIMERS);
}

int expwdog11_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG11_RSTTIMERS);
}

int expwdog12_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG12_RSTTIMERS);
}

int expwdog13_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG13_RSTTIMERS);
}

int expwdog14_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG14_RSTTIMERS);
}

int expwdog15_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG15_RSTTIMERS);
}

int expwdog16_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG16_RSTTIMERS);
}

int expwdog17_crm_rsttimers(void)
{
    return crm_get_u32(EXPWDOG17_RSTTIMERS);
}

void sys_clk_crm_hw_sel(sys_clk_hw_sw_spdma_e  status)
{
    crm_set_u32(SYS_CLK_HW_EN, status);
}

void sys_clk_crm_sw_sel(sys_clk_hw_sw_spdma_e status)
{
    crm_set_u32(SYS_CLK_SW_EN, status);
}

void wdogexp_crm_cfg0_set(mt_u32 times)
{
    crm_set_u32(WDOGEXP_CFG0_REG, times);
}

 void wdogexp_crm_cfg1_set(mt_u32 times)
{
    crm_set_u32(WDOGEXP_CFG1_REG, times);
}

void rststat_crm_clr(mt_u32 status)
{
    crm_set_u32(RSTSTAT_CLR_REG, status);
}

void crm_avcpu_wdog_state_clr(mt_u32 status)
{
    crm_set_u32(RSTSTAT_CLR_REG, status);
}

mt_u8 rststat_crm_avcpu_wdog_state(void)
{
   return crm_get_u32(RSTSTAT_REG)>>16;
}

mt_u32 rststat_crm_reg(void)
{
   return (crm_get_u32(RSTSTAT_REG)&BIT_RSTSTATE_MASK);
}
/*=======================================*/

#endif




/*=======inteface  for modules operation,like reset  ,clk enable ,module clk set,
/========if the interface cant satify your requirement,
/========you can use the upper interface, like set,sel mode and so on,
/=========but the the meaning of the interface ,maybe you need to
/==========look up the reference datasheet ======*/

int crm_module_reset(mt_u32 mod_id)
{
    switch(mod_id)
    {
        case HAL_MAC:
            crm_mac_reset();
        break;
        case HAL_CPU1:
            crm_avcpu_clk_reset(AVCPUAHB_SRTSTN);
            crm_avcpu_clk_reset(AVCPU_AXI_SRSTN);
            crm_avcpu_clk_reset(AVCPUCORE_SRSTN);
        break;
        case HAL_DDRMC:
		crm_sys_reset(DDRMCAHB);
		crm_sys_reset(DDRMCAXI);
		crm_sys_reset(DDRMCCORE);
        break;
        case HAL_DMA:
		crm_sys_reset(DMAAXI|DMAAHB);
		crm_sys_reset(DMACORE);
        break;
	 case HAL_SMC0:
	 case HAL_SMC1:
		crm_intf_reset(INTF_SMC_RSTN);
		crm_intf_reset(INTF_SMCPHY_RSTN);
        break;

	 case HAL_USB0:
             crm_intf_reset(INTF_USB0AHB_RSTN);
		crm_intf_reset(INTF_USB0AXI_RSTN);
		crm_intf_reset(INTF_USB0CORE_RSTN);
		crm_intf_reset(INTF_USB0PHY_RSTN);
        break;

	 case HAL_USB1:
             crm_intf_reset(INTF_USB1AHB_RSTN);
		crm_intf_reset(INTF_USB1AXI_RSTN);
		crm_intf_reset(INTF_USB1CORE_RSTN);
		crm_intf_reset(INTF_USB1PHY_RSTN);
	 break;
	 case HAL_GPE:
		crm_gra_reset();
        break;
	 case HAL_JPEG:
		crm_jpg_ahb_reset();
		crm_jpg_axi_reset();
		crm_jpg_core_reset();
        break;
	 case HAL_WATCHDOG0:
		crm_sys_reset(SWWDOG0);
        break;

	 case HAL_WATCHDOG1:
		crm_sys_reset(SWWDOG1);
        break;

	 case HAL_MNT:
		crm_sys_reset(SWNMTXTAL);
		crm_sys_reset(SWMNTSOSC);
		crm_sys_reset(SWMNTMISC);
        break;

	 case HAL_SDIO0:
        {
		crm_intf_reset(INTF_SDIO0AHB_RSTN);
		crm_intf_reset(INTF_SDIO0AXI_RSTN);
		crm_intf_reset(INTF_SDIO0CORE_RSTN);
        }
        break;

	 case HAL_SDIO1:
        {
		crm_intf_reset(INTF_SDIO1AHB_RSTN);
		crm_intf_reset(INTF_SDIO1AXI_RSTN);
		crm_intf_reset(INTF_SDIO1CORE_RSTN);
        }
        break;

        case HAL_PNAND:
        {
		crm_intf_reset(INTF_PNANDAHB_RSTN);
		crm_intf_reset(INTF_PNANDAXI_RSTN);
		crm_intf_reset(INTF_PNANDCORE_RSTN);
		crm_intf_reset(INTF_PNANDREG_RSTN);
        }
        break;

        case HAL_SPI0:
        {
		crm_intf_reset(INTF_SPI0AHB_RSTN);
		crm_intf_reset(INTF_SPI0CORE_RSTN);
		crm_intf_reset(INTF_SPI0REG_RSTN);
        }
        break;

        case HAL_SPI1:
        {
		crm_intf_reset(INTF_SPI1AHB_RSTN);
		crm_intf_reset(INTF_SPI1AXI_RSTN);
		crm_intf_reset(INTF_SPI1CORE_RSTN);
		crm_intf_reset(INTF_SPI1REG_RSTN);
        }
        break;

        case HAL_UART0:
        {
		crm_intf_reset(INTF_UART0_RSTN);
        }
        break;

        case HAL_UART1:
        {
		crm_intf_reset(INTF_UART1_RSTN);
        }
        break;


	 case HAL_I2C:
	 {
		crm_intf_reset(INTF_I2C0_RSTN);
		crm_intf_reset(INTF_I2C1_RSTN);
	 }
	 break;

        case HAL_I2C0:
        {
		crm_intf_reset(INTF_I2C0_RSTN);
        }
        break;

        case HAL_I2C1:
        {
		crm_intf_reset(INTF_I2C1_RSTN);
        }
        break;
        case HAL_SPDMA:
            crm_avcpu_clk_reset(SPDMAAHB_SRSTN);
            crm_avcpu_clk_reset(SPDMAAXI_SRSTN);
            crm_avcpu_clk_reset(SPDMACORE_SRSTN);
        break;


	 case HAL_TIMER0:
        {
		crm_sys_reset(SWTIMER0);
        }
        break;

	 case HAL_TIMER1:
        {
		crm_sys_reset(SWTIMER1);
        }
        break;

	 case HAL_TIMER2:
        {
		crm_sys_reset(SWTIMER2);
        }
        break;

	 case HAL_TIMER3:
        {
		crm_sys_reset(SWTIMER3);
        }
        break;




        case HAL_DISPLAY:
       {
		crm_disp_core_reset();
        }
        break;

        case HAL_DI:
       {
  		crm_disp_di_reset();
        }
        break;

	case HAL_OSDC:
       {
 		crm_disp_osdc_reset();
        }
        break;

	 case HAL_VDEC:
        {
  		crm_vdec_core_core_reset();
		crm_vdec_core_axi_reset();
		crm_vdec_reset();
 	 }
        break;

        case HAL_AUDIO_OUT:
        {
		crm_aout_adac_reset();
		crm_aout_mclk_reset();
		crm_aout_core_reset();

        }
        break;

        case HAL_VBI:
        {
		crm_vout_reset(VOUT_SRSTN_VBI);
        }
        break;


        case HAL_HDMI:
        {
		crm_vout_reset(VOUT_SRSTN_HDMI_AHB);
		crm_vout_reset(VOUT_SRSTN_HDMI_CORE);
        }
        break;

        case HAL_LCD:
        {
		crm_vout_reset(VOUT_SRSTN_LCD_AHB);
		crm_vout_reset(VOUT_SRSTN_LCD_CORE);
        }
        break;

        case HAL_HD_VIDEO:
        {
		crm_vout_reset(VOUT_SRSTN_HDVENC_AHB);
		crm_vout_reset(VOUT_SRSTN_HDVENC_HD);
        }
        break;

        case HAL_SD_VIDEO:
        {
		crm_vout_reset(VOUT_SRSTN_SDVENC_CORE);
		crm_vout_reset(VOUT_SRSTN_SDVENC_AHB);
        }
        break;

        case HAL_PNG:
        {
		crm_png_core_reset();
        }
        break;

        case HAL_DAI:
        {
		crm_dai_reset(DAI_SRSTN_AHB);
		crm_dai_reset(DAI_SRSTN_AXI);
		crm_dai_reset(DAI_SRSTN_TX);
		crm_dai_reset(DAI_SRSTN_RX);
		crm_dai_reset(DAI_SRSTN_PDM);
		crm_dai_reset(DAI_SRSTN_DAC);
		crm_dai_reset(DAI_SRSTN_ADC);
        }
        break;

        case HAL_TSI:
        {
		crm_tsi_reset(TSI_SRSTN_AHB);
		crm_tsi_reset(TSI_SRSTN_AXI);
		crm_tsi_reset(TSI_SRSTN_CORE);
        }
        break;


        case HAL_TS0:
        {
		crm_tsi_reset(TSI_SRSTN_TS0);
        }
        break;

        case HAL_TS1:
        {
		crm_tsi_reset(TSI_SRSTN_TS1);
        }
        break;

        case HAL_TS2:
        {
		crm_tsi_reset(TSI_SRSTN_TS2);
        }
        break;

        case HAL_TS3:
        {
		crm_tsi_reset(TSI_SRSTN_TS3);
        }
        break;

        case HAL_DEMUX:
        {
		crm_tsi_reset(TSI_SRSTN_DEMUX);
        }
        break;

	 case HAL_T2MI:
        {
		crm_tsi_reset(TSI_SRSTN_T2MI);
        }
        break;

	 case HAL_TSI_AVSYNC:
        {
		crm_tsi_reset(TSI_SRSTN_AVSYNC_SYS);
		crm_tsi_reset(TSI_SRSTN_AVSYNC_PCR);
		crm_tsi_reset(TSI_SRSTN_AVSYNC_AHB);
        }
        break;

	 case HAL_TSI_SF:
        {
		crm_tsi_reset(TSI_SRSTN_SF);
		crm_tsi_reset(TSI_SRSTN_SF_AXI);
		crm_tsi_reset(TSI_SRSTN_SF_REG);
        }
        break;

	 case HAL_TSI_TRPP:
        {
		crm_tsi_reset(TSI_SRSTN_TRPP);
		crm_tsi_reset(TSI_SRSTN_TRPP_REG);
		crm_tsi_reset(TSI_SRSTN_TRPP_AXI);
        }
        break;

	 case HAL_TSI_SWTSI:
        {
		crm_tsi_reset(TSI_SRSTN_SWTSI);
		crm_tsi_reset(TSI_SRSTN_SWTSI_AXI);
        }
        break;

	 case HAL_TSI_TSPOOL:
        {
		crm_tsi_reset(TSI_SRSTN_TSPOOL);
        }
        break;

	 case HAL_CI:
        {
		crm_ci_ahb_reset();
		crm_ci_core_reset();
	 }
        break;

        case HAL_GLITCH_DET:
        {
		crm_secure_reset(SECURE_SRSTN_GLITCH_DET);
        }
        break;


        case HAL_SECHD0:
        {
		crm_secure_reset(SECURE_SRSTN_SECHD0);
        }
        break;

        case HAL_KT:
        {
		crm_secure_reset(SECURE_SRSTN_KT_CORE);
		crm_secure_reset(SECURE_SRSTN_KT_IBUS);
        }
        break;

        case HAL_M2M:
        {
		crm_secure_reset(SECURE_SRSTN_M2M_CORE);
		crm_secure_reset(SECURE_SRSTN_M2M_AXI);
		crm_secure_reset(SECURE_SRSTN_M2M_AHB);
		crm_secure_reset(SECURE_SRSTN_M2M_CIPHER);
        }
        break;

        case HAL_PKA:
        {
		crm_secure_reset(SECURE_SRSTN_PKA_CORE);
		crm_secure_reset(SECURE_SRSTN_PKA_IBUS);
        }
        break;

        case HAL_DS:
        {
		crm_secure_reset(SECURE_SRSTN_DS);
        }
        break;

        case HAL_KL_CW:
        {
		crm_secure_reset(SECURE_SRSTN_KL);
        }
        break;
#if 0
        case HAL_IFCP_GLB:
        {
		ifcp_crm_reset();
        }
        break;
#endif
        case HAL_AO_LPMSET:
        {
		crm_ao_lpm_reset();
        }
        break;

        case HAL_AO_IRDA:
        {
		crm_ao_irda_reset();
        }
        break;

        case HAL_AO_RTC:
        {
		crm_ao_rtc_reset();
	 }
        break;

        case HAL_AO_LEDKB:
        {
		crm_ao_ledkb_reset();
        }
        break;

        case HAL_AO_GPIO:
        {
		crm_ao_gpio_reset();
        }
        break;

        case HAL_AO_KADC:
        {
		crm_ao_kadc_reset();
        }
        break;

        case HAL_AO_FPI2C:
        {
		crm_ao_fpi2c_reset();
        }
        break;

        case HAL_AO_FPSPI:
        {
		crm_ao_fpspi_reset();
        }
        break;

        case HAL_AO_FPSPIREG:
        {
		crm_ao_fpspireg_reset();
        }
        break;

        case HAL_AO_PINMUX:
        {
		crm_ao_pinmux_reset();
        }
        break;

        case HAL_AO_TIMER0:
        case HAL_AO_TIMER1:
        {
		crm_ao_timer_reset();
        }
        break;

        case HAL_AO_MNT:
        {
		crm_ao_mntsosc_reset();
		crm_ao_mntxtal_reset();
        }
        break;

        case HAL_AO_MAILBOX:
        {
		crm_ao_mailbox_reset();
        }
        break;

        case HAL_AO_CEC:
        {
		crm_ao_cec_reset();
        }
        break;

        case HAL_AO_AVS:
        {
		crm_ao_avs_reset();
        }
        break;

        case HAL_AO_AGTIMER:
        {
		crm_ao_agtimer_reset();
	 }
        break;

        case HAL_AO_WDOG:
        {
		crm_ao_wdog_reset();
	 }
        break;

        default:
            return -1;
    }

    return 0;
}

int crm_module_clk_enable(mt_u32 mod_id, MT_BOOL onoff)
{
    int ret = 0;
    switch(mod_id)
    {
        case HAL_MAC:
            ret = crm_mac_clk_en(onoff);
            break;
        case HAL_MAC_RMII:
            ret = crm_rmii_clk_en(onoff);
            break;
        case HAL_AXI:
            ret = crm_bus_axi_clk_en(onoff);
            break;
        case HAL_AXI_DEBUG:
            ret = crm_bus_axi_debug_clk_en(onoff);
            break;
        case HAL_AXI_REG:
            ret = crm_bus_axi_reg_clk_en(onoff);
            break;
        case HAL_CPU1:
            ret = crm_avcpu_clken(onoff);
            break;
        case HAL_DMA:
            ret = crm_sys_clken(DMA_CLKEN, onoff);
            break;
        case HAL_SMC:
            ret = crm_intf_clken(SMC_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_USB0:
            ret = crm_intf_clken(USB0_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_USB1:
            ret = crm_intf_clken(USB1_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_I2C0:
            ret = crm_intf_clken(I2C0_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_I2C1:
            ret = crm_intf_clken(I2C1_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_I2C_DEBUG:
            ret = crm_intf_clken(I2CDEBUG_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_SPI0:
            ret = crm_intf_clken(SPI0_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_SPI1:
            ret = crm_intf_clken(SPI1_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_PNAND:
            ret = crm_intf_clken(PNAND_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_SDIO0:
            ret = crm_intf_clken(SDIO0_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_SDIO1:
            ret = crm_intf_clken(SDIO1_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_UART0:
            ret = crm_intf_clken(UART0_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_UART1:
            ret = crm_intf_clken(UART1_CLKEN, onoff ? INTF_CLK_OPEN : INTF_CLK_CLOSED);
            break;
        case HAL_GPE:
            ret = crm_gra_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_JPEG:
            ret = crm_jpg_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_DISPLAY:
            ret = crm_disp_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_VDEC:
            ret = crm_vdec_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AUDIO_OUT:
            ret = crm_aout_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_SPDF:
            ret = crm_aout_spdf_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_HDMI:
            ret = crm_vout_hdmi_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_LCD:
            ret = crm_vout_lcd_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_HD_VIDEO:
            ret = crm_vout_hdvenc_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_VBI:
            ret = crm_vout_vbi_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_SD_VIDEO:
            ret = crm_vout_sdvenc_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_PNG:
            ret = crm_png_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_DAI:
            ret = crm_dai_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_TSI:
            ret = crm_tsi_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_CI:
            ret = crm_ci_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_M2M:
            ret = crm_secure_clken(SECURE_M2M_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_PKA:
            ret = crm_secure_clken(SECURE_PKA_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_GLITCH_DET:
            ret = crm_secure_clken(SECURE_GLITCHDET_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_KT:
            ret = crm_secure_clken(SECURE_KT_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_SECHD0:
            ret = crm_secure_clken(SECURE_SECHD0_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_KL_CW:
            ret = crm_secure_clken(SECURE_KL_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_KDF:
            ret = crm_secure_clken(SECURE_KDF_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_SECURE:
            ret = crm_secure_clken(SECURE_SECMISC_CLKEN, onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_IFCP_KLM:
            ret = crm_ifcp_klm_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_IFCP_CRYPTO:
        case HAL_IFCP_SYS:
            ret = crm_ifcp_crypto_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
#if 0
        case HAL_DDRMC:
            crm_sys_clken(DDRMCAXI_CLKEN, onoff);
            crm_sys_clken(DDRMCAHB_CLKEN, onoff);
            crm_sys_clken(DDRMCCORE_CLKEN, onoff);
            crm_sys_clken(DDRMCREGS_CLKEN, onoff);
            break;
        case HAL_IFCP_CRYPTO:
            ifcp_crm_crypto_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_IFCP_KLM:
            ifcp_crm_klm_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
#endif
        case HAL_AO_LPMSET:
            ret = crm_ao_lpm_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_IRDA:
            ret = crm_ao_irda_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_LEDKB:
            ret = crm_ao_ledkb_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_GPIO:
            ret = crm_ao_gpio_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_KADC:
            ret = crm_ao_kadc_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_ANA:
            ret = crm_ao_ana_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_FPI2C:
            ret = crm_ao_fpi2c_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_FPSPI:
            ret = crm_ao_fpspi_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_SECRAM:
            ret = crm_ao_recram_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_PINMUX:
            ret = crm_ao_pinmux_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_TIMER0:
        case HAL_AO_TIMER1:
            ret = crm_ao_timer_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_MAILBOX:
            ret = crm_ao_mailbox_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_CEC:
            ret = crm_ao_cec_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_AVS:
            ret = crm_ao_avs_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_AGTIMER:
            ret = crm_ao_agtimer_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_AO_MCU:
            ret = crm_ao_aomcu_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_RNG:
            crm_reg_set_valid_bit(REG_CLK_ANALOG, 24, 1, onoff ? CLKEN_OFF:CLKEN_ON);
            break;
        case HAL_RNG2:
            crm_reg_set_valid_bit(REG_CLK_ANALOG, 29, 1, onoff ? CLKEN_OFF:CLKEN_ON);
            break;
        case HAL_PANTHER2:
            crm_panther2_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_SADC:
            crm_reg_set_valid_bit(REG_CLK_ANALOG, 21, 1, onoff ? CLKEN_OFF:CLKEN_ON);
            break;
        case HAL_CADC:
            crm_reg_set_valid_bit(REG_CLK_ANALOG, 20, 1, onoff ? CLKEN_OFF:CLKEN_ON);
            break;
        case HAL_DEMO_C:
             crm_reg_set_valid_bit(REG_CLK_DEMOD, 0, 1, onoff?CLKEN_ON:CLKEN_OFF);
            break;
        case HAL_DEMO_S:
             crm_reg_set_valid_bit(REG_CLK_DEMOD, 3, 1, onoff?CLKEN_ON:CLKEN_OFF);
            break;
        case HAL_DEMO_J83B:
             crm_reg_set_valid_bit(REG_CLK_DEMOD, 2, 1, onoff?CLKEN_ON:CLKEN_OFF);
            break;
        case HAL_TSENSOR:
            crm_reg_set_valid_bit(REG_CLK_ANALOG, 27, 1, onoff?0:1);
            break;
        default:
            return -1;
    }

    return ret;
}

int crm_module_clk_is_enabled(mt_u32 mod_id)
{
    switch(mod_id)
    {
        case HAL_MAC:
            return crm_mac_clk_enabled();
            break;
        case HAL_MAC_RMII:
            return crm_rmii_clk_enabled();
            break;
        case HAL_AXI:
            return crm_bus_axi_clk_enabled();
            break;
        case HAL_AXI_DEBUG:
            return crm_bus_axi_debug_clk_enabled();
            break;
        case HAL_AXI_REG:
            return crm_bus_axi_reg_clk_enabled();
            break;
        case HAL_CPU1:
            return crm_avcpu_clk_enabled();
            break;
        case HAL_DMA:
            return crm_intf_clk_enabled(DMA_CLKEN);
            break;
        case HAL_SMC:
            return crm_intf_clk_enabled(SMC_CLKEN);
            break;
        case HAL_USB0:
            return crm_intf_clk_enabled(USB0_CLKEN);
            break;
        case HAL_USB1:
            return crm_intf_clk_enabled(USB1_CLKEN);
            break;
        case HAL_I2C0:
            return crm_intf_clk_enabled(I2C0_CLKEN);
            break;
        case HAL_I2C1:
            return crm_intf_clk_enabled(I2C1_CLKEN);
            break;
        case HAL_I2C_DEBUG:
            return crm_intf_clk_enabled(I2CDEBUG_CLKEN);
            break;
        case HAL_SPI0:
           return  crm_intf_clk_enabled(SPI0_CLKEN);
            break;
        case HAL_SPI1:
            return crm_intf_clk_enabled(SPI1_CLKEN);
            break;
        case HAL_PNAND:
            return crm_intf_clk_enabled(PNAND_CLKEN);
            break;
        case HAL_SDIO0:
            return crm_intf_clk_enabled(SDIO0_CLKEN);
            break;
        case HAL_SDIO1:
            return crm_intf_clk_enabled(SDIO1_CLKEN);
            break;
        case HAL_UART0:
            return crm_intf_clk_enabled(UART0_CLKEN);
            break;
        case HAL_UART1:
            return crm_intf_clk_enabled(UART1_CLKEN);
            break;
        case HAL_GPE:
            return crm_gra_clk_enabled();
            break;
        case HAL_JPEG:
            return crm_jpg_clk_enabled();
            break;
        case HAL_DISPLAY:
            return crm_disp_clk_enabled();
            break;
        case HAL_VDEC:
            return crm_vdec_clk_enabled();
            break;
        case HAL_AUDIO_OUT:
            return crm_aout_clk_enabled();
            break;
        case HAL_SPDF:
            return crm_aout_spdf_clk_enabled();
            break;
        case HAL_HDMI:
            return crm_vout_hdmi_clk_enabled();
            break;
        case HAL_LCD:
            return crm_vout_lcd_clk_enabled();
            break;
        case HAL_HD_VIDEO:
            return crm_vout_hdvenc_clk_enabled();
            break;
        case HAL_VBI:
            return crm_vout_vbi_clk_enabled();
            break;
        case HAL_SD_VIDEO:
            return crm_vout_sdvenc_clk_enabled();
            break;
        case HAL_PNG:
            return crm_png_clk_enabled();
            break;
        case HAL_DAI:
            return crm_dai_clk_enabled();
            break;
        case HAL_TSI:
            return crm_tsi_clk_enabled();
            break;
        case HAL_CI:
            return crm_ci_clk_enabled();
            break;
        case HAL_M2M:
            return crm_secure_clk_enabled(SECURE_M2M_CLKEN);
            break;
        case HAL_PKA:
            return crm_secure_clk_enabled(SECURE_PKA_CLKEN);
            break;
        case HAL_GLITCH_DET:
            return crm_secure_clk_enabled(SECURE_GLITCHDET_CLKEN);
            break;
        case HAL_KT:
            return crm_secure_clk_enabled(SECURE_KT_CLKEN);
            break;
        case HAL_SECHD0:
            return crm_secure_clk_enabled(SECURE_SECHD0_CLKEN);
            break;
        case HAL_KL_CW:
            return crm_secure_clk_enabled(SECURE_KL_CLKEN);
            break;
        case HAL_KDF:
            return crm_secure_clk_enabled(SECURE_KDF_CLKEN);
            break;
        case HAL_SECURE:
            return crm_secure_clk_enabled(SECURE_SECMISC_CLKEN);
            break;
        case HAL_IFCP_KLM:
            return crm_ifcp_klm_clk_enabled();
            break;
        case HAL_IFCP_CRYPTO:
        case HAL_IFCP_SYS:
            return crm_ifcp_crypto_clk_enabled();
            break;
#if 0
        case HAL_DDRMC:
            crm_sys_clken(DDRMCAXI_CLKEN, onoff);
            crm_sys_clken(DDRMCAHB_CLKEN, onoff);
            crm_sys_clken(DDRMCCORE_CLKEN, onoff);
            crm_sys_clken(DDRMCREGS_CLKEN, onoff);
            break;
        case HAL_IFCP_CRYPTO:
            ifcp_crm_crypto_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
        case HAL_IFCP_KLM:
            ifcp_crm_klm_clken(onoff ? CLKEN_ON : CLKEN_OFF);
            break;
#endif
        case HAL_AO_LPMSET:
            return crm_ao_lpm_clk_enabled();
            break;
        case HAL_AO_IRDA:
            return crm_ao_irda_clk_enabled();
            break;
        case HAL_AO_LEDKB:
            return crm_ao_ledkb_clk_enabled();
            break;
        case HAL_AO_GPIO:
            return crm_ao_gpio_clk_enabled();
            break;
        case HAL_AO_KADC:
            return crm_ao_kadc_clk_enabled();
            break;
        case HAL_AO_ANA:
            return crm_ao_ana_clk_enabled();
            break;
        case HAL_AO_FPI2C:
            return crm_ao_fpi2c_clk_enabled();
            break;
        case HAL_AO_FPSPI:
            return crm_ao_fpspi_clk_enabled();
            break;
        case HAL_AO_RECRAM:
            return crm_ao_recram_clk_enabled();
            break;
        case HAL_AO_PINMUX:
            return crm_ao_pinmux_clk_enabled();
            break;
        case HAL_AO_TIMER0:
        case HAL_AO_TIMER1:
            return crm_ao_timer_clk_enabled();
            break;
        case HAL_AO_MAILBOX:
            return crm_ao_mailbox_clk_enabled();
            break;
        case HAL_AO_CEC:
            return crm_ao_cec_clk_enabled();
            break;
        case HAL_AO_AVS:
            return crm_ao_avs_clk_enabled();
            break;
        case HAL_AO_AGTIMER:
            return crm_ao_agtimer_clk_enabled();
            break;
        case HAL_AO_MCU:
            return crm_ao_aomcu_clk_enabled();
            break;
        case HAL_RNG:
            return !(crm_get_u32(REG_CLK_ANALOG) >> 24) & 0x1;
            break;
        case HAL_RNG2:
            return !(crm_get_u32(REG_CLK_ANALOG) >> 29) & 0x1;
            break;
        case HAL_TSENSOR:
            return !(crm_get_u32(REG_CLK_ANALOG) >> 27) & 0x1;
            break;
        case HAL_SADC:
            return !(crm_get_u32(REG_CLK_ANALOG) >> 21) & 0x1;
            break;
        case HAL_CADC:
            return !(crm_get_u32(REG_CLK_ANALOG) >> 20) & 0x1;
            break;
        case HAL_PANTHER2:
            return crm_panther2_clk_enabled();
            break;
        case HAL_DEMO_C:
             return (crm_get_u32(REG_CLK_DEMOD) >> 0) & 0x1;
            break;
        case HAL_DEMO_S:
            return (crm_get_u32(REG_CLK_DEMOD) >> 3) & 0x1;
            break;
        case HAL_DEMO_J83B:
            return (crm_get_u32(REG_CLK_DEMOD) >> 2) & 0x1;
            break;

        default:
            return -1;
    }

    return 0;
}

int crm_module_clk_set(mt_u32 m_id, mt_u32 type)
{
    switch(m_id)
    {
        case HAL_MAC:
            crm_mac_clk_sel(type);
            break;
        case HAL_HB:
            crm_bus_ahb_clk_sel(type);
            break;
        case HAL_PB:
            crm_bus_apb_clk_sel(type);
            break;
        case HAL_CPU1:
            crm_avcpu_clk_sel((avcpu_clksel_e)type);
            break;
        case HAL_DMA:
            crm_sys_dma_clksel((sys_dma_clksel_e)type);
            break;
        case HAL_SMC:
            crm_intf_smc_phyclksel((intf_smc_phyclksel_e)type);
            break;
        case HAL_SPI0:
            crm_intf_spi0_clksel((intf_spi_clksel_e)type);
            break;
        case HAL_SPI1:
            crm_intf_spi1_clksel((intf_spi_clksel_e)type);
            break;
        case HAL_PNAND:
            crm_intf_pnand_clksel((intf_pnand_clksel_e)type);
            break;
        case HAL_SDIO0:
            crm_intf_sdio0_clksel((intf_sdio_clksel_e)type);
            break;
        case HAL_SDIO1:
            crm_intf_sdio1_clksel((intf_sdio_clksel_e)type);
            break;
        case HAL_UART0:
            crm_intf_uart0phy_clksel((intf_uartphy_clksel_e)type);
            break;
        case HAL_UART1:
            crm_intf_uart1phy_clksel((intf_uartphy_clksel_e)type);
            break;
        case HAL_GPE:
            crm_gra_clksel((gra_clksel_e)type);
            break;
        case HAL_JPEG:
            crm_jpg_clksel((jpg_clksel_e)type);
            break;
        case HAL_DISPLAY:
            crm_disp_core_clksel((dispcore_clksel_e)type);
            break;
        case HAL_DI:
            crm_disp_di_clksel((dispdi_clksel_e)type);
            break;
        case HAL_OSDC:
            crm_disp_osdc_clksel((disposdc_clksel_e)type);
            break;
        case HAL_VDEC:
            crm_vdec_clksel((vdec_clksel_e)type);
            break;
        case HAL_AUDIO_OUT:
            crm_aout_clksel((aout_clksel_e)type);
            break;
        case HAL_AUDIO_MCLK:
            crm_aout_mclk_clksel((aout_mclk_clksel_e)type);
            break;
        case HAL_LCDC:
            crm_vout_lcdc_clksel((vout_lcdc_clksel_e)type);
            break;
        case HAL_LCDHD:
            crm_vout_lcdhd_clksel((vout_lcdhd_clksel_e)type);
            break;
        case HAL_LCD2X:
            crm_vout_lcd2x_clksel((vout_lcd2x_clksel_e)type);
            break;
        case HAL_HD_VIDEO: //need to fix ,related to analog
            crm_vout_hdvenc_clksel((vout_hdvenc_clksel_e)type);
            break;
        case HAL_PNG:
            crm_png_clksel((png_clksel_e)type);
            break;
        case HAL_TSI:
            crm_tsi_clksel((tsi_clksel_e)type);
            break;
        case HAL_TS0:
            crm_tsi_ts0_clksel((ts0_clksel_mode_e)type);
            break;
        case HAL_TS1:
            crm_tsi_ts1_clksel((ts1_clksel_mode_e)type);
            break;
        case HAL_TS2:
            crm_tsi_ts2_clksel((ts2_clksel_mode_e)type);
            break;
        case HAL_TS3:
            crm_tsi_ts3_clksel((ts3_clksel_mode_e)type);
            break;
        case HAL_CITSIN:
            crm_ci_tsin_clksel((citsin_clksel_mode_e)type);
            break;
        case HAL_CI:
            crm_ci_clksel((ci_clksel_e)type);
            break;
        case HAL_XTAL_MODE:
            crm_ao_xtal_clksel((crm_xtal_sel_e)type);
            break;
        case HAL_M2M:
            crm_secure_m2m_cipher_clksel((secure_m2m_chiper_clksel_e)type);
            break;
        case HAL_SECURE:
            crm_secure_clksel((secure_clksel_e)type);
            break;
      case HAL_IFCP_SYS:
            crm_ifcp_sys_clksel((ifcp_sys_clksel_e)type);
            break;
      case HAL_IFCP_CRYPTO:
            crm_ifcp_crypto_clksel((ifcp_crypto_clksel_e)type);
            break;
#if 0
        case HAL_DDRMC:
        crm_sys_ddrpll_pnclk_clksel((sys_ddr_pll_clk_e)type);
        break;
        case HAL_IFCP_CRYPTO:
        ifcp_crm_crypto_clksel((ifcp_crypto_clksel_e)type);
        break;

        case HAL_IFCP_SYS:
        ifcp_crm_sys_clksel((ifcp_sys_clksel_e)type);
        break;
#endif
        default:
            return -1;
    }
  return 0;
}

int crm_module_clk_get(mt_u32 m_id)
{
  unsigned long freq = 0;

  switch(m_id)
  {
       case HAL_XTAL:
            freq =  crm_xtal_clk_get();
            break;
      case HAL_MAC:
      case HAL_MAC_RMII:
            freq = crm_mac_clk_get(m_id);
            break;
      case HAL_AXI:
            freq = crm_bus_axi_clk_get();
            break;
      case HAL_HB:
            freq = crm_bus_ahb_clk_get();
            break;
      case HAL_PB:
            freq = crm_bus_apb_clk_get();
            break;
      case HAL_CPU1:
            freq = crm_avcpu_clk_get();
            break;
      case HAL_DMA:
            freq = crm_sys_dma_clk_get();
            break;
      case HAL_SMC:
            freq = crm_intf_smc_phyclk_get();
            break;
      case HAL_SPI0:
            freq = crm_intf_spi_clk_get(0);
            break;
      case HAL_SPI1:
            freq = crm_intf_spi_clk_get(1);
            break;
      case HAL_PNAND:
            freq = crm_intf_pnand_clk_get();
            break;
      case HAL_SDIO0:
            freq = crm_intf_sdio_clk_get(0);
            break;
      case HAL_SDIO1:
            freq = crm_intf_sdio_clk_get(1);
            break;
      case HAL_UART0:
            freq = crm_intf_uart_clk_get(0);
            break;
      case HAL_UART1:
            freq = crm_intf_uart_clk_get(1);
            break;
      case HAL_GPE:
            freq = crm_gra_clk_get();
            break;
      case HAL_JPEG:
            freq = crm_jpg_clk_get();
            break;
      case HAL_DISPLAY:
            freq = crm_disp_core_clk_get();
            break;
      case HAL_DI:
            freq = crm_disp_di_clk_get();
            break;
      case HAL_OSDC:
            freq = crm_disp_osdc_clk_get();
            break;
      case HAL_VDEC:
            freq = crm_vdec_clk_get();
            break;
      case HAL_AUDIO_OUT:
            freq = crm_aout_clk_get();
            break;
      case HAL_AUDIO_MCLK:
            freq = crm_aout_mclk_get();
            break;
      case HAL_HD_VIDEO:
            freq = crm_vout_hdvenc_clk_get();
            break;
      case HAL_LCD2X:
            freq = crm_vout_lcd2x_clk_get();
            break;
      case HAL_LCDHD:
            freq = crm_vout_lcdhd_clk_get();
            break;
      case HAL_LCDC:
            freq = crm_vout_lcdc_clk_get();
            break;
      case HAL_PNG:
            freq = crm_png_clk_get();
            break;
      case HAL_TSI:
            freq = crm_tsi_clk_get();
            break;
      case HAL_TS0:
            freq = crm_tsi_ts0_clk_get();
            break;
      case HAL_TS1:
            freq = crm_tsi_ts1_clk_get();
            break;
      case HAL_TS2:
            freq = crm_tsi_ts2_clk_get();
            break;
      case HAL_TS3:
            freq = crm_tsi_ts3_clk_get();
            break;
      case HAL_CITSIN:
            freq = crm_ci_tsin_clk_get();
            break;
      case HAL_CI:
            freq = crm_ci_clk_get();
            break;
        case HAL_XTAL_MODE:
            freq = crm_ao_xtal_clk_mode_get();
            break;
        case HAL_M2M:
            freq = crm_secure_m2m_cipher_clk_get();
            break;
      case HAL_SECURE:
            freq = crm_secure_clk_get();
            break;
      case HAL_IFCP_SYS:
            freq = crm_ifcp_sys_clk_get();
            break;
      case HAL_IFCP_CRYPTO:
            freq = crm_ifcp_crypto_clk_get();
            break;
      default:
      return -1;
  }
  if(freq > 10) // number less than 10 for mode type
      freq *= 1000000;
   return freq;
}


void crm_analog_config(void)
{
   u32 val = 0;
   int ret = 0;
   u8 package_type = 0;
   unsigned long chip_ver = symphony_get_chip_rev();
   otp_open(NULL, NULL);
   ret = sys_otp_read(0x3FD4,4, (u32 *)&val);
   if(ret == MT_SUCCESS)
       package_type=(val >> 7) & 0x1;
   //ADAC 0dbu
   if(package_type == 1)
   {
       /* all analog on */
       crm_reg_set_valid_bit(REG_CLK_ANALOG, 12, 8, 0x55);
       //crm_reg_set_valid_bit(REG_CLK_ANALOG, 0, 32, 0x55000); /* keep all module open for function test */
       crm_reg_set_valid_bit(R_BIAS_REG, 0, 3, 0x3);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 9, 1, 0x0);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 6, 1, 0x1);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 4, 2, 0x1);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 10, 2, 0x0);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 0, 2, 0x2);

   }
   else //adac 2Vrms
   {
       /* all analog on */
       crm_reg_set_valid_bit(REG_CLK_ANALOG, 12, 8, 0x66);
       //crm_reg_set_valid_bit(REG_CLK_ANALOG, 0, 32, 0x66000); /* keep all module open for function test */
       /* analog vdd1p5-50mV only in A0B */
       if(chip_ver == CHIP_SYMPHONY4_A0)
           crm_reg_set_valid_bit(R_BIAS_REG, 3, 3, 0x5);
       /* adac output enable */
       crm_reg_set_valid_bit(R_BIAS_REG, 0, 3, 0x3);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 9, 1, 0x0);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 6, 1, 0x1);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 4, 2, 0x1);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 10, 2, 0x1);
       crm_reg_set_valid_bit(R_ADAC_SW_REG0, 0, 2, 0x2);//adac start up
   }
       /* ephy on and reset */
#ifdef CONFIG_MT_SYMPHONY4_ETH
       crm_reg_set_valid_bit(R_CLKGEN_EPHY_REG, 25, 1, 0x1);
       mdelay(2);
       crm_reg_set_valid_bit(R_CLKGEN_EPHY_REG, 7, 1, 0x1);
       udelay(10);
       crm_reg_set_valid_bit(R_CLKGEN_EPHY_REG, 7, 1, 0x0);
       mdelay(2);
#endif

       crm_reg_set_valid_bit(R_VDAC_SW_REG2, 1, 1, 0x0);
       udelay(10);
       crm_reg_set_valid_bit(R_VDAC_SW_REG2, 1, 1, 0x1);

       /* enable drv0/drv1 */
       //REG32(0xbf5d009c) = 0;
       //crm_set_u32(R_CLKGEN_PDSYS_SW_REG, 0);
       //crm_reg_set_valid_bit(R_CLKGEN_CPUPLL_SW_REG0, 4, 4, 0x0);
       /* reduce chopping spur */
       crm_reg_set_valid_bit(R_CLKGEN_TEST_SW_REG, 28, 2, 0x0);
       /* cadc clock inv phase */
       crm_reg_set_valid_bit(R_CADC_REG0, 19, 1, 0x0);
}
#if 0
static void crm_demod_clk_enable(void)
{
    crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 18, 1, 0x1);
    udelay(2);

    crm_reg_set_valid_bit(BUS_CLKSEL_REG, 6, 2, 0x3);
    crm_set_u32(BUS_CLKEN_REG, 0x0);
    udelay(2);
    crm_reg_set_valid_bit(TOPCLK_CTRL6_REG, 24, 1, 0x1);
    udelay(2);
    crm_reg_set_valid_bit(BUS_CLKSEL_REG, 6, 2, 0x0);
    crm_set_u32(BUS_CLKEN_REG, 0x7);
}
#endif
void crm_module_clk_resume(void)
{
    //crm_demod_clk_enable();

#ifdef CONFIG_MT_SYMPHONY4_ETH
    /* mac */
    crm_module_clk_enable(HAL_MAC, TRUE);
    crm_module_reset(HAL_MAC);
#endif

    /* dma */
    crm_module_clk_enable(HAL_DMA, TRUE);
    /* av cpu */
    crm_module_clk_enable(HAL_CPU1, TRUE);
   // crm_module_reset(HAL_CPU1);

    /* interface */
#ifdef CONFIG_MMC_SDHCI_MT
    crm_module_clk_set(HAL_SDIO0, SDIO_CLKSEL_90M);
    crm_module_clk_set(HAL_SDIO1, SDIO_CLKSEL_90M);
#endif

#ifdef CONFIG_MTD_MT_SYMPHONY_SPI_FLASH
    crm_module_clk_set(HAL_SPI0, SPI_CLKSEL_288M);
#endif

    crm_set_u32(INTF_CLKEN_REG, 0x1fff);
    crm_reg_set_valid_bit(INTF_SRSTN_REG, 0, 11, 0x0);
    crm_set_u32(INTF_SRSTN_REG, 0x7fffffff);

    /* gra */
    crm_module_clk_enable(HAL_GPE, TRUE);
    crm_module_reset(HAL_GPE);

    /* jpeg */
    crm_module_clk_enable(HAL_JPEG, TRUE);
    crm_module_reset(HAL_JPEG);

    /* disp */
    crm_module_clk_enable(HAL_DISPLAY, TRUE);
    crm_module_reset(HAL_DISPLAY);

    /* vdec */
    crm_module_clk_enable(HAL_VDEC, TRUE);
    crm_module_reset(HAL_VDEC);

    /* aout */
    crm_module_clk_enable(HAL_AUDIO_OUT, TRUE);
    crm_module_clk_enable(HAL_SPDF, TRUE);
    crm_module_reset(HAL_AUDIO_OUT);

    /* vout */
    crm_module_clk_enable(HAL_SD_VIDEO, TRUE);
    crm_module_clk_enable(HAL_HD_VIDEO, TRUE);
    crm_module_clk_enable(HAL_HDMI, TRUE);
    //crm_module_clk_enable(HAL_LCD, TRUE);
    crm_module_clk_enable(HAL_VBI, TRUE);
    crm_module_reset(HAL_SD_VIDEO);
    crm_module_reset(HAL_HD_VIDEO);
    crm_module_reset(HAL_HDMI);
    //crm_module_reset(HAL_LCD);
    crm_module_reset(HAL_VBI);

    /* png */
    crm_module_clk_enable(HAL_PNG, TRUE);
    crm_module_reset(HAL_PNG);

#ifdef CONFIG_SYMPHONY_SMART_SPEAKER
    /* dai */
    crm_module_clk_enable(HAL_DAI, TRUE);
    crm_module_reset(HAL_DAI);
#endif

    /* tsi */
    crm_module_clk_set(HAL_TS1, TS1_CLKSEL_DEMOS_CLK);
    crm_module_clk_set(HAL_TS2, TS2_CLKSEL_DEMOC_CLK);
    crm_module_clk_enable(HAL_TSI, TRUE);
    crm_module_reset(HAL_TSI);
}

void crm_avcpu_suspend(void)
{
   //printk("==crm_avcpu_suspend start==\n");
#if 0/*if spi drv str func not ok,must do those code for spi config*/
   u32 addr = SYMPHONY_SFLASH_VIRT_BASE+0x104;
   spi_register=REG32(addr);
   printk("now_val:0x%x,bk_val:0x%x\n",REG32(addr),spi_register);
#endif
   //printk("==crm_avcpu_suspend end==\n");
}
void crm_avcpu_resume(void)
{
   u32 ret;
   //printk("==crm_avcpu_resume start==\n");
#if 0/*if spi drv str func not ok,must do those code for spi config*/
   u32 addr = SYMPHONY_SFLASH_VIRT_BASE+0x104;

   if(REG32(addr)!=spi_register)
   {
   	  printk("now_val:0x%x,bk_val:0x%x,spi reg err,need config sync before suspend\n",REG32(addr),spi_register);
	  REG32(addr)=spi_register;
   }
#endif

if(mt_avcpu_get_encry_status())
{
	//printk("==avcpu decrypt start==\n");
	ret=mt_avcpu_decrypt(avcpu_boot_addr);
	if(!ret)
	{
		printk("==avcpu decrypt fail,resume avcpu stop==\n");
		return;
	}
}else{
#ifdef CONFIG_DECOMPRESS_LZMA
   ret=avcpu_decompress(avcpu_boot_addr);
   if(!ret)
   {
   	  printk("==avcpu decompress fail,resume avcpu stop==\n");
	  return;
   }
#else
  printk("==CONFIG_DECOMPRESS_LZMA not open,avcpu decompress fail==\n");
#endif
}

	crm_config_avcpu_boot_addr(crm_get_avcpu_boot_addr());
   //	printk("==avcpu boot end==\n");
    crm_module_reset(HAL_CPU1);
	//	printk("==crm_avcpu_resume end==\n");
}
EXPORT_SYMBOL(crm_module_reset);
EXPORT_SYMBOL(crm_module_clk_enable);
EXPORT_SYMBOL(crm_module_clk_is_enabled);
EXPORT_SYMBOL(crm_module_clk_set);
EXPORT_SYMBOL(crm_module_clk_get);
EXPORT_SYMBOL(crm_analog_config);
EXPORT_SYMBOL(crm_module_clk_resume);
EXPORT_SYMBOL(crm_avcpu_resume);
EXPORT_SYMBOL(crm_avcpu_suspend);

#define M2S(x) #x

char *module_id[HAL_MODULES_NUM] =
{
/*!
  CPU0
 */
  M2S(HAL_CPU0),
#if 1
  M2S(HAL_CPU1),
/*!
  CPU2
 */
  M2S(HAL_CPU2),
/*!
  High speed bus
 */
  M2S(HAL_HB),
/*!
  Peripheral low speed bus
 */
  M2S(HAL_PB),
/*!
  Graphic
 */
  M2S(HAL_GPE),
/*!
  Audio output
 */
  M2S(HAL_AUDIO_OUT),
 /*!
  Audio output
 */
  M2S(HAL_SPDF),
 /*!
  VBI
 */
  M2S(HAL_VBI),
/*!
  SD video
 */
  M2S(HAL_SD_VIDEO),
/*!
  HD video
 */
  M2S(HAL_HD_VIDEO),
/*!
  OCP Clock
 */
  M2S(HAL_OCP),
/*!
  VDEC Clock
 */
  M2S(HAL_VDEC),
/*!
  JPEG Clock
 */
  M2S(HAL_JPEG),
/*!
  Display Clock
 */
  M2S(HAL_DISPLAY),
/*!
  DI Clock
 */
  M2S(HAL_DI),
  /*!
 Osdc  Clock
 */
  M2S(HAL_OSDC),
 /*!
  DMA Clock
 */
  M2S(HAL_DMA),
/*!
  TSI Clock
 */
  M2S(HAL_TSI),
/*!
  TSI CSA3.0
 */
  M2S(HAL_TSI_CSA30),
/*!
  DS DES symphony2
 */
  M2S(HAL_DS_DES),
/*!
  DS TDES symphony2
 */
  M2S(HAL_DS_TDES),
/*!
  DS AES symphony2
 */
  M2S(HAL_DS_AES),
/*!
  DS CSA3 symphony2
 */
  M2S(HAL_DS_CSA3),
/*!
  DS CSA2 symphony2
 */
  M2S(HAL_DS_CSA2),
/*!
  DS SECHD1 symphony2
 */
  M2S(HAL_DS_SECHD1),
/*!
  KL PVR symphony2
 */
  M2S(HAL_KL_PVR),
/*!
  CRYPTO DES symphony2
 */
  M2S(HAL_CRYPTO_DES),
/*!
  CRYPTO TDES symphony2
 */
  M2S(HAL_CRYPTO_TDES),
/*!
  CRYPTO AES symphony2
 */
  M2S(HAL_CRYPTO_AES),
/*!
  CRYPTO SHA symphony2
 */
  M2S(HAL_CRYPTO_SHA),
/*!
  CRYPTO RSA symphony2
 */
  M2S(HAL_CRYPTO_RSA),
/*!
  SECHD0 symphony2
 */
  M2S(HAL_SECHD0),
/*!
  KT symphony2
 */
  M2S(HAL_KT),
/*!
  KL CW symphony2
 */
  M2S(HAL_KL_CW),
/*!
  DS),
 */
  M2S(HAL_DS),
/*!
  KDF symphony2
 */
  M2S(HAL_KDF),
/*!
  OTP PRELOAD symphony2
 */
  M2S(HAL_OTP_PRELOAD),
/*!
  SECURE Clock
 */
  M2S(HAL_SECURE),
/*!
  UART0 Clock
 */
  M2S(HAL_UART0),
/*!
  UART1 Clock
 */
  M2S(HAL_UART1),
 /*!
  SMC0 Clock
 */
  M2S(HAL_SMC0),
/*!
  SMC1 Clock
 */
  M2S(HAL_SMC1),
 /*!
  SPI0 Clock
 */
  M2S(HAL_SPI0),
/*!
  SPI1 Clock
 */
  M2S(HAL_SPI1),
/*!
  SPI2 Clock
 */
  M2S(HAL_SPI2),
/*!
  SDMMC Clock
 */
  M2S(HAL_SDMMC),
/*!
  IRDA Clock
 */
  M2S(HAL_IRDA),
/*!
  LEDKB Clock
 */
  M2S(HAL_LEDKB),
/*!
  EPI Clock
 */
  M2S(HAL_EPI),
/*!
  I2C Clock
 */
  M2S(HAL_I2C),
 /*!
  I2C Clock
 */
  M2S(HAL_I2C0),
  /*!
  I2C Clock
 */
  M2S(HAL_I2C1),
  /*!
  I2C debug Clock
 */
  M2S(HAL_I2C_DEBUG),
/*!
  TIMER Clock
 */
  M2S(HAL_TIMER),
/*!
  WATCHDOG Clock
 */
  M2S(HAL_WATCHDOG),
  /*!
  CRYPTO
 */
  M2S(HAL_CRYPTO),
  /*!
  GLITCH_DET symphony2
 */
  M2S(HAL_GLITCH_DET),
   /*!
  SPDIF
 */
  M2S(HAL_CLK_SPDIF),
  /*!
  HDMI
 */
  M2S(HAL_HDMI),
  /*!
  MAC
 */
  M2S(HAL_MAC),
  /*!
  MAC RMII
 */
  M2S(HAL_MAC_RMII),
  /*!
  USB0
 */
  M2S(HAL_USB0),
  /*!
  USB1
 */
  M2S(HAL_USB1),
  /*!
  AO LPMSET symphony2
 */
  M2S(HAL_AO_LPMSET),
  /*!
  AO PINMUX symphony2
 */
  M2S(HAL_AO_PINMUX),
  /*!
  AO GPIO symphony2
 */
  M2S(HAL_AO_GPIO),
  /*!
  AO ANA symphony2
 */
  M2S(HAL_AO_ANA),
  /*!
  AO SECRAM symphony2
 */
  M2S(HAL_AO_SECRAM),
  /*!
  AO KADC symphony2
 */
  M2S(HAL_AO_KADC),
  /*!
  AO RTC symphony2
 */
  M2S(HAL_AO_RTC),
  /*!
  AO MAILBOX symphony2
 */
  M2S(HAL_AO_MAILBOX),
  /*!
  AO TIMER1 symphony2
 */
  M2S(HAL_AO_TIMER1),
  /*!
  AO TIMER0 symphony2
 */
  M2S(HAL_AO_TIMER0),
  /*!
  AO AOMCU symphony2
 */
  M2S(HAL_AO_AOMCU),
  /*!
  AO IRDA symphony2
 */
  M2S(HAL_AO_IRDA),
  /*!
  AO LEDKB symphony2
 */
  M2S(HAL_AO_LEDKB),
  /*!
  AO FPI2C symphony2
 */
  M2S(HAL_AO_FPI2C),
  /*!
  AO FPSPI symphony2
 */
  M2S(HAL_AO_FPSPI),
   /*!
  AO FPSPI symphony2
 */
  M2S(HAL_AO_FPSPIREG),
  /*!
  AO MNT
 */
  M2S(HAL_AO_MNT),
  /*!
  AO CEC
 */
  M2S(HAL_AO_CEC),
  /*!
  AO AVS
 */
  M2S(HAL_AO_AVS),
  /*!
  AO AGTIMER
 */
  M2S(HAL_AO_AGTIMER),
  /*!
  AO WATCHDOG
 */
  M2S(HAL_AO_WDOG),
  /*!
  AO MCU
 */
  M2S(HAL_AO_MCU),
  /*!
  DEMO symphony2
 */
  M2S(HAL_DEMO),
   /*!
  DEMO-C
 */
  M2S(HAL_DEMO_C),
  /*!
  DEMO-S
 */
  M2S(HAL_DEMO_S),
    /*!
  CADC
 */
  M2S(HAL_CADC),
    /*!
  SADC
 */
  M2S(HAL_SADC),
    /*!
  ADAC
 */
  M2S(HAL_ADAC),
   /*!
  VDAC0
 */
  M2S(HAL_VDAC0),
    /*!
  VDAC1
 */
  M2S(HAL_VDAC1),
    /*!
  VDAC2
 */
  M2S(HAL_VDAC2),
    /*!
  VDAC3
 */
  M2S(HAL_VDAC3),
  /*!
  RNG
 */
  M2S(HAL_RNG),
  /*!
  RNG2 symphony2
 */
  M2S(HAL_RNG2),
  /*!
  TSENSOR symphony2
 */
  M2S(HAL_TSENSOR),
  /*!
  PM symphony2
 */
  M2S(HAL_PM),
  /*!
  PLL_CPU
 */
  M2S(HAL_PLL_CPU),
  /*!
  arch timer symphony4
 */
  M2S(HAL_ARCH_TIMER),
  /*!
  SPDMA Clock
 */
  M2S(HAL_SPDMA),
  /*!
  hal intf
 */
  M2S(HAL_INTF),
  /*!
  hal ddr
 */
  M2S(HAL_DDRMC),
  /*!
  hal mnt
 */
  M2S(HAL_MNT),
  /*!
  hal sdio
 */
  M2S(HAL_SDIO0),
  /*!
  hal sdio
 */
  M2S(HAL_SDIO1),
  /*!
  hal pnand
 */
  M2S(HAL_PNAND),
  /*!
  hal lcd
 */
  M2S(HAL_LCD),
  /*!
  hal png
 */
  M2S(HAL_PNG),
  /*!
  hal dai
 */
  M2S(HAL_DAI),
  /*!
  hal ci
 */
  M2S(HAL_CI),
  /*!
  hal ci
 */
  M2S(HAL_CITSIN),
  /*!
  hal xtal
 */
  M2S(HAL_XTAL),
  /*!
  hal m2m
 */
  M2S(HAL_M2M),
  /*!
  hal pka
 */
  M2S(HAL_PKA),
  /*!
  hal ifcp glb
 */
  M2S(HAL_IFCP_GLB),
   /*!
  hal ifcp glb
 */
  M2S(HAL_IFCP_KLM),
    /*!
  hal ifcp glb
 */
  M2S(HAL_IFCP_CRYPTO),
    /*!
  hal ifcp glb
 */
  M2S(HAL_IFCP_SYS),
/*!
  TIMER Clock 0
 */
  M2S(HAL_TIMER0),
/*!
  TIMER Clock 1
 */
  M2S(HAL_TIMER1),
/*!
  TIMER Clock 2
 */
  M2S(HAL_TIMER2),
/*!
  TIMER Clock 3
 */
  M2S(HAL_TIMER3),

/*!
  WATCHDOG Clock
 */
  M2S(HAL_WATCHDOG0),

/*!
  WATCHDOG Clock
 */
  M2S(HAL_WATCHDOG1),


 /*!
  ts0
 */
  M2S(HAL_TS0),

 /*!
  ts1
 */
  M2S(HAL_TS1),

 /*!
  ts2
 */
  M2S(HAL_TS2),

 /*!
  ts3
 */
  M2S(HAL_TS3),

 /*!
  demux
 */
  M2S(HAL_DEMUX),

   /*!
  t2mi
 */
  M2S(HAL_T2MI),

  /*!
  av sync
 */
  M2S(HAL_TSI_AVSYNC),
  /*!
  tsi sf
 */
  M2S(HAL_TSI_SF),
  /*!
  tsi sf
 */
 M2S(HAL_TSI_TRPP),
  /*!
  tsi swtsi
  */
  M2S(HAL_TSI_SWTSI),
  /*!
  tsi tspool
  */
  M2S(HAL_TSI_TSPOOL),
  /*!
  smc
  */
  M2S(HAL_SMC),
  M2S(HAL_AXI),
  M2S(HAL_AXI_DEBUG),
  M2S(HAL_AXI_REG),
  M2S(HAL_LCDC),
  M2S(HAL_LCDHD),
  M2S(HAL_LCD2X),
  M2S(HAL_XTAL_MODE),
  M2S(HAL_AUDIO_MCLK),
  M2S(HAL_TEMPSENSOR),
  M2S(HAL_PANTHER2),
  M2S(HAL_DEMO_J83B),
#endif
};

void crm_module_info_dump(void)
{
     int idx = 0, ret = 0, ret2 = 0;

     for(idx = 0; idx < HAL_MODULES_NUM; idx++)
     {
         ret = crm_module_clk_is_enabled(idx);
         ret2 = crm_module_clk_get(idx);
         if(ret == -1 && ret2 == -1)
            continue;
         if(ret2 != -1)
             printk(KERN_INFO "%s : %s  \tClock: %d\n", module_id[idx], ret==1?"ON":(ret == 0)?"OFF":"N/A", ret2);
         else
             printk(KERN_INFO "%s : %s\n", module_id[idx], ret==1?"ON":(ret == 0)?"OFF":"N/A");
     }
}
void crm_module_init(void)
{
     /* enable hardware auto gating */
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf50f904), 1, 1, 0); //SPDF auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf50f900), 1, 1, 1);
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf400038), 0, 3, 0); //enable dma auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf2701e4), 0, 11, 0x240); //enable tsi auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf240048), 0, 9, 0x1ff); //enable descrambler auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf35000c), 3, 3, 0x0); //enable pka auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf314500), 0, 1, 0x0); //enable otp auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf18d008), 16, 5, 0x0); //enable pnand auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf01012c), 8, 2, 0x0); //enable spi0 auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf02012c), 8, 2, 0x0); //enable spi1 auto gating
#if 0
     crm_module_clk_set(HAL_CPU1, AVCPU_CLKSEL_360M);
     crm_module_clk_set(HAL_IFCP_SYS, IFCP_SYS_CLKSEL_24M);//vscpu
     crm_module_clk_set(HAL_HB, AHB_CLK_RESV);
     crm_module_clk_set(HAL_PB, APB_CLK_90M);
     crm_module_clk_set(HAL_DMA, DMA_CLKESEL_262M);
     crm_module_clk_set(HAL_TSI, TSI_CLKSEL_206M);
     crm_module_clk_set(HAL_SECURE, SECURE_CLKSEL_206M);
     crm_module_clk_set(HAL_VDEC, VDEC_CLKSEL_262M);
     crm_module_clk_set(HAL_DISPLAY, DISPCORE_CLKSEL_206M);
     crm_module_clk_set(HAL_DI, DISPDI_CLKSEL_288M);
     crm_module_clk_set(HAL_GPE, GRA_CLKSEL_262M);
     crm_module_clk_set(HAL_JPEG, JPG_CLKSEL_206M);
     crm_module_clk_set(HAL_AUDIO_OUT, AOUT_CLKSEL_288M);

     crm_module_clk_enable(HAL_IFCP_CRYPTO, 0);  //close seccpu
     crm_module_clk_enable(HAL_IFCP_KLM, 0);  //
     crm_module_clk_enable(HAL_KL_CW, 0); //kl pvr, cw
     crm_module_clk_enable(HAL_SECHD0, 0); //sechd0
     crm_module_clk_enable(HAL_KT, 1); //KT
     crm_module_clk_enable(HAL_KDF, 0);
     crm_module_clk_enable(HAL_GLITCH_DET, 0);
     crm_module_clk_enable(HAL_UART1, 0);
     crm_module_clk_enable(HAL_SDIO0, 0);
     crm_module_clk_enable(HAL_SDIO1, 0);
     crm_module_clk_enable(HAL_PNAND, 0);
     crm_module_clk_enable(HAL_SPI1, 0);
     crm_module_clk_enable(HAL_LCD, 0);
     crm_module_clk_enable(HAL_CI, 0);
     crm_module_clk_enable(HAL_AXI_DEBUG, 0);
     crm_module_clk_enable(HAL_IFCP_KLM, 0);
     crm_module_clk_enable(HAL_PKA, 1);
     crm_module_clk_enable(HAL_M2M, 0);
     crm_module_clk_enable(HAL_PANTHER2, 0);
     //crm_module_clk_enable(HAL_I2C1, 0);
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf314500), 0, 1, 0);  //otp preload
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf50f904), 1, 1, 0); //for spdma auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf50f900), 1, 1, 1);
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xBF50C000), 7, 1, 0); //close m2m
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xBF50C000), 2, 2, 0); //close kle, secgd0
     //crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xBF303010), 0, 11, 0); //enable m2m auto gating
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xBF305010), 13, 1, 0); //power down analog part of Glitch_Det0
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xBF305010), 29, 1, 0); //power down analog part of Glitch_Det1
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xBF30504c), 13, 1, 0); //power down analog part of Glitch_Det2
#endif
     crm_reg_set_valid_bit(SYMPHONY_IO_VA(0xbf240048), 0, 9, 0x1ff);//DS DES/TDES/AES/CSA3/CSA2/SECHD1, hw auto gating

     //crm_module_info_dump();
}
void crm_test(void)
{
     //int idx = 0, ret = 0, ret2 = 0;
     printk("\n=====default config=====\n");
     crm_module_info_dump();
     crm_module_clk_set(HAL_CPU1, AVCPU_CLKSEL_576M);
     crm_module_clk_set(HAL_HB, AHB_CLK_144M);
     crm_module_clk_set(HAL_PB, APB_CLK_90M);
     crm_module_clk_set(HAL_DMA, DMA_CLKESEL_131M);
     crm_module_clk_set(HAL_SMC, SMC_CLKSEL_XTAL_DIV2);
     //crm_module_clk_set(HAL_SPI0, SPI_CLKSEL_288M);
     crm_module_clk_set(HAL_SPI1, SPI_CLKSEL_360M);
     crm_module_clk_set(HAL_PNAND, PNAND_CLKSEL_288M);
     crm_module_clk_set(HAL_SDIO0, SDIO_CLKSEL_100M);
     crm_module_clk_set(HAL_SDIO1, SDIO_CLKSEL_100M);
     //crm_module_clk_set(HAL_UART0, UARTPHY_CLKSEL_XTAL);
     crm_module_clk_set(HAL_UART1, UARTPHY_CLKSEL_120M);
     crm_module_clk_set(HAL_GPE, GRA_CLKSEL_320M);
     crm_module_clk_set(HAL_JPEG, JPG_CLKSEL_144M);
     crm_module_clk_set(HAL_DISPLAY, DISPCORE_CLKSEL_131M);
     crm_module_clk_set(HAL_DI, DISPDI_CLKSEL_288M);
     crm_module_clk_set(HAL_OSDC, DISPOSDC_CLKSEL_144M);
     crm_module_clk_set(HAL_VDEC, VDEC_CLKSEL_131M);
     crm_module_clk_set(HAL_AUDIO_OUT, AOUT_CLKSEL_206M);
     crm_module_clk_set(HAL_LCDC, VOUT_LCDC_CLKSEL_LCD2XCLK_DIV2);
     crm_module_clk_set(HAL_LCDHD, VOUT_LCDHD_CLKSEL_LCDC_EN);
     crm_module_clk_set(HAL_LCD2X, VOUT_LCD_CLKSEL_80M);
     crm_module_clk_set(HAL_HD_VIDEO, VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV2); //need to fix ,related to analog
     crm_module_clk_set(HAL_PNG, PNG_CLKSEL_144M);
     crm_module_clk_set(HAL_TSI, TSI_CLKSEL_262M);
     crm_module_clk_set(HAL_TS0, TS0_CLKSEL_PAD_TS0_CLK_REVERSE);
     crm_module_clk_set(HAL_TS1, TS1_CLKSEL_PAD_CLK_REVERSE);
     crm_module_clk_set(HAL_TS2, TS2_CLKSEL_DEMOC_CLK_REVERSE);
     crm_module_clk_set(HAL_TS3, TS3_CLKSEL_PAD_CLK_REVERSE);
     crm_module_clk_set(HAL_CITSIN, CITSIN_CLKSEL_DEMOS_TS1);
     crm_module_clk_set(HAL_CI, CI_CLKSEL_120M);
     //crm_module_clk_set(HAL_XTAL_MODE, XTAL_CLK_SECURITY_OSC);
     crm_module_clk_set(HAL_M2M, SECURE_M2M_CIPHER_CLKSEL_240M);
     crm_module_clk_set(HAL_SECURE, SECURE_CLKSEL_262M);

     printk("\n=====after clock update=====\n");
     crm_module_info_dump();

     crm_module_clk_enable(HAL_MAC, 0);
     crm_module_clk_enable(HAL_MAC_RMII, 0);
     //crm_module_clk_enable(HAL_AXI, 0);
     //crm_module_clk_enable(HAL_AXI_DEBUG, 0);
     //crm_module_clk_enable(HAL_AXI_REG, 0);
     //crm_module_clk_enable(HAL_CPU1, 0);
     crm_module_clk_enable(HAL_DMA, 0);
     crm_module_clk_enable(HAL_SMC, 0);
     crm_module_clk_enable(HAL_USB0, 0);
     crm_module_clk_enable(HAL_USB1, 0);
     crm_module_clk_enable(HAL_I2C0, 0);
     crm_module_clk_enable(HAL_I2C1, 0);
     crm_module_clk_enable(HAL_I2C_DEBUG, 0);
     //crm_module_clk_enable(HAL_SPI0, 0);
     crm_module_clk_enable(HAL_SPI1, 0);
     crm_module_clk_enable(HAL_PNAND, 0);
     crm_module_clk_enable(HAL_SDIO0, 0);
     crm_module_clk_enable(HAL_SDIO1, 0);
     //crm_module_clk_enable(HAL_UART0, 0);
     crm_module_clk_enable(HAL_UART1, 0);
     crm_module_clk_enable(HAL_GPE, 0);
     crm_module_clk_enable(HAL_JPEG, 0);
     crm_module_clk_enable(HAL_DISPLAY, 0);
     crm_module_clk_enable(HAL_VDEC, 0);
     crm_module_clk_enable(HAL_AUDIO_OUT, 0);
     crm_module_clk_enable(HAL_SPDF, 0);
     crm_module_clk_enable(HAL_HDMI, 0);
     crm_module_clk_enable(HAL_LCD, 0);
     crm_module_clk_enable(HAL_HD_VIDEO, 0);
     crm_module_clk_enable(HAL_VBI, 0);
     crm_module_clk_enable(HAL_SD_VIDEO, 0);
     crm_module_clk_enable(HAL_PNG, 0);
     crm_module_clk_enable(HAL_DAI, 0);
     crm_module_clk_enable(HAL_TSI, 0);
     crm_module_clk_enable(HAL_CI, 0);
     crm_module_clk_enable(HAL_M2M, 0);
     crm_module_clk_enable(HAL_PKA, 0);
     crm_module_clk_enable(HAL_GLITCH_DET, 0);
     crm_module_clk_enable(HAL_KT, 0);
     crm_module_clk_enable(HAL_SECHD0, 0);
     crm_module_clk_enable(HAL_KL_CW, 0);
     crm_module_clk_enable(HAL_KDF, 0);
     crm_module_clk_enable(HAL_SECURE, 0);
     //crm_module_clk_enable(HAL_DDRMC, 0);
     crm_module_clk_enable(HAL_IFCP_CRYPTO, 0);
     crm_module_clk_enable(HAL_IFCP_KLM, 0);
     crm_module_clk_enable(HAL_AO_LPMSET, 0);
     crm_module_clk_enable(HAL_AO_IRDA, 0);
     crm_module_clk_enable(HAL_AO_LEDKB, 0);
     crm_module_clk_enable(HAL_AO_GPIO, 0);
     crm_module_clk_enable(HAL_AO_KADC, 0);
     crm_module_clk_enable(HAL_AO_ANA, 0);
     crm_module_clk_enable(HAL_AO_FPI2C, 0);
     crm_module_clk_enable(HAL_AO_FPSPI, 0);
     crm_module_clk_enable(HAL_AO_SECRAM, 0);
     crm_module_clk_enable(HAL_AO_PINMUX, 0);
     crm_module_clk_enable(HAL_AO_TIMER0, 0);
     crm_module_clk_enable(HAL_AO_TIMER1, 0);
     crm_module_clk_enable(HAL_AO_MAILBOX, 0);
     crm_module_clk_enable(HAL_AO_CEC, 0);
     crm_module_clk_enable(HAL_AO_AVS, 0);
     crm_module_clk_enable(HAL_AO_AGTIMER, 0);
     crm_module_clk_enable(HAL_AO_MCU, 0);
     crm_module_clk_enable(HAL_PANTHER2, 0);
     crm_module_clk_enable(HAL_TEMPSENSOR, 0);
     printk("\n=====after module close=====\n");
     crm_module_info_dump();


}
