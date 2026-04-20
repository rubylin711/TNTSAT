/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include <linux/printk.h>

#include "sony_i2c.h"

#include "../lib/sony_stdlib.h" /* for memcpy */

#define BURST_WRITE_MAX 128 /* Max length of burst write */

sony_result_t sony_i2c_CommonReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
	uint8_t w_len, r_len;
	uint8_t buf[130];

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonReadRegister");

    if(!pI2c || (!pData)){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

#if 1
	w_len = (size >> 16) & 0xFF;
	r_len = size & 0xFF;

	buf[0] = subAddress;
	memcpy(&buf[1], pData, w_len);

	w_len += 1;
	size = (w_len << 16) + r_len;

    result = pI2c->Read(pI2c, deviceAddress, buf, size, SONY_I2C_START_EN | SONY_I2C_STOP_EN);

	memcpy(pData, buf, r_len);
#else
    result = pI2c->Write(pI2c, deviceAddress, &subAddress, 1, SONY_I2C_START_EN);
    if(result == SONY_RESULT_OK){
        result = pI2c->Read(pI2c, deviceAddress, pData, size, SONY_I2C_START_EN | SONY_I2C_STOP_EN);
    }
#endif

	//printk("%s[%d] ---- addr[%02x], reg[%02x], data[%02x], result = %d\n", __FUNCTION__, __LINE__, deviceAddress, subAddress, pData[0], result);

    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t sony_i2c_CommonWriteRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t buffer[BURST_WRITE_MAX + 1];

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteRegister");

    if(!pI2c || (!pData)){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(size > BURST_WRITE_MAX){
        /* Buffer is too small... */
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    buffer[0] = subAddress;
    sony_memcpy(&(buffer[1]), pData, size);

    /* send the new buffer */
    result = pI2c->Write(pI2c, deviceAddress, buffer, size+1, SONY_I2C_START_EN | SONY_I2C_STOP_EN);

	//printk("%s[%d] ---- addr[%02x], reg[%02x], data[%02x], result = %d\n", __FUNCTION__, __LINE__, deviceAddress, subAddress, pData[0], result);

    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t sony_i2c_CommonWriteOneRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteOneRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = pI2c->WriteRegister(pI2c, deviceAddress, subAddress, &data, 1);

	//printk("%s[%d] ---- addr[%02x], reg[%02x], data[%02x], result = %d\n", __FUNCTION__, __LINE__, deviceAddress, subAddress, data, result);

    SONY_TRACE_I2C_RETURN(result);
}

/* For Read-Modify-Write */
sony_result_t sony_i2c_SetRegisterBits(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data, uint8_t mask)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_SetRegisterBits");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(mask == 0x00){
        /* Nothing to do */
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    }

    if(mask != 0xFF){
        uint8_t rdata = 0x00;
        result = pI2c->ReadRegister(pI2c, deviceAddress, subAddress, &rdata, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_I2C_RETURN(result); }
        data = (uint8_t)((data & mask) | (rdata & (mask ^ 0xFF)));
    }

    result = pI2c->WriteOneRegister(pI2c, deviceAddress, subAddress, data);

	//printk("%s[%d] ---- addr[%02x], reg[%02x], data[%02x], mask[%02x], result = %d\n", __FUNCTION__, __LINE__, deviceAddress, subAddress, data, mask, result);

    SONY_TRACE_I2C_RETURN(result);
}

