/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2020, Montage-LZ Technology Co., Ltd.
 *
 * File Name      : mt_drv_pinctrl.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/7/4
 * Description    : Montage LZ Pinctrl driver api definition.
 * History        :
 * 1.Date         : 2022/7/4
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_DRV_PINCTRL__
#define __INC_MT_DRV_PINCTRL__

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/seq_file.h>

/*
 * Symphony6 PINMUX Index
 */
typedef enum
{
	INDEX_SW_PIN_CTRL000 = 0,
	INDEX_SW_PIN_CTRL001,
	INDEX_SW_PIN_CTRL002,
	INDEX_SW_PIN_CTRL003,
	INDEX_SW_PIN_CTRL004,
	INDEX_SW_PIN_CTRL005,
	INDEX_SW_PIN_CTRL006,
	INDEX_SW_PIN_CTRL007,
	INDEX_SW_PIN_CTRL008,
	INDEX_SW_PIN_CTRL009,
	INDEX_SW_PIN_CTRL010,
	INDEX_SW_PIN_CTRL011,
	INDEX_SW_PIN_CTRL012,
	INDEX_SW_PIN_CTRL013,
	INDEX_SW_PIN_CTRL014,
	INDEX_SW_PIN_CTRL015,
	INDEX_SW_PIN_CTRL016,
	INDEX_SW_PIN_CTRL017,
	INDEX_SW_PIN_CTRL018,
	INDEX_SW_PIN_CTRL019,
	INDEX_SW_PIN_CTRL020,
	INDEX_SW_PIN_CTRL021,
	INDEX_SW_PIN_CTRL022,
	INDEX_SW_PIN_CTRL023,
	INDEX_SW_PIN_CTRL024,
	INDEX_SW_PIN_CTRL025,
	INDEX_SW_PIN_CTRL026,
	INDEX_SW_PIN_CTRL027,
	INDEX_SW_PIN_CTRL028,
	INDEX_SW_PIN_CTRL029,
	INDEX_SW_PIN_CTRL030,
	INDEX_SW_PIN_CTRL031_A,
	INDEX_SW_PIN_CTRL032_A,
	INDEX_SW_PIN_CTRL033_A,
	INDEX_SW_PIN_CTRL034_A,
	INDEX_SW_PIN_CTRL035,
	INDEX_SW_PIN_CTRL036,
	INDEX_SW_PIN_CTRL037,
	INDEX_SW_PIN_CTRL038,
	INDEX_SW_PIN_CTRL039,
	INDEX_SW_PIN_CTRL040,
	INDEX_SW_PIN_CTRL041,
	INDEX_SW_PIN_CTRL042,
	INDEX_SW_PIN_CTRL043,
	INDEX_SW_PIN_CTRL044,
	INDEX_SW_PIN_CTRL045,
	INDEX_SW_PIN_CTRL046,
	INDEX_SW_PIN_CTRL047,
	INDEX_SW_PIN_CTRL048,
	INDEX_SW_PIN_CTRL049,
	INDEX_SW_PIN_CTRL050,
	INDEX_SW_PIN_CTRL051,
	INDEX_SW_PIN_CTRL052,
	INDEX_SW_PIN_CTRL053,
	INDEX_SW_PIN_CTRL054,
	INDEX_SW_PIN_CTRL055,
	INDEX_SW_PIN_CTRL056,
	INDEX_SW_PIN_CTRL057,
	INDEX_SW_PIN_CTRL058,
	INDEX_SW_PIN_CTRL059,
	INDEX_SW_PIN_CTRL060,
	INDEX_SW_PIN_CTRL061,
	INDEX_SW_PIN_CTRL062,
	INDEX_SW_PIN_CTRL063,
	INDEX_SW_PIN_CTRL064,
	INDEX_SW_PIN_CTRL065,
	INDEX_SW_PIN_CTRL066,
	INDEX_SW_PIN_CTRL067,
	INDEX_SW_PIN_CTRL068,
	INDEX_SW_PIN_CTRL069,
	INDEX_SW_PIN_CTRL070,
	INDEX_SW_PIN_CTRL071,
	INDEX_SW_PIN_CTRL072,
	INDEX_SW_PIN_CTRL073,
	INDEX_SW_PIN_CTRL074,
	INDEX_SW_PIN_CTRL075,
	INDEX_SW_PIN_CTRL076,
	INDEX_SW_PIN_CTRL077,
	INDEX_SW_PIN_CTRL078,
	INDEX_SW_PIN_CTRL079,
	INDEX_SW_PIN_CTRL080,
	INDEX_SW_PIN_CTRL081,
	INDEX_SW_PIN_CTRL082,
	INDEX_SW_PIN_CTRL083,
	INDEX_SW_PIN_CTRL084,
	INDEX_SW_PIN_CTRL085,
	INDEX_SW_PIN_CTRL086,
	INDEX_SW_PIN_CTRL087,
	INDEX_SW_PIN_CTRL088,
	INDEX_SW_PIN_CTRL089,
	INDEX_SW_PIN_CTRL090,
	INDEX_SW_PIN_CTRL091,
	INDEX_SW_PIN_CTRL092,
	INDEX_SW_PIN_CTRL093,
	INDEX_SW_PIN_CTRL094,
	INDEX_SW_PIN_CTRL095,
	INDEX_SW_PIN_CTRL096,
	INDEX_SW_PIN_CTRL097,
	INDEX_SW_PIN_CTRL098,
	INDEX_SW_PIN_CTRL099,

	INDEX_SW_PIN_CTRL100,
	INDEX_SW_PIN_CTRL101,
	INDEX_SW_PIN_CTRL102,
	INDEX_SW_PIN_CTRL103,
	INDEX_SW_PIN_CTRL104,
	INDEX_SW_PIN_CTRL105,
	INDEX_SW_PIN_CTRL106,
	INDEX_SW_PIN_CTRL107,
	INDEX_SW_PIN_CTRL108,
	INDEX_SW_PIN_CTRL109,
	INDEX_SW_PIN_CTRL110,
	INDEX_SW_PIN_CTRL111,
	INDEX_SW_PIN_CTRL112,
	INDEX_SW_PIN_CTRL113,
	INDEX_SW_PIN_CTRL114,
	INDEX_SW_PIN_CTRL115,
	INDEX_SW_PIN_CTRL116,
	INDEX_SW_PIN_CTRL117,
	INDEX_SW_PIN_CTRL118,
	INDEX_SW_PIN_CTRL119,
	INDEX_SW_PIN_CTRL120,
	INDEX_SW_PIN_CTRL121,
	INDEX_SW_PIN_CTRL122,
	INDEX_SW_PIN_CTRL123,

	INDEX_SW_PIN_CTRL_IN0,	/* 124 */
	INDEX_SW_PIN_CTRL_IN1,
	INDEX_SW_PIN_CTRL_IN2,
	INDEX_SW_PIN_CTRL_IN3,
	INDEX_SW_PIN_CTRL_IN4,
	INDEX_SW_PIN_CTRL_IN5,
	INDEX_SW_PIN_CTRL_SEL,
	INDEX_DA_PIN_SEL = INDEX_SW_PIN_CTRL_SEL,

	INDEX_SW_PIN_CTRL_IN_MAX,

	INDEX_AO_PIN_CTRL0 = 200,   /* AO from 200 */
	INDEX_AO_PIN_CTRL1,
	INDEX_AO_PIN_CTRL2,
	INDEX_AO_PIN_CTRL3,
	INDEX_AO_PIN_CTRL4,
	INDEX_AO_PIN_CTRL5,
	INDEX_AO_PIN_CTRL6,
	INDEX_AO_PIN_CTRL7,
	INDEX_AO_PIN_CTRL8,
	INDEX_AO_PIN_CTRL9,
	INDEX_AO_PIN_CTRL10_A,
	INDEX_AO_PIN_CTRL11_A,
	INDEX_AO_PIN_CTRL12_A,
	INDEX_AO_PIN_CTRL13_A,

	INDEX_PINMUX_MAX

} PINMUX_INDEX_E;

/**
 * @brief get PIN IO CTRL value
 *
 * @param[in] index PIN index
 *
 * @retval IO CTRL value
 */
unsigned int mt_pinctrl_get_io_ctrl(PINMUX_INDEX_E index);

/**
 * @brief set PIN IO CTRL value
 *
 * @param[in] index PIN index
 * @param[in] val IO CTRL value
 */
void mt_pinctrl_set_io_ctrl(PINMUX_INDEX_E index, unsigned int val);

/**
 * @brief get PIN FUNCTION select(mux) value
 *
 * @param[in] index PIN index
 *
 * @retval PIN FUNCTION select(mux) value
 */
unsigned int mt_pinctrl_get_function(PINMUX_INDEX_E index);

/**
 * @brief set PIN FUNCTION select(mux) value
 *
 * @param[in] index PIN index
 * @param[in] val PIN FUNCTION select(mux) value
 */
void mt_pinctrl_set_function(PINMUX_INDEX_E index, unsigned int val);

#endif

