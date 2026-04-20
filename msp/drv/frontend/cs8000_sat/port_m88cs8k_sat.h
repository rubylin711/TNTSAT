/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
//#include "mt_type.h"
#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_def_cs8000_sat.h"

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

    //MT_UNF_PIN_CONFIG_PARA_S cfg;
    mt_unf_fe_config_para_t cfg;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle;
    //i2c_bus_t *i2c_master;
    MT_FE_BS_TP_INFO bs_info;
    mt_unf_fe_channel_info_t cur_channel;
    u8 diseqc_tx_buf[8];
    mt_unf_fe_diseqc_cmd_t cur_diseqc;
    //mt_unf_fe_scan_info_t scan_info;
    //mt_unf_fe_blindscan_para_t scan_info;
    fe_blindscan_param_t scan_info;
    u32 status;
    //void *lnb_agent_handle;
    u32 print_level;
    mt_unf_fe_lnbctrl_type_t lnbctrl_dev; /**<LNB power supply and control device*/
    u16 lnb_dev_addr;                     /**<LNB control device address*/
    u16 lnb_i2c_id;
    mt_unf_fe_connect_para_t connect_para;
    mt_u32 lock_or_connect_time;
} mt_fe_cs8k_sat_priv_t, *mt_fe_cs8k_sat_priv_handle;

#if 0
typedef enum _PORT_CS8K_SAT_IOCTL_CMD_E
{
    NIM_IOCTRL_CHANNEL_CHECK_LOCK,
    NIM_IOCTRL_CHANGE_TN_MODE,
    NIM_IOCTRL_DISEQC1X,
    NIM_IOCTRL_DISEQC2X,
    NIM_IOCTRL_SET_PORLAR,
    NIM_IOCTRL_SET_LNB_ONOFF,
    NIM_IOCTRL_CHECK_LNB_SC_PROT,
    NIM_IOCTRL_LNB_SC_PROT_RESTORE,
    NIM_IOCTRL_REMOVE_PROTECT,
    NIM_IOCTRL_ENABLE_CHECK_PROTECT,
    NIM_IOCTRL_SET_22K_ONOFF,
    NIM_IOCTRL_GET_PORLAR,
    NIM_IOCTRL_GET_22K_ONOFF,
    NIM_IOCTRL_GET_TN_VERSION,
    NIM_IOCTRL_SCAN_CANCEL,
    NIM_IOCTRL_GET_SCAN_STATUS,
    NIM_IOCTRL_GET_SCAN_RESULT,
    NIM_IOCTRL_RECOVER,
    NIM_IOCTRL_SET_CHANNEL_INFO,
    NIM_IOCTRL_GET_SIGNAL_INFO,
    NIM_IOCTRL_GET_CHANNEL_INFO,
    NIM_IOCTRL_SET_DM_GPIO0_OUTPUT,
    NIM_IOCTRL_GET_DM_GPIO0_INPUT,
    NIM_IOCTRL_SET_DM_GPIO1_OUTPUT,
    NIM_IOCTRL_GET_DM_GPIO1_INPUT,
    NIM_IOCTRL_DISEQC_CTL
}PORT_CS8K_SAT_IOCTL_CMD;
#endif

extern mt_fe_cs8k_sat_priv_handle g_cs8k_sat_priv;

void port_m88cs8k_sat_notify_to_up_layer(MT_FE_MSG msg, void *p_param);
