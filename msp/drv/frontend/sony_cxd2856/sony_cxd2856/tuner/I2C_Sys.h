/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

#ifndef _I2C_SYS_H_
#define _I2C_SYS_H_

//#include "R850/R850.h"
//#include "R858C/R858.h"

typedef struct _I2C_LEN_TYPE
{
	UINT8 Data[100];
	UINT8 RegAddr;
	UINT8 Len;
	UINT8 I2cAddr;
	UINT8 Temp;
}I2C_LEN_TYPE;

typedef struct _I2C_TYPE
{
	UINT8 RegAddr;
	UINT8 Data;
	UINT8 I2cAddr;
	UINT8 Temp;
}I2C_TYPE;

#endif

