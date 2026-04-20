/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MPFS_IPC_TYPE_H__
#define __MPFS_IPC_TYPE_H__

#include <sys/types.h>
#include <pthread.h>
#include <dirent.h>
#include "mt_type.h"
#include "lxc_ipc.h"

#define IPC_FS_DEBUG	(0)

#define USB_INSERT_EVENT	(1)
#define USB_REMOVE_EVENT	(0)

#define IPC_MOUNT_PARAM_MAXLIMIT (256)

#define IPC_EVENT_UNREGISTER_FLAG (0xcc)

#define MAX_FILE_PATH	(1024)

enum IPC_MPFS_OPERATE {
	IPC_FS_REGISTER,
	IPC_FS_UNREGISTER,
	IPC_FS_MOUNT,
	IPC_FS_UNMOUNT,
	IPC_FS_FOPEN,
	IPC_FS_FCLOSE,
	IPC_FS_FREAD,
	IPC_FS_FWRITE,
	IPC_FS_FSEEK,
	IPC_FS_FSEEKO,
	IPC_FS_FREWIND,
	IPC_FS_FTELL,
	IPC_FS_FTELLO,
	IPC_FS_FLUSH,
	IPC_FS_FSYNC,
	IPC_FS_OPEN,
	IPC_FS_CLOSE,
	IPC_FS_READ,
	IPC_FS_WRITE,
	IPC_FS_PREAD,
	IPC_FS_PWRITE,	
	IPC_FS_LSEEK,
	IPC_FS_SYNC,
	IPC_FS_FSTAT,
	IPC_FS_STAT,
	IPC_FS_ULINK,
	IPC_FS_MKDIR,
	IPC_FS_RMDIR,
	IPC_FS_OPENDIR,
	IPC_FS_CLOSEDIR,
	IPC_FS_READDIR,
	IPC_FS_STATFS,
	IPC_FS_USBEVENT,
	IPC_FS_SEVER_CLOSE,
	IPC_FS_ACCESS,
	IPC_FS_REMOVE,
	IPC_FS_FADVICE,   //same as posix_fadvise
	IPC_FS_RENAME
};

typedef struct ipc_fs_handle {
	struct lxc_ipc ipc_fs;
	pthread_mutex_t mutex;
	int flag;
}ipc_fs_handle_t;

struct ipc_mp_param {
	int ret;
	ulong fd;		/* as FILE pointer or file fd, so only can check -1, not check 0 or negative */
	DIR* fdir;
	struct dirent dirent;
	int flag;
	off_t pos;
	ulong memoff;         //memory offset
	unsigned int mode;
	unsigned int count;
	unsigned int size;
	char fmode[16];
	char content[0];
};


typedef void(*ipc_usbevent_callback)(int eventflag,char *devname);

int ipc_fs_init(ipc_fs_handle_t *fs_handle,char *name);
int ipc_fs_deinit(ipc_fs_handle_t *fs_handle);
int ipc_fs_register(ipc_fs_handle_t *fs_handle,ipc_usbevent_callback callback);
int ipc_fs_unregister(ipc_fs_handle_t *fs_handle);
int ipc_fs_mount(ipc_fs_handle_t *fs_handle,const char *dev, const char *dir, const char *fs, unsigned long flags, const void* data );
int ipc_fs_umount(ipc_fs_handle_t *fs_handle,const char *dir);

ulong ipc_fs_fopen(ipc_fs_handle_t *fs_handle,const char *path, const char * mode);
int ipc_fs_fclose(ipc_fs_handle_t *fs_handle,ulong fd);
int ipc_fs_fread(ipc_fs_handle_t *fs_handle,unsigned char* buff,unsigned int size,unsigned int count,ulong fd);
int ipc_fs_fwrite(ipc_fs_handle_t *fs_handle,unsigned char* buff,unsigned int size,unsigned int count,ulong fd);
int ipc_fs_fseek(ipc_fs_handle_t *fs_handle,ulong fd,off_t offset,int mode);
int ipc_fs_fseeko(ipc_fs_handle_t *fs_handle,ulong fd,off_t offset,int mode);
int ipc_fs_frewind(ipc_fs_handle_t *fs_handle,ulong fd);
off_t ipc_fs_ftell(ipc_fs_handle_t *fs_handle,ulong fd);
off_t ipc_fs_ftello(ipc_fs_handle_t *fs_handle,ulong fd);
int ipc_fs_flush(ipc_fs_handle_t *fs_handle,ulong fd);
int ipc_fs_fsync(ipc_fs_handle_t *fs_handle,ulong fd);
ulong ipc_fs_open(ipc_fs_handle_t *fs_handle,const char *path, int flag,unsigned int mode);
int ipc_fs_close(ipc_fs_handle_t *fs_handle,ulong fd);
int ipc_fs_read(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size);
int ipc_fs_write(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size);
int ipc_fs_pread(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset);
int ipc_fs_pwrite(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset);
int ipc_fs_pwrite_nocopy(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset);
int ipc_fs_pread_nocopy(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset);
off_t ipc_fs_lseek(ipc_fs_handle_t *fs_handle,ulong fd,off_t offser,int mode);
int ipc_fs_sync(ipc_fs_handle_t *fs_handle,ulong fd);
int ipc_fs_fstat(ipc_fs_handle_t *fs_handle,ulong fd,struct stat *buf);
int ipc_fs_stat(ipc_fs_handle_t *fs_handle,const char *filename,struct stat *buf);
int ipc_fs_unlink(ipc_fs_handle_t *fs_handle,const char *path);
int ipc_fs_mkdir(ipc_fs_handle_t *fs_handle,const char *path, unsigned int mode);
int ipc_fs_rmdir(ipc_fs_handle_t *fs_handle,const char *path);
DIR* ipc_fs_opendir(ipc_fs_handle_t *fs_handle,const char *path);
int ipc_fs_closedir(ipc_fs_handle_t *fs_handle,DIR* fdir);
int ipc_fs_readdir(ipc_fs_handle_t *fs_handle,DIR* fdir, struct dirent *dirp);
int ipc_fs_access(ipc_fs_handle_t *fs_handle,const char *filename, int mode);
int ipc_fs_remove(ipc_fs_handle_t *fs_handle,const char *filename);
int ipc_fs_getinbuffer(ipc_fs_handle_t *fs_handle,void **buff,off_t *in_phy,int size);      //get inbuffer
int ipc_fs_freebuffer(ipc_fs_handle_t *fs_handle);                            //free inbuffer
int ipc_fs_fadvise(ipc_fs_handle_t *fs_handle,ulong fd, off_t offset, off_t len, int advice); //posix_fadvise
int ipc_fs_rename(ipc_fs_handle_t *fs_handle,const char *old_filename,const char *new_filename);
int ipc_fs_statfs(ipc_fs_handle_t *fs_handle,const char *filepath,struct statfs *fs);
#endif /* __MPFS_IPC_TYPE_H__ */

