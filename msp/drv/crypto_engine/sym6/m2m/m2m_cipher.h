/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_CIPHER_H_
#define _M2M_CIPHER_H_

#include "hw_m2m_if.h"
#include "m2m_desc.h"
#include "mt_lib.h"

typedef struct _m2m_cipher_ctx_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u16 reserved;
    mt_u8 tid;
    mt_u8 key_slot;
    mt_u8 div[HW_M2M_MAX_DIV_SIZE];
    mt_u8 dk[HW_M2M_MAX_CMD0_DK_SIZE];
    m2m_tee_info_t tee;
} m2m_cipher_ctx_t;

typedef struct _m2m_cipher_req_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 tid;
    mt_u8 key_slot;
    mt_u8 use_div;
    mt_u8 los;
    mt_u8 cryptb;
    mt_u8 skipb;
    mt_u8 lcryptb;
    mt_u8 lskipb;
    mt_u16 req_cnt;
    mt_u16 rsvd;
    mt_u8 div[HW_M2M_MAX_DIV_SIZE];
    mt_u8 dk[HW_M2M_MAX_CMD1_DK_SIZE];
    m2m_tee_info_t tee;
    m2m_desc_list_t *desc_list;
} m2m_cipher_req_t;

mt_s32 m2m_cmd0_cipher_operation(mt_u32 profile, mt_u32 channel,
                                 mt_u8 key_slot, const mt_u8 *direct_key, const mt_u8 *direct_iv,
                                 mt_u8 *dst, const mt_u8 *src, mt_u32 clear_data_size,
                                 mt_u32 prot_data_size, const m2m_tee_info_t *tee);

mt_s32 m2m_cmd0_cipher_init(m2m_cipher_ctx_t *ctx, mt_u32 profile,
                            mt_u32 channel, mt_u8 key_slot,
                            const mt_u8 *direct_key, const mt_u8 *direct_iv);

mt_s32 m2m_cmd0_cipher_set_tee(m2m_cipher_ctx_t *ctx,
                               const m2m_tee_info_t *tee);

mt_s32 m2m_cmd0_cipher_start_operation(m2m_cipher_ctx_t *ctx,
                                       mt_u8 *dst, const mt_u8 *src,
                                       mt_u32 clear_data_size, mt_u32 prot_data_size);

mt_s32 m2m_cmd0_cipher_wait_complete(m2m_cipher_ctx_t *ctx);

mt_s32 m2m_cmd1_cipher_init(m2m_cipher_req_t *req, mt_u32 profile,
                            mt_u32 channel, mt_u8 key_slot,
                            const mt_u8 *direct_key, mt_u32 direct_key_size,
                            const mt_u8 *iv, mt_u32 iv_size);

mt_s32 m2m_cmd1_cipher_set_tee(m2m_cipher_req_t *req,
                               const m2m_tee_info_t *tee);

mt_s32 m2m_cmd1_cipher_set_cbcscens(m2m_cipher_req_t *req,
                                mt_u32 profile, mt_u32 skip_block, mt_u32 crypt_block);

mt_s32 m2m_cmd1_cipher_request(m2m_cipher_req_t *req, phys_addr_t dst, const phys_addr_t src,
                               mt_u32 clear_data_size, mt_u8 los, mt_u32 prot_data_size);

mt_s32 m2m_cmd1_cipher_start_operation(m2m_cipher_req_t *req);

mt_s32 m2m_cmd1_cipher_wait_complete(m2m_cipher_req_t *req);

mt_s32 m2m_cmd1_cipher_update(m2m_cipher_req_t *req, phys_addr_t dst,
                              const phys_addr_t src, mt_u32 clear_data_size,
                              mt_u8 los, mt_u32 prot_data_size);

void m2m_cmd1_cipher_deinit(m2m_cipher_req_t *req);

#endif /*end of include guard: _M2M_CIPHER_H_ */
