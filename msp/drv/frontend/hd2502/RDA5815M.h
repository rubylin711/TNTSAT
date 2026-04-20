/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef HD2502_RDA515M_H
#define HD2502_RDA515M_H

#include "HDIC2501.h"

#define HD2502_RDA5815M_DEV_ADDR		0x18//0x0c//0x1A//0x18//0x0D//0x0C

void RDA5815mInitial(void);
unsigned char RDA5815mSet(unsigned long fPLL, unsigned long fSym);

#endif// HD2502_RDA515M_H

