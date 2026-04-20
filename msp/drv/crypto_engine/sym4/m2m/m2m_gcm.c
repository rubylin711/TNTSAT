/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_gcm.h"
#include "m2m_ghash.h"
#include "mt_lib.h"

static int m2m_gcm_soft_inject = 0;

static mt_s32 m2m_gcm_get_iv(mt_u32 profile, mt_u32 channel,
                             mt_u8 key_slot, const mt_u8 *key, mt_u32 key_size,
                             const mt_u8 *iv, mt_u32 iv_size, mt_u8 *out)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (iv_size == 12)
    {
        memset(out, 0, 16);
        memcpy(out, iv, 12);
        out[15] = 1;
        res = HW_CE_SUCCESS;
    }
    else
    {
        m2m_ghash_req_t req;
        mt_u32 algo;
        mt_u32 hash;

        algo = HW_M2M_CIPHER_ALGO(profile);
        switch (algo)
        {
        case HW_M2M_CIPHER_ALGO_AES128:
            hash = GHASH_AES128;
            break;
        case HW_M2M_CIPHER_ALGO_AES192:
            hash = GHASH_AES192;
            break;
        case HW_M2M_CIPHER_ALGO_AES256:
            hash = GHASH_AES256;
            break;
        default:
            return HW_CE_ERROR_BAD_PARAMETERS;
            break;
        }

        res = m2m_ghash_init(&req, hash, channel, key_slot, key, key_size);
        if (res != HW_CE_SUCCESS)
            return res;
        res = m2m_ghash_request_cipher(&req, iv, iv_size);
        if (res != HW_CE_SUCCESS)
            goto out;
        res = m2m_ghash_start_operation(&req);
        if (res != HW_CE_SUCCESS)
            goto out;
        res = m2m_ghash_wait_complete(&req);
        if (res != HW_CE_SUCCESS)
            goto out;
        res = m2m_ghash_digest(&req, out);
out:
        m2m_ghash_deinit(&req);
    }

    return res;
}

mt_s32 m2m_gcm_init(m2m_gcm_req_t *req, mt_u32 profile, mt_u32 channel,
                    mt_u8 key_slot, const mt_u8 *direct_key,
                    mt_u32 direct_key_size, const mt_u8 *iv, mt_u32 iv_size)
{
    mt_u32 key_size = 0;
    mt_u8 use_dk = 0;
    mt_u32 algo;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memset(req, 0, sizeof(m2m_gcm_req_t));

    algo = HW_M2M_CIPHER_ALGO(profile);
    switch (algo)
    {
    case HW_M2M_CIPHER_ALGO_AES128:
        key_size = MIN(sizeof(req->dk), 16u);
        break;
    case HW_M2M_CIPHER_ALGO_AES192:
        key_size = MIN(sizeof(req->dk), 24u);
        use_dk = 1;
        break;
    case HW_M2M_CIPHER_ALGO_AES256:
        key_size = MIN(sizeof(req->dk), 32u);
        use_dk = 1;
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    key_size = MIN(key_size, direct_key_size);

    req->profile = profile;
    req->channel = channel;

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

        memcpy(req->dk, direct_key, direct_key_size);
    }

    res = m2m_gcm_get_iv(profile, channel, key_slot, direct_key, key_size, iv,
                         iv_size, req->iv);
    if (res != HW_CE_SUCCESS)
        return res;

    res = m2m_desc_list_alloc(&req->auth_desc_list);
    if (res != HW_CE_SUCCESS)
        return res;
    res = m2m_desc_list_alloc(&req->cipher_desc_list);
    if (res != HW_CE_SUCCESS)
        goto err_free_auth;

    return HW_CE_SUCCESS;

err_free_auth:
    m2m_desc_list_free(req->auth_desc_list);
    req->auth_desc_list = NULL;
    return res;
}

mt_s32 m2m_gcm_set_tee(m2m_gcm_req_t *req, const m2m_tee_info_t *tee)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&req->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&req->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_gcm_request_auth(m2m_gcm_req_t *req, const mt_u8 *aad, mt_u32 len)
{
    m2m_desc_t *desc;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!len)
        return HW_CE_SUCCESS;

    if (!req->auth_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = req->auth_desc_list->ops->alloc_desc(req->auth_desc_list->ctx, &desc);
    if (res != HW_CE_SUCCESS)
        return res;

    desc->SrcDataAddr = HW_M2M_ADDR(aad);
    desc->ProtectedDataSize = len;
    /*desc->Q = 1; */
    desc->G = 1;
    desc->S = 0;
    desc->E = 0;

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((mt_u32)aad, len);
#endif
    req->aad_len += len;

    return HW_CE_SUCCESS;
}

mt_s32 m2m_gcm_request_cipher(m2m_gcm_req_t *req, mt_u8 *dst,
                              const mt_u8 *src, mt_u32 clear_data_size,
                              mt_u32 prot_data_size)
{
    m2m_desc_t *desc;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!clear_data_size && !prot_data_size)
        return HW_CE_SUCCESS;

    if (!req->cipher_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = req->cipher_desc_list->ops->alloc_desc(req->cipher_desc_list->ctx, &desc);
    if (res != HW_CE_SUCCESS)
        return res;

    desc->SrcDataAddr = HW_M2M_ADDR(src);
    desc->DstDataAddr = HW_M2M_ADDR(dst);
    desc->ClearDataSize = clear_data_size;
    desc->ProtectedDataSize = prot_data_size;
    desc->LS = 1;
    desc->LOS = 0;
    /*desc->Q = 1; */
    desc->G = 0;
    desc->S = 0;
    desc->E = 0;
    desc->F = 0;
    desc->I = 0;

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((mt_u32)src, clear_data_size + prot_data_size);
    if (dst != src)
        flush_dcache_range((mt_u32)dst, clear_data_size + prot_data_size);
#endif
    req->data_len += prot_data_size;

    return HW_CE_SUCCESS;
}

mt_s32 m2m_gcm_start_operation(m2m_gcm_req_t *req)
{
    m2m_cmd1_t cmd1;
    size_t auth_count, cipher_count;
    m2m_desc_t *head_auth_desc = NULL;
    m2m_desc_t *tail_auth_desc = NULL;
    m2m_desc_t *head_cipher_desc = NULL;
    m2m_desc_t *tail_cipher_desc = NULL;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->auth_desc_list || !req->cipher_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    auth_count = req->auth_desc_list->ops->get_count(req->auth_desc_list->ctx);
    cipher_count = req->cipher_desc_list->ops->get_count(req->cipher_desc_list->ctx);

    res = req->auth_desc_list->ops->first_desc(req->auth_desc_list->ctx, &head_auth_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->auth_desc_list->ops->last_desc(req->auth_desc_list->ctx, &tail_auth_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->cipher_desc_list->ops->first_desc(req->cipher_desc_list->ctx, &head_cipher_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->cipher_desc_list->ops->last_desc(req->cipher_desc_list->ctx, &tail_cipher_desc);
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

    req->aad_len <<= 3;
    req->data_len <<= 3;
    byteswap(&req->aad_len, &req->aad_len, sizeof(req->aad_len));
    byteswap(&req->data_len, &req->data_len, sizeof(req->data_len));

    memset(&req->final_req, 0, sizeof(req->final_req));
    req->final_req.SrcDataAddr = HW_M2M_PHYS_ADDR(&req->aad_len);
    req->final_req.ProtectedDataSize = sizeof(req->aad_len) + sizeof(req->data_len);
    /*req->final_req.Q = 1; */
    req->final_req.G = 1;
    req->final_req.S = 0;
    req->final_req.E = 1;
    req->final_req.F = 1;
    req->final_req.I = 1;
    if (m2m_gcm_soft_inject)
    {
        req->final_req.HF = 1;
    }
    byteswap(req->final_req.GCM_IVauth, req->iv,
             sizeof(req->final_req.GCM_IVauth));
    req->final_req.T = 1;

    flush_dcache_range((mt_u32)&req->aad_len,
                       sizeof(req->aad_len) + sizeof(req->data_len));

    if (auth_count > 0)
    {
        head_auth_desc->S = 1;
        if (cipher_count > 0)
        {
            tail_auth_desc->DescNextAddr = HW_M2M_PHYS_ADDR(head_cipher_desc);
            tail_cipher_desc->DescNextAddr = HW_M2M_PHYS_ADDR(&req->final_req);
            if (m2m_gcm_soft_inject)
            {
                tail_auth_desc->Q = 1;
                tail_auth_desc->P = 1;
                head_cipher_desc->HF = 1;
                tail_cipher_desc->Q = 1;
                tail_cipher_desc->P = 1;
            }

            if (head_cipher_desc->LS == 0)
            {
                head_cipher_desc->LS = 1;
                head_cipher_desc->LOS = 0;
            }
            head_cipher_desc->F = 1;
            head_cipher_desc->I = 1;
            byteswap(head_cipher_desc->GCM_DIV, req->iv, sizeof(head_cipher_desc->GCM_DIV));
            head_cipher_desc->GCM_DIV[0] += 1;
        }
        else
        {
            tail_auth_desc->DescNextAddr = HW_M2M_PHYS_ADDR(&req->final_req);

            if (m2m_gcm_soft_inject)
            {
                tail_auth_desc->Q = 1;
                tail_auth_desc->P = 1;
            }
        }

        cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_auth_desc);
    }
    else if (cipher_count > 0)
    {
        tail_cipher_desc->DescNextAddr = HW_M2M_PHYS_ADDR(&req->final_req);

        if (m2m_gcm_soft_inject)
        {
            head_cipher_desc->HF = 1;
            tail_cipher_desc->Q = 1;
            tail_cipher_desc->P = 1;
        }

        if (head_cipher_desc->LS == 0)
        {
            head_cipher_desc->LS = 1;
            head_cipher_desc->LOS = 0;
        }
        head_cipher_desc->S = 1;
        head_cipher_desc->F = 1;
        head_cipher_desc->I = 1;
        byteswap(head_cipher_desc->GCM_DIV, req->iv,
                 sizeof(head_cipher_desc->GCM_DIV));
        head_cipher_desc->GCM_DIV[0] += 1;

        cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_cipher_desc);
    }
    else
    {
        req->final_req.S = 1;

        cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(&req->final_req);
    }

    req->auth_desc_list->ops->flush(req->auth_desc_list->ctx);
    req->cipher_desc_list->ops->flush(req->cipher_desc_list->ctx);
    flush_dcache_range((mt_u32)&req->final_req, sizeof(req->final_req));

    return m2m_send_cmd(req->channel, &cmd1, sizeof(m2m_cmd1_t));
}

mt_s32 m2m_gcm_wait_complete(m2m_gcm_req_t *req)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;
    size_t auth_count, cipher_count;
    m2m_desc_t *head_auth_desc = NULL;
    m2m_desc_t *tail_auth_desc = NULL;
    m2m_desc_t *head_cipher_desc = NULL;
    m2m_desc_t *tail_cipher_desc = NULL;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->auth_desc_list || !req->cipher_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    auth_count = req->auth_desc_list->ops->get_count(req->auth_desc_list->ctx);
    cipher_count = req->cipher_desc_list->ops->get_count(req->cipher_desc_list->ctx);
    res = req->auth_desc_list->ops->first_desc(req->auth_desc_list->ctx, &head_auth_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->auth_desc_list->ops->last_desc(req->auth_desc_list->ctx, &tail_auth_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->cipher_desc_list->ops->first_desc(req->cipher_desc_list->ctx, &head_cipher_desc);
    if (res != HW_CE_SUCCESS)
        return res;
    res = req->cipher_desc_list->ops->last_desc(req->cipher_desc_list->ctx, &tail_cipher_desc);
    if (res != HW_CE_SUCCESS)
        return res;


    if (m2m_gcm_soft_inject)
    {
        mt_u8 buf[HW_M2M_MAX_GHASH_DIGEST_SIZE];

        if (auth_count > 0)
        {
            res = m2m_wait_desc_finish(req->channel, req->tid);
            if (res != HW_CE_SUCCESS)
                return res;

            res = m2m_read_hash(req->channel, buf, HW_M2M_MAX_GHASH_DIGEST_SIZE);
            if (res != HW_CE_SUCCESS)
                return res;

            if (cipher_count > 0)
            {
                memcpy(head_cipher_desc->GCM_GHASH, buf, sizeof(head_cipher_desc->GCM_GHASH));
                req->cipher_desc_list->ops->flush(req->cipher_desc_list->ctx);

                tail_auth_desc->P = 0;
                req->auth_desc_list->ops->flush(req->auth_desc_list->ctx);

                res = m2m_wait_desc_finish(req->channel, req->tid);
                if (res != HW_CE_SUCCESS)
                    return res;

                res = m2m_read_hash(req->channel, buf, HW_M2M_MAX_GHASH_DIGEST_SIZE);
                if (res != HW_CE_SUCCESS)
                    return res;

                memcpy(req->final_req.GCM_GHASH, buf, sizeof(req->final_req.GCM_GHASH));
                flush_dcache_range((mt_u32)&req->final_req, sizeof(req->final_req));

                tail_cipher_desc->P = 0;
                req->cipher_desc_list->ops->flush(req->cipher_desc_list->ctx);
            }
            else
            {
                memcpy(req->final_req.GCM_GHASH, buf, sizeof(req->final_req.GCM_GHASH));
                flush_dcache_range((mt_u32)&req->final_req, sizeof(req->final_req));

                tail_auth_desc->P = 0;
                req->auth_desc_list->ops->flush(req->auth_desc_list->ctx);
            }
        }
        else if (cipher_count > 0)
        {
            res = m2m_wait_desc_finish(req->channel, req->tid);
            if (res != HW_CE_SUCCESS)
                return res;

            res = m2m_read_hash(req->channel, buf, HW_M2M_MAX_GHASH_DIGEST_SIZE);
            if (res != HW_CE_SUCCESS)
                return res;

            memcpy(req->final_req.GCM_GHASH, buf, sizeof(req->final_req.GCM_GHASH));
            flush_dcache_range((mt_u32)&req->final_req, sizeof(req->final_req));

            tail_cipher_desc->P = 0;
            req->cipher_desc_list->ops->flush(req->cipher_desc_list->ctx);
        }
    }

    res = m2m_wait_cmd_finish(req->channel, req->tid);
    if (res != HW_CE_SUCCESS)
        return res;

    req->auth_desc_list->ops->clean(req->auth_desc_list->ctx);
    req->cipher_desc_list->ops->clean(req->cipher_desc_list->ctx);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_gcm_get_tag(m2m_gcm_req_t *req, mt_u8 *tag)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->auth_desc_list || !req->cipher_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (tag)
        return m2m_read_final_hash(req->channel, req->tid, tag, HW_M2M_MAX_GCM_TAG_SIZE);

    return HW_CE_SUCCESS;
}

void m2m_gcm_deinit(m2m_gcm_req_t *req)
{
    if (!req)
        return;

    if (req->auth_desc_list)
    {
        m2m_desc_list_free(req->auth_desc_list);
        req->auth_desc_list = NULL;
    }

    if (req->cipher_desc_list)
    {
        m2m_desc_list_free(req->cipher_desc_list);
        req->cipher_desc_list = NULL;
    }
}

void m2m_gcm_enable_soft_inject(int enable)
{
    m2m_gcm_soft_inject = enable;
}
