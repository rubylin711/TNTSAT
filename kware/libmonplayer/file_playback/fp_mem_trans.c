/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#define MODULE_TAG "FPMEM"
#include "mutil.h"
#include "mlog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mt_type.h"
#include "file_playback_sequence.h"
#include "lib_memp.h"
#include "mt_common.h"


#if MEMP_DEBUG
void dump_allocated_mem(lib_memp_t *p_memp)
{
    int idx;
    memp_cb_t   *p_memp_cb = (memp_cb_t *) p_memp;
    memp_link_t *p_buffer  = p_memp_cb->p_piece;
    int mem_size  = p_memp_cb->size;

    while (mem_size > 0) {
        MLOGI("piece%4d - size = %d addr = 0x%x! alloced = %d\n",
            idx, p_buffer->size, p_buffer, p_buffer->p_next ? 0 : 1);
        int piece_size = p_buffer->size + MEMP_PIECE_HEADER_SIZE;
        p_buffer  = (memp_link_t *) ((intptr_t) p_buffer + piece_size);
        mem_size -= piece_size;
    }
}
#endif

static void lib_memp_info_1(lib_memp_t *p_memp)
{
    memp_cb_t *p_memp_cb = NULL;
    memp_link_t *p_buffer = NULL;

    if (p_memp == NULL) {
        return;
    }

    p_memp_cb = (memp_cb_t *)p_memp;
    if (p_memp_cb->p_piece == NULL) {
        MLOGW("NO piece in memp, ERROR!\n");
        return;
    }

    unsigned int i = 0;
    unsigned int size = 0;
    MLOGI("All unallocated pieces:\n");
    p_buffer = p_memp_cb->p_piece;
    while (p_buffer != NULL) {
        MLOGI("piece%4d - size = %d addr = 0x%x!\n", i, p_buffer->size, p_buffer);
        size += p_buffer->size;
        p_buffer = p_buffer->p_next;
        i++;
    }

    MLOGW("Total size of unallocated pieces = %d\n", size);
}

/* adaption layer for create a memory partition */
RET_CODE lib_memp_create(lib_memp_t *p_memp, void *buffer, size_t size)
{
    memp_cb_t *p_memp_cb = NULL;
    memp_link_t *p_piece = NULL;

    if (p_memp == NULL) {
        return ERR_PARAM;
    }
    if (buffer == NULL) {
        return ERR_PARAM;
    }
    if (size <= MEMP_PIECE_HEADER_SIZE) {
        return ERR_NO_MEM;
    }

    /* initialize memory pool */
    memset(buffer, 0, size);
    p_piece = (memp_link_t *)buffer;
    p_piece->size   = size - MEMP_PIECE_HEADER_SIZE;
    p_piece->p_next = NULL;

    p_memp_cb          = (memp_cb_t *) p_memp;
    p_memp_cb->p_piece = p_piece;
#if MEMP_DEBUG
    p_memp_cb->size    = (int) size;
    p_memp_cb->address = (unsigned long) buffer;
#endif
    return SUCCESS;
}

/* adaption layer for destroy specified memory partition */
RET_CODE lib_memp_destroy(lib_memp_t *p_memp)
{
    if (p_memp == NULL) {
        return ERR_PARAM;
    }
#if MEMP_DEBUG
    lib_memp_info_1(p_memp);
    dump_allocated_mem(p_memp);
#endif
    memset(p_memp, 0, sizeof(lib_memp_t));
    return SUCCESS;
}

/* allocate buffer from specified memory partition */
void *lib_memp_alloc(lib_memp_t *p_memp, size_t size)
{
    memp_cb_t   *p_memp_cb = NULL;
    memp_link_t *p_prev    = NULL;
    memp_link_t *p_buffer  = NULL;
    memp_link_t *p_new     = NULL;
    size_t align = LIB_MEMP_SLICE_ALIGN;

    if (p_memp == NULL) {
        return NULL;
    }

    if (size == 0) {
        MLOGF("Alloc size can't be 0!%s, line:%d\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    p_memp_cb = (memp_cb_t *)p_memp;
    if (p_memp_cb->p_piece == NULL) {
        return NULL;
    }

    size = ((size + align - 1) / align) * align;
    p_prev = NULL;
    p_buffer = p_memp_cb->p_piece;
    while (p_buffer != NULL) {
        /*p_buffer size may be same as the size requested, this case I think it need be malloced*/
        if (p_buffer->size >= size) {
            /* if buffer is so big */
            if (p_buffer->size > size + MEMP_PIECE_HEADER_SIZE + MEMP_MIN_SIZE) {
                /* split buffer */
                p_new =
                    (memp_link_t *)((u8 *)p_buffer + MEMP_PIECE_HEADER_SIZE + size);
                p_new->p_next = p_buffer->p_next;
                p_new->size = p_buffer->size - size - MEMP_PIECE_HEADER_SIZE;
                p_buffer->size = size;
                if (p_prev != NULL) {
                    p_prev->p_next = p_new;
                } else {
                    p_memp_cb->p_piece = p_new;
                }
            } else {
                if (p_prev != NULL) {
                    p_prev->p_next = p_buffer->p_next;
                } else {
                    p_memp_cb->p_piece = p_buffer->p_next;
                }
            }
            p_buffer->p_next = NULL;
            MLOGA("MALLOC:%p size:%d\n", (void *)((u8 *)p_buffer + MEMP_PIECE_HEADER_SIZE), size + MEMP_PIECE_HEADER_SIZE);
            return (void *)((u8 *)p_buffer + MEMP_PIECE_HEADER_SIZE);
        }
        p_prev = p_buffer;
        p_buffer = p_buffer->p_next;
    }

    MLOGE("Alloc size = %d failed!!!\n", size);
    lib_memp_info_1(p_memp);
    return NULL;
}

/* check Repeated memory free */
static RET_CODE inline is_repeated_memory_free(
    memp_link_t *meme, memp_link_t *curr)
{
    uintptr_t memory_start = (uintptr_t) meme;
    uintptr_t memory_end   = memory_start + meme->size + MEMP_PIECE_HEADER_SIZE;

    return ((uintptr_t) curr >= memory_start &&
            (uintptr_t) curr < memory_end);
}

/* free buffer to specified memory partition */
RET_CODE lib_memp_free(lib_memp_t *p_memp, void *p_piece)
{
    memp_cb_t *p_memp_cb = NULL;
    memp_link_t *p_prev  = NULL;
    memp_link_t *p_next  = NULL;
    memp_link_t *p_curr  = NULL;

    if (p_memp == NULL || p_piece == NULL) {
        return ERR_FAILURE;
    }

    p_memp_cb = (memp_cb_t *)p_memp;
    p_curr = (memp_link_t *)((u8 *)p_piece - MEMP_PIECE_HEADER_SIZE);

    MLOGA("Free:%p size:%d\n", p_piece, p_curr->size + MEMP_PIECE_HEADER_SIZE);
    /* find insert position */
    p_prev = NULL;
    p_next = p_memp_cb->p_piece;
    while (p_next != NULL) {
        /* Check repeated memory free, don't change function order. */
        if (is_repeated_memory_free(p_next, p_curr)) {
            mt_backtrace();
            return ERR_FAILURE;
        }
        if ((p_prev < p_curr) && (p_curr < p_next)) {
            break;
        }
        p_prev = p_next;
        p_next = p_next->p_next;
    }

    /* absorb left */
    if (p_prev != NULL) {
        if (p_curr ==
            (memp_link_t *)((u8 *)p_prev + MEMP_PIECE_HEADER_SIZE + p_prev->size)) {
            p_prev->size += p_curr->size + MEMP_PIECE_HEADER_SIZE;
            p_curr = p_prev;
        } else {
            p_curr->p_next = p_prev->p_next;
            p_prev->p_next = p_curr;
        }
    } else {
        p_memp_cb->p_piece = p_curr;
        p_curr->p_next = p_next;
    }

    /* absorb right */
    if (p_next ==
        (memp_link_t *)((u8 *)p_curr + MEMP_PIECE_HEADER_SIZE + p_curr->size)) {
        p_curr->size += p_next->size + MEMP_PIECE_HEADER_SIZE;
        p_curr->p_next = p_next->p_next;
    }

    return SUCCESS;
}

void *calloc33(u32 n, u32 size)
{
    void *buf = NULL;

    FILE_SEQ_T *p_file_seq = file_seq_get_instance();

    if (p_file_seq && MT_TRUE == p_file_seq->use_ext_heap) {
        buf = p_file_seq->mem_alloc(p_file_seq, n * size);
    } else {
        buf = malloc(n * size);
    }

    if (buf) {
        memset(buf, 0x00, n * size);
    } else {
        MLOGF("%s:[ERROR][ERROR][%d] malloc memory failed size:%d!!!!!!!!!!!!!!!!!!\n", __func__, __LINE__, n * size);
    }

    return buf;
}

void *malloc33(u32 size)
{
    void *buf = NULL;

    FILE_SEQ_T *p_file_seq = file_seq_get_instance();

    if (p_file_seq && MT_TRUE == p_file_seq->use_ext_heap) {
        buf = p_file_seq->mem_alloc(p_file_seq, size);
    } else {
        buf = malloc(size);
    }

    if (buf == NULL) {
        MLOGF("%s:[ERROR][ERROR][%d] malloc memory failed size:%d!!!!!!!!!!!!!!!!!!\n", __func__, __LINE__, size);
    }
    return buf;
}

void *realloc33(void *old_ptr, u32 size)
{
    void *new_ptr = NULL;
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();

    if (p_file_seq && MT_TRUE == p_file_seq->use_ext_heap) {
        new_ptr = p_file_seq->mem_alloc(p_file_seq, size);
        if (new_ptr) {
            memset(new_ptr, 0, size);
        }
    } else {
        new_ptr = realloc(old_ptr, size);
        old_ptr = NULL;
    }

    if (new_ptr == NULL) {
        MLOGF("%s:[ERROR][ERROR][%d] malloc memory failed size:%d!!!!!!!!!!!!!!!!!!\n", __func__, __LINE__, size);
        return new_ptr;
    }

    if (old_ptr) {
        u32 tmp_size = 0;
        if (p_file_seq && MT_TRUE == p_file_seq->use_ext_heap) {
            tmp_size = ((memp_link_t *)(old_ptr - MEMP_PIECE_HEADER_SIZE))->size;
            if (tmp_size > size) {
                tmp_size = size;
            }
            memcpy(new_ptr, old_ptr, tmp_size);
        }

        if (p_file_seq && MT_TRUE == p_file_seq->use_ext_heap) {
            p_file_seq->mem_free(p_file_seq, old_ptr);
        }
    }

    return new_ptr;
}

void free33(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    if (p_file_seq && MT_TRUE == p_file_seq->use_ext_heap) {
        p_file_seq->mem_free(p_file_seq, ptr);
    } else {
        free(ptr);
    }
}

char *strdup33(const char *s)
{
    char *new_string = NULL;

    if (s == NULL) {
        return NULL;
    }

    new_string = (char *)malloc33(strlen(s) + 1);
    if (new_string) {
        memset(new_string, 0, strlen(s) + 1);
        strcpy(new_string, s);
    }

    return new_string;
}
