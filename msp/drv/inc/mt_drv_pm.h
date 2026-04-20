/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_PM_H__
#define __MT_DRV_IR_H__

#include "mt_type.h"
#include "mt_common.h"
#include "mt_debug.h"

#define MAX_MT_STANDBY_BLOCK_NAME	(32)

/*!
  data register between AP and standby cpu
  */
typedef struct trans_info_standby
{
	/*!
	raw map for led display, byte0-byte3
	*/
	u8 bitmap[4];

	/*!
	current time hour,byte4
	*/
	u8 cur_hour;

	/*!
	current time min,byte5
	*/
	u8 cur_min;

	/*!
	current time sec,byte6
	*/
	u8 cur_sec;

	/*!
	wake up hour,byte7
	*/
	u8 wake_hour;

	/*!
	wake up min,byte8
	*/
	u8 wake_min;

	/*!
	wake up sec,byte9
	*/
	u8 wake_sec;

	/*!
	wake up day,byte10
	*/
	u8 wake_day;

	/*!
	wake up key,byte11
	*/
	u8 wake_up_key;

	/*!
	byte12
	bit 7: time_wake; bit 6:time_disp; bit5 char_disp;bit4 repeat;bit[3:1] led bright; bit[0]gpen val
	*/
	u8 standby_config_info;

	/*!
	byte13
	bit0,1:com0, bit2,3:com1, bit4,5:com2, bit6,7:com3,
	standby_config_info2: bit1, config led_pos?
	*/
	u8 led_pos;

	/*!
	byte14
	bit 0:bri_adj, bit6,7 fixed colon pos(0-3), bit5 colon_config??
	bit 4,3 fixed lock pos!, bit2 lock config?? bit1, config led_pos?
	*/
	u8 standby_config_info2;

	/*!
	byte15
	cec wake up config, bit[0]:enable cec, bit[3:1]:aogpio pin number
	*/
	u8 cec_config;

	/*!
	byte16
	bit[6] kadc hw version(0->new,1->old)
	bit[5] is for return time unix_time_sec or hour/min/sec format
	bit[4]store osc cal increase or decrease flag;
	bit[3]diff osc or not for symphony2 chip;
	bit[2:1] store kadc type uio_kadc_type_t;bit[0]diff symphony chip version
	*/
	u8 standby_param;

	/*!
	byte17
	bit[4:7] store osc cal sec;
	bit[3:0] store osc cal min;
	*/
	u8 osc_cal_param;
	/* 18
	for fd650 led display if fd650 wake up want to Turn on the green light
	bit 0: Turn on the green light after you wake up stb
	bit 1: Flip display(reverse display)
	bit 2: whether config the position of power(colon's second point),1->config, 0->not
	bit4,3:the position of power(colon's second point(0-3))
	bit6,5: config which to display when wake up
	0:normal(same as standby), 1:turn off display, 2:display 'ON', 3:display 'boot', default is 0
	*/
	u8 fd650_display_config;

	/*! 19
	bit[3:0] select fp_stb pin(0:aogpio0,1:aogpio1...)
	*/
	u8 standby_config_info3;

	/*
	byte20-byte23
	only for need unix time
	*/
	u32 unix_time_sec;
}trans_info_standby_t;

/*!
  aomcu fw config
  */
typedef struct aomcu_fw_info {
	/*!
	use extern aomcu ROM
	*/
	mt_u8	extern_fw;
	mt_u32	fw_size;
	mt_u8	*p_fw;
}aomcu_fw_info_t;


struct mt_standby_component {
	char name[MAX_MT_STANDBY_BLOCK_NAME+1];
	int (*enter_standby)(void*);
	void *data;
};


struct mt_standby_node {
	struct mt_standby_component component;
    struct list_head list;
};

//FIXME: for symphony1/2/4, how about Aria?
#define AOMCU_RAM_ADDR       (SYMPHONY_IO_VA(0xBF160000))
#define LPM_MESSAGE2AO_0     (SYMPHONY_IO_VA(0xBF150040))
#define LPM_MESSAGE2AO_1     (SYMPHONY_IO_VA(0xBF150044))
#define LPM_MESSAGE2AO_2     (SYMPHONY_IO_VA(0xBF150048))
#define LPM_MESSAGE2AO_3     (SYMPHONY_IO_VA(0xBF15004C))
#define LPM_AOMCU_EN         (SYMPHONY_IO_VA(0xBF150050))
#define LPM_AOMCU_MESSAGE  	 (SYMPHONY_IO_VA(0xBF150024))


#endif
