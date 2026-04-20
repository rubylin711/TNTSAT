/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_pq.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/02/27
  Description   :
  History       :
  1.Date        : 2017/02/27
    Author      :
    Modification:

*********************************************************************************************/

#ifndef __MT_UNF_PQ_H__
#define __MT_UNF_PQ_H__

#include "mt_type.h"
#include "mt_unf_disp.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */
/******************************************************************************/


/********************************Struct Definition********************************/
/** \addtogroup      PQ */
/** @{ */  /** <!-- [PQ] */

/**<Image Mode Set Different Parameters Under Different Mode*/
/**<CNcomment:图像模式选择；在不同的模式下配置不同的参数 CNend*/
typedef enum hiUNF_PQ_IMAGE_MODE_E
{
    MT_UNF_OPTION_MODE_NATURAL = 0,  /**<Natural mode*/ /**<CNcomment:自然模式 CNend*/
    MT_UNF_OPTION_MODE_PERSON,       /**<Person mode*/ /**<CNcomment:人物模式 CNend*/
    MT_UNF_OPTION_MODE_FILM,         /**<Film mode*/ /**<CNcomment:电影模式 CNend*/
    MT_UNF_OPTION_MODE_UD,           /**<User Defined mode*/ /**<CNcomment:用户自定义模式 CNend*/
    MT_UNF_OPTION_MODE_BUTT,
} MT_UNF_PQ_IMAGE_MODE_E;

/**<HD and SD Channels Parameter*/
/**<CNcomment:高标清通道参数 CNend*/
typedef struct hiUNF_PQ_OPT_CHAN_S
{
    mt_u32       u32Brightness;         /**<Brightness; Range:0~100; recommended: 50 */ /**<CNcomment:亮度；范围: 0~100；推荐值: 50 CNend*/
    mt_u32       u32Contrast;           /**<Contrast; Range:0~100; recommended: 50 */ /**<CNcomment:对比度；范围: 0~100；推荐值: 50 CNend*/
    mt_u32       u32Hue;                /**<HuePlus; Range:0~100; recommended: 50 */ /**<CNcomment:色调；范围: 0~100；推荐值: 50 CNend*/
    mt_u32       u32Saturation;         /**<Saturation; Range:0~100; recommended: 50 */ /**<CNcomment:饱和度；范围: 0~100；推荐值: 50 CNend*/
    mt_u32       u32Colortemperature;   /**<Color temperature mode; Range:0~2 Cold,Middle,Warm */ /**<CNcomment: 色温模式；范围: 0~2 冷色温,中间色温,暖色温 CNend*/
    mt_u32       u32GammaMode;          /**<Gamma mode; Range:0~4 Intension_1.8,Intension_2.5,Curve_Light,Curve_Dark, Disabled */ /**<CNcomment:伽马模式选择；范围: 0~4 1.8强度,2.5强度,亮,暗,关闭 CNend*/
    mt_u32       u32DynamicContrast;    /**<Color Dynamic Contrast mode; Range:0~3 Low, Middle,High,Disabled */ /**<CNcomment:动态对比度；范围: 0~3 低,中,高,关闭 CNend*/
    mt_u32       u32IntelligentColor;   /**<Color Intelligent Color; Range:0~5 Blue,Green,BG,Skin,Vivid,Disabled */ /**<CNcomment:智能颜色管理；范围: 0~5 蓝色,绿色,蓝绿增强,肤色,生动,关闭 CNend*/
} MT_UNF_PQ_OPT_CHAN_S;

/**<Common Parameter*/
/**<CNcomment:通用参数 CNend*/
typedef struct hiUNF_PQ_OPT_COMMON_S
{
    mt_u32       u32Sharpeness;      /**<Sharpeness; Range:0~100; recommended: 50 */ /**<CNcomment:锐化强度；范围:0~100；推荐值:50 CNend*/
    mt_u32       u32Denoise;         /**<Denoise; Range:0~1 Disabled,Auto */ /**<CNcomment:降噪使能；范围:0~1 关闭,打开 CNend*/
    mt_u32       u32FilmMode;        /**<FileMode; Range:0~1 Disabled,Auto */ /**<CNcomment:电影模式使能选择；范围:0~1 关闭,打开 CNend*/

} MT_UNF_PQ_OPT_COMMON_S;

/**<Interface Type and Interface Ialue*/
/**<CNcomment:接口类型和通道参数 CNend*/
typedef struct hiUNF_PQ_OPT_CHANS_S
{
    MT_UNF_DISP_E enChan;               /**<interface type */ /**<CNcomment:接口类型 CNend*/
    MT_UNF_PQ_OPT_CHAN_S stChanOpt;     /**<interface value*/ /**<CNcomment:通道参数 CNend*/
} MT_UNF_PQ_OPT_CHANS_S;


/******************************************************************************/

/* 卖场模式 */
typedef enum hiUNF_PQ_DEMO_E
{
     MT_UNF_PQ_DEMO_SHARPNESS = 0,
     MT_UNF_PQ_DEMO_DCI,
     MT_UNF_PQ_DEMO_COLOR,
     MT_UNF_PQ_DEMO_SR,
     MT_UNF_PQ_DEMO_ALL,

     MT_UNF_PQ_DEMO_BUTT
}  MT_UNF_PQ_DEMO_E;

/* PQ模块*/
typedef enum hiUNF_PQ_MODULE_E
{
     MT_UNF_PQ_MODULE_SHARPNESS = 0,
     MT_UNF_PQ_MODULE_DCI,
     MT_UNF_PQ_MODULE_COLOR,
     MT_UNF_PQ_MODULE_SR,
    MT_UNF_PQ_MODULE_ALL,

     MT_UNF_PQ_MODULE_BUTT
}  MT_UNF_PQ_MODULE_E;



/*SR演示模式*/
typedef enum hiUNF_PQ_SR_DEMO_E
{
     MT_UNF_PQ_SR_DISABLE  = 0,//关掉SR,只ZME
     MT_UNF_PQ_SR_ENABLE_R,  //  右边SR
     MT_UNF_PQ_SR_ENABLE_L,  //左边SR
     MT_UNF_PQ_SR_ENABLE_A,  //全屏

     MT_UNF_PQ_SR_DEMO_BUTT
}MT_UNF_PQ_SR_DEMO_E;


/*六基色增强参数*/
typedef struct  hiUNF_PQ_SIX_BASE_S
{
    mt_u32  u32Red;       //范围:0~100
    mt_u32  u32Green;     //范围:0~100
    mt_u32  u32Blue;      //范围:0~100

    mt_u32  u32Cyan;      //范围:0~100
    mt_u32  u32Magenta;   //范围:0~100
    mt_u32  u32Yellow;    //范围:0~100
}MT_UNF_PQ_SIX_BASE_S;


/*肤色增益参数*/
typedef enum hiUNF_PQ_FLESHTONE_E
{
     MT_UNF_PQ_FLESHTONE_GAIN_OFF = 0,
     MT_UNF_PQ_FLESHTONE_GAIN_LOW,
     MT_UNF_PQ_FLESHTONE_GAIN_MID,
     MT_UNF_PQ_FLESHTONE_GAIN_MTGH,

     MT_UNF_PQ_FLESHTONE_GAIN_BUTT
}  MT_UNF_PQ_FLESHTONE_E;


/*颜色增强类型*/
typedef enum hiUNF_PQ_COLOR_ENHANCE_E
{
    MT_UNF_PQ_COLOR_ENHANCE_FLESHTONE = 0,    //肤色增强
    MT_UNF_PQ_COLOR_ENHANCE_SIX_BASE,         //六基色增强,自定义颜色的增强
    MT_UNF_PQ_COLOR_ENHANCE_SPEC_COLOR_MODE,  //固定模式的颜色增强模式
    MT_UNF_PQ_COLOR_ENHANCE_BUTT
} MT_UNF_PQ_COLOR_ENHANCE_E;


/*颜色增强类型*/
typedef enum hiUNF_PQ_COLOR_SPEC_MODE_E
{
    MT_UNF_PQ_COLOR_MODE_RECOMMEND = 0, //推荐的颜色增强模式
    MT_UNF_PQ_COLOR_MODE_BLUE,          //固定的蓝色增强模式
    MT_UNF_PQ_COLOR_MODE_GREEN,         //固定的绿色增强模式
    MT_UNF_PQ_COLOR_MODE_BG,            //固定的蓝绿色增强模式
    MT_UNF_PQ_COLOR_MODE_BUTT
} MT_UNF_PQ_COLOR_SPEC_MODE_E;


/*颜色增强参数*/
typedef struct  hiUNF_PQ_COLOR_ENHANCE_S
{
    MT_UNF_PQ_COLOR_ENHANCE_E    enColorEnhanceType;   //色彩增强类型
    union
    {
        MT_UNF_PQ_FLESHTONE_E    enFleshtone;        //肤色增强参数
        MT_UNF_PQ_SIX_BASE_S     stSixBase;          //六基色增强参数
        MT_UNF_PQ_COLOR_SPEC_MODE_E   enColorMode;   //固定的颜色增强模式

    } unColorGain;
} MT_UNF_PQ_COLOR_ENHANCE_S;



/******************************************************************************/
/**
 \brief Set PQ mode . CNcomment: 设置图像模式 CNend
 \attention \n
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] enImageMode Destination DISP channel PQ mode CNcomment: 目标通道图像模式 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_PQ_SetImageMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_IMAGE_MODE_E enImageMode);

/**
 \brief Get PQ mode . CNcomment: 获取图像模式 CNend
 \attention \n
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] penImageMode  pointer of image mode CNcomment: 指针类型，指向图像模式 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_PQ_GetImageMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_IMAGE_MODE_E* penImageMode);

/**
 \brief Init PQ mode . CNcomment: 初始化图像模式 CNend
 \attention \n
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] penImageMode  pointer of image mode CNcomment: 指针类型，指向图像模式 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_InitImageMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_IMAGE_MODE_E enImageMode);

/**
 \brief Set channel option. CNcomment: 设置通道option值 CNend
 \attention \n
 \param[in] pstChanOption pointer of channel option CNcomment: 指针类型，指向通道option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_SetChanOption(const MT_UNF_PQ_OPT_CHANS_S* pstChanOption);

/**
 \brief Set channel option. CNcomment: 获取通道option值 CNend
 \attention \n
 \param[out] pstChanOption pointer of channel option CNcomment: 指针类型，指向通道option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_GetChanOption(MT_UNF_PQ_OPT_CHANS_S * pstChanOption);

/**
 \brief Set channel common option. CNcomment: 设置通道 common option值 CNend
 \attention \n
 \param[in] pstCommOption pointer of channel common option CNcomment: 指针类型，指向通道common option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_SetCommOption(const MT_UNF_PQ_OPT_COMMON_S* pstCommOption);

/**
 \brief Set channel option. CNcomment: 获取通道common option值 CNend
 \attention \n
 \param[out] pstChanOption pointer of channel option CNcomment: 指针类型，指向通道common option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_GetCommOption(MT_UNF_PQ_OPT_COMMON_S* pstCommOption);


/**
 \brief Modifies the basic configuration information.  CNcomment:更新PQ配置区信息 CNend
	\attention \n
	\param[in] N/A CNcomment: 无 CNend
	\retval ::MT_SUCCESS Success CNcomment: 成功 CNend
	\see \n
 N/A CNcomment: 无 CNend
	*/

mt_s32 MT_UNF_PQ_UpdatePqParam(mt_void);

/**
 \brief Set the default PQ configuration for video parameter test.  CNcomment: 为入网指标测试设置PQ 的默认值CNend
	\attention \n
	\param[in] N/A CNcomment: 无 CNend
	\retval ::MT_SUCCESS Success CNcomment: 成功 CNend
	\see \n
 N/A CNcomment: 无 CNend
	*/

mt_s32 MT_UNF_PQ_SetDefaultParam(mt_void);


/******************************************************************************/


/** \addtogroup      PQ */
/** @{ */  /** <!-- 【PQ】 */

/**
 \brief Initializes the picture quality unit (PQU). CNcomment: 初始化PQ CNend
 \attention \n
Before calling the PQU, you must call this application programming interface (API).
CNcomment: 调用PQ模块要求首先调用该接口 CNend
 \param[in] N/A CNcomment: 无 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \retval ::MT_ERR_PQ_DEV_NOT_EXIST No PQ device exists. CNcomment: 设备不存在 CNend
 \retval ::MT_ERR_PQ_NOT_DEV_FILE The file is not a PQ file. CNcomment: 文件非设备 CNend
 \retval ::MT_ERR_PQ_DEV_OPENED The PQ device fails to be started. CNcomment: 打开设备失败 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

extern mt_s32 MT_UNF_PQ_Init(mt_void);

/**
 \brief Deinitializes the PQU. CNcomment: 去初始化PQ CNend
 \attention \n
 \param[in] N/A CNcomment: 无 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

extern mt_s32 MT_UNF_PQ_DeInit(mt_void);

/**
 \brief 获取亮度
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] pu32Brightness：亮度值,有效范围: 0~100;
 \param[out]

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetBrightness(MT_UNF_DISP_E enChan, mt_u32 *pu32Brightness);

/**
 \brief 设置亮度
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] u32Brightness：亮度值,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetBrightness(MT_UNF_DISP_E enChan, mt_u32 u32Brightness);

/**
 \brief 获取对比度
 \attention \n
无

 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] pu32Contrast：对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetContrast(MT_UNF_DISP_E enChan, mt_u32 *pu32Contrast);

/**
 \brief 设置对比度
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] u32Contrast：对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetContrast(MT_UNF_DISP_E enChan, mt_u32 u32Contrast);

/**
 \brief 获取饱和度
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] pu32Saturation：饱和度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetSaturation(MT_UNF_DISP_E enChan, mt_u32 *pu32Saturation);

/**
 \brief 设置饱和度
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] u32Saturation：饱和度,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetSaturation(MT_UNF_DISP_E enChan, mt_u32 u32Saturation);

/**
 \brief 获取降噪强度
 \attention \n
无

 \param[out] pu32NRLevel: 降噪等级, 有效范围: 0~255


 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetNR(MT_UNF_DISP_E enChan, mt_u32* pu32NRLevel);

/**
 \brief 设置降噪强度
 \attention \n
无

 \param[in] u32NRLevel: 降噪等级, 有效范围: 0~255

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetNR(MT_UNF_DISP_E enChan, mt_u32 u32NRLevel);

/**
 \brief 获取自动降噪开关状态
 \attention \n
无

 \param[out] pu32OnOff


 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetNRAutoMode(MT_UNF_DISP_E enChan, mt_u32* pu32OnOff);

/**
 \brief 设置降噪自动模式开关
 \attention \n
无

 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetNRAutoMode(MT_UNF_DISP_E enChan, mt_u32 u32OnOff);

/**
 \brief 获取色调
 \attention \n
无

 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] pu32Hue：色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetHue(MT_UNF_DISP_E enChan, mt_u32 *pu32Hue);

/**
 \brief 设置色调
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] u32Hue：色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetHue(MT_UNF_DISP_E enChan, mt_u32 u32Hue);



/**
 \brief 获取SR演示类型
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] *penType


 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetSRMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_SR_DEMO_E *penType);

/**
 \brief 设置SR演示类型
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] enType

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetSRMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_SR_DEMO_E enType);

/**
 \brief 获取清晰度
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] pu32Sharpness：清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetSharpness(MT_UNF_DISP_E enChan, mt_u32 *pu32Sharpness);

/**
 \brief 设置清晰度
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] u32Sharpness：清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetSharpness(MT_UNF_DISP_E enChan, mt_u32 u32Sharpness);

/**
 \brief 获取块降噪De-blocking强度
 \attention \n
无

 \param[out] pu32DBlevel：降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetDeBlocking(MT_UNF_DISP_E enChan, mt_u32* pu32DBlevel);

/**
 \brief 设置块降噪De-blocking强度
 \attention \n
无

 \param[in] u32DBlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetDeBlocking(MT_UNF_DISP_E enChan, mt_u32 u32DBlevel);

/**
 \brief 获取去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[out] pu32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetDeRinging(MT_UNF_DISP_E enChan, mt_u32* pu32DRlevel);

/**
 \brief 设置去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[in] u32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetDeRinging(MT_UNF_DISP_E enChan, mt_u32 u32DRlevel);

/**
 \brief 获取颜色增强
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] pu32ColorGainLevel

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetColorGain(MT_UNF_DISP_E enChan, mt_u32 *pu32ColorGainLevel);

/**
 \brief 设置颜色增强
 \attention \n
无
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] u32ColorGainLevel，范围0-100

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetColorGain(MT_UNF_DISP_E enChan, mt_u32 u32ColorGainLevel);

/**
 \brief 获取肤色增强
 \attention \n
  无

 \param[out] pu32FleshToneLevel

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetFleshTone(MT_UNF_DISP_E enChan, MT_UNF_PQ_FLESHTONE_E* pu32FleshToneLevel);

/**
 \brief 设置肤色增强
 \attention \n
  无

 \param[in] enFleshToneLevel，参考MT_COLOR_GAIN_E

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetFleshTone(MT_UNF_DISP_E enChan, MT_UNF_PQ_FLESHTONE_E enFleshToneLevel);

/**
 \brief 获取颜色增强的类型和强度
 \attention \n
无

 \param[out] pstColorEnhanceParam:颜色增强的类型和强度;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetColorEnhanceParam(MT_UNF_PQ_COLOR_ENHANCE_S *pstColorEnhanceParam);

/**
 \brief 设置颜色增强的类型和强度
 \attention \n
无

 \param[out] enColorEnhanceType:颜色增强的类型和强度;

 \retval ::MT_SUCCESS

 */

 extern mt_s32 MT_UNF_PQ_SetColorEnhanceParam(MT_UNF_PQ_COLOR_ENHANCE_S stColorEnhanceParam);

/**
 \brief 获取DCI（动态对比度增强）的强度范围
 \attention \n
无

 \param[out] pu32DCIlevel:动态对比度等级, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetDynamicContrast(mt_u32 *pu32DCIlevel);

/**
 \brief 设置DCI（动态对比度增强）的强度范围
 \attention \n
无

 \param[in] u32DCIlevel:动态对比度等级, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetDynamicContrast(mt_u32 u32DCIlevel);



/**
 \brief 设置PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetPQModule( MT_UNF_PQ_MODULE_E enFlags, mt_u32 u32OnOff);

/**
 \brief 获取PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] pu32OnOff

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_GetPQModule( MT_UNF_PQ_MODULE_E enFlags, mt_u32 *pu32OnOff);

/**
 \brief 设置卖场模式开关
 \attention \n
无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_UNF_PQ_SetDemo( MT_UNF_PQ_DEMO_E enFlags, mt_u32 u32OnOff);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* End of #ifndef __MT_UNF_PQ_H__ */


