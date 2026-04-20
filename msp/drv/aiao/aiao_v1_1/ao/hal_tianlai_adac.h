/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __AUD_TIANLAI_ADAC_HAL_H__
#define __AUD_TIANLAI_ADAC_HAL_H__


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#include "hi_type.h"
#include "hi_unf_sound.h"

/***************************************************************************
Description:
    define const varible
***************************************************************************/
typedef union
{
    // Define the struct bits
    struct
    {
        mt_u32 dacr_vol             : 7; // [6..0]
        mt_u32 reserved1     : 1; // [7]
        mt_u32 dacl_vol               : 7; // [14..8]
        mt_u32 reserved2           : 3; // [17..15]
        mt_u32 deemphasis_fs       : 2; // [19..18]
        mt_u32 dacr_deemph       : 1; // [20]
        mt_u32 dacl_deemph       : 1; // [21]
        mt_u32 dacr_path       : 1; // [22]
        mt_u32 dacl_path       : 1; // [23]
        mt_u32 popfreer       : 1; // [24]
        mt_u32 popfreel       : 1; // [25]
        mt_u32 fs       : 1; // [26]
        mt_u32 pd_vref       : 1; // [27]
        mt_u32 mute_dacr       : 1; // [28]
        mt_u32 mute_dacl       : 1; // [29]
        mt_u32 pd_dacr       : 1; // [30]
        mt_u32 pd_dacl       : 1; // [31]
        
    } bits;

    // Define an unsigned member
    mt_u32 u32;
} SC_PERI_TIANLAI_ADAC0;

typedef union
{
    // Define the struct bits
    struct
    {
        mt_u32 reserved1             : 8; // [7..0]
        mt_u32 clkdgesel     : 1; // [8]
        mt_u32 clksel2               : 1; // [9]
        mt_u32 adj_refbf           : 2; // [11..10]    
        mt_u32 rst       : 1; // [12]
        mt_u32 adj_ctcm       : 1; // [13]      
        mt_u32 adj_dac       : 2; // [15..14]
        mt_u32 reserved2       : 3; // [18:16]
        mt_u32 sample_sel       : 3; // [21..19]
        mt_u32 data_bits       : 2; // [23..22]
        mt_u32 reserved3       : 1; // [24]
        mt_u32 mute_rate       : 2; // [26..25]
        mt_u32 dacvu       : 1; // [27]
        mt_u32 sunmuter       : 1; // [28]
        mt_u32 sunmutel       : 1; // [29]
        mt_u32 smuter       : 1; // [30]
        mt_u32 smutel       : 1; // [31]
    } bits;

    // Define an unsigned member
    mt_u32 u32;
} SC_PERI_TIANLAI_ADAC1;

// Define the union U_S40_TIANLAI_ADAC_CRG
typedef union
{
    // Define the struct bits
    struct
    {
        unsigned int adac_cken             : 1; // [0]
        unsigned int Reserved_0            : 3; // [3..1]
        unsigned int adac_srst_req         : 1; // [4]
        unsigned int Reserved_1            : 27; // [31..5]
    } bits;

    // Define an unsigned member
    unsigned int u32;
} U_S40_TIANLAI_ADAC_CRG;

/***************************************************************************
Description:
    define emum varible
***************************************************************************/

/*****************************************************************************
 Description  : ADAC API
*****************************************************************************/
mt_void ADAC_TIANLAI_Init(mt_u32 uSampelRate, MT_BOOL bResume);
mt_void ADAC_TIANLAI_DeInit(MT_BOOL bSuspend);




#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __AUD_TIANLAI_ADAC_HAL_H__ */
