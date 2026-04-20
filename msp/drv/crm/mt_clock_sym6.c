/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_clock_sym6.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT Symphony6 digital clocks definition.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 * 2.Date         : 2024/05/09
 *   Modification : Update to CRM datasheet v1.51
 *
 *****************************************************************************/
#ifdef __UBOOT__
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/delay.h>
#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"
#else
/* RTOS */
#include <string.h>
#include <sys_types.h>
#include <sys_define.h>

#include "mtos_misc.h"

#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"

#define udelay 			mtos_task_delay_us
#endif

#include "mt_clock.h"
#include "mt_crm_clock_reg.h"
#include "mt_crm_top_reg.h"
#include "mt_crm_lock_reg.h"
#include "mt_crm_reset_reg.h"

#include "mt_clock_top.h"
#include "mt_mod_internal.h"

/* for optimize size */
#define CFG_CLOCK_OPT_SIZE

/* not use ephypll */
#define CFG_EPHYPLL_UNUSED

/*
 * Comments:
 *    venc_osclk: clkgen_vhdintp_reg <3:1> "clk os sel"
 *
 *    audio_mclk_hdmi(o_spdf_mclk/o_aout_mclk): see Audio datasheet - "spdif_path_sel"
 *      o_spdf_mclk
 *			        > audio_mclk_hdmi
 *      o_aout_mclk
 *
 *    i_spdf_mclk/o_spdf_mclk?
 *    aout_mclk/o_aout_mclk?
 *
 *                              i_spdf_mclk -> o_spdf_mclk
 *    aout_clk("aout_clksel") <
 *                              aout_mclk   -> o_aout_mclk
 *                              (i_dig_mclk)
 *
 *    i_spdf_mclk, aout_mclk(i_dig_mclk): see Audio datasheet
 *
 *    spdf_digdiv_mclk:
 *      spdf_audiopll_clk
 *                        > spdf_digdiv_mclk
 *        spdf_digdiv_clk
 *
 *	PLL:
 *    clk_usbpll: 2880MHz
 *    clk_usb30pll: 2500MHz
 *    ddrphy: ddr_pll / 2, 2080 / 2 = 1040MHz
 *    clk_vsdpll: 3240MHz
 *    clk_vhdpll: 2970MHz
 *    ephypll(ethpll): 2000MHz
 *    clk_audiopll: clkgen_usbaclk_reg2 <11:0>, 98.343MHz (Dong Qingxiang)
 *
 * TODO:
 *   ana_hdmi_clk_tmds: "hdmi_clk_tmds", <CN>Óë"venc_osclk"Ë³Ðò¶ÔÆë (Dong Qingxiang)
 *     => "hdmitx_clk"?
 *   i_spdf_mclk
 *   aout_mclk
 */


/** Hide internal clocks */
#define ANA_HDMI_CLK_TMDS		"ana_hdmi_clk_tmds"
#define I_SPDF_MCLK				"i_spdf_mclk"

//---------------------------------------------------------------------------//

#if !defined(CONFIG_TARGET_SYMPHONY6_LITE) && !defined(CONFIG_TARGET_SYMPHONY6_MINI)

/* mac mux */
static struct mt_clk_map_table g_mac_mux[] =
{
	{0, 0, "mii_clk"},		/* MII */
	{1, 1, MT_CLK_RMII},	/* RMII */
	{-1, -1}
};

#if 0
/* axi divider */
static struct mt_clk_map_table g_axi_div[] =
{
	{0, 700, MT_USBPLL},		/* clk_usbpll_d7, 411.4MHz, reserved */
	//{1, 250, MT_CLK_DDRPHY},	/* ddrphy 1/2.5 */							/* A1 RM */
	{2, 275, MT_CLK_DDRPHY},	/* ddrphy 1/2.75 */
	//{3, 300, MT_CLK_DDRPHY},	/* ddrphy 1/3 */							/* A1 RM */
	//{4, 350, MT_CLK_DDRPHY},	/* ddrphy 1/3.5 */							/* A1 RM */
	{5, 750, MT_USBPLL},		/* clk_usbpll_d7p5, 384MHz */
	{6, 800, MT_USBPLL},		/* clk_usbpll_d8, 360MHz */
	{7, 100, MT_CLK_XTAL},		/* Xtal, 24/27MHz */
	{-1, -1}
};
#else
/* axi rate */
static struct mt_clk_map_table g_axi_rate[] =
{
	{0, 411400000},				/* clk_usbpll_d7, 411.4MHz, reserved */
	{2, 0, "ddrphy_clk_d2p75"},
	{5, 384000000},				/* clk_usbpll_d7p5, 384MHz */
	{6, 360000000},				/* clk_usbpll_d8, 360MHz */
	{7, 0, MT_CLK_XTAL},		/* xtal 24M/27M */
	{-1, -1}
};
#endif

/* apb rate */
static struct mt_clk_map_table g_apb_rate[] =
{
	{0, 80000000},
	//{1, 64000000},			/* A1 RM */
	{2, 90000000},
	{3, 0, MT_CLK_XTAL},		/* xtal 24M/27M */
	{-1, -1}
};

/* ahb rate */
static struct mt_clk_map_table g_ahb_rate[] =
{
	{0, 206000000},
	{1, 240000000},				/* reserved */
	//{2, 144000000},			/* A1 RM */
	//{3, 90000000},			/* A1 RM */
	{4, 180000000},
	{5, 0, MT_CLK_XTAL},		/* xtal 24M/27M */
	{-1, -1}
};

/* apcpu mux */
static struct mt_clk_map_table g_apcpu_mux[] =
{
	{0, 0, MT_CLK_APBACKUP},	/* apbackup_clk */
	{1, 1, MT_ARMPLL},			/* armpll */
	{-1, -1}
};

/* apbackup rate */
static struct mt_clk_map_table g_apbackup_rate[] =
{
	//{0, 1620000000},			/* clk_vsdpll_d2 */				/* A1 RM */
	{1, 1440000000},			/* clk_usbpll_d2 */
	{2, 1280000000},			/* clk_usbpll_d2p25 */
#ifndef CFG_EPHYPLL_UNUSED
	{3, 1000000000},			/* clk_ethpll_d2 */
#endif
	{4, 960000000},				/* clk_usbpll_d3 */
#ifndef CFG_EPHYPLL_UNUSED
	{5, 800000000},				/* clk_ethpll_d2p5 */
#endif
	{6, 720000000},				/* clk_usbpll_d4, default */
	//{7, 576000000},				/* clk_usbpll_d5 */			/* A1 RM */
	//{8, 480000000},				/* clk_usbpll_d6 */			/* A1 RM */
	{9, 240000000},				/* clk_usbpll_d12 */
	//{10, 24000000},				/* clk_usbpll_d120 */		/* A1 RM */
	{11, 0, MT_CLK_XTAL},		/* xtal 24M/27M */
	{-1, -1}
};

/* avcpu rate */
static struct mt_clk_map_table g_avcpu_rate[] =
{
#ifndef CFG_EPHYPLL_UNUSED
	{0, 667000000},		/* clk_ethpll_d3 */
#endif
	//{1, 810000000},		/* clk_vsdpll_d4, reserved */		/* A1 RM */
	{2, 720000000},		/* clk_usbpll_d4, MAX */
	{3, 576000000},		/* clk_usbpll_d5 */
	{4, 480000000},		/* clk_usbpll_d6 */
	{5, 360000000},		/* clk_usbpll_d8, default */
	//{6, 288000000},		/* clk_usbpll_d10 */				/* A1 RM */
	//{7, 120000000},		/* clk_usbpll_d24, <CN>Òþº¬¾§Õñ */
	{7, 0, TOP_AVCPU_XTAL_MUX},		/* clk_usbpll_d24, <CN>Òþº¬¾§Õñ */
	{-1, -1}
};

#ifndef CFG_CLOCK_OPT_SIZE
/* omc mux */
static struct mt_clk_map_table g_omc_mux[] =
{
	{0, 0, "ddrphy1"},	/* ddrphy1: ddrpll_clk_2t2mc[1] */
	{1, 1, "ddrphy0"},	/* ddrphy0: ddrpll_clk_2t2mc[0] */
	{-1, -1}
};
#endif

/* dma rate */
static struct mt_clk_map_table g_dma_rate[] =
{
	{0, 262000000},
	{1, 411500000},
	//{2, 131000000},		/* <CN>Òþº¬¾§Õñ */
	{2, 0, TOP_DMA_XTAL_MUX}, 	/* <CN>Òþº¬¾§Õñ */
	//{3, 320000000},		/* A1 RM */
	{-1, -1}
};

/* usb1 app divider */
static struct mt_clk_map_table g_usb1_app_div[] =
{
	{0, 2000, MT_USB30PLL},	/* clk_usb30pll_d4 / 5, 125M */
	{1, 1600, MT_EPHYPLL},	/* pll_eth_d16, 125M */
	{-1, -1}
};

/* usb0 core rate */
static struct mt_clk_map_table g_usb0_core_rate[] =
{
	{0, 60000000},
	{1, 240000000},
	{-1, -1}
};

/* sdio rate */
static struct mt_clk_map_table g_sdio_rate[] =
{
	//{0, 800000000},		/* clk_vsdpll_d4, reserved */		/* A1 RM */
	{1, 720000000},		/* clk_usbpll_d4 */
	{2, 640000000},		/* clk_usbpll_d4p5 */
	{3, 576000000},		/* clk_usbpll_d5 */
	{4, 480000000},		/* clk_usbpll_d6 */
	{5, 96000000}, 		/* clk_usbpll_d30 */
	{-1, -1}
};

/* spi high rate */
static struct mt_clk_map_table g_spi_high_rate[] =
{
	{0, 0, MT_CLK_XTAL},	/* xtal 24M/27M */
	//{1, 160000000},			/* clk_usbpll_d18 */			/* A1 RM */
	{2, 576000000},			/* clk_usbpll_d5 */
	{3, 720000000},			/* clk_usbpll_d4 */
#ifndef CFG_EPHYPLL_UNUSED
	{4, 800000000},			/* clk_ethpll_d2p5 */
#endif
	{5, 960000000},			/* clk_usbpll_d3 */
	{-1, -1}
};

/* spi divider */
static struct mt_clk_map_table g_spi_div[] =
{
	{0, 100, SPI0_HIGH_CLK},	/* spi0_high_clk / 1 */
	{1, 100, SPI0_HIGH_CLK},	/* spi0_high_clk / 1 */
	{2, 200, SPI0_HIGH_CLK},	/* spi0_high_clk / 2 */
	{3, 400, SPI0_HIGH_CLK},	/* spi0_high_clk / 4 */
	{-1, -1}
};

/* pnand high rate */
static struct mt_clk_map_table g_pnand_high_rate[] =
{
	{0, 262000000},			/* clk_usbpll_d11 */
	//{1, 640000000},			/* clk_usbpll_d4p5, reserved */		/* A1 RM */
	{2, 576000000},			/* clk_usbpll_d5, default */
	{3, 480000000},			/* clk_usbpll_d6 */
#ifndef CFG_EPHYPLL_UNUSED
	{4, 400000000},			/* clk_ethpll_d5 */
#endif
	{5, 0, MT_CLK_XTAL},	/* xtal 24M/27M */
	{-1, -1}
};

/* pnand divider */
static struct mt_clk_map_table g_pnand_div[] =
{
	{0, 100, PNAND_HIGH_CLK},		/* pnand_high_clk / 1 */
	{1, 100, PNAND_HIGH_CLK},		/* pnand_high_clk / 1 */
	{2, 200, PNAND_HIGH_CLK},		/* pnand_high_clk / 2 */
	{3, 400, PNAND_HIGH_CLK},		/* pnand_high_clk / 4 */
	{-1, -1}
};

/* uart phy rate */
static struct mt_clk_map_table g_uartphy_rate[] =
{
	{0, 0, MT_CLK_XTAL},		/* xtal 24M/27M/20M */
	{1, 120000000},
	{-1, -1}
};

/* smc rate */
static struct mt_clk_map_table g_smc_rate[] =
{
	{0, 90000000},
	{1, 0, "xtal_clk_d2"},		/* xtal 24M/27M/40M div 2*/
	{-1, -1}
};

/* gra rate */
static struct mt_clk_map_table g_gra_rate[] =
{
	//{0, 667000000},			/* clk_ethpll_d3, reserved */		/* A1 RM */
	//{1, 615380000},			/* clk_ethpll_d3p25, reserved */	/* A1 RM */
	{2, 576000000},			/* clk_usbpll_d5, MAX */
	//{3, 500000000},			/* clk_ethpll_d4 */					/* A1 RM */
	//{4, 480000000}, 		/* clk_usbpll_d6 */						/* A1 RM */
	{5, 360000000}, 		/* clk_usbpll_d8, default */
	{6, 262000000}, 		/* clk_usbpll_d11 */
	{7, 0, MT_CLK_XTAL}, 	/* xtal 24M/27M */
	{-1, -1}
};

/* jpg rate */
static struct mt_clk_map_table g_jpg_rate[] =
{
	{0, 262000000},
	//{1, 320000000},		/* reserved */					/* A1 RM */
	//{2, 144000000},		/* <CN>Òþº¬¾§Õñ */
	{2, 0, TOP_JPG_XTAL_MUX}, 	/* <CN>Òþº¬¾§Õñ */
	//{3, 288000000},		/* reserved */					/* A1 RM */
	{-1, -1}
};

/* dispvdc rate */
static struct mt_clk_map_table g_dispvdc_rate[] =
{
	{0, 594000000},			/* clk_vhdpll_d5 */
	{1, 480000000},			/* clk_usbpll_d6, default */
#ifndef CFG_EPHYPLL_UNUSED
	{2, 615380000},			/* pll_eth_d3p5, 615.38, reserved */
#endif
	{3, 0, MT_CLK_AXI},		/* axi_clk */
	{-1, -1}
};

/* disposdc rate */
static struct mt_clk_map_table g_disposdc_rate[] =
{
	{0, 262000000},		/* clk_usbpll_d11, default */
	//{1, 247500000},		/* clk_vhdpll_d12 */			/* A1 RM */
	{2, 206000000},		/* clk_usbpll_d14 */
	//{3, 144000000},		/* clk_usbpll_d20 */			/* A1 RM */
	{-1, -1}
};

/* dispdi rate */
static struct mt_clk_map_table g_dispdi_rate[] =
{
	{0, 288000000},		/* default */
	//{1, 262000000},											/* A1 RM */
	{2, 160000000},
	//{3, 60000000},		/* <CN>Òþº¬¾§Õñ */
	//{3, 0, TOP_DISP_XTAL_MUX},		/* <CN>Òþº¬¾§Õñ */		/* A1 RM */
	{-1, -1}
};

/* dispcore rate */
static struct mt_clk_map_table g_dispcore_rate[] =
{
#ifndef CFG_EPHYPLL_UNUSED
	{0, 615380000},		/* clk_ethpll_d3p25, reserved */
#endif
	{1, 594000000},		/* clk_vhdpll_d5, default */
	{2, 320000000},		/* clk_usbpll_d9 */
	{3, 206000000},		/* clk_usbpll_d14 */
	//{4, 131000000},		/* clk_usbpll_d22 */							/* A1 RM */
	//{5, 60000000},		/* clk_usbpll_d48, <CN>Òþº¬¾§Õñ */
	//{5, 0, TOP_DISP_XTAL_MUX},		/* clk_usbpll_d48, <CN>Òþº¬¾§Õñ */	/* A1 RM */
	{-1, -1}
};

/* vdec rate */
static struct mt_clk_map_table g_vdec_rate[] =
{
	{0, 594000000},		/* clk_vhdpll_d5, reserved */
	{1, 576000000},		/* clk_usbpll_d5 */
	{2, 411000000},		/* ? */
	{3, 495000000},		/* clk_vhdpll_d6 */
	{4, 262000000},		/* clk_usbpll_d11 */
	//{5, 0, 131000000},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */
	//{5, 0, TOP_VDEC_XTAL_MUX},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */	/* A1 RM */
	{-1, -1}
};

/* aout rate */
static struct mt_clk_map_table g_aout_rate[] =
{
	{0, 288000000},		/* clk_usbpll_d10 */
	{1, 262000000},		/* clk_usbpll_d11, 261818182 */
	//{2, 206000000},		/* clk_usbpll_d14, 205714286 */					/* A1 RM */
	//{3, 131000000},		/* clk_usbpll_d22, 130909091, <CN>Òþº¬¾§Õñ */
	//{3, 0, TOP_AOUT_XTAL_MUX},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */	/* A1 RM */
	{-1, -1}
};

/* spdf mux */
static struct mt_clk_map_table g_spdf_mux[] =
{
	{0, 0, I_SPDF_MCLK},			/* audio internal i_spdf_mclk */
	{1, 1, I_SPDF_MCLK},			/* audio internal i_spdf_mclk */
	{2, 2, SPDF_AUDIOPLL_CLK},		/* "spdf_audiopll_clk" */
	{3, 3, SPDF_DIGDIV_CLK},		/* "spdf_digdiv_clk" */
	{-1, -1}
};

#ifndef CFG_CLOCK_OPT_SIZE
/* spdf digdiv mclk mux */
static struct mt_clk_map_table g_spdf_digdiv_mux[] =
{
	{0, 0, SPDF_AUDIOPLL_CLK},		/* "spdf_audiopll_clk" */
	{1, 1, SPDF_DIGDIV_CLK},		/* "spdf_digdiv_clk" */
	{-1, -1}
};
#endif

/* adac mux */
static struct mt_clk_map_table g_adac_mux[] =
{
	{0, 0, AOUT_MCLK},			/* audio internal aout_mclk */
	{1, 1, "aout_mclk_d2"},		/* audio internal aout_mclk div 2 */
	{2, 2, DAI_TX_CLK},			/* DAI dai_tx_clk */
	{3, 3, DAI_TX_CLK},			/* DAI dai_tx_clk */
	{-1, -1}
};

/* hdmi aud rate */
static struct mt_clk_map_table g_hdmi_aud_rate[] =
{
	{0, 320000000},
	{1, 160000000},
	{2, 0, SPDF_DIGDIV_MCLK},	/* "aout_spdf_digdiv_mclk" */
	//{3, 120000000},			/* A1 RM */
	//{4, 90000000},			/* A1 RM */
	/*
	 o_spdf_mclk
	             > audio_mclk_hdmi
	 o_aout_mclk
	*/
	{5, 0, AUDIO_MCLK_HDMI},	/* "audio_mclk_hdmi" */
	{-1, -1}
};

#ifndef CFG_CLOCK_OPT_SIZE
/* hdmi aud mux */
static struct mt_clk_map_table g_hdmi_aud_mux[] =
{
	{0, 0, AOUT_MCLK},				/* audio internal aout_mclk */
	{1, 1, MT_CLK_SPDIF}, 			/* "o_spdf_mclk" */
	{-1, -1}
};
#endif

/* hdmi pixnx divider */
static struct mt_clk_map_table g_hdmi_pixnx_div[] =
{
	{0, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{1, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{2, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{3, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{4, 200, MT_ANA_VENC_OS},		/* venc_osclk / 2 */
	{5, 400, MT_ANA_VENC_OS},		/* venc_osclk / 4 */
	{6, 600, MT_ANA_VENC_OS},		/* venc_osclk / 6 */
	{7, 800, MT_ANA_VENC_OS},		/* venc_osclk / 8 */
	{-1, -1}
};

/* hdmi tmds divider */
static struct mt_clk_map_table g_hdmi_tmds_div[] =
{
	{0, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{1, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{2, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{3, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{4, 200, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 2 */
	{5, 400, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 4 */
	{6, 600, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 6 */
	{7, 800, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 8 */
	{-1, -1}
};

#ifndef CFG_CLOCK_OPT_SIZE
/* spdif audiopll divider */
static struct mt_clk_map_table g_spdf_audiopll_div[] =
{
	{0, 100, MT_AUDIOPLL},		/* clk_audiopll / 1 */
	{1, 200, MT_AUDIOPLL},		/* clk_audiopll / 2 */
	{2, 300, MT_AUDIOPLL},		/* clk_audiopll / 3 */
	{3, 400, MT_AUDIOPLL},		/* clk_audiopll / 4 */
	{4, 600, MT_AUDIOPLL},		/* clk_audiopll / 6 */
	{5, 800, MT_AUDIOPLL},		/* clk_audiopll / 8 */
	{6, 1200, MT_AUDIOPLL},		/* clk_audiopll / 12 */
	{7, 1600, MT_AUDIOPLL},		/* clk_audiopll / 16 */
	{8, 2400, MT_AUDIOPLL},		/* clk_audiopll / 24 */
	{9, 3200, MT_AUDIOPLL},		/* clk_audiopll / 32 */
	{10, 4800, MT_AUDIOPLL},	/* clk_audiopll / 48 */
	{11, 9600, MT_AUDIOPLL},	/* clk_audiopll / 96 */
	{-1, -1}
};
#endif

/* hdvenc divider */
static struct mt_clk_map_table g_hdvenc_div[] =
{
	{0, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{1, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{2, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{3, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{4, 200, MT_ANA_VENC_OS},		/* venc_osclk / 2 */
	{5, 400, MT_ANA_VENC_OS},		/* venc_osclk / 4 */
	{6, 600, MT_ANA_VENC_OS},		/* venc_osclk / 6 */
	{7, 800, MT_ANA_VENC_OS},		/* venc_osclk / 8 */
	{-1, -1}
};

/* hdmi xtal mux */
static struct mt_clk_map_table g_hdmi_mux[] =
{
	{0, 0, MT_CLK_HDMI},
	{1, 1, MT_CLK_XTAL},
	{-1, -1}
};

/* sdvenc xtal mux */
static struct mt_clk_map_table g_sdvenc_mux[] =
{
	{0, 0, SDVENC_27M_CLK},
	{1, 1, MT_CLK_XTAL},
	{-1, -1}
};

/* sdvenc 27m mux */
static struct mt_clk_map_table g_sdvenc_27m_mux[] =
{
	{0, 0, MT_CLK_XTAL},
	{1, 1, "ana_sdvenc_clk_d4"},	/* ana_sdvenc_clk div 4, 108M / 4 = 27M */
	{-1, -1}
};

/* png rate */
static struct mt_clk_map_table g_png_rate[] =
{
	{0, 262000000},
	{1, 320000000},		/* reserved */
	//{2, 144000000},		/* <CN>Òþº¬¾§Õñ */
	{2, 0, TOP_PNG_XTAL_MUX},		/* <CN>Òþº¬¾§Õñ */
	//{3, 288000000},		/* reserved */				/* A1 RM */
	{-1, -1}
};

#ifndef CFG_CLOCK_OPT_SIZE
/* dai rx mux */
static struct mt_clk_map_table g_dai_rx_mux[] =
{
	{0, 0, DAI_RX_AUDIOPLL_CLK},
	{1, 1, DAI_RX_DIGDIV_CLK},
	{-1, -1}
};

/* dai tx mux */
static struct mt_clk_map_table g_dai_tx_mux[] =
{
	{0, 0, DAI_TX_AUDIOPLL_CLK},
	{1, 1, DAI_TX_DIGDIV_CLK},
	{-1, -1}
};

/* dai tx/rx audiopll divider */
static struct mt_clk_map_table g_dai_audiopll_div[] =
{
	{0, 100, MT_AUDIOPLL},		/* clk_audiopll / 1 */
	{1, 200, MT_AUDIOPLL},		/* clk_audiopll / 2 */
	{2, 300, MT_AUDIOPLL},		/* clk_audiopll / 3 */
	{3, 400, MT_AUDIOPLL},		/* clk_audiopll / 4 */
	{4, 600, MT_AUDIOPLL},		/* clk_audiopll / 6 */
	{5, 800, MT_AUDIOPLL},		/* clk_audiopll / 8 */
	{6, 1200, MT_AUDIOPLL},		/* clk_audiopll / 12 */
	{7, 1600, MT_AUDIOPLL},		/* clk_audiopll / 16 */
	{8, 2400, MT_AUDIOPLL},		/* clk_audiopll / 24 */
	{9, 3200, MT_AUDIOPLL},		/* clk_audiopll / 32 */
	{10, 4800, MT_AUDIOPLL},	/* clk_audiopll / 48 */
	{11, 9600, MT_AUDIOPLL},	/* clk_audiopll / 96 */
	{-1, -1}
};
#endif

/* gpu rate */
static struct mt_clk_map_table g_gpu_rate[] =
{
	//{0, 720000000},		/* clk_usbpll_d4, reserved */		/* A1 RM */
#ifndef CFG_EPHYPLL_UNUSED
	//{1, 667000000},		/* clk_ethpll_d3, reserved */		/* A1 RM */
	{2, 615380000},		/* clk_ethpll_d3p25, reserved */
#endif
	{3, 576000000},		/* clk_usbpll_d5 */
	{4, 480000000},		/* clk_usbpll_d6 */
	{5, 360000000},		/* clk_usbpll_d8, default */
	{6, 594000000},		/* clk_vhdpll_d5, reserved */
	//{7, 120000000},		/* clk_usbpll_d24, <CN>Òþº¬¾§Õñ */
	{7, 0, TOP_GPU_XTAL_MUX},		/* clk_usbpll_d24, <CN>Òþº¬¾§Õñ */
	{-1, -1}
};

/* gmac mux */
static struct mt_clk_map_table g_gmac_mux[] =
{
	{0, 0, "GMII/MII"},
	{1, 1, "RGMII"},
	{4, 4, "RMII"},
	{-1, -1}
};

/* gmac phyref rate */
static struct mt_clk_map_table g_gmac_phyref_rate[] =
{
	{0, 0},				/* no output clk */
	{1, 125000000},		/* clk_ethpll_d16 */
	{2, 50000000},
	{3, 25000000},
	{-1, -1}
};

/* gmac ptp rate */
static struct mt_clk_map_table g_gmac_ptp_rate[] =
{
	{0, 125000000},		/* clk_ethpll_d16 */
	{1, 125000000},
	{2, 50000000},
	{3, 25000000},
	{-1, -1}
};

/* tsi rate */
static struct mt_clk_map_table g_tsi_rate[] =
{
#ifndef CFG_EPHYPLL_UNUSED
	{0, 400000000},		/* clk_ethpll_d5 */
#endif
	{1, 360000000},		/* clk_usbpll_d8 */
	{2, 262000000},		/* clk_usbpll_d11 */
	//{3, 131000000},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */
	{3, 0, TOP_TSI_XTAL_MUX},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */
	{-1, -1}
};

/* descramble rate */
static struct mt_clk_map_table g_dsc_rate[] =
{
	//{0, 400000000},		/* clk_ethpll_d5, reverved */				/* A1 RM */
	{1, 360000000},		/* clk_usbpll_d9 */
	{2, 262000000},		/* clk_usbpll_d11 */
	//{3, 131000000},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */
	{3, 0, TOP_DS_XTAL_MUX},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */
	{-1, -1}
};

/* m2m cipher rate */
static struct mt_clk_map_table g_m2m_rate[] =
{
	//{0, 400000000},		/* clk_ethpll_d5, reverved */				/* A1 RM */
	{1, 360000000},		/* clk_usbpll_d9 */
	{2, 262000000},		/* clk_usbpll_d11 */
	//{3, 131000000},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */
	{3, 0, TOP_CIPHER_XTAL_MUX},		/* clk_usbpll_d22, <CN>Òþº¬¾§Õñ */
	{-1, -1}
};

/* secure rate */
static struct mt_clk_map_table g_secure_rate[] =
{
	//{0, 262000000},			/* reverved */					/* A1 RM */
	{1, 206000000},
	{2, 240000000},			/* reverved */
	//{3, 131000000},		/* <CN>Òþº¬¾§Õñ */
	{3, 0, TOP_SECURE_XTAL_MUX},		/* <CN>Òþº¬¾§Õñ */
	{-1, -1}
};

/* pka rate */
static struct mt_clk_map_table g_pka_rate[] =
{
	{0, 576000000},
	{1, 206000000},
	{-1, -1}
};

/* ifcp crypto rate */
static struct mt_clk_map_table g_ifcp_crypto_rate[] =
{
	//{0, 262000000},			/* reverved */					/* A1 RM */
	{1, 206000000},
	//{2, 120000000},		/* <CN>Òþº¬¾§Õñ */
	{2, 0, TOP_IFCPCRYPTO_XTAL_MUX},		/* <CN>Òþº¬¾§Õñ */
	{3, 240000000},			/* reverved */
	{-1, -1}
};

/* ifcp sys rate */
static struct mt_clk_map_table g_ifcp_sys_rate[] =
{
	//{0, 400000000},		/* clk_ethpll_d5, reserved */		/* A1 RM */
	//{1, 360000000},		/* clk_usbpll_d8, reserved */		/* A1 RM */
	//{2, 320000000},		/* clk_usbpll_d9, reserved */		/* A1 RM */
	{3, 240000000},		/* clk_usbpll_d12, default */
	{4, 120000000},		/* clk_usbpll_d24 */
	{5, 24000000},		/* clk_usbpll_d120 */
	{-1, -1}
};

/* xtal rate */
static struct mt_clk_map_table g_xtal_rate[] =
{
	{0, 27000000},
	{1, 24000000},
#if 0
	{2, 40000000},
	{3, 40000000},
#endif
	{-1, -1}
};

#ifndef CFG_CLOCK_OPT_SIZE
/* spdf/dai digdiv rate */
static struct mt_clk_map_table g_digdiv_rate[] =
{
	/* Freq(o_clk) = (increment / 33554432) * 960MHz */
	{0x346dc5, 98304000},	/* default */
	{-1, -1}
};
#endif

/* ddrphy rate */
static struct mt_clk_map_table g_ddrphy_rate[] =
{
	{0, 533000000},		/* 0: 1066Mhz */
	{1, 667000000},		/* 1: 1333Mhz */
	{2, 800000000},		/* 2: 1600Mhz */
	{3, 933000000},		/* 3: 1866Mhz */
	{4, 1040000000},	/* 4: 2080Mhz */
	{5, 1066000000},	/* 5: 2133Mhz */
	{6, 1100000000},	/* 6: 2200Mhz */
	{7, 1200000000},	/* 7: 2400Mhz */
	{-1, -1}
};

/* armpll rate */
static struct mt_clk_map_table g_armpll_rate[] =
{
	/* XTAL: 27MHz */
	{0x9C2E91D9, 702000000},
	{0x9C3691D9, 774000000},
	{0x9C3991D9, 801000000},

	{0x5C3E91D9, 846000000},
	{0x5C4491D9, 900000000},
	{0x5C4691D9, 918000000},

	{0x1C4E91DA, 990000000},
	{0x1C4F91DA, 999000000},
	{0x1C5691DA, 1062000000},
	{0x1C5B91DA, 1107000000},
	{0x1C5E91DA, 1134000000},

	{0xDC2391C9, 1206000000},
	{0xDC2791C9, 1278000000},
	{0xDC2891C9, 1296000000},
	{0xDC2991C9, 1314000000},
	{0xDC2B91C9, 1350000000},
	{0xDC2E91C9, 1404000000},

	{0x9C2F91C9, 1422000000},
	{0x9C3091C9, 1440000000},
	{0x9C3391C9, 1494000000},
	{0x9C3591C9, 1530000000},
	{0x9C3791C9, 1566000000},

	{0x5C3991C9, 1602000000},
	{0x5C3A91C9, 1620000000},
	{0x5C3B91C9, 1638000000},
	{0x5C3F91C9, 1710000000},
	{0x5C4391C9, 1782000000},

	/* XTAL: 24MHz */
	{0x9C3891D9, 704000000},
	{0x9C4191D9, 776000000},
	{0x9C4491D9, 800000000},

	{0x5C4A91D9, 848000000},
	{0x5C5091D9, 896000000},
	{0x5C5391D9, 920000000},

	{0x1C5C91DA, 992000000},
	{0x1C5D91DA, 1000000000},
	{0x1C6591DA, 1064000000},
	{0x1C6A91DA, 1104000000},
	{0x1C6E91DA, 1136000000},
	{0x1C7691DA, 1200000000},

	{0xDC7791D9, 1208000000},
	{0xDC8091D9, 1280000000},
	{0xDC8291D9, 1296000000},
	{0xDC8391D9, 1304000000},
	{0xDC8491D9, 1312000000},
	{0xDC8991D9, 1352000000},
	{0xDC8F91D9, 1400000000},

	{0x9C3991C9, 1424000000},
	{0x9C3A91C9, 1440000000},
	{0x9C3E91C9, 1504000000},
	{0x9C4091C9, 1536000000},
	{0x9C4391C9, 1584000000},
	{0x9C4491C9, 1600000000},

	{0x5C4891C9, 1664000000},
	{0x5C4A91C9, 1696000000},
	{0x5C4D91C9, 1744000000},

	{-1, -1}
};

struct mt_clk_in mt_clk_table[] =
{
	/* MAC */
	{
		.name = MT_CLK_MAC,
		.flags = FLAG_CLK_MUX|FLAG_CLK_ATTR,
		.gate = MT_INIT_GATE(REG_MAC_CLKEN, MAC_CLKEN_SHIFT,
							REG_MAC_SLOCK, REG_MAC_LOCK, MAC_CLKEN_LOCK_SHIFT),
		/* 0: MII, 25MHz 1: RMII, 50MHz */
		/* !Caution: has clock glitches! <CNComment>ÓÐÃ«´Ì! */
		.mux  = MT_INIT_MUX(REG_MAC_CLKSEL, MAC_CLKMODE_SHIFT, 1, g_mac_mux,
							REG_MAC_SLOCK, REG_MAC_LOCK, MAC_CLKMODE_LOCK_SHIFT),
 		/*
		 * mac_clksel
		 *   0: original 50MHz
		 *   1: original 50MHz revert
		 * !Caution: has clock glitches! <CNComment>ÓÐÃ«´Ì!
		 */
		.attr = MT_INIT_ATTR(REG_MAC_CLKSEL, MAC_CLKSEL_SHIFT, 1,
							REG_MAC_SLOCK, REG_MAC_LOCK, MAC_CLKSEL_LOCK_SHIFT)
	},
	/* RMII */
	{
		.name = MT_CLK_RMII,
		.flags = FLAG_CLK_FIXED_RATE,
		.gate = MT_INIT_GATE(REG_MAC_CLKEN, RMII_CLKEN_SHIFT,
							REG_MAC_SLOCK, REG_MAC_LOCK, RMII_CLKEN_LOCK_SHIFT),
		.fixed_rate = 50 * ONE_MHZ
	},

	/* AHB */
	{
		.name = MT_CLK_AHB,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_BUS_CLKSEL, AHB_CLKSEL_SHIFT, 3, g_ahb_rate,
							REG_BUS_SLOCK, REG_BUS_LOCK, AHB_CLKSEL_LOCK_SHIFT)
	},
	/* APB */
	{
		.name = MT_CLK_APB,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_BUS_CLKSEL, APB_CLKSEL_SHIFT, 2, g_apb_rate,
							REG_BUS_SLOCK, REG_BUS_LOCK, APB_CLKSEL_LOCK_SHIFT)
	},
	/* AXI */
#if 0
	{
		.name = MT_CLK_AXI,
		.flags = FLAG_CLK_DIV,
		.gate = {0},
		.div = MT_INIT_DIV(REG_BUS_CLKSEL, AXI_CLKSEL_SHIFT, 3, g_axi_div,
							REG_BUS_SLOCK, REG_BUS_LOCK, AXI_CLKSEL_LOCK_SHIFT)
	},
#else
	{
		.name = MT_CLK_AXI,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_BUS_CLKSEL, AXI_CLKSEL_SHIFT, 3, g_axi_rate,
							REG_BUS_SLOCK, REG_BUS_LOCK, AXI_CLKSEL_LOCK_SHIFT)
	},
#endif

	/* APCPU CLK(AXI/AHB) */
	{
		.name = MT_CLK_APCPU,
		.flags = FLAG_CLK_MUX|FLAG_CLK_ATTR,
		.gate = MT_INIT_GATE(REG_APCPU_CLKEN, APCPU_CLKEN_SHIFT,
							REG_APCPU_SLOCK, REG_APCPU_LOCK, APCPU_CLKEN_LOCK_SHIFT),
		/* 0: backup, 1: armpll */
		.mux  = MT_INIT_MUX(REG_APCPU_CLKSEL, BACKUP_ARMPLL_CLKSEL_SHIFT, 1, g_apcpu_mux,
							REG_APCPU_SLOCK, REG_APCPU_LOCK, APCPU_CLKSEL_LOCK_SHIFT),
		/*
		 * armpll_delay_set
		 *   while backup_armpll_clksel=1, delay 0~31ms
		 */
		.attr = MT_INIT_ATTR(REG_TOPCLK_CTRL7, TOP_ARMPLL_DELAY_SET_SHIFT, 5,
							REG_TOPCLK_CTRL7_SLOCK, REG_TOPCLK_CTRL7_LOCK, ARMPLL_DELAY_SET_LOCK_SHIFT)
	},
#ifndef CFG_CLOCK_OPT_SIZE
	/* APCPU APB CLK */
	{
		.name = APCPU_PCLK,		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_APCPU_CLKEN, APCPU_PCLKEN_SHIFT,
							REG_APCPU_SLOCK, REG_APCPU_LOCK, APCPU_PCLKEN_LOCK_SHIFT)
	},
#endif
	/* AP_BACKUP_CLK */
	{
		.name = MT_CLK_APBACKUP,
		.flags = FLAG_CLK_RATE|FLAG_CLK_ATTR,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_APCPU_CLKSEL, AP_BACKUP_CLK_SEL_SHIFT, 4, g_apbackup_rate,
							REG_APCPU_SLOCK, REG_APCPU_LOCK, APCPU_CLKSEL_LOCK_SHIFT),
		/*
		 * 0: switch clock directly
		 * 1: switch clock after wait 63 pclk cycles
		 */
		.attr = MT_INIT_ATTR(REG_TOPCLK_CTRL6, APBACKUP_GROUP_CLKSEL_MODE_SHIFT, 1,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT22_LOCK_SHIFT)
	},

	/* AVCPU */
	{
		.name = MT_CLK_AVCPU,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_AVCPU_CLKEN, AVCPU_CLKEN_SHIFT,
							REG_AVCPU_SLOCK, REG_AVCPU_LOCK, AVCPU_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_AVCPU_CLKSEL, AVCPU_CLKSEL_SHIFT, 3, g_avcpu_rate,
							REG_AVCPU_SLOCK, REG_AVCPU_LOCK, AVCPU_CLKSEL_LOCK_SHIFT)
	},

#ifndef CFG_CLOCK_OPT_SIZE
	/* oic_axi_clk */
	{
		.name = OIC_AXI_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SYS_CLKEN, OIC_AXI_CLKEN_SHIFT,
							REG_SYS_SLOCK, REG_SYS_LOCK, OIC_AXI_CLKEN_LOCK_SHIFT)
	},
	/* axi_reg_clk */
	{
		.name = AXI_REG_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SYS_CLKEN, AXI_REG_CLKEN_SHIFT,
							REG_SYS_SLOCK, REG_SYS_LOCK, AXI_REG_CLKEN_LOCK_SHIFT)
	},
	/* omc_dfi_clk */
	{
		.name = OMC_DFI_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SYS_CLKEN, OMCDFI_CLKEN_SHIFT,
							REG_SYS_SLOCK, REG_SYS_LOCK, OMC_DFI_CLKEN_LOCK_SHIFT)
	},
	/* omc_phy_clk */
	{
		.name = OMC_PHY_CLK,	/* Hide */
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_GATE(REG_SYS_CLKEN, OMCPHY_CLKEN_SHIFT,
							REG_SYS_SLOCK, REG_SYS_LOCK, OMC_PHY_CLKEN_LOCK_SHIFT),
		/*
		 * 0: ddrphy1 - ddrpll_clk_2t2mc[1]
		 * 1: ddrphy0 - ddrpll_clk_2t2mc[0]
		 */
		.mux = MT_INIT_MUX(REG_SYS_CLKSEL, OMC_CLKSEL_SHIFT, 1, g_omc_mux,
							REG_SYS_SLOCK, REG_SYS_LOCK, OMC_CLKSEL_LOCK_SHIFT)
	},
#endif
	/* DMA */
	{
		.name = MT_CLK_DMA,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_SYS_CLKEN, DMA_CLKEN_SHIFT,
							REG_SYS_SLOCK, REG_SYS_LOCK, DMA_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_SYS_CLKSEL, DMA_CLKSEL_SHIFT, 2, g_dma_rate,
							REG_SYS_SLOCK, REG_SYS_LOCK, DMA_CLKSEL_LOCK_SHIFT)
	},
	/* sw_twm_clk */
	{
		.name = SW_TWM_CLK,		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SYS_CLKEN, SW_TWM_CLKEN_SHIFT,
							REG_SYS_SLOCK, REG_SYS_LOCK, SWTWM_CLKEN_LOCK_SHIFT)
	},

#if 0
	/* usb1_phy_utmi_clk */
	{
		.name = USB1_PHY_UTMI_CLK, 	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB1_PHY_UTMI_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB1_CLKEN_LOCK_SHIFT)
	},
	/* usb1_phy_clk */
	{
		.name = USB1_PHY_CLK, 		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB1_PHY_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB1_CLKEN_LOCK_SHIFT)
	},
	/* usb1_lfps_clk */
	{
		.name = USB1_LFPS_CLK, 		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB1_LFPS_CLKEN_SHIFT,
		 					REG_INTF_SLOCK, REG_INTF_LOCK, USB1_CLKEN_LOCK_SHIFT)
	},
	/* usb1_pclk */
	{
		.name = USB1_PCLK, 		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB1_PCLK_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB1_CLKEN_LOCK_SHIFT)
	},
	/* usb1_aclk */
	{
		.name = USB1_ACLK, 		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB1_ACLK_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB1_CLKEN_LOCK_SHIFT)
	},
#endif
	/* usb1_app_clk */
	{
		.name = USB1_APP_CLK, 	/* Hide */
		.flags = FLAG_CLK_DIV,
#if 0
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB1_APP_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB1_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		/*
		 * 0: clk_usb30pll_d4 / 5, 125M
		 * 1: pll_eth_d16, 125M
		 */
		.div = MT_INIT_DIV(REG_INTF_CLKSEL, USB1APP_CLKSEL_SHIFT, 1, g_usb1_app_div,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB_CLKSEL_LOCK_SHIFT)
	},
#if 1
	/* usb1_clk */
	{
		.name = MT_CLK_USB1,	/* usb1 all clocks */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_MUX(REG_INTF_CLKEN, USB1_APP_CLKEN_SHIFT, 6, NULL,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB1_CLKEN_LOCK_SHIFT)
	},
#endif

#if 0
	/* usb0_phy_utmi_clk */
	{
		.name = USB0_PHY_UTMI_CLK, 	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB0_PHY_UTMI_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB0_CLKEN_LOCK_SHIFT)
	},
	/* usb0_phy_clk */
	{
		.name = USB0_PHY_CLK, 		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB0_PHY_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB0_CLKEN_LOCK_SHIFT)
	},
	/* usb0_ahb_axi_clk */
	{
		.name = USB0_AHB_AXI_CLK, 		/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, USB0_AHB_AXI_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB0_CLKEN_LOCK_SHIFT)
	},
#endif
#if 1
	/* usb0_clk */
	{
		.name = MT_CLK_USB0,	/* usb0 all clocks */
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_MUX(REG_INTF_CLKEN, USB0_AHB_AXI_CLKEN_SHIFT, 3, NULL,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB0_CLKEN_LOCK_SHIFT),
		.rate = MT_INIT_RATE(REG_INTF_CLKSEL, USB0_CORE_CLKSEL_SHIFT, 1, g_usb0_core_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, USB_CLKSEL_LOCK_SHIFT)
	},
#endif

	/* SDIO1 */
	{
		.name = MT_CLK_SDIO1,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, SDIO1_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, SDIO1_CLKEN_LOCK_SHIFT),
		.rate = MT_INIT_RATE(REG_INTF_CLKSEL, SDIO1_CLKSEL_SHIFT, 3, g_sdio_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, SDIO1_CLKSEL_LOCK_SHIFT)
	},
	/* SDIO0 */
	{
		.name = MT_CLK_SDIO0,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, SDIO0_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, SDIO0_CLKEN_LOCK_SHIFT),
		.rate = MT_INIT_RATE(REG_INTF_CLKSEL, SDIO0_CLKSEL_SHIFT, 3, g_sdio_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, SDIO0_CLKSEL_LOCK_SHIFT)
	},
	/* pnand_high */
	{
		.name = PNAND_HIGH_CLK,	/* Hide */
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_INTF_CLKSEL, PNAND_HIGH_CLKSEL_SHIFT, 3, g_pnand_high_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, PNAND_CLKSEL_LOCK_SHIFT)
	},
	/* PNAND */
	{
		.name = MT_CLK_PNAND,
		.flags = FLAG_CLK_DIV,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, PNAND_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, PNAND_CLKEN_LOCK_SHIFT),
		.div = MT_INIT_DIV(REG_INTF_CLKSEL, PNAND_CORE_CLKSEL_SHIFT, 2, g_pnand_div,
							REG_INTF_SLOCK, REG_INTF_LOCK, PNAND_CLKSEL_LOCK_SHIFT)
	},
	/* spi0_high */
	{
		.name = SPI0_HIGH_CLK,	/* Hide */
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_INTF_CLKSEL, SPI0_HIGH_CLKSEL_SHIFT, 3, g_spi_high_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, SPI0_CLKSEL_LOCK_SHIFT)
	},
	/* SPI0 */
	{
		.name = MT_CLK_SPI0,
		.flags = FLAG_CLK_DIV,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, SPI0_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, SPI0_CLKEN_LOCK_SHIFT),
		.div = MT_INIT_DIV(REG_INTF_CLKSEL, SPI0_CORE_CLKSEL_SHIFT, 2, g_spi_div,
							REG_INTF_SLOCK, REG_INTF_LOCK, SPI0_CLKSEL_LOCK_SHIFT)
	},
	/* UART1 */
	{
		.name = MT_CLK_UART1,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, UART1_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, UART1_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_INTF_CLKSEL, UART1PHY_CLKSEL_SHIFT, 1, g_uartphy_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, UART1PHY_CLKSEL_LOCK_SHIFT)
	},
	/* UART0 */
	{
		.name = MT_CLK_UART0,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, UART0_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, UART0_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_INTF_CLKSEL, UART0PHY_CLKSEL_SHIFT, 1, g_uartphy_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, UART0PHY_CLKSEL_LOCK_SHIFT)
	},
	/* I2CDEBUG */
	{
		.name = MT_CLK_I2CDEBUG,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, I2CDEBUG_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, I2CDEBUG_CLKEN_LOCK_SHIFT)
	},
	/* I2C1 */
	{
		.name = MT_CLK_I2C1,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, I2C1_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, I2C1_CLKEN_LOCK_SHIFT)
	},
	/* I2C0 */
	{
		.name = MT_CLK_I2C0,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, I2C0_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, I2C0_CLKEN_LOCK_SHIFT)
	},
	/* SMC */
	{
		.name = MT_CLK_SMC,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_INTF_CLKEN, SMC_CLKEN_SHIFT,
							REG_INTF_SLOCK, REG_INTF_LOCK, SMC_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_INTF_CLKSEL, SMC_PHY_CLKSEL_SHIFT, 1, g_smc_rate,
							REG_INTF_SLOCK, REG_INTF_LOCK, SMC_PHY_CLKSEL_LOCK_SHIFT)
	},

	/* GRA */
	{
		.name = MT_CLK_GRA,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_GRA_CLKEN, GRA_CLKEN_SHIFT,
							REG_GRA_SLOCK, REG_GRA_LOCK, GRA_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_GRA_CLKSEL, GRA_CLKSEL_SHIFT, 3, g_gra_rate,
							REG_GRA_SLOCK, REG_GRA_LOCK, GRA_CLKSEL_LOCK_SHIFT)
	},
	/* JPG */
	{
		.name = MT_CLK_JPG,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_JPG_CLKEN, JPG_CLKEN_SHIFT,
							REG_JPG_SLOCK, REG_JPG_LOCK, JPG_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_JPG_CLKSEL, JPG_CLKSEL_SHIFT, 2, g_jpg_rate,
							REG_JPG_SLOCK, REG_JPG_LOCK, JPG_CLKSEL_LOCK_SHIFT)
	},

	/* DISP */
	{
		.name = MT_CLK_DISP,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_DISP_CLKEN, DISP_CLKEN_SHIFT,
							REG_DISP_SLOCK, REG_DISP_LOCK, DISP_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_DISP_CLKSEL, DISPCORE_CLKSEL_SHIFT, 3, g_dispcore_rate,
							REG_DISP_SLOCK, REG_DISP_LOCK, DISPCORE_CLKSEL_LOCK_SHIFT)
	},
	/* DISP DI */
	{
		.name = MT_CLK_DISP_DI,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_DISP_CLKSEL, DISPDI_CLKSEL_SHIFT, 2, g_dispdi_rate,
							REG_DISP_SLOCK, REG_DISP_LOCK, DISPDI_CLKSEL_LOCK_SHIFT)
	},
	/* DISP OSDC */
	{
		.name = DISP_OSDC_CLK,	/* Hide */
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_DISP_CLKSEL, DISPOSDC_CLKSEL_SHIFT, 2, g_disposdc_rate,
							REG_DISP_SLOCK, REG_DISP_LOCK, DISPOSDC_CLKSEL_LOCK_SHIFT)
	},
	/* DISP VDC */
	{
		.name = DISP_VDC_CLK, 	/* Hide */
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_DISP_CLKSEL, DISPVDC_CLKSEL_SHIFT, 2, g_dispvdc_rate,
							REG_DISP_SLOCK, REG_DISP_LOCK, DISPVDC_CLKSEL_LOCK_SHIFT)
	},

	/* VDEC */
	{
		.name = MT_CLK_VDEC,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_VDEC_CLKEN, VDEC_CLKEN_SHIFT,
							REG_VDEC_SLOCK, REG_VDEC_LOCK, VDEC_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_VDEC_CLKSEL, VDEC_CLKSEL_SHIFT, 3, g_vdec_rate,
							REG_VDEC_SLOCK, REG_VDEC_LOCK, VDEC_CLKSEL_LOCK_SHIFT)
	},

	/* AOUT */
	{
		.name = MT_CLK_AOUT,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_AOUT_CLKEN, AOUT_CLKEN_SHIFT,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUT_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_AOUT_CLKSEL, AOUT_CLKSEL_SHIFT, 2, g_aout_rate,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, AOUT_CLKSEL_LOCK_SHIFT)
	},
	/* SPDIF("o_spdf_mclk") */
	{
		.name = MT_CLK_SPDIF,
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_GATE(REG_AOUT_CLKEN, SPDF_CLKEN_SHIFT,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, SPDF_CLKEN_LOCK_SHIFT),
		/*
		 * 0x: audio internal i_spdf_mclk(?)
		 * 10: "spdf_audiopll_clk"
		 * 11: "spdf_digdiv_clk"
		 */
		.mux = MT_INIT_MUX(REG_AOUT_CLKSEL, SPDF_MCLK_CLKSEL_SHIFT, 2, g_spdf_mux,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, SPDF_CLKSEL_LOCK_SHIFT)
	},
	/* ADAC */
	{
		.name = MT_CLK_ADAC,
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_GATE(REG_AOUT_CLKEN, ADAC_CLKEN_SHIFT,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, ADAC_CLKEN_LOCK_SHIFT),
		/*
		 * adac_mclk:
		 *  0: audio internal aout_mclk
		 *  1: aout_mclk / 2
		 *  2/3: DAI dai_tx_clk
		 */
		.mux = MT_INIT_MUX(REG_AOUT_CLKSEL, ADAC_MCLK_CLKSEL_SHIFT, 2, g_adac_mux,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, ADAC_MCLK_CLKSEL_LOCK_SHIFT)
	},
#ifndef CFG_CLOCK_OPT_SIZE
	/* SPDF DIG DIV CLK */
	{
		.name = SPDF_DIGDIV_CLK, /* Hide */
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_AOUT_CLKEN, SPDF_DIG_DIV_CLKEN_SHIFT,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, SPDF_DIG_DIV_CLKEN_LOCK_SHIFT),
		/*
		 * spdf_mclk_increment
		 * Freq(o_clk) = (spdf_mclk_increment / 33554432) * 960MHz
		 */
		.rate = MT_INIT_RATE(REG_AOUT_SPDF_DIGDIV, SPDF_MCLK_INCREMENT_SHIFT, 25, g_digdiv_rate,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, SPDF_CLKSEL_LOCK_SHIFT)
	},
	/* SPDIF AUDIOPLL CLK */
	{
		.name = SPDF_AUDIOPLL_CLK,	/* Hide */
		.flags = FLAG_CLK_DIV,
		.gate = {0},
		.div = MT_INIT_DIV(REG_AOUT_CLKSEL, SPDF_AUDIOPLL_CLKDIV_SHIFT, 4, g_spdf_audiopll_div,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, SPDF_CLKSEL_LOCK_SHIFT)
	},
	/* SPDF_DIGDIV_MCLK("o_audio_spdf_digdiv_mclk") */
	{
		.name = SPDF_DIGDIV_MCLK,	/* Hide */
		.flags = FLAG_CLK_MUX,
		.gate = {0},
		/*
		 * 0: "spdf_audiopll_clk"
		 * 1: "spdf_digdiv_clk"
		 */
		.mux = MT_INIT_MUX(REG_AOUT_CLKSEL, SPDF_MCLK_CLKSEL_SHIFT, 1, g_spdf_digdiv_mux,
							REG_AOUT_SLOCK, REG_AOUT_LOCK, SPDF_CLKSEL_LOCK_SHIFT)
	},
#endif

	/* SDVENC */
	{
		.name = MT_CLK_SDVENC,
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, SDVENC_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, SDVENC_CLKEN_LOCK_SHIFT),
		/*
		 * 0: none XTAL
		 * 1: XTAL
		 */
		.mux = MT_INIT_MUX(REG_VOUT_CLKSEL, SDVENC_XTAL_CLKSEL_SHIFT, 1, g_sdvenc_mux,
						REG_VOUT_SLOCK, REG_VOUT_LOCK, SDVENC_XTAL_CLKSEL_LOCK_SHIFT),
	},
	/* SDVENC_27M */
	{
		.name = SDVENC_27M_CLK,		/* Hide */
		.flags = FLAG_CLK_MUX,
		.gate = {0},
		/*
		 * sdvenc_sdclk_27m_sel
		 *	 0: xtal_clk
		 *	 1: ana_sdvenc_clk div 4
		 */
		.mux = MT_INIT_MUX(REG_VOUT_CLKEN, SDVENC_SDCLK_27_SEL_SHIFT, 1, g_sdvenc_27m_mux,
							0, 0, 0/*FIXME*/)
	},
	/* VBI */
	{
		.name = MT_CLK_VBI,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, VBI_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, VBI_CLKEN_LOCK_SHIFT)
	},
	/* HDVENC */
	{
		.name = MT_CLK_HDVENC,
		.flags = FLAG_CLK_DIV,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDVENC_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDVENC_CLKEN_LOCK_SHIFT),
		.div = MT_INIT_DIV(REG_VOUT_CLKSEL, HDVENC_CLKSEL_SHIFT, 3, g_hdvenc_div,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDVENC_CLKSEL_LOCK_SHIFT)
	},
	/* HDMI */
	{
		.name = MT_CLK_HDMI,
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_MUX(REG_VOUT_CLKEN, HDMI_PIXCLKNX_CLKEN_SHIFT, 11, NULL,
						REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT),
		/*
		 * 0: none XTAL
		 * 1: XTAL
		 */
		.mux = MT_INIT_MUX(REG_VOUT_CLKSEL, HDMI_XTAL_CLKSEL_SHIFT, 1, g_hdmi_mux,
						REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_XTAL_CLKSEL_LOCK_SHIFT)
	},
	/* HDMI TMDS CLK */
	{
		.name = HDMI_TMDS_CLK,
		.flags = FLAG_CLK_DIV,
#if 0
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_TMDSCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		.div = MT_INIT_DIV(REG_VOUT_CLKSEL, HDMI_TMDS_CLKSEL_SHIFT, 3, g_hdmi_tmds_div,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_TMDS_CLKSEL_LOCK_SHIFT)
	},
	/* HDMI PIXNX CLK */
	{
		.name = HDMI_PIXNX_CLK,
		.flags = FLAG_CLK_DIV,
#if 0
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_PIXCLKNX_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		.div = MT_INIT_DIV(REG_VOUT_CLKSEL, HDMI_PIXNX_CLKSEL_SHIFT, 3, g_hdmi_pixnx_div,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_PIXNX_CLKSEL_LOCK_SHIFT)
	},
	/* HDMI AUD CLK */
	{
		.name = HDMI_AUD_CLK,	/* Hide */
		.flags = FLAG_CLK_RATE,
#if 0
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_AUDCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		.comb = MT_INIT_RATE(REG_VOUT_CLKSEL, HDMI_AUD_CLKSEL_SHIFT, 3, g_hdmi_aud_rate,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_AUD_CLKSEL_LOCK_SHIFT)
	},
#ifndef CFG_CLOCK_OPT_SIZE
	/* AUDIO_MCLK_HDMI("audio_mclk_hdmi") */
	{
		.name = AUDIO_MCLK_HDMI,	/* Hide */
		.flags = FLAG_CLK_MUX,
		.gate = {0},
		/*
		 * 0: "o_aout_mclk"
		 * 1: "o_spdf_mclk"
		 */
		.mux = MT_INIT_MUX(REG_AOUT_AUD_I2S_SPDIF_CFG, HDMI_SPDIF_PATH_SEL_SHIFT, 1, g_hdmi_aud_mux,
							0, 0, 0)
	},
#endif

#if 0
	/* HDMI MIF CLK */
	{
		.name = HDMI_MIF_CLK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_MIFCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
	/* HDMI XCLK */
	{
		.name = HDMI_XCLK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_XCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
	/* HDMI HCLK */
	{
		.name = HDMI_HCLK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_HCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
	/* HDMI CCLK */
	{
		.name = HDMI_CCLK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_CCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
	/* HDMI CEC CLK */
	{
		.name = HDMI_CEC_CLK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_CECCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
	/* HDMI OSC CLK */
	{
		.name = HDMI_OSC_CLK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_OSCCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
	/* HDMI HDCP2X CLK */
	{
		.name = HDMI_HDCP2X_CLK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_HDCP2XCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
	/* HDMI SCK */
	{
		.name = HDMI_SCK, /* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_SCK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT)
	},
#endif

	/* PNG */
	{
		.name = MT_CLK_PNG,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_PNG_CLKEN, PNG_CLKEN_SHIFT,
							REG_PNG_SLOCK, REG_PNG_LOCK, PNG_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_PNG_CLKSEL, PNG_CLKSEL_SHIFT, 2, g_png_rate,
							REG_PNG_SLOCK, REG_PNG_LOCK, PNG_CLKSEL_LOCK_SHIFT)
	},

	/* DAI */
	{
		.name = MT_CLK_DAI,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_MUX(REG_DAI_CLKEN, DAI_CLKEN_SHIFT, 5, NULL,
						REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKEN_LOCK_SHIFT),
	},
#ifndef CFG_CLOCK_OPT_SIZE
	/* DAI RX */
	{
		.name = DAI_RX_CLK,	/* Hide */
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_GATE(REG_DAI_CLKEN, DAI_RX_CLKEN_SHIFT,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAIRX_LOCK_SHIFT),
		/*
		 * 0: "dai_rx_audiopll"
		 * 1: "dai_rx_digdiv"
		 */
		.mux = MT_INIT_MUX(REG_DAI_CLKSEL, DAI_RX_CLKSEL_SHIFT, 1, g_dai_rx_mux,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT)
	},
	/* DAI TX */
	{
		.name = DAI_TX_CLK,	/* Hide */
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_GATE(REG_DAI_CLKEN, DAI_TX_CLKEN_SHIFT,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAITX_LOCK_SHIFT),
		/*
		 * 0: "dai_tx_audiopll"
		 * 1: "dai_tx_digdiv"
		 */
		.mux = MT_INIT_MUX(REG_DAI_CLKSEL, DAI_TX_CLKSEL_SHIFT, 1, g_dai_tx_mux,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT)
	},
	/* DAI DAC */
	{
		.name = DAI_DAC_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_DAI_CLKEN, DAI_DAC_CLKEN_SHIFT,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAIDAC_LOCK_SHIFT)
	},
	/* DAI PDM */
	{
		.name = DAI_PDM_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_DAI_CLKEN, DAI_PDM_CLKEN_SHIFT,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAIPDM_LOCK_SHIFT)
	},
	/* DAI TX DIGDIV */
	{
		.name = DAI_TX_DIGDIV_CLK,	/* Hide */
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_DAI_CLKEN, DAI_TX_DIGDIV_CLKEN_SHIFT,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT),
		/*
		 * DAI tx dig div increment
		 * Freq(o_clk) = (dai_tx_clk_increment / 33554432) * 960MHz
		 */
		.rate = MT_INIT_RATE(REG_DAI_TX_DIGDIV, DAI_TX_CLK_INCREMENT_SHIFT, 25, g_digdiv_rate,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT)
	},
	/* DAI RX DIGDIV */
	{
		.name = DAI_RX_DIGDIV_CLK,	/* Hide */
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_DAI_CLKEN, DAI_RX_DIGDIV_CLKEN_SHIFT,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT),
		/*
		 * DAI rx dig div increment
		 * Freq(o_clk) = (dai_rx_clk_increment / 33554432) * 960MHz
		 */
		.rate = MT_INIT_RATE(REG_DAI_RX_DIGDIV, DAI_RX_CLK_INCREMENT_SHIFT, 25, g_digdiv_rate,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT)
	},
	/* DAI TX AUDIOPLL */
	{
		.name = DAI_TX_AUDIOPLL_CLK,	/* Hide */
		.flags = FLAG_CLK_DIV,
		.gate = {0},
		.div = MT_INIT_DIV(REG_DAI_CLKSEL, DAI_TX_AUDIOPLL_CLKDIV_SHIFT, 4, g_dai_audiopll_div,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT)
	},
	/* DAI RX AUDIOPLL */
	{
		.name = DAI_RX_AUDIOPLL_CLK,	/* Hide */
		.flags = FLAG_CLK_DIV,
		.gate = {0},
		.div = MT_INIT_DIV(REG_DAI_CLKSEL, DAI_RX_AUDIOPLL_CLKDIV_SHIFT, 4, g_dai_audiopll_div,
							REG_DAI_SLOCK, REG_DAI_LOCK, DAI_CLKSEL_LOCK_SHIFT)
	},
#endif

	/* GPU */
	{
		.name = MT_CLK_GPU,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_GPU_CLKEN, GPU_CLKEN_SHIFT,
							REG_GPU_SLOCK, REG_GPU_LOCK, GPU_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_GPU_CLKSEL, GPU_CLKSEL_SHIFT, 3, g_gpu_rate,
							REG_GPU_SLOCK, REG_GPU_LOCK, GPU_CLKSEL_LOCK_SHIFT)
	},

	/* GMAC */
	{
		.name = MT_CLK_GMAC,
		.flags = FLAG_CLK_MUX|FLAG_CLK_ATTR,
#if 0
		/* bit: 0 */
		.gate = MT_INIT_GATE(REG_GMAC_CLKEN, GMAC_CLKEN_SHIFT,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT),
#else
		/* bit: 0-21 */
		.gate = MT_INIT_MUX(REG_GMAC_CLKEN, GMAC_CLKEN_SHIFT, 22, NULL,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT),
#endif
		/*
		 * "gmac_phy_intfsel"
		 * 000: GMII/MII(NOT Support)
		 * 001: RGMII
		 * 100: RMII
		 */
		.mux = MT_INIT_MUX(REG_GMAC_CLKSEL, GMAC_PHY_INTFSEL_SHIFT, 3, g_gmac_mux,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKSEL_LOCK_SHIFT),
		/*
		 * "gmac_speed_cfg"
		 * 00: 1000Mbps
		 * 01: 2500Mbps (reserved)
		 * 10: 10Mbps
		 * 11: 100Mbps
		 */
		.attr = MT_INIT_ATTR(REG_GMAC_CLKSEL, GMAC_SPEED_CFG_SHIFT, 2,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKSEL_LOCK_SHIFT)
	},
	/* GMAC PHYREF */
	{
		.name = GMAC_PHY_REFCLK,		/* Hide */
		.flags = FLAG_CLK_RATE,
#if 0
		.gate = MT_INIT_GATE(REG_GMAC_CLKEN, GMAC_PHYREF_CLKEN_SHIFT,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		/*
		 * 00: no output clk
		 * 01: output 125MHz clk
		 * 10: output 50MHz clk
		 * 11: output 25MHz clk
		 */
		.rate = MT_INIT_RATE(REG_GMAC_CLKSEL, GMAC_PHY_REF_CLKSEL_SHIFT, 2, g_gmac_phyref_rate,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKSEL_LOCK_SHIFT)
	},
	/* GMAC RX */
	{
		.name = GMAC_RX_CLK, 	/* Hide */
		.flags = FLAG_CLK_ATTR,
#if 0
		.gate = MT_INIT_GATE(REG_GMAC_CLKEN, GMAC_RX_CLKEN_SHIFT,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		/*
		 * 00,01: pll_eth div gmac_rxclk
		 * 10: IO clk
		 * 11: IO clk revert
		 * !Caution: has clock glitches! <CNComment>ÓÐÃ«´Ì!
		 */
		.attr = MT_INIT_ATTR(REG_GMAC_CLKSEL, GMAC_RX_CLKSEL_SHIFT, 2,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKSEL_LOCK_SHIFT)
	},
	/* GMAC PTP */
	{
		.name = GMAC_PTP_CLK,	/* Hide */
		.flags = FLAG_CLK_RATE,
#if 0
		.gate = MT_INIT_GATE(REG_GMAC_CLKEN, GMAC_PTP_CLKEN_SHIFT,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		.rate = MT_INIT_RATE(REG_GMAC_CLKSEL, GMAC_PTP_CLKSEL_SHIFT, 2, g_gmac_ptp_rate,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKSEL_LOCK_SHIFT)
	},
#if 0
	/* GMAC TX */
	{
		.name = GMAC_TX_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_GMAC_CLKEN, GMAC_TX_CLKEN_SHIFT,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT)
	},
	/* GMAC RMII */
	{
		.name = GMAC_RMII_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_GMAC_CLKEN, GMAC_RMII_CLKEN_SHIFT,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT)
	},
	/* GMAC PHYTX */
	{
		.name = GMAC_PHYTX_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_GMAC_CLKEN, GMAC_PHYTX_CLKEN_SHIFT,
							REG_GMAC_SLOCK, REG_GMAC_LOCK, GMAC_CLKEN_LOCK_SHIFT)
	},
#endif

	/* TSI */
	{
		.name = MT_CLK_TSI,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_TSI_CLKEN, TSI_CLKEN_SHIFT,
							REG_TSI_SLOCK, REG_TSI_LOCK, TSI_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_TSI_CLKSEL, TSI_CLKSEL_SHIFT, 2, g_tsi_rate,
							REG_TSI_SLOCK, REG_TSI_LOCK, TSI_CLKSEL_LOCK_SHIFT)
	},
	/* TS0 */
	{
		.name = TS0_CLK,		/* Hide */
		.flags = FLAG_CLK_ATTR,
		.gate = {0},
		/*
		 * 000: pad_ts0_clk
		 * 001: pad_ts0_clk revert
		 * 010: democ_ts2_clk
		 * 011: democ_ts2_clk revert
		 * 100: demos2_1_ts_clk
		 * 101: demos2_1_ts_clk revert
		 * !Caution: has clock glitches! <CNComment>ÓÐÃ«´Ì!
		 */
		.attr = MT_INIT_ATTR(REG_TSI_CLKSEL, TS0_CLKSEL_SHIFT, 3,
							REG_TSI_SLOCK, REG_TSI_LOCK, TS0_CLKSEL_LOCK_SHIFT)
	},
	/* TS1 */
	{
		.name = TS1_CLK,		/* Hide */
		.flags = FLAG_CLK_ATTR,
		.gate = {0},
		/*
		 * 00: pad_ts1_clk
		 * 01: demos_ts1_clk
		 * 10: pad_ts1_clk revert
		 * 11: demos_ts1_clk revert
		 * !Caution: has clock glitches! <CNComment>ÓÐÃ«´Ì!
		 */
		.attr = MT_INIT_ATTR(REG_TSI_CLKSEL, TS1_CLKSEL_SHIFT, 2,
							REG_TSI_SLOCK, REG_TSI_LOCK, TS1_CLKSEL_LOCK_SHIFT)
	},
	/* TS2 */
	{
		.name = TS2_CLK,		/* Hide */
		.flags = FLAG_CLK_ATTR,
		.gate = {0},
		/*
		 * 000: pad_ts2_clk
		 * 001: pad_ts2_clk revert
		 * 010: democ_ts2_clk
		 * 011: democ_ts2_clk revert
		 * 100: cicam_tsout_clk
		 * 101: cicam_tsout_clk revert
		 * !Caution: has clock glitches! <CNComment>ÓÐÃ«´Ì!
		 */
		.attr = MT_INIT_ATTR(REG_TSI_CLKSEL, TS2_CLKSEL_SHIFT, 3,
							REG_TSI_SLOCK, REG_TSI_LOCK, TS2_CLKSEL_LOCK_SHIFT)
	},
	/* TS3 */
	{
		.name = TS3_CLK,		/* Hide */
		.flags = FLAG_CLK_ATTR,
		.gate = {0},
		/*
		 * 00: pad_ts3_clk
		 * 01: demos2_1_ts_clk
		 * 10: pad_ts3_clk revert
		 * 11: demos2_1_ts_clk revert
		 * !Caution: has clock glitches! <CNComment>ÓÐÃ«´Ì!
		 */
		.attr = MT_INIT_ATTR(REG_TSI_CLKSEL, TS3_CLKSEL_SHIFT, 2,
							REG_TSI_SLOCK, REG_TSI_LOCK, TS3_CLKSEL_LOCK_SHIFT)
	},

	/* CI */
	{
		.name = MT_CLK_CI,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_CI_CLKEN, CI_CLKEN_SHIFT,
							REG_CI_SLOCK, REG_CI_LOCK, CI_CLKEN_LOCK_SHIFT),
	},

	/* DESCRAMBLE */
	{
		.name = MT_CLK_DSC,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, DS_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, DS_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_SECURE_CLKSEL, DESCRAMBLE_CLKSEL_SHIFT, 2, g_dsc_rate,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, DS_CLKSEL_LOCK_SHIFT)
	},
	/* M2M Cipher */
	{
		.name = MT_CLK_M2M,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, M2M_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, M2M_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_SECURE_CLKSEL, M2M_CIPHER_CLKSEL_SHIFT, 2, g_m2m_rate,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, M2M_CIPHER_CLKSEL_LOCK_SHIFT)
	},
	/* PKA */
	{
		.name = MT_CLK_PKA,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, PKA_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, PKA_CLKEN_LOCK_SHIFT),
		.rate = MT_INIT_RATE(REG_SECURE_CLKSEL, PKA_CLKSEL_SHIFT, 1, g_pka_rate,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, PKA_LOCK_SHIFT)
	},
	/* Glitchdet */
	{
		.name = MT_CLK_GLITCHDET,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, GLITCHDET_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, GLITCHDET_CLKEN_LOCK_SHIFT)
	},
	/* KT */
	{
		.name = MT_CLK_KT,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, KT_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, KT_CLKEN_LOCK_SHIFT)
	},
	/* Sechd0 */
	{
		.name = MT_CLK_SECHD0,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, SECHD0_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, SECHD0_CLKEN_LOCK_SHIFT)
	},
	/* KLE */
	{
		.name = MT_CLK_KLE,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, KLE_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, KL_CLKEN_LOCK_SHIFT)
	},
	/* KDF */
	{
		.name = MT_CLK_KDF,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, KDF_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, KDF_CLKEN_LOCK_SHIFT)
	},
	/* SECURE */
	{
		.name = MT_CLK_SECURE,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_SECURE_CLKEN, SECMISC_CLKEN_SHIFT,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, SECMISC_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_SECURE_CLKSEL, SECURE_CLKSEL_SHIFT, 2, g_secure_rate,
							REG_SECURE_SLOCK, REG_SECURE_LOCK, SECURE_CLKSEL_LOCK_SHIFT)
	},

	/* IFCP_KLM */
	{
		.name = MT_CLK_IFCP_KLM,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_IFCP_CLKEN, IFCP_KLM_CLKEN_SHIFT,
							REG_IFCP_SLOCK, REG_IFCP_LOCK, IFCP_KLM_CLKEN_LOCK_SHIFT)
	},
	/* IFCP_CRYPTO */
	{
		.name = MT_CLK_IFCP_CRYPTO,
		.flags = FLAG_CLK_RATE,
		.gate = MT_INIT_GATE(REG_IFCP_CLKEN, IFCP_CRYPTO_CLKEN_SHIFT,
							REG_IFCP_SLOCK, REG_IFCP_LOCK, IFCP_CRYPTO_CLKEN_LOCK_SHIFT),
		.comb = MT_INIT_RATE(REG_IFCP_CLKSEL, IFCP_CRYPTO_CLKSEL_SHIFT, 2, g_ifcp_crypto_rate,
							REG_IFCP_SLOCK, REG_IFCP_LOCK, IFCP_CRYPTO_CLKSEL_LOCK_SHIFT)
	},
	/* IFCP_SYS */
	{
		.name = MT_CLK_IFCP_SYS,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_IFCP_CLKSEL, IFCP_SYS_CLKSEL_SHIFT, 3, g_ifcp_sys_rate,
							REG_IFCP_SLOCK, REG_IFCP_LOCK, IFCP_SYS_CLKSEL_LOCK_SHIFT)
	},

	/* AO_LPM */
	{
		.name = MT_CLK_AO_LPM,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_LPMCLK, LPM_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, LPM_LOCK_SHIFT)
	},
	/* AO_IRDA */
	{
		.name = MT_CLK_AO_IRDA,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_IRDACLK, IRDA_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, IRDA_LOCK_SHIFT)
	},
	/* AO_LEDKB */
	{
		.name = MT_CLK_AO_LEDKB,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_LEDKBCLK, LEDKB_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, LEDKB_LOCK_SHIFT)
	},
	/* AO_GPIO */
	{
		.name = MT_CLK_AO_GPIO,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_GPIOCLK, AOGPIO_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, GPIO_LOCK_SHIFT)
	},
	/* AO_KADC */
	{
		.name = MT_CLK_AO_KADC,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_KADCCLK, KADC_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, KADC_LOCK_SHIFT)
	},
	/* AO_ANAREG */
	{
		.name = MT_CLK_AO_ANAREG,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_ANAREGCLK, AO_ANAREG_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, ANA_LOCK_SHIFT)
	},
	/* AO_FPI2C */
	{
		.name = MT_CLK_AO_FPI2C,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_FPI2CCLK, FPI2C_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, FPI2C_LOCK_SHIFT)
	},
	/* AO_FPSPI */
	{
		.name = MT_CLK_AO_FPSPI,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_FPSPICLK, FPSPI_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, FPSPI_LOCK_SHIFT)
	},
	/* AO_RECRAM */
	{
		.name = MT_CLK_AO_RECRAM,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_RECRAMCLK, RECRAM_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, RECRAM_LOCK_SHIFT)
	},
	/* AO_PINMUX */
	{
		.name = MT_CLK_AO_PINMUX,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_PINMUXCLK, PINMUX_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, PINMUX_LOCK_SHIFT)
	},
	/* AO_TIMER */
	{
		.name = MT_CLK_AO_TIMER,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_TIMERCLK, TIMER_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, TIMER_LOCK_SHIFT)
	},
	/* AO_MAILBOX */
	{
		.name = MT_CLK_AO_MAILBOX,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_MAILBOXCLK, MAILBOX_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, MAILBOX_LOCK_SHIFT)
	},
	/* AO_CEC */
	{
		.name = MT_CLK_AO_CEC,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_CECCLK, CEC_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, CEC_LOCK_SHIFT)
	},
	/* AO_AVS */
	{
		.name = MT_CLK_AO_AVS,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_AVSCLK, AVS_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, AVS_LOCK_SHIFT)
	},
	/* AO_AGTIMER */
	{
		.name = MT_CLK_AO_AGTIMER,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_AGTIMERCLK, AGTIMER_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, AGTIMER_LOCK_SHIFT)
	},
	/* AO_MCU */
	{
		.name = MT_CLK_AO_MCU,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_AO_AOMCUCLK, AOMCU_CLKEN_SHIFT,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, AOMCU_LOCK_SHIFT)
	},
	/* XTAL */
	{
		.name = MT_CLK_XTAL,
		.flags = FLAG_CLK_RATE|FLAG_CLK_ATTR,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_XTAL_CLKSEL, XTAL_FREQ_SHIFT, 2, g_xtal_rate,
							0, 0, 0),	/* Read Only */
		/* 0: osc, 1: security osc */
		.attr = MT_INIT_ATTR(REG_XTAL_CLKSEL, XTAL_CLKSEL_SHIFT, 1,
							REG_AOCRM_SLOCK, REG_AOCRM_LOCK, XTALSEL_LOCK_SHIFT)
	},

	/* DDRPHY */
	{
		.name = MT_CLK_DDRPHY,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_DDR_PHY_BASE, 8, 4, g_ddrphy_rate,
							0, 0, 0),
	},

	/* ARMPLL */
	{
		.name = MT_ARMPLL,
		.flags = FLAG_CLK_RATE|FLAG_CLK_GATE_ACTIVE_LOW,
		.gate = MT_INIT_GATE(REG_APCPU_ARMPLL_PD, ARMPLL_PD_SHIFT,
							0, 0, 0 /*FIXME*/),
		.rate = MT_INIT_RATE(REG_APCPU_ARMPLL, 0, 32, g_armpll_rate,	/* FIXME */
							REG_APCPU_SLOCK, REG_APCPU_LOCK, ARMPLL_CFG_LOCK_SHIFT),
	},

};

#else

/* CONFIG_TARGET_SYMPHONY6_LITE || CONFIG_TARGET_SYMPHONY6_MINI */

/* hdmi pixnx divider */
static struct mt_clk_map_table g_hdmi_pixnx_div[] =
{
	{0, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{1, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{2, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{3, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{4, 200, MT_ANA_VENC_OS},		/* venc_osclk / 2 */
	{5, 400, MT_ANA_VENC_OS},		/* venc_osclk / 4 */
	{6, 600, MT_ANA_VENC_OS},		/* venc_osclk / 6 */
	{7, 800, MT_ANA_VENC_OS},		/* venc_osclk / 8 */
	{-1, -1}
};

/* hdmi tmds divider */
static struct mt_clk_map_table g_hdmi_tmds_div[] =
{
	{0, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{1, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{2, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{3, 100, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 1 */
	{4, 200, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 2 */
	{5, 400, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 4 */
	{6, 600, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 6 */
	{7, 800, ANA_HDMI_CLK_TMDS},		/* ana_hdmi_clk_tmds / 8 */
	{-1, -1}
};

/* hdvenc divider */
static struct mt_clk_map_table g_hdvenc_div[] =
{
	{0, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{1, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{2, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{3, 100, MT_ANA_VENC_OS},		/* venc_osclk / 1 */
	{4, 200, MT_ANA_VENC_OS},		/* venc_osclk / 2 */
	{5, 400, MT_ANA_VENC_OS},		/* venc_osclk / 4 */
	{6, 600, MT_ANA_VENC_OS},		/* venc_osclk / 6 */
	{7, 800, MT_ANA_VENC_OS},		/* venc_osclk / 8 */
	{-1, -1}
};

/* hdmi xtal mux */
static struct mt_clk_map_table g_hdmi_mux[] =
{
	{0, 0, MT_CLK_HDMI},
	{1, 1, MT_CLK_XTAL},
	{-1, -1}
};

/* sdvenc xtal mux */
static struct mt_clk_map_table g_sdvenc_mux[] =
{
	{0, 0, SDVENC_27M_CLK},
	{1, 1, MT_CLK_XTAL},
	{-1, -1}
};

struct mt_clk_in mt_clk_table[] =
{
	/* SDVENC */
	{
		.name = MT_CLK_SDVENC,
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, SDVENC_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, SDVENC_CLKEN_LOCK_SHIFT),
		/*
		 * 0: none XTAL
		 * 1: XTAL
		 */
		.mux = MT_INIT_MUX(REG_VOUT_CLKSEL, SDVENC_XTAL_CLKSEL_SHIFT, 1, g_sdvenc_mux,
						REG_VOUT_SLOCK, REG_VOUT_LOCK, SDVENC_XTAL_CLKSEL_LOCK_SHIFT)
	},
	/* HDVENC */
	{
		.name = MT_CLK_HDVENC,
		.flags = FLAG_CLK_DIV,
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDVENC_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDVENC_CLKEN_LOCK_SHIFT),
		.div = MT_INIT_DIV(REG_VOUT_CLKSEL, HDVENC_CLKSEL_SHIFT, 3, g_hdvenc_div,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDVENC_CLKSEL_LOCK_SHIFT)
	},
	/* HDMI */
	{
		.name = MT_CLK_HDMI,
		.flags = FLAG_CLK_MUX,
		.gate = MT_INIT_MUX(REG_VOUT_CLKEN, HDMI_PIXCLKNX_CLKEN_SHIFT, 11, NULL,
						REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT),
		/*
		 * 0: none XTAL
		 * 1: XTAL
		 */
		.mux = MT_INIT_MUX(REG_VOUT_CLKSEL, HDMI_XTAL_CLKSEL_SHIFT, 1, g_hdmi_mux,
						REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_XTAL_CLKSEL_LOCK_SHIFT)
	},
	/* HDMI TMDS CLK */
	{
		.name = HDMI_TMDS_CLK, /* Hide */
		.flags = FLAG_CLK_DIV,
#if 0
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_TMDSCLK_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		.div = MT_INIT_DIV(REG_VOUT_CLKSEL, HDMI_TMDS_CLKSEL_SHIFT, 3, g_hdmi_tmds_div,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_TMDS_CLKSEL_LOCK_SHIFT)
	},
	/* HDMI PIXNX CLK */
	{
		.name = HDMI_PIXNX_CLK, /* Hide */
		.flags = FLAG_CLK_DIV,
#if 0
		.gate = MT_INIT_GATE(REG_VOUT_CLKEN, HDMI_PIXCLKNX_CLKEN_SHIFT,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_CLKEN_LOCK_SHIFT),
#else
		.gate = {0},
#endif
		.div = MT_INIT_DIV(REG_VOUT_CLKSEL, HDMI_PIXNX_CLKSEL_SHIFT, 3, g_hdmi_pixnx_div,
							REG_VOUT_SLOCK, REG_VOUT_LOCK, HDMI_PIXNX_CLKSEL_LOCK_SHIFT)
	},
};

#endif

u32 mt_clk_table_size = sizeof(mt_clk_table)/sizeof(mt_clk_table[0]);

/* autogate parameters */
/* A0 */
mt_io_param_t mt_autogate_params_a0[] =
{
	/* SPDMA */
	{ {(void*)(REG_CRM_BASE+0xF904), 1, 1}, 0},
	{ {(void*)(REG_CRM_BASE+0xF900), 1, 1}, 1},
	/* FW_REG */
	{ {(void*)(REG_CRM_BASE+0xF904), 0, 1}, 0},
	/* PKA */
	{ {(void*)(REG_RSA_BASE+0x0C), 3, 3}, 0},
	/* EDMA */
	{ {(void*)(REG_EDMA_BASE+0x38), 0, 3}, 0},
	/* TSI_DS */
	{ {(void*)(REG_TSI_DS_BASE+0x48), 0, 18}, 0x3FFFF},
	/* M2M */
	{ {(void*)(REG_SECURE_BASE+0x3010), 0, 12}, 0},
	/* LEDKB */
	{ {(void*)(REG_AO_LEDKB_BASE+0x90), 0, 2}, 0},
	/* SPI_FP */
	{ {(void*)(REG_AO_FPSPI_BASE+0x12C), 8, 2}, 0},
	/* VDEC */
	{ {(void*)(REG_VDEC_BASE+0x03F8), 0, 1}, 0},
	/* TSI */
	{ {(void*)(REG_TSI_GLB_BASE+0x01E4), 0, 14}, 0},
/*
 * S6 A0 Chip has issue
 *   A0: 0xF, always on
 *   A1: 0, auto
 */
#if 1
	/* GRA */
	{ {(void*)(REG_GRA_BASE+0x02F0), 0, 4}, 0xF},
#endif
	/* HD_VENC */
	{ {(void*)(REG_VENC_HD_BASE), 31, 1}, 1},
	/* SD_VENC */
	{ {(void*)(REG_VENC_SD_BASE+0x100EC), 8, 1}, 1},
	/* SMC */
	{ {(void*)(REG_SMC_BASE+0x3C), 2, 1}, 0},
	/* NFLASH */
	//{ {(void*)(REG_PNAND_BASE+0xD008), 16, 5}, 0},
	{ {(void*)(REG_PNAND_BASE+0xD008), 16, 5}, 0xA},			/* 16, 18, 20: 0 / 17, 19: 1 -- '01010', for SPINAND ECC Clock always on */
	/* SFLASH */
	{ {(void*)(REG_SFLASH_BASE+0x012C), 8, 2}, 0},
	/* OTP */
	{ {(void*)(REG_OTPC_BASE+0x4500), 0, 1}, 0},
	/* KLE */
	{ {(void*)(REG_SECURE_BASE+0xB0F0), 0, 1}, 0},
#if 0
	/* IFCP_CRYPTO - VSCPU */
	{ {(void*)(IFCP_KL_M2M_ADDR+0x4000), 0, 4}, 0},
#endif
	/* JPEG */
	{ {(void*)(REG_JPEG_BASE+0xF0), 0, 4}, 0},
	/* PNG */
	{ {(void*)(REG_PNG_BASE+0x10), 0, 3}, 0},

};

/* A1 */
mt_io_param_t mt_autogate_params[] =
{
	/* SPDMA */
	{ {(void*)(REG_CRM_BASE+0xF904), 1, 1}, 0},
	{ {(void*)(REG_CRM_BASE+0xF900), 1, 1}, 1},
	/* FW_REG */
	{ {(void*)(REG_CRM_BASE+0xF904), 0, 1}, 0},
	/* PKA */
	{ {(void*)(REG_RSA_BASE+0x0C), 3, 3}, 0},
	/* EDMA */
	{ {(void*)(REG_EDMA_BASE+0x38), 0, 3}, 0},
	/* TSI_DS */
	{ {(void*)(REG_TSI_DS_BASE+0x48), 0, 18}, 0x3FFFF},
	/* M2M */
	{ {(void*)(REG_SECURE_BASE+0x3010), 0, 12}, 0},
	/* LEDKB */
	{ {(void*)(REG_AO_LEDKB_BASE+0x90), 0, 2}, 0},
	/* SPI_FP */
	{ {(void*)(REG_AO_FPSPI_BASE+0x12C), 8, 2}, 0},
	/* VDEC */
	{ {(void*)(REG_VDEC_BASE+0x03F8), 0, 1}, 0},
	/* TSI */
	{ {(void*)(REG_TSI_GLB_BASE+0x01E4), 0, 14}, 0},
/*
 * S6 A0 Chip has issue
 *   A0: 0xF, always on
 *   A1: 0, auto
 */
#if 0
	/* GRA */
	{ {(void*)(REG_GRA_BASE+0x02F0), 0, 4}, 0xF},
#else
	{ {(void*)(REG_GRA_BASE+0x02F0), 0, 4}, 0},
#endif
	/* HD_VENC */
	{ {(void*)(REG_VENC_HD_BASE), 31, 1}, 1},
	/* SD_VENC */
	{ {(void*)(REG_VENC_SD_BASE+0x100EC), 8, 1}, 1},
	/* SMC */
	{ {(void*)(REG_SMC_BASE+0x3C), 2, 1}, 0},
	/* NFLASH */
	//{ {(void*)(REG_PNAND_BASE+0xD008), 16, 5}, 0},
	{ {(void*)(REG_PNAND_BASE+0xD008), 16, 5}, 0xA},			/* 16, 18, 20: 0 / 17, 19: 1 -- '01010', for SPINAND ECC Clock always on */
	/* SFLASH */
	{ {(void*)(REG_SFLASH_BASE+0x012C), 8, 2}, 0},
	/* OTP */
	{ {(void*)(REG_OTPC_BASE+0x4500), 0, 1}, 0},
	/* KLE */
	{ {(void*)(REG_SECURE_BASE+0xB0F0), 0, 1}, 0},
#if 0
	/* IFCP_CRYPTO - VSCPU */
	{ {(void*)(IFCP_KL_M2M_ADDR+0x4000), 0, 4}, 0},
#endif
	/* JPEG */
	{ {(void*)(REG_JPEG_BASE+0xF0), 0, 4}, 0},
	/* PNG */
	{ {(void*)(REG_PNG_BASE+0x10), 0, 3}, 0},

	/* A1+ */
	/* AHB */
	{ {(void*)(REG_CRM_BASE+0xF904), 2, 1}, 0},
};

/* read only */
mt_io_param_t mt_autogate_params_rd[] =
{
/*
 * FIXME: bug apcpu hold after disable GPU
 *		  20240301: modify GPU up/down flow to avoid this issue.
 *		  20240305: move to bootinit/auxcode
 */
	/* OIC */
	{ {(void*)(REG_AXI_SECURE_BASE+0x02C0), 0, 32}, 0x800},
};

u32 mt_autogate_params_size = sizeof(mt_autogate_params)/sizeof(mt_autogate_params[0]);
u32 mt_autogate_params_size_a0 = sizeof(mt_autogate_params_a0)/sizeof(mt_autogate_params_a0[0]);

/* clock backup registers for suspend/resume */
mt_reg_value_t suspend_clock_regs[] =
{
	{REG_DBG_PROT_BASE, 0xD5C9},		/* debugi2c, jtag */

//TOP
	{REG_TOPCLK_CTRL0, 0xFFFFFFFF},
	{REG_TOPCLK_CTRL1, 0xFFFFFFFF},
	{REG_TOPCLK_CTRL2, 0xFFF9FFFF},
	{REG_TOPCLK_CTRL3, 0xFFFFFFFF},
	{REG_TOPCLK_CTRL4, 0xFFFFFFFF},
	{REG_TOPCLK_CTRL5, 0xFF40FFFF},
	{REG_WDOGEXP_CFG1, 0x38000},

	{REG_MAC_CLKEN, 	0},
	//{REG_MAC_CLKSEL,	0x102},

	{REG_AVCPU_CLKEN,	0},
	{REG_AVCPU_CLKSEL, 	0x5},

	//FIXME: kernel panic!
	//{REG_SYS_CLKEN, 	0x1EF},
	//{REG_SYS_CLKSEL,	0},

	{REG_INTF_CLKEN,	0x7F9},
	//Intf clocks might too high!
	//{REG_INTF_CLKSEL,	0x55005000},

	{REG_GRA_CLKEN, 	0},
	{REG_GRA_CLKSEL,	0x5},

	{REG_JPG_CLKEN, 	0},
	{REG_JPG_CLKSEL,	0},

	{REG_DISP_CLKEN,	0},
	{REG_DISP_CLKSEL,	0x1001},

	{REG_VDEC_CLKEN,	0},
	{REG_VDEC_CLKSEL,	0x3},

	{REG_AOUT_CLKEN,	0},
	{REG_AOUT_CLKSEL,	0x1},
	{REG_AOUT_SPDF_DIGDIV,	0x346DC5},

	{REG_VOUT_CLKEN,	0xFFFF},
	{REG_VOUT_CLKSEL,	0x100},

	{REG_PNG_CLKEN, 	0},
	{REG_PNG_CLKSEL,	0},

	{REG_DAI_CLKEN, 	0},
//	{REG_DAI_TX_DIGDIV, 0x346DC5},
//	{REG_DAI_RX_DIGDIV, 0x346DC5},
//	{REG_DAI_CLKSEL,	0},

	{REG_GPU_CLKEN, 	0},
	{REG_GPU_CLKSEL,	0x5},

	{REG_GMAC_CLKEN,	0xFF0000},
//	{REG_GMAC_CLKDLY,	0},
//	{REG_GMAC_CLKSEL,	0x24},

	{REG_TSI_CLKEN, 	0},
	{REG_TSI_CLKSEL,	0x2},

	{REG_CI_CLKEN,		0},

	{REG_SECURE_CLKEN,	0x1FF},
	{REG_SECURE_CLKSEL, 0x29},

	//FIXME: REE or TEE?
	//{REG_IFCP_CLKEN,	0x3},
	//{REG_IFCP_CLKSEL,	0xB},

	//FIXME: kernel panic!
#if 0
	{REG_AO_LPMCLK, 	0x1},
	{REG_AO_IRDACLK, 	0x1},
	{REG_AO_LEDKBCLK, 	0x1},
	{REG_AO_GPIOCLK, 	0x1},
	{REG_AO_KADCCLK, 	0x1},
	{REG_AO_ANAREGCLK,  0x1},
	{REG_AO_FPI2CCLK, 	0x1},
	{REG_AO_FPSPICLK, 	0x1},
	{REG_AO_RECRAMCLK,  0x1},
	{REG_AO_PINMUXCLK,  0x1},
	{REG_AO_TIMERCLK,  	0x1},
	{REG_AO_MAILBOXCLK, 0x1},
	{REG_AO_CECCLK, 	0x1},
	{REG_AO_AVSCLK,		0x1},
	{REG_AO_AGTIMERCLK, 0x1},
	//{REG_AO_AOMCUCLK, 	0x1},
#endif
};

u32 suspend_clock_regs_size = sizeof(suspend_clock_regs)/sizeof(suspend_clock_regs[0]);

/*
 * Patch after enable clock
 *   1.GPU: shall enable chain0/chain1, disable ISO_EN
 *   2.SMC: shall enable IO & VDD
 *   3.MAC: shall release EPHY
 */
void patch_clk_enable(mt_clk_t *clk)
{
	/* GPU */
	if (strcmp(clk, MT_CLK_GPU) == 0)
	{
		MT_LOGW("%s: %s!\n", __FUNCTION__, clk);
		/* [0]: chain0, [1]: chain1, [2]: ISO_EN */
		//MT_SET_BITS(REG_PUB_MISC_BASE+0xE4, 0, 3, 0);
		MT_SET_BIT(REG_PUB_MISC_BASE+0xE4, 0, 0);
		udelay(2);
		MT_SET_BIT(REG_PUB_MISC_BASE+0xE4, 1, 0);
		udelay(2);
		MT_SET_BIT(REG_PUB_MISC_BASE+0xE4, 2, 0);
	}
	/* SMC */
	else if (strcmp(clk, MT_CLK_SMC) == 0)
	{
		MT_LOGW("%s: %s!\n", __FUNCTION__, clk);
		/* 3.3V IO & VDD */
		MT_SET_BIT(REG_SMC_BASE+0x20, 0, 1);
		/* 5V IO & VDD */
		MT_SET_BITS(REG_SMC_BASE+0xAC, 0, 8, 0x7);
	}
	/* EPHY(MAC) */
	else if (strcmp(clk, MT_CLK_MAC) == 0)
	{
		MT_LOGW("%s: %s!\n", __FUNCTION__, clk);
		/* release ephy */
		MT_SET_BIT(REG_MAC_SRSTN, EPHY_SRSTN_SHIFT, 1);
	}
}

/*
 * Patch before disable clock
 *   1.GPU: must reset, disable chain0/chain1, enable ISO_EN, before disable it
 *   2.SMC: shall disable IO & VDD
 *   3.MAC: shall reset EPHY
 */
void patch_clk_disable(mt_clk_t *clk)
{
	/* GPU */
	if (strcmp(clk, MT_CLK_GPU) == 0)
	{
		MT_LOGW("%s: %s!\n", __FUNCTION__, clk);
		/* reset */
		//writel(0, (volatile void*)REG_GPU_SRSTN);		/* BUG, if bit [4:3] not set 0b11, install gpu ko dead! */
		MT_SET_BITS(REG_GPU_SRSTN, 0, 3, 0);			/* clear [2:0] only */
		/* [0]: chain0, [1]: chain1, [2]: ISO_EN */
		//MT_SET_BITS(REG_PUB_MISC_BASE+0xE4, 0, 3, 0x7);
		MT_SET_BIT(REG_PUB_MISC_BASE+0xE4, 2, 0x1);
		MT_SET_BIT(REG_PUB_MISC_BASE+0xE4, 1, 0x1);
		udelay(2);
		MT_SET_BIT(REG_PUB_MISC_BASE+0xE4, 0, 0x1);
		udelay(2);
	}
	/* SMC */
	else if (strcmp(clk, MT_CLK_SMC) == 0)
	{
		MT_LOGW("%s: %s!\n", __FUNCTION__, clk);
		/* 3.3V IO & VDD */
		MT_SET_BIT(REG_SMC_BASE+0x20, 0, 0);
		/* 5V IO & VDD */
		MT_SET_BITS(REG_SMC_BASE+0xAC, 0, 8, 0);
	}
	/* EPHY(MAC) */
	else if (strcmp(clk, MT_CLK_MAC) == 0)
	{
		MT_LOGW("%s: %s!\n", __FUNCTION__, clk);
		/* reset ephy */
		MT_SET_BIT(REG_MAC_SRSTN, EPHY_SRSTN_SHIFT, 0);
	}
}

