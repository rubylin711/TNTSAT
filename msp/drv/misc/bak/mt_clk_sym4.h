/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk_sym4.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Symphony4 Clock definition.
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
#define REG_CRM_BASE		SYMPHONY_IO_VA(0xBF500000)
#define REG_AO_BASE			SYMPHONY_IO_VA(0xBF150000)
#else
/* Uboot */
#define REG_CRM_BASE		0xBF500000
#define REG_AO_BASE			0xBF150000
#endif

#define REG_MAC_CLKEN		(REG_CRM_BASE + 0x1C)
#define REG_MAC_CLKSEL		(REG_CRM_BASE + 0x50)
#define REG_MAC_SLOCK		(REG_CRM_BASE + 0x70)
#define REG_MAC_LOCK		(REG_CRM_BASE + 0x74)

#define REG_BUS_CLKEN		(REG_CRM_BASE + 0x8000)
#define REG_BUS_CLKSEL		(REG_CRM_BASE + 0x8004)
#define REG_BUS_SLOCK		(REG_CRM_BASE + 0x8018)
#define REG_BUS_LOCK		(REG_CRM_BASE + 0x801C)

#define REG_APCPU_CLKEN		(REG_CRM_BASE + 0x8100)
#define REG_APCPU_CLKSEL	(REG_CRM_BASE + 0x8104)
#define REG_APCPU_ARMPLL	(REG_CRM_BASE + 0x8108)
#define REG_APCPU_SLOCK		(REG_CRM_BASE + 0x8118)
#define REG_APCPU_LOCK		(REG_CRM_BASE + 0x811C)

#define REG_AVCPU_CLKEN		(REG_CRM_BASE + 0x8200)
#define REG_AVCPU_CLKSEL	(REG_CRM_BASE + 0x8204)
#define REG_AVCPU_SLOCK		(REG_CRM_BASE + 0x8218)
#define REG_AVCPU_LOCK		(REG_CRM_BASE + 0x821C)

#define REG_SYS_CLKEN		(REG_CRM_BASE + 0x8300)
#define REG_SYS_CLKSEL		(REG_CRM_BASE + 0x8304)
#define REG_SYS_SLOCK		(REG_CRM_BASE + 0x8318)
#define REG_SYS_LOCK		(REG_CRM_BASE + 0x831C)

#define REG_INTF_CLKEN		(REG_CRM_BASE + 0x9000)
#define REG_INTF_CLKSEL		(REG_CRM_BASE + 0x9004)
#define REG_INTF_SLOCK		(REG_CRM_BASE + 0x9018)
#define REG_INTF_LOCK		(REG_CRM_BASE + 0x901C)

#define REG_GRA_CLKEN		(REG_CRM_BASE + 0xA100)
#define REG_GRA_CLKSEL		(REG_CRM_BASE + 0xA104)
#define REG_GRA_SLOCK		(REG_CRM_BASE + 0xA118)
#define REG_GRA_LOCK		(REG_CRM_BASE + 0xA11C)

#define REG_JPG_CLKEN		(REG_CRM_BASE + 0xA200)
#define REG_JPG_CLKSEL		(REG_CRM_BASE + 0xA204)
#define REG_JPG_SLOCK		(REG_CRM_BASE + 0xA218)
#define REG_JPG_LOCK		(REG_CRM_BASE + 0xA21C)

#define REG_DISP_CLKEN		(REG_CRM_BASE + 0xA300)
#define REG_DISP_CLKSEL		(REG_CRM_BASE + 0xA304)
#define REG_DISP_SLOCK		(REG_CRM_BASE + 0xA318)
#define REG_DISP_LOCK		(REG_CRM_BASE + 0xA31C)

#define REG_VDEC_CLKEN		(REG_CRM_BASE + 0xA400)
#define REG_VDEC_CLKSEL		(REG_CRM_BASE + 0xA404)
#define REG_VDEC_SLOCK		(REG_CRM_BASE + 0xA418)
#define REG_VDEC_LOCK		(REG_CRM_BASE + 0xA41C)

#define REG_AOUT_CLKEN		(REG_CRM_BASE + 0xA500)
#define REG_AOUT_CLKSEL		(REG_CRM_BASE + 0xA504)
#define REG_AOUT_SLOCK		(REG_CRM_BASE + 0xA518)
#define REG_AOUT_LOCK		(REG_CRM_BASE + 0xA51C)

#define REG_VOUT_CLKEN		(REG_CRM_BASE + 0xA600)
#define REG_VOUT_CLKSEL		(REG_CRM_BASE + 0xA604)
#define REG_VOUT_SLOCK		(REG_CRM_BASE + 0xA618)
#define REG_VOUT_LOCK		(REG_CRM_BASE + 0xA61C)

#define REG_PNG_CLKEN		(REG_CRM_BASE + 0xA700)
#define REG_PNG_CLKSEL		(REG_CRM_BASE + 0xA704)
#define REG_PNG_SLOCK		(REG_CRM_BASE + 0xA718)
#define REG_PNG_LOCK		(REG_CRM_BASE + 0xA71C)

#define REG_DAI_CLKEN		(REG_CRM_BASE + 0xA800)
#define REG_DAI_SLOCK		(REG_CRM_BASE + 0xA818)
#define REG_DAI_LOCK		(REG_CRM_BASE + 0xA81C)

#define REG_TSI_CLKEN		(REG_CRM_BASE + 0xB000)
#define REG_TSI_CLKSEL		(REG_CRM_BASE + 0xB004)
#define REG_TSI_SLOCK		(REG_CRM_BASE + 0xB018)
#define REG_TSI_LOCK		(REG_CRM_BASE + 0xB01C)

#define REG_CI_CLKEN		(REG_CRM_BASE + 0xB100)
#define REG_CI_CLKSEL		(REG_CRM_BASE + 0xB104)
#define REG_CI_SLOCK		(REG_CRM_BASE + 0xB118)
#define REG_CI_LOCK			(REG_CRM_BASE + 0xB11C)

//Secure
#define REG_SECURE_CLKEN	(REG_CRM_BASE + 0xC000)
#define REG_SECURE_CLKSEL	(REG_CRM_BASE + 0xC004)
#define REG_SECURE_SLOCK	(REG_CRM_BASE + 0xC01C)
#define REG_SECURE_LOCK		(REG_CRM_BASE + 0xC018)

//IFCP
#define REG_IFCP_CLKEN		(REG_CRM_BASE + 0xC800)
#define REG_IFCP_CLKSEL		(REG_CRM_BASE + 0xC804)
#define REG_IFCP_SLOCK		(REG_CRM_BASE + 0xC818)
#define REG_IFCP_LOCK		(REG_CRM_BASE + 0xC81C)

//TOP!!!
#define REG_TOPCLK_CTRL6		(REG_CRM_BASE + 0xF818)
#define REG_TOPCLK_CTRL6_SLOCK	(REG_CRM_BASE + 0xF84C)
#define REG_TOPCLK_CTRL6_LOCK	(REG_CRM_BASE + 0xF850)
#define BIT_AVCPU_XTAL_SEL		19
#define BIT_APB_XTAL_SEL		18
#define BIT_CIPHER_XTAL_SEL		17
#define BIT_SECURE_XTAL_SEL		16
#define BIT_CI_XTAL_SEL			15
#define BIT_TSI_XTAL_SEL		14
#define BIT_IFCPCRYPTO_XTAL_SEL	13
#define BIT_PNG_XTAL_SEL		12
#define BIT_AOUT_XTAL_SEL		11
#define BIT_VDEC_XTAL_SEL		10
#define BIT_DISP_XTAL_SEL		9
#define BIT_JPG_XTAL_SEL		8
#define BIT_GRA_XTAL_SEL		7
#define BIT_DMA_XTAL_SEL		6
#define BIT_APBACKUP_XTAL_SEL	5

#define REG_AO_LPM_CLKEN		(REG_AO_BASE + 0x3800)
#define REG_AO_IRDA_CLKEN		(REG_AO_BASE + 0x3808)
#define REG_AO_LEDKB_CLKEN		(REG_AO_BASE + 0x3818)
#define REG_AO_GPIO_CLKEN		(REG_AO_BASE + 0x3820)
#define REG_AO_KADC_CLKEN		(REG_AO_BASE + 0x3828)
#define REG_AO_ANAREG_CLKEN		(REG_AO_BASE + 0x3830)
#define REG_AO_FPI2C_CLKEN		(REG_AO_BASE + 0x3838)
#define REG_AO_FPSPI_CLKEN		(REG_AO_BASE + 0x3840)
#define REG_AO_RECRAM_CLKEN		(REG_AO_BASE + 0x3848)
#define REG_AO_PINMUX_CLKEN		(REG_AO_BASE + 0x3850)
#define REG_AO_TIMER_CLKEN		(REG_AO_BASE + 0x3858)
#define REG_AO_MAILBOX_CLKEN	(REG_AO_BASE + 0x3868)
#define REG_AO_CEC_CLKEN		(REG_AO_BASE + 0x3870)
#define REG_AO_AVS_CLKEN		(REG_AO_BASE + 0x3878)
#define REG_AO_AGTIMER_CLKEN	(REG_AO_BASE + 0x3880)
#define REG_AO_MCU_CLKEN		(REG_AO_BASE + 0x3888)
#define REG_AO_XTAL_CLKSEL		(REG_AO_BASE + 0x3890)

#define REG_AOCRM_SLOCK			(REG_AO_BASE + 0x3894)
#define REG_AOCRM_LOCK			(REG_AO_BASE + 0x3898)

//<CN>¾§ÕñCLK
#define VAR_XTAL_CLK			(0xFFFFFFFF)
#define VAR_XTAL_CLK_HALF		(VAR_XTAL_CLK / 2)

#define VAR_VENC_OSCLK			(0xFFFFFFFC)

//No Lock Bit
#define NA_LOCK_BIT_IDX			(32)

/* Notice: the following rates are in unit of KHz. */

/////axi0_clk
static struct mt_clk_div_table axi0_div_table[] =
{
	{0, MT_DIV_2},		//default
	{1, MT_DIV_2_5},
	{2, MT_DIV_2_75},
	{3, MT_DIV_3},
	{4, MT_DIV_3_5},
	{5, MT_DIV_8},
	{0, 0}				//terminate with zero
};

/////axi1_clk
static struct mt_clk_rate_table axi1_rate_table[] =
{
	{0, 480000},	//reserved
	{1, 240000},
	{2, 120000},
	{3, 60000},		//default
	{4, 30000},
	{5, 444000},	//reserved
	{0, 0}			//terminate with zero
};

/////apb_clk
static struct mt_clk_rate_table apb_rate_table[] =
{
	{0, 80000},
	{1, 64000},
	{2, 90000},
	{3, 45000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}		//terminate with zero
};

/////ahb_clk
static struct mt_clk_rate_table ahb_rate_table[] =
{
	{0, 206000},
	{1, 240000},	//reserved
	{2, 144000},
	{3, 90000},
	{4, 60000},
	{5, 30000},
	{0, 0}			//terminate with zero
};

/////ap_backup_clk1
static struct mt_clk_rate_table apbackup1_rate_table[] =
{
	{0, 1280000},	//reserved
	{1, 1000000},
	{2, 810000},	//reserved
	{3, 576000},
	{4, 360000},
	{5, 60000},		//120M/60M, <CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////ap_backup_clk0
static struct mt_clk_rate_table apbackup0_rate_table[] =
{
	{0, 1440000},	//reserved
	{1, 1080000},
	{2, 960000},
	{3, 720000},	//default
	{4, 480000},
	{5, 240000},
	{0, 0}			//terminate with zero
};

/////at_clk
static struct mt_clk_div_table atclk_div_table[] =
{
	{0, MT_DIV_2},
	{1, MT_DIV_3},
	{2, MT_DIV_4},
	{3, MT_DIV_4},
	{0, 0}			//terminate with zero
};

/////armpll_clk
static struct mt_clk_div_table armpll_div_table[] =
{
	/* clk ref */
	//{8, },
	//{9, },

	/* vco/DIV */
	{10, MT_DIV_3},
	{11, MT_DIV_3},
	{12, MT_DIV_1},
	{13, MT_DIV_2},
	{14, MT_DIV_4},
	{15, MT_DIV_8},
	{0, 0}			//terminate with zero
};

/////avcpu_clk
static struct mt_clk_rate_table avcpu_rate_table[] =
{
	{0, 720000},	//reserved
	{1, 667000},	//from ephy pll
	{2, 576000},
	{3, 480000},
	{4, 360000},
	{5, 288000},
	{6, 180000},
	{7, 90000},		//90M/45M, <CN>Òþº¬Ñ¡Ôñ¾§Õñ
					//REG_TOPCLK_CTRL6: bit 22
	{0, 0}			//terminate with zero
};

/////dma_clk
static struct mt_clk_rate_table dma_rate_table[] =
{
	{0, 262000},	//reserved
	{1, 288000},	//reserved
	{2, 131000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{3, 206000},
	{0, 0}			//terminate with zero
};

/////sdio_clk
static struct mt_clk_rate_table sdio_rate_table[] =
{
	{0, 100000},	//from ephy pll
	{1, 90000},
	{0, 0}			//terminate with zero
};

/////pnand_clk
static struct mt_clk_rate_table pnand_rate_table[] =
{
	{0, VAR_XTAL_CLK},	//24M/27M/20M
						//NOTE: 40M should div 2 to 20M
	{1, 288000},
	{2, 262000},
	{3, 200000},		//from ephy pll
	{0, 0}				//terminate with zero
};

/////spi_clk
static struct mt_clk_rate_table spi_rate_table[] =
{
	{0, VAR_XTAL_CLK},	//24M/27M/20M
						//NOTE: 40M should div 2 to 20M
	{1, 360000},
	{2, 288000},
	{3, 400000},		//from ephy pll
	{0, 0}				//terminate with zero
};

/////uart_clk
static struct mt_clk_rate_table uart_rate_table[] =
{
	{0, VAR_XTAL_CLK},	//24M/27M/20M
						//NOTE: 40M should div 2 to 20M
	{1, 120000},
	{0, 0}				//terminate with zero
};

/////smc_clk
static struct mt_clk_rate_table smc_rate_table[] =
{
	{0, 90000},
	{1, VAR_XTAL_CLK_HALF},
	{0, 0}					//terminate with zero
};

/////gra_clk
static struct mt_clk_rate_table gra_rate_table[] =
{
	{0, 262000},
	{1, 360000},
	{2, 444000},	//reserved from ephy pll
	{3, 320000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////jpg_clk
static struct mt_clk_rate_table jpg_rate_table[] =
{
	{0, 262000},	//reserved
	{1, 206000},
	{2, 144000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{3, 288000},	//reserved
	{0, 0}			//terminate with zero
};

/////disposdc_clk
static struct mt_clk_rate_table disposdc_rate_table[] =
{
	{0, 206000},
	{1, 250000},	//from ephy pll
	{3, 144000},
	{4, 80000},
	{5, 60000},		//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////dispdi_clk
static struct mt_clk_rate_table dispdi_rate_table[] =
{
	{0, 288000},
	{1, 262000},
	{2, 160000},
	{3, 131000},
	{4, 80000},
	{5, 60000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}		//terminate with zero
};

/////dispcore_clk
static struct mt_clk_rate_table dispcore_rate_table[] =
{
	{1, 206000},
	{2, 180000},
	{3, 131000},
	{4, 40000},
	{5, 60000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}		//terminate with zero
};

/////vdec_clk
static struct mt_clk_rate_table vdec_rate_table[] =
{
	{0, 262000},
	{1, 206000},
	{2, 131000},
	{3, 45000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}		//terminate with zero
};

/////aout_clk
static struct mt_clk_rate_table aout_rate_table[] =
{
	{0, 288000},
	{1, 262000},
	{2, 206000},
	{3, 131000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////lcd_cclk
static struct mt_clk_div_table lcd_div_table[] =
{
	{2, MT_DIV_2},
	{4, MT_DIV_4},
	{6, MT_DIV_6},
	{0, 0}			//terminate with zero
};

/////hdvenc_clk
static struct mt_clk_div_table hdvenc_div_table[] =
{
	{0, MT_DIV_1},
	{1, MT_DIV_1},
	{2, MT_DIV_1},
	{3, MT_DIV_1},
	{4, MT_DIV_2},
	{5, MT_DIV_4},
	{6, MT_DIV_6},
	{7, MT_DIV_8},
	{0, 0}			//terminate with zero
};

/////lcd2x_clk
static struct mt_clk_rate_table lcd2x_rate_table[] =
{
	{0, 64000},
	{1, 72000},
	{2, 80000},
	{3, 131000},
	{4, 138000},
	{5, 54000},
	{6, VAR_VENC_OSCLK},	//venc_osclk, max 148.5M
	{0, 0}					//terminate with zero
};

/////png_clk
static struct mt_clk_rate_table png_rate_table[] =
{
	{0, 262000},	//reserved
	{1, 206000},
	{2, 144000},	//Òþº¬Ñ¡Ôñ¾§Õñ
	{3, 288000},	//reserved
	{0, 0}			//terminate with zero
};

/////tsi_clk
static struct mt_clk_rate_table tsi_rate_table[] =
{
	{0, 288000},	//reserved
	{1, 262000},
	{2, 206000},
	{3, 131000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////ci_clk
static struct mt_clk_rate_table ci_rate_table[] =
{
	{0, 80000},
	{1, 120000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////m2m_clk
static struct mt_clk_rate_table m2m_rate_table[] =
{
	{0, 262000},
	{1, 240000},
	{2, 206000},
	{3, 131000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////secure_clk
static struct mt_clk_rate_table secure_rate_table[] =
{
	{0, 262000},
	{1, 206000},
	{2, 240000},
	{3, 131000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{0, 0}			//terminate with zero
};

/////ifcp_crypto_clk
static struct mt_clk_rate_table ifcp_crypto_rate_table[] =
{
	{0, 262000},	//reserved
	{1, 206000},
	{2, 120000},	//<CN>Òþº¬Ñ¡Ôñ¾§Õñ
	{3, 240000},
	{0, 0}			//terminate with zero
};

/////ifcp_sys_clk
static struct mt_clk_rate_table ifcp_sys_rate_table[] =
{
	{2, 296300},	//reserved, 296.3M
	{3, 288000},
	{4, 240000},
	{5, 180000},
	{6, 90000},
	{0, 0}			//terminate with zero
};

/////xtal_clk
static struct mt_clk_rate_table xtal_rate_table[] =
{
	{0, 27000},
	{1, 24000},
	{2, 40000},
	{3, 40000},
	{0, 0}			//terminate with zero
};

//---------------------------------------------------------------------------//

static struct mt_clk mt_clk_table[] =
{
	//MAC
	{ MT_RMII_CLK, FLAG_CLK_GATE, 0, REG_MAC_SLOCK, REG_MAC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_MAC_CLKEN, 5, 15/*LOCK*/) },
	{ MT_MAC_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX | FLAG_CLK_ATTR, 0, REG_MAC_SLOCK, REG_MAC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_MAC_CLKEN, 4, 14/*LOCK*/),
	  //"mac_clkmode"
	  //0: MII
	  //1: RMII, 50M
	  .mux = INIT_MT_CLK_MUX(REG_MAC_CLKSEL, 8, 1, 17/*LOCK*/),
	  /* "mac_clksel" */
	  //0: IO clk
	  //1: IO clk reverse
	  //2: 50MHz
	  //3: 50MHz reverse
	  .attr = INIT_MT_CLK_ATTR(REG_MAC_CLKSEL, 0, 2, 16/*LOCK*/) },

	//BUS
	{ "axireg_clk", FLAG_CLK_GATE, 0, REG_BUS_SLOCK, REG_BUS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_BUS_CLKEN, 2, 2/*LOCK*/) },
	{ "axidebug_clk", FLAG_CLK_GATE, 0, REG_BUS_SLOCK, REG_BUS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_BUS_CLKEN, 1, 1/*LOCK*/) },
	{ "axi_clk", FLAG_CLK_GATE | FLAG_CLK_MUX, 0, REG_BUS_SLOCK, REG_BUS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_BUS_CLKEN, 0, 0/*LOCK*/),
	  //0: axi0 clk
	  //1: axi1 clk
	  .mux = INIT_MT_CLK_MUX(REG_BUS_CLKSEL, 14, 1, 6/*LOCK*/) },
	{ "axi1_clk", FLAG_CLK_RATE, 0, REG_BUS_SLOCK, REG_BUS_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_BUS_CLKSEL, 11, 3, axi1_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "axi0_clk", FLAG_CLK_DIVIDER, 0, REG_BUS_SLOCK, REG_BUS_LOCK,
	  /* divide from ddrphy clk */
	  .divider = INIT_MT_CLK_DIV(REG_BUS_CLKSEL, 8, 3, axi0_div_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ MT_APB_CLK, FLAG_CLK_RATE, 0, REG_BUS_SLOCK, REG_BUS_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_BUS_CLKSEL, 6, 2, apb_rate_table, 5/*LOCK*/) },
	{ MT_AHB_CLK, FLAG_CLK_RATE, 0, REG_BUS_SLOCK, REG_BUS_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_BUS_CLKSEL, 3, 3, ahb_rate_table, 4/*LOCK*/) },

	//PLL_TEST: TODO

	//APCPU
	/* APCPU apb */
	{ "apcpu_pclk", FLAG_CLK_GATE, 0, REG_APCPU_SLOCK, REG_APCPU_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_APCPU_CLKEN, 1, 1/*LOCK*/) },
	/* APCPU axi&ahb */
	{ MT_APCPU_CLK, FLAG_CLK_GATE | FLAG_CLK_MUX | FLAG_CLK_ATTR, 0, REG_APCPU_SLOCK, REG_APCPU_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_APCPU_CLKEN, 0, 0/*LOCK*/),
	  //"backup_armpll_clksel"
	  //0: backup
	  //1: arm pll
	  .mux = INIT_MT_CLK_MUX(REG_APCPU_CLKSEL, 7, 1, NA_LOCK_BIT_IDX/*LOCK*/),
	  //"apcpu_clk_switch_mask"
	  //0: switch directly
	  //1: wait apcpu idle to switch clk
	  .attr = INIT_MT_CLK_ATTR(REG_APCPU_CLKSEL, 12, 1, NA_LOCK_BIT_IDX/*LOCK*/) },
	//"ap_backup_clksel"
	//0: ap_backup_clk0
	//1: ap_backup_clk1
	{ "apbackup_mux", FLAG_CLK_MUX, 0, REG_APCPU_SLOCK, REG_APCPU_LOCK,
	  .mux = INIT_MT_CLK_MUX(REG_APCPU_CLKSEL, 3, 1, 2/*LOCK*/) },
	{ "apbackup_clk1", FLAG_CLK_RATE, 0, REG_APCPU_SLOCK, REG_APCPU_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_APCPU_CLKSEL, 9, 3, apbackup1_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	{ "apbackup_clk0", FLAG_CLK_RATE, 0, REG_APCPU_SLOCK, REG_APCPU_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_APCPU_CLKSEL, 0, 3, apbackup0_rate_table, NA_LOCK_BIT_IDX/*LOCK*/) },
	/* DIV from vco */
	/* vco = freq_xtal / (2 + bit[11:8]) * (32 + bit[23:16]) * (1 + bit[12]) */
	{ "armpll_clk", FLAG_CLK_DIVIDER, 0, REG_APCPU_SLOCK, REG_APCPU_LOCK,
	  .divider = INIT_MT_CLK_DIV(REG_APCPU_ARMPLL, 4, 4, armpll_div_table, 4/*LOCK*/) },
	//amba test clk = cpu_clk / DIV
	{ "amba_test_clk", FLAG_CLK_DIVIDER | FLAG_CLK_ATTR, 0, REG_APCPU_SLOCK, REG_APCPU_LOCK,
	  .divider = INIT_MT_CLK_DIV(REG_APCPU_CLKSEL, 4, 2, atclk_div_table, 3/*LOCK*/),
	  //"atclk_cfg_freq_update"
	  .attr = INIT_MT_CLK_ATTR(REG_APCPU_CLKSEL, 6, 1, NA_LOCK_BIT_IDX/*LOCK*/) },

	//AVCPU
	{ MT_AVCPU_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_ATTR, 0, REG_AVCPU_SLOCK, REG_AVCPU_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AVCPU_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_AVCPU_CLKSEL, 0, 3, avcpu_rate_table, 1/*LOCK*/),
	  //"avcpu_clk_switch_mask"
	  //0: switch directly
	  //1: wait avcpu idle to switch
	  .attr = INIT_MT_CLK_ATTR(REG_AVCPU_CLKSEL, 3, 1, NA_LOCK_BIT_IDX/*LOCK*/) },

	//SYS
	{ "sw_twm_clk", FLAG_CLK_GATE, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SYS_CLKEN, 5, 5/*LOCK*/) },
	{ MT_DMA_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SYS_CLKEN, 4, 4/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_SYS_CLKSEL, 0, 2, dma_rate_table, 9/*LOCK*/) },
	{ "ddrmcregs_clk", FLAG_CLK_GATE, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SYS_CLKEN, 3, 3/*LOCK*/) },
	{ "ddrmccore_clk", FLAG_CLK_GATE, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SYS_CLKEN, 2, 2/*LOCK*/) },
	{ "ddrmcahb_clk", FLAG_CLK_GATE, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SYS_CLKEN, 1, 1/*LOCK*/) },
	{ "ddrmcaxi_clk", FLAG_CLK_GATE, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SYS_CLKEN, 0, 0/*LOCK*/) },
	{ "ddr_d3_duty_sel", FLAG_CLK_ATTR, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  //0: 50% d3
	  //1: !50% d3
	  .attr = INIT_MT_CLK_ATTR(REG_SYS_CLKSEL, 3, 1, 11/*LOCK*/) },
	{ "ddrpll_pnclk_clksel", FLAG_CLK_EDGE, 0, REG_SYS_SLOCK, REG_SYS_LOCK,
	  //0: positive edge
	  //1: negative edge
	  .edge = INIT_MT_CLK_EDGE(REG_SYS_CLKSEL, 2, 1, 10/*LOCK*/) },

	//INTF
	{ MT_SDIO1_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 12, 12/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 8, 1, sdio_rate_table, 18/*LOCK*/) },
	{ MT_SDIO0_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 11, 11/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 7, 1, sdio_rate_table, 17/*LOCK*/) },
	{ MT_PNAND_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 10, 10/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 5, 2, pnand_rate_table, 16/*LOCK*/) },
	{ MT_SPI1_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 9, 9/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 3, 2, spi_rate_table, 15/*LOCK*/) },
	{ MT_SPI0_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 8, 8/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 1, 2, spi_rate_table, 14/*LOCK*/) },
	{ MT_UART1_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 7, 7/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 10, 1, uart_rate_table, 20/*LOCK*/) },
	{ MT_UART0_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 6, 6/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 9, 1, uart_rate_table, 19/*LOCK*/) },
	{ MT_I2CDEBUG_CLK, FLAG_CLK_GATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 5, 5/*LOCK*/) },
	{ MT_I2C1_CLK, FLAG_CLK_GATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 4, 4/*LOCK*/) },
	{ MT_I2C0_CLK, FLAG_CLK_GATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 3, 3/*LOCK*/) },
	{ MT_USB1_CLK, FLAG_CLK_GATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 2, 2/*LOCK*/) },
	{ MT_USB0_CLK, FLAG_CLK_GATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 1, 1/*LOCK*/) },
	{ MT_SMC0_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_INTF_SLOCK, REG_INTF_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_INTF_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_INTF_CLKSEL, 0, 1, smc_rate_table, 13/*LOCK*/) },

	//GRA
	{ MT_GRA_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_GRA_SLOCK, REG_GRA_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_GRA_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_GRA_CLKSEL, 0, 2, gra_rate_table, 1/*LOCK*/) },

	//JPG
	{ MT_JPG_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_JPG_SLOCK, REG_JPG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_JPG_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_JPG_CLKSEL, 0, 2, jpg_rate_table, 1/*LOCK*/) },

	//DISP
	{ MT_DISP_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_DISP_SLOCK, REG_DISP_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_DISP_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_DISP_CLKSEL, 0, 3, dispcore_rate_table, 1/*LOCK*/) },
	{ "disposdc_clk", FLAG_CLK_RATE, 0, REG_DISP_SLOCK, REG_DISP_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_DISP_CLKSEL, 6, 3, disposdc_rate_table, 3/*LOCK*/) },
	{ MT_DI_CLK, FLAG_CLK_RATE, 0, REG_DISP_SLOCK, REG_DISP_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_DISP_CLKSEL, 3, 3, dispdi_rate_table, 2/*LOCK*/) },

	//VDEC
	{ MT_VDEC_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_VDEC_SLOCK, REG_VDEC_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_VDEC_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_VDEC_CLKSEL, 0, 2, vdec_rate_table, 1/*LOCK*/) },

	//AOUT
	{ MT_SPDIF_CLK, FLAG_CLK_GATE, 0, REG_AOUT_SLOCK, REG_AOUT_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AOUT_CLKEN, 1, 1/*LOCK*/) },
	{ MT_AOUT_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_AOUT_SLOCK, REG_AOUT_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AOUT_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_AOUT_CLKSEL, 0, 2, aout_rate_table, 2/*LOCK*/) },
	{ "mclk_clk", FLAG_CLK_MUX, 0, REG_AOUT_SLOCK, REG_AOUT_LOCK,
	  //0: dig_mclk
	  //1: audio_dac_clk
	  .mux = INIT_MT_CLK_MUX(REG_AOUT_CLKSEL, 2, 1, 3/*LOCK*/) },

	//VOUT
	{ MT_HDMI_CLK, FLAG_CLK_GATE, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_VOUT_CLKEN, 4, 4/*LOCK*/) },
	{ MT_LCD_CLK, FLAG_CLK_GATE, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_VOUT_CLKEN, 3, 3/*LOCK*/) },
	{ "lcd_cclk", FLAG_CLK_DIVIDER, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  /* divide from "lcd2x_clk" */
	  .divider = INIT_MT_CLK_DIV(REG_VOUT_CLKSEL, 7, 3, lcd_div_table, 8/*LOCK*/) },
	{ "lcdhd_clk", FLAG_CLK_MUX, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  //0: hdvenc_hdclk
	  //1: lcd_cclk
	  .mux = INIT_MT_CLK_MUX(REG_VOUT_CLKSEL, 6, 1, 7/*LOCK*/) },
	{ "lcd2x_clk", FLAG_CLK_RATE, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_VOUT_CLKSEL, 3, 3, lcd2x_rate_table, 6/*LOCK*/) },
	{ MT_HVENC_CLK, FLAG_CLK_GATE | FLAG_CLK_DIVIDER, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_VOUT_CLKEN, 2, 2/*LOCK*/),
	  /* divide from "venc_osclk" */
	  .divider = INIT_MT_CLK_DIV(REG_VOUT_CLKSEL, 0, 3, hdvenc_div_table, 5/*LOCK*/) },
	{ MT_VBI_CLK, FLAG_CLK_GATE, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_VOUT_CLKEN, 1, 1/*LOCK*/) },
	{ MT_SVENC_CLK, FLAG_CLK_GATE, 0, REG_VOUT_SLOCK, REG_VOUT_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_VOUT_CLKEN, 0, 0/*LOCK*/) },

	//PNG
	{ MT_PNG_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_PNG_SLOCK, REG_PNG_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_PNG_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_PNG_CLKSEL, 0, 2, png_rate_table, 1/*LOCK*/) },

	//DAI
	{ MT_DAI_CLK, FLAG_CLK_GATE, 0, REG_DAI_SLOCK, REG_DAI_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_DAI_CLKEN, 0, 0/*LOCK*/) },

	//TSI
	{ MT_TSI_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_TSI_SLOCK, REG_TSI_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_TSI_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_TSI_CLKSEL, 0, 2, tsi_rate_table, 1/*LOCK*/) },
	{ MT_TS3_CLK, FLAG_CLK_MUX, 0, REG_TSI_SLOCK, REG_TSI_LOCK,
	  //0: pad_ts3_clk
	  //1: cicam_tsout_clk
	  //2: pad_ts3_clk reverse
	  //3: cicam_tsout_clk reverse
	  .mux = INIT_MT_CLK_MUX(REG_TSI_CLKSEL, 7, 2, 5/*LOCK*/) },
	{ MT_TS2_CLK, FLAG_CLK_MUX, 0, REG_TSI_SLOCK, REG_TSI_LOCK,
	  //0: pad_ts2_clk
	  //1: democ_ts2_clk
	  //2: pad_ts2_clk reverse
	  //3: democ_ts2_clk reverse
	  .mux = INIT_MT_CLK_MUX(REG_TSI_CLKSEL, 5, 2, 4/*LOCK*/) },
	{ MT_TS1_CLK, FLAG_CLK_MUX, 0, REG_TSI_SLOCK, REG_TSI_LOCK,
	  //0: pad_ts1_clk
	  //1: demos_ts1_clk
	  //2: pad_ts1_clk reverse
	  //3: demos_ts1_clk reverse
	  .mux = INIT_MT_CLK_MUX(REG_TSI_CLKSEL, 3, 2, 3/*LOCK*/) },
	{ MT_TS0_CLK, FLAG_CLK_ATTR, 0, REG_TSI_SLOCK, REG_TSI_LOCK,
	  //0: pad_ts0_clk
	  //1: pad_ts0_clk reverse
	  .attr = INIT_MT_CLK_ATTR(REG_TSI_CLKSEL, 2, 1, 2/*LOCK*/) },

	//CI
	{ MT_CI_CLK, FLAG_CLK_GATE | FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_CI_SLOCK, REG_CI_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_CI_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_CI_CLKSEL, 0, 1, ci_rate_table, 1/*LOCK*/),
	  //"citsin_clksel"
	  //0: pad_ts0_clk
	  //1: pad_ts1_clk
	  //2: demos_ts1_clk
	  //3: democ_ts2_clk
	  //4: pad_ts0_clk reverse
	  //5: pad_ts1_clk reverse
	  //6: demos_ts1_clk reverse
	  //7: democ_ts2_clk reverse
	  .mux = INIT_MT_CLK_MUX(REG_CI_CLKSEL, 1, 3, 2/*LOCK*/) },

	//Secure
	{ "m2m_clk", FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 7, 7/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_SECURE_CLKSEL, 2, 2, m2m_rate_table, 9/*LOCK*/) },
	{ "pka_clk", FLAG_CLK_GATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 6, 6/*LOCK*/) },
	{ "glitchdet_clk", FLAG_CLK_GATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 5, 5/*LOCK*/) },
	/* key table */
	{ "kt_clk", FLAG_CLK_GATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 4, 4/*LOCK*/) },
	{ "sechd0_clk", FLAG_CLK_GATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 3, 3/*LOCK*/) },
	/* key ladder */
	{ "kl_clk", FLAG_CLK_GATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 2, 2/*LOCK*/) },
	/* key derivation */
	{ "kdf_clk", FLAG_CLK_GATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 1, 1/*LOCK*/) },
	{ "secmisc_clk", FLAG_CLK_GATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_SECURE_CLKEN, 0, 0/*LOCK*/) },
	{ "secure_clk", FLAG_CLK_RATE, 0, REG_SECURE_SLOCK, REG_SECURE_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_SECURE_CLKSEL, 0, 2, secure_rate_table, 8/*LOCK*/) },

	//IFCP
	{ "ifcp_klm_clk", FLAG_CLK_GATE, 0, REG_IFCP_SLOCK, REG_IFCP_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_IFCP_CLKEN, 1, 1/*LOCK*/) },
	{ "ifcp_crypto_clk", FLAG_CLK_GATE | FLAG_CLK_RATE, 0, REG_IFCP_SLOCK, REG_IFCP_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_IFCP_CLKEN, 0, 0/*LOCK*/),
	  .rate = INIT_MT_CLK_RATE(REG_IFCP_CLKSEL, 3, 2, ifcp_crypto_rate_table, 3/*LOCK*/) },
	{ "ifcp_sys_clk", FLAG_CLK_RATE, 0, REG_IFCP_SLOCK, REG_IFCP_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_IFCP_CLKSEL, 0, 3, ifcp_sys_rate_table, 2/*LOCK*/) },
	{ "vscpu_clk", FLAG_CLK_ATTR, 0, REG_IFCP_SLOCK, REG_IFCP_LOCK,
	  //0: switch directly
	  //1: wait vscpu idle to switch
	  .attr = INIT_MT_CLK_ATTR(REG_IFCP_CLKSEL, 5, 1, NA_LOCK_BIT_IDX/*LOCK*/) },

	//TOP: TODO
	//<CN>TOP¶¥²ãÊ±ÖÓ¿ª¹Ø,Ã¿bitÎªÒ»¸öÊ±ÖÓµÄ¿ª¹Ø
	//<CN>Ñ¡ÔñÊÇ·ñÓÃ¾§ÕñÆµÂÊ
	//<CN>1: ÊÇ
	//<CN>0: ·ñ
	//TOPCLK_CTRL6: [19:5]
	{ "avcpu_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_AVCPU_XTAL_SEL, BIT_AVCPU_XTAL_SEL/*LOCK*/) },
	{ "apb_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_APB_XTAL_SEL, BIT_APB_XTAL_SEL/*LOCK*/) },
	{ "cipher_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_CIPHER_XTAL_SEL, BIT_CIPHER_XTAL_SEL/*LOCK*/) },
	{ "secure_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_SECURE_XTAL_SEL, BIT_SECURE_XTAL_SEL/*LOCK*/) },
	{ "ci_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_CI_XTAL_SEL, BIT_CI_XTAL_SEL/*LOCK*/) },
	{ "tsi_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_TSI_XTAL_SEL, BIT_TSI_XTAL_SEL/*LOCK*/) },
	{ "ifcpcrypto_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_IFCPCRYPTO_XTAL_SEL, BIT_IFCPCRYPTO_XTAL_SEL/*LOCK*/) },
	{ "png_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_PNG_XTAL_SEL, BIT_PNG_XTAL_SEL/*LOCK*/) },
	{ "aout_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_AOUT_XTAL_SEL, BIT_AOUT_XTAL_SEL/*LOCK*/) },
	{ "vdec_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_VDEC_XTAL_SEL, BIT_VDEC_XTAL_SEL/*LOCK*/) },
	{ "disp_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_DISP_XTAL_SEL, BIT_DISP_XTAL_SEL/*LOCK*/) },
	{ "jpg_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_JPG_XTAL_SEL, BIT_JPG_XTAL_SEL/*LOCK*/) },
	{ "gra_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_GRA_XTAL_SEL, BIT_GRA_XTAL_SEL/*LOCK*/) },
	{ "dma_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_DMA_XTAL_SEL, BIT_DMA_XTAL_SEL/*LOCK*/) },
	{ "apbackup_xtal_sel", FLAG_CLK_XTAL_SEL, 0, REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK,
	  .xtal_sel = INIT_MT_CLK_XTAL_SEL(REG_TOPCLK_CTRL6, BIT_APBACKUP_XTAL_SEL, BIT_APBACKUP_XTAL_SEL/*LOCK*/) },

	//AO(AlwaysOn)
	{ MT_AOLPM_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_LPM_CLKEN, 0, 0/*LOCK*/) },
	{ MT_IRDA_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_IRDA_CLKEN, 0, 1/*LOCK*/) },
	{ MT_LEDKB_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_LEDKB_CLKEN, 0, 3/*LOCK*/) },
	{ MT_AOGPIO_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_GPIO_CLKEN, 0, 4/*LOCK*/) },
	{ MT_KADC_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_KADC_CLKEN, 0, 5/*LOCK*/) },
	{ MT_AOANA_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_ANAREG_CLKEN, 0, 6/*LOCK*/) },
	{ MT_FPI2C_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_FPI2C_CLKEN, 0, 7/*LOCK*/) },
	{ MT_FPSPI_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_FPSPI_CLKEN, 0, 8/*LOCK*/) },
	{ MT_RECRAM_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_RECRAM_CLKEN, 0, 9/*LOCK*/) },
	{ MT_AOPINMUX_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_PINMUX_CLKEN, 0, 10/*LOCK*/) },
	{ MT_TIMER_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_TIMER_CLKEN, 0, 11/*LOCK*/) },
	{ MT_MAILBOX_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_MAILBOX_CLKEN, 0, 13/*LOCK*/) },
	{ MT_CEC_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_CEC_CLKEN, 0, 14/*LOCK*/) },
	{ "avs_clk", FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_AVS_CLKEN, 0, 15/*LOCK*/) },
	{ "agtimer_clk", FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_AGTIMER_CLKEN, 0, 16/*LOCK*/) },
	{ MT_AOMCU_CLK, FLAG_CLK_GATE, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .gate = INIT_MT_CLK_GATE(REG_AO_MCU_CLKEN, 0, 17/*LOCK*/) },
	{ MT_XTAL_CLK, FLAG_CLK_RATE | FLAG_CLK_MUX, 0, REG_AOCRM_SLOCK, REG_AOCRM_LOCK,
	  .rate = INIT_MT_CLK_RATE(REG_AO_XTAL_CLKSEL, 1, 2, xtal_rate_table, NA_LOCK_BIT_IDX/*LOCK*/), /* Read Only */
	  //0: <CN>¾§Õñclk
	  //1: security osc
	  .mux = INIT_MT_CLK_MUX(REG_AO_XTAL_CLKSEL, 0, 1, 19/*LOCK*/) },

	//TODO: add your clk here

};

