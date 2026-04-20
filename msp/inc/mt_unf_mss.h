/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_MSS_H__
#define __MT_UNF_MSS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_unf_cipher_v2.h"


/*********************************************************************
 * ==========================Communication between CPUs==============*
 ********************************************************************/
/*!
 * \brief: Initialize the necessary components.
 *
 * \return MT_SUCCESS
 */
int mt_unf_mss_see_init(void);

/*!
 * \brief: De-initialize the components.
 *
 * \return MT_SUCCESS
 */
int mt_unf_mss_see_deinit(void);

/*!
 * \brief: Channels to send/recv messages
 */
typedef enum
{
    MT_MSG_CH0 = 0,
    MT_MSG_CH1,
    MT_MSG_CH2,
    MT_MSG_CH3,
    MT_MSG_CH4,
    MT_MSG_CH5,
    MT_MSG_CH6,
    MT_MSG_CH7
} MT_MSG_CH_E;

typedef struct
{
    MT_MSG_CH_E channel_id;
    unsigned int ack;       /* need ack from peer or not */ 
    unsigned int msg_id;    /* pre-defined message id negotiated with peer-CPU */
    unsigned int param1;    /* parameter1 if has */
    unsigned int param2;    /* parameter2 if has */
} MT_PEER_MSG_S;

/*!
 * \brief: Receive message from peer-CPU.
 *
 * \param[in][out] p_msg: pointer of msg, input channel_id, ack, output msg_id,param1,param2. 
 *
 * \return: MT_SUCCESS or failed. 
 */
int mt_unf_mss_see_recv_peer_msg(MT_PEER_MSG_S *p_msg);

/*!
 * \brief: Send message to peer-CPU.
 *
 * \param[in] p_msg: pointer of msg.
 *
 * \return MT_SUCCESS or failed.
 */
int mt_unf_mss_see_send_peer_msg(MT_PEER_MSG_S *p_msg);


/*!
 * \brief: Exchange information with peer-CPU.
 *
 * \param[in] msg_cmd: pre-define command id between peers.
 * \param[in] p_input: pointer to input message.
 * \param[in] input_len: length of input message.
 * \param[out] p_output: pointer to output message.
 * \param[out] p_output_len: pointer to output message length.
 *
 * \return  MT_SUCCESS or failed.
 */
int mt_unf_mss_see_peer_exchange(unsigned int msg_cmd, unsigned char *p_input, unsigned int input_len, unsigned char *p_output, unsigned int *p_output_len);


/*!
 * \brief: Create key attribute based on the p_ctrl.
 *
 * \param[in] p_ctrl: control info.
 * \param[out] p_key_attr: pointer of key attribute.
 * \param[out] p_kattr_size: attribute size.
 *
 * \return MT_SUCCESS or failed.
 */
int mt_unf_mss_see_keyattr_create(MT_CIPHER_CTRL_S *p_ctrl, unsigned int *p_key_attr, unsigned int * p_kattr_size);

/*!
 * \brief: Free the key attribute that created by key_attr_create.
 *
 * \param[in] p_key_attr: pointer of key attribute.
 */
void mt_unf_mss_see_keyattr_destroy(unsigned int *p_key_attr);

typedef enum {
    PEER_CPU_NOT_READY,
    PEER_CPU_READY,
    LOCAL_CPU_NOT_READY,
    LOCAL_CPU_READY,
} MT_CPU_STATE_E;

/*!
 * \brief: Check if peer-CPU is ready.
 *
 * \return xxx_READY or xxx_NOT_READY.
 */
MT_CPU_STATE_E mt_unf_mss_see_check_peer_state(void);

/*!
 * \brief: Set local CPU status, to notify peer-CPU.
 *
 * \param[in] sta: specify LOCAL_CPU_xxx.
 *
 * NOTE:
 * Once this function is called, the peer cpu will know the local cpu is ready.
 * This is the only way to notify the peer cpu. So, this function must be called 
 * only after all the necessay work has been done by the local cpu.
 */
void mt_unf_mss_see_set_local_state(MT_CPU_STATE_E sta);

/*!
 * \brief: Lock peer-CPU on shared resources, e.g. CryptoEngine.
 *
 * \return MT_SUCCESS or failed.
 */
int mt_unf_mss_see_lock_peer(void);

/*!
 * \brief: Un-lock peer-CPU on shared resources.
 *
 * \return MT_SUCCESS or failed.
 */
int mt_unf_mss_see_unlock_peer(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_MSS_ H*/
