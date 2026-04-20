/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_mod_reset.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT clock module reset header file.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_MODULE_RESET_H__
#define __INC_MT_MODULE_RESET_H__

/* 100us for normal state soft reset, according to CRM datasheet */
#define CFG_MOD_SFT_RST_HOLD_US			(100)

/* 3ms for exception state soft reset, according to CRM datasheet */
#define CFG_MOD_EXP_SFT_RST_HOLD_US		(3000)

/*
 * 10 cycle after soft-reset, module delay time for cpu accessing
 * XTAL: 24MHz
 * 10 cycle = 1/24 * 10 = 0.5us
 */
#define CFG_MOD_SFT_RST_WAIT_US			(1)

/*
 * Reset Module(Module Soft Reset)
 */
typedef char mt_mod_t;

/*
 * @brief Simple Reset Module definition
 */
struct mt_mod_rst_simple
{
	const char *name;

	union
	{
		mt_io_objlk_t ahb;
		mt_io_objlk_t apb;
		mt_io_objlk_t axi;
		mt_io_objlk_t core;
		/* phy/reg/ibus/... */
	};

	int active_low;

	unsigned int delay_us;	/* hold time after reset, before release, in unit of us.
	                         * '0' means use default value.
	                         */
};

/*
 * @brief Composite Reset Module definition
 */
struct mt_mod_rst_composite
{
	const char *name;

	/*
	 * for digital module, might has ahb/apb/axi.
	 * for analog & pll module, has no ahb/apb/axi.
	 */
	union
	{
		struct mt_mod_rst_simple *ahb;
		struct mt_mod_rst_simple *apb;
	};
	struct mt_mod_rst_simple *axi;
	struct mt_mod_rst_simple *core;

	union
	{
		struct mt_mod_rst_simple *reg;
		struct mt_mod_rst_simple *phy;
		struct mt_mod_rst_simple *ibus;
		struct mt_mod_rst_simple *ext1;
	};

	struct mt_mod_rst_simple *ext2;
};

/*
 * a: address
 * s: shift
 * aslk: address of slock
 * alk: address of lock
 * slk: shift of slock & lock
 */
#define MT_INIT_RST(a, s, aslk, alk, slk) \
					MT_INIT_OBJLK(a, s, 1, aslk, slk, 1, alk, slk, 1, NULL)

/* init digital module reset fields */
#define INIT_DIG_RST_VAL 			\
			.active_low = 1, \
			.delay_us = CFG_MOD_SFT_RST_HOLD_US

/* init analog/pll module reset fields */
#define INIT_ANA_RST_VAL 			\
			.active_low = 0, \
			.delay_us = CFG_MT_ANA_RESET_UDELAY_MIN

extern struct mt_mod_rst_simple mt_rst_modules[];
extern u32 mt_rst_modules_count;

extern struct mt_mod_rst_composite mt_com_rst_modules[];
extern u32 mt_com_rst_modules_count;

/**
 * @brief get module reset status
 *
 * @param[in] mod module name
 * @param[out] status module reset status, >0: reset, 0: release, <0: Not available
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_mod_get_reset_status(mt_mod_t *mod, int *status);

/**
 * @brief reset module(soft reset)
 *
 *   mt_mod_reset = mt_mod_rst_reset + delay + mt_mod_rst_release
 *
 * @param[in] mod module name
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_mod_reset(mt_mod_t *mod);

/**
 * @brief reset module(soft reset) when module exception(abnormal)
 *
 *   mt_mod_reset_exp = mt_mod_rst_reset + delay(long) + mt_mod_rst_release
 *
 * @param[in] mod module name
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_mod_reset_exp(mt_mod_t *mod);

/**
 * @brief reset module(soft reset), but not release reset
 *
 * @param[in] mod module name
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_mod_rst_reset(mt_mod_t *mod);

/**
 * @brief release reset module
 *
 * @param[in] mod module name
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_mod_rst_release(mt_mod_t *mod);

#endif

