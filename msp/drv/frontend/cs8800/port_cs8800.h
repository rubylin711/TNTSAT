/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef __NIM_MN_CS8800_H__
#define __NIM_MN_CS8800_H__

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
  MT_FE_CS8800_Device_Handle cs8800_handle;
  //i2c_bus_t *i2c_master;
  MT_FE_BS_TP_INFO bs_info;
  mt_unf_fe_channel_info_t cur_channel;
  u8 diseqc_tx_buf[8];
  u8 diseqc_rx_buf[8];
  u8 diseqc_rx_cnt;
  mt_unf_fe_diseqc_cmd_t cur_diseqc;
  //mt_unf_fe_scan_info_t scan_info;
  //mt_unf_fe_blindscan_para_t scan_info;
  fe_blindscan_param_t scan_info;
  //MT_FE_LOCK_STATE ss2_status;
  //void *lnb_agent_handle;
  u32 print_level;
  u32 sig_type;
  struct task_struct *bl_thread;
} mt_fe_cs8800_priv_t, *mt_fe_cs8800_priv_handle;

extern mt_fe_cs8800_priv_handle g_cs8800_priv;

void port_cs8800_notify_to_up_layer(MT_FE_MSG msg, void *p_param);

/*!
  The nim xtal enum
  */
typedef enum _NIM_FE_XTAL_CS8800
{
  NIM_CS8800_XTALMode_24M = 0,
  NIM_CS8800_XTALMode_27M
} NIM_FE_XTAL_CS8800;

/*!
  The nim tc6800 xtal enum
  */
typedef enum _NIM_FE_TC6800_XTAL_CS8800
{
  NIM_CS8800_TC6800_XTALMode_27M = 0,
  NIM_CS8800_TC6800_XTALMode_24M
} NIM_FE_TC6800_XTAL_CS8800;

/*!
  The nim tc3800 xtal enum
  */
typedef enum _NIM_FE_TC3800_XTAL_CS8800
{
  NIM_CS8800_TC3800_XTALMode_27M = 0,
  NIM_CS8800_TC3800_XTALMode_24M
} NIM_FE_TC3800_XTAL_CS8800;

typedef enum _NIM_FE_TUNER_CS8800
{
  NIM_CS8800_TUNER_UNDEF = 0,
  NIM_CS8800_TUNER_MxL603,
  NIM_CS8800_TUNER_TC3800,
  NIM_CS8800_TUNER_TS2022,
  NIM_CS8800_TUNER_RDA5815S,
  NIM_CS8800_TUNER_R836,
  NIM_CS8800_TUNER_R848,
  NIM_CS8800_TUNER_TC6800,
  NIM_CS8800_TUNER_TS6011,
} NIM_FE_TUNER_CS8800;

/*!
  The nim cs8800 config
  */
typedef struct nim_cs8800_config
{
  unsigned int x_crystal;        /* NIM_FE_XTAL */
  unsigned int udvbc_tuner;      /* tuner for dvbc */
  unsigned int tun2_crystal;     /* c/b tuner crystal */
  unsigned int tun2_loop;        /* c/b tuner loop through */
  unsigned int tun2_clk_out;     /* c/b tuner clock out on/off */
  unsigned int udvbs_tuner;      /* tuner for dvbs */
  unsigned int udvbc_serialtsno; /* c/b ts output position */
  unsigned int udvbs_serialtsno; /* s/s2 ts output position */
} nim_cs8800_config_t;

typedef struct _nim_cs8800_sat_attr
{
  signed char snr;
  double ber;
  unsigned char code_rate;
  unsigned char mod_mode;
} nim_cs8800_sat_attr;

void nim_cs8800_set_config(nim_cs8800_config_t *p_cfg);

/*see NIM_DMD_WORK_MODE
  0 for return success
  other for fail
*/
int nim_cs8800_work_mode(int workmode);
int port_m88cs8800_register_netlink_family(void);
#endif

