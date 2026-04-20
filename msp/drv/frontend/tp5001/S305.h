/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifndef S305_H
#define S305_H

#include "TP5001.h"
#ifdef _USE_TP5001_CHIP_

#define S305_DEV_ADDR		0x61

TP_UINT8 S305_init(void);
TP_UINT8 S305_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value);


#endif

#endif// _USE_TP5001_CHIP_
