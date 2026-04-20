/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_I2C_IOCTL_H__
#define __DRV_I2C_IOCTL_H__

#include "mt_unf_i2c.h"
#include "mt_drv_i2c.h"

#define SEND_MAX_BUFF	(120)

#pragma pack(4)
typedef struct mt_i2c_data
{
    mt_u32 i2c_id;
    mt_u8 dev_addr;
    mt_u8 reserved[3];
    mt_u32 reg_addr;
    mt_u32 reg_count;
    mt_u32 data_len;
    mt_u32 reserved32;
    mt_u64 p_data;          //mt_u8 *
} i2c_data_t;

typedef struct mt_i2c_data_ex
{
    mt_u32 i2c_id;
    mt_u8 dev_addr;
    mt_u8 reserved[3];
    mt_u32 reg_addr;
    mt_u32 reg_count;
    mt_u32 data_len;
/*
    uapi/linux/i2c.h
#define I2C_M_RD		    0x0001  // only read
#define I2C_M_TEN		    0x0010  // no use
#define I2C_M_STD_RD		0x0020	// addr+[write reg],stop,read
#define I2C_M_SEQ_RD		0x0040	// addr+[write reg],nostop,read
#define I2C_M_SALVE_TYPE	0x0080	// demod,need use me to config pinmux
*/
    mt_u16 flags;
/*
uapi/linux/i2c.h
#define I2C_SLAVE_DEV_SOC_EXTER		0x01
#define I2C_SLAVE_DEV_SOC_INTER		0x02
#define I2C_SLAVE_DEV_SOC_INTER_DEVICE1	0x03
#define I2C_SLAVE_DEV_SOC_INTER_DEVICE2	0x04
#define I2C_SLAVE_DEV_SOC_EXTER_DEVICE1	0x05
*/
    mt_u16 types;
    mt_u64 p_data;        //mt_u8 *
} i2c_data_ex_t;

typedef struct mt_i2c_rate
{
    mt_u32 i2c_id;
    mt_u32 i2c_rate;
} i2c_rate_t;

typedef struct mt_i2c_gpio
{
    mt_u32 i2c_id;
    mt_u32 scl_gpio_no;
    mt_u32 sda_gpio_no;
    MT_BOOL b_used;
    mt_u32 count;
} i2c_gpio_t;
#pragma pack()

/* Ioctl definitions */
#define CMD_I2C_READ        _IOW(MT_ID_I2C, 0x1, i2c_data_t)	    //
#define CMD_I2C_WRITE       _IOW(MT_ID_I2C, 0x2, i2c_data_t)	    //
#define CMD_I2C_SET_RATE    _IOW(MT_ID_I2C, 0x3, i2c_rate_t)
#define CMD_I2C_CONFIG      _IOWR(MT_ID_I2C, 0x4, i2c_gpio_t)	    //config  gpioi2c
#define CMD_I2C_DESTROY     _IOW(MT_ID_I2C, 0x5, i2c_gpio_t)	    //destroy gpioi2c
#define CMD_I2C_RESET       _IOW(MT_ID_I2C, 0x6, mt_u32)	        //destroy gpioi2c
#define CMD_I2C_READ_EX     _IOWR(MT_ID_I2C, 0x7, i2c_data_ex_t)   // read i2c with flag
#define CMD_I2C_WRITE_EX    _IOW(MT_ID_I2C, 0x8, i2c_data_ex_t)    // write i2c with flag

void i2c_fp_enable(void);
int i2c_read_fp(i2c_data_t data);
int i2c_write_fp(i2c_data_t data);

#endif /* End of #ifndef __DRV_I2C_IOCTL_H__*/
