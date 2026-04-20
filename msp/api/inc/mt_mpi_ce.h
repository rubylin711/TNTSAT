/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_CRYPTO_ENGINE_H__
#define __MT_MPI_CRYPTO_ENGINE_H__
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//==========================CHANNEL================================
typedef enum  _MT_CE_CHANNEL_E
{
  MT_CE_CHANNEL_0,
  MT_CE_CHANNEL_1,
  MT_CE_CHANNEL_2,
  MT_CE_CHANNEL_3,
  MT_CE_CHANNEL_DEFAULT,
}MT_CE_CHANNEL_E;

//==========================ADES================================
typedef enum  _MT_CE_ADES_OPERATION_E
{
  MT_CE_ADES_OPERATION_ENCRYPT,
  MT_CE_ADES_OPERATION_DECRYPT,
}MT_CE_ADES_OPERATION_E;

typedef enum  _MT_CE_ADES_ALGO_MODE_E
{
  MT_CE_ADES_ALGO_MODE_AES128,
  MT_CE_ADES_ALGO_MODE_AES192,
  MT_CE_ADES_ALGO_MODE_AES256,
  MT_CE_ADES_ALGO_MODE_DES,
  MT_CE_ADES_ALGO_MODE_TDES_ABA,
  MT_CE_ADES_ALGO_MODE_SM4,
//  MT_CE_ADES_ALGO_MODE_TDES_ABC,
}MT_CE_ADES_ALGO_MODE_E;

typedef enum  _MT_CE_ADES_WORK_MODE_E
{  
  MT_CE_ADES_WORK_MODE_ECB,
  MT_CE_ADES_WORK_MODE_CBC,
  MT_CE_AES_WORK_MODE_CTR,				//用于AES 模式
  MT_CE_ADES_WORK_MODE_CBCDVS042,
  MT_CE_ADES_WORK_MODE_CBCCTS,
//  MT_CE_ADES_WORK_MODE_RCBCCTS,			//用于TS 模式
//  MT_CE_ADES_WORK_MODE_ECBCTS,			//用于TS 模式
  MT_CE_ADES_WORK_MODE_CFB1,
  MT_CE_ADES_WORK_MODE_CFB8,
  MT_CE_ADES_WORK_MODE_CFB,
  MT_CE_ADES_WORK_MODE_OFB1,
  MT_CE_ADES_WORK_MODE_OFB8,
  MT_CE_ADES_WORK_MODE_OFB,
  MT_CE_ADES_WORK_MODE_CBCS,
  MT_CE_ADES_WORK_MODE_CENS,
  MT_CE_ADES_WORK_MODE_UNKNOWN,
}MT_CE_ADES_WORK_MODE_E;

typedef struct _MT_CE_ADES_PARA_S
{
  MT_CE_CHANNEL_E chan_num;

  /*mt_handle kt_hdlr;*/
  /*mt_u8 *iv;*/
  /*mt_u8 *key;		//aes128 key size 16字节,aes192 key size 24字节,aes256 key size 32字节,des key size 8字节,tdes aba key size 16字节,tdes abc key size 24字节*/

  mt_u32 key_slot;
  
  MT_CE_ADES_ALGO_MODE_E algo_mode;
  MT_CE_ADES_WORK_MODE_E work_mode;

  mt_u32 skip_blocks;
  mt_u32 crypt_blocks;
} MT_CE_ADES_PARA_S;

typedef struct _MT_CE_ADES_CTRL_S
{
  MT_CE_ADES_OPERATION_E operation;
  MT_CE_ADES_PARA_S ades_para;
} MT_CE_ADES_CTRL_S;

typedef enum  _MT_CE_GRP_MODE_E
{
  MT_CE_GRP_MID,				// 00
  MT_CE_GRP_FIRST,			// 01
  MT_CE_GRP_LAST,				// 10
  MT_CE_GRP_NONE,		// 11
}MT_CE_GRP_MODE_E;

//==========================TS================================
typedef enum  _MT_CE_TS_OPERATION_E
{
  MT_CE_TS_OPERATION_SCRAMBLE,
  MT_CE_TS_OPERATION_DESCRAMBLE,
}MT_CE_TS_OPERATION_E;

typedef enum  _MT_CE_TS_ALGO_MODE_E
{
  MT_CE_TS_ALGO_MODE_AES128,
//  MT_CE_TS_ALGO_MODE_AES192,
//  MT_CE_TS_ALGO_MODE_AES256,
  MT_CE_TS_ALGO_MODE_DES,
  MT_CE_TS_ALGO_MODE_TDES_ABA,
//  MT_CE_TS_ALGO_MODE_TDES_ABC,
}MT_CE_TS_ALGO_MODE_E;

typedef enum  _MT_CE_TS_WORK_MODE_E
{
  MT_CE_TS_WORK_MODE_ADES_ECB,			//0000,
  MT_CE_TS_WORK_MODE_ADES_CBC,			//0001 ,
  MT_CE_TS_WORK_MODE_AES_CTR,			//0010,
  MT_CE_TS_WORK_MODE_ADES_CBCDVS042,	//0011 ,
  MT_CE_TS_WORK_MODE_ADES_CBCCTS,		//0100 ,
  MT_CE_TS_WORK_MODE_ADES_RCBCCTS,		//0101 ,
  MT_CE_TS_WORK_MODE_ADES_ECBCTS,		//0110 ,
}MT_CE_TS_WORK_MODE_E;

typedef enum  _MT_CE_TS_ENC_KSEL_E
{
  MT_CE_TS_ENC_ODD_KEY,				// 0,
  MT_CE_TS_ENC_EVEN_KEY,				// 1 ,
}MT_CE_TS_ENC_KSEL_E;

typedef enum  _MT_CE_TS_DEC_IND_E
{
  MT_CE_TS_DEC_IND_CLEAR,			// 0,
  MT_CE_TS_DEC_IND_KEEP,			// 1 ,
}MT_CE_TS_DEC_IND_E;

typedef enum  _MT_CE_TS_PKT_LEN_E
{
  MT_CE_TS_LEN_PKT_188BYTE,			// 0,
  MT_CE_TS_LEN_PKT_192BYTE,			// 1 ,
}MT_CE_TS_PKT_LEN_E;

typedef enum  _MT_CE_TS_IVE_MODE_E
{
  MT_CE_TS_IVE_MDI,				// 0,
  MT_CE_TS_IVE_MDD,				// 1 ,
  MT_CE_TS_IVE_MSC,				// 2 ,
  MT_CE_TS_IVE_OFF,
}MT_CE_TS_IVE_MODE_E;

typedef enum  _MT_CE_TS_SHORT_MODE_E
{
  MT_CE_TS_SHORT_HEAD,			// 0,
  MT_CE_TS_SHORT_TAIL,			// 1 ,
}MT_CE_TS_SHORT_MODE_E;

typedef enum  _MT_CE_TS_SMALL_MODE_E
{
  MT_CE_TS_SMALL_CLEAR,			// 0,
  MT_CE_TS_SMALL_DVS042_TAIL,		// 1 ,
  MT_CE_TS_SMALL_XOR_IVE,			// 2 ,
}MT_CE_TS_SMALL_MODE_E;

typedef enum  _MT_CE_TS_PID_FILT_EN_E
{
  MT_CE_TS_PID_FILT_DISABLE,			// 0,
  MT_CE_TS_PID_FILT_ENABLE,			// 1 ,
}MT_CE_TS_PID_FILT_EN_E;

typedef enum  _MT_CE_TS_FORCE_ENC_EN_E
{
  MT_CE_TS_FORCE_ENC_DISABLE,			// 0,
  MT_CE_TS_FORCE_ENC_ENABLE,			// 1 ,
}MT_CE_TS_FORCE_ENC_EN_E;

typedef struct _MT_CE_TS_PARA_S
{
  MT_CE_CHANNEL_E chan_num;

  /*mt_handle kt_hdlr;*/
  mt_u32 even_key_slot;
  mt_u32 odd_key_slot;

  MT_CE_TS_ALGO_MODE_E algo_mode;
  MT_CE_TS_WORK_MODE_E work_mode;

  MT_CE_TS_ENC_KSEL_E ts_enc_ksel;
  MT_CE_TS_DEC_IND_E ts_dec_ind;
  MT_CE_TS_PKT_LEN_E ts_pkt_len;
  MT_CE_TS_IVE_MODE_E ts_ive_mode;
  MT_CE_TS_SHORT_MODE_E ts_short_mode;
  MT_CE_TS_SMALL_MODE_E ts_small_mode;

  MT_CE_TS_PID_FILT_EN_E ts_pid0_filt_en;
  MT_CE_TS_PID_FILT_EN_E ts_pid1_filt_en;
  MT_CE_TS_PID_FILT_EN_E ts_pid2_filt_en;
  MT_CE_TS_PID_FILT_EN_E ts_pid3_filt_en;
  MT_CE_TS_PID_FILT_EN_E ts_pid4_filt_en;
  MT_CE_TS_PID_FILT_EN_E ts_pid5_filt_en;
  MT_CE_TS_PID_FILT_EN_E ts_pid6_filt_en;
  MT_CE_TS_PID_FILT_EN_E ts_pid7_filt_en;
  mt_u16					ts_pid0_filt_num;
  mt_u16					ts_pid1_filt_num;
  mt_u16					ts_pid2_filt_num;
  mt_u16					ts_pid3_filt_num;
  mt_u16					ts_pid4_filt_num;
  mt_u16					ts_pid5_filt_num;
  mt_u16					ts_pid6_filt_num;
  mt_u16					ts_pid7_filt_num;
  MT_CE_TS_FORCE_ENC_EN_E	ts_force_enc_en;
} MT_CE_TS_PARA_S;

typedef struct _MT_CE_TS_CTRL_S
{
  MT_CE_TS_OPERATION_E operation;
  MT_CE_TS_PARA_S ts_para;
} MT_CE_TS_CTRL_S;

//==========================SHA================================
typedef enum  _MT_CE_SHA_ALGO_MODE_E
{
  MT_CE_HASH_ALGO_MODE_SHA1,
  MT_CE_HASH_ALGO_MODE_SHA224,
  MT_CE_HASH_ALGO_MODE_SHA256,
  MT_CE_HASH_ALGO_MODE_SHA384,
  MT_CE_HASH_ALGO_MODE_SHA512,
  MT_CE_HASH_ALGO_MODE_SM3,
  MT_CE_HASH_ALGO_MODE_HMAC_SHA224,
  MT_CE_HASH_ALGO_MODE_HMAC_SHA256,
  MT_CE_HASH_ALGO_MODE_HMAC_SHA384,
  MT_CE_HASH_ALGO_MODE_HMAC_SHA512,
  MT_CE_HASH_ALGO_MODE_HMAC_SM3,
  MT_CE_HASH_ALGO_MODE_CMAC_AES128,
  MT_CE_HASH_ALGO_MODE_CMAC_SM4,
  MT_CE_HASH_ALGO_MODE_MAX,
}MT_CE_SHA_ALGO_MODE_E;

typedef struct _MT_CE_SHA_PARA_S
{
  MT_CE_CHANNEL_E chan_num;
  MT_CE_SHA_ALGO_MODE_E algo_mode;
  mt_u32 digest_size;
  mt_u32 key_size;
  mt_u32 key_slot;
  mt_u8 hmac_key[128];
} MT_CE_SHA_PARA_S;

typedef struct  _MT_CE_SHA_CTRL_S
{
  MT_CE_SHA_PARA_S sha_para;
}MT_CE_SHA_CTRL_S;

//==========================RSA================================
typedef struct _MT_CE_RSA_PARA_S
{
  mt_u8 p_m[256];			//public key
  mt_u8 p_e[256];			//exponent
  mt_u32 key_length;
  mt_u32 exp_length;
} MT_CE_RSA_PARA_S;

typedef struct _MT_CE_RSA_CTRL_S
{
  MT_CE_RSA_PARA_S rsa_para;
} MT_CE_RSA_CTRL_S;

//==========================EC POINT================================
typedef struct _MT_CE_EC_POINT_S
{
	mt_u8 *X;
	mt_u8 *Y;
} MT_CE_EC_POINT_S;

typedef struct _MT_CE_EC_PARAMS_S
{
  mt_u8* q;
  /**<  Finite field: equal to p in case of prime field curves or equal
   *    to 2^n in case of binary field curves.
  */
  mt_u8* a;
  /**<  Curve parameter a (q-3 in Suite B)
  */
  mt_u8* b;
  /**<  Curve parameter b
  */
  mt_u8* GX;
  /**<  X coordinates of G which is a base point on the curve
  */
  mt_u8* GY;
  /**<  Y coordinates of G which is a base point on the curve
  */
  mt_u8* n;
  /**<  Prime which is the order of G point
  */
  mt_u8* h;
  /**<  Cofactor, which is the order of the elliptic curve divided by the order
   *    of the point G. For the Suite B curves, h = 1.
  */
  mt_u32         keySize;
  /**<  Key size in bytes. It corresponds to the size in bytes of the prime n
   *    and is equal to:
   *    - P-256: 32 bytes
  */
} MT_CE_EC_PARAMS_S;

//==========================BGC================================
typedef enum {
    MT_CE_BGC_DELAY_NONE = 0,
    MT_CE_BGC_DELAY_QUARTER_SEC,
    MT_CE_BGC_DELAY_HALF_SEC,
    MT_CE_BGC_DELAY_ONE_SEC,
} MT_CE_BGC_DELAY;

typedef enum {
    MT_CE_BGC_UNLOCK = 0,
    MT_CE_BGC_LOCK,
} MT_CE_BGC_LOCK_S;

typedef enum {
    MT_CE_BGC_SEM_REQ_ONE_TIME = 0,
    MT_CE_BGC_SEM_REQ_WAIT_FOREVER,
} MT_CE_BGC_SEM_REQ_S;

#ifdef __cplusplus
}
#endif

#endif //__MT_UNF_CRYPTO_ENGINE_H__


#ifndef __CE_RESULT_H__
#define __CE_RESULT_H__
/*!
  Result Code
  */
#define CE_SUCCESS	0

#define CE_GET_HANDLE_FAILED			-1
#define CE_INVALID_HANDLE				-2
#define CE_INVALID_CHANNEL				-3
#define CE_INIT_FAILED					-4
#define CE_BAD_PARAMETERS               -5
#define CE_NOT_SUPPORT                  -6

#define CE_TS_ALGO_MODE_ERROR				-120
#define CE_TS_WORK_MODE_ERROR				-121
#define CE_TS_WORK_ALGO_MODE_CONFLICT		-122
#define CE_TS_NOT_SUPPORT_RCBCCTS_ENC		-124
#define CE_TS_NOT_SUPPORT_ECBCTS_ENC		-125
#define CE_TS_ECB_MODE_NOT_NEED_IVE			-126
#define CE_TS_OTHER_ERROR				-127

#define CE_ADES_ALGO_MODE_ERROR				-130
#define CE_ADES_WORK_MODE_ERROR				-131
#define CE_ADES_WORK_ALGO_MODE_CONFLICT	-132
#define CE_ADES_CPU_WRITE_TIMEOUT 			-134
#define CE_ADES_CPU_READ_TIMEOUT				-135
#define CE_ADES_CPU_ADDR_ERROR				-136
#define CE_ADES_CPU_LENGTH_MISMATCH			-137
#define CE_ADES_CPU_WRITE_LENGTH_ERROR		-138
#define CE_ADES_CPU_READ_LENGTH_ERROR		-139
#define CE_ADES_DMA_READ_TIMEOUT				-140
#define CE_ADES_DMA_ADDR_ERROR				-141
#define CE_ADES_GRP_LENGTH_ERROR				-142
#define CE_ADES_GRP_STATE_ERROR				-143
#define CE_ADES_GRP_MODE_ERROR				-144
#define CE_ADES_OTHER_ERROR				-145

#define CE_SHA_ALGO_MODE_ERROR				-240
//#define CE_SHA_HMAC_KEY_SIZE_ERROR			-241
//#define CE_SHA_HMAC_KEY_NULL_ERROR			-242
#define CE_SHA_MSG_EMPTYLOAD_ERROR			-243
#define CE_SHA_MSG_OVERLOAD_ERROR			-244
#define CE_SHA_MSG_GRPLENGTH_ERROR			-245
#define CE_SHA_INIT_FAILED						-246
#define CE_SHA_CPU_WRITE_TIMEOUT 			-247
#define CE_SHA_CALC_TIMEOUT					-248
#define CE_SHA_CPU_ADDR_ERROR				-249
#define CE_SHA_DMA_ADDR_ERROR				-250
#define CE_SHA_POLLING_IN_INVALID_GRP			-251
#define CE_SHA_OTHER_ERROR				-252

#define CE_RSA_CONFIG_FAILED        -300
#define CE_RSA_ALGO_MODE_ERROR		-301
#define CE_RSA_CALC_TIMEOUT			-302
#define CE_RSA_LENGTH_MISMATCH		-303
#define CE_RSA_PROCESS_FAILED       -304

#define CE_BN_MOD_CALC1_TIMEOUT                         -400
#define CE_BN_MOD_CALC2_TIMEOUT                         -401
#define CE_EC_POINT_CALC_TIMEOUT                        -402
#define CE_EC_POINT_CALC_INFINITY	       	        -403
#define CE_BN_MOD_CALC2_LENGTH1_ERROR           -404
#define CE_BN_MOD_CALC2_LENGTH2_ERROR           -405
#define CE_BN_MOD_CALC2_LENGTH3_ERROR           -406
#define CE_BN_MOD_CALC2_LENGTH4_ERROR           -407
#define CE_BN_MOD_LENGTH_ERROR                          -411
#define CE_BN_MOD_MUL_INPUT_ERROR                   -421
#define CE_BN_MOD_ADD_INPUT_ERROR                   -422
#define CE_BN_MOD_SUB_INPUT_ERROR                   -423
#define CE_BN_MOD_INV_INPUT_ERROR                    -424
#define CE_EC_POINT_MUL_INPUT_ERROR                 -425
#define CE_EC_POINT_MUL_160K1G_ERROR                -426
#define CE_EC_POINT_MUL_160K2G_ERROR                -427
#define CE_EC_POINT_MUL_160K1K2G_ERROR             -428

#define CE_ECDSA_VERIFY_OK					    0
#define CE_ECDSA_SIGN_GEN_RANDOM_FAILED	    -900
#define CE_ECDSA_SIGN_DGST2E_ERROR		    -901
#define CE_ECDSA_VERIFY_DGST2E_ERROR		    -902
#define CE_ECDSA_VERIFY_CHECK_R_FAILED	    -903
#define CE_ECDSA_VERIFY_CHECK_S_FAILED	    -904
#define CE_ECDSA_VERIFY_FAILED				    -905
#define CE_EC_POINT_BAD_PARAMS			    -906

#endif	/*__CE_RESULT_H__*/

