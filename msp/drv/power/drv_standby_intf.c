/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/of_platform.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/poll.h>
//#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/irq.h>
#include "mt_mach/irq.h"
#include <linux/suspend.h>


//FIXME: for symphony1/2/4, how about Aria?
#include "mt_mach/clock.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "mt_osal.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_module.h"
#include "mt_drv_pm.h"
#include "drv_pm_ioctl.h"

#include "mt_drv_log.h"
#include "mt_module_debug.h"
#include "mt_unf_ir.h"
#include "mt_drv_clock.h"


struct mt_aomcu_dev
{
	//struct device *dev;
	void __iomem *base; /* virtual */
	//ulong base;
	//struct mutex mutex_dmac;
};


struct mt_pm_device {
	struct mt_aomcu_dev ao_dev;
	struct device *dev;
	struct cdev cdev;
	struct class *pm_class;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

static struct mt_pm_device *g_pm_drv = NULL;

//static mt_device_s g_PmRegisterData;
mt_u16 *p_standby_fw;
mt_u32 standby_fw_size = 0;

trans_info_standby_t standby_info;
static aomcu_fw_info_t aofw_info;

MT_U8  fp_type = 0xff;


#define  WRITE_REG_B(Addr, Value)			HAL_PUT_U8((volatile u8*)(Addr), (u8)(Value))
#define  READ_REG_B(Addr)					HAL_GET_U8((volatile u8*)(Addr))
#define  WRITE_REG(Addr, Value)				HAL_PUT_U32((volatile u32*)(Addr), (u32)(Value))
#define  READ_REG(Addr)						HAL_GET_U32((volatile u32*)(Addr))

#define I2C_CTR_EN							0x80

#define FD650_ROM_PATH						"aomcu_rom/aomcu_fd650_original.rom"
#define CT1642_ROM_PATH						"aomcu_rom/aomcu_ct1642_original.rom"
#define OSC_ROM_PATH						"aomcu_rom/aomcu_osc_original.rom"
#define NOFP_ROM_PATH						"aomcu_rom/aomcu_nofp.rom"
#define NOFP_ORI_ROM_PATH					"aomcu_rom/aomcu_nofp_original.rom"
#define GPIO_ROM_PATH						"aomcu_rom/aomcu_gpio_original.rom"
#define TT1629_ROM_PATH						"aomcu_rom/aomcu_tt1629.rom"
#define TT1629B_ROM_PATH					"aomcu_rom/aomcu_tt1629b.rom"
#define PT6393_ROM_PATH						"aomcu_rom/aomcu_pt6393_original.rom"
#define STR_ROM_PATH						"aomcu_rom/aomcu_original_str.rom"
#define FD650_KADC_ROM_PATH					"aomcu_rom/aomcu_fd650_keyadc.rom"



#define I2C2_BASE_ADDR						SYMPHONY_IO_VA(0xBF158000UL)
#define R_PM_BASE_ADDR						SYMPHONY_IO_VA(0xBF150000UL)

#define R_I2C2_PRER_H						(I2C2_BASE_ADDR + 0x18)
#define R_I2C2_PRER_L						(I2C2_BASE_ADDR + 0x1c)
#define R_I2C2_CTR							(I2C2_BASE_ADDR + 0x10)

#define BUS_CLK_SEL							SYMPHONY_IO_VA(0xBF508004UL)
#define ARM_PLL_CFG							SYMPHONY_IO_VA(0xBF5A001CUL)
#define AO_TIMER0_MODE						SYMPHONY_IO_VA(0xBF15C090UL)


#if 0
#define PM_LPM_GET_WAKE_UP_FLAG				(R_PM_BASE_ADDR+0x10)  //Use only bit0 ~bit2
#define PM_LPM_MESSAGE2AO_0					(R_PM_BASE_ADDR+0x40)
#define PM_LPM_MESSAGE2AO_1					(R_PM_BASE_ADDR+0x44)
#define PM_LPM_MESSAGE2AO_2					(R_PM_BASE_ADDR+0x48)
#define PM_LPM_MESSAGE2AO_3					(R_PM_BASE_ADDR+0x4C)
#define PM_LPM_MESSAGE2AO_4					(R_PM_BASE_ADDR+0x20)
#define PM_LPM_MESSAGE2AO_5					(R_PM_BASE_ADDR+0x38)
#define PM_LPM_AOMCU_EN						(R_PM_BASE_ADDR+0x50)
#define PM_AOMCU_RAM_ADDR					(R_PM_BASE_ADDR+0x10000)
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#define PM_I2C_PINMAX_ADDR					(R_PM_BASE_ADDR+0xb400)
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#define PM_I2C_CLK_PINMAX_ADDR				(R_PM_BASE_ADDR+0xb400)
#define PM_I2C_DAT_PINMAX_ADDR				(R_PM_BASE_ADDR+0xb404)
#endif
#endif
#define PM_LPM_GET_WAKE_UP_FLAG				(0x10)  //Use only bit0 ~bit2
#define PM_LPM_AOMCU_MESSAGE				(0x24)
#define PM_LPM_MESSAGE2AO_0					(0x40)
#define PM_LPM_MESSAGE2AO_1					(0x44)
#define PM_LPM_MESSAGE2AO_2					(0x48)
#define PM_LPM_MESSAGE2AO_3					(0x4C)
#define PM_LPM_MESSAGE2AO_4					(0x20)
#define PM_LPM_MESSAGE2AO_5					(0x38)
#define PM_LPM_AOMCU_EN						(0x50)
#define PM_AOMCU_RAM_ADDR					(0x10000)
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#define PM_I2C_PINMAX_ADDR					(0xb400)
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#define PM_I2C_CLK_PINMAX_ADDR				(0xb400)
#define PM_I2C_DAT_PINMAX_ADDR				(0xb404)
#endif


#define PROC_PARAM_MAXLEN (64)
//#define SYM6_GPIO_CHIP_TEST		1

#ifdef SYM6_GPIO_CHIP_TEST
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>

static void sym6_chip_test_3626(void);
static void sym6_chip_test_mailbox(void);
static void sym6_chip_test_timer(void);
static void sym6_chip_test_3632(void);

extern void symphony_wdt_reset(void);
#endif

extern void drv_gpio_standby(void);

extern void irda_symphony_enable_filteren(void);
extern void irda_symphony_clk_reset(u32 clk);
extern void (*pm_power_off)(void);
static void montage_enter_standby(void);

static inline void stby_writel(volatile u32* addr, u32 val)
{
	HAL_PUT_U32(addr, val);
}

static inline u32 stby_readl(volatile u32* addr)
{
	return HAL_GET_U32(addr);
}

static void i2c_fp_set_pinmux(void)
{
     u32 val = 0;
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	//set pinmux for fp_clk and fp_data

	u32 reg = g_pm_drv->ao_dev.base + PM_I2C_PINMAX_ADDR;

	val = stby_readl((volatile u32*)reg);
	val &= ~0xff;
	val |= 0x33;
	stby_writel((volatile u32*)reg, val);
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	val = stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_I2C_CLK_PINMAX_ADDR));
	val &= ~0x7;
	val |= 0x3;
	stby_writel((volatile u32*)(g_pm_drv->ao_dev.base + PM_I2C_CLK_PINMAX_ADDR), val);
	val = stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_I2C_DAT_PINMAX_ADDR));
	val &= ~0x7;
	val |= 0x3;
	stby_writel((volatile u32*)(g_pm_drv->ao_dev.base + PM_I2C_DAT_PINMAX_ADDR), val);
#endif
}

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
extern long symphony4_get_clock(unsigned long m_id, unsigned long  *p_clk);
static void i2c_fp_init(void)
{
	u8 data = 0;
	u32 tmp = 0;
	unsigned long apb_clk = 0;
	u32 busclk_khz = 100;

	WRITE_REG_B(R_I2C2_CTR, 0);

	symphony_get_clock((unsigned long)HAL_PB, &apb_clk);
	tmp = ((apb_clk / (5000 * (busclk_khz))) - 1);
	WRITE_REG_B(R_I2C2_PRER_L, (u8)tmp);
	WRITE_REG_B(R_I2C2_PRER_H, (u8)(tmp >> 8));

	MT_INFO_PM(KERN_INFO"i2c_fp_init  apb_clk = 0x%x , tmp = %u\n", (u32)apb_clk, tmp);

	/* i2c function enable and if use interrupt mode, enable i2c interrupt */
	data = I2C_CTR_EN;
	WRITE_REG_B(R_I2C2_CTR, data);
}
#endif

mt_s32 standby_cpu_attach_fw_fd650_original_symphony(void)
{
	static mt_u16 standby_buff_fd650[] =
	{
		#include FD650_ROM_PATH
	};

	p_standby_fw = standby_buff_fd650;
	standby_fw_size = sizeof(standby_buff_fd650)/sizeof(mt_u16);
	i2c_fp_set_pinmux();
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	i2c_fp_init();
#endif
	MT_INFO_PM("standby_cpu_attach_fw_fd650_original_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_pt6393_original_symphony(void)
{
	static mt_u16 standby_buff_pt6393[] =
	{
		#include PT6393_ROM_PATH
	};

	p_standby_fw = standby_buff_pt6393;
	standby_fw_size = sizeof(standby_buff_pt6393)/sizeof(mt_u16);
	MT_INFO_PM("standby_cpu_attach_fw_pt6393_original_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_no_fp_symphony(void)
{
	static mt_u16 standby_buff_nofp[] =
	{
		#include NOFP_ROM_PATH
	};

	p_standby_fw = standby_buff_nofp;
	standby_fw_size = sizeof(standby_buff_nofp)/sizeof(mt_u16);
	MT_INFO_PM("standby_cpu_attach_fw_no_fp_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_no_fp_original_symphony(void)
{
	static mt_u16 standby_buff_nofp_ori[] =
	{
		#include NOFP_ORI_ROM_PATH
	};

	p_standby_fw = standby_buff_nofp_ori;
	standby_fw_size = sizeof(standby_buff_nofp_ori)/sizeof(mt_u16);
	MT_INFO_PM("standby_cpu_attach_fw_no_fp_original_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_osc_original_symphony(void)
{
	static mt_u16 standby_buff_osc_ori[] =
	{
		#include OSC_ROM_PATH
	};

	p_standby_fw = standby_buff_osc_ori;
	standby_fw_size = sizeof(standby_buff_osc_ori)/sizeof(mt_u16);
	MT_INFO_PM("standby_cpu_attach_fw_osc_original_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_ct1642_original_symphony(void)
{
	static mt_u16 standby_buff_ct1642[] =
	{
		#include CT1642_ROM_PATH
	};

	p_standby_fw = standby_buff_ct1642;
	standby_fw_size = sizeof(standby_buff_ct1642)/sizeof(mt_u16);
	MT_INFO_PM("standby_cpu_attach_fw_ct1642_original_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_aogpio_original_symphony(void)
{
	static mt_u16 standby_buff_gpio[] =
	{
		#include GPIO_ROM_PATH
	};

	p_standby_fw = standby_buff_gpio;
	standby_fw_size = sizeof(standby_buff_gpio)/sizeof(mt_u16);
	MT_INFO_PM("standby_cpu_attach_fw_aogpio_original_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_tt1629b_symphony(void)
{
	static mt_u16 standby_buff_tt1629b[] =
	{
		#include TT1629B_ROM_PATH
	};

	p_standby_fw = standby_buff_tt1629b;
	standby_fw_size = sizeof(standby_buff_tt1629b)/sizeof(u16);
	MT_INFO_PM("standby_cpu_attach_fw_tt1629b_symphony##\n");
	return MT_SUCCESS;
}


mt_s32 standby_cpu_attach_fw_tt1629_symphony(void)
{
	static mt_u16 standby_buff_tt1629[] =
	{
		#include TT1629_ROM_PATH
	};

	p_standby_fw = standby_buff_tt1629;
	standby_fw_size = sizeof(standby_buff_tt1629)/sizeof(u16);
	MT_INFO_PM("standby_cpu_attach_fw_tt1629_symphony##\n");
	return MT_SUCCESS;
}

mt_s32 standby_cpu_attach_fw_str_symphony4(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	static mt_u16 standby_buff_str[] =
	{
		#include STR_ROM_PATH
	};

	//printk(KERN_ERR "[%s %d]enter\n", __FUNCTION__, __LINE__);
	p_standby_fw = standby_buff_str;
	standby_fw_size = sizeof(standby_buff_str)/sizeof(u16);
	MT_PRINT("standby_buff_str_poweroff##\n");
#endif

	return MT_SUCCESS;
}

 mt_s32 standby_cpu_attach_fw_fd650_kadc_symphony(void)
 {
	 static mt_u16 standby_buff_fd650_kadc[] =
	 {
		#include FD650_KADC_ROM_PATH
	 };

	 p_standby_fw = standby_buff_fd650_kadc;
	 standby_fw_size = sizeof(standby_buff_fd650_kadc)/sizeof(mt_u16);
	 i2c_fp_set_pinmux();
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	 i2c_fp_init();
#endif
	 MT_INFO_PM("standby_cpu_attach_fw_fd650_kadc_symphony##\n");
	 return MT_SUCCESS;
 }

 void symphony_dump(int count)
{
	u16* base_addr = NULL;
	int i ;

	base_addr = (u16*)((u32)AOMCU_RAM_ADDR);
	for(i = 0 ; i < count ; i++)
	{
		MT_INFO_PM("[0x%x] ",*base_addr++);
	}
	MT_INFO_PM("\n ****ok ***\n ");
}


mt_u32 load_mcu(int need_dump)
{
	int i =0 ;
	u32 size = 0;
	u16 *p_data = NULL ;

	volatile u32* base_addr = NULL;

	if ((NULL == g_pm_drv) || (NULL == g_pm_drv->ao_dev.base))
	{
		MT_ERR_PM("\n###aomcu %s.%d aomcu SRAM Addr is NULL!!!\n",__FUNCTION__,__LINE__);
		return MT_FAILURE;
	}

	mt_clk_enable(MT_CLK_AO_MCU);
	mdelay(10);

	base_addr = (volatile u32*)(g_pm_drv->ao_dev.base + PM_AOMCU_RAM_ADDR);

	if (aofw_info.extern_fw)
	{
		size = aofw_info.fw_size / (sizeof(u16));
		p_data = (u16*)aofw_info.p_fw;
		MT_INFO_PM("\n###aomcu %s.%d extern_fw:%d size:0x%x\n",__FUNCTION__,__LINE__,aofw_info.extern_fw,size);
	}
	else
	{
		size = standby_fw_size;
		p_data = p_standby_fw;
	}

	if(standby_fw_size == 0)
		MT_ASSERT(0);


	if(p_data != NULL)
	{

		for(i =0;i<size;i++)
		{

			volatile u32*temp = base_addr;

			//*(volatile u32*)base_addr++ = (u32)p_data[i];

			stby_writel(base_addr++,(u32)p_data[i]);
			//printk("standby_buff_aomcu %x :  %x\n",i,p_data[i]);
			if(((u32)p_data[i]) != (stby_readl(temp)))
			{
				//printk("0x%x ,0x%x,0x%x\n",i,p_data[i],stby_readl(temp));
				return MT_FAILURE;
			}
		}
	}
	else
	{
		MT_ERR_PM("download.....unsupport bin\n");
		return MT_FAILURE;
	}

	if(need_dump == 1)
	{
		symphony_dump(size);
	}

	return MT_SUCCESS;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
/* read vcode */
static void _read_vcode(u8 *vcode0, u8 *vcode1)
{
	*vcode0 = READ_REG_B(R_PM_BASE_ADDR + 0x2201);
	*vcode1 = READ_REG_B(R_PM_BASE_ADDR + 0x2101);
}

/*
 * read and store vcode into PM_LPM_MESSAGE2AO register
 */
static int store_vcode_msg(void)
{
	u8 vcode0, vcode1;

	_read_vcode(&vcode0, &vcode1);

	pr_info("vcode before standby core=%u,cpu=%u\n", vcode0, vcode1);

#if 1
	if (vcode0 > 41)
		vcode0 -= 40;
	else
		vcode0 = 1;
#endif

	//FIXME: cpu vcode = vcode0 too in mcu!
	pr_info("vcode when standby core=%u,cpu=%u\n", vcode0, vcode0);

	/* Stored into PM_LPM_MESSAGE2AO - 0x29 */
	WRITE_REG_B(R_PM_BASE_ADDR + 0x29, vcode0);

	pr_err("reg(0xbf1500%x) = 0x%02x\n\n", 0x29, READ_REG_B(R_PM_BASE_ADDR + 0x29));

	return 0;
}
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
//powerdown some modules before standby
void sym4_pd_module_before_standby(void)
{
	ulong va_addr;
	u32 reg_value;

	printk(KERN_ERR "[%s %d]close some modules before standby\n", __FUNCTION__, __LINE__);

	va_addr = SYMPHONY_IO_VA(0xBF5D0120);
	reg_value = stby_readl((volatile u32 *)va_addr);
	//reg_value |= (1<<11);//power down Panther2 clock-gen,BF5D0120H[11]=1,only for sym4,sym6 moved this config
	reg_value |= (1<<1);//hdmi dcc power down
	stby_writel((volatile u32 *)va_addr, reg_value);

	va_addr = SYMPHONY_IO_VA(0xBF5D0144);//power down HDMI SSC clock-gen, BF5D0144H[4]=1
	reg_value = stby_readl((volatile u32 *)va_addr);
	reg_value |= (1<<4);
	stby_writel((volatile u32 *)va_addr, reg_value);

#if 0
	//sym6 analog_pd_reg can not find this define
	va_addr = SYMPHONY_IO_VA(0xBF5D011C);//power down ephy analog, BF5D011C[25]=0
	reg_value = stby_readl((volatile u32 *)va_addr);
	reg_value &= (~(1<<25));
	stby_writel((volatile u32 *)va_addr, reg_value);
#endif

	va_addr = SYMPHONY_IO_VA(0xBF590020);//smc power off, BF590020[0]=0
	reg_value = stby_readl((volatile u32 *)va_addr);
	reg_value &= (~(1<<0));
	stby_writel((volatile u32 *)va_addr, reg_value);

#if 0
	//need keep?
	va_addr = SYMPHONY_IO_VA(0xBF5900AC);//smc req<7:0>, BF5900AC[7:0] = 0
	reg_value = stby_readl((volatile u32 *)va_addr);
	reg_value &= (~0xFF);
	stby_writel((volatile u32 *)va_addr, reg_value);

	va_addr = SYMPHONY_IO_VA(0xBF5900B4);//3V mode, BF5900b4[0]=0
	reg_value = stby_readl((volatile u32 *)va_addr);
	reg_value &= (~(1<<0));
	stby_writel((volatile u32 *)va_addr, reg_value);
#endif

#if 0
//sym6 standby no need switch clk
//#ifdef CONFIG_CPU_CLK_SYMPHONY4
	//to be confirm?
	mt_apcpu_prepare_suspend(576000000);//change to backup clk
//#endif

	va_addr = SYMPHONY_IO_VA(0xBF5A001C);//power down armpll, BF5A001C[0]=_1
	reg_value = stby_readl((volatile u32 *)va_addr);
	reg_value |= 0x1;
	//reg_value &= (~(1<<0));
	stby_writel((volatile u32 *)va_addr, reg_value);
#endif
}
#endif

mt_u32 call_standbycpu_symphony(void)
{
	u8 *p_standby_info = (u8*)&standby_info;
	u8 i=0, len = 0;

    irda_symphony_enable_filteren();

	drv_gpio_standby();

	if(MT_SUCCESS != load_mcu(0))
	{
		MT_ERR_PM("load standby bin error ............\n");
		return MT_FAILURE;
	}

	if ((NULL == g_pm_drv) || (NULL == g_pm_drv->ao_dev.base))
	{
		MT_ERR_PM("\n###aomcu %s.%d aomcu base Addr is NULL!!!\n",__FUNCTION__,__LINE__);
		return MT_FAILURE;
	}

	len = sizeof(trans_info_standby_t);
	printk("[%s %d]len=%d\n", __FUNCTION__, __LINE__, len);

	//write according to the length of struct, and write one byte instead of 4 bytes every time,
	//because it will cause global-out-of-bounds error when open sanitize to check memory
	if (len <= 16)//0<len<=16
	{
		for (i=0; i<len; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0+i), (u8)(*p_standby_info++));
		}
	}
	else if (len <= 20)//16<len<=20
	{
		for (i=0; i<16; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0+i), (u8)(*p_standby_info++));
		}

		for (i=0; i<len-16; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_4+i), (u8)(*p_standby_info++));
		}
	}
	else//len>20
	{
		if (len > 24)//at present there are only 6 registers to use
		{
			len = 24;
		}

		for (i=0; i<16; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0+i), (u8)(*p_standby_info++));
		}

		for (i=0; i<4; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_4+i), (u8)(*p_standby_info++));
		}

		for (i=0; i<len-20; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_5+i), (u8)(*p_standby_info++));
		}
	}

	printk(KERN_ERR "\nreg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_0, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_1, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_1)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_2, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_2)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_3, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_3)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_4, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_4)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n\n", PM_LPM_MESSAGE2AO_5, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_5)));

	mdelay(20);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	store_vcode_msg();
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	sym4_pd_module_before_standby();
#endif

#if 1
// let TEEOS or PM to enable MCU
	return 0;
#endif

	stby_writel((volatile u32 *)(g_pm_drv->ao_dev.base + PM_LPM_AOMCU_EN), 1);
	mdelay(5);

	/*For tee standby*/
	if (!(stby_readl((volatile u32 *)(g_pm_drv->ao_dev.base + PM_LPM_AOMCU_EN)) & 1))
	{
		if(pm_power_off)
		{
   			pm_power_off();
		}
	}

//	*((volatile u32 *)LPM_AOMCU_EN) = 1;   // standby cpu work@@@

	return MT_FAILURE; // can not go here....
}

mt_u32 call_standbycpu_suspend(void)
{
	u8 *p_standby_info = (u8*)&standby_info;

	u8 i=0, len = 0;
	u8 kadc_info = 0;

    irda_symphony_enable_filteren();

	drv_gpio_standby();

	if(MT_SUCCESS != load_mcu(0))
	{
		MT_ERR_PM("load standby bin error ............\n");
		return MT_FAILURE;
	}

	if ((NULL == g_pm_drv) || (NULL == g_pm_drv->ao_dev.base))
	{
		MT_ERR_PM("\n###aomcu %s.%d aomcu base Addr is NULL!!!\n",__FUNCTION__,__LINE__);
		return MT_FAILURE;
	}

	len = sizeof(trans_info_standby_t);
	printk("[%s %d]len=%d\n", __FUNCTION__, __LINE__, len);

	kadc_info = (standby_info.standby_param >> 1) & 0x03;
	if (KADC_KEYS_DISABLE != kadc_info)
	{
		standby_info.standby_param |= (2 << 1);//set kadc type to 7 keys
	}

	//if not set wakeup info, we set default value(1min or key1 to wakeup)
	if ((0 == standby_info.standby_config_info) && (0 == standby_info.wake_up_key))
	{
		standby_info.wake_min = 1;
		//standby_info.wake_sec = 10;
		standby_info.standby_config_info |= (0x1 << 7);

		standby_info.wake_up_key = 1;
	}

	//write according to the length of struct, and write one byte instead of 4 bytes every time,
	//because it will cause global-out-of-bounds error when open sanitize to check memory
	if (len <= 16)//0<len<=16
	{
		for (i=0; i<len; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0+i), (u8)(*p_standby_info++));
		}
	}
	else if (len <= 20)//16<len<=20
	{
		for (i=0; i<16; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0+i), (u8)(*p_standby_info++));
		}

		for (i=0; i<len-16; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_4+i), (u8)(*p_standby_info++));
		}
	}
	else//len>20
	{
		if (len > 24)//at present there are only 6 registers to use
		{
			len = 24;
		}

		for (i=0; i<16; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0+i), (u8)(*p_standby_info++));
		}

		for (i=0; i<4; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_4+i), (u8)(*p_standby_info++));
		}

		for (i=0; i<len-20; i++)
		{
			WRITE_REG_B((g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_5+i), (u8)(*p_standby_info++));
		}
	}

	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_0, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_0)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_1, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_1)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_2, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_2)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_3, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_3)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n", PM_LPM_MESSAGE2AO_4, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_4)));
	printk(KERN_ERR "reg(0xbf1500%x) = 0x%08x\n\n", PM_LPM_MESSAGE2AO_5, stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_MESSAGE2AO_5)));

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	store_vcode_msg();
#endif

	return MT_FAILURE; // can not go here....
}

mt_s32 PM_Open(struct inode *inode, struct file *filp)
{
	MT_INFO_PM("PM_Open DO!");
	return MT_SUCCESS;
}

void  standby_aomcu_switch_clk_osc(void)
{
	u32 val = 0;

#if (defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2))
	u32 result = 0;

	writel(0x13c68000, (volatile u32 *)0xbf314040);
	writel(0x13c68400, (volatile u32 *)0xbf314040);
	writel(0x13c68000, (volatile u32 *)0xbf314040);

	//write 0x1f5d00f4[7]=1
	val = readl((volatile u32 *)0xBF5D00F4);
	val |= 1<< 7;
	writel(val, (volatile u32 *)0xBF5D00F4);

	//write 0x1f5d00f4[5]=1
	val = readl((volatile u32 *)0xBF5D00F4);
	val |= 1<< 5;
	writel(val, (volatile u32 *)0xBF5D00F4);

	//write 0x1f5d00f4[6]=0
	val = readl((volatile u32 *)0xBF5D00F4);
	val &= ~(1<< 6);
	writel(val, (volatile u32 *)0xBF5D00F4);

	//write 0x1f5d00f4[4]=0
	val = readl((volatile u32 *)0xBF5D00F4);
	val &= ~(1<< 4);
	writel(val, (volatile u32 *)0xBF5D00F4);

	msleep(5);

	//write 0x1f5d00f4[4]=1
	val = readl((volatile u32 *)0xBF5D00F4);
	val |= 1<< 4;
	writel(val, (volatile u32 *)0xBF5D00F4);

	//mtos_task_delay_ms(30);
	msleep(30);
	//read 0xBF5D00F4 [21:16]
	val = readl((volatile u32 *)0xBF5D00F4);
	result = (val >> 16) & 0x3f;

	MT_INFO_PM("Fun[%s] Line[%u]  result = 0x%x \n", __FUNCTION__, __LINE__, result);
	val = readl((volatile u32 *)0xbf314040);
	val |= 0xff << 12;
	val |= 1 << 10;
	val &= ~(1 << 9);
	val |= 1 << 8;
	val &= ~(0x3f << 2);
	val |= result<< 2;
	val |= 1 << 1;
	writel(val, (volatile u32 *)0xbf314040);

	val = readl((volatile u32 *)0xbf153000);
	val |= 1<<12;
	writel(val, (volatile u32 *)0xbf153000);
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	ulong va_addr;

	printk(KERN_ERR "[%s %d]switch to OSC Clock!\n", __FUNCTION__, __LINE__);

	va_addr = SYMPHONY_IO_VA(0xBF153890);
	val = stby_readl((volatile u32 *)va_addr);
	val |= 0x01;//0:xtal, 1:security osc
	stby_writel((volatile u32 *)va_addr, val);
#if 0
	//sym6 this is bit20, move to aomcu config
	va_addr = SYMPHONY_IO_VA(0xBF157004);
	val = stby_readl((volatile u32 *)va_addr);
	//val &= (~(3<<20));//bit21-bit20, 00:ext clk disable
	val |= (0x1 << 20);
	stby_writel((volatile u32 *)va_addr, val);
#endif
#endif
	msleep(100);
}

//extern int serial_init_symphony_osc(u8 id);
extern void nec_time_recfg_osc(void);
extern mt_s32 irda_symphony_reset_wfilt_osc(void);
#if defined CONFIG_MT_KEYLED_ADC
extern MT_U8 kadc_get_type(MT_VOID);
#endif

static long PM_Ioctl(struct file *filp, unsigned int cmd, unsigned long param)
{
	u32 wake_up_type = 0;
#if defined CONFIG_MT_KEYLED_ADC
	u8 kadc_type;
#endif
	cec_config_t cec_cfg ={0};
	sty_aomcu_fw_conf_t aomcu_fw_cfg;

	MT_INFO_PM("PM_Ioctl DO!   cmd = 0x%x\n", cmd);

	switch (cmd)
	{
		case CMD_PM_SET_FPDEV_TYPE:
		{
			fp_type = param;
			if(param == FD650)
			{
				(void)standby_cpu_attach_fw_fd650_original_symphony();
			}
			else if(param == CT1642)
			{
				(void)standby_cpu_attach_fw_ct1642_original_symphony();
			}
			else if(param == AOGPIO)
			{
				(void)standby_cpu_attach_fw_aogpio_original_symphony();
			}
			else if(param == NOFP)
			{
				(void)standby_cpu_attach_fw_no_fp_original_symphony();
			#if defined CONFIG_MT_KEYLED_ADC
				kadc_type = (standby_info.standby_param >> 1) & 0x03;
				if (KADC_KEYS_DISABLE != kadc_type)
				{
					kadc_type = kadc_get_type();
					standby_info.standby_param |= ((kadc_type & 0x03) << 1);
				}
			#endif
			}
			else if(param == OSC)
			{
				(void)standby_cpu_attach_fw_osc_original_symphony();
			}
			else if(param == TT1629)
			{
 				(void)standby_cpu_attach_fw_tt1629_symphony();
			}
			else if(param == TT1629B)
			{
				(void)standby_cpu_attach_fw_tt1629b_symphony();
			}
			else if(param == STRPOWER)
			{
				(void)standby_cpu_attach_fw_str_symphony4();
			}
			else if(param == PT6393)
			{
				(void)standby_cpu_attach_fw_pt6393_original_symphony();
			}
            #if defined CONFIG_MT_KEYLED_ADC
			else if(param == FD650_KADC)
			{
				kadc_type = (standby_info.standby_param >> 1) & 0x03;
				if (KADC_KEYS_DISABLE != kadc_type)
				{
					kadc_type = kadc_get_type();//fd650_keyadc_get_type();
					standby_info.standby_param |= ((kadc_type & 0x03) << 1);
				}
				(void)standby_cpu_attach_fw_fd650_kadc_symphony();
			}
            #endif
			else
			{
				return MT_FAILURE;
			}

			break;
		}

		case CMD_PM_SWITCH_OSC_CLOCK:
		{
			// printk(KERN_ERR "%s[%d]: set osc clock\n", __FUNCTION__, __LINE__);
		#if (defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6))
			MT_INFO_PM("##CMD_PM_SWITCH_OSC_CLOCK##\n");
			standby_info.standby_param |= (1<<3);//set OSC_FLAG
			standby_aomcu_switch_clk_osc();
			//serial_init_symphony_osc(0);
#if 0		/* use linux standard ir now. nec_time_recfg_osc is setting hardware decode, but we don't use hardware decode now */
			nec_time_recfg_osc();
#endif
			//irda_symphony_reset_wfilt_osc();
			irda_symphony_clk_reset(21);


			break;
		#endif
		}

		case CMD_PM_IN_LOW_POWER:
		{
//#ifdef CONFIG_TEE
//			(void)call_standbycpu_symphony();
//#else
			pm_suspend(PM_SUSPEND_STANDBY);
//#endif
			break;
		}

		case CMD_PM_SET_DISP_MODE:
		{
			sty_disp_conf_t dconfig = {0};

			if (copy_from_user((void *)&dconfig, (void *)param, sizeof(struct sty_disp_conf)) != 0)
			{
				return -EIO;
			}

			standby_info.cur_hour = dconfig.disp_time.cur_hour;
			standby_info.cur_min = dconfig.disp_time.cur_min;
			standby_info.cur_sec = dconfig.disp_time.cur_sec;

			if(dconfig.disp_mode & LED_DIS_TIME)
			{
				standby_info.standby_config_info |= (0x1 << 6);
			}
			else if(dconfig.disp_mode & LED_DIS_CHAR)
			{
				standby_info.standby_config_info |= (0x1 << 5);
			}

			break;
		}

		case CMD_PM_SET_WAKEUP_MODE:
		{
			sty_wakeup_conf_t wconfig = {0};
			if (copy_from_user((void *)&wconfig, (void *)param, sizeof(struct sty_wakeup_conf)) != 0)
			{
				return -EIO;
			}

			standby_info.wake_up_key = wconfig.w_key.fp_wkey;

			if(wconfig.w_mode & TIME_WAKE_UP)
			{
				standby_info.wake_day = wconfig.w_time.pass_day;
				standby_info.wake_hour= wconfig.w_time.wakeup_hour;
				standby_info.wake_min= wconfig.w_time.wakeup_min;
				standby_info.wake_sec= wconfig.w_time.wakeup_sec;
				standby_info.standby_config_info |= (0x1 << 7);
			}

			break;
		}

		case CMD_PM_SET_WAKEUP_CONFIG:
		{
			sty_wakeup_display_t wdisp = {0};
			if (copy_from_user((void *)&wdisp, (void *)param, sizeof(struct sty_wakeup_display)) != 0)
			{
				return -EIO;
			}

			if (wdisp.lock_led)
			{
				standby_info.fd650_display_config |= 0x1;
			}

			if (wdisp.fp_reverse)
			{
				standby_info.fd650_display_config |= 0x2;
			}

			if (wdisp.display_config)
				standby_info.fd650_display_config |= (wdisp.display_config & 0x03) << 5;
			else
				standby_info.fd650_display_config &= ~(0x03 << 5);

			break;
		}

		case CMD_PM_SET_PARAM:
		{
			sty_param_conf_t ledconfg;

			memset(&ledconfg, 0, sizeof(struct sty_param_conf));
			if (copy_from_user((void *)&ledconfg, (void *)param, sizeof(struct sty_param_conf)) != 0)
			{
				return -EIO;
			}

			standby_info.bitmap[0] = ((ledconfg.map[1]<<4)|ledconfg.map[0]);
			standby_info.bitmap[1] = ((ledconfg.map[3]<<4)|ledconfg.map[2]);
			standby_info.bitmap[2] = ((ledconfg.map[5]<<4)|ledconfg.map[4]);
			standby_info.bitmap[3] = ((ledconfg.map[7]<<4)|ledconfg.map[6]);

			if(ledconfg.led_bri.en_led_bright)
			{
				standby_info.standby_config_info |= ((ledconfg.led_bri.bri_val & 0x07) << 1);
			}
			if(ledconfg.led_pos.config_led_en)
			{
				standby_info.standby_config_info2 |= (0x1 << 1);
				standby_info.led_pos= (ledconfg.led_pos.led_pos[0] & 0x3) + ((ledconfg.led_pos.led_pos[1] & 0x3) << 2) +  \
				((ledconfg.led_pos.led_pos[2] & 0x3) << 4) + ((ledconfg.led_pos.led_pos[3] & 0x3) << 6);
			}
			else
			{
				standby_info.standby_config_info2 &= ~(0x1 << 1);
			}

			if(ledconfg.led_pos.config_lock_en)
			{
				standby_info.standby_config_info2 |= (0x1 << 2);
				standby_info.standby_config_info2 &= ~(0x3 << 3);
				standby_info.standby_config_info2 |= ((ledconfg.led_pos.lock_pos & 0x3) << 3);
			}
			else
			{
				standby_info.standby_config_info2 &= ~(0x1 << 2);
				standby_info.standby_config_info2 &= ~(0x3 << 3);
			}

			if(ledconfg.led_pos.config_colon_en)
			{
				standby_info.standby_config_info2 |= (0x1 << 5);
				standby_info.standby_config_info2 &= ~(0x3 << 6);
				standby_info.standby_config_info2 |= ((ledconfg.led_pos.colon_pos & 0x3) << 6);
			}
			else
			{
				standby_info.standby_config_info2 &= ~(0x1 << 5);
				standby_info.standby_config_info2 &= ~(0x3 << 6);
			}

			if(ledconfg.led_pos.config_power_en)
			{
				standby_info.fd650_display_config |= (0x1 << 2);
				standby_info.fd650_display_config &= ~(0x3 << 3);
				standby_info.fd650_display_config |= ((ledconfg.led_pos.power_pos & 0x3) << 3);
			}
			else
			{
				standby_info.fd650_display_config &= ~(0x1 << 2);
				standby_info.fd650_display_config &= ~(0x3 << 3);
			}
			break;
		}

		case CMD_PM_GET_STANDBY_TIME:
		{
			u32 tmp = 0;
			standby_time_t p_get_time = {0};

			if (copy_from_user((void *)&p_get_time, (void *)param, sizeof(struct standby_time)) != 0)
			{
				return -EIO;
			}

			//FIXME: for symphony1/2/4, how about Aria?
			//tmp = *((volatile u32 *)(g_pm_drv->ao_dev.base + PM_LPM_AOMCU_MESSAGE));
			if ((NULL == g_pm_drv) || (NULL == g_pm_drv->ao_dev.base))
			{
				MT_ERR_PM("\n###aomcu %s.%d aomcu base Addr is NULL!!!\n",__FUNCTION__,__LINE__);
				return -ENXIO;
			}
			tmp = HAL_GET_U32((volatile u32 *)(g_pm_drv->ao_dev.base + PM_LPM_AOMCU_MESSAGE));
			p_get_time.pass_day = (tmp >> 24) & 0xff;
			p_get_time.cur_hour = (tmp >> 4) & 0x3f;
			p_get_time.cur_min = (tmp >> 10) & 0x3f;
			p_get_time.cur_sec = (tmp >> 16) & 0x3f;
			if (copy_to_user((void *)param, (void *)&p_get_time, sizeof(struct standby_time)) != 0)
			{
				return -EIO;
			}

			//printk("[kernel get time] ][0x%x]:[0x%x]:[0x%x]  day[0x%x]\n", p_get_time.cur_hour, p_get_time.cur_min, p_get_time.cur_sec, p_get_time.pass_day);
			break;
		}
		case CMD_PM_SET_GPEN:
		{
			u8 value = 0;
			value = (u8)param;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
			if(stby_readl((volatile u32 *)SYMPHONY_IO_VA(0xbf31366c)) == 0)	//OTP bf31366c，0--> no repair，other-->repair
				value = 0;		//no repair NORMAL_MEM
			else
				value = 1;		//repair no define NORMAL_MEM
#endif
			standby_info.standby_config_info |= ((u8)value & 0x01);
			break;
		}

		case CMD_PM_GET_STANDBY_INFO:
		{
			wake_up_type = HAL_GET_U32((volatile u32 *)(g_pm_drv->ao_dev.base + PM_LPM_GET_WAKE_UP_FLAG));
			wake_up_type = wake_up_type & 0x7;
			if (copy_to_user((void *)param, (void *)&wake_up_type, sizeof(u32)) != 0)
			{
				return -EIO;
			}
			break;
		}

		case CMD_PM_CEC_CONFIG:
		{
			if (copy_from_user((void *)&cec_cfg, (void *)param, sizeof(cec_config_t)) != 0)
			{
				return -EIO;
			}

			if(cec_cfg.cec_enable)
			{
				standby_info.cec_config = ((cec_cfg.cec_aogpio_pin & 0x07) << 1) | cec_cfg.cec_enable;
			}
			break;
		}

		case CMD_PM_SET_UNIX_TIME:
		{
			u32 set_time = 0;

			if (copy_from_user((void *)&set_time, (void *)param, sizeof(set_time)) != 0)
			{
				return -EIO;
			}

			standby_info.unix_time_sec = set_time;
			standby_info.standby_param |= 0x1 << 0x5;
			MT_INFO_PM(" #### standby_info.unix_time_sec =%d ###\n",standby_info.unix_time_sec);
			break;
		}

		case CMD_PM_GET_UNIX_TIME:
		{
			u32 get_time;

			get_time = stby_readl((volatile u32*)(g_pm_drv->ao_dev.base + PM_LPM_AOMCU_MESSAGE));
			MT_INFO_PM(" #### get_time =0x%x ###\n",get_time);
			if (copy_to_user((void *)param, (void *)&get_time, sizeof(get_time)) != 0)
			{
				return -EIO;
			}
			break;
		}

		case CMD_PM_CLEAR_STANDBY_INFO://clear wake up info register
		{
			wake_up_type = HAL_GET_U32((volatile u32 *)(g_pm_drv->ao_dev.base + PM_LPM_GET_WAKE_UP_FLAG));
			wake_up_type &= (~0x7);
			HAL_PUT_U32((volatile u32 *)(g_pm_drv->ao_dev.base + PM_LPM_GET_WAKE_UP_FLAG), wake_up_type);
			break;
		}

		case CMD_PM_SET_BLE_WAKEUP:
		{
			sty_wakeup_ble_conf_t ble_config = {0};
			if (copy_from_user((void *)&ble_config, (void *)param, sizeof(sty_wakeup_ble_conf_t)) != 0)
			{
				return -EIO;
			}

			montage_enter_standby();

			if (ble_config.ble_enable)
			{
				standby_info.standby_config_info3 |= (0x1 << 7);
				standby_info.standby_config_info3 |= (ble_config.aogpio & 0x0f);
				standby_info.standby_config_info3 &= ~(0x1 << 4);
				standby_info.standby_config_info3 |= ((ble_config.level & 0x1) << 4);
			}
			break;

		}

#if (defined CONFIG_MT_KEYLED_ADC)
		case CMD_PM_KADC_CONFIG:
		{
				standby_kadc_config_t kadc_cfg;
				u8 kadc_keys = KADC_KEYS_DISABLE;
				memset(&kadc_cfg,0,sizeof(kadc_cfg));
				if (copy_from_user(&kadc_cfg, (void *)param, sizeof(standby_kadc_config_t)) != 0)
				{
					return -EIO;
				}

				if (kadc_cfg.enable_cfg)
				{
					if(kadc_cfg.disable_kadc)
					{
						kadc_keys = KADC_KEYS_DISABLE;
					}
					else
					{
						kadc_keys = kadc_cfg.kadc_keys;
					}

					standby_info.standby_param |= ((kadc_keys & 0x03) << 1);
					standby_info.standby_param |= ((kadc_cfg.hw_version & 0x1) << 6);

				}

				break;
		}
#endif
		case CMD_PM_AO_FW_CONFIG:
		{
			memset(&aomcu_fw_cfg,0,sizeof(sty_aomcu_fw_conf_t));
			if (copy_from_user(&aomcu_fw_cfg, (void *)param, sizeof(sty_aomcu_fw_conf_t)) != 0)
			{
				MT_ERR_PM("\n###aomcu %s.%d FW config ERR.\n",__FUNCTION__,__LINE__);
				return -EIO;
			}

			if (aomcu_fw_cfg.fw_extern && aomcu_fw_cfg.fw_size && aomcu_fw_cfg.fw_info)
			{
				if (aofw_info.p_fw)
				{
					kfree(aofw_info.p_fw);
					aofw_info.p_fw = NULL;
				}

				aofw_info.fw_size = aomcu_fw_cfg.fw_size;
				aofw_info.p_fw = kmalloc(aofw_info.fw_size, GFP_KERNEL);
				if (!aofw_info.p_fw)
				{
					aofw_info.fw_size = 0;
					aofw_info.extern_fw = 0;
					MT_ERR_PM("\n###aomcu %s.%d FW config ERR.\n",__FUNCTION__,__LINE__);
					return -EAGAIN;
				}
				else
				{

					if (copy_from_user(aofw_info.p_fw, (void *)aomcu_fw_cfg.fw_info, aofw_info.fw_size) != 0)
					{
						MT_ERR_PM("\n###aomcu %s.%d FW config ERR.\n",__FUNCTION__,__LINE__);
						return -EIO;
					}

					aofw_info.extern_fw = 1;
					printk(KERN_ERR "\n###aomcu %s.%d fw_size:0x%x\n",__FUNCTION__,__LINE__,aofw_info.fw_size);
				}
			}
			else
			{
				MT_ERR_PM("\n###aomcu %s.%d FW config ERR.\n",__FUNCTION__,__LINE__);
				return -EINVAL;
			}
			break;
		}

		default	:
			break;
	}
	return MT_SUCCESS;
}

mt_s32 PM_Release(struct inode *inode, struct file *filp)
{
	MT_INFO_PM("PM_Release DO!");
	return MT_SUCCESS;
}
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
extern void crm_avcpu_suspend(void);
#endif


#if 0
static baseops_s pm_baseOps = {
	.probe		= standby_pm_probe,
	.remove		= NULL,
	.shutdown	= NULL,
	.prepare	= NULL,
	.complete	= NULL,
	.suspend	= standby_pm_suspend,
	.suspend_late	= NULL,
	.resume_early	= NULL,
	.resume		= standby_pm_resume
};
#endif

static struct file_operations PM_FOPS =
{
	.owner = THIS_MODULE,
	open :PM_Open,
	release:PM_Release,
	unlocked_ioctl:PM_Ioctl,
};
static mt_s32 standby_proc_read(struct seq_file *p, mt_void *v)
{
	p += PROC_PRINT(p,"+++++++this is standby proc+++++++\n");

	p += PROC_PRINT(p, "standby_info wake_up_key =%0x \n",standby_info.wake_up_key);

	switch(fp_type)
	{
		case FD650:
			p += PROC_PRINT(p, "standby_info fp_type is FD650\n");
			break;
		case CT1642:
			p += PROC_PRINT(p, "standby_info fp_type is CT1642\n");
			break;
		case AOGPIO:
			p += PROC_PRINT(p, "standby_info fp_type is AOGPIO\n");
			break;
		case NOFP:
			p += PROC_PRINT(p, "standby_info fp_type is NOFP\n");
			break;
		case OSC:
			p += PROC_PRINT(p, "standby_info fp_type is OSC\n");
			break;
		case TT1629:
			p += PROC_PRINT(p, "standby_info fp_type is TT1629\n");
			break;
		case TT1629B:
			p += PROC_PRINT(p, "standby_info fp_type is TT1629B\n");
			break;
		default:
			p += PROC_PRINT(p, "standby_info fp_type is unsupport!!!\n");
			break;
	}

	return MT_SUCCESS;
}

mt_s32 standby_proc_write(struct file * file, const char __user * buf, size_t count, loff_t *ppos)
{
	mt_char ProcPara[PROC_PARAM_MAXLEN] = {0};
	//mt_u32 para_val[PROC_PARAM_MAXLEN] = {0};
	mt_char *param = NULL;

	if(count > PROC_PARAM_MAXLEN)
	{
		printk(KERN_ERR "write data is too long!\n");
		return -EFAULT;
	}

	memset(ProcPara,0,PROC_PARAM_MAXLEN);
	if(copy_from_user(ProcPara, buf, count))
	{
		printk(KERN_ALERT "write data is too long!\n");
		return -EFAULT;
	}
	ProcPara[PROC_PARAM_MAXLEN-1] = 0;
	param =ProcPara;

#ifdef SYM6_GPIO_CHIP_TEST

	//ProcPara[0] == 'r'
	printk(KERN_ALERT "\n###[%s.%d] cmd:%s\n",__FUNCTION__,__LINE__,ProcPara);

	if(('a' == ProcPara[0]) && ('0' == ProcPara[1]))
	{
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150038)), 0x0);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)), 0x0);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150044)), 0x0);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150024)), 0x0);
	}

	if(('a' == ProcPara[0]) && ('2' == ProcPara[1]))
	{
		//stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150038)), 0x0);
		//stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150024)), 0x0);
		sym6_chip_test_3626();
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150024)), 0x2);
	}

	if(('a' == ProcPara[0]) && ('3' == ProcPara[1]))
	{
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150024)), 0x3);
		sym6_chip_test_mailbox();
	}

	if(('a' == ProcPara[0]) && ('4' == ProcPara[1]))
	{
		sym6_chip_test_timer();
	}

	if(('d' == ProcPara[0]) && ('o' == ProcPara[1]) && ('z' == ProcPara[2]) && ('e' == ProcPara[3]))
	{
		sym6_chip_test_3632();
	}

	if(('c' == ProcPara[0]) && ('1' == ProcPara[1]))
	{
		printk(KERN_ERR "###clk %s.%d",__FUNCTION__,__LINE__);
		mt_apcpu_prepare_suspend(576000000);
		printk(KERN_ERR "###clk %s.%d",__FUNCTION__,__LINE__);
	}
#endif
	return count;
}

#ifdef SYM6_GPIO_CHIP_TEST

static struct task_struct *ao_thread = NULL;
static struct task_struct *doze_thread = NULL;
static struct task_struct *doze_timer_thread = NULL;


static irqreturn_t sym_ao2ap_mb_isr(int irq, void *dev_id)
{
	mt_u32 val = 0;
	mt_u32 val1 = 0;
	mt_u32 data = 0;
	val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf15e000)));
	val1 = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf15e008)));
	printk(KERN_ALERT "\n###[IRQ_MBOX_AO2AP] %s.%d 0xbf15e000=0x%x 0xbf15e008=0x%x\n", __FUNCTION__,__LINE__,val,val1);
	data = 0xffffffff;
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15e004)), data);
	printk(KERN_ALERT "\n###[IRQ_MBOX_AO2AP] %s.%d irq:%d clr.\n", __FUNCTION__,__LINE__,irq);
    return IRQ_HANDLED;
}

static irqreturn_t sym_aotimer0_isr(int irq, void *dev_id)
{
	mt_u32 val = 0;
	mt_u32 data = 0;

	printk(KERN_ALERT "\naotimer0 end(684s).\n");
	val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)));
	if(0x7 == (val & 0x7))
	{
		printk(KERN_ALERT "\n###[aomcu] %s.%d aotimer0 happeded.\n", __FUNCTION__,__LINE__);
		val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c088)));
		data = val | (0x1 << 3);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c088)), data);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)), 0x0);
		printk(KERN_ALERT "\n###[apcpu] %s.%d clear aotimer0 intterrupt.\n", __FUNCTION__,__LINE__);
	}
	printk(KERN_ALERT "\n###[apcpu] %s.%d aotimer0 happened irq:%d.\n", __FUNCTION__,__LINE__,irq);

    return IRQ_HANDLED;
}

static irqreturn_t sym_aotimer1_isr(int irq, void *dev_id)
{
	mt_u32 val = 0;
	mt_u32 data = 0;
	printk(KERN_ALERT "\naotimer1 end(21s).\n");
	val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)));
	if(0x8 == (val & 0x8))
	{
		printk(KERN_ALERT "\n###[aomcu] %s.%d aotimer1 happeded.\n", __FUNCTION__,__LINE__);
		val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c188)));
		data = val | (0x1 << 3);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c188)), data);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)), 0x0);
		printk(KERN_ALERT "\n###[apcpu] %s.%d clear aotimer1 intterrupt.\n", __FUNCTION__,__LINE__);
	}

	printk(KERN_ALERT "\n###[apcpu] %s.%d aotimer1 happened irq:%d.\n", __FUNCTION__,__LINE__,irq);

    return IRQ_HANDLED;
}

static int g_mb_ao_cnt = 0;
int sym6_chip_test_task(void *data) {
	mt_u32 reg_val = 0;
    while (!kthread_should_stop()) {
        // 执行你的内核线程代码
        reg_val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf150038)));
		if (g_mb_ao_cnt != reg_val)
		{
			g_mb_ao_cnt = reg_val;
			printk(KERN_ERR "\n###[mailbox] aomcu get intterrupt count is:%d \n",g_mb_ao_cnt);
		}

		reg_val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)));
		if (0x7 == reg_val)
		{
			printk(KERN_ERR "\n###[aotimer0] aomcu timer0 happened intterrupt!!! 0xbf150040=0x%x \n",reg_val);
		}

		reg_val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)));
		if (0x8 == reg_val)
		{
			printk(KERN_ERR "\n###[aotimer1] aomcu timer1 happened intterrupt!!! 0xbf150040=0x%x \n",reg_val);
		}

		ssleep(1);
    }
    return 0;
}


static void sym6_chip_test_3626(void)
{
	if (NULL == ao_thread)
	{
		ao_thread = kthread_run(sym6_chip_test_task, NULL, "sym6_chip_test_thread");
		if (ao_thread) {
			printk(KERN_INFO "Kernel thread created successfully\n");
		} else {
			printk(KERN_ERR "Failed to create kernel thread\n");
		}
	}
}

static int g_mb_irq = 0;
static void sym6_chip_test_mailbox(void)
{

	int ret = 0;
	//stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150038)), 0x0);
	if (0 == g_mb_irq)
	{
		ret = request_irq(IRQ_MBOX_AO2AP_ID, sym_ao2ap_mb_isr, IRQF_TRIGGER_HIGH, "mt_ap_mb", NULL);
		if (ret < 0){
	        printk(KERN_ALERT "\nrequest irq ret = %d failed.\n", ret);
			return;
	    }

		printk(KERN_ALERT "\nrequest irq:%d success.\n", IRQ_MBOX_AO2AP_ID);
		g_mb_irq = 1;
	}
	//mb_aomcu_enaset
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15e010)), 0x2);
	//mb_aomcu_intset
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15e00c)), 0x2);

	printk(KERN_ALERT "\n###[mailbox] AP config,aomcu will get intterrupt!!!\n");
	printk(KERN_ALERT "\n###[mailbox] aomcu get intterrupt count saved in 0xbf150038\n");
}

static int g_aotimer_irq = 0;
static void sym6_chip_test_timer(void)
{
	//IRQ_AO_TM0_ID
	int ret = 0;

	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf150040)), 0x0);
	if (0 == g_aotimer_irq)
	{
		ret = request_irq(IRQ_AO_TM0_ID, sym_aotimer0_isr, IRQF_TRIGGER_HIGH, "mt_aotimer_0", NULL);
		if (ret < 0){
	        printk(KERN_ALERT "\nrequest aotimer0 irq ret = %d failed.\n", ret);
			return;
	    }

		printk(KERN_ALERT "\nrequest aotimer0 irq:%d success.\n", IRQ_AO_TM0_ID);
		ret = request_irq(IRQ_AO_TM1_ID, sym_aotimer1_isr, IRQF_TRIGGER_HIGH, "mt_aotimer_1", NULL);
		if (ret < 0){
	        printk(KERN_ALERT "\nrequest aotimer1 irq ret = %d failed.\n", ret);
			return;
	    }

		g_aotimer_irq = 1;
		printk(KERN_ALERT "\nrequest aotimer1 irq:%d success.\n", IRQ_AO_TM1_ID);
	}
	//(void *)SYMPHONY_IO_VA(0xbf15c080);

	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c080)), 0x28C50300);
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c088)), 0x3);
	printk(KERN_ALERT "\naotimer0 start(684s).\n");

	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c180)), 0x1406F40);
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf15c188)), 0x3);
	printk(KERN_ALERT "\naotimer1 start(21s).\n");

/*
###t0
TI0_INIT_AO:0xbf15c080,684s,0x28C50300
TI0_CW_AO:0xbf15c088,0x3
TI0_MODE_AO:0xbf15c090
###t1
TI1_INIT_AO:0xbf15c180,21s,0x1406F40
TI1_CW_AO:0xbf15c188
TI1_MODE_AO:0xbf15c190

*/

}

static void sym6_doze_config(u8 enter_doze)
{
	if(enter_doze)
	{
		//config OMC:DDR precharge for all bank
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf530010)), 0x38);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf530014)), 0x1);
		msleep(1);
		//config OMC:DDR enter self-refresh
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf530010)), 0x39);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf530014)), 0x1);
#if 0
		//close TOPCLK
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f800)), 0x34);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f804)), 0x0);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f808)), 0x0);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f80c)), 0x0);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f810)), 0x4000000);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f814)), 0x0);

		//config apb and ahb xtal
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf508004)), 0xe8);
		//config apcpu xtal
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf508104)), 0x2b);
//#if 0
		//analog
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf157000)), 0xFBFFFFFF);
		msleep(1);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf157004)), 0xFF4400EF);
#endif
	}
	else
	{
		//config OMC:DDR exit self-refresh
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf530010)), 0x3b);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf530014)), 0x1);
#if 0
		//config TOPCLK
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f800)), 0xFFFFFFFF);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f804)), 0xFFFFFFFF);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f808)), 0xFFFFFFFF);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f80c)), 0xFFFFFFFF);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f810)), 0xFFFFFFFF);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf50f814)), 0xFFFFFFFF);

		//config apb and ahb xtal
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf508004)), 0x00000100);
		//config apcpu xtal
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf508104)), 0x00000026);

		//analog
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf157000)), 0x00355000);
		msleep(1);
		stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf157004)), 0x004A0000);
#endif
	}
}

static int g_sw_timer_irq = 0;
static int g_doze_wakeup = 0;
static int sym6_chip_doze_task(void *data)
{
    while (!kthread_should_stop()) {
        // 执行你的内核线程代码
		msleep(100);
		printk(KERN_ALERT "\n%s[%d]: DDR enter self_refresh.\n", __FUNCTION__, __LINE__);
		sym6_doze_config(1);
		while(!g_doze_wakeup)
		{
			// cpu in wait state
			__asm__("wfi");
		}

		ssleep(1);
		printk(KERN_ALERT "\n%s[%d]: DDR exit self_refresh.\n", __FUNCTION__, __LINE__);
		sym6_doze_config(0);
		symphony_wdt_reset();
		ssleep(1);
    }
    return 0;
}

static int sym6_chip_doze_timer_task(void *data)
{
	u32 ti_init = 0, ti_cap = 0, ti_cw = 0;
    while (!kthread_should_stop()) {
        // 执行你的内核线程代码
		ssleep(1);
		printk(KERN_ALERT "\n%s[%d]: doze SW task run.\n", __FUNCTION__, __LINE__);
		ti_init = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf100180)));
		ti_cap = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf100184)));
		ti_cw = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf100188)));
		printk(KERN_ALERT "\n%s[%d]: sw timer1: TI_INIT = 0x%08x, TI_CAP = 0x%08x, TI_CW = 0x%08x\n", __FUNCTION__, __LINE__, ti_init, ti_cap, ti_cw);
    }
    return 0;
}


static void sym_sw_timer1_init(void)
{
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf100180)), 0x2DC6C0);//0xF4240->1s
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf100188)), 0x3);
	printk(KERN_ALERT "\nsw timer1 start(3s).\n");
}

static irqreturn_t sym_sw_timer1_isr(int irq, void *dev_id)
{
	mt_u32 val = 0;
	mt_u32 data = 0;

	printk(KERN_ALERT "\nsw timer1 end(3s).\n");
	printk(KERN_ALERT "\n###[apmcu] %s.%d sw timer1 happeded.\n", __FUNCTION__,__LINE__);
	val = stby_readl((volatile u32 *)(SYMPHONY_IO_VA(0xbf100188)));
	data = val | (0x1 << 3);
	stby_writel((volatile u32 *)(SYMPHONY_IO_VA(0xbf100188)), data);
	printk(KERN_ALERT "\n###[apcpu] %s.%d clear sw timer1 intterrupt.\n", __FUNCTION__,__LINE__);
	g_doze_wakeup = 1;

    return IRQ_HANDLED;
}


static void sym6_chip_test_3632(void)
{
	int ret = 0;
	sym_sw_timer1_init();

	if (NULL == doze_thread)
	{
		doze_thread = kthread_run(sym6_chip_doze_task, NULL, "sym6_chip_test_doze_thread");
		if (doze_thread) {
			printk(KERN_INFO "Kernel thread created successfully\n");
		} else {
			printk(KERN_ERR "Failed to create kernel thread\n");
		}
	}

	if (0 == g_sw_timer_irq)
	{
		ret = request_irq(IRQ_T1_ID, sym_sw_timer1_isr, IRQF_TRIGGER_HIGH, "mt_timer_1", NULL);
		if (ret < 0){
	        printk(KERN_ALERT "\nrequest sw timer1 irq ret = %d failed.\n", ret);
			return;
	    }

		g_sw_timer_irq = 1;
		printk(KERN_ALERT "\nrequest sw timer1 irq:%d success.\n", IRQ_T1_ID);
		//sym_sw_timer1_init();
	}

	if (NULL == doze_timer_thread)
	{
		doze_timer_thread = kthread_run(sym6_chip_doze_timer_task, NULL, "sym6_chip_doze_timer_thread");
		if (doze_timer_thread) {
			printk(KERN_INFO "Kernel thread created successfully\n");
		} else {
			printk(KERN_ERR "Failed to create kernel thread\n");
		}
	}
}


#endif


static int symphony_pm_suspend (struct platform_device *pdev, pm_message_t stState)
{
#if 0
	(void)standby_cpu_attach_fw_str_symphony4();
    (void)call_standbycpu_suspend();
#else
	MT_PRINT("PM suspend do nothing\n");
#endif
	return 0;
}

static int symphony_pm_resume(struct platform_device *pdev)
{
	MT_PRINT("PM resume do nothing\n");
	return 0;
}

static int symphony_pm_remove(struct platform_device *pdev)
{
	struct mt_pm_device *pmdev = NULL;

	pmdev = platform_get_drvdata(pdev);
	if (pmdev) {

		cdev_del(&pmdev->cdev);
		kfree(pmdev);
	}

	return 0;
}

static ssize_t symphony_pm_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	// Code to read the attribute value and store it in the buf
	return 0;
}

static ssize_t symphony_pm_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	// Code to write the attribute value from buf
	return 0;
}

static DEVICE_ATTR(pm, 0664, symphony_pm_show, symphony_pm_store);

#ifdef CONFIG_PM
extern void sym4_pm_set_fn(void *fn);

// callback function registered to PM
// do load MCU ROM, and power down some modules before enter Standby and Suspend(STR)
static void symphony_pm_post(suspend_state_t state)
{
	if(standby_fw_size == 0)
	{
		standby_info.wake_up_key = 1;
		standby_info.standby_param |= ((2 & 0x03) << 1);
		standby_cpu_attach_fw_no_fp_original_symphony();
		printk("%s %d standby or str default cfg!\n",__FUNCTION__,__LINE__);
	}

	switch (state)
	{
		case PM_SUSPEND_STANDBY:
			(void)call_standbycpu_symphony();
			break;
		case PM_SUSPEND_MEM:
			(void)standby_cpu_attach_fw_str_symphony4();
			(void)call_standbycpu_suspend();
			break;
		default:
			break;
	}
}
#endif

static int symphony_pm_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct mt_pm_device *pmdev;
	mt_proc_entry_t *pProcItem;
	struct resource *res;

	pmdev = kzalloc(sizeof(struct mt_pm_device), GFP_KERNEL);
	if (NULL == pmdev) {
		ret = -ENOMEM;
		goto fail_pm_kzalloc;
	}

	pmdev->pm_class = class_create(UMAP_DEVNAME_PM);
	if (IS_ERR(pmdev->pm_class)) {
		ret = PTR_ERR(pmdev->pm_class);
		goto fail_pm_class;
	}

	pmdev->minor = UMAP_MIN_MINOR_PM;
	pmdev->minors = UMAP_DEV_NUM_PM;
	pmdev->devt = MKDEV(MT_DEVICE_MAJOR, pmdev->minor);
	pmdev->major = MAJOR(pmdev->devt);

	cdev_init(&pmdev->cdev, &PM_FOPS);
	pmdev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&pmdev->cdev, pmdev->devt, pmdev->minors);
	if (ret) {
		ret = -EINVAL;
		goto fail_pm_cdev;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		pmdev->ao_dev.base = (void *)SYMPHONY_IO_VA(res->start);
		//printk("[%s_%d]start = 0x%lx\n", __func__, __LINE__, (ulong)res->start);
		//printk("[%s_%d]end = 0x%lx\n", __func__, __LINE__, (ulong)res->end);
	}
	else
	{
		pmdev->ao_dev.base = (void *)R_PM_BASE_ADDR;
	}

	//pr_info("\naomcu %s.%d remapped=%p\n",__FUNCTION__,__LINE__,pmdev->ao_dev.base);
	pmdev->dev = device_create(pmdev->pm_class, NULL, pmdev->devt, NULL, UMAP_DEVNAME_PM);
	if (IS_ERR(pmdev->dev)) {
			ret = PTR_ERR(pmdev->dev);
			printk("PM device_create failed. ret = %d\n",ret);
			goto fail_pm_device;
	}

	if (device_create_file(&pdev->dev, &dev_attr_pm)) {
		ret = -ENOENT;
		printk("PM device_create_file failed.\n");
		goto fail_pm_create_file;
	}

	g_pm_drv = pmdev;

	pProcItem = mt_drv_proc_add_module(MT_MOD_PM, NULL, NULL);
	if (!pProcItem)
	{
		printk("PM add proc failed.\n");
		ret = -1;
		g_pm_drv = NULL;
		goto fail_pm_create_file;
	}
//#ifndef CONFIG_TEE
	//sym4_pm_set_fn(call_standbycpu_symphony);
#ifdef CONFIG_PM
	sym4_pm_set_fn(symphony_pm_post);
#endif
//#endif
	pProcItem->read = standby_proc_read;
	pProcItem->write = standby_proc_write;

	platform_set_drvdata(pdev, pmdev);
	dev_set_drvdata(pmdev->dev, pmdev);

	return ret;

fail_pm_create_file:
	device_destroy(pmdev->pm_class, pmdev->devt);
fail_pm_device:
	cdev_del(&pmdev->cdev);
fail_pm_cdev:
	class_destroy(pmdev->pm_class);
fail_pm_class:
	kfree(pmdev);
fail_pm_kzalloc:
	return ret;

}

#if defined(CONFIG_OF)
	 static const struct of_device_id symphony_pm_of_match[] = {
		 { .compatible = "montage,pm" },
		 {},
	 };
MODULE_DEVICE_TABLE(of, symphony_pm_of_match);
#endif


static struct platform_driver symphony_pm_driver = {
	.probe 		= symphony_pm_probe,
	.remove		= symphony_pm_remove,
	.suspend	= symphony_pm_suspend,
	.resume		= symphony_pm_resume,
	.driver	= {
		.name = UMAP_DEVNAME_PM,
		.of_match_table = of_match_ptr(symphony_pm_of_match),
	},
};

mt_s32 __init STANDBY_DRV_ModInit(mt_void)
{
	int  ret = 0;

	ret = platform_driver_register(&symphony_pm_driver);
	if (ret){
		return ret;
	}

	memset(&aofw_info,0,sizeof(aomcu_fw_info_t));
	return 0;
#if 0
	mt_proc_entry_t *pProcItem;

	mt_drv_proc_t stdFnOpt =
	{
		.fnRead = standby_Proc,
	};

	(mt_void)mt_drv_module_register(MT_ID_PM, "MT_PM", MT_NULL);

	mt_osal_snprintf(g_PmRegisterData.devfs_name, sizeof(g_PmRegisterData.devfs_name), UMAP_DEVNAME_PM);
	g_PmRegisterData.minor	= UMAP_MIN_MINOR_PM;
	g_PmRegisterData.owner	= THIS_MODULE;
	g_PmRegisterData.fops	= &PM_FOPS;
	g_PmRegisterData.drvops = &pm_baseOps;

	if (mt_drv_dev_register(&g_PmRegisterData) < 0)
	{
		MT_PRINT("register pm failed.\n");
		return MT_FAILURE;
	}

	memset(&standby_info, 0, sizeof(trans_info_standby_t));
	pProcItem = mt_drv_proc_add_module("standby", &stdFnOpt, NULL);
	if (pProcItem != MT_NULL)
	{
		// pProcItem->write = hdmi_ProcWrite;
	}

	return 0;
#endif
}


mt_void __exit STANDBY_DRV_ModExit(mt_void)
{
#if 0
	mt_drv_proc_rm_module(MT_MOD_PM);
	mt_drv_dev_unregister(&g_PmRegisterData);
	mt_drv_module_unregister(MT_ID_PM);
#endif
	platform_driver_unregister(&symphony_pm_driver);
	return;
}


static LIST_HEAD(mt_standby_list);
static void montage_enter_standby(void)
{
    struct mt_standby_node *node;
    list_for_each_entry(node, &mt_standby_list, list) {
        node->component.enter_standby(node->component.data);
    }
}

int register_mt_standby(struct mt_standby_component *comp)
{

    struct mt_standby_node *node = NULL;
    node = kmalloc(sizeof(struct mt_standby_node), GFP_KERNEL);
    if (!node) {
        return -ENOMEM;
    }

	memcpy(&node->component,comp,sizeof(struct mt_standby_component));

    INIT_LIST_HEAD(&node->list);
    list_add_tail(&node->list, &mt_standby_list);
    return 0;
}
EXPORT_SYMBOL_GPL(register_mt_standby);

void unregister_mt_standby(struct mt_standby_component *comp)
{
    struct mt_standby_node *node, *next;
    list_for_each_entry_safe(node, next, &mt_standby_list, list) {
        if (0 == strcmp(node->component.name, comp->name)) {
            list_del(&node->list);
            kfree(node);
        }
    }
}
EXPORT_SYMBOL_GPL(unregister_mt_standby);


