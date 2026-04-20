/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file
 \brief Describes the header file of the 2D effect library. CNcomment:2D特性库头文件
 \author 
 \version 1.0
 \author 
 \date 
 */

#ifndef __MT_EFFECT_H__
#define __MT_EFFECT_H__

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#include <mt_type.h>
#include <mt_go.h>



/***************************** Macro Definition ******************************/
/** \addtogroup      Effect */
/** @{ */  /** <!-- [Effect module] */

/**Definition of the HiEffect error ID*/
/** CNcomment:HiEffect 项目错误ID */
#define MTEFFECT_ERR_APPID (0x80000000L + 0x40000000L)

/**Definition of the HiEffect module ID*/
/** CNcomment:HiEffect 模块ID */
#define MTEFFECT_ERR_MODUID 0x00000000

/** @} */  /*! <!-- Macro Definition end */

/*************************** Structure Definition ****************************/
/** \addtogroup      Effect */
/** @{ */  /** <!-- [Effect module] */


typedef enum mtEFFECTLOG_ERRLEVEL_E
{
    MTEFFECT_LOG_LEVEL_DEBUG = 0,  /**<debug-level                                  */
    MTEFFECT_LOG_LEVEL_INFO,       /**<informational                                */
    MTEFFECT_LOG_LEVEL_NOTICE,     /**<normal but significant condition             */
    MTEFFECT_LOG_LEVEL_WARNING,    /**<warning conditions                           */
    MTEFFECT_LOG_LEVEL_ERROR,      /**<error conditions                             */
    MTEFFECT_LOG_LEVEL_CRIT,       /**<critical conditions                          */
    MTEFFECT_LOG_LEVEL_ALERT,      /**<action must be taken immediately             */
    MTEFFECT_LOG_LEVEL_FATAL,      /**<just for compatibility with previous version */
    MTEFFECT_LOG_LEVEL_BUTT
} MTEFFECT_LOG_ERRLEVEL_E;

/**Definition of the HiEffect error code*/
/** CNcomment:HiEffect 错误码定义宏 */
#define MTEFFECT_DEF_ERR(module, errid) \
    ((mt_s32)((MTEFFECT_ERR_APPID) | ((module) << 16) | ((MTEFFECT_LOG_LEVEL_ERROR) << 13) | (errid)))


enum MTEFFECT_ErrorCode_E
{
    ERR_MTEFFECT_CODE_UNDEF,               
    ERR_MTEFFECT_PTR_NULL,                 /**<pointer is NULL*/
    ERR_MTEFFECT_NO_OPEN,                  /**<effect not open*/
    ERR_MTEFFECT_OPENED,                   /**<effect opened*/
    ERR_MTEFFECT_CLOSED,                   /**<effect closed*/

    /* New */
    ERR_MTEFFECT_INVALID_DEVICE,           /**<invalid device */
    ERR_MTEFFECT_INVALID_HANDLE,           /**<invalid handle*/
    ERR_MTEFFECT_INVALID_PARAMETER,        /**<invalid parameter */
    ERR_MTEFFECT_INVALID_OPERATION,        /**<invalid operation*/
    ERR_MTEFFECT_ADDR_FAULT,               /**<address fault */
    ERR_MTEFFECT_NO_MEM                    /**<mem alloc fail */
};

#define MT_ERR_MTEFFECT_CODE_UNDEF\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_CODE_UNDEF)

#define MT_ERR_MTEFFECT_PTR_NULL\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_PTR_NULL)

#define MT_ERR_MTEFFECT_OPENED\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_OPENED)

#define MT_ERR_MTEFFECT_CLOSED\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_CLOSED)

#define MT_ERR_MTEFFECT_NO_OPEN\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_NO_OPEN)


#define MT_ERR_MTEFFECT_INVALID_DEVICE\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_INVALID_DEVICE)

#define MT_ERR_MTEFFECT_INVALID_HANDLE\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_INVALID_HANDLE)

#define MT_ERR_MTEFFECT_INVALID_PARAMETER\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_INVALID_PARAMETER)

#define MT_ERR_MTEFFECT_INVALID_OPERATION\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_INVALID_OPERATION)

#define MT_ERR_MTEFFECT_ADDR_FAULT\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_ADDR_FAULT)

#define MT_ERR_MTEFFECT_NO_MEM\
    MTEFFECT_DEF_ERR(MTEFFECT_ERR_MODUID, ERR_MTEFFECT_NO_MEM)

/**Screen refresh callback*/
/** CNcomment:屏幕刷新回调函数*/
typedef MT_S32 (*pRefresh_Callback)(mt_void);
typedef enum hiEFFECT_MODE_E
{
    MT_EFFECT_NONE = 0, 			/**<Copy *//**<CNcomment:拷贝显示*/
    MT_EFFECT_TURNPAGE, 			/**<Turnpage *//**<CNcomment:翻页*/
    MT_EFFECT_ROLLPAGE, 			/**<Rollpage *//**<CNcomment:卷轴*/
    MT_EFFECT_VERTICALSHUTTER,		/**<Vertical shutter *//**<CNcomment:垂直百叶窗*/
    MT_EFFECT_HORIZONTALSHUTTER,    /**<Horizontal shutter *//**<CNcomment:水平百叶窗*/
    MT_EFFECT_LEFTIN,				/**<Leftin*//**<CNcomment:从左抽出*/
    MT_EFFECT_TOPIN, 				/**<Topin*//**<CNcomment:从上抽出*/
    MT_EFFECT_TRANSIN, 				/**<Transin*//**<CNcomment:渐进渐出*/
    MT_EFFECT_ROTATE,  				/**<Rotate*//**<CNcomment:螺旋*/
    MT_EFFECT_CENTEROUT,  			/**<Centerout*//**<CNcomment:中央渐出*/
    MT_EFFECT_CENTERIN, 			/**<Centerin*//**<CNcomment:中央渐入*/
} EFFECT_MODE_E;

typedef enum mtEFFECT_SPEED
{
    MT_EFFECT_SPEED_FAST   = 1,
    MT_EFFECT_SPEED_NORMAL = 2,
    MT_EFFECT_SPEED_SLOW = 4,
} EFFECT_SPEED;


/** @} */  /*! <!-- Structure Definition end */

/******************************* API declaration *****************************/
/** \addtogroup      Effect */
/** @{ */  /** <!-- [Effect module] */

/** 
\brief Effect init function. CNcomment:特效初始化函数 CNend

\attention \n
CNcomment: 注册屏幕刷新回调函数并设置显示模式 
bLetfbox 为MT_TRUE, 表示图像比例与屏幕比例不一致时添加黑边 
bLetfbox 为MT_FALSE, 表示图像缩放成全屏,如果图形比例与屏幕比例不一致会导致变形. 
无论那种方式,图像都全屏显示 CNend
\param[in] pfCallBack Screen refresh callback function.CNcomment:屏幕刷新回调函数 CNend
\param[in] bLetfbox Picture dispaly mode.CNcomment:图形显示方式 CNend
\retval ::MT_ERR_MTEFFECT_OPENED
\retval ::MT_ERR_MTEFFECT_PTR_NULL
\retval ::MT_SUCCESS

\see \n
::MT_Effect_Init \n
::MT_Effect_Deinit
*/
extern mt_s32  MT_Effect_Init(pRefresh_Callback pfCallBack, MT_BOOL bLetfbox);

/** 
\brief Effect deinit function. CNcomment:特效去初始化函数 CNend
\attention \n
N/A
\param N/A. CNcomment:无 CNend
\retval ::MT_ERR_MTEFFECT_NO_OPEN
\retval ::MT_SUCCESS
\see \n
::MT_GO_Init \n
::MT_Effect_Deinit
*/
extern mt_s32  MT_Effect_Deinit(mt_void);

/** 
\brief Effect play function. CNcomment:特效播放函数 CNend

\attention \n
CNcomment: 特效播放函数, 该函数为阻塞函数,特效完成才退出
所有surface的像素格式必须保持一致
目前支持的格式:
    MTGO_PF_4444,
    MTGO_PF_0444,
    MTGO_PF_1555,
    MTGO_PF_0555,
    MTGO_PF_565, CNend
\param[in] hNewSurface new surface
\param[in] hScreenSurface Screen suface
\param[in] mode Effect mode.
\retval ::MT_ERR_MTEFFECT_NO_OPEN
\retval ::MT_ERR_MTEFFECT_INVALID_PARAMETER
\retval ::MT_ERR_MTEFFECT_INVALID_OPERATION
\retval ::MT_ERR_MTEFFECT_NO_MEM
\retval ::MT_SUCCESS
\see \n
N/A. CNcomment:无 CNend
*/
extern mt_s32  MT_Effect_Play(MT_HANDLE hNewSurface, MT_HANDLE hScreenSurface, EFFECT_MODE_E mode);


/** 
\brief Effect Set Layer handle function. CNcomment:设置特效句柄 CNend
\attention \n
N/A
\param[in] Layer Layer handle
\retval ::MT_SUCCESS
\see \n
N/A. CNcomment:无 CNend
*/
extern mt_s32 MT_Effect_SetLayer(MT_HANDLE Layer);

/** 
\brief Effect Stop function. CNcomment:特效播放中止,可以提高按键响应速度 CNend
\attention \n
N/A
\param N/A. CNcomment:无 CNend
\retval ::MT_ERR_MTEFFECT_NO_OPEN
\retval ::MT_SUCCESS
\see \n
N/A. CNcomment:无
*/
extern mt_s32  MT_Effect_Stop(mt_void);

/** 
\brief Set Effect Speed function. CNcomment:设置特效播放速度 CNend
\attention \n
N/A
\param[in] eSpeed Effect Speed   
\retval ::MT_ERR_MTEFFECT_NO_OPEN
\retval ::MT_ERR_MTEFFECT_INVALID_PARAMETER
\retval ::MT_SUCCESS
\see \n
N/A. CNcomment:无 CNend
*/
extern mt_s32  MT_Effect_SetSpeed(EFFECT_SPEED eSpeed);

/** @} */  /*! <!-- API declaration end */
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_PEFFECT_H__ */
