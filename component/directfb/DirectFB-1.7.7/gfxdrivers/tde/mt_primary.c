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

    memset(&Alpha, 0, sizeof(Alpha));
    Alpha.bAlphaEnable  = MT_TRUE;
    Alpha.bAlphaChannel = MT_TRUE;
    Alpha.u8GlobalAlpha = opacity;
    Alpha.u8Alpha0 = 0;
    Alpha.u8Alpha1 = 255;
    Alpha.bRegionAlphaEnable = MT_FALSE;
    Alpha.u8RegionAlpha = 255;

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

    memset(&struColorKey, 0, sizeof(struColorKey));
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
        if(config->options & DLOP_SRC_COLORKEY)
        {
            ret = dfb_fbdev_set_colorkey(&(config->src_key), MT_TRUE);
            if(ret != DFB_OK)
            {
                D_PERROR( "DirectFB/FBDev: Could not set layer SRCKEY!, TRUE\n" );
            }
        }
        else
        {
            ret = dfb_fbdev_set_colorkey(&(config->src_key), MT_FALSE);
            if(ret != DFB_OK)
            {
                D_PERROR( "DirectFB/FBDev: Could not set layer SRCKEY, FALSE!\n" );
            }
        }
    }

    if (updated & CLRCF_DSTKEY)
    {
        if(config->options & DLOP_DST_COLORKEY)
        {
            ret = dfb_fbdev_set_colorkey(&(config->dst_key), MT_TRUE);
            if(ret != DFB_OK)
            {
                D_PERROR( "DirectFB/FBDev: Could not set layer DSTKEY!, TRUE\n" );
            }
        }
        else
        {
            ret = dfb_fbdev_set_colorkey(&(config->dst_key), MT_FALSE);
            if(ret != DFB_OK)
            {
                D_PERROR( "DirectFB/FBDev: Could not set layer DSTKEY! , FALSE\n" );
            }
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

static void
mt_dfb_var_to_mode( const struct fb_var_screeninfo *var,
                       VideoMode                      *mode,
                       DFBSurfacePixelFormat          *format)
{
     DFBSurfacePixelFormat      pixelformat = DSPF_ARGB;
	 if (!var)
		 return;

	 if (mode) {
	     mode->xres          = var->xres;
	     mode->yres          = var->yres;
	     mode->bpp           = var->bits_per_pixel;
	     mode->hsync_len     = var->hsync_len;
	     mode->vsync_len     = var->vsync_len;
	     mode->left_margin   = var->left_margin;
	     mode->right_margin  = var->right_margin;
	     mode->upper_margin  = var->upper_margin;
	     mode->lower_margin  = var->lower_margin;
	     mode->pixclock      = var->pixclock;
	     mode->hsync_high    = (var->sync & FB_SYNC_HOR_HIGH_ACT) ? 1 : 0;
	     mode->vsync_high    = (var->sync & FB_SYNC_VERT_HIGH_ACT) ? 1 : 0;
	     mode->csync_high    = (var->sync & FB_SYNC_COMP_HIGH_ACT) ? 1 : 0;
	     mode->sync_on_green = (var->sync & FB_SYNC_ON_GREEN) ? 1 : 0;
	     mode->external_sync = (var->sync & FB_SYNC_EXT) ? 1 : 0;
	     mode->broadcast     = (var->sync & FB_SYNC_BROADCAST) ? 1 : 0;
	     mode->laced         = (var->vmode & FB_VMODE_INTERLACED) ? 1 : 0;
	     mode->doubled       = (var->vmode & FB_VMODE_DOUBLE) ? 1 : 0;
	 }
	 
	 if (format) {
	 /* Get pixel format */
		if ((var->transp.length == 0) &&
			(var->red.length == 5) &&
			(var->green.length == 5) &&
			(var->blue.length == 5)) {
			pixelformat = DSPF_RGB555;
		} else if ((var->transp.length == 1) &&
			(var->red.length == 5) &&
			(var->green.length == 5) &&
			(var->blue.length == 5)) {
			pixelformat = DSPF_ARGB1555;
		} else if ((var->transp.length == 0) &&
			(var->red.length == 5) &&
			(var->green.length == 6) &&
			(var->blue.length == 5)) {
			pixelformat = DSPF_RGB16;
		} else if ((var->transp.length == 0) &&
			(var->red.length == 4) &&
			(var->green.length == 4) &&
			(var->blue.length == 4)) {
			pixelformat = DSPF_RGB444;
		} else if ((var->transp.length == 4) &&
			(var->red.length == 4) &&
			(var->green.length == 4) &&
			(var->blue.length == 4)) {
			pixelformat = DSPF_ARGB4444;
		} else if ((var->transp.length == 0) &&
			(var->red.length == 8) &&
			(var->green.length == 8) &&
			(var->blue.length == 8)) {
			pixelformat = DSPF_RGB32;
		} else if ((var->transp.length == 8) &&
			(var->red.length == 8) &&
			(var->green.length == 8) &&
			(var->blue.length == 8)) {
			pixelformat = DSPF_ARGB;
		} else if ((var->transp.length == 0) &&
			(var->red.length == 8) &&
			(var->green.length == 0) &&
			(var->blue.length == 0)) {
			/* for PDK1.17, the default fb2 format in driver is clut8.
			 * And the bits for ARGB arg: 0 8 0 0.
			 */
			pixelformat = DSPF_LUT8;
		} 

		*format = pixelformat;
	}
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
     struct fb_var_screeninfo var;
     FBDev *dfb_fbdev    = dfb_system_data();
     DFBSurfacePixelFormat  format;

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
	ret = ioctl(  dfb_fbdev->fd, FBIOGET_VSCREENINFO, &var);

	  mt_dfb_var_to_mode(&var, NULL, &format);

     if (dfb_config->mode.format != DSPF_UNKNOWN)
          config->pixelformat = dfb_config->mode.format;
     else if (dfb_config->mode.depth > 0)
          config->pixelformat = dfb_pixelformat_for_depth( dfb_config->mode.depth );
     else
          config->pixelformat = DSPF_ARGB1555;

	      /* fill out the default configuration */
     config->flags       = DLCONF_WIDTH       | DLCONF_HEIGHT |
                           DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_OPTIONS;
     config->width       = var.xres_virtual;
     config->height      = var.yres_virtual;
     config->pixelformat = format;
     config->buffermode  = DLBM_FRONTONLY;
     config->options     = DLOP_ALPHACHANNEL;

  
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
/*
    if(first)
	{
		first = false;
		return DFB_OK;
	}*/
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
mtPrimaryTestRegion( CoreLayer                  *layer,
               void                       *driver_data,
               void                       *layer_data,
               CoreLayerRegionConfig      *config,
               CoreLayerRegionConfigFlags *failed )
{
     CoreLayerRegionConfigFlags fail = 0;

     D_DEBUG("%s() fmt=0x%x region (%d, %d, %d x %d, %d %d)\n",
			 __FUNCTION__, config->format,
			 config->dest.x,  config->dest.y,config->dest.w,  config->dest.h, 
			 config->width, config->height);
	
     if (config->width  < 8 || config->width  > 1920)
          fail |= CLRCF_WIDTH;

     if (config->height < 8 || config->height > 1080)
          fail |= CLRCF_HEIGHT;

     if (config->dest.x < 0 || config->dest.y < 0)
          fail |= CLRCF_DEST;

     if (config->dest.x + config->dest.w > 1920)
          fail |= CLRCF_DEST;

     if (failed)
          *failed = fail;
 
	//D_ERROR("%s() fail = 0x%x\n",	 __FUNCTION__, fail);
	
	 if (fail) {
	     D_DEBUG("%s() fmt=0x%x region (%d, %d, %d x %d, %d %d)\n",
			 __FUNCTION__, config->format,
			 config->dest.x,  config->dest.y,config->dest.w,  config->dest.h, 
			 config->width, config->height);	
          D_ERROR("%s() fail = %x\n", __FUNCTION__, fail );
          return DFB_UNSUPPORTED;
	 }

	 D_DEBUG("%s() OK\n", __FUNCTION__);
	
     return DFB_OK;
}

static DFBResult
mtprimaryFlipRegion( CoreLayer             *layer,
                   void                  *driver_data,
                   void                  *layer_data,
                   void                  *region_data,
                   CoreSurface           *surface,
                   DFBSurfaceFlipFlags    flags,
                   const DFBRegion       *left_update,
                   CoreSurfaceBufferLock *left_lock,
                   const DFBRegion       *right_update,
                   CoreSurfaceBufferLock *right_lock )
{
     DFBResult ret;
     CoreLayerRegionConfig *config = NULL;
     FBDev *dfb_fbdev = dfb_system_data();
     FBDevShared  *shared = dfb_fbdev->shared;

	 config = &shared->config;

     if (((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAITFORSYNC) &&
         !dfb_config->pollvsync_after)
          mtPrimaryWaitVSync(NULL, NULL, NULL);

     ret = mt_dfb_fbdev_pan( config->source.x,
                          left_lock->offset / left_lock->pitch + config->source.y,
                          (flags & DSFLIP_WAITFORSYNC) == DSFLIP_ONSYNC );
     if (ret)
          return ret;

     if ((flags & DSFLIP_WAIT) &&
         (dfb_config->pollvsync_after || !(flags & DSFLIP_ONSYNC)))
          mtPrimaryWaitVSync(NULL, NULL, NULL);

     dfb_surface_flip( surface, false );

     return DFB_OK;
}



static DFBResult
mtPrimaryRemoveRegion( CoreLayer *layer,
                 void      *driver_data,
                 void      *layer_data,
                 void      *region_data )
{

     return DFB_OK;
}


static DFBResult
mtPrimaryInitScreen( CoreScreen           *screen,
                   CoreGraphicsDevice   *device,
                   void                 *driver_data,
                   void                 *screen_data,
                   DFBScreenDescription *description )
{
     D_DEBUG("%s()\n", __FUNCTION__ );

     /* Set the screen capabilities. */
     description->caps = DSCCAPS_VSYNC;

     /* Set the screen name. */
     snprintf( description->name, DFB_SCREEN_DESC_NAME_LENGTH, "Montage Screen" );

     return DFB_OK;
}

static DFBResult
mtPrimaryGetScreenSize( CoreScreen *screen,
                      void       *driver_data,
                      void       *screen_data,
                      int        *ret_width,
                      int        *ret_height )
{
       struct fb_var_screeninfo *var;
     FBDev *dfb_fbdev = dfb_system_data();
     FBDevShared  *shared = dfb_fbdev->shared;
     var = &shared->current_var;     

      D_DEBUG("%s() -> (%d x %d)\n", __FUNCTION__, var->xres_virtual, var->yres_virtual );

     *ret_width  = var->xres_virtual;
     *ret_height = var->yres_virtual;

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
     CoreSurfaceBuffer   *buffer;
     CoreLayerRegionConfig *config = NULL;

     buffer = left_lock->buffer;
     D_ASSERT( buffer != NULL );
    // mt_dfb_fbdev_pan( var->xoffset,left_lock->offset, 0);    

     return DFB_OK;
}

ScreenFuncs mtPrimaryScreenFuncs = {
    .InitScreen    = mtPrimaryInitScreen,
     .WaitVSync         = mtPrimaryWaitVSync,   
      .GetScreenSize = mtPrimaryGetScreenSize,
};

DisplayLayerFuncs mtPrimaryLayerFuncs = {     
     .SetRegion         = mtPrimarySetRegion,
     .InitLayer         = mtPrimaryInitLayer, 
     .UpdateRegion      = mtPrimaryUpdateRegion,
     .FlipRegion  = mtprimaryFlipRegion,
     .TestRegion    = mtPrimaryTestRegion,
     .RemoveRegion  = mtPrimaryRemoveRegion,
};

