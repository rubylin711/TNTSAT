/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_FILE_CACHE_H__
#define __MT_PVR_FILE_CACHE_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */


/**  base macro definition **************************************************************/

/* index file parameter macro */
#define PVR_FC_IDX_READ_BUF_SIZE    (1024*8)    /* read buffer size */
#define PVR_FC_IDX_READ_BUF_MIN     3           /* the min read buffer num */
#define PVR_FC_IDX_READ_BUF_MAX     32          /* the max read buffer num */
#define PVR_FC_IDX_READ_ONCE_MAX    (1024*8)    /* the max read byte once */
#define PVR_FC_IDX_WRITE_BUF_SIZE   (1024*8)    /* write buffer size */
#define PVR_FC_IDX_WRITE_BUF_MIN    3           /* the min write buffer num */
#define PVR_FC_IDX_WRITE_BUF_MAX    32          /* the max write buffer num */
#define PVR_FC_IDX_WRITE_ONCE_MAX   (1024*8)    /* the max write byte once */
#define PVR_FC_IDX_WRITE_WATERLINE  (1024*5)    /* start write line */
#define PVR_FC_IDX_WRITE_TIMEOUT    1000        /* time out for writing, in millisecond */
#define PVR_FC_IDX_SYNC_TIMEOUT     0000        /* time out for sync */

/* data file parameter macro */
#define PVR_FC_DAT_READ_BUF_SIZE    (1024*256)  /* data file: read buffer size */
#define PVR_FC_DAT_READ_BUF_MIN     3           /* data file: the min read buffer num */
#define PVR_FC_DAT_READ_BUF_MAX     8           /* data file: the max read buffer num */
#define PVR_FC_DAT_READ_ONCE_MAX    (1024*64)   /* data file: the max read byte once */
#define PVR_FC_DAT_WRITE_BUF_SIZE   (1024*128)  /* data file: write buffer size */
#define PVR_FC_DAT_WRITE_BUF_MIN    3           /* data file: the min write buffer num */
#define PVR_FC_DAT_WRITE_BUF_MAX    32          /* data file: the max write buffer num */
#define PVR_FC_DAT_WRITE_ONCE_MAX   (1024*64)   /* data file: the max write byte once */
#define PVR_FC_DAT_WRITE_WATERLINE  (1024*45)   /* data file: start write line */
#define PVR_FC_DAT_WRITE_TIMEOUT    1000        /* data file: time out for writing, in millisecond */
#define PVR_FC_DAT_SYNC_TIMEOUT     0000        /* data file: time out for sync */

#define pvr_open                            open
#define pvr_close                           close
#define pvr_pread                           pread
#define pvr_pwrite                          pwrite
#define pvr_read                            read
#define pvr_write                           write
#define pvr_lseek                           lseek
#define pvr_remove                          remove
#define pvr_fsync                           fsync
#define pvr_truncate                        truncate
#define pvr_mount                           mount
#define pvr_umount                          umount
#define pvr_FILE                            FILE
#define pvr_fopen                           fopen
#define pvr_fclose                          fclose
#define pvr_fread                           fread
#define pvr_fwrite                          fwrite
#define pvr_fseek                           fseek
#define pvr_ftell                           ftell
#define pvr_fflush                          fflush
#define pvr_feof                            feof
#define pvr_ferror                          ferror




/** base function ************************************************************/

extern int PVR_FC_open(const char *pathname, int flags, mode_t mode);
extern int PVR_FC_close(int fildes);
extern ssize_t PVR_FC_pread(int fildes, void *buf, size_t nbyte, off_t offset);
extern ssize_t PVR_FC_read(int fildes, void *buf, size_t nbyte);
extern ssize_t PVR_FC_pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
extern ssize_t PVR_FC_write(int fildes, const void *buf, size_t nbyte);
extern off_t PVR_FC_lseek(int fildes, off_t offset, int whence);
extern int PVR_FC_remove(const char *pathname);
extern int PVR_FC_fsync(int fildes);
extern int PVR_FC_truncate(const char *path, off_t length);
extern pvr_FILE *PVR_FC_fopen(const char *path, const char *mode);
extern int PVR_FC_fclose(pvr_FILE *fp);
extern size_t PVR_FC_fread(void *ptr, size_t size, size_t nmemb, pvr_FILE *stream);
extern size_t PVR_FC_fwrite(const void *ptr, size_t size, size_t nmemb, pvr_FILE *stream);
extern int PVR_FC_fseek(pvr_FILE *stream, long offset, int whence);
extern long PVR_FC_ftell(pvr_FILE *stream);
extern int PVR_FC_fflush(pvr_FILE *stream);
extern int PVR_FC_feof(pvr_FILE *stream);
extern int PVR_FC_ferror(pvr_FILE *stream);
extern off_t PVR_FC_fsize(pvr_FILE *stream);
extern off_t PVR_FC_size(int fildes);
extern int PVR_FC_mount(const char *source, const char *target,
                 const char *filesystemtype, unsigned long mountflags,
                 const void *data);
extern int PVR_FC_umount(const char *target);
/*-----------------------------
    filesystemtype:
        "vfat"
        "ntfs"
------------------------------*/

/**  pre read function ************************************************************/

extern MT_VOID PVR_FC_OpenAutoPreread(pvr_FILE *stream, int fildes);
extern MT_VOID PVR_FC_CloseAutoPreread(pvr_FILE *stream, int fildes);
extern MT_VOID PVR_FC_OpenDatAutoPreread();
extern MT_VOID PVR_FC_CloseDatAutoPreread();
extern MT_VOID PVR_FC_ClearUserPreread(pvr_FILE *stream, int fildes);
extern MT_VOID PVR_FC_UserPreread(pvr_FILE *stream, int fildes, off_t offset, ssize_t size, MT_BOOL bClear, MT_BOOL bEnd);

#define PVR_FC_OPEN_AUTO_PREREAD(fildes)    PVR_FC_OpenAutoPreread(MT_NULL, (fildes))
#define PVR_FC_CLOSE_AUTO_PREREAD(fildes)   PVR_FC_CloseAutoPreread(MT_NULL, (fildes))
#define PVR_FC_CLEAR_USER_PREREAD(fildes)   PVR_FC_ClearUserPreread(MT_NULL, (fildes))
#define PVR_FC_USER_PREREAD(fildes, offset, size, clear, end)   PVR_FC_UserPreread(MT_NULL, (fildes), (off_t)(offset), (ssize_t)(size), (clear), (end))


/**  attribute control interface************************************************************/

typedef enum tagPVR_FC_FILE_TYPE_E
{
    PVR_FC_FILE_TYPE_IDX,   /* index file */
    PVR_FC_FILE_TYPE_DAT,   /* data file */
    PVR_FC_FILE_TYPE_OTHER
} PVR_FC_FILE_TYPE_E;

typedef struct tagPVR_FC_ATTR_S
{
    MT_U32 u32ReadBufSize;
    MT_U32 u32ReadBufMin;
    MT_U32 u32ReadBufMax;
    MT_U32 u32ReadOnceMax;
    MT_U32 u32WriteBufSize;
    MT_U32 u32WriteBufMin;
    MT_U32 u32WriteBufMax;
    MT_U32 u32WriteOnceMax;
    MT_U32 u32WriteWaterline;
    MT_U32 u32WriteTimeout;
    MT_BOOL bAutoPreread;
    MT_U32 u32SyncTimeout;
    PVR_FC_FILE_TYPE_E enFileType;
    MT_U32 u32Pad;
} PVR_FC_ATTR_S;

extern int PVR_FC_GetAttr(pvr_FILE *stream, int fildes, PVR_FC_ATTR_S *pstAttr);
extern int PVR_FC_SetAttr(pvr_FILE *stream, int fildes, PVR_FC_ATTR_S *pstAttr);

#define PVR_FC_SET_IDX_ATTR(fildes)     \
    do {                                \
        PVR_FC_ATTR_S stAttr = {        \
            PVR_FC_IDX_READ_BUF_SIZE,   \
            PVR_FC_IDX_READ_BUF_MIN,    \
            PVR_FC_IDX_READ_BUF_MAX,    \
            PVR_FC_IDX_READ_ONCE_MAX,   \
            PVR_FC_IDX_WRITE_BUF_SIZE,  \
            PVR_FC_IDX_WRITE_BUF_MIN,   \
            PVR_FC_IDX_WRITE_BUF_MAX,   \
            PVR_FC_IDX_WRITE_ONCE_MAX,  \
            PVR_FC_IDX_WRITE_WATERLINE, \
            PVR_FC_IDX_WRITE_TIMEOUT,   \
            MT_TRUE,                    \
            PVR_FC_IDX_SYNC_TIMEOUT,    \
            PVR_FC_FILE_TYPE_IDX };     \
        PVR_FC_SetAttr(MT_NULL, (fildes), &stAttr); \
    } while (0)

#define PVR_FC_SET_DAT_ATTR(fildes)     \
    do {                                \
        PVR_FC_ATTR_S stAttr = {        \
            PVR_FC_DAT_READ_BUF_SIZE,   \
            PVR_FC_DAT_READ_BUF_MIN,    \
            PVR_FC_DAT_READ_BUF_MAX,    \
            PVR_FC_DAT_READ_ONCE_MAX,   \
            PVR_FC_DAT_WRITE_BUF_SIZE,  \
            PVR_FC_DAT_WRITE_BUF_MIN,   \
            PVR_FC_DAT_WRITE_BUF_MAX,   \
            PVR_FC_DAT_WRITE_ONCE_MAX,  \
            PVR_FC_DAT_WRITE_WATERLINE, \
            PVR_FC_DAT_WRITE_TIMEOUT,   \
            MT_TRUE,                    \
            PVR_FC_DAT_SYNC_TIMEOUT,    \
            PVR_FC_FILE_TYPE_DAT };     \
        PVR_FC_SetAttr(MT_NULL, (fildes), &stAttr); \
    } while (0)


/**  debug function info *******************************************************/

typedef struct tagPVR_FC_STAT_INFO_S
{
    MT_U32 u32OpenNodeCount;
    MT_U32 u32CloseNodeCount;
    MT_U32 u32OpenFileCount;
    MT_U32 u32CloseFileCount;
    MT_U32 u32UsedMemoryMax;
    MT_U32 u32UsedMemoryCurrent;
    MT_S64 s64ReadUserDataTotal;
    MT_S64 s64ReadFileDataInUIT;
    MT_S64 s64WriteUserDataTotal;
    MT_S64 s64WriteFileDataInUIT;
    MT_S64 s64ReadTime;     /* in nanosecond */
    MT_S64 s64WriteTime;    /* in nanosecond */
    MT_S64 s64ReadBytes;
    MT_S64 s64WriteBytes;
    MT_S64 s64RunTime;      /* total run time */
    MT_S64 s64FOTRunTimeSum;   /* all the file thread run total time */
    MT_S64 s64FOTWaitTimeSum;  /* all the file thread wait total time */
    MT_S64 s64UITRunTimeSum;   /* all the user interface thread run total time */
    MT_S64 s64UITWaitTimeSum;  /* all the user interface thread wait total time */
    MT_S64 s64UITSendTimeSum;  /* the total time of all the user interface thread send msg to file thread */
    MT_U32 u32UITSendTimePercentMax;
    MT_BOOL bOpenDatAutoPreread;
} PVR_FC_STAT_INFO_S;

MT_VOID PVR_FC_GetStatInfo(PVR_FC_STAT_INFO_S *pstInfo);
MT_VOID PVR_FC_ClearStatInfo(MT_VOID);
MT_VOID PVR_FC_OpenTime(MT_VOID);
int PVR_FC_Test(int argc, char *argv[]);

/**  other interface ****************************************************************/

MT_U64 PVR_FC_GetCurrentTime(MT_VOID);
int PVR_FC_CreateThread(pthread_t * thread, int detached, void *(*start_routine)(void*), void * arg);
int PVR_FC_PthreadJoin(pthread_t thread, MT_VOID **thread_return);

typedef struct tagPVR_FC_EVENT_S
{
    pthread_mutex_t stLock;   
    struct list_head stThreadLink;
    MT_BOOL bState;
    MT_BOOL bManualReset;
    MT_U32 u32WaitCount;
    MT_U32 u32Pad;
} PVR_FC_EVENT_S;
MT_S32 PVR_FC_EventInit(PVR_FC_EVENT_S * pstEvent, MT_BOOL bManualReset, MT_BOOL bInitialState);
MT_S32 PVR_FC_EventReinit(PVR_FC_EVENT_S * pstEvent);
MT_S32 PVR_FC_EventSet(PVR_FC_EVENT_S * pstEvent, MT_BOOL bState);
MT_S32 PVR_FC_EventWait(PVR_FC_EVENT_S * pstEvent, MT_U32 u32Timeout);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifdef __MT_PVR_FILE_CACHE_H__ */


