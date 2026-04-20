/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __INC_MT_MODULE_INTERNAL_H__
#define __INC_MT_MODULE_INTERNAL_H__


/********************** Clock Modules ********************/

/* APCPU */
#define APCPU_PCLK				"apcpu_pclk"			/* APB */

/* SYS */
#define OIC_AXI_CLK				"oic_axi_clk"
#define AXI_REG_CLK				"axi_reg_clk"
#define OMC_PHY_CLK				"omc_phy_clk"
#define OMC_DFI_CLK				"omc_dfi_clk"
#define SW_TWM_CLK				"sw_twm_clk"			/* sw timer/watch dog */

/* INTF */
#define USB1_PHY_UTMI_CLK		"usb1_phy_utmi_clk"
#define USB1_PHY_CLK			"usb1_phy_clk"
#define USB1_LFPS_CLK			"usb1_lfps_clk"
#define USB1_PCLK				"usb1_pclk"				/* APB */
#define USB1_ACLK				"usb1_aclk"				/* AXI */
#define USB1_APP_CLK			"usb1_app_clk"

#define USB0_PHY_UTMI_CLK		"usb0_phy_utmi_clk"
#define USB0_PHY_CLK			"usb0_phy_clk"
#define USB0_AHB_AXI_CLK		"usb0_ahb_axi_clk"
//#define USB0_CORE_CLK			"usb0_core_clk"

#define PNAND_HIGH_CLK			"pnand_high_clk"
#define SPI0_HIGH_CLK			"spi0_high_clk"

/* DISP */
#define DISP_OSDC_CLK			"disp_osdc_clk"
#define DISP_VDC_CLK			"disp_vdc_clk"

/* AOUT */
#define SPDF_DIGDIV_CLK			"spdf_digdiv_clk"
#define SPDF_AUDIOPLL_CLK		"spdf_audiopll_clk"
#define SPDF_DIGDIV_MCLK		"spdf_digdiv_mclk"
#define AOUT_MCLK				"aout_mclk"

/* VOUT */
#define AUDIO_MCLK_HDMI			"audio_mclk_hdmi"
#define SDVENC_27M_CLK			"sdvenc_27m_clk"
//#define HDMI_TMDS_CLK			"hdmi_tmds_clk"
//#define HDMI_PIXNX_CLK			"hdmi_pixnx_clk"
#define HDMI_AUD_CLK			"hdmi_aud_clk"
#define HDMI_MIF_CLK			"hdmi_mif_clk"
#define HDMI_XCLK				"hdmi_xclk"
#define HDMI_HCLK				"hdmi_hclk"
#define HDMI_CCLK				"hdmi_cclk"
#define HDMI_CEC_CLK			"hdmi_cec_clk"
#define HDMI_OSC_CLK			"hdmi_osc_clk"
#define HDMI_HDCP2X_CLK			"hdmi_hdcp2x_clk"
#define HDMI_SCK				"hdmi_sck"

/* DAI */
#define DAI_RX_CLK				"dai_rx_clk"
#define DAI_TX_CLK				"dai_tx_clk"
#define DAI_DAC_CLK				"dai_dac_clk"
#define DAI_PDM_CLK				"dai_pdm_clk"

#define DAI_TX_DIGDIV_CLK		"dai_tx_digdiv_clk"
#define DAI_RX_DIGDIV_CLK		"dai_rx_digdiv_clk"
#define DAI_TX_AUDIOPLL_CLK		"dai_tx_audiopll_clk"
#define DAI_RX_AUDIOPLL_CLK		"dai_rx_audiopll_clk"

/* GPU */
#define GPU_MBIST_CLK			"gpu_mbist_clk"

/* GMAC */
#define GMAC_PHY_REFCLK			"gmac_phy_refclk"
#define GMAC_RX_CLK				"gmac_rx_clk"
#define GMAC_PTP_CLK			"gmac_ptp_clk"
#define GMAC_TX_CLK				"gmac_tx_clk"
#define GMAC_RMII_CLK			"gmac_rmii_clk"
#define GMAC_PHYTX_CLK			"gmac_phytx_clk"

/* TSI */
#define TS0_CLK					"ts0_clk"
#define TS1_CLK					"ts1_clk"
#define TS2_CLK					"ts2_clk"
#define TS3_CLK					"ts3_clk"


/********************** Reset Modules ********************/

/* EMAC */
#define MT_RST_EPHY				"ephy_rst"

/* SYS */
#define MT_RST_OIC				"oic"
#define MT_RST_OMC				"omc"
#define MT_RST_SWT_MISC			"swt_misc"
#define MT_RST_SWMNT_SOSC		"swmnt_sosc"
#define MT_RST_SWMNT_XTAL		"swmnt_xtal"
#define MT_RST_SWWDOG1			"sw_wdg1"
#define MT_RST_SWWDOG0			"sw_wdg0"
#define MT_RST_SWTM_CLK3		"sw_twm_clk3"
#define MT_RST_SWTM_CLK2		"sw_twm_clk2"
#define MT_RST_SWTM_CLK1		"sw_twm_clk1"
#define MT_RST_SWTM_CLK0		"sw_twm_clk0"

#define MT_RST_USB30			"usb30_rst"

/* TSI */
#define MT_RST_TSI_TSPOOL		"tsi_tspool"
#define MT_RST_TSI_DEMUX		"tsi_demux"
#define MT_RST_TSI_AVSYNC		"tsi_avsync"
#define MT_RST_TSI_T2MI			"tsi_t2mi"
#define MT_RST_TSI_CI_REG		"tsi_ci_reg"
#define MT_RST_TSI_FP_REG		"tsi_fp_reg"

#define MT_RST_TSI_SF			"tsi_sf"
#define MT_RST_TSI_TRPP			"tsi_trpp"
#define MT_RST_TSI_SWTSI		"tsi_swtsi"

/* IFCP */
//#define MT_RST_IFCP_GLB			"ifcp_glb"

/* AO */
#define MT_RST_AO_RTC			"ao_rtc"
#define MT_RST_AO_FPSPI_REG		"ao_fpspi_reg"
#define MT_RST_AO_MNT_XTAL		"ao_mnt_xtal"
#define MT_RST_AO_MNT_SOSC		"ao_mnt_sosc"
#define MT_RST_AO_WDOG			"ao_wdg"

#endif

