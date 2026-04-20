/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <stdio.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/messages.h>

#include <core/gfxcard.h>
#include <core/system.h>

#include <misc/conf.h>


#include "tde_2d.h"
#include "tde_gfxdriver.h"
#include "mt_tde_api.h"
#include "mt_tde_type.h"
#include "mt_tde_errcode.h"
#include "mt_layer.h"
#include "mt_common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/graphics_driver.h>

#include <fbdev/fb.h>
#include <fbdev/fbdev.h>

D_DEBUG_DOMAIN( TDE, "TDE/Driver", "TDE graphics driver for montage" );

#include <core/graphics_driver.h>

DFB_GRAPHICS_DRIVER( tde )

#define TDE_FD "/dev/mt_tde"
//#define TDE_FD "/dev/tde"

/**********************************************************************************************************************/

/** 
function: probe which device is support,
param:
    if support return non 0 value, else return 0*/
static int
driver_probe( CoreGraphicsDevice *device )
{
    int fd;
	D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);

     /* if tde fd is find , then  suport tde, else return 0*/
    fd = open(TDE_FD, O_RDWR);
    if (fd < 0)
    {
        D_ERROR("unsupport TDE device\n");
        return 0;
    }
	D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);

    close (fd);
     
    return 1;
}

static void
driver_get_info( CoreGraphicsDevice *device,
                 GraphicsDriverInfo *info )
{
	D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);

     /* fill driver info structure */
     snprintf( info->name,
               DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
               "Montage tde Driver" );

     snprintf( info->vendor,
               DFB_GRAPHICS_DRIVER_INFO_VENDOR_LENGTH,
               "graphic group" );

     info->version.major = 0;
     info->version.minor = 5;

     info->driver_data_size = sizeof(TDEDriverData);
     info->device_data_size = sizeof(TDEDeviceData);
     return;
}

/** Initialize the driver.*/
static DFBResult
driver_init_driver( CoreGraphicsDevice  *device,
                    GraphicsDeviceFuncs *funcs,
                    void                *driver_data,
                    void                *device_data,
                    CoreDFB             *core )
{
     /* add tde init code*/
     int ret;
	 D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);

     D_DEBUG_AT( TDE, "%s()\n", __FUNCTION__ );

	 mt_sys_init();

     /* initialize function pointers */
     funcs->EngineSync    = TDEEngineSync;
     funcs->EngineReset   = TDEEngineReset;
     funcs->EmitCommands  = TDEEmitCommands;
     funcs->CheckState    = TDECheckState;
     funcs->SetState      = TDESetState;
     funcs->FillRectangle = TDEFillRectangle;
     funcs->DrawRectangle = TDEDrawRectangle;
     funcs->DrawLine      = TDEDrawLine;
     funcs->FillTriangle  = TDEFillTriangle;
     funcs->Blit          = TDEBlit;
     funcs->StretchBlit   = TDEStretchBlit;

     /* Choose accelerated font format */
     if (!dfb_config->software_only) {
          dfb_config->font_format  = DSPF_ARGB;
          dfb_config->font_premult = true;
     }
	 D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);

     dfb_screens_hook_primary( device, driver_data,
                            &mtPrimaryScreenFuncs,
                            NULL,
                            NULL);
	 D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);	 
     dfb_layers_hook_primary( device, driver_data, 
                            &mtPrimaryLayerFuncs, 
                            &mtOldPrimaryFuncs, 
                            &mtOldPrimaryDriverData );
	 D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);

     ret = MT_TDE2_Open();
     if (ret != MT_SUCCESS)
     {
        D_ERROR("tde init error\n");
        return DFB_INIT;
     }
	 D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);	 
     dfb_layers_register(dfb_screens_at(DSCID_PRIMARY), driver_data, &mt_layer_funcs);
	 D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);	 
     return DFB_OK;
}

/**Initialize hardware. All necessary hardware initialization should be processed here*/
static DFBResult
driver_init_device( CoreGraphicsDevice *device,
                    GraphicsDeviceInfo *device_info,
                    void               *driver_data,
                    void               *device_data )
{
	D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);
     mt_sys_deinit();

     /* fill device info */
     snprintf( device_info->vendor, DFB_GRAPHICS_DEVICE_INFO_VENDOR_LENGTH, "TDE Acceleration -" );
     snprintf( device_info->name,   DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,   "graphic group" );
	 D_INFO("\r\n %s, %d,~~~~~~~~~~~~", __FUNCTION__, __LINE__);


     /* device limitations */
     device_info->limits.surface_byteoffset_alignment = 8;
     device_info->limits.surface_bytepitch_alignment  = 8;

    /** attention : 
        directfb donot check the info below, before setting hard device 
        the accel info is supplied to  user interface */
     device_info->caps.flags    = CCF_CLIPPING | CCF_RENDEROPTS;
     device_info->caps.accel    = TDE_SUPPORTED_DRAWINGFUNCTIONS |
                                  TDE_SUPPORTED_BLITTINGFUNCTIONS;
     device_info->caps.drawing  = TDE_SUPPORTED_DRAWINGFLAGS;
     device_info->caps.blitting = TDE_SUPPORTED_BLITTINGFLAGS;


     return DFB_OK;
}

static void
driver_close_device( CoreGraphicsDevice *device,
                     void               *driver_data,
                     void               *device_data )
{
    return;
}

static void
driver_close_driver( CoreGraphicsDevice *device,
                     void               *driver_data )
{
    MT_TDE2_Close();
    return;
}


