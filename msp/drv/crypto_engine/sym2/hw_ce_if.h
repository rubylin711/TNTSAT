/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_CE_IF_H__
#define __HW_CE_IF_H__

#include "drv_ce_ioctl.h"

//==============================HW CHANNEL================================
typedef enum  _HW_CE_CHANNEL_E
{
  HW_CE_CHANNEL_0,
  HW_CE_CHANNEL_1,
  HW_CE_CHANNEL_2,
  HW_CE_CHANNEL_3,
}HW_CE_CHANNEL_E;

//==============================HW TS================================
typedef enum  _HW_CE_TS_ALGO_MODE_E
{
  HW_CE_TS_ALGO_ENC,					// 0,
  HW_CE_TS_ALGO_DEC,					// 1 ,
}HW_CE_TS_ALGO_MODE_E;

typedef enum  _HW_CE_TS_ENC_KSEL_E
{
  HW_CE_TS_ENC_ODD_KEY,		// 0,
  HW_CE_TS_ENC_EVEN_KEY,		// 1 ,
}HW_CE_TS_ENC_KSEL_E;

typedef enum  _HW_CE_TS_DEC_IND_E
{
  HW_CE_TS_DEC_IND_CLEAR,			// 0,
  HW_CE_TS_DEC_IND_KEEP,			// 1 ,
}HW_CE_TS_DEC_IND_E;

typedef enum  _HW_CE_TS_PKT_LEN_E
{
  HW_CE_TS_PKT_LEN_188BYTE,	// 0,
  HW_CE_TS_PKT_LEN_192BYTE,	// 1 ,
}HW_CE_TS_PKT_LEN_E;

typedef enum  _HW_CE_TS_SWITCH_E
{
  HW_CE_TS_SWITCH_OFF,			// 0,
  HW_CE_TS_SWITCH_ON,			// 1 ,
}HW_CE_TS_SWITCH_E;

typedef enum  _HW_CE_TS_IVE_CAL_EN_E
{
  HW_CE_TS_IVE_CAL_DISABLE,		// 0,
  HW_CE_TS_IVE_CAL_ENABLE,		// 1 ,
}HW_CE_TS_IVE_CAL_EN_E;

typedef enum  _HW_CE_TS_IVE_MODE_E
{
  HW_CE_TS_IVE_MDI,				// 0,
  HW_CE_TS_IVE_MDD,				// 1 ,
  HW_CE_TS_IVE_MSC,				// 2 ,
}HW_CE_TS_IVE_MODE_E;

typedef enum  _HW_CE_TS_CTS_MODE_E
{
  HW_CE_TS_CBCCTS_TSPARSE,		// 0,
  HW_CE_TS_CBCCTS_NORMAL,		// 1 ,
}HW_CE_TS_CTS_MODE_E;

typedef enum  _HW_CE_TS_SHORT_MODE_E
{
  HW_CE_TS_SHORT_HEAD,			// 0,
  HW_CE_TS_SHORT_TAIL,			// 1 ,
}HW_CE_TS_SHORT_MODE_E;

typedef enum  _HW_CE_TS_SMALL_MODE_E
{
  HW_CE_TS_SMALL_CLEAR,			// 0,
  HW_CE_TS_SMALL_DVS042_TAIL,	// 1 ,
  HW_CE_TS_SMALL_XOR_IVE,		// 2 ,
}HW_CE_TS_SMALL_MODE_E;

typedef enum  _HW_CE_GRP_MODE_E
{
  HW_CE_GRP_MID,				// 00
  HW_CE_GRP_FIRST,			// 01
  HW_CE_GRP_LAST,				// 10
  HW_CE_GRP_FIRST_LAST,		// 11
}HW_CE_GRP_MODE_E;

typedef enum  _HW_CE_PROC_MODE_E
{
  HW_CE_PROC_AUTO,
  HW_CE_PROC_CPU,
  HW_CE_PROC_DMA,
}HW_CE_PROC_MODE_E;

typedef enum  _HW_CE_TS_PID_FILT_EN_E
{
  HW_CE_TS_PID_FILT_DISABLE,			// 0,
  HW_CE_TS_PID_FILT_ENABLE,			// 1 ,
}HW_CE_TS_PID_FILT_EN_E;

typedef enum  _HW_CE_TS_FORCE_ENC_EN_E
{
  HW_CE_TS_FORCE_ENC_DISABLE,		// 0,
  HW_CE_TS_FORCE_ENC_ENABLE,			// 1 ,
}HW_CE_TS_FORCE_ENC_EN_E;

//==============================HW ADES================================
typedef enum  _HW_CE_ADES_ALGO_SEL_E
{
  HW_CE_ADES_ALGO_SEL_AES,		// 00
  HW_CE_ADES_ALGO_SEL_DES,		// 01
}HW_CE_ADES_ALGO_SEL_E;

typedef enum  _HW_CE_ADES_ALGO_MODE_E
{
  HW_CE_ADES_ALGO_MODE_AES128_ENC = 0,		// 0 00
  HW_CE_ADES_ALGO_MODE_DES_XXX_ENC = 0,	// 0 00
  //HW_CE_ADES_ALGO_MODE_AES192_ENC,			// 0 01
  //HW_CE_ADES_ALGO_MODE_DES_xxx_ENC,			// 0 01
  //HW_CE_ADES_ALGO_MODE_AES256_ENC,			// 0 10
  HW_CE_ADES_ALGO_MODE_TDES_ABA_ENC = 2,	// 0 10
  //HW_CE_ADES_ALGO_MODE_TDES_ABC_ENC,		// 0 11
  HW_CE_ADES_ALGO_MODE_AES128_DEC = 4,		// 1 00
  HW_CE_ADES_ALGO_MODE_DES_XXX_DEC = 4,	// 1 00
  //HW_CE_ADES_ALGO_MODE_AES192_DEC,			// 1 01
  //HW_CE_ADES_ALGO_MODE_DES_xxx_DEC,			// 1 01
  //HW_CE_ADES_ALGO_MODE_AES256_DEC,			// 1 10
  HW_CE_ADES_ALGO_MODE_TDES_ABA_DEC = 6,	// 1 10
  //HW_CE_ADES_ALGO_MODE_TDES_ABC_DEC,		// 1 11
}HW_CE_ADES_ALGO_MODE_E;

typedef enum  _HW_CE_ADES_WORK_MODE_E
{
  HW_CE_ADES_WORK_MODE_ECB,				//0000,
  HW_CE_ADES_WORK_MODE_CBC,			//0001 ,
  HW_CE_ADES_WORK_MODE_CTR,			//0010,
  HW_CE_ADES_WORK_MODE_CBCDVS042,		//0011 ,
  HW_CE_ADES_WORK_MODE_CBCCTS,			//0100 ,
  HW_CE_ADES_WORK_MODE_RCBCCTS,		//0101 ,
  HW_CE_ADES_WORK_MODE_ECBCTS,			//0110 ,
  HW_CE_ADES_WORK_MODE_CFB1= 8,			//1000 ,
  HW_CE_ADES_WORK_MODE_CFB8,			//1001 ,
  HW_CE_ADES_WORK_MODE_CFB,				//1010 ,
  HW_CE_ADES_WORK_MODE_OFB1= 12,		//1100 ,
  HW_CE_ADES_WORK_MODE_OFB8,			//1101 ,
  HW_CE_ADES_WORK_MODE_OFB,			//1110 ,
}HW_CE_ADES_WORK_MODE_E;

typedef struct _HW_CE_ADES_CTRL_S
{
  void *priv;
  HW_CE_CHANNEL_E chan_num;
  
  mt_u32 even_key_slot;
  mt_u32 odd_key_slot;
  //mt_u8 iv[16];
  //mt_u8 key[16];
  
  HW_CE_ADES_WORK_MODE_E work_mode;
  HW_CE_ADES_ALGO_MODE_E algo_mode;
  HW_CE_ADES_ALGO_SEL_E algo_sel;
  
  HW_CE_TS_ALGO_MODE_E ts_enc_dec;
  HW_CE_TS_ENC_KSEL_E ts_enc_ksel;
  HW_CE_TS_DEC_IND_E ts_dec_ind;
  HW_CE_TS_PKT_LEN_E ts_pkt_len;
  HW_CE_TS_SWITCH_E ts_on_off;
  HW_CE_TS_IVE_CAL_EN_E ts_ive_cal_en;
  HW_CE_TS_IVE_MODE_E ts_ive_mode;
  HW_CE_TS_CTS_MODE_E ts_cts_mode;	
  HW_CE_TS_SHORT_MODE_E ts_short_mode;
  HW_CE_TS_SMALL_MODE_E ts_small_mode;	
  HW_CE_TS_PID_FILT_EN_E		ts_pid0_filt_en;
  HW_CE_TS_PID_FILT_EN_E		ts_pid1_filt_en;
  HW_CE_TS_PID_FILT_EN_E		ts_pid2_filt_en;
  HW_CE_TS_PID_FILT_EN_E		ts_pid3_filt_en;
  HW_CE_TS_PID_FILT_EN_E		ts_pid4_filt_en;
  HW_CE_TS_PID_FILT_EN_E		ts_pid5_filt_en;
  HW_CE_TS_PID_FILT_EN_E		ts_pid6_filt_en;
  HW_CE_TS_PID_FILT_EN_E		ts_pid7_filt_en;
  mt_u16						ts_pid0_filt_num;
  mt_u16						ts_pid1_filt_num;
  mt_u16						ts_pid2_filt_num;
  mt_u16						ts_pid3_filt_num;
  mt_u16						ts_pid4_filt_num;
  mt_u16						ts_pid5_filt_num;
  mt_u16						ts_pid6_filt_num;
  mt_u16						ts_pid7_filt_num;
  HW_CE_TS_FORCE_ENC_EN_E	ts_force_enc_en;

  HW_CE_GRP_MODE_E	grp_mode;
  HW_CE_PROC_MODE_E proc_mode;

  mt_handle priv_hdlr;
} HW_CE_ADES_CTRL_S;

//==============================HW SHA================================
typedef enum  _HW_CE_SHA_ALGO_MODE_E
{
  HW_CE_HASH_ALGO_MODE_SHA1 = 1,
//  HW_CE_HASH_ALGO_MODE_SHA224,
  HW_CE_HASH_ALGO_MODE_SHA256 = 3,
  HW_CE_HASH_ALGO_MODE_HMAC_SHA256,
  HW_CE_HASH_ALGO_MODE_MAX,
} HW_CE_SHA_ALGO_MODE_E;

typedef enum  _HW_CE_SHA_GRP_MODE_E
{
  HW_CE_HASH_GRP_NONE,
  HW_CE_HASH_GRP_FIRST,
  HW_CE_HASH_GRP_MID,
  HW_CE_HASH_GRP_LAST,
} HW_CE_SHA_GRP_MODE_E;

typedef struct _HW_CE_SHA_GRP_CTX_S{
  mt_u32 total_length;
  mt_u32 state[8];
} HW_CE_SHA_GRP_CTX_S;

typedef struct _SW_CE_SHA_MGR_CTX_S{
  //mt_u32 cnt;
  mt_u32 rem_length;
  mt_u8 rem_data[128];
} SW_CE_SHA_MGR_CTX_S;

typedef struct _HW_CE_SHA_CTRL_S
{
  void *priv;
  HW_CE_CHANNEL_E chan_num;
  
  /* soft hmac key */
  mt_u8 hmac_key[128];
  mt_u32 key_size;

  /* hw hmac */
  mt_u32 key_slot;

  HW_CE_SHA_ALGO_MODE_E algo_mode;
  HW_CE_PROC_MODE_E proc_mode;
  HW_CE_SHA_GRP_MODE_E grp_mode;

  HW_CE_SHA_GRP_CTX_S grp_ctx;
  SW_CE_SHA_MGR_CTX_S mgr_ctx;
} HW_CE_SHA_CTRL_S;

//==============================HW ECC================================
/*!
  big number method type
  */
typedef enum  _HW_BN_METH_E
{
  HW_MOD_MUL = 3,
  HW_MOD_ADD,
  HW_MOD_SUB,
  HW_MOD_INV,
  HW_MOD_MOD,
  HW_MOD_EXP,
}HW_BN_METH_E;

typedef struct _HW_BN_OP_S
{
  void *priv;
  mt_u8 *a;
  mt_u8 *b;
  mt_u8 *m;
  mt_u8 *r;
  mt_u32 a_length;
  mt_u32 b_length;
  mt_u32 m_length;
} HW_BN_OP_S;

/*!
  ec method type
  */
typedef enum  _HW_EC_METH_E
{
  HW_POINT_MUL = 1,
  HW_POINT_ADD,
}HW_EC_METH_E;

typedef struct _HW_EC_POINT_ADD_S
{
  mt_u8 *pt1X;
  mt_u8 *pt1Y;
  mt_u8 *pt2X;
  mt_u8 *pt2Y;
}HW_EC_POINT_ADD_S;

typedef struct _HW_EC_POINT_MUL_S
{
  mt_u8 *ptX;
  mt_u8 *ptY;
  mt_u8 *scaler;
}HW_EC_POINT_MUL_S;

typedef struct _HW_EC_OP_S
{
  void *priv;
  mt_u8 *curve_p;
  mt_u8 *curve_a;
  union 
  {
    HW_EC_POINT_MUL_S ec_point_mul;
    HW_EC_POINT_ADD_S ec_point_add;
  };
  mt_u8 *rsX;
  mt_u8 *rsY;
  mt_u32 length;
} HW_EC_OP_S;

#if 0
//==============================HW KEYTABLE================================
typedef enum _HW_KT_SLOT_ACTIVE_E
{
	HW_KT_SLOT_DO_INACTIVE = 0,
	HW_KT_SLOT_DO_ACTIVE = 1,
}HW_KT_SLOT_ACTIVE_E;

typedef enum _HW_KT_SLOT_ID_E
{
	HW_KT_SLOT_ID_0,
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
	HW_KT_SLOT_ID_INVALID = 63,
	HW_KT_SLOT_MAX_NUM = 63,
	
}HW_KT_SLOT_ID_E;

typedef enum _HW_KT_SLOT_STATUS_E
{
	HW_KT_SLOT_FREE = 0,
	HW_KT_SLOT_BUSY = 1,
}HW_KT_SLOT_STATUS_E;

typedef enum _HW_KT_SLOT_SIZE_E
{
	HW_KT_SLOT_8B_SIZE = 8,    
	HW_KT_SLOT_16B_SIZE = 16,
}HW_KT_SLOT_SIZE_E;

typedef enum _HW_KT_OPERATION_E
{
	HW_KT_OPER_ACTIVE = 0,    
	HW_KT_OPER_W_KEY = 1,
	HW_KT_OPER_W_IV = 2,
	HW_KT_OPER_W_ATTRIBUTE = 3,
	HW_KT_OPER_R = 4,
}HW_KT_OPERATION_E;

typedef enum  _HW_KT_KEYATTR_KEYSIZE_E
{
	HW_KT_KEYATTR_8BKEY,				//0000,
	HW_KT_KEYATTR_16BKEY,			//0001 ,
}HW_KT_KEYATTR_KEYSIZE_E;

typedef enum  _HW_KT_KEYATTR_KEYSOURC_E
{
	HW_KT_KEYATTR_APCPU,				//000,
	HW_KT_KEYATTR_SCPU,				//001 ,
	HW_KT_KEYATTR_AKL,				//010,
	HW_KT_KEYATTR_CW_KL,			//011 ,
	HW_KT_KEYATTR_PVR_KL,			//100 ,
}HW_KT_KEYATTR_KEYSOURC_E;

typedef enum  _HW_KT_KEYATTR_TDES_KEY_CHK_E
{
	HW_KT_KEYATTR_NO_CHK,					//000,
	HW_KT_KEYATTR_WITH_PARITY_CHK,			//001 ,
	HW_KT_KEYATTR_WITHOUT_PARITY_CHK,		//010,
}HW_KT_KEYATTR_TDES_KEY_CHK_E;

typedef struct _HW_KT_KEY_ATTR_S
{
	mt_u32 AES_ONOFF;
	mt_u32 DES_ONOFF;
	mt_u32 TDES_ONOFF;
	mt_u32 CSAv2_ONOFF;
	mt_u32 CSAv3_ONOFF;
	mt_u32 SM2_3_4_ONOFF;
	mt_u32 HMAC_ONOFF;
	mt_u32 M2M_ONOFF;
	mt_u32 ASA_ONOFF;
	mt_u32 Multi2_ONOFF;
	mt_u32 REE_ONOFF;
	mt_u32 DEC_ONOFF;
	mt_u32 ENC_ONOFF;
	HW_KT_KEYATTR_KEYSIZE_E KEY_SIZE;
	HW_KT_KEYATTR_KEYSOURC_E KEY_SOURCE;
	HW_KT_KEYATTR_TDES_KEY_CHK_E TDES_KEYCHK;
}HW_KT_KEY_ATTR_S;
#endif





//mt_s32 hw_ce_ts_config_data_prepare(MT_CE_TS_CTRL_S *app_ctrl, HW_CE_ADES_CTRL_S *hw_ctrl);
//mt_s32 hw_ce_ades_config_data_prepare(MT_CE_ADES_CTRL_S *app_ctrl, HW_CE_ADES_CTRL_S *hw_ctrl);
mt_s32 hw_ce_ades_config(HW_CE_ADES_CTRL_S *hw_ctrl);
mt_s32 hw_ce_ades_process(HW_CE_ADES_CTRL_S *hw_ctrl, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length);
mt_s32 hw_ce_ades_process_start(HW_CE_ADES_CTRL_S *hw_ctrl, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length);
mt_s32 hw_ce_ades_process_polling(HW_CE_ADES_CTRL_S *hw_ctrl, mt_u32 time_out);
mt_s32 hw_ce_ades_process_stop(HW_CE_ADES_CTRL_S *hw_ctrl);

//mt_s32 hw_ce_sha_config_data_prepare(MT_CE_SHA_CTRL_S *app_ctrl, HW_CE_SHA_CTRL_S *hw_ctrl);
mt_s32 hw_ce_sha_init(HW_CE_SHA_CTRL_S *hw_ctrl);
mt_s32 hw_ce_sha_update(HW_CE_SHA_CTRL_S *hw_ctrl, mt_u8 *p_msg, mt_u32 length);
mt_s32 hw_ce_sha_final(HW_CE_SHA_CTRL_S *hw_ctrl, mt_u8 *p_dgst);
mt_s32 hw_ce_sha_update_start(HW_CE_SHA_CTRL_S *hw_ctrl, mt_u8 *p_msg, mt_u32 length);
mt_s32 hw_ce_sha_update_polling(HW_CE_SHA_CTRL_S *hw_ctrl, mt_u32 time_out);
mt_s32 hw_ce_sha_update_stop(HW_CE_SHA_CTRL_S *hw_ctrl);

/*mt_s32 hw_ce_rsa_process(HW_CE_RSA_CTRL_S *hw_ctrl, mt_u32 *p_src_addr, mt_u32 *p_dst_addr, mt_u32 src_length);*/
s32 hw_ce_rsa_process(mt_u32 *P, mt_u32 len_P, mt_u32 *E, mt_u32 len_E, 
        mt_u32 *p_src_addr, mt_u32 *p_dst_addr, mt_u32 src_length);

mt_s32 hw_bn_mod(HW_BN_METH_E meth, HW_BN_OP_S *op);
mt_s32 hw_ec_point(HW_EC_METH_E meth, HW_EC_OP_S *op);

mt_s32 hw_ce_reset(mt_u32 mode);

#endif	/*__HW_CE_IF_H__*/

