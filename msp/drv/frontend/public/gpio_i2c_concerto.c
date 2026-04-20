/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/kernel.h>
#include <linux/delay.h>
#include "mt_type.h"
#include "drv_gpio.h"
#include "mt_module_debug.h"

#define DEBUG_ON
#ifdef DEBUG_ON
#define DEBUG MT_INFO_FRONTEND
#else
#undef DEBUG
#define DEBUG(...) \
    do {           \
    } while (1)
#endif

/*!
    i2c gpio time out in ms
  */
#define I2C_GPIO_TIMES_OUT 10
/*!
    i2c gpio delay ms funtion
  */
#define I2C_GPIO_DELAY_MS mdelay

/*!
    i2c gpio delay us funtion
  */
#define I2C_GPIO_DELAY_US ndelay

/*!
    i2c gpio delay ns funtion
  */
#define I2C_GPIO_DELAY_NS ndelay

/*!
    i2c gpio private data
  */
typedef struct
{
    u32 clock_period;
    u32 sda;
    u32 scl;
} i2c_gpio_priv_t;

#define SET_SDA_OUT(priv) gpio_set_dir(priv->sda, GPIO_DIR_OUTPUT)
#define SET_SDA_IN(priv) gpio_set_dir(priv->sda, GPIO_DIR_INPUT)
#define SET_SDA_HI(priv) gpio_set_value(priv->sda, GPIO_VALUE_HIGH_LEVEL)
#define SET_SDA_LO(priv) gpio_set_value(priv->sda, GPIO_VALUE_LOW_LEVEL)
//#define GET_SDA(priv)           //HAL_GPIO_BIT_GET(i2c_gpio_reg[id].sda)
#define SET_SCL_OUT(priv) gpio_set_dir(priv->scl, GPIO_DIR_OUTPUT)
#define SET_SCL_IN(priv) gpio_set_dir(priv->scl, GPIO_DIR_INPUT)
#define SET_SCL_HI(priv) gpio_set_value(priv->scl, GPIO_VALUE_HIGH_LEVEL)
#define SET_SCL_LO(priv) gpio_set_value(priv->scl, GPIO_VALUE_LOW_LEVEL)
//#define GET_SCL(priv)         HAL_GPIO_BIT_GET(i2c_gpio_reg[id].scl)

static i2c_gpio_priv_t i2c_gpio_priv = { 0 };
inline u8 GET_SDA(i2c_gpio_priv_t *p_priv)
{
    return gpio_get_value(p_priv->sda);
}

inline u8 GET_SCL(i2c_gpio_priv_t *p_priv)
{
    return gpio_get_value(p_priv->scl);
}

/*---------------------------------------------------
int i2c_gpio_phase_start();
        Generate i2c_gpio_phase_start Condition:
        Stream Format:
                SCL   _____/--------\___
                SDA   =---------\_____
                width (4.7u)4.7u|4.7u|
        Arguments:
                NONE
        Return value:
                int SUCCESS                             0
                int ERR_STATUS  1
----------------------------------------------------*/
static RET_CODE i2c_gpio_phase_start(i2c_gpio_priv_t *p_priv)
{
    /* Make sure is out */
    SET_SDA_OUT(p_priv);
    SET_SCL_OUT(p_priv);

    SET_SDA_HI(p_priv); /* Set SDA high */
    if (0 == GET_SCL(p_priv)) {
	I2C_GPIO_DELAY_US(p_priv->clock_period);
    }

    SET_SCL_HI(p_priv); /* Set SCL high */
    I2C_GPIO_DELAY_US(p_priv->clock_period);
    if (0 == GET_SCL(p_priv)) {
	return ERR_STATUS;
    }

    if (0 == GET_SDA(p_priv)) {
	return ERR_STATUS;
    }

    SET_SDA_LO(p_priv);
    I2C_GPIO_DELAY_US(p_priv->clock_period);
    SET_SCL_LO(p_priv);

    return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_phase_stop(id);
        Generate i2c_gpio_phase_stop Condition:
        Stream Format:
                SCL   _____/-------------------------------
                SDA   __________/--------------------------
                width  4.7u|4.7u|4.7u from next i2c_gpio_phase_start bit
        Arguments:
                NONE
        Return value:
                int SUCCESS                             0
                int ERR_I2C_SCL_LOCK    1
                int ERR_I2C_SDA_LOCK    2
----------------------------------------------------*/
static RET_CODE i2c_gpio_phase_stop(i2c_gpio_priv_t *p_priv)
{
    /* Make sure is out */
    SET_SDA_OUT(p_priv);
    SET_SCL_OUT(p_priv);

    SET_SDA_LO(p_priv);
    I2C_GPIO_DELAY_US(p_priv->clock_period);
    SET_SCL_HI(p_priv);
    I2C_GPIO_DELAY_US(p_priv->clock_period);
    if (0 == GET_SCL(p_priv)) {
	return ERR_STATUS;
    }

    SET_SDA_HI(p_priv);
    I2C_GPIO_DELAY_US(2);
    if (0 == GET_SDA(p_priv)) {
	return ERR_STATUS;
    }

    return SUCCESS;
}

/*---------------------------------------------------
void i2c_gpio_phase_set_bit(id, int val);
        Set a BIT (Hi or Low)
        Stream Format:
                SCL   _____/---\
                SDA   ??AAAAAAAA
                width  4.7u| 4u|
        Arguments:
                int i   : Set(1) or Clear(0) this bit on iic bus
        Return value:
                NONE
----------------------------------------------------*/
static void i2c_gpio_phase_set_bit(i2c_gpio_priv_t *p_priv, int val)
{
    /* Make sure is out */
    SET_SDA_OUT(p_priv);
    SET_SCL_OUT(p_priv);

    SET_SCL_LO(p_priv);
    if (val) {
	SET_SDA_HI(p_priv);
    } else {
	SET_SDA_LO(p_priv);
    }
    I2C_GPIO_DELAY_US(p_priv->clock_period);

    SET_SCL_HI(p_priv);
    I2C_GPIO_DELAY_US(p_priv->clock_period);
    SET_SCL_LO(p_priv);

    return;
}

/*---------------------------------------------------
int i2c_gpio_phase_get_bit(id);
        Set a BIT (Hi or Low)
        Stream Format:
                SCL   _____/---\
                SDA   ??AAAAAAAA
                width  4.7u| 4u|
        Arguments:
                NONE
        Return value:
                int i   : Set(1) or Clear(0) this bit on iic bus
----------------------------------------------------*/
static int i2c_gpio_phase_get_bit(i2c_gpio_priv_t *p_priv)
{
    int ret = 0;

    SET_SDA_IN(p_priv);
    SET_SDA_HI(p_priv); /* Hi Ind */

    I2C_GPIO_DELAY_US(p_priv->clock_period);
    SET_SCL_HI(p_priv);
    I2C_GPIO_DELAY_US(p_priv->clock_period);
    ret = GET_SDA(p_priv);
    SET_SCL_LO(p_priv);

    return ret;
}

/*---------------------------------------------------
int i2c_gpio_phase_set_byte(UINT32 id, UINT8 data);
        Perform a byte write process
        Stream Format:
                SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\__/-\
                SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-Check
                Clock Low: 4.7u, High: 4.0u.                            Ack
        Data exchanged at CLK Low, ready at SCL High
        Arguments:
                char data       - Data to send on iic bus
        return value:
                The /ack signal returned from slave
----------------------------------------------------*/
static RET_CODE i2c_gpio_phase_set_byte(i2c_gpio_priv_t *p_priv, u8 data)
{
    int i = 0;

    for (i = 0; i < 8; i++) {
	if ((data & 0x80) == 0x80) {
	    i2c_gpio_phase_set_bit(p_priv, 1);
	} else {
	    i2c_gpio_phase_set_bit(p_priv, 0);
	}

	data <<= 1;
    }

    return (i2c_gpio_phase_get_bit(p_priv));
}

/*---------------------------------------------------
char i2c_gpio_phase_get_byte(UINT32 id, int ack);
        Perform a byte read process
                        by Charlemagne Yue
        SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\
        SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-(Ack)
        Clock Low: 4.7u, High: 4.0u.
    Data exchanged at CLK Low, ready at SCL High
----------------------------------------------------*/
static u8 i2c_gpio_phase_get_byte(i2c_gpio_priv_t *p_priv, int ack)
{
    u8 ret = 0;
    int i = 0;

    for (i = 0; i < 8; i++) {
	ret <<= 1;
	ret |= i2c_gpio_phase_get_bit(p_priv);
    }
    i2c_gpio_phase_set_bit(p_priv, ack);

    return ret;
}

/*---------------------------------------------------
INT32 i2c_gpio_read_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
        Perform bytes read process but no stop
        Stream Format:
                S<SLV_R><Read>
                S               : Start
                <SLV_R> : Set Slave addr & Read Mode
                <Read>  : Read Data
        Arguments:
                BYTE slv_addr - Slave Address
                BYTE reg_addr - Data address
        Return value:
                Data returned
----------------------------------------------------*/
static RET_CODE i2c_gpio_read_no_stop(i2c_gpio_priv_t *p_priv,
                                      u8 slv_addr, u8 *p_buf, int len)
{
    int i = I2C_GPIO_TIMES_OUT;

    slv_addr |= 1; /* Read */
    while (--i)    /* Ack polling !! */
    {
	i2c_gpio_phase_start(p_priv);
	/* has /ACK => i2c_gpio_phase_start transfer */
	if (!i2c_gpio_phase_set_byte(p_priv, slv_addr)) {
	    break;
	}
	/* device is busy, issue i2c_gpio_phase_stop and chack again later */
	i2c_gpio_phase_stop(p_priv);
	I2C_GPIO_DELAY_MS(1); /* wait for 1mS */
    }

    if (i == 0) {
	return ERR_TIMEOUT;
    }

    for (i = 0; i < (len - 1); i++) {
	/*with no /ack to stop process */
	p_buf[i] = i2c_gpio_phase_get_byte(p_priv, 0);
    }
    p_buf[len - 1] = i2c_gpio_phase_get_byte(p_priv, 1);

    return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
        Perform bytes write process but no stop
        Stream Format:
                S<SLV_W><Write>
                S               : Start
                <SLV_W> : Set Slave addr & Write Mode
                <Write> : Send Data
        Arguments:
                BYTE slv_addr - Slave Address
                BYTE value    - data to write
        Return value:
                NONE
----------------------------------------------------*/
static RET_CODE i2c_gpio_write_no_stop(i2c_gpio_priv_t *p_priv,
                                       u8 slv_addr, u8 *p_buf, int len)
{
    int i = I2C_GPIO_TIMES_OUT;

    slv_addr &= 0xFE; /*Write*/
    while (--i)       /* Ack polling !! */
    {
	i2c_gpio_phase_start(p_priv);
	/* has /ACK => i2c_gpio_phase_start transfer */
	if (0 == i2c_gpio_phase_set_byte(p_priv, slv_addr)) {
	    break;
	}
	/* device is busy, issue i2c_gpio_phase_stop and chack again later */
	i2c_gpio_phase_stop(p_priv);
	I2C_GPIO_DELAY_MS(1); /* wait for 1mS */
    }

    if (i == 0) {
	return ERR_TIMEOUT;
    }

    for (i = 0; i < len; i++) {
	i2c_gpio_phase_set_byte(p_priv, p_buf[i]);
    }

    return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_read(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
        Perform a byte read process
        Stream Format:
                S<SLV_R><Read>P
                S               : Start
                P               : Stop
                <SLV_R> : Set Slave addr & Read Mode
                <Read>  : Read Data
        Arguments:
                BYTE slv_addr - Slave Address
                BYTE reg_addr - Data address
        Return value:
                Data returned
----------------------------------------------------*/
RET_CODE i2c_gpio_read(
    u8 slv_addr, u8 *p_buf, u32 size, u32 param)
{
    RET_CODE ret = 0;
    i2c_gpio_priv_t *p_priv = (i2c_gpio_priv_t *)&i2c_gpio_priv;

    if ((ret = i2c_gpio_read_no_stop(p_priv, slv_addr, p_buf, size)) != SUCCESS) {
	DEBUG("@@i2c read error!\n");
	return ret;
    }
    i2c_gpio_phase_stop(p_priv);

    return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write(UINT8 slv_addr, UINT8 *data, UINT32 len);
        Perform bytes write process
        Stream Format:
                S<SLV_W><Write>P
                S               : Start
                P               : Stop
                <SLV_W> : Set Slave addr & Write Mode
                <Write> : Send Data
        Arguments:
                BYTE slv_addr - Slave Address
                BYTE value    - data to write
        Return value:
                NONE
----------------------------------------------------*/
RET_CODE i2c_gpio_write(
    u8 slv_addr, u8 *p_buf, u32 size, u32 param)
{
    RET_CODE ret = 0;

    i2c_gpio_priv_t *p_priv = (i2c_gpio_priv_t *)&i2c_gpio_priv;
    if ((ret = i2c_gpio_write_no_stop(p_priv, slv_addr, p_buf, size)) != SUCCESS) {
	DEBUG("@@i2c write error!\n");
	return ret;
    }
    i2c_gpio_phase_stop(p_priv);

    return SUCCESS;
}

RET_CODE i2c_gpio_seq_read(
    u8 slv_addr,
    u8 *p_buf,
    u32 wlen,
    u32 rlen,
    u32 param)
{
    RET_CODE ret = 0;
    i2c_gpio_priv_t *p_priv = (i2c_gpio_priv_t *)&i2c_gpio_priv;

    if (wlen == 0) {
	return i2c_gpio_read(slv_addr, p_buf, rlen, param);
    }

    ret = i2c_gpio_write_no_stop(p_priv, slv_addr, p_buf, wlen);
    if (SUCCESS != ret) {
	DEBUG("@@i2c seq_read error1!\n");
	return ret;
    }

    ret = i2c_gpio_read_no_stop(p_priv, slv_addr, p_buf, rlen);
    if (SUCCESS != ret) {
	DEBUG("@@i2c seq_read error2!\n");
	return ret;
    }

    i2c_gpio_phase_stop(p_priv);

    return SUCCESS;
}

RET_CODE i2c_gpio_std_read(
    u8 slv_addr,
    u8 *p_buf,
    u32 wlen,
    u32 rlen,
    u32 param)
{
    RET_CODE ret = 0;
    i2c_gpio_priv_t *p_priv = (i2c_gpio_priv_t *)&i2c_gpio_priv;
    if (wlen == 0) {
	return i2c_gpio_read(slv_addr, p_buf, rlen, param);
    }

    ret = i2c_gpio_write(slv_addr, p_buf, wlen, param);
    if (SUCCESS != ret) {
	DEBUG("@@i2c seq_read error1!\n");
	return ret;
    }

    ret = i2c_gpio_read_no_stop(p_priv, slv_addr, p_buf, rlen);
    if (SUCCESS != ret) {
	DEBUG("@@i2c seq_read error2!\n");
	return ret;
    }

    i2c_gpio_phase_stop(p_priv);

    return SUCCESS;
}

RET_CODE i2c_gpio_open(u32 bus_clk_khz)
{
    i2c_gpio_priv_t *p_priv = (i2c_gpio_priv_t *)&i2c_gpio_priv;
    unsigned long reg32 = 0;

    p_priv->clock_period = 1000000 / (2 * bus_clk_khz); //500000/1000/cfg->bus_clk_khz
    p_priv->scl = 52;
    p_priv->sda = 51;

    reg32 = *(unsigned long *)0xbf156018;
    reg32 &= ~(0xff << 20);
    reg32 |= 2 << 20;
    reg32 |= 2 << 22;
    *(unsigned long *)0xbf156018 = reg32;

    reg32 = *(unsigned long *)0xbf156308;
    reg32 |= 3 << 28;
    *(unsigned long *)0xbf156308 = reg32;

    SET_SDA_HI(p_priv);
    SET_SCL_HI(p_priv);
    SET_SCL_OUT(p_priv);

    return SUCCESS;
}
