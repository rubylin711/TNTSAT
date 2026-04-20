/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_vo.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "mt_mpi_win.h"
#include "mt_mpi_vi.h"
#include "mt_error_mpi.h"
#include "mpi_disp_tran.h"
#include "mt_mpi_avplay.h"
#include "mt_mpi_disp.h"


mt_s32 VO_ConvertWinAttrToMPI(MT_UNF_WINDOW_ATTR_S *pUnfAttr, MT_DRV_WIN_ATTR_S *pstMpiAttr);
mt_s32 VO_ConvertWinAttrToUNF(MT_DRV_WIN_ATTR_S *pstMpiAttr, MT_UNF_WINDOW_ATTR_S *pUnfAttr);
mt_s32 MT_UNF_VO_MAPFrame(mt_handle hWindow, phys_addr_t paddr);

mt_s32 MT_UNF_VO_Init(MT_UNF_VO_DEV_MODE_E enDevMode)
{
    if (MT_UNF_VO_DEV_MODE_BUTT <= enDevMode)
    {
        MT_FATAL_WIN("Invalid mode!\n");
        return MT_FAILURE;
    }

    return MT_MPI_WIN_Init();
}

mt_s32 MT_UNF_VO_DeInit(mt_void)
{
    return MT_MPI_WIN_DeInit();
}

mt_s32 VO_ConvertWinAttrToMPI(MT_UNF_WINDOW_ATTR_S *pUnfAttr, MT_DRV_WIN_ATTR_S *pstMpiAttr)
{
    if (!pstMpiAttr)
    {
        MT_ERR_WIN("para pstMpiAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    if (!pUnfAttr)
    {
        MT_ERR_WIN("para pUnfAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    /* conver unf parameter to driver parameters */
    memset(pstMpiAttr, 0, sizeof(MT_DRV_WIN_ATTR_S));

    Transfer_DispID(&pUnfAttr->enDisp, &pstMpiAttr->enDisp, MT_TRUE);

    pstMpiAttr->bUseSubLayer = pUnfAttr->bUseSubLayer;
    pstMpiAttr->bSetVideoBot = pUnfAttr->bSetVideoBot;
    pstMpiAttr->bVirtual = pUnfAttr->bVirtual;

    if (MT_UNF_VO_ASPECT_CVRS_BUTT <= pUnfAttr->stWinAspectAttr.enAspectCvrs)
    {
        MT_ERR_WIN("para enAspectCvrs is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (pUnfAttr->stWinAspectAttr.bUserDefAspectRatio)
    {
        pstMpiAttr->stCustmAR.u32ARw = pUnfAttr->stWinAspectAttr.u32UserAspectWidth;
        pstMpiAttr->stCustmAR.u32ARh = pUnfAttr->stWinAspectAttr.u32UserAspectHeight;
    }
    else
    {
        pstMpiAttr->stCustmAR.u32ARw = 0;
        pstMpiAttr->stCustmAR.u32ARh = 0;
    }
    Transfe_ARConvert(&pUnfAttr->stWinAspectAttr.enAspectCvrs, &pstMpiAttr->enARCvrs, MT_TRUE);

    pstMpiAttr->bUseCropRect = pUnfAttr->bUseCropRect;
    pstMpiAttr->stCropRect.u32LeftOffset = pUnfAttr->stCropRect.u32LeftOffset;
    pstMpiAttr->stCropRect.u32TopOffset = pUnfAttr->stCropRect.u32TopOffset;
    pstMpiAttr->stCropRect.u32RightOffset = pUnfAttr->stCropRect.u32RightOffset;
    pstMpiAttr->stCropRect.u32BottomOffset = pUnfAttr->stCropRect.u32BottomOffset;

    pstMpiAttr->stInRect = pUnfAttr->stInputRect;
    pstMpiAttr->stOutRect = pUnfAttr->stOutputRect;

    if (pUnfAttr->bVirtual)
    {
        Transfer_VideoFormat(&pUnfAttr->enVideoFormat, &pstMpiAttr->enDataFormat, MT_TRUE);
        if (pstMpiAttr->enDataFormat != MT_DRV_PIX_FMT_NV21 && pstMpiAttr->enDataFormat != MT_DRV_PIX_FMT_NV61_2X1)
        {
            MT_ERR_WIN("para enVideoFormat can't be supported.\n");
            return MT_ERR_VO_INVALID_PARA;
        }
    }

    return MT_SUCCESS;
}

mt_s32 VO_ConvertWinAttrToUNF(MT_DRV_WIN_ATTR_S *pstMpiAttr, MT_UNF_WINDOW_ATTR_S *pUnfAttr)
{
    if (!pstMpiAttr)
    {
        MT_ERR_WIN("para pstMpiAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    if (!pUnfAttr)
    {
        MT_ERR_WIN("para pUnfAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    /* conver unf parameter to driver parameters */
    memset(pUnfAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
    Transfer_DispID(&pUnfAttr->enDisp, &pstMpiAttr->enDisp, MT_FALSE);

    pUnfAttr->bUseSubLayer = pstMpiAttr->bUseSubLayer;
    pUnfAttr->bSetVideoBot = pstMpiAttr->bSetVideoBot;
    pUnfAttr->bVirtual = pstMpiAttr->bVirtual;
#if 0
    if (pstMpiAttr->stCustmAR.u8ARw && pstMpiAttr->stCustmAR.u8ARh)
    {
        pUnfAttr->stWinAspectAttr.bUserDefAspectRatio = MT_TRUE;
    }

    pUnfAttr->stWinAspectAttr.u32UserAspectWidth  = pstMpiAttr->stCustmAR.u8ARw;
    pUnfAttr->stWinAspectAttr.u32UserAspectHeight = pstMpiAttr->stCustmAR.u8ARh;

    Transfe_ARConvert(&pUnfAttr->stWinAspectAttr.enAspectCvrs, &pstMpiAttr->enARCvrs, MT_FALSE);
    pUnfAttr->stWinAspectAttr.enAspectCvrs = (MT_UNF_VO_ASPECT_CVRS_E)pstMpiAttr->enARCvrs;
#else

    if (pstMpiAttr->stCustmAR.u32ARw != 0 && pstMpiAttr->stCustmAR.u32ARh != 0)
    {
        pUnfAttr->stWinAspectAttr.bUserDefAspectRatio = MT_TRUE;
        pUnfAttr->stWinAspectAttr.u32UserAspectWidth = pstMpiAttr->stCustmAR.u32ARw;
        pUnfAttr->stWinAspectAttr.u32UserAspectHeight = pstMpiAttr->stCustmAR.u32ARh;
    }
    else
    {
        pUnfAttr->stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
        pUnfAttr->stWinAspectAttr.u32UserAspectWidth = pstMpiAttr->stCustmAR.u32ARw;
        pUnfAttr->stWinAspectAttr.u32UserAspectHeight = pstMpiAttr->stCustmAR.u32ARh;
    }
    Transfe_ARConvert(&pUnfAttr->stWinAspectAttr.enAspectCvrs, &pstMpiAttr->enARCvrs, MT_FALSE);

#endif

    pUnfAttr->bUseCropRect = pstMpiAttr->bUseCropRect;
    pUnfAttr->stCropRect.u32LeftOffset = pstMpiAttr->stCropRect.u32LeftOffset;
    pUnfAttr->stCropRect.u32TopOffset = pstMpiAttr->stCropRect.u32TopOffset;
    pUnfAttr->stCropRect.u32RightOffset = pstMpiAttr->stCropRect.u32RightOffset;
    pUnfAttr->stCropRect.u32BottomOffset = pstMpiAttr->stCropRect.u32BottomOffset;

    pUnfAttr->stInputRect = pstMpiAttr->stInRect;

    pUnfAttr->stOutputRect = pstMpiAttr->stOutRect;

    if (pstMpiAttr->bVirtual)
    {
        Transfer_VideoFormat(&pUnfAttr->enVideoFormat, &pstMpiAttr->enDataFormat, MT_FALSE);
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_VO_CreateWindow(const MT_UNF_WINDOW_ATTR_S *pWinAttr, mt_handle *phWindow)
{
    MT_DRV_WIN_ATTR_S stMpiAttr;
    mt_s32 s32Ret;

    if (!pWinAttr)
    {
        MT_ERR_WIN("para pWinAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    if (!phWindow)
    {
        MT_ERR_WIN("para phWindow is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    s32Ret = VO_ConvertWinAttrToMPI((MT_UNF_WINDOW_ATTR_S *)pWinAttr, &stMpiAttr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_WIN("para pWinAttr is err.\n");
        return s32Ret;
    }

    s32Ret = MT_MPI_WIN_Create(&stMpiAttr, phWindow);
    return s32Ret;
}

mt_s32 MT_UNF_VO_CreateWindowExt(const MT_UNF_WINDOW_ATTR_S *pWinAttr,
                                 mt_handle *phWindow,
                                 MT_BOOL bVirtScreen)
{
    MT_DRV_WIN_ATTR_S stMpiAttr;
    mt_s32 s32Ret;

    if (!pWinAttr)
    {
        MT_ERR_WIN("para pWinAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    if (!phWindow)
    {
        MT_ERR_WIN("para phWindow is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    s32Ret = VO_ConvertWinAttrToMPI((MT_UNF_WINDOW_ATTR_S *)pWinAttr, &stMpiAttr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_WIN("para pWinAttr is err.\n");
        return s32Ret;
    }

    s32Ret = MT_MPI_WIN_Create_Ext(&stMpiAttr, phWindow, bVirtScreen);

    return s32Ret;
}

mt_s32 MT_UNF_VO_DestroyWindow(mt_handle hWindow)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_WIN_Destroy(hWindow);

    return s32Ret;
}

mt_s32 MT_UNF_VO_SetWindowEnable(mt_handle hWindow, MT_BOOL bEnable)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_WIN_SetEnable(hWindow, bEnable);

    return s32Ret;
}

mt_s32 MT_UNF_VO_GetWindowEnable(mt_handle hWindow, MT_BOOL *pbEnable)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_WIN_GetEnable(hWindow, pbEnable);

    return s32Ret;
}

mt_s32 MT_UNF_VO_SetWindowAttr(mt_handle hWindow, const MT_UNF_WINDOW_ATTR_S *pWinAttr)
{
    MT_DRV_WIN_ATTR_S stMpiAttr;
    mt_s32 s32Ret;

    if (!pWinAttr)
    {
        MT_ERR_WIN("para pWinAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    s32Ret = VO_ConvertWinAttrToMPI((MT_UNF_WINDOW_ATTR_S *)pWinAttr, &stMpiAttr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_WIN("para pWinAttr is err.\n");
        return s32Ret;
    }

    s32Ret = MT_MPI_WIN_SetAttr(hWindow, &stMpiAttr);

    return s32Ret;
}

mt_s32 MT_UNF_VO_GetWindowAttr(mt_handle hWindow, MT_UNF_WINDOW_ATTR_S *pWinAttr)
{
    MT_DRV_WIN_ATTR_S stMpiAttr;
    mt_s32 s32Ret;

    if (!pWinAttr)
    {
        MT_ERR_WIN("para pWinAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    s32Ret = MT_MPI_WIN_GetAttr(hWindow, &stMpiAttr);
    if (!s32Ret)
    {
        VO_ConvertWinAttrToUNF(&stMpiAttr, pWinAttr);
    }

    return s32Ret;
}
mt_s32 MT_UNF_VO_QFrame(mt_handle hWindow, void *pFrame1)
{
    mt_s32 s32Ret;
    MT_DRV_VIDEO_FRAME_S pFrame;
    memset(&pFrame,0,sizeof(MT_DRV_VIDEO_FRAME_S));
    pFrame.u32Width = 1920;
    pFrame.u32Height= 1080;
    pFrame.ePixFormat=MT_DRV_PIX_FMT_NV12 ;

    MT_INFO_WIN("[%s]line %d\n", __FUNCTION__, __LINE__);

    s32Ret = MT_MPI_WIN_QueueFrame(hWindow, &pFrame);

    return s32Ret;
}

mt_s32 MT_UNF_VO_GetFrame(mt_handle hWindow, void *pFrame)
{
    mt_s32 s32Ret;

    MT_INFO_WIN("[%s]line %d\n", __FUNCTION__, __LINE__);

    s32Ret = MT_MPI_WIN_GetFrame(hWindow, (MT_DRV_VIDEO_FRAME_S *)pFrame);

    return s32Ret;
}


mt_s32 MT_UNF_VO_SENDPARA(mt_handle hWindow, void *pInfo)
{
    mt_s32 s32Ret;
    mt_u32 *p_u32tmp = pInfo;
    MT_DRV_WIN_PARA_S para;
    int w=1920,h=1080;
    if(!p_u32tmp)
            return MT_ERR_VO_NULL_PTR;
    if(*p_u32tmp == 1)
        para.u32ctlcmd = 0x1;
    else if(*p_u32tmp == 2)
    {
        para.u32ctlcmd = 0x2;
        para.send[0] = (mt_u32)((w>>1)|(1<<16));
        para.send[1] = (mt_u32)((h>>1)|(1<<16));
    }
    else if(*p_u32tmp == 3)
    {
        para.u32ctlcmd = 0x3;
        para.send[0] = (mt_u32)((w>>2)|(1<<16));
        para.send[1] = (mt_u32)((h>>2)|(1<<16));
    }
    else if(*p_u32tmp == 4)
    {
        para.u32ctlcmd = 0x4;
        para.send[0] = (mt_u32)((w*5/4)|((w>>3)<<16));
        para.send[1] = (mt_u32)((h*5/4)|((h>>3)<<16));
    }
    para.hwin = hWindow;

    MT_INFO_WIN("[%s]line %d\n", __FUNCTION__, __LINE__);

    s32Ret = MT_MPI_WIN_SET_PARA(hWindow, &para);

    return s32Ret;
}

mt_s32 MT_UNF_VO_MAPFrame(mt_handle hWindow, phys_addr_t paddr)
{
    mt_s32 s32Ret;
    MT_DRV_WIN_PARA_S para;
    MT_INFO_WIN("[%s]line %d\n", __FUNCTION__, __LINE__);
    if (!paddr)
        return MT_ERR_VO_NULL_PTR;
    para.hwin = hWindow;
    para.u32ctlcmd = 0x100;
    para.send[0] = paddr;
    s32Ret = MT_MPI_WIN_SET_PARA(hWindow, &para);

    return s32Ret;
}

mt_s32 MT_UNF_VO_AcquireFrame(mt_handle hWindow, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameinfo, mt_u32 u32TimeoutMs)
{
    MT_DRV_VIDEO_FRAME_S stMpi;
    mt_s32 s32TimeRet;
    mt_s32 s32Ret;
    mt_u32 u32OriTime = 0;
    mt_u32 u32Time = 0;
    mt_u32 u32Delta;

    if (!pstFrameinfo)
    {
        MT_ERR_WIN("para pstFrameinfo is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    stMpi.stLBufAddr[0].u32YAddr = pstFrameinfo->stLinearFrameAddr[0].u32YAddr;
    stMpi.stLBufAddr[0].u32CAddr = pstFrameinfo->stLinearFrameAddr[0].u32CAddr;
    stMpi.stLBufAddr[0].u32CrAddr = pstFrameinfo->stLinearFrameAddr[0].u32CrAddr;
    stMpi.stLBufAddr[0].u32BufSize = pstFrameinfo->stLinearFrameAddr[0].u32BufSize;
    stMpi.stLBufAddr[1].u32YAddr = pstFrameinfo->stLinearFrameAddr[1].u32YAddr;
    stMpi.stLBufAddr[1].u32CAddr = pstFrameinfo->stLinearFrameAddr[1].u32CAddr;
    stMpi.stLBufAddr[1].u32CrAddr = pstFrameinfo->stLinearFrameAddr[1].u32CrAddr;
    stMpi.stLBufAddr[1].u32BufSize = pstFrameinfo->stLinearFrameAddr[1].u32BufSize;
    s32TimeRet = mt_sys_get_time_stamp_ms(&u32OriTime); //MT_SYS_GetTimeStampMs(&u32OriTime);
    if (s32TimeRet != MT_SUCCESS)
    {
        MT_ERR_WIN("GetTimeStampMs Failed\n");
        return MT_ERR_VO_OPERATION_DENIED;
    }

    do
    {
        s32Ret = MT_MPI_WIN_AcquireFrame(hWindow, &stMpi);
        if (!s32Ret)
        {
            Transfer_Frame(pstFrameinfo, &stMpi, MT_FALSE);
        }

        s32TimeRet = mt_sys_get_time_stamp_ms(&u32OriTime); //MT_SYS_GetTimeStampMs(&u32Time);
        if (s32TimeRet != MT_SUCCESS)
        {
            MT_ERR_WIN("GetTimeStampMs Failed\n");
            if (!s32Ret)
            {
                return s32Ret;
            }
            else
            {
                return MT_ERR_VO_OPERATION_DENIED;
            }
        }

        u32Delta = u32Time - u32OriTime;
        (mt_void) MT_USLEEP(1 * 1000);

    } while (s32Ret == MT_FAILURE && u32Delta <= u32TimeoutMs);

    return s32Ret;
}

mt_s32 MT_UNF_VO_ReleaseFrame(mt_handle hWindow, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameinfo)
{
    MT_DRV_VIDEO_FRAME_S stMpi;
    mt_s32 s32Ret;

    if (!pstFrameinfo)
    {
        MT_ERR_WIN("para pstFrameinfo is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    s32Ret = Transfer_Frame(pstFrameinfo, &stMpi, MT_TRUE);
    if (MT_SUCCESS == s32Ret)
    {
        s32Ret = MT_MPI_WIN_ReleaseFrame(hWindow, &stMpi);
    }

    return s32Ret;
}
#if 0
mt_s32 MT_UNF_VO_SetWindowZorder(mt_handle hWindow, MT_LAYER_ZORDER_E enZFlag)
{
    MT_DRV_DISP_ZORDER_E enZorder;
    mt_s32 s32Ret;

    if (enZFlag >= MT_LAYER_ZORDER_BUTT)
    {
        MT_ERR_WIN("Invalid zorder parameter!\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    Transfe_ZOrder(&enZFlag, &enZorder, MT_TRUE);

    s32Ret = MT_MPI_WIN_SetZorder(hWindow, enZorder);

    return s32Ret;
}

mt_s32 MT_UNF_VO_GetWindowZorder(mt_handle hWindow, mt_u32 *pu32Zorder)
{
    mt_s32 s32Ret;
    if (!pu32Zorder)
    {
        MT_ERR_WIN("para pu32Zorder is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    s32Ret = MT_MPI_WIN_GetZorder(hWindow, pu32Zorder);

    return s32Ret;
}
#endif
mt_s32 MT_UNF_VO_AttachWindow(mt_handle hWindow, mt_handle hSrc)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u8 u8Handle = (hSrc >> 16) & 0xFF;

    if (MT_ID_AVPLAY == u8Handle)
    {
        s32Ret = MT_MPI_AVPLAY_AttachWindow(hSrc, hWindow); //other_module
    }
#ifdef MT_VI_SUPPORT
    else if (MT_ID_VI == u8Handle)
    {
        s32Ret = MT_MPI_VI_Attach(hSrc, hWindow);
    }
#endif
    else
    {
        MT_ERR_WIN("invalid handle!\n");
        s32Ret = MT_FAILURE;
    }

    if (s32Ret)
    {
        MT_ERR_WIN("Vo AttachWindow failed!\n");
        return s32Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_VO_DetachWindow(mt_handle hWindow, mt_handle hSrc)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u8 u8Handle = (hSrc >> 16) & 0xFF;

    if (MT_ID_AVPLAY == u8Handle)
    {
        s32Ret = MT_MPI_AVPLAY_DetachWindow(hSrc, hWindow); //other_module
    }
#ifdef MT_VI_SUPPORT
    else if (MT_ID_VI == u8Handle)
    {
        s32Ret = MT_MPI_VI_Detach(hSrc, hWindow);
    }
#endif
    else
    {
        MT_ERR_WIN("invalid handle!\n");
        s32Ret = MT_FAILURE;
    }

    if (s32Ret)
    {
        MT_ERR_WIN("Vo AVPLAY_DettachWindow failed!\n");
        return s32Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_VO_FreezeWindow(mt_handle hWindow, MT_BOOL bEnable, MT_UNF_WINDOW_FREEZE_MODE_E enWinFreezeMode)
{
    MT_DRV_WIN_SWITCH_E eFrzMode;
    mt_s32 s32Ret;

    Transfe_SwitchMode(&enWinFreezeMode, &eFrzMode, MT_TRUE);
    s32Ret = MT_MPI_WIN_Freeze(hWindow, bEnable, eFrzMode);

    return s32Ret;
}

mt_s32 MT_UNF_VO_GetWindowFreezeStatus(mt_handle hWindow, MT_BOOL *pbEnable, MT_UNF_WINDOW_FREEZE_MODE_E *penWinFreezeMode)
{
    MT_DRV_WIN_SWITCH_E eFrzMode;
    mt_s32 s32Ret;

    s32Ret = MT_MPI_WIN_GetFreezeStat(hWindow, pbEnable, &eFrzMode);
    if (s32Ret == MT_SUCCESS)
    {
        Transfe_SwitchMode(penWinFreezeMode, &eFrzMode, MT_FALSE);
    }

    return s32Ret;
}

mt_s32 MT_UNF_VO_SetWindowFieldMode(mt_handle hWindow, MT_BOOL bEnable)
{
    return MT_ERR_VO_WIN_UNSUPPORT;
}

mt_s32 MT_UNF_VO_GetWindowFieldMode(mt_handle hWindow, MT_BOOL *pbEnable)
{
    return MT_ERR_VO_WIN_UNSUPPORT;
}

mt_s32 MT_UNF_VO_ResetWindow(mt_handle hWindow, MT_UNF_WINDOW_FREEZE_MODE_E enWinFreezeMode)
{
    MT_DRV_WIN_SWITCH_E eRstMode;
    mt_s32 s32Ret;

    Transfe_SwitchMode(&enWinFreezeMode, &eRstMode, MT_TRUE);
    s32Ret = MT_MPI_WIN_Reset(hWindow, eRstMode);

    return s32Ret;
}

mt_s32 MT_UNF_VO_AttachExternBuffer(mt_handle hWindow, MT_UNF_BUFFER_ATTR_S *pstBufAttr)
{
    MT_DRV_VIDEO_BUFFER_POOL_S stBufPool;
    mt_s32 s32Ret;

    memset(&stBufPool, 0, sizeof(MT_DRV_VIDEO_BUFFER_POOL_S));
    if (!pstBufAttr)
    {
        MT_ERR_WIN("para pstBufAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    Transfer_BufferPool(pstBufAttr, &stBufPool, MT_TRUE);

    s32Ret = MT_MPI_WIN_SetExtBuffer(hWindow, &stBufPool);

    return s32Ret;
}

mt_s32 MT_UNF_VO_SetQuickOutputEnable(mt_handle hWindow, MT_BOOL bQuickOutputEnable)
{
    mt_s32 s32Ret;

    if ((MT_TRUE != bQuickOutputEnable) && (MT_FALSE != bQuickOutputEnable))
    {
        MT_ERR_WIN("para bQuickOutputEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    s32Ret = MT_MPI_WIN_SetQuickOutput(hWindow, bQuickOutputEnable);
    return s32Ret;
}

mt_s32 MT_UNF_VO_GetQuickOutputStatus(mt_handle hWindow, MT_BOOL *pbQuickOutputEnable)
{
    mt_s32 s32Ret;

    if (!pbQuickOutputEnable)
    {
        MT_ERR_WIN("param null ptr.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    s32Ret = MT_MPI_WIN_GetQuickOutputStatus(hWindow, pbQuickOutputEnable);
    return s32Ret;
}

mt_s32 MT_UNF_VO_CapturePicture(mt_handle hWindow, MT_UNF_VIDEO_FRAME_INFO_S *pstCapPicture)
{
    MT_DRV_VIDEO_FRAME_S stMpi;
    mt_s32 s32Ret = MT_SUCCESS;

    if (!pstCapPicture)
    {
        MT_ERR_WIN("para pstCapPicture is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    memset((void *)&stMpi, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
    s32Ret = MT_MPI_WIN_CapturePicture(hWindow, &stMpi);
    if (s32Ret == MT_SUCCESS)
    {
        Transfer_Frame(pstCapPicture, &stMpi, MT_FALSE);
    }

    return s32Ret;
}

mt_s32 MT_UNF_VO_CapturePictureRelease(mt_handle hWindow, MT_UNF_VIDEO_FRAME_INFO_S *pstCapPicture)
{
    MT_DRV_VIDEO_FRAME_S stMpi;
    mt_s32 s32Ret = MT_SUCCESS;

    if (!pstCapPicture)
    {
        MT_ERR_WIN("para pstCapPicture is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    s32Ret = Transfer_Frame(pstCapPicture, &stMpi, MT_TRUE);

    if (MT_SUCCESS == s32Ret)
    {
        s32Ret = MT_MPI_WIN_CapturePictureRelease(hWindow, &stMpi);
    }

    return s32Ret;
}

mt_s32 MT_UNF_VO_SetRotation(mt_handle hWindow, MT_UNF_VO_ROTATION_E enRotation)
{
    MT_DRV_ROT_ANGLE_E eRot = MT_DRV_ROT_ANGLE_BUTT;
    mt_s32 s32Ret;

    s32Ret = Transfe_Rotate(&enRotation, &eRot, MT_TRUE);
    if (s32Ret)
    {
        MT_ERR_WIN("para enRotation is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    s32Ret = MT_MPI_WIN_SetRotation(hWindow, eRot);
    return s32Ret;
}

mt_s32 MT_UNF_VO_GetRotation(mt_handle hWindow, MT_UNF_VO_ROTATION_E *penRotation)
{
    MT_DRV_ROT_ANGLE_E eRot;
    mt_s32 s32Ret;
    if (!penRotation)
    {
        MT_ERR_WIN("para penRotation is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    s32Ret = MT_MPI_WIN_GetRotation(hWindow, &eRot);
    if (!s32Ret)
    {
        Transfe_Rotate(penRotation, &eRot, MT_FALSE);
    }

    return s32Ret;
}

mt_s32 MT_UNF_VO_SetFlip(mt_handle hWindow, MT_BOOL bHoriFlip, MT_BOOL bVertFlip)
{
    mt_s32 s32Ret;
    s32Ret = MT_MPI_WIN_SetFlip(hWindow, bHoriFlip, bVertFlip);
    return s32Ret;
}

mt_s32 MT_UNF_VO_GetFlip(mt_handle hWindow, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip)
{
    mt_s32 s32Ret;

    if ((!pbVertFlip) || (!pbHoriFlip))
    {
        MT_ERR_WIN("para is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }
    s32Ret = MT_MPI_WIN_GetFlip(hWindow, pbHoriFlip, pbVertFlip);
    return s32Ret;
}

mt_s32 MT_UNF_VO_SetStereoDetpth(mt_handle hWindow, mt_s32 s32Depth)
{
    return MT_ERR_VO_WIN_UNSUPPORT;
}

mt_s32 MT_UNF_VO_GetStereoDetpth(mt_handle hWindow, mt_s32 *ps32Depth)
{
    return MT_ERR_VO_WIN_UNSUPPORT;
}
