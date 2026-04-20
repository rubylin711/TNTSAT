/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_hal_s40v300.h"
#include "linux/kthread.h"
#include "vpss_common.h"

#include <asm/barrier.h>
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#if 1
VPSS_HAL_CTX_S  stHalCtx[VPSS_IP_BUTT] =
{
    {
        .bInit = MT_FALSE,
        .bClockEn = MT_FALSE,
        .u32LogicVersion = HAL_VERSION_3798M,
        .u32BaseRegPhy   = VPSS0_BASE_ADDR,
        .u32BaseRegVir   = 0,
    },

    {
        .bInit = MT_FALSE,
        .bClockEn = MT_FALSE,
        .u32LogicVersion = HAL_VERSION_3798M,
        .u32BaseRegPhy   = VPSS1_BASE_ADDR,
        .u32BaseRegVir   = 0,
    }
};
#endif

VPSS_REG_PORT_E VPSS_HAL_AllocPortId(VPSS_HAL_PORT_INFO_S *pstHalPort,
    MT_BOOL abPortUsed[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER])
{
    if (MT_FALSE == abPortUsed[VPSS_REG_HD])
    {
         abPortUsed[VPSS_REG_HD] = MT_TRUE;
         return VPSS_REG_HD;
    }

    if (MT_FALSE == abPortUsed[VPSS_REG_STR])
    {
         abPortUsed[VPSS_REG_STR] = MT_TRUE;
         return VPSS_REG_STR;
    }

    if (MT_FALSE == abPortUsed[VPSS_REG_SD])
    {
         abPortUsed[VPSS_REG_SD] = MT_TRUE;
         return VPSS_REG_SD;
    }

    return VPSS_REG_BUTT;
}

ZME_FORMAT_E VPSS_HAL_GetZmeFmt(MT_DRV_PIX_FORMAT_E enFormat)
{
    ZME_FORMAT_E enZmeFmt = MT_PQ_ALG_PIX_FORMAT_SP420;

    switch(enFormat)
    {
        case MT_DRV_PIX_FMT_NV21:
        case MT_DRV_PIX_FMT_NV12:
        case MT_DRV_PIX_FMT_NV21_CMP:
        case MT_DRV_PIX_FMT_NV12_CMP:
        case MT_DRV_PIX_FMT_NV12_TILE:
        case MT_DRV_PIX_FMT_NV21_TILE:
        case MT_DRV_PIX_FMT_NV12_TILE_CMP:
        case MT_DRV_PIX_FMT_NV21_TILE_CMP:
            enZmeFmt = MT_PQ_ALG_PIX_FORMAT_SP420;
            break;
        case MT_DRV_PIX_FMT_NV61_2X1:
        case MT_DRV_PIX_FMT_NV16_2X1:
        case MT_DRV_PIX_FMT_NV61_2X1_CMP:
        case MT_DRV_PIX_FMT_NV16_2X1_CMP:
        case MT_DRV_PIX_FMT_YUYV:
        case MT_DRV_PIX_FMT_YVYU:
        case MT_DRV_PIX_FMT_UYVY:
        //case MT_DRV_PIX_FMT_ARGB8888:   //sp420->sp422->csc->rgb
        //case MT_DRV_PIX_FMT_ABGR8888:
        //case MT_DRV_PIX_FMT_KBGR8888:
            enZmeFmt = MT_PQ_ALG_PIX_FORMAT_SP422;
            break;
        default:
            VPSS_FATAL("REG ERROR format %d\n",enFormat);
    }

    return enZmeFmt;
}

mt_s32 VPSS_HAL_GetZmeCoef(VPSS_IP_E enIP,VPSS_HAL_ZME_PARAM_S *pstZmeParam,
    mt_u32 *pu32YH, mt_u32 *pu32CH, mt_u32 *pu32YV, mt_u32 *pu32CV)
{
    mt_u32 i;
    mt_s32 s32Ret = MT_SUCCESS;
    MT_PQ_SCALER_S stScalerH, stScalerV;

    /* 找一个可用的系数地址 */
    for (i=0; i<VPSS_ZME_COEF_NUM; i++)
    {
        if (MT_FALSE == stHalCtx[enIP].abUsed[i])
        {
            stHalCtx[enIP].abUsed[i] = MT_TRUE;
            break;
        }
    }

    if(i>=VPSS_ZME_COEF_NUM)
    {
        VPSS_FATAL("No Enough Zme Coef Phy\n");
        return MT_FAILURE;
    }

    stScalerH.bHorizontal = MT_TRUE;
	stScalerH.bYUV        = pstZmeParam->bYUV;
    stScalerH.u32YRatio   = pstZmeParam->u32YHRatio;
    stScalerH.u32CRatio   = pstZmeParam->u32CHRatio;
    stScalerH.enInFmt     = pstZmeParam->enInFmt;
    stScalerH.enOutFmt    = pstZmeParam->enOutFmt;

    /*s32Ret = DRV_PQ_GetVpssScalerCoef(&stScalerH,
                (mt_void *)stHalCtx[enIP].au32ZmeCoefVir[i][0],
                (mt_void*)stHalCtx[enIP].au32ZmeCoefVir[i][1]);*///Rock_hu
    if(s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("H Coef Get Failed\n");
        return MT_FAILURE;
    }

    stScalerV.bHorizontal = MT_FALSE;
	stScalerV.bYUV        = pstZmeParam->bYUV;
    stScalerV.u32YRatio   = pstZmeParam->u32YVRatio;
    stScalerV.u32CRatio   = pstZmeParam->u32CVRatio;
    stScalerV.enInFmt     = pstZmeParam->enInFmt;
    stScalerV.enOutFmt    = pstZmeParam->enOutFmt;

    /*s32Ret = DRV_PQ_GetVpssScalerCoef(&stScalerV,
                (mt_void *)stHalCtx[enIP].au32ZmeCoefVir[i][2],
                (mt_void*)stHalCtx[enIP].au32ZmeCoefVir[i][3]);*///Rock_hu
    if(s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("V Coef Get Failed\n");
        return MT_FAILURE;
    }

    *pu32YH = stHalCtx[enIP].au32ZmeCoefPhy[i][0];
    *pu32CH = stHalCtx[enIP].au32ZmeCoefPhy[i][1];
    *pu32YV = stHalCtx[enIP].au32ZmeCoefPhy[i][2];
    *pu32CV = stHalCtx[enIP].au32ZmeCoefPhy[i][3];

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_SetDnrCfg(VPSS_IP_E enIP,mt_u32 u32AppVir,
                            VPSS_HAL_INFO_S *pstHalInfo)
{
    VPSS_HAL_FRAME_S *pstInInfo;
    MT_BOOL bdbEnHort;
    MT_BOOL bdbEnVert;
    MT_BOOL bdrEn;
    MT_BOOL bdbEn;

    pstInInfo = &(pstHalInfo->stInInfo);

    if(pstInInfo->u32Height <= 576 && pstInInfo->u32Width <= 720)
    {
        if (((pstInInfo->enFormat != MT_DRV_PIX_FMT_NV21)
                && (pstInInfo->enFormat != MT_DRV_PIX_FMT_NV12)
                && (pstInInfo->enFormat != MT_DRV_PIX_FMT_NV21_TILE)
                && (pstInInfo->enFormat != MT_DRV_PIX_FMT_NV12_TILE)
                && (pstInInfo->enFormat != MT_DRV_PIX_FMT_NV12_TILE_CMP)
                && (pstInInfo->enFormat != MT_DRV_PIX_FMT_NV21_TILE_CMP))
            //||pstInst->stProcCtrl.bUseCropRect == MT_TRUE
            //|| pstAlgCfg->stAuTunnelCfg.stDeiCfg.bDei == MT_FALSE
            //|| ((pstInst->stProcCtrl.bUseCropRect == MT_FALSE)
            //    && (pstInst->stProcCtrl.stInRect.s32Height != 0
            //        || pstInst->stProcCtrl.stInRect.s32Width != 0
            //        || pstInst->stProcCtrl.stInRect.s32X != 0
            //        || pstInst->stProcCtrl.stInRect.s32Y != 0))
            || ((pstInInfo->u32Height & 0x0000000f) != 0x0)
            || ((pstInInfo->u32Width  & 0x0000000f) != 0x0)
			//|| pstInst->u32Rwzb > 0
		    )
        {
            bdbEnHort = 0;
            bdbEnVert = 0;
            bdrEn = 0;
            bdbEn = 0;
        }
        else
        {
            bdbEnHort = 1;
            bdbEnVert = 1;
            bdrEn = 1;
            bdbEn = 1;
        }
    }
    else
    {
        bdbEnHort = 0;
        bdbEnVert = 0;
        bdrEn = 0;
        bdbEn = 0;
    }

    VPSS_REG_SetDREn(u32AppVir, bdrEn);
    VPSS_REG_SetDBEn(u32AppVir,bdbEn);
}
mt_s32 VPSS_HAL_SetZmeCfg(VPSS_IP_E enIP,mt_u32 u32AppVir,
                            VPSS_HAL_INFO_S *pstHalInfo,
                            VPSS_REG_PORT_E enPort,
                            mt_u32 u32PortCnt)
{
    VPSS_HAL_PORT_INFO_S *pstHalPort = MT_NULL;
    MT_PQ_ZME_PARA_IN_S stZmeDrvPara;
    MT_PQ_ZME_PARA_IN_S *pstZmeDrvPara;
    VPSS_HAL_FRAME_S *pstInInfo;
    MT_PQ_VPSS_ZME_LAYER_E enPqPort;


    pstHalPort = &pstHalInfo->astPortInfo[u32PortCnt];

    pstInInfo = &(pstHalInfo->stInInfo);

    pstZmeDrvPara  = &(stZmeDrvPara);

    memset(pstZmeDrvPara,0,sizeof(MT_PQ_ZME_PARA_IN_S));

    if (pstInInfo->bProgressive != MT_TRUE)
    {
        pstZmeDrvPara->bZmeFrmFmtIn = 1;
    }
    else
    {
        if (pstInInfo->enFieldMode == MT_DRV_FIELD_ALL)
        {
            pstZmeDrvPara->bZmeFrmFmtIn = 1;
        }
        else
        {
            pstZmeDrvPara->bZmeFrmFmtIn = 0;
        }
    }
    pstZmeDrvPara->bZmeFrmFmtOut = 1;

    if (pstInInfo->enFieldMode == MT_DRV_FIELD_TOP)
    {
        pstZmeDrvPara->bZmeBFIn = 0;
    }
    else if (pstInInfo->enFieldMode == MT_DRV_FIELD_BOTTOM)
    {
        pstZmeDrvPara->bZmeBFIn = 1;
    }
    else
    {
        pstZmeDrvPara->bZmeBFIn = 0;
    }
    pstZmeDrvPara->bZmeBFOut = 0;

    if (pstInInfo->enFormat == MT_DRV_PIX_FMT_NV21_CMP
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV12_CMP
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV21
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV12
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV21_TILE
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV12_TILE
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV21_TILE_CMP
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV12_TILE_CMP
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV61
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_NV16
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_YUV422_1X2
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_YUV420p
        || pstInInfo->enFormat == MT_DRV_PIX_FMT_YUV410p
        )
    {
        pstZmeDrvPara->u8ZmeYCFmtIn = 0;//PQ_ALG_ZME_PIX_FORMAT_SP420;
    }
    else
    {
        pstZmeDrvPara->u8ZmeYCFmtIn = 1;//PQ_ALG_ZME_PIX_FORMAT_SP422;
    }

    if (pstHalPort->stOutInfo.enFormat == MT_DRV_PIX_FMT_NV21
        || pstHalPort->stOutInfo.enFormat == MT_DRV_PIX_FMT_NV12
        || pstHalPort->stOutInfo.enFormat == MT_DRV_PIX_FMT_NV21_CMP
        || pstHalPort->stOutInfo.enFormat == MT_DRV_PIX_FMT_NV12_CMP)
    {
        pstZmeDrvPara->u8ZmeYCFmtOut = 0;//PQ_ALG_ZME_PIX_FORMAT_SP420;
    }
    else
    {
        pstZmeDrvPara->u8ZmeYCFmtOut = 1;//PQ_ALG_ZME_PIX_FORMAT_SP422;
    }

    pstZmeDrvPara->u32ZmeFrmHIn = pstInInfo->u32Height;
    pstZmeDrvPara->u32ZmeFrmWIn = pstInInfo->u32Width;

    if (pstInInfo->bProgressive == MT_TRUE)
    {
        if (pstInInfo->enFieldMode != MT_DRV_FIELD_ALL)
        {
            pstZmeDrvPara->u32ZmeFrmHIn = pstZmeDrvPara->u32ZmeFrmHIn*2;
        }
    }

    pstZmeDrvPara->u32ZmeFrmHOut = pstHalPort->stVideoRect.s32Height;
    pstZmeDrvPara->u32ZmeFrmWOut = pstHalPort->stVideoRect.s32Width;

    pstZmeDrvPara->stOriRect.s32X = 0;
    pstZmeDrvPara->stOriRect.s32Y = 0;
    pstZmeDrvPara->stOriRect.s32Width = pstInInfo->u32Width;
    pstZmeDrvPara->stOriRect.s32Height = pstInInfo->u32Height;
    pstZmeDrvPara->u32InRate = 25000;
    pstZmeDrvPara->u32OutRate = 25000;
    pstZmeDrvPara->bDispProgressive = MT_TRUE;
    pstZmeDrvPara->u32Fidelity = 0;

    switch(enPort)
    {
        case VPSS_REG_HD:
            enPqPort = MT_PQ_VPSS_PORT0_LAYER_ZME;
            break;
        case VPSS_REG_SD:
            enPqPort = MT_PQ_VPSS_PORT1_LAYER_ZME;
            break;
        case VPSS_REG_STR:
            enPqPort = MT_PQ_VPSS_PORT2_LAYER_ZME;
            break;
        default:
            break;
    }
    DRV_PQ_SetVpssZme(enPqPort,
                      (VPSS_REG_S*)u32AppVir,
                      pstZmeDrvPara,
                      MT_TRUE);

}
/*
*  VPSS_HAL_SetPortCfg函数应该只需要传 VPSS_HAL_PORT_INFO_S数组信息即可，
*  但由于缩放需要源格式 所以传递了VPSS_HAL_INFO_S信息，也同时为后期预缩放场景提供扩展
*/
mt_s32 VPSS_HAL_SetPortCfg(VPSS_IP_E enIP,mt_u32 u32AppVir,
    VPSS_HAL_INFO_S *pstHalInfo)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u32 u32Count;
    MT_BOOL abPortUsed[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER]
        = {MT_FALSE, MT_FALSE, MT_FALSE};

    /* 先默认关闭三个硬件输出通道 */
    VPSS_REG_EnPort(u32AppVir, VPSS_REG_HD, MT_FALSE);
    VPSS_REG_EnPort(u32AppVir, VPSS_REG_STR, MT_FALSE);
    VPSS_REG_EnPort(u32AppVir, VPSS_REG_SD,  MT_FALSE);
    VPSS_REG_SetFidelity(u32AppVir, MT_FALSE);

    for (u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count ++)
    {
        VPSS_HAL_PORT_INFO_S *pstHalPort = MT_NULL;

        pstHalPort = &pstHalInfo->astPortInfo[u32Count];
        if (pstHalPort->bEnable)
        {
            mt_u32 u32Zmeiw, u32Zmeih, u32Zmeow, u32Zmeoh;
            mt_u32 u32CoefYH = 0, u32CoefCH = 0, u32CoefYV = 0, u32CoefCV = 0;
            VPSS_HAL_ZME_PARAM_S stZmeParam;
            VPSS_REG_PORT_E enPort = VPSS_REG_BUTT;
            VPSS_HAL_FRAME_S *pstOutFrm = MT_NULL;
            MT_DRV_VID_FRAME_ADDR_S *pstOutAddr = MT_NULL;

            enPort = VPSS_HAL_AllocPortId(pstHalPort, abPortUsed);
            if (enPort == VPSS_REG_BUTT)
            {
                MT_ASSERT(0);
            }

            pstOutFrm = &pstHalPort->stOutInfo;
            pstOutAddr = &pstOutFrm->stAddr;

            /* Flip&Mirro */
            VPSS_REG_SetPortMirrorEn(u32AppVir, enPort,pstHalPort->bNeedMirror);  //暂时未考虑旋转
            VPSS_REG_SetPortFlipEn(u32AppVir, enPort,pstHalPort->bNeedFlip);    //暂时未考虑旋转

            /* UV反转 */


            /* PreZme */
            //VPSS_REG_SetFrmPreZmeEn(u32AppVir, enPort, MT_FALSE, MT_FALSE);

            /*ZME*/
            VPSS_HAL_SetZmeCfg(enIP,u32AppVir,
                            pstHalInfo,
                            enPort,
                            u32Count);
//            VPSS_REG_SetPortZmeEn(u32AppVir, enPort, MT_FALSE);
            /* LBX */
            VPSS_REG_SetLBABg(u32AppVir, enPort, 0x108080, 0x7f);
            VPSS_REG_SetLBAVidPos(u32AppVir, enPort,
                        (mt_u32)pstHalPort->stVideoRect.s32X,
                        (mt_u32)pstHalPort->stVideoRect.s32Y,
                        pstHalPort->stVideoRect.s32Height,
                        pstHalPort->stVideoRect.s32Width);
            VPSS_REG_SetLBADispPos(u32AppVir, enPort, 0, 0,
                pstOutFrm->u32Height, pstOutFrm->u32Width);
            VPSS_REG_SetLBAEn(u32AppVir, enPort, MT_TRUE);

            /* 输出格式 */
            VPSS_REG_SetFrmSize(u32AppVir,enPort,pstOutFrm->u32Height, pstOutFrm->u32Width);
            VPSS_REG_SetFrmAddr(u32AppVir,enPort,pstOutAddr->u32PhyAddr_Y, pstOutAddr->u32PhyAddr_C);
            VPSS_REG_SetFrmStride(u32AppVir,enPort,pstOutAddr->u32Stride_Y,pstOutAddr->u32Stride_C);
            VPSS_REG_SetFrmFormat(u32AppVir,enPort,pstOutFrm->enFormat);
            //VPSS_REG_SetFrmBitWidth(u32AppVir,enPort,pstOutFrm->enBitWidth);

            VPSS_REG_EnPort(u32AppVir, enPort, MT_TRUE);
        }
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_SetAllAlgCfgAddr(mt_u32 u32AppVir, mt_u32 u32AppPhy)
{
    #if 0
    VPSS_REG_SetVc1StrCfgAddr(u32AppVir, u32AppPhy+0x2400);
    VPSS_REG_SetZmeCfgAddr(u32AppVir, u32AppPhy+0x2000);
    VPSS_REG_SetHspCfgAddr(u32AppVir, u32AppPhy+0x2100);
    VPSS_REG_SetSnrCfgAddr(u32AppVir, u32AppPhy+0x3000);
    VPSS_REG_SetDbCfgAddr(u32AppVir, u32AppPhy+0x2200);
    VPSS_REG_SetDrCfgAddr(u32AppVir, u32AppPhy+0x2300);
    VPSS_REG_SetDeiCfgAddr(u32AppVir, u32AppPhy+0x1000);
    VPSS_REG_SetTnrCfgAddr(u32AppVir, u32AppPhy+0x3800);
    VPSS_REG_SetTnrClutCfgAddr(u32AppVir, u32AppPhy+0x3b00);
    VPSS_REG_SetEsCfgAddr(u32AppVir, u32AppPhy+0x2500);
    #endif
    return MT_SUCCESS;
}
mt_s32 VPSS_HAL_SetRwzbCfg(mt_u32 u32AppAddr,VPSS_RWZB_INFO_S *pstRwzbInfo)
{
    mt_u32 u32Count;

    if( pstRwzbInfo->u32EnRwzb == 0x1)
    {
        for(u32Count = 0; u32Count < 6; u32Count++)
        {
            VPSS_REG_SetDetBlk(u32AppAddr,u32Count,&(pstRwzbInfo->u32Addr[u32Count][0]));
                #if 1
                VPSS_INFO("adddr X %d Y %d\n",pstRwzbInfo->u32Addr[u32Count][0],
                                pstRwzbInfo->u32Addr[u32Count][1]);
                                #endif
        }

    }
    VPSS_REG_SetDetEn(u32AppAddr,pstRwzbInfo->u32EnRwzb);
    VPSS_REG_SetDetMode(u32AppAddr,pstRwzbInfo->u32Mode);

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_SetFieldNode(VPSS_IP_E enIP, VPSS_HAL_INFO_S *pstHalInfo,
    mt_u32 u32AppVir)
{
    MT_DRV_VID_FRAME_ADDR_S *pstCur = MT_NULL;
    MT_DRV_VID_FRAME_ADDR_S *pstRef = MT_NULL;
    MT_DRV_VID_FRAME_ADDR_S *pstRfr = MT_NULL;
    VPSS_NRMADCFG_S *pstNrMad = MT_NULL;

    VPSS_REG_ResetAppReg(u32AppVir, pstHalInfo->pstPqCfg);

    /*tunnel*/
    if (pstHalInfo->stInInfo.u32TunnelAddr)
    {
        VPSS_REG_SetCurTunlEn(u32AppVir, MT_TRUE);
        VPSS_REG_SetCurTunlAddr(u32AppVir,
                                CUR_FRAME,
                                pstHalInfo->stInInfo.u32TunnelAddr);
    }
    else
    {
        VPSS_REG_SetCurTunlEn(u32AppVir, MT_FALSE);
    }
    /*rwzb*/
    #if 0
    VPSS_HAL_SetRwzbCfg(u32AppVir, &(pstHalInfo->stRwzbInfo));
    #endif

    /* 输入源信息 */
    VPSS_REG_SetInCropEn(u32AppVir, MT_FALSE);
    //VPSS_REG_SetImgBitWidth(u32AppVir, pstHalInfo->stInInfo.enBitWidth);
    VPSS_REG_SetImgFormat(u32AppVir, pstHalInfo->stInInfo.enFormat);
    VPSS_REG_SetImgSize(u32AppVir,
        pstHalInfo->stInInfo.u32Width, pstHalInfo->stInInfo.u32Height,
        pstHalInfo->stInInfo.bProgressive);

    /* 帧信息配置 */
    pstCur = &pstHalInfo->stInInfo.stAddr;
    pstRef = &pstHalInfo->stInRefInfo[0].stAddr;
    pstRfr = &pstHalInfo->stInWbcInfo.stAddr;

    /* 输入帧信息 */
    VPSS_REG_SetImgAddr(u32AppVir,CUR_FIELD,
                        pstCur->u32PhyAddr_Y,
                        pstCur->u32PhyAddr_C,
                        0);
    VPSS_REG_SetImgStride(u32AppVir,CUR_FIELD, pstCur->u32Stride_Y, pstCur->u32Stride_C);


    /*db/dr*/
    VPSS_HAL_SetDnrCfg(enIP,u32AppVir,pstHalInfo);

    /* 输出Port信息 */
    VPSS_HAL_SetPortCfg(enIP, u32AppVir, pstHalInfo);

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_SetFrameNode(VPSS_IP_E enIP, VPSS_HAL_INFO_S *pstHalInfo,
    mt_u32 u32AppVir)
{
    MT_DRV_VID_FRAME_ADDR_S *pstCur = MT_NULL;
    VPSS_NRMADCFG_S *pstNrMad = MT_NULL;
    //mt_u32 u32Width;
    //mt_u32 u32Height;

    VPSS_REG_ResetAppReg(u32AppVir, pstHalInfo->pstPqCfg);

    /*tunnel*/
    if (pstHalInfo->stInInfo.u32TunnelAddr)
    {
        VPSS_REG_SetCurTunlEn(u32AppVir, MT_TRUE);
        VPSS_REG_SetCurTunlAddr(u32AppVir,
                                CUR_FRAME,
                                pstHalInfo->stInInfo.u32TunnelAddr);
    }
    else
    {
        VPSS_REG_SetCurTunlEn(u32AppVir, MT_FALSE);
    }

    #if 0
	/*rwzb*/
    VPSS_HAL_SetRwzbCfg(u32AppVir, &(pstHalInfo->stRwzbInfo));
    #endif

    /* 输入源信息 */
    VPSS_REG_SetInCropEn(u32AppVir, MT_FALSE);
    //VPSS_REG_SetImgBitWidth(u32AppVir, pstHalInfo->stInInfo.enBitWidth);
    VPSS_REG_SetImgFormat(u32AppVir, pstHalInfo->stInInfo.enFormat);


    pstCur = &pstHalInfo->stInInfo.stAddr;
	#if 1
    if (pstHalInfo->stInInfo.enFieldMode == MT_DRV_FIELD_TOP
        || pstHalInfo->stInInfo.enFieldMode == MT_DRV_FIELD_BOTTOM)
    {
        pstHalInfo->stInInfo.u32Height =
                    pstHalInfo->stInInfo.u32Height/2;
        pstCur->u32Stride_Y = pstCur->u32Stride_Y*2;
        pstCur->u32Stride_C = pstCur->u32Stride_C*2;
        if (pstHalInfo->stInInfo.enFieldMode == MT_DRV_FIELD_BOTTOM)
        {
            pstCur->u32PhyAddr_Y = pstCur->u32PhyAddr_Y + pstCur->u32Stride_Y;
            pstCur->u32PhyAddr_C = pstCur->u32PhyAddr_C+ pstCur->u32Stride_C;
        }
    }

    #endif
    VPSS_REG_SetImgSize(u32AppVir,
        pstHalInfo->stInInfo.u32Width, pstHalInfo->stInInfo.u32Height,
        pstHalInfo->stInInfo.bProgressive);

    /* 帧信息配置 */

    /* 输入帧信息 */
    VPSS_REG_SetImgAddr(u32AppVir,CUR_FIELD,
                        pstCur->u32PhyAddr_Y,
                        pstCur->u32PhyAddr_C,
                        0);

    VPSS_REG_SetImgStride(u32AppVir,CUR_FIELD, pstCur->u32Stride_Y, pstCur->u32Stride_C);


    /*db/dr*/
    VPSS_REG_SetDREn(u32AppVir, MT_FALSE);
    VPSS_REG_SetDBEn(u32AppVir, MT_FALSE);

    /* 输出Port信息 */
    VPSS_HAL_SetPortCfg(enIP, u32AppVir, pstHalInfo);

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_Set5FieldNode(VPSS_IP_E enIP, VPSS_HAL_INFO_S *pstHalInfo,
    mt_u32 u32AppVir,mt_u32 u32AppPhy)
{
    mt_u32  VPSS_REFYADDR;
    mt_u32  VPSS_REFCADDR;
    mt_u32  VPSS_REFCRADDR;

    mt_u32  VPSS_NEXT1YADDR;
    mt_u32  VPSS_NEXT1CADDR;
    mt_u32  VPSS_NEXT1CRADDR;

    mt_u32  VPSS_NEXT3YADDR;
    mt_u32  VPSS_NEXT3CADDR;
    mt_u32  VPSS_NEXT3CRADDR;

    mt_u32  VPSS_DEI_ADDR;

    MT_DRV_VID_FRAME_ADDR_S *pstCur = MT_NULL;


    VPSS_REG_ResetAppReg(u32AppVir, pstHalInfo->pstPqCfg);

    /* 输入源信息 */
    VPSS_REG_SetInCropEn(u32AppVir, MT_FALSE);
    //VPSS_REG_SetImgBitWidth(u32AppVir, pstHalInfo->stInInfo.enBitWidth);
    VPSS_REG_SetImgFormat(u32AppVir, pstHalInfo->stInInfo.enFormat);
    VPSS_REG_SetImgSize(u32AppVir,
        pstHalInfo->stInInfo.u32Width, pstHalInfo->stInInfo.u32Height,
        pstHalInfo->stInInfo.bProgressive);

    /* 帧信息配置 */
    pstCur = &pstHalInfo->stInInfo.stAddr;

    /* 输入帧信息 */
    VPSS_REG_SetImgAddr(u32AppVir,CUR_FIELD,
                        pstCur->u32PhyAddr_Y,
                        pstCur->u32PhyAddr_C,
                        0);
    VPSS_REG_SetImgStride(u32AppVir,CUR_FIELD, pstCur->u32Stride_Y, pstCur->u32Stride_C);


    /*db/dr*/
    VPSS_HAL_SetDnrCfg(enIP,u32AppVir,pstHalInfo);

    /*RWZB*/
    VPSS_HAL_SetRwzbCfg(u32AppVir, &(pstHalInfo->stRwzbInfo));

    /*dei*/
    VPSS_REG_SetImgReadMod(u32AppVir, MT_TRUE);
    VPSS_REG_EnDei(u32AppVir, MT_TRUE);
    VPSS_REG_SetDeiTopFirst(u32AppVir, pstHalInfo->stInInfo.bTopFirst);

    switch(pstHalInfo->stInInfo.enFieldMode)
    {
        case MT_DRV_FIELD_BOTTOM:
            VPSS_REG_SetDeiFieldMode(u32AppVir, MT_TRUE);
            break;
        case MT_DRV_FIELD_TOP:
            VPSS_REG_SetDeiFieldMode(u32AppVir, MT_FALSE);
            break;
        default:
            VPSS_FATAL("No spt field Type:%d!\n", pstHalInfo->stInInfo.enFieldMode);
            return MT_FAILURE;
    }
    VPSS_REG_SetModeEn(u32AppVir,REG_DIE_MODE_CHROME,MT_TRUE);
    VPSS_REG_SetModeEn(u32AppVir,REG_DIE_MODE_LUMA,MT_TRUE);

    #if 1
    VPSS_REG_SetStWrAddr(u32AppVir,pstHalInfo->stHisAddr.u32WPhyAddr);
    VPSS_REG_SetStRdAddr(u32AppVir,
                        pstHalInfo->stHisAddr.u32RPhyAddr);
    VPSS_REG_SetStStride(u32AppVir,
                        pstHalInfo->stHisAddr.u32Stride);

    //mt_s32 VPSS_REG_SetDeiParaAddr(mt_u32 u32ParaAddr);
    VPSS_DEI_ADDR = u32AppPhy + VPSS_REG_SIZE_CALC(VPSS_CTRL,VPSS_DIECTRL)-sizeof(mt_u32);
    VPSS_REG_SetDeiParaAddr(u32AppVir,
                        VPSS_DEI_ADDR);
    #endif
    VPSS_REG_SetMode(u32AppVir, REG_DIE_MODE_ALL, 0);

    VPSS_REFYADDR = pstHalInfo->stFieldAddr[0].u32PhyAddr_Y;
    VPSS_REFCADDR = pstHalInfo->stFieldAddr[0].u32PhyAddr_C;
    VPSS_REFCRADDR = pstHalInfo->stFieldAddr[0].u32PhyAddr_Cr;
    VPSS_REG_SetDeiAddr(u32AppVir,LAST_FIELD,VPSS_REFYADDR,VPSS_REFCADDR, VPSS_REFCRADDR);
    VPSS_REG_SetDeiStride(u32AppVir,LAST_FIELD,
                            pstHalInfo->stFieldAddr[0].u32Stride_Y,
                            pstHalInfo->stFieldAddr[0].u32Stride_C);

    /*********/

    /*********/
    VPSS_NEXT1YADDR = pstHalInfo->stFieldAddr[3].u32PhyAddr_Y;
    VPSS_NEXT1CADDR = pstHalInfo->stFieldAddr[3].u32PhyAddr_C;
    VPSS_NEXT1CRADDR = pstHalInfo->stFieldAddr[3].u32PhyAddr_Cr;
    VPSS_REG_SetDeiAddr(u32AppVir,CUR_FIELD,VPSS_NEXT1YADDR,VPSS_NEXT1CADDR, VPSS_NEXT1CRADDR);
    VPSS_REG_SetDeiStride(u32AppVir,CUR_FIELD,
                            pstHalInfo->stFieldAddr[3].u32Stride_Y,
                            pstHalInfo->stFieldAddr[3].u32Stride_C);

    /*********/

    /*********/
    VPSS_NEXT3YADDR = pstHalInfo->stFieldAddr[5].u32PhyAddr_Y;
    VPSS_NEXT3CADDR = pstHalInfo->stFieldAddr[5].u32PhyAddr_C;
    VPSS_NEXT3CRADDR = pstHalInfo->stFieldAddr[5].u32PhyAddr_Cr;
    VPSS_REG_SetDeiAddr(u32AppVir,NEXT_FRAME,VPSS_NEXT3YADDR,VPSS_NEXT3CADDR, VPSS_NEXT3CRADDR);
    VPSS_REG_SetDeiStride(u32AppVir,NEXT_FRAME,
                            pstHalInfo->stFieldAddr[5].u32Stride_Y,
                            pstHalInfo->stFieldAddr[5].u32Stride_C);


    /* 输出Port信息 */
    VPSS_HAL_SetPortCfg(enIP, u32AppVir, pstHalInfo);
    #if 0
    /* 填写Die默认值 */
    //VPSS_REG_WriteDei(u32AppVir);

    /* 需要强制设置的模式和开关 */




    //VPSS_REG_SetDeiOutSel(u32AppVir, REG_DIE_MODE_ALL, MT_FALSE);/* 暂时使用间插模式 */

    VPSS_REG_SetCcclEn(u32AppVir, pstHalInfo->stCCCLInfo.bCCCLEn);
    VPSS_REG_SetCcclMode(u32AppVir, 0x1);

    /* 需要根据策略制定的开关 */
    VPSS_REG_SetStrDetEn(u32AppVir, MT_FALSE);/* 暂时关闭码流检测功能 */
    //VPSS_REG_SetIglbEn(u32AppVir, MT_TRUE);/* 暂时关闭iGLB功模,开关控制有PQ模块负责 */
    VPSS_REG_SetIFmdEn(u32AppVir, MT_TRUE);/* 暂时关闭iFMD功能 */
    VPSS_REG_SetUVConvertEn(u32AppVir, MT_FALSE);/* 暂时不反转 */
    VPSS_REG_SetSCDEn(u32AppVir, MT_TRUE);/* 暂时开启SCD */
	VPSS_REG_SetSCDPixy(u32AppVir,12);	/*SCD设置12或16*/
    VPSS_REG_SetSCDTnr(u32AppVir,pstHalInfo->u32ScdValue);    /*设置TNR SCD*/
    VPSS_REG_SetSCDSnr(u32AppVir,pstHalInfo->u32ScdValue);    /*设置SNR SCD*/
    //VPSS_REG_SetPdDebugEn(u32AppVir, MT_FALSE);/* 暂时关闭PdDebug,开关控制有PQ模块负责 */
    //VPSS_REG_SetPdMode(u32AppVir, 0);/* 暂时设置为0 ,开关控制有PQ模块负责*/
    VPSS_REG_SetFr0VC1En(u32AppVir, MT_FALSE);/* 暂时关闭VC1功能 */
    VPSS_REG_SetFr1VC1En(u32AppVir, MT_FALSE);/* 暂时关闭VC1功能 */
    VPSS_REG_SetFr2VC1En(u32AppVir, MT_FALSE);/* 暂时关闭VC1功能 */

    VPSS_REG_SetRfrEn(u32AppVir, MT_TRUE);/* 暂时都把回写打开 */
    VPSS_REG_SetNrEn(u32AppVir, MT_TRUE);/* 暂时都把NR打开 */

    VPSS_REG_SetLBDCfg(u32AppVir,
        pstHalInfo->stInInfo.u32Width, pstHalInfo->stInInfo.u32Height, MT_TRUE);

    VPSS_HAL_SetRwzbCfg(u32AppVir, &(pstHalInfo->stRwzbInfo));

    /* 输入源信息 */
    VPSS_REG_SetInCropEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetImgBitWidth(u32AppVir, pstHalInfo->stInInfo.enBitWidth);
    VPSS_REG_SetImgFormat(u32AppVir, pstHalInfo->stInInfo.enFormat);
    VPSS_REG_SetImgSize(u32AppVir,
        pstHalInfo->stInInfo.u32Width, pstHalInfo->stInInfo.u32Height);

    /* 帧信息配置 */
    pstCur  = &pstHalInfo->stInRefInfo[1].stAddr;
    pstRef  = &pstHalInfo->stInRefInfo[0].stAddr;
    pstNxt1 = &pstHalInfo->stInRefInfo[2].stAddr;
    pstNxt2 = &pstHalInfo->stInRefInfo[3].stAddr;
    pstNxt3 = &pstHalInfo->stInInfo.stAddr;
    pstRfr  = &pstHalInfo->stInWbcInfo.stAddr;

    pstPr1 = &pstHalInfo->stCCCLInfo.stInRefInfo[0].stAddr;
    pstPr2 = &pstHalInfo->stCCCLInfo.stInRefInfo[1].stAddr;

    /* Cur帧信息 */
    VPSS_REG_SetCurXYOffset(u32AppVir, 0, 0);//无偏移
    VPSS_REG_SetCurFmt(u32AppVir, pstHalInfo->stInRefInfo[1].enFormat);
    VPSS_REG_SetCurHeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetCurHeadAddr(u32AppVir, pstCur->u32PhyAddr_YHead, pstCur->u32PhyAddr_CHead);
    VPSS_REG_SetCurAddr(u32AppVir, pstCur->u32PhyAddr_Y,pstCur->u32PhyAddr_C);
    VPSS_REG_SetCurStride(u32AppVir, pstCur->u32Stride_Y, pstCur->u32Stride_C);

    /* 参考帧信息 */
    VPSS_REG_SetRefXYOffset(u32AppVir, 0, 0);
    VPSS_REG_SetRefFmt(u32AppVir, pstHalInfo->stInRefInfo[0].enFormat);
    VPSS_REG_SetRefHeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetRefHeadAddr(u32AppVir, pstRef->u32PhyAddr_YHead, pstRef->u32PhyAddr_CHead);
    VPSS_REG_SetRefAddr(u32AppVir, pstRef->u32PhyAddr_Y,pstRef->u32PhyAddr_C);
    VPSS_REG_SetRefStride(u32AppVir, pstRef->u32Stride_Y, pstRef->u32Stride_C);

    /* NEXT1 */
    VPSS_REG_SetNxt1XYOffset(u32AppVir, 0, 0);
    VPSS_REG_SetNxt1Fmt(u32AppVir, pstHalInfo->stInRefInfo[2].enFormat);
    VPSS_REG_SetNxt1HeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetNxt1HeadAddr(u32AppVir, pstNxt1->u32PhyAddr_YHead, pstNxt1->u32PhyAddr_CHead);
    VPSS_REG_SetNxt1Addr(u32AppVir, pstNxt1->u32PhyAddr_Y,pstNxt1->u32PhyAddr_C);
    VPSS_REG_SetNxt1Stride(u32AppVir, pstNxt1->u32Stride_Y, pstNxt1->u32Stride_C);

    /* NEXT2 */
    VPSS_REG_SetNxt2XYOffset(u32AppVir, 0, 0);
    VPSS_REG_SetNxt2Fmt(u32AppVir, pstHalInfo->stInRefInfo[3].enFormat);
    VPSS_REG_SetNxt2HeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetNxt2HeadAddr(u32AppVir, pstNxt2->u32PhyAddr_YHead, pstNxt2->u32PhyAddr_CHead);
    VPSS_REG_SetNxt2Addr(u32AppVir, pstNxt2->u32PhyAddr_Y,pstNxt2->u32PhyAddr_C);
    VPSS_REG_SetNxt2Stride(u32AppVir, pstNxt2->u32Stride_Y, pstNxt2->u32Stride_C);

    /* NEXT3 */
    VPSS_REG_SetNxt3XYOffset(u32AppVir, 0, 0);
    VPSS_REG_SetNxt3Fmt(u32AppVir, pstHalInfo->stInInfo.enFormat);
    VPSS_REG_SetNxt3HeadMode(u32AppVir, 1, 1, 1);//暂时先按356bit对齐,目前用不到压缩
    VPSS_REG_SetNxt3HeadAddr(u32AppVir, pstNxt3->u32PhyAddr_YHead, pstNxt3->u32PhyAddr_CHead);
    VPSS_REG_SetNxt3Addr(u32AppVir, pstNxt3->u32PhyAddr_Y,pstNxt3->u32PhyAddr_C);
    VPSS_REG_SetNxt3Stride(u32AppVir, pstNxt3->u32Stride_Y, pstNxt3->u32Stride_C);

    /* PR1 */
    VPSS_REG_SetPr1XYOffset(u32AppVir, 0, 0);
    VPSS_REG_SetPr1Fmt(u32AppVir, pstHalInfo->stCCCLInfo.stInRefInfo[0].enFormat);
    VPSS_REG_SetPr1HeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetPr1HeadAddr(u32AppVir, pstPr1->u32PhyAddr_YHead, pstPr1->u32PhyAddr_CHead);
    VPSS_REG_SetPr1Addr(u32AppVir, pstPr1->u32PhyAddr_Y,pstPr1->u32PhyAddr_C);
    VPSS_REG_SetPr1Stride(u32AppVir, pstPr1->u32Stride_Y, pstPr1->u32Stride_C);

    /* PR2 */
    VPSS_REG_SetPr2XYOffset(u32AppVir, 0, 0);
    VPSS_REG_SetPr2Fmt(u32AppVir, pstHalInfo->stCCCLInfo.stInRefInfo[1].enFormat);
    VPSS_REG_SetPr2HeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetPr2HeadAddr(u32AppVir, pstPr2->u32PhyAddr_YHead, pstPr2->u32PhyAddr_CHead);
    VPSS_REG_SetPr2Addr(u32AppVir, pstPr2->u32PhyAddr_Y,pstPr2->u32PhyAddr_C);
    VPSS_REG_SetPr2Stride(u32AppVir, pstPr2->u32Stride_Y, pstPr2->u32Stride_C);

    /* 回写帧信息 */
    VPSS_REG_SetRfrDitherEn(u32AppVir, MT_FALSE); //默认关闭该功能
    VPSS_REG_SetRfrFmt(u32AppVir, pstHalInfo->stInWbcInfo.enFormat);
    VPSS_REG_SetRfrHeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetRfrHeadAddr(u32AppVir, pstRfr->u32PhyAddr_YHead, pstRfr->u32PhyAddr_CHead);
    VPSS_REG_SetRfrAddr(u32AppVir, pstRfr->u32PhyAddr_Y,pstRfr->u32PhyAddr_C);
    VPSS_REG_SetRfrStride(u32AppVir, pstRfr->u32Stride_Y, pstRfr->u32Stride_C);

    /* 运动信息 */
    pstCCCLCnt = &pstHalInfo->stCCCLInfo.stCCCLCntCfg;
    VPSS_REG_SetCcclYCnt(u32AppVir, pstCCCLCnt->u32Ycnt_raddr,
        pstCCCLCnt->u32Ycnt_raddr, pstCCCLCnt->u32ycnt_stride);
    VPSS_REG_SetCcclCCnt(u32AppVir, pstCCCLCnt->u32Ccnt_raddr,
            pstCCCLCnt->u32Ccnt_raddr, pstCCCLCnt->u32Ccnt_stride);

    pstDeiSt = &pstHalInfo->stDieInfo.stDieStCfg;
    VPSS_REG_SetDeiSt(u32AppVir, pstDeiSt->u32PPreAddr, pstDeiSt->u32PreAddr,
        pstDeiSt->u32CurAddr, pstDeiSt->u32Stride);

    pstNrMad = &pstHalInfo->stNrInfo.stNrMadCfg;
    VPSS_REG_SetNrMad(u32AppVir, pstNrMad->u32Tnrmad_raddr, pstNrMad->u32Tnrmad_waddr,
        pstNrMad->u32Snrmad_raddr, pstNrMad->u32madstride);
    VPSS_REG_SetSttWaddr(u32AppVir, pstHalInfo->u32stt_w_phy_addr);

    #endif
    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_Set3DFrameNode(VPSS_IP_E enIP, VPSS_HAL_INFO_S *pstHalInfo,
    mt_u32 u32AppVir)
{
    mt_u32 u32W, u32H;

    (mt_void)VPSS_HAL_SetFrameNode(enIP, pstHalInfo, u32AppVir);
    #if 0
    /* 根据输入格式设置偏移 */
    switch(pstHalInfo->stInInfo.eFrmType)
    {
        case MT_DRV_FT_SBS:
            u32W = pstHalInfo->stInInfo.u32Width;
            VPSS_REG_SetCurXYOffset(u32AppVir, u32W, 0);
            break;

        case MT_DRV_FT_TAB:
            u32H = pstHalInfo->stInInfo.u32Height;
            VPSS_REG_SetCurXYOffset(u32AppVir, 0, u32H);
            break;

        default:
            VPSS_FATAL("eFrmType invaild\n", pstHalInfo->stInInfo.eFrmType);
            return MT_FAILURE;
    }
    #endif
    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_SetUHDNode(VPSS_IP_E enIP, VPSS_HAL_INFO_S *pstHalInfo,
    mt_u32 u32AppVir)
{
    #if 0
    MT_DRV_VID_FRAME_ADDR_S *pstCur = MT_NULL;

    VPSS_REG_ResetAppReg(u32AppVir, pstHalInfo->pstPqCfg);

    VPSS_REG_SetImgReadMod(u32AppVir, MT_FALSE);
    VPSS_REG_SetIglbEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetIFmdEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetRotateEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetRfrLbaEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetDeiEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetCcclEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetCcclMode(u32AppVir, 0x1);
    VPSS_REG_SetStrDetEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetPglbEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetPFmdEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetUVConvertEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetSCDEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetPdDebugEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetPdMode(u32AppVir, 0);
    VPSS_REG_SetFr0VC1En(u32AppVir, MT_FALSE);
    VPSS_REG_SetFr1VC1En(u32AppVir, MT_FALSE);
    VPSS_REG_SetFr2VC1En(u32AppVir, MT_FALSE);
    VPSS_REG_SetRfrEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetNrEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetDbDrEn(u32AppVir, MT_FALSE);

    VPSS_REG_SetLBDCfg(u32AppVir,
        pstHalInfo->stInInfo.u32Width, pstHalInfo->stInInfo.u32Height, MT_TRUE);

    /* 输入源信息 */
    VPSS_REG_SetInCropEn(u32AppVir, MT_FALSE);
    VPSS_REG_SetImgBitWidth(u32AppVir, pstHalInfo->stInInfo.enBitWidth);
    VPSS_REG_SetImgFormat(u32AppVir, pstHalInfo->stInInfo.enFormat);

    /* 帧信息配置 */
    pstCur = &pstHalInfo->stInInfo.stAddr;
    switch(pstHalInfo->enNodeType)
    {
        case VPSS_HAL_NODE_UHD_SPLIT_L:
            {
                mt_u32 u32ImgHeight,u32ImgWidth;
                u32ImgWidth = pstHalInfo->stInInfo.u32Width/2;
                u32ImgHeight = pstHalInfo->stInInfo.u32Height;
                VPSS_REG_SetImgSize(u32AppVir,
                    u32ImgWidth,u32ImgHeight);
                VPSS_REG_SetCurAddr(u32AppVir, pstCur->u32PhyAddr_Y,pstCur->u32PhyAddr_C);

                VPSS_REG_SetCurXYOffset(u32AppVir, 0, 0);//无偏移
            }
            break;
        case VPSS_HAL_NODE_UHD_SPLIT_R:
            {
                mt_u32 u32ImgHeight,u32ImgWidth;
                mt_u32 u32Yaddr = 0;
                mt_u32 u32Caddr = 0;
                mt_u32 u32Xoffset,u32Yoffset;
                #if 0
                if ((pstHalInfo->stInInfo.u32Width/2)%256 != 0)
                {
                    u32ImgWidth = (((pstHalInfo->stInInfo.u32Width/2)+255)/256)*256;
                }
                else
                {
                    u32ImgWidth = pstHalInfo->stInInfo.u32Width/2;
                }
                #endif

                u32ImgWidth = pstHalInfo->stInInfo.u32Width/2;

                u32Xoffset = u32ImgWidth;
                u32Yoffset = 0;


                u32ImgHeight = pstHalInfo->stInInfo.u32Height;

                VPSS_REG_SetImgSize(u32AppVir,
                    u32ImgWidth,u32ImgHeight);
                #if 0
                VPSS_OSAL_GetTileOffsetAddr(u32ImgWidth,0,
                                            &u32Yaddr,&u32Caddr,
                                            pstCur);
                #endif
                u32Yaddr = pstCur->u32PhyAddr_Y;
                u32Caddr = pstCur->u32PhyAddr_C;
                VPSS_REG_SetCurAddr(u32AppVir, u32Yaddr,u32Caddr);

                VPSS_REG_SetCurXYOffset(u32AppVir, u32Xoffset, u32Yoffset);//无偏移
            }
            break;
        case VPSS_HAL_NODE_UHD:
            {
                mt_u32 u32ImgHeight,u32ImgWidth;
                u32ImgWidth = pstHalInfo->stInInfo.u32Width;
                u32ImgHeight = pstHalInfo->stInInfo.u32Height;
                VPSS_REG_SetImgSize(u32AppVir,
                    u32ImgWidth, u32ImgHeight);
                VPSS_REG_SetCurAddr(u32AppVir, pstCur->u32PhyAddr_Y,pstCur->u32PhyAddr_C);

                VPSS_REG_SetCurXYOffset(u32AppVir, 0, 0);//无偏移
            }
        default:
            break;
    }
    /* 输入帧信息 */
    //VPSS_REG_SetCurXYOffset(u32AppVir, 0, 0);//无偏移

    VPSS_REG_SetCurFmt(u32AppVir, pstHalInfo->stInInfo.enFormat);
    VPSS_REG_SetCurHeadMode(u32AppVir, 1, 1, 1);//暂时先按256bit对齐,目前用不到压缩
    VPSS_REG_SetCurHeadAddr(u32AppVir, pstCur->u32PhyAddr_YHead, pstCur->u32PhyAddr_CHead);
     VPSS_REG_SetCurStride(u32AppVir, pstCur->u32Stride_Y, pstCur->u32Stride_C);

    /* 输出Port信息 */
    VPSS_HAL_SetPortCfg(enIP, u32AppVir, pstHalInfo);

    VPSS_REG_SetSttWaddr(u32AppVir, pstHalInfo->u32stt_w_phy_addr);
    #endif
    return MT_SUCCESS;
}
mt_s32 VPSS_HAL_SetRotateNode(VPSS_IP_E enIP, VPSS_HAL_INFO_S *pstHalInfo,
    mt_u32 u32AppVir, MT_BOOL bRotateY, mt_u32 u32PortId)
{
    #if 1
    MT_DRV_VPSS_ROTATION_E enRotation;
    MT_DRV_VID_FRAME_ADDR_S *pstCur = MT_NULL;
    VPSS_REG_PORT_E enPort = VPSS_REG_HD;
    VPSS_HAL_FRAME_S *pstOutFrm = MT_NULL;
    MT_DRV_VID_FRAME_ADDR_S *pstOutAddr = MT_NULL;
    mt_u32 u32Angle;

    VPSS_REG_ResetAppReg(u32AppVir, NULL);

     /* 输入源信息 */
    VPSS_REG_SetInCropEn(u32AppVir, MT_FALSE);
    //VPSS_REG_SetImgBitWidth(u32AppVir, pstHalInfo->stInInfo.enBitWidth);
    VPSS_REG_SetImgFormat(u32AppVir, pstHalInfo->stInInfo.enFormat);
    VPSS_REG_SetImgSize(u32AppVir,
        pstHalInfo->stInInfo.u32Width, pstHalInfo->stInInfo.u32Height,
        pstHalInfo->stInInfo.bProgressive);

    /* 输入帧配置 */
    pstCur = &pstHalInfo->stInInfo.stAddr;

    /* 输入帧信息 */
    VPSS_REG_SetImgAddr(u32AppVir,CUR_FIELD,
                        pstCur->u32PhyAddr_Y,
                        pstCur->u32PhyAddr_C,
                        0);
    VPSS_REG_SetImgStride(u32AppVir,CUR_FIELD, pstCur->u32Stride_Y, pstCur->u32Stride_C);

    /* 输出配置 */
    pstOutFrm = &pstHalInfo->astPortInfo[u32PortId].stOutInfo;
    pstOutAddr = &pstOutFrm->stAddr;
    VPSS_REG_SetFrmSize(u32AppVir,enPort,pstOutFrm->u32Height, pstOutFrm->u32Width);
    VPSS_REG_SetFrmAddr(u32AppVir,enPort,pstOutAddr->u32PhyAddr_Y, pstOutAddr->u32PhyAddr_C);
    VPSS_REG_SetFrmStride(u32AppVir,enPort,pstOutAddr->u32Stride_Y,pstOutAddr->u32Stride_C);
    VPSS_REG_SetFrmFormat(u32AppVir,enPort,pstOutFrm->enFormat);

    VPSS_REG_EnPort(u32AppVir, enPort, MT_TRUE);

    /* 旋转配置 */
    enRotation = pstHalInfo->astPortInfo[u32PortId].enRotation;
    switch(enRotation)
    {
        case MT_DRV_VPSS_ROTATION_90:
            u32Angle = 0x0;
            break;
        case MT_DRV_VPSS_ROTATION_270:
            u32Angle = 0x1;
            break;
        default:
            VPSS_FATAL("Ro Error  %d\n",u32Angle);
            break;
    }
    VPSS_REG_SetRotation(u32AppVir,u32Angle);
    #endif
    return MT_SUCCESS;
}

mt_void VPSS_HAL_DumpReg(VPSS_IP_E enIP, VPSS_HAL_TASK_NODE_E enTaskNodeId)
{
    mt_u32 i;
    mt_u32 *pu32Reg;
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;

    pstHalCtx = &stHalCtx[enIP];

    pu32Reg = (mt_u32 *)(pstHalCtx->au32AppVir[enTaskNodeId]);

    printk("\n\nPhyAddr %8x   enTaskNodeId = %d\n\n",
        pstHalCtx->au32AppPhy[enTaskNodeId], enTaskNodeId);

    for(i=0; i<(64*4); i++)
    {
        if(i%(64) == 0)
        {
            printk("\nBASE %x", i*4);
        }

        if(i%4 == 0)
        {
            printk("\n%x0:", (i%64)/4);
        }

        printk("%.8x  ", *(pu32Reg+i));
    }

    printk("\n\n");

    for(i=1024; i<1024+64; i++)
    {
        if(i%(64) == 0)
        {
            printk("\nBASE %x", i*4);
        }

        if(i%4 == 0)
        {
            printk("\n%x0:", (i%64)/4);
        }

        printk("%.8x  ", *(pu32Reg+i));
    }

    printk("\n\n");

    for(i=2048; i<2048+64; i++)
    {
        if(i%(64) == 0)
        {
            printk("\nBASE %x", i*4);
        }

        if(i%4 == 0)
        {
            printk("\n%x0:", (i%64)/4);
        }

        printk("%.8x  ", *(pu32Reg+i));
    }

    printk("\n\n");

}

#if 0
/* 以上为内部调用函数 */
/* 以下为外部调用函数 */
#endif

mt_s32 VPSS_HAL_Init(VPSS_IP_E enIP)
{
    mt_u32 i, j;
    mt_s32 s32Ret = MT_SUCCESS;
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;

    VPSS_HAL_CHECK_IP_VAILD(enIP);

    pstHalCtx = &stHalCtx[enIP];

    if (pstHalCtx->bInit)
    {
        VPSS_INFO("VPSS IP%d, Already Init\n", enIP);
        return MT_SUCCESS;
    }

    /* 申请和分配NODE节点 */
    s32Ret = MT_DRV_MMZ_AllocAndMap("VPSS_RegBuf", MT_NULL,
        VPSS_REG_SIZE*VPSS_HAL_TASK_NODE_BUTT, 0, &pstHalCtx->stRegBuf);
    if (s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("Alloc VPSS_RegBuf Failed\n");
        return MT_FAILURE;
    }

    for (i=0; i<VPSS_HAL_TASK_NODE_BUTT; i++)
    {
        pstHalCtx->au32AppPhy[i] = pstHalCtx->stRegBuf.u32StartPhyAddr
                                 + VPSS_REG_SIZE*i;

        pstHalCtx->au32AppVir[i] = pstHalCtx->stRegBuf.u32StartVirAddr
                                 + VPSS_REG_SIZE*i;
    }

    /* 申请和分配缩放系数 */
    s32Ret = MT_DRV_MMZ_AllocAndMap("VPSS_ZmeCoefBuf", MT_NULL,
        VPSS_ZME_COEF_SIZE*4*VPSS_ZME_COEF_NUM, 0, &pstHalCtx->stZmeCoefBuf);
    if (s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("Alloc VPSS_ZmeCoefBuf Failed\n");
        return MT_FAILURE;
    }

    for (i = 0; i < VPSS_ZME_COEF_NUM; i++)
    {
        pstHalCtx->abUsed[i] = MT_FALSE;

        for(j=0; j<4; j++)
        {
            pstHalCtx->au32ZmeCoefPhy[i][j] =
                pstHalCtx->stZmeCoefBuf.u32StartPhyAddr + VPSS_ZME_COEF_SIZE * (4*i+j) ;

            pstHalCtx->au32ZmeCoefVir[i][j] =
                pstHalCtx->stZmeCoefBuf.u32StartVirAddr + VPSS_ZME_COEF_SIZE * (4*i+j);
        }
    }

    /* 映射寄存器地址 */
    pstHalCtx->u32BaseRegVir
        = (mt_u32)IO_ADDRESS(pstHalCtx->u32BaseRegPhy);
    if (0 == pstHalCtx->u32BaseRegVir)
    {
        VPSS_FATAL("io_address VPSS_REG(%#x) Failed\n", pstHalCtx->u32BaseRegPhy);
        MT_DRV_MMZ_UnmapAndRelease(&pstHalCtx->stRegBuf);
        return MT_FAILURE;
    }

    pstHalCtx->bInit = MT_TRUE;

    VPSS_HAL_SetClockEn(enIP, MT_FALSE);

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_DelInit(VPSS_IP_E enIP)
{
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;

    VPSS_HAL_CHECK_IP_VAILD(enIP);

    pstHalCtx = &stHalCtx[enIP];

    if (!pstHalCtx->bInit)
    {
        VPSS_INFO("VPSS IP%d, Already DeInit\n", enIP);
        return MT_SUCCESS;
    }

    VPSS_HAL_SetClockEn(enIP, MT_FALSE);

    MT_ASSERT(pstHalCtx->u32BaseRegVir != 0);
    if (pstHalCtx->u32BaseRegVir != 0)
    {
        pstHalCtx->u32BaseRegVir = 0;
    }

    MT_DRV_MMZ_UnmapAndRelease(&pstHalCtx->stRegBuf);
    memset(&pstHalCtx->stRegBuf, 0, sizeof(mmz_buffer_s));

    MT_DRV_MMZ_UnmapAndRelease(&pstHalCtx->stZmeCoefBuf);
    memset(&pstHalCtx->stZmeCoefBuf, 0, sizeof(mmz_buffer_s));

    pstHalCtx->bInit = MT_FALSE;

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_SetClockEn(VPSS_IP_E enIP, MT_BOOL bClockEn)
{
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;

    VPSS_HAL_CHECK_IP_VAILD(enIP);

    pstHalCtx = &stHalCtx[enIP];
    VPSS_HAL_CHECK_INIT(pstHalCtx->bInit);

    if(pstHalCtx->bClockEn == bClockEn)
    {
        return MT_SUCCESS;
    }

    if (bClockEn)
    {
        VPSS_REG_SetClockEn(enIP, MT_TRUE);

        VPSS_REG_ResetAppReg(pstHalCtx->u32BaseRegVir, MT_NULL);
        VPSS_REG_SetTimeOut(pstHalCtx->u32BaseRegVir, DEF_LOGIC_TIMEOUT);
        VPSS_REG_SetIntMask(pstHalCtx->u32BaseRegVir, 0xfe);
    }
    else
    {
        VPSS_REG_SetClockEn(enIP, MT_FALSE);
    }

    pstHalCtx->bClockEn = bClockEn;

    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_GetClockEn(VPSS_IP_E enIP, MT_BOOL *pbClockEn)
{
    //:TODO:后续加入CRG和时钟控制
    return MT_SUCCESS;
}

mt_s32 VPSS_HAL_GetIntState(VPSS_IP_E enIP, mt_u32* pu32IntState)
{
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;

    VPSS_HAL_CHECK_IP_VAILD(enIP);
    VPSS_HAL_CHECK_NULL_PTR(pu32IntState);

    pstHalCtx = &stHalCtx[enIP];
    VPSS_HAL_CHECK_INIT(pstHalCtx->bInit);

    return VPSS_REG_GetIntState(pstHalCtx->u32BaseRegVir, pu32IntState);
}

mt_s32 VPSS_HAL_ClearIntState(VPSS_IP_E enIP, mt_u32 u32IntState)
{
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;

    VPSS_HAL_CHECK_IP_VAILD(enIP);

    pstHalCtx = &stHalCtx[enIP];
    VPSS_HAL_CHECK_INIT(pstHalCtx->bInit);

    return VPSS_REG_ClearIntState(pstHalCtx->u32BaseRegVir, u32IntState);
}

mt_s32 VPSS_HAL_SetNodeInfo(VPSS_IP_E enIP,
     VPSS_HAL_INFO_S *pstHalInfo,  VPSS_HAL_TASK_NODE_E enTaskNodeId)
{
    mt_u32 u32PortId = 0;
    mt_s32 s32Ret = MT_FAILURE;
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;
    mt_u32 u32AppVir, u32AppPhy;

    VPSS_HAL_CHECK_IP_VAILD(enIP);
    VPSS_HAL_CHECK_NULL_PTR(pstHalInfo);
    VPSS_HAL_CHECK_NODE_ID_VAILD(enTaskNodeId);

    pstHalCtx = &stHalCtx[enIP];
    VPSS_HAL_CHECK_INIT(pstHalCtx->bInit);

    u32AppVir = pstHalCtx->au32AppVir[enTaskNodeId];
    u32AppPhy = pstHalCtx->au32AppPhy[enTaskNodeId];

    switch(pstHalInfo->enNodeType)
    {
        case VPSS_HAL_NODE_2D_Field:
            s32Ret = VPSS_HAL_SetFieldNode(enIP, pstHalInfo, u32AppVir);
            break;
        case VPSS_HAL_NODE_2D_FRAME:
            s32Ret = VPSS_HAL_SetFrameNode(enIP, pstHalInfo, u32AppVir);
            break;
        case VPSS_HAL_NODE_2D_5Field:
            s32Ret = VPSS_HAL_Set5FieldNode(enIP, pstHalInfo, u32AppVir,
                                            u32AppPhy);
            break;
        case VPSS_HAL_NODE_3D_FRAME_R:
            s32Ret = VPSS_HAL_Set3DFrameNode(enIP, pstHalInfo, u32AppVir);
            break;
        case VPSS_HAL_NODE_UHD:
        case VPSS_HAL_NODE_UHD_SPLIT_L:
        case VPSS_HAL_NODE_UHD_SPLIT_R:
            s32Ret = VPSS_HAL_SetUHDNode(enIP, pstHalInfo, u32AppVir);
            break;
#if 0
        case VPSS_HAL_NODE_2D_3Field:
            //VPSS_HAL_SetFrameNode(pstHalInfo, u32AppVir);
            break;
        case VPSS_HAL_NODE_PZME:
            //VPSS_HAL_SetFrameNode(pstHalInfo, u32AppVir);
            break;

        case VPSS_HAL_NODE_3DDET:
            //VPSS_HAL_SetFrameNode(pstHalInfo, u32AppVir);
            break;
        case VPSS_HAL_NODE_ZME_2L:
            //VPSS_HAL_SetFrameNode(pstHalInfo, u32AppVir);
            break;
        case VPSS_HAL_NODE_ROTATION_Y:
            //VPSS_HAL_SetFrameNode(pstHalInfo, u32AppVir);
            break;
        case VPSS_HAL_NODE_ROTATION_C:
            //VPSS_HAL_SetFrameNode(pstHalInfo, u32AppVir);
            break;
#endif
        case VPSS_HAL_NODE_ROTATION_Y:
            u32PortId = (enTaskNodeId - VPSS_HAL_TASK_NODE_P0_RO_Y) / 2;//:TODO:优化
            s32Ret = VPSS_HAL_SetRotateNode(enIP, pstHalInfo, u32AppVir, MT_TRUE,u32PortId);
            break;
        case VPSS_HAL_NODE_ROTATION_C:
            u32PortId = (enTaskNodeId - VPSS_HAL_TASK_NODE_P0_RO_Y) / 2;
            s32Ret = VPSS_HAL_SetRotateNode(enIP, pstHalInfo, u32AppVir, MT_FALSE,u32PortId);
            break;
        case VPSS_HAL_NODE_ROTATION:
            u32PortId = (enTaskNodeId - VPSS_HAL_TASK_NODE_P0_RO);//:TODO:优化
            s32Ret = VPSS_HAL_SetRotateNode(enIP, pstHalInfo, u32AppVir, MT_TRUE,u32PortId);
            break;
        default:
            VPSS_FATAL("No this Node Type:%d!\n", pstHalInfo->enNodeType);
            return MT_FAILURE;

    }

    (mt_void)VPSS_HAL_SetAllAlgCfgAddr(u32AppVir, u32AppPhy);

    //VPSS_FATAL("enTaskNodeId = %d, this Node Type:%d!\n", enTaskNodeId, pstHalInfo->enNodeType);
    //VPSS_HAL_DumpReg(enIP,enTaskNodeId);

    return s32Ret;
}

mt_s32 VPSS_HAL_StartLogic(VPSS_IP_E enIP,
    MT_BOOL abNodeVaild[VPSS_HAL_TASK_NODE_BUTT])
{
    mt_u32 i = 0;
    VPSS_HAL_TASK_NODE_E enId, enLastVaild = VPSS_HAL_TASK_NODE_BUTT;
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;

    VPSS_HAL_CHECK_IP_VAILD(enIP);
    VPSS_HAL_CHECK_NULL_PTR(abNodeVaild);

    pstHalCtx = &stHalCtx[enIP];
    VPSS_HAL_CHECK_INIT(pstHalCtx->bInit);

    /* 从后向前链接所有节点 */
    enLastVaild = VPSS_HAL_TASK_NODE_BUTT;
    for(i=0;i<VPSS_HAL_TASK_NODE_BUTT;i++)
    {
        enId = VPSS_HAL_TASK_NODE_BUTT - 1 - i;

        if(MT_TRUE == abNodeVaild[enId])
        {
            if(VPSS_HAL_TASK_NODE_BUTT == enLastVaild)
            {
                VPSS_REG_StartLogic(0, pstHalCtx->au32AppVir[enId]);
            }
            else
            {
                VPSS_REG_StartLogic(pstHalCtx->au32AppPhy[enLastVaild],
                    pstHalCtx->au32AppVir[enId]);
            }

            enLastVaild = enId;
        }
    }

    /* 清空所有ZME系数使用 */
    for(i=0; i<VPSS_ZME_COEF_NUM; i++)
    {
        pstHalCtx->abUsed[i] = MT_FALSE;
    }

    MT_ASSERT(enLastVaild != VPSS_HAL_TASK_NODE_BUTT);
    if (enLastVaild == VPSS_HAL_TASK_NODE_BUTT)
    {
        VPSS_FATAL("No Node Needs Start\n");
        return MT_FAILURE;
    }
    else
    {
        /* 启动硬件 */
        return VPSS_REG_StartLogic(pstHalCtx->au32AppPhy[enLastVaild],
                            pstHalCtx->u32BaseRegVir);
    }
}


mt_s32 VPSS_HAL_GetSCDInfo(mt_u32 u32AppAddr,mt_s32 s32SCDInfo[32])
{
	//return VPSS_REG_GetSCDInfo(u32AppAddr,s32SCDInfo);
	return MT_SUCCESS;
}

mt_void VPSS_HAL_GetDetPixel(VPSS_IP_E enIP,mt_u32 BlkNum, mt_u8* pstData)
{
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;
    mt_u32 u32AppAddr;

    pstHalCtx = &stHalCtx[enIP];

    u32AppAddr = pstHalCtx->u32BaseRegVir;

    VPSS_REG_GetDetPixel(u32AppAddr,BlkNum,pstData);
}

mt_s32 VPSS_HAL_GetBaseRegAddr(VPSS_IP_E enIP,
                                 mt_u32 *pu32PhyAddr,
                                 mt_u32 *pu32VirAddr)
{
    VPSS_HAL_CTX_S *pstHalCtx = MT_NULL;
    mt_u32 u32AppAddr;

    pstHalCtx = &stHalCtx[enIP];

    *pu32PhyAddr = pstHalCtx->u32BaseRegPhy;
    *pu32VirAddr = pstHalCtx->u32BaseRegVir;

    return MT_SUCCESS;
}
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif  /* __cplusplus */
