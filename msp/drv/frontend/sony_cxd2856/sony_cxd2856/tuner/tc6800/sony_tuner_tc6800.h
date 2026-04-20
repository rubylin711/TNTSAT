/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**

 @file    sony_tuner_r836.h

          This file provides the DVB port of the TC6800 tuner driver.

          This driver wraps around the TC6800 driver provided by
          sony_tc6800.h by using an instance of the TC6800 (sony_tc6800_t)
          driver in the ::sony_tuner_terr_cable_t::user pointer.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_TC6800_H_
#define SONY_TUNER_TC6800_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "../sony_tuner_terr_cable.h"
#include "mt_fe_tn_tc6800.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
extern const char* sony_tuner_tc6800_version;  /**< TC6800 driver version */

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_TUNER_TC6800_OFFSET_CUTOFF_HZ         50000   /**< Maximum carrier offset frequency before requiring a retune */

typedef enum {
    SONY_TC6800_STATE_UNKNOWN,    /**< TC6800 state is Unknown */
    SONY_TC6800_STATE_SLEEP,      /**< TC6800 state is Sleep */
    SONY_TC6800_STATE_ACTIVE      /**< TC6800 state is Active */
} sony_tc6800_state_t;

typedef struct
{
    uint8_t                   i2cAddress;   /**< I2C slave address of the TC6800 tuner (8-bit form - 8'bxxxxxxx0) */
    sony_i2c_t*               pI2c;         /**< I2C API instance. */
    uint32_t                  flags;        /**< ORed value of SONY_TC6800_CONFIG_XXXX */

    sony_tc6800_state_t       state;        /**< The driver operating state. */
    uint32_t                  frequencykHz; /**< Currently RF frequency(kHz) tuned. */
    sony_dtv_system_t         tvSystem;     /**< Current broadcasting system tuned. */
	sony_dtv_bandwidth_t      tvBandwidth;  /**< Current broadcasting band-width. */

    int                       dev_id;
    void*                     user;         /**< User defined data. */
} sony_tc6800_t, *sony_tc6800_device_handle;

extern sony_tc6800_t *g_pTC6800Tuner;
//#define CUR_FE_ID   (g_pTC6800Tuner->dev_id)

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the TC6800 tuner driver

 @param pTuner The tuner driver instance to create. Memory
        must have been allocated for this instance before
        creation.
 @param i2cAddress The I2C address of the TC6800 device.
        Typically 0x34.
 @param pI2c The I2C driver that the tuner driver will use for
        communication.
 @param configFlags See "#define" ::SONY_TC6800_CONFIG_SLEEP_DISABLEXTAL,
        ::SONY_TC6800_CONFIG_LOOPFILTER_INTERNAL,
        ::SONY_TC6800_CONFIG_LOOPTHRU_ENABLE,
        ::SONY_TC6800_CONFIG_RFIN_MATCHING_ENABLE,
        ::SONY_TC6800_CONFIG_IF2_ATV,
        ::SONY_TC6800_CONFIG_AGC2_ATV,
        ::SONY_TC6800_CONFIG_IF2_DTV,
        ::SONY_TC6800_CONFIG_AGC2_DTV,
        ::SONY_TC6800_CONFIG_IFAGCSEL_ALL1,
        ::SONY_TC6800_CONFIG_IFAGCSEL_ALL2,
        ::SONY_TC6800_CONFIG_IFAGCSEL_A1D2,
        ::SONY_TC6800_CONFIG_IFAGCSEL_D1A2,
        ::SONY_TC6800_CONFIG_REFOUT_500mVpp
        ::SONY_TC6800_CONFIG_REFOUT_400mVpp
        ::SONY_TC6800_CONFIG_REFOUT_600mVpp
        ::SONY_TC6800_CONFIG_REFOUT_800mVpp defined in
        \link sony_ascot3.h \endlink
 @param pTC6800Tuner The TC6800 tuner driver pointer to use.
        Memory must have been allocated for the TC6800 driver structure.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_tuner_tc6800_Create(sony_tuner_terr_cable_t * pTuner,
                                      uint8_t i2cAddress,
                                      sony_i2c_t * pI2c,
                                      uint32_t configFlags,
                                      int dev_id,
                                      sony_tc6800_t * pTC6800Tuner);


sony_result_t mt_fe_tn_init_tc6800_sony(sony_tc6800_device_handle handle);
sony_result_t mt_fe_tn_set_freq_tc6800_sony(sony_tc6800_device_handle handle, uint32_t Freq_KHz);
sony_result_t mt_fe_tn_get_strength_tc6800_sony(sony_tc6800_device_handle handle, int8_t *p_strength);
sony_result_t mt_fe_tn_sleep_tc6800_sony(sony_tc6800_device_handle handle);
sony_result_t mt_fe_tn_wake_up_tc6800_sony(sony_tc6800_device_handle handle);
sony_result_t mt_fe_tn_xtal_tc6800_sony(sony_tc6800_device_handle handle, uint32_t xtal_khz);
sony_result_t mt_fe_tn_enable_internal_FEF_tc6800_sony(sony_tc6800_device_handle handle);
sony_result_t mt_fe_tn_disable_internal_FEF_tc6800_sony(sony_tc6800_device_handle handle);


#endif /* SONY_TUNER_TC6800_H_ */
