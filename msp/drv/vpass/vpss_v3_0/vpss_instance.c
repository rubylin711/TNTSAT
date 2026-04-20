/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_reg_vpss.h"
#include "vpss_instance.h"
#include "vpss_common.h"
#include "mt_drv_stat.h"
#include "vpss_ctrl.h"
#include "vpss_alg_ratio.h"
#include "mt_reg_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif
#define SAVEYUV 0
#define IMAGEINFO 0

//#define MT_VPSS_MAX_BUFFER_NUMB 6 //yihua and binxuan

#define MT_VPSS_MAX_BUFFER_NUMB 16


mt_s32 VPSS_INST_UserGetImage(VPSS_HANDLE hVPSS, MT_DRV_VIDEO_FRAME_S *pstImage)
{
    VPSS_INSTANCE_S * pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    return VPSS_SRCIN_GetImage(&pstInstance->stSrcIn, pstImage);
}

mt_s32 VPSS_INST_UserRelImage(VPSS_HANDLE hVPSS, MT_DRV_VIDEO_FRAME_S *pstImage)
{
    VPSS_INSTANCE_S * pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    return VPSS_SRCIN_RelImage(&pstInstance->stSrcIn, pstImage);
}

mt_s32 VPSS_INST_SetUserActiveMode(VPSS_INSTANCE_S * pstInstance)
{
    VPSS_INFO("VPSS_INST_SetUserActiveMode.\n");

    pstInstance->eSrcImgMode = VPSS_SOURCE_MODE_USERACTIVE;
    pstInstance->stSrcFuncs.VPSS_GET_SRCIMAGE = VPSS_INST_UserGetImage;
    pstInstance->stSrcFuncs.VPSS_REL_SRCIMAGE = VPSS_INST_UserRelImage;

    (mt_void)VPSS_SRCIN_Init(&pstInstance->stSrcIn);

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_RevisePortRect(VPSS_INSTANCE_S * pstInstance)
{
    mt_u32 u32Count;

    VPSS_PORT_S *pstPort;

	mt_u32 u32LevelH = 0;
	mt_u32 u32LevelW = 0;

    VPSS_IN_STREAM_INFO_S *pstStreamInfo;

    pstStreamInfo = &(pstInstance->stInEntity.stStreamInfo);

	u32LevelW = pstInstance->u32UhdLevelW;
	u32LevelH = pstInstance->u32UhdLevelH;

    for(u32Count = 0;
        u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER;
        u32Count ++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);

        if(pstPort->s32PortId != VPSS_INVALID_HANDLE)
        {
            if ((pstPort->s32OutputWidth > u32LevelW
                || pstPort->s32OutputHeight > u32LevelH)
                #if 0
                && (pstStreamInfo->u32StreamW <= 1920
					&& pstStreamInfo->u32StreamH <= 1080)
				#endif
					)
            {
                mt_u32 u32RatioW;
                mt_u32 u32RatioH;
                mt_u32 u32TmpH;
                mt_u32 u32TmpW;

                if (pstPort->s32OutputHeight > u32LevelH)
                {
                    u32RatioW = pstPort->s32OutputWidth*2048/u32LevelW;
                    u32RatioH = pstPort->s32OutputHeight*2048/u32LevelH;

                    if (u32RatioW > u32RatioH)
                    {
                        u32TmpW = pstPort->s32OutputWidth*2048/u32RatioW;
                        u32TmpH = pstPort->s32OutputHeight*2048/u32RatioW;
                    }
                    else
                    {
                        u32TmpW = pstPort->s32OutputWidth*2048/u32RatioH;
                        u32TmpH = pstPort->s32OutputHeight*2048/u32RatioH;
                    }
                }
                else
                {
                    u32TmpW = 1920;
                    u32TmpH = pstPort->s32OutputHeight;
                }

                u32TmpW = u32TmpW & 0xfffffffe;
                u32TmpH = u32TmpH & 0xfffffffc;

                pstPort->stDispPixAR.u32ARw =
                    (((pstPort->stDispPixAR.u32ARw
                    * pstPort->s32OutputWidth)/u32TmpW )
                    * u32TmpH) / pstPort->s32OutputHeight;

                pstPort->s32OutputWidth = u32TmpW;
                pstPort->s32OutputHeight = u32TmpH;

            }
        }
    }

    return MT_SUCCESS;
}
mt_s32 VPSS_INST_SyncUsrCfg(VPSS_INSTANCE_S * pstInstance)
{
    unsigned long flags;
    MT_DRV_VPSS_CFG_S *pstInstUsrcCfg;
    mt_u32 u32Count;
    VPSS_PORT_S *pstPort;
    MT_DRV_VPSS_PORT_CFG_S *pstPortCfg;
    VPSS_IN_ATTR_S stInAttr;

    VPSS_OSAL_DownSpin(&(pstInstance->stUsrSetSpin),&flags);
    if(pstInstance->bCfgNew)
    {
        pstInstUsrcCfg = &(pstInstance->stUsrInstCfg);

        memset(&stInAttr,0,sizeof(VPSS_IN_ATTR_S));
        stInAttr.bAlwaysFlushSrc = pstInstUsrcCfg->bAlwaysFlushSrc;
        stInAttr.enProgInfo = pstInstUsrcCfg->enProgInfo;
        stInAttr.bProgRevise = pstInstUsrcCfg->bProgRevise;
        stInAttr.enSrcCS = pstInstUsrcCfg->enSrcCS;
        VPSS_IN_SetAttr(&(pstInstance->stInEntity), stInAttr);

        pstInstance->stProcCtrl = pstInstUsrcCfg->stProcCtrl;

        for(u32Count = 0;
            u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER;
            u32Count ++)
        {
            pstPort = &(pstInstance->stPort[u32Count]);

            if(pstPort->s32PortId != VPSS_INVALID_HANDLE)
            {
                pstPortCfg = &(pstInstance->stUsrPortCfg[u32Count]);

                pstPort->eFormat = pstPortCfg->eFormat;
                pstPort->s32OutputWidth = pstPortCfg->s32OutputWidth;
                pstPort->s32OutputHeight = pstPortCfg->s32OutputHeight;
                //pstPort->s32OutputWidth = 0;
                //pstPort->s32OutputHeight = 0;

                pstPort->eDstCS = pstPortCfg->eDstCS;
                pstPort->stDispPixAR = pstPortCfg->stDispPixAR;
                pstPort->eAspMode = pstPortCfg->eAspMode;
                pstPort->stCustmAR = pstPortCfg->stCustmAR;
                pstPort->stScreen = pstPortCfg->stScreen;
                pstPort->bInterlaced = pstPortCfg->bInterlaced;

                pstPort->bOnlyKeyFrame = pstPortCfg->bOnlyKeyFrame;
                pstPort->bLBDCropEn = pstPortCfg->bLBDCropEn;

                memcpy(&pstPort->stVideoRect, &pstPortCfg->stVideoRect, sizeof(mt_rect_s));

                pstPort->bUseCropRect = pstPortCfg->bUseCropRect;
                memcpy(&pstPort->stInRect, &pstPortCfg->stInRect, sizeof(mt_rect_s));
                memcpy(&pstPort->stCropRect, &pstPortCfg->stCropRect, sizeof(MT_DRV_CROP_RECT_S));

                pstPort->bTunnelEnable = pstPortCfg->bTunnelEnable;
                pstPort->s32SafeThr = pstPortCfg->s32SafeThr;
                pstPort->u32MaxFrameRate = pstPortCfg->u32MaxFrameRate;

                pstPort->b3Dsupport = pstPortCfg->b3Dsupport;
                pstPort->stFrmInfo.stBufListCfg = pstPortCfg->stBufListCfg;
                pstPort->stProcCtrl = pstPortCfg->stProcCtrl;
                pstPort->enRotation = pstPortCfg->enRotation;
                pstPort->bHoriFlip = pstPortCfg->bHoriFlip;
                pstPort->bVertFlip = pstPortCfg->bVertFlip;

                pstPort->enOutBitWidth  = pstPortCfg->enOutBitWidth;

            }
        }

        pstInstance->bCfgNew = MT_FALSE;

    }

    (mt_void)VPSS_INST_RevisePortRect(pstInstance);

    VPSS_OSAL_UpSpin(&(pstInstance->stUsrSetSpin),&flags);

    for(u32Count = 0;
        u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER;
        u32Count ++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);

        if(pstPort->s32PortId != VPSS_INVALID_HANDLE)
        {
            if (pstPort->stFrmInfo.stBufListCfg.eBufType
                    == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE)
            {
                if (pstPort->b3Dsupport == MT_TRUE)
                {
                    VPSS_FB_AllocExtBuffer(&(pstPort->stFrmInfo),
                            pstPort->stFrmInfo.stBufListCfg.u32BufNumber);
                }
                else
                {
                    VPSS_FB_AllocExtBuffer(&(pstPort->stFrmInfo),0);
                }

                VPSS_FB_RlsExtBuffer(&(pstPort->stFrmInfo));
            }
        }
    }

    return MT_SUCCESS;
}


VPSS_PORT_S *VPSS_INST_GetPort(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort)
{

    mt_u32 u32PortID;
    VPSS_PORT_S *pstPort;
    u32PortID = PORTHANDLE_TO_PORTID(hPort);

	if (u32PortID >= DEF_MT_DRV_VPSS_PORT_MAX_NUMBER)
    {
        VPSS_FATAL("Invalid PortID %#x.",u32PortID);
        return MT_NULL;
    }

    pstPort = &(pstInstance->stPort[u32PortID]);

    if (pstPort->s32PortId == VPSS_INVALID_HANDLE)
    {
        VPSS_FATAL("Port doesn't Exit.\n");

        pstPort = MT_NULL;
    }

    return pstPort;
}

mt_s32 VPSS_INST_ResetPort(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort)
{
    VPSS_PORT_S *pstPort;
    mt_u32 u32PortID;

    u32PortID = PORTHANDLE_TO_PORTID(hPort);

    pstPort = MT_NULL;

    pstPort = VPSS_INST_GetPort(pstInstance,hPort);
    if(!pstPort)
    {
        return MT_FAILURE;
    }
    pstPort->u32OutCount = 0;
    VPSS_FB_Reset(&(pstPort->stFrmInfo));

    return MT_SUCCESS;

}

mt_s32 VPSS_INST_Reset(VPSS_INSTANCE_S *pstInstance)
{
    mt_u32 u32Count;
    mt_s32 s32Ret = MT_SUCCESS;
    VPSS_IN_ENTITY_S *pstInEntity;
    VPSS_IN_INTF_S stInIntf;

    VPSS_PORT_S *pstPort;

    VPSS_CHECK_NULL(pstInstance);

    VPSS_CTRL_Pause(pstInstance->ID);

    pstInEntity = &pstInstance->stInEntity;

    s32Ret = VPSS_IN_GetIntf(pstInEntity, &stInIntf);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get In Intf Failed\n");
        return MT_FAILURE;
    }

    s32Ret = stInIntf.pfnReset(pstInEntity);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Reset Failed\n");
        return MT_FAILURE;
    }

    /*reset port*/
    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count ++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);
        if (pstPort->s32PortId != VPSS_INVALID_HANDLE)
        {
            VPSS_INST_ResetPort(pstInstance,pstPort->s32PortId);
        }
    }

    pstInstance->u32CheckRate = 0;
    pstInstance->u32CheckSucRate = 0;;
    pstInstance->u32CheckCnt = 0;
    pstInstance->u32CheckSucCnt = 0;

    pstInstance->u32ImgRate = 0;
    pstInstance->u32ImgSucRate = 0;
    pstInstance->u32ImgCnt = 0;
    pstInstance->u32ImgSucCnt = 0;

    pstInstance->u32SrcRate = 0;
    pstInstance->u32SrcSucRate = 0;
    pstInstance->u32SrcCnt = 0;
    pstInstance->u32SrcSucCnt = 0;

    pstInstance->u32BufRate = 0;
    pstInstance->u32BufSucRate = 0;
    pstInstance->u32BufCnt = 0;
    pstInstance->u32BufSucCnt = 0;
    pstInstance->u32ScenceChgCnt = 0;
    VPSS_CTRL_Resume(pstInstance->ID);

	return MT_SUCCESS;
}

MT_BOOL VPSS_INST_CheckAllDone(VPSS_INSTANCE_S *pstInstance)
{
    mt_s32 s2Ret = MT_FAILURE;
    VPSS_PORT_S *pstPort;
    VPSS_FB_INFO_S *pstFrmListInfo;
    VPSS_SRC_DATA_S *pstData = MT_NULL;
    mt_u32 u32Count;
    MT_BOOL bDone;
    unsigned long flags;
    VPSS_IN_ENTITY_S *pstInEntity;
    VPSS_IN_INTF_S stInIntf = {0};

    pstInEntity = &pstInstance->stInEntity;

    s2Ret = VPSS_IN_GetIntf(pstInEntity, &stInIntf);
    if (MT_SUCCESS != s2Ret)
    {
        VPSS_ERROR("Get In Intf Failed\n");
        return MT_FALSE;
    }

    VPSS_CHECK_NULL(stInIntf.pfnGetProcessImage);
    // TODO:此处返回值有问题
    /*
     *check image all done
     */
    s2Ret = stInIntf.pfnGetProcessImage(pstInEntity, &pstData);
    if (MT_SUCCESS == s2Ret)
    {
        return MT_FALSE;
    }

    /*
     *check all outframe acquired
     */
    bDone = MT_TRUE;
    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER;u32Count ++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);
        if(pstPort->s32PortId != VPSS_INVALID_HANDLE)
        {
            pstFrmListInfo = &(pstPort->stFrmInfo);
            VPSS_OSAL_DownSpin(&(pstFrmListInfo->stFulBufSpin),&flags);
            if (pstFrmListInfo->pstTarget_1 != pstFrmListInfo->stFulFrmList.prev)
            {
                bDone = MT_FALSE;
            }
            VPSS_OSAL_UpSpin(&(pstFrmListInfo->stFulBufSpin),&flags);
        }
    }

    return bDone;
}

MT_BOOL VPSS_INST_CheckPortBuffer(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort)
{
    VPSS_PORT_S *pstPort;
    VPSS_FB_INFO_S *pstFrmListInfo;
    MT_BOOL bAvailable = MT_FALSE;
    VPSS_FB_NODE_S *pstFrmNode;
    MT_DRV_VIDEO_FRAME_S *pstFrm;
    LIST *pstNextNode;
    unsigned long flags;
    pstPort = MT_NULL;

    pstPort = VPSS_INST_GetPort(pstInstance,hPort);

    if(!pstPort)
    {
        return MT_FALSE;
    }
    pstFrmListInfo = &(pstPort->stFrmInfo);
    VPSS_OSAL_DownSpin(&(pstFrmListInfo->stFulBufSpin),&flags);

    pstNextNode = (pstFrmListInfo->pstTarget_1)->next;

    //printk("RRRR 0000 pstTarget_1 = %x, next = %x \n",
    //  pstFrmListInfo->pstTarget_1, (pstFrmListInfo->pstTarget_1)->next);//Rock_hu
    if (pstNextNode != &(pstFrmListInfo->stFulFrmList))
    {
        pstFrmNode = list_entry(pstNextNode,
                        VPSS_FB_NODE_S, node);
        pstFrm = &(pstFrmNode->stOutFrame);

        if(pstFrm->eFrmType == MT_DRV_FT_NOT_STEREO)
        {
            bAvailable = MT_TRUE;
        }
        else
        {
			if(pstNextNode->next != &(pstFrmListInfo->stFulFrmList))
            {
				bAvailable = MT_TRUE;
            }
            else
            {
                bAvailable = MT_FALSE;
            }
        }
    }
    else
    {
        //printk("RRRR 0000 VPSS_INST_CheckPortBuffer fail \n"); yihua
        bAvailable = MT_FALSE;
    }
    VPSS_OSAL_UpSpin(&(pstFrmListInfo->stFulBufSpin),&flags);

    return bAvailable;
}

mt_s32 VPSS_INST_SetState(VPSS_INSTANCE_S *pstInstance,VPSS_INSTANCE_STATE_E enState)
{
    //VPSS_OSAL_DownLock(&(pstInstance->stInstLock));
    pstInstance->enState = enState;
    //VPSS_OSAL_UpLock(&(pstInstance->stInstLock));
    return MT_SUCCESS;
}

mt_s32 VPSS_INST_Init(VPSS_INSTANCE_S *pstInstance,MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
    MT_DRV_VPSS_CFG_S stTmpCfg;
    VPSS_IN_ENV_S stInEnv;
    mt_u32 u32Count;
    mt_s32 s32Ret;
	mt_u32 u32MemSize = 0;

    memset(pstInstance,0,sizeof(VPSS_INSTANCE_S));

    VPSS_OSAL_InitLOCK(&(pstInstance->stInstLock),1);
    VPSS_OSAL_InitSpin(&(pstInstance->stUsrSetSpin));

    pstInstance->ID = 0;
    pstInstance->hDst = 0;
    pstInstance->pfUserCallBack = MT_NULL;
    pstInstance->eSrcImgMode = VPSS_SOURCE_MODE_BUTT;
    pstInstance->stSrcFuncs.VPSS_GET_SRCIMAGE = MT_NULL;
    pstInstance->stSrcFuncs.VPSS_REL_SRCIMAGE = MT_NULL;
    pstInstance->u32ScenceChgCnt = 0;

	s32Ret = VPSS_OSAL_GetSysMemSize(&u32MemSize);
	if (MT_SUCCESS == s32Ret)
	{
		if (u32MemSize == VPSS_SYS_MEM_MIN)
		{
			pstInstance->u32UhdLevelW = VPSS_UHD_LOW_W;
			pstInstance->u32UhdLevelH = VPSS_UHD_LOW_H;
		}
		else
		{
			pstInstance->u32UhdLevelW = VPSS_UHD_MIDDLE_W;
			pstInstance->u32UhdLevelH = VPSS_UHD_MIDDLE_H;
		}
	}
	else
	{
		pstInstance->u32UhdLevelW = VPSS_UHD_MIDDLE_W;
		pstInstance->u32UhdLevelH = VPSS_UHD_MIDDLE_H;
	}

    (mt_void)VPSS_OSAL_GetVpssVersion(&(stInEnv.enVersion));

    VPSS_RWZB_Init(&(pstInstance->stRwzbInfo));

    s32Ret = VPSS_IN_Init(&(pstInstance->stInEntity),stInEnv);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Vpss Init Error\n");
        return MT_FAILURE;
    }


    memset(&(pstInstance->stPort), 0,
                sizeof(VPSS_PORT_S)*DEF_MT_DRV_VPSS_PORT_MAX_NUMBER);
    u32Count = 0;
    while(u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER)
    {
        pstInstance->stPort[u32Count].s32PortId = VPSS_INVALID_HANDLE;
        pstInstance->stPort[u32Count].bEnble = MT_FALSE;
        u32Count++;
    }

    if (MT_NULL == pstVpssCfg)
    {
        VPSS_INST_GetDefInstCfg(&stTmpCfg);
        VPSS_INST_SetInstCfg(pstInstance,&stTmpCfg);
    }
    else
    {
        VPSS_INST_SetInstCfg(pstInstance,pstVpssCfg);
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_CompleteImage(VPSS_INSTANCE_S *pstInstance)
{
    mt_s32 s32Ret;
    VPSS_IN_INTF_S stInIntf;

    VPSS_IN_ENTITY_S *pstEntity;

    //printk("GGGG VPSS_INST_CompleteImage \n");

    pstEntity = &pstInstance->stInEntity;

    s32Ret = VPSS_IN_GetIntf(pstEntity, &stInIntf);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get In Intf Failed\n");
        return MT_FAILURE;
    }

    stInIntf.pfnCompleteImage(pstEntity);

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_DelInit(VPSS_INSTANCE_S *pstInstance)
{
    mt_u32 u32Count;
    VPSS_PORT_S *pstPort;

    mt_s32 s32Ret = MT_SUCCESS;


    if(pstInstance->eSrcImgMode == VPSS_SOURCE_MODE_USERACTIVE)
    {
        VPSS_SRCIN_DeInit(&pstInstance->stSrcIn);
    }

    VPSS_RWZB_DeInit(&(pstInstance->stRwzbInfo));

    s32Ret = VPSS_IN_DeInit(&(pstInstance->stInEntity));
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Vpss In DeInit Error");
        return MT_FAILURE;
    }

    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count ++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);
        if (pstPort->s32PortId != VPSS_INVALID_HANDLE)
        {
            VPSS_INST_DestoryPort(pstInstance,pstPort->s32PortId);

            pstPort->s32PortId = VPSS_INVALID_HANDLE;
        }
    }

    VPSS_DBG_DbgDeInit(&(pstInstance->stDbgCtrl));

    //(mt_void)DRV_PQ_UpdateVpssPQ(pstInstance->ID, MT_NULL, MT_NULL, MT_NULL);//Rock_hu

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_GetDefInstCfg(MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
    MT_DRV_VPSS_PROCESS_S *pstProcCtrl;

    pstVpssCfg->s32Priority = 0;

    pstVpssCfg->bAlwaysFlushSrc = MT_FALSE;

    pstVpssCfg->enProgInfo = MT_DRV_VPSS_PRODETECT_AUTO;
    pstVpssCfg->bProgRevise = MT_TRUE;

    pstVpssCfg->enSrcCS = MT_DRV_CS_UNKNOWN;

    pstProcCtrl = &(pstVpssCfg->stProcCtrl);
    pstProcCtrl->eACC = MT_DRV_VPSS_ACC_DISABLE;
    pstProcCtrl->eACM = MT_DRV_VPSS_ACM_DISABLE;
    pstProcCtrl->eDR = MT_DRV_VPSS_DR_AUTO;
    pstProcCtrl->eDB = MT_DRV_VPSS_DB_AUTO;
    pstProcCtrl->eHFlip = MT_DRV_VPSS_HFLIP_DISABLE;
    pstProcCtrl->eVFlip = MT_DRV_VPSS_VFLIP_DISABLE;
    pstProcCtrl->eCC = MT_DRV_VPSS_CC_AUTO;
    pstProcCtrl->eDEI = MT_DRV_VPSS_DIE_5FIELD;
    pstProcCtrl->eRotation = MT_DRV_VPSS_ROTATION_DISABLE;
    pstProcCtrl->eSharpness = MT_DRV_VPSS_SHARPNESS_AUTO;
    pstProcCtrl->eStereo = MT_DRV_VPSS_STEREO_DISABLE;
    pstProcCtrl->bIFMD = MT_TRUE;

    pstProcCtrl->stInRect.s32X  = 0;
    pstProcCtrl->stInRect.s32Y  = 0;
    pstProcCtrl->stInRect.s32Height = 0;
    pstProcCtrl->stInRect.s32Width = 0;

    pstProcCtrl->bUseCropRect = MT_FALSE;
    pstProcCtrl->stCropRect.u32LeftOffset = 0;
    pstProcCtrl->stCropRect.u32RightOffset= 0;
    pstProcCtrl->stCropRect.u32TopOffset  = 0;
    pstProcCtrl->stCropRect.u32BottomOffset = 0;

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_SetInstCfg(VPSS_INSTANCE_S *pstInstance,
                                MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
    unsigned long flags;
    MT_DRV_VPSS_CFG_S *pstInstUsrcCfg;

    pstInstUsrcCfg = &(pstInstance->stUsrInstCfg);

    //VPSS_OSAL_DownLock(&(pstInstance->stInstLock));
    VPSS_OSAL_DownSpin(&(pstInstance->stUsrSetSpin),&flags);
    pstInstance->bCfgNew = MT_TRUE;
    memcpy(pstInstUsrcCfg,pstVpssCfg,sizeof(MT_DRV_VPSS_CFG_S));
    //VPSS_OSAL_UpLock(&(pstInstance->stInstLock));
    VPSS_OSAL_UpSpin(&(pstInstance->stUsrSetSpin),&flags);

    return MT_SUCCESS;
}

mt_u32 VPSS_INST_GetInstCfg(VPSS_INSTANCE_S *pstInstance,
                                MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
    unsigned long flags;
    MT_DRV_VPSS_CFG_S *pstInstUsrcCfg;
    pstInstUsrcCfg = &(pstInstance->stUsrInstCfg);

    //VPSS_OSAL_DownLock(&(pstInstance->stInstLock));
    VPSS_OSAL_DownSpin(&(pstInstance->stUsrSetSpin),&flags);
    pstVpssCfg->s32Priority = pstInstUsrcCfg->s32Priority;
    pstVpssCfg->bAlwaysFlushSrc = pstInstUsrcCfg->bAlwaysFlushSrc;
    pstVpssCfg->enProgInfo = pstInstUsrcCfg->enProgInfo;
    pstVpssCfg->bProgRevise = pstInstUsrcCfg->bProgRevise;
    pstVpssCfg->enSrcCS = pstInstUsrcCfg->enSrcCS;
    memcpy(&(pstVpssCfg->stProcCtrl),&(pstInstUsrcCfg->stProcCtrl),
                                        sizeof(MT_DRV_VPSS_PROCESS_S));
    VPSS_OSAL_UpSpin(&(pstInstance->stUsrSetSpin),&flags);
    //VPSS_OSAL_UpLock(&(pstInstance->stInstLock));

    return MT_SUCCESS;
}

mt_u32 VPSS_INST_GetDefPortCfg(MT_DRV_VPSS_PORT_CFG_S *pstPortCfg)
{
    memset(pstPortCfg,0,sizeof(MT_DRV_VPSS_PORT_CFG_S));

    pstPortCfg->bTunnelEnable = MT_FALSE;
    pstPortCfg->s32SafeThr = 100;

    pstPortCfg->bOnlyKeyFrame = MT_FALSE;
    pstPortCfg->bLBDCropEn = MT_FALSE;
    pstPortCfg->stVideoRect.s32X = 0;
    pstPortCfg->stVideoRect.s32Y = 0;
    pstPortCfg->stVideoRect.s32Width  = 0;
    pstPortCfg->stVideoRect.s32Height = 0;

    pstPortCfg->stInRect.s32X  = 0;
    pstPortCfg->stInRect.s32Y  = 0;
    pstPortCfg->stInRect.s32Height = 0;
    pstPortCfg->stInRect.s32Width = 0;

    pstPortCfg->bUseCropRect = MT_FALSE;
    pstPortCfg->stCropRect.u32LeftOffset = 0;
    pstPortCfg->stCropRect.u32RightOffset= 0;
    pstPortCfg->stCropRect.u32TopOffset  = 0;
    pstPortCfg->stCropRect.u32BottomOffset = 0;

    pstPortCfg->s32OutputWidth = 0;
    pstPortCfg->s32OutputHeight = 0;

    pstPortCfg->stScreen.s32Height = 576;
    pstPortCfg->stScreen.s32Width = 720;
    pstPortCfg->stScreen.s32X = 0;
    pstPortCfg->stScreen.s32Y = 0;
    pstPortCfg->stDispPixAR.u32ARh = 0;
    pstPortCfg->stDispPixAR.u32ARw = 0;
    pstPortCfg->stCustmAR.u32ARh = 0;
    pstPortCfg->stCustmAR.u32ARw = 0;

    pstPortCfg->eDstCS = MT_DRV_CS_BUTT;
    pstPortCfg->eAspMode  = MT_DRV_ASP_RAT_MODE_FULL;
    pstPortCfg->stProcCtrl.eCSC = MT_DRV_VPSS_CSC_AUTO;
    pstPortCfg->stProcCtrl.eFidelity = MT_DRV_VPSS_FIDELITY_DISABLE;

    pstPortCfg->eFormat = MT_DRV_PIX_FMT_NV21;
    pstPortCfg->u32MaxFrameRate = 60;

    pstPortCfg->stBufListCfg.eBufType = MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE;
    pstPortCfg->stBufListCfg.u32BufNumber = MT_VPSS_MAX_BUFFER_NUMB;
    //pstPortCfg->stBufListCfg.u32BufSize = 720*576*2;
    //pstPortCfg->stBufListCfg.u32BufStride = 720;

    pstPortCfg->stBufListCfg.u32BufSize = 1920*1080*2;
    pstPortCfg->stBufListCfg.u32BufStride = 1920;

    pstPortCfg->b3Dsupport = MT_FALSE;
    pstPortCfg->enRotation = MT_DRV_VPSS_ROTATION_DISABLE;
    pstPortCfg->bHoriFlip = MT_FALSE;
    pstPortCfg->bVertFlip = MT_FALSE;

    pstPortCfg->enOutBitWidth = MT_DRV_PIXEL_BITWIDTH_8BIT;

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_CreatePort(VPSS_INSTANCE_S *pstInstance,
                                MT_DRV_VPSS_PORT_CFG_S *pstPortCfg,
                                VPSS_HANDLE *phPort)
{
    mt_u32 u32Count;
    VPSS_PORT_S *pstPort;
    mt_s32 s32Ret;

    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count ++)
    {
        pstPort = &(pstInstance->stPort[u32Count]);
        if (pstPort->s32PortId == VPSS_INVALID_HANDLE)
        {
            break;
        }
    }

    if (u32Count == DEF_MT_DRV_VPSS_PORT_MAX_NUMBER)
    {
        VPSS_FATAL("Port Number is MAX.\n");

        *phPort = 0;
        return MT_FAILURE;
    }
    else
    {
        MT_DRV_VPSS_PORT_CFG_S stPortDefCfg;
        MT_DRV_VPSS_PORT_CFG_S *pstPortSetCfg;
        memset(pstPort,0,sizeof(VPSS_PORT_S));

        if(pstPortCfg == MT_NULL)
        {
            VPSS_INST_GetDefPortCfg(&(stPortDefCfg));
            pstPortSetCfg = &(stPortDefCfg);
        }
        else
        {
            pstPortSetCfg = pstPortCfg;
        }

        s32Ret = VPSS_FB_Init(&(pstPort->stFrmInfo),&(pstPortSetCfg->stBufListCfg));
        if (s32Ret == MT_FAILURE)
        {
            return MT_FAILURE;
        }
        pstPort->bEnble = MT_FALSE;
        pstPort->s32PortId = (pstInstance->ID * 256) + u32Count;
        pstPort->u32OutCount = 0;

        VPSS_INST_SetPortCfg(pstInstance,pstPort->s32PortId,pstPortSetCfg);

        *phPort = pstPort->s32PortId;
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_DestoryPort(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort)
{
    VPSS_PORT_S *pstPort;
    mt_u32 u32PortID;

    u32PortID = PORTHANDLE_TO_PORTID(hPort);

    pstPort = MT_NULL;

    pstPort = VPSS_INST_GetPort(pstInstance,hPort);
    if(!pstPort)
    {
        VPSS_FATAL("Invalid PortID %#x.", hPort);
        return MT_FAILURE;
    }

    msleep(100);/*暂时规避释放buffer被中断打断的情况*/

    VPSS_FB_DelInit(&(pstPort->stFrmInfo));

    memset(pstPort,0,sizeof(VPSS_PORT_S));

    pstPort->s32PortId = VPSS_INVALID_HANDLE;

    return MT_SUCCESS;

}

mt_s32 VPSS_INST_CheckPortCfg(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort,
                                MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg)
{
    VPSS_PORT_S *pstPort;
    MT_DRV_VPSS_PORT_PROCESS_S *pstProcCtrl;
    pstPort = MT_NULL;

    pstPort = VPSS_INST_GetPort(pstInstance,hPort);

    if(!pstPort)
    {
        return MT_FAILURE;
    }

    pstProcCtrl = &(pstVpssPortCfg->stProcCtrl);

    if(pstVpssPortCfg->eFormat == MT_DRV_PIX_FMT_NV12
		|| pstVpssPortCfg->eFormat == MT_DRV_PIX_FMT_NV16_2X1
		|| pstVpssPortCfg->eFormat == MT_DRV_PIX_FMT_NV21
		|| pstVpssPortCfg->eFormat == MT_DRV_PIX_FMT_NV61_2X1)
    {

    }
	else
	{
		VPSS_FATAL("Invalid Port eFormat %d\n",pstVpssPortCfg->eFormat);
		return MT_FAILURE;
	}

    return MT_SUCCESS;
}


mt_s32 VPSS_INST_GetPortCfg(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort,
                                MT_DRV_VPSS_PORT_CFG_S *pstPortCfg)
{
    unsigned long flags;
    VPSS_PORT_S *pstPort;
    mt_u32 u32PortID;
    MT_DRV_VPSS_PORT_CFG_S *pstUsrPortCfg;
    pstPort = MT_NULL;
    pstPort = VPSS_INST_GetPort(pstInstance,hPort);
    if(!pstPort)
    {
        return MT_FAILURE;
    }

    u32PortID = PORTHANDLE_TO_PORTID(hPort);
    if (u32PortID >= DEF_MT_DRV_VPSS_PORT_MAX_NUMBER)
    {
        VPSS_FATAL("Invalid PortID %#x.",u32PortID);
        return MT_FAILURE;
    }

    //VPSS_OSAL_DownLock(&(pstInstance->stInstLock));
    VPSS_OSAL_DownSpin(&(pstInstance->stUsrSetSpin),&flags);
    pstUsrPortCfg = &(pstInstance->stUsrPortCfg[u32PortID]);
    memcpy(pstPortCfg, pstUsrPortCfg, sizeof(MT_DRV_VPSS_PORT_CFG_S));
    VPSS_OSAL_UpSpin(&(pstInstance->stUsrSetSpin),&flags);
    //VPSS_OSAL_UpLock(&(pstInstance->stInstLock));

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_SetPortCfg(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort,
                                MT_DRV_VPSS_PORT_CFG_S *pstPortCfg)
{
    unsigned long flags;
    VPSS_PORT_S *pstPort;
    mt_u32 u32PortID;
    MT_DRV_VPSS_PORT_CFG_S *pstUsrPortCfg;
    pstPort = MT_NULL;
    pstPort = VPSS_INST_GetPort(pstInstance,hPort);
    if(!pstPort)
    {
        return MT_FAILURE;
    }

    u32PortID = PORTHANDLE_TO_PORTID(hPort);
    if (u32PortID >= DEF_MT_DRV_VPSS_PORT_MAX_NUMBER)
    {
        VPSS_FATAL("Invalid PortID %#x.",u32PortID);
        return MT_FAILURE;
    }

    //VPSS_OSAL_DownLock(&(pstInstance->stInstLock));
    VPSS_OSAL_DownSpin(&(pstInstance->stUsrSetSpin),&flags);
    pstInstance->bCfgNew = MT_TRUE;
    pstUsrPortCfg = &(pstInstance->stUsrPortCfg[u32PortID]);
    memcpy(pstUsrPortCfg, pstPortCfg, sizeof(MT_DRV_VPSS_PORT_CFG_S));
    VPSS_OSAL_UpSpin(&(pstInstance->stUsrSetSpin),&flags);
    //VPSS_OSAL_UpLock(&(pstInstance->stInstLock));

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_EnablePort(VPSS_INSTANCE_S * pstInstance, VPSS_HANDLE hPort,MT_BOOL bEnPort)
{
    VPSS_PORT_S *pstPort;

    pstPort = MT_NULL;

    pstPort = VPSS_INST_GetPort(pstInstance,hPort);

    if(!pstPort)
    {
        return MT_FAILURE;
    }

    pstPort->bEnble = bEnPort;

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_ReplyUserCommand(VPSS_INSTANCE_S * pstInstance,
                                    MT_DRV_VPSS_USER_COMMAND_E eCommand,
                                    mt_void *pArgs)
{
    MT_BOOL *pbAllDone;
    MT_BOOL bVpu;
    MT_DRV_VPSS_PORT_AVAILABLE_S *pstAvailable;
    MT_DRV_VPSS_IPMODE_E enIpmode = MT_DRV_VPSS_IPMODE_AUTO;

    switch ( eCommand )
    {
        case MT_DRV_VPSS_USER_COMMAND_RESET:
            VPSS_INST_Reset(pstInstance);
            break;
        case MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE:
            pbAllDone = (MT_BOOL *)pArgs;
            *pbAllDone = VPSS_INST_CheckAllDone(pstInstance);
            break;
        case MT_DRV_VPSS_USER_COMMAND_CHECKAVAILABLE:
            pstAvailable = (MT_DRV_VPSS_PORT_AVAILABLE_S *)pArgs;
            pstAvailable->bAvailable
                = VPSS_INST_CheckPortBuffer(pstInstance,pstAvailable->hPort);
            break;
        case MT_DRV_VPSS_USER_COMMAND_START:
            (mt_void)VPSS_INST_SetState(pstInstance, INSTANCE_STATE_WORING);
            break;
        case MT_DRV_VPSS_USER_COMMAND_STOP:
            (mt_void)VPSS_INST_SetState(pstInstance, INSTANCE_STATE_STOP);
            break;
        case MT_DRV_VPSS_USER_COMMAND_CHANGEIP:
            bVpu = *(MT_BOOL *)pArgs;
            enIpmode = (bVpu == MT_TRUE) ? MT_DRV_VPSS_IPMODE_IP0: MT_DRV_VPSS_IPMODE_IP1;
            VPSS_CTRL_SyncDistributeIP(pstInstance,enIpmode);
            break;
        default:
            break;
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_GetPortFrame(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort,MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    VPSS_PORT_S *pstPort;
    VPSS_FB_INFO_S *pstFrameList;
    mt_u32 u32PortID;
    mt_s32 s32Ret;
    mt_char *pchFile = MT_NULL;

    u32PortID = PORTHANDLE_TO_PORTID(hPort);

    if (u32PortID >= DEF_MT_DRV_VPSS_PORT_MAX_NUMBER)
    {
        VPSS_FATAL("Invalid PortID %#x.",u32PortID);
        return MT_FAILURE;
    }

    pstPort = MT_NULL;
    pstPort = VPSS_INST_GetPort(pstInstance,hPort);

    if(!pstPort)
    {
        return MT_FAILURE;
    }


    pstFrameList = &(pstPort->stFrmInfo);

    s32Ret = VPSS_FB_GetFulFrmBuf(pstFrameList,pstFrame,pchFile);

    if (s32Ret == MT_SUCCESS)
    {
        #if 0
        if (pstPort->s32PortId == 0x0)
        {
            mt_u8 chFile[20] = "vpss_out.yuv";
            pchFile = chFile;
            VPSS_OSAL_WRITEYUV(pstFrame, pchFile);
        }
        #endif
#if DEF_VPSS_DEBUG
        {
            VPSS_HANDLE hDbgPart;
            switch(pstPort->s32PortId & 0x000000ff)
            {
                case 0:
                    hDbgPart = DEF_DBG_PORT0_ID;
                    break;
                case 1:
                    hDbgPart = DEF_DBG_PORT1_ID;
                    break;
                case 2:
                    hDbgPart = DEF_DBG_PORT2_ID;
                    break;
                default:
                    VPSS_FATAL("Invalid Port ID %#x\n",pstPort->s32PortId);
                    hDbgPart = DEF_DBG_PORT0_ID;
                    break;
            }
            VPSS_DBG_ReplyDbgCmd(&(pstInstance->stDbgCtrl),
                                 DBG_INFO_FRM,
                                 &hDbgPart,
                                 pstFrame);

            VPSS_DBG_ReplyDbgCmd(&(pstInstance->stDbgCtrl),
                                 DBG_W_YUV,
                                 &hDbgPart,
                                 pstFrame);
        }
#endif
    }
    return s32Ret;
}

mt_s32 VPSS_INST_RelPortFrame(VPSS_INSTANCE_S *pstInstance,VPSS_HANDLE hPort,MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    VPSS_PORT_S *pstPort;
    VPSS_FB_INFO_S *pstFrameList;
    mt_s32 s32Ret;

    pstPort = MT_NULL;
    pstPort = VPSS_INST_GetPort(pstInstance,hPort);

    if(!pstPort)
    {
        return MT_FAILURE;
    }
    pstFrameList = &(pstPort->stFrmInfo);

    if (pstFrameList->stBufListCfg.eBufType == MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE
        || pstFrameList->stBufListCfg.eBufType == MT_DRV_VPSS_BUF_USER_ALLOC_VPSS_MANAGE)
    {

    }
    else
    {
        VPSS_FATAL("Buffer type don't support RelPortFrame\n");
        return MT_FAILURE;
    }
    s32Ret = VPSS_FB_RelFulFrmBuf(pstFrameList,pstFrame);

    return s32Ret;
}

mt_s32 VPSS_INST_SetCallBack(VPSS_INSTANCE_S *pstInstance,
                            mt_handle hDst, PFN_VPSS_CALLBACK pfVpssCallback)
{
    pstInstance->hDst = hDst;
    if(pfVpssCallback != MT_NULL)
    {
        pstInstance->pfUserCallBack = pfVpssCallback;

        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }

}

mt_s32 VPSS_INST_GetPortListState(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE hPort,MT_DRV_VPSS_PORT_BUFLIST_STATE_S *pstListState)
{
    VPSS_PORT_S *pstPort;
    VPSS_FB_INFO_S *pstFrameList;
    VPSS_FB_STATE_S stFbState;
    mt_s32 s32Ret;

    pstPort = MT_NULL;
    pstPort = VPSS_INST_GetPort(pstInstance,hPort);

    if(!pstPort)
    {
        return MT_FAILURE;
    }
    pstFrameList = &(pstPort->stFrmInfo);

    s32Ret = VPSS_FB_GetState(pstFrameList, &(stFbState));

    if(MT_SUCCESS == s32Ret)
    {
        pstListState->u32TotalBufNumber = stFbState.u32TotalNumb;
        pstListState->u32FulBufNumber = stFbState.u32FulListNumb;
    }
    else
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_INST_GetUserImage(VPSS_INSTANCE_S *pstInstance,
                                MT_DRV_VIDEO_FRAME_S *pstSrcImage)
{
    mt_s32 s32Ret = MT_FAILURE;
    PFN_VPSS_SRC_FUNC  pfUsrCallBack;

    pfUsrCallBack = pstInstance->stSrcFuncs.VPSS_GET_SRCIMAGE;
    if(MT_NULL == pfUsrCallBack)
    {
        VPSS_FATAL("VPSS_GET_SRCIMAGE doesn't Exit.\n");
        return MT_FAILURE;
    }

    s32Ret = pfUsrCallBack(pstInstance->ID,pstSrcImage);
    return s32Ret;
}



mt_s32 VPSS_INST_CheckUndoImage(VPSS_INSTANCE_S *pstInstance)
{
    mt_s32 s32Ret = MT_SUCCESS;
    VPSS_IN_ENTITY_S *pstEntity;
    MT_VPSS_PQ_INFO_S stVpssPqInfo;
    VPSS_IN_INTF_S stInIntf;

    pstEntity = &pstInstance->stInEntity;

    s32Ret = VPSS_IN_GetIntf(pstEntity, &stInIntf);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get Intf Failed\n");
        return MT_FAILURE;
    }

    s32Ret = stInIntf.pfnRefresh(pstEntity);
    //printk("vpss input refresh 0x%x\n", s32Ret);//Rock_hu
    if (pstEntity->stStreamInfo.u32IsNewImage == MT_TRUE)
    {
        stVpssPqInfo.enInputSrc = pstEntity->stOriInfo.enSource;
        stVpssPqInfo.u32Width = pstEntity->stOriInfo.u32Width;
        stVpssPqInfo.u32Height = pstEntity->stOriInfo.u32Height;
        stVpssPqInfo.u32FrameRate = pstEntity->stOriInfo.u32FrmRate;
        stVpssPqInfo.bInterlace = pstEntity->stOriInfo.bInterlace;
        stVpssPqInfo.enColorSys = pstEntity->stOriInfo.enColorSys;

        //(mt_void)DRV_PQ_UpdateVpssPQ(pstInstance->ID, &stVpssPqInfo, &pstInstance->stPqRegData, &pstInstance->stPQModule);//Rock_hu

        pstEntity->stStreamInfo.u32IsNewImage = MT_FALSE;
    }
    #if 0
    s32Ret = stInIntf.pfnGetProcessImage(pstEntity,&pstImage);
    #endif

    return s32Ret;


}

mt_s32 VPSS_INST_CheckInstAvailable(VPSS_INSTANCE_S *pstInstance)
{
    mt_s32 hDst;
    mt_u32 u32Count;
    mt_s32 s32BufIsEnough;
    MT_BOOL bHasEnPort;
    PFN_VPSS_CALLBACK pfUserCallBack;

    MT_DRV_VPSS_BUFFUL_STRATAGY_E eBufStratagy;
    VPSS_FB_INFO_S * pstFrameList;

    //check if at least one port enabled

    bHasEnPort = 0;
    for (u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        if (pstInstance->stPort[u32Count].s32PortId != VPSS_INVALID_HANDLE
            && pstInstance->stPort[u32Count].bEnble == MT_TRUE)
        {
            bHasEnPort = MT_TRUE;
            break;
        }
    }

    if (bHasEnPort == 0)
    {
        return MT_FAILURE;
    }

    //check undo image
    pstInstance->u32SrcCnt ++;
    if(MT_SUCCESS != VPSS_INST_CheckUndoImage(pstInstance))
    {
        return MT_FAILURE;
    }
    pstInstance->u32ImgCnt
        = pstInstance->stInEntity.pstSrcImagesList->u32GetUsrTotal;
    pstInstance->u32ImgSucCnt
        = pstInstance->stInEntity.pstSrcImagesList->u32GetUsrSuccess;

    pstInstance->u32SrcSucCnt ++;
    s32BufIsEnough = 0;
    bHasEnPort = 0;
    //check if all enabled port has write space
    pstInstance->u32BufCnt ++;

    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        if (pstInstance->stPort[u32Count].s32PortId != VPSS_INVALID_HANDLE
            && pstInstance->stPort[u32Count].bEnble == MT_TRUE)
        {
            bHasEnPort = MT_TRUE;
            s32BufIsEnough = s32BufIsEnough - 1;
            pstFrameList = &((pstInstance->stPort[u32Count]).stFrmInfo);
            if(VPSS_FB_CheckIsAvailable(pstFrameList))
            {
                s32BufIsEnough = s32BufIsEnough + 1;
            }
            else
            {

            }
        }

    }

    if(bHasEnPort == 0)
    {
        return MT_FAILURE;
    }

    if (s32BufIsEnough != 0)
    {
        if(pstInstance->pfUserCallBack == MT_NULL)
        {
            VPSS_FATAL("Inst %d UserCallBack is NULL.\n",pstInstance->ID);
            return MT_FAILURE;
        }

        pfUserCallBack = pstInstance->pfUserCallBack;
        hDst = pstInstance->hDst;
        eBufStratagy = MT_DRV_VPSS_BUFFUL_BUTT;
        pfUserCallBack(hDst, VPSS_EVENT_BUFLIST_FULL, &eBufStratagy);

        if(eBufStratagy == MT_DRV_VPSS_BUFFUL_PAUSE
            || eBufStratagy == MT_DRV_VPSS_BUFFUL_BUTT)
        {
            VPSS_INFO("Inst %d OUT Buf Is FULL.\n",pstInstance->ID);
            return MT_FAILURE;
        }
    }

    pstInstance->u32BufSucCnt ++;
    return MT_SUCCESS;

}

mt_s32 VPSS_INST_RelFrmBuffer(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE  hPort,
                                MT_DRV_VPSS_BUFLIST_CFG_S   *pstBufCfg,
                                mmz_buffer_s *pstMMZBuf)
{
    mt_s32 s32Ret;
    MT_DRV_VPSS_FRMBUF_S stFrmBuf;

    if(pstInstance == MT_NULL)
    {
        VPSS_FATAL("pstInstance is NULL\n");
        return MT_FAILURE;
    }

    if(pstInstance->pfUserCallBack == MT_NULL)
    {
        VPSS_FATAL("pfUserCallBack is NULL\n");
        return MT_FAILURE;
    }
    stFrmBuf.hPort = hPort;
    stFrmBuf.u32Size = pstBufCfg->u32BufSize;
    stFrmBuf.u32Stride = pstBufCfg->u32BufStride;

    stFrmBuf.u32Size = pstMMZBuf->u32Size;
    stFrmBuf.u32StartPhyAddr = pstMMZBuf->u32StartPhyAddr;
    stFrmBuf.u32StartVirAddr = pstMMZBuf->u32StartVirAddr;

    s32Ret = pstInstance->pfUserCallBack(pstInstance->hDst,VPSS_EVENT_REL_FRMBUFFER,&stFrmBuf);

    if(s32Ret == MT_SUCCESS)
    {
        memset(pstMMZBuf,0,sizeof(mmz_buffer_s));
    }

    return s32Ret;
}
mt_s32 VPSS_INST_GetFrmBuffer(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE hPort,
                    MT_DRV_VPSS_BUFLIST_CFG_S* pstBufCfg,VPSS_BUFFER_S *pstBuffer,
                    mt_u32 u32StoreH,mt_u32 u32StoreW)
{
    mmz_buffer_s *pstMMZBuf;
    mt_s32 s32Ret;
    MT_DRV_VPSS_BUFFER_TYPE_E eBufferType;
    MT_DRV_VPSS_FRMBUF_S stFrmBuf;
    mt_u32 u32BufSize;

    if(pstInstance == MT_NULL)
    {
        VPSS_FATAL("pstInstance is NULL\n");
        return MT_FAILURE;
    }
    eBufferType = pstBufCfg->eBufType;
    u32BufSize = pstBufCfg->u32BufSize;

    pstMMZBuf = &(pstBuffer->stMMZBuf);
    if(pstInstance->pfUserCallBack == MT_NULL)
    {
        VPSS_FATAL("pfUserCallBack is NULL\n");
        return MT_FAILURE;
    }
    stFrmBuf.hPort = hPort;
    stFrmBuf.u32Size = pstBufCfg->u32BufSize;
    stFrmBuf.u32Stride = pstBufCfg->u32BufStride;
    stFrmBuf.u32FrmH = u32StoreH;
    stFrmBuf.u32FrmW = u32StoreW;
    s32Ret = pstInstance->pfUserCallBack(pstInstance->hDst,VPSS_EVENT_GET_FRMBUFFER,&stFrmBuf);

    if(s32Ret == MT_SUCCESS)
    {
        pstMMZBuf->u32Size = stFrmBuf.u32Size;
        pstMMZBuf->u32StartPhyAddr = stFrmBuf.u32StartPhyAddr;
        pstMMZBuf->u32StartVirAddr= stFrmBuf.u32StartVirAddr;
        pstBuffer->u32Stride = stFrmBuf.u32Stride;
    }

    return s32Ret;
}
mt_s32 VPSS_INST_ReportNewFrm(VPSS_INSTANCE_S* pstInstance,
                                VPSS_HANDLE  hPort,MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    MT_DRV_VPSS_FRMINFO_S stFrmInfo;

    if(pstInstance == MT_NULL)
    {
        VPSS_FATAL("pstInstance is NULL\n");
        return MT_FAILURE;
    }

    if(pstInstance->pfUserCallBack == MT_NULL)
    {
        VPSS_FATAL("pfUserCallBack is NULL\n");
        return MT_FAILURE;
    }
    stFrmInfo.hPort = hPort;
    memcpy(&(stFrmInfo.stFrame),pstFrm,sizeof(MT_DRV_VIDEO_FRAME_S));

    s32Ret = pstInstance->pfUserCallBack(pstInstance->hDst,VPSS_EVENT_NEW_FRAME,&stFrmInfo);

    return s32Ret;
}

MT_BOOL VPSS_INST_CheckIsDropped(VPSS_INSTANCE_S *pstInstance,mt_u32 u32OutRate,mt_u32 u32OutCount)
{
    mt_u32 u32Multiple;
    mt_u32 u32Quote;
    MT_BOOL bDropped;
    VPSS_IN_INTF_S stInIntf;
    VPSS_IN_STREAM_INFO_S stInfo;
    mt_s32 s32Ret = MT_SUCCESS;
    bDropped = MT_FALSE;

    s32Ret = VPSS_IN_GetIntf(&(pstInstance->stInEntity), &stInIntf);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get intf Failed\n");
        return MT_FALSE;
    }

    (mt_void)stInIntf.pfnGetInfo(&(pstInstance->stInEntity),
                                VPSS_IN_INFO_STREAM,
                                MT_DRV_BUF_ADDR_MAX,
                                &stInfo);


    if(stInfo.u32InRate < u32OutRate || u32OutRate == 0)
    {
         bDropped = MT_FALSE;
    }
    else
    {
        u32Multiple = stInfo.u32InRate*10 / u32OutRate;

        u32Quote = (u32Multiple + 5)/10;

        if(u32OutCount % u32Quote == 1)
        {
            bDropped = MT_TRUE;
        }
        else
        {
            bDropped = MT_FALSE;
        }
    }

    return bDropped;
}

typedef enum{
	MODE_32        =  0,
	MODE_2332      =  1,
	MODE_2224      =  2,
	MODE_64        =  3,
	MODE_55        =  4,
	MODE_32322     =  5,
	MODE_87        =  6,
	MODE_11_2_3    =  7,
	MODE_22        =  8
} PulldownMode;

VPSS_HAL_NODE_TYPE_E VPSS_INST_Check2DNodeType(VPSS_INSTANCE_S* pstInst)
{
    VPSS_IN_INTF_S stIntf = {0};
    MT_DRV_VIDEO_FRAME_S *pstImage;
    mt_s32 s32Ret = MT_SUCCESS;

    (mt_void)VPSS_IN_GetIntf(&(pstInst->stInEntity), &stIntf);

    VPSS_CHECK_NULL(stIntf.pfnGetProcessImage);

    s32Ret = stIntf.pfnGetProcessImage(&(pstInst->stInEntity),&pstImage);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get Process Image Failed\n");
        return MT_FAILURE;
    }

    if(pstImage->u32Width > 1920)
    {
        /*
          VPSS_HAL_NODE_UHD 3840*2160 ->3840*2160
          VPSS_HAL_NODE_UHD_SPLIT_L  3840*2160 ->1920*2160 1920*2160 ->3840*2160
          VPSS_HAL_NODE_UHD_SPLIT_R  3840*2160 ->1920*2160 1920*2160 ->3840*2160
          VPSS_HAL_NODE_UHD_HALF  3840*2160 ->1920*2160
          */
        return VPSS_HAL_NODE_UHD;
    }
    else
    {
        if (pstImage->bProgressive)
        {
            if (pstImage->enFieldMode == MT_DRV_FIELD_ALL)
            {
                return VPSS_HAL_NODE_2D_FRAME;
            }
            else
            {
                return VPSS_HAL_NODE_2D_Field;
            }
        }
        else
        {
            return VPSS_HAL_NODE_2D_5Field;
        }
    }

    return VPSS_HAL_NODE_2D_FRAME;
}

VPSS_HAL_NODE_TYPE_E VPSS_INST_Check3DNodeType(VPSS_INSTANCE_S* pstInst)
{
    VPSS_IN_INTF_S stIntf = {0};
    MT_DRV_VIDEO_FRAME_S *pstImage;
    mt_s32 s32Ret;

    (mt_void)VPSS_IN_GetIntf(&(pstInst->stInEntity), &stIntf);

    if (MT_NULL == stIntf.pfnGetInfo)
    {
        return VPSS_HAL_NODE_BUTT;
    }

    VPSS_CHECK_NULL(stIntf.pfnGetProcessImage);

    s32Ret = stIntf.pfnGetProcessImage(&(pstInst->stInEntity),&pstImage);
    if (MT_SUCCESS != s32Ret)
    {
        VPSS_ERROR("Get Process Image Failed\n");
        return MT_FAILURE;
    }
    //MT_ASSERT(pstInst->stStreamInfo.eStreamFrmType < MT_DRV_FT_BUTT);
    //MT_ASSERT(pstInst->stStreamInfo.eStreamFrmType != MT_DRV_FT_NOT_STEREO);

    if (pstImage->eFrmType == MT_DRV_FT_FPK)
    {
        return VPSS_HAL_NODE_2D_FRAME;
    }
    else if (pstImage->eFrmType == MT_DRV_FT_TAB
            || pstImage->eFrmType == MT_DRV_FT_SBS)
    {
        return VPSS_HAL_NODE_3D_FRAME_R;
    }
    else
    {
        VPSS_ERROR("Invalid frame type %d\n",pstImage->eFrmType);
    }

    return VPSS_HAL_NODE_2D_FRAME;
}


mt_void VPSS_INST_SetHalFrameInfo(MT_DRV_VIDEO_FRAME_S *pstFrame,
    VPSS_HAL_FRAME_S *pstHalFrm, MT_DRV_BUF_ADDR_E enBufLR)
{
    if(pstFrame == MT_NULL || pstHalFrm == MT_NULL)
    {
        VPSS_FATAL("pstFrame(%p), pstHalFrm(%p) NULL\n", pstFrame, pstHalFrm);
        return;
    }
    pstHalFrm->eFrmType = pstFrame->eFrmType;
    pstHalFrm->u32Width = pstFrame->u32Width;
    pstHalFrm->u32Height = pstFrame->u32Height;

    pstHalFrm->enFormat = pstFrame->ePixFormat;
    pstHalFrm->enFieldMode = pstFrame->enFieldMode;
    pstHalFrm->bProgressive = pstFrame->bProgressive;
    memcpy(&pstHalFrm->stAddr, &pstFrame->stBufAddr[enBufLR], sizeof(MT_DRV_VID_FRAME_ADDR_S));
    pstHalFrm->bCompressd = pstFrame->bCompressd;
    pstHalFrm->enBitWidth = pstFrame->enBitWidth;
    pstHalFrm->bTopFirst = pstFrame->bTopFieldFirst;
}

mt_void VPSS_INST_SetOutFrameInfo(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, VPSS_BUFFER_S *pstBuf,
    MT_DRV_VIDEO_FRAME_S *pstFrm, MT_DRV_BUF_ADDR_E enBufLR)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32DstW;
    mt_u32 u32DstH;
    mt_u32 u32BufSize = 0;
    mt_u32 u32BufStride = 0;
    MT_DRV_VIDEO_FRAME_S *pstCur;
    MT_DRV_VIDEO_PRIVATE_S *pstPriv;
    VPSS_PORT_S *pstPort;
    mt_u32 u32PhyAddr, u32Stride;
    VPSS_IN_INTF_S stIntf = {0};

    (mt_void)VPSS_IN_GetIntf(&(pstInst->stInEntity), &stIntf);
    if (MT_NULL == stIntf.pfnGetProcessImage)
    {
        return;
    }

    /* INInfo */
    s32Ret = stIntf.pfnGetProcessImage(&(pstInst->stInEntity), &pstCur);
    if (MT_FAILURE == s32Ret)
    {
        VPSS_FATAL("GetProcessImage failed!\n");
        return;
    }

    memcpy(pstFrm, pstCur, sizeof(MT_DRV_VIDEO_FRAME_S));

    pstPort = &pstInst->stPort[PortId];

    u32DstW = pstPort->s32OutputWidth;
    u32DstH = pstPort->s32OutputHeight;

    if((u32DstW == 0)||(u32DstH == 0))
    {
        mt_rect_s stInRect;
        VPSS_INST_GetInCrop(pstInst, PortId, &stInRect);
        u32DstW = (mt_u32)stInRect.s32Width;
        u32DstH = (mt_u32)stInRect.s32Height;
        //u32DstW = pstInst->stStreamInfo.u32StreamW;
        //u32DstH = pstInst->stStreamInfo.u32StreamH;
    }

    if ((MT_DRV_VPSS_ROTATION_90 == pstPort->enRotation)
        || (MT_DRV_VPSS_ROTATION_270 == pstPort->enRotation))
    {
        pstFrm->u32Width  = u32DstH;
        pstFrm->u32Height = u32DstW;
        VPSS_OSAL_CalBufSize(&u32BufSize, &u32BufStride, pstFrm->u32Height
                             , pstFrm->u32Width, pstPort->eFormat, pstPort->enOutBitWidth);
        if ((pstBuf->stMMZBuf.u32Size != u32BufSize)
            || (pstBuf->u32Stride != u32BufStride))
        {
            if (pstBuf->stMMZBuf.u32Size != 0)
            {
                (mt_void)mt_drv_mmz_unmap_and_release(&(pstBuf->stMMZBuf));
            }

            s32Ret = mt_drv_mmz_alloc_and_map("VPSS_RoBuf", "VPSS",
                                            u32BufSize, 0, &(pstBuf->stMMZBuf));
            if (s32Ret != MT_SUCCESS)
            {
                VPSS_FATAL("Alloc RoBuf Failed\n");
            }

            pstBuf->u32Stride = u32BufStride;

        }


    }
    else
    {
        pstFrm->u32Width  = u32DstW;
        pstFrm->u32Height = u32DstH;
    }

    pstFrm->u32AspectHeight = pstPort->stDispPixAR.u32ARh;
    pstFrm->u32AspectWidth = pstPort->stDispPixAR.u32ARw;

    pstFrm->ePixFormat = pstPort->eFormat;
    pstFrm->bProgressive = MT_TRUE;
    pstFrm->enFieldMode = MT_DRV_FIELD_ALL;

    pstFrm->enBitWidth = pstPort->enOutBitWidth;


    pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstFrm->u32Priv[0]);
    pstPriv->eOriginField = pstCur->enFieldMode;
    VPSS_RWZB_GetRwzbType(&(pstInst->stRwzbInfo), &(pstPriv->u32Fidelity));
    if(pstPort->b3Dsupport && (pstCur->eFrmType != MT_DRV_FT_NOT_STEREO))
    {
        pstFrm->eFrmType = MT_DRV_FT_FPK;
    }
    else
    {
        pstFrm->eFrmType = MT_DRV_FT_NOT_STEREO;
    }

    pstFrm->u32Circumrotate = 0;
    pstFrm->bToFlip_V = 0;

    //:TODO:先简单实现stLbxInfo
    pstFrm->stLbxInfo.s32X = 0;
    pstFrm->stLbxInfo.s32Y = 0;
    pstFrm->stLbxInfo.s32Width  = u32DstW;
    pstFrm->stLbxInfo.s32Height = u32DstH;

    /* 填充地址信息 */
	u32PhyAddr = pstBuf->stMMZBuf.u32StartPhyAddr;
    u32Stride  = pstBuf->u32Stride;

    pstFrm->stBufAddr[enBufLR].u32Stride_Y  =  u32Stride;
    pstFrm->stBufAddr[enBufLR].u32Stride_C  =  u32Stride;

    if(pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV12_CMP
        ||pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV21_CMP
        ||pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV16_2X1_CMP
        ||pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV61_2X1_CMP
        )
    {
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_YHead = u32PhyAddr;
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y =
                    pstFrm->stBufAddr[enBufLR].u32PhyAddr_YHead + pstFrm->u32Height*16;
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_CHead =
                    pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y +u32Stride*pstFrm->u32Height;

        if(pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV12_CMP
           || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV21_CMP)
        {
            pstFrm->stBufAddr[enBufLR].u32PhyAddr_C =
                    pstFrm->stBufAddr[enBufLR].u32PhyAddr_CHead + pstFrm->u32Height*16/2;
        }
        else
        {
            pstFrm->stBufAddr[enBufLR].u32PhyAddr_C =
                    pstFrm->stBufAddr[enBufLR].u32PhyAddr_CHead + pstFrm->u32Height*16;
        }
    }
    else if(pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV12
            ||pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV21
            ||pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV61_2X1
            ||pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV16_2X1)
    {
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y =  u32PhyAddr;
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_C =  u32PhyAddr +
                                    u32Stride*pstFrm->u32Height;
    }
    else if(pstFrm->ePixFormat == MT_DRV_PIX_FMT_ARGB8888
            ||pstFrm->ePixFormat == MT_DRV_PIX_FMT_ABGR8888)
    {
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y =  u32PhyAddr;
    }
    else
    {
        VPSS_FATAL("Invalid Out pixFormat %d,can't get addr\n",
                    pstFrm->ePixFormat);
		return;
    }

}

mt_void VPSS_INST_SetRotationOutFrameInfo(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, VPSS_BUFFER_S* pstBuf,
                                  MT_DRV_VIDEO_FRAME_S* pstFrm, MT_DRV_BUF_ADDR_E enBufLR)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32DstW;
    mt_u32 u32DstH;
    MT_DRV_VIDEO_FRAME_S* pstCur;
    VPSS_PORT_S* pstPort;
    mt_u32 u32PhyAddr, u32Stride;
    VPSS_IN_INTF_S stIntf = {0};

    /* INInfo */
    (mt_void)VPSS_IN_GetIntf(&(pstInst->stInEntity), &stIntf);

    if (MT_NULL == stIntf.pfnGetProcessImage)
    {
        return;
    }

    s32Ret = stIntf.pfnGetProcessImage(&(pstInst->stInEntity), &pstCur);
    if (MT_FAILURE == s32Ret)
    {
        VPSS_FATAL("GetProcessImage failed!\n");
        return;
    }

    memcpy(pstFrm, pstCur, sizeof(MT_DRV_VIDEO_FRAME_S));

    pstPort = &pstInst->stPort[PortId];

    u32DstW = pstPort->s32OutputWidth;
    u32DstH = pstPort->s32OutputHeight;

    if ((u32DstW == 0) || (u32DstH == 0))
    {
        mt_rect_s stInRect;
        VPSS_INST_GetInCrop(pstInst, PortId, &stInRect);
        u32DstW = (mt_u32)stInRect.s32Width;
        u32DstH = (mt_u32)stInRect.s32Height;
    }

    pstFrm->u32Width  = u32DstW;
    pstFrm->u32Height = u32DstH;

    pstFrm->u32AspectHeight = pstPort->stDispPixAR.u32ARh;
    pstFrm->u32AspectWidth = pstPort->stDispPixAR.u32ARw;

    pstFrm->ePixFormat = pstPort->eFormat;
    pstFrm->bProgressive = MT_TRUE;
    pstFrm->enFieldMode = MT_DRV_FIELD_ALL;

    pstFrm->enBitWidth = pstPort->enOutBitWidth;

    if (pstPort->b3Dsupport && (pstCur->eFrmType != MT_DRV_FT_NOT_STEREO))
    {
        pstFrm->eFrmType = MT_DRV_FT_FPK;
    }
    else
    {
        pstFrm->eFrmType = MT_DRV_FT_NOT_STEREO;
    }

    pstFrm->u32Circumrotate = 0;
    pstFrm->bToFlip_V = 0;
    pstFrm->stLbxInfo.s32X = 0;
    pstFrm->stLbxInfo.s32Y = 0;
    pstFrm->stLbxInfo.s32Width  = 0;
    pstFrm->stLbxInfo.s32Height = 0;

    /* 填充地址信息 */
    u32PhyAddr = pstBuf->stMMZBuf.u32StartPhyAddr;
    u32Stride  = pstBuf->u32Stride;

    pstFrm->stBufAddr[enBufLR].u32Stride_Y  =  u32Stride;
    pstFrm->stBufAddr[enBufLR].u32Stride_C  =  u32Stride;

    if (pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV12_CMP
        || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV21_CMP
        || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV16_2X1_CMP
        || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV61_2X1_CMP
       )
    {
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_YHead = u32PhyAddr;
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y =
            pstFrm->stBufAddr[enBufLR].u32PhyAddr_YHead + pstFrm->u32Height * 16;
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_CHead =
            pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y + u32Stride * pstFrm->u32Height;

        if (pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV12_CMP
            || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV21_CMP)
        {
            pstFrm->stBufAddr[enBufLR].u32PhyAddr_C =
                pstFrm->stBufAddr[enBufLR].u32PhyAddr_CHead + pstFrm->u32Height * 16 / 2;
        }
        else
        {
            pstFrm->stBufAddr[enBufLR].u32PhyAddr_C =
                pstFrm->stBufAddr[enBufLR].u32PhyAddr_CHead + pstFrm->u32Height * 16;
        }
    }
    else if (pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV12
             || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV21
             || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV61_2X1
             || pstFrm->ePixFormat == MT_DRV_PIX_FMT_NV16_2X1)
    {
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y =  u32PhyAddr;
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_C =  u32PhyAddr +
                u32Stride * pstFrm->u32Height;
    }
    else if (pstFrm->ePixFormat == MT_DRV_PIX_FMT_ARGB8888
             || pstFrm->ePixFormat == MT_DRV_PIX_FMT_ABGR8888)
    {
        pstFrm->stBufAddr[enBufLR].u32PhyAddr_Y =  u32PhyAddr;
    }
    else
    {
        VPSS_FATAL("Invalid Out pixFormat %d,can't get addr\n",
                   pstFrm->ePixFormat);
        return;
    }

}
mt_void VPSS_INST_GetInCrop(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, mt_rect_s *pstInCropRect)
{
    mt_u32 u32SrcW;
    mt_u32 u32SrcH;
    VPSS_IN_STREAM_INFO_S stInfo;
    VPSS_IN_INTF_S stIntf = {0};

    (mt_void)VPSS_IN_GetIntf(&(pstInst->stInEntity), &stIntf);

    if (MT_NULL == stIntf.pfnGetInfo)
    {
        return;
    }

    (mt_void)stIntf.pfnGetInfo(&(pstInst->stInEntity),
                                VPSS_IN_INFO_STREAM,
                                MT_DRV_BUF_ADDR_MAX,
                                &stInfo);

    u32SrcW = stInfo.u32StreamW;
    u32SrcH = stInfo.u32StreamH;

    pstInCropRect->s32X = 0;
    pstInCropRect->s32Y = 0;
    pstInCropRect->s32Width  = u32SrcW;
    pstInCropRect->s32Height = u32SrcH;

    if(pstInst->stPort[PortId].bUseCropRect)
    {
        MT_DRV_CROP_RECT_S *pCrpRct;

        pCrpRct = &pstInst->stPort[PortId].stCropRect;

        if(((pCrpRct->u32LeftOffset + pCrpRct->u32RightOffset) > u32SrcW)
            ||((pCrpRct->u32TopOffset + pCrpRct->u32BottomOffset) > u32SrcH))
        {
            VPSS_INFO("u32LeftOffset(%d) add u32RightOffset(%d) is too large(%d)\n"
                "Or TopOffset(%d) add u32BottomOffset(%d) is too large(%d)\n",
                pCrpRct->u32LeftOffset, pCrpRct->u32RightOffset, u32SrcW,
                pCrpRct->u32TopOffset, pCrpRct->u32BottomOffset, u32SrcH);
            return;
        }

        pstInCropRect->s32X = pCrpRct->u32LeftOffset;
        pstInCropRect->s32Y = pCrpRct->u32TopOffset;
        pstInCropRect->s32Width  = u32SrcW - (pCrpRct->u32LeftOffset + pCrpRct->u32RightOffset);
        pstInCropRect->s32Height = u32SrcH - (pCrpRct->u32TopOffset + pCrpRct->u32BottomOffset);

    }
    else
    {
        mt_rect_s *pInRct;

        pInRct= &pstInst->stPort[PortId].stInRect;

        if((pInRct->s32X<0)||(pInRct->s32Y<0)||(pInRct->s32Width<=0)||(pInRct->s32Height<=0))
        {
            VPSS_INFO("s32X(%d)  s32Y(%d) s32Width(%d) s32Height(%d) is invaild\n",
                pInRct->s32X, pInRct->s32Y, pInRct->s32Width, pInRct->s32Height);

            return;
        }

        if(((pInRct->s32X + pInRct->s32Width) > u32SrcW)
            ||((pInRct->s32Y + pInRct->s32Height) > u32SrcH))
        {
            VPSS_INFO("s32X(%d) add s32Width(%d) is too large  u32SrcW(%d)n"
                "Or s32Y(%d) add s32Height(%d) is too large  u32SrcH(%d)n",
            pInRct->s32X, pInRct->s32Width, u32SrcW,
            pInRct->s32Y, pInRct->s32Height,u32SrcH);

            return;
        }

        pstInCropRect->s32X = pInRct->s32X;
        pstInCropRect->s32Y = pInRct->s32Y;
        pstInCropRect->s32Width  = pInRct->s32Width;
        pstInCropRect->s32Height = pInRct->s32Height;

    }

    return;
}

mt_void VPSS_INST_GetVideoRect(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, mt_rect_s *pstInCropRect, mt_rect_s *pstVideoRect)
{
    mt_u32 u32DstW;
    mt_u32 u32DstH;

    if ((MT_DRV_VPSS_ROTATION_90 == pstInst->stPort[PortId].enRotation)
        || (MT_DRV_VPSS_ROTATION_270 == pstInst->stPort[PortId].enRotation))
    {
        u32DstW = pstInst->stPort[PortId].s32OutputHeight;
        u32DstH = pstInst->stPort[PortId].s32OutputWidth;
    }
    else
    {
        u32DstW = pstInst->stPort[PortId].s32OutputWidth;
        u32DstH = pstInst->stPort[PortId].s32OutputHeight;
    }

    if((u32DstW == 0)||u32DstH == 0)
    {
        if ((MT_DRV_VPSS_ROTATION_90 == pstInst->stPort[PortId].enRotation)
        || (MT_DRV_VPSS_ROTATION_270 == pstInst->stPort[PortId].enRotation))
        {
            u32DstW = pstInCropRect->s32Height;
            u32DstH = pstInCropRect->s32Width;
        }
        else
        {
            u32DstW = pstInCropRect->s32Width;
            u32DstH = pstInCropRect->s32Height;
        }


        pstVideoRect->s32X = 0;
        pstVideoRect->s32Y = 0;
        pstVideoRect->s32Width  = u32DstW;
        pstVideoRect->s32Height = u32DstH;

        return;
    }

    pstVideoRect->s32X = 0;
    pstVideoRect->s32Y = 0;
    pstVideoRect->s32Width  = u32DstW;
    pstVideoRect->s32Height = u32DstH;

    if(pstInst->stPort[PortId].eAspMode == MT_DRV_ASP_RAT_MODE_TV)
    {
        mt_rect_s *pVRct;

        pVRct = &pstInst->stPort[PortId].stVideoRect;

        if((pVRct->s32X<0)||(pVRct->s32Y<0)||(pVRct->s32Width<=0)||(pVRct->s32Height<=0))
        {
            VPSS_INFO("s32X(%d)  s32Y(%d) s32Width(%d) s32Height(%d) is invaild\n",
                pVRct->s32X, pVRct->s32Y, pVRct->s32Width, pVRct->s32Height);

            return;
        }

        if(((pVRct->s32X + pVRct->s32Width) > u32DstW)
            ||((pVRct->s32Y + pVRct->s32Height) > u32DstH))
        {
            VPSS_WARN("s32X(%d) add s32Width(%d) is too large  u32DstW(%d)\n"
                "Or s32Y(%d) add s32Height(%d) is too large  u32DstH(%d)\n",
            pVRct->s32X, pVRct->s32Width, u32DstW,
            pVRct->s32Y, pVRct->s32Height,u32DstH);

            return;
        }

        pstVideoRect->s32X = pVRct->s32X;
        pstVideoRect->s32Y = pVRct->s32Y;
        pstVideoRect->s32Width  = pVRct->s32Width;
        pstVideoRect->s32Height = pVRct->s32Height;
    }
    else if (pstInst->stPort[PortId].eAspMode == MT_DRV_ASP_RAT_MODE_LETTERBOX)
    {
        mt_rect_s stScreen;
        mt_rect_s stOutWnd;
        mt_s32 s32Ret;
        mt_u32 u32InHeight;
        mt_u32 u32InWidth;

        VPSS_PORT_S* pstPort;

        MT_DRV_VIDEO_FRAME_S *pstImg;

        ALG_RATIO_DRV_PARA_S stAspDrvPara;
        ALG_RATIO_OUT_PARA_S stAspOutPara;
        VPSS_IN_INTF_S stIntf = {0};

        (mt_void)VPSS_IN_GetIntf(&(pstInst->stInEntity), &stIntf);

        pstPort = &(pstInst->stPort[PortId]);

        if (MT_NULL == stIntf.pfnGetProcessImage)
        {
            return;
        }

        /* INInfo */
        s32Ret = stIntf.pfnGetProcessImage(&(pstInst->stInEntity), &pstImg);
        if (MT_FAILURE == s32Ret)
        {
            pstVideoRect->s32X = 0;
            pstVideoRect->s32Y = 0;
            pstVideoRect->s32Width  = u32DstW;
            pstVideoRect->s32Height = u32DstH;
            return ;
        }

        stScreen.s32Height = pstPort->stScreen.s32Height;
        stScreen.s32Width = pstPort->stScreen.s32Width;
        stScreen.s32X = pstPort->stScreen.s32X;
        stScreen.s32Y = pstPort->stScreen.s32Y;

        /*To calculate Rotation AspectRatinTransfer, we must use the origin OutRect*/
        if (pstPort->s32OutputHeight != 0
            && pstPort->s32OutputWidth != 0)
        {
            stOutWnd.s32Height = pstPort->s32OutputHeight;
            stOutWnd.s32Width = pstPort->s32OutputWidth;
        }
        else
        {
            stOutWnd.s32Height = pstImg->u32Height;
            stOutWnd.s32Width = pstImg->u32Width;
        }

        stOutWnd.s32X = 0;
        stOutWnd.s32Y = 0;

        stAspDrvPara.AspectHeight = pstImg->u32AspectHeight;
        stAspDrvPara.AspectWidth  = pstImg->u32AspectWidth;

        stAspDrvPara.DeviceHeight = pstPort->stDispPixAR.u32ARh;
        stAspDrvPara.DeviceWidth  = pstPort->stDispPixAR.u32ARw;

        stAspDrvPara.eAspMode = pstPort->eAspMode;

        stAspDrvPara.stInWnd.s32X = 0;
        stAspDrvPara.stInWnd.s32Y = 0;

        u32InHeight = pstInCropRect->s32Height;
        u32InWidth = pstInCropRect->s32Width;

        if (pstImg->eFrmType == MT_DRV_FT_NOT_STEREO
            || pstImg->eFrmType == MT_DRV_FT_FPK)
        {
            stAspDrvPara.stInWnd.s32Height = u32InHeight;
            stAspDrvPara.stInWnd.s32Width = u32InWidth;
        }
        else if (pstImg->eFrmType == MT_DRV_FT_SBS)
        {
            stAspDrvPara.stInWnd.s32Height = u32InHeight;
            stAspDrvPara.stInWnd.s32Width = u32InWidth * 2;
        }
        else if (pstImg->eFrmType == MT_DRV_FT_TAB)
        {
            stAspDrvPara.stInWnd.s32Height = u32InHeight * 2;
            stAspDrvPara.stInWnd.s32Width = u32InWidth;
        }
        else
        {

        }


        stAspDrvPara.stOutWnd.s32X = 0;
        stAspDrvPara.stOutWnd.s32Y = 0;
        stAspDrvPara.stOutWnd.s32Height = stOutWnd.s32Height;
        stAspDrvPara.stOutWnd.s32Width = stOutWnd.s32Width;

        stAspDrvPara.stScreen.s32X = stScreen.s32X;
        stAspDrvPara.stScreen.s32Y = stScreen.s32Y;
        stAspDrvPara.stScreen.s32Height = stScreen.s32Height;
        stAspDrvPara.stScreen.s32Width = stScreen.s32Width;

        if (pstPort->stCustmAR.u32ARh != 0
            && pstPort->stCustmAR.u32ARw != 0)
        {
            stAspDrvPara.stUsrAsp.bUserDefAspectRatio = MT_TRUE;
        }
        else
        {
            stAspDrvPara.stUsrAsp.bUserDefAspectRatio = MT_FALSE;
        }

        stAspDrvPara.stUsrAsp.u32UserAspectHeight = pstPort->stCustmAR.u32ARh;
        stAspDrvPara.stUsrAsp.u32UserAspectWidth = pstPort->stCustmAR.u32ARw;
        #if 1
        switch(pstPort->enRotation)
        {
            case MT_DRV_VPSS_ROTATION_DISABLE:
            case MT_DRV_VPSS_ROTATION_180:
                break;
            case MT_DRV_VPSS_ROTATION_90:
            case MT_DRV_VPSS_ROTATION_270:
                stAspDrvPara.stOutWnd.s32Width = stOutWnd.s32Height;
                stAspDrvPara.stOutWnd.s32Height = stOutWnd.s32Width;
                stAspDrvPara.stScreen.s32X = stScreen.s32Y;
                stAspDrvPara.stScreen.s32Y = stScreen.s32X;
                stAspDrvPara.stScreen.s32Height = stScreen.s32Width;
                stAspDrvPara.stScreen.s32Width = stScreen.s32Height;
                stAspDrvPara.stUsrAsp.u32UserAspectHeight = pstPort->stCustmAR.u32ARw;
                stAspDrvPara.stUsrAsp.u32UserAspectWidth = pstPort->stCustmAR.u32ARh;
                stAspDrvPara.DeviceHeight = pstPort->stDispPixAR.u32ARw;
                stAspDrvPara.DeviceWidth  = pstPort->stDispPixAR.u32ARh;
                break;
            default:
                VPSS_FATAL("Invalid Rotation Type %d\n",pstPort->enRotation);
                break;
        }
        #endif
        stAspOutPara.bEnAsp = MT_TRUE;
        VPSS_ALG_GetAspCfg(&stAspDrvPara,
                    pstPort->eAspMode,&stScreen,
                    &stAspOutPara);

        /*Get Zme out H/W via AspAlg*/
        pstVideoRect->s32X = stAspOutPara.stOutWnd.s32X;
        pstVideoRect->s32Y = stAspOutPara.stOutWnd.s32Y;
        pstVideoRect->s32Width  = stAspOutPara.stOutWnd.s32Width;
        pstVideoRect->s32Height = stAspOutPara.stOutWnd.s32Height;


    }
    else if (pstInst->stPort[PortId].eAspMode == MT_DRV_ASP_RAT_MODE_FULL
            || pstInst->stPort[PortId].eAspMode == MT_DRV_ASP_RAT_MODE_PANANDSCAN
            || pstInst->stPort[PortId].eAspMode == MT_DRV_ASP_RAT_MODE_COMBINED)
    {
        pstVideoRect->s32X = 0;
        pstVideoRect->s32Y = 0;
        pstVideoRect->s32Width  = u32DstW;
        pstVideoRect->s32Height = u32DstH;
    }
    else
    {
        pstVideoRect->s32X = 0;
        pstVideoRect->s32Y = 0;
        pstVideoRect->s32Width  = u32DstW;
        pstVideoRect->s32Height = u32DstH;
        VPSS_WARN("Aspect Mode %d can't support\n",pstInst->stPort[PortId].eAspMode);

    }

}
mt_void VPSS_INST_GetRotate(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, VPSS_HAL_PORT_INFO_S *pstHalPortInfo, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
	if (pstInst->stPort[PortId].enRotation == MT_DRV_VPSS_ROTATION_180)
	{
        pstHalPortInfo->bNeedFlip = MT_TRUE;
        pstHalPortInfo->bNeedMirror = MT_TRUE;
	}
	else
	{
        pstHalPortInfo->bNeedFlip = MT_FALSE;
        pstHalPortInfo->bNeedMirror = MT_FALSE;
	}

    if (pstFrm->u32Circumrotate != 0)
    {
        pstHalPortInfo->bNeedFlip = !pstHalPortInfo->bNeedFlip;
    }
    else
    {
		/* make tsscan happy */
        //pstHalPortInfo->bNeedFlip = pstHalPortInfo->bNeedFlip;
    }

    pstHalPortInfo->enRotation = pstInst->stPort[PortId].enRotation;

    if (pstInst->stPort[PortId].bHoriFlip == MT_TRUE)
    {
        pstHalPortInfo->bNeedMirror = !pstHalPortInfo->bNeedMirror;
    }

    if (pstInst->stPort[PortId].bVertFlip == MT_TRUE)
    {
        pstHalPortInfo->bNeedFlip = !pstHalPortInfo->bNeedFlip;
    }

    return ;
}
#define VPSS_SAVE_SUB(a,b) (((a)<=(b))?0:((a)-(b)))

#define VPSS_PRINT_RECT(name, a) \
do{\
     break;printk("%s, Top:%d, Bot:%d, Left:%d, Right:%d\n", \
            name,\
            a.u32TopOffset,\
            a.u32BottomOffset,\
            a.u32LeftOffset,\
            a.u32RightOffset);\
}while(0)


mt_void VPSS_INST_LBX_DET(VPSS_LBX_DET_S *pstLbxDet, mt_u32 u32sttWbcInfo)
{
    #if 0
    mt_u32 i;
    S_VPSSWB_REGS_TYPE *psttwbcInfo;

    /* 获取寄存器的LBX信息, 只看左眼的统计信息 */
    psttwbcInfo = (S_VPSSWB_REGS_TYPE *)u32sttWbcInfo;

    pstLbxDet->u32valid_top   = psttwbcInfo->LBD_INFO_0.bits.lbd_fix_top;
    pstLbxDet->u32valid_bot   = psttwbcInfo->LBD_INFO_0.bits.lbd_fix_bot;
    pstLbxDet->u32valid_left  = psttwbcInfo->LBD_INFO_1.bits.lbd_fix_left;
    pstLbxDet->u32valid_right = psttwbcInfo->LBD_INFO_1.bits.lbd_fix_right;

    pstLbxDet->m_top[pstLbxDet->u32NodeIndex]   = pstLbxDet->u32valid_top;
    pstLbxDet->m_bot[pstLbxDet->u32NodeIndex]   = pstLbxDet->u32valid_bot;
    pstLbxDet->m_left[pstLbxDet->u32NodeIndex]  = pstLbxDet->u32valid_left;
    pstLbxDet->m_right[pstLbxDet->u32NodeIndex] = pstLbxDet->u32valid_right;

    for(i=0; i<VPSS_LBX_DET_NODE_NUM; i++)
    {
        if( pstLbxDet->m_top[i] < pstLbxDet->u32valid_top)
            pstLbxDet->u32valid_top = pstLbxDet->m_top[i];
        if( pstLbxDet->m_bot[i] > pstLbxDet->u32valid_bot)
            pstLbxDet->u32valid_bot = pstLbxDet->m_bot[i];
        if( pstLbxDet->m_left[i] < pstLbxDet->u32valid_left)
            pstLbxDet->u32valid_left = pstLbxDet->m_left[i];
        if( pstLbxDet->m_right[i] > pstLbxDet->u32valid_right)
            pstLbxDet->u32valid_right = pstLbxDet->m_right[i];
    }

    pstLbxDet->u32NodeIndex =(pstLbxDet->u32NodeIndex+1)%32;
    #endif
}

mt_void VPSS_INST_GetLbxInfo(VPSS_INSTANCE_S* pstInst, mt_u32 PortId, mt_rect_s *pstLbx)
{
    mt_u32 u32SrcW=0, u32SrcH=0;
    mt_u32 u32DstW=0, u32DstH=0;
    mt_rect_s stInRect;
    #if 0
    mt_rect_s stVideoRect;
    MT_DRV_CROP_RECT_S stLdb;  /* 硬件检测的结果 */
    MT_DRV_CROP_RECT_S stCrop; /* 用户配置的CROP */
    MT_DRV_CROP_RECT_S stLeave;/* 硬件检测减去CROP剩下的部分 */
    MT_DRV_CROP_RECT_S stCalc; /* 经过缩放之后的LBX */
    MT_DRV_CROP_RECT_S stAdd;  /* 用户新加的LBX */
    MT_DRV_CROP_RECT_S stFinal;/* 最终的LBX */
	#endif
    VPSS_IN_STREAM_INFO_S stInfo;
    VPSS_IN_INTF_S stIntf = {0};
    VPSS_PORT_S *pstPort;

    (mt_void)VPSS_IN_GetIntf(&(pstInst->stInEntity), &stIntf);

    if (stIntf.pfnGetInfo == MT_NULL)
    {
        return;
    }

    (mt_void)stIntf.pfnGetInfo(&(pstInst->stInEntity),
                                VPSS_IN_INFO_STREAM,
                                MT_DRV_BUF_ADDR_MAX,
                                &stInfo);


    u32SrcW = stInfo.u32StreamW;
    u32SrcH = stInfo.u32StreamH;

    VPSS_INST_GetInCrop(pstInst, PortId, &stInRect);

    pstPort = &pstInst->stPort[PortId];

    u32DstW = pstPort->s32OutputWidth;
    u32DstH = pstPort->s32OutputHeight;

    if((u32DstW == 0)||(u32DstH == 0))
    {
        u32DstW = (mt_u32)stInRect.s32Width;
        u32DstH = (mt_u32)stInRect.s32Height;
    }
        #if 0
    (mt_void)stIntf.pfnGetInfo(&(pstInst->stInEntity),
                                VPSS_IN_INFO_WBCREG_VA,
                                MT_DRV_BUF_ADDR_LEFT,
                                &u32WbcRegVirAddr);

    VPSS_INST_LBX_DET(&pstInst->stLbxDet, u32WbcRegVirAddr);

    stLdb.u32TopOffset  = pstInst->stLbxDet.u32valid_top + 1;
    stLdb.u32LeftOffset = pstInst->stLbxDet.u32valid_left + 1;
    stLdb.u32BottomOffset
        = VPSS_SAVE_SUB(u32SrcH, pstInst->stLbxDet.u32valid_bot);
    stLdb.u32RightOffset
        = VPSS_SAVE_SUB(u32SrcW, pstInst->stLbxDet.u32valid_right);

    VPSS_PRINT_RECT("stLdb",stLdb);


    /* 获取裁剪的信息 */
    stCrop.u32TopOffset  = (mt_u32)stInRect.s32Y;
    stCrop.u32LeftOffset = (mt_u32)stInRect.s32X;
    stCrop.u32BottomOffset
        = VPSS_SAVE_SUB(u32SrcH, ((mt_u32)stInRect.s32Y + (mt_u32)stInRect.s32Height));
    stCrop.u32RightOffset
        = VPSS_SAVE_SUB(u32SrcW, ((mt_u32)stInRect.s32X + (mt_u32)stInRect.s32Width));

    VPSS_PRINT_RECT("stCrop",stCrop);

    /* 剩余的LBX信息 */
    stLeave.u32TopOffset    = VPSS_SAVE_SUB(stLdb.u32TopOffset, stCrop.u32TopOffset);
    stLeave.u32LeftOffset   = VPSS_SAVE_SUB(stLdb.u32LeftOffset, stCrop.u32LeftOffset);
    stLeave.u32BottomOffset = VPSS_SAVE_SUB(stLdb.u32BottomOffset, stCrop.u32BottomOffset);
    stLeave.u32RightOffset  = VPSS_SAVE_SUB(stLdb.u32RightOffset, stCrop.u32RightOffset);

    VPSS_PRINT_RECT("stLeave",stLeave);

    /* 计算缩放之后的LBX信息 */
    VPSS_INST_GetVideoRect(pstInst, PortId, &stInRect, &stVideoRect);

    stCalc.u32TopOffset    = (stLeave.u32TopOffset * (mt_u32)stVideoRect.s32Height * 1024 + 512)
        / ((mt_u32)stInRect.s32Height*1024);
    stCalc.u32LeftOffset   = (stLeave.u32LeftOffset * (mt_u32)stVideoRect.s32Width * 1024 + 512)
        / ((mt_u32)stInRect.s32Width*1024);
    stCalc.u32BottomOffset = (stLeave.u32BottomOffset * (mt_u32)stVideoRect.s32Height * 1024 + 512)
        / ((mt_u32)stInRect.s32Height*1024);
    stCalc.u32RightOffset  = (stLeave.u32RightOffset * (mt_u32)stVideoRect.s32Width * 1024 + 512)
        / ((mt_u32)stInRect.s32Width*1024);

    VPSS_PRINT_RECT("stCalc",stCalc);

    /* 新加的LBX信息 */
    stAdd.u32TopOffset  = (mt_u32)stVideoRect.s32Y;
    stAdd.u32LeftOffset = (mt_u32)stVideoRect.s32X;
    stAdd.u32BottomOffset
        = VPSS_SAVE_SUB(u32DstH, ((mt_u32)stVideoRect.s32Y + (mt_u32)stVideoRect.s32Height));
    stAdd.u32RightOffset
        = VPSS_SAVE_SUB(u32DstW, ((mt_u32)stVideoRect.s32X + (mt_u32)stVideoRect.s32Width));
    VPSS_PRINT_RECT("stAdd",stAdd);

    /* 最终的LBX信息 */
    stFinal.u32TopOffset    = stCalc.u32TopOffset + stAdd.u32TopOffset;
    stFinal.u32LeftOffset   = stCalc.u32LeftOffset + stAdd.u32LeftOffset;
    stFinal.u32BottomOffset = stCalc.u32BottomOffset + stAdd.u32BottomOffset;
    stFinal.u32RightOffset  = stCalc.u32RightOffset + stAdd.u32RightOffset;

    VPSS_PRINT_RECT("stFinal",stFinal);

    pstLbx->s32X = (mt_s32)stFinal.u32LeftOffset;
    pstLbx->s32Y = (mt_s32)stFinal.u32TopOffset;
    pstLbx->s32Width  = (mt_s32)(VPSS_SAVE_SUB(u32DstW, (stFinal.u32LeftOffset + stFinal.u32RightOffset)));
    pstLbx->s32Height = (mt_s32)(VPSS_SAVE_SUB(u32DstH, (stFinal.u32TopOffset + stFinal.u32BottomOffset)));
    #else
    pstLbx->s32X = 0;
    pstLbx->s32Y = 0;
    pstLbx->s32Width  = u32DstW;
    pstLbx->s32Height = u32DstH;
    #endif
}

mt_s32 VPSS_INST_GetPortPrc(VPSS_INSTANCE_S* pstInstance,VPSS_HANDLE hPort,VPSS_PORT_PRC_S *pstPortPrc)
{
    VPSS_PORT_S *pstPort;
    pstPort = MT_NULL;

    if (hPort == VPSS_INVALID_HANDLE)
    {
        memset(pstPortPrc,0,sizeof(VPSS_PORT_PRC_S));
        pstPortPrc->s32PortId = VPSS_INVALID_HANDLE;
    }
    else
    {
        pstPort = VPSS_INST_GetPort(pstInstance,hPort);
        if (pstPort == MT_NULL)
        {
            VPSS_FATAL("Get Port Proc Error.\n");
            return MT_FAILURE;
        }
        pstPortPrc->s32PortId = pstPort->s32PortId ;
        pstPortPrc->bEnble = pstPort->bEnble ;
        pstPortPrc->eFormat = pstPort->eFormat ;
        pstPortPrc->s32OutputWidth = pstPort->s32OutputWidth ;
        pstPortPrc->s32OutputHeight = pstPort->s32OutputHeight ;
        pstPortPrc->eDstCS = pstPort->eDstCS ;
        pstPortPrc->stDispPixAR = pstPort->stDispPixAR ;
        pstPortPrc->eAspMode = pstPort->eAspMode ;
        pstPortPrc->stCustmAR = pstPort->stCustmAR ;
        pstPortPrc->bInterlaced = pstPort->bInterlaced ;
        pstPortPrc->stScreen = pstPort->stScreen ;
        pstPortPrc->u32MaxFrameRate = pstPort->u32MaxFrameRate ;
        pstPortPrc->u32OutCount = pstPort->u32OutCount ;
        pstPortPrc->stProcCtrl = pstPort->stProcCtrl ;
        pstPortPrc->bTunnelEnable = pstPort->bTunnelEnable ;
        pstPortPrc->s32SafeThr = pstPort->s32SafeThr ;
        pstPortPrc->b3Dsupport = pstPort->b3Dsupport;
        pstPortPrc->stBufListCfg.eBufType = pstPort->stFrmInfo.stBufListCfg.eBufType;
        pstPortPrc->stBufListCfg.u32BufNumber = pstPort->stFrmInfo.stBufListCfg.u32BufNumber;
        pstPortPrc->stBufListCfg.u32BufSize = pstPort->stFrmInfo.stBufListCfg.u32BufSize;
        pstPortPrc->stBufListCfg.u32BufStride = pstPort->stFrmInfo.stBufListCfg.u32BufStride;

        pstPortPrc->bVertFlip = pstPort->bVertFlip;
        pstPortPrc->bHoriFlip = pstPort->bHoriFlip;
        pstPortPrc->enRotation = pstPort->enRotation;
        VPSS_FB_GetState(&(pstPort->stFrmInfo),&(pstPortPrc->stFbPrc));
    }

    return MT_SUCCESS;
}


mt_s32 VPSS_INST_UpdatePqInfo(VPSS_INSTANCE_S *pstInstance,mt_u32 u32Width,mt_u32 u32Height)
{
    MT_PQ_MOTION_INPUT_S stMotionInput;
    //unused
    //MT_PQ_MOTION_INFO_S stMotionResult;
    stMotionInput.u32HandleNo = pstInstance->ID;
    stMotionInput.u32Width = u32Width;
    stMotionInput.u32Height = u32Height;
    //(mt_void)DRV_PQ_GetDeiGlobalMotion(&stMotionInput, &stMotionResult);//Rock_hu

    return MT_SUCCESS;
}
mt_s32 VPSS_INST_GetSrcListState(VPSS_INSTANCE_S* pstInstance,VPSS_IMAGELIST_STATE_S *pstListState)
{
    VPSS_IN_ENTITY_S *pstInEntity;
    pstInEntity = &pstInstance->stInEntity;

    VPSS_CHECK_NULL(pstInEntity->pstSrcImagesList);

    pstListState->u32GetUsrTotal = pstInEntity->pstSrcImagesList->u32GetUsrTotal;
    pstListState->u32GetUsrSuccess = pstInEntity->pstSrcImagesList->u32GetUsrSuccess;
    pstListState->u32RelUsrTotal = pstInEntity->pstSrcImagesList->u32RelUsrTotal;
    pstListState->u32RelUsrSuccess = pstInEntity->pstSrcImagesList->u32RelUsrSuccess;
    return MT_SUCCESS;
}
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
