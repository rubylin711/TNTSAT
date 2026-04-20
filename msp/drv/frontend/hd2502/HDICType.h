/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#pragma once

#define UCHAR	unsigned char
#define USHORT	unsigned short

#define INT8	char		/**<  8 bits, bit  7 is the signed bit */
#define INT16	short		/**< 16 bits, bit 15 is the signed bit */
#define INT32	long		/**< 32 bits, bit 31 is the signed bit */

#define UINT8	unsigned char	/**<  8 bits */
#define UINT16	unsigned short	/**< 16 bits */
#define UINT32	unsigned long	/**< 32 bits */

#define RX_Undefined 0xFF	/**< Not defined Value */

#define HDIC_NO_ERROR						0		/**< no error */
#define HDIC_AUTO_DETECT_FAILED				0xFF	/**< auto detect failed */
#define HDIC_I2C_TRANSFER_ERROR				0xFE	/*I2C Transfer Error*/

#define HDIC_Print(x)					printk x

