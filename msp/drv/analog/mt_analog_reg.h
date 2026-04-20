/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_analog_reg.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT Symphony6 analog register definition.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC__MT_ANALOG_REG_H__
#define __INC__MT_ANALOG_REG_H__

#define REG_AO_ANALOG_SIZE		0x1000
#define REG_ANALOG_SW_SIZE		0x10000

#define ANA_AO_REG0				(REG_AO_ANALOG_BASE)
#define HDMI_TX_RESET_SHIFT		7
#define PROC_MNT_RESET_SHIFT	23
#define TSENSOR_PD_SHIFT		27

#define ANA_AO_REG1				(REG_AO_ANALOG_BASE + 4)

#define REG_ADAC_SW				(REG_ANALOG_SW_BASE)

#define INTP_CTRL_REG0			(REG_ANALOG_SW_BASE + 0x08)
#define INTP_CTRL_REG1			(REG_ANALOG_SW_BASE + 0x0C)

#define VDAC_REG0				(REG_ANALOG_SW_BASE + 0x10)
//#define VDAC_REG1				(REG_ANALOG_SW_BASE + 0x14)
//#define VDAC_REG2				(REG_ANALOG_SW_BASE + 0x18)

#define USB0_REG0				(REG_ANALOG_SW_BASE + 0x20)
#define USB0_REG1				(REG_ANALOG_SW_BASE + 0x24)
#define USB1_REG0				(REG_ANALOG_SW_BASE + 0x28)
#define USB1_REG1				(REG_ANALOG_SW_BASE + 0x2C)

#define USB0PHY_SW_REG0			(REG_ANALOG_SW_BASE + 0x30)
#define USB1PHY_SW_REG0			(REG_ANALOG_SW_BASE + 0x34)
#define USB0PHY_SW_REG1			(REG_ANALOG_SW_BASE + 0x38)
#define USB1PHY_SW_REG1			(REG_ANALOG_SW_BASE + 0x3C)

#define REG_CLKGEN_CPUPLL		(REG_ANALOG_SW_BASE + 0x48)
#define REG_CLKGEN_ADCPLL		REG_CLKGEN_CPUPLL
#define REG_CLKGEN_ETHINTP		(REG_ANALOG_SW_BASE + 0x4C)

#define REG_CLKGEN_USBPLL		(REG_ANALOG_SW_BASE + 0x54)
#define REG_CLKGEN_VHDPLL		(REG_ANALOG_SW_BASE + 0x58)
#define REG_CLKGEN_VHDINTP		(REG_ANALOG_SW_BASE + 0x5C)
#define REG_CLKGEN_VSDPLL		(REG_ANALOG_SW_BASE + 0x60)
#define REG_CLKGEN_VSDINTP		(REG_ANALOG_SW_BASE + 0x64)

/* Symphony4 */
/*#define HDMI_TX_REG0			(REG_ANALOG_SW_BASE + 0x6C)*/

#define ANA_TOP_REG0			(REG_ANALOG_SW_BASE + 0x90)
#define ANA_TOP_REG1			(REG_ANALOG_SW_BASE + 0x94)
/* 0: ADC input; 1: digital input */
#define SADC_INPUT_MODE_SHIFT	0
#define ADAC_RESET_SHIFT		2
/* 0: ADC input; 1: digital input */
#define CADC_INPUT_MODE_SHIFT	3

#define REG_CLKGEN_TEST			(REG_ANALOG_SW_BASE + 0x98)
#define REG_CLKGEN_PDSYS		(REG_ANALOG_SW_BASE + 0x9C)

#define ADACPHY_SW_REG0			(REG_ANALOG_SW_BASE + 0xA0)
#define ADACPHY_SW_REG1			(REG_ANALOG_SW_BASE + 0xA4)
#define ADACPHY_SW_REG2			(REG_ANALOG_SW_BASE + 0xA8)
#define ADACPHY_SW_REG3			(REG_ANALOG_SW_BASE + 0xAC)

#define CLKGEN_VIDEO_SW_HD_REG0	(REG_ANALOG_SW_BASE + 0xB0)
#define CLKGEN_VIDEO_SW_HD_REG1	(REG_ANALOG_SW_BASE + 0xB4)
#define CLKGEN_VIDEO_SW_SD_REG0	(REG_ANALOG_SW_BASE + 0xB8)
#define CLKGEN_VIDEO_SW_SD_REG1	(REG_ANALOG_SW_BASE + 0xBC)

#define REG_USB_IMP				(REG_ANALOG_SW_BASE + 0xC0)
#define REG_USB_LOGIC			(REG_ANALOG_SW_BASE + 0xC4)
#define SADC_REG0				(REG_ANALOG_SW_BASE + 0xC8)
#define SADC_REG1				(REG_ANALOG_SW_BASE + 0xCC)

#define CADC_REG0				(REG_ANALOG_SW_BASE + 0xD0)
#define REG_ADC_LOG				(REG_ANALOG_SW_BASE + 0xD4)
#define PM_SW_REG0				(REG_ANALOG_SW_BASE + 0xD8)
#define REG_VDDMNT_DOUT_RST		(REG_ANALOG_SW_BASE + 0xDC)

//#define REG_ANA_SW_LOCK			(REG_ANALOG_SW_BASE + 0xE0)

#define REG_TSENSOR_TH			(REG_ANALOG_SW_BASE + 0xE4)
#define REG_TSENSOR_INT			(REG_ANALOG_SW_BASE + 0xE8)

#define REG_TSENSOR				(REG_ANALOG_SW_BASE + 0xEC)
#define TSENSOR_RESET_SHIFT		(5)

#define REG_TSENSOR_DOUT		(REG_ANALOG_SW_BASE + 0xF0)
/* bit [12:0] */
#define TSENSOR_DOUT_MASK		(0x1FFF)

/* sec_osc_reg, sec_osc_log, sec_osc_regao */
#define REG_SEC_OSC				(REG_ANALOG_SW_BASE + 0xF4)

//symphony2 added
#define ANA_TOP_REG2			(REG_ANALOG_SW_BASE + 0xF8)
#define RNG1_RESET_SHIFT		(8+7)
#define RNG2_RESET_SHIFT		(19+7)

//#define REG_ANA_SW_CFG_LOCK		(REG_ANALOG_SW_BASE + 0x10C)

//symphony2-B0 added
#define REG_CLKGEN_EPHYPLL		(REG_ANALOG_SW_BASE + 0x118)
#define REG_CLKGEN_EPHY			(REG_ANALOG_SW_BASE + 0x11C)	/* 32bit */
#define REG_CLKGEN_EPHYRX		(REG_ANALOG_SW_BASE + 0x11C)	/* 16bit */
#define REG_CLKGEN_PDEPHY		(REG_ANALOG_SW_BASE + 0x11E)	/* 16bit */

//symphony4 added
#define CLKGEN_USBACLK_REG0		(REG_ANALOG_SW_BASE + 0x120)
#define CLKGEN_USBACLK_REG1		(REG_ANALOG_SW_BASE + 0x124)
#define CLKGEN_USBACLK_REG2		(REG_ANALOG_SW_BASE + 0x128)

#define REG_AADC_SW				(REG_ANALOG_SW_BASE + 0x12C)	/* not used? */
#define REG_OTPREG25			(REG_ANALOG_SW_BASE + 0x130)	/* security */

#define REG_BIAS				(REG_ANALOG_SW_BASE + 0x138)
#define ANA_TOP_REG3			(REG_ANALOG_SW_BASE + 0x13C)

#define REG_CLKGEN_VHDSSC		(REG_ANALOG_SW_BASE + 0x140)
#define REG_CLKGEN_VSDSSC		(REG_ANALOG_SW_BASE + 0x144)
#define REG_CLKGEN_SSCSET		REG_CLKGEN_VSDSSC

#define REG_VDDMONITOR_SET		(REG_ANALOG_SW_BASE + 0x148)
#define REG_TSENSOR_RST_REQ_TH	(REG_ANALOG_SW_BASE + 0x14C)

/* Control Register for 1.8V Output Current */
#define REG_LDO_PSW				(REG_ANALOG_SW_BASE + 0x150)

#define USB3P0_REG0				(REG_ANALOG_SW_BASE + 0x180)
#define USB3P0_REG1				(REG_ANALOG_SW_BASE + 0x184)
#define USB3P0_REG2				(REG_ANALOG_SW_BASE + 0x188)
#define USB3P0_REG3				(REG_ANALOG_SW_BASE + 0x18C)

#define USB3P0_REG4				(REG_ANALOG_SW_BASE + 0x190)
#define REG_USB3P0_RXCAL_L		(REG_ANALOG_SW_BASE + 0x194)
#define REG_USB3P0_RXCAL_H		(REG_ANALOG_SW_BASE + 0x198)
#define USB3P0_SSC_REG1			(REG_ANALOG_SW_BASE + 0x19C)

#define USB3P0_SSC_REG2			(REG_ANALOG_SW_BASE + 0x1A0)
#define REG_USB3P0_PLL			(REG_ANALOG_SW_BASE + 0x1A4)

/* Symphony6 */
#define HDMI_TX_REG0			(REG_ANALOG_SW_BASE + 0x1BC)
#define HDMI_TX_REG1			(REG_ANALOG_SW_BASE + 0x1C0)
#define HDMI_TX_REG2			(REG_ANALOG_SW_BASE + 0x1C4)
#define REG_HDMI_TEST			(REG_ANALOG_SW_BASE + 0x1C8)
#define REG_HDMI_LOGIC			(REG_ANALOG_SW_BASE + 0x1CC)

#define REG_USB3P0_POWER		(REG_ANALOG_SW_BASE + 0x1D4)

/* KEY_ADC_CFG */
#define REG_KEY_ADC_CFG			(REG_AO_KEY_ADC_BASE)
#define REG_KEY_ADC2_CFG		(REG_KADC_BASE)
#define KEY_ADC_PD_SHIFT		1	/* analog */
#define KEY_ADC_EN_SHIFT		4	/* digital */
#define KEY_ADC_RST_SHIFT		8	/* reset */

/* AVS_DAC_CPU */
#define REG_AVS_CPU_CTRL		(REG_AO_ANALOG_BASE + 0xC0)
#define AVS_DAC_CPU_SHIFT		0	/* bit 0-7 */
#define AVS_PD_CPU_SHIFT		8

/* AVS_DAC_SYS */
#define REG_AVS_SYS_CTRL		(REG_AO_ANALOG_BASE + 0xC4)
#define AVS_DAC_SYS_SHIFT		0	/* bit 0-7 */
#define AVS_PD_SYS_SHIFT		8

#endif

