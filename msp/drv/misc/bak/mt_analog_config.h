/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_analog_config.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/05/14
 * Description    : Montage Analog Configuration.
 * History        :
 * 1.Date         : 2021/05/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_ANALOG_CONFIG_H__
#define __INC_MT_ANALOG_CONFIG_H__

#ifdef __UBOOT__
#if !defined(CONFIG_MT_CHIP_SYMPHONY1) && !defined(CONFIG_MT_CHIP_SYMPHONY2) && !defined(CONFIG_MT_CHIP_SYMPHONY4)
//#define CONFIG_MT_CHIP_SYMPHONY4
#define CONFIG_MT_CHIP_SYMPHONY6
#endif
#endif

#define CFG_MT_ANA_RESET_UDELAY		10

/* PD(Power Down) Modules Configuration */
//#define CFG_MT_ANA_PD_USB0
//#define CFG_MT_ANA_PD_USB1

//#define CFG_MT_ANA_PD_VDAC0
//#define CFG_MT_ANA_PD_VDAC0_DET

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)

//#define CFG_MT_ANA_PD_ADAC_L
//#define CFG_MT_ANA_PD_ADAC_R
//#define CFG_MT_ANA_PD_VDAC1
//#define CFG_MT_ANA_PD_VDAC2
//#define CFG_MT_ANA_PD_VDAC3
//#define CFG_MT_ANA_PD_VDAC1_DET
//#define CFG_MT_ANA_PD_VDAC2_DET
//#define CFG_MT_ANA_PD_VDAC3_DET

//#define CFG_MT_ANA_PD_CPU_PLL

#endif

//#define CFG_MT_ANA_ADAC_MUTE
//#define CFG_MT_ANA_PD_HDMITX_CH
//#define CFG_MT_ANA_PD_CADC
//#define CFG_MT_ANA_PD_SADC
#define CFG_MT_ANA_PD_PROC_MON
#define CFG_MT_ANA_PD_RNG1

//#define CFG_MT_ANA_PD_VSD_PLL
//#define CFG_MT_ANA_PD_VHD_PLL
//#define CFG_MT_ANA_PD_USB_PLL

/* clkgen_pdsys_reg: bit[31], pd clk cal in xtaltop */
//#define CFG_MT_ANA_PD_CLK_CAL

#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)

#define CFG_MT_ANA_PD_TEMP_SENSOR
#define CFG_MT_ANA_PD_RNG2
//#define CFG_MT_ANA_PD_EPHY_PLL

#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)

#define CFG_MT_ANA_PD_AADC_0
#define CFG_MT_ANA_PD_AADC_1

//#define CFG_MT_ANA_PD_ADAC_BUF
/* ADAC: 2Vrms or 0dBu */
#define CFG_MT_ANA_ADAC_2VRMS
//#define CFG_MT_ANA_ADAC_0DBU

//#define CFG_MT_ANA_PD_OTP_REG
#define CFG_MT_ANA_PD_AADC_MIC
//#define CFG_MT_ANA_PD_ADC_PLL

/* clkgen_usbaclk_reg0: bit[11], pd aclk_top reg */
#define CFG_MT_ANA_PD_ACLK_TOP

#endif

#endif	//__INC_MT_ANALOG_CONFIG_H__

