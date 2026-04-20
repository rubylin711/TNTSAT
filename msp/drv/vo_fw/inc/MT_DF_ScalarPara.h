/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef MT_DF_SCALAR_H
#define MT_DF_SCALAR_H

#include"MT_DF_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagCalcScalarPara
{
    MT_DF_BOOL bFrameIn;
    MT_DF_BOOL bFrameOut;
    MT_DF_BOOL bUseHalfScaleV;
    MT_DF_BOOL bUseHalfScaleH;
    MT_U32 u32WidthIn;
    MT_U32 u32WidthOut;
    MT_U32 u32HeightIn;
    MT_U32 u32HeightOut;
    MT_U32 VerticalInt;
    MT_U32 VerticalFra;
    MT_U32 HoriInt;
    MT_U32 HoriFra;
    MT_U32 TopInt;
    MT_U32 TopFra;
    MT_U32 BotInt;
    MT_U32 BotFra;

}DF_CALC_SCALAR_PARA;

MT_S32 DF_Calc_Scalar_Para(DF_CALC_SCALAR_PARA * );

#ifdef __cplusplus
}
#endif
#endif

