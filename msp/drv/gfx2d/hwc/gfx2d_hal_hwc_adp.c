/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "gfx2d_hal_hwc_adp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

static HWC_CAPABILITY_S gs_stCapability = 
{
    7,
    6,
    1,
    {0x00c043d0,0xfe7efc},
    {0x0c040c0,0x200},
    1,
    0xffff,
    16,
    1,
#if defined(CHIP_TYPE_hi3798cv100) || defined(CHIP_TYPE_hi3798cv100_a) || defined(CHIP_TYPE_hi3796cv100) || defined(CHIP_TYPE_hi3796cv100_a) || defined(CHIP_TYPE_hi3798mv100_a)
    3840,
#else
    2560,
#endif
    1,
#if defined(CHIP_TYPE_hi3798cv100) || defined(CHIP_TYPE_hi3798cv100_a) || defined(CHIP_TYPE_hi3796cv100) || defined(CHIP_TYPE_hi3796cv100_a) || defined(CHIP_TYPE_hi3798mv100_a)
    2160,
#else
    1600,
#endif
    32,
    32,
    256,
    16,
    2,
    {0x400000,0x0}
};

HI_S32 HWC_ADP_GetCapability(HWC_CAPABILITY_S *pstCapability)
{
    memcpy(pstCapability, &gs_stCapability, sizeof(HWC_CAPABILITY_S));
    
    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/
