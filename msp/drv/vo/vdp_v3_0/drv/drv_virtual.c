/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/sizes.h>	/*SZ_1K*/

#include "drv_virtual.h"
#include "drv_win_priv.h"
#include "mt_drv_sys.h"
#include "drv_venc_ext.h"
#include "mt_drv_module.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

mt_s32 VIR_BUFFER_Init(VIR_BUFFER_S *pstBuffer)
{
    memset(pstBuffer->stBufArray, 0,
           sizeof(MT_DRV_VIDEO_FRAME_S) * DEF_VIR_BUFFER_LENGTH);

    pstBuffer->u32Head = 0;
    pstBuffer->u32Tail = 0;

    pstBuffer->enType = MUTUAL_TYPE_SINKACTIVE;

    return MT_SUCCESS;
}
mt_s32 VIR_BUFFER_DeInit(VIR_BUFFER_S *pstBuffer)
{
    pstBuffer->u32Head = 0;
    pstBuffer->u32Tail = 0;

    pstBuffer->enType = MUTUAL_TYPE_BUTT;

    return MT_SUCCESS;
}

mt_s32 VIR_BUFFER_Reset(VIR_BUFFER_S *pstBuffer)
{
    memset(pstBuffer->stBufArray, 0,
           sizeof(MT_DRV_VIDEO_FRAME_S) * DEF_VIR_BUFFER_LENGTH);
    pstBuffer->u32Head = 0;
    pstBuffer->u32Tail = 0;
    return MT_SUCCESS;
}

mt_s32 VIR_BUFFER_SetType(VIR_BUFFER_S *pstBuffer, MUTUAL_TYPE_E enType)
{
    if (pstBuffer->enType != MUTUAL_TYPE_BUTT)
    {
        WIN_ERROR("Virtual Window buffer type can't be changed.\n");
        return MT_FAILURE;
    }

    pstBuffer->enType = enType;

    return MT_SUCCESS;
}

mt_s32 VIR_BUFFER_GetFrm(VIR_BUFFER_S *pstBuffer, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    MUTUAL_TYPE_E enType;
    mt_s32 s32Ret;
    mt_u32 u32Head;
    mt_u32 u32Tail;
    MT_DRV_VIDEO_FRAME_S *pstArrayFrm;

    enType = pstBuffer->enType;

    switch (enType)
    {
        case MUTUAL_TYPE_SRCACTIVE:
            WIN_ERROR("MUTUAL_TYPE_SRCACTIVE Can't be supported.\n");
            s32Ret = MT_FAILURE;
            break;
        case MUTUAL_TYPE_SINKACTIVE:
            u32Head = pstBuffer->u32Head;
            u32Tail = pstBuffer->u32Tail;
            if (u32Tail != u32Head) {
                pstArrayFrm = &(pstBuffer->stBufArray[u32Head]);
                memcpy(pstFrm, pstArrayFrm, sizeof(MT_DRV_VIDEO_FRAME_S));
                memset(pstArrayFrm, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
                pstBuffer->u32Head = (pstBuffer->u32Head + 1) % DEF_VIR_BUFFER_LENGTH;

                s32Ret = MT_SUCCESS;
            } else {
                s32Ret = MT_FAILURE;
            }
            break;
        default:
            WIN_ERROR("MUTUAL TYPE Can't be supported.\n");
            s32Ret = MT_FAILURE;
            break;
    }

    return s32Ret;
}

mt_s32 VIR_BUFFER_AddFrm(VIR_BUFFER_S *pstBuffer, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    MUTUAL_TYPE_E enType;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Head;
    mt_u32 u32Tail;
    MT_DRV_VIDEO_FRAME_S *pstArrayFrm;

    enType = pstBuffer->enType;

    switch (enType)
    {
        case MUTUAL_TYPE_SRCACTIVE:
            WIN_ERROR("MUTUAL_TYPE_SRCACTIVE Can't be supported.\n");
            s32Ret = MT_FAILURE;
            break;
        case MUTUAL_TYPE_SINKACTIVE:
            //while(s32Ret == MT_FAILURE){
            u32Head = pstBuffer->u32Head;
            u32Tail = pstBuffer->u32Tail;
            if ((u32Tail + 1) % DEF_VIR_BUFFER_LENGTH != u32Head)
            {
                pstArrayFrm = &(pstBuffer->stBufArray[u32Tail]);
                memcpy(pstArrayFrm, pstFrm, sizeof(MT_DRV_VIDEO_FRAME_S));
                pstBuffer->u32Tail = (pstBuffer->u32Tail + 1) % DEF_VIR_BUFFER_LENGTH;
                s32Ret = MT_SUCCESS;
            }
            else
            {
                s32Ret = MT_FAILURE;
                            //printk("[%s]head %d,tail %d\n",__FUNCTION__,u32Head,u32Tail);
            }
                 //}
            break;
        default:
            WIN_ERROR("MUTUAL TYPE Can't be supported.\n");
            s32Ret = MT_FAILURE;
            break;
    }

    return s32Ret;
}

mt_s32 WIN_VIR_Create(MT_DRV_WIN_ATTR_S *pWinAttr, VIRTUAL_S **ppstVirWin)
{
    VIRTUAL_S *pstVirWin;

    pstVirWin = (VIRTUAL_S *)DISP_MALLOC(sizeof(VIRTUAL_S));
    if (!pstVirWin)
    {
        WIN_ERROR("Malloc VIRTUAL_S failed in %s!\n", __FUNCTION__);
        return MT_ERR_VO_MALLOC_FAILED;
    }

    DISP_MEMSET(pstVirWin, 0, sizeof(VIRTUAL_S));

    pstVirWin->stAttrBuf = *pWinAttr;

    pstVirWin->stAttrBuf.stCropRect.u32BottomOffset = 0;
    pstVirWin->stAttrBuf.stCropRect.u32LeftOffset = 0;
    pstVirWin->stAttrBuf.stCropRect.u32RightOffset = 0;
    pstVirWin->stAttrBuf.stCropRect.u32TopOffset = 0;

    atomic_set(&pstVirWin->bNewAttrFlag, 1);

    VIR_BUFFER_Init(&(pstVirWin->stBuffer));

    pstVirWin->enBufType = MUTUAL_TYPE_SINKACTIVE;
    pstVirWin->stSrcInfo.hSrc = MT_INVALID_HANDLE;
    pstVirWin->stSrcInfo.pfAcqFrame = MT_NULL;
    pstVirWin->stSrcInfo.pfRlsFrame = MT_NULL;
    pstVirWin->stSrcInfo.pfSendWinInfo = MT_NULL;
    pstVirWin->hSink = MT_INVALID_HANDLE;
    pstVirWin->pfnQueueFrm = MT_NULL;
    pstVirWin->pfnDequeueFrame = MT_NULL;
    pstVirWin->enRotation = MT_DRV_ROT_ANGLE_0;
    pstVirWin->bHoriFlip = MT_FALSE;
    pstVirWin->bVertFlip = MT_FALSE;
    *ppstVirWin = pstVirWin;

    return MT_SUCCESS;
}

mt_s32 WIN_VIR_Destroy(VIRTUAL_S *pstVirWin)
{

    WinCheckNullPointer(pstVirWin);

    VIR_BUFFER_DeInit(&(pstVirWin->stBuffer));

    DISP_FREE(pstVirWin);

    return MT_SUCCESS;
}

mt_s32 WIN_VIR_Reset(VIRTUAL_S *pstVirWin)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MT_DRV_VIDEO_FRAME_S stFrame;
    mt_handle hSrc;
    if (pstVirWin->stBuffer.enType == MUTUAL_TYPE_SRCACTIVE)
    {
        s32Ret = MT_SUCCESS;
    }
    else if (pstVirWin->stBuffer.enType == MUTUAL_TYPE_SINKACTIVE)
    {
        if (0)
        {
            DISP_ERROR("Window is working ,can't reset %d\n", pstVirWin->stBuffer.enType);
            s32Ret = MT_ERR_VO_INVALID_OPT;
        }
        else
        {
            s32Ret = VIR_BUFFER_GetFrm(&(pstVirWin->stBuffer), &stFrame);

            while (MT_SUCCESS == s32Ret)
            {
                hSrc = pstVirWin->stSrcInfo.hSrc;
                WinCheckNullPointer(pstVirWin->stSrcInfo.pfRlsFrame);

                (mt_void) pstVirWin->stSrcInfo.pfRlsFrame(hSrc, &stFrame);

                s32Ret = VIR_BUFFER_GetFrm(&(pstVirWin->stBuffer), &stFrame);
            }

            (mt_void) VIR_BUFFER_Reset(&(pstVirWin->stBuffer));

            s32Ret = MT_SUCCESS;
        }
    }
    else
    {
        DISP_ERROR("Invalid Buffer Type %d\n", pstVirWin->stBuffer.enType);
        s32Ret = MT_ERR_VO_INVALID_OPT;
    }
    return s32Ret;
}

mt_s32 WIN_VIR_SendAttrToSource(VIRTUAL_S *pstVirWin)
{
    MT_DRV_WIN_PRIV_INFO_S stWinPriv;

    memset(&stWinPriv, 0, sizeof(MT_DRV_WIN_PRIV_INFO_S));

    if (pstVirWin->stSrcInfo.pfSendWinInfo != MT_NULL)
    {
        //stWinPriv.ePixFmt = pstVirWin->stAttrBuf.enDataFormat;
        stWinPriv.ePixFmt = MT_DRV_PIX_FMT_NV21;

        //Max Frame Rate 1000 means 1000fps
        stWinPriv.u32MaxRate = 1000;

        stWinPriv.stOutRect.s32X = 0;
        stWinPriv.stOutRect.s32Y = 0;
        if ((pstVirWin->u32Width == 0 && pstVirWin->u32Height == 0) || pstVirWin->u32Width > 1920 || pstVirWin->u32Height > 1088)
        {
            stWinPriv.stOutRect.s32Height = 1080;
            stWinPriv.stOutRect.s32Width = 1920;
        }
        else
        {
            stWinPriv.stOutRect.s32Height = pstVirWin->u32Height;
            stWinPriv.stOutRect.s32Width = pstVirWin->u32Width;
        }
        stWinPriv.stCropRect = pstVirWin->stAttrBuf.stCropRect;
        stWinPriv.stInRect = pstVirWin->stAttrBuf.stInRect;
        stWinPriv.bUseCropRect = pstVirWin->stAttrBuf.bUseCropRect;

        stWinPriv.stScreen = stWinPriv.stOutRect;

        stWinPriv.stScreenAR.u32ARh = 0;
        stWinPriv.stScreenAR.u32ARw = 0;

        stWinPriv.stCustmAR = pstVirWin->stAttrBuf.stCustmAR;
        stWinPriv.enARCvrs = pstVirWin->stAttrBuf.enARCvrs;
        stWinPriv.bTunnelSupport = MT_TRUE;
        stWinPriv.bHoriFlip = pstVirWin->bHoriFlip;
        stWinPriv.bVertFlip = pstVirWin->bVertFlip;
        stWinPriv.enRotation = pstVirWin->enRotation;

        pstVirWin->stSrcInfo.pfSendWinInfo(pstVirWin->stSrcInfo.hSrc, &stWinPriv);
    }
    else
    {
    }
    return MT_SUCCESS;
}
mt_s32 WIN_VIR_SetAttr(VIRTUAL_S *pstVirWin, MT_DRV_WIN_ATTR_S *pWinAttr)
{
    atomic_set(&pstVirWin->bNewAttrFlag, 0);

    pstVirWin->stAttrBuf = *pWinAttr;

    atomic_set(&pstVirWin->bNewAttrFlag, 1);

    WIN_VIR_SendAttrToSource(pstVirWin);

    return MT_SUCCESS;
}
mt_s32 WIN_VIR_SetSize(VIRTUAL_S *pstVirWin, mt_u32 u32Width, mt_u32 u32Height)
{

    if (u32Width > 1920 || u32Height > 1088 || u32Width < 64 || u32Height < 64)
    {
        WIN_ERROR("Set Virtual Win Invalid H %d and W %d ,only support 64*64 ~ 1280*720.\n",
            u32Height, u32Width);
        return MT_FAILURE;
    }
    pstVirWin->u32Width = u32Width;
    pstVirWin->u32Height = u32Height;

    WIN_VIR_SendAttrToSource(pstVirWin);

    return MT_SUCCESS;
}

mt_s32 WIN_VIR_GetFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (pstVirWin->bEnable == MT_TRUE)
    {
        switch (pstVirWin->enBufType)
        {
            case MUTUAL_TYPE_SRCACTIVE:
                WIN_ERROR("MUTUAL_TYPE_SRCACTIVE Can't be supported.\n");
                break;
            case MUTUAL_TYPE_SINKACTIVE:
                s32Ret = VIR_BUFFER_GetFrm(&(pstVirWin->stBuffer), pstFrm);
                //if(s32Ret == MT_SUCCESS)
                // printk("get frm 2:ret %d,no %d  id %d w %d, h %d\n",s32Ret,
                //   pstFrm->u32FrameNo,pstFrm->u32FrameIndex,pstFrm->u32Width,pstFrm->u32Height);

                break;
            default:
                WIN_ERROR("MUTUAL TYPE Can't be supported.\n");
                break;
        }
    }
    else
    {
        WIN_ERROR("drv_virtual,getfrm: Window is disabled.\n");
        s32Ret = MT_FAILURE;
    }

    return s32Ret;
}

mt_s32 WIN_VIR_RelFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_handle hSrc;

    if (pstVirWin->bEnable == MT_TRUE)
    {
        hSrc = pstVirWin->stSrcInfo.hSrc;
        WinCheckNullPointer(pstVirWin->stSrcInfo.pfRlsFrame);

        s32Ret = pstVirWin->stSrcInfo.pfRlsFrame(hSrc, pstFrm);
    }
    else
    {
        WIN_ERROR("Window is disabled.\n");
        s32Ret = MT_FAILURE;
    }

    return s32Ret;
}

/*AVPLAY put in frame*/
mt_s32 WIN_VIR_AddNewFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    mt_handle hSink;
    mt_handle hSrc;
    FN_VENC_PUT_FRAME pfnQueueFrm;

    if (pstVirWin->bEnable == MT_TRUE)
    {
        switch (pstVirWin->enBufType)
        {
            case MUTUAL_TYPE_SRCACTIVE:
                hSink = pstVirWin->hSink;
                WinCheckNullPointer(pstVirWin->pfnQueueFrm);
                pfnQueueFrm = (FN_VENC_PUT_FRAME)pstVirWin->pfnQueueFrm;

                s32Ret = pfnQueueFrm(hSink, pstFrm);

                MT_INFO_VO("Qfrm 1:ret %d,no %d  [w%d, h%d]\n",s32Ret,
                   pstFrm->u32FrameIndex, pstFrm->slotInfo.pic_width,
                   pstFrm->slotInfo.pic_height);
                break;
            case MUTUAL_TYPE_SINKACTIVE:
                s32Ret = VIR_BUFFER_AddFrm(&(pstVirWin->stBuffer), pstFrm);

                // printk("Qfrm 2:ret %d,no %d  [w%d, h%d]\n",s32Ret,
                //  pstFrm->u32FrameNo,pstFrm->slotInfo.pic_width,
                //  pstFrm->slotInfo.pic_height);

                break;
            default:
                WIN_ERROR("MUTUAL TYPE Can't be supported.\n");
                return MT_FAILURE;
                break;
        }
    }
    else
    {
        s32Ret = MT_FAILURE;
    }

    if (MT_FAILURE == s32Ret)
    {
        hSrc = pstVirWin->stSrcInfo.hSrc;
        WinCheckNullPointer(pstVirWin->stSrcInfo.pfRlsFrame);
        s32Ret = pstVirWin->stSrcInfo.pfRlsFrame(hSrc, pstFrm);
        s32Ret = MT_SUCCESS;
    }

    return s32Ret;
}

mt_s32 WIN_VIR_AddUlsFrm(VIRTUAL_S *pstVirWin, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    mt_handle hSrc;

    hSrc = pstVirWin->stSrcInfo.hSrc;
    WinCheckNullPointer(pstVirWin->stSrcInfo.pfRlsFrame);
    s32Ret = pstVirWin->stSrcInfo.pfRlsFrame(hSrc, pstFrm);

    return s32Ret;
}

MT_BOOL WinCheckVirtual(mt_u32 u32WinIndex)
{
    if (WIN_INDEX_PREFIX != WinGetPrefix(u32WinIndex))
    {
        WIN_ERROR("Invalid Handle.\n");
        return MT_FALSE;
    }

    if (WIN_INDEX_VIRTUAL_CHANNEL == WinGetDispId(u32WinIndex))
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}

mt_s32 WIN_VIR_DetachSink(VIRTUAL_S *pstVirWin, mt_handle hSink)
{
    mt_u32 enModID;
    mt_s32 s32Ret;

    enModID = (mt_u32)((hSink & 0xff0000) >> 16);
    if (pstVirWin->stSrcInfo.hSrc != MT_INVALID_HANDLE)
    {
        WIN_ERROR("Sink module can't be detached,please detach source first\n");
        return MT_FAILURE;
    }
    if (hSink == pstVirWin->hSink)
    {
        pstVirWin->hSink = MT_INVALID_HANDLE;
        pstVirWin->pfnQueueFrm = MT_NULL;
        pstVirWin->pfnDequeueFrame = MT_NULL;

        pstVirWin->enBufType = MUTUAL_TYPE_SINKACTIVE;
        pstVirWin->stBuffer.enType = MUTUAL_TYPE_SINKACTIVE;
        s32Ret = MT_SUCCESS;
    }
    else
    {
        WIN_ERROR("Invalid Detach Handle.\n");
        s32Ret = MT_FAILURE;
    }

    return s32Ret;
}

mt_s32 WIN_VIR_AttachSink(VIRTUAL_S *pstVirWin, mt_handle hSink)
{
    mt_u32 enModID;
    VENC_EXPORT_FUNC_S *pstVenFunc;
    mt_s32 s32Ret;

    if (pstVirWin->hSink != MT_INVALID_HANDLE)
    {
        WIN_ERROR("Virtual Window is already attached,hSink=%#x\n", pstVirWin->hSink);
        return MT_FAILURE;
    }
    enModID = (mt_u32)((hSink & 0xff0000) >> 16);

    if (MT_ID_VENC == enModID)
    {
        s32Ret = mt_drv_module_getfunction(enModID, (mt_void **)&(pstVenFunc)); //MT_DRV_MODULE_GetFunction

        if (MT_SUCCESS != s32Ret)
        {
            WIN_ERROR("get venc func error when in virtual win.\n");
            return MT_FAILURE;
        }

        pstVirWin->hSink = hSink;
        pstVirWin->pfnQueueFrm = pstVenFunc->pfnVencQueueFrame;
        pstVirWin->pfnDequeueFrame = MT_NULL;
        pstVirWin->enBufType = MUTUAL_TYPE_SRCACTIVE;
        pstVirWin->stBuffer.enType = MUTUAL_TYPE_SRCACTIVE;
        s32Ret = MT_SUCCESS;
    }
    else
    {
        s32Ret = MT_FAILURE;
    }

    return s32Ret;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
