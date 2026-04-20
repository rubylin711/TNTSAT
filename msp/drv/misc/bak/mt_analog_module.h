/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_analog_module.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/05/14
 * Description    : Montage Analog modules definition.
 * History        :
 * 1.Date         : 2021/05/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_ANANLOG_MOD_H__
#define __INC_MT_ANANLOG_MOD_H__

#ifdef __UBOOT__
/* Uboot */
#define REG_ANA_TOP_BASE			0xBF157000
#define REG_ANALOG_SW_BASE			0xBF5D0000
#else
#define REG_ANA_TOP_BASE			SYMPHONY_IO_VA(0xBF157000)
#define REG_ANALOG_SW_BASE			SYMPHONY_IO_VA(0xBF5D0000)
#endif

//*ANA_AO
#undef ANA_AO_REG0
#define ANA_AO_REG0					(REG_ANA_TOP_BASE)
#undef ANA_AO_REG1
#define ANA_AO_REG1					(REG_ANA_TOP_BASE + 0x04)

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
#define REG_ANA_AO_LOCK				(REG_ANA_TOP_BASE + 0x08)
#define REG_ANA_AO_CFG_LOCK			(REG_ANA_TOP_BASE + 0x0C)
#else
#define REG_ANA_AO_LOCK				0
#define REG_ANA_AO_CFG_LOCK			0
#endif

/* REG_ANA_AO_LOCK/REG_ANA_AO_CFG_LOCK Lock Bit */
#define BIT_LOCK_ANA_AO_REG0		0
#define BIT_LOCK_ANA_AO_REG1		1
#define BIT_LOCK_ANA_AO_REG0_0_24	2

//*ANA_TOP
#undef ANA_TOP_REG1
#define ANA_TOP_REG1				(REG_ANALOG_SW_BASE + 0x94)
#define REG_CLKGEN_CPUPLL			(REG_ANALOG_SW_BASE + 0x48)
#define REG_CLKGEN_TEST				(REG_ANALOG_SW_BASE + 0x98)
#define REG_CLKGEN_PDSYS			(REG_ANALOG_SW_BASE + 0x9C)
#define REG_CLKGEN_EPHYPLL			(REG_ANALOG_SW_BASE + 0x118)
/* CLKGEN_EPHY_REG: CLKGEN_EPHYTX_REG[15:0] + CLKGEN_PDEPHY_REG[31:16] */
#define REG_CLKGEN_EPHY				(REG_ANALOG_SW_BASE + 0x11C)
#define CLKGEN_USBACLK_REG0			(REG_ANALOG_SW_BASE + 0x120)

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
#define REG_ANA_SW_LOCK				(REG_ANALOG_SW_BASE + 0xE0)
#define REG_ANA_SW_CFG_LOCK			(REG_ANALOG_SW_BASE + 0x10C)
#else
#define REG_ANA_SW_LOCK				0
#define REG_ANA_SW_CFG_LOCK			0
#endif

/* REG_ANA_SW_LOCK/REG_ANA_SW_CFG_LOCK Lock Bit */
#define BIT_LOCK_ANA_TOP_REG1		1
#define BIT_LOCK_CLKGEN_TEST		6
#define BIT_LOCK_CLKGEN_USBACLK		16
#define BIT_LOCK_CLKGEN_EPHY		21

//No Lock Bit
#ifndef NA_LOCK_BIT_IDX
#define NA_LOCK_BIT_IDX				(32)
#endif

//---------------------------------------------------------------------------//

/* Flags */
#define FLAG_ANA_GATE		0x01
#define FLAG_ANA_MUX		0x02
#define FLAG_ANA_RESET		0x04

#define INIT_MT_ANA_GATE(reg, sft, wd, slock, lock, lock_bit) {	\
				.reg_addr = reg,			\
				.shift = sft,				\
				.width = wd,				\
				.reg_slock = slock,			\
				.reg_lock = lock,			\
				.bit_lock = lock_bit,		\
				.reverse = 0 }

#define INIT_MT_ANA_GATE_REVERSE(reg, sft, wd, slock, lock, lock_bit) {	\
				.reg_addr = reg,			\
				.shift = sft, 				\
				.width = wd, 				\
				.reg_slock = slock, 		\
				.reg_lock = lock,			\
				.bit_lock = lock_bit,		\
				.reverse = 1 }

#define INIT_MT_ANA_MUX(reg, sft, wd, slock, lock, lock_bit) {	\
				.reg_addr = reg,			\
				.shift = sft,				\
				.width = wd,				\
				.reg_slock = slock,			\
				.reg_lock = lock,			\
				.bit_lock = lock_bit }

#define INIT_MT_ANA_RESET(reg, sft, wd, slock, lock, lock_bit) {	\
				.reg_addr = reg,			\
				.shift = sft,				\
				.width = wd,				\
				.reg_slock = slock,			\
				.reg_lock = lock,			\
				.bit_lock = lock_bit }

//---------------------------------------------------------------------------//

static struct mt_analog mt_ana_table[] =
{
	//ANA_AO
	{ "bias", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE(ANA_AO_REG0, 0, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0_0_24) },
	{ "usb0", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 1, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "usb1", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 2, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "icx", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 3, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	{ "adac_l", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 4, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_r", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 5, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_mute", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE(ANA_AO_REG0, 6, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "aadc_0", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 4, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "aadc_1", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 5, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac0", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 6, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
#endif

	{ "adac", FLAG_ANA_RESET, 0,
	  .reset = INIT_MT_ANA_RESET(ANA_TOP_REG1, 2, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_ANA_TOP_REG1) },
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "aadc", FLAG_ANA_RESET, 0,
	  .reset = INIT_MT_ANA_RESET(ANA_TOP_REG1, 1, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_ANA_TOP_REG1) },
#endif

	//8-11: hdmi tx channel
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	{ "hdmitx_ch", FLAG_ANA_GATE|FLAG_ANA_RESET, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 8, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0),
	  .reset = INIT_MT_ANA_RESET(ANA_AO_REG0, 7, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "hdmitx_ch0", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 9, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "hdmitx_ch1", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 10, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "hdmitx_ch2", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 11, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "hdmitx_ch0", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 8, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "hdmitx_ch1", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 9, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "hdmitx_ch2", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 10, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "hdmitx_ch", FLAG_ANA_GATE|FLAG_ANA_RESET, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 11, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0),
	  .reset = INIT_MT_ANA_RESET(ANA_AO_REG0, 7, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	{ "vdac0", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 12, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac1", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 13, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac2", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 14, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac3", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 15, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac0_det", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 16, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac1_det", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 17, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac2_det", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 18, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "vdac3_det", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 19, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	//12-19: adac_buf
	{ "adac_buf0", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 12, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_buf1", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 13, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_buf2", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 14, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_buf3", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 15, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_buf4", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 16, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_buf5", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 17, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_buf6", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 18, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_buf7", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 19, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
#endif

	{ "cadc",
#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	  FLAG_ANA_GATE,
#elif defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	  FLAG_ANA_GATE|FLAG_ANA_MUX,
#endif
	  0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 20, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0),
#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	  //0: ADC input
	  //1: digital input
	  .mux = INIT_MT_ANA_MUX(ANA_TOP_REG1, 3, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_ANA_TOP_REG1)
#endif
	},
	{ "sadc", FLAG_ANA_GATE|FLAG_ANA_MUX, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 21, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0),
	  //0: ADC input
	  //1: digital input
	  .mux = INIT_MT_ANA_MUX(ANA_TOP_REG1, 0, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_ANA_TOP_REG1) },
	//process monitor
	{ "proc_mon", FLAG_ANA_GATE|FLAG_ANA_RESET, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 22, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0),
	  .reset = INIT_MT_ANA_RESET(ANA_AO_REG0, 23, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "rng1", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 24, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0_0_24) },
	{ "usb_imp_cal", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 25, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "dreg_dc_test", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE(ANA_AO_REG0, 26, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	//temperature sensor
	{ "temp_sensor", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 27, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "adac_out_pull_down", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE(ANA_AO_REG0, 28, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	{ "rng2", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 29, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0) },
	//[31:30]
	{ "ephy_pll", FLAG_ANA_GATE|FLAG_ANA_RESET, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG0, 30, 2, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG0),
	  //pll soft reset
	  .reset = INIT_MT_ANA_RESET(REG_CLKGEN_EPHYPLL, 14, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_CLKGEN_EPHY) },
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "otp_reg", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 0, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
	{ "vdac0_det", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 1, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
	{ "adac_mute", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE(ANA_AO_REG1, 2, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
	{ "aadc_mic", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 4, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
#endif

	{ "vsd_pll", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 24, 2, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
	{ "vhd_pll", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 26, 2, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
	{ "usb_pll", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 28, 3, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	{ "cpu_pll", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 31, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "adc_pll", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(ANA_AO_REG1, 31, 1, REG_ANA_AO_LOCK, REG_ANA_AO_CFG_LOCK, BIT_LOCK_ANA_AO_REG1) },
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	{ "ephy", FLAG_ANA_GATE|FLAG_ANA_RESET, 0,
	  //ephy ana enable
	  .gate = INIT_MT_ANA_GATE(REG_CLKGEN_EPHY, 25, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_CLKGEN_EPHY),
	  //ephyrx reset
	  .reset = INIT_MT_ANA_RESET(REG_CLKGEN_EPHY, 7, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_CLKGEN_EPHY) },
#endif

	{ "clktest", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE(REG_CLKGEN_TEST, 11, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_CLKGEN_TEST) },
	{ "aclk_top", FLAG_ANA_GATE, 0,
	  .gate = INIT_MT_ANA_GATE_REVERSE(CLKGEN_USBACLK_REG0, 11, 1, REG_ANA_SW_LOCK, REG_ANA_SW_CFG_LOCK, BIT_LOCK_CLKGEN_USBACLK) },
};

#endif

