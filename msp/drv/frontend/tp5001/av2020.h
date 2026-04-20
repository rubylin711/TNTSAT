/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#include "TP5001.h"

#ifdef _USE_TP5001_CHIP_

#ifndef AV2020_H
#define AV2020_H



//#define AV2020_DEV_ADDR		0x63
extern TP_UINT8 AV2020_DEV_ADDR;
TP_UINT8 av2020_init(void);
unsigned int Tuner_control (unsigned int channel_freq, unsigned int bb_sym);


#endif

#endif// _USE_TP5001_CHIP_
