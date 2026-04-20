/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_mgr.h"
#include "hw_m2m_register.h"

#define CTR_GCM_DEC_REE_DIS      (1 << 3)
#define CTR_GCM_ENC_REE_DIS      (1 << 2)
#define DEC_FORCE                (1 << 1)
#define ENC_FORCE                (1 << 0)
#define REE_LEN_EN               (1 << 2)

void m2m_allow_key_src(mt_u32 channel, mt_u32 key_src)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_KEYSRC_OPT(channel);

    io_write32(addr, io_read32(addr) | key_src);
}

void m2m_forbid_key_src(mt_u32 channel, mt_u32 key_src)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_KEYSRC_OPT(channel);

    io_write32(addr, io_read32(addr) & ~key_src);
}

void m2m_lock_key_src(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_KEYSRC_OPT(channel);

    io_write32(addr, io_read32(addr) | 1 << 31);
}

void m2m_enable_ctr_gcm_enc_for_ree(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) & ~CTR_GCM_ENC_REE_DIS);
}

void m2m_disable_ctr_gcm_enc_for_ree(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) | CTR_GCM_ENC_REE_DIS);
}

void m2m_enable_ctr_gcm_dec_for_ree(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) & ~CTR_GCM_DEC_REE_DIS);
}

void m2m_disable_ctr_gcm_dec_for_ree(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) | CTR_GCM_DEC_REE_DIS);
}

void m2m_enable_ts_mode_force_enc(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) | ENC_FORCE);
}

void m2m_disable_ts_mode_force_enc(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) & ~ENC_FORCE);
}

void m2m_enable_ts_mode_force_dec(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) | DEC_FORCE);
}

void m2m_disable_ts_mode_force_dec(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_ENC_DEC_FORCE(channel);

    io_write32(addr, io_read32(addr) & ~DEC_FORCE);
}

void m2m_forbid_channel_access(mt_u32 cpu_id, mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_FORBID_ACCESS_REG(channel);

    io_write32(addr, io_read32(addr) | 1 << cpu_id);
}

void m2m_forbid_bgc_access(mt_u32 cpu_id)
{
    mt_u32 addr = M2M_BGC_ACCESS_FORBID_REG;

    io_write32(addr, io_read32(addr) | 1 << cpu_id);
}

void m2m_set_ree_len_limit(mt_u32 channel, mt_u32 limit)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_REE_LEN_LIMIT(channel);

    io_write32(addr, limit);
}

void m2m_enable_ree_len_limit(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_REE_LEN_LIMIT(channel);

    io_write32(addr, io_read32(addr) | REE_LEN_EN);
}

void m2m_disable_ree_len_limit(mt_u32 channel)
{
    mt_u32 addr;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    addr = M2M_CHn_REE_LEN_LIMIT(channel);

    io_write32(addr, io_read32(addr) & ~REE_LEN_EN);
}
