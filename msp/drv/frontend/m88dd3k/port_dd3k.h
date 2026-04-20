/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
//#include "mt_type.h"
#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_def.h"

typedef struct
{
    s16 cur_tun_offset_khz;

    //MT_UNF_PIN_CONFIG_PARA_S cfg;
    mt_unf_fe_config_para_t cfg;
    MT_FE_DD_Device_Handle dd3k_handle;
    //i2c_bus_t *i2c_master;
    mt_unf_fe_channel_info_t cur_channel;
    u32 status;
    u32 print_level;
    u8 sig_type;
} mt_fe_dd3k_priv_t, *mt_fe_dd3k_priv_handle;


extern mt_fe_dd3k_priv_handle g_dd3k_priv;


