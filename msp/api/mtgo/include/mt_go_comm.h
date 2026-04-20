/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_GO_COMM_H__
#define __MT_GO_COMM_H__

/* add include here */
#include "mt_type.h"
#include "mt_go_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

/***************************** Macro Definition ******************************/
/*************************** Structure Definition ****************************/
/** \addtogroup      MTGO_COMMON */
/** @{ */  /**<!—[MTGO_COMMON] */

/**Maximum number of colors in the palette*//** CNcomment: 调色板最大颜色数 */
#define MAX_PALETTE_COLOR_SIZE 256

/**Color value*//** CNcomment: 颜色值 */
typedef mt_u32 MT_COLOR;

/**Palette*//** CNcomment: 调色板 */
typedef MT_COLOR MT_PALETTE[MAX_PALETTE_COLOR_SIZE];

typedef enum 
{
    MTGO_IMGTYPE_JPEG = 0, /**<.jpeg picture*//**<CNcomment: JPEG格式图片*/
    MTGO_IMGTYPE_GIF,      /**<.gif picture*//**<CNcomment: GIF格式图片*/
    MTGO_IMGTYPE_BMP,       /**<.bmp picture*//**<CNcomment: BMP格式图片 */
    MTGO_IMGTYPE_PNG,      /**<.png picture*//**<CNcomment: PNG格式图片 */
    MTGO_IMGTYPE_RLE,      /**<.rle picture*//**<CNcomment: RLE格式图片 */
    MTGO_IMGTPYE_BUTT
} MTGO_IMGTYPE_E;


/**Rectangle*//** CNcomment: 矩形 */
typedef struct
{
    mt_s32 x, y;

    mt_s32 w, h;
} MT_RECT;

/**Region*//** CNcomment: 区域 */
typedef struct
{
    mt_s32 l;
    mt_s32 t;
    mt_s32 r;
    mt_s32 b;
} MT_REGION;

/**Mode of adjusting the window z-order*//**CNcomment:Z序调整方式*/
typedef enum
{
    MTGO_ZORDER_MOVETOP = 0,  /**<Move to the top*//**<CNcomment:移到最顶部*/
    MTGO_ZORDER_MOVEUP,       /**<Move upwards*//**<CNcomment:向上移*/
    MTGO_ZORDER_MOVEBOTTOM,   /**<Move to the bottom*//**<CNcomment:移到最底部*/
    MTGO_ZORDER_MOVEDOWN,     /**<Move downwards*//**<CNcomment:向下移*/
    MTGO_ZORDER_BUTT
} MTGO_ZORDER_E;

#define MTGO_INVALID_HANDLE 0x0

/**Stream position*//** CNcomment: 流式位置*/


/** @} */  /*! <!-- Structure Definition end */

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
/** \addtogroup      MTGO_COMMON */
/** @{ */  /** <!—[MTGO_COMMON] */


/** 
\brief Initializes the MtGO.CNcomment:MtGO初始化 CNend
\attention \n
Before using the MtGO, you must call this application programming interface (API) to initialize the MtGO. The MtGO 
includes the graphic device (Gdev) module, decoder, Winc module, bit block transfer (Bliter) module, and surface 
module.
CNcomment:使用MtGO功能必须先调用该接口，完成MtGO的初始化，主要包括gdev, decoder, winc, blit, surface模块 CNend
\param N/A.CNcomment:无 CNend

\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\retval ::MTGO_ERR_DEPEND_TDE
\retval ::MTGO_ERR_INITFAILED

\see \n
::MT_GO_Deinit
*/
mt_s32	MT_GO_Init(mt_void);

/**
\brief Deinitializes the MtGO.CNcomment: MtGO去初始化 CNend
\attention \n
If the MtGO is not used, you need to call this API to release resources.CNcomment: 不再使用MtGO功能时需要调用该接口，释放资源 CNend
\param N/A.CNcomment: 无 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_DEINITFAILED

\see \n
::MT_GO_Init
*/
mt_s32	MT_GO_Deinit(mt_void);

/** 
\brief Initializes the extended library of the MtGO.CNcomment:MtGO 扩展库初始化 CNend
\attention \n
If the extended library of the MtGO is not used, you need to call this API to release resources, especially the 
resources of the cursor and text modules.
CNcomment:主要是CURSOR和TEXT模块。CNend
\param N/A.CNcomment:无 CNend

\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\retval ::MTGO_ERR_DEPEND_TDE
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_DEINITFAILED

\see \n
::MT_GO_DeInitExt
*/
mt_s32 MT_GO_InitExt(mt_void);

/**
\brief Deinitializes the extended library of the MtGO.CNcomment: MtGO扩展库去初始化 CNend
\attention \n
If the MtGO is not used, you need to call this API to release resources.CNcomment: 不再使用MtGO扩展库功能时需要调用该接口，释放资源，主要是CURSOR和TEXT模块。CNend
\param N/A.CNcomment: 无 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_DEINITFAILED

\see \n
::MT_GO_InitExt
*/
mt_s32 MT_GO_DeInitExt(mt_void);

/** 
\brief Obtains the version information.CNcomment:获取版本信息 CNend
\attention \n
N/A.CNcomment:无 CNend
\param[out] ppVersion Output address of the version information string. The value cannot be empty.CNcomment: 版本信息字符串输出地址，不可为空 CNend
\param[out] ppBuildTime Build Output address of the time string. The value cannot be empty.CNcomment:Build时间字符串输出地址，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NULLPTR

\see \n
N/A.CNcomment:无 CNend
*/
mt_s32 MT_GO_GetVersion(mt_char ** ppVersion, mt_char **ppBuildTime);

/**
\brief Converts the data on a surface into a .bmp picture for output.CNcomment: 将Surface中的数据转换成BMP格式图象输出。CNend
\attention \n
The output file is [year]-[date]-[hour]-[min]-[second]-[ms].bmp.CNcomment: 输出文件名为[year]-[date]-[hour]-[min]-[second]-[ms].bmp CNend
The output picture must be a 16-bit bitmap.CNcomment: 输出图片固定为16位图。CNend
It is recommended that you call MT_GO_EncodeToFile rather than MT_GO_Surface2Bmp.CNcomment: 建议使用MT_GO_EncodeToFile接口来代替此接口 CNend

\param[in] Surface Data to be captured.CNcomment: 需要进行截屏的数据。CNend
\param[in] pRect Pointer to a rectangle. If this parameter is not set, it indicates the entire surface.CNcomment: 区域指针,为空表示整个surface。CNend


\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVPARAM
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVSURFACESIZE
\retval ::MTGO_ERR_INVSURFACEPF
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NOMEM

*/
mt_s32	MT_GO_Surface2Bmp(mt_handle Surface, const MT_RECT *pRect);

/** 
\brief Enables the memory management module.CNcomment:使能内存管理模块 CNend
\attention \n
The memory management module is disabled by default. Before using the memory statistics function, you must call this 
API to enable the memory management module.
CNcomment:默认情况处于非使能状态，只有该接口打开后才能使用内存统计功能 CNend

\param[in] bEnable Whether to enable the memory statistics function.CNcomment:是否开启内存统计功能 CNend

\retval ::MT_SUCCESS

*/
mt_s32 MT_GO_EnableMemMng(MT_BOOL bEnable);



/** 
\brief Obtains the enable status of the memory management module.CNcomment:获取内存管理模块使能状态 CNend
\attention \n
\param[out] pbEnable Whether to obtain the enable status of the memory statistics function.CNcomment:获取内存统计功能是否开启 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NULLPTR

*/
mt_s32 MT_GO_GetMemMngStatus(MT_BOOL *pbEnable);

/** 
\brief Outputs the general information about the system memory.CNcomment:输出系统内存的总体信息 CNend
\attention \n
\param N/A.CNcomment:无 CNend

\retval ::MT_SUCCESS
*/
mt_s32 MT_GO_SysMemQuene(mt_void);

/** 
\brief Outputs the general information about the media memory zone (MMZ).CNcomment: 输出MMZ内存的总体信息。CNend
\attention \n
\param N/A.CNcomment:无 CNend

\retval ::MT_SUCCESS

*/
mt_s32 MT_GO_MMZMemQuene(mt_void);

/** @} */  /*! <!-- API declaration end */


#ifdef __cplusplus
#if __cplusplus
}
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

#endif /* __MT_GO_COMM_H__ */

