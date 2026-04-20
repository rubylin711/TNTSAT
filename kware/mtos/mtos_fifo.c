/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>

#include "mt_type.h"
#include "mtos_fifo.h"
#include "mt_debug.h"

void mtos_fifo_flush(os_fifo_t *p_fifo)
{
    if (p_fifo == NULL) {
	MT_ASSERT(0);
	return;
    }

    p_fifo->m_cnt = 0;
    p_fifo->m_head = 0;
    p_fifo->m_tail = 0;

    return;
}

void mtos_fifo_put(os_fifo_t *p_fifo, u16 data)
{
    if (p_fifo == NULL) {
	MT_ASSERT(0);
	return;
    }

    if (p_fifo->m_cnt < p_fifo->m_size) {
	/* buffer isn't full */
	p_fifo->p_buffer[p_fifo->m_head] = data;

	p_fifo->m_head = (p_fifo->m_head + 1) % p_fifo->m_size;
	p_fifo->m_cnt++;

	return;
    } else if (p_fifo->m_overlay == TRUE) {
	/* buffer is full, but it's overlay*/
	p_fifo->p_buffer[p_fifo->m_head] = data;

	p_fifo->m_head = (p_fifo->m_head + 1) % p_fifo->m_size;
	p_fifo->m_tail = (p_fifo->m_tail + 1) % p_fifo->m_size;

	return;
    }

    MT_ASSERT(0);
    return;
}

MT_BOOL mtos_fifo_get(os_fifo_t *p_fifo, u16 *p_data)
{
    if ((p_fifo == NULL) || (p_data == NULL)) {
	MT_ASSERT(0);
	return FALSE;
    }

    if (p_fifo->m_cnt > 0) {
	/* buffer isn't empty */
	*p_data = p_fifo->p_buffer[p_fifo->m_tail];

	p_fifo->m_tail = (p_fifo->m_tail + 1) % p_fifo->m_size;
	p_fifo->m_cnt--;

	return TRUE;
    }

    return FALSE;
}

MT_BOOL mtos_fifo_get_2param(os_fifo_2data_t *p_fifo, u16 *p_data, u16 *p_data2)
{
    if ((p_fifo == NULL) || (p_data == NULL)) {
	MT_ASSERT(0);
	return FALSE;
    }

    if (p_fifo->m_cnt > 0) {
	/* buffer isn't empty */
	*p_data = p_fifo->p_buffer[p_fifo->m_tail];
	*p_data2 = p_fifo->p_buffer2[p_fifo->m_tail];
	p_fifo->m_tail = (p_fifo->m_tail + 1) % p_fifo->m_size;
	p_fifo->m_cnt--;

	return TRUE;
    }

    return FALSE;
}
