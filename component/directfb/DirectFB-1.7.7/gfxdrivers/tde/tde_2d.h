/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __TDE_2D_H__
#define __TDE_2D_H__

/* add include here */


#ifdef __cplusplus
extern "C" {
#endif
/***************************** Macro Definition ******************************/

#define TDE_SUPPORTED_DRAWINGFLAGS      (DSDRAW_NOFX|DSDRAW_BLEND|DSDRAW_SRC_PREMULTIPLY|DSDRAW_XOR|DSDRAW_DST_COLORKEY)

#define TDE_SUPPORTED_DRAWINGFUNCTIONS  (DFXL_FILLRECTANGLE|DFXL_DRAWRECTANGLE)
/** DFXL_DRAWLINE could not be support now,  */

#define TDE_SUPPORTED_BLITTINGFLAGS    (DSBLIT_BLEND_ALPHACHANNEL | \
                                        DSBLIT_BLEND_COLORALPHA | \
                                        DSBLIT_SRC_COLORKEY| \
                                        DSBLIT_DST_COLORKEY| \
                                        DSBLIT_SRC_PREMULTIPLY| \
                                        DSBLIT_SRC_PREMULTCOLOR | \
                                        DSBLIT_SRC_MASK_ALPHA | \
                                        DSBLIT_SRC_MASK_COLOR | \
                                        DSBLIT_XOR | \
                                        DSBLIT_COLORIZE | \
                                        DSBLIT_ROTATE90 | \
                                        DSBLIT_ROTATE180 | \
                                        DSBLIT_ROTATE270 | \
                                        DSBLIT_FLIP_HORIZONTAL | \
                                        DSBLIT_FLIP_VERTICAL)
                                        
/*          
						DSBLIT_DST_PREMULTIPLY| \
						DSBLIT_ROTATE90 | \
                                        DSBLIT_ROTATE180| \
                                        DSBLIT_ROTATE270 | \                                        
                                        DSBLIT_SRC_PREMULTCOLOR)
                                                                                
        
*/

#define TDE_SUPPORTED_BLITTINGFUNCTIONS (DFXL_BLIT | \
                                         DFXL_STRETCHBLIT|DFXL_BLIT2)

/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/

DFBResult TDEEngineSync   ( void                *drv,
                           void                *dev );

void      TDEEngineReset  ( void                *drv,
                           void                *dev );

void      TDEEmitCommands ( void                *drv,
                           void                *dev );

void      TDECheckState   ( void                *drv,
                           void                *dev,
                           CardState           *state,
                           DFBAccelerationMask  accel );

void      TDESetState     ( void                *drv,
                           void                *dev,
                           GraphicsDeviceFuncs *funcs,
                           CardState           *state,
                           DFBAccelerationMask  accel );

bool      TDEFillRectangle( void                *drv,
                           void                *dev,
                           DFBRectangle        *rect );

bool      TDEDrawRectangle( void                *drv,
                           void                *dev,
                           DFBRectangle        *rect );

bool      TDEDrawLine     ( void                *drv,
                           void                *dev,
                           DFBRegion           *line );

bool      TDEFillTriangle ( void                *drv,
                           void                *dev,
                           DFBTriangle         *tri );

bool      TDEBlit         ( void                *drv,
                           void                *dev,
                           DFBRectangle        *srect,
                           int                  dx,
                           int                  dy );

bool      TDEStretchBlit  ( void                *drv,
                           void                *dev,
                           DFBRectangle        *srect,
                           DFBRectangle        *drect );

bool      TDEBlit2         ( void *drv, void *dev, DFBRectangle *srect, int dx, int dy ,int sx2, int sy2);


bool TDEBatchBlit( void *driver_data, void *device_data,
                    const DFBRectangle *rects, const DFBPoint *points,
                    unsigned int num, unsigned int *ret_num );

bool TDEBatchFill( void *driver_data, void *device_data,
                    const DFBRectangle *rects,
                    unsigned int num, unsigned int *ret_num );


#ifdef __cplusplus
}
#endif
#endif /* __HI3720_2D_H__ */
