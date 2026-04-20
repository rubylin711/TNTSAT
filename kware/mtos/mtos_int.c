/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mtos_int.h"

void mtos_irq_init()
{
    return;
}

void mtos_irq_request(u8 irq, void (*handle)(void), u8 type)
{
    return;
}

void mtos_irq_release(u8 irq, void (*handle)(void))
{
    return;
}

void mtos_irq_enable(MT_BOOL enable)
{
    return;
}

MT_BOOL mtos_irq_check_int(MT_BOOL *p_int)
{
    return TRUE;
}

MT_BOOL mtos_irq_set_trigger(u8 irq, irq_trg_t type)
{
    return TRUE;
}

MT_BOOL mtos_irq_bhr_create(mtos_bhr_t *p_bhr,
                         void (*routine)(void *),
                         void *param)
{
    return TRUE;
}

MT_BOOL mtos_irq_bhr_destroy(mtos_bhr_t *p_bhr)
{
    return TRUE;
}

MT_BOOL mtos_irq_bhr_start(mtos_bhr_t *p_bhr)
{
    return TRUE;
}
