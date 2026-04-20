/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_wbc.h"
#include "vpss_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

mt_u32 VPSS_WBC_GetTotalBufSize(VPSS_WBC_ATTR_S* pstAttr, mt_u32 u32NodeBufSize)
{
    mt_u32 u32TotalBufSize = 0;
    switch (pstAttr->enMode)
    {
        case VPSS_WBC_MODE_NORMAL:
        {
            u32TotalBufSize = u32NodeBufSize * 2;
            break;
        }
        case VPSS_WBC_MODE_3FIELD:
        {
            u32TotalBufSize = u32NodeBufSize * 2;
            break;
        }
        case VPSS_WBC_MODE_5FIELD:
        {
            u32TotalBufSize = u32NodeBufSize * 3;
            break;
        }
        default:
        {
            VPSS_ERROR("Vpss wbc GetTotalBufSize error(WbcMode = %d).\n", pstAttr->enMode);
            return 0;
        }
    }

    return u32TotalBufSize;
}
mt_s32 VPSS_WBC_CalBufSize(mt_u32* pSize, mt_u32* pStride, VPSS_WBC_ATTR_S* pstAttr)
{
    mt_u32 u32RetSize = 0;
    mt_u32 u32RetStride = 0;
    mt_u32 u32Height = pstAttr->u32Height;
    mt_u32 u32Width = pstAttr->u32Width;
    MT_DRV_PIX_FORMAT_E ePixFormat = pstAttr->ePixFormat;
    MT_DRV_PIXEL_BITWIDTH_E  enBitWidth = pstAttr->enBitWidth;

    switch (ePixFormat)
    {
        case MT_DRV_PIX_FMT_NV12:
        case MT_DRV_PIX_FMT_NV21:
        case MT_DRV_PIX_FMT_NV12_TILE://wbc don't support tile,use linear
        case MT_DRV_PIX_FMT_NV21_TILE://wbc don't support tile,use linear
        {
            if (MT_DRV_PIXEL_BITWIDTH_8BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 3 / 2;
            }
            else if (MT_DRV_PIXEL_BITWIDTH_10BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 3 / 2;
            }
            else
            {
                VPSS_FATAL("Unsupport BitWidth %d.\n", enBitWidth);
                return MT_FAILURE;
            }
            break;
        }
        case MT_DRV_PIX_FMT_NV16_2X1:
        case MT_DRV_PIX_FMT_NV61_2X1:
        case MT_DRV_PIX_FMT_YUYV:
        case MT_DRV_PIX_FMT_YVYU:
        case MT_DRV_PIX_FMT_UYVY:
        {
            if (MT_DRV_PIXEL_BITWIDTH_8BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 2;
            }
            else if (MT_DRV_PIXEL_BITWIDTH_10BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 2;
            }
            else
            {
                VPSS_FATAL("Unsupport BitWidth %d.\n", enBitWidth);
                return MT_FAILURE;
            }
            break;
        }
        case MT_DRV_PIX_FMT_NV12_CMP:
        case MT_DRV_PIX_FMT_NV21_CMP:
        {
            if (MT_DRV_PIXEL_BITWIDTH_8BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 3 / 2 + 16 * u32Height * 3 / 2;
            }
            else if (MT_DRV_PIXEL_BITWIDTH_10BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 3 / 2 + 16 * u32Height * 3 / 2;
            }
            else
            {
                VPSS_FATAL("Unsupport BitWidth %d.\n", enBitWidth);
                return MT_FAILURE;
            }
            break;
        }
        case MT_DRV_PIX_FMT_NV16_CMP:
        case MT_DRV_PIX_FMT_NV61_CMP:
        {
            if (MT_DRV_PIXEL_BITWIDTH_8BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 3 / 2 + 16 * u32Height * 2;
            }
            else if (MT_DRV_PIXEL_BITWIDTH_10BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
                u32RetSize = u32Height * u32RetStride * 3 / 2 + 16 * u32Height * 2;
            }
            else
            {
                VPSS_FATAL("Unsupport BitWidth %d.\n", enBitWidth);
                return MT_FAILURE;
            }
            break;
        }
        case MT_DRV_PIX_FMT_ARGB8888:
        case MT_DRV_PIX_FMT_ABGR8888:
        {
            if (MT_DRV_PIXEL_BITWIDTH_8BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width*4);
                u32RetSize = u32Height * u32RetStride;
            }
            else if (MT_DRV_PIXEL_BITWIDTH_10BIT == enBitWidth)
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width*4);
                u32RetSize = u32Height * u32RetStride;
            }
            else
            {
                VPSS_FATAL("Unsupport BitWidth %d.\n", enBitWidth);
                return MT_FAILURE;
            }
            break;
        }
        default:
        {
            VPSS_FATAL("Unsupport PixFormat %d.\n", ePixFormat);
            return MT_FAILURE;
        }
    }
    *pSize = u32RetSize;
    *pStride = u32RetStride;
    return MT_SUCCESS;
}

mt_s32 VPSS_WBC_InitFrame(MT_DRV_VIDEO_FRAME_S* pstWbcFrame, VPSS_WBC_ATTR_S* pstAttr, mt_u32 u32PhyAddr)
{
    mt_u32 u32Stride = 0;
    mt_u32 u32BuffSize = 0;
    mt_s32 s32Ret = MT_FAILURE;

    s32Ret = VPSS_WBC_CalBufSize(&u32BuffSize, &u32Stride, pstAttr);

    if (MT_FAILURE == s32Ret)
    {
        return MT_FAILURE;
    }

    pstWbcFrame->u32Width = pstAttr->u32Width;
    pstWbcFrame->u32Height = pstAttr->u32Height;
    pstWbcFrame->enBitWidth = pstAttr->enBitWidth;
    pstWbcFrame->ePixFormat = pstAttr->ePixFormat;
    pstWbcFrame->stBufAddr[0].u32Stride_Y = u32Stride;
    pstWbcFrame->stBufAddr[0].u32Stride_C = u32Stride;
    pstWbcFrame->u32FrameIndex = pstAttr->u32FrameIndex;
    pstWbcFrame->u32Pts = pstAttr->u32Pts;


    switch (pstAttr->ePixFormat)
    {
        case MT_DRV_PIX_FMT_NV12_TILE://wbc don't support tile,use linear
            pstWbcFrame->ePixFormat = MT_DRV_PIX_FMT_NV12;
            break;
        case MT_DRV_PIX_FMT_NV21_TILE://wbc don't support tile,use linear
            pstWbcFrame->ePixFormat = MT_DRV_PIX_FMT_NV21;
            break;
        case MT_DRV_PIX_FMT_NV12_TILE_CMP://wbc don't support tile,use linear
            pstWbcFrame->ePixFormat = MT_DRV_PIX_FMT_NV12_CMP;
            break;
        case MT_DRV_PIX_FMT_NV21_TILE_CMP://wbc don't support tile,use linear
            pstWbcFrame->ePixFormat = MT_DRV_PIX_FMT_NV21_CMP;
            break;
        case MT_DRV_PIX_FMT_YUYV:
        case MT_DRV_PIX_FMT_YVYU:
        case MT_DRV_PIX_FMT_UYVY:
            pstWbcFrame->ePixFormat = MT_DRV_PIX_FMT_NV61_2X1;
            break;
        default:
        {
            pstWbcFrame->ePixFormat = pstAttr->ePixFormat;
        }
    }

    switch (pstAttr->ePixFormat)
    {
        case MT_DRV_PIX_FMT_NV12:
        case MT_DRV_PIX_FMT_NV21:
        case MT_DRV_PIX_FMT_NV12_TILE://wbc don't support tile,use linear
        case MT_DRV_PIX_FMT_NV21_TILE://wbc don't support tile,use linear
        case MT_DRV_PIX_FMT_NV16_2X1:
        case MT_DRV_PIX_FMT_NV61_2X1:
        case MT_DRV_PIX_FMT_YUYV:
        case MT_DRV_PIX_FMT_YVYU:
        case MT_DRV_PIX_FMT_UYVY:
        {
            pstWbcFrame->stBufAddr[0].u32PhyAddr_Y = u32PhyAddr;
            pstWbcFrame->stBufAddr[0].u32PhyAddr_C = u32PhyAddr + u32Stride * pstAttr->u32Height;

            break;
        }
        case MT_DRV_PIX_FMT_NV12_CMP:
        case MT_DRV_PIX_FMT_NV21_CMP:
        case MT_DRV_PIX_FMT_NV16_CMP:
        case MT_DRV_PIX_FMT_NV61_CMP:
        {
            pstWbcFrame->stBufAddr[0].u32PhyAddr_YHead = u32PhyAddr;
            pstWbcFrame->stBufAddr[0].u32PhyAddr_Y = u32PhyAddr + pstAttr->u32Height * 16;
            pstWbcFrame->stBufAddr[0].u32PhyAddr_CHead =
                pstWbcFrame->stBufAddr[0].u32PhyAddr_Y + u32Stride * pstAttr->u32Height;

            if (MT_DRV_PIX_FMT_NV12_CMP == pstAttr->ePixFormat
                || MT_DRV_PIX_FMT_NV21_CMP == pstAttr->ePixFormat)
            {
                pstWbcFrame->stBufAddr[0].u32PhyAddr_C =
                    pstWbcFrame->stBufAddr[0].u32PhyAddr_CHead + pstAttr->u32Height * 16 / 2;
            }
            else
            {
                pstWbcFrame->stBufAddr[0].u32PhyAddr_C =
                    pstWbcFrame->stBufAddr[0].u32PhyAddr_CHead + pstAttr->u32Height * 16;
            }

            break;
        }
        case MT_DRV_PIX_FMT_ARGB8888:
        case MT_DRV_PIX_FMT_ABGR8888:
        {
            pstWbcFrame->stBufAddr[0].u32PhyAddr_Y = u32PhyAddr;
            break;
        }
        default:
        {
            VPSS_FATAL("Unsupport PixFormat %d.\n", pstAttr->ePixFormat);
            return MT_FAILURE;
        }
    }

    memcpy(&pstWbcFrame->stBufAddr[1], &pstWbcFrame->stBufAddr[0],
        sizeof(MT_DRV_VID_FRAME_ADDR_S));

    return MT_SUCCESS;
}
mt_s32 VPSS_WBC_Init(VPSS_WBC_S* pstWbc, VPSS_WBC_ATTR_S* pstAttr)
{
    LIST* pListHead = MT_NULL;
    mt_u32 u32Stride = 0;
    mt_u32 u32NodeBuffSize = 0;
    mt_u32 u32TotalBuffSize = 0;
    mt_u32 u32PhyAddr = 0;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 i = 0;
    mt_u32 u32NodeCount;

    if (MT_NULL == pstWbc)
    {
        VPSS_ERROR("Vpss wbc Init error(null pointer).\n");
        return MT_FAILURE;
    }
    if (MT_TRUE == pstWbc->bInit)
    {
        (mt_void)VPSS_WBC_DeInit(pstWbc);
    }

    if ((VPSS_WBC_REF_MODE_INIT > pstAttr->enRefMode) || (VPSS_WBC_REF_MODE_BUTT <= pstAttr->enRefMode))
    {
        VPSS_ERROR("Vpss wbc Init error(RefMode = %d).\n", pstAttr->enRefMode);
        return MT_FAILURE;
    }

    if ((MT_DRV_PIXEL_BITWIDTH_8BIT != pstAttr->enBitWidth) && (MT_DRV_PIXEL_BITWIDTH_10BIT != pstAttr->enBitWidth))
    {
        VPSS_ERROR("Vpss wbc Init error(BitWidth = %d).\n", pstAttr->enBitWidth);
        return MT_FAILURE;
    }

    memset(pstWbc, 0, sizeof(VPSS_WBC_S));
    memcpy(&pstWbc->stWbcAttr, pstAttr, sizeof(VPSS_WBC_ATTR_S));

    s32Ret = VPSS_WBC_CalBufSize(&u32NodeBuffSize, &u32Stride, pstAttr);

    if (MT_FAILURE == s32Ret)
    {
        return MT_FAILURE;
    }

    u32TotalBuffSize = VPSS_WBC_GetTotalBufSize(pstAttr, u32NodeBuffSize);
    s32Ret = mt_drv_mmz_alloc_and_map( "VPSS_WbcBuf", "VPSS", u32TotalBuffSize, 0, &(pstWbc->stMMZBuf));

    if (MT_FAILURE == s32Ret)
    {
        VPSS_FATAL("VPSS WBC Alloc memory failed.\n");
        return MT_FAILURE;
    }

    u32PhyAddr = pstWbc->stMMZBuf.u32StartPhyAddr;
    u32NodeCount = (mt_u32)pstWbc->stWbcAttr.enMode;
    pListHead = &(pstWbc->stDataList[0].node);
    INIT_LIST_HEAD(pListHead);
    pstWbc->pstFirstRef = pListHead;

    for (i = 1 ; i < u32NodeCount; i++)
    {
        list_add_tail(&(pstWbc->stDataList[i].node), pListHead);
    }

    switch (pstAttr->enMode)
    {
        case VPSS_WBC_MODE_NORMAL:
        {
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[0].stWbcFrame), pstAttr, u32PhyAddr);
            u32PhyAddr = u32PhyAddr + u32NodeBuffSize;
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[1].stWbcFrame), pstAttr, u32PhyAddr);
            break;
        }
        case VPSS_WBC_MODE_3FIELD:
        {
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[0].stWbcFrame), pstAttr, u32PhyAddr);
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[1].stWbcFrame), pstAttr, u32PhyAddr + u32Stride);
            u32PhyAddr = u32PhyAddr + u32NodeBuffSize;
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[2].stWbcFrame), pstAttr, u32PhyAddr);
            break;
        }
        case VPSS_WBC_MODE_5FIELD:
        {
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[0].stWbcFrame), pstAttr, u32PhyAddr);
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[1].stWbcFrame), pstAttr, u32PhyAddr + u32Stride);
            u32PhyAddr = u32PhyAddr + u32NodeBuffSize;
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[2].stWbcFrame), pstAttr, u32PhyAddr);
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[3].stWbcFrame), pstAttr, u32PhyAddr + u32Stride);
            u32PhyAddr = u32PhyAddr + u32NodeBuffSize;
            VPSS_WBC_InitFrame(&(pstWbc->stDataList[4].stWbcFrame), pstAttr, u32PhyAddr);
            break;
        }
        default:
        {
            VPSS_ERROR("Vpss wbc Init error(WbcMode = %d).\n", pstWbc->stWbcAttr.enMode);
            return MT_FAILURE;
        }
    }

    pstWbc->bInit = MT_TRUE;
    return MT_SUCCESS;
}

mt_s32 VPSS_WBC_DeInit(VPSS_WBC_S* pstWbc)
{
    if (MT_NULL == pstWbc)
    {
        VPSS_ERROR("Vpss wbc DeInit error(null pointer).\n");
        return MT_FAILURE;
    }
    if (MT_FALSE == pstWbc->bInit)
    {
        VPSS_WARN("Vpss wbc DeInit error(not init).\n");
        return MT_SUCCESS;
    }


    if (0 != pstWbc->stMMZBuf.u32StartPhyAddr)
    {
        mt_drv_mmz_unmap_and_release(& (pstWbc->stMMZBuf));
    }

    memset(pstWbc,0,sizeof(VPSS_WBC_S));

    return MT_SUCCESS;
}

mt_s32 VPSS_WBC_GetRefInfo(VPSS_WBC_S* pstWbc,
                           MT_DRV_VIDEO_FRAME_S** pstDataList)
{
    mt_u32 i;
    VPSS_WBC_DATA_S* pstRefNodeData = MT_NULL;
    LIST* pstRefNode = MT_NULL;
    VPSS_WBC_REF_MODE_E enRefMode;
    mt_u32 u32NodeCount;

    if ((MT_NULL == pstWbc) || (MT_NULL == pstDataList))
    {
        VPSS_ERROR("Vpss wbc GetRefInfo error(null pointer).\n");
        return MT_FAILURE;
    }

    if (MT_FALSE == pstWbc->bInit)
    {
        VPSS_ERROR("Vpss wbc GetRefInfo error(not init).\n");
        return MT_FAILURE;
    }

    enRefMode = pstWbc->stWbcAttr.enRefMode;
    u32NodeCount = (mt_u32)pstWbc->stWbcAttr.enMode - 1;
    pstRefNode = pstWbc->pstFirstRef->prev->prev;

    for (i = 0; i < u32NodeCount; i++)
    {
        pstRefNodeData = list_entry(pstRefNode, VPSS_WBC_DATA_S, node);
        pstDataList[u32NodeCount - i - 1] = &(pstRefNodeData->stWbcFrame);

        pstRefNode = pstRefNode->prev;
    }

    for (i = pstWbc->u32CompleteCount; i < u32NodeCount; i++)
    {
        if (VPSS_WBC_REF_MODE_NULL == enRefMode)
        {
            pstDataList[u32NodeCount - i - 1] = MT_NULL;
        }
    }

    return MT_SUCCESS;
}
mt_s32 VPSS_WBC_GetWbcInfo(VPSS_WBC_S* pstWbc, MT_DRV_VIDEO_FRAME_S** pstData)
{
    VPSS_WBC_DATA_S* pstWbcNodeData = MT_NULL;
    LIST* pstWbcNode = MT_NULL;

    if ((MT_NULL == pstWbc) || (MT_NULL == pstData))
    {
        VPSS_ERROR("Vpss wbc GetWbcInfo error(null pointer).\n");
        return MT_FAILURE;
    }

    if (MT_FALSE == pstWbc->bInit)
    {
        VPSS_ERROR("Vpss wbc GetWbcInfo error(not init).\n");
        return MT_FAILURE;
    }

    pstWbcNode = pstWbc->pstFirstRef->prev;
    pstWbcNodeData = list_entry(pstWbcNode, VPSS_WBC_DATA_S, node);
    *pstData = &(pstWbcNodeData->stWbcFrame);

    return MT_SUCCESS;

}

mt_s32 VPSS_WBC_Complete(VPSS_WBC_S* pstWbc)
{
    if (MT_NULL == pstWbc)
    {
        VPSS_ERROR("Vpss wbc complete error(null pointer).\n");
        return MT_FAILURE;
    }

    if (MT_FALSE == pstWbc->bInit)
    {
        VPSS_ERROR("Vpss wbc complete error(not init).\n");
        return MT_FAILURE;
    }

    if (0 == (pstWbc->u32CompleteCount + 1))
    {
        pstWbc->u32CompleteCount = VPSS_WBC_MAX_NODE;
    }

    pstWbc->u32CompleteCount++;
    pstWbc->pstFirstRef = pstWbc->pstFirstRef->next;

    return MT_SUCCESS;
}

mt_s32 VPSS_WBC_Reset(VPSS_WBC_S* pstWbc)
{
    if (MT_NULL == pstWbc)
    {
        VPSS_WARN("Vpss wbc reset error(null pointer).\n");
        return MT_FAILURE;
    }

    if (MT_FALSE == pstWbc->bInit)
    {
        VPSS_WARN("Vpss wbc reset error(not init).\n");
        return MT_FAILURE;
    }

    pstWbc->u32CompleteCount = 0;

    return MT_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


