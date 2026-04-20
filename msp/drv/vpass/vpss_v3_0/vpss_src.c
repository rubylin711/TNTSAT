/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_src.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

static inline MT_BOOL VPSS_SRC_IsProgressive(VPSS_SRC_DATA_S *pstData)
{
	return pstData->bProgressive;
}

static inline MT_BOOL VPSS_SRC_IsFieldAll(VPSS_SRC_DATA_S *pstData)
{
	return (MT_DRV_FIELD_ALL == pstData->enFieldMode) ;
}

static inline MT_BOOL VPSS_SRC_IsTopFirst(VPSS_SRC_DATA_S *pstData)
{
	return pstData->bTopFieldFirst;
}

static inline MT_BOOL VPSS_SRC_IsFull(VPSS_SRC_S *pstSrc)
{
    return list_empty_careful(&(pstSrc->stEmptySrcList));
}

static inline MT_BOOL VPSS_SRC_IsEmpty(VPSS_SRC_S *pstSrc)
{
    return list_empty_careful(&(pstSrc->stFulSrcList));
}

static inline MT_BOOL VPSS_SRC_IsSomeSpaceLeft(VPSS_SRC_S *pstSrc,mt_u32 u32Numb)
{
    mt_u32 u32Cnt = 0;
    LIST *pos, *n;
    list_for_each_safe(pos, n, &(pstSrc->stEmptySrcList))
    {
        u32Cnt++;
    }
    return (u32Cnt >= u32Numb);
}
static inline mt_u32 VPSS_SRC_FulListLenth(VPSS_SRC_S *pstSrc)
{
    mt_u32 u32Cnt = 0;
    LIST *pos, *n;
    list_for_each_safe(pos, n, &(pstSrc->stFulSrcList))
    {
        u32Cnt++;
    }
    return u32Cnt;
}
static inline MT_BOOL VPSS_SRC_IsUndoDataLeft(VPSS_SRC_S *pstSrc)
{
    return (pstSrc->pstTarget_1->next != &(pstSrc->stFulSrcList));
}
static inline VPSS_SRC_DATA_S *VPSS_SRC_GetData(LIST *pstNodePtr)
{
    VPSS_SRC_NODE_S *pstNode;
    pstNode = list_entry(pstNodePtr, VPSS_SRC_NODE_S, node);
    return &(pstNode->stSrcData);
}
static inline mt_void VPSS_SRC_AddFulNode(VPSS_SRC_S *pstSrc,VPSS_SRC_NODE_S* pstDataNode)
{
    list_add_tail(&(pstDataNode->node), &(pstSrc->stFulSrcList));
}
static inline mt_void VPSS_SRC_AddEmptyNode(VPSS_SRC_S *pstSrc,VPSS_SRC_NODE_S* pstDataNode)
{
    list_add_tail(&(pstDataNode->node), &(pstSrc->stEmptySrcList));
}

static VPSS_SRC_NODE_S* VPSS_SRC_DelEmptyNode(VPSS_SRC_S *pstSrc)
{
    LIST *pos, *n;
    VPSS_SRC_NODE_S *pstTarget = MT_NULL;
    list_for_each_safe(pos, n, &(pstSrc->stEmptySrcList))
    {
        pstTarget = list_entry(pos, VPSS_SRC_NODE_S, node);
        list_del_init(pos);
        break;
    }

    if(pstTarget)
    {
        memset(&(pstTarget->stSrcData), 0,
                sizeof(VPSS_SRC_DATA_S));
        return pstTarget;
    }
    else
    {
        return MT_NULL;
    }
}
#if 1
mt_s32 VPSS_SRC_RlsDoneFullNode(VPSS_SRC_S *pstSrc)
{
    LIST *pos, *n, *head, *ListNewFirstNode;
    VPSS_SRC_DATA_S *pstFstData;
    VPSS_SRC_DATA_S *pstSndData;
    VPSS_SRC_NODE_S *pstNode;

    head = &(pstSrc->stFulSrcList);

    if (pstSrc->enMode == SRC_MODE_FIELD
        || pstSrc->enMode == SRC_MODE_NTSC
        || pstSrc->enMode == SRC_MODE_PAL)
    {
        pos = (head)->next;
        n = pos->next;
        ListNewFirstNode = n->next;
        if (pos == head || n == head)
        {
            VPSS_ERROR("Can't get first two released nodes\n");
            return MT_FAILURE;
        }
        pstFstData = VPSS_SRC_GetData(pstSrc->stFulSrcList.next);
        pstSndData = VPSS_SRC_GetData(pstSrc->stFulSrcList.next->next);
        if (pstFstData->u32FrameIndex != pstSndData->u32FrameIndex)
        {
            VPSS_ERROR("not same frame(%d), (%d)!\n",
                pstFstData->u32FrameIndex, pstSndData->u32FrameIndex);
            return MT_FAILURE;
        }

        if(pstFstData->enFieldMode == MT_DRV_FIELD_TOP)
        {
            pstSrc->pfnRlsImage(pstSrc->hSrcModule,pstFstData);
        }
        else
        {
            pstSrc->pfnRlsImage(pstSrc->hSrcModule,pstSndData);
        }
    }
    else
    {
        pos = (head)->next;
        ListNewFirstNode = pos->next;
        if (pos == head)
        {
            VPSS_ERROR("Can't get first one released nodes\n");
            return MT_FAILURE;
        }
        pstFstData = VPSS_SRC_GetData(pstSrc->stFulSrcList.next);

        pstSrc->pfnRlsImage(pstSrc->hSrcModule,pstFstData);
    }

    for (pos = (head)->next, n = pos->next;
         pos != ListNewFirstNode
         && pos != head;
         pos = n, n = pos->next)
    {
        pstNode = list_entry(pos,
                        VPSS_SRC_NODE_S, node);
        if (pos == pstSrc->pstTarget_1)
        {
            pstSrc->pstTarget_1 = pstSrc->pstTarget_1->prev;
        }
        list_del_init(&(pstNode->node));

        VPSS_SRC_AddEmptyNode(pstSrc,pstNode);
    }

    pstSrc->u32ReleaseSrcCount++;
    return MT_SUCCESS;
}
#endif
mt_s32 VPSS_SRC_Init(VPSS_SRC_S *pstSrc,VPSS_SRC_ATTR_S stAttr)
{
    mt_u32 u32Cnt = 0;
    VPSS_SRC_NODE_S *pstSrcNode;
    LIST *pos, *n;

    VPSS_CHECK_NULL(pstSrc);

    if (stAttr.enMode != SRC_MODE_FRAME
        && stAttr.enMode != SRC_MODE_FIELD
        && stAttr.enMode != SRC_MODE_NTSC
        && stAttr.enMode != SRC_MODE_PAL)
    {
        VPSS_ERROR("Para enMode %#x is Invalid.\n",stAttr.enMode);
        return MT_FAILURE;
    }

    VPSS_CHECK_NULL(stAttr.pfnRlsImage);

    if(pstSrc->bInit)
    {
        (mt_void)VPSS_SRC_DeInit(pstSrc);
    }

    memset(pstSrc,0,sizeof(VPSS_SRC_S));

    switch(stAttr.enMode)
    {
        case SRC_MODE_FRAME:
            pstSrc->u32ListLenth = DEF_SRC_MODE_FRAME_NUMB;
            break;
        case SRC_MODE_FIELD:
            pstSrc->u32ListLenth = DEF_SRC_MODE_FIELD_NUMB;
            break;
        case SRC_MODE_NTSC:
            pstSrc->u32ListLenth = DEF_SRC_MODE_NTSC_NUMB;
            break;
        case SRC_MODE_PAL:
            pstSrc->u32ListLenth = DEF_SRC_MODE_PAL_NUMB;
            break;
        default:
            VPSS_ERROR("Para enMode %#x is Invalid.\n",stAttr.enMode);
            return MT_FAILURE;
    }

    pstSrc->enMode  = stAttr.enMode;

    INIT_LIST_HEAD(&(pstSrc->stEmptySrcList));
    INIT_LIST_HEAD(&(pstSrc->stFulSrcList));

    for (u32Cnt = 0; u32Cnt < pstSrc->u32ListLenth; u32Cnt++)
    {
        pstSrcNode = (VPSS_SRC_NODE_S*)VPSS_KMALLOC(sizeof(VPSS_SRC_NODE_S),GFP_ATOMIC);
        if (pstSrcNode == MT_NULL)
        {
            VPSS_ERROR("vmalloc SrcNode failed\n");
            goto SRC_Init_Failed;
        }
        memset(&(pstSrcNode->stSrcData), 0,
                sizeof(VPSS_SRC_DATA_S));

        list_add_tail(&(pstSrcNode->node),
                        &(pstSrc->stEmptySrcList));
    }
    pstSrc->pstTarget_1 = &(pstSrc->stFulSrcList);


    pstSrc->pfnRlsImage = stAttr.pfnRlsImage;
    pstSrc->hSrcModule = stAttr.hSrcModule;

    pstSrc->bInit = MT_TRUE;

    return MT_SUCCESS;

SRC_Init_Failed:
    list_for_each_safe(pos, n, &(pstSrc->stEmptySrcList))
    {
        pstSrcNode = list_entry(pos, VPSS_SRC_NODE_S, node);

        list_del_init(pos);
        VPSS_KFREE(pstSrcNode);
    }

    return MT_FAILURE;
}

mt_s32 VPSS_SRC_DeInit(VPSS_SRC_S* pstSrc)
{
    VPSS_SRC_NODE_S *pstFstNode;
    VPSS_SRC_NODE_S *pstSndNode;
    VPSS_SRC_DATA_S *pstFstData;
    VPSS_SRC_DATA_S *pstSndData;

    LIST *pos, *n,*tmp;

    VPSS_CHECK_NULL(pstSrc);

    if(!pstSrc->bInit)
    {
        VPSS_WARN("Delint.\n");
        return MT_FAILURE;
    }

    //release source buffer
    list_for_each_safe(pos, n, &(pstSrc->stFulSrcList))
    {
        if(SRC_MODE_FRAME == pstSrc->enMode)
        {
            pstFstNode = list_entry(pos, VPSS_SRC_NODE_S, node);
            pstFstData = VPSS_SRC_GetData(pos);
            pstSrc->pfnRlsImage(pstSrc->hSrcModule,pstFstData);
            list_del_init(pos);
            VPSS_SRC_AddEmptyNode(pstSrc,pstFstNode);
        }
        else
        {
            pstFstNode = list_entry(pos, VPSS_SRC_NODE_S, node);
            pstSndNode = list_entry(n, VPSS_SRC_NODE_S, node);
            pstFstData = VPSS_SRC_GetData(pos);
            pstSndData = VPSS_SRC_GetData(n);
            if (pstFstData->u32FrameIndex != pstSndData->u32FrameIndex)
            {
                VPSS_ERROR("not same frame(%d), (%d)!\n",
                    pstFstData->u32FrameIndex, pstSndData->u32FrameIndex);
                return MT_FAILURE;
            }

            if(pstFstData->enFieldMode == MT_DRV_FIELD_TOP)
            {
                pstSrc->pfnRlsImage(pstSrc->hSrcModule,pstFstData);
            }
            else
            {
                pstSrc->pfnRlsImage(pstSrc->hSrcModule,pstSndData);
            }

            tmp = n->next;
            list_del_init(pos);
            list_del_init(n);
            VPSS_SRC_AddEmptyNode(pstSrc,pstFstNode);
            VPSS_SRC_AddEmptyNode(pstSrc,pstSndNode);
            n = tmp;
        }

    }

    list_for_each_safe(pos, n, &(pstSrc->stEmptySrcList))
    {
        pstFstNode = list_entry(pos, VPSS_SRC_NODE_S, node);

        list_del_init(pos);

        VPSS_KFREE(pstFstNode);
    }

    memset(pstSrc,0,sizeof(VPSS_SRC_S));

    return MT_SUCCESS;
}
mt_s32 VPSS_SRC_Reset(VPSS_SRC_S* pstSrc)
{
    LIST *pos, *n;
    VPSS_SRC_NODE_S *pstSrcNode;
    VPSS_CHECK_NULL(pstSrc);
    if(!pstSrc->bInit)
    {
        VPSS_INFO("VPSS src reset failed(not init).\n");
        return MT_FAILURE;
    }

    list_for_each_safe(pos, n, &(pstSrc->stFulSrcList))
    {
        pstSrcNode = list_entry(pos, VPSS_SRC_NODE_S, node);

        list_del_init(pos);

        VPSS_SRC_AddEmptyNode(pstSrc,pstSrcNode);
    }

    pstSrc->pstTarget_1 = &(pstSrc->stFulSrcList);
    pstSrc->u32PutSrcCount = 0;
    pstSrc->u32CompleteSrcCount = 0;
    pstSrc->u32ReleaseSrcCount = 0;
    return MT_SUCCESS;
}

#define DEF_SRC_FIELD_SPACE_NUMB 2
#define DEF_SRC_FRAME_SPACE_NUMB 1

mt_s32 VPSS_SRC_PutImage(VPSS_SRC_S *pstSrc,VPSS_SRC_DATA_S *pstData)
{
    MT_BOOL bProgressive;
    MT_BOOL bFieldAll;
    MT_BOOL bTopfirst;
    MT_BOOL bEnoughSpace;
    VPSS_SRC_NODE_S *pstDstNode;
    MT_DRV_VIDEO_PRIVATE_S *pstFrmPriv;
    MT_VDEC_PRIV_FRAMEINFO_S *pstVdecPriv;

    VPSS_CHECK_NULL(pstSrc);

    if(!pstSrc->bInit)
    {
        VPSS_INFO("VPSS_SRC_PutImage Delint.\n");
        return MT_FAILURE;
    }

    pstSrc->u32PutSrcCount++;
    bProgressive = VPSS_SRC_IsProgressive(pstData);
    bFieldAll = VPSS_SRC_IsFieldAll(pstData);
    bTopfirst = VPSS_SRC_IsTopFirst(pstData);

    if (bProgressive)// && bFieldAll
    {
        bEnoughSpace = VPSS_SRC_IsSomeSpaceLeft(pstSrc,DEF_SRC_FRAME_SPACE_NUMB);
    }
    else
    {
        bEnoughSpace = VPSS_SRC_IsSomeSpaceLeft(pstSrc,DEF_SRC_FIELD_SPACE_NUMB);
    }

    if (!bEnoughSpace)
    {
        VPSS_ERROR("there is not enough space.\n");
        return MT_FAILURE;
    }

    if (bProgressive && bFieldAll)
    {
        pstDstNode = VPSS_SRC_DelEmptyNode(pstSrc);
        if(MT_NULL == pstDstNode)
        {
            return MT_FAILURE;
        }
        memcpy(&(pstDstNode->stSrcData),pstData,sizeof(VPSS_SRC_DATA_S));
        VPSS_SRC_AddFulNode(pstSrc,pstDstNode);
    }
    else
    {
        VPSS_SRC_NODE_S *pstFstDstNode;
        VPSS_SRC_NODE_S *pstSndDstNode;
        VPSS_SRC_NODE_S *pstReviseNode;
        mt_u32 u32BaseAddr;
        mt_u32 u32BaseStride;
        pstFstDstNode = VPSS_SRC_DelEmptyNode(pstSrc);
        if(MT_NULL == pstFstDstNode)
        {
            return MT_FAILURE;
        }

        memcpy(&(pstFstDstNode->stSrcData),pstData,sizeof(VPSS_SRC_DATA_S));
        VPSS_SRC_AddFulNode(pstSrc,pstFstDstNode);

        pstSndDstNode = VPSS_SRC_DelEmptyNode(pstSrc);
        if(MT_NULL == pstSndDstNode)
        {
            return MT_FAILURE;
        }

        memcpy(&(pstSndDstNode->stSrcData),pstData,sizeof(VPSS_SRC_DATA_S));
        VPSS_SRC_AddFulNode(pstSrc,pstSndDstNode);

        if (bTopfirst)
        {
            pstFstDstNode->stSrcData.bProgressive = MT_FALSE;
            pstFstDstNode->stSrcData.enFieldMode = MT_DRV_FIELD_TOP;
            pstSndDstNode->stSrcData.bProgressive = MT_FALSE;
            pstSndDstNode->stSrcData.enFieldMode = MT_DRV_FIELD_BOTTOM;

            /*Revise field addr*/
            pstReviseNode = pstSndDstNode;

        }
        else
        {
            pstFstDstNode->stSrcData.bProgressive = MT_FALSE;
            pstFstDstNode->stSrcData.enFieldMode = MT_DRV_FIELD_BOTTOM;
            pstSndDstNode->stSrcData.bProgressive = MT_FALSE;
            pstSndDstNode->stSrcData.enFieldMode = MT_DRV_FIELD_TOP;

            /*Revise field addr*/
            pstReviseNode = pstFstDstNode;
        }

        pstFstDstNode->stSrcData.u32FrameRate *= 2;
        pstSndDstNode->stSrcData.u32FrameRate *= 2;
        pstFrmPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstFstDstNode->stSrcData.u32Priv[0]);
        pstVdecPriv = (MT_VDEC_PRIV_FRAMEINFO_S *)&(pstFrmPriv->u32Reserve[0]);
        pstSndDstNode->stSrcData.u32Pts = pstFstDstNode->stSrcData.u32Pts + pstVdecPriv->s32InterPtsDelta;

        //Is available?
        //:TODO:后面处理3D隔行的情况/宽度小于256时的情况暂时没有处理
        switch (pstData->ePixFormat)
        {
            case MT_DRV_PIX_FMT_NV12_TILE:
            case MT_DRV_PIX_FMT_NV21_TILE:
            case MT_DRV_PIX_FMT_NV12_TILE_CMP:
            case MT_DRV_PIX_FMT_NV21_TILE_CMP:
            {
                u32BaseAddr = pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_Y;
                u32BaseStride = 256;//pstReviseNode->stSrcData.stBufAddr[0].u32Stride_Y;

                pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_Y =
                                    u32BaseAddr + u32BaseStride;

                u32BaseAddr = pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_C;
                u32BaseStride = 256;//pstReviseNode->stSrcData.stBufAddr[0].u32Stride_C;

                pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_C =
                                    u32BaseAddr + u32BaseStride;
                break;
            }

            default:
            {
                u32BaseAddr = pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_Y;
                u32BaseStride =  pstReviseNode->stSrcData.stBufAddr[0].u32Stride_Y;

                pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_Y =
                                    u32BaseAddr + u32BaseStride;

                u32BaseAddr = pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_C;
                u32BaseStride =  pstReviseNode->stSrcData.stBufAddr[0].u32Stride_C;

                pstReviseNode->stSrcData.stBufAddr[0].u32PhyAddr_C =
                                    u32BaseAddr + u32BaseStride;
            }
        }

    }

    return MT_SUCCESS;
}

mt_s32 VPSS_SRC_GetProcessImage(VPSS_SRC_S *pstSrc,VPSS_SRC_DATA_S **pstData)
{
    mt_s32 s32Ret = MT_SUCCESS;
    VPSS_CHECK_NULL(pstSrc);

    if(!pstSrc->bInit)
    {
        VPSS_INFO("VPSS_SRC_GetProcessImage Delint.\n");
        return MT_FAILURE;
    }

    if (VPSS_SRC_IsUndoDataLeft(pstSrc))
    {
        *pstData = VPSS_SRC_GetData(pstSrc->pstTarget_1->next);
        s32Ret = MT_SUCCESS;
    }
    else
    {
        *pstData = MT_NULL;
        s32Ret = MT_FAILURE;
    }

    return s32Ret;
}

mt_s32 VPSS_SRC_GetPreImgInfo(VPSS_SRC_S *pstSrc,
                               VPSS_SRC_DATA_S **pPtrPreData,
                               VPSS_SRC_DATA_S **pPtrPpreData
                               )
{
    mt_u32 u32Cnt = 0;
    LIST *pos, *n;
    VPSS_SRC_DATA_S *pstData;
    VPSS_SRC_DATA_S *pstDataArray[SRC_MODE_PAL];
    VPSS_SRC_DATA_S *pstCurData;
	VPSS_SRC_MODE_E enMode;

    if(!pstSrc->bInit)
    {
        VPSS_INFO("VPSS_SRC_GetPreImgInfo Delint.\n");
        return MT_FAILURE;
    }

	enMode = pstSrc->enMode;
	if((SRC_MODE_NTSC != enMode) && (SRC_MODE_PAL != enMode))
	{
        *pPtrPreData = MT_NULL;
        *pPtrPpreData = MT_NULL;
		return MT_SUCCESS;
	}

    if (pstSrc->pstTarget_1->next != &(pstSrc->stFulSrcList))
    {
        pstCurData = VPSS_SRC_GetData(pstSrc->pstTarget_1->next);
        list_for_each_safe(pos, n, &(pstSrc->stFulSrcList))
        {
            pstData = VPSS_SRC_GetData(pos);
            pstDataArray[u32Cnt] = pstData;
            u32Cnt++;
            if (pstData == pstCurData)
                break;
        }
    }
    else
    {
        *pPtrPreData = MT_NULL;
        *pPtrPpreData = MT_NULL;
		return MT_FAILURE;
    }

	if(SRC_MODE_NTSC == enMode)
	{
		if(5 <= u32Cnt)
		{
			*pPtrPreData = pstDataArray[u32Cnt-1-2];
			*pPtrPpreData = pstDataArray[u32Cnt-1-4];
		}
		else
		{
	        *pPtrPreData = MT_NULL;
		    *pPtrPpreData = MT_NULL;
		}
	}
	else
	{
		if(9 <= u32Cnt)
		{
			*pPtrPreData = pstDataArray[u32Cnt-1-4];
			*pPtrPpreData = pstDataArray[u32Cnt-1-8];
		}
		else
		{
	        *pPtrPreData = MT_NULL;
		    *pPtrPpreData = MT_NULL;
		}
	}


    return MT_SUCCESS;
}

mt_s32 VPSS_SRC_CompleteImage(VPSS_SRC_S* pstSrc,mt_void *arg)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VPSS_CHECK_NULL(pstSrc);
    VPSS_CHECK_NULL(pstSrc->pfnRlsImage);

    if(!pstSrc->bInit)
    {
        VPSS_INFO("VPSS_SRC_CompleteImage Delint.\n");
        return MT_FAILURE;
    }

    if (VPSS_SRC_IsEmpty(pstSrc))
    {
        VPSS_ERROR("Srclist is Empty\n");
        return MT_FAILURE;
    }

    pstSrc->u32CompleteSrcCount++;
    pstSrc->pstTarget_1 = pstSrc->pstTarget_1->next;

    if (!VPSS_SRC_IsUndoDataLeft(pstSrc))
    {
        if (VPSS_SRC_IsFull(pstSrc))
        {
            s32Ret = VPSS_SRC_RlsDoneFullNode(pstSrc);
        }
    }

    if (MT_FAILURE == s32Ret)
    {
        VPSS_ERROR("CompleteImage Failed\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


mt_s32 VPSS_SRCIN_Init(VPSS_SRCIN_S *pSrcIn)
{
    mt_u32 i;

    VPSS_CHECK_NULL(pSrcIn);

    if(pSrcIn->bInit)
    {
        VPSS_INFO("SRCIN has been Inited\n");
        return MT_SUCCESS;
    }

    VPSS_OSAL_InitSpin(&pSrcIn->stSrcInLock);

    INIT_LIST_HEAD(&pSrcIn->stBusyList);
    INIT_LIST_HEAD(&pSrcIn->stFreeList);
    INIT_LIST_HEAD(&pSrcIn->stReleaseList);

    /* 空闲buffer初始化时空闲buffer全部插入到free队列 */
    for (i = 0; i < SRCIN_NODE_NUM; i++)
    {
        list_add_tail(&pSrcIn->astSrcNode[i].node, &pSrcIn->stFreeList);
    }

    pSrcIn->bInit = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 VPSS_SRCIN_DeInit(VPSS_SRCIN_S *pSrcIn)
{
    VPSS_CHECK_NULL(pSrcIn);

    if(!pSrcIn->bInit)
    {
        VPSS_INFO("SRCIN has been DeInited\n");
        return MT_SUCCESS;
    }

    INIT_LIST_HEAD(&pSrcIn->stBusyList);
    INIT_LIST_HEAD(&pSrcIn->stFreeList);
    INIT_LIST_HEAD(&pSrcIn->stReleaseList);

    pSrcIn->bInit = MT_FALSE;

    return MT_SUCCESS;
}

mt_s32 VPSS_SRCIN_SendImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData)
{
    unsigned long u32Flag;
    VPSS_SRC_NODE_S *pstDateTmp;
    LIST *pstListTmp;

    VPSS_CHECK_NULL(pSrcIn);
    VPSS_CHECK_NULL(pstData);

    if(!pSrcIn->bInit)
    {
        VPSS_INFO("SRCIN has been DeInited\n");
        return MT_FAILURE;
    }

    VPSS_OSAL_DownSpin(&pSrcIn->stSrcInLock, &u32Flag);

    if(MT_TRUE == list_empty(&pSrcIn->stFreeList))
    {
        VPSS_ERROR("SRCIN No Node to Fill\n");
        VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);
        return MT_FAILURE;
    }

	pstListTmp = pSrcIn->stFreeList.next;
	list_del(pstListTmp);
	pstDateTmp = (VPSS_SRC_NODE_S *)list_entry(pstListTmp, VPSS_SRC_NODE_S, node);
    memcpy(&pstDateTmp->stSrcData, pstData, sizeof(VPSS_SRC_DATA_S));
    list_add_tail(&pstDateTmp->node, &pSrcIn->stBusyList);

    VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);

    return MT_SUCCESS;

}

mt_s32 VPSS_SRCIN_CallImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData)
{
    unsigned long u32Flag;
    VPSS_SRC_NODE_S *pstDateTmp;
    LIST *pstListTmp;

    VPSS_CHECK_NULL(pSrcIn);
    VPSS_CHECK_NULL(pstData);

    if(!pSrcIn->bInit)
    {
        VPSS_INFO("SRCIN has been DeInited\n");
        return MT_FAILURE;
    }

    VPSS_OSAL_DownSpin(&pSrcIn->stSrcInLock, &u32Flag);

    if(MT_TRUE == list_empty(&pSrcIn->stReleaseList))
    {
        VPSS_INFO("SRCIN No Node to Call\n");
        VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);
        return MT_FAILURE;
    }

    pstListTmp = pSrcIn->stReleaseList.next;
	list_del(pstListTmp);
	pstDateTmp = (VPSS_SRC_NODE_S *)list_entry(pstListTmp, VPSS_SRC_NODE_S, node);
    memcpy(pstData, &pstDateTmp->stSrcData, sizeof(VPSS_SRC_DATA_S));
    list_add_tail(&pstDateTmp->node, &pSrcIn->stFreeList);

    VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);

    return MT_SUCCESS;

}

mt_s32 VPSS_SRCIN_GetImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData)
{
    unsigned long u32Flag;
    VPSS_SRC_NODE_S *pstDateTmp;
    LIST *pstListTmp;

    VPSS_CHECK_NULL(pSrcIn);
    VPSS_CHECK_NULL(pstData);

    if(!pSrcIn->bInit)
    {
        VPSS_INFO("SRCIN has been DeInited\n");
        return MT_FAILURE;
    }

    VPSS_OSAL_DownSpin(&pSrcIn->stSrcInLock, &u32Flag);

    if(MT_TRUE == list_empty(&pSrcIn->stBusyList))
    {
        VPSS_INFO("SRCIN No Node to Get\n");
        VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);
        return MT_FAILURE;
    }

    pstListTmp = pSrcIn->stBusyList.next;
    list_del(pstListTmp);
    pstDateTmp = (VPSS_SRC_NODE_S *)list_entry(pstListTmp, VPSS_SRC_NODE_S, node);
    memcpy(pstData, &pstDateTmp->stSrcData, sizeof(VPSS_SRC_DATA_S));
    //list_add_tail(&pstDateTmp->node, &pSrcIn->stFreeList);
    pSrcIn->pstNodeUsed = pstDateTmp;

    VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);

    return MT_SUCCESS;

}

mt_s32 VPSS_SRCIN_RelImage(VPSS_SRCIN_S *pSrcIn, VPSS_SRC_DATA_S *pstData)
{
    unsigned long u32Flag;
    VPSS_SRC_NODE_S *pstDateTmp;

    VPSS_CHECK_NULL(pSrcIn);
    VPSS_CHECK_NULL(pstData);
    VPSS_CHECK_NULL(pSrcIn->pstNodeUsed);

    if(!pSrcIn->bInit)
    {
        VPSS_INFO("SRCIN has been DeInited\n");
        return MT_FAILURE;
    }

    VPSS_OSAL_DownSpin(&pSrcIn->stSrcInLock, &u32Flag);

    pstDateTmp = pSrcIn->pstNodeUsed;

    if(pstDateTmp->stSrcData.u32FrameIndex != pstData->u32FrameIndex)
    {
        VPSS_ERROR("this Node is invaild\n");
        VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);
        return MT_FAILURE;
    }

    list_add_tail(&pstDateTmp->node, &pSrcIn->stReleaseList);

    VPSS_OSAL_UpSpin(&pSrcIn->stSrcInLock, &u32Flag);

    return MT_SUCCESS;
}


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
