/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_GHASH_H_
#define _M2M_GHASH_H_

#include "hw_m2m_if.h"
#include "m2m_desc.h"
#include "mt_lib.h"

typedef struct _m2m_ghash_req_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 tid;
    mt_u8 key_slot;
    mt_u8 dk[HW_M2M_MAX_CMD1_DK_SIZE];
    m2m_tee_info_t tee;
    m2m_desc_list_t *auth_desc_list;
    m2m_desc_list_t *cipher_desc_list;
} m2m_ghash_req_t;

mt_s32 m2m_ghash_init(m2m_ghash_req_t *req, mt_u32 profile, mt_u32 channel,
                      mt_u8 key_slot, const mt_u8 *direct_key,
                      mt_u32 direct_key_size);

mt_s32 m2m_ghash_set_tee(m2m_ghash_req_t *req, const m2m_tee_info_t *tee);

mt_s32 m2m_ghash_request_auth(m2m_ghash_req_t *req, const mt_u8 *aad,
                              mt_u32 len);

mt_s32 m2m_ghash_request_cipher(m2m_ghash_req_t *req, const mt_u8 *data,
                                mt_u32 len);

mt_s32 m2m_ghash_start_operation(m2m_ghash_req_t *req);

mt_s32 m2m_ghash_wait_complete(m2m_ghash_req_t *req);

mt_s32 m2m_ghash_digest(m2m_ghash_req_t *req, mt_u8 *result);

void m2m_ghash_deinit(m2m_ghash_req_t *req);

#endif /*end of include guard: _M2M_GHASH_H_ */
