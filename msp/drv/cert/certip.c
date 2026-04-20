/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include "certip.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#define SECHD0_BASE_ADDRESS SYMPHONY_IO_VA(0xbf30c000)
#else
#define SECHD0_BASE_ADDRESS (0xbf30c000)
#endif

#define AKL_DATA_IN_0 (SECHD0_BASE_ADDRESS + 0x00)
#define AKL_DATA_IN_1 (SECHD0_BASE_ADDRESS + 0x04)
#define AKL_DATA_IN_2 (SECHD0_BASE_ADDRESS + 0x08)
#define AKL_DATA_IN_3 (SECHD0_BASE_ADDRESS + 0x0c)
#define AKL_DATA_IN_4 (SECHD0_BASE_ADDRESS + 0x10)
#define AKL_DATA_IN_5 (SECHD0_BASE_ADDRESS + 0x14)
#define AKL_DATA_IN_6 (SECHD0_BASE_ADDRESS + 0x18)
#define AKL_DATA_IN_7 (SECHD0_BASE_ADDRESS + 0x1c)
#define AKL_DATA_OUT_0 (SECHD0_BASE_ADDRESS + 0x20)
#define AKL_DATA_OUT_1 (SECHD0_BASE_ADDRESS + 0x24)
#define AKL_DATA_OUT_2 (SECHD0_BASE_ADDRESS + 0x28)
#define AKL_DATA_OUT_3 (SECHD0_BASE_ADDRESS + 0x2c)
#define AKL_DATA_OUT_4 (SECHD0_BASE_ADDRESS + 0x30)
#define AKL_DATA_OUT_5 (SECHD0_BASE_ADDRESS + 0x34)
#define AKL_DATA_OUT_6 (SECHD0_BASE_ADDRESS + 0x38)
#define AKL_DATA_OUT_7 (SECHD0_BASE_ADDRESS + 0x3c)
#define AKL_STATUS (SECHD0_BASE_ADDRESS + 0x40)
#define AKL_COMMAND (SECHD0_BASE_ADDRESS + 0x44)
#define AKL_INTERRUPT (SECHD0_BASE_ADDRESS + 0x48)

#define SECHD0_CFG_CONFIG_REG (SECHD0_BASE_ADDRESS + 0x100)
#define SECHD0_SOC_UID_31T0_REG (SECHD0_BASE_ADDRESS + 0x104)
#define SECHD0_SOC_UID_63T32_REG (SECHD0_BASE_ADDRESS + 0x108)
#define SECHD0_CFG_CODE_VERSIONING_REG (SECHD0_BASE_ADDRESS + 0x10c)
#define SECHD0_CFG_MSID_REG (SECHD0_BASE_ADDRESS + 0x110)
#define SECHD0_CFG_STB_CA_SN_REG (SECHD0_BASE_ADDRESS + 0x114)
#define SECHD0_CFG_STATE_REG (SECHD0_BASE_ADDRESS + 0x118)
#define SECHD0_CFG_RAM_ENC_ENABLE_REG (SECHD0_BASE_ADDRESS + 0x11c)
#define SECHD0_CFG_FLASH_ENC_ENABLE_REG (SECHD0_BASE_ADDRESS + 0x120)
#define SECHD0_OTP_SLOT_LOCK_REG (SECHD0_BASE_ADDRESS + 0x200)
#define SECHD0_KEYDELIVERY_START_REG (SECHD0_BASE_ADDRESS + 0x300)
#define SECHD0_SECHD0_KEY_ACK_REG (SECHD0_BASE_ADDRESS + 0x304)
#define SECHD0_KEYDELIVERY_INFO_REG (SECHD0_BASE_ADDRESS + 0x308)
#define SECHD0_SECHD0_KEY_USAGE_REG (SECHD0_BASE_ADDRESS + 0x30c)
#define SECHD0_SECHD0_INFO_REG (SECHD0_BASE_ADDRESS + 0x310)
#define SECHD0_SLOW_CLOCK_DETECTED_REG (SECHD0_BASE_ADDRESS + 0x400)
#define SECHD0_SECHD0_CLKCHK_CNTMAX_REG (SECHD0_BASE_ADDRESS + 0x404)

#define AKL_STATUS_ERROR_FLAG (1 << 0)
#define AKL_STATUS_KEY_OUTPUT_INTERFACE_INFO (1 << 1)
#define AKL_STATUS_OTP_RECORD0_LOCK_INFO (1 << 2)
#define AKL_STATUS_OTP_RECORD0_PROGRAMMING_INFO (1 << 3)
#define AKL_STATUS_OTP_RECORD0_AUTHENTICATION_INFO (1 << 4)
#define AKL_STATUS_OTP_RECORD1_LOCK_INFO (1 << 5)
#define AKL_STATUS_OTP_RECORD1_PROGRAMMING_INFO (1 << 6)
#define AKL_STATUS_OTP_RECORD1_AUTHENTICATION_INFO (1 << 7)
#define AKL_STATUS_OTP_RECORD2_LOCK_INFO (1 << 8)
#define AKL_STATUS_OTP_RECORD2_PROGRAMMING_INFO (1 << 9)
#define AKL_STATUS_OTP_RECORD2_AUTHENTICATION_INFO (1 << 10)
#define AKL_STATUS_OTP_RECORD3_LOCK_INFO (1 << 11)
#define AKL_STATUS_OTP_RECORD3_PROGRAMMING_INFO (1 << 12)
#define AKL_STATUS_OTP_RECORD3_AUTHENTICATION_INFO (1 << 13)
#define AKL_STATUS_OTP_PROGRAMMING_ERROR (1 << 14)
#define AKL_STATUS_OTP_READ_ERROR (1 << 15)

#define CERTIP_CMD_TIMEOUT_DEFAULT 5000
#define CERTIP_CMD_TIMEOUT_OTP_WRITE 5500

//#define AKL_DEBUG
#ifdef AKL_DEBUG
#define akl_print printk
#else
#define akl_print(x...) \
    do {                \
    } while (0);
#endif

typedef enum {
    CERT_NO_ERROR,
    CERT_ERROR_BAD_HANDLE,
    CERT_ERROR_BAD_EMI,
    CERT_ERROR_BAD_USAGE,
    CERT_ERROR_TIMEOUT,
    CERT_ERROR,
    CERT_DRV_ERROR,
    LAST_CERT_STATUS
} TCERTSTATUS;

#if 0
static void akl_hex_dump(char *message, unsigned char *buffer, int len)
{
    int i = 0;
    if(message)
    {
        akl_print("%s:", message);
    }
    for(i=0;i<len;i++)
    {
        akl_print("%02X", buffer[i]);
    }
    akl_print("\n");
}
#endif

static int big_endian_register_read(volatile unsigned int *address, unsigned char *output, int size)
{
    int i = 0;
    volatile unsigned int reg = 0;
    for (i = 0; i < size; i += 4) {
        reg = address[i / 4];
        output[i + 0] = (reg >> 24) & 0xFF;
        output[i + 1] = (reg >> 16) & 0xFF;
        output[i + 2] = (reg >> 8) & 0xFF;
        output[i + 3] = (reg >> 0) & 0xFF;
    }
    return 0;
}

static int big_endian_register_write(volatile unsigned int *address, unsigned char *input, int size)
{
    int i = 0;
    for (i = 0; i < size; i += 4) {
        address[i / 4] = ((unsigned int)input[i + 0] << 24)
            | ((unsigned int)input[i + 1] << 16)
            | ((unsigned int)input[i + 2] << 8)
            | input[i + 3];
        //akl_print("address %08x, input %08x, result %08x\n", (int)&address[i/4], ((unsigned int)input[i+0] << 24) | ((unsigned int)input[i+1] << 16) | ((unsigned int)input[i+2] << 8) | input[i+3], address[i/4]);
    }
    return 0;
}

#if 0
static void certip_delay_function(unsigned int ms)
{
    msleep(ms);
}
#endif

static int certip_wait_cmd_done(unsigned int ms)
{
    int ret = CERT_ERROR_TIMEOUT;

    unsigned long timeout = jiffies + HZ * (ms / 1000); /* vpu wait timeout to 1sec */

    while (!time_after(jiffies, timeout)) {
        if ((HAL_GET_U32((volatile u32 *)(AKL_COMMAND)) & 0x01) == 0) {
            ret = CERT_NO_ERROR;
            break;
        }
    }
#if 0
    for(i=0;i<ms;i++)
    {
        //akl_print("cmd = %08x\n", HAL_GET_U32((volatile u32 *)(AKL_COMMAND)));
        certip_delay_function(1);
        if((HAL_GET_U32((volatile u32 *)(AKL_COMMAND)) & 0x01) == 0)
        {
            akl_print("ms = %d\n", i);
            ret = CERT_NO_ERROR;
            break;
        }
    }
#endif
    if (ret == CERT_ERROR_TIMEOUT) {
        akl_print("cert timeout\n");
    }
    return ret;
}

int certip_reset(void)
{
    volatile unsigned int rdata;

#if defined(CONFIG_MT_CHIP_SYMPHONY2)
    rdata = HAL_GET_U32((volatile u32 *)(0xbf30f0e0));

    rdata = rdata & (~(0x1 << 10));
    HAL_PUT_U32((volatile u32 *)(0xbf30f0e0), rdata);

    rdata = rdata | (0x1 << 10);
    HAL_PUT_U32((volatile u32 *)(0xbf30f0e0), rdata);

#elif defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    rdata = HAL_GET_U32((volatile u32 *)(SYMPHONY_IO_VA(0xbf50c00c)));

    rdata = rdata & (~(0x1 << 1));
    HAL_PUT_U32((volatile u32 *)(SYMPHONY_IO_VA(0xbf50c00c)), rdata);

    rdata = rdata | (0x1 << 1);
    HAL_PUT_U32((volatile u32 *)(SYMPHONY_IO_VA(0xbf50c00c)), rdata);
#endif

    return certip_wait_cmd_done(CERTIP_CMD_TIMEOUT_OTP_WRITE);
}

int certip_exchange(unsigned int cmds_num, certip_command_s *p_cmds,
    unsigned int *p_processed_num, int *cert_status)
{
    int ret = 0;
    int ret_cert = 0;
    unsigned long bytes_not_copy = 0;
    unsigned int i = 0;
    unsigned int timeout = 0;
    certip_command_s *cmds_in = NULL;

    if (cmds_num == 0 || p_cmds == NULL
        || p_processed_num == NULL || cert_status == NULL) {
        return -1;
    }

    cmds_in = (certip_command_s *)vmalloc(sizeof(certip_command_s) * cmds_num);
    if (cmds_in == NULL) {
        *cert_status = CERT_DRV_ERROR;
        return -1;
    }
    
    bytes_not_copy = copy_from_user(cmds_in, p_cmds, sizeof(certip_command_s) * cmds_num);
    if (bytes_not_copy > 0) {
        *cert_status = CERT_DRV_ERROR;
        vfree(cmds_in);
        return -1;
    }

    while (i < cmds_num) {
        big_endian_register_write((volatile unsigned int *)AKL_DATA_IN_0,
            (unsigned char *)cmds_in[i].input, 32);
        big_endian_register_write((volatile unsigned int *)AKL_COMMAND,
            (unsigned char *)cmds_in[i].opcodes, 4);
        timeout = cmds_in[i].timeout == CERTIP_TIMEOUT_OTP ?
            CERTIP_CMD_TIMEOUT_OTP_WRITE : CERTIP_CMD_TIMEOUT_DEFAULT;

        ret_cert = certip_wait_cmd_done(timeout);
        if (ret_cert == CERT_ERROR_TIMEOUT) {
            *cert_status = ret_cert;
            break;
        }
        big_endian_register_read((volatile unsigned int *)AKL_DATA_OUT_0,
            (unsigned char *)cmds_in[i].output, 32);
        big_endian_register_read((volatile unsigned int *)AKL_STATUS,
            (unsigned char *)cmds_in[i].status, 4);
        copy_to_user(p_cmds[i].output, cmds_in[i].output, 32);
        copy_to_user(p_cmds[i].status, cmds_in[i].status, 4);

        if (cmds_in[i].status[3] & AKL_STATUS_ERROR_FLAG) {
            i++;
            *cert_status = CERT_ERROR;
            break;
        }
        i++;
    }
    *p_processed_num = i;

    vfree(cmds_in);

    return ret;
}

int certip_output_key(unsigned int slot_id, unsigned int ext_attr)
{
    int ret = CERT_ERROR;
    unsigned char status[4] = { 0 };

    if ((HAL_GET_U32((volatile u32 *)(SECHD0_SECHD0_INFO_REG)) & 0x1) == 0) {
        big_endian_register_read((volatile unsigned int *)AKL_STATUS, (unsigned char *)status, 4);
        akl_print("no key valid, akl status %02X%02X%02X%02X\n",
            status[0], status[1], status[2], status[3]);
        return ret;
    }

    HAL_PUT_U32((volatile u32 *)(SECHD0_KEYDELIVERY_INFO_REG),
        (slot_id & 0xFF) | ((ext_attr << 8) & 0x3F));
    HAL_PUT_U32((volatile u32 *)(SECHD0_KEYDELIVERY_START_REG), 1);
    while ((HAL_GET_U32((volatile u32 *)(SECHD0_KEYDELIVERY_START_REG)) & 0x1) == 1)
        ;

    return CERT_NO_ERROR;
}

int certip_key_ack(void)
{
    unsigned char status[4] = { 0 };

    if ((HAL_GET_U32((volatile u32 *)(SECHD0_SECHD0_INFO_REG)) & 0x1) == 0) {
        big_endian_register_read((volatile unsigned int *)AKL_STATUS, (unsigned char *)status, 4);
        akl_print("no key valid, akl status %02X%02X%02X%02X\n",
            status[0], status[1], status[2], status[3]);
        return CERT_ERROR;
    }

    HAL_PUT_U32((volatile u32 *)(SECHD0_SECHD0_KEY_ACK_REG), 1);

    return CERT_NO_ERROR;
}
