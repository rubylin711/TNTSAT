/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  __DRV_KEYLED_IOCTL_H__
#define  __DRV_KEYLED_IOCTL_H__

#include "mt_type.h"
#include "mt_module.h"

typedef enum
{
	REC_,
	PLAY_,
	USB_,
	TSHIFT_,
	MOVIE_,
	MP3_,
	JPG_,
	ALL_,
	CYCLE_,
	SATTV_,
	RADIO_,
	STEREO_,
	AUDL_,
	AUDR_,
	Q,
	S,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
}led_special_ch_t;

typedef enum
{
	LED_LEVEL1_BRIGHT = 1,
	LED_LEVEL2_BRIGHT,
	LED_LEVEL3_BRIGHT,
	LED_LEVEL4_BRIGHT,
	LED_LEVEL5_BRIGHT,
	LED_LEVEL6_BRIGHT,
	LED_LEVEL7_BRIGHT,
	LED_LEVEL8_BRIGHT,
	LED_DISPLAY_CLOSE,
	LED_DISPLAY_OPEN,
}keyled_display_led_bright_t;

typedef enum
{
	LBD_TYPE_POWER = 0,
	LBD_TYPE_LOCK,
	LBD_TYPE_COLON,
	LBD_TYPE_SPOT,
}keyled_display_lbd_type_t;

#pragma pack(4)
typedef struct _keyled_keyval_s
{
    MT_U32 pressStatus;
    MT_U32 keyVal;
} keyled_keyval_s;

typedef struct _keyled_keyval_char_s
{
    MT_U32 len;
    MT_U8 ch[8];
} keyled_keyval_char_s;

typedef struct _keyled_display_char_s
{
    MT_U8 ch[8];
	MT_BOOL enable_power;
	MT_BOOL enable_lock;
	MT_BOOL enable_colon;
	MT_BOOL enable_plot;
} keyled_display_char_s;

typedef struct _keyled_display_lbd_s
{
	keyled_display_lbd_type_t   type;
	MT_BOOL   enable;		//1:display, 0:not display
	MT_U8	grid_pos;	//which led is to display.
	MT_U8	seg_pos;	//which segment of the led to display.
	MT_U8 reserved[2];
} keyled_display_lbd_s;

typedef struct _keyled_select_type_v2_s
{
	MT_U8 keyled_type; // keyled type
	MT_U8 kadc_type;  //kadc type, use it when keyled_type is KADC
	MT_U8 reserved[2];
}keyled_select_type_v2;

typedef struct _keyled_param_s
{
	MT_U8 buf[16];
	MT_U8 length;
	MT_U8 reserved[3];
} keyled_param_s;

/*!
LED special character
*/
typedef struct
{
	/*!
	ascii character to display
	*/
	u8 on_off;//0:display off, 1 display on
	u8 reserved1;
	u8 reserved2;
	u8 reserved3;
	/*!
	bitmap
	*/
	led_special_ch_t ch;
}led_special_dis_cfg_t;

typedef struct
{
	/*!
	ascii character to display
	*/
	u8 ch[16];

	/*!
	bitmap
	*/
	u8 len;

	u8 reserved[3];
}vfd_display_char;
#pragma pack()

#define KEYLED_IOC_BASE	'K'
#define KEYLED_IOC_SELECT_TYPE					_IOW(KEYLED_IOC_BASE, 0, MT_U8)
#define KEYLED_IOC_GET_VALUE					_IOWR(KEYLED_IOC_BASE, 1, keyled_keyval_s)
#define KEYLED_IOC_SET_REPKEY_TIMEOUT			_IOW(KEYLED_IOC_BASE, 2, MT_U32)
#define KEYLED_IOC_ENABLE_REPKEY				_IOW(KEYLED_IOC_BASE, 3, MT_U32)
#define KEYLED_IOC_ENABLE_KEYUP					_IOW(KEYLED_IOC_BASE, 4, MT_U32)
#define KEYLED_IOC_DISPLAY						_IOW(KEYLED_IOC_BASE, 5, MT_U32)
#define KEYLED_IOC_SET_KADC_TYPE				_IOW(KEYLED_IOC_BASE, 6, MT_U32)
#define KEYLED_IOC_DISPLAY_ASC					_IOW(KEYLED_IOC_BASE, 7, keyled_display_char_s)
#define KEYLED_IOC_DISPLAY_LBD					_IOW(KEYLED_IOC_BASE, 8, keyled_display_lbd_s)
#define KEYLED_IOC_SET_BRIGHT_LEVEL				_IOW(KEYLED_IOC_BASE, 9, int)

#define KEYLED_IOC_SELECT_TYPE_V2				_IOW(KEYLED_IOC_BASE, 10, keyled_select_type_v2)
#define KEYLED_IOC_HW_INIT						_IOW(KEYLED_IOC_BASE, 11, MT_U8)
#define KEYLED_IOC_SET_LED_POS					_IOW(KEYLED_IOC_BASE, 12, keyled_param_s)
#define KEYLED_IOC_SET_LED_MAP					_IOW(KEYLED_IOC_BASE, 13, keyled_param_s)
#define KEYLED_IOC_VFD_SPECIAL_CHAR_DISPLAY		_IOW(KEYLED_IOC_BASE, 14, led_special_dis_cfg_t)
#define KEYLED_IOC_VFD_DISPLAY					_IOW(KEYLED_IOC_BASE, 15, vfd_display_char)
#define KEYLED_IOC_VFD_RESET					_IOW(KEYLED_IOC_BASE, 16, MT_U8)

#endif  /*  __DRV_KEYLED_IOCTL_H__ */
