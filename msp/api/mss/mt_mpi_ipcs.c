/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "mt_common.h"
#include "mt_mpi_ipcs.h"

#undef MOD_NAME
#define MOD_NAME			"ipcs"

#define IPCS_BUF_ALIGN_SIZE 0x20

mt_s32 mt_mpi_ipcs_buf_malloc(mt_ipcs_mmz_buf_s *p_buf, mt_u32 bufsize)
{
    if (NULL == p_buf)
        return MT_FAILURE;

    p_buf->bufsize = (bufsize+IPCS_BUF_ALIGN_SIZE-1)/IPCS_BUF_ALIGN_SIZE*IPCS_BUF_ALIGN_SIZE;
    p_buf->cached = 0;

    p_buf->phyaddr = mt_mmz_new(p_buf->bufsize, IPCS_BUF_ALIGN_SIZE, NULL, "ipcs");
    p_buf->user_viraddr = mt_mmz_map(p_buf->phyaddr, 0);

    if (NULL == p_buf->user_viraddr)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 mt_mpi_ipcs_buf_malloc_cached(mt_ipcs_mmz_buf_s *p_buf, mt_u32 bufsize)
{
    if (NULL == p_buf)
        return MT_FAILURE;

    p_buf->bufsize = (bufsize+IPCS_BUF_ALIGN_SIZE-1)/IPCS_BUF_ALIGN_SIZE*IPCS_BUF_ALIGN_SIZE;
    p_buf->cached = 1;

    p_buf->phyaddr = mt_mmz_new(p_buf->bufsize, IPCS_BUF_ALIGN_SIZE, NULL, MOD_NAME);
    p_buf->user_viraddr = mt_mmz_map(p_buf->phyaddr, 1);

    if (NULL == p_buf->user_viraddr)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 mt_mpi_ipcs_buf_free(mt_ipcs_mmz_buf_s *p_buf)
{
    if (p_buf)
    {
        if (p_buf->user_viraddr)
        {
            mt_mmz_unmap(p_buf->user_viraddr);
            mt_mmz_delete(p_buf->phyaddr);
        }
    }

    return MT_SUCCESS;
}

int mt_mpi_ipcs_open(mt_handle *p_handle)
{
    int fd = -1;

    fd = open("/dev/mt_ipcs", O_RDWR);

    if(fd >= 0)
    {
        *p_handle = (mt_handle)fd;
        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

int mt_mpi_ipcs_close(mt_handle handle)
{
    if (handle == 0)
        return MT_FAILURE;

    return close((int)handle);
}

int mt_mpi_ipcs_get_scpu_response(mpi_ipcs_msg_t *p_msg)
{
    mt_handle handle;
    struct ipcs_recv_msg_priv *p_recv_msg;
    struct ipcs_recv_msg_priv recv_msg;
    mt_ipcs_mmz_buf_s mmz_msg_id;
    mt_ipcs_mmz_buf_s mmz_param1;
    mt_ipcs_mmz_buf_s mmz_param2;
    int ret = MT_FAILURE;

    if (NULL == p_msg)
        return MT_FAILURE;

    if (MT_SUCCESS != mt_mpi_ipcs_buf_malloc(&mmz_msg_id, 4))
    {
        ret = MT_FAILURE;
        goto EXIT;
    }

    if (MT_SUCCESS != mt_mpi_ipcs_buf_malloc(&mmz_param1, 4))
    {
        ret = MT_FAILURE;
        goto EXIT;
    }

    if (MT_SUCCESS != mt_mpi_ipcs_buf_malloc(&mmz_param2, 4))
    {
        ret = MT_FAILURE;
        goto EXIT;
    }

    p_recv_msg = &recv_msg;

    p_recv_msg->channel_id = p_msg->channel_id;
    p_recv_msg->ack = p_msg->ack;

    p_recv_msg->p_msg_id = (mmz_msg_id.phyaddr | 0xA0000000);
    p_recv_msg->p_param1 = (mmz_param1.phyaddr | 0xA0000000);
    p_recv_msg->p_param2 = (mmz_param2.phyaddr | 0xA0000000);

    if (MT_SUCCESS != mt_mpi_ipcs_open(&handle))
    {
        ret = MT_FAILURE;
        goto EXIT;
    }

    ret = ioctl((int)handle, CMD_IPCS_RECV_MSG, p_recv_msg);

    mt_mpi_ipcs_close(handle);

    p_msg->msg_id = (unsigned int)*((unsigned int *)mmz_msg_id.user_viraddr);
    p_msg->param1 =(unsigned int)*((unsigned int *)mmz_param1.user_viraddr);
    p_msg->param2 = (unsigned int)*((unsigned int *)mmz_param2.user_viraddr);

EXIT:
    mt_mpi_ipcs_buf_free(&mmz_msg_id);
    mt_mpi_ipcs_buf_free(&mmz_param1);
    mt_mpi_ipcs_buf_free(&mmz_param2);

    return ret;
}

int mt_mpi_ipcs_send_scpu_msg(mpi_ipcs_msg_t *p_msg)
{
    mt_handle handle;
    struct ipcs_send_msg_priv *p_send_msg;
    mt_ipcs_mmz_buf_s mmz_send_msg;
    int ret = MT_FAILURE;

    if (NULL == p_msg)
        return MT_FAILURE;

    if (MT_SUCCESS != mt_mpi_ipcs_buf_malloc(&mmz_send_msg, sizeof(struct ipcs_send_msg_priv)))
    {
        return MT_FAILURE;
    }

    p_send_msg = (struct ipcs_send_msg_priv *)mmz_send_msg.user_viraddr;

    p_send_msg->channel_id = p_msg->channel_id;
    p_send_msg->ack = p_msg->ack;
    p_send_msg->msg_id = p_msg->msg_id;
    p_send_msg->param1 = p_msg->param1;
    p_send_msg->param2 = p_msg->param2;

    if (MT_SUCCESS != mt_mpi_ipcs_open(&handle))
    {
        ret = MT_FAILURE;
        goto EXIT;
    }

    ret = ioctl((int)handle, CMD_IPCS_SEND_MSG, p_send_msg);

    mt_mpi_ipcs_close(handle);

EXIT:
    mt_mpi_ipcs_buf_free(&mmz_send_msg);

    return ret;
}

unsigned int mt_mpi_ipcs_check_peer(void)
{
    int ret = MT_FAILURE;
    mt_handle handle;
    struct ipcs_peer_status_priv peer_sta = {0};

    if (MT_SUCCESS != mt_mpi_ipcs_open(&handle))
    {
        return (unsigned int)MT_FAILURE;
    }

    ret = ioctl((int)handle, CMD_IPCS_CHECK_PEER, &peer_sta);

    mt_mpi_ipcs_close(handle);

    if (ret != MT_SUCCESS)
        return (unsigned int)MT_FAILURE;

    return peer_sta.peer_status;

}

int mt_mpi_ipcs_set_local(unsigned int status)
{
    int ret = MT_FAILURE;
    mt_handle handle;
    struct ipcs_peer_status_priv local_sta = {0};

    if (MT_SUCCESS != mt_mpi_ipcs_open(&handle))
    {
        return MT_FAILURE;
    }

    local_sta.local_status = status;

    ret = ioctl((int)handle, CMD_IPCS_SET_LOCAL, &local_sta);

    mt_mpi_ipcs_close(handle);

    return ret;
}

