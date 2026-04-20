/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_I2C_H__
#define __MT_UNF_I2C_H__

#include "mt_common.h"
#include "mt_error_mpi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup      I2C */
/** @{*/ /** <!-[I2C]*/

#define mt_unf_i2c_open mt_unf_i2c_init
#define mt_unf_i2c_close mt_unf_i2c_deinit

/**Maximum I2C channel ID*/
#define MT_I2C_MAX_NUM_USER (6)

/**Rate type of the I2C module*/
/**CNcomment:I2C的速率类型 */
typedef enum _unf_i2c_rate_t {
    MT_UNF_I2C_RATE_10K = 0,
    /**<Standard rate: 10 kbit/s*/
    MT_UNF_I2C_RATE_50K,
    /**<Standard rate: 50 kbit/s*/
    MT_UNF_I2C_RATE_100K,
    /**<Standard rate: 100 kbit/s*/
    MT_UNF_I2C_RATE_200K,
    /**<Standard rate: 200 kbit/s*/
    MT_UNF_I2C_RATE_300K,
    /**<Standard rate: 300 kbit/s*/
    MT_UNF_I2C_RATE_400K,
    /**<Fast rate: 400 kbit/s*/
    MT_UNF_I2C_RATE_BUTT
} mt_unf_i2c_rate_t;

/** ==== Structure Definition End ====*/

/******************************* API Declaration *****************************/

/**
 \brief Init the I2C device.
 \param N/A
 \retval 0 Success
 \retval ::MT_ERR_I2C_OPEN_ERR  Open I2c Error
 */
mt_s32 mt_unf_i2c_init(mt_void);

/**
 \brief  DeInit the I2C device.
 \attention:
 \This API is called after I2C operations are completed.
 \param N/A
 \retval 0 Success
 \retval ::MT_ERR_I2C_CLOSE_ERR  Close I2c Error.
 */
mt_s32 mt_unf_i2c_deinit(mt_void);

/**
 \brief  Get the number of I2C module.
 \attention:
 \Call this API to get the number of I2C module befor read/write data.
 \retval 0 Success
 */
mt_s32 mt_unf_i2c_get_capability(mt_u32 *p_i2c_id);

/**
 \The I2C device is not initialized.
 \attention:
 \If the specified GPIO pins are used, this API fails to be called.
 \param[out] i2c_id  ID of the obtained I2C bus
 \param[in] u32SCLGpioNo  SCL Pin number, ranging from 0 to 103, ranging is different in otherness chip type
 \param[in] u32SDAGpioNo  SDA Pin number, ranging from 0 to 103, ranging is different in otherness chip type
 \retval 0 Success
 \retval ::MT_FAILURE	Create gpioi2c failed
 \retval ::MT_ERR_I2C_NULL_PTR 		The pointer parameter is NULL
 \retval ::MT_ERR_GPIO_INVALID_PARA  The parameter is invalid.
 */
mt_s32 mt_unf_i2c_create_gpioi2c(mt_u32 *p_i2c_id, mt_u32 scl_gpio_no, mt_u32 sda_gpio_no);

/**
 \brief Destroys a inter-integrated circuit (I2C) channel that simulates the general-purpose input/output (GPIO) function.
 \attention:
 \If the I2C channel is not used, a code indicating success is returned.\n
 \param[in] i2c_id ID of the I2C bus to be destroyed
 \retval 0  Success
 \retval ::MT_FAILURE	Destroy gpioi2c failed
 \retval ::MT_ERR_GPIO_INVALID_PARA  The parameter is invalid.
 */
mt_s32 mt_unf_i2c_destroy_gpioi2c(mt_u32 i2c_id);

/**
 \brief Reads data by using the I2C bus.
 \param[in] i2c_id  I2C bus of the device to be read
 \param[in] dev_addr  Address of a device on the I2C bus
 \param[in] reg_addr  On-chip offset address of a device
 \param[in] reg_addr_count  Length of an on-chip offset address.
	1: 8-bit sub address
	2: 16-bit sub address
	3: 24-bit sub address
	4: 32-bit sub address
 \param[out] p_buf   Buffer for storing the data to be read
 \param[in] len  Length of the data to be read
 \retval 0 Success
 \retval ::MT_FAILURE	Read data failed
 \retval ::MT_ERR_I2C_NOT_INIT  The I2C device is not initialized.
 \retval ::MT_ERR_I2C_NULL_PTR  The I2C pointer is invalid.
 \retval ::MT_ERR_I2C_INVALID_PARA  The I2C parameter is invalid.
 \retval ::MT_ERR_I2C_FAILED_READ  Data fails to be read by using the I2C bus.
 */
mt_s32 mt_unf_i2c_read(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                       mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len);

/**
 \brief Writes data by using the I2C bus. That is, you can call this API to write data to the device mounted on the I2C bus through the I2C channel.
 \param[in] i2c_id  I2C bus of the device to be written
 \param[in] dev_addr  Address of a device on the I2C bus
 \param[in] reg_addr  On-chip offset address of a device
 \param[in] reg_addr_count Length of an on-chip offset address.
	1: 8-bit sub address
	2: 16-bit sub address
	3: 24-bit sub address
	4: 32-bit sub address
 \param[in]  p_buf   Buffer for storing the data to be written
 \param[in] len  Length of the data to be written
 \retval 0  Success
 \retval ::MT_FAILURE	Write data failed
 \retval ::MT_ERR_I2C_NOT_INIT  The I2C device is not initialized.
 \retval ::MT_ERR_I2C_NULL_PTR  The I2C pointer is invalid.
 \retval ::MT_ERR_I2C_INVALID_PARA  The I2C parameter is invalid.
 \retval ::MT_ERR_I2C_FAILED_WRITE  Data fails to be written by using the I2C bus.
 */
mt_s32 mt_unf_i2c_write(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                        mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len);

/**
 \brief Reads data by using the I2C bus.
CNcomment:\brief 通过I2C读数据。CNend

 \attention \n
N/A
 \param[in] i2c_id  I2C bus of the device to be read           CNcomment:所读取设备使用的I2C总线。CNend
 \param[in] dev_addr  Address of a device on the I2C bus      CNcomment:设备在I2C总线上的地址。CNend
 \param[in] reg_addr  On-chip offset address of a device        CNcomment:设备的片内偏移地址。CNend
 \param[in] reg_addr_count  Length of an on-chip offset address. CNcomment:片内偏移地址的长度单位。CNend
                      1: 8-bit sub address                       CNcomment:1：表示8bit子地址；CNend
                      2: 16-bit sub address                      CNcomment:2：表示16bit子地址；CNend
                      3: 24-bit sub address                      CNcomment:3：表示24bit子地址；CNend
                      4: 32-bit sub address                      CNcomment:4：表示32bit子地址。CNend
 \param[in] flag, the mode of i2c communication,see i2c_data_ex_t.CNcomment:i2c 通信方式，参考i2c_data_ex_t。CNend
 \param[in] type, set type of demod,see i2c_data_ex_t.            CNcomment:设置dmod类型，参考i2c_data_ex_t。CNend

 \param[out] p_buf   Buffer for storing the data to be read                            CNcomment:读Buffer，存放读取数据。CNend
 \param[in] len  Length of the data to be read                                    CNcomment:要读取的数据长度。CNend
 \retval 0 Success                                                                      CNcomment:成功。CNend
 \retval ::MT_FAILURE	Read data failed					  	CNcomment:失败。CNend
 \retval ::MT_ERR_I2C_NOT_INIT  The I2C device is not initialized.                      CNcomment:I2C设备未初始化。CNend
 \retval ::MT_ERR_I2C_NULL_PTR  The I2C pointer is invalid.                        	   CNcomment:I2C无效指针。CNend
 \retval ::MT_ERR_I2C_INVALID_PARA  The I2C parameter is invalid.                       CNcomment:I2C无效参数。CNend
 \retval ::MT_ERR_I2C_FAILED_READ  Data fails to be read by using the I2C bus.          CNcomment:I2C读数据失败。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_i2c_read_ex(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                       mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len, mt_u16 flag, mt_u16 type);

/**
 \brief Writes data by using the I2C bus. That is, you can call this API to write data to the device mounted on the I2C bus through the I2C channel.
CNcomment:\brief 通过I2C写数据。通过I2C通道，向I2C总线上挂接的设备进行写操作。CNend

 \attention \n
N/A
 \param[in] i2c_id  I2C bus of the device to be written         CNcomment:待写设备使用的I2C总线。CNend
 \param[in] dev_addr  Address of a device on the I2C bus       CNcomment:设备在I2C总线上的地址。CNend
 \param[in] reg_addr  On-chip offset address of a device         CNcomment:设备的片内偏移地址。CNend
 \param[in] reg_addr_count Length of an on-chip offset address.   CNcomment:片内偏移地址的长度单位。CNend
                    1: 8-bit sub address                          CNcomment:1：表示8bit子地址；CNend
                    2: 16-bit sub address                         CNcomment:2：表示16bit子地址；CNend
                    3: 24-bit sub address                         CNcomment:3：表示24bit子地址；CNend
                    4: 32-bit sub address                         CNcomment:4：表示32bit子地址。CNend
 \param[in] flag, the mode of i2c communication,see i2c_data_ex_t.CNcomment:i2c 通信方式，参考i2c_data_ex_t。CNend
 \param[in] type, set type of demod,see i2c_data_ex_t.            CNcomment:设置dmod类型，参考i2c_data_ex_t。CNend

 \param[in]  p_buf   Buffer for storing the data to be written                         CNcomment:写Buffer，存放待写入数据。CNend
 \param[in] len  Length of the data to be written                                 CNcomment:要写入的数据的长度。CNend
 \retval 0  Success                                                                     CNcomment:成功。CNend
 \retval ::MT_FAILURE	Write data failed					  	CNcomment:失败。CNend
 \retval ::MT_ERR_I2C_NOT_INIT  The I2C device is not initialized.                      CNcomment:I2C设备未初始化。CNend
 \retval ::MT_ERR_I2C_NULL_PTR  The I2C pointer is invalid.                        	   CNcomment:I2C无效指针。CNend
 \retval ::MT_ERR_I2C_INVALID_PARA  The I2C parameter is invalid.                       CNcomment:I2C无效参数。CNend
 \retval ::MT_ERR_I2C_FAILED_WRITE  Data fails to be written by using the I2C bus.      CNcomment:I2C写数据失败。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_i2c_write_ex(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                        mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len, mt_u16 flag, mt_u16 type);

/**
 \brief Sets the transfer rate of the I2C bus.
 \Call this API Only be effect in standard i2c, gpio simulate i2c is noneffective.
 \If you do not call this API to set the transfer rate, the rate 100 kbit/s is used by default.
 \param[in] i2c_id  D of channel corresponding to the device to be written on the I2C bus
 \param[in] enI2cRate  I2C clock rate. For details about the definition, see the description of ::mt_unf_i2c_rate_t.
 \retval 0  Success
 \retval ::MT_FAILURE	Set rate failed
 \retval ::MT_ERR_I2C_NOT_INIT  The I2C device is not initialized.
 \retval ::MT_ERR_I2C_INVALID_PARA  The I2C parameter is invalid.
 */
mt_s32 mt_unf_i2c_set_rate(mt_u32 i2c_id, mt_unf_i2c_rate_t i2c_rate);

/**
 \brief Sets the transfer rate of the I2C bus.
 \attention:
 \Call this API Only be effect in standard i2c, gpio simulate i2c is noneffective.
 \If you do not call this API to set the transfer rate, the rate 100 kbit/s is used by default.
 \param[in] i2c_id  D of channel corresponding to the device to be written on the I2C bus
 \param[in] i2c_rate  I2C clock rate.
 \retval 0  Success
 \retval ::MT_FAILURE	Set rate failed
 \retval ::MT_ERR_I2C_NOT_INIT  The I2C device is not initialized.
 \retval ::MT_ERR_I2C_INVALID_PARA  The I2C parameter is invalid.
 */
mt_s32 mt_unf_i2c_set_rate_ex(mt_u32 i2c_id, mt_u32 i2c_rate);

mt_s32 mt_unf_i2c_reset(mt_u32 id);

/** ==== API Declaration End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_UNF_ECS_TYPE_H__ */
