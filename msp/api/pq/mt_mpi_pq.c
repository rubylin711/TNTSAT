/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_pq.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/02/27
  Description   :
  History       :
  1.Date        : 2017/02/27
    Author      :
    Modification:

*********************************************************************************************/

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "mt_type.h"
#include "mt_mpi_pq.h"
#include "mt_module_debug.h"



/* PQ设备文件描述符 */
static mt_s32 sg_s32PQFd = -1;

static MT_BOOL sg_bPQInitFlag = MT_FALSE;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/**
 \PQ初始化
 \attention \n

 \param[in] pszPath:PQ配置文件路径
 \param[out]

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_Init(mt_char* pszPath)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_PATE_S stPqPath;

    if (MT_TRUE == sg_bPQInitFlag)
    {
        return MT_SUCCESS;
    }

    /* PQ设备初始化 */
    sg_s32PQFd = open("/dev/mt_pq", O_RDWR | O_NONBLOCK| O_CLOEXEC);
    if (sg_s32PQFd <= 0)
    {
        MT_ERR_PQ("PQ device open error!");
        return MT_FAILURE;
    }
    sg_bPQInitFlag = MT_TRUE;

    /* PQ配置文件路径*/
    if (pszPath != NULL)
    {
        memset(&stPqPath, 0, sizeof(MT_PQ_PATE_S));
        strncpy(stPqPath.cPqPath, pszPath, sizeof(stPqPath.cPqPath) - 1);
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_PQ_PATH, &stPqPath);
        if (s32Ret != MT_SUCCESS)
        {
            MT_ERR_PQ("set pq bin path error!");
        }
    }
    else
    {
        //MT_ERR_PQ("pq bin path is invalid,Use default PQ setting!");
    }
    return MT_SUCCESS;
}

/**
 \PQ去初始化
 \attention \n

 \param[in]
 \param[out]

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_DeInit(mt_void)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (MT_FALSE == sg_bPQInitFlag)
    {
        return MT_SUCCESS;
    }

    sg_bPQInitFlag = MT_FALSE;
    s32Ret = close(sg_s32PQFd);
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

mt_s32 MT_MPI_PQ_GetBrightness(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Brightness)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32RetNum = 0;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SD_BRIGHTNESS, &u32RetNum);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_HD_BRIGHTNESS, &u32RetNum);
    }

    *pu32Brightness = u32RetNum;
    return s32Ret;
}

/**
 \brief 设置亮度
 \attention \n
无

 \param[in] u32Brightness, 亮度值,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetBrightness(MT_DRV_DISPLAY_E enChan, mt_u32 u32Brightness)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 Brightness = u32Brightness;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SD_BRIGHTNESS, &Brightness);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_HD_BRIGHTNESS, &Brightness);
    }

    return s32Ret;
}


/**
 \brief 获取对比度
 \attention \n
无

 \param[in]
 \param[out] pu32Contrast 对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetContrast(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Contrast)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32RetNum = 0;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SD_CONTRAST, &u32RetNum);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_HD_CONTRAST, &u32RetNum);
    }

    *pu32Contrast = u32RetNum;
    return s32Ret;
}

/**
 \brief 设置对比度
 \attention \n
无

 \param[in] u32Contrast, 对比度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetContrast(MT_DRV_DISPLAY_E enChan, mt_u32 u32Contrast)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 Contrast = u32Contrast;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SD_CONTRAST, &Contrast);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_HD_CONTRAST, &Contrast);
    }

    return s32Ret;
}

/**
 \brief 获取饱和度
 \attention \n
无

 \param[out] pu32Saturation：饱和度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetSaturation(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Saturation)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32RetNum = 0;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SD_SATURATION, &u32RetNum);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_HD_SATURATION, &u32RetNum);
    }

    *pu32Saturation = u32RetNum;
    return s32Ret;
}

/**
 \brief 设置饱和度
 \attention \n
无

 \param[in] u32Saturation：饱和度,有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSaturation(MT_DRV_DISPLAY_E enChan, mt_u32 u32Saturation)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 Saturation = u32Saturation;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SD_SATURATION, &Saturation);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_HD_SATURATION, &Saturation);
    }

    return s32Ret;
}

/**
 \brief 获取色调
 \attention \n
无

 \param[in]
 \param[out] pu32Hue：色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetHue(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Hue)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32RetNum = 0;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SD_HUE, &u32RetNum);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_HD_HUE, &u32RetNum);
    }

    *pu32Hue = u32RetNum;
    return s32Ret;
}

/**
 \brief 设置色调
 \attention \n
无

 \param[in] u32Hue：色调, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetHue(MT_DRV_DISPLAY_E enChan, mt_u32 u32Hue)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 Hue = u32Hue;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (MT_DRV_DISPLAY_0 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SD_HUE, &Hue);
    }
    else if (MT_DRV_DISPLAY_1 == enChan)
    {
        s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_HD_HUE, &Hue);
    }

    return s32Ret;
}

/**
 \brief 获取降噪强度
 \attention \n
无

 \param[out] pu32NRLevel: 降噪等级, 有效范围: 0~255


 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetNR(mt_u32* pu32NRLevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_NR, pu32NRLevel);

    return s32Ret;
}

/**
 \brief 设置降噪强度
 \attention \n
无

 \param[in] u32NRLevel: 降噪等级, 有效范围: 0~255

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetNR(mt_u32 u32NRLevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_NR, &u32NRLevel);

    return s32Ret;
}

/**
 \brief 获取自动降噪开关状态
 \attention \n
无

 \param[out] pu32OnOff


 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetNRAutoMode(mt_u32* pu32OnOff)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_NR_AUTO, pu32OnOff);

    return s32Ret;
}

/**
 \brief 设置降噪自动模式开关
 \attention \n
无

 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetNRAutoMode(mt_u32 u32OnOff)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_NR_AUTO, &u32OnOff);

    return s32Ret;
}

/**
 \brief 获取SR演示类型
 \attention \n
无

 \param[out] *penType


 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetSRMode(MT_PQ_SR_DEMO_E* penType)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SR_DEMO, penType);

    return s32Ret;
}

/**
 \brief 设置SR演示类型
 \attention \n
无

 \param[in] enType

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSRMode(MT_PQ_SR_DEMO_E enType)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SR_DEMO, &enType);

    return s32Ret;
}

/**
 \brief 获取清晰度
 \attention \n
无

 \param[out] pu32Sharpness：清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetSharpness(mt_u32* pu32Sharpness)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Num = 0;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SHARPNESS, &u32Num);
    *pu32Sharpness = u32Num;

    return s32Ret;
}

/**
 \brief 设置清晰度
 \attention \n
无

 \param[in] u32Sharpness：清晰度, 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSharpness(mt_u32 u32Sharpness)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Num = u32Sharpness;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SHARPNESS, &u32Num);

    return s32Ret;
}

/**
 \brief 获取块降噪De-blocking强度
 \attention \n
无

 \param[out] pu32DBlevel：降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetDeBlocking(mt_u32* pu32DBlevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_DB, pu32DBlevel);

    return s32Ret;
}

/**
 \brief 设置块降噪De-blocking强度
 \attention \n
无

 \param[in] u32DBlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetDeBlocking(mt_u32 u32DBlevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_DB, &u32DBlevel);

    return s32Ret;
}

/**
 \brief 获取去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[out] pu32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetDeRinging(mt_u32* pu32DRlevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_DR, pu32DRlevel);

    return s32Ret;
}


/**
 \brief 设置去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[in] u32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetDeRinging(mt_u32 u32DRlevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_DR, &u32DRlevel);

    return s32Ret;
}

/**
 \brief 获取色温参数
 \attention \n
无

 \param[in] pstColorTemp：色温参数
 \param[out]

 \retval ::MT_SUCCESS

 */

/*mt_s32 MT_MPI_PQ_GetColorTemp( MT_UNF_PQ_COLOR_TEMP_S *pstColorTemp)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_COLOR_TEMP_S stColorTemp = {0};

    if(sg_bPQInitFlag==MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd,MTIOC_PQ_G_COLORTEMP,&stColorTemp);
    if(MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }
    pstColorTemp->u32BlueGain    = (mt_u32)stColorTemp.s16BlueGain;
    pstColorTemp->u32GreenGain   = (mt_u32)stColorTemp.s16GreenGain;
    pstColorTemp->u32RedGain     = (mt_u32)stColorTemp.s16RedGain;
    pstColorTemp->u32BlueOffset  = (mt_u32)stColorTemp.s16BlueOffset;
    pstColorTemp->u32GreenOffset = (mt_u32)stColorTemp.s16GreenOffset;
    pstColorTemp->u32RedOffset   = (mt_u32)stColorTemp.s16RedOffset;

    return MT_SUCCESS;
}
*/


/**
 \brief 设置色温参数
 \attention \n
无

 \param[in] pstColorTemp:色温参数
 \param[out]

 \retval ::MT_SUCCESS

 */

/*mt_s32 MT_MPI_PQ_SetColorTemp(MT_UNF_PQ_COLOR_TEMP_S *pstColorTemp)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_COLOR_TEMP_S stColorTemp = {0};

    if(sg_bPQInitFlag==MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    stColorTemp.s16BlueGain    = (mt_s16)pstColorTemp->u32BlueGain;
    stColorTemp.s16GreenGain   = (mt_s16)pstColorTemp->u32GreenGain;
    stColorTemp.s16RedGain     = (mt_s16)pstColorTemp->u32RedGain;
    stColorTemp.s16BlueOffset  = (mt_s16)pstColorTemp->u32BlueOffset;
    stColorTemp.s16GreenOffset = (mt_s16)pstColorTemp->u32GreenOffset;
    stColorTemp.s16RedOffset   = (mt_s16)pstColorTemp->u32RedOffset;
    s32Ret = ioctl(sg_s32PQFd,MTIOC_PQ_S_COLORTEMP,&stColorTemp);

    return MT_SUCCESS;
}*/


/**
 \brief 获取颜色增强
 \attention \n
无

 \param[out] pu32ColorGainLevel 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetColorGain(mt_u32* pu32ColorGainLevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Num = 0;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_COLORGAIN, &u32Num);
    *pu32ColorGainLevel = u32Num;

    return s32Ret;
}

/**
 \brief 设置颜色增强
 \attention \n
无

 \param[in] u32ColorGainLevel 有效范围: 0~100;

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetColorGain(mt_u32 u32ColorGainLevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Num = u32ColorGainLevel;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_COLORGAIN, &u32Num);

    return s32Ret;
}

/**
 \brief 获取肤色增强
 \attention \n
  无

 \param[out] penFleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetFleshTone(MT_UNF_PQ_FLESHTONE_E* penFleshToneLevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_FLESHTONE, penFleshToneLevel);

    return s32Ret;
}

/**
 \brief 设置肤色增强
 \attention \n
  无

 \param[in] enFleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetFleshTone( MT_UNF_PQ_FLESHTONE_E enFleshToneLevel)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_FLESHTONE, &enFleshToneLevel);

    return s32Ret;
}


/**
 \brief 设置颜色增强类型
 \attention \n
  无

 \param[in] enSixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetColorEnhanceMode(MT_PQ_COLOR_SPEC_MODE_E enColorMode)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_COLOR_ENHANCE_MODE, &enColorMode);

    return s32Ret;
}


/**
 \brief 获取颜色增强类型
 \attention \n
  无

 \param[in] enSixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetColorEnhanceMode(MT_UNF_PQ_COLOR_SPEC_MODE_E* penColorMode)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }


    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_COLOR_ENHANCE_MODE, penColorMode);

    return s32Ret;
}

/**
 \brief 获取六基色类型
 \attention \n
  无

 \param[out] pu32SixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetSixBaseColor(MT_UNF_PQ_SIX_BASE_S* pstSixBaseColor)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SIXBASECOLOR, pstSixBaseColor);

    return s32Ret;
}

/**
 \brief 设置六基色类型
 \attention \n
  无

 \param[in] enSixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSixBaseColor(MT_PQ_SIX_BASE_COLOR_S* pstSixBaseColor)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SIXBASECOLOR, pstSixBaseColor);

    return s32Ret;
}

/**
 \brief 获取PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] pu32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetPQModule( MT_UNF_PQ_MODULE_E enFlags, mt_u32* pu32OnOff)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_MODULE_S stPQModule = {MT_PQ_MODULE_SHARPNESS, 0};

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }


    if (MT_UNF_PQ_MODULE_SHARPNESS == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_SHARPNESS;
    }
    else if (MT_UNF_PQ_MODULE_DCI == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_DCI;
    }
    else if (MT_UNF_PQ_MODULE_COLOR == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_COLOR;
    }
    else if (MT_UNF_PQ_MODULE_SR == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_SR;
    }
    else
    {
        MT_ERR_PQ("PQ MODULE is error!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_MODULE, &stPQModule);
    *pu32OnOff = stPQModule.u32OnOff;

    return s32Ret;
}

/**
 \brief 设置PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetPQModule(MT_UNF_PQ_MODULE_E enFlags, mt_u32 u32OnOff)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_MODULE_S stPQModule = {MT_PQ_MODULE_SHARPNESS, 0};

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    stPQModule.u32OnOff = u32OnOff;

    if (MT_UNF_PQ_MODULE_SHARPNESS == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_SHARPNESS;
    }
    else if (MT_UNF_PQ_MODULE_DCI == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_DCI;
    }
    else if (MT_UNF_PQ_MODULE_COLOR == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_COLOR;
    }
    else if (MT_UNF_PQ_MODULE_SR == enFlags)
    {
        stPQModule.enModule = MT_PQ_MODULE_SR;
    }
    else
    {
        MT_ERR_PQ("PQ MODULE is error!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_MODULE, &stPQModule);

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

mt_s32 MT_MPI_PQ_SetDemo( MT_UNF_PQ_DEMO_E enFlags, mt_u32 u32OnOff)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_DEMO_S stDemo;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    if (1 == u32OnOff)
    {
        stDemo.bOnOff = MT_TRUE;
    }
    else
    {
        stDemo.bOnOff = MT_FALSE;
    }

    if (MT_UNF_PQ_DEMO_SHARPNESS == enFlags)
    {
        stDemo.enModule = MT_PQ_DEMO_SHARPNESS;
    }
    else if (MT_UNF_PQ_DEMO_DCI == enFlags)
    {
        stDemo.enModule = MT_PQ_DEMO_DCI;
    }
    else if (MT_UNF_PQ_DEMO_COLOR == enFlags)
    {
        stDemo.enModule = MT_PQ_DEMO_COLOR;
    }
    else if (MT_UNF_PQ_DEMO_SR == enFlags)
    {
        stDemo.enModule = MT_PQ_DEMO_SR;
    }
    else if (MT_UNF_PQ_DEMO_ALL == enFlags)
    {
        stDemo.enModule = MT_PQ_DEMO_ALL;
    }
    else
    {
        MT_ERR_PQ("PQ DEMO is error!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_DEMO, &stDemo);
    return s32Ret;
}

/**
 \brief 设置PQ寄存器
 \attention \n
无

 \param[in] u32RegAddr
 \param[in] u32Data

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_WritePQRegister(mt_u32 u32RegAddr, mt_u32 u32Data)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_REGISTER_S stRegister = {0};

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    stRegister.u32RegAddr = u32RegAddr;
    stRegister.u32Value = u32Data;
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_REGISTER, &stRegister);
    return s32Ret;
}

/**
 \brief 获取PQ寄存器
 \attention \n
无

 \param[in] u32RegAddr
 \param[in] *pu32Value

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_ReadPQRegister(mt_u32 u32RegAddr, mt_u32* pu32Value)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_REGISTER_S stRegister = {0};

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    stRegister.u32RegAddr = u32RegAddr;
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_REGISTER, &stRegister);
    *pu32Value = stRegister.u32Value;
    return s32Ret;
}

/**
 \brief 获取ACM曲线
 \attention \n
  无

 \param[in] *pstGammaTable

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetAcmTable( MPI_ACM_PARAM_S* pstColorTable)
{
    mt_s32 s32Ret = MT_FAILURE;

    /*MT_PQ_COLOR_S stColorLuma, stColorHue, stColorSat;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_ACM_LUMA, &stColorLuma);
    s32Ret |= ioctl(sg_s32PQFd, MTIOC_PQ_G_ACM_HUE, &stColorHue);
    s32Ret |= ioctl(sg_s32PQFd, MTIOC_PQ_G_ACM_SAT, &stColorSat);

    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    memcpy(pstColorTable->as16Luma, &stColorLuma, sizeof(MT_PQ_COLOR_S));
    memcpy(pstColorTable->as16Hue, &stColorHue, sizeof(MT_PQ_COLOR_S));
    memcpy(pstColorTable->as16Sat, &stColorSat, sizeof(MT_PQ_COLOR_S));*/
    return s32Ret;
}

/**
 \brief 设置ACM曲线
 \attention \n
  无

 \param[in] *pstGammaTable

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetAcmTable(mt_u32 u32RegAddr, MPI_ACM_PARAM_S* pstColorTable)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_PQ_ACM_LUT_S* pstAcmLut = MT_NULL;

    if (MT_NULL == pstColorTable)
    {
        MT_ERR_PQ("%s:pstColorTable is Null poniter!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    pstAcmLut = (MT_PQ_ACM_LUT_S*)malloc(sizeof(MT_PQ_ACM_LUT_S));
    if (MT_NULL == pstAcmLut)
    {
        MT_ERR_PQ("%s:pstAcmLut is Null poniter!\n", __FUNCTION__);
        return MT_FAILURE;
    }
    pstAcmLut->u32LutType = u32RegAddr;

    if (MT_FALSE == sg_bPQInitFlag)
    {
        MT_ERR_PQ("PQ not  init!");
        free(pstAcmLut);
        pstAcmLut = MT_NULL;
        return MT_FAILURE;
    }

    memcpy(pstAcmLut->as16Lut, pstColorTable->as16Luma, sizeof(pstAcmLut->as16Lut));
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_ACM_LUMA, pstAcmLut);

    memcpy(pstAcmLut->as16Lut, pstColorTable->as16Hue, sizeof(pstAcmLut->as16Lut));
    s32Ret |= ioctl(sg_s32PQFd, MTIOC_PQ_S_ACM_HUE, pstAcmLut);

    memcpy(pstAcmLut->as16Lut, pstColorTable->as16Sat, sizeof(pstAcmLut->as16Lut));
    s32Ret |= ioctl(sg_s32PQFd, MTIOC_PQ_S_ACM_SAT, pstAcmLut);

    free(pstAcmLut);
    pstAcmLut = MT_NULL;

    return s32Ret;
}

/**
 \brief 获取DCI曲线
 \attention \n
  无

 \param[in] *pstDCITable

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetDciTable( MT_PQ_DCI_WGT_S* pstDCITable)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_DCI, (MT_PQ_DCI_WGT_S*)pstDCITable);

    return s32Ret;
}

/**
 \brief 设置DCI曲线
 \attention \n
  无

 \param[in] *pstDCITable

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetDciTable( MT_PQ_DCI_WGT_S* pstDCITable)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_DCI, (MT_PQ_DCI_WGT_S*)pstDCITable);

    return s32Ret;
}

/**
 \brief 获取DCI直方图
 \attention \n
  无

 \param[in] *pstDCIHistgram

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetHistgram( MT_PQ_DCI_HISTGRAM_S* pstDCIHistgram)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_DCI_HIST, (MT_PQ_DCI_HISTGRAM_S*)pstDCIHistgram);

    return s32Ret;
}

/**
 \brief 获取DCI LEVEL
 \attention \n
  无

 \param[in] *pstDCITable

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetDciLevel(mt_u32* pu32DCIlevel)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_DCI_LEVEL, pu32DCIlevel);

    return s32Ret;
}

/**
 \brief 设置DCI LEVEL
 \attention \n
  无

 \param[in] *pstDCITable

 \retval ::MT_SUCCESS

 */


mt_s32 MT_MPI_PQ_SetDciLevel( mt_u32 u32DCIlevel)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_DCI_LEVEL, &u32DCIlevel);

    return s32Ret;
}

/**
 \brief 获取TNR的亮度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetTNRLumaPixMean2Ratio( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_TNR_Y_PIXMEAN_2_RATIO, (MT_PQ_TNR_S*)pstTnrData);
    return s32Ret;
}
/**
 \brief 设置TNR的亮度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetTNRLumaPixMean2Ratio( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_TNR_Y_PIXMEAN_2_RATIO, (MT_PQ_TNR_S*)pstTnrData);

    return s32Ret;
}

/**
 \brief 获取TNR的色度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetTNRChromPixMean2Ratio( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_TNR_C_PIXMEAN_2_RATIO, (MT_PQ_TNR_S*)pstTnrData);
    return s32Ret;
}
/**
 \brief 设置TNR的色度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetTNRChromPixMean2Ratio( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_TNR_C_PIXMEAN_2_RATIO, (MT_PQ_TNR_S*)pstTnrData);

    return s32Ret;
}

/**
 \brief 获取TNR的亮度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetTNRLumaMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_TNR_Y_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);
    return s32Ret;
}
/**
 \brief 设置TNR的亮度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetTNRLumaMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_TNR_Y_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);

    return s32Ret;
}

/**
 \brief 获取TNR的色度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetTNRChromMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_TNR_C_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);
    return s32Ret;
}
/**
 \brief 设置TNR的色度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetTNRChromMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_TNR_C_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);

    return s32Ret;
}

/**
 \brief 获取TNR的亮度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetTNRLumaFinalMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_TNR_Y_FINAL_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);
    return s32Ret;
}
/**
 \brief 设置TNR的亮度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetTNRLumaFinalMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_TNR_Y_FINAL_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);

    return s32Ret;
}

/**
 \brief 获取TNR的色度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetTNRChromFinalMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_TNR_C_FINAL_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);
    return s32Ret;
}
/**
 \brief 设置TNR的色度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetTNRChromFinalMotionMapping( MT_PQ_TNR_S* pstTnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_TNR_C_FINAL_MOTION_MAPPING, (MT_PQ_TNR_S*)pstTnrData);

    return s32Ret;
}

/**
 \brief 获取SNR的pixmean-ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetSNRPixmean2Ratio( MT_PQ_SNR_PIXMEAN_2_RATIO_S* pstSnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SNR_PIXMEAN_2_RATIO, (MT_PQ_SNR_PIXMEAN_2_RATIO_S*)pstSnrData);
    return s32Ret;
}
/**
 \brief 设置SNR的pixmean-ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSNRPixmean2Ratio( MT_PQ_SNR_PIXMEAN_2_RATIO_S* pstSnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SNR_PIXMEAN_2_RATIO, (MT_PQ_SNR_PIXMEAN_2_RATIO_S*)pstSnrData);

    return s32Ret;
}

/**
 \brief 获取SNR的pixdiff-edgestr曲线
 \attention \n
  无

 \param[in] *pstSnrData

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_GetSNRPixdiff2Edgestr( MT_PQ_SNR_PIXDIFF_2_EDGESTR_S* pstSnrData)
{
    mt_s32 s32Ret = MT_FAILURE;
    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }
    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_G_SNR_PIXDIFF_2_EDGESTR, (MT_PQ_SNR_PIXDIFF_2_EDGESTR_S*)pstSnrData);
    return s32Ret;
}
/**
 \brief 设置SNR的pixdiff-edgestr曲线
 \attention \n
  无

 \param[in] *pstSnrData

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSNRPixdiff2Edgestr( MT_PQ_SNR_PIXDIFF_2_EDGESTR_S* pstSnrData)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (sg_bPQInitFlag == MT_FALSE)
    {
        MT_ERR_PQ("PQ not  init!");
        return MT_FAILURE;
    }

    s32Ret = ioctl(sg_s32PQFd, MTIOC_PQ_S_SNR_PIXDIFF_2_EDGESTR, (MT_PQ_SNR_PIXDIFF_2_EDGESTR_S*)pstSnrData);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
