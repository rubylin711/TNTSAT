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

          This file provides the DVB port of the R836 tuner driver.

          This driver wraps around the R836 driver provided by
          sony_r836.h by using an instance of the R836 (sony_r836_t)
          driver in the ::sony_tuner_terr_cable_t::user pointer.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_R850_H_
#define SONY_TUNER_R850_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "../sony_tuner_terr_cable.h"
#include "R850.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
extern const char* sony_tuner_r850_version;  /**< R836 driver version */

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_TUNER_R850_OFFSET_CUTOFF_HZ         50000   /**< Maximum carrier offset frequency before requiring a retune */

typedef enum {
    SONY_R850_STATE_UNKNOWN,    /**< R836 state is Unknown */
    SONY_R850_STATE_SLEEP,      /**< R836 state is Sleep */
    SONY_R850_STATE_ACTIVE      /**< R836 state is Active */
} sony_r850_state_t;

typedef struct {
    uint8_t                   i2cAddress;   /**< I2C slave address of the R836 tuner (8-bit form - 8'bxxxxxxx0) */
    sony_i2c_t*               pI2c;         /**< I2C API instance. */
    uint32_t                  flags;        /**< ORed value of SONY_R836_CONFIG_XXXX */

    sony_r850_state_t         state;        /**< The driver operating state. */
    uint32_t                  frequencykHz; /**< Currently RF frequency(kHz) tuned. */
    R850_Standard_Type        tvSystem;     /**< Current broadcasting system tuned. */

    int                       dev_id;
    void*                     user;         /**< User defined data. */
} sony_r850_t;

extern sony_r850_t *g_pR850Tuner;
//#define CUR_FE_ID   (g_pR850Tuner->dev_id)  //clean warning

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the R836 tuner driver

 @param pTuner The tuner driver instance to create. Memory
        must have been allocated for this instance before
        creation.
 @param i2cAddress The I2C address of the R836 device.
        Typically 0x34.
 @param pI2c The I2C driver that the tuner driver will use for
        communication.
 @param configFlags See "#define" ::SONY_R836_CONFIG_SLEEP_DISABLEXTAL,
        ::SONY_R836_CONFIG_LOOPFILTER_INTERNAL,
        ::SONY_R836_CONFIG_LOOPTHRU_ENABLE,
        ::SONY_R836_CONFIG_RFIN_MATCHING_ENABLE,
        ::SONY_R836_CONFIG_IF2_ATV,
        ::SONY_R836_CONFIG_AGC2_ATV,
        ::SONY_R836_CONFIG_IF2_DTV,
        ::SONY_R836_CONFIG_AGC2_DTV,
        ::SONY_R836_CONFIG_IFAGCSEL_ALL1,
        ::SONY_R836_CONFIG_IFAGCSEL_ALL2,
        ::SONY_R836_CONFIG_IFAGCSEL_A1D2,
        ::SONY_R836_CONFIG_IFAGCSEL_D1A2,
        ::SONY_R836_CONFIG_REFOUT_500mVpp
        ::SONY_R836_CONFIG_REFOUT_400mVpp
        ::SONY_R836_CONFIG_REFOUT_600mVpp
        ::SONY_R836_CONFIG_REFOUT_800mVpp defined in
        \link sony_ascot3.h \endlink
 @param pR836Tuner The R836 tuner driver pointer to use.
        Memory must have been allocated for the R836 driver structure.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_tuner_r850_Create(sony_tuner_terr_cable_t * pTuner,
                                      uint8_t i2cAddress,
                                      sony_i2c_t * pI2c,
                                      uint32_t configFlags,
                                      int dev_id,
                                      sony_r850_t * pR850Tuner);


#endif /* SONY_TUNER_R836_H_ */
