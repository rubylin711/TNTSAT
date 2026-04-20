/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_CIPHER_H__
#define __MT_UNF_CIPHER_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum  _MT_CIPHER_OPERATION_E
{
    MT_CIPHER_OPERATION_DECRYPT,
    MT_CIPHER_OPERATION_ENCRYPT,
} MT_CIPHER_OPERATION_E;

/*!
  Cipher algorithm
  */
typedef enum  _MT_CIPHER_ALG_E
{
	MT_CIPHER_ALG_DES,
	MT_CIPHER_ALG_TDES,
	MT_CIPHER_ALG_AES,
	MT_CIPHER_ALG_RSA,
	MT_CIPHER_ALG_BUTT,
} MT_CIPHER_ALGORITHM_E;

/*!
  Cipher work mode
  */
typedef enum  _MT_CIPHER_WORK_MODE_E
{
	MT_CIPHER_WORK_MODE_ECB,
	MT_CIPHER_WORK_MODE_CBC,
	MT_CIPHER_WORK_MODE_CTR,
	MT_CIPHER_WORK_MODE_CBCDVS042,
	MT_CIPHER_WORK_MODE_CBCCTS,
	MT_CIPHER_WORK_MODE_RCBC,
	MT_CIPHER_WORK_MODE_ECBCTS,
	MT_CIPHER_WORK_MODE_CFB,
	MT_CIPHER_WORK_MODE_CFB8,
	MT_CIPHER_WORK_MODE_CFB64_128,
	MT_CIPHER_WORK_MODE_OFB,
	MT_CIPHER_WORK_MODE_OFB8,
	MT_CIPHER_WORK_MODE_OFB64_128,
} MT_CIPHER_WORK_MODE_E;

/*!
  Cipher key ladder source
  */
typedef enum  _MT_CIPHER_KEY_LADDER_SOURCE_E
{
	MT_CIPHER_KEY_LADDER_FLASH,
	MT_CIPHER_KEY_LADDER_UNIQUE,
	MT_CIPHER_KEY_LADDER_COMMON,
	MT_CIPHER_KEY_LADDER_CW,
	MT_CIPHER_KEY_LADDER_PVR,
	MT_CIPHER_KEY_LADDER_DATA,
	MT_CIPHER_KEY_LADDER_HDCP,
	//new added
	MT_CIPHER_KEY_LADDER_ESWCK,
	MT_CIPHER_KEY_LADDER_RESERVED,
} MT_CIPHER_KEY_LADDER_SOURCE_E;

/*!
  RSA algrithm type
  */
typedef enum _MT_CIPHER_RSA_KEY_LENGTH_E
{
	MT_CIPHER_RSA_KEY_LENGTH_1024 = 1024,
	MT_CIPHER_RSA_KEY_LENGTH_2048 = 2048,
} MT_CIPHER_RSA_KEY_LENGTH_E;

/*!
  Hash algrithm type
  */
typedef enum _MT_CIPHER_HASH_TYPE_E
{
	MT_CIPHER_HASH_TYPE_SHA1,
	MT_CIPHER_HASH_TYPE_SHA224,
	MT_CIPHER_HASH_TYPE_SHA256,
	MT_CIPHER_HASH_TYPE_SHA384,
	MT_CIPHER_HASH_TYPE_SHA512,
} MT_CIPHER_HASH_TYPE_E;

/*!
  hmac algrithm type
  */
typedef enum _MT_CIPHER_HMAC_TYPE_E
{
	MT_CIPHER_HMAC_TYPE_SHA1,
	MT_CIPHER_HMAC_TYPE_SHA256,
	MT_CIPHER_HMAC_BUTT,
} MT_CIPHER_HMAC_TYPE_E;

typedef struct
{
	mt_u8 *p_hmac_key;
	mt_u32 key_len;
	MT_CIPHER_HMAC_TYPE_E hmac_type;
} MT_CIPHER_HMAC_ATTS_S;

/*!
  CW type
  */
typedef enum _MT_CIPHER_CW_TYPE_E
{
	MT_CIPHER_CW_EVEN_KEY,
	MT_CIPHER_CW_ODD_KEY,
} MT_CIPHER_CW_TYPE_E;


/*!
  Cipher special commands
  */
typedef enum  _MT_CIPHER_SPECIAL_COMMAND_E
{
	MT_CIPHER_COMMAND_INVALID,
	MT_CIPHER_COMMAND_DDR_READY,
	MT_CIPHER_COMMAND_R_REG,
	MT_CIPHER_COMMAND_W_REG,
	MT_CIPHER_COMMAND_R_OTP,
	MT_CIPHER_COMMAND_W_OTP,
	MT_CIPHER_COMMAND_RT_APCPU_SETTING,
	MT_CIPHER_COMMAND_RT_AVCPU_SETTING,
	MT_CIPHER_COMMAND_RT_SECCPU_SETTING,
	MT_CIPHER_COMMAND_RT_APPC_SETTING,
	MT_CIPHER_COMMAND_RUN_SOS,
	MT_CIPHER_COMMAND_MBOOT_VER_CHECK,
	MT_CIPHER_COMMAND_APP_VER_CHECK,
	MT_CIPHER_COMMAND_AV_VER_CHECK,
	MT_CIPHER_COMMAND_DDR_PROTECT,
	MT_CIPHER_COMMAND_SET_KEY_DERIV_PARAS,
	MT_CIPHER_COMMAND_SET_FLASHHW_KEY,
	MT_CIPHER_COMMAND_SET_LOCK_MODULES,
} MT_CIPHER_SPECIAL_COMMAND_E;


/** Structure of the DES algorithm parameter */
typedef struct _MT_CIPHER_DES_PARA_S
{
	mt_u8 iv[16];
	mt_u8 *p_key;
	mt_u32 key_length;
	MT_CIPHER_WORK_MODE_E work_mode;
} MT_CIPHER_DES_PARA_S;

/** Structure of the AES algorithm parameter */
typedef struct _MT_CIPHER_AES_PARA_S
{
	mt_u8 iv[16];
	mt_u8 *p_key;
	mt_u32 key_length;
	MT_CIPHER_WORK_MODE_E work_mode;
} MT_CIPHER_AES_PARA_S;

/** Structure of the RSA algorithm parameter */
typedef struct _MT_CIPHER_RSA_PARA_S
{
	mt_u8 *p_m;
	mt_u8 *p_e;
	MT_CIPHER_RSA_KEY_LENGTH_E key_length;
} MT_CIPHER_RSA_PARA_S;

/** Structure of the cipher control information */
typedef struct _MT_CIPHER_CTRL_S
{
	MT_CIPHER_OPERATION_E operation;
	MT_CIPHER_ALGORITHM_E algorithm;
	union
	{
		MT_CIPHER_DES_PARA_S des_para;
		MT_CIPHER_DES_PARA_S tdes_para;
		MT_CIPHER_AES_PARA_S aes_para;
		MT_CIPHER_RSA_PARA_S rsa_para;
	}parameter;
} MT_CIPHER_CTRL_S;

/** Structure of the cipher control word information */
typedef struct _MT_CIPHER_CW_INFO_S
{
	MT_CIPHER_CW_TYPE_E type;
	mt_u32 descrambler_id;
	mt_u16 offset;
	mt_u16 length;
} MT_CIPHER_CW_INFO_S;

/*!
  Init the cipher device

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_init(void);

/*!
  Deinit the cipher device

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_deinit(void);

/*!
  Obtain a keyladder handle

  \param[in] (keyladder_source) keyladder source
  \param[in] (index) source key select
  \param[out] (p_keyladder) created key ladder handle

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_keyladder_create(MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source, mt_u32 index, mt_handle *p_keyladder);

/*!
  Destroy the existing keyladder handle

  \param[in] (keyladder) the keyladder handle will be destroied

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_keyladder_destroy(mt_handle keyladder);

/*!
  Link new level to keyladder

  \param[in] (keyladder) key ladder handle
  \param[in] (p_ctrl) cipher attribute
  \param[in] (input) input buffer
  \param[in] (length) input length

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_keyladder_link(mt_handle keyladder, MT_CIPHER_CTRL_S *p_ctrl, mt_u8 *input, mt_u32 length);

/*!
  Create hash operation handle

  \param[in] (p_attr) hash type
  \param[out] (p_handle) created cipher hash handle

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, mt_handle *p_handle);

/*!
  Calculate the hash

  \param[in] (handle) hash handle
  \param[in] (p_data) input data
  \param[in] (length) length of data

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_hash_update(mt_handle handle, mt_u8 *p_data, mt_u32 length);

/*!
  Get the final hash value

  \param[in] (handle) hash handle
  \param[out] (p_output_hash) final output hash value

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_hash_final(mt_handle handle, mt_u8 *p_output_hash);

/*!
  Create hmac operation handle

  \param[in] (p_hmac_attr) hmac info
  \param[out] (p_handle) created cipher hash handle

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_hmac_create(MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, mt_handle *p_handle);

/*!
  Calculate the hmac

  \param[in] (handle) hash handle
  \param[in] (p_data) input data
  \param[in] (length) length of data

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_hmac_update(mt_handle handle, mt_u8 *p_data, mt_u32 length);

/*!
  Get the final hmac value

  \param[in] (handle) hash handle
  \param[out] (p_output_hmac) final output hmac value

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_hmac_final(mt_handle handle, mt_u8 *p_output_hmac);

/*!
  Obtain a cipher handle for encryption decryption and hash operation

  \param[in] (key_ladder_handle) if using key ladder, set the handle of key ladder
  \param[out] (ph_cipher) created cipher handle

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_create(mt_handle key_ladder_handle, mt_handle *p_cipher);

/*!
  Destroy the existing cipher handle

  \param[in] (h_cipher) the cipher handle will be destroied

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_destroy(mt_handle cipher);

/*!
  Configures the cipher control information

  \param[in] (cipher) cipher handle
  \param[in] (p_ctrl) cipher attributes

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_config(mt_handle cipher, MT_CIPHER_CTRL_S *p_ctrl);

/*!
  Performs encryption or decryption

  \param[in] (cipher) cipher handle
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (length) length of data

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_process(mt_handle cipher, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length);
/*!
  Set CW to descrambler

  \param[in] (cipher) cipher handle
  \param[in] (p_cw_info) CW information
  \param[in] (src_addr) source data address
  \param[in] (length) length of data

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_set_cw(mt_handle cipher, MT_CIPHER_CW_INFO_S *p_cw_info, mt_u8 *p_src_addr, mt_u32 length);

/*!
  Get a random number

  \param[out] (p_random_number) point to the random number

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_get_random_number(mt_u32 *p_random_number);
/*!
  Get PVR key

  \param[in] (channel_id) pvr channel id
  \param[in] (key_length) pvr key length
  \param[out] (p_pvr_key) pvr key

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_get_pvr_key(mt_u32 channel_id, mt_u32 key_length, mt_u8 *p_pvr_key);

/*!
  Do special command

  \param[in] (cmd_id) command id
  \param[in] (p_input) input buffer
  \param[in] (in_len) input buffer length
  \param[in] (p_output) output buffer
  \param[in] (out_len) output buffer length

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_special_command(MT_CIPHER_SPECIAL_COMMAND_E cmd_id, void *p_input, mt_u32 in_len, void *p_output, mt_u32 out_len);

/*!
  send message to secure firmware

  \param[in] (channel_id) channel id (0-2)
  \param[in] (wait_ack) 1: wait ack, sync message 0: not
  \param[in] (msg_id) message id
  \param[in] (param1) message param1
  \param[in] (param2) message param2

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_send_message(mt_u32 channel_id, mt_u32 wait_ack, mt_u32 msg_id, mt_u32 param1, mt_u32 param2);

/*!
  receive message from secure firmware

  \param[in] (channel_id) channel id (0-2)
  \param[in] (send_ack) 1: send ack, sync message 0: not
  \param[out] (p_msg_id) pointer to message id
  \param[out] (p_param1) pointer to message param1
  \param[out] (p_param2) pointer to message param2

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_receive_message(mt_u32 channel_id, mt_u32 send_ack, mt_u32 *p_msg_id, mt_u32 *p_param1, mt_u32 *p_param2);

/*!
  Load an encrypted hdcp key.

  \param[in] (p_enc_hdcpkey) the encrypted hdcpkey to be loaded.
  \param[in] (len) it should be 304 bytes.

  \return SUCCESS, else fail
  */
mt_s32 MT_UNF_CIPHER_load_hdcpkey(mt_u8 *p_enc_hdcpkey, mt_u32 len);

/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_CIPHER_H__ */
