/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_pq.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/02/27
  Description   :
  History       :
  1.Date        : 2017/02/27
    Author      : 
    Modification: 

*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_type.h"
#include "mt_unf_pq.h"
#include "mt_mpi_pq.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/**
 \brief 初始化PQ
 \attention \n
无

 \param[in] pszPath:PQ配置文件路径

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_Init(mt_void)
{
    mt_char* pszPath = MT_NULL;
    return MT_MPI_PQ_Init(pszPath);
}


/**
 \brief 去初始化PQ
 \attention \n
无

 \param[in] none

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_DeInit(mt_void)
{
    return MT_MPI_PQ_DeInit();
}


/**
 \brief Set PQ mode . CNcomment: 设置图像模式 CNend
 \attention \n
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[in] enImageMode Destination DISP channel PQ mode CNcomment: 目标通道图像模式 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_PQ_SetImageMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_IMAGE_MODE_E enImageMode)
{

    return MT_SUCCESS;
}


/**
 \brief Get PQ mode . CNcomment: 获取图像模式 CNend
 \attention \n
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] penImageMode  pointer of image mode CNcomment: 指针类型，指向图像模式 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_PQ_GetImageMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_IMAGE_MODE_E* penImageMode)
{
    if (NULL == penImageMode )
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/**
 \brief Init PQ mode . CNcomment: 初始化图像模式 CNend
 \attention \n
 \param[in] enChan Destination DISP channel CNcomment: 目标通道号 CNend
 \param[out] penImageMode  pointer of image mode CNcomment: 指针类型，指向图像模式 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_InitImageMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_IMAGE_MODE_E enImageMode)
{

    return MT_SUCCESS;
}


/**
 \brief Set channel option. CNcomment: 设置通道option值 CNend
 \attention \n
 \param[in] pstChanOption pointer of channel option CNcomment: 指针类型，指向通道option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_SetChanOption(const MT_UNF_PQ_OPT_CHANS_S* pstChanOption)
{
    MT_UNF_DISP_E enChan;

    if (NULL == pstChanOption )
    {
        return MT_FAILURE;
    }

    enChan = pstChanOption->enChan;

    MT_MPI_PQ_SetBrightness((MT_DRV_DISPLAY_E)enChan, pstChanOption->stChanOpt.u32Brightness );
    MT_MPI_PQ_SetContrast((MT_DRV_DISPLAY_E)enChan, pstChanOption->stChanOpt.u32Contrast );
    MT_MPI_PQ_SetHue((MT_DRV_DISPLAY_E)enChan, pstChanOption->stChanOpt.u32Hue);
    MT_MPI_PQ_SetSaturation((MT_DRV_DISPLAY_E)enChan, pstChanOption->stChanOpt.u32Saturation);

    return MT_SUCCESS;
}


/**
 \brief Set channel option. CNcomment: 获取通道option值 CNend
 \attention \n
 \param[out] pstChanOption pointer of channel option CNcomment: 指针类型，指向通道option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_GetChanOption(MT_UNF_PQ_OPT_CHANS_S* pstChanOption)
{
    MT_UNF_DISP_E enChan;
    mt_u32 u32Brightness = 0;
    mt_u32 u32Contrast = 0;
    mt_u32 u32Hue = 0;
    mt_u32 u32Saturation = 0;

    if (NULL == pstChanOption )
    {
        return MT_FAILURE;
    }

    enChan = pstChanOption->enChan;

    MT_MPI_PQ_GetBrightness((MT_DRV_DISPLAY_E)enChan, &u32Brightness);
    MT_MPI_PQ_GetContrast((MT_DRV_DISPLAY_E)enChan, &u32Contrast);
    MT_MPI_PQ_GetHue((MT_DRV_DISPLAY_E)enChan, &u32Hue);
    MT_MPI_PQ_GetSaturation((MT_DRV_DISPLAY_E)enChan, &u32Saturation);

    pstChanOption->stChanOpt.u32Brightness = u32Brightness;
    pstChanOption->stChanOpt.u32Contrast = u32Contrast;
    pstChanOption->stChanOpt.u32Hue = u32Hue;
    pstChanOption->stChanOpt.u32Saturation = u32Saturation;
    pstChanOption->stChanOpt.u32Colortemperature = 0;
    pstChanOption->stChanOpt.u32GammaMode = 0;
    pstChanOption->stChanOpt.u32DynamicContrast = 0;
    pstChanOption->stChanOpt.u32IntelligentColor = 0;

    return MT_SUCCESS;
}


/**
 \brief Set channel common option. CNcomment: 设置通道 common option值 CNend
 \attention \n
 \param[in] pstCommOption pointer of channel common option CNcomment: 指针类型，指向通道common option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_SetCommOption(const MT_UNF_PQ_OPT_COMMON_S* pstCommOption)
{

    if (NULL == pstCommOption )
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetSharpness(pstCommOption->u32Sharpeness);
}


/**
 \brief Set channel option. CNcomment: 获取通道common option值 CNend
 \attention \n
 \param[out] pstChanOption pointer of channel option CNcomment: 指针类型，指向通道common option值 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
N/A CNcomment: 无 CNend
 */

mt_s32 MT_UNF_PQ_GetCommOption(MT_UNF_PQ_OPT_COMMON_S* pstCommOption)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Sharpness = 0;


    if ( NULL == pstCommOption)
    {
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_PQ_GetSharpness(&u32Sharpness);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    pstCommOption->u32Sharpeness = u32Sharpness;
    pstCommOption->u32Denoise = 0;
    pstCommOption->u32FilmMode = 0;

    return s32Ret;
}



/**
 \brief Modifies the basic configuration information.  CNcomment:更新PQ配置区信息 CNend
 \attention \n
 \param[in] N/A CNcomment: 无 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
 N/A CNcomment: 无 CNend
	*/

mt_s32 MT_UNF_PQ_UpdatePqParam(mt_void)
{

    return MT_SUCCESS;
}


/**
 \brief Set the default PQ configuration for video parameter test.  CNcomment: 为入网指标测试设置PQ 的默认值CNend
 \attention \n
 \param[in] N/A CNcomment: 无 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \see \n
 N/A CNcomment: 无 CNend
	*/

mt_s32 MT_UNF_PQ_SetDefaultParam(mt_void)
{
    mt_s32 s32Ret;
    mt_u32 u32OnOff = 0;

    s32Ret = MT_MPI_PQ_SetBrightness(MT_DRV_DISPLAY_0, 50);
    s32Ret |= MT_MPI_PQ_SetBrightness(MT_DRV_DISPLAY_1, 50);
    s32Ret |= MT_MPI_PQ_SetContrast(MT_DRV_DISPLAY_0, 50);
    s32Ret |= MT_MPI_PQ_SetContrast(MT_DRV_DISPLAY_1, 50);
    s32Ret |= MT_MPI_PQ_SetSaturation(MT_DRV_DISPLAY_0, 50);
    s32Ret |= MT_MPI_PQ_SetSaturation(MT_DRV_DISPLAY_1, 50);
    s32Ret |= MT_MPI_PQ_SetHue(MT_DRV_DISPLAY_0, 50);
    s32Ret |= MT_MPI_PQ_SetHue(MT_DRV_DISPLAY_1, 50);

    s32Ret |= MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_SHARPNESS, u32OnOff);
    s32Ret |= MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_DCI, u32OnOff);
    s32Ret |= MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_COLOR, u32OnOff);
    s32Ret |= MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_SR, u32OnOff);

    return s32Ret;
}


/**
 \brief 获取亮度
 \attention \n
无

 \param[in] pu32Brightness 亮度值,有效范围: 0~100;
 \param[out]

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetBrightness(MT_UNF_DISP_E enChan, mt_u32* pu32Brightness)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Brightness = 0;

    if ( NULL == pu32Brightness)
    {
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_PQ_GetBrightness((MT_DRV_DISPLAY_E)enChan, &u32Brightness);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    *pu32Brightness = u32Brightness;
    return s32Ret;
}

/**
 \brief 设置亮度
 \attention \n
无

 \param[in] u32Brightness, 亮度值,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetBrightness(MT_UNF_DISP_E enChan, mt_u32 u32Brightness)
{
    if ( u32Brightness > 100)
    {
        MT_ERR_PQ("The brightness is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetBrightness((MT_DRV_DISPLAY_E)enChan, u32Brightness);
}


/**
 \brief 获取对比度
 \attention \n
无

 \param[in]
 \param[out] pu32Contrast 对比度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetContrast(MT_UNF_DISP_E enChan, mt_u32* pu32Contrast)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Contrast = 0;

    if ( NULL == pu32Contrast)
    {
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_PQ_GetContrast((MT_DRV_DISPLAY_E)enChan, &u32Contrast);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    *pu32Contrast = u32Contrast;
    return s32Ret;
}

/**
 \brief 设置对比度
 \attention \n
无

 \param[in] u32Contrast, 对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetContrast(MT_UNF_DISP_E enChan, mt_u32 u32Contrast)
{
    if ( u32Contrast > 100)
    {
        MT_ERR_PQ("The Contrast is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetContrast((MT_DRV_DISPLAY_E)enChan, u32Contrast);
}

/**
 \brief 获取色调
 \attention \n
无

 \param[in]
 \param[out] pu32Hue：色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetHue(MT_UNF_DISP_E enChan, mt_u32* pu32Hue)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Hue = 0;

    if ( NULL == pu32Hue)
    {
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_PQ_GetHue((MT_DRV_DISPLAY_E)enChan, &u32Hue);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    *pu32Hue = u32Hue;
    return s32Ret;
}

/**
 \brief 设置色调
 \attention \n
无

 \param[in] u32Hue：色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetHue(MT_UNF_DISP_E enChan, mt_u32 u32Hue)
{
    if ( u32Hue > 100)
    {
        MT_ERR_PQ("The Hue level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetHue((MT_DRV_DISPLAY_E)enChan, u32Hue);
}

/**
 \brief 获取饱和度
 \attention \n
无

 \param[out] pu32Saturation：饱和度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetSaturation(MT_UNF_DISP_E enChan, mt_u32* pu32Saturation)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Saturation = 0;


    if ( NULL == pu32Saturation)
    {
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_PQ_GetSaturation((MT_DRV_DISPLAY_E)enChan, &u32Saturation);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    *pu32Saturation = u32Saturation;
    return s32Ret;
}

/**
 \brief 设置饱和度
 \attention \n
无

 \param[in] u32Saturation：饱和度,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetSaturation(MT_UNF_DISP_E enChan, mt_u32 u32Saturation)
{
    if ( u32Saturation > 100)
    {
        MT_ERR_PQ("The Saturation level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetSaturation((MT_DRV_DISPLAY_E)enChan, u32Saturation);
}

/**
 \brief 获取降噪强度
 \attention \n
无

 \param[out] pu32NRLevel: 降噪等级, 有效范围: 0~255


 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetNR(MT_UNF_DISP_E enChan, mt_u32* pu32NRLevel)
{
    if ( NULL == pu32NRLevel)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetNR(pu32NRLevel);
}

/**
 \brief 设置降噪强度
 \attention \n
无

 \param[in] u32NRLevel: 降噪等级, 有效范围: 0~255

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetNR(MT_UNF_DISP_E enChan, mt_u32 u32NRLevel)
{
    if ( u32NRLevel > 255)
    {
        MT_ERR_PQ("The NR level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetNR(u32NRLevel);
}

/**
 \brief 获取自动降噪开关状态
 \attention \n
无

 \param[out] pu32OnOff


 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetNRAutoMode(MT_UNF_DISP_E enChan, mt_u32* pu32OnOff)
{
    if ( NULL == pu32OnOff)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetNRAutoMode(pu32OnOff);
}

/**
 \brief 设置降噪自动模式开关
 \attention \n
无

 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetNRAutoMode(MT_UNF_DISP_E enChan, mt_u32 u32OnOff)
{
    return MT_MPI_PQ_SetNRAutoMode(u32OnOff);
}

/**
 \brief 获取SR演示类型
 \attention \n
无

 \param[out] *penType


 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetSRMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_SR_DEMO_E* penType)
{
    if ( NULL == penType)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetSRMode((MT_PQ_SR_DEMO_E*)penType);
}

/**
 \brief 设置SR演示类型
 \attention \n
无

 \param[in] enType

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetSRMode(MT_UNF_DISP_E enChan, MT_UNF_PQ_SR_DEMO_E enType)
{
    return MT_MPI_PQ_SetSRMode((MT_PQ_SR_DEMO_E)enType);
}

/**
 \brief 获取清晰度
 \attention \n
无

 \param[out] pu32Sharpness：清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetSharpness(MT_UNF_DISP_E enChan, mt_u32* pu32Sharpness)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Sharpness = 0;


    if ( NULL == pu32Sharpness)
    {
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_PQ_GetSharpness(&u32Sharpness);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    *pu32Sharpness = u32Sharpness;
    return s32Ret;
}

/**
 \brief 设置清晰度
 \attention \n
无

 \param[in] u32Sharpness：清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetSharpness(MT_UNF_DISP_E enChan, mt_u32 u32Sharpness)
{
    if ( u32Sharpness > 100)
    {
        MT_ERR_PQ("The Sharpness level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetSharpness(u32Sharpness);
}

/**
 \brief 获取块降噪De-blocking强度
 \attention \n
无

 \param[out] pu32DBlevel：降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetDeBlocking(MT_UNF_DISP_E enChan, mt_u32* pu32DBlevel)
{
    if ( NULL == pu32DBlevel)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetDeBlocking(pu32DBlevel);
}

/**
 \brief 设置块降噪De-blocking强度
 \attention \n
无

 \param[in] u32DBlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetDeBlocking(MT_UNF_DISP_E enChan, mt_u32 u32DBlevel)
{
    if ( u32DBlevel > 255)
    {
        MT_ERR_PQ("The DB level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetDeBlocking(u32DBlevel);
}

/**
 \brief 获取去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[out] pu32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetDeRinging(MT_UNF_DISP_E enChan, mt_u32* pu32DRlevel)
{
    if ( NULL == pu32DRlevel)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetDeRinging(pu32DRlevel);
}


/**
 \brief 设置去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[in] u32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetDeRinging(MT_UNF_DISP_E enChan, mt_u32 u32DRlevel)
{
    if ( u32DRlevel > 255)
    {
        MT_ERR_PQ("The DR level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetDeRinging(u32DRlevel);
}


/**
 \brief 获取颜色增强
 \attention \n
无

 \param[out] pu32ColorGainLevel

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetColorGain(MT_UNF_DISP_E enChan, mt_u32* pu32ColorGainLevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32ColorGainLevel = 0;

    if ( NULL == pu32ColorGainLevel)
    {
        return MT_FAILURE;
    }

    s32Ret = MT_MPI_PQ_GetColorGain(&u32ColorGainLevel);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    *pu32ColorGainLevel = u32ColorGainLevel;
    return s32Ret;
}

/**
 \brief 设置颜色增强
 \attention \n
无

 \param[in] enColorGainLevel

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetColorGain(MT_UNF_DISP_E enChan, mt_u32 u32ColorGainLevel)
{
    if ( u32ColorGainLevel > 100)
    {
        MT_ERR_PQ("The ColorGain level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetColorGain(u32ColorGainLevel);
}

/**
 \brief 获取肤色增强
 \attention \n
  无

 \param[out] pu32FleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetFleshTone(MT_UNF_DISP_E enChan, MT_UNF_PQ_FLESHTONE_E* pu32FleshToneLevel)
{
    if ( NULL == pu32FleshToneLevel)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetFleshTone(pu32FleshToneLevel);
}

/**
 \brief 设置肤色增强
 \attention \n
  无

 \param[in] enFleshToneLevel，参考MT_COLOR_GAIN_E

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetFleshTone(MT_UNF_DISP_E enChan, MT_UNF_PQ_FLESHTONE_E enFleshToneLevel)
{
    if ( enFleshToneLevel >= MT_UNF_PQ_FLESHTONE_GAIN_BUTT)
    {
        MT_ERR_PQ("The FleshTone level is out of range!");

        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetFleshTone(enFleshToneLevel);
}

/**
 \brief 获取PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] pu32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetPQModule( MT_UNF_PQ_MODULE_E enFlags, mt_u32* pu32OnOff)
{
    if ((NULL == pu32OnOff ) || (enFlags >= MT_UNF_PQ_MODULE_BUTT))
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetPQModule(enFlags, pu32OnOff);
}

/**
 \brief 设置PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetPQModule( MT_UNF_PQ_MODULE_E enFlags, mt_u32 u32OnOff)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (MT_UNF_PQ_MODULE_BUTT <= enFlags)
    {
        return MT_FAILURE;
    }

    if (MT_UNF_PQ_MODULE_ALL == enFlags)
    {
        s32Ret  = MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_SHARPNESS, u32OnOff);
        s32Ret |= MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_DCI, u32OnOff);
        s32Ret |= MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_COLOR, u32OnOff);
        s32Ret |= MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_SR, u32OnOff);
    }
    else
    {
        s32Ret  = MT_MPI_PQ_SetPQModule(enFlags, u32OnOff);
    }

    return s32Ret;
}

/**
 \brief 设置卖场模式开关
 \attention \n
无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetDemo( MT_UNF_PQ_DEMO_E enFlags, mt_u32 u32OnOff)
{
    if (MT_UNF_PQ_DEMO_BUTT <= enFlags)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetDemo(enFlags, u32OnOff);
}

/**
 \brief 获取颜色增强的类型和强度
 \attention \n
无

 \param[out] pstColorEnhanceParam:颜色增强的类型和强度;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetColorEnhanceParam(MT_UNF_PQ_COLOR_ENHANCE_S* pstColorEnhanceParam)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_UNF_PQ_COLOR_ENHANCE_E 	 enType;
    MT_UNF_PQ_FLESHTONE_E enFleshToneLevel = MT_UNF_PQ_FLESHTONE_GAIN_OFF;
    MT_UNF_PQ_SIX_BASE_S stSixBase = {0};
    MT_UNF_PQ_COLOR_SPEC_MODE_E enColorMode = MT_UNF_PQ_COLOR_MODE_RECOMMEND;

    if (NULL == pstColorEnhanceParam )
    {
        return MT_FAILURE;
    }

    enType = pstColorEnhanceParam->enColorEnhanceType;

    if (MT_UNF_PQ_COLOR_ENHANCE_FLESHTONE == enType)
    {
        s32Ret = MT_MPI_PQ_GetFleshTone(&enFleshToneLevel);
        if (MT_FAILURE == s32Ret )
        {
            return MT_FAILURE;
        }
        pstColorEnhanceParam->unColorGain.enFleshtone = enFleshToneLevel;
    }
    else if (MT_UNF_PQ_COLOR_ENHANCE_SIX_BASE == enType)
    {
        s32Ret = MT_MPI_PQ_GetSixBaseColor(&stSixBase);
        if (MT_FAILURE == s32Ret )
        {
            return MT_FAILURE;
        }

        pstColorEnhanceParam->unColorGain.stSixBase = stSixBase;
    }
    else if (MT_UNF_PQ_COLOR_ENHANCE_SPEC_COLOR_MODE == enType)
    {
        s32Ret = MT_MPI_PQ_GetColorEnhanceMode(&enColorMode);
        if (MT_FAILURE == s32Ret )
        {
            return MT_FAILURE;
        }

        pstColorEnhanceParam->unColorGain.enColorMode = enColorMode;
    }

    return s32Ret;
}


/**
 \brief 设置颜色增强的类型和强度
 \attention \n
无

 \param[out] enColorEnhanceType:颜色增强的类型和强度;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetColorEnhanceParam(MT_UNF_PQ_COLOR_ENHANCE_S stColorEnhanceParam)
{
    MT_UNF_PQ_COLOR_ENHANCE_E    enType;
    enType = stColorEnhanceParam.enColorEnhanceType;
    MT_UNF_PQ_SIX_BASE_S stSixBaseColor;
    MT_UNF_PQ_COLOR_SPEC_MODE_E enColorSpecMode;

    if (MT_UNF_PQ_COLOR_ENHANCE_FLESHTONE == enType)
    {
        return MT_MPI_PQ_SetFleshTone(stColorEnhanceParam.unColorGain.enFleshtone);
    }
    else if (MT_UNF_PQ_COLOR_ENHANCE_SIX_BASE == enType)
    {
        stSixBaseColor = stColorEnhanceParam.unColorGain.stSixBase;
        return MT_MPI_PQ_SetSixBaseColor((MT_PQ_SIX_BASE_COLOR_S*)&stSixBaseColor);
    }
    else if (MT_UNF_PQ_COLOR_ENHANCE_SPEC_COLOR_MODE == enType)
    {
        enColorSpecMode = stColorEnhanceParam.unColorGain.enColorMode;
        return MT_MPI_PQ_SetColorEnhanceMode((MT_PQ_COLOR_SPEC_MODE_E)enColorSpecMode);
    }

    return MT_FAILURE;
}


/**
 \brief 获取DCI（动态对比度增强）的强度范围
 \attention \n
无

 \param[out] pu32DCIlevel:动态对比度等级, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_GetDynamicContrast(mt_u32* pu32DCIlevel)
{
    if (NULL == pu32DCIlevel )
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_GetDciLevel(pu32DCIlevel);
}


/**
 \brief 设置DCI（动态对比度增强）的强度范围
 \attention \n
无

 \param[in] u32DCIlevel:动态对比度等级, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_UNF_PQ_SetDynamicContrast(mt_u32 u32DCIlevel)
{
    if (u32DCIlevel > 100)
    {
        return MT_FAILURE;
    }

    return MT_MPI_PQ_SetDciLevel(u32DCIlevel);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

