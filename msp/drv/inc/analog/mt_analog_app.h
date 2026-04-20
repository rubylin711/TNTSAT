/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */
#ifndef __INC_MT_ANALOG_APP_H__
#define __INC_MT_ANALOG_APP_H__

/* APL API for HDMI, ADC, etc. */

/* Enums for HDMI analog configurations */
enum MT_HDMI_ANALOG_CFG_E
{
	HDMI_ANALOG_CFG_148P5_X_1P5,		/* 148.5M x 1.5 */
	HDMI_ANALOG_CFG_148P5_X_1P25,		/* 148.5M x 1.25 */
	HDMI_ANALOG_CFG_148P5,				/* 148.5M */

	HDMI_ANALOG_CFG_74P25_X_1P5,		/* 74.25M x 1.5 */
	HDMI_ANALOG_CFG_74P25_X_1P25,		/* 74.25M x 1.25 */
	HDMI_ANALOG_CFG_74P25,				/* 74.25M */

	HDMI_ANALOG_CFG_27_X_1P5,			/* 27M x 1.5 */
	HDMI_ANALOG_CFG_27_X_1P25,			/* 27M x 1.25 */
	HDMI_ANALOG_CFG_27,					/* 27M */

	HDMI_ANALOG_CFG_297_X_1P5_594,		/* 297M x 1.5, clk os 594M */
	HDMI_ANALOG_CFG_297_X_1P25_594, 	/* 297M x 1.25, clk os 594M */
	HDMI_ANALOG_CFG_297_594,			/* 297M, clk os 594M */

	HDMI_ANALOG_CFG_594,				/* 594M */

	HDMI_ANALOG_CFG_297_X_1P5,			/* 297M x 1.5 */
	HDMI_ANALOG_CFG_297_X_1P25, 		/* 297M x 1.25 */
	HDMI_ANALOG_CFG_297,				/* 297M */

	HDMI_ANALOG_CFG_54_X_1P5,			/* 54M x 1.5 */
	HDMI_ANALOG_CFG_54_X_1P25,			/* 54M x 1.25 */
	HDMI_ANALOG_CFG_54,					/* 54M */

	HDMI_ANALOG_CFG_108,				/* 108M */

	HDMI_ANALOG_CFG_MAX
};

/* Enums for ADC analog configurations */
enum MT_ADC_ANALOG_CFG_E
{
	ADC_ANALOG_CFG_SDEMOD_960             = 0,		/* S-demod 960M */
	ADC_ANALOG_CFG_CDEMOD_144             = 0,		/* C-demod 144M */
	ADC_ANALOG_CFG_SDEMOD_960_CDEMOD_144  = 0,		/* S-demod 960M + C-demod 144M */
	ADC_ANALOG_CFG_SDEMOD_1350_CDEMOD_144 = 1,		/* S-demod 1350M + C-demod 144M */
	ADC_ANALOG_CFG_SDEMOD_1350            = 2,		/* S-demod 1350M */
	ADC_ANALOG_CFG_SDEMOD_1080 	          = 2,		/* S-demod 1080M */
	ADC_ANALOG_CFG_J83BDEMOD_270          = 3,		/* J83.B-demod 270M */

	ADC_ANALOG_CFG_MAX
};

enum MT_HDMI_ANALOG_CFG_E TMDS_TO_IDX(u32 tmds_clk, u32 os_clk);

/**
 * @brief HDMI analog configuration
 *
 * Caution:
 *   Called in ISR!
 *
 * @param[in] id HDMI device ID, default: 0
 * @param[in] cfg_idx configuration index, see: enum MT_HDMI_ANALOG_CFG_E
 *
 * @return
 *   0: success
 *  !0: failure
 */
int mt_hdmi_analog_config(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx);

/**
 * @brief HDMI analog post configuration
 *
 * Caution:
 *   Called in ISR!
 *
 * @param[in] id HDMI device ID, default: 0
 * @param[in] cfg_idx configuration index, see: enum MT_HDMI_ANALOG_CFG_E
 *
 * @return
 *   0: success
 *  !0: failure
 */
int mt_hdmi_analog_config_post(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx);

/**
 * @brief HDMI digital prefix configuration
 *
 * Caution:
 *   Called in ISR!
 *
 * @param[in] id HDMI device ID, default: 0
 * @param[in] cfg_idx configuration index, see: enum MT_HDMI_ANALOG_CFG_E
 *
 * @return
 *   0: success
 *  !0: failure
 */
int mt_hdmi_digit_config_prefix(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx);

/**
 * @brief HDMI digital post configuration
 *
 * Caution:
 *   Called in ISR!
 *
 * @param[in] id HDMI device ID, default: 0
 * @param[in] cfg_idx configuration index, see: enum MT_HDMI_ANALOG_CFG_E
 *
 * @return
 *   0: success
 *  !0: failure
 */
int mt_hdmi_digit_config_post(u32 id, enum MT_HDMI_ANALOG_CFG_E cfg_idx);

/**
 * @brief ADC analog configuration
 *
 * @param[in] idx configuration index, see: enum MT_ADC_ANALOG_CFG_E
 *
 * @return
 *   0: success
 *  !0: failure
 */
int mt_adc_analog_config(enum MT_ADC_ANALOG_CFG_E idx);

/*****************************************************************************/

/**
 * @brief hdmi analog and clock configuration
 */
typedef struct mt_hdmi_ana_clk_cfg {

	u32 tmds_clk;			/** TMDS clock */
	u32 os_clk;				/** Venc OS clock */

	int ssc_onoff;			/** SSC on/off */
	u32 ssc_param;			/** SSC parameter */

	int pd_pi;				/** Power down PI */

	int gate_vclk;			/** Gate Video clock */

} mt_hdmi_ana_clk_cfg_t;

/*
 * Porting HDMI private function
 * in mta_hdmi_priv.c
 *
 * @param[in] cfg HDMI analog and clock configuration
 *
 */
void mta_hdmi_clk_cfg_v3(mt_hdmi_ana_clk_cfg_t *cfg);

#endif

