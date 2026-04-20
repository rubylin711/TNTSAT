/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "m2m_desc.h"

struct _m2m_desc_node_t
{
    m2m_desc_t desc;
    struct list_head head;
};

typedef struct _m2m_desc_node_t m2m_desc_node_t;

struct _m2m_desc_list_ctx_t
{
    struct list_head list;
    size_t count;
};

static mt_s32 m2m_desc_list_alloc_desc(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc);
static mt_s32 m2m_desc_list_first_desc(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc);
static mt_s32 m2m_desc_list_last_desc(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc);
static mt_s32 m2m_desc_list_specified_desc(m2m_desc_list_ctx_t *ctx, size_t index, m2m_desc_t **desc);
static size_t m2m_desc_list_get_count(m2m_desc_list_ctx_t *ctx);
static void m2m_desc_list_flush(m2m_desc_list_ctx_t *ctx);
static void m2m_desc_list_clean(m2m_desc_list_ctx_t *ctx);

static m2m_desc_list_ops_t m2m_desc_list_ops =
{
    .alloc_desc = m2m_desc_list_alloc_desc,
    .first_desc = m2m_desc_list_first_desc,
    .last_desc = m2m_desc_list_last_desc,
    .specified_desc = m2m_desc_list_specified_desc,
    .get_count = m2m_desc_list_get_count,
    .flush = m2m_desc_list_flush,
    .clean = m2m_desc_list_clean,
};

static mt_s32 m2m_desc_list_alloc_desc(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc)
{
    m2m_desc_node_t *node = NULL;

    if (!ctx || !desc)
        return HW_CE_ERROR_BAD_PARAMETERS;

    node = kmalloc(sizeof(*node), GFP_KERNEL);
    if (!node)
        return HW_CE_ERROR_OUT_OF_MEMORY;

    memset(node, 0, sizeof(*node));
    list_add_tail(&node->head, &ctx->list);

    if (node != list_first_entry(&ctx->list, m2m_desc_node_t, head))
    {
        m2m_desc_node_t *prev = list_prev_entry(node, head);
        prev->desc.DescNextAddr = HW_M2M_PHYS_ADDR(&node->desc);
        prev->desc.AT_ND = m2m_get_mapping_bus(HW_M2M_PHYS_ADDR(&node->desc), sizeof(node->desc));
    }

    *desc = &node->desc;
    ctx->count++;
    return HW_CE_SUCCESS;
}

static mt_s32 m2m_desc_list_first_desc(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (list_empty(&ctx->list))
    {
        *desc = NULL;
        return HW_CE_SUCCESS;
    }

    *desc = &list_first_entry(&ctx->list, m2m_desc_node_t, head)->desc;
    return HW_CE_SUCCESS;
}

static mt_s32 m2m_desc_list_last_desc(m2m_desc_list_ctx_t *ctx, m2m_desc_t **desc)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (list_empty(&ctx->list))
    {
        *desc = NULL;
        return HW_CE_SUCCESS;
    }

    *desc = &list_last_entry(&ctx->list, m2m_desc_node_t, head)->desc;
    return HW_CE_SUCCESS;
}

static mt_s32 m2m_desc_list_specified_desc(m2m_desc_list_ctx_t *ctx, size_t index, m2m_desc_t **desc)
{
    m2m_desc_node_t *node = NULL;
    size_t desc_pos = 1;

    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if (list_empty(&ctx->list))
    {
        *desc = NULL;
        return HW_CE_SUCCESS;
    }

    list_for_each_entry(node, &ctx->list, head)
    {
        if (index == desc_pos)
            break;
        desc_pos++;
    }

    if (index != desc_pos)
        return HW_CE_ERROR_BAD_PARAMETERS;

    *desc = &node->desc;
    return HW_CE_SUCCESS;
}

static size_t m2m_desc_list_get_count(m2m_desc_list_ctx_t *ctx)
{
    if (!ctx)
        return HW_CE_ERROR_BAD_PARAMETERS;

    return ctx->count;
}

static void m2m_desc_list_flush(m2m_desc_list_ctx_t *ctx)
{
    m2m_desc_node_t *node = NULL;

    if (!ctx)
        return;

    list_for_each_entry(node, &ctx->list, head)
    {
        flush_dcache_range((ulong)&node->desc, sizeof(node->desc));
    }
}

static void m2m_desc_list_clean(m2m_desc_list_ctx_t *ctx)
{
    m2m_desc_node_t *node = NULL;
    m2m_desc_node_t *next = NULL;

    if (!ctx)
        return;

    list_for_each_entry_safe(node, next, &ctx->list, head)
    {
        list_del(&node->head);
        kfree(node);
    }

    ctx->count = 0;
}

mt_s32 m2m_desc_list_alloc(m2m_desc_list_t **list)
{
    m2m_desc_list_t *desc_list = NULL;
    mt_s32 res = HW_CE_ERROR_GENERIC;

    if (!list)
        return HW_CE_ERROR_BAD_PARAMETERS;

    desc_list = kmalloc(sizeof(*desc_list), GFP_KERNEL);
    if (!desc_list)
        return HW_CE_ERROR_OUT_OF_MEMORY;

    memset(desc_list, 0, sizeof(*desc_list));

    desc_list->ops = &m2m_desc_list_ops;
    desc_list->ctx = kmalloc(sizeof(*desc_list->ctx), GFP_KERNEL);
    if (!desc_list->ctx)
    {
        res = HW_CE_ERROR_OUT_OF_MEMORY;
        goto err_free_list;
    }

    memset(desc_list->ctx, 0, sizeof(*desc_list->ctx));

    INIT_LIST_HEAD(&desc_list->ctx->list);
    *list = desc_list;
    return HW_CE_SUCCESS;

err_free_list:
    kfree(desc_list);
    return res;
}

void m2m_desc_list_free(m2m_desc_list_t *list)
{
    if (!list)
        return;

    if (list->ctx)
    {
        list->ops->clean(list->ctx);

        kfree(list->ctx);
        list->ctx = NULL;
    }

    list->ops = NULL;
    kfree(list);
}
