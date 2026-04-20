/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_ts.h"
#include "hw_m2m_register.h"

#define TS_PACKET_MAX_LEN (188)

mt_s32 m2m_cmd0_ts_init(m2m_ts_ctx_t *ctx, mt_u32 profile, mt_u32 channel,
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
    case HW_M2M_CIPHER_ALGO_AES192:
        key_size = 24;
        iv_size = 16;
        break;
    case HW_M2M_CIPHER_ALGO_AES256:
        key_size = 32;
        iv_size = 16;
        break;
    case HW_M2M_CIPHER_ALGO_SM4:
        key_size = 16;
        iv_size = 16;
        break;
    case HW_M2M_CIPHER_ALGO_GOST28147:
        key_size = 32;
        iv_size = 8;
        break;
    case HW_M2M_CIPHER_ALGO_GOSTR3412K:
        key_size = 32;
        iv_size = 16;
        break;
    case HW_M2M_CIPHER_ALGO_GOSTR3412M:
        key_size = 32;
        iv_size = 8;
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

mt_s32 m2m_cmd0_ts_start_operation(m2m_ts_ctx_t *ctx, mt_u32 pid_num,
                              const mt_u16 pid[], mt_u8 scr_mode, phys_addr_t dst,
                              const phys_addr_t src, mt_u32 len)
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

    cmd0.S = 1;
    cmd0.E = 1;
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
    cmd0.AT = m2m_get_mapping_bus(cmd0.SrcDataAddr, 1);

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((mt_u64)src, len);
    if (dst != src)
        flush_dcache_range((mt_u64)dst, len);
#endif
    return m2m_send_cmd(ctx->channel, &cmd0, sizeof(cmd0));
}

mt_s32 m2m_cmd0_ts_wait_complete(m2m_ts_ctx_t *ctx)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    return m2m_wait_cmd_finish(ctx->channel, ctx->tid);
}

mt_s32 m2m_cmd0_ts_process(m2m_ts_ctx_t *ctx, mt_u32 pid_num, const mt_u16 pid[],
                      mt_u8 scr_mode, phys_addr_t dst, const phys_addr_t src,
                      mt_u32 len)
{
    mt_s32 ret;

    ret = m2m_cmd0_ts_start_operation(ctx, pid_num, pid, scr_mode, dst, src, len);
    if (ret < 0)
        return ret;

    return m2m_cmd0_ts_wait_complete(ctx);
}

mt_s32 m2m_cmd1_ts_init(m2m_ts_req_t *req, mt_u32 profile,
        mt_u32 channel, mt_u8 scr_mode,
        mt_u8 even_key_slot, mt_u8 odd_key_slot,
        const mt_u8 *direct_key, mt_u32 direct_key_size,
        const mt_u8 *direct_iv, mt_u32 direct_iv_size)
{
    mt_u32 key_size = 0;
    mt_u32 iv_size = 0;
    bool use_dk = false;
    mt_u32 algo;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (direct_key)
        use_dk = true;

    memset(req, 0, sizeof(m2m_ts_req_t));

    algo = HW_M2M_CIPHER_ALGO(profile);
    switch (algo) {
    case HW_M2M_CIPHER_ALGO_DES:
        key_size = MIN(sizeof(req->dk), 8u);
        iv_size = MIN(sizeof(req->div), 8u);
        break;
    case HW_M2M_CIPHER_ALGO_TDES:
        key_size = MIN(sizeof(req->dk), 16u);
        iv_size = MIN(sizeof(req->div), 8u);
        break;
    case HW_M2M_CIPHER_ALGO_AES128:
        key_size = MIN(sizeof(req->dk), 16u);
        iv_size = MIN(sizeof(req->div), 16u);
        break;
    case HW_M2M_CIPHER_ALGO_AES192:
        key_size = MIN(sizeof(req->dk), 24u);
        iv_size = MIN(sizeof(req->div), 16u);
        use_dk = true;
        break;
    case HW_M2M_CIPHER_ALGO_AES256:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 16u);
        use_dk = true;
        break;
    case HW_M2M_CIPHER_ALGO_SM4:
        key_size = MIN(sizeof(req->dk), 16u);
        iv_size = MIN(sizeof(req->div), 16u);
        use_dk = true;
        break;
    case HW_M2M_CIPHER_ALGO_GOST28147:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 8u);
        use_dk = true;
        break;
    case HW_M2M_CIPHER_ALGO_GOSTR3412K:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 16u);
        use_dk = true;
        break;
    case HW_M2M_CIPHER_ALGO_GOSTR3412M:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 8u);
        use_dk = true;
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    req->profile = profile;
    req->channel = channel;
    req->scr_mode = scr_mode;

    if (use_dk) {
        req->even_key_slot = HW_M2M_INVALID_KEY_SLOT;
        req->odd_key_slot = HW_M2M_INVALID_KEY_SLOT;
    } else {
        req->even_key_slot = even_key_slot;
        req->odd_key_slot = odd_key_slot;
    }

    if ((req->even_key_slot == HW_M2M_INVALID_KEY_SLOT)
            && (req->odd_key_slot == HW_M2M_INVALID_KEY_SLOT)) {
        if ((direct_key_size < key_size) || (direct_iv_size < iv_size))
            return HW_CE_ERROR_GENERIC;

        /*
         * 8 byte key stored in high 64 bit memory while 8 byte iv in low 64 bit.
         *
         * The endian order of req->dk and req->div is big-endian.
         */
        if (direct_key)
            memcpy(req->dk, direct_key, direct_key_size);
        if (direct_iv)
            memcpy(req->div + sizeof(req->div) - direct_iv_size, direct_iv, direct_iv_size);

        req->use_div = 1;
    }

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_ts_set_tee(m2m_ts_req_t *req,
        const m2m_tee_info_t *tee)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&req->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&req->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_ts_request(m2m_ts_req_t *req, mt_u32 pid_num,
        const mt_u16 pid[], mt_u8 *dst, const mt_u8 *src, mt_u32 len)
{
    m2m_desc_t *desc;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req || !dst || !src)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = req->desc_list->ops->alloc_desc(req->desc_list->ctx, &desc);
    if (res != HW_CE_SUCCESS)
        return res;

    memset(desc, 0, sizeof(m2m_desc_t));
    desc->SrcDataAddr = HW_M2M_ADDR((ulong)src);
    desc->DstDataAddr = HW_M2M_ADDR((ulong)dst);
    desc->DesMagNum = M2M_DES_MAGIC_NUM;
    desc->ProtectedDataSize = len / TS_PACKET_MAX_LEN;
    desc->AT_DAT = m2m_get_mapping_bus(desc->SrcDataAddr, 1);
    /*desc->Q = 1; */
    desc->F = 0;
    desc->I = 0;
    if (pid_num) {
        mt_u32 i;

        for (i = 0; (i < pid_num) && (i < 4); i++) {
            desc->TS_PIDn[i] = pid[i];
        }
        for (; i < 4; i++) {
            desc->TS_PIDn[i] = pid[0];
        }
    }

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((ulong)src, len);
    if (src != dst)
        flush_dcache_range((ulong)dst, len);
#endif

    req->req_cnt++;

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_ts_start_operation(m2m_ts_req_t *req)
{
    m2m_cmd1_t cmd1;
    m2m_desc_t *head_desc = NULL;
    m2m_desc_t *tail_desc = NULL;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (!req->desc_list->ops->get_count(req->desc_list->ctx))
        return HW_CE_SUCCESS;

    res = req->desc_list->ops->first_desc(req->desc_list->ctx, &head_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->desc_list->ops->last_desc(req->desc_list->ctx, &tail_desc);
    if (res != HW_CE_SUCCESS)
        return res;

    req->tid = m2m_alloc_cmd_tid(req->channel);

    memset(&cmd1, 0, sizeof(m2m_cmd1_t));
    cmd1.Cmd = HW_M2M_CMD1;
    cmd1.Q = 1;
    if ((req->even_key_slot == HW_M2M_INVALID_KEY_SLOT)
            && (req->odd_key_slot == HW_M2M_INVALID_KEY_SLOT)) {
        cmd1.KeyEn = HW_M2M_USE_DKEY;
        byteswap(cmd1.DK, req->dk, sizeof(cmd1.DK));
    } else {
        cmd1.KeyEn = HW_M2M_USE_KEYTABLE;
        cmd1.KeyIndx = req->even_key_slot;
        cmd1.OddKeyIndx = req->odd_key_slot;
    }

    cmd1.T = 1;
    if (!head_desc->TS_PID0) {
        cmd1.P = 0;
    } else {
        cmd1.P = 1;
    }
    cmd1.Fscb = req->scr_mode;
    cmd1.Profile = req->profile;
    cmd1.TID = req->tid;
    cmd1.WCID = req->tee.wcid;
    cmd1.SCID = req->tee.scid;
    cmd1.SC = req->tee.sc;

    if (head_desc->LS == 0) {
        head_desc->LS = 1;
        head_desc->LOS = 0;
    }

    head_desc->F = 1;
    if (req->use_div) {
        head_desc->I = 1;
        byteswap(head_desc->ECB_CBC_CTR_DIV, req->div, sizeof(head_desc->ECB_CBC_CTR_DIV));
    }
    tail_desc->T = 1;

    cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_desc);
    cmd1.AT = m2m_get_mapping_bus(cmd1.DescStartAddr, sizeof(m2m_desc_t));

    req->desc_list->ops->flush(req->desc_list->ctx);

    if (HW_M2M_CIPHER_ALGO(req->profile) == HW_M2M_CIPHER_ALGO_GOST28147) {
        m2m_set_gost_param(req->channel, 0, 0);
    } else if (HW_M2M_CIPHER_ALGO(req->profile) == HW_M2M_CIPHER_ALGO_GOSTR3412M) {
        m2m_set_gost_param(req->channel, 0, 5);
    } else {}
    return m2m_send_cmd(req->channel, &cmd1, sizeof(m2m_cmd1_t));
}

mt_s32 m2m_cmd1_ts_wait_complete(m2m_ts_req_t *req)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = m2m_wait_cmd_finish(req->channel, req->tid);
    if (res != HW_CE_SUCCESS)
        return res;

    req->desc_list->ops->clean(req->desc_list->ctx);

    return HW_CE_SUCCESS;
}
