/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk_sym1.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Symphony1 Clock definition.
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
#define REG_CLK_BASE			SYMPHONY_IO_VA(0xBF500000)
#define REG_AO_BASE				SYMPHONY_IO_VA(0xBF150000)
#define REG_BOOT_BASE			SYMPHONY_IO_VA(0xBF140000)
#else
/* Uboot */
#define REG_CLK_BASE			0xBF500000
#define REG_AO_BASE				0xBF150000
#define REG_BOOT_BASE			0xBF140000
#endif

#define REG_BOOT_CFG			(REG_BOOT_BASE + 0x20)

#define REG_CLK_SYS_CFG			(REG_CLK_BASE)

#define REG_CLK_GATE_CFG0		(REG_CLK_BASE + 0x10)
#define REG_CLK_GATE_CFG1		(REG_CLK_BASE + 0x14)
#define REG_CLK_GATE_CFG2		(REG_CLK_BASE + 0x18)
#define REG_CLK_GATE_CFG3		(REG_CLK_BASE + 0x1C)

#define REG_CLK_GATE_CFG_AO		(REG_AO_BASE + 0x3000)

#define REG_CLK_DMA_CFG			(REG_CLK_BASE + 0x20)
#define REG_CLK_TRANSPORT_CFG	(REG_CLK_BASE + 0x24)
#define REG_CLK_VIDEO_CFG		(REG_CLK_BASE + 0x30)
#define REG_CLK_AUDIO_CFG		(REG_CLK_BASE + 0x34)

//*
//#define REG_CLK_UART0_CFG		(REG_CLK_BASE + 0x40)
//*
//#define REG_CLK_UART1_CFG		(REG_CLK_BASE + 0x44)

#define REG_CLK_SMC_CFG			(REG_CLK_BASE + 0x48)
#define REG_CLK_SPI_CFG			(REG_CLK_BASE + 0x4C)
#define REG_CLK_MAC_CFG			(REG_CLK_BASE + 0x50)
#define REG_CLK_CORE_CFG		(REG_CLK_BASE + 0x54)

//<CN>æß’ÒCLK
#define VAR_XTAL_CLK			(0xFFFFFFFF)
#define VAR_XTAL_CLK_HALF		(VAR_XTAL_CLK / 2)

//VOS
#define VAR_VENC_OSCLK			(0xFFFFFFFC)

#define VAR_HCLK_CLK			(0xFFFFFFFA)
#define VAR_HCLK_CLK_HALF		(VAR_HCLK_CLK / 2)

#define VAR_ACLK_CLK			(400000)
#define VAR_ACLK_CLK_HALF		(VAR_ACLK_CLK / 2)

//No Lock Bit
#define NA_LOCK_BIT_IDX			(32)

/* Notice: the following rates are in unit of KHz. */

static struct mt_clk_div_table apb_div_table[] =
{
	{3, MT_DIV_4},
	{4, MT_DIV_5},
	{5, MT_DIV_6},
	{6, MT_DIV_7},
	{7, MT_DIV_8},
	{8, MT_DIV_9},
	{9, MT_DIV_10},
	{10, MT_DIV_11},
	{11, MT_DIV_12},
	{12, MT_DIV_13},
	{13, MT_DIV_14},
	{14, MT_DIV_15},
	{15, MT_DIV_16},
	{0, 0}			//terminate with zero
};

static struct mt_clk_div_table ahb_div_table[] =
{
	{1, MT_DIV_2},
	{2, MT_DIV_3},
	{3, MT_DIV_4},
	{4, MT_DIV_5},
	{5, MT_DIV_6},
	{6, MT_DIV_7},
	{7, MT_DIV_8},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table sys2_rate_table[] =
{
	{0, 576000},
	{1, 463000},
	{2, 360000},
	{3, 262000},
	{4, 288000},
	{5, 231500},	//231.5
	{6, 180000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table sys1_rate_table[] =
{
	{0, 576000},
	{1, 648000},
	{2, 411000},
	{3, 720000},
	{4, 288000},
	{5, 324000},
	{6, 205500},	//205.5
	{7, 360000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table sys0_rate_table[] =
{
	{0, 576000},
	{1, 648000},
	{2, 720000},
	{3, 720000},
	{4, 288000},
	{5, 324000},
	{6, 360000},
	{7, 360000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table dma_rate_table[] =
{
	//{0, 295},
	{1, 262000},
	{2, VAR_HCLK_CLK},
	//{3, VAR_ACLK_CLK},
	{4, 147500},				//147.5
	{5, 131000},
	{6, VAR_HCLK_CLK_HALF},
	{7, VAR_ACLK_CLK_HALF},
	{0, 0}						//terminate with zero
};

static struct mt_clk_rate_table tsi_rate_table[] =
{
	{3, 262000},
	{4, 147500},	//147.5
	{5, 205500},	//205.5
	{6, 231500},	//231.5
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table aout_rate_table[] =
{
	{3, 262000},
	{4, 147500},	//147.5
	{5, 288000},
	{6, 205500},	//205.5
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_div_table venc_div_table[] =
{
	{0, MT_DIV_2},
	{1, MT_DIV_4},
	{2, MT_DIV_6},
	{3, MT_DIV_8},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table gra_rate_table[] =
{
	{3, 262000},
	{4, 147500},	//147.5
	{6, 205500},	//205.5
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table disp_rate_table[] =
{
	{3, 262000},
	{4, 147500},	//147.5
	{6, 205500},	//205.5
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table jpg_rate_table[] =
{
	{3, 262000},
	{4, 147500},	//147.5
	{6, 205500},	//205.5
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table vdec_rate_table[] =
{
	{0, 360000},
	{1, 405000},
	{2, 320000},
	{3, 262000},
	{4, 180000},
	{5, 202500},	//202.5
	{6, 160000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table spi_rate_table[] =
{
	{0, VAR_XTAL_CLK},
	{2, 594000},
	{3, 405000},
	{4, VAR_XTAL_CLK_HALF},
	{5, 360000},
	{6, 297000},
	{7, 204500},	//204.5
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table secure_hs_rate_table[] =
{
	{4, 147500},	//147.5
	{5, 205500},	//205.5
	{6, 160000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table dispdi_rate_table[] =
{
	{0, 295000},
	{3, 262000},
	{4, 147500},	//147.5
	{5, 288000},
	{6, 160000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table xtal_rate_table[] =
{
	{0, 27000},
	{1, 24000},
	{0, 0}			//terminate with zero
};

//---------------------------------------------------------------------------//

static struct mt_clk mt_clk_table[] =
{
	//SYS
	{ MT_APB_CLK, FLAG_CLK_DIVIDER | FLAG_CLK_MUX, 0, 0, 0,
	  .divider = INIT_MT_CLK_DIV(REG_CLK_SYS_CFG, 24, 4, apb_div_table, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: xtal clk
	  //1: sys_clk_0 / apb_div
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 4, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_AHB_CLK, FLAG_CLK_DIVIDER | FLAG_CLK_MUX, 0, 0, 0,
	  .divider = INIT_MT_CLK_DIV(REG_CLK_SYS_CFG, 20, 3, ahb_div_table, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: xtal clk
	  //1: sys_clk_0 / ahb_div
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 5, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "sys_clk2", FLAG_CLK_RATE, 0, 0, 0,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SYS_CFG, 16, 3, sys2_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "sys_clk1", FLAG_CLK_RATE, 0, 0, 0,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SYS_CFG, 12, 3, sys1_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "sys_clk0", FLAG_CLK_RATE, 0, 0, 0,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SYS_CFG, 8, 3, sys0_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "cpu2_clk", FLAG_CLK_GATE | FLAG_CLK_MUX, 0, 0, 0,
	  //0: xtal clk
	  //1: sys_clk_2
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 2, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 2, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_AVCPU_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX, 0, 0, 0,
	  //0: xtal clk
	  //1: sys_clk_1
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 1, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_APCPU_CLK, FLAG_CLK_MUX, 0, 0, 0,
	  //0: xtal clk
	  //1: sys_clk_0
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 0, 1, NA_LOCK_BIT_IDX/*LOCK*/) },

	//CFG0
	{ "avdma_clk", FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 12, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_DMA_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 8, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_DMA_CFG, 16, 3, dma_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_DDR_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 4, NA_LOCK_BIT_IDX/*LOCK*/) },

	//CFG1
	{ "crypto_clk", FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG1, 8, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_TSI_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG1, 0, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 20, 3, tsi_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },

	//CFG2
	{ MT_SPDIF_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 25, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_AOUT_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 24, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_AUDIO_CFG, 4, 3, aout_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_VBI_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 11, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_HDMI_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 10, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_HVENC_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX | FLAG_CLK_DIVIDER | FLAG_CLK_ATTR, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 9, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: analog hd clk("vhd_clk")
	  //1: analog os clk("venc_osclk") div
	  .mux = INIT_MT_CLK_MUX(REG_CLK_VIDEO_CFG, 19, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: No DIV
	  //1: DIV
	  .attr = INIT_MT_CLK_MUX(REG_CLK_VIDEO_CFG, 18, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  .divider = INIT_MT_CLK_DIV(REG_CLK_VIDEO_CFG, 16, 2, venc_div_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_SVENC_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 8, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_GRA_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 3, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 12, 3, gra_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_DISP_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 2, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 4, 3, disp_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_JPG_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 16, 3, jpg_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_VDEC_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 0, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 0, 3, vdec_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },

	//CFG3
	{ MT_I2CDEBUG_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 20, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_SMC0_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 16, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: apb_clk
	  //1: xtal_clk / 2
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SMC_CFG, 0, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_I2C3_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 15, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_I2C2_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 14, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_I2C1_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 13, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_I2C0_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 12, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_SPI1_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 10, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_SPI0_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 9, NA_LOCK_BIT_IDX/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SPI_CFG, 0, 3, spi_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_RMII_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 5, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_MAC_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX | FLAG_CLK_ATTR, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 4, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: MII
	  //1: RMII
	  .mux = INIT_MT_CLK_MUX(REG_CLK_MAC_CFG, 8, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: IO
	  //1: IO reverse
	  //2: org 50MHz
	  //3: org 50MHz reverse
	  .attr = INIT_MT_CLK_ATTR(REG_CLK_MAC_CFG, 0, 2, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_USB1_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_USB0_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 0, NA_LOCK_BIT_IDX/*LOCK*/) },

	//CFG_AO
	{ MT_IRDA_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 3, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_LEDKB_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 2, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_FPI2C_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_FPSPI_CLK, FLAG_CLK_GATE, 0, 0, 0,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 0, NA_LOCK_BIT_IDX/*LOCK*/) },

	//TRANSPORT
	{ "secure_clk", FLAG_CLK_MUX, 0, 0, 0,
	  //0: xtal_clk
	  //1: secure_hs_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TRANSPORT_CFG, 19, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "secure_hs_clk", FLAG_CLK_RATE, 0, 0, 0,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_TRANSPORT_CFG, 16, 3, secure_hs_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_TS3_CLK, FLAG_CLK_EDGE, 0, 0, 0,
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 7, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_TS2_CLK, FLAG_CLK_EDGE, 0, 0, 0,
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 6, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_TS1_CLK, FLAG_CLK_MUX | FLAG_CLK_EDGE, 0, 0, 0,
	  //0: S DEMO clk
	  //1: pad ts1_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TRANSPORT_CFG, 1, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 5, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_TS0_CLK, FLAG_CLK_MUX | FLAG_CLK_EDGE, 0, 0, 0,
	  //0: C DEMO clk
	  //1: pad ts0_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TRANSPORT_CFG, 0, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 4, 1, NA_LOCK_BIT_IDX/*LOCK*/) },

	//CORE
	{ MT_DI_CLK, FLAG_CLK_RATE, 0, 0, 0,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 8, 3, dispdi_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },

	//BOOT_CFG
	{ MT_XTAL_CLK, FLAG_CLK_RATE, 0, 0, 0,
	  .rate = INIT_MT_CLK_RATE(REG_BOOT_CFG, 30, 1, xtal_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) }
};

