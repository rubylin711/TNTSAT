/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_mac.h"
#include "hw_m2m_if.h"
#include "mt_lib.h"

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
        return 1;
        break;
    default:
        return 0;
        break;
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
    case CMAC_TDES:
    case CBCMAC_AES128:
        size = 16;
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
        size = 16;
    default:
        break;
    }

    return size;
}

static mt_s32 m2m_cmd0_mac_operation(m2m_mac_ctx_t *ctx, const mt_u8 *data,
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
    if (ctx->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        cmd0.KeyEn = HW_M2M_USE_DKEY;
        byteswap(cmd0.DK, ctx->dk, sizeof(cmd0.DK));
    }
    else
    {
        cmd0.KeyEn = HW_M2M_USE_KEYTABLE;
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

#ifdef M2M_USE_VIRT_ADDR
    if (data && len)
        flush_dcache_range((mt_u32)data, len);
#endif

    res = m2m_send_cmd(ctx->channel, &cmd0, sizeof(m2m_cmd0_t));
    if (res != HW_CE_SUCCESS)
        return res;

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
        key_size = MIN(sizeof(ctx->dk), 64u);
        break;
    case HMAC_SHA2_384:
    case HMAC_SHA2_512:
        key_size = MIN(sizeof(ctx->dk), 128u);
        break;
    case CMAC_AES128:
    case CMAC_TDES:
    case CBCMAC_AES128:
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
    if (ctx->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        if (direct_key_size > key_size)
            return HW_CE_ERROR_BAD_PARAMETERS;

        if (!direct_key)
            return HW_CE_ERROR_BAD_PARAMETERS;

        memcpy(ctx->dk, direct_key, direct_key_size);
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

mt_s32 m2m_cmd0_mac_update(m2m_mac_ctx_t *ctx, const mt_u8 *data, mt_u32 len)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;
#ifdef M2M_CMD0_MAC_COPY_BUFFER
    mt_u32 free_size;
    mt_u32 n_blocks;
    mt_u32 residue;
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
    if (!m2m_mac_is_hmac(ctx->profile))
        return m2m_cmd0_mac_operation(ctx, data, len, true);

    free_size = HW_M2M_MAX_MAC_BLOCK_SIZE - ctx->block_size;
    if ((ctx->block_size > 0) && (len > free_size))
    {
        memcpy(ctx->block + ctx->block_size, data, free_size);
        data += free_size;
        len -= free_size;
        res = m2m_cmd0_mac_operation(ctx, ctx->block, HW_M2M_MAX_MAC_BLOCK_SIZE, false);
        if (res != HW_CE_SUCCESS)
            return res;
        ctx->block_size = 0;
    }

    n_blocks = len / HW_M2M_MAX_MAC_BLOCK_SIZE;
    residue = len % HW_M2M_MAX_MAC_BLOCK_SIZE;

    if (n_blocks > 0)
    {
        res = m2m_cmd0_mac_operation(ctx, data, n_blocks * HW_M2M_MAX_MAC_BLOCK_SIZE, false);
        if (res != HW_CE_SUCCESS)
            return res;
        data += n_blocks * HW_M2M_MAX_MAC_BLOCK_SIZE;
    }

    if (residue > 0)
    {
        memcpy(ctx->block + ctx->block_size, data, residue);
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
        const mt_u8 *data, mt_u32 len)
{
    m2m_desc_t *desc;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = req->desc_list->ops->alloc_desc(req->desc_list->ctx, &desc);
    if (res != HW_CE_SUCCESS)
        return res;

    /* desc->Q = 1; */
    desc->S = 0;
    desc->E = 0;
    desc->SrcDataAddr = HW_M2M_ADDR(data);
    desc->ProtectedDataSize = len;

#ifdef M2M_USE_VIRT_ADDR
    if (data && len)
        flush_dcache_range((mt_u32)data, len);
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
        key_size = MIN(sizeof(req->key), 64u);
        break;
    case HMAC_SHA2_384:
    case HMAC_SHA2_512:
        key_size = MIN(sizeof(req->key), 128u);
        break;
    case CBCMAC_AES128:
    case CMAC_TDES:
    case CMAC_AES128:
        key_size = MIN(sizeof(req->key), 16u);
        break;
    case CMAC_AES192:
        key_size = MIN(sizeof(req->key), 24u);
        use_dk = 1;
        break;
    case CMAC_AES256:
        key_size = MIN(sizeof(req->key), 32u);
        use_dk = 1;
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    req->profile = profile;
    req->channel = channel;
    req->key_slot = key_slot;

    if (use_dk)
        req->key_slot = HW_M2M_INVALID_KEY_SLOT;
    else
        req->key_slot = key_slot;

    if (req->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        if (direct_key_size > key_size)
            return HW_CE_ERROR_BAD_PARAMETERS;

        if (!direct_key)
            return HW_CE_ERROR_BAD_PARAMETERS;

        memcpy(req->key, direct_key, direct_key_size);
        if (direct_key_size > HW_M2M_MAX_CMD1_DK_SIZE)
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

mt_s32 m2m_cmd1_mac_request(m2m_mac_req_t *req, const mt_u8 *data, mt_u32 len)
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
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (!req->desc_list->ops->get_count(req->desc_list->ctx))
    {
        res = m2m_cmd1_mac_request_internal(req, NULL, 0);
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

    head_desc->S = 1;
    tail_desc->E = 1;
    tail_desc->T = 1;

    memset(&cmd1, 0, sizeof(m2m_cmd1_t));
    cmd1.Cmd = HW_M2M_CMD1;
    cmd1.Q = 1;
    if (req->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        if (req->use_mac_key)
        {
            cmd1.KeyEn = HW_M2M_USE_MACKEY;
            byteswap(head_desc->HMAC_mackey, req->key, sizeof(head_desc->HMAC_mackey));
        }
        else
        {
            cmd1.KeyEn = HW_M2M_USE_DKEY;
            byteswap(cmd1.DK, req->key, sizeof(cmd1.DK));
        }
    }
    else
    {
        cmd1.KeyEn = HW_M2M_USE_KEYTABLE;
        cmd1.KeyIndx = req->key_slot;
    }
    cmd1.Profile = req->profile;
    cmd1.TID = req->tid;
    cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_desc);
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

    res = m2m_wait_cmd_finish(req->channel, req->tid);
    if (res != HW_CE_SUCCESS)
        return res;

    req->desc_list->ops->clean(req->desc_list->ctx);
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
int calc_default_mac(unsigned char *buf1, unsigned int len, unsigned char *buf2,
                     unsigned int len2, unsigned char *p_dst_addr)
{
    int rc = ~1;

    //printk(KERN_INFO "calc_default_mac	 buf 0x%x  len 0x%x  dst 0x%x \n", (unsigned int)buf1, (unsigned int)len, (unsigned int)p_dst_addr );

    m2m_mac_ctx_t ctx;
    rc = m2m_cmd0_mac_init(&ctx, HMAC_SHA2_256, HW_M2M_CH0, 0, NULL, 0);
    rc = m2m_cmd0_mac_update(&ctx, (const mt_u8 *)buf1, len);
    rc = m2m_cmd0_mac_update(&ctx, (const mt_u8 *)buf2, len2);

    rc = m2m_cmd0_mac_final(&ctx, p_dst_addr);

    return 0;
}
