/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
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
    MT_FE_CT8K_Device_Handle ct8k_handle;
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

// fix 112220 start
    u8 bSupportDvbTT2Auto : 1;
    u8 bSupportDvbT2      : 1;
    u8 bSupportDvbT       : 1;
    u8 bSupportDvbC       : 1;
    u8 bSupportJ83B       : 1;
    u8 bSupportDvbSS2Auto : 1;
    u8 bSupportDvbS2      : 1;
    u8 bSupportDvbS       : 1;
// fix 112220 end
} mt_fe_ct8k_priv_t, *mt_fe_ct8k_priv_handle;


extern mt_fe_ct8k_priv_handle g_ct8k_priv;

void port_ct8k_notify_to_up_layer(MT_FE_MSG msg, void *p_param);


/*!
  The nim xtal enum
  */
typedef enum _NIM_FE_XTAL_CT8K
{
	NIM_CT8K_XTALMode_24M = 0,
	NIM_CT8K_XTALMode_27M
} NIM_FE_XTAL_CT8K;

/*!
  The nim tc6800 xtal enum
  */
typedef enum _NIM_FE_TC6800_XTAL_CT8K
{
	NIM_CT8K_TC6800_XTALMode_27M = 0,
	NIM_CT8K_TC6800_XTALMode_24M
} NIM_FE_TC6800_XTAL_CT8K;

/*!
  The nim tc3800 xtal enum
  */
typedef enum _NIM_FE_TC3800_XTAL_CT8K
{
	NIM_CT8K_TC3800_XTALMode_27M = 0,
	NIM_CT8K_TC3800_XTALMode_24M
} NIM_FE_TC3800_XTAL_CT8K;


typedef enum _NIM_FE_TUNER_CT8K
{
	NIM_CT8K_TUNER_UNDEF = 0,
	NIM_CT8K_TUNER_MxL603,
	NIM_CT8K_TUNER_TC3800,
	NIM_CT8K_TUNER_TS2022,
	NIM_CT8K_TUNER_RDA5815S,
	NIM_CT8K_TUNER_R836,
	NIM_CT8K_TUNER_R848,
	NIM_CT8K_TUNER_TC6800,
	NIM_CT8K_TUNER_TS6011,
} NIM_FE_TUNER_CT8K;

/*!
  The nim ct8k config
  */
typedef struct nim_ct8k_config
{
  unsigned int x_crystal;          /* NIM_FE_XTAL */
  unsigned int udvbt_tuner;        /* tuner for dvbt */
  unsigned int tun2_crystal;       /* tc3800 crystal */
  unsigned int tun2_loop;          /* tc3800 loop through */
  unsigned int tun2_clk_out;       /* tc3800 clock out on/off */
  unsigned int udvbs_tuner;        /* tuner for dvbs */
  unsigned int udvbt_serialtsno;   /* c/t/t2 ts output position */
  unsigned int udvbs_serialtsno;   /* s/s2 ts output position */
}nim_ct8k_config_t;


#if 0
typedef enum _NIM_DMD_WORK_MODE
{
	NIM_DMD_NORMAL = 0x20,
	NIM_DMD_REPEAT
} NIM_DMD_WORK_MODE;
#endif


typedef struct _nim_signal_info_ct8k
{
	unsigned char  AAGC_LOCK;
	unsigned char  T2FEC_LOCK;
	unsigned long ldpc_frame_cnt;
	unsigned long bch_error_cnt0;
	unsigned char 	snr;
	unsigned char  ssi;
	unsigned char  sqi;
	char 	strength;
	unsigned char  IFAGC_Cur;
	double dbFreq_Loop;
	//MT_FE_T2_TPS_INFO start
	unsigned long			t2_qam;
	unsigned long			t2_fft;
	unsigned long 		t2_guard;
	unsigned long			t2_pp;
	unsigned long			t2_code;
	//MT_FE_T2_TPS_INFO end
	unsigned char plp_num;
	unsigned char plp_id;
	unsigned long frequency;
	unsigned long bandwidth;
}nim_signal_info_ct8k;

typedef struct _nim_ct8k_t2_attr
{
	unsigned long			t2_qam;
	unsigned long			t2_fft;
	unsigned long 			t2_guard;
	unsigned long			t2_pp;
	unsigned long			t2_code;
} nim_ct8k_t2_attr;

typedef struct _nim_ct8k_sat_attr
{
	signed   char snr;
	double   ber;
	unsigned char code_rate;
	unsigned char mod_mode;
} nim_ct8k_sat_attr;

void nim_ct8k_set_config(nim_ct8k_config_t *p_cfg);

/*see NIM_DMD_WORK_MODE
  0 for return success
  other for fail
*/
int nim_ct8k_work_mode(int workmode);

#endif

