/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_common.h"
#include "mt_mpi_ipcs.h"
#include "mt_unf_cipher_v2.h"
#include "mt_see_cipher.h"
#include "mt_see_misc.h"
#include "mt_unf_mss.h"

int mt_unf_mss_see_init(void)
{
    see_mcpu_mailbox_init();
    
    mt_unf_see_cipher_init(0, 0, 0, 0);

    return MT_SUCCESS;
}

int mt_unf_mss_see_deinit(void)
{
    return MT_SUCCESS;
}

int mt_unf_mss_see_recv_peer_msg(MT_PEER_MSG_S *p_msg)
{
    int ret = MT_SUCCESS;
    mpi_ipcs_msg_t ipcs_msg = {0};

    if (p_msg == NULL)
        return MT_FAILURE;

    ipcs_msg.channel_id = p_msg->channel_id;
    ipcs_msg.ack = p_msg->ack;

    ret = mt_mpi_ipcs_get_scpu_response(&ipcs_msg);
    if (ret != MT_SUCCESS)
        return ret;

    p_msg->msg_id = ipcs_msg.msg_id;
    p_msg->param1 = ipcs_msg.param1;
    p_msg->param2 = ipcs_msg.param2;

    return MT_SUCCESS;
}

int mt_unf_mss_see_send_peer_msg(MT_PEER_MSG_S *p_msg)
{
    mpi_ipcs_msg_t ipcs_msg = {0};

    ipcs_msg.channel_id = p_msg->channel_id;
    ipcs_msg.ack = p_msg->ack;
    ipcs_msg.msg_id = p_msg->msg_id;
    ipcs_msg.param1 = p_msg->param1;
    ipcs_msg.param2 = p_msg->param2;

    return mt_mpi_ipcs_send_scpu_msg(&ipcs_msg);
}

int mt_unf_mss_see_peer_exchange(unsigned int msg_cmd, unsigned char *p_input, unsigned int input_len, unsigned char *p_output, unsigned int *p_output_len)
{
    return mt_unf_see_cipher_transfer((mt_u32)msg_cmd, (mt_u8 *)p_input, (mt_u32)input_len, (mt_u8 *)p_output, (mt_u32 *)p_output_len);
}

int mt_unf_mss_see_keyattr_create(MT_CIPHER_CTRL_S *p_ctrl, unsigned int *p_key_attr, unsigned int * p_kattr_size)
{
    return mt_unf_see_key_attr_create(p_ctrl, p_key_attr, p_kattr_size);
}

void mt_unf_mss_see_keyattr_destroy(unsigned int *p_key_attr)
{
    mt_unf_see_key_attr_destroy(p_key_attr);
}

MT_CPU_STATE_E mt_unf_mss_see_check_peer_state(void)
{
    unsigned int status = 0;

    status = mt_mpi_ipcs_check_peer();
    if (status == 0)
        return PEER_CPU_NOT_READY;
    else if (status == 1)
        return PEER_CPU_READY;
    else
        return MT_FAILURE;
}

void mt_unf_mss_see_set_local_state(MT_CPU_STATE_E sta)
{
    if (sta == LOCAL_CPU_NOT_READY)
        mt_mpi_ipcs_set_local(0);
    else 
        mt_mpi_ipcs_set_local(1);
}

int mt_unf_mss_see_lock_peer(void)
{
    return see_mb_take_cryptolock();
}

int mt_unf_mss_see_unlock_peer(void)
{
    return see_mb_give_cryptolock();
}

