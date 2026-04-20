/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_SEE_CIPHER_H__
#define __MT_UNF_SEE_CIPHER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define SEE_CIPHER_SUCCESS      (0)
#define SEE_CIPHER_ERROR      (-1)
#define SEE_CIPHER_LEN_ERR      (-2)
#define SEE_CIPHER_ADDR_ERR      (-3)
#define SEE_CIPHER_MB_ERR      (-4)




/**
 * @brief 
 *
 * @param input_addr
 * @param input_max
 * @param output_addr
 * @param output_max
 *
 * @return 
 */
int mt_unf_see_cipher_init(unsigned int input_addr, unsigned int input_max, unsigned int output_addr, unsigned int output_max);


/**
 * @brief 
 *
 * @param p_ctrl
 * @param p_key_attr
 * @param p_kattr_size
 *
 * @return 
 */
int mt_unf_see_key_attr_create(MT_CIPHER_CTRL_S *p_ctrl, unsigned int *p_key_attr, unsigned int * p_kattr_size);

/**
 * @brief 
 *
 * @param p_key_attr
 */
void mt_unf_see_key_attr_destroy(unsigned int *p_key_attr);

/**
 * @brief 
 *
 * @param msg_cmd
 * @param p_input
 * @param input_len
 * @param p_output
 * @param p_output_len
 *
 * @return 
 */
int mt_unf_see_cipher_transfer(mt_u32 msg_cmd, mt_u8 *p_input, mt_u32 input_len, mt_u8 *p_output, mt_u32 *p_output_len);


/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_SEE_CIPHER_H__ */
