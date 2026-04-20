/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/mount.h>
#include <semaphore.h>
#include <sys/vfs.h>
#include "mt_common.h"
#include "mt_debug.h"
#include "lxc_ipc.h"
#include "mt_unf_ipcfs_type.h"

#define IPC_PAGE_SIZE       PAGE_SIZE
#define IPC_MINSIZE_BUFFER	PAGE_SIZE
#define IPC_MAX_BUFFER		(0x100000)


struct ipc_usbevent_param {
	ipc_fs_handle_t *ipc_handle;
	ipc_usbevent_callback callback;
};

static void *usbevent_callback_task(void *ipc_handle)
{
	struct lxc_ipc *ipc_fs = NULL;
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_usbevent_param *usbevent = NULL;
	struct ipc_mp_param *param = NULL;

	usbevent = (struct ipc_usbevent_param *)ipc_handle;
	if(NULL == usbevent)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return 0;
	}
	if(NULL == usbevent->ipc_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return 0;
	}
	ipc_fs = &(usbevent->ipc_handle->ipc_fs);

	lxc_ipc_resize(ipc_fs, &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf is null error!\n", __FUNCTION__,__LINE__);
		return 0;
	}
	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->mode = 0;
	info_flag = IPC_FS_REGISTER;
	lxc_ipc_call(ipc_fs, info_flag);
	if(-2 == param->ret)
	{
#if IPC_FS_DEBUG
		MT_DEBUG_LOG("%s %d exit\n", __FUNCTION__,__LINE__);
#endif
		return 0;
	}
	param->ret = -1;
	info_flag = IPC_FS_USBEVENT;

	while (1)
	{
		lxc_ipc_call(ipc_fs, info_flag);
		if(IPC_EVENT_UNREGISTER_FLAG == param->mode)
		{
			in_size = 0;
			out_size = 0;
			lxc_ipc_resize(ipc_fs, &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
			free(usbevent);
			break;
		}
		if((0 == param->ret) && usbevent->callback)
		{
			usbevent->callback(param->flag,param->content);
			param->ret = -1;
		}
	}
#if IPC_FS_DEBUG
	MT_DEBUG_LOG("%s %d exit\n", __FUNCTION__,__LINE__);
#endif
	return 0;
}

int ipc_fs_init(ipc_fs_handle_t *fs_handle,char *name)
{
	if(fs_handle->flag == 1)
		return 0;
	if (lxc_ipc_open_client(name, &(fs_handle->ipc_fs))) {
		MT_ERROR_LOG("%s %d ERROR!\n", __FUNCTION__,__LINE__);
		return -1;
	}
	pthread_mutex_init(&(fs_handle->mutex),NULL);
	fs_handle->flag = 1;
	return 0;
}

int ipc_fs_deinit(ipc_fs_handle_t *fs_handle)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	if(fs_handle->flag == -1)
	{
		return 0;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	info_flag = IPC_FS_SEVER_CLOSE;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	lxc_ipc_close(&(fs_handle->ipc_fs));
	fs_handle->flag = -1;
	pthread_mutex_unlock(&(fs_handle->mutex));
	pthread_mutex_destroy(&(fs_handle->mutex));
	return 0;
}

int ipc_fs_register(ipc_fs_handle_t *fs_handle,ipc_usbevent_callback callback)
{
	struct ipc_usbevent_param *param = NULL;
	pthread_t thread_callback;

	param = (struct ipc_usbevent_param*)malloc(sizeof(struct ipc_usbevent_param));
	if(NULL == param)
	{
		MT_ERROR_LOG("%s %d malloc is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	param->callback = callback;
	param->ipc_handle=fs_handle;
	pthread_create(&thread_callback, NULL, usbevent_callback_task, (void*)param);
	return 0;
}

int ipc_fs_unregister(ipc_fs_handle_t *fs_handle)
{
	int info_flag = 0;;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	info_flag = IPC_FS_UNREGISTER;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	sleep(2);

	return 0;
}

int ipc_fs_mount(ipc_fs_handle_t *fs_handle,const char *dev, const char *dir, const char *fs, unsigned long flags, const void* data )
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;	/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == dev) || (NULL == dir) || (NULL == fs))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		return -1;
	}
	memset(in_buf,0,IPC_MINSIZE_BUFFER);
	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->mode = flags;

	strncpy(param->content, dev,strnlen(dev,IPC_MOUNT_PARAM_MAXLIMIT));
	strncpy(param->content+IPC_MOUNT_PARAM_MAXLIMIT, dir,strnlen(dir,IPC_MOUNT_PARAM_MAXLIMIT));
	strncpy(param->content+(2*IPC_MOUNT_PARAM_MAXLIMIT), fs,strnlen(fs,IPC_MOUNT_PARAM_MAXLIMIT));
	strncpy(param->content+(3*IPC_MOUNT_PARAM_MAXLIMIT), data,strnlen(data,IPC_MOUNT_PARAM_MAXLIMIT));
	info_flag = IPC_FS_MOUNT;

	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	return 0;
}

int ipc_fs_umount(ipc_fs_handle_t *fs_handle,const char *dir)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == dir))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		return -1;
	}
	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, dir,MAX_FILE_PATH);
	info_flag = IPC_FS_UNMOUNT;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	return param->ret;
}

ulong ipc_fs_fopen(ipc_fs_handle_t *fs_handle,const char *path, const char * mode)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == path) || (NULL == mode))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = -1;
	info_flag = IPC_FS_FOPEN;
	strncpy(param->fmode, mode,16);
	strncpy(param->content, path,MAX_FILE_PATH);
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->fd;
}

int ipc_fs_fclose(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fd= fd;
	info_flag = IPC_FS_FCLOSE;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_fread(ipc_fs_handle_t *fs_handle,unsigned char* buff,unsigned int size,unsigned int count,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	unsigned int len = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (0 == count))
	{
		return 0;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	len = size*count;
	in_size = len + sizeof(struct ipc_mp_param);
	if(in_size> IPC_MAX_BUFFER)
	{
		//need Fragmentation
		if(size > IPC_MAX_BUFFER)
		{
			MT_ERROR_LOG("%s %d no support!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		else
		{
			in_size = IPC_MAX_BUFFER+sizeof(struct ipc_mp_param);
			lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
			if(NULL == in_buf)
			{
				MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			param = (struct ipc_mp_param *)in_buf;
			param->fd = fd;
			info_flag = IPC_FS_FREAD;
			param->size = size;
			param->count = 1;
			for(len = 0;len <count;len++)
			{
				lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

				if((param->ret == 0) && (1 == param->count))
				{
					memcpy((char*)buff+(len*size),param->content,size);
				}
				else
				{
					pthread_mutex_unlock(&(fs_handle->mutex));
					if(len > 0)
					{
						return (int)len;
					}
					return -1;
				}

			}
			pthread_mutex_unlock(&(fs_handle->mutex));
			return (int)count;
		}
	}
	else
	{
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->count = count;
		param->size = size;
		info_flag = IPC_FS_FREAD;
		lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
		if(param->ret == 0)
		{
			memcpy(buff,param->content,param->count*size);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return (int)(param->count);
		}
		else
		{
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
	}

}

int ipc_fs_fwrite(ipc_fs_handle_t *fs_handle,unsigned char* buff,unsigned int size,unsigned int count,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	unsigned int len = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (0 == count))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	len = size*count;
	in_size = len + sizeof(struct ipc_mp_param);
	if(in_size> IPC_MAX_BUFFER)
	{
		//need Fragmentation
		if(size > IPC_MAX_BUFFER)
		{
			MT_ERROR_LOG("%s %d no support!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		else
		{
			in_size = IPC_MAX_BUFFER+sizeof(struct ipc_mp_param);
			lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
			if(NULL == in_buf)
			{
				MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			param = (struct ipc_mp_param *)in_buf;
			param->fd = fd;
			info_flag = IPC_FS_FWRITE;
			param->size = size;
			param->count = 1;
			for(len = 0;len <count;len++)
			{
				memcpy(param->content,(char*)buff+(len*size),size);
				lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

				if((param->ret != 0) && (1 != param->count))
				{
					pthread_mutex_unlock(&(fs_handle->mutex));
					if(len > 0)
					{
						return (int)len;
					}
					return -1;
				}
			}
			pthread_mutex_unlock(&(fs_handle->mutex));
			return (int)count;
		}
	}
	else
	{
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->count = count;
		param->size = size;
		memcpy(param->content,buff,param->count*param->size);
		info_flag = IPC_FS_FWRITE;
		lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
		pthread_mutex_unlock(&(fs_handle->mutex));
		if(param->ret == 0)
		{
			return (int)(param->count);
		}
		else
		{
			return -1;
		}
	}

}

int ipc_fs_fseek(ipc_fs_handle_t *fs_handle,ulong fd,off_t offset,int mode)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fd= fd;
	param->pos = offset;
	param->flag = mode;
	info_flag = IPC_FS_FSEEK;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return 0;
}

int ipc_fs_fseeko(ipc_fs_handle_t *fs_handle,ulong fd,off_t offset,int mode)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fd= fd;
	param->flag = mode;
	param->pos = offset;

	info_flag = IPC_FS_FSEEKO;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_frewind(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = fd;
	info_flag = IPC_FS_FREWIND;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return 0;
}

off_t ipc_fs_ftell(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = fd;
	info_flag = IPC_FS_FTELL;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->pos;
}


off_t ipc_fs_ftello(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = fd;
	info_flag = IPC_FS_FTELLO;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->pos;
}

int ipc_fs_flush(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = fd;
	info_flag = IPC_FS_FLUSH;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return 0;
}

int ipc_fs_fsync(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if(NULL == fs_handle)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = fd;
	info_flag = IPC_FS_FSYNC;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return 0;
}

ulong ipc_fs_open(ipc_fs_handle_t *fs_handle,const char *path, int flag,unsigned int mode)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == path))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = -1;
	param->flag = flag;
	param->mode = mode;
	info_flag = IPC_FS_OPEN;
	strncpy(param->content, path,MAX_FILE_PATH);
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->fd;
}

int ipc_fs_close(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (fd == -1))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fd= fd;
	info_flag = IPC_FS_CLOSE;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_read(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	int rtsize = 0;
	unsigned int i = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (fd == -1))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	in_size = size + IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
	if(in_size> IPC_MAX_BUFFER)
	{
		//need Fragmentation
		in_size = IPC_MAX_BUFFER+ IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		info_flag = IPC_FS_READ;
		param->size = IPC_MAX_BUFFER;

		for(i = 0;i <size/IPC_MAX_BUFFER;i++)
		{
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			else if(param->ret != IPC_MAX_BUFFER)
			{
				memcpy((char*)buff+rtsize,(char*)in_buf+IPC_PAGE_SIZE,(unsigned int)param->ret);
				pthread_mutex_unlock(&(fs_handle->mutex));
				return param->ret+rtsize;
			}
			memcpy((char*)buff+rtsize,(char*)in_buf+IPC_PAGE_SIZE,IPC_MAX_BUFFER);
			rtsize+= IPC_MAX_BUFFER;
		}
		param->ret = 0;
		if(0 != (size%IPC_MAX_BUFFER))
		{
			param->size = (size%IPC_MAX_BUFFER);
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			memcpy((char*)buff+rtsize,(char*)in_buf+IPC_PAGE_SIZE,(unsigned int)param->ret);
		}
		pthread_mutex_unlock(&(fs_handle->mutex));
		return rtsize+param->ret;

	}
	else
	{
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->size = size;
		info_flag = IPC_FS_READ;
		lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
		if(param->ret > 0)
		{
			memcpy(buff,(char*)in_buf+IPC_PAGE_SIZE,(unsigned int)(param->ret));
		}
		pthread_mutex_unlock(&(fs_handle->mutex));
		return param->ret;
	}

}

int ipc_fs_write(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	int rtsize = 0;
	unsigned int i = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (fd == -1))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	in_size = size + IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
	if(in_size> IPC_MAX_BUFFER)
	{
		//need Fragmentation
		in_size = IPC_MAX_BUFFER + IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		info_flag = IPC_FS_WRITE;
		param->size = IPC_MAX_BUFFER;
		for(i = 0;i <size/IPC_MAX_BUFFER;i++)
		{
			memcpy((char*)in_buf+IPC_PAGE_SIZE,(char*)buff+rtsize,IPC_MAX_BUFFER);
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			else if(param->ret != IPC_MAX_BUFFER)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return param->ret+rtsize;
			}
			rtsize+=IPC_MAX_BUFFER;
		}
		param->ret = 0;
		if(0 != (size%IPC_MAX_BUFFER))
		{
			param->size = (size%IPC_MAX_BUFFER);
			memcpy((char*)in_buf+IPC_PAGE_SIZE,(char*)buff+rtsize,param->size);
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
		}
		pthread_mutex_unlock(&(fs_handle->mutex));
		return rtsize+param->ret;

	}
	else
	{
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->size = size;

		memcpy((char*)in_buf+IPC_PAGE_SIZE,buff,param->size);
		info_flag = IPC_FS_WRITE;
		lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return param->ret;
	}

}


int ipc_fs_pread(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	int rtsize = 0;
	unsigned int i = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (fd == -1))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	in_size = size + IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
	if(in_size> IPC_MAX_BUFFER)
	{
		//need Fragmentation
		in_size = IPC_MAX_BUFFER+ IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->pos = offset;
		info_flag = IPC_FS_PREAD;
		param->size = IPC_MAX_BUFFER;
		for(i = 0;i <size/IPC_MAX_BUFFER;i++)
		{
			param->memoff=0;
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			else if(param->ret != IPC_MAX_BUFFER)
			{
				memcpy((char*)buff+rtsize,(char*)in_buf+IPC_PAGE_SIZE,(unsigned int)param->ret);
				pthread_mutex_unlock(&(fs_handle->mutex));
				return param->ret+rtsize;
			}
			memcpy((char*)buff+rtsize,(char*)in_buf+IPC_PAGE_SIZE,IPC_MAX_BUFFER);
			rtsize+= IPC_MAX_BUFFER;
			param->pos = offset + rtsize;
		}
		param->ret = 0;
		if(0 != (size%IPC_MAX_BUFFER))
		{
			param->size = (size%IPC_MAX_BUFFER);
			param->memoff=0;
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			memcpy((char*)buff+rtsize,(char*)in_buf+IPC_PAGE_SIZE,(unsigned int)param->ret);
		}
		pthread_mutex_unlock(&(fs_handle->mutex));
		return rtsize+param->ret;

	}
	else
	{
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->size = size;
		param->pos = offset;
		param->memoff=0;
		info_flag = IPC_FS_PREAD;
		lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
		if(param->ret > 0)
		{
			memcpy(buff,(char*)in_buf+IPC_PAGE_SIZE,(unsigned int)(param->ret));
		}
		pthread_mutex_unlock(&(fs_handle->mutex));
		return param->ret;
	}
}

int ipc_fs_pwrite(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	int rtsize = 0;
	unsigned int i = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (fd == -1))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	in_size = size + IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
	if(in_size> IPC_MAX_BUFFER)
	{
		//need Fragmentation
		in_size = IPC_MAX_BUFFER+ IPC_PAGE_SIZE;//sizeof(struct ipc_mp_param);
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->pos = offset;
		info_flag = IPC_FS_PWRITE;
		param->size = IPC_MAX_BUFFER;
		for(i = 0;i <size/IPC_MAX_BUFFER;i++)
		{
			memcpy((char*)in_buf+IPC_PAGE_SIZE,(char*)buff+rtsize,IPC_MAX_BUFFER);
			param->memoff=0;
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
			else if(param->ret != IPC_MAX_BUFFER)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return param->ret+rtsize;
			}
			rtsize+=IPC_MAX_BUFFER;
			param->pos = offset + rtsize;
		}
		param->ret = 0;
		if(0 != (size%IPC_MAX_BUFFER))
		{
			param->size = (size%IPC_MAX_BUFFER);
			memcpy((char*)in_buf+IPC_PAGE_SIZE,(char*)buff+rtsize,param->size);
			param->memoff=0;
			lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
			if(-1 == param->ret)
			{
				pthread_mutex_unlock(&(fs_handle->mutex));
				return -1;
			}
		}
		pthread_mutex_unlock(&(fs_handle->mutex));
		return rtsize+param->ret;

	}
	else
	{
		lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
		if(NULL == in_buf)
		{
			MT_ERROR_LOG("%s %d in_buf is null error!\n",__FUNCTION__,__LINE__);
			pthread_mutex_unlock(&(fs_handle->mutex));
			return -1;
		}
		param = (struct ipc_mp_param *)in_buf;
		param->fd = fd;
		param->size = size;
		param->pos = offset;
		memcpy((char*)in_buf+IPC_PAGE_SIZE,buff,param->size);
		info_flag = IPC_FS_PWRITE;
		param->memoff=0;
		lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return param->ret;
	}

}

int ipc_fs_pwrite_nocopy(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	int rtsize = 0;
	unsigned int i = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (fd == -1))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	param = (struct ipc_mp_param *)fs_handle->ipc_fs.in_buf;
	param->fd = fd;
	param->size = size;
	param->pos = offset;
	param->memoff = ((ulong)buff-(ulong)((unsigned char*)fs_handle->ipc_fs.in_buf+IPC_PAGE_SIZE));
	info_flag = IPC_FS_PWRITE;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	pthread_mutex_unlock(&(fs_handle->mutex));

	return param->ret;
}
int ipc_fs_pread_nocopy(ipc_fs_handle_t *fs_handle,ulong fd,void *buff,size_t size,off_t offset)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	int rtsize = 0;
	unsigned int i = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == buff) || (0 == size) || (fd == -1))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	param = (struct ipc_mp_param *)fs_handle->ipc_fs.in_buf;
	param->fd = fd;
	param->size = size;
	param->pos = offset;
	param->memoff = ((ulong)buff-(ulong)((unsigned char*)fs_handle->ipc_fs.in_buf+IPC_PAGE_SIZE));
	info_flag = IPC_FS_PREAD;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	if (param->ret > 0) {
		//MT_DEBUG_LOG("++client.pr.same1=%x,%x,%x,%x\n",(char*)in_buf+IPC_PAGE_SIZE,fd,(param->ret),size);
	}
	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

off_t ipc_fs_lseek(ipc_fs_handle_t *fs_handle,ulong fd,off_t offser,int mode)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (fd == -1))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param = (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fd= fd;
	param->flag = mode;
	param->pos = offser;
	info_flag = IPC_FS_LSEEK;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->pos;
}

int ipc_fs_sync(ipc_fs_handle_t *fs_handle,ulong fd)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (fd == -1))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d buff is null\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param = (struct ipc_mp_param *)in_buf;
	param->fd = fd;
	info_flag = IPC_FS_SYNC;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return 0;
}

int ipc_fs_fstat(ipc_fs_handle_t *fs_handle,ulong fd,struct stat *buf)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;
	char *tmp = NULL;

	if((NULL == fs_handle) || (fd == -1))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fd = fd;
	tmp = (char*)buf;
	info_flag = IPC_FS_FSTAT;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	if(0 == param->ret)
	{
		memcpy(tmp,param->content,sizeof(struct stat));
		pthread_mutex_unlock(&(fs_handle->mutex));
		return 0;
	}
	else
	{
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
}

int ipc_fs_stat(ipc_fs_handle_t *fs_handle,const char *filename,struct stat *buf)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;
	char *tmp = NULL;

	if((NULL == fs_handle) || (NULL == filename) || (NULL == buf))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, filename,MAX_FILE_PATH);
	tmp = (char*)buf;
	info_flag = IPC_FS_STAT;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	if(0 == param->ret)
	{
		memcpy(tmp,param->content,sizeof(struct stat));
		pthread_mutex_unlock(&(fs_handle->mutex));
		return 0;
	}
	else
	{
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
}
int ipc_fs_unlink(ipc_fs_handle_t *fs_handle,const char *path)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == path))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, path,MAX_FILE_PATH);
	info_flag = IPC_FS_ULINK;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_mkdir(ipc_fs_handle_t *fs_handle,const char *path, unsigned int mode)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == path))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);

	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->mode = mode;
	strncpy(param->content, path,MAX_FILE_PATH);
	info_flag = IPC_FS_MKDIR;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_rmdir(ipc_fs_handle_t *fs_handle,const char *path)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == path))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, path,MAX_FILE_PATH);
	info_flag = IPC_FS_RMDIR;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

DIR* ipc_fs_opendir(ipc_fs_handle_t *fs_handle,const char *path)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (NULL == path))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return NULL;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return NULL;
	}

	param =  (struct ipc_mp_param *)in_buf;
	param->fdir = NULL;
	strncpy(param->content, path,MAX_FILE_PATH);
	info_flag = IPC_FS_OPENDIR;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->fdir;
}

int ipc_fs_closedir(ipc_fs_handle_t *fs_handle,DIR* fdir)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (fdir < 0))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fdir = fdir;
	info_flag = IPC_FS_CLOSEDIR;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_readdir(ipc_fs_handle_t *fs_handle,DIR* fdir, struct dirent *dirp)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (fdir < 0))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}

	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	param->fdir = fdir;
	info_flag = IPC_FS_READDIR;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	memcpy(dirp, &param->dirent, sizeof(struct dirent));
	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_access(ipc_fs_handle_t *fs_handle,const char *filename, int mode)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;
	char *tmp = NULL;
	if((NULL == fs_handle) || (NULL == filename))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, filename,MAX_FILE_PATH);
	param->flag = mode;
	info_flag = IPC_FS_ACCESS;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}
int ipc_fs_remove(ipc_fs_handle_t *fs_handle,const char *filename)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;
	char *tmp = NULL;
	if((NULL == fs_handle) || (NULL == filename))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, filename,MAX_FILE_PATH);
	info_flag = IPC_FS_REMOVE;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_getinbuffer(ipc_fs_handle_t *fs_handle,void **buff,off_t *buff_phy,int size)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	off_t in_phy;
	size_t in_size = IPC_PAGE_SIZE+size;
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;
	char *tmp = NULL;
	mt_u32 page_block_order=0,pages_per_block=0,align_v=PAGE_SIZE;
	mt_s32 tmp_ret=0;
	if((NULL == fs_handle) || (NULL == buff))
	{
		//MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	*buff=NULL;
	tmp_ret=mt_mem_get_pageinfo(&page_block_order,&pages_per_block);
	if (0==tmp_ret) {
		align_v=((1 << page_block_order) * PAGE_SIZE);
		in_size = ((in_size+(align_v-1))&(~(align_v-1)));
	}else{
		MT_ERROR_LOG("%s %d get_pageinfo fail!\n",__FUNCTION__,__LINE__);
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,align_v,align_v,&in_phy);
	if(NULL == in_buf)
	{
		//MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	*buff = (char*)in_buf+IPC_PAGE_SIZE;
	if (buff_phy) {
		*buff_phy = in_phy+IPC_PAGE_SIZE;
	}
	//MT_DEBUG_LOG("cli.g=%x,%x\n",in_buf,fs_handle->ipc_fs.in_buf);
	pthread_mutex_unlock(&(fs_handle->mutex));
	return 0;
}

int ipc_fs_freebuffer(ipc_fs_handle_t *fs_handle)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_PAGE_SIZE;
	size_t out_size = 0;		/* no use*/

	if(NULL == fs_handle)
	{
		//MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		//MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	pthread_mutex_unlock(&(fs_handle->mutex));
	return 0;
}

int ipc_fs_fadvise(ipc_fs_handle_t *fs_handle,ulong fd, off_t offset, off_t len, int advice)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	int rtsize = 0;
	unsigned int i = 0;
	struct ipc_mp_param *param = NULL;

	if((NULL == fs_handle) || (0 == len) || (fd == -1))
	{
		return 0;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	param = (struct ipc_mp_param *)fs_handle->ipc_fs.in_buf;
	param->fd = fd;
	param->size = len;
	param->pos = offset;
	param->memoff = advice;
	info_flag = IPC_FS_FADVICE;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	pthread_mutex_unlock(&(fs_handle->mutex));

	return 0;
}

int ipc_fs_rename(ipc_fs_handle_t *fs_handle,const char *old_filename,const char *new_filename)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;
	char *tmp = NULL;
	if((NULL == fs_handle) || (NULL == old_filename) || (NULL == new_filename))
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, old_filename,strnlen(old_filename,IPC_MOUNT_PARAM_MAXLIMIT));
	strncpy(param->content+IPC_MOUNT_PARAM_MAXLIMIT, new_filename,strnlen(new_filename,IPC_MOUNT_PARAM_MAXLIMIT));
	info_flag = IPC_FS_RENAME;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);
	pthread_mutex_unlock(&(fs_handle->mutex));
	return param->ret;
}

int ipc_fs_statfs(ipc_fs_handle_t *fs_handle,const char *filepath,struct statfs *fs)
{
	void *in_buf = NULL;
	void *out_buf = NULL;
	size_t in_size = IPC_MINSIZE_BUFFER;		/* size should more than all param */
	size_t out_size = 0;		/* no use*/
	int info_flag = 0;
	struct ipc_mp_param *param = NULL;
	char *tmp = NULL;

	if((NULL == fs_handle) || filepath == NULL)
	{
		MT_ERROR_LOG("%s %d param is error!\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pthread_mutex_lock(&(fs_handle->mutex));
	lxc_ipc_resize(&(fs_handle->ipc_fs), &in_buf, &out_buf, &in_size, &out_size,PAGE_SIZE,PAGE_SIZE,NULL);
	if(NULL == in_buf)
	{
		MT_ERROR_LOG("%s %d in_buf = %p\n",__FUNCTION__,__LINE__,in_buf);
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
	param =  (struct ipc_mp_param *)in_buf;
	param->ret = -1;
	strncpy(param->content, filepath,MAX_FILE_PATH);
	tmp = (char*)fs;
	info_flag = IPC_FS_STATFS;
	lxc_ipc_call(&(fs_handle->ipc_fs), info_flag);

	if(0 == param->ret)
	{
		memcpy(tmp,param->content,sizeof(struct statfs));
		pthread_mutex_unlock(&(fs_handle->mutex));
		return 0;
	}
	else
	{
		pthread_mutex_unlock(&(fs_handle->mutex));
		return -1;
	}
}