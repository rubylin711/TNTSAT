/*------------------------------------------------------------------------------
  Copyright 2021 Sony Semiconductor Solutions Corporation

  Last Updated    : 2021/12/13
  Modification ID : 3cc1b1863ae94a345715678192a80bd188f58cc0
------------------------------------------------------------------------------*/
/**

 @file    sony_tuner_freia.h

          This file provides the DVB port of the Sony FREIA tuner driver.

          This driver wraps around the Freia driver provided by
          sony_freia.h by using an instance of the Freia (sony_freia_t)
          driver in the ::sony_tuner_t::user pointer.

          Please note, if the Freia tuner is used in a system without the
          terrestrial or satellite demodulator components then the unused
          structure, pTunerSat or pTunerTerrCable must be declared prior to
          calling tuner create function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_FREIA_H
#define SONY_TUNER_FREIA_H

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "sony_tuner.h"
#include "sony_freia.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
extern const char* sony_tuner_freia_version;              /**< FREIA driver version */

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_TUNER_FREIA_OFFSET_CUTOFF_HZ         50000   /**< Maximum carrier offset frequency before requiring a retune */

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the FREIA tuner driver

 @param pTuner The tuner driver instance to create.
        Memory must have been allocated for this instance before creation.
 @param i2cAddress The I2C address of the FREIA device.
        Typically 0xC0.
 @param pI2c The I2C driver that the tuner driver will use for
        communication.
 @param configFlags See ::SONY_FREIA_CONFIG_EXT_REF,
            ::SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL,
            ::SONY_FREIA_CONFIG_LOOPFILTER_INTERNAL,
            ::SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE,
            ::SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_DISABLE
            ::SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_ENABLE
            ::SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE
            ::SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_ENABLE
            ::SONY_FREIA_CONFIG_POWERSAVE_SAT_NORMAL
            ::SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE
            ::SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE_AGC_FULL
            ::SONY_FREIA_CONFIG_OUTLMT_ATV_1_5Vpp
            ::SONY_FREIA_CONFIG_OUTLMT_ATV_1_2Vpp
            ::SONY_FREIA_CONFIG_OUTLMT_DTV_1_5Vpp
            ::SONY_FREIA_CONFIG_OUTLMT_DTV_1_2Vpp
            ::SONY_FREIA_CONFIG_OUTLMT_STV_0_75Vpp
            ::SONY_FREIA_CONFIG_OUTLMT_STV_0_6Vpp
            ::SONY_FREIA_CONFIG_REFOUT_500mVpp
            ::SONY_FREIA_CONFIG_REFOUT_400mVpp
            ::SONY_FREIA_CONFIG_REFOUT_600mVpp
            ::SONY_FREIA_CONFIG_REFOUT_800mVpp
            ::SONY_FREIA_CONFIG_IFAGC_SEL_ALL1
            ::SONY_FREIA_CONFIG_IFAGC_SEL_ALL2
            ::SONY_FREIA_CONFIG_IFAGC_SEL_A1D2
            ::SONY_FREIA_CONFIG_IFAGC_SEL_D1A2
            ::SONY_FREIA_CONFIG_IFOUT_DC_BIAS_750mV
            ::SONY_FREIA_CONFIG_IFOUT_DC_BIAS_500mV
            defined in
            \link sony_freia.h \endlink
 @param pFreiaTuner The Freia tuner driver pointer to use.
        Memory must have been allocated for the Freia driver structure.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_tuner_freia_Create (sony_tuner_t * pTuner,
                                       uint8_t i2cAddress,
                                       sony_i2c_t * pI2c,
                                       uint32_t configFlags,
                                       sony_freia_t * pFreiaTuner);

/**
 @brief Write to GPIO0 or GPIO1.

 @param pTuner  Instance of the tuner driver.
 @param id      Pin ID 0 = GPIO0, 1 = GPIO1
 @param value   Output logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_tuner_freia_SetGPO (sony_tuner_t * pTuner, uint8_t id, uint8_t value);

/**
 @brief Read from GPIO0 or GPIO1.

 @param pTuner  Instance of the tuner driver.
 @param id      Pin ID 0 = GPIO0, 1 = GPIO1
 @param pValue  Read logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_tuner_freia_GetGPI (sony_tuner_t * pTuner, uint8_t id, uint8_t * pValue);

/**
 @brief RF filter compensation setting.
        (Please see RFVGA Description of datasheet.)

        New setting will become effective after next tuning.

        mult = coeff / 128
        (compensated value) = (original value) * mult + offset

 @param pTuner       Instance of the tuner driver.
 @param vlCoeff      Multiplier value. (8bit unsigned)
 @param vlOffset     Additional term. (8bit 2s complement)
 @param uvhCoeff     Multiplier value. (8bit unsigned)
 @param uvhOffset    Additional term. (8bit 2s complement)

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_tuner_freia_RFFilterConfig (sony_tuner_t * pTuner, uint8_t vlCoeff, uint8_t vlOffset,
       uint8_t uvhCoeff, uint8_t uvhOffset);

/**
 @brief Tune to a given RF frequency with satellite broadcasting system delivered by cable broadcast.

 @param pTuner         Instance of the tuner driver.
 @param centerFreqKHz  RF frequency to tune. (kHz)
 @param system         The type of broadcasting system to tune.
 @param symbolRateKSps Symbol rate to tune. (ksps)

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_tuner_freia_terr_IQ_Tune (sony_tuner_t * pTuner,
                                             uint32_t centerFreqKHz,
                                             sony_dtv_system_t system,
                                             uint32_t symbolRateKSps);

#endif /* SONY_TUNER_FREIA_H */
