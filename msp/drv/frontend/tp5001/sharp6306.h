/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifndef SHARP6306_H
#define SHARP6306_H

#include "TP5001.h"
#ifdef _USE_TP5001_CHIP_

#define SHARP6306_DEV_ADDR		0x60

TP_UINT8 sharp_6306_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value);


#endif

#endif// _USE_TP5001_CHIP_


