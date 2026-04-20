/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_KT_IF_S2_H__
#define __HW_KT_IF_S2_H__

#include "../kt_result.h"



//==============================HW KEYTABLE================================
#if 0
typedef enum _HW_KT_SLOT_ACTIVE_E
{
	HW_KT_SLOT_DO_INACTIVE = 0,
	HW_KT_SLOT_DO_ACTIVE = 1,
}HW_KT_SLOT_ACTIVE_E;
#endif
typedef enum _HW_KT_SLOT_VALID_E
{
    HW_KT_SLOT_INVALIDATE = 0,
    HW_KT_SLOT_VALIDATE = 1,
}HW_KT_SLOT_VALID_E;

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
	HW_KT_SLOT_ID_62, // flash
	HW_KT_SLOT_MAX_NUM,
	HW_KT_SLOT_ID_INVALID = 0xFF,
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

void hw_kt_write_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E valid);
void hw_kt_read_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E *p_valid);
void hw_kt_write_attribute(HW_KT_SLOT_ID_E slot_id, HW_KT_KEY_ATTR_S key_attr);
void hw_kt_read_attribute(HW_KT_SLOT_ID_E slot_id, HW_KT_KEY_ATTR_S *p_attr);
void hw_kt_write_key(HW_KT_SLOT_ID_E slot_id, mt_u8 *p_key, HW_KT_SLOT_SIZE_E size);
void hw_kt_read_key(HW_KT_SLOT_ID_E slot_id, mt_u8 *p_key, HW_KT_SLOT_SIZE_E size);
void hw_kt_write_iv(HW_KT_SLOT_ID_E slot_id,  mt_u8 *p_iv, HW_KT_SLOT_SIZE_E size);
void hw_kt_read_iv(HW_KT_SLOT_ID_E slot_id, mt_u8 *p_iv, HW_KT_SLOT_SIZE_E size);

#endif	/*__HW_KT_IF_S2_H__*/

