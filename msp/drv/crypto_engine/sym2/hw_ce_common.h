/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_CE_COMMON_H__
#define __HW_CE_COMMON_H__

#include <linux/delay.h>
#include "mt_type.h"
#include "mt_drv_dev.h"


#define HW_CE_HASH_MSG_LENGTH_MAX	0x400000

#define HW_CE_FIFO_DEPTH		128
#define HW_TS_PKT188_FIFO_DEPTH		188
#define HW_TS_PKT192_FIFO_DEPTH		192

#define CHK_IF_CPU_MODE_ADES		(input_addr & 0x00000007)		\
									||(output_addr & 0x00000007)	\
									||((0x0 != input_addr>>28)	\
									&&(0x0 != input_addr>>28)	)	\
									||((0x0 != output_addr>>28)	\
									&&(0x0 != output_addr>>28))	

#define CHK_IF_CPU_MODE_SHA		(input_addr & 0x00000007)			\
									|| ((0x0 != (input_addr >> 28))		\
									&& (0x0 != (input_addr >> 28)))
									
void hw_ce_data_print(char *string, mt_u8 *data, mt_u32 length, mt_u8 align);
void hw_ce_msdelay_customize(mt_u32 ms);

#endif	/*__HW_CE_COMMON_H__*/
