/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef __NIM_MN_RS6060_H__
#define __NIM_MN_RS6060_H__

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
    MT_FE_RS6060_Device_Handle rs6060_handle;
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
    u32 sig_type;
    struct task_struct *bl_thread;
} mt_fe_rs6060_priv_t, *mt_fe_rs6060_priv_handle;


extern mt_fe_rs6060_priv_handle g_rs6060_priv;

void port_rs6060_notify_to_up_layer(MT_FE_MSG msg, void *p_param);


/*!
  The nim rs6060 config
  */
typedef struct nim_rs6060_config
{
  unsigned int x_crystal;          /* NIM_FE_XTAL */
  unsigned int udvbc_tuner;        /* tuner for dvbc */
  unsigned int tun2_crystal;       /* c/b tuner crystal */
  unsigned int tun2_loop;          /* c/b tuner loop through */
  unsigned int tun2_clk_out;       /* c/b tuner clock out on/off */
  unsigned int udvbs_tuner;        /* tuner for dvbs */
  unsigned int udvbc_serialtsno;   /* c/b ts output position */
  unsigned int udvbs_serialtsno;   /* s/s2 ts output position */
}nim_rs6060_config_t;


typedef struct _nim_rs6060_sat_attr
{
	signed   char snr;
	double   ber;
	unsigned char code_rate;
	unsigned char mod_mode;
} nim_rs6060_sat_attr;

void nim_rs6060_set_config(nim_rs6060_config_t *p_cfg);

/*see NIM_DMD_WORK_MODE
  0 for return success
  other for fail
*/
int nim_rs6060_work_mode(int workmode);
int port_m88rs6060_register_netlink_family(void);
#endif

