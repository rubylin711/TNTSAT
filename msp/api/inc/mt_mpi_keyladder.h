/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_KEY_LADDER_H__
#define __MT_MPI_KEY_LADDER_H__
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum _KL_SEM_CPU_E
{
    KL_SEM_CPU_APCPU_ID_REE			= 0x0,
    KL_SEM_CPU_SECCPU_ID_TEE		= 0x5,
    KL_SEM_CPU_VSCPU_ID             = 0x4,
} KL_SEM_CPU_E;

typedef enum {
    KL_SELECT_SRC_SCK0          = 0x0,
    KL_SELECT_SRC_SCK1,
    KL_SELECT_SRC_SCK2,
    KL_SELECT_SRC_SCK3,
    KL_SELECT_SRC_SCK4,
    KL_SELECT_SRC_SCK5,
    KL_SELECT_SRC_SCK6,
    KL_SELECT_SRC_SCK7,
    KL_SELECT_SRC_SCK8,
    KL_SELECT_SRC_SCK9,
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_SELECT_SRC_SCK10,
    KL_SELECT_SRC_SCK11,
    KL_SELECT_SRC_SCK12,
    KL_SELECT_SRC_SCK13,
    KL_SELECT_SRC_SCK14,
    KL_SELECT_SRC_SCK15,
    KL_SELECT_SRC_HWSCK0,
    KL_SELECT_SRC_HWSCK1,
    KL_SELECT_SRC_HWSCK2,
    KL_SELECT_SRC_HWSCK3,
    KL_SELECT_SRC_HWSCK4,
    KL_SELECT_SRC_PRIVATE0 		= 0x18,
    KL_SELECT_SRC_PRIVATE1 		= 0x19,
    /* 0x1a ~ 0x1b only for CRI mode */
    KL_SELECT_SRC_CFAES_KEY		= 0x1a,
    KL_SELECT_SRC_CFCWC 		= 0x1b,
#else
    KL_SELECT_SRC_PRIVATE0      = 0xa,
    KL_SELECT_SRC_PRIVATE1      = 0xb,
#endif
} KL_SCK_SOURCE_E;

typedef enum _KL_MOVE_SRC_E
{
    KL_MOVE_SRC_SCK_SELECTED         = 0x0,
    KL_MOVE_SRC_AES_DOUT             = 0x1,
    KL_MOVE_SRC_TDES_OUT             = 0x2,
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_MOVE_SRC_SM4_OUT              = 0x3,
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_MOVE_SRC_INVT_DOUT            = 0x4,
#endif
    KL_MOVE_SRC_HARDWIRED_KEY0       = 0x5,
    KL_MOVE_SRC_HARDWIRED_KEY1       = 0x6,
    KL_MOVE_SRC_HARDWIRED_KEY2       = 0x7,
    KL_MOVE_SRC_HARDWIRED_KEY3       = 0x8,
    KL_MOVE_SRC_HARDWIRED_KEY4       = 0x9,
    KL_MOVE_SRC_HARDWIRED_KEY5       = 0xa,
    KL_MOVE_SRC_HARDWIRED_KEY6       = 0xb,
    KL_MOVE_SRC_HARDWIRED_KEY7       = 0xc,
    KL_MOVE_SRC_HASH_DOUT_L          = 0xd,
    KL_MOVE_SRC_HASH_DOUT_H          = 0xe,
    KL_MOVE_SRC_XOR_DOUT             = 0xf,
    KL_MOVE_SRC_REG_CPU0             = 0x10,
    KL_MOVE_SRC_REG_CPU1             = 0x11,
    KL_MOVE_SRC_REG_CPU2             = 0x12,
    KL_MOVE_SRC_REG_CPU3             = 0x13,
    KL_MOVE_SRC_REG_CPU4             = 0x14,
    KL_MOVE_SRC_REG_CPU5             = 0x15,
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_MOVE_SRC_CWCW0                = 0x18,
    KL_MOVE_SRC_CWCW1                = 0x19,
    KL_MOVE_SRC_CWCW2                = 0x1a,
    KL_MOVE_SRC_CWCW3                = 0x1b,
    KL_MOVE_SRC_AUTO_DATA            = 0x51,
    KL_MOVE_SRC_AUTO_KEY             = 0x52,
#endif
} KL_MOVE_SRC_E;

typedef enum _KL_MOVE_DST_E
{
    KL_MOVE_DST_AES_KEY              = 0x0,
    KL_MOVE_DST_AES_DIN              = 0x1,
    KL_MOVE_DST_TDES_KEY             = 0x2,
    KL_MOVE_DST_TDES_DIN             = 0x3,
    KL_MOVE_DST_HASH_DIN_L           = 0x4,
    KL_MOVE_DST_XOR_DIN_A            = 0x5,
    KL_MOVE_DST_XOR_DIN_B            = 0x6,
    KL_MOVE_DST_HASH_DIN_H           = 0x7,
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_MOVE_DST_INVT_DIN             = 0x8,
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_MOVE_DST_SM4_KEY = 0xc,
    KL_MOVE_DST_SM4_DIN = 0xd,
#endif
} KL_MOVE_DST_E;

typedef enum _KL_MOVE_TRIGGER_E
{
    KL_ADDS_MOVE               = 0x0,
    KL_ADDS_MOVE_AND_TRIGGER   = 0x1,
} KL_MOVE_TRIGGER_E;

typedef enum _KL_MOVE_ALGO_E
{
    KL_MOVE_ALGO_TDES          = 0x0,
    KL_MOVE_ALGO_AES           = 0x1
} KL_MOVE_ALGO_E;

typedef enum _KL_MOVE_ENC_E
{
    KL_MOVE_ADDs_DEC           = 0x0,
    KL_MOVE_ADDs_ENC           = 0x1
} KL_MOVE_ENC_E;

typedef enum _KL_STORE_SRC_E
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_STORE_SRC_AES_DOUT      =    0x10,
    KL_STORE_SRC_TDES_DOUT     =    0x11,
    KL_STORE_SRC_HASH_DOUT_H   =    0x12,
    KL_STORE_SRC_HASH_DOUT_L   =    0x13,
    KL_STORE_SRC_XOR_DOUT      =    0x14,
    /* 0x15 only for CRI mode */
    KL_STORE_SRC_SCK_SELECTED  =    0x15,
#else
    KL_STORE_SRC_AES_DOUT      =    0x0,
    KL_STORE_SRC_TDES_DOUT     =    0x1,
    KL_STORE_SRC_HASH_DOUT_H   =    0x2,
    KL_STORE_SRC_HASH_DOUT_L   =    0x3,
    KL_STORE_SRC_XOR_DOUT      =    0x4,
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_STORE_SRC_SM4_DOUT		=    0x16,
#endif
} KL_STORE_SRC_E;

typedef enum _KL_STORE_DST_E
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_STORE_DST_PRIVATE0      = 0x14,
    KL_STORE_DST_PRIVATE1      = 0x15,
    /* 0x18 ~ 0x1b only for CRI & Conax mode */
    KL_STORE_DST_CWCW0         = 0x18,
    KL_STORE_DST_CWCW1         = 0x19,
    KL_STORE_DST_CWCW2         = 0x1a,
    KL_STORE_DST_CWCW3         = 0x1b,
    /* 0x1e ~ 0x1f only for CRI mode */
    KL_STORE_DST_CFAES_KEY     = 0x1e,
    KL_STORE_DST_HWDECM        = 0x1f,
#else
    KL_STORE_DST_PRIVATE0	   = 0x0,
    KL_STORE_DST_PRIVATE1	   = 0x1,
#endif
} KL_STORE_DST_E;

typedef  enum _KL_EXPORT_KEY_SOURCE_E
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_EXPORT_SRC_AES_DOUT          = 0x18,
    KL_EXPORT_SRC_TDES_DOUT         = 0x19,
    KL_EXPORT_SRC_HASH_DOUT_H       = 0x1a,
    KL_EXPORT_SRC_HASH_DOUT_L       = 0x1b,
    KL_EXPORT_SRC_XOR_DOUT          = 0x1c,
    KL_EXPORT_SRC_SCK_SELECTED      = 0x1d,
#else
    KL_EXPORT_SRC_AES_DOUT          = 0x0,
    KL_EXPORT_SRC_TDES_DOUT         = 0x1,
    KL_EXPORT_SRC_HASH_DOUT_H       = 0x2,
    KL_EXPORT_SRC_HASH_DOUT_L       = 0x3,
    KL_EXPORT_SRC_XOR_DOUT          = 0x4,
    KL_EXPORT_SRC_SCK_SELECTED      = 0x5,
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_EXPORT_SRC_SM4_DOUT			=       0x1e,
#endif
} KL_EXPORT_SOURCE_E;

typedef  enum _KL_EXPORT_DST_E
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    KL_EXPORT_DST_KT                = 0x1,
    KL_EXPORT_DST_SCPU              = 0xa,
    KL_EXPORT_DST_JTAG              = 0x13,
    KL_EXPORT_DST_ACPU              = 0x15,
#else
    KL_EXPORT_DST_KT                = 0x0,
    KL_EXPORT_DST_ACPU              = 0x1,
    KL_EXPORT_DST_SCPU              = 0x2,
#endif
} KL_EXPORT_DST_E;

typedef enum _KL_HASH_POSITION_E
{
    KL_HASH_FIRST_HALF  = 0x0,
    KL_HASH_SECOND_HALF = 0x1,
} KL_HASH_POSITION_E;

typedef enum _KEYLADDER_TYPE_E
{
    KL_TYPE_0  = 0x0,
    KL_TYPE_1  = 0x1,
} KEYLADDER_TYPE_E;

typedef enum _KL_HARDWIRED_SOURCE_E
{
    KL_HARDWIRED_KEY0 = 0x0,
    KL_HARDWIRED_KEY1 = 0x1,
    KL_HARDWIRED_KEY2 = 0x2,
    KL_HARDWIRED_KEY3 = 0x3,
    KL_HARDWIRED_KEY4 = 0x4,
    KL_HARDWIRED_KEY5 = 0x5,
    KL_HARDWIRED_KEY6 = 0x6,
    KL_HARDWIRED_KEY7 = 0x7,
} KL_HARDWIRED_SOURCE_E;

typedef enum _KL_STANDARD_PROFILE_E
{
    KL_SCTE_201_2013_P0 = 0x0,
    KL_SCTE_201_2013_P1 = 0x1,
    KL_SCTE_201_2013_P1A = 0x2,
    KL_SCTE_201_2013_P2 = 0x3,
    KL_SCTE_201_2013_P2A = 0x4,
    KL_SCTE_201_2013_P2B = 0x5,
    KL_ETSI_TS_103_162 = 0x6,
    KL_GY_T_255_2012 = 0x7,
    KL_LAST_PROFILE = 0x8,
} KL_STANDARD_PROFILE_E;

typedef enum _KL_STORE_LOCK_E
{
    KL_STORE_ADDS_NO_LOCK      = 0x0,
    KL_STORE_ADDS_LOCK         = 0x1,
} KL_STORE_LOCK_E;

typedef enum _KL_INPUT_POSITION_E
{
    KL_INPUT_REG_CPU0 = 0x0,
    KL_INPUT_REG_CPU1 = 0x1,
    KL_INPUT_REG_CPU2 = 0x2,
    KL_INPUT_REG_CPU3 = 0x3,
    KL_INPUT_REG_CPU4 = 0x4,
    KL_INPUT_REG_CPU5 = 0x5,
    KL_INPUT_REG_MAX,
} KL_INPUT_POSITION_E;

typedef enum {
    KL_ADDT_EXPORTSWAP = 0x0,
    KL_ADDT_MAXNUM
} KL_ADDITIONS_E;

typedef enum {
    KL_DISABLE = 0x0,
    KL_ENABLE = 0x1
} KL_FUNC_ENABLE_E;

typedef enum {
    KL_SEM_CPU_ID_APCPU             = 0x0,
    KL_SEM_CPU_ID_SECCPU            = 0x5,
    KL_SEM_CPU_ID_VSCPU             = 0x4,
} KL_SEM_CPU_ID;

typedef enum {
    KL_SIG_KEYSRC_HWKEY0 = 0,
    KL_SIG_KEYSRC_HWKEY1_KPH
} KL_SIG_KEYSRC_E;

typedef enum {
    KL_SIG_CPU_REE = 0,
    KL_SIG_CPU_TEE,
    KL_SIG_CPU_VSCPU
} KL_SIG_CPU_E;

mt_s32 mt_mpi_kl_open(KEYLADDER_TYPE_E type, mt_handle *p_handle);

mt_s32 mt_mpi_kl_close(mt_handle handle);

mt_s32 mt_mpi_kl_lock(mt_handle handle);

mt_s32 mt_mpi_kl_unlock(mt_handle handle);

mt_s32 mt_mpi_kl_request_sem(mt_handle handle);

mt_s32 mt_mpi_kl_release_sem(mt_handle handle);

mt_s32 mt_mpi_kl_set_signature(mt_handle handle, mt_u8 *p_signature);

mt_s32 mt_mpi_kl_select_rootkey(mt_handle handle, KL_SCK_SOURCE_E rootkey_source);

mt_s32 mt_mpi_kl_link_aes(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type);

mt_s32 mt_mpi_kl_link_tdes(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type);

mt_s32 mt_mpi_kl_export_key(mt_handle handle, KL_EXPORT_DST_E kl_dst, mt_u32 slot_id);

mt_s32 mt_mpi_kl_wait_complete(mt_handle handle, mt_s32 *p_error);

mt_s32 mt_mpi_kl_read_key(mt_handle handle, mt_u8 *p_key_buffer);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 mt_mpi_kl_extr_tdc(mt_handle handle, mt_u8 *tdc);

mt_s32 mt_mpi_kl_run_tdc(mt_handle handle, mt_u8 *p_input, KL_MOVE_ALGO_E algo_type, KL_MOVE_ENC_E enc_type);

mt_s32 mt_mpi_kl_storeKey(mt_handle handle, KL_STORE_DST_E store_dst);

mt_s32 mt_mpi_kl_extra(mt_handle handle, KL_ADDITIONS_E addt, KL_FUNC_ENABLE_E en);

mt_s32 mt_mpi_kl_LinkSeedv(mt_handle handle, mt_u8 *p_input, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile);

mt_s32 mt_mpi_kl_InputData(mt_handle handle, KL_INPUT_POSITION_E postion, mt_u8 *p_input);
#endif

/*mt_s32 mt_mpi_kl_LinkXOR(mt_handle handle, mt_u8 *p_input);*/

/*mt_s32 mt_mpi_kl_LinkHash(mt_handle handle, mt_u8 *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos);*/

/*mt_s32 mt_mpi_kl_LinkSeedv(mt_handle handle, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile);*/

/*mt_s32 mt_mpi_kl_MoveCmd(mt_handle handle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger);*/

/*mt_s32 mt_mpi_kl_StoreCmd(mt_handle handle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock);*/

/*mt_s32 mt_mpi_kl_ExportCmd(mt_handle handle, KL_EXPORT_SOURCE_E key_src);*/

#ifdef __cplusplus
}
#endif

#endif //__MT_MPI_KEY_LADDER_H__

