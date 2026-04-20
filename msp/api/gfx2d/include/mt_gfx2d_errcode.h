/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
#ifndef _MT_GFX2D_ERRCODE_H_
#define _MT_GFX2D_ERRCODE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

#define MT_ERR_GFX2D_BASE 0x80000001

enum
{
    MT_ERR_GFX2D_DEV_NOT_OPEN = MT_ERR_GFX2D_BASE,  /**<  device not open yet */
    MT_ERR_GFX2D_DEV_PERMISSION,              /*< device operation failed */
    MT_ERR_GFX2D_INVALID_DEVID,
    MT_ERR_GFX2D_NULL_PTR,                        /**<  parameters contain null ptr */
    MT_ERR_GFX2D_INVALID_COMPOSECNT,      /**< invalid composor count(>7 or =0) */
    MT_ERR_GFX2D_INVALID_SURFACE_TYPE,        /**< invalid surface info:colorfmt,phyaddr,stride,resolution... */
    MT_ERR_GFX2D_INVALID_SURFACE_RESO,
    MT_ERR_GFX2D_INVALID_SURFACE_FMT,
    MT_ERR_GFX2D_INVALID_SURFACE_ADDR,
    MT_ERR_GFX2D_INVALID_SURFACE_STRIDE,
    MT_ERR_GFX2D_INVALID_SURFACE_CMPTYPE,
    MT_ERR_GFX2D_INVALID_RECT,            /**< invalid opt rect:1.no intersection with surface */
    MT_ERR_GFX2D_INVALID_RESIZE_FILTERMODE,          /**< invalid resize info:1.invalid resize raito */
    MT_ERR_GFX2D_INVALID_RESIZE_RATIO,
    MT_ERR_GFX2D_INVALID_CLIP_MODE,                /**< invalid clip info */
    MT_ERR_GFX2D_INVALID_CLIP_RECT,
    MT_ERR_GFX2D_UNSUPPORT,           /**<  unsupported operation */
    MT_ERR_GFX2D_NO_MEM,                          /**<  lack of memory  */
    MT_ERR_GFX2D_TIMEOUT,                     /**<  sync task timeout */
    MT_ERR_GFX2D_INTERRUPT,               /*sync task interrupted by system*/
    MT_ERR_GFX2D_SYS,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

#endif /*_MT_GFX2D_ERRCODE_H_*/
