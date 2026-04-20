/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_fb.h"
#include "mt_drv_stat.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif
mt_s32 VPSS_FB_Init(VPSS_FB_INFO_S *pstFrameList,
                MT_DRV_VPSS_BUFLIST_CFG_S *pstBufListCfg)
{
    mt_u32 u32Count;
    mt_s32 s32Ret;
    VPSS_FB_NODE_S* pstNode;
    VPSS_BUFFER_S* pstBuf;
    mmz_buffer_s* pstMMZBuf;
    MT_DRV_VPSS_BUFLIST_CFG_S* pstBufCfg;

    pstBufCfg = &( pstFrameList->stBufListCfg );

    memcpy( pstBufCfg, pstBufListCfg,
            sizeof( MT_DRV_VPSS_BUFLIST_CFG_S ) );

    INIT_LIST_HEAD( &( pstFrameList->stEmptyFrmList ) );
    INIT_LIST_HEAD( &( pstFrameList->stFulFrmList ) );
    INIT_LIST_HEAD( &( pstFrameList->stExtFrmList ) );


    VPSS_OSAL_InitSpin(&(pstFrameList->stFulBufSpin ));
    VPSS_OSAL_InitSpin(&(pstFrameList->stEmptyBufSpin));
    VPSS_OSAL_InitSpin(&(pstFrameList->stExtBufSpin));

    pstFrameList->u32ExtCnt = 0;
    pstFrameList->u32ExtNumb = 0;

    pstFrameList->u32GetTotal = 0;
    pstFrameList->u32GetSuccess = 0;
    pstFrameList->u32RelTotal = 0;
    pstFrameList->u32RelSuccess= 0;
    pstFrameList->u32ListFul = 0;
    pstFrameList->ulStart = jiffies;
    pstFrameList->u32GetHZ = 0;
    pstFrameList->u32GetLast = 0;

    for ( u32Count = 0; u32Count < pstBufListCfg->u32BufNumber; u32Count ++ )
    {
        pstNode = ( VPSS_FB_NODE_S* )VPSS_VMALLOC( sizeof( VPSS_FB_NODE_S ));
        if (pstNode != MT_NULL)
        {
            memset( &( pstNode->stOutFrame ), 0, sizeof( MT_DRV_VIDEO_FRAME_S ) );
        	pstBuf = &( pstNode->stBuffer );

        	pstBuf->u32Stride = 0;

        	pstMMZBuf = &( pstBuf->stMMZBuf );

       	 	switch(pstBufCfg->eBufType)
        	{
            	case MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE:
                    pstMMZBuf->u32StartPhyAddr = 0;
                    pstMMZBuf->u32StartVirAddr = 0;
                    pstMMZBuf->u32Size = 0;
    				s32Ret = MT_SUCCESS;
                    break;
                case MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE:
                    pstMMZBuf->u32StartPhyAddr = 0;
                    pstMMZBuf->u32StartVirAddr = 0;
                    pstMMZBuf->u32Size = pstBufListCfg->u32BufSize;
                    s32Ret = MT_SUCCESS;
                    break;
                case MT_DRV_VPSS_BUF_USER_ALLOC_VPSS_MANAGE:
                    VPSS_FATAL("Buffer Type MT_DRV_VPSS_BUF_USER_ALLOC_VPSS_MANAGE can't support now.\n");
                    s32Ret = MT_FAILURE;
                    #if 0
                    pstMMZBuf->u32StartPhyAddr = pstBufListCfg->u32BufPhyAddr[u32Count];
                    pstMMZBuf->u32StartVirAddr = 0;
                    pstMMZBuf->u32Size = pstBufListCfg->u32BufSize;
                    s32Ret = MT_SUCCESS;
                    #endif
                    break;
                default:
                    VPSS_FATAL("Invalid Buffer Type.\n");
                    s32Ret = MT_FAILURE;
                    break;
        	}
        }
		else
		{
			VPSS_FATAL("Vpss Fb Vmalloc Fail\n");
			s32Ret = MT_FAILURE;
		}

        if ( MT_SUCCESS != s32Ret )
        {
			goto FB_Init_Failed;
        }

        list_add_tail( &( pstNode->node ), &( pstFrameList->stEmptyFrmList ) );

    }

    pstFrameList->pstTarget_1 = &( pstFrameList->stFulFrmList );
    return MT_SUCCESS;

FB_Init_Failed:
    if (pstNode != MT_NULL)
	{
    	VPSS_VFREE(pstNode);
	}
	VPSS_FATAL( "Vpss Alloc Buffer failed,already alloc %d total %d.\n",
                u32Count,pstBufListCfg->u32BufNumber);
    VPSS_FB_DelInit(pstFrameList);
    return MT_FAILURE;

}
mt_s32 VPSS_FB_DelInit(VPSS_FB_INFO_S *pstFrameList)
{
    MT_DRV_VPSS_BUFLIST_CFG_S *pstBufCfg;
    VPSS_FB_NODE_S *pstTarget;
    LIST *pos, *n;
    mt_u32 u32DelCount;
    mmz_buffer_s *pstMMZ;
    pstBufCfg = &(pstFrameList->stBufListCfg);

    u32DelCount = 0;

    //VPSS_OSAL_DownSpin(&(pstFrameList->stEmptyBufSpin));
    list_for_each_safe(pos, n, &(pstFrameList->stEmptyFrmList))
    {
        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);

        pstMMZ = &(pstTarget->stBuffer.stMMZBuf);

		if (pstBufCfg->eBufType == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE)
		{
        	if (pstMMZ->u32StartPhyAddr != 0 || pstMMZ->u32StartVirAddr!= 0)
        	{
            	mt_drv_mmz_unmap_and_release(&(pstTarget->stBuffer.stMMZBuf));
        	}
		}
        list_del_init(pos);

        VPSS_VFREE(pstTarget);
        u32DelCount++;
    }
    //VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin));


    //VPSS_OSAL_DownSpin(&(pstFrameList->stFulBufSpin));
    list_for_each_safe(pos, n, &(pstFrameList->stFulFrmList))
    {
        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);

        pstMMZ = &(pstTarget->stBuffer.stMMZBuf);
        if (pstBufCfg->eBufType == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE)
		{
        	if (pstMMZ->u32StartPhyAddr != 0 || pstMMZ->u32StartVirAddr!= 0)
			{
            	mt_drv_mmz_unmap_and_release(&(pstTarget->stBuffer.stMMZBuf));
        	}
		}

        list_del_init(pos);

        VPSS_VFREE(pstTarget);
        u32DelCount++;
    }
    //VPSS_OSAL_UpSpin(&(pstFrameList->stFulBufSpin));


    //VPSS_OSAL_DownSpin(&(pstFrameList->stExtBufSpin));
    list_for_each_safe(pos, n, &(pstFrameList->stExtFrmList))
    {
        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);

        pstMMZ = &(pstTarget->stBuffer.stMMZBuf);
        if (pstBufCfg->eBufType == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE)
		{
        	if (pstMMZ->u32StartPhyAddr != 0 || pstMMZ->u32StartVirAddr!= 0)
			{
            	mt_drv_mmz_unmap_and_release(&(pstTarget->stBuffer.stMMZBuf));
        	}
		}

        list_del_init(pos);

        VPSS_VFREE(pstTarget);
        u32DelCount++;
    }
    //VPSS_OSAL_UpSpin(&(pstFrameList->stExtBufSpin));

    if (u32DelCount != pstBufCfg->u32BufNumber + pstFrameList->u32ExtCnt)
    {
        VPSS_FATAL("Vpss buffer destory error already delete %d total %d+%d(%d).\n",
                    u32DelCount,
                    pstBufCfg->u32BufNumber,
                    pstFrameList->u32ExtNumb,
                    pstFrameList->u32ExtCnt);
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_FB_GetFulFrmBuf(VPSS_FB_INFO_S *pstFrameList,MT_DRV_VIDEO_FRAME_S *pstFrame,mt_char* pchFile)
{
    VPSS_FB_NODE_S *pstFrmNode;
    MT_DRV_VIDEO_FRAME_S *pstFrm;
    LIST *pstNextNode;
    VPSS_FB_NODE_S *pstLeftNode;
    VPSS_FB_NODE_S *pstRightNode;
    unsigned long flags;
    mt_s32 s32Ret = MT_FAILURE;//Rock_hu


    pstFrameList->u32GetTotal++;

    if(jiffies - pstFrameList->ulStart >= HZ)
    {
        pstFrameList->ulStart = jiffies;
        pstFrameList->u32GetHZ = pstFrameList->u32GetTotal - pstFrameList->u32GetLast;
        pstFrameList->u32GetLast = pstFrameList->u32GetTotal;
    }
    VPSS_OSAL_DownSpin(&(pstFrameList->stFulBufSpin),&flags);

    pstNextNode = (pstFrameList->pstTarget_1)->next;

    if(pstNextNode != &(pstFrameList->stFulFrmList))
    {
        pstFrmNode = list_entry(pstNextNode, VPSS_FB_NODE_S, node);

        pstFrm = &(pstFrmNode->stOutFrame);

        if(pstFrm->eFrmType == MT_DRV_FT_NOT_STEREO)
        {
            memcpy(pstFrame,pstFrm,sizeof(MT_DRV_VIDEO_FRAME_S));

            pstFrameList->pstTarget_1 =  pstNextNode;
            pstFrameList->u32GetSuccess++;
			s32Ret = MT_SUCCESS;
        }
        else
        {
			if(pstNextNode->next != &(pstFrameList->stFulFrmList))
            {
            	pstLeftNode = list_entry(pstNextNode, VPSS_FB_NODE_S, node);
            	pstNextNode = pstNextNode->next;
            	pstRightNode = list_entry(pstNextNode, VPSS_FB_NODE_S, node);
            	pstFrmNode = list_entry(pstNextNode, VPSS_FB_NODE_S, node);

            	pstFrm = &(pstFrmNode->stOutFrame);

            	if (pstLeftNode->stOutFrame.u32FrameIndex
                	!= pstRightNode->stOutFrame.u32FrameIndex )
            	{
                	VPSS_FATAL("GetFulFrmBuf 3D Error.\n");
            	}

                /* 将左眼的输出地址拷贝到右眼 */
                memcpy(&pstFrm->stBufAddr[0], &(pstLeftNode->stOutFrame.stBufAddr[0]), sizeof(MT_DRV_VID_FRAME_ADDR_S));

            	memcpy(pstFrame,pstFrm,sizeof(MT_DRV_VIDEO_FRAME_S));

            	pstFrameList->pstTarget_1 =  pstNextNode;
				s32Ret = MT_SUCCESS;
				pstFrameList->u32GetSuccess++;
            }
            else
            {
                s32Ret = MT_FAILURE;
            }
        }
    }
    else
    {
        s32Ret = MT_FAILURE;
    }

    //printk("RRRR 9999 s32Ret = %x\n", s32Ret);

    VPSS_OSAL_UpSpin(&(pstFrameList->stFulBufSpin),&flags);

    return s32Ret;
}

mt_s32 VPSS_FB_RelFulFrmBuf(VPSS_FB_INFO_S *pstFrameList,MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    VPSS_FB_NODE_S *pstTarget = MT_NULL;
    VPSS_FB_NODE_S *pstRelTarget_1 = MT_NULL;
    VPSS_FB_NODE_S *pstRelTarget_2 = MT_NULL;
    mt_u32 u32RelCount = 0;
    VPSS_BUFFER_S *pstBuf;
    LIST *pos, *n;
    mt_s32 s32GetFrm = MT_FAILURE;
    mt_u32 u32Count = 0;
    unsigned long flags;

    pstFrameList->u32RelTotal++;

    //printk("RRRR 6666 %d\n", pstFrameList->u32RelTotal);//Rock_hu rlease image
    //printk("WWWW 3333 %x, %x, %d \n", pstFrame->stBufAddr[0].u32PhyAddr_Y, pstFrame->stBufAddr[1].u32PhyAddr_Y, pstFrame->u32FrameIndex);

    VPSS_OSAL_DownSpin(&(pstFrameList->stFulBufSpin),&flags);
    for (pos = (pstFrameList->stFulFrmList).next, n = pos->next;
        pos != &(pstFrameList->stFulFrmList) && pos != (pstFrameList->pstTarget_1)->next;
		pos = n, n = pos->next)
    {
        if ( u32Count >= DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER)
        {
            VPSS_FATAL("\n RelFulFrmBuf Error\n");
        }
        u32Count ++;
        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);

        pstBuf = &(pstTarget->stBuffer);

        if(pstBuf->stMMZBuf.u32StartPhyAddr
           == pstFrame->stBufAddr[0].u32PhyAddr_Y
            /*FOR 3D FRM*/
           || pstBuf->stMMZBuf.u32StartPhyAddr
           == pstFrame->stBufAddr[1].u32PhyAddr_Y)
        {
            //if(pstTarget->stOutFrame.u32FrameIndex != pstFrame->u32FrameIndex)//binxuan
            if(pstTarget->stOutFrame.slotInfo.filedInfoTop.slot_idx != pstFrame->slotInfo.filedInfoTop.slot_idx)//binxuan
            {
               VPSS_WARN("\nRel Error BufferId %d FrmId %d\n",
                    pstTarget->stOutFrame.u32FrameIndex,
                    pstFrame->u32FrameIndex);

				VPSS_ERROR("%s: Topfield slot_idx: target %d != rls %d, but PhyAddr_Y(%x %x) match!\n",__FUNCTION__,
						pstTarget->stOutFrame.slotInfo.filedInfoTop.slot_idx,
						pstFrame->slotInfo.filedInfoTop.slot_idx,
						pstFrame->stBufAddr[0].u32PhyAddr_Y,
						pstFrame->stBufAddr[1].u32PhyAddr_Y);
				WARN(1, "Topfield slot_idx: target %d != rls %d!",
						pstTarget->stOutFrame.slotInfo.filedInfoTop.slot_idx,
						pstFrame->slotInfo.filedInfoTop.slot_idx);

               s32GetFrm = MT_FAILURE;
               break;
            }


            if(pstFrameList->pstTarget_1 != pos)
            {

            }
            else
            {
                pstFrameList->pstTarget_1 = (pstFrameList->pstTarget_1)->prev;
            }
               //printk("[VPSS_FB_RelFulFrmBuf] mmzbuf1 %x, buf0 %x,buf1 %x,buf slot %d,frm slot %d\n",pstBuf->stMMZBuf.u32StartPhyAddr,
                //pstFrame->stBufAddr[0].u32PhyAddr_Y,pstFrame->stBufAddr[1].u32PhyAddr_Y,pstTarget->stOutFrame.slotInfo.filedInfoTop.slot_idx,pstFrame->slotInfo.filedInfoTop.slot_idx);


            list_del_init(pos);

            if(u32RelCount == 0)
            {
                pstRelTarget_1 = pstTarget;
                u32RelCount ++;
            }
            else if (u32RelCount == 1)
            {
                pstRelTarget_2 = pstTarget;
                u32RelCount ++;
            }
            else
            {
                VPSS_FATAL("Rel Error too many buffer\n");
            }

            s32GetFrm = MT_SUCCESS;
        }
    }
     VPSS_OSAL_UpSpin(&(pstFrameList->stFulBufSpin),&flags);
    if(s32GetFrm == MT_FAILURE)
    {
        VPSS_WARN("Can't Get RelFrm %d\n",pstFrame->u32FrameIndex);
    }
    else
    {
        pstFrameList->u32RelSuccess++;
        if(u32RelCount == 0)
        {
            VPSS_FATAL("Rel Error no buffer\n");
        }
        else if (u32RelCount == 1)
        {
            VPSS_FB_AddEmptyFrmBuf(pstFrameList,pstRelTarget_1,VPSS_FB_TYPE_NORMAL);
        }
        else if (u32RelCount == 2)
        {
            VPSS_FB_AddEmptyFrmBuf(pstFrameList,pstRelTarget_1,VPSS_FB_TYPE_NORMAL);
            VPSS_FB_AddEmptyFrmBuf(pstFrameList,pstRelTarget_2,VPSS_FB_TYPE_NORMAL);
        }
        else
        {
            VPSS_FATAL("Rel Error too many buffer\n");
        }

    }

    return s32GetFrm;
}

VPSS_FB_NODE_S * VPSS_FB_GetEmptyFrmBuf(VPSS_FB_INFO_S *pstFrameList,
                            mt_u32 u32Height,mt_u32 u32Width,
                            MT_DRV_PIX_FORMAT_E ePixFormat,MT_DRV_PIXEL_BITWIDTH_E  enOutBitWidth)
{
    mt_s32 s32Ret;
    VPSS_FB_NODE_S *pstTarget;
    LIST *pos, *n;
    MT_DRV_VPSS_BUFLIST_CFG_S* pstBufCfg;
    mmz_buffer_s *pstMMZ;
    unsigned long flags;
    pstBufCfg = &( pstFrameList->stBufListCfg );

    pstTarget = MT_NULL;

    VPSS_OSAL_DownSpin(&(pstFrameList->stEmptyBufSpin),&flags);
    list_for_each_safe(pos, n, &(pstFrameList->stEmptyFrmList))
    {
        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);
        list_del_init(pos);
        break;
    }
    VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin),&flags);
    if (pstTarget)
    {
        memset(&(pstTarget->stOutFrame), 0,
                sizeof(MT_DRV_VIDEO_FRAME_S));

        if (pstFrameList->stBufListCfg.eBufType == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE)
        {
            mt_u32 u32BufSize = 0;
            mt_u32 u32BufStride = 0;

            pstMMZ = &(pstTarget->stBuffer.stMMZBuf);

            VPSS_OSAL_CalBufSize(&u32BufSize, &u32BufStride,
                        u32Height, u32Width, ePixFormat,enOutBitWidth);

            if (pstMMZ->u32Size == 0
               || pstTarget->stBuffer.u32Stride == 0
               || u32BufSize != pstMMZ->u32Size
               || u32BufStride != pstTarget->stBuffer.u32Stride)
            {
                if (pstMMZ->u32StartPhyAddr != 0)
                {
                    mt_drv_mmz_unmap_and_release(pstMMZ);
                    pstMMZ->u32Size = 0;
                    pstTarget->stBuffer.u32Stride = 0;
                }

                s32Ret = mt_drv_mmz_alloc_and_map( "VPSS_FrmBuf", "VPSS",
                                            u32BufSize, 0,
                                            pstMMZ);
                if (s32Ret == MT_FAILURE)
                {
                    pstMMZ->u32Size = 0;
                    pstTarget->stBuffer.u32Stride = 0;
                    pstMMZ->u32StartPhyAddr = 0;
                    pstMMZ->u32StartVirAddr = 0;
                    VPSS_FB_AddEmptyFrmBuf(pstFrameList, pstTarget,VPSS_FB_TYPE_NORMAL);
                    VPSS_FATAL("Dynamic Alloc Buffer Failed.BufSize %#x\n",u32BufSize);
                    return MT_NULL;
                }
                pstTarget->stBuffer.u32Stride = u32BufStride;


            }
        }
        return pstTarget;
    }
    else
    {
        return MT_NULL;
    }
}
mt_s32 VPSS_FB_AddFulFrmBuf(VPSS_FB_INFO_S *pstFrameList,VPSS_FB_NODE_S *pstFBNode)
{
    unsigned long flags;
    VPSS_OSAL_DownSpin(&(pstFrameList->stFulBufSpin),&flags);
    list_add_tail(&(pstFBNode->node), &(pstFrameList->stFulFrmList));

    //printk("RRRR 4444 VPSS_FB_AddFulFrmBuf \n");//yihua
    VPSS_OSAL_UpSpin(&(pstFrameList->stFulBufSpin),&flags);
    if (pstFBNode->stOutFrame.bIsFirstIFrame)
    {
        mt_drv_stat_event(STAT_EVENT_VPSSOUTFRM, 0);
    }
    return MT_SUCCESS;
}
mt_s32 VPSS_FB_AddEmptyFrmBuf(VPSS_FB_INFO_S *pstFrameList,
                                VPSS_FB_NODE_S *pstFBNode,
                                VPSS_FB_TYPE_E enType)
{
    MT_BOOL bFree = MT_FALSE;
    unsigned long flags;

    //printk("RRRR 5555 VPSS_FB_AddEmptyFrmBuf \n");//yihua
    VPSS_OSAL_DownSpin(&(pstFrameList->stEmptyBufSpin),&flags);
    if (enType == VPSS_FB_TYPE_NORMAL)
    {
        if (pstFrameList->u32ExtCnt <= pstFrameList->u32ExtNumb)
        {
            list_add(&(pstFBNode->node), &(pstFrameList->stEmptyFrmList));
        }
        else
        {
            bFree = MT_TRUE;
            pstFrameList->u32ExtCnt--;
        }
    }
    else if(enType == VPSS_FB_TYPE_EXTERN)
    {
        list_add_tail(&(pstFBNode->node), &(pstFrameList->stEmptyFrmList));
        pstFrameList->u32ExtCnt++;
    }
    else
    {
        VPSS_VFREE(pstFBNode);
        VPSS_FATAL("Invalid VPSS Fb Type %d\n",enType);
    }
    VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin),&flags);

    if (bFree == MT_TRUE)
    {
        VPSS_OSAL_DownSpin(&(pstFrameList->stExtBufSpin),&flags);

        list_add_tail(&(pstFBNode->node), &(pstFrameList->stExtFrmList));
        VPSS_OSAL_UpSpin(&(pstFrameList->stExtBufSpin),&flags);
    }
    return MT_SUCCESS;

}

MT_BOOL VPSS_FB_CheckIsAvailable(VPSS_FB_INFO_S *pstFrameList)
{
    LIST* pstEmptyList;
    unsigned long flags;
    pstEmptyList = &(pstFrameList->stEmptyFrmList);
    VPSS_OSAL_DownSpin(&(pstFrameList->stEmptyBufSpin),&flags);
    if(pstEmptyList->next != pstEmptyList
      && pstEmptyList->next->next != pstEmptyList)
    {
        VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin),&flags);
        return MT_TRUE;
    }
    else
    {
        pstFrameList->u32ListFul++;
        VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin),&flags);
        return MT_FALSE;
    }
}

//仅释放stFulFrmList队列中未被取走的项,已被Display读取的项不会被Reset,
//仍需要Display调用VPSS_FB_RelFulFrmBuf()来释放.
mt_s32 VPSS_FB_Reset(VPSS_FB_INFO_S *pstFrameList)
{
    MT_DRV_VPSS_BUFLIST_CFG_S *pstBufCfg;
    VPSS_FB_NODE_S *pstTarget;
    VPSS_FB_NODE_S *pstRelTarget[DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER];
    LIST *pos, *n;
    mt_u32 u32Count;
    mt_u32 u32RelCount;
    unsigned long flags;
    pstBufCfg = &(pstFrameList->stBufListCfg);

    VPSS_OSAL_DownSpin(&(pstFrameList->stFulBufSpin),&flags);

	//Added for debug begin:
    for (pos = &(pstFrameList->stFulFrmList), n = pos->next;
        pos != (pstFrameList->pstTarget_1)->next;
		pos = n, n = pos->next)
    {
        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);

		VPSS_ERROR("%s: Error, slot %u not released yet!\n",__FUNCTION__,
				pstTarget->stOutFrame.slotInfo.filedInfoTop.slot_idx);
    }
	//Added for debug end.

    u32Count = 0;
    u32RelCount = 0;
    for (pos = (pstFrameList->pstTarget_1)->next, n = pos->next;
        pos != &(pstFrameList->stFulFrmList);
		pos = n, n = pos->next)
    {
        if ( u32Count >= DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER)
        {
            VPSS_FATAL("Reset Error\n");
        }

        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);
        list_del_init(pos);


        pstRelTarget[u32RelCount] = pstTarget;

        u32Count ++;
        u32RelCount++;
    }
    VPSS_OSAL_UpSpin(&(pstFrameList->stFulBufSpin),&flags);

    if ( u32RelCount >= DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER)
    {
       VPSS_FATAL("Reset Error\n");
    }

	if (u32RelCount > 0)
	{
    	VPSS_ERROR("\n%s: ERROR, VPSS reset fb queue is not empty! %u remained!\n",__FUNCTION__,u32RelCount);
    }

    for(u32Count = 0; u32Count < u32RelCount;u32Count ++)
    {
        VPSS_FB_AddEmptyFrmBuf(pstFrameList,pstRelTarget[u32Count],VPSS_FB_TYPE_NORMAL);
    }

    pstFrameList->u32GetTotal = 0 ;
    pstFrameList->u32GetSuccess = 0 ;
    pstFrameList->u32RelTotal = 0 ;
    pstFrameList->u32RelSuccess = 0 ;
    pstFrameList->u32GetHZ = 0 ;
    pstFrameList->u32GetLast = 0 ;

    return MT_SUCCESS;
}
mt_s32 VPSS_FB_GetState(VPSS_FB_INFO_S *pstFrameList,VPSS_FB_STATE_S *pstFbState)
{
    mt_u32 u32Count;
    VPSS_FB_NODE_S *pstFbNode;
    LIST *pos, *n;
    mt_u32 u32Total = 0;
    mt_u32 u32DoneFlag;
    unsigned long flags;


    VPSS_OSAL_DownSpin(&(pstFrameList->stFulBufSpin),&flags);
    if (pstFrameList->pstTarget_1 != &(pstFrameList->stFulFrmList))
    {
        pstFbState->u32Target_1 = (mt_u32)list_entry(pstFrameList->pstTarget_1, VPSS_FB_NODE_S, node);
        pstFbNode = list_entry(pstFrameList->pstTarget_1, VPSS_FB_NODE_S, node);
        pstFbState->u32OutRate = pstFbNode->stOutFrame.u32FrameRate;
    }
    else
    {
        pstFbState->u32Target_1 = (mt_u32)&(pstFrameList->stFulFrmList);
    }

    if (pstFrameList->pstTarget_1 == &(pstFrameList->stFulFrmList))
    {
        u32DoneFlag = 2;
    }
    else
    {
        u32DoneFlag = 1;
    }


    u32Count = 0;
    list_for_each_safe(pos, n, &(pstFrameList->stFulFrmList))
    {
        if (u32Count >= DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER)
        {
            VPSS_FATAL("Get FbList Error\n");
            break;
        }
        #if FB_DBG
        pstFbNode = list_entry(pos, VPSS_FB_NODE_S, node);

        pstFbState->u32FulList[u32Count] = (mt_u32)pstFbNode;

        pstFbState->u32List[u32Total][0] = pstFbNode->stOutFrame.u32FrameIndex;
        pstFbState->u32List[u32Total][1] = u32DoneFlag;

        if (pstFbState->u32Target_1 == (mt_u32)pstFbNode)
        {
            u32DoneFlag = 2;
        }
        #endif
        u32Count++;
        u32Total++;
    }
    VPSS_OSAL_UpSpin(&(pstFrameList->stFulBufSpin),&flags);
    pstFbState->u32FulListNumb = u32Count;

    u32Count = 0;
    VPSS_OSAL_DownSpin(&(pstFrameList->stEmptyBufSpin),&flags);
    list_for_each_safe(pos, n, &(pstFrameList->stEmptyFrmList))
    {
        if (u32Count >= DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER)
        {
            VPSS_FATAL("Get FbList Error\n");
            break;
        }
        #if FB_DBG
        pstFbNode = list_entry(pos, VPSS_FB_NODE_S, node);
        pstFbState->u32EmptyList[u32Count] = (mt_u32)pstFbNode;

        pstFbState->u32List[u32Total][0] = -1;
        pstFbState->u32List[u32Total][1] = 0;
        #endif

        u32Count++;
        u32Total++;
    }
    VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin),&flags);
    pstFbState->u32EmptyListNumb = u32Count;

    #if FB_DBG
    while(u32Total < DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER)
    {
        pstFbState->u32List[u32Total][0] = -1;
        pstFbState->u32List[u32Total][1] = 3;
        u32Total++;
    }
    #endif

    pstFbState->u32ExtListNumb = pstFrameList->u32ExtNumb;
    pstFbState->u32TotalNumb = pstFrameList->stBufListCfg.u32BufNumber;

    pstFbState->u32GetHZ = pstFrameList->u32GetHZ;
    pstFbState->u32GetTotal = pstFrameList->u32GetTotal;
    pstFbState->u32GetSuccess = pstFrameList->u32GetSuccess;

    pstFbState->u32RelTotal = pstFrameList->u32RelTotal;
    pstFbState->u32RelSuccess = pstFrameList->u32RelSuccess;

    pstFbState->u32ListFul = pstFrameList->u32ListFul;

    return MT_SUCCESS;
}


mt_s32 VPSS_FB_AllocExtBuffer(VPSS_FB_INFO_S *pstFrameList,mt_u32 u32ExtNumb)
{
    mt_u32 i;
    VPSS_FB_NODE_S* pstNode;
    VPSS_BUFFER_S* pstBuf;
    mmz_buffer_s* pstMMZBuf;
    MT_DRV_VPSS_BUFLIST_CFG_S* pstBufCfg;
    mt_s32 s32Ret;
    unsigned long flags;

    pstBufCfg = &( pstFrameList->stBufListCfg );
    if (u32ExtNumb != pstFrameList->stBufListCfg.u32BufNumber
        && u32ExtNumb != 0)
    {
        VPSS_FATAL("Alloc Extern 3D Buffer number %d Attr %d Failed\n",
            u32ExtNumb,pstFrameList->stBufListCfg.u32BufNumber);
        return MT_FAILURE;
    }

    /*Cnt > numb means:3D->2D buffer decrease,if numb != 0, it is an error*/
    if (pstFrameList->u32ExtCnt > u32ExtNumb)
    {
        if (u32ExtNumb != 0)
        {
            VPSS_FATAL("Release Extern 3D Buffer number %d Cnt %d Failed\n",
                u32ExtNumb,pstFrameList->u32ExtCnt);
        }
        VPSS_OSAL_DownSpin(&(pstFrameList->stEmptyBufSpin),&flags);
        pstFrameList->u32ExtNumb = u32ExtNumb;
        VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin),&flags);
        return MT_SUCCESS;
    }
    else
    {
        VPSS_OSAL_DownSpin(&(pstFrameList->stEmptyBufSpin),&flags);
        pstFrameList->u32ExtNumb = u32ExtNumb;
        VPSS_OSAL_UpSpin(&(pstFrameList->stEmptyBufSpin),&flags);
        for (i = pstFrameList->u32ExtCnt;i < u32ExtNumb;i ++)
        {

            pstNode = ( VPSS_FB_NODE_S* )VPSS_VMALLOC( sizeof( VPSS_FB_NODE_S ));
            if (pstNode != MT_NULL)
            {
                memset( &( pstNode->stOutFrame ), 0, sizeof( MT_DRV_VIDEO_FRAME_S ) );
                pstBuf = &( pstNode->stBuffer );

                pstBuf->u32Stride = 0;

                pstMMZBuf = &( pstBuf->stMMZBuf );
                pstMMZBuf->u32StartPhyAddr = 0;
                pstMMZBuf->u32StartVirAddr = 0;
                pstMMZBuf->u32Size = 0;

                VPSS_FB_AddEmptyFrmBuf(pstFrameList,pstNode,VPSS_FB_TYPE_EXTERN);

        		s32Ret = MT_SUCCESS;
            }
            else
            {

            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_FB_RlsExtBuffer(VPSS_FB_INFO_S *pstFrameList)
{
    MT_DRV_VPSS_BUFLIST_CFG_S *pstBufCfg;
    VPSS_FB_NODE_S *pstTarget;
    LIST *pos, *n;
    mmz_buffer_s *pstMMZ;
    mt_u32 u32RlsCnt;
    mt_u32 i;
    unsigned long flags;

    VPSS_FB_NODE_S *pstRlsNode[DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER] = {0};

    pstBufCfg = &(pstFrameList->stBufListCfg);

    VPSS_OSAL_DownSpin(&(pstFrameList->stExtBufSpin),&flags);
    u32RlsCnt= 0;
    list_for_each_safe(pos, n, &(pstFrameList->stExtFrmList))
    {
        pstTarget = list_entry(pos, VPSS_FB_NODE_S, node);

        pstRlsNode[u32RlsCnt] = pstTarget;
        list_del_init(pos);
        u32RlsCnt++;
    }
    VPSS_OSAL_UpSpin(&(pstFrameList->stExtBufSpin),&flags);

    for (i = 0; i < DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER; i++)
    {
        if (pstRlsNode[i] != MT_NULL)
        {
            pstTarget = pstRlsNode[i];
            pstMMZ = &(pstTarget->stBuffer.stMMZBuf);
            if (pstBufCfg->eBufType == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE)
    		{
            	if (pstMMZ->u32StartPhyAddr != 0 || pstMMZ->u32StartVirAddr != 0)
    			{
                	mt_drv_mmz_unmap_and_release(&(pstTarget->stBuffer.stMMZBuf));
            	}
            	else
            	{

            	}
    		}
            VPSS_VFREE(pstTarget);
        }
    }
    return MT_SUCCESS;
}
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
