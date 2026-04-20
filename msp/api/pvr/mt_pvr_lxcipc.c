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

#include "mt_common.h"
#include "lxc_ipc.h"
#include "mt_unf_ipcfs_type.h"
#include "mt_pvr_fs_def.h"

static pthread_mutex_t PvrLxcIpc_Lock = PTHREAD_MUTEX_INITIALIZER;
//static ipc_fs_handle_t g_ipc_pvr;
volatile static int g_flag_init=0;

#define pvripc_print(x...) do {} while (0)
//#define pvripc_print printf
#define MAX_CLIENT  12
typedef struct managclient
{
    pid_t tid;
    int dio;
    int fd;
    ipc_fs_handle_t ipc_pvr;
}MANAGCLIENT;
volatile MANAGCLIENT g_ipc_client[MAX_CLIENT] =
{
	{.tid = -1, .dio = 0, .fd = -1}
};

static void mt_pvr_deltid(int pos)
{
    pthread_mutex_lock(&PvrLxcIpc_Lock);
    {                                  //thread exit, so free mem
        g_ipc_client[pos].fd=-1;
        //printf("+++pvr.free pos=%x,%x\n",pos,g_ipc_client[pos].tid);
        g_ipc_client[pos].tid=-1;
        g_ipc_client[pos].dio=0;
        ipc_fs_deinit(&g_ipc_client[pos].ipc_pvr);//ipc_fs_freebuffer(&g_ipc_client[pos].ipc_pvr);
    }
    pthread_mutex_unlock(&PvrLxcIpc_Lock);
}

static int mt_pvr_findtid(int tid,int dio)
{
    int i=0;
    pthread_mutex_lock(&PvrLxcIpc_Lock);
    if(0==g_flag_init){                                 //init here!
        memset(g_ipc_client,0x00,sizeof(g_ipc_client));
        for(i=0;i<MAX_CLIENT;i++){
            g_ipc_client[i].fd=-1;
            g_ipc_client[i].tid=-1;
            g_ipc_client[i].dio=0;
        }
        g_flag_init=1;
    }
    for(i=0;i<MAX_CLIENT;i++){                          //find same
        if((tid==g_ipc_client[i].tid)&&(dio==g_ipc_client[i].dio)){
            break;
        }
    }
    pthread_mutex_unlock(&PvrLxcIpc_Lock);
    return i;
}
static int mt_pvr_findtid_diofd(int tid,int dio,int fd)
{
    int i=0;
    pthread_mutex_lock(&PvrLxcIpc_Lock);
    if(0==g_flag_init){                                 //init here!
        memset(g_ipc_client,0x00,sizeof(g_ipc_client));
        for(i=0;i<MAX_CLIENT;i++){
            g_ipc_client[i].fd=-1;
            g_ipc_client[i].tid=-1;
            g_ipc_client[i].dio=0;
        }
        g_flag_init=1;
    }
    for(i=0;i<MAX_CLIENT;i++){                          //find same
        if((tid==g_ipc_client[i].tid)&&(dio==g_ipc_client[i].dio)&&(fd==g_ipc_client[i].fd)){
            break;
        }
    }
    pthread_mutex_unlock(&PvrLxcIpc_Lock);
    return i;
}

static int mt_pvr_getblankpos(int tid)
{
    int i=0;
    pthread_mutex_lock(&PvrLxcIpc_Lock);
    for(i=0;i<MAX_CLIENT;i++){
        if(-1==g_ipc_client[i].tid){
            break;
        }
    }
    if(MAX_CLIENT != i){
        char pvr_name[LXC_IPC_NAME_SIZE] = "ipc_fs_server";
        (void)ipc_fs_init(&g_ipc_client[i].ipc_pvr,pvr_name);
        g_ipc_client[i].tid=tid;
        g_ipc_client[i].fd=-1;
        g_ipc_client[i].dio=0;
    }
    pthread_mutex_unlock(&PvrLxcIpc_Lock);
    return i;
}
static void mt_pvr_setnocopyfd(int pos,int fd)
{
    if(MAX_CLIENT > pos){
        pthread_mutex_lock(&PvrLxcIpc_Lock);
        g_ipc_client[pos].fd=fd;
        g_ipc_client[pos].dio=1;
        pthread_mutex_unlock(&PvrLxcIpc_Lock);
    }
}

void mt_pvr_ipc_clear_tid(void)
{
    int pos;
    pid_t tid = gettid();

    pos=mt_pvr_findtid(tid,0);
    pvripc_print("++pvr cleartid i %x\n",pos);
    if(MAX_CLIENT!=pos){
        mt_pvr_deltid(pos);
    }
    pos=mt_pvr_findtid(tid,1);
    pvripc_print("++pvr cleartid i %x\n",pos);
    if(MAX_CLIENT!=pos){
        mt_pvr_deltid(pos);
    }
    pvripc_print("++pvr cleartid o %x\n",pos);
}
void* mt_pvr_ipc_getbufferby_tid(int fd,int size,off_t *in_phy)
{
    int pos;
    void *buff=NULL;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();

    pos=mt_pvr_findtid(tid,1);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return NULL;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr getbuf i %x\n",pos);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
    	//FIXME
        (void)ipc_fs_getinbuffer(ipc_fp,&buff,in_phy,size);
        mt_pvr_setnocopyfd(pos,fd);
    }
    pvripc_print("++pvr getbuf o %x %x\n",pos,buff);
    return buff;
}
int mt_pvr_ipc_open(const char *path, int flag,unsigned int mode)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();

    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr open i %x,%s,%x,%x\n",pos,path,flag,mode);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_open(ipc_fp,path,flag,mode);
    }
    pvripc_print("++pvr open o %x,%s,%x,%x\n",pos,path,flag,mode,ret);

    return ret;
}
int mt_pvr_ipc_close(int fd)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr close i %x,%x,%x\n",pos,fd,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_close(ipc_fp,fd);
    }
    pvripc_print("++pvr close o %x,%x,%x\n",pos,fd,ret);
    return ret;
}

int mt_pvr_ipc_fadvise(int fd, off_t offset, off_t len, int advice)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pvripc_print("++pvr fad i1 %d,%x,%x,%x,%llx,%x\n",pos,fd,len,offset,ret);
    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr fad i2 %d,%x,%x,%x,%llx,%x\n",pos,fd,len,offset,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_fadvise(ipc_fp,fd,offset,len,advice);
    }
    pvripc_print("++pvr fad o3 %d,%x,%x,%x,%llx,%x\n",pos,fd,len,offset,ret);
    return ret;
}

int mt_pvr_ipc_pread(int fd,void *buff,size_t size,off_t offset)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pvripc_print("++pvr pr i1 %d,%x,%x,%x,%llx,%x\n",pos,fd,(int)buff,size,offset,ret);
    pos=mt_pvr_findtid_diofd(tid,1,fd);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_findtid(tid,0);
    }
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr pr i2 %d,%x,%x,%x,%llx,%x\n",pos,fd,(int)buff,size,offset,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        if(g_ipc_client[pos].fd==fd){
            ret=ipc_fs_pread_nocopy(ipc_fp,fd,buff,size,offset);
        }else{
            ret=ipc_fs_pread(ipc_fp,fd,buff,size,offset);
        }
    }
    pvripc_print("++pvr pr o3 %d,%x,%x,%x,%llx,%x\n",pos,fd,(int)buff,size,offset,ret);
    return ret;
}
int mt_pvr_ipc_pwrite(int fd,void *buff,size_t size,off_t offset)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pvripc_print("++pvr pw i1 %d,%x,%x,%x,%llx,%x\n",pos,fd,(int)buff,size,offset,ret);
    pos=mt_pvr_findtid_diofd(tid,1,fd);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_findtid(tid,0);
    }
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr pw i2 %d,%x,%x,%x,%llx,%x\n",pos,fd,(int)buff,size,offset,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        if(g_ipc_client[pos].fd==fd){
            ret=ipc_fs_pwrite_nocopy(ipc_fp,fd,buff,size,offset);
        }else{
            ret=ipc_fs_pwrite(ipc_fp,fd,buff,size,offset);
        }
    }
    pvripc_print("++pvr pw o3 %d,%x,%x,%x,%llx,%x\n",pos,fd,(int)buff,size,offset,ret);
    return ret;
}
int mt_pvr_ipc_read(int fd,void *buff,size_t size)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pvripc_print("++pvr r i1 %x,%x,%x,%x,%x\n",pos,fd,(int)buff,size,ret);
    pos=mt_pvr_findtid_diofd(tid,1,fd);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_findtid(tid,0);
    }
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr r i2 %x,%x,%x,%x,%x\n",pos,fd,(int)buff,size,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_read(ipc_fp,fd,buff,size);
    }
    pvripc_print("++pvr r o3 %x,%x,%x,%x,%x\n",pos,fd,(int)buff,size,ret);
    return ret;
}
int mt_pvr_ipc_write(int fd,void *buff,size_t size)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pvripc_print("++pvr w i1 %x,%x,%x,%x,%x\n",pos,fd,(int)buff,size,ret);
    pos=mt_pvr_findtid_diofd(tid,1,fd);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_findtid(tid,0);
    }
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr w i2 %x,%x,%x,%x,%x\n",pos,fd,(int)buff,size,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_write(ipc_fp,fd,buff,size);
    }
    pvripc_print("++pvr w o3 %x,%x,%x,%x,%x\n",pos,fd,(int)buff,size,ret);
    return ret;
}
int mt_pvr_ipc_lseek(int fd,off_t offset,int mode)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr lseek i %x,%x,%llx,%x,%x\n",pos,fd,offset,mode,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_lseek(ipc_fp,fd,offset,mode);
    }
    pvripc_print("++pvr lseek o %x,%x,%llx,%x,%x\n",pos,fd,offset,mode,ret);
    return ret;
}
int mt_pvr_ipc_fsync(int fd)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;

    pid_t tid = gettid();
    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr fsync i %x,%x,%x\n",pos,fd,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_sync(ipc_fp,fd);
    }
    pvripc_print("++pvr fsync o %x,%x,%x\n",pos,fd,ret);
    return ret;
}
int mt_pvr_ipc_access(char *filename, int mode)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr access i %x,%x\n",pos,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_access(ipc_fp,filename, mode);
    }
    pvripc_print("++pvr access o %x,%x\n",pos,ret);
    return ret;
}
int mt_pvr_ipc_remove(char *filename)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr remove i %x,%x\n",pos,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_remove(ipc_fp,filename);
    }
    pvripc_print("++pvr remove o %x,%x\n",pos,ret);
    return ret;
}

int mt_pvr_ipc_rename(char *oldfilename, char *newfilename)
{
    int pos ,ret=-1;
    ipc_fs_handle_t *ipc_fp=NULL;
    pid_t tid = gettid();
    pos=mt_pvr_findtid(tid,0);
    if(MAX_CLIENT==pos){
        pos=mt_pvr_getblankpos(tid);
        if(MAX_CLIENT==pos){
            pvripc_print("++pvr bad %s\n",__FUNCTION__);
            return -1;
        }
        pvripc_print("++pvr.first %s\n",__FUNCTION__);
    }
    pvripc_print("++pvr rename i %x,%x\n",pos,ret);
    ipc_fp=&g_ipc_client[pos].ipc_pvr;
    if(NULL!=ipc_fp){
        ret=ipc_fs_rename(ipc_fp,oldfilename,newfilename);
    }
    pvripc_print("++pvr rename o %x,%x\n",pos,ret);
    return ret;
}


