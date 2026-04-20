/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */
#ifndef __INC_MT_ANALOG_H__
#define __INC_MT_ANALOG_H__

#if defined(__UBOOT__)
#include "mt_analog_attr.h"
#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include "analog/mt_analog_attr.h"
#else
/* RTOS */
#include <sys_types.h>
#include <sys_define.h>

#include "analog/mt_analog_attr.h"
#endif

#if !defined(__UBOOT__)

/* check null pointer */
#ifndef CHECK_NULL_PTR
#if defined(__KERNEL__)
#define CHECK_NULL_PTR(p)		do{ if (!(p)) { printk("[E]%s: null pointer!\n", __FUNCTION__); return (-1); } } while(0)
#else
/* RTOS */
#define CHECK_NULL_PTR(p)		do{ if (!(p)) { OS_PRINTF("[E]%s: null pointer!\n", __FUNCTION__); return (-1); } } while(0)
#endif
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* analog module reset delay */
#define CFG_MT_ANA_RESET_UDELAY_MIN		20

/* tsensor reset need delay 100us, others delay 10us! */
#define CFG_MT_ANA_RESET_UDELAY_MAX		200

#if !defined(__UBOOT__)

/*
 * PLL
 */
typedef char mt_pll_t;

#define MT_ARMPLL			"armpll"
//#define MT_CPUPLL			"cpupll"		/* "cpupll" is a clerical error, ADCPLL for Demod infact! */
#define MT_ADCPLL			"adcpll"
#define MT_USBPLL			"usbpll"
#define MT_VHDPLL			"vhdpll"
#define MT_VSDPLL			"vsdpll"
#define MT_EPHYPLL			"ephypll"
#define MT_DDRPLL			"ddrpll"
#define MT_REFPLL			"refpll"
#define MT_AUDIOPLL			"audiopll"
#define MT_USB30PLL			"usb30pll"

#endif

/* Analog type define */
typedef char mt_ana_t;

/* Define Analog modules for enable/disable/reset */
#define MT_ANA_BIAS				"bias_ana"			/* Stat: Y, Reset: N */
#define MT_ANA_USB0				"usb0_ana"			/* Stat: Y, Reset: Y */
#define MT_ANA_USB1				"usb1_ana"			/* Stat: Y, Reset: Y */
#define MT_ANA_ICX				"icx_ana"			/* Stat: Y, Reset: N */
#define MT_ANA_VDAC				"vdac_ana"			/* Stat: Y, Reset: Y */
#define MT_ANA_HDMITX			"hdmitx_ana"		/* Stat: Y, Reset: Y, Clk Rate: Y */
#define MT_ANA_ADAC				"adac_ana"			/* Stat: Y, Reset: Y */
#define MT_ANA_CADC				"cadc_ana"			/* Stat: Y, Reset: N, Clk Rate: Y */
#define MT_ANA_SADC				"sadc_ana"			/* Stat: Y, Reset: Y, Clk Rate: Y */
#define MT_ANA_PROCMON			"pmon_ana"			/* process monitor */
													/* Stat: Y, Reset: Y, Clk Rate: Y */
#define MT_ANA_RNG1				"rng1_ana"			/* Stat: Y, Reset: Y */
#define MT_ANA_TSENSOR			"tsensor_ana"		/* temp sensor */
													/* Stat: Y, Reset: Y */
#define MT_ANA_RNG2				"rng2_ana"			/* Stat: Y, Reset: Y */
#define MT_ANA_VDAC_DET			"vdac_det_ana"		/* Stat: Y, Reset: N */
#define MT_ANA_MSTR_REG			"mstr_reg_ana"		/* master regulator */
													/* Stat: Y, Reset: N */
#define MT_ANA_USB3_PHY			"usb3_phy_ana"		/* Stat: Y, Reset: N */
#define MT_ANA_EPHY				"ephy_ana"			/* Stat: Y, Reset: Y */

#define MT_ANA_VSDSSC			"vsdssc_ana"		/* Stat: Y, Reset: Y */
#define MT_ANA_HDMISSC			MT_ANA_VSDSSC

#define MT_ANA_KADC				"key_adc_ana"		/* Stat: Y, Reset: Y */
#define MT_ANA_KADC2			"key_adc2_ana"		/* Stat: Y, Reset: Y */

/* Internal Used */
#define VSD_CLK_ANA				"vsd_clk_ana"		/* Stat: Y */
#define VHD_CLK_ANA				"vhd_clk_ana"		/* Stat: Y */

#define MT_ANA_VENC_OS			"venc_osclk_ana"	/* Clk Rate: Y */

/* ADC */
#define DRV0_CLK_ANA			"drv0_clk_ana"		/* Stat: Y, Clk Rate: Y */
#define DRV1_CLK_ANA			"drv1_clk_ana"		/* Stat: Y, Clk Rate: Y */

//---------------------------------------------------------------------------//

/*
 * Usage:
 *  1.
 *    mt_analog_acquire_lock
 *    mt_analog_get_attr_no_lock
 *    modify analog attributes...
 *    mt_analog_set_attr_no_lock
 *    mt_analog_release_lock
 *  2.
 *    mt_analog_get_attr_lock
 *    [mt_analog_get_attr_no_lock]
 *    modify analog attributes...
 *    [mt_analog_set_attr_no_lock]
 *    mt_analog_set_attr_unlock
 */

/**
 * @brief Acquire Analog Lock
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_acquire_lock(void);

/**
 * @brief Release Analog Lock
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_release_lock(void);

/**
 * @brief get Analog attribute(lock and unlock)
 *
 * @param[in] index Analog module index
 * @param[out] attr Analog module attribute
 *
 *    index                attr
 *    MT_ANA_INDEX_AO_R0   struct mt_analog_ao_r0_attr*
 *    MT_ANA_INDEX_AO_R1   struct mt_analog_ao_r1_attr*
 *    MT_ANA_INDEX_TOP_R1  struct mt_analog_top_r1_attr*
 *    ...
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_get_attr(enum MT_ANALOG_INDEX_E index, void *attr);

/**
 * @brief get Analog attribute(no lock)
 *
 * @param[in] index Analog module index
 * @param[out] attr Analog module attribute
 *
 *    index                attr
 *    MT_ANA_INDEX_AO_R0   struct mt_analog_ao_r0_attr*
 *    MT_ANA_INDEX_AO_R1   struct mt_analog_ao_r1_attr*
 *    MT_ANA_INDEX_TOP_R1  struct mt_analog_top_r1_attr*
 *    ...
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_get_attr_no_lock(enum MT_ANALOG_INDEX_E index, void *attr);

/**
 * @brief get Analog attribute(with lock, but no unlock)
 *
 * pair with mt_analog_set_attr_unlock!
 *
 * @param[in] index Analog module index
 * @param[out] attr Analog module attribute
 *
 *    index                attr
 *    MT_ANA_INDEX_AO_R0   struct mt_analog_ao_r0_attr*
 *    MT_ANA_INDEX_AO_R1   struct mt_analog_ao_r1_attr*
 *    MT_ANA_INDEX_TOP_R1  struct mt_analog_top_r1_attr*
 *    ...
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_get_attr_lock(enum MT_ANALOG_INDEX_E index, void *attr);

/**
 * @brief set Analog attribute(no lock)
 *
 * @param[in] index Analog module index
 * @param[in] attr Analog module attribute
 *
 *    index                attr
 *    MT_ANA_INDEX_AO_R0   struct mt_analog_ao_r0_attr*
 *    MT_ANA_INDEX_AO_R1   struct mt_analog_ao_r1_attr*
 *    MT_ANA_INDEX_TOP_R1  struct mt_analog_top_r1_attr*
 *    ...
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_set_attr_no_lock(enum MT_ANALOG_INDEX_E index, void *attr);

/**
 * @brief set Analog attribute(with unlock, but no lock)
 *
 * pair with mt_analog_get_attr_lock!
 *
 * @param[in] index Analog module index
 * @param[in] attr Analog module attribute
 *
 *    index                attr
 *    MT_ANA_INDEX_AO_R0   struct mt_analog_ao_r0_attr*
 *    MT_ANA_INDEX_AO_R1   struct mt_analog_ao_r1_attr*
 *    MT_ANA_INDEX_TOP_R1  struct mt_analog_top_r1_attr*
 *    ...
 *
 * @return
 *    0: success
 *    !0: failure
 *
 */
int mt_analog_set_attr_unlock(enum MT_ANALOG_INDEX_E index, void *attr);

/*
 * Idx: enum MT_ANALOG_INDEX_E
 * Type: attribute structure
 * Field: field of attribute structure
 * Val: value of field
 *
 * e.g.
 *    MT_ANALOG_UP_ATTR(MT_ANA_INDEX_AO_R0, struct mt_analog_ao_r0_attr, pd_usb0, 1);
 */
#define MT_ANALOG_UP_ATTR(Idx, Type, Field, Val)		\
			do { 										\
				Type attr;								\
				mt_analog_get_attr_lock(Idx, (void*)&attr);		\
				attr.Field = Val;						\
				mt_analog_set_attr_unlock(Idx, (void*)&attr);	\
			} while(0)

/**
 * @brief get Analog/PLL module status
 *
 * @param[in] ana Analog/PLL module name
 * @param[out] status Analog/PLL module status,
 *                     0: disable
 *                    >0: enable
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_get_status(mt_ana_t *ana, u32 *status);

/**
 * @brief enable Analog/PLL module
 *
 * @param[in] ana Analog/PLL module name
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_enable(mt_ana_t *ana);

/**
 * @brief disable Analog/PLL module
 *
 * @param[in] ana Analog/PLL module name
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_disable(mt_ana_t *ana);

/**
 * @brief get Analog/PLL module reset status
 *
 * @param[in] ana Analog/PLL module name
 * @param[out] status module reset status, >0: reset, 0: release, <0: Not available
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_get_reset_status(mt_ana_t *ana, int *status);

/**
 * @brief reset Analog/PLL module
 *
 * @param[in] ana Analog/PLL module name
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_reset(mt_ana_t *ana);

/**
 * @brief release Analog/PLL module
 *
 * @param[in] ana Analog/PLL module name
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_release(mt_ana_t *ana);

/**
 * @brief get Analog clock rate
 *
 * @param[in] ana Analog module name
 * @param[out] rate clock frequency, in unit of HZ.
 *            e.g. 720MHz, rate=720000000
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_get_rate(mt_ana_t *ana, unsigned long *rate);

/**
 * @brief set Analog clock rate
 *
 * @param[in] ana Analog module name
 * @param[in] rate clock frequency, in unit of HZ.
 *            e.g. 720MHz, rate=720000000
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_analog_set_rate(mt_ana_t *ana, unsigned long rate);

/**
 * @brief dump Analog module status
 */
#if defined(__UBOOT__)
void mt_analog_dump_status(void);
#elif defined(__KERNEL__)
void mt_analog_dump_status(struct seq_file *s);
#else
/* RTOS */
void mt_analog_dump_status(int (*misc_printf)(const char *fmt, ...));
#endif

/**
 * @brief dump Analog module state
 */
#if defined(__UBOOT__)
void mt_analog_dump_state(void);
#elif defined(__KERNEL__)
void mt_analog_dump_state(struct seq_file *s);
#else
/* RTOS */
void mt_analog_dump_state(void);
#endif

/**
 * @brief Initialize Temperature Sensor
 *
 * @return
 *    0: success
 *   !0: failure
 */
int mt_tsensor_init(void);

/**
 * @brief Deinit Temperature Sensor
 *
 * @return
 *    0: success
 *   !0: failure
 */
int mt_tsensor_deinit(void);

/**
 * @brief get temperature
 *
 * param[out] temp_int integer of temperature
 * param[out] temp_dec two decimal of temperature
 */
void mt_analog_get_temperature(int *temp_int, int *temp_dec);
void mt_analog_get_temperature_async(int *temp_int, int *temp_dec);

#if defined(__KERNEL__) && !defined(__UBOOT__)

/**
 * @brief backup analog registers when suspend
 *
 * @return none
 */
void mt_analog_suspend(void);

/**
 * @brief restore analog registers when resume
 *
 * @return none
 */
void mt_analog_resume(void);

//---------------------------------------------------------------------------//

/**
 * @brief Board Analog Config
 *
 * @return
 *   0: success
 *  !0: failure
 */
int board_analog_cfg(void);

/**
 * @brief Board ADAC Config
 *    When board cold bootup, reboot, standby bootup, or STR resume,
 *  should call this function.
 *
 * @return
 *   0: success
 *  !0: failure
 */
int board_adac_cfg(void);

/**
 * @brief ADAC Switch On/Off dynamicly
 *
 * @return
 *   0: success
 *  !0: failure
 */
int board_adac_onoff(int on);

/*
 * get temperature function pointer defined in kernel
 * see: mt_analog_get_temperature
 */
extern void (*p_mt_analog_get_temperature)(int *temp_int, int *temp_dec);
extern void (*p_mt_analog_get_temperature_async)(int *temp_int, int *temp_dec);

#endif

#ifdef __cplusplus
}
#endif

#endif	//__INC_MT_ANALOG_H__

