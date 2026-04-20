/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Montage Clock definition.
 * History        :
 * 1.Date         : 2021/02/08
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_CLK_H__
#define __INC_MT_CLK_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* clock device */
#define MT_CLK_DEV			NULL

/* clock name */
#define MT_XTAL_CLK			"xtal_clk"
#define MT_AHB_CLK			"ahb_clk"
#define MT_APB_CLK			"apb_clk"

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
#define MT_APCPU_CLK		"apcpu_clk"
#define MT_AVCPU_CLK		"avcpu_clk"
#else
#define MT_APCPU_CLK		"cpu0_clk"	/*Symphony1/2*/
#define MT_AVCPU_CLK		"cpu1_clk"	/*Symphony1/2*/
#endif

#define MT_MAC_CLK			"mac_clk"
#define MT_RMII_CLK			"rmii_clk"

#define MT_DDR_CLK			"ddr_clk"
#define MT_DMA_CLK			"dma_clk"

#define MT_TSI_CLK			"tsi_clk"
#define MT_TS0_CLK			"ts0_clk"
#define MT_TS1_CLK			"ts1_clk"
#define MT_TS2_CLK			"ts2_clk"
#define MT_TS3_CLK			"ts3_clk"

#define MT_VDEC_CLK			"vdec_clk"
#define MT_JPG_CLK			"jpg_clk"
#define MT_DISP_CLK			"disp_clk"
#define MT_DI_CLK			"dispdi_clk"
#define MT_GRA_CLK			"gra_clk"
#define MT_SVENC_CLK		"svenc_clk"
#define MT_HVENC_CLK		"hvenc_clk"
#define MT_HDMI_CLK			"hdmi_clk"
#define MT_VBI_CLK			"vbi_clk"
#define MT_AOUT_CLK			"aout_clk"
#define MT_SPDIF_CLK		"spdif_clk"

#define MT_USB0_CLK			"usb0_clk"
#define MT_USB1_CLK			"usb1_clk"
#define MT_SPI0_CLK			"spi0_clk"
#define MT_SPI1_CLK			"spi1_clk"
#define MT_I2C0_CLK			"i2c0_clk"
#define MT_I2C1_CLK			"i2c1_clk"
#define MT_I2C2_CLK			"i2c2_clk"
#define MT_I2C3_CLK			"i2c3_clk"
#define MT_SMC0_CLK			"smc0_clk"
#define MT_I2CDEBUG_CLK		"i2cdebug_clk"

/* Symphony4 */
#define MT_UART0_CLK		"uart0_clk"
#define MT_UART1_CLK		"uart1_clk"
#define MT_PNAND_CLK		"pnand_clk"
#define MT_SDIO0_CLK		"sdio0_clk"
#define MT_SDIO1_CLK		"sdio1_clk"

#define MT_LCD_CLK			"lcd_clk"
#define MT_PNG_CLK			"png_clk"
#define MT_DAI_CLK			"dai_clk"
#define MT_CI_CLK			"ci_clk"

//AO(AlwaysOn)
#define MT_FPSPI_CLK		"fpspi_clk"
#define MT_FPI2C_CLK		"fpi2c_clk"
#define MT_LEDKB_CLK		"ledkb_clk"
#define MT_IRDA_CLK			"irda_clk"
#define MT_AOMCU_CLK		"aomcu_clk"
#define MT_TIMER0_CLK		"timer0_clk"		/* Symphony2 Only */
#define MT_TIMER1_CLK		"timer1_clk"		/* Symphony2 Only */
#define MT_TIMER_CLK		"timer_clk"			/* Symphony4 Only */
#define MT_MAILBOX_CLK		"mailbox_clk"
#define MT_RTC_CLK			"rtc_clk"			/* Symphony2 Only */
#define MT_KADC_CLK			"kadc_clk"
#define MT_RECRAM_CLK		"recram_clk"
#define MT_AOANA_CLK		"ao_ana_clk"
#define MT_AOGPIO_CLK		"ao_gpio_clk"
#define MT_AOPINMUX_CLK		"ao_pinmux_clk"
#define MT_AOLPM_CLK		"ao_lpm_clk"

/* Symphony4 */
#define MT_CEC_CLK			"cec_clk"

/* Flags */
#define FLAG_CLK_GATE		0x01
#define FLAG_CLK_RATE		0x02
#define FLAG_CLK_MUX		0x04
#define FLAG_CLK_DIVIDER	0x08
#define FLAG_CLK_EDGE		0x10
#define FLAG_CLK_ATTR		0x20
#define FLAG_CLK_XTAL_SEL	0x40

/* Divide factor */
enum MT_DIV_E
{
	MT_DIV_1 	= 100,
	MT_DIV_2 	= 200,	/* 1/2 */
	MT_DIV_2_5 	= 250,	/* 1/2.5 */
	MT_DIV_2_75 = 275,	/* 1/2.75 */
	MT_DIV_3 	= 300,	/* 1/3 */
	MT_DIV_3_5 	= 350,	/* 1/3.5 */
	MT_DIV_4 	= 400,	/* 1/4 */
	MT_DIV_5	= 500,	/* 1/5 */
	MT_DIV_6 	= 600,	/* 1/6 */
	MT_DIV_7	= 700,	/* 1/7 */
	MT_DIV_8 	= 800,	/* 1/8 */
	MT_DIV_9	= 900,	/* 1/9 */
	MT_DIV_10	= 1000,	/* 1/10 */
	MT_DIV_11	= 1100,	/* 1/11 */
	MT_DIV_12	= 1200,	/* 1/12 */
	MT_DIV_13	= 1300,	/* 1/13 */
	MT_DIV_14	= 1400,	/* 1/14 */
	MT_DIV_15	= 1500,	/* 1/15 */
	MT_DIV_16	= 1600	/* 1/16 */
};

/* Common fields of MT CLK */
#define MT_CLK_COMMON_FIELDS			\
				mt_u32 reg_addr; 		\
				union {					\
					mt_u32 bit_idx;		\
					mt_u32 shift;		\
				};						\
				mt_u32 width;			\
				mt_u32 bit_idx_lock;	/* LOCK&SLOCK bit_idx */

/* Initialize common fields of MT CLK */
#define INIT_MT_CLK(reg, sft, wd, lck) {	\
				.reg_addr = reg,			\
				.shift = sft,				\
				.width = wd,				\
				.bit_idx_lock = lck }

#define INIT_MT_CLK_GATE(reg, idx, lck) {	\
				.reg_addr = reg,			\
				.shift = idx,				\
				.width = 1,					\
				.bit_idx_lock = lck }

#define INIT_MT_CLK_RATE(reg, sft, wd, tab, lck) {	\
				.reg_addr = reg,					\
				.shift = sft,						\
				.width = wd,						\
				.bit_idx_lock = lck,				\
				.ptable = tab }

#define INIT_MT_CLK_DIV(reg, sft, wd, tab, lck)	{	\
				.reg_addr = reg,					\
				.shift = sft,						\
				.width = wd,						\
				.bit_idx_lock = lck,				\
				.ptable = tab }

#define INIT_MT_CLK_MUX		INIT_MT_CLK
#define INIT_MT_CLK_EDGE	INIT_MT_CLK
#define INIT_MT_CLK_ATTR	INIT_MT_CLK

#define INIT_MT_CLK_XTAL_SEL(reg, idx, lck)		\
				INIT_MT_CLK(reg, idx, 1, lck)

/**
 * gate clock
 */
struct mt_clk_gate
{
	MT_CLK_COMMON_FIELDS
};

/**
 * rate map table
 */
struct mt_clk_rate_table
{
	mt_u32 val;		/* register value */
	mt_u32 rate;
};

/**
 * rate clock
 */
struct mt_clk_rate
{
	MT_CLK_COMMON_FIELDS

	struct mt_clk_rate_table *ptable;
};

/**
 * divide map table
 */
struct mt_clk_div_table
{
	mt_u32 val;		//register value
	mt_u32 div;		//see: enum MT_DIV_E
};

/**
 * divider clock
 */
struct mt_clk_divider
{
	MT_CLK_COMMON_FIELDS

	struct mt_clk_div_table *ptable;
};

/**
 * mux clock
 */
struct mt_clk_mux
{
	MT_CLK_COMMON_FIELDS
};

/**
 * clock edge
 */
struct mt_clk_edge
{
	MT_CLK_COMMON_FIELDS
};

/**
 * clock attribute
 * <CN>¿©’π Ù–‘
 */
struct mt_clk_attribute
{
	MT_CLK_COMMON_FIELDS
};

/**
 * clock xtal select
 */
struct mt_clk_xtal_sel
{
	MT_CLK_COMMON_FIELDS
};

/**
 * composite clock
 */
struct mt_clk
{
	const char *name;
	mt_u32 flags;		/* FLAG_CLK_GATE,
						   FLAG_CLK_RATE,
						   FLAG_CLK_MUX,
						   FLAG_CLK_DIVIDER,
						   FLAG_CLK_ATTR,
						   FLAG_CLK_XTAL_SEL */
	mt_u32 ref_count;	/* child referenced count */

	/* X_SLOCK_REG */
	mt_u32 reg_slock;
	/* X_LOCK_REG */
	mt_u32 reg_lock;

	struct mt_clk_gate gate;

	struct mt_clk_rate rate;

	struct mt_clk_mux mux;

	struct mt_clk_divider divider;

	struct mt_clk_edge edge;

	struct mt_clk_attribute attr;

	struct mt_clk_xtal_sel xtal_sel;
};

//---------------------------------------------------------------------------//

/**
 * @brief get clock handle
 *
 * @param[in] dev not used, reserved for future use.
 * @param[in] id clock name, such as "mac_clk".
 */
struct mt_clk *mt_clk_get(void *dev, const char *id);

mt_s32 mt_clk_enable(struct mt_clk *clk);

void mt_clk_disable(struct mt_clk *clk);

mt_u32 mt_clk_get_enable(struct mt_clk *clk);

mt_u32 mt_clk_get_rate(struct mt_clk *clk);

/**
 * @brief set clock rate
 *
 * @param[in] rate clock rate, in unit of KHz.
 */
mt_s32 mt_clk_set_rate(struct mt_clk *clk, mt_u32 rate);

mt_u32 mt_clk_get_div(struct mt_clk *clk);

/**
 * @brief set divide factor
 *
 * @param[in] div divide factor, see enum MT_DIV_E
 */
mt_s32 mt_clk_set_div(struct mt_clk *clk, mt_u32 div);

mt_u32 mt_clk_get_mux(struct mt_clk *clk);

mt_s32 mt_clk_set_mux(struct mt_clk *clk, mt_u32 mux);

mt_u32 mt_clk_get_edge(struct mt_clk *clk);

mt_s32 mt_clk_set_edge(struct mt_clk *clk, mt_u32 edge);

mt_u32 mt_clk_get_attr(struct mt_clk *clk);

mt_s32 mt_clk_set_attr(struct mt_clk *clk, mt_u32 attr);

/**
 * @hide
 *
 * @brief select hide xtal clock rate
 *
 * @param[in] clk xtal select clock, such as "avcpu_xtal_sel"
 */
mt_s32 mt_clk_select_xtal(struct mt_clk *clk);

/**
 * @hide
 *
 * @brief unselect hide xtal clock rate
 *
 * @param[in] clk xtal select clock, such as "avcpu_xtal_sel"
 */
mt_s32 mt_clk_unselect_xtal(struct mt_clk *clk);

/**
 *@hide
 */
mt_u32 mt_clk_get_xtal_sel(struct mt_clk *clk);

///////////////////////////////////////////////////////////////////////////////
//APL

/**
 * @brief select to hide xtal clk rate.
 *   used for power saving.
 *
 * @param[in] clk clock, such as "avcpu_clk"
 *
 * @return
 *    MT_SUCCESS
 *    MT_FAILURE
 */
mt_s32 mt_clk_apl_select_xtal(struct mt_clk *clk);

/**
 * @brief unselect from hide xtal clk rate
 *
 * @param[in] clk clock, such as "avcpu_clk"
 * @param[in] rate new clock rate, recovered from xtal clk
 *
 * @return
 *    MT_SUCCESS
 *    MT_FAILURE
 */
mt_s32 mt_clk_apl_unselect_xtal(struct mt_clk *clk, mt_u32 rate);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

