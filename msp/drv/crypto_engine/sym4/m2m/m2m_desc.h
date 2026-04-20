/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_DESC_H_
#define _M2M_DESC_H_

#include "mt_lib.h"
#include "hw_m2m_if.h"

typedef struct _m2m_desc_list_ctx_t m2m_desc_list_ctx_t;

typedef struct _m2m_desc_list_ops_t
{
    mt_s32(*alloc_desc)(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc);
    mt_s32(*first_desc)(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc);
    mt_s32(*last_desc)(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc);
    size_t (*get_count)(m2m_desc_list_ctx_t *ctx);
    void (*flush)(m2m_desc_list_ctx_t *ctx);
    void (*clean)(m2m_desc_list_ctx_t *ctx);
} m2m_desc_list_ops_t;

typedef struct _m2m_desc_list_t
{
    m2m_desc_list_ctx_t *ctx;
    const m2m_desc_list_ops_t *ops;
} m2m_desc_list_t;

mt_s32 m2m_desc_list_alloc(m2m_desc_list_t **list);

void m2m_desc_list_free(m2m_desc_list_t *list);

#endif /* end of include guard: _M2M_DESC_H_ */
