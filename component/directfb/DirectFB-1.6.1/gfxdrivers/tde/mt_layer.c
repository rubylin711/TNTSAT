#include <config.h>

#include <asm/types.h>

#include <stdio.h>
#include <sys/mman.h>

#include <directfb.h>

//#include <core/screens.h>

#include <direct/debug.h>
#include <direct/messages.h>

#include <sys/ioctl.h>
#include <fbdev/fbdev.h>
#include <core/layers.h>
#include "mt_layer.h"

//#include "mt_unf_disp.h"
#include "mt_mpi_disp.h"

#include "mt_unf_vo.h"
#include "mt_mpi_win.h"

#include "mt_mpi_hdmi.h"
#if 0
 /*
  * Return size of layer data (shared memory).
  */
int mt_layer_data_size ( void )
{
	return 0;
}

 /*
  * Return size of region data (shared memory).
  */
int mt_region_data_size ( void )
{
	return 0;
}


static int find_window_from_layer(CoreLayer *layer, MT_DRV_WIN_ATTR_S *attr)
{
	int i;
	int       num = dfb_layer_num();
	MT_HANDLE win;

	win = (MT_HANDLE)((MT_ID_VO << 16) | (MT_UNF_DISPLAY1 << 8)  |(((MT_U32)layer->shared->layer_id - VIDEO_LAY_ID)&0xff));

	if (MT_SUCCESS != MT_MPI_WIN_GetAttr(win, attr))
		return -1;
	return MT_SUCCESS;
}
#endif

#define VIDEO_LAY_ID (1UL)

 /*
  * Called once by the master to initialize layer data and reset hardware.
  * Return layer description, default configuration and color adjustment.
  */
DFBResult mt_init_layer ( CoreLayer                  *layer,
                              void                       *driver_data,
                              void                       *layer_data,
                              DFBDisplayLayerDescription *description,
                              DFBDisplayLayerConfig      *config,
                              DFBColorAdjustment         *adjustment )
{
	//MT_HANDLE win,cwin;
	//MT_UNF_WINDOW_ATTR_S attr;

	if (MT_SUCCESS != MT_MPI_WIN_Init()) {
		D_ERROR("%s->%d, fail!\n",__func__,__LINE__);
		return DFB_FAILURE;
	}

     /* set capabilities and type */
     description->caps = DLCAPS_SCREEN_LOCATION | DLCAPS_SURFACE;
     description->type = DLTF_VIDEO | DLTF_STILL_PICTURE;

     /* set name */
     snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "MT Video" );

     /* fill out the default configuration */
     config->flags  = DLCONF_WIDTH | DLCONF_HEIGHT | DLCONF_BUFFERMODE; 
	 config->buffermode = DLBM_FRONTONLY;
     //config->options     = DLOP_NONE;


	/* FIXME! Is set output rect right? */
     //config->width       = attr.stOutputRect.s32Width;
     //config->height      = attr.stOutputRect.s32Height;
	
	 config->width       = 1920;
     config->height      = 1080;
     return DFB_OK;

}
/*
* Check all parameters and return if this region is supported.
*/
DFBResult mt_test_region   ( CoreLayer                  *layer,
                         void                       *driver_data,
                         void                       *layer_data,
                         CoreLayerRegionConfig      *config,
                         CoreLayerRegionConfigFlags *failed )
{
	 /* FIXME! DO WHAT ? */
     //layer->shared->contexts.primary->primary.region->state &= ~CLRSF_FROZEN;
     if (failed)
		*failed = CLRCF_NONE;
	return DFB_OK;
}
 /*
  * Setup hardware, called once after AddRegion() or when parameters
  * have changed. Surface and palette are only set if updated or new.
  */
DFBResult mt_set_region( CoreLayer                  *layer,
                             void                       *driver_data,
                             void                       *layer_data,
                             void                       *region_data,
                             CoreLayerRegionConfig      *config,
                             CoreLayerRegionConfigFlags  updated,
                             CoreSurface                *surface,
                             CorePalette                *palette,
                             CoreSurfaceBufferLock      *lock )
{
	MT_DRV_WIN_ATTR_S attr;
	MT_HANDLE win;

	if (!layer || !layer->shared) {
		D_ERROR("%s->%d, !layer %d, !layer->shared:%d,Cannot get window handle!\n",__func__,__LINE__,!layer, !layer->shared);
		return DFB_FAILURE;
	}

	win = (MT_HANDLE)((MT_ID_VO << 16) | (MT_UNF_DISPLAY1 << 8)  |(((MT_U32)layer->shared->layer_id - VIDEO_LAY_ID)&0xff));

	printf("%s->%d, win hanlde:0x%x\n",__func__,__LINE__,win);

	if (MT_SUCCESS != MT_MPI_WIN_GetAttr(win,  &attr)) {
		D_ERROR("mt_set_region Fail to get window attr!\n");
		return DFB_FAILURE;
	}

	attr.stOutRect.s32X = config->dest.x;
	attr.stOutRect.s32Y = config->dest.y;
	attr.stOutRect.s32Width = config->dest.w;
	attr.stOutRect.s32Height = config->dest.h;

	if (MT_SUCCESS != MT_MPI_WIN_SetAttr(win, &attr)) {
		D_ERROR("mt_set_region Fail to set window attr!\n");
		return DFB_FAILURE;
	}

	return DFB_OK;
}

/*
* Return the z position of the layer.
*/
DFBResult mt_get_level ( CoreLayer              *layer,
                                 void                   *driver_data,
                                 void                   *layer_data,
                                 int                    *level )
{
	return DFB_UNSUPPORTED;
}

/*
* Move the layer below or on top of others (z position).
*/
DFBResult mt_set_level ( CoreLayer              *layer,
                                 void                   *driver_data,
                                 void                   *layer_data,
                                 int                     level )
{
	return DFB_UNSUPPORTED;
}

/*
* Adjust brightness, contrast, saturation etc.
*/
DFBResult mt_set_color_adjustment ( CoreLayer              *layer,
                                 void                   *driver_data,
                                 void                   *layer_data,
                                 DFBColorAdjustment     *adjustment )

{
	return DFB_UNSUPPORTED;
}

DisplayLayerFuncs mt_layer_funcs = {
//	.LayerDataSize = mt_layer_data_size,
//	.RegionDataSize = mt_region_data_size,
	
	.InitLayer = mt_init_layer,

	.TestRegion = mt_test_region,
	.SetRegion = mt_set_region,

    .GetLevel = mt_get_level,
    .SetLevel = mt_set_level,

    .SetColorAdjustment = mt_set_color_adjustment,
};
