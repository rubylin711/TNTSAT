/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@~english
@file mt_unf_keyled.h
@brief the header file of frontpanel
*/
#ifndef __MT_UNF_KEYLED_H__
#define __MT_UNF_KEYLED_H__

#include "mt_common.h"

#ifdef __cplusplus
	#if __cplusplus
extern "C" {
	#endif
#endif /* __cplusplus */

/*!
@brief Error code
*/
typedef enum mtUNF_KEYLED_ERROR_E
{
	/*!
	open frontpanel failed
	*/
	MT_UNF_KEYLED_ERR_OPEN_FAIL					= -0x100,
	/*!
	frontpanel is not supported
	*/
	MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED			= -0x101,
	/*!
	No key is pressed
	*/
	MT_UNF_KEYLED_ERR_NO_KEYVAL					= -0x102,
	/*!
	Device is not opened
	*/
	MT_UNF_KEYLED_ERR_DEV_NOT_OPENED			= -0x103,
} MT_UNF_KEYLED_ERROR_E;


/*!
@brief frontpanel type
*/
typedef enum
{
	MT_UNF_KEYLED_TYPE_FD650 = 1,
	MT_UNF_KEYLED_TYPE_CT1642,
	MT_UNF_KEYLED_TYPE_TT1629B,
	MT_UNF_KEYLED_TYPE_PT6393,
	MT_UNF_KEYLED_TYPE_FD612,
	MT_UNF_KEYLED_TYPE_KEYADC = 0x10,	//(MT_UNF_KEYLED_TYPE_FD650|MT_UNF_KEYLED_TYPE_KEYADC) == 'fd650+kadc'
	MT_UNF_KEYLED_TYPE_MAX
}MT_UNF_KEYLED_TYPE_E;

/*!
@brief key status
*/
#ifndef DEFINED_MT_UNF_KEY_STATUS_E // Redefine in mt_unf_ir.h
#define DEFINED_MT_UNF_KEY_STATUS_E
typedef enum
{
	MT_UNF_KEY_STATUS_DOWN = 0,
	MT_UNF_KEY_STATUS_HOLD,
	MT_UNF_KEY_STATUS_UP,
	MT_UNF_KEY_STATUS_BUTT
}MT_UNF_KEY_STATUS_E ;
#endif

typedef enum
{
	MT_UNF_LBD_TYPE_POWER = 0,
	MT_UNF_LBD_TYPE_LOCK,
	MT_UNF_LBD_TYPE_COLON,
	MT_UNF_LBD_TYPE_SPOT,
}MT_UNF_LBD_TYPE_T;


typedef enum
{
	MT_UNF_LED_LEVEL1_BRIGHT = 1,
	MT_UNF_LED_LEVEL2_BRIGHT,
	MT_UNF_LED_LEVEL3_BRIGHT,
	MT_UNF_LED_LEVEL4_BRIGHT,
	MT_UNF_LED_LEVEL5_BRIGHT,
	MT_UNF_LED_LEVEL6_BRIGHT,
	MT_UNF_LED_LEVEL7_BRIGHT,
	MT_UNF_LED_LEVEL8_BRIGHT,
	MT_UNF_LED_DISPLAY_CLOSE,
	MT_UNF_LED_DISPLAY_OPEN,
}MT_UNF_DISPLAY_LED_BRIGHT;

typedef struct _MT_UNF_DIS_PLAY_LBD_E
{
	MT_UNF_LBD_TYPE_T type;
	MT_BOOL enable;		//1:display, 0:not display
	MT_U8	grid_pos;	//which led is to display(0,1,2...).
	MT_U8	seg_pos;	//which segment of the led to display(0,1,2...)
}MT_UNF_DIS_PLAY_LBD_E;

typedef enum
{
	REC,
	PLAY,
	USB,
	TSHIFT,
	MOVIE,
	MP3,
	JPG,
	ALL,
	CYCLE,
	SATTV,
	RADIO,
	STEREO,
	AUDL,
	AUDR,
	OTHER_Q,
	OTHER_S,
	OTHER_R1,
	OTHER_R2,
	OTHER_R3,
	OTHER_R4,
	OTHER_R5,
	OTHER_R6,
	OTHER_R7,
	OTHER_R8,
	OTHER_R9,
	OTHER_R10,
	EMAIL_,
	POWER_,
	TWODOT_,
	MUSIC_,
	WAVE_,
	SIGNAL_LOCK_,
	SIGNAL_UNLOCK_,
}MT_UNF_VFD_SPECIAL_CHAR;

typedef struct _MT_UNF_VFD_DIS_SPECIAL_CHAR
{
	/*!
	ascii character to display
	*/
	MT_U8 on_off;//0:display off, 1 display on

	/*!
	bitmap
	*/
	MT_UNF_VFD_SPECIAL_CHAR ch;
}MT_UNF_VFD_DIS_SPECIAL_CHAR;

typedef struct _MT_UNF_VFD_DIS_CHAR
{
	/*!
	ascii character to display
	*/
	u8 ch[11];

	/*!
	bitmap
	*/
	u8 len;
}MT_UNF_VFD_DIS_CHAR;


/*!
@brief LED show time
*/
typedef struct
{
    MT_U32 u32Hour;
    MT_U32 u32Minute;
}MT_UNF_KEYLED_TIME_S, *MT_UNF_KEYLED_TIME_S_PTR;

/*!
 kadc type:symphony chip only support: 3 keys,5 keys,7 keys
*/
 typedef enum
 {
	/*!
	3 key
	*/
	KADC_KEY_TYPE_3 = 0,
	/*!
	5 key
	*/
	KADC_KEY_TYPE_5,
	/*!
	7 key
	*/
	KADC_KEY_TYPE_7,
	/*!
	key max
	*/
	KADC_KEY_TYPE_MAX,

 }MT_UNF_KEYLED_KADC_TYPR_E;

 typedef struct
 {
	MT_UNF_KEYLED_TYPE_E 		keyled_type; // keyled type
	MT_UNF_KEYLED_KADC_TYPR_E 	kadc_type;	//kadc type, use it when keyled_type is MT_UNF_KEYLED_TYPE_KEYADC
 }MT_UNF_KEYLED_TYPE_V2_E;


/*!
@brief GPIO panel configure
*/
typedef struct
{
	/*!
	gpio list for key scanning
	*/
	MT_U8 key_gpio[16];
	/*!
	gpio list for led control
	*/
	MT_U8 led_gpio[16];
	/*!
	Actual use of key gpio number
	*/
	MT_U8 key_gpio_num;
	/*!
	Actual use of led gpio number
	*/
	MT_U8 led_gpio_num;
	/*!
	gpio level when key is pressed
	*/
	MT_U8 key_gpio_active_level;
	/*!
	gpio level when LED lights up.
	*/
	MT_U8 led_gpio_active_level;
}MT_UNF_GPIOKB_CONFIG_S, *MT_UNF_GPIOKB_CONFIG_S_PTR;

/*!
@brief  Initialize the front panel module
@retval :: MT_SUCCESS/MT_FAILURE  success/failed.
*/
MT_S32 MT_UNF_KEYLED_Init(MT_VOID);

/*!
@~chinese
@brief Deinitialize the front panel module
@retval :: MT_SUCCESS/MT_FAILURE  success/failed.
*/
MT_S32 MT_UNF_KEYLED_DeInit(MT_VOID);

/*!
@brief set frontpanel type
@param [in] enKeyLedType frontpanel type
Value								| Description
------------------------------------|-----------------------
MT_UNF_KEYLED_TYPE_CT1642			| CT1642
MT_UNF_KEYLED_TYPE_FD650			| FD650

@Return								| Description
------------------------------------|--------------
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
MT_SUCCESS							| Success
*/
MT_S32 MT_UNF_KEYLED_SelectType(MT_UNF_KEYLED_TYPE_E enKeyLedType);

/*!
@~english

@brief set frontpanel type
@param [in] KeyLedType.keyled_type
Value
-------------------------------
MT_UNF_KEYLED_TYPE_FD650
MT_UNF_KEYLED_TYPE_CT1642
MT_UNF_KEYLED_TYPE_KEYADC

@param [in] KeyLedType.kadc_type,
if the keyled_type is MT_UNF_KEYLED_TYPE_KEYADC, should set this param
Value
-------------------------------
KADC_KEY_TYPE_3
KADC_KEY_TYPE_5
KADC_KEY_TYPE_7
@Return								| Description
------------------------------------|--------------
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
MT_SUCCESS							| Success
*/
MT_S32 MT_UNF_KEYLED_SelectType_V2(MT_UNF_KEYLED_TYPE_V2_E KeyLedType);

/*!
@brief Open frontpanel device
@Return								| Description
------------------------------------|--------------
MT_SUCCESS							| Open success
MT_UNF_KEYLED_ERR_OPEN_FAIL			| Open failed
*/
MT_S32 MT_UNF_LED_Open(MT_VOID);

/*!
@brief Close frontpanel device
@Return								| Description
------------------------------------|--------------
MT_SUCCESS							| Close success
MT_FAILURE							| Close failed
*/
MT_S32 MT_UNF_LED_Close(MT_VOID);

/*!
@brief Display LED
@details
	The following figure is the digital tube corresponding to the bit bit ,For example display "1111",\n
	u32CodeValue is 0x06060606.bit7 represents the dot of the digital tuben.
@verbatim
  ----0----
  |        |
  5         1
  |        |
  ----6----
  |        |
  4         2
  |        |
  ----3--- .7
@endverbatim
@param [in] u32CodeValue  Value is an integer
@Return								| Desciption
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
*/
MT_S32 MT_UNF_LED_Display(MT_U32 u32CodeValue);

/*!
@brief Display LED
@param [in] p_data  Value is a string
@Return								| Desciption
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
*/
MT_S32 MT_UNF_LED_Display_Asc(MT_U8 *p_data);

/*!
@~chinese

@brief:Control the on/off of any LED segment in a 4-bit 8-segment digital tube
	(Usually used to control the on/off of power, lock, and col LEDs)
@Return								| Desciption
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
*/
MT_S32 MT_UNF_LED_Display_LBD(MT_UNF_DIS_PLAY_LBD_E *p_data);

/*!
@brief Display time
@details Display time
@param [in] stLedTime Time
@Return								| Description
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
*/
MT_S32 MT_UNF_LED_DisplayTime(MT_UNF_KEYLED_TIME_S stLedTime);

/*!
@brief Get key value
@param [out] pu32PressStatus key status
@Return								| Description
------------------------------------|--------------
MT_UNF_KEY_STATUS_DOWN				| Down
MT_UNF_KEY_STATUS_HOLD				| Hold
MT_UNF_KEY_STATUS_UP				| Up

@param [out] pu32KeyId Key value
@Return								| Description
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_NO_KEYVAL			| No key
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel is not supported
*/
MT_S32 MT_UNF_KEY_GetValue(MT_U32 *pu32PressStatus, MT_U32 *pu32KeyId);

/*!
@brief  Set the sampling time for repeat(hold) keys
@param [in] u32RepTimeMs timeout,unit: ms
@Return								| Description
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED 	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
*/
MT_S32 MT_UNF_KEY_RepKeyTimeoutVal(MT_U32 u32RepTimeMs);

/*!
@brief  Config whether support duplicate(hold) keys
@param [in] u32IsRepKey whether support duplicate keys
@Value								| Description
------------------------------------|--------------
0									| Disable
1									| Enable
@Return								| Description
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
*/
MT_S32 MT_UNF_KEY_IsRepKey(MT_U32 u32IsRepKey);

/*!
@brief Config whether support reporting key up events.
@param [in] u32IsRepKey whether support reporting key up events
@Value								| Description
------------------------------------|--------------
0									| Disable
1									| Enable
@Return								| Description
------------------------------------|--------------
MT_SUCCESS							| Success
MT_UNF_KEYLED_ERR_DEV_NOT_OPENED	| Device is not opened
MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED	| Frontpanel type is not supported
*/
MT_S32 MT_UNF_KEY_IsKeyUp(MT_U32 u32IsKeyUp);

/*!
@brief Config how many kadc keys.
@attention:
	Setup kadc type as MT_UNF_KEYLED_KADC_TYPR_E.
	The error code MT_SUCCESS is returned if this API is called repeatedly.
@retval ::MT_SUCCESS Success
@retval ::MT_ERR_KEYLED_NOT_INIT  The KEYLED device is not initialized.
@retval ::MT_FAILURE  Calling Ioctrl fails.
*/
MT_S32 MT_UNF_KEY_setKeyadcType(MT_UNF_KEYLED_KADC_TYPR_E u8KadcType);

/*
@brief Configure LED brightness level
@attention:
	Currently only supported tt1629b and fd650
	Config led display bright from 1 to 8 ,8 is  Brightest
@param MT_UNF_LED_DISPLAY_CLOSE can disable display
@param MT_UNF_LED_DISPLAY_OPEN can enable display
*/
MT_S32 MT_UNF_LED_Display_Bright_Level(MT_UNF_DISPLAY_LED_BRIGHT level);

/*
@brief Config led display position according to hardware
*/
MT_S32 MT_UNF_KEYLED_SetLedPos(MT_U8 *p_buf, MT_U8 len);

/*
@brief Config led map according to hardware
*/
MT_S32 MT_UNF_KEYLED_SetLedMap(MT_U8 *p_buf, MT_U8 len);

/*
@brief Initialize hardware
*/
MT_S32 MT_UNF_KEYLED_Hw_Init(MT_VOID);

/*
@brief Control vfd(pt6393) special char
@attention:
	This fun is only for pt6393 vfd
*/
MT_S32 MT_UNF_LED_Vfd_Display_Special_char(MT_UNF_VFD_DIS_SPECIAL_CHAR *s_data);

/*
@brief Vfd(pt6393) display string
@attention:
	This fun is only for pt6393 vfd
*/
MT_S32 MT_UNF_LED_Vfd_Display(MT_U8 *p_buf, MT_U8 len);

#ifdef CONFIG_MT_FPGA
MT_S32 MT_UNF_KEYLED_Reset(MT_U8 reset);
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_UNF_KEYLED_H__ */
