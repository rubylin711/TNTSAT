/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mt_type.h"
#include "drv_cipher_scpu_ioctl.h"
#include "hal_cipher.h"
#include "mt_mpi_cipher.h"
#include "mt_module_debug.h"

int g_cipher_devfd = -1;
/*************************** Structure Definition ****************************/
/*!
  Cipher operation decrypt or encrypt
  */


mt_s32 MT_MPI_CIPHER_init(void)
{
    if(g_cipher_devfd >= 0)
    {
        return MT_SUCCESS;
    }
    //open fd in /dev/xxx
    g_cipher_devfd = open("/dev/mt_cipher", O_RDWR, 0);
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Open cipher fd error!\n");
        return MT_FAILURE;
    }
    //MT_ERR_CIPHER("open success!,fd = %d\n", g_cipher_devfd);
    ioctl(g_cipher_devfd, CMD_SEC_CIPHER_INIT, &g_cipher_devfd);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_CIPHER_deinit(void)
{
    //close fd in /dev/xx
    if(g_cipher_devfd >= 0)
    {
        close(g_cipher_devfd);
        g_cipher_devfd = -1;
        return MT_SUCCESS;
    }
    else
    {
        MT_ERR_CIPHER("Already closed!\n");
        return MT_FAILURE;
    }
}

mt_s32 MT_MPI_CIPHER_keyladder_create(MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source, mt_u32 index, mt_handle *p_keyladder)
{
    mt_s32 ret = MT_FAILURE;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_keyladder_priv ci = {
            .keyladder_source = keyladder_source,
            .index = index,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_KEYLADDER_CREATE, &ci);
        if(ret != MT_FAILURE)	
            *p_keyladder = ci.p_keyladder;   
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_keyladder_destroy(mt_handle keyladder)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_KEYLADDER_DESTROY, keyladder);
    }
    return ret;

}

mt_s32 MT_MPI_CIPHER_keyladder_link(mt_handle keyladder, MT_CIPHER_CTRL_S *p_ctrl, mt_u8 *input, mt_u32 length)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_keyladder_priv ci = {
            .p_keyladder = keyladder,
            .p_ctrl = p_ctrl,
            .input = input,
            .length = length,
        };	    
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_KEYLADDER_LINK, &ci);
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, mt_handle *p_handle)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_hash_data_priv ci ={
            .hash_type = hash_type,
        };	    
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_HASH_CREATE, &ci);
        if(ret){
            MT_ERR_CIPHER("ret = %d\n", ret);
            return ret;
        }
        else
            *p_handle = ci.hash_handle;
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_hash_update(mt_handle handle, mt_u8 *p_data, mt_u32 length)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_hash_data_priv ci ={
            .hash_handle = handle,
            .p_data = p_data,
            .length = length,
        };	    
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_HASH_UPDATE, &ci);
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_hash_final(mt_handle handle, mt_u8 *p_output_hash)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_hash_data_priv ci ={
            .hash_handle = handle,
            .p_output_hash = p_output_hash,
        };	    
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_HASH_FINAL, &ci);
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_create(mt_handle key_ladder_handle, mt_handle *p_cipher)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_handle_priv ci = {
            .key_ladder_handle = key_ladder_handle,
            .p_cipher = *p_cipher,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_HANDLE_CREATE, &ci);
        if(ret){
            return ret;
        }
        else
            *p_cipher = ci.p_cipher;    
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_destroy(mt_handle cipher)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_HANDLE_DESTROY, cipher);
    }	
    return ret;
}

mt_s32 MT_MPI_CIPHER_config(mt_handle cipher, MT_CIPHER_CTRL_S *p_ctrl)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_config_priv ci = {
            .cipher = cipher,
            .p_ctrl = p_ctrl,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_INFO_CONFIG, &ci);
    }	
    return ret;

}

mt_s32 MT_MPI_CIPHER_process(mt_handle cipher, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_data_priv ci = {
            .cipher 	  = cipher,
            .p_src_addr   = p_src_addr,
            .p_dest_addr  = p_dest_addr,
            .length 	  = length,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_DATA_PROCESS, &ci);
    }	
    return ret;
}

mt_s32 MT_MPI_CIPHER_set_cw(mt_handle cipher, MT_CIPHER_CW_INFO_S *p_cw_info, mt_u8 *p_src_addr, mt_u32 length)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_cw_priv ci = {
            .cipher_handle = cipher,
            .p_cw_info     = p_cw_info,
            .p_src_addr    = p_src_addr,
            .length 	   = length,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_SET_CW, &ci);
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_get_random_number(mt_u32 *p_random_number)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_GET_RND_NUM, p_random_number);
    }
    return ret;
}
mt_s32 MT_MPI_CIPHER_get_pvr_key(mt_u32 channel_id, mt_u32 key_length, mt_u8 *p_pvr_key)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_pvr_priv ci = {
            .channel_id = channel_id,
            .key_length = key_length,
            .p_pvr_key = p_pvr_key,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_GET_PVR_KEY, &ci);
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_special_command(MT_CIPHER_SPECIAL_COMMAND_E cmd_id, void *p_input, mt_u32 in_len, void *p_output, mt_u32 out_len)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_cmd_priv ci = {
            .cmd_id = cmd_id,
            .p_input = p_input,
            .in_len = in_len,
            .p_output = p_output,
            .out_len = out_len,	    
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_SPECIAL_CMD, &ci);
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_send_message(mt_u32 channel_id, mt_u32 wait_ack, mt_u32 msg_id, mt_u32 param1, mt_u32 param2)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_send_msg_priv ci = {
            .channel_id = channel_id,
            .ack   	= wait_ack,
            .msg_id	= msg_id,
            .param1	= param1,
            .param2	= param2,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_RECV_MSG, &ci);
    }
    return ret;

}


mt_s32 MT_MPI_CIPHER_receive_message(mt_u32 channel_id, mt_u32 send_ack, mt_u32 *p_msg_id, mt_u32 *p_param1, mt_u32 *p_param2)
{
    int ret = -1;
    if(g_cipher_devfd < 0)
    {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    else
    {
        struct cipher_recv_msg_priv ci = {
            .channel_id = channel_id,
            .ack   	= send_ack,
            .p_msg_id	= p_msg_id,
            .p_param1	= p_param1,
            .p_param2	= p_param2,
        };
        ret = ioctl(g_cipher_devfd, CMD_SEC_CIPHER_SEND_MSG, &ci);
    }
    return ret;
}

mt_s32 MT_MPI_CIPHER_load_hdcpkey(mt_u8 *p_enc_hdcpkey, mt_u32 len)
{
    mt_s32 ret = MT_SUCCESS;
    MT_CIPHER_CTRL_S info;
    mt_handle cipher_handle;
    mt_handle keyladder_handle;
    MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source = MT_CIPHER_KEY_LADDER_HDCP;

    if (p_enc_hdcpkey == NULL)
        return MT_FAILURE;

    if (len != 304)
        return MT_FAILURE;

    ret = MT_MPI_CIPHER_keyladder_create(keyladder_source, 0, &keyladder_handle);
    if (ret != MT_SUCCESS) {
        return ret;
    }

    memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));

    ret = MT_MPI_CIPHER_create(keyladder_handle, &cipher_handle);
    if (ret != MT_SUCCESS) {
        MT_MPI_CIPHER_keyladder_destroy(keyladder_handle);
        return ret;
    }

    info.operation = MT_CIPHER_OPERATION_DECRYPT;
    info.algorithm = MT_CIPHER_ALG_AES;
    info.parameter.aes_para.work_mode = MT_CIPHER_WORK_MODE_ECB;
    info.parameter.aes_para.key_length = 16;    
    ret = MT_MPI_CIPHER_config(cipher_handle, &info);
    if (ret != MT_SUCCESS) {
        MT_MPI_CIPHER_keyladder_destroy(keyladder_handle);
        MT_MPI_CIPHER_destroy(cipher_handle);
        return ret;
    }

    //No output
    ret = MT_MPI_CIPHER_process(cipher_handle, p_enc_hdcpkey, NULL, len);

    MT_MPI_CIPHER_keyladder_destroy(keyladder_handle);
    MT_MPI_CIPHER_destroy(cipher_handle);

    return ret;
}


