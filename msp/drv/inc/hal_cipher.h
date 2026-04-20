/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_HAL_CIPHER_H__
#define __MT_HAL_CIPHER_H__

#include "drv_cipher_scpu_ioctl.h"
#include "mt_unf_cipher.h"

struct cipher_handle_priv
{
    mt_handle key_ladder_handle;
    mt_handle p_cipher;
};

struct cipher_config_priv
{
    mt_handle cipher;
    MT_CIPHER_CTRL_S *p_ctrl;
};

struct cipher_data_priv
{
    mt_handle cipher;
    mt_u8 *p_src_addr;
    mt_u8 *p_dest_addr;
    mt_u32 length;
};

//mass data
struct cipher_data_mass_priv
{
    mt_handle cipher;
    CIPHER_CIPHER_DATA_S *p_cipher_data;
    mt_u32 length;
};

struct cipher_cw_priv
{
    mt_handle cipher_handle;
    MT_CIPHER_CW_INFO_S * p_cw_info;
    mt_u8 *p_src_addr;
    mt_u32 length;
};
struct cipher_pvr_priv
{
    mt_u32 channel_id;
    mt_u32 key_length;
    mt_u8 *p_pvr_key;
};
struct cipher_cmd_priv
{
    MT_CIPHER_SPECIAL_COMMAND_E cmd_id;
    void *p_input;
    mt_u32 in_len;
    void *p_output;
    mt_u32 out_len;
};
struct cipher_keyladder_priv
{
    MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source;
    mt_u32 index;
    mt_handle p_keyladder;
    MT_CIPHER_CTRL_S *p_ctrl;
    mt_u8 *input;
    mt_u32 length;
};
struct cipher_hash_data_priv
{
    mt_handle hash_handle;
    mt_u8 *p_data;
    mt_u32 length;
    mt_u8 *p_output_hash;
    mt_u32 hash_type;
};

struct cipher_send_msg_priv
{
    mt_u32 channel_id;
    mt_u32 ack;
    mt_u32 msg_id;
    mt_u32 param1;
    mt_u32 param2;
};
struct cipher_recv_msg_priv
{
    mt_u32 channel_id;
    mt_u32 ack;
    mt_u32 *p_msg_id;
    mt_u32 *p_param1;
    mt_u32 *p_param2;
};

#endif
