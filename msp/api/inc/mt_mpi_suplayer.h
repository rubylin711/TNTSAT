/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_suplayer.h
  Version       : Initial Draft
  Author        : montage multimedia software group
  Created       : 2015/11/25
  Description   :
  History       :
  1.Date        : 2015/11/25
    Author      :
    Modification: Created file
********************************************************************************************/
#ifndef __MT_MPI_SUPLAYER_H__
#define __MT_MPI_SUPLAYER_H__

#include "mt_drv_suplayer.h"
#include "drv_suplayer_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
	extern "C"
	{
#endif
#endif
mt_s32 MT_MPI_SUPLAYER_Init(mt_void);
mt_s32 MT_MPI_SUPLAYER_DeInit(mt_void);
mt_s32 MT_MPI_SUPLAYER_Create(void *pstAvAttr, mt_handle *phSuplayer);
mt_s32 MT_MPI_SUPLAYER_Destroy(mt_handle hSuplayer);
mt_s32 MT_MPI_SUPLAYER_Get_Proc_Info(mt_handle hSuplayer, SUPLAYER_S **psuplayerdata);
void SUPLAYER_ThreadMutex_Lock(pthread_mutex_t *ss);
void SUPLAYER_ThreadMutex_UnLock(pthread_mutex_t *ss);
void SUPLAYER_Mutex_Lock(pthread_mutex_t *ss);
void SUPLAYER_Mutex_UnLock(pthread_mutex_t *ss);
mt_s32 SUPLAYER_CheckHandle(mt_handle hSuplayer, SUPLAYER_USR_ADDR_S  *pSuplayerUsrAddr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
