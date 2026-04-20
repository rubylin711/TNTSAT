/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_crm_reset_reg.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT Symphony6 CRM reset registers.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
*****************************************************************************/
#ifndef __INC__MT_CRM_RESET_REG_H__
#define __INC__MT_CRM_RESET_REG_H__

/* Symphony6 CRM Module Reset Registers Definition */

//MAC
#define REG_MAC_SRSTN				(REG_CRM_BASE + 0x18)
#define MACAHB_SRSTN_SHIFT			3
#define MACAXI_SRSTN_SHIFT			2
#define EPHY_SRSTN_SHIFT			1

//APCPU
#define REG_APCPU_SRSTN				(REG_CRM_BASE + 0x810C)
#define APCPU_NTRST_SHIFT			12	/* jtag */
#define APCPU_NPOTRST_SHIFT			11	/* jtag power */
#define APCPU_CSRST_SHIFT			10	/* coresight debug */
#define APCPU_AHBRST_SHIFT			9
#define APCPU_APBRST_SHIFT			8
#define L2_RST_SHIFT				6
#define NCORE_RST1_SHIFT			3	/* Core1 */
#define NCORE_RST0_SHIFT			2	/* Core0 */
#define NCOREPO_RST1_SHIFT			1	/* core1 poreset */
#define NCOREPO_RST0_SHIFT			0	/* core1 poreset */

//AVCPU
#define REG_AVCPU_SRSTN				(REG_CRM_BASE + 0x820C)
#define SPDMACORE_SRSTN_SHIFT		5
#define SPDMAAHB_SRSTN_SHIFT		4
#define SPDMAAXI_SRSTN_SHIFT		3
#define AVCPUCORE_SRSTN_SHIFT		2
#define AVCPUAHB_SRSTN_SHIFT		1
#define AVCPUAXI_SRSTN_SHIFT		0

//SYSTEM
#define REG_SYS_SRSTN				(REG_CRM_BASE + 0x830C)
#define OMC_PHY_SRSTN_SHIFT			18
#define OMC_CORE_SRSTN_SHIFT		17
#define OMC_PCLK_SRSTN_SHIFT		16
#define OMC_RCLK_SRSTN_SHIFT		15
#define SWTMISC_SRSTN_SHIFT			14
#define SWMNTSOSC_SRSTN_SHIFT		13
#define SWMNTXTAL_SRSTN_SHIFT		12
#define SWWDOG1_SRSTN_SHIFT			11
#define SWWDOG0_SRSTN_SHIFT			10
#define SWTIMER3_SRSTN_SHIFT		9
#define SWTIMER2_SRSTN_SHIFT		8
#define SWTIMER1_SRSTN_SHIFT		7
#define SWTIMER0_SRSTN_SHIFT		6
#define DMACORE_SRSTN_SHIFT			5
#define DMAAHB_SRSTN_SHIFT			4
#define DMAAXI_SRSTN_SHIFT			3
#define OIC_OMC_SRSTN_SHIFT			2
#define OIC_APB_SRSTN_SHIFT			1
#define OIC_AXI_SRSTN_SHIFT			0

//INTF
#define REG_INTF_SRSTN				(REG_CRM_BASE + 0x900C)
#define SDIO1CORE_SRSTN_SHIFT		30
#define SDIO1AXI_SRSTN_SHIFT		29
#define SDIO1AHB_SRSTN_SHIFT		28
#define SDIO0CORE_SRSTN_SHIFT		27
#define SDIO0AXI_SRSTN_SHIFT		26
#define SDIO0AHB_SRSTN_SHIFT		25
#define PNANDREG_SRSTN_SHIFT		24
#define PNANDCORE_SRSTN_SHIFT		23
#define PNANDAXI_SRSTN_SHIFT		22
#define PNANDAHB_SRSTN_SHIFT		21
#define USB30_SS_PHY_SRSTN_SHIFT	18
#define USB_DPHY_BRIDGE_SRSTN_SHIFT	17
#define SPI0REG_SRSTN_SHIFT			16
#define SPI0CORE_SRSTN_SHIFT		15
#define SPI0AHB_SRSTN_SHIFT			14
#define UART1_SRSTN_SHIFT			13
#define UART0_SRSTN_SHIFT			12
#define I2C1_SRSTN_SHIFT			11
#define I2C0_SRSTN_SHIFT			10
#define USB1PHY_SRSTN_SHIFT			9
#define USB1CORE_SRSTN_SHIFT		8
#define USB1AXI_SRSTN_SHIFT			7
#define USB1AHB_SRSTN_SHIFT			6
#define USB0PHY_SRSTN_SHIFT			5
#define USB0CORE_SRSTN_SHIFT		4
#define USB0AXI_SRSTN_SHIFT			3
#define USB0AHB_SRSTN_SHIFT			2
#define SMCPHY_SRSTN_SHIFT			1
#define SMC_SRSTN_SHIFT				0

//GRA
#define REG_GRA_SRSTN				(REG_CRM_BASE + 0xA10C)
#define GRACORE_SRSTN_SHIFT			2
#define GRAAXI_SRSTN_SHIFT			1
#define GRAAHB_SRSTN_SHIFT			0

//JPG
#define REG_JPG_SRSTN				(REG_CRM_BASE + 0xA20C)
#define JPGCORE_SRSTN_SHIFT			2
#define JPGAXI_SRSTN_SHIFT			1
#define JPGAHB_SRSTN_SHIFT			0

//DISP
#define REG_DISP_SRSTN				(REG_CRM_BASE + 0xA30C)
#define DISP_HDVENC_SRSTN_SHIFT		7
#define DISP_SDVENC_SRSTN_SHIFT		6
#define DISPVDC_SRSTN_SHIFT			5
#define DISPOSDC_SRSTN_SHIFT		4
#define DISPDI_SRSTN_SHIFT			3
#define DISPCORE_SRSTN_SHIFT		2
#define DISPAXI_SRSTN_SHIFT			1
#define DISPAHB_SRSTN_SHIFT			0

//VDEC
#define REG_VDEC_SRSTN				(REG_CRM_BASE + 0xA40C)
#define VDECCORECORE_SRSTN_SHIFT	4
#define VDECCORE_SRSTN_SHIFT		2
#define VDECAXI_SRSTN_SHIFT			1
#define VDECAHB_SRSTN_SHIFT			0

//AOUT
#define REG_AOUT_SRSTN				(REG_CRM_BASE + 0xA50C)
#define AOUT_SRC_SRSTN_SHIFT		5
#define AOUT_ADAC_SRSTN_SHIFT		4
#define AOUT_MCLK_SRSTN_SHIFT		3
#define AOUTCORE_SRSTN_SHIFT		2
#define AOUTAXI_SRSTN_SHIFT			1
#define AOUTAHB_SRSTN_SHIFT			0

//VOUT
#define REG_VOUT_SRSTN				(REG_CRM_BASE + 0xA60C)
#define HDMICORE_SRSTN_SHIFT		8
#define HDMIAHB_SRSTN_SHIFT			7
#define HDVENCHD_SRSTN_SHIFT		4
#define HDVENCAHB_SRSTN_SHIFT		3
#define VBI_SRSTN_SHIFT				2
#define SDVENCAHB_SRSTN_SHIFT		1
#define SDVENCCORE_SRSTN_SHIFT		0

//PNG
#define REG_PNG_SRSTN				(REG_CRM_BASE + 0xA70C)
#define PNGCORE_SRSTN_SHIFT			2
#define PNGAXI_SRSTN_SHIFT			1
#define PNGAHB_SRSTN_SHIFT			0

//DAI
#define REG_DAI_SRSTN				(REG_CRM_BASE + 0xA80C)
#define DAIDAC_SRSTN_SHIFT			5
#define DAIPDM_SRSTN_SHIFT			4
#define DAIRX_SRSTN_SHIFT			3
#define DAITX_SRSTN_SHIFT			2
#define DAIAXI_SRSTN_SHIFT			1
#define DAIAHB_SRSTN_SHIFT			0

//GPU
#define REG_GPU_SRSTN				(REG_CRM_BASE + 0xA90C)
#define GPU_2PSRAM_SRSTN_SHIFT		4
#define GPUMBIST_SRSTN_SHIFT		3
#define GPUAXI_SRSTN_SHIFT			2
#define GPUCORE_SRSTN_SHIFT			1
#define GPUAHB_SRSTN_SHIFT			0

//GMAC
#define REG_GMAC_SRSTN				(REG_CRM_BASE + 0xAA0C)
#define GMAC_CRM_SRSTN_SHIFT		2
#define GMACAXI_SRSTN_SHIFT			1
#define GMACAHB_SRSTN_SHIFT			0

//TSI
#define REG_TSI_SRSTN				(REG_CRM_BASE + 0xB00C)
#define TSI_FPREG_SRSTN_SHIFT		22
#define TSI_CIREG_SRSTN_SHIFT		21
#define TSI_TRPPREG_SRSTN_SHIFT		20
#define TSI_SFREG_SRSTN_SHIFT		19
#define TSI_T2MI_SRSTN_SHIFT		18

#define AVSYNCAHB_SRSTN_SHIFT		17
#define AVSYNC_PCR_SRSTN_SHIFT		16
#define AVSYNC_SYS_SRSTN_SHIFT		15

#define SFAXI_SRSTN_SHIFT			14
#define TRPPAXI_SRSTN_SHIFT			13
#define SWTSIAXI_SRSTN_SHIFT		12

#define DEMUX_SRSTN_SHIFT			11
#define TSPOOL_SRSTN_SHIFT			10

#define SWTSI_SRSTN_SHIFT			9
#define TRPP_SRSTN_SHIFT			8
#define SF_SRSTN_SHIFT				7

#define TS3_SRSTN_SHIFT				6
#define TS2_SRSTN_SHIFT				5
#define TS1_SRSTN_SHIFT				4
#define TS0_SRSTN_SHIFT				3

#define TSICORE_SRSTN_SHIFT			2
#define TSIAXI_SRSTN_SHIFT			1
#define TSIAHB_SRSTN_SHIFT			0

//CI
#define REG_CI_SRSTN				(REG_CRM_BASE + 0xB10C)
#define CIAHB_SRSTN_SHIFT			0

//SECURE
#define REG_SECURE_SRSTN			(REG_CRM_BASE + 0xC00C)
#define M2MCORE1_SRSTN_SHIFT		14
#define DS_M2REG_SRSTN_SHIFT		13
#define DS_M2CORE_SRSTN_SHIFT		12
#define DS_SRSTN_SHIFT				11
#define M2MCIPHER_SRSTN_SHIFT		10
#define M2MAHB_SRSTN_SHIFT			9
#define M2MAXI_SRSTN_SHIFT			8
#define M2MCORE_SRSTN_SHIFT			7
#define PKA_IBUS_SRSTN_SHIFT		6
#define PKACORE_SRSTN_SHIFT			5
#define GLITCHDET_SRSTN_SHIFT		4
#define KT_IBUS_SRSTN_SHIFT			3
#define KTCORE_SRSTN_SHIFT			2
#define SECHD0_SRSTN_SHIFT			1
#define KL_SRSTN_SHIFT				0

//IFCP - TODO
#define REG_IFCP_GLB_SRSTN			(REG_CRM_BASE + 0x34)
#define IFCP_SRSTN_SHIFT			0

/* TODO: Can Write by VSCPU Only! */
#define REG_SHAREREG_VSCPU_RST_CTRL		(REG_IFCP_SHAREREG_BASE + 0x50)
#define IFCP_SRSTN_LOCK3				25
#define IFCP_SRSTN_LOCK2				24
#define IFCP_SRSTN_LOCK1				18
#define IFCP_SRSTN_LOCK0				16
#define IFCP_CRYPTOFIFO_SRSTN			9
#define IFCP_KLM_SRSTN1					8
#define IFCP_KLM_SRSTN0					7
#define IFCP_CRYPTO_SRSTN2				6
#define IFCP_CRYPTO_SRSTN1				5
#define IFCP_CRYPTO_SRSTN0				4
#define IFCP_KT_SRSTN					2
#define IFCP_KLM_SRSTN					1
#define VSCPU_SEC_RSTN					0

////EXCEPTION////
#define REG_EXPRST_CFG					(REG_CRM_BASE + 0xF004)
#define REG_RST_CFG_REQ_SHIFT			0

#define REG_EXPWDOG0_RSTTIMERS			(REG_CRM_BASE + 0xF008)
#define WDOG0_RSTTIMES_SHIFT			0

#define REG_EXPWDOG1_RSTTIMERS			(REG_CRM_BASE + 0xF00C)
#define VSCPU_PCMNT_RSTTIMES_SHIFT		0

#define REG_EXPWDOG2_RSTTIMERS			(REG_CRM_BASE + 0xF010)
#define SANDWICH_FAILED_RSTTIMES_SHIFT	0	/* OTP sandwich */

#define REG_EXPWDOG3_RSTTIMERS			(REG_CRM_BASE + 0xF014)
#define PRELOAD_ERR_RSTTIMES_SHIFT		0	/* OTP FSM */

#define REG_EXPWDOG4_RSTTIMERS			(REG_CRM_BASE + 0xF018)
#define AO_WDOG_RSTTIMES_SHIFT			0

#define REG_EXPWDOG5_RSTTIMERS			(REG_CRM_BASE + 0xF01C)
#define M2M_BGCERR_RSTTIMES_SHIFT		0

#define REG_EXPWDOG6_RSTTIMERS			(REG_CRM_BASE + 0xF020)
#define REG_CFG_RSTTIMES_SHIFT			0	/* EXPRST_CFG_REG */

#define REG_EXPWDOG7_RSTTIMERS			(REG_CRM_BASE + 0xF024)
#define CLKMNT_RSTTIMES_SHIFT			0

#define REG_EXPWDOG8_RSTTIMERS			(REG_CRM_BASE + 0xF028)
#define GLITCH_RSTTIMES_SHIFT			0

#define REG_EXPWDOG9_RSTTIMERS			(REG_CRM_BASE + 0xF02C)
#define IFCP_FI_RSTTIMES_SHIFT			0

#define REG_EXPWDOG10_RSTTIMERS			(REG_CRM_BASE + 0xF030)
#define OTP_BGC_RSTTIMES_SHIFT			0

#define REG_EXPWDOG11_RSTTIMERS			(REG_CRM_BASE + 0xF034)
#define ARMCPU_FI_RSTTIMES_SHIFT		0

#define REG_EXPWDOG12_RSTTIMERS			(REG_CRM_BASE + 0xF038)
#define AVCPU_PCMNT_RSTTIMES_SHIFT		0

#define REG_EXPWDOG13_RSTTIMERS			(REG_CRM_BASE + 0xF03C)
#define ARMC0_PCMNT_RSTTIMES_SHIFT		0

#define REG_EXPWDOG14_RSTTIMERS			(REG_CRM_BASE + 0xF040)
#define ARMC1_PCMNT_RSTTIMES_SHIFT		0

#define REG_EXPWDOG15_RSTTIMERS			(REG_CRM_BASE + 0xF044)
#define WDOG1_RSTTIMES_SHIFT			0

#define REG_EXPWDOG16_RSTTIMERS			(REG_CRM_BASE + 0xF048)
#define TSENSOR_RSTTIMES_SHIFT			0

#define REG_EXPWDOG17_RSTTIMERS			(REG_CRM_BASE + 0xF04C)
#define AXI_MON_RSTTIMES_SHIFT			0

#define REG_EXPWDOG18_RSTTIMERS			(REG_CRM_BASE + 0xF050)
#define AVCPU_WDOG_RSTTIMES_SHIFT		0

#define REG_EXPWDOG19_RSTTIMERS			(REG_CRM_BASE + 0xF054)
#define TOTAL_EXPWDOG_RSTTIMES_SHIFT	0

////WDOGEXP
#define REG_WDOGEXP_CFG0				(REG_CRM_BASE + 0xFA00)
#define WDOGEXP_RSTTIME_SHIFT 			0

#define REG_WDOGEXP_CFG1				(REG_CRM_BASE + 0xFA04)
/* Wdogexp_mask */
#define WDOGEXP_AXI_MON_SHIFT			17
#define WDOGEXP_TSENSOR_SHIFT			16
#define WDOGEXP_WD1_SHIFT				15
#define WDOGEXP_ARMC1_PC_SHIFT			14
#define WDOGEXP_ARMC0_PC_SHIFT			13
#define WDOGEXP_AVCPU_PC_SHIFT			12
#define WDOGEXP_ARMCPU_SHIFT			11
#define WDOGEXP_OTP_BGC_SHIFT			10
#define WDOGEXP_IFCP_SHIFT				9
#define WDOGEXP_GLITCH_SHIFT			8
#define WDOGEXP_CLKMNT_SHIFT			7
#define WDOGEXP_RST_SHIFT				6
#define WDOGEXP_M2M_SHIFT				5
#define WDOGEXP_AOWD_SHIFT				4
#define WDOGEXP_OTP_FSM_SHIFT			3
#define WDOGEXP_SANDWICH_SHIFT			2
#define WDOGEXP_VSCPU_PC_SHIFT			1
#define WDOGEXP_WD0_SHIFT				0

#define REG_RSTSTATE_CLR				(REG_CRM_BASE + 0xFA08)
#define AVCPU_WD_STATE_CLR_SHIFT		18

#define REG_RSTSTATE					(REG_CRM_BASE + 0xFA0C)
#define AVCPU_WD_STATE_SHIFT			18

////AO
#define REG_AO_LPMRST				(REG_AO_CRM_BASE + 0x804)
#define AOLPM_SRSTN_SHIFT			0

#define REG_AO_IRDARST				(REG_AO_CRM_BASE + 0x80C)
#define IRDA_SRSTN_SHIFT			0

#define REG_AO_RTCRST				(REG_AO_CRM_BASE + 0x814)
#define RTC_SRSTN_SHIFT				0

#define REG_AO_LEDKBRST				(REG_AO_CRM_BASE + 0x81C)
#define LEDKB_SRSTN_SHIFT			0

#define REG_AO_GPIORST				(REG_AO_CRM_BASE + 0x824)
#define AOGPIO_SRSTN_SHIFT			0

#define REG_AO_KADCRST				(REG_AO_CRM_BASE + 0x82C)
#define KADC_SRSTN_SHIFT			0

#define REG_AO_FPI2CRST				(REG_AO_CRM_BASE + 0x83C)
#define FPI2C_SRSTN_SHIFT			0

#define REG_AO_FPSPIRST				(REG_AO_CRM_BASE + 0x844)
#define FPSPI_REG_SRSTN_SHIFT		1
#define FPSPI_SRSTN_SHIFT			0

#define REG_AO_PINMUXRST			(REG_AO_CRM_BASE + 0x854)
#define AOPINMUX_SRSTN_SHIFT		0

#define REG_AO_TIMERRST				(REG_AO_CRM_BASE + 0x85C)
#define AOTIMER_SRSTN_SHIFT			0

#define REG_AO_MNTRST				(REG_AO_CRM_BASE + 0x864)
#define MNTSOSC_SRSTN_SHIFT			1
#define MNTXTAL_SRSTN_SHIFT			0

#define REG_AO_MAILBOXRST			(REG_AO_CRM_BASE + 0x86C)
#define AOMAILBOX_SRSTN_SHIFT		0

#define REG_AO_CECRST				(REG_AO_CRM_BASE + 0x874)
#define CEC_SRSTN_SHIFT				0

#define REG_AO_AVSRST				(REG_AO_CRM_BASE + 0x87C)
#define AVS_SRSTN_SHIFT				0

#define REG_AO_AGTIMERRST			(REG_AO_CRM_BASE + 0x884)
#define AGTIMER_SRSTN_SHIFT			0

#define REG_AO_WDOGRST				(REG_AO_CRM_BASE + 0x88C)
#define AOWDOG_SRSTN_SHIFT			0

#endif

