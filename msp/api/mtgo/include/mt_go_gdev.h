/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_GO_GDEV_H__
#define __MT_GO_GDEV_H__

#include "mt_go_comm.h"
#include "mt_go_surface.h"

#ifdef __cplusplus
extern "C"{
#endif  /*__cplusplus*/

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/
/** \addtogroup      MTGO_GDEV */
/** @{ */  /** <!-- [MTGO_GDEV] */

/** Definition of the graphics layer ID*/
/** CNcomment:图层ID定义*/
typedef enum
{
    MTGO_LAYER_BACKGROUND = 0x0,
    MTGO_LAYER_OSD0,
    MTGO_LAYER_OSD1,
    MTGO_LAYER_SUB,
    MTGO_LAYER_STILL,
    MTGO_LAYER_HD_0,
    MTGO_LAYER_HD_1,
    MTGO_LAYER_HD_2,
    MTGO_LAYER_HD_3,
    MTGO_LAYER_SD_0,    
    MTGO_LAYER_SD_1,    
    MTGO_LAYER_SD_2,     
    MTGO_LAYER_SD_3, 
    MTGO_LAYER_AD_0,  
    MTGO_LAYER_AD_1,
    MTGO_LAYER_AD_2,
    MTGO_LAYER_AD_3,
    MTGO_LAYER_CURSOR,
    MTGO_LAYER_BUTT
/*    
    MTGO_LAYER_SD_0 = 0,
    MTGO_LAYER_SD_1, 
    MTGO_LAYER_HD_0,
    MTGO_LAYER_HD_1,
    MTGO_LAYER_HD_2,    
    MTGO_LAYER_HD_3,    
    MTGO_LAYER_AD_0, 
    MTGO_LAYER_AD_1, 
    MTGO_LAYER_BUTT,
*/    
}MTGO_LAYER_E;

/**Standard-definition (SD) graphics layer 0*/
/** CNcomment:标清图形叠加层0 */
#define MTGO_LAYER_RGB0 MTGO_LAYER_SD_0

/**SD graphics layer 1*/
/** CNcomment:标清图形叠加层1 */
#define MTGO_LAYER_RGB1 MTGO_LAYER_SD_1

#define GRAPMTCS_LAYER_MAX_NUM 4
/** The following macro defines the buffer mode of each graphics layer of the MtGo. The canvas buffer is used for drawing,
and the display buffer is used for display output.*/
/** CNcomment:下面宏定义了mtgo的每个图层的buffer模式，其中canvas buffer是供用户绘制的buffer, display buffer是用于显示输出的buffer.*/
#define MTGO_LAYER_BUFFER_SINGLE      0x02 /**<One canvas buffer, and no display buffer*//**<CNcomment:1块canvas buffer, 无display buffer */    
#define MTGO_LAYER_BUFFER_DOUBLE      0x04 /**<One canvas buffer, and one display buffer. Dual buffers are supported.*//**<CNcomment:1块canvas buffer, 1块display buffer 支持双缓冲 */ 
#define MTGO_LAYER_BUFFER_TRIPLE      0x08 /**<One canvas buffer, and two display buffers. The flip function is supported.*//**<CNcomment:1块canvas buffer, 2块display buffer 支持flip,刷新的时候等待任务完成 */
#define MTGO_LAYER_BUFFER_OVER        0x10 /**<One canvas buffer, and two display buffers. The flip function is supported. If tasks are being performed during refresh, the current frame is discarded.*//**<CNcomment:1块canvas buffer, 2块display buffer 支持flip,如果刷新的时候带有任务忙，则丢弃当前帧 */

/**Refresh mode of graphics layers for complying with old definitions. The mode is not recommended.*/
/** CNcomment:为兼容老定义，图层的刷新方式，不建议使用*/
typedef enum 
{
    MTGO_LAYER_FLUSH_FLIP        = MTGO_LAYER_BUFFER_TRIPLE, 
    MTGO_LAYER_FLUSH_DOUBBUFER   = MTGO_LAYER_BUFFER_DOUBLE, 
    MTGO_LAYER_FLUSH_NORMAL      = MTGO_LAYER_BUFFER_SINGLE, 
    MTGO_LAYER_FLUSH_OVER        = MTGO_LAYER_BUFFER_OVER,
    MTGO_LAYER_FLUSH_BUTT
} MTGO_LAYER_FLUSHTYPE_E;

/**Anti-flicker level, ranging from low to high. The higher the level, the better the anti-flicker effect, but the more blurred the picture.*/
/** CNcomment:抗闪烁级别，值为LOW~MTGH,值越大抗闪烁效果越好，但越模糊*/
typedef enum
{
    MTGO_LAYER_DEFLICKER_NONE = 0, 
    MTGO_LAYER_DEFLICKER_LOW,
    MTGO_LAYER_DEFLICKER_MIDDLE,
    MTGO_LAYER_DEFLICKER_MTGH,
    MTGO_LAYER_DEFLICKER_AUTO,    
    MTGO_LAYER_DEFLICKER_BUTT
}MTGO_LAYER_DEFLICKER_E;



/**3D STEREO mode*/
/** CNcomment:3D STEREO模式*/
typedef enum
{
    MTGO_STEREO_MODE_HW_FULL = 0x0,  /**< 3d stereo function use hardware and transfer full frame to vo, note: hardware doesn't support the mode if encoder picture delivery method is top and bottom */    
    MTGO_STEREO_MODE_HW_HALF ,    /**< 3d stereo function use hardware and transfer half frame to vo*/
    MTGO_STEREO_MODE_SW_EMUL,              /**<3d stereo function use software emulation */
    MTGO_STEREO_MODE_BUTT
}MTGO_STEREO_MODE_E;

/**Layer attribute parameters*/
/** CNcomment:图层属性参数*/
typedef struct 
{
    mt_s32                 ScreenWidth;    /**<Height of a graphics layer on the screen. The value must be greater than 0.*//**<CNcomment:图层在屏幕上显示宽度，必须大于0 */
    mt_s32                 ScreenHeight;   /**<Width of a graphics layer on the screen. The value must be greater than 0.*//**<CNcomment:图层在屏幕上显示高度，必须大于0 */
    mt_s32                 CanvasWidth;    /**<Width of the canvas buffer of a graphics layer. If the value is 0, no canvas buffer is created.*//**<CNcomment:图层的绘制buffer宽度，为0时，不创建绘制buffer */
    mt_s32                 CanvasHeight;   /**<Height of the canvas buffer of a graphics layer. If the value is 0, no canvas buffer is created.*//**<CNcomment:图层的绘制buffer高度，为0时，不创建绘制buffer */
    mt_s32                 DisplayWidth;   /**<Width of the display buffer of a graphics layer. If the value is 0, the value of ScreenWidth is used.*//**<CNcomment:图层的显示buffer宽度，为0时，和ScreenWidth相同*/
    mt_s32                 DisplayHeight;  /**<Height of the display buffer of a graphics layer. If the value is 0, the value of ScreenHeight is used.*//**<CNcomment:图层的显示buffer高度，为0时，和ScreenHeight相同*/
    MTGO_LAYER_FLUSHTYPE_E LayerFlushType; /**< Refresh mode of the layer. You can choose the refresh mode based on the actual scenario to improve the refresh efficiency. If the value is 0, the dual-buffer+flip refresh mode is used by default. *//**<CNcomment:图层的刷新方式，用户可根据使用场景选择不同的刷新模式来提高刷新效率,等于0时默认使用双缓冲+Flip刷新模式 */
    MTGO_LAYER_DEFLICKER_E AntiLevel;      /**<Anti-flicker level of a graphics layer*//**<CNcomment:图层抗闪烁级别 */
    MTGO_PF_E              PixelFormat;    /**<Pixel format of a graphics layer. The format must be supported by the hardware layer. You need to choose the pixel format parameters of the layer based on hardware devices.*//**<CNcomment:图层的像素格式，此格式必须为硬件图层所支持的格式，请根据不同硬件设备来选择图层的像素格式参数 */
    MTGO_LAYER_E           LayerID;        /**<Hardware ID of a graphics layer. The supported graphics layer depends on the chip platform. For example, the Hi3720 supports an HD graphics layer and an SD graphics layer.*//**<CNcomment:图层硬件ID，能支持图层取决于芯片平台，hi3720高清支持一个高清和一个标清 */
    mt_u32                 EnableOsdc;     /**<use OSD compression & decompression or not.*//**<CNcomment: 是否启用OSDC模块 */
} MTGO_LAYER_INFO_S;

/**Status of a graphics layer*/
/** CNcomment:图层状态结构 */
typedef struct 
{
    MT_BOOL bShow;             /**<Whether to display a graphics layer.*//**<CNcomment:图层是否显示 */
} MTGO_LAYER_STATUS_S;

typedef struct
{
    MT_COLOR ColorKey;         /**<Transparency of a graphics layer*//**<CNcomment:图层的透明色*/
    MT_BOOL bEnableCK;         /**<Whether the colorkey of a graphics layer is enabled.*//**<CNcomment:图层是否使能colorkey */
}MTGO_LAYER_KEY_S;

typedef struct
{
    MT_BOOL bAlphaEnable;   /**<Alpha pixel enable flag*//**<CNcomment:alpha像素使能标志 */
    MT_BOOL bAlphaChannel;  /**<Alpha channel enable flag*//**<CNcomment:alpha通道使能标志  */
    mt_u8   Alpha0;         /**<Alpha0 value. It is valid in ARGB1555 format.*//**<CNcomment:alpha0值,在ARGB1555格式下生效 */
    mt_u8   Alpha1;         /**<Alpha1 value. It is valid in ARGB1555 format.*//**<CNcomment:alpha1值,在ARGB1555格式下生效 */
    mt_u8   GlobalAlpha;    /**<Global alpha. This value is valid only when the alpha channel is valid.*//**<CNcomment:全局alpha，该值只有在alpha通道有效的时候才有意义 */
    MT_BOOL bRegionAlphaEnable;   /**<  region alpha enable flag *//**<CNcomment: region alpha使能标识*/
    mt_u8 u8RegionAlpha;    /**<  region alpha value *//**<CNcomment: region alpha取值*/
}MTGO_LAYER_ALPHA_S;

#if 0
/**Frame encode format*/
/**CNcomment:帧编码传输格式*/
typedef enum
{
    MTGO_STEREO_MONO   = 0x0,             /**< Normal display, no 3D TV*//**<CNcomment:正常输出，非3D 电视*/
    MTGO_STEREO_SIDEBYSIDE_HALF,          /**< L/R frames are downscaled horizontally by 2 andpacked side-by-side into a single frame, left on lefthalf of frame*//**<CNcomment:将L/R帧水平缩放到单帧中*/
    MTGO_STEREO_TOPANDBOTTOM,             /**< L/R frames are downscaled vertically by 2 andpacked into a single frame, left on top*//**<CNcomment:将L/R帧垂直缩放到单帧中*/
    MTGO_STEREO_BUTT
}MTGO_STEREO_MODE_E;
#endif

typedef struct
{
    mt_handle      Layer;          /** The layer the scrolltext want to show */
    MT_RECT       stScrollRect;   /** the position you wanted to show on the layer */
    MTGO_PF_E    ePixelFormat;    /** the color format of scrolltext content */
    mt_u16         u16CacheNum;       /** The cached buffer number for store scrolltext content */
    mt_u16         u16RefreshFreq;    /** The refresh frequency you wanted */    
    MT_BOOL      bDeflicker;        /** Whether enable antificker */
} MTGO_SCROLLTEXT_ATTR_S;

typedef struct
{
    mt_u32   u32PhyAddr;  /** The physical address of the scrolltext content buffer */
    mt_u8    *pu8VirAddr;  /** The virtual address of the scrolltext content buffer */
    mt_u32   u32Stride;     /** The stride of the scrolltext content buffer */
} MTGO_SCROLLTEXT_DATA_S;



/** @} */  /*! <!-- Structure Definition end */

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
/** \addtogroup      MTGO_GDEV */
/** @{ */  /** <!--[MTGO_GDEV] */

/** 
\brief Initializes a display device. CNcomment:初始化显示设备 CNend
\attention \n
When ::MT_GO_Init is called, this API is also called.
CNcomment: ::MT_GO_Init已包含了对该接口的调用 CNend
\param N/A. CNcomment:无 CNend

\retval ::MT_FAILURE
\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INITFAILED

\see \n
::MT_GO_Init \n
::MT_GO_DeinitDisplay
*/
mt_s32 MT_GO_InitDisplay(mt_void);

/** 
\brief Deinitializes a display device. CNcomment:去初始化显示设备 CNend
\attention \n
When ::MT_GO_Deinit is called, this API is also called.
CNcomment: ::MT_GO_Deinit已包含了对该接口的调用 CNend
\param N/A. CNcomment:无 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_DEINITFAILED

\see \n
::MT_GO_Deinit \n
::MT_GO_InitDisplay
*/
mt_s32 MT_GO_DeinitDisplay(mt_void);

/**
\brief Obtains the default parameters of an SD or HD graphics layer based on its ID.
If you do not want to use default values, you can set the members of pLayerInfo.
CNcomment:根据图层ID获取相应图层(SD,HD)创建时的默认参数，
如果需要使用非默认值，可以直接设置pLayerInfo各个成员 CNend
\attention \n

\param[in] LayerID Layer ID. CNcomment:图层ID CNend
\param[in]  pLayerInfo Obtained parameters of a graphics layer when it is created. CNcomment:获取到的创建参数 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVLAYERID
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVPARAM
\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_GetLayerDefaultParam (MTGO_LAYER_E LayerID, MTGO_LAYER_INFO_S *pLayerInfo);


/** 
\brief Creates a graphics layer. CNcomment:创建图层 CNend
\attention \n
The platform determines whether VO hardware scaling is supported.
If VO hardware scaling is supported, the display size is scaled to fit the screen when the display size is inconsistent with the screen size.
If VO hardware scaling is not supported, the display size and screen size must be the same.
CNcomment:是否支持VO硬件缩放取决于平台。
如果平台支持VO 硬件缩放，即在设置displaysize与screensize不一致的情况下，最终会缩放到screensize的大小。
如果平台不支持VO 硬件缩放，即使displaysize与screensize不一致，也会强制要求displaysize与screensize一致 CNend

\param[in]  pLayerInfo Basic attributes of a graphics layer. The value cannot be empty. CNcomment:图层基本属性，不可为空 CNend
\param[out] pLayer Pointer to the handle of a graphics layer. The value cannot be empty. CNcomment:图层的句柄指针，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\retval ::MTGO_ERR_NOTINIT 
\retval ::MTGO_ERR_NULLPTR 
\retval ::MTGO_ERR_INVSIZE 
\retval ::MTGO_ERR_INVLAYERID 
\retval ::MTGO_ERR_INVPIXELFMT 
\retval ::MTGO_ERR_INVFLUSHTYPE
\retval ::MTGO_ERR_INVANILEVEL
\retval ::MTGO_ERR_INVSIZE
\retval ::MTGO_ERR_NOMEM
\retval ::MTGO_ERR_INTERNAL
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB
\retval ::MTGO_ERR_ALREADYBIND
\retval ::MTGO_ERR_INVLAYERSURFACE

\see \n
::MT_GO_DestroyLayer
*/
mt_s32 MT_GO_CreateLayer (const MTGO_LAYER_INFO_S *pLayerInfo, mt_handle* pLayer);

/** 
\brief Destroys a graphics layer. CNcomment:销毁图层 CNend
\attention \n
If a graphics layer is in use (for example, the desktop based on the layer is not destroyed), the layer cannot be destroyed.
CNcomment:当图层正在被使用时（例如基于此图层的桌面未销毁），则图层无法被销毁 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend

\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE


\see \n
::MT_GO_CreateLayer
*/
mt_s32 MT_GO_DestroyLayer(mt_handle Layer);


/** 
\brief Sets the position of a graphics layer on the screen. CNcomment:设置图层在屏幕中的位置 CNend
\attention \n
The value takes effect at once and you do not need to refresh it. If the start position of a layer exceeds the screen, it is automatically adjusted to the boundary of the screen.
CNcomment:立即生效，无需刷新。当设置的图层的起始位置超出屏幕范围时会自动调整至屏幕的边界值 CNend
\param[in] Layer  Layer handle. CNcomment:图层句柄 CNend
\param[in] u32StartX Horizontal coordinate of the position where a layer appears on the screen. CNcomment:图层在屏幕上的显示位置X坐标 CNend
\param[in] u32StartY Vertical coordinate of the position where a layer appears on the screen. CNcomment:图层在屏幕上的显示位置Y坐标 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVLAYERPOS
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_GetLayerPos
*/
mt_s32 MT_GO_SetLayerPos(mt_handle Layer, mt_u32 u32StartX, mt_u32 u32StartY);

/** 
\brief Obtains the position where a layer appears on the screen. CNcomment:获取图层在屏幕上的位置 CNend
\attention \n
N/A. CNcomment:无 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[out] pStartX Pointer of the horizontal coordinate of the position where a layer appears on the screen. The value cannot be empty. CNcomment:图层在屏幕上的显示位置X坐标指针，不可为空 CNend
\param[out] pStartY Pointer of the vertical coordinate of the position where a layer appears on the screen. The value cannot be empty. CNcomment:图层在屏幕上的显示位置Y坐标指针，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_SetLayerPos
*/
mt_s32 MT_GO_GetLayerPos(mt_handle Layer, mt_u32 *pStartX, mt_u32 *pStartY);


/** 
\brief Sets the output height and width of a graphics layer on the display device. CNcomment:设置在输出显示设备上的输出的高度和宽度 CNend
\attention \n
This API supports zoom in operations. The maximum range is specified when a graphics layer is created.\n
CNcomment:在内存支持的前提下，该接口可以支持缩放 CNend

\param[in]  Layer Layer handle. CNcomment:图层句柄 CNend
\param[in] u32SWidth  Actual output width. CNcomment:实际输出宽度 CNend
\param[in] u32SHeight Actual output height. CNcomment:实际输出高度 CNend
\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVSIZE
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_GetScreenSize
*/
mt_s32 MT_GO_SetScreenSize(mt_handle Layer, mt_u32 u32SWidth, mt_u32 u32SHeight);


/** 
\brief Obtains the output height and width of a graphics layer on the display device. CNcomment:获取在输出显示设备上的输出的高度和宽度。CNend 
\attention \n
N/A. CNcomment:无 CNend
\param[in]  Layer Layer handle. CNcomment:图层句柄 CNend
\param[out] pSWidth  Pointer to the actual output width. CNcomment:实际输出宽度指针 CNend
\param[out] pSHeight Pointer to the actual output height. CNcomment:实际输出高度指针 CNend
\retval none
\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_SetScreenSize
*/
mt_s32 MT_GO_GetScreenSize(mt_handle Layer, mt_u32 *pSWidth, mt_u32 *pSHeight);

/** 
\brief Sets the global alpha value of a layer surface. CNcomment:设置图层surface的全局alpha值 CNend
\attention \n
The value takes effect at once, and you do not need to refresh it.
CNcomment:立即生效，无需刷新 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in] Alpha Global alpha value. The value cannot be empty and ranges from 0 to 255. CNcomment:Alpha 全局alpha值，不可为空，范围0-255 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_SETALPHAFAILED
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_GetLayerAlpha
*/

mt_s32 MT_GO_SetLayerAlpha(mt_handle Layer, mt_u8 Alpha);

/** 
\brief Obtains the global alpha value of a layer surface. CNcomment:获取图层surface的全局alpha值 CNend
\attention \n
N/A. CNcomment:无 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[out] pAlpha Pointer to the global alpha value. CNcomment:全局alpha指针 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_SetLayerAlpha
*/
mt_s32 MT_GO_GetLayerAlpha(mt_handle Layer, mt_u8* pAlpha);

/** 
\brief Sets the region alpha value of a layer surface. CNcomment:设置图层surface的region alpha值 CNend
\attention \n
The value takes effect at once, and you do not need to refresh it.
CNcomment:立即生效，无需刷新 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in] Alpha region alpha value. The value cannot be empty and ranges from 0 to 255. CNcomment:Alpha region alpha值，不可为空，范围0-255 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_SETALPHAFAILED
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_GetLayerAlpha
*/

mt_s32 MT_GO_SetRegionAlpha(mt_handle Layer, MT_BOOL Enable, mt_u8 Alpha);

/** 
\brief Obtains the region alpha value of a layer surface. CNcomment:获取图层surface的region alpha值 CNend
\attention \n
N/A. CNcomment:无 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[out] pAlpha Pointer to the region alpha value. CNcomment:region alpha指针 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_SetLayerAlpha
*/
mt_s32 MT_GO_GetRegionAlpha(mt_handle Layer, mt_u8* pAlpha);

/** 
\brief Obtains the surface of a graphics layer. CNcomment:获取图层的surface CNend
\attention \n
Surface of a graphics layer. It cannot be released by calling MT_GO_FreeSurface. The surface can be released only when the corresponding layer is destroyed.
After a process is switched, you must obtain the layer surface again by calling mt_s32 MT_GO_GetLayerSurface.
CNcomment:图层surface，不能使用MT_GO_FreeSurface来释放。只有在销毁图层的时候才会被释放
进程切换之后必须调用该接口来重新获取图层surface; CNend
\param[in] Layer handle. CNcomment:Layer 图层句柄 CNend
\param[out] pSurface Pointer to the surface handle. The value cannot be empty. CNcomment:surface句柄指针，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVLAYERSURFACE
\retval ::MTGO_ERR_NOMEM
\retval ::MTGO_ERR_DEPEND_FB

\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_GetLayerSurface(mt_handle Layer, mt_handle *pSurface);

/**
\brief Shows or hides a graphics layer. CNcomment:显示或隐藏图层 CNend
\attention \n
The value takes effect at once, and you do not need to refresh it.
CNcomment:立即生效，无需刷新 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in] bVisbile Show/hide flag. MT_TRUE: show; MT_FALSE: hide. CNcomment:显示隐藏标志。MT_TRUE：显示；MT_FALSE：隐藏 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_GetLayerStatus
*/
mt_s32 MT_GO_ShowLayer(mt_handle Layer, MT_BOOL bVisbile);

/** 
\brief Obtains the current status of a graphics layer. CNcomment:获取图层当前状态 CNend
\attention \n
N/A. CNcomment:无 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[out] pLayerStatus Pointer to the current status of a graphics layer. The value cannot be empty. CNcomment:图层当前状态结构指针，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_ShowLayer
*/
mt_s32 MT_GO_GetLayerStatus(mt_handle Layer, MTGO_LAYER_STATUS_S* pLayerStatus);

/** 
\brief Refreshes a graphics layer. CNcomment:刷新图层 CNend
\attention \n
1. After drawing, you need to refresh the layer to display the drawing result.
2. There are two display modes after a graphics layer is refreshed. If there is no window, the contents of the layer surface are displayed; if there is a window, its contents are displayed.
CNcomment:1.绘制完成后，需刷新图层才能显示绘制后结果 
2.刷新有两种模式，一种是没有任何窗口的时候就显示layersurface的内容，否则显示窗口中的内容。CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in]  pRect Rectangle to be refreshed. If the value is NULL, the entire screen is refreshed. CNcomment:刷新的矩形区域，如果是NULL, 则刷新整个全屏 CNend
\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVLAYERSURFACE
\retval ::MTGO_ERR_DEPEND_FB
\retval ::MTGO_ERR_OUTOFBOUNDS
\retval ::MTGO_ERR_EMPTYRECT

\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_RefreshLayer(mt_handle Layer, const MT_RECT* pRect);

/** 
\brief Refreshes a graphics layer sync with display. CNcomment:刷新图层,和显示同步 CNend
\attention \n
1. After drawing, you need to refresh the layer to display the drawing result.
2. There are two display modes after a graphics layer is refreshed. If there is no window, the contents of the layer surface are displayed; if there is a window, its contents are displayed.
CNcomment:1.绘制完成后，需刷新图层才能显示绘制后结果 
2.刷新有两种模式，一种是没有任何窗口的时候就显示layersurface的内容，否则显示窗口中的内容。CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in]  pRect Rectangle to be refreshed. If the value is NULL, the entire screen is refreshed. CNcomment:刷新的矩形区域，如果是NULL, 则刷新整个全屏 CNend
\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVLAYERSURFACE
\retval ::MTGO_ERR_DEPEND_FB
\retval ::MTGO_ERR_OUTOFBOUNDS
\retval ::MTGO_ERR_EMPTYRECT

\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_RefreshLayerSync(mt_handle Layer, const MT_RECT* pRect);

/** 
\brief Refreshes a graphics layer sync with display. CNcomment:刷新图层, 同步缩放到指定显存位置 CNend
\attention \n
1. After drawing, you need to refresh the layer to display the drawing result.
2. There are two display modes after a graphics layer is refreshed. If there is no window, the contents of the layer surface are displayed; if there is a window, its contents are displayed.
CNcomment:1.绘制完成后，需刷新图层才能显示绘制后结果 
2.刷新有两种模式，一种是没有任何窗口的时候就显示layersurface的内容，否则显示窗口中的内容。CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in]  pRect Destination Rectangle to be refreshed. If the value is NULL, the entire screen is refreshed. CNcomment:刷新的显存矩形区域，如果是NULL, 则刷新到整个显存 CNend
\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVLAYERSURFACE
\retval ::MTGO_ERR_DEPEND_FB
\retval ::MTGO_ERR_OUTOFBOUNDS
\retval ::MTGO_ERR_EMPTYRECT

\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_RefreshLayerSync_Zoom(mt_handle Layer, const MT_RECT* pZoomRect);


/**
\brief Sets the canvas buffer of a graphics layer. CNcomment:设置层的canvas surface. CNend
\attention \n
If the stream sources of the HD and SD graphics layer are the same, the canvas buffer of the SD graphics layer can be shared with the HD graphics layer.
CNcomment:高清和标清同源时，我们可以让标清的canvas buffer与高清的共享同一个 
     Surrface的内存类型必须是MMZ类型的. CNend
\param[in] Layer     Layer handle. CNcomment:图层句柄 CNend
\param[in] hSurface  Surface of a graphics layer. If the value is INVALID_HANDLE, there is no user handle. CNcomment:图层的surface，如果该参数为INVALID_HANDLE表示没有用户句柄 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVLAYERSURFACE
\retval ::MTGO_ERR_INVPIXELFMT
\retval ::MTGO_ERR_INVPARAM

\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_SetLayerSurface(mt_handle Layer, mt_handle hSurface);



/**
\brief Sets the background color of a graphics layer. CNcomment:设置图层的背景颜色 CNend
\attention \n
The background color of a graphics layer takes effect only when widows are overlaid with each other.
CNcomment:图层背景色，只有在叠加窗口才有效 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in] Color Background color of a graphics layer. CNcomment:图层背景颜色 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE

\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_SetLayerBGColor(mt_handle Layer, MT_COLOR Color);

/**
\brief Obtains the background color of a graphics layer. CNcomment:获取图层的背景颜色 CNend
\attention \n
This API is available only when there are windows on graphics layers. Otherwise, the configured background color does not take effect.
CNcomment:此接口只有在图层上有窗口的时候才会有效。否则该颜色设置无效。CNend
\param[in] Layer Layer handle. CNcomment:图层句柄 CNend
\param[in] pColor Used for storing the background color of a graphics layer. CNcomment:存储图层背景颜色 CNend
 
\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE

\see \n
N/A. CNcomment:无 CNend
*/
mt_s32 MT_GO_GetLayerBGColor(mt_handle Layer, MT_COLOR* pColor);

/**
\brief Sets the size of a display buffer. CNcomment:设置显示buffer size CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer      Layer handle. CNcomment:图层句柄 CNend
\param[in] u32DWidth     Pointer to the width. CNcomment:宽度指针 CNend
\param[in] u32DHeight    Pointer to the height. CNcomment:高度指针 CNend
\param[out] N/A . CNcomment:无  CNend

\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVSIZE 
\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE

\see \n
    ::MT_GO_GetDisplaySize 
*/
mt_s32 MT_GO_SetDisplaySize(mt_handle Layer, mt_u32 u32DWidth, mt_u32 u32DHeight);

/**
\brief Obtains the size of a display buffer. CNcomment:获取显示buffer的size CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer      Layer handle. CNcomment:图层句柄 CNend
\param[in] pDWidth    Pointer to the width. The value cannot be empty. CNcomment:宽度指针，不可为空  CNend
\param[in] pDHeight   Pointer to the height. The value cannot be empty. CNcomment:高度指针，不可为空 CNend
\param[out] N/A. CNcomment:无 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE

\see \n
    ::MT_GO_SetDisplaySize 
*/
mt_s32 MT_GO_GetDisplaySize(mt_handle Layer, mt_u32 *pDWidth, mt_u32 *pDHeight);

/**
\brief Sets the refresh mode. CNcomment:设置刷新类型 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer         Layer handle. CNcomment:图层句柄 CNend           
\param[in] FlushType     Refresh mode of a graphics layer. CNcomment:图层刷新类型 CNend
\param[out] N/A. CNcomment:无 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVFLUSHTYPE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_GetFlushType 
*/
mt_s32 MT_GO_SetFlushType(mt_handle Layer, MTGO_LAYER_FLUSHTYPE_E FlushType);

/**
\brief Obtains the refresh mode. CNcomment:获取刷新类型 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄  CNend             
\param[in] *pFlushType Refresh mode of a graphics layer. The value cannot be empty. CNcomment:图层刷新类型，不可为空  CNend
\param[out] N/A. CNcomment:无  CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_SetFlushType
*/
mt_s32 MT_GO_GetFlushType(mt_handle Layer, MTGO_LAYER_FLUSHTYPE_E *pFlushType);

/**
\brief Sets the transparency of a graphics layer. CNcomment:设置图层的透明色 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄  CNend             
\param[in] pKey        Pointer to the transparency information. The value cannot be empty. CNcomment:透明色信息指针，不可为空 CNend
\param[out] N/A. CNcomment:无  CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_GetLayerColorkey
*/
mt_s32 MT_GO_SetLayerColorkey(mt_handle Layer, const MTGO_LAYER_KEY_S *pKey);

/**
\brief Obtains the transparency information about a graphics layer. CNcomment:获取透明色信息 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend              
\param[in] pKey        Pointer to the transparency information. CNcomment:透明色信息指针 CNend
\param[out] N/A. CNcomment:无  CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_SetLayerColorkey
*/
mt_s32 MT_GO_GetLayerColorkey(mt_handle Layer, MTGO_LAYER_KEY_S *pKey);

/**
\brief Obtains the palette of a graphics layer. CNcomment:获取图层调色板 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend               
\param[in] Palette     Palette of a graphics layer. CNcomment:图层调色板 CNend
\param[out] N/A. CNcomment:无  CNend 

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_SetLayerPalette
*/
mt_s32 MT_GO_GetLayerPalette(mt_handle Layer, MT_PALETTE Palette);

/**
\brief Waits for the blanking area. CNcomment:等待消隐区 CNend
\attention \n
N/A
CNcomment:无  CNend
\param[in] Layer       Layer handle. CNcomment:图层句柄  CNend            
\param[out] N/A. CNcomment:无  CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_SetLayerPalette
*/
mt_s32 MT_GO_WaitForBlank(mt_handle Layer);


/**
\brief Configures the alpha information about a graphics layer. This API is an extended API. CNcomment:设置图层alpha信息，扩展接口 CNend
\attention \n
The alpha0 and alpha1 of the data structure MTGO_LAYER_ALPHA_S are valid only in ARGB1555 format.
This API is used to implement the translucent effect in ARGB1555 format.
CNcomment:该接口MTGO_LAYER_ALPHA_S结构体的alpha0,alpha1只在ARGB1555格式下才有效，
用于在ARGB1555格式下实现半透明效果 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend              
\param[in] pAlphaInfo   Alpha information about a graphics layer. The value cannot be empty. CNcomment:图层alpha信息，不可为空 CNend  
\param[out] N/A. 

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_DEPEND_FB

\see \n
::MT_GO_GetLayerAlphaEx
*/
mt_s32 MT_GO_SetLayerAlphaEx(mt_handle Layer,  MTGO_LAYER_ALPHA_S *pAlphaInfo);


/**
\brief Obtains the alpha information about a graphics layer. This API is an extended API. CNcomment:获取图层alpha信息，扩展接口 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend             
\param[out] pAlphaInfo  Alpha information about a graphics layer. The value cannot be empty. CNcomment:图层alpha信息，不可为空 CNend    

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_SetLayerAlphaEx
*/
mt_s32 MT_GO_GetLayerAlphaEx(mt_handle Layer,  MTGO_LAYER_ALPHA_S *pAlphaInfo);

/** 
\brief set the Z order of graphics layer in the sample display. CNcomment:改变同一显示设备上图形层的Z序。CNend
\attention \n
this function make effect imediately. CNcomment:该功能需要硬件支持Z序的修改，立即生效，无需刷新 CNend
\param[in] Layer Layer handle. CNcomment:图层句柄  CNend     
\param[in] ZFlag zorder flag. CNcomment:修改Z序标志 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVORDERFLAG
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_CANNOTCHANGE

\see \n
::MT_GO_GetLayerZorder
*/
mt_s32 MT_GO_SetLayerZorder(mt_handle Layer, MTGO_ZORDER_E enZOrder);

/** 
\brief get the Z order of graphics layer in the sample display. CNcomment:获取同一显示设备上图形层的Z序。CNend
\attention \n
CNcomment:Z序越小的图层越靠下 CNend
\param[in] Layer    Layer handle. CNcomment:图层句柄  CNend
\param[out] pZOrder Z Order Information. CNcomment:图层Z序信息。CNend


\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE

\see \n
::MT_GO_ChangeLayerZorder
*/
mt_s32 MT_GO_GetLayerZorder(mt_handle Layer, mt_u32* pu32ZOrder);

/**
\brief Sets frame input encode mode of a graphic layer. CNcomment:设置图形层输入3D格式，输出格式自动跟随VO进行设置。CNend
\attention \n
MONO is the default mode.
CNcomment:默认为MONO格式，即普通非3D Stereo格式. CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend             
\param[in] EncPicFrm  Frame encode mode.The value cannot be empty. CNcomment:图层帧传输编码格式，不可为空 CNend  

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_GetStereoMode
*/
mt_s32 MT_GO_SetStereoMode(mt_handle Layer,  MTGO_STEREO_MODE_E InputEnc);



/**
\brief Obtains frame encode mode of a graphic layer. CNcomment:获取图形层3D格式。CNend
\attention \n
CNcomment:无 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend             
\param[out] pInputEnc  Frame encode mode.The value cannot be empty. CNcomment:图层帧传输编码格式，不可为空 CNend  

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_SetStereoMode
*/
mt_s32 MT_GO_GetStereoMode(mt_handle Layer,  MTGO_STEREO_MODE_E *pInputEnc);


/**
\brief set the 3d depth, only support depth in Horizontal . CNcomment:设置景深,只支持水平方向景深 CNend
\attention \n
CNcomment:无 CNend

\param[in]  Layer       Layer handle. CNcomment:图层句柄 CNend             
\param[out] StereoDepth  3D depth. CNcomment:景深 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_GetStereoDepth
*/
mt_s32 MT_GO_SetStereoDepth(mt_handle Layer,  mt_s32  s32StereoDepth);


/**
\brief get the 3d depth, only support depth in Horizontal . CNcomment:获取景深,只支持水平方向景深 CNend
\attention \n
CNcomment:无 CNend

\param[in]  Layer       Layer handle. CNcomment:图层句柄 CNend              
\param[out] pStereoDepth  3D depth. CNcomment:景深 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_GetStereoDepth
*/
mt_s32 MT_GO_GetStereoDepth(mt_handle Layer,  mt_s32  *ps32StereoDepth);

/**
\brief Sets compress mode of a graphic layer. CNcomment:设置使能压缩模式 CNend
\attention \n
When compress is enable, only MTGO_PF_8888 is supported;SD layer and  STEREO TOPANDBOTTOM
are also not supported.
CNcomment:只支持像素格式为MTGO_PF_8888，不支持标清和3D STEREO TOPANDBOTTOM模式 CNend

\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend              
\param[in] StereoMode  Stereo mode, the value cannot be empty. CNcomment:压缩使能，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_GetStereoMode
*/
mt_s32 MT_GO_SetCompression(mt_handle Layer,  MT_BOOL bEnable);


/**
\brief Sets compress mode of a graphic layer. CNcomment:获取压缩模式是否使能 CNend
\attention \n
N/A. CNcomment:无 CNend
\param[in] Layer       Layer handle. CNcomment:图层句柄 CNend             
\param[out] pbEnable  Stereo mode, the value cannot be empty. CNcomment:压缩使能，不可为空 CNend  

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_SetStereoMode
*/
mt_s32 MT_GO_GetCompression(mt_handle Layer,  MT_BOOL* pbEnable);

/**
\brief Create ScrollText. CNcomment:创建滚动字幕 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] pstScrollAttr  ScrollText attribute,the value cannot be empty. CNcomment:滚动字幕属性 CNend
\param[out] phScrollText  ScrollText handle. CNcomment:滚动字幕的输出句柄 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_DEPEND_FB 
\retval ::MTGO_ERR_DEPEND_FB
\retval ::MTGO_ERR_INVSURFACEPF
\retval ::MTGO_ERR_INUSE

\see \n
    ::MT_GO_CreateScrollText
*/
mt_s32 MT_GO_CreateScrollText(MTGO_SCROLLTEXT_ATTR_S * pstScrollAttr, mt_handle * phScrollText);


/**
\brief Fill data to ScrollText cache buffer. CNcomment:向滚动字幕送入数据 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] hScrollText  ScrollText handle.     CNcomment:滚动字幕的句柄 CNend
\param[out] pstScrollData ScrollText data information. CNcomment:滚动字幕的数据信息 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR 
\retval ::MTGO_ERR_DEPEND_FB
\retval ::MTGO_ERR_INVHANDLE

\see \n
    ::MT_GO_FillScrollText
*/
mt_s32 MT_GO_FillScrollText(mt_handle hScrollText, MTGO_SCROLLTEXT_DATA_S * pstScrollData);


/**
\brief Pause the ScrollText. CNcomment:暂停滚动字幕 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] hScrollText  ScrollText handle.   CNcomment:滚动字幕的句柄 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_PauseScrollText
*/
mt_s32  MT_GO_PauseScrollText(mt_handle hScrollText);


/**
\brief Resume the ScrollText,the ScrollText has Paused. CNcomment:恢复已暂停的滚动字幕 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] hScrollText  ScrollText handle.  CNcomment:滚动字幕的句柄 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_ResumeScrollText
*/
mt_s32  MT_GO_ResumeScrollText(mt_handle hScrollText);


/**
\brief Destroy ScrollText. CNcomment:销毁滚动字幕 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] hScrollText ScrollText handle.  CNcomment:滚动字幕的句柄 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR 
\retval ::MTGO_ERR_DEPEND_FB

\see \n
    ::MT_GO_DestoryScrollText
*/
mt_s32  MT_GO_DestoryScrollText(mt_handle hScrollText);

/**
\brief Sets the palette of a display buffer. CNcomment:设置图层的调色板 CNend
\attention \n
N/A. CNcomment:无 CNend

\param[in] Layer      Layer handle. CNcomment:图层句柄 CNend
\param[in] pPalette      Pointer to the palette. CNcomment:调色板指针 CNend
\param[in] u32Start     start position of palette. CNcomment:调色板起始位置 CNend
\param[in] u32Len    length from start. CNcomment:从start开始的调色板数目 CNend
\param[out] N/A . CNcomment:无  CNend

\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVSIZE 
\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE

\see \n
    ::MT_GO_SetLayerPalette 
*/
mt_s32 MT_GO_SetLayerPalette(mt_handle Layer, mt_u32 *pPalette, mt_u32 u32Start, mt_u32 u32Len);

/** @} */  /*! <!-- API declaration end */

#ifdef __cplusplus
}
#endif  /*__cplusplus*/

#endif /* __MT_GO_GDEV_H__ */

