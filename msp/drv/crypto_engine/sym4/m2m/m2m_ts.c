/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_ts.h"
#include "hw_m2m_register.h"

#define TS_PACKET_MAX_LEN (188)

mt_s32 m2m_ts_init(m2m_ts_ctx_t *ctx, mt_u32 profile, mt_u32 channel,
                   mt_u8 even_key_slot, mt_u8 odd_key_slot,
                   const mt_u8 *direct_key, const mt_u8 *direct_iv)
{
    mt_u32 key_size = 0;
    mt_u32 iv_size = 0;
    mt_u32 algo;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memset(ctx, 0, sizeof(m2m_ts_ctx_t));

    algo = HW_M2M_CIPHER_ALGO(profile);
    switch (algo)
    {
    case HW_M2M_CIPHER_ALGO_DES:
        key_size = 8;
        iv_size = 8;
        break;
    case HW_M2M_CIPHER_ALGO_TDES:
        key_size = 16;
        iv_size = 8;
        break;
    case HW_M2M_CIPHER_ALGO_AES128:
        key_size = 16;
        iv_size = 16;
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    ctx->profile = profile;
    ctx->channel = channel;
    ctx->even_key_slot = even_key_slot;
    ctx->odd_key_slot = odd_key_slot;
    if ((ctx->even_key_slot == HW_M2M_INVALID_KEY_SLOT)
            && (ctx->odd_key_slot == HW_M2M_INVALID_KEY_SLOT))
    {
        /*
         * 8 byte key stored in high 64 bit memory while 8 byte iv in low 64 bit.
         * The endian order of ctx->dk and ctx->div is big-endian.
         */
        if (direct_key)
            memcpy(ctx->dk, direct_key, key_size);
        if (direct_iv)
            memcpy(ctx->div + sizeof(ctx->div) - iv_size, direct_iv, iv_size);
    }

    return HW_CE_SUCCESS;
}

mt_s32 m2m_ts_set_tee(m2m_ts_ctx_t *ctx, const m2m_tee_info_t *tee)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&ctx->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&ctx->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_ts_process(m2m_ts_ctx_t *ctx, mt_u32 pid_num, const mt_u16 pid[],
                      mt_u8 scr_mode, mt_u8 *dst, const mt_u8 *src,
                      mt_u32 len)
{
    mt_s32 ret;

    ret = m2m_ts_start_operation(ctx, pid_num, pid, scr_mode, dst, src, len);
    if (ret < 0)
        return ret;

    return m2m_ts_wait_complete(ctx);
}

mt_s32 m2m_ts_start_operation(m2m_ts_ctx_t *ctx, mt_u32 pid_num,
                              const mt_u16 pid[], mt_u8 scr_mode, mt_u8 *dst,
                              const mt_u8 *src, mt_u32 len)
{
    m2m_cmd0_t cmd0;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!ctx || !dst || !src || (pid_num && !pid))
        return HW_CE_ERROR_BAD_PARAMETERS;

    ctx->tid = m2m_alloc_cmd_tid(ctx->channel);

    memset(&cmd0, 0, sizeof(m2m_cmd0_t));
    cmd0.Cmd = HW_M2M_CMD0;
    if ((ctx->even_key_slot == HW_M2M_INVALID_KEY_SLOT)
            && (ctx->odd_key_slot == HW_M2M_INVALID_KEY_SLOT))
    {
        cmd0.KeyEn = HW_M2M_USE_DKEY;
        buf_to_le_reg(cmd0.DK, ctx->dk, sizeof(cmd0.DK));
        cmd0.I = 1;
        res = m2m_write_iv(ctx->channel, ctx->div, sizeof(ctx->div));
        if (res != HW_CE_SUCCESS)
            return res;
    }
    else
    {
        cmd0.KeyEn = HW_M2M_USE_KEYTABLE;
        cmd0.KeyIndx = ctx->even_key_slot;
        cmd0.OddKeyIndx = ctx->odd_key_slot;
        cmd0.I = 0;
    }

    cmd0.T = 1;
    cmd0.F = 1;
    cmd0.Q = 1;
    if (!pid_num)
    {
        cmd0.P = 0;
    }
    else
    {
        mt_u32 i;

        cmd0.P = 1;
        for (i = 0; (i < pid_num) && (i < 4); i++)
        {
            cmd0.TS_PIDn[i] = pid[i];
        }
        for (; i < 4; i++)
        {
            cmd0.TS_PIDn[i] = pid[0];
        }
    }
    cmd0.Fscb = scr_mode;
    cmd0.Profile = ctx->profile;
    cmd0.TID = ctx->tid;
    cmd0.SrcDataAddr = HW_M2M_ADDR(src);
    cmd0.DstDataAddr = HW_M2M_ADDR(dst);
    cmd0.ProtectedDataSize = len / TS_PACKET_MAX_LEN;
    cmd0.WCID = ctx->tee.wcid;
    cmd0.SCID = ctx->tee.scid;
    cmd0.SC = ctx->tee.sc;

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((mt_u32)src, len);
    if (dst != src)
        flush_dcache_range((mt_u32)dst, len);
#endif
    return m2m_send_cmd(ctx->channel, &cmd0, sizeof(cmd0));
}

mt_s32 m2m_ts_wait_complete(m2m_ts_ctx_t *ctx)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    return m2m_wait_cmd_finish(ctx->channel, ctx->tid);
}
