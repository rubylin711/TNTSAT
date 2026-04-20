/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef TP5001_RDA515M_H
#define TP5001_RDA515M_H

#include "TP5001.h"

#ifdef _USE_TP5001_CHIP_
#define RDA5815M_DEV_ADDR		0x0c//0x1A//0x18//0x0D//0x0C

TP_UINT8 rda_5815M_init(void);
TP_UINT8 rda_5815M_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value);


#endif

#endif// _USE_TP5001_CHIP_
