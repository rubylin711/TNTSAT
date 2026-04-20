/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_crm_clock_reg.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT Symphony6 CRM clock registers.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
*****************************************************************************/
#ifndef __INC__MT_CRM_CLOCK_REG_H__
#define __INC__MT_CRM_CLOCK_REG_H__

/* Symphony6 CRM Module Clock Registers Definition */

//MAC
#define REG_MAC_CLKEN					(REG_CRM_BASE+0x1C)
#define RMII_CLKEN_SHIFT				5
#define MAC_CLKEN_SHIFT					4

#define REG_MAC_CLKSEL					(REG_CRM_BASE+0x50)
#define MAC_CLKMODE_SHIFT				8
#define MAC_CLKSEL_SHIFT				0

//BUS
#define REG_BUS_CLKSEL					(REG_CRM_BASE+0x8004)
#define AXI_CLKSEL_SHIFT				8
#define APB_CLKSEL_SHIFT				6
#define AHB_CLKSEL_SHIFT				3

//APCPU
#define REG_APCPU_CLKEN					(REG_CRM_BASE+0x8100)
#define APCPU_PCLKEN_SHIFT				1	/* APCPU apb clk */
#define APCPU_CLKEN_SHIFT				0

#define REG_APCPU_CLKSEL				(REG_CRM_BASE+0x8104)
#define BACKUP_ARMPLL_CLKSEL_SHIFT		7
#define ATCLK_CFG_FREQ_UPDATE_SHIFT		6
#define ATCLK_CFG_DIV_SEL_SHIFT			4
#define AP_BACKUP_CLK_SEL_SHIFT			0

/*
 Default: 9C38_910AH

 vco=Freq_xtal/(2+bit[11:8])*(32+bit[23:16])*(1+bit[12])

 e.g.
   bit[23:16]: 0x38(56)
   bit[12]: 1

   Freq_xtal: 24
   bit[11:8]: 1
   vco=24/(2+1)*(32+56)*(1+1)=1408

   Freq_xtal: 27
   bit[11:8]: 1
   vco=27/(2+1)*(32+56)*(1+1)=1584

   Freq_xtal: 40
   bit[11:8]: 3
   vco=40/(2+3)*(32+56)*(1+1)=1408

 */
#define REG_APCPU_ARMPLL				(REG_CRM_BASE+0x8108)
#define ARMPLL_SRESET_SHIFT				14

//AVCPU
#define REG_AVCPU_CLKEN					(REG_CRM_BASE+0x8200)
#define AVCPU_CLKEN_SHIFT				0

#define REG_AVCPU_CLKSEL				(REG_CRM_BASE+0x8204)
#define AVCPU_CLKSEL_SHIFT				0

//SYS
#define REG_SYS_CLKEN					(REG_CRM_BASE+0x8300)
/* sw timer/watch dog */
#define SW_TWM_CLKEN_SHIFT				5
#define DMA_CLKEN_SHIFT					4
#define OMCDFI_CLKEN_SHIFT				3
#define OMCPHY_CLKEN_SHIFT				2
#define AXI_REG_CLKEN_SHIFT				1
#define OIC_AXI_CLKEN_SHIFT				0

#define REG_SYS_CLKSEL					(REG_CRM_BASE+0x8304)
#define DDR_D3_DUTY_CLKSEL_SHIFT		3
#define OMC_CLKSEL_SHIFT				2
#define DMA_CLKSEL_SHIFT				0

//INTF
#define REG_INTF_CLKEN					(REG_CRM_BASE+0x9000)
#define USB1_PHY_UTMI_CLKEN_SHIFT		21
#define USB1_PHY_CLKEN_SHIFT			20
#define USB1_LFPS_CLKEN_SHIFT			19
#define USB1_PCLK_CLKEN_SHIFT			18
#define USB1_ACLK_CLKEN_SHIFT			17
#define USB1_APP_CLKEN_SHIFT			16
#define USB0_PHY_UTMI_CLKEN_SHIFT		15
#define USB0_PHY_CLKEN_SHIFT			14
#define USB0_AHB_AXI_CLKEN_SHIFT		13
#define SDIO1_CLKEN_SHIFT				12
#define SDIO0_CLKEN_SHIFT				11
#define PNAND_CLKEN_SHIFT				10
#define SPI0_CLKEN_SHIFT				8
#define UART1_CLKEN_SHIFT				7
#define UART0_CLKEN_SHIFT				6
#define I2CDEBUG_CLKEN_SHIFT			5
#define I2C1_CLKEN_SHIFT				4
#define I2C0_CLKEN_SHIFT				3
#define SMC_CLKEN_SHIFT					0

#define REG_INTF_CLKSEL					(REG_CRM_BASE+0x9004)
/* emmc */
#define SDIO1_CLKSEL_SHIFT				28
#define SDIO0_CLKSEL_SHIFT				24
#define SPI0_HIGH_CLKSEL_SHIFT			15
#define PNAND_HIGH_CLKSEL_SHIFT			12
#define UART1PHY_CLKSEL_SHIFT			10
#define UART0PHY_CLKSEL_SHIFT			9
#define USB1APP_CLKSEL_SHIFT			8
#define USB0_CORE_CLKSEL_SHIFT			7
#define PNAND_CORE_CLKSEL_SHIFT			5
#define SPI0_CORE_CLKSEL_SHIFT			1
#define SMC_PHY_CLKSEL_SHIFT			0

//GRA
#define REG_GRA_CLKEN					(REG_CRM_BASE+0xA100)
#define GRA_CLKEN_SHIFT					0

#define REG_GRA_CLKSEL					(REG_CRM_BASE+0xA104)
#define GRA_CLKSEL_SHIFT				0

//JPEG
#define REG_JPG_CLKEN					(REG_CRM_BASE+0xA200)
#define JPG_CLKEN_SHIFT					0

#define REG_JPG_CLKSEL					(REG_CRM_BASE+0xA204)
#define JPG_CLKSEL_SHIFT				0

//DISP
#define REG_DISP_CLKEN					(REG_CRM_BASE+0xA300)
#define DISP_CLKEN_SHIFT				0

#define REG_DISP_CLKSEL					(REG_CRM_BASE+0xA304)
#define DISPVDC_CLKSEL_SHIFT			12
#define DISPOSDC_CLKSEL_SHIFT			6
#define DISPDI_CLKSEL_SHIFT				3
#define DISPCORE_CLKSEL_SHIFT			0

//VDEC
#define REG_VDEC_CLKEN					(REG_CRM_BASE+0xA400)
#define VDEC_CLKEN_SHIFT				0

#define REG_VDEC_CLKSEL					(REG_CRM_BASE+0xA404)
#define VDEC_CLKSEL_SHIFT				0

//AOUT
#define REG_AOUT_CLKEN					(REG_CRM_BASE+0xA500)
#define SPDF_DIG_DIV_CLKEN_SHIFT		3
#define ADAC_CLKEN_SHIFT				2
#define SPDF_CLKEN_SHIFT				1
#define AOUT_CLKEN_SHIFT				0

#define REG_AOUT_CLKSEL					(REG_CRM_BASE+0xA504)
#define ADAC_MCLK_CLKSEL_SHIFT			8
#define SPDF_AUDIOPLL_CLKDIV_SHIFT		4
#define SPDF_MCLK_CLKSEL_SHIFT			2
#define AOUT_CLKSEL_SHIFT				0

#define REG_AOUT_SPDF_DIGDIV			(REG_CRM_BASE+0xA508)
#define SPDF_MCLK_INCREMENT_SHIFT		0	/* 24:0 */

//VOUT
#define REG_VOUT_CLKEN					(REG_CRM_BASE+0xA600)
#define SDVENC_SDCLK_27_SEL_SHIFT		15
#define HDMI_SCK_CLKEN_SHIFT			14
#define HDMI_HDCP2XCLK_CLKEN_SHIFT		13
#define HDMI_OSCCLK_CLKEN_SHIFT			12
#define HDMI_AUDCLK_CLKEN_SHIFT			11
#define HDMI_CECCLK_CLKEN_SHIFT			10
#define HDMI_CCLK_CLKEN_SHIFT			9
#define HDMI_HCLK_CLKEN_SHIFT			8
#define HDMI_XCLK_CLKEN_SHIFT			7
#define HDMI_MIFCLK_CLKEN_SHIFT			6
#define HDMI_TMDSCLK_CLKEN_SHIFT		5
#define HDMI_PIXCLKNX_CLKEN_SHIFT		4
#define HDVENC_CLKEN_SHIFT				2
#define VBI_CLKEN_SHIFT					1
#define SDVENC_CLKEN_SHIFT				0

#define REG_VOUT_CLKSEL					(REG_CRM_BASE+0xA604)
#define HDMI_AUD_CLKSEL_SHIFT			24
#define SDVENC_XTAL_CLKSEL_SHIFT		21
#define HDMI_XTAL_CLKSEL_SHIFT			20
#define HDMI_PIXNX_CLKSEL_SHIFT			16
#define HDMI_TMDS_CLKSEL_SHIFT			12
#define HDVENC_CLKSEL_SHIFT				0

//PNG
#define REG_PNG_CLKEN					(REG_CRM_BASE+0xA700)
#define PNG_CLKEN_SHIFT					0

#define REG_PNG_CLKSEL					(REG_CRM_BASE+0xA704)
#define PNG_CLKSEL_SHIFT				0

//DAI
#define REG_DAI_CLKEN					(REG_CRM_BASE+0xA800)
#define DAI_RX_DIGDIV_CLKEN_SHIFT		6
#define DAI_TX_DIGDIV_CLKEN_SHIFT		5
#define DAI_PDM_CLKEN_SHIFT				4
#define DAI_DAC_CLKEN_SHIFT				3
#define DAI_TX_CLKEN_SHIFT				2
#define DAI_RX_CLKEN_SHIFT				1
#define DAI_CLKEN_SHIFT					0

#define REG_DAI_CLKSEL					(REG_CRM_BASE+0xA804)
#define DAI_TX_CLKSEL_SHIFT				9
#define DAI_RX_CLKSEL_SHIFT				8
#define DAI_TX_AUDIOPLL_CLKDIV_SHIFT	4
#define DAI_RX_AUDIOPLL_CLKDIV_SHIFT	0

#define REG_DAI_TX_DIGDIV				(REG_CRM_BASE+0xA810)
#define DAI_TX_CLK_INCREMENT_SHIFT		0	/* 24:0 */

#define REG_DAI_RX_DIGDIV				(REG_CRM_BASE+0xA814)
#define DAI_RX_CLK_INCREMENT_SHIFT		0	/* 24:0 */

//GPU
#define REG_GPU_CLKEN					(REG_CRM_BASE+0xA900)
#define GPU_CLKEN_SHIFT					0

#define REG_GPU_CLKSEL					(REG_CRM_BASE+0xA904)
#define GPU_CLKSEL_SHIFT				0

//GMAC
#define REG_GMAC_CLKEN					(REG_CRM_BASE+0xAA00)
#define GMAC_BYPASS_RX_PRE_MODE_SHIFT	22
#define GMAC_PHYTX_CLKEN_SHIFT			21
#define GMAC_PHYREF_CLKEN_SHIFT			20
#define GMAC_RMII_CLKEN_SHIFT			19
#define GMAC_PTP_CLKEN_SHIFT			18
#define GMAC_RX_CLKEN_SHIFT				17
#define GMAC_TX_CLKEN_SHIFT				16
#define GMAC_CLKEN_SHIFT				0

#define REG_GMAC_CLKSEL					(REG_CRM_BASE+0xAA04)
#define GMAC_PHY_REF_CLKSEL_SHIFT		12
#define GMAC_RX_CLKSEL_SHIFT			10
#define GMAC_PTP_CLKSEL_SHIFT			8
#define GMAC_SPEED_CFG_SHIFT			4
#define GMAC_PHY_INTFSEL_SHIFT			0

#define REG_GMAC_CLKDLY					(REG_CRM_BASE+0xAA08)
#define GMAC_RXCLK_DLY_SMALL_SHIFT		20
#define GMAC_REFCLK_DLY_LARGE_SHIFT		16
#define GMAC_PHY_TXCLK_DLY_LARGE_SHIFT	4
#define GMAC_PHY_TXCLK_DLY_SMALL_SHIFT	0

//TSI
#define REG_TSI_CLKEN					(REG_CRM_BASE+0xB000)
#define TSI_CLKEN_SHIFT					0

#define REG_TSI_CLKSEL					(REG_CRM_BASE+0xB004)
#define TS0_CLKSEL_SHIFT				12
#define TS1_CLKSEL_SHIFT				8
#define TS2_CLKSEL_SHIFT				4
#define TS3_CLKSEL_SHIFT				2
#define TSI_CLKSEL_SHIFT				0

//CI
#define REG_CI_CLKEN					(REG_CRM_BASE+0xB100)
#define CI_CLKEN_SHIFT					0

//SECURE
#define REG_SECURE_CLKEN				(REG_CRM_BASE+0xC000)
#define DS_CLKEN_SHIFT					8
#define M2M_CLKEN_SHIFT					7
#define PKA_CLKEN_SHIFT					6
#define GLITCHDET_CLKEN_SHIFT			5
#define KT_CLKEN_SHIFT					4
#define SECHD0_CLKEN_SHIFT				3
#define KLE_CLKEN_SHIFT					2
#define KDF_CLKEN_SHIFT					1
#define SECMISC_CLKEN_SHIFT				0

#define REG_SECURE_CLKSEL				(REG_CRM_BASE+0xC004)
#define PKA_CLKSEL_SHIFT				6
#define DESCRAMBLE_CLKSEL_SHIFT			4
#define M2M_CIPHER_CLKSEL_SHIFT			2
#define SECURE_CLKSEL_SHIFT				0

//IFCP
#define REG_IFCP_CLKEN					(REG_CRM_BASE+0xC800)
#define IFCP_KLM_CLKEN_SHIFT			1
#define	IFCP_CRYPTO_CLKEN_SHIFT			0

#define REG_IFCP_CLKSEL					(REG_CRM_BASE+0xC804)
#define IFCP_CRYPTO_CLKSEL_SHIFT		3
#define IFCP_SYS_CLKSEL_SHIFT			0

#define REG_IFCP_VSCPU_MODE				(REG_CRM_BASE+0x30)
#define IFCP_VSCPU_MODE_SHIFT			0

//AO_LPM
#define REG_AO_LPMCLK					(REG_AO_CRM_BASE+0x800)
#define LPM_CLKEN_SHIFT					0

//AO_IRDA
#define REG_AO_IRDACLK					(REG_AO_CRM_BASE+0x808)
#define IRDA_CLKEN_SHIFT				0

//AO_LEDKB
#define REG_AO_LEDKBCLK					(REG_AO_CRM_BASE+0x818)
#define LEDKB_CLKEN_SHIFT				0

//AO_GPIO
#define REG_AO_GPIOCLK					(REG_AO_CRM_BASE+0x820)
#define AOGPIO_CLKEN_SHIFT				0

//AO_KADC
#define REG_AO_KADCCLK					(REG_AO_CRM_BASE+0x828)
#define KADC_CLKEN_SHIFT				0

//AO_ANAREG
#define REG_AO_ANAREGCLK				(REG_AO_CRM_BASE+0x830)
#define AO_ANAREG_CLKEN_SHIFT			0

//AO_FPI2C
#define REG_AO_FPI2CCLK					(REG_AO_CRM_BASE+0x838)
#define FPI2C_CLKEN_SHIFT				0

//AO_FPSPI
#define REG_AO_FPSPICLK					(REG_AO_CRM_BASE+0x840)
#define FPSPI_CLKEN_SHIFT				0

//AO_RECRAM
#define REG_AO_RECRAMCLK				(REG_AO_CRM_BASE+0x848)
#define RECRAM_CLKEN_SHIFT				0

//AO_PINMUX
#define REG_AO_PINMUXCLK				(REG_AO_CRM_BASE+0x850)
#define PINMUX_CLKEN_SHIFT				0

//AO_TIMER
#define REG_AO_TIMERCLK					(REG_AO_CRM_BASE+0x858)
#define TIMER_CLKEN_SHIFT				0

//AO_MAILBOX
#define REG_AO_MAILBOXCLK				(REG_AO_CRM_BASE+0x868)
#define MAILBOX_CLKEN_SHIFT				0

//AO_CEC
#define REG_AO_CECCLK					(REG_AO_CRM_BASE+0x870)
#define CEC_CLKEN_SHIFT					0

//AO_AVS
#define REG_AO_AVSCLK					(REG_AO_CRM_BASE+0x878)
#define AVS_CLKEN_SHIFT					0

//AO_AGTIMER
#define REG_AO_AGTIMERCLK				(REG_AO_CRM_BASE+0x880)
#define AGTIMER_CLKEN_SHIFT				0

//AO_AOMCU
#define REG_AO_AOMCUCLK					(REG_AO_CRM_BASE+0x888)
#define AOMCU_CLKEN_SHIFT				0

//XTAL
#define REG_XTAL_CLKSEL					(REG_AO_CRM_BASE+0x890)
#define XTAL_FREQ_SHIFT					1
#define XTAL_CLKSEL_SHIFT				0

//---------------------------------------------------------------------------//

//AOUT
#define REG_AOUT_AUD_I2S_SPDIF_CFG		(REG_AOUT_BASE+0x04)
#define HDMI_SPDIF_PATH_SEL_SHIFT		11

//APCPU_ARM
#define REG_APCPU_ARMPLL_PD				(REG_ARMCPU_BASE+0x1C)
#define ARMPLL_PD_SHIFT					0

//DEMOD
#define REG_DEMO_CFG					(REG_DEMO_BASE+0x0C04)
#define DEMO_C_EN_SHIFT					0
#define DEMO_J83B_EN_SHIFT				2
#define DEMO_S_EN_SHIFT					3

#endif

