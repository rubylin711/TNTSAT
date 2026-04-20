/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_IR_IOCTL_H__
#define __DRV_IR_IOCTL_H__

#include "mt_type.h"
#include "mt_unf_ir.h"

#if 0
typedef enum decode_mode
{
    DECODE_MODE_HW  = 0,
    DECODE_MODE_SOFT,
    DECODE_MODE_PANA,
    DECODE_MODE_UNKOWN
} decode_mode_e;
#endif

typedef enum sample_freq
{
    SAMPLE_FREQ_3MHz = 0,
    SAMPLE_FREQ_750KHz,
    SAMPLE_FREQ_187_5KHz,
    SAMPLE_FREQ_46_875KHz,
    SAMPLE_FREQ_UNKOWN
} sample_freq_e;


typedef struct ir_config
{
    irda_protocol_t protocol;// setup protocol for initial firstly
    MT_U8 ucfilter_en; //!<用户码过滤使能，0=不使能，1=使能，仅适用硬件解码模式
    MT_U16 usercode; //!<指定用户码，ucfilter_en=1时有效
    MT_U8 kcfilter_en; //!<键码过滤使能，0=不使能，1=使能，仅适用硬件解码模式
    MT_U16 keycode; //!<指定键码,kcfilter_en=1时有效
    /*!
        重复键码采样间隔，有效值0~15，每隔repkey_interval个简码采样一次
        仅适用硬件解码模式
    */
    MT_U8 repkey_interval;
    sample_freq_e sample_freq; //!<红外采样频率，硬件模式必须为SAMPLE_FREQ_187_5KHz
    /*!
        红外没有输入时，输入PIN脚的电平，0=低电平，1=高电平
        仅适用软件解码模式
    */
    MT_U8 idle_level;
    /*!
        中断间隔，采样到多少个有效周期触发一次中断
        仅适用软件解码模式
    */
    MT_U8 int_interval;
    /*!
        采样超时，超时值=(sample_timeout * 16) / sample_freq
        仅适用软件解码模式
    */
    MT_U8 sample_timeout;
    /*!
        是否使能按键抬起
        仅适用硬件解码模式
    */
    MT_U8 is_keyup;
    /*!
        是否使能重复键
        仅适用硬件解码模式
    */
    MT_U8 is_repeat;
} ir_config_s;



/********************************* Ioctl definitions ************/
/* 1:check keyup */
#define CMD_IR_ENABLE_KEYUP _IOW(MT_ID_IR, 0x1, uint)

/* 1:check repkey, 0:hardware behave */
#define CMD_IR_ENABLE_REPKEY _IOW(MT_ID_IR, 0x2, uint)
#define CMD_IR_SET_REPKEY_TIMEOUT _IOW(MT_ID_IR, 0x3, uint)

/* 1:enable ir, 0:disable ir */
//#define CMD_IR_SET_ENABLE _IOW(MT_ID_IR, 0x4, uint)
//#define CMD_IR_RESET _IO(MT_ID_IR, 0x5)
#define CMD_IR_SET_HW_USERCODE _IOW(MT_ID_IR, 0x4, uint)
#define CMD_IR_SET_HW_KEYCODE _IOW(MT_ID_IR, 0x5,uint)
#define CMD_IR_SET_BLOCKTIME _IOW(MT_ID_IR, 0x6, uint)
#define CMD_IR_SET_FORMAT _IOW(MT_ID_IR, 0x7, uint)
#define CMD_IR_SET_BUF _IOW(MT_ID_IR, 0x8, uint)

/* raw symbol fetch(1) or key fetch(0) */
#define CMD_IR_SET_FETCH_METHOD _IOW(MT_ID_IR, 0x9, uint)

/* enable or disalbe a protocol */
#define CMD_IR_SET_PROT_ENABLE _IOW(MT_ID_IR, 0xa, uint)
#define CMD_IR_SET_PROT_DISABLE _IOW(MT_ID_IR, 0xb, uint)
#define CMD_IR_GET_PROT_ENABLED _IOWR(MT_ID_IR, 0xc, uint)

#define CMD_IR_SET_CONFIG  _IOWR(MT_ID_IR, 0xd, struct ir_config)
#define CMD_IR_SET_WAVE_FILTER _IOWR(MT_ID_IR, 0xe, struct ir_wavefilter_config)
#define CMD_IR_CLEAR_KEY _IOWR(MT_ID_IR, 0xf, uint)
#define CMD_IR_GET_PLUSE_DATA _IOWR(MT_ID_IR, 0x10, ir_pluse_data_s)

#endif /* __DRV_IR_IOCTL_H__ */
