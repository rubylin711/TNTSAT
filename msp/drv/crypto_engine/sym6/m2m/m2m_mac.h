/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_MAC_H_
#define _M2M_MAC_H_

#include "hw_m2m_if.h"
#include "m2m_desc.h"
#include "mt_lib.h"

/*#define M2M_CMD0_MAC_COPY_BUFFER*/
#define M2M_CMD1_MAC_COPY_BUFFER

typedef struct _m2m_mac_ctx_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 tid;
    mt_u8 state;
    mt_u8 key_slot;
    mt_u8 block_size;
    mt_u32 use_mac_key;
    mt_u8 dk[HW_M2M_MAX_CMD0_DK_SIZE];
    mt_u32 len_processed;
    m2m_tee_info_t tee;
    mt_u8 context[HW_M2M_MAX_MAC_DIGEST_SIZE];
#ifdef M2M_CMD0_MAC_COPY_BUFFER
    mt_u8 block[HW_M2M_MAX_HASH_BLOCK_SIZE];
#endif
} m2m_mac_ctx_t;

typedef struct _m2m_mac_req_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 tid;
    mt_u8 state;
    mt_u8 key_slot;
    mt_u8 block_size;
    mt_u32 use_mac_key;
    mt_u8 key[HW_M2M_MAX_CMD1_MACKEY_SIZE];
    mt_u32 len_processed;
    m2m_tee_info_t tee;
    m2m_desc_list_t *desc_list;
    mt_u8 context[HW_M2M_MAX_MAC_DIGEST_SIZE];
#ifdef M2M_CMD1_MAC_COPY_BUFFER
    mt_u8 block[HW_M2M_MAX_HASH_BLOCK_SIZE];
#endif
} m2m_mac_req_t;

mt_s32 m2m_cmd0_mac_init(m2m_mac_ctx_t *ctx, mt_u32 profile, mt_u32 channel,
                         mt_u8 key_slot, const mt_u8 *direct_key,
                         mt_u32 direct_key_size);

mt_s32 m2m_cmd0_mac_set_tee(m2m_mac_ctx_t *ctx, const m2m_tee_info_t *tee);

mt_s32 m2m_cmd0_mac_update(m2m_mac_ctx_t *ctx, const phys_addr_t data, mt_u32 len);

mt_s32 m2m_cmd0_mac_save_context(m2m_mac_ctx_t *ctx, mt_u8 *context);

mt_s32 m2m_cmd0_mac_restore_context(m2m_mac_ctx_t *ctx, const mt_u8 *context);

mt_s32 m2m_cmd0_mac_final(m2m_mac_ctx_t *ctx, mt_u8 *out);

mt_s32 m2m_cmd1_mac_init(m2m_mac_req_t *req, mt_u32 profile, mt_u32 channel,
                         mt_u8 key_slot, const mt_u8 *direct_key,
                         mt_u32 direct_key_size);

mt_s32 m2m_cmd1_mac_set_tee(m2m_mac_req_t *req, const m2m_tee_info_t *tee);

mt_s32 m2m_cmd1_mac_set_context(m2m_mac_req_t *req, const mt_u8 *context);

mt_s32 m2m_cmd1_mac_request(m2m_mac_req_t *req, const phys_addr_t data, mt_u32 len);

mt_s32 m2m_cmd1_mac_start_operation(m2m_mac_req_t *req);

mt_s32 m2m_cmd1_mac_wait_complete(m2m_mac_req_t *req);

mt_s32 m2m_cmd1_mac_update(m2m_mac_req_t *req, const phys_addr_t data, mt_u32 len);

mt_s32 m2m_cmd1_mac_digest(m2m_mac_req_t *req, mt_u8 *digest);

mt_s32 m2m_cmd1_mac_final(m2m_mac_req_t *req, mt_u8 *out);

void m2m_cmd1_mac_deinit(m2m_mac_req_t *req);

#endif /*end of include guard: _M2M_MAC_H_ */
