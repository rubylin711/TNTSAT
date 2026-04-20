/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
//#include "mt_type.h"
#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_def.h"

typedef struct
{
    u8 bs_stop : 1;
    u8 for_scan : 1;
    u8 diseqc_2x : 1;
    u8 onoff_22k : 1;
    u8 lnb_onoff : 1;
    u8 lnb_voltage : 2;
    u8 reserved : 1;
    u8 lnb_polar;
    s16 cur_tun_offset_khz;

    //MT_UNF_PIN_CONFIG_PARA_S cfg;
    mt_unf_fe_config_para_t cfg;
    MT_FE_DM6K_Device_Handle dm6k_handle;
    //i2c_bus_t *i2c_master;
    MT_FE_BS_TP_INFO bs_info;
    mt_unf_fe_channel_info_t cur_channel;
    u8 diseqc_tx_buf[8];
    mt_unf_fe_diseqc_cmd_t cur_diseqc;
    //mt_unf_fe_scan_info_t scan_info;
    //mt_unf_fe_blindscan_para_t scan_info;
    fe_blindscan_param_t scan_info;
    u32 status;
    //void *lnb_agent_handle;
    u32 print_level;
    u8 sig_type;
} mt_fe_dm6k_priv_t, *mt_fe_dm6k_priv_handle;


extern mt_fe_dm6k_priv_handle g_dm6k_priv;

void port_dm6k_notify_to_up_layer(MT_FE_MSG msg, void *p_param);

