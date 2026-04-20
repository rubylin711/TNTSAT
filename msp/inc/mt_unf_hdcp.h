/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_HDCP_H__
#define __MT_UNF_HDCP_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
typedef struct mtUNF_HDCPKEY_HDCP_S
{
    MT_BOOL enc_flag; //if true, the input hdcpkey is encrypted.
    mt_u8 ext_encrypt_key[16];  //AES-ECB key used to encrypt the clear hdcpkey outside the STB(Ignore this, we use a default key to enc/dec.).
    mt_u8 hdcpkey[304]; //hdcpkey encrypted or clear, depends on enc_flag.
} MT_UNF_HDCP_HDCPKEY_S;

/**
\brief Encrypt HDCP key. HDCP key is encrypted using hdcp root key in otp.
\param[in]:st_hdcpkey.This parameter is used to define hdcp key in encrypted mode or clear mode.  CNcomment: 该参数为HDCP key的参数定义,包含加密/非加密的方式. CNend
\param[out]:out_encrypted_key. Output the encrypted hdcp key.   CNcomment: 输出的加密后的HDCP KEY参数。 CNend
\retval MT_SUCCESS  Call this API succussful.  	CNcomment:API系统调用成功。 CNend
\retval MT_FAILURE  Call this API fails.  		CNcomment:API系统调用失败。 CNend
*/
mt_s32 MT_UNF_HDCP_encrypt_hdcpkey(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[304]);


/**
\brief Encrypt HDCP key. HDCP key is encrypted using hdcp root key in otp.
\param[in]:st_hdcpkey.This parameter is used to define hdcp key in encrypted mode or clear mode.  CNcomment: 该参数为HDCP key的参数定义,包含加密/非加密的方式. CNend
\param[out]:out_encrypted_key. Output the encrypted hdcp key.   CNcomment: 输出的加密后的HDCP KEY参数。 CNend
\retval MT_SUCCESS  Call this API succussful.  	CNcomment:API系统调用成功。 CNend
\retval MT_FAILURE  Call this API fails.  		CNcomment:API系统调用失败。 CNend
*/
mt_s32 MT_UNF_HDCP_encrypt_hdcpkey_tee(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[304]);


/**
\brief Load encrypted HDCP key. HDCP key is encrypted using hdcp root key in otp. TEE will decrypt the key and load clear hdcp key into HDMI SRAM.
\param[in]:in_encrypted_key. input the encrypted hdcp key.   CNcomment: input encrypted hdcp key. 
\retval MT_SUCCESS  Call this API succussful.  	CNcomment:API系统调用成功。 CNend
\retval MT_FAILURE  Call this API fails.  		CNcomment:API系统调用失败。 CNend
*/
mt_s32 MT_UNF_HDCP_load_hdcpkey(mt_u8 in_encrypted_key[304]);

/** @} */ /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_HDCP_H__ */
