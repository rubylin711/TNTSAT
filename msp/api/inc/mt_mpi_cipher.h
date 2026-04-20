/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_CIPHER_H__
#define __MT_MPI_CIPHER_H__

/******************************* API Declaration *****************************/

/*!
  Init the cipher device
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_init(void);

/*!
  Deinit the cipher device
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_deinit(void);

/*!
  Obtain a keyladder handle
  
  \param[in] (keyladder_source) keyladder source
  \param[in] (index) source key select
  \param[out] (p_keyladder) created key ladder handle
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_keyladder_create(MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source, mt_u32 index, mt_handle *p_keyladder);

/*!
  Destroy the existing keyladder handle
  
  \param[in] (keyladder) the keyladder handle will be destroied
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_keyladder_destroy(mt_handle keyladder);

/*!
  Link new level to keyladder
  
  \param[in] (keyladder) key ladder handle
  \param[in] (p_ctrl) cipher attribute
  \param[in] (input) input buffer
  \param[in] (length) input length
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_keyladder_link(mt_handle keyladder, MT_CIPHER_CTRL_S *p_ctrl, mt_u8 *input, mt_u32 length);

/*!
  Create hash operation handle
  
  \param[in] (p_attr) hash type
  \param[out] (p_handle) created cipher hash handle
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, 
    mt_handle *p_handle);

/*!
  Calculate the hash
  
  \param[in] (handle) hash handle
  \param[in] (p_data) input data 
  \param[in] (length) length of data
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_hash_update(mt_handle handle, mt_u8 *p_data, mt_u32 length);

/*!
  Get the final hash value
  
  \param[in] (handle) hash handle
  \param[out] (p_output_hash) final output hash value
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_hash_final(mt_handle handle, mt_u8 *p_output_hash);

/*!
  Obtain a cipher handle for encryption decryption and hash operation

  \param[in] (key_ladder_handle) if using key ladder, set the handle of key ladder
  \param[out] (ph_cipher) created cipher handle
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_create(mt_handle key_ladder_handle, mt_handle *p_cipher);

/*!
  Destroy the existing cipher handle
  
  \param[in] (h_cipher) the cipher handle will be destroied
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_destroy(mt_handle cipher);

/*!
  Configures the cipher control information
  
  \param[in] (cipher) cipher handle
  \param[in] (p_ctrl) cipher attributes
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_config(mt_handle cipher, MT_CIPHER_CTRL_S *p_ctrl);

/*!
  Performs encryption or decryption 
  
  \param[in] (cipher) cipher handle
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (length) length of data
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_process(mt_handle cipher, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length);

/*!
  Set CW to descrambler
  
  \param[in] (cipher) cipher handle
  \param[in] (p_cw_info) CW information
  \param[in] (src_addr) source data address
  \param[in] (length) length of data
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_set_cw(mt_handle cipher, MT_CIPHER_CW_INFO_S *p_cw_info, mt_u8 *p_src_addr, mt_u32 length);

/*!
  Get a random number
  
  \param[out] (p_random_number) point to the random number
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_get_random_number(mt_u32 *p_random_number);
/*!
  Get PVR key
  
  \param[in] (channel_id) pvr channel id
  \param[in] (key_length) pvr key length
  \param[out] (p_pvr_key) pvr key
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_get_pvr_key(mt_u32 channel_id, mt_u32 key_length, mt_u8 *p_pvr_key);

/*!
  Do special command
  
  \param[in] (cmd_id) command id
  \param[in] (p_input) input buffer
  \param[in] (in_len) input buffer length
  \param[in] (p_output) output buffer 
  \param[in] (out_len) output buffer length
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_special_command(MT_CIPHER_SPECIAL_COMMAND_E cmd_id, void *p_input, mt_u32 in_len, void *p_output, mt_u32 out_len);

/*!
  send message to secure firmware
  
  \param[in] (channel_id) channel id (0-2)
  \param[in] (wait_ack) 1: wait ack, sync message 0: not
  \param[in] (msg_id) message id
  \param[in] (param1) message param1
  \param[in] (param2) message param2
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_send_message(mt_u32 channel_id, mt_u32 wait_ack, mt_u32 msg_id, mt_u32 param1, mt_u32 param2);

/*!
  receive message from secure firmware
  
  \param[in] (channel_id) channel id (0-2)
  \param[in] (send_ack) 1: send ack, sync message 0: not
  \param[out] (p_msg_id) pointer to message id
  \param[out] (p_param1) pointer to message param1
  \param[out] (p_param2) pointer to message param2
  
  \return SUCCESS, else fail
  */
mt_s32 MT_MPI_CIPHER_receive_message(mt_u32 channel_id, mt_u32 send_ack, mt_u32 *p_msg_id, mt_u32 *p_param1, mt_u32 *p_param2);


/*!
 * Load an encrypted hdcp-key
 * \param[in] (p_enc_hdcpkey) encrypted hdcp-key to set
 * \param[in] (len) length of the encrypted hdcp-key
 */
mt_s32 MT_MPI_CIPHER_load_hdcpkey(mt_u8 *p_enc_hdcpkey, mt_u32 len);

#endif
