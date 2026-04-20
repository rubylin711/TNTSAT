/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_IPCS_IOCTL_H___
#define __DRV_IPCS_IOCTL_H___

/*
 * define 'i' as the magic character of ipcs 
 */
#define HAL_IPCS_IOC_MAGIC     'i'   

/*
 * enum for cmd character 
 */
enum
{
    E_IPCS_SEND_MSG,
    E_IPCS_RECV_MSG,
    E_IPCS_CHECK_PEER,
    E_IPCS_SET_LOCAL,
};

struct ipcs_send_msg_priv
{
    unsigned int channel_id;
    unsigned int ack;
    unsigned int msg_id;
    unsigned int param1;
    unsigned int param2;
};

struct ipcs_recv_msg_priv
{
    unsigned int channel_id;
    unsigned int ack;
    phys_addr_t p_msg_id;
    phys_addr_t p_param1;
    phys_addr_t p_param2;
};

struct ipcs_peer_status_priv
{
    unsigned int peer_status;
    unsigned int local_status;
};

#define CMD_IPCS_SEND_MSG    _IOWR(HAL_IPCS_IOC_MAGIC,E_IPCS_SEND_MSG, struct ipcs_send_msg_priv)

#define CMD_IPCS_RECV_MSG    _IOWR(HAL_IPCS_IOC_MAGIC,E_IPCS_RECV_MSG, struct ipcs_recv_msg_priv)

#define CMD_IPCS_CHECK_PEER  _IOWR(HAL_IPCS_IOC_MAGIC,E_IPCS_CHECK_PEER, struct ipcs_peer_status_priv)

#define CMD_IPCS_SET_LOCAL   _IOWR(HAL_IPCS_IOC_MAGIC,E_IPCS_SET_LOCAL, struct ipcs_peer_status_priv)

#endif
