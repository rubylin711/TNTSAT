/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_type.h"
#include "mt_module.h"
#include <linux/string.h>
#include "mt_drv_mem.h"
#include "hal_cast.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_mem.h"
#include "mt_drv_module.h"

#include "mt_drv_ao.h"
#include "circ_buf.h"
#include "hal_aoe.h"
#include "hal_cast.h"
#include "audio_util.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

typedef struct
{
    mt_handle hCast[AIAO_CAST_BUTT];
} CAST_GLOBAL_SOURCE_S;

typedef struct
{
    mt_u32 uTotalByteWrite;
    mt_u32 uTryWriteCnt;
    mt_u32 uTotalByteRead;
    mt_u32 uTryReadCnt;
} CAST_PROC_STATUS_S;

typedef struct
{
    mt_u32 u32BufPhyAddr;  // hw aoe
    mt_u32 u32BufPhyWptr;  // hw aoe
    mt_u32 u32BufPhyRptr;  // hw aoe
    mt_u32 u32BufVirAddr;  // sw aoe
    mt_u32 u32BufVirWptr;  // sw aoe
    mt_u32 u32BufVirRptr;  // sw aoe
    mt_u32 u32BufSize;
} CAST_VIRTUAL_RBUF_S;

typedef struct
{
    AIAO_CAST_ATTR_S stUserAttr;

    /* internal state */
    AIAO_CAST_ID_E     enCast;
    mt_u32             u32BufFrameSize;
    AIAO_CAST_STATUS_E enCurnStatus;
    CIRC_BUF_S         stCB;
    CAST_PROC_STATUS_S stProc;
} CAST_CHN_STATE_S;

/* private state */
static CAST_GLOBAL_SOURCE_S g_CastRm;

#define CHECK_CAST_OPEN(Id) \
    do {\
        if (MT_NULL == g_CastRm.hCast[Id])\
        {\
            MT_ERR_AO("CAST(%d) is not create.\n", Id); \
            return MT_FAILURE; \
        } \
    } while (0)

static mt_void CASTFlushState(CAST_CHN_STATE_S *state)
{
    memset(&state->stProc, 0, sizeof(CAST_PROC_STATUS_S));
    CIRC_BUF_Flush(&state->stCB);
}
        
static      AIAO_CAST_ID_E  GetFreeCAST(mt_void)
{
    AIAO_CAST_ID_E enFreeCast;

    for (enFreeCast = AIAO_CAST_0; enFreeCast < AIAO_CAST_BUTT; enFreeCast++)
    {
        if (!g_CastRm.hCast[enFreeCast])
        {
            return enFreeCast;
        }
    }

    return AIAO_CAST_BUTT;
}

static mt_u32 UTIL_CalcFrameSize(mt_u32 uCh, mt_u32 uBitDepth)
{
    mt_u32 uFrameSize = 0;

    switch (uBitDepth)
    {
    case 16:
        uFrameSize = ((mt_u32)uCh) * sizeof(mt_u16);
        break;
    case 24:
        uFrameSize = ((mt_u32)uCh) * sizeof(mt_u32);
        break;
    }

    return uFrameSize;
}

/* global function */
mt_s32                  HAL_CAST_Init(mt_void)
{
    AIAO_CAST_ID_E enCast;

    /* init rm */
    for (enCast = AIAO_CAST_0; enCast < AIAO_CAST_BUTT; enCast++)
    {
        g_CastRm.hCast[enCast] = MT_NULL;
    }

    return MT_SUCCESS;
}

mt_void                 HAL_CAST_DeInit(mt_void)
{
    AIAO_CAST_ID_E enCast;

    /* deinit rm */
    for (enCast = AIAO_CAST_0; enCast < AIAO_CAST_BUTT; enCast++)
    {
        if (g_CastRm.hCast[enCast])
        {
            HAL_CAST_Destroy(enCast);
        }

        g_CastRm.hCast[enCast] = MT_NULL;
    }
}


mt_s32  HAL_CAST_Create(AIAO_CAST_ID_E *penCAST, AIAO_CAST_ATTR_S *pstAttr)
{
    CAST_CHN_STATE_S *state = MT_NULL;
    AIAO_CAST_ID_E enCAST;
    mt_s32 Ret = MT_FAILURE;

    // todo , check attr
    if (MT_NULL == pstAttr)
    {
        MT_FATAL_AO("pstAttr is null\n");
        return MT_FAILURE;
    }
    if (!pstAttr->extDmaMem.u32BufPhyAddr || !pstAttr->extDmaMem.u32BufVirAddr)
    {
        MT_FATAL_AO("BufPhyAddr(0x%x) BufVirAddr(0x%x) invalid\n",pstAttr->extDmaMem.u32BufPhyAddr,pstAttr->extDmaMem.u32BufVirAddr);
        return MT_FAILURE;
    }
    if (pstAttr->extDmaMem.u32BufSize <AIAO_CAST_BUFSIZE_MIN)
    {
        MT_FATAL_AO("BufSize(0x%x) less than MinSize(0x%x) invalid\n",pstAttr->extDmaMem.u32BufSize,AIAO_CAST_BUFSIZE_MIN);
        return MT_FAILURE;
    }

    enCAST = GetFreeCAST();
    if (AIAO_CAST_BUTT == enCAST)
    {
        MT_FATAL_AO("CAST source is not enough\n");
        return MT_FAILURE;
    }

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(CAST_CHN_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc CAST_CHN_STATE_S failed\n");
        goto CAST_Create_ERR_EXIT;
    }

    memset(state, 0, sizeof(CAST_CHN_STATE_S));
    g_CastRm.hCast[enCAST] = (mt_handle)state;

    if (MT_SUCCESS != (Ret = HAL_CAST_SetAttr(enCAST, pstAttr)))
    {
        goto CAST_Create_ERR_EXIT;
    }

    state->enCurnStatus = AIAO_CAST_STATUS_STOP;
    state->enCast = enCAST;
    *penCAST = enCAST;
    return MT_SUCCESS;

CAST_Create_ERR_EXIT:
    *penCAST = AIAO_CAST_BUTT;
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_CastRm.hCast[enCAST] = MT_NULL;
    return Ret;
}

mt_void     HAL_CAST_Destroy(AIAO_CAST_ID_E enCAST)
{
    CAST_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_CastRm.hCast[enCAST])
    {
        return;
    }

    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP != state->enCurnStatus)
    {
        HAL_CAST_Stop(enCAST);
    }

    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_CastRm.hCast[enCAST] = MT_NULL;
    return;
}

mt_s32  HAL_CAST_SetAttr(AIAO_CAST_ID_E enCAST, AIAO_CAST_ATTR_S *pstAttr)
{
    CAST_CHN_STATE_S *state = MT_NULL;

    CHECK_CAST_OPEN(enCAST);

    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];
    if (AIAO_CAST_STATUS_STOP != state->enCurnStatus)
    {
        return MT_FAILURE;
    }

    if (pstAttr->extDmaMem.u32BufSize < AIAO_CAST_BUFSIZE_MIN)
    {
        return MT_FAILURE;
    }

#if 0
    CIRC_BUF_Init(&state->stCB,
                  (mt_u32 *)(pstAttr->extDmaMem.u32WptrAddr),
                  (mt_u32 *)(pstAttr->extDmaMem.u32RptrAddr),
                  (mt_u32 *)pstAttr->extDmaMem.u32BufVirAddr,
                  pstAttr->extDmaMem.u32BufSize);
#endif

    state->u32BufFrameSize = UTIL_CalcFrameSize(pstAttr->u32BufChannels,
                                                pstAttr->u32BufBitPerSample);

    memset(&state->stProc, 0, sizeof(CAST_PROC_STATUS_S));
    memcpy(&state->stUserAttr, pstAttr, sizeof(AIAO_CAST_ATTR_S));
    return MT_SUCCESS;
}

mt_s32  HAL_CAST_GetAttr(AIAO_CAST_ID_E enCAST, AIAO_CAST_ATTR_S *pstAttr)
{
    CAST_CHN_STATE_S *state = MT_NULL;

    CHECK_CAST_OPEN(enCAST);

    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    memcpy(pstAttr, &state->stUserAttr, sizeof(AIAO_CAST_ATTR_S));
    return MT_SUCCESS;
}

mt_s32  HAL_CAST_Start(AIAO_CAST_ID_E enCAST)
{
    CAST_CHN_STATE_S *state = MT_NULL;

    CHECK_CAST_OPEN(enCAST);
    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_START == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    state->enCurnStatus = AIAO_CAST_STATUS_START;
    return MT_SUCCESS;
}

mt_s32  HAL_CAST_Stop(AIAO_CAST_ID_E enCAST)
{
    CAST_CHN_STATE_S *state = MT_NULL;

    CHECK_CAST_OPEN(enCAST);
    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }
    
    CASTFlushState(state);
    state->enCurnStatus = AIAO_CAST_STATUS_STOP;
    return MT_SUCCESS;
}

mt_u32  HAL_CAST_QueryBufData(AIAO_CAST_ID_E enCAST)
{
    CAST_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_CastRm.hCast[enCAST])
    {
        MT_ERR_AO("CAST(%d) is not create.\n", enCAST); 
        return 0 ;
    }
    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP == state->enCurnStatus)
    {
        return 0;
    }

    return CIRC_BUF_QueryBusy(&state->stCB);
}

mt_u32                  HAL_CAST_QueryBufFree(AIAO_CAST_ID_E enCAST)
{
    CAST_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_CastRm.hCast[enCAST])
    {
        MT_ERR_AO("CAST(%d) is not create.\n", enCAST); 
        return 0 ;
    }
    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP == state->enCurnStatus)
    {
        return 0;
    }

    return CIRC_BUF_QueryFree(&state->stCB);
}

mt_u32                  HAL_CAST_ReadData(AIAO_CAST_ID_E enCAST, mt_u32 * pu32DataOffset, mt_u32 u32DestSize)
{
    CAST_CHN_STATE_S *state = MT_NULL;
    mt_u32 Bytes;

    if (MT_NULL == g_CastRm.hCast[enCAST])
    {
        MT_ERR_AO("CAST(%d) is not create.\n", enCAST); 
        return MT_FAILURE;
    }
    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP == state->enCurnStatus)
    {
        return 0;
    }

    state->stProc.uTryReadCnt++;
    Bytes = CIRC_BUF_CastRead(&state->stCB, pu32DataOffset, u32DestSize); //just read not update readptr
    state->stProc.uTotalByteRead += Bytes;
    return Bytes;
}
mt_u32                  HAL_CAST_ReleaseData(AIAO_CAST_ID_E enCAST, mt_u32 u32DestSize)
{
    CAST_CHN_STATE_S *state = MT_NULL;
    mt_u32 Bytes;

    if (MT_NULL == g_CastRm.hCast[enCAST])
    {
        MT_ERR_AO("CAST(%d) is not create.\n", enCAST); 
        return 0 ;
    }
    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP == state->enCurnStatus)
    {
        return 0;
    }

    state->stProc.uTryReadCnt++;
    Bytes = CIRC_BUF_CastRelese(&state->stCB, u32DestSize); //just update readptr not read
    state->stProc.uTotalByteRead += Bytes;
    return Bytes;
}

mt_u32                  HAL_CAST_WriteBufData(AIAO_CAST_ID_E enCAST, mt_u8 * pu32Src, mt_u32 u32SrcBytes, mt_u32 ChanExist)
{
    CAST_CHN_STATE_S *state = MT_NULL;
    mt_u32 Bytes;

    if (MT_NULL == g_CastRm.hCast[enCAST])
    {
        MT_ERR_AO("CAST(%d) is not create.\n", enCAST); 
        return 0 ;
    }
    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP == state->enCurnStatus)
    {
        return 0;
    }

    state->stProc.uTryWriteCnt++;
    Bytes = CIRC_BUF_Write(&state->stCB, pu32Src, u32SrcBytes,  ChanExist);
    state->stProc.uTotalByteWrite += Bytes;
    return Bytes;
}

mt_void                 HAL_CAST_GetBufDelayMs(AIAO_CAST_ID_E enCAST, mt_u32 *pDelayms)
{
    CAST_CHN_STATE_S *state = MT_NULL;
    mt_u32 FreeBytes = 0;

    if (MT_NULL == g_CastRm.hCast[enCAST])
    {
        *pDelayms = 0;
        return;
    }

    state = (CAST_CHN_STATE_S*)g_CastRm.hCast[enCAST];

    if (AIAO_CAST_STATUS_STOP == state->enCurnStatus)
    {
        *pDelayms = 0;
        return;
    }

    FreeBytes = CIRC_BUF_QueryBusy(&state->stCB);
    *pDelayms = CALC_LATENCY_MS(state->stUserAttr.u32BufSampleRate, state->u32BufFrameSize, FreeBytes);
    return;
}

mt_void HAL_CAST_GetDefAttr(AIAO_CAST_ATTR_S *pstAttr)
{
    pstAttr->u32BufChannels = 2;
    pstAttr->u32BufBitPerSample = 16;
    pstAttr->u32BufSampleRate = 48000;
    pstAttr->u32BufDataFormat = 0;
    pstAttr->u32BufLatencyThdMs = AOE_CAST_BUFF_LATENCYMS_MAX-8;
    return;
}

#if 0
mt_s32 HAL_AIAO_P_GetStatus(AIAO_CAST_ID_E enCAST, AIAO_PORT_STAUTS_S *pstProcInfo)
{
    mt_s32 Ret = MT_FAILURE;
    mt_u32 Id = PORT2ID(enPortID);

    if (g_AIAORm.hPort[Id])
    {
        Ret = iHAL_AIAO_P_GetStatus(g_AIAORm.hPort[Id], pstProcInfo);
    }

    return Ret;
}
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
