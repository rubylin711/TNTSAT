/********************************************************************************************/
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef __CAS_FASTPLAY_H__
#define __CAS_FASTPLAY_H__
#include <string.h>

/*!
  Signed integer, weight is 8 bits. In most case it equal to char.
  */
typedef signed char s8_;

/*!
  Unsigned integer, weight is 8 bits. In most case it equal to unsigned char.
  */
typedef unsigned char u8_;

/*!
  Signed integer, weight is 16 bits. In most case it equal to short.
  */
typedef signed short s16_;

/*!
  Unsigned integer, weight is 16 bits. In most case it equal to unsigned short.
  */
typedef unsigned short u16_;

/*!
  Signed integer, weight is 32 bits. In most case it equal to long.
  */
typedef signed long s32_;

/*!
  Unsigned integer, weight is 32 bits. In most case it equal to unsigned long.
  */
typedef unsigned long u32_;


#define CAS_ALL_CHANNEL_NUMMAX  128
#define CAS_CHANNEL_NUMMAX  48
#define AUDIO_CHANNEL_NUMMAX 16
#define KEY_LEN_MAX  16

typedef struct 
{
  u16_ ecm_pid;
  u16_ ca_system_id;
  u16_ service_id;
  u16_ v_pid;
  u8_ a_chn_num;
  u8_ a_valid_index;
  u8_ ecm_type;   // 1 audoe vidoe same ecml ,  2 audio  ecm ,  3 video ecm
  u8_ v_type;
  u8_ a_type;
  u8_ is_used;
  u8_ index;
  u8_ reserve2;
  u16_ a_pid[AUDIO_CHANNEL_NUMMAX];
}cas_fastplay_ecm_t;


typedef struct 
{
  u16_ ecmpid;
  u16_ pg_id;
  u16_ v_id;
  u16_ a_id;
  u8_ oddkey[KEY_LEN_MAX];
  u8_ eventkey[KEY_LEN_MAX];
  u8_ keylen;
  u8_ is_used;
  u8_ ecm_type; // 1 all   2 audio   3 video
  u8_ reserve;
}cas_fastplay_key_t;

typedef struct
{
    u16_ curr_sid;
    u16_ pg_num;
    cas_fastplay_ecm_t channel_info[CAS_ALL_CHANNEL_NUMMAX];
}cas_fastplay_pginfo_t;

typedef s32_ (*casfast_callback_set_ecm)(cas_fastplay_ecm_t *pecm,u16_ num,u16_ cur_serviceid,u8_ set_flag);
typedef s32_ (*casfast_callback_set_cw)(u8_ *oddkey,u8_ *eventkey,u8_ key_len,u16_ v_pid,u16_ a_pid);
typedef s32_ (*casfast_callback_timer_create)(u32_ ntimerval, void(*p_timerproc)(u32_ funcontext), u32_  context,int  bcycleen);
typedef s32_ (*casfast_callback_timer_stop)(s32_ ntimerid);
typedef void (*casfast_callback_timer_start)(s32_ ntimerid);
typedef void (*casfast_callback_timer_reset)(s32_ ntimerid,u32_ ntimerval);
typedef int (*casfast_callback_printf)(const char *p_fmt, ...);
typedef void (*casfast_callback_change_channel)(u16_ service_id,u8_ id);
typedef u8_ (*casfast_callback_ca_is_valid)(u16_ ca_sys_id); // return 0 invalid  1 valid

typedef struct
{
    casfast_callback_set_ecm set_ecm_cal;  //set_flag : 1 set one channel , 2 set  other channel , 3 set all channel
    casfast_callback_set_cw set_cw_cal;
    casfast_callback_timer_create timer_create_cal;
    casfast_callback_timer_stop timer_stop_cal;
    casfast_callback_timer_start timer_start_cal;
    casfast_callback_timer_reset timer_reset_cal;
    casfast_callback_printf printf_cal;
    casfast_callback_change_channel change_channel;
    casfast_callback_ca_is_valid ca_is_valid_cal;
}cas_fastplay_callback;

cas_fastplay_callback * cas_fastplay_get_callback_handle();
s32_ cas_fastplay_init(u8_ is_addecm,u8_ ecm_max);
s32_ cas_fastplay_rebuild_all_channel(cas_fastplay_pginfo_t *p_pginfo);
void cas_fastplay_refreshcw_by_serviceid(u32_ ts_id,u16_ sevice_id);
void cas_fastplay_set_cur_channel(u32_ pg_id);
void cas_fastplay_get_cur_channel_ecm(cas_fastplay_ecm_t **channel_ecm,u8_ *ecm_num);
void cas_fastplay_updatecw_by_ecmpid(u16_       wEcmPID,
                          const u8_* pbyOddKey,  
                          const u8_* pbyEvenKey, 
                          u8_        byKeyLen);
void cas_fastplay_updatecw_by_avpid(u16_       avPID,
                          const u8_* pbyOddKey,  
                          const u8_* pbyEvenKey, 
                          u8_        byKeyLen);
void cas_fastplay_set_aud_channel(u16_ service_id,u16_ a_pid);

#endif
