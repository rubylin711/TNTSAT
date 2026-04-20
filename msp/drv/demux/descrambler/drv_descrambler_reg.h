/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 File Name     : drv_descrambler_reg.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       : 2013/04/16
 Description   :
******************************************************************************/

#ifndef __DRV_DESCRAMBLER_REG_H__
#define __DRV_DESCRAMBLER_REG_H__

#include "mt_type.h"

#define DMX_CW_CTRL0                    (0x3B70)    /* cw order change reg0 */
#define DMX_CW_CTRL2                    (0x3B78)    /* cw order change reg2 */

#define CW_SET                          (0x3B80)    /* CW configuration control register */
#define DMX_CW_DATA                     (0x3B84)    /* CW configuration register */

#ifdef DMX_DESCRAMBLER_VERSION_0
#define CW_SEL                          (0x3B88)    /* CA selection register */
#define CW_CSA3                         (0x3B8C)    /* CSA3 selection register */
#endif

#define CA_INFO0                        (0x3B90)    /* hardware OTP CA set [31:0] */

#ifdef DMX_DESCRAMBLER_VERSION_1
typedef union
{
    struct
    {
        mt_u32  hardonly_csa2       : 1;    // [0]
        mt_u32  hardonly_spe        : 1;    // [1]
        mt_u32  hardonly_des        : 1;    // [2]
        mt_u32  hardonly_novel      : 1;    // [3]
        mt_u32  hardonly_csa3       : 1;    // [4]
        mt_u32  hardonly_others     : 1;    // [5]
        mt_u32  reserved0           : 2;    // [7:6]
        mt_u32  dis_csa2            : 1;    // [8]
        mt_u32  dis_spe             : 1;    // [9]
        mt_u32  dis_des             : 1;    // [10]
        mt_u32  dis_novel           : 1;    // [11]
        mt_u32  dis_csa3            : 1;    // [12]
        mt_u32  dis_others          : 1;    // [13]
        mt_u32  dis_tdes            : 1;    // [14]
        mt_u32  reserved2           : 15;   // [29:15]
        mt_u32  ca_tsout_dis        : 1;    // [30]
        mt_u32  cainfo_dbg_dis      : 1;    // [31]
    } bits;

    mt_u32 value;
} U_CA_INFO0;
#endif

#define DMX_CA_DBG_0                    (0x3B94)    /* hardware OTP CA set [63:32] */
#define CA_ENTROPY                      (0x3BA8)    /* reserved register. Entropy decrease register */
#define DMX_CW_SET(DmxId)               (0x3BC0 + ((DmxId) << 2))       /* Set demux(x)'s CW table id */
#define CHAN_CW_TAB_ID(ChanId)          (0x3BE0 + (ChanId) / 16 * 4)    /* Set channel's CW table id, 2bit for each channel */

#define DMX_CW_VALUE(Idx, flag, half)   (0x3E00+16*(Idx) + 8 *(flag) +4*(half))  /* soft-way CW word set register */

#endif  // __DRV_DESCRAMBLER_REG_H__


