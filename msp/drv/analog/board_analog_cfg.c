/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Montage LZ Co., Ltd.
 */
#ifdef __UBOOT__
#include <common.h>

#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#include <asm/arch-symphony6/mt_analog_attr.h>
#define NO_DUMP_ADAC_STATE
#include <asm/arch-symphony6/mt_analog_adac.h>

#include <asm/arch-symphony6/mt_drv_otp.h>

#include "board_config.h"

#ifdef PATCH_BIAS_MICO
#define NO_DUMP_BIAS_STATE
#include <asm/arch-symphony6/mt_analog_bias.h>
#endif

#elif defined(__KERNEL__)

#include <linux/types.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <mach/chipinfo.h>

#include <mt_common.h>

#include "../crm/mt_reg_base.h"

#include <mt_drv_analog.h>
#include <mt_drv_clock.h>
#include <mt_drv_otp.h>

#include <analog/mt_analog_attr.h>
#define NO_DUMP_ADAC_STATE
#include <analog/mt_analog_adac.h>

#include "mt_drv_pinctrl.h"
#include "drv_gpio_ioctl.h"

#define debug(...)		do{}while(0)

/*
 * User Force defined ADAC Type
 *  0: 0dBu, external
 *  1: both internal and external
 *  2: 2Vrms, internal
 */
/*#define CFG_USR_FORCE_ADAC_TYPE			1*/

#else

/* TODO: RTOS*/
#error "Board CFG only support Uboot and Kernel by now!"

#endif

#ifndef CFG_EXT_ADAC_AMP_MUTE_GPIO
/*
 * External ADAC AMP Mute/UnMute GPIO
 */
#define CFG_EXT_ADAC_AMP_MUTE_GPIO		38U
#endif

#ifndef CFG_ADAC_RL_SWP
/*
 * ADAC R/L data swap
 */
#define CFG_ADAC_RL_SWP
#endif

/*
 * Board Analog Configurations:
 *    TOP/AO,
 *    ADAC,
 *    BIAS,
 *    EPHY,
 *    ADC,
 *    USB,
 *    ...
 */

int board_analog_cfg(void)
{
	debug("%s\n", __FUNCTION__);

	/*
	 * step 1: disable
	 * step 2: reset/release
	 * step 3: config
	 * step 4: enable
	 */

/* disable analog module */
#if 0
	mt_analog_disable(MT_ANA_ADAC);
#endif

/* reset/release analog module */
#if 0
	mt_analog_reset(MT_ANA_ADAC);
	udelay(CFG_MT_ANA_RESET_UDELAY_MIN);
	mt_analog_release(MT_ANA_ADAC);
#endif

/* set analog module attribute */
#if 0
	struct mt_analog_adac_attr attr;

	memset(&attr, 0, sizeof(attr));

	mt_analog_get_attr(MT_ANA_INDEX_ADAC, &attr);

	//TODO:
	//attr.scsw = ;
	//...

	mt_analog_set_attr(MT_ANA_INDEX_ADAC, &attr);
#endif

	//e.g.
	//MT_ANALOG_UP_ATTR(MT_ANA_INDEX_AO_R0, struct mt_analog_ao_r0_attr, pd_usb0, 1);

/* enable analog module */
#if 0
	mt_analog_enable(MT_ANA_ADAC);
#endif

	//TODO
	mt_analog_reset(MT_ANA_VDAC);
	udelay(CFG_MT_ANA_RESET_UDELAY_MIN);
	mt_analog_release(MT_ANA_VDAC);
	mt_analog_enable(MT_ANA_VDAC);

/* Bias(HDMI) Patch for MICO */
#ifdef PATCH_BIAS_MICO
	/* BF5D0138<8:6> 0b100 -> 0b010 */
	printf("Patch Bias for Mico - Enable\n");
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_BIAS, struct mt_analog_bias_attr, vdd0p9, 0x2);
	printf("Reg[%8lX]: %08X\r\n", REG_ANALOG_SW_BASE + 0x138, readl(REG_ANALOG_SW_BASE + 0x138));
#endif

	//usb30 pll dcc
	mt_analog_disable((mt_ana_t*)"usb30_pll_dcc");

	return 0;
}

//---------------------------------------------------------------------------//

/* ADAC Configs */

/*
 * mute/unmute external ADAC AMP by GPIO
 */
#ifdef __UBOOT__
static int adac_set_ext_mute(int mute)
{
#ifdef CFG_EXT_ADAC_AMP_MUTE_GPIO
	if (mute)
	{
		debug("Mute External ADAC AMP.\r\n");
		mt_gpio_set_value(CFG_EXT_ADAC_AMP_MUTE_GPIO, GPIO_VALUE_LOW_LEVEL);
	}
	else
	{
		debug("UnMute External ADAC AMP.\r\n");
		mt_pinctrl_set_function((PINMUX_INDEX_E)CFG_EXT_ADAC_AMP_MUTE_GPIO, 1/*gpio: 1*/);

		mt_gpio_set_enable(CFG_EXT_ADAC_AMP_MUTE_GPIO, 1);
		mt_gpio_set_dir(CFG_EXT_ADAC_AMP_MUTE_GPIO, GPIO_DIR_OUTPUT);
		mt_gpio_set_value(CFG_EXT_ADAC_AMP_MUTE_GPIO, GPIO_VALUE_HIGH_LEVEL);
	}
#endif
	return 0;
}
#elif defined(__KERNEL__)
static int adac_set_ext_mute(int mute)
{
#ifdef CFG_EXT_ADAC_AMP_MUTE_GPIO
	if (mute)
	{
		debug("Mute External ADAC AMP.\r\n");
		drv_gpio_set_value(CFG_EXT_ADAC_AMP_MUTE_GPIO, GPIO_VALUE_LOW_LEVEL);
	}
	else
	{
		debug("UnMute External ADAC AMP.\r\n");
		mt_pinctrl_set_function((PINMUX_INDEX_E)CFG_EXT_ADAC_AMP_MUTE_GPIO, 1/*gpio: 1*/);

		drv_gpio_io_enable(CFG_EXT_ADAC_AMP_MUTE_GPIO, 1);
		drv_gpio_set_dir(CFG_EXT_ADAC_AMP_MUTE_GPIO, GPIO_DIR_OUTPUT);
		drv_gpio_set_value(CFG_EXT_ADAC_AMP_MUTE_GPIO, GPIO_VALUE_HIGH_LEVEL);
	}
#endif
	return 0;
}
#else

/* TODO: RTOS */
#error "Board CFG only support Uboot and Kernel by now!"

#endif

static int board_adac_cfg_a0(void)
{
	struct mt_analog_ao_r0_attr aor0_attr;
	struct mt_analog_adac_attr  adac_attr;

	debug("board ADAC config ...\r\n");

	debug("Reg[%8lX]: %08X\r\n", REG_AO_ANALOG_BASE, readl(REG_AO_ANALOG_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_ANALOG_SW_BASE, readl(REG_ANALOG_SW_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_CRM_BASE+0xA500/*AOUT*/, readl(REG_CRM_BASE+0xA500));
	debug("Reg[%8lX]: %08X\r\n", REG_AOUT_BASE + 0x8/*AOUT*/, readl(REG_AOUT_BASE + 0x8));

	//FIXME
	mt_analog_reset(MT_ANA_ADAC);

	//0
	mt_clk_enable(MT_CLK_AOUT);
	mt_clk_reset(MT_CLK_AOUT);
	mt_clk_enable(MT_CLK_SPDIF);
	mt_clk_enable(MT_CLK_ADAC);
	mdelay(10);

	//1
	/* startup mode */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, scsw, 3);
	/* normal operation */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, scsw, 0);

	//2
	/* buffer internal power down */
	mt_analog_get_attr_lock(MT_ANA_INDEX_AO_R0, &aor0_attr);
	aor0_attr.pd_buf_rch  = 1;
	aor0_attr.pd_adac_rch = 0;
	aor0_attr.pd_vddb1p2  = 0;
	aor0_attr.pd_buf_lch  = 1;
	aor0_attr.pd_adac_lch = 0;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_AO_R0, &aor0_attr);

	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 26-29 */
	/* 1100: buffer input current 2.5uA */
	//adac_attr.enb_svpw = 0xC;					/* FIXME bit 26-28 + bit 29 */
	adac_attr.enb_svpw = 0x4;					/* bit 26-28 */
	adac_attr.vsel2 = 1;						/* bit 29 */
	/*
	 * bit 25: 1, PMOS switch to vdd off
	 */
	adac_attr.buf_start_P = 1;
	/*
	 * bit 23: 1, assisted-m2p8 on
	 */
	adac_attr.buf_start_vddb2p8 = 1;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);
	mdelay(2);

	//3
	/* discharge */
	/* bit 20-21: 01 */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, buf_en_clk, 1);

	/* bit 20-21: 00 */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, buf_en_clk, 0);
	mdelay(1);

	//4
	/* buffer internal power on */
	mt_analog_get_attr_lock(MT_ANA_INDEX_AO_R0, &aor0_attr);
	aor0_attr.pd_buf_rch  = 0;
	aor0_attr.pd_adac_rch = 0;
	aor0_attr.pd_vddb1p2  = 0;
	aor0_attr.pd_buf_lch  = 0;
	aor0_attr.pd_adac_lch = 0;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_AO_R0, &aor0_attr);

	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 26-29 */
	/* 0011: buffer input current 50uA */
	//adac_attr.enb_svpw = 0x3;					/* FIXME bit 26-28 + bit 29 */
	adac_attr.enb_svpw = 0x3;					/* bit 26-28 */
	adac_attr.vsel2 = 0;						/* bit 29 */
	/* bit 20-21 */
	/* 10: charge pump starts work */
	adac_attr.buf_en_clk = 0x2;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);
	mdelay(1);

	//5
	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 20-21 */
	/* 11: charge pump works */
	adac_attr.buf_en_clk = 0x3;

	/*
	 * bit 24: 1, NMOS switch to vdd off
	 */
	adac_attr.buf_start_N = 1;
	/*
	 * bit 23: 0, assisted-m2p8 off
	 */
	adac_attr.buf_start_vddb2p8 = 0;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);

#ifdef CFG_ADAC_RL_SWP
	/*adac R/L swap control, default 0.*/
	/* bit 0: 1 */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_TOP_R3, struct mt_analog_top_r3_attr, adac_rl_data_swap, 1);
#endif

	//FIXME
	//0xBF490008 = 0x8000
	writel(0x8000, (void*)(REG_AOUT_BASE + 0x8));			/* AOUT open from mute */

	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 16-17 */
	/* 01: 2Vrms buff load 10kohm resistor */
	adac_attr.buf_sel_out = 0x1;

	/* bit 9 */
	adac_attr.mode_sel = 0;		/* 0: 0dB gain for 0dBu output */
	//adac_attr.mode_sel = 0x1;		/* 1: -5dB gain for 2Vrms output */
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);
	udelay(100);

	//FIXME
	mt_analog_release(MT_ANA_ADAC);

	debug("Reg[%8lX]: %08X\r\n", REG_AO_ANALOG_BASE, readl(REG_AO_ANALOG_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_ANALOG_SW_BASE, readl(REG_ANALOG_SW_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_CRM_BASE+0xA500/*AOUT*/, readl(REG_CRM_BASE+0xA500));
	debug("Reg[%8lX]: %08X\r\n", REG_AOUT_BASE + 0x8/*AOUT*/, readl(REG_AOUT_BASE + 0x8));

	debug("board ADAC config ... done\r\n");

	return 0;
}

static int board_adac_cfg_a1(void)
{
	struct mt_analog_ao_r0_attr aor0_attr;
	struct mt_analog_adac_attr  adac_attr;
	//int OTP_ADAC_Type = 0;		/* 0dBu, external */
	int OTP_ADAC_Type = 1;		/* both internal and external */
	//int OTP_ADAC_Type = 2;		/* 2Vrms, internal */

	debug("board ADAC config ...\r\n");

	debug("Reg[%8lX]: %08X\r\n", REG_AO_ANALOG_BASE, readl(REG_AO_ANALOG_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_ANALOG_SW_BASE, readl(REG_ANALOG_SW_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_CRM_BASE+0xA500/*AOUT*/, readl(REG_CRM_BASE+0xA500));
	debug("Reg[%8lX]: %08X\r\n", REG_AOUT_BASE + 0x8/*AOUT*/, readl(REG_AOUT_BASE + 0x8));

#ifdef CFG_USR_FORCE_ADAC_TYPE
	OTP_ADAC_Type = CFG_USR_FORCE_ADAC_TYPE;
#else
	mt_otp_get_obj(OTP_ADAC_PackageType, &OTP_ADAC_Type, NULL);
#endif
	debug("ADAC Type: %d\r\n", OTP_ADAC_Type);

	//FIXME
	mt_analog_reset(MT_ANA_ADAC);

	//1
	mt_clk_enable(MT_CLK_AOUT);
	mt_clk_enable(MT_CLK_SPDIF);
	mt_clk_enable(MT_CLK_ADAC);
	mt_clk_reset(MT_CLK_AOUT);

	mdelay(10);		//DELAY 150

	//2
	/* bit 1-0 */
	/* startup mode */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, scsw, 0x3);
	/* normal operation */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, scsw, 0);

	//3
	/* bit 19-12 */
	/* buffer internal power down for A1 */
	mt_analog_get_attr_lock(MT_ANA_INDEX_AO_R0, &aor0_attr);
	aor0_attr.pd_buf_rch  = 1;
	aor0_attr.pd_adac_rch = 0;
	aor0_attr.pd_1bit_int = 0;
	aor0_attr.pd_vddb1p2  = 0;

	aor0_attr.pd_buf_lch  = 1;
	aor0_attr.pd_adac_lch = 0;
	aor0_attr.reserved4   = 0;
	aor0_attr.pd_internal_vdd2p7 = 0;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_AO_R0, &aor0_attr);

	//4
	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 26-28 */
	/* 100: buffer input current 2.5uA */
	adac_attr.enb_svpw = 0x4;
	/* bit 25 */
	/* PMOS switch to vdd off */
	adac_attr.buf_start_P = 1;
	/*
	 * bit 23: 1, assisted-m2p8 on
	 */
	adac_attr.buf_start_vddb2p8 = 1;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);

	mdelay(5);		//DELAY 50

	//5
	/* bit 21-20 */
	/* discharge */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, buf_en_clk, 1);
	/* disable */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr, buf_en_clk, 0);
	mdelay(1);		//DELAY 1

	//6
	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 15 */
	/* 1bit_pcm_start_status for A1 */
	adac_attr.buf_pcm_start = 1;
	/* bit 31 */
	/* adac_scfilter_start for A1 */
	adac_attr.scfilter_start = 1;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);

	//7
	/* bit 19-12 */
	/* buffer internal power on */
	mt_analog_get_attr_lock(MT_ANA_INDEX_AO_R0, &aor0_attr);
	if (OTP_ADAC_Type == 1) {
		/* Both */
		aor0_attr.pd_buf_rch  = 0;
		aor0_attr.pd_adac_rch = 0;
		aor0_attr.pd_1bit_int = 1;			/* A1 */
		aor0_attr.pd_vddb1p2  = 0;

		aor0_attr.pd_buf_lch  = 0;
		aor0_attr.pd_adac_lch = 0;
		aor0_attr.reserved4   = 1;
		aor0_attr.pd_internal_vdd2p7 = 0;	/* A1 */
	} else if (OTP_ADAC_Type == 0) {
		/* 0dBu */
		aor0_attr.pd_buf_rch  = 1;
		aor0_attr.pd_adac_rch = 0;
		aor0_attr.pd_1bit_int = 1;			/* A1 */
		aor0_attr.pd_vddb1p2  = 1;

		aor0_attr.pd_buf_lch  = 1;
		aor0_attr.pd_adac_lch = 0;
		aor0_attr.reserved4   = 1;
		aor0_attr.pd_internal_vdd2p7 = 0;	/* A1 */
	} else {
		/* 2Vrms */
		aor0_attr.pd_buf_rch  = 0;
		aor0_attr.pd_adac_rch = 1;
		aor0_attr.pd_1bit_int = 1;			/* A1 */
		aor0_attr.pd_vddb1p2  = 0;

		aor0_attr.pd_buf_lch  = 0;
		aor0_attr.pd_adac_lch = 1;
		aor0_attr.reserved4   = 1;
		aor0_attr.pd_internal_vdd2p7 = 0;	/* A1 */
	}
	mt_analog_set_attr_unlock(MT_ANA_INDEX_AO_R0, &aor0_attr);

	//8
	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 26-28 */
	/* 011: buffer input current 50uA */
	adac_attr.enb_svpw = 0x3;
	/* bit 21-20 */
	/* charge pump starts work */
	adac_attr.buf_en_clk = 0x2;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);
	mdelay(1);		//DELAY 1

	//9
	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 21-20 */
	/* normal operation */
	adac_attr.buf_en_clk = 0x3;
	/* bit 24 */
	/* NMOS switch to sub/vdd off */
	adac_attr.buf_start_N = 1;
	/* bit 23 */
	/* assisted-m2p8 off */
	adac_attr.buf_start_vddb2p8 = 0;
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);

#ifdef CFG_ADAC_RL_SWP
	/* adac R/L swap control, default 0. */
	/* bit 0: 1 */
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_TOP_R3, struct mt_analog_top_r3_attr, adac_rl_data_swap, 1);
#endif

	//10
	/* Aout open from mute, gain=max */
	//0xBF490008 = 0x8000
	writel(0x8000, (void*)(REG_AOUT_BASE + 0x8));

	//11
	mt_analog_get_attr_lock(MT_ANA_INDEX_ADAC, &adac_attr);
	/* bit 17-16 */
	/* 2Vrms buff load 10kohm resistor */
	adac_attr.buf_sel_out = 0x1;
	/* bit 9 */
	/* 0: 0dB gain for 0dBu output */
	/* 1: -5dB gain for 2Vrms output */
	if ((OTP_ADAC_Type == 0) || (OTP_ADAC_Type == 1)/*Both*/)
		adac_attr.mode_sel = 0;	/* 0dBu */
	else
		adac_attr.mode_sel = 1;	/* 2Vrms */
	mt_analog_set_attr_unlock(MT_ANA_INDEX_ADAC, &adac_attr);

	mdelay(10);		//DELAY 100

	//FIXME
	mt_analog_release(MT_ANA_ADAC);

	//External AMP: unmute
	if ((OTP_ADAC_Type == 0) || (OTP_ADAC_Type == 1)/*Both*/)
	{
		adac_set_ext_mute(0);
	}

	debug("Reg[%8lX]: %08X\r\n", REG_AO_ANALOG_BASE, readl(REG_AO_ANALOG_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_ANALOG_SW_BASE, readl(REG_ANALOG_SW_BASE));
	debug("Reg[%8lX]: %08X\r\n", REG_CRM_BASE+0xA500/*AOUT*/, readl(REG_CRM_BASE+0xA500));
	debug("Reg[%8lX]: %08X\r\n", REG_AOUT_BASE + 0x8/*AOUT*/, readl(REG_AOUT_BASE + 0x8));

	debug("board ADAC config ... done\r\n");

	return 0;
}

int board_adac_cfg(void)
{
#ifdef __UBOOT__
	if (mt_get_chip_id() == MT_CHIP_SYMPHONY6_A0)
		return board_adac_cfg_a0();
	else
		return board_adac_cfg_a1();
#elif defined(__KERNEL__)
	if (symphony_get_chip_rev() == CHIP_SYMPHONY6_A0)
		return board_adac_cfg_a0();
	else
		return board_adac_cfg_a1();
#else
/* TODO: RTOS */
#endif
}

int board_adac_onoff(int on)
{
	struct mt_analog_ao_r0_attr aor0_attr;
	//int OTP_ADAC_Type = 0;		/* 0dBu, external */
	int OTP_ADAC_Type = 1;		/* both internal and external */
	//int OTP_ADAC_Type = 2;		/* 2Vrms, internal */

	debug("ADAC switch - %s\r\n", on?"on":"off");

#ifdef CFG_USR_FORCE_ADAC_TYPE
	OTP_ADAC_Type = CFG_USR_FORCE_ADAC_TYPE;
#else
	mt_otp_get_obj(OTP_ADAC_PackageType, &OTP_ADAC_Type, NULL);
#endif

	/* switch on */
	if (on)
	{
		//FIXME: might has pop noise issue!!!

		/* bit 19-12 */
		/* buffer internal power down for A1 */
		mt_analog_get_attr_lock(MT_ANA_INDEX_AO_R0, &aor0_attr);
		if (OTP_ADAC_Type == 1) {
			/* Both */
			aor0_attr.pd_buf_rch  = 0;
			aor0_attr.pd_adac_rch = 0;
			aor0_attr.pd_1bit_int = 1;			/* A1 */
			aor0_attr.pd_vddb1p2  = 0;

			aor0_attr.pd_buf_lch  = 0;
			aor0_attr.pd_adac_lch = 0;
			aor0_attr.reserved4   = 1;
			aor0_attr.pd_internal_vdd2p7 = 0;	/* A1 */
		} else if (OTP_ADAC_Type == 0) {
			/* 0dBu */
			aor0_attr.pd_buf_rch  = 1;
			aor0_attr.pd_adac_rch = 0;
			aor0_attr.pd_1bit_int = 1;			/* A1 */
			aor0_attr.pd_vddb1p2  = 1;

			aor0_attr.pd_buf_lch  = 1;
			aor0_attr.pd_adac_lch = 0;
			aor0_attr.reserved4   = 1;
			aor0_attr.pd_internal_vdd2p7 = 0;	/* A1 */
		} else {
			/* 2Vrms */
			aor0_attr.pd_buf_rch  = 0;
			aor0_attr.pd_adac_rch = 1;
			aor0_attr.pd_1bit_int = 1;			/* A1 */
			aor0_attr.pd_vddb1p2  = 0;

			aor0_attr.pd_buf_lch  = 0;
			aor0_attr.pd_adac_lch = 1;
			aor0_attr.reserved4   = 1;
			aor0_attr.pd_internal_vdd2p7 = 0;	/* A1 */
		}
		mt_analog_set_attr_unlock(MT_ANA_INDEX_AO_R0, &aor0_attr);

		//External AMP: UnMute
		if ((OTP_ADAC_Type == 0) || (OTP_ADAC_Type == 1)/*Both*/)
		{
			adac_set_ext_mute(0);
		}
	}
	else
	{
		//External AMP: Mute
		if ((OTP_ADAC_Type == 0) || (OTP_ADAC_Type == 1)/*Both*/)
		{
			adac_set_ext_mute(1);

#if defined(__UBOOT__)
			mdelay(50);
#elif defined(__KERNEL__)
			msleep(50);
#else
/* TODO: RTOS */
#endif
		}

		//FIXME: might has pop noise issue if internal AMP!

		/* bit 19-12 */
		/* buffer internal power down for A1 */
		mt_analog_get_attr_lock(MT_ANA_INDEX_AO_R0, &aor0_attr);
		aor0_attr.pd_buf_rch  = 1;
		aor0_attr.pd_adac_rch = 1;
		aor0_attr.pd_1bit_int = 1;
		aor0_attr.pd_vddb1p2  = 1;

		aor0_attr.pd_buf_lch  = 1;
		aor0_attr.pd_adac_lch = 1;
		aor0_attr.reserved4   = 1;
		aor0_attr.pd_internal_vdd2p7 = 1;
		mt_analog_set_attr_unlock(MT_ANA_INDEX_AO_R0, &aor0_attr);
	}

	return 0;
}

