/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_mod_reset_sym6.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT Symphony6 reset clock modules definition.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __UBOOT__
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"
#else
/* RTOS */
#include <sys_types.h>
#include <sys_define.h>

#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"
#endif

#include "mt_mod_reset.h"
#include "mt_crm_clock_reg.h"
#include "mt_crm_reset_reg.h"
#include "mt_crm_top_reg.h"
#include "mt_crm_lock_reg.h"

#ifdef __UBOOT__
#include "mt_analog_reg.h"
#elif defined(__KERNEL__)
#include "../analog/mt_analog_reg.h"
#else
/* RTOS */
#include "../analog/mt_analog_reg.h"
#endif

#include "mt_mod_internal.h"

#if !defined(CONFIG_TARGET_SYMPHONY6_LITE) && !defined(CONFIG_TARGET_SYMPHONY6_MINI)

/* for optimize size */
#define CFG_RESET_OPT_SIZE

/* Symphony6 Simple Reset Modules */
struct mt_mod_rst_simple mt_rst_modules[] =
{
	/* analog modules */

	//USB0
	{
		.name = MT_ANA_USB0,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(USB0_REG1, 13, 0, 0, 0)
	},
	//USB1
	{
		.name = MT_ANA_USB1,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(USB1_REG1, 13, 0, 0, 0)
	},
	//VDAC
	{
		.name = MT_ANA_VDAC,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(VDAC_REG0, 15, 0, 0, 0)
	},
	//HDMI_TX
	{
		.name = MT_ANA_HDMITX,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(ANA_AO_REG0, HDMI_TX_RESET_SHIFT,
							REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	//ADAC
	{
		.name = MT_ANA_ADAC,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(ANA_TOP_REG1, ADAC_RESET_SHIFT,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, ANA_TOP_REG1_LOCK_SHIFT)
	},
	//SADC
	{
		.name = MT_ANA_SADC,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(SADC_REG0, 8, 0, 0, 0)
	},
	//Process Monitor
	{
		.name = MT_ANA_PROCMON,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(ANA_AO_REG0, PROC_MNT_RESET_SHIFT,
							REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},
	//RNG1
	{
		.name = MT_ANA_RNG1,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(ANA_TOP_REG2, RNG1_RESET_SHIFT,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, ANA_TOP_REG2_LOCK_SHIFT)
	},
	//Temp Sensor
	{
		.name = MT_ANA_TSENSOR,
		.active_low = 0,
		.delay_us = CFG_MT_ANA_RESET_UDELAY_MAX,
		.core = MT_INIT_RST(REG_TSENSOR, TSENSOR_RESET_SHIFT,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, TSENSOR_REG_LOCK_SHIFT)
	},
	//RNG2
	{
		.name = MT_ANA_RNG2,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(ANA_TOP_REG2, RNG2_RESET_SHIFT,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, ANA_TOP_REG2_LOCK_SHIFT)
	},
	//VSDSSC
	{
		.name = MT_ANA_VSDSSC,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_VSDSSC, 6,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VSDSSC_REG_LOCK_SHIFT)
	},
	//EPHY(analog)
	{
		.name = MT_ANA_EPHY,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_EPHY, 7,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_EPHY_REG_LOCK_SHIFT)
	},

	//KADC
	{
		.name = MT_ANA_KADC,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_KEY_ADC_CFG, KEY_ADC_RST_SHIFT,
							0, 0, 0)
	},
	//KADC2
	{
		.name = MT_ANA_KADC2,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_KEY_ADC2_CFG, KEY_ADC_RST_SHIFT,
							0, 0, 0)
	},

	//DEMO
	{
		.name = MT_CLK_DEMO,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_TOPCLK_CTRL6, DEMO_SRESET_SHIFT,
						0, 0, 0)	/* FIXME */
	},

	/* pll modules */

	//CPUPLL(ADCPLL)
	{
		.name = MT_ADCPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_CPUPLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_CPUPLL_REG_LOCK_SHIFT)
	},
	//USBPLL
	{
		.name = MT_USBPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_USBPLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_USBPLL_REG_LOCK_SHIFT)
	},
	//VHDPLL
	{
		.name = MT_VHDPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_VHDPLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VHDPLL_REG_LOCK_SHIFT)
	},
	//VSDPLL
	{
		.name = MT_VSDPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_VSDPLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VSDPLL_REG_LOCK_SHIFT)
	},
	//REFPLL
	{
		.name = MT_REFPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(CLKGEN_USBACLK_REG1, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_USBACLK_REG_LOCK_SHIFT)
	},
	//AUDIOPLL
	{
		.name = MT_AUDIOPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(CLKGEN_USBACLK_REG2, 13,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_USBACLK_REG_LOCK_SHIFT)
	},
	//USB30PLL
	{
		.name = MT_USB30PLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_USB3P0_PLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, USB3P0_PLL_REG_LOCK_SHIFT)
	},
	//EPHYPLL
	{
		.name = MT_EPHYPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_EPHYPLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_EPHY_REG_LOCK_SHIFT)
	},
#ifndef CFG_RESET_OPT_SIZE
	//DDRPLL
	{
		.name = MT_DDRPLL,
		INIT_ANA_RST_VAL,
		/* ddr_pll_reg */
		.core = MT_INIT_RST(REG_DDR_PHY_BASE+0x70, 14, 0, 0, 0/*no slock/lock*/)
	},
#endif
	//ARMPLL
	{
		.name = MT_ARMPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_APCPU_ARMPLL, 14,
							REG_APCPU_SLOCK, REG_APCPU_LOCK, ARMPLL_CFG_LOCK_SHIFT)
	},

	/* digital modules */

	//EPHY(digital)
	{
		.name = MT_RST_EPHY,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_MAC_SRSTN, EPHY_SRSTN_SHIFT,
						REG_MAC_SLOCK, REG_MAC_LOCK, MAC_SRSTN_LOCK_SHIFT)
	},

#ifndef CFG_RESET_OPT_SIZE
	//SWTMISC
	{
		.name = MT_RST_SWT_MISC, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWTMISC_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWTMISC_SRST_LOCK_SHIFT)
	},
	//SWMNTSOSC
	{
		.name = MT_RST_SWMNT_SOSC, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWMNTSOSC_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWMNTSOSC_SRST_LOCK_SHIFT)
	},
	//SWMNTXTAL
	{
		.name = MT_RST_SWMNT_XTAL, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWMNTXTAL_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWMNTXTAL_SRST_LOCK_SHIFT)
	},
#endif

	//SWWDOG1
	{
		.name = MT_RST_SWWDOG1, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWWDOG1_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWWDOG1_SRST_LOCK_SHIFT)
	},
	//SWWDOG0
	{
		.name = MT_RST_SWWDOG0, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWWDOG0_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWWDOG0_SRST_LOCK_SHIFT)
	},
	//SWTIMER3
	{
		.name = MT_RST_SWTM_CLK3, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWTIMER3_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWTIMER3_SRST_LOCK_SHIFT)
	},
	//SWTIMER2
	{
		.name = MT_RST_SWTM_CLK2, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWTIMER2_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWTIMER2_SRST_LOCK_SHIFT)
	},
	//SWTIMER1
	{
		.name = MT_RST_SWTM_CLK1, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWTIMER1_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWTIMER1_SRST_LOCK_SHIFT)
	},
	//SWTIMER0
	{
		.name = MT_RST_SWTM_CLK0, 		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SYS_SRSTN, SWTIMER0_SRSTN_SHIFT,
						REG_SYS_SLOCK, REG_SYS_LOCK, SWTIMER0_SRST_LOCK_SHIFT)
	},
	//UART1
	{
		.name = MT_CLK_UART1,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_INTF_SRSTN, UART1_SRSTN_SHIFT,
						REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, UART1_SRST_LOCK_SHIFT)
	},
	//UART0
	{
		.name = MT_CLK_UART0,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_INTF_SRSTN, UART0_SRSTN_SHIFT,
						REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, UART0_SRST_LOCK_SHIFT)
	},
	//I2C1
	{
		.name = MT_CLK_I2C1,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_INTF_SRSTN, I2C1_SRSTN_SHIFT,
						REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, I2C1_SRST_LOCK_SHIFT)
	},
	//I2C0
	{
		.name = MT_CLK_I2C0,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_INTF_SRSTN, I2C0_SRSTN_SHIFT,
						REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, I2C0_SRST_LOCK_SHIFT)
	},

	//DISP_DI
	{
		.name = MT_CLK_DISP_DI,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_DISP_SRSTN, DISPDI_SRSTN_SHIFT,
						REG_DISP_SLOCK, REG_DISP_LOCK, DISPDI_LOCK_SHIFT)
	},
	//DISP_OSDC
	{
		.name = DISP_OSDC_CLK, 			/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_DISP_SRSTN, DISPOSDC_SRSTN_SHIFT,
						REG_DISP_SLOCK, REG_DISP_LOCK, DISPOSDC_LOCK_SHIFT)
	},
	//DISP_VDC
	{
		.name = DISP_VDC_CLK,			/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_DISP_SRSTN, DISPVDC_SRSTN_SHIFT,
						REG_DISP_SLOCK, REG_DISP_LOCK, DISPVDC_LOCK_SHIFT)
	},

	//AOUT_MCLK
	{
		.name = AOUT_MCLK,				/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AOUT_SRSTN, AOUT_MCLK_SRSTN_SHIFT,
						REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUT_MCLK_LOCK_SHIFT)
	},
	//AOUT_ADAC
	{
		.name = MT_CLK_ADAC,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AOUT_SRSTN, AOUT_ADAC_SRSTN_SHIFT,
						REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUT_ADAC_LOCK_SHIFT)
	},

	//VBI
	{
		.name = MT_CLK_VBI,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_VOUT_SRSTN, VBI_SRSTN_SHIFT,
						REG_VOUT_SLOCK, REG_VOUT_LOCK, VBI_LOCK_SHIFT)
	},

#ifndef CFG_RESET_OPT_SIZE
	//DAI_TX
	{
		.name = DAI_TX_CLK,			/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_DAI_SRSTN, DAITX_SRSTN_SHIFT,
						REG_DAI_SLOCK, REG_DAI_LOCK, DAITX_LOCK_SHIFT)
	},
	//DAI_RX
	{
		.name = DAI_RX_CLK,			/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_DAI_SRSTN, DAIRX_SRSTN_SHIFT,
						REG_DAI_SLOCK, REG_DAI_LOCK, DAIRX_LOCK_SHIFT)
	},
	//DAI_PDM
	{
		.name = DAI_PDM_CLK,		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_DAI_SRSTN, DAIPDM_SRSTN_SHIFT,
						REG_DAI_SLOCK, REG_DAI_LOCK, DAIPDM_LOCK_SHIFT)
	},
	//DAI_DAC
	{
		.name = DAI_DAC_CLK,		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_DAI_SRSTN, DAIDAC_SRSTN_SHIFT,
						REG_DAI_SLOCK, REG_DAI_LOCK, DAIDAC_LOCK_SHIFT)
	},
#endif

	//TS0
	{
		.name = TS0_CLK,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TS0_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TS0_LOCK_SHIFT)
	},
	//TS1
	{
		.name = TS1_CLK,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TS1_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TS1_LOCK_SHIFT)
	},
	//TS2
	{
		.name = TS2_CLK,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TS2_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TS2_LOCK_SHIFT)
	},
	//TS3
	{
		.name = TS3_CLK,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TS3_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TS3_LOCK_SHIFT)
	},
	//TSPOOL
	{
		.name = MT_RST_TSI_TSPOOL,	/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TSPOOL_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TSPOOL_LOCK_SHIFT)
	},
	//DEMUX
	{
		.name = MT_RST_TSI_DEMUX,	/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, DEMUX_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, DEMUX_LOCK_SHIFT)
	},
	//TSI_T2MI
	{
		.name = MT_RST_TSI_T2MI, 	/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TSI_T2MI_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TSI_T2MI_LOCK_SHIFT)
	},
	//TSI_CI_REG
	{
		.name = MT_RST_TSI_CI_REG,	/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TSI_CIREG_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TSI_CI_REG_LOCK_SHIFT)
	},
	//TSI_FP_REG
	{
		.name = MT_RST_TSI_FP_REG,	/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_TSI_SRSTN, TSI_FPREG_SRSTN_SHIFT,
						REG_TSI_SLOCK, REG_TSI_LOCK, TSI_FP_REG_LOCK_SHIFT)
	},

	//CI
	{
		.name = MT_CLK_CI,
		INIT_DIG_RST_VAL,
		.ahb = MT_INIT_RST(REG_CI_SRSTN, CIAHB_SRSTN_SHIFT,
						REG_CI_SLOCK, REG_CI_LOCK, CIAHB_LOCK_SHIFT),
	},

	//KL
	{
		.name = MT_CLK_KLE,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SECURE_SRSTN, KL_SRSTN_SHIFT,
						REG_SECURE_SLOCK, REG_SECURE_LOCK, KL_LOCK_SHIFT)
	},
	//SECHD0
	{
		.name = MT_CLK_SECHD0,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SECURE_SRSTN, SECHD0_SRSTN_SHIFT,
						REG_SECURE_SLOCK, REG_SECURE_LOCK, SECHD0_LOCK_SHIFT)
	},
	//GLITCH_DET
	{
		.name = MT_CLK_GLITCHDET,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_SECURE_SRSTN, GLITCHDET_SRSTN_SHIFT,
						REG_SECURE_SLOCK, REG_SECURE_LOCK, GLITCH_DET_LOCK_SHIFT)
	},

	//IFCP_GLB(MT_CLK_IFCP_SYS)
	{
		.name = MT_CLK_IFCP_SYS,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_IFCP_GLB_SRSTN, IFCP_SRSTN_SHIFT,
						0/*no slock*/, REG_SHAREREG_VSCPU_RST_CTRL, IFCP_SRSTN_LOCK0)
	},

	//AO_LPM
	{
		.name = MT_CLK_AO_LPM,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_LPMRST, AOLPM_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, LPM_LOCK_SHIFT)
	},
	//AO_IRDA
	{
		.name = MT_CLK_AO_IRDA,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_IRDARST, IRDA_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, IRDA_LOCK_SHIFT)
	},
	//AO_RTC
	{
		.name = MT_RST_AO_RTC,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_RTCRST, RTC_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, RTC_LOCK_SHIFT)
	},
	//AO_LEDKB
	{
		.name = MT_CLK_AO_LEDKB,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_LEDKBRST, LEDKB_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, LEDKB_LOCK_SHIFT)
	},
	//AO_GPIO
	{
		.name = MT_CLK_AO_GPIO,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_GPIORST, AOGPIO_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, GPIO_LOCK_SHIFT)
	},
	//AO_KADC
	{
		.name = MT_CLK_AO_KADC,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_KADCRST, KADC_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, KADC_LOCK_SHIFT)
	},
	//AO_FPI2C
	{
		.name = MT_CLK_AO_FPI2C,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_FPI2CRST, FPI2C_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, FPI2C_LOCK_SHIFT)
	},
	//AO_FPSPI_REG
	{
		.name = MT_RST_AO_FPSPI_REG,		/*Hide*/
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_FPSPIRST, FPSPI_REG_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, FPSPI_LOCK_SHIFT),
	},
	//AO_FPSPI
	{
		.name = MT_CLK_AO_FPSPI,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_FPSPIRST, FPSPI_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, FPSPI_LOCK_SHIFT)
	},
	//AO_PINMUX
	{
		.name = MT_CLK_AO_PINMUX,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_PINMUXRST, AOPINMUX_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, PINMUX_LOCK_SHIFT)
	},
	//AO_TIMER
	{
		.name = MT_CLK_AO_TIMER,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_TIMERRST, AOTIMER_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, TIMER_LOCK_SHIFT)
	},
	//AO_MNTXTAL
	{
		.name = MT_RST_AO_MNT_XTAL,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_MNTRST, MNTXTAL_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, MNT_LOCK_SHIFT)
	},
	//AO_MNTSOSC
	{
		.name = MT_RST_AO_MNT_SOSC,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_MNTRST, MNTSOSC_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, MNT_LOCK_SHIFT)
	},
	//AO_MAILBOX
	{
		.name = MT_CLK_AO_MAILBOX,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_MAILBOXRST, AOMAILBOX_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, MAILBOX_LOCK_SHIFT)
	},
	//AO_CEC
	{
		.name = MT_CLK_AO_CEC,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_CECRST, CEC_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, CEC_LOCK_SHIFT)
	},
	//AO_AVS
	{
		.name = MT_CLK_AO_AVS,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_AVSRST, AVS_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, AVS_LOCK_SHIFT)
	},
	//AO_AGTIMER
	{
		.name = MT_CLK_AO_AGTIMER,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_AGTIMERRST, AGTIMER_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, AGTIMER_LOCK_SHIFT)
	},
	//AO_WDOG
	{
		.name = MT_RST_AO_WDOG,
		INIT_DIG_RST_VAL,
		.core = MT_INIT_RST(REG_AO_WDOGRST, AOWDOG_SRSTN_SHIFT,
						REG_AOCRM_SLOCK, REG_AOCRM_LOCK, WDOG_LOCK_SHIFT)
	},

};

u32 mt_rst_modules_count = sizeof(mt_rst_modules) / sizeof(mt_rst_modules[0]);

//---------------------------------------------------------------------------//

/* Symphony6 simple reset modules for Composite Reset Modules */

//MAC
static struct mt_mod_rst_simple mac_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_MAC_SRSTN, MACAHB_SRSTN_SHIFT,
					REG_MAC_SLOCK, REG_MAC_LOCK, MAC_SRSTN_LOCK_SHIFT),
};

static struct mt_mod_rst_simple mac_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_MAC_SRSTN, MACAXI_SRSTN_SHIFT,
					REG_MAC_SLOCK, REG_MAC_LOCK, MAC_SRSTN_LOCK_SHIFT),
};

//APCPU - TODO

//AVCPU
static struct mt_mod_rst_simple avcpu_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_AVCPU_SRSTN, AVCPUAHB_SRSTN_SHIFT,
					REG_AVCPU_SLOCK, REG_AVCPU_LOCK, AVCPUAHB_SRSTN_LOCK_SHIFT),
};
static struct mt_mod_rst_simple avcpu_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_AVCPU_SRSTN, AVCPUAXI_SRSTN_SHIFT,
					REG_AVCPU_SLOCK, REG_AVCPU_LOCK, AVCPUAXI_SRSTN_LOCK_SHIFT),
};
static struct mt_mod_rst_simple avcpu_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_AVCPU_SRSTN, AVCPUCORE_SRSTN_SHIFT,
					REG_AVCPU_SLOCK, REG_AVCPU_LOCK, AVCPUCORE_SRSTN_LOCK_SHIFT)
};

//SPDMA
static struct mt_mod_rst_simple spdma_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_AVCPU_SRSTN, SPDMAAHB_SRSTN_SHIFT,
					REG_AVCPU_SLOCK, REG_AVCPU_LOCK, SPDMAAHB_SRSTN_LOCK_SHIFT),
};
static struct mt_mod_rst_simple spdma_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_AVCPU_SRSTN, SPDMAAXI_SRSTN_SHIFT,
					REG_AVCPU_SLOCK, REG_AVCPU_LOCK, SPDMAAXI_SRSTN_LOCK_SHIFT),
};
static struct mt_mod_rst_simple spdma_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_AVCPU_SRSTN, SPDMACORE_SRSTN_SHIFT,
					REG_AVCPU_SLOCK, REG_AVCPU_LOCK, SPDMACORE_SRSTN_LOCK_SHIFT)
};

#ifndef CFG_RESET_OPT_SIZE
//OIC
static struct mt_mod_rst_simple oic_apb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.apb = MT_INIT_RST(REG_SYS_SRSTN, OIC_APB_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, OIC_APB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple oic_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_SYS_SRSTN, OIC_AXI_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, OIC_AXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple oic_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SYS_SRSTN, OIC_OMC_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, OIC_OMC_LOCK_SHIFT),
};

//OMC
static struct mt_mod_rst_simple omc_phy =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SYS_SRSTN, OMC_PHY_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, OMC_PHY_LOCK_SHIFT),
};
static struct mt_mod_rst_simple omc_apb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.apb = MT_INIT_RST(REG_SYS_SRSTN, OMC_PCLK_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, OMC_PCLK_LOCK_SHIFT),
};
static struct mt_mod_rst_simple omc_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_SYS_SRSTN, OMC_RCLK_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, OMC_RCLK_LOCK_SHIFT),
};
static struct mt_mod_rst_simple omc_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SYS_SRSTN, OMC_CORE_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, OMC_CORE_LOCK_SHIFT),
};
#endif

//DMA
static struct mt_mod_rst_simple dma_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_SYS_SRSTN, DMAAHB_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, DMAAHB_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple dma_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_SYS_SRSTN, DMAAXI_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, DMAAXI_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple dma_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SYS_SRSTN, DMACORE_SRSTN_SHIFT,
					REG_SYS_SLOCK, REG_SYS_LOCK, DMACORE_SRST_LOCK_SHIFT)
};

//SDIO1
static struct mt_mod_rst_simple sdio1_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_INTF_SRSTN, SDIO1AHB_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SDIO1AHB_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple sdio1_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_INTF_SRSTN, SDIO1AXI_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SDIO1AXI_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple sdio1_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, SDIO1CORE_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SDIO1CORE_SRST_LOCK_SHIFT)
};

//SDIO0
static struct mt_mod_rst_simple sdio0_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_INTF_SRSTN, SDIO0AHB_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SDIO0AHB_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple sdio0_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_INTF_SRSTN, SDIO0AXI_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SDIO0AXI_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple sdio0_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, SDIO0CORE_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SDIO0CORE_SRST_LOCK_SHIFT)
};

//PNAND
static struct mt_mod_rst_simple pnand_reg =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, PNANDREG_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, PNANDREG_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple pnand_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_INTF_SRSTN, PNANDAHB_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, PNANDAHB_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple pnand_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_INTF_SRSTN, PNANDAXI_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, PNANDAXI_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple pnand_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, PNANDCORE_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, PNANDCORE_SRST_LOCK_SHIFT)
};

//SPI0
static struct mt_mod_rst_simple spi0_reg =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, SPI0REG_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SPI0REG_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple spi0_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_INTF_SRSTN, SPI0AHB_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SPI0AHB_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple spi0_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, SPI0CORE_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SPI0CORE_SRST_LOCK_SHIFT)
};

//USB30
static struct mt_mod_rst_simple usb30_apb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.apb = MT_INIT_RST(REG_INTF_SRSTN, USB_DPHY_BRIDGE_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB30_APB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple usb30_ss_phy =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, USB30_SS_PHY_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB30_PHY_LOCK_SHIFT),
};

//USB1
static struct mt_mod_rst_simple usb1_phy =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, USB1PHY_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB1PHY_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple usb1_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_INTF_SRSTN, USB1AHB_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB1AHB_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple usb1_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_INTF_SRSTN, USB1AXI_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB1AXI_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple usb1_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, USB1CORE_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB1CORE_SRST_LOCK_SHIFT)
};

//USB0
static struct mt_mod_rst_simple usb0_phy =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, USB0PHY_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB0PHY_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple usb0_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_INTF_SRSTN, USB0AHB_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB0AHB_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple usb0_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_INTF_SRSTN, USB0AXI_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB0AXI_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple usb0_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, USB0CORE_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, USB0CORE_SRST_LOCK_SHIFT)
};

//SMC
static struct mt_mod_rst_simple smc_phy =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, SMCPHY_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SMCPHY_SRST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple smc_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_INTF_SRSTN, SMC_SRSTN_SHIFT,
					REG_INTF_RST_SLOCK, REG_INTF_RST_LOCK, SMC_SRST_LOCK_SHIFT)
};

//GRA
static struct mt_mod_rst_simple gra_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_GRA_SRSTN, GRAAHB_SRSTN_SHIFT,
					REG_GRA_SLOCK, REG_GRA_LOCK, GRAAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple gra_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_GRA_SRSTN, GRAAXI_SRSTN_SHIFT,
					REG_GRA_SLOCK, REG_GRA_LOCK, GRAAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple gra_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_GRA_SRSTN, GRACORE_SRSTN_SHIFT,
					REG_GRA_SLOCK, REG_GRA_LOCK, GRACORE_LOCK_SHIFT)
};

//JPG
static struct mt_mod_rst_simple jpg_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_JPG_SRSTN, JPGAHB_SRSTN_SHIFT,
					REG_JPG_SLOCK, REG_JPG_LOCK, JPGAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple jpg_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_JPG_SRSTN, JPGAXI_SRSTN_SHIFT,
					REG_JPG_SLOCK, REG_JPG_LOCK, JPGAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple jpg_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_JPG_SRSTN, JPGCORE_SRSTN_SHIFT,
					REG_JPG_SLOCK, REG_JPG_LOCK, JPGCORE_LOCK_SHIFT)
};

//DISP
static struct mt_mod_rst_simple disp_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_DISP_SRSTN, DISPAHB_SRSTN_SHIFT,
					REG_DISP_SLOCK, REG_DISP_LOCK, DISPAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple disp_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_DISP_SRSTN, DISPAXI_SRSTN_SHIFT,
					REG_DISP_SLOCK, REG_DISP_LOCK, DISPAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple disp_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_DISP_SRSTN, DISPCORE_SRSTN_SHIFT,
					REG_DISP_SLOCK, REG_DISP_LOCK, DISPCORE_LOCK_SHIFT)
};
static struct mt_mod_rst_simple disp_sdvenc =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_DISP_SRSTN, DISP_SDVENC_SRSTN_SHIFT,
					REG_DISP_SLOCK, REG_DISP_LOCK, DISP_SDVENC_LOCK_SHIFT)
};
static struct mt_mod_rst_simple disp_hdvenc =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_DISP_SRSTN, DISP_HDVENC_SRSTN_SHIFT,
					REG_DISP_SLOCK, REG_DISP_LOCK, DISP_HDVENC_LOCK_SHIFT)
};

//VDEC
static struct mt_mod_rst_simple vdec_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_VDEC_SRSTN, VDECAHB_SRSTN_SHIFT,
					REG_VDEC_SLOCK, REG_VDEC_LOCK, VDECAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple vdec_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_VDEC_SRSTN, VDECAXI_SRSTN_SHIFT,
					REG_VDEC_SLOCK, REG_VDEC_LOCK, VDECAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple vdec_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_VDEC_SRSTN, VDECCORE_SRSTN_SHIFT,
					REG_VDEC_SLOCK, REG_VDEC_LOCK, VDECCORE_LOCK_SHIFT)
};
static struct mt_mod_rst_simple vdeccore_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_VDEC_SRSTN, VDECCORECORE_SRSTN_SHIFT,
					REG_VDEC_SLOCK, REG_VDEC_LOCK, VDECCORE_CORE_LOCK_SHIFT)
};

//AOUT
static struct mt_mod_rst_simple aout_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_AOUT_SRSTN, AOUTAHB_SRSTN_SHIFT,
					REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUTAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple aout_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_AOUT_SRSTN, AOUTAXI_SRSTN_SHIFT,
					REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUTAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple aout_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_AOUT_SRSTN, AOUTCORE_SRSTN_SHIFT,
					REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUTCORE_LOCK_SHIFT)
};
static struct mt_mod_rst_simple aout_src =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_AOUT_SRSTN, AOUT_SRC_SRSTN_SHIFT,
					REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUT_SRC_LOCK_SHIFT)
};

//HDMI
static struct mt_mod_rst_simple hdmi_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_VOUT_SRSTN, HDMIAHB_SRSTN_SHIFT,
					REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_APB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple hdmi_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_VOUT_SRSTN, HDMICORE_SRSTN_SHIFT,
					REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CORE_LOCK_SHIFT)
};

//HDVENC
static struct mt_mod_rst_simple hdvenc_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_VOUT_SRSTN, HDVENCAHB_SRSTN_SHIFT,
					REG_VOUT_SLOCK, REG_VOUT_LOCK, HDVENC_AHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple hdvenc_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_VOUT_SRSTN, HDVENCHD_SRSTN_SHIFT,
					REG_VOUT_SLOCK, REG_VOUT_LOCK, HDVENC_HD_LOCK_SHIFT)
};

//SDVENC
static struct mt_mod_rst_simple sdvenc_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_VOUT_SRSTN, SDVENCAHB_SRSTN_SHIFT,
					REG_VOUT_SLOCK, REG_VOUT_LOCK, SDVENC_AHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple sdvenc_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_VOUT_SRSTN, SDVENCCORE_SRSTN_SHIFT,
					REG_VOUT_SLOCK, REG_VOUT_LOCK, SDVENC_CORE_LOCK_SHIFT)
};

//PNG
static struct mt_mod_rst_simple png_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_PNG_SRSTN, PNGAHB_SRSTN_SHIFT,
					REG_PNG_SLOCK, REG_PNG_LOCK, PNGAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple png_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_PNG_SRSTN, PNGAXI_SRSTN_SHIFT,
					REG_PNG_SLOCK, REG_PNG_LOCK, PNGAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple png_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_PNG_SRSTN, PNGCORE_SRSTN_SHIFT,
					REG_PNG_SLOCK, REG_PNG_LOCK, PNGCORE_LOCK_SHIFT)
};

//#ifndef CFG_RESET_OPT_SIZE
//DAI
static struct mt_mod_rst_simple dai_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_DAI_SRSTN, DAIAHB_SRSTN_SHIFT,
					REG_DAI_SLOCK, REG_DAI_LOCK, DAIAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple dai_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_DAI_SRSTN, DAIAXI_SRSTN_SHIFT,
					REG_DAI_SLOCK, REG_DAI_LOCK, DAIAXI_LOCK_SHIFT),
};
//#endif

//GPU
static struct mt_mod_rst_simple gpu_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_GPU_SRSTN, GPUAHB_SRSTN_SHIFT,
					REG_GPU_SLOCK, REG_GPU_LOCK, GPUAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple gpu_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_GPU_SRSTN, GPUAXI_SRSTN_SHIFT,
					REG_GPU_SLOCK, REG_GPU_LOCK, GPUAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple gpu_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_GPU_SRSTN, GPUCORE_SRSTN_SHIFT,
					REG_GPU_SLOCK, REG_GPU_LOCK, GPUCORE_LOCK_SHIFT)
};
static struct mt_mod_rst_simple gpu_mbist =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_GPU_SRSTN, GPUMBIST_SRSTN_SHIFT,
					REG_GPU_SLOCK, REG_GPU_LOCK, GPUMBIST_LOCK_SHIFT),
};
static struct mt_mod_rst_simple gpu_2psram =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_GPU_SRSTN, GPU_2PSRAM_SRSTN_SHIFT,
					REG_GPU_SLOCK, REG_GPU_LOCK, GPU_2PSRAM_LOCK_SHIFT),
};

//GMAC
static struct mt_mod_rst_simple gmac_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_GMAC_SRSTN, GMACAHB_SRSTN_SHIFT,
					REG_GMAC_SLOCK, REG_GMAC_LOCK, GMACAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple gmac_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_GMAC_SRSTN, GMACAXI_SRSTN_SHIFT,
					REG_GMAC_SLOCK, REG_GMAC_LOCK, GMACAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple gmac_crm =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_GMAC_SRSTN, GMAC_CRM_SRSTN_SHIFT,
					REG_GMAC_SLOCK, REG_GMAC_LOCK, GMACCRM_LOCK_SHIFT),
};

//TSI
static struct mt_mod_rst_simple tsi_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_TSI_SRSTN, TSIAHB_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, TSIAHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple tsi_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_TSI_SRSTN, TSIAXI_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, TSIAXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple tsi_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, TSICORE_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, TSICORE_LOCK_SHIFT)
};

//SF
static struct mt_mod_rst_simple sf_reg =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, TSI_SFREG_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, TSI_SF_REG_LOCK_SHIFT),
};
static struct mt_mod_rst_simple sf_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_TSI_SRSTN, SFAXI_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, SF_AXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple sf_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, SF_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, SF_LOCK_SHIFT)
};

//TRPP
static struct mt_mod_rst_simple trpp_reg =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, TSI_TRPPREG_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, TSI_TRPP_REG_LOCK_SHIFT),
};
static struct mt_mod_rst_simple trpp_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_TSI_SRSTN, TRPPAXI_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, TRPP_AXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple trpp_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, TRPP_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, TRPP_LOCK_SHIFT)
};

//SWTSI
static struct mt_mod_rst_simple swtsi_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_TSI_SRSTN, SWTSIAXI_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, SWTSI_AXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple swtsi_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, SWTSI_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, SWTSI_LOCK_SHIFT)
};

//AVSYNC(TSI)
static struct mt_mod_rst_simple avsync_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_TSI_SRSTN, AVSYNCAHB_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, AVSYNC_AHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple avsync_pcr =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, AVSYNC_PCR_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, AVSYNC_PCR_LOCK_SHIFT),
};
static struct mt_mod_rst_simple avsync_sys =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_TSI_SRSTN, AVSYNC_SYS_SRSTN_SHIFT,
					REG_TSI_SLOCK, REG_TSI_LOCK, AVSYNC_SYS_LOCK_SHIFT),
};

//M2M
static struct mt_mod_rst_simple m2m_core1 =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, M2MCORE1_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, M2M_CORE1_LOCK_SHIFT),
};
static struct mt_mod_rst_simple m2m_cipher =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, M2MCIPHER_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, M2M_CIPHER_LOCK_SHIFT),
};
static struct mt_mod_rst_simple m2m_ahb =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.ahb = MT_INIT_RST(REG_SECURE_SRSTN, M2MAHB_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, M2M_AHB_LOCK_SHIFT),
};
static struct mt_mod_rst_simple m2m_axi =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.axi = MT_INIT_RST(REG_SECURE_SRSTN, M2MAXI_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, M2M_AXI_LOCK_SHIFT),
};
static struct mt_mod_rst_simple m2m_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, M2MCORE_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, M2M_CORE_LOCK_SHIFT)
};

//DSC
static struct mt_mod_rst_simple ds_m2reg =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, DS_M2REG_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, DS_M2REG_LOCK_SHIFT),
};
static struct mt_mod_rst_simple ds_m2core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, DS_M2CORE_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, DS_M2CORE_LOCK_SHIFT),
};
static struct mt_mod_rst_simple ds_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, DS_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, DS_LOCK_SHIFT)
};

//KT
static struct mt_mod_rst_simple kt_ibus =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, KT_IBUS_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, KT_IBUS_LOCK_SHIFT),
};
static struct mt_mod_rst_simple kt_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, KTCORE_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, KT_CORE_LOCK_SHIFT)
};

//PKA
static struct mt_mod_rst_simple pka_ibus =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, PKA_IBUS_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, PKA_IBUS_LOCK_SHIFT),
};
static struct mt_mod_rst_simple pka_core =
{
	.name = NULL,
	INIT_DIG_RST_VAL,
	.core = MT_INIT_RST(REG_SECURE_SRSTN, PKACORE_SRSTN_SHIFT,
					REG_SECURE_SLOCK, REG_SECURE_LOCK, PKA_IBUS_LOCK_SHIFT)
};

//---------------------------------------------------------------------------//

/* Symphony6 Composite Reset Modules */

struct mt_mod_rst_composite mt_com_rst_modules[] =
{
	//MAC
	{
		.name = MT_CLK_MAC,
		.ahb = &mac_ahb,
		.axi = &mac_axi,
	},
	//AVCPU
	{
		.name = MT_CLK_AVCPU,
		.ahb = &avcpu_ahb,
		.axi = &avcpu_axi,
		.core = &avcpu_core,
	},
	//SPDMA
	{
		.name = MT_CLK_SPDMA,
		.ahb = &spdma_ahb,
		.axi = &spdma_axi,
		.core = &spdma_core,
	},
#ifndef CFG_RESET_OPT_SIZE
	//OIC
	{
		.name = MT_RST_OIC, 	/*Hide*/
		.apb = &oic_apb,
		.axi = &oic_axi,
		.core = &oic_core,
	},
	//OMC
	{
		.name = MT_RST_OMC, 	/*Hide*/
		.apb = &omc_apb,
		.axi = &omc_axi,
		.core = &omc_core,
		.phy = &omc_phy,
	},
#endif
	//DMA
	{
		.name = MT_CLK_DMA,
		.ahb = &dma_ahb,
		.axi = &dma_axi,
		.core = &dma_core,
	},
	//SDIO1
	{
		.name = MT_CLK_SDIO1,
		.ahb = &sdio1_ahb,
		.axi = &sdio1_axi,
		.core = &sdio1_core,
	},
	//SDIO0
	{
		.name = MT_CLK_SDIO0,
		.ahb = &sdio0_ahb,
		.axi = &sdio0_axi,
		.core = &sdio0_core,
	},
	//PNAND
	{
		.name = MT_CLK_PNAND,
		.ahb = &pnand_ahb,
		.axi = &pnand_axi,
		.core = &pnand_core,
		.reg = &pnand_reg,
	},
	//SPI0
	{
		.name = MT_CLK_SPI0,
		.ahb = &spi0_ahb,
		.core = &spi0_core,
		.reg = &spi0_reg,
	},
	//USB30
	{
		.name = MT_RST_USB30,
		.apb = &usb30_apb,
		.phy = &usb30_ss_phy,
	},
	//USB1
	{
		.name = MT_CLK_USB1,
		.ahb = &usb1_ahb,
		.axi = &usb1_axi,
		.core = &usb1_core,
		.phy = &usb1_phy,
	},
	//USB0
	{
		.name = MT_CLK_USB0,
		.ahb = &usb0_ahb,
		.axi = &usb0_axi,
		.core = &usb0_core,
		.phy = &usb0_phy,
	},
	//SMC
	{
		.name = MT_CLK_SMC,
		.core = &smc_core,
		.phy = &smc_phy,
	},
	//GRA
	{
		.name = MT_CLK_GRA,
		.ahb = &gra_ahb,
		.axi = &gra_axi,
		.core = &gra_core,
	},
	//JPG
	{
		.name = MT_CLK_JPG,
		.ahb = &jpg_ahb,
		.axi = &jpg_axi,
		.core = &jpg_core,
	},
	//DISP
	{
		.name = MT_CLK_DISP,
		.ahb = &disp_ahb,
		.axi = &disp_axi,
		.core = &disp_core,
		.ext1 = &disp_sdvenc,
		.ext2 = &disp_hdvenc,
	},
	//VDEC
	{
		.name = MT_CLK_VDEC,
		.ahb = &vdec_ahb,
		.axi = &vdec_axi,
		.core = &vdec_core,
		.ext1 = &vdeccore_core,
	},
	//AOUT
	{
		.name = MT_CLK_AOUT,
		.ahb = &aout_ahb,
		.axi = &aout_axi,
		.core = &aout_core,
		.ext1 = &aout_src,
	},
	//HDMI
	{
		.name = MT_CLK_HDMI,
		.ahb = &hdmi_ahb,
		.core = &hdmi_core,
	},
	//HDVENC
	{
		.name = MT_CLK_HDVENC,
		.ahb = &hdvenc_ahb,
		.core = &hdvenc_core,
	},
	//SDVENC
	{
		.name = MT_CLK_SDVENC,
		.ahb = &sdvenc_ahb,
		.core = &sdvenc_core,
	},
	//PNG
	{
		.name = MT_CLK_PNG,
		.ahb = &png_ahb,
		.axi = &png_axi,
		.core = &png_core,
	},
//#ifndef CFG_RESET_OPT_SIZE
	//DAI
	{
		.name = MT_CLK_DAI,
		.ahb = &dai_ahb,
		.axi = &dai_axi,
	},
//#endif
	//GPU
	{
		.name = MT_CLK_GPU,
		.ahb = &gpu_ahb,
		.axi = &gpu_axi,
		.core = &gpu_core,
		.ext1 = &gpu_mbist,
		.ext2 = &gpu_2psram,
	},
	//GMAC
	{
		.name = MT_CLK_GMAC,
		.ahb = &gmac_ahb,
		.axi = &gmac_axi,
		.core = &gmac_crm,		/* core for crm */
	},
	//TSI
	{
		.name = MT_CLK_TSI,
		.ahb = &tsi_ahb,
		.axi = &tsi_axi,
		.core = &tsi_core,
	},
	//SF
	{
		.name = MT_RST_TSI_SF,		/*Hide*/
		.axi = &sf_axi,
		.core = &sf_core,
		.reg = &sf_reg,
	},
	//TRPP
	{
		.name = MT_RST_TSI_TRPP,	/*Hide*/
		.axi = &trpp_axi,
		.core = &trpp_core,
		.reg = &trpp_reg,
	},
	//SWTSI
	{
		.name = MT_RST_TSI_SWTSI, /*Hide*/
		.axi = &swtsi_axi,
		.core = &swtsi_core,
	},
	//AVSYNC
	{
		.name = MT_RST_TSI_AVSYNC, /*Hide*/
		.ahb = &avsync_ahb,
		.ext1 = &avsync_pcr,
		.ext2 = &avsync_sys,
	},
	//M2M
	{
		.name = MT_CLK_M2M,
		.ahb = &m2m_ahb,
		.axi = &m2m_axi,
		.core = &m2m_core,
		.ext1 = &m2m_cipher,
		.ext2 = &m2m_core1,
	},
	//DSC
	{
		.name = MT_CLK_DSC,
		.core = &ds_core,
		.reg = &ds_m2reg,
		.ext2 = &ds_m2core,
	},
	//KT
	{
		.name = MT_CLK_KT,
		.core = &kt_core,
		.ibus = &kt_ibus,
	},
	//PKA
	{
		.name = MT_CLK_PKA,
		.core = &pka_core,
		.ibus = &pka_ibus,
	},
};

u32 mt_com_rst_modules_count = sizeof(mt_com_rst_modules) / sizeof(mt_com_rst_modules[0]);

#else

/* CONFIG_TARGET_SYMPHONY6_LITE || CONFIG_TARGET_SYMPHONY6_MINI */

struct mt_mod_rst_simple mt_rst_modules[] =
{
	/* TODO: */

	//VDAC
	{
		.name = MT_ANA_VDAC,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(VDAC_REG0, 15, 0, 0, 0)
	},

	//HDMI_TX
	{
		.name = MT_ANA_HDMITX,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(ANA_AO_REG0, HDMI_TX_RESET_SHIFT,
							REG_ANA_AO_SLOCK, REG_ANA_AO_LOCK, ANA_AO_REG0_LOCK_SHIFT)
	},

	//VSDSSC
	{
		.name = MT_ANA_VSDSSC,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_VSDSSC, 6,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VSDSSC_REG_LOCK_SHIFT)
	},

	//VHDPLL
	{
		.name = MT_VHDPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_VHDPLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VHDPLL_REG_LOCK_SHIFT)
	},
	//VSDPLL
	{
		.name = MT_VSDPLL,
		INIT_ANA_RST_VAL,
		.core = MT_INIT_RST(REG_CLKGEN_VSDPLL, 14,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VSDPLL_REG_LOCK_SHIFT)
	},
};

u32 mt_rst_modules_count = sizeof(mt_rst_modules) / sizeof(mt_rst_modules[0]);

struct mt_mod_rst_composite mt_com_rst_modules[] =
{
	/* TODO: */
};

u32 mt_com_rst_modules_count = sizeof(mt_com_rst_modules) / sizeof(mt_com_rst_modules[0]);

#endif

