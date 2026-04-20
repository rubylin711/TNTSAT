/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifndef RDA515_H
#define RDA515_H

#include "TP5001.h"
#ifdef _USE_TP5001_CHIP_

#define RDA5815_DEV_ADDR		0x0C

TP_UINT8 rda_5815_init(void);
TP_UINT8 rda_5815_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value);


#endif

#endif// _USE_TP5001_CHIP_
