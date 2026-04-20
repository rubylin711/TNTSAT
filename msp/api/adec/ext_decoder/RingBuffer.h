/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
 *  Allen.Wang, 2018/01/01
*/
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
#ifdef __cplusplus
extern "C"
{
#endif

#define TMS_MAX_CNT       (64)
#define RB_BLOCK_SIZE     (128*1024)//(682*192)
#define RB_FLAG_WR  0   //ring buffer write flag
#define RB_FLAG_RD  1   //ring buffer read flag

//p : pointer, s : size
#define RB_MUTEX_TYPE_T        pthread_mutex_t
#define RB_MUTEX_LOCK(p)       pthread_mutex_lock(p)
#define RB_MUTEX_UNLOCK(p)     pthread_mutex_unlock(p)
#define RB_MUTEX_DESTROY(p)    pthread_mutex_destroy(p)
#define RB_MUTEX_INIT(p)       pthread_mutex_init(p,NULL)
#define RB_MALLOC(s)           malloc(s)
#define RB_FREE(p)             free(p)
//#define RB_PRINT(fmt, args...) printf(fmt, ##args)
#define RB_PRINT(fmt, args...) do{}while(0)

typedef unsigned int guint32;
typedef unsigned short guint16;
typedef int gboolean;
typedef unsigned long long guint64;
typedef int gint;
typedef unsigned char guint8;
typedef char gint8;

//#define INVALID_TMS (-1)
#define INVALID_TMS 0xFFFFFFFFFFFFFFFF
#define RB_FAILURE  (-1)
#define RB_SUCCESS  (0)

#ifdef __GNUC__
#define rb_likely(p)   __builtin_expect(!!(p), 1)
#define rb_unlikely(p) __builtin_expect(!!(p), 0)
#else
#define rb_likely(p)   (!!(p))
#define rb_unlikely(p) (!!(p))
#endif

/*   ---- RAW_DATA_RING_BUFFER_T ----
    start : the memory start address of the ring buffer
    end : the memory end address of the ring buffer
    rd : read position for ring buffer
    wr : write index for ptms_buf write
    size : the size of the ring buffer (Bytes)
    ext_size : used for data continuous when ring buffer rollback
    cnt : the valid data size of the ring buffer(Bytes)
    mutex : for write and read
    < for example: >
    ring buffer ext_size : 256
    ring buffer start : 0x80000000
    ring buffer end :   0x80003fff
    ring buffer size :  0x4000
    ring buffer cnt :   (0x80002200 - 0x80001000)
    ______ _____________________________________________________________
   |_ext__|_______________|____________________|________________________|
         start            rd                   wr                      end
       0x80000000     0x80001000           0x80002200         (0x80004000 - 0x1)
*/
typedef struct _ring_buffer //ring buffer for elementary stream
{
    guint8 *start_ext;
    guint8 *start;
    guint8 *end;
    guint8 *rd;
    guint8 *wr;
    guint32 cnt;
    guint32 size_ext;
    guint32 size;

    RB_MUTEX_TYPE_T mutex;
}RAW_DATA_RING_BUFFER_T;

typedef struct _frame_info_struct
{
    guint64 tms;
    guint8 *pos;
    guint32 size;
}FRM_INFO_T;

typedef struct _ring_buffer_tms //ring buffer for timestamp
{
    guint32 idx_rd;
    guint32 idx_wr;
    guint32 cnt;
    guint32 size;
    FRM_INFO_T *ptfrm;

    RB_MUTEX_TYPE_T mutex;
}TMS_DATA_RING_BUFFER_T;

typedef struct _media_data_ring_buffer
{
    guint32 used;
    RAW_DATA_RING_BUFFER_T raw_rb;
    TMS_DATA_RING_BUFFER_T tms_rb;
}MEDIA_DATA_RING_BUFFER_T;

/*   ---- RING_BUFFER_PLUGIN_T ----
    an universal plugin for ring buffer operation.

    [initialize] : intialize a media ring buffer by a raw data buffer length and
    a time stamp array count.
    [is_enough] : check whether it will be successfull for reading/writing ring
    buffer or not.
    [write] : write data to raw data ring buffer and record a time stamp to time
    stamp ring buffer.
    [read] :  read data from raw data ring buffer and get a time stamp from time
    stamp ring buffer.
    [finalize] : finalize the media ring buffer
*/
typedef struct _rb_plugin
{
    gboolean (*initialize)(MEDIA_DATA_RING_BUFFER_T *m_rb, guint32 raw_data_len,
        guint32 ext_data_len, guint32 tms_num);
    gboolean (*is_enough)(MEDIA_DATA_RING_BUFFER_T *m_rb, guint32 len,
        guint8 flag, guint32 *real_len, guint8 **addr);
    gboolean (*validate)(MEDIA_DATA_RING_BUFFER_T *m_rb, guint8 flag, guint32 size);
    gint (*write)(MEDIA_DATA_RING_BUFFER_T *m_rb, const guint8 *src,
        guint32 len, gboolean do_copy, guint64 tms);
    gint (*read)(MEDIA_DATA_RING_BUFFER_T *m_rb, guint8 *dst,
        guint32 len, gboolean do_copy, guint32 *real_len, guint64 *tms);
    gboolean (*finalize)(MEDIA_DATA_RING_BUFFER_T *m_rb);
    gboolean (*reset)(MEDIA_DATA_RING_BUFFER_T *m_rb);
}RING_BUFFER_PLUGIN_T;

RING_BUFFER_PLUGIN_T *get_rb_plugin(void);

typedef struct _rb_class
{
    RING_BUFFER_PLUGIN_T *plgrb;
    MEDIA_DATA_RING_BUFFER_T *prb;
}RING_BUFFER_CLASS_T;
typedef int (*pReadFunc)(void *, unsigned char *, int);//priv, buffer, size
typedef int (*pWriteFunc)(void *, unsigned char *, int);//priv, buffer, size
#ifdef __cplusplus
}
#endif

#endif

