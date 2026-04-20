/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_KEYTABLE_H__
#define __MT_MPI_KEYTABLE_H__
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MT_KT_ATTR_ON 1
#define MT_KT_ATTR_OFF 0

//==========================KEY TABLE================================
typedef enum _MT_KT_SLOT_ACTIVE_E
{
	MT_KT_SLOT_DO_INACTIVE = 0,
	MT_KT_SLOT_DO_ACTIVE = 1,
}MT_KT_SLOT_ACTIVE_E;

typedef enum _MT_KT_SLOT_ID_E
{
    MT_KT_SLOT_ID_0, //SYMPHONY4:NONCE
    MT_KT_SLOT_ID_1,
    MT_KT_SLOT_ID_2,
    MT_KT_SLOT_ID_3,
    MT_KT_SLOT_ID_4,
    MT_KT_SLOT_ID_5,
    MT_KT_SLOT_ID_6,
    MT_KT_SLOT_ID_7,
    MT_KT_SLOT_ID_8,
    MT_KT_SLOT_ID_9,
    MT_KT_SLOT_ID_10,
    MT_KT_SLOT_ID_11,
    MT_KT_SLOT_ID_12,
    MT_KT_SLOT_ID_13,
    MT_KT_SLOT_ID_14,
    MT_KT_SLOT_ID_15,
    MT_KT_SLOT_ID_16,
    MT_KT_SLOT_ID_17,
    MT_KT_SLOT_ID_18,
    MT_KT_SLOT_ID_19,
    MT_KT_SLOT_ID_20,
    MT_KT_SLOT_ID_21,
    MT_KT_SLOT_ID_22,
    MT_KT_SLOT_ID_23,
    MT_KT_SLOT_ID_24,
    MT_KT_SLOT_ID_25,
    MT_KT_SLOT_ID_26,
    MT_KT_SLOT_ID_27,
    MT_KT_SLOT_ID_28,
    MT_KT_SLOT_ID_29,
    MT_KT_SLOT_ID_30,
    MT_KT_SLOT_ID_31,
    MT_KT_SLOT_ID_32,
    MT_KT_SLOT_ID_33,
    MT_KT_SLOT_ID_34,
    MT_KT_SLOT_ID_35,
    MT_KT_SLOT_ID_36,
    MT_KT_SLOT_ID_37,
    MT_KT_SLOT_ID_38,
    MT_KT_SLOT_ID_39,
    MT_KT_SLOT_ID_40,
    MT_KT_SLOT_ID_41,
    MT_KT_SLOT_ID_42,
    MT_KT_SLOT_ID_43,
    MT_KT_SLOT_ID_44,
    MT_KT_SLOT_ID_45,
    MT_KT_SLOT_ID_46,
    MT_KT_SLOT_ID_47,
    MT_KT_SLOT_ID_48,
    MT_KT_SLOT_ID_49,
    MT_KT_SLOT_ID_50,
    MT_KT_SLOT_ID_51,
    MT_KT_SLOT_ID_52,
    MT_KT_SLOT_ID_53,
    MT_KT_SLOT_ID_54,
    MT_KT_SLOT_ID_55,
    MT_KT_SLOT_ID_56,
    MT_KT_SLOT_ID_57,
    MT_KT_SLOT_ID_58,
    MT_KT_SLOT_ID_59,
    MT_KT_SLOT_ID_60,
    MT_KT_SLOT_ID_61,
    MT_KT_SLOT_ID_62,
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_KT_SLOT_ID_63,
    MT_KT_SLOT_ID_64,
    MT_KT_SLOT_ID_65,
    MT_KT_SLOT_ID_66,
    MT_KT_SLOT_ID_67,
    MT_KT_SLOT_ID_68,
    MT_KT_SLOT_ID_69,
    MT_KT_SLOT_ID_70,
    MT_KT_SLOT_ID_71,
    MT_KT_SLOT_ID_72,
    MT_KT_SLOT_ID_73,
    MT_KT_SLOT_ID_74,
    MT_KT_SLOT_ID_75,
    MT_KT_SLOT_ID_76,
    MT_KT_SLOT_ID_77,
    MT_KT_SLOT_ID_78,
    MT_KT_SLOT_ID_79,
    MT_KT_SLOT_ID_80,
    MT_KT_SLOT_ID_81,
    MT_KT_SLOT_ID_82,
    MT_KT_SLOT_ID_83,
    MT_KT_SLOT_ID_84,
    MT_KT_SLOT_ID_85,
    MT_KT_SLOT_ID_86,
    MT_KT_SLOT_ID_87,
    MT_KT_SLOT_ID_88,
    MT_KT_SLOT_ID_89,
    MT_KT_SLOT_ID_90,
    MT_KT_SLOT_ID_91,
    MT_KT_SLOT_ID_92,
    MT_KT_SLOT_ID_93,
    MT_KT_SLOT_ID_94,
    MT_KT_SLOT_ID_95,
    MT_KT_SLOT_ID_96,
    MT_KT_SLOT_ID_97,
    MT_KT_SLOT_ID_98,
    MT_KT_SLOT_ID_99,
    MT_KT_SLOT_ID_100,
    MT_KT_SLOT_ID_101,
    MT_KT_SLOT_ID_102,
    MT_KT_SLOT_ID_103,
    MT_KT_SLOT_ID_104,
    MT_KT_SLOT_ID_105,
    MT_KT_SLOT_ID_106,
    MT_KT_SLOT_ID_107,
    MT_KT_SLOT_ID_108,
    MT_KT_SLOT_ID_109,
    MT_KT_SLOT_ID_110,
    MT_KT_SLOT_ID_111,
    MT_KT_SLOT_ID_112,
    MT_KT_SLOT_ID_113,
    MT_KT_SLOT_ID_114,
    MT_KT_SLOT_ID_115,
    MT_KT_SLOT_ID_116,
    MT_KT_SLOT_ID_117,
    MT_KT_SLOT_ID_118,
    MT_KT_SLOT_ID_119,
    MT_KT_SLOT_ID_120,
    MT_KT_SLOT_ID_121,
    MT_KT_SLOT_ID_122,
    MT_KT_SLOT_ID_123,
    MT_KT_SLOT_ID_124,
    MT_KT_SLOT_ID_125,
    MT_KT_SLOT_ID_126,
    MT_KT_SLOT_ID_127,
#endif
    MT_KT_SLOT_MAX_NUM,
    MT_KT_SLOT_ID_INVALID = 0xFF,
}MT_KT_SLOT_ID_E;

typedef enum _MT_KT_SLOT_SIZE_E
{
	MT_KT_SLOT_8B_SIZE = 8,    
	MT_KT_SLOT_16B_SIZE = 16,
	MT_KT_SLOT_32B_SIZE = 32,
}MT_KT_SLOT_SIZE_E;

typedef enum  _MT_KT_KEYATTR_KEYSIZE_E
{
	MT_KT_KEYATTR_8BKEY = 0x1,				//0000,
	MT_KT_KEYATTR_16BKEY = 0x10,			//0001 ,
}MT_KT_KEYATTR_KEYSIZE_E;

typedef enum  _MT_KT_KEYATTR_KEYSOURC_E
{
	MT_KT_KEYATTR_APCPU,				//000,
	MT_KT_KEYATTR_SCPU,				//001 ,
	MT_KT_KEYATTR_AKL,				//010,
	MT_KT_KEYATTR_CW_KL,			//011 ,
	MT_KT_KEYATTR_PVR_KL,			//100 ,
}MT_KT_KEYATTR_KEYSOURC_E;

typedef enum  _MT_KT_KEYATTR_TDES_KEY_CHK_E
{
	MT_KT_KEYATTR_NO_CHK,					//000,
	MT_KT_KEYATTR_WITH_PARITY_CHK,			//001 ,
	MT_KT_KEYATTR_WITHOUT_PARITY_CHK,		//010,
}MT_KT_KEYATTR_TDES_KEY_CHK_E;

typedef struct _MT_KT_KEY_ATTR_S
{
    mt_u8 AES_ONOFF;
    mt_u8 DES_ONOFF;
    mt_u8 TDES_ONOFF;
    mt_u8 CSAv2_ONOFF;
    mt_u8 CSAv3_ONOFF;
    mt_u8 SM2_3_4_ONOFF;
    mt_u8 ASA_ONOFF;
    mt_u8 CSAv2_CONFORMANCE_ONOFF;
    mt_u8 GOST_28147_89_OR_R34_12_MAGMA_ONOFF;
    mt_u8 GOST_R34_12_KUZNYECHIK_ONOFF;
    mt_u8 TS_ONOFF;
    mt_u8 M2M_ONOFF;
    mt_u8 MAC_ONOFF;
    mt_u8 Multi2_ONOFF;
    mt_u8 REE_ONOFF;
    mt_u8 DEC_ONOFF;
    mt_u8 ENC_ONOFF;
    mt_u32 KEY_SIZE;
    mt_u32 IV_SIZE;
    mt_u32 KEY_SOURCE;
    mt_u32 TDES_KEYCHK;
}MT_KT_KEY_ATTR_S;

typedef enum _MT_KT_SLOT_STATE_E
{
	MT_KT_STATE_INITED,                 /* the kt slot has been initialized */
	MT_KT_STATE_UNINIT,                 /* the kt slot has not been initialized */
	MT_KT_STATE_INVALID,               /* the kt slot has not been requested */
	MT_KT_STATE_READ_DIS,            /* OTP_KTAttrReadDis has been set */
}MT_KT_SLOT_STATE_E;

/*mt_s32 mt_mpi_kt_init(mt_void);*/
/*mt_s32 mt_mpi_kt_deinit(mt_void);*/
mt_s32 mt_mpi_kt_open(mt_handle *p_handle);
mt_s32 mt_mpi_kt_close(mt_handle handle);
mt_s32 mt_mpi_kt_slot_request(mt_handle handle, unsigned int *p_slot_id);
mt_s32 mt_mpi_kt_slot_request_multi(mt_handle handle, unsigned int num, unsigned int *p_slot_id);
mt_s32 mt_mpi_kt_slot_release(mt_handle handle, unsigned int slot_id);
mt_s32 mt_mpi_kt_slot_active(mt_handle handle, unsigned int slot_id, MT_KT_SLOT_ACTIVE_E active);
mt_s32 mt_mpi_kt_write_attr(mt_handle handle, unsigned int slot_id, MT_KT_KEY_ATTR_S attr);
mt_s32 mt_mpi_kt_read_attr(mt_handle handle, unsigned int slot_id, MT_KT_KEY_ATTR_S *p_attr);
mt_s32 mt_mpi_kt_write_key(mt_handle handle, unsigned int slot_id, const mt_u8 *p_key, MT_KT_SLOT_SIZE_E size);
mt_s32 mt_mpi_kt_read_key(mt_handle handle, unsigned int slot_id, mt_u8 *p_key, MT_KT_SLOT_SIZE_E size);
mt_s32 mt_mpi_kt_write_iv(mt_handle handle, unsigned int slot_id, const mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size);
mt_s32 mt_mpi_kt_read_iv(mt_handle handle, unsigned int slot_id, mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size);
mt_s32 mt_mpi_kt_get_state(mt_handle handle, mt_u32 slot_id, MT_KT_SLOT_STATE_E *p_state);

mt_s32 mt_mpi_kt_slot_info(mt_handle handle, mt_u32 slot_id);
mt_s32 mt_mpi_kt_control_status(mt_handle handle);

#ifdef CONFIG_MT_CHIP_SYMPHONY6
mt_s32 mt_mpi_kt_read_metadata(mt_handle handle, mt_u32 slot_id, mt_u32 *p_metadata);
#endif

#ifdef __cplusplus
}
#endif

#endif //__MT_UNF_KEYTABLE_H__

