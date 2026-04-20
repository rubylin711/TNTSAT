/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_CIPHER_H__
#define __MT_UNF_CIPHER_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


/*!
 * Cipher error code
 */
enum _MT_CIPHER_ERR_E {
    MT_CIPHER_ERR_NOT_INITIALIZED       = -255,
    MT_CIPHER_ERR_FEATURE_NOT_SUPPORT,
    MT_CIPHER_ERR_INVALID_HANDLE,
    MT_CIPHER_ERR_MULTIPLE_HANDLE,              /*Only one handle is supported at a time*/
    MT_CIPHER_ERR_BAD_PARAMETERS,
    MT_CIPHER_ERR_BUFFER_ALLOCATE_FAILED,

    MT_CIPHER_ERR_KT_REQUEST_FAILED,        /*request/release key slot failed*/
    MT_CIPHER_ERR_KT_RELEASE_FAILED,
    MT_CIPHER_ERR_KT_SET_ERR,

    MT_CIPHER_ERR_CRYPTO_CREATE,
    MT_CIPHER_ERR_CRYPTO_CONFIG,
    MT_CIPHER_ERR_CRYPTO_PROCESS,

    MT_CIPHER_ERR_RSA_CREATE,
    MT_CIPHER_ERR_RSA_CONFIG,
    MT_CIPHER_ERR_RSA_PROCESS,

    MT_CIPHER_ERR_HASH_CREATE,
    MT_CIPHER_ERR_HASH_UPDATE,
    MT_CIPHER_ERR_HASH_FINAL,

    MT_CIPHER_ERR_MAC_CREATE,
    MT_CIPHER_ERR_MAC_UPDATE,
    MT_CIPHER_ERR_MAC_FINAL,

    MT_CIPHER_ERR_BN_CREATE,
    MT_CIPHER_ERR_BN_DESTROY,
    MT_CIPHER_ERR_BN_MODMOD,
    MT_CIPHER_ERR_BN_MODADD,
    MT_CIPHER_ERR_BN_MODSUB,
    MT_CIPHER_ERR_BN_MODMUL,
    MT_CIPHER_ERR_BN_MODEXP,
    MT_CIPHER_ERR_BN_MODINV,

    MT_CIPHER_ERR_ECP_CREATE,
    MT_CIPHER_ERR_ECP_DESTROY,
    MT_CIPHER_ERR_ECP_ADD,
    MT_CIPHER_ERR_ECP_MUL,

    MT_CIPHER_ERR_BGC_REQUEST,
    MT_CIPHER_ERR_BGC_RELEASE,
};

/*!
  Cipher function enable
 */
typedef enum _CIPHRE_ENABLE_E {
    MT_CIPHER_DISABLE = 0x0,
    MT_CIPHER_ENABLE
} MT_CIPHER_ENABLE_E;

/*!
  Cipher operation
  */
typedef enum  _MT_CIPHER_OPERATION_E
{
    MT_CIPHER_OPERATION_DECRYPT,
    MT_CIPHER_OPERATION_ENCRYPT,
}MT_CIPHER_OPERATION_E;

/*!
  Cipher algorithm
  */
typedef enum  _MT_CIPHER_ALG_E
{
    MT_CIPHER_ALG_DES,
    MT_CIPHER_ALG_TDES,
    MT_CIPHER_ALG_AES,
    MT_CIPHER_ALG_HMAC,
    MT_CIPHER_ALG_CSA2,
    MT_CIPHER_ALG_CSA3,
    MT_CIPHER_ALG_AES256,
    MT_CIPHER_ALG_HMAC256,
    MT_CIPHER_ALG_SM4,
    MT_CIPHER_ALG_BUTT,
}MT_CIPHER_ALGORITHM_E;

/*!
  Cipher Core
  */
typedef enum  _MT_CIPHER_CORE_E
{
    MT_CIPHER_CORE_M2M_RAW,
    MT_CIPHER_CORE_M2M_TS,
    MT_CIPHER_CORE_DSC_TS,
}MT_CIPHER_CORE_E;

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
    MT_CIPHER_WORK_MODE_OFB,
    MT_CIPHER_WORK_MODE_CBCS,
    MT_CIPHER_WORK_MODE_CENS,
    MT_CIPHER_WORK_MODE_UNKNOWN,
}MT_CIPHER_WORK_MODE_E;

/*!
 * Keyladder type: 0, or 1
 */
typedef enum  _MT_CIPHER_KEYLADDER_TYPE_E
{
    MT_CIPHER_KEYLADDER_0,
    MT_CIPHER_KEYLADDER_1,
    MT_CIPHER_KEYLADDER_UNKNOWN,
} MT_CIPHER_KEYLADDER_TYPE_E;

/*!
  Cipher key ladder source
  */
typedef enum  _MT_CIPHER_KEYLADDER_SOURCE_E
{
    MT_CIPHER_KEYLADDER_SCK_0          = 0x0,
    MT_CIPHER_KEYLADDER_SCK_1,
    MT_CIPHER_KEYLADDER_SCK_2,
    MT_CIPHER_KEYLADDER_SCK_3,
    MT_CIPHER_KEYLADDER_SCK_4,
    MT_CIPHER_KEYLADDER_SCK_5,
    MT_CIPHER_KEYLADDER_SCK_6,
    MT_CIPHER_KEYLADDER_SCK_7,
    MT_CIPHER_KEYLADDER_SCK_8,
    MT_CIPHER_KEYLADDER_SCK_9,
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_CIPHER_KEYLADDER_SCK_10,
    MT_CIPHER_KEYLADDER_SCK_11,
    MT_CIPHER_KEYLADDER_SCK_12,
    MT_CIPHER_KEYLADDER_SCK_13,
    MT_CIPHER_KEYLADDER_SCK_14,
    MT_CIPHER_KEYLADDER_SCK_15,
    MT_CIPHER_KEYLADDER_HWSCK_0,
    MT_CIPHER_KEYLADDER_HWSCK_1,
    MT_CIPHER_KEYLADDER_HWSCK_2,
    MT_CIPHER_KEYLADDER_HWSCK_3,
    MT_CIPHER_KEYLADDER_HWSCK_4,
    MT_CIPHER_KEYLADDER_PRIVATE_0      = 0x18,
    MT_CIPHER_KEYLADDER_PRIVATE_1      = 0x19,
    /* 0x1a ~ 0x1b only for CRI mode */
    MT_CIPHER_KEYLADDER_CFAES_KEY      = 0x1a,
    MT_CIPHER_KEYLADDER_CFCWC          = 0x1b,
#else
    MT_CIPHER_KEYLADDER_PRIVATE_0      = 0xa,
    MT_CIPHER_KEYLADDER_PRIVATE_1      = 0xb,
#endif
    MT_CIPHER_KEYLADDER_SCK_UNKNOWN,
} MT_CIPHER_KEYLADDER_SOURCE_E;

/*!
  Cipher key ladder store key
  */
typedef enum _MT_CIPHER_KEYLADDER_STORE_E
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_CIPHER_KEYLADDER_STORE_PRIVATE0      = 0x14,
    MT_CIPHER_KEYLADDER_STORE_PRIVATE1      = 0x15,
    /* 0x18 ~ 0x1b only for CRI & Conax mode */
    MT_CIPHER_KEYLADDER_STORE_CWCW0         = 0x18,
    MT_CIPHER_KEYLADDER_STORE_CWCW1         = 0x19,
    MT_CIPHER_KEYLADDER_STORE_CWCW2         = 0x1a,
    MT_CIPHER_KEYLADDER_STORE_CWCW3         = 0x1b,
    /* 0x1e ~ 0x1f only for CRI mode */
    MT_CIPHER_KEYLADDER_STORE_CFAES_KEY     = 0x1e,
    MT_CIPHER_KEYLADDER_STORE_HWDECM        = 0x1f,
#else
    MT_CIPHER_KEYLADDER_STORE_PRIVATE0	    = 0x0,
    MT_CIPHER_KEYLADDER_STORE_PRIVATE1	    = 0x1,
#endif
} MT_CIPHER_KEYLADDER_STORE_E;

/*!
 * Crypto Channel
 */
typedef enum _MT_CIPHER_CRYPTO_CH_E
{
    MT_CIPHER_CRYPTO_CH_0,
    MT_CIPHER_CRYPTO_CH_1,
    MT_CIPHER_CRYPTO_CH_2,
    MT_CIPHER_CRYPTO_CH_3,
    MT_CIPHER_CRYPTO_CH_UNKNOWN,
} MT_CIPHER_CRYPTO_CH_E;

/*!
  RSA algrithm type
  */
typedef enum _MT_CIPHER_RSA_KEY_LENGTH_E
{
    MT_CIPHER_RSA_KEY_LENGTH_1024 = 1024,
    MT_CIPHER_RSA_KEY_LENGTH_2048 = 2048,
}MT_CIPHER_RSA_KEY_LENGTH_E;

typedef enum _MT_CIPHER_RSA_EXP_LENGTH_E
{
    MT_CIPHER_RSA_EXP_LENGTH_3 = 1,
    MT_CIPHER_RSA_EXP_LENGTH_17 = 17,
    MT_CIPHER_RSA_EXP_LENGTH_65537 = 65537,
} MT_CIPHER_RSA_EXP_LENGTH_E;

/*!
  Hash algrithm
  */
typedef enum _MT_CIPHER_HASH_TYPE_E
{
    MT_CIPHER_HASH_TYPE_SHA1,
    MT_CIPHER_HASH_TYPE_SHA224,
    MT_CIPHER_HASH_TYPE_SHA256,
    MT_CIPHER_HASH_TYPE_SHA384,
    MT_CIPHER_HASH_TYPE_SHA512,
    MT_CIPHER_HASH_TYPE_SM3,
}MT_CIPHER_HASH_TYPE_E;

/*!
  HMAC/CMAC/CBCMAC algrithm
  */
typedef enum _MT_CIPHER_MAC_TYPE_E
{
    MT_CIPHER_MAC_TYPE_SHA224,
    MT_CIPHER_MAC_TYPE_SHA256,
    MT_CIPHER_MAC_TYPE_SHA384,
    MT_CIPHER_MAC_TYPE_SHA512,
    MT_CIPHER_MAC_TYPE_SM3,
    MT_CIPHER_MAC_TYPE_CMAC_AES128,
    MT_CIPHER_MAC_TYPE_CMAC_SM4,
    MT_CIPHER_MAC_TYPE_SHA1,
}MT_CIPHER_MAC_TYPE_E;

typedef struct
{
    unsigned char *p_hmac_key;
    unsigned int key_len;
    unsigned int key_slot[2];
}MT_CIPHER_HMAC_ATTS_S;

/*!
 * Key Slot ID
 */
#define MT_CIPHER_KEYSLOT_INVALID 255

/*!
 * Structure of the cbcs/cens algorithm parameter
 */
typedef struct {
    unsigned int skip_blocks;
    unsigned int crypt_blocks;
} MT_CIPHER_CBCS_PARA_S;

/*!
 * Structure of the RSA algorithm parameter
 */
typedef struct _MT_CIPHER_RSA_PARA_S
{
    unsigned char *p_m;
    unsigned char *p_e;
    unsigned int key_length;
    unsigned int exp_length;
} MT_CIPHER_RSA_PARA_S;

typedef struct _MT_CIPHER_RSA_CTRL_S
{
    MT_CIPHER_OPERATION_E operation;
    MT_CIPHER_RSA_PARA_S rsa_para;
} MT_CIPHER_RSA_CTRL_S;

/*! 
 * Structure cipher control parameter 
 */
typedef struct _MT_CIPHER_CTRL_S
{
    MT_CIPHER_CORE_E core;
    MT_CIPHER_OPERATION_E operation;
    MT_CIPHER_ALGORITHM_E algorithm;
    MT_CIPHER_WORK_MODE_E work_mode;
    MT_CIPHER_CBCS_PARA_S cbcs_params;
} MT_CIPHER_CTRL_S;

typedef enum _MT_KT_PURPOSE_E
{
    MT_KT_PURPOSE_TS = (0x1 << 0),
    MT_KT_PURPOSE_M2M = (0x1 << 1),
    MT_KT_PURPOSE_MAC = (0x1 << 2),
}MT_KT_PURPOSE_E;

typedef enum _MT_KT_OPERATION_E
{
    MT_KT_OPERATION_DECRYPT = (0x1 << 0),
    MT_KT_OPERATION_ENCRYPT = (0x1 << 1),
}MT_KT_OPERATION_E;

typedef enum _MT_KT_ALG_E
{
    MT_KT_ALG_DES = (0x1 << 0),
    MT_KT_ALG_TDES = (0x1 << 1),
    MT_KT_ALG_AES = (0x1 << 2),
    MT_KT_ALG_CSA2 = (0x1 << 3),
    MT_KT_ALG_CSA3 = (0x1 << 4),
    MT_KT_ALG_SM4 = (0x1 << 5),
    MT_KT_ALG_GOST28147 = (0x1 << 6),
    MT_KT_ALG_GOSTR34 = (0x1 << 7),
}MT_KT_ALG_E;

typedef enum _MT_KT_KEYSIZE_E
{
    MT_KT_KEYSIZE_64 = (0x1 << 0),
    MT_KT_KEYSIZE_128 = (0x1 << 1),
    MT_KT_KEYSIZE_256 = (0x1 << 2),
}MT_KT_KEYSIZE_E;

typedef struct _MT_KT_CTRL_S
{
    unsigned int purpose;
    unsigned int operation;
    unsigned int algorithm;
    unsigned int keysize;
} MT_KT_CTRL_S;

/*************************************************
 * TS Parameters
 ************************************************/
typedef enum _MT_TS_ENC_KSEL_E
{
    MT_TS_ENC_ODD_KEY,				// 0,
    MT_TS_ENC_EVEN_KEY,				// 1,
} MT_TS_ENC_KSEL_E;

typedef enum  _MT_TS_DEC_IND_E
{
    MT_TS_DEC_IND_CLEAR,			// 0,
    MT_TS_DEC_IND_KEEP,			    // 1,
} MT_TS_DEC_IND_E;

typedef enum _MT_TS_PKT_LEN_E
{
    MT_TS_LEN_PKT_188BYTE,			// 0,
    MT_TS_LEN_PKT_192BYTE,			// 1,
} MT_TS_PKT_LEN_E;

typedef enum _MT_TS_IVE_MODE_E
{
    MT_TS_IVE_MDI,				// 0,
    MT_TS_IVE_MDD,				// 1,
    MT_TS_IVE_MSC,				// 2,
    MT_TS_IVE_OFF,              // choose OFF to use user input IV
} MT_TS_IVE_MODE_E;

typedef enum _MT_TS_SHORT_MODE_E
{
    MT_TS_SHORT_HEAD,			// 0, shorter data in head
    MT_TS_SHORT_TAIL,			// 1, shorter data in tail
} MT_TS_SHORT_MODE_E;

typedef enum _MT_TS_SMALL_MODE_E
{
    MT_TS_SMALL_CLEAR,			    // 0,   keep clear
    MT_TS_SMALL_DVS042_TAIL,		// 1,   dvs042
    MT_TS_SMALL_XOR_IVE,			// 2,   xor with iv
} MT_TS_SMALL_MODE_E;

typedef enum _MT_TS_PID_FILT_EN_E
{
    MT_TS_PID_FILT_DISABLE,			// 0,
    MT_TS_PID_FILT_ENABLE,			// 1,
} MT_TS_PID_FILT_EN_E;

typedef enum _MT_TS_FORCE_ENC_EN_E
{
    MT_TS_FORCE_ENC_DISABLE,		// 0,
    MT_TS_FORCE_ENC_ENABLE,			// 1,
} MT_TS_FORCE_ENC_EN_E;

typedef struct _MT_CIPHER_TS_PARA_S
{
    MT_TS_ENC_KSEL_E ts_enc_ksel;               /* select key for encryption */
    MT_TS_DEC_IND_E ts_dec_ind;                 /* clear indicator or keep */
    MT_TS_PKT_LEN_E ts_pkt_len;                 /* select packet length */
    MT_TS_IVE_MODE_E ts_ive_mode;               /* select IV mode. If OFF, use user input IV instead */
    MT_TS_SHORT_MODE_E ts_short_mode;           /* see MT_TS_SHORT_MODE_E */
    MT_TS_SMALL_MODE_E ts_small_mode;           /* how to process shorter part, see MT_TS_SMALL_MODE_E */

    MT_TS_PID_FILT_EN_E ts_pid0_filt_en;
    MT_TS_PID_FILT_EN_E ts_pid1_filt_en;
    MT_TS_PID_FILT_EN_E ts_pid2_filt_en;
    MT_TS_PID_FILT_EN_E ts_pid3_filt_en;
    MT_TS_PID_FILT_EN_E ts_pid4_filt_en;
    MT_TS_PID_FILT_EN_E ts_pid5_filt_en;
    MT_TS_PID_FILT_EN_E ts_pid6_filt_en;
    MT_TS_PID_FILT_EN_E ts_pid7_filt_en;
    mt_u16	ts_pid0_filt_num;
    mt_u16	ts_pid1_filt_num;
    mt_u16	ts_pid2_filt_num;
    mt_u16	ts_pid3_filt_num;
    mt_u16	ts_pid4_filt_num;
    mt_u16	ts_pid5_filt_num;
    mt_u16	ts_pid6_filt_num;
    mt_u16	ts_pid7_filt_num;

    MT_TS_FORCE_ENC_EN_E	ts_force_enc_en;    /* force encrypt if enabled */

} MT_CIPHER_TS_PARA_S;


/*!
 * background check command delay time
 */
typedef enum {
    MT_CIPHER_BGC_DELAY_NONE = 0,
    MT_CIPHER_BGC_DELAY_QUARTER_SEC,
    MT_CIPHER_BGC_DELAY_HALF_SEC,
    MT_CIPHER_BGC_DELAY_ONE_SEC,
} MT_CIPHER_BGC_DELAY;

/*!
 * background check slot lock
 */
typedef enum {
    MT_CIPHER_BGC_UNLOCK = 0,
    MT_CIPHER_BGC_LOCK,
} MT_CIPHER_BGC_LOCK_S;

/*!
 * request background check hw semephare
 */
typedef enum {
    MT_CIPHER_BGC_SEM_REQ_ONE_TIME = 0,
    MT_CIPHER_BGC_SEM_REQ_WAIT_FOREVER,
} MT_CIPHER_BGC_SEM_REQ_S;

/*!
 * KDF profile list
 */
typedef enum {
    MT_CIPHER_KL_SCTE_201_2013_P0 = 0x0,
    MT_CIPHER_KL_SCTE_201_2013_P1 = 0x1,
    MT_CIPHER_KL_SCTE_201_2013_P1A = 0x2,
    MT_CIPHER_KL_SCTE_201_2013_P2 = 0x3,
    MT_CIPHER_KL_SCTE_201_2013_P2A = 0x4,
    MT_CIPHER_KL_SCTE_201_2013_P2B = 0x5,
    MT_CIPHER_KL_ETSI_TS_103_162 = 0x6,
    MT_CIPHER_KL_GY_T_255_2012 = 0x7,
    MT_CIPHER_KL_LAST_PROFILE = 0x8,
} MT_CIPHER_STANDARD_PROFILE_S;

/*!
 * connect command hardwired key list
 */
typedef enum {
    MT_CIPHER_HARDWIRED_KEY0 = 0x0,
    MT_CIPHER_HARDWIRED_KEY1 = 0x1,
    MT_CIPHER_HARDWIRED_KEY2 = 0x2,
    MT_CIPHER_HARDWIRED_KEY3 = 0x3,
    MT_CIPHER_HARDWIRED_KEY4 = 0x4,
    MT_CIPHER_HARDWIRED_KEY5 = 0x5,
    MT_CIPHER_HARDWIRED_KEY6 = 0x6,
    MT_CIPHER_HARDWIRED_KEY7 = 0x7,
} MT_CIPHER_HARDWIRED_SOURCE_E;

/*!
 * additional conditions of keyladder
 */
typedef enum {
    MT_CIPHER_KL_ADDT_EXPORTSWAP = 0x0,
    MT_CIPHER_KL_ADDT_MAXNUM
} MT_CIPHER_KL_ADDITIONS_E;

/*!
 * status of keytable slot
 */
typedef enum _MT_CIPHER_KT_STATE_E
{
	MT_CIPHER_KT_INITED,                 /* the kt slot has been initialized */
	MT_CIPHER_KT_UNINIT,                 /* the kt slot has not been initialized */
	MT_CIPHER_KT_INVALID,               /* the kt slot has not been requested */
	MT_CIPHER_KT_READ_DIS,            /* OTP_KTAttrReadDis has been set */
}MT_CIPHER_KT_STATE_E;


/*!
 * \brief: malloc for continuity memory 
 *
 * \param[in] length: Length in bytes.
 *
 * \return Allocated buffer pointer, or NULL if failed.
 */
unsigned char *mt_unf_cipher_malloc(unsigned int length);

/*!
 * free
 * [in] p: pointer of the buffer
 *
 * return: MT_SUCCESS, else fail
 */
int mt_unf_cipher_free(unsigned char *p);

/*!
  Init the cipher device
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_init(void);

/*!
  Deinit the cipher device
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_deinit(void);

/*!
  Request a key slot
  
  \param[out] (p_keyslot) created key slot ID
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_request(unsigned int *p_keyslot);

/*!
  Request a group of keyslots
  Starting with a multiple of 'num', and continuous slots

  \param[in] (num) request keylots num
  \param[out] (p_keyslot) array of keyslots ID

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_request_multi(unsigned int num, unsigned int *p_keyslot);

/*!
  Set clear key to key slot.
  if IV only, set p_ctrl & p_key null.

  \param[in] (keyslot) the key slot to set
  \param[in] (p_ctrl) the key slot information
  \param[in] (p_key) the key value
  \param[in] (p_iv) the initial value
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_set(unsigned int keyslot, MT_CIPHER_CTRL_S *p_ctrl, unsigned char *p_key, unsigned char *p_iv);

/*!
  Set clear key to key slot.
  if IV only, set p_ctrl & p_key null.

  \param[in] (keyslot) the key slot to set
  \param[in] (p_ctrl) the key slot control information
  \param[in] (p_key) the key value
  \param[in] (p_iv) the initial value
  
  \return SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_set_ext(unsigned int keyslot, MT_KT_CTRL_S *p_ctrl, unsigned char *p_key, unsigned char *p_iv);

/*!
  Release key slot
  
  \param[in] (keyslot) the key slot ID to release
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_release(unsigned int keyslot);

/*!
  Set iv to key slot.

  \param[in] (keyslot) the key slot to set
  \param[in] (p_iv) the initial value
  \param[in] (ivsize) the iv size

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_set_iv(unsigned int keyslot, unsigned char *p_iv, unsigned int ivsize);

/*!
  Get iv of key slot.

  \param[in] (keyslot) the key slot to set
  \param[out] (p_iv) the initial value
  \param[in] (ivsize) the iv size

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_get_iv(unsigned int keyslot, unsigned char *p_iv, unsigned int ivsize);

/*!
  Get metadata of key slot.

  \param[in] (keyslot) the key slot to set
  \param[out] (p_metadata) metadata

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_get_metadata(unsigned int keyslot, unsigned int *p_metadata);

/*!
  Get state of key slot.

  \param[in] (keyslot) the key slot to set
  \param[out] (p_state) the kt slot state, refer to MT_CIPHER_KT_STATE_E

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_get_state(unsigned int keyslot, MT_CIPHER_KT_STATE_E *p_state);

/*!
  Dump key slot info

  \param[in] (keyslot) the key slot ID to dump

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_info(unsigned int keyslot);

/*!
  Dump key slot control status

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyslot_control_status(void);

/*!
  Obtain a keyladder handle
  
  \param[in] (index) keyladder type select, do not care it in symphony4
  \param[out] (p_keyladder) created key ladder handle
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_create(MT_CIPHER_KEYLADDER_TYPE_E index, mt_handle *p_keyladder);

/*!
  Destroy the existing keyladder handle
  
  \param[in] (keyladder) the keyladder handle will be destroied
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_destroy(mt_handle keyladder);

/*!
  Set signature of the Keyladder you create(CA vendor specific!).
  
  \param[in] (keyladder) the keyladder handle
  \param[in] (p_sig) pointer of signaure.
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_set_signature(mt_handle keyladder, unsigned char *p_sig);

/*!
  Link new level to keyladder
  
  \param[in] (keyladder) key ladder handle
  \param[in] (p_ctrl) cipher attribute
  \param[in] (input) input buffer
  \param[in] (length) input length
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_link(mt_handle keyladder, MT_CIPHER_CTRL_S *p_ctrl, unsigned char *input, unsigned int length);

/*!
  Start keyladder with a rootkey source
  
  \param[in] (keyladder) key ladder handle
  \param[in] (keyladder_source) keyladder rootkey source
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_start(mt_handle keyladder, MT_CIPHER_KEYLADDER_SOURCE_E keyladder_source);

/*!
  If keyladder operation succeed, one key will be exported to the key_slot.
  
  \param[in] (keyladder) key ladder handle
  \param[in] (key_slot) key slot ID in keytable
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_end(mt_handle keyladder, unsigned int key_slot);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
/*!
  If keyladder operation succeed, the clear tdc data will transfer to TDC SRAM, use for Irdeto Invt.
  
  \param[in] (keyladder) key ladder handle
  \param[in] (tdc_data) cipher TDC data
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_extr_tdc(mt_handle keyladder, unsigned char *tdc_data);

/*!
  If keyladder operation succeed, the input will be transpose by Invt.
  
  \param[in] (keyladder) key ladder handle
  \param[in] (inupt) the input data
  \param[in] (enc_ctrl) cipher attribute
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_run_tdc(mt_handle keyladder, unsigned char *inupt, MT_CIPHER_CTRL_S *enc_ctrl);

/*!
  If keyladder operation succeed, the key will be stored to the destination.
  
  \param[in] (keyladder) key ladder handle
  \param[in] (store_dst) key destination
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_store(mt_handle keyladder, MT_CIPHER_KEYLADDER_STORE_E store_dst);

/*!
  If keyladder operation succeed, the key will be stored to the destination.
  
  \param[in] (keyladder) key ladder handle
  \param[in] (vendor_id) vendor id for Key Derivation Function
  \param[in] (kdf_profile) KDF profile
  \param[in] (hw_key) hardwired key selection
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_kdf_seedv(mt_handle keyladder, unsigned char *vendor_id, MT_CIPHER_STANDARD_PROFILE_S kdf_profile, MT_CIPHER_HARDWIRED_SOURCE_E hw_key);

/*!
  Install or uninstall additional conditions to keyladder before trigger.
  
  \param[in] (keyladder) key ladder handle
  \param[in] (additions) additional condition
  \param[in] (en) disable or enable the additional condition
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_keyladder_extra_infor(mt_handle keyladder, MT_CIPHER_KL_ADDITIONS_E additions, MT_CIPHER_ENABLE_E en);
#endif

/*!
  Create hash operation handle
  
  \param[in] (p_attr) hash type
  \param[in] (p_hmac_attr) HMAC parameters, set NULL for pure hash
  \param[out] (p_hash) created cipher hash handle
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, mt_handle *p_hash);

/*!
  Calculate the hash
  
  \param[in] (hash) hash handle
  \param[in] (p_data) input data 
  \param[in] (length) length of data
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_hash_update(mt_handle hash, unsigned char *p_data, unsigned int length);

/*!
  Get the final hash value
  
  \param[in] (hash) hash handle
  \param[out] (p_output_hash) final output hash value
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_hash_final(mt_handle hash, unsigned char *p_output_hash);

/*!
  Create mac operation handle
  
  \param[in] (mac_type) mac type
  \param[in] (p_hmac_attr) MAC parameters
  \param[out] (p_mac) created cipher mac handle
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_mac_create(MT_CIPHER_MAC_TYPE_E mac_type, MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, mt_handle *p_mac);

/*!
  Calculate the mac
  
  \param[in] (mac) hash handle
  \param[in] (p_data) input data 
  \param[in] (length) length of data
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_mac_update(mt_handle mac, unsigned char *p_data, unsigned int length);

/*!
  Get the final mac value
  
  \param[in] (mac) mac handle
  \param[out] (p_output_mac) final output mac value
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_mac_final(mt_handle mac, unsigned char *p_output_mac);

/*!
  Obtain a crypto handle for encryption or decryption

  \param[in] (index) crypto engine channel index
  \param[out] (p_crypto) created crypto handle
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_E index, mt_handle *p_crypto);

/*!
  Destroy the existing crypto handle
  
  \param[in] (crypto) the crypto handle will be destroied
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_crypto_destroy(mt_handle crypto);

/*!
  Configures the crypto control information
  
  \param[in] (cipher) crypto handle
  \param[in] (p_ctrl) crypto attributes
  \param[in] (key_slot) key slot id
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_crypto_config(mt_handle crypto, MT_CIPHER_CTRL_S *p_ctrl, unsigned int key_slot);

/*!
  Performs encryption or decryption 
  
  \param[in] (crypto) cipher handle
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (length) length of data
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_crypto_process(mt_handle crypto, unsigned char *p_src_addr, unsigned char *p_dest_addr, unsigned int length);

/*!
  Performs encryption or decryption

  \param[in] (crypto) cipher handle
  \param[in] (p_src_phyaddr) source data physical address
  \param[in] (p_dest_phyaddr) target data physical address
  \param[in] (length) length of data

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_crypto_process_phy(mt_handle crypto, phys_addr_t p_src_phyaddr, phys_addr_t p_dest_phyaddr, unsigned int length);

/*!
  Performs encryption or decryption extension function

  \param[in] (crypto) cipher handle
  \param[in] (p_src_phyaddr) source data physical address
  \param[in] (p_dest_phyaddr) target data physical address
  \param[in] (clear_length) clear length of data
  \param[in] (cipher_length) cipher length of data

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_crypto_process_phy_ext(mt_handle crypto, phys_addr_t p_src_phyaddr, phys_addr_t p_dest_phyaddr, unsigned int clear_length, unsigned int cipher_length);

/*!
  Performs encryption or decryption

  \param[in] (crypto) cipher handle
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (offset) the offset in source where the operation performing starts
  \param[in] (length) length of data

  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_crypto_process_ext(mt_handle crypto, unsigned char *p_src_addr, unsigned char *p_dest_addr, unsigned int offset, unsigned int length);

/*!
  Request encryption or decryption asynchronously

  \param[in] (crypto) cipher handle
  \param[in] (src) source data address
  \param[in] (dst) target data address
  \param[in] (count) count of the pair of (clear_length, protected_length)
  \param[in] (clear_length) length of clear data
  \param[in] (protected_length) length of protected data

  \return SUCCESS, else fail
  */
mt_s32 mt_unf_cipher_crypto_async_request(mt_handle crypto, const mt_u8 *src, mt_u8 *dst, mt_u32 count, mt_u32 clear_length, mt_u32 protected_length);

/*!
  Start encryption or decryption asynchronously

  \param[in] (crypto) cipher handle

  \return SUCCESS, else fail
  */
mt_s32 mt_unf_cipher_crypto_async_start(mt_handle crypto);

/*!
  Wait encryption or decryption for completion

  \param[in] (crypto) cipher handle

  \return SUCCESS, else fail
  */
mt_s32 mt_unf_cipher_crypto_async_wait(mt_handle crypto);

/*!
  Obtain a crypto TS handle for scrambling or descrambling

  \param[in] (index) ts engine channel index
  \param[out] (p_ts_handle) created ts handle
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_ts_create(MT_CIPHER_CRYPTO_CH_E index, mt_handle *p_ts_handle);

/*!
  Destroy the existing ts handle
  
  \param[in] (ts_handle) the ts handle will be destroyed
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_ts_destroy(mt_handle ts_handle);

/*!
  Configures the crypto control information
  
  \param[in] (ts_handle) ts handle
  \param[in] (p_ctrl) crypto attributes
  \param[in] (p_ts_para) TS patameters
  \param[in] (even_key_slot) even key slot id
  \param[in] (odd_key_slot) odd key slot id
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_ts_config(mt_handle ts_handle, MT_CIPHER_CTRL_S *p_ctrl, MT_CIPHER_TS_PARA_S *p_ts_para, 
        unsigned int even_key_slot, unsigned int odd_key_slot);

/*!
  Performs encryption or decryption 
  
  \param[in] (ts_handle) ts handle
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (length) length of data
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_ts_process(mt_handle ts_handle, phys_addr_t p_src_addr, phys_addr_t p_dest_addr, unsigned int length);

/*!
  Obtain a cipher handle for RSA encryption decryption

  \param[in] (p_rsa_handle) RSA handle to get
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_rsa_create(mt_handle *p_rsa_handle);

/*!
  Destroy the existing rsa handle
  
  \param[in] (rsa_handle) the cipher handle
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_rsa_destroy(mt_handle rsa_handle);

/*!
  Configures the rsa control information
  
  \param[in] (rsa_handle) rsa handle
  \param[in] (p_ctrl) cipher attributes
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_rsa_config(mt_handle rsa_handle, MT_CIPHER_RSA_CTRL_S *p_ctrl);

/*!
  Performs rsa encryption or decryption 
  
  \param[in] (cipher) cipher handle
  \param[in] (p_ctrl) rsa parameters
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (length) length of data
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_rsa_process(mt_handle rsa_handle, unsigned char *p_src_addr, unsigned char *p_dst_addr, unsigned int length);


/***********************************************************************************
 * MISC Functions
 **********************************************************************************/
int mt_unf_cipher_get_random_number(unsigned int bytes_to_get, unsigned char *p_random_number);

/*!
  Obtain a bn handle

  \param[out] (p_bn_handle) create bn handle
  
  \return MT_SUCCESS, else fail
*/
int mt_unf_cipher_bn_create(mt_handle *p_bn_handle);

/*!
  Destroy the existing cipher handle
  
  \param[in] (handle) the bn handle
  
  \return MT_SUCCESS, else fail
  */
int mt_unf_cipher_bn_destroy(mt_handle handle);

/*!
 * \brief modular
 *
 * \param[in] handle:
 * \param[out] r:   remainder 
 * \param[in]  d:   in data  
 * \param[in]  m:   modulo
 * \param[in]  d_length:    data length
 * \param[in]  m_length:    modulo length
 *
 * \return MT_SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_mod(mt_handle handle, unsigned char *r, unsigned char *d, unsigned char *m, unsigned int d_length, unsigned int m_length);

/*!
 * \brief modular multiply
 *
 * \param[in] handle:
 * \param[out] r:   remainder 
 * \param[in]  a:   input a
 * \param[in]  b:   input b
 * \param[in]  m:   modulo
 * \param[in]  a_length:    data length
 * \param[in]  b_length:    data length
 * \param[in]  m_length:    modulo length
 *
 * \return MT_SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_mul(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *b, unsigned char *m, unsigned int a_length, unsigned int b_length, unsigned int m_length);

/*!
 * \brief modular addition
 *
 * \param[in] handle:
 * \param[out] r:   remainder 
 * \param[in]  a:   input a
 * \param[in]  b:   input b
 * \param[in]  m:   modulo
 * \param[in]  a_length:    data length
 * \param[in]  b_length:    data length
 * \param[in]  m_length:    modulo length
 *
 * \return MT_SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_add(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *b, unsigned char *m, unsigned int a_length, unsigned int b_length, unsigned int m_length);

/*!
 * \brief modular subtraction
 *
 * \param[in] handle:
 * \param[out] r:   remainder 
 * \param[in]  a:   input a
 * \param[in]  b:   input b
 * \param[in]  m:   modulo
 * \param[in]  a_length:    data length
 * \param[in]  b_length:    data length
 * \param[in]  m_length:    modulo length
 *
 * \return MT_SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_sub(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *b, unsigned char *m, unsigned int a_length, unsigned int b_length, unsigned int m_length);

/*!
 * \brief modular inverse
 *
 * \param[in] handle:
 * \param[out] r:   remainder 
 * \param[in]  d:   input data
 * \param[in]  m:   modulo
 * \param[in]  d_length:    data length
 * \param[in]  m_length:    modulo length
 *
 * \return MT_SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_inv(mt_handle handle, unsigned char *r, unsigned char *d, unsigned char *m, unsigned int d_length, unsigned int m_length);

/*!
 * brief modular exponentiation
 *
 * \param[in] handle:
 * \param[out] r:   remainder 
 * \param[in]  a:   input data
 * \param[in]  p:   input data
 * \param[in]  m:   modulo
 * \param[in]  a_length:    data length
 * \param[in]  p_length:    data length
 * \param[in]  m_length:    modulo length
 *
 * \return MT_SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_exp(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *p, unsigned char *m, unsigned int a_length, unsigned int p_length, unsigned int m_length);

/*********************************************************************
 * ==========================EC POINT================================*
 ********************************************************************/
typedef struct _MT_CIPHER_EC_POINT_S
{
	unsigned char *X;
	unsigned char *Y;
} MT_CIPHER_EC_POINT_S;

typedef struct _MT_CIPHER_EC_PARAMS_S
{
  unsigned char* q;
  /**<  Finite field: equal to p in case of prime field curves or equal
   *    to 2^n in case of binary field curves.
  */
  unsigned char* a;
  /**<  Curve parameter a (q-3 in Suite B)
  */
  unsigned char* b;
  /**<  Curve parameter b
  */
  unsigned char* GX;
  /**<  X coordinates of G which is a base point on the curve
  */
  unsigned char* GY;
  /**<  Y coordinates of G which is a base point on the curve
  */
  unsigned char* n;
  /**<  Prime which is the order of G point
  */
  unsigned char* h;
  /**<  Cofactor, which is the order of the elliptic curve divided by the order
   *    of the point G. For the Suite B curves, h = 1.
  */
  unsigned int  keySize;
  /**<  Key size in bytes. It corresponds to the size in bytes of the prime n
   *    and is equal to:
   *    - P-256: 32 bytes
  */
} MT_CIPHER_EC_PARAMS_S;

/**
 * @brief Create EC Point handle
 *
 * @param[out] p_handle
 *
 * @return  MT_SUCCESS or MT_FAILURE
 */
int mt_unf_cipher_ecp_create(mt_handle *p_handle);


/*!
 * \brief: Destroy the EC Point handler. 
 *
 * \param[] handle
 *
 * \return MT_SUCCESS or MT_FAILURE
 */
int mt_unf_cipher_ecp_destroy(mt_handle handle);

/*!
 * \brief: ECP multiply. 
 *
 * \param[] handle
 * \param[] xParams
 * \param[] r
 * \param[] g_scalar
 * \param[] p_point
 * \param[] p_scalar
 *
 * \return  MT_SUCCESS or MT_FAILURE
 */
int mt_unf_cipher_ecp_mul(
        mt_handle handle, 
        MT_CIPHER_EC_PARAMS_S xParams, 
        MT_CIPHER_EC_POINT_S *r, 
        const unsigned char *g_scalar,  
        MT_CIPHER_EC_POINT_S *p_point, 
        const unsigned char *p_scalar);

/*!
 * \brief: ECP add.
 *
 * \param[] handle
 * \param[] xParams
 * \param[] r
 * \param[] a
 * \param[] b
 *
 * \return MT_SUCCESS or MT_FAILURE
 */
int mt_unf_cipher_ecp_add(
        mt_handle handle, 
        MT_CIPHER_EC_PARAMS_S xParams, 
        MT_CIPHER_EC_POINT_S *r, 
        const MT_CIPHER_EC_POINT_S *a, 
        const MT_CIPHER_EC_POINT_S *b);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
/*!
  request background check semaphore and a valid slot.
  
  \param[out] (bgc_slot) the available background check slot
  \param[in] (time_out) how to do, if requset semephare timeout
  \param[in] (bgc_addr) the start address of bgc. if it is 0 and bgc_size = 0, said to kernel text+rodata-section;
                    if it is used for the user process, there should be a physical address. 
  \param[in] (size) the length of bgc, if size = 0, will setup kernel's 'text+rodata'-section to bgc area
  \param[in] (delay) the delay time to execute next command
  \param[in] (golden_hash) if NULL, this api will automatic calculate it
  \param[in] (lock) lock the bgc slot, or not.
  
  \return SUCCESS, else fail
  */
mt_s32 mt_unf_cipher_bgc_request(mt_u32 *bgc_slot, \
            MT_CIPHER_BGC_SEM_REQ_S time_out, const mt_u8 *bgc_addr, mt_u32 size, \
            MT_CIPHER_BGC_DELAY delay, mt_u8 *golden_hash, MT_CIPHER_BGC_LOCK_S lock);

/*!
  release the background check semaphore.

  \param[in] (bgc_slot) background check slot
  
  \return SUCCESS, else fail
  */
mt_s32 mt_unf_cipher_bgc_release(mt_handle bgc_slot, MT_CIPHER_BGC_SEM_REQ_S time_out);
#endif

/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_CIPHER_H__ */
