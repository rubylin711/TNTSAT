/******************************************************************************

  Copyright (C), 2001-2013, Huawei Tech. Co., Ltd.

 ******************************************************************************
 File Name     : mt_primary.c
Version       : Initial Draft
Author        : p00203646
Created       : 2013/08/12
Last Modified :
Description   : Mt Primary Layer Funcs
Function List :
History       :
1.Date        : 2013/08/12
Author      : p00203646
Modification: Created file

 ******************************************************************************/

#include <config.h>

#include <string.h>
#include <sys/ioctl.h>

#include <dfb_types.h>
#include <directfb.h>
#include <directfb_util.h>
#include <direct/debug.h>

#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/screens.h>
#include <core/core.h>

#include <fbdev/fb.h>
#include <fbdev/fbdev.h>

#include "mtfb.h"

/* primary layer hooks */

DisplayLayerFuncs  mtOldPrimaryFuncs;
void              *mtOldPrimaryDriverData;


static inline
void waitretrace (void)
{
#if defined(HAVE_INB_OUTB_IOPL)
     if (iopl(3))
          return;

     if (!(inb (0x3cc) & 1)) {
          while ((inb (0x3ba) & 0x8))
               ;

          while (!(inb (0x3ba) & 0x8))
               ;
     }
     else {
          while ((inb (0x3da) & 0x8))
               ;

          while (!(inb (0x3da) & 0x8))
               ;
     }
#endif
}

static DFBResult
mtPrimaryWaitVSync( CoreScreen *screen,
                  void       *driver_data,
                  void       *screen_data )
{
     if (dfb_config->pollvsync_none)
          return DFB_OK;

     FBDev *dfb_fbdev = dfb_system_data();

     /*wait for VBLANK,this interface is support by hisilicon fb*/
     if (ioctl( dfb_fbdev->fd, FBIOGET_VBLANK_MTFB))
          waitretrace();

     return DFB_OK;
}

static DFBResult 
dfb_fbdev_set_opacity(u8 opacity)
{
    MTFB_ALPHA_S Alpha;

    Alpha.bAlphaEnable  = MT_TRUE;
    Alpha.bAlphaChannel = MT_TRUE;
    Alpha.u8GlobalAlpha = opacity;
    Alpha.u8Alpha0 = 0;
    Alpha.u8Alpha1 = 255;

    FBDev *dfb_fbdev = dfb_system_data();
    if (ioctl(dfb_fbdev->fd, FBIOPUT_ALPHA_MTFB,  &Alpha) <0 )
    {
         D_PERROR( "DirectFB/FBDev: Could not set the alpha!\n" );
         return errno2result(errno);
    }

    return DFB_OK;
}


static DFBResult 
dfb_fbdev_set_colorkey(DFBColorKey *pKey, MT_U32 bEnable)
{
    MTFB_COLORKEY_S struColorKey;

    struColorKey.bKeyEnable  = bEnable;
    struColorKey.u32Key = (pKey->r << 16) | (pKey->g << 8) | (pKey->b);

    FBDev *dfb_fbdev = dfb_system_data();
    if (ioctl(dfb_fbdev->fd, FBIOPUT_COLORKEY_MTFB,  &struColorKey) <0 )
    {
          D_PERROR( "DirectFB/FBDev: Could not set the colorkey!\n" );
          perror("colorkey:n");
          return errno2result(errno);
    }

     return DFB_OK;
}

static DFBResult 
dfb_fbdev_setscreensize(DFBRectangle *pDstRectangle)
{
    //attention screen size is not support by DFB
    MTFB_POINT_S struPos;
    
    struPos.s32XPos = (MT_S32)pDstRectangle->x;
    struPos.s32YPos = (MT_S32)pDstRectangle->y;

    FBDev *dfb_fbdev = dfb_system_data();
    if (ioctl(dfb_fbdev->fd, FBIOPUT_SCREEN_ORIGIN_MTFB,  &struPos) <0 )
    {
        D_PERROR( "DirectFB/FBDev: Could not set layer pos!\n" );
        return errno2result(errno);
    }

#ifdef SUPPORT_SET_SCREEN  /*support set screen size*/
    MTFB_SIZE_S  screensize;
    screensize.u32Width  = pDstRectangle->w;
    screensize.u32Height = pDstRectangle->h;    

    if (ioctl(dfb_fbdev->fd, FBIOPUT_SCREENSIZE,  &screensize) <0 )
    {
        D_PERROR( "DirectFB/FBDev: Could not set layer size!\n" );
        return errno2result(errno);
    }
#else /*unsupport set screen size*/
    if((pDstRectangle->w != dfb_fbdev->shared->modes->xres)||
        (pDstRectangle->h != dfb_fbdev->shared->modes->yres))
    {
         D_PERROR( "DirectFB/FBDev: Could not set screen size!\n" );
         return errno2result(errno);
    }
#endif

    return DFB_OK;
}


static DFBResult
mtPrimarySetRegion( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  void                       *region_data,
                  CoreLayerRegionConfig      *config,
                  CoreLayerRegionConfigFlags  updated,
                  CoreSurface                *surface,
                  CorePalette                *palette,
                  CoreSurfaceBufferLock      *left_lock,
                  CoreSurfaceBufferLock      *right_lock )
{
    DFBResult     ret;

     /* call the original function */
    ret = mtOldPrimaryFuncs.SetRegion( layer, mtOldPrimaryDriverData,
                                        layer_data, region_data,
                                        config, updated, surface,
                                        palette, left_lock, right_lock);
    if (ret)
        return ret;

    if (updated & CLRCF_OPACITY)
	{
		ret = dfb_fbdev_set_opacity(config->opacity);
		if(ret != DFB_OK)
		{
			D_PERROR( "DirectFB/FBDev: Could not set layer OPACITY!\n" );
		}
	}

	if (updated & CLRCF_SRCKEY)
	{
		ret = dfb_fbdev_set_colorkey(&(config->src_key), MT_TRUE);
		if(ret != DFB_OK)
		{
			D_PERROR( "DirectFB/FBDev: Could not set layer SRCKEY!\n" );
		}
	}

	if (updated & CLRCF_DSTKEY)
	{
		ret = dfb_fbdev_set_colorkey(&(config->dst_key), MT_TRUE);
		if(ret != DFB_OK)
		{
			D_PERROR( "DirectFB/FBDev: Could not set layer DSTKEY!\n" );
		}
	}

	if (updated & CLRCF_DEST)
	{
		ret = dfb_fbdev_setscreensize(&(config->dest));
		if(ret != DFB_OK)
		{
			D_PERROR( "DirectFB/FBDev: Could not set layer screensize!\n" );
		}
	}
    
    return DFB_OK;
}


static DFBResult
mtPrimaryInitLayer( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  DFBDisplayLayerDescription *description,
                  DFBDisplayLayerConfig      *config,
                  DFBColorAdjustment         *adjustment )
{
     DFBResult ret;

     /* call the original initialization function first */
     ret = mtOldPrimaryFuncs.InitLayer( layer,
                                        mtOldPrimaryDriverData,
                                        layer_data, description,
                                        config, adjustment );
     if (ret)
          return ret;

     snprintf( description->name,
               DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "Hisi FBDev Primary Layer" );
     
     /* add some capabilities */
     description->caps |= DLCAPS_OPACITY|DLCAPS_SCREEN_LOCATION|
                         DLCAPS_SRC_COLORKEY|DLCAPS_DST_COLORKEY|
                         DLCAPS_WINDOWS|DLCAPS_SCREEN_POSITION|
                         DLCAPS_SCREEN_SIZE;

     if (dfb_config->mode.format != DSPF_UNKNOWN)
          config->pixelformat = dfb_config->mode.format;
     else if (dfb_config->mode.depth > 0)
          config->pixelformat = dfb_pixelformat_for_depth( dfb_config->mode.depth );
     else
          config->pixelformat = DSPF_ARGB1555;
  
     return DFB_OK;
}

static DFBResult
mt_dfb_fbdev_pan( int xoffset, int yoffset, bool onsync )
{
     int result;
     static bool first = true; 
     struct fb_var_screeninfo *var;
     FBDev *dfb_fbdev    = dfb_system_data();
     FBDevShared *shared = dfb_fbdev->shared;

     if (!shared->fix.xpanstep && !shared->fix.ypanstep && !shared->fix.ywrapstep)
          return DFB_OK;

     var = &shared->current_var;

     if (var->xres_virtual < xoffset + var->xres) {
          D_ERROR( "DirectFB/FBDev: xres %d, vxres %d, xoffset %d\n",
                    var->xres, var->xres_virtual, xoffset );
          D_BUG( "panning buffer out of range" );
          return DFB_BUG;
     }

     if (var->yres_virtual < yoffset + var->yres) {
          D_ERROR( "DirectFB/FBDev: yres %d, vyres %d, offset %d\n",
                    var->yres, var->yres_virtual, yoffset );
          D_BUG( "panning buffer out of range" );
          return DFB_BUG;
     }

     if (shared->fix.xpanstep)
          var->xoffset = xoffset - (xoffset % shared->fix.xpanstep);
     else
          var->xoffset = 0;

     if (shared->fix.ywrapstep) {
          var->yoffset = yoffset - (yoffset % shared->fix.ywrapstep);
          var->vmode |= FB_VMODE_YWRAP;
     }
     else if (shared->fix.ypanstep) {
          var->yoffset = yoffset - (yoffset % shared->fix.ypanstep);
          var->vmode &= ~FB_VMODE_YWRAP;
     }
     else {
          var->yoffset = 0;
     }

     var->activate = onsync ? FB_ACTIVATE_VBL : FB_ACTIVATE_NOW;

    if(first)
	{
		first = false;
		return DFB_OK;
	}
     if (ioctl( dfb_fbdev->fd, FBIOPAN_DISPLAY, var ) < 0) {
          result = errno;

          D_PERROR( "DirectFB/FBDev: Panning display failed (x=%u y=%u ywrap=%d vbl=%d)!\n",
                    var->xoffset, var->yoffset,
                    (var->vmode & FB_VMODE_YWRAP) ? 1 : 0,
                    (var->activate & FB_ACTIVATE_VBL) ? 1 : 0);

          return errno2result(result);
     }

     return DFB_OK;
}
     
static DFBResult                                 
mtPrimaryUpdateRegion( CoreLayer             *layer,
                     void                  *driver_data,
                     void                  *layer_data,
                     void                  *region_data,
                     CoreSurface           *surface,
                     const DFBRegion       *left_update,
                     CoreSurfaceBufferLock *left_lock,
                     const DFBRegion       *right_update,
                     CoreSurfaceBufferLock *right_lock )
{    
     struct fb_var_screeninfo *var;
     FBDev *dfb_fbdev = dfb_system_data();
     FBDevShared  *shared = dfb_fbdev->shared;
     var = &shared->current_var;     
     mt_dfb_fbdev_pan( var->xoffset, var->yoffset, 0);    

     return DFB_OK;
}

ScreenFuncs mtPrimaryScreenFuncs = {
     .WaitVSync         = mtPrimaryWaitVSync,     
};

DisplayLayerFuncs mtPrimaryLayerFuncs = {     
     .SetRegion         = mtPrimarySetRegion,
     .InitLayer         = mtPrimaryInitLayer, 
     .UpdateRegion      = mtPrimaryUpdateRegion,
};

