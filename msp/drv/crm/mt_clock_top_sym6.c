/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_clock_top_sym6.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2023/02/10
 * Description    : MT Symphony6 top clocks definition.
 * History        :
 * 1.Date         : 2023/02/10
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
#include "mt_crm_top_reg.h"
#include "mt_crm_clock_reg.h"
#include "mt_crm_lock_reg.h"

#include "mt_clock_top.h"
#include "mt_mod_internal.h"

#if !defined(CONFIG_TARGET_SYMPHONY6_LITE) && !defined(CONFIG_TARGET_SYMPHONY6_MINI)

#define FIX_RATE_90M				(90 * ONE_MHZ)
#define FIX_RATE_262M				(262 * ONE_MHZ)
#define FIX_RATE_206M				(206 * ONE_MHZ)
#define FIX_RATE_144M				(144 * ONE_MHZ)
#define FIX_RATE_288M				(288 * ONE_MHZ)
#define FIX_RATE_80M				(80 * ONE_MHZ)
#define FIX_RATE_86P5M				86500000UL
#define FIX_RATE_173M				(173 * ONE_MHZ)
#define FIX_RATE_180M				(180 * ONE_MHZ)
#define FIX_RATE_50M				(50 * ONE_MHZ)

//Demo
#define DRV0_CLK_TOP				"drv0_clk_top"
#define DRV1_CLK_TOP				"drv1_clk_top"

/* top dma/xtal mux(rate) */
static struct mt_clk_map_table g_top_dma_mux[] =
{
	{0, 131000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top jpg/xtal mux(rate) */
static struct mt_clk_map_table g_top_jpg_mux[] =
{
	{0, 144000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top disp/xtal mux(rate) */
static struct mt_clk_map_table g_top_disp_mux[] =
{
	{0, 60000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top vdec/xtal mux(rate) */
static struct mt_clk_map_table g_top_vdec_mux[] =
{
	{0, 131000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top aout/xtal mux(rate) */
static struct mt_clk_map_table g_top_aout_mux[] =
{
	{0, 131000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top png/xtal mux(rate) */
static struct mt_clk_map_table g_top_png_mux[] =
{
	{0, 144000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top ifcpcrypto/xtal mux(rate) */
static struct mt_clk_map_table g_top_ifcpcrypto_mux[] =
{
	{0, 120000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top tsi/xtal mux(rate) */
static struct mt_clk_map_table g_top_tsi_mux[] =
{
	{0, 131000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top ds/xtal mux(rate) */
static struct mt_clk_map_table g_top_ds_mux[] =
{
	{0, 131000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top secure/xtal mux(rate) */
static struct mt_clk_map_table g_top_secure_mux[] =
{
	{0, 131000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top cipher/xtal mux(rate) */
static struct mt_clk_map_table g_top_cipher_mux[] =
{
	{0, 131000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

#if 1
/* top vhdpll d5 mux(rate) */
static struct mt_clk_map_table g_top_vhdpll_d5_mux[] =
{
	{0, 594000000},	/* vhdpll_d5 */
	{1, 615380000},	/* ethpll_d3p25 */
	{-1, -1}
};
#endif

/* top avcpu/xtal mux(rate) */
static struct mt_clk_map_table g_top_avcpu_mux[] =
{
	{0, 120000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

/* top gpu/xtal mux(rate) */
static struct mt_clk_map_table g_top_gpu_mux[] =
{
	{0, 120000000},
	{1, 0, MT_CLK_XTAL},
	{-1, -1}
};

#if 1
/* top ethpll d5 mux(rate) */
static struct mt_clk_map_table g_top_ethpll_d5_mux[] =
{
	{0, 400000000}, /* pll_eth_d5 */
	{1, 411000000}, /* pll_usb_d7 */
	{-1, -1}
};
#endif

/* top sdvenc_d4 mux */
static struct mt_clk_map_table g_top_sdvenc_d4_mux[] =
{
	{0, 0, "sdvenc_clk_d4"},
	{1, 1, MT_CLK_XTAL},
	{-1, -1}
};

/* top demo clk mux */
static struct mt_clk_map_table g_top_demo_mux[] =
{
	{0, 0, DRV0_CLK_ANA " & " DRV1_CLK_ANA},
	{1, 1, DRV0_CLK_TOP " & " DRV1_CLK_TOP},
	{-1, -1}
};

/* top drv0 rate */
static struct mt_clk_map_table g_top_drv0_rate[] =
{
	{0, 57600000},
	{1, 81000000},		/* from adcpll(cpupll) */
	{2, 81000000},		/* from vsdpll */
	{3, 81000000},		/* from vsdpll */
	{-1, -1}
};

/* top drv1 rate */
static struct mt_clk_map_table g_top_drv1_rate[] =
{
	{0, 192000000},
	{1, 270000000},
	{-1, -1}
};

/* top aout mclk mux */
static struct mt_clk_map_table g_top_aout_mclk_mux[] =
{
	{0, 0, AOUT_MCLK},
	{1, 1, AOUT_MCLK},
	{2, 1, DAI_TX_AUDIOPLL_CLK},
	{3, 1, DAI_TX_DIGDIV_CLK},
	{-1, -1}
};

/* top clock table */
struct mt_clk_in mt_top_clk_table[] =
{
	/* TOPCLK_CTRL0 */

	/* TOP_AXI */
	{
		.name = TOP_AXI_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL0, TOP_AXI_CLKEN_SHIFT,
							REG_TOPCLK_CTRL0_SLOCK, REG_TOPCLK_CTRL0_LOCK, CTRL0_GROUP0_LOCK_SHIFT)
	},
	/* TOP_DDROMC */
	{
		.name = TOP_DDROMC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL0, TOP_DDROMC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL0_SLOCK, REG_TOPCLK_CTRL0_LOCK, CTRL0_GROUP0_LOCK_SHIFT)
	},
	/* TOP_HDMI_AUD */
	{
		.name = TOP_HDMI_AUD_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL0, TOP_HDMI_AUD_CLKEN_SHIFT,
							REG_TOPCLK_CTRL0_SLOCK, REG_TOPCLK_CTRL0_LOCK, CTRL0_GROUP0_LOCK_SHIFT)
	},
	/* TOP_AHB */
	{
		.name = TOP_AHB_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL0, TOP_AHB_CLKEN_SHIFT,
							REG_TOPCLK_CTRL0_SLOCK, REG_TOPCLK_CTRL0_LOCK, CTRL0_GROUP2_LOCK_SHIFT)
	},
	/* TOP_APB */
	{
		.name = TOP_APB_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL0, TOP_APB_CLKEN_SHIFT,
							REG_TOPCLK_CTRL0_SLOCK, REG_TOPCLK_CTRL0_LOCK, CTRL0_GROUP4_LOCK_SHIFT)
	},
	/* TOP_APBACKUP */
	{
		.name = TOP_APBACKUP_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL0, TOP_APBACKUP_CLKEN_SHIFT,
							REG_TOPCLK_CTRL0_SLOCK, REG_TOPCLK_CTRL0_LOCK, CTRL0_GROUP5_LOCK_SHIFT)
	},

	/* TOPCLK_CTRL1 */

	/* TOP_AVCPU */
	{
		.name = TOP_AVCPU_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_AVCPU_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP0_LOCK_SHIFT)
	},
	/* TOP_SMC_90M */
	{
		.name = TOP_SMC_90M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_90M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_SMC_90M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP2_LOCK_SHIFT),
	},
	/* TOP_SMC_XTAL_D2 */
	{
		.name = TOP_SMC_XTAL_D2_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_SMC_XTAL_D2_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP2_LOCK_SHIFT)
	},
	/* TOP_SPI */
	{
		.name = TOP_SPI_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_SPI_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP2_LOCK_SHIFT)
	},
	/* TOP_DS */
	{
		.name = TOP_DS_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_DS_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP4_LOCK_SHIFT)
	},
	/* A1+ */
	/* TOP_PKA */
	{
		.name = TOP_PKA_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_PKA_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP4_LOCK_SHIFT)
	},
	/* TOP_SDIO_0 */
	{
		.name = TOP_SDIO_0_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_SDIO0_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP5_LOCK_SHIFT)
	},
	/* TOP_SDIO_1 */
	{
		.name = TOP_SDIO_1_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_SDIO1_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP5_LOCK_SHIFT)
	},
	/* TOP_GRA */
	{
		.name = TOP_GRA_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_GRA_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP6_LOCK_SHIFT)
	},
	/* TOP_JPG_262M */
	{
		.name = TOP_JPG_262M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_262M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_JPG_262M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP7_LOCK_SHIFT),
	},
	/* TOP_JPG_206M */
	{
		.name = TOP_JPG_206M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_206M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_JPG_206M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP7_LOCK_SHIFT),
	},
	/* TOP_JPG_144M */
	{
		.name = TOP_JPG_144M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_144M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_JPG_144M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP7_LOCK_SHIFT),
	},
	/* TOP_JPG_288M */
	{
		.name = TOP_JPG_288M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_288M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL1, TOP_JPG_288M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL1_SLOCK, REG_TOPCLK_CTRL1_LOCK, CTRL1_GROUP7_LOCK_SHIFT),
	},

	/* TOPCLK_CTRL2 */

	/* TOP_HDMI_XCLK */
	{
		.name = TOP_HDMI_XCLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_HDMI_XCLK_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP0_LOCK_SHIFT)
	},
	/* TOP_HDMI_CEC */
	{
		.name = TOP_HDMI_CEC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_HDMI_CEC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP0_LOCK_SHIFT)
	},
	/* TOP_HDMI_OSC */
	{
		.name = TOP_HDMI_OSC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_HDMI_OSC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP0_LOCK_SHIFT)
	},
	/* TOP_VDEC */
	{
		.name = TOP_VDEC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_VDEC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP1_LOCK_SHIFT),
	},
	/* TOP_AOUT */
	{
		.name = TOP_AOUT_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_AOUT_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP2_LOCK_SHIFT)
	},
#if 1
	/* TOP_AUDIOPLL_SCAN */
	{
		.name = TOP_AUDIOPLL_SCAN_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_AUDIOPLL_SCAN_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP4_LOCK_SHIFT)
	},
	/* TOP_USB0_CORE_SCAN */
	{
		.name = TOP_USB0_CORE_SCAN_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_USB0_CORE_SCAN_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP4_LOCK_SHIFT)
	},
#endif
	/* TOP_AVSYNC */
	{
		.name = TOP_AVSYNC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_AVSYNC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP4_LOCK_SHIFT)
	},
	/* TOP_PNG_262M */
	{
		.name = TOP_PNG_262M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_262M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_PNG_262M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP5_LOCK_SHIFT),
	},
	/* TOP_PNG_206M */
	{
		.name = TOP_PNG_206M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_206M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_PNG_206M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP5_LOCK_SHIFT),
	},
	/* TOP_PNG_144M */
	{
		.name = TOP_PNG_144M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_144M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_PNG_144M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP5_LOCK_SHIFT),
	},
	/* TOP_PNG_288M */
	{
		.name = TOP_PNG_288M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_288M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_PNG_288M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP5_LOCK_SHIFT),
	},
	/* TOP_SDVENC */
	{
		.name = TOP_SDVENC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_SDVENC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP6_LOCK_SHIFT)
	},
	/* TOP_HDVENC */
	{
		.name = TOP_HDVENC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_HDVENC_OSC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP6_LOCK_SHIFT)
	},
	/* TOP_TSI */
	{
		.name = TOP_TSI_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_TSI_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP6_LOCK_SHIFT)
	},
	/* TOP_HDMI_TMDS */
	{
		.name = TOP_HDMI_TMDS_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL2, TOP_HDMI_TMDS_CLKEN_SHIFT,
							REG_TOPCLK_CTRL2_SLOCK, REG_TOPCLK_CTRL2_LOCK, CTRL2_GROUP6_LOCK_SHIFT)
	},

	/* TOPCLK_CTRL3 */

	/* TOP_IFCP_SYS */
	{
		.name = TOP_IFCP_SYS_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL3, TOP_IFCP_SYS_CLKEN_SHIFT,
							REG_TOPCLK_CTRL3_SLOCK, REG_TOPCLK_CTRL3_LOCK, CTRL3_GROUP0_LOCK_SHIFT)
	},
	/* TOP_IFCP_CRYPTO */
	{
		.name = TOP_IFCP_CRYPTO_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL3, TOP_IFCP_CRYPTO_CLKEN_SHIFT,
							REG_TOPCLK_CTRL3_SLOCK, REG_TOPCLK_CTRL3_LOCK, CTRL3_GROUP2_LOCK_SHIFT)
	},
	/* TOP_OTP_80M */
	{
		.name = TOP_OTP_80M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_80M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL3, TOP_OTP_80M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL3_SLOCK, REG_TOPCLK_CTRL3_LOCK, CTRL3_GROUP4_LOCK_SHIFT),
	},
	/* TOP_OTP_86P5M */
	{
		.name = TOP_OTP_86P5M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_86P5M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL3, TOP_OTP_86P5_CLKEN_SHIFT,
							REG_TOPCLK_CTRL3_SLOCK, REG_TOPCLK_CTRL3_LOCK, CTRL3_GROUP4_LOCK_SHIFT),
	},
	/* TOP_SWTIMER */
	{
		.name = TOP_SWTIMER_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL3, TOP_SWTIMER_CLKEN_SHIFT,
							REG_TOPCLK_CTRL3_SLOCK, REG_TOPCLK_CTRL3_LOCK, CTRL3_GROUP5_LOCK_SHIFT)
	},
	/* TOP_IFCP_TIMER */
	{
		.name = TOP_IFCP_TIMER_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL3, TOP_IFCPTIMER_CLKEN_SHIFT,
							REG_TOPCLK_CTRL3_SLOCK, REG_TOPCLK_CTRL3_LOCK, CTRL3_GROUP6_LOCK_SHIFT)
	},
	/* TOP_GLITCH */
	{
		.name = TOP_GLITCH_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL3, TOP_GLITCH_CLKEN_SHIFT,
							REG_TOPCLK_CTRL3_SLOCK, REG_TOPCLK_CTRL3_LOCK, CTRL3_GROUP7_LOCK_SHIFT)
	},

	/* TOPCLK_CTRL4 */

	/* TOP_SOSC_173M */
	{
		.name = TOP_SOSC_173M_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_173M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_SOSC_173M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP0_LOCK_SHIFT),
	},
	/* TOP_SOSC_FAST_SCAN: 180M */
	{
		.name = TOP_SOSC_FAST_SCAN_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_180M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_SOSC_180M_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP0_LOCK_SHIFT),
	},
	/* TOP_SECURE */
	{
		.name = TOP_SECURE_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_SECURE_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP1_LOCK_SHIFT)
	},
	/* TOP_M2M_CIPHER */
	{
		.name = TOP_M2M_CIPHER_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_M2MCIPHER_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP2_LOCK_SHIFT)
	},
	/* TOP_DMA */
	{
		.name = TOP_DMA_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_DMA_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP3_LOCK_SHIFT)
	},
	/* TOP_GPU */
	{
		.name = TOP_GPU_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_GPU_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP4_LOCK_SHIFT)
	},
	/* TOP_UART */
	{
		.name = TOP_UART_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_UART_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP6_LOCK_SHIFT)
	},
	/* TOP_USB3 */
	{
		.name = TOP_USB3_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_USB3_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP6_LOCK_SHIFT)
	},
	/* TOP_DAI_RX */
	{
		.name = TOP_DAI_RX_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_DAI_RX_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP7_LOCK_SHIFT)
	},
	/* TOP_DAI_TX */
	{
		.name = TOP_DAI_TX_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_DAI_TX_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP7_LOCK_SHIFT)
	},
	/* TOP_AUDIO_SPDF */
	{
		.name = TOP_AUDIO_SPDF_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL4, TOP_AUDIO_SPDF_CLKEN_SHIFT,
							REG_TOPCLK_CTRL4_SLOCK, REG_TOPCLK_CTRL4_LOCK, CTRL4_GROUP7_LOCK_SHIFT)
	},

	/* TOPCLK_CTRL5 */

	/* TOP_DISP_CCLK */
	{
		.name = TOP_DISP_CCLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_DISP_CCLK_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP0_LOCK_SHIFT)
	},
	/* TOP_DISP_DI */
	{
		.name = TOP_DISP_DI_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_DISP_DI_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP0_LOCK_SHIFT)
	},
	/* TOP_DISP_OSDC */
	{
		.name = TOP_DISP_OSDC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_DISP_OSDC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP0_LOCK_SHIFT)
	},
	/* TOP_DISP_VDC */
	{
		.name = TOP_DISP_VDC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_DISP_VDC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP0_LOCK_SHIFT)
	},
	/* TOP_ETHERNET */
	{
		.name = TOP_ETHERNET_CLK,
		.flags = FLAG_CLK_FIXED_RATE,
		.fixed_rate = FIX_RATE_50M,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_ETHERNET_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP3_LOCK_SHIFT),
	},
	/* TOP_PNAND */
	{
		.name = TOP_PNAND_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_PNAND_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP5_LOCK_SHIFT)
	},
#if 1
	/* TOP_PNAND_ACSCAN */
	{
		.name = TOP_PNAND_ACSCAN_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_PNAND_ACSCAN_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP5_LOCK_SHIFT)
	},
#endif
	/* TOP_GMAC */
	{
		.name = TOP_GMAC_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL5, TOP_GMAC_CLKEN_SHIFT,
							REG_TOPCLK_CTRL5_SLOCK, REG_TOPCLK_CTRL5_LOCK, CTRL5_GROUP7_LOCK_SHIFT)
	},

	/* TOPCLK_CTRL6 */

	/* TOP_DMA_XTAL_MUX */
	{
		.name = TOP_DMA_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, DMA_XTAL_SEL_SHIFT, 1, g_top_dma_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT6_LOCK_SHIFT)
	},
	/* TOP_JPG_XTAL_MUX */
	{
		.name = TOP_JPG_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, JPG_XTAL_SEL_SHIFT, 1, g_top_jpg_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT8_LOCK_SHIFT)
	},
	/* TOP_DISP_XTAL_MUX */
	{
		.name = TOP_DISP_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, DISP_XTAL_SEL_SHIFT, 1, g_top_disp_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT9_LOCK_SHIFT)
	},
	/* TOP_VDEC_XTAL_MUX */
	{
		.name = TOP_VDEC_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, VDEC_XTAL_SEL_SHIFT, 1, g_top_vdec_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT10_LOCK_SHIFT)
	},
	/* TOP_AOUT_XTAL_MUX */
	{
		.name = TOP_AOUT_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, AOUT_XTAL_SEL_SHIFT, 1, g_top_aout_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT11_LOCK_SHIFT)
	},
	/* TOP_PNG_XTAL_MUX */
	{
		.name = TOP_PNG_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, PNG_XTAL_SEL_SHIFT, 1, g_top_png_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT12_LOCK_SHIFT)
	},
	/* TOP_IFCPCRYPTO_XTAL_MUX */
	{
		.name = TOP_IFCPCRYPTO_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, IFCPCRYPTO_XTAL_SEL_SHIFT, 1, g_top_ifcpcrypto_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT13_LOCK_SHIFT)
	},
	/* TOP_TSI_XTAL_MUX */
	{
		.name = TOP_TSI_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, TSI_XTAL_SEL_SHIFT, 1, g_top_tsi_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT14_LOCK_SHIFT)
	},
	/* TOP_DS_XTAL_MUX */
	{
		.name = TOP_DS_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, DESCRAMBLE_XTAL_SEL_SHIFT, 1, g_top_ds_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT15_LOCK_SHIFT)
	},
	/* TOP_SECURE_XTAL_MUX */
	{
		.name = TOP_SECURE_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, SECURE_XTAL_SEL_SHIFT, 1, g_top_secure_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT16_LOCK_SHIFT)
	},
	/* TOP_CIPHER_XTAL_MUX */
	{
		.name = TOP_CIPHER_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, CIPHER_XTAL_SEL_SHIFT, 1, g_top_cipher_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT17_LOCK_SHIFT)
	},
#if 1
	/* TOP_VHDPLL_D5_MUX */
	{
		.name = TOP_VHDPLL_D5_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_TOPCLK_CTRL6, TOP_VHDPLL_D5_SEL_SHIFT, 1, g_top_vhdpll_d5_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT18_LOCK_SHIFT)
	},
#endif
	/* TOP_AVCPU_XTAL_MUX */
	{
		.name = TOP_AVCPU_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, AVCPU_XTAL_SEL_SHIFT, 1, g_top_avcpu_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT19_LOCK_SHIFT)
	},
#if 0
	/* TOP_TEST */
	{
		.name = TOP_TEST_CLK,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL6, TESTCLK_CLOSED_SHIFT,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT20_LOCK_SHIFT)
	},
#endif
	/* TOP_GPU_XTAL_MUX */
	{
		.name = TOP_GPU_XTAL_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.comb = MT_INIT_RATE(REG_TOPCLK_CTRL6, GPU_XTAL_SEL_SHIFT, 1, g_top_gpu_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT23_LOCK_SHIFT)
	},
#if 1
	/* TOP_ETHPLL_D5_MUX */
	{
		.name = TOP_ETHPLL_D5_MUX,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_TOPCLK_CTRL6, TOP_ETHPLL_D5_SEL_SHIFT, 1, g_top_ethpll_d5_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT24_LOCK_SHIFT)
	},
#endif

	/* A1+ */
	/* TOP_SDVENC_D4_XTAL_MUX */
	{
		.name = TOP_SDVENC_D4_XTAL_MUX,
		.flags = FLAG_CLK_MUX,
		.gate = {0},
		/*
		 * 0: no select Xtal
		 * 1: select Xtal
		 */
		.mux = MT_INIT_MUX(REG_TOPCLK_CTRL6, SDVENC_CLK_D4_XTAL_SEL_SHIFT, 1, g_top_sdvenc_d4_mux,
							REG_TOPCLK_CTRL6_SLOCK, REG_TOPCLK_CTRL6_LOCK, CTRL6_BIT25_LOCK_SHIFT)
	},

#if 1
	/* A1+ */
	/* GPU_MBIST_CLK */
	{
		.name = GPU_MBIST_CLK,	/* Hide */
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL6, GPU_MBIST_CLKEN_SHIFT,
							0, 0, 0/*FIXME*/)
	},
#endif

	/* DEMO_CLK */
	{
		.name = MT_CLK_DEMO,	/* Demo Bus */
		.flags = FLAG_CLK_MUX|FLAG_CLK_GATE_ACTIVE_LOW,
		.gate = MT_INIT_GATE(REG_TOPCLK_CTRL6, DEMO_CLKEN_SHIFT,
							0, 0, 0),	/* FIXME */
		/*
		 * 0: clkgen_ethintp_reg : "drv0_clk" & "drv1_clk"  => DRV0_CLK_ANA & DRV1_CLK_ANA
		 * 1: demo_clk_sel[2:0]  : "drv0_clk" & "drv1_clk"  => DRV0_CLK_TOP & DRV1_CLK_TOP
		 */
		.mux = MT_INIT_MUX(REG_TOPCLK_CTRL7, DEMO_CLKSEL_SHIFT, 1, g_top_demo_mux,
							0, 0, 0)	/* FIXME */
	},
	/* DRV0_CLK_TOP */
	{
		.name = DRV0_CLK_TOP,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_TOPCLK_CTRL7, DEMO_DRV0_CLKSEL_SHIFT, 2, g_top_drv0_rate,
							0, 0, 0)	/* FIXME */
	},
	/* DRV1_CLK_TOP */
	{
		.name = DRV1_CLK_TOP,
		.flags = FLAG_CLK_RATE,
		.gate = {0},
		.rate = MT_INIT_RATE(REG_TOPCLK_CTRL7, DEMO_DRV1_CLKSEL_SHIFT, 1, g_top_drv1_rate,
							0, 0, 0)	/* FIXME */
	},

	/* DEMO_C_CLK */
	{
		.name = MT_CLK_DEMO_C,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_DEMO_CFG, DEMO_C_EN_SHIFT,
							0, 0, 0),	/* FIXME */
	},
	/* DEMO_S_CLK */
	{
		.name = MT_CLK_DEMO_S,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_DEMO_CFG, DEMO_S_EN_SHIFT,
							0, 0, 0),	/* FIXME */
	},
	/* DEMO_J83B_CLK */
	{
		.name = MT_CLK_DEMO_J83B,
		.flags = FLAG_CLK_NONE,
		.gate = MT_INIT_GATE(REG_DEMO_CFG, DEMO_J83B_EN_SHIFT,
							0, 0, 0),	/* FIXME */
	},

	/* A1+ */
	/* AOUT_MCLK */
	{
		.name = AOUT_MCLK,
		.flags = FLAG_CLK_MUX,
		.gate = {0},
		/*
		 * 00 01: aout_mclk
		 * 10: DAI TX CLK -> Dai_tx_audiopll_clk
		 * 11: DAI TX CLK -> Dai_tx_digdiv_clk
		 */
		.mux = MT_INIT_MUX(REG_TOPCLK_CTRL7, AOUT_MCLK_CLKSEL_SHIFT, 2, g_top_aout_mclk_mux,
							0, 0, 0)	/* FIXME */
	},
};

#else

/* CONFIG_TARGET_SYMPHONY6_LITE || CONFIG_TARGET_SYMPHONY6_MINI */

struct mt_clk_in mt_top_clk_table[] =
{
	/* TODO: */
};

#endif

/* top clock table count */
u32 mt_top_clk_table_size = sizeof(mt_top_clk_table)/sizeof(mt_top_clk_table[0]);

//---------------------------------------------------------------------------//

void mt_top_clk_enable_all(void)
{
	u32 val;

	val = MT_IO_READ32(REG_TOPCLK_CTRL0);
	val |= TOP_APBACKUP_CLKEN_MASK;
	val |= TOP_APB_CLKEN_MASK;
	val |= TOP_AHB_CLKEN_MASK;
	val |= TOP_HDMI_AUD_CLKEN_MASK;
	val |= TOP_DDROMC_CLKEN_MASK;
	val |= TOP_AXI_CLKEN_MASK;
	MT_IO_WRITE32(REG_TOPCLK_CTRL0, val);

	val = MT_IO_READ32(REG_TOPCLK_CTRL1);
	val |= TOP_JPG_CLKEN_MASK;
	val |= TOP_GRA_CLKEN_MASK;
	val |= TOP_SDIO_CLKEN_MASK;
	val |= TOP_PKA_CLKEN_MASK;
	val |= TOP_DS_CLKEN_MASK;
	val |= TOP_SPI_CLKEN_MASK;
	val |= TOP_SMC_CLKEN_MASK;
	val |= TOP_AVCPU_CLKEN_MASK;
	MT_IO_WRITE32(REG_TOPCLK_CTRL1, val);

	val = MT_IO_READ32(REG_TOPCLK_CTRL2);
	val |= TOP_HDMI_TMDS_CLKEN_MASK;
	val |= TOP_TSI_CLKEN_MASK;
	val |= TOP_VENC_CLKEN_MASK;
	val |= TOP_PNG_CLKEN_MASK;
	val |= TOP_AVSYNC_CLKEN_MASK;
	//val |= TOP_USB0_CORE_SCAN_CLKEN_MASK;
	//val |= TOP_AUDIOPLL_SCAN_CLKEN_MASK;
	val |= TOP_AOUT_CLKEN_MASK;
	val |= TOP_VDEC_CLKEN_MASK;
	val |= TOP_HDMI_CLKEN_MASK;
	MT_IO_WRITE32(REG_TOPCLK_CTRL2, val);

	val = MT_IO_READ32(REG_TOPCLK_CTRL3);
	val |= TOP_GLITCH_CLKEN_MASK;
	val |= TOP_IFCPTIMER_CLKEN_MASK;
	val |= TOP_SWTIMER_CLKEN_MASK;
	val |= TOP_OTP_CLKEN_MASK;
	val |= TOP_IFCP_CRYPTO_CLKEN_MASK;
	val |= TOP_IFCP_SYS_CLKEN_MASK;
	MT_IO_WRITE32(REG_TOPCLK_CTRL3, val);

	val = MT_IO_READ32(REG_TOPCLK_CTRL4);
	val |= TOP_AUDIO_SPDF_CLKEN_MASK;
	val |= TOP_DAI_TX_CLKEN_MASK;
	val |= TOP_DAI_RX_CLKEN_MASK;
	val |= TOP_USB3_CLKEN_MASK;
	val |= TOP_UART_CLKEN_MASK;
	val |= TOP_GPU_CLKEN_MASK;
	val |= TOP_DMA_CLKEN_MASK;
	val |= TOP_M2MCIPHER_CLKEN_MASK;
	val |= TOP_SECURE_CLKEN_MASK;
	val |= TOP_SOSC_CLKEN_MASK;
	MT_IO_WRITE32(REG_TOPCLK_CTRL4, val);

	val = MT_IO_READ32(REG_TOPCLK_CTRL5);
	val |= TOP_GMAC_CLKEN_MASK;
	//val |= TOP_PNAND_ACSCAN_CLKEN_MASK;
	val |= TOP_PNAND_CLKEN_MASK;
	val |= TOP_ETHERNET_CLKEN_MASK;
	val |= TOP_DISP_CLKEN_MASK;
	MT_IO_WRITE32(REG_TOPCLK_CTRL5, val);

	//val = MT_IO_READ32(REG_TOPCLK_CTRL6);
	//FIXME:
	//0:enable
	//1:disable
	//val &= ~(0x1 << DEMO_CLKEN_SHIFT);
	//FIXME: enable "testclk"?
	//val |= (0x1 << TESTCLK_CLOSED_SHIFT);
	//MT_IO_WRITE32(REG_TOPCLK_CTRL6, val);
}

void mt_top_clk_hw_auto_gate_enable(void)
{
	u32 val;

	/* enable SW auto gate */
	val = MT_IO_READ32(REG_SYS_CLK_SW_EN);
	val |= (0x1 << SPDMA_CLK_SW_EN_SHIFT);
	val |= (0x1 << AHB_CLK_SW_EN_SHIFT);
	//TODO: add your module bits here

	MT_IO_WRITE32(REG_SYS_CLK_SW_EN, val);

	/* enable HW auto gate */
	val = MT_IO_READ32(REG_SYS_CLK_HW_EN);
	val |= (0x1 << SPDMA_CLK_HW_EN_SHIFT);
	val |= (0x1 << AHB_CLK_HW_EN_SHIFT);
	//TODO: add your module bits here

	MT_IO_WRITE32(REG_SYS_CLK_HW_EN, val);
}

