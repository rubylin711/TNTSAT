
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_cast.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#include <linux/sizes.h>	/*SZ_1K*/

#include "drv_disp_com.h"
#include "drv_disp_cast.h"
#include "drv_venc_ext.h"
#include "mt_drv_module.h"
#include "drv_disp_priv.h"
#include "drv_display.h"
#include "mt_drv_stat.h"
#include "hd_enc_aria_reg.h"
#if defined(CONFIG_MT_CHIP_ARIA)
#include "MT_DF_Aria_reg.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#include "drv_disp_Symphony_reg.h"
#endif
#include "mt_module_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

CAST_RELEASE_PTR_S g_stReleasePtrArray[DISP_CAST_BUFFER_MAX_NUMBER];

static mt_s32 CastCheckCfg(MT_DRV_DISP_CAST_CFG_S *pstCfg, MT_DISP_DISPLAY_INFO_S *pstInfo)
{
#if 1
    if ((pstCfg->u32Width < DISP_CAST_MIN_W) || (pstCfg->u32Width > DISP_CAST_MAX_W) ||
        (pstCfg->u32Height < DISP_CAST_MIN_H) || (pstCfg->u32Height > DISP_CAST_MAX_H) ||
        ((pstCfg->u32Width & 0x1) != 0) || ((pstCfg->u32Height & 0x3) != 0))
    {
        DISP_ERROR("Cast w= %d or h=%d invalid\n", pstCfg->u32Width, pstCfg->u32Height);
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstCfg->eFormat != MT_DRV_PIX_FMT_NV21) && (pstCfg->eFormat != MT_DRV_PIX_FMT_NV12) &&
        (pstCfg->eFormat != MT_DRV_PIX_FMT_NV16_2X1) && (pstCfg->eFormat != MT_DRV_PIX_FMT_NV61_2X1) &&
        (pstCfg->eFormat != MT_DRV_PIX_FMT_YUYV) && (pstCfg->eFormat != MT_DRV_PIX_FMT_YVYU) &&
        (pstCfg->eFormat != MT_DRV_PIX_FMT_UYVY))
    {
        DISP_ERROR("Cast pixfmt = %d invalid\n", pstCfg->eFormat);
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstCfg->u32BufNumber < DISP_CAST_BUFFER_MIN_NUMBER) || (pstCfg->u32BufNumber > DISP_CAST_BUFFER_MAX_NUMBER))
    {
        DISP_ERROR("Cast u32BufNumber =%d invalid\n", pstCfg->u32BufNumber);
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (pstCfg->bUserAlloc)
    {
        DISP_ERROR("Cast not support User Alloc memory\n");
        return MT_ERR_DISP_NOT_SUPPORT;
    }
#endif
    return MT_SUCCESS;
}

static mt_s32 CastSetFrameDemoPartA(mt_handle cast_ptr,
                                    MT_DRV_DISP_CAST_CFG_S *pstCfg,
                                    DISP_CAST_ATTR_S *pstAttr)
{
#if 1
    MT_DRV_VIDEO_FRAME_S *pstFrame;
    DISP_CAST_PRIV_FRAME_S *pstPriv;

    // set frame demo
    DISP_MEMSET(pstAttr, 0, sizeof(DISP_CAST_ATTR_S));

    pstAttr->stOut.s32Width = (mt_s32)pstCfg->u32Width;
    pstAttr->stOut.s32Height = (mt_s32)pstCfg->u32Height;

    pstFrame = &pstAttr->stFrameDemo;
    pstFrame->eFrmType = MT_DRV_FT_NOT_STEREO;
    pstFrame->ePixFormat = pstCfg->eFormat;

    pstFrame->bProgressive = MT_TRUE;
    pstFrame->u32Width = pstCfg->u32Width;
    pstFrame->u32Height = pstCfg->u32Height;
    pstFrame->stDispRect = pstAttr->stOut;

    pstPriv = (DISP_CAST_PRIV_FRAME_S *)&(pstFrame->u32Priv[0]);

    pstPriv->cast_ptr = cast_ptr;
    pstPriv->stPrivInfo.u32PlayTime = 1;
#endif
    return MT_SUCCESS;
}

#define Cast_ERROR_DEAL(a, flag) \
    do {                         \
        if (MT_SUCCESS != (a)) { \
            return MT_FAILURE;   \
        }                        \
    } while (0);

static mt_s32 Cast_GetFrame(DISP_CAST_S *pstCast,
                            mt_u32 *u32BufId,
                            mt_u32 buf_type,
                            MT_DRV_VIDEO_FRAME_S *frameInfo)
{
#if 1
    if (buf_type == 1)
    {
        Cast_ERROR_DEAL(BP_GetFullBuf(&pstCast->stBP, u32BufId), 0);
        Cast_ERROR_DEAL(BP_DelFullBuf(&pstCast->stBP, *u32BufId), 1);
        Cast_ERROR_DEAL(BP_GetFrame(&pstCast->stBP, *u32BufId, frameInfo), 1);
    }
    else
    {
        Cast_ERROR_DEAL(BP_GetEmptyBuf(&pstCast->stBP, u32BufId), 0);
        Cast_ERROR_DEAL(BP_DelEmptyBuf(&pstCast->stBP, *u32BufId), 1);
        Cast_ERROR_DEAL(BP_GetFrame(&pstCast->stBP, *u32BufId, frameInfo), 1);
    }
#endif
    return MT_SUCCESS;
}

static MT_BOOL Cast_CheckOutputSizeChange(DISP_CAST_S *pstCast,
                                          MT_DRV_VIDEO_FRAME_S *pstFrame)
{
#if 1
    if ((pstFrame->u32Width == pstCast->stAttr.stOut.s32Width) && (pstFrame->u32Height == pstCast->stAttr.stOut.s32Height))
    {
        return MT_FALSE;
    }
    else
    {
        return MT_TRUE;
    }
#endif
    return MT_TRUE;
}

static mt_s32 Cast_KThread_ReAllocate(mt_void *pstCastmp)
{
#if 1
    mt_u32 i = 0;
    MT_DRV_VIDEO_FRAME_S stFrame;
    DISP_CAST_S *pstCast = MT_NULL;

    memset((void *)&stFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
    pstCast = (DISP_CAST_S *)pstCastmp;

    while (1)
    {
        if (kthread_should_stop())
            break;

        for (i = 0; i < DISP_CAST_BUFFER_MAX_NUMBER; i++)
        {
            /*need to reallocate.*/
            if (CAST_RETRIVE_NODE_REALLOCATE == atomic_read(&g_stReleasePtrArray[i].atReleaseNodeStatus))
            {
                if (BP_ReAllocBuf(&pstCast->stBP, g_stReleasePtrArray[i].u32BufID))
                    break;

                /*set the node to normal status.*/
                atomic_set(&g_stReleasePtrArray[i].atReleaseNodeStatus, CAST_RETRIVE_NODE_READY_TO_RETURN);
            }
        }

        msleep(10);
    }
#endif
    return MT_SUCCESS;
}

static mt_s32 Cast_ReturnFrameToList(DISP_CAST_S *pstCast)
{
#if 1
    mt_u32 i = 0;
    mt_s32 nRet = MT_SUCCESS;

    for (i = 0; i < DISP_CAST_BUFFER_MAX_NUMBER; i++)
    {
        if (CAST_RETRIVE_NODE_READY_TO_RETURN == atomic_read(&g_stReleasePtrArray[i].atReleaseNodeStatus))
        {
            nRet = BP_SetBufReading(&pstCast->stBP, g_stReleasePtrArray[i].u32BufID);
            if (nRet)
                break;

            nRet = BP_AddEmptyBuf(&pstCast->stBP, g_stReleasePtrArray[i].u32BufID);
            if (nRet)
                break;

            nRet = BP_SetBufEmpty(&pstCast->stBP, g_stReleasePtrArray[i].u32BufID);
            if (nRet)
                break;

            if (!g_stReleasePtrArray[i].bInternalRelease)
                pstCast->u32CastReleaseOkCnt++;

            atomic_set(&g_stReleasePtrArray[i].atReleaseNodeStatus, CAST_RETRIVE_NODE_EMPTY);
        }
    }
#endif
    return MT_SUCCESS;
}

static mt_s32 Cast_CreateKthread(DISP_CAST_S *pstCast)
{
#if 1
    pstCast->kThreadReleaseFrame = kthread_create(Cast_KThread_ReAllocate, pstCast, "MT_DISP_CastRelease");

    if (IS_ERR(pstCast->kThreadReleaseFrame))
    {
        DISP_ERROR("Cast create release thread failed!\n");
        return MT_ERR_DISP_CREATE_ERR;
    }
    else
    {
        wake_up_process(pstCast->kThreadReleaseFrame);
    }
#endif
    return MT_SUCCESS;
}

mt_s32 set_buf_val_1(mt_u8 * pbuf,mt_u32 pitch,mt_u32 height)
{
    int i,j;
    mt_u8 v = 0;
    for(j = 0; j < height;j++)
    {
        for(i =0; i < pitch; i++)
        {
            pbuf[j * pitch + i] = v;
            if(v == 0xff)
                v = 0;
            else
                v++;
        }
    }
    return MT_SUCCESS;
}

mt_s32 set_buf_val_2(mt_u8 * pbuf,mt_u32 pitch,mt_u32 height)
{
    int i,j;
    for(j = 0; j < height;j++)
    for(i =0; i < pitch; i++)
    {
       pbuf[j * pitch + i] = 0x7f;
    }
    return MT_SUCCESS;
}

mt_s32 DISP_CastCreate(MT_DRV_DISPLAY_E enDisp,
                       MT_DISP_DISPLAY_INFO_S *pstInfo,
                       MT_DRV_DISP_CAST_CFG_S *pstCfg,
                       mt_handle *cast_ptr)
{
#if 1
    MT_DRV_DISP_CALLBACK_TYPE_E eCallType;
    MT_DRV_DISP_CALLBACK_S stCB1, stCB2;
    DISP_CAST_S *pstCast;
    BUF_ALLOC_S stAlloc;
    mt_s32 nRet;

    // check cfg
    if (CastCheckCfg(pstCfg, pstInfo))
    {
        DISP_ERROR("Cast config invalid!\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    // alloc mem
    pstCast = (DISP_CAST_S *)DISP_MALLOC(sizeof(DISP_CAST_S));
    if (!pstCast)
    {
        DISP_ERROR("Cast malloc failed!\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    DISP_MEMSET(pstCast, 0, sizeof(DISP_CAST_S));

    // set attr
    CastSetFrameDemoPartA((mt_handle)pstCast, pstCfg, &pstCast->stAttr);

// get hal operation
#if 0
    nRet = DISP_HAL_GetOperation(&pstCast->stIntfOpt);
    if (nRet)
    {
        DISP_ERROR("Cast get hal operation failed!\n");
        goto _ERR_MALLOC;
    }

    // get wbclayer
    nRet = pstCast->stIntfOpt.PF_AcquireWbcByChn(enDisp, &pstCast->eWBC);
    if (nRet)
    {
        DISP_ERROR("Cast get wbc layer failed!\n");
        goto _ERR_MALLOC;
    }
#endif

    // create buffer
    stAlloc.bFbAllocMem = !pstCfg->bUserAlloc;
    stAlloc.eDataFormat = pstCfg->eFormat;
    stAlloc.u32BufWidth = pstCfg->u32Width;
    stAlloc.u32BufHeight = pstCfg->u32Height;
    stAlloc.u32BufStride = pstCfg->u32BufStride;
    stAlloc.u32BufSize = pstCfg->u32BufSize;
    nRet = BP_Create(pstCfg->u32BufNumber, &stAlloc, &pstCast->stBP);
    if (nRet)
    {
        DISP_ERROR("Cast alloc buffer failed!\n");
        goto _ERR_MALLOC;
    }
#if 1//def USE_PRE_LINEAR
    {
        mt_char BufName[10] = { 'V', 'D', 'P', '_', 'C', 'a', 's', 't', '1', '\0' };
        mt_u32 u  = 0;

        for (u = 0; u < DISP_CAST_OUTFRM_CNT_MAX; u++)
        {
            BufName[8] = (mt_char)('0' + u);

            nRet = DISP_OS_MMZ_AllocAndMap((const char *)BufName, MT_NULL, DISP_CAST_OUTFRM_BUFSZ * 2, 16, &(pstCast->stCastOutFrm[u].stMem));
            if (nRet)
            {
                DISP_ERROR("Cast alloc buffer failed 222!\n");
                goto _ERR_MALLOC;
            }

            pstCast->stCastOutFrm[u].stBufAddr[0].u32PhyAddr_Y = pstCast->stCastOutFrm[u].stMem.u32StartPhyAddr;
            pstCast->stCastOutFrm[u].stBufAddr[0].u32PhyAddr_C =  pstCast->stCastOutFrm[u].stBufAddr[0].u32PhyAddr_Y + DISP_CAST_OUTFRM_BUFSZ;
            pstCast->stCastOutFrm[u].u32RWStatus = CAST_OUTBUF_EMPTY;

            MT_INFO_VO("\n\n\n0x%x,0x%x\n\n\n\n",pstCast->stCastOutFrm[u].stBufAddr[0].u32PhyAddr_Y,pstCast->stCastOutFrm[u].stBufAddr[0].u32PhyAddr_C);


            set_buf_val_1((mt_u8 *)(pstCast->stCastOutFrm[u].stMem.u32StartVirAddr),(GRA_SCALE0_OUTSZ_W + 127) /128 * 128, GRA_SCALE0_OUTSZ_H);

            set_buf_val_1((mt_u8 *)((pstCast->stCastOutFrm[u].stMem.u32StartVirAddr + DISP_CAST_OUTFRM_BUFSZ)),(GRA_SCALE0_OUTSZ_W + 127) /128 * 128, GRA_SCALE0_OUTSZ_H/2);
        }
    }
#endif

    //create tmp outbuf

    // register callback
    stCB1.hDst = (mt_handle)pstCast;
    stCB1.pfDISP_Callback = DISP_CastCB_GenarateFrame;
    nRet = DISP_ISR_RegCallback(enDisp, MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB1);
    if (nRet)
    {
        DISP_ERROR("Cast register work callback failed!\n");
        goto _ERR_BP;
    }

    stCB2.hDst = (mt_handle)pstCast;
    stCB2.pfDISP_Callback = DISP_CastPushFrame;
    if (pstCfg->bLowDelay)
    {
        eCallType = MT_DRV_DISP_C_DHD0_WBC;
    }
    else
    {
        eCallType = MT_DRV_DISP_C_INTPOS_0_PERCENT;
    }

    nRet = DISP_ISR_RegCallback(enDisp, /*eCallType*/ MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB2);
    if (nRet)
    {
        DISP_ERROR("Cast register work callback failed!\n");
        goto __ERR_REG_CALLBACK1;
    }

    pstCast->u32LastCfgBufId = 0;
    pstCast->u32LastFrameBufId = 0;

    pstCast->bToGetDispInfo = MT_TRUE;
    pstCast->bOpen = MT_TRUE;
    pstCast->bLowDelay = pstCfg->bLowDelay;
    pstCast->eDisp = enDisp;
    pstCast->u32Ref = 1;

    pstCast->stConfig = *pstCfg;
    *cast_ptr = (mt_handle)pstCast;

    //Cast create release thread
    nRet = Cast_CreateKthread(pstCast);
    if (nRet)
        goto __ERR_REG_CALLBACK2;

    memset((void *)g_stReleasePtrArray,
           0,
           sizeof(CAST_RELEASE_PTR_S) * DISP_CAST_BUFFER_MAX_NUMBER);

    DISP_PRINT("DISP_CastCreate ok\n");
    return MT_SUCCESS;

__ERR_REG_CALLBACK2:
    DISP_ISR_UnRegCallback(enDisp, eCallType, &stCB2);
__ERR_REG_CALLBACK1:
    DISP_ISR_UnRegCallback(enDisp, MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB1);
_ERR_BP:
    BP_Destroy(&pstCast->stBP);
_ERR_MALLOC:
    DISP_FREE(pstCast);
    return nRet;
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_CastDestroy(mt_handle cast_ptr)
{
#if 1
    MT_DRV_DISP_CALLBACK_TYPE_E eCallType;
    MT_DRV_DISP_CALLBACK_S stCB1, stCB2;
    DISP_CAST_S *pstCast;
    mt_u32 u  = 0;

    pstCast = (DISP_CAST_S *)cast_ptr;

    if (pstCast->kThreadReleaseFrame)
    {
        kthread_stop(pstCast->kThreadReleaseFrame);
        pstCast->kThreadReleaseFrame = NULL;
    }

    // set disable
    pstCast->bEnable = MT_FALSE;
    /*release  buffer and stop intr func is asynchronus,
      so we should wait. after intr stop, then release the mmz mem,
       to avoid writing after release.*/
    msleep(100);

    stCB1.hDst = (mt_handle)cast_ptr;
    stCB1.pfDISP_Callback = DISP_CastCB_GenarateFrame;
    (mt_void) DISP_ISR_UnRegCallback(pstCast->eDisp, MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB1);

    if (pstCast->bLowDelay)
    {
        eCallType = MT_DRV_DISP_C_DHD0_WBC;
    }
    else
    {
        eCallType = MT_DRV_DISP_C_INTPOS_0_PERCENT;
    }

    stCB2.hDst = (mt_handle)cast_ptr;
    stCB2.pfDISP_Callback = DISP_CastPushFrame;
    (mt_void) DISP_ISR_UnRegCallback(pstCast->eDisp, eCallType, &stCB2);
    msleep(60);

#if 1//def USE_PRE_LINEAR

    for (u = 0; u < DISP_CAST_OUTFRM_CNT_MAX; u++)
    {
        DISP_OS_MMZ_UnmapAndRelease(&(pstCast->stCastOutFrm[u].stMem));
    }
#endif
// destroy buffer
    BP_Destroy(&pstCast->stBP);
#if 0
    pstCast->stIntfOpt.PF_ReleaseWbc(pstCast->eWBC);
#endif
    // free mem
    DISP_FREE(pstCast);
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_CastSetEnable(mt_handle cast_ptr, MT_BOOL bEnable)
{
#if defined(CONFIG_MT_CHIP_ARIA)
#if 1
    DISP_CAST_S *pstCast;

    pstCast = (DISP_CAST_S *)cast_ptr;

    pstCast->bEnable = bEnable;

    if(bEnable == MT_FALSE)
    {
        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_0(0);
        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_2(0);
    }

    DISP_PRINT("DISP_CastSetEnable  bEnable = 0x%x\n", (mt_u32)bEnable);
#endif
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_CastGetEnable(mt_handle cast_ptr, MT_BOOL *pbEnable)
{
#if 1
    DISP_CAST_S *pstCast;

    pstCast = (DISP_CAST_S *)cast_ptr;

    *pbEnable = pstCast->bEnable;
#endif
    return MT_SUCCESS;
}

mt_s32 DispCastSetFramePTS(MT_DRV_VIDEO_FRAME_S *pstCastFrame, MIRA_GET_PTS_E pts_flag)
{
#if 1
    DISP_CAST_PRIV_FRAME_S *pstPrivFrame;

    pstPrivFrame = (DISP_CAST_PRIV_FRAME_S *)&(pstCastFrame->u32Priv[0]);

    if (!pts_flag)
    {
        DISP_OS_GetTime(&pstPrivFrame->u32Pts0);
        pstCastFrame->u32Pts = pstPrivFrame->u32Pts0;
        pstCastFrame->u32SrcPts = pstPrivFrame->u32Pts0;
    }
    else
    {
        DISP_OS_GetTime(&pstPrivFrame->u32Pts1);
    }

#endif
    return MT_SUCCESS;
}

static mt_s32 Cast_AcquireFrame(DISP_CAST_S *pstCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
#if 1
    mt_s32 nRet;
    mt_u32 u32BufId = 0;
    nRet = Cast_GetFrame(pstCast, &u32BufId, 1, pstCastFrame);
    if (nRet)
    {
        DISP_WARN("Cast get id failed!\n");
        return nRet;
    }

    DispCastSetFramePTS(pstCastFrame, MIRA_SET_AQUIRE_PTS);
#endif
    return MT_SUCCESS;
}

mt_void Cast_SendTimeStamps(DISP_CAST_S *pstCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame, /* MT_LD_Event_ID_E */ mt_u32 eEvent)
{
#if 1
    mt_ld_event_s evt;
    mt_u32 TmpTime = 0;

    mt_drv_sys_gettimestampms(&TmpTime);
    evt.evt_id = eEvent;
    evt.frame = pstCastFrame->u32FrameIndex;
    evt.handle = (MT_ID_DISP << 16) | pstCast->eDisp;
    evt.time = TmpTime;
    mt_drv_ld_notify_event(&evt);
#endif
    return;
}

static mt_s32 Cast_PushFrameToBackMod(DISP_CAST_S *pstCast)
{
#if 1
    mt_s32 nRet = MT_SUCCESS;
    MT_DRV_VIDEO_FRAME_S stCastFrame;
    FN_VENC_PUT_FRAME venc_queue_pfn = NULL;
    mt_u32 u32BufId = 0;
#if 1
    /*if this is null, means get frame though unf api, not attach mode.*/
    if (pstCast->attach_pairs[0].pfnQueueFrm)
    {
        while (1)
        {
            pstCast->u32CastAcquireTryCnt++;

            Cast_ERROR_DEAL(BP_GetFullBuf(&pstCast->stBP, &u32BufId), 0);
            Cast_ERROR_DEAL(BP_DelFullBuf(&pstCast->stBP, u32BufId), 1);
            Cast_ERROR_DEAL(BP_GetFrame(&pstCast->stBP, u32BufId, &stCastFrame), 1);

            venc_queue_pfn = pstCast->attach_pairs[0].pfnQueueFrm;

            nRet = venc_queue_pfn(pstCast->attach_pairs[0].hSink, &stCastFrame);
            if (nRet == MT_SUCCESS)
            {
                Cast_SendTimeStamps(pstCast, &stCastFrame, EVENT_CAST_FRM_OUT);
                pstCast->u32CastAcquireOkCnt++;
            }
            else
            {
                (mt_void) BP_SetBufReading(&pstCast->stBP, u32BufId);
                (mt_void) BP_AddEmptyBuf(&pstCast->stBP, u32BufId);
                DISP_ERROR("Cast ADD buf failed!\n");
                break;
            }
        }
    }
#else

    while (1)
    {
        pstCast->u32CastAcquireTryCnt++;

        Cast_ERROR_DEAL(BP_GetFullBuf(&pstCast->stBP, &u32BufId), 0);
        Cast_ERROR_DEAL(BP_DelFullBuf(&pstCast->stBP, u32BufId), 1);
        Cast_ERROR_DEAL(BP_GetFrame(&pstCast->stBP, u32BufId, &stCastFrame), 1);

        //printk("full id 0x%x\n",u32BufId);
        Cast_SendTimeStamps(pstCast, &stCastFrame, EVENT_CAST_FRM_OUT);
        pstCast->u32CastAcquireOkCnt++;

        DISP_CastReleaseFrame(pstCast, &stCastFrame);
    }
#endif

    return nRet;
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_CastAcquireFrame(mt_handle cast_ptr, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
#if 1
    DISP_CAST_S *pstCast;
    mt_s32 Ret = 0;

    pstCast = (DISP_CAST_S *)cast_ptr;

    if (pstCast->attach_pairs[0].pfnQueueFrm)
    {
        DISP_ERROR("attach mode is going on ,you can't acquire through unf api.");
        return MT_FAILURE;
    }

    Ret = Cast_AcquireFrame(pstCast, pstCastFrame);

    pstCast->u32CastAcquireTryCnt++;
    if (MT_SUCCESS == Ret)
    {
        Cast_SendTimeStamps(pstCast, pstCastFrame, EVENT_CAST_FRM_OUT);
        pstCast->u32CastAcquireOkCnt++;
    }

#if 0
    printk("cast acquire bufid=0x%x, w=%d,h=%d,index=%d, y=0x%x, c=0x%x\n",
    u32BufId,
    pstCastFrame->u32Width,
    pstCastFrame->u32Height,
    pstCastFrame->u32FrmCnt,
    pstCastFrame->stBufAddr[0].u32PhyAddr_Y,
    pstCastFrame->stBufAddr[0].u32PhyAddr_C
    );
#endif
    return Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 DispCastCheckFrame(mt_handle cast_ptr, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
#if 1
    DISP_CAST_PRIV_FRAME_S *pstPrivFrame;

    pstPrivFrame = (DISP_CAST_PRIV_FRAME_S *)&(pstCastFrame->u32Priv[0]);

    if (pstPrivFrame->cast_ptr == cast_ptr)
    {
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
#endif
}

mt_s32 DispCastGetBufId(mt_handle cast_ptr,
                        MT_DRV_VIDEO_FRAME_S *pstCastFrame,
                        mt_u32 *pu32BufId)
{
#if 1
    DISP_CAST_PRIV_FRAME_S *pstPrivFrame;

    pstPrivFrame = (DISP_CAST_PRIV_FRAME_S *)&(pstCastFrame->u32Priv[0]);

    if (pstPrivFrame->cast_ptr != cast_ptr)
    {
        DISP_PRINT("PRIV=[%x][%x]\n",
                   pstPrivFrame->cast_ptr,
                   pstPrivFrame->stPrivInfo.u32BufferID);

        return MT_FAILURE;
    }

    *pu32BufId = pstPrivFrame->stPrivInfo.u32BufferID;
#endif
    return MT_SUCCESS;
}

mt_s32 Cast_AddNode2ReallocateList(mt_u32 u32BufId,
                                   MT_BOOL bInternalRelease,
                                   MT_BOOL bNeedReAllocate)
{
#if 1
    mt_u32 i = 0;

    for (i = 0; i < DISP_CAST_BUFFER_MAX_NUMBER; i++)
    {
        /*search the empty node.*/
        if (!atomic_read(&g_stReleasePtrArray[i].atReleaseNodeStatus))
        {
            g_stReleasePtrArray[i].u32BufID = u32BufId;
            g_stReleasePtrArray[i].bInternalRelease = bInternalRelease;

            if (bNeedReAllocate)
            {
                //need to reallocate.
                atomic_set(&g_stReleasePtrArray[i].atReleaseNodeStatus, CAST_RETRIVE_NODE_REALLOCATE);
            }
            else
            {
                /*set the flag, just put it to nomal empty list, no
                             *necessary to reallocate.
                             */
                atomic_set(&g_stReleasePtrArray[i].atReleaseNodeStatus, CAST_RETRIVE_NODE_READY_TO_RETURN);
            }

            break;
        }
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_CastReleaseFrame(mt_handle cast_ptr, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
#if 1
    DISP_CAST_S *pstCast;
    mt_u32 u32BufId;
    mt_s32 nRet = MT_SUCCESS;
    MT_BOOL bNeedReAllocate = MT_FALSE;

    pstCast = (DISP_CAST_S *)cast_ptr;
    pstCast->u32CastReleaseTryCnt++;

    nRet = DispCastGetBufId(cast_ptr, pstCastFrame, &u32BufId);
    if (nRet)
    {
        DISP_WARN("Cast release frame invalid!\n");
        return nRet;
    }

    if (Cast_CheckOutputSizeChange(pstCast, pstCastFrame))
    {
        bNeedReAllocate = MT_TRUE;
    }
    else
    {
        bNeedReAllocate = MT_FALSE;
    }

    Cast_AddNode2ReallocateList(u32BufId, MT_FALSE, bNeedReAllocate);
#endif
    return MT_SUCCESS;
}

mt_s32 DispSetFrameDemoPartB(mt_handle cast_ptr,
                             mt_u32 u32Rate,
                             MT_DRV_COLOR_SPACE_E eColorSpace,
                             MT_DRV_VIDEO_FRAME_S *pstFrame)
{
#if 1
    DISP_CAST_S *pstCast;
    DISP_CAST_PRIV_FRAME_S *pstPriv;

    pstCast = (DISP_CAST_S *)cast_ptr;
    pstPriv = (DISP_CAST_PRIV_FRAME_S *)&(pstFrame->u32Priv[0]);

    //venc and vpss and others,they want a rate * 1000;
    pstFrame->u32FrameRate = u32Rate * 10;
    pstPriv->stPrivInfo.eColorSpace = eColorSpace;
#endif
    return MT_SUCCESS;
}

mt_void DISP_CastCBSetDispMode(mt_handle cast_ptr,
                               const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
#if 1
    DISP_CAST_S *pstCast;
    mt_u32 Rate;

    pstCast = (DISP_CAST_S *)cast_ptr;

    // set display info
    pstCast->stDispInfo = pstInfo->stDispInfo;

    pstCast->stAttr.stIn = pstInfo->stDispInfo.stPixelFmtResolution;
    pstCast->stAttr.bInterlace = pstInfo->stDispInfo.bInterlace;
    pstCast->stAttr.eInColorSpace = pstInfo->stDispInfo.eColorSpace;
    //Todo
    if (pstCast->stAttr.stOut.s32Height >= 720 && pstCast->stAttr.stOut.s32Width >= 1280)
    {
        pstCast->stAttr.eOutColorSpace = MT_DRV_CS_BT709_YUV_LIMITED;
    }
    else
    {
        pstCast->stAttr.eOutColorSpace = MT_DRV_CS_BT601_YUV_LIMITED;
    }

    Rate = pstInfo->stDispInfo.u32RefreshRate;
    pstCast->stAttr.u32InRate = Rate;

    pstCast->u32Periods = 1;
    while (Rate > DISP_CAST_MAX_FRAME_RATE)
    {
        pstCast->u32Periods = pstCast->u32Periods << 1;
        Rate = Rate >> 1;
    }

    pstCast->stAttr.u32OutRate = Rate;
    DispSetFrameDemoPartB(cast_ptr, Rate, pstCast->stAttr.eOutColorSpace, &pstCast->stAttr.stFrameDemo);

    DISP_PRINT("CAST: iw=%d, ih=%d, ow=%d, oh=%d, or=%d\n ",
               pstCast->stAttr.stIn.s32Width,
               pstCast->stAttr.stIn.s32Height,
               pstCast->stAttr.stOut.s32Width,
               pstCast->stAttr.stOut.s32Height,
               Rate);
#endif
    return;
}

mt_s32 DispCastSetFrameInfo(DISP_CAST_S *pstCast, MT_DRV_VIDEO_FRAME_S *pstCurFrame)
{
#if 1
    DISP_CAST_PRIV_FRAME_S *pstPrivFrame;
    MT_DRV_VIDEO_FRAME_S *pstFrame = MT_NULL;

    pstCast->stAttr.stFrameDemo.u32FrameIndex = pstCast->u32FrameCnt;
    pstCast->stAttr.stFrameDemo.stBufAddr[0] = pstCurFrame->stBufAddr[0];

    pstFrame = &pstCast->stAttr.stFrameDemo;
    pstFrame->u32Width = pstCurFrame->u32Width;
    pstFrame->u32Height = pstCurFrame->u32Height;
    pstFrame->stDispRect = pstCurFrame->stDispRect;

    pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
    pstFrame->hTunnelSrc = (MT_ID_DISP << 16) | pstCast->eDisp;

    if (pstCast->bLowDelay)
    {
        pstFrame->u32TunnelPhyAddr = pstCurFrame->u32TunnelPhyAddr;
    }
    else
    {
        pstFrame->u32TunnelPhyAddr = 0;
    }

    pstPrivFrame = (DISP_CAST_PRIV_FRAME_S *)&(pstCast->stAttr.stFrameDemo.u32Priv[0]);
    pstPrivFrame->stPrivInfo.u32FrmCnt = pstCast->u32FrameCnt;
    pstPrivFrame->stPrivInfo.u32BufferID = pstCast->u32LastCfgBufId;

    DISP_OS_GetTime(&pstCast->stAttr.stFrameDemo.u32Pts);
#endif
    return MT_SUCCESS;
}


#if defined(CONFIG_MT_CHIP_ARIA)
static u32 frm_cnt = 0;
#endif

#include "mt_drv_dma.h"
hal_dma_io_param_t g_dma = {0};
mt_s32 DR_PreScl_DMA(MT_DISP_CAST_OUTFRM_S *pstCstFrm, MT_DRV_VIDEO_FRAME_S * pstDstFrm)
{
    mt_s32            Ret = 0;
#if defined(CONFIG_MT_CHIP_ARIA)
    mt_u32 b_hd_field,u32CstW,u32CstH;

    b_hd_field = reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();

    u32CstW = reg_aria1_disp_get_gra_scale0_output_size_gra_scale0_width_out();
    u32CstH = reg_aria1_disp_get_gra_scale0_output_size_gra_scale0_height_out();
    if(MT_TRUE == b_hd_field)
    {
       u32CstH *= 2;
    }

    g_dma.param.config.dst_peripheral = 0xf;
    g_dma.param.config.src_peripheral = 0xf;
    g_dma.param.config.dst_endian = 0;
    g_dma.param.config.src_endian = 0;
    g_dma.param.config.dst_clk = 0;
    g_dma.param.config.src_clk = 0;
    g_dma.param.config.dst_i = DMA_ADDR_INC;
    g_dma.param.config.src_i = DMA_ADDR_INC;
    g_dma.param.config.dst_usize = DMA_USIZE_64BIT;
    g_dma.param.config.src_usize = DMA_USIZE_64BIT;
    g_dma.param.config.dst_bsize = DMA_BURST_NUM16;
    g_dma.param.config.src_bsize = DMA_BURST_NUM16;
    g_dma.param.control.int_link_en = 1;
    g_dma.param.control.int_node_en = 1;
    g_dma.param.control.chn_param_reg_en = 0;

#if 1
    //luma
    g_dma.param.len = (pstDstFrm->u32Width + 127)/128 * 128 * pstDstFrm->u32Height;
    g_dma.param.phy_src_addr  = pstCstFrm->stBufAddr[0].u32PhyAddr_Y;
    g_dma.param.phy_dst_addr  = pstDstFrm->stBufAddr[0].u32PhyAddr_Y;

    g_dma.chn_id = hal_dma_get_free_channel();
    Ret = hal_dma_start(g_dma.chn_id, (mt_void *)&g_dma.param, (mt_void *)g_dma.p_notify);
     //printk("src 0x%x,dst 0x%x  id %d Ret %d\n",g_dma.param.phy_src_addr,g_dma.param.phy_dst_addr,g_dma.chn_id,Ret);

    MT_INFO_VO("w %d, h %d g_dma.param.len %d,cstw %d, csth %d\n",pstDstFrm->u32Width,pstDstFrm->u32Height,g_dma.param.len,u32CstW,u32CstH);
    while (DMA_STATUS_STOP != hal_dma_check(g_dma.chn_id));
    hal_dma_stop(g_dma.chn_id);

#endif

#if 1
    //chroma
    g_dma.param.len = (pstDstFrm->u32Width + 127)/128 * 128 * pstDstFrm->u32Height/ 2;
    g_dma.param.phy_src_addr  = pstCstFrm->stBufAddr[0].u32PhyAddr_C;
    g_dma.param.phy_dst_addr  = pstDstFrm->stBufAddr[0].u32PhyAddr_C;

    g_dma.chn_id = hal_dma_get_free_channel();
    hal_dma_start(g_dma.chn_id, (mt_void *)&g_dma.param, (mt_void *)g_dma.p_notify);
    while (DMA_STATUS_STOP != hal_dma_check(g_dma.chn_id));
    hal_dma_stop(g_dma.chn_id);

#endif

    pstDstFrm->stBufAddr[0].u32Stride_Y = (pstDstFrm->u32Width + 127)/128 * 128;
    pstDstFrm->stBufAddr[0].u32Stride_C = (pstDstFrm->u32Width + 127)/128 * 128;

    pstCstFrm->stPreSclInfo.eOrderStatus = PS_ORDER_STATUS_DONE;
    pstCstFrm->u32RWStatus = CAST_OUTBUF_BE_READ;
#endif

    return Ret;
}

mt_s32 CastFrmToOutBuf(MT_DISP_CAST_OUTFRM_S *pstCstFrm)
{
#if defined(CONFIG_MT_CHIP_ARIA)
    mt_u32 u32CstW = 0;
    mt_u32 u32CstH = 0;
    MT_BOOL b_hd_field  = MT_FALSE;

    if(pstCstFrm->u32RWStatus != CAST_OUTBUF_EMPTY &&
      pstCstFrm->u32RWStatus != CAST_OUTBUF_BE_READ)
    {
        MT_ERR_VO("CastFrmToOutBuf status error \n");
        //return 1;
    }

    if(pstCstFrm->stPreSclInfo.eOrderStatus != PS_ORDER_STATUS_DONE && pstCstFrm->u32RWStatus == CAST_OUTBUF_BE_READ)
    {
        MT_ERR_VO("pre scale not done !!!!!!!\n");
    }

    if(0)//frm_cnt == 0x2
    {
        MT_INFO_VO("%d  Lum 0x%x, Chr 0x%x LR 0x%x, CR 0x%x\n",frm_cnt,pstCstFrm->stPreSclInfo.u32LumaAddrWrite,pstCstFrm->stPreSclInfo.u32ChromaAddrWrite,
        pstCstFrm->stPreSclInfo.u32LumaAddrRead,pstCstFrm->stPreSclInfo.u32ChromaAddrRead);
        while(1)
        {
            if(0xffffffff == reg_aria1_disp_get_pres_id())
                break;
        }
    }

    reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_0(0);
    reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_2(0);

    if(1)//frm_cnt <= 2
    {
        reg_aria1_disp_set_down_scale_data_endian_ctrl_wr_data_endian_ctrl(3);
        reg_aria1_disp_set_down_scale_data_endian_ctrl_cbcr_swap(1);

        b_hd_field = reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
        if(MT_TRUE == b_hd_field)
        {
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_0(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_2(1);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_0(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_2(0);
        }
        else
        {
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_0(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_2(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_0(1);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_2(1);
        }

        u32CstW = reg_aria1_disp_get_gra_scale0_output_size_gra_scale0_width_out();
        u32CstH = reg_aria1_disp_get_gra_scale0_output_size_gra_scale0_height_out();
        if(MT_TRUE == b_hd_field)
        {
           u32CstH *= 2;
        }
        reg_aria1_disp_set_down_scale_out_size_down_scale_output_height(u32CstH);
        reg_aria1_disp_set_down_scale_out_size_down_scale_output_width(u32CstW);

        reg_aria1_disp_set_down_scale_wr_stride_down_scale_wr_stride((u32CstW + 127) / 128 * 128);//
        //top field
        reg_aria1_disp_set_down_scale_luma_wr_addr_0_down_scale_luma_wr_addr_0(pstCstFrm->stBufAddr[0].u32PhyAddr_Y);
        reg_aria1_disp_set_down_scale_cbcr_wr_addr_0_down_scale_cbcr_wr_addr_0(pstCstFrm->stBufAddr[0].u32PhyAddr_C);
        //bottom field
        reg_aria1_disp_set_down_scale_luma_wr_addr_2_down_scale_luma_wr_addr_2(pstCstFrm->stBufAddr[0].u32PhyAddr_Y);
        reg_aria1_disp_set_down_scale_cbcr_wr_addr_2_down_scale_cbcr_wr_addr_2(pstCstFrm->stBufAddr[0].u32PhyAddr_C);

        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_0(1);
        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_2(1);

        pstCstFrm->stBufAddr[0].u32Stride_Y =reg_aria1_disp_get_down_scale_wr_stride_down_scale_wr_stride();
        pstCstFrm->stBufAddr[0].u32Stride_C = pstCstFrm->stBufAddr[0].u32Stride_Y;

    }

    pstCstFrm->u32RWStatus = CAST_OUTBUF_BE_WROTE;
#endif
    return 0;

}

mt_s32 PreSclLinearInput(MT_DISP_CAST_OUTFRM_S *pstCstFrm, MT_DRV_VIDEO_FRAME_S * pstPreSclFrm)
{
#if defined(CONFIG_MT_CHIP_ARIA)
    mt_s32 ret = 0;

    if(pstCstFrm->u32RWStatus == CAST_OUTBUF_BE_WROTE)
    {
        pstCstFrm->stPreSclInfo.eMode=FRAMER_FRAMEW_ONEFRAME;
        pstCstFrm->stPreSclInfo.eOrderPriority=PS_ORDER_PRI_HIGH;//PS_ORDER_PRI_LOW;
        pstCstFrm->stPreSclInfo.eOrderStatus=PS_ORDER_STATUS_NEW;
        pstCstFrm->stPreSclInfo.u32LumaAddrRead=pstCstFrm->stBufAddr[0].u32PhyAddr_Y;
        pstCstFrm->stPreSclInfo.u32LumaAddrRead2=pstCstFrm->stBufAddr[0].u32PhyAddr_Y;
        pstCstFrm->stPreSclInfo.u32ChromaAddrRead=pstCstFrm->stBufAddr[0].u32PhyAddr_C;
        pstCstFrm->stPreSclInfo.u32ChromaAddrRead2=pstCstFrm->stBufAddr[0].u32PhyAddr_C;
        pstCstFrm->stPreSclInfo.u32SrcWidth =reg_aria1_disp_get_down_scale_out_size_down_scale_output_width();
        pstCstFrm->stPreSclInfo.u32SrcHeight=reg_aria1_disp_get_down_scale_out_size_down_scale_output_height();

        pstCstFrm->stPreSclInfo.u32LumaAddrWrite=pstPreSclFrm->stBufAddr[0].u32PhyAddr_Y;
        pstCstFrm->stPreSclInfo.u32ChromaAddrWrite=pstPreSclFrm->stBufAddr[0].u32PhyAddr_C;
        pstCstFrm->stPreSclInfo.u32DstWidth= pstPreSclFrm->u32Width;
        pstCstFrm->stPreSclInfo.u32DstHeight=pstPreSclFrm->u32Height;
        pstCstFrm->stPreSclInfo.u32DstStride=(pstPreSclFrm->u32Width + 127) / 128 *128;

        pstCstFrm->stPreSclInfo.bIsTileMode = MT_FALSE;
        pstCstFrm->stPreSclInfo.u32LinearInputStride = pstCstFrm->stBufAddr[0].u32Stride_Y;

        pstCstFrm->stPreSclInfo.row_jump_value=0;
        pstCstFrm->stPreSclInfo.row_jump_offset=0;
        pstCstFrm->stPreSclInfo.u8TileCfg=0;
        pstCstFrm->stPreSclInfo.u8ColSize=0;
        pstCstFrm->stPreSclInfo.u8FieldPicture=0;


        pstPreSclFrm->stBufAddr[0].u32Stride_Y = pstCstFrm->stPreSclInfo.u32DstStride;
        pstPreSclFrm->stBufAddr[0].u32Stride_C = pstCstFrm->stPreSclInfo.u32DstStride;


        pstCstFrm->stPreSclInfo.u8InputEndian = 0xf;

        pstCstFrm->stPreSclInfo.u8OutputEndian = 0xf;

        pstCstFrm->stPreSclInfo.u8UVChange = 0x0;

        //make the order
        ret = PS_AddOrder(&pstCstFrm->stPreSclInfo);

 #if 0
        printk("LR=%08x LR2=%08x CR=%08x CR2=%08x SW=%d SH=%d LW=%08x CW=%08x DW=%08x DH=%d DS=%d InStride=%d AddOrder=%x Aret=%d\n",
          pstCstFrm->stPreSclInfo.u32LumaAddrRead,
          pstCstFrm->stPreSclInfo.u32LumaAddrRead2,
          pstCstFrm->stPreSclInfo.u32ChromaAddrRead,
          pstCstFrm->stPreSclInfo.u32ChromaAddrRead2,
          pstCstFrm->stPreSclInfo.u32SrcWidth,
          pstCstFrm->stPreSclInfo.u32SrcHeight,
          pstCstFrm->stPreSclInfo.u32LumaAddrWrite,
          pstCstFrm->stPreSclInfo.u32ChromaAddrWrite,
          pstCstFrm->stPreSclInfo.u32DstWidth,
          pstCstFrm->stPreSclInfo.u32DstHeight,
          pstCstFrm->stPreSclInfo.u32DstStride,
          pstCstFrm->stPreSclInfo.u32LinearInputStride,
          &pstCstFrm->stPreSclInfo,
          ret);
 #endif

        PS_Loop();

        pstCstFrm->u32RWStatus = CAST_OUTBUF_BE_READ;
    }
    else
    {
        ret = 1;
    }
#endif
    //printk("PreSclLinearInput ret %d\n",ret);
    return MT_SUCCESS;
}

mt_s32 DispCastSendTask(DISP_CAST_S *pstCast)
{
#if defined(CONFIG_MT_CHIP_ARIA)

#ifdef USE_PRE_LINEAR

    DISP_CAST_ATTR_S *pstAttr;
    pstAttr = &pstCast->stAttr;

    if(frm_cnt == 0)
    {
        CastFrmToOutBuf(&(pstCast->stCastOutFrm[0]));
        MT_INFO_VO("castfrm to outbuf[0]\n");
    }
    else
    {
        PreSclLinearInput( &(pstCast->stCastOutFrm[(frm_cnt -1) % 2]),&(pstAttr->stFrameDemo));
        //DR_PreScl_DMA( &(pstCast->stCastOutFrm[(frm_cnt -1) % 2]),&(pstAttr->stFrameDemo));

      CastFrmToOutBuf(&(pstCast->stCastOutFrm[frm_cnt % 2]));

       //printk("prescale outbuf[%d]  ,cast newfrm to outbuf[%d]\n",(frm_cnt -1) % 2,frm_cnt % 2);
    }

    frm_cnt ++;
#else

    DISP_CAST_ATTR_S *pstAttr;

    pstAttr = &pstCast->stAttr;

    /*
    printk("yaddr 0x%x,stride %d, cbcr 0x%x stride %d w %d, h %d\n",pstAttr->stFrameDemo.stBufAddr[0].u32PhyAddr_Y,
                          pstAttr->stFrameDemo.stBufAddr[0].u32Stride_Y,
                          pstAttr->stFrameDemo.stBufAddr[0].u32PhyAddr_C,
                          pstAttr->stFrameDemo.stBufAddr[0].u32Stride_C,
                          pstAttr->stFrameDemo.u32Width,
                          pstAttr->stFrameDemo.u32Height);
    */

    /*  if (frm_cnt == 0)*/
    {
        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_0(0);
        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_2(0);

        reg_aria1_disp_set_down_scale_data_endian_ctrl_wr_data_endian_ctrl(3);
        reg_aria1_disp_set_down_scale_data_endian_ctrl_cbcr_swap(1);

        reg_aria1_disp_set_down_scale_out_size_down_scale_output_height(pstAttr->stFrameDemo.u32Height);
        reg_aria1_disp_set_down_scale_out_size_down_scale_output_width(pstAttr->stFrameDemo.u32Width);

        pstAttr->stFrameDemo.stBufAddr[0].u32Stride_Y = (pstAttr->stFrameDemo.u32Width + 127)/128 * 128;

        reg_aria1_disp_set_down_scale_wr_stride_down_scale_wr_stride(pstAttr->stFrameDemo.stBufAddr[0].u32Stride_Y);
        //top field
        reg_aria1_disp_set_down_scale_luma_wr_addr_0_down_scale_luma_wr_addr_0(pstAttr->stFrameDemo.stBufAddr[0].u32PhyAddr_Y);
        reg_aria1_disp_set_down_scale_cbcr_wr_addr_0_down_scale_cbcr_wr_addr_0(pstAttr->stFrameDemo.stBufAddr[0].u32PhyAddr_C);
        //bottom field
        reg_aria1_disp_set_down_scale_luma_wr_addr_2_down_scale_luma_wr_addr_2(pstAttr->stFrameDemo.stBufAddr[0].u32PhyAddr_Y);
        reg_aria1_disp_set_down_scale_cbcr_wr_addr_2_down_scale_cbcr_wr_addr_2(pstAttr->stFrameDemo.stBufAddr[0].u32PhyAddr_C);

        if(MT_TRUE == reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode())
        {
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_0(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_2(1);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_0(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_2(0);
        }
        else
        {
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_0(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_top_field_flag_2(0);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_0(1);
            reg_aria1_disp_set_down_scale_ctrl_wr_progressive_flag_2(1);
        }

        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_0(1);
        reg_aria1_disp_set_down_scale_ctrl_down_scale_wr_en_2(1);
    }

    frm_cnt++;

#endif
#endif
    return MT_SUCCESS;
}

mt_void DISP_CastPushFrame(mt_handle cast_ptr, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
#if 1
    DISP_CAST_S *pstCast;
    mt_u32 u32BufId, u32BufState = 0;
    mt_s32 nRet;
    mt_u32 u32FrameWbcStage = 0;

    pstCast = (DISP_CAST_S *)cast_ptr;

    if ((pstInfo->eEventType == MT_DRV_DISP_C_PREPARE_CLOSE) || (pstInfo->eEventType == MT_DRV_DISP_C_PREPARE_TO_PEND))
    {
        pstCast->bMasked = MT_TRUE;
    }
    else
    {
        pstCast->bMasked = MT_FALSE;
    }

    if (!pstCast->bEnable || pstCast->bMasked)
    {
        return;
    }

    if (atomic_read(&pstCast->bBufBusy))
    {
        return;
    }
    else
    {
        atomic_set(&pstCast->bBufBusy, MT_TRUE);
    }

    {
        pstCast->u32CastIntrCnt++;
        while (1)
        {
            u32BufState = pstCast->bLowDelay ? CAST_BUFFER_STATE_IN_CFGLIST_WRITING : CAST_BUFFER_STATE_IN_CFGLIST_WRITE_FINISH;

            nRet = BP_GetCfgWritingBuf(&pstCast->stBP,
                                       &u32BufId,
                                       u32BufState,
                                       &u32FrameWbcStage);
            if (nRet)
            {
                DISP_WARN("Cast Get cfg  buf failed!\n");
                break;
            }

            (mt_void) BP_DelCfgWritingBuf(&pstCast->stBP, u32BufId);
            nRet = BP_AddFullBuf(&pstCast->stBP, u32BufId);
            if (nRet)
            {
                (mt_void) BP_SetBufReading(&pstCast->stBP, u32BufId);
                (mt_void) BP_AddEmptyBuf(&pstCast->stBP, u32BufId);
                DISP_ERROR("Cast ADD buf failed!\n");
                break;
            }
        }

        Cast_PushFrameToBackMod(pstCast);
    }

    atomic_set(&pstCast->bBufBusy, MT_FALSE);
#endif
    return;
}

mt_void DISP_CastCB_GenarateFrame(mt_handle cast_ptr, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
#if 1
    MT_DRV_VIDEO_FRAME_S stCurFrame;
    DISP_CAST_S *pstCast;
    mt_s32 nRet;
    DISP_CAST_PRIV_FRAME_S *pstPrivFrame = NULL;

    pstCast = (DISP_CAST_S *)cast_ptr;

    if ((pstInfo->eEventType == MT_DRV_DISP_C_PREPARE_CLOSE) || (pstInfo->eEventType == MT_DRV_DISP_C_PREPARE_TO_PEND))
    {
        pstCast->bMasked = MT_TRUE;
        pstCast->bToGetDispInfo = MT_TRUE;
    }
    else
    {
        pstCast->bMasked = MT_FALSE;
    }

    if (pstInfo->eEventType == MT_DRV_DISP_C_OPEN)
    {
        pstCast->bToGetDispInfo = MT_TRUE;
    }

    if (pstCast->bToGetDispInfo)
    {
        DISP_CastCBSetDispMode(cast_ptr, pstInfo);

        pstCast->bToGetDispInfo = MT_FALSE;
    }

    if (pstCast->bScheduleWbc)
    {
#if 0 //hardware
        pstCast->stIntfOpt.PF_SetWbcEnable(pstCast->eWBC, MT_FALSE);
        pstCast->stIntfOpt.PF_UpdateWbc(pstCast->eWBC);
#endif
        pstCast->bScheduleWbcStatus = MT_TRUE;
        return;
    }

    // check state
    if (!pstCast->bEnable || pstCast->bMasked)
    {
#if 0 //hardware
        pstCast->stIntfOpt.PF_SetWbcEnable(pstCast->eWBC, MT_FALSE);
        pstCast->stIntfOpt.PF_UpdateWbc(pstCast->eWBC);
#endif
        return;
    }

    /*when isr and process conflict, try to lock.*/
    if (atomic_read(&pstCast->bBufBusy))
    {
        return;
    }
    else
    {
        atomic_set(&pstCast->bBufBusy, MT_TRUE);
    }

    if (pstInfo->eEventType == MT_DRV_DISP_C_VT_INT)
    {
        mt_u32 u32BufId;

/*stop wbc first, because in 50hz condition. every 2 times, one wbc occurs.*/
#if 0 //hardware
        pstCast->stIntfOpt.PF_SetWbcEnable(pstCast->eWBC, MT_FALSE);
        pstCast->stIntfOpt.PF_UpdateWbc(pstCast->eWBC);
#endif
        /*retrieve all the frame release by venc or reallocated frame.*/
        Cast_ReturnFrameToList(pstCast);

        /*all the frame node's stage ++ */
        BP_IncreaseAllCfgWritingState(&pstCast->stBP, MT_NULL);

        /*get the frame writing now, and send time stamp for stastics.*/
        nRet = BP_GetCfgWritingBuf_JustWriting(&pstCast->stBP, &u32BufId, CAST_BUFFER_STATE_IN_CFGLIST_WRITING);
        if ((!nRet) && (!BP_GetFrame(&pstCast->stBP, u32BufId, &stCurFrame)))
        {
            Cast_SendTimeStamps(pstCast, &stCurFrame, EVENT_CAST_FRM_BEGIN);
        }

        /*since we may get 25pfs in 50hz fmt, so a interval is necessary.*/
        pstCast->u32TaskCount++;
        if ((pstCast->u32TaskCount % pstCast->u32Periods) != 0) {
            atomic_set(&pstCast->bBufBusy, MT_FALSE);
            return;
        }

        nRet = Cast_GetFrame(pstCast, &u32BufId, 0, &stCurFrame); //get empty buf
        if (nRet)
        {
            DISP_WARN("Cast get empty id failed!\n");
            atomic_set(&pstCast->bBufBusy, MT_FALSE);
            return;
        }
        else
        {
            //printk("disp cast new frame to bufid 0x%x\n", u32BufId);
        }

        while (Cast_CheckOutputSizeChange(pstCast, &stCurFrame))
        {
            Cast_AddNode2ReallocateList(u32BufId, MT_TRUE, MT_TRUE);
            nRet = Cast_GetFrame(pstCast, &u32BufId, 0, &stCurFrame);
            if (nRet)
            {
                atomic_set(&pstCast->bBufBusy, MT_FALSE);
                DISP_WARN("Cast get empty id failed!\n");
                return;
            }
        }

        nRet = BP_AddCfgWritingBuf(&pstCast->stBP, u32BufId);
        if (nRet) {
            atomic_set(&pstCast->bBufBusy, MT_FALSE);
            DISP_WARN("Cast add to cfg list  failed!\n");
            return;
        }

        pstCast->u32LastCfgBufId = u32BufId;
        pstCast->u32FrameCnt++;

        DispCastSetFrameInfo(pstCast, &stCurFrame); //stCurFrame video buf addr give stFrameDemo

        DispCastSetFramePTS(&pstCast->stAttr.stFrameDemo, MIRA_SET_CREATE_PTS);

        pstPrivFrame = (DISP_CAST_PRIV_FRAME_S *)&(pstCast->stAttr.stFrameDemo.u32Priv[0]);
        pstPrivFrame->u32Pts0 += 100000 / 25; //pstCast->stDispInfo.u32RefreshRate;

        pstCast->stAttr.stFrameDemo.u32Pts = pstPrivFrame->u32Pts0;
        pstCast->stAttr.stFrameDemo.u32SrcPts = pstPrivFrame->u32Pts0;
        //setting hardware attr by stFrameDemo
        DispCastSendTask(pstCast);
        //stFrameDemo been pushed in queue
        (mt_void) BP_SetFrame(&pstCast->stBP, pstCast->u32LastCfgBufId, &pstCast->stAttr.stFrameDemo);
    }

    atomic_set(&pstCast->bBufBusy, MT_FALSE);
#endif
    return;
}

mt_s32 DISP_Cast_AttachSink(mt_handle cast_ptr, mt_handle hSink)
{
#if 1
    mt_mod_id_e enModID;
    DISP_CAST_S *pstCast;
    mt_s32 s32Ret;
    VENC_EXPORT_FUNC_S *pstVenFunc = MT_NULL;
    mt_u32 attach_index = 0, u32Cnt = 0;

    pstCast = (DISP_CAST_S *)cast_ptr;
    enModID = (mt_mod_id_e)((hSink & 0xff0000) >> 16);

    while (atomic_read(&pstCast->bBufBusy))
    {
        msleep(10);
        u32Cnt++;
        if (u32Cnt > 200) {
            DISP_ERROR("DISP_Cast_AttachSink timeout.\n");
            return MT_ERR_DISP_TIMEOUT;
        }
    }
    atomic_set(&pstCast->bBufBusy, MT_TRUE);

    if (MT_ID_VENC == enModID)
    {

        s32Ret = mt_drv_module_getfunction(enModID, (mt_void **)&(pstVenFunc));
        if (MT_SUCCESS != s32Ret)
        {
            DISP_ERROR("Get null venc ptr when cast.\n");
            atomic_set(&pstCast->bBufBusy, MT_FALSE);
            return MT_ERR_DISP_NULL_PTR;
        }

        for (attach_index = 0; attach_index < DISPLAY_ATTACH_CNT_MAX; attach_index++)
        {
            if (pstCast->attach_pairs[attach_index].hSink == hSink)
            {
                pstCast->bAttached = MT_TRUE;
                atomic_set(&pstCast->bBufBusy, MT_FALSE);
                return MT_SUCCESS;
            }
        }

        for (attach_index = 0; attach_index < DISPLAY_ATTACH_CNT_MAX; attach_index++)
        {
            if (pstCast->attach_pairs[attach_index].hSink == 0)
            {
                pstCast->attach_pairs[attach_index].hSink = hSink;
                pstCast->attach_pairs[attach_index].pfnQueueFrm = pstVenFunc->pfnVencQueueFrame;
                pstCast->attach_pairs[attach_index].pfnDequeueFrame = MT_NULL;
                break;
            }
        }

        if (attach_index == DISPLAY_ATTACH_CNT_MAX)
        {
            s32Ret = MT_FAILURE;
        }
        else
        {
            s32Ret = MT_SUCCESS;
            pstCast->bAttached = MT_TRUE;
        }
    }
    else
    {
        s32Ret = MT_FAILURE;
    }

    /*we support dynamic attach and dettach without disabling cast(as a producer),
      when attach again,  we should reset all the buffer, or else
      there is no empty buffer node, because cast enabled and full buf full.*/
    if (s32Ret == MT_SUCCESS)
    {
        /*msleep 60 ms, for when reset we should wait until wbc stop to write back.*/
        msleep(60);

        BP_Reset(&pstCast->stBP);
        /*because in the first, venc and cast output resolutin
               *may not consistent.
               */
        (mt_void) BP_ReAllocAllBuf(&pstCast->stBP);
        memset((void *)g_stReleasePtrArray,
               0,
               sizeof(CAST_RELEASE_PTR_S) * DISP_CAST_BUFFER_MAX_NUMBER);
    }

    atomic_set(&pstCast->bBufBusy, MT_FALSE);
    return s32Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_Cast_DeAttachSink(mt_handle cast_ptr, mt_handle hSink)
{
#if 1
    mt_mod_id_e enModID;
    DISP_CAST_S *pstCast;
    mt_s32 s32Ret;
    mt_u32 attach_index = 0, u32Cnt = 0;

    pstCast = (DISP_CAST_S *)cast_ptr;
    enModID = (mt_mod_id_e)((hSink & 0xff0000) >> 16);

    while (atomic_read(&pstCast->bBufBusy))
    {
        msleep(10);
        u32Cnt++;
        if (u32Cnt > 200)
        {
            DISP_ERROR("DISP_Cast_AttachSink timeout.\n");
            return MT_ERR_DISP_TIMEOUT;
        }
    }
    atomic_set(&pstCast->bBufBusy, MT_TRUE);

    if (MT_ID_VENC == enModID)
    {

        for (attach_index = 0; attach_index < DISPLAY_ATTACH_CNT_MAX; attach_index++)
        {
            if (pstCast->attach_pairs[attach_index].hSink == hSink)
            {
                pstCast->attach_pairs[attach_index].hSink = 0;
                pstCast->attach_pairs[attach_index].pfnQueueFrm = MT_NULL;
                pstCast->attach_pairs[attach_index].pfnDequeueFrame = MT_NULL;
                break;
            }
        }

        if (attach_index == DISPLAY_ATTACH_CNT_MAX)
        {
            s32Ret = MT_FAILURE;
        }
        else
        {
            s32Ret = MT_SUCCESS;
            pstCast->bAttached = MT_FALSE;
        }

    }
    else
    {
        s32Ret = MT_FAILURE;
    }

    /*as a result of asynchrounous bettween Cast and venc,
     *we should wait ,because if venc detach and return, cast may keep
     * writing  to venc.
     */
    msleep(40);
    atomic_set(&pstCast->bBufBusy, MT_FALSE);

    return s32Ret;
#endif
    return MT_SUCCESS;
}

static mt_void Cast_UpdateAttr(DISP_CAST_S *pstCast)
{
#if 1
    pstCast->stBP.stAlloc.u32BufWidth = pstCast->stAttr.stOut.s32Width;
    pstCast->stBP.stAlloc.u32BufHeight = pstCast->stAttr.stOut.s32Height;
#endif

    return;
}

mt_s32 DISP_Cast_SetAttr(mt_handle cast_ptr, MT_DRV_DISP_Cast_Attr_S *castAttr)
{
#if 1
    DISP_CAST_S *pstCast = MT_NULL;

    pstCast = (DISP_CAST_S *)cast_ptr;

    pstCast->stAttr.stOut.s32Width = castAttr->s32Width;
    pstCast->stAttr.stOut.s32Height = castAttr->s32Height;

    Cast_UpdateAttr(pstCast);
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_Cast_GetAttr(mt_handle cast_ptr, MT_DRV_DISP_Cast_Attr_S *castAttr)
{
#if 1
    DISP_CAST_S *pstCast = MT_NULL;

    pstCast = (DISP_CAST_S *)cast_ptr;

    castAttr->s32Width = pstCast->stAttr.stOut.s32Width;
    castAttr->s32Height = pstCast->stAttr.stOut.s32Height;
#endif
    return MT_SUCCESS;
}

#define SNAPSHOT_MAGIC 0x534e4150 /* ASCII code of "SNAP" */
//static mt_u32 s_u32FrameCnt = 1;

mt_s32 DISP_Acquire_Snapshot(MT_DRV_DISPLAY_E enDisp, mt_handle *snapshotHandle, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
#if 0
    BUF_ALLOC_S stAlloc;
    mt_s32 ret;
    MT_DRV_VIDEO_FRAME_S* pstVideoFrame = pstFrame;
    DISP_INTF_OPERATION_S stFunc;
    DISP_WBC_E eWBC;
    MT_DISP_DISPLAY_INFO_S stInfo;
    mt_u32 u32BufId = 0;
    DISP_SNAPSHOT_PRIV_FRAME_S *pstPriv;
    mt_u32 u32Pts;
    DISP_SNAPSHOT_S *pstSnapshot = MT_NULL;
    MT_BOOL  bBufAlloc = 0;

    *snapshotHandle  = 0;

    pstSnapshot = (DISP_SNAPSHOT_S*)DISP_MALLOC(sizeof(DISP_SNAPSHOT_S));
    if (!pstSnapshot)
        return MT_FAILURE;

    DISP_MEMSET(pstSnapshot, 0, sizeof(DISP_SNAPSHOT_S));
    ret = DISP_HAL_GetOperation(&stFunc);
    if (ret)
    {
        goto __ERR_EXIT__;
    }

    ret = stFunc.PF_AcquireWbcByChn(enDisp, &eWBC);
    if (ret)
    {
        if (eWBC >= DISP_WBC_BUTT)
            goto __ERR_EXIT__;
    }

    (mt_void)DISP_GetDisplayInfo( enDisp, &stInfo);
    stAlloc.bFbAllocMem = MT_TRUE;
    stAlloc.eDataFormat = MT_DRV_PIX_FMT_NV21;
    stAlloc.u32BufHeight = stInfo.stFmtResolution.s32Height;
    stAlloc.u32BufWidth = stInfo.stFmtResolution.s32Width;
    stAlloc.u32BufStride = 0;
    ret = BP_Create(1, &stAlloc, &pstSnapshot->stBP);
    if (ret)
    {
        goto __ERR_EXIT__;
    }

    bBufAlloc = 1;
    ret = BP_GetEmptyBuf(&pstSnapshot->stBP, &u32BufId);
    if (ret)
    {
        goto __ERR_EXIT__;
    }

    ret = BP_DelEmptyBuf(&pstSnapshot->stBP, u32BufId);
    if (ret)
    {
        goto __ERR_EXIT__;
    }

    ret = BP_GetFrame(&pstSnapshot->stBP, u32BufId, pstVideoFrame);
    if (ret)
    {
        goto __ERR_EXIT__;
    }

    pstVideoFrame->u32Width  = stInfo.stFmtResolution.s32Width;
    pstVideoFrame->u32Height = stInfo.stFmtResolution.s32Height;
    pstVideoFrame->u32AspectWidth = 16;
    pstVideoFrame->u32AspectHeight = 9;
    pstVideoFrame->u32FrameRate = 0;
    pstVideoFrame->ePixFormat = MT_DRV_PIX_FMT_NV21;
    pstVideoFrame->bProgressive = MT_TRUE;
    pstVideoFrame->enFieldMode = MT_DRV_FIELD_ALL;
    pstVideoFrame->bTopFieldFirst = 0;
    pstVideoFrame->stDispRect = stInfo.stFmtResolution;
    pstVideoFrame->eFrmType = MT_DRV_FT_NOT_STEREO;
    pstVideoFrame->u32FrameIndex = 0;
    memset(pstVideoFrame->u32Priv, 0, sizeof(pstVideoFrame->u32Priv));
    // config pixformat
    stFunc.PF_SetWbcPixFmt(eWBC, MT_DRV_PIX_FMT_NV21);

    /*FIXME:  ourrect  why be stInfo.stFmtResolution? error?*/
    stFunc.PF_SetWbcIORect(eWBC, &stInfo, &stInfo.stPixelFmtResolution, &stInfo.stFmtResolution);

    stFunc.PF_SetWbc3DInfo(eWBC, &stInfo, &stInfo.stPixelFmtResolution);

    // config csc
    stFunc.PF_SetWbcColorSpace(eWBC, stInfo.eColorSpace, stInfo.eColorSpace);

    // config addr
    stFunc.PF_SetWbcAddr(eWBC, &(pstVideoFrame->stBufAddr[0]));

    // set enable FIXME!!!
    stFunc.PF_SetWbcEnable(eWBC, MT_TRUE);
    stFunc.PF_UpdateWbc(eWBC);

    /*we should wait util wbc finished ,else acquire api may get useless data.*/
    if (!stInfo.u32RefreshRate)
    {
        msleep(60);
    }
    else
    {
        msleep((100000/stInfo.u32RefreshRate + 1) * 2);
    }

    /* set pts. */
    DISP_OS_GetTime(&u32Pts);
    pstFrame->u32Pts    = u32Pts;
    pstFrame->u32SrcPts = u32Pts;

    /* construct some private info. */
    pstPriv = (DISP_SNAPSHOT_PRIV_FRAME_S*)&(pstFrame->u32Priv[0]);
    pstPriv->stPrivInfo.u32FrmCnt = s_u32FrameCnt ++;
    pstPriv->u32BPAddr = (mt_u32)&(pstSnapshot->stBP);
    pstPriv->u32Magic = SNAPSHOT_MAGIC;

    pstSnapshot->bWork = 1;

    *snapshotHandle  = (mt_handle)pstSnapshot;
    return MT_SUCCESS;

__ERR_EXIT__:
    if (bBufAlloc)
        BP_Destroy(&pstSnapshot->stBP);

    DISP_FREE(pstSnapshot);
    return MT_FAILURE;
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_Release_Snapshot(MT_DRV_DISPLAY_E enDisp, mt_handle snapshotHandle, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
#if 0
    DISP_SNAPSHOT_PRIV_FRAME_S *pstPriv;
    DISP_SNAPSHOT_S *pstSnapshot = MT_NULL;

    pstPriv = (DISP_SNAPSHOT_PRIV_FRAME_S *)&(pstFrame->u32Priv[0]);
    if (pstPriv->u32Magic != SNAPSHOT_MAGIC)
        return MT_ERR_DISP_INVALID_PARA;

    if (!snapshotHandle)
        return MT_ERR_DISP_NULL_PTR;

    pstSnapshot = (DISP_SNAPSHOT_S *)snapshotHandle;

     if (pstSnapshot->bWork == 0)
        return MT_SUCCESS;

    pstSnapshot->bWork = 0;
    BP_Destroy(&pstSnapshot->stBP);
    DISP_FREE(pstSnapshot);
#endif
    return 0;
}

mt_s32 DISP_SnapshotDestroy(mt_handle snapshot_ptr)
{
#if 0
    DISP_SNAPSHOT_S *pstSnapshot = MT_NULL;



    pstSnapshot = (DISP_SNAPSHOT_S *)snapshot_ptr;
    if (!pstSnapshot)
        return MT_ERR_DISP_NULL_PTR;

    if (pstSnapshot->bWork == 0)
        return MT_SUCCESS;

    /*when we destroy, wbc may be going on,so we wait.*/
    msleep(40);

    pstSnapshot->bWork = 0;
    BP_Destroy(&pstSnapshot->stBP);
    DISP_FREE(pstSnapshot);
#endif
    return MT_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
