/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __KEY_LADDER_IOCTL_H__
#define __KEY_LADDER_IOCTL_H__
#include <linux/types.h>
#include "mt_module.h"
#include "mt_mpi_keyladder.h"

typedef struct _KL_KEY_BUFFER_S
{
    mt_u8 buffer[16];
}KL_KEY_BUFFER_S;

typedef struct _KL_KEY_SIGNATURE_S
{
    mt_u8 buffer[32];
}KL_KEY_SIGNATURE_S;

typedef struct _KL_SLOTID_S
{
    mt_u32 slot_id;
} KL_SLOTID_S;

typedef struct _KL_LINK_CRYPTO_S
{
    mt_u8 input[16];
    KL_MOVE_ALGO_E algo_type;
    KL_MOVE_ENC_E enc_ptye;
} KL_LINK_CRYPTO_S;

typedef struct _KL_LINK_HASH_S
{
    mt_u8 input[16];
    KL_HASH_POSITION_E input_data_pos;
    KL_HASH_POSITION_E output_key_pos;
}KL_LINK_HASH_S;

typedef struct _KL_LINK_SEEDV_S
{
    mt_u8 input[16];
    KL_HARDWIRED_SOURCE_E mask_key;
    KL_STANDARD_PROFILE_E profile;
}KL_LINK_SEEDV_S;

typedef struct _KL_INPUT_DATA_S
{
    KL_INPUT_POSITION_E postion;
    mt_u8 input[16];
}KL_INPUT_DATA_S;

typedef struct _KL_MOVE_CMD_S
{
    KL_MOVE_SRC_E move_src;
    KL_MOVE_DST_E move_dst;
	KL_MOVE_ENC_E move_enc;
    KL_MOVE_TRIGGER_E trigger;
}KL_MOVE_CMD_S;

typedef struct _KL_STORE_CMD_S
{
    KL_STORE_SRC_E store_src;
    KL_STORE_DST_E store_dst;
    KL_STORE_LOCK_E lock;
}KL_SOTRE_CMD_S;

typedef struct _KL_EXPORT_CMD_S
{
    KL_EXPORT_SOURCE_E key_src;
    KL_EXPORT_DST_E export_dst;
    mt_u32 slot_id;
}KL_EXPORT_CMD_S;

typedef struct _KL_EXPORT_KEY_S
{
    KL_EXPORT_DST_E export_dst;
    mt_u32 slot_id;
}KL_EXPORT_KEY_S;

typedef struct _KL_SETSIGSRC_S
{
	KL_SIG_CPU_E cpu;
	KL_SIG_KEYSRC_E keysrc;
} KL_SETSIGSRC_S;

typedef struct _KL_LOCKSIGSRC_S
{
	KL_SIG_CPU_E cpu;
	unsigned char lock;
} KL_LOCKSIGSRC_S;

typedef struct _KL_TDC_BUFFER_S
{
    mt_u8 buffer[416];
}KL_TDC_BUFFER_S;

typedef struct _KL_ADDITIONS_S
{
    KL_ADDITIONS_E addt;
    KL_FUNC_ENABLE_E en;
}KL_ADDITIONS_S;

#define KEYLADDER_DRV_IOC_LOCK              _IO(MT_ID_KEYLADDER, 0x0)
#define KEYLADDER_DRV_IOC_UNLOCK            _IO(MT_ID_KEYLADDER, 0x1)
#define KEYLADDER_DRV_IOC_SETSIGNATURE      _IOW(MT_ID_KEYLADDER, 0x2, KL_KEY_SIGNATURE_S)
#define KEYLADDER_DRV_IOC_SELECTROOTKEY     _IOW(MT_ID_KEYLADDER, 0x3, mt_u32)
#define KEYLADDER_DRV_IOC_LINKAES           _IOW(MT_ID_KEYLADDER, 0x4, KL_LINK_CRYPTO_S)
#define KEYLADDER_DRV_IOC_LINKTDES          _IOW(MT_ID_KEYLADDER, 0x5, KL_LINK_CRYPTO_S)
#define KEYLADDER_DRV_IOC_LINKXOR           _IOW(MT_ID_KEYLADDER, 0x6, KL_KEY_BUFFER_S)
#define KEYLADDER_DRV_IOC_LINKHASH          _IOW(MT_ID_KEYLADDER, 0x7, KL_LINK_HASH_S)
#define KEYLADDER_DRV_IOC_LINKSEEDV         _IOW(MT_ID_KEYLADDER, 0x8, KL_LINK_SEEDV_S)
#define KEYLADDER_DRV_IOC_STOREKEY          _IOW(MT_ID_KEYLADDER, 0x9, mt_u32)
#define KEYLADDER_DRV_IOC_EXPORTKEY         _IOW(MT_ID_KEYLADDER, 0xa, KL_EXPORT_KEY_S)
#define KEYLADDER_DRV_IOC_INPUTDATA         _IOW(MT_ID_KEYLADDER, 0xb, KL_INPUT_DATA_S)
#define KEYLADDER_DRV_IOC_MOVECMD           _IOW(MT_ID_KEYLADDER, 0xc, KL_MOVE_CMD_S)
#define KEYLADDER_DRV_IOC_STORECMD          _IOW(MT_ID_KEYLADDER, 0xd, KL_SOTRE_CMD_S)
#define KEYLADDER_DRV_IOC_EXPORTCMD         _IOW(MT_ID_KEYLADDER, 0xe, KL_EXPORT_CMD_S)
#define KEYLADDER_DRV_IOC_WAITCOMPLETE      _IOR(MT_ID_KEYLADDER, 0xf, mt_u32)
#define KEYLADDER_DRV_IOC_READKEY           _IOR(MT_ID_KEYLADDER, 0x10, KL_KEY_BUFFER_S)
#define KEYLADDER_DRV_IOC_GETSLOT           _IOR(MT_ID_KEYLADDER, 0x11, KL_SLOTID_S)
#define KEYLADDER_DRV_IOC_REQSEM            _IO(MT_ID_KEYLADDER, 0x12)
#define KEYLADDER_DRV_IOC_RLSSEM            _IO(MT_ID_KEYLADDER, 0x13)
#define KEYLADDER_DRV_IOC_EXTRTDC           _IOW(MT_ID_KEYLADDER, 0x14, KL_TDC_BUFFER_S)
#define KEYLADDER_DRV_IOC_RUNTDC            _IOW(MT_ID_KEYLADDER, 0x15, KL_LINK_CRYPTO_S)
#define KEYLADDER_DRV_IOC_ADDITION          _IOW(MT_ID_KEYLADDER, 0x16, KL_ADDITIONS_S)
#define KEYLADDER_DRV_IOC_SETSIGSRC         _IOW(MT_ID_KEYLADDER, 0x17, KL_LINK_CRYPTO_S)
#define KEYLADDER_DRV_IOC_LOCKSIGSRC        _IOW(MT_ID_KEYLADDER, 0x18, KL_ADDITIONS_S)

#endif

