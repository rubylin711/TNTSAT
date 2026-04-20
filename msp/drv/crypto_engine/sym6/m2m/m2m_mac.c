/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "hw_m2m_register.h"
#include "m2m_mac.h"
#include "mt_drv_mmz.h"

enum M2M_MAC_STATE
{
    M2M_MAC_STATE_INIT = 0,
    M2M_MAC_STATE_RUNNING,
    M2M_MAC_STATE_PAUSE,
    M2M_MAC_STATE_TERMINATED,
};

static inline int m2m_mac_is_hmac(mt_u32 profile)
{
    switch (profile)
    {
    case HMAC_SHA2_224:
    case HMAC_SHA2_256:
    case HMAC_SHA2_384:
    case HMAC_SHA2_512:
    case HMAC_SM3:
        return 1;
    default:
        return 0;
    }

    return 0;
}

static inline mt_u32 m2m_mac_digest_size(mt_u32 profile)
{
    mt_u32 size = 0;

    switch (profile)
    {
    case HMAC_SHA2_224:
        size = 28;
        break;
    case HMAC_SHA2_256:
    case HMAC_SM3:
        size = 32;
        break;
    case HMAC_SHA2_384:
        size = 48;
        break;
    case HMAC_SHA2_512:
        size = 64;
        break;
    case CMAC_AES128:
    case CMAC_AES192:
    case CMAC_AES256:
    case CBCMAC_AES128:
    case CBCMAC_SM4:
    case CMAC_SM4:
        size = 16;
        break;
    case CMAC_TDES:
    case CBCMAC_TDES:
        size = 8;
        break;
    default:
        break;
    }

    return size;
}

static inline mt_u32 m2m_mac_block_size(mt_u32 profile)
{
    mt_u32 size = 0;

    switch (profile)
    {
    case HMAC_SHA2_224:
    case HMAC_SHA2_256:
    case HMAC_SM3:
        size = 64;
        break;
    case HMAC_SHA2_384:
    case HMAC_SHA2_512:
        size = 128;
        break;
    case CMAC_AES128:
    case CMAC_AES192:
    case CMAC_AES256:
    case CMAC_TDES:
    case CBCMAC_AES128:
    case CBCMAC_SM4:
    case CBCMAC_TDES:
    case CMAC_SM4:
        size = 16;
        break;
    default:
        break;
    }

    return size;
}

static mt_s32 m2m_cmd0_mac_operation(m2m_mac_ctx_t *ctx, const phys_addr_t data,
                                     mt_u32 len, bool terminate)
{
    m2m_cmd0_t cmd0;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (ctx->state == M2M_MAC_STATE_TERMINATED)
    {
        if (!data && !len)
            return HW_CE_SUCCESS;

        return HW_CE_ERROR_BAD_STATE;
    }

    /* Multi-segment is not supported except for HMAC. */
    if (!terminate && !m2m_mac_is_hmac(ctx->profile))
        terminate = true;

    /* Unaligned block must be the last block. */
    if (!terminate)
    {
        mt_u32 block_size = m2m_mac_block_size(ctx->profile);

        if (len & (block_size - 1))
            terminate = true;
    }

    ctx->tid = m2m_alloc_cmd_tid(ctx->channel);

    memset(&cmd0, 0, sizeof(m2m_cmd0_t));
    cmd0.Cmd = HW_M2M_CMD0;
    cmd0.Q = 1;
    if (ctx->key_slot == HW_M2M_INVALID_KEY_SLOT) {
        cmd0.KeyEn = HW_M2M_USE_DKEY;
        byteswap(cmd0.DK, ctx->dk, sizeof(cmd0.DK));
    } else {
        if (ctx->use_mac_key == 1) {
            cmd0.KeyEn = HW_M2M_USE_KEYTABLE;
        } else {
            cmd0.KeyEn = HW_M2M_USE_KEYTABLE_256BIT;
        }
        cmd0.KeyIndx = ctx->key_slot;
    }

    if (ctx->state == M2M_MAC_STATE_PAUSE)
    {
        cmd0.HF = 1;
        res = m2m_write_hash(ctx->channel, ctx->context, HW_M2M_MAX_MAC_DIGEST_SIZE);
        if (res != HW_CE_SUCCESS)
            return res;

        ctx->state = M2M_MAC_STATE_RUNNING;
    }
    else if (ctx->state == M2M_MAC_STATE_INIT)
    {
        cmd0.S = 1;
        ctx->state = M2M_MAC_STATE_RUNNING;
    }

    if (terminate)
    {
        cmd0.E = 1;
        ctx->state = M2M_MAC_STATE_TERMINATED;
    }
    else
    {
        cmd0.E = 0;
    }

    cmd0.Profile = ctx->profile;
    cmd0.TID = ctx->tid;
    cmd0.SrcDataAddr = HW_M2M_ADDR(data);
    cmd0.ProtectedDataSize = len;
    cmd0.WCID = ctx->tee.wcid;
    cmd0.SCID = ctx->tee.scid;
    cmd0.SC = ctx->tee.sc;
    cmd0.AT = m2m_get_mapping_bus(cmd0.SrcDataAddr, 1);
    if (cmd0.S != 1)
        cmd0.HashDataSum = ctx->len_processed;

#ifdef M2M_USE_VIRT_ADDR
    if (data && len)
        flush_dcache_range((mt_u64)data, len);
#endif

    res = m2m_send_cmd(ctx->channel, &cmd0, sizeof(m2m_cmd0_t));
    if (res != HW_CE_SUCCESS)
        return res;

    ctx->len_processed += len;
    return m2m_wait_cmd_finish(ctx->channel, ctx->tid);
}

mt_s32 m2m_cmd0_mac_init(m2m_mac_ctx_t *ctx, mt_u32 profile, mt_u32 channel,
                         mt_u8 key_slot, const mt_u8 *direct_key,
                         mt_u32 direct_key_size)
{
    mt_u32 key_size = 0;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memset(ctx, 0, sizeof(m2m_mac_ctx_t));

    switch (profile)
    {
    case HMAC_SHA2_224:
    case HMAC_SHA2_256:
    case HMAC_SM3:
        key_size = MIN(sizeof(ctx->dk), 64u);
        break;
    case HMAC_SHA2_384:
    case HMAC_SHA2_512:
        key_size = MIN(sizeof(ctx->dk), 128u);
        break;
    case CMAC_AES128:
    case CMAC_TDES:
    case CBCMAC_AES128:
    case CBCMAC_SM4:
    case CBCMAC_TDES:
        key_size = MIN(sizeof(ctx->dk), 16u);
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    ctx->profile = profile;
    ctx->channel = channel;
    ctx->state = M2M_MAC_STATE_INIT;
    ctx->block_size = 0;
    ctx->key_slot = key_slot;
    if (ctx->key_slot == HW_M2M_INVALID_KEY_SLOT) {
        if (direct_key_size > key_size)
            return HW_CE_ERROR_BAD_PARAMETERS;

        if (!direct_key)
            return HW_CE_ERROR_BAD_PARAMETERS;

        memcpy(ctx->dk, direct_key, direct_key_size);
    } else {
        if (direct_key_size <= HW_M2M_MAX_CMD0_DK_SIZE)
            ctx->use_mac_key = 1;
    }

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_mac_set_tee(m2m_mac_ctx_t *ctx, const m2m_tee_info_t *tee)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&ctx->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&ctx->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_mac_update(m2m_mac_ctx_t *ctx, const phys_addr_t data, mt_u32 len)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;
#ifdef M2M_CMD0_MAC_COPY_BUFFER
    mt_u32 max_block_size = 0;
    mt_u32 free_size = 0;
    mt_u32 n_blocks = 0;
    mt_u32 residue = 0;
    mt_u8 *virt_addr;
#endif

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!data || !len)
        return HW_CE_SUCCESS;

#ifndef M2M_CMD0_MAC_COPY_BUFFER
    res = m2m_cmd0_mac_operation(ctx, data, len, false);
    if (res != HW_CE_SUCCESS)
        return res;
#else

    max_block_size = m2m_mac_block_size(ctx->profile);
    virt_addr = mt_remap_mmz_k((phys_addr_t)data, len, 0);

    if (!m2m_mac_is_hmac(ctx->profile))
        return m2m_cmd0_mac_operation(ctx, data, len, true);

    free_size = max_block_size - ctx->block_size;
    if ((ctx->block_size > 0) && (len > free_size)) {
        memcpy(ctx->block + ctx->block_size, virt_addr, free_size);
        len -= free_size;
        virt_addr += free_size;

        flush_dcache_range((mt_u64)ctx->block, max_block_size);

        res = m2m_cmd0_mac_operation(ctx, (phys_addr_t)HW_M2M_PHYS_ADDR(ctx->block), max_block_size, false);
        if (res != HW_CE_SUCCESS)
            return res;
        ctx->block_size = 0;
    } else {
        free_size = 0;
    }

    n_blocks = len / max_block_size;
    residue = len % max_block_size;

    if (n_blocks > 0)
    {
        res = m2m_cmd0_mac_operation(ctx, data + free_size, n_blocks * max_block_size, false);
        if (res != HW_CE_SUCCESS)
            return res;

        virt_addr += n_blocks * max_block_size;
    }

    if (residue > 0)
    {
        memcpy(ctx->block + ctx->block_size, virt_addr, residue);
        ctx->block_size += residue;
    }
#endif

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_mac_save_context(m2m_mac_ctx_t *ctx, mt_u8 *context)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!m2m_mac_is_hmac(ctx->profile))
        return HW_CE_ERROR_NOT_SUPPORTED;

    if (ctx->state == M2M_MAC_STATE_RUNNING)
    {
        res = m2m_read_final_hash(ctx->channel, ctx->tid, ctx->context, HW_M2M_MAX_MAC_DIGEST_SIZE);
        if (res != HW_CE_SUCCESS)
            return res;

        if (context)
            memcpy(context, ctx->context, HW_M2M_MAX_MAC_DIGEST_SIZE);

        ctx->state = M2M_MAC_STATE_PAUSE;
    }
    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_mac_restore_context(m2m_mac_ctx_t *ctx, const mt_u8 *context)
{
    if (!ctx || !context)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!m2m_mac_is_hmac(ctx->profile))
        return HW_CE_ERROR_NOT_SUPPORTED;

    if (ctx->block_size > 0)
        return HW_CE_ERROR_BAD_STATE;

    memcpy(ctx->context, context, HW_M2M_MAX_MAC_DIGEST_SIZE);
    ctx->state = M2M_MAC_STATE_PAUSE;

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_mac_final(m2m_mac_ctx_t *ctx, mt_u8 *out)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!ctx || !out)
        return HW_CE_ERROR_BAD_PARAMETERS;

#ifndef M2M_CMD0_MAC_COPY_BUFFER
    res = m2m_cmd0_mac_operation(ctx, 0, 0, true);
#else
    res = m2m_cmd0_mac_operation(ctx, ctx->block, ctx->block_size, true);
#endif
    if (res != HW_CE_SUCCESS)
        return res;

    return m2m_read_final_hash(ctx->channel, ctx->tid, out,
                               m2m_mac_digest_size(ctx->profile));
}

static mt_s32 m2m_cmd1_mac_request_internal(m2m_mac_req_t *req,
        const phys_addr_t data, mt_u32 len)
{
    m2m_desc_t *desc;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = req->desc_list->ops->alloc_desc(req->desc_list->ctx, &desc);
    if (res != HW_CE_SUCCESS)
        return res;

    desc->DesMagNum = M2M_DES_MAGIC_NUM;
    desc->SrcDataAddr = HW_M2M_ADDR(data);
    desc->AT_DAT = m2m_get_mapping_bus(desc->SrcDataAddr, 1);
    desc->ProtectedDataSize = len;
    desc->S = 0;
    desc->E = 0;
    desc->Q = 1;

#ifdef M2M_USE_VIRT_ADDR
    if (data && len)
        flush_dcache_range((mt_u64)data, len);
#endif

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_mac_init(m2m_mac_req_t *req, mt_u32 profile, mt_u32 channel,
                         mt_u8 key_slot, const mt_u8 *direct_key, mt_u32 direct_key_size)
{
    mt_u32 key_size = 0;
    mt_u32 use_dk = 0;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memset(req, 0, sizeof(m2m_mac_req_t));

    switch (profile)
    {
    case HMAC_SHA2_224:
    case HMAC_SHA2_256:
    case HMAC_SM3:
        key_size = MIN(sizeof(req->key), 64u);
        break;
    case HMAC_SHA2_384:
    case HMAC_SHA2_512:
        key_size = MIN(sizeof(req->key), 128u);
        break;
    case CBCMAC_AES128:
    case CBCMAC_SM4:
    case CBCMAC_TDES:
    case CMAC_TDES:
    case CMAC_AES128:
    case CMAC_SM4:
        key_size = MIN(sizeof(req->key), 16u);
        break;
    case CMAC_AES192:
        key_size = MIN(sizeof(req->key), 24u);
        use_dk = 1;
        break;
    case CMAC_AES256:
        key_size = MIN(sizeof(req->key), 32u);
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    if (direct_key_size > key_size)
        return HW_CE_ERROR_BAD_PARAMETERS;

    req->profile = profile;
    req->channel = channel;
    req->key_slot = key_slot;
    req->state = M2M_MAC_STATE_INIT;

    if (direct_key_size > HW_M2M_MAX_CMD1_DK_SIZE)
        use_dk = 1;

    if (use_dk)
        req->key_slot = HW_M2M_INVALID_KEY_SLOT;
    else
        req->key_slot = key_slot;

    if (req->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        if (!direct_key)
            return HW_CE_ERROR_BAD_PARAMETERS;

        memcpy(req->key, direct_key, direct_key_size);

        if (direct_key_size > HW_M2M_MAX_CMD1_DK_SIZE)
            req->use_mac_key = 3;
        else
            req->use_mac_key = 2;
    } else {
        if (direct_key_size < HW_M2M_MAX_CMD1_DK_SIZE)
            req->use_mac_key = 1;
    }
    return m2m_desc_list_alloc(&req->desc_list);
}

mt_s32 m2m_cmd1_mac_set_tee(m2m_mac_req_t *req, const m2m_tee_info_t *tee)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&req->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&req->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_mac_request(m2m_mac_req_t *req, const phys_addr_t data, mt_u32 len)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!len)
        return HW_CE_SUCCESS;

    return m2m_cmd1_mac_request_internal(req, data, len);
}

mt_s32 m2m_cmd1_mac_start_operation(m2m_mac_req_t *req)
{
    m2m_cmd1_t cmd1;
    m2m_desc_t *head_desc = NULL;
    m2m_desc_t *tail_desc = NULL;
    m2m_desc_t *specified_desc = NULL;
    size_t desp_index = 0, desc_count = 0;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    desc_count = req->desc_list->ops->get_count(req->desc_list->ctx);
    if (!desc_count)
    {
        res = m2m_cmd1_mac_request_internal(req, 0, 0);
        if (res != HW_CE_SUCCESS)
            return res;
    }

    res = req->desc_list->ops->first_desc(req->desc_list->ctx, &head_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->desc_list->ops->last_desc(req->desc_list->ctx, &tail_desc);
    if (res != HW_CE_SUCCESS)
        return res;

    req->tid = m2m_alloc_cmd_tid(req->channel);

    if (req->state == M2M_MAC_STATE_INIT) {
        head_desc->S = 1;
        head_desc->P = 1;
        head_desc->F = 1;
        head_desc->I = 1;
        head_desc->LS = 1;
        head_desc->E = 0;
        head_desc->T = 0;
        req->state = M2M_MAC_STATE_RUNNING;
    } else if (req->state == M2M_MAC_STATE_TERMINATED) {
        tail_desc->E = 1;
        tail_desc->T = 1;

        if (desc_count == 1) {
            head_desc->S = 1;
            head_desc->F = 1;
            head_desc->I = 1;
            head_desc->LS = 1;
        }
    } else if (req->state == M2M_MAC_STATE_RUNNING) {

    }

    memset(&cmd1, 0, sizeof(m2m_cmd1_t));
    cmd1.Cmd = HW_M2M_CMD1;
    cmd1.Q = 1;
    if (req->key_slot == HW_M2M_INVALID_KEY_SLOT) {
        if (req->use_mac_key == 3) {
            cmd1.KeyEn = HW_M2M_USE_MACKEY;
            for (desp_index = 1; desp_index <= desc_count; desp_index++) {
                res = req->desc_list->ops->specified_desc(req->desc_list->ctx, desp_index, &specified_desc);
                if (res != HW_CE_SUCCESS)
                    return res;
                byteswap(specified_desc->HMAC_mackey, req->key, sizeof(specified_desc->HMAC_mackey));
            }
        } else if (req->use_mac_key == 2) {
            cmd1.KeyEn = HW_M2M_USE_DKEY;
            byteswap(cmd1.DK, req->key, sizeof(cmd1.DK));
        } else {
            EMSG("HMAC key index select error.\n");
            return HW_CE_ERROR_BAD_PARAMETERS;
        }
    } else {
        if (req->use_mac_key == 0) {
            cmd1.KeyEn = HW_M2M_USE_KEYTABLE_256BIT;
        } else if (req->use_mac_key == 1) {
            cmd1.KeyEn = HW_M2M_USE_KEYTABLE;
        } else {
            EMSG("HMAC key index select error.\n");
            return HW_CE_ERROR_BAD_PARAMETERS;
        }
        cmd1.KeyIndx = req->key_slot;
    }
    cmd1.Profile = req->profile;
    cmd1.TID = req->tid;
    cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_desc);
    cmd1.AT = m2m_get_mapping_bus(cmd1.DescStartAddr, sizeof(m2m_desc_t));
    cmd1.WCID = req->tee.wcid;
    cmd1.SCID = req->tee.scid;
    cmd1.SC = req->tee.sc;

    req->desc_list->ops->flush(req->desc_list->ctx);

    return m2m_send_cmd(req->channel, &cmd1, sizeof(m2m_cmd1_t));
}

mt_s32 m2m_cmd1_mac_wait_complete(m2m_mac_req_t *req)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (!req->desc_list->ops->get_count(req->desc_list->ctx))
        return HW_CE_SUCCESS;

    res = m2m_wait_desc_finish(req->channel, req->tid);
    if (res != HW_CE_SUCCESS)
        return res;

    if (req->state == M2M_MAC_STATE_TERMINATED) {
        req->desc_list->ops->clean(req->desc_list->ctx);

        res = m2m_wait_cmd_finish(req->channel, req->tid);
    }

    return res;
}

static mt_s32 m2m_cmd1_mac_operation(m2m_mac_req_t *req, const phys_addr_t data,
                                     mt_u32 len, bool terminate)
{
    size_t desc_count = 0;
    m2m_desc_t *specified_desc = NULL;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (req->state == M2M_MAC_STATE_TERMINATED)
    {
        if (!data && !len)
            return HW_CE_SUCCESS;

        return HW_CE_ERROR_BAD_STATE;
    }

    /* Unaligned block must be the last block. */
    if (!terminate)
    {
        mt_u32 block_size = m2m_mac_block_size(req->profile);

        if (len & (block_size - 1))
            terminate = true;
    }

    res = m2m_cmd1_mac_request_internal(req, data, len);
    if (res != HW_CE_SUCCESS)
        return res;

    if (req->state == M2M_MAC_STATE_INIT) {
        if (terminate)
            req->state = M2M_MAC_STATE_TERMINATED;

        res = m2m_cmd1_mac_start_operation(req);
        if (res != HW_CE_SUCCESS)
            return res;
    } else {
        desc_count = req->desc_list->ops->get_count(req->desc_list->ctx);
        if (desc_count < 2) {
            EMSG("%s,%d: there is no valid descript!\n", __func__, __LINE__);
            return HW_CE_ERROR_BAD_STATE;
        }

        res = req->desc_list->ops->specified_desc(req->desc_list->ctx, desc_count, &specified_desc);
        if (res != HW_CE_SUCCESS) {
            return res;
        } else {
            if (terminate == true) {
                specified_desc->P = 0;
                specified_desc->E = 1;
                specified_desc->T = 1;

                byteswap(specified_desc->HMAC_mackey, req->key, sizeof(specified_desc->HMAC_mackey));
                req->state = M2M_MAC_STATE_TERMINATED;
            } else {
                specified_desc->P = 1;
            }

            flush_dcache_range((ulong)specified_desc, sizeof(m2m_desc_t));
        }

        res = req->desc_list->ops->specified_desc(req->desc_list->ctx, desc_count - 1, &specified_desc);
        if (res != HW_CE_SUCCESS) {
            return res;
        } else {
            flush_dcache_range((ulong)specified_desc, sizeof(m2m_desc_t));
            specified_desc->P = 0;
        }

        flush_dcache_range((ulong)specified_desc, sizeof(m2m_desc_t));
    }

    res = m2m_cmd1_mac_wait_complete(req);
    if (res != HW_CE_SUCCESS) {
        m2m_print_debug(req->channel);
    }

    return res;
}

mt_s32 m2m_cmd1_mac_update(m2m_mac_req_t *req, const phys_addr_t data, mt_u32 len)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;
#ifdef M2M_CMD1_MAC_COPY_BUFFER
    mt_u32 max_block_size = 0;
    mt_u32 free_size = 0;
    mt_u32 n_blocks = 0;
    mt_u32 residue = 0;
    mt_u8 *virt_addr;
#endif

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!data || !len)
        return HW_CE_SUCCESS;

#ifndef M2M_CMD1_MAC_COPY_BUFFER
    res = m2m_cmd1_mac_operation(req, data, len, false);
    if (res != HW_CE_SUCCESS)
        return res;
#else

    if (!m2m_mac_is_hmac(req->profile))
        return m2m_cmd1_mac_operation(req, data, len, true);

    max_block_size = m2m_mac_block_size(req->profile);
    virt_addr = mt_remap_mmz_k((phys_addr_t)data, len, 0);

    free_size = max_block_size - req->block_size;
    if ((req->block_size > 0) && (len > free_size)) {
        memcpy(req->block + req->block_size, virt_addr, free_size);
        len -= free_size;
        virt_addr += free_size;

        flush_dcache_range((ulong)req->block, max_block_size);

        res = m2m_cmd1_mac_operation(req, (phys_addr_t)HW_M2M_PHYS_ADDR(req->block), max_block_size, false);
        if (res != HW_CE_SUCCESS)
            return res;
        req->block_size = 0;
    } else {
        free_size = 0;
    }

    n_blocks = len / max_block_size;
    residue = len % max_block_size;

    if (n_blocks > 0)
    {
        res = m2m_cmd1_mac_operation(req, data + free_size, n_blocks * max_block_size, false);
        if (res != HW_CE_SUCCESS)
            return res;

        virt_addr += n_blocks * max_block_size;
    }

    if (residue > 0)
    {
        memcpy(req->block + req->block_size, virt_addr, residue);
        req->block_size += residue;
    }
#endif

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_mac_digest(m2m_mac_req_t *req, mt_u8 *digest)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (digest)
        return m2m_read_final_hash(req->channel, req->tid, digest, m2m_mac_digest_size(req->profile));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_mac_final(m2m_mac_req_t *req, mt_u8 *out)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

#ifndef M2M_CMD1_MAC_COPY_BUFFER
    res = m2m_cmd1_mac_operation(req, 0, 0, true);
#else
    if (!m2m_mac_is_hmac(req->profile))
        return m2m_cmd1_mac_digest(req, out);

    if (req->block_size)
        flush_dcache_range((ulong)req->block, req->block_size);

    res = m2m_cmd1_mac_operation(req, (phys_addr_t)HW_M2M_PHYS_ADDR(req->block), req->block_size, true);
#endif
    if (res != HW_CE_SUCCESS)
        return res;

    return m2m_read_final_hash(req->channel, req->tid, out,
                               m2m_mac_digest_size(req->profile));
}

void m2m_cmd1_mac_deinit(m2m_mac_req_t *req)
{
    if (!req)
        return;

    if (req->desc_list)
    {
        m2m_desc_list_free(req->desc_list);
        req->desc_list = NULL;
    }
}

/*this is used just for str*/
int calc_default_mac(phys_addr_t buf1, unsigned int len, phys_addr_t buf2,
                     unsigned int len2, unsigned char *p_dst_addr)
{
    mt_s32 ret = -1;
    m2m_mac_req_t *req = NULL;

    printk("calc_default_mac [buf1: 0x%x, len: 0x%x], [buf2: 0x%x, len: 0x%x], dst: %p\n", (unsigned int)buf1, (unsigned int)len, (unsigned int)buf2, (unsigned int)len2, p_dst_addr);

    m2m_extra_notice(M2M_EXT_NOTICE_NOIRQ, M2M_EXT_NOTICE_MODE_REGISTER);

    req = (m2m_mac_req_t *)kmalloc(sizeof(m2m_mac_req_t), GFP_KERNEL);
    if (!req) {
        printk("%s:%d, kmalloc fail!\n", __func__, __LINE__);
        return -1;
    }

    ret = m2m_cmd1_mac_init(req, HMAC_SHA2_256, HW_M2M_CH0, 0, NULL, 0);
    if (len)
        ret |= m2m_cmd1_mac_update(req, buf1, len);
    if (len2)
        ret |= m2m_cmd1_mac_update(req, buf2, len2);
    ret |= m2m_cmd1_mac_final(req, p_dst_addr);
    if (ret) {
        printk("%s,%d: calculate hmac error!\n", __func__, __LINE__);
        ret = -1;
    }

    m2m_extra_notice(M2M_EXT_NOTICE_NOIRQ, M2M_EXT_NOTICE_MODE_UNREGISTER);

    if (req)
        kfree(req);

    return ret;
}
