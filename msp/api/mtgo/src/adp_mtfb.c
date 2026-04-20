/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/types.h>
#include "mt_drv_tde.h"

#include "mtfb.h"
#include "mtfb_drv_common.h"

#include "mt_module.h"
#include "drv_osd_ioctrl.h"
// use the osd drv to call the optm interface
#include "optm_mtfb.h"
#include "adp_mtfb.h"

#define OSD_CHECK_FD() \
    do {                                         \
        if (-1 == g_s32OsdFd)                    \
        {                                        \
            return 1;      \
        }                                        \
    } while (0)

static const char *g_pszOsdDevName = "/dev/mt_osd";

static mt_s32 g_s32OsdFd = -1;              /* tde device handle */

static mt_s32 g_s32OsdRef = 0;



#define MTFB_FUN_IN     printf("%s, LINE IN: %d\n", __FUNCTION__, __LINE__)
#define MTFB_FUN_OUT     printf("%s, LINE OUT: %d\n", __FUNCTION__, __LINE__)
#define MTFB_LOG    printf

static mt_s32 MTFB_OsdDrvOpen(mt_void)
{
//    MTFB_FUN_IN;
      
    if (-1 != g_s32OsdFd)
    {
        g_s32OsdRef++;
        return MT_SUCCESS;
    }

    g_s32OsdFd = open(g_pszOsdDevName, O_RDWR, 0);
    if (g_s32OsdFd < 0)
    {
        return MT_ERR_TDE_DEV_OPEN_FAILED;
    }
    g_s32OsdRef++;

//    MTFB_FUN_OUT;
      
    return MT_SUCCESS;
}
#if 0
static mt_void MTFB_OsdDrvClose(mt_void)
{
//   MTFB_FUN_IN;
   
   if (-1 == g_s32OsdFd)
    {
        return;
    }
    g_s32OsdRef--;
    
    if(g_s32OsdRef > 0)
    {
        return ;
    }
    else
    {
        g_s32OsdRef = 0;
    }
    
    close(g_s32OsdFd);

    g_s32OsdFd = -1;

//     MTFB_FUN_OUT;
     
    return;
}
#endif

mt_s32 MTFB_GfxInit(mt_void)
{
   mt_s32 ret = 0;

//   MTFB_FUN_IN;
   
   MTFB_OsdDrvOpen();
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_INIT, 0);

 //  MTFB_FUN_OUT;
    
   return ret;
}

mt_s32 MTFB_GfxDeInit(mt_void)
{
   mt_s32 ret = 0;

//   MTFB_FUN_IN;

//   MTFB_FUN_OUT;
   return ret;
}


mt_s32 MTFB_GfxOpenLayer(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnableOsdc)
{ 
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.EnableOsdc = bEnableOsdc;

//   MTFB_FUN_IN;
   
   OSD_CHECK_FD();

//   MTFB_LOG("PARAM: [%x]\n", enLayerId);
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_OPEN_LAYER, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}


mt_s32 MTFB_GfxCloseLayer(MTFB_LAYER_ID_E enLayerId)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;

//   MTFB_FUN_IN;
   
   OSD_CHECK_FD();

//   MTFB_LOG("PARAM: [%x]\n", enLayerId);
    
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_CLOSE_LAYER, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}


mt_s32 MTFB_GfxSetEnable(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.bEnable = bEnable;

//   MTFB_FUN_IN;
   
   OSD_CHECK_FD();

//   MTFB_LOG("PARAM: [%x] [%x]\n", enLayerId, bEnable);
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_SET_ENABLE, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}

mt_s32 MTFB_GfxMaskLayer(MTFB_LAYER_ID_E enLayerId, MT_BOOL bMask)
{
   mt_s32 ret = 0;

//   MTFB_FUN_IN;
//   MTFB_LOG("PARAM: [%x] [%x]\n", enLayerId, bMask);
   
//   MTFB_FUN_OUT;
   
   return ret;
}

mt_s32 MTFB_GfxSetLayerAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Addr)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.u32Addr = u32Addr;

//   MTFB_FUN_IN;
//   MTFB_LOG("PARAM: [%x] [%x]\n", enLayerId, u32Addr);
   
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_SET_LAYER_ADDR, &param);

//   MTFB_FUN_OUT;
  
   return ret;
}


mt_s32 MTFB_GfxSetLayerStride(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Stride)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.u32Stride = u32Stride;

//   MTFB_FUN_IN;
   
   OSD_CHECK_FD();

//   MTFB_LOG("PARAM: [%x] [%x]\n", enLayerId, u32Stride);
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_SET_LAYER_STRIDE, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}


mt_s32 MTFB_GfxSetLayerDataFmt(MTFB_LAYER_ID_E enLayerId, MTFB_COLOR_FMT_E enDataFmt)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.enDataFmt = enDataFmt;

//   MTFB_FUN_IN;

//   MTFB_LOG("PARAM: [%x] [%x]\n", enLayerId, enDataFmt);
   
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_SET_LAYER_DATA_FMT, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}


mt_s32 MTFB_GfxSetLayerAlpha(MTFB_LAYER_ID_E enLayerId, MTFB_ALPHA_S *pstAlpha)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.pstAlpha = pstAlpha;

//   MTFB_FUN_IN;

//   MTFB_LOG("PARAM: [%x] 0x[%x][%x] [%x][%x] [%x]\n", enLayerId, pstAlpha->bAlphaChannel, pstAlpha->bAlphaEnable, pstAlpha->u8Alpha0, pstAlpha->u8Alpha1, pstAlpha->u8GlobalAlpha);
   
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_SET_LAYER_ALPHA, &param);

//   MTFB_FUN_OUT;
  
   return ret;
}


mt_s32 MTFB_GfxSetLayerRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstRect)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.pstRect = (MTFB_RECT *)pstRect;

//   MTFB_FUN_IN;

//   MTFB_LOG("PARAM: [%d] [%d][%d][%d][%d]\n", enLayerId, pstRect->x, pstRect->y, pstRect->w, pstRect->h);
   
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_SET_LAYER_RECT, &param);


//   MTFB_FUN_OUT;
   
   return ret;
}


mt_s32 MTFB_GfxSetColorReg(MTFB_LAYER_ID_E u32LayerId, mt_u32 u32OffSet, mt_u32 u32Color, mt_s32 UpFlag)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = u32LayerId;
   param.u32OffSet = u32OffSet;
   param.u32Color = u32Color;
   param.UpFlag = UpFlag;

//   MTFB_FUN_IN;

//    MTFB_LOG("PARAM: [%x] 0x[%x][%x][%x]\n", u32LayerId, u32OffSet, u32Color, UpFlag);
    
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_UP_LAYER_REG, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}

mt_s32 MTFB_GfxSetLayKeyMask(MTFB_LAYER_ID_E enLayerId, const MTFB_COLORKEYEX_S *pstColorkey)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.pstColorKey = (MTFB_COLORKEYEX_S *)pstColorkey;

//   MTFB_FUN_IN;

//   MTFB_LOG("PARAM: [%d] [%d][0x%x]\n", enLayerId, pstColorkey->bKeyEnable, pstColorkey->u32Key);

   OSD_CHECK_FD();

   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_SET_LAYKEY_MASK, &param);

//   MTFB_FUN_OUT;
   
   return ret;

}

mt_s32 MTFB_GfxCmpDecmpProcess(MTFB_LAYER_ID_E enLayerId, mt_u32 u32RdAddr, 
                                                         mt_u32 pic_width, mt_u32 pic_height, mt_u32 u32Stride, mt_u32 u32WrAddr)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.u32Addr = u32WrAddr;
   param.u32Stride = u32Stride;
   param.pic_width = pic_width;
   param.pic_height = pic_height;
   param.u32RdAddr = u32RdAddr;

//   MTFB_FUN_IN;

//    MTFB_LOG("PARAM: [%x] 0x[%x][%x][%x][%x][%x]\n", enLayerId, u32RdAddr, pic_width, pic_height, u32Stride, u32WrAddr);
    
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_CMP_DECMP_PROCESS, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}

#ifdef CFG_MTGO_PROC_SUPPORT

mt_s32 MTFB_GfxProcSurfaceInfo(MTFB_LAYER_ID_E enLayerId, MTFB_PROC_SURFACE_S *p_info)
{
   mt_s32 ret = 0;
   OSD_IOC_PARAM_S param;
   param.LayerID = enLayerId;
   param.u32Addr = p_info->addr;
   param.u32Stride = p_info->stride;
   param.pic_width = p_info->width;
   param.pic_height = p_info->height;
   param.enDataFmt = p_info->fmt;
//   MTFB_FUN_IN;
    
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_PROC_SURFACE_INFO, &param);

//   MTFB_FUN_OUT;
   
   return ret;
}

#endif

mt_s32 MTFB_GfxWaitSync(mt_void)
{
   mt_s32 ret = 0;
   
   OSD_CHECK_FD();
   
   ret = ioctl(g_s32OsdFd, CMD_IOC_GFX_WAIT_SYNC, 0);

//   MTFB_FUN_OUT;
  
   return ret;
}


