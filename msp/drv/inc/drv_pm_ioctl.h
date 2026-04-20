/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_PM_IOCTL_H__
#define __DRV_PM_IOCTL_H__

#include "mt_type.h"
#include "mt_module.h"
#include "mt_unf_pm.h"

/********************************* Ioctl definitions ************************/
/* 1:IN_LOW_POWER */
#define CMD_PM_IN_LOW_POWER				_IOW(MT_ID_PM, 0x0, uint)
#define CMD_PM_SET_FPDEV_TYPE			_IOW(MT_ID_PM, 0x1, sty_fp_type_t)
#define CMD_PM_SET_DISP_MODE			_IOW(MT_ID_PM, 0x2, sty_disp_conf_t)
#define CMD_PM_SET_WAKEUP_MODE			_IOW(MT_ID_PM, 0x3, sty_wakeup_conf_t)
#define CMD_PM_SET_PARAM				_IOW(MT_ID_PM, 0x4, sty_param_conf_t)
#define CMD_PM_GET_STANDBY_TIME			_IOWR(MT_ID_PM, 0x5, standby_time_t)
#define CMD_PM_GET_STANDBY_INFO			_IOWR(MT_ID_PM, 0x6, uint)
#define CMD_PM_GET_DISP_MODE			_IOWR(MT_ID_PM, 0x7, sty_disp_conf_t)
#define CMD_PM_SET_GPEN					_IOW(MT_ID_PM, 0x8, sty_gpen_pin_t)
#define CMD_PM_SWITCH_OSC_CLOCK			_IOW(MT_ID_PM, 0x9, uint)
#define CMD_PM_CEC_CONFIG				_IOW(MT_ID_PM, 0xa, cec_config_t)
#define CMD_PM_SET_UNIX_TIME			_IOW(MT_ID_PM, 0xb, uint)
#define CMD_PM_GET_UNIX_TIME			_IOW(MT_ID_PM, 0xc, uint)
#define CMD_PM_CLEAR_STANDBY_INFO		_IOWR(MT_ID_PM, 0xd, uint)
#define CMD_PM_SET_WAKEUP_CONFIG		_IOW(MT_ID_PM, 0xe, sty_wakeup_display_t)
#define CMD_PM_SET_BLE_WAKEUP			_IOW(MT_ID_PM, 0xf, sty_wakeup_ble_conf_t)
#define CMD_PM_KADC_CONFIG				_IOW(MT_ID_PM, 0x10, sty_kadc_keys_info_e)
#define CMD_PM_AO_FW_CONFIG				_IOW(MT_ID_PM, 0x11, sty_aomcu_fw_conf_t)

#endif /* __DRV_IR_IOCTL_H__ */
