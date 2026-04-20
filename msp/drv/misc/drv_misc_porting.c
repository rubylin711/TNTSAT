/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef MKSTR
#define MKSTR(x)					#x
#endif

//#ifndef AXI_REG_CLK
//#define AXI_REG_CLK				"axi_reg_clk"
//#endif
//#ifndef DISP_OSDC_CLK
//#define DISP_OSDC_CLK			"disp_osdc_clk"
//#endif
//#ifndef SW_TWM_CLK
//#define SW_TWM_CLK				"sw_twm_clk"
//#endif

/* mod id map to mod name */
struct misc_mod_map
{
	unsigned int mod_id;
	void *name;
};

/* misc group porting */
struct misc_group_port
{
	struct misc_mod_map *map_tab;

	int (*reset)(void *name);
	int (*enable)(void *name);
	int (*disable)(void *name);
	int (*get_status)(void *name, u32 *status);

	int (*get_rate)(void *name, unsigned long *rate);
};

/* analog map table */
static struct misc_mod_map analog_map_tab[] =
{
	{HAL_CADC, 			(void*)MT_ANA_CADC},
	{HAL_SADC, 			(void*)MT_ANA_SADC},
	{HAL_VDAC0, 		(void*)MT_ANA_VDAC},
	{HAL_RNG, 			(void*)MT_ANA_RNG1},
	{HAL_RNG2, 			(void*)MT_ANA_RNG2},
	//{HAL_PM, 			(void*)MT_ANA_PROCMON},	/* process monitor */
	{HAL_TSENSOR, 		(void*)MT_ANA_TSENSOR},

	{HAL_ADCPLL,		(void*)MT_ADCPLL},

	{HAL_EPHYPLL,		(void*)MT_EPHYPLL},
	//{HAL_AUDIOPLL,		(void*)MT_AUDIOPLL},

	{0, NULL},
};

/* clock map table */
static struct misc_mod_map clock_map_tab[] =
{
	//Sym6 AXI has no gate
	{HAL_AXI,			(void*)MT_CLK_AXI},
	{HAL_HB,			(void*)MT_CLK_AHB},
	{HAL_PB,			(void*)MT_CLK_APB},

	{HAL_CPU1,			(void*)MT_CLK_AVCPU},

	{HAL_DMA,			(void*)MT_CLK_DMA},

	{HAL_I2C0,			(void*)MT_CLK_I2C0},
	{HAL_I2C1,			(void*)MT_CLK_I2C1},
	{HAL_I2C_DEBUG,		(void*)MT_CLK_I2CDEBUG},
	{HAL_SPI0,			(void*)MT_CLK_SPI0},
	{HAL_PNAND,			(void*)MT_CLK_PNAND},
	{HAL_SDIO0,			(void*)MT_CLK_SDIO0},
	{HAL_SDIO1,			(void*)MT_CLK_SDIO1},
	{HAL_UART0,			(void*)MT_CLK_UART0},
	{HAL_UART1,			(void*)MT_CLK_UART1},

	{HAL_GPE,			(void*)MT_CLK_GRA},
	{HAL_JPEG,			(void*)MT_CLK_JPG},

	{HAL_DISPLAY,		(void*)MT_CLK_DISP},
	{HAL_DI, 			(void*)MT_CLK_DISP_DI},
	//{HAL_OSDC,			(void*)DISP_OSDC_CLK},

	{HAL_VDEC,			(void*)MT_CLK_VDEC},

	{HAL_AUDIO_OUT,		(void*)MT_CLK_AOUT},
	{HAL_SPDF,			(void*)MT_CLK_SPDIF},

	{HAL_HD_VIDEO,		(void*)MT_CLK_HDVENC},
	{HAL_VBI,			(void*)MT_CLK_VBI},
	{HAL_SD_VIDEO,		(void*)MT_CLK_SDVENC},

	{HAL_PNG,			(void*)MT_CLK_PNG},

	//{HAL_DAI,			(void*)MT_CLK_DAI},

	{HAL_TSI,			(void*)MT_CLK_TSI},
	//Sym6 TS0/TS1/TS2/TS3 have no gates
	//{HAL_TS0,			(void*)},
	//{HAL_TS1,			(void*)},
	//{HAL_TS2,			(void*)},
	//{HAL_TS3,			(void*)},

	{HAL_CI,			(void*)MT_CLK_CI},

	{HAL_DDRMC,			(void*)MT_CLK_DDRPHY},

	{HAL_GPU,			(void*)MT_CLK_GPU},
	{HAL_GMAC,			(void*)MT_CLK_GMAC},

	{HAL_DS, 			(void*)MT_CLK_DSC},
	{HAL_M2M,			(void*)MT_CLK_M2M},
	{HAL_PKA,			(void*)MT_CLK_PKA},
	{HAL_GLITCH_DET,	(void*)MT_CLK_GLITCHDET},
	{HAL_KT,			(void*)MT_CLK_KT},
	{HAL_SECHD0,		(void*)MT_CLK_SECHD0},
	{HAL_KL_CW,			(void*)MT_CLK_KLE},
	{HAL_KDF,			(void*)MT_CLK_KDF},
	{HAL_SECURE,		(void*)MT_CLK_SECURE},

	{HAL_IFCP_KLM,		(void*)MT_CLK_IFCP_KLM},
	{HAL_IFCP_CRYPTO,	(void*)MT_CLK_IFCP_CRYPTO},
	{HAL_IFCP_SYS,		(void*)MT_CLK_IFCP_SYS},

	{HAL_AO_LPMSET,		(void*)MT_CLK_AO_LPM},
	{HAL_AO_IRDA,		(void*)MT_CLK_AO_IRDA},
	{HAL_AO_LEDKB,		(void*)MT_CLK_AO_LEDKB},
	{HAL_AO_GPIO,		(void*)MT_CLK_AO_GPIO},
	{HAL_AO_KADC,		(void*)MT_CLK_AO_KADC},
	{HAL_AO_ANA,		(void*)MT_CLK_AO_ANAREG},
	{HAL_AO_FPI2C,		(void*)MT_CLK_AO_FPI2C},
	{HAL_AO_FPSPI,		(void*)MT_CLK_AO_FPSPI},
	{HAL_AO_RECRAM,		(void*)MT_CLK_AO_RECRAM},
	{HAL_AO_PINMUX,		(void*)MT_CLK_AO_PINMUX},
	{HAL_AO_TIMER0,		(void*)MT_CLK_AO_TIMER},
	//Sym6 has no AO timer1
	//{HAL_AO_TIMER1,		(void*)},
	{HAL_AO_MAILBOX,	(void*)MT_CLK_AO_MAILBOX},
	{HAL_AO_CEC,		(void*)MT_CLK_AO_CEC},
	{HAL_AO_AVS,		(void*)MT_CLK_AO_AVS},
	{HAL_AO_AGTIMER,	(void*)MT_CLK_AO_AGTIMER},
	{HAL_AO_MCU,		(void*)MT_CLK_AO_MCU},
	//Sym6 has no AO Uart
	//{HAL_AO_UART,		(void*)MT_CLK_AO_UART},

	{HAL_XTAL,			(void*)MT_CLK_XTAL},

	{HAL_DEMO_BUS,		(void*)MT_CLK_DEMO},
	{HAL_DEMO_C,		(void*)MT_CLK_DEMO_C},
	{HAL_DEMO_S,		(void*)MT_CLK_DEMO_S},
	{HAL_DEMO_J83B,		(void*)MT_CLK_DEMO_J83B},

	{0, NULL},
};

/* composite module(analog&clock) map table */
static struct misc_mod_map comp_map_tab[] =
{
	{HAL_HDMI,			(void*)MKSTR(HAL_HDMI)},
	{HAL_MAC,			(void*)MKSTR(HAL_MAC)},
	{HAL_USB0,			(void*)MKSTR(HAL_USB0)},
	{HAL_USB1,			(void*)MKSTR(HAL_USB1)},
	{HAL_USB1_P20,		(void*)MKSTR(HAL_USB1_P20)},
	{HAL_ADAC,			(void*)MKSTR(HAL_ADAC)},
	{HAL_DAI,			(void*)MKSTR(HAL_DAI)},
	{HAL_SMC,			(void*)MKSTR(HAL_SMC)},

	{0, NULL},
};

/**
 * get module name by id
 */
static void *get_mod_name(unsigned int mod_id, struct misc_mod_map *map)
{
	while (map->name)
	{
		if (mod_id == map->mod_id)
			return map->name;

		map ++;
	}

	return NULL;
}

static int analog_reset_porting(void *ana)
{
	int ret = 0;

	ret = mt_analog_reset((mt_ana_t*)ana);
	if (ret == 0)
	{
		udelay(CFG_MT_ANA_RESET_UDELAY_MAX);

		ret = mt_analog_release((mt_ana_t*)ana);
	}

	return ret;
}

static int analog_enable_porting(void *ana)
{
	/* Patch */
	if (strcmp((char*)ana, MT_ANA_ADAC) == 0)
	{
		return board_adac_onoff(1);
	}

	return mt_analog_enable((mt_ana_t*)ana);
}

static int analog_disable_porting(void *ana)
{
	/* Patch */
	if (strcmp((char*)ana, MT_ANA_ADAC) == 0)
	{
		return board_adac_onoff(0);
	}

	return mt_analog_disable((mt_ana_t*)ana);
}

static int analog_get_status_porting(void *ana, u32 *status)
{
	return mt_analog_get_status((mt_ana_t*)ana, status);
}

static int clock_reset_porting(void *clk)
{
	return mt_clk_reset((mt_clk_t*)clk);
}

static int clock_enable_porting(void *clk)
{
	return mt_clk_enable((mt_clk_t*)clk);
}

static int clock_disable_porting(void *clk)
{
	return mt_clk_disable((mt_clk_t*)clk);
}

static int clock_get_status_porting(void *clk, u32 *status)
{
	return mt_clk_get_status((mt_clk_t*)clk, status);
}

static int clock_get_rate_porting(void *clk, unsigned long *rate)
{
	return mt_clk_get_rate((mt_clk_t*)clk, rate);
}

static int comp_reset_porting(void *mod)
{
	int id;

	id = mt_hal_mod_get_id(mod);

	return mt_hal_mod_reset(id, 0);
}

static int comp_enable_porting(void *mod)
{
	int id;

	id = mt_hal_mod_get_id(mod);

	/* Patch */
	if (id == HAL_ADAC)
	{
		return board_adac_onoff(1);
	}

	return mt_hal_mod_enable(id);
}

static int comp_disable_porting(void *mod)
{
	int id;

	id = mt_hal_mod_get_id(mod);

	/* Patch */
	if (id == HAL_ADAC)
	{
		return board_adac_onoff(0);
	}

	return mt_hal_mod_disable(id);
}

static int comp_get_status_porting(void *mod, u32 *status)
{
	int id;

	id = mt_hal_mod_get_id(mod);

	return mt_hal_mod_get_status(id, (int*)status);
}

static int comp_get_rate_porting(void *mod, unsigned long *rate)
{
	int id;

	id = mt_hal_mod_get_id(mod);

	return mt_hal_mod_get_rate(id, rate);
}

static struct misc_group_port misc_groups[] =
{
	/* analog */
	{ .map_tab 		= analog_map_tab,
	  .reset 		= analog_reset_porting,
	  .enable 		= analog_enable_porting,
	  .disable 		= analog_disable_porting,
	  .get_status 	= analog_get_status_porting,
	  .get_rate 	= NULL
	},
	/* clock */
	{ .map_tab 		= clock_map_tab,
	  .reset 		= clock_reset_porting,
	  .enable 		= clock_enable_porting,
	  .disable 		= clock_disable_porting,
	  .get_status 	= clock_get_status_porting,
	  .get_rate 	= clock_get_rate_porting,
	},
	/* composite module */
	{ .map_tab		= comp_map_tab,
	  .reset		= comp_reset_porting,
	  .enable		= comp_enable_porting,
	  .disable		= comp_disable_porting,
	  .get_status	= comp_get_status_porting,
	  .get_rate 	= comp_get_rate_porting,
	},
};

static unsigned int misc_group_count = sizeof(misc_groups) / sizeof(misc_groups[0]);

#define FOR_EACH_MISC_GROUP \
					unsigned int i; \
					struct misc_group_port *pGrp = &misc_groups[0]; \
					for (i=0; i<misc_group_count; i++, pGrp++)

