/******************************************************************************

  Copyright (C), 2017, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : drv_pq.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/01/20
  Description   :
  History       :
  1.Date        :
  Author        :
  Modification  : Created file

*******************************************************************************/

#include <linux/pm.h>

#include "drv_pq.h"
#include "drv_pdm_ext.h"
#include "drv_pq_ext.h"
#include "mt_drv_dev.h"
#include "pq_hal.h"
#include "mt_module_debug.h"
#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

PQ_PARAM_S*     g_pstPqParam = MT_NULL; /*PQ BIN */
mmz_buffer_s    g_stPqBinBuf;
MT_BOOL         g_bLoadPqBin;
mt_u32 p_pq_addr;

static MT_DRV_PQ_PARAM_S stPqParam;
//static mt_u32 sg_u32SourceMode = 0;//SOURCE_MODE_NO;
//static mt_u32 sg_u32OutputMode = 0;//OUTPUT_MODE_NO;
//static mt_u32 sg_u32Zme2LThl = 4; /*Zme二级缩放门限值 */
static MT_BOOL sg_bFastBoot = MT_FALSE;

#define PQ_TRACE(level, range, fmt...) \
    do{\
        if (level <= range)\
        {\
            MT_TRACE(MT_LOG_LEVEL_ERROR, HI_ID_PQ, fmt);\
        }\
    }while(0)


#define PQ_CHECK_NULL_PTR(ptr) \
    do{\
        if (NULL == ptr){\
            MT_TRACE(MT_LOG_LEVEL_ERROR, MT_ID_PQ, "pointer is NULL!\n");\
            return MT_FAILURE;\
        }\
    }while(0)

//static mt_s32 DRV_PQ_CheckChipName(MT_CHAR* pchChipName);
//static mt_s32 DRV_PQ_GetFlashPqBin(PQ_PARAM_S* pstPqParam);
static mt_s32 DRV_PQ_GetPicSetting(mt_void);

mt_s32 DRV_PQ_Suspend(basedev_s *pdev, pm_message_t state)
{
    if (PM_EVENT_FREEZE == state.event)
    {
        sg_bFastBoot = MT_TRUE;
    }
    else
    {
        sg_bFastBoot = MT_FALSE;
    }

    return MT_SUCCESS;
}

mt_s32 DRV_PQ_Resume(basedev_s *pdev)
{
    mt_s32 s32Ret;

    if (MT_TRUE == sg_bFastBoot)
    {
        s32Ret = DRV_PQ_GetPicSetting();
        if (MT_SUCCESS != s32Ret)
        {
            MT_WARN_PQ("ERR: DRV_PQ_GetPicSetting failed!\n");
        }
    }

    return MT_SUCCESS;
}


PQ_EXPORT_FUNC_S   g_PqExportFuncs =
{
    .pfnPQ_GetPqParam             = DRV_PQ_GetPqParam,
    .pfnPQ_UpdateVpssPQ           = DRV_PQ_UpdateVpssPQ,
    .pfnPQ_UpdateVdpPQ            = DRV_PQ_UpdateVdpPQ,
    .pfnPQ_UpdateVdpCSC           = DRV_PQ_UpdateVdpCSC,
    .pfnPQ_UpdateDCIWin           = DRV_PQ_UpdateDCIWin,
    .pfnPQ_GetVpssScalerCoef      = DRV_PQ_GetVpssScalerCoef,
    .pfnPQ_GetScalerCoef          = DRV_PQ_GetScalerCoef,
    .pfnPQ_GetWbcScalerCoef       = DRV_PQ_GetWbcScalerCoef,
    .pfnPQ_SetZme                 = DRV_PQ_SetZme,
    .pfnPQ_SetVpssZme             = DRV_PQ_SetVpssZme,
    .pfnPQ_GetCSCCoef             = DRV_PQ_GetCSCCoef,
    .pfnPQ_Get8BitCSCCoef         = DRV_PQ_Get8BitCSCCoef,
    .pfnPQ_GetWbcInfo             = DRV_PQ_GetWbcInfo,
    .pfnPQ_SetAlgCalcCfg          = DRV_PQ_SetAlgCalcCfg,
    .pfnPQ_GGetTnrGlobalMotion    = DRV_PQ_GetTnrGlobalMotion,
    .pfnPQ_GetAdaptiveDBStrength  = DRV_PQ_GetAdaptiveDBStrength,
    .pfnPQ_PfmdDetect             = DRV_PQ_PfmdDetect,
    .pfnPQ_IfmdDect               = DRV_PQ_IfmdDect,
    .pfnPQ_GetDciHistgram         = DRV_PQ_GetDciHistgram,
    .pfnPQ_GetHDPictureSetting    = DRV_PQ_GetHDPictureSetting,
    .pfnPQ_GetSDPictureSetting    = DRV_PQ_GetSDPictureSetting,
    .pfnPQ_SetHDPictureSetting    = DRV_PQ_SetHDPictureSetting,
    .pfnPQ_SetSDPictureSetting    = DRV_PQ_SetSDPictureSetting
};

/**
 \brief 去初始化PQ模块;
 \attention \n
  无

 \param[in]

 \retval ::MT_SUCCESS

 */
mt_s32 MT_DRV_PQ_DeInit(mt_void)
{
    //mt_s32 s32Ret;

    mt_drv_module_unregister(MT_ID_PQ);
    mt_drv_mmz_unmap_and_release((mmz_buffer_s*)(&g_stPqBinBuf));


    return MT_SUCCESS;
}

/**
 \brief 初始化PQ模块;
 \attention \n
  无

 \param[in] pszPath: PQ Table文件路径, 如果pszPath参数为空指针, 会采用PQ SDK内部默认参数;

 \retval ::MT_SUCCESS

 */
mt_s32 MT_DRV_PQ_Init(MT_CHAR* pszPath)
{
    mt_s32 s32Ret;
    mt_s32 i;
    g_bLoadPqBin = MT_FALSE;
    g_pstPqParam = MT_NULL;

    s32Ret = mt_drv_module_register(MT_ID_PQ, PQ_NAME, (mt_void*)&g_PqExportFuncs);

    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_PQ("ERR: MT_DRV_MODULE_Register!\n");
        return s32Ret;
    }

    s32Ret = mt_drv_mmz_alloc_and_map("PQ_FLASH_BIN", MT_NULL, sizeof(PQ_PARAM_S), 0, (mmz_buffer_s*)(&g_stPqBinBuf));
    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_PQ("ERR: Pqdriver mmz memory failed!\n");
        g_bLoadPqBin = MT_FALSE;
        return s32Ret;
    }

    g_pstPqParam = (PQ_PARAM_S*)g_stPqBinBuf.startVirAddr;
    MT_INFO_PQ("\ng_stPqBinBuf.u32StartVirAddr = 0x%x,g_stPqBinBuf.u32StartVirAddr = 0x%x\n", g_stPqBinBuf.startPhyAddr, g_stPqBinBuf.startVirAddr);

    stPqParam.stSDPictureSetting.u16Brightness = 128;
    stPqParam.stSDPictureSetting.u16Contrast   = 128;
    stPqParam.stSDPictureSetting.u16Hue        = 128;
    stPqParam.stSDPictureSetting.u16Saturation = 128;
    stPqParam.stHDPictureSetting.u16Brightness = 128;
    stPqParam.stHDPictureSetting.u16Contrast   = 128;
    stPqParam.stHDPictureSetting.u16Hue        = 128;
    stPqParam.stHDPictureSetting.u16Saturation = 128;

    /*Init Customer PQ setting*/
    stPqParam.stColorTemp.s16RedGain     = 128;
    stPqParam.stColorTemp.s16GreenGain   = 128;
    stPqParam.stColorTemp.s16BlueGain    = 128;
    stPqParam.stColorTemp.s16RedOffset   = 128;
    stPqParam.stColorTemp.s16GreenOffset = 128;
    stPqParam.stColorTemp.s16BlueOffset  = 128;

    stPqParam.stDciWin.u16HStar = 0;
    stPqParam.stDciWin.u16HEnd  = 0;
    stPqParam.stDciWin.u16VStar = 0;
    stPqParam.stDciWin.u16VEnd  = 0;

    stPqParam.u32NRLevel        = 128;
    stPqParam.u32Sharpness      = 128;
    stPqParam.u32DBLevel        = 128;
    stPqParam.u32DRLevel        = 128;
    stPqParam.u32ColorGainLevel = 128;
    stPqParam.u323dSharpen      = MT_FALSE;
    stPqParam.u32NrAuto         = MT_FALSE;
    stPqParam.u32DCILevelGain   = 50;
    stPqParam.stColorEnhance.enColorEnhanceType      = MT_PQ_COLOR_ENHANCE_SPEC_COLOR_MODE;
    stPqParam.stColorEnhance.unColorGain.enColorMode = MT_PQ_COLOR_MODE_RECOMMEND;

    for (i = 0; i < MT_PQ_DEMO_BUTT; i++)
    {
        stPqParam.bDemoOnOff[i] = MT_FALSE;
    }

    for (i = 0; i < MT_PQ_MODULE_BUTT; i++)
    {
        stPqParam.bModuleOnOff[i] = MT_FALSE;
    }

    stPqParam.bModuleOnOff[MT_PQ_MODULE_SR] = MT_TRUE;

    p_pq_addr = mt_get_display_base();

    return MT_SUCCESS;
}

/**
 \brief 去初始化客户PQ模块;
 \attention \n
  无

 \param[in]

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_Comsumer_DeInit(mt_void)
{
    //mt_s32 s32Ret;

    return MT_SUCCESS;
}

/**
 \brief 初始化客户PQ模块;
 \attention \n
  无

 \param[in] pszPath: PQ Table文件路径, 如果pszPath参数为空指针, 会采用PQ SDK内部默认参数;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_Comsumer_Init(MT_CHAR* pszPath)
{
    //mt_s32 s32Ret = MT_FAILURE;

    return MT_SUCCESS;
}


/**
 \brief 获取标清亮度
 \attention \n
无

 \param[out] pu32Brightness 亮度值,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDBrightness(mt_u32* pu32Brightness)
{
    PQ_CHECK_NULL_PTR(pu32Brightness);

    *pu32Brightness = NUM2LEVEL(stPqParam.stSDPictureSetting.u16Brightness);
    return MT_SUCCESS;
}

/**
 \brief 设置标清亮度
 \attention \n
无

 \param[in] u32Brightness, 亮度值,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSDBrightness(mt_u32 u32Brightness)
{
    reg_aria2_disp_set_sd_video_effect_coef_sd_bright_coeff(LEVEL2NUM(u32Brightness));

    stPqParam.stSDPictureSetting.u16Brightness = LEVEL2NUM(u32Brightness);

    return MT_SUCCESS;
}

/**
 \brief 获取标清对比度
 \attention \n
无

 \param[out] pu32Contrast 对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDContrast(mt_u32* pu32Contrast)
{
    PQ_CHECK_NULL_PTR(pu32Contrast);

    *pu32Contrast = NUM2LEVEL(stPqParam.stSDPictureSetting.u16Contrast);

    return MT_SUCCESS;
}

/**
 \brief 设置标清对比度
 \attention \n
无

 \param[in] u32Contrast, 对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSDContrast(mt_u32 u32Contrast)
{
    reg_aria2_disp_set_sd_video_effect_coef_sd_contrast_coeff(LEVEL2NUM(u32Contrast));

    stPqParam.stSDPictureSetting.u16Contrast = LEVEL2NUM(u32Contrast);

    return MT_SUCCESS;
}

/**
 \brief 获取标清色调
 \attention \n
无

 \param[out] pu32Hue  色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDHue(mt_u32* pu32Hue)
{

    return MT_SUCCESS;
}

/**
 \brief 设置标清色调
 \attention \n
无

 \param[in] u32Hue   色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetSDHue(mt_u32 u32Hue)
{
    return MT_SUCCESS;
}

/**
 \brief 获取标清饱和度
 \attention \n
无

 \param[out] pu32Saturation  饱和度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSDSaturation(mt_u32* pu32Saturation)
{
    PQ_CHECK_NULL_PTR(pu32Saturation);

    *pu32Saturation = NUM2LEVEL(stPqParam.stSDPictureSetting.u16Saturation);

    return MT_SUCCESS;
}

/**
 \brief 设置标清饱和度
 \attention \n
无

 \param[in] u32Saturation 饱和度,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSDSaturation(mt_u32 u32Saturation)
{
    reg_aria2_disp_set_sd_video_effect_coef_sd_saturation_coeff(LEVEL2NUM(u32Saturation));

    stPqParam.stSDPictureSetting.u16Saturation = LEVEL2NUM(u32Saturation);

    return MT_SUCCESS;
}

/**
 \brief 获取高清亮度
 \attention \n
无

 \param[out] pu32Brightness 亮度值,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetHDBrightness(mt_u32* pu32Brightness)
{
    PQ_CHECK_NULL_PTR(pu32Brightness);
    *pu32Brightness = NUM2LEVEL(stPqParam.stHDPictureSetting.u16Brightness);

    return MT_SUCCESS;
}

/**
 \brief 设置高清亮度
 \attention \n
无

 \param[in] u32Brightness, 亮度值,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetHDBrightness(mt_u32 u32Brightness)
{
    reg_aria2_disp_set_hd_video_effect_coef_bright_coeff(LEVEL2NUM(u32Brightness));

    stPqParam.stHDPictureSetting.u16Brightness = LEVEL2NUM(u32Brightness);
    return MT_SUCCESS;
}

/**
 \brief 获取高清对比度
 \attention \n
无

 \param[out] pu32Contrast 对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetHDContrast(mt_u32* pu32Contrast)
{
    PQ_CHECK_NULL_PTR(pu32Contrast);
    *pu32Contrast = NUM2LEVEL(stPqParam.stHDPictureSetting.u16Contrast);

    return MT_SUCCESS;
}

/**
 \brief 设置高清对比度
 \attention \n
无

 \param[in] u32Contrast, 对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetHDContrast(mt_u32 u32Contrast)
{
    reg_aria2_disp_set_hd_video_effect_coef_contrast_coeff(LEVEL2NUM(u32Contrast));

    stPqParam.stHDPictureSetting.u16Contrast = LEVEL2NUM(u32Contrast);
    return MT_SUCCESS;
}


/**
 \brief 获取高清色调
 \attention \n
无

 \param[out] pu32Hue  色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetHDHue(mt_u32* pu32Hue)
{

    return MT_SUCCESS;
}


/**
 \brief 设置高清色调
 \attention \n
无

 \param[in] u32Hue   色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetHDHue(mt_u32 u32Hue)
{
    return MT_SUCCESS;
}


/**
 \brief 获取高清饱和度
 \attention \n
无

 \param[out] pu32Saturation  饱和度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetHDSaturation(mt_u32* pu32Saturation)
{
    PQ_CHECK_NULL_PTR(pu32Saturation);
    *pu32Saturation = NUM2LEVEL(stPqParam.stHDPictureSetting.u16Saturation);

    return MT_SUCCESS;
}


/**
 \brief 设置高清饱和度
 \attention \n
无

 \param[in] u32Saturation 饱和度,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetHDSaturation(mt_u32 u32Saturation)
{
    reg_aria2_disp_set_hd_video_effect_coef_saturation_coeff(LEVEL2NUM(u32Saturation));

    stPqParam.stHDPictureSetting.u16Saturation = LEVEL2NUM(u32Saturation);
    return MT_SUCCESS;
}


/**
 \brief 获取高清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[out] u32Hue   亮度/对比度/色调/饱和度/色温

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetHDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting)
{
    //PICTURE_SETTING_S stPictureSetting;
    //COLOR_TEMPERATURE_S stColorTemp;

    PQ_CHECK_NULL_PTR(pstPictureSetting);

    //PQ_MNG_GetHDPictureSetting(&stPictureSetting, &stColorTemp);
    pstPictureSetting->u16Brightness = NUM2LEVEL(stPqParam.stHDPictureSetting.u16Brightness);
    pstPictureSetting->u16Contrast   = NUM2LEVEL(stPqParam.stHDPictureSetting.u16Contrast);
    pstPictureSetting->u16Hue        = NUM2LEVEL(stPqParam.stHDPictureSetting.u16Hue);
    pstPictureSetting->u16Saturation = NUM2LEVEL(stPqParam.stHDPictureSetting.u16Saturation);

    //pstPictureSetting->s16RedGain     = NUM2LEVEL(stColorTemp.s16BlueGain);
    //pstPictureSetting->s16GreenGain   = NUM2LEVEL(stColorTemp.s16GreenGain);
    //pstPictureSetting->s16BlueGain    = NUM2LEVEL(stColorTemp.s16BlueGain);
    //pstPictureSetting->s16RedOffset   = NUM2LEVEL(stColorTemp.s16RedOffset);
    //pstPictureSetting->s16GreenOffset = NUM2LEVEL(stColorTemp.s16GreenOffset);
    //pstPictureSetting->s16BlueOffset  = NUM2LEVEL(stColorTemp.s16BlueOffset);

    return MT_SUCCESS;
}


/**
 \brief 获取标清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[out] u32Hue   亮度/对比度/色调/饱和度/色温

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetSDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting)
{
    //PICTURE_SETTING_S stPictureSetting;
    //COLOR_TEMPERATURE_S stColorTemp;

    PQ_CHECK_NULL_PTR(pstPictureSetting);

    //PQ_MNG_GetSDPictureSetting(&stPictureSetting, &stColorTemp);
    pstPictureSetting->u16Brightness = NUM2LEVEL(stPqParam.stSDPictureSetting.u16Brightness);
    pstPictureSetting->u16Contrast   = NUM2LEVEL(stPqParam.stSDPictureSetting.u16Contrast);
    pstPictureSetting->u16Hue        = NUM2LEVEL(stPqParam.stSDPictureSetting.u16Hue);
    pstPictureSetting->u16Saturation = NUM2LEVEL(stPqParam.stSDPictureSetting.u16Saturation);

    //pstPictureSetting->s16RedGain     = NUM2LEVEL(stColorTemp.s16BlueGain);
    //pstPictureSetting->s16GreenGain   = NUM2LEVEL(stColorTemp.s16GreenGain);
    //pstPictureSetting->s16BlueGain    = NUM2LEVEL(stColorTemp.s16BlueGain);
    //pstPictureSetting->s16RedOffset   = NUM2LEVEL(stColorTemp.s16RedOffset);
    //pstPictureSetting->s16GreenOffset = NUM2LEVEL(stColorTemp.s16GreenOffset);
    //pstPictureSetting->s16BlueOffset  = NUM2LEVEL(stColorTemp.s16BlueOffset);

    return MT_SUCCESS;
}


/**
 \brief 设置高清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[in] u32Hue   亮度/对比度/色调/饱和度/色温;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetHDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting)
{

    return MT_SUCCESS;
}


/**
 \brief 设置标清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[in] u32Hue   亮度/对比度/色调/饱和度/色温;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetSDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting)
{
    return MT_SUCCESS;
}

#if 0
/**
 \brief 刷新DB API 全局变量
 \attention \n
无

 \param[in]  MT_PQ_WBC_INFO_S* pstWbcInfo

 \retval ::MT_SUCCESS

 */
static mt_s32 DRV_PQ_DBCfgRefresh(mt_u32 u32HandleNo, mt_u32 u32Height, mt_u32 u32Width, mt_u32 u32FRate)
{
    //API Init 场景切换刷新DB全局变量值，这个不是每帧都刷新，运算值会存储在全局变量中
    PQ_MNG_DB_API_Init(u32HandleNo, u32Height, u32Width, u32FRate);

    return MT_SUCCESS;
}
#endif

/**
 \brief 获取清晰度
 \attention \n
无

 \param[out] pu32Sharpness  清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSharpness(mt_u32* pu32Sharpness)
{
    *pu32Sharpness = NUM2LEVEL(stPqParam.u32Sharpness);
    return MT_SUCCESS;
}

/**
 \brief 设置清晰度
 \attention \n
无

 \param[in] u32Sharpness, 清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetSharpness(mt_u32 u32Sharpness)
{
    //mt_s32 s32Ret;
    //mt_u32 u32Num = LEVEL2NUM(u32Sharpness);//0~100 trans to 0~255


    return MT_SUCCESS;
}



/**
 \brief 获取色温参数
 \attention \n
无

 \param[in] pstColorTemp: 色温属性

 \retval::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetColorTemp(MT_PQ_COLOR_TEMP_S* pstColorTemp)
{
    return MT_SUCCESS;
}


/**
 \brief 设置色温参数
 \attention \n
无

 \param[out] pstColorTemp: 色温属性

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetColorTemp(MT_PQ_COLOR_TEMP_S* pstColorTemp)
{
    return MT_SUCCESS;
}


/**
 \brief 获取颜色增强
 \attention \n
无

 \param[out] pu32ColorGainLevel

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetColorEhance(mt_u32* pu32ColorGainLevel)
{
    //mt_s32 s32Ret;
    PQ_CHECK_NULL_PTR(pu32ColorGainLevel);


    return MT_SUCCESS;
}


/**
 \brief 设置颜色增强
 \attention \n
无

 \param[in] u32ColorGainLevel

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetColorEhance(mt_u32 u32ColorGainLevel)
{
    //mt_u32 u32Num = LEVEL2NUM(u32ColorGainLevel);

    return MT_SUCCESS;
}

/**
 \brief 获取肤色增强
 \attention \n
  无

 \param[out] pu32FleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetFleshToneLevel(mt_u32* pu32FleshToneLevel)
{
    return MT_SUCCESS;
}

/**
 \brief 设置肤色增强
 \attention \n
  无

 \param[in] enFleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetFleshToneLevel(MT_PQ_FLESHTONE_E enFleshToneLevel)
{
    stPqParam.stColorEnhance.enColorEnhanceType = MT_PQ_COLOR_ENHANCE_FLESHTONE;
    stPqParam.stColorEnhance.unColorGain.enFleshtone = enFleshToneLevel;

    MT_DBG_PQ("Set FleshTone level %d\n", enFleshToneLevel);
    return MT_SUCCESS;
}

/**
 \brief 六基色控制设置
 \attention \n
  无

 \param[in] enSixColorType;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetSixBaseColorLevel(MT_PQ_SIX_BASE_S* pstSixBaseColorOffset)
{
    return MT_SUCCESS;
}

/**
 \brief 颜色增强模式设置
 \attention \n
  无

 \param[in] enColorSpecMode 0-RECOMMEND;1-BLUE;2-GREEN;3-BG;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetColorEnhanceMode(MT_PQ_COLOR_SPEC_MODE_E enColorSpecMode)
{
    return MT_SUCCESS;
}


/**
 \brief 设置DCI强度增益等级
 \attention \n
无

 \param[in] u32DCILevelGain;

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetDCILevelGain(mt_u32 u32DCILevelGain)
{
    return MT_SUCCESS;
}

#if 0 //janny
/**
 \brief 设置DCI配置曲线
 \attention \n
无

 \param[in] pstDciCoef;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetDCIWgtLut(DCI_WGT_S* pstDciCoef)
{
    return MT_SUCCESS;
}
#endif

/**
 \brief 设置卖场模式开关
 \attention \n
无

 \param[in] enFlags
 \param[in] bOnOff

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetDemoMode(MT_PQ_DEMO_E enFlags, MT_BOOL bOnOff)
{
    return MT_SUCCESS;
}


/**
 \brief 获取PQ模块开关状态
 \attention \n
  无

 \param[in] enFlags
 \param[out] *pu32OnOff

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetPQModule(MT_PQ_MODULE_E enFlags, mt_u32* pu32OnOff)
{

    return MT_SUCCESS;
}


/**
 \brief 设置PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetPQModule(MT_PQ_MODULE_E enFlags, mt_u32 u32OnOff)
{
    //mt_s32 s32Ret = MT_FAILURE;

    return MT_SUCCESS;
}

/**
 \brief Timming变化后进行VPSS PQ参数更新
 \attention \n
无

 \param[in]  *u32HandleNo:   VPSS通道号
 \param[in]  *pstTimingInfo: Timming Info
 \param[in]  *pstVPSSReg:    VPSS 虚拟寄存器地址
 \param[out] *pstPQModule:   PQ传给驱动的开关参数

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_UpdateVpssPQ(mt_u32 u32HandleNo, MT_VPSS_PQ_INFO_S* pstTimingInfo, /*PQ_VPSS_CFG_REG_S**/mt_void* pstVPSSReg, /*PQ_VPSS_WBC_REG_S**/mt_void* pstWbcReg, MT_PQ_VPSS_MODULE_S* pstPQModule)
{
    //mt_s32 s32Ret;


    return MT_SUCCESS;
}

/**
 \brief Timming变化后进行VDP PQ参数更新
 \attention \n
无

 \param[in] pstTimingInfo: Timming Info
 \param[in] *pstVDPReg:VDP 虚拟寄存器地址

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_UpdateVdpPQ(mt_u32 u32DisplayId, MT_VDP_PQ_INFO_S* pstTimingInfo, /*S_VDP_REGS_TYPE**/mt_void* pstVDPReg)
{
    //mt_s32 s32Ret;


    return MT_SUCCESS;
}

/**
 \brief 设置各通道的CSC
 \attention \n
无

 \param[in] enDisplayId:
 \param[in] pstCscMode: 色彩空间

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_UpdateVdpCSC(MT_PQ_CSC_ID_E enDisplayId, MT_PQ_VDP_CSC_S* pstCscMode)
{
    return MT_SUCCESS;
}

/**
 \brief 更新DCI直方图统计窗口
 \attention \n
无

 \param[in] *pstWin;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_UpdateDCIWin(MT_PQ_DCI_WIN_S* pstWin, MT_BOOL bDciEnable)
{
    return MT_SUCCESS;
}

/**
 \brief 获取CSC系数
 \attention \n
无

 \param[in] enCSCMode:
 \param[out] pstCSCCoef:
 \param[out] pstCSCDCCoef:

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetCSCCoef(MT_PQ_CSC_MODE_E  enCSCMode, MT_PQ_CSC_COEF_S* pstCSCCoef, MT_PQ_CSC_DCCOEF_S* pstCSCDCCoef)
{
    return MT_SUCCESS;
}

/**
 \brief 获取8bit CSC系数
 \attention \n
无

 \param[in] enCSCMode:
 \param[out] pstCSCCoef:
 \param[out] pstCSCDCCoef:

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_Get8BitCSCCoef(MT_PQ_CSC_MODE_E  enCSCMode, MT_PQ_CSC_COEF_S* pstCSCCoef, MT_PQ_CSC_DCCOEF_S* pstCSCDCCoef)
{
    return MT_SUCCESS;
}

mt_s32 DRV_PQ_SetZme(MT_PQ_ZME_LAYER_E e32LayerId, MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_BOOL  bFirEnable)
{
    return MT_SUCCESS;
}

mt_s32 DRV_PQ_SetVpssZme(MT_PQ_VPSS_ZME_LAYER_E e32LayerId, /*S_CAS_REGS_TYPE**/mt_void* pstReg, MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_BOOL  bFirEnable)
{
    return MT_SUCCESS;
}

/**
 \brief VPSS ZME二级缩放校验;
 \attention \n
无

 \param[in]
 u32InWitdh: 输入宽; u32InHeigh: 输入高; stZmeWin: 各层输出宽高;

 \param[out]
 u32OutWitdh 输出宽; u32OutHeigh 输出高;

 \retval :
MT_SUCCESS : 需要二级缩放
MT_FAILURE : 不需要二级缩放

 */
mt_s32 DRV_PQ_ZME_2L_Check(mt_u32 u32InWitdh, mt_u32 u32InHeigh, MT_PQ_ZME_WIN_S stZmeWin, mt_u32* pu32OutWitdh, mt_u32* pu32OutHeigh)
{

    return MT_SUCCESS;
}

/**
 \brief 获取VPSS scaler分块缩放的配置参数
 \attention \n
无

 \param[in] pstZmeDrvPara:缩放输入信息
 \param[out] pstZmeSplitPara:分块缩放的配置信息

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetSplitBlockZme(MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_PQ_SPLIT_ZME_PARA_S* pstZmeSplitPara)
{

    return MT_SUCCESS;
}


/**
 \brief 获取VPSS scaler缩放系数
 \attention \n
无

 \param[in] pstScalerInfo:缩放信息
 \param[out] pvCoefAddr:缩放系数地址

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetVpssScalerCoef(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pLCoefAddr, mt_void* pCCoefAddr)
{
    return MT_SUCCESS;
}
/**
 \brief 获取VDP scaler缩放系数
 \attention \n
无

 \param[in] pstScalerInfo:缩放信息
 \param[out] pvCoefAddr:缩放系数地址

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetScalerCoef(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pLCoefAddr, mt_void* pCCoefAddr)
{
    return MT_SUCCESS;
}
/**
 \brief 获取wbc scaler缩放系数
 \attention \n
无

 \param[in] pstScalerInfo:缩放信息
 \param[out] pvCoefAddr:缩放系数地址

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetWbcScalerCoef(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pLCoefAddr, mt_void* pCCoefAddr)
{
    return MT_SUCCESS;
}

/**
 \brief 获取各通道的CSC(用于调试)
 \attention \n
无

 \param[in] enDisplayId
 \param[out] pstCSCMode

 \retval ::MT_SUCCESS

 */
 #if 0 //janny
mt_s32 DRV_PQ_GetCSCMode(MT_PQ_CSC_ID_E enDisplayId, MT_PQ_VDP_CSC_S* pstCSCMode)
{
    return MT_SUCCESS;
}
 #endif

/**
 \brief 获取SR演示模式开关
 \attention \n
无

 \param[in] ps32Type

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetSRMode(mt_s32* ps32Type)
{
    return MT_SUCCESS;
}

/**
 \brief 设置SR演示模式开关
 \attention \n
无

 \param[in] eSRMode: SR演示模式0-只ZME;1-右边SR；2-左边SR；3-SR开

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_SetSRMode(MT_PQ_SR_DEMO_E eSRMode)
{
    return MT_SUCCESS;
}

#if 0
/**
 \brief 读取DB运算所需的寄存器信息，配置到寄存器
 \attention \n
无

 \param[in]  MT_PQ_WBC_INFO_S* pstWbcInfo

 \retval ::MT_SUCCESS

 */
static mt_s32 DRV_PQ_DBCalcCfg(MT_PQ_DB_CALC_INFO_S* pstDbCalcInfo)
{
    if (pstDbCalcInfo == NULL)
    {
        MT_ERR_PQ("get Db Calc Info pointer is null!\n");
        return MT_FAILURE;
    }

    PQ_MNG_DBCalcConfigCfg((DB_CALC_INFO_S*)pstDbCalcInfo);

    return MT_SUCCESS;
}
#endif
/**
 \brief 获取WbcInfo信息
 \attention \n
无

 \param[in]  MT_PQ_WBC_INFO_S* pstWbcInfo

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_GetWbcInfo(MT_PQ_WBC_INFO_S* pstVpssWbcInfo)
{

    return MT_SUCCESS;
}
#if 0
/**
 \brief 获取Globa Motion信息
 \attention \n
无

 \param[in]  *pstMotionHist:直方图
 \param[out] *pstMotionResult:运动信息

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetDeiGlobalMotion(MT_PQ_MOTION_INPUT_S* pstMotionInput, MT_PQ_MOTION_INFO_S* pstMotionResult)
{
    if (pstMotionInput == NULL || pstMotionResult == NULL)
    {
        MT_ERR_PQ("get Globa Motion point is null!\n");
        return MT_FAILURE;
    }

    return PQ_MNG_GetGlobalMotion((MOTION_INPUT_S*)pstMotionInput, (MOTION_RESULT_S*)pstMotionResult);
}
#endif

/**
 \brief 设置算法运算完之后的寄存器
 \attention \n
无

 \param[in]  *pstWbcInfo
 \param[out] *pstVpssCfgInfo
 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_SetAlgCalcCfg(MT_PQ_WBC_INFO_S* pstVpssWbcInfo, MT_PQ_CFG_INFO_S* pstVpssCfgInfo)
{

    return MT_SUCCESS;
}


/**
\brief     :Tnr的Global Motion软算法API

\attention :
            本算法参考DEI模块进行移植
            算法提供：裴朝科 p00188942；移植&维护：吕明君 l00268071

\param[in] :pstParamIn结构，包含如下元素
            u32HdlNo 实例的ID
            u32Width 输入图像宽度
            u32Height 输入图像高度
            pstMotionReg 寄存器地址

\param[out]:pstMotionResult结构，包含如下元素
            u32GlobalMotion 配给寄存器VPSS_TNR_TFM_PARA中的tfm_globalmotion
            u32AdjustGain

\retval    :MT_SUCCESS/MT_FAILURE

*/
mt_s32 DRV_PQ_GetTnrGlobalMotion(MT_PQ_TNR_MOTION_PARAM_IN_S* pstParamIn, MT_PQ_TNR_MOTION_RESULT_S* pstMotionResult)
{
#if 0
    if (pstParamIn == NULL || pstMotionResult == NULL)
    {
        MT_ERR_PQ("DRV_PQ_GetTnrGlobalMotion param-pointer is null!\n");
        return MT_FAILURE;
    }

    return PQ_MNG_TnrGetGlobalMotion((TNR_MOTION_PARAM_IN_S*)pstParamIn, (TNR_MOTION_RESULT_S*)pstMotionResult);
#else
    return MT_SUCCESS;
#endif
}


/**
\brief     :根据Global Motion获取DB的自适应强度值API

\attention :函数内对多实例的ID u32HdlNo 不做活动性检测，仅判断阈值
            隔行或者逐行信号源所采用的Global Motion不同；
            算法内已经将输出参数u8HWeight和u8VWeight配到寄存器VPSS_DB_WEIGHT中
            算法提供：晏秀梅 y00224511；移植&维护：吕明君 l00268071

\param[in] :pstParamIn结构，包含如下元素
            u32HdlNo 实例的ID
            u8SCDStr 是根据SCD的api计算得出的一个值[0,255]
            pstReg   global motion 寄存器地址
            eType 数据源类型，隔行或逐行，是否做DEI

\param[out]:pstDBstr  DB强度值。对应寄存器VPSS_DB_WEIGHT，分别为u8HWeight和u8VWeight

\retval    :MT_SUCCESS/MT_FAILURE

*/
extern mt_s32 DRV_PQ_GetAdaptiveDBStrength(MT_PQ_DB_STR_PARAM_IN_S* pstParamIn, MT_PQ_DB_WEIGHT_S* pstDBstr)
{
    return MT_SUCCESS;
}


/**
 \brief 逐行FMD模式检测
 \attention \n
无

 \param[in] pstVPSSReg: VPSS物理寄存器地址

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_PfmdDetect(MT_PQ_PFMD_CALC_S* pstPfmdCalc, MT_PQ_PFMD_PLAYBACK_S* pstPfmdResult)
{
    return MT_SUCCESS;
}

/**
 \brief 隔行FMD模式检测
 \attention \n
无

 \param[in] pstIfmdCalc  算法所需参数(统计信息地址，场序，顶底场标识等)
 \param[out] pstIfmdResult IFMD计算结果
 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_IfmdDect(MT_PQ_IFMD_CALC_S* pstIfmdCalc, MT_PQ_IFMD_PLAYBACK_S* pstIfmdResult)
{

    return MT_SUCCESS;
}

/**
 \brief 获取DCI直方图
 \attention \n
无

 \param[in] *pstDciHist:0-255

 \retval ::MT_SUCCESS

 */

mt_s32 DRV_PQ_GetDciHistgram(MT_PQ_HISTGRAM_S* pstDciHist)
{
    return MT_SUCCESS;
}

/**
 \brief 显示PQ状态信息
 \attention \n
无

 \param[in] *s;

 \retval ::MT_SUCCESS

 */
mt_s32 DRV_PQ_ProcRead(struct seq_file* s, mt_void* data)
{
    mt_u32 u32SDBrightness = 0;
    mt_u32 u32SDContrast = 0;
    mt_u32 u32SDHue = 0;
    mt_u32 u32SDSaturation = 0;
    mt_u32 u32HDBrightness = 0;
    mt_u32 u32HDContrast = 0;
    mt_u32 u32HDHue = 0;
    mt_u32 u32HDSaturation = 0;
    mt_u32 u32Sharpness = 0;
    mt_u32 u32FleshTone = 0;
    mt_u32 u32DciLevelGain = 0;
    mt_u32 u32OnOff = 0;

    PROC_PRINT(s, "\n------------------------ PQ  Driver info --------------------------\n");
    PROC_PRINT(s, "------------ PQ Driver Version = %s\n", PQ_VERSION);
//    PROC_PRINT(s, "------------ PQ Driver Bin Size = %ld\n", sizeof(PQ_PARAM_S));

    if (MT_FALSE == g_bLoadPqBin)
    {
        PROC_PRINT(s, "------------ Load PQ Bin Failure\n");
    }
    else
    {
        PROC_PRINT(s, "------------ PQ Driver Get Flash Data : Success\n");
        PROC_PRINT(s, "------------ PQ Bin Info Version = %s\n", g_pstPqParam->stPQFileHeader.u8Version);
        PROC_PRINT(s, "------------ PQ Bin Info Chipname = %s\n", g_pstPqParam->stPQFileHeader.u8ChipName);
        PROC_PRINT(s, "------------ PQ Bin Info Sdkversion = %s\n", g_pstPqParam->stPQFileHeader.u8SDKVersion);
        PROC_PRINT(s, "------------ PQ Bin Info Author = %s\n", g_pstPqParam->stPQFileHeader.u8Author);
        PROC_PRINT(s, "------------ PQ Bin Info Describe = %s\n", g_pstPqParam->stPQFileHeader.u8Desc);
        PROC_PRINT(s, "------------ PQ Bin Info Time = %s\n", g_pstPqParam->stPQFileHeader.u8Time);
    }

#if 0
    if (SOURCE_MODE_SD == sg_u32SourceMode)
    {
        PROC_PRINT(s, "------------ Source Type : SD\n");
    }
    else if (SOURCE_MODE_HD == sg_u32SourceMode)
    {
        PROC_PRINT(s, "------------ Source Type : HD\n");
    }
    if (SOURCE_MODE_UHD == sg_u32SourceMode)
    {
        PROC_PRINT(s, "------------ Source Type : UHD\n");
    }

    if (OUTPUT_MODE_SD == sg_u32OutputMode)
    {
        PROC_PRINT(s, "------------ Output Type : SD\n");
    }
    else if (OUTPUT_MODE_HD == sg_u32OutputMode)
    {
        PROC_PRINT(s, "------------ Output Type : HD\n");
    }
    if (OUTPUT_MODE_UHD == sg_u32OutputMode)
    {
        PROC_PRINT(s, "------------ Output Type : UHD\n");
    }
#endif

    /* Picture Setting Information */
    //PROC_PRINT(s, "\n---------------Picture Setting Information---------------\n");

    DRV_PQ_GetSDBrightness(&u32SDBrightness);
    DRV_PQ_GetSDContrast(&u32SDContrast);
    DRV_PQ_GetSDHue(&u32SDHue);
    DRV_PQ_GetSDSaturation(&u32SDSaturation);
    DRV_PQ_GetHDBrightness(&u32HDBrightness);
    DRV_PQ_GetHDContrast(&u32HDContrast);
    DRV_PQ_GetHDHue(&u32HDHue);
    DRV_PQ_GetHDSaturation(&u32HDSaturation);
    DRV_PQ_GetSharpness(&u32Sharpness);
    DRV_PQ_GetFleshToneLevel(&u32FleshTone);
//    PQ_MNG_GetDCILevelGain(&u32DciLevelGain);

    PROC_PRINT(s, "\n---------------------------Dispaly 0-------------------------------\n");
    PROC_PRINT(s, "Brightness = %d\n" "Contrast = %d\n" "Hue = %d\n" "Saturation = %d\n", \
               u32SDBrightness, u32SDContrast, \
               u32SDHue,  u32SDSaturation);

    PROC_PRINT(s, "\n---------------------------Dispaly 1-------------------------------\n");
    PROC_PRINT(s, "Brightness = %d\n" "Contrast = %d\n" "Hue = %d\n" "Saturation = %d\n", \
               u32HDBrightness, u32HDContrast, u32HDHue, u32HDSaturation);
    PROC_PRINT(s, "\n");
    PROC_PRINT(s, "Sharpness = %d\n" "DciLevelGain = %d\n", u32Sharpness, u32DciLevelGain);

    if (0 == u32FleshTone)
    {
        PROC_PRINT(s, "FleshTone = off\n");
    }
    else if (1 == u32FleshTone)
    {
        PROC_PRINT(s, "FleshTone = low\n");
    }
    else if (2 == u32FleshTone)
    {
        PROC_PRINT(s, "FleshTone = middle\n");
    }
    if (3 == u32FleshTone)
    {
        PROC_PRINT(s, "FleshTone = high\n");
    }

    PROC_PRINT(s, "\n");
    DRV_PQ_GetPQModule(MT_PQ_MODULE_SHARPNESS, &u32OnOff);
    PROC_PRINT(s, "Module Sharpness: %s\n",  u32OnOff ? "on" : "off");
    DRV_PQ_GetPQModule(MT_PQ_MODULE_DCI, &u32OnOff);
    PROC_PRINT(s, "Module DCI      : %s\n",  u32OnOff ? "on" : "off");
    DRV_PQ_GetPQModule(MT_PQ_MODULE_COLOR, &u32OnOff);
    PROC_PRINT(s, "Module Color    : %s\n",  u32OnOff ? "on" : "off");
    DRV_PQ_GetPQModule(MT_PQ_MODULE_SR, &u32OnOff);
    PROC_PRINT(s, "Module SR       : %s\n",  u32OnOff ? "on" : "off");

    PROC_PRINT(s, "\n-------------------------------------------------------------------\n");

    return MT_SUCCESS;
}

mt_s32 DRV_PQ_GetPqParam(mt_void)
{

    return MT_SUCCESS;
}

//static mt_s32 DRV_PQ_GetFlashPqBin(PQ_PARAM_S* pstPqParam)
//{
//    return MT_SUCCESS;
//}

mt_s32 DRV_PQ_GetBinPhyAddr(mt_u32* pu32Addr)
{
    *pu32Addr = g_stPqBinBuf.startPhyAddr;

    return MT_SUCCESS;
}

mt_s32 DRV_PQ_GetPicSetting(mt_void)
{
    //mt_s32 s32Ret;
    //MT_DISP_PARAM_S stSDDispParam;
    //MT_DISP_PARAM_S stHDDispParam;
    //PDM_EXPORT_FUNC_S* pstPdmFuncs = MT_NULL;


    return MT_SUCCESS;
}

mt_s32 DRV_PQ_SetAcmLuma(MT_PQ_ACM_LUT_S* pstAttr)
{
    return MT_SUCCESS;
}

mt_s32 DRV_PQ_SetAcmHue(MT_PQ_ACM_LUT_S* pstAttr)
{
    return MT_SUCCESS;
}

mt_s32 DRV_PQ_SetAcmSat(MT_PQ_ACM_LUT_S* pstAttr)
{
    return MT_SUCCESS;
}


mt_s32 DRV_PQ_SetReg(MT_PQ_REGISTER_S* pstAttr)
{
    return MT_SUCCESS;
}


mt_s32 DRV_PQ_GetReg(MT_PQ_REGISTER_S* pstAttr)
{
    mt_u32 u32Addr, u32Value;
    MT_U8  u8Lsb, u8Msb, u8SourceMode, u8OutputMode;
    mt_u32 i;

    PQ_CHECK_NULL_PTR(g_pstPqParam);

    for (i = 0; i < PHY_REG_MAX; i++)
    {
        u32Addr       = g_pstPqParam->stPQPhyReg[i].u32RegAddr;
        u32Value      = g_pstPqParam->stPQPhyReg[i].u32Value;
        u8Lsb         = g_pstPqParam->stPQPhyReg[i].u8Lsb;
        u8Msb         = g_pstPqParam->stPQPhyReg[i].u8Msb;
        u8SourceMode  = g_pstPqParam->stPQPhyReg[i].u8SourceMode;
        u8OutputMode  = g_pstPqParam->stPQPhyReg[i].u8OutputMode;

        if (u32Addr != pstAttr->u32RegAddr)
        {
            continue;
        }

        if (u8Lsb != pstAttr->u8Lsb)
        {
            continue;
        }

        if (u8Msb != pstAttr->u8Msb)
        {
            continue;
        }

#if 0
        if ((SOURCE_MODE_NO != u8SourceMode) && (u8SourceMode != pstAttr->u8SourceMode))
        {
            continue;
        }

        if ((OUTPUT_MODE_NO != u8OutputMode) && (u8OutputMode != pstAttr->u8OutputMode))
        {
            continue;
        }
#endif

        pstAttr->u32Value = u32Value;
        return MT_SUCCESS;
        //MT_INFO_PQ("Load RegType:%d, SourceMode:%d, Module:%d\n", enRegType, u32SourceMode, u32Module);
    }

    MT_ERR_PQ("Error! not find Phy Register List[Address:0x%x, Bit:%d~%d],SourceMode:[%d], OutputMode:[%d]\n", \
              pstAttr->u32RegAddr, pstAttr->u8Lsb, pstAttr->u8Msb, pstAttr->u8SourceMode, \
              pstAttr->u8OutputMode);

    return MT_FAILURE;
}

#if 0
static mt_s32 DRV_PQ_CheckChipName(MT_CHAR* pchChipName)
{
    mt_s32 s32Ret = MT_FAILURE;
    PQ_CHECK_NULL_PTR(pchChipName);

#if defined(CHIP_TYPE_symphony_A0)
    s32Ret = strncmp(pchChipName , "MT_CHIP_TYPE_SYMPHONY_A0", strlen("MT_CHIP_TYPE_SYMPHONY_A0"));
#elif defined(CHIP_TYPE_symphony_A1)
    s32Ret = strncmp(pchChipName , "MT_CHIP_TYPE_SYMPHONY_A1", strlen("MT_CHIP_TYPE_SYMPHONY_A1"));
#elif defined(CHIP_TYPE_symphony_A2)
    s32Ret = strncmp(pchChipName , "MT_CHIP_TYPE_SYMPHONY_A2", strlen("MT_CHIP_TYPE_SYMPHONY_A2"));

#else
    MT_ERR_PQ("unknown Chip Type \r\n");
    return MT_FAILURE;
#endif

    if (0 != s32Ret)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}
#endif

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(DRV_PQ_UpdateVpssPQ);
EXPORT_SYMBOL(DRV_PQ_UpdateVdpPQ);
EXPORT_SYMBOL(DRV_PQ_UpdateVdpCSC);
EXPORT_SYMBOL(DRV_PQ_UpdateDCIWin);
EXPORT_SYMBOL(DRV_PQ_GetVpssScalerCoef);
EXPORT_SYMBOL(DRV_PQ_GetScalerCoef);
EXPORT_SYMBOL(DRV_PQ_GetWbcScalerCoef);
EXPORT_SYMBOL(DRV_PQ_GetWbcInfo);
EXPORT_SYMBOL(DRV_PQ_SetAlgCalcCfg);
//EXPORT_SYMBOL(DRV_PQ_SetDeiGlobalMotion);
EXPORT_SYMBOL(DRV_PQ_GetTnrGlobalMotion);
EXPORT_SYMBOL(DRV_PQ_GetAdaptiveDBStrength);
EXPORT_SYMBOL(DRV_PQ_GetCSCCoef);
EXPORT_SYMBOL(DRV_PQ_Get8BitCSCCoef);
EXPORT_SYMBOL(DRV_PQ_PfmdDetect);
EXPORT_SYMBOL(DRV_PQ_IfmdDect);
EXPORT_SYMBOL(DRV_PQ_GetDciHistgram);
EXPORT_SYMBOL(DRV_PQ_GetHDPictureSetting);
EXPORT_SYMBOL(DRV_PQ_GetSDPictureSetting);
EXPORT_SYMBOL(DRV_PQ_SetZme);
EXPORT_SYMBOL(DRV_PQ_SetVpssZme);
EXPORT_SYMBOL(DRV_PQ_SetHDPictureSetting);
EXPORT_SYMBOL(DRV_PQ_SetSDPictureSetting);
EXPORT_SYMBOL(DRV_PQ_SetSplitBlockZme);
EXPORT_SYMBOL(DRV_PQ_ZME_2L_Check);



