/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_PQ_H__
#define __DRV_PQ_H__

#include "mt_osal.h"
#include "mt_debug.h"
#include "mt_type.h"

#include "mt_drv_pq.h"
#include "mt_drv_proc.h"
#include "mt_drv_mem.h"
#include "mt_drv_mmz.h"
#include "mt_drv_module.h"

#include "drv_pq_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PQ_NAME            "MT_PQ"

#define NUM2LEVEL(Num)   ((Num) * 100 + 127) / 255
#define LEVEL2NUM(Level) ((Level) * 255 + 50) / 100

/*亮度/对比度/色调/饱和度设定*/
typedef struct mtPICTURE_SETTING_S
{
    mt_u16 u16Brightness;
    mt_u16 u16Contrast;
    mt_u16 u16Hue;
    mt_u16 u16Saturation;
} PICTURE_SETTING_S;

/*色温设定*/
typedef struct mtCOLOR_TEMPERATURE_S
{
    mt_s16 s16RedGain;
    mt_s16 s16GreenGain;
    mt_s16 s16BlueGain;
    mt_s16 s16RedOffset;
    mt_s16 s16GreenOffset;
    mt_s16 s16BlueOffset;
} COLOR_TEMPERATURE_S;

/*色彩空间标准*/
typedef enum mtCOLOR_SPACE_TYPE_E
{
    OPTM_CS_eUnknown = 0           ,
    OPTM_CS_eItu_R_BT_709 = 1      ,
    OPTM_CS_eFCC = 4               ,
    OPTM_CS_eItu_R_BT_470_2_BG = 5 ,
    OPTM_CS_eSmpte_170M = 6        ,
    OPTM_CS_eSmpte_240M = 7        ,
    OPTM_CS_eXvYCC_709 = OPTM_CS_eItu_R_BT_709,
    OPTM_CS_eXvYCC_601 = 8         ,
    OPTM_CS_eRGB = 9               ,
    
    OPTM_CS_BUTT
} COLOR_SPACE_TYPE_E;

/*色彩空间设定*/
typedef struct mtCOLOR_SPACE_S
{
    COLOR_SPACE_TYPE_E u16InputColorSpace;    /*输入色彩空间*/
    COLOR_SPACE_TYPE_E u16OutputColorSpace;   /*输出色彩空间*/
    MT_BOOL            bFullRange;            /*0:limit,1:full range*/
} COLOR_SPACE_S;

/*用户PQ 数据结构*/
typedef struct  hiDRV_PQ_PARAM_S
{
    PICTURE_SETTING_S   stSDPictureSetting;
    PICTURE_SETTING_S   stHDPictureSetting;
    COLOR_TEMPERATURE_S stColorTemp;
    MT_PQ_DCI_WIN_S     stDciWin;
    mt_u32  u32NRLevel;
    mt_u32  u32Sharpness;
    mt_u32  u32DBLevel;
    mt_u32  u32DRLevel;
    mt_u32  u32ColorGainLevel;
    mt_u32  u32DCILevelGain;
    mt_u32  u323dSharpen;
    mt_u32  u32NrAuto;
    MT_BOOL bDemoOnOff[MT_PQ_DEMO_BUTT];
    MT_BOOL bModuleOnOff[MT_PQ_MODULE_BUTT];
    MT_PQ_COLOR_ENHANCE_S stColorEnhance;
} MT_DRV_PQ_PARAM_S;


/**
 \brief 去初始化客户PQ模块;
 \attention \n
  无

 \param[in]

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_Comsumer_DeInit(mt_void);

/**
 \brief 初始化客户PQ模块;
 \attention \n
  无

 \param[in] pszPath: PQ Table文件路径, 如果pszPath参数为空指针, 会采用PQ SDK内部默认参数;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_Comsumer_Init(mt_char* pszPath);

/**
 \brief 显示PQ状态信息
 \attention \n
无

 \param[in] *s;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_ProcRead(struct seq_file* s, mt_void* data);

/**
 \brief 获取标清亮度
 \attention \n
无

 \param[out] pu32Brightness 亮度值,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDBrightness(mt_u32* pu32Brightness);

/**
 \brief 设置标清亮度
 \attention \n
无

 \param[in] u32Brightness, 亮度值,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSDBrightness(mt_u32 u32Brightness);

/**
 \brief 获取标清对比度
 \attention \n
无

 \param[out] pu32Contrast 对比度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDContrast(mt_u32* pu32Contrast);

/**
 \brief 设置标清对比度
 \attention \n
无

 \param[in] u32Contrast, 对比度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSDContrast(mt_u32 u32Contrast);

/**
 \brief 获取标清色调
 \attention \n
无

 \param[out] pu32Hue  色调, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDHue(mt_u32* pu32Hue);

/**
 \brief 设置标清色调
 \attention \n
无

 \param[in] u32Hue   色调, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSDHue(mt_u32 u32Hue);

/**
 \brief 获取标清饱和度
 \attention \n
无

 \param[out] pu32Saturation  饱和度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDSaturation(mt_u32* pu32Saturation);

/**
 \brief 设置标清饱和度
 \attention \n
无

 \param[in] u32Saturation 饱和度,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSDSaturation(mt_u32 u32Saturation);

/**
 \brief 获取高清亮度
 \attention \n
无

 \param[out] pu32Brightness 亮度值,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetHDBrightness(mt_u32* pu32Brightness);

/**
 \brief 设置高清亮度
 \attention \n
无

 \param[in] u32Brightness, 亮度值,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetHDBrightness(mt_u32 u32Brightness);

/**
 \brief 获取高清对比度
 \attention \n
无

 \param[out] pu32Contrast 对比度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetHDContrast(mt_u32* pu32Contrast);

/**
 \brief 设置高清对比度
 \attention \n
无

 \param[in] u32Contrast, 对比度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetHDContrast(mt_u32 u32Contrast);

/**
 \brief 获取高清色调
 \attention \n
无

 \param[out] pu32Hue  色调, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetHDHue(mt_u32* pu32Hue);

/**
 \brief 设置高清色调
 \attention \n
无

 \param[in] u32Hue   色调, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetHDHue(mt_u32 u32Hue);

/**
 \brief 获取高清饱和度
 \attention \n
无

 \param[out] pu32Saturation  饱和度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetHDSaturation(mt_u32* pu32Saturation);

/**
 \brief 设置高清饱和度
 \attention \n
无

 \param[in] u32Saturation 饱和度,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetHDSaturation(mt_u32 u32Saturation);

/**
 \brief 获取清晰度
 \attention \n
无

 \param[out] pu32Sharpness  清晰度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSharpness(mt_u32* pu32Sharpness);

/**
 \brief 设置清晰度
 \attention \n
无

 \param[in] u32Sharpness, 清晰度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSharpness(mt_u32 u32Sharpness);


/**
 \brief 获取色温参数
 \attention \n
无

 \param[in] pstColorTemp: 色温属性

 \retval::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetColorTemp(MT_PQ_COLOR_TEMP_S* pstColorTemp);

/**
 \brief 设置色温参数
 \attention \n
无

 \param[out] pstColorTemp: 色温属性

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetColorTemp(MT_PQ_COLOR_TEMP_S* pstColorTemp);

/**
 \brief 获取降噪强度
 \attention \n
无

 \param[out] pu32NRLevel: 降噪等级, 有效范围: 0~255


 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetNRLevel(mt_u32* pu32NRLevel);



/**
 \brief 获取块降噪De-blocking强度
 \attention \n
无

 \param[out] *pu32DBLevel: 降噪等级, 有效范围: 0~255


 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetDeBlocking(mt_u32* pu32DBLevel);

/**
 \brief 设置块降噪de-blocking强度
 \attention \n
无

 \param[in] u32DBLevel: 降噪等级, 有效范围: 0~255

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetDeBlocking(mt_u32 u32DBLevel);

/**
 \brief 获取去纹躁de-ringing强度
 \attention \n
无

 \param[out] *pu32DBLevel: 降噪等级, 有效范围: 0~255


 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetDeRinging(mt_u32* pu32DRLevel);

/**
 \brief 设置去纹躁de-ringing强度
 \attention \n
无

 \param[in] u32DBLevel: 降噪等级, 有效范围: 0~255

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetDeRinging(mt_u32 u32DRLevel);

/**
 \brief 获取颜色增强
 \attention \n
无

 \param[out] pu32ColorGainLevel

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetColorEhance(mt_u32* pu32ColorGainLevel);

/**
 \brief 设置颜色增强
 \attention \n
无

 \param[in] u32ColorGainLevel

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetColorEhance(mt_u32 u32ColorGainLevel);

/**
 \brief 获取肤色增强
 \attention \n
  无

 \param[out] pu32FleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetFleshToneLevel(mt_u32* pu32FleshToneLevel);

/**
 \brief 设置肤色增强
 \attention \n
  无

 \param[in] enFleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetFleshToneLevel(MT_PQ_FLESHTONE_E enFleshToneLevel);

/**
 \brief 设置DCI强度增益等级
 \attention \n
无

 \param[in] none;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetDCILevelGain(mt_u32 u32DCILevelGain);

/**
 \brief 设置DCI配置曲线
 \attention \n
无

 \param[in] pstDciCoef;

 \retval ::MT_SUCCESS

 */
 
#if 0 //janny
mt_s32 DRV_PQ_SetDCIWgtLut(DCI_WGT_S* pstDciCoef);
#endif

/**
 \brief 获取NR 自动开关状态
 \attention \n
  无

 \param[out] pu32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetNrAutoMode(mt_u32* pu32OnOff);

/**
 \brief 设置NR自动开关
 \attention \n
  无

 \param[in] u32OnOff 开关

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetNrAutoMode(mt_u32 u32OnOff);

/**
 \brief 获取3D Sharpness 模式状态
 \attention \n
  无

 \param[out] pu32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_Get3DSharpMode(mt_u32* pu32OnOff);

/**
 \brief 设置3D Sharpness 模式
 \attention \n
  无

 \param[in] u32OnOff 开关

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_Set3DSharpMode(mt_u32 u32OnOff);

/**
 \brief 获取各通道的CSC(用于调试)
 \attention \n
无

 \param[in] enDisplayId
 \param[out] pstCSCMode

 \retval ::MT_SUCCESS

 */
 #if 0 //janny
mt_s32 DRV_PQ_GetCSCMode(MT_PQ_CSC_ID_E enDisplayId, MT_PQ_VDP_CSC_S* pstCSCMode);
#endif

/**
 \brief 设置卖场模式开关
 \attention \n
无

 \param[in] enFlags

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetDemoMode(MT_PQ_DEMO_E enFlags, MT_BOOL bOnOff);

/**
 \brief 获取PQ模块开关状态
 \attention \n
  无

 \param[in] enFlags
 \param[out] *pu32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetPQModule(MT_PQ_MODULE_E enFlags, mt_u32* pu32OnOff);

/**
 \brief 设置PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetPQModule(MT_PQ_MODULE_E enFlags, mt_u32 u32OnOff);

/**
 \brief 获取SR演示模式开关
 \attention \n
无

 \param[in] ps32Type

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSRMode(mt_s32* ps32Type);

/**
 \brief 设置SR演示模式开关
 \attention \n
无

 \param[in] eSRMode: SR演示模式0-只ZME;1-右边SR；2-左边SR；3-SR开

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSRMode(MT_PQ_SR_DEMO_E eSRMode);

/**
 \brief 获取PQBin 的物理地址
 \attention \n
无

 \param[in] pu32Addr:

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetBinPhyAddr(mt_u32* pu32Addr);

/**
 \brief 设置ACM 的Luma 曲线查找表
 \attention \n
无

 \param[in] pstAttr:

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetAcmLuma(MT_PQ_ACM_LUT_S* pstAttr);

/**
 \brief 设置ACM 的Hue 曲线查找表
 \attention \n
无

 \param[in] pstAttr:

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetAcmHue(MT_PQ_ACM_LUT_S* pstAttr);

/**
 \brief 设置ACM 的Sat 曲线查找表
 \attention \n
无

 \param[in] pstAttr:

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetAcmSat(MT_PQ_ACM_LUT_S* pstAttr);

/**
 \brief 六基色控制设置
 \attention \n
  无

 \param[in] enSixColorType;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetSixBaseColorLevel(MT_PQ_SIX_BASE_S* pstSixBaseColorOffset);

/**
 \brief 颜色增强模式设置
 \attention \n
  无

 \param[in] enColorSpecMode 0-RECOMMEND;1-BLUE;2-GREEN;3-BG;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetColorEnhanceMode(MT_PQ_COLOR_SPEC_MODE_E enColorSpecMode);


mt_s32 DRV_PQ_SetReg(MT_PQ_REGISTER_S* pstAttr);

mt_s32 DRV_PQ_GetReg(MT_PQ_REGISTER_S* pstAttr);

mt_s32 DRV_PQ_GetPqParam(mt_void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __DRV_PQ_H__ */
