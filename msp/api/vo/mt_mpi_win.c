/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_win.c
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
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <linux/types.h>


#include "mt_drv_video.h"
#include "mt_drv_disp.h"
#include "mt_mpi_win.h"
#include "drv_win_ioctl.h"

#include "mt_mpi_avplay.h"
#include "mt_error_mpi.h"
#include "mt_drv_struct.h"
//#include "drv_vdec_ext.h"

#include "drv_venc_ext.h"

mt_void InitCompressor(mt_void);

int decompress(unsigned char *pData, int DataLen, int Width, int Height, int stride_luma, int stride_chrome, unsigned char *pFrame);

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

typedef struct mtDRV_CUR_QUEUE_FRAME_S
{
    mt_handle hWindow;
    MT_DRV_VIDEO_FRAME_S stFrame;
}MT_DRV_CUR_QUEUE_FRAME_S;


static mt_s32           g_VoDevFd = -1;
static const mt_char    g_VoDevName[] = "/dev/"UMAP_DEVNAME_VO;
static pthread_mutex_t  g_VoMutex = PTHREAD_MUTEX_INITIALIZER;

static MT_DRV_CUR_QUEUE_FRAME_S current_queued_frame[MAX_WIN_NUM] = {0};

#define MT_VO_LOCK()     (void)pthread_mutex_lock(&g_VoMutex);
#define MT_VO_UNLOCK()   (void)pthread_mutex_unlock(&g_VoMutex);

#define CHECK_VO_INIT()\
do{\
    MT_VO_LOCK();\
    if (g_VoDevFd < 0)\
    {\
        MT_ERR_WIN("VO is not init.\n");\
        MT_VO_UNLOCK();\
        return MT_ERR_VO_NO_INIT;\
    }\
    MT_VO_UNLOCK();\
}while(0)

mt_s32 MT_MPI_VO_SetMainWindowEnable(mt_handle hWindow, MT_BOOL bEnable);
mt_s32 MT_MPI_VO_GetMainWindowEnable(mt_handle hWindow, MT_BOOL *pbEnable);
mt_s32 MT_MPI_VO_GetWindowsVirtual(mt_handle hWindow, MT_BOOL *pbVirutal);
mt_s32 MT_MPI_WIN_Attach(mt_handle hWindow, mt_handle hSrc);
mt_s32 MT_MPI_WIN_Detach(mt_handle hWindow, mt_handle hSrc);
mt_s32 MT_MPI_VO_SetWindowRatio(mt_handle hWindow, mt_u32 u32WinRatio);
mt_s32 MT_MPI_WIN_SetFieldMode(mt_handle hWindow, MT_BOOL bEnable);
mt_s32 MT_MPI_WIN_DDequeueFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame);
//mt_s32 MT_MPI_VO_GetWindowDelay(mt_handle hWindow, MT_DRV_WIN_PLAY_INFO_S *pDelay);
mt_s32 MT_MPI_VO_DisableDieMode(mt_handle hWindow);
mt_s32 MT_MPI_VO_UseDNRFrame(mt_handle hWindow, MT_BOOL bEnable);
mt_s32 MT_MPI_VO_CapturePictureExt(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pstCapPicture);
mt_s32 MT_MPI_WIN_GetHandle(WIN_GET_HANDLE_S *pstWinHandle);

mt_s32 MT_MPI_WIN_Init(mt_void)
{
    struct stat st;

    MT_VO_LOCK();

    if (g_VoDevFd > 0)
    {
        MT_VO_UNLOCK();
        return MT_SUCCESS;
    }

    if (MT_FAILURE == stat(g_VoDevName, &st))
    {
        MT_FATAL_WIN("VO is not exist.\n");
        MT_VO_UNLOCK();
        return MT_ERR_VO_DEV_NOT_EXIST;
    }

    if (!S_ISCHR (st.st_mode))
    {
        MT_FATAL_WIN("VO is not device.\n");
        MT_VO_UNLOCK();
        return MT_ERR_VO_NOT_DEV_FILE;
    }

    g_VoDevFd = open(g_VoDevName, O_RDWR|O_NONBLOCK| O_CLOEXEC, 0);

    if (g_VoDevFd < 0)
    {
        MT_FATAL_WIN("open VO err.\n");
        MT_VO_UNLOCK();
        return MT_ERR_VO_DEV_OPEN_ERR;
    }

    MT_VO_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_DeInit(mt_void)
{
    mt_s32 Ret;

    MT_VO_LOCK();

    if (g_VoDevFd < 0)
    {
        MT_VO_UNLOCK();
        return MT_SUCCESS;
    }

    Ret = close(g_VoDevFd);

    if(MT_SUCCESS != Ret)
    {
        MT_FATAL_WIN("DeInit VO err.\n");
        MT_VO_UNLOCK();
        return MT_ERR_VO_DEV_CLOSE_ERR;
    }

    g_VoDevFd = -1;

    MT_VO_UNLOCK();

    return MT_SUCCESS;
}


mt_s32 MT_MPI_WIN_Create(const MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWindow)
{
    mt_s32           Ret;
    mt_s32             i;
    WIN_CREATE_S  VoWinCreate;

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

    if (pWinAttr->enDisp >= MT_DRV_DISPLAY_BUTT
        && pWinAttr->bVirtual == MT_FALSE)
    {
        MT_ERR_WIN("para pWinAttr->enVo is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (pWinAttr->enARCvrs >= MT_DRV_ASP_RAT_MODE_BUTT)
    {
        MT_ERR_WIN("para pWinAttr->enAspectCvrs is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    memcpy(&VoWinCreate.WinAttr, pWinAttr, sizeof(MT_DRV_WIN_ATTR_S));
    VoWinCreate.bVirtScreen = MT_TRUE;

    Ret = ioctl(g_VoDevFd, CMD_WIN_CREATE, &VoWinCreate);
    if (Ret != MT_SUCCESS)
    {
	    MT_ERR_WIN("  MT_MPI_WIN_Create failed.\n");
        return Ret;
    }

    *phWindow = VoWinCreate.hWindow;

    for(i = 0; i < MAX_WIN_NUM; i++)
    {
        if(current_queued_frame[i].hWindow == VoWinCreate.hWindow)
        {
            memset(&current_queued_frame[i].stFrame, 0, sizeof(current_queued_frame[i].stFrame));
            break;
        }
        else if(current_queued_frame[i].hWindow == 0)
        {
            current_queued_frame[i].hWindow = VoWinCreate.hWindow;
            memset(&current_queued_frame[i].stFrame, 0, sizeof(current_queued_frame[i].stFrame));
            break;
        }
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_Create_Ext(const MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWindow, MT_BOOL bVirtScreen)
{
    mt_s32           Ret;
    WIN_CREATE_S  VoWinCreate;

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

    if (pWinAttr->enDisp >= MT_DRV_DISPLAY_BUTT
        && pWinAttr->bVirtual == MT_FALSE)
    {
        MT_ERR_WIN("para pWinAttr->enVo is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (pWinAttr->enARCvrs >= MT_DRV_ASP_RAT_MODE_BUTT)
    {
        MT_ERR_WIN("para pWinAttr->enAspectCvrs is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    memcpy(&VoWinCreate.WinAttr, pWinAttr, sizeof(MT_DRV_WIN_ATTR_S));
    VoWinCreate.bVirtScreen = bVirtScreen;

    Ret = ioctl(g_VoDevFd, CMD_WIN_CREATE, &VoWinCreate);
    if (Ret != MT_SUCCESS)
    {
	    MT_ERR_WIN("  MT_MPI_WIN_Create failed.\n");
        return Ret;
    }

    *phWindow = VoWinCreate.hWindow;

    return MT_SUCCESS;
}


mt_s32 MT_MPI_WIN_Destroy(mt_handle hWindow)
{
    mt_s32      Ret;
    mt_s32      i;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    for(i = 0; i < MAX_WIN_NUM; i++)
    {
        if(current_queued_frame[i].hWindow == hWindow)
        {
            memset(&current_queued_frame[i], 0, sizeof(current_queued_frame[0]));
            break;
        }
    }

    Ret = ioctl(g_VoDevFd, CMD_WIN_DESTROY, &hWindow);

    return Ret;
}

mt_s32 MT_MPI_WIN_GetInfo(mt_handle hWin, MT_DRV_WIN_INFO_S * pstInfo)
{
    mt_s32 Ret;
    WIN_PRIV_INFO_S WinPriv;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_VO_INIT();

    WinPriv.hWindow = hWin;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_INFO, &WinPriv);
	if (!Ret)
	{
	    *pstInfo = WinPriv.stPrivInfo;
	}

    return Ret;
}
mt_s32 MT_MPI_WIN_SET_PARA(mt_handle hWin,MT_DRV_WIN_PARA_S *para)
{
    mt_s32 Ret;
    //WIN_PLAY_INFO_S WinPlay;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_VO_INIT();


    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_PARA, para);
    return Ret;

}
mt_s32 MT_MPI_WIN_CleanAllFrm(mt_handle hWin, MT_DRV_WIN_FLUSH_TYPE_E eType)
{
    mt_s32 Ret;
    MT_DRV_WIN_PARA_S para;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_VO_INIT();
    para.hwin = hWin;
    para.send[0] = (mt_u32)eType;


    Ret = ioctl(g_VoDevFd, CMD_WIN_CLEAN_ALLFRAME, &para);
    return Ret;

}

mt_s32 MT_MPI_WIN_GetPlayInfo(mt_handle hWin, MT_DRV_WIN_PLAY_INFO_S * pstInfo)
{
    mt_s32 Ret;
    WIN_PLAY_INFO_S WinPlay;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_VO_INIT();

    WinPlay.hWindow = hWin;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_PLAY_INFO, &WinPlay);
	if (!Ret)
	{
	    *pstInfo = WinPlay.stPlayInfo;
	}

    return Ret;
}


mt_s32 MT_MPI_WIN_SetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc)
{
    mt_s32 Ret;
    WIN_SOURCE_S VoWinAttach;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_VO_INIT();

    VoWinAttach.hWindow = hWin;
    VoWinAttach.stSrc   = *pstSrc;

    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_SOURCE, &VoWinAttach);

    return Ret;
}

mt_s32 MT_MPI_WIN_GetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc)
{

    return MT_FAILURE;
}





mt_s32 MT_MPI_WIN_SetEnable(mt_handle hWindow, MT_BOOL bEnable)
{
    mt_s32            Ret;
    WIN_ENABLE_S   VoWinEnable;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if ((bEnable != MT_TRUE)
      &&(bEnable != MT_FALSE)
       )
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    VoWinEnable.hWindow = hWindow;
    VoWinEnable.bEnable = bEnable;

    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_ENABLE, &VoWinEnable);

    return Ret;
}


mt_s32 MT_MPI_VO_SetMainWindowEnable(mt_handle hWindow, MT_BOOL bEnable)
{

    return MT_FAILURE;
}


mt_s32 MT_MPI_WIN_GetEnable(mt_handle hWindow, MT_BOOL *pbEnable)
{
    mt_s32            Ret;
    WIN_ENABLE_S   VoWinEnable;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pbEnable)
    {
        MT_ERR_WIN("para pbEnable is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinEnable.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_ENABLE, &VoWinEnable);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pbEnable = VoWinEnable.bEnable;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VO_GetMainWindowEnable(mt_handle hWindow, MT_BOOL *pbEnable)
{


    return MT_FAILURE;
}


mt_s32 MT_MPI_VO_GetWindowsVirtual(mt_handle hWindow, MT_BOOL *pbVirutal)
{


    return MT_FAILURE;
}

mt_s32 MT_MPI_WIN_AcquireFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrameinfo)
{
    mt_s32              Ret;
    WIN_FRAME_S      VoWinFrame;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pFrameinfo)
    {
        MT_ERR_WIN("para pFrameinfo is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinFrame.hWindow = hWindow;
    memcpy(&(VoWinFrame.stFrame.stLBufAddr[0]), &(pFrameinfo->stLBufAddr[0]),2*sizeof(MT_DRV_LINEAR_FRAME_ADDR_S));

    Ret = ioctl(g_VoDevFd, CMD_WIN_VIR_ACQUIRE, &VoWinFrame);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    memcpy(pFrameinfo, &(VoWinFrame.stFrame), sizeof(MT_DRV_VIDEO_FRAME_S));

    return MT_SUCCESS;

}

mt_s32 MT_MPI_WIN_ReleaseFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrameinfo)
{
    mt_s32              Ret;
    WIN_FRAME_S      VoWinFrame;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    VoWinFrame.hWindow = hWindow;
	VoWinFrame.stFrame = *pFrameinfo;

    Ret = ioctl(g_VoDevFd, CMD_WIN_VIR_RELEASE, &VoWinFrame);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_SetAttr(mt_handle hWindow, const MT_DRV_WIN_ATTR_S *pWinAttr)
{
    mt_s32           Ret;
    WIN_CREATE_S  VoWinCreate;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pWinAttr)
    {
        MT_ERR_WIN("para pWinAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }


    if (pWinAttr->enARCvrs >= MT_DRV_ASP_RAT_MODE_BUTT)
    {
        MT_ERR_WIN("para pWinAttr->enAspectCvrs is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    VoWinCreate.hWindow = hWindow;
    memcpy(&VoWinCreate.WinAttr, pWinAttr, sizeof(MT_DRV_WIN_ATTR_S));

    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_ATTR, &VoWinCreate);

    return Ret;
}

mt_s32 MT_MPI_WIN_GetAttr(mt_handle hWindow, MT_DRV_WIN_ATTR_S *pWinAttr)
{
    mt_s32           Ret;
    WIN_CREATE_S  VoWinCreate;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pWinAttr)
    {
        MT_ERR_WIN("para pWinAttr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinCreate.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_ATTR, &VoWinCreate);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    memcpy(pWinAttr, &VoWinCreate.WinAttr, sizeof(MT_DRV_WIN_ATTR_S));

    return MT_SUCCESS;
}

#if 0
mt_s32 MT_MPI_WIN_SetZorder(mt_handle hWindow, MT_DRV_DISP_ZORDER_E enZFlag)
{
    mt_s32            Ret;
    WIN_ZORDER_S   VoWinZorder;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (enZFlag >= MT_DRV_DISP_ZORDER_BUTT)
    {
        MT_ERR_WIN("para enZFlag is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    VoWinZorder.hWindow = hWindow;
    VoWinZorder.eZFlag = enZFlag;

    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_ZORDER, &VoWinZorder);

    return Ret;
}

mt_s32 MT_MPI_WIN_GetZorder(mt_handle hWindow, mt_u32 *pu32Zorder)
{
    mt_s32            Ret;
    WIN_ORDER_S   VoWinOrder;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pu32Zorder)
    {
        MT_ERR_WIN("para SrcHandle is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinOrder.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_ORDER, &VoWinOrder);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pu32Zorder = VoWinOrder.Order;

    return MT_SUCCESS;
}
#endif

mt_s32 MT_MPI_WIN_Attach(mt_handle hWindow, mt_handle hSrc)
{
#if 0
    mt_s32                  Ret;
    WIN_SOURCE_S         VoWinAttach;
    mt_handle               hVdec;
    mt_handle               hSync;
    MT_UNF_AVPLAY_ATTR_S    AvplayAttr;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!hSrc)
    {
        MT_ERR_WIN("para hSrc is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    if(MT_ID_VI == ((hSrc&0xff0000)>>16))
    {
        VoWinAttach.ModId = MT_ID_VI;
    }
    else if(MT_ID_AVPLAY == ((hSrc&0xff0000)>>16))
    {
        Ret = MT_MPI_AVPLAY_GetAttr(hSrc, MT_UNF_AVPLAY_ATTR_ID_STREAM_MODE, &AvplayAttr);
        if (MT_SUCCESS != Ret)
        {
            return MT_ERR_VO_INVALID_PARA;
        }

        VoWinAttach.ModId = MT_ID_AVPLAY;
    }
    else
    {
        return MT_ERR_VO_INVALID_PARA;
    }

    if (MT_ID_AVPLAY == VoWinAttach.ModId)
    {
        Ret = MT_MPI_AVPLAY_GetSyncVdecHandle(hSrc, &hVdec, &hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_AVPLAY_GetVdecHandle failed.\n");
            return Ret;
        }
        VoWinAttach.hSrc = hVdec;
        VoWinAttach.hSync = hSync;
    }
    else
    {
        VoWinAttach.hSrc = hSrc;
    }


    VoWinAttach.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_SOURCE, &VoWinAttach);

    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    if (MT_ID_AVPLAY == VoWinAttach.ModId)
    {
        Ret = MT_MPI_AVPLAY_AttachWindow(hSrc, hWindow);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_WIN("call MT_MPI_AVPLAY_AttachWindow failed.\n");
            (mt_void)ioctl(g_VoDevFd, CMD_VO_WIN_DETACH, &VoWinAttach);
            return Ret;
        }
    }
#endif

    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_Detach(mt_handle hWindow, mt_handle hSrc)
{

    return MT_FAILURE;
}

mt_s32 MT_MPI_VO_SetWindowRatio(mt_handle hWindow, mt_u32 u32WinRatio)
{

    return MT_FAILURE;
}

mt_s32 MT_MPI_WIN_Freeze(mt_handle hWindow, MT_BOOL bEnable, MT_DRV_WIN_SWITCH_E enWinFreezeMode)
{
    mt_s32           Ret;
    WIN_FREEZE_S  VoWinFreeze;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if ((bEnable != MT_TRUE)
      &&(bEnable != MT_FALSE)
       )
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (enWinFreezeMode >= MT_DRV_WIN_SWITCH_BUTT)
    {
        MT_ERR_WIN("para enWinFreezeMode is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    VoWinFreeze.hWindow = hWindow;
    VoWinFreeze.bEnable = bEnable;
    VoWinFreeze.eMode   = enWinFreezeMode;

    Ret = ioctl(g_VoDevFd, CMD_WIN_FREEZE, &VoWinFreeze);

    return Ret;
}

mt_s32 MT_MPI_WIN_GetFreezeStat(mt_handle hWindow, MT_BOOL *bEnable, MT_DRV_WIN_SWITCH_E *enWinFreezeMode)
{
    mt_s32           Ret;
    WIN_FREEZE_S    stWinFreeze;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if ((!bEnable) || (!enWinFreezeMode))
    {
        MT_ERR_WIN("param ptr is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    stWinFreeze.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_FREEZE_STATUS, &stWinFreeze);
    if (Ret == MT_SUCCESS)
    {
        *bEnable = stWinFreeze.bEnable;
        *enWinFreezeMode = stWinFreeze.eMode;
    }

    return Ret;
}

mt_s32 MT_MPI_WIN_SetFieldMode(mt_handle hWindow, MT_BOOL bEnable)
{

    return MT_FAILURE;
}

mt_s32 MT_MPI_WIN_SendFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32             Ret;
    WIN_FRAME_S     VoWinFrame;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pFrame)
    {
        MT_ERR_WIN("para pFrame is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinFrame.hWindow  = hWindow;
	VoWinFrame.stFrame = *pFrame;

    Ret = ioctl(g_VoDevFd, CMD_WIN_SEND_FRAME, &VoWinFrame);

    return Ret;
}

mt_s32 MT_MPI_WIN_DequeueFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32  Ret;
    WIN_FRAME_S VoWinFrame;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pFrame)
    {
        MT_ERR_WIN("para pFrameinfo is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinFrame.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_DQ_FRAME, &VoWinFrame);
    if (!Ret)
    {
		*pFrame = VoWinFrame.stFrame;
    }

    return Ret;
}

mt_s32 MT_MPI_WIN_DDequeueFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32  Ret;
    WIN_FRAME_S VoWinFrame;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("[%s]line %d,para hWindow is invalid.\n",__FUNCTION__,__LINE__);
        return MT_ERR_VO_INVALID_PARA;
    }


    CHECK_VO_INIT();

    VoWinFrame.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_DQ_FRAME, &VoWinFrame);
    if (!Ret)
    {
		MT_ERR_WIN("[%s]line %d,DDequeueFrame fail\n",__FUNCTION__,__LINE__);
    }

    return Ret;
}

mt_s32 MT_MPI_WIN_QueueFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32             Ret;
    //WIN_FRAME_S     VoWinFrame;
    WIN_CMD_FRAME_S     VoWinFrame;
    mt_s32 i = 0;
    
    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pFrame)
    {
        MT_ERR_WIN("para pFrameinfo is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinFrame.hWindow = hWindow;
	//VoWinFrame.stFrame = *pFrame;
	VoWinFrame.pstFrame = pFrame;

    for(i = 0; i < MAX_WIN_NUM; i++)
    {
        if(current_queued_frame[i].hWindow == hWindow)
        {
            current_queued_frame[i].stFrame = *pFrame;
            break;
        }
    }
    
	Ret = ioctl(g_VoDevFd, CMD_WIN_QU_FRAME, &VoWinFrame);

    return Ret;
}

mt_s32 MT_MPI_WIN_GetFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32             Ret = MT_SUCCESS;
    mt_s32             i;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pFrame)
    {
        MT_ERR_WIN("para pFrame is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    for(i = 0; i < MAX_WIN_NUM; i++)
    {
        if(current_queued_frame[i].hWindow == hWindow)
        {
            *pFrame = current_queued_frame[i].stFrame;
            break;
        }
    }

    if(i == MAX_WIN_NUM)
        return MT_ERR_VO_WIN_NOT_EXIST;
        
    return Ret;
}

mt_s32 MT_MPI_WIN_QueueUselessFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32             Ret;
    WIN_FRAME_S     VoWinFrame;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pFrame)
    {
        MT_ERR_WIN("para pFrameinfo is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinFrame.hWindow = hWindow;
	VoWinFrame.stFrame = *pFrame;

    Ret = ioctl(g_VoDevFd, CMD_WIN_QU_ULSFRAME, &VoWinFrame);

    return Ret;
}


mt_s32 MT_MPI_WIN_Reset(mt_handle hWindow, MT_DRV_WIN_SWITCH_E enWinFreezeMode)
{
    WIN_RESET_S   VoWinReset;
    mt_s32 Ret;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (enWinFreezeMode >= MT_DRV_WIN_SWITCH_BUTT)
    {
        MT_ERR_WIN("para enWinFreezeMode is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    VoWinReset.hWindow = hWindow;
    VoWinReset.eMode = enWinFreezeMode;

    Ret = ioctl(g_VoDevFd, CMD_WIN_RESET, &VoWinReset);

    return Ret;
}

mt_s32 MT_MPI_WIN_Pause(mt_handle hWindow, MT_BOOL bEnable)
{
    WIN_PAUSE_S   VoWinPause;
    mt_s32           Ret;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if ((bEnable != MT_TRUE)
      &&(bEnable != MT_FALSE)
       )
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    VoWinPause.hWindow = hWindow;
    VoWinPause.bEnable = bEnable;

    Ret = ioctl(g_VoDevFd, CMD_WIN_PAUSE, &VoWinPause);

    return Ret;
}


mt_s32 MT_MPI_VO_GetWindowDelay(mt_handle hWindow, MT_DRV_WIN_PLAY_INFO_S *pDelay)
{
    WIN_PLAY_INFO_S   VoWinDelay;
    mt_s32 Ret;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!pDelay)
    {
        MT_ERR_WIN("para pDelay is null.\n");
        return MT_ERR_VO_NULL_PTR;
    }

    CHECK_VO_INIT();

    VoWinDelay.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_PLAY_INFO, &VoWinDelay);
    if (!Ret)
    {
        *pDelay = VoWinDelay.stPlayInfo;
    }

    return Ret;
}

mt_s32 MT_MPI_WIN_SetStepMode(mt_handle hWindow, MT_BOOL bStepMode)
{
    mt_s32              Ret;
    WIN_STEP_MODE_S  WinStepMode;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    WinStepMode.hWindow = hWindow;
    WinStepMode.bStep = bStepMode;

    Ret = ioctl(g_VoDevFd, CMD_WIN_STEP_MODE, &WinStepMode);

    return Ret;
}

mt_s32 MT_MPI_WIN_SetStepPlay(mt_handle hWindow)
{
    mt_s32      Ret;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    Ret = ioctl(g_VoDevFd, CMD_WIN_STEP_PLAY, &hWindow);

    return Ret;
}

mt_s32 MT_MPI_VO_DisableDieMode(mt_handle hWindow)
{
    return MT_FAILURE;
}

mt_s32 MT_MPI_WIN_SetExtBuffer(mt_handle hWindow, MT_DRV_VIDEO_BUFFER_POOL_S* pstBufAttr)
{
    mt_s32      Ret;
//    mt_s32      s32Index;
    WIN_BUF_POOL_S  bufferAttr;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    bufferAttr.hwin = hWindow;
    bufferAttr.stBufPool = *pstBufAttr;

    Ret = ioctl(g_VoDevFd, CMD_WIN_VIR_EXTERNBUF, &bufferAttr);

    return Ret;
}

mt_s32 MT_MPI_WIN_SetQuickOutput(mt_handle hWindow, MT_BOOL bEnable)
{
    WIN_SET_QUICK_S stQuickOutputAttr;
    mt_s32      Ret = MT_FAILURE;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    stQuickOutputAttr.hWindow = hWindow;
    stQuickOutputAttr.bQuickEnable = bEnable;

    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_QUICK, &stQuickOutputAttr);

    return Ret;
}

mt_s32 MT_MPI_WIN_GetQuickOutputStatus(mt_handle hWindow, MT_BOOL *bQuickOutputEnable)
{
    WIN_SET_QUICK_S stQuickOutputAttr;
    mt_s32      Ret = MT_FAILURE;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();
    stQuickOutputAttr.hWindow = hWindow;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_QUICK, &stQuickOutputAttr);
    if (Ret == MT_SUCCESS)
    {
        *bQuickOutputEnable = stQuickOutputAttr.bQuickEnable;
    }

    return Ret;
}

mt_s32 MT_MPI_VO_UseDNRFrame(mt_handle hWindow, MT_BOOL bEnable)
{
    return MT_FAILURE;
}


mt_s32 MT_MPI_VO_CapturePictureExt(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pstCapPicture)
{
    mt_s32            Ret = MT_SUCCESS;
    WIN_CAPTURE_S     VoWinCapture;
    mt_u32          datalen = 0, i = 0, y_stride = 0, height = 0;
    mt_uchar        *DecompressOutBuf = MT_NULL, *DecompressInBuf = MT_NULL;
    mt_uchar        *Inptr = MT_NULL, *Outptr = MT_NULL;

    CHECK_VO_INIT();
    if ((MT_INVALID_HANDLE == hWindow) || (!pstCapPicture))
    {
        MT_ERR_WIN(" invalid param.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    /*first,  we get a frame from the window.*/
    VoWinCapture.hWindow = hWindow;
    Ret = ioctl(g_VoDevFd, CMD_VO_WIN_CAPTURE_START, &VoWinCapture);
    if (Ret != MT_SUCCESS)
        return Ret;

    y_stride = VoWinCapture.CapPicture.stBufAddr[0].u32Stride_Y;

    height = VoWinCapture.CapPicture.u32Height;

    DecompressOutBuf = (mt_uchar *)(mt_mmap(VoWinCapture.driver_supply_addr.startPhyAddr,
                                                VoWinCapture.driver_supply_addr.length));

    if ( MT_DRV_PIX_FMT_NV21 == VoWinCapture.CapPicture.ePixFormat)
        datalen = height * y_stride * 3 / 2;
    else
        datalen = height * y_stride * 2;

    DecompressInBuf =(mt_uchar *)(mt_mmap(VoWinCapture.CapPicture.stBufAddr[0].u32PhyAddr_Y, datalen));

    /*third  step, we make a decompress or simple copy from driver-vpss frame to user frame.*/
    Inptr = DecompressInBuf;
    Outptr = DecompressOutBuf;

    if ( (MT_NULL ==Inptr) ||(MT_NULL ==Outptr)
          ||(0 == y_stride))
        return MT_FAILURE;

    for(i = 0 ; i < datalen / y_stride; i++)
    {
        memcpy(Outptr, Inptr, y_stride);
        Inptr += y_stride;
        Outptr += y_stride;
    }

    if (MT_SUCCESS != mt_munmap((void*)DecompressInBuf))
    {
        MT_ERR_WIN("decompress buffer unmap fail\r\n");
        (mt_void)mt_munmap((void*)DecompressOutBuf);
        goto release;
    }

    if (MT_SUCCESS != mt_munmap((void*)DecompressOutBuf))
    {
        MT_ERR_WIN("decompress buffer unmap fail\r\n");
        goto release;
    }

release:

    Ret = ioctl(g_VoDevFd, CMD_VO_WIN_CAPTURE_RELEASE, &VoWinCapture);
    if (Ret != MT_SUCCESS)
        return Ret;

    *pstCapPicture = VoWinCapture.CapPicture;
    /*FIXME: we should know how the 3d will be dealed with*/
    pstCapPicture->stBufAddr[0].u32PhyAddr_Y = VoWinCapture.driver_supply_addr.startPhyAddr;
    pstCapPicture->stBufAddr[1].u32PhyAddr_Y = VoWinCapture.driver_supply_addr.startPhyAddr;

    pstCapPicture->stBufAddr[0].u32PhyAddr_C =
        pstCapPicture->stBufAddr[0].u32PhyAddr_Y +
        height * y_stride ;
    pstCapPicture->stBufAddr[1].u32PhyAddr_C =
        pstCapPicture->stBufAddr[1].u32PhyAddr_Y +
        height * y_stride ;
    return MT_SUCCESS;
}


mt_s32 MT_MPI_WIN_CapturePicture(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pstCapPicture)
{
    mt_s32            Ret = MT_SUCCESS;

    Ret = MT_MPI_VO_CapturePictureExt(hWindow, pstCapPicture);
    return Ret;
}


mt_s32 MT_MPI_WIN_CapturePictureRelease(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pstCapPicture)
{
    mt_s32          Ret = MT_SUCCESS;
    WIN_CAPTURE_S   VoWinRls;

    CHECK_VO_INIT();
    if ((MT_INVALID_HANDLE == hWindow) || (!pstCapPicture))
    {
        MT_ERR_WIN("invalid  param.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    VoWinRls.hWindow = hWindow;
    VoWinRls.CapPicture = *pstCapPicture;
    VoWinRls.driver_supply_addr.startPhyAddr = pstCapPicture->stBufAddr[0].u32PhyAddr_Y;

    Ret = ioctl(g_VoDevFd, CMD_VO_WIN_CAPTURE_FREE, &VoWinRls);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_CapturePictureRelease fail (INVALID_PARA)\r\n");
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_SetRotation(mt_handle hWindow, MT_DRV_ROT_ANGLE_E enRotation)
{
    mt_s32          Ret = MT_SUCCESS;
    WIN_ROTATION_S  VoWinRotation;
    VoWinRotation.hWindow = hWindow;
    VoWinRotation.enRotation = enRotation;
    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_ROTATION, &VoWinRotation);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_SetRotation fail \r\n");
        return Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_GetRotation(mt_handle hWindow, MT_DRV_ROT_ANGLE_E *penRotation)
{
    mt_s32          Ret = MT_SUCCESS;
    WIN_ROTATION_S  VoWinRotation;
    VoWinRotation.hWindow = hWindow;
    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_ROTATION, &VoWinRotation);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_GetRotation fail \r\n");
        return Ret;
    }
    *penRotation = VoWinRotation.enRotation;
    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_SetFlip(mt_handle hWindow, MT_BOOL bHoriFlip, MT_BOOL bVertFlip)
{
    mt_s32          Ret = MT_SUCCESS;
    WIN_FLIP_S  VoWinFlip;
    VoWinFlip.hWindow = hWindow;
    VoWinFlip.bHoriFlip = bHoriFlip;
    VoWinFlip.bVertFlip = bVertFlip;
    Ret = ioctl(g_VoDevFd, CMD_WIN_SET_FLIP, &VoWinFlip);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_SetFlip fail \r\n");
        return Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_GetFlip(mt_handle hWindow, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip)
{
    mt_s32          Ret = MT_SUCCESS;
    WIN_FLIP_S  VoWinFlip;
    VoWinFlip.hWindow = hWindow;
    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_FLIP, &VoWinFlip);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_GetRotation fail \r\n");
        return Ret;
    }
    *pbHoriFlip = VoWinFlip.bHoriFlip;
    *pbVertFlip = VoWinFlip.bVertFlip;
    return MT_SUCCESS;
}

#if 0
mt_s32 MT_MPI_VO_SetWindowExtAttr(mt_handle hWindow, VO_WIN_EXTATTR_E detType, MT_BOOL bEnable)
{
    mt_s32      Ret;
    VO_WIN_DETECT_S stDetType;
    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();
    stDetType.hWindow = hWindow;
    stDetType.detType = detType;
    stDetType.bEnable = bEnable;
    Ret = ioctl(g_VoDevFd, CMD_VO_SET_DET_MODE, &stDetType);

    return Ret;
}

mt_s32 MT_MPI_VO_GetWindowExtAttr(mt_handle hWindow, VO_WIN_EXTATTR_E detType, MT_BOOL *bEnable)
{
    mt_s32      Ret;
    VO_WIN_DETECT_S stDetType;
    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_VO_INIT();

    stDetType.hWindow = hWindow;
    stDetType.detType = detType;
    Ret = ioctl(g_VoDevFd, CMD_VO_GET_DET_MODE, &stDetType);
    *bEnable = stDetType.bEnable;

    return Ret;
}
#endif

mt_s32 MT_MPI_WIN_Suspend(mt_void)
{
    mt_u32 u32Value = 0x88888888;
    mt_s32 Ret;

    CHECK_VO_INIT();

    Ret = ioctl(g_VoDevFd, CMD_WIN_SUSPEND, &u32Value);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_Suspend failed\n");
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_Resume(mt_void)
{
    mt_u32 u32Value = 0x88888888;
    mt_s32 Ret;

    CHECK_VO_INIT();

    Ret = ioctl(g_VoDevFd, CMD_WIN_RESUM, &u32Value);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_Resume failed\n");
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_WIN_GetHandle(WIN_GET_HANDLE_S *pstWinHandle)
{
    mt_s32 Ret;

    CHECK_VO_INIT();

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_HANDLE, pstWinHandle);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_GetHandle failed\n");
    }

    return Ret;
}

mt_s32 MT_MPI_WIN_GetWinParam(mt_handle hWin, MT_DRV_WIN_INTF_S *pstWinIntf)
{
    mt_s32      Ret;
    WIN_INTF_S stWinIntf;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_INTF, &stWinIntf);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_AttachSink failed\n");
    }
    else
    {

        pstWinIntf->pfAcqFrame = stWinIntf.pfAcqFrame;
        pstWinIntf->pfRlsFrame = stWinIntf.pfRlsFrame;
        pstWinIntf->pfSetWinAttr = stWinIntf.pfSetWinAttr;
    }

    return Ret;
}


mt_s32 MT_MPI_WIN_AttachWinSink(mt_handle hWin, mt_handle hSink)
{
    mt_s32      Ret;
    WIN_ATTACH_S stAttach;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    stAttach.enType = ATTACH_TYPE_SINK;
    stAttach.hWindow = hWin;
    stAttach.hMutual = hSink;

    Ret = ioctl(g_VoDevFd, CMD_WIN_ATTACH, &stAttach);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_AttachSink failed\n");
    }

    return Ret;
}

mt_s32 MT_MPI_WIN_DetachWinSink(mt_handle hWin, mt_handle hSink)
{
    mt_s32      Ret;
    WIN_ATTACH_S stAttach;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    stAttach.enType = ATTACH_TYPE_SINK;
    stAttach.hWindow = hWin;
    stAttach.hMutual = hSink;

    Ret = ioctl(g_VoDevFd, CMD_WIN_DETACH, &stAttach);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_WIN("MT_MPI_WIN_DetachSink failed\n");
    }

    return Ret;
}


mt_s32 MT_MPI_WIN_GetLatestFrameInfo(mt_handle hWin, MT_DRV_VIDEO_FRAME_S  *frame_info)
{
    mt_s32      Ret;
    WIN_FRAME_S frame_struct;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    frame_struct.hWindow = hWin;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_LATESTFRAME_INFO, &frame_struct);
    if (Ret != MT_SUCCESS)
    {
	    MT_ERR_WIN("get latest frame info failed\n");
	    return Ret;
    }

    *frame_info  = frame_struct.stFrame;
    return Ret;
}

mt_s32 MT_MPI_WIN_GetUnloadTimes(mt_handle hWin, mt_u32 *pu32Time)
{
    mt_s32      Ret;
    WIN_UNLOAD_S stWinUnload;

    if (MT_INVALID_HANDLE == hWin)
    {
        MT_ERR_WIN("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    CHECK_VO_INIT();

    stWinUnload.hWindow = hWin;

    Ret = ioctl(g_VoDevFd, CMD_WIN_GET_UNLOAD, &stWinUnload);
    if (Ret != MT_SUCCESS)
    {
	    MT_ERR_WIN("get latest frame info failed\n");
	    return Ret;
    }

    *pu32Time  = stWinUnload.u32Times;
    return Ret;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

