
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_buffer.c
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#include <linux/sizes.h>	/*SZ_1K*/

#include "drv_disp_buffer.h"
#include "mt_osal.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

mt_s32 BP_Create(mt_u32 u32BufNum, BUF_ALLOC_S *pstAlloc, BUF_POOL_S *pstBP)
{
    mt_u32 u, BufStride;
    mt_s32 nRet;

    if ((!pstBP) || (!pstAlloc))
    {
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((u32BufNum < WIN_BUFFER_MIN_NUMBER) || (u32BufNum > WIN_BUFFER_MAX_NUMBER))
    {
        return MT_ERR_DISP_INVALID_PARA;
    }

    DISP_MEMSET(pstBP, 0, sizeof(BUF_POOL_S));

    pstBP->enMemType = BUF_MEM_SRC_SUPPLY;

    if ((pstAlloc->eDataFormat != MT_DRV_PIX_FMT_NV21) && (pstAlloc->eDataFormat != MT_DRV_PIX_FMT_NV12) &&
        (pstAlloc->eDataFormat != MT_DRV_PIX_FMT_NV16_2X1) && (pstAlloc->eDataFormat != MT_DRV_PIX_FMT_NV61_2X1) &&
        (pstAlloc->eDataFormat != MT_DRV_PIX_FMT_YUYV) && (pstAlloc->eDataFormat != MT_DRV_PIX_FMT_YVYU) &&
        (pstAlloc->eDataFormat != MT_DRV_PIX_FMT_UYVY))
    {
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstAlloc->u32BufWidth < WIN_BUFFER_MIN_W) || (pstAlloc->u32BufWidth > WIN_BUFFER_MAX_W) ||
        (pstAlloc->u32BufHeight < WIN_BUFFER_MIN_H) || (pstAlloc->u32BufHeight > WIN_BUFFER_MAX_H))
    {
        return MT_ERR_DISP_INVALID_PARA;
    }

    pstBP->stAlloc = *pstAlloc;

    MT_INFO_VO("cfg : bufsize 0x%x,  bufstride 0x%x \n", pstBP->u32BufSize, pstBP->u32BufStride);

    if (!pstAlloc->bFbAllocMem)
    {
        if ((pstAlloc->u32BufStride < pstAlloc->u32BufWidth) || ((pstAlloc->u32BufStride & 0xful) != 0))
        {
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
    else
    {
        if ((pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV21) || (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV12) ||
            (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV16_2X1) || (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV61_2X1))
        {
            BufStride = (pstAlloc->u32BufWidth + 127) / 128 * 128;
	    MT_INFO_VO("----------------\n");
        }
        else if ((pstAlloc->eDataFormat == MT_DRV_PIX_FMT_YUYV) || (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_YVYU) ||
                 (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_UYVY))
        {
            BufStride = ((pstAlloc->u32BufWidth * 2) + 15) & 0xFFFFFFF0ul;
	    MT_INFO_VO("----------------\n");

        }
        else
        {
            BufStride = (pstAlloc->u32BufWidth + 15) & 0xFFFFFFF0ul;
	    MT_INFO_VO("----------------\n");
        }

        pstBP->stAlloc.u32BufStride = BufStride;
    }

    if (pstAlloc->bFbAllocMem)
        pstBP->enMemType = BUF_MEM_FB_SUPPLY;
    else
        pstBP->enMemType = BUF_MEM_USER_SUPPLY;

    pstBP->pstBufQueue = (BUF_S *)DISP_MALLOC(sizeof(BUF_S) * u32BufNum);
    pstBP->pstEmptyQueue = (BUF_ID_S *)DISP_MALLOC(sizeof(BUF_ID_S) * u32BufNum);
    pstBP->pstFullQueue = (BUF_ID_S *)DISP_MALLOC(sizeof(BUF_ID_S) * u32BufNum);
    pstBP->pstCfgWritingQueue = (BUF_ID_S *)DISP_MALLOC(sizeof(BUF_ID_S) * u32BufNum);

    if (!pstBP->pstBufQueue || !pstBP->pstEmptyQueue || !pstBP->pstFullQueue || !pstBP->pstCfgWritingQueue)
    {
        goto __ERR_EXIT__;
    }

    if (pstBP->enMemType == BUF_MEM_FB_SUPPLY)
    {
        mt_u32 BufSize = 0;
        mt_char BufName[10] = { 'V', 'D', 'P', '_', 'C', 'a', 's', 't', '0', '\0' };

        if ((pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV21) || (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV12))
        {
            BufSize = (pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 3 >> 1);
            //BufSize = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 2;
	    MT_INFO_VO("----------------\n");
        }

        if ((pstAlloc->eDataFormat == MT_DRV_PIX_FMT_YUYV) || (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_YVYU) || (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_UYVY))
        {
            BufSize = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 2;
	    MT_INFO_VO("----------------\n");
        }

        if ((pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV16_2X1) || (pstAlloc->eDataFormat == MT_DRV_PIX_FMT_NV61_2X1))
        {
            BufSize = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 2;
	    MT_INFO_VO("----------------\n");
        }
        BufSize += 0x1000;
        for (u = 0; u < u32BufNum; u++)
        {
            BufName[8] = (mt_char)('0' + u);

            nRet = DISP_OS_MMZ_Alloc((const char *)BufName, MT_NULL, BufSize, 16, &pstBP->pstBufQueue[u].stMem);

            MT_INFO_VO("BP_create : bufsize 0x%x, addr 0x%x \n",BufSize,pstBP->pstBufQueue[u].stMem.u32StartPhyAddr);

            if (nRet)
            {
                break;
            }
        }

        if (u < u32BufNum)
        {
            mt_u32 k;

            for (k = 0; k < u; k++)
            {
                DISP_OS_MMZ_Release(&pstBP->pstBufQueue[k].stMem);
            }
            goto __ERR_EXIT__;
        }

        MT_INFO_VO("BP buf malloc : size %d\n", BufSize);
    }

    pstBP->u32BufNum = u32BufNum;

    /*this is just for storing the buffer size passed by user, not allocated buffer.*/
    pstBP->u32BufSize = pstBP->stAlloc.u32BufSize;//pstAlloc->u32BufSize;
    pstBP->u32BufStride = pstBP->stAlloc.u32BufStride;//pstAlloc->u32BufStride;

    MT_INFO_VO("final : bufsize 0x%x,  bufstride 0x%x \n", pstBP->u32BufSize, pstBP->u32BufStride);

    BP_Reset(pstBP);

    return MT_SUCCESS;

__ERR_EXIT__:

    if (pstBP->pstFullQueue)
    {
        DISP_FREE(pstBP->pstFullQueue);
    }

    if (pstBP->pstEmptyQueue)
    {
        DISP_FREE(pstBP->pstEmptyQueue);
    }

    if (pstBP->pstBufQueue)
    {
        DISP_FREE(pstBP->pstBufQueue);
    }

    if (pstBP->pstCfgWritingQueue)
    {
        DISP_FREE(pstBP->pstCfgWritingQueue);
    }

    return MT_SUCCESS;
}

mt_s32 BP_Destroy(BUF_POOL_S *pstBP)
{
    mt_u32 u;

	if (pstBP == NULL)
	{
		return MT_FAILURE;
	}

    // if buffer in local memory, release memory
    if (pstBP->enMemType == BUF_MEM_FB_SUPPLY)
    {
		if (pstBP->pstBufQueue != NULL)
		{
	        for (u = 0; u < pstBP->u32BufNum; u++)
	        {
	            DISP_OS_MMZ_Release(&pstBP->pstBufQueue[u].stMem);
	        }
        }
    }

    if (pstBP->pstFullQueue)
    {
        DISP_FREE(pstBP->pstFullQueue);
        pstBP->pstFullQueue = MT_NULL;
    }

    if (pstBP->pstEmptyQueue)
    {
        DISP_FREE(pstBP->pstEmptyQueue);
        pstBP->pstEmptyQueue = MT_NULL;
    }

    if (pstBP->pstBufQueue)
    {
        DISP_FREE(pstBP->pstBufQueue);
        pstBP->pstBufQueue = MT_NULL;
    }

    if (pstBP->pstCfgWritingQueue)
    {
        DISP_FREE(pstBP->pstCfgWritingQueue);
        pstBP->pstCfgWritingQueue = MT_NULL;
    }

    return MT_SUCCESS;
}

mt_s32 BP_Reset(BUF_POOL_S *pstBP)
{
    mt_u32 u;
    mt_u32 Coffset = 0;

    // s1 reset buf queue
    if ((pstBP->enMemType == BUF_MEM_FB_SUPPLY) || (pstBP->enMemType == BUF_MEM_USER_SUPPLY))
    {
        Coffset = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight;
    }

    for (u = 0; u < pstBP->u32BufNum; u++)
    {
        pstBP->pstBufQueue[u].u32Index = u;
        pstBP->pstBufQueue[u].enState = BUF_STATE_EMPTY;

        if ((pstBP->enMemType == BUF_MEM_FB_SUPPLY) || (pstBP->enMemType == BUF_MEM_USER_SUPPLY))
        {
            pstBP->pstBufQueue[u].stFrame.stBufAddr[0].u32Stride_Y = pstBP->stAlloc.u32BufStride;
            pstBP->pstBufQueue[u].stFrame.stBufAddr[0].u32Stride_C = pstBP->stAlloc.u32BufStride;
            pstBP->pstBufQueue[u].stFrame.u32TunnelPhyAddr = pstBP->pstBufQueue[u].stMem.u32StartPhyAddr;

            pstBP->pstBufQueue[u].stFrame.stBufAddr[0].u32PhyAddr_Y = pstBP->pstBufQueue[u].stMem.u32StartPhyAddr + 0x1000;
            pstBP->pstBufQueue[u].stFrame.stBufAddr[0].u32PhyAddr_C = pstBP->pstBufQueue[u].stMem.u32StartPhyAddr + 0x1000 + Coffset;
            pstBP->pstBufQueue[u].stFrame.u32Width = pstBP->stAlloc.u32BufWidth;
            pstBP->pstBufQueue[u].stFrame.u32Height = pstBP->stAlloc.u32BufHeight;
            pstBP->pstBufQueue[u].stFrame.enFieldMode = MT_DRV_FIELD_ALL;

#if 1
            MT_INFO_VO("F: y=0x%x, c=0x%x\n",
            pstBP->pstBufQueue[u].stFrame.stBufAddr[0].u32PhyAddr_Y,
            pstBP->pstBufQueue[u].stFrame.stBufAddr[0].u32PhyAddr_C);
#endif
        }
    }

    // s2 reset empty queue
    DISP_MEMSET(pstBP->pstEmptyQueue, 0, sizeof(BUF_ID_S) * pstBP->u32BufNum);

    for (u = 0; u < pstBP->u32BufNum; u++)
    {
        pstBP->pstEmptyQueue[u].u32Index = WIN_BUFFER_INDEX_PREFIX | (u << WIN_BUFFER_INDEX_SHIFT);
    }

    pstBP->u32EmptyRPtr = 0;
    pstBP->u32EmptyWPtr = 0;

    // s3 reset full queue
    DISP_MEMSET(pstBP->pstFullQueue, 0, sizeof(BUF_ID_S) * pstBP->u32BufNum);
    for (u = 0; u < pstBP->u32BufNum; u++)
    {
        pstBP->pstFullQueue[u].u32Index = WIN_BUFFER_INDEX_INVALID;
    }

    pstBP->u32FullRPtr = 0;
    pstBP->u32FullWPtr = 0;

    DISP_MEMSET(pstBP->pstCfgWritingQueue, 0, sizeof(BUF_ID_S) * pstBP->u32BufNum);
    for (u = 0; u < pstBP->u32BufNum; u++)
    {
        pstBP->pstCfgWritingQueue[u].u32Index = WIN_BUFFER_INDEX_INVALID;
    }

    pstBP->u32CfgWritingRPtr = 0;
    pstBP->u32CfgWritingWPtr = 0;
    // s4 clean statistic
    DISP_MEMSET(&pstBP->stStatistic, 0, sizeof(BUF_STAT_S));

    return MT_SUCCESS;
}

// produce and consume empty buffer
// put in node
mt_s32 BP_GetEmptyBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId)
{
    mt_u32 id, index;

    id = pstBP->pstEmptyQueue[pstBP->u32EmptyRPtr].u32Index;
    if (WIN_BUFFER_INDEX_INVALID == id)
    {
        DISP_INFO("GE queue empty!\n");
        return MT_FAILURE;
    }

    index = (id >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;
    if (BUF_STATE_EMPTY == pstBP->pstBufQueue[index].enState)
    {
        *pu32BufId = id;
        return MT_SUCCESS;
    }
    else
    {
        DISP_ERROR("GE invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
}

mt_s32 BP_GetDoneBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId)
{
    mt_u32 id, index;

    id = pstBP->pstEmptyQueue[pstBP->u32EmptyRPtr].u32Index;
    if (WIN_BUFFER_INDEX_INVALID == id)
    {
        DISP_INFO("GD queue empty!\n");
        return MT_FAILURE;
    }

    index = (id >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;
    if (BUF_STATE_DONE == pstBP->pstBufQueue[index].enState)
    {
        *pu32BufId = id;
        return MT_SUCCESS;
    }
    else
    {
        DISP_ERROR("GD invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
}

mt_s32 BP_DelEmptyBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("DE invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (BUF_STATE_EMPTY == pstBP->pstBufQueue[index].enState)
    {
        pstBP->pstBufQueue[index].enState = BUF_STATE_WRITING;
        pstBP->pstEmptyQueue[pstBP->u32EmptyRPtr].u32Index = WIN_BUFFER_INDEX_INVALID;

        pstBP->u32EmptyRPtr = (pstBP->u32EmptyRPtr + 1) % pstBP->u32BufNum;

        pstBP->stStatistic.u32EmptyDoing = index;
        pstBP->stStatistic.u32EmptyDel++;
        return MT_SUCCESS;
    }
    else
    {
        DISP_ERROR("DE invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
}

mt_s32 BP_SetBufReading(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("DE invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    pstBP->pstBufQueue[index].enState = BUF_STATE_READING;
    return MT_SUCCESS;
}

mt_s32 BP_AddEmptyBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("AE invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (pstBP->pstEmptyQueue[pstBP->u32EmptyWPtr].u32Index != WIN_BUFFER_INDEX_INVALID)
    {
        DISP_ERROR("AE queue full!\n");
        return MT_FAILURE;
    }
    else if (pstBP->pstBufQueue[index].enState != BUF_STATE_READING)
    {
        DISP_ERROR("AE invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
    {
        pstBP->pstBufQueue[index].enState = BUF_STATE_DONE;
        pstBP->pstEmptyQueue[pstBP->u32EmptyWPtr].u32Index = u32BufId;
        pstBP->u32EmptyWPtr = (pstBP->u32EmptyWPtr + 1) % pstBP->u32BufNum;

        pstBP->stStatistic.u32EmptyAdd++;
        return MT_SUCCESS;
    }
}

mt_s32 BP_SetBufEmpty(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("SE invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (pstBP->pstBufQueue[index].enState != BUF_STATE_DONE)
    {
        DISP_ERROR("SE invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
    {
        pstBP->pstBufQueue[index].enState = BUF_STATE_EMPTY;

        return MT_SUCCESS;
    }
}

// produce and consume full buffer
mt_s32 BP_GetFullBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId)
{
    mt_u32 id, index;

    id = pstBP->pstFullQueue[pstBP->u32FullRPtr].u32Index;
    if (WIN_BUFFER_INDEX_INVALID == id)
    {
        DISP_INFO("GF queue empty!\n");
        return MT_FAILURE;
    }

    index = (id >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (BUF_STATE_FULL == pstBP->pstBufQueue[index].enState)
    {
        *pu32BufId = id;
        return MT_SUCCESS;
    }
    else
    {
        DISP_ERROR("GF invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
}

mt_s32 BP_DelFullBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("DF invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (BUF_STATE_FULL == pstBP->pstBufQueue[index].enState)
    {
        pstBP->pstBufQueue[index].enState = BUF_STATE_READING;
        pstBP->pstFullQueue[pstBP->u32FullRPtr].u32Index = WIN_BUFFER_INDEX_INVALID;
        pstBP->u32FullRPtr = (pstBP->u32FullRPtr + 1) % pstBP->u32BufNum;

        pstBP->stStatistic.u32FullDoing = index;
        pstBP->stStatistic.u32FullDel++;
        return MT_SUCCESS;
    }
    else
    {
        DISP_ERROR("DF invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
}

mt_s32 BP_AddFullBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("AF invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (pstBP->pstFullQueue[pstBP->u32FullWPtr].u32Index != WIN_BUFFER_INDEX_INVALID)
    {
        DISP_ERROR("AF queue full!\n");
        return MT_FAILURE;
    }
    else if (pstBP->pstBufQueue[index].enState != BUF_STATE_WRITING)
    {
        DISP_ERROR("AF invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
    {
        pstBP->pstBufQueue[index].enState = BUF_STATE_FULL;
        pstBP->pstFullQueue[pstBP->u32FullWPtr].u32Index = u32BufId;
        pstBP->u32FullWPtr = (pstBP->u32FullWPtr + 1) % pstBP->u32BufNum;

        pstBP->stStatistic.u32FullAdd++;
        return MT_SUCCESS;
    }
}

// get and set frame to buffer
mt_s32 BP_GetFrame(BUF_POOL_S *pstBP, mt_u32 u32BufId, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("GetF invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

#if 0
    if (pstBP->pstBufQueue[index].enState != BUF_STATE_WORKING)
    {
        DISP_ERROR("GetF invalid state buf[%d]=%d\n", index,(mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
#endif
    {
        *pstFrame = pstBP->pstBufQueue[index].stFrame;

        return MT_SUCCESS;
    }
}

mt_s32 BP_ReAllocBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index = 0, Coffset = 0;
    mt_s32 nRet = MT_SUCCESS, BufSize = 0;
    mt_u8 BufName[40] = { '\0' };

    MT_INFO_VO("BP_ReAllocBuf\n");

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("GetF invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV21) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV12) ||
        (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV16_2X1) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV61_2X1))
    {
        //pstBP->stAlloc.u32BufStride = (pstBP->stAlloc.u32BufWidth + 15) & 0xFFFFFFF0ul;
        pstBP->stAlloc.u32BufStride = (pstBP->stAlloc.u32BufWidth + 127) /128 *128;
    }
    else if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YUYV) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YVYU) ||
             (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_UYVY))
    {
        pstBP->stAlloc.u32BufStride = ((pstBP->stAlloc.u32BufWidth * 2) + 15) & 0xFFFFFFF0ul;
    }
    else
    {
        DISP_ERROR("invalid pixel format: %d\n", pstBP->stAlloc.eDataFormat);
        return MT_ERR_DISP_NOT_SUPPORT_FMT;
    }

    if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV21) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV12))
    {
        BufSize = (pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 3 >> 1);
    }

    if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YUYV) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YVYU) ||
        (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_UYVY))
    {
        BufSize = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight;
    }

    if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV16_2X1) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV61_2X1))
    {
        BufSize = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 2;
    }

    BufSize += DISP_CAST_LOWDELAY_MEM_LENGTH;
    DISP_OS_MMZ_Release(&pstBP->pstBufQueue[index].stMem);

    mt_osal_snprintf(BufName, 40, "VDP_Cast%d", index);

    nRet = DISP_OS_MMZ_Alloc((const char *)BufName, MT_NULL, BufSize, 16, &pstBP->pstBufQueue[index].stMem);
    if (nRet)
        return MT_ERR_DISP_CREATE_ERR;

    if ((pstBP->enMemType == BUF_MEM_FB_SUPPLY) || (pstBP->enMemType == BUF_MEM_USER_SUPPLY))
    {
        Coffset = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight;
    }

    pstBP->pstBufQueue[index].u32Index = index;
    pstBP->pstBufQueue[index].enState = BUF_STATE_READING;

    if ((pstBP->enMemType == BUF_MEM_FB_SUPPLY) || (pstBP->enMemType == BUF_MEM_USER_SUPPLY))
    {
        pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32Stride_Y = pstBP->stAlloc.u32BufStride;
        pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32Stride_C = pstBP->stAlloc.u32BufStride;
        pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32PhyAddr_Y = pstBP->pstBufQueue[index].stMem.u32StartPhyAddr + 0x1000;
        pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32PhyAddr_C = pstBP->pstBufQueue[index].stMem.u32StartPhyAddr + 0x1000 + Coffset;
        pstBP->pstBufQueue[index].stFrame.u32TunnelPhyAddr = pstBP->pstBufQueue[index].stMem.u32StartPhyAddr;

        pstBP->pstBufQueue[index].stFrame.u32Width = pstBP->stAlloc.u32BufWidth;
        pstBP->pstBufQueue[index].stFrame.u32Height = pstBP->stAlloc.u32BufHeight;
    }

    return MT_SUCCESS;
}

mt_s32 BP_ReAllocAllBuf(BUF_POOL_S *pstBP)
{
    mt_u32 index = 0, Coffset = 0;
    mt_s32 nRet = MT_SUCCESS, BufSize = 0;
    mt_u8 BufName[40] = { '\0' };

    MT_INFO_VO("BP_ReAllcoAllBuf\n");
    for (index = 0; index < pstBP->u32BufNum; index++)
    {
        if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV21) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV12) ||
            (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV16_2X1) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV61_2X1))
        {
            //pstBP->stAlloc.u32BufStride = (pstBP->stAlloc.u32BufWidth + 15) & 0xFFFFFFF0ul;
            pstBP->stAlloc.u32BufStride = (pstBP->stAlloc.u32BufWidth + 127)/128 *128;
        }
        else if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YUYV) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YVYU) ||
                 (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_UYVY))
        {
            pstBP->stAlloc.u32BufStride = ((pstBP->stAlloc.u32BufWidth * 2) + 15) & 0xFFFFFFF0ul;
        }
        else
        {
            DISP_ERROR("invalid pixel format: %d\n", pstBP->stAlloc.eDataFormat);
            return MT_ERR_DISP_NOT_SUPPORT_FMT;
        }

        if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV21) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV12))
        {
            BufSize = (pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 3 >> 1);
        }

        if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YUYV) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_YVYU) ||
            (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_UYVY))
        {
            BufSize = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight;
        }

        if ((pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV16_2X1) || (pstBP->stAlloc.eDataFormat == MT_DRV_PIX_FMT_NV61_2X1))
        {
            BufSize = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight * 2;
        }

        BufSize += DISP_CAST_LOWDELAY_MEM_LENGTH;
        DISP_OS_MMZ_Release(&pstBP->pstBufQueue[index].stMem);

        mt_osal_snprintf(BufName, 40, "VDP_Cast%d", index);

        nRet = DISP_OS_MMZ_Alloc((const char *)BufName, MT_NULL, BufSize, 16, &pstBP->pstBufQueue[index].stMem);
        if (nRet)
            return MT_ERR_DISP_CREATE_ERR;

        if ((pstBP->enMemType == BUF_MEM_FB_SUPPLY) || (pstBP->enMemType == BUF_MEM_USER_SUPPLY))
        {
            Coffset = pstBP->stAlloc.u32BufStride * pstBP->stAlloc.u32BufHeight;
        }

        pstBP->pstBufQueue[index].u32Index = index;
        pstBP->pstBufQueue[index].enState = BUF_STATE_EMPTY;

        if ((pstBP->enMemType == BUF_MEM_FB_SUPPLY) || (pstBP->enMemType == BUF_MEM_USER_SUPPLY))
        {
            pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32Stride_Y = pstBP->stAlloc.u32BufStride;
            pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32Stride_C = pstBP->stAlloc.u32BufStride;
            pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32PhyAddr_Y = pstBP->pstBufQueue[index].stMem.u32StartPhyAddr + 0x1000;
            pstBP->pstBufQueue[index].stFrame.stBufAddr[0].u32PhyAddr_C = pstBP->pstBufQueue[index].stMem.u32StartPhyAddr + 0x1000 + Coffset;
            pstBP->pstBufQueue[index].stFrame.u32TunnelPhyAddr = pstBP->pstBufQueue[index].stMem.u32StartPhyAddr;

            pstBP->pstBufQueue[index].stFrame.u32Width = pstBP->stAlloc.u32BufWidth;
            pstBP->pstBufQueue[index].stFrame.u32Height = pstBP->stAlloc.u32BufHeight;
        }
    }

    return MT_SUCCESS;
}

mt_s32 BP_SetFrame(BUF_POOL_S *pstBP, mt_u32 u32BufId, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("SetF invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (pstBP->pstBufQueue[index].enState != BUF_STATE_WRITING)
    {
        DISP_ERROR("SetF invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
    {
        pstBP->pstBufQueue[index].stFrame = *pstFrame;
        return MT_SUCCESS;
    }
}

// get and set user data to buffer
mt_s32 BP_GetUserData(BUF_POOL_S *pstBP, mt_u32 u32BufId, BUF_USERDATA_S *pstData)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("GetU invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

#if 0
    if (pstBP->pstBufQueue[index].enState != BUF_STATE_WORKING)
    {
        DISP_ERROR("GetU invalid state buf[%d]=%d\n", index,(mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
#endif
    {
        memcpy(pstData, &pstBP->pstBufQueue[index].stUserData, sizeof(BUF_USERDATA_S));
        return MT_SUCCESS;
    }
}

mt_s32 BP_SetUserData(BUF_POOL_S *pstBP, mt_u32 u32BufId, BUF_USERDATA_S *pstData)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("SetU invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (pstBP->pstBufQueue[index].enState != BUF_STATE_WRITING)
    {
        DISP_ERROR("SetU invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
    {
        memcpy(&pstBP->pstBufQueue[index].stUserData, pstData, sizeof(BUF_USERDATA_S));
        return MT_SUCCESS;
    }
}

mt_s32 BP_GetFullBufNum(BUF_POOL_S *pstBP, mt_u32 *pu32BufNum)
{
    mt_u32 num;
    mt_u32 r, w;

    r = pstBP->u32FullRPtr;
    w = pstBP->u32FullWPtr;
    if (w >= r)
    {
        num = w - r;
    }
    else
    {
        num = w + pstBP->u32BufNum - r;
    }

    if (num <= pstBP->u32BufNum)
    {
        *pu32BufNum = num;
    }
    else
    {
        DISP_ERROR("BP_GetFullBufNum invalid!\n");
        *pu32BufNum = 0;
    }

    return MT_SUCCESS;
}

mt_s32 BP_GetEmptyBufNum(BUF_POOL_S *pstBP, mt_u32 *pu32BufNum)
{
    mt_u32 num;

    if (pstBP->u32EmptyWPtr >= pstBP->u32EmptyRPtr)
    {
        num = pstBP->u32EmptyWPtr - pstBP->u32EmptyRPtr;
    }
    else
    {
        num = pstBP->u32EmptyWPtr + pstBP->u32BufNum - pstBP->u32EmptyRPtr;
    }

    if (num <= pstBP->u32BufNum)
    {
        *pu32BufNum = num;
    }
    else
    {
        DISP_ERROR("BP_GetEmptyBufNum invalid!\n");
        *pu32BufNum = 0;
    }

    return MT_SUCCESS;
}

mt_s32 BP_GetBufState(BUF_POOL_S *pstBP, BUF_STT_S *pstBufState)
{
    mt_u32 BufId, u;

    if (!pstBP || !pstBufState)
    {
        DISP_ERROR("%s input null pointer!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    DISP_MEMSET(pstBufState, 0, sizeof(BUF_STT_S));

    pstBufState->u32BufNum = pstBP->u32BufNum;
    pstBufState->enMemType = (mt_u32)pstBP->enMemType;
    pstBufState->enPixFmt = (mt_u32)pstBP->stAlloc.eDataFormat;

    pstBufState->u32EmptyRPtr = pstBP->u32EmptyRPtr;
    pstBufState->u32EmptyWPtr = pstBP->u32EmptyWPtr;
    pstBufState->u32FullRPtr = pstBP->u32FullRPtr;
    pstBufState->u32FullWPtr = pstBP->u32FullWPtr;
    pstBufState->u32EmptyDel = pstBP->stStatistic.u32EmptyDel;
    pstBufState->u32EmptyAdd = pstBP->stStatistic.u32EmptyAdd;
    pstBufState->u32EmptyDoing = pstBP->stStatistic.u32EmptyDoing;
    pstBufState->u32FullDel = pstBP->stStatistic.u32FullDel;
    pstBufState->u32FullAdd = pstBP->stStatistic.u32FullAdd;
    pstBufState->u32FullDoing = pstBP->stStatistic.u32FullDoing;

    for (u = 0; u < pstBP->u32BufNum; u++)
    {
        pstBufState->stBufState[u] = (mt_u32)pstBP->pstBufQueue[u].enState;
        pstBufState->stEmptyQueue[u] = pstBP->pstEmptyQueue[u].u32Index;
        pstBufState->stFullQueue[u] = pstBP->pstFullQueue[u].u32Index;
    }

    if (!BP_GetFullBuf(pstBP, &BufId))
    {
        (mt_void) BP_GetFrame(pstBP, BufId, &pstBufState->stFrame);
    }

    return MT_SUCCESS;
}

static mt_u32 s_u32SDwritebackmemCount = 0;
static DISP_MMZ_BUF_S s_SDwritebackMem;
//Symphony 444 -> 422
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
#define ARIA_SD_WR_BACK_SIZE (720 * 576 * 2 * 2)
#else
#define ARIA_SD_WR_BACK_SIZE (720 * 576 * 4 * 2)
#endif
mt_s32 BP_CreateSDWriteBackMem(bool sd_wrback_422, mt_u8 field_num)
{
    mt_u32 BufSize = ARIA_SD_WR_BACK_SIZE;
    mt_s32 nRet;

    if (s_u32SDwritebackmemCount)
    {
        DISP_DEBUGK("SDWriteBackmem has been created! %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    if(sd_wrback_422)
    {
        BufSize = 720 * 576 * 2 * field_num / 2;   // 2 bytes per pixel
    }
    else
    {
        BufSize = 720 * 576 * 4 * field_num / 2;  // 4bytes per pixel
    }
		
    DISP_MEMSET(&s_SDwritebackMem, 0, sizeof(DISP_MMZ_BUF_S));
    nRet = DISP_OS_MMZ_AllocAndMap((const char *)"DISP_SDWBMem", MMZ_ZONE_AV, BufSize, 16, &s_SDwritebackMem);
    DISP_DEBUGK("[%s]alloc SDWriteBackmem szie[%d],origsize[%d]\n", __FUNCTION__, s_SDwritebackMem.u32Size, BufSize);
    if (nRet)
    {
        DISP_DEBUGK("alloc SDWriteBackmem failed %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    s_u32SDwritebackmemCount++;
    return MT_SUCCESS;
}

mt_s32 BP_DestroySDWriteBackMem(mt_void)
{
    if (!s_u32SDwritebackmemCount)
    {
        DISP_ERROR("SDWriteBackmem has not been created!\n");
        return MT_FAILURE;
    }

    DISP_OS_MMZ_UnmapAndRelease(&s_SDwritebackMem);

    DISP_MEMSET(&s_SDwritebackMem, 0, sizeof(DISP_MMZ_BUF_S));

    s_u32SDwritebackmemCount = 0;
    return MT_SUCCESS;
}

DISP_MMZ_BUF_S *BP_GetSDWriteBackMemInfo(mt_void)
{
    if (s_u32SDwritebackmemCount)
    {
        return &s_SDwritebackMem;
    }

    DISP_ERROR("SDWBMem has not been created OR NULL pointer!\n");
    return MT_NULL;
}

static mt_u32 s_u32DebugTestMemCount = 0;
static DISP_MMZ_BUF_S s_DebugTestMem;
#define ARIA_DebugTestMem_SIZE (0x1000)
volatile mt_u32 *p_intcnt;
mt_s32 BP_CreateDebugTestMem(mt_void)
{
    mt_u32 BufSize = ARIA_DebugTestMem_SIZE;
    mt_uchar *pdata;
    mt_s32 nRet;

    if (s_u32DebugTestMemCount)
    {
        DISP_DEBUGK("HDTestMem has been created! %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    DISP_MEMSET(&s_DebugTestMem, 0, sizeof(DISP_MMZ_BUF_S));
    nRet = DISP_OS_MMZ_AllocAndMap((const char *)"DISP_DebugTestMem", MT_NULL, BufSize, 1024, &s_DebugTestMem);
    DISP_DEBUGK("[%s]alloc HDTestMem szie[%d],origsize[%d],phy[%llx],virt[%lx]\n", __FUNCTION__, s_DebugTestMem.u32Size, BufSize, s_DebugTestMem.u32StartPhyAddr, s_DebugTestMem.u32StartVirAddr);
    if (nRet)
    {
        DISP_DEBUGK("alloc HDTestMem failed %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    pdata = (mt_uchar *)(s_DebugTestMem.u32StartVirAddr);
    DISP_MEMSET(pdata, 0x0, BufSize);
    p_intcnt = (mt_u32 *)(s_DebugTestMem.u32StartVirAddr);
    s_u32DebugTestMemCount++;
    return MT_SUCCESS;
}

mt_s32 BP_DestroyDebugTestMem(mt_void)
{
    if (!s_u32DebugTestMemCount)
    {
        DISP_ERROR("HDTestMem has not been created!\n");
        return MT_FAILURE;
    }

    DISP_OS_MMZ_UnmapAndRelease(&s_DebugTestMem);

    DISP_MEMSET(&s_DebugTestMem, 0, sizeof(DISP_MMZ_BUF_S));

    s_u32DebugTestMemCount = 0;
    return MT_SUCCESS;
}

DISP_MMZ_BUF_S *BP_GetDebugTestMemInfo(mt_void)
{
    if (s_u32DebugTestMemCount)
    {
        return &s_DebugTestMem;
    }

    DISP_ERROR("HDTestMem has not been created OR NULL pointer!\n");
    return MT_NULL;
}
static mt_u32 s_u32HDTestMemCount = 0;
static DISP_MMZ_BUF_S s_HDTestMem;
#define ARIA_HDTestMem_SIZE (0x100)
mt_s32 BP_CreateHDTestMem(mt_void)
{
    mt_u32 BufSize = ARIA_HDTestMem_SIZE;
    mt_uchar *pdata;
    mt_s32 nRet;

    if (s_u32HDTestMemCount)
    {
        DISP_DEBUGK("HDTestMem has been created! %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    DISP_MEMSET(&s_HDTestMem, 0, sizeof(DISP_MMZ_BUF_S));
    nRet = DISP_OS_MMZ_AllocAndMap((const char *)"DISP_HDTestMem", MT_NULL, BufSize, 4096, &s_HDTestMem);
    DISP_DEBUGK("[%s]alloc HDTestMem szie[%d],origsize[%d],phy[%llx],virt[%lx]\n", __FUNCTION__, s_HDTestMem.u32Size, BufSize, s_HDTestMem.u32StartPhyAddr, s_HDTestMem.u32StartVirAddr);
    if (nRet)
    {
        DISP_DEBUGK("alloc HDTestMem failed %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    pdata = (mt_uchar *)(s_HDTestMem.u32StartVirAddr);
    DISP_MEMSET(pdata, 0x0, BufSize);
    s_u32HDTestMemCount++;
    return MT_SUCCESS;
}

mt_s32 BP_DestroyHDTestMem(mt_void)
{
    if (!s_u32HDTestMemCount)
    {
        DISP_ERROR("HDTestMem has not been created!\n");
        return MT_FAILURE;
    }

    DISP_OS_MMZ_UnmapAndRelease(&s_HDTestMem);

    DISP_MEMSET(&s_HDTestMem, 0, sizeof(DISP_MMZ_BUF_S));

    s_u32HDTestMemCount = 0;
    return MT_SUCCESS;
}

DISP_MMZ_BUF_S *BP_GetHDTestMemInfo(mt_void)
{
    if (s_u32HDTestMemCount)
    {
        return &s_HDTestMem;
    }

    DISP_ERROR("HDTestMem has not been created OR NULL pointer!\n");
    return MT_NULL;
}

static mt_u32 s_u32BlackFrameCount = 0;
static BUF_S s_stBlackFrame;

#define BLACK_FRAME_WIDTH 64
#define BLACK_FRAME_HEIGHT 64
#define BLACK_FRAME_PIXFMT MT_DRV_PIX_FMT_NV21

mt_s32 BP_CreateBlackFrame(mt_void)
{
    mt_u32 BufSize = BLACK_FRAME_WIDTH * BLACK_FRAME_HEIGHT * 2;
    mt_uchar *pY, *pUV;
    MT_DRV_VIDEO_FRAME_S *pstFrame = &(s_stBlackFrame.stFrame);
    mt_s32 nRet;

    if (s_u32BlackFrameCount)
    {
        DISP_DEBUGK("black frame has been created! %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    DISP_MEMSET(&s_stBlackFrame, 0, sizeof(BUF_S));
    nRet = DISP_OS_MMZ_AllocAndMap((const char *)"VDP_BlackFrame", MT_NULL, BufSize, 16, &s_stBlackFrame.stMem);
    if (nRet)
    {
        DISP_DEBUGK("Alloc black frame memory failed %s\n", __FUNCTION__);
        return MT_FAILURE;
    }

    pY = (mt_uchar *)(s_stBlackFrame.stMem.u32StartVirAddr);
    DISP_MEMSET(pY, 0x10, BufSize / 2);
    pUV = (mt_uchar *)(s_stBlackFrame.stMem.u32StartVirAddr + BLACK_FRAME_WIDTH * BLACK_FRAME_HEIGHT);
    DISP_MEMSET(pUV, 0x80, BufSize / 2);

    pstFrame->u32FrameIndex = 0;
    pstFrame->stBufAddr[0].u32PhyAddr_Y = s_stBlackFrame.stMem.u32StartPhyAddr;
    pstFrame->stBufAddr[0].u32PhyAddr_C = pstFrame->stBufAddr[0].u32PhyAddr_Y + BLACK_FRAME_WIDTH * BLACK_FRAME_HEIGHT;
    pstFrame->stBufAddr[0].u32Stride_Y = BLACK_FRAME_WIDTH;
    pstFrame->stBufAddr[0].u32Stride_C = BLACK_FRAME_WIDTH;

#if 0
    printk("@@@@@@@@@ Black frame Y=0x%x, C=0x%x\n",
             pstFrame->stBufAddr[0].u32PhyAddr_Y,
             pstFrame->stBufAddr[0].u32PhyAddr_C);
#endif

    pstFrame->u32Width = BLACK_FRAME_WIDTH;
    pstFrame->u32Height = BLACK_FRAME_HEIGHT;

    pstFrame->u32SrcPts = 0xffffffff; /* 0xffffffff means unknown */
    pstFrame->u32Pts = 0xffffffff;    /* 0xffffffff means unknown */

    pstFrame->u32AspectWidth = 0;
    pstFrame->u32AspectHeight = 0;
    pstFrame->u32FrameRate = 0; /* in 1/100 Hz, 0 means unknown */

    pstFrame->ePixFormat = BLACK_FRAME_PIXFMT;
    pstFrame->bProgressive = MT_TRUE;
    pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
    pstFrame->bTopFieldFirst = MT_TRUE;

    //display region in rectangle (x,y,w,h)
    pstFrame->stDispRect.s32X = 0;
    pstFrame->stDispRect.s32Y = 0;
    pstFrame->stDispRect.s32Width = pstFrame->u32Width;
    pstFrame->stDispRect.s32Height = pstFrame->u32Height;

    pstFrame->eFrmType = MT_DRV_FT_NOT_STEREO;
    pstFrame->u32Circumrotate = 0;
    pstFrame->bToFlip_H = MT_FALSE;
    pstFrame->bToFlip_V = MT_FALSE;
    pstFrame->u32ErrorLevel = 0;

    s_u32BlackFrameCount++;
    return MT_SUCCESS;
}

mt_s32 BP_DestroyBlackFrame(mt_void)
{
    if (!s_u32BlackFrameCount)
    {
        DISP_ERROR("black frame has not been created!\n");
        return MT_FAILURE;
    }

    DISP_OS_MMZ_UnmapAndRelease(&s_stBlackFrame.stMem);

    DISP_MEMSET(&s_stBlackFrame, 0, sizeof(BUF_S));

    s_u32BlackFrameCount = 0;
    return MT_SUCCESS;
}

MT_DRV_VIDEO_FRAME_S *BP_GetBlackFrameInfo(mt_void)
{
    if (s_u32BlackFrameCount)
    {
        return &s_stBlackFrame.stFrame;
    }

    DISP_ERROR("black frame has not been created OR NULL pointer!\n");
    return MT_NULL;
}

mt_s32 BP_AddCfgWritingBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;
    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("AC invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingWPtr].u32Index != WIN_BUFFER_INDEX_INVALID)
    {
        DISP_ERROR("AC queue full!\n");
        return MT_FAILURE;
    }
    else if (pstBP->pstBufQueue[index].enState != BUF_STATE_WRITING)
    {
        DISP_ERROR("AC invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
    else
    {
        pstBP->pstBufQueue[index].enState = BUF_STATE_WRITING;
        pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingWPtr].u32Index = u32BufId;
        pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingWPtr].u32NodeStage = 0;

        pstBP->u32CfgWritingWPtr = (pstBP->u32CfgWritingWPtr + 1) % pstBP->u32BufNum;
        pstBP->stStatistic.u32CfgAdd++;
        return MT_SUCCESS;
    }
}

mt_s32 BP_DelCfgWritingBuf(BUF_POOL_S *pstBP, mt_u32 u32BufId)
{
    mt_u32 index;

    index = (u32BufId >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    if (((u32BufId & WIN_BUFFER_INDEX_PREFIX_MASK) != WIN_BUFFER_INDEX_PREFIX) || (index >= pstBP->u32BufNum))
    {
        DISP_ERROR("DC invalid id=0x%x\n", u32BufId);
        return MT_FAILURE;
    }

    if (BUF_STATE_WRITING == pstBP->pstBufQueue[index].enState)
    {
        pstBP->pstBufQueue[index].enState = BUF_STATE_WRITING;
        pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingRPtr].u32Index = WIN_BUFFER_INDEX_INVALID;
        pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingRPtr].u32NodeStage = 0;

        pstBP->u32CfgWritingRPtr = (pstBP->u32CfgWritingRPtr + 1) % pstBP->u32BufNum;
        pstBP->stStatistic.u32CfgDoing = index;
        pstBP->stStatistic.u32CfgDel++;
        return MT_SUCCESS;
    }
    else
    {
        DISP_WARN("DC invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
}
mt_s32 BP_GetCfgWritingBuf(BUF_POOL_S *pstBP, mt_u32 *pu32BufId, mt_u32 u32CfgStage, mt_u32 *u32CfgStageAct)
{
    mt_u32 id, index;

    id = pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingRPtr].u32Index;
    if (WIN_BUFFER_INDEX_INVALID == id)
    {
        DISP_INFO("GC queue empty!\n");
        return MT_FAILURE;
    }

    index = (id >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;

    /*cfg frame, may be configured, may be write back already
     *we just get we want.
     */
    if ((BUF_STATE_WRITING == pstBP->pstBufQueue[index].enState) &&
        (u32CfgStage <= pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingRPtr].u32NodeStage))
    {
        *pu32BufId = id;
        *u32CfgStageAct = pstBP->pstCfgWritingQueue[pstBP->u32CfgWritingRPtr].u32NodeStage;
        return MT_SUCCESS;
    }
    else
    {
        DISP_WARN("GC invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
        return MT_FAILURE;
    }
}

mt_s32 BP_GetCfgWritingBuf_JustWriting(BUF_POOL_S *pstBP, mt_u32 *pu32BufId, mt_u32 u32Stage)
{
    mt_u32 u32Id = 0, index = 0, i = 0;

    if (pstBP->u32CfgWritingRPtr == pstBP->u32CfgWritingWPtr)
    {
        return MT_FAILURE;
    }

    for (i = pstBP->u32CfgWritingRPtr; (i % pstBP->u32BufNum) != pstBP->u32CfgWritingWPtr; i++)
    {
        u32Id = pstBP->pstCfgWritingQueue[i % pstBP->u32BufNum].u32Index;
        if (WIN_BUFFER_INDEX_INVALID == u32Id)
        {
            DISP_INFO("GC queue empty!\n");
            return MT_FAILURE;
        }

        index = (u32Id >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;
        if (BUF_STATE_WRITING != pstBP->pstBufQueue[index].enState)
        {
            DISP_INFO("GC queue empty!\n");
            return MT_FAILURE;
        }

        /*equal to 1 means the frame is written back just now.*/
        if (u32Stage == pstBP->pstCfgWritingQueue[i % pstBP->u32BufNum].u32NodeStage)
        {
            *pu32BufId = u32Id;

            return MT_SUCCESS;
        }
    }

    return MT_FAILURE;
}

mt_void BP_IncreaseAllCfgWritingState(BUF_POOL_S *pstBP, mt_u32 *pu32BufId)
{
    mt_u32 u32Id = 0, index, i = 0;

    if (pstBP->u32CfgWritingRPtr == pstBP->u32CfgWritingWPtr) {
  return;
    }

    for (i = pstBP->u32CfgWritingRPtr; (i % pstBP->u32BufNum) != pstBP->u32CfgWritingWPtr; i++)
    {
        u32Id = pstBP->pstCfgWritingQueue[i % pstBP->u32BufNum].u32Index;
        if (WIN_BUFFER_INDEX_INVALID == u32Id)
        {
            DISP_INFO("GC queue empty!\n");
            return;
        }

        index = (u32Id >> WIN_BUFFER_INDEX_SHIFT) & WIN_BUFFER_INDEX_MASK;
        if (BUF_STATE_WRITING == pstBP->pstBufQueue[index].enState)
        {
            pstBP->pstCfgWritingQueue[i % pstBP->u32BufNum].u32NodeStage++;
        }
        else
        {
            DISP_WARN("GC invalid state buf[%d]=%d\n", index, (mt_s32)pstBP->pstBufQueue[index].enState);
            break;
        }
    }

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
