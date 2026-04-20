/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_hash.h"
#include "hw_m2m_if.h"
#include "mt_lib.h"

enum M2M_HASH_STATE
{
    M2M_HASH_STATE_INIT = 0,
    M2M_HASH_STATE_RUNNING,
    M2M_HASH_STATE_PAUSE,
    M2M_HASH_STATE_TERMINATED,
};

static inline mt_u32 m2m_hash_digest_size(mt_u32 profile)
{
    mt_u32 size = 0;

    switch (profile)
    {
    case SHA1:
        size = 20;
        break;
    case SHA2_224:
        size = 28;
        break;
    case SHA2_256:
        size = 32;
        break;
    case SHA2_384:
        size = 48;
        break;
    case SHA2_512:
        size = 64;
        break;
    default:
        break;
    }

    return size;
}

static inline mt_u32 m2m_hash_block_size(mt_u32 profile)
{
    mt_u32 size = 0;

    switch (profile)
    {
    case SHA1:
    case SHA2_224:
    case SHA2_256:
        size = 64;
        break;
    case SHA2_384:
    case SHA2_512:
        size = 128;
        break;
    default:
        break;
    }

    return size;
}

static mt_s32 m2m_cmd0_hash_operation(m2m_hash_ctx_t *ctx, const mt_u8 *data,
                                      mt_u32 len, bool terminate)
{
    m2m_cmd0_t cmd0;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (ctx->state == M2M_HASH_STATE_TERMINATED)
    {
        if (!data && !len)
            return HW_CE_SUCCESS;

        return HW_CE_ERROR_BAD_STATE;
    }

    /* Unaligned block must be the last block. */
    if (!terminate)
    {
        mt_u32 block_size = m2m_hash_block_size(ctx->profile);

        if (len & (block_size - 1))
            terminate = true;
    }

    ctx->tid = m2m_alloc_cmd_tid(ctx->channel);

    memset(&cmd0, 0, sizeof(m2m_cmd0_t));
    cmd0.Cmd = HW_M2M_CMD0;
    cmd0.Q = 1;

    if (ctx->state == M2M_HASH_STATE_PAUSE)
    {
        cmd0.HF = 1;
        res = m2m_write_hash(ctx->channel, ctx->context, HW_M2M_MAX_HASH_DIGEST_SIZE);
        if (res != HW_CE_SUCCESS)
            return res;

        ctx->state = M2M_HASH_STATE_RUNNING;
    }
    else if (ctx->state == M2M_HASH_STATE_INIT)
    {
        cmd0.S = 1;
        ctx->state = M2M_HASH_STATE_RUNNING;
    }

    if (terminate)
    {
        cmd0.E = 1;
        ctx->state = M2M_HASH_STATE_TERMINATED;
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

mt_s32 m2m_cmd0_hash_init(m2m_hash_ctx_t *ctx, mt_u32 profile, mt_u32 channel)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!(SHA1 == profile ||
            SHA2_224 == profile ||
            SHA2_256 == profile ||
            SHA2_384 == profile ||
            SHA2_512 == profile))
    {
        return HW_CE_ERROR_BAD_PARAMETERS;
    }

    memset(ctx, 0, sizeof(m2m_hash_ctx_t));
    ctx->profile = profile;
    ctx->channel = channel;
    ctx->state = M2M_HASH_STATE_INIT;
    ctx->block_size = 0;
    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_hash_set_tee(m2m_hash_ctx_t *ctx, const m2m_tee_info_t *tee)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&ctx->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&ctx->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_hash_update(m2m_hash_ctx_t *ctx, const mt_u8 *data, mt_u32 len)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;
#ifdef M2M_CMD0_HASH_COPY_BUFFER
    mt_u32 max_block_size;
    mt_u32 free_size;
    mt_u32 n_blocks;
    mt_u32 residue;
#endif

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!len)
        return HW_CE_SUCCESS;

#ifndef M2M_CMD0_HASH_COPY_BUFFER
    res = m2m_cmd0_hash_operation(ctx, data, len, false);
    if (res != HW_CE_SUCCESS)
        return res;
#else
    max_block_size = m2m_hash_block_size(ctx->profile);
    free_size = max_block_size - ctx->block_size;
    if ((ctx->block_size > 0) && (len > free_size))
    {
        memcpy(ctx->block + ctx->block_size, data, free_size);
        data += free_size;
        len -= free_size;

        res = m2m_cmd0_hash_operation(ctx, ctx->block, max_block_size, false);
        if (res != HW_CE_SUCCESS)
            return res;

        ctx->block_size = 0;
    }

    n_blocks = len / max_block_size;
    residue = len % max_block_size;

    if (n_blocks > 0)
    {
        res = m2m_cmd0_hash_operation(ctx, data, n_blocks * max_block_size, false);
        if (res != HW_CE_SUCCESS)
            return res;

        data += n_blocks * max_block_size;
    }

    if (residue > 0)
    {
        memcpy(ctx->block + ctx->block_size, data, residue);
        ctx->block_size += residue;
    }
#endif

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_hash_save_context(m2m_hash_ctx_t *ctx, mt_u8 *context)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (ctx->state == M2M_HASH_STATE_RUNNING)
    {
        res = m2m_read_hash(ctx->channel, ctx->context, HW_M2M_MAX_HASH_DIGEST_SIZE);
        if (res != HW_CE_SUCCESS)
            return res;

        if (context)
            memcpy(context, ctx->context, HW_M2M_MAX_HASH_DIGEST_SIZE);

        ctx->state = M2M_HASH_STATE_PAUSE;
    }
    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_hash_restore_context(m2m_hash_ctx_t *ctx,
                                     const mt_u8 *context)
{
    if (!ctx || !context)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memcpy(ctx->context, context, HW_M2M_MAX_HASH_DIGEST_SIZE);
    ctx->state = M2M_HASH_STATE_PAUSE;
    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd0_hash_final(m2m_hash_ctx_t *ctx, mt_u8 *out)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

#ifndef M2M_CMD0_HASH_COPY_BUFFER
    res = m2m_cmd0_hash_operation(ctx, 0, 0, true);
#else
    res = m2m_cmd0_hash_operation(ctx, ctx->block, ctx->block_size, true);
#endif
    if (res != HW_CE_SUCCESS)
        return res;

    return m2m_read_final_hash(ctx->channel, ctx->tid, out,
                               m2m_hash_digest_size(ctx->profile));
}

static mt_s32 m2m_cmd1_hash_request_internal(m2m_hash_req_t *req,
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

mt_s32 m2m_cmd1_hash_init(m2m_hash_req_t *req, mt_u32 profile, mt_u32 channel)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!(SHA1 == profile ||
            SHA2_224 == profile ||
            SHA2_256 == profile ||
            SHA2_384 == profile ||
            SHA2_512 == profile))
    {
        return HW_CE_ERROR_BAD_PARAMETERS;
    }

    memset(req, 0, sizeof(m2m_hash_req_t));
    req->profile = profile;
    req->channel = channel;
    return m2m_desc_list_alloc(&req->desc_list);
}

mt_s32 m2m_cmd1_hash_set_tee(m2m_hash_req_t *req, const m2m_tee_info_t *tee)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&req->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&req->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_cmd1_hash_request(m2m_hash_req_t *req, const mt_u8 *data, mt_u32 len)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!len)
        return HW_CE_SUCCESS;

    return m2m_cmd1_hash_request_internal(req, data, len);
}

mt_s32 m2m_cmd1_hash_start_operation(m2m_hash_req_t *req)
{
    m2m_cmd1_t cmd1;
    m2m_desc_t *head_desc = NULL;
    m2m_desc_t *tail_desc = NULL;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return -1;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (!req->desc_list->ops->get_count(req->desc_list->ctx))
    {
        res = m2m_cmd1_hash_request_internal(req, NULL, 0);
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
    cmd1.Profile = req->profile;
    cmd1.TID = req->tid;
    cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_desc);
    cmd1.WCID = req->tee.wcid;
    cmd1.SCID = req->tee.scid;
    cmd1.SC = req->tee.sc;

    req->desc_list->ops->flush(req->desc_list->ctx);
    return m2m_send_cmd(req->channel, &cmd1, sizeof(m2m_cmd1_t));
}

mt_s32 m2m_cmd1_hash_wait_complete(m2m_hash_req_t *req)
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

mt_s32 m2m_cmd1_hash_digest(m2m_hash_req_t *req, mt_u8 *digest)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (digest)
        return m2m_read_final_hash(req->channel, req->tid, digest, m2m_hash_digest_size(req->profile));

    return HW_CE_SUCCESS;
}

void m2m_cmd1_hash_deinit(m2m_hash_req_t *req)
{
    if (!req)
        return;

    if (req->desc_list)
    {
        m2m_desc_list_free(req->desc_list);
        req->desc_list = NULL;
    }
}
