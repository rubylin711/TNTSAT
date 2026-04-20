/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_disp.c
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

#include "mt_mpi_disp.h"
#include "mt_drv_struct.h"
#include "mt_error_mpi.h"

#include "mt_module_debug.h"
#include "mt_drv_disp.h"
#include "drv_disp_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif





static mt_s32            g_DispDevFd    = -1;
static const mt_char     g_DispDevName[] ="/dev/"UMAP_DEVNAME_DISP;
static pthread_mutex_t   g_DispMutex = PTHREAD_MUTEX_INITIALIZER;

#define MT_DISP_LOCK()       (void)pthread_mutex_lock(&g_DispMutex);
#define MT_DISP_UNLOCK()     (void)pthread_mutex_unlock(&g_DispMutex);


#define CHECK_DISP_PTR(pointer)\
do{\
    if (!(pointer))\
    {\
        MT_ERR_DISP("para is null ptr.\n");\
        return MT_ERR_DISP_NULL_PTR;\
    }\
}while(0)



mt_s32 MT_MPI_DISP_Init(mt_void)
{
    struct stat st;

    MT_DISP_LOCK();

    if (g_DispDevFd > 0)
    {
        MT_DISP_UNLOCK();
        return MT_SUCCESS;
    }

    DISP_DEBUGF(" g_DispDevName %s  %s\n",g_DispDevName,__FUNCTION__);
    if (MT_FAILURE == stat(g_DispDevName, &st))
    {
        MT_FATAL_DISP("DISP is not exist.\n");
        MT_DISP_UNLOCK();
        return MT_ERR_DISP_DEV_NOT_EXIST;
    }

    if (!S_ISCHR (st.st_mode))
    {
        MT_FATAL_DISP("DISP is not device.\n");
        MT_DISP_UNLOCK();
        return MT_ERR_DISP_NOT_DEV_FILE;
    }

    g_DispDevFd = open(g_DispDevName, O_RDWR|O_NONBLOCK| O_CLOEXEC, 0);
     DISP_DEBUGF("g_DispDevFd %d  %s\n",g_DispDevFd,__FUNCTION__);


    if (g_DispDevFd < 0)
    {
        MT_FATAL_DISP("open DISP err.\n");
        MT_DISP_UNLOCK();
        return MT_ERR_DISP_DEV_OPEN_ERR;
    }

    MT_DISP_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_DeInit(mt_void)
{
    mt_s32 Ret;

    MT_DISP_LOCK();

    if (g_DispDevFd < 0)
    {
        MT_DISP_UNLOCK();
        return MT_SUCCESS;
    }

    Ret = close(g_DispDevFd);

    if(MT_SUCCESS != Ret)
    {
        MT_FATAL_DISP("DeInit DISP err.\n");
        MT_DISP_UNLOCK();
        return MT_ERR_DISP_DEV_CLOSE_ERR;
    }

    g_DispDevFd = -1;

    MT_DISP_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_Attach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    mt_s32         Ret;
    DISP_ATTACH_S  DispAttach;

    if (enMaster >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enMaster is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (enSlave >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enSlave is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispAttach.enMaster = enMaster;
    DispAttach.enSlave  = enSlave;

    Ret = ioctl(g_DispDevFd, CMD_DISP_ATTACH, &DispAttach);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_Detach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    mt_s32          Ret;
    DISP_ATTACH_S   DispAttach;

    if (enMaster >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enMaster is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (enSlave >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enSlave is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispAttach.enMaster = enMaster;
    DispAttach.enSlave  = enSlave;

    Ret = ioctl(g_DispDevFd, CMD_DISP_DETACH, &DispAttach);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_Open(MT_DRV_DISPLAY_E enDisp)
{
    mt_s32         Ret;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    Ret = ioctl(g_DispDevFd, CMD_DISP_OPEN, &enDisp);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_Close(MT_DRV_DISPLAY_E enDisp)
{
    mt_s32      Ret;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    Ret = ioctl(g_DispDevFd, CMD_DISP_CLOSE, &enDisp);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
    mt_s32          Ret;
    DISP_ENABLE_S   DispEnable;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((bEnable != MT_TRUE)
      &&(bEnable != MT_FALSE)
       )
    {
        MT_ERR_DISP("para bEnable is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispEnable.enDisp  = enDisp;
    DispEnable.bEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_ENABLE, &DispEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbEnable)
{
    mt_s32          Ret;
    DISP_ENABLE_S   DispEnable;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pbEnable)
    {
        MT_ERR_DISP("para pbEnable is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispEnable.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_ENABLE, &DispEnable);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pbEnable = DispEnable.bEnable;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetSdEncPqParam(MT_DRV_DISPLAY_E enDisp, DISP_SD_ENC_PQ_PARA_S *pPara)
{
    mt_s32          Ret;

    if (enDisp != MT_DRV_DISPLAY_0)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }


    
    MT_ERR_DISP("delay 100ms\n");
    MT_USLEEP(100 * 1000);
//    DISP_SD_ENC_PQ_PARA_S t_par = {DISP_SD_ENC_CFIG1, 0x00330033};
    MT_ERR_DISP("%s %d: index:%x val:%x.\n", __FUNCTION__, __LINE__, pPara->item, pPara->val);
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_SD_ENC_PQ_PARA, pPara);
    if (Ret != MT_SUCCESS)
    {
        DISP_DEBUGF(" ioctl  failed %s\n",__FUNCTION__);

        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E enStereo, MT_DRV_DISP_FMT_E enFormat)
{
    mt_s32          Ret;
    DISP_FORMAT_S   DispFormat;

    if (enDisp > MT_DRV_DISPLAY_1)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (enStereo > MT_DRV_DISP_STEREO_TAB)
    {
        MT_ERR_DISP("para enStereo is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }
#if 0
    if (enStereo == MT_DRV_DISP_STEREO_FRAME_PACKING)
    {
        if (  (enFormat < MT_DRV_DISP_FMT_1080P_24_FP)
            ||(enFormat > MT_DRV_DISP_FMT_720P_50_FP)
            )
        {
            MT_ERR_DISP("para enStereo is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
    else if (enStereo == MT_DRV_DISP_STEREO_SBS_HALF)
    {
        if (  (enFormat != MT_DRV_DISP_FMT_1080i_60)
            &&(enFormat != MT_DRV_DISP_FMT_1080i_50)
            )
        {
            MT_ERR_DISP("para enStereo is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
    else if (enStereo == MT_DRV_DISP_STEREO_TAB)
    {
        if (  (enFormat != MT_DRV_DISP_FMT_1080P_24)
            &&(enFormat != MT_DRV_DISP_FMT_720P_50)
            &&(enFormat != MT_DRV_DISP_FMT_720P_60)
            )
        {
            MT_ERR_DISP("para enStereo is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
    else  
#endif        
    {
        if (enFormat > MT_DRV_DISP_FMT_CUSTOM)
        {
            MT_ERR_DISP("para enFormat is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }

    CHECK_DISP_INIT();

    DispFormat.enDisp   = enDisp;
    DispFormat.enStereo = enStereo;
    DispFormat.enFormat = enFormat;
    DISP_DEBUGF("ioctl  before %s\n",__FUNCTION__);
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_FORMAT, &DispFormat);
    if (Ret != MT_SUCCESS)
    {
        DISP_DEBUGF(" ioctl  failed %s\n",__FUNCTION__);

        return Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_GetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E *penStereo,MT_DRV_DISP_FMT_E *penFormat)
{

    mt_s32                   Ret;
    DISP_FORMAT_S            DispFormat;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_PTR(penStereo);
    CHECK_DISP_PTR(penFormat);
    CHECK_DISP_INIT();

    DispFormat.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_FORMAT, &DispFormat);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *penFormat = DispFormat.enFormat;
    *penStereo = DispFormat.enStereo;
    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetRightEyeFirst(MT_DRV_DISPLAY_E enDisp, MT_BOOL bRFirst)
{
    mt_s32 Ret;
    DISP_R_EYE_FIRST_S stRF;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    stRF.enDisp = enDisp;
    stRF.bREFirst = bRFirst;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_R_E_FIRST, &stRF);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetVirtualScreen(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Width, mt_u32 u32Height)
{
    mt_s32 Ret;

    DISP_VIRTSCREEN_S virtscreen;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    virtscreen.enDisp = enDisp;
    virtscreen.stVirtScreen.s32X        = 0;
    virtscreen.stVirtScreen.s32Y        = 0;
    virtscreen.stVirtScreen.s32Height   = (mt_s32)u32Height;
    virtscreen.stVirtScreen.s32Width    = (mt_s32)u32Width;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_VIRTSCREEN, &virtscreen);

    return Ret;
}
mt_s32 MT_MPI_DISP_SetSmallWindow(MT_DRV_DISPLAY_E enDisp, mt_s32 xstart, mt_s32 ystart,mt_u32 u32Width, mt_u32 u32Height)
{
    mt_s32 Ret;

    DISP_VIRTSCREEN_S virtscreen;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    virtscreen.enDisp = enDisp;
    virtscreen.stVirtScreen.s32X        = xstart;
    virtscreen.stVirtScreen.s32Y        = ystart;
    virtscreen.stVirtScreen.s32Height   = (mt_s32)u32Height;
    virtscreen.stVirtScreen.s32Width    = (mt_s32)u32Width;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_SMALLWINDOW,&virtscreen);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetVirtualScreen(MT_DRV_DISPLAY_E enDisp, mt_u32 *u32Width, mt_u32 *u32Height)
{
    mt_s32 Ret;
    DISP_VIRTSCREEN_S virtscreen;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((!u32Width) || (!u32Height))
    {
        MT_ERR_DISP("para ptr is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    virtscreen.enDisp = enDisp;
    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_VIRTSCREEN, &virtscreen);

    if (Ret != MT_SUCCESS)
        return Ret;

    *u32Width    =   (mt_u32)virtscreen.stVirtScreen.s32Width;
    *u32Height   =   (mt_u32)virtscreen.stVirtScreen.s32Height;

    return Ret;
}


mt_s32 MT_MPI_DISP_SetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstOffset)
{
    mt_s32 Ret;
    DISP_SCREENOFFSET_S screen_offset;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstOffset)
    {
        MT_ERR_DISP("para ptr is null.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    screen_offset.enDisp = enDisp;
    screen_offset.stScreenOffset =  *pstOffset;
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_SCREENOFFSET, &screen_offset);

    return Ret;
}


mt_s32 MT_MPI_DISP_GetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstOffset)
{
    mt_s32 Ret;
    DISP_SCREENOFFSET_S screen_offset;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstOffset)
    {
        MT_ERR_DISP("para ptr is null.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    screen_offset.enDisp = enDisp;
    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_SCREENOFFSET, &screen_offset);
    if (Ret != MT_SUCCESS)
        return Ret;

    *pstOffset  =  screen_offset.stScreenOffset;
    return Ret;
}


mt_s32 MT_MPI_DISP_SetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 u32ARHori, mt_u32 u32ARVert)
{
    mt_s32        Ret;
    DISP_ASPECT_RATIO_S   DispDevRatio;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enVo is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispDevRatio.enDisp = enDisp;
    DispDevRatio.u32ARHori = u32ARHori;
    DispDevRatio.u32ARVert = u32ARVert;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_DEV_RATIO, &DispDevRatio);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32ARHori, mt_u32 *pu32ARVert)
{
    mt_s32        Ret;
    DISP_ASPECT_RATIO_S   DispDevRatio;

     if ( !pu32ARHori || !pu32ARVert)
    {
        MT_ERR_DISP("para pstDispAspectRatio is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enVo is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }
    CHECK_DISP_INIT();

    DispDevRatio.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_DEV_RATIO, &DispDevRatio);

    if (!Ret)
    {
        *pu32ARHori = DispDevRatio.u32ARHori;
        *pu32ARVert = DispDevRatio.u32ARVert;
    }

    return Ret;
}



mt_s32 MT_MPI_DISP_AddIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{
    mt_s32 Ret;
    DISP_SET_INTF_S DispIntf;

    CHECK_DISP_INIT();

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    DispIntf.enDisp = enDisp;
    memcpy(&DispIntf.stIntf, pstIntf, sizeof(MT_DRV_DISP_INTF_S));

    Ret = ioctl(g_DispDevFd, CMD_DISP_ADD_INTF, &DispIntf);
    return Ret;
}


mt_s32 MT_MPI_DISP_DelIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{
    mt_s32 Ret;
    DISP_SET_INTF_S DispIntf;

    CHECK_DISP_INIT();

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    DispIntf.enDisp     = enDisp;
    memcpy(&DispIntf.stIntf, pstIntf, sizeof(MT_DRV_DISP_INTF_S));

    Ret = ioctl(g_DispDevFd, CMD_DISP_DEL_INTF, &DispIntf);
    return Ret;
}


mt_s32 MT_MPI_DISP_SetTiming(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_TIMING_S *pstTiming)
{
    mt_s32          Ret;
    DISP_TIMING_S DispTiming;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstTiming)
    {
        MT_ERR_DISP("para pstLcdPara is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispTiming.enDisp = enDisp;
    memcpy(&DispTiming.stTimingPara, pstTiming, sizeof(MT_DRV_DISP_TIMING_S));

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_TIMING, &DispTiming);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetTiming(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_TIMING_S *pstTiming)
{
    mt_s32          Ret;
    DISP_TIMING_S DispTiming;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstTiming)
    {
        MT_ERR_DISP("para pstLcdPara is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispTiming.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_TIMING, &DispTiming);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    memcpy(pstTiming,&DispTiming.stTimingPara,sizeof(MT_DRV_DISP_TIMING_S));
    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetLayerZorder(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ZORDER_ABS_E enZFlag)
{
    mt_s32           Ret;
    DISP_ZORDER_S    DispZorder;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (enZFlag >= MT_DRV_DISP_ZORDER_ABS_BUTT)
    {
        MT_ERR_DISP("para enZFlag is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispZorder.enDisp = enDisp;
    DispZorder.ZFlag = enZFlag;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_ZORDER, &DispZorder);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetLayerZorder(MT_DRV_DISPLAY_E enDisp,  mt_u32 *pu32Zorder)
{
    mt_s32           Ret;
    DISP_ORDER_S     DispOrder;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pu32Zorder)
    {
        MT_ERR_DISP("para pu32Zorder is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispOrder.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_ORDER, &DispOrder);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pu32Zorder = DispOrder.Order;
    return MT_SUCCESS;
}


#define MT_DISP_BGC_MAX_VALUE 255
mt_s32 MT_MPI_DISP_SetBGColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor)
{
    mt_s32          Ret;
    DISP_BGC_S      DispBgc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstBgColor)
    {
        MT_ERR_DISP("para pstBgColor is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispBgc.enDisp = enDisp;
    memcpy(&DispBgc.stBgColor, pstBgColor, sizeof(MT_DRV_DISP_COLOR_S));

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_BGC, &DispBgc);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetBGColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor)
{
    mt_s32          Ret;
    DISP_BGC_S      DispBgc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstBgColor)
    {
        MT_ERR_DISP("para pstBgColor is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispBgc.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_BGC, &DispBgc);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    memcpy(pstBgColor, &DispBgc.stBgColor, sizeof(MT_DRV_DISP_COLOR_S));

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetColorbar(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
    mt_s32          Ret;
    DISP_ENABLE_S      DispColorbar;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispColorbar.enDisp = enDisp;
    DispColorbar.bEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_COLORBAR, &DispColorbar);

    return Ret;

}

mt_s32 MT_MPI_DISP_SetOutputEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
    mt_s32          Ret;
    DISP_ENABLE_S      DispOutputEnable;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispOutputEnable.enDisp = enDisp;
    DispOutputEnable.bEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_OUTPUT_ENABLE, &DispOutputEnable);

    return Ret;

}




mt_s32 MT_MPI_DISP_SetBrightness(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Brightness)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (u32Brightness > 100)
    {
        MT_ERR_DISP("para u32Brightness is %d invalid.\n", u32Brightness);
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;
    DispCsc.CscValue = u32Brightness;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_BRIGHT, &DispCsc);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetBrightness(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Brightness)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pu32Brightness)
    {
        MT_ERR_DISP("para pu32Brightness is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_BRIGHT, &DispCsc);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pu32Brightness = DispCsc.CscValue;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetContrast(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Contrast)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (u32Contrast > 100)
    {
        MT_ERR_DISP("para u32Contrast is %d invalid.\n", u32Contrast);
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;
    DispCsc.CscValue = u32Contrast;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_CONTRAST, &DispCsc);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetContrast(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Contrast)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pu32Contrast)
    {
        MT_ERR_DISP("para pu32Contrast is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_CONTRAST, &DispCsc);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pu32Contrast = DispCsc.CscValue;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetSaturation(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Saturation)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (u32Saturation > 100)
    {
        MT_ERR_DISP("para u32Saturation is %d invalid.\n", u32Saturation);
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;
    DispCsc.CscValue = u32Saturation;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_SATURATION, &DispCsc);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetSaturation(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Saturation)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pu32Saturation)
    {
        MT_ERR_DISP("para pu32Saturation is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_SATURATION, &DispCsc);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pu32Saturation = DispCsc.CscValue;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetHuePlus(MT_DRV_DISPLAY_E enDisp, mt_u32 u32HuePlus)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (u32HuePlus > 100)
    {
        MT_ERR_DISP("para u32HuePlus is %d invalid.\n", u32HuePlus);
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;
    DispCsc.CscValue = u32HuePlus;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_HUE, &DispCsc);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetHuePlus(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32HuePlus)
{
    mt_s32          Ret;
    DISP_CSC_S      DispCsc;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pu32HuePlus)
    {
        MT_ERR_DISP("para pu32HuePlus is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispCsc.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_HUE, &DispCsc);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *pu32HuePlus = DispCsc.CscValue;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetAlgCfg(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ALG_CFG_S *pstAlg)
{
    mt_s32          Ret;
    DISP_ALG_S      DispAlg;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

     if (NULL == pstAlg)
    {
        MT_ERR_DISP("para pstAlg is NULL.\n");
        return MT_ERR_DISP_NULL_PTR;
    }
#if 0
    if ((pstAlg->bAccEnable != MT_TRUE)
      &&(pstAlg->bAccEnable != MT_FALSE)
       )
    {
        MT_ERR_DISP("para bEnable is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstAlg->bSharpEnable != MT_TRUE)
      &&(pstAlg->bSharpEnable != MT_FALSE)
       )
    {
        MT_ERR_DISP("para bEnable is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }
#endif

    CHECK_DISP_INIT();

    DispAlg.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_ALG, &DispAlg);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetAlgCfg(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ALG_CFG_S *pstAlg)
{
    mt_s32          Ret;
    DISP_ALG_S      DispAlg;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

     if (NULL == pstAlg)
    {
        MT_ERR_DISP("para pstAlg is NULL.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispAlg.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_ALG, &DispAlg);

//    pstAlg->bAccEnable = DispAlg.stAlg.bAccEnable;
//    pstAlg->bSharpEnable = DispAlg.stAlg.bAccEnable;

    return Ret;
}


mt_s32 MT_MPI_DISP_CreateVBI(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_VBI_CFG_S *pstCfg, mt_handle *phVbi)
{
    mt_s32 Ret;
    DISP_VBI_CREATE_CHANNEL_S    DispVbiCrtChanl;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstCfg)
    {
        MT_ERR_DISP("para pstCfg is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (!phVbi)
    {
        MT_ERR_DISP("para phVbi is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }
    CHECK_DISP_INIT();

    DispVbiCrtChanl.enDisp = enDisp;

    memcpy(&DispVbiCrtChanl.stCfg, pstCfg, sizeof(MT_DRV_DISP_VBI_CFG_S));

    Ret = ioctl(g_DispDevFd, CMD_DISP_CREATE_VBI_CHANNEL, &DispVbiCrtChanl);
     if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *phVbi = DispVbiCrtChanl.hVbi;
    return Ret;
}

mt_s32 MT_MPI_DISP_DestroyVBI(mt_handle hVbi)
{
    mt_s32 Ret;
    if ( MT_INVALID_HANDLE == hVbi )
    {
        MT_ERR_DISP("para hVbi is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }
    CHECK_DISP_INIT();

    Ret = ioctl(g_DispDevFd, CMD_DISP_DESTROY_VBI_CHANNEL, &hVbi);
    return Ret;
}

mt_s32 MT_MPI_DISP_SendVBIData(mt_handle hVbi, const MT_DRV_DISP_VBI_DATA_S *pstVbiData)
{
    mt_s32          Ret;
    DISP_VBI_S      DispVbi;

    if (hVbi >= (mt_handle)MT_DRV_DISP_VBI_TYPE_BUTT)
    {
        MT_ERR_DISP("para hVbi is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstVbiData)
    {
        MT_ERR_DISP("para pstVbiData is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (!(pstVbiData->pu8DataAddr))
    {
        MT_ERR_DISP("para pstVbiData->pu8DataAddr is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (!(pstVbiData->u32DataLen))
    {
        return MT_SUCCESS;
    }

    CHECK_DISP_INIT();

    DispVbi.hVbi = hVbi;
    memcpy(&DispVbi.stVbiData, pstVbiData, sizeof(MT_DRV_DISP_VBI_DATA_S));

    Ret = ioctl(g_DispDevFd, CMD_DISP_SEND_VBI, &DispVbi);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetWss(MT_DRV_DISPLAY_E enDisp, const MT_DRV_DISP_WSS_DATA_S *pstWssData)
{
    mt_s32          Ret;
    DISP_WSS_S      DispWss;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstWssData)
    {
        MT_ERR_DISP("para pstWssData is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if ((pstWssData->bEnable != MT_TRUE)
      &&(pstWssData->bEnable != MT_FALSE)
       )
    {
        MT_ERR_DISP("para pstWssData->bEnable is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispWss.enDisp = enDisp;
    memcpy(&DispWss.WssData, pstWssData, sizeof(MT_DRV_DISP_WSS_DATA_S));

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_WSS, &DispWss);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E enMode)
{
    mt_s32          Ret;
    DISP_MCRVSN_S   DispMcrvsn;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }
    
    //mt_drv_disp_macrovision_butt --- this mode will be used to disable macrovision
    /*
    if (enmode >= mt_drv_disp_macrovision_butt)      {
        mt_err_disp("para enmode is invalid.\n");
        return mt_err_disp_invalid_para;
    }
    */
    CHECK_DISP_INIT();

    DispMcrvsn.enDisp = enDisp;
    DispMcrvsn.eMcrvsn = enMode;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_MCRVSN, &DispMcrvsn);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E *penMode)
{
    mt_s32          Ret;
    DISP_MCRVSN_S   DispMcrvsn;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!penMode)
    {
        MT_ERR_DISP("para penMode is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispMcrvsn.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_MCRVSN, &DispMcrvsn);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    *penMode = DispMcrvsn.eMcrvsn;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_SetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg)
{
    mt_s32          Ret;
    DISP_HDMIINF_S  DispHmdiIntf;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstCfg)
    {
        MT_ERR_DISP("para pstCfg is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispHmdiIntf.enDisp = enDisp;
    memcpy(&(DispHmdiIntf.HDMIInf), pstCfg, sizeof(MT_DRV_DISP_HDMI_S));
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_HDMI_INTF, &DispHmdiIntf);
    MT_INFO_DISP("CMD_DISP_SET_HDMI_INTF invoke (%d):0x%x\n", g_DispDevFd, Ret);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DISP_GetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg)
{
    mt_s32          Ret;
    DISP_HDMIINF_S  DispHmdiIntf;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstCfg)
    {
        MT_ERR_DISP("para pstCfg is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    CHECK_DISP_INIT();

    DispHmdiIntf.enDisp = enDisp;
    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_HDMI_INTF, &DispHmdiIntf);
    MT_INFO_DISP("CMD_DISP_GET_HDMI_INTF invoke (%d):0x%x\n", g_DispDevFd, Ret);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }
    memcpy(pstCfg, &(DispHmdiIntf.HDMIInf), sizeof(MT_DRV_DISP_HDMI_S));

    return MT_SUCCESS;
}

/* MPI_disp interface for CGMS; 2011-06-02, Huang Minghu */
mt_s32 MT_MPI_DISP_SetCgms(MT_DRV_DISPLAY_E enDisp, const MT_DRV_DISP_CGMSA_CFG_S *pstCgmsCgf)
{
    mt_s32      Ret = 0;
    DISP_CGMS_S DispCgms;

    /* DISP validity check */
    if(enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* pointer validity check */
    if (!pstCgmsCgf)
    {
        MT_ERR_DISP("para pstCgmsCgf is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    /* parameters assignment */
    DispCgms.enDisp    = enDisp;
    DispCgms.stCgmsCfg = *pstCgmsCgf;

    /* check validity of pstCgmsCgf->bEnable */
    if (  (pstCgmsCgf->bEnable != MT_TRUE)
        &&(pstCgmsCgf->bEnable != MT_FALSE)
       )
    {
        MT_ERR_DISP("para pstCgmsCgf->bEnable is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* check validity of pstCgmsCgf->enType */
    if (pstCgmsCgf->enType >= MT_DRV_DISP_CGMSA_TYPE_BUTT)

    {
        MT_ERR_DISP("para pstCgmsCgf->enType is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* check validity of pstCgmsCgf->enMode */
    if (pstCgmsCgf->enMode >= MT_DRV_DISP_CGMSA_MODE_BUTT)

    {
        MT_ERR_DISP("para pstCgmsCgf->enMode is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* call SET_CGMS by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_CGMS, &DispCgms);

    return Ret;

}

mt_s32 MT_MPI_DISP_Snapshot_Acquire(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S * pstSnapShotFrame)
{
	DISP_SNAPSHOT_FRAME_S stFrame;
	mt_s32 Ret;
    /* DISP validity check */
    if(enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();
    memset((void*)&stFrame, 0, sizeof(DISP_SNAPSHOT_FRAME_S));

	stFrame.enDispLayer = enDisp;
	Ret =  ioctl(g_DispDevFd, CMD_DISP_ACQUIRE_SNAPSHOT, &stFrame);

	if (!Ret)
	    memcpy(pstSnapShotFrame, &(stFrame.stFrame), sizeof(MT_DRV_VIDEO_FRAME_S));

	return Ret;
}

mt_s32 MT_MPI_DISP_Snapshot_Release(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S * pstSnapShotFrame)
{
	DISP_SNAPSHOT_FRAME_S stFrame;

    /* DISP validity check */
    if(enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

	stFrame.enDispLayer = enDisp;
	stFrame.stFrame = *pstSnapShotFrame;

	return ioctl(g_DispDevFd, CMD_DISP_RELEASE_SNAPSHOT, &stFrame);
}

mt_s32 MT_MPI_DISP_CreateCast(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CAST_CFG_S * pstCfg, mt_handle *phCast)
{
    DISP_CAST_CREATE_S stCast;
    mt_s32      Ret = 0;

    /* DISP validity check */
    if(enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* pointer validity check */
    if (!pstCfg || !phCast)
    {
        MT_ERR_DISP("par pstCfg or phCast is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    stCast.enDisp = enDisp;
    stCast.stCfg  = *pstCfg;

    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_CREATE_CAST, &stCast);
    if (!Ret)
    {
        *phCast = stCast.hCast;
    }

    return Ret;
}
mt_s32 MT_MPI_DISP_DestroyCast(mt_handle hCast)
{
    DISP_CAST_DESTROY_S stCast;
    mt_s32      Ret = 0;

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    stCast.hCast = hCast;

    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_DESTROY_CAST, &stCast);
    return Ret;
}

mt_s32 MT_MPI_DISP_SetLowDelayEnable(mt_handle hCast, MT_BOOL bEnable)
{
    DISP_LOWDELAY_ENABLE_S stCast;
    mt_s32      Ret = 0;
    CHECK_DISP_INIT();
    stCast.hCast = hCast;
    stCast.bEnable = bEnable;
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_LOWDELAY_ENABLE, &stCast);
    return Ret;
}
mt_s32 MT_MPI_DISP_SetCastEnable(mt_handle hCast, MT_BOOL bEnable)
{
    DISP_CAST_ENABLE_S stCast;
    mt_s32      Ret = 0;

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    stCast.hCast = hCast;
    stCast.bEnable = bEnable;

    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_CAST_ENABLE, &stCast);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable)
{
    DISP_CAST_ENABLE_S stCast;
    mt_s32      Ret = 0;

    /* pointer validity check */
    if (!pbEnable)
    {
        MT_ERR_DISP("par pbEnable or phCast is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    stCast.hCast = hCast;

    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_CAST_ENABLE, &stCast);
	if (!Ret)
	{
		*pbEnable = stCast.bEnable;
	}

    return Ret;
}
mt_s32 MT_MPI_DISP_AcquireCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
    DISP_CAST_FRAME_S stCast;
    mt_s32      Ret = 0;

    /* pointer validity check */
    if (!pstCastFrame)
    {
        MT_ERR_DISP("par pstCastFrame or phCast is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    stCast.hCast = hCast;

    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_ACQUIRE_CAST_FRAME, &stCast);
    if (!Ret)
    {
        *pstCastFrame = stCast.stFrame;
#if 0
        MPI_DISP_DEBUGF(" CMD_DISP_ACQUIRE_CAST_FRAME w=%d, h=%d,y=0x%x,c=0x%x\n",
                    pstCastFrame->u32Width, pstCastFrame->u32Height,
                    pstCastFrame->stBufAddr[0].u32PhyAddr_Y,
                    pstCastFrame->stBufAddr[0].u32PhyAddr_C);
#endif
    }

    return Ret;
}
mt_s32 MT_MPI_DISP_ReleaseCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
    DISP_CAST_FRAME_S stCast;
    mt_s32      Ret = 0;

    /* pointer validity check */
    if (!pstCastFrame)
    {
        MT_ERR_DISP("par pstCastFrame is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    stCast.hCast = hCast;
    stCast.stFrame = *pstCastFrame;

    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_RELEASE_CAST_FRAME, &stCast);

    return Ret;
}

mt_s32 MT_MPI_DISP_ExtAttach(mt_handle hCast, mt_handle hSink)
{
    mt_s32      Ret = MT_SUCCESS;
    DISP_EXT_ATTACH_S stAttach;

    if (MT_INVALID_HANDLE == hCast)
    {
        MT_ERR_DISP("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_DISP_INIT();

    stAttach.enType = EXT_ATTACH_TYPE_SINK;
    stAttach.hCast =  hCast;
    stAttach.hMutual = hSink;

    Ret = ioctl(g_DispDevFd, CMD_DISP_EXT_ATTACH, &stAttach);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_DISP("MT_MPI_DISP_ExtAttach failed\n");
    }

    return Ret;
}

mt_s32 MT_MPI_DISP_ExtDeAttach(mt_handle hCast, mt_handle hSink)
{
    mt_s32      Ret = MT_SUCCESS;
    DISP_EXT_ATTACH_S stAttach;

    if (MT_INVALID_HANDLE == hCast)
    {
        MT_ERR_DISP("para hWindow is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    CHECK_DISP_INIT();

    stAttach.enType = EXT_ATTACH_TYPE_SINK;
    stAttach.hCast =  hCast;
    stAttach.hMutual = hSink;

    Ret = ioctl(g_DispDevFd, CMD_DISP_EXT_DEATTACH, &stAttach);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_DISP("MT_MPI_DISP_ExtAttach failed\n");
    }

    return Ret;
}

mt_s32 MT_MPI_DISP_SetColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS)
{
    MT_ERR_DISP(" The Func is not support.\n");
    return MT_ERR_DISP_NOT_EXIST;
}

mt_s32 MT_MPI_DISP_GetColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS)
{
    MT_ERR_DISP(" The Func is not support.\n");
    return MT_ERR_DISP_NOT_EXIST;
}

mt_s32 MT_MPI_DISP_Suspend(mt_void)
{
    mt_u32 u32Value = 0xaaaaaaaa;
    mt_s32 Ret = 0;


    /* check DISP initialization status */
    CHECK_DISP_INIT();


    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_SUSPEND, &u32Value);

    return Ret;
}

mt_s32 MT_MPI_DISP_Resume(mt_void)
{
    mt_u32 u32Value = 0xaaaaaaaa;
    mt_s32 Ret = 0;


    /* check DISP initialization status */
    CHECK_DISP_INIT();


    /* call DISP_SCREEN by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_RESUME, &u32Value);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetDacOutputEnable(MT_BOOL bEnable)
{
    mt_s32      Ret = 0;
    MT_BOOL     bDacEnable;
    CHECK_DISP_INIT();
    bDacEnable = bEnable;
    Ret = ioctl(g_DispDevFd, CMD_DISP_FORCESET_DAC_ENABLE, &bDacEnable);
    return Ret;
}
mt_s32 MT_MPI_DISP_SetLayerShow(MT_DRV_DISPLAY_E enDisp,MT_DRV_DISP_LAYER_ID_E elayer,MT_BOOL bEnable)
{
    mt_s32      Ret = 0;
    DISP_LAYERSHOW_S para;
    CHECK_DISP_INIT();
    para.enDisp = enDisp;
    para.elayer = elayer;
    para.b_on = bEnable;
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_LAYER_SHOW, &para);
    return Ret;
}

mt_s32 MT_MPI_DISP_SetPPMode(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_PPMODE_E enMode)
{
    mt_s32          Ret;
    DISP_PP_S   DispPPMode;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (enMode >= MT_DRV_DISP_PPMODE_BUTT)
    {
        MT_ERR_DISP("para enMode is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispPPMode.enDisp = enDisp;
    DispPPMode.enPPMode = enMode;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_PPMODE, &DispPPMode);

    return Ret;
}

mt_s32 MT_MPI_DISP_VidLayerShow(MT_BOOL bEnable)
{
    mt_s32          Ret;
    MT_BOOL     bLayerEnable;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bLayerEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_LAYER_ENABLE, &bLayerEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetVidLayerEnable(MT_BOOL *pbEnable)
{
    mt_s32      Ret = 0;
    MT_BOOL     bEnable;

    /* pointer validity check */
    if (!pbEnable)
    {
        MT_ERR_DISP("par pbEnable or phCast is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_LAYER_ENABLE, &bEnable);
	if (!Ret)
	{
		*pbEnable = bEnable;
	}

    return Ret;
}

mt_s32 MT_MPI_DISP_GetVideoSize(mt_u32 *pWidth, mt_u32 *pHeight)
{

    mt_s32          Ret;
	DISP_VIDEO_SIZE_S videoSize;

    CHECK_DISP_INIT();

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_VIDEO_SIZE, &videoSize);
	*pWidth = videoSize.u32Width;
	*pHeight = videoSize.u32Height;
    return Ret;
}

mt_s32 MT_MPI_DISP_SetAlpha(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Alpha)
{
    mt_s32          Ret;
    DISP_ALPHA_S   DispAlpha;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispAlpha.enDisp = enDisp;
    DispAlpha.u32Alpha= u32Alpha;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_LAYER_ALPHA, &DispAlpha);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetAlpha(MT_DRV_DISPLAY_E enDisp, mt_u32 *u32Alpha)
{
    mt_s32          Ret;
    DISP_ALPHA_S   DispAlpha;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispAlpha.enDisp = enDisp;

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_LAYER_ALPHA, &DispAlpha);

	if (!Ret)
	{
		*u32Alpha = DispAlpha.u32Alpha;
	}

    return Ret;
}

mt_s32 MT_MPI_DISP_SetCscEnable(MT_BOOL bEnable)
{
    mt_s32          Ret;
    MT_BOOL     bCscEnable;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bCscEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_CSC_ENABLE, &bCscEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetDenoiseEnable(MT_BOOL bEnable)
{
    mt_s32          Ret;
    MT_BOOL     bDenoiseEnable;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bDenoiseEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_DENOISE_ENABLE, &bDenoiseEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetAfdEnable(MT_BOOL bEnable)
{
    mt_s32          Ret;
    MT_BOOL     bAfdEnable;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bAfdEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_AFD_ENABLE, &bAfdEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetHdVideoEnable(MT_BOOL bEnable)
{
    mt_s32          Ret;
    MT_BOOL     bHdVideoEnable;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bHdVideoEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_HDVIDEO_ENABLE, &bHdVideoEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetSdVideoEnable(MT_BOOL bEnable)
{
    mt_s32          Ret;
    MT_BOOL     bSdVideoEnable;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bSdVideoEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_SDVIDEO_ENABLE, &bSdVideoEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetVdacOnOff(dac_index_t eDacId, MT_BOOL bEnable)
{
    mt_s32      Ret = 0;
    DISP_VDACOUTPUT_S para;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    para.dac_id = eDacId;
    para.b_enable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_VDAC_ONOFF, &para);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetTrickMode(DISP_TRICK_MODE_E trickmode)
{
    mt_s32      Ret = 0;

    CHECK_DISP_INIT();
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_TRICK_MODE, &trickmode);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetUnblankMode(MT_DRV_DISP_UNBLANK_MODE_E unblank_mode)
{
    mt_s32      Ret = 0;

    CHECK_DISP_INIT();
    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_UNBLANK_MODE, &unblank_mode);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetDiOnOff(MT_BOOL bEnable)
{
    mt_s32          Ret;
    MT_BOOL     bDiEnable;

    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bDiEnable = bEnable;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_DI_ENABLE, &bDiEnable);

    return Ret;
}

mt_s32 MT_MPI_DISP_ForceShowDS(mt_bool bStillLayer, MT_BOOL bDsEnable)
{
    mt_s32          Ret;
    DISP_SHOW_DS_S   DispShowDsPicPara;
    
    if (((MT_TRUE != bStillLayer) && (MT_FALSE != bStillLayer))
        || ((MT_TRUE != bDsEnable) && (MT_FALSE != bDsEnable))) {
    	MT_ERR_DISP("para bEnable is invalid.\n");
    	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    DispShowDsPicPara.bStillLayer = bStillLayer ? 1 : 0;
    DispShowDsPicPara.bShowDsPic = bDsEnable ? 1 : 0;

    Ret = ioctl(g_DispDevFd, CMD_FORCE_SHOW_DS_PIC, &DispShowDsPicPara);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetTvCapability(MT_DRV_DISP_HDMI_MODE_E enTvCap)
{
    mt_s32          Ret;

    if (enTvCap >= MT_DRV_DISP_HDMI_MODE_BUTT)
    {
        MT_ERR_DISP("para enMode is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_TVCAP, &enTvCap);

    return Ret;
}

mt_s32 MT_MPI_DISP_Set_Sl_Hdr(MT_DRV_DISPLAY_E enDisp, mt_u32 transparent_mode, mt_u32 display_Brightness, mt_u32 tuning_level, mt_u32 display_OETF)
{
    mt_s32          Ret;
    DISP_SL_HDR_PARA_S   DispSlHdrPara;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();

    DispSlHdrPara.enDisp = enDisp;
    DispSlHdrPara.transparent_mode = transparent_mode;
    DispSlHdrPara.u32Display_Brightness = display_Brightness;
    DispSlHdrPara.u32Tuning_level = tuning_level;
    DispSlHdrPara.u32Display_OETF = display_OETF;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_SL_HDR_PARA, &DispSlHdrPara);

    return Ret;
}

mt_s32 MT_MPI_DISP_GetSlHdrVersion(mt_u32 *pVer)
{
    mt_s32      Ret = 0;
    mt_u32      Ver;

    /* pointer validity check */
    if (!pVer)
    {
        MT_ERR_DISP("par pVer is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    Ret = ioctl(g_DispDevFd, CMD_DISP_GET_SlHDR_VER, &Ver);
	if (!Ret)
	{
		*pVer = Ver;
	}
    else
    {
        *pVer = 0;
    }

    return Ret;
}


mt_s32 MT_MPI_DISP_ResetHardware(MT_BOOL bHighSpeed)
{
    mt_s32          Ret;
    MT_BOOL     bHighSpeedClock;

    if ((MT_TRUE != bHighSpeed) && (MT_FALSE != bHighSpeed)) {
	MT_ERR_DISP("para bEnable is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    bHighSpeedClock = bHighSpeed;

    Ret = ioctl(g_DispDevFd, CMD_DISP_RESET_HARDWARE, &bHighSpeedClock);

    return Ret;
}

mt_s32 MT_MPI_DISP_SetSdScalerEnable(mt_u32 state)
{
    mt_s32          Ret;
    mt_u32     uState;

    if ((0 != state) && (1 != state) && (2 != state)) {
	MT_ERR_DISP("para bState is invalid.\n");
	return MT_ERR_DISP_INVALID_PARA;
    }

    CHECK_DISP_INIT();
    uState = state;

    Ret = ioctl(g_DispDevFd, CMD_DISP_SET_SD_SCALER_ENABLE, &uState);

    return Ret;
}

mt_s32 MT_MPI_DISP_DumpScaler2OSD(MT_DRV_DISP_DUMP_SCALER_PARA_S *pstParam)
{
    mt_s32      Ret = 0;

    /* pointer validity check */
    if (!pstParam)
    {
        MT_ERR_DISP("para pstParam is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check DISP initialization status */
    CHECK_DISP_INIT();

    /* check validity of pstCgmsCgf->bEnable */
    if (  (pstParam->b_enable != MT_TRUE)
        &&(pstParam->b_enable != MT_FALSE)
       )
    {
        MT_ERR_DISP("pstParam->b_enable: %d is invalid.\n", pstParam->b_enable);
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* check validity of pstParam->source */
    if (pstParam->source >= MT_DRV_DISP_DSCALER_IN__BUTT)
    {
        MT_ERR_DISP("para pstParam->source: %d is invalid.\n", pstParam->source);
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* check validity of pstParam->dst_layer */
    if (pstParam->dst_layer != DISP_LAYER_ID_SUBTITL
        && pstParam->dst_layer != DISP_LAYER_ID_OSD0
        && pstParam->dst_layer != DISP_LAYER_ID_OSD1)

    {
        MT_ERR_DISP("para pstParam->dst_layer: %d  is invalid.\n", pstParam->dst_layer);
        return MT_ERR_DISP_INVALID_PARA;
    }

    /* call SET_CGMS by kernel */
    Ret = ioctl(g_DispDevFd, CMD_DISP_DUMP_SCALER_TO_OSD, pstParam);

    return Ret;


}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

