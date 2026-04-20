/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_suplayer.c
  Version       : Initial Draft
  Author        : Montage software group
  Created       : 2015/11/25
  Description   : Common definitions of MT_CODEC(video).
                  The codec wants to register to MT_CODEC need to adapt to MT_CODEC_S.
  History       :
  1.Date        : 2015/11/25
  Author      :
  Modification: Created file

*******************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>

#include "mt_common.h"
#include "mt_mpi_suplayer.h"
#include "mt_error_mpi.h"
#include "mt_mpi_mem.h"
#include "mt_module.h"
#include "mt_drv_struct.h"
#include <sys/syscall.h>
#include "mt_mpi_disp.h"
#include "mt_module_debug.h"
#include "mt_drv_adec.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#ifndef SUPLAYER_BUG
#define SUPLAYER_BUG()	do { \
							MT_ERR_SUPLAYER("[*BUG] %s: AVPlay Bug @%s:%d\n",__FUNCTION__,__FILE__,__LINE__); \
						} while (0)
#endif

#define SUPLAYER_AUD_SPEED_ADJUST_SUPPORT

static mt_s32            g_SuplayerDevFd    = -1;
static const mt_char     g_SuplayerDevName[] ="/dev/"UMAP_DEVNAME_SUPLAYER;
static pthread_mutex_t   g_SuplayerMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t   g_SuplayerResMutex[SUPLAYER_MAX_NUM] = {PTHREAD_MUTEX_INITIALIZER};

static const mt_u8 s_szSUPLAYERVersion[] __attribute__((used)) = "SDK_VERSION:["\
                            MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
                            __DATE__", "__TIME__"]";


//static mt_u32 u32ThreadMutexCount = 0, u32SuplayerMutexCount = 0;
void SUPLAYER_ThreadMutex_Lock(pthread_mutex_t *ss)
{
    //u32ThreadMutexCount ++;
    //MT_INFO_SUPLAYER("lock u32ThreadMutexCount:%d\n", u32ThreadMutexCount);
    pthread_mutex_lock(ss);
}

void SUPLAYER_ThreadMutex_UnLock(pthread_mutex_t *ss)
{
    //u32ThreadMutexCount --;
    //MT_INFO_SUPLAYER("unlock u32ThreadMutexCount:%d\n", u32ThreadMutexCount);
    pthread_mutex_unlock(ss);
}

void SUPLAYER_Mutex_Lock(pthread_mutex_t *ss)
{
    //u32SuplayerMutexCount ++;
    //MT_INFO_SUPLAYER("lock u32SuplayerMutexCount:%d\n", u32SuplayerMutexCount);
    pthread_mutex_lock(ss);
}

void SUPLAYER_Mutex_UnLock(pthread_mutex_t *ss)
{
    //u32SuplayerMutexCount --;
    //MT_INFO_SUPLAYER("unlock u32SuplayerMutexCount:%d\n", u32SuplayerMutexCount);
    pthread_mutex_unlock(ss);
}

#define MT_SUPLAYER_LOCK()        (void)pthread_mutex_lock(&g_SuplayerMutex);
#define MT_SUPLAYER_UNLOCK()      (void)pthread_mutex_unlock(&g_SuplayerMutex);

#define MT_SUPLAYER_INST_LOCK()        \
do{\
    if((hSuplayer & 0xff) >=SUPLAYER_MAX_NUM)\
    {\
        MT_ERR_SUPLAYER("avplay support %d instance, but this para:%d is illegal\n", SUPLAYER_MAX_NUM, (hSuplayer & 0xff));\
        return MT_ERR_SUPLAYER_INVALID_PARA;\
    }\
    (void)pthread_mutex_lock(&g_SuplayerResMutex[(hSuplayer & 0xff)]);\
}while(0)

#define MT_SUPLAYER_INST_UNLOCK()        \
do{\
    if((hSuplayer & 0xff)>=SUPLAYER_MAX_NUM)\
    {\
        MT_ERR_SUPLAYER("avplay support %d instance, but this para:%d is illegal\n", SUPLAYER_MAX_NUM, (hSuplayer & 0xff));\
        return MT_ERR_SUPLAYER_INVALID_PARA;\
    }\
    (void)pthread_mutex_unlock(&g_SuplayerResMutex[(hSuplayer & 0xff)]);\
}while(0)

#define SUPLAYER_GET_INST_AND_LOCK()\
do{\
    MT_SUPLAYER_LOCK();\
    if (g_SuplayerDevFd < 0)\
    {\
        MT_ERR_SUPLAYER("SUPLAYER is not init.\n");\
        MT_SUPLAYER_UNLOCK();\
        return MT_ERR_SUPLAYER_DEV_NO_INIT;\
    }\
    MT_SUPLAYER_UNLOCK();\
    MT_SUPLAYER_INST_LOCK(); \
    memset(&SuplayerUsrAddr, 0, sizeof(SuplayerUsrAddr)); \
    Ret = SUPLAYER_CheckHandle(hSuplayer, &SuplayerUsrAddr);\
    if (Ret != MT_SUCCESS)\
    {\
        MT_SUPLAYER_INST_UNLOCK();\
        return MT_ERR_SUPLAYER_INVALID_PARA;\
    }\
    pSuplayer = (SUPLAYER_S *)SuplayerUsrAddr.SuplayerUsrAddr;\
}while(0)

mt_s32 SUPLAYER_CheckHandle(mt_handle hSuplayer, SUPLAYER_USR_ADDR_S  *pSuplayerUsrAddr)
{
    if ((hSuplayer & 0xffff0000) != (MT_ID_SUPLAYER << 16))
    {
        MT_WARN_SUPLAYER("this is invalid handle : 0x%x.\n",hSuplayer);
        return MT_ERR_SUPLAYER_INVALID_PARA;
    }

    pSuplayerUsrAddr->SuplayerId = hSuplayer & 0xff;

    /* check if the handle is valid */
    return ioctl(g_SuplayerDevFd, CMD_SUPLAYER_CHECK_ID, pSuplayerUsrAddr);
}

mt_s32 MT_MPI_SUPLAYER_Init(mt_void)
{
//    mt_s32 Ret;

    MT_SUPLAYER_LOCK();

    // already opened in this process
    if (g_SuplayerDevFd > 0)
    {
        MT_SUPLAYER_UNLOCK();

        return MT_SUCCESS;
    }

    g_SuplayerDevFd = open(g_SuplayerDevName, O_RDWR | O_NONBLOCK, 0);

    if (g_SuplayerDevFd < 0)
    {
        MT_SUPLAYER_UNLOCK();

        MT_FATAL_SUPLAYER("open %s error\n", g_SuplayerDevName);

        return MT_ERR_SUPLAYER_DEV_OPEN_ERR;
    }

    MT_SUPLAYER_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_SUPLAYER_DeInit(mt_void)
{
    mt_s32  Ret;
    mt_u32  SuplayerNum = 0;

    MT_SUPLAYER_LOCK();

    if (g_SuplayerDevFd < 0)
    {
        MT_SUPLAYER_UNLOCK();
        return MT_SUCCESS;
    }

    if (SuplayerNum)
    {
        MT_ERR_SUPLAYER("there are %d SUPLAYER not been destroied.\n", SuplayerNum);
        MT_SUPLAYER_UNLOCK();
        return MT_ERR_SUPLAYER_INVALID_OPT;
    }

    Ret = close(g_SuplayerDevFd);
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_SUPLAYER("DeInit SUPLAYER err.\n");
        MT_SUPLAYER_UNLOCK();
        return MT_ERR_SUPLAYER_DEV_CLOSE_ERR;
    }

    g_SuplayerDevFd = -1;

    MT_SUPLAYER_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_SUPLAYER_Create(/*const MT_UNF_SUPLAYER_ATTR_S *pstAvAttr*/void *pstAvAttr, mt_handle *phSuplayer)
{
    SUPLAYER_S               *pSuplayer = MT_NULL;
    SUPLAYER_CREATE_S        SuplayerCreate;
    SUPLAYER_USR_ADDR_S      SuplayerUsrAddr;
//    mt_u32                 i;
    mt_s32                 Ret = 0;

    if (!pstAvAttr)
    {
        MT_ERR_SUPLAYER("para pstAvAttr is null.\n");
        return MT_ERR_SUPLAYER_NULL_PTR;
    }

    if (!phSuplayer)
    {
        MT_ERR_SUPLAYER("para phSuplayer is null.\n");
        return MT_ERR_SUPLAYER_NULL_PTR;
    }

    MT_SUPLAYER_LOCK();

    if (g_SuplayerDevFd < 0)
    {
        MT_ERR_SUPLAYER("SUPLAYER is not init.\n");
        MT_SUPLAYER_UNLOCK();
        return MT_ERR_SUPLAYER_DEV_NO_INIT;
    }

    MT_SUPLAYER_UNLOCK();

    /* create suplayer */
    Ret = ioctl(g_SuplayerDevFd, CMD_SUPLAYER_CREATE, &SuplayerCreate);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_SUPLAYER("SUPLAYER CMD_SUPLAYER_CREATE failed.\n");
        goto RET;
    }

    /* remap the memories allocated in kernel space to user space */
    //Bug: 0x2000 < sizeof(SUPLAYER_S)
    //pSuplayer = (SUPLAYER_S *)(mt_mmap(SuplayerCreate.SuplayerPhyAddr, 0x2000));
    pSuplayer = (SUPLAYER_S *)(mt_mmap(SuplayerCreate.SuplayerPhyAddr, sizeof(SUPLAYER_S)));
    MT_INFO_SUPLAYER("SuplayerCreate.SuplayerPhyAddr:0x%x, sizeof(SUPLAYER_S):0x%x, sizeof(pthread_mutex_t):0x%x\n", SuplayerCreate.SuplayerPhyAddr, sizeof(SUPLAYER_S), sizeof(pthread_mutex_t));
    if (!pSuplayer)
    {
        MT_ERR_SUPLAYER("SUPLAYER memmap failed.\n");
        Ret = MT_ERR_SUPLAYER_CREATE_ERR;
        goto SUPLAYER_DESTROY;
    }

    pSuplayer->pSuplayerThreadMutex = (pthread_mutex_t *)mt_malloc(MT_ID_SUPLAYER, sizeof(pthread_mutex_t));
    if (pSuplayer->pSuplayerThreadMutex == MT_NULL)
    {
        Ret = MT_ERR_SUPLAYER_CREATE_ERR;
        goto SUPLAYER_UNMAP;
    }
    (mt_void)pthread_mutex_init(pSuplayer->pSuplayerThreadMutex, NULL);

    SuplayerUsrAddr.SuplayerId = SuplayerCreate.SuplayerId;
    SuplayerUsrAddr.SuplayerUsrAddr = (ulong)pSuplayer;

    //printf("SuplayerUsrAddr.SuplayerUsrAddr = %x ROCK \n",  &SuplayerUsrAddr);

    Ret = ioctl(g_SuplayerDevFd, CMD_SUPLAYER_SET_USRADDR, &SuplayerUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_SUPLAYER("SUPLAYER set user addr failed.\n");
        goto DESTROY_THREAD_MUTEX;
    }

    SUPLAYER_Mutex_Lock(&g_SuplayerResMutex[SuplayerCreate.SuplayerId]);
    SUPLAYER_Mutex_Lock(pSuplayer->pSuplayerThreadMutex);

    /* record stream attributes */
    //memcpy(&pSuplayer->SuplayerAttr, pstAvAttr, sizeof(MT_UNF_SUPLAYER_ATTR_S));

    pSuplayer->hSuplayer = (MT_ID_SUPLAYER << 16) | SuplayerCreate.SuplayerId;

    *phSuplayer = (MT_ID_SUPLAYER << 16) | SuplayerCreate.SuplayerId;
    pSuplayer->ctrl.dump_aes = 0;
    pSuplayer->ctrl.dump_ses = 0;
    pSuplayer->ctrl.dump_ves = 0;
    pSuplayer->ctrl.send_aud = 1;
    pSuplayer->ctrl.send_subt = 1;
    pSuplayer->ctrl.send_vid = 1;

    SUPLAYER_Mutex_UnLock(pSuplayer->pSuplayerThreadMutex);
    SUPLAYER_Mutex_UnLock(&g_SuplayerResMutex[SuplayerCreate.SuplayerId]);

    return     MT_SUCCESS;

//SUPLAYER_UNLOCK:
//    SUPLAYER_Mutex_UnLock(pSuplayer->pSuplayerThreadMutex);
//    SUPLAYER_Mutex_UnLock(&g_SuplayerResMutex[SuplayerCreate.SuplayerId]);


DESTROY_THREAD_MUTEX:
    (mt_void)pthread_mutex_destroy(pSuplayer->pSuplayerThreadMutex);
    mt_free(MT_ID_SUPLAYER, (mt_void*)(pSuplayer->pSuplayerThreadMutex));

SUPLAYER_UNMAP:
    (mt_void)mt_munmap(pSuplayer);

SUPLAYER_DESTROY:
    (mt_void)ioctl(g_SuplayerDevFd, CMD_SUPLAYER_DESTROY, &(SuplayerCreate.SuplayerId));

RET:
    return Ret;
}

mt_s32 MT_MPI_SUPLAYER_Destroy(mt_handle hSuplayer)
{
    SUPLAYER_S           *pSuplayer = MT_NULL;
    SUPLAYER_USR_ADDR_S  SuplayerUsrAddr;
    mt_s32             Ret;

    SUPLAYER_GET_INST_AND_LOCK();

    /* stop thread */
    (mt_void)pthread_mutex_destroy(pSuplayer->pSuplayerThreadMutex);
    mt_free(MT_ID_SUPLAYER, (mt_void*)(pSuplayer->pSuplayerThreadMutex));

	(mt_void)mt_munmap((mt_void *)SuplayerUsrAddr.SuplayerUsrAddr);
    Ret = ioctl(g_SuplayerDevFd, CMD_SUPLAYER_DESTROY, &SuplayerUsrAddr.SuplayerId);
    if (Ret != MT_SUCCESS)
    {
        MT_SUPLAYER_INST_UNLOCK();
        return Ret;
    }

    MT_SUPLAYER_INST_UNLOCK();

    return MT_SUCCESS ;
}

mt_s32 MT_MPI_SUPLAYER_Get_Proc_Info(mt_handle hSuplayer, SUPLAYER_S **psuplayerdata)
{
    SUPLAYER_S           *pSuplayer = MT_NULL;
    SUPLAYER_USR_ADDR_S  SuplayerUsrAddr;
    mt_s32             Ret;

    SUPLAYER_GET_INST_AND_LOCK();
    *psuplayerdata = pSuplayer;
    MT_SUPLAYER_INST_UNLOCK();
    return MT_SUCCESS ;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


