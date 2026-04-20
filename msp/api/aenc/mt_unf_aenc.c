/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_error_mpi.h"
#include "mt_unf_aenc.h"
//#include "mt_drv_aenc.h"
#define AENC_INSTANCE_MASK 0xffff
#if 0
#define API_AENC_CheckHandle(hAenc) \
    do{ \
        if ((MT_ID_AENC != (hAenc>>16)) || ((hAenc&AENC_INSTANCE_MASK) >= AENC_INSTANCE_MAXNUM)) \
        { \
            MT_ERR_AENC("invalid Aenc handle(%d).\n", hAenc); \
            return MT_ERR_AENC_INVALID_PARA; \
        } \
    }while(0)
#endif

mt_s32 MT_UNF_AENC_Init(mt_void)
{

    //return MT_MPI_AENC_Init(NULL);
    return 1;
}

mt_s32 MT_UNF_AENC_DeInit(mt_void)
{

    //return MT_MPI_AENC_DeInit();
    return 1;
}

mt_s32 MT_UNF_AENC_Create(const MT_UNF_AENC_ATTR_S *pstAencAttr, mt_handle *phAenc)
{
    mt_s32 ret = MT_ERR_AENC_NULL_PTR;

    if (NULL == pstAencAttr)
    {
        return MT_ERR_AENC_NULL_PTR;
    }

    //ret = MT_MPI_AENC_Open(phAenc, pstAencAttr);
    return ret;
}

mt_s32 MT_UNF_AENC_Destroy(mt_handle hAenc)
{

    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_Close(hAenc);
    return 1;

}

mt_s32 MT_UNF_AENC_AttachInput(mt_handle hAenc, mt_handle hSrc)
{
    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_AttachInput(hAenc,hSrc);
    return 1;
}

mt_s32 MT_UNF_AENC_DetachInput(mt_handle hAenc)
{
    //mt_s32 Ret;
    //mt_handle hSrc = MT_INVALID_HANDLE;

    //API_AENC_CheckHandle(hAenc);
#if 0
    Ret = MT_MPI_AENC_GetAttachSrc(hAenc, &hSrc);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AENC("call AENC_GetAttachSrc failed!\n");
        return Ret;
    }

    return MT_MPI_AENC_DetachInput(hAenc);
#endif
    return 1;
}

mt_s32 MT_UNF_AENC_Start(mt_handle hAenc)
{
    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_SetEnable(hAenc, MT_TRUE);
    return 1;
}

mt_s32 MT_UNF_AENC_Stop(mt_handle hAenc)
{
    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_SetEnable(hAenc, MT_FALSE);
    return 1;
}


mt_s32 MT_UNF_AENC_SendFrame(mt_handle hAenc, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{

    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_SendBuffer(hAenc, pstAOFrame);
    return 1;
}

mt_s32 MT_UNF_AENC_AcquireStream(mt_handle hAenc, MT_UNF_ES_BUF_S *pstStream, mt_u32 u32TimeoutMs)
{

    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_ReceiveStream(hAenc, (AENC_STREAM_S *)pstStream, u32TimeoutMs);
    return 1;
}

mt_s32 MT_UNF_AENC_ReleaseStream(mt_handle hAenc, const MT_UNF_ES_BUF_S *pstStream)
{

    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_ReleaseStream(hAenc, (AENC_STREAM_S *)pstStream);
    return 1;
}

mt_s32 MT_UNF_AENC_RegisterEncoder(const mt_char *pszCodecDllName)
{
    //return MT_MPI_AENC_RegisterEncoder(pszCodecDllName);
    return 1;
}

mt_s32 MT_UNF_AENC_SetAttr(mt_handle hAenc, const MT_UNF_AENC_ATTR_S *pstAencAttr)
{

    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_SetAttr(hAenc, pstAencAttr);
    return 1;
}

mt_s32 MT_UNF_AENC_GetAttr(mt_handle hAenc, MT_UNF_AENC_ATTR_S *pstAencAttr)
{

    //API_AENC_CheckHandle(hAenc);

    //return MT_MPI_AENC_GetAttr(hAenc, pstAencAttr);
    return 1;
}

