/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_KT_IF_S6_H__
#define __HW_KT_IF_S6_H__

/* ==============================HW KEYTABLE================================ */
typedef enum _HW_KT_SLOT_VALID_E {
	HW_KT_SLOT_INVALIDATE = 0,
	HW_KT_SLOT_VALIDATE = 1,
} HW_KT_SLOT_VALID_E;

typedef enum _HW_KT_SLOT_ID_E {
	HW_KT_SLOT_ID_0,	/* NOUNCE */
	HW_KT_SLOT_ID_1,
	HW_KT_SLOT_ID_2,
	HW_KT_SLOT_ID_3,
	HW_KT_SLOT_ID_4,
	HW_KT_SLOT_ID_5,
	HW_KT_SLOT_ID_6,
	HW_KT_SLOT_ID_7,
	HW_KT_SLOT_ID_8,
	HW_KT_SLOT_ID_9,
	HW_KT_SLOT_ID_10,
	HW_KT_SLOT_ID_11,
	HW_KT_SLOT_ID_12,
	HW_KT_SLOT_ID_13,
	HW_KT_SLOT_ID_14,
	HW_KT_SLOT_ID_15,
	HW_KT_SLOT_ID_16,
	HW_KT_SLOT_ID_17,
	HW_KT_SLOT_ID_18,
	HW_KT_SLOT_ID_19,
	HW_KT_SLOT_ID_20,
	HW_KT_SLOT_ID_21,
	HW_KT_SLOT_ID_22,
	HW_KT_SLOT_ID_23,
	HW_KT_SLOT_ID_24,
	HW_KT_SLOT_ID_25,
	HW_KT_SLOT_ID_26,
	HW_KT_SLOT_ID_27,
	HW_KT_SLOT_ID_28,
	HW_KT_SLOT_ID_29,
	HW_KT_SLOT_ID_30,
	HW_KT_SLOT_ID_31,
	HW_KT_SLOT_ID_32,
	HW_KT_SLOT_ID_33,
	HW_KT_SLOT_ID_34,
	HW_KT_SLOT_ID_35,
	HW_KT_SLOT_ID_36,
	HW_KT_SLOT_ID_37,
	HW_KT_SLOT_ID_38,
	HW_KT_SLOT_ID_39,
	HW_KT_SLOT_ID_40,
	HW_KT_SLOT_ID_41,
	HW_KT_SLOT_ID_42,
	HW_KT_SLOT_ID_43,
	HW_KT_SLOT_ID_44,
	HW_KT_SLOT_ID_45,
	HW_KT_SLOT_ID_46,
	HW_KT_SLOT_ID_47,
	HW_KT_SLOT_ID_48,
	HW_KT_SLOT_ID_49,
	HW_KT_SLOT_ID_50,
	HW_KT_SLOT_ID_51,
	HW_KT_SLOT_ID_52,
	HW_KT_SLOT_ID_53,
	HW_KT_SLOT_ID_54,
	HW_KT_SLOT_ID_55,
	HW_KT_SLOT_ID_56,
	HW_KT_SLOT_ID_57,
	HW_KT_SLOT_ID_58,
	HW_KT_SLOT_ID_59,
	HW_KT_SLOT_ID_60,
	HW_KT_SLOT_ID_61,
	HW_KT_SLOT_ID_62,
	HW_KT_SLOT_ID_63,
	HW_KT_SLOT_ID_64,
	HW_KT_SLOT_ID_65,
	HW_KT_SLOT_ID_66,
	HW_KT_SLOT_ID_67,
	HW_KT_SLOT_ID_68,
	HW_KT_SLOT_ID_69,
	HW_KT_SLOT_ID_70,
	HW_KT_SLOT_ID_71,
	HW_KT_SLOT_ID_72,
	HW_KT_SLOT_ID_73,
	HW_KT_SLOT_ID_74,
	HW_KT_SLOT_ID_75,
	HW_KT_SLOT_ID_76,
	HW_KT_SLOT_ID_77,
	HW_KT_SLOT_ID_78,
	HW_KT_SLOT_ID_79,
	HW_KT_SLOT_ID_80,
	HW_KT_SLOT_ID_81,
	HW_KT_SLOT_ID_82,
	HW_KT_SLOT_ID_83,
	HW_KT_SLOT_ID_84,
	HW_KT_SLOT_ID_85,
	HW_KT_SLOT_ID_86,
	HW_KT_SLOT_ID_87,
	HW_KT_SLOT_ID_88,
	HW_KT_SLOT_ID_89,
	HW_KT_SLOT_ID_90,
	HW_KT_SLOT_ID_91,
	HW_KT_SLOT_ID_92,
	HW_KT_SLOT_ID_93,
	HW_KT_SLOT_ID_94,
	HW_KT_SLOT_ID_95,
	HW_KT_SLOT_ID_96,
	HW_KT_SLOT_ID_97,
	HW_KT_SLOT_ID_98,
	HW_KT_SLOT_ID_99,
	HW_KT_SLOT_ID_100,
	HW_KT_SLOT_ID_101,
	HW_KT_SLOT_ID_102,
	HW_KT_SLOT_ID_103,
	HW_KT_SLOT_ID_104,
	HW_KT_SLOT_ID_105,
	HW_KT_SLOT_ID_106,
	HW_KT_SLOT_ID_107,
	HW_KT_SLOT_ID_108,
	HW_KT_SLOT_ID_109,
	HW_KT_SLOT_ID_110,
	HW_KT_SLOT_ID_111,
	HW_KT_SLOT_ID_112,
	HW_KT_SLOT_ID_113,
	HW_KT_SLOT_ID_114,
	HW_KT_SLOT_ID_115,
	HW_KT_SLOT_ID_116,
	HW_KT_SLOT_ID_117,
	HW_KT_SLOT_ID_118,
	HW_KT_SLOT_ID_119,
	HW_KT_SLOT_ID_120,
	HW_KT_SLOT_ID_121,
	HW_KT_SLOT_ID_122,
	HW_KT_SLOT_ID_123,
	HW_KT_SLOT_ID_124,
	HW_KT_SLOT_ID_125,
	HW_KT_SLOT_ID_126,
	HW_KT_SLOT_ID_127,
	HW_KT_SLOT_MAX_NUM,
	HW_KT_SLOT_ID_INVALID = 0xFF,
} HW_KT_SLOT_ID_E;

typedef enum _HW_KT_SLOT_STATUS_E {
	HW_KT_SLOT_FREE = 0,
	HW_KT_SLOT_BUSY = 1,
} HW_KT_SLOT_STATUS_E;

typedef enum _HW_KT_SLOT_SIZE_E {
	HW_KT_SLOT_8B_SIZE = 8,
	HW_KT_SLOT_16B_SIZE = 16,
	HW_KT_SLOT_32B_SIZE = 32,
} HW_KT_SLOT_SIZE_E;

typedef enum _HW_KT_OPERATION_E {
	HW_KT_OPER_VALID = 0,
	HW_KT_OPER_W_KEY = 1,
	HW_KT_OPER_W_IV = 2,
	HW_KT_OPER_W_ATTR = 3,
	HW_KT_OPER_W_TEE = 4,
	HW_KT_OPER_W_KEY256 = 5,
	HW_KT_OPER_R_TEE = 6,
	HW_KT_OPER_R = 7,
} HW_KT_OPERATION_E;

typedef enum _HW_KT_KEY_SOURCE_E {
	HW_KT_KEY_SRC_ACPU = 0,
	HW_KT_KEY_SRC_SCPU = 1,
	HW_KT_KEY_SRC_VSCPU = 2,
	HW_KT_KEY_SRC_KLE_VSCPU = 3,
	HW_KT_KEY_SRC_KLE_ACPU = 4,
	HW_KT_KEY_SRC_KLE_SCPU = 5,
	HW_KT_KEY_SRC_AKL_ACPU = 6,
	HW_KT_KEY_SRC_AKL_SCPU = 7,
	HW_KT_KEY_SRC_KLM_ACPU = 8,
	HW_KT_KEY_SRC_KLM_SCPU = 9,
} HW_KT_KEY_SOURCE_E;

typedef union _HW_KT_KEY_ATTR_S {
	mt_u32 reg;
	struct {
		/* [13:0] key usage */
		mt_u32 AES_OFFON:1;
		mt_u32 DES_OFFON:1;
		mt_u32 TDES_OFFON:1;
		mt_u32 CSAv2_OFFON:1;
		mt_u32 CSAv3_OFFON:1;
		mt_u32 SM4_OFFON:1;
		mt_u32 CSAv2_CONFORMANCE_OFFON:1;
		mt_u32 GOST_28147_89_OR_R34_12_MAGMA_OFFON:1;
		mt_u32 ASA_OFFON:1;
		mt_u32 GOST_R34_12_KUZNYECHIK_OFFON:1;
		mt_u32 MULTI2_OFFON:1;
		mt_u32 resv1:3;

		/* [16:14] */
		mt_u32 TS_OFFON:1;
		mt_u32 M2M_OFFON:1;
		mt_u32 MAC_OFFON:1;

		/* [17] */
		mt_u32 M2M_REE_OFFON:1;

		/* [19:18] */
		mt_u32 DEC_ONOFF:1;
		mt_u32 ENC_ONOFF:1;

		/* [22:20] */
		mt_u32 KEY_SIZE64_OFFON:1;
		mt_u32 KEY_SIZE128_OFFON:1;
		mt_u32 KEY_SIZE256_OFFON:1;

		/* [27:23] */
		mt_u32 resv2:5;

		/* [31:28] */
		mt_u32 KEY_SOURCE:4;
	} bitc;
} HW_KT_KEY_ATTR_S;

typedef union _HW_KT_TEE_S {
	mt_u32 reg;
	struct {
		mt_u32 TEE_PRM:4;	/* 0: any known key source can write this entry
								others: key source from the secure domain(TEE)*/
		mt_u32 TEE_WCID:5;	/* the protected channel ID for write transaction */
		mt_u32 TEE_SCID:5;	/* the protected channel ID from input source */
		mt_u32 TEE_SC:1;		/* 0: RDCID shall be matched with SCID
								1: don't care the value of SCID*/
		mt_u32 TEE_TP:1;		/* 0: M2M can be used in REE; 1: M2M can be used in TEE */
		mt_u32 TEE_ENC_OFFON:1;	/* 0: allow encryption; 1: forbid encryption */
		mt_u32 TEE_DEC_OFFON:1;	/* 0: allow decryption; 1: forbid decryption */
		mt_u32 TEE_AUDIO:1;	/* 0: the WCID is not for audio PES/ES
								1: the WCID is  for audio PES/ES*/
		mt_u32:13;
	} bitc;
} HW_KT_TEE_S;


typedef enum _HW_KT_NONCE_STATUS_E {
	HW_KT_NONCE_INVALID_RANDOM_DATA = 0,
	HW_KT_NONCE_VALID_RANDOM_DATA = 1,
} HW_KT_NONCE_STATUS_E;

typedef enum _HW_KT_SLOT_TEE_VALID_E {
	HW_KT_SLOT_TEE_INVALIDATE = 0,
	HW_KT_SLOT_TEE_VALID = 1,
} HW_KT_SLOT_TEE_VALID_E;

typedef enum _HW_TEE_CFG_DATA_CHKEN_E {
	HW_TEE_CFG_DATA_CHK_DISABLE = 0,
	HW_TEE_CFG_DATA_CHK_ENABLE = 1,
} HW_TEE_CFG_DATA_CHKEN_E;

typedef enum _HW_TEE_CFG_DATA_CHK_LOCK_E {
	HW_TEE_CFG_DATA_CHK_UNLOCK = 0,
	HW_TEE_CFG_DATA_CHK_LOCK = 1,
} HW_TEE_CFG_DATA_CHK_LOCK_E;

typedef enum _HW_TEE_CFG_DATA_CHK_INVALID_E {
	HW_TEE_CFG_DATA_CHK_NOT_INVALID = 0,
	HW_TEE_CFG_DATA_CHK_INVALID = 1,
} HW_TEE_CFG_DATA_CHK_INVALID_E;

typedef struct _HW_TEE_CFG_DATA_CHK_S {
	mt_u32 invalid_keyslot:1;
	mt_u32 chk_field_1t31:31;
	mt_u32 chk_field_32t35:4;
	mt_u32 resv:28;
} HW_TEE_CFG_DATA_CHK_S;

typedef enum _HW_METADATA_ALGO_E {
	HW_METADATA_ALGO_AES = 0,
	HW_METADATA_ALGO_TDES = 1,
	HW_METADATA_ALGO_DES = 2,
	HW_METADATA_ALGO_ASA_LIGHT = 7,
	HW_METADATA_ALGO_ASA = 8,
	HW_METADATA_ALGO_CSA3 = 9,
	HW_METADATA_ALGO_CSA2 = 10,
	HW_METADATA_ALGO_MULTI2 = 11,
	HW_METADATA_ALGO_HMAC256 = 13,
} HW_METADATA_ALGO_E;

typedef enum _HW_METADATA_KEYSIZE_E {
	HW_METADATA_KEYSIZE_64 = 0,
	HW_METADATA_KEYSIZE_128 = 1,
	HW_METADATA_KEYSIZE_192 = 2,
	HW_METADATA_KEYSIZE_256 = 3,
} HW_METADATA_KEYSIZE_E;

typedef struct _HW_KT_METADATA_S {
	mt_u32 algo:4;
	mt_u32 keysize:2;
	mt_u32 enc_onoff:1;
	mt_u32 dec_onoff:1;
	mt_u32 resv:24;
} HW_KT_METADATA_S;

void hw_kt_write_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E valid);
void hw_kt_read_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E *p_valid);

void hw_kt_write_attribute(HW_KT_SLOT_ID_E slot_id, mt_u32 attr);
void hw_kt_read_attribute(HW_KT_SLOT_ID_E slot_id, mt_u32 *p_attr);

void hw_kt_write_key(
		HW_KT_SLOT_ID_E slot_id,
		const mt_u8 *p_key,
		HW_KT_SLOT_SIZE_E size);
void hw_kt_read_key(
		HW_KT_SLOT_ID_E slot_id,
		mt_u8 *p_key,
		HW_KT_SLOT_SIZE_E size);

void hw_kt_write_iv(
		HW_KT_SLOT_ID_E slot_id,
		const mt_u8 *p_iv,
		HW_KT_SLOT_SIZE_E size);
void hw_kt_read_iv(
		HW_KT_SLOT_ID_E slot_id,
		mt_u8 *p_iv,
		HW_KT_SLOT_SIZE_E size);

void hw_kt_write_key256(HW_KT_SLOT_ID_E slot_id, const mt_u8 *p_key);
void hw_kt_set_k256_endian(mt_u32 k256_endian);
void hw_kt_get_k256_info(mt_u32 *k256_err_sta, mt_u32 *k256_err_index);
void hw_kt_clear_k256_sta(void);

void hw_kt_nonce_clear(void);
void hw_kt_get_nonce_status(HW_KT_NONCE_STATUS_E *p_stat);

void hw_kt_write_tee(HW_KT_SLOT_ID_E slot_id, mt_u32 tee_cfg);
void hw_kt_read_tee(HW_KT_SLOT_ID_E slot_id, mt_u32 *p_tee);

void hw_kt_read_tee_valid(
		HW_KT_SLOT_ID_E slot_id,
		HW_KT_SLOT_TEE_VALID_E *p_valid);

void hw_kt_write_tee_cfgdata_chk_lock(HW_TEE_CFG_DATA_CHK_LOCK_E lock);
void hw_kt_read_tee_cfgdata_chk_lock(HW_TEE_CFG_DATA_CHK_LOCK_E * p_lock);
void hw_kt_write_tee_cfgdata_checken(HW_TEE_CFG_DATA_CHKEN_E enable);
void hw_kt_read_tee_cfgdata_checken(HW_TEE_CFG_DATA_CHKEN_E *enable);
void hw_kt_write_tee_cfgdata_chk(HW_TEE_CFG_DATA_CHK_S *p_check);
void hw_kt_read_tee_cfgdata_chk(HW_TEE_CFG_DATA_CHK_S *p_check);

void hw_kt_read_metadata(HW_KT_SLOT_ID_E slot_id, mt_u32 *p_metadata);

void hw_kt_read_sw_reg(mt_u32 *p_reg, mt_u32 index);
void hw_kt_write_sw_reg(mt_u32 data, mt_u32 index);

#endif /*__HW_KT_IF_S6_H__*/

