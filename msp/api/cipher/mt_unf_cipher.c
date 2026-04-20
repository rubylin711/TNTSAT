/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_common.h"
#include "drv_cipher_scpu_ioctl.h"
#include "hal_cipher.h"
#include "mt_mpi_cipher.h"
#include "mt_unf_otp.h"
#include "mt_unf_cipher.h"
#include "mt_module_debug.h"
#include <errno.h>

static pthread_mutex_t g_cipher_mutex = PTHREAD_MUTEX_INITIALIZER;

#define cipher_lock()       (void)pthread_mutex_lock(&g_cipher_mutex);
#define cipher_unlock()     (void)pthread_mutex_unlock(&g_cipher_mutex);

#define HASH_BLOCK_SIZE (64)
#define SHA1_SIGN_SIZE  (20)
#define SHA256_SIGN_SIZE  (32)


static mt_u8 g_i_key_pad[HASH_BLOCK_SIZE] = {0};

static mt_u8 g_hmackey[HASH_BLOCK_SIZE];
static mt_u32 g_hmackey_size;
static MT_CIPHER_HASH_TYPE_E g_hmac_hash_type;


#if 0
static void dump(mt_u8 *buffer, mt_u32 len)
{
    MT_INFO_CIPHER("\n==================================\n");
    for(int i = 0; i < len; i++)
    {
        if((i%16) == 0 && i != 0)
            MT_INFO_CIPHER("\n");
        MT_INFO_CIPHER("%02X ", buffer[i]);
    }
    MT_INFO_CIPHER("\n==================================\n");
}
#endif

mt_s32 MT_UNF_CIPHER_init(void)
{
    mt_s32 ret = MT_MPI_CIPHER_init();
    return ret;
}

mt_s32 MT_UNF_CIPHER_deinit(void)
{
    return MT_MPI_CIPHER_deinit();
}

mt_s32 MT_UNF_CIPHER_keyladder_create(MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source, mt_u32 index, mt_handle *p_keyladder)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_keyladder_create(keyladder_source, index, p_keyladder);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_keyladder_destroy(mt_handle keyladder)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_keyladder_destroy(keyladder);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_keyladder_link(mt_handle keyladder, MT_CIPHER_CTRL_S *p_ctrl, mt_u8 *input, mt_u32 length)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_keyladder_link(keyladder, p_ctrl, input, length);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, mt_handle *p_handle)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_hash_create(hash_type, p_handle);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        return ret;
    }
    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_hash_update(mt_handle handle, mt_u8 *p_data, mt_u32 length)
{
  mt_s32 ret = MT_SUCCESS;

  cipher_lock();

  ret = MT_MPI_CIPHER_hash_update(handle, p_data, length);

  cipher_unlock();

  return ret;
}

mt_s32 MT_UNF_CIPHER_hash_final(mt_handle handle, mt_u8 *p_output_hash)
{
    mt_s32 ret;

    cipher_lock();
    ret = MT_MPI_CIPHER_hash_final(handle, p_output_hash);
    cipher_unlock();

    return ret;
}

/*
 * Call it inside lock
 */
static mt_s32 hmac_key_init(mt_u8 *hmac_key, mt_u32 length, mt_u8 out_hmac_key[HASH_BLOCK_SIZE])
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle handle;
    mt_mmz_buf_s new_buf;

    if (hmac_key == NULL || length == 0)
        return MT_FAILURE;

    if (length <= HASH_BLOCK_SIZE) {
        memcpy(out_hmac_key, hmac_key, length);
        g_hmackey_size = length;
        return MT_SUCCESS;
    }

    //key length is bigger, we should calc a hash for it
    ret = MT_MPI_CIPHER_hash_create(g_hmac_hash_type, &handle);
    if (ret != MT_SUCCESS)
        return ret;

    memcpy(new_buf.bufname, "hmac", 4);
    new_buf.bufname[4] = '\0';
    new_buf.bufsize = length;
    ret = mt_mmz_malloc(&new_buf);
    if (ret != MT_SUCCESS) {
        /* mmz buffer for hmac key failed */
        return ret;
    }

    memcpy(&new_buf.user_viraddr, hmac_key, length);

    ret = MT_MPI_CIPHER_hash_update(handle, (mt_u8 *)new_buf.phyaddr, length);
    if (ret != MT_SUCCESS)
        goto __hmac_key_init_out;

    ret = MT_MPI_CIPHER_hash_final(handle, out_hmac_key);
    if (ret != MT_SUCCESS)
        goto __hmac_key_init_out;

    if (g_hmac_hash_type == MT_CIPHER_HASH_TYPE_SHA1)
        g_hmackey_size = 20;
    else
        g_hmackey_size = 32;

__hmac_key_init_out:
    mt_mmz_free(&new_buf);
    return ret;
}

mt_s32 MT_UNF_CIPHER_hmac_create(MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, mt_handle *p_handle)
{
    mt_s32 ret;
    MT_CIPHER_HASH_TYPE_E hash_type;
    mt_u8 i_key_pad[HASH_BLOCK_SIZE];
    mt_u32 i;

    if (p_hmac_attr == NULL)
        return MT_FAILURE;

    if (p_hmac_attr->hmac_type >= MT_CIPHER_HMAC_BUTT
            || p_hmac_attr->p_hmac_key == NULL
            || p_hmac_attr->key_len == 0)
        return MT_FAILURE;

    cipher_lock();

    if (p_hmac_attr->hmac_type == MT_CIPHER_HMAC_TYPE_SHA1)
        hash_type = MT_CIPHER_HASH_TYPE_SHA1;
    else if (p_hmac_attr->hmac_type == MT_CIPHER_HMAC_TYPE_SHA256)
        hash_type = MT_CIPHER_HASH_TYPE_SHA256;
    else {
        cipher_unlock();
        return MT_FAILURE;
    }

    g_hmac_hash_type = hash_type;

    //init hmac key and generate i_key_pad
    memset(g_hmackey, 0, sizeof(g_hmackey));
    ret = hmac_key_init(p_hmac_attr->p_hmac_key, p_hmac_attr->key_len, g_hmackey);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        return MT_FAILURE;
    }

    ret = MT_MPI_CIPHER_hash_create(hash_type, p_handle);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        return MT_FAILURE;
    }

	memset(i_key_pad, 0x36, HASH_BLOCK_SIZE);
    for (i = 0; i < g_hmackey_size; i++) {
        i_key_pad[i] = i_key_pad[i] ^ g_hmackey[i];
    }

    cipher_unlock();

	//hash i_key_pad update
    memcpy(g_i_key_pad, i_key_pad, sizeof(i_key_pad));

    return MT_SUCCESS;
}

mt_s32 MT_UNF_CIPHER_hmac_update(mt_handle handle, mt_u8 *p_data, mt_u32 length)
{
    mt_s32 ret = MT_SUCCESS;
    mt_mmz_buf_s new_buf;
    mt_u8 *input_viraddr;

    if (p_data == NULL || length == 0)
        return MT_FAILURE;

    /* reallocate input buffer + key */
    memcpy(new_buf.bufname, "hmac", 4);
    new_buf.bufname[4] = '\0';
    new_buf.bufsize = length + sizeof(g_i_key_pad);
    ret = mt_mmz_malloc(&new_buf);
    if (ret != MT_SUCCESS) {
        /* reallocate mmz buffer for msg+i_key_pad failed */
        return ret;
    }

    cipher_lock();

    ///* p_data is a phyaddr */
    //input_viraddr = (mt_u8 *)mt_mmz_map((mt_u32)p_data, 0);
    input_viraddr = p_data;

    memcpy(new_buf.user_viraddr, g_i_key_pad, sizeof(g_i_key_pad));
    memcpy((new_buf.user_viraddr + sizeof(g_i_key_pad)), input_viraddr, length);
    ret = MT_MPI_CIPHER_hash_update(handle, (mt_u8 *)new_buf.phyaddr, length+sizeof(g_i_key_pad));

    mt_mmz_free(&new_buf);
    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_hmac_final(mt_handle handle, mt_u8 *p_output_hmac)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 hash_sum_1[SHA256_SIGN_SIZE] = {0}; //define bigger size
    mt_u32 hash_sum_1_len = 0;
    mt_u8 o_key_pad[HASH_BLOCK_SIZE];
    mt_handle tmp_handle;
    mt_mmz_buf_s new_buf;
    mt_u32 i;

	if (p_output_hmac == NULL)
		return MT_FAILURE;

    cipher_lock();

    //get hash of i_key_pad+message
    ret |= MT_MPI_CIPHER_hash_final(handle, hash_sum_1);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        return MT_FAILURE;
    }

    //o_key_pad
    memset(o_key_pad, 0x5c, HASH_BLOCK_SIZE);
    for (i = 0; i < g_hmackey_size; i++) {
        o_key_pad[i] = o_key_pad[i] ^ g_hmackey[i];
    }

    memcpy(new_buf.bufname, "hmac", 4);
    new_buf.bufname[4] = '\0';

    //cat o_key_pad+hash_sum_1
    if (g_hmac_hash_type == MT_CIPHER_HASH_TYPE_SHA1)
        hash_sum_1_len = SHA1_SIGN_SIZE;
    else
        hash_sum_1_len = SHA256_SIGN_SIZE;
   
    new_buf.bufsize = HASH_BLOCK_SIZE + hash_sum_1_len;
    ret = mt_mmz_malloc(&new_buf);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        return ret;
    }

    memcpy(new_buf.user_viraddr, o_key_pad, HASH_BLOCK_SIZE);
    memcpy((new_buf.user_viraddr + HASH_BLOCK_SIZE), hash_sum_1, hash_sum_1_len);

    //calc o_key_pad+hash_sum_1 hash */
    ret = MT_MPI_CIPHER_hash_create(g_hmac_hash_type, &tmp_handle);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        mt_mmz_free(&new_buf);
        return MT_FAILURE;
    }

    ret = MT_MPI_CIPHER_hash_update(tmp_handle, (mt_u8 *)new_buf.phyaddr, HASH_BLOCK_SIZE + hash_sum_1_len);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        mt_mmz_free(&new_buf);
        return MT_FAILURE;
    }

    mt_mmz_free(&new_buf);

    //final hash of o_key_pad+hash_sum_1
    ret = MT_MPI_CIPHER_hash_final(tmp_handle, p_output_hmac);

    cipher_unlock();
    return ret;
}

mt_s32 MT_UNF_CIPHER_create(mt_handle key_ladder_handle, mt_handle *p_cipher)
{
    mt_s32 ret;


    cipher_lock();

    ret = MT_MPI_CIPHER_create(key_ladder_handle, p_cipher);
    if (ret != MT_SUCCESS) {
        cipher_unlock();
        return ret;
    }

    cipher_unlock();

    return ret;
}


mt_s32 MT_UNF_CIPHER_destroy(mt_handle cipher)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_destroy(cipher);

    cipher_unlock();


    return ret;
}

mt_s32 MT_UNF_CIPHER_config(mt_handle cipher, MT_CIPHER_CTRL_S *p_ctrl)
{
    mt_s32 ret = MT_SUCCESS;

    cipher_lock()

    ret = MT_MPI_CIPHER_config(cipher, p_ctrl);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_process(mt_handle cipher, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length)
{
    mt_s32 ret;

    if (p_src_addr == NULL || p_dest_addr == NULL || length == 0) {
        return MT_FAILURE;
    }

    cipher_lock();

    ret = MT_MPI_CIPHER_process(cipher, p_src_addr, p_dest_addr, length);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_set_cw(mt_handle cipher, MT_CIPHER_CW_INFO_S *p_cw_info, mt_u8 *p_src_addr, mt_u32 length)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_set_cw(cipher, p_cw_info, p_src_addr, length);

    cipher_unlock();

    return ret;
}


mt_s32 MT_UNF_CIPHER_get_random_number(mt_u32 *p_random_number)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_get_random_number(p_random_number);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_get_pvr_key(mt_u32 channel_id, mt_u32 key_length, mt_u8 *p_pvr_key)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_get_pvr_key(channel_id, key_length, p_pvr_key);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_special_command(MT_CIPHER_SPECIAL_COMMAND_E cmd_id, void *p_input, mt_u32 in_len, void *p_output, mt_u32 out_len)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_special_command(cmd_id, p_input, in_len, p_output, out_len);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_send_message(mt_u32 channel_id, mt_u32 wait_ack, mt_u32 msg_id, mt_u32 param1, mt_u32 param2)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_send_message(channel_id, wait_ack, msg_id, param1, param2);

    cipher_unlock();

    return ret;
}


mt_s32 MT_UNF_CIPHER_receive_message(mt_u32 channel_id, mt_u32 send_ack, mt_u32 *p_msg_id, mt_u32 *p_param1, mt_u32 *p_param2)
{
    mt_s32 ret;

    cipher_lock();

    ret = MT_MPI_CIPHER_receive_message(channel_id, send_ack, p_msg_id, p_param1, p_param2);

    cipher_unlock();

    return ret;
}

mt_s32 MT_UNF_CIPHER_load_hdcpkey(mt_u8 *p_enc_hdcpkey, mt_u32 len)
{
    mt_s32 ret = MT_FAILURE;

    cipher_lock();
    ret = MT_MPI_CIPHER_load_hdcpkey(p_enc_hdcpkey, len);
    cipher_unlock();

    return ret;
}
