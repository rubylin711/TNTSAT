/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
    initialize a ring buffer.
    *m_rb_ctrl : a pointer to MEDIA_DATA_RING_BUFFER_T
    raw_data_len : raw data buffer length
    tms_count : time stamp count

    return :
    RB_SUCCESS if initialize successful
    RB_FAILURE if initialize failed

    Allen.Wang, 2018/01/01
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "RingBuffer.h"

#define FALSE 0
#define TRUE (!FALSE)

static gboolean ring_buffer_initialize(MEDIA_DATA_RING_BUFFER_T *m_rb_ctrl,
    guint32 raw_data_len, guint32 ext_data_len, guint32 tms_num)
{
    guint8 *buf_start = NULL;

    buf_start = (guint8 *)RB_MALLOC(raw_data_len + ext_data_len);
	RB_MUTEX_INIT(&m_rb_ctrl->raw_rb.mutex);

    if (NULL == buf_start) {
        RB_PRINT("%s malloc raw buf error!\n", __func__);
        return FALSE;
    }

    m_rb_ctrl->used = 1;

    m_rb_ctrl->raw_rb.start_ext = buf_start;
    m_rb_ctrl->raw_rb.start = m_rb_ctrl->raw_rb.start_ext + ext_data_len;
    m_rb_ctrl->raw_rb.rd = m_rb_ctrl->raw_rb.wr = m_rb_ctrl->raw_rb.start;
    m_rb_ctrl->raw_rb.end = m_rb_ctrl->raw_rb.start + raw_data_len - 1;
    m_rb_ctrl->raw_rb.cnt = 0;
    m_rb_ctrl->raw_rb.size_ext = ext_data_len;
    m_rb_ctrl->raw_rb.size = raw_data_len;

    RB_PRINT("ringbuffer : st=%p, end=%p, size=0x%x\n",
        m_rb_ctrl->raw_rb.start, m_rb_ctrl->raw_rb.end, m_rb_ctrl->raw_rb.size);

	RB_MUTEX_INIT(&m_rb_ctrl->tms_rb.mutex);
    m_rb_ctrl->tms_rb.idx_rd = 0;
    m_rb_ctrl->tms_rb.idx_wr = 0;
    m_rb_ctrl->tms_rb.cnt = 0;
    m_rb_ctrl->tms_rb.size = tms_num;
    m_rb_ctrl->tms_rb.ptfrm = (FRM_INFO_T *)RB_MALLOC(sizeof(FRM_INFO_T)*(tms_num));
    if (NULL == m_rb_ctrl->tms_rb.ptfrm ) {
        RB_PRINT("%s malloc ptfrm buf failed!\n", __func__);
        free(buf_start);
        return FALSE;
    } else {
        memset(m_rb_ctrl->tms_rb.ptfrm, 0, sizeof(FRM_INFO_T)*(tms_num));
    }
    printf("Ring Buffer initialize successfully!\n");

    return TRUE;
}

/*
    check if the ring buffer is enough for write or read operation.
    This function mainly can be used for ring buffer data peeking.
    
    Arguments :
    m_rb_ctrl :
    op_len : length for read or write operation
    rw_flag : read or write flag.
    *real_len : The real length you can read or write
    *addr : the start address which you can read or write

    return : real size in bytes ring buffer has
*/
static gboolean ring_buffer_enough(MEDIA_DATA_RING_BUFFER_T *m_rb_ctrl,
    guint32 op_len, guint8 rw_flag, guint32 *real_len, guint8 **addr)
{
    gboolean enough = FALSE;
    RB_MUTEX_LOCK(&m_rb_ctrl->raw_rb.mutex);
    if (!real_len) {
        RB_PRINT("read length is null\n");
        RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);
        return FALSE;
    }
    //G_LIKELY()
    if (RB_FLAG_RD == rw_flag) {
        //rb_unlikely()
        if (op_len > 0) { //read by length
            if (m_rb_ctrl->raw_rb.cnt >= op_len) {
                enough = TRUE;
                *real_len = op_len;
                 if (addr) {
                    guint32 tail_len;

                    tail_len = m_rb_ctrl->raw_rb.end - m_rb_ctrl->raw_rb.rd + 1;

                    if (rb_unlikely(tail_len < op_len)) { // rollback
                        if(rb_unlikely(tail_len < m_rb_ctrl->raw_rb.size_ext)) {
                        //to make sure the data is continuous
                            guint8 *new_start;

                            new_start = m_rb_ctrl->raw_rb.start - tail_len;
                            memcpy((void *)new_start,
                                (const void *)m_rb_ctrl->raw_rb.rd, (size_t)tail_len);
                            m_rb_ctrl->raw_rb.rd = (m_rb_ctrl->raw_rb.start - tail_len);
                        } else {
                            *real_len = tail_len;
                        }
                    }
                    *addr = m_rb_ctrl->raw_rb.rd;
                }
            }
            else {
                enough = FALSE;
                *real_len = m_rb_ctrl->raw_rb.cnt;
            }
        }
    }
    else if (RB_FLAG_WR == rw_flag) {
    //for write we only care the size, the timestamp is processed in write
        if ((m_rb_ctrl->raw_rb.cnt + op_len) <= m_rb_ctrl->raw_rb.size) {
            enough = TRUE;
        } else {//data is not enough
            enough = FALSE;
        }
        *real_len = m_rb_ctrl->raw_rb.size - m_rb_ctrl->raw_rb.cnt;
        if (addr)
            *addr = m_rb_ctrl->raw_rb.wr;
    } else {
        RB_PRINT("unsupported operation to ring buffer!\n");
        enough = FALSE;
    }
    RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);
    return enough;
}

/*
    update the read / write pointer and valid data counter of the ring buffer,
    if the tms is valid , we also update its pointer and counter

    Arguments :
    m_rb_ctrl :
    rw_flag : read or write flag
    tms_valid : if time stamp shoud be updated
    size : length for read or write operation

    return :
    TRUE
*/
static gboolean ring_buffer_validate(MEDIA_DATA_RING_BUFFER_T *m_rb_ctrl,
    guint8 rw_flag, guint32 size)
{
    RB_MUTEX_LOCK(&m_rb_ctrl->raw_rb.mutex);
    if (RB_FLAG_WR == rw_flag) {
        m_rb_ctrl->raw_rb.wr += size;
        if (m_rb_ctrl->raw_rb.wr > m_rb_ctrl->raw_rb.end) {
            m_rb_ctrl->raw_rb.wr = m_rb_ctrl->raw_rb.start +
            (m_rb_ctrl->raw_rb.wr - m_rb_ctrl->raw_rb.end) - 1;
        }
        m_rb_ctrl->raw_rb.cnt += size;
    } else {
        m_rb_ctrl->raw_rb.rd += size;
        if (m_rb_ctrl->raw_rb.rd > m_rb_ctrl->raw_rb.end) {
            m_rb_ctrl->raw_rb.rd = m_rb_ctrl->raw_rb.start +
                (m_rb_ctrl->raw_rb.rd - m_rb_ctrl->raw_rb.end) - 1;
        }
        m_rb_ctrl->raw_rb.cnt -= size;
    }
    RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);

    return TRUE;
}

static gint ring_buffer_write(MEDIA_DATA_RING_BUFFER_T *m_rb_ctrl, const guint8 *src, 
    guint32 write_len, gboolean do_copy, guint64 tms)
{
    RB_MUTEX_LOCK(&m_rb_ctrl->raw_rb.mutex);
    if (m_rb_ctrl->raw_rb.size < (m_rb_ctrl->raw_rb.cnt + write_len)) {
        RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);
        return RB_FAILURE;
    }

    // copy the raw data to ring buffer
    if (do_copy) {
        if ((m_rb_ctrl->raw_rb.wr + write_len) > m_rb_ctrl->raw_rb.end) {
            guint32 copy_len, remain_len;

            //g_print("wr rollback!\n");
            copy_len = m_rb_ctrl->raw_rb.end - m_rb_ctrl->raw_rb.wr + 1;
            remain_len = write_len - copy_len;
            memcpy((void *)m_rb_ctrl->raw_rb.wr, (const void *)src, (size_t)copy_len);
            memcpy((void *)m_rb_ctrl->raw_rb.start, (const void *)(src + copy_len),
                (size_t)remain_len);
        } else {
            //printf("ring buffer write %p, %p, len %u\n", m_rb_ctrl->raw_rb.wr, src, write_len);
            memcpy((void *)m_rb_ctrl->raw_rb.wr, (const void *)src, (size_t)write_len);
        }
    }

    if (m_rb_ctrl->tms_rb.cnt >= m_rb_ctrl->tms_rb.size) {
        printf("tms write out of range : %d %d!\n", m_rb_ctrl->tms_rb.cnt, m_rb_ctrl->tms_rb.size);
    } else {
        gboolean is_repeated = FALSE;
        if (m_rb_ctrl->tms_rb.cnt > 0) {
            guint32 last_wr_pos = 0;
            if (m_rb_ctrl->tms_rb.idx_wr == 0) {
                last_wr_pos = m_rb_ctrl->tms_rb.size - 1;
            } else {
                last_wr_pos = m_rb_ctrl->tms_rb.idx_wr - 1;
            }
            if (m_rb_ctrl->tms_rb.ptfrm[last_wr_pos].tms == tms)
                is_repeated = TRUE;
        }
        if (!is_repeated) {//new tms
            m_rb_ctrl->tms_rb.ptfrm[m_rb_ctrl->tms_rb.idx_wr].pos = m_rb_ctrl->raw_rb.wr;//record the write pos
            m_rb_ctrl->tms_rb.ptfrm[m_rb_ctrl->tms_rb.idx_wr].tms = tms;
            //printf("write tms : %p, 0x%llx\n", m_rb_ctrl->tms_rb.ptfrm[m_rb_ctrl->tms_rb.idx_wr].pos, tms);
            m_rb_ctrl->tms_rb.ptfrm[m_rb_ctrl->tms_rb.idx_wr].size = write_len;
            m_rb_ctrl->tms_rb.cnt++;
            m_rb_ctrl->tms_rb.idx_wr++;
            m_rb_ctrl->tms_rb.idx_wr %= m_rb_ctrl->tms_rb.size;
        } else {
            //printf("repeated tms : 0x%llx!\n", tms);
            m_rb_ctrl->tms_rb.ptfrm[m_rb_ctrl->tms_rb.idx_wr].size += write_len;
        }
    }

    RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);

    return RB_SUCCESS;
}

static gint ring_buffer_read(MEDIA_DATA_RING_BUFFER_T *m_rb_ctrl,
    guint8 *dst, guint32 read_len, gboolean do_copy, guint32 *real_len, guint64 *tms)
{
    guint32 cur_rd_pos, next_rd_pos, frm_size;
    RB_MUTEX_LOCK(&m_rb_ctrl->raw_rb.mutex);
    //rb_unlikely
    if (m_rb_ctrl->raw_rb.cnt < read_len || !read_len) {
        //g_print("data not enough for read %d %d, !\n",
        //    m_rb_ctrl->raw_rb.cnt, read_len);
        RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);
        return RB_FAILURE;
    }

    frm_size = read_len;
    if (m_rb_ctrl->tms_rb.cnt == 0) {
        *tms = INVALID_TMS;
    } else {
        if (m_rb_ctrl->tms_rb.cnt == 1) {
            *tms = m_rb_ctrl->tms_rb.ptfrm[m_rb_ctrl->tms_rb.idx_rd].tms;
        } else { // >1
            cur_rd_pos = m_rb_ctrl->tms_rb.idx_rd;
            next_rd_pos = (m_rb_ctrl->tms_rb.idx_rd + 1)%(m_rb_ctrl->tms_rb.size);
            if (m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].pos > m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos) {
                if (m_rb_ctrl->raw_rb.rd >= m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].pos) {
                    frm_size = m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos + m_rb_ctrl->raw_rb.size - m_rb_ctrl->raw_rb.rd;
                    *tms = m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].tms;
                    //printf("%s %d : %d,%d, %d, %p %p %p %p\n", __func__, __LINE__, frm_size, m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].size,
                    //    m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].size,  m_rb_ctrl->raw_rb.start,  m_rb_ctrl->raw_rb.rd,
                } else if (m_rb_ctrl->raw_rb.rd < m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos) {
                    frm_size = m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos - m_rb_ctrl->raw_rb.rd;
                    *tms = m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].tms;
                } else if (m_rb_ctrl->raw_rb.rd == m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos) {
                    *tms = m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].tms;
                    m_rb_ctrl->tms_rb.cnt--;
                    m_rb_ctrl->tms_rb.idx_rd++;
                    m_rb_ctrl->tms_rb.idx_rd %= m_rb_ctrl->tms_rb.size;
                    if (frm_size > m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].size)
                        frm_size = m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].size;
                } else {
                    printf("%s %d %d %d, unexpected case : %p %p %p rd>next or rd<cur !\n", __func__, __LINE__, m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].size,
                        frm_size, m_rb_ctrl->raw_rb.rd, m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos, m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].pos);
                }
            } else {
                if ((m_rb_ctrl->raw_rb.rd >= m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].pos) &&
                    (m_rb_ctrl->raw_rb.rd < m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos)) {
                    *tms = m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].tms;
                    frm_size = m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos - m_rb_ctrl->raw_rb.rd;
                } else if (m_rb_ctrl->raw_rb.rd == m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos){
                    *tms = m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].tms;
                    m_rb_ctrl->tms_rb.cnt--;
                    m_rb_ctrl->tms_rb.idx_rd++;
                    m_rb_ctrl->tms_rb.idx_rd %= m_rb_ctrl->tms_rb.size;
                    if (frm_size > m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].size)
                        frm_size = m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].size;
                } else {
                    printf("%s %d %d %d, unexpected case : %p %p %p rd>next or rd<cur !\n", __func__, __LINE__, m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].size,
                        frm_size, m_rb_ctrl->raw_rb.rd, m_rb_ctrl->tms_rb.ptfrm[next_rd_pos].pos, m_rb_ctrl->tms_rb.ptfrm[cur_rd_pos].pos);
                }
            }
        }
    }

    if (read_len > frm_size)
        read_len = frm_size;
    if (do_copy) {
        if (m_rb_ctrl->raw_rb.rd + read_len > m_rb_ctrl->raw_rb.end) {
            guint32 copy_len, remain_len;

            //g_print("rd rollback!\n");
            copy_len = m_rb_ctrl->raw_rb.end - m_rb_ctrl->raw_rb.rd + 1;
            remain_len = read_len - copy_len;
            memcpy((void *)dst, (const void *)m_rb_ctrl->raw_rb.rd, (size_t)copy_len);
            memcpy((void *)(dst + copy_len), (const void *)m_rb_ctrl->raw_rb.start,
                (size_t)remain_len);
        } else {
            memcpy((void *)dst, (const void *)m_rb_ctrl->raw_rb.rd, (size_t)read_len);
        }
    }
    *real_len = read_len;
    RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);

    return RB_SUCCESS;
}

static gboolean ring_buffer_finalize(MEDIA_DATA_RING_BUFFER_T *m_rb_ctrl)
{
    if (0 != m_rb_ctrl->raw_rb.start_ext)
        RB_FREE((void *)m_rb_ctrl->raw_rb.start_ext);
    RB_MUTEX_DESTROY(&m_rb_ctrl->raw_rb.mutex);

    if (m_rb_ctrl->tms_rb.ptfrm)
        RB_FREE((void *)m_rb_ctrl->tms_rb.ptfrm);

    m_rb_ctrl->used = 0;

    return TRUE;
}

static gboolean ring_buffer_reset(MEDIA_DATA_RING_BUFFER_T *m_rb_ctrl)
{
    RB_MUTEX_LOCK(&m_rb_ctrl->raw_rb.mutex);
    m_rb_ctrl->raw_rb.cnt = 0;
    m_rb_ctrl->raw_rb.rd = m_rb_ctrl->raw_rb.wr = m_rb_ctrl->raw_rb.start;
    m_rb_ctrl->tms_rb.cnt = 0;
    m_rb_ctrl->tms_rb.idx_rd = m_rb_ctrl->tms_rb.idx_wr = 0;
    RB_MUTEX_UNLOCK(&m_rb_ctrl->raw_rb.mutex);
   
    return TRUE;
}

static RING_BUFFER_PLUGIN_T rb_entity = {
    ring_buffer_initialize,
    ring_buffer_enough,
    ring_buffer_validate,
    ring_buffer_write,
    ring_buffer_read,
    ring_buffer_finalize,
    ring_buffer_reset
};

RING_BUFFER_PLUGIN_T *get_rb_plugin(void)
{
    return &rb_entity;
}

