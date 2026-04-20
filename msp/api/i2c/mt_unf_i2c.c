/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "drv_i2c_ioctl.h"
#include "mt_drv_struct.h"

static mt_s32 g_i2cdev_fd = -1;

static pthread_mutex_t g_i2c_mutex = PTHREAD_MUTEX_INITIALIZER;

//static const mt_u8 g_i2c_version[] = "SDK_VERSION:[" MKMARCOTOSTR(SDK_VERSION) "] Build Time:[" __DATE__ ", " __TIME__ "]";

#define MT_I2C_LOCK() (void) pthread_mutex_lock(&g_i2c_mutex);
#define MT_I2C_UNLOCK() (void) pthread_mutex_unlock(&g_i2c_mutex);

#define CHECK_I2C_OPEN()                      \
    do {                                      \
	MT_I2C_LOCK();                        \
	if (g_i2cdev_fd < 0) {                \
	    MT_ERR_I2C("I2C is not open.\n"); \
	    MT_I2C_UNLOCK();                  \
	    return MT_ERR_I2C_NOT_INIT;       \
	}                                     \
	MT_I2C_UNLOCK();                      \
    } while (0)

/*******************************************
Function:      mt_unf_i2c_init
Description:   init i2c device
Calls:         mt_drv_i2c_open
Data Accessed: NA
Data Updated:  NA
Input:         NA
Output:        NA
return:        ErrorCode(reference to document)
Others:        NA
*******************************************/
mt_s32 mt_unf_i2c_init(mt_void)
{
    MT_I2C_LOCK();

    /* reopen will direct return success*/
    if (g_i2cdev_fd > 0)
    {
        MT_I2C_UNLOCK();
        return MT_SUCCESS;
    }

    g_i2cdev_fd = open("/dev/" UMAP_DEVNAME_I2C, O_RDWR, 0);
    if (g_i2cdev_fd < 0)
    {
        MT_FATAL_I2C("open I2C err.\n");
        MT_I2C_UNLOCK();
        return MT_ERR_I2C_OPEN_ERR;
    }

    MT_I2C_UNLOCK();
    return MT_SUCCESS;
}

/*******************************************
Function:      mt_unf_i2c_DeInit
Description:   deinit i2c device
Calls:         mt_drv_i2c_close
Data Accessed: NA
Data Updated:  NA
Input:         NA
Output:        NA
return:        ErrorCode(reference to document)
Others:        NA
*******************************************/
mt_s32 mt_unf_i2c_deinit(mt_void)
{
	mt_s32 ret;

    MT_I2C_LOCK();

    if (g_i2cdev_fd < 0)
    {
        MT_I2C_UNLOCK();
        return MT_SUCCESS;
    }

    ret = close(g_i2cdev_fd);

    if (MT_SUCCESS != ret)
    {
        MT_FATAL_I2C("Close I2C err.\n");
        MT_I2C_UNLOCK();
        return MT_ERR_I2C_CLOSE_ERR;
    }

    g_i2cdev_fd = -1;

    MT_I2C_UNLOCK();
    return MT_SUCCESS;
}

/*******************************************
Function:mt_unf_i2c_get_capability
Description:Call this API to get the number of I2C module befor read/write data
Calls:
Data Accessed:NA
Data Updated:NA
Input:NA
Output:p_i2c_id - the number of I2C module
return:ErrorCode(reference to document)
Others:NA
*******************************************/
mt_s32 mt_unf_i2c_get_capability(mt_u32 *p_i2c_id)
{
    if (MT_NULL != p_i2c_id) {
	*p_i2c_id = MT_STD_I2C_NUM;
	return MT_SUCCESS;
    }

    return MT_FAILURE;
}

/*******************************************
Function:     mt_unf_i2c_read
Description:  read data via i2c
Calls:        mt_drv_i2c_read
Data Accessed:      NA
Data Updated:        NA
Input:                  NA
Output:                NA
return:         ErrorCode(reference to document)
Others:                  NA
*******************************************/
mt_s32 mt_unf_i2c_read(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                       mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len)
{
	mt_s32 ret = 0;
    i2c_data_t I2cData;

    if (i2c_id > MT_I2C_MAX_NUM_USER)
    {
        MT_ERR_I2C("para i2c_id is %d invalid.\n",i2c_id);
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (reg_addr_count > 4)
    {
        MT_ERR_I2C("para reg_addr_count is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (!p_buf)
    {
        MT_ERR_I2C("para penPressStatus is null.\n");
        return MT_ERR_I2C_NULL_PTR;
    }

    if ((len > MT_I2C_MAX_LENGTH) || (0 == len))
    {
        MT_ERR_I2C("para len is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    CHECK_I2C_OPEN();

    I2cData.i2c_id = i2c_id;
    I2cData.dev_addr = dev_addr;
    I2cData.reg_addr = reg_addr;
    I2cData.reg_count = reg_addr_count;
    I2cData.p_data = (ulong)p_buf;
    I2cData.data_len = len;

    ret = ioctl(g_i2cdev_fd, CMD_I2C_READ, &I2cData);
    if (ret != MT_SUCCESS)
    {
        return ret;
    }
    return MT_SUCCESS;
}

/*******************************************
Function:              mt_unf_i2c_write
Description:  write data via i2c
Calls:        mt_drv_i2c_write
Data Accessed:      NA
Data Updated:        NA
Input:                  NA
Output:                NA
return:         ErrorCode(reference to document)
Others:                  NA
*******************************************/
mt_s32 mt_unf_i2c_write(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                        mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len)
{
    mt_s32 ret = 0;
    i2c_data_t I2cData;

    if (i2c_id > MT_I2C_MAX_NUM_USER)
    {
        MT_ERR_I2C("para i2c_id is %d invalid.\n",i2c_id);
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (reg_addr_count > 4)
    {
        MT_ERR_I2C("para reg_addr_count is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (!p_buf)
    {
        MT_ERR_I2C("para penPressStatus is null.\n");
        return MT_ERR_I2C_NULL_PTR;
    }

    if ((len > MT_I2C_MAX_LENGTH) || (0 == len))
    {
        MT_ERR_I2C("para len is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    CHECK_I2C_OPEN();

    I2cData.i2c_id = i2c_id;
    I2cData.dev_addr = dev_addr;
    I2cData.reg_addr = reg_addr;
    I2cData.reg_count = reg_addr_count;
    I2cData.p_data = (ulong)p_buf;
    I2cData.data_len = len;

    ret = ioctl(g_i2cdev_fd, CMD_I2C_WRITE, &I2cData);
    if (ret != MT_SUCCESS)
    {
        return ret;
    }

    return MT_SUCCESS;
}

/*******************************************
Function:     mt_unf_i2c_read_ex
Description:  read data via i2c with flag, not for fp_i2c and gpio_i2c
Calls:        i2c_read_common
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
return:         ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_i2c_read_ex(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                       mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len, mt_u16 flag, mt_u16 type)
{
	mt_s32 ret = 0;
    i2c_data_ex_t I2cData_ex;

    if (i2c_id > MT_I2C_MAX_NUM_USER) {
        MT_ERR_I2C("para i2c_id is %d invalid.\n",i2c_id);
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (reg_addr_count > 4) {
        MT_ERR_I2C("para reg_addr_count is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (!p_buf) {
        MT_ERR_I2C("para penPressStatus is null.\n");
        return MT_ERR_I2C_NULL_PTR;
    }

    if ((len > MT_I2C_MAX_LENGTH) || (0 == len)) {
        MT_ERR_I2C("para len is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    CHECK_I2C_OPEN();

    I2cData_ex.i2c_id = i2c_id;
    I2cData_ex.dev_addr = dev_addr;
    I2cData_ex.reg_addr = reg_addr;
    I2cData_ex.reg_count = reg_addr_count;
    I2cData_ex.p_data = (ulong)p_buf;
    I2cData_ex.data_len = len;
    I2cData_ex.flags = flag;
    I2cData_ex.types = type;

    ret = ioctl(g_i2cdev_fd, CMD_I2C_READ_EX, &I2cData_ex);
    if (ret != MT_SUCCESS) {
        return ret;
    }
    return MT_SUCCESS;
}

/*******************************************
Function:     mt_unf_i2c_write_ex
Description:  write data via i2c with flag, not for fp_i2c and gpio_i2c
Calls:        i2c_write_common
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
return:         ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_i2c_write_ex(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr,
                        mt_u32 reg_addr_count, mt_u8 *p_buf, mt_u32 len, mt_u16 flag, mt_u16 type)
{
    mt_s32 ret = 0;
    i2c_data_ex_t I2cData_ex;

    if (i2c_id > MT_I2C_MAX_NUM_USER) {
        MT_ERR_I2C("para i2c_id is %d invalid.\n",i2c_id);
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (reg_addr_count > 4) {
        MT_ERR_I2C("para reg_addr_count is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (!p_buf) {
        MT_ERR_I2C("para penPressStatus is null.\n");
        return MT_ERR_I2C_NULL_PTR;
    }

    if ((len > MT_I2C_MAX_LENGTH) || (0 == len)) {
        MT_ERR_I2C("para len is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    CHECK_I2C_OPEN();

    I2cData_ex.i2c_id = i2c_id;
    I2cData_ex.dev_addr = dev_addr;
    I2cData_ex.reg_addr = reg_addr;
    I2cData_ex.reg_count = reg_addr_count;
    I2cData_ex.p_data = (ulong)p_buf;
    I2cData_ex.data_len = len;
    I2cData_ex.flags = flag;
    I2cData_ex.types = type;

    ret = ioctl(g_i2cdev_fd, CMD_I2C_WRITE_EX, &I2cData_ex);
    if (ret != MT_SUCCESS) {
        return ret;
    }

    return MT_SUCCESS;
}

/*******************************************
Function:       mt_unf_i2c_create_gpioi2c
Description:    config param
Calls:          gpio_i2c_config
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:
return:         ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_i2c_create_gpioi2c(mt_u32 *p_i2c_id, mt_u32 scl_gpio_no, mt_u32 sda_gpio_no)
{
    i2c_gpio_t I2cGpio;
    mt_s32 ret = 0;

    CHECK_I2C_OPEN();

    if (MT_NULL == p_i2c_id)
    {
        MT_ERR_I2C("para i2c_id is NULL.\n");
        return MT_ERR_I2C_NULL_PTR;
    }

    if (scl_gpio_no == sda_gpio_no)
    {
        MT_ERR_I2C("scl_gpio_no == sda_gpio_no is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    I2cGpio.scl_gpio_no = scl_gpio_no;
    I2cGpio.sda_gpio_no = sda_gpio_no;
    I2cGpio.b_used = MT_TRUE;

    ret = ioctl(g_i2cdev_fd, CMD_I2C_CONFIG, &I2cGpio);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_I2C("  CreateGpioI2c number failed .\n");
        return ret;
    }

    *p_i2c_id = I2cGpio.i2c_id;

    return MT_SUCCESS;
}

/*******************************************
Function:       HI_UNF_I2C_DestroyGpioI2c
Description:    config param
Calls:          gpio_i2c_destroy
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:
return:         ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_i2c_destroy_gpioi2c(mt_u32 i2c_id)
{
	i2c_gpio_t I2cGpio;
    mt_s32 ret = 0;

    if ((MT_STD_I2C_NUM > i2c_id) || (MT_I2C_MAX_NUM <= i2c_id))
    {
        MT_ERR_I2C("para i2c_id = %d is invalid.\n", i2c_id);
        return MT_ERR_I2C_INVALID_PARA;
    }

    CHECK_I2C_OPEN();

    I2cGpio.i2c_id = i2c_id;

    I2cGpio.scl_gpio_no = 0;
    I2cGpio.sda_gpio_no = 0;
    I2cGpio.b_used = MT_FALSE;

    ret = ioctl(g_i2cdev_fd, CMD_I2C_DESTROY, &I2cGpio);
    if (ret != MT_SUCCESS)
    {
        return ret;
    }

    return MT_SUCCESS;
}

/*******************************************
Function:              HI_UNF_I2C_SetRate
Description:  set the rate
Calls:        HI_I2C_SetRate
Data Accessed:      NA
Data Updated:        NA
Input:                  NA
Output:                NA
return:         ErrorCode(reference to document)
Others:                  NA
*******************************************/
mt_s32 mt_unf_i2c_set_rate(mt_u32 i2c_id, mt_unf_i2c_rate_t i2c_rate)
{
    mt_s32 ret = 0;
    i2c_rate_t I2cRate;

    if (i2c_id >= MT_STD_I2C_NUM)
    {
        MT_ERR_I2C("para i2c_id is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    CHECK_I2C_OPEN();

    I2cRate.i2c_id = i2c_id;

    switch (i2c_rate)
    {
    case MT_UNF_I2C_RATE_10K:
        I2cRate.i2c_rate = 10000;
        break;
    case MT_UNF_I2C_RATE_50K:
        I2cRate.i2c_rate = 50000;
        break;
    case MT_UNF_I2C_RATE_100K:
        I2cRate.i2c_rate = 100000;
        break;
    case MT_UNF_I2C_RATE_200K:
        I2cRate.i2c_rate = 200000;
        break;
    case MT_UNF_I2C_RATE_300K:
        I2cRate.i2c_rate = 300000;
        break;
    case MT_UNF_I2C_RATE_400K:
        I2cRate.i2c_rate = 400000;
        break;
    default:
        MT_ERR_I2C("para i2c_rate is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    ret = ioctl(g_i2cdev_fd, CMD_I2C_SET_RATE, &I2cRate);
    if (ret != MT_SUCCESS)
    {
        return ret;
    }
    return MT_SUCCESS;
}

/*******************************************
Function:     mt_unf_i2c_set_rate_ex
Description:  set the rate
Calls:        mt_i2c_set_rate
Data Accessed:      NA
Data Updated:        NA
Input:                  NA
Output:                NA
return:         ErrorCode(reference to document)
Others:                  NA
*******************************************/
mt_s32 mt_unf_i2c_set_rate_ex(mt_u32 i2c_id, mt_u32 i2c_rate)
{
    mt_s32 ret = 0;
    i2c_rate_t I2cRate;

    if (i2c_id >= MT_STD_I2C_NUM)
    {
        MT_ERR_I2C("para i2c_id is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    if (i2c_rate > 400 * 1000)
    {
        MT_ERR_I2C("para i2c_rate is invalid.\n");
        return MT_ERR_I2C_INVALID_PARA;
    }

    CHECK_I2C_OPEN();

    I2cRate.i2c_id  = i2c_id;
    I2cRate.i2c_rate = i2c_rate;

    ret = ioctl(g_i2cdev_fd, CMD_I2C_SET_RATE, &I2cRate);
    if (ret != MT_SUCCESS)
    {
        return ret;
    }
    return MT_SUCCESS;
}


mt_s32 mt_unf_i2c_reset(mt_u32 id)
{
	mt_s32 ret = 0;
	mt_u32 i2c_id = 0;

	i2c_id = id;
	if (i2c_id > MT_I2C_MAX_NUM_USER)
	{
		MT_ERR_I2C("para i2c_id is %d invalid.\n",i2c_id);
		return MT_ERR_I2C_INVALID_PARA;
	}

	CHECK_I2C_OPEN();

	ret = ioctl(g_i2cdev_fd, CMD_I2C_RESET, &i2c_id);
	if (ret != MT_SUCCESS)
	{
		return ret;
	}
	return MT_SUCCESS;
}

