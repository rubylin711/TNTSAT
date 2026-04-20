/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __NIM_MN_CT8K_H__
#define __NIM_MN_CT8K_H__

#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_def_tc6930.h"

typedef struct
{
    //MT_UNF_PIN_CONFIG_PARA_S cfg;
    mt_unf_fe_config_para_t cfg;
    MT_FE_TC6930_Device_Handle tc6930_handle;
    //i2c_bus_t *i2c_master;
    mt_unf_fe_channel_info_t cur_channel;
    u32 status;
    u32 print_level;
    u32 sig_type;
} mt_fe_tc6930_priv_t, *mt_fe_tc6930_priv_handle;


extern mt_fe_tc6930_priv_handle g_tc6930_priv;



#endif

