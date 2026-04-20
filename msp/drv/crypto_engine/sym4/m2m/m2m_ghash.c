/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_ghash.h"
#include "mt_lib.h"

mt_s32 m2m_ghash_init(m2m_ghash_req_t *req, mt_u32 profile, mt_u32 channel,
                      mt_u8 key_slot, const mt_u8 *direct_key,
                      mt_u32 direct_key_size)
{
    mt_u32 key_size = 0;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    memset(req, 0, sizeof(m2m_ghash_req_t));

    switch (profile)
    {
    case GHASH_AES128:
        key_size = MIN(sizeof(req->dk), 16u);
        break;
    case GHASH_AES192:
        key_size = MIN(sizeof(req->dk), 24u);
        break;
    case GHASH_AES256:
        key_size = MIN(sizeof(req->dk), 32u);
        break;
    default:
        return HW_CE_ERROR_BAD_PARAMETERS;
        break;
    }

    key_size = MIN(key_size, direct_key_size);

    req->profile = profile;
    req->channel = channel;
    req->key_slot = key_slot;
    if (req->key_slot == HW_M2M_INVALID_KEY_SLOT)
    {
        if (key_size < direct_key_size)
            return HW_CE_ERROR_BAD_PARAMETERS;

        if (direct_key)
            memcpy(req->dk, direct_key, key_size);
    }

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

mt_s32 m2m_ghash_set_tee(m2m_ghash_req_t *req, const m2m_tee_info_t *tee)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!tee)
        memset(&req->tee, 0, sizeof(m2m_tee_info_t));
    else
        memcpy(&req->tee, tee, sizeof(m2m_tee_info_t));

    return HW_CE_SUCCESS;
}

mt_s32 m2m_ghash_request_auth(m2m_ghash_req_t *req, const mt_u8 *aad,
                              mt_u32 len)
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

    return HW_CE_SUCCESS;
}

mt_s32 m2m_ghash_request_cipher(m2m_ghash_req_t *req, const mt_u8 *data,
                                mt_u32 len)
{
    m2m_desc_t *desc;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!len)
        return HW_CE_SUCCESS;

    if (!req->cipher_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    res = req->cipher_desc_list->ops->alloc_desc(req->cipher_desc_list->ctx, &desc);
    if (res != HW_CE_SUCCESS)
        return res;

    desc->SrcDataAddr = HW_M2M_ADDR(data);
    desc->ProtectedDataSize = len;
    /*desc->Q = 1; */
    desc->G = 0;
    desc->S = 0;
    desc->E = 0;

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((mt_u32)data, len);
#endif
    return HW_CE_SUCCESS;
}

mt_s32 m2m_ghash_start_operation(m2m_ghash_req_t *req)
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

    if ((auth_count + cipher_count) == 0)
        return HW_CE_SUCCESS;

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

    if (auth_count > 0)
    {
        head_auth_desc->S = 1;
        if (cipher_count > 0)
        {
            tail_auth_desc->DescNextAddr = HW_M2M_PHYS_ADDR(head_cipher_desc);
            tail_cipher_desc->E = 1;
            tail_cipher_desc->T = 1;
        }
        else
        {
            tail_auth_desc->E = 1;
            tail_auth_desc->T = 1;
        }

        cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_auth_desc);
    }
    else if (cipher_count > 0)
    {
        head_cipher_desc->S = 1;
        tail_cipher_desc->E = 1;
        tail_cipher_desc->T = 1;

        cmd1.DescStartAddr = HW_M2M_PHYS_ADDR(head_cipher_desc);
    }

    cmd1.WCID = req->tee.wcid;
    cmd1.SCID = req->tee.scid;
    cmd1.SC = req->tee.sc;

    req->auth_desc_list->ops->flush(req->auth_desc_list->ctx);
    req->cipher_desc_list->ops->flush(req->cipher_desc_list->ctx);

    return m2m_send_cmd(req->channel, &cmd1, sizeof(m2m_cmd1_t));
}

mt_s32 m2m_ghash_wait_complete(m2m_ghash_req_t *req)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;
    size_t auth_count, cipher_count;

    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->auth_desc_list || !req->cipher_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    auth_count = req->auth_desc_list->ops->get_count(req->auth_desc_list->ctx);
    cipher_count = req->cipher_desc_list->ops->get_count(req->cipher_desc_list->ctx);

    if ((auth_count + cipher_count) == 0)
        return HW_CE_SUCCESS;

    res = m2m_wait_cmd_finish(req->channel, req->tid);
    if (res != HW_CE_SUCCESS)
        return res;

    req->auth_desc_list->ops->clean(req->auth_desc_list->ctx);
    req->cipher_desc_list->ops->clean(req->cipher_desc_list->ctx);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_ghash_digest(m2m_ghash_req_t *req, mt_u8 *digest)
{
    if (!req)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (!req->auth_desc_list || !req->cipher_desc_list)
        return HW_CE_ERROR_BAD_STATE;

    if (digest)
        return m2m_read_final_hash(req->channel, req->tid, digest, HW_M2M_MAX_GHASH_DIGEST_SIZE);

    return HW_CE_SUCCESS;
}

void m2m_ghash_deinit(m2m_ghash_req_t *req)
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
