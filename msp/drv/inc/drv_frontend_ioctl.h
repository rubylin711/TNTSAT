/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_FRONTEND_IOCTL_H__
#define __DRV_FRONTEND_IOCTL_H__

/*#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/atomic.h>
#include <linux/miscdevice.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/pm.h>*/

#include "mt_type.h"
//#include "mt_drv_i2c.h"
#include "mt_debug.h"
#include "mt_drv_struct.h"
//#include "drv_i2c_ext.h"
//#include "drv_gpioi2c_ext.h"
//#include "drv_gpio_ext.h"
#include "mt_drv_frontend.h"
#include "mt_unf_frontend.h"

//#include <linux/poll.h>

//#include <linux/mutex.h>
//#include <linux/dvb/frontend.h>
//#include "dvb_frontend.h"

//#include "common_dev.h"
//#include "common_proc.h"
//#include "common_mem.h"
//#include "common_stat.h"

#if 1
#define DEMOD_PORT0_ADDR 0xA0
#define DEMOD_PORT1_ADDR 0xA2
#define DEMOD_PORT2_ADDR 0xA0
#define DEMOD_PORT3_ADDR 0xA2
#define DEMOD_PORT4_ADDR 0xA0
#define TUNER_PORT0_ADDR 0xC0
#define TUNER_PORT1_ADDR 0xC0
#define TUNER_PORT2_ADDR 0xC0
#define TUNER_PORT3_ADDR 0xC0
#define TUNER_PORT4_ADDR 0xC0
//#define XTAL_CLK 28800

#define FE_NUM 5

typedef enum mt_fe_data_src_t {
    fe_data_src_adc = 0,
    fe_data_src_equ,
    fe_data_src_buit
} fe_data_src_t;

typedef enum mt_fe_qam_type_t {
    QAM_TYPE_16 = 0,
    QAM_TYPE_32,
    QAM_TYPE_64,
    QAM_TYPE_128,
    QAM_TYPE_256
} fe_qam_type_t;

typedef struct mt_fe_agc_qam_param_t
{
    mt_u32 freq; /* frequency kHz*/
    union
    {
	mt_u32 sym_rate;   /* symbolrate Hz*/
	mt_u32 band_width; /* BandWidth KHz*/
    } srbw;
    fe_qam_type_t qam_type;
    MT_BOOL b_si; /*reverse*/
    mt_unf_fe_polar_t polar;
    union
    {
	mt_unf_fe_ter_acc_t dvbt2;
	mt_unf_fe_ts_priority_t dvbt;
    } ter;
    mt_u8 dvbt_mode; /* 0 dvb-t2; 1 dvb-t */
} fe_agc_qam_param_t;

typedef struct mt_fe_data_t
{
    mt_u32 port;
    mt_u32 data;
} fe_data_t, *fe_data_handle;

typedef struct mt_fe_databuf_t
{
    mt_u32 port;
    mt_u32 databuf[3];
} fe_databuf_t, *fe_databuf_handle;

typedef struct mt_fe_datalist_t
{
    mt_u32 port;
    mt_u8 data_list[32];
} fe_datalist_t, *fe_datalist_handle;

typedef struct mt_fe_signal_strength_t
{
    mt_u32 tuner_id;
    mt_u32 strength[3];
} fe_signal_strength_t, *fe_signal_strength_handle;

typedef struct mt_fe_config
{
    mt_u32 port;
    /*!
      NIM lock indication pin's active level, 0:low active, 1:high active
      */
    mt_u8 lock_indicate : 1;
    /*!
      NIM voltage selection pin's active level, 0:select 13v when the pin is low, 1:select
      13v when the pin is high
      */
    mt_u8 vsel_when_13v : 1;
    /*!
      VSEL pin's active level when LNB is in standby state, 0: low active, 1:high active
      */
    mt_u8 vsel_when_lnb_off : 1;
    /*!
      DiSEqC out pin's active level when LNB is in standby state, 0: low active, 1:high active
      */
    mt_u8 diseqc_out_when_lnb_off : 1;
    /*!
      LNB enable pin's active level, 0: low active, 1:high active
      */
    mt_u8 lnb_enable : 1;
    /*!
      LNB short circuit protection pin's level, 0: protect enable(short circuit happened) on low level,
      1:protect enable on high level
      */
    mt_u8 lnb_prot_level : 1;
    /*!
      LNB enable(power enable or short circuit protection) pin's control by mcu or nim, 1: by mcu, 0:by nim
      */
    mt_u8 lnb_enable_by_mcu : 1;
    /*!
      LNB short circuit protection by mcu or nim, 1: by mcu, 0:by nim
      */
    mt_u8 lnb_prot_by_mcu : 1;
    /*!
      LNB enable pin's number
      */
    mt_u8 lnb_enable_pin;
    /*!
      LNB short circuit protection pin's number
      */
    mt_u8 lnb_prot_pin;
    /*!
      flag to indicate if lock pin is by mcu's gpio
      */
    mt_u8 lock_pin_by_mcu : 1;
    /*!
      lock gpio pin number
      */
    mt_u8 lock_pin : 7;
    /*!
      flag to indicate if voltage select pin is by mcu's gpio
      */
    mt_u8 lnb_vol_pin_by_mcu : 1;
    /*!
      voltage select gpio pin number
      */
    mt_u8 lnb_vol_pin : 7;
    /*!
      demod reset gpio pin number
    */
    mt_u8 demod_reset_pin;
    /*!
      lnb voltage selection pin mode, is only valid when lnb_vol_pin_by_mcu is 1
      0:default(demo board); 1:custom mode1; 2:custom mode2; ...
    */
    mt_u8 lnb_vol_pin_mode;

    mt_u8 reset_gpio_no;
    mt_u8 use_unicable;
} fe_config_t, *fe_config_handle;

/*!
  The NIM device configuration structure, used by nim_open()
  */
typedef struct nim_config
{
    /*!
    demod version, see nim_dem_ver_t
    */
    mt_u8 dem_ver;
    /*!
    demod version, see nim_dem_ver_t
    */
    mt_u8 tn_version;
    /*!
      2-wire bus device handle
      */
    mt_void *p_dem_bus;
    /*!
      NIM type, see enum nim_type_t.
      */
    mt_u8 nim_type;
    /*!
      Demodulator's 2-wire bus address
      */
    mt_u8 dem_addr;
    /*!
      Demodulator's crystal frequency in KHz
      */
    mt_u16 dem_crystal_khz;
    /*!
      Demodulator's working clock frequency in KHz
      */
    mt_u32 dem_clock_khz;
    /*!
      TS output mode, serial or parallel, see enum nim_ts_mode.
      */
    mt_u32 ts_mode;
    /*!
      2-wire bus device, if tuner i2c is repeated by demod, set NULL here
      */
    mt_void *p_tun_bus;
    /*!
      Tuner's 2-wire bus address(for s/s2 use when board has two tuners)
      */
    mt_u8 tun_addr;
    /*!
      Tuner's 2-wire bus address(for t/t2 use when board has two tuners)
      */
    mt_u8 tun2_addr;
    /*!
      For t/t2 use(0:none, 1:using, other:for future use)
      */
    mt_u8 lna_type;
    /*!
      Tuner's crystal frequency in KHz
      */
    mt_u16 tun_crystal_khz;
    /*!
      Check if the tuner's spectral I/Q pin is connected invertedly.
      */
    mt_u8 tun_iq_invertion;
    /*!
      The tuner supported by this NIM driver, see enum nim_tuner_ver_t
      */
    mt_u8 tun_support;
    /*!
      If you need loop RF through function, please set it to 1 , else set it to 0
      */
    mt_u8 tun_rf_bypass_en : 1;
    /*!
      If you need clock out from tuner clkout pin to other chip use, set it to 1
      */
    mt_u8 tun_clk_out_by_tn : 1;
    /*!
      If you need clock out from tuner xtalout pin to demodulator use, please set it to 1 , else set it 0
      */
    mt_u8 tun_clk_out_by_xtal : 1;
    /*!
      The tuner clock out division, it is valid if tun_clk_out_by_tn is 1
      */
    mt_u8 tun_clk_out_div : 5;
    /*!
      The times of blind scan
      */
    mt_u8 bs_times;
    /*!
      NIM's working mode, lab testing or normal
      */
    mt_u8 work_mode;

    /*!
     NIM's demode system frequency
     */
    mt_u32 x_crystal;
    /*!
      The on-board pin's level configuration
      */
    fe_config_t pin_config;
    /*!
      The bandwidth of demod
      */
    mt_u8 demod_band_width;
    /*!
      if use the public sharing driver service, set the handle here
      */
    mt_void *p_drvsvc;
    /*!
      if not use the public sharing driver service, set this, the notify task priority
      */
    mt_u32 task_prio;
    /*!
      if not use the public sharing driver service, set this, the notify task size
      */
    mt_u32 stack_size;
    /*!
      Tuner loopthrough
      */
    mt_u8 tuner_loopthrough;
    /*!
      tuner_bandwidth
      */
    mt_u8 tuner_bandwidth;
    /*!
      tuner mode if needed, 0: DVB-C, 2: MMDS
      */
    mt_u8 tuner_mode;
    /*!
      tuner driver version
      */
    mt_u8 tuner_ver;
    /*!
        The locking mode of function call, see dev_lock_mode
      */
    mt_u8 lock_mode;
    /*!
        number of set limit  lock ferquency
      */
    mt_u32 freq_offset_limit;
} nim_config_t;

typedef struct mt_fe_signal_info_t
{
    mt_u32 tuner_id;
    mt_unf_fe_signal_info_t info;
} fe_signal_info_t, *fe_signal_info_handle;

//#if 0
#define MAX_TP_NUM (128)

typedef struct mt_fe_blindscan_init_param_t
{
    mt_u32 data; /*For extend*/
} fe_blindscan_init_param_t, *fe_blindscan_init_param_handle;

typedef struct mt_fe_blindscan_t
{
    mt_u32 tuner_id;
    fe_blindscan_init_param_t param;
} fe_blindscan_t, *fe_blindscan_handle;

typedef struct mt_fe_blindscan_param_t
{
#if 0
    mt_u32 start_freq; /*Blind scan centre frequency, kHz*/
    mt_u32 end_freq;   /*Blind scan centre frequency, kHz*/
    mt_u16 count;      /*Channel find in this scanning */
    mt_u16 tp_num;
    mt_u16 tp_max_num;
    mt_u8 bs_mode;
    mt_u8 bs_action;
    //MT_FE_TP_INFO *p_tp_info;
    //mt_unf_fe_sat_tpinfo_t *p_tp_info;
#if 1
    union
    {
        mt_unf_fe_sat_tpinfo_t sat[MAX_TP_NUM];
    }unResult;
#endif

#endif

    /**<LNB Polarization type, only take effect in manual blind scan mode*/
    /**<CNcomment:LNB极化方式，自动扫描模式设置无效*/
    mt_unf_fe_polar_t polar;

    /**<LNB 22K signal status, for Ku band LNB which has dual LO, 22K ON will select high LO and 22K off select low LO,
        only take effect in manual blind scan mode*/
    /**<CNcomment:LNB 22K状态，对于Ku波段双本振LNB，ON选择高本振，OFF选择低本振，自动扫描模式设置无效*/
    mt_unf_fe_lnb_22k_t lnb_22k;

    /**<Blind scan start IF, in kHz, only take effect in manual blind scan mode */
    /**<CNcomment:盲扫起始频率(中频)，单位：kHz，自动扫描模式设置无效*/
    mt_u32 start_freq;

    /**<Blind scan stop IF, in kHz, only take effect in manual blind scan mode */
    /**<CNcomment:盲扫结束频率(中频)，单位：kHz，自动扫描模式设置无效*/
    mt_u32 stop_freq;

    /*!
    the start frequency(in KHz) in blind scan, will return the next start
    frequency auto-detected by NIM after every scan window.
    */
    //MT_U32 start_freq;
    /*!
    the end frequency(in KHz) in blind scan
    */
    //MT_U32 end_freq;
    /*!
    the channel information in blind scan, this array is malloced by app and used by driver
    */
    mt_unf_fe_channel_info_t *p_channel_info;
    /*!
    the max channel count can be stored in p_channel_info, it is set by app
    */
    mt_u32 max_count;
    /*!
    this time channel number scaned stored in the channel_info, it will be set by driver
    */
    mt_u32 channel_num_cur;
    /*!
    all channel number scaned stored in the channel_info, it will be set by driver
    */
    mt_u32 channel_num_total;
    /*!
    the uni-cable paramter, see nim_unicable_param_t
    */
    mt_unf_fe_unicable_param_t uc_param;

    mt_u8 invert_spectrum;

#if 1

    /**<Callback when scan status change, scan progress change or find new channel.*/
    /**<CNcomment:扫描状态或进度百分比发生变化时、发现新的频点时回调*/
    //mt_void (*scan_notify)(mt_u32 port, mt_unf_fe_blindscan_evt_t evt, mt_unf_fe_blindscan_notify_t * p_notify);
    //mt_void (*scan_notify)(void *handle, mt_unf_fe_blindscan_evt_t msg, void *p_param);
    mt_void (*scan_notify)(mt_u32 tuner_id, mt_unf_fe_blindscan_evt_t msg, void *p_param);
#endif
} fe_blindscan_param_t, *fe_blindscan_param_handle;

typedef struct mt_fe_terscan_param_t
{
    mt_unf_fe_ter_scan_attr_t ter;
    mt_unf_fe_ter_tpinfo_t tpinfo; /**<result of tp array*/ /**<CNcomment:扫描频点用数组形式存储*/
} fe_terscan_param_t;

#if 0
/*!
    Uni-cable paramter
  */
typedef struct
{
    /*!
    the flag to indicate if use uni-cable
    */
    mt_u8 use_uc : 1;
    /*!
    the bank number, from 0 to 7
    */
    mt_u8 bank : 7;
    /*!
    the user band number, from 0 to 11
    */
    mt_u8 user_band;
    /*!
    the user band frequency in MHz.
    */
    mt_u16 ub_freq_mhz;
} fe_unicable_param_t;

/*!
    The channel performance structure
  */
typedef struct nim_channel_perf
{
    /*!
      Channel is locked or not
      */
    mt_u8 lock;
    /*!
      Signal strength from NIM AGC gain
      */
    mt_u32 agc;
    /*!
      Signal noise rate in percentage
      */
    mt_u32 snr;
    /*!
      Bit error rate, normally it is a very little number.
      */
    mt_double ber;
} fe_channel_perf_t;

/*!
    dvbs channel parameter used for "struct nim_channel_info" "dvbs"
  */
typedef struct nim_dvbs_param
{
    /*!
      symbol rate in Symbols per second, in KSs
      */
    mt_u32 symbol_rate;
    /*!
      forward error correction, see enum nim_code_rate_t
      */
    mt_u8 fec_inner;
    /*!
      NIM type, see enum nim_type_t
      */
    mt_u8 nim_type;
    /*!
      uni-cable parameter
      */
    fe_unicable_param_t uc_param;
    /*!
      performance
      */
    fe_channel_perf_t perf;
} fe_dvbs_param_t;

#if 1
/*!
    dvbc channel parameter used for "struct nim_channel_info" "dvbc"
  */
typedef struct fe_dvbc_param
{
    /*!
      symbol rate in Symbols per second, in KSs
      */
    mt_u32 symbol_rate;
    /*!
      modulation type, see enum nim_modulation
      */
    mt_u16 modulation;
} fe_dvbc_param_t;
#endif

/*!
    dvbt channel parameter used for "struct nim_channel_info" "dvbt"
  */
typedef struct fe_dvbt_param
{
    /*!
      nim_type
      */
    mt_u32 nim_type;
    /*!
      band_width
      */
    mt_u16 band_width;
    /*!
      current set plp_id
      */
    mt_u8 plp_id;
    /*!
      PLP-number
     */
    mt_u8 DataPlpNumber;
    /*!
      plp_id index of PLP array
     */
    mt_u8 PLP_index;
    /*!
      PLP-ID
     */
    mt_u8 DataPlpIdArray[255];
} fe_dvbt_param_t;

/*!
    NIM tuning parameter
  */
typedef union
{
    /*!
      DVB-S QPSK parameter, see nim_dvbs_param_t
      */
    fe_dvbs_param_t dvbs;
    /*!
      DVB-C QAM parameter, see nim_dvbc_param_t
      */
    fe_dvbc_param_t dvbc;
    /*!
      DVB-T/T2 QAM parameter, see nim_dvbt_param_t
      */
    fe_dvbt_param_t dvbt;
} fe_param_t;
#endif

#if 0
/*!
    The channel blind scan information
  */
typedef struct fe_scan_info
{
    /*!
      the start frequency(in KHz) in blind scan, will return the next start
      frequency auto-detected by NIM after every scan window.
      */
    mt_u32 start_freq;
    /*!
      the end frequency(in KHz) in blind scan
      */
    mt_u32 end_freq;
    /*!
      the channel information in blind scan, this array is malloced by app and used by driver
      */
    struct fe_channel_info *p_channel_info;
    /*!
      the max channel count can be stored in p_channel_info, it is set by app
      */
    mt_u32 max_count;
    /*!
      the channel number stored in the channel_info, it will be set by driver
      */
    mt_u32 channel_num;
    /*!
      the uni-cable paramter, see nim_unicable_param_t
      */
    fe_unicable_param_t uc_param;
} fe_scan_info_t;
#endif

typedef struct mt_fe_blindscan_info_t
{
    mt_u32 tuner_id;
    fe_blindscan_param_t *scan_info;
} fe_blindscan_info_t, *fe_blindscan_info_handle;

typedef struct mt_fe_terscan_info_t
{
    mt_u32 port;
    fe_terscan_param_t param;
} fe_terscan_info_t, *fe_terscan_info_handle;

typedef struct _mt_fe_tn_param_t
{
    mt_u32 tuner_id;
    mt_unf_fe_sig_type_t sig_type;
    mt_s32 freq_KHz;
    mt_s32 sym_rate_KSs;
} fe_tn_param_t;

typedef enum
{
    FunctMode_Demod = 0,
    FunctMode_BlindScan = 1
} fe_func_mode_t;

/* LNB out voltage enum */
typedef enum {
    TUNER_LNB_OUT_0V = 0,
    TUNER_LNB_OUT_13V = 13,
    TUNER_LNB_OUT_14V = 14, /* 14 or 15V */
    TUNER_LNB_OUT_18V = 18,
    TUNER_LNB_OUT_19V = 19, /* 19 or 20V */
    TUNER_LNB_OUT_BUTT
} fe_lnb_output_voltage;

/* LNB out voltage struct */
typedef struct
{
    mt_u32 port;
    //mt_u8 lnb_enable;
    fe_lnb_output_voltage voltage;
    //mt_u8 vsel_when_13v;
    //mt_u8 vsel_when_lnb_off;
    //mt_u8 diseqc_out_when_lnb_off;
    mt_u8 on_off;//open or close the lnb power
} fe_lnb_out_t, *fe_lnb_out_handle;

#if 0
/*!
  Polarization, used for IO control "NIM_IOCTRL_SET_PORLAR" command
  */
typedef enum nim_lnb_porlar {
    /*!
        LNB porlarity horizontal
      */
    NIM_PORLAR_HORIZONTAL = 0x00,
    /*!
        LNB porlarity vertical
      */
    NIM_PORLAR_VERTICAL = 0x01,
    /*!
        LNB porlarity left
      */
    NIM_PORLAR_LEFT = 0x02,
    /*!
        LNB porlarity right
      */
    NIM_PORLAR_RIGHT = 0x03,
    /*!
        All LNB porlarity
      */
    NIM_PORLAR_ALL = 0x04,
} nim_lnb_porlar_t;
#endif

/*!
  NIM forward error correction code rate
  */
typedef enum fe_code_rate {
    /*!
      NIM none code rate
      */
    NIM_CR_NONE = 0,
    /*!
      NIM code rate, 1/4
      */
    NIM_CR_1_4,
    /*!
      NIM code rate, 1/3
      */
    NIM_CR_1_3,
    /*!
      NIM code rate, 2/5
      */
    NIM_CR_2_5,
    /*!
      NIM code rate 1/2
      */
    NIM_CR_1_2,
    /*!
      NIM code rate, 3/5
      */
    NIM_CR_3_5,
    /*!
      NIM code rate 2/3
      */
    NIM_CR_2_3,
    /*!
      NIM code rate 3/4
      */
    NIM_CR_3_4,
    /*!
      NIM code rate 4/5
      */
    NIM_CR_4_5,
    /*!
      NIM code rate 5/6
      */
    NIM_CR_5_6,
    /*!
      NIM code rate 6/7
      */
    NIM_CR_6_7,
    /*!
      NIM code rate 7/8
      */
    NIM_CR_7_8,
    /*!
      NIM code rate 8/9
      */
    NIM_CR_8_9,
    /*!
      NIM code rate, 9/10
      */
    NIM_CR_9_10,
    /*!
      NIM auto code rate
      */
    NIM_CR_AUTO
} fe_code_rate_t;

/*!
    NIM types used for "struct nim_config" "nim_type"
  */
typedef enum fe_type {
    /*!
      NIM for undefined type
      */
    NIM_UNDEF = 0x00,
    /*!
      NIM for DVB-S
      */
    NIM_DVBS = 0x01,
    /*!
      NIM for DVB-S2
      */
    NIM_DVBS2 = 0x02,
    /*!
      NIM for DVB-C
      */
    NIM_DVBC = 0x04,
    /*!
      NIM for ABS-S
      */
    NIM_ABSS = 0x05,
    /*!
      NIM for DVB-T
      */
    NIM_DVBT = 0x06,
    /*!
      NIM for DVB-T2
      */
    NIM_DVBT2 = 0x07,
    /*!
      NIM for test mode
      */
    NIM_TEST = 0x08,

    /*!
      NIM for for DVB-T only
      */
    NIM_DVBT_ONLY = 0x09,
    /*!
      NIM for for DVB-T2 only
      */
    NIM_DVBT2_ONLY = 0x0a,

    /*!
      NIM for DVBT  auto  will sercher dvbt2 first, if not lock then dvbt,
      */
    NIM_DVBT_AUTO = 0x0b,

    /*!
      NIM for DVBS  auto  will sercher dvbs2 first, if not lock then dvbs,
      */
    NIM_DVBS_AUTO = 0x0c,
    /*!
      NIM for DTMB
      */
    NIM_DTMB = 0x0d
} fe_type_t;

/*!
    I/Q inverted indecator used for "struct nim_config" "iq_polar"
  */
typedef enum fe_spectral_polar {
    /*!
      I/Q is normal
      */
    NIM_IQ_NORMAL = 0x00,
    /*!
      I/Q is inverted
      */
    NIM_IQ_INVERT,
    /*!
      Auto I/Q detection
      */
    NIM_IQ_AUTO
} fe_spectral_polar_t;

/*!
    Modulation mode used for "struct nim_channel_info" "modulation"
  */
typedef enum fe_modulation {
    /*!
      Auto modulation detection
      */
    NIM_MODULA_AUTO = 0,
    /*!
      BPSK
      */
    NIM_MODULA_BPSK,
    /*!
      QPSK
      */
    NIM_MODULA_QPSK,
    /*!
      8PSK
      */
    NIM_MODULA_8PSK,
    /*!
      4QAM
      */
    NIM_MODULA_4QAM,
    /*!
      4QAM NR
      */
    NIM_MODULA_4QAM_NR,
    /*!
      QAM 16
      */
    NIM_MODULA_QAM16,
    /*!
      QAM 32
      */
    NIM_MODULA_QAM32,
    /*!
      QAM 64
      */
    NIM_MODULA_QAM64,
    /*!
      QAM 128
      */
    NIM_MODULA_QAM128,
    /*!
      QAM 256
      */
    NIM_MODULA_QAM256,
    /*!
      16APSK
      */
    NIM_MODULA_16APSK,
    /*!
      32APSK
      */
    NIM_MODULA_32APSK,
    /*!
      modulation undef
      */
    NIM_MODULA_UNDEF,
} fe_modulation_t;

typedef enum fe_lock {
    NIM_UNLOCKED = 0,
    NIM_LOCKED = 1,
    NIM_SLEEP,
} fe_lock_t;

/* DiSEqC send message */
typedef struct
{
    mt_u32 port;
    mt_unf_fe_diseqc_sendmsg_t msg;
} fe_diseqc_sendmsg_t, *fe_diseqc_sendmsg_handle;

typedef struct mtfe_diseqc_recvmsg_t
{
    mt_u32 port;
    mt_unf_fe_diseqc_recvmsg_t *msg; /* Read data */
} fe_diseqc_recvmsg_t, *fe_diseqc_recvmsg_handle;

typedef struct mt_fe_set_tuner_t
{
    mt_u32 freq;
    mt_s32 times;
} fe_set_tuner_t, *fe_set_tuner_handle;

typedef struct mt_fe_signal_t
{
    mt_u32 tuner_id;
    mt_unf_fe_sig_type_t sig_type;
    fe_agc_qam_param_t signal;
    //mt_unf_fe_connect_para_t para;
} fe_signal_t, *fe_signal_handle;

typedef struct mtfe_tp_verify_param_t
{
    mt_u32 *pfrequency; /**<frequency kHz*/               /**<CNcomment:频率，单位kHz*/
    mt_u32 *psym_rate; /**<symbolrate kBaud*/             /**<CNcomment:符号率，单位kBaud*/
    mt_unf_fe_polar_t polarization; /**<TP polarization*/ /**<CNcomment:TP的极化方式*/
    mt_u8 cbs_reliablity; /**<TP reliability*/            /**<CNcomment:TP的可靠度*/
    mt_s32 cbs_th; /**<blindscan threshold*/              /**<CNcomment:盲扫阈值*/
    mt_u8 fs_grade;
    mt_u32 *fec_ok_cnt; /**<TP number of fec ok*/         /**<CNcomment:fec ok的TP数量*/
    mt_u32 *fec_no_ok_cnt; /**<TP number of fec  not ok*/ /**<CNcomment:fec not ok的TP数量*/
} fe_tp_verify_param_t;

typedef struct mt_fe_tp_verify_info_t
{
    mt_u32 port;
    fe_tp_verify_param_t tp_verify_prm;
} fe_tp_verify_info_t, *fe_tp_verify_info_handle;

typedef struct
{
    fe_data_src_t data_src;
    mt_u32 data_len;
    mt_unf_fe_sample_data_t *p_data;
} mt_fe_sample_data_param_t;

typedef enum {
    FE_QAMINSIDE_IN = 0,
    FE_QAMINSIDE_OUT,
    FE_QAMINSIDE_INVALID,
} mt_fe_qam_inside_t;

typedef struct
{
    mt_unf_tuner_type_t tuner_type; /**<Tuner type*/ /**<CNcomment:TUNER类型*/
    mt_u32 tuner_addr;
} mt_tuner_attr_t;

typedef struct
{
    mt_unf_demod_type_t demod_type; /**<QAM type*/ /**<CNcomment:QAM类型*/
    mt_u32 demod_addr;
    mt_u32 demod_xtal_clk;
    //mt_u32 use_unicable;
} mt_demod_attr_t;

typedef struct
{
    mt_u8 *sendbuf;
    mt_u32 sendlen;
    mt_u8 *recvbuf;
    mt_u32 recvlen;
} fe_i2c_data_t, *fe_i2c_data_handle;

typedef struct mt_agc_test_t
{
    mt_u32 port;
    mt_u32 agc1;
    mt_u32 agc2;
    MT_BOOL b_lockflag;
    MT_BOOL b_agc_lockflag;
    mt_u8 b_agc_ctrl;
    mt_u32 count;
} agc_test_t;

typedef struct _fe_attr_t
{
    mt_u32 tuner_id;
    mt_unf_fe_attr_t attr;
} fe_attr_t;

typedef struct _fe_connect_para_t
{
    mt_u32 tuner_id;
    mt_unf_fe_connect_para_t para;
} fe_connect_para_t;

typedef struct _fe_status_t
{
    mt_u32 tuner_id;
    mt_unf_fe_status_t status;
} fe_status_t;

typedef struct _fe_ber_t
{
    mt_u32 tuner_id;
    mt_u32 ber[3];
} fe_ber_t;
typedef struct _fe_accurate_snr_t
{
    mt_u32 tuner_id;
    mt_s32 snr;
} fe_accurate_snr_t;
typedef struct _fe_snr_t
{
    mt_u32 tuner_id;
    mt_u32 snr;
} fe_snr_t;

typedef struct _fe_signal_quality_t
{
    mt_u32 tuner_id;
    mt_u32 quality;
} fe_signal_quality_t;

typedef struct _fe_def_timeout_t
{
    mt_u32 tuner_id;
    mt_u32 timeout;
} fe_def_timeout_t;

typedef struct _fe_set_io_t
{
    mt_u32 tuner_id;
    MT_BOOL onoff;
} fe_set_io_t;

typedef struct
{
    mt_u32 tuner_id;
    MT_BOOL onoff;
} fe_set_lnb_onoff_t;

typedef struct
{
    mt_u32 tuner_id;
    MT_BOOL onoff;
} fe_set_22k_onoff_t;

typedef struct
{
    mt_u32 tuner_id;
    mt_unf_fe_lnb_polar polar;
} fe_set_polar_t;

#if 0
typedef struct
{
    mt_u32 tuner_id;
    mt_unf_fe_blindscan_para_t scan_info;
} fe_blind_scan_t;
#endif

typedef struct
{
    mt_u32 tuner_id;
    mt_unf_fe_channel_info_t info;
} fe_channel_info_t;

typedef struct
{
    mt_u32 tuner_id;
    mt_unf_fe_blindscan_para_t *p_blind_scan_info;
} fe_bs_result_t;

typedef struct
{
    mt_u32 tuner_id;
    mt_unf_fe_diseqc_cmd_t diseqc_cmd;
} fe_diseqc_control_t;

typedef struct
{
    mt_u32 tuner_id;
    mt_u32 total_bit_rate;
    mt_u32 valid_bit_rate;
} fe_bit_rate_t;

typedef struct
{
    mt_u32 tuner_id;
    mt_unf_fe_gse_label_filter label_filter;
} fe_gse_label_filter_t;

#if 0
typedef enum _MT_FE_RET
{
	MtFeErr_Ok					= 0
	,MtFeErr_Undef				= -1
	,MtFeErr_Uninit				= -2
	,MtFeErr_Param				= -3
	,MtFeErr_NoSupportFunc		= -4
	,MtFeErr_NoSupportTuner		= -5
	,MtFeErr_NoSupportDemod		= -6
	,MtFeErr_UnLock				= -7
	,MtFeErr_I2cErr				= -8
	,MtFeErr_DiseqcBusy			= -9
	,MtFeErr_NoMemory			= -10
	,MtFeErr_NullPointer		= -11
	,MtFeErr_TimeOut			= -12
	,MtFeErr_Fail				= -13
} MT_FE_RET;
#endif

/* standard tuner operation */
typedef struct _fe_ops_s
{
#if 0
	mt_unf_fe_sig_type_t sig_type;
    mt_mt_u8 tuner_i2c_id;
    mt_mt_u8 demod_i2c_id;
    mt_unf_tuner_type_t   tuner_type;
    mt_unf_demod_type_t	  demod_type;
	fe_agc_qam_param_t    current_param;
	mt_unf_fe_output_mode_t	ts_type;

	mt_u32	demod_addr;
	mt_u32	xtal_clk;
	mt_u32  tuner_addr;
	mt_u32  qam_current_mode;
	mt_mt_u8   adc_type;
	mt_mt_u8   agc_out_sel;
	mt_mt_u8   adc_data_fmt;
#endif

#if 0
    //mt_s32 (*fe_connect)(mt_u32 port, fe_agc_qam_param_t *p_qam);
    MT_FE_RET (*fe_connect)(void *handle, mt_u32 freq, mt_u32 sym_rate);
    MT_FE_RET (*fe_get_status)(void *handle, mt_unf_fe_lock_status_t *p_status, mt_u32 is_bs);
    MT_FE_RET (*fe_get_ber)(void *handle, mt_u32 *p_ber);
    MT_FE_RET (*fe_get_snr)(void *handle, mt_s8 *p_snr);
    MT_FE_RET (*fe_get_signal_strength)(void *handle, mt_s8 *p_strength);
    MT_FE_RET (*fe_get_signal_quality)(void *handle, mt_u32 *p_quality);//montage
    MT_FE_RET (*fe_set_ts_type)(void *handle, mt_unf_fe_output_mode_t ts_type);
    MT_FE_RET (*set_fe)(void *handle, mt_u8 i2c_id, mt_u32 rfagc);
	MT_FE_RET (*fe_get_freq_symb_offset)(void *handle, mt_u32 *pfreq, mt_u32 *p_symb);
	MT_FE_RET  (*fe_get_rs)(void *handle, mt_u32 *p_rs);

#else

    //mt_s32 (*fe_connect)(void *handle, mt_u32 freq, mt_u32 sym_rate);
    //mt_s32 (*fe_connect)(void *handle, mt_u32 freq, mt_u32 sym_rate, mt_u32 dvb_type, mt_u32 is_bs);
    mt_s32 (*fe_connect)(mt_u32 port, void *handle);

    //mt_s32 (*fe_get_status)(void *handle, mt_unf_fe_lock_status_t *p_status, mt_u32 is_bs);
    //mt_s32 (*fe_get_status)(mt_u32 port, void *handle, void *p_status, mt_u32 is_bs);
    mt_s32 (*fe_get_status)(mt_u32 port, void *handle, void *p_status);
    mt_s32 (*fe_get_ber)(mt_u32 port, void *handle, mt_u32 *p_ber);
    mt_s32 (*fe_get_snr)(mt_u32 port, void *handle, mt_s8 *p_snr);
    mt_s32 (*fe_get_signal_strength)(mt_u32 port, void *handle, mt_s8 *p_strength);
    mt_s32 (*fe_get_signal_quality)(mt_u32 port, void *handle, mt_u32 *p_quality); //montage
    mt_s32 (*fe_set_ts_type)(mt_u32 port, void *handle, mt_unf_fe_output_mode_t ts_type);
    mt_s32 (*set_fe)(void *handle, mt_u8 i2c_id, mt_u32 rfagc);
    mt_s32 (*fe_get_freq_symb_offset)(void *handle, mt_u32 *pfreq, mt_u32 *p_symb);
    mt_s32 (*fe_get_rs)(void *handle, mt_u32 *p_rs);
#endif

    mt_void (*manage_after_chipreset)(mt_u32 port);
    mt_void (*recalculate_signal_strength)(mt_u32 port, mt_u32 *p_strength);
    mt_void (*fe_test_single_agc)(mt_u32 port, agc_test_t *p_agctest); /* just for test */
    mt_void (*fe_resume)(mt_u32 port);
    mt_void (*fe_get_registers)(mt_u32 port, void *p);
    mt_void (*fe_connect_timeout)(mt_u32 timeout);

    //mt_s32 (*fe_init)(mt_u32 port, mt_u8 i2c_id, mt_unf_tuner_type_t cur_tuner_type);
    mt_s32 (*fe_init)(mt_u32 port, void *handle);
    mt_s32 (*fe_set_sat_attr)(mt_u32 port, mt_unf_fe_sat_attr_t *p_sat_attr);
    mt_s32 (*fe_set_ter_attr)(mt_u32 port, mt_unf_fe_ter_attr_t *p_ter_attr);
    mt_s32 (*fe_get_signal_info)(mt_u32 port, mt_unf_fe_signal_info_t *pinfo);

    mt_s32 (*fe_blindscan_init)(mt_u32 port, fe_blindscan_init_param_t *p_param);
    mt_s32 (*fe_blindscan_action)(void *handle, mt_u32 begin_freq, mt_u32 end_freq, void *p_bs_info, mt_u32 is_connect);
    //mt_s32 (*fe_blindscan_action)(void *handle, fe_blindscan_param_t *p_param);
    //MT_FE_RET mt_fe_dmd_ds3k_blindscan(MT_FE_DS3K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info, U32 is_connect)

    mt_s32 (*fe_terscan_action)(mt_u32 port, mt_unf_fe_ter_scan_attr_t *p_scan_param, mt_unf_fe_ter_tpinfo_t *p_tpinfo);
    mt_s32 (*fe_lnbctrl_dev_init)(mt_u32 port, mt_u32 i2c_num, mt_u8 dev_addr,
                                  mt_unf_demod_type_t demod_type);
    mt_s32 (*fe_lnbctrl_dev_standby)(mt_u32 port, mt_u32 standby);
    mt_s32 (*fe_set_lnb_out)(mt_u32 port, fe_lnb_out_t out_voltage);
    mt_s32 (*fe_send_continuous_22K)(mt_u32 port, mt_u32 continuous_22k);
    mt_s32 (*fe_send_tone)(mt_u32 port, mt_u32 tone);
    mt_s32 (*fe_diseqc_send_msg)(mt_u32 port, mt_unf_fe_diseqc_sendmsg_t *p_msg);
    mt_s32 (*fe_diseqc_recv_msg)(mt_u32 port, mt_unf_fe_diseqc_recvmsg_t *p_msg);

    mt_s32 (*fe_standby)(mt_u32 port, void *handle);
    mt_s32 (*fe_wakeup)(mt_u32 port, void *handle); //montage

    mt_s32 (*fe_set_func_mode)(mt_u32 port, fe_func_mode_t func_mode);
    mt_s32 (*fe_set_plp_id)(mt_u32 port, mt_u8 plp_id, mt_u32 mode);
    mt_s32 (*fe_set_common_plp_id)(mt_u32 port, mt_u8 plp_id);
    mt_s32 (*fe_set_common_plp_combination)(mt_u32 port, mt_u8 com_plpena);
    mt_s32 (*fe_get_plp_num)(mt_u32 port, mt_u8 *p_plpnum);
    mt_s32 (*fe_get_current_plp_type)(mt_u32 port, mt_unf_fe_t2_plp_type_t *p_plptype);
    mt_s32 (*fe_get_plp_id)(mt_u32 port, mt_u8 *p_plpid);
    mt_s32 (*fe_get_group_plp_id)(mt_u32 port, mt_u8 *p_plp_grpid);
    mt_s32 (*fe_set_antena_power)(mt_u32 port, mt_unf_fe_ter_antenna_power_t power);
    mt_s32 (*fe_tp_verify)(mt_u32 port, fe_tp_verify_param_t *p_channel);
    mt_s32 (*fe_set_ts_out)(mt_u32 port, mt_unf_fe_ts_out_t *p_tsout);
    mt_s32 (*fe_data_sample)(mt_u32 port, fe_data_src_t data_src, mt_u32 data_len, mt_unf_fe_sample_data_t *p_data);
    mt_s32 (*fe_get_agc)(mt_u32 port, mt_u32 center, mt_u32 *agc);
    mt_s32 (*fe_get_default_timeout)(mt_u32 port, mt_u32 *timeout);
    mt_s32 (*fe_set_io)(mt_u32 port, MT_BOOL onoff);

    //mt_s32 (*tuner_init)(mt_u32 port, mt_u8 i2c_id, mt_unf_tuner_type_t cur_tuner_type);
    mt_s32 (*tuner_init)(mt_u32 port, void *handle);
    mt_s32 (*tuner_set)(mt_u32 port, void *handle, mt_u32 freq, mt_u32 sym_rate, mt_s16 lpf_offset);
    mt_s32 (*tuner_strength)(mt_u32 port, void *handle, mt_s8 *p_strength);
    mt_s32 (*tuner_sleep)(mt_u32 port, void *handle);
    mt_s32 (*tuner_wakeup)(mt_u32 port, void *handle);
    //void *handle;
} fe_ops_s;

#if 0
typedef struct _frontend_info_s
{
    //int is_attach;

    mt_unf_fe_sig_type_t sig_type;
    mt_u8 tuner_i2c_id;
    mt_u8 demod_i2c_id;
    mt_unf_tuner_type_t tuner_type;
    mt_unf_demod_dev_type_t	demod_type;
	fe_agc_qam_param_t current_param;
	mt_unf_fe_output_mode_t	ts_type;
	MT_BOOL tuner_bus_on;/* 2-wire bus repeater for tuner is ON or not */

	mt_u32	demod_addr;
	mt_u32	xtal_clk;/* demod clock */
	mt_u32  tuner_addr;
	mt_u32  qam_current_mode;
	mt_u8   adc_type;
	mt_u8   agc_out_sel;
	mt_u8   adc_data_fmt;

	fe_config_t pin_cfg;

	fe_ops_s ops;
	//struct fe_events events;
	//struct semaphore sem;

    //MT_UNF_TUNER_ATTR_S pre_attr;
    void *handle;
} frontend_info_s;
#endif

typedef struct _demod_ops_s
{
    int (*connect)(void *handle, mt_unf_fe_connect_para_t *para);
    int (*get_status)(void *handle, mt_unf_fe_status_t *status);
    int (*get_ber)(void *handle, mt_u32 *ber);
    int (*get_snr)(void *handle, mt_u32 *snr);
    int (*get_signal_strength)(void *handle, mt_u32 *strength);
    int (*get_signal_quality)(void *handle, mt_u32 *quality);
    int (*get_signal_agc)(void *handle, mt_u32 *agc);
    int (*standby)(void *handle);
    int (*wakeup)(void *handle);
    int (*get_default_timeout)(void *handle, mt_u32 *timeout);
    int (*set_io)(void *handle, MT_BOOL onoff);
    int (*port_ioctl)(void *handle, mt_u32 cmd, ulong param);
    int (*connect_asychronous)(void *handle, mt_unf_fe_connect_para_t *para);
    //int (*blind_scan)(void *handle, mt_unf_fe_scan_info_t *para);
    //int (*blind_scan)(void *handle, mt_unf_fe_blindscan_para_t *para);
    int (*blind_scan)(void *handle, fe_blindscan_param_t *para);
} demod_ops_s;

typedef struct _frontend_info_s
{
    int is_attach;
    demod_ops_s ops;
    mt_unf_fe_attr_t pre_attr;
    void *handle;
} frontend_info_s;

typedef enum _PORT_FRONTEND_IOCTL_CMD_T {
    NIM_IOCTRL_CHANNEL_CHECK_LOCK,
    NIM_IOCTRL_CHANGE_TN_MODE,
    NIM_IOCTRL_DISEQC1X,
    NIM_IOCTRL_DISEQC2X,
    NIM_IOCTRL_SET_PORLAR,
    NIM_IOCTRL_SET_LNB_ONOFF,
    NIM_IOCTRL_CHECK_LNB_SC_PROT,
    NIM_IOCTRL_LNB_SC_PROT_RESTORE,
    NIM_IOCTRL_REMOVE_PROTECT,
    NIM_IOCTRL_ENABLE_CHECK_PROTECT,
    NIM_IOCTRL_SET_22K_ONOFF,
    NIM_IOCTRL_GET_PORLAR,
    NIM_IOCTRL_GET_22K_ONOFF,
    NIM_IOCTRL_GET_TN_VERSION,
    NIM_IOCTRL_SCAN_CANCEL,
    NIM_IOCTRL_GET_SCAN_STATUS,
    NIM_IOCTRL_GET_SCAN_RESULT,
    NIM_IOCTRL_RECOVER,
    NIM_IOCTRL_SET_CHANNEL_INFO,
    NIM_IOCTRL_GET_SIGNAL_INFO,
    NIM_IOCTRL_GET_CHANNEL_INFO,
    NIM_IOCTRL_SET_DM_GPIO0_OUTPUT,
    NIM_IOCTRL_GET_DM_GPIO0_INPUT,
    NIM_IOCTRL_SET_DM_GPIO1_OUTPUT,
    NIM_IOCTRL_GET_DM_GPIO1_INPUT,
    NIM_IOCTRL_DISEQC_SENDMSG,
    NIM_IOCTRL_DISEQC_SEND_TONEBURST,
    NIM_IOCTRL_DISEQC_RECVMSG,

	// for DVB-T2
	NIM_IOCTRL_T2_GET_PLP_NUM,					// Get DVB-T2 plp number
	NIM_IOCTRL_T2_SET_PLP_NO,					// Set DVB-T2 data_plp id
	NIM_IOCTRL_T2_GET_PLP_NO,					// Get DVB-T2 data_plp id

	NIM_IOCTRL_T2_GET_PLP_INFO,					// Get DVB-T2 plp info

	NIM_IOCTRL_T2_SET_COMMON_PLP_ID,			// Set DVB-T2 common_plp id
	NIM_IOCTRL_T2_GET_COMMON_PLP_ID,			// Get DVB-T2 common_plp id

	// for DVB-T
	NIM_IOCTRL_T_GET_HIERARCHY_NUM,				// Get DVB-T hierarchy number
	NIM_IOCTRL_T_SET_HIERARCHY_NO,				// Set DVB-T hierarchy id
	NIM_IOCTRL_T_GET_HIERARCHY_NO,				// Get DVB-T hierarchy id

	NIM_IOCTRL_T_T2_GET_CELL_ID,				// Get DVB-T/T2 cell id

	// for DVB-S/S2
	NIM_IOCTRL_SAT_BS_EVENT_PROCESSED,

	NIM_IOCTRL_GET_SAT_REAL_FREQ,
	NIM_IOCTRL_GET_SAT_FREQ_OFFSET,
	NIM_IOCTRL_GET_SAT_REAL_SYM,

	NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_CNT,
	NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_LIST,
	NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID,


	NIM_IOCTRL_SET_MODULE_INDEX,			// Select current module index for multi-frontend applications
	NIM_IOCTRL_TEST_FOR_GSE,				//for gse test
	NIM_IOCTRL_GET_TN_DBG_INFO,

	NIM_IOCTRL_GET_PRE_BER,

	NIM_IOCTRL_SET_SUPER_SEARCH,			// switch super search mode on/off

	NIM_IOCTRL_UNICABLE_RETRY,				// re-send last DiSEqC message for Unicable

	NIM_IOCTRL_SUSPEND,
	NIM_IOCTRL_RESUME,
	NIM_IOCTRL_DSS_SCID_FILTER,

	NIM_IOCTRL_SET_TN_PARAM,
	NIM_IOCTRL_GET_TS_GSE_MODE,
	NIM_IOCTRL_SET_GS_PACKAGE_MODE,
	NIM_IOCTRL_GET_SNR_TEN_DIVIDE,
	NIM_IOCTRL_REGISTER_NLK_FAMILY,
	NIM_IOCTRL_SET_LOWPOWER,
	NIM_IOCTRL_GET_ACCURATE_SNR,            // Get accurate SNR value (Unit: 0.001 dB)
	NIM_IOCTRL_SET_GS_LABEL_FILTER,
	NIM_IOCTRL_SET_TN_GLOBAL_RESET,
	NIM_IOCTRL_SET_BBFRAME_PADDING_ONOFF,
	NIM_IOCTRL_GET_FAST_LOCK,
} PORT_FRONTEND_IOCTL_CMD;

//extern  i2c_ext_func_t *p_i2c_func;
//extern  gpio_ext_func_t *p_gpio_func;
//extern  gpioi2c_ext_func_t *p_gpioi2c_func;

/*---- FE COMMAND----*/
#define MT_FE_IOC_MAGIC 't'
#define FE_SET_ATTR_CMD _IOW(MT_FE_IOC_MAGIC, 0, fe_attr_t)
#define FE_CONNECT_CMD _IOW(MT_FE_IOC_MAGIC, 1, fe_connect_para_t)
//#define FE_CONNECT_CMD 		 		_IOW(MT_FE_IOC_MAGIC, 1, fe_signal_t)
#define FE_GET_STATUS_CMD _IOWR(MT_FE_IOC_MAGIC, 2, fe_status_t)
//#define FE_GET_STATUS_CMD      		_IOWR(MT_FE_IOC_MAGIC, 2, fe_data_t)
#define FE_GET_BER_CMD _IOWR(MT_FE_IOC_MAGIC, 3, fe_ber_t)
//#define FE_GET_BER_CMD 		 		_IOWR(MT_FE_IOC_MAGIC, 3, fe_databuf_t)
#define FE_GET_SNR_CMD _IOWR(MT_FE_IOC_MAGIC, 4, fe_snr_t)
//#define FE_GET_SNR_CMD		 		_IOWR(MT_FE_IOC_MAGIC, 4, fe_data_t)
#define FE_GET_SIGNAL_STRENGTH_CMD _IOWR(MT_FE_IOC_MAGIC, 5, fe_signal_strength_t)
//#define FE_GET_SIGNAL_STRENGTH_CMD  	_IOWR(MT_FE_IOC_MAGIC, 5, fe_databuf_t)
#define FE_GET_SIGNAL_QUALITY_CMD _IOWR(MT_FE_IOC_MAGIC, 6, fe_signal_quality_t)
//#define FE_GET_SIGNAL_QUALITY_CMD     _IOWR(MT_FE_IOC_MAGIC, 6, fe_databuf_t)
#define FE_STANDBY_CMD _IOW(MT_FE_IOC_MAGIC, 7, mt_u32)
//#define FE_STANDBY_CMD 				_IOWR(MT_FE_IOC_MAGIC, 7, fe_data_t)
#define FE_WAKEUP_CMD _IOW(MT_FE_IOC_MAGIC, 8, mt_u32)
//#define FE_WAKEUP_CMD 			 	_IOWR(MT_FE_IOC_MAGIC, 8, fe_data_t)

//#define FE_SET_TSTYPE_CMD _IOW(MT_FE_IOC_MAGIC, 6, fe_data_t)
//#define FE_SELECT_TYPE_CMD _IOW(MT_FE_IOC_MAGIC, 7, fe_databuf_t)
//#define FE_SELECT_I2C_CMD _IOW(MT_FE_IOC_MAGIC, 8, fe_data_t)
//#define FE_SELECT_RW_CMD _IOWR(MT_FE_IOC_MAGIC, 9, TUNER_RegRW_S)
#define FE_GET_DEFAULT_TIMEOUT_CMD _IOWR(MT_FE_IOC_MAGIC, 9, fe_def_timeout_t)

#if 0
#define FE_SET_TUNER_CMD _IOWR(MT_FE_IOC_MAGIC, 10, fe_set_tuner_t)
#define FE_LOW_CONS_CMD _IO(MT_FE_IOC_MAGIC, 11)
#define FE_NORMAL_MODE_CMD _IO(MT_FE_IOC_MAGIC, 12)
//#define FE_SET_QAMINSIDE_CMD _IOR(MT_FE_IOC_MAGIC, 13, fe_qam_inside_t)
#define FE_CONNECT_UNBLOCK_CMD _IOW(MT_FE_IOC_MAGIC, 14, fe_signal_t)
#define FE_SELECT_SYMBOLRATE_CMD _IOR(MT_FE_IOC_MAGIC, 15, fe_data_t)
#define FE_CHECK_VALID_I2CADDR _IOW(MT_FE_IOC_MAGIC, 16, fe_data_t)
#define FE_TEST_SINGLE_AGC _IOWR(MT_FE_IOC_MAGIC, 17, agc_test_t)
#define FE_GET_FREQ_SYMB_OFFSET _IOWR(MT_FE_IOC_MAGIC, 18, fe_databuf_t)
#define FE_CONNECT_TIMEOUT_CMD _IOWR(MT_FE_IOC_MAGIC, 19, fe_databuf_t)
#define FE_IOC_SET_IO _IOW(MT_FE_IOC_MAGIC, 12, fe_set_io_t)

#else

#define FE_CHANNEL_CHECK_LOCK_CMD _IOR(MT_FE_IOC_MAGIC, 10, mt_u8)
#define FE_SET_LNBOUT_CMD _IOWR(MT_FE_IOC_MAGIC, 11, fe_lnb_out_t)
#define FE_SET_PORLAR_CMD _IOW(MT_FE_IOC_MAGIC, 12, mt_u8)
#define FE_DISEQC1X_CMD _IOW(MT_FE_IOC_MAGIC, 14, mt_u8)
#define FE_DISEQC2X_CMD _IOW(MT_FE_IOC_MAGIC, 15, mt_u8)

#define FE_GET_AGC_CMD _IOWR(MT_FE_IOC_MAGIC, 16, fe_data_t)

#define FE_CHECK_LNB_SC_PROT_CMD _IOW(MT_FE_IOC_MAGIC, 18, mt_u8)
#define FE_LNB_SC_PROT_RESTORE_CMD _IOW(MT_FE_IOC_MAGIC, 19, mt_u8)
#endif

#define FE_GET_SIGNAL_INFO_CMD _IOWR(MT_FE_IOC_MAGIC, 21, fe_signal_info_t)
#define FE_BLINDSCAN_INIT_CMD _IOWR(MT_FE_IOC_MAGIC, 22, fe_blindscan_t)
#define FE_BLINDSCAN_ACTION_CMD _IOWR(MT_FE_IOC_MAGIC, 23, fe_blindscan_info_t)
#define FE_SEND_CONTINUOUS_22K_CMD _IOWR(MT_FE_IOC_MAGIC, 25, fe_set_22k_onoff_t)
#define FE_SEND_TONE_CMD _IOWR(MT_FE_IOC_MAGIC, 26, fe_data_t)
#define FE_DISEQC_SEND_MSG_CMD _IOWR(MT_FE_IOC_MAGIC, 27, fe_diseqc_sendmsg_t)
#define FE_DISEQC_RECV_MSG_CMD _IOWR(MT_FE_IOC_MAGIC, 28, fe_diseqc_recvmsg_t)

#define FE_DISABLE_CMD _IOWR(MT_FE_IOC_MAGIC, 30, fe_data_t)
#define FE_SET_FUNCMODE_CMD _IOW(MT_FE_IOC_MAGIC, 31, fe_data_t)
#define FE_SET_PLPNO_CMD _IOW(MT_FE_IOC_MAGIC, 32, fe_data_t)
#define FE_GET_PLPNUM_CMD _IOWR(MT_FE_IOC_MAGIC, 33, fe_data_t)
#define FE_GET_CURPLPTYPE_CMD _IOWR(MT_FE_IOC_MAGIC, 34, fe_data_t)
#define FE_SET_TSOUT_CMD _IOWR(MT_FE_IOC_MAGIC, 35, fe_data_t)

#define FE_TPVERIFY_CMD _IOWR(MT_FE_IOC_MAGIC, 36, fe_tp_verify_info_t)
#define FE_SET_DEMODATTR_CMD _IOWR(MT_FE_IOC_MAGIC, 37, fe_data_t)
#define FE_SAMPLE_DATA_CMD _IOWR(MT_FE_IOC_MAGIC, 38, fe_data_t)
#define FE_SET_COMMONPLP_CMD _IOW(MT_FE_IOC_MAGIC, 39, fe_data_t)
#define FE_SET_COMMONPLP_COMBINATION_CMD _IOW(MT_FE_IOC_MAGIC, 40, fe_data_t)
#define FE_GET_PLP_ID_CMD _IOWR(MT_FE_IOC_MAGIC, 41, fe_data_t)
#define FE_GET_GROUP_PLP_ID_CMD _IOWR(MT_FE_IOC_MAGIC, 42, fe_data_t)
#define FE_SET_ANTENNA_POWER_CMD _IOW(MT_FE_IOC_MAGIC, 43, fe_data_t)
#define FE_TERSCAN_ACTION_CMD _IOWR(MT_FE_IOC_MAGIC, 44, fe_terscan_info_t)
#define FE_GET_AGC _IOWR(MT_FE_IOC_MAGIC, 45, fe_databuf_t)
#define FE_OPEN _IOW(MT_FE_IOC_MAGIC, 46, )
#define FE_SET_INIT_CFG_CMD _IOW(MT_FE_IOC_MAGIC, 47, fe_config_t)

#define FE_BLIND_SCAN_CANCEL_CMD _IOW(MT_FE_IOC_MAGIC, 49, mt_u8)
#define FE_CHECK_BLIND_SCAN_STATUS_CMD _IOW(MT_FE_IOC_MAGIC, 50, fe_data_t)
#define FE_GET_BLIND_SCAN_RESULT_CMD _IOWR(MT_FE_IOC_MAGIC, 51, fe_blindscan_info_t)

#define FE_BLIND_SCAN_EVENT_CMD _IOW(MT_FE_IOC_MAGIC, 52, mt_u8)
#define FE_SET_LNB_ONOFF_CMD _IOWR(MT_FE_IOC_MAGIC, 53, fe_lnb_out_t)

#define FE_GET_REAL_FREQ _IOWR(MT_FE_IOC_MAGIC, 54, fe_data_t)
#define FE_GET_REAL_SYMBOL _IOWR(MT_FE_IOC_MAGIC, 55, fe_data_t)
#define FE_GET_FREQ_OFFSET _IOWR(MT_FE_IOC_MAGIC, 56, fe_data_t)
#define FE_GET_SYM_OFFSET _IOWR(MT_FE_IOC_MAGIC, 57, fe_data_t)

#define FE_GET_S2_MULTI_STREAM_TS_CNT _IOWR(MT_FE_IOC_MAGIC, 58, fe_data_t)
//#define FE_GET_S2_MULTI_STREAM_TS_ID _IOWR(MT_FE_IOC_MAGIC, 59, fe_data_t)
#define FE_GET_S2_MULTI_STREAM_TS_ID _IOWR(MT_FE_IOC_MAGIC, 59, fe_datalist_t)
#define FE_SET_S2_MULTI_STREAM_TS_ID _IOWR(MT_FE_IOC_MAGIC, 60, fe_data_t)

#define FE_GET_T_T2_CELL_ID_CMD _IOWR(MT_FE_IOC_MAGIC, 61, fe_data_t)

#define FE_GET_TN_DBG_INFO_CMD _IOWR(MT_FE_IOC_MAGIC, 62, fe_data_t)

#define FE_GET_HIERARCHY_NUM_CMD _IOWR(MT_FE_IOC_MAGIC, 63, fe_data_t)
#define FE_SET_HIERARCHY_ID_CMD _IOW(MT_FE_IOC_MAGIC, 64, fe_data_t)
#define FE_GET_HIERARCHY_ID_CMD _IOWR(MT_FE_IOC_MAGIC, 65, fe_data_t)

#define FE_GET_S2_GSE_ID_CMD _IOWR(MT_FE_IOC_MAGIC, 66, fe_data_t)

#define FE_SUSPEND_CMD _IOW(MT_FE_IOC_MAGIC, 67, mt_u32)
#define FE_RESUME_CMD _IOW(MT_FE_IOC_MAGIC, 68, mt_u32)

#define FE_DSS_FILTER_CMD  _IOW(MT_FE_IOC_MAGIC, 69, mt_u32)

#define FE_GET_PRE_BER_CMD _IOW(MT_FE_IOC_MAGIC, 75, fe_ber_t)

#define FE_SET_TUNER_PARAM_CMD _IOW(MT_FE_IOC_MAGIC, 80, fe_tn_param_t)
#define FE_GET_S2_BBH_TS_GSE_MODE_CMD _IOWR(MT_FE_IOC_MAGIC, 81, fe_data_t)
#define FE_SET_S2_GS_PACKAGE_MODE_CMD _IOWR(MT_FE_IOC_MAGIC, 82, fe_data_t)
#define FE_SET_S2_GS_LABEL_FILTER_CMD _IOWR(MT_FE_IOC_MAGIC, 83, fe_gse_label_filter_t)

#define FE_SET_SUPER_SEARCH_CMD _IOW(MT_FE_IOC_MAGIC, 90, fe_data_t)
#define FE_SET_UNICABLE_RETRY_CMD _IOW(MT_FE_IOC_MAGIC, 91, fe_data_t)
#define FE_REGISTER_NLK_FAMILY_CMD _IOW(MT_FE_IOC_MAGIC, 92, fe_data_t)
#define FE_SET_LOWPOWER_CMD _IOW(MT_FE_IOC_MAGIC, 93, fe_data_t)
#define FE_GET_ACCURATE_SNR_CMD _IOWR(MT_FE_IOC_MAGIC, 94, fe_accurate_snr_t)
#define FE_SET_BBFARME_PADDING_CMD _IOW(MT_FE_IOC_MAGIC, 95, fe_data_t)
#define FE_GET_FAST_LOCK_CMD _IOWR(MT_FE_IOC_MAGIC, 96, fe_status_t)

extern fe_ops_s g_fe_ops[FE_NUM];
extern mt_u32 g_reset_gpio_no[FE_NUM];
extern mt_u32 g_reset_crugpio_no;

extern int m88ca8k_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88ca8k_detach(frontend_info_s *info);
#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_CABLE
extern int m88cs8k_cab_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88cs8k_cab_detach(frontend_info_s *info);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE
extern int m88cs8k_sat_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88cs8k_sat_detach(frontend_info_s *info);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DM6K
extern int m88dm6k_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88dm6k_detach(frontend_info_s *info);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_CT8K
extern int m88ct8k_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88ct8k_detach(frontend_info_s *info);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DD3K
extern int m88dd3k_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88dd3k_detach(frontend_info_s *info);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_TENOR
extern int m88tc6930_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88tc6930_detach(frontend_info_s *info);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_CS8800
extern int m88cs8800_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88cs8800_detach(frontend_info_s *info);

extern int m88cs8800_suspend(void);
extern int m88cs8800_resume(void);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD2856
extern int sony_cxd2856_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int sony_cxd2856_detach(frontend_info_s *info);
extern int sony_cxd2856_suspend(void *handle);
extern int sony_cxd2856_resume(void *handle);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY
extern int sony_cxd_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int sony_cxd_detach(frontend_info_s *info);
extern int sony_cxd_suspend(void *handle);
extern int sony_cxd_resume(void *handle);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_RS6060
extern int m88rs6060_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88rs6060_detach(frontend_info_s *info);

extern int m88rs6060_suspend(void);
extern int m88rs6060_resume(void);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DS6103
extern int m88ds6103_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88ds6103_detach(frontend_info_s *info);

extern int m88ds6103_suspend(void);
extern int m88ds6103_resume(void);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DS6113
extern int m88ds6113_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int m88ds6113_detach(frontend_info_s *info);

extern int m88ds6113_suspend(void);
extern int m88ds6113_resume(void);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_TP5001
extern int tp5001_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int tp5001_detach(frontend_info_s *info);

extern int tp5001_suspend(void);
extern int tp5001_resume(void);
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_HD2502
extern int hd2502_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr);
extern int hd2502_detach(frontend_info_s *info);

extern int hd2502_suspend(void);
extern int hd2502_resume(void);
#endif

mt_void mt_unf_fe_calc_PLS_gold_code(mt_u8 *pNormalCode, mt_u32 PLSGoldCode);


#endif

#endif
