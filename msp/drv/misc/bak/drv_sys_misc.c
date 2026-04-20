/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
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
#include <linux/syscore_ops.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/suspend.h>


#include "mt_common.h"
#include "mt_reg_common.h"
//FIXME: for symphony1/2/4, how about Aria?
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_module_debug.h"

#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_sys.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_osal.h"
#include "drv_sys_misc.h"
#include "mt_mach/chipinfo.h"
#include "mt_unf_misc.h"
#include "mt_analog.h"

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#include "drv_sym4_reset.h"
extern int crm_module_reset(mt_u32 mod_id);
extern int crm_module_clk_enable(mt_u32 mod_id,MT_BOOL onoff);
extern int crm_module_clk_is_enabled(mt_u32 mod_id);
extern int crm_module_clk_set(mt_u32 m_id, mt_u32 type);
extern int crm_module_clk_get(mt_u32 m_id);
extern void crm_module_info_dump(void);
#endif

//FIXME: for symphony1/2/4, how about Aria?
#define SYMPHONY_AO_CRM_REG_BASE	(SYMPHONY_IO_VA(0xBF153000))

#define M_SYS_TIMER_MAX_NUM    (4)
extern unsigned long plat_get_xtal_freq(void);
extern long symphony_get_clock(unsigned long m_id, unsigned long  *p_clk);

#define MISC_CLK_HZ (plat_get_xtal_freq())


#define MS_CLK_PRELOAD  ((MISC_CLK_HZ/1000)-1)

//static unsigned int sn_misc_base = 0;

//#define MISC_REG_BASE (sn_misc_base)

#define MISC_WRITE32(addr, v) 	HAL_PUT_U32((volatile u32 *)(addr), (u32)(v))
#define MISC_READ32(addr) 		HAL_GET_U32((volatile u32 *)(addr))

#define SYM2_OTP_FUSE_VERSION	(0x1D420)
#define SYM2_OTP_INFO1	(0x1D440)
#define SYM2_OTP_INFO2	(0x1D460)
#define SYM2_OTP_INFO3	(0x1E080)

static unsigned long driver_open = 0;

extern package_id_symphony_t chip_package_get(void);

extern int sys_otp_read(u32 bit_addr, u8 len, u32 *p_result);

static int reg_set_valid_bit(u32 reg, u8 sbit, u8 size, u32 val)
{
    u32 tmp = 0, vbit = 0, cur_val = 0;

    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = MISC_READ32((volatile u32 *)reg);
    cur_val = (tmp >> sbit) & vbit; //get old value from register
    if(val == cur_val)
        return 0;
    tmp &= ~(vbit << sbit);
    tmp |= (val << sbit);
    MISC_WRITE32((volatile u32 *)reg, tmp);

    return 0;
}
static int reg_get_valid_bit(unsigned int reg, unsigned int sbit, unsigned int size, unsigned int *val)
{
    unsigned int tmp = 0, vbit = 0;

    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = MISC_READ32((volatile unsigned int *)reg);
    *val = (tmp >> sbit) & vbit; //get old value from register
    return 0;
}

#if !defined(CONFIG_MT_CHIP_SYMPHONY4)
/*!
  store pinmux debug bit, hal_symphony_demo_pinmuxset will check its value and as it makes different decision
  for each bit, 0 means default mode, 1 means debug mode
  */
static unsigned int g_dbg_pinmux_init_cfg = 0;
#endif

static unsigned int get_pinmux_reg_addr(unsigned int index)
{
  unsigned int base_addr = 0;

  switch (index)
  {
    case SW_PIN0:
    case SW_PIN1:
    case SW_PIN2:
    case SW_PIN3:
    case SW_PIN4:
    case SW_PIN5:
    case SW_PIN6:
    case SW_PIN7:
      base_addr = R_SW_PIN_ADDR + index * 4;
      break;
    case AO_PIN0:
    case AO_PIN1:
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	case AO_PIN2:
	case AO_PIN3:
	case AO_PIN4:
	case AO_PIN5:
	case AO_PIN6:
	case AO_PIN7:
	case AO_PIN8:
#endif
      base_addr = R_AO_PIN_ADDR + (index - AO_PIN0) * 4;
      break;
#if !defined(CONFIG_MT_CHIP_SYMPHONY4)
    case I2C_PIN0:
      base_addr = R_I2C_PIN_ADDR + (index - I2C_PIN0) * 4;
      break;
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	case SMC_PIN0:
		base_addr = R_SMC_PIN_ADDR + (index - SMC_PIN0) * 4;
		break;
#endif
    default:
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
		if (index > SW_PIN7 && index < SW_PIN_MAX)
		{
			base_addr = R_SW_PIN_ADDR + (index - SW_PIN0) * 4;
		}
		else
		{
			//BUG!
			MT_ERR_MISC("invalid pinmux index: %u\n", index);
		}
#endif
      break;
  }

  return base_addr;
}

 /*!
  Get bits register value

  \param[in] addr register address
  \param[in] offset the offset from 0 bit
  \param[in] num the number bits you want to get

  \return bits value
  */
unsigned int symphony_getpinmux(unsigned int index, unsigned int offset, unsigned int len)
{
  unsigned int base_addr = 0;
  unsigned int val = 0;

  base_addr = get_pinmux_reg_addr(index);

  if (base_addr != 0)
    reg_get_valid_bit(base_addr,offset,len, &val);

  //printk("%s reg:0x%x val:0x%x\n", __func__, base_addr, val);
  MT_INFO_MISC("idx %u, addr %x, off %u, len %u, val %u\n", index, base_addr, offset, len, val);
  return val;
}

/*!
  Set bits register value

  \param[in] addr register address
  \param[in] offset the offset from 0 bit
  \param[in] num the number bits you want to get
  \param[in] value the bits value you want to set

  \return
  */
void symphony_setpinmux(unsigned int index, unsigned int offset, unsigned int len, unsigned int value)
{
  unsigned int base_addr = 0;

  base_addr = get_pinmux_reg_addr(index);

  if (base_addr != 0)
  {
    reg_set_valid_bit(base_addr, offset, len, value);
    //printk("%s reg:0x%x val:0x%x\n", __func__, base_addr, value);
	MT_INFO_MISC("idx %u, addr %x, off %u, len %u, val %u\n", index, base_addr, offset, len, value);
  }
}

/*
 drv demo board cfg
*/
void symphony_pinmux_preset(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	//symphony4 no preset pinmux in drv anymore.
	//but set in uboot.
	//see: uboot_symphony_pinctrl.env
	MT_WARN_MISC("symphony4 not support preset pinmux in drv, pls preset in uboot!\n");
	return;
#else
    unsigned int temp=0;
    package_id_symphony_t packet_id = 0;
    unsigned int chip_rev = 0;

    temp = MISC_READ32((volatile unsigned long *)R_CHIP_CFG);
    packet_id = chip_package_get();
    chip_rev = MISC_READ32((volatile unsigned long *)R_CHIP_ID) & 0xffff;

    if(chip_rev >= 0x9000) //don't config pinmux here for symphony2, because pinmux had been preset in uboot.
         return;

    switch(packet_id){
      case PACKET_CHIP_SIP68C_DDR2://SIP68C
          symphony_setpinmux(SW_PIN0,0,32,0x11111111);
          symphony_setpinmux(SW_PIN1,0,32,0x11001);
          symphony_setpinmux(SW_PIN2,0,32,0x13333110);
          symphony_setpinmux(SW_PIN3,0,32,0x121);
          symphony_setpinmux(SW_PIN4,0,32,0x111000);
          symphony_setpinmux(SW_PIN5,0,32,0x111001);
          symphony_setpinmux(AO_PIN0,0,32,0x100100);
          symphony_setpinmux(AO_PIN1,0,8,0);
          symphony_setpinmux(I2C_PIN0,4,1,1);
          symphony_setpinmux(I2C_PIN0,0,1,1);
          break;
      case PACKET_CHIP_SIP68S_DDR2: //SIP68S
          symphony_setpinmux(SW_PIN0,0,32,0x11111111);
          symphony_setpinmux(SW_PIN1,0,32,0x11001);
          if((g_dbg_pinmux_init_cfg >> PINMUX_DBG_JTAG) & 0x1)
              symphony_setpinmux(SW_PIN2,0,32,0x13333110);
          else
              symphony_setpinmux(SW_PIN2,0,32,0x12212110);
          symphony_setpinmux(SW_PIN3,0,32,0x111);
          symphony_setpinmux(SW_PIN4,0,32,0x220000);
          symphony_setpinmux(SW_PIN5,0,32,0x111001);
          symphony_setpinmux(AO_PIN0,0,32,0x100100);
          symphony_setpinmux(AO_PIN1,0,8,0);
          symphony_setpinmux(I2C_PIN0,4,1,1);
          symphony_setpinmux(I2C_PIN0,0,1,1);
          break;
      case PACKET_CHIP_QFP_144: //144pin
      case PACKET_CHIP_SIP144_DDR2:
      case PACKET_CHIP_SIP144_DDR3:
          symphony_setpinmux(SW_PIN0,0,32,0x0);
          symphony_setpinmux(SW_PIN1,0,32,0x11000);
          if((g_dbg_pinmux_init_cfg >> PINMUX_DBG_JTAG) & 0x1)
              symphony_setpinmux(SW_PIN2,0,32,0x13333110);
          else
              symphony_setpinmux(SW_PIN2,0,32,0x12212110);
          symphony_setpinmux(SW_PIN3,0,32,0x111);
          symphony_setpinmux(SW_PIN4,0,32,0x220000);
          symphony_setpinmux(SW_PIN5,0,32,0x111000);//fix bug 99011
          symphony_setpinmux(AO_PIN0,0,32,0x100100);
          symphony_setpinmux(AO_PIN1,0,8,0);
          symphony_setpinmux(I2C_PIN0,4,1,1);
          symphony_setpinmux(I2C_PIN0,0,1,1);
          break;
      default:
      break;
    }
#endif
}
EXPORT_SYMBOL(symphony_pinmux_preset);

extern void hal_symphony2_a1_onoff_ephy(u32 on);

int symphony_module_reset(unsigned int mid)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY4
       crm_module_reset(mid);
#endif
	return 0;
}


int symphony_module_config(unsigned int mid, unsigned int on)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    return crm_module_clk_enable(mid, on);
#else
    unsigned int reg = 0;
    unsigned int start = 0;
    unsigned int size = 1;
    unsigned int val = 0;
    u32 chip_rev=0;
    chip_rev= symphony_get_chip_rev();

    //module_clk_enable(mid, on);

    switch(mid)
    {
          case HAL_CPU1:
            reg = CLK_GATE_CFG0;
            start = 1;
            val = on?1:0;
            break;
          case HAL_SECURE:
            reg = CLK_GATE_CFG0;
            start = 2;
            val = on?1:0;
            break;
          case HAL_DMA:
            reg = CLK_GATE_CFG0;
            start = 8;
            val = on?1:0;
            break;
          case HAL_GLITCH_DET:
            reg = CLK_GATE_CFG1;
            start = 10;
            val = on?1:0;
            break;
          case HAL_CRYPTO:
            reg = CLK_GATE_CFG1;
            start = 8;
            val = on?1:0;
            break;
          case HAL_TSI:
            reg = CLK_GATE_CFG1;
            start = 0;
            val = on?1:0;
            break;
          case TSI_CSA3_REG:
            reg = 0xbf2702b8;
            start = 0;
            val = on?0:1;
            break;
          case HAL_DS_DES:
            reg = 0xbf30f0d0;
            start = 5;
            val = on?1:0;
            break;
          case HAL_DS_TDES:
            reg = 0xbf30f0d0;
            start = 4;
            val = on?1:0;
            break;
          case HAL_DS_AES:
            reg = 0xbf30f0d0;
            start = 3;
            val = on?1:0;
            break;
          case HAL_DS_CSA3:
            reg = 0xbf30f0d0;
            start = 2;
            val = on?1:0;
            break;
          case HAL_DS_CSA2:
            reg = 0xbf30f0d0;
            start = 1;
            val = on?1:0;
            break;
          case HAL_DS_SECHD1:
            reg = 0xbf30f0d0;
            start = 0;
            val = on?1:0;
            break;
          case HAL_KL_PVR:
            reg = 0xbf30f0e0;
            start = 17;
            val = on?1:0;
            break;
          case HAL_CRYPTO_DES:
            reg = 0xbf30f0e0;
            start = 9;
            val = on?1:0;
            break;
          case HAL_CRYPTO_TDES:
            reg = 0xbf30f0e0;
            start = 8;
            val = on?1:0;
            break;
          case HAL_CRYPTO_AES:
            reg = 0xbf30f0e0;
            start = 7;
            val = on?1:0;
            break;
          case HAL_CRYPTO_SHA:
            reg = 0xbf30f0e0;
            start = 6;
            val = on?1:0;
            break;
          case HAL_CRYPTO_RSA:
            reg = 0xbf30f0e0;
            start = 5;
            val = on?1:0;
            break;
          case HAL_SECHD0:
            reg = 0xbf30f0e0;
            start = 4;
            val = on?1:0;
            break;
          case HAL_KT:
            reg = 0xbf30f0e0;
            start = 3;
            val = on?1:0;
            break;
          case HAL_KL_CW:
            reg = 0xbf30f0e0;
            start = 2;
            val = on?1:0;
            break;
          case HAL_KDF:
            reg = 0xbf30f0e0;
            start = 1;
            val = on?1:0;
            break;
          case HAL_OTP_PRELOAD:
            reg = 0xbf30f0e0;
            start = 0;
            val = on?1:0;
            break;
          case HAL_AUDIO_OUT:
            reg = CLK_GATE_CFG2;
            start = 24;
            val = on?1:0;
            break;
          case HAL_CLK_SPDIF:
            reg = CLK_GATE_CFG2;
            start = 25;
            val = on?1:0;
            break;
          case HAL_HDMI:
            reg = CLK_GATE_CFG2;
            start = 10;
            val = on?1:0;
	      if(chip_rev >= CHIP_SYMPHONY2_A0)
	      	{
               reg_set_valid_bit(reg, start,size,val);
               reg = REG_CLK_ANALOG;
               start = 8;
               size = 4;
               val = on?0:7;
		 }
            break;
          case HAL_VBI:
            reg = CLK_GATE_CFG2;
            start = 11;
            val = on?1:0;
            break;
          case HAL_SD_VIDEO:
            reg = CLK_GATE_CFG2;
            start = 8;
            val = on?1:0;
            break;
          case HAL_HD_VIDEO:
            reg = CLK_GATE_CFG2;
            start = 9;
            val = on?1:0;
            break;
          case HAL_GPE:
            reg = CLK_GATE_CFG2;
            start = 3;
            val = on?1:0;
            break;
          case HAL_DISPLAY:
            reg = CLK_GATE_CFG2;
            start = 2;
            val = on?1:0;
            break;
          case HAL_JPEG:
            reg = CLK_GATE_CFG2;
            start = 1;
            val = on?1:0;
            break;
          case HAL_VDEC:
            reg = CLK_GATE_CFG2;
            start = 0;
            val = on?1:0;
            break;
          case HAL_MAC:
            reg = CLK_GATE_CFG3;
            start = 4;
            val = on?1:0;
            if(chip_rev < CHIP_SYMPHONY2_A0)
            {
                reg_set_valid_bit(reg, start,size,val);
                reg = REG_CLK_ANALOG;
                start = 30;
                val = on?0:1;
            }
            else
                hal_symphony2_a1_onoff_ephy(val);
            break;
          case HAL_MAC_RMII:
            reg = CLK_GATE_CFG3;
            start = 5;
            val = on?1:0;
            break;
          case HAL_USB0:
            reg = CLK_GATE_CFG3;
            start = 0;
            val = on?1:0;
            reg_set_valid_bit(reg, start,size,val);
            reg = REG_CLK_ANALOG;
            start = 1;
            val = on?0:1;
            break;
          case HAL_USB1:
            reg = CLK_GATE_CFG3;
            start = 1;
            val = on?1:0;
            reg_set_valid_bit(reg, start,size,val);
            reg = REG_CLK_ANALOG;
            start = 2;
            val = on?0:1;
            break;
          case HAL_AO_LPMSET:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 16;
            val = on?1:0;
            break;
          case HAL_AO_PINMUX:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 15;
            val = on?1:0;
            break;
          case HAL_AO_GPIO:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 14;
            val = on?1:0;
            break;
          case HAL_AO_ANA:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 13;
            val = on?1:0;
            break;
          case HAL_AO_SECRAM:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 10;
            val = on?1:0;
            break;
          case HAL_AO_KADC:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 9;
            val = on?1:0;
            break;
          case HAL_AO_RTC:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 8;
            val = on?1:0;
            break;
          case HAL_AO_MAILBOX:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 7;
            val = on?1:0;
            break;
          case HAL_AO_TIMER1:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 6;
            val = on?1:0;
            break;
          case HAL_AO_TIMER0:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 5;
            val = on?1:0;
            break;
          case HAL_AO_AOMCU:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 4;
            val = on?1:0;
            break;
          case HAL_AO_IRDA:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 3;
            val = on?1:0;
            break;
          case HAL_AO_LEDKB:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 2;
            val = on?1:0;
            break;
          case HAL_AO_FPI2C:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 1;
            val = on?1:0;
            break;
          case HAL_AO_FPSPI:
            reg = SYMPHONY_AO_CRM_REG_BASE;
            start = 0;
            val = on?1:0;
            break;
          case HAL_DEMO_C:
            reg = 0xbf138004;
            start = 0;
            val = on?0:1;
            reg_set_valid_bit(reg, start,size,val);
            reg = CLK_GATE_CFG3;
            start = 24;
            val = on?0:1;
            break;
          case HAL_DEMO_S:
            #if 0
            reg = SYMPHONY_IO_VA(0xbf138004);
            start = 0;
            val = on?0:1;
            reg_set_valid_bit(reg, start,size,val);
            #endif
            reg = CLK_GATE_CFG3;
            start = 24;
            val = on?0:1;
            break;
          case HAL_CADC:
            reg = REG_CLK_ANALOG;
            start = 20;
            val = on?0:1;
            break;
          case HAL_SADC:
            reg = REG_CLK_ANALOG;
            start = 21;
            val = on?0:1;
            break;
          case HAL_ADAC:
            reg = REG_CLK_ANALOG;
            start = 4;
            size = 3;
            val = on?0:7;
            break;
          case HAL_VDAC0:
            reg = REG_CLK_ANALOG;
            start = 12;
            val = on?0:1;
            break;
          case HAL_VDAC1:
            reg = REG_CLK_ANALOG;
            start = 13;
            val = on?0:1;
            break;
          case HAL_VDAC2:
            reg = REG_CLK_ANALOG;
            start = 14;
            val = on?0:1;
            break;
          case HAL_VDAC3:
            reg = REG_CLK_ANALOG;
            start = 15;
            val = on?0:1;
            break;
          case HAL_RNG:
            reg = REG_CLK_ANALOG;
            start = 24;
            val = on?0:1;
            break;
          case HAL_RNG2:
            reg = REG_CLK_ANALOG;
            start = 29;
            val = on?0:1;
            break;
          case HAL_TSENSOR:
            reg = REG_CLK_ANALOG;
            start = 27;
            val = on?0:1;
            break;
          case HAL_PM:
            reg = REG_CLK_ANALOG;
            start = 22;
            val = on?0:1;
            break;
          case HAL_PLL_CPU:
	      if(chip_rev >= CHIP_SYMPHONY2_A0)
	      	{
               reg = 0xbf157004;
               start = 31;
		 }
		else
		{
               reg = CLKGEN_PDSYS_REG;
               start = 30;
		 }
            val = on?0:1;
            break;
          default:
            MT_ERR_MISC(KERN_ERR "ERROR, wrong module id:%d \n", mid);
            break;
        }

    if (reg)
    {
        reg_set_valid_bit(reg, start, size, val);
    }

	return 0;
#endif
}
int symphony_module_clock_set(unsigned int mid, unsigned int clk)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
     return crm_module_clk_set(mid, clk);
#else
     return 0;
#endif
}

int symphony_module_clock_get(unsigned int mid)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    unsigned int freq = 0;
    freq = crm_module_clk_get(mid);
    return freq;
#else
    return 0;
#endif
}

int symphony_module_is_enabled(unsigned int mid)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    return crm_module_clk_is_enabled(mid);
#else
    return 0;
#endif
}
static unsigned int symphony_temperature_get(void)
{
    u32 val = 0;
    symphony_module_config(HAL_TSENSOR, 1);
    msleep(150);
    /* Read L5=BF5D00F0[12:0];
        TEMP=761.73*HEX2DEC(L5)/8192-280.68 */

    val = MISC_READ32(SYMPHONY_IO_VA(0xBF5D00F0)) & 0x1fff;
    symphony_module_config(HAL_TSENSOR, 0);
    return ((761730 * val)/8192 - 280680);
}

static int misc_open(struct inode *inode, struct file *file)
{
    int ret = 0;

    if (test_and_set_bit(0, &driver_open))
    {
    return -EBUSY;
    }

    return ret;
}

static int misc_release(struct inode *inode, struct file *file)
{
    clear_bit(0, &driver_open);
    return 0;
}
static long misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
#if 1
    void __user *argp = (void __user *)arg;
    struct drv_misc_ioctl drv_arg;

    memset(&drv_arg, 0, sizeof(struct drv_misc_ioctl));

    if (argp != 0 && copy_from_user(&drv_arg, argp, sizeof(struct drv_misc_ioctl))) {
		return -EFAULT;
	}

    //printk("%s %d cmd:%d id:%d off:%d len:%d val:%d\n", __func__, __LINE__, cmd, drv_arg.id, drv_arg.offset, drv_arg.len, drv_arg.val);
    switch (cmd)
    {
        case MISC_IOCTL_PINMUX_PRESET:
            symphony_pinmux_preset();
            break;
        case MISC_IOCTL_PINMUX_SET:
            symphony_setpinmux(drv_arg.id, drv_arg.offset, drv_arg.len, drv_arg.val);
            break;
        case MISC_IOCTL_PINMUX_GET:
            drv_arg.val =symphony_getpinmux(drv_arg.id, drv_arg.offset, drv_arg.len);
            if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            //printk("%s id:%d pinval:0x%x\n", __func__, drv_arg.id, drv_arg.val);
            break;
        case MISC_IOCTL_MODULE_ONOFF:
            symphony_module_config(drv_arg.id, (drv_arg.val > 0)?1:0);
            break;

	case MISC_IOCTL_MODULE_RESET:
            symphony_module_reset(drv_arg.id);
            break;
	case MISC_IOCTL_MODULE_CLK_GET:
             drv_arg.val = symphony_module_clock_get(drv_arg.id);
             if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            break;
	case MISC_IOCTL_MODULE_CLK_SET:
            symphony_module_clock_set(drv_arg.id, drv_arg.val);
            break;
	case MISC_IOCTL_MODULE_CLK_IS_ENABLED:
            drv_arg.val = symphony_module_is_enabled(drv_arg.id);
            if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            break;
        case MISC_IOCTL_REG_SET:
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
            reg_set_valid_bit(SYMPHONY_IO_VA(drv_arg.id), drv_arg.offset, drv_arg.len, drv_arg.val);
#else
            reg_set_valid_bit(drv_arg.id, drv_arg.offset, drv_arg.len, drv_arg.val);
#endif
            break;
        case MISC_IOCTL_REG_GET:
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
            reg_get_valid_bit(SYMPHONY_IO_VA(drv_arg.id), drv_arg.offset, drv_arg.len, &drv_arg.val);
#else
            reg_get_valid_bit(drv_arg.id, drv_arg.offset, drv_arg.len, &drv_arg.val);
#endif
            if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            break;
        case MISC_IOCTL_TEMPERATURE_GET:
            drv_arg.val = symphony_temperature_get();
            if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            break;
		case MISC_IOCTL_CHIP_PRODUCTINFO_GET:
			{
				unsigned int value = 0;
				int ret = 0;
				unsigned char str[64];
				unsigned long chip_rev;
				chip_feature_t chip_info;
				memset((u8*)&chip_info,0xff,sizeof(chip_feature_t));
				memset(str,0,64);
				chip_rev = symphony_get_chip_rev();
				value = 0;
				switch(chip_rev)
				{
					case CHIP_SYMPHONY_A0:
					case CHIP_SYMPHONY_A1:
					case CHIP_SYMPHONY_A2:
					case CHIP_SYMPHONY3_A0:
						printk("%s %d no support!\n",__FUNCTION__,__LINE__);
						break;
					case CHIP_SYMPHONY2_A0:
					case CHIP_SYMPHONY2_A1:
					case CHIP_SYMPHONY2_A2:
					case CHIP_SYMPHONY2_A3:
						ret = sys_otp_read(SYM2_OTP_INFO1,32,&value);
						if(0 == ret)
						{
							chip_info.type = (value >> 28) & 0x0f;
							chip_info.family= (value >> 26) & 0x03;
							chip_info.genernation = (value >> 24) & 0x03;
							chip_info.display= (value >> 20) & 0x0f;
							chip_info.security = (value >> 16) & 0x0f;
						}

						value = 0;
						ret = sys_otp_read(SYM2_OTP_INFO2,32,&value);
						if(0 == ret)
						{
							chip_info.package= (value >> 28) & 0x0f;
							chip_info.sipdramsize= (value >> 24) & 0x0f;
							chip_info.cavendor= (value >> 20) & 0x0f;
							chip_info.caversion = (value >> 18) & 0x03;
							chip_info.dolbyenable= ((((value >> 16) & 0x03) == 0)?1:0);
							chip_info.drminfo= (value >> 12) & 0x0f;
							chip_info.iplicense = (value >> 8) & 0x0f;
							chip_info.hddecenable= (((value >> 7) & 0x01)?0:1);
							chip_info.draenable= (((value >> 6) & 0x01)?0:1);
							chip_info.dramkgd= (value & 0x0f);
						}

						value = 0;
						ret = sys_otp_read(SYM2_OTP_INFO3,32,&value);
						if(0 == ret)
						{
							chip_info.dvbt = 0;
							chip_info.dvbs = 0;
							chip_info.hd = (((value >> 31) & 0x01)?0:1);
							chip_info.macrovision= (((value >> 30) & 0x01)?0:1);
							chip_info.h265 = (((value >> 29) & 0x01)?0:1);
							chip_info.usb0 = (((value >> 28) & 0x01)?0:1);
							chip_info.usb1 = (((value >> 27) & 0x01)?0:1);
							chip_info.usbcount = chip_info.usb0 + chip_info.usb1;
							chip_info.dvbt |= ((((value >> 26) & 0x01)?0:1) << 1);		/*T2*/
							chip_info.dvbt |= (((value >> 25) & 0x01)?0:1);				/*T*/
							chip_info.dvbs |= ((((value >> 24) & 0x01)?0:1) << 1);		/*S2X*/
							chip_info.dvbs |= (((value >> 23) & 0x01)?0:1);			/*S/S2/S2X*/
							chip_info.j83b = (((value >> 22) & 0x01)?0:1);
							chip_info.dvbc = (((value >> 21) & 0x01)?0:1);
							chip_info.ephy= (((value >> 20) & 0x01)?0:1);
							chip_info.hdr= (((value >> 19) & 0x01)?0:1);
							chip_info.vdec10b= (((value >> 18) & 0x01)?0:1);
							chip_info.ethernet= ((((value >> 14) & 0x03)== 0)?1:0);
						}

						writel(0xffff,(volatile unsigned long *)R_CHIP_ID_RDEN);
						chip_info.inter_chip_id = readl((volatile unsigned long *)R_CHIP_ID) & 0xffff;
						writel(0,(volatile unsigned long *)R_CHIP_ID_RDEN);

						if((0 == chip_info.family) || (0 == chip_info.genernation))
						{
							chip_info.genernation = chip_info.inter_chip_id & 0x01;
							chip_info.family = ((((chip_info.inter_chip_id >> 12) & CHIP_SYMPHONY2) == CHIP_SYMPHONY2)?CHIP_SYMPHONY2:0xff);
						}
						else
						{
							switch(chip_info.family){
								case 1:
									chip_info.family = CHIP_SYMPHONY1;
									break;
								case 2:
									chip_info.family = CHIP_SYMPHONY2;
									break;
								case 3:
									chip_info.family = CHIP_SYMPHONY3;
									break;
							}
						}

						str[0] = 'M';
						switch(chip_info.type){
							case 0:
								str[1] = 'S';
								break;
							case 1:
								str[1] = 'C';
								break;
							case 2:
								str[1] = 'T';
								break;
							case 3:
								str[1] = 'M';
								break;
							default:
								str[1] = 0;
								break;
						}

						if(CHIP_SYMPHONY2 == chip_info.family)
						{
							str[2] = '2';
							str[3] = '2';
						}
						else
						{
							str[2] = 0;
							str[3] = 0;
						}

						if(chip_info.display != 0xff)
						{
							if(chip_info.display == 0)
							{
								str[4]= '1';
							}
							else
							{
								str[4]= chip_info.display+0x30;
							}
						}
						else
						{
							str[4]= 0;
						}

						switch(chip_info.security){
							case 0:
								str[5] = '0';
								break;
							case 2:
								str[5] = '3';
								break;
							case 4:
								str[5] = '1';
								break;
							case 8:
								str[5] = '2';
								break;
							case 12:
								str[5] = '6';
								break;
							default:
								str[5] = 0;
								break;
						}
						str[6]='-';

						switch(chip_info.package){
							case 0:
								str[7] = 'L';
								break;
							case 1:
								str[7] = 'T';
								break;
							case 2:
								str[7] = 'Q';
								break;
							case 3:
								str[7] = 'M';
								break;
							case 4:
								str[7] = 'B';
								break;
							case 5:
								str[7] = 'C';
								break;
							default:
								str[7] = 0;
								break;
						}

						if(chip_info.sipdramsize != 0xff)
						{
							if(chip_info.sipdramsize == 0x0b)
							{
								str[8] = 'C';
							}
							else
							{
								str[8] = chip_info.sipdramsize + 0x30;
							}
						}
						else
						{
							str[8] = 0;
						}

						switch(chip_info.cavendor){
							case 0:
								str[9] = '0';
								break;
							case 1:
								str[9] = 'G';
								break;
							case 2:
								str[9] = 'C';
								break;
							case 3:
								str[9] = 'T';
								break;
							case 4:
								str[9] = 'A';
								break;
							case 5:
								str[9] = 'U';
								break;
							case 6:
								str[9] = 'P';
								break;
							case 7:
								str[9] = 'V';
								break;
							default:
								str[9] = 0;
								break;
						}
						if(chip_info.caversion != 0xff)
						{
							str[10] = 0x30 + (chip_info.caversion & 0x03);
						}
						else
						{
							str[10] = '0';
						}

						switch(chip_info.drminfo){
							case 0:
								str[11] = '0';
								break;
							case 1:
								str[11] = 'P';
								break;
							case 2:
								str[11] = 'W';
								break;
							case 3:
								str[11] = 'M';
								break;
							case 4:
								str[11] = 'A';
								break;
							default:
								str[11] = 0;
								break;
						}

						switch(chip_info.iplicense){
							case 0:
								str[12] = '1';
								break;
							case 3:
								str[12] = 'E';
								break;
							case 5:
								str[12] = 'F';
								break;
							case 7:
								str[12] = 'D';
								break;
							case 8:
								str[12] = 'X';
								break;
							case 9:
								str[12] = 'U';
								break;
							case 10:
								str[12] = 'W';
								break;
							case 11:
								str[12] = 'T';
								break;
							case 13:
								str[12] = 'M';
								break;
							case 14:
								str[12] = 'V';
								break;
							case 15:
								str[12] = '0';
								break;
							default:
								str[12] = 0;
								break;
						}
						str[13] = 0;
						copy_to_user((void __user *)arg, str, 63);
						break;
				}
			}
			break;
        default:
            return -ENOIOCTLCMD;
    }
#endif
    return 0;
}

//Debug
//extern int mt_clk_testsuite(void);
extern void mt_clk_dump_state(struct seq_file *p);
extern void mt_ana_dump_state(struct seq_file *p);

static mt_s32 misc_proc(struct seq_file *p, mt_void *v)
{
#if !defined(CONFIG_MT_CHIP_SYMPHONY4)
	unsigned long p_clk=0;
	unsigned int val1 = 0;
#endif
    unsigned int val2 = 0, chip_rev = 0;

    printk(KERN_INFO "\n-------Sysinfo Dump-------\n");
    chip_rev = symphony_get_chip_rev();
    val2 = chip_package_get();
    printk(KERN_INFO "cver:0x%x pkg:0x%x\n", chip_rev, val2);

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    crm_module_info_dump();
#else
    symphony_get_clock(HAL_DDRMC,&p_clk);
    printk(KERN_INFO "ddr_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_CPU0,&p_clk);
    printk(KERN_INFO "cpu0_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_CPU1,&p_clk);
    printk(KERN_INFO "cpu1_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_CPU2,&p_clk);
    printk(KERN_INFO "cpu2_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_HB,&p_clk);
    printk(KERN_INFO "ahb_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_PB,&p_clk);
    printk(KERN_INFO "apb_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_DMA,&p_clk);
    printk(KERN_INFO "dma_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_TSI,&p_clk);
    printk(KERN_INFO "tsi_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_SECURE,&p_clk);
    printk(KERN_INFO "secure_clk=%ld\n", p_clk);
    //symphony_get_clock(HAL_OCP,&p_clk);
    //printk("ocp_clk=%d\n", p_clk);
    symphony_get_clock(HAL_VDEC,&p_clk);
    printk(KERN_INFO "vdec_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_DISPLAY,&p_clk);
    printk(KERN_INFO "display_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_DI,&p_clk);
    printk(KERN_INFO "di_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_GPE,&p_clk);
    printk(KERN_INFO "gpe_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_JPEG,&p_clk);
    printk(KERN_INFO "jpeg_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_AUDIO_OUT,&p_clk);
    printk(KERN_INFO "aout_clk=%ld\n", p_clk);

    symphony_get_clock(HAL_UART0,&p_clk);
    printk(KERN_DEBUG "uart0_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_UART1,&p_clk);
    printk(KERN_DEBUG "uart1_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_SMC0,&p_clk);
    printk(KERN_DEBUG "smc0_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_SPI0,&p_clk);
    printk(KERN_DEBUG "spi0_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_IRDA,&p_clk);
    printk(KERN_DEBUG "irda_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_LEDKB,&p_clk);
    printk(KERN_DEBUG "ledkb_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_I2C,&p_clk);
    printk(KERN_DEBUG "i2c_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_TIMER,&p_clk);
    printk(KERN_DEBUG "timer_clk=%ld\n", p_clk);
    symphony_get_clock(HAL_WATCHDOG,&p_clk);
    printk(KERN_DEBUG "watchdog_clk=%ld\n\n", p_clk);
    val1 = readl((volatile unsigned long *)CLK_GATE_CFG0);
    printk(KERN_INFO "cpu1: %s\n", ((val1 >> 1) & 0x1)?"ON":"OFF");
    if(chip_rev >= CHIP_SYMPHONY2_A0)
    {
	   	val1 = readl((volatile unsigned long *)0xbf30f200);
    	printk(KERN_INFO "cpu2: %s\n", ((val1 >> 2) & 0x1)?"ON":"OFF");
    }
    else
    	printk(KERN_INFO "cpu2: %s\n", ((val1 >> 2) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "DDR: %s\n", ((val1 >> 4) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "DMA: %s\n", ((val1 >> 8) & 0x1)?"ON":"OFF");
    val1 = readl((volatile unsigned long *)CLK_GATE_CFG1);
    printk(KERN_INFO "CRYPTO: %s\n", ((val1 >> 8) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "TSI: %s\n", ((val1 >> 0) & 0x1)?"ON":"OFF");
    if(chip_rev >= CHIP_SYMPHONY2_A0)
    {
    	val1 = readl((volatile unsigned long *)0xBF30F0D0);
    	printk(KERN_INFO "DS_DES: %s\n", ((val1 >> 5) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DS_TDES: %s\n", ((val1 >> 4) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DS_AES: %s\n", ((val1 >> 3) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DS_CSA3: %s\n", ((val1 >> 2) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DS_CSA2: %s\n", ((val1 >> 1) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DS_SECHD1: %s\n", ((val1 >> 0) & 0x1)?"ON":"OFF");
    	val1 = readl((volatile unsigned long *)0xBF30F0E0);
    	printk(KERN_INFO "KL_PVR: %s\n", ((val1 >> 17) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "CRYPTO_DES: %s\n", ((val1 >> 9) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "CRYPTO_TDES: %s\n", ((val1 >> 8) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "CRYPTO_AES: %s\n", ((val1 >> 7) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "CRYPTO_SHA: %s\n", ((val1 >> 6) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "CRYPTO_RSA: %s\n", ((val1 >> 5) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "SECHD0: %s\n", ((val1 >> 4) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "KT: %s\n", ((val1 >> 3) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "KL_CW: %s\n", ((val1 >> 2) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "KDF: %s \n", ((val1 >> 1) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "OTP_PRELOAD: %s\n", ((val1 >> 0) & 0x1)?"ON":"OFF");
    }
    else
    {
    	val1 = readl((volatile unsigned long *)0xBF2702B8);
    	printk(KERN_INFO "TSI-CSA3.0: %s\n", ((val1 >> 0) & 0x1)?"OFF":"ON");
    }
    val1 = readl((volatile unsigned long *)CLK_GATE_CFG2);
    val2 = readl((volatile unsigned long *)REG_CLK_ANALOG);
    printk(KERN_INFO "AOUT: %s\n", ((val1 >> 24) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "SPDIF: %s\n", ((val1 >> 25) & 0x1)?"ON":"OFF");
    if(chip_rev >= CHIP_SYMPHONY2_A0)
    {
    	printk(KERN_INFO "HDMI dig: %s\n", ((val1 >> 10) & 0x1) == 1?"ON":"OFF");
   	printk(KERN_INFO "HDMI ana: %s\n", ((val2 >> 8) & 0xf) == 0?"ON":"OFF");
    }
    else
    	printk(KERN_INFO "HDMI: %s\n", ((val1 >> 10) & 0x1)?"ON":"OFF");
    if(chip_rev >= CHIP_SYMPHONY2_A0)
    {
    	val1 = readl((volatile unsigned long *)0xbf510018);
    	printk(KERN_INFO "EPHY: %s\n", ((val1 >> 1) & 0x1)?"ON": "OFF");
    }
    printk(KERN_INFO "VBI: %s\n", ((val1 >> 11) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "SDVENC: %s\n", ((val1 >> 8) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "HDVENC: %s\n", ((val1 >> 9) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "GRA: %s\n", ((val1 >> 3) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "DISP: %s\n", ((val1 >> 2) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "JPEG: %s\n", ((val1 >> 1) & 0x1)?"ON":"OFF");
    printk(KERN_INFO "VDEC: %s\n", ((val1 >> 0) & 0x1)?"ON":"OFF");
    val1 = readl((volatile unsigned long *)CLK_GATE_CFG3);
    val2 = readl((volatile unsigned long *)REG_CLK_ANALOG);
    printk(KERN_INFO "RMII: %s\n", ((val1 >> 5) & 0x1)?"ON":"OFF");
    if(chip_rev >= CHIP_SYMPHONY2_A0)
    {
    	printk(KERN_INFO "MAC: %s\n", ((val1 >> 4) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "USB0 dig: %s\n", ((val1 >> 0) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "USB1 dig: %s\n", ((val1 >> 1) & 0x1)?"ON":"OFF");
   	printk(KERN_INFO "USB0 ana: %s\n", ((val2 >> 1) & 0x1)?"OFF":"ON");
    	printk(KERN_INFO "USB1 ana: %s\n", ((val2 >> 2) & 0x1)?"OFF":"ON");
    	val1 = readl((volatile unsigned long *)0xBF153000);
    	printk(KERN_INFO "AO_LPMSET: %s\n", ((val1 >> 16) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_PINMUX: %s\n", ((val1 >> 15) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_GPIO: %s\n", ((val1 >> 14) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_ANA: %s\n", ((val1 >> 13) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_SECRAM: %s\n", ((val1 >> 10) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_KADC: %s\n", ((val1 >> 9) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_RTC: %s\n", ((val1 >> 8) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_MAILBOX: %s\n", ((val1 >> 7) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_TIMER1: %s\n", ((val1 >> 6) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_TIMER0: %s\n", ((val1 >> 5) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_AOMCU: %s\n", ((val1 >> 4) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_IRDA: %s\n", ((val1 >> 3) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_LEDKB: %s\n", ((val1 >> 2) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_FPI2C: %s\n", ((val1 >> 1) & 0x1)?"ON": "OFF");
    	printk(KERN_INFO "AO_FPSPI: %s\n", ((val1 >> 0) & 0x1)?"ON": "OFF");
    }
    else
    {
    	printk(KERN_INFO "MAC: %s\n", (((val1 >> 4) & 0x1) == 1 && ((val2 >> 30) & 0x1) == 0)?"ON":((((val1 >> 4) & 0x1) == 0 && ((val2 >> 30) & 0x1) == 1) ? "OFF":"Unknown"));
    	printk(KERN_INFO "USB0: %s\n", (((val1 >> 0) & 0x1) == 1 && ((val2 >> 1) & 0x1) == 0)?"ON":((((val1 >> 0) & 0x1) == 0 && ((val2 >> 1) & 0x1) == 1) ? "OFF":"Unknown"));
    	printk(KERN_INFO "USB1: %s\n", (((val1 >> 1) & 0x1) == 1 && ((val2 >> 2) & 0x1) == 0)?"ON":((((val1 >> 1) & 0x1) == 0 && ((val2 >> 2) & 0x1) == 1) ? "OFF":"Unknown"));
    }
    if(chip_rev >= CHIP_SYMPHONY2_A0)
    {
    	val1 = readb((volatile u8 *)0xbf5b0b04);
    	printk(KERN_INFO "DEMO-J83B: %s\n", ((val1 >> 2) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DEMO-C: %s\n", ((val1 >> 0) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DEMO-S/S2/S2X: %s\n", ((val1 >> 3) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DEMO-T: %s\n", ((val1 >> 1) & 0x1)?"ON":"OFF");
    	printk(KERN_INFO "DEMO-T2: %s\n", ((val1 >> 4) & 0x1)?"ON":"OFF");
    }
    else
    {
   		val1 = readl((volatile unsigned long *)CLKGEN_PDSYS_REG);
    	val2 = readl((volatile unsigned long *)0xbf138004);
    	printk(KERN_INFO "DEMO-C: %s\n", (((val1 >> 24) & 0x1) == 1 && ((val2 >> 0) & 0x1) == 1)?"OFF":((((val1 >> 24) & 0x1) == 0 && ((val2 >> 0) & 0x1) == 0) ? "ON":"Unknown"));
    	printk(KERN_INFO "DEMO-S: %s\n", (((val1 >> 25) & 0x1) == 1)?"OFF":"Unknown");
    	printk(KERN_INFO "PLL_CPU: %s\n", ((val1 >> 30) & 0x1)?"OFF":"ON");
    }
    val1 = readl((volatile unsigned long *)REG_CLK_ANALOG);
    printk(KERN_INFO "CADC: %s\n", ((val1 >> 20) & 0x1)?"OFF":"ON");
    printk(KERN_INFO "SADC: %s\n", ((val1 >> 21) & 0x1)?"OFF":"ON");
    printk(KERN_INFO "ADAC: %s\n", ((val1 >> 4) & 0x7) == 0?"ON":"OFF");
    printk(KERN_INFO "VDAC0: %s\n", ((val1 >> 12) & 0x1)?"OFF":"ON");
    printk(KERN_INFO "VDAC1: %s\n", ((val1 >> 13) & 0x1)?"OFF":"ON");
    printk(KERN_INFO "VDAC2: %s\n", ((val1 >> 14) & 0x1)?"OFF":"ON");
    printk(KERN_INFO "VDAC3: %s\n", ((val1 >> 15) & 0x1)?"OFF":"ON");
    if(chip_rev >= CHIP_SYMPHONY2_A0)
    {
    	printk(KERN_INFO "RNG1: %s\n", ((val1 >> 24) & 0x1)?"OFF":"ON");
    	printk(KERN_INFO "RNG2: %s\n", ((val1 >> 29) & 0x1)?"OFF":"ON");
  	printk(KERN_INFO "TSENSOR: %s\n", ((val1 >> 27) & 0x1)?"OFF":"ON");
    	printk(KERN_INFO "PM: %s\n", ((val1 >> 22) & 0x1)?"OFF":"ON");
    	val1 = readl((volatile unsigned long *)0xbf157004);
    	printk(KERN_INFO "PLL_CPU: %s\n", ((val1 >> 31) & 0x1)?"OFF":"ON");
    }
    else
    	printk(KERN_INFO "RNG: %s\n", ((val1 >> 24) & 0x1)?"OFF":"ON");
    val1 = symphony_temperature_get();
    printk(KERN_INFO "temperature: val:%u %d.%d\n", val1, val1 /1000, val1 % 1000);
    printk(KERN_INFO "\nreg:0x%x val:0x%x \n", CLK_SYS_CFG, readl((volatile unsigned long *)CLK_SYS_CFG));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_CORE_CFG, readl((volatile unsigned long *)CLK_CORE_CFG));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_DMA_CFG, readl((volatile unsigned long *)CLK_DMA_CFG));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_TRANSPORT_CFG, readl((volatile unsigned long *)CLK_TRANSPORT_CFG));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_AUDIO_CFG, readl((volatile unsigned long *)CLK_AUDIO_CFG));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_GATE_CFG0, readl((volatile unsigned long *)CLK_GATE_CFG0));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_GATE_CFG1, readl((volatile unsigned long *)CLK_GATE_CFG1));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_GATE_CFG2, readl((volatile unsigned long *)CLK_GATE_CFG2));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLK_GATE_CFG3, readl((volatile unsigned long *)CLK_GATE_CFG3));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", REG_CLK_ANALOG, readl((volatile unsigned long *)REG_CLK_ANALOG));
    printk(KERN_INFO "reg:0x%x val:0x%x \n", CLKGEN_PDSYS_REG, readl((volatile unsigned long *)CLKGEN_PDSYS_REG));
#endif

//Debug
//	(void)mt_clk_testsuite();
	mt_clk_dump_state(p);
	mt_ana_dump_state(p);

    return 0;
}
#ifndef CONFIG_MT_CHIP_SYMPHONY4
extern unsigned long symp1_int_count[CONCERTO_PIC_NUM_IRQ];
char symp1_int_name[CONCERTO_PIC_NUM_IRQ][10] = {
                            "Timer0", "Timer1", "Timer2", "Timer3",
                            "MBOX_AV", "SPDMA", "DMA", "SECURE",
                            "MBOX_SEC", "TSI_PVR", "TSI_GLB", "AO_T0",
                            "TSI_SF", "VDEC", "JPEG", "AOUT",
                            "AVSYNC", "GRA_ENG", "AO_T1", "HD_TOP",
                            "HD_BOT", "SD_VENC", "HDMI", "GPIO_G",
                            "MAC0", "MAC1", "UART0", "UART1",
                            "I2C1", "AO_GPIO_G", "USB0", "USB1",
                            "MBOX_AO", "DDR", "GPIO", "AO_GPIO",
                            "GLITCH", "I2C0", "I2C_FP", "LEDKB",
                            "IRDA", "SMC0", "AKL", "SPI0",
                            "SPI1", "SPI2", "TSENSOR", "KADC"};


static int int_proc(struct seq_file *p, mt_void *v)
{
    int idx = 0;
    PROC_PRINT(p, "INT\t TIMES\t Name\n");
    for(idx = 0; idx < CONCERTO_PIC_NUM_IRQ; idx++)
    {
         PROC_PRINT(p, "%d\t %lu\t %s\n", idx, symp1_int_count[idx], symp1_int_name[idx]);
    }
    return 0;
}
#endif
/*
*  Kernel Interfaces
*/

static struct file_operations misc_fops =
{
    .owner   = THIS_MODULE,
    .llseek  = no_llseek,
    //  .write      = misc_write,
    .unlocked_ioctl   = misc_ioctl,
    .open    = misc_open,
    .release = misc_release,
};


static mt_device_s g_misc_register_data;
//unsigned int  mt_get_pic0_base(void);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) && defined(CONFIG_PM_SLEEP)

extern void crm_avcpu_resume(void);

static int mt_misc_pm_notify(struct notifier_block *nb,
							 unsigned long action, void *data)
{
	MT_INFO_MISC("call mt_misc_pm_notify(%lu)\n", action);

	if (PM_SUSPEND_PREPARE == action)
	{
		//do nothing
	}
	else if (PM_POST_SUSPEND == action)
	{
#ifdef CONFIG_TEE
		//TODO
#else
		//do resume avcpu
		crm_avcpu_resume();
#endif
	}

	return 0;
}

static int crm_pm_suspend(void)
{
    return 0;
}

extern void crm_module_clk_resume(void);
extern void crm_analog_config(void);
extern void mt_ana_batch_config(void);

static void crm_pm_resume(void)
{
    MT_INFO_MISC("crm_pm_resume\n");
    mt_ana_batch_config();
    crm_analog_config();
    crm_module_clk_resume();
}

static struct syscore_ops crm_pm_syscore_ops = {
    .suspend  = crm_pm_suspend,
    .resume   = crm_pm_resume,
};

#endif

mt_s32 __init misc_drv_modinit(mt_void)
{
    int ret = 0;
    mt_proc_entry_t *item = NULL;
    mt_drv_proc_t misc_proc_ops;
#ifndef CONFIG_MT_CHIP_SYMPHONY4
    mt_drv_proc_t int_proc_ops;
#endif
    MT_INFO_MISC("\n ... misc init  ... \n");
    (mt_void)mt_drv_module_register(MT_ID_MISC, "misc", NULL);
    //sn_misc_base = mt_get_pic0_base();
    //MT_INFO_MISC("\n ... misc init  .sn_misc_base=%x.. \n", sn_misc_base);


    /* Check that the default_margin value is within it's range ; if not reset to the default */

    snprintf(g_misc_register_data.devfs_name, sizeof(g_misc_register_data.devfs_name), UMAP_DEVNAME_MISC);
    g_misc_register_data.minor = UMAP_MIN_MINOR_MISC;
    g_misc_register_data.owner = THIS_MODULE;
    g_misc_register_data.fops   = &misc_fops;
    g_misc_register_data.drvops = NULL;
    if (mt_drv_dev_register(&g_misc_register_data) < 0)
    {
        MT_ERR_MISC(" misc mt_drv_dev_register err (err=%d)\n", ret);
        return MT_FAILURE;
    }

    memset(&misc_proc_ops, 0, sizeof(mt_drv_proc_t));
    misc_proc_ops.fnRead = misc_proc;
    item = mt_drv_proc_add_module(MT_MOD_MISC, &misc_proc_ops, NULL);
    if(!item)
    {
        return -1;
    }
#ifndef CONFIG_MT_CHIP_SYMPHONY4
    memset(&int_proc_ops, 0, sizeof(mt_drv_proc_t));
    int_proc_ops.fnRead = int_proc;
    item = mt_drv_proc_add_module("interrupt", &int_proc_ops, NULL);
    if(!item)
    {
        return -1;
    }
#endif
    //symphony_pinmux_preset();

#if defined(CONFIG_MT_CHIP_SYMPHONY4) && defined(CONFIG_PM_SLEEP)
    pm_notifier(mt_misc_pm_notify, 0);

    register_syscore_ops(&crm_pm_syscore_ops);

#endif

    return ret;
}


mt_void __exit misc_drv_modexit(mt_void)
{
    MT_INFO_MISC("------------------------\n");
#if 0
    mt_drv_module_unregister(MT_ID_MISC);
    mt_drv_dev_unregister(&g_misc_register_data);
    //misc_set_timeout(0, 0);
#endif
}

