/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_HASH_H_
#define _M2M_HASH_H_

#include "hw_m2m_if.h"
#include "m2m_desc.h"
#include "mt_lib.h"

/*#define M2M_CMD0_HASH_COPY_BUFFER*/

typedef struct _m2m_hash_ctx_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 state;
    mt_u8 tid;
    mt_u16 block_size;
    m2m_tee_info_t tee;
    mt_u8 context[HW_M2M_MAX_HASH_DIGEST_SIZE];
#ifdef M2M_CMD0_HASH_COPY_BUFFER
    mt_u8 block[HW_M2M_MAX_HASH_BLOCK_SIZE];
#endif
} m2m_hash_ctx_t;

typedef struct _m2m_hash_req_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 tid;
    m2m_tee_info_t tee;
    m2m_desc_list_t *desc_list;
} m2m_hash_req_t;

mt_s32 m2m_cmd0_hash_init(m2m_hash_ctx_t *ctx, mt_u32 profile, mt_u32 channel);

mt_s32 m2m_cmd0_hash_set_tee(m2m_hash_ctx_t *ctx, const m2m_tee_info_t *tee);

mt_s32 m2m_cmd0_hash_update(m2m_hash_ctx_t *ctx, const mt_u8 *data,
                            mt_u32 len);

mt_s32 m2m_cmd0_hash_save_context(m2m_hash_ctx_t *ctx, mt_u8 *context);

mt_s32 m2m_cmd0_hash_restore_context(m2m_hash_ctx_t *ctx,
                                     const mt_u8 *context);

mt_s32 m2m_cmd0_hash_final(m2m_hash_ctx_t *ctx, mt_u8 *out);

mt_s32 m2m_cmd1_hash_init(m2m_hash_req_t *req, mt_u32 profile, mt_u32 channel);

mt_s32 m2m_cmd1_hash_set_tee(m2m_hash_req_t *req, const m2m_tee_info_t *tee);

mt_s32 m2m_cmd1_hash_request(m2m_hash_req_t *req, const mt_u8 *data, mt_u32 len);

mt_s32 m2m_cmd1_hash_start_operation(m2m_hash_req_t *req);

mt_s32 m2m_cmd1_hash_wait_complete(m2m_hash_req_t *req);

mt_s32 m2m_cmd1_hash_digest(m2m_hash_req_t *req, mt_u8 *digest);

void m2m_cmd1_hash_deinit(m2m_hash_req_t *req);

#endif /*end of include guard: _M2M_HASH_H_ */
