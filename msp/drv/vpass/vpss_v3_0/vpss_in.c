/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_in.h"
#include "mt_module_debug.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

mt_s32 VPSS_IN_VFREE_V1(VPSS_IN_ENTITY_S *pstEntity)
{
    mt_s32 s32Ret = MT_SUCCESS;

    if (pstEntity->pstSrcImagesList)
    {
        s32Ret |= VPSS_IMG_DeInit(pstEntity->pstSrcImagesList);

        VPSS_VFREE(pstEntity->pstSrcImagesList);
    }

    if (pstEntity->pstMtInfo)
    {
        s32Ret |= VPSS_MT_DeInit(pstEntity->pstMtInfo);

        VPSS_VFREE(pstEntity->pstMtInfo);
    }

    return s32Ret;
}



mt_s32 VPSS_IN_VMALLOC_V1(VPSS_IN_ENTITY_S *pstEntity)
{
    mt_s32 s32Ret;

    pstEntity->pstSrcImagesList =
        VPSS_VMALLOC(sizeof(VPSS_IMAGELIST_INFO_S));
    if (!pstEntity->pstSrcImagesList)
    {
        goto V1_VMALLOC_ERROR;
    }

    s32Ret = VPSS_IMG_Init(pstEntity->pstSrcImagesList);
    if (MT_SUCCESS != s32Ret)
    {
        goto V1_VMALLOC_ERROR;
    }

    pstEntity->pstMtInfo =
        VPSS_VMALLOC(sizeof(VPSS_MT_INFO_S));
    if (!pstEntity->pstMtInfo)
    {
        goto V1_VMALLOC_ERROR;
    }

    s32Ret = VPSS_MT_Init(pstEntity->pstMtInfo);
    if (MT_SUCCESS != s32Ret)
    {
        goto V1_VMALLOC_ERROR;
    }

    return MT_SUCCESS;

V1_VMALLOC_ERROR:
    return MT_FAILURE;
}

mt_s32 VPSS_IN_VFREE_V2(VPSS_IN_ENTITY_S *pstEntity)
{
    if (pstEntity->pstSrc)
        VPSS_VFREE(pstEntity->pstSrc);

    if (pstEntity->pstWbcInfo[0])
        VPSS_VFREE(pstEntity->pstWbcInfo[0]);
    if (pstEntity->pstWbcInfo[1])
        VPSS_VFREE(pstEntity->pstWbcInfo[0]);

    if (pstEntity->pstSttWbc[0])
        VPSS_VFREE(pstEntity->pstSttWbc[0]);
    if (pstEntity->pstSttWbc[1])
        VPSS_VFREE(pstEntity->pstSttWbc[0]);

    if (pstEntity->pstDieStInfo[0])
        VPSS_VFREE(pstEntity->pstDieStInfo[0]);
    if (pstEntity->pstDieStInfo[1])
        VPSS_VFREE(pstEntity->pstDieStInfo[1]);

    if (pstEntity->pstCcclCntInfo[0])
        VPSS_VFREE(pstEntity->pstCcclCntInfo[0]);
    if (pstEntity->pstCcclCntInfo[1])
        VPSS_VFREE(pstEntity->pstCcclCntInfo[1]);

    if (pstEntity->pstNrMadInfo[0])
        VPSS_VFREE(pstEntity->pstNrMadInfo[0]);
    if (pstEntity->pstNrMadInfo[1])
        VPSS_VFREE(pstEntity->pstNrMadInfo[1]);

    return MT_SUCCESS;
}

mt_s32 VPSS_IN_VMALLOC_V2(VPSS_IN_ENTITY_S *pstEntity)
{
    pstEntity->pstSrc = (VPSS_SRC_S*)VPSS_VMALLOC(sizeof(VPSS_SRC_S));
    if (MT_NULL == pstEntity->pstSrc)
    {
        VPSS_ERROR("malloc VPSS_SRC_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstSrc,0,sizeof(VPSS_SRC_S));

    pstEntity->pstWbcInfo[0] = (VPSS_WBC_S*)VPSS_VMALLOC(sizeof(VPSS_WBC_S));
    if (MT_NULL == pstEntity->pstWbcInfo[0])
    {
        VPSS_ERROR("malloc VPSS_WBC_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstWbcInfo[0],0,sizeof(VPSS_WBC_S));

    pstEntity->pstWbcInfo[1] = (VPSS_WBC_S*)VPSS_VMALLOC(sizeof(VPSS_WBC_S));
    if (MT_NULL == pstEntity->pstWbcInfo[1])
    {
        VPSS_ERROR("malloc VPSS_WBC_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstWbcInfo[1],0,sizeof(VPSS_WBC_S));

    pstEntity->pstSttWbc[0] = (VPSS_STTWBC_S*)VPSS_VMALLOC(sizeof(VPSS_STTWBC_S)*2);
    if (MT_NULL == pstEntity->pstSttWbc[0])
    {
        VPSS_ERROR("malloc VPSS_STTWBC_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstSttWbc[0],0,sizeof(VPSS_STTWBC_S));

    pstEntity->pstSttWbc[1] = (VPSS_STTWBC_S*)VPSS_VMALLOC(sizeof(VPSS_STTWBC_S));
    if (MT_NULL == pstEntity->pstSttWbc[1])
    {
        VPSS_ERROR("malloc VPSS_STTWBC_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstSttWbc[1],0,sizeof(VPSS_STTWBC_S));

    pstEntity->pstDieStInfo[0] = (VPSS_DIESTINFO_S*)VPSS_VMALLOC(sizeof(VPSS_DIESTINFO_S));
    if (MT_NULL == pstEntity->pstDieStInfo[0])
    {
        VPSS_ERROR("malloc VPSS_DIESTINFO_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstDieStInfo[0],0,sizeof(VPSS_DIESTINFO_S));

    pstEntity->pstDieStInfo[1] = (VPSS_DIESTINFO_S*)VPSS_VMALLOC(sizeof(VPSS_DIESTINFO_S));
    if (MT_NULL == pstEntity->pstDieStInfo[1])
    {
        VPSS_ERROR("malloc VPSS_DIESTINFO_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstDieStInfo[1],1,sizeof(VPSS_DIESTINFO_S));


    pstEntity->pstCcclCntInfo[0] = (VPSS_CCCLCNTINFO_S*)VPSS_VMALLOC(sizeof(VPSS_CCCLCNTINFO_S));
    if (MT_NULL == pstEntity->pstCcclCntInfo[0])
    {
        VPSS_ERROR("malloc VPSS_CCCLCNTINFO_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstCcclCntInfo[0],0,sizeof(VPSS_CCCLCNTINFO_S));

    pstEntity->pstCcclCntInfo[1] = (VPSS_CCCLCNTINFO_S*)VPSS_VMALLOC(sizeof(VPSS_CCCLCNTINFO_S));
    if (MT_NULL == pstEntity->pstCcclCntInfo[1])
    {
        VPSS_ERROR("malloc VPSS_CCCLCNTINFO_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstCcclCntInfo[1],0,sizeof(VPSS_CCCLCNTINFO_S));

    pstEntity->pstNrMadInfo[0] = (VPSS_NRMADINFO_S*)VPSS_VMALLOC(sizeof(VPSS_NRMADINFO_S)*2);
    if (MT_NULL == pstEntity->pstNrMadInfo[0])
    {
        VPSS_ERROR("malloc VPSS_NRMADINFO_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstNrMadInfo[0],0,sizeof(VPSS_NRMADINFO_S)*2);

    pstEntity->pstNrMadInfo[1] = (VPSS_NRMADINFO_S*)VPSS_VMALLOC(sizeof(VPSS_NRMADINFO_S));
    if (MT_NULL == pstEntity->pstNrMadInfo[1])
    {
        VPSS_ERROR("malloc VPSS_NRMADINFO_S failed\n");
        goto V2_VMALLOC_ERROR;
    }
    memset(pstEntity->pstNrMadInfo[1],0,sizeof(VPSS_NRMADINFO_S));


    return MT_SUCCESS;

V2_VMALLOC_ERROR:
    return MT_FAILURE;
}

MT_BOOL VPSS_IN_CheckImage_V1(MT_DRV_VIDEO_FRAME_S *pstImage)
{
    MT_DRV_VIDEO_PRIVATE_S *pstPriv;
    MT_BOOL bSupported = MT_TRUE;
    pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstImage->u32Priv[0]);

    MT_INFO_VPSS("UUUUUUUUUUU  VPSS_IN_CheckImage_V1 !!!!!!!!!!! \n");

    if (pstPriv->u32LastFlag == DEF_MT_DRV_VPSS_LAST_ERROR_FLAG)
    {
        return MT_TRUE;
    }
    if (pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12_TILE
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21_TILE
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12_TILE_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21_TILE_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV16
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV61
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV16_2X1
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV61_2X1
        && pstImage->ePixFormat !=  MT_DRV_PIX_FMT_YUV400
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV_444
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV422_2X1
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV422_1X2
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV420p
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV411
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV410p
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUYV
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YVYU
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_UYVY
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ARGB8888
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ABGR8888
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ARGB1555
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ABGR1555
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_RGB565
        )

    {
        //ROck_hu
        //VPSS_FATAL("In image can't be processed Pixformat %d\n",pstImage->ePixFormat);
        //bSupported = MT_FALSE;
    }

    if (pstImage->eFrmType >= MT_DRV_FT_BUTT)
    {
        //Rock_hu
        //VPSS_FATAL("In image can't be processed FrmType %d\n",pstImage->eFrmType);
        //bSupported = MT_FALSE;
    }

    if ((pstImage->u32Height < VPSS_FRAME_MIN_HEIGHT) || (pstImage->u32Width < VPSS_FRAME_MIN_WIDTH)
        || (pstImage->u32Height > VPSS_FRAME_MAX_HEIGHT) || (pstImage->u32Width > VPSS_FRAME_MAX_WIDTH))
    {
        //Rock_hu
        //VPSS_FATAL("In image can't be processed H %d W %d\n",
        //    pstImage->u32Height,
        //    pstImage->u32Width);
       // bSupported = MT_FALSE;
    }

    return bSupported;
}
MT_BOOL VPSS_IN_CheckImage_V2(MT_DRV_VIDEO_FRAME_S *pstImage)
{
    MT_DRV_VIDEO_PRIVATE_S *pstPriv;
    MT_BOOL bSupported = MT_TRUE;
    pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstImage->u32Priv[0]);

    if (pstPriv->u32LastFlag == DEF_MT_DRV_VPSS_LAST_ERROR_FLAG)
    {
        VPSS_ERROR("receive last frame,error flag\n");
        return MT_FALSE;
    }

    if (pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12_TILE
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21_TILE
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV12_TILE_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV21_TILE_CMP
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV16
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV61
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV16_2X1
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_NV61_2X1
        && pstImage->ePixFormat !=  MT_DRV_PIX_FMT_YUV400
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV_444
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV422_2X1
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV422_1X2
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV420p
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV411
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUV410p
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YUYV
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_YVYU
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_UYVY
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ARGB8888
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ABGR8888
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ARGB1555
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_ABGR1555
        && pstImage->ePixFormat != MT_DRV_PIX_FMT_RGB565
        )

    {
        VPSS_FATAL("In image can't be processed Pixformat %d\n",pstImage->ePixFormat);
        bSupported = MT_FALSE;
    }

    if (pstImage->eFrmType >= MT_DRV_FT_BUTT)
    {
        VPSS_FATAL("In image can't be processed FrmType %d\n",pstImage->eFrmType);
        bSupported = MT_FALSE;
    }

    if ((pstImage->u32Height < VPSS_FRAME_MIN_HEIGHT) || (pstImage->u32Width < VPSS_FRAME_MIN_WIDTH)
        || (pstImage->u32Height > VPSS_FRAME_MAX_HEIGHT) || (pstImage->u32Width > VPSS_FRAME_MAX_WIDTH))
    {
        VPSS_FATAL("In image can't be processed H %d W %d\n",
            pstImage->u32Height,
            pstImage->u32Width);
        bSupported = MT_FALSE;
    }

    return bSupported;
}

mt_s32 VPSS_IN_CorrectFieldOrder(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S *pImage)
{
    VPSS_IN_STREAM_INFO_S *pstStreamInfo;

    pstStreamInfo = &(pstEntity->stStreamInfo);

    /*
      * when get first image ,believe its topfirst info
      */
    if(pstStreamInfo->u32RealTopFirst == DEF_TOPFIRST_BUTT)
    {
        if(pImage->bProgressive == MT_TRUE)
        {
            pstStreamInfo->u32RealTopFirst = DEF_TOPFIRST_PROG;
        }
        else
        {
            pstStreamInfo->u32RealTopFirst =
                        pImage->bTopFieldFirst;
        }
    }
    else
    {
        /*
           * detect progressive
           */
        if (pstStreamInfo->u32RealTopFirst == DEF_TOPFIRST_PROG)
        {
            if(pImage->bProgressive == MT_TRUE)
            {

            }
            else
            {
                pstStreamInfo->u32RealTopFirst =
                    pImage->bTopFieldFirst;
            }
        }
        /*
           * detect interlace
           */
        else
        {
            if(pImage->bProgressive == MT_TRUE)
            {
                pstStreamInfo->u32RealTopFirst =
                    DEF_TOPFIRST_PROG;
            }
            else
            {
                pImage->bTopFieldFirst =
                    pstStreamInfo->u32RealTopFirst;
            }
        }

    }

    return MT_SUCCESS;
}
mt_s32 VPSS_IN_CorrectProgInfo(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S *pImage)
{
    MT_DRV_VIDEO_PRIVATE_S *pstFrmPriv;
    MT_VDEC_PRIV_FRAMEINFO_S *pstVdecPriv;

    pstFrmPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pImage->u32Priv[0]);
    pstVdecPriv = (MT_VDEC_PRIV_FRAMEINFO_S *)&(pstFrmPriv->u32Reserve[0]);

    if (pstFrmPriv->u32LastFlag == DEF_MT_DRV_VPSS_LAST_ERROR_FLAG)
    {
        return MT_SUCCESS;
    }

    if (pstEntity->bAlwaysFlushSrc == MT_TRUE)
    {
        pImage->bProgressive = MT_TRUE;
        return  MT_SUCCESS;
    }

    if (pstFrmPriv->u32LastFlag == DEF_MT_DRV_VPSS_LAST_FRAME_FLAG)
    {
        return MT_SUCCESS;
    }

    // belive VI, VI will give the right info
    if (((pImage->hTunnelSrc >> 16) & 0xff) == MT_ID_VI)
    {
        return MT_SUCCESS;
    }

    if (pstFrmPriv->stVideoOriginalInfo.enSource != MT_DRV_SOURCE_DTV)
    {
        return MT_SUCCESS;
    }

    if ( 0x2 == (pstVdecPriv->u8Marker &= 0x2))
    {
        pImage->bProgressive = MT_TRUE;
        return  MT_SUCCESS;
    }

    if ( pImage->u32Height > 1088 || pImage->u32Width > 1920)
    {
        pImage->bProgressive = MT_TRUE;
        return  MT_SUCCESS;
    }

    if ( pImage->u32Height == 1080 && pImage->u32Width == 1920)
    {
        if (pImage->u32FrameRate > 50000)
        {
            pImage->bProgressive = MT_TRUE;
            return  MT_SUCCESS;
        }
    }

    if ((pImage->eFrmType == MT_DRV_FT_SBS) ||
        (pImage->eFrmType == MT_DRV_FT_TAB) ||
        (pImage->eFrmType == MT_DRV_FT_FPK))
    {
        pImage->bProgressive = MT_TRUE;
        return MT_SUCCESS;
    }


    if (pImage->ePixFormat == MT_DRV_PIX_FMT_YUYV
        || pImage->ePixFormat == MT_DRV_PIX_FMT_YVYU
        || pImage->ePixFormat == MT_DRV_PIX_FMT_UYVY)
    {
        pImage->bProgressive = MT_TRUE;
        return MT_SUCCESS;
    }

    if (pstEntity->bProgRevise == MT_FALSE)
    {
		pImage->bProgressive = MT_TRUE;
        return MT_SUCCESS;
    }
    if(pstEntity->enProgInfo == MT_DRV_VPSS_PRODETECT_AUTO)
    {
        if (  (MT_UNF_VCODEC_TYPE_REAL8 == pstVdecPriv->entype)
                ||(MT_UNF_VCODEC_TYPE_REAL9 == pstVdecPriv->entype)
                ||(MT_UNF_VCODEC_TYPE_MPEG4 == pstVdecPriv->entype))
        {
            return MT_SUCCESS;
        }

        if(pImage->u32Height == 720)
        {
            pImage->bProgressive = MT_TRUE;
        }
        /* un-trust bit-stream information  */
        else if(pImage->u32Height <= 576)
        {
            if ( (240 >= pImage->u32Height) && (320 >= pImage->u32Width) )
            {
                pImage->bProgressive = MT_TRUE;
            }
            else if (pImage->u32Height <= (pImage->u32Width * 9 / 14 ) )
            {
                // Rule: wide aspect ratio stream is normal progressive, we think that progressive info is correct.
            }
            else
            {
                pImage->bProgressive = MT_FALSE;
            }
        }
        else
        {

        }
    }

    else if(pstEntity->enProgInfo == MT_DRV_VPSS_PRODETECT_INTERLACE)
    {
        pImage->bProgressive = MT_FALSE;
    }
	else if (pstEntity->enProgInfo == MT_DRV_VPSS_PRODETECT_PROGRESSIVE)
	{
        if (pImage->bProgressive == MT_FALSE)
        {
            if (pImage->bTopFieldFirst == MT_TRUE)
            {
                pImage->bProgressive = MT_TRUE;
                pImage->enFieldMode = MT_DRV_FIELD_BOTTOM;
            }
            else
            {
                pImage->bProgressive = MT_TRUE;
                pImage->enFieldMode = MT_DRV_FIELD_TOP;
            }
        }
    }
    else
    {
        VPSS_FATAL("Invalid ProgInfo %d\n",pstEntity->enProgInfo);
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_IN_ReviseImage(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S *pImage)
{
    MT_DRV_VIDEO_PRIVATE_S *pstFrmPriv;

    /*1.Revise image height to 4X*/
    pImage->u32Height =  pImage->u32Height & 0xfffffffc;
    if (pImage->u32Width > 1920)
    {
        pImage->u32Width =  pImage->u32Width & 0xfffffffc;
    }

    pstFrmPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pImage->u32Priv[0]);
    if (pstFrmPriv->stVideoOriginalInfo.enSource == MT_DRV_SOURCE_DTV)
    {
        pstFrmPriv->stVideoOriginalInfo.bInterlace = (pImage->bProgressive == MT_TRUE) ? MT_FALSE : MT_TRUE;
        pstFrmPriv->stVideoOriginalInfo.u32FrmRate = pstEntity->stStreamInfo.u32InRate * 1000;

        if (pstEntity->enSrcCS == MT_DRV_CS_UNKNOWN)
        {
            if (pImage->u32Width >= 1280 || pImage->u32Height >= 720)
            {
                pstFrmPriv->eColorSpace  = MT_DRV_CS_BT709_YUV_LIMITED;
            }
            else
            {
                pstFrmPriv->eColorSpace  = MT_DRV_CS_BT601_YUV_LIMITED;
            }
        }
        else
        {
            pstFrmPriv->eColorSpace = pstEntity->enSrcCS;
        }
    }

    if (pstEntity->stStreamInfo.u32RealTopFirst != DEF_TOPFIRST_BUTT
        && pstEntity->stStreamInfo.u32StreamProg == pImage->bProgressive)
    {
        pImage->bTopFieldFirst = pstEntity->stStreamInfo.u32RealTopFirst;
    }

    /*
      * 2. 3d addr revise
      * SBS TAB read half image
      * MVC read two addr
      */
    if ((pImage->eFrmType == MT_DRV_FT_SBS) || (pImage->eFrmType == MT_DRV_FT_TAB))
    {
        memcpy(&(pImage->stBufAddr[1]), &(pImage->stBufAddr[0]),
               sizeof(MT_DRV_VID_FRAME_ADDR_S));

        switch(pImage->ePixFormat)
        {
            case MT_DRV_PIX_FMT_NV12_TILE:
            case MT_DRV_PIX_FMT_NV21_TILE:
                /* 对于非压缩的格式，将宽高直接除根据格式进行除2 */
                if(pImage->eFrmType == MT_DRV_FT_SBS)
                {
                    pImage->u32Width = pImage->u32Width/2;
                    if(pImage->u32Width%2 != 0)
                    {
                        VPSS_FATAL("3D image can't be processed W %d\n",
                            pImage->u32Width);
                    }
                }

                if(pImage->eFrmType == MT_DRV_FT_TAB)
                {
                    pImage->u32Height = pImage->u32Height/2;
                    if(pImage->u32Height%2 != 0)
                    {
                        VPSS_FATAL("3D image can't be processed H %d\n",
                            pImage->u32Height);
                    }
                }

                break;
            default:
                 VPSS_FATAL("3D tile image can't be processed ePixFormat %d\n",
                    pImage->ePixFormat);
        }
    }

    /*
     *revise frame rate
     */
    if (pImage->u32FrameRate == 0)
    {
        pImage->u32FrameRate = 25000;
    }
    /*
     * if not top/bottom Interleaved,force Split
     */
    if (pImage->enFieldMode != MT_DRV_FIELD_ALL)
    {
        pImage->bProgressive = MT_TRUE;
    }
    return MT_SUCCESS;
}

mt_s32 VPSS_IN_ChangeInRate(VPSS_IN_ENTITY_S *pstEntity,mt_u32 u32InRate)
{
    mt_u32 u32HzRate; /*0 -- 100*/

    u32HzRate = u32InRate / 1000;

    if(u32HzRate < 10)
    {
        u32HzRate = 1;
    }
    else if(u32HzRate < 20)
    {
        u32HzRate = 10;
    }
    else if(u32HzRate < 30)
    {
        u32HzRate = 25;
    }
    else if(u32HzRate < 40)
    {
        u32HzRate = 30;
    }
    else if(u32HzRate < 60)
    {
        u32HzRate = 50;
    }
    else
    {
        u32HzRate = u32HzRate / 10 * 10;
    }

    pstEntity->stStreamInfo.u32InRate = u32HzRate;

    return MT_SUCCESS;
}

mt_s32 VPSS_IN_UpdateStreamInfo(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S *pstSrcImage)
{
    VPSS_IN_STREAM_INFO_S *pstStreamInfo;
    pstStreamInfo = &(pstEntity->stStreamInfo);

    if(pstStreamInfo->u32StreamProg != pstSrcImage->bProgressive)
    {
        pstStreamInfo->u32IsNewImage = MT_TRUE;
    }
    else if(pstStreamInfo->u32StreamH != pstSrcImage->u32Height)
    {
        pstStreamInfo->u32IsNewImage = MT_TRUE;
    }
    else if(pstStreamInfo->u32StreamW != pstSrcImage->u32Width)
    {
        pstStreamInfo->u32IsNewImage = MT_TRUE;
    }
    else if(pstStreamInfo->eStreamFrmType != pstSrcImage->eFrmType)
    {
        pstStreamInfo->u32IsNewImage = MT_TRUE;
    }
    else
    {
        pstStreamInfo->u32IsNewImage = MT_FALSE;
    }

    if(MT_TRUE == pstStreamInfo->u32IsNewImage)
    {
        pstEntity->u32ScenceChgCnt++;
    }

    pstStreamInfo->u32StreamInRate = pstSrcImage->u32FrameRate;
    pstStreamInfo->u32StreamTopFirst = pstSrcImage->bTopFieldFirst;
    pstStreamInfo->u32StreamProg = pstSrcImage->bProgressive;
    pstStreamInfo->u32StreamH = pstSrcImage->u32Height;
    pstStreamInfo->u32StreamW = pstSrcImage->u32Width;
    pstStreamInfo->eStreamFrmType = pstSrcImage->eFrmType;

    return MT_SUCCESS;
}
mt_s32 VPSS_IN_GetSrcInitMode_V2(VPSS_IN_ENTITY_S *pstEntity)
{
    VPSS_IN_STREAM_INFO_S *pstStreamInfo;

    VPSS_CHECK_NULL(pstEntity);

    VPSS_CHECK_VERSION(pstEntity->enVersion, VPSS_VERSION_V2_0);

    pstStreamInfo = &(pstEntity->stStreamInfo);

    if(MT_TRUE == pstStreamInfo->u32StreamProg)
    {
        return SRC_MODE_FRAME;
    }
    else
    if((720 == pstStreamInfo->u32StreamW) && (480 == pstStreamInfo->u32StreamH))
    {
        return SRC_MODE_NTSC;
    }
    else
    if((720 == pstStreamInfo->u32StreamW) && (576 == pstStreamInfo->u32StreamH))
    {
        return SRC_MODE_PAL;
    }
    else
    {
        return SRC_MODE_FIELD;
    }

}

mt_s32 VPSS_IN_Refresh_V1(VPSS_IN_ENTITY_S *pstEntity)
{
    mt_s32 s32Ret = MT_SUCCESS;
    //unused
    //MT_BOOL bSupport = MT_TRUE;
    MT_DRV_VIDEO_PRIVATE_S *pstPriv;
    MT_DRV_VIDEO_FRAME_S stSrcImage;	//stack might overflow!
    MT_DRV_VIDEO_FRAME_S *pstImage;

    VPSS_CHECK_NULL(pstEntity);

    VPSS_CHECK_VERSION(pstEntity->enVersion,VPSS_VERSION_V1_0);

    VPSS_CHECK_NULL(pstEntity->pfnAcqCallback);

    VPSS_CHECK_NULL(pstEntity->pfnRlsCallback);

    if(VPSS_IMG_CheckImageList(pstEntity->pstSrcImagesList) == MT_TRUE)
    {
        return MT_SUCCESS;
    }

    if (!VPSS_IMG_CheckEmptyNode(pstEntity->pstSrcImagesList))
    {
        return MT_FAILURE;
    }

    memset(&stSrcImage,0,sizeof(MT_DRV_VIDEO_FRAME_S));

    pstImage = &stSrcImage;

    pstEntity->pstSrcImagesList->u32GetUsrTotal++;


    s32Ret = pstEntity->pfnAcqCallback(pstEntity->hSource,pstImage);//rock_hu
    //printk("VPSS_IN_Refresh_V1 ret 0x%x\n", s32Ret);//yihua


    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    if (0)
    {
        MT_DRV_VIDEO_FRAME_S *pstFrm;
        MT_DRV_VIDEO_PRIVATE_S *pstPriv;
        MT_VDEC_PRIV_FRAMEINFO_S *pstVdecPriv;

        pstFrm = pstImage;

        pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstFrm->u32Priv[0]);
        pstVdecPriv = (MT_VDEC_PRIV_FRAMEINFO_S *)&(pstPriv->u32Reserve[0]);


        MT_PRINT("Image Info:Index %d Type %d Format %d W %d H %d Prog %d FieldMode %d PTS %d Rate %d LastFlag %#x Delta %d CodeType %d,SourceType %d,BitWidth %d\n"
                 "           L:Y %#x C %#x YH %#x CH %#x YS %d CS %d \n"
                 "           R:Y %#x C %#x YH %#x CH %#x YS %d CS %d \n",
                pstFrm->u32FrameIndex,
                pstFrm->eFrmType,
                pstFrm->ePixFormat,
                pstFrm->u32Width,
                pstFrm->u32Height,
                pstFrm->bProgressive,
                pstFrm->enFieldMode,
                pstFrm->u32Pts,
                pstFrm->u32FrameRate,
                pstPriv->u32LastFlag,
                pstVdecPriv->s32InterPtsDelta,
                pstVdecPriv->entype,
                pstPriv->stVideoOriginalInfo.enSource,
                pstFrm->enBitWidth,
                pstFrm->stBufAddr[0].u32PhyAddr_Y,
                pstFrm->stBufAddr[0].u32PhyAddr_C,
                pstFrm->stBufAddr[0].u32PhyAddr_YHead,
                pstFrm->stBufAddr[0].u32PhyAddr_CHead,
                pstFrm->stBufAddr[0].u32Stride_Y,
                pstFrm->stBufAddr[0].u32Stride_C,
                pstFrm->stBufAddr[1].u32PhyAddr_Y,
                pstFrm->stBufAddr[1].u32PhyAddr_C,
                pstFrm->stBufAddr[1].u32PhyAddr_YHead,
                pstFrm->stBufAddr[1].u32PhyAddr_CHead,
                pstFrm->stBufAddr[1].u32Stride_Y,
                pstFrm->stBufAddr[1].u32Stride_C);
    }
    pstEntity->pstSrcImagesList->u32GetUsrSuccess++;

#if 0 //Rock_hu 该部分代码应该删除
    bSupport = VPSS_IN_CheckImage_V1(pstImage);
    if (MT_TRUE != bSupport)
    {
        pstEntity->pstSrcImagesList->u32RelUsrTotal ++;
        pstEntity->pfnRlsCallback(pstEntity->hSource,pstImage);
        pstEntity->pstSrcImagesList->u32RelUsrSuccess++;
        return MT_FAILURE;
    }
    if (0)
    {
        mt_u8 chFile[20] = "vpss_in.yuv";
        VPSS_OSAL_WRITEYUV(pstImage, chFile);
    }
#endif

    {
        mt_ld_event_s evt;
        mt_u32 TmpTime = 0;
		    mt_drv_sys_gettimestampms(&TmpTime);
        evt.evt_id = EVENT_VPSS_FRM_IN;
        evt.frame = pstImage->u32FrameIndex;
        evt.handle = pstImage->hTunnelSrc;
        evt.time = TmpTime;
        mt_drv_ld_notify_event(&evt);
    }


    if (pstImage->bIsFirstIFrame)
    {
        mt_drv_stat_event(STAT_EVENT_VPSSGETFRM, 0);
    }

    pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstImage->u32Priv[0]);

    if (pstPriv->u32LastFlag == DEF_MT_DRV_VPSS_LAST_ERROR_FLAG)
    {

    }
    else
    {
#if 0 //Rock_hu 删除图像处理的相关操作
        (mt_void)VPSS_IN_CorrectProgInfo(pstEntity, pstImage);

        (mt_void)VPSS_IN_ReviseImage(pstEntity, pstImage);

        (mt_void)VPSS_IN_ChangeInRate(pstEntity, pstImage->u32FrameRate);

        (mt_void)VPSS_IN_CorrectFieldOrder(pstEntity, pstImage);

        (mt_void)VPSS_IN_UpdateStreamInfo(pstEntity, pstImage);

        if (MT_TRUE == pstEntity->stStreamInfo.u32IsNewImage)
        {
            pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstImage->u32Priv[0]);
            pstEntity->stOriInfo.enSource = pstPriv->stVideoOriginalInfo.enSource;
            pstEntity->stOriInfo.u32Width = pstPriv->stVideoOriginalInfo.u32Width;
            pstEntity->stOriInfo.u32Height = pstPriv->stVideoOriginalInfo.u32Height;
            pstEntity->stOriInfo.u32FrmRate = pstPriv->stVideoOriginalInfo.u32FrmRate;
            pstEntity->stOriInfo.bInterlace = (pstImage->bProgressive == MT_TRUE) ? MT_FALSE : MT_TRUE;
            pstEntity->stOriInfo.enColorSys = pstPriv->stVideoOriginalInfo.enColorSys;
        }
#endif
    }
    //printk("\n VPSS_IMG_AddNewImg frame cnt %d\n", pstImage->u32FrameNo);//yihua
    if (MT_SUCCESS != VPSS_IMG_AddNewImg(pstEntity->pstSrcImagesList, pstImage))
    {
        VPSS_ERROR("Add New Image Failed\n");
        pstEntity->pfnRlsCallback(pstEntity->hSource,pstImage);
        return MT_FAILURE;
    }

    if(VPSS_IMG_CheckImageList(pstEntity->pstSrcImagesList) == MT_TRUE)
    {
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
}

mt_s32 VPSS_IN_Refresh_V2(VPSS_IN_ENTITY_S *pstEntity)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MT_BOOL bSupport = MT_TRUE;
    mt_s32 s32InitListRet = MT_FAILURE;
    MT_DRV_VIDEO_PRIVATE_S *pstPriv;
    MT_DRV_VIDEO_FRAME_S stSrcImage;
    MT_DRV_VIDEO_FRAME_S *pstImage;

    VPSS_CHECK_NULL(pstEntity);

    VPSS_CHECK_VERSION(pstEntity->enVersion,VPSS_VERSION_V2_0);

    VPSS_CHECK_NULL(pstEntity->pfnAcqCallback);

    VPSS_CHECK_NULL(pstEntity->pfnRlsCallback);

    memset(&stSrcImage,0,sizeof(MT_DRV_VIDEO_FRAME_S));

    pstImage = &stSrcImage;


    s32Ret = pstEntity->pfnAcqCallback(pstEntity->hSource,pstImage);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    bSupport = VPSS_IN_CheckImage_V2(pstImage);
    if (MT_TRUE != bSupport)
    {
        pstEntity->pfnRlsCallback(pstEntity->hSource,pstImage);
        return MT_FAILURE;
    }

    (mt_void)VPSS_IN_CorrectProgInfo(pstEntity, pstImage);
    (mt_void)VPSS_IN_ReviseImage(pstEntity, pstImage);
    (mt_void)VPSS_IN_ChangeInRate(pstEntity, pstImage->u32FrameRate);
    (mt_void)VPSS_IN_UpdateStreamInfo(pstEntity, pstImage);

    if (MT_TRUE == pstEntity->stStreamInfo.u32IsNewImage)
    {
        #if 1
        VPSS_SRC_ATTR_S stSrcAttr;
        VPSS_WBC_ATTR_S stWbcAttr;
        VPSS_NR_ATTR_S stNrAttr;

        VPSS_SRC_S* pstSrcInfo;
        VPSS_WBC_S* pstWbcInfo;
        VPSS_STTWBC_S* psttWbc;
        VPSS_DIESTINFO_S* pstDieStInfo;
        VPSS_CCCLCNTINFO_S* pstCcclCntInfo;
        VPSS_NRMADINFO_S* pstNrMadInfo;
        unsigned long flags;

        pstSrcInfo = (VPSS_SRC_S*)pstEntity->pstSrc;
        pstWbcInfo = (VPSS_WBC_S*)pstEntity->pstWbcInfo[0];
        psttWbc = (VPSS_STTWBC_S*)pstEntity->pstSttWbc[0];
        pstDieStInfo = (VPSS_DIESTINFO_S*)pstEntity->pstDieStInfo[0];
        pstCcclCntInfo = (VPSS_CCCLCNTINFO_S*)pstEntity->pstCcclCntInfo[0];
        pstNrMadInfo = (VPSS_NRMADINFO_S*)pstEntity->pstNrMadInfo[0];

        VPSS_CHECK_NULL(pstSrcInfo);
        VPSS_CHECK_NULL(pstWbcInfo);
        VPSS_CHECK_NULL(psttWbc);
        VPSS_CHECK_NULL(pstDieStInfo);
        VPSS_CHECK_NULL(pstCcclCntInfo);
        VPSS_CHECK_NULL(pstNrMadInfo);

        pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstImage->u32Priv[0]);
        pstEntity->stOriInfo.enSource = pstPriv->stVideoOriginalInfo.enSource;
        pstEntity->stOriInfo.u32Width = pstPriv->stVideoOriginalInfo.u32Width;
        pstEntity->stOriInfo.u32Height = pstPriv->stVideoOriginalInfo.u32Height;
        pstEntity->stOriInfo.u32FrmRate = pstPriv->stVideoOriginalInfo.u32FrmRate;
        pstEntity->stOriInfo.bInterlace = (pstImage->bProgressive == MT_TRUE) ? MT_FALSE : MT_TRUE;
        pstEntity->stOriInfo.enColorSys = pstPriv->stVideoOriginalInfo.enColorSys;

        //if first image , take its fieldorder
        pstEntity->stStreamInfo.u32RealTopFirst = pstImage->bTopFieldFirst;

        stSrcAttr.hSrcModule = pstEntity->hSource;
        stSrcAttr.pfnRlsImage = (PFN_SRC_FUNC)pstEntity->pfnRlsCallback;

        stWbcAttr.enBitWidth = pstImage->enBitWidth;
        stWbcAttr.enRefMode = VPSS_WBC_REF_MODE_NULL;
        stWbcAttr.ePixFormat = pstImage->ePixFormat;
        stWbcAttr.u32Height = pstImage->u32Height;
        stWbcAttr.u32Width = pstImage->u32Width;
        stNrAttr.u32Height = pstImage->u32Height;
        stNrAttr.u32Width = pstImage->u32Width;

        stSrcAttr.enMode = VPSS_IN_GetSrcInitMode_V2(pstEntity);

        if (MT_TRUE == pstEntity->stStreamInfo.u32StreamProg)
        {
            stWbcAttr.enMode = VPSS_WBC_MODE_NORMAL;
            stNrAttr.enMode = NR_MODE_FRAME;
        }
        else
        {
            stWbcAttr.enMode = VPSS_WBC_MODE_5FIELD;
            stNrAttr.enMode = NR_MODE_5FIELD;
        }

        if(pstImage->u32Width > 1920)
        {
            VPSS_OSAL_DownSpin(&(pstEntity->stSrcSpin),&flags);
            s32InitListRet = VPSS_SRC_Init(pstSrcInfo, stSrcAttr);
            VPSS_OSAL_UpSpin(&(pstEntity->stSrcSpin),&flags);

            s32InitListRet |= VPSS_STTINFO_SttWbcInit(psttWbc);
        }
        else
        {
            if (MT_DRV_FT_NOT_STEREO == pstImage->eFrmType)
            {
                VPSS_OSAL_DownSpin(&(pstEntity->stSrcSpin),&flags);
                s32InitListRet = VPSS_SRC_Init(pstSrcInfo, stSrcAttr);
                VPSS_OSAL_UpSpin(&(pstEntity->stSrcSpin),&flags);

                s32InitListRet |= VPSS_WBC_Init(pstWbcInfo, &stWbcAttr);
                s32InitListRet |= VPSS_STTINFO_DieInit(pstDieStInfo, stWbcAttr.u32Width, stWbcAttr.u32Height);
                s32InitListRet |= VPSS_STTINFO_CcclInit(pstCcclCntInfo, stWbcAttr.u32Width, stWbcAttr.u32Height);
                s32InitListRet |= VPSS_STTINFO_NrInit(pstNrMadInfo, &stNrAttr);
                s32InitListRet |= VPSS_STTINFO_SttWbcInit(psttWbc);
            }
            else
            {
                VPSS_OSAL_DownSpin(&(pstEntity->stSrcSpin),&flags);
                s32InitListRet = VPSS_SRC_Init(pstSrcInfo, stSrcAttr);
                VPSS_OSAL_UpSpin(&(pstEntity->stSrcSpin),&flags);

                s32InitListRet |= VPSS_WBC_Init(pstWbcInfo, &stWbcAttr);
                s32InitListRet |= VPSS_WBC_Init(pstWbcInfo + 1, &stWbcAttr);
                s32InitListRet |= VPSS_STTINFO_DieInit(pstDieStInfo, stWbcAttr.u32Width, stWbcAttr.u32Height);
                s32InitListRet |= VPSS_STTINFO_DieInit(pstDieStInfo + 1, stWbcAttr.u32Width, stWbcAttr.u32Height);
                s32InitListRet |= VPSS_STTINFO_CcclInit(pstCcclCntInfo, stWbcAttr.u32Width, stWbcAttr.u32Height);
                s32InitListRet |= VPSS_STTINFO_CcclInit(pstCcclCntInfo + 1, stWbcAttr.u32Width, stWbcAttr.u32Height);
                s32InitListRet |= VPSS_STTINFO_NrInit(pstNrMadInfo, &stNrAttr);
                s32InitListRet |= VPSS_STTINFO_NrInit(pstNrMadInfo + 1, &stNrAttr);
                s32InitListRet |= VPSS_STTINFO_SttWbcInit(psttWbc);
                s32InitListRet |= VPSS_STTINFO_SttWbcInit(psttWbc + 1);
            }
        }

        if(MT_SUCCESS != s32InitListRet)
        {
            s32Ret = MT_FAILURE;
            goto REFRESH_V2_OUT;
        }
        #endif
        s32Ret = VPSS_SRC_PutImage(pstSrcInfo, (VPSS_SRC_DATA_S*)pstImage);
    }
    else
    {
        VPSS_SRC_S* pstSrcInfo;

        pstSrcInfo = (VPSS_SRC_S*)pstEntity->pstSrc;

        VPSS_CHECK_NULL(pstSrcInfo);

        s32Ret = VPSS_SRC_PutImage(pstSrcInfo, (VPSS_SRC_DATA_S*)pstImage);

    }

REFRESH_V2_OUT:
    return s32Ret;
}

mt_s32 VPSS_IN_GetProcessImage_V1(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S **ppstFrame)
{
    return VPSS_IMG_GetProcessImg(pstEntity->pstSrcImagesList,
                               ppstFrame);
}


mt_s32 VPSS_IN_GetProcessImage_V2(VPSS_IN_ENTITY_S *pstEntity,MT_DRV_VIDEO_FRAME_S **ppstFrame)
{
    VPSS_SRC_S *pstSrc;
    mt_s32 s32Ret = MT_SUCCESS;
    unsigned long flags;

    VPSS_CHECK_NULL(pstEntity);

    VPSS_CHECK_VERSION(pstEntity->enVersion,VPSS_VERSION_V2_0);

    VPSS_CHECK_NULL(pstEntity->pstSrc);

    pstSrc = (VPSS_SRC_S *)pstEntity->pstSrc;


    VPSS_OSAL_DownSpin(&(pstEntity->stSrcSpin),&flags);
    s32Ret = VPSS_SRC_GetProcessImage(pstSrc,ppstFrame);
    VPSS_OSAL_UpSpin(&(pstEntity->stSrcSpin),&flags);

    return s32Ret;
}

mt_s32 VPSS_IN_GetInfo_V1(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_INFO_TYPE_E enType,
                                    MT_DRV_BUF_ADDR_E enLR,
                                    mt_void* pstInfo)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VPSS_CHECK_NULL(pstEntity);

    VPSS_CHECK_VERSION(pstEntity->enVersion,VPSS_VERSION_V1_0);

    switch(enType)
    {
        case VPSS_IN_INFO_STREAM:
            if (MT_DRV_BUF_ADDR_MAX == enLR)
            {
                VPSS_IN_STREAM_INFO_S *pstStreamInfo;
                pstStreamInfo = (VPSS_IN_STREAM_INFO_S *)pstInfo;
                memcpy(pstStreamInfo,&(pstEntity->stStreamInfo),
                        sizeof(VPSS_IN_STREAM_INFO_S));
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_FIELD:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR)
            {
                MT_DRV_VID_FRAME_ADDR_S *pstFieldAddr;
                pstFieldAddr = (MT_DRV_VID_FRAME_ADDR_S *)pstInfo;
                VPSS_IMG_GetFieldAddr(pstEntity->pstSrcImagesList,
                                     pstFieldAddr,
                                     enLR);
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_MT_ADDR:
            if (MT_DRV_BUF_ADDR_MAX == enLR)
            {
                VPSS_MT_ADDR_S *pstHisAddr;
                pstHisAddr = (VPSS_MT_ADDR_S *)pstInfo;
                VPSS_MT_GetAddr(pstEntity->pstMtInfo,
                                     pstHisAddr);
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_CORRECT_FIELD:
            if (MT_DRV_BUF_ADDR_MAX == enLR)
            {
                MT_BOOL bTopFirst;
                bTopFirst = *(MT_BOOL *)pstInfo;
                if(bTopFirst == pstEntity->stStreamInfo.u32RealTopFirst)
                {
                    s32Ret = MT_SUCCESS;
                    goto GET_DONE;
                }

                if(pstEntity->stStreamInfo.u32RealTopFirst == DEF_TOPFIRST_BUTT)
                {
                    pstEntity->stStreamInfo.u32RealTopFirst = bTopFirst;
                    s32Ret = MT_SUCCESS;
                    goto GET_DONE;
                }

                s32Ret = VPSS_IMG_CorrectListOrder(pstEntity->pstSrcImagesList, bTopFirst);

                pstEntity->stStreamInfo.u32RealTopFirst = bTopFirst;

            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        default:
            VPSS_ERROR("Invalid Info Type %d\n",enType);
            s32Ret = MT_FAILURE;
    }

GET_DONE:
    return s32Ret;
}
mt_s32 VPSS_IN_GetInfo_V2(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_INFO_TYPE_E enType,
                                    MT_DRV_BUF_ADDR_E enLR,
                                    mt_void* pstInfo)
{
    mt_s32 s32Ret = MT_SUCCESS;
    VPSS_IN_STREAM_INFO_S *pstStreamInfo;

    VPSS_CHECK_NULL(pstEntity);

    VPSS_CHECK_VERSION(pstEntity->enVersion,VPSS_VERSION_V2_0);

    switch(enType)
    {
        case VPSS_IN_INFO_SRC:
            if (MT_DRV_BUF_ADDR_MAX == enLR)
            {

            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_WBC:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                MT_DRV_VIDEO_FRAME_S **ppstData;
                VPSS_WBC_S* pstWbc;

                ppstData = (MT_DRV_VIDEO_FRAME_S **)pstInfo;

                pstWbc = (VPSS_WBC_S*)pstEntity->pstWbcInfo[enLR];

                s32Ret = VPSS_WBC_GetWbcInfo(pstWbc, ppstData);
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_REF:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                MT_DRV_VIDEO_FRAME_S** ppstRef;
                VPSS_WBC_S* pstWbc;

                ppstRef = (MT_DRV_VIDEO_FRAME_S **)pstInfo;

                pstWbc = (VPSS_WBC_S*)pstEntity->pstWbcInfo[enLR];

                s32Ret = VPSS_WBC_GetRefInfo(pstWbc, ppstRef);
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_STREAM:
            if (MT_DRV_BUF_ADDR_MAX == enLR)
            {
                pstStreamInfo = (VPSS_IN_STREAM_INFO_S *)pstInfo;
                memcpy(pstStreamInfo,&(pstEntity->stStreamInfo),
                        sizeof(VPSS_IN_STREAM_INFO_S));
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_WBCREG_VA:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                VPSS_WBC_S *pstWbc;
                mt_u32 *pu32VirAddr;

                pstWbc = (VPSS_WBC_S*)pstEntity->pstWbcInfo[enLR];

                pu32VirAddr = (mt_u32 *)pstInfo;

                *pu32VirAddr = pstWbc->stMMZBuf.u32StartVirAddr;

                s32Ret = MT_SUCCESS;
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_WBCREG_PA:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                VPSS_WBC_S *pstWbc;
                mt_u32 *pu32VirAddr;

                pstWbc = (VPSS_WBC_S*)pstEntity->pstWbcInfo[enLR];

                pu32VirAddr = (mt_u32 *)pstInfo;

                *pu32VirAddr = pstWbc->stMMZBuf.u32StartPhyAddr;

                s32Ret = MT_SUCCESS;
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_NR:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                VPSS_NRMADCFG_S *pstNrCfg;
                VPSS_NRMADINFO_S* pstNrMadInfo;

                pstNrCfg = (VPSS_NRMADCFG_S *)pstInfo;
                pstNrMadInfo = (VPSS_NRMADINFO_S*)pstEntity->pstNrMadInfo[enLR];

                s32Ret = VPSS_STTINFO_NrGetInfo(pstNrMadInfo, pstNrCfg);
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_DIE:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                VPSS_DIESTINFO_S* pstDieStInfo;
                VPSS_DIESTCFG_S* pstDieStCfg;


                pstDieStCfg = (VPSS_DIESTCFG_S *)pstInfo;
                pstDieStInfo = (VPSS_DIESTINFO_S*)pstEntity->pstDieStInfo[enLR];

                s32Ret = VPSS_STTINFO_DieGetInfo(pstDieStInfo, pstDieStCfg);
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_CCREF:
            if (MT_DRV_BUF_ADDR_MAX == enLR)
            {
                VPSS_SRC_S *pstSrc;
                VPSS_SRC_DATA_S *pPtrPreData = MT_NULL;
                VPSS_SRC_DATA_S *pPtrPpreData = MT_NULL;
                VPSS_SRC_DATA_S **pstCCCl;
                pstSrc = (VPSS_SRC_S*)pstEntity->pstSrc;

                pstCCCl = (VPSS_SRC_DATA_S **)pstInfo;

                s32Ret = VPSS_SRC_GetPreImgInfo(pstSrc, &pPtrPreData, &pPtrPpreData);
                if (MT_SUCCESS == s32Ret)
                {
                    pstCCCl[0] = pPtrPreData;
                    pstCCCl[1] = pPtrPpreData;
                    s32Ret = MT_SUCCESS;
                }
                else
                {
                    s32Ret = MT_FAILURE;
                }
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_WBCMODE:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                VPSS_WBC_S *pstWbcInfo;
                VPSS_WBC_MODE_E *penMode;

                pstWbcInfo = (VPSS_WBC_S*)pstEntity->pstWbcInfo[enLR];

                penMode = (VPSS_WBC_MODE_E *)pstInfo;

                *penMode = pstWbcInfo->stWbcAttr.enMode;

                s32Ret = MT_SUCCESS;
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        case VPSS_IN_INFO_CCCL:
            if (MT_DRV_BUF_ADDR_LEFT == enLR
                || MT_DRV_BUF_ADDR_RIGHT == enLR )
            {
                VPSS_CCCLCNTINFO_S* pstCcclCntInfo;
                VPSS_CCCLCNTCFG_S* pstCcclCntCfg;

                pstCcclCntCfg = (VPSS_CCCLCNTCFG_S *)pstInfo;
                pstCcclCntInfo = (VPSS_CCCLCNTINFO_S*)pstEntity->pstCcclCntInfo[enLR];

                s32Ret = VPSS_STTINFO_CcclGetInfo(pstCcclCntInfo,pstCcclCntCfg);
            }
            else
            {
                VPSS_ERROR("Invalid L/R Type %d\n",enLR);
                s32Ret = MT_FAILURE;
            }
            break;
        default:
            VPSS_ERROR("Invalid Info Type %d\n",enType);
            s32Ret = MT_FAILURE;
    }

    return s32Ret;
}

mt_s32 VPSS_IN_CompleteImage_V1(VPSS_IN_ENTITY_S *pstEntity)
{
    //printk("GGGG VPSS_IN_CompleteImage_V1 \n");//yihua
    VPSS_IMG_Complete(pstEntity->pstSrcImagesList);

    return MT_SUCCESS;
}
mt_s32 VPSS_IN_CompleteImage_V2(VPSS_IN_ENTITY_S *pstEntity)
{
    VPSS_SRC_S* pstSrcInfo;
    VPSS_WBC_S* pstWbcInfo;
    VPSS_STTWBC_S* psttWbc;
    VPSS_DIESTINFO_S* pstDieStInfo;
    VPSS_CCCLCNTINFO_S* pstCcclCntInfo;
    VPSS_NRMADINFO_S* pstNrMadInfo;

    VPSS_CHECK_NULL(pstEntity);
    VPSS_CHECK_NULL(pstEntity->pstSrc);
    VPSS_CHECK_NULL(pstEntity->pstWbcInfo);
    VPSS_CHECK_NULL(pstEntity->pstSttWbc);
    VPSS_CHECK_NULL(pstEntity->pstDieStInfo);
    VPSS_CHECK_NULL(pstEntity->pstCcclCntInfo);
    VPSS_CHECK_NULL(pstEntity->pstNrMadInfo);


    pstSrcInfo = pstEntity->pstSrc;
    pstWbcInfo = pstEntity->pstWbcInfo[0];

    psttWbc = pstEntity->pstSttWbc[0];
    pstDieStInfo = pstEntity->pstDieStInfo[0];
    pstCcclCntInfo = pstEntity->pstCcclCntInfo[0];
    pstNrMadInfo = pstEntity->pstNrMadInfo[0];

    VPSS_SRC_CompleteImage(pstSrcInfo,MT_NULL);

    if(pstEntity->stStreamInfo.u32StreamW <= 1920)
    {
        VPSS_WBC_Complete(pstWbcInfo);
        VPSS_STTINFO_DieComplete(pstDieStInfo);
        VPSS_STTINFO_CcclComplete(pstCcclCntInfo);
        VPSS_STTINFO_NrComplete(pstNrMadInfo);
        VPSS_STTINFO_SttWbcComplete(psttWbc);

        if(pstEntity->stStreamInfo.eStreamFrmType != MT_DRV_FT_NOT_STEREO)
        {
            VPSS_WBC_Complete(pstWbcInfo + 1);
            VPSS_STTINFO_DieComplete(pstDieStInfo + 1);
            VPSS_STTINFO_CcclComplete(pstCcclCntInfo + 1);
            VPSS_STTINFO_NrComplete(pstNrMadInfo + 1);
            VPSS_STTINFO_SttWbcComplete(psttWbc + 1);
        }
    }
    else
    {
        VPSS_STTINFO_SttWbcComplete(psttWbc);
        VPSS_STTINFO_SttWbcComplete(psttWbc+1);
    }

    return MT_SUCCESS;
}

//仅Reset IMG队列,不Reset FB队列.
//VPSS_INST_Reset -> VPSS_INST_ResetPort -> VPSS_FB_Reset 负责Reset FB队列.
mt_s32 VPSS_IN_Reset_V1(VPSS_IN_ENTITY_S *pstEntity)
{
    VPSS_IMG_Reset(pstEntity->pstSrcImagesList);
	return MT_SUCCESS;
}
mt_s32 VPSS_IN_Reset_V2(VPSS_IN_ENTITY_S *pstEntity)
{
    VPSS_SRC_S* pstSrcInfo;
    VPSS_WBC_S* pstWbcInfo;

    VPSS_STTWBC_S* psttWbc;
    VPSS_DIESTINFO_S* pstDieStInfo;
    VPSS_CCCLCNTINFO_S* pstCcclCntInfo;
    VPSS_NRMADINFO_S* pstNrMadInfo;

    VPSS_CHECK_NULL(pstEntity);
    VPSS_CHECK_NULL(pstEntity->pstSrc);
    VPSS_CHECK_NULL(pstEntity->pstWbcInfo);
    VPSS_CHECK_NULL(pstEntity->pstSttWbc);
    VPSS_CHECK_NULL(pstEntity->pstDieStInfo);
    VPSS_CHECK_NULL(pstEntity->pstCcclCntInfo);
    VPSS_CHECK_NULL(pstEntity->pstNrMadInfo);

    pstSrcInfo = pstEntity->pstSrc;
    pstWbcInfo = pstEntity->pstWbcInfo[0];
    psttWbc = pstEntity->pstSttWbc[0];
    pstDieStInfo = pstEntity->pstDieStInfo[0];
    pstCcclCntInfo = pstEntity->pstCcclCntInfo[0];
    pstNrMadInfo = pstEntity->pstNrMadInfo[0];

    /*reset image list*/
    //:TODO:在调用此接口时，VPSS正在处理数据，如何保证同步,加入锁策略
    VPSS_SRC_Reset(pstSrcInfo);

    if(pstEntity->enMode == VPSS_IN_MODE_USRACTIVE)
    {
        //:TODO:拉模式下上层是否会调用RESET
    }

    if(pstEntity->stStreamInfo.u32StreamW <= 1920)
    {
        VPSS_WBC_Reset(pstWbcInfo);
        VPSS_STTINFO_DieReset(pstDieStInfo);
        VPSS_STTINFO_CcclReset(pstCcclCntInfo);
        VPSS_STTINFO_NrReset(pstNrMadInfo);
        VPSS_STTINFO_SttWbcReset(psttWbc);

        if(pstEntity->stStreamInfo.eStreamFrmType != MT_DRV_FT_NOT_STEREO)
        {
            VPSS_WBC_Reset(pstWbcInfo+1);
            VPSS_STTINFO_DieReset(pstDieStInfo+1);
            VPSS_STTINFO_CcclReset(pstCcclCntInfo+1);
            VPSS_STTINFO_NrReset(pstNrMadInfo+1);
            VPSS_STTINFO_SttWbcReset(psttWbc+1);
        }
    }
    else
    {
        VPSS_STTINFO_SttWbcReset(psttWbc);
        VPSS_STTINFO_SttWbcReset(psttWbc+1);
    }

    #if 0
    if(pstEntity->enMode == VPSS_IN_MODE_USRACTIVE)
    {
        VPSS_SRCIN_DeInit(&pstEntity->stSrcIn);
    }
    #endif

    if(pstEntity->stStreamInfo.u32StreamW <= 1920)
    {
        VPSS_WBC_DeInit(pstWbcInfo);
        VPSS_STTINFO_DieDeInit(pstDieStInfo);
        VPSS_STTINFO_CcclDeInit(pstCcclCntInfo);
        VPSS_STTINFO_NrDeInit(pstNrMadInfo);
        VPSS_STTINFO_SttWbcDeInit(psttWbc);

        if(pstEntity->stStreamInfo.eStreamFrmType != MT_DRV_FT_NOT_STEREO)
        {
            VPSS_WBC_DeInit(pstWbcInfo + 1);
            VPSS_STTINFO_DieDeInit(pstDieStInfo + 1);
            VPSS_STTINFO_CcclDeInit(pstCcclCntInfo + 1);
            VPSS_STTINFO_NrDeInit(pstNrMadInfo + 1);
            VPSS_STTINFO_SttWbcDeInit(psttWbc + 1);
        }
    }
	else
	{
		VPSS_STTINFO_SttWbcDeInit(psttWbc);
		VPSS_STTINFO_SttWbcDeInit(psttWbc+1);
	}

	memset(&(pstEntity->stStreamInfo),0,sizeof(VPSS_IN_STREAM_INFO_S));


	return MT_SUCCESS;
}

mt_s32 VPSS_IN_GetIntf(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_INTF_S *pstIntf)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VPSS_CHECK_NULL(pstEntity);

    if (pstEntity->enVersion == VPSS_VERSION_V2_0)
    {
        pstIntf->pfnRefresh = VPSS_IN_Refresh_V2;
        pstIntf->pfnCompleteImage = VPSS_IN_CompleteImage_V2;
        pstIntf->pfnGetProcessImage = VPSS_IN_GetProcessImage_V2;
        pstIntf->pfnReset =  VPSS_IN_Reset_V2;
        pstIntf->pfnGetInfo = VPSS_IN_GetInfo_V2;
    }
    else if (pstEntity->enVersion == VPSS_VERSION_V1_0)
    {
        pstIntf->pfnRefresh = VPSS_IN_Refresh_V1;
        pstIntf->pfnCompleteImage = VPSS_IN_CompleteImage_V1;
        pstIntf->pfnGetProcessImage = VPSS_IN_GetProcessImage_V1;
        pstIntf->pfnReset =  VPSS_IN_Reset_V1;
        pstIntf->pfnGetInfo = VPSS_IN_GetInfo_V1;
    }
    else
    {

        VPSS_ERROR("Version %d can't support\n",pstEntity->enVersion);
        s32Ret = MT_FAILURE;
    }

    return s32Ret;
}

mt_s32 VPSS_IN_Init(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_ENV_S stEnv)
{
    mt_s32 s32Ret = MT_SUCCESS;
    VPSS_CHECK_NULL(pstEntity);

    pstEntity->stStreamInfo.u32RealTopFirst = DEF_TOPFIRST_BUTT;

    if (stEnv.enVersion == VPSS_VERSION_V2_0)
    {
        pstEntity->enVersion = VPSS_VERSION_V2_0;

        s32Ret = VPSS_IN_VMALLOC_V2(pstEntity);
        if (MT_SUCCESS != s32Ret)
        {
           goto INIT_ERROR;
        }

        VPSS_OSAL_InitSpin(&(pstEntity->stSrcSpin));
    }
    else if (stEnv.enVersion == VPSS_VERSION_V1_0)
    {
        s32Ret = VPSS_IN_VMALLOC_V1(pstEntity);
        if (MT_SUCCESS != s32Ret)
        {
           goto INIT_ERROR;
        }
    }
    else
    {
        VPSS_ERROR("Version %d can't support\n",stEnv.enVersion);
        s32Ret = MT_FAILURE;
    }

    return s32Ret;


INIT_ERROR:
    if (stEnv.enVersion == VPSS_VERSION_V2_0)
    {
        (mt_void)VPSS_IN_VFREE_V2(pstEntity);
    }
    else if (stEnv.enVersion == VPSS_VERSION_V1_0)
    {
        (mt_void)VPSS_IN_VFREE_V1(pstEntity);
    }
    else
    {

    }
    return MT_FAILURE;
}

mt_s32 VPSS_IN_DeInit(VPSS_IN_ENTITY_S *pstEntity)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VPSS_CHECK_NULL(pstEntity);

    if (pstEntity->enVersion == VPSS_VERSION_V2_0)
    {
        VPSS_SRC_S* pstSrcInfo;
        VPSS_WBC_S* pstWbcInfo;
        VPSS_STTWBC_S* psttWbc;
        VPSS_DIESTINFO_S* pstDieStInfo;
        VPSS_CCCLCNTINFO_S* pstCcclCntInfo;
        VPSS_NRMADINFO_S* pstNrMadInfo;
        unsigned long flags;

        VPSS_CHECK_NULL(pstEntity);
        VPSS_CHECK_NULL(pstEntity->pstSrc);
        VPSS_CHECK_NULL(pstEntity->pstWbcInfo[0]);
        VPSS_CHECK_NULL(pstEntity->pstSttWbc[0]);
        VPSS_CHECK_NULL(pstEntity->pstDieStInfo[0]);
        VPSS_CHECK_NULL(pstEntity->pstCcclCntInfo[0]);
        VPSS_CHECK_NULL(pstEntity->pstNrMadInfo[0]);

        pstSrcInfo = pstEntity->pstSrc;
        pstWbcInfo = pstEntity->pstWbcInfo[0];
        psttWbc = pstEntity->pstSttWbc[0];
        pstDieStInfo = pstEntity->pstDieStInfo[0];
        pstCcclCntInfo = pstEntity->pstCcclCntInfo[0];
        pstNrMadInfo = pstEntity->pstNrMadInfo[0];

        VPSS_OSAL_DownSpin(&(pstEntity->stSrcSpin),&flags);
        VPSS_SRC_DeInit(pstSrcInfo);
        VPSS_OSAL_UpSpin(&(pstEntity->stSrcSpin),&flags);

        // TODO: 推送模式
        if(pstEntity->enMode == VPSS_IN_MODE_USRACTIVE)
        {
            //VPSS_SRCIN_DeInit(&pstInstance->stSrcIn);
        }

        if(pstEntity->stStreamInfo.u32StreamW <= 1920)
        {
            VPSS_WBC_DeInit(pstWbcInfo);
            VPSS_STTINFO_DieDeInit(pstDieStInfo);
            VPSS_STTINFO_CcclDeInit(pstCcclCntInfo);
            VPSS_STTINFO_NrDeInit(pstNrMadInfo);
            VPSS_STTINFO_SttWbcDeInit(psttWbc);

            if(pstEntity->stStreamInfo.eStreamFrmType != MT_DRV_FT_NOT_STEREO)
            {
                VPSS_WBC_DeInit(pstEntity->pstWbcInfo[1]);
                VPSS_STTINFO_DieDeInit(pstEntity->pstDieStInfo[1]);
                VPSS_STTINFO_CcclDeInit(pstEntity->pstCcclCntInfo[1]);
                VPSS_STTINFO_NrDeInit(pstEntity->pstNrMadInfo[1]);
                VPSS_STTINFO_SttWbcDeInit(pstEntity->pstSttWbc[1]);
            }
        }
		else
		{
			VPSS_STTINFO_SttWbcDeInit(psttWbc);
			VPSS_STTINFO_SttWbcDeInit(pstEntity->pstSttWbc[1]);
		}

        s32Ret = VPSS_IN_VFREE_V2(pstEntity);
    }
    else if (pstEntity->enVersion == VPSS_VERSION_V1_0)
    {
        s32Ret = VPSS_IN_VFREE_V1(pstEntity);
    }
    else
    {
        VPSS_ERROR("VpssVersion %d can't support\n",pstEntity->enVersion);
        s32Ret = MT_FAILURE;
        goto DEINIT_OUT;

    }
DEINIT_OUT:
    return s32Ret;
}

mt_s32 VPSS_IN_SetAttr(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_ATTR_S stAttr)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VPSS_CHECK_NULL(pstEntity);

    pstEntity->enProgInfo = stAttr.enProgInfo;
    pstEntity->bProgRevise = stAttr.bProgRevise;
    pstEntity->bAlwaysFlushSrc = stAttr.bAlwaysFlushSrc;
    pstEntity->enSrcCS = stAttr.enSrcCS;
    return s32Ret;

}

mt_s32 VPSS_IN_GetAttr(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_ATTR_S *pstAttr)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VPSS_CHECK_NULL(pstEntity);

    pstAttr->enProgInfo = pstEntity->enProgInfo;
    pstAttr->bProgRevise = pstEntity->bProgRevise;
    pstAttr->bAlwaysFlushSrc = pstEntity->bAlwaysFlushSrc;

    return s32Ret;
}

mt_s32 VPSS_IN_SetSrcMode(VPSS_IN_ENTITY_S *pstEntity,VPSS_IN_SOURCE_S stMode)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VPSS_CHECK_NULL(pstEntity);

    pstEntity->hSource = stMode.hSource;
    pstEntity->enMode = stMode.enMode;
    pstEntity->pfnAcqCallback = stMode.pfnAcqCallback;
    pstEntity->pfnRlsCallback = stMode.pfnRlsCallback;

    if (pstEntity->enVersion == VPSS_VERSION_V1_0)
    {
        VPSS_IMG_CALLBACK_S stCallback;
        stCallback.hSrc = pstEntity->hSource;
        stCallback.pfnAcqImage = (PFN_IMG_FUNC)pstEntity->pfnAcqCallback;
        stCallback.pfnRlsImage = (PFN_IMG_FUNC)pstEntity->pfnRlsCallback;
        VPSS_IMG_Regist(pstEntity->pstSrcImagesList, stCallback);
    }
    return s32Ret;
}
#ifdef __cplusplus
 #if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
