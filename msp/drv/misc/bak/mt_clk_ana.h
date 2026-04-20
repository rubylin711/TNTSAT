/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk_ana.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Symphony ANALOG Clock definition.
 * History        :
 * 1.Date         : 2021/02/08
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "mt_clk.h"

#ifdef __KERNEL__
#define REG_ANA_TOP_BASE		SYMPHONY_IO_VA(0xBF157000)
#define REG_ANALOG_SW_BASE		SYMPHONY_IO_VA(0xBF5D0000)
#else
/* Uboot */
#define REG_ANA_TOP_BASE		0xBF157000
#define REG_ANALOG_SW_BASE		0xBF5D0000
#endif

//*ANA_TOP
#undef ANA_AO_REG0
#define ANA_AO_REG0				(REG_ANA_TOP_BASE)
#undef ANA_AO_REG1
#define ANA_AO_REG1				(REG_ANA_TOP_BASE + 0x04)

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
#define REG_ANA_AO_LOCK			(REG_ANA_TOP_BASE + 0x08)
#define REG_ANA_AO_CFG_LOCK		(REG_ANA_TOP_BASE + 0x0C)
#else
#define REG_ANA_AO_LOCK			0
#define REG_ANA_AO_CFG_LOCK		0
#endif

/* REG_ANA_AO_LOCK/REG_ANA_AO_CFG_LOCK Lock Bit */
#define BIT_LOCK_ANA_AO_REG0		0
#define BIT_LOCK_ANA_AO_REG1		1
#define BIT_LOCK_ANA_AO_REG0_0_24	2

//*ANALOG_SW
#undef ANA_TOP_REG1
#define ANA_TOP_REG1			(REG_ANALOG_SW_BASE + 0x94)

#define REG_CLKGEN_CPUPLL		(REG_ANALOG_SW_BASE + 0x48)
#define REG_CLKGEN_ETHINTP		(REG_ANALOG_SW_BASE + 0x4C)
#define REG_CLKGEN_VHDPLL		(REG_ANALOG_SW_BASE + 0x58)
#define REG_CLKGEN_VHDINTP		(REG_ANALOG_SW_BASE + 0x5C)
#define REG_CLKGEN_VSDPLL		(REG_ANALOG_SW_BASE + 0x60)
#define REG_CLKGEN_VSDINTP		(REG_ANALOG_SW_BASE + 0x64)
/*
 * Low 16 Bit : clkgen_ephyrx_reg[15:0]
 * High 16 Bit: clkgen_pdephy_reg[15:0]
 */
#define REG_CLKGEN_EPHY			(REG_ANALOG_SW_BASE + 0x11C)
#define CLKGEN_USBACLK_REG0		(REG_ANALOG_SW_BASE + 0x120)
#define REG_CLKGEN_VSDSSC		(REG_ANALOG_SW_BASE + 0x144)

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
#define REG_ANA_SW_LOCK			(REG_ANALOG_SW_BASE + 0xE0)
#define REG_ANA_SW_CFG_LOCK		(REG_ANALOG_SW_BASE + 0x10C)
#else
#define REG_ANA_SW_LOCK			0
#define REG_ANA_SW_CFG_LOCK		0
#endif

/* REG_ANA_SW_LOCK/REG_ANA_SW_CFG_LOCK Lock Bit */
#define BIT_LOCK_ANA_TOP_REG1	1
#define BIT_LOCK_VHDINTP		2
#define BIT_LOCK_ETHINTP		5
#define BIT_LOCK_CPUPLL			9
#define BIT_LOCK_VSDINTP		10
#define BIT_LOCK_USBACLK		16
#define BIT_LOCK_VSDSSC			17
#define BIT_LOCK_EPHY			21

//No Lock Bit
#ifndef NA_LOCK_BIT_IDX
#define NA_LOCK_BIT_IDX			(32)
#endif

/* Notice: the following rates are in unit of KHz. */

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
static struct mt_clk_rate_table cadc_rate_table[] =
{
	{0, 144000},
	{1, 270000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table sadc_rate_table[] =
{
	{0, 960000},
	{1, 1100000},
	{0, 0}			//terminate with zero
};
#endif

static struct mt_clk_rate_table hdmitx_rate_table[] =
{
	{0, 742500},	//742.5
	{1, 742500},	//742.5
	{2, 1485000},
	{3, 270000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table vhd_rate_table[] =
{
	{0, 74250},		//74.25
	{1, 148500},	//148.5
	{2, 108000},
	{3, 27000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table vos_rate_table[] =
{
	{0, 74250},		//74.25
	{1, 148500},	//148.5
	{2, 108000},
	{3, 27000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table vsd_rate_table[] =
{
	{0, 108000},
	{1, 27000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table vhd_ana_rate_table[] =
{
	{0, 74250},		//74.25
	{1, 148500},	//148.5
	{2, 297000},
	{4, 108000},
	{5, 27000},
	{6, 108000},
	{7, 27000},
	{0, 0}			//terminate with zero
};

//---------------------------------------------------------------------------//

static struct mt_clk mt_clk_ana_table[] =
{
	//ANA_TOP
	{ "bias", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(ANA_AO_REG0, 0, BIT_LOCK_ANA_AO_REG0_0_24/*LOCK*/) },
	{ MT_USB0_ANA_CLK, FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 1, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ MT_USB1_ANA_CLK, FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 2, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "icx", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 3, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	{ "adac_l", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 4, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_r", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 5, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_mute", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(ANA_AO_REG0, 6, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "aadc0", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 4, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "aadc1", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 5, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac0", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 6, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#endif

	//8-11: hdmi tx channel
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	{ "hdmitx_ch", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 8, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "hdmitx_ch0", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 9, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "hdmitx_ch1", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 10, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "hdmitx_ch2", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 11, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "hdmitx_ch", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 11, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "hdmitx_ch0", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 8, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "hdmitx_ch1", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 9, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "hdmitx_ch2", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 10, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	{ "vdac0", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 12, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac1", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 13, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac2", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 14, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac3", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 15, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac0_det", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 16, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac1_det", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 17, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac2_det", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 18, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "vdac3_det", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 19, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	//12-19: adac_buf
	{ "adac_buf0", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 12, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_buf1", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 13, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_buf2", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 14, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_buf3", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 15, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_buf4", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 16, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_buf5", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 17, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_buf6", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 18, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_buf7", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 19, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#endif

	{ "cadc_clk", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 20, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "cadc_clk_amd", FLAG_CLK_MUX | FLAG_CLK_RATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  //0: ADC input
	  //1: digital input
	  .mux = INIT_MT_CLK_MUX(ANA_TOP_REG1, 3, 1, BIT_LOCK_ANA_TOP_REG1/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLKGEN_CPUPLL, 13, 1, cadc_rate_table, BIT_LOCK_CPUPLL/*LOCK*/) },
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	{ "sadc_clk", FLAG_CLK_GATE | FLAG_CLK_MUX, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 21, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: ADC input
	  //1: digital input
	  .mux = INIT_MT_CLK_MUX(ANA_TOP_REG1, 0, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "sadc_clk", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 21, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "sadc_clk_amd", FLAG_CLK_MUX | FLAG_CLK_RATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  //0: ADC input
	  //1: digital input
	  .mux = INIT_MT_CLK_MUX(ANA_TOP_REG1, 0, 1, BIT_LOCK_ANA_TOP_REG1/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLKGEN_ETHINTP, 4, 1, sadc_rate_table, BIT_LOCK_ETHINTP/*LOCK*/) },
#endif
	//process monitor
	{ "proc_mon", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 22, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	{ "rng", FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 24, NA_LOCK_BIT_IDX/*LOCK*/) },
#endif

	{ "usb_imp_cal", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 25, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "rng1", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 24, BIT_LOCK_ANA_AO_REG0_0_24/*LOCK*/) },
	{ "dreg_dc_test", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(ANA_AO_REG0, 26, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	//temperature sensor
	{ "temp_sensor", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 27, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "adac_out_pull_down", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(ANA_AO_REG0, 28, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
	{ "rng2", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG0, 29, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },

	//[31:30]
	{ "ephy_pll", FLAG_CLK_ATTR, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .attr = INIT_MT_CLK_ATTR(ANA_AO_REG0, 30, 2, BIT_LOCK_ANA_AO_REG0/*LOCK*/) },
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY2)
	//[25:24] pd vsd pll
	//[27:26] pd vhd pll
	//[30:28] pd usb pll
	{ "vsd_pll", FLAG_CLK_ATTR, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .attr = INIT_MT_CLK_ATTR(ANA_AO_REG1, 24, 2, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
	{ "vhd_pll", FLAG_CLK_ATTR, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .attr = INIT_MT_CLK_ATTR(ANA_AO_REG1, 26, 2, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
	{ "usb_pll", FLAG_CLK_ATTR, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .attr = INIT_MT_CLK_ATTR(ANA_AO_REG1, 28, 3, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },

	{ "cpu_pll", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG1, 31, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "otp_reg", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG1, 0, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
	{ "vdac_det", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG1, 1, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
	{ "adac_mute", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(ANA_AO_REG1, 2, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
	{ "aadc_mic", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG1, 4, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },

	//[25:24] pd vsd pll
	//[27:26] pd vhd pll
	//[30:28] pd usb pll
	{ "vsd_pll", FLAG_CLK_ATTR, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .attr = INIT_MT_CLK_ATTR(ANA_AO_REG1, 24, 2, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
	{ "vhd_pll", FLAG_CLK_ATTR, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .attr = INIT_MT_CLK_ATTR(ANA_AO_REG1, 26, 2, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
	{ "usb_pll", FLAG_CLK_ATTR, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .attr = INIT_MT_CLK_ATTR(ANA_AO_REG1, 28, 3, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },

	{ "adc_pll", FLAG_CLK_GATE, 0, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(ANA_AO_REG1, 31, BIT_LOCK_ANA_AO_REG1/*LOCK*/) },
#endif

	//*VHDINTP
	{ "hdmitx_clk", FLAG_CLK_RATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLKGEN_VHDINTP, 4, 2, hdmitx_rate_table, BIT_LOCK_VHDINTP/*LOCK*/) },
	{ "venc_osclk", FLAG_CLK_RATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLKGEN_VHDINTP, 2, 2, vos_rate_table, BIT_LOCK_VHDINTP/*LOCK*/) },
	{ "vhd_clk", FLAG_CLK_RATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLKGEN_VHDINTP, 0, 2, vhd_rate_table, BIT_LOCK_VHDINTP/*LOCK*/) },

	//*VSDINTP
	{ "vsd_clk", FLAG_CLK_RATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLKGEN_VSDINTP, 3, 1, vsd_rate_table, BIT_LOCK_VSDINTP/*LOCK*/) },
	{ "vhd_ana_clk", FLAG_CLK_RATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLKGEN_VSDINTP, 0, 3, vhd_ana_rate_table, BIT_LOCK_VSDINTP/*LOCK*/) },

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	//*REG_CLKGEN_EPHY
	{ "ephy_ana_clk", FLAG_CLK_GATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLKGEN_EPHY, 25, BIT_LOCK_EPHY/*LOCK*/) },

	//*REG_CLKGEN_VSDSSC
	{ "vsd_ssc_clk", FLAG_CLK_GATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(REG_CLKGEN_VSDSSC, 4, BIT_LOCK_VSDSSC/*LOCK*/) },

	//*CLKGEN_USBACLK_REG0
	{ "aclk_top_reg", FLAG_CLK_GATE, 0, REG_ANA_SW_CFG_LOCK, REG_ANA_SW_LOCK,
	  .gate = INIT_MT_CLK_GATE_REVERSE(CLKGEN_USBACLK_REG0, 11, BIT_LOCK_USBACLK/*LOCK*/) }
#endif
};

