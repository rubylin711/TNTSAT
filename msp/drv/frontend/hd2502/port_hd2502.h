/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifndef __NIM_MN_HD2502_H__
#define __NIM_MN_HD2502_H__

#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"
#include "mt_fe_tn_montage_ts6011.h"
#include "HDIC2501.h"

#define MT_FE_CFG_CUSTOMER_SELECT			0	// 0: Public	1: Vestel

//#define HD2502_DEBUG
#ifdef HD2502_DEBUG
#define HD2502_DEBUG_PRINTF  printk
#else
#define HD2502_DEBUG_PRINTF(...)   do{}while(0) 
#endif
typedef struct
{
    u8 bs_stop : 1;
    u8 for_scan : 1;
    u8 diseqc_2x : 1;
    u8 onoff_22k : 1;
    u8 lnb_onoff : 1;
    u8 lnb_voltage : 2;
    u8 reserved : 1;
    u8 lnb_polar;
    s16 cur_tun_offset_khz;

    u32 tuner_handle;

    //MT_UNF_PIN_CONFIG_PARA_S cfg;
    mt_unf_fe_config_para_t cfg;
    //MT_FE_HD2502_Device_Handle hd2502_handle;
    //i2c_bus_t *i2c_master;
    mt_unf_fe_channel_info_t cur_channel;
    u8 diseqc_tx_buf[8];
    mt_unf_fe_diseqc_cmd_t cur_diseqc;
    //mt_unf_fe_scan_info_t scan_info;
    //mt_unf_fe_blindscan_para_t scan_info;
    fe_blindscan_param_t scan_info;
    u32 status;
    //void *lnb_agent_handle;
    u32 print_level;
    u32 sig_type;
	u8 lnb_onoff_gpio;
} mt_fe_hd2502_priv_t, *mt_fe_hd2502_priv_handle;


typedef enum _MT_FE_RET
{
	MtFeErr_Ok					 = 0,
	MtFeErr_Undef				 = -1,
	MtFeErr_Uninit				 = -2,
	MtFeErr_Param				 = -3,
	MtFeErr_NoSupportFunc		 = -4,
	MtFeErr_NoSupportTuner		 = -5,
	MtFeErr_NoSupportDemod		 = -6,
	MtFeErr_UnLock				 = -7,
	MtFeErr_I2cErr				 = -8,
	MtFeErr_DiseqcBusy			 = -9,
	MtFeErr_NoMemory			 = -10,
	MtFeErr_NullPointer			 = -11,
	MtFeErr_TimeOut				 = -12,
	MtFeErr_Fail				 = -13,
	MtFeErr_NoMatch				 = -14,
	MtFeErr_FirmwareErr			 = -15
} MT_FE_RET;


typedef enum _MT_FE_LOCK_STATE
{
	MtFeLockState_Undef = 0,
	MtFeLockState_Unlocked,
	MtFeLockState_Locked,
	MtFeLockState_Waiting
} MT_FE_LOCK_STATE;

extern mt_fe_hd2502_priv_handle g_hd2502_priv;
void _mt_sleep_hd2502(U32 ms);
UINT8 Write_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);
UINT8 Read_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);
#endif

