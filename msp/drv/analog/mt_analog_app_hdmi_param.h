/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 *
 * History:
 *   20240125:
 *     Symphony6_HDMI_IF_register_setting_V0 1_20240124.xls
 *   20240130:
 *     Symphony6_HDMI_IF_register_setting_V0.3_20240130.xls
 *   20250305:
 *     Symphony6_HDMI_IF_register_setting_V0.3a_20240513.xls
 *   20250321:
 *     Symphony6A1_HDMI_IF_register_setting_V1.1_20250319.xls
 *
 *****************************************************************************/
#ifndef __INC_MT_ANALOG_APP_HDMI_PARAM_H__
#define __INC_MT_ANALOG_APP_HDMI_PARAM_H__

/* HDMI Patch for MICO */
/*#define PATCH_HDMI_MICO*/

/* default vhd ssc */
#define DEF_VHDSSC					0x31CFB872

/* hdmi_tx clk sel 270M from VHDPLL */
#define VHD_270M					(270000000U + 'H')

/* TMDS 27M clk from VHDPLL, PD VSDPLL for power saving */
#define CFG_TMDS_27M_FROM_VHDPLL

/*****************************************************************************/

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
	 *   00: 270M from vhd
	 *   01: 742.5M
	 *   10: 1485M
	 *   11: 270M from vsd
	 */
	unsigned long hdmitx_rate;

	/*
	 * hdmi clk select
	 *
	 * 0xBF5D0120 clkgen_usbaclk_reg0 {[8], [9], [10]}
	 */
	struct hdmiclk_param hdmiclk_sel;

/* A1 Added */
	/*
	 * video clk source select
	 *
	 * 0xBF5D005C clkgen_vhdintp_reg {[0], [7]}
	 */
	struct vclk_source_param vclk_src;

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

/* A1 Added: pd vsdpll for power saving */
	int pd_vsdpll;

	/*
	 * hdmi PHY clk DCC
	 *
	 * 0xBF5D0120 clkgen_usbaclk_reg0 {[1], [3:2]}
	 */
	struct hdmidcc_param hdmi_dcc;
};

/*****************************************************************************/

/* no ssc, include pi*/
static struct hdmi_analog_params sym6_hdmi_params_nossc_inc_pi[] =
{
/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 148500000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B059}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 148500000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B042}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 148500000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180FA}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC},  74250000,     742500000,  {0,0,1},     {0,0},    1, .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B059}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC},  74250000,     742500000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B042}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC},  74250000,     742500000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180FA}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vsd_pll_pi={0x5C4CB05A}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vsd_pll_pi={0x5C3AB05A}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,      VHD_270M,  {0,0,0},     {1,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00800A}, 0, .vhd_pll_pi={0x1C22A05A}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vsd_pll_pi={0x1C0480FA}, 0,         {0,0} },
#endif

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

/* 4K */
#if 1
	/* FIXME: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{ 1, .vhd_clk={0, 0, 1, 1}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {1,0,1},     {0,0},    1, .vhd_pll={0xCC43B058}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B058}, 0,         {1,3} },
#else
	{0},
#endif

	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B040}, 0,         {1,3} },

	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M  */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0x1C0180F8}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180F8}, 0,         {1,3} },

	/* HDMI_ANALOG_CFG_594 - 594M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {1,0,1},     {0,0},    1, .vhd_pll={0x1C0180F8}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180F8}, 0,         {1,3} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{ 1, .vhd_clk={0, 0, 1, 1}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 297000000,    1485000000,  {1,0,1},     {0,0},    1, .vhd_pll={0xCC43B058}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B058}, 0,         {1,3} },

	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 297000000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B040}, 0,         {1,3} },

	/* HDMI_ANALOG_CFG_297 - 297M	*/
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 297000000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0x1C0180F8}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180F8}, 0,         {1,3} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {0,1,0},     {0,0},    1, .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vsd_pll_pi={0x5C4CB05A}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {0,1,0},     {0,0},    1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vsd_pll_pi={0x5C3AB05A}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,      VHD_270M,  {0,0,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00800A}, 0, .vhd_pll_pi={0x1C22A05A}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {0,1,0},     {0,0},    1, .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vsd_pll_pi={0x1C0480FA}, 0,         {0,0} },
#endif

    /* HDMI_ANALOG_CFG_108 - 108M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,      VHD_270M,  {1,0,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00800A}, 0, .vhd_pll_pi={0x1C22A05A}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {1,1,0},     {0,0},    1, .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vsd_pll_pi={0x1C0480FA}, 0,         {0,0} },
#endif

};

/* no ssc, pd pi*/
static struct hdmi_analog_params sym6_hdmi_params_nossc_pd_pi[] =
{
/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 148500000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 148500000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 148500000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC},  74250000,     742500000,  {0,0,1},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC},  74250000,     742500000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC},  74250000,     742500000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,      VHD_270M,  {0,0,0},     {1,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 1, .vhd_pll_pi={0}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },
#endif

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

/* 4K */
#if 1
	/* FIXME: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{ 1, .vhd_clk={0, 0, 1, 1}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {1,0,1},     {0,0},    1, .vhd_pll={0xCC43B048}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },
#else
	{0},
#endif
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {0,0,1},     {0,0},    0, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },

	/* HDMI_ANALOG_CFG_594 - 594M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 594000000,    1485000000,  {1,0,1},     {0,0},    1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{ 1, .vhd_clk={0, 0, 1, 1}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 297000000,    1485000000,  {1,0,1},     {0,0},    1, .vhd_pll={0xCC43B048}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 297000000,    1485000000,  {0,0,1},     {0,0},    0, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 297000000,    1485000000,  {0,0,1},     {0,0},    1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {0,1,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {0,1,0},     {0,0},    1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,      VHD_270M,  {0,1,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 1, .vhd_pll_pi={0}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {0,1,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },
#endif

    /* HDMI_ANALOG_CFG_108 - 108M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
    { 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,      VHD_270M,  {1,1,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 1, .vhd_pll_pi={0}, 1,         {0,0} },
#else
    { 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1}, {DEF_VHDSSC}, 108000000,     270000000,  {1,1,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },
#endif

};

//#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)

/* ssc, include pi */
static struct hdmi_analog_params sym6_hdmi_params_ssc_inc_pi[] =
{
/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 1,         {2, 1, 0}, {0x0AF6EE76}, 148500000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 0, .vhd_pll_pi={0xCC43B059}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {1, 1, 0}, {0x0AF6DA76}, 148500000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B052}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {1, 1, 0}, {0x10F0DA76}, 148500000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A05A}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 1,         {2, 1, 0}, {0x10F0C276},  74250000,     742500000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 0, .vhd_pll_pi={0xCC43B059}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {1, 1, 0}, {0x10F0B676},  74250000,     742500000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B052}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {1, 1, 0}, {0x10F0DC76},  74250000,     742500000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A05A}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 1,         {6, 1, 0}, {0x10F0C876},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 0, .vsd_pll_pi={0x5C4CB05A}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {6, 1, 0}, {0x10F0BC76},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vsd_pll_pi={0x5C3AB05A}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {4, 1, 0}, {0x10F0E476},  27000000,      VHD_270M,  {0,0,0},     {1,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 0, .vhd_pll_pi={0x1C22A05A}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {5, 1, 0}, {0x10F0E476},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vsd_pll_pi={0x1C28A05A}, 0,         {0,0} },
#endif

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

/* 4K */
	/* TODO: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M, Not support SSC */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {2, 1, 0}, {0x0CF4D07D}, 594000000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B050}, 0,         {1,3} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {2, 1, 0}, {0x0CF4BE7D}, 594000000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A058}, 0,         {1,3} },

	/* TODO: HDMI_ANALOG_CFG_594 - 594M, Not support SSC */
	{0},

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* TODO: HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5, Not support SSC */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {2, 1, 0}, {0x0CF4D07D}, 297000000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B050}, 0,         {1,3} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {2, 1, 0}, {0x0CF4BE7D}, 297000000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A058}, 0,         {1,3} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi                       .pd_vsdpll .dcc */

	/* TODO: HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{0},

	/* TODO: HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{0},

	/* HDMI_ANALOG_CFG_54 - 54M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {4, 1, 0}, {0x10F0E476}, 108000000,      VHD_270M,  {0,0,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 0, .vhd_pll_pi={0x1C22A05A}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {6, 1, 0}, {0x10F0E476}, 108000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vsd_pll_pi={0x1C28A05A}, 0,         {0,0} },
#endif

	/* HDMI_ANALOG_CFG_108 - 108M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {4, 1, 0}, {0x10F0E476}, 108000000,      VHD_270M,  {1,0,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 0, .vhd_pll_pi={0x1C22A05A}, 1,         {0,0} },
#else
	{0},
#endif

};

/* ssc, pd pi*/
static struct hdmi_analog_params sym6_hdmi_params_ssc_pd_pi[] =
{
/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 1,         {2, 1, 0}, {0x0AF6EE76}, 148500000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {1, 1, 0}, {0x0AF6DA76}, 148500000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {1, 1, 0}, {0x10F0DA76}, 148500000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 1,         {2, 1, 0}, {0x10F0C276},  74250000,     742500000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {1, 1, 0}, {0x10F0B676},  74250000,     742500000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {1, 1, 0}, {0x10F0DC76},  74250000,     742500000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {0,0} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0, .vhd_clk={1, 1, 1, 3}, .vsd_clk={1, 1, 1, 3}, 1,         {6, 1, 0}, {0x10F0C876},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {6, 1, 0}, {0x10F0BC76},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {4, 1, 0}, {0x10F0E476},  27000000,      VHD_270M,  {0,0,0},     {1,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 1, .vhd_pll_pi={0}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {5, 1, 0}, {0x10F0E476},  27000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },
#endif

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

/* 4K */
	/* TODO: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {2, 1, 0}, {0x0CF4D07D}, 594000000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {2, 1, 0}, {0x0CF4BE7D}, 594000000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },

	/* TODO: HDMI_ANALOG_CFG_594 - 594M, Not support SSC */
	{0},

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* TODO: HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1, .vhd_clk={0, 0, 1, 2}, .vsd_clk={1, 1, 1, 2}, 1,         {2, 1, 0}, {0x0CF4D07D}, 297000000,    1485000000,  {0,0,0},     {0,0},    0, .vhd_pll={0x0C85B040}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {2, 1, 0}, {0x0CF4BE7D}, 297000000,    1485000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, 0,         {1,3} },

/*    .vhd                                             .ssc_valid .vsd_ssc   .vhd_ssc      .venc_os_rate .hdmitx_rate .hdmiclk_sel .vclk_src .reset_pll                                       .pd_pi              .pd_vsdpll .dcc */

	/* TODO: HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{0},

	/* TODO: HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{0},

	/* HDMI_ANALOG_CFG_54 - 54M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {4, 1, 0}, {0x10F0E476}, 108000000,      VHD_270M,  {0,0,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 1, .vhd_pll_pi={0}, 1,         {0,0} },
#else
	{ 0, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {6, 1, 0}, {0x10F0E476}, 108000000,     270000000,  {0,0,0},     {0,0},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, 0,         {0,0} },
#endif

	/* HDMI_ANALOG_CFG_108 - 108M */
#ifdef CFG_TMDS_27M_FROM_VHDPLL
/* use vhdpll for power saving */
	{ 1, .vhd_clk={0, 0, 0, 0}, .vsd_clk={0, 0, 0, 0}, 1,         {4, 1, 0}, {0x10F0E476}, 108000000,      VHD_270M,  {1,0,0},     {1,1},    1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C00A00A}, 1, .vhd_pll_pi={0}, 1,         {0,0} },
#else
	{0},
#endif

};
//#endif

/*****************************************************************************/

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

/* hdmi_tx_reg2 <31:28> */
#define MUTE_MASK			(0xF << 28)

static struct hdmi_analog_post_params sym6_hdmi_post_params[] =
{
/*	 .test         .ch0          .ch1          .ch2 */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_27 - 27M */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{ {0x080000F0}, {0x0130442C}, {0x0130442C}, {0x0130442C | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ {0x080000F0}, {0x0130442C}, {0x0130442C}, {0x0130442C | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_594 - 594M */
#ifdef PATCH_HDMI_MICO
	{ {0x080000F8}, {0x0130442F}, {0x0130442F}, {0x0130442F | MUTE_MASK} },
#else
	{ {0x080000F8}, {0x0130442C}, {0x0130442C}, {0x0130442C | MUTE_MASK} },
#endif

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{ {0x080000F0}, {0x0130442C}, {0x0130442C}, {0x0130442C | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ {0x080000F0}, {0x0130442C}, {0x0130442C}, {0x0130442C | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ {0x1500019A}, {0x031044CA}, {0x031044CA}, {0x031044CA | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },
	/* HDMI_ANALOG_CFG_54 - 54M */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },

	/* HDMI_ANALOG_CFG_108 - 108M */
	{ {0x08000108}, {0x0100BB08}, {0x0100BB08}, {0x0100BB08 | MUTE_MASK} },
};

/*****************************************************************************/

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
	{5/*4分频*/,	0,				4/*2分频*/},
	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{5/*4分频*/,	0,				4/*2分频*/},
	/* HDMI_ANALOG_CFG_54 - 54M */
	{5/*4分频*/,	0,				4/*2分频*/},

	/* HDMI_ANALOG_CFG_108 - 108M */
	{5/*4分频*/,	0,				0},
};

#endif

