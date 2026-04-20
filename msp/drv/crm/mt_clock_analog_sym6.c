/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2023, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_clock_analog_sym6.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2023/02/16
 * Description    : MT Symphony6 analog clocks definition.
 * History        :
 * 1.Date         : 2023/02/16
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

#include "mt_clock.h"
#include "mt_crm_lock_reg.h"

#ifdef __UBOOT__
#include "mt_analog_reg.h"
#elif defined(__KERNEL__)
#include "../analog/mt_analog_reg.h"
#else
/* RTOS */
#include "../analog/mt_analog_reg.h"
#endif

#if !defined(CONFIG_TARGET_SYMPHONY6_LITE) && !defined(CONFIG_TARGET_SYMPHONY6_MINI)

#define DRV0_MCLK					"drv0_mclk_ana"

//#define VSD_HD_CLK					"vsd_hd_clk"

//#define ETH_CLK						"eth_clk_ana"

/* cadc rate */
static struct mt_clk_map_table g_cadc_rate[] =
{
	{0, 144000000},
	{1, 270000000},
	{-1, -1}
};

/* sadc rate */
static struct mt_clk_map_table g_sadc_rate[] =
{
	{0, 960000000},
	{1, 1100000000},
	{-1, -1}
};

/* drv0_clk rate */
static struct mt_clk_map_table g_drv0_rate[] =
{
	{0, 0, DRV0_MCLK},
	{1, 81000000},
	{-1, -1}
};

/* drv0_mclk rate */
static struct mt_clk_map_table g_drv0_mclk_rate[] =
{
	{0, 57600000},
	{1, 28800000},
	{-1, -1}
};

/* drv1_clk rate */
static struct mt_clk_map_table g_drv1_rate[] =
{
	{0, 192000000},
	{1, 96000000},
	{-1, -1}
};

/* VENC OS rate */
static struct mt_clk_map_table g_vencos_rate[] =
{
	{0, 74250000},

	{1, 148500000},
	{2, 297000000},

	{3, 594000000},
	{4, 108000000},
	{6, 27000000},

	/* move to last one, not used */
#if 0
	{5, 108000000},
	{7, 27000000},
#endif

	{-1, -1}
};

/* hdmi tx rate */
static struct mt_clk_map_table g_hdmitx_rate[] =
{
	{1, 742500000},
	{2, 1485000000},
	{3, 270000000},			/* 270M from vsd */

	{0, 270000000U + 'H'},	/* 270M from vhd, hack '0' as 270M + 'H', avoid conflict with '3' */

	{-1, -1}
};

#if 0
/* VSD HD ANA rate */
static struct mt_clk_map_table g_vsd_hdana_rate[] =
{
	{0, 74250000},
	{1, 148500000},
	{2, 297000000},
	{3, 74250000},
	{4, 108000000},
	{5, 27000000},
	{6, 108000000},
	{7, 27000000},
	{-1, -1}
};

/* VSD rate */
static struct mt_clk_map_table g_vsd_rate[] =
{
	{0, 108000000},
	{1, 27000000},
	{-1, -1}
};
#endif

#if 0
/* ETH rate */
static struct mt_clk_map_table g_eth_rate[] =
{
	{0, 25000000},
	{1, 50000000},
	{-1, -1}
};
#endif

/* procmonitor rate */
static struct mt_clk_map_table g_pmon_rate[] =
{
	{0, 5000000},
	{1, 2500000},
	{-1, -1}
};

/*
 * audiopll = 480*2^12/FRAC_NUM/5
 *
 * e.g.
 *	 FRAC_NUM = 0xFA0
 *	 audiopll = 480 * 4096 / 4000 / 5 = 98.304
 */
static struct mt_clk_map_table g_audiopll_rate[] =
{
	{0xD04, 118012000},
	{0xFA0, 98304000},
	//TODO:
	{-1, -1}
};

/* analog clock table */
struct mt_clk_in mt_analog_clk_table[] =
{
	/* CADC CLK */
	{
		.name = MT_ANA_CADC,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_ADCPLL, 13, 1, g_cadc_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_CPUPLL_REG_LOCK_SHIFT)
	},
	/* SADC CLK */
	{
		.name = MT_ANA_SADC,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_ETHINTP, 4, 1, g_sadc_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_ETHINTP_REG_LOCK_SHIFT)
	},
	/* DRV0_CLK */
	{
		.name = DRV0_CLK_ANA,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_CLKGEN_ETHINTP, 1, 1, g_drv0_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_ETHINTP_REG_LOCK_SHIFT)
	},
	/* DRV0_MCLK */
	{
		.name = DRV0_MCLK,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_PDSYS, 26, 1, g_drv0_mclk_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},
	/* DRV1_CLK */
	{
		.name = DRV1_CLK_ANA,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_PDSYS, 27, 1, g_drv1_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_PDSYS_REG_LOCK_SHIFT)
	},

	/* VHD: VENC_OSCLK */
	{
		.name = MT_ANA_VENC_OS,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_VHDINTP, 1, 3, g_vencos_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VHDINTP_REG_LOCK_SHIFT)
	},
	/* VHD: HDMI_TX */
	{
		.name = MT_ANA_HDMITX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_VHDINTP, 4, 2, g_hdmitx_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VHDINTP_REG_LOCK_SHIFT)
	},
#if 0
	/* VSD: HD_ANA */
	{
		.name = VSD_HD_CLK,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_VSDINTP, 0, 3, g_vsd_hdana_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VSDINTP_REG_LOCK_SHIFT)
	},
	/* VSD */
	{
		 .name = VSD_CLK_ANA,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_VSDINTP, 3, 1, g_vsd_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VSDINTP_REG_LOCK_SHIFT)
	},
#endif

#if 0
	/* ETH_CLK */
	{
		.name = ETH_CLK,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_ETHINTP, 0, 1, g_eth_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_ETHINTP_REG_LOCK_SHIFT)
	},
#endif
	/* MT_ANA_PROCMON */
	{
		.name = MT_ANA_PROCMON,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(PM_SW_REG0, 11, 1, g_pmon_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, PM_SW_REG_LOCK_SHIFT)
	},

	/* MT_AUDIOPLL */
	{
		.name = MT_AUDIOPLL,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(CLKGEN_USBACLK_REG2, 0, 12, g_audiopll_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_USBACLK_REG_LOCK_SHIFT)
	},
};

#else

/* CONFIG_TARGET_SYMPHONY6_LITE || CONFIG_TARGET_SYMPHONY6_MINI */

/* VENC OS rate */
static struct mt_clk_map_table g_vencos_rate[] =
{
	{0, 74250000},

	{1, 148500000},
	{2, 297000000},

	{3, 594000000},
	{4, 108000000},
	{6, 27000000},

	/* move to last one, not used */
#if 0
	{5, 108000000},
	{7, 27000000},
#endif

	{-1, -1}
};

/* hdmi tx rate */
static struct mt_clk_map_table g_hdmitx_rate[] =
{
	{1, 742500000},
	{2, 1485000000},
	{3, 270000000},			/* 270M from vsd */

	{0, 270000000U + 'H'},	/* 270M from vhd, hack '0' as 270M + 'H', avoid conflict with '3' */

	{-1, -1}
};

struct mt_clk_in mt_analog_clk_table[] =
{
	/* VHD: VENC_OSCLK */
	{
		.name = MT_ANA_VENC_OS,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_VHDINTP, 1, 3, g_vencos_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VHDINTP_REG_LOCK_SHIFT)
	},
	/* VHD: HDMI_TX */
	{
		.name = MT_ANA_HDMITX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_CLKGEN_VHDINTP, 4, 2, g_hdmitx_rate,
							REG_ANA_SW_SLOCK, REG_ANA_SW_LOCK, CLKGEN_VHDINTP_REG_LOCK_SHIFT)
	},
};

#endif

/* analog clock table count */
u32 mt_analog_clk_table_size = sizeof(mt_analog_clk_table)/sizeof(mt_analog_clk_table[0]);

