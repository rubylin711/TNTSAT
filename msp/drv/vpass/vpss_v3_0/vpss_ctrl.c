/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_reg_vpss.h"
#include "vpss_ctrl.h"
#include "vpss_common.h"
#include "mt_drv_proc.h"
#include "mt_module_debug.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


mt_u8 *g_pAlgModeString[4] = {
    "off",
    "on",
    "auto",
    "butt",
};
mt_u8 *g_pInstState[3] = {
    "stop",
    "working",
    "butt",
};
mt_u8 *g_pProgDetectString[4] = {
    "P",
    "I",
    "auto",
    "butt",
};
mt_u8 *g_pRotationString[5] = {
    "00",
    "90",
    "180",
    "270",
    "butt",
};
mt_u8 *g_pDeiString[9] = {
    "off",
    "auto",
    "2 field",
    "3 field",
    "4 field",
    "5 field",
    "6 field",
    "7 field",
    "butt",
};

mt_u8 *g_pSrcMutualString[3] = {
    "src active",
    "vpss active",
    "butt",
};

mt_u8 *g_pPixString[8] = {
    "YCbCr420",
    "YCrCb420",
    "YCbCr411",
    "YCbCr422",
    "YCrCb422",
    "YCbCr422_2X1",
    "YCrCb422_2X1",
    "butt",
};

mt_u8 *g_pAspString[8] = {
    "Full",
    "LBOX",
    "PANSCAN",
    "COMBINED",
    "FULL_H",
    "FULL_V",
    "CUSTOMER",
    "butt",
};

mt_u8 *g_pSrcModuleString[10] = {
    "Vdec",
    "Unknow",
    "Unknow",
    "Unknow",
    "Unknow",
    "Unknow",
    "Unknow",
    "Vi",
    "Venc",
    "Unknow"
};

mt_u8 *g_pBufTypeString[3] = {
    "vpss",
    "usr",
    "unknow",
};
mt_u8 *g_pRotateString[5] = {
    "Rotation_00",
    "Rotation_90",
    "Rotation_180",
    "Rotation_270",
    "Rotation_butt",
};
mt_u8 *g_pCscString[20] = {
    "UNKNOWN",
    "DEFAULT",
    "BT601_YUV",
    "BT601_YUV",
    "BT601_RGB",
    "BT601_RGB",
    "NTSC1953",
    "BT470_SYSTEM_M",
    "BT470_SYSTEM_BG",
    "BT709_YUV",
    "BT709_YUV",
    "BT709_RGB",
    "BT709_RGB",
    "REC709",
    "SMPT170M",
    "SMPT240M",
    "BT878",
    "XVYCC",
    "JPEG",
    "BUTT",
};
static VPSS_CTRL_S g_stVpssCtrl[VPSS_IP_BUTT] =
{
    {
        .bIPVaild = MT_FALSE,
        .enIP     = VPSS_IP_0,
        .u32VpssIrqNum = VPSS0_IRQ_NUM,
        .pVpssIntService = VPSS0_CTRL_IntService,
        .isr_name = "VPSS0_ISR",
        .s32IsVPSSOpen = 0,
    },

    {
        .bIPVaild = MT_TRUE,
        .enIP     = VPSS_IP_1,
        .u32VpssIrqNum = VPSS1_IRQ_NUM,
        .pVpssIntService = VPSS1_CTRL_IntService,
        .isr_name = "VPSS1_ISR",
        .s32IsVPSSOpen = 0,
    }
};

mt_void VPSS_CTRL_InitInstList(VPSS_IP_E enIp)
{
    mt_u32 u32Count;
    VPSS_INST_CTRL_S *pstInsList;

    pstInsList = &(g_stVpssCtrl[enIp].stInstCtrlInfo);
    rwlock_init(&(pstInsList->stListLock));
    pstInsList->u32Target = 0;
    pstInsList->u32InstanceNum = 0;

    for(u32Count = 0; u32Count < VPSS_INSTANCE_MAX_NUMB; u32Count ++)
    {
        pstInsList->pstInstPool[u32Count] = MT_NULL;
    }

}

mt_s32 VPSS_CTRL_RegistISR(VPSS_IP_E enIp)
{
//FIXME: Montage HW has no HW VPSS module.
#if 0
    if (request_irq(g_stVpssCtrl[enIp].u32VpssIrqNum, g_stVpssCtrl[enIp].pVpssIntService,
                        0, g_stVpssCtrl[enIp].isr_name, &(g_stVpssCtrl[enIp].hVpssIRQ)))
    {
        VPSS_FATAL("VPSS%d registe IRQ failed!\n",(mt_u32)enIp);
        return MT_FAILURE;
    }
    else
#endif
    {
        return MT_SUCCESS;
    }
}

mt_s32 VPSS_CTRL_UnRegistISR(VPSS_IP_E enIp)
{
//FIXME: Montage HW has no HW VPSS module.
#if 0
    free_irq(g_stVpssCtrl[enIp].u32VpssIrqNum, &(g_stVpssCtrl[enIp].hVpssIRQ));
#endif
    return MT_SUCCESS;
}

mt_u32 VPSS_CTRL_GetDistributeIP(VPSS_IP_E *penVpssIp)
{
    mt_u32 u32MinInstCount = VPSS_INSTANCE_MAX_NUMB;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 i;

    for(i = 0; i < VPSS_IP_BUTT; i++)
    {
        if((MT_TRUE == g_stVpssCtrl[i].bIPVaild) && (u32MinInstCount > g_stVpssCtrl[i].stInstCtrlInfo.u32InstanceNum))
        {
            u32MinInstCount = g_stVpssCtrl[i].stInstCtrlInfo.u32InstanceNum;
            *penVpssIp = i;
            s32Ret = MT_SUCCESS;
        }
    }

    return s32Ret;
}


mt_s32 VPSS_CTRL_SyncDistributeIP(VPSS_INSTANCE_S *pstInstance,MT_DRV_VPSS_IPMODE_E enIpmode)
{
    mt_u32 i;
    VPSS_IP_E enDstVpssIp;
    VPSS_IP_E enSrcVpssIp;
    VPSS_INST_CTRL_S *pstInstCtrlInfo;
    unsigned long u32LockFlag;
    VPSS_INSTANCE_S *pstChgInst = MT_NULL;

    //step1:get instance real ip
    //step2:change instance to new ip
    //step2.1:if the new ip has instance ,change it together
    if(MT_DRV_VPSS_IPMODE_BUTT <= enIpmode)
    {
        VPSS_FATAL("Invalid Ipmode %d.",enIpmode);
        return MT_FAILURE;
    }

    if((MT_FALSE == g_stVpssCtrl[VPSS_IP_0].bIPVaild)
        || (MT_FALSE == g_stVpssCtrl[VPSS_IP_1].bIPVaild))
    {
        VPSS_ERROR("Can not change IP %d,single ctrl",enIpmode);
        return MT_FAILURE;
    }

    if(MT_DRV_VPSS_IPMODE_IP0 == enIpmode)
    {
        enDstVpssIp = VPSS_IP_0;
    }
    else
    {
        enDstVpssIp = VPSS_IP_1;
    }

    pstInstCtrlInfo = &(g_stVpssCtrl[enDstVpssIp].stInstCtrlInfo);
    if(enDstVpssIp == pstInstance->CtrlID)
    {
        return MT_SUCCESS;
    }

    enSrcVpssIp = pstInstance->CtrlID;

    //we need exchange instance to keep balance
    if(0 < pstInstCtrlInfo->u32InstanceNum)
	{
		pstInstCtrlInfo = &(g_stVpssCtrl[enDstVpssIp].stInstCtrlInfo);

		read_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);

		for(i = 0; i < VPSS_INSTANCE_MAX_NUMB; i++)
		{
			if(MT_NULL != pstInstCtrlInfo->pstInstPool[i])
			{
				pstChgInst = pstInstCtrlInfo->pstInstPool[i];
				pstInstCtrlInfo->pstInstPool[i] = pstInstance;
				pstInstance->CtrlID = enDstVpssIp;
				break;
			}
		}

		read_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

		if (MT_NULL == pstChgInst)
		{
			VPSS_ERROR("Can't get DstInst\n");
			return MT_FAILURE;
		}

		pstInstCtrlInfo = &(g_stVpssCtrl[enSrcVpssIp].stInstCtrlInfo);

		read_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);

		for(i = 0; i < VPSS_INSTANCE_MAX_NUMB; i++)
		{
			if(pstInstance == pstInstCtrlInfo->pstInstPool[i])
			{
				pstInstCtrlInfo->pstInstPool[i] = pstChgInst;
				pstChgInst->CtrlID = enSrcVpssIp;
				break;
			}
		}
		read_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

	}
    else
    {
        pstInstCtrlInfo = &(g_stVpssCtrl[enSrcVpssIp].stInstCtrlInfo);

        read_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);

        for(i = 0; i < VPSS_INSTANCE_MAX_NUMB; i++)
        {
            if(pstInstance == pstInstCtrlInfo->pstInstPool[i])
            {
                pstInstCtrlInfo->pstInstPool[i] = MT_NULL;
                pstInstCtrlInfo->u32InstanceNum--;
                break;
            }
        }

        read_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

        pstInstCtrlInfo = &(g_stVpssCtrl[enDstVpssIp].stInstCtrlInfo);

        read_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);

        for(i = 0; i < VPSS_INSTANCE_MAX_NUMB; i++)
        {
            if(MT_NULL == pstInstCtrlInfo->pstInstPool[i])
            {
                pstInstCtrlInfo->pstInstPool[i] = pstInstance;
                pstInstance->CtrlID = enDstVpssIp;
                pstInstCtrlInfo->u32InstanceNum++;
                break;
            }
        }

        read_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

    }

    return MT_SUCCESS;
}

VPSS_INSTANCE_S *VPSS_CTRL_GetServiceInstance(VPSS_IP_E enIp)
{
    mt_s32 s32LockRet;
    mt_s32 s32CheckRet;
    mt_u32 u32CycleTime;
    mt_u32 u32CurPos;
    unsigned long  u32LockFlag;
    VPSS_INST_CTRL_S  *pstInstCtrlInfo;
    VPSS_INSTANCE_S *pstInstance;

    pstInstCtrlInfo = &(g_stVpssCtrl[enIp].stInstCtrlInfo);
    u32CycleTime = 0;
    u32CurPos = pstInstCtrlInfo->u32Target;

    while(u32CycleTime < VPSS_INSTANCE_MAX_NUMB)
    {
        read_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);
        pstInstance = pstInstCtrlInfo->pstInstPool[u32CurPos];
        read_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

        if(pstInstance == MT_NULL)
        {
            u32CurPos = (u32CurPos + 1) % VPSS_INSTANCE_MAX_NUMB;
            u32CycleTime++;
            continue;
        }

        s32LockRet = MT_SUCCESS;//VPSS_OSAL_TryLock(&(pstInstance->stInstLock));
        //s32LockRet = VPSS_OSAL_TryLock(&(pstInstance->stInstLock));
        if (s32LockRet == MT_SUCCESS)
		{
			pstInstance->u32CheckCnt++;
			if (jiffies - pstInstance->u32LastCheckTime > HZ)
			{
				pstInstance->u32CheckRate = pstInstance->u32CheckCnt;
				pstInstance->u32CheckSucRate = pstInstance->u32CheckSucCnt;
				pstInstance->u32BufRate = pstInstance->u32BufCnt;
				pstInstance->u32BufSucRate = pstInstance->u32BufSucCnt;
				pstInstance->u32SrcRate = pstInstance->u32SrcCnt;
				pstInstance->u32SrcSucRate = pstInstance->u32SrcSucCnt;
				pstInstance->u32CheckCnt = 0;
				pstInstance->u32CheckSucCnt = 0;
				pstInstance->u32BufCnt = 0;
				pstInstance->u32BufSucCnt = 0;
				pstInstance->u32SrcCnt = 0;
				pstInstance->u32SrcSucCnt = 0;

				pstInstance->u32ImgRate
					= pstInstance->u32ImgCnt - pstInstance->u32ImgLastCnt;
				pstInstance->u32ImgSucRate
					= pstInstance->u32ImgSucCnt - pstInstance->u32ImgLastSucCnt;
				pstInstance->u32ImgLastCnt = pstInstance->u32ImgCnt;
				pstInstance->u32ImgLastSucCnt = pstInstance->u32ImgSucCnt;
				pstInstance->u32LastCheckTime = jiffies;
			}

			if (pstInstance->enState == INSTANCE_STATE_WORING)
			{
				(mt_void)VPSS_INST_SyncUsrCfg(pstInstance);

				s32CheckRet = VPSS_INST_CheckInstAvailable(pstInstance);
				if(s32CheckRet == MT_SUCCESS)
				{
					pstInstCtrlInfo->u32Target = (u32CurPos + 1) % VPSS_INSTANCE_MAX_NUMB;
					pstInstance->u32CheckSucCnt++;
					//VPSS_OSAL_UpLock(&(pstInstance->stInstLock));
					return pstInstance;
				}
				else
				{
					//VPSS_OSAL_UpLock(&(pstInstance->stInstLock));
				}
			}
		}

        u32CurPos = (u32CurPos + 1) % VPSS_INSTANCE_MAX_NUMB;
        u32CycleTime++;
    }

    return MT_NULL;
}

mt_s32 VPSS_CTRL_FixTask(VPSS_IP_E enIp, MT_DRV_BUF_ADDR_E enLR, VPSS_TASK_S *pstTask)
{
    mt_s32 i;
    mt_s32 s32Ret = MT_FAILURE;
    VPSS_HAL_INFO_S *pstHalInfo;
    VPSS_INSTANCE_S* pstInst;
    MT_DRV_VIDEO_FRAME_S *pstCur;
    VPSS_FB_NODE_S *pstFrmNode = MT_NULL;
    VPSS_IN_INTF_S stInIntf;

    pstHalInfo = &pstTask->stVpssHalInfo;
    pstInst = pstTask->pstInstance;

    pstHalInfo->pstPqCfg = &pstInst->stPqRegData;

    s32Ret = VPSS_IN_GetIntf(&(pstInst->stInEntity), &stInIntf);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get Interface Failed\n");
        return MT_FAILURE;
    }

    VPSS_CHECK_NULL(stInIntf.pfnGetProcessImage);

    /* INInfo */
    s32Ret = stInIntf.pfnGetProcessImage(&pstInst->stInEntity, &pstCur);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_FATAL("VPSS_SRC_GetProcessImage failed!\n");
        return MT_FAILURE;
    }

    if (pstHalInfo->enNodeType == VPSS_HAL_NODE_2D_5Field)
    {
        MT_PQ_IFMD_CALC_S  stIfmdInPara;
        MT_PQ_IFMD_PLAYBACK_S  stIfmdOutCfg = {0};
		MT_DRV_VIDEO_PRIVATE_S *pstPriv;

        VPSS_INST_UpdatePqInfo(pstInst,pstCur->u32Width,pstCur->u32Height);

        stIfmdInPara.u32HandleNo = pstInst->ID;
        stIfmdInPara.u32WidthY = pstCur->u32Width;
        stIfmdInPara.u32HeightY = pstCur->u32Height;
        stIfmdInPara.s32FieldOrder = !pstCur->bTopFieldFirst;
        if(pstCur->enFieldMode == MT_DRV_FIELD_TOP)
        {
           stIfmdInPara.s32FieldMode = 0;
        }
        else if(pstCur->enFieldMode == MT_DRV_FIELD_BOTTOM)
        {
            stIfmdInPara.s32FieldMode = 1;
        }
        else
        {
            VPSS_ERROR("Dei Error.\n");
        }
        pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)pstCur->u32Priv;

        stIfmdInPara.u32FrameRate = pstCur->u32FrameRate;
        stIfmdInPara.bPreInfo = MT_FALSE;

        stIfmdInPara.stVdecInfo.IsProgressiveFrm = pstPriv->ePictureMode;
        stIfmdInPara.stVdecInfo.IsProgressiveSeq = pstPriv->eSampleType;
        stIfmdInPara.stVdecInfo.RealFrmRate = 2500;

        //(mt_void)DRV_PQ_IfmdDect(&stIfmdInPara,&stIfmdOutCfg);//Rock_hu

        if (stIfmdOutCfg.s32FieldOrder != 2)
        {
            if(pstInst->stInEntity.stStreamInfo.u32RealTopFirst == MT_FALSE)
            {
                if(pstCur->enFieldMode == MT_DRV_FIELD_TOP)
                {
                    MT_BOOL bTopFirst;
                    bTopFirst = !stIfmdOutCfg.s32FieldOrder;

                    stInIntf.pfnGetInfo(&pstInst->stInEntity,
                            VPSS_IN_INFO_CORRECT_FIELD,
                            MT_DRV_BUF_ADDR_MAX,
                            &bTopFirst);
                }
            }
            else if(pstInst->stInEntity.stStreamInfo.u32RealTopFirst == MT_TRUE)
            {
                if(pstCur->enFieldMode == MT_DRV_FIELD_BOTTOM)
                {
                    MT_BOOL bTopFirst;
                    bTopFirst = !stIfmdOutCfg.s32FieldOrder;

                    stInIntf.pfnGetInfo(&pstInst->stInEntity,
                            VPSS_IN_INFO_CORRECT_FIELD,
                            MT_DRV_BUF_ADDR_MAX,
                            &bTopFirst);
                }
            }
            else
            {

            }
        }
    }


    if (pstHalInfo->enNodeType == VPSS_HAL_NODE_UHD_SPLIT_R
        || pstHalInfo->enNodeType == VPSS_HAL_NODE_UHD_SPLIT_L)
    {
        VPSS_INST_SetHalFrameInfo(pstCur,
                                  &pstHalInfo->stInInfo,
                                  MT_DRV_BUF_ADDR_LEFT);
    }
    else
    {
        VPSS_INST_SetHalFrameInfo(pstCur,
                                  &pstHalInfo->stInInfo,
                                  enLR);
        if (pstInst->bAlwaysFlushSrc == MT_TRUE)
        {
            pstHalInfo->stInInfo.u32TunnelAddr = pstCur->u32TunnelPhyAddr;
        }
        else
        {
            pstHalInfo->stInInfo.u32TunnelAddr = 0;
        }
    }

    if(pstHalInfo->stInInfo.u32Width <= 1920)
    {
        VPSS_RWZB_IMG_S stRwzbImage;
        #if 1
        /*RWZB*/
        if(pstHalInfo->stInInfo.enFieldMode == MT_DRV_FIELD_BOTTOM
           || pstHalInfo->stInInfo.bProgressive == MT_TRUE)
        {
            VPSS_RWZB_GetRwzbData(&(pstInst->stRwzbInfo), &pstHalInfo->stRwzbInfo);

        }

        stRwzbImage.bProgressive = pstHalInfo->stInInfo.bProgressive;
        stRwzbImage.enFieldMode = pstHalInfo->stInInfo.enFieldMode;
        stRwzbImage.u32Height = pstHalInfo->stInInfo.u32Height;
        stRwzbImage.u32Width = pstHalInfo->stInInfo.u32Width;

        VPSS_RWZB_GetRwzbInfo(&(pstInst->stRwzbInfo), &pstHalInfo->stRwzbInfo, &stRwzbImage);

        #endif
    }

    switch(pstHalInfo->enNodeType)
    {
        case VPSS_HAL_NODE_2D_FRAME:
		case VPSS_HAL_NODE_2D_Field:
        case VPSS_HAL_NODE_3D_FRAME_R:
            break;
        case VPSS_HAL_NODE_2D_5Field:
            stInIntf.pfnGetInfo(&(pstInst->stInEntity),
                                    VPSS_IN_INFO_FIELD,
                                    MT_DRV_BUF_ADDR_LEFT,
                                     (mt_void*)pstHalInfo->stFieldAddr);
            stInIntf.pfnGetInfo(&(pstInst->stInEntity),
                                    VPSS_IN_INFO_MT_ADDR,
                                    MT_DRV_BUF_ADDR_MAX,
                                     (mt_void*)&(pstHalInfo->stHisAddr));
            break;
        case VPSS_HAL_NODE_UHD:
        case VPSS_HAL_NODE_UHD_SPLIT_L:
        case VPSS_HAL_NODE_UHD_SPLIT_R:
            /* Do Nothing */
            break;
        default:
            VPSS_FATAL("Node Type(%x) is Not Surport,\n", pstHalInfo->enNodeType);
            return MT_FAILURE;
    }
    for(i=0; i<DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; i++)
    {
        if (pstHalInfo->enNodeType == VPSS_HAL_NODE_UHD_SPLIT_L
            || pstHalInfo->enNodeType == VPSS_HAL_NODE_UHD_SPLIT_R)
        {
            pstFrmNode = pstTask->pstFrmNode[i*2];
        }
        else
        {
            pstFrmNode = pstTask->pstFrmNode[i*2 + enLR];
        }

        if (pstFrmNode!= MT_NULL)
        {
            pstHalInfo->astPortInfo[i].bEnable = MT_TRUE;
			pstHalInfo->astPortInfo[i].bConfig = MT_FALSE;

            //如果开启了旋转，使用旋转BUFFER
            if ((MT_DRV_VPSS_ROTATION_90 == pstInst->stPort[i].enRotation)
                || (MT_DRV_VPSS_ROTATION_270 == pstInst->stPort[i].enRotation))
            {
                VPSS_INST_SetOutFrameInfo(pstInst, i,
                    &g_stVpssCtrl[enIp].stRoBuf[i], &pstFrmNode->stOutFrame, enLR);
            }
            else
            {
                VPSS_INST_SetOutFrameInfo(pstInst, i,
                    &pstFrmNode->stBuffer, &pstFrmNode->stOutFrame, enLR);
            }

            if (pstHalInfo->enNodeType == VPSS_HAL_NODE_UHD_SPLIT_R)
            {
                memcpy(&(pstFrmNode->stOutFrame.stBufAddr[0]),
                        &(pstFrmNode->stOutFrame.stBufAddr[1]),
                        sizeof(MT_DRV_VID_FRAME_ADDR_S));
            }

            VPSS_INST_SetHalFrameInfo(&pstFrmNode->stOutFrame,
                &pstHalInfo->astPortInfo[i].stOutInfo, enLR);
            VPSS_INST_GetInCrop(pstInst, i, &pstHalInfo->astPortInfo[i].stInCropRect);
            if (   pstHalInfo->stInInfo.u32Width == 704
                && (pstHalInfo->stInInfo.u32Height == 576 || pstHalInfo->stInInfo.u32Height == 480)
                && (pstHalInfo->stInInfo.u32Height == pstHalInfo->astPortInfo[i].stOutInfo.u32Height)
                && pstHalInfo->astPortInfo[i].stOutInfo.u32Width == 720)
            {
                pstHalInfo->astPortInfo[i].stVideoRect.s32Width =
                                    pstHalInfo->stInInfo.u32Width;
                pstHalInfo->astPortInfo[i].stVideoRect.s32Height =
                                    pstHalInfo->stInInfo.u32Height;
                pstHalInfo->astPortInfo[i].stVideoRect.s32X = 8;
                pstHalInfo->astPortInfo[i].stVideoRect.s32Y = 0;
            }
            else
            {
                VPSS_INST_GetVideoRect(pstInst, i, &pstHalInfo->astPortInfo[i].stInCropRect,
                    &pstHalInfo->astPortInfo[i].stVideoRect);
            }

            memcpy(&(pstFrmNode->stOutFrame.stLbxInfo),
                    &(pstHalInfo->astPortInfo[i].stVideoRect),
                    sizeof(mt_rect_s));

            VPSS_INST_GetRotate(pstInst,i,&pstHalInfo->astPortInfo[i],pstCur);
        }
        else
        {
            pstHalInfo->astPortInfo[i].bEnable = MT_FALSE;
        }

    }

    return MT_SUCCESS;

}
MT_BOOL VPSS_CTRL_CheckVirtualStart(VPSS_IP_E enIp, VPSS_TASK_S *pstTask)
{
	MT_BOOL bStart = MT_FALSE;

	mt_u32 u32Cnt;

    VPSS_HAL_INFO_S *pstHalInfo;
    VPSS_INSTANCE_S* pstInst;
	VPSS_HAL_PORT_INFO_S *pstPortInfo;

    pstHalInfo = &pstTask->stVpssHalInfo;
    pstInst = pstTask->pstInstance;

	for(u32Cnt = 0 ; u32Cnt < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Cnt ++)
	{
		pstPortInfo = &(pstHalInfo->astPortInfo[u32Cnt]);

		if (pstPortInfo->bEnable == MT_TRUE
			&& pstPortInfo->bConfig == MT_FALSE)
		{
			bStart = MT_TRUE;
		}
	}

	return bStart;
}
mt_s32 VPSS_CTRL_Start2DTask(VPSS_IP_E enIp, VPSS_TASK_S *pstTask)
{
    mt_s32 s32Ret = MT_FAILURE;
    VPSS_HAL_INFO_S *pstHalInfo;
    VPSS_INSTANCE_S* pstInst;

    pstHalInfo = &pstTask->stVpssHalInfo;
    pstInst = pstTask->pstInstance;

    pstHalInfo->enNodeType = VPSS_INST_Check2DNodeType(pstInst);

    s32Ret = VPSS_CTRL_FixTask(enIp, MT_DRV_BUF_ADDR_LEFT, pstTask);
    //MT_ASSERT(s32Ret != MT_FAILURE);

    if (pstHalInfo->enNodeType == VPSS_HAL_NODE_2D_Field)
    {
        pstInst->abNodeVaild[VPSS_HAL_TASK_NODE_2D_FIELD] = MT_TRUE;
        VPSS_HAL_SetNodeInfo(enIp, pstHalInfo, VPSS_HAL_TASK_NODE_2D_FIELD);

		if (VPSS_CTRL_CheckVirtualStart(enIp,pstTask))
		{
			pstInst->abNodeVaild[VPSS_HAL_TASK_NODE_2D_FIELD_VIRTUAL]
				= MT_TRUE;
			VPSS_HAL_SetNodeInfo(enIp, pstHalInfo, VPSS_HAL_TASK_NODE_2D_FIELD_VIRTUAL);
		}
    }
    else
    {
		pstInst->abNodeVaild[VPSS_HAL_TASK_NODE_2D] = MT_TRUE;
		VPSS_HAL_SetNodeInfo(enIp, pstHalInfo, VPSS_HAL_TASK_NODE_2D);
		if (VPSS_CTRL_CheckVirtualStart(enIp,pstTask))
		{
			pstInst->abNodeVaild[VPSS_HAL_TASK_NODE_2D_VIRTUAL]
				= MT_TRUE;
			VPSS_HAL_SetNodeInfo(enIp, pstHalInfo, VPSS_HAL_TASK_NODE_2D_VIRTUAL);
		}
    }

    return MT_SUCCESS;
}

MT_BOOL VPSS_CTRL_Check2DStart(VPSS_TASK_S *pstTask)
{
    mt_u32 i;

    for(i=0; i<DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; i++)
    {
        if (pstTask->pstFrmNode[i*2] != MT_NULL)
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}

mt_s32 VPSS_CTRL_Start3DTask(VPSS_IP_E enIp, VPSS_TASK_S *pstTask)
{
    mt_s32 s32Ret = MT_FAILURE;
    VPSS_HAL_INFO_S *pstHalInfo;
    VPSS_INSTANCE_S* pstInst;

    pstHalInfo = &pstTask->stVpssHalInfo;
    pstInst = pstTask->pstInstance;

    pstHalInfo->enNodeType = VPSS_INST_Check3DNodeType(pstInst);

    s32Ret = VPSS_CTRL_FixTask(enIp, MT_DRV_BUF_ADDR_RIGHT, pstTask);
    //MT_ASSERT(s32Ret != MT_FAILURE);

    pstInst->abNodeVaild[VPSS_HAL_TASK_NODE_3D_R] = MT_TRUE;
    VPSS_HAL_SetNodeInfo(enIp, pstHalInfo, VPSS_HAL_TASK_NODE_3D_R);

    return MT_SUCCESS;
}

MT_BOOL VPSS_CTRL_Check3DStart(VPSS_TASK_S *pstTask)
{
    mt_u32 i;

    for(i=0; i<DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; i++)
    {
        if (pstTask->pstFrmNode[i*2 + 1] != MT_NULL)
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}
MT_BOOL VPSS_CTRL_CheckRotateStart(VPSS_TASK_S *pstTask, mt_u32 PortId)
{
    VPSS_HAL_PORT_INFO_S  *pstHalPortInfo;

    pstHalPortInfo = &pstTask->stVpssHalInfo.astPortInfo[PortId];
    if((MT_DRV_VPSS_ROTATION_DISABLE == pstHalPortInfo->enRotation)
        ||(MT_DRV_VPSS_ROTATION_180 == pstHalPortInfo->enRotation)
        || (MT_FALSE == pstTask->pstInstance->stPort[PortId].bEnble))
    {
        return MT_FALSE;
    }

    return MT_TRUE;
}

mt_s32 VPSS_CTRL_StartRotateTask(VPSS_IP_E enIp, VPSS_TASK_S *pstTask, mt_u32 PortId)
{
    VPSS_HAL_INFO_S *pstHalInfo;
    VPSS_INSTANCE_S* pstInst;
    MT_DRV_VIDEO_FRAME_S *pstInputFrame;
    VPSS_FB_NODE_S *pstOutputInfo;

    pstHalInfo = &pstTask->stVpssHalInfo;
    pstInst = pstTask->pstInstance;

    //set rotation input frame info
    pstInputFrame = &(pstTask->pstFrmNode[PortId*2+MT_DRV_BUF_ADDR_LEFT]->stOutFrame);
    VPSS_INST_SetHalFrameInfo(pstInputFrame, &pstHalInfo->stInInfo, MT_DRV_BUF_ADDR_LEFT);

    //set rotation output frame info
    pstOutputInfo = pstTask->pstFrmNode[PortId*2+MT_DRV_BUF_ADDR_LEFT];
    VPSS_INST_SetRotationOutFrameInfo(pstInst, PortId,
                    &pstOutputInfo->stBuffer, &pstOutputInfo->stOutFrame, MT_DRV_BUF_ADDR_LEFT);

    memcpy(&(pstOutputInfo->stOutFrame.stLbxInfo),
            &(pstHalInfo->astPortInfo[PortId].stVideoRect),
            sizeof(mt_rect_s));

    VPSS_INST_SetHalFrameInfo(&pstOutputInfo->stOutFrame,
        &pstHalInfo->astPortInfo[PortId].stOutInfo, MT_DRV_BUF_ADDR_LEFT);


    //rotation Y
    pstHalInfo->enNodeType = VPSS_HAL_NODE_ROTATION_Y;
    pstInst->abNodeVaild[VPSS_HAL_TASK_NODE_P0_RO_Y + PortId * 2] = MT_TRUE;
    VPSS_HAL_SetNodeInfo(enIp, pstHalInfo, VPSS_HAL_TASK_NODE_P0_RO_Y + PortId * 2);

    //rotation C
    pstHalInfo->enNodeType = VPSS_HAL_NODE_ROTATION_C;
    pstInst->abNodeVaild[VPSS_HAL_TASK_NODE_P0_RO_C + PortId * 2] = MT_TRUE;
    VPSS_HAL_SetNodeInfo(enIp, pstHalInfo, VPSS_HAL_TASK_NODE_P0_RO_C + PortId * 2);

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_CreateTask(VPSS_IP_E enIp, VPSS_TASK_S *pstTask)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32StoreH;
    mt_u32 u32StoreW;
    mt_u32 u32Count;
    VPSS_PORT_S* pstPort;
    MT_DRV_VPSS_BUFLIST_CFG_S*  pstBufListCfg;
    VPSS_IN_INTF_S stInIntf = {0};
    MT_DRV_VIDEO_FRAME_S *pstImage;

    /*
        Traversal instance list to find a Available inst
        available means two requirement:
        1.one undo image
        2.at least two writting space
     */
    pstTask->pstInstance = VPSS_CTRL_GetServiceInstance(enIp);

    if (MT_NULL == pstTask->pstInstance)
    {
        return MT_FAILURE;
    }

    /*
        get the image info to  inform user out buffer size
    */
    (mt_void)VPSS_IN_GetIntf(&(pstTask->pstInstance->stInEntity), &stInIntf);

    VPSS_CHECK_NULL(stInIntf.pfnGetProcessImage);

    (mt_void)stInIntf.pfnGetProcessImage(&(pstTask->pstInstance->stInEntity),
                        &pstImage);

    //printk("\n pfnGetProcessImage frame cnt %d\n", pstImage->u32FrameNo);//yihua

    /***********config out buffer************************/

    for (u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        pstPort = &((pstTask->pstInstance)->stPort[u32Count]);
        pstBufListCfg = &(pstPort->stFrmInfo.stBufListCfg);
        //printk("config outbuf %x %x\n", pstPort->s32PortId, pstPort->bEnble);
        if (pstPort->s32PortId != VPSS_INVALID_HANDLE &&
            pstPort->bEnble == MT_TRUE)
        {
            if (pstPort->s32OutputHeight == 0 && pstPort->s32OutputWidth == 0)
            {
                mt_rect_s stInRect;
                VPSS_INST_GetInCrop(pstTask->pstInstance, u32Count, &stInRect);
                u32StoreW = (mt_u32)stInRect.s32Width;
                u32StoreH = (mt_u32)stInRect.s32Height;

                //u32StoreH = pstStreamInfo->u32StreamH;
                //u32StoreW = pstStreamInfo->u32StreamW;
            }
            else
            {
                u32StoreH = pstPort->s32OutputHeight;
                u32StoreW = pstPort->s32OutputWidth;
            }

            /*2D image -> 1 outFrame 1 buffer*/
            if (pstImage->eFrmType == MT_DRV_FT_NOT_STEREO
                || pstPort->b3Dsupport == MT_FALSE)
            {
                pstTask->pstFrmNode[u32Count * 2] =
                    VPSS_FB_GetEmptyFrmBuf(&(pstPort->stFrmInfo),
                                           u32StoreH, u32StoreW,
                                           pstPort->eFormat,pstPort->enOutBitWidth);
                pstTask->pstFrmNode[u32Count * 2 + 1] = MT_NULL;
            }
            /*3D image -> 1 outFrame 2 buffer*/
            else
            {
                pstTask->pstFrmNode[u32Count * 2] =
                    VPSS_FB_GetEmptyFrmBuf(&(pstPort->stFrmInfo),
                                           u32StoreH, u32StoreW,
                                           pstPort->eFormat,pstPort->enOutBitWidth);
                pstTask->pstFrmNode[u32Count * 2 + 1] =
                    VPSS_FB_GetEmptyFrmBuf(&(pstPort->stFrmInfo),
                                           u32StoreH, u32StoreW,
                                           pstPort->eFormat,pstPort->enOutBitWidth);
                if (pstTask->pstFrmNode[u32Count * 2] == MT_NULL
                    || pstTask->pstFrmNode[u32Count * 2 + 1] == MT_NULL)
                {

                    if (pstTask->pstFrmNode[u32Count * 2] != MT_NULL)
                    {
                        VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                                               pstTask->pstFrmNode[u32Count * 2],
                                               VPSS_FB_TYPE_NORMAL);
                    }

                    if (pstTask->pstFrmNode[u32Count * 2 + 1] != MT_NULL)
                    {
                        VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                                               pstTask->pstFrmNode[u32Count * 2 + 1],
                                               VPSS_FB_TYPE_NORMAL);
                    }
                    pstTask->pstFrmNode[u32Count * 2] = MT_NULL;
                    pstTask->pstFrmNode[u32Count * 2 + 1] = MT_NULL;
                }
            }

#if 1
            if(pstBufListCfg->eBufType == MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE)
            {
                /*************************/

                if(pstPort->eFormat == MT_DRV_PIX_FMT_NV12
                       || pstPort->eFormat == MT_DRV_PIX_FMT_NV21)
                {
                    pstBufListCfg->u32BufStride = MT_ALIGN_8BIT_YSTRIDE(u32StoreW);
                    pstBufListCfg->u32BufSize =
                                pstBufListCfg->u32BufStride * u32StoreH * 3 / 2;
                }
                else if(pstPort->eFormat == MT_DRV_PIX_FMT_NV16_2X1
                        || pstPort->eFormat == MT_DRV_PIX_FMT_NV61_2X1)
                {
                    pstBufListCfg->u32BufStride =  MT_ALIGN_8BIT_YSTRIDE(u32StoreW);
                    pstBufListCfg->u32BufSize =
                                pstBufListCfg->u32BufStride * u32StoreH * 2;
                }
                else
                {
                    VPSS_FATAL("Port %x OutFormat isn't supported.\n",pstPort->s32PortId);
                }
                /*************************/
                if(pstTask->pstFrmNode[u32Count*2] != MT_NULL)
                {
                    s32Ret = VPSS_INST_GetFrmBuffer(pstTask->pstInstance,pstPort->s32PortId,pstBufListCfg,
                            &(pstTask->pstFrmNode[u32Count*2]->stBuffer),u32StoreH,u32StoreW);
                    if (s32Ret != MT_SUCCESS)
                    {
                        VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                            pstTask->pstFrmNode[u32Count*2],
                            VPSS_FB_TYPE_NORMAL);
                        if (pstTask->pstFrmNode[u32Count*2+1] != MT_NULL)
                        {
                            VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                                pstTask->pstFrmNode[u32Count*2+1],
                                VPSS_FB_TYPE_NORMAL);
                        }
                        VPSS_OSAL_UpLock(&(pstTask->pstInstance->stInstLock));
                        return MT_FAILURE;
                    }
                }

                if(pstTask->pstFrmNode[u32Count*2+1] != MT_NULL)
                {
                    s32Ret = VPSS_INST_GetFrmBuffer(pstTask->pstInstance,pstPort->s32PortId,pstBufListCfg,
                            &(pstTask->pstFrmNode[u32Count*2+1]->stBuffer),u32StoreH,u32StoreW);
                    if(s32Ret != MT_SUCCESS)
                    {
                        VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                            pstTask->pstFrmNode[u32Count*2],
                            VPSS_FB_TYPE_NORMAL);
                        VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                            pstTask->pstFrmNode[u32Count*2+1],
                            VPSS_FB_TYPE_NORMAL);
                        VPSS_OSAL_UpLock(&(pstTask->pstInstance->stInstLock));
                        return MT_FAILURE;
                    }
                }
            }
            else
            {

            }
#endif
        }
        else
        {
            pstTask->pstFrmNode[u32Count * 2] = MT_NULL;
            pstTask->pstFrmNode[u32Count * 2 + 1] = MT_NULL;
        }
    }

    for (u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER * 2; u32Count++)
    {
        if (pstTask->pstFrmNode[u32Count] != MT_NULL)
        {
            s32Ret = MT_SUCCESS;
            break;
        }
    }


    return s32Ret;

}

mt_s32 VPSS_CTRL_StartTask(VPSS_IP_E enIp, VPSS_TASK_S *pstTask)
{
    mt_u32 i;
    MT_BOOL bStart2D = MT_TRUE;
    MT_BOOL bStart3D = MT_FALSE;
    mt_s32 s32Ret;

    /* 所有节点都置为MT_FALSE */
    for(i=0; i<VPSS_HAL_TASK_NODE_BUTT;i++)
    {
        pstTask->pstInstance->abNodeVaild[i] = MT_FALSE;
    }

    bStart2D = VPSS_CTRL_Check2DStart(pstTask);
    if(bStart2D)
    {
        s32Ret = VPSS_CTRL_Start2DTask(enIp, pstTask);
        if (MT_SUCCESS != s32Ret)
        {
            VPSS_ERROR("Start 2D Task Failed\n");
            return s32Ret;
        }
        //rotate
        for(i = 0; i < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; i++)
        {
            if(VPSS_CTRL_CheckRotateStart(pstTask,i))
            {
                VPSS_CTRL_StartRotateTask(enIp, pstTask, i);
            }
            else
            {
                //释放旋转内存，节省SDK空间占用
                if(0 != g_stVpssCtrl[enIp].stRoBuf[i].stMMZBuf.u32Size)
                {
                    (mt_void)mt_drv_mmz_unmap_and_release(&(g_stVpssCtrl[enIp].stRoBuf[i].stMMZBuf));
                    g_stVpssCtrl[enIp].stRoBuf[i].stMMZBuf.u32Size = 0;
                }
            }
        }
    }

    bStart3D = VPSS_CTRL_Check3DStart(pstTask);
    if(bStart3D)
    {
        (mt_void)VPSS_CTRL_Start3DTask(enIp, pstTask);
    }

    return VPSS_HAL_StartLogic(enIp, pstTask->pstInstance->abNodeVaild);
}

mt_s32 VPSS_CTRL_GetGlbMotionInfo(VPSS_TASK_S *pstTask)
{
    mt_s32 s32Ret = MT_FAILURE;
    #if 0
    MT_PQ_MOTION_INPUT_S stMotionInput;
    MT_PQ_TNR_MOTION_PARAM_IN_S stParamIn;
	mt_u32 u32WbcVirAddr;

    u32WbcVirAddr = pstTask->stVpssHalInfo.u32stt_w_vir_addr;

    if(MT_TRUE == pstTask->stVpssHalInfo.stInInfo.bProgressive)
    {
        stParamIn.u32HdlNo = pstTask->pstInstance->ID;
        stParamIn.u32Height = pstTask->pstInstance->stInEntity.stStreamInfo.u32StreamH;
		stParamIn.u32Width = pstTask->pstInstance->stInEntity.stStreamInfo.u32StreamW;
		stParamIn.pstMotionReg = (S_VPSSWB_REGS_TYPE *)u32WbcVirAddr;
        s32Ret = DRV_PQ_GetTnrGlobalMotion(&stParamIn,(MT_PQ_TNR_MOTION_RESULT_S *)(&pstTask->pstInstance->stGlbMotionRls));
    }
    else
    {
        stMotionInput.u32HandleNo = pstTask->pstInstance->ID;
        stMotionInput.u32Height = pstTask->pstInstance->stInEntity.stStreamInfo.u32StreamH;
		stMotionInput.u32Width = pstTask->pstInstance->stInEntity.stStreamInfo.u32StreamW;
		stMotionInput.pstMotionReg = (S_VPSSWB_REGS_TYPE *)u32WbcVirAddr;
        //s32Ret = DRV_PQ_GetDeiGlobalMotion(&stMotionInput,(MT_PQ_MOTION_INFO_S *)(&pstTask->pstInstance->stGlbMotionRls));//Rock_hu
    }

	if(MT_SUCCESS != s32Ret)
	{
		VPSS_FATAL("Get GlobalMotion Info from pq failed\n");
	}
	#endif
    return s32Ret;
}


mt_s32 VPSS_CTRL_GetDbInfo(VPSS_TASK_S *pstTask)
{
    mt_s32 s32Ret = MT_FAILURE;
    #if 0
	MT_PQ_DB_STR_PARAM_IN_S stParamIn;
	mt_u32 u32WbcVirAddr;

    u32WbcVirAddr = pstTask->stVpssHalInfo.u32stt_w_vir_addr;

	stParamIn.u8SCDStr = pstTask->pstInstance->stSCDRls.SCW_P1;  // use scd calc result
	stParamIn.u32HdlNo = pstTask->pstInstance->ID;
	stParamIn.pstReg = (S_VPSSWB_REGS_TYPE *)u32WbcVirAddr;

    if(MT_TRUE == pstTask->stVpssHalInfo.stInInfo.bProgressive)
	{
		stParamIn.eType = MT_PQ_DB_PROGRESSIVE;
	}
	else
	{
		stParamIn.eType = MT_PQ_DB_INTERLACE;
	}

	s32Ret = DRV_PQ_GetAdaptiveDBStrength(&stParamIn,&pstTask->pstInstance->stDBRls);


	if(MT_SUCCESS != s32Ret)
	{
		VPSS_FATAL("Get DB Info from pq failed\n");
	}
	#endif
    return s32Ret;

}

mt_s32 VPSS_CTRL_GetSCDInfo(VPSS_TASK_S *pstTask)
{
    #if 0
	SCDInput stCDInput;
	mt_u32 u32WbcVirAddr;


    u32WbcVirAddr = pstTask->stVpssHalInfo.u32stt_w_vir_addr;

	stCDInput.Width = pstTask->stVpssHalInfo.stInInfo.u32Width;
	stCDInput.Height = pstTask->stVpssHalInfo.stInInfo.u32Height;

	if(MT_TRUE == pstTask->stVpssHalInfo.stInInfo.bProgressive)
	{
		stCDInput.Field = MT_DRV_FIELD_ALL;
	}
	else
	{
		stCDInput.Field = pstTask->stVpssHalInfo.stInInfo.enFieldMode;
	}

	VPSS_HAL_GetSCDInfo(u32WbcVirAddr,pstTask->pstInstance->stSCDRls.s32SCHist_CF);

	SCDDetection(&stCDInput,&(pstTask->pstInstance->stSCDRls));
	#endif
	return MT_SUCCESS;

}


mt_s32 VPSS_CTRL_GetRwzbData(VPSS_IP_E enIP,VPSS_TASK_S *pstTask)
{
    #if 1
    VPSS_INSTANCE_S* pstInstance;
    mt_u32 u32Count;
    VPSS_RWZB_S *pstRwzb;

    pstInstance = pstTask->pstInstance;
    pstRwzb = &(pstInstance->stRwzbInfo);

    for(u32Count = 0; u32Count < 6 ; u32Count ++)
    {
       VPSS_HAL_GetDetPixel(enIP,u32Count,&(pstRwzb->u8RwzbData[u32Count][0]));
    }
    #endif

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_StoreDeiData(VPSS_INSTANCE_S *pstInstance)
{
    mt_u32 u32RegSize;
    mt_u32 u32BaseRegPhyAddr;
    mt_u32 u32BaseRegVirAddr;
    VPSS_REG_S *pstPhyReg;
#if 0
    (mt_void)VPSS_HAL_GetBaseRegAddr(VPSS_IP_0,
                &u32BaseRegPhyAddr,
                &u32BaseRegVirAddr);

    pstPhyReg = (VPSS_REG_S *)u32BaseRegVirAddr;

    u32RegSize = VPSS_REG_SIZE_CALC(VPSS_DIEGLBMTNBIN, VPSS_DIEMIDDLMTNSUM);

    memcpy((mt_void*)&(pstInstance->stPqRegData.VPSS_DIEGLBMTNBIN[0]),
            (mt_void*)&(pstPhyReg->VPSS_DIEGLBMTNBIN[0]),
            u32RegSize);

    u32RegSize = VPSS_REG_SIZE_CALC(VPSS_PDICHD, VPSS_PDICHD);

    memcpy((mt_void*)&(pstInstance->stPqRegData.VPSS_PDICHD),
            (mt_void*)&(pstPhyReg->VPSS_PDICHD),
            u32RegSize);

    u32RegSize = VPSS_REG_SIZE_CALC(VPSS_PDFRMITDIFF, VPSS_PDLASICNT34);

    memcpy((mt_void*)&(pstInstance->stPqRegData.VPSS_PDFRMITDIFF),
            (mt_void*)&(pstPhyReg->VPSS_PDFRMITDIFF),
            u32RegSize);
#else
(mt_void)VPSS_HAL_GetBaseRegAddr(VPSS_IP_0,
                &u32BaseRegPhyAddr,
                &u32BaseRegVirAddr);

    pstPhyReg = (VPSS_REG_S *)u32BaseRegVirAddr;

    u32RegSize = VPSS_REG_SIZE_CALC(VPSS_DIEGLBMTNBIN, VPSS_DIEMTNSUM4);

    memcpy((mt_void*)&(pstInstance->stPqRegData.VPSS_DIEGLBMTNBIN[0]),
            (mt_void*)&(pstPhyReg->VPSS_DIEGLBMTNBIN[0]),
            u32RegSize);

    u32RegSize = VPSS_REG_SIZE_CALC(VPSS_PDICHD, VPSS_PDICHD);

    memcpy((mt_void*)&(pstInstance->stPqRegData.VPSS_PDICHD),
            (mt_void*)&(pstPhyReg->VPSS_PDICHD),
            u32RegSize);

    u32RegSize = VPSS_REG_SIZE_CALC(VPSS_PDFRMITDIFF, VPSS_PDLASICNT341);

    memcpy((mt_void*)&(pstInstance->stPqRegData.VPSS_PDFRMITDIFF),
            (mt_void*)&(pstPhyReg->VPSS_PDFRMITDIFF),
            u32RegSize);
#endif
    return MT_SUCCESS;
}


mt_s32 VPSS_CTRL_CompleteTask(VPSS_IP_E enIp, VPSS_TASK_S *pstTask)
{
    mt_u32 u32Count;
    VPSS_PORT_S* pstPort;
    VPSS_INSTANCE_S* pstInstance;
    VPSS_FB_NODE_S* pstLeftFbNode;
    VPSS_FB_NODE_S* pstRightFbNode;
    MT_BOOL bDropped;
    MT_DRV_VIDEO_FRAME_S stTmpFrame;
    MT_DRV_VIDEO_PRIVATE_S* pstPriv;
    MT_DRV_VPSS_BUFFER_TYPE_E ePortType = MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE;
    MT_DRV_VIDEO_FRAME_S *pstCur;
    VPSS_IN_INTF_S stInIntf;
    mt_s32 s32Ret = MT_FAILURE;

    pstInstance = pstTask->pstInstance;

    s32Ret = VPSS_IN_GetIntf(&(pstInstance->stInEntity), &stInIntf);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get Interface Failed\n");
        return MT_FAILURE;
    }

    /*step 0.0:get  info from pq module*/
	for (u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);
        if ((MT_TRUE == pstPort->bEnble) &&
            ((MT_DRV_VPSS_ROTATION_90 == pstPort->enRotation)
            || (MT_DRV_VPSS_ROTATION_270 == pstPort->enRotation)))
        {
            //correct src info ,because rotation change it

            VPSS_CHECK_NULL(stInIntf.pfnGetProcessImage);

            s32Ret = stInIntf.pfnGetProcessImage(&pstInstance->stInEntity, &pstCur);
            if (MT_SUCCESS != s32Ret)
            {
                VPSS_FATAL("VPSS_SRC_GetProcessImage failed!\n");
                return MT_FAILURE;
            }

            pstTask->stVpssHalInfo.stInInfo.u32Width = pstCur->u32Width;
            pstTask->stVpssHalInfo.stInInfo.u32Height = pstCur->u32Height;
            pstTask->stVpssHalInfo.stInInfo.bProgressive = pstCur->bProgressive;
            pstTask->stVpssHalInfo.stInInfo.enFieldMode = pstCur->enFieldMode;
            break;
        }
	}
	   //VPSS_CTRL_GetRwzbData(enIp,pstTask);//Rock_hu

	   /*get logic dei date*/
    //VPSS_CTRL_StoreDeiData(g_stVpssCtrl[enIp].stTask.pstInstance);//Rock_hu

    /*step 1.0 :release done image*/
    VPSS_INST_CompleteImage(pstInstance);

    /*step 2:add outframe to outPort list*/
    for (u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);
        if (pstPort->s32PortId == VPSS_INVALID_HANDLE)
        {
            continue;
        }
        pstLeftFbNode = pstTask->pstFrmNode[u32Count * 2];
        pstRightFbNode = pstTask->pstFrmNode[u32Count * 2 + 1];

        bDropped = MT_FALSE;
        if (pstLeftFbNode != MT_NULL || pstRightFbNode != MT_NULL)
        {
            //mt_rect_s stLbxTmp = {0};
            pstPort->u32OutCount ++;
            bDropped = VPSS_INST_CheckIsDropped(pstInstance,
                                                pstPort->u32MaxFrameRate,
                                                pstPort->u32OutCount);
            #if 0
            /* 加入LBX统计信息 */
            /* 1,获取LBX信息 */
            /* 2,添加到帧结构体中 */
            VPSS_INST_GetLbxInfo(pstInstance, u32Count, &stLbxTmp);

            if (pstLeftFbNode != MT_NULL)
            {
                memcpy(&pstLeftFbNode->stOutFrame.stLbxInfo, &stLbxTmp, sizeof(mt_rect_s));
            }

            if (pstRightFbNode != MT_NULL)
            {
                memcpy(&pstRightFbNode->stOutFrame.stLbxInfo, &stLbxTmp, sizeof(mt_rect_s));
            }
            #endif

            //if the last frame  ,it can't be dropped
            if (pstLeftFbNode != MT_NULL)
            {
                memcpy(&stTmpFrame, &(pstLeftFbNode->stOutFrame), sizeof(MT_DRV_VIDEO_FRAME_S));
                pstPriv = (MT_DRV_VIDEO_PRIVATE_S*) & (stTmpFrame.u32Priv[0]);
                if (pstPriv->u32LastFlag == DEF_MT_DRV_VPSS_LAST_FRAME_FLAG)
                {
                    bDropped = MT_FALSE;
                }
            }
        }

        if (pstLeftFbNode != MT_NULL )
        {
            memcpy(&stTmpFrame,
                   &(pstLeftFbNode->stOutFrame),
                   sizeof(MT_DRV_VIDEO_FRAME_S));
                 //printk("ebuf type 0x%x \n", pstPort->stFrmInfo.stBufListCfg.eBufType);//yihua

            if (pstPort->stFrmInfo.stBufListCfg.eBufType == MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE)
            {
                /*Revise the Port Type to MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE
                    *in order to decide whether report newframe
                    */
                ePortType = MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE;

                if (MT_FALSE == bDropped)
                {
                    VPSS_INST_ReportNewFrm(pstTask->pstInstance,
                                           pstPort->s32PortId,
                                           &(pstLeftFbNode->stOutFrame));
                }
                else
                {
                    VPSS_INST_RelFrmBuffer(pstTask->pstInstance, pstPort->s32PortId,
                                           &(pstPort->stFrmInfo.stBufListCfg),
                                           &(pstLeftFbNode->stBuffer.stMMZBuf));
                }
                VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                                       pstTask->pstFrmNode[u32Count],
                                       VPSS_FB_TYPE_NORMAL);
            }
            else
            {
                if (MT_FALSE == bDropped)
                {
                    pstPriv = (MT_DRV_VIDEO_PRIVATE_S*) & (pstLeftFbNode->stOutFrame.u32Priv[0]);
                    pstPriv->u32FrmCnt = pstPort->u32OutCount;
                    if ((pstInstance->stInEntity.stStreamInfo.u32InRate > pstPort->u32MaxFrameRate)
                        &&(pstPort->u32MaxFrameRate != 0))
                    {
                        pstLeftFbNode->stOutFrame.u32FrameRate
                            = pstInstance->stInEntity.stStreamInfo.u32InRate / 2 * 1000;
                    }

                    if (u32Count == 0)
                    {
                        mt_ld_event_s evt;
                        mt_u32 TmpTime = 0;
                        mt_drv_sys_gettimestampms(&TmpTime);
                        evt.evt_id = EVENT_VPSS_FRM_OUT;
                        evt.frame = pstLeftFbNode->stOutFrame.u32FrameIndex;
                        evt.handle = pstLeftFbNode->stOutFrame.hTunnelSrc;
                        evt.time = TmpTime;
                        mt_drv_ld_notify_event(&evt);
                    }

                    //printk("LLLL 6666 slot_index =%x \n", pstLeftFbNode->stOutFrame.slotInfo.filedInfoTop.slot_idx);//yihua
                    VPSS_FB_AddFulFrmBuf(&(pstPort->stFrmInfo), pstLeftFbNode);
                }
                else
                {
                    VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                                           pstLeftFbNode,
                                           VPSS_FB_TYPE_NORMAL);
                }
            }
        }

        if (pstRightFbNode != MT_NULL)
        {
            memcpy(&(pstRightFbNode->stOutFrame.stBufAddr[0]),
                   &(stTmpFrame.stBufAddr[0]),
                   sizeof(MT_DRV_VID_FRAME_ADDR_S));
            if (pstPort->stFrmInfo.stBufListCfg.eBufType == MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE)
            {
                if (MT_FALSE == bDropped)
                {
                    VPSS_INST_ReportNewFrm(pstTask->pstInstance,
                                           pstPort->s32PortId,
                                           &(pstRightFbNode->stOutFrame));
                }
                else
                {
                    VPSS_INST_RelFrmBuffer(pstTask->pstInstance, pstPort->s32PortId,
                                           &(pstPort->stFrmInfo.stBufListCfg),
                                           &(pstRightFbNode->stBuffer.stMMZBuf));
                }

                VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                                       pstTask->pstFrmNode[u32Count],
                                       VPSS_FB_TYPE_NORMAL);
            }
            else
            {

                if (MT_FALSE == bDropped)
                {
                    pstPriv = (MT_DRV_VIDEO_PRIVATE_S*) & (pstRightFbNode->stOutFrame.u32Priv[0]);
                    pstPriv->u32FrmCnt = pstPort->u32OutCount;
                    VPSS_FB_AddFulFrmBuf(&(pstPort->stFrmInfo), pstRightFbNode);
                }
                else
                {
                    VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                                           pstRightFbNode,
                                           VPSS_FB_TYPE_NORMAL);
                }
            }
        }
    }

    if (ePortType == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE)
    {
        if (pstInstance->pfUserCallBack != MT_NULL)
        {
            pstInstance->pfUserCallBack(pstInstance->hDst, VPSS_EVENT_NEW_FRAME, MT_NULL);
        }
        else
        {
            VPSS_FATAL("Can't report VPSS_EVENT_NEW_FRAME,pfUserCallBack is NULL");
        }
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_ClearTask(VPSS_IP_E enIp, VPSS_TASK_S *pstTask)
{
    mt_u32 u32Count;
    VPSS_PORT_S *pstPort;
    VPSS_FB_NODE_S *pstFbNode;
    VPSS_IN_INTF_S stInIntf;
    VPSS_INSTANCE_S *pstInst;

    mt_s32 s32Ret = MT_FAILURE;

    pstInst = pstTask->pstInstance;


    s32Ret = VPSS_IN_GetIntf(&(pstInst->stInEntity), &stInIntf);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get Interface Failed\n");
        return MT_FAILURE;
    }

    //printk("FFFF VPSS_CTRL_ClearTask \n");

    //step1:release src buffer
    s32Ret = stInIntf.pfnCompleteImage(&(pstInst->stInEntity));
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_FATAL("pfnCompleteImage failed!\n");
        return MT_FAILURE;
    }

    //step2:release port buffer
    for(u32Count = 0;u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER;u32Count++)
    {
        pstPort = &((pstTask->pstInstance)->stPort[u32Count]);
        pstFbNode = pstTask->pstFrmNode[u32Count*2];
        if(pstFbNode != MT_NULL)
        {
            if(pstPort->stFrmInfo.stBufListCfg.eBufType == MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE)
            {
                VPSS_INST_RelFrmBuffer(pstTask->pstInstance,pstPort->s32PortId,
                        &(pstPort->stFrmInfo.stBufListCfg),
                        &(pstTask->pstFrmNode[u32Count]->stBuffer.stMMZBuf));
            }
            VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                pstFbNode,
                VPSS_FB_TYPE_NORMAL);
        }

        pstFbNode = pstTask->pstFrmNode[u32Count*2 + 1];
        if(pstFbNode != MT_NULL)
        {

            if(pstPort->stFrmInfo.stBufListCfg.eBufType == MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE)
            {
                VPSS_INST_RelFrmBuffer(pstTask->pstInstance,pstPort->s32PortId,
                        &(pstPort->stFrmInfo.stBufListCfg),
                        &(pstTask->pstFrmNode[u32Count]->stBuffer.stMMZBuf));
            }
            VPSS_FB_AddEmptyFrmBuf(&(pstPort->stFrmInfo),
                            pstFbNode,
                            VPSS_FB_TYPE_NORMAL);
        }
    }

    //step3:reset vpss logic
    VPSS_REG_ReSetCRG(pstTask->pstInstance->CtrlID);

    return MT_SUCCESS;

}
extern mt_u32  mt_drv_timer_read_cnt(int id);
mt_s32 VPSS_CTRL_ThreadProc(mt_void* pArg)
{
    VPSS_IP_E enIp;
    mt_s32 s32CreateRet   = MT_FAILURE;
    mt_s32 s32StartRet    = MT_FAILURE;
    //unused
    //mt_s32 s32WaitRet     = MT_FAILURE;

    mt_u32 u32NowTime = 0;
    mt_u32 t1, t2;
    VPSS_CTRL_S *pstVpssCtrl = MT_NULL;

    enIp = (VPSS_IP_E)pArg;
    pstVpssCtrl = &(g_stVpssCtrl[enIp]);

    pstVpssCtrl->s32ThreadPos = 0;

    VPSS_OSAL_InitEvent(&(pstVpssCtrl->stTaskNext), EVENT_UNDO, EVENT_UNDO);
    VPSS_OSAL_InitEvent(&(pstVpssCtrl->stNewTask), EVENT_UNDO, EVENT_UNDO);

    pstVpssCtrl->stTask.u32LastTotal = 0;
    pstVpssCtrl->stTask.u32SuccessTotal = 0;
    pstVpssCtrl->stTask.u32Create = 0;
    pstVpssCtrl->stTask.u32Fail = 0;
    pstVpssCtrl->stTask.u32TimeOut= 0;

    while(!kthread_should_stop())//Rock_Hu vpss 处理流程
    {
        //printk("mi enter %d\n", pstVpssCtrl->u32ThreadSleep);

		//unused
        //mt_s32 s32Ret = MT_FAILURE;
        pstVpssCtrl->stTask.stState = TASK_STATE_READY;

        if(pstVpssCtrl->u32ThreadSleep == 1)
        {
            goto VpssThreadIdle;
        }

        pstVpssCtrl->s32ThreadPos = 1;

        pstVpssCtrl->stTask.u32Create++;

        s32CreateRet =  VPSS_CTRL_CreateTask(enIp, &(pstVpssCtrl->stTask));

        //if(0 == s32CreateRet)
        //    VPSS_INFO("...............CreateTask 0x%x\n", s32CreateRet);//yihua

        /* create success  running -> waitting */
        if(s32CreateRet == MT_SUCCESS)
        {
            pstVpssCtrl->s32ThreadPos = 2;

            //VPSS_HAL_SetClockEn(enIp, MT_TRUE);

            //VPSS_OSAL_ResetEvent(&(pstVpssCtrl->stTaskNext), EVENT_UNDO, EVENT_UNDO);
            s32StartRet = VPSS_CTRL_StartTask(enIp, &(pstVpssCtrl->stTask));
            if (s32StartRet == MT_SUCCESS)
            {
                //VPSS_INFO("...............StartTask\n");
                /*
                    start logic running, waitting for irq to wakeup thread
                    */
                pstVpssCtrl->stTask.stState = TASK_STATE_WAIT;

#if 0
                s32WaitRet = VPSS_OSAL_WaitEvent(&(pstVpssCtrl->stTaskNext), ms_to_ktime(1000));
                if (s32WaitRet == MT_SUCCESS)
                {
                    pstVpssCtrl->s32ThreadPos = 3;

                    VPSS_CTRL_CompleteTask(enIp, &(pstVpssCtrl->stTask));

                    if(jiffies - u32NowTime >= HZ)
                    {
                        u32NowTime = jiffies;
                        pstVpssCtrl->stTask.u32SucRate = pstVpssCtrl->stTask.u32SuccessTotal
                                                       - pstVpssCtrl->stTask.u32LastTotal;
                        pstVpssCtrl->stTask.u32LastTotal = pstVpssCtrl->stTask.u32SuccessTotal;
                    }

                    pstVpssCtrl->stTask.u32SuccessTotal ++;
                    msleep(1);
                }
                else
                {
                    VPSS_FATAL("...............Wait OutTime Faild\n");
                    pstVpssCtrl->s32ThreadPos = 4;
                    pstVpssCtrl->stTask.u32TimeOut++;

                    VPSS_CTRL_ClearTask(enIp, &(pstVpssCtrl->stTask));
                }
#else
                    pstVpssCtrl->s32ThreadPos = 3;
                    VPSS_CTRL_CompleteTask(enIp, &(pstVpssCtrl->stTask));//yihua

                    if(jiffies - u32NowTime >= HZ)
                    {
                        u32NowTime = jiffies;
                        pstVpssCtrl->stTask.u32SucRate = pstVpssCtrl->stTask.u32SuccessTotal
                                                       - pstVpssCtrl->stTask.u32LastTotal;
                        pstVpssCtrl->stTask.u32LastTotal = pstVpssCtrl->stTask.u32SuccessTotal;
                    }

                    pstVpssCtrl->stTask.u32SuccessTotal ++;
                    msleep_interruptible(1);
#endif
            }
            else
            {
                    msleep_interruptible(2);
                    MT_INFO_VPSS("SHOULD NOT ENTER HERE\n");
                    MT_ASSERT(0);

            }

        }
        else/*create failed or start failed running -> idle*/
        {
            pstVpssCtrl->stTask.u32Fail++;

VpssThreadIdle:
            pstVpssCtrl->s32ThreadPos = 5;
            pstVpssCtrl->stTask.stState = TASK_STATE_IDLE;

            //VPSS_HAL_SetClockEn(enIp, MT_FALSE);

            //s32Ret = VPSS_OSAL_WaitEvent(&(pstVpssCtrl->stNewTask),ms_to_ktime(10));
            //if(s32Ret == MT_SUCCESS)
            //{
            //    VPSS_INFO("WakeUpThread Success.\n");
            //}

            //VPSS_OSAL_ResetEvent(&(pstVpssCtrl->stNewTask), EVENT_UNDO, EVENT_UNDO);
          //printk("mi exit b2\n");
             t1 = mt_drv_timer_read_cnt(0);
                    msleep_interruptible(1);
                     t2 = mt_drv_timer_read_cnt(0);
          //if ((t2 - t1) > 25)
          //printk("mi exit t2=%d t1=%d %d \n", t2, t1, t2 - t1);
                    //printk("vpss ctrl thread idle status\n");
        }
    }

    VPSS_CTRL_ClearTask(enIp, &(pstVpssCtrl->stTask));

    pstVpssCtrl->s32ThreadPos = 6;
          MT_INFO_VPSS("\n\n\n ... mi exit b2333\n");

    VPSS_INFO("s32ThreadPos = %d...\n",pstVpssCtrl->s32ThreadPos);

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_CreateThread(VPSS_IP_E enIp)
{
    struct sched_param param;
    g_stVpssCtrl[enIp].u32ThreadKilled = 0;
    g_stVpssCtrl[enIp].u32ThreadSleep = 0;
    g_stVpssCtrl[enIp].hThread =
        kthread_create(VPSS_CTRL_ThreadProc, (mt_void *)enIp, "MT_VPSS_Process");

    if (MT_NULL == g_stVpssCtrl[enIp].hThread)
    {
        VPSS_FATAL("Can not create thread.\n");
        return MT_FAILURE;
    }

    param.sched_priority = 99;
    sched_setscheduler(g_stVpssCtrl[enIp].hThread, SCHED_RR, &param);

    wake_up_process(g_stVpssCtrl[enIp].hThread);

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_DestoryThread(VPSS_IP_E enIp)
{
    mt_s32 s32Ret;

    s32Ret = kthread_stop(g_stVpssCtrl[enIp].hThread);

    if (s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("Destory Thread Error.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_CreateInstProc(VPSS_HANDLE hVPSS)
{
    mt_char           ProcName[20];
    mt_proc_entry_t  *pProcItem;

    mt_osal_snprintf(ProcName, 20, "vpss%02x", (mt_u32)(hVPSS));

    pProcItem = mt_drv_proc_add_module(ProcName, MT_NULL, MT_NULL);

    if (!pProcItem)
    {
        VPSS_FATAL("Vpss add proc failed!\n");
        return MT_FAILURE;
    }

    pProcItem->data  = (mt_void *)hVPSS;
    pProcItem->read  = VPSS_CTRL_ProcRead;
    pProcItem->write = VPSS_CTRL_ProcWrite;

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_DestoryInstProc(VPSS_HANDLE hVPSS)
{
    mt_char           ProcName[20];
    mt_osal_snprintf(ProcName, 20, "vpss%02x", (mt_u32)(hVPSS));
    mt_drv_proc_rm_module(ProcName);
    return MT_SUCCESS;
}

mt_u32 VPSS_CTRL_MallocInstanceId(mt_void)
{
    mt_u32 i;
    mt_u32 u32InstCount;
    MT_BOOL bFindVpss;
    mt_u32 u32VpssId;
    VPSS_IP_E enVpssIp;
    unsigned long  u32LockFlag;
    VPSS_INST_CTRL_S *pstInstCtrlInfo;

    u32InstCount = 0;

    for(u32VpssId = 0; u32VpssId < VPSS_IP_BUTT * VPSS_INSTANCE_MAX_NUMB; u32VpssId++)
    {
        bFindVpss = MT_FALSE;

        for(enVpssIp = VPSS_IP_0; enVpssIp < VPSS_IP_BUTT; enVpssIp++)
        {
			//FIX: Critical wild global pointer issue!
			//g_stVpssCtrl[1] is not initialized,
			//access it as wild pointer, panic!
	        if(MT_TRUE != g_stVpssCtrl[enVpssIp].bIPVaild)
	        {
	            continue;
	        }

            pstInstCtrlInfo = &(g_stVpssCtrl[enVpssIp].stInstCtrlInfo);
            read_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);
            for(i = 0; i < VPSS_INSTANCE_MAX_NUMB; i++)
            {
                if(pstInstCtrlInfo->pstInstPool[i] != MT_NULL )
                {
                    if(u32VpssId == pstInstCtrlInfo->pstInstPool[i]->ID)
                    {
                        bFindVpss = MT_TRUE;
                        u32InstCount++;
                        break;
                    }
                }
            }
            read_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);
        }

        if(!bFindVpss)
        {
            break;
        }
    }

    if(u32InstCount == 0)
    {
        return 0;
    }
    else
    {
        return u32VpssId;
    }
}
VPSS_HANDLE VPSS_CTRL_AddInstance(VPSS_INSTANCE_S *pstInstance)
{
    mt_u32 u32VpssId;
    VPSS_INST_CTRL_S *pstInstCtrlInfo;
    mt_u32 u32Count;
    unsigned long  u32LockFlag;
    VPSS_IP_E enVpssIp;


    enVpssIp = pstInstance->CtrlID;
    pstInstCtrlInfo = &(g_stVpssCtrl[enVpssIp].stInstCtrlInfo);
    u32VpssId = VPSS_CTRL_MallocInstanceId();
    write_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);

    for(u32Count = 0; u32Count < VPSS_INSTANCE_MAX_NUMB; u32Count++)
    {
        if (pstInstCtrlInfo->pstInstPool[u32Count] == MT_NULL)
        {
            pstInstCtrlInfo->pstInstPool[u32Count] = pstInstance;

            break;
        }
    }
    write_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

    if (u32Count == VPSS_INSTANCE_MAX_NUMB)
    {
        VPSS_FATAL("Instance Number is Max.\n");

        return VPSS_INVALID_HANDLE;
    }
    else
    {
        pstInstance->ID = u32VpssId;
        pstInstCtrlInfo->u32InstanceNum++;
        return pstInstance->ID;
    }
}

mt_s32 VPSS_CTRL_DelInstance(VPSS_INSTANCE_S* pstInstance)
{
    mt_u32 i;
    VPSS_IP_E enVpssIP;
    VPSS_INST_CTRL_S *pstInstCtrlInfo;
    unsigned long  u32LockFlag;

    VPSS_CHECK_NULL(pstInstance);

    enVpssIP = pstInstance->CtrlID;
    pstInstCtrlInfo = &(g_stVpssCtrl[enVpssIP].stInstCtrlInfo);

    write_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);
    for(i = 0; i < VPSS_INSTANCE_MAX_NUMB; i++)
    {
        if(pstInstCtrlInfo->pstInstPool[i] == pstInstance)
        {
            pstInstCtrlInfo->pstInstPool[i] = MT_NULL;
            break;
        }
    }
    write_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

    return MT_SUCCESS;
}


#if 0
#endif

mt_s32 VPSS_CTRL_Init(mt_void)
{
    mt_s32 s32Ret;
    mt_u32 i,j;

    g_stVpssCtrl[0].bIPVaild = MT_TRUE;
    g_stVpssCtrl[1].bIPVaild = MT_FALSE;


    for(i = 0; i < VPSS_IP_BUTT; i++)
    {
        if(MT_TRUE != g_stVpssCtrl[i].bIPVaild)
        {
			//FIX: Critical wild global pointer issue!
			//g_stVpssCtrl[1] is not initialized,
			//but somewhere access it as wild pointer, panic!
			VPSS_CTRL_InitInstList((VPSS_IP_E)i);
            continue;
        }

        if(0 == g_stVpssCtrl[i].s32IsVPSSOpen)
        {
            s32Ret = VPSS_CTRL_RegistISR((VPSS_IP_E)i);

            if(MT_SUCCESS != s32Ret)
            {
                goto VPSS_IP_DEL_INT;
            }

            VPSS_CTRL_InitInstList((VPSS_IP_E)i);

            s32Ret = VPSS_HAL_Init((VPSS_IP_E)i);
            if(MT_SUCCESS != s32Ret)
            {
                goto VPSS_IP_UnRegist_IRQ;
            }

            s32Ret = VPSS_CTRL_CreateThread((VPSS_IP_E)i);
            if (s32Ret != MT_SUCCESS)
            {
                VPSS_FATAL("VPSS_CTRL_CreateThread Failed\n");
                goto VPSS_IP_HAL_DEL_INIT;
            }
        }

        g_stVpssCtrl[i].s32IsVPSSOpen++;

    }

    return MT_SUCCESS;

VPSS_IP_HAL_DEL_INIT:
    (mt_void)VPSS_HAL_DelInit((VPSS_IP_E)i);
VPSS_IP_UnRegist_IRQ:
    (mt_void)VPSS_CTRL_UnRegistISR((VPSS_IP_E)i);

VPSS_IP_DEL_INT:
    for(j = 0; j < i; j++)
    {
        if(MT_TRUE == g_stVpssCtrl[j].bIPVaild)
        {
            (mt_void)VPSS_CTRL_UnRegistISR((VPSS_IP_E)j);
            (mt_void)VPSS_HAL_DelInit((VPSS_IP_E)j);
            (mt_void)VPSS_CTRL_DestoryThread((VPSS_IP_E)j);
        }
    }

    return MT_FAILURE;

}

mt_s32 VPSS_CTRL_DelInit(mt_void)
{
    mt_u32 i;
    mt_u32 u32Count;
    VPSS_BUFFER_S *pstVpssBuf;

    for(i = 0; i < VPSS_IP_BUTT; i++)
    {
        if(MT_TRUE != g_stVpssCtrl[i].bIPVaild)
        {
            continue;
        }

        if(1 > g_stVpssCtrl[i].s32IsVPSSOpen)
        {
            VPSS_FATAL("CTRL_DelInit Error,vpss hasn't initted.\n");
            return MT_FAILURE;
        }

        g_stVpssCtrl[i].s32IsVPSSOpen--;
        if(1 == g_stVpssCtrl[i].s32IsVPSSOpen)
        {
        for(u32Count = 0; u32Count < VPSS_INSTANCE_MAX_NUMB; u32Count ++)
        {
            if (MT_NULL != g_stVpssCtrl[i].stInstCtrlInfo.pstInstPool[u32Count])
            {
                VPSS_FATAL("CTRL_DelInit Error,destroy instance first.\n");
                g_stVpssCtrl[i].s32IsVPSSOpen++;
                return MT_FAILURE;
                }
            }
            //add for low power
            VPSS_HAL_SetClockEn(i, MT_FALSE);

            //release rotation buffer

            for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count ++)
            {
                pstVpssBuf = &(g_stVpssCtrl[i].stRoBuf[u32Count]);
                if (pstVpssBuf->stMMZBuf.u32Size != 0)
                {
                    mt_drv_mmz_unmap_and_release(&(pstVpssBuf->stMMZBuf));
                    pstVpssBuf->u32Stride = 0;
                    pstVpssBuf->stMMZBuf.u32Size = 0;
                }
            }


        }

        if(0 == g_stVpssCtrl[i].s32IsVPSSOpen)
        {
            (mt_void)VPSS_CTRL_UnRegistISR((VPSS_IP_E)i);
            (mt_void)VPSS_CTRL_DestoryThread((VPSS_IP_E)i);
            (mt_void)VPSS_HAL_DelInit((VPSS_IP_E)i);
        }
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_SetMceFlag(MT_BOOL bInMCE)
{
    mt_u32 i;

    for(i = 0; i < VPSS_IP_BUTT; i++)
    {
        g_stVpssCtrl[i].bInMCE = bInMCE;
    }

    return MT_SUCCESS;
}

VPSS_HANDLE VPSS_CTRL_CreateInstance(MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
    mt_s32 s32Ret;
    mt_s32 s32InstHandle = VPSS_INVALID_HANDLE;
    VPSS_IP_E enVpssIp = VPSS_IP_BUTT;
    VPSS_INSTANCE_S* pstInstance;

    s32Ret = VPSS_CTRL_GetDistributeIP(&enVpssIp);
    if(MT_SUCCESS != s32Ret)
    {
        VPSS_FATAL("vpss ctrl isn't enable\n");
        return s32InstHandle;
    }

    if (1 >= g_stVpssCtrl[enVpssIp].s32IsVPSSOpen)
    {
        VPSS_FATAL("vpss ctrl isn't opened\n");
        return s32InstHandle;
    }

    pstInstance = (VPSS_INSTANCE_S*)VPSS_VMALLOC(sizeof(VPSS_INSTANCE_S));
    if (pstInstance != MT_NULL)
    {
        s32Ret = VPSS_INST_Init(pstInstance,pstVpssCfg);
        if(MT_SUCCESS != s32Ret)
        {
            VPSS_VFREE(pstInstance);
            return s32InstHandle;
        }

        pstInstance->CtrlID = enVpssIp;

        s32InstHandle = VPSS_CTRL_AddInstance(pstInstance);
        if (s32InstHandle != VPSS_INVALID_HANDLE)
        {
            if (g_stVpssCtrl[enVpssIp].bInMCE == MT_FALSE)
            {
                (mt_void)VPSS_CTRL_CreateInstProc(s32InstHandle);
            }
        }
        else
        {
            VPSS_VFREE(pstInstance);
        }

        return s32InstHandle;
    }
    else
    {
        VPSS_FATAL("vmalloc instance node failed \n");
        s32Ret = MT_FAILURE;
        return s32InstHandle;
    }

}

mt_s32 VPSS_CTRL_DestoryInstance(VPSS_HANDLE hVPSS)
{
    VPSS_IP_E enVpssIp = VPSS_IP_BUTT;
    VPSS_INSTANCE_S* pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);
    if (pstInstance == MT_NULL)
    {
        VPSS_FATAL("hVPSS(%d) is Not be Vaild Or Created\n", hVPSS);
        return MT_FAILURE;
    }

    if (pstInstance->enState != INSTANCE_STATE_STOP)
    {
        VPSS_FATAL("Instance is still working, please stop first\n");
        return MT_FAILURE;
    }

    /*
      *  when deletting instance
      *  must get lock first to ensure that it isn't being served
     */
    enVpssIp = pstInstance->CtrlID;
    if (enVpssIp < 0 || enVpssIp >= VPSS_IP_BUTT)
    {
        VPSS_FATAL("VPSS_CTRL_DestoryInstance: enVpssIp(%d) is invalid\n", enVpssIp);
        return MT_FAILURE;
    }
    g_stVpssCtrl[enVpssIp].u32ThreadSleep = 1;

    while(g_stVpssCtrl[enVpssIp].s32ThreadPos != 5)
    {
        msleep(10);
    }

    if (g_stVpssCtrl[enVpssIp].bInMCE == MT_FALSE)
    {
        VPSS_CTRL_DestoryInstProc(hVPSS);
    }

    g_stVpssCtrl[enVpssIp].stInstCtrlInfo.u32InstanceNum--;
    VPSS_CTRL_DelInstance(pstInstance);
    (mt_void)VPSS_INST_DelInit(pstInstance);
    VPSS_VFREE(pstInstance);

    pstInstance = MT_NULL;

    g_stVpssCtrl[enVpssIp].u32ThreadSleep = 0;
    return MT_SUCCESS;

}


VPSS_INSTANCE_S* VPSS_CTRL_GetInstance(VPSS_HANDLE hVPSS)
{
    mt_u32 i;
    mt_u32 u32IpPos;
    VPSS_INST_CTRL_S *pstInstCtrlInfo;
    VPSS_INSTANCE_S *pstRetPtr = MT_NULL;
    unsigned long u32LockFlag;


    if ((hVPSS < 0) || (hVPSS >= VPSS_INSTANCE_MAX_NUMB * VPSS_IP_BUTT))
    {
        VPSS_FATAL("Invalid VPSS HANDLE %x.\n",hVPSS);
        return MT_NULL;
    }

    for(u32IpPos = VPSS_IP_0; u32IpPos < VPSS_IP_BUTT; u32IpPos++)
    {
		//FIX: Critical wild global pointer issue!
		//g_stVpssCtrl[1] is not initialized,
		//access it as wild pointer, panic!
        if(MT_TRUE != g_stVpssCtrl[u32IpPos].bIPVaild)
        {
            continue;
        }

        pstInstCtrlInfo = &(g_stVpssCtrl[u32IpPos].stInstCtrlInfo);
        read_lock_irqsave(&(pstInstCtrlInfo->stListLock),u32LockFlag);
        for(i = 0; i < VPSS_INSTANCE_MAX_NUMB; i++)
		{
			if(pstInstCtrlInfo->pstInstPool[i] != MT_NULL )
			{
				if(hVPSS == pstInstCtrlInfo->pstInstPool[i]->ID)
				{
					pstRetPtr = pstInstCtrlInfo->pstInstPool[i];
					break;
				}
			}
		}
      read_unlock_irqrestore(&(pstInstCtrlInfo->stListLock),u32LockFlag);

    }

    return pstRetPtr;
}

mt_s32 VPSS_CTRL_WakeUpThread(mt_void)
{

    MT_INFO_VPSS("FFFF VPSS_CTRL_WakeUpThread \n");
    if (g_stVpssCtrl[VPSS_IP_0].s32IsVPSSOpen >= 1)
    {
        VPSS_OSAL_GiveEvent(&(g_stVpssCtrl[VPSS_IP_0].stNewTask), 1, 0);
    }

    if (g_stVpssCtrl[VPSS_IP_1].s32IsVPSSOpen >= 1)
    {
        VPSS_OSAL_GiveEvent(&(g_stVpssCtrl[VPSS_IP_1].stNewTask), 1, 0);
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_Pause(VPSS_HANDLE hVPSS)
{
    VPSS_IP_E enVpssIp = VPSS_IP_BUTT;
    VPSS_INSTANCE_S* pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    VPSS_CHECK_NULL(pstInstance);

    enVpssIp = pstInstance->CtrlID;
    if (enVpssIp < 0 || enVpssIp >= VPSS_IP_BUTT)
    {
        VPSS_FATAL("VPSS_CTRL_Pause: enVpssIp(%d) is invalid\n", enVpssIp);
        return MT_FAILURE;
    }
    g_stVpssCtrl[enVpssIp].u32ThreadSleep = 1;

    while(g_stVpssCtrl[enVpssIp].s32ThreadPos != 5)
    {
        msleep(10);
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_CTRL_Resume(VPSS_HANDLE hVPSS)
{
    VPSS_IP_E enVpssIp = VPSS_IP_BUTT;
    VPSS_INSTANCE_S* pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    VPSS_CHECK_NULL(pstInstance);

    enVpssIp = pstInstance->CtrlID;
    if (enVpssIp < 0 || enVpssIp >= VPSS_IP_BUTT)
    {
        VPSS_FATAL("VPSS_CTRL_Resume: enVpssIp(%d) is invalid\n", enVpssIp);
        return MT_FAILURE;
    }

    g_stVpssCtrl[enVpssIp].u32ThreadSleep = 0;

    VPSS_OSAL_GiveEvent(&(g_stVpssCtrl[enVpssIp].stNewTask), 1, 0);

    return MT_SUCCESS;
}

irqreturn_t VPSS0_CTRL_IntService(mt_s32 irq, mt_void *dev_id)
{
    mt_u32 u32State = 0;

    (mt_void)VPSS_HAL_GetIntState(VPSS_IP_0,&u32State);

    if(u32State & 0x80)
    {
        VPSS_FATAL("IRQ DCMP ERR    state = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_0,0x80);
    }

    if(u32State & 0x4)
    {
        VPSS_FATAL("IRQ BUS ERR  state = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_0,0x4);
    }

    if(u32State & 0x2)
    {
        VPSS_FATAL("TIMEOUT  state = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_0,0x2);
    }

    if(u32State & 0x70)//   0xf ---> 0xff open tunl mask and dcmp err mask
    {
        VPSS_FATAL(" Tunnel = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_0,0x70);
    }

    if(u32State & 0x1)
    {
        //VPSS_FATAL("NODE  state = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_0,0x1);
    }

    if(u32State & 0x8)
    {
        VPSS_HAL_ClearIntState(VPSS_IP_0,0x8);
        VPSS_OSAL_GiveEvent(&(g_stVpssCtrl[VPSS_IP_0].stTaskNext),EVENT_DONE,EVENT_UNDO);
    }

    return IRQ_HANDLED;
}

irqreturn_t VPSS1_CTRL_IntService(mt_s32 irq, mt_void *dev_id)
{
    mt_u32 u32State = 0;

    (mt_void)VPSS_HAL_GetIntState(VPSS_IP_1,&u32State);

    if(u32State & 0x80)
    {
        VPSS_FATAL("IRQ DCMP ERR    state = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_1,0x80);
    }

    if(u32State & 0x4)
    {
        VPSS_FATAL("IRQ BUS ERR  state = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_1,0x4);
    }

    if(u32State & 0x2)
    {
        VPSS_FATAL("TIMEOUT  state = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_1,0x2);
    }

    if(u32State & 0x70)//   0xf ---> 0xff open tunl mask and dcmp err mask
    {
        VPSS_FATAL(" Tunnel = %x \n", u32State);
        VPSS_HAL_ClearIntState(VPSS_IP_1,0x70);
    }

    if(u32State & 0x1)
    {
        VPSS_HAL_ClearIntState(VPSS_IP_1,0x1);
    }

    if(u32State & 0x8)
    {
        VPSS_HAL_ClearIntState(VPSS_IP_1,0x8);
        VPSS_OSAL_GiveEvent(&(g_stVpssCtrl[VPSS_IP_1].stTaskNext),EVENT_DONE,EVENT_UNDO);
    }

    return IRQ_HANDLED;
}

mt_s32 VPSS_CTRL_ProcRead(struct seq_file *p, mt_void *v)
{
    VPSS_INSTANCE_S* pstInstance;
    VPSS_IN_STREAM_INFO_S* pstStreamInfo;
    VPSS_IN_ENTITY_S* pstInEntity;
    VPSS_IMAGELIST_STATE_S stImgListState;
    mt_proc_entry_t *pProcItem;
    VPSS_PORT_PRC_S *pstPortPrc[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER];
    VPSS_PORT_S *pstPort;
    mt_s32 s32SrcModuleID;
    mt_u32 u32Count;
    pProcItem = p->private;

    pstInstance = VPSS_CTRL_GetInstance((VPSS_HANDLE)pProcItem->data);

    if(!pstInstance)
    {
        VPSS_FATAL("Can't get instance %x proc!\n",(VPSS_HANDLE)pProcItem->data);
        return MT_FAILURE;
    }
    pstInEntity = &(pstInstance->stInEntity);
    pstStreamInfo = &(pstInstance->stInEntity.stStreamInfo);

    VPSS_INST_GetSrcListState(pstInstance, &stImgListState);

    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        pstPortPrc[u32Count] = VPSS_VMALLOC(sizeof(VPSS_PORT_PRC_S));
        if (pstPortPrc[u32Count] == MT_NULL)
        {
            VPSS_FATAL("Vmalloc Proc space Failed.\n");

            goto READFREE;
        }
        memset(pstPortPrc[u32Count],0,sizeof(VPSS_PORT_PRC_S));

    }

    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);
        VPSS_INST_GetPortPrc(pstInstance,pstPort->s32PortId,pstPortPrc[u32Count]);
    }

    s32SrcModuleID = (pstInstance->hDst & 0x00ff0000) >>16;

    #if 1
    PROC_PRINT(p,
        "--------VPSS%04x---------------|"   "------------------------PortInfo------------------------|\n"
        "ID               :0x%-8x   |"       "ID               :0x%-8x  |0x%-8x  |0x%-8x  |\n"
        "State            :%-10s   |"        "State            :%-3s         |%-3s         |%-3s         |\n"
        "Priority         :%-10d   |"        "PixelFormat      :%-12s|%-12s|%-12s|\n"
        "QuickOutPut      :%-10s   |"        "Resolution       :%4d*%-4d   |%4d*%-4d   |%4d*%-4d   |\n"
        "SourceID         :%-6s(%02x)   |"   "ColorSpace       :%-12s|%-12s|%-12s|\n"
        "Version          :%-10s   |"        "DispPixelAR(W/H) :%2d/%-2d       |%2d/%-2d       |%2d/%-2d       |\n"
        "                               |"   "Aspect Mode      :%-12s|%-12s|%-12s|\n"
        "                               |"   "Support3DStream  :%-12s|%-12s|%-12s|\n"
        "                               |"   "MaxFrameRate     :%-5d       |%-5d       |%-5d       |\n"
        "-------- Algorithm-------------|"   "*LowDelay        :%-12s|%-12s|%-12s|\n"
        "P/I Setting   :%-10s      |"        "HorizonFlip      :%-12s|%-12s|%-12s|\n"
        "Deinterlace   :%-10s      |"        "VerticalFlip     :%-12s|%-12s|%-12s|\n"
        "Sharpness     :%-10s      |"        "Rotation         :%-12s|%-12s|%-12s|\n"
        "*ProgRevise   :%-10s      |"        "                              |            |            |\n"
        "                               |"   "                              |            |            |\n"
        "--------Detect Info------------|"   "                              |            |            |\n"
        "TopFirst(Src):%6s(%-6s)   |"        "                              |            |            |\n"
        "InRate(Src)  :%6d(%-6d)   |"        "                              |            |            |\n"
        "Progressive/Interlace(Src):%-1s(%-1s)|"        "                              |            |            |\n",
        /* attribute */
        pstInstance->ID,
        pstInstance->ID,
                                                pstPortPrc[0]->s32PortId,
                                                pstPortPrc[1]->s32PortId,
                                                pstPortPrc[2]->s32PortId,
        g_pInstState[pstInstance->enState],
                                                g_pAlgModeString[pstInstance->stPort[0].bEnble],
                                                g_pAlgModeString[pstInstance->stPort[1].bEnble],
                                                g_pAlgModeString[pstInstance->stPort[2].bEnble],

        0,
                                                ((pstPortPrc[0]->eFormat - MT_DRV_PIX_FMT_NV12) <= 6)?
                                                g_pPixString[pstPortPrc[0]->eFormat - MT_DRV_PIX_FMT_NV12]:
                                                g_pPixString[7],
                                                ((pstPortPrc[1]->eFormat - MT_DRV_PIX_FMT_NV12) <= 6)?
                                                g_pPixString[pstPortPrc[1]->eFormat - MT_DRV_PIX_FMT_NV12]:
                                                g_pPixString[7],
                                                ((pstPortPrc[2]->eFormat - MT_DRV_PIX_FMT_NV12) <= 6)?
                                                g_pPixString[pstPortPrc[2]->eFormat - MT_DRV_PIX_FMT_NV12]:
                                                g_pPixString[7],

        g_pAlgModeString[pstInstance->bAlwaysFlushSrc],
                                                pstPortPrc[0]->s32OutputWidth,
                                                pstPortPrc[0]->s32OutputHeight,
                                                pstPortPrc[1]->s32OutputWidth,
                                                pstPortPrc[1]->s32OutputHeight,
                                                pstPortPrc[2]->s32OutputWidth,
                                                pstPortPrc[2]->s32OutputHeight,

        (s32SrcModuleID >= MT_ID_VFMW && s32SrcModuleID <= MT_ID_VENC)?
         g_pSrcModuleString[s32SrcModuleID - MT_ID_VFMW]:
        (s32SrcModuleID == 0?g_pSrcModuleString[0]:
         g_pSrcModuleString[9]),
         (pstInstance->hDst & 0x000000ff),
                                                g_pCscString[pstPortPrc[0]->eDstCS],
                                                g_pCscString[pstPortPrc[1]->eDstCS],
                                                g_pCscString[pstPortPrc[2]->eDstCS],
         DEF_SDK_VERSIO_LOG,
                                                pstPortPrc[0]->stDispPixAR.u32ARw,
                                                pstPortPrc[0]->stDispPixAR.u32ARh,
                                                pstPortPrc[1]->stDispPixAR.u32ARw,
                                                pstPortPrc[1]->stDispPixAR.u32ARh,
												pstPortPrc[2]->stDispPixAR.u32ARw,
                                                pstPortPrc[2]->stDispPixAR.u32ARh,

                                                g_pAspString[pstPortPrc[0]->eAspMode],
                                                g_pAspString[pstPortPrc[1]->eAspMode],
                                                g_pAspString[pstPortPrc[2]->eAspMode],

                                                g_pAlgModeString[pstPortPrc[0]->b3Dsupport],
                                                g_pAlgModeString[pstPortPrc[1]->b3Dsupport],
                                                g_pAlgModeString[pstPortPrc[2]->b3Dsupport],

                                                pstPortPrc[0]->u32MaxFrameRate,
                                                pstPortPrc[1]->u32MaxFrameRate,
                                                pstPortPrc[2]->u32MaxFrameRate,
        /*alg config*/                             g_pAlgModeString[pstPortPrc[0]->bTunnelEnable],
                                                g_pAlgModeString[pstPortPrc[1]->bTunnelEnable],
                                                g_pAlgModeString[pstPortPrc[2]->bTunnelEnable],

        g_pProgDetectString[pstInEntity->enProgInfo],
                                                g_pAlgModeString[pstPortPrc[0]->bHoriFlip],
                                                g_pAlgModeString[pstPortPrc[1]->bHoriFlip],
                                                g_pAlgModeString[pstPortPrc[2]->bHoriFlip],
        g_pDeiString[pstInstance->stProcCtrl.eDEI],
                                                g_pAlgModeString[pstPortPrc[0]->bVertFlip],
                                                g_pAlgModeString[pstPortPrc[1]->bVertFlip],
                                                g_pAlgModeString[pstPortPrc[2]->bVertFlip],
        g_pAlgModeString[pstInstance->stProcCtrl.eSharpness],
                                                g_pRotationString[pstPortPrc[0]->enRotation],
                                                g_pRotationString[pstPortPrc[1]->enRotation],
                                                g_pRotationString[pstPortPrc[2]->enRotation],
        g_pAlgModeString[pstInEntity->bProgRevise],
        (pstStreamInfo->u32RealTopFirst == 0 || pstStreamInfo->u32RealTopFirst == 1)?
        ((pstStreamInfo->u32RealTopFirst == 0)?"Bottom":"Top"):"NA",
        (pstStreamInfo->u32StreamTopFirst == 0)?"Bottom":"Top",

        pstStreamInfo->u32InRate*1000,pstStreamInfo->u32StreamInRate,


        (pstStreamInfo->u32RealTopFirst == 0 || pstStreamInfo->u32RealTopFirst == 1)?
        "I":"P",
        (pstStreamInfo->u32StreamTopFirst == 0)?"I":"P"
        );
    #endif
    #if 1
    PROC_PRINT(p,
    "-----SourceFrameList Info------|"  "--------------------OutFrameList Info-------------------|\n"
    "      (source to vpss)         |"  "                     (vpss to sink)                     |\n"
    "*Mutual Mode  :%-11s     |"        "BufManager       :%-10s  |%-10s  |%-10s  |\n"
    "                               |"  "BufNumber        :%-2d+%-2d       |%-2d+%-2d       |%-2d+%-2d       |\n"
    "GetSrcImgHZ(Try/OK)  :%3d/%-3d  |" "BufFul           :%-2d          |%-2d          |%-2d          |\n"
    "GetOutBufHZ(Try/OK)  :%3d/%-3d  |" "BufEmpty         :%-2d          |%-2d          |%-2d          |\n"
    "ProcessHZ(Try/OK)    :%3d/%-3d  |" "AcquireHZ        :%-10d  |%-10d  |%-10d  |\n"
    "Acquire(Try/OK):               |"  "Acquire(Try/OK):              |            |            |\n"
    " %10d/%-10d         |"             " %10d/%-10d%10d/%-10d%10d/%-10d\n"
    "Release(Try/OK):               |"  "Release(Try/OK):              |            |            |\n"
    " %10d/%-10d         |"             " %10d/%-10d%10d/%-10d%10d/%-10d\n",

    g_pSrcMutualString[pstInstance->eSrcImgMode],
                                        g_pBufTypeString[pstPortPrc[0]->stBufListCfg.eBufType],
                                        g_pBufTypeString[pstPortPrc[1]->stBufListCfg.eBufType],
                                        g_pBufTypeString[pstPortPrc[2]->stBufListCfg.eBufType],

    pstPortPrc[0]->stBufListCfg.u32BufNumber,
    pstPortPrc[0]->stFbPrc.u32ExtListNumb,
    pstPortPrc[1]->stBufListCfg.u32BufNumber,
    pstPortPrc[1]->stFbPrc.u32ExtListNumb,
    pstPortPrc[2]->stBufListCfg.u32BufNumber,
    pstPortPrc[2]->stFbPrc.u32ExtListNumb,


    pstInstance->u32ImgRate,
    pstInstance->u32ImgSucRate,
                                        pstPortPrc[0]->stFbPrc.u32FulListNumb,
                                        pstPortPrc[1]->stFbPrc.u32FulListNumb,
                                        pstPortPrc[2]->stFbPrc.u32FulListNumb,
    pstInstance->u32BufRate,
    pstInstance->u32BufSucRate,
                                        pstPortPrc[0]->stFbPrc.u32EmptyListNumb,
                                        pstPortPrc[1]->stFbPrc.u32EmptyListNumb,
                                        pstPortPrc[2]->stFbPrc.u32EmptyListNumb,
    pstInstance->u32CheckRate,
    pstInstance->u32CheckSucRate,
                                        pstPortPrc[0]->stFbPrc.u32GetHZ,
                                        pstPortPrc[1]->stFbPrc.u32GetHZ,
                                        pstPortPrc[2]->stFbPrc.u32GetHZ,


    stImgListState.u32GetUsrTotal,      stImgListState.u32GetUsrSuccess,
    pstPortPrc[0]->stFbPrc.u32GetTotal, pstPortPrc[0]->stFbPrc.u32GetSuccess,
    pstPortPrc[1]->stFbPrc.u32GetTotal, pstPortPrc[1]->stFbPrc.u32GetSuccess,
    pstPortPrc[2]->stFbPrc.u32GetTotal, pstPortPrc[2]->stFbPrc.u32GetSuccess,
    stImgListState.u32RelUsrTotal,      stImgListState.u32RelUsrSuccess,
    pstPortPrc[0]->stFbPrc.u32RelTotal, pstPortPrc[0]->stFbPrc.u32RelSuccess,
    pstPortPrc[1]->stFbPrc.u32RelTotal, pstPortPrc[1]->stFbPrc.u32RelSuccess,
    pstPortPrc[2]->stFbPrc.u32RelTotal, pstPortPrc[2]->stFbPrc.u32RelSuccess
    );


    #endif




READFREE:
    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        if (pstPortPrc[u32Count] != MT_NULL)
            VPSS_VFREE(pstPortPrc[u32Count]);
    }
    return MT_SUCCESS;

}

mt_s32 VPSS_CTRL_ProcWrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    struct seq_file   *s = file->private_data;
    mt_proc_entry_t  *pProcItem = s->private;
    VPSS_HANDLE hVpss;
    mt_char  chCmd[60] = {0};
    mt_char  chArg1[DEF_FILE_NAMELENGTH] = {0};
    mt_char  chArg2[DEF_FILE_NAMELENGTH] = {0};
    mt_char  chArg3[DEF_FILE_NAMELENGTH] = {0};
    mt_s32   s32Ret;
    VPSS_INSTANCE_S *pstInstance;

    hVpss = (VPSS_HANDLE)pProcItem->data;
    pstInstance = VPSS_CTRL_GetInstance(hVpss);
    if (pstInstance == MT_NULL)
    {
        VPSS_FATAL("Can't Get Debug Instance.\n");
        return count;
    }

    if(count > 40)
    {
        VPSS_FATAL("Error:Echo too long.\n");
        return (-1);
    }

    if(copy_from_user(chCmd,buf,count))
    {
        VPSS_FATAL("copy from user failed\n");
        return (-1);
    }


    VPSS_OSAL_GetProcArg(chCmd, chArg1, 1);
    VPSS_OSAL_GetProcArg(chCmd, chArg2, 2);
    VPSS_OSAL_GetProcArg(chCmd, chArg3, 3);

    if (chArg1[0] == 'h' && chArg1[1] == 'e' && chArg1[2] == 'l' && chArg1[3] == 'p')
    {

        mt_drv_proc_echohelp("-------------------VPSS debug options--------------------     \n"
               "you can perform VPSS debug with such command                  \n"
               "echo [arg1] [arg2] [arg3] > /proc/msp/vpssXX\n                \n\n"
               "debug action       arg1              arg2              arg3    \n"
               "-------------    ----------   ---------------------  -----------\n"
               " save one yuv     saveyuv     src/port0/port1/port2             \n"
               " print frameinfo  printinfo   src/port0/port1/port2              \n"
               " turn off info    none        src/port0/port1/port2               \n");
    }
    else
    {
        VPSS_DBG_CMD_S stDbgCmd;

        if (!mt_osal_strncmp(chArg1,DEF_DBG_CMD_YUV,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.enDbgType = DBG_W_YUV;
        }
        else if (!mt_osal_strncmp(chArg1,DEF_DBG_CMD_DBG,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.enDbgType = DBG_INFO_FRM;
        }
        else if (!mt_osal_strncmp(chArg1,DEF_DBG_CMD_NONE,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.enDbgType = DBG_INFO_NONE;
        }
        else if (!mt_osal_strncmp(chArg1,DEF_DBG_CMD_STREAM,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.enDbgType = DBG_W_STREAM;
        }
        else
        {
            VPSS_FATAL("Cmd Can't Support\n");
            goto PROC_EXIT;
        }

        s32Ret = MT_SUCCESS;
        if (!mt_osal_strncmp(chArg2,DEF_DBG_SRC,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.hDbgPart = DEF_DBG_SRC_ID;
        }
        else if (!mt_osal_strncmp(chArg2,DEF_DBG_PORT_0,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.hDbgPart = DEF_DBG_PORT0_ID;
        }
        else if (!mt_osal_strncmp(chArg2,DEF_DBG_PORT_1,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.hDbgPart = DEF_DBG_PORT1_ID;
        }
        else if (!mt_osal_strncmp(chArg2,DEF_DBG_PORT_2,DEF_FILE_NAMELENGTH))
        {
            stDbgCmd.hDbgPart = DEF_DBG_PORT2_ID;
        }
        else
        {
            VPSS_FATAL("Invalid para2 %s\n",chArg2);
            goto PROC_EXIT;
        }

#if DEF_VPSS_DEBUG
        VPSS_DBG_SendDbgCmd(&(pstInstance->stDbgCtrl), &stDbgCmd);
#endif
    }
PROC_EXIT:

    return count;
}





#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

