/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "string.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mtos_mem.h"
#include "mtos_sem.h"
#include "mtos_task.h"
#include "fifo_kw.h"
#include "ts_sequence.h"

#define SUBT_BUF_SIZE_MAX 32*1024  //tizhang@20180822 for 104045

static void reset_kw_fifo(fifo_type_t *fifo)
{
    fifo->wr_pos  = fifo->start_pos;
    fifo->rd_pos  = fifo->start_pos;
    fifo->end_pos = fifo->start_pos + fifo->len;
}

void *init_fifo_kw(char *p_fifo, int fifoSize)
{
    fifo_type_t *p_ts_fifo = NULL;
    MT_BOOL ret = MT_FALSE;

#ifdef WIN32
    p_ts_fifo = malloc(sizeof(fifo_type_t));
#else
    p_ts_fifo = (fifo_type_t *) mtos_malloc(sizeof(fifo_type_t));
#endif
    if (NULL == p_ts_fifo) {
        return NULL;
    }

    memset(p_ts_fifo, 0, sizeof(fifo_type_t));
    if (p_ts_fifo->start_pos == 0) {
        p_ts_fifo->len = fifoSize;
        if (NULL == p_fifo) {
            p_ts_fifo->mem_malloc = MT_TRUE;
#ifdef WIN32
            p_ts_fifo->start_pos = (uintptr_t) malloc(p_ts_fifo->len);
#else
            p_ts_fifo->start_pos = (uintptr_t) mtos_malloc(p_ts_fifo->len);
#endif
        } else {
            p_ts_fifo->mem_malloc = MT_FALSE;
            p_ts_fifo->start_pos = (uintptr_t)p_fifo;
        }
        if (p_ts_fifo->start_pos == 0) {
            mtos_free((void *)(p_ts_fifo));
            return NULL;
        }
        memset((char *)p_ts_fifo->start_pos, 0, p_ts_fifo->len);
    }

    reset_kw_fifo(p_ts_fifo);
#ifdef WIN32
    p_ts_fifo->mutex = (unsigned int)SDL_CreateMutex();
#else
    ret = mtos_sem_create(&(p_ts_fifo->mutex), MT_TRUE);
    MT_ASSERT(ret == MT_TRUE);
#endif

    return (void *)(p_ts_fifo);
}

void deinit_fifo_kw(void *handle)
{
    fifo_type_t *p_ts_fifo = (fifo_type_t *)handle;

    if (p_ts_fifo->start_pos && p_ts_fifo->mem_malloc) {
#ifdef WIN32
        free((char *)(p_ts_fifo->start_pos));
#else
        mtos_free((void *)(p_ts_fifo->start_pos));
#endif
        p_ts_fifo->start_pos = 0;
    }

#ifdef WIN32
    SDL_DestroyMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
    mtos_sem_destroy(&(p_ts_fifo->mutex), 0);
#endif

#ifdef WIN32
    free(p_ts_fifo);
    p_ts_fifo = NULL;
#else
    mtos_free((void *)(p_ts_fifo));
#endif
}

void clear_sub_fifo_kw(void *p_handle)
{
    fifo_type_t *p_ts_fifo = (fifo_type_t *)p_handle;

#ifdef WIN32
    SDL_LockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
    mtos_sem_take((os_sem_t *)(&(p_ts_fifo->mutex)), 0);
#endif

    reset_kw_fifo(p_ts_fifo);
#ifdef WIN32
    SDL_UnlockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
    mtos_sem_give((os_sem_t *)(&(p_ts_fifo->mutex)));
#endif
    return;
}

int read_sub_fifo_kw(void   *p_handle, char *p_data)
{
    fifo_type_t  *p_ts_fifo = (fifo_type_t *)p_handle;
    unsigned long available = 0;
    int read_size = 0;
    int temp = 0;
    int overflow = 0;//tizhang@20180822 for 104045

    unsigned char *p_data2 = (unsigned char *)p_data;

#ifdef WIN32
    SDL_LockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
    mtos_sem_take((os_sem_t *)(&(p_ts_fifo->mutex)), 0);
#endif

    if (p_ts_fifo->rd_pos < p_ts_fifo->wr_pos)
    {
        available = p_ts_fifo->wr_pos - p_ts_fifo->rd_pos;
    }
    else if (p_ts_fifo->rd_pos > p_ts_fifo->wr_pos)
    {
        available = (p_ts_fifo->end_pos  - p_ts_fifo->start_pos)
                    - (p_ts_fifo->rd_pos - p_ts_fifo->wr_pos);
    }
    else
    {
        available = 0;
    }

    if (available > 0)
    {
        memcpy(p_data2, (char *)(p_ts_fifo->rd_pos), 4);

        read_size = (p_data2[0] << 24) + (p_data2[1] << 16)
                    + (p_data2[2] << 8) + (p_data2[3]) + 8;
         if(SUBT_BUF_SIZE_MAX < read_size){//tizhang@20180822 for 104045
             overflow = 1;
         }
         
         if(p_ts_fifo->rd_pos + read_size > p_ts_fifo->end_pos) 
          {
            temp = p_ts_fifo->end_pos - p_ts_fifo->rd_pos;
              if(overflow == 0)
            memcpy(p_data2, (char *)(p_ts_fifo->rd_pos), temp);
            p_ts_fifo->rd_pos = p_ts_fifo->start_pos;
              if(overflow == 0)
            memcpy(p_data2 + temp, (char *)(p_ts_fifo->rd_pos), read_size - temp);
            p_ts_fifo->rd_pos += (read_size - temp);
          }
         else
          {
              if(overflow == 0)
            memcpy(p_data2, (char *)(p_ts_fifo->rd_pos), read_size);
            p_ts_fifo->rd_pos += read_size;
              if(p_ts_fifo->rd_pos == p_ts_fifo->end_pos)
                p_ts_fifo->rd_pos = p_ts_fifo->start_pos;
            }
        }
    

#ifdef WIN32
    SDL_UnlockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
    mtos_sem_give((os_sem_t *)(&(p_ts_fifo->mutex)));
#endif
    return read_size;
}

/*
*  PARAM[1]:  p_data:   point to memory buffer where data will be put to this queue
*  PARAM[2]: size  :      the length of data which will be moved to queue
*  RETURN VALUE:
*/
int write_sub_fifo_kw(void *p_handle, unsigned char *p_data, long size)
{

    long free_size = 0;
    long temp = 0;

    fifo_type_t  *p_ts_fifo = (fifo_type_t *)p_handle;

    long totalSize = size;
    long needWrNum = 0;
    unsigned char  *p_cur_data = (unsigned char *)p_data;

    MT_ASSERT(size >= 0);
    if (totalSize > p_ts_fifo->len) {
        needWrNum = p_ts_fifo->len;
    } else {
        needWrNum = totalSize;
    }
    do {

#ifdef WIN32
        SDL_LockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
        mtos_sem_take((os_sem_t *)(&(p_ts_fifo->mutex)), 0);
#endif

        if (p_ts_fifo->wr_pos > p_ts_fifo->rd_pos) {
            free_size = (p_ts_fifo->end_pos - p_ts_fifo->start_pos)
                        - (p_ts_fifo->wr_pos - p_ts_fifo->rd_pos);
        } else if (p_ts_fifo->wr_pos < p_ts_fifo->rd_pos) {
            free_size = p_ts_fifo->rd_pos - p_ts_fifo->wr_pos;
        } else {
            free_size = p_ts_fifo->end_pos - p_ts_fifo->start_pos;
            p_ts_fifo->wr_pos = p_ts_fifo->start_pos;
            p_ts_fifo->rd_pos = p_ts_fifo->start_pos;
        }

        if (free_size > needWrNum) {
            if (p_ts_fifo->wr_pos == p_ts_fifo->end_pos) {
                p_ts_fifo->wr_pos = p_ts_fifo->start_pos;
            }

            if (p_ts_fifo->wr_pos + needWrNum > p_ts_fifo->end_pos) {
                temp = p_ts_fifo->end_pos - p_ts_fifo->wr_pos;
                memcpy((char *)(p_ts_fifo->wr_pos), p_cur_data, temp);
                p_ts_fifo->wr_pos = p_ts_fifo->start_pos;
                memcpy((char *)(p_ts_fifo->wr_pos), p_cur_data + temp, needWrNum - temp);
                p_ts_fifo->wr_pos += (needWrNum - temp);
            } else {
                // mtos_printk("[%s]  ,[0x%x]  ,[0x%x] ",__func__,p_cur_data[3],p_cur_data[7]);
                memcpy((char *)(p_ts_fifo->wr_pos), p_cur_data, needWrNum);
                p_ts_fifo->wr_pos += needWrNum;
            }

            //temp = needWrNum;
            totalSize -= needWrNum;

            if (totalSize > p_ts_fifo->len) {
                needWrNum = p_ts_fifo->len;
            } else if (totalSize > 0 && totalSize <= p_ts_fifo->len) {
                needWrNum = totalSize;
            }

#ifdef WIN32
            SDL_UnlockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
            mtos_sem_give((os_sem_t *)(&(p_ts_fifo->mutex)));
#endif

        } else {
#ifdef WIN32
            SDL_UnlockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
            mtos_sem_give((os_sem_t *)(&(p_ts_fifo->mutex)));
#endif
            break;
        }
    } while (totalSize > 0);

    return size;
}

/*
*
*  RETURN VALUE:
*/
int tell_fifo_kw(void *p_handle)
{
    ts_seq_t  *p_ts_seq = (ts_seq_t *)(p_handle);
    fifo_type_t  *p_ts_fifo = (fifo_type_t *)p_ts_seq->p_TsQueue;
    unsigned long available = 0;

    if (p_ts_seq->isExit || p_ts_fifo == NULL) {
        return 0;
    }

#ifdef WIN32
    SDL_LockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
    mtos_sem_take((os_sem_t *)(&(p_ts_fifo->mutex)), 0);
#endif

    if (p_ts_fifo->rd_pos < p_ts_fifo->wr_pos) {
        available = p_ts_fifo->wr_pos - p_ts_fifo->rd_pos;
    } else if (p_ts_fifo->rd_pos > p_ts_fifo->wr_pos) {
        available = (p_ts_fifo->end_pos  - p_ts_fifo->start_pos)
                    - (p_ts_fifo->rd_pos - p_ts_fifo->wr_pos);
    } else {
        available = 0;
    }

#ifdef WIN32
    SDL_UnlockMutex((SDL_mutex *)p_ts_fifo->mutex);
#else
    mtos_sem_give((os_sem_t *)(&(p_ts_fifo->mutex)));
#endif

    if (p_ts_seq->isExit) {
        return 0;
    }

    return available;
}
