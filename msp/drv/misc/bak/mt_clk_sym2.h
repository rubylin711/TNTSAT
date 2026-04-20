/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk_sym2.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Symphony2 Clock definition.
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
#define REG_SECURE_BASE			SYMPHONY_IO_VA(0xBF300000)
#define REG_BOOT_BASE			SYMPHONY_IO_VA(0xBF140000)
#else
/* Uboot */
#define REG_CLK_BASE			0xBF500000
#define REG_AO_BASE				0xBF150000
#define REG_SECURE_BASE			0xBF300000
#define REG_BOOT_BASE			0xBF140000
#endif

#define REG_BOOT_CFG			(REG_BOOT_BASE + 0x20)

#define REG_CLK_SYS_CFG			(REG_CLK_BASE)

#define REG_CLK_GATE_CFG0		(REG_CLK_BASE + 0x10)
#define REG_CLK_GATE_CFG1		(REG_CLK_BASE + 0x14)
#define REG_CLK_GATE_CFG2		(REG_CLK_BASE + 0x18)
#define REG_CLK_GATE_CFG3		(REG_CLK_BASE + 0x1C)

#define REG_CLK_DMA_CFG			(REG_CLK_BASE + 0x20)
#define REG_CLK_TRANSPORT_CFG	(REG_CLK_BASE + 0x24)
#define REG_CLK_VIDEO_CFG		(REG_CLK_BASE + 0x30)
#define REG_CLK_AUDIO_CFG		(REG_CLK_BASE + 0x34)
#define REG_CLK_SMC_CFG			(REG_CLK_BASE + 0x48)
#define REG_CLK_SPI_CFG			(REG_CLK_BASE + 0x4C)
#define REG_CLK_MAC_CFG			(REG_CLK_BASE + 0x50)
#define REG_CLK_CORE_CFG		(REG_CLK_BASE + 0x54)
#define REG_CLK_GF_CFG			(REG_CLK_BASE + 0x58)
#define REG_CLK_TSICLK_CFG		(REG_CLK_BASE + 0x5C)

#define REG_CLK_CFG_OWNER_LOCK	(REG_CLK_BASE + 0x70)
#define REG_CLK_CFG_LOCK		(REG_CLK_BASE + 0x74)

//LOCK Bit
#define BIT_LOCK_INTERFACE		18
#define BIT_LOCK_MAC			17
#define BIT_LOCK_USB			16
#define BIT_LOCK_AOUT			15
#define BIT_LOCK_HDMI			14
#define BIT_LOCK_VDENC			13
#define BIT_LOCK_GRA			12
#define BIT_LOCK_DISP			11
#define BIT_LOCK_JPG			10
#define BIT_LOCK_VDEC			9
#define BIT_LOCK_DEMO			8
#define BIT_LOCK_SECURE			7
#define BIT_LOCK_TSI			6
#define BIT_LOCK_DMA			5
#define BIT_LOCK_DDR			4
//#define BIT_LOCK_SYS			3	//reserved
#define BIT_LOCK_SYS			32	//NA_LOCK_BIT_IDX
#define BIT_LOCK_CPU2			2
#define BIT_LOCK_CPU1			1
#define BIT_LOCK_CPU0			0

////AO
#define REG_CLK_GATE_CFG_AO		(REG_AO_BASE + 0x3000)

#define REG_CLK_CFG_AO_OWNER_LOCK	(REG_AO_BASE + 0x3000 + 0x10)
#define REG_CLK_CFG_AO_LOCK		(REG_AO_BASE + 0x3000 + 0x14)
//LOCK Bit
#define BIT_LOCK_LPM			16
#define BIT_LOCK_PINMUX			15
#define BIT_LOCK_GPIO			14
#define BIT_LOCK_ANA			13
#define BIT_LOCK_XTAL			12
#define BIT_LOCK_RNG			11
#define BIT_LOCK_RECRAM			10
#define BIT_LOCK_KADC			9
#define BIT_LOCK_RTC			8
#define BIT_LOCK_MAILBOX		7
#define BIT_LOCK_TIMER1			6
#define BIT_LOCK_TIMER0			5
#define BIT_LOCK_MCU			4
#define BIT_LOCK_IRDA			3
#define BIT_LOCK_LEDKB			2
#define BIT_LOCK_FPI2C			1
#define BIT_LOCK_FPSPI			0

////SECURE
#define REG_CRM_DS_CFG			(REG_SECURE_BASE + 0xF0D0)
#define REG_CRM_SEC_CFG			(REG_SECURE_BASE + 0xF0E0)

#define REG_CRM_SEC_LOCK		(REG_SECURE_BASE + 0xF0C0)
#define BIT_LOCK_CRM_DS			1
#define BIT_LOCK_CRM_SEC		0

#define REG_CPU2_CLK_GATE		(REG_SECURE_BASE + 0xF200)
#define REG_CPU2_CLK_GATE_LOCK	(REG_SECURE_BASE + 0xF204)

//<CN>æß’ÒCLK
#define VAR_XTAL_CLK			(0xFFFFFFFF)
#define VAR_XTAL_CLK_HALF		(VAR_XTAL_CLK / 2)

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
	{0, 0}				//terminate with zero
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
	{0, 0}				//terminate with zero
};

static struct mt_clk_rate_table sys2_rate_table[] =
{
	{1, 411000},
	{2, 360000},
	{3, 262000},
	{4, 288000},
	{5, 205500},	//205.5
	{6, 180000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table sys1_rate_table[] =
{
	{0, 576000},
	{2, 411000},
	{4, 288000},
	{5, 333500},	//333.5
	{6, 205500},	//205.5
	{7, 360000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table sys0_rate_table[] =
{
	{0, 576000},
	{1, 667000},
	{2, 720000},
	{3, 720000},
	{4, 288000},
	{5, 333500},	//333.5
	{6, 360000},
	{7, 360000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table dma_rate_table[] =
{
	{1, 262000},
	{2, VAR_HCLK_CLK},
	{4, 147500},				//147.5
	{5, 131000},
	{6, VAR_HCLK_CLK_HALF},
	{7, VAR_ACLK_CLK_HALF},
	{0, 0}						//terminate with zero
};

static struct mt_clk_rate_table tsi_rate_table[] =
{
	{3, 262000},
	{4, 144000},
	{5, 205500},	//205.5
	{6, 240000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table aout_rate_table[] =
{
	{3, 262000},
	{4, 144000},
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
	{4, 144000},
	{6, 205500},	//205.5
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table disp_rate_table[] =
{
	{2, 205500},	//205.5
	{4, 147500},	//147.5
	{5, 180000},
	{6, 102750},	//102.75
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table jpg_rate_table[] =
{
	{3, 262000},
	{4, 144000},
	{6, 205500},	//205.5
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table vdec_rate_table[] =
{
	{3, 262000},
	{4, 180000},
	{5, 200000},
	{6, 160000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table spi_rate_table[] =
{
	{0, VAR_XTAL_CLK},
	{3, 400000},
	{4, VAR_XTAL_CLK_HALF},
	{5, 360000},
	{6, 288000},
	{7, 200000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table secure_hs_rate_table[] =
{
	{3, 262000},
	{4, 144000},
	{5, 205500},	//205.5
	{6, 160000},
	{7, 131000},
	{0, 0}			//terminate with zero
};

static struct mt_clk_rate_table dispdi_rate_table[] =
{
	{0, 288000},
	{3, 262000},
	{4, 144000},
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
	{ MT_APB_CLK, FLAG_CLK_DIVIDER | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .divider = INIT_MT_CLK_DIV(REG_CLK_SYS_CFG, 24, 4, apb_div_table, BIT_LOCK_SYS/*LOCK*/),
	  //0: xtal clk
	  //1: sys_clk_0 / apb_div
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 4, 1, BIT_LOCK_SYS/*LOCK*/) },
	{ MT_AHB_CLK, FLAG_CLK_DIVIDER | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .divider = INIT_MT_CLK_DIV(REG_CLK_SYS_CFG, 20, 3, ahb_div_table, BIT_LOCK_SYS/*LOCK*/),
	  //0: xtal clk
	  //1: sys_clk_0 / ahb_div
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 5, 1, BIT_LOCK_SYS/*LOCK*/) },
	{ "sys_clk2", FLAG_CLK_RATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SYS_CFG, 16, 3, sys2_rate_table, BIT_LOCK_CPU2/*LOCK*/) },
	{ "sys_clk1", FLAG_CLK_RATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SYS_CFG, 12, 3, sys1_rate_table, BIT_LOCK_CPU1/*LOCK*/) },
	{ "sys_clk0", FLAG_CLK_RATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SYS_CFG, 8, 3, sys0_rate_table, BIT_LOCK_CPU0/*LOCK*/) },
	{ "cpu2_clk", FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: xtal clk
	  //1: sys_clk_2
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 2, 1, BIT_LOCK_CPU2/*LOCK*/) },
	//TODO
	{ "cpu2_clk_gate", FLAG_CLK_GATE, 0, REG_CPU2_CLK_GATE_LOCK, REG_CPU2_CLK_GATE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CPU2_CLK_GATE, 0, 0/*LOCK*/) },
	{ MT_AVCPU_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: xtal clk
	  //1: sys_clk_1
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 1, 1, BIT_LOCK_CPU1/*LOCK*/),
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 1, BIT_LOCK_CPU1/*LOCK*/) },
	{ MT_APCPU_CLK, FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: xtal clk
	  //1: sys_clk_0
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SYS_CFG, 0, 1, BIT_LOCK_CPU0/*LOCK*/) },

	//CFG0
	{ "avdma_clk", FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 12, BIT_LOCK_DMA/*LOCK*/) },
	{ MT_DMA_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 8, BIT_LOCK_DMA/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_DMA_CFG, 16, 3, dma_rate_table, BIT_LOCK_DMA/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 4, 1, BIT_LOCK_DMA/*LOCK*/) },
	{ MT_DDR_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG0, 4, BIT_LOCK_DDR/*LOCK*/) },

	//CFG1
	{ "glitchdet_clk", FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG1, 10, BIT_LOCK_SECURE/*LOCK*/) },
	{ "crypto_clk", FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG1, 8, BIT_LOCK_SECURE/*LOCK*/) },
	{ MT_TSI_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG1, 0, BIT_LOCK_TSI/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 20, 3, tsi_rate_table, BIT_LOCK_TSI/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 6, 1, BIT_LOCK_TSI/*LOCK*/) },

	//CFG2
	{ MT_SPDIF_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 25, BIT_LOCK_AOUT/*LOCK*/) },
	{ MT_AOUT_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 24, BIT_LOCK_AOUT/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_AUDIO_CFG, 4, 3, aout_rate_table, BIT_LOCK_AOUT/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 13, 1, BIT_LOCK_AOUT/*LOCK*/) },
	{ MT_VBI_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 11, BIT_LOCK_VDENC/*LOCK*/) },
	{ MT_HDMI_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 10, BIT_LOCK_HDMI/*LOCK*/) },
	{ MT_HVENC_CLK, FLAG_CLK_GATE | FLAG_CLK_DIVIDER | FLAG_CLK_ATTR, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 9, BIT_LOCK_VDENC/*LOCK*/),
	  //0: No DIV
	  //1: DIV
	  .attr = INIT_MT_CLK_MUX(REG_CLK_VIDEO_CFG, 18, 1, BIT_LOCK_VDENC/*LOCK*/),
	  .divider = INIT_MT_CLK_DIV(REG_CLK_VIDEO_CFG, 16, 2, venc_div_table, BIT_LOCK_VDENC/*LOCK*/) },
	{ MT_SVENC_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 8, BIT_LOCK_VDENC/*LOCK*/) },
	{ MT_GRA_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 3, BIT_LOCK_GRA/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 12, 3, gra_rate_table, BIT_LOCK_GRA/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 12, 1, BIT_LOCK_GRA/*LOCK*/) },
	{ MT_DISP_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 2, BIT_LOCK_DISP/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 4, 3, disp_rate_table, BIT_LOCK_DISP/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 8, 1, BIT_LOCK_DISP/*LOCK*/) },
	{ MT_JPG_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 1, BIT_LOCK_JPG/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 16, 3, jpg_rate_table, BIT_LOCK_JPG/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 14, 1, BIT_LOCK_JPG/*LOCK*/) },
	{ MT_VDEC_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG2, 0, BIT_LOCK_VDEC/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 0, 3, vdec_rate_table, BIT_LOCK_VDENC/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 15, 1, BIT_LOCK_VDEC/*LOCK*/) },

	//CFG3
	{ MT_I2CDEBUG_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 20, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_SMC0_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 16, BIT_LOCK_INTERFACE/*LOCK*/),
	  //0: apb_clk
	  //1: xtal_clk / 2
	  .mux = INIT_MT_CLK_MUX(REG_CLK_SMC_CFG, 0, 1, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_I2C3_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 15, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_I2C2_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 14, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_I2C1_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 13, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_I2C0_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 12, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_SPI1_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 10, BIT_LOCK_INTERFACE/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 1, 1, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_SPI0_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 9, BIT_LOCK_INTERFACE/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CLK_SPI_CFG, 0, 3, spi_rate_table, BIT_LOCK_INTERFACE/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 0, 1, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_RMII_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 5, BIT_LOCK_MAC/*LOCK*/) },
	{ MT_MAC_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX | FLAG_CLK_ATTR, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 4, BIT_LOCK_MAC/*LOCK*/),
	  //0: MII
	  //1: RMII
	  .mux = INIT_MT_CLK_MUX(REG_CLK_MAC_CFG, 8, 1, BIT_LOCK_INTERFACE/*LOCK*/),
	  //0: IO
	  //1: IO reverse
	  //2: org 50MHz
	  //3: org 50MHz reverse
	  .attr = INIT_MT_CLK_ATTR(REG_CLK_MAC_CFG, 0, 2, BIT_LOCK_INTERFACE/*LOCK*/) },
	{ MT_USB1_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 1, BIT_LOCK_USB/*LOCK*/) },
	{ MT_USB0_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG3, 0, BIT_LOCK_USB/*LOCK*/) },

	//CFG_AO
	{ MT_AOLPM_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 16, BIT_LOCK_LPM/*LOCK*/) },
	{ MT_AOPINMUX_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 15, BIT_LOCK_PINMUX/*LOCK*/) },
	{ MT_AOGPIO_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 14, BIT_LOCK_GPIO/*LOCK*/) },
	{ MT_AOANA_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 13, BIT_LOCK_ANA/*LOCK*/) },
	{ MT_XTAL_CLK, FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  //0: i_xtal_clk
	  //1: i_ana_security_clk / 8
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GATE_CFG_AO, 12, 1, BIT_LOCK_XTAL/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_BOOT_CFG, 30, 1, xtal_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_RECRAM_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 10, BIT_LOCK_RECRAM/*LOCK*/) },
	{ MT_KADC_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 9, BIT_LOCK_KADC/*LOCK*/) },
	{ MT_RTC_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 8, BIT_LOCK_RTC/*LOCK*/) },
	{ MT_MAILBOX_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 7, BIT_LOCK_MAILBOX/*LOCK*/) },
	{ MT_TIMER1_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 6, BIT_LOCK_TIMER1/*LOCK*/) },
	{ MT_TIMER0_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 5, BIT_LOCK_TIMER0/*LOCK*/) },
	{ MT_AOMCU_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 4, BIT_LOCK_MCU/*LOCK*/) },
	{ MT_IRDA_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 3, BIT_LOCK_IRDA/*LOCK*/) },
	{ MT_LEDKB_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 2, BIT_LOCK_LEDKB/*LOCK*/) },
	{ MT_FPI2C_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 1, BIT_LOCK_FPI2C/*LOCK*/) },
	{ MT_FPSPI_CLK, FLAG_CLK_GATE, 0, REG_CLK_CFG_AO_OWNER_LOCK, REG_CLK_CFG_AO_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CLK_GATE_CFG_AO, 0, BIT_LOCK_FPSPI/*LOCK*/) },

	//TRANSPORT
	{ "secure_clk", FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: xtal_clk
	  //1: secure_hs_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TRANSPORT_CFG, 19, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "secure_hs_clk", FLAG_CLK_RATE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_TRANSPORT_CFG, 16, 3, secure_hs_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_TS3_CLK, FLAG_CLK_EDGE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 7, 1, BIT_LOCK_TSI/*LOCK*/) },
	{ MT_TS2_CLK, FLAG_CLK_EDGE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 6, 1, BIT_LOCK_TSI/*LOCK*/) },
	{ MT_TS1_CLK, FLAG_CLK_MUX | FLAG_CLK_EDGE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: demo0 clk
	  //1: pad ts2_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TRANSPORT_CFG, 1, 1, BIT_LOCK_TSI/*LOCK*/),
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 5, 1, BIT_LOCK_TSI/*LOCK*/) },
	{ MT_TS0_CLK, FLAG_CLK_MUX | FLAG_CLK_EDGE, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: demoS1 clk
	  //1: pad ts1_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TRANSPORT_CFG, 0, 1, BIT_LOCK_TSI/*LOCK*/),
	  //0: Rise Edge
	  //1: Falling Edge
	  .edge = INIT_MT_CLK_EDGE(REG_CLK_TRANSPORT_CFG, 4, 1, BIT_LOCK_TSI/*LOCK*/) },

	//CORE
	{ MT_DI_CLK, FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_CLK_CORE_CFG, 8, 3, dispdi_rate_table, BIT_LOCK_DISP/*LOCK*/),
	  //0:
	  //1: xtal_clk
	  .mux = INIT_MT_CLK_MUX(REG_CLK_GF_CFG, 9, 1, BIT_LOCK_DISP/*LOCK*/) },

	//TSICLK
	{ "demos2_ts3_clk", FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: pad
	  //1: demoS2
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TSICLK_CFG, 2, 1, BIT_LOCK_TSI/*LOCK*/) },
	{ "demos1_ts1_clk", FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: 0xBF500024 bit[0]
	  //   0: demoS1
	  //   1: pad ts1
	  //1: demoS1
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TSICLK_CFG, 1, 1, BIT_LOCK_TSI/*LOCK*/) },
	{ "demos1_ts0_clk", FLAG_CLK_MUX, 0, REG_CLK_CFG_OWNER_LOCK, REG_CLK_CFG_LOCK,
	  //0: pad
	  //1: demoS1
	  .mux = INIT_MT_CLK_MUX(REG_CLK_TSICLK_CFG, 0, 1, BIT_LOCK_TSI/*LOCK*/) },

	//DS
	{ "ds_des_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_DS_CFG, 5, BIT_LOCK_CRM_DS/*LOCK*/) },
	{ "ds_tdes_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_DS_CFG, 4, BIT_LOCK_CRM_DS/*LOCK*/) },
	{ "ds_aes_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_DS_CFG, 3, BIT_LOCK_CRM_DS/*LOCK*/) },
	{ "ds_csa3_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_DS_CFG, 2, BIT_LOCK_CRM_DS/*LOCK*/) },
	{ "ds_csa2_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_DS_CFG, 1, BIT_LOCK_CRM_DS/*LOCK*/) },
	{ "ds_sechd1_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_DS_CFG, 0, BIT_LOCK_CRM_DS/*LOCK*/) },

	//SEC
	{ "pvr_kl_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 17, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "crypto_des_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 9, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "crypto_tdes_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 8, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "crypto_aes_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 7, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "crypto_sha_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 6, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "crypto_rsa_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 5, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "sechd0_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 4, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "kt_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 3, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "kl_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 2, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "kdf_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 1, BIT_LOCK_CRM_SEC/*LOCK*/) },
	{ "otp_preload_clk", FLAG_CLK_GATE, 0, REG_CRM_SEC_LOCK, REG_CRM_SEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CRM_SEC_CFG, 0, BIT_LOCK_CRM_SEC/*LOCK*/) },

};

