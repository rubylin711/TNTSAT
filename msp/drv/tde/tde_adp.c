/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "tde_hal.h"
#include "tde_define.h"
#include "tde_adp.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

static mt_u32 gs_u32Capability = ROP|ALPHABLEND|COLORIZE|CLUT|COLORKEY|CLIP|DEFLICKER|RESIZE|MIRROR|CSCCOVERT|QUICKFILL|QUICKCOPY|\
PATTERFILL;
mt_s32 TdeHalGetCapability(mt_u32 *pstCapability)
{
    *pstCapability = gs_u32Capability;
    return MT_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif  /*__cplusplus*/
#endif  /*__cplusplus*/

