/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_analog.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/05/14
 * Description    : Montage Analog driver.
 * History        :
 * 1.Date         : 2021/05/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_ANALOG_H__
#define __INC_MT_ANALOG_H__

/* Common fields of MT Analog */
#define MT_ANA_COMMON_FIELDS			\
				mt_u32 reg_addr; 		\
				mt_u32 shift;			\
				mt_u32 width;			\
				mt_u32 reg_slock;		\
				mt_u32 reg_lock;		\
				mt_u32 bit_lock;

/**
 * Analog module Gate
 */
struct mt_ana_gate
{
	MT_ANA_COMMON_FIELDS

	/**
	 * reverse
	 *    0: ON
	 *    1: OFF
	 */
	mt_u32 reverse;
};

/**
 * Analog module Mux
 */
struct mt_ana_mux
{
	MT_ANA_COMMON_FIELDS
};

/**
 * Analog module Reset
 */
struct mt_ana_reset_st
{
	MT_ANA_COMMON_FIELDS
};

/**
 * Analog module definition
 */
struct mt_analog
{
	const char *name;
	mt_u32 flags;		/* FLAG_ANA_GATE, ... */
	mt_u32 ref_count;	/* child referenced count */

	struct mt_ana_gate 		gate;
	struct mt_ana_mux  		mux;
	struct mt_ana_reset_st 	reset;
};

//---------------------------------------------------------------------------//

/**
 * Analog batch configuration
 */
void mt_ana_batch_config(void);

/**
 * @brief get analog module handle
 *
 * @param[in] dev not used, reserved for future use.
 * @param[in] id analog module name, such as "usb0".
 */
struct mt_analog *mt_ana_get(void *dev, const char *id);

mt_s32 mt_ana_enable(struct mt_analog *ana);
void mt_ana_disable(struct mt_analog *ana);
mt_u32 mt_ana_get_enable(struct mt_analog *ana);

mt_u32 mt_ana_get_mux(struct mt_analog *ana);
mt_s32 mt_ana_set_mux(struct mt_analog *ana, mt_u32 value);

mt_s32 mt_ana_reset(struct mt_analog *ana);

#endif

