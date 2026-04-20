/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __NIM_MN_SONY_CXD_H__
#define __NIM_MN_SONY_CXD_H__

#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"

#include "family_source/sony_demod.h"
#include "family_source/sony_integ.h"
#include "family_source/i2c/sony_i2c.h"
#include "family_source/tuner/sony_tuner.h"

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
#include "family_source/dvb_cable/sony_integ_dvbc.h"
#include "family_source/dvb_cable/sony_integ_dvbc2.h"
#include "family_source/dvb_terr/sony_integ_dvbt.h"
#include "family_source/dvb_terr/sony_integ_dvbt2.h"
#include "family_source/dvb_terr/sony_integ_dvbt_t2.h"
#endif

#ifdef SONY_DEMOD_SUPPORT_SAT
#include "family_source/dvb_sat/sony_integ_dvbs_s2.h"
#include "family_source/sat_device_ctrl/lnb_controller/sony_lnbc.h"
#include "family_source/dvb_sat/sony_demod_dvbs_s2.h"
#include "family_source/sat_device_ctrl/lnb_controller/sony_lnbc.h"
#include "family_source/sat_device_ctrl/sony_integ_sat_device_ctrl.h"
#include "family_source/sat_device_ctrl/lnb_controller/intersil_isl9492/intersil_isl9492.h"
#endif

#include "family_source/j83b/sony_demod_j83b.h"
#include "family_source/j83b/sony_integ_j83b.h"


#define MAX_SONY_CXD_DEVICES 2


typedef struct
{
    sony_demod_t demod; /**< Demod IC driver struct instance */
    sony_integ_t integ; /**< Integ struct instance */

    sony_integ_singlecable_data_t singleCableData; /**< Single Cable control data */

    sony_i2c_t i2c; /**< I2C struct instance */

	sony_tuner_t tuner;/**< tuner driver instance */
    //sony_tuner_terr_cable_t tunerTerrCable; /**< Terrestrial/Cable tuner driver instance */
    //sony_tuner_sat_t tunerSat;              /**< Satellite tuner driver instance */
    sony_lnbc_t lnbc;                       /**< LNB controller driver instance */

    //sony_ascot3_t ascot3; /**< Sony ASCOT3 tuner driver instance */
    //sony_helene_t helene; /**< Sony HELENE tuner driver instance */
} sony_example_driver_instance_t, *sony_example_driver_handle;

typedef struct
{
    //MT_UNF_PIN_CONFIG_PARA_S cfg;
    mt_unf_fe_config_para_t cfg;
    sony_example_driver_handle sony_cxd_handle;
    //i2c_bus_t *i2c_master;
    //mt_unf_fe_channel_info_t cur_channel;
    mt_unf_fe_connect_para_t param;
    u8 diseqc_tx_buf[8];
    mt_unf_fe_diseqc_cmd_t cur_diseqc;
    //mt_unf_fe_scan_info_t scan_info;
    //mt_unf_fe_blindscan_para_t scan_info;
    fe_blindscan_param_t scan_info;
    u32 status;
    //void *lnb_agent_handle;
    u32 print_level;
    u32 sig_type;
    u8 fe_index;
} mt_fe_sony_cxd_priv_t, *mt_fe_sony_cxd_priv_handle;

extern mt_fe_sony_cxd_priv_handle g_sony_cxd_priv[MAX_SONY_CXD_DEVICES];

//void port_sony_cxd2856_notify_to_up_layer(MT_FE_MSG msg, void *p_param);

typedef enum _NIM_FE_TUNER_SONY_CXD
{
    NIM_SONY_CXD_TUNER_UNDEF = 0,
    NIM_SONY_CXD_TUNER_MxL603,
    NIM_SONY_CXD_TUNER_TC3800,
    NIM_SONY_CXD_TUNER_TS2022,
    NIM_SONY_CXD_TUNER_RDA5815S,
    NIM_SONY_CXD_TUNER_R836,
    NIM_SONY_CXD_TUNER_R848,
    NIM_SONY_CXD_TUNER_TC6800,
    NIM_SONY_CXD_TUNER_TS6011,
} NIM_FE_TUNER_SONY_CXD;

#endif

