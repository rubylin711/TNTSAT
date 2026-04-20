/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_pq.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/02/27
  Description   :
  History       :
  1.Date        : 2017/02/27
    Author      : 
    Modification: 

*********************************************************************************************/

#ifndef __MT_MPI_PQ_V2_H__
#define __MT_MPI_PQ_V2_H__

#include "mt_type.h"
#include "mt_unf_pq.h"
#include "mt_drv_pq.h"
#include "mt_mpi_disp.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef MT_PQ_V2_0
#define ACM_Y_NUM            9
#define ACM_H_NUM            29
#define ACM_S_NUM            13
#endif

/*ACM table结构*/
typedef struct hiMPI_COLOR_PARAM_S
{
    mt_u16 u16Stretch;
    mt_u16 u16ClipRange;
    mt_u16 u16HueGain;         /*表示对Hue的增益，范围0-1023*/
    mt_u16 u16SatGain;         /*表示对Saturation的增益*/
    mt_u16 u16LumaGain;        /*表示对Luma的增益*/
    mt_u16 u16CbCrThre;        /*CbCr的门限值，若低于门限值就不做Color处理*/
    mt_s16 as16Luma[ACM_Y_NUM][ACM_S_NUM][ACM_H_NUM]; /*Luma查找表*/
    mt_s16 as16Hue[ACM_Y_NUM][ACM_S_NUM][ACM_H_NUM];  /*Hue查找表*/
    mt_s16 as16Sat[ACM_Y_NUM][ACM_S_NUM][ACM_H_NUM];  /*Saturation查找表*/

} MPI_COLOR_PARAM_S;

/*ACM 曲线存储结构*/
typedef struct hiMPI_ACM_PARAM_S
{
    mt_s16 as16Luma[ACM_Y_NUM][ACM_S_NUM][ACM_H_NUM]; /*Luma查找表*/
    mt_s16 as16Hue[ACM_Y_NUM][ACM_S_NUM][ACM_H_NUM];  /*Hue查找表*/
    mt_s16 as16Sat[ACM_Y_NUM][ACM_S_NUM][ACM_H_NUM];  /*Saturation查找表*/
} MPI_ACM_PARAM_S;



/**
 \PQ初始化
 \attention \n

 \param[in] pszPath:PQ配置文件路径
 \param[out]

 \retval ::MT_SUCCESS

 */
mt_s32 MT_MPI_PQ_Init(MT_CHAR* pszPath);

/**
 \PQ去初始化
 \attention \n

 \param[in]
 \param[out]

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_DeInit(MT_VOID);

/**
 \brief 获取亮度
 \attention \n
无

 \param[in] pu32Brightness：亮度值,有效范围: 0~255;
 \param[out]

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetBrightness(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Brightness);

/**
 \brief 设置亮度
 \attention \n
无

 \param[in] u32Brightness：亮度值,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetBrightness(MT_DRV_DISPLAY_E enChan, mt_u32 u32Brightness);

/**
 \brief 获取对比度
 \attention \n
无

 \param[in]
 \param[out] pu32Contrast：对比度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetContrast(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Contrast);

/**
 \brief 设置对比度
 \attention \n
无

 \param[in] u32Contrast：对比度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetContrast(MT_DRV_DISPLAY_E enChan, mt_u32 u32Contrast);

/**
 \brief 获取色调
 \attention \n
无

 \param[in]
 \param[out] pu32Hue：色调, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetHue(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Hue);

/**
 \brief 设置色调
 \attention \n
无

 \param[in] u32Hue：色调, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetHue(MT_DRV_DISPLAY_E enChan, mt_u32 u32Hue);

/**
 \brief 获取饱和度
 \attention \n
无

 \param[out] pu32Saturation：饱和度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetSaturation(MT_DRV_DISPLAY_E enChan, mt_u32* pu32Saturation);

/**
 \brief 设置饱和度
 \attention \n
无

 \param[in] u32Saturation：饱和度,有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetSaturation(MT_DRV_DISPLAY_E enChan, mt_u32 u32Saturation);

/**
 \brief 获取降噪强度
 \attention \n
无

 \param[out] pu32NRLevel: 降噪等级, 有效范围: 0~255


 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetNR(mt_u32* pu32NRLevel);

/**
 \brief 设置降噪强度
 \attention \n
无

 \param[in] u32NRLevel: 降噪等级, 有效范围: 0~255

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetNR(mt_u32 u32NRLevel);

/**
 \brief 获取自动降噪开关状态
 \attention \n
无

 \param[out] pu32OnOff


 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetNRAutoMode(mt_u32* pu32OnOff);

/**
 \brief 设置降噪自动模式开关
 \attention \n
无

 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetNRAutoMode(mt_u32 u32OnOff);

/**
 \brief 获取SR演示类型
 \attention \n
无

 \param[out] *penType


 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetSRMode(MT_PQ_SR_DEMO_E* penType);

/**
 \brief 设置SR演示类型
 \attention \n
无

 \param[in] enType

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSRMode(MT_PQ_SR_DEMO_E enType);

/**
 \brief 获取清晰度
 \attention \n
无

 \param[out] pu32Sharpness：清晰度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetSharpness(mt_u32* pu32Sharpness);

/**
 \brief 设置清晰度
 \attention \n
无

 \param[in] u32Sharpness：清晰度, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetSharpness(mt_u32 u32Sharpness);

/**
 \brief 获取块降噪De-blocking强度
 \attention \n
无

 \param[out] pu32DBlevel：降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetDeBlocking(mt_u32* pu32DBlevel);

/**
 \brief 设置块降噪De-blocking强度
 \attention \n
无

 \param[in] u32DBlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetDeBlocking(mt_u32 u32DBlevel);

/**
 \brief 获取去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[out] pu32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetDeRinging(mt_u32* pu32DRlevel);

/**
 \brief 设置去除蚊虫噪声de-ringing强度
 \attention \n
无

 \param[in] u32DRlevel:降噪等级, 有效范围: 0~255;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetDeRinging(mt_u32 u32DRlevel);


/**
 \brief 获取颜色增强
 \attention \n
无

 \param[out] pu32ColorGainLevel

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetColorGain(mt_u32* pu32ColorGainLevel);

/**
 \brief 设置颜色增强
 \attention \n
无

 \param[in] enColorGainLevel

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetColorGain(mt_u32 u32ColorGainLevel);

/**
 \brief 获取肤色增强
 \attention \n
  无

 \param[out] penFleshToneLevel

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetFleshTone(MT_UNF_PQ_FLESHTONE_E* penFleshToneLevel);

/**
 \brief 设置颜色增强类型
 \attention \n
  无

 \param[in] enSixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetColorEnhanceMode(MT_PQ_COLOR_SPEC_MODE_E enColorMode);

/**
 \brief 获取颜色增强类型
 \attention \n
  无

 \param[in] enSixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetColorEnhanceMode(MT_UNF_PQ_COLOR_SPEC_MODE_E* penColorMode);


/**
 \brief 设置肤色增强
 \attention \n
  无

 \param[in] enFleshToneLevel，参考MT_COLOR_GAIN_E

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetFleshTone( MT_UNF_PQ_FLESHTONE_E enFleshToneLevel);

/**
 \brief 获取六基色类型
 \attention \n
  无

 \param[out] pstSixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_GetSixBaseColor(MT_UNF_PQ_SIX_BASE_S* pstSixBaseColor);

/**
 \brief 设置六基色类型
 \attention \n
  无

 \param[in] stSixBaseColor

 \retval ::MT_SUCCESS

 */

mt_s32 MT_MPI_PQ_SetSixBaseColor(MT_PQ_SIX_BASE_COLOR_S* pstSixBaseColor);

/**
 \brief 设置PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetPQModule( MT_UNF_PQ_MODULE_E enFlags, mt_u32 u32OnOff);

/**
 \brief 获取PQ模块开关
 \attention \n
  无

 \param[in] enFlags
 \param[in] pu32OnOff

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetPQModule( MT_UNF_PQ_MODULE_E enFlags, mt_u32* pu32OnOff);

/**
 \brief 设置卖场模式开关
 \attention \n
无

 \param[in] enFlags
 \param[in] u32OnOff

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetDemo( MT_UNF_PQ_DEMO_E enFlags, mt_u32 u32OnOff);

/**
 \brief 设置PQ寄存器
 \attention \n
无

 \param[in] u32RegAddr
 \param[in] u32Data

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_WritePQRegister(mt_u32 u32RegAddr, mt_u32 u32Data);

/**
 \brief 获取PQ寄存器
 \attention \n
无

 \param[in] u32RegAddr
 \param[in] *pu32Value

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_ReadPQRegister(mt_u32 u32RegAddr, mt_u32* pu32Value);

/**
 \brief 获取ACM曲线
 \attention \n
  无

 \param[in] *pstGammaTable

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetAcmTable( MPI_ACM_PARAM_S* pstColorTable);

/**
 \brief 设置ACM曲线
 \attention \n
  无

 \param[in] *pstGammaTable

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetAcmTable(mt_u32 u32RegAddr, MPI_ACM_PARAM_S* pstColorTable);

/**
 \brief 获取DCI曲线
 \attention \n
  无

 \param[in] *pstDCITable

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetDciTable( MT_PQ_DCI_WGT_S* pstDCITable);

/**
 \brief 设置DCI曲线
 \attention \n
  无

 \param[in] *pstDCITable

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetDciTable( MT_PQ_DCI_WGT_S* pstDCITable);

/**
 \brief 获取DCI直方图
 \attention \n
  无

 \param[in] *pstDCIHistgram

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_GetHistgram( MT_PQ_DCI_HISTGRAM_S* pstDCIHistgram);

/**
 \brief 获取TNR的亮度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetTNRLumaPixMean2Ratio( MT_PQ_TNR_S* pstTnrData);

/**
 \brief 设置TNR的亮度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetTNRLumaPixMean2Ratio( MT_PQ_TNR_S* pstTnrData);


/**
 \brief 获取TNR的色度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetTNRChromPixMean2Ratio( MT_PQ_TNR_S* pstTnrData);

/**
 \brief 设置TNR的色度PixMean-to-Ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetTNRChromPixMean2Ratio( MT_PQ_TNR_S* pstTnrData);


/**
 \brief 获取TNR的亮度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetTNRLumaMotionMapping( MT_PQ_TNR_S* pstTnrData);

/**
 \brief 设置TNR的亮度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetTNRLumaMotionMapping( MT_PQ_TNR_S* pstTnrData);


/**
 \brief 获取TNR的色度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetTNRChromMotionMapping( MT_PQ_TNR_S* pstTnrData);

/**
 \brief 设置TNR的色度MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetTNRChromMotionMapping( MT_PQ_TNR_S* pstTnrData);


/**
 \brief 获取TNR的亮度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetTNRLumaFinalMotionMapping( MT_PQ_TNR_S* pstTnrData);

/**
 \brief 设置TNR的亮度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetTNRLumaFinalMotionMapping( MT_PQ_TNR_S* pstTnrData);


/**
 \brief 获取TNR的色度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetTNRChromFinalMotionMapping( MT_PQ_TNR_S* pstTnrData);

/**
 \brief 设置TNR的色度FINAL MotionMapping曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetTNRChromFinalMotionMapping( MT_PQ_TNR_S* pstTnrData);

/**
 \brief 获取SNR的pixmean-ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetSNRPixmean2Ratio( MT_PQ_SNR_PIXMEAN_2_RATIO_S* pstSnrData);

/**
 \brief 设置SNR的pixmean-ratio曲线
 \attention \n
  无

 \param[in] *pstTnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetSNRPixmean2Ratio( MT_PQ_SNR_PIXMEAN_2_RATIO_S* pstSnrData);

/**
 \brief 获取SNR的pixdiff-edgestr曲线
 \attention \n
  无

 \param[in] *pstSnrData

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_MPI_PQ_GetSNRPixdiff2Edgestr( MT_PQ_SNR_PIXDIFF_2_EDGESTR_S* pstSnrData);

/**
 \brief 设置SNR的pixdiff-edgestr曲线
 \attention \n
  无

 \param[in] *pstSnrData

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_MPI_PQ_SetSNRPixdiff2Edgestr( MT_PQ_SNR_PIXDIFF_2_EDGESTR_S* pstSnrData);

extern mt_s32 MT_MPI_PQ_GetDciLevel(mt_u32* pu32DCIlevel);
extern mt_s32 MT_MPI_PQ_SetDciLevel( mt_u32 u32DCIlevel);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __MT_MPI_PQ_H__ */



