
/******************************************************************************
 *
 * History:
 *   202309:
 *     symphony6 HDMI 8bit_10bit_12bit clock config(20230912).xlsx
 *
 *****************************************************************************/

/* no ssc, include pi*/
static struct hdmi_analog_params sym6_hdmi_params_nossc_inc_pi[] =
{
/*    .vhd .phy                            .ssc_valid .vsd_ssc    .vhd_ssc .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                 .pd_pi                       .post_phy */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 3},  0,         {0, 0, 0},  {0},     148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0xCC43B0F9}, 0, .vhd_pll_pi={0xCC43B0F9}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 1, 2},  0,         {0, 0, 0},  {0},     148500000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B0F2}, 0, .vhd_pll_pi={0x0C85B0F2}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  0,         {0, 0, 0},  {0},     148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C0180FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 3},  0,         {0, 0, 0},  {0},      74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0xCC43B0F9}, 0, .vhd_pll_pi={0xCC43B0F9}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 1, 2},  0,         {0, 0, 0},  {0},      74250000,     742500000,  {0,0,0},     0,  .vhd_pll={0x0C85B0F2}, 0, .vhd_pll_pi={0x0C85B0F2}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  0,         {0, 0, 0},  {0},      74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0x1C0180FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 0},  {0},      27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x5C1690FA}, 0, .vsd_pll_pi={0x5C1690FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 0},  {0},      27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x5C3AB0FA}, 0, .vsd_pll_pi={0x5C3AB0FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   {1, 0}, .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 0},  {0},      27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x1C0480FA}, 0, .vsd_pll_pi={0x1C0480FA}, {0, 0} },

/* 4K */
	/* HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{ 1,   {1, 1}, .vhd_clk={0, 0, 1, 1},  0,         {0, 0, 0},  {0},     594000000,    1485000000,  {1,0,1},     1,  .vhd_pll={0xCC43B0F9}, 0, .vhd_pll_pi={0xCC43B0F9}, {0, 1} },

	/* HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{ 1,   {1, 1}, .vhd_clk={0, 0, 1, 2},  0,         {0, 0, 0},  {0},     594000000,    1485000000,  {0,0,1},     0,  .vhd_pll={0x0C85B0F2}, 0, .vhd_pll_pi={0x0C85B0F2}, {0, 1} },

	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M  */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  0,         {0, 0, 0},  {0},     594000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x1C0180FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_594 - 594M */
	{ 1,   {1, 1}, .vhd_clk={0, 0, 0, 0},  0,         {0, 0, 0},  {0},     594000000,    1485000000,  {1,0,1},     1,  .vhd_pll={0x1C0180FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0, 1} },

//+++++//

	/* HDMI_ANALOG_CFG_297_X_1P5 - 297M x 1.5 */
	{ 1,   {1, 1}, .vhd_clk={1, 1, 1, 3},  0,         {0, 0, 0},  {0},     297000000,    1485000000,  {1,0,1},     1,  .vhd_pll={0xCC43B0F9}, 0, .vhd_pll_pi={0xCC43B0F9}, {0, 1} },

	/* HDMI_ANALOG_CFG_297_X_1P25 - 297M x 1.25 */
	{ 1,   {1, 1}, .vhd_clk={0, 0, 1, 2},  0,         {0, 0, 0},  {0},     297000000,    1485000000,  {0,0,1},     0,  .vhd_pll={0x0C85B0F2}, 0, .vhd_pll_pi={0x0C85B0F2}, {0, 1} },

	/* HDMI_ANALOG_CFG_297 - 297M	*/
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  0,         {0, 0, 0},  {0},     297000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x1C0180FA}, 0, .vhd_pll_pi={0x1C0180FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 3},  0,         {0, 0, 0},  {0},     108000000,     270000000,  {1,0,0},     1,  .vsd_pll={0x5C1690FA}, 0, .vsd_pll_pi={0x5C1690FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 2},  0,         {0, 0, 0},  {0},     108000000,     270000000,  {1,0,0},     1,  .vsd_pll={0x5C3AB0FA}, 0, .vsd_pll_pi={0x5C3AB0FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
	{ 0,   {1, 0}, .vsd_clk={0, 0, 0, 0},  0,         {0, 0, 0},  {0},     108000000,     270000000,  {1,0,0},     1,  .vsd_pll={0x1C0D82FA}, 0, .vsd_pll_pi={0x1C0D82FA}, {0, 0} },

	/* HDMI_ANALOG_CFG_108 - 108M */
	{0},

};

#if 0
/* no ssc, pd pi*/
static struct hdmi_analog_params sym6_hdmi_params_nossc_pd_pi[] =
{
/*    .vhd .phy                           .ssc_valid .vsd_ssc    .vhd_ssc .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                   .pd_pi              .post_phy */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 3},    0,         {0, 0, 1},  {0},     148500000,    1485000000,  {0,0,0}, .vhd_pll={0xCCA69089}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 2},    0,         {0, 0, 1},  {0},     148500000,    1485000000,  {0,0,0}, .vhd_pll={0xCC85B08A}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0},    0,         {0, 0, 1},  {0},     148500000,    1485000000,  {0,0,0}, .vhd_pll={0x1C0180CA}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 3},    0,         {0, 0, 1},  {0},      74250000,     742500000,  {0,0,0}, .vhd_pll={0xCCA69089}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 2},    0,         {0, 0, 1},  {0},      74250000,     742500000,  {0,0,0}, .vhd_pll={0xCC85B08A}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0},    0,         {0, 0, 1},  {0},      74250000,     742500000,  {0,0,0}, .vhd_pll={0x1C0180CA}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 3}, 0,         {0, 0, 1},  {0},      27000000,     270000000,  {0,0,0}, .vsd_pll={0x5CB8908A}, 1, .vsd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 2}, 0,         {0, 0, 1},  {0},      27000000,     270000000,  {0,0,0}, .vsd_pll={0x5C94B08A}, 1, .vsd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   {1, 0}, .vsd_clk={0, 0, 0, 0}, 0,         {0, 0, 1},  {0},      27000000,     270000000,  {0,0,0}, .vsd_pll={0x1C0D82CA}, 1, .vsd_pll_pi={0}, {0, 0} },

/* 4K */
	/* TODO: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{0},

	/* TODO: HDMI_ANALOG_CFG_594 - 594M */
	{0},

	/* TODO: HDMI_ANALOG_CFG_297_X_1P5 - 297M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_X_1P25 - 297M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297 - 297M */
	{0},

	/* TODO: HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{0},

	/* TODO: HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{0},

	/* TODO: HDMI_ANALOG_CFG_54 - 54M */
	{0},

	/* HDMI_ANALOG_CFG_108 - 108M */
	{0},

};
#endif

#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)

/* ssc, include pi */
static struct hdmi_analog_params sym6_hdmi_params_ssc_inc_pi[] =
{
/*    .vhd .phy                            .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                 .pd_pi                       .post_phy */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x11EFBE76},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, 0, .vhd_pll_pi={0xCC43B059}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 1, 2},  1,         {1, 1, 0},  {0x11EFB276},  148500000,    1485000000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, 0, .vhd_pll_pi={0x0C85B052}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x15EBEE73},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x22DEBE76},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, 0, .vhd_pll_pi={0xCC43B059}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 1, 2},  1,         {1, 1, 0},  {0x22DEB276},   74250000,     742500000,  {0,0,0},     0,  .vhd_pll={0x0C85B042}, 0, .vhd_pll_pi={0x0C85B052}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x14ECF673},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x1FE1D271},   27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x5C4CB04A}, 0, .vsd_pll_pi={0x5C4CB05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x1FE1D571},   27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x5C3AB04A}, 0, .vsd_pll_pi={0x5C3AB05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   {1, 0}, .vsd_clk={0, 0, 0, 0},  1,         {5, 1, 0},  {0x32CEBC72},   27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x1C28A04A}, 0, .vsd_pll_pi={0x1C28A05A}, {0, 0} },

/* 4K */
	/* TODO: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{0},

	/* HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x15EBEE73},	 594000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x1C22A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_594 - 594M */
	{ 1,   {1, 1}, .vhd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x15EBEE73},	 594000000,    1485000000,  {1,0,1},     1,  .vhd_pll={0x1C22A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0, 1} },

//+++++//

	/* TODO: HDMI_ANALOG_CFG_297_X_1P5 - 297M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_X_1P25 - 297M */
	{0},
	/* HDMI_ANALOG_CFG_297 - 297M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x15EBEE73},	 297000000,    1485000000,  {0,0,1},     1,  .vhd_pll={0x1C22A04A}, 0, .vhd_pll_pi={0x1C22A05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x1FE1D271},  108000000,     270000000,  {1,0,0},     1,  .vsd_pll={0x5C4CB04A}, 0, .vsd_pll_pi={0x5C4CB05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x1FE1D571},  108000000,     270000000,  {1,0,0},     1,  .vsd_pll={0x5C3AB04A}, 0, .vsd_pll_pi={0x5C3AB05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
	{ 0,   {1, 0}, .vsd_clk={0, 0, 0, 0},  1,         {5, 1, 0},  {0x32CEBC72},  108000000,     270000000,  {1,0,0},     1,  .vsd_pll={0x1C28A04A}, 0, .vsd_pll_pi={0x1C28A05A}, {0, 0} },

	/* HDMI_ANALOG_CFG_108 - 108M */
	{0},

};

/* ssc, pd pi*/
static struct hdmi_analog_params sym6_hdmi_params_ssc_pd_pi[] =
{
/*    .vhd .phy                            .ssc_valid .vsd_ssc    .vhd_ssc       .venc_os_rate .hdmitx_rate .hdmiclk_sel .reset_pll                 .pd_pi              .post_phy */

	/* HDMI_ANALOG_CFG_148P5_X_1P5 - 148.5M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x11EFBE76},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5_X_1P25 - 148.5M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 2},  1,         {2, 1, 0},  {0x11EFB276},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0xCC85B08A}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_148P5 - 148.5M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x15EBEE73},  148500000,    1485000000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P5 - 74.25M x 1.5 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 3},  1,         {2, 1, 0},  {0x22DEBE76},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0xCC43B049}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25_X_1P25 - 74.25M x 1.25 */
	{ 1,   {1, 0}, .vhd_clk={1, 1, 1, 2},  1,         {2, 1, 0},  {0x22DEB276},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0xCC85B08A}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_74P25 - 74.25M */
	{ 1,   {1, 0}, .vhd_clk={0, 0, 0, 0},  1,         {1, 1, 0},  {0x14ECF673},   74250000,     742500000,  {0,0,0},     1,  .vhd_pll={0x1C22A04A}, 1, .vhd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P5 - 27M x 1.5 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x1FE1D271},   27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_27_X_1P25 - 27M x 1.25 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x1FE1D571},   27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_27 - 27M */
	{ 0,   {1, 0}, .vsd_clk={0, 0, 0, 0},  1,         {5, 1, 0},  {0x32CEBC72},   27000000,     270000000,  {0,0,0},     1,  .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, {0, 0} },

/* 4K */
	/* TODO: HDMI_ANALOG_CFG_297_X_1P5_594 - 297M x 1.5, clk os 594M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_X_1P25_594 - 297M x 1.25, clk os 594M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_594 - 297M, clk os 594M */
	{0},

	/* TODO: HDMI_ANALOG_CFG_594 - 594M */
	{0},

//+++++//

	/* TODO: HDMI_ANALOG_CFG_297_X_1P5 - 297M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297_X_1P25 - 297M */
	{0},
	/* TODO: HDMI_ANALOG_CFG_297 - 297M */
	{0},

	/* HDMI_ANALOG_CFG_54_X_1P5 - 54M x 1.5 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 3},  1,         {6, 1, 0},  {0x1FE1D271},  108000000,    270000000,  {1,0,0},     1,  .vsd_pll={0x5C4CB04A}, 1, .vsd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_54_X_1P25 - 54M x 1.25 */
	{ 0,   {1, 0}, .vsd_clk={1, 1, 1, 2},  1,         {6, 1, 0},  {0x1FE1D571},  108000000,    270000000,  {1,0,0},     1,  .vsd_pll={0x5C3AB04A}, 1, .vsd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_54 - 54M */
	{ 0,   {1, 0}, .vsd_clk={0, 0, 0, 0},  1,         {5, 1, 0},  {0x32CEBC72},  108000000,    270000000,  {1,0,0},     1,  .vsd_pll={0x1C28A04A}, 1, .vsd_pll_pi={0}, {0, 0} },

	/* HDMI_ANALOG_CFG_108 - 108M */
	{0},

};
#endif

