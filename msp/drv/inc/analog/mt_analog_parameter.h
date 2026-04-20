/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Montage LZ Co., Ltd.
 */
#ifndef __INC_MT_ANALOG_PARAMETER_H__
#define __INC_MT_ANALOG_PARAMETER_H__

/*
 * ANALOG_PARAM_INDEX_HDMIPHY
 */
struct hdmiphy_param
{
	/* HDMI_TX_REG2: 0xBF5D01C4 */
	/*
	 * 0: unmute
	 * 1: mute
	 */
	u8 mute;				/* bit 28/29/30/31 */
};

/*
 * ANALOG_PARAM_INDEX_HDMICLK
 */
struct hdmiclk_param
{
	/* CLKGEN_USBACLK_REG0: 0xBF5D0120 */

	/*
	 * 0: follow sym4
	 * 1: 297 594 108 (sym6)
	 */
	u8 hdmi_clk_tmds_sel;	/* bit 8 */

	/*
	 * 0: follow sym4
	 * 1: 270x1 x1.25 x1.5 change to 540x1 x1.25 x1.5 (sym6)
	 */
	u8 sel540M;				/* bit 9 */

	/*
	 * 0: follow sym4
	 * 1: 594M
	 */
	u8 sel2970M;			/* bit 10 */
};

/*
 * ANALOG_PARAM_INDEX_HDMIDCC
 */
struct hdmidcc_param
{
	/* CLKGEN_USBACLK_REG0: 0xBF5D0120 */

	u8 dcc_xpd;				/* bit 1, 0: pd dcc */

	u8 dcc_sel;				/* bit 2-3 */
};

/*
 * ANALOG_PARAM_INDEX_HDMIPHY_TST
 */
struct hdmiphy_tst_param
{
	/* REG_HDMI_TEST: 0xBF5D01C8 */
	u32 all;
};

/*
 * ANALOG_PARAM_INDEX_HDMITX_CH
 */
struct hdmitx_ch_param
{
	/* HDMI_TX_REG0/HDMI_TX_REG1/HDMI_TX_REG2: 0xBF5D01BC/0xBF5D01C0/0xBF5D01C4 */
	u32 all;
};

/*
 * ANALOG_PARAM_INDEX_VHDCLK
 */
struct vhdclk_param
{
	/* REG_CLKGEN_PDSYS: 0xBF5D009C */

	/*
	 *  0: when hdmi_clk*1.0
	 *  1: when hdmi_clk*1.25/hdmi_clk*1.5
	 */
	u8 div_sel;			/* bit 8 */

	/*
	 * 0: hdmi_clk 1485M/742.5M
	 * 1: hdmi_clk 1485M*1.25  /1485M*1.5
	 *             742.5M*1.25/742.5M*1.5
	 */
	u8 clk_sel_1;		/* bit 9 */

	/*
	 * clk drv0 sel
	 * 0: when hdmi_clk*1.0
	 * 1: when hdmi_clk*1.25/hdmi_clk*1.5
	 */
	u8 drv0_sel;		/* bit 10 */

	/*
	 * 0x: hdmi_clk 1485M/742.5M*1.0
	 * 10: hdmi_clk 1485M/742.5M*1.25
	 * 11: hdmi_clk 1485M/742.5M*1.5
	 */
	u8 clk_sel_2;		/* bit 12-13 */
};

/*
 * ANALOG_PARAM_INDEX_VSDCLK
 */
struct vsdclk_param
{
	/* REG_CLKGEN_PDSYS: 0xBF5D009C */

	/*
	 *  0: when hdmi_clk*1.0
	 *  1: when hdmi_clk*1.25/hdmi_clk*1.5
	 */
	u8 div_sel;			/* bit 16 */

	/*
	 * 0: hdmi_clk 270M 1: hdmi_clk 270M*1.5/270M*1.25
	 */
	u8 clk_sel_1;		/* bit 17 */

	/*
	 * clk drv0 sel
	 * 0: when hdmi_clk*1.0
	 * 1: when hdmi_clk*1.25/hdmi_clk*1.5
	 */
	u8 drv0_sel;		/* bit 18 */

	/*
	 * 0x: hdmi_clk 270M*1.0
	 * 10: hdmi_clk 270M*1.25
	 * 11: hdmi_clk 270M*1.5
	 */
	u8 clk_sel_2;		/* bit 20-21 */
};

/*
 * ANALOG_PARAM_INDEX_VHDSSC
 */
struct vhdssc_param
{
	/* REG_CLKGEN_VHDSSC: 0xBF5D0140 */
	u32 all;			/* bit 0-31 */
};

/*
 * ANALOG_PARAM_INDEX_VSDSSC
 */
struct vsdssc_param
{
	/* REG_CLKGEN_VSDSSC: 0xBF5D0144 */

	/*
	 * 000: no ssc
	 * 001: ssc 1485 or 74.25
	 * 010: ssc 1485 or 74.24 x1 x1.25 x1.5 x2
	 * 011: ssc 1485 or 74.24 x1 x1.25 x1.5 x2
	 * 100: no ssc
	 * 101: ssc 270
	 * 110: ssc 270 x1.25 x1.5 x2
	 * 111: ssc 270 x1.25 x1.5 x2
	 */
	u8 clk_sel;				/* bit 0-2 */

	u8 en_ssc;				/* bit 3 */
	u8 pd_ssc;				/* bit 4 */
};

/*
 * ANALOG_PARAM_INDEX_VHDPLL
 */
struct vhdpll_param
{
	/* REG_CLKGEN_VHDPLL: 0xBF5D0058 */
	u32 all;			/* bit 0-31 */
};

/*
 * ANALOG_PARAM_INDEX_VSDPLL
 */
struct vsdpll_param
{
	/* REG_CLKGEN_VSDPLL: 0xBF5D0060 */
	u32 all;			/* bit 0-31 */
};

/*
 * ANALOG_PARAM_INDEX_VCLK_SRC
 */
struct vclk_source_param
{
	/* REG_CLKGEN_VHDINTP: 0xBF5D005C */

	/*
	 * A1
	 * 0: vsd os 108M from vsdpll
	 * 1: vsd os 108M from vhdpll
	 */
	u8 vsd_os_108M_sel;		/* bit 0 */

	/*
	 * A1
	 * 0: tmds 27M from vhdpll
	 * 1: tmds 54M/108M from vhdpll
	 */
	u8 tmds_sel; 			/* bit 7 */
};

/*

1. sadc
                 960M
                     \
                      sadc_clk_sel  --> sadc clk
                     /
1100M - pd_sadc_1100M

2. cadc
             144M
                 \
                  cadc_clk_sel  --> cadc clk
                 /
     pd_cadc_270M
    /
270M
    \
     pd_sdemod_270M  --> s-demod 270M

3. drv0_clk
            57.6M
                 \
                  drv0_clk_sel
                 /            \
            28.8M              drv0_clk_mux - pd_drv0_clk  --> drv0_clk
                              /
81M cpu pll                  /
           \                /
            drv0_clk_81M_mux
           /                \
81M vsd pll                  pd_cdemod_81M  --> c-demod 81M

4. drv1_clk
 192M
     \
      drv1_clk_sel - pd_drv1_clk  --> drv1_clk
     /
  96M

*/

/*
 * ANALOG_PARAM_INDEX_ADC
 */
struct adc_param
{
	/* sadc */
	struct {
		/* REG_CLKGEN_CPUPLL: 0xBF5D0048 */
		u8 pd_sadc_1100M;			/* bit 6, pd clk 1000M~1350M for s-adc (1350M) */
		u8 pd_sdemod_270M;			/* bit 7, pd drv0_clk (source in cpu pll) for s-demod (270M) */

		/* REG_CLKGEN_ETHINTP: 0xBF5D004C */
		u8 sadc_clk_sel;			/* bit 4, 0: 960M, 1: 1100M */
	} sadc;

	/* cadc */
	struct {
		/* REG_CLKGEN_CPUPLL: 0xBF5D0048 */
		u8 pd_cadc_270M;			/* bit 4, pd 270M for c-adc */
		u8 pd_cdemod_81M;			/* bit 5, pd 81M for c-demod */
		u8 cadc_clk_sel;			/* bit 13, 0: 144M, 1: 270M */
	} cadc;

	/* drv0_clk */
	struct {
		/* REG_CLKGEN_PDSYS: 0xBF5D009C */
		u8 pd_drv0_clk;				/* bit 24 */
		u8 drv0_clk_sel;			/* bit 26, 0: 57.6M, 1: 28.8M */

		/* REG_CLKGEN_ETHINTP: 0xBF5D004C */
		u8 drv0_clk_mux;			/* bit 1, 0: 28.8/57.6M, 1: 81M */
		u8 drv0_clk_81M_mux;		/* bit 5, 0: from cpu pll, 1: from vsd pll */
	} drv0;

	/* drv1_clk */
	struct {
		/* REG_CLKGEN_PDSYS: 0xBF5D009C */
		u8 pd_drv1_clk;				/* bit 25 */
		u8 drv1_clk_sel;			/* bit 27, 0: 192M, 1: 96M */
	} drv1;
};

/*
 * general pll parameters
 * <CNComment>只用于配置PLL必要的参数,不用于配置其它的参数!
 */
struct pll_param
{
	u8 div_cal_valid;
	u8 div_cal;						/* bit 4-7
	                                 * adc pll: reserved
	                                 * usb pll: valid
	                                 * vhd pll: valid
	                                 * vsd pll: feedback div setting, reserved(?)
	                                 * audio pll: valid
	                                 * usb3p0 pll: valid
	                                 * ephy pll: valid
	                                 * ddr pll: reserved
	                                 * arm pll: clk cpu sel, valid
	                                 */

	u16 div_front;					/* bit 8-12 */

	u8 pre_div_valid;
	u8 pre_div;						/* bit 13
	                                 * adc pll: reserved
	                                 * usb pll: valid
	                                 * vhd pll: reserved
	                                 * vsd pll: reserved
	                                 * audio pll: valid
	                                 * usb3p0 pll: valid
	                                 * ephy pll: reserved
	                                 * ddr pll: valid
	                                 * arm pll: reserved
	                                 */

	u16 div_fb;						/* bit 16-23 */

	u8 vco_ext;						/* bit 28-29, ext kvco sel */
	u8 vco_sel;						/* bit 30-31 */
};

//---------------------------------------------------------------------------//

/* Analog Parameter Enums */
enum MT_ANALOG_PARAM_INDEX_E
{
	/* HDMI_TX_REG2 */
	ANALOG_PARAM_INDEX_HDMIPHY = 1,		/* PHY mute, struct hdmiphy_param */

	/* REG_CLKGEN_PDSYS */
	ANALOG_PARAM_INDEX_VHDCLK,			/* vhd clk, struct vhdclk_param */
	ANALOG_PARAM_INDEX_VSDCLK,			/* vsd clk, struct vsdclk_param */

	/* REG_CLKGEN_VHDSSC */
	ANALOG_PARAM_INDEX_VHDSSC,			/* vhd ssc, struct vhdssc_param */
	/* REG_CLKGEN_VSDSSC */
	ANALOG_PARAM_INDEX_VSDSSC,			/* vsd ssc, struct vsdssc_param */

	/* REG_CLKGEN_VHDPLL */
	ANALOG_PARAM_INDEX_RAW_VHDPLL,		/* raw vhd pll, struct vhdpll_param */
	/* REG_CLKGEN_VSDPLL */
	ANALOG_PARAM_INDEX_RAW_VSDPLL,		/* raw vsd pll, struct vsdpll_param */

	ANALOG_PARAM_INDEX_ADC,				/* sadc/cadc, struct adc_param */

	ANALOG_PARAM_INDEX_ADCPLL,			/* adc pll, struct pll_param */
	ANALOG_PARAM_INDEX_USBPLL,			/* usb pll, struct pll_param */
	ANALOG_PARAM_INDEX_VHDPLL,			/* vhd pll, struct pll_param */
	ANALOG_PARAM_INDEX_VSDPLL,			/* vsd pll, struct pll_param */
	ANALOG_PARAM_INDEX_REFPLL,			/* ref pll, struct pll_param */
	ANALOG_PARAM_INDEX_USB3P0PLL,		/* usb30 pll, struct pll_param */
	ANALOG_PARAM_INDEX_EPHYPLL, 		/* ephy pll, struct pll_param */
	ANALOG_PARAM_INDEX_DDRPLL,			/* ddr pll, struct pll_param */
	ANALOG_PARAM_INDEX_ARMPLL,			/* arm pll, struct pll_param */

	ANALOG_PARAM_INDEX_HDMICLK,			/* hdmi clk select, struct hdmiclk_param */
	ANALOG_PARAM_INDEX_HDMIDCC, 		/* hdmi PHY clk DCC, struct hdmidcc_param */

	/* HDMI_TEST_REG0 */
	ANALOG_PARAM_INDEX_HDMIPHY_TST,		/* PHY test reg, struct hdmiphy_tst_param */

	ANALOG_PARAM_INDEX_HDMITX_CH0,		/* hdmitx reg0, struct hdmitx_ch_param */
	ANALOG_PARAM_INDEX_HDMITX_CH1, 		/* hdmitx reg0, struct hdmitx_ch_param */
	ANALOG_PARAM_INDEX_HDMITX_CH2, 		/* hdmitx reg2, struct hdmitx_ch_param */

	ANALOG_PARAM_INDEX_VCLK_SRC,		/* clkgen_vhdintp_reg, struct vclk_source_param */

	ANALOG_PARAM_INDEX_MAX
};

/**
 * @brief get Analog parameter
 *
 * @param[in] index Analog parameter index
 * @param[out] para Analog parameter
 *
 *    index                        parameter
 *    ANALOG_PARAM_INDEX_HDMIPHY   struct hdmiphy_param*
 *    ANALOG_PARAM_INDEX_VHDCLK    struct vhdclk_param*
 *    ANALOG_PARAM_INDEX_VSDCLK    struct vsdclk_param*
 *    ...
 *
 * @return
 *    0: success
 *   !0: failure
 */
int mt_analog_get_parameter(enum MT_ANALOG_PARAM_INDEX_E index, void *para);

/**
 * @brief set Analog parameter
 *
 * @param[in] index Analog parameter index
 * @param[in] para Analog parameter
 *
 *    index                        parameter
 *    ANALOG_PARAM_INDEX_HDMIPHY   struct hdmiphy_param*
 *    ANALOG_PARAM_INDEX_VHDCLK    struct vhdclk_param*
 *    ANALOG_PARAM_INDEX_VSDCLK    struct vsdclk_param*
 *    ...
 *
 * @return
 *    0: success
 *   !0: failure
 */
int mt_analog_set_parameter(enum MT_ANALOG_PARAM_INDEX_E index, void *para);

#endif	//__INC_MT_ANALOG_PARAMETER_H__

