/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_FRONTEND_H__
#define __MT_DRV_FRONTEND_H__

#include "mt_type.h"
//#include "mt_drv_i2c.h"
#include "mt_debug.h"
#include "mt_drv_struct.h"
//#include "drv_i2c_ext.h"
#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"

#define MT_FATAL_FE(fmt...) \
    MT_FATAL_PRINT(MT_ID_FRONTEND, fmt)

#define MT_ERR_FE(fmt...) \
    MT_ERR_PRINT(MT_ID_FRONTEND, fmt)

#define MT_WARN_FE(fmt...) \
    MT_WARN_PRINT(MT_ID_FRONTEND, fmt)

#define MT_INFO_FE(fmt...) \
    MT_INFO_PRINT(MT_ID_FRONTEND, fmt)

mt_s32 mt_drv_fe_init(mt_void);
mt_void mt_drv_fe_deinit(mt_void);

mt_s32 mt_drv_fe_get_default_attr(mt_u32 tuner_id, mt_unf_fe_attr_t *p_fe_attr);
mt_s32 mt_drv_fe_set_attr(mt_u32 tuner_id, const mt_unf_fe_attr_t *p_fe_attr);
mt_s32 mt_drv_fe_set_sat_attr(mt_u32 tuner_id, const mt_unf_fe_sat_attr_t *p_sat_attr);
mt_s32 mt_drv_fe_open(mt_u32 tuner_id);
mt_s32 mt_drv_fe_close(mt_u32 tuner_id);
mt_s32 mt_drv_fe_connect(mt_u32 tuner_id, const mt_unf_fe_connect_para_t *p_conn_prm, mt_u32 timeout);
mt_s32 mt_drv_fe_get_status(mt_u32 tuner_id, mt_unf_fe_status_t *p_fe_status);
mt_s32 mt_drv_fe_get_ber(mt_u32 tuner_id, mt_u32 *p_ber);
mt_s32 mt_drv_fe_get_rs(mt_u32 tuner_id, mt_u32 *p_rs);

mt_s32 mt_drv_fe_get_snr(mt_u32 tuner_id, mt_s8 *p_snr);

mt_s32 mt_drv_fe_get_rssi(mt_u32 tuner_id, mt_u32 *p_rssi);
mt_s32 mt_drv_fe_get_freq_symb_offset(mt_u32 tuner_id, mt_u32 *p_offset);
mt_s32 mt_drv_fe_set_lnb_out(mt_u32 tuner_id, int lnb_power);
mt_s32 mt_drv_fe_set_switch_freq(mt_u32 switch_freq);
mt_s32 mt_drv_fe_get_switch_freq(mt_u32 *p_switch_freq);
mt_s32 mt_drv_fe_set_loop_through(mt_u32 tuner_id);
mt_s32 mt_drv_fe_cancel_loop_through(mt_u32 tuner_id);
mt_s32 mt_drv_fe_set_ts_out(mt_u32 tuner_id, mt_unf_fe_ts_out_t *p_ts_out);

#if 0
mt_s32 mt_drv_fe_set_lnb_config( mt_u32 tuner_id, const mt_unf_fe_lnb_config_t *p_lnb);
mt_s32 mt_drv_fe_get_lnb_config( mt_u32 tuner_id, mt_unf_fe_lnb_config_t *p_lnb);
mt_s32 mt_drv_fe_set_lnb_power(mt_u32 tuner_id, mt_unf_fe_lnb_power_t en_lnbpower);
mt_s32 mt_drv_fe_get_lnb_power(mt_u32 tuner_id, mt_unf_fe_lnb_power_t *p_en_lnbpower);
mt_s32 mt_drv_fe_switch_22k(mt_u32 tuner_id, mt_unf_fe_switch_22k_t en_port);
mt_s32 mt_drv_fe_diseqc_switch_4port(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch4port_t* p_prm);
mt_s32 mt_drv_fe_diseqc_switch_16port(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch16port_t* p_prm);
mt_s32 mt_drv_fe_diseqc_store_pos(mt_u32 tuner_id, const mt_unf_fe_diseqc_position_t *p_prm);
mt_s32 mt_drv_fe_diseqc_goto_pos(mt_u32 tuner_id, const mt_unf_fe_diseqc_position_t *p_prm);
mt_s32 mt_drv_fe_diseqc_set_limit(mt_u32 tuner_id, const mt_unf_fe_diseqc_limit_t* p_prm);
mt_s32 mt_drv_fe_diseqc_move(mt_u32 tuner_id, const mt_unf_fe_diseqc_move_t *p_prm);
mt_s32 mt_drv_fe_diseqc_stop(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t en_level);
mt_s32 mt_drv_fe_diseqc_recalculate(mt_u32 tuner_id, const mt_unf_fe_diseqc_recalculate_t *p_prm);
mt_s32 mt_drv_fe_diseqc_calc_angular(mt_u32 tuner_id, mt_unf_fe_diseqc_usals_para_t *p_prm);
mt_s32 mt_drv_fe_diseqc_goto_angular(mt_u32 tuner_id, const mt_unf_fe_diseqc_usals_angular_t *p_prm);
mt_s32 mt_drv_fe_diseqc_reset(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t en_level);
mt_s32 mt_drv_fe_diseqc_standby(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t en_level);
mt_s32 mt_drv_fe_diseqc_wakeup(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t en_level);
#endif

#endif
