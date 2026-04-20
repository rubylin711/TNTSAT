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
#include <dirent.h>


#include "mt_common.h"
#include "lxc_ipc.h"
#include "mt_unf_ipcfs_type.h"

#define IPC_PAGES_SIZE_SEVER  4096
typedef struct ipc_info{
	struct lxc_ipc *ipc_mp;
	void* buf_mp;
	struct ipc_info *next;
} ipc_list;

static sem_t sem_mp;
static sem_t listsem_mp;
static sem_t plugevent_sem_mp;
static char uevent[8192];

static ipc_list *g_usb_ipclist = NULL;

static int ipc_insert(struct lxc_ipc *ipc_mp,void* buf_mp)
{
	ipc_list *usb_ipc_list = NULL;
	ipc_list *tmp_usb_ipc_list = NULL;

	sem_wait(&listsem_mp);

	usb_ipc_list = g_usb_ipclist;

	if(g_usb_ipclist== NULL)
	{
		g_usb_ipclist = (ipc_list *)malloc(sizeof(ipc_list));
		if(NULL == g_usb_ipclist)
		{
			sem_post(&listsem_mp);
			return -1;
		}
		g_usb_ipclist->ipc_mp = ipc_mp;
		g_usb_ipclist->buf_mp = buf_mp;
		g_usb_ipclist->next = NULL;
		sem_post(&listsem_mp);
		return 0;
	}

	do{
		if((usb_ipc_list->ipc_mp == ipc_mp) && (usb_ipc_list->ipc_mp->tid == ipc_mp->tid))
		{
			sem_post(&listsem_mp);
#if IPC_FS_DEBUG
			printf("%s %d return -2\n", __FUNCTION__,__LINE__);
#endif
			return -2;
		}
		usb_ipc_list = usb_ipc_list->next;
	}while(usb_ipc_list!=NULL);

	usb_ipc_list = g_usb_ipclist;
	while(usb_ipc_list->next)
	{
		usb_ipc_list= usb_ipc_list->next;
	}
	tmp_usb_ipc_list = (ipc_list *)malloc(sizeof(ipc_list));
	if(NULL == tmp_usb_ipc_list)
	{
		sem_post(&listsem_mp);
		return -1;
	}
	tmp_usb_ipc_list->ipc_mp = ipc_mp;
	tmp_usb_ipc_list->buf_mp = buf_mp;
	tmp_usb_ipc_list->next = NULL;

	usb_ipc_list->next = tmp_usb_ipc_list;
	sem_post(&listsem_mp);
	return 0;

}

static int ipc_delete(struct lxc_ipc *ipc_mp)
{
	ipc_list *usb_ipc_list = NULL;
	ipc_list *tmp_usb_ipc_list = NULL;

	sem_wait(&listsem_mp);

	usb_ipc_list = g_usb_ipclist;
	tmp_usb_ipc_list = g_usb_ipclist;

	if(g_usb_ipclist== NULL)
	{
		sem_post(&listsem_mp);
		return 0;
	}

	while(((usb_ipc_list->ipc_mp !=ipc_mp) || (usb_ipc_list->ipc_mp->tid == ipc_mp->tid)) && usb_ipc_list->next!=NULL)
	{
		tmp_usb_ipc_list=usb_ipc_list;
		usb_ipc_list=usb_ipc_list->next;
	}
	if((usb_ipc_list->ipc_mp == ipc_mp) && (usb_ipc_list->ipc_mp->tid == ipc_mp->tid))
	{
		if(usb_ipc_list == g_usb_ipclist)
		{
			if(NULL == g_usb_ipclist->next)
			{
				g_usb_ipclist = NULL;
			}
			else
			{
				g_usb_ipclist=g_usb_ipclist->next;
			}
		}
		else
		{
			tmp_usb_ipc_list->next=usb_ipc_list->next;
		}
		free(usb_ipc_list);
		usb_ipc_list = NULL;
	}
	sem_post(&listsem_mp);
	return 0;

}

static int ipc_show(int event,char* buff)
{

	ipc_list *usb_ipc_list = NULL;
	struct ipc_mp_param *param = NULL;
	sem_wait(&listsem_mp);
	usb_ipc_list = g_usb_ipclist;
	if(usb_ipc_list==NULL)
	{
		sem_post(&listsem_mp);
		return -1;
	}
	else
	{
		do{
			if(usb_ipc_list->ipc_mp)
			{
				param = (struct ipc_mp_param *)usb_ipc_list->buf_mp;
				param->ret = 0;
				param->flag = event;
				memset(param->content,0,IPC_MOUNT_PARAM_MAXLIMIT);
				strncpy(param->content,buff,IPC_MOUNT_PARAM_MAXLIMIT);
				lxc_ipc_done(usb_ipc_list->ipc_mp);
			}
			usb_ipc_list=usb_ipc_list->next;
		}
		while(usb_ipc_list!=NULL);
	}
	sem_post(&listsem_mp);
	return 0;
}

static void *ipc_fs_server(void *arg)
{
	struct lxc_ipc ipc_mp;
	void *in_buf_mp;
	void *out_buf_mp;
	size_t in_size_mp;		/* size should more than all param */
	size_t out_size_mp;		/* size should more than all result */
	int do_what_mp;
	struct ipc_mp_param *param = NULL;
	char name_mp[16];

	ipc_mp = *((struct lxc_ipc *)arg);
	sem_post(&sem_mp);

	snprintf(name_mp, sizeof(name_mp), "ipc_fs_server_%d", ipc_mp.tid);
	mt_set_pthread_name(name_mp);

	while (1) {
		lxc_ipc_accept(&ipc_mp, &do_what_mp, &in_buf_mp, &out_buf_mp, &in_size_mp, &out_size_mp);
		/* in_size_mp and out_size_mp was be update by kernel */

		param =  (struct ipc_mp_param *)in_buf_mp;
#if IPC_FS_DEBUG
		printf("%s %d do_what_mp = %d\n",__FUNCTION__,__LINE__,do_what_mp);
#endif
		switch (do_what_mp) {
		case IPC_FS_MOUNT:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
#if IPC_FS_DEBUG
				printf("%s %d mount dev : %s\n",__FUNCTION__,__LINE__,param->content);
				printf("%s %d mount path : %s\n",__FUNCTION__,__LINE__,param->content+IPC_MOUNT_PARAM_MAXLIMIT);
				printf("%s %d mount filesystem : %s\n",__FUNCTION__,__LINE__,param->content+2*IPC_MOUNT_PARAM_MAXLIMIT);
				printf("%s %d mount data : %s\n",__FUNCTION__,__LINE__,param->content+3*IPC_MOUNT_PARAM_MAXLIMIT);
#endif
				if(strstr(param->content+2*IPC_MOUNT_PARAM_MAXLIMIT,"vfat"))
				{
					param->ret = mount(param->content, param->content+IPC_MOUNT_PARAM_MAXLIMIT,
						param->content+2*IPC_MOUNT_PARAM_MAXLIMIT,param->mode,
						param->content+3*IPC_MOUNT_PARAM_MAXLIMIT);
				}
				else if(strstr(param->content+2*IPC_MOUNT_PARAM_MAXLIMIT,"ntfs"))
				{
					char cmd[1024] = {0};
					snprintf(cmd, sizeof(cmd), "ntfs-3g %s ", param->content);
					strcat(cmd,param->content+IPC_MOUNT_PARAM_MAXLIMIT);
#if IPC_FS_DEBUG
					printf("%s %d cmd : %s\n",__FUNCTION__,__LINE__,cmd);
#endif
					param->ret = system(cmd);
				}
				else
				{
					param->ret = -1;
				}

				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_UNMOUNT:
			if(NULL == param)
			{
				printf("%s %d param is null\n",__FUNCTION__,__LINE__);
				lxc_ipc_done(&ipc_mp);
				break;
			}
			param->content[MAX_FILE_PATH-1] = 0;
#if IPC_FS_DEBUG
			printf("%s %d umount : %s\n",__FUNCTION__,__LINE__,param->content);
#endif
			param->ret = umount(param->content);
			lxc_ipc_done(&ipc_mp);
			break;
		case IPC_FS_FOPEN:
			{
				FILE *fp = NULL;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				fp = fopen(param->content,param->fmode);
				if(NULL == fp)
				{
					printf("%s %d fopen error\n",__FUNCTION__,__LINE__);
					param->fd = -1;
				}
				else
				{
					param->fd = (ulong)fp;
				}
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FCLOSE:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret = fclose((FILE *)(param->fd));
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FREAD:
			{
				FILE *fp = NULL;
				unsigned int count = 0;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				fp = (FILE *)param->fd;
				count  = fread(param->content,param->size,param->count,fp);
				if(count>param->count)
				{
					printf("%s %d size = %d count = %d error!\n",__FUNCTION__,__LINE__,param->size,param->count);
					param->ret = -1;
				}
				else
				{
					param->ret = 0;
					param->count = count;
				}
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FWRITE:
			{
				FILE *fp = NULL;
				unsigned int count = 0;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				fp = (FILE *)param->fd;
				count  = fwrite(param->content,param->size,param->count,fp);
				if(count>param->count)
				{
					param->ret = -1;
				}
				else
				{
					param->ret = 0;
					param->count = count;
				}
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FSEEK:
			{
				FILE *fp = NULL;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				fp = (FILE *)param->fd;
				param->ret  = fseek(fp,(long)param->pos,param->flag);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FSEEKO:
			{
				FILE *fp = NULL;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				fp = (FILE *)param->fd;
				param->ret  = fseeko(fp,param->pos,param->flag);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FREWIND:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				rewind((FILE *)(param->fd));
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FTELL:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->pos = ftell((FILE *)(param->fd));
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FTELLO:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->pos = ftello((FILE *)(param->fd));
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FLUSH:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret = fflush((FILE *)(param->fd));
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FSYNC:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret =  fsync(fileno((FILE *)(param->fd)));
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_OPEN:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->fd  = open(param->content,param->flag,param->mode);

				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_CLOSE:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				if(param->fd == -1)
				{
					printf("%s %d fd =%d error!\n",__FUNCTION__,__LINE__,param->fd);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret = close(param->fd);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_READ:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret  = read(param->fd,(char*)param+IPC_PAGES_SIZE_SEVER,param->size);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_WRITE:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret  = write(param->fd,(char*)param+IPC_PAGES_SIZE_SEVER,param->size);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_PREAD:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret=pread(param->fd,(char*)param+IPC_PAGES_SIZE_SEVER+param->memoff,param->size,param->pos);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_PWRITE:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret  = pwrite(param->fd,(char*)param+IPC_PAGES_SIZE_SEVER+param->memoff,param->size,param->pos);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_LSEEK:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->pos  = lseek(param->fd,param->pos,param->flag);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_SYNC:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret =  fsync(param->fd);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_FSTAT:
			{
				struct stat fdata;
				char *tmp = NULL;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret =  fstat(param->fd,&fdata);
				if(0 == param->ret)
				{
					tmp = (char*)(&fdata);
					memcpy(param->content,tmp,sizeof(struct stat));
				}
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_STAT:
			{
				struct stat fdata;
				char *tmp = NULL;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret =  stat(param->content,&fdata);
				if(0 == param->ret)
				{
					tmp = (char*)(&fdata);
					memcpy(param->content,tmp,sizeof(struct stat));
				}
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_ULINK:
			{
				if(NULL == param)
				{
					printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
					break;
				}
				param->content[MAX_FILE_PATH-1] = 0;
				param->ret =  unlink(param->content);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_MKDIR:
			{
				if(NULL == param)
				{
					printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
					break;
				}
				param->content[MAX_FILE_PATH-1] = 0;
				param->ret =  mkdir(param->content, (mode_t)param->mode);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_RMDIR:
			{
				if(NULL == param)
				{
					printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
					break;
				}
				param->content[MAX_FILE_PATH-1] = 0;
				param->ret =  rmdir(param->content);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_OPENDIR:
			{
				DIR * fdir = NULL;
				if(NULL == param)
				{
					printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
					break;
				}
				param->content[MAX_FILE_PATH-1] = 0;
				fdir = opendir(param->content);
				param->fdir= fdir;
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_CLOSEDIR:
			{
				if(NULL == param)
				{
					printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
					break;
				}
				param->ret =  closedir((DIR *)(param->fdir));
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_READDIR:
			{
				struct dirent* dirp = NULL;
				if(NULL == param)
				{
					printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
					break;
				}
				dirp =  readdir((DIR *)(param->fdir));
				if (dirp != NULL) {
					memcpy(&param->dirent, dirp, sizeof(struct dirent));
					param->ret = 0;
				} else {
					param->ret = -1;
				}
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_ACCESS:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret =  access(param->content, param->flag);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_REMOVE:
			{
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				param->ret =  remove(param->content);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		case IPC_FS_STATFS:
			lxc_ipc_done(&ipc_mp);
			break;
		case IPC_FS_FADVICE:
			if(NULL == param)
			{
				//printf("%s %d param is null\n",__FUNCTION__,__LINE__);
				lxc_ipc_done(&ipc_mp);
				break;
			}      
			param->ret = posix_fadvise(param->fd, param->pos, param->size, param->memoff);
			lxc_ipc_done(&ipc_mp);
			break;
		case IPC_FS_SEVER_CLOSE:
			lxc_ipc_close_accepted(&ipc_mp);
			lxc_ipc_done(&ipc_mp);
			goto exit;
		case IPC_FS_RENAME:
			{
				char *old_filename=NULL;
				char *new_filename=NULL;
				if(NULL == param)
				{
					printf("%s %d param is null\n",__FUNCTION__,__LINE__);
					lxc_ipc_done(&ipc_mp);
					break;
				}
				old_filename = param->content;
				new_filename = param->content+IPC_MOUNT_PARAM_MAXLIMIT;
				param->ret = rename(old_filename,new_filename);
				lxc_ipc_done(&ipc_mp);
			}
			break;
		default:
			lxc_ipc_done(&ipc_mp);
			break;
		}
	}

exit:
	return NULL;
}

struct usb_dev_entry
{
	char* path;
	char* dev;
};
static struct usb_dev_entry usb_dev_table[] = {
	{"/sys/block/sda/sda1", "/dev/sda1"},
	{"/sys/block/sda/sda2", "/dev/sda2"},
	{"/sys/block/sda/sda3", "/dev/sda3"},
	{"/sys/block/sda/sda4", "/dev/sda4"},
	{"/sys/block/sda/sda5", "/dev/sda5"},
	{"/sys/block/sda/sdb1", "/dev/sdb1"},
	{"/sys/block/sda/sdb2", "/dev/sdb2"},
	{"/sys/block/sda/sdb3", "/dev/sdb3"},
	{"/sys/block/sda/sdb4", "/dev/sdb4"}
};
static int first_check_flag = 0;
static void check_usb_device_file(void) {
	int num = sizeof(usb_dev_table) /sizeof(struct usb_dev_entry);
	int i;
	for (i = 0; i < num; i ++) {
		if (access(usb_dev_table[i].path, 0) == 0) {
			ipc_show(USB_INSERT_EVENT,usb_dev_table[i].dev);
			return;
		}
	}
}
static void *ipc_usb_server(void *arg)
{
	struct lxc_ipc ipc_mp;
	void *in_buf_mp;
	void *out_buf_mp;
	size_t in_size_mp;		/* size should more than all param */
	size_t out_size_mp;		/* size should more than all result */
	int do_what_mp;
	struct ipc_mp_param *param = NULL;
	char name_mp[16];

	ipc_mp = *((struct lxc_ipc *)arg);
	sem_post(&plugevent_sem_mp);

	snprintf(name_mp, sizeof(name_mp), "ipc_usb_server_%d", ipc_mp.tid);
	mt_set_pthread_name(name_mp);

	while (1) {
		lxc_ipc_accept(&ipc_mp, &do_what_mp, &in_buf_mp, &out_buf_mp, &in_size_mp, &out_size_mp);
		/* in_size_mp and out_size_mp was be update by kernel */

		param =  (struct ipc_mp_param *)in_buf_mp;
#if IPC_FS_DEBUG
		printf("%s %d do_what_usb = %d\n",__FUNCTION__,__LINE__,do_what_mp);
#endif
		switch (do_what_mp) {
		case IPC_FS_REGISTER:
			if(NULL == in_buf_mp)
			{
				printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
				lxc_ipc_done(&ipc_mp);
				break;
			}
			param->ret = ipc_insert(&ipc_mp,in_buf_mp);
			if(-2 == param->ret)
			{
				lxc_ipc_done(&ipc_mp);
			}
			lxc_ipc_done(&ipc_mp);
			first_check_flag = 1;
			break;
		case IPC_FS_UNREGISTER:
			if(NULL == in_buf_mp)
			{
				printf("%s %d in_buf_mp = %p\n",__FUNCTION__,__LINE__,in_buf_mp);
				lxc_ipc_done(&ipc_mp);
				break;
			}
			param->ret = ipc_delete(&ipc_mp);
			param->mode = IPC_EVENT_UNREGISTER_FLAG;
			lxc_ipc_done(&ipc_mp);
			lxc_ipc_done(&ipc_mp);
			break;
		case IPC_FS_USBEVENT:
			if (first_check_flag) {
				check_usb_device_file();
				first_check_flag = 0;
			}
			break;
		case IPC_FS_SEVER_CLOSE:
			lxc_ipc_close_accepted(&ipc_mp);
			lxc_ipc_done(&ipc_mp);
			goto exit;
		default:
			break;
		}
	}

exit:
	return NULL;
}


static void *usb_thread(void *arg)
{
	struct sockaddr_nl snl;
	int snl_fd = 0;
	ssize_t len = 0;
	char *p = NULL;
	char disk_partition[32] = {0};
	char dev[32] = {0};

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = (__u32)getpid();
	snl.nl_groups = 1; /* must be 1, because kernel set it as 1 when create NETLINK_KOBJECT_UEVENT */

	snl_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (snl_fd == -1) {
		printf("socket failed\n");
		return NULL;
	}

	bind(snl_fd, (void *)&snl, sizeof(snl));

	while (1) {
		len = recv(snl_fd, uevent, sizeof(uevent), 0);
		if (len == -1) {
			printf("recv failed\n");
		} else if (len == 0) {
			break;
		} else {
#if IPC_FS_DEBUG
			printf("%s %d %s\n\n",__FUNCTION__,__LINE__,uevent);
#endif
		}

		if (strstr(uevent, "block/sd")) {
			if (strstr(uevent, "add@")) {
				p = strstr(uevent, "block/sd");
				p += strlen("block/sd");
				if (*(p + 1) == '\0') {
					snprintf(disk_partition, sizeof(disk_partition), "sd%c", *p);
					snprintf(dev, sizeof(dev), "/dev/%s", disk_partition);
				} else if (*(p + 1) == '/') {
					snprintf(disk_partition, sizeof(disk_partition), "sd%c%s", *p, p + 5);
					snprintf(dev, sizeof(dev), "/dev/%s", disk_partition);
#if IPC_FS_DEBUG
					printf("%s %d dev: %s \n",__FUNCTION__,__LINE__,dev);
#endif
					ipc_show(USB_INSERT_EVENT,dev);
				}
			}
			else if (strstr(uevent, "remove@")) {
				p = strstr(uevent, "block/sd");
				p += strlen("block/sd");
				if (*(p + 1) == '\0') {
					/* don't rmdir /mnt/sda */
				} else if (*(p + 1) == '/') {
					snprintf(disk_partition, sizeof(disk_partition), "sd%c%s", *p, p + 5);
					snprintf(dev, sizeof(dev), "/dev/%s", disk_partition);
					ipc_show(USB_REMOVE_EVENT,dev);
					/* don't rmdir /mnt/sda1 */
				}
			}
		}
	}

	return NULL;
}

static void *plugevent_thread(void *arg)
{
	char name_mp[LXC_IPC_NAME_SIZE] = "ipc_usb_server";
	struct lxc_ipc ipc_mp_listen;
	struct lxc_ipc ipc_mp;
	pthread_t thread_mp;

	sem_init(&plugevent_sem_mp, 0, 0);

	if (lxc_ipc_open_server(name_mp, &ipc_mp_listen)) {
		exit(1);
	}

	while (1) {
		lxc_ipc_listen(&ipc_mp_listen, &ipc_mp);
		pthread_create(&thread_mp, NULL, ipc_usb_server, &ipc_mp);
		pthread_detach(thread_mp);
		sem_wait(&plugevent_sem_mp);
	}

	lxc_ipc_close(&ipc_mp_listen);
}

int main(int argc, char **argv)
{
	char name_mp[LXC_IPC_NAME_SIZE] = "ipc_fs_server";
	struct lxc_ipc ipc_mp_listen;
	struct lxc_ipc ipc_mp;
	pthread_t thread_mp;
	pthread_t usbmount_thread;
	pthread_t usbevent_thread;

	printf("ipc_fs_server running\n");

	//daemon_init();
	sem_init(&listsem_mp, 0, 1);

	pthread_create(&usbmount_thread, NULL, usb_thread, NULL);
	pthread_create(&usbevent_thread, NULL, plugevent_thread, NULL);

	sem_init(&sem_mp, 0, 0);

	if (lxc_ipc_open_server(name_mp, &ipc_mp_listen)) {
		exit(1);
	}

	while (1) {
		lxc_ipc_listen(&ipc_mp_listen, &ipc_mp);
		pthread_create(&thread_mp, NULL, ipc_fs_server, &ipc_mp);
		pthread_detach(thread_mp);
		sem_wait(&sem_mp);
	}

	lxc_ipc_close(&ipc_mp_listen);

	pthread_join(usbmount_thread, NULL);
	pthread_join(usbevent_thread, NULL);

	return 0;
}
