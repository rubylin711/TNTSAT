/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifndef __IIC_H__
#define __IIC_H__

#include "TP_TYPE.h"
#ifdef _USE_TP5001_CHIP_

#ifdef   __cplusplus
    extern   "C" 
    {
#endif

// ==============================================================================================
// External Function
// 

// ---------------------------------------------------------------------------------------------
// Function Name: TP_iic_write
// Description: TP5001芯片，IIC接口写操作
// Parameter:
//		device_address: IIC设备地址，7bit地址
//		register_address：IIC寄存器地址
//		value_buffer：写入值指针
//		length: 写入长度, 最大256字节
// Return:
//		TP_SUCCESS: 操作成功
//		TP_IIC_WRERR: IIC写入错误
//		TP_IIC_WR_TOO_LONG: 写入长度过长
//
TP_UINT8 TP_iic_write(TP_UINT8 device_address, TP_UINT16 register_address, TP_UINT8 * value_buffer, TP_UINT32 length);
TP_UINT8 TP_iic_write_tuner(TP_UINT8 device_address, TP_UINT8 * value_buffer, TP_UINT32 length);
// ---------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------
// Function Name: TP_iic_read
// Description: TP5001芯片，IIC接口读操作
// Parameter:
//		device_address: IIC设备地址，7bit地址
//		register_address：IIC寄存器地址
//		value_buffer：读出值指针
//		length: 读出长度, 最大256字节
// Return:
//		TP_SUCCESS: 操作成功
//		TP_IIC_RDERR: IIC读取错误
//		TP_IIC_RD_TOO_LONG：读取长度过长
//
TP_UINT8 TP_iic_read(TP_UINT8 device_address, TP_UINT16 register_address, TP_UINT8 * value_buffer, TP_UINT32 length);
TP_UINT8 TP_iic_read_tuner(TP_UINT8 device_address, TP_UINT8 register_address, TP_UINT8 * value_buffer, TP_UINT32 length);
// ---------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------
// Function Name: TP_Delay
// Description: TP5001芯片，延时xx ms
// Parameter:
//		uiMS: Delay多少ms
// Return:
//		None
//
void TP_Delay( TP_UINT32 uiMS );
// ---------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------
// Function Name: TP_Reset
// Description: TP5001芯片的复位管脚设置复位
// Return:
//		None
//
void TP_Reset(void);
// ---------------------------------------------------------------------------------------------
// ==============================================================================================


#ifdef   __cplusplus
    }
#endif

#endif

#endif// _USE_TP5001_CHIP_
