/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include"MT_DF_common.h"

MT_RET MT_DF_DCE(MT_U32 *hist_in, MT_U8 *enh_func, MT_U32 hist_pix_num, MT_S32* enh_deg_para,
    MT_U32* max_enh_deg, MT_U32 max_adj_dif, MT_U16 clip_limit);
void MT_DF_DCE_8To10(MT_U8 *enh_func, MT_U16 *enh_func_10);


