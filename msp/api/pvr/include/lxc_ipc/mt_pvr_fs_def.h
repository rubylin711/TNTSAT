/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_LXCIPC_H__
#define __MT_PVR_LXCIPC_H__

#include "mt_type.h"

#if MT_OS_TYPE == MT_OS_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <fcntl.h>

#include "mt_type.h"
#include "mt_drv_pvr.h"
#include "mt_pvr_cipher_cfg.h"

#include "mt_unf_ipcfs_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

#define PVR_FOPEN_MODE_CADATA_TRUNC   (O_CREAT | O_RDWR | O_LARGEFILE  | O_TRUNC )
#define PVR_FOPEN_MODE_CADATA_BOTH   (O_CREAT | O_RDWR | O_LARGEFILE  | O_APPEND)
#define PVR_FOPEN_MODE_INDEX_BOTH   (O_CREAT | O_RDWR|  O_LARGEFILE/*| O_APPEND*/ )
#define PVR_FOPEN_MODE_INDEX_WRITE  (O_CREAT | O_RDWR| O_LARGEFILE /*| O_APPEND*/ )
#define PVR_FOPEN_MODE_INDEX_READ   (O_RDONLY  | O_LARGEFILE )

#define PVR_FOPEN_MODE_DATA_BOTH    (O_CREAT | O_RDWR | O_LARGEFILE /* | O_APPEND*/  )
#define PVR_FOPEN_MODE_DATA_WRITE   (O_CREAT | O_RDWR | O_LARGEFILE /* | O_APPEND*/ )
#define PVR_FOPEN_MODE_DATA_READ    (          O_RDWR | O_LARGEFILE)

#define PVR_FOPEN_MODE_DATA_BOTH_DIO    (PVR_FOPEN_MODE_DATA_BOTH  | O_DIRECT /*| O_SYNC | O_APPEND*/)
#define PVR_FOPEN_MODE_DATA_WRITE_DIO   (PVR_FOPEN_MODE_DATA_WRITE | O_DIRECT /*| O_SYNC | O_APPEND*/) //disable O_SYNC for write performance


/* PVR file descriptor                                                      */
typedef int            PVR_FILE;
typedef int            PVR_FILE64;

/* invalid file description                                                 */
#define PVR_FILE_INVALID_FILE             (-1)

#if 0
#define pvr_open                            open
#define pvr_close                           close
#define pvr_pread                           pread
#define pvr_pwrite                          pwrite
#define pvr_lseek                           lseek
#define pvr_read                            read
#define pvr_write                           write
#define pvr_fsync                           fsync
#else
#define pvr_open                            mt_pvr_ipc_open
#define pvr_close                           mt_pvr_ipc_close
#define pvr_pread                           mt_pvr_ipc_pread
#define pvr_pwrite                          mt_pvr_ipc_pwrite
#define pvr_lseek                           mt_pvr_ipc_lseek
#define pvr_read                            mt_pvr_ipc_read
#define pvr_write                           mt_pvr_ipc_write
#define pvr_fsync                           mt_pvr_ipc_fsync
#define pvr_access							mt_pvr_ipc_access
#define pvr_remove							mt_pvr_ipc_remove
#define pvr_fadvise							mt_pvr_ipc_fadvise
#define pvr_rename                          mt_pvr_ipc_rename
#endif

#define PVR_FC_open         pvr_open
#define PVR_FC_close        pvr_close
#define PVR_FC_pread        pvr_pread
#define PVR_FC_read         pvr_read
#define PVR_FC_pwrite       pvr_pwrite
#define PVR_FC_write        pvr_write
#define PVR_FC_lseek        pvr_lseek
#define PVR_FC_fsync        pvr_fsync
#define PVR_FC_fadvise      pvr_fadvise
#define PVR_FC_rename       pvr_rename
#define PVR_FC_remove       pvr_remove

extern int PVR_Mount(const char *source, const char *target,
                 const char *filesystemtype, unsigned long mountflags,
                 const void *data);
extern int PVR_Umount(const char *target);
/*-----------------------------
    filesystemtype:
        "vfat"
        "ntfs"
------------------------------*/

#define PVR_OPEN(filename, mode)            PVR_FC_open(filename, mode, 0777)

#define PVR_READ(pMem, size, file, offset)  PVR_FC_pread(file, (void *)pMem, (size_t)(size), (off_t)(offset))
#define PVR_WRITE(pMem, size, file, offset) PVR_FC_pwrite(file, pMem, size, (off_t)(offset))
#define PVR_SEEK(file, offset, whence)      PVR_FC_lseek(file, (offset), whence)
#define PVR_READ_C(file, pMem, size)        PVR_FC_read(file, (void *)(pMem), (size_t)(size))
#define PVR_WRITE_C(file, pMem, size)       PVR_FC_write(file, (void *)(pMem), (size_t)(size))
#define PVR_CLOSE(file)                     PVR_FC_close(file)
#define PVR_FSYNC(file)                     PVR_FC_fsync(file)
#define PVR_FADVISE                         PVR_FC_fadvise
#define PVR_RENAME                          PVR_FC_rename
#define PVR_REMOVE                          PVR_FC_remove

extern ssize_t PVR_READALL(void *buf, size_t count, PVR_FILE fd,  off64_t offset);
extern ssize_t PVR_WRITEALL(const void *buf, size_t count, PVR_FILE fd, off64_t offset);

extern MT_BOOL PVR_CHECK_FILE_EXIST(const MT_CHAR *pszFileName);
extern MT_U64 PVR_FILE_GetFileSize(const MT_CHAR *pszFileName);
extern MT_VOID PVR_SET_MAXFILE_SIZE(PVR_FILE64 file, MT_U64 maxSize);


//////////////////////////////////////////////////////////////////
/****************************************************************/
/*                PVR FILE for >4G file @ FAT32                 */
/****************************************************************/
//////////////////////////////////////////////////////////////////
#define PVR_FILE_MAX_FILE          10 /* 5 for read + 5 for write */

#define PVR_FILE_FD_BASE           100

#define PVRFileGetPVRFd(fd)  (fd - PVR_FILE_FD_BASE)


#define PVRFileCheckPVRFd(pvrFd)  do\
    {\
        if (pvrFd >= PVR_FILE_MAX_FILE)\
        {\
            return -1; \
        }\
    }while(0)

#define PVRFileCheckPVRFdOpen(pvrFd)  do\
    {\
        if (g_stPvrFiles[pvrFd].bOpened != MT_TRUE)\
        {\
            return -1; \
        }\
    }while(0)


extern MT_BOOL PVR_CHECK_FILE_EXIST64(const MT_CHAR *pszFileName);
extern MT_VOID PVR_REMOVE_FILE64(const MT_CHAR *pszFileName);
extern MT_U64  PVR_FILE_GetFileSize64(const MT_CHAR *pszFileName);

extern PVR_FILE64 PVR_OPEN64(const MT_CHAR *filename, int mode,
                                        MT_BOOL bSupportTimeShiftEvent,
                                        MT_U32 u32TimeShiftEventDataUnitSize,
                                        MT_U32 u32TimeShiftEventLoopTimeInMs,
                                        MT_U32 node);
extern MT_S32  PVR_CLOSE64(PVR_FILE64 file, MT_BOOL bTimeShiftEvent);
extern MT_S32  PVR_FSYNC64(PVR_FILE64 file);
extern ssize_t PVR_PREAD64(MT_U8 *pMem, MT_U32 size, PVR_FILE64 file, MT_U64 offset);
extern ssize_t PVR_TS_PREAD64(MT_U8 *pMem, MT_U32 size, PVR_FILE64 file, MT_U64 offset);
//extern ssize_t PVR_PWRITE64(const void *pMem,  MT_U32 size, PVR_FILE64 file, MT_U64 offset);

extern MT_S32  PVR_GetTsFileFd(PVR_FILE64 file);

extern ssize_t PVR_READ64(MT_U8 *pMem, MT_U32 size, PVR_FILE64 file);
extern ssize_t PVR_WRITE64(const void *pMem,  MT_U32 size, PVR_FILE64 file);
extern ssize_t PVR_SEEK64(PVR_FILE64 file, mt_s64 offset, int whence);

MT_S32 PVRFileGetOffsetFName(PVR_FILE64 file, MT_U64 offset, MT_CHAR *pRealName);
//MT_S32 PVRFileGetRealOffset(PVR_FILE64 file, MT_U64 *offset);
MT_S32 PVRFileGetRealOffset(PVR_FILE64 file, MT_U64 *offset, MT_U64 *pu64OffsetLen);

extern void mt_pvr_ipc_clear_tid(void);

extern void* mt_pvr_ipc_getbufferby_tid(int fd,int size,off_t *in_phy);
extern int mt_pvr_ipc_open(const char *path, int flag,unsigned int mode);
extern int mt_pvr_ipc_close(int fd);
extern int mt_pvr_ipc_pread(int fd,void *buff,size_t size,off_t offset);
extern int mt_pvr_ipc_pwrite(int fd,void *buff,size_t size,off_t offset);
extern int mt_pvr_ipc_read(int fd,void *buff,size_t size);
extern int mt_pvr_ipc_write(int fd,void *buff,size_t size);
extern int mt_pvr_ipc_lseek(int fd,off_t offset,int mode);
extern int mt_pvr_ipc_fsync(int fd);
extern int mt_pvr_ipc_rename(char *oldfilename, char *newfilename);
extern int mt_pvr_ipc_remove(char *filename);

extern int mt_pvr_ipc_access(char *filename, int mode);
extern int mt_pvr_ipc_fadvise(int fd, off_t offset, off_t len, int advice);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifdef __MT_PVR_H__ */

