/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Montage LZ Co., Ltd.
 */
#if defined(__UBOOT__)
#include <common.h>
#include <command.h>

#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#include <asm/arch-symphony6/mt_analog_parameter.h>
#include <asm/arch-symphony6/mt_analog_app.h>

#define NO_DUMP_VXDSSC_STATE
#include <asm/arch-symphony6/mt_analog_vxdssc.h>

#include "mt_crm_clock_reg.h"

#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/timekeeping.h>

#include "mt_common.h"
#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "../crm/mt_crm_clock_reg.h"

#include "mt_drv_analog.h"
#include "analog/mt_analog_parameter.h"
#include "analog/mt_analog_app.h"

#define NO_DUMP_VXDSSC_STATE
#include "analog/mt_analog_vxdssc.h"

#include "mt_drv_clock.h"
#else
/* RTOS */
#include <sys_types.h>
#include <sys_define.h>

#include "mtos_misc.h"

#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "../crm/mt_crm_clock_reg.h"

#include "mt_drv_analog.h"
#include "analog/mt_analog_parameter.h"
#include "analog/mt_analog_app.h"

#define NO_DUMP_VXDSSC_STATE
#include "analog/mt_analog_vxdssc.h"

#include "mt_drv_clock.h"

typedef unsigned int 		mt_u32;

#ifndef ULONG_MAX
#define ULONG_MAX			0xFFFFFFFFFFFFFFFFUL
#endif

#endif

#include "mt_analog_reg.h"
#include "mt_log_internal.h"

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#if defined(__UBOOT__)
#define REG_HDMI_SSC				(0xBF4800E8UL)
#elif defined(__KERNEL__)
#define REG_HDMI_SSC				SYMPHONY_IO_VA(0xBF4800E8UL)
#endif
#define SSC_EN_SHIFT				29
#endif

#define DELAY_VID_CLK				1U
#define DELAY_RESET_SSC				10U
#define DELAY_RESET_PLL				2000U

#define DEFAULT_HDMI_ID				0

#if defined(__UBOOT__)
//FIXME?
//#define SPIN_LOCK()					disable_interrupts()
//#define SPIN_UNLOCK()				enable_interrupts()
#define SPIN_LOCK()					do{}while(0)
#define SPIN_UNLOCK()				do{}while(0)
#elif defined(__KERNEL__)
static DEFINE_SPINLOCK(ana_app_lock);

#if 0

/* mt_hdmi_analog_config called in ISR! */

#if 1
/* multi core */
#define SPIN_LOCK()					spin_lock(&ana_app_lock)
#define SPIN_UNLOCK()				spin_unlock(&ana_app_lock)
#else
/* single core */
#define SPIN_LOCK()					do{}while(0)
#define SPIN_UNLOCK()				do{}while(0)
#endif

#else
/* mt_hdmi_analog_config NOT called in ISR! */

static unsigned long flags;
#define SPIN_LOCK()					spin_lock_irqsave(&ana_app_lock, flags)
#define SPIN_UNLOCK()				spin_unlock_irqrestore(&ana_app_lock, flags)
#endif

#else
/* RTOS */
static u32 flags;
#define SPIN_LOCK()					do{mtos_critical_enter(&flags);}while(0)
#define SPIN_UNLOCK()				do{mtos_critical_exit(flags);}while(0)
#endif

#if defined(__UBOOT__) || defined(__KERNEL__)
#define APP_UDELAY					udelay
#else
/* RTOS */
#define APP_UDELAY					mtos_task_delay_us
#endif

/* global hdmi analog parameters */
struct hdmi_analog_params
{
	/* 1: vhd, 0: vsd */
	int vhd;

	/* video clk: vhd/vsd clk */
#if 0
	union
	{
		/* 0xBF5D009C clkgen_pdsys_reg {[8], [9], [10], [13:12]} */
		struct vhdclk_param vhd_clk;

		/* 0xBF5D009C clkgen_pdsys_reg {[16], [17], [18], [21:20]} */
		struct vsdclk_param vsd_clk;
	};
#else
	/* 0xBF5D009C clkgen_pdsys_reg {[8], [9], [10], [13:12]} */
	struct vhdclk_param vhd_clk;

	/* 0xBF5D009C clkgen_pdsys_reg {[16], [17], [18], [21:20]} */
	struct vsdclk_param vsd_clk;
#endif

	/* 1: ssc, 0: no ssc */
	int ssc_valid;

	/* vsd ssc */
	/* 0xBF5D0144 clkgen_vsdssc_reg {[2:0], [3], [4]} */
	struct vsdssc_param vsd_ssc;

	/* vhd ssc */
	/* 0xBF5D0140 clkgen_vhdssc_reg [31:0] */
	struct vhdssc_param vhd_ssc;

	/* hdmi clk, clkgen_vhdintp */
	/*
	 * 0xBF5D005C clkgen_vhdintp_reg [3:1]
	 *
	 * [3:1] clk os sel
	 *   000: 74.25
	 *   001: 148.5
	 *   010: 297
	 *   011: 594
	 *   10x: 108
	 *   11x: 27
	 */
	unsigned long venc_os_rate;

	/*
	 * TMDS Clk = clkgen_vhdintp_reg <5:4> "hdmitx_rate" + clkgen_usbaclk_reg0 <10:8> "hdmiclk_sel"
	 */

	/*
	 * 0xBF5D005C clkgen_vhdintp_reg [5:4]
	 *
	 * [5:4] clk hdmitx sel
	 *   00: 742.5M
	 *   01: 742.5M
	 *   10: 1485M
	 *   11: 270M
	 */
	unsigned long hdmitx_rate;

	/*
	 * hdmi clk select
	 *
	 * 0xBF5D0120 clkgen_usbaclk_reg0 {[8], [9], [10]}
	 */
	struct hdmiclk_param hdmiclk_sel;

	/*
	 * reset pll flag
	 * some pll setting not reset it, e.g. hdmi 148.5M*1.25
	 */
	int reset_pll;

	/* vhd/vsd pll */
#if 0
	union
	{
		/* 0xBF5D0058 clkgen_vhdpll_reg [31:0] */
		struct vhdpll_param vhd_pll;

		/* 0xBF5D0060 clkgen_vsdpll_reg [31:0] */
		struct vsdpll_param vsd_pll;
	};
#else
	/* 0xBF5D0058 clkgen_vhdpll_reg [31:0] */
	struct vhdpll_param vhd_pll;

	/* 0xBF5D0060 clkgen_vsdpll_reg [31:0] */
	struct vsdpll_param vsd_pll;
#endif

	/*
	 * pi:
	 * 1: pd pi, 0: include pi
	 */
	int pd_pi;

	/* vhd/vsd pll: include or pd pi */
	union
	{
		/* 0xBF5D0058 clkgen_vhdpll_reg [31:0] */
		struct vhdpll_param vhd_pll_pi;

		/* 0xBF5D0060 clkgen_vsdpll_reg [31:0] */
		struct vsdpll_param vsd_pll_pi;
	};

	/*
	 * hdmi PHY clk DCC
	 *
	 * 0xBF5D0120 clkgen_usbaclk_reg0 {[1], [3:2]}
	 */
	struct hdmidcc_param hdmi_dcc;
};

/* global hdmi analog post parameters */
struct hdmi_analog_post_params
{
	/* 0xBF5D01C8 hdmi_test_reg0 [31:0] */
	struct hdmiphy_tst_param test;

	/* 0xBF5D01BC hdmi_tx_reg0 [31:0] */
	struct hdmitx_ch_param ch0;

	/* 0xBF5D01C0 hdmi_tx_reg1 [31:0] */
	struct hdmitx_ch_param ch1;

	/* 0xBF5D01C4 hdmi_tx_reg2 [31:0] */
	struct hdmitx_ch_param ch2;
};

/* global hdmi digital parameters */
struct hdmi_digit_params
{
	/* VOUT_CLKSEL_REG: 0xBF50A604 */
	/*
	 * 0xx: 不分频
	 * 100: 2分频
	 * 101: 4分频
	 * 110: 6分频
	 * 111: 8分频
	 */
	u32 hdvenc_clksel;			/* bit 0-2 */
	u32 hdmi_tmds_clksel;		/* bit 12-14 */
	u32 hdmi_pixnx_clksel;		/* bit 16-18 */
};

/* include your version of hdmi parameters here: */
#include "mt_analog_app_hdmi_param_20240130.h"

/* current selected hdmi parameters */
static struct hdmi_analog_params *current_params = sym6_hdmi_params_nossc_inc_pi;

static mt_hdmi_ana_clk_cfg_t sym6_hdmi_conf = {0};

/* hdmi_tx_reg2 <31:28> */
#define MUTE_MASK			(0xF << 28)
#define UNMUTE_MASK			(0 << 28)

static struct hdmi_analog_post_params sym6_hdmi_post_params[] =
{
/*	 .test         .ch0          .ch1          .ch2 */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_27 - 27M */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{ {0x080000F0}, {0x013044CA}, {0x013044CA}, {0x013044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ {0x080000F0}, {0x013044CA}, {0x013044CA}, {0x013044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_594 - 594M */
#ifdef PATCH_HDMI_MICO
	{ {0x080000F8}, {0x0130442F}, {0x0130442F}, {0x0130442F | MUTE_MASK} },
#else
	{ {0x080000F8}, {0x013044CA}, {0x013044CA}, {0x013044CA | MUTE_MASK} },
#endif

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{ {0x080000F0}, {0x013044CA}, {0x013044CA}, {0x013044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ {0x080000F0}, {0x013044CA}, {0x013044CA}, {0x013044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_54 - 54M */
	{ {0x08000108}, {0x0200BB08}, {0x0200BB08}, {0x0200BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_108 - 108M */
	{ {/*FIXME*/0x080000F0}, {0x013044CA}, {0x013044CA}, {0x013044CA | MUTE_MASK} },
};

static struct hdmi_digit_params sym6_hdmi_dig_params[] =
{
/*   .hdvenc_clksel .hdmi_tmds_clksel .hdmi_pixnx_clksel */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{0,             0,                0},
	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{0,             0,                0},
	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{0,             0,                0},

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{0,             0,                0},
	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{0,             0,                0},
	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{0,             0,                0},

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{0,             0,                0},
	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{0,             0,                0},
	/* HDMI_ANALOG_CFG_27 - 27M */
	{0,             0,                0},

	/* HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{0,             0,                4/*2分频*/},
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{0,             0,                4/*2分频*/},
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{0, 			0,				  4/*2分频*/},

	/* HDMI_ANALOG_CFG_594 - 594M */
	{0, 			0,				  0},

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{0, 			0,				  0},
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{0, 			0,				  0},
	/* HDMI_ANALOG_CFG_297 - 297M */
	{0, 			0,				  0},

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{0, 			0,				  0},
	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{0, 			0,				  0},
	/* HDMI_ANALOG_CFG_54 - 54M */
	{0, 			0,				  0},
	/* HDMI_ANALOG_CFG_108 - 108M */
	{0, 			0,				  0},
};

//---------------------------------------------------------------------------//

static int get_gate_vclk_conf(void);

/* in mta_hdmi_priv.c */
void mta_hdmi_clk_cfg(mt_u32 clk, mt_u32 ssc_mode, mt_u32 pd_pi_mode, mt_u32 gate_vclk);
void mta_hdmi_clk_cfg_v2(mt_u32 tmds_clk, mt_u32 ssc_mode, mt_u32 pd_pi_mode, mt_u32 gate_vclk, mt_u32 os_clk);

#if defined (__UBOOT__)
#elif defined(__KERNEL__)
static unsigned long timer_get_us(void)
{
	return ktime_get_boottime_ns() / 1000;
}
#else
/* RTOS */
static unsigned long timer_get_us(void)
{
	return (unsigned long)mtos_hw_ticks_get();
}
#endif

/* dump hdmi analog parameters */
static void dump_param(struct hdmi_analog_params *param)
{
	int is_vhd;

	if (param == NULL)
		return;

	is_vhd = param->vhd;

	DUMP_LOG("=====Dump HDMI Analog Parameters=====\n");
	DUMP_LOG("                 vhd: %d\n", param->vhd);

	//if (is_vhd)
	{
		DUMP_LOG("     vhd_clk.div_sel: %u\n", param->vhd_clk.div_sel);
		DUMP_LOG("   vhd_clk.clk_sel_1: %u\n", param->vhd_clk.clk_sel_1);
		DUMP_LOG("    vhd_clk.drv0_sel: %u\n", param->vhd_clk.drv0_sel);
		DUMP_LOG("   vhd_clk.clk_sel_2: %u\n", param->vhd_clk.clk_sel_2);
	}
	//else
	{
		DUMP_LOG("     vsd_clk.div_sel: %u\n", param->vsd_clk.div_sel);
		DUMP_LOG("   vsd_clk.clk_sel_1: %u\n", param->vsd_clk.clk_sel_1);
		DUMP_LOG("    vsd_clk.drv0_sel: %u\n", param->vsd_clk.drv0_sel);
		DUMP_LOG("   vsd_clk.clk_sel_2: %u\n", param->vsd_clk.clk_sel_2);
	}

	DUMP_LOG("           ssc_valid: %u\n", param->ssc_valid);
	DUMP_LOG("     vsd ssc.clk_sel: %u\n", param->vsd_ssc.clk_sel);
	DUMP_LOG("      vsd ssc.en_ssc: %u\n", param->vsd_ssc.en_ssc);
	DUMP_LOG("      vsd ssc.pd_ssc: %u\n", param->vsd_ssc.pd_ssc);

	DUMP_LOG("             vhd ssc: 0x%08X\n", param->vhd_ssc.all);

	DUMP_LOG("        venc os rate: %lu\n", param->venc_os_rate);
	DUMP_LOG("        hdmi tx rate: %lu\n", param->hdmitx_rate);

	DUMP_LOG("   hdmi clk tmds sel: %u\n", param->hdmiclk_sel.hdmi_clk_tmds_sel);
	DUMP_LOG("            sel 540M: %u\n", param->hdmiclk_sel.sel540M);
	DUMP_LOG("           sel 2970M: %u\n", param->hdmiclk_sel.sel2970M);

	DUMP_LOG("           reset pll: %d\n", param->reset_pll);
	if (is_vhd)
	{
		DUMP_LOG("             vhd_pll: 0x%08X\n", param->vhd_pll.all);
		DUMP_LOG("             vsd_pll: 0x%08X\n", param->vsd_pll.all);
	}
	else
	{
		DUMP_LOG("             vsd_pll: 0x%08X\n", param->vsd_pll.all);
		DUMP_LOG("             vhd_pll: 0x%08X\n", param->vhd_pll.all);
	}

	DUMP_LOG("               pd_pi: %u\n", param->pd_pi);
	if (!param->pd_pi)
	{
		if (is_vhd)
		{
			DUMP_LOG("          vhd_pll_pi: 0x%08X\n", param->vhd_pll_pi.all);
		}
		else
		{
			DUMP_LOG("          vsd_pll_pi: 0x%08X\n", param->vsd_pll_pi.all);
		}
	}

	DUMP_LOG("             dcc xpd: %u\n", param->hdmi_dcc.dcc_xpd);
	DUMP_LOG("             dcc sel: %u\n", param->hdmi_dcc.dcc_sel);

	DUMP_LOG("       delay_vid_clk: %u\n", DELAY_VID_CLK);
	DUMP_LOG("     delay_reset_ssc: %u\n", DELAY_RESET_SSC);
	DUMP_LOG("     delay_reset_pll: %u\n", DELAY_RESET_PLL);

	DUMP_LOG("-------------------------------------\n");
}

/* phy mute/unmute */
static void _hdmiphy_mute(int mute)
{
	struct hdmiphy_param phy_mute;

	//MT_LOGI("HDMI PHY: mute = %d\r\n", mute);

	(void)mt_analog_get_parameter(ANALOG_PARAM_INDEX_HDMIPHY, &phy_mute);
	phy_mute.mute = (u8)mute;
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMIPHY, &phy_mute);
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
static void enable_digital_ssc(void)
{
	//FIXME: Sym4
	MT_LOGW("[Warning]Symphony4 support enable Digital SSC, Symphony6 NOT support!\n");

	dump_reg(REG_HDMI_SSC);
	MT_SET_BIT(REG_HDMI_SSC, SSC_EN_SHIFT, 1);
	dump_reg(REG_HDMI_SSC);

	CHECK_REGISTER(REG_HDMI_SSC, SSC_EN_SHIFT, 1, 0x1);
}
#endif

static void _set_pi_state(int pd_pi)
{
/* clkgen_vhdintp_reg/clkgen_vsdintp_reg <14:12> */
#define I_SEL_PD_PI				0
#define I_SEL_INC_PI			0x3	/* 011 */

/* clkgen_vhdintp_reg/clkgen_vsdintp_reg <15> */
#define DCO_CTRL_VALID			1

#define DCO_CTRL_VALID_DELAY	2	/* delay 2us */

	TRACE_LOG("set pi state: pd_pi = %d\n", pd_pi);

	dump_reg(REG_CLKGEN_VSDINTP);
	dump_reg(REG_CLKGEN_VHDINTP);

	if (pd_pi)
	{
		MT_ANALOG_UP_ATTR(MT_ANA_INDEX_VSDINTP, struct mt_analog_vsdintp_attr, i_sel, I_SEL_PD_PI);
		MT_ANALOG_UP_ATTR(MT_ANA_INDEX_VHDINTP, struct mt_analog_vhdintp_attr, i_sel, I_SEL_PD_PI);
	}
	else
	{
		MT_ANALOG_UP_ATTR(MT_ANA_INDEX_VSDINTP, struct mt_analog_vsdintp_attr, i_sel, I_SEL_INC_PI);
		MT_ANALOG_UP_ATTR(MT_ANA_INDEX_VHDINTP, struct mt_analog_vhdintp_attr, i_sel, I_SEL_INC_PI);
	}

	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_VSDINTP, struct mt_analog_vsdintp_attr, dco_ctrl_valid, DCO_CTRL_VALID);
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_VHDINTP, struct mt_analog_vhdintp_attr, dco_ctrl_valid, DCO_CTRL_VALID);

	dump_reg(REG_CLKGEN_VSDINTP);
	dump_reg(REG_CLKGEN_VHDINTP);

	APP_UDELAY(DCO_CTRL_VALID_DELAY);
}

/**
 * HDMI digital clock configuration
 */
static void do_hdmi_digit_config(struct hdmi_digit_params *param)
{
	if (param == NULL)
		return;

	TRACE_LOG("HDMI Digital Config Begin...\n");

	DUMP_LOG("    hdvenc_clksel: %u\n", param->hdvenc_clksel);
	DUMP_LOG(" hdmi_tmds_clksel: %u\n", param->hdmi_tmds_clksel);
	DUMP_LOG("hdmi_pixnx_clksel: %u\n", param->hdmi_pixnx_clksel);

	dump_reg(REG_VOUT_CLKSEL);

	(void)mt_clk_set_mux(MT_CLK_HDVENC, param->hdvenc_clksel);
	(void)mt_clk_set_mux(HDMI_TMDS_CLK, param->hdmi_tmds_clksel);
	(void)mt_clk_set_mux(HDMI_PIXNX_CLK, param->hdmi_pixnx_clksel);

	dump_reg(REG_VOUT_CLKSEL);

	TRACE_LOG("HDMI Digital Config End.\n");
}

/**
 * <CNComment>
 *
 *  1.HDMI/Analog/Display/VOUT配置的时机问题
 *    (1)是否需要在Display/HDMI的中断里进行配置?
 *    (2)是否需要关中断?
 *    (3)如何保证修改配置时,输出的完整性?(避免出现画面撕裂等问题)
 *
 *  2.配置的效果问题
 *    (1)输出信号稳定需要多久?
 *    (2)TV响应需要多久?(避免输出还未稳定的情况下,又进行下一次的配置)
 */
static int do_hdmi_analog_config(struct hdmi_analog_params *param)
{
#if 0
	int step = 1;
#endif
	int is_vhd;
	unsigned long start_time, end_time;

	if (param == NULL)
		return (-1);

	/* check */
	if (param->venc_os_rate == 0 || param->hdmitx_rate == 0)
	{
		MT_LOGE("#####################################################################################################");
		MT_LOGE("#    Error: %s, param invalid! Pls check your TMDS, OS, SSC, PI parameters!!!    #\r\n", __FUNCTION__);
		MT_LOGE("#####################################################################################################");
		return (-1);
	}

	is_vhd = param->vhd;

	start_time = timer_get_us();
	TRACE_LOG("[%lu]HDMI Analog Config Begin...\n", start_time);

	dump_param(param);

	/* step 1: PHY mute */
	TRACE_LOG("[%d]-PHY mute\n", step++);
	_hdmiphy_mute(1);

	/* step 2: reset phy tmds clk */
	TRACE_LOG("[%d]-reset phy tmds clk\n", step++);
	(void)mt_analog_reset(MT_ANA_HDMITX);
	dump_reg(ANA_AO_REG0);
	CHECK_REGISTER(ANA_AO_REG0, HDMI_TX_RESET_SHIFT, 1, 1);

#ifdef CONFIG_MT_CHIP_SYMPHONY4
	/* step 3: digital ssc enable */
	TRACE_LOG("[%d]-digital ssc enable\n", step++);
	enable_digital_ssc();
#endif

	/* step 3: gate video clk? */
	if (get_gate_vclk_conf())
	{
		TRACE_LOG("[%d]-gate video clk(vhd/vsd)\n", step++);
		dump_reg(REG_CLKGEN_PDSYS);
		(void)mt_analog_disable(VSD_CLK_ANA);
		(void)mt_analog_disable(VHD_CLK_ANA);
		dump_reg(REG_CLKGEN_PDSYS);
	}
	else
	{
		TRACE_LOG("[%d]-no gate video clk(vhd/vsd)\n", step++);
	}

	/* step 4: set video clk */
	TRACE_LOG("[%d]-set video clk(vhd/vsd)\n", step++);
	if (is_vhd) {
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VHDCLK, &param->vhd_clk);
		/* Patch: Fix Bug #26071 */
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VSDCLK, &param->vsd_clk);
	} else {
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VSDCLK, &param->vsd_clk);
		/* Patch: Fix Bug #26071 */
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VHDCLK, &param->vhd_clk);
	}

	/* wait */
	if (DELAY_VID_CLK) {
		TRACE_LOG("delay %u(us)\n", DELAY_VID_CLK);
		APP_UDELAY(DELAY_VID_CLK);
	}

	/* ssc */
	if (param->ssc_valid)
	{
		/* step 5: reset ssc */
		TRACE_LOG("[%d]-reset ssc\n", step++);
		(void)mt_analog_reset(MT_ANA_VSDSSC);
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VSDSSC, &param->vsd_ssc);

		/* wait? */
		if (DELAY_RESET_SSC) {
			TRACE_LOG("delay %u(us)\n", DELAY_RESET_SSC);
			APP_UDELAY(DELAY_RESET_SSC);
		}

		/* step 5: release ssc */
		TRACE_LOG("[%d]-release ssc\n", step);
		(void)mt_analog_release(MT_ANA_VSDSSC);
		dump_reg(REG_CLKGEN_VSDSSC);
	}
	else
	{
		/* no ssc */
		TRACE_LOG("[%d]-no ssc\n", step++);
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VSDSSC, &param->vsd_ssc);
	}

	/* step 6: tmds clk sel */
	TRACE_LOG("[%d]-tmds, os clk sel\n", step++);
	dump_reg(REG_CLKGEN_VHDINTP);
	TRACE_LOG("vencos rate: %lu, hdmitx rate: %lu\n", param->venc_os_rate, param->hdmitx_rate);
	(void)mt_analog_set_rate(MT_ANA_HDMITX, param->hdmitx_rate);
	(void)mt_analog_set_rate(MT_ANA_VENC_OS, param->venc_os_rate);
	dump_reg(REG_CLKGEN_VHDINTP);

	/* step 7: select hdmi clk */
	TRACE_LOG("[%d]-tmds clk sel x2\n", step++);
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMICLK, &param->hdmiclk_sel);

	/* step 8: reset pll */
	TRACE_LOG("[%d]-set and reset pll %d\n", step++, param->reset_pll);
	if (is_vhd) {
		/*
		 * FIXME:
		 *   reset or set parameter first?
		 * Caution:
		 *   set_parameter override the register if call reset first!
		 */
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_RAW_VHDPLL, &param->vhd_pll);
		if (param->reset_pll)
			(void)mt_analog_reset(MT_VHDPLL);

		/* Patch: Fix Bug #26071 */
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_RAW_VSDPLL, &param->vsd_pll);

		dump_reg(REG_CLKGEN_VHDPLL);
		dump_reg(REG_CLKGEN_VSDPLL);
	} else {
		/*
		 * FIXME:
		 *   reset or set parameter first?
		 * Caution:
		 *   set_parameter override the register if call reset first!
		 */
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_RAW_VSDPLL, &param->vsd_pll);
		if (param->reset_pll)
			(void)mt_analog_reset(MT_VSDPLL);

		/* Patch: Fix Bug #26071 */
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_RAW_VHDPLL, &param->vhd_pll);

		dump_reg(REG_CLKGEN_VSDPLL);
		dump_reg(REG_CLKGEN_VHDPLL);
	}

	/* wait */
	if (param->reset_pll) {
		if (DELAY_RESET_PLL) {
			TRACE_LOG("hold %u(us)\n", DELAY_RESET_PLL);
			APP_UDELAY(DELAY_RESET_PLL);
		}

		/* step 8: release pll */
		TRACE_LOG("[%d]-release pll\n", step);
		if (is_vhd) {
			(void)mt_analog_release(MT_VHDPLL);
			dump_reg(REG_CLKGEN_VHDPLL);
		} else {
			(void)mt_analog_release(MT_VSDPLL);
			dump_reg(REG_CLKGEN_VSDPLL);
		}
	}

	/* hold 2ms */
	if (DELAY_RESET_PLL) {
		TRACE_LOG("delay %u(us)\n", DELAY_RESET_PLL);
		APP_UDELAY(DELAY_RESET_PLL);
	}

	/* include pi */
	if (!param->pd_pi)
	{
		/* step 8: include pi in pll */
		TRACE_LOG("[%d]-include pi in pll\n", step);
		if (is_vhd) {
			(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_RAW_VHDPLL, &param->vhd_pll_pi);
		} else {
			(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_RAW_VSDPLL, &param->vsd_pll_pi);
		}
	}
	else
	{
		TRACE_LOG("[%d]-pd pi\n", step);
	}

	_set_pi_state(param->pd_pi);

	/* ssc */
	if (param->ssc_valid)
	{
		/* step 9: config ssc */
		TRACE_LOG("[%d]-config vhd ssc\n", step++);
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VHDSSC, &param->vhd_ssc);
	}
	else
	{
		TRACE_LOG("[%d]-config default vhd ssc\n", step++);
		(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_VHDSSC, &param->vhd_ssc);
	}

	/* step 10: turn on video clk */
	TRACE_LOG("[%d]-turn on video(%s) clk\n", step++, is_vhd?"VHD":"VSD");
	//if (get_gate_vclk_conf())
	{
		dump_reg(REG_CLKGEN_PDSYS);
		/* turn on both clocks */
		(void)mt_analog_enable(VHD_CLK_ANA);
		(void)mt_analog_enable(VSD_CLK_ANA);
		dump_reg(REG_CLKGEN_PDSYS);
	}

	/* step 11: hdmi PHY clk DCC */
	TRACE_LOG("[%d]-hdmi PHY clk DCC\n", step++);
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMIDCC, &param->hdmi_dcc);

	/* step 12: release phy tmds clk */
	TRACE_LOG("[%d]-release phy tmds clk\n", step++);
	(void)mt_analog_release(MT_ANA_HDMITX);
	dump_reg(ANA_AO_REG0);
	CHECK_REGISTER(ANA_AO_REG0, HDMI_TX_RESET_SHIFT, 1, 0);

	/* config digital:  do_hdmi_digit_config */

	/* step 13: do_hdmi_analog_post_config, set clk, D0-D2 driver, PHY unmute */

	end_time = timer_get_us();
	TRACE_LOG("[%lu]HDMI Analog Config End.\n", end_time);
	//FIXME: end_time < start_time?
	if (end_time >= start_time)
	{
		TRACE_LOG("[%lu]time used(us)\n", end_time - start_time);
	}
	else
	{
		TRACE_LOG("[%lu]time used(us)\n", ULONG_MAX - start_time + end_time);
	}

	return 0;
}

static int do_hdmi_analog_post_config(struct hdmi_analog_post_params *param)
{
	if (param == NULL)
		return (-1);

	TRACE_LOG("HDMI Analog Post Config Begin...\n");

	/* step 13: set channel clk driver */
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMIPHY_TST, &param->test);

	/* step 13: set D0, D1, D2 driver */
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMITX_CH0, &param->ch0);
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMITX_CH1, &param->ch1);
	(void)mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMITX_CH2, &param->ch2);

	TRACE_LOG("HDMI Analog Post Config End.\n");

	return 0;
}

//---------------------------------------------------------------------------//

/* Debug */
#if 0
static const char *get_cfg_str(enum MT_HDMI_ANALOG_CFG_E cfg_idx)
{
	static const char *str_cfg[] = {
							"148.5M x 1.5",
							"148.5M x 1.25",
							"148.5M",
							"74.25M x 1.5",
							"74.25M x 1.25",
							"74.25M",
							"27M x 1.5",
							"27M x 1.25",
							"27M",
							"297M x 1.5, clk os 594M",
							"297M x 1.25, clk os 594M",
							"297M, clk os 594M",
							"594M",
							"297M x 1.5",
							"297M x 1.25",
							"297M",
							"54M x 1.5",
							"54M x 1.25",
							"54M",
							"108M",
							};

	if (cfg_idx < HDMI_ANALOG_CFG_148P5_X_1P5 || cfg_idx >= HDMI_ANALOG_CFG_MAX)
		return NULL;

	return str_cfg[cfg_idx];
}
#endif

int mt_hdmi_analog_config(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx)
{
	int ret;

	if (cfg_idx < HDMI_ANALOG_CFG_148P5_X_1P5 || cfg_idx >= HDMI_ANALOG_CFG_MAX)
	{
		MT_LOGE("error: %s, invalid config(%d)!\r\n", __FUNCTION__, cfg_idx);
		return (-22);
	}

	if (current_params == NULL)
	{
		MT_LOGE("error: %s, current param is null!\r\n", __FUNCTION__);
		return (-22);
	}

	MT_LOGI("%s: id %u, idx %d(%s)\n", __FUNCTION__, id, cfg_idx, get_cfg_str(cfg_idx));

	/* mt_hdmi_analog_config called in ISR! */
	SPIN_LOCK();

	ret = do_hdmi_analog_config(&current_params[cfg_idx]);

	SPIN_UNLOCK();

	return ret;
}

int mt_hdmi_analog_config_post(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx)
{
	int ret;

	if (cfg_idx < HDMI_ANALOG_CFG_148P5_X_1P5 || cfg_idx >= HDMI_ANALOG_CFG_MAX)
	{
		MT_LOGE("error: %s, invalid config(%d)!\r\n", __FUNCTION__, cfg_idx);
		return (-22);
	}

	MT_LOGI("%s: id %u, idx %d(%s)\n", __FUNCTION__, id, cfg_idx, get_cfg_str(cfg_idx));

	/* mt_hdmi_analog_config_post called in ISR! */
	SPIN_LOCK();

	ret = do_hdmi_analog_post_config(&sym6_hdmi_post_params[cfg_idx]);

	SPIN_UNLOCK();

	return ret;
}

int mt_hdmi_digit_config_prefix(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx)
{
	int ret = 0;

	if (cfg_idx < HDMI_ANALOG_CFG_148P5_X_1P5 || cfg_idx >= HDMI_ANALOG_CFG_MAX)
	{
		MT_LOGE("error: %s, invalid config(%d)!\r\n", __FUNCTION__, cfg_idx);
		return (-22);
	}

	/* mt_hdmi_digit_config_prefix called in ISR! */
	SPIN_LOCK();

	/* step 1: Switch to Xtal */
	TRACE_LOG("Switch Xtal\n");
	mt_clk_set_mux(MT_CLK_HDMI, 1);

	/* Fix Bug #26239 */
	if (get_gate_vclk_conf())
		mt_clk_set_mux(MT_CLK_SDVENC, 1);

	SPIN_UNLOCK();

	return ret;
}

int mt_hdmi_digit_config_post(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx)
{
	int ret = 0;

	if (cfg_idx < HDMI_ANALOG_CFG_148P5_X_1P5 || cfg_idx >= HDMI_ANALOG_CFG_MAX)
	{
		MT_LOGE("error: %s, invalid config(%d)!\r\n", __FUNCTION__, cfg_idx);
		return (-22);
	}

	/* mt_hdmi_digit_config_post called in ISR! */
	SPIN_LOCK();

	do_hdmi_digit_config(&sym6_hdmi_dig_params[cfg_idx]);

	/* step 20: Switch back from Xtal */
	TRACE_LOG("Switch back from Xtal\n");
	/* Fix Bug #26239 */
	if (get_gate_vclk_conf())
		mt_clk_set_mux(MT_CLK_SDVENC, 0);

	mt_clk_set_mux(MT_CLK_HDMI, 0);

	SPIN_UNLOCK();

	return ret;
}

#define TMDS_RATE_148P5_X_1P5			222750000
#define TMDS_RATE_148P5_X_1P25			185625000
#define TMDS_RATE_148P5					148500000

#define TMDS_RATE_74P25_X_1P5			111375000
#define TMDS_RATE_74P25_X_1P25			92812500
#define TMDS_RATE_74P25					74250000

#define TMDS_RATE_27_X_1P5				40500000
#define TMDS_RATE_27_X_1P25				33750000
#define TMDS_RATE_27					27000000

#define TMDS_RATE_297_X_1P5				445500000
#define TMDS_RATE_297_X_1P25			371250000
#define TMDS_RATE_297					297000000

#define TMDS_RATE_594					594000000

#define TMDS_RATE_54_X_1P5				81000000
#define TMDS_RATE_54_X_1P25				67500000
#define TMDS_RATE_54					54000000

#define TMDS_RATE_108					108000000

/* venc os clock rate 594M */
#define VENCOS_CLK_RATE_594				594000000

enum MT_HDMI_ANALOG_CFG_E TMDS_TO_IDX(u32 tmds_clk, u32 os_clk)
{
	enum MT_HDMI_ANALOG_CFG_E cfg_idx = HDMI_ANALOG_CFG_MAX;

	switch (tmds_clk)
	{
		case TMDS_RATE_148P5_X_1P5:
			cfg_idx = HDMI_ANALOG_CFG_148P5_X_1P5;
			break;
		case TMDS_RATE_148P5_X_1P25:
			cfg_idx = HDMI_ANALOG_CFG_148P5_X_1P25;
			break;
		case TMDS_RATE_148P5:
			cfg_idx = HDMI_ANALOG_CFG_148P5;
			break;
		case TMDS_RATE_74P25_X_1P5:
			cfg_idx = HDMI_ANALOG_CFG_74P25_X_1P5;
			break;
		case TMDS_RATE_74P25_X_1P25:
			cfg_idx = HDMI_ANALOG_CFG_74P25_X_1P25;
			break;
		case TMDS_RATE_74P25:
			cfg_idx = HDMI_ANALOG_CFG_74P25;
			break;
		case TMDS_RATE_27_X_1P5:
			cfg_idx = HDMI_ANALOG_CFG_27_X_1P5;
			break;
		case TMDS_RATE_27_X_1P25:
			cfg_idx = HDMI_ANALOG_CFG_27_X_1P25;
			break;
		case TMDS_RATE_27:
			cfg_idx = HDMI_ANALOG_CFG_27;
			break;

		case TMDS_RATE_297_X_1P5:
			if (os_clk == VENCOS_CLK_RATE_594)
				cfg_idx = HDMI_ANALOG_CFG_297_X_1P5_594;
			else
				cfg_idx = HDMI_ANALOG_CFG_297_X_1P5;
			break;
		case TMDS_RATE_297_X_1P25:
			if (os_clk == VENCOS_CLK_RATE_594)
				cfg_idx = HDMI_ANALOG_CFG_297_X_1P25_594;
			else
				cfg_idx = HDMI_ANALOG_CFG_297_X_1P25;
			break;
		case TMDS_RATE_297:
			if (os_clk == VENCOS_CLK_RATE_594)
				cfg_idx = HDMI_ANALOG_CFG_297_594;
			else
				cfg_idx = HDMI_ANALOG_CFG_297;
			break;

		case TMDS_RATE_594:
			cfg_idx = HDMI_ANALOG_CFG_594;
			break;

		case TMDS_RATE_54_X_1P5:
			cfg_idx = HDMI_ANALOG_CFG_54_X_1P5;
			break;
		case TMDS_RATE_54_X_1P25:
			cfg_idx = HDMI_ANALOG_CFG_54_X_1P25;
			break;
		case TMDS_RATE_54:
			cfg_idx = HDMI_ANALOG_CFG_54;
			break;

		case TMDS_RATE_108:
			cfg_idx = HDMI_ANALOG_CFG_108;
			break;
		default:
			MT_LOGE("%s: invalid clk %u!\n", __FUNCTION__, tmds_clk);
 			break;
	}

	return cfg_idx;
}

/*
 * Caution: do NOT support multi thread!
 * <CNComment>不支持多线程调用!
 */
static void select_current_params(mt_u32 ssc_mode, mt_u32 pd_pi_mode)
{
	SPIN_LOCK();

	if (ssc_mode == 0)
	{
		if (pd_pi_mode == 0)
		{
			current_params = sym6_hdmi_params_nossc_inc_pi;
		}
		else
		{
			current_params = sym6_hdmi_params_nossc_pd_pi;
		}
	}
	else
	{
#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)
		if (pd_pi_mode == 0)
		{
			current_params = sym6_hdmi_params_ssc_inc_pi;
		}
		else
		{
			current_params = sym6_hdmi_params_ssc_pd_pi;
		}
#endif
	}

	/* hack: */
	//current_params = sym6_hdmi_params_nossc_inc_pi;
	//current_params = sym6_hdmi_params_nossc_pd_pi;
	//current_params = sym6_hdmi_params_ssc_inc_pi;
	//current_params = sym6_hdmi_params_ssc_pd_pi;

	SPIN_UNLOCK();
}

static void update_conf(mt_hdmi_ana_clk_cfg_t *cfg)
{
	SPIN_LOCK();
	memcpy(&sym6_hdmi_conf, cfg, sizeof(mt_hdmi_ana_clk_cfg_t));
	SPIN_UNLOCK();
}

static int get_gate_vclk_conf(void)
{
	return sym6_hdmi_conf.gate_vclk;
}

/*
 * Porting HDMI private function
 * in mta_hdmi_priv.c
 *
 * @param[in] cfg HDMI analog and clock configuration
 *
 */
void mta_hdmi_clk_cfg_v3(mt_hdmi_ana_clk_cfg_t *cfg)
{
	enum MT_HDMI_ANALOG_CFG_E cfg_idx = HDMI_ANALOG_CFG_MAX;
	int ret;

	MT_LOGI("%s: tmds clk %u, os_clk %u, ssc (%d, %x) pd_pi %d, gate clk %d\r\n", __FUNCTION__,
		cfg->tmds_clk, cfg->os_clk, cfg->ssc_onoff, cfg->ssc_param, cfg->pd_pi, cfg->gate_vclk);

   /*
	*							   include pi	 pd pi
	* clkgen_vsdintp_reg<15:12>    1011 		 1000
	* clkgen_vhdintp_reg<15:12>    1011 		 1000
    */
	//dump_reg(REG_CLKGEN_VSDINTP);
	//dump_reg(REG_CLKGEN_VHDINTP);

	cfg_idx = TMDS_TO_IDX(cfg->tmds_clk, cfg->os_clk);

	select_current_params(cfg->ssc_onoff, cfg->pd_pi);

	update_conf(cfg);

	ret = mt_hdmi_digit_config_prefix(DEFAULT_HDMI_ID, cfg_idx);
	if (ret != 0)
	{
		MT_LOGE("%s: pre digital config failed!\r\n", __FUNCTION__);
		goto Fail;
	}

	ret = mt_hdmi_analog_config(DEFAULT_HDMI_ID, cfg_idx);
	if (ret != 0)
	{
		MT_LOGE("%s: analog config failed!\r\n", __FUNCTION__);
		goto Fail;
	}

	ret = mt_hdmi_digit_config_post(DEFAULT_HDMI_ID, cfg_idx);
	if (ret != 0)
	{
		MT_LOGE("%s: post digital config failed!\r\n", __FUNCTION__);
		goto Fail;
	}

	ret = mt_hdmi_analog_config_post(DEFAULT_HDMI_ID, cfg_idx);
	if (ret != 0)
	{
		MT_LOGE("%s: post analog config failed!\r\n", __FUNCTION__);
		goto Fail;
	}

	/* for linux kernel, unmute is called by HDMI driver, not here! */
#ifdef __UBOOT__
	_hdmiphy_mute(0);
#endif

Fail:
	return;
}

/*
 * Porting HDMI private function
 * in mta_hdmi_priv.c
 *
 * @param[in] tmds_clk HDMI TMDS clock rate
 * @param[in] os_clk VENC OS clock rate
 *
 */
void mta_hdmi_clk_cfg_v2(mt_u32 tmds_clk, mt_u32 ssc_mode, mt_u32 pd_pi_mode, mt_u32 gate_vclk, mt_u32 os_clk)
{
	mt_hdmi_ana_clk_cfg_t cfg = {0};

	cfg.tmds_clk 	= (u32)tmds_clk;
	cfg.os_clk 		= (u32)os_clk;
	cfg.ssc_onoff 	= (int)ssc_mode;
	/* FIXME */
	cfg.ssc_param 	= (u32)0;
	cfg.pd_pi 		= (int)pd_pi_mode;
	cfg.gate_vclk 	= (int)gate_vclk;

	mta_hdmi_clk_cfg_v3(&cfg);
}

/*
 * Caution: mta_hdmi_clk_cfg() do NOT support TMDS 297 + OS CLK 594 cases!
 */
/*
 * Porting HDMI private function
 * in mta_hdmi_priv.c
 *
 * @param[in] clk HDMI TMDS clock rate
 *
 */
void mta_hdmi_clk_cfg(mt_u32 clk, mt_u32 ssc_mode, mt_u32 pd_pi_mode, mt_u32 gate_vclk)
{
	mta_hdmi_clk_cfg_v2(clk,
			ssc_mode,
			pd_pi_mode,
			gate_vclk,
			clk	/* FIXME: should use OS clk */
			);
}

#if defined(__UBOOT__)
#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)
static int do_cmd_hdmi_analog(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	int id = DEFAULT_HDMI_ID;
	int ssc = 0;
	int pdpi = 0;
	int index = 0;

	if (argc < 5)
		return CMD_RET_USAGE;

	id = simple_strtoul(argv[1], NULL, 0);
	ssc = simple_strtoul(argv[2], NULL, 0);
	pdpi = simple_strtoul(argv[3], NULL, 0);
	index = simple_strtoul(argv[4], NULL, 0);

	select_current_params((mt_u32)ssc, (mt_u32)pdpi);

	ret = mt_hdmi_digit_config_prefix((u32)id, (enum MT_HDMI_ANALOG_CFG_E)index);
	ret |= mt_hdmi_analog_config((u32)id, (enum MT_HDMI_ANALOG_CFG_E)index);
	ret |= mt_hdmi_digit_config_post((u32)id, (enum MT_HDMI_ANALOG_CFG_E)index);
	ret |= mt_hdmi_analog_config_post((u32)id, (enum MT_HDMI_ANALOG_CFG_E)index);

	if (ret == 0) {
		return CMD_RET_SUCCESS;
	} else {
		return CMD_RET_FAILURE;
	}
}

U_BOOT_CMD(
	mt_hdmi_analog_cfg, 5, 0, do_cmd_hdmi_analog,
	"HDMI Analog Configure",
	"\nmt_hdmi_analog_cfg id ssc pdpi index - hdmi analog configure\n"
	"                              id: HDMI device ID, default: 0\n"
	"                             ssc: 0: no ssc, 1: ssc\n"
	"                            pdpi: 0: include pi, 1: pd pi\n"
	"                           index: configuration index\n"
	"                               0: 148.5M x 1.5\n"
	"                               1: 148.5M x 1.25\n"
	"                               2: 148.5M\n"
	"                               3: 74.25M x 1.5\n"
	"                               4: 74.25M x 1.25\n"
	"                               5: 74.25M\n"
	"                               6: 27M x 1.5\n"
	"                               7: 27M x 1.25\n"
	"                               8: 27M\n"
	"                               9: 297M x 1.5, clk os 594M\n"
	"                              10: 297M x 1.25, clk os 594M\n"
	"                              11: 297M, clk os 594M\n"
	"                              12: 594M\n"
	"                              13: 297M x 1.5\n"
	"                              14: 297M x 1.25\n"
	"                              15: 297M\n"
	"                              16: 54M x 1.5\n"
	"                              17: 54M x 1.25\n"
	"                              18: 54M\n"
	"                              19: 108M\n"
);
#endif
#endif

