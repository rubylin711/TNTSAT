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
 @file    sony_demod_dvbs_s2_tune_srs.h

          This file provides demodulator setting for TuneSRS function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_TUNE_SRS_H
#define SONY_DEMOD_DVBS_S2_TUNE_SRS_H

#include "../../sony_common.h"
#include "../../sony_demod.h"

/*------------------------------------------------------------------------------
 Definitions
------------------------------------------------------------------------------*/

/**
 @brief Frequency range for TuneSRS
*/
typedef enum {
    SONY_DEMOD_DVBS_S2_TUNE_SRS_FREQ_RANGE_10MHz,   /**< +/- 10MHz */
    SONY_DEMOD_DVBS_S2_TUNE_SRS_FREQ_RANGE_5MHz     /**< +/-  5MHz */
} sony_demod_dvbs_s2_tune_srs_freq_range_t;

#endif /* SONY_DEMOD_DVBS_S2_TUNE_SRS_H */
