/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_TS_H_
#define _M2M_TS_H_

#include "mt_lib.h"
#include "hw_m2m_if.h"
#include "m2m_desc.h"

typedef enum _M2M_TS_SCR_MODE_E
{
    M2M_TS_SCR_NO_CHANGE = 0,
    M2M_TS_SCR_FORCE_UNSCRAMBLED,
    M2M_TS_SCR_FORCE_EVEN_KEY,
    M2M_TS_SCR_FORCE_ODD_KEY,
} M2M_TS_SCR_MODE_E;

typedef struct _m2m_ts_ctx_t
{
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 reserved;
    mt_u8 tid;
    mt_u8 even_key_slot;
    mt_u8 odd_key_slot;
    mt_u8 dk[HW_M2M_MAX_CMD0_DK_SIZE];
    mt_u8 div[HW_M2M_MAX_DIV_SIZE];
    m2m_tee_info_t tee;
} m2m_ts_ctx_t;

typedef struct _m2m_ts_req_t {
    mt_u16 profile;
    mt_u16 channel;
    mt_u8 even_key_slot;
    mt_u8 odd_key_slot;
    mt_u8 tid;
    mt_u8 use_div;
    mt_u8 los;
    mt_u8 scr_mode;
    mt_u16 req_cnt;
    mt_u8 dk[HW_M2M_MAX_CMD1_DK_SIZE];
    mt_u8 div[HW_M2M_MAX_DIV_SIZE];
    m2m_tee_info_t tee;
    m2m_desc_list_t *desc_list;
} m2m_ts_req_t;

mt_s32 m2m_cmd0_ts_init(m2m_ts_ctx_t *ctx, mt_u32 profile, mt_u32 channel,
        mt_u8 even_key_slot, mt_u8 odd_key_slot,
        const mt_u8 *direct_key, const mt_u8 *direct_iv);

mt_s32 m2m_cmd0_ts_set_tee(m2m_ts_ctx_t *ctx, const m2m_tee_info_t *tee);

mt_s32 m2m_cmd0_ts_process(m2m_ts_ctx_t *ctx, mt_u32 pid_num, const mt_u16 pid[],
        mt_u8 scr_mode, phys_addr_t dst, const phys_addr_t src, mt_u32 len);

mt_s32 m2m_cmd0_ts_start_operation(m2m_ts_ctx_t *ctx, mt_u32 pid_num, const mt_u16 pid[],
        mt_u8 scr_mode, phys_addr_t dst, const phys_addr_t src, mt_u32 len);

mt_s32 m2m_cmd0_ts_wait_complete(m2m_ts_ctx_t *ctx);

mt_s32 m2m_cmd1_ts_init(m2m_ts_req_t *req, mt_u32 profile,
        mt_u32 channel, mt_u8 scr_mode,
        mt_u8 even_key_slot, mt_u8 odd_key_slot,
        const mt_u8 *direct_key, mt_u32 direct_key_size,
        const mt_u8 *direct_iv, mt_u32 direct_iv_size);

mt_s32 m2m_cmd1_ts_set_tee(m2m_ts_req_t *req,
        const m2m_tee_info_t *tee);

mt_s32 m2m_cmd1_ts_request(m2m_ts_req_t *req, mt_u32 pid_num,
        const mt_u16 pid[], mt_u8 *dst, const mt_u8 *src, mt_u32 len);

mt_s32 m2m_cmd1_ts_start_operation(m2m_ts_req_t *req);

mt_s32 m2m_cmd1_ts_wait_complete(m2m_ts_req_t *req);

#endif /*end of include guard: _M2M_TS_H_ */
