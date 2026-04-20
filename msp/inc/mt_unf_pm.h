#ifndef __MT_UNF_PM_H__
#define __MT_UNF_PM_H__

#include "mt_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


typedef struct standby_time
{
	MT_U8 cur_hour;
	MT_U8 cur_min;
	MT_U8 cur_sec;
	MT_U8 pass_day;
}standby_time_t;

typedef struct wakeup_time
{
	MT_U8 wakeup_hour;
	MT_U8 wakeup_min;
	MT_U8 wakeup_sec;
	MT_U8 pass_day;
}wakeup_time_t;

typedef struct wakeup_key
{
	MT_U8 fp_wkey;
	MT_U8 irda_wkey0;
	MT_U8 irda_wkey1;
	MT_U8 irda_wkey2;
	MT_U8 irda_wkey3;
}wakeup_key_t;

typedef enum sty_fp_type{
	FD650		= 0,
	CT1642		= 1,
	AOGPIO		= 2,
	NOFP		= 3,
	OSC			= 4,
	TT1629		= 5,
	TT1629B		= 6,
	STRPOWER	= 7,
	PT6393		= 8,
	FD650_KADC	= 9,
	//STRPOWERON = 7,
	//STRPOWEROFF= 8,
	FP_TYPE_MAX,
}sty_fp_type_t;

typedef enum sty_disp_mode{
	LED_DIS_NO		= 0,
	LED_DIS_CHAR	= 1,
	LED_DIS_TIME	= 2,
	DISP_MODE_MAX,
}sty_disp_mode_t;

typedef struct sty_disp_conf
{
	sty_disp_mode_t disp_mode;
	standby_time_t disp_time;
	MT_U32 disp_char;
}sty_disp_conf_t;

typedef enum sty_wakeup_mode{
	TIME_WAKE_UP    = 1,
	WAKE_MODE_MAX,
}sty_wakeup_mode_t;

typedef struct sty_wakeup_conf
{
	sty_wakeup_mode_t w_mode;
	wakeup_key_t w_key;
	wakeup_time_t w_time;
}sty_wakeup_conf_t;

typedef struct sty_wakeup_display
{
	MT_U8 lock_led;//Wake up and start fighting lock
	MT_U8 fp_reverse;//fp reverse display
	/*
	* config which to display when wake up
	* 0:normal(display time), 1:turn off display, 2:display 'ON', 3:display 'boot', default is 0
	*/
	MT_U8 display_config;
}sty_wakeup_display_t;


typedef struct sty_led_bright{
	MT_BOOL en_led_bright;
	MT_U8 bri_val;
}sty_led_bright_t;


typedef struct sty_led_info{
	MT_BOOL	config_led_en;
	MT_U8	led_pos[4];
	MT_BOOL	config_colon_en;
	MT_U8	colon_pos;
	MT_BOOL	config_lock_en;
	MT_U8	lock_pos;
	MT_BOOL	config_power_en;
	MT_U8	power_pos;
}sty_led_info_t;


typedef struct sty_param_conf
{
	MT_U8 map[8];
	sty_led_bright_t  led_bri;
	sty_led_info_t    led_pos;
}sty_param_conf_t;


typedef enum sty_gpen_pin{
	GPEN_LOW   = 0,
	GPEN_HIGH  = 1,
	GPEN_VAL_MAX,
}sty_gpen_pin_t;

typedef struct cec_config{
	MT_U8 cec_enable;
	MT_U8 cec_aogpio_pin;

}cec_config_t;

typedef struct sty_wakeup_ble_conf
{
	MT_U8 aogpio;//aogpio number
	MT_U8 level;//0:high to low wakeup;1:low to high wakeup
	MT_U8 ble_enable;
}sty_wakeup_ble_conf_t;

typedef enum
{
	KADC_KEYS_NO_3 = 0,
	KADC_KEYS_NO_5,
	KADC_KEYS_NO_7,
	KADC_KEYS_DISABLE,
}sty_kadc_keys_info_e;

/*
	kadc config
*/
typedef struct standby_kadc_config
{
   u8 enable_cfg;
   sty_kadc_keys_info_e kadc_keys;
   /*
    disable kadc
   */
  u8 disable_kadc;
  /*!
    config kadc hw version(only for sym2)
    0:new version(default)
    1:old version
   */
  u8 hw_version;
}standby_kadc_config_t;


typedef struct sty_aomcu_fw_conf
{
	MT_U8	fw_extern;//use extern aomcu fw
	MT_U32	fw_size;
	MT_U8	*fw_info;
}sty_aomcu_fw_conf_t;


MT_S32 MT_UNF_PMOC_SwitchSystemMode(void);

MT_S32 MT_UNF_PMOC_SetStandbyDispMode(sty_disp_conf_t dconf);

MT_S32 MT_UNF_PMOC_SetWakeUpAttr(sty_wakeup_conf_t  wconf);

MT_S32 MT_UNF_PMOC_SetDevType( sty_fp_type_t dev_type);

MT_S32 MT_UNF_PMOC_ConfigParams(sty_param_conf_t config);

MT_S32 MT_UNF_PMOC_GetStandbyTime(standby_time_t *ptime);

MT_S32 MT_UNF_PMOC_GetStandbyInfo(MT_U32 *pinfo);

MT_S32 MT_UNF_PMOC_GetDispMode(sty_disp_conf_t *pdconf);

MT_S32 MT_UNF_PMOC_Init(void);

MT_S32 MT_UNF_PMOC_DeInit(void);

MT_S32 MT_UNF_PMOC_SetGpenPin( sty_gpen_pin_t  val);

MT_S32 MT_UNF_PMOC_SwitchOSCClock(void);

MT_S32 MT_UNF_PMOC_SetCecConfig(cec_config_t  cec_cfg);

MT_S32 MT_UNF_PMOC_SetUnixTime(MT_U32  time);

MT_S32 MT_UNF_PMOC_GetUnixTime(MT_U32  *time);

MT_S32 MT_UNF_PMOC_ClearStandbyInfo(void);

MT_S32 MT_UNF_PMOC_SetWakeUpConfig(sty_wakeup_display_t  wconf);

MT_S32 MT_UNF_PMOC_SetBleWakeup(sty_wakeup_ble_conf_t ble_cfg);
MT_S32 MT_UNF_PMOC_KadcConfig(standby_kadc_config_t kadc_cfg);
MT_S32 MT_UNF_PMOC_ExternAomcuConfig(sty_aomcu_fw_conf_t aomcu_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
