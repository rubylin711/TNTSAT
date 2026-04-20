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
 @file    sony_dvbc.h

          This file provides DVB-C related type definitions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DVBC_H
#define SONY_DVBC_H

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/**
 @brief DVB-C constellation.
*/
typedef enum {
    SONY_DVBC_CONSTELLATION_16QAM,          /**< 16-QAM */
    SONY_DVBC_CONSTELLATION_32QAM,          /**< 32-QAM */
    SONY_DVBC_CONSTELLATION_64QAM,          /**< 64-QAM */
    SONY_DVBC_CONSTELLATION_128QAM,         /**< 128-QAM */
    SONY_DVBC_CONSTELLATION_256QAM          /**< 256-QAM */
} sony_dvbc_constellation_t;

#endif /* SONY_DVBC_H */
