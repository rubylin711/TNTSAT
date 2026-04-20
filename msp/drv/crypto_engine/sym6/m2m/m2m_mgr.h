/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_MGR_H_
#define _M2M_MGR_H_

#include "mt_lib.h"

enum M2M_KEY_SRC
{
    M2M_KEY_SRC_REECPU = 1 << 0,
    M2M_KEY_SRC_TEECPU = 1 << 1,
    M2M_KEY_SRC_VSCPU = 1 << 2,
    M2M_KEY_SRC_KLE_BY_VSCPU = 1 << 3,
    M2M_KEY_SRC_KLE_BY_REECPU = 1 << 4,
    M2M_KEY_SRC_KLE_BY_TEECPU = 1 << 5,
    M2M_KEY_SRC_AKL_BY_REECPU = 1 << 6,
    M2M_KEY_SRC_AKL_BY_TEECPU = 1 << 7,
    M2M_KEY_SRC_KLM_BY_REECPU = 1 << 8,
    M2M_KEY_SRC_KLM_BY_TEECPU = 1 << 9,
};

enum M2M_CPU_ID
{
    M2M_CPU_ID_REECPU = 0,
    M2M_CPU_ID_TEECPU,
    M2M_CPU_ID_VSCPU,
};

enum M2M_REE_LEN_LIMIT
{
    M2M_REE_LEN_LIMIT_256M = 0,
    M2M_REE_LEN_LIMIT_64M,
    M2M_REE_LEN_LIMIT_8M,
    M2M_REE_LEN_LIMIT_1M,
};

void m2m_allow_key_src(mt_u32 channel, mt_u32 key_src);

void m2m_forbid_key_src(mt_u32 channel, mt_u32 key_src);

void m2m_lock_key_src(mt_u32 channel);

void m2m_enable_ctr_gcm_enc_for_ree(mt_u32 channel);

void m2m_disable_ctr_gcm_enc_for_ree(mt_u32 channel);

void m2m_enable_ctr_gcm_dec_for_ree(mt_u32 channel);

void m2m_disable_ctr_gcm_dec_for_ree(mt_u32 channel);

void m2m_enable_ts_mode_force_enc(mt_u32 channel);

void m2m_disable_ts_mode_force_enc(mt_u32 channel);

void m2m_enable_ts_mode_force_dec(mt_u32 channel);

void m2m_disable_ts_mode_force_dec(mt_u32 channel);

void m2m_forbid_channel_access(mt_u32 cpu_id, mt_u32 channel);

void m2m_forbid_bgc_access(mt_u32 cpu_id);

void m2m_set_ree_len_limit(mt_u32 channel, mt_u32 limit);

void m2m_enable_ree_len_limit(mt_u32 channel);

void m2m_disable_ree_len_limit(mt_u32 channel);

#endif /* end of include guard: _M2M_MGR_H_ */
