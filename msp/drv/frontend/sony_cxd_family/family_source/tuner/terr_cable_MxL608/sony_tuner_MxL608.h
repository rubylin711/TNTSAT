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

 @file    sony_tuner_MxL608.h

          This file provides the DVB port of the Sony ASCOT3 tuner driver.

          This driver wraps around the Ascot3 driver provided by
          sony_ascot3.h by using an instance of the Ascot3 (sony_ascot3_t)
          driver in the ::sony_tuner_terr_cable_t::user pointer.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_MXL608_H_
#define SONY_TUNER_MXL608_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "../sony_tuner.h"

#include "MxL608_TunerApi.h"
#include "MxL608_TunerCfg.h"
#include "MxL_Debug.h"


typedef struct sony_MxL608_t
{
    MXL608_XTAL_FREQ_E        xtalFreq;     /**< Xtal frequency for MxL608. */
    uint8_t                   i2cAddress;   /**< I2C slave address of the MxL608 tuner (8-bit form - 8'bxxxxxxx0) */
    sony_i2c_t*               pI2c;         /**< I2C API instance. */
    uint32_t                  flags;        /**< ORed value of SONY_MxL608_CONFIG_XXXX */

    /* For saving current setting */
    MXL_STATUS                state;        /**< The driver operating state. */
    uint32_t                  frequencykHz; /**< Currently RF frequency(kHz) tuned. */
    MXL608_SIGNAL_MODE_E      tvSystem;     /**< Current broadcasting system tuned. */

    MXL608_VER_INFO_T         versionInfo;  /**< Auto detected chip ID at initialization */

    /* Following Xtal related parameters can be changed if optimization is necessary. */
    uint8_t                   xosc_sel;     /**< Driver current setting for crystal oscillator. (Addr:0x82 Bit[4:0]) */
    uint8_t                   xosc_cap_set; /**< Driver current setting for crystal oscillator. (Addr:0x83 Bit[6:0]) */

    void*                     user;         /**< User defined data. */
} sony_MxL608_t; 

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
extern const char* sony_tuner_mxl608_version;  /**< MxL608 driver version */

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the ASCOT3 tuner driver

 @param pTuner The tuner driver instance to create. Memory
        must have been allocated for this instance before
        creation.
 @param xtalFreq The crystal frequency of the tuner.
 @param i2cAddress The I2C address of the ASCOT3 device.
        Typically 0xC0.
 @param pI2c The I2C driver that the tuner driver will use for
        communication.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_tuner_MxL608_Create (sony_tuner_t * pTuner,
                                        MXL608_XTAL_FREQ_E xtalFreq,
                                        uint8_t i2cAddress,
                                        sony_i2c_t * pI2c,
                                        uint32_t configFlags,
                                        sony_MxL608_t * pMxL608Tuner);


#endif /* SONY_TUNER_MXL608_H_ */

