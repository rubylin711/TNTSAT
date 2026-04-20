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
 *
 *****************************************************************************/

/* HDMI Patch for MICO */
/*#define PATCH_HDMI_MICO*/

/* default vhd ssc */
#define DEF_VHDSSC					0x31CFB872

/* no ssc, include pi*/
static struct hdmi_analog_params sym6_hdmi_params_nossc_inc_pi[] =
{
/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  148500000,    1485000000,  {0,0,1},     1,  .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B059}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  148500000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B042}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0,0} },

/*	  .vhd                                                 .ssc_valid .vsd_ssc	  .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll	                                       .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},   74250000,     742500000,  {0,0,1},     1,  .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B059}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},   74250000,     742500000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B042}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0,0} },

/*	  .vhd                                                 .ssc_valid .vsd_ssc	  .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll	                                       .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vsd_pll_pi={0x5C4CB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vsd_pll_pi={0x5C3AB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vsd_pll_pi={0x1C0480FA}, {0,0} },

/*	  .vhd                                                 .ssc_valid .vsd_ssc	  .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

/* 4K */
#if 1
	/* FIXME: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 1, 1},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {1,0,1},     1,  .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B059}, {0,0} },
#else
	{0},
#endif

	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B042}, {0,0} },

	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M  */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0,0} },


	/* HDMI_ANALOG_CFG_594 - 594M */
#ifdef PATCH_HDMI_MICO
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,	{1,0,1},     1,  .vhd_pll={0x1C0180F8}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180F8}, {1,3} },
#else
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {1,0,1},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180FA}, {1,3} },
#endif

/*	  .vhd                                                 .ssc_valid .vsd_ssc	  .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{ 1,   .vhd_clk={0, 0, 1, 1},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  297000000,    1485000000,  {1,0,1},     1,  .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vhd_pll_pi={0xCC43B059}, {0,0} },

	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  297000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vhd_pll_pi={0x0C85B042}, {0,0} },

	/* HDMI_ANALOG_CFG_297 - 297M	*/
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  297000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0,0} },

/*	  .vhd                                                 .ssc_valid .vsd_ssc	  .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  108000000,     270000000,  {1,1,0},     1,  .vhd_pll={0xCC43B059}, .vsd_pll={0x5C4CB05A}, 0, .vsd_pll_pi={0x5C4CB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  108000000,     270000000,  {1,1,0},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB05A}, 0, .vsd_pll_pi={0x5C3AB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  108000000,     270000000,  {0,1,0},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vsd_pll_pi={0x1C0480FA}, {0,0} },


    /* HDMI_ANALOG_CFG_108 - 108M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  108000000,     270000000,  {1,1,0},     1,  .vhd_pll={0x1C0180FA}, .vsd_pll={0x1C0480FA}, 0, .vsd_pll_pi={0x1C0480FA}, {0,0} },

};

/* no ssc, pd pi*/
static struct hdmi_analog_params sym6_hdmi_params_nossc_pd_pi[] =
{
/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                       .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  148500000,    1485000000,  {0,0,1},     1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  148500000,    1485000000,  {0,0,0},     0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  148500000,    1485000000,  {0,0,0},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                       .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},   74250000,     742500000,  {0,0,1},     1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},   74250000,     742500000,  {0,0,0},     0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},   74250000,     742500000,  {0,0,0},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                       .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},   27000000,     270000000,  {0,0,0},     1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},   27000000,     270000000,  {0,0,0},     1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},   27000000,     270000000,  {0,0,0},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                       .pd_pi              .dcc */

/* 4K */
#if 1
	/* FIXME: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 1, 1},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {1,0,1},     1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, {0,0} },
#else
	{0},
#endif
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {0,0,1},     0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {0,0,1},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },


	/* HDMI_ANALOG_CFG_594 - 594M */
#ifdef PATCH_HDMI_MICO
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {1,0,1},     1, .vhd_pll={0x1C22A048}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {1,3} },
#else
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  594000000,    1485000000,  {1,0,1},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {1,3} },
#endif

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                       .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{ 1,   .vhd_clk={0, 0, 1, 1},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  297000000,    1485000000,  {1,0,1},     1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, {0,0} },
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  297000000,    1485000000,  {0,0,1},     0, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  297000000,    1485000000,  {0,0,1},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                       .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 1},  {DEF_VHDSSC},  108000000,     270000000,  {1,1,0},     1, .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 1},  {DEF_VHDSSC},  108000000,     270000000,  {1,1,0},     1, .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},  108000000,     270000000,  {0,1,0},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, {0,0} },


    /* HDMI_ANALOG_CFG_108 - 108M */
    { 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 1},  {DEF_VHDSSC},	 108000000,     270000000,  {1,1,0},     1, .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, {0,0} },

};

#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)

/* ssc, include pi */
static struct hdmi_analog_params sym6_hdmi_params_ssc_inc_pi[] =
{
/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x0AF6EE76},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 0, .vhd_pll_pi={0xCC43B059}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {1, 1, 0},  {0x0AF6DA76},  148500000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B052}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x10F0DA76},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x10F0C276},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 0, .vhd_pll_pi={0xCC43B059}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {1, 1, 0},  {0x10F0B676},   74250000,     742500000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B052}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x10F0DC76},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x10F0C876},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 0, .vsd_pll_pi={0x5C4CB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x10F0BC76},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vsd_pll_pi={0x5C3AB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {5, 1, 0},  {0x10F0E476},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vsd_pll_pi={0x1C28A05A}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

/* 4K */
	/* TODO: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M, Not support SSC */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {2, 1, 0},  {0x0CF4D07D},  594000000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B052}, {0,0} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {2, 1, 0},  {0x0CF4BE7D},  594000000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0,0} },


	/* TODO: HDMI_ANALOG_CFG_594 - 594M, Not support SSC */
	{0},

/*    .vhd                                                 .ssc_valid .vsd_ssc	  .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* TODO: HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5, Not support SSC */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {2, 1, 0},  {0x0CF4D07D},	 297000000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vhd_pll_pi={0x0C85B052}, {0,0} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {2, 1, 0},  {0x0CF4BE7D},	 297000000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi                       .dcc */

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x10F0C876},  108000000,     270000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 0, .vsd_pll_pi={0x5C4CB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x10F0BC76},  108000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 0, .vsd_pll_pi={0x5C3AB05A}, {0,0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {6, 1, 0},  {0x10F0E476},  108000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 0, .vsd_pll_pi={0x1C28A05A}, {0,0} },


	/* TODO: HDMI_ANALOG_CFG_108 - 108M */
	{0},

};

/* ssc, pd pi*/
static struct hdmi_analog_params sym6_hdmi_params_ssc_pd_pi[] =
{
/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x0AF6EE76},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {1, 1, 0},  {0x0AF6DA76},  148500000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x10F0DA76},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x10F0C276},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {1, 1, 0},  {0x10F0B676},   74250000,     742500000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x10F0DC76},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc        .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                       .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x10F0C876},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x10F0BC76},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {5, 1, 0},  {0x10F0E476},   27000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi              .dcc */

/* 4K */
	/* TODO: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {2, 1, 0},  {0x0CF4D07D},  594000000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },
	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {2, 1, 0},  {0x0CF4BE7D},  594000000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },


	/* TODO: HDMI_ANALOG_CFG_594 - 594M, Not support SSC */
	{0},

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi              .dcc */

	/* TODO: HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{0},
	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {2, 1, 0},  {0x0CF4D07D},  297000000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vhd_pll_pi={0}, {0,0} },
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ 1,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {2, 1, 0},  {0x0CF4BE7D},  297000000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vhd_pll_pi={0}, {0,0} },

/*    .vhd                                                 .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                                        .pd_pi              .dcc */

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0,   .vhd_clk={1, 1, 1, 3},  .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x10F0C876},  108000000,     270000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0,   .vhd_clk={0, 0, 1, 2},  .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x10F0BC76},  108000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x0C85B042}, .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, {0,0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
	{ 0,   .vhd_clk={0, 0, 0, 0},  .vsd_clk={0, 0, 0, 0},  1,         {6, 1, 0},  {0x10F0E476},  108000000,     270000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, {0,0} },


	/* TODO: HDMI_ANALOG_CFG_108 - 108M */
	{0},

};
#endif

