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

          This file provides the DVB port of the R840 tuner driver.

          This driver wraps around the R840 driver provided by
          sony_r840.h by using an instance of the R840 (sony_r840_t)
          driver in the ::sony_tuner_terr_cable_t::user pointer.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_R840_H_
#define SONY_TUNER_R840_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "../sony_tuner.h"
#include "R836.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
extern const char* sony_tuner_r840_version;  /**< R840 driver version */

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_TUNER_R840_OFFSET_CUTOFF_HZ         50000   /**< Maximum carrier offset frequency before requiring a retune */

typedef enum {
    SONY_R840_STATE_UNKNOWN,    /**< R840 state is Unknown */
    SONY_R840_STATE_SLEEP,      /**< R840 state is Sleep */
    SONY_R840_STATE_ACTIVE      /**< R840 state is Active */
} sony_r840_state_t;

typedef struct {
    uint8_t                   i2cAddress;   /**< I2C slave address of the R840 tuner (8-bit form - 8'bxxxxxxx0) */
    sony_i2c_t*               pI2c;         /**< I2C API instance. */
    uint32_t                  flags;        /**< ORed value of SONY_R840_CONFIG_XXXX */

    sony_r840_state_t         state;        /**< The driver operating state. */
    uint32_t                  frequencykHz; /**< Currently RF frequency(kHz) tuned. */
    R840_Standard_Type        tvSystem;     /**< Current broadcasting system tuned. */

    int                       dev_id;
    void*                     user;         /**< User defined data. */
} sony_r840_t;

extern sony_r840_t *g_pR840Tuner;
//#define CUR_FE_ID   (g_pR840Tuner->dev_id) //clean warning

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the R840 tuner driver

 @param pTuner The tuner driver instance to create. Memory
        must have been allocated for this instance before
        creation.
 @param i2cAddress The I2C address of the R840 device.
        Typically 0x34.
 @param pI2c The I2C driver that the tuner driver will use for
        communication.
 @param configFlags See "#define" ::SONY_R840_CONFIG_SLEEP_DISABLEXTAL,
        ::SONY_R840_CONFIG_LOOPFILTER_INTERNAL,
        ::SONY_R840_CONFIG_LOOPTHRU_ENABLE,
        ::SONY_R840_CONFIG_RFIN_MATCHING_ENABLE,
        ::SONY_R840_CONFIG_IF2_ATV,
        ::SONY_R840_CONFIG_AGC2_ATV,
        ::SONY_R840_CONFIG_IF2_DTV,
        ::SONY_R840_CONFIG_AGC2_DTV,
        ::SONY_R840_CONFIG_IFAGCSEL_ALL1,
        ::SONY_R840_CONFIG_IFAGCSEL_ALL2,
        ::SONY_R840_CONFIG_IFAGCSEL_A1D2,
        ::SONY_R840_CONFIG_IFAGCSEL_D1A2,
        ::SONY_R840_CONFIG_REFOUT_500mVpp
        ::SONY_R840_CONFIG_REFOUT_400mVpp
        ::SONY_R840_CONFIG_REFOUT_600mVpp
        ::SONY_R840_CONFIG_REFOUT_800mVpp defined in
        \link sony_ascot3.h \endlink
 @param pR840Tuner The R840 tuner driver pointer to use.
        Memory must have been allocated for the R840 driver structure.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_tuner_r840_Create(sony_tuner_t * pTuner,
                                      uint8_t i2cAddress,
                                      sony_i2c_t * pI2c,
                                      uint32_t configFlags,
                                      int dev_id,
                                      sony_r840_t * pR840Tuner);


#endif /* SONY_TUNER_R840_H_ */
