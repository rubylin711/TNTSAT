/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef __MT_FE_TN_TS6011_DEF__
#define __MT_FE_TN_TS6011_DEF__

#include "port_hd2502.h"
#include "mt_type.h" //flax
//#include "port_hd2502.h"//mt_fe_i2c_ds6103.h"


#define MT_FE_TS6011_DEBUG_PRNT                         0        /*    0 off, 1 on*/
#if (MT_FE_TS6011_DEBUG_PRNT == 1)
    #define mt_fe_print_ts6011(str)                printk str
#else
    #define mt_fe_print_ts6011(str)
#endif

#define TS6011_I2C_ADDRESS    0x58

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x)        ((void)(x))
#endif

typedef struct _MT_FE_TN_Device_Settings_TS6011
{
    U8      tuner_init_OK;
    U8      tuner_dev_addr;

    U16     tuner_custom_cfg;
    U32     tuner_version;
    U32     tuner_time;

    U32     tuner_freq_MHz;
    U32     tuner_symbol_rate_KSs;
    U32     tuner_lo_freq_KHz;
    U32     tuner_lpf_offset_KHz;

    U32     tuner_crystal_KHz;

    U8      tuner_input_mode;
    U8      tuner_clock_out;
} MT_FE_TN_DEVICE_SETTINGS_TS6011, *MT_FE_Tuner_Handle_TS6011;


//extern S32 _mt_fe_tn_get_reg_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 *reg_data);
//extern S32 _mt_fe_tn_set_reg_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 reg_data);
//extern S32 _mt_fe_tn_set_reg_bit_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit);
extern void _mt_sleep_hd2502(U32 ticks_ms);

S32 mt_fe_tn_init_ts6011_hd2502(void *handle);
void mt_fe_tn_ts6011_init_hd2502(MT_FE_Tuner_Handle_TS6011 handle);
S32 mt_fe_tn_ts6011_set_freq_hd2502(MT_FE_Tuner_Handle_TS6011 handle, U32 Freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
void mt_fe_tn_adjust_AGC_ts6011_hd2502(MT_FE_Tuner_Handle_TS6011 handle);
S32 mt_fe_tn_ts6011_get_freq_offset_hd2502(MT_FE_Tuner_Handle_TS6011 handle);
//S32 mt_fe_tn_ts6011_get_gain_hd2502(MT_FE_Tuner_Handle_TS6011 handle, U32 Vagc);
S32 mt_fe_tn_ts6011_get_signal_strength_hd2502(MT_FE_Tuner_Handle_TS6011 handle, S32 Vagc, MT_BOOL bLocked);
//MT_FE_RET mt_fe_tn_set_freq_ts6011_hd2502(mt_fe_hd2502_priv_handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
//MT_FE_RET mt_fe_tn_set_freq_ts6011_hd2502(mt_fe_hd2502_priv_handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
void mt_fe_tn_ts6011_dump_reg(void);

#endif
