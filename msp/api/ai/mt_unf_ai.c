/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_module.h"

#include "mt_mpi_ai.h"
#include "mt_error_mpi.h"


mt_s32 MT_UNF_AI_Init(mt_void)
{
  printf("\n unf layer MT_UNF_AI_Init \n");
    return MT_MPI_AI_Init();    
}

mt_s32   MT_UNF_AI_DeInit(mt_void)
{

    return MT_MPI_AI_DeInit();
}

mt_s32   MT_UNF_AI_GetDefaultAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr)
{

    return MT_MPI_AI_GetDefaultAttr(enAiPort,pstAttr);
}

mt_s32 MT_UNF_AI_SetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr)
{

    return MT_MPI_AI_SetAttr(hAI,pstAttr);
}

mt_s32 MT_UNF_AI_GetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr)
{

    return MT_MPI_AI_GetAttr(hAI,pstAttr);
}

mt_s32 MT_UNF_AI_Create(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, mt_handle *phandle)
{

    return MT_MPI_AI_Create(enAiPort,pstAttr,phandle);
}

mt_s32 MT_UNF_AI_Destroy(mt_handle hAI)
{
    return MT_MPI_AI_Destroy(hAI);
}

mt_s32 MT_UNF_AI_SetEnable(mt_handle hAI, MT_BOOL bEnable)
{
    return MT_MPI_AI_SetEnable(hAI,bEnable);
}

mt_s32 MT_UNF_AI_GetEnable(mt_handle hAI, MT_BOOL *pbEnable)
{
    return MT_MPI_AI_GetEnable(hAI, pbEnable);
}

mt_s32 MT_UNF_AI_AcquireFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame, mt_u32 u32TimeoutMs)
{
    return MT_MPI_AI_AcquireFrame(hAI,pstFrame);
}

mt_s32 MT_UNF_AI_ReleaseFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    return MT_MPI_AI_ReleaseFrame(hAI,pstFrame);
}

mt_s32 MT_UNF_AI_SetDelay(mt_handle hAI, const MT_UNF_AI_DELAY_S *pstDelay)
{
    return MT_ERR_AI_NOTSUPPORT;
}

mt_s32 MT_UNF_AI_GetDelay(mt_handle hAI, MT_UNF_AI_DELAY_S *pstDelay)
{
    return MT_ERR_AI_NOTSUPPORT;
}    

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
