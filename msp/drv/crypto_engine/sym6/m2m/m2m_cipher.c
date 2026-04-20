/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_cipher.h"
#include "hw_m2m_register.h"

static bool m2m_profile_is_ctr(mt_u32 profile)
{
    mt_u32 mode;

    mode = HW_M2M_CIPHER_MODE(profile);
    if ((mode == HW_M2M_CIPHER_MODE_CTR)
            || (mode == HW_M2M_CIPHER_MODE_CTR64))
        return true;

    return false;
}

static bool m2m_profile_is_censcbcs(mt_u32 profile)
{
    mt_u32 mode;

    mode = HW_M2M_CIPHER_MODE(profile);
    if ((mode == HW_M2M_CIPHER_MODE_CBCS)
        || (mode == HW_M2M_CIPHER_MODE_CENS))
        return true;

    return false;
}

mt_s32 m2m_cmd0_cipher_operation(mt_u32 profile, mt_u32 channel,
                                 mt_u8 key_slot, const mt_u8 *direct_key, const mt_u8 *direct_iv,
                                 mt_u8 *dst, const mt_u8 *src,
                                 mt_u32 clear_data_size, mt_u32 prot_data_size, const m2m_tee_info_t *tee)
{
    m2m_cipher_ctx_t ctx;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    res = m2m_cmd0_cipher_init(&ctx, profile, channel, key_slot, direct_key, direct_iv);
    if (res != HW_CE_SUCCESS)
        return res;

    res = m2m_cmd0_cipher_set_tee(&ctx, tee);
    if (res != HW_CE_SUCCESS)
        return res;

    if (!prot_data_size) {
        return res;
    }

    res = m2m_cmd0_cipher_start_operation(&ctx, dst, src, clear_data_size, prot_data_size);
    if (res != HW_CE_SUCCESS)
        return res;

    return m2m_cmd0_cipher_wait_complete(&ctx);
}

mt_s32 m2m_cmd0_cipher_init(m2m_cipher_ctx_t *ctx, mt_u32 profile,
                            mt_u32 channel, mt_u8 key_slot,
                            const mt_u8 *direct_key, const mt_u8 *direct_iv)
{
    mt_u32 key_size, iv_size;
    mt_u32 algo;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memset(ctx, 0, sizeof(m2m_cipher_ctx_t));

    algo = HW_M2M_CIPHER_ALGO(profile);
    switch (algo) {
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
    ctx->key_slot = key_slot;
    if (ctx->key_slot == HW_M2M_INVALID_KEY_SLOT) {
        if (!direct_key)
            return HW_CE_ERROR_GENERIC;

        /*
         * 8 byte key stored in high 64 bit memory while 8 byte iv in low 64 bit.
         *
         * The endian order of ctx->dk and ctx->div is big-endian.
         */
        if (direct_key)
            memcpy(ctx->dk, direct_key, key_size);
        if (direct_iv)
            memcpy(ctx->div + sizeof(ctx->div) - iv_size, direct_iv, iv_size);
    }

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_cipher_set_tee(m2m_cipher_ctx_t *ctx,
                               const m2m_tee_info_t *tee)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&ctx->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&ctx->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_cipher_start_operation(m2m_cipher_ctx_t *ctx,
                                       mt_u8 *dst, const mt_u8 *src,
                                       mt_u32 clear_data_size, mt_u32 prot_data_size)
{
    m2m_cmd0_t cmd0;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!clear_data_size && !prot_data_size)
        return HW_CE_SUCCESS;

    ctx->tid = m2m_alloc_cmd_tid(ctx->channel);

    memset(&cmd0, 0, sizeof(m2m_cmd0_t));
    cmd0.Cmd = HW_M2M_CMD0;
    if (ctx->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        cmd0.KeyEn = HW_M2M_USE_DKEY;
        byteswap(cmd0.DK, ctx->dk, sizeof(cmd0.DK));
        cmd0.I = 1;
        res = m2m_write_iv(ctx->channel, ctx->div, sizeof(ctx->div));
        if (res != HW_CE_SUCCESS)
            return res;
    }
    else
    {
        cmd0.KeyEn = HW_M2M_USE_KEYTABLE;
        cmd0.KeyIndx = ctx->key_slot;
        cmd0.I = 0;
    }

    cmd0.F = 1;
    cmd0.Q = 1;
    cmd0.Profile = ctx->profile;
    cmd0.TID = ctx->tid;
    cmd0.SrcDataAddr = HW_M2M_ADDR((ulong)src);
    cmd0.DstDataAddr = HW_M2M_ADDR((ulong)dst);
    cmd0.ClearDataSize = clear_data_size;
    cmd0.ProtectedDataSize = prot_data_size;
    cmd0.WCID = ctx->tee.wcid;
    cmd0.SCID = ctx->tee.scid;
    cmd0.SC = ctx->tee.sc;
    if (((ulong)src >= AHB_BSRAM_ADDR)
        && ((ulong)src < AHB_BSRAM_ADDR + AHB_BSRAM_SIZE)) {
        cmd0.AT = 1;
    }

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((ulong)src, clear_data_size + prot_data_size);
    if (dst != src)
        flush_dcache_range((ulong)dst, clear_data_size + prot_data_size);
#endif
    return m2m_send_cmd(ctx->channel, &cmd0, sizeof(cmd0));
}

mt_s32 m2m_cmd0_cipher_wait_complete(m2m_cipher_ctx_t *ctx)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    return m2m_wait_cmd_finish(ctx->channel, ctx->tid);
}

mt_s32 m2m_cmd1_cipher_set_tee(m2m_cipher_req_t *req,
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

mt_s32 m2m_cmd1_cipher_set_cbcscens(m2m_cipher_req_t *req,
                                mt_u32 profile, mt_u32 skip_block, mt_u32 crypt_block)
{
	if (m2m_profile_is_censcbcs(profile)) {
		if (!req || (skip_block > 15) || (crypt_block > 15))
			return HW_CE_ERROR_BAD_PARAMETERS;

		req->skipb = skip_block;
		req->cryptb = crypt_block;
		req->lcryptb = 0;
		req->lskipb = 0;
	}

	return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_cipher_init(m2m_cipher_req_t *req, mt_u32 profile,
                            mt_u32 channel, mt_u8 key_slot,
                            const mt_u8 *direct_key, mt_u32 direct_key_size,
                            const mt_u8 *direct_iv, mt_u32 direct_iv_size)
{
    mt_u32 key_size = 0;
    mt_u32 iv_size = 0;
    mt_u32 algo;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memset(req, 0, sizeof(m2m_cipher_req_t));

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
        break;
    case HW_M2M_CIPHER_ALGO_AES256:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 16u);
        break;
    case HW_M2M_CIPHER_ALGO_SM4:
        key_size = MIN(sizeof(req->dk), 16u);
        iv_size = MIN(sizeof(req->div), 16u);
        break;
    case HW_M2M_CIPHER_ALGO_GOST28147:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 8u);
        break;
    case HW_M2M_CIPHER_ALGO_GOSTR3412K:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 16u);
        break;
    case HW_M2M_CIPHER_ALGO_GOSTR3412M:
        key_size = MIN(sizeof(req->dk), 32u);
        iv_size = MIN(sizeof(req->div), 8u);
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    req->profile = profile;
    req->channel = channel;

    if (direct_key)
        req->key_slot = HW_M2M_INVALID_KEY_SLOT;
    else
        req->key_slot = key_slot;

    if (req->key_slot == HW_M2M_INVALID_KEY_SLOT) {
        if ((direct_key_size > key_size) || (direct_iv_size > iv_size))
            return HW_CE_ERROR_BAD_PARAMETERS;

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

    return m2m_desc_list_alloc(&req->desc_list);
}

mt_s32 m2m_cmd1_cipher_request(m2m_cipher_req_t *req, phys_addr_t dst,
                               const phys_addr_t src, mt_u32 clear_data_size,
                               mt_u8 los, mt_u32 prot_data_size)
{
    m2m_desc_t *desc = NULL;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!clear_data_size && !prot_data_size)
        return HW_CE_SUCCESS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = req->desc_list->ops->alloc_desc(req->desc_list->ctx, &desc);
    if (res != HW_CE_SUCCESS)
        return res;

    desc->DesMagNum = M2M_DES_MAGIC_NUM;
    desc->SrcDataAddr = HW_M2M_ADDR((ulong)src);
    desc->DstDataAddr = HW_M2M_ADDR((ulong)dst);
    desc->AT_DAT = m2m_get_mapping_bus(desc->SrcDataAddr, 1);
    if (m2m_profile_is_ctr(req->profile))
    {
        if (los != 0)
        {
            desc->LS = 1;
            desc->LOS = los;
        }
    }
    else
    {
        desc->LS = 1;
        desc->LOS = los;
    }
    desc->ClearDataSize = clear_data_size;
    desc->ProtectedDataSize = prot_data_size;
    /*desc->Q = 1; */
    desc->F = 0;
    desc->I = 0;

    if (m2m_profile_is_censcbcs(req->profile)) {
        desc->LOS = los;
        desc->CryptoB = req->cryptb;
        desc->SkipB = req->skipb;
        desc->LCryptoB = req->lcryptb;
    }

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((mt_u64)src, clear_data_size + prot_data_size);
    if (src != dst)
        flush_dcache_range((mt_u64)dst, clear_data_size + prot_data_size);
#endif

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_cipher_start_operation(m2m_cipher_req_t *req)
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
    if (req->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        cmd1.KeyEn = HW_M2M_USE_DKEY;
        byteswap(cmd1.DK, req->dk, sizeof(cmd1.DK));
    }
    else
    {
        cmd1.KeyEn = HW_M2M_USE_KEYTABLE;
        cmd1.KeyIndx = req->key_slot;
    }
    cmd1.Profile = req->profile;
    cmd1.TID = req->tid;
    cmd1.WCID = req->tee.wcid;
    cmd1.SCID = req->tee.scid;
    cmd1.SC = req->tee.sc;

    if (head_desc->LS == 0)
    {
        head_desc->LS = 1;
        head_desc->LOS = 0;
    }

	head_desc->BS = 1;
	head_desc->S = 1;
	tail_desc->E = 1;

    head_desc->F = 1;
    if (req->use_div)
    {
        head_desc->I = 1;
        byteswap(head_desc->ECB_CBC_CTR_DIV, req->div,
                 sizeof(head_desc->ECB_CBC_CTR_DIV));
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

mt_s32 m2m_cmd1_cipher_wait_complete(m2m_cipher_req_t *req)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (!req->desc_list->ops->get_count(req->desc_list->ctx))
        return HW_CE_SUCCESS;

    res = m2m_wait_cmd_finish(req->channel, req->tid);
    if (res != HW_CE_SUCCESS) {
		m2m_print_debug(req->channel);
        return res;
    }

    req->desc_list->ops->clean(req->desc_list->ctx);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_cipher_update(m2m_cipher_req_t *req, phys_addr_t dst,
                              const phys_addr_t src, mt_u32 clear_data_size,
                              mt_u8 los, mt_u32 prot_data_size)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (los == 0)
        los = req->los;

    res = m2m_cmd1_cipher_request(req, dst, src,
                                  clear_data_size, los, prot_data_size);
    if (res != HW_CE_SUCCESS)
        return res;

    res = m2m_cmd1_cipher_start_operation(req);
    if (res != HW_CE_SUCCESS)
        return res;

    res = m2m_cmd1_cipher_wait_complete(req);
    if (res != HW_CE_SUCCESS)
        return res;

    req->use_div = 1;
    res = m2m_read_final_iv(req->channel, req->tid, req->div, sizeof(req->div));
    if (res != HW_CE_SUCCESS)
        return res;

    if (m2m_profile_is_ctr(req->profile))
    {
        mt_u32 algo;
        mt_u32 block_size;
        mt_u32 residue;

        algo = HW_M2M_CIPHER_ALGO(req->profile);

        switch (algo) {
        case HW_M2M_CIPHER_ALGO_DES:
        case HW_M2M_CIPHER_ALGO_TDES:
        case HW_M2M_CIPHER_ALGO_GOST28147:
        case HW_M2M_CIPHER_ALGO_GOSTR3412M:
            block_size = 8;
            break;
        case HW_M2M_CIPHER_ALGO_AES128:
        case HW_M2M_CIPHER_ALGO_AES192:
        case HW_M2M_CIPHER_ALGO_AES256:
        case HW_M2M_CIPHER_ALGO_SM4:
        case HW_M2M_CIPHER_ALGO_GOSTR3412K:
            block_size = 16;
            break;
        default:
            return HW_CE_ERROR_GENERIC;
            break;
        }

        residue = (prot_data_size - los) & (block_size - 1);
        if (residue > 0)
            req->los = block_size - residue;
        else
            req->los = 0;
    }

    return HW_CE_SUCCESS;
}

void m2m_cmd1_cipher_deinit(m2m_cipher_req_t *req)
{
    if (!req)
        return;

    if (req->desc_list)
    {
        m2m_desc_list_free(req->desc_list);
        req->desc_list = NULL;
    }
}
