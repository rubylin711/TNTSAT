/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */
#ifdef __UBOOT__
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#include "mt_crm_clock_reg.h"
#include "mt_crm_lock_reg.h"
#elif defined(__KERNEL__)
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "mt_drv_analog.h"
#include "mt_drv_clock.h"

/* FIXME */
#include "../crm/mt_crm_clock_reg.h"
#include "../crm/mt_crm_lock_reg.h"
#else
/* RTOS */
#include <sys_types.h>
#include <sys_define.h>

#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "mt_drv_analog.h"
#include "mt_drv_clock.h"

#include "../crm/mt_crm_clock_reg.h"
#include "../crm/mt_crm_lock_reg.h"
#endif

#include "mt_analog_reg.h"
#include "mt_analog_dev.h"

/*
 * <CNComment>SSc: (Spread Spectrum)
 */

#if !defined(CONFIG_TARGET_SYMPHONY6_LITE) && !defined(CONFIG_TARGET_SYMPHONY6_MINI)

#define HDMI_TX_CH0				"hdmi_tx_ch0"
#define HDMI_TX_CH1				"hdmi_tx_ch1"
#define HDMI_TX_CH2				"hdmi_tx_ch2"
#define HDMI_TX_CLK_CH			"hdmi_tx_clk_ch"

#define ADAC_BUF_RCH			"adac_buf_rch"
#define ADAC_RCH				"adac_rch"
#define ADAC_1BIT_INT			"adac_1bit_int"		/* A1+ */
#define ADAC_VDDB1P2			"adac_vddb1p2"
#define ADAC_BUF_LCH			"adac_buf_lch"
#define ADAC_LCH				"adac_lch"
#define ADAC_VDD2P7				"adac_vdd2p7"		/* A1+ */

#define ADAC_OUT				"adac_out"

#define ADAC_MUTE				"adac_mute"

/* A1+ */
#define ADAC_EXT_1BIT_PCM		"adac_1bit_pcm_ext"		/* pu 1bit pcm to external */
#define ADAC_INT_1BIT_PCM		"adac_1bit_pcm_int"		/* pu 1bit pcm to internal buffer */
#define ADAC_OE					"adac_oe"				/* adac digital pad on/off */


#define AVS_CPU					"avs_cpu"
#define AVS_SYS					"avs_sys"

#define USB_IMP_CAL				"usb_imp_cal"
#define HDMI_DCC				"hdmi_dcc"
#define USB30_PLL_DCC			"usb30_pll_dcc"
#define DDR_PLL_SSC				"ddr_pll_ssc"

/* analog modules */
struct mt_ana_dev mt_ana_table[] =
{
	/* BIAS */
	{
		.name	    = MT_ANA_BIAS,
		.power_down = 0,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 0, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_BIT0_BIT24_LOCK_SHIFT)
	},
	/* USB0 */
	{
		.name	    = MT_ANA_USB0,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 1, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* USB1 */
	{
		.name	    = MT_ANA_USB1,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 2, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ICX */
	{
		.name	    = MT_ANA_ICX,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 3, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* VDAC */
	{
		.name	    = MT_ANA_VDAC,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 6, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
#if 0
	/* HDMI_TX_CH0 */
	{
		.name	    = HDMI_TX_CH0,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 8, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* HDMI_TX_CH1 */
	{
		.name	    = HDMI_TX_CH1,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 9, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* HDMI_TX_CH2 */
	{
		.name	    = HDMI_TX_CH2,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 10, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* HDMI_TX_CLK_CH */
	{
		.name	    = HDMI_TX_CLK_CH,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 11, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
#endif
	/* HDMI_TX */
	{
		.name	    = MT_ANA_HDMITX,	/* (ch0-ch2, clk ch) */
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 8, 4,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},

#if 0
	/* ADAC BUF_RCH */
	{
		.name	    = ADAC_BUF_RCH,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 12+0, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ADAC_RCH */
	{
		.name	    = ADAC_RCH,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 12+1, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ADAC_1BIT_INT (A1) */
	{
		.name		= ADAC_1BIT_INT,
		.power_down = 1,
		.stat		= MT_INIT_ANA(ANA_AO_REG0, 12+2, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ADAC VDDB1P2 */
	{
		.name		= ADAC_VDDB1P2,
		.power_down = 1,
		.stat		= MT_INIT_ANA(ANA_AO_REG0, 12+3, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ADAC BUF_LCH */
	{
		.name		= ADAC_BUF_LCH,
		.power_down = 1,
		.stat		= MT_INIT_ANA(ANA_AO_REG0, 12+4, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ADAC_LCH */
	{
		.name		= ADAC_LCH,
		.power_down = 1,
		.stat		= MT_INIT_ANA(ANA_AO_REG0, 12+5, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ADAC_VDD2P7 (A1) */
	{
		.name		= ADAC_VDD2P7,
		.power_down = 1,
		.stat		= MT_INIT_ANA(ANA_AO_REG0, 12+7, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
#endif
#if 1
	/* ADAC */
	{
		.name		= MT_ANA_ADAC,
		.power_down = 1,
		/*
		 * Off: 11111111
		 *  On:
		 *     2Vrms: 01100110
		 *      0dBu: 00011001
		 */
		.stat		= MT_INIT_ANA(ANA_AO_REG0, 12, 8,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
#endif

	/* CADC */
	{
		.name	    = MT_ANA_CADC,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 20, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* SADC */
	{
		.name	    = MT_ANA_SADC,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 21, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* Process Monitor */
	{
		.name	    = MT_ANA_PROCMON,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 22, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* RNG1 */
	{
		.name	    = MT_ANA_RNG1,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 24, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_BIT0_BIT24_LOCK_SHIFT)
	},
	/* USB_IMP_CAL */
	{
		.name	    = USB_IMP_CAL,	/*Hide*/
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 25, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_BIT0_BIT24_LOCK_SHIFT)
	},
	/* Temp Sensor */
	{
		.name	    = MT_ANA_TSENSOR,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 27, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* ADAC_OUT */
	{
		.name		= ADAC_OUT, /*Hide*/
		.power_down = 1,
		.stat		= MT_INIT_ANA(ANA_AO_REG0, 28, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* RNG2 */
	{
		.name	    = MT_ANA_RNG2,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 29, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},

	/* VDAC_DET */
	{
		.name	    = MT_ANA_VDAC_DET,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 1, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* ADAC_MUTE */
	{
		.name		= ADAC_MUTE,
		.power_down = 0,
		.stat		= MT_INIT_ANA(ANA_AO_REG1, 2, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* MASTER_REGULATOR */
	{
		.name	    = MT_ANA_MSTR_REG,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 4, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* USB3_PHY */
	{
		.name	    = MT_ANA_USB3_PHY,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 5, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},

	/* A1+ */
	/* ADAC_EXT_1BIT_PCM */
	{
		.name		= ADAC_EXT_1BIT_PCM,
		.power_down = 0,
		.stat		= MT_INIT_ANA(ANA_TOP_REG3, 14, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, ANA_TOP_REG3_LOCK_SHIFT)
	},
	/* ADAC_INT_1BIT_PCM */
	{
		.name		= ADAC_INT_1BIT_PCM,
		.power_down = 0,
		.stat		= MT_INIT_ANA(ANA_TOP_REG3, 15, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, ANA_TOP_REG3_LOCK_SHIFT)
	},
	/* ADAC_OE */
	{
		.name		= ADAC_OE,
		.power_down = 0,
		.stat		= MT_INIT_ANA(ANA_TOP_REG3, 16, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, ANA_TOP_REG3_LOCK_SHIFT)
	},

	/* VHD_CLK_ANA */
	{
		.name		= VHD_CLK_ANA,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_CLKGEN_PDSYS, 11, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},
	/* VSD_CLK_ANA */
	{
		.name		= VSD_CLK_ANA,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_CLKGEN_PDSYS, 19, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},
	/* DRV0_CLK_ANA */
	{
		.name		= DRV0_CLK_ANA,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_CLKGEN_PDSYS, 24, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},
	/* DRV1_CLK_ANA */
	{
		.name		= DRV1_CLK_ANA,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_CLKGEN_PDSYS, 25, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},

	/* EPHY */
	{
		.name	    = MT_ANA_EPHY,
		.power_down = 0,
		.stat	    = MT_INIT_ANA(REG_CLKGEN_EPHY, 25, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_EPHY_REG_LOCK_SHIFT)
	},

	/* VSD SSC(HDMI SSC) */
	{
		.name		= MT_ANA_VSDSSC,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_CLKGEN_VSDSSC, 4, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VSDSSC_REG_LOCK_SHIFT)
	},

	/* HDMI DCC */
	{
		.name		= HDMI_DCC,	/*Hide*/
		.power_down = 0,
		.stat		= MT_INIT_ANA(CLKGEN_USBACLK_REG0, 1, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_USBACLK_REG_LOCK_SHIFT)
	},

	/* KEY_ADC */
	{
		.name		= MT_ANA_KADC,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_KEY_ADC_CFG, KEY_ADC_PD_SHIFT, 1,
								0, 0, 0)
	},
	/* KEY_ADC2 */
	{
		.name		= MT_ANA_KADC2,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_KEY_ADC2_CFG, KEY_ADC_PD_SHIFT, 1,
								0, 0, 0)
	},

	/* AVS_DAC_CPU */
	{
		.name		= AVS_CPU,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_AVS_CPU_CTRL, AVS_PD_CPU_SHIFT, 1,
								0, 0, 0)
	},

	/* AVS_DAC_SYS */
	{
		.name		= AVS_SYS,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_AVS_SYS_CTRL, AVS_PD_SYS_SHIFT, 1,
								0, 0, 0)
	},

	/* PLL */

	/* EPHYPLL */
	{
		.name	    = MT_EPHYPLL,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 30, 2,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	/* USB30PLL */
	{
		.name	    = MT_USB30PLL,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 6, 2,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* USB3P0 PLL DCC */
	{
		.name		= USB30_PLL_DCC, /*Hide*/
		.power_down = 1,
		.stat		= MT_INIT_ANA(USB3P0_SSC_REG2, 13, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, USB3P0_SSC_REG2_LOCK_SHIFT)
	},
	/* VSDPLL */
	{
		.name	    = MT_VSDPLL,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 24, 2,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* VHDPLL */
	{
		.name	    = MT_VHDPLL,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 26, 2,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* USBPLL */
	{
		.name	    = MT_USBPLL,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 28, 3,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* CPUPLL(ADCPLL) */
	{
		.name	    = MT_ADCPLL,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG1, 31, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* REFPLL */
	{
		.name	    = MT_REFPLL,
		.power_down = 1,
		.stat		= MT_INIT_ANA(ANA_AO_REG1, 21, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG1_LOCK_SHIFT)
	},
	/* AUDIOPLL */
	{
		.name		= MT_AUDIOPLL,
		.power_down = 1,
		.stat		= MT_INIT_ANA(CLKGEN_USBACLK_REG2, 14, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_USBACLK_REG_LOCK_SHIFT)
	},
	/* DDRPLL */
	{
		.name	    = MT_DDRPLL,
		.power_down = 1,
		/* phy_ctr0, 'pd_pll' bit4 */
		.stat	    = MT_INIT_ANA(REG_DDR_PHY_BASE+0x4, 4, 3,
								0, 0, 0/*FIXME*/)
	},
	/* DDR PLL SSC */
	{
		.name		= DDR_PLL_SSC, /*Hide*/
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_DDR_PHY_BASE+0xB4, 4, 1,
								0, 0, 0/*FIXME*/)
	},
	/* ARMPLL */
	{
		.name	    = MT_ARMPLL,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(REG_APCPU_ARMPLL_PD, ARMPLL_PD_SHIFT, 1,
								0, 0, 0/*FIXME*/)
	},
};

#else

/* CONFIG_TARGET_SYMPHONY6_LITE || CONFIG_TARGET_SYMPHONY6_MINI */

struct mt_ana_dev mt_ana_table[] =
{
	/* TODO: */

	/* VDAC */
	{
		.name	    = MT_ANA_VDAC,
		.power_down = 1,
		.stat	    = MT_INIT_ANA(ANA_AO_REG0, 6, 1,
								REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},

	/* VHD_CLK_ANA */
	{
		.name		= VHD_CLK_ANA,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_CLKGEN_PDSYS, 11, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},
	/* VSD_CLK_ANA */
	{
		.name		= VSD_CLK_ANA,
		.power_down = 1,
		.stat		= MT_INIT_ANA(REG_CLKGEN_PDSYS, 19, 1,
								REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},
};

#endif

u32 mt_ana_table_size = sizeof(mt_ana_table) / sizeof(mt_ana_table[0]);

mt_reg_value_t suspend_analog_regs[] =
{
	{ANA_AO_REG0, 0x00300040},
	{ANA_AO_REG1, 0x004A00E0},

	//FIXME: kernel panic!
	//{ANA_TOP_REG0, 0},

	//{ANA_TOP_REG1, 0},
	//{ANA_TOP_REG2, 0xD0821000},

	//FIXME: kernel panic!
	//{ANA_TOP_REG3, 0},

	//FIXME: kernel panic!
	//{REG_CLKGEN_EPHY, 0x0000B900},

	//for "audiopll"
	{CLKGEN_USBACLK_REG2, 0x00001D04},

	//for smc "kadc"
	//{REG_KADC_BASE, 0x00002D00},

	//for tsensor
	{REG_TSENSOR_TH, 0},
	{REG_TSENSOR_RST_REQ_TH, 0x1138},
	{REG_TSENSOR_INT, 0x7000},

};

u32 suspend_analog_regs_size = sizeof(suspend_analog_regs) / sizeof(suspend_analog_regs[0]);

