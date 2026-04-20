/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/syscore_ops.h>
#include <asm/io.h>
#include <linux/string.h>
#include <linux/suspend.h>
#include <linux/printk.h>
#include <linux/delay.h>

#include "mt_common.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"
#include "mt_module_debug.h"

#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "drv_sys_misc.h"
#include "mt_unf_misc.h"
#include "mt_drv_analog.h"
#include "mt_drv_clock.h"
#include "mt_drv_pinctrl.h"
#include "mt_drv_otp.h"

#if !defined(CONFIG_TEE)
#include "avcpu/drv_avcpu.h"
#endif

#include "drv_misc_porting.c"

#define MISC_PROC_PARAM_MAXLEN	(64)

//static unsigned long driver_open = 0;

#define M2S(x) #x

/* HAL modules' name */
static char *module_id[HAL_MODULES_NUM] =
{
	M2S(HAL_CPU0),
	M2S(HAL_AVCPU),
	M2S(HAL_CPU2),
	M2S(HAL_HB),
	M2S(HAL_PB),
	M2S(HAL_GPE),
	M2S(HAL_AUDIO_OUT),
	M2S(HAL_SPDF),
	M2S(HAL_VBI),
	M2S(HAL_SD_VIDEO),
	M2S(HAL_HD_VIDEO),
	M2S(HAL_OCP),
	M2S(HAL_VDEC),
	M2S(HAL_JPEG),
	M2S(HAL_DISPLAY),
	M2S(HAL_DI),
	M2S(HAL_OSDC),
	M2S(HAL_DMA),
	M2S(HAL_TSI),
	M2S(HAL_TSI_CSA30),
	M2S(HAL_DS_DES),
	M2S(HAL_DS_TDES),
	M2S(HAL_DS_AES),
	M2S(HAL_DS_CSA3),
	M2S(HAL_DS_CSA2),
	M2S(HAL_DS_SECHD1),
	M2S(HAL_KL_PVR),
	M2S(HAL_CRYPTO_DES),
	M2S(HAL_CRYPTO_TDES),
	M2S(HAL_CRYPTO_AES),
	M2S(HAL_CRYPTO_SHA),
	M2S(HAL_CRYPTO_RSA),
	M2S(HAL_SECHD0),
	M2S(HAL_KT),
	M2S(HAL_KL_CW),
	M2S(HAL_DS),
	M2S(HAL_KDF),
	M2S(HAL_OTP_PRELOAD),
	M2S(HAL_SECURE),
	M2S(HAL_UART0),
	M2S(HAL_UART1),
	M2S(HAL_SMC0),
	M2S(HAL_SMC1),
	M2S(HAL_SPI0),
	M2S(HAL_SPI1),
	M2S(HAL_SPI2),
	M2S(HAL_SDMMC),
	M2S(HAL_IRDA),
	M2S(HAL_LEDKB),
	M2S(HAL_EPI),
	M2S(HAL_I2C),
	M2S(HAL_I2C0),
	M2S(HAL_I2C1),
	M2S(HAL_I2C_DEBUG),
	M2S(HAL_TIMER),
	M2S(HAL_WATCHDOG),
	M2S(HAL_CRYPTO),
	M2S(HAL_GLITCH_DET),
	/*M2S(HAL_CLK_SPDIF),*/
	M2S(HAL_HDMI),
	M2S(HAL_MAC),
	M2S(HAL_MAC_RMII),
	//M2S(HAL_USB0_PHY),
	M2S(HAL_USB0_P20),
	//M2S(HAL_USB1_20PHY),
	M2S(HAL_USB1_P30),
	M2S(HAL_AO_LPMSET),
	M2S(HAL_AO_PINMUX),
	M2S(HAL_AO_GPIO),
	M2S(HAL_AO_ANA),
	M2S(HAL_AO_RECRAM),
	M2S(HAL_AO_KADC),
	M2S(HAL_AO_RTC),
	M2S(HAL_AO_MAILBOX),
	M2S(HAL_AO_TIMER1),
	M2S(HAL_AO_TIMER0),
	M2S(HAL_AO_AOMCU),
	M2S(HAL_AO_IRDA),
	M2S(HAL_AO_LEDKB),
	M2S(HAL_AO_FPI2C),
	M2S(HAL_AO_FPSPI),
	M2S(HAL_AO_FPSPIREG),
	M2S(HAL_AO_MNT),
	M2S(HAL_AO_CEC),
	M2S(HAL_AO_AVS),
	M2S(HAL_AO_AGTIMER),
	M2S(HAL_AO_WDOG),
	M2S(HAL_AO_MCU),
	M2S(HAL_DEMO),
	M2S(HAL_DEMO_C),
	M2S(HAL_DEMO_S),
	M2S(HAL_CADC),
	M2S(HAL_SADC),
	M2S(HAL_ADAC),
	M2S(HAL_VDAC0),
	M2S(HAL_VDAC1),
	M2S(HAL_VDAC2),
	M2S(HAL_VDAC3),
	M2S(HAL_RNG),
	M2S(HAL_RNG2),
	M2S(HAL_TSENSOR),
	M2S(HAL_PM),
	M2S(HAL_ADCPLL),			/* M2S(HAL_PLL_CPU) */
	M2S(HAL_ARCH_TIMER),
	M2S(HAL_SPDMA),
	M2S(HAL_INTF),
	M2S(HAL_DDRMC),
	M2S(HAL_MNT),
	M2S(HAL_SDIO0),
	M2S(HAL_SDIO1),
	M2S(HAL_PNAND),
	M2S(HAL_LCD),
	M2S(HAL_PNG),
	M2S(HAL_DAI),
	M2S(HAL_CI),
	M2S(HAL_CITSIN),
	M2S(HAL_XTAL),
	M2S(HAL_M2M),
	M2S(HAL_PKA),
	M2S(HAL_IFCP_GLB),
	M2S(HAL_IFCP_KLM),
	M2S(HAL_IFCP_CRYPTO),
	M2S(HAL_IFCP_SYS),
	M2S(HAL_TIMER0),
	M2S(HAL_TIMER1),
	M2S(HAL_TIMER2),
	M2S(HAL_TIMER3),
	M2S(HAL_WATCHDOG0),
	M2S(HAL_WATCHDOG1),
	M2S(HAL_TS0),
	M2S(HAL_TS1),
	M2S(HAL_TS2),
	M2S(HAL_TS3),
	M2S(HAL_DEMUX),
	M2S(HAL_T2MI),
	M2S(HAL_TSI_AVSYNC),
	M2S(HAL_TSI_SF),
	M2S(HAL_TSI_TRPP),
	M2S(HAL_TSI_SWTSI),
	M2S(HAL_TSI_TSPOOL),
	M2S(HAL_SMC),
	M2S(HAL_AXI),
	M2S(HAL_AXI_DEBUG),
	M2S(HAL_AXI_REG),
	M2S(HAL_LCDC),
	M2S(HAL_LCDHD),
	M2S(HAL_LCD2X),
	M2S(HAL_XTAL_MODE),
	M2S(HAL_AUDIO_MCLK),
	/*M2S(HAL_TEMPSENSOR),*/
	M2S(HAL_PANTHER2),
	M2S(HAL_DEMO_J83B),
	M2S(HAL_GPU),
	M2S(HAL_GMAC),
	M2S(HAL_AO_UART),

	//M2S(HAL_ADAC_DIG),
	//M2S(HAL_USB0_DIG),
	//M2S(HAL_USB1_DIG),
	//M2S(HAL_HDMI_DIG),

	//M2S(HAL_EPHY),
	M2S(HAL_EPHYPLL),
	//M2S(HAL_AUDIOPLL),
	M2S(HAL_DEMO_BUS),
	M2S(HAL_USB1_P20),
};

static int reg_set_valid_bit(ulong reg, unsigned int sbit, unsigned int size, u32 val)
{
    u32 tmp = 0, vbit = 0, cur_val = 0;

    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = HAL_GET_U32((volatile u32 *)reg);
    cur_val = (tmp >> sbit) & vbit;
    if(val == cur_val)
        return 0;
    tmp &= ~(vbit << sbit);
    tmp |= (val << sbit);
    HAL_PUT_U32((volatile u32 *)reg, tmp);

    return 0;
}

static int reg_get_valid_bit(ulong reg, unsigned int sbit, unsigned int size, unsigned int *val)
{
    *val = (unsigned int)HAL_GET_BITS((volatile unsigned long *)reg, (unsigned long)sbit, (unsigned long)size);

    return 0;
}

static unsigned int symphony_getpinmux(unsigned int index, unsigned int offset, unsigned int len)
{
	return mt_pinctrl_get_function((PINMUX_INDEX_E)index);
}

static void symphony_setpinmux(unsigned int index, unsigned int offset, unsigned int len, unsigned int value)
{
	mt_pinctrl_set_function((PINMUX_INDEX_E)index, value);
}

/**
 @deprecate
 */
void symphony_pinmux_preset(void)
{
	/* FIXME */
	printk(KERN_WARNING "%s: function deprecated! pls use DTSI or Uboot CMD to set pinmux!\r\n", __FUNCTION__);
	return;
}
EXPORT_SYMBOL(symphony_pinmux_preset);

static int symphony_module_reset(unsigned int mid)
{
	void *name;

	/* FIXME */
	//printk(KERN_WARNING "%s: function deprecated! pls call mt_clk_reset/mt_analog_reset to reset modules!\r\n", __FUNCTION__);

	FOR_EACH_MISC_GROUP
	{
		if (pGrp->reset)
		{
			name = get_mod_name(mid, pGrp->map_tab);
			if (name)
			{
				return pGrp->reset(name);
			}
		}
	}

	printk(KERN_WARNING "%s: mod id %u not found!\r\n", __FUNCTION__, mid);
	return -ENODEV;
}

static int symphony_module_config(unsigned int mid, unsigned int on)
{
	void *name;

	/* FIXME */
	//printk(KERN_WARNING "%s: function deprecated! pls call mt_clk_enable/mt_clk_disable/mt_analog_enable/mt_analog_disable!\r\n", __FUNCTION__);

	FOR_EACH_MISC_GROUP
	{
		if (pGrp->enable && pGrp->disable)
		{
			name = get_mod_name(mid, pGrp->map_tab);
			if (name)
			{
				if (on)
					return pGrp->enable(name);
				else
					return pGrp->disable(name);
			}
		}
	}

	printk(KERN_WARNING "%s: mod id %u not found!\r\n", __FUNCTION__, mid);
	return -ENODEV;
}

/**
 @deprecate
 */
static int symphony_module_clock_set(unsigned int mid, unsigned long clk)
{
	/* FIXME */
	printk(KERN_WARNING "%s: function deprecated! pls call mt_clk_set_rate to set clock rate!\r\n", __FUNCTION__);
	return 0;
}

static unsigned long symphony_module_clock_get(unsigned int mid)
{
	void *name;
	unsigned long rate = 0;
	int ret;

	/* FIXME */
	//printk(KERN_WARNING "%s: function deprecated! pls call mt_clk_get_rate to get clock rate!\r\n", __FUNCTION__);

	FOR_EACH_MISC_GROUP
	{
		if (pGrp->get_rate)
		{
			name = get_mod_name(mid, pGrp->map_tab);
			if (name)
			{
				ret = pGrp->get_rate(name, &rate);
				if (ret != 0)
				{
					printk(KERN_WARNING "%s: get mod %u rate failed return %d!\r\n", __FUNCTION__, mid, ret);
					return 0;
				}

				return rate;
			}
		}
	}

	printk(KERN_WARNING "%s: mod id %u not found!\r\n", __FUNCTION__, mid);
	return 0;
}

static int symphony_module_is_enabled(unsigned int mid)
{
	void *name;
	u32 status = 0;
	int ret;

	FOR_EACH_MISC_GROUP
	{
		if (pGrp->get_status)
		{
			name = get_mod_name(mid, pGrp->map_tab);
			if (name)
			{
				ret = pGrp->get_status(name, &status);
				if (ret != 0)
				{
					printk(KERN_WARNING "%s: get mod %u status failed return %d!\r\n", __FUNCTION__, mid, ret);
					return ret;
				}

				return (int)status;
			}
		}
	}

	printk(KERN_WARNING "%s: mod id %u not found!\r\n", __FUNCTION__, mid);
	return -ENODEV;
}

static long symphony_temperature_get(void)
{
	int temp_int = 0;
	int temp_dec = 0;

	mt_analog_get_temperature_async(&temp_int, &temp_dec);

	return (temp_int * 1000 + temp_dec * 10);
}

static int misc_open(struct inode *inode, struct file *file)
{
    int ret = 0;
/*
    if (test_and_set_bit(0, &driver_open))
    {
    	return -EBUSY;
    }
*/
    return ret;
}

static int misc_release(struct inode *inode, struct file *file)
{
    //clear_bit(0, &driver_open);
    return 0;
}

static long misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    struct drv_misc_ioctl drv_arg;

    memset(&drv_arg, 0, sizeof(struct drv_misc_ioctl));

    if (argp != 0 && copy_from_user(&drv_arg, argp, sizeof(struct drv_misc_ioctl))) {
		return -EFAULT;
	}

    switch (cmd)
    {
        case MISC_IOCTL_PINMUX_PRESET:
            symphony_pinmux_preset();
            break;

        case MISC_IOCTL_PINMUX_SET:
            symphony_setpinmux((unsigned int)drv_arg.id, drv_arg.offset, drv_arg.len, (unsigned int)drv_arg.val);
            break;

        case MISC_IOCTL_PINMUX_GET:
            drv_arg.val = (unsigned long)symphony_getpinmux((unsigned int)drv_arg.id, drv_arg.offset, drv_arg.len);
            if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            break;

        case MISC_IOCTL_MODULE_ONOFF:
            symphony_module_config((unsigned int)drv_arg.id, (drv_arg.val > 0)?1:0);
            break;

		case MISC_IOCTL_MODULE_RESET:
            symphony_module_reset((unsigned int)drv_arg.id);
            break;

		case MISC_IOCTL_MODULE_CLK_GET:
            drv_arg.val = symphony_module_clock_get((unsigned int)drv_arg.id);
            if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            break;

		case MISC_IOCTL_MODULE_CLK_SET:
            symphony_module_clock_set((unsigned int)drv_arg.id, drv_arg.val);
            break;

		case MISC_IOCTL_MODULE_CLK_IS_ENABLED:
		{
			int status = 0;
			status = symphony_module_is_enabled((unsigned int)drv_arg.id);
            drv_arg.val = (unsigned long)status>0?1:0;
            if (copy_to_user(argp, &drv_arg, sizeof(struct drv_misc_ioctl))) {
				return -EFAULT;
			}
            break;
		}
        case MISC_IOCTL_REG_SET:
            reg_set_valid_bit(SYMPHONY_IO_VA(drv_arg.id), drv_arg.offset, drv_arg.len, (u32)drv_arg.val);
            break;

        case MISC_IOCTL_REG_GET:
            reg_get_valid_bit(SYMPHONY_IO_VA(drv_arg.id), drv_arg.offset, drv_arg.len, (unsigned int*)&drv_arg.val);
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
				/* FIXME */
				printk(KERN_WARNING "%s: IO MISC_IOCTL_CHIP_PRODUCTINFO_GET deprecated!\r\n", __FUNCTION__);
				return -ENOIOCTLCMD;
			}
			break;

        default:
            return -ENOIOCTLCMD;
			break;
    }

    return 0;
}

static char *mod2str(unsigned int i)
{
	return module_id[i];
}

static void misc_dump_state(struct seq_file *p)
{
	int i;
	unsigned int idx = 0;
	int ret = 0;
	unsigned long rate = 0;

	/* key modules */
	unsigned int key_tab[] = {
		HAL_DDRMC,
		//HAL_AXI,

		HAL_AVCPU,

		//HAL_AUDIO_OUT,
		HAL_ADAC,

		HAL_VDAC0,
		HAL_SD_VIDEO,

		HAL_TSI,

		HAL_MAC,
		//HAL_EPHY,
		HAL_EPHYPLL,
		HAL_GMAC,

		//HAL_USB0_PHY,
		//HAL_USB1_20PHY,
		//HAL_USB1_30PHY,
		HAL_USB0,
		HAL_USB1,
		HAL_USB1_P20,

		HAL_CADC,
		HAL_SADC,
		HAL_ADCPLL,
		HAL_DEMO_C,
		HAL_DEMO_S,
		HAL_DEMO_J83B,

		HAL_SDIO0,
		HAL_SDIO1,
		HAL_PNAND,

		HAL_DAI,
		//HAL_AUDIOPLL,

		HAL_SMC,
		HAL_GPU,

	};
	/* normal modules */
	unsigned int normal_tab[] = {
		//HAL_HB,
		//HAL_PB,
		HAL_DMA,

		HAL_SPI0,

		HAL_GPE,
		HAL_JPEG,
		HAL_PNG,

		HAL_DISPLAY,
		HAL_HD_VIDEO,
		HAL_VBI,
		HAL_HDMI,

		HAL_VDEC,

		HAL_DS,
		HAL_M2M,
		HAL_PKA,
		HAL_SECHD0,
		HAL_KL_CW,
		HAL_KDF,
		HAL_SECURE,
		HAL_RNG,
		HAL_RNG2,
		HAL_IFCP_KLM,
		HAL_IFCP_CRYPTO,
		//HAL_IFCP_SYS,

		HAL_AO_RECRAM,
		HAL_AO_MAILBOX,
		HAL_AO_TIMER0,
		HAL_AO_AGTIMER,
		HAL_AO_MCU,

		HAL_DEMO_BUS,
		HAL_TSENSOR,
	};

	seq_printf(p, "*(Main Power Consuming Modules)\n");
	seq_printf(p, "[IDX] %16s   %s  %s\n", "Module", "ON/OFF", "Frequency(MHz)");

	/* dump key modules */
	for (i=0; i<sizeof(key_tab)/sizeof(key_tab[0]); i++)
	{
		idx = key_tab[i];
		ret = symphony_module_is_enabled(idx);
		rate = symphony_module_clock_get(idx);

		if(ret < 0 && rate == 0)
		   continue;

		if (rate != 0
			&& (idx == HAL_DDRMC || idx == HAL_CPU1))
		{
			seq_printf(p, "[%03u] *%15s : %3s  \tClock: %lu\n", idx, mod2str(idx), ret>0?"ON":(ret==0)?"OFF":"N/A", rate/1000000);
		}
		else
		{
			seq_printf(p, "[%03u] *%15s : %3s\n", idx, mod2str(idx), ret>0?"ON":(ret==0)?"OFF":"N/A");
		}
	}

	/* dump normal modules */
	for (i=0; i<sizeof(normal_tab)/sizeof(normal_tab[0]); i++)
	{
		idx = normal_tab[i];
		ret = symphony_module_is_enabled(idx);
		//rate = symphony_module_clock_get(idx);

		//if(ret < 0 && rate == 0)
		if(ret < 0)
		   continue;

#if 0
		if (rate != 0)
		{
			seq_printf(p, "[%03u] %16s : %3s  \tClock: %lu\n", idx, mod2str(idx), ret>0?"ON":(ret==0)?"OFF":"N/A", rate/1000000);
		}
		else
#endif
		{
			seq_printf(p, "[%03u] %16s : %3s\n", idx, mod2str(idx), ret>0?"ON":(ret==0)?"OFF":"N/A");
		}
	}

	/* dump clock autogate */
	mt_clk_dump_autogate(p);
}

static mt_s32 misc_proc(struct seq_file *p, mt_void *v)
{
    unsigned long chip_rev = 0;
    package_id_symphony_t package = 0;
    long temp = 0;

    seq_printf(p, "\n--------------------Sysinfo Dump--------------------\n");
    chip_rev = symphony_get_chip_rev();
    package = chip_package_get();
    seq_printf(p, "cver:0x%lx pkg:0x%x\n", chip_rev, package);

	temp = symphony_temperature_get();
    seq_printf(p, "temperature: %ld.%03ld\n", temp/1000, temp>=0?temp%1000:-temp%1000);

    misc_dump_state(p);

    return 0;
}

static mt_void misc_prochelp(mt_void)
{
	mt_drv_proc_echohelp("cat /proc/mt/msp/misc                 -- display all base misc information\n");
	mt_drv_proc_echohelp("echo chipinfo > /proc/mt/msp/misc     -- show chip_production\n");
	mt_drv_proc_echohelp("echo temperature > /proc/mt/msp/misc  -- show chip temperature\n");
}

static int misc_procwrite(struct file * file,
                     const char __user * buf, size_t count, loff_t *ppos)
{
    //struct seq_file *s = file->private_data;

	mt_char ProcPara[MISC_PROC_PARAM_MAXLEN] = {0};

	mt_chip_silkscreen_t silkscreen;
	long val = 0;

	if(count > MISC_PROC_PARAM_MAXLEN)
	{
		printk(KERN_ERR "write data is too long!\n");
		return -EFAULT;
	}

	if(copy_from_user(ProcPara, buf, count))
	{
		printk(KERN_ERR "copy_from_user error!\n");
		return -EFAULT;
	}

	ProcPara[MISC_PROC_PARAM_MAXLEN-1] = 0;
	if(strstr(ProcPara,"chipinfo") && ProcPara[0] == 'c')
	{
		if(0 == mt_otp_get_chip_silkscreen(&silkscreen))
		{
			//seq_printf(s, "Chip Silk Screen: %s\n", silkscreen.string);
			printk(KERN_WARNING "Chip Silk Screen: %s\n", silkscreen.string);
		}
		else
		{
			printk(KERN_ERR "mt_otp_get_chip_silkscreen error!\n");
			return -EFAULT;
		}
	}
	else if(strstr(ProcPara,"temperature") && ProcPara[0] == 't')
	{
		val = symphony_temperature_get();

		//seq_printf(s, "symphony_temperature_get val = %ld\n", val);
		printk(KERN_WARNING "symphony_temperature_get val = %ld\n", val);
	}
	else if(strstr(ProcPara,"help") && ProcPara[0] == 'h')
	{
		misc_prochelp();
	}

	return count;
}

/*
 *  Kernel Interfaces
 */

static struct file_operations misc_fops =
{
    .owner   		= THIS_MODULE,
    .llseek  		= no_llseek,
    //  .write      = misc_write,
    .unlocked_ioctl = misc_ioctl,
    .open    		= misc_open,
    .release 		= misc_release,
};

static mt_device_s g_misc_register_data;

#if defined(CONFIG_PM_SLEEP)
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)

static int mt_misc_pm_notify(struct notifier_block *nb,
							 unsigned long action, void *data)
{
	MT_INFO_MISC("call mt_misc_pm_notify(%lu)\n", action);

	if (PM_SUSPEND_PREPARE == action)
	{
		/* backup analog & clock */
		mt_analog_suspend();
		mt_clk_suspend();
	}
	else if (PM_POST_SUSPEND == action)
	{
		board_adac_cfg();	/* last one, avoid pop noise */

#ifdef CONFIG_TEE
		//TODO
#else
		//do resume avcpu
		misc_avcpu_resume();
#endif
	}

	return 0;
}

static int misc_pm_suspend(void)
{
    MT_INFO_MISC("misc_pm_suspend\n");
    return 0;
}

static void misc_pm_resume(void)
{
    MT_INFO_MISC("misc_pm_resume\n");

	/* restore analog & clock */
	mt_analog_resume();
	board_analog_cfg();

    mt_clk_resume();
}

static struct syscore_ops misc_pm_syscore_ops = {
    .suspend  = misc_pm_suspend,
    .resume   = misc_pm_resume,
};

#endif
#endif

mt_s32 __init misc_drv_modinit(mt_void)
{
    int ret = 0;
    mt_proc_entry_t *item = NULL;
    mt_drv_proc_t misc_proc_ops;

    MT_INFO_MISC("misc init  ... \n");

    (mt_void)mt_drv_module_register(MT_ID_MISC, "misc", NULL);

    /* Check that the default_margin value is within it's range ; if not reset to the default */
    snprintf(g_misc_register_data.devfs_name, sizeof(g_misc_register_data.devfs_name), UMAP_DEVNAME_MISC);
    g_misc_register_data.minor  = UMAP_MIN_MINOR_MISC;
    g_misc_register_data.owner  = THIS_MODULE;
    g_misc_register_data.fops   = &misc_fops;
    g_misc_register_data.drvops = NULL;
    if (mt_drv_dev_register(&g_misc_register_data) < 0)
    {
        MT_ERR_MISC(" misc mt_drv_dev_register err (err=%d)\n", ret);
        return MT_FAILURE;
    }

    memset(&misc_proc_ops, 0, sizeof(mt_drv_proc_t));
    misc_proc_ops.fnRead = misc_proc;
	misc_proc_ops.fnWrite = misc_procwrite;
    item = mt_drv_proc_add_module(MT_MOD_MISC, &misc_proc_ops, NULL);
    if(!item)
    {
        return -1;
    }

#if defined(CONFIG_PM_SLEEP)
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    pm_notifier(mt_misc_pm_notify, 0);

    register_syscore_ops(&misc_pm_syscore_ops);

#endif
#endif

    return ret;
}

mt_void __exit misc_drv_modexit(mt_void)
{
    MT_INFO_MISC("misc exit  ... \n");
#if 0
    mt_drv_module_unregister(MT_ID_MISC);
    mt_drv_dev_unregister(&g_misc_register_data);
    //misc_set_timeout(0, 0);
#endif
}

