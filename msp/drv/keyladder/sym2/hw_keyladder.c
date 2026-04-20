/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/string.h>
#include <linux/printk.h>
#include "hw_keyladder_bare_api.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

//register address defines
#define KEYLADDER_CW_REG_BASE                                   0xbf30b000
#define KEYLADDER_PVR_REG_BASE                                  0xbf30b800

#define KEYLADDER_REG_CMD_TRI                                (handle->base_address + 0x0)
#define KEYLADDER_REG_KEYTB_SLOT_INDEX                       (handle->base_address + 0x4)
#define KEYLADDER_REG_STATE_CHECK                            (handle->base_address + 0x8)
#define KEYLADDER_REG_CPU0                                   (handle->base_address + 0x10)
#define KEYLADDER_REG_CPU1                                   (handle->base_address + 0x20)
#define KEYLADDER_REG_CPU2                                   (handle->base_address + 0x30)
#define KEYLADDER_REG_CPU3                                   (handle->base_address + 0x40)
#define KEYLADDER_REG_CPU4                                   (handle->base_address + 0x50)
#define KEYLADDER_REG_CPU5                                   (handle->base_address + 0x60)
#define KEYLADDER_REG_ACPU                                   (handle->base_address + 0x70)
#define KEYLADDER_REG_SCPU                                   (handle->base_address + 0x80)
#define KEYLADDER_REG_SIGNATURE                                 (handle->base_address + 0x90)
#define KEYLADDER_REG_RAM_CMD                                (handle->base_address + 0x400)

//key ladder commands list
#define CMD_FUNC_SELECT_SCK                                        (0x1 << 13)
#define CMD_FUNC_MOVE                                                  (0x2 << 13)
#define CMD_FUNC_STORE_TO_PRIVATE                            (0x5 << 13)
#define CMD_FUNC_EXPORT_KEY                                       (0x7 << 13)
#define CMD_FUNC_MASK                                                  (0x7 << 13)

//key ladder export command parameters
#define CMD_EXPORT_SRC_AES_DOUT                              ((KL_EXPORT_SRC_AES_DOUT+0x18)<<8)
#define CMD_EXPORT_SRC_TDES_DOUT                            ((KL_EXPORT_SRC_TDES_DOUT+0x18)<<8)
#define CMD_EXPORT_SRC_HASH_DOUT_L                        ((KL_EXPORT_SRC_HASH_DOUT_L+0x18)<<8)
#define CMD_EXPORT_SRC_HASH_DOUT_H                       ((KL_EXPORT_SRC_HASH_DOUT_H+0x18)<<8)
#define CMD_EXPORT_SRC_XOR_DOUT                              ((KL_EXPORT_SRC_XOR_DOUT+0x18)<<8)
#define CMD_EXPORT_SRC_SCK_SELECTED                        ((KL_EXPORT_SRC_SCK_SELECTED+0x18)<<8)

//key ladder restore command parameters
#define CMD_RESTORE_SRC_AES_DOUT                              ((KL_STORE_SRC_AES_DOUT+0x10)<<8)
#define CMD_RESTORE_SRC_TDES_DOUT                            ((KL_STORE_SRC_TDES_DOUT+0x10)<<8)
#define CMD_RESTORE_SRC_HASH_DOUT_L                        ((KL_STORE_SRC_HASH_DOUT_L+0x10)<<8)
#define CMD_RESTORE_SRC_HASH_DOUT_H                       ((KL_STORE_SRC_HASH_DOUT_H+0x10)<<8)
#define CMD_RESTORE_SRC_XOR_DOUT                              ((KL_STORE_SRC_XOR_DOUT+0x10)<<8)

//key ladder move command parameters
#define CMD_MOVE_SRC_SCK_SELECTED                          (KL_MOVE_SRC_SCK_SELECTED << 8)
#define CMD_MOVE_SRC_AES_DOUT                                (KL_MOVE_SRC_AES_DOUT << 8)
#define CMD_MOVE_SRC_TDES_DOUT                              (KL_MOVE_SRC_TDES_OUT << 8)
#define CMD_MOVE_SRC_XOR_DOUT                               (KL_MOVE_SRC_XOR_DOUT << 8)
#define CMD_MOVE_SRC_HASH_DOUT_L                          (KL_MOVE_SRC_HASH_DOUT_L << 8)
#define CMD_MOVE_SRC_HASH_DOUT_H                          (KL_MOVE_SRC_HASH_DOUT_H << 8)
#define CMD_MOVE_SRC_REG_CPU(x)                              ((KL_MOVE_SRC_REG_CPU0+(x)) << 8)
#define CMD_MOVE_DST_AES_KEY                                   (KL_MOVE_DST_AES_KEY << 3)
#define CMD_MOVE_DST_AES_DIN                                  (KL_MOVE_DST_AES_DIN << 3)
#define CMD_MOVE_DST_TDES_KEY                                 (KL_MOVE_DST_TDES_KEY << 3)
#define CMD_MOVE_DST_TDES_DIN                                (KL_MOVE_DST_TDES_DIN << 3)
#define CMD_MOVE_DST_XOR_DIN_A                              (KL_MOVE_DST_XOR_DIN_A << 3)
#define CMD_MOVE_DST_XOR_DIN_B                              (KL_MOVE_DST_XOR_DIN_B << 3)
#define CMD_MOVE_DST_HASH_DIN_L                            (KL_MOVE_DST_HASH_DIN_L << 3)
#define CMD_MOVE_DST_HASH_DIN_H                           (KL_MOVE_DST_HASH_DIN_H << 3)
#define CMD_ADDS_MOVE                                               (KL_ADDS_MOVE)
#define CMD_ADDS_MOVE_AND_TRIGGER                      (KL_ADDS_MOVE_AND_TRIGGER)

#define CMD_EXEC_BUSY                                                  (0x1 << 31)
#define CMD_UNKONOW_FLAG                                        (0x1 << 11)
#define CMD_FORBIDDEN_FLAG                                       (0x1 << 10)
#define CMD_ERROR_CODE(x)                                           (((x)>>2) & 0xff)
#define CMD_HASH_ERR(x)                                                (((x)>>1) & 0x1)
#define CMD_ENDIAN_ERR(x)                                            (((x)>>0) & 0x1)

#define KEYLADDER_CMD_RAM_SIZE                                64

//#define KEYLADDER_DISABLE_CW_KL

#define SUCCESS             ((int) 0)       /* Success return */
#define ERR_FAILURE         ((int)-1)       /* Fail for common reason */
#define ERR_TIMEOUT         ((int)-2)       /* Fail for waiting timeout */

#define KL_CMD_DEBUG
#ifdef KL_CMD_DEBUG
#define cmd_print printk
#else
#define cmd_print(x...) do{}while(0);
#endif

typedef struct _keyladder_handle_t
{
    KEYLADDER_TYPE_E type;
    unsigned long key_pos;
    unsigned long data_pos;
    unsigned long base_address;
    unsigned long cmd_cnt;
    unsigned char signature[16];
    unsigned short cmd_ram[KEYLADDER_CMD_RAM_SIZE];
}kl_handle_t;

#ifndef KEYLADDER_DISABLE_CW_KL
static kl_handle_t kl_handle_cw;
#endif
static kl_handle_t kl_handle_pvr;

extern void kl_delay_function(unsigned long ms);

static void memcpy_from_reg(unsigned char *dest, unsigned char *reg, int len)
{
    int i = 0;
    unsigned long value = 0;
    for(i=0;i<len/4;i++)
    {
		value = HAL_GET_U32((volatile u32 *)(reg+i*4));
        dest[i*4+0] = (value >> 0) & 0xFF;
        dest[i*4+1] = (value >> 8) & 0xFF;
        dest[i*4+2] = (value >> 16) & 0xFF;
        dest[i*4+3] = (value >> 24) & 0xFF;
    }
}

static void memcpy_to_reg(unsigned char *reg, unsigned char *src, int len)
{
    int i = 0;
    unsigned long value = 0;
    for(i=0;i<len/4;i++)
    {
        value = ((unsigned long)src[i*4+0] << 0) | ((unsigned long)src[i*4+1] << 8) | ((unsigned long)src[i*4+2] << 16) | ((unsigned long)src[i*4+3] << 24);
		HAL_PUT_U32((volatile u32 *)(reg+i*4), value);
    }
}

static unsigned char kl_cmd_parity(unsigned short cmd)
{
    unsigned char parity = 0;
    int i = 0;
    for(i=0;i<16;i++)
    {
        if(cmd & (1<<i))
        {
            parity++;
        }
    }
    return (parity & 0x1);
}

static int kl_cmd_ram_init(kl_handle_t *handle)
{
    handle->cmd_cnt = 0;
    memset(handle->cmd_ram, 0, KEYLADDER_CMD_RAM_SIZE*sizeof(unsigned short));
    return SUCCESS;
}

static int kl_exec_cmd(kl_handle_t *handle)
{
    unsigned long i = 0;
    unsigned long cmd = 0;
    unsigned short *p_input_cmd = NULL;
    unsigned char parity = 0;
    p_input_cmd = handle->cmd_ram;

    for(i=0; i<KEYLADDER_CMD_RAM_SIZE; i++)
    {
        parity = kl_cmd_parity(p_input_cmd[i]);
        //cmd_print("%02x%02x", (p_input_cmd[i] & 0xFF), ((p_input_cmd[i]>>8) & 0xFF));
        *(volatile unsigned long *)(KEYLADDER_REG_RAM_CMD+i*4) = ((unsigned long)p_input_cmd[i]<<2) | (parity << 1) | ((~parity)&0x1);
        //cmd_print("%08x = %08x\n", KEYLADDER_REG_RAM_CMD+i*4, p_input_cmd[i]);
    }
    memcpy_to_reg((unsigned char *)KEYLADDER_REG_SIGNATURE, handle->signature, 16);
    cmd = 0xF | ((handle->cmd_cnt-1) << 24) | (7 << 4);
    *(volatile unsigned long *)KEYLADDER_REG_CMD_TRI = cmd;
    //cmd_print("TRI %08x= %08x\n", KEYLADDER_REG_CMD_TRI, cmd);
    //cmd_print("\n");
    return SUCCESS;
}

static int kl_set_cmd(kl_handle_t *handle, unsigned short cmd)
{
    if(handle->cmd_cnt < KEYLADDER_CMD_RAM_SIZE)
    {
        handle->cmd_ram[handle->cmd_cnt] = cmd;
        handle->cmd_cnt++;

        if(((cmd & CMD_FUNC_MASK) == CMD_FUNC_STORE_TO_PRIVATE) || ((cmd & CMD_FUNC_MASK) == CMD_FUNC_EXPORT_KEY))
        {
            return kl_exec_cmd(handle);
        }
        return SUCCESS;
    }

    return ERR_FAILURE;
}


static int kl_wait_cmd_timeout(unsigned long state_reg, unsigned long *p_state)
{
    unsigned long cnt = 0;
    unsigned long state = 0;
    do
    {
        state = *((volatile unsigned long *)state_reg);
        kl_delay_function(10);
    }
    while((state & CMD_EXEC_BUSY) && (cnt++ < 300));
    if(cnt >= 300)
    {
        cmd_print("cmd overtime, check your cmd!\n");
        return ERR_FAILURE;
    }
    *p_state = state;
    return SUCCESS;
}
static int kl_set_input(kl_handle_t *handle, unsigned char *p_input)
{
    if((p_input != NULL) && (handle->data_pos < 6))
    {
        memcpy_to_reg((unsigned char *)(KEYLADDER_REG_CPU0+(handle->data_pos<<4)), p_input, 16);
        handle->data_pos++;
        return SUCCESS;
    }
    return ERR_FAILURE;
}

static int kl_temporary_save_key(kl_handle_t *handle)
{
    unsigned short cmd = 0;
    unsigned char zero_input[16];

    if(handle->key_pos == CMD_MOVE_SRC_XOR_DOUT)
    {
        return ERR_FAILURE;
    }

    memset(zero_input, 0, 16);
    if(kl_set_input(handle, zero_input) != SUCCESS)
    {
        return ERR_FAILURE;
    }

    cmd = CMD_FUNC_MOVE|handle->key_pos|CMD_MOVE_DST_XOR_DIN_A|CMD_ADDS_MOVE;
    kl_set_cmd(handle, cmd);

    cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_REG_CPU(handle->data_pos-1)|CMD_MOVE_DST_XOR_DIN_B|CMD_ADDS_MOVE_AND_TRIGGER;
    kl_set_cmd(handle, cmd);

    handle->key_pos = CMD_MOVE_SRC_XOR_DOUT;

    return SUCCESS;
}

int bare_kl_init(void **p_bhandle, KEYLADDER_TYPE_E type)
{
#ifndef KEYLADDER_DISABLE_CW_KL
    if(type == KL_TYPE_0)
    {
        memset(&kl_handle_cw, 0, sizeof(kl_handle_t));
        kl_handle_cw.type = type;
        kl_handle_cw.base_address = KEYLADDER_CW_REG_BASE;
        *p_bhandle = (void *)&kl_handle_cw;
    }
    else
#endif
    {
        memset(&kl_handle_pvr, 0, sizeof(kl_handle_t));
        kl_handle_pvr.type = type;
        kl_handle_pvr.base_address = KEYLADDER_PVR_REG_BASE;
        *p_bhandle = (void *)&kl_handle_pvr;
    }

    return SUCCESS;
}

int bare_kl_set_signature(void *bhandle, unsigned char *p_signature)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    memcpy(handle->signature, p_signature, 16);
    return SUCCESS;
}

int bare_kl_select_rootkey(void *bhandle, KL_SCK_SOURCE_E rootkey_source)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = CMD_FUNC_SELECT_SCK | (rootkey_source<<8) | (((~rootkey_source) & 0x1f)<<3) | 0x1;

    kl_cmd_ram_init(handle);

    kl_set_cmd(handle, cmd);
    handle->data_pos = 0;
    handle->key_pos = CMD_MOVE_SRC_SCK_SELECTED;
    return SUCCESS;
}

int bare_kl_link_aes(void *bhandle, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = 0;

    //set input to reg_cpu0(fixed)
    if(kl_set_input(handle, p_input) != SUCCESS)
    {
        return ERR_FAILURE;
    }
    //set input to aes_din
    cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_REG_CPU(handle->data_pos-1)|CMD_MOVE_DST_AES_DIN|CMD_ADDS_MOVE;
    kl_set_cmd(handle, cmd);

    //set aes_key
    cmd = CMD_FUNC_MOVE|handle->key_pos|CMD_MOVE_DST_AES_KEY|CMD_ADDS_MOVE_AND_TRIGGER;
    kl_set_cmd(handle, cmd);

    //update aes_out as key for next level
    handle->key_pos = CMD_MOVE_SRC_AES_DOUT;
    return SUCCESS;
}

int bare_kl_link_tdes(void *bhandle, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = 0;

    if(kl_set_input(handle, p_input) != SUCCESS)
    {
        return ERR_FAILURE;
    }
    //set input to tdes_int
    cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_REG_CPU(handle->data_pos-1)|CMD_MOVE_DST_TDES_DIN|CMD_ADDS_MOVE;
    kl_set_cmd(handle, cmd);

    //set tdes_key
    cmd = CMD_FUNC_MOVE|handle->key_pos|CMD_MOVE_DST_TDES_KEY|CMD_ADDS_MOVE_AND_TRIGGER;
    kl_set_cmd(handle, cmd);

    handle->key_pos = CMD_MOVE_SRC_TDES_DOUT;
    return SUCCESS;
}

int bare_kl_link_xor(void *bhandle, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = 0;

    if(kl_set_input(handle, p_input) != SUCCESS)
    {
        return ERR_FAILURE;
    }
    cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_REG_CPU(handle->data_pos-1)|CMD_MOVE_DST_XOR_DIN_A|CMD_ADDS_MOVE;
    kl_set_cmd(handle, cmd);

    cmd = CMD_FUNC_MOVE|handle->key_pos|CMD_MOVE_DST_XOR_DIN_B|CMD_ADDS_MOVE_AND_TRIGGER;
    kl_set_cmd(handle, cmd);

    //update the xor_dout as key_source for next level
    handle->key_pos = CMD_MOVE_SRC_XOR_DOUT;
    return SUCCESS;
}

int bare_kl_link_hash(void *bhandle, unsigned char *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = 0;
    unsigned long cmd_hash_pos = 0;

    if(kl_set_input(handle, p_input) != SUCCESS)
    {
        return ERR_FAILURE;
    }
    if(input_data_pos == KL_HASH_FIRST_HALF)
    {
        cmd_hash_pos = CMD_MOVE_DST_HASH_DIN_H;
    }
    else
    {
        cmd_hash_pos = CMD_MOVE_DST_HASH_DIN_L;
    }
    cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_REG_CPU(handle->data_pos-1)|cmd_hash_pos|CMD_ADDS_MOVE;
    kl_set_cmd(handle, cmd);

    if(input_data_pos == KL_HASH_FIRST_HALF)
    {
        cmd_hash_pos = CMD_MOVE_DST_HASH_DIN_L;
    }
    else
    {
        cmd_hash_pos = CMD_MOVE_DST_HASH_DIN_H;
    }
    cmd = CMD_FUNC_MOVE|handle->key_pos|cmd_hash_pos|CMD_ADDS_MOVE_AND_TRIGGER;
    kl_set_cmd(handle, cmd);

    if(output_key_pos == KL_HASH_FIRST_HALF)
    {
        handle->key_pos = CMD_MOVE_SRC_HASH_DOUT_H;
    }
    else
    {
        handle->key_pos = CMD_MOVE_SRC_HASH_DOUT_L;
    }
    return SUCCESS;
}

int bare_kl_link_seedv(void *bhandle, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = 0;
    unsigned long cmd_in = 0;
    unsigned long cmd_out = 0;
    unsigned long cmd_key = 0;

    if(profile == KL_ETSI_TS_103_162)
    {
        //move mask key to XORb
        cmd = CMD_FUNC_MOVE|((mask_key+0x5)<<8)|CMD_MOVE_DST_XOR_DIN_B|CMD_ADDS_MOVE;
        kl_set_cmd(handle, cmd);
        //move vendor_id to XORa, XOR
        cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_REG_CPU(0)|CMD_MOVE_DST_XOR_DIN_A|CMD_ADDS_MOVE_AND_TRIGGER;
        kl_set_cmd(handle, cmd);
        //move SCKv to AES key
        cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_HASH_DOUT_L|CMD_MOVE_DST_AES_KEY|CMD_ADDS_MOVE;
        kl_set_cmd(handle, cmd);
        //move seedv to AES in
        cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_XOR_DOUT|CMD_MOVE_DST_AES_DIN|CMD_ADDS_MOVE_AND_TRIGGER;
        kl_set_cmd(handle, cmd);
        handle->key_pos = CMD_MOVE_SRC_AES_DOUT;
    }
    else if(profile == KL_GY_T_255_2012)
    {
        //move AES_out to HASH_H
        cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_AES_DOUT|CMD_MOVE_DST_HASH_DIN_H|CMD_ADDS_MOVE;
        kl_set_cmd(handle, cmd);
        //select mask key to AES key
        cmd = CMD_FUNC_MOVE|((mask_key+0x5)<<8)|CMD_MOVE_DST_AES_KEY|CMD_ADDS_MOVE_AND_TRIGGER;
        kl_set_cmd(handle, cmd);
        //move seedv to HASH_L
        cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_AES_DOUT|CMD_MOVE_DST_HASH_DIN_L|CMD_ADDS_MOVE_AND_TRIGGER;
        kl_set_cmd(handle, cmd);

        handle->key_pos = CMD_MOVE_SRC_HASH_DOUT_L;
    }
    else
    {
        switch(profile)
        {
            case KL_SCTE_201_2013_P1:
            case KL_SCTE_201_2013_P1A:
                cmd_in = CMD_MOVE_DST_TDES_DIN;
                cmd_out = CMD_MOVE_SRC_TDES_DOUT;
                cmd_key = CMD_MOVE_DST_TDES_KEY;
                break;
            case KL_SCTE_201_2013_P2:
            case KL_SCTE_201_2013_P2A:
            case KL_SCTE_201_2013_P2B:
                cmd_in = CMD_MOVE_DST_AES_DIN;
                cmd_out = CMD_MOVE_SRC_AES_DOUT;
                cmd_key = CMD_MOVE_DST_AES_KEY;
                break;
            default:
                cmd_print("unknow command\n");
                return ERR_FAILURE;
        }
        if(kl_temporary_save_key(handle) != SUCCESS)
        {
            return ERR_FAILURE;
        }

        //select mask key
        cmd = CMD_FUNC_MOVE|((mask_key+0x5)<<8)|cmd_key|CMD_ADDS_MOVE;
        kl_set_cmd(handle, cmd);
        //move vendor_id to data_in
        cmd = CMD_FUNC_MOVE|CMD_MOVE_SRC_REG_CPU(0)|cmd_in|CMD_ADDS_MOVE_AND_TRIGGER;
        kl_set_cmd(handle, cmd);
        //move seedv to XORa
        cmd = CMD_FUNC_MOVE|cmd_out|CMD_MOVE_DST_XOR_DIN_A|CMD_ADDS_MOVE;
        kl_set_cmd(handle, cmd);
        //move seedv to data_in
        cmd = CMD_FUNC_MOVE|cmd_out|cmd_in|CMD_ADDS_MOVE;
        kl_set_cmd(handle, cmd);
        //move SCKv to key_in
        cmd = CMD_FUNC_MOVE|handle->key_pos|cmd_key|CMD_ADDS_MOVE_AND_TRIGGER;
        kl_set_cmd(handle, cmd);
        //move data_out to XORb, XOR
        cmd = CMD_FUNC_MOVE|cmd_out |CMD_MOVE_DST_XOR_DIN_B|CMD_ADDS_MOVE_AND_TRIGGER;
        kl_set_cmd(handle, cmd);
        //update the xor_dout as key_source for next level
        handle->key_pos = CMD_MOVE_SRC_XOR_DOUT;
    }
    return SUCCESS;
}

int bare_kl_export_key(void *bhandle, KL_EXPORT_DST_E dst, unsigned long slot_id)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned long export_key_pos = 0;
    unsigned long export_dst = 0;
    unsigned short cmd = 0;

    switch(handle->key_pos)
    {
        case CMD_MOVE_SRC_SCK_SELECTED:
            export_key_pos = CMD_EXPORT_SRC_SCK_SELECTED;
            break;
        case CMD_MOVE_SRC_AES_DOUT:
            export_key_pos = CMD_EXPORT_SRC_AES_DOUT;
            break;
        case CMD_MOVE_SRC_TDES_DOUT:
            export_key_pos = CMD_EXPORT_SRC_TDES_DOUT;
            break;
        case CMD_MOVE_SRC_XOR_DOUT:
            export_key_pos = CMD_EXPORT_SRC_XOR_DOUT;
            break;
        case CMD_MOVE_SRC_HASH_DOUT_L:
            export_key_pos = CMD_EXPORT_SRC_HASH_DOUT_L;
            break;
        case CMD_MOVE_SRC_HASH_DOUT_H:
            export_key_pos = CMD_EXPORT_SRC_HASH_DOUT_H;
            break;
        default:
            return ERR_FAILURE;
    }
    switch(dst)
    {
        case KL_EXPORT_DST_KT:
            export_dst = 1;
            *(volatile unsigned long*)KEYLADDER_REG_KEYTB_SLOT_INDEX = slot_id;
            break;
        case KL_EXPORT_DST_ACPU:
            export_dst = 0x15;
            break;
        case KL_EXPORT_DST_SCPU:
            export_dst = 0x0a;
            break;
        default:
            return ERR_FAILURE;
    }

    cmd = CMD_FUNC_EXPORT_KEY | export_key_pos | (export_dst<<3) | 0x6;
    return kl_set_cmd(handle, cmd);
}

int bare_kl_store_key(void *bhandle, KL_STORE_DST_E store_dst)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned long store_key_pos = 0;
    unsigned short cmd = 0;

    switch(handle->key_pos)
    {
        case CMD_MOVE_SRC_AES_DOUT:
            store_key_pos = CMD_RESTORE_SRC_AES_DOUT;
            break;
        case CMD_MOVE_SRC_TDES_DOUT:
            store_key_pos = CMD_RESTORE_SRC_TDES_DOUT;
            break;
        case CMD_MOVE_SRC_XOR_DOUT:
            store_key_pos = CMD_RESTORE_SRC_XOR_DOUT;
            break;
        case CMD_MOVE_SRC_HASH_DOUT_L:
            store_key_pos = CMD_RESTORE_SRC_HASH_DOUT_L;
            break;
        case CMD_MOVE_SRC_HASH_DOUT_H:
            store_key_pos = CMD_RESTORE_SRC_HASH_DOUT_H;
            break;
        default:
            return ERR_FAILURE;
    }

    cmd = CMD_FUNC_STORE_TO_PRIVATE|store_key_pos|((store_dst+0x14)<<3)|KL_STORE_ADDS_LOCK;
    return kl_set_cmd(handle, cmd);
}

int bare_kl_store_cmd(void *bhandle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = CMD_FUNC_STORE_TO_PRIVATE|((store_src+0x10)<<8)|((store_dst+0x14)<<3)|lock;
    return kl_set_cmd(handle, cmd);
}

int bare_kl_move_cmd(void *bhandle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = CMD_FUNC_MOVE|(move_src<<8)|(move_dst<<3)|trigger;
    return kl_set_cmd(handle, cmd);
}

int bare_kl_export_cmd(void *bhandle, KL_EXPORT_SOURCE_E key_src, KL_EXPORT_DST_E dst, unsigned long slot_id)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned long export_dst = 0;
    unsigned short cmd;
    switch(dst)
    {
        case KL_EXPORT_DST_KT:
            export_dst = 1;
            *(volatile unsigned long*)KEYLADDER_REG_KEYTB_SLOT_INDEX = slot_id;
            break;
        case KL_EXPORT_DST_ACPU:
            export_dst = 0x15;
            break;
        case KL_EXPORT_DST_SCPU:
            export_dst = 0x0a;
            break;
        default:
            return ERR_FAILURE;
    }
    cmd = CMD_FUNC_EXPORT_KEY | ((key_src + 0x18) << 8) | (export_dst<<3) | 0x6;
    return kl_set_cmd(handle, cmd);
}

int bare_kl_wait_complete(void *bhandle, unsigned long *p_error)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned long state = 0;

    if(kl_wait_cmd_timeout(KEYLADDER_REG_STATE_CHECK, &state) == ERR_FAILURE)
    {
        if(p_error != NULL)
        {
            *p_error = state;
        }
        return 2;
    }

    if(state != 0)
    {
        if(p_error!= NULL)
        {
            *p_error = state;
        }
        //cmd_print("cmd error, state = %x\n", state);
        return 1;
    }

    if(p_error != NULL)
    {
        *p_error = 0;
    }
    return SUCCESS;

}

int bare_kl_read_key(void *bhandle, unsigned char *p_key_buffer)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    if(p_key_buffer)
    {
        memcpy_from_reg(p_key_buffer, (unsigned char *)KEYLADDER_REG_ACPU, 16);
        return SUCCESS;
    }
    return ERR_FAILURE;
}

int bare_kl_input_data(void *bhandle, KL_INPUT_POSITION_E postion, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    if(p_input)
    {
        memcpy_to_reg((unsigned char *)(KEYLADDER_REG_CPU0 + ((postion)<<4)), p_input, 16);
        return SUCCESS;
    }
    return ERR_FAILURE;
}

