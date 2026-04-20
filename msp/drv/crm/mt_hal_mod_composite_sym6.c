/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2025 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#if defined(__UBOOT__) || defined(__KERNEL__)
#include "mt_unf_misc.h"
#else
/* RTOS */
#include "hal_misc.h"
#endif
#include "mt_mod_internal.h"

#ifndef MKSTR
#define MKSTR(x)					#x
#endif

/* Sym6 Composite Modules */
static mt_comp_mod_t mt_comp_modules[] =
{
	{ .id = HAL_HDMI,
	  .name = MKSTR(HAL_HDMI),
	  .ana_com = { MT_ANA_HDMITX, NULL },
	  .dig_com = { MT_CLK_HDMI, MKSTR(TOP_HDMI_XCLK), MKSTR(TOP_HDMI_CEC_CLK), MKSTR(TOP_HDMI_OSC_CLK), MKSTR(TOP_HDMI_TMDS_CLK)
	  },
	  .rate = NULL,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_ANA_HDMITX, MT_CLK_HDMI, NULL },
#endif
	},

	{ .id = HAL_MAC,
	  .name = MKSTR(HAL_MAC),
	  .ana_com = { MT_ANA_EPHY, NULL },
	  .dig_com = { MT_CLK_MAC, MT_CLK_RMII, MKSTR(TOP_ETHERNET_CLK), NULL },
	  .rate = NULL,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_ANA_EPHY, MT_RST_EPHY, MT_CLK_MAC, NULL },
#endif
	},

	/* usb0 2.0 phy & clock */
	{ .id = HAL_USB0,
	  .name = MKSTR(HAL_USB0),
	  .ana_com = { MT_ANA_USB0, NULL },
	  .dig_com = { MT_CLK_USB0, NULL },
	  .rate = MT_CLK_USB0,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_ANA_USB0, MT_CLK_USB0, NULL },
#endif
	},

	/* usb1 2.0 phy & clock */
	{ .id = HAL_USB1_P20,
	  .name = MKSTR(HAL_USB1_P20),
	  .ana_com = { MT_ANA_USB1,NULL },
	  .dig_com = { MT_CLK_USB1, NULL },
	  .rate = NULL,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_ANA_USB1, MT_CLK_USB1, NULL },
#endif
	},

	/* usb1 3.0 phy, pll and clock */
	{ .id = HAL_USB1,
	  .name = MKSTR(HAL_USB1),
	  .ana_com = { MT_ANA_USB3_PHY, MT_USB30PLL, MT_ANA_USB1, NULL },
	  .dig_com = { MT_CLK_USB1, MKSTR(TOP_USB3_CLK), NULL },
	  .rate = NULL,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_ANA_USB1, MT_USB30PLL, MT_RST_USB30, MT_CLK_USB1, NULL },
#endif
	},

	{ .id = HAL_ADAC,
	  .name = MKSTR(HAL_ADAC),
	  .ana_com = { MT_ANA_ADAC, NULL },
	  .dig_com = { MT_CLK_ADAC, NULL },
	  .rate = NULL,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_ANA_ADAC, MT_CLK_ADAC, NULL },
#endif
	},

	{ .id = HAL_DAI,
	  .name = MKSTR(HAL_DAI),
	  .ana_com = { MT_AUDIOPLL, NULL },
	  .dig_com = { MT_CLK_DAI, MKSTR(TOP_DAI_RX_CLK), MKSTR(TOP_DAI_TX_CLK), NULL },
	  .rate = NULL,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_AUDIOPLL, MT_CLK_DAI, NULL },
#endif
	},

	{ .id = HAL_SMC,
	  .name = MKSTR(HAL_SMC),
	  .ana_com = { MT_ANA_KADC2, NULL },
	  .dig_com = { MT_CLK_SMC, MKSTR(TOP_SMC_90M_CLK), MKSTR(TOP_SMC_XTAL_D2_CLK), NULL },
	  .rate = MT_CLK_SMC,
#ifdef CFG_RESET_SUPPORT
	  .resets = { MT_ANA_KADC2, MT_CLK_SMC, NULL },
#endif
	},

};

static unsigned int mt_comp_modules_count = sizeof(mt_comp_modules) / sizeof(mt_comp_modules[0]);

