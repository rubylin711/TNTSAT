/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef __MT_FE_TN_TS6011_DEF__
#define __MT_FE_TN_TS6011_DEF__

#include "mt_fe_def.h"
#include "mt_fe_i2c_cs8800.h"


#define MT_FE_TS6011_DEBUG_PRNT                         0        /*    0 off, 1 on*/
#if (MT_FE_TS6011_DEBUG_PRNT == 1)
    #define mt_fe_print_ts6011(str)                printf str
#else
    #define mt_fe_print_ts6011(str)
#endif



#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x)        ((void)(x))
#endif

/*    VARIABLE TYPE DEFINES*/
#if 0   //TYPE DEFINES
#if 1
#define U8  unsigned char           /* 8bit unsigned     */
#define S8  signed char             /* 8bit unsigned     */
#define U16 unsigned short          /* 16bit unsigned    */
#define S16 signed short            /* 16bit unsigned    */
#define U32 unsigned int            /* 32bit unsigned    */
#define S32 signed int              /* 16bit unsigned    */
#define DB  double
#else
typedef unsigned char   U8;         /* 8bit unsigned    */
typedef unsigned char   S8;         /* 8bit unsigned    */
typedef unsigned short  U16;        /* 16bit unsigned   */
typedef signed short    S16;        /* 16bit unsigned   */
typedef unsigned int    U32;        /* 32bit unsigned   */
typedef signed int      S32;        /* 16bit unsigned   */
typedef double          DB;
#endif


#ifndef NULL
#define NULL    0
#endif

#ifndef BOOL
#define BOOL    int
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE    0
#endif
#endif //TYPE DEFINES

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
//extern void _mt_sleep(U32 ticks_ms);


#if MT_FE_DMD_DVBS_S2_SUPPORT
void mt_fe_tn_ts6011_init(MT_FE_Tuner_Handle_TS6011 handle);
S32 mt_fe_tn_ts6011_set_freq(MT_FE_Tuner_Handle_TS6011 handle, U32 Freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
S32 mt_fe_tn_ts6011_get_freq_offset(MT_FE_Tuner_Handle_TS6011 handle);
S32 mt_fe_tn_ts6011_get_gain(MT_FE_Tuner_Handle_TS6011 handle, U32 Vagc);
S32 mt_fe_tn_ts6011_get_signal_strength(MT_FE_Tuner_Handle_TS6011 handle, S32 Vagc, MT_BOOL bLocked);
void mt_fe_tn_ts6011_adjust_AGC(MT_FE_Tuner_Handle_TS6011 handle);
#endif

#endif

