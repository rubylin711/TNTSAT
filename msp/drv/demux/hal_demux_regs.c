/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mt_module_debug.h"

#include <linux/kernel.h>
#include "hal_demux_regs.h"
#include "mt_mach/chipinfo.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "drv_demux_config.h"

/*----------------------------------------------------------------------------*/
/* mirror variables                                                           */
/*----------------------------------------------------------------------------*/

#define VID_CH_REG_PRE_SET_VAL  0x5FA072D1

#define AUD_CH_REG_PRE_SET_VAL  0x7F8030D1

/*----------------------------------------------------------------------------*/
/* register dmx_ts0_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts0_sample_ctrl(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), data);
}

mt_u32  reg_get_ts0_sample_ctrl(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
}

mt_void reg_set_ts0_sample_ctrl_syncon_th(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.syncon_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_syncon_th(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
	return d.bitc.syncon_th;    
}

mt_void reg_set_ts0_sample_ctrl_syncoff_th(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.syncoff_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_syncoff_th(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.syncoff_th;
}

mt_void reg_set_ts0_sample_ctrl_ts_188_reg(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.ts_188_reg = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_ts_188_reg(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.ts_188_reg;
}

mt_void reg_set_ts0_sample_ctrl_ts_188_reg_en(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.ts_188_reg_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_ts_188_reg_en(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.ts_188_reg_en;
}

mt_void reg_set_ts0_sample_ctrl_brk_sel(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.brk_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_brk_sel(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.brk_sel;
}

mt_void reg_set_ts0_sample_ctrl_sop_token(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.sop_token = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_sop_token(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.sop_token;
}

mt_void reg_set_ts0_sample_ctrl_sync_bypass(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.sync_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_sync_bypass(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.sync_bypass;
}

mt_void reg_set_ts0_sample_ctrl_val_bypass(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.val_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_val_bypass(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.val_bypass;
}

mt_void reg_set_ts0_sample_ctrl_sync_err_bypass(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.sync_err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_sync_err_bypass(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.sync_err_bypass;
}

mt_void reg_set_ts0_sample_ctrl_tei_bypass(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.tei_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_tei_bypass(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.tei_bypass;
}

mt_void reg_set_ts0_sample_ctrl_err_bypass(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_err_bypass(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.err_bypass;
}

mt_void reg_set_ts0_sample_ctrl_err_pol(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.err_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_err_pol(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.err_pol;
}

mt_void reg_set_ts0_sample_ctrl_val_pol(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.val_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_val_pol(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.val_pol;
}

mt_void reg_set_ts0_sample_ctrl_sync_pol(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.sync_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_sync_pol(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.sync_pol;
}

mt_void reg_set_ts0_sample_ctrl_serial_sel(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.serial_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_serial_sel(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.serial_sel;
}

mt_void reg_set_ts0_sample_ctrl_serial_en(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.serial_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_serial_en(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.serial_en;
}

mt_void reg_set_ts0_sample_ctrl_ts_en(mt_u8 data)
{
    reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    d.bitc.ts_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl), d.all);
}

mt_u8   reg_get_ts0_sample_ctrl_ts_en(mt_void)
{
	reg_ts0_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_ctrl));
    return d.bitc.ts_en;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts0_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts0_sample_sta(mt_void)
{
	reg_ts0_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_sta));
    return d.all;
}

mt_u8   reg_get_ts0_sample_sta_err_cnt(mt_void)
{
	reg_ts0_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_sta));
    return d.bitc.err_cnt;
}

mt_u8   reg_get_ts0_sample_sta_ts_cnt(mt_void)
{
	reg_ts0_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_sta));
    return d.bitc.ts_cnt;
}

mt_u8   reg_get_ts0_sample_sta_ts_188_en(mt_void)
{
	reg_ts0_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_sta));
    return d.bitc.ts_188_en;
}

mt_u8   reg_get_ts0_sample_sta_sync_lock(mt_void)
{
	reg_ts0_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_sta));
    return d.bitc.sync_lock;
}

mt_u8   reg_get_ts0_sample_sta_ts_lock(mt_void)
{
	reg_ts0_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts0_sample_sta));
    return d.bitc.ts_lock;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts1_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts1_sample_ctrl(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), data);
}

mt_u32  reg_get_ts1_sample_ctrl(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.all;
}

mt_void reg_set_ts1_sample_ctrl_syncon_th(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.syncon_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_syncon_th(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.syncon_th;
}

mt_void reg_set_ts1_sample_ctrl_syncoff_th(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.syncoff_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_syncoff_th(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.syncoff_th;
}

mt_void reg_set_ts1_sample_ctrl_ts_188_reg(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.ts_188_reg = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_ts_188_reg(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.ts_188_reg;
}

mt_void reg_set_ts1_sample_ctrl_ts_188_reg_en(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.ts_188_reg_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_ts_188_reg_en(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.ts_188_reg_en;
}

mt_void reg_set_ts1_sample_ctrl_brk_sel(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.brk_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_brk_sel(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.brk_sel;
}

mt_void reg_set_ts1_sample_ctrl_sop_token(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.sop_token = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_sop_token(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.sop_token;
}

mt_void reg_set_ts1_sample_ctrl_sync_bypass(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.sync_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_sync_bypass(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.sync_bypass;
}

mt_void reg_set_ts1_sample_ctrl_val_bypass(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.val_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_val_bypass(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.val_bypass;
}

mt_void reg_set_ts1_sample_ctrl_sync_err_bypass(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.sync_err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_sync_err_bypass(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.sync_err_bypass;
}

mt_void reg_set_ts1_sample_ctrl_tei_bypass(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.tei_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_tei_bypass(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.tei_bypass;
}

mt_void reg_set_ts1_sample_ctrl_err_bypass(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_err_bypass(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.err_bypass;
}

mt_void reg_set_ts1_sample_ctrl_err_pol(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.err_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_err_pol(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.err_pol;
}

mt_void reg_set_ts1_sample_ctrl_val_pol(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.val_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_val_pol(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.val_pol;
}

mt_void reg_set_ts1_sample_ctrl_sync_pol(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.sync_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_sync_pol(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.sync_pol;
}

mt_void reg_set_ts1_sample_ctrl_serial_sel(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.serial_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_serial_sel(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.serial_sel;
}

mt_void reg_set_ts1_sample_ctrl_serial_en(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.serial_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_serial_en(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.serial_en;
}

mt_void reg_set_ts1_sample_ctrl_ts_en(mt_u8 data)
{
    reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    d.bitc.ts_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), d.all);
}

mt_u8   reg_get_ts1_sample_ctrl_ts_en(mt_void)
{
	reg_ts1_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
    return d.bitc.ts_en;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts1_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts1_sample_sta(mt_void)
{
	reg_ts1_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_sta));
    return d.all;
}

mt_u8   reg_get_ts1_sample_sta_err_cnt(mt_void)
{
	reg_ts1_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_sta));
    return d.bitc.err_cnt;
}

mt_u8   reg_get_ts1_sample_sta_ts_cnt(mt_void)
{
	reg_ts1_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_sta));
    return d.bitc.ts_cnt;
}

mt_u8   reg_get_ts1_sample_sta_ts_188_en(mt_void)
{
	reg_ts1_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_sta));
    return d.bitc.ts_188_en;
}

mt_u8   reg_get_ts1_sample_sta_sync_lock(mt_void)
{
	reg_ts1_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_sta));
    return d.bitc.sync_lock;
}

mt_u8   reg_get_ts1_sample_sta_ts_lock(mt_void)
{
	reg_ts1_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_sta));
    return d.bitc.ts_lock;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts2_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts2_sample_ctrl(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), data);
}

mt_u32  reg_get_ts2_sample_ctrl(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.all;
}

mt_void reg_set_ts2_sample_ctrl_syncon_th(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.syncon_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_syncon_th(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.syncon_th;
}

mt_void reg_set_ts2_sample_ctrl_syncoff_th(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.syncoff_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_syncoff_th(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.syncoff_th;
}

mt_void reg_set_ts2_sample_ctrl_ts_188_reg(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.ts_188_reg = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_ts_188_reg(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.ts_188_reg;
}

mt_void reg_set_ts2_sample_ctrl_ts_188_reg_en(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.ts_188_reg_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_ts_188_reg_en(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.ts_188_reg_en;
}

mt_void reg_set_ts2_sample_ctrl_brk_sel(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.brk_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_brk_sel(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.brk_sel;
}

mt_void reg_set_ts2_sample_ctrl_sop_token(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.sop_token = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_sop_token(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.sop_token;
}

mt_void reg_set_ts2_sample_ctrl_sync_bypass(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.sync_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_sync_bypass(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.sync_bypass;
}

mt_void reg_set_ts2_sample_ctrl_val_bypass(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.val_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_val_bypass(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.val_bypass;
}

mt_void reg_set_ts2_sample_ctrl_sync_err_bypass(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.sync_err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_sync_err_bypass(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.sync_err_bypass;
}

mt_void reg_set_ts2_sample_ctrl_tei_bypass(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.tei_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_tei_bypass(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.tei_bypass;
}

mt_void reg_set_ts2_sample_ctrl_err_bypass(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_err_bypass(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.err_bypass;
}

mt_void reg_set_ts2_sample_ctrl_err_pol(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.err_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_err_pol(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.err_pol;
}

mt_void reg_set_ts2_sample_ctrl_val_pol(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.val_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_val_pol(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.val_pol;
}

mt_void reg_set_ts2_sample_ctrl_sync_pol(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.sync_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_sync_pol(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.sync_pol;
}

mt_void reg_set_ts2_sample_ctrl_serial_sel(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.serial_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_serial_sel(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.serial_sel;
}

mt_void reg_set_ts2_sample_ctrl_serial_en(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.serial_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_serial_en(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.serial_en;
}

mt_void reg_set_ts2_sample_ctrl_ts_en(mt_u8 data)
{
    reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    d.bitc.ts_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl), d.all);
}

mt_u8   reg_get_ts2_sample_ctrl_ts_en(mt_void)
{
	reg_ts2_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_ctrl));
    return d.bitc.ts_en;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts2_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts2_sample_sta(mt_void)
{
	reg_ts2_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_sta));
    return d.all;
}

mt_u8   reg_get_ts2_sample_sta_err_cnt(mt_void)
{
	reg_ts2_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_sta));
    return d.bitc.err_cnt;
}

mt_u8   reg_get_ts2_sample_sta_ts_cnt(mt_void)
{
	reg_ts2_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_sta));
    return d.bitc.ts_cnt;
}

mt_u8   reg_get_ts2_sample_sta_ts_188_en(mt_void)
{
	reg_ts2_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_sta));
    return d.bitc.ts_188_en;
}

mt_u8   reg_get_ts2_sample_sta_sync_lock(mt_void)
{
	reg_ts2_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_sta));
    return d.bitc.sync_lock;
}

mt_u8   reg_get_ts2_sample_sta_ts_lock(mt_void)
{
	reg_ts2_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts2_sample_sta));
    return d.bitc.ts_lock;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts3_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts3_sample_ctrl(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), data);
}

mt_u32  reg_get_ts3_sample_ctrl(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.all;
}

mt_void reg_set_ts3_sample_ctrl_syncon_th(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.syncon_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_syncon_th(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.syncon_th;
}

mt_void reg_set_ts3_sample_ctrl_syncoff_th(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.syncoff_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_syncoff_th(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.syncoff_th;
}

mt_void reg_set_ts3_sample_ctrl_ts_188_reg(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.ts_188_reg = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_ts_188_reg(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.ts_188_reg;
}

mt_void reg_set_ts3_sample_ctrl_ts_188_reg_en(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.ts_188_reg_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_ts_188_reg_en(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.ts_188_reg_en;
}

mt_void reg_set_ts3_sample_ctrl_brk_sel(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.brk_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_brk_sel(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.brk_sel;
}

mt_void reg_set_ts3_sample_ctrl_sop_token(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.sop_token = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_sop_token(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.sop_token;
}

mt_void reg_set_ts3_sample_ctrl_sync_bypass(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.sync_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_sync_bypass(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.sync_bypass;
}

mt_void reg_set_ts3_sample_ctrl_val_bypass(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.val_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_val_bypass(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.val_bypass;
}

mt_void reg_set_ts3_sample_ctrl_sync_err_bypass(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.sync_err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_sync_err_bypass(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.sync_err_bypass;
}

mt_void reg_set_ts3_sample_ctrl_tei_bypass(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.tei_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_tei_bypass(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.tei_bypass;
}

mt_void reg_set_ts3_sample_ctrl_err_bypass(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.err_bypass = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_err_bypass(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.err_bypass;
}

mt_void reg_set_ts3_sample_ctrl_err_pol(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.err_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_err_pol(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.err_pol;
}

mt_void reg_set_ts3_sample_ctrl_val_pol(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.val_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_val_pol(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.val_pol;
}

mt_void reg_set_ts3_sample_ctrl_sync_pol(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.sync_pol = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_sync_pol(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.sync_pol;
}

mt_void reg_set_ts3_sample_ctrl_serial_sel(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.serial_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_serial_sel(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.serial_sel;
}

mt_void reg_set_ts3_sample_ctrl_serial_en(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.serial_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_serial_en(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.serial_en;
}

mt_void reg_set_ts3_sample_ctrl_ts_en(mt_u8 data)
{
    reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    d.bitc.ts_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl), d.all);
}

mt_u8   reg_get_ts3_sample_ctrl_ts_en(mt_void)
{
	reg_ts3_sample_ctrl_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_ctrl));
    return d.bitc.ts_en;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts3_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts3_sample_sta(mt_void)
{
	reg_ts3_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_sta));
    return d.all;
}

mt_u8   reg_get_ts3_sample_sta_err_cnt(mt_void)
{
	reg_ts3_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_sta));
    return d.bitc.err_cnt;
}

mt_u8   reg_get_ts3_sample_sta_ts_cnt(mt_void)
{
	reg_ts3_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_sta));
    return d.bitc.ts_cnt;
}

mt_u8   reg_get_ts3_sample_sta_ts_188_en(mt_void)
{
	reg_ts3_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_sta));
    return d.bitc.ts_188_en;
}

mt_u8   reg_get_ts3_sample_sta_sync_lock(mt_void)
{
	reg_ts3_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_sta));
    return d.bitc.sync_lock;
}

mt_u8   reg_get_ts3_sample_sta_ts_lock(mt_void)
{
	reg_ts3_sample_sta_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts3_sample_sta));
    return d.bitc.ts_lock;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ts_stop_cnt_len (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts_stop_cnt_len(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts_stop_cnt_len), data);
}

mt_u32  reg_get_ts_stop_cnt_len(mt_void)
{
	reg_ts_stop_cnt_len_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts_stop_cnt_len));
    return d.all;
}

mt_void reg_set_ts_stop_cnt_len_ts_stop_cnt_len(mt_u8 data)
{
    reg_ts_stop_cnt_len_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts_stop_cnt_len));
    d.bitc.ts_stop_cnt_len = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts_stop_cnt_len), d.all);
}

mt_u8   reg_get_ts_stop_cnt_len_ts_stop_cnt_len(mt_void)
{
	reg_ts_stop_cnt_len_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts_stop_cnt_len));
    return d.bitc.ts_stop_cnt_len;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_urgent_cfg (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_urgent_cfg(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_urgent_cfg),  data);
}

mt_u32  reg_get_swtsi_urgent_cfg(mt_void)
{
	reg_swtsi_urgent_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_urgent_cfg));
    return d.all;
}

mt_void reg_set_swtsi_urgent_cfg_swtsi_urgent_mode(mt_u8 data)
{
    reg_swtsi_urgent_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_urgent_cfg));
    d.bitc.swtsi_urgent_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)((ulong)reg_dmx_swtsi_urgent_cfg), d.all);
}

mt_u8   reg_get_swtsi_urgent_cfg_swtsi_urgent_mode(mt_void)
{
	reg_swtsi_urgent_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_urgent_cfg));
    return d.bitc.swtsi_urgent_mode;
}

mt_void reg_set_swtsi_urgent_cfg_swtsi_burst_mode(mt_u8 data)
{
    reg_swtsi_urgent_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_urgent_cfg));
    d.bitc.swtsi_burst_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)((ulong)reg_dmx_swtsi_urgent_cfg), d.all);
}

mt_u8   reg_get_swtsi_urgent_cfg_swtsi_burst_mode(mt_void)
{
	reg_swtsi_urgent_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_urgent_cfg));
    return d.bitc.swtsi_burst_mode;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_lln_addr (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_lln_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_lln_addr + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_lln_addr(mt_u8 index)
{
	reg_swtsi_chn_lln_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_lln_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_lln_addr_ch_lln_addr(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_lln_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_lln_addr + index * 0x100));
    d.bitc.ch_lln_addr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_lln_addr + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_lln_addr_ch_lln_addr(mt_u8 index)
{
	reg_swtsi_chn_lln_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_lln_addr + index * 0x100));
    return d.bitc.ch_lln_addr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_control (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_control(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_control(mt_u8 index)
{
	reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_control_ch_enable(mt_u8 index, mt_u8 data)
{
    reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    d.bitc.ch_enable = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100), d.all);
}

mt_u8   reg_get_swtsi_chn_control_ch_enable(mt_u8 index)
{
	reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    return d.bitc.ch_enable;
}

mt_void reg_set_swtsi_chn_control_ch_load_en(mt_u8 index, mt_u8 data)
{
    reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    d.bitc.ch_load_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100), d.all);
}

mt_u8   reg_get_swtsi_chn_control_ch_load_en(mt_u8 index)
{
	reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    return d.bitc.ch_load_en;
}

mt_void reg_set_swtsi_chn_control_ch_lln_reload_en(mt_u8 index, mt_u8 data)
{
    reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    d.bitc.ch_lln_reload_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100), d.all);
}

mt_u8   reg_get_swtsi_chn_control_ch_lln_reload_en(mt_u8 index)
{
	reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    return d.bitc.ch_lln_reload_en;
}

mt_void reg_set_swtsi_chn_control_ch_src_endian(mt_u8 index, mt_u8 data)
{
    reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    d.bitc.ch_src_endian = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100), d.all);
}

mt_u8   reg_get_swtsi_chn_control_ch_src_endian(mt_u8 index)
{
	reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    return d.bitc.ch_src_endian;
}

mt_void reg_set_swtsi_chn_control_ch_time_care(mt_u8 index, mt_u8 data)
{
    reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    d.bitc.ch_time_care = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100), d.all);
}

mt_u8   reg_get_swtsi_chn_control_ch_time_care(mt_u8 index)
{
	reg_swtsi_chn_control_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_control + index * 0x100));
    return d.bitc.ch_time_care;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_state (read)                                        */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_state(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.all;
}

mt_u8   reg_get_swtsi_chn_state_ch_busy_state(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_busy_state;
}

mt_u8   reg_get_swtsi_chn_state_ch_load_busy_state(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_load_busy_state;
}

mt_u8   reg_get_swtsi_chn_state_ch_buf_data_valid(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_buf_data_valid;
}

mt_u8   reg_get_swtsi_chn_state_ch_lln_info_valid(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_lln_info_valid;
}

mt_u8   reg_get_swtsi_chn_state_ch_time_interval_valid(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_time_interval_valid;
}

mt_u8   reg_get_swtsi_chn_state_ch_sync0_err(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_sync0_err;
}

mt_u8   reg_get_swtsi_chn_state_ch_sync1_err(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_sync1_err;
}

mt_u8   reg_get_swtsi_chn_state_ch_len_err(mt_u8 index)
{
	reg_swtsi_chn_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_state + index * 0x100));
    return d.bitc.ch_len_err;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg0 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg0(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg0 + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_af_cfg0(mt_u8 index)
{
	reg_swtsi_chn_af_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg0 + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_af_cfg0_swtsi_af_cfg0(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_af_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg0 + index * 0x100));
    d.bitc.swtsi_af_cfg0 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg0 + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_af_cfg0_swtsi_af_cfg0(mt_u8 index)
{
	reg_swtsi_chn_af_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg0 + index * 0x100));
    return d.bitc.swtsi_af_cfg0;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg1 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg1(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg1 + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_af_cfg1(mt_u8 index)
{
	reg_swtsi_chn_af_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg1 + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_af_cfg1_swtsi_af_cfg1(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_af_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg1 + index * 0x100));
    d.bitc.swtsi_af_cfg1 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg1 + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_af_cfg1_swtsi_af_cfg1(mt_u8 index)
{
	reg_swtsi_chn_af_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg1 + index * 0x100));
    return d.bitc.swtsi_af_cfg1;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg2 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg2(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg2 + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_af_cfg2(mt_u8 index)
{
	reg_swtsi_chn_af_cfg2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg2 + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_af_cfg2_swtsi_af_cfg2(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_af_cfg2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg2 + index * 0x100));
    d.bitc.swtsi_af_cfg2 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg2 + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_af_cfg2_swtsi_af_cfg2(mt_u8 index)
{
	reg_swtsi_chn_af_cfg2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg2 + index * 0x100));
    return d.bitc.swtsi_af_cfg2;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg3 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg3(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg3 + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_af_cfg3(mt_u8 index)
{
	reg_swtsi_chn_af_cfg3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg3 + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_af_cfg3_swtsi_af_cfg3(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_af_cfg3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg3 + index * 0x100));
    d.bitc.swtsi_af_cfg3 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg3 + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_af_cfg3_swtsi_af_cfg3(mt_u8 index)
{
	reg_swtsi_chn_af_cfg3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg3 + index * 0x100));
    return d.bitc.swtsi_af_cfg3;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg4 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg4(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg4 + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_af_cfg4(mt_u8 index)
{
	reg_swtsi_chn_af_cfg4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg4 + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_af_cfg4_swtsi_af_cfg4(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_af_cfg4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg4 + index * 0x100));
    d.bitc.swtsi_af_cfg4 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg4 + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_af_cfg4_swtsi_af_cfg4(mt_u8 index)
{
	reg_swtsi_chn_af_cfg4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg4 + index * 0x100));
    return d.bitc.swtsi_af_cfg4;
}


/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg5 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg5(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg5 + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_af_cfg5(mt_u8 index)
{
	reg_swtsi_chn_af_cfg5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg5 + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_af_cfg5_swtsi_af_cfg5(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_af_cfg5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg5 + index * 0x100));
    d.bitc.swtsi_af_cfg5 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg5 + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_af_cfg5_swtsi_af_cfg5(mt_u8 index)
{
	reg_swtsi_chn_af_cfg5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_af_cfg5 + index * 0x100));
    return d.bitc.swtsi_af_cfg5;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_next_lln (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_next_lln(mt_u8 index)
{
	reg_swtsi_chn_next_lln_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_next_lln + index * 0x100));
    return d.all;
}

mt_u32  reg_get_swtsi_chn_next_lln_ch_next_lln(mt_u8 index)
{
	reg_swtsi_chn_next_lln_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_next_lln + index * 0x100));
    return d.bitc.ch_next_lln;
}

mt_u8   reg_get_swtsi_chn_next_lln_ch_lln_vld(mt_u8 index)
{
	reg_swtsi_chn_next_lln_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_next_lln + index * 0x100));
    return d.bitc.ch_lln_vld;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_staddr (read)                                  */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_staddr(mt_u8 index)
{
	reg_swtsi_chn_dbuf_staddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_staddr + index * 0x100));
    return d.all;
}

mt_u32  reg_get_swtsi_chn_dbuf_staddr_ch_dbuf_staddr(mt_u8 index)
{
	reg_swtsi_chn_dbuf_staddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_staddr + index * 0x100));
    return d.bitc.ch_dbuf_staddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_cfg (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_cfg(mt_u8 index)
{
	reg_swtsi_chn_dbuf_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_cfg + index * 0x100));
	return d.all;
}

mt_u16  reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_pid(mt_u8 index)
{
	reg_swtsi_chn_dbuf_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_cfg + index * 0x100));
    return d.bitc.ch_dbuf_pid;
}

mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_vpts(mt_u8 index)
{
	reg_swtsi_chn_dbuf_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_cfg + index * 0x100));
    return d.bitc.ch_dbuf_vpts;
}

mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_vdts(mt_u8 index)
{
	reg_swtsi_chn_dbuf_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_cfg + index * 0x100));
    return d.bitc.ch_dbuf_vdts;
}

mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_wpont_care_mode(mt_u8 index)
{
	reg_swtsi_chn_dbuf_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_cfg + index * 0x100));
    return d.bitc.ch_wpont_care_mode;
}

mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_node_num(mt_u8 index)
{
	reg_swtsi_chn_dbuf_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_cfg + index * 0x100));
    return d.bitc.ch_node_num;
}

mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_type(mt_u8 index)
{
	reg_swtsi_chn_dbuf_cfg_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_cfg + index * 0x100));
    return d.bitc.ch_dbuf_type;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_pid (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_pid(mt_u8 index)
{
	reg_swtsi_chn_dbuf_pid_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_pid + index * 0x100));
    return d.all;
}

mt_u32  reg_get_swtsi_chn_dbuf_pid_ch_dbuf_length(mt_u8 index)
{
	reg_swtsi_chn_dbuf_pid_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_pid + index * 0x100));
    return d.bitc.ch_dbuf_length;
}

mt_u8   reg_get_swtsi_chn_dbuf_pid_ch_dbuf_streamid(mt_u8 index)
{
	reg_swtsi_chn_dbuf_pid_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_pid + index * 0x100));
    return d.bitc.ch_dbuf_streamid;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_pts (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_pts(mt_u8 index)
{
	reg_swtsi_chn_dbuf_pts_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_pts + index * 0x100));
    return d.all;
}

mt_u32  reg_get_swtsi_chn_dbuf_pts_ch_dbuf_pts(mt_u8 index)
{
	reg_swtsi_chn_dbuf_pts_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_pts + index * 0x100));
    return d.bitc.ch_dbuf_pts;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_dts (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_dts(mt_u8 index)
{
	reg_swtsi_chn_dbuf_dts_t d;
    d.all =  HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_dts + index * 0x100));
    return d.all;
}

mt_u32  reg_get_swtsi_chn_dbuf_dts_ch_dbuf_dts(mt_u8 index)
{
	reg_swtsi_chn_dbuf_dts_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_dts + index * 0x100));
    return d.bitc.ch_dbuf_dts;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_rdpoint (read)                                 */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_rdpoint(mt_u8 index)
{
	reg_swtsi_chn_dbuf_rdpoint_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_rdpoint + index * 0x100));
    return d.all;
}

mt_u32  reg_get_swtsi_chn_dbuf_rdpoint_ch_dbuf_rdpoint(mt_u8 index)
{
	reg_swtsi_chn_dbuf_rdpoint_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_rdpoint + index * 0x100));
    return d.bitc.ch_dbuf_rdpoint;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_sublen (read)                                  */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_sublen(mt_u8 index)
{
	reg_swtsi_chn_dbuf_sublen_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_sublen + index * 0x100));
    return d.all;
}

mt_u32  reg_get_swtsi_chn_dbuf_sublen_ch_dbuf_sublen(mt_u8 index)
{
	reg_swtsi_chn_dbuf_sublen_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_sublen + index * 0x100));
    return d.bitc.ch_dbuf_sublen;
}

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_wrpoint (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_dbuf_wrpoint(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_wrpoint + index * 0x100), data);
}

mt_u32  reg_get_swtsi_chn_dbuf_wrpoint(mt_u8 index)
{
	reg_swtsi_chn_dbuf_wrpoint_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_wrpoint + index * 0x100));
    return d.all;
}

mt_void reg_set_swtsi_chn_dbuf_wrpoint_ch_dbuf_wrpoint(mt_u8 index, mt_u32 data)
{
    reg_swtsi_chn_dbuf_wrpoint_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_wrpoint + index * 0x100));
    d.bitc.ch_dbuf_wrpoint = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_wrpoint + index * 0x100), d.all);
}

mt_u32  reg_get_swtsi_chn_dbuf_wrpoint_ch_dbuf_wrpoint(mt_u8 index)
{
	reg_swtsi_chn_dbuf_wrpoint_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_swtsi_chn_dbuf_wrpoint + index * 0x100));
    return d.bitc.ch_dbuf_wrpoint;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsp_pcrset0 (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_tsp_pcrsetn(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04), data);
}

mt_u32  reg_get_dmx_tsp_pcrsetn(mt_u8 index)
{
	reg_dmx_tsp_pcrsetn_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04));
    return d.all;
}

mt_void reg_set_dmx_tsp_pcrsetn_pcr_ena(mt_u8 index, mt_u32 data)
{
    reg_dmx_tsp_pcrsetn_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04));
    d.bitc.pcr_ena = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04), d.all);
}

mt_u32  reg_get_dmx_tsp_pcrsetn_pcr_ena(mt_u8 index)
{
	reg_dmx_tsp_pcrsetn_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04));
    return d.bitc.pcr_ena;
}

mt_void reg_set_dmx_tsp_pcrsetn_pcr_ch(mt_u8 index, mt_u32 data)
{
    reg_dmx_tsp_pcrsetn_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04));
    d.bitc.pcr_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04), d.all);
}

mt_u32  reg_get_dmx_tsp_pcrsetn_pcr_ch(mt_u8 index)
{
	reg_dmx_tsp_pcrsetn_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04));
    return d.bitc.pcr_ch;
}

mt_void reg_set_dmx_tsp_pcrsetn_pcr_id(mt_u8 index, mt_u32 data)
{
    reg_dmx_tsp_pcrsetn_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04));
    d.bitc.pcr_id = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04), d.all);
}

mt_u32  reg_get_dmx_tsp_pcrsetn_pcr_id(mt_u8 index)
{
	reg_dmx_tsp_pcrsetn_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsp_pcrsetn + index * 0x04));
    return d.bitc.pcr_id;
}

/*----------------------------------------------------------------------------*/
/* register dmx_demux_slotn_cfg0 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_slotn_cfg0(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), data);
}

mt_u32  reg_get_demux_slotn_cfg0(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.all;
}

mt_void reg_set_demux_slotn_cfg0_pid(mt_u8 index, mt_u16 data)
{
    reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    d.bitc.pid = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), d.all);
}

mt_u16  reg_get_demux_slotn_cfg0_pid(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.bitc.pid;
}

mt_void reg_set_demux_slotn_cfg0_pid_filter_en(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    d.bitc.pid_filter_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg0_pid_filter_en(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.bitc.pid_filter_en;
}

mt_void reg_set_demux_slotn_cfg0_pid_filter_mode(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    d.bitc.pid_filter_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg0_pid_filter_mode(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.bitc.pid_filter_mode;
}

mt_void reg_set_demux_slotn_cfg0_cc_judge_mode(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    d.bitc.cc_judge_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg0_cc_judge_mode(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.bitc.cc_judge_mode;
}

mt_void reg_set_demux_slotn_cfg0_src(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    d.bitc.src = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg0_src(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.bitc.src;
}

mt_void reg_set_demux_slotn_cfg0_errts_del_en(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    d.bitc.errts_del_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg0_errts_del_en(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.bitc.errts_del_en;
}

mt_void reg_set_demux_slotn_cfg0_slot_en(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    d.bitc.slot_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg0_slot_en(mt_u8 index)
{
	reg_demux_slotn_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg0 + index * 0x08));
    return d.bitc.slot_en;
}

/*----------------------------------------------------------------------------*/
/* register dmx_demux_slotn_cfg1 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_slotn_cfg1(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), data);
}

mt_u32  reg_get_demux_slotn_cfg1(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.all;
}

mt_void reg_set_demux_slotn_cfg1_process_type(mt_u8 index, mt_u8 data)
{
   	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.process_type = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_process_type(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.process_type;
}

mt_void reg_set_demux_slotn_cfg1_cw_ch(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.cw_ch_0_3 = data&0xf;
    d.bitc.cw_ch_4 = data>>4;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_cw_ch(mt_u8 index)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return (d.bitc.cw_ch_0_3) | (d.bitc.cw_ch_4 << 4);

}

mt_u8   reg_get_demux_slotn_cfg1_ci_en(mt_u8 index)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return (d.bitc.cw_ch_0_3) | (d.bitc.cw_ch_4 << 4);

}

mt_void reg_set_demux_slotn_cfg1_ci_en(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.ci_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}


mt_void reg_set_demux_slotn_cfg1_descrambler_en(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.descrambler_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_descrambler_en(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.descrambler_en;
}

mt_void reg_set_demux_slotn_cfg1_rec_ch(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.rec_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_rec_ch(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.rec_ch;
}

mt_void reg_set_demux_slotn_cfg1_buf_full_mode(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.buf_full_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_buf_full_mode(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.buf_full_mode;
}

mt_void reg_set_demux_slotn_cfg1_sec_filter_mode(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.sec_filter_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_sec_filter_mode(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.sec_filter_mode;
}

mt_void reg_set_demux_slotn_cfg1_multisec_dis(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.multisec_dis = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_multisec_dis(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.multisec_dis;
}

mt_void reg_set_demux_slotn_cfg1_sec_discard_mode(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.sec_discard_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_sec_discard_mode(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.sec_discard_mode;
}

mt_void reg_set_demux_slotn_cfg1_sec_mode(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.sec_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_sec_mode(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.sec_mode;
}

mt_void reg_set_demux_slotn_cfg1_play_ch(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.play_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_play_ch(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.play_ch;
}

mt_void reg_set_demux_slotn_cfg1_sc_fetch_en(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.sc_fetch_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_sc_fetch_en(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.sc_fetch_en;
}

mt_void reg_set_demux_slotn_cfg1_sc_fetch_ch(mt_u8 index, mt_u8 data)
{
    reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    d.bitc.sc_fetch_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08), d.all);
}

mt_u8   reg_get_demux_slotn_cfg1_sc_fetch_ch(mt_u8 index)
{
	reg_demux_slotn_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_slotn_cfg1 + index * 0x08));
    return d.bitc.sc_fetch_ch;
}

/*----------------------------------------------------------------------------*/
/* register dmx_demux_pause_cfg0 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_pause_cfg0(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_pause_cfg0), data);
}

mt_u32  reg_get_demux_pause_cfg0(mt_void)
{
	reg_demux_pause_cfg0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_pause_cfg0));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_demux_pause_cfg1 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_pause_cfg1(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_pause_cfg1), data);
}

mt_u32  reg_get_demux_pause_cfg1(mt_void)
{
	reg_demux_pause_cfg1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_pause_cfg1));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register reg_dmx_demux_multi_rec_en (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_demux_multi_rec_en(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_multi_rec_en));
}

mt_void reg_set_demux_multi_rec_en(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_demux_multi_rec_en), data);
}

/*----------------------------------------------------------------------------*/
/* register dmx_demux_state (read)                                            */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_demux_state(mt_void)
{
	reg_demux_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_state));
    return d.all;
}

mt_u8   reg_get_demux_state_demux_busy(mt_void)
{
	reg_demux_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_state));
    return d.bitc.demux_busy;
}

mt_u8   reg_get_demux_state_pause_flag_uninserted(mt_void)
{
	reg_demux_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_state));
    return d.bitc.pause_flag_uninserted;
}

mt_u8   reg_get_demux_state_demux_req_cnt(mt_void)
{
	reg_demux_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_state));
    return d.bitc.demux_req_cnt;
}

mt_u8   reg_get_demux_state_demux_grant_cn(mt_void)
{
	reg_demux_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_demux_state));
    return d.bitc.demux_grant_cn;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
/*----------------------------------------------------------------------------*/
/* register dmx fast play                                                     */
/*----------------------------------------------------------------------------*/
mt_u32   reg_get_demux_fp_set_cfg(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_fp_set_cfg));
}

mt_void reg_set_demux_fp_set_cfg(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_fp_set_cfg), data);
}

mt_u32   reg_get_demux_lln_num_start(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_lln_num_start));
}

mt_void reg_set_demux_lln_num_start(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_lln_num_start), data);
}

mt_u32   reg_get_demux_fp_status(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_fp_status));
}

mt_u32   reg_get_demux_fp_swtsi_ch_set(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_fp_swtsi_ch_set));
}

mt_void reg_set_demux_fp_swtsi_ch_set(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_fp_swtsi_ch_set), data);
}
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) 
/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_dsch (read)                                            */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_tsi_ds_dsch(mt_void)
{
	reg_tsi_ds_dsch_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_dsch));
    return d.all;
}

mt_u8   reg_get_tsi_ds_dsch_err_ch(mt_void)
{
	reg_tsi_ds_dsch_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_dsch));
    return d.bitc.err_ch;
}

mt_u8   reg_get_tsi_ds_dsch_ds_busy(mt_void)
{
	reg_tsi_ds_dsch_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_dsch));
    return d.bitc.ds_busy;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_cw_op (read/write)                                     */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_cw_op(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op), data);
}

mt_u32  reg_get_tsi_ds_cw_op(mt_void)
{
	reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    return d.all;
}

mt_void reg_set_tsi_ds_cw_op_clr_en(mt_u8 data)
{
    reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    d.bitc.clr_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op), d.all);
}

mt_u8   reg_get_tsi_ds_cw_op_clr_en(mt_void)
{
	reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    return d.bitc.clr_en;
}

mt_void reg_set_tsi_ds_cw_op_odd_push_en(mt_u8 data)
{
    reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    d.bitc.odd_push_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op), d.all);
}

mt_u8   reg_get_tsi_ds_cw_op_odd_push_en(mt_void)
{
	reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    return d.bitc.odd_push_en;
}

mt_void reg_set_tsi_ds_cw_op_even_push_en(mt_u8 data)
{
    reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    d.bitc.even_push_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op), d.all);
}

mt_u8   reg_get_tsi_ds_cw_op_even_push_en(mt_void)
{
	reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    return d.bitc.even_push_en;
}

mt_void reg_set_tsi_ds_cw_op_cw_ch(mt_u8 data)
{
    reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    d.bitc.cw_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op), d.all);
}

mt_u8   reg_get_tsi_ds_cw_op_cw_ch(mt_void)
{
	reg_tsi_ds_cw_op_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_cw_op));
    return d.bitc.cw_ch;
}
#endif

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive0_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive0_init(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive0_init), data);
}

mt_u32  reg_get_tsi_ades_ive0_init(mt_void)
{
	reg_tsi_ades_ive0_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive0_init));
    return d.all;
}

mt_void reg_set_tsi_ades_ive0_init_ive_init0(mt_u32 data)
{
    reg_tsi_ades_ive0_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive0_init));
    d.bitc.ive_init0 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive0_init), d.all);
}

mt_u32  reg_get_tsi_ades_ive0_init_ive_init0(mt_void)
{
	reg_tsi_ades_ive0_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive0_init));
    return d.bitc.ive_init0;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive1_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive1_init(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive1_init), data);
}

mt_u32  reg_get_tsi_ades_ive1_init(mt_void)
{
	reg_tsi_ades_ive1_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive1_init));
    return d.all;
}

mt_void reg_set_tsi_ades_ive1_init_ive_init1(mt_u32 data)
{
    reg_tsi_ades_ive1_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive1_init));
    d.bitc.ive_init1 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive1_init), d.all);
}

mt_u32  reg_get_tsi_ades_ive1_init_ive_init1(mt_void)
{
	reg_tsi_ades_ive1_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive1_init));
    return d.bitc.ive_init1;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive2_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive2_init(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive2_init), data);
}

mt_u32  reg_get_tsi_ades_ive2_init(mt_void)
{
	reg_tsi_ades_ive2_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive2_init));
    return d.all;
}

mt_void reg_set_tsi_ades_ive2_init_ive_init0(mt_u32 data)
{
    reg_tsi_ades_ive2_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive2_init));
    d.bitc.ive_init2 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive2_init), d.all);
}

mt_u32  reg_get_tsi_ades_ive2_init_ive_init2(mt_void)
{
	reg_tsi_ades_ive2_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive2_init));
    return d.bitc.ive_init2;
}


/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive3_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive3_init(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive3_init), data);
}

mt_u32  reg_get_tsi_ades_ive3_init(mt_void)
{
	reg_tsi_ades_ive3_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive3_init));
    return d.all;
}

mt_void reg_set_tsi_ades_ive3_init_ive_init3(mt_u32 data)
{
    reg_tsi_ades_ive3_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive3_init));
    d.bitc.ive_init3 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive3_init), d.all);
}

mt_u32  reg_get_tsi_ades_ive3_init_ive_init3(mt_void)
{
	reg_tsi_ades_ive3_init_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive3_init));
    return d.bitc.ive_init3;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd0 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd0(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd0 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_odd0(mt_u8 index)
{
	reg_tsi_ds_chn_odd0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd0 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_odd0_cw_odd0(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_odd0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd0 + index * 0x80));
    d.bitc.cw_odd0 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd0 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_odd0_cw_odd0(mt_u8 index)
{
	reg_tsi_ds_chn_odd0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd0 + index * 0x80));
    return d.bitc.cw_odd0;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd1 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd1(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd1 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_odd1(mt_u8 index)
{
	reg_tsi_ds_chn_odd1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd1 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_odd1_cw_odd1(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_odd1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd1 + index * 0x80));
    d.bitc.cw_odd1 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd1 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_odd1_cw_odd1(mt_u8 index)
{
	reg_tsi_ds_chn_odd1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd1 + index * 0x80));
    return d.bitc.cw_odd1;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd2 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd2(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd2 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_odd2(mt_u8 index)
{
	reg_tsi_ds_chn_odd2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd2 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_odd2_cw_odd2(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_odd2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd2 + index * 0x80));
    d.bitc.cw_odd2 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd2 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_odd2_cw_odd2(mt_u8 index)
{
	reg_tsi_ds_chn_odd2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd2 + index * 0x80));
    return d.bitc.cw_odd2;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd3 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd3(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd3 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_odd3(mt_u8 index)
{
	reg_tsi_ds_chn_odd3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd3 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_odd3_cw_odd3(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_odd3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd3 + index * 0x80));
    d.bitc.cw_odd3 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd3 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_odd3_cw_odd3(mt_u8 index)
{
	reg_tsi_ds_chn_odd3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd3 + index * 0x80));
    return d.bitc.cw_odd3;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd4 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd4(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd4 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_odd4(mt_u8 index)
{
	reg_tsi_ds_chn_odd4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd4 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_odd4_cw_odd4(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_odd4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd4 + index * 0x80));
    d.bitc.cw_odd4 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd4 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_odd4_cw_odd4(mt_u8 index)
{
	reg_tsi_ds_chn_odd4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd4 + index * 0x80));
    return d.bitc.cw_odd4;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd5 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd5(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd5 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_odd5(mt_u8 index)
{
	reg_tsi_ds_chn_odd5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd5 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_odd5_cw_odd5(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_odd5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd5 + index * 0x80));
    d.bitc.cw_odd5 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd5 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_odd5_cw_odd5(mt_u8 index)
{
	reg_tsi_ds_chn_odd5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_odd5 + index * 0x80));
    return d.bitc.cw_odd5;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even0 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even0(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even0 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_even0(mt_u8 index)
{
	reg_tsi_ds_chn_even0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even0 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_even0_cw_even0(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_even0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even0 + index * 0x80));
    d.bitc.cw_even0 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even0 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_even0_cw_even0(mt_u8 index)
{
	reg_tsi_ds_chn_even0_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even0 + index * 0x80));
    return d.bitc.cw_even0;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even1 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even1(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even1 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_even1(mt_u8 index)
{
	reg_tsi_ds_chn_even1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even1 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_even1_cw_even1(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_even1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even1 + index * 0x80));
    d.bitc.cw_even1 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even1 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_even1_cw_even1(mt_u8 index)
{
	reg_tsi_ds_chn_even1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even1 + index * 0x80));
    return d.bitc.cw_even1;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even2 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even2(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even2 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_even2(mt_u8 index)
{
	reg_tsi_ds_chn_even2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even2 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_even2_cw_even2(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_even2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even2 + index * 0x80));
    d.bitc.cw_even2 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even2 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_even2_cw_even2(mt_u8 index)
{
	reg_tsi_ds_chn_even2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even2 + index * 0x80));
    return d.bitc.cw_even2;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even3 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even3(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even3 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_even3(mt_u8 index)
{
	reg_tsi_ds_chn_even3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even3 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_even3_cw_even3(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_even3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even3 + index * 0x80));
    d.bitc.cw_even3 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even3 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_even3_cw_even3(mt_u8 index)
{
	reg_tsi_ds_chn_even3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even3 + index * 0x80));
    return d.bitc.cw_even3;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even4 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even4(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even4 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_even4(mt_u8 index)
{
	reg_tsi_ds_chn_even4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even4 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_even4_cw_even4(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_even4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even4 + index * 0x80));
    d.bitc.cw_even4 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even4 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_even4_cw_even4(mt_u8 index)
{
	reg_tsi_ds_chn_even4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even4 + index * 0x80));
    return d.bitc.cw_even4;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even5 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even5(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even5 + index * 0x80), data);
}

mt_u32  reg_get_tsi_ds_chn_even5(mt_u8 index)
{
	reg_tsi_ds_chn_even5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even5 + index * 0x80));
    return d.all;
}

mt_void reg_set_tsi_ds_chn_even5_cw_even5(mt_u8 index, mt_u32 data)
{
    reg_tsi_ds_chn_even5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even5 + index * 0x80));
    d.bitc.cw_even5 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even5 + index * 0x80), d.all);
}

mt_u32  reg_get_tsi_ds_chn_even5_cw_even5(mt_u8 index)
{
	reg_tsi_ds_chn_even5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_even5 + index * 0x80));
    return d.bitc.cw_even5;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_info (read)                                        */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_tsi_ds_chn_info(mt_u8 index)
{
	reg_tsi_ds_chn_info_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_info + index * 0x80));
    return d.all;
}

mt_u8   reg_get_tsi_ds_chn_info_odd_cw_sta(mt_u8 index)
{
	reg_tsi_ds_chn_info_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_info + index * 0x80));
    return d.bitc.odd_cw_sta;
}

mt_u8   reg_get_tsi_ds_chn_info_even_cw_sta(mt_u8 index)
{
	reg_tsi_ds_chn_info_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_info + index * 0x80));
    return d.bitc.even_cw_sta;
}
/*----------------------------------------------------------------------------*/
/* register for descrambler                                                                                                 */
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6) 
mt_void reg_set_tsi_ds_chn_tscfg(mt_u8 index, mt_u32 data)
{
	MT_INFO_DEMUX("reg_dmx_tsi_ds_chn_tscfg_sym6=0x%x,index=0x%x, data=0x%x\n",
					reg_dmx_tsi_ds_chn_tscfg_sym6,index, data);
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), data);
}

mt_u32  reg_get_tsi_ds_chn_tscfg(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	return d.all;
}

mt_void reg_set_tsi_ds_chn_tscfg_ds_mode(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.ds_mode = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_ds_mode(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.ds_mode;
}

mt_void reg_set_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.scrtag_clr = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.scrtag_clr;
}

mt_void reg_set_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.tsscr_clr_range = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.tsscr_clr_range;
}

mt_void reg_set_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.ts_cwopt1_mode = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.ts_cwopt1_mode;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.pes_cwopt1_mode = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.pes_cwopt1_mode;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_enc_odd_even_eco(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.enc_odd_even_eco = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_enc_odd_even_eco(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.enc_odd_even_eco;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_enc_mode_eco(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.enc_mode_eco = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_enc_mode_eco(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.enc_mode_eco;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_scr_enc_force(mt_u8 index, mt_u8 data)
{
    reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    d.bitc.scr_enc_force = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_scr_enc_force(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.scr_enc_force;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_des_key_msb64(mt_u8 index, mt_u8 data)
{
    reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    d.bitc.des_key_msb64 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_des_key_msb64(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.des_key_msb64;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_csa2_key_msb64(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.csa2_key_msb64 = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_csa2_key_msb64(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.csa2_key_msb64;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_multi2_key_msb64(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.multi2_key_msb64 = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_multi2_key_msb64(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.multi2_key_msb64;
}


mt_void reg_set_tsi_ds_chn_tscfg_pes_gost_sbox_sel(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.gost_sbox_sel = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_gost_sbox_sel(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.gost_sbox_sel;
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_gost_bit_inv(mt_u8 index, mt_u8 data)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
	d.bitc.gost_bit_inv = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_gost_bit_inv(mt_u8 index)
{
	reg_tsi_ds_chn_tscfg_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym6 + index * 0x40));
    return d.bitc.gost_bit_inv;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_core (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_core(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    MT_INFO_DEMUX("reg_dmx_tsi_ds_core_sym6=0x%x,index=0x%x, data=0x%x\n",
			reg_dmx_tsi_ds_core_sym6,index, data);
		HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40), data);  
    } 
}

mt_u32  reg_get_tsi_ds_core(mt_u8 index)
{
	reg_tsi_ds_core_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40));
    return d.all;
}

mt_void reg_set_tsi_ds_core_ds_core_sel(mt_u8 index, mt_u8 data)
{
    reg_tsi_ds_core_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40));
    d.bitc.ds_core_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40), d.all);    
}

mt_u8   reg_get_tsi_ds_core_ds_core_sel(mt_u8 index)
{
	reg_tsi_ds_core_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40));
    return d.bitc.ds_core_sel;
}

mt_void reg_set_tsi_ds_core_csa3_opti(mt_u8 index, mt_u8 data)
{
    reg_tsi_ds_core_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40));
    d.bitc.csa3_opti = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40), d.all);   
}

mt_u8   reg_get_tsi_ds_core_csa3_opti(mt_u8 index)
{
	reg_tsi_ds_core_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym6 + index * 0x40));
    return d.bitc.csa3_opti;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_aes_ive (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_aes_ive(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40), data);   
}

mt_u32  reg_get_tsi_aes_ive(mt_u8 index)
{
	reg_tsi_aes_ive_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40));
    return d.all;
 
}

mt_void reg_set_tsi_aes_ive_ivecal_en(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_aes_ive_t_sym6 d;
    	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40));
	    d.bitc.ivecal_en = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40), d.all);
    }  
}

mt_u8   reg_get_tsi_aes_ive_ivecal_en(mt_u8 index)
{
	reg_tsi_aes_ive_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40));
    return d.bitc.ivecal_en;
}

mt_void reg_set_tsi_aes_ive_ivecal_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_aes_ive_t_sym6 d;
    	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40));
	    d.bitc.ivecal_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40), d.all);
    }
}

mt_u8   reg_get_tsi_aes_ive_ivecal_mode(mt_u8 index)
{
	reg_tsi_aes_ive_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_ive_sym6 + index * 0x40));
    return d.bitc.ivecal_mode;   
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_disc_mode (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_disc_mode(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    MT_INFO_DEMUX("reg_dmx_tsi_ades_disc_mode_sym6=0x%x,index=0x%x, data=0x%x\n",
			reg_dmx_tsi_ades_disc_mode_sym6,index, data);
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym6 + index * 0x40), data);
    }
  
}

mt_u32  reg_get_tsi_ades_disc_mode(mt_u8 index)
{
	reg_tsi_ades_disc_mode_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym6 + index * 0x40));
    return d.all;
   
}

mt_void reg_set_tsi_ades_disc_mode_disc_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_ades_disc_mode_t_sym6 d;
	    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym6 + index * 0x40));
	    d.bitc.disc_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym6 + index * 0x40), d.all);
    }    
}

mt_u8   reg_get_tsi_ades_disc_mode_disc_mode(mt_u8 index)
{
	reg_tsi_ades_disc_mode_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym6 + index * 0x40));
    return d.bitc.disc_mode;  
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_pktmode (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_pktmode(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    MT_INFO_DEMUX("reg_dmx_tsi_ades_pktmode_sym6=0x%x,index=0x%x, data=0x%x\n",
			reg_dmx_tsi_ades_pktmode_sym6,index, data);
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40), data);
    }
}

mt_u32  reg_get_tsi_ades_pktmode(mt_u8 index)
{
	reg_tsi_ades_pktmode_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40));
    return d.all; 
}

mt_void reg_set_tsi_ades_pktmode_short_pkt_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_ades_pktmode_t_sym6 d;
    	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40));
	    d.bitc.short_pkt_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40), d.all);
    } 
}

mt_u8   reg_get_tsi_ades_pktmode_short_pkt_mode(mt_u8 index)
{
	reg_tsi_ades_pktmode_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40));
    return d.bitc.short_pkt_mode; 
}

mt_void reg_set_tsi_ades_pktmode_small_pkt_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_ades_pktmode_t_sym6 d;
	    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40));
	    d.bitc.small_pkt_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40), d.all);
    } 
}

mt_u8   reg_get_tsi_ades_pktmode_small_pkt_mode(mt_u8 index)
{
	reg_tsi_ades_pktmode_t_sym6 d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym6 + index * 0x40));
    return d.bitc.small_pkt_mode;
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_keyslot_tab (read/write)                                   */
/*----------------------------------------------------------------------------*/
void reg_set_tsi_keyslot_tab(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), data);
}

mt_u32  reg_get_tsi_keyslot_tab(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x40));
    return d.all;
}

void reg_set_tsi_keyslot_tab_entry_valid(mt_u8 index, mt_u32 data)
{
    reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x40));
    d.bitc.entry_valid = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), d.all);
}

mt_u32  reg_get_tsi_keyslot_tab_entry_valid(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x40));
    return d.bitc.entry_valid;
}

void reg_set_tsi_keyslot_tab_even_key_slot_index(mt_u8 index, mt_u32 data)
{
    reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x40));
    d.bitc.even_key_slot_index = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), d.all);
}

mt_u32  reg_get_tsi_keyslot_tab_even_key_slot_index(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x40));
    return d.bitc.even_key_slot_index;
}

void reg_set_tsi_keyslot_tab_odd_key_slot_index(mt_u8 index, mt_u32 data)
{
    reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x40));
    d.bitc.odd_key_slot_index = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), d.all);
}

mt_u32  reg_get_tsi_keyslot_tab_odd_key_slot_index(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x40));
    return d.bitc.odd_key_slot_index;
}

/*----------------------------------------------------------------------------*/
/* register dmx_kt_endian (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_kt_endian(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_kt_endian), data);
}

mt_u32  reg_get_dmx_kt_endian(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_kt_endian));
}

mt_void reg_set_dmx_ds_big_little_endian(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_csa2_bigendian_sym6), data);
}

mt_u32 reg_get_dmx_ds_big_little_endian(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_csa2_bigendian_sym6));
}

/*
**  debug regs
*/
mt_u32 reg_get_dmx_ds_sechd1_busy(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_sechd1_busy_sym6));
}

mt_u32 reg_get_dmx_ds_sechd1_status(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_sechd1_status_sym6));
}

mt_u32 reg_get_dmx_ds_dbg0_attr_err(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_dbg0_attr_err_sym6));
}

mt_u32 reg_get_dmx_ds_dbg1(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_dbg1_sym6));
}

mt_u32 reg_get_dmx_ds_dbg2(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_dbg2_sym6));
}

mt_u32 reg_get_dmx_ds_status_rdata0(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_status_rdata0_sym6));
}

mt_u32 reg_get_dmx_ds_status_rdata1(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_status_rdata1_sym6));
}

mt_u32 reg_get_dmx_ds_status_rdata2(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_status_rdata2_sym6));
}

mt_void reg_set_dmx_ds_hwcg_mode(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_hwcg_mode_sym6), data);
}

mt_u32 reg_get_dmx_ds_hwcg_mode(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_hwcg_mode_sym6));
}

mt_void reg_set_dmx_ds_tskey_src_opt(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_tskey_src_opt_sym6), data);
}

mt_u32 reg_get_dmx_ds_tskey_src_opt(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_tskey_src_opt_sym6));
}

mt_void reg_set_dmx_ds_fw(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_fw_sym6), data);
}

mt_u32 reg_get_dmx_ds_fw(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_fw_sym6));
}


#else  /*#elif defined(CONFIG_MT_CHIP_SYMPHONY6) */
/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_tscfg (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_tscfg(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40), data);
    }
    else
    {
    	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80), data);
    }
}

mt_u32  reg_get_tsi_ds_chn_tscfg(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        return d.all;
    }
    else
    {
    	reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        return d.all;
    }
}

mt_void reg_set_tsi_ds_chn_tscfg_ds_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));        
        d.bitc.ds_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40), d.all);
    }
    else
    {
        reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        d.bitc.ds_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ds_chn_tscfg_ds_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        return d.bitc.ds_mode;
    }
    else
    {
    	reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        return d.bitc.ds_mode;
    }
}

mt_void reg_set_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index, mt_u8 data)
{
     if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
     {
        reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        d.bitc.scrtag_clr = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40), d.all);
     }
     else
     {
		reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
		d.bitc.scrtag_clr = data;
		HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        return d.bitc.scrtag_clr;
    }
    else
    {
    	reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        return d.bitc.scrtag_clr;
    }
}

mt_void reg_set_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
        reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        d.bitc.tsscr_clr_range = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40), d.all);
    }
    else
    {
        reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        d.bitc.tsscr_clr_range = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        return d.bitc.tsscr_clr_range;
    }
    else
    {
    	reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        return d.bitc.tsscr_clr_range;
    }
}

mt_void reg_set_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index, mt_u8 data)
{

    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
        reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        d.bitc.ts_cwopt1_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40), d.all);

    }
    else
    {
        reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        d.bitc.ts_cwopt1_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        return d.bitc.ts_cwopt1_mode;
    }
    else
    {
    	reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        return d.bitc.ts_cwopt1_mode;
    }
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
        reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        d.bitc.pes_cwopt1_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40), d.all);
    }
    else
    {
        reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        d.bitc.pes_cwopt1_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        return d.bitc.pes_cwopt1_mode;
    }
    else
    {
    	reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        return d.bitc.pes_cwopt1_mode;
    }
}

mt_void reg_set_tsi_ds_chn_tscfg_pes_cwopt3_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
        reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        //d.bitc.pes_cwopt3_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40), d.all);
    }
    else
    {
        reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        d.bitc.pes_cwopt3_mode = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ds_chn_tscfg_pes_enc_odd_even_eco_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_chn_tscfg_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + index * 0x40));
        return d.bitc.enc_odd_even_eco;
    }
    else
    {
    	reg_tsi_ds_chn_tscfg_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg + index * 0x80));
        return d.bitc.pes_cwopt3_mode;
    }
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_core (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_core(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
		HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym2 + index * 0x40), data);
    }
    else
    {
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core + index * 0x80), data);
    }
}

mt_u32  reg_get_tsi_ds_core(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_core_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym2 + index * 0x40));
        return d.all;
    }
    else
    {
    	reg_tsi_ds_core_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core + index * 0x80));
        return d.all;
    }
}

mt_void reg_set_tsi_ds_core_ds_core_sel(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
        reg_tsi_ds_core_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym2 + index * 0x40));
        d.bitc.ds_core_sel = data;
        HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym2 + index * 0x40), d.all);
    }
    else
    {
        reg_tsi_ds_core_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core + index * 0x80));
        d.bitc.ds_core_sel = data;
		HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ds_core_ds_core_sel(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ds_core_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym2 + index * 0x40));
        return d.bitc.ds_core_sel;
    }
    else
    {
    	reg_tsi_ds_core_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ds_core + index * 0x80));
        return d.bitc.ds_core_sel;
    }
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_aes_ive (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_aes_ive(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40), data);
    }
    else
    {
    	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80), data);
    }
}

mt_u32  reg_get_tsi_aes_ive(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_aes_ive_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40));
    	return d.all;
    }
    else
    {
    	reg_tsi_aes_ive_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80));
    	return d.all;
    }
}

mt_void reg_set_tsi_aes_ive_ivecal_en(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_aes_ive_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40));
	    d.bitc.ivecal_en = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40), d.all);
    }
    else
    {
	    reg_tsi_aes_ive_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80));
	    d.bitc.ivecal_en = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_aes_ive_ivecal_en(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_aes_ive_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40));
    	return d.bitc.ivecal_en;
    }
    else
    {
    	reg_tsi_aes_ive_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80));
    	return d.bitc.ivecal_en;
    }
}

mt_void reg_set_tsi_aes_ive_ivecal_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_aes_ive_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40));
	    d.bitc.ivecal_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40), d.all);
    }
    else
    {
	    reg_tsi_aes_ive_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80));
	    d.bitc.ivecal_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_aes_ive_ivecal_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_aes_ive_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40));
    	return d.bitc.ivecal_mode;
    }
    else
    {
    	reg_tsi_aes_ive_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80));
    	return d.bitc.ivecal_mode;
    }
}

mt_void reg_set_tsi_aes_ive_iveinit_reg_sel(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_aes_ive_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40));
	    d.bitc.iveinit_reg_sel = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40), d.all);
    }
    else
    {
	    reg_tsi_aes_ive_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80));
	    d.bitc.iveinit_reg_sel = data;
		HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_aes_ive_iveinit_reg_sel(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_aes_ive_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + index * 0x40));
    	return d.bitc.iveinit_reg_sel;
    }
    else
    {
    	reg_tsi_aes_ive_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_aes_ive + index * 0x80));
    	return d.bitc.iveinit_reg_sel;
    }
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_disc_mode (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_disc_mode(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym2 + index * 0x40), data);
    }
    else
    {
    	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode + index * 0x80), data);
    }
}
mt_u32  reg_get_tsi_ades_disc_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ades_disc_mode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym2 + index * 0x40));
    	return d.all;
    }
    else
    {
    	reg_tsi_ades_disc_mode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode + index * 0x80));  
    	return d.all;
    }
}
mt_void reg_set_tsi_ades_disc_mode_disc_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_ades_disc_mode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym2 + index * 0x40));
	    d.bitc.disc_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym2 + index * 0x40), d.all);
    }
    else
    {
    	reg_tsi_ades_disc_mode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode + index * 0x80));    
	    d.bitc.disc_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode + index * 0x80), d.all);
    }
}
mt_u8   reg_get_tsi_ades_disc_mode_disc_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ades_disc_mode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym2 + index * 0x40));
    	return d.bitc.disc_mode;
    }
    else
    {
    	reg_tsi_ades_disc_mode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode + index * 0x80));  
    	return d.bitc.disc_mode;
    }
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_pktmode (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_pktmode(mt_u8 index, mt_u32 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40), data);
    }
    else
    {
    	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80), data);
    }
}

mt_u32  reg_get_tsi_ades_pktmode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ades_pktmode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40));
    	return d.all;
    }
    else
    {
    	reg_tsi_ades_pktmode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80));
    	return d.all;
    }
}

mt_void reg_set_tsi_ades_pktmode_short_pkt_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_ades_pktmode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40));
	    d.bitc.short_pkt_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40), d.all);
    }
    else
    {
	    reg_tsi_ades_pktmode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80));
	    d.bitc.short_pkt_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ades_pktmode_short_pkt_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ades_pktmode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40));
    	return d.bitc.short_pkt_mode;
    }
    else
    {
    	reg_tsi_ades_pktmode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80));
    	return d.bitc.short_pkt_mode;
    }
}

mt_void reg_set_tsi_ades_pktmode_small_pkt_mode(mt_u8 index, mt_u8 data)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    reg_tsi_ades_pktmode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40));
	    d.bitc.small_pkt_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40), d.all);
    }
    else
    {
	    reg_tsi_ades_pktmode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80));
	    d.bitc.small_pkt_mode = data;
	    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80), d.all);
    }
}

mt_u8   reg_get_tsi_ades_pktmode_small_pkt_mode(mt_u8 index)
{
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
    	reg_tsi_ades_pktmode_t_sym2 d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + index * 0x40));
    	return d.bitc.small_pkt_mode;
    }
    else
    {
    	reg_tsi_ades_pktmode_t d;
		d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode + index * 0x80));
    	return d.bitc.small_pkt_mode;
    }
}

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_keyslot_tab (read/write)                                   */
/*----------------------------------------------------------------------------*/
void reg_set_tsi_keyslot_tab(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), data);
}

mt_u32  reg_get_tsi_keyslot_tab(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04));
    return d.all;
}

void reg_set_tsi_keyslot_tab_entry_valid(mt_u8 index, mt_u32 data)
{
    reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04));
    d.bitc.entry_valid = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), d.all);
}

mt_u32  reg_get_tsi_keyslot_tab_entry_valid(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04));
    return d.bitc.entry_valid;
}

void reg_set_tsi_keyslot_tab_even_key_slot_index(mt_u8 index, mt_u32 data)
{
    reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04));
    d.bitc.even_key_slot_index = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), d.all);
}

mt_u32  reg_get_tsi_keyslot_tab_even_key_slot_index(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04));
    return d.bitc.even_key_slot_index;
}

void reg_set_tsi_keyslot_tab_odd_key_slot_index(mt_u8 index, mt_u32 data)
{
    reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04));
    d.bitc.odd_key_slot_index = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04), d.all);
}

mt_u32  reg_get_tsi_keyslot_tab_odd_key_slot_index(mt_u8 index)
{
	reg_tsi_tsi_keyslot_tab_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + index * 0x04));
    return d.bitc.odd_key_slot_index;
}

/*----------------------------------------------------------------------------*/
/* register dmx_kt_endian (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_kt_endian(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_kt_endian), data);
}

mt_u32  reg_get_dmx_kt_endian(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_kt_endian));
}

mt_void reg_set_dmx_ds_big_little_endian(mt_u32 data)
{
	HAL_PUT_U32((volatile u32 *)(reg_dmx_ds_big_little_endian), data);
}

mt_u32 reg_get_dmx_ds_big_little_endian(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_big_little_endian));
}
#endif  //#elif defined(CONFIG_MT_CHIP_SYMPHONY6) 

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_algo_cw_ive_port (read/write)                                 */
/*----------------------------------------------------------------------------*/
void reg_set_tsi_algo_cw_ive_port(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40), data);
}

mt_u32  reg_get_tsi_algo_cw_ive_port(mt_u8 index)
{
	reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    return d.all;
}

void reg_set_tsi_algo_cw_ive_port_ades_dw_port(mt_u8 index, mt_u8 data)
{
    reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    d.bitc.ades_dw_port = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_algo_cw_ive_port_ades_dw_port(mt_u8 index)
{
	reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    return d.bitc.ades_dw_port;
}

void reg_set_tsi_algo_cw_ive_port_ades_byte_port(mt_u8 index, mt_u8 data)
{
    reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    d.bitc.ades_byte_port = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_algo_cw_ive_port_ades_byte_port(mt_u8 index)
{
	reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    return d.bitc.ades_byte_port;
}

void reg_set_tsi_algo_cw_ive_port_csa3_dw_port(mt_u8 index, mt_u8 data)
{
    reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    d.bitc.csa3_dw_port = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_algo_cw_ive_port_csa3_dw_port(mt_u8 index)
{
	reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    return d.bitc.csa3_dw_port;
}

void reg_set_tsi_algo_cw_ive_port_csa3_byte_port(mt_u8 index, mt_u8 data)
{
    reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    d.bitc.csa3_byte_port = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_algo_cw_ive_port_csa3_byte_port(mt_u8 index)
{
	reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    return d.bitc.csa3_byte_port;
}

void reg_set_tsi_algo_cw_ive_port_csa2_dw_port(mt_u8 index, mt_u8 data)
{
    reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    d.bitc.csa2_dw_port = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_algo_cw_ive_port_csa2_dw_port(mt_u8 index)
{
	reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    return d.bitc.csa2_dw_port;
}

void reg_set_tsi_algo_cw_ive_port_csa2_byte_port(mt_u8 index, mt_u8 data)
{
    reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    d.bitc.csa2_byte_port = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40), d.all);
}

mt_u8   reg_get_tsi_algo_cw_ive_port_csa2_byte_port(mt_u8 index)
{
	reg_tsi_algo_cw_ive_port_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + index * 0x40));
    return d.bitc.csa2_byte_port;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_staddr (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_staddr(mt_u8 index, mt_u32 data)
{	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_staddr + index * 0x04), data);
}

mt_u32  reg_get_bufn_staddr(mt_u8 index)
{
	reg_bufn_staddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_staddr + index * 0x04));
    return d.all;
}

mt_void reg_set_bufn_staddr_buf_ch_staddr(mt_u8 index, mt_u32 data)
{
    reg_bufn_staddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_staddr + index * 0x04));
    d.bitc.buf_ch_staddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_staddr + index * 0x04), d.all);
}

mt_u32  reg_get_bufn_staddr_buf_ch_staddr(mt_u8 index)
{
	reg_bufn_staddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_staddr + index * 0x04));
    return d.bitc.buf_ch_staddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_size (read/write)                                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_size(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04), data);
}

mt_u32  reg_get_bufn_size(mt_u8 index)
{
	reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    return d.all;
}

mt_void reg_set_bufn_size_disc_ch_size(mt_u8 index, mt_u8 data)
{
    reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    d.bitc.disc_ch_size = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04), d.all);
}

mt_u8   reg_get_bufn_size_disc_ch_size(mt_u8 index)
{
	reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    return d.bitc.disc_ch_size;
}

mt_void reg_set_bufn_size_data_ch_size(mt_u8 index, mt_u16 data)
{
    reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    d.bitc.data_ch_size = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04), d.all);
}

mt_u16  reg_get_bufn_size_data_ch_size(mt_u8 index)
{
	reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    return d.bitc.data_ch_size;
}

mt_void reg_set_bufn_size_disc_ch_rptr(mt_u8 index, mt_u8 data)
{
    reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    d.bitc.disc_ch_rptr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04), d.all);
}

mt_u8   reg_get_bufn_size_disc_ch_rptr(mt_u8 index)
{
	reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    return d.bitc.disc_ch_rptr;
}

mt_void reg_set_bufn_size_data_ch_rptr(mt_u8 index, mt_u16 data)
{
    reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    d.bitc.data_ch_rptr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04), d.all);
}

mt_u16  reg_get_bufn_size_data_ch_rptr(mt_u8 index)
{
	reg_bufn_size_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_size + index * 0x04));
    return d.bitc.data_ch_rptr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_disc_wptr (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_disc_wptr(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_disc_wptr + index * 0x04), data);
}

mt_u32  reg_get_bufn_disc_wptr(mt_u8 index)
{
	reg_bufn_disc_wptr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_disc_wptr + index * 0x04));
    return d.all;
}

mt_void reg_set_bufn_disc_wptr_disc_ch_wptr(mt_u8 index, mt_u16 data)
{
    reg_bufn_disc_wptr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_disc_wptr + index * 0x04));
    d.bitc.disc_ch_wptr = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_disc_wptr + index * 0x04), d.all);
}

mt_u16  reg_get_bufn_disc_wptr_disc_ch_wptr(mt_u8 index)
{
	reg_bufn_disc_wptr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_disc_wptr + index * 0x04));
    return d.bitc.disc_ch_wptr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_ts_int_cfg (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_ts_int_cfg(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04), data);
}

mt_u32  reg_get_bufn_ts_int_cfg(mt_u8 index)
{
	reg_bufn_ts_int_cfg_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04));
    return d.all;
}

mt_void reg_set_bufn_ts_int_cfg_ts_inf_cfg(mt_u8 index, mt_u8 data)
{
    reg_bufn_ts_int_cfg_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04));
    d.bitc.ts_inf_cfg = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04), d.all);
}

mt_u8   reg_get_bufn_ts_int_cfg_ts_inf_cfg(mt_u8 index)
{
	reg_bufn_ts_int_cfg_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04));
    return d.bitc.ts_inf_cfg;
}

mt_void reg_set_bufn_ts_int_cfg_ts_rcv_cnt(mt_u8 index, mt_u8 data)
{
    reg_bufn_ts_int_cfg_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04));
    d.bitc.ts_rcv_cnt = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04), d.all);
}

mt_u8   reg_get_bufn_ts_int_cfg_ts_rcv_cnt(mt_u8 index)
{
	reg_bufn_ts_int_cfg_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_ts_int_cfg + index * 0x04));
    return d.bitc.ts_rcv_cnt;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_data_wptr (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_data_wptr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_data_wptr + index * 0x04), data);
}

mt_u32  reg_get_bufn_data_wptr(mt_u8 index)
{
	reg_bufn_data_wptr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_data_wptr + index * 0x04));
    return d.all;
}

mt_void reg_set_bufn_data_wptr_data_ch_wptr(mt_u8 index, mt_u32 data)
{
    reg_bufn_data_wptr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_data_wptr + index * 0x04));
    d.bitc.data_ch_wptr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_data_wptr + index * 0x04), d.all);
}

mt_u32  reg_get_bufn_data_wptr_data_ch_wptr(mt_u8 index)
{
	reg_bufn_data_wptr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_data_wptr + index * 0x04));
    return (*(volatile reg_bufn_data_wptr_t *)(((ulong)reg_dmx_bufn_data_wptr) + index * 0x04)).bitc.data_ch_wptr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_int_sta (read)                                           */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_bufn_int_sta(mt_u8 index)
{
	reg_bufn_int_sta_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_int_sta + index * 0x04));
    return d.all;
}

mt_u8   reg_get_bufn_int_sta_buf_ch_int_sta(mt_u8 index)
{
	reg_bufn_int_sta_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_int_sta + index * 0x04));
    return d.bitc.buf_ch_int_sta;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_cursec_len (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_cursec_len(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04), data);
}

mt_u32  reg_get_bufn_cursec_len(mt_u8 index)
{
	reg_bufn_cursec_len_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04));
    return d.all;
}

mt_void reg_set_bufn_cursec_len_vld_byte(mt_u8 index, mt_u16 data)
{
    reg_bufn_cursec_len_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04));
    d.bitc.vld_byte = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04), d.all);
}

mt_u16  reg_get_bufn_cursec_len_vld_byte(mt_u8 index)
{
	reg_bufn_cursec_len_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04));
    return d.bitc.vld_byte;
}

mt_void reg_set_bufn_cursec_len_res_length(mt_u8 index, mt_u16 data)
{
    reg_bufn_cursec_len_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04));
    d.bitc.res_length = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04), d.all);
}

mt_u16  reg_get_bufn_cursec_len_res_length(mt_u8 index)
{
	reg_bufn_cursec_len_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04));
    return d.bitc.res_length;
}

mt_void reg_set_bufn_cursec_len_syntax(mt_u8 index, mt_u8 data)
{
    reg_bufn_cursec_len_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04));
    d.bitc.syntax = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04), d.all);
}

mt_u8   reg_get_bufn_cursec_len_syntax(mt_u8 index)
{
	reg_bufn_cursec_len_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_cursec_len + index * 0x04));
    return d.bitc.syntax;
}


/*----------------------------------------------------------------------------*/
/* register ((ulong)reg_dmx_bufn_crc_value) (read)                                  */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_bufn_crc_value(mt_u8 index)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bufn_crc_value + index * 0x04));
}

/*----------------------------------------------------------------------------*/
/* register dmx_filtern_config (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_filtern_config(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), data);
}

mt_u32  reg_get_filtern_config(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.all;
}

mt_void reg_set_filtern_config_filter_root(mt_u8 index, mt_u8 data)
{
    reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    d.bitc.filter_root = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), d.all);
}

mt_u8   reg_get_filtern_config_filter_root(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.bitc.filter_root;
}

mt_void reg_set_filtern_config_filter_en(mt_u8 index, mt_u8 data)
{
    reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    d.bitc.filter_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), d.all);
}

mt_u8   reg_get_filtern_config_filter_en(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.bitc.filter_en;
}

mt_void reg_set_filtern_config_rcv_mode(mt_u8 index, mt_u8 data)
{
    reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    d.bitc.rcv_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), d.all);
}

mt_u8   reg_get_filtern_config_rcv_mode(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.bitc.rcv_mode;
}

mt_void reg_set_filtern_config_slot_num(mt_u8 index, mt_u8 data)
{
    reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    d.bitc.buf_id = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), d.all);
}

mt_u8   reg_get_filtern_config_slot_num(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.bitc.buf_id;
}

mt_void reg_set_filtern_config_filt_sta(mt_u8 index, mt_u8 data)
{
    reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    d.bitc.filt_sta = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), d.all);
}

mt_u8   reg_get_filtern_config_filt_sta(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.bitc.filt_sta;
}

mt_void reg_set_filtern_config_single_end_flag(mt_u8 index, mt_u8 data)
{
    reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    d.bitc.single_end_flag = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), d.all);
}

mt_u8   reg_get_filtern_config_single_end_flag(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.bitc.single_end_flag;
}

mt_void reg_set_filtern_config_filter_store(mt_u8 index, mt_u8 data)
{
    reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    d.bitc.filter_store = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04), d.all);
}

mt_u8   reg_get_filtern_config_filter_store(mt_u8 index)
{
	reg_filtern_config_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_filtern_config + index * 0x04));
    return d.bitc.filter_store;
}

/*----------------------------------------------------------------------------*/
/* register dmx_funit_filter_data (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_funit_filter_data(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04), data);
}

mt_u32  reg_get_funit_filter_data(mt_u8 index)
{
	reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    return d.all;
}

mt_void reg_set_funit_filter_data_filter_data_byte3(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    d.bitc.filter_data_byte3 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_data_filter_data_byte3(mt_u8 index)
{
	reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    return d.bitc.filter_data_byte3;
}

mt_void reg_set_funit_filter_data_filter_data_byte2(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    d.bitc.filter_data_byte2 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_data_filter_data_byte2(mt_u8 index)
{
	reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    return d.bitc.filter_data_byte2;
}

mt_void reg_set_funit_filter_data_filter_data_byte1(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    d.bitc.filter_data_byte1 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_data_filter_data_byte1(mt_u8 index)
{
	reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    return d.bitc.filter_data_byte1;
}

mt_void reg_set_funit_filter_data_filter_data_byte0(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    d.bitc.filter_data_byte0 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_data_filter_data_byte0(mt_u8 index)
{
	reg_funit_filter_data_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_data + index * 0x04));
    return d.bitc.filter_data_byte0;
}

/*----------------------------------------------------------------------------*/
/* register dmx_funit_filter_mask (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_funit_filter_mask(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04), data);
}

mt_u32  reg_get_funit_filter_mask(mt_u8 index)
{
	reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    return d.all;
}

mt_void reg_set_funit_filter_mask_filter_mask_byte3(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    d.bitc.filter_mask_byte3 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_mask_filter_mask_byte3(mt_u8 index)
{
	reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    return d.bitc.filter_mask_byte3;
}

mt_void reg_set_funit_filter_mask_filter_mask_byte2(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    d.bitc.filter_mask_byte2 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_mask_filter_mask_byte2(mt_u8 index)
{
	reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    return d.bitc.filter_mask_byte2;
}

mt_void reg_set_funit_filter_mask_filter_mask_byte1(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    d.bitc.filter_mask_byte1 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_mask_filter_mask_byte1(mt_u8 index)
{
	reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    return d.bitc.filter_mask_byte1;
}

mt_void reg_set_funit_filter_mask_filter_mask_byte0(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    d.bitc.filter_mask_byte0 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_mask_filter_mask_byte0(mt_u8 index)
{
	reg_funit_filter_mask_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mask + index * 0x04));
    return d.bitc.filter_mask_byte0;
}

/*----------------------------------------------------------------------------*/
/* register dmx_funit_filter_mode (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_funit_filter_mode(mt_u8 index, mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04), data);
}

mt_u32  reg_get_funit_filter_mode(mt_u8 index)
{
	reg_funit_filter_mode_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04));
    return d.all;
}

mt_void reg_set_funit_filter_mode_filter_next(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_mode_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04));
    d.bitc.filter_next = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_mode_filter_next(mt_u8 index)
{
	reg_funit_filter_mode_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04));
    return d.bitc.filter_next;
}

mt_void reg_set_funit_filter_mode_filter_mode(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_mode_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04));
    d.bitc.filter_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_mode_filter_mode(mt_u8 index)
{
	reg_funit_filter_mode_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04));
    return d.bitc.filter_mode;
}

mt_void reg_set_funit_filter_mode_filter_root_end(mt_u8 index, mt_u8 data)
{
    reg_funit_filter_mode_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04));
    d.bitc.filter_root_end = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04), d.all);
}

mt_u8   reg_get_funit_filter_mode_filter_root_end(mt_u8 index)
{
	reg_funit_filter_mode_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_funit_filter_mode + index * 0x04));
    return d.bitc.filter_root_end;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_channel_parse_en (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_channel_parse_en(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_channel_parse_en), data);
}

mt_u32  reg_get_trpp_channel_parse_en(mt_void)
{
	reg_trpp_channel_parse_en_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_channel_parse_en));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_clear_status (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_clear_status(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_clear_status), data);
}

mt_u32  reg_get_trpp_ch_clear_status(mt_void)
{
	reg_trpp_ch_clear_status_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_clear_status));
    return d.all;
}

mt_void reg_set_trpp_ch_clear_status_trpp_ch_clr_ok(mt_u8 data)
{
    reg_trpp_ch_clear_status_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_clear_status));
    d.bitc.trpp_ch_clr_ok = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_clear_status), d.all);
}

mt_u8   reg_get_trpp_ch_clear_status_trpp_ch_clr_ok(mt_void)
{
	reg_trpp_ch_clear_status_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_clear_status));
    return d.bitc.trpp_ch_clr_ok;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_bus_urgent (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_bus_urgent(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent), data);
}

mt_u32  reg_get_trpp_bus_urgent(mt_void)
{
	reg_trpp_bus_urgent_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent));
    return d.all;
}

mt_void reg_set_trpp_bus_urgent_trpp_urgent_mod(mt_u8 data)
{
    reg_trpp_bus_urgent_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent));
    d.bitc.trpp_urgent_mod = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent), d.all);
}

mt_u8   reg_get_trpp_bus_urgent_trpp_urgent_mod(mt_void)
{
	reg_trpp_bus_urgent_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent));
    return d.bitc.trpp_urgent_mod;
}

mt_void reg_set_trpp_bus_urgent_trpp_burst_len_mod(mt_u8 data)
{
    reg_trpp_bus_urgent_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent));
    d.bitc.trpp_burst_len_mod = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent), d.all);
}

mt_u8   reg_get_trpp_bus_urgent_trpp_burst_len_mod(mt_void)
{
	reg_trpp_bus_urgent_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_bus_urgent));
    return d.bitc.trpp_burst_len_mod;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_channel_record_en (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_channel_record_en(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_channel_record_en), data);
}

mt_u32  reg_get_trpp_channel_record_en(mt_void)
{
	reg_trpp_channel_record_en_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_channel_record_en));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt1_4 (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt1_4(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4), data);
}

mt_u32  reg_get_trpp_sc_index_flt1_4(mt_void)
{
	reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    return d.all;
}

mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt1(mt_u8 data)
{
    reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    d.bitc.trpp_sc_idx_flt1 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt1(mt_void)
{
	reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    return d.bitc.trpp_sc_idx_flt1;
}

mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt2(mt_u8 data)
{
    reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    d.bitc.trpp_sc_idx_flt2 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt2(mt_void)
{
	reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    return d.bitc.trpp_sc_idx_flt2;
}

mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt3(mt_u8 data)
{
    reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    d.bitc.trpp_sc_idx_flt3 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt3(mt_void)
{
	reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    return d.bitc.trpp_sc_idx_flt3;
}

mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt4(mt_u8 data)
{
    reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    d.bitc.trpp_sc_idx_flt4 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt4(mt_void)
{
	reg_trpp_sc_index_flt1_4_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt1_4));
    return d.bitc.trpp_sc_idx_flt4;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt5_8 (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt5_8(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8), data);
}

mt_u32  reg_get_trpp_sc_index_flt5_8(mt_void)
{
	reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    return d.all;
}

mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt5(mt_u8 data)
{
    reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    d.bitc.trpp_sc_idx_flt5 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt5(mt_void)
{
	reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    return d.bitc.trpp_sc_idx_flt5;
}

mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt6(mt_u8 data)
{
    reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    d.bitc.trpp_sc_idx_flt6 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt6(mt_void)
{
	reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    return d.bitc.trpp_sc_idx_flt6;
}

mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt7(mt_u8 data)
{
    reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    d.bitc.trpp_sc_idx_flt7 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt7(mt_void)
{
	reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    return d.bitc.trpp_sc_idx_flt7;
}

mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt8(mt_u8 data)
{
    reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    d.bitc.trpp_sc_idx_flt8 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt8(mt_void)
{
	reg_trpp_sc_index_flt5_8_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt5_8));
    return d.bitc.trpp_sc_idx_flt8;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt9_10 (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt9_10(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10), data);
}

mt_u32  reg_get_trpp_sc_index_flt9_10(mt_void)
{
	reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    return d.all;
}

mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_l(mt_u8 data)
{
    reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    d.bitc.trpp_sc_idx_flt9_l = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_l(mt_void)
{
	reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    return d.bitc.trpp_sc_idx_flt9_l;
}

mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_h(mt_u8 data)
{
    reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    d.bitc.trpp_sc_idx_flt9_h = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_h(mt_void)
{
	reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    return d.bitc.trpp_sc_idx_flt9_h;
}

mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_l(mt_u8 data)
{
    reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    d.bitc.trpp_sc_idx_flt10_l = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_l(mt_void)
{
	reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    return d.bitc.trpp_sc_idx_flt10_l;
}

mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_h(mt_u8 data)
{
    reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    d.bitc.trpp_sc_idx_flt10_h = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_h(mt_void)
{
	reg_trpp_sc_index_flt9_10_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt9_10));
    return d.bitc.trpp_sc_idx_flt10_h;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt11_12 (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt11_12(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12), data);
}

mt_u32  reg_get_trpp_sc_index_flt11_12(mt_void)
{
	reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    return d.all;
}

mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_l(mt_u8 data)
{
    reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    d.bitc.trpp_sc_idx_flt11_l = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_l(mt_void)
{
	reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    return d.bitc.trpp_sc_idx_flt11_l;
}

mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_h(mt_u8 data)
{
    reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    d.bitc.trpp_sc_idx_flt11_h = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_h(mt_void)
{
	reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    return d.bitc.trpp_sc_idx_flt11_h;
}

mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_l(mt_u8 data)
{
    reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    d.bitc.trpp_sc_idx_flt12_l = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_l(mt_void)
{
	reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    return d.bitc.trpp_sc_idx_flt12_l;
}

mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_h(mt_u8 data)
{
    reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    d.bitc.trpp_sc_idx_flt12_h = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_h(mt_void)
{
	reg_trpp_sc_index_flt11_12_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt11_12));
    return d.bitc.trpp_sc_idx_flt12_h;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt0 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt0(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0), data);
}

mt_u32  reg_get_trpp_sc_index_flt0(mt_void)
{
	reg_trpp_sc_index_flt0_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0));
    return d.all;
}

mt_void reg_set_trpp_sc_index_flt0_trpp_sc_idx_byte31(mt_u8 data)
{
    reg_trpp_sc_index_flt0_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0));
    d.bitc.trpp_sc_idx_byte31 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt0_trpp_sc_idx_byte31(mt_void)
{
	reg_trpp_sc_index_flt0_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0));
    return d.bitc.trpp_sc_idx_byte31;
}

mt_void reg_set_trpp_sc_index_flt0_trpp_sc_idx_byte32(mt_u8 data)
{
    reg_trpp_sc_index_flt0_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0));
    d.bitc.trpp_sc_idx_byte32 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0), d.all);
}

mt_u8   reg_get_trpp_sc_index_flt0_trpp_sc_idx_byte32(mt_void)
{
	reg_trpp_sc_index_flt0_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_sc_index_flt0));
    return d.bitc.trpp_sc_idx_byte32;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_esbuf_ch (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_esbuf_ch(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch), data);
}

mt_u32  reg_get_trpp_esbuf_ch(mt_void)
{
	reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    return d.all;
}

mt_void reg_set_trpp_mode(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_mode), data);
}

mt_u32  reg_get_trpp_mode(mt_void)
{
	reg_trpp_bus_urgent_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_mode));
    return d.all;
}

mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp0_chsel(mt_u8 data)
{
    reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    d.bitc.trpp_esbufwp0_chsel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch), d.all);
}

mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp0_chsel(mt_void)
{
	reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    return d.bitc.trpp_esbufwp0_chsel;
}

mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp1_chsel(mt_u8 data)
{
    reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    d.bitc.trpp_esbufwp1_chsel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch), d.all);
}

mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp1_chsel(mt_void)
{
	reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    return d.bitc.trpp_esbufwp1_chsel;
}

mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp2_chsel(mt_u8 data)
{
    reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    d.bitc.trpp_esbufwp2_chsel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch), d.all);
}

mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp2_chsel(mt_void)
{
	reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    return d.bitc.trpp_esbufwp2_chsel;
}

mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp3_chsel(mt_u8 data)
{
    reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    d.bitc.trpp_esbufwp3_chsel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch), d.all);
}

mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp3_chsel(mt_void)
{
	reg_trpp_esbuf_ch_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_esbuf_ch));
    return d.bitc.trpp_esbufwp3_chsel;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_property (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_property(mt_u8 index, mt_u32 data)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_u32 val = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));

    /* for irdeto solution: the audio channel was already initialized
     * in the auxcode, here we just skip it.
     */
    if(val == AUD_CH_REG_PRE_SET_VAL) {
        printk(KERN_DEBUG "do not init chan proterty becuase it's already initialize in auxcode\n");
        return;
    }
#endif

	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_property(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_property_trpp_ch_es_mode(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_es_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_es_mode(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_es_mode;
}

mt_void reg_set_trpp_ch_property_trpp_ch_pusi_detect(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_pusi_detect = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_pusi_detect(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_pusi_detect;
}

mt_void reg_set_trpp_ch_property_trpp_ch_pusi_mode(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_pusi_mode = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8 reg_get_trpp_ch_property_trpp_ch_pusi_mode(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_pusi_mode;
}

mt_void reg_set_trpp_ch_property_trpp_ch_strid_mod(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_strid_mod = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8 reg_get_trpp_ch_property_trpp_ch_strid_mod(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_strid_mod;
}

mt_void reg_set_trpp_ch_property_trpp_ch_pusi_mode2(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_pusi_mode2 = data;

    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8 reg_get_trpp_ch_property_trpp_ch_pusi_mode2(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_pusi_mode2;
}

mt_void reg_set_trpp_ch_property_trpp_ch_time_info(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_u32 val = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    
    /* for irdeto solution: the audio channel was already initialized to
     * in the auxcode, here we just skip it.
     */
    if(val == AUD_CH_REG_PRE_SET_VAL) {
        printk(KERN_DEBUG "do not set ch time info chan proterty becuase it's already initialize in auxcode\n");
        return;
    }
#endif

    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_time_info = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_time_info(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_time_info;
}

mt_void reg_set_trpp_ch_property_trpp_ch_dscrpt_en(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_dscrpt_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_dscrpt_en(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_dscrpt_en;
}

mt_void reg_set_trpp_ch_property_trpp_ch_pes_head_en(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_pes_head_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_pes_head_en(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_pes_head_en;
}

mt_void reg_set_trpp_ch_property_trpp_ch_fsc_en(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_fsc_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_fsc_en(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_fsc_en;
}

mt_void reg_set_trpp_ch_property_trpp_ch_stream_id(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_stream_id = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_stream_id(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_stream_id;
}

mt_void reg_set_trpp_ch_property_trpp_ch_str_id_msk(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    d.bitc.trpp_ch_str_id_msk = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_property_trpp_ch_str_id_msk(mt_u8 index)
{
	reg_trpp_ch_property_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_property + index * 0x100));
    return d.bitc.trpp_ch_str_id_msk;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_parse_set (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_parse_set(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_parse_set(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_parse_set_trpp_ch_fsc_cp_en(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    d.bitc.trpp_ch_fsc_cp_en = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_fsc_cp_en(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.bitc.trpp_ch_fsc_cp_en;
}

mt_void reg_set_trpp_ch_parse_set_trpp_ch_sh_detect(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    d.bitc.trpp_ch_sh_detect = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_sh_detect(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.bitc.trpp_ch_sh_detect;
}

mt_void reg_set_trpp_ch_parse_set_trpp_ch_sh_en(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    d.bitc.trpp_ch_sh_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_sh_en(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.bitc.trpp_ch_sh_en;
}

mt_void reg_set_trpp_ch_parse_set_trpp_ch_pes_len_mod(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    d.bitc.trpp_ch_pes_len_mod = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_pes_len_mod(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.bitc.trpp_ch_pes_len_mod;
}

mt_void reg_set_trpp_ch_parse_set_trpp_ch_fsc_nbytes(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    d.bitc.trpp_ch_fsc_nbytes = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_fsc_nbytes(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.bitc.trpp_ch_fsc_nbytes;
}

mt_void reg_set_trpp_ch_parse_set_trpp_ch_insrt_nbytes(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    d.bitc.trpp_ch_insrt_nbytes = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_insrt_nbytes(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.bitc.trpp_ch_insrt_nbytes;
}

mt_void reg_set_trpp_ch_parse_set_trpp_ch_int_nbytes(mt_u8 index, mt_u16 data)
{
    reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    d.bitc.trpp_ch_int_nbytes = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100), d.all);
}

mt_u16  reg_get_trpp_ch_parse_set_trpp_ch_int_nbytes(mt_u8 index)
{
	reg_trpp_ch_parse_set_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_parse_set + index * 0x100));
    return d.bitc.trpp_ch_int_nbytes;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_start_code1 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_start_code1(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_start_code1(mt_u8 index)
{
	reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_31(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    d.bitc.trpp_ch_fsc_31 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_31(mt_u8 index)
{
	reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    return d.bitc.trpp_ch_fsc_31;
}

mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_32(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    d.bitc.trpp_ch_fsc_32 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_32(mt_u8 index)
{
	reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    return d.bitc.trpp_ch_fsc_32;
}

mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_41(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    d.bitc.trpp_ch_fsc_41 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_41(mt_u8 index)
{
	reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    return d.bitc.trpp_ch_fsc_41;
}

mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_42(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    d.bitc.trpp_ch_fsc_42 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_42(mt_u8 index)
{
	reg_trpp_ch_start_code1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code1 + index * 0x100));
    return d.bitc.trpp_ch_fsc_42;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_frm_start_code_m1 (read/write)                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_frm_start_code_m1(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_frm_start_code_m1(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_31(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    d.bitc.trpp_ch_fscm_31 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_31(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    return d.bitc.trpp_ch_fscm_31;
}

mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_32(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    d.bitc.trpp_ch_fscm_32 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_32(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    return d.bitc.trpp_ch_fscm_32;
}

mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_41(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    d.bitc.trpp_ch_fscm_41 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_41(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    return d.bitc.trpp_ch_fscm_41;
}

mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_42(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    d.bitc.trpp_ch_fscm_42 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_42(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m1 + index * 0x100));
    return d.bitc.trpp_ch_fscm_42;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_start_code2 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_start_code2(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_start_code2(mt_u8 index)
{
	reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_51(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    d.bitc.trpp_ch_fsc_51 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_51(mt_u8 index)
{
	reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    return d.bitc.trpp_ch_fsc_51;
}

mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_52(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    d.bitc.trpp_ch_fsc_52 = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_52(mt_u8 index)
{
	reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    return d.bitc.trpp_ch_fsc_52;
}

mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_61(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    d.bitc.trpp_ch_fsc_61 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_61(mt_u8 index)
{
	reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    return d.bitc.trpp_ch_fsc_61;
}

mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_62(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    d.bitc.trpp_ch_fsc_62 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_62(mt_u8 index)
{
	reg_trpp_ch_start_code2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_start_code2 + index * 0x100));
    return d.bitc.trpp_ch_fsc_62;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_frm_start_code_m2 (read/write)                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_frm_start_code_m2(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_frm_start_code_m2(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m2_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_51(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    d.bitc.trpp_ch_fscm_51 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_51(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m2_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    return d.bitc.trpp_ch_fscm_51;
}

mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_52(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    d.bitc.trpp_ch_fscm_52 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_52(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m2_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    return d.bitc.trpp_ch_fscm_52;
}

mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_61(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    d.bitc.trpp_ch_fscm_61 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_61(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m2_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    return d.bitc.trpp_ch_fscm_61;
}

mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_62(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_frm_start_code_m2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    d.bitc.trpp_ch_fscm_62 = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_62(mt_u8 index)
{
	reg_trpp_ch_frm_start_code_m2_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_frm_start_code_m2 + index * 0x100));
    return d.bitc.trpp_ch_fscm_62;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_start_addr (read/write)                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_start_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_dscrpt_start_addr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_start_addr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_dscrpt_start_addr_trpp_ch_data_mem_th(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_dscrpt_start_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100));
    d.bitc.trpp_ch_data_mem_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_dscrpt_start_addr_trpp_ch_data_mem_th(mt_u8 index)
{
	reg_trpp_ch_dscrpt_start_addr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100));
    return d.bitc.trpp_ch_data_mem_th;
}

mt_void reg_set_trpp_ch_dscrpt_start_addr_trpp_ch_data_saddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_dscrpt_start_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100));
    d.bitc.trpp_ch_data_saddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100), d.all);	
}

mt_u32  reg_get_trpp_ch_dscrpt_start_addr_trpp_ch_data_saddr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_start_addr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_start_addr + index * 0x100));
    return d.bitc.trpp_ch_data_saddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_start_addr (read/write)                          */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_start_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_data_start_addr(mt_u8 index)
{
	reg_trpp_ch_data_start_addr_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_data_start_addr_trpp_ch_data_mem_th(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_data_start_addr_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100));
    d.bitc.trpp_ch_data_mem_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_data_start_addr_trpp_ch_data_mem_th(mt_u8 index)
{
	reg_trpp_ch_data_start_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100));
    return d.bitc.trpp_ch_data_mem_th;
}

mt_void reg_set_trpp_ch_data_start_addr_trpp_ch_data_saddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_data_start_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100));
    d.bitc.trpp_ch_data_saddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_data_start_addr_trpp_ch_data_saddr(mt_u8 index)
{
	reg_trpp_ch_data_start_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_start_addr + index * 0x100));
    return d.bitc.trpp_ch_data_saddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_end_addr (read/write)                          */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_end_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_end_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_dscrpt_end_addr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_end_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_end_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_dscrpt_end_addr_trpp_ch_data_eaddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_dscrpt_end_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_end_addr + index * 0x100));
    d.bitc.trpp_ch_data_eaddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_end_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_dscrpt_end_addr_trpp_ch_data_eaddr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_end_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_end_addr + index * 0x100));
    return d.bitc.trpp_ch_data_eaddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_end_addr (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_end_addr(mt_u8 index, mt_u32 data)
{    
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_end_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_data_end_addr(mt_u8 index)
{
	reg_trpp_ch_data_end_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_end_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_data_end_addr_trpp_ch_data_eaddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_data_end_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_end_addr + index * 0x100));
    d.bitc.trpp_ch_data_eaddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_end_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_data_end_addr_trpp_ch_data_eaddr(mt_u8 index)
{
	reg_trpp_ch_data_end_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_end_addr + index * 0x100));
    return d.bitc.trpp_ch_data_eaddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_rd_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_rd_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_rd_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_dscrpt_rd_addr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_rd_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_rd_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_dscrpt_rd_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_rd_addr + index * 0x100));
    d.bitc.trpp_ch_dscrpt_raddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_rd_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_rd_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_rd_addr + index * 0x100));
    return d.bitc.trpp_ch_dscrpt_raddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_rd_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_rd_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_rd_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_data_rd_addr(mt_u8 index)
{
	reg_trpp_ch_data_rd_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_rd_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_data_rd_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_rd_addr + index * 0x100));
    d.bitc.trpp_ch_data_raddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_rd_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_data_rd_addr_trpp_ch_data_raddr(mt_u8 index)
{
	reg_trpp_ch_data_rd_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_rd_addr + index * 0x100));
    return d.bitc.trpp_ch_data_raddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_wr_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_wr_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_wr_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_dscrpt_wr_addr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_wr_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_wr_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_dscrpt_wr_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_wr_addr + index * 0x100));
    d.bitc.trpp_ch_dscrpt_waddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_wr_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(mt_u8 index)
{
	reg_trpp_ch_dscrpt_wr_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_dscrpt_wr_addr + index * 0x100));
    return d.bitc.trpp_ch_dscrpt_waddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_wr_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_wr_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_wr_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_data_wr_addr(mt_u8 index)
{
	reg_trpp_ch_data_wr_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_wr_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_data_wr_addr_trpp_ch_data_waddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_data_wr_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_wr_addr + index * 0x100));
    d.bitc.trpp_ch_data_waddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_wr_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_data_wr_addr_trpp_ch_data_waddr(mt_u8 index)
{
	reg_trpp_ch_data_wr_addr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_data_wr_addr + index * 0x100));
    return d.bitc.trpp_ch_data_waddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info1 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info1(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info1(mt_u8 index)
{
	reg_trpp_ch1_ini_info1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info1_trpp_ch1_reserved(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch1_ini_info1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100));
    d.bitc.trpp_ch1_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch1_ini_info1_trpp_ch1_reserved(mt_u8 index)
{
	reg_trpp_ch1_ini_info1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100));
    return d.bitc.trpp_ch1_reserved;
}

mt_void reg_set_trpp_ch1_ini_info1_trpp_ch1_ini_flag_par(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch1_ini_info1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100));
    d.bitc.trpp_ch1_ini_flag_par = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch1_ini_info1_trpp_ch1_ini_flag_par(mt_u8 index)
{
	reg_trpp_ch1_ini_info1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info1 + index * 0x100));
    return d.bitc.trpp_ch1_ini_flag_par;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info2 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info2(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info2 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info2(mt_u8 index)
{
	reg_trpp_ch1_ini_info2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info2 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info2_trpp_ch1_dts(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch1_ini_info2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info2 + index * 0x100));
    d.bitc.trpp_ch1_dts = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info2 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch1_ini_info2_trpp_ch1_dts(mt_u8 index)
{
	reg_trpp_ch1_ini_info2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info2 + index * 0x100));
    return d.bitc.trpp_ch1_dts;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info3 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info3(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info3 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info3(mt_u8 index)
{
	reg_trpp_ch1_ini_info3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info3 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info3_trpp_ch1_pts(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch1_ini_info3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info3 + index * 0x100));
    d.bitc.trpp_ch1_pts = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info3 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch1_ini_info3_trpp_ch1_pts(mt_u8 index)
{
	reg_trpp_ch1_ini_info3_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info3 + index * 0x100));
    return d.bitc.trpp_ch1_pts;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info4 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info4(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info4 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info4(mt_u8 index)
{
	reg_trpp_ch1_ini_info4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info4 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info4_trpp_ch1_reserved(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch1_ini_info4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info4 + index * 0x100));
    d.bitc.trpp_ch1_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info4 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch1_ini_info4_trpp_ch1_reserved(mt_u8 index)
{
	reg_trpp_ch1_ini_info4_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info4 + index * 0x100));
    return d.bitc.trpp_ch1_reserved;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info5 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info5(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info5 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info5(mt_u8 index)
{
	reg_trpp_ch1_ini_info5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info5 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info5_trpp_ch1_reserved(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch1_ini_info5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info5 + index * 0x100));
    d.bitc.trpp_ch1_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info5 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch1_ini_info5_trpp_ch1_reserved(mt_u8 index)
{
	reg_trpp_ch1_ini_info5_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info5 + index * 0x100));
    return d.bitc.trpp_ch1_reserved;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info6 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info6(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info6 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info6(mt_u8 index)
{
	reg_trpp_ch1_ini_info6_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info6 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info6_trpp_ch1_frame_cnt(mt_u8 index, mt_u16 data)
{
    reg_trpp_ch1_ini_info6_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info6 + index * 0x100));
    d.bitc.trpp_ch1_frame_cnt = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info6 + index * 0x100), d.all);
}

mt_u16  reg_get_trpp_ch1_ini_info6_trpp_ch1_frame_cnt(mt_u8 index)
{
	reg_trpp_ch1_ini_info6_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info6 + index * 0x100));
    return d.bitc.trpp_ch1_frame_cnt;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info7 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info7(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info7(mt_u8 index)
{
	reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_reserved(mt_u8 index, mt_u16 data)
{
    reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    d.bitc.trpp_ch1_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100), d.all);
}

mt_u16  reg_get_trpp_ch1_ini_info7_trpp_ch1_reserved(mt_u8 index)
{
	reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    return d.bitc.trpp_ch1_reserved;
}

mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_chnum(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    d.bitc.trpp_ch1_chnum = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_chnum(mt_u8 index)
{
	reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    return d.bitc.trpp_ch1_chnum;
}

mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_gotstrid(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    d.bitc.trpp_ch1_gotstrid = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_gotstrid(mt_u8 index)
{
	reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    return d.bitc.trpp_ch1_gotstrid;
}

mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_discard_flag(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    d.bitc.trpp_ch1_discard_flag = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_discard_flag(mt_u8 index)
{
	reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    return d.bitc.trpp_ch1_discard_flag;
}

mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_reserve(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    d.bitc.trpp_ch1_reserve = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_reserve(mt_u8 index)
{
	reg_trpp_ch1_ini_info7_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info7 + index * 0x100));
    return d.bitc.trpp_ch1_reserve;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info8 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info8(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info8 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info8(mt_u8 index)
{
	reg_trpp_ch1_ini_info8_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info8 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info8_trpp_ch1_reserved(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch1_ini_info8_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info8 + index * 0x100));
    d.bitc.trpp_ch1_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info8 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch1_ini_info8_trpp_ch1_reserved(mt_u8 index)
{
	reg_trpp_ch1_ini_info8_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info8 + index * 0x100));
    return d.bitc.trpp_ch1_reserved;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info9 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info9(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch1_ini_info9(mt_u8 index)
{
	reg_trpp_ch1_ini_info9_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch1_ini_info9_trpp_ch1_pes_data_cnt(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch1_ini_info9_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100));
    d.bitc.trpp_ch1_pes_data_cnt = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch1_ini_info9_trpp_ch1_pes_data_cnt(mt_u8 index)
{
	reg_trpp_ch1_ini_info9_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100));
    return d.bitc.trpp_ch1_pes_data_cnt;
}

mt_void reg_set_trpp_ch1_ini_info9_trpp_ch1_reserved(mt_u8 index, mt_u16 data)
{
    reg_trpp_ch1_ini_info9_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100));
    d.bitc.trpp_ch1_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100), d.all);
}

mt_u16  reg_get_trpp_ch1_ini_info9_trpp_ch1_reserved(mt_u8 index)
{
	reg_trpp_ch1_ini_info9_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch1_ini_info9 + index * 0x100));
    return d.bitc.trpp_ch1_reserved;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_set (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_set(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_rec_set(mt_u8 index)
{
	reg_trpp_ch_rec_set_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_rec_set_trpp_ch_rec_cnt_th(mt_u8 index, mt_u16 data)
{
    reg_trpp_ch_rec_set_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100));
    d.bitc.trpp_ch_rec_cnt_th = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100), d.all);
}

mt_u16  reg_get_trpp_ch_rec_set_trpp_ch_rec_cnt_th(mt_u8 index)
{
	reg_trpp_ch_rec_set_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100));
    return d.bitc.trpp_ch_rec_cnt_th;
}

mt_void reg_set_trpp_ch_rec_set_trpp_ch_rec_sel(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_rec_set_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100));
    d.bitc.trpp_ch_rec_sel = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_rec_set_trpp_ch_rec_sel(mt_u8 index)
{
	reg_trpp_ch_rec_set_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100));
    return d.bitc.trpp_ch_rec_sel;
}

mt_void reg_set_trpp_ch_rec_set_trpp_ch_rec_mod(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_rec_set_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100));
    d.bitc.trpp_ch_rec_mod = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_rec_set_trpp_ch_rec_mod(mt_u8 index)
{
	reg_trpp_ch_rec_set_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_set + index * 0x100));
    return d.bitc.trpp_ch_rec_mod;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_start_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_start_addr(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_rec_start_addr(mt_u8 index)
{
	reg_trpp_ch_rec_start_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_rec_start_addr_trpp_ch_rec_mem_th(mt_u8 index, mt_u8 data)
{
    reg_trpp_ch_rec_start_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100));
    d.bitc.trpp_ch_rec_mem_th = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100), d.all);
}

mt_u8   reg_get_trpp_ch_rec_start_addr_trpp_ch_rec_mem_th(mt_u8 index)
{
	reg_trpp_ch_rec_start_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100));
    return d.bitc.trpp_ch_rec_mem_th;
}

mt_void reg_set_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_rec_start_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100));
    d.bitc.trpp_ch_rec_saddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(mt_u8 index)
{
	reg_trpp_ch_rec_start_addr_t d;
	
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_start_addr + index * 0x100));
    return d.bitc.trpp_ch_rec_saddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_end_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_end_addr(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_end_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_rec_end_addr(mt_u8 index)
{
	reg_trpp_ch_rec_end_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_end_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_rec_end_addr_trpp_ch_rec_eaddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_rec_end_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_end_addr + index * 0x100));
    d.bitc.trpp_ch_rec_eaddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_end_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_rec_end_addr_trpp_ch_rec_eaddr(mt_u8 index)
{
	reg_trpp_ch_rec_end_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_end_addr + index * 0x100));
    return d.bitc.trpp_ch_rec_eaddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_rd_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_rd_addr(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_rd_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_rec_rd_addr(mt_u8 index)
{
	reg_trpp_ch_rec_rd_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_rd_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_rec_rd_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_rd_addr + index * 0x100));
    d.bitc.trpp_ch_rec_raddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_rd_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(mt_u8 index)
{
	reg_trpp_ch_rec_rd_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_rd_addr + index * 0x100));
    return d.bitc.trpp_ch_rec_raddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_wr_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_wr_addr(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_wr_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_rec_wr_addr(mt_u8 index)
{
	reg_trpp_ch_rec_wr_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_wr_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_rec_wr_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_wr_addr + index * 0x100));
    d.bitc.trpp_ch_rec_waddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_wr_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(mt_u8 index)
{
	reg_trpp_ch_rec_wr_addr_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_rec_wr_addr + index * 0x100));
    return d.bitc.trpp_ch_rec_waddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch11_ini_info1 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_ini_info1(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_ini_info1(mt_u8 index)
{
	reg_trpp_ch11_ini_info1_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info1 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch11_ini_info1_trpp_ch11_reserved(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch11_ini_info1_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info1 + index * 0x100));
    d.bitc.trpp_ch11_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info1 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch11_ini_info1_trpp_ch11_reserved(mt_u8 index)
{
	reg_trpp_ch11_ini_info1_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info1 + index * 0x100));
    return d.bitc.trpp_ch11_reserved;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch11_ini_info2 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_ini_info2(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info2 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_ini_info2(mt_u8 index)
{
	reg_trpp_ch11_ini_info2_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0; 
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info2 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch11_ini_info2_trpp_ch11_reserved(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch11_ini_info2_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info2 + index * 0x100));
    d.bitc.trpp_ch11_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info2 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch11_ini_info2_trpp_ch11_reserved(mt_u8 index)
{
	reg_trpp_ch11_ini_info2_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info2 + index * 0x100));
    return d.bitc.trpp_ch11_reserved;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch11_ini_info3 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_ini_info3(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info3 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_ini_info3(mt_u8 index)
{
	reg_trpp_ch11_ini_info3_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info3 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch11_ini_info3_trpp_ch11_reserved(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch11_ini_info3_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info3 + index * 0x100));
    d.bitc.trpp_ch11_reserved = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info3 + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch11_ini_info3_trpp_ch11_reserved(mt_u8 index)
{
	reg_trpp_ch11_ini_info3_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_ini_info3 + index * 0x100));
    return d.bitc.trpp_ch11_reserved;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_ts_sn (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_ts_sn(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_ts_sn + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_ts_sn(mt_u8 index)
{
	reg_trpp_ch_ts_sn_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_ts_sn + index * 0x100));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx record link mode (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_lln_set0(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set0 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_lln_set0(mt_u8 index)
{
	reg_trpp_ch11_lln_set0_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set0 + index * 0x100));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx link node register (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_lln_set1(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_lln_set1(mt_u8 index)
{
	reg_trpp_ch11_lln_set1_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set1 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch11_lln_set2(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set2 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_lln_set2(mt_u8 index)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set2 + index * 0x100));
}

mt_void reg_set_trpp_ch11_lln_set3(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set3 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_lln_set3(mt_u8 index)
{
	reg_trpp_ch11_lln_set3_t d;

	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_set3 + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch11_lln_idx_set0(mt_u8 rec_index, mt_u8 id_index, mt_u32 data)
{
	if (rec_index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		rec_index += 8;
	#else
		return 0;
	#endif
	}
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_id0_set0 + rec_index * 0x100 + id_index*8), data);
}

mt_u32  reg_get_trpp_ch11_lln_idx_set0(mt_u8 rec_index, mt_u8 id_index)
{
	reg_trpp_ch11_lln_idx_set0_t d;

	if (rec_index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		rec_index += 8;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_id0_set0 + rec_index * 0x100 + id_index*8));
    return d.all;
}

mt_void reg_set_trpp_ch11_lln_idx_set1(mt_u8 rec_index, mt_u8 id_index, mt_u32 data)
{
	if (rec_index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		rec_index += 8;
	#else
		return 0;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_id0_set1 + rec_index * 0x100 + id_index*8), data);
}

mt_u32  reg_get_trpp_ch11_lln_idx_set1(mt_u8 rec_index, mt_u8 id_index)
{
	reg_trpp_ch11_lln_idx_set1_t d;
	
	if (rec_index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		rec_index += 8;
	#else
		return 0;
	#endif
	}
		
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_id0_set1 + rec_index * 0x100 + id_index*8));
    return d.all;
}

mt_void reg_set_trpp_ch11_lln_full_0(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_full_0 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_lln_full_0(mt_u8 index)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_full_0 + index * 0x100));
}

mt_void reg_set_trpp_ch11_lln_full_1(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_full_1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_lln_full_1(mt_u8 index)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_full_1 + index * 0x100));
}

mt_void reg_set_trpp_ch11_lln_rd_byte_set(mt_u8 index, mt_u32 data)
{
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_rd_byte_set + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch11_lln_data_byte_num(mt_u8 index)
{	
	if (index >= 4)//the base addr is 0xbf261d00
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 8;
	#else
		return 0;
	#endif
	}
	
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch11_lln_data_byte_num + index * 0x100));
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_mode (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_mode(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_mode + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_idx_mode(mt_u8 index)
{
	reg_trpp_ch_idx_mode_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_mode + index * 0x100));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_enable (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_enable(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_enable + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_idx_enable(mt_u8 index)
{
	reg_trpp_ch_idx_mode_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_enable + index * 0x100));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_start_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_start_addr(mt_u8 index, mt_u32 data)
{	
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_start_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_idx_start_addr(mt_u8 index)
{
	reg_trpp_ch_idx_start_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_start_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_idx_start_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_start_addr + index * 0x100));
    d.bitc.trpp_ch_idx_saddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_start_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(mt_u8 index)
{
	reg_trpp_ch_idx_start_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_start_addr + index * 0x100));
    return d.bitc.trpp_ch_idx_saddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_end_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_end_addr(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_end_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_idx_end_addr(mt_u8 index)
{
	reg_trpp_ch_idx_end_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_end_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_idx_end_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_end_addr + index * 0x100));
    d.bitc.trpp_ch_idx_eaddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_end_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(mt_u8 index)
{
	reg_trpp_ch_idx_end_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_end_addr + index * 0x100));
    return d.bitc.trpp_ch_idx_eaddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_rd_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_rd_addr(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_rd_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_idx_rd_addr(mt_u8 index)
{
	reg_trpp_ch_idx_rd_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_rd_addr + index * 0x100));
    return d.all;
}

mt_void reg_set_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_idx_rd_addr_t d;
	
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}	
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_rd_addr + index * 0x100));
    d.bitc.trpp_ch_idx_raddr = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_rd_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(mt_u8 index)
{
	reg_trpp_ch_idx_rd_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_rd_addr + index * 0x100));
    return d.bitc.trpp_ch_idx_raddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_wr_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_wr_addr(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_wr_addr + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch_idx_wr_addr(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_wr_addr + index * 0x100));
}

mt_void reg_set_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(mt_u8 index, mt_u32 data)
{
    reg_trpp_ch_idx_wr_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_wr_addr + index * 0x100));
    d.bitc.trpp_ch_idx_waddr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_wr_addr + index * 0x100), d.all);
}

mt_u32  reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(mt_u8 index)
{
	reg_trpp_ch_idx_wr_addr_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch_idx_wr_addr + index * 0x100));
    return d.bitc.trpp_ch_idx_waddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info1 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info1(mt_u8 index, mt_u32 data)
{    
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_ini_info1(mt_u8 index)
{
	reg_trpp_ch15_ini_info1_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info1 + index * 0x100));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info2 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info2(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info2 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_ini_info2(mt_u8 index)
{
	reg_trpp_ch15_ini_info2_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info2 + index * 0x100));
	return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info3 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info3(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info3 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_ini_info3(mt_u8 index)
{
	reg_trpp_ch15_ini_info3_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info3 + index * 0x100));
	return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info4 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info4(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info4 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_ini_info4(mt_u8 index)
{
	reg_trpp_ch15_ini_info4_t d;

	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
		
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info4 + index * 0x100));
    return d.all;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info 5-7 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info5(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info5 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_ini_info5(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info5 + index * 0x100));
}

mt_void reg_set_trpp_ch15_ini_info6(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info6 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_ini_info6(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info6 + index * 0x100));
}

mt_void reg_set_trpp_ch15_ini_info7(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info7 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_ini_info7(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_ini_info7 + index * 0x100));
}

/*----------------------------------------------------------------------------*/
/* register reg_dmx_trpp_ch15_sc_flt_set0~4 (read/write)                                      */
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_void reg_set_trpp_ch15_sc_flt_set0(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	} 
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set0 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_sc_flt_set0(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set0 + index * 0x100));
}

mt_void reg_set_trpp_ch15_sc_flt_set1(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set1 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_sc_flt_set1(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set1 + index * 0x100));
}

mt_void reg_set_trpp_ch15_sc_flt_set2(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set2 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_sc_flt_set2(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set2 + index * 0x100));
}

mt_void reg_set_trpp_ch15_sc_flt_set3(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set3 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_sc_flt_set3(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set3 + index * 0x100));
}

mt_void reg_set_trpp_ch15_sc_flt_set4(mt_u8 index, mt_u32 data)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return;
	#endif
	}
	
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set4 + index * 0x100), data);
}

mt_u32  reg_get_trpp_ch15_sc_flt_set4(mt_u8 index)
{
	if (index >= 8)//the base addr is 0xbf262000
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		index += 3;
	#else
		return 0;
	#endif
	}
	
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_ch15_sc_flt_set4 + index * 0x100));
}

#endif

/*---------------------------------------------------------*/
/* register    ts sample interrupt function                */
/*---------------------------------------------------------*/
mt_void reg_set_ts_sample_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts_sample_int_mask), data);
}

mt_u32  reg_get_ts_sample_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts_sample_int_mask));
}

mt_void reg_set_ts_sample_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts_sample_int_edge), data);
}

mt_u32  reg_get_ts_sample_int_edge(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts_sample_int_edge));
}

mt_void reg_set_ts_sample_int_clr(mt_u32 data)
{
  	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts_sample_int_clr), data);
}

mt_u32 reg_get_ts_sample_int_clr(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts_sample_int_clr));
}

mt_u32 reg_get_ts_sample_int_state(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts_sample_int_state));
}

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_mask (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), data);
}

mt_u32  reg_get_ds_int_mask(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
	return d.all;
}

mt_void reg_set_ds_int_mask_cw_unvld_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.cw_unvld_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);	
}

mt_u8   reg_get_ds_int_mask_cw_unvld_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.cw_unvld_im;
}

mt_void reg_set_ds_int_mask_ts_err1_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.ts_err1_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_ts_err1_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.ts_err1_im;
}

mt_void reg_set_ds_int_mask_pes_err1_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.pes_err1_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_pes_err1_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.pes_err1_im;
}

mt_void reg_set_ds_int_mask_pes_err2_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.pes_err2_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_pes_err2_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.pes_err2_im;
}

mt_void reg_set_ds_int_mask_pes_err3_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.pes_err3_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_pes_err3_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.pes_err3_im;
}

mt_void reg_set_ds_int_mask_pes_err4_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.pes_err4_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_pes_err4_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.pes_err4_im;
}

mt_void reg_set_ds_int_mask_pes_err5_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.pes_err5_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_pes_err5_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.pes_err5_im;
}

mt_void reg_set_ds_int_mask_pes_err6_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.pes_err6_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_pes_err6_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.pes_err6_im;
}

mt_void reg_set_ds_int_mask_pes_err7_im(mt_u8 data)
{
    reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    d.bitc.pes_err7_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask), d.all);
}

mt_u8   reg_get_ds_int_mask_pes_err7_im(mt_void)
{
	reg_ds_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_mask));
    return d.bitc.pes_err7_im;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_edge (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), data);
}

mt_u32  reg_get_ds_int_edge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.all;
}

mt_void reg_set_ds_int_edge_cw_unvld_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.cw_unvld_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_cw_unvld_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.cw_unvld_iedge;
}

mt_void reg_set_ds_int_edge_ts_err1_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.ts_err1_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_ts_err1_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.ts_err1_iedge;
}

mt_void reg_set_ds_int_edge_pes_err1_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.pes_err1_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);	
}

mt_u8   reg_get_ds_int_edge_pes_err1_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.pes_err1_iedge;
}

mt_void reg_set_ds_int_edge_pes_err2_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.pes_err2_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_pes_err2_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.pes_err2_iedge;
}

mt_void reg_set_ds_int_edge_pes_err3_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.pes_err3_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_pes_err3_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.pes_err3_iedge;
}

mt_void reg_set_ds_int_edge_pes_err4_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.pes_err4_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_pes_err4_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.pes_err4_iedge;
}

mt_void reg_set_ds_int_edge_pes_err5_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.pes_err5_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_pes_err5_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.pes_err5_iedge;
}

mt_void reg_set_ds_int_edge_pes_err6_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.pes_err6_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_pes_err6_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.pes_err6_iedge;
}

mt_void reg_set_ds_int_edge_pes_err7_iedge(mt_u8 data)
{
    reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    d.bitc.pes_err7_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge), d.all);
}

mt_u8   reg_get_ds_int_edge_pes_err7_iedge(mt_void)
{
	reg_ds_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_edge));
    return d.bitc.pes_err7_iedge;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_clr (read/write)                                       */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_clr(mt_u32 data)
{
	 HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr), data);
}

mt_u32  reg_get_ds_int_clr(mt_void)
{
	reg_ds_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr));
    return d.all;
}

mt_void reg_set_ds_int_clr_ds_int_clr(mt_u16 data)
{
    reg_ds_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr));
    d.bitc.ds_int_clr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr), d.all);
}

mt_u16  reg_get_ds_int_clr_ds_int_clr(mt_void)
{
	reg_ds_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr));
    return d.bitc.ds_int_clr;
}

mt_void reg_set_ds_int_clr_ds_int_rd(mt_u8 data)
{
    reg_ds_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr));
    d.bitc.ds_int_rd = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr), d.all);
}

mt_u8   reg_get_ds_int_clr_ds_int_rd(mt_void)
{
	reg_ds_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_clr));
    return d.bitc.ds_int_rd;
}

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_state (read/write)                                     */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_state(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), data);
}

mt_u32  reg_get_ds_int_state(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.all;
}

mt_void reg_set_ds_int_state_cw_unvld_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.cw_unvld_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);	
}

mt_u8   reg_get_ds_int_state_cw_unvld_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.cw_unvld_ista;
}

mt_void reg_set_ds_int_state_ts_err1_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.ts_err1_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_ts_err1_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.ts_err1_ista;
}

mt_void reg_set_ds_int_state_pes_err1_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.pes_err1_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_pes_err1_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.pes_err1_ista;
}

mt_void reg_set_ds_int_state_pes_err2_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.pes_err2_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_pes_err2_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.pes_err2_ista;
}

mt_void reg_set_ds_int_state_pes_err3_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.pes_err3_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_pes_err3_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.pes_err3_ista;
}

mt_void reg_set_ds_int_state_pes_err4_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.pes_err4_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_pes_err4_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.pes_err4_ista;
}

mt_void reg_set_ds_int_state_pes_err5_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.pes_err5_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_pes_err5_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.pes_err5_ista;
}

mt_void reg_set_ds_int_state_pes_err6_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.pes_err6_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_pes_err6_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.pes_err6_ista;
}

mt_void reg_set_ds_int_state_pes_err7_ista(mt_u8 data)
{
    reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    d.bitc.pes_err7_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ds_int_state), d.all);
}

mt_u8   reg_get_ds_int_state_pes_err7_ista(mt_void)
{
	reg_ds_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ds_int_state));
    return d.bitc.pes_err7_ista;
}

mt_void reg_set_trpp_int_mask(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask + index*0x0c), data);
}

mt_u32  reg_get_trpp_int_mask(mt_u8 index)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask + index*0x0c));
}

mt_void reg_set_trpp_int_edge(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge + index*0x0c), data);
}

mt_u32  reg_get_trpp_int_edge(mt_u8 index)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge + index*0x0c));
}

mt_void reg_set_trpp_int_clr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr + index*0x0c), data);
}

mt_u32  reg_get_trpp_int_clr(mt_u8 index)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr + index*0x0c));
}

mt_void reg_set_trpp_int_state(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state + index*0x0c), data);
}

mt_u32  reg_get_trpp_int_state(mt_u8 index)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state + index*0x0c));
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_mask (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), data);
}

mt_u32  reg_get_trpp0_int_mask(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.all;
}

mt_void reg_set_trpp0_int_mask_ch0_crc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch0_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch0_crc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch0_crc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch0_str_id_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch0_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch0_str_id_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch0_str_id_err_im;
}

mt_void reg_set_trpp0_int_mask_ch0_pes_wr_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch0_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch0_pes_wr_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch0_pes_wr_err_im;
}

mt_void reg_set_trpp0_int_mask_ch0_ts_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch0_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch0_ts_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch0_ts_err_im;
}

mt_void reg_set_trpp0_int_mask_ch0_pes_sc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch0_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch0_pes_sc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch0_pes_sc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch0_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch0_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch0_pes_data_cnt_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch0_pes_data_cnt_im;
}

mt_void reg_set_trpp0_int_mask_ch1_crc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch1_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch1_crc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch1_crc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch1_str_id_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch1_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch1_str_id_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch1_str_id_err_im;
}

mt_void reg_set_trpp0_int_mask_ch1_pes_wr_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch1_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch1_pes_wr_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch1_pes_wr_err_im;
}

mt_void reg_set_trpp0_int_mask_ch1_ts_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch1_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch1_ts_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch1_ts_err_im;
}

mt_void reg_set_trpp0_int_mask_ch1_pes_sc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch1_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch1_pes_sc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch1_pes_sc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch1_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch1_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch1_pes_data_cnt_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch1_pes_data_cnt_im;
}

mt_void reg_set_trpp0_int_mask_ch2_crc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch2_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch2_crc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch2_crc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch2_str_id_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch2_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch2_str_id_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch2_str_id_err_im;
}

mt_void reg_set_trpp0_int_mask_ch2_pes_wr_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch2_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch2_pes_wr_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch2_pes_wr_err_im;
}

mt_void reg_set_trpp0_int_mask_ch2_ts_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch2_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch2_ts_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch2_ts_err_im;
}

mt_void reg_set_trpp0_int_mask_ch2_pes_sc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch2_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch2_pes_sc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch2_pes_sc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch2_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch2_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch2_pes_data_cnt_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch2_pes_data_cnt_im;
}

mt_void reg_set_trpp0_int_mask_ch3_crc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch3_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch3_crc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch3_crc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch3_str_id_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch3_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch3_str_id_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch3_str_id_err_im;
}

mt_void reg_set_trpp0_int_mask_ch3_pes_wr_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch3_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch3_pes_wr_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch3_pes_wr_err_im;
}

mt_void reg_set_trpp0_int_mask_ch3_ts_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch3_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch3_ts_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch3_ts_err_im;
}

mt_void reg_set_trpp0_int_mask_ch3_pes_sc_err_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch3_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch3_pes_sc_err_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch3_pes_sc_err_im;
}

mt_void reg_set_trpp0_int_mask_ch3_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    d.bitc.ch3_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask), d.all);
}

mt_u8   reg_get_trpp0_int_mask_ch3_pes_data_cnt_im(mt_void)
{
	reg_trpp0_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_mask));
    return d.bitc.ch3_pes_data_cnt_im;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_edge (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), data);
}

mt_u32  reg_get_trpp0_int_edge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.all;
}

mt_void reg_set_trpp0_int_edge_ch0_crc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch0_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch0_crc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch0_crc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch0_str_id_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch0_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch0_str_id_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch0_str_id_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch0_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch0_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch0_pes_wr_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch0_pes_wr_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch0_ts_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch0_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch0_ts_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch0_ts_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch0_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch0_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch0_pes_sc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch0_pes_sc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch0_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch0_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch0_pes_data_cnt_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch0_pes_data_cnt_iedge;
}

mt_void reg_set_trpp0_int_edge_ch1_crc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch1_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch1_crc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch1_crc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch1_str_id_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch1_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch1_str_id_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch1_str_id_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch1_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch1_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch1_pes_wr_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch1_pes_wr_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch1_ts_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch1_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch1_ts_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch1_ts_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch1_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch1_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch1_pes_sc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch1_pes_sc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch1_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch1_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch1_pes_data_cnt_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch1_pes_data_cnt_iedge;
}

mt_void reg_set_trpp0_int_edge_ch2_crc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch2_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch2_crc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch2_crc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch2_str_id_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch2_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch2_str_id_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch2_str_id_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch2_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch2_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch2_pes_wr_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch2_pes_wr_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch2_ts_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch2_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch2_ts_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch2_ts_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch2_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch2_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch2_pes_sc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch2_pes_sc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch2_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch2_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch2_pes_data_cnt_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch2_pes_data_cnt_iedge;
}

mt_void reg_set_trpp0_int_edge_ch3_crc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch3_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch3_crc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch3_crc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch3_str_id_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch3_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch3_str_id_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch3_str_id_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch3_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch3_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch3_pes_wr_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch3_pes_wr_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch3_ts_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch3_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch3_ts_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch3_ts_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch3_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch3_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch3_pes_sc_err_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch3_pes_sc_err_iedge;
}

mt_void reg_set_trpp0_int_edge_ch3_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp0_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    d.bitc.ch3_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge), d.all);
}

mt_u8   reg_get_trpp0_int_edge_ch3_pes_data_cnt_iedge(mt_void)
{
	reg_trpp0_int_edge_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_edge));
    return d.bitc.ch3_pes_data_cnt_iedge;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_clr (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr), data);
}

mt_u32  reg_get_trpp0_int_clr(mt_void)
{
	reg_trpp0_int_clr_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr));
    return d.all;
}

mt_void reg_set_trpp0_int_clr_trpp0_int_clr(mt_u32 data)
{
    reg_trpp0_int_clr_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr));
    d.bitc.trpp0_int_clr = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr), d.all);
}

mt_u32  reg_get_trpp0_int_clr_trpp0_int_clr(mt_void)
{
	reg_trpp0_int_clr_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr));
    return d.bitc.trpp0_int_clr;
}

mt_void reg_set_trpp0_int_clr_trpp0_int_rd(mt_u8 data)
{
    reg_trpp0_int_clr_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr));
    d.bitc.trpp0_int_rd = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr), d.all);	
}

mt_u8   reg_get_trpp0_int_clr_trpp0_int_rd(mt_void)
{
	reg_trpp0_int_clr_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_clr));
    return d.bitc.trpp0_int_rd;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_state (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_state(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), data);
}

mt_u32  reg_get_trpp0_int_state(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.all;
}

mt_void reg_set_trpp0_int_state_ch0_crc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch0_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch0_crc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch0_crc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch0_str_id_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch0_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch0_str_id_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch0_str_id_err_ista;
}

mt_void reg_set_trpp0_int_state_ch0_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch0_pes_wr_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch0_pes_wr_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch0_pes_wr_err_ista;
}

mt_void reg_set_trpp0_int_state_ch0_ts_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch0_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch0_ts_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch0_ts_err_ista;
}

mt_void reg_set_trpp0_int_state_ch0_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch0_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch0_pes_sc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch0_pes_sc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch0_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch0_pes_data_cnt_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch0_pes_data_cnt_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch0_pes_data_cnt_ista;
}

mt_void reg_set_trpp0_int_state_ch1_crc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch1_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch1_crc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch1_crc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch1_str_id_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch1_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch1_str_id_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch1_str_id_err_ista;
}

mt_void reg_set_trpp0_int_state_ch1_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch1_pes_wr_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch1_pes_wr_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch1_pes_wr_err_ista;
}

mt_void reg_set_trpp0_int_state_ch1_ts_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch1_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch1_ts_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch1_ts_err_ista;
}

mt_void reg_set_trpp0_int_state_ch1_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch1_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch1_pes_sc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch1_pes_sc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch1_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch1_pes_data_cnt_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch1_pes_data_cnt_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch1_pes_data_cnt_ista;
}

mt_void reg_set_trpp0_int_state_ch2_crc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch2_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch2_crc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch2_crc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch2_str_id_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch2_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch2_str_id_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch2_str_id_err_ista;
}

mt_void reg_set_trpp0_int_state_ch2_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch2_pes_wr_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch2_pes_wr_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch2_pes_wr_err_ista;
}

mt_void reg_set_trpp0_int_state_ch2_ts_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch2_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch2_ts_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch2_ts_err_ista;
}

mt_void reg_set_trpp0_int_state_ch2_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch2_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch2_pes_sc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch2_pes_sc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch2_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch2_pes_data_cnt_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch2_pes_data_cnt_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch2_pes_data_cnt_ista;
}

mt_void reg_set_trpp0_int_state_ch3_crc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch3_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch3_crc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch3_crc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch3_str_id_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch3_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch3_str_id_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch3_str_id_err_ista;
}

mt_void reg_set_trpp0_int_state_ch3_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch3_pes_wr_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch3_pes_wr_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch3_pes_wr_err_ista;
}

mt_void reg_set_trpp0_int_state_ch3_ts_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch3_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);

}

mt_u8   reg_get_trpp0_int_state_ch3_ts_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch3_ts_err_ista;
}

mt_void reg_set_trpp0_int_state_ch3_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch3_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch3_pes_sc_err_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch3_pes_sc_err_ista;
}

mt_void reg_set_trpp0_int_state_ch3_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp0_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    d.bitc.ch3_pes_data_cnt_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state), d.all);
}

mt_u8   reg_get_trpp0_int_state_ch3_pes_data_cnt_ista(mt_void)
{
	reg_trpp0_int_state_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp0_int_state));
    return d.bitc.ch3_pes_data_cnt_ista;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_mask (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), data);
}

mt_u32  reg_get_trpp1_int_mask(mt_void)
{
	reg_trpp1_int_mask_t d;	
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.all;
}

mt_void reg_set_trpp1_int_mask_ch4_crc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch4_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);	
}

mt_u8   reg_get_trpp1_int_mask_ch4_crc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch4_crc_err_im;
}

mt_void reg_set_trpp1_int_mask_ch4_str_id_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch4_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch4_str_id_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch4_str_id_err_im;
}
mt_void reg_set_trpp1_int_mask_ch4_pes_wr_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch4_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch4_pes_wr_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch4_pes_wr_err_im;
}

mt_void reg_set_trpp1_int_mask_ch4_ts_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch4_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch4_ts_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch4_ts_err_im;
}

mt_void reg_set_trpp1_int_mask_ch4_pes_sc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch4_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch4_pes_sc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch4_pes_sc_err_im;
}

mt_void reg_set_trpp1_int_mask_ch4_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch4_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch4_pes_data_cnt_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch4_pes_data_cnt_im;
}

mt_void reg_set_trpp1_int_mask_ch5_crc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch5_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch5_crc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch5_crc_err_im;
}
mt_void reg_set_trpp1_int_mask_ch5_str_id_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch5_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch5_str_id_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch5_str_id_err_im;
}

mt_void reg_set_trpp1_int_mask_ch5_pes_wr_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch5_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch5_pes_wr_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch5_pes_wr_err_im;
}

mt_void reg_set_trpp1_int_mask_ch5_ts_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch5_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch5_ts_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch5_ts_err_im;
}

mt_void reg_set_trpp1_int_mask_ch5_pes_sc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch5_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch5_pes_sc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch5_pes_sc_err_im;
}

mt_void reg_set_trpp1_int_mask_ch5_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch5_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch5_pes_data_cnt_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch5_pes_data_cnt_im;
}

mt_void reg_set_trpp1_int_mask_ch6_crc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch6_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch6_crc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch6_crc_err_im;
}

mt_void reg_set_trpp1_int_mask_ch6_str_id_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch6_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch6_str_id_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch6_str_id_err_im;
}

mt_void reg_set_trpp1_int_mask_ch6_pes_wr_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch6_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch6_pes_wr_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch6_pes_wr_err_im;
}

mt_void reg_set_trpp1_int_mask_ch6_ts_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch6_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch6_ts_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch6_ts_err_im;
}

mt_void reg_set_trpp1_int_mask_ch6_pes_sc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch6_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch6_pes_sc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch6_pes_sc_err_im;
}

mt_void reg_set_trpp1_int_mask_ch6_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch6_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch6_pes_data_cnt_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch6_pes_data_cnt_im;
}

mt_void reg_set_trpp1_int_mask_ch7_crc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch7_crc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch7_crc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch7_crc_err_im;
}

mt_void reg_set_trpp1_int_mask_ch7_str_id_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch7_str_id_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch7_str_id_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch7_str_id_err_im;
}

mt_void reg_set_trpp1_int_mask_ch7_pes_wr_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch7_pes_wr_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch7_pes_wr_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch7_pes_wr_err_im;
}

mt_void reg_set_trpp1_int_mask_ch7_ts_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch7_ts_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch7_ts_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch7_ts_err_im;
}

mt_void reg_set_trpp1_int_mask_ch7_pes_sc_err_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch7_pes_sc_err_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch7_pes_sc_err_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch7_pes_sc_err_im;
}

mt_void reg_set_trpp1_int_mask_ch7_pes_data_cnt_im(mt_u8 data)
{
    reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    d.bitc.ch7_pes_data_cnt_im = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask), d.all);
}

mt_u8   reg_get_trpp1_int_mask_ch7_pes_data_cnt_im(mt_void)
{
	reg_trpp1_int_mask_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_mask));
    return d.bitc.ch7_pes_data_cnt_im;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_edge (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), data);
}

mt_u32  reg_get_trpp1_int_edge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.all;
}

mt_void reg_set_trpp1_int_edge_ch4_crc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch4_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch4_crc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch4_crc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch4_str_id_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch4_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch4_str_id_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch4_str_id_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch4_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch4_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch4_pes_wr_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch4_pes_wr_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch4_ts_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch4_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch4_ts_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch4_ts_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch4_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch4_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch4_pes_sc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch4_pes_sc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch4_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch4_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch4_pes_data_cnt_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch4_pes_data_cnt_iedge;
}

mt_void reg_set_trpp1_int_edge_ch5_crc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch5_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch5_crc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch5_crc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch5_str_id_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch5_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch5_str_id_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch5_str_id_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch5_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch5_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch5_pes_wr_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch5_pes_wr_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch5_ts_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch5_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch5_ts_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch5_ts_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch5_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch5_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch5_pes_sc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch5_pes_sc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch5_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch5_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch5_pes_data_cnt_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch5_pes_data_cnt_iedge;
}

mt_void reg_set_trpp1_int_edge_ch6_crc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch6_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch6_crc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch6_crc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch6_str_id_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch6_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch6_str_id_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch6_str_id_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch6_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch6_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch6_pes_wr_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch6_pes_wr_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch6_ts_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch6_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch6_ts_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch6_ts_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch6_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch6_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch6_pes_sc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch6_pes_sc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch6_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch6_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch6_pes_data_cnt_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch6_pes_data_cnt_iedge;
}

mt_void reg_set_trpp1_int_edge_ch7_crc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch7_crc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch7_crc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch7_crc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch7_str_id_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch7_str_id_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch7_str_id_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch7_str_id_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch7_pes_wr_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch7_pes_wr_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch7_pes_wr_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch7_pes_wr_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch7_ts_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch7_ts_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch7_ts_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch7_ts_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch7_pes_sc_err_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch7_pes_sc_err_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch7_pes_sc_err_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch7_pes_sc_err_iedge;
}

mt_void reg_set_trpp1_int_edge_ch7_pes_data_cnt_iedge(mt_u8 data)
{
    reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    d.bitc.ch7_pes_data_cnt_iedge = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge), d.all);
}

mt_u8   reg_get_trpp1_int_edge_ch7_pes_data_cnt_iedge(mt_void)
{
	reg_trpp1_int_edge_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_edge));
    return d.bitc.ch7_pes_data_cnt_iedge;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_clr (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_clr(mt_u32 data)
{
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr), data); 
}

mt_u32  reg_get_trpp1_int_clr(mt_void)
{
	reg_trpp1_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr)); 
    return d.all;
}

mt_void reg_set_trpp1_int_clr_trpp1_int_clr(mt_u32 data)
{
	reg_trpp1_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr));    
    d.bitc.trpp1_int_clr = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr), d.all); 
}

mt_u32  reg_get_trpp1_int_clr_trpp1_int_clr(mt_void)
{
	reg_trpp1_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr)); 
    return d.bitc.trpp1_int_clr;
}

mt_void reg_set_trpp1_int_clr_trpp1_int_rd(mt_u8 data)
{
    reg_trpp1_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr)); 
    d.bitc.trpp1_int_rd = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr), d.all); 
}

mt_u8   reg_get_trpp1_int_clr_trpp1_int_rd(mt_void)
{
	reg_trpp1_int_clr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_clr)); 
    return d.bitc.trpp1_int_rd;
}

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_state (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_state(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), data);
}

mt_u32  reg_get_trpp1_int_state(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.all;
}

mt_void reg_set_trpp1_int_state_ch4_crc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch4_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch4_crc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch4_crc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch4_str_id_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch4_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch4_str_id_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch4_str_id_err_ista;
}

mt_void reg_set_trpp1_int_state_ch4_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch4_pes_wr_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);	
}

mt_u8   reg_get_trpp1_int_state_ch4_pes_wr_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch4_pes_wr_err_ista;
}

mt_void reg_set_trpp1_int_state_ch4_ts_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch4_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch4_ts_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch4_ts_err_ista;
}

mt_void reg_set_trpp1_int_state_ch4_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch4_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch4_pes_sc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch4_pes_sc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch4_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch4_pes_data_cnt_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch4_pes_data_cnt_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch4_pes_data_cnt_ista;
}

mt_void reg_set_trpp1_int_state_ch5_crc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch5_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch5_crc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch5_crc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch5_str_id_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch5_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch5_str_id_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch5_str_id_err_ista;
}

mt_void reg_set_trpp1_int_state_ch5_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch5_pes_wr_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch5_pes_wr_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch5_pes_wr_err_ista;
}

mt_void reg_set_trpp1_int_state_ch5_ts_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch5_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch5_ts_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch5_ts_err_ista;
}

mt_void reg_set_trpp1_int_state_ch5_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch5_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch5_pes_sc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch5_pes_sc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch5_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch5_pes_data_cnt_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch5_pes_data_cnt_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch5_pes_data_cnt_ista;
}

mt_void reg_set_trpp1_int_state_ch6_crc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch6_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch6_crc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch6_crc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch6_str_id_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch6_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch6_str_id_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch6_str_id_err_ista;
}

mt_void reg_set_trpp1_int_state_ch6_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch6_pes_wr_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch6_pes_wr_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch6_pes_wr_err_ista;
}

mt_void reg_set_trpp1_int_state_ch6_ts_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch6_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch6_ts_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch6_ts_err_ista;
}

mt_void reg_set_trpp1_int_state_ch6_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch6_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch6_pes_sc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch6_pes_sc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch6_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch6_pes_data_cnt_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch6_pes_data_cnt_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch6_pes_data_cnt_ista;
}

mt_void reg_set_trpp1_int_state_ch7_crc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch7_crc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch7_crc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch7_crc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch7_str_id_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch7_str_id_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch7_str_id_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch7_str_id_err_ista;
}

mt_void reg_set_trpp1_int_state_ch7_pes_wr_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch7_pes_wr_err_ista = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch7_pes_wr_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch7_pes_wr_err_ista;
}

mt_void reg_set_trpp1_int_state_ch7_ts_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch7_ts_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch7_ts_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch7_ts_err_ista;
}

mt_void reg_set_trpp1_int_state_ch7_pes_sc_err_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch7_pes_sc_err_ista = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch7_pes_sc_err_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch7_pes_sc_err_ista;
}

mt_void reg_set_trpp1_int_state_ch7_pes_data_cnt_ista(mt_u8 data)
{
    reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    d.bitc.ch7_pes_data_cnt_ista = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state), d.all);
}

mt_u8   reg_get_trpp1_int_state_ch7_pes_data_cnt_ista(mt_void)
{
	reg_trpp1_int_state_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp1_int_state));
    return d.bitc.ch7_pes_data_cnt_ista;
}

mt_void reg_set_trpp2_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp2_int_mask), data);
}

mt_u32  reg_get_trpp2_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp2_int_mask));
}

mt_u32  reg_get_trpp2_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp2_int_state));
}

mt_void reg_set_trpp3_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp3_int_mask), data);
}

mt_u32  reg_get_trpp3_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp3_int_mask));
}

mt_u32  reg_get_trpp3_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp3_int_state));
}

mt_void reg_set_trpp4_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp4_int_mask), data);
}

mt_u32  reg_get_trpp4_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp4_int_mask));
}

mt_u32  reg_get_trpp4_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp4_int_state));
}

mt_void reg_set_trpp4_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp4_int_clr), data);
}

mt_u32  reg_get_trpp4_int_clr(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp4_int_clr));
}

mt_void reg_set_trpp5_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp5_int_mask), data);
}

mt_u32  reg_get_trpp5_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp5_int_mask));
}

mt_void  reg_set_trpp5_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp5_int_clr), data);
}

mt_u32  reg_get_trpp5_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp5_int_state));
}

mt_void reg_set_trpp6_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp6_int_mask), data);
}

mt_u32  reg_get_trpp6_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp6_int_mask));
}

mt_u32  reg_get_trpp6_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp6_int_state));
}


mt_void reg_set_trpp7_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp7_int_mask), data);
}

mt_u32  reg_get_trpp7_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp7_int_mask));
}

mt_u32  reg_get_trpp7_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp7_int_state));
}

mt_void reg_set_trpp8_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp8_int_mask), data);
}

mt_u32  reg_get_trpp8_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp8_int_mask));
}

mt_u32  reg_get_trpp8_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp8_int_state));
}

mt_void reg_set_trpp9_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp9_int_mask), data);
}

mt_u32  reg_get_trpp9_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp9_int_mask));
}

mt_u32  reg_get_trpp9_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp9_int_state));
}

mt_void reg_set_trpp10_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp10_int_mask), data);
}

mt_u32  reg_get_trpp10_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp10_int_mask));
}

mt_void  reg_set_trpp10_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp10_int_clr), data);
}

mt_u32  reg_get_trpp10_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp10_int_state));
}

mt_void reg_set_trpp11_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp11_int_mask), data);
}

mt_u32  reg_get_trpp11_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp11_int_mask));
}

mt_void  reg_set_trpp11_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp11_int_clr), data);
}

mt_u32  reg_get_trpp11_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp11_int_state));
}

mt_void reg_set_trpp12_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp12_int_mask), data);
}

mt_u32  reg_get_trpp12_int_mask(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp12_int_mask));
}

mt_void  reg_set_trpp12_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp12_int_clr), data);
}

mt_u32  reg_get_trpp12_int_state(mt_void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp12_int_state));
}

/*---------------------------------------------------------*/
/* register pvr   interrupt    function                    */
/*---------------------------------------------------------*/
mt_void reg_set_pvr_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_mask), data);
}

mt_u32  reg_get_pvr_int_mask(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_mask));
}

mt_void reg_set_pvr_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_edge), data);
}

mt_u32 reg_get_pvr_int_edge(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_edge));
}

mt_void reg_set_pvr_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_clr), data);
}

mt_u32 reg_get_pvr_int_clr(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_clr));
}

mt_void reg_set_pvr_int_state(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_state), data);
}

mt_u32 reg_get_pvr_int_state(mt_void)
{
	return  HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr_int_state));
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_void reg_set_pvr1_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_mask), data);
}

mt_u32  reg_get_pvr1_int_mask(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_mask));
}

mt_void reg_set_pvr1_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_edge), data);
}

mt_u32 reg_get_pvr1_int_edge(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_edge));
}

mt_void reg_set_pvr1_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_clr), data);
}

mt_u32 reg_get_pvr1_int_clr(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_clr));
}

mt_void reg_set_pvr1_int_rd(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_rd), data);	  
}

mt_u32 reg_get_pvr1_int_rd(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_rd));
}

mt_u32 reg_get_pvr1_int_state(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr1_int_state));
}

mt_void reg_set_pvr2_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_mask), data);
}

mt_u32  reg_get_pvr2_int_mask(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_mask));
}

mt_void reg_set_pvr2_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_edge), data);
}

mt_u32 reg_get_pvr2_int_edge(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_edge));
}

mt_void reg_set_pvr2_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_clr), data);
}

mt_u32 reg_get_pvr2_int_clr(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_clr));
}

mt_void reg_set_pvr2_int_rd(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_rd), data);	  
}

mt_u32 reg_get_pvr2_int_rd(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_rd));
}

mt_u32 reg_get_pvr2_int_state(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_trpp_pvr2_int_state));
}
#endif

/*----------------------------------------------------------------------------*/
/* register gglb int function                                                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_gglb_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_gglb_int_mask), data);
}

mt_u32 reg_get_dmx_gglb_int_mask(mt_void)
{
  return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_gglb_int_mask));
}

mt_void reg_set_dmx_gglb_int_edge(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_gglb_int_edge), data);
}

mt_u32 reg_get_dmx_gglb_int_edge(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_gglb_int_edge));
}

mt_void reg_set_dmx_gglb_int_clr(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_gglb_int_clr), data);
}

mt_u32 reg_get_dmx_gglb_int_clr(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_gglb_int_clr));
}

mt_void reg_set_dmx_gglb_int_state(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_gglb_int_state), data);
}

mt_u32 reg_get_dmx_gglb_int_state(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_gglb_int_state));
}

/*----------------------------------------------------------------------------*/
/* register dmx_pcr_fifo_cnt function (read)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_pcr_fifo_cnt(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_fifo_cnt), data);
}

mt_u32  reg_get_dmx_pcr_fifo_cnt(mt_void)
{
	reg_dmx_pcr_fifo_cnt_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_fifo_cnt));
  	return d.all;
}

mt_void reg_set_dmx_pcr_fifo_cnt_pcr_fifo_cnt(mt_u32 data)
{
	reg_dmx_pcr_fifo_cnt_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_fifo_cnt));
	d.bitc.pcr_fifo_cnt = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_fifo_cnt), d.all);
}

mt_u32  reg_get_dmx_pcr_fifo_cnt_pcr_fifo_cnt(mt_void)
{
	reg_dmx_pcr_fifo_cnt_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_fifo_cnt));
  	return d.bitc.pcr_fifo_cnt;
}

/*----------------------------------------------------------------------------*/
/* register dmx_pcr_value_low function (read)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_pcr_value_low(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_value_low), data);
}

mt_u32  reg_get_dmx_pcr_value_low(mt_void)
{
	reg_dmx_pcr_value_low_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_low));
  	return d.all;
}

mt_void reg_set_dmx_pcr_value_low_pcr_value_low(mt_u32 data)
{
	reg_dmx_pcr_value_low_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_low));
	d.bitc.pcr_value_low = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_value_low), d.all);
}

mt_u32  reg_get_dmx_pcr_value_low_pcr_value_low(mt_void)
{
	reg_dmx_pcr_value_low_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_low));
	return d.bitc.pcr_value_low;
}

/*----------------------------------------------------------------------------*/
/* register dmx_pcr_value_high function (read)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_pcr_value_high(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high), data);
}

mt_u32  reg_get_dmx_pcr_value_high(mt_void)
{
	reg_dmx_pcr_value_high_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high));
  	return d.all;
}

mt_void reg_set_dmx_pcr_value_high_pcr_value_high(mt_u32 data)
{
	reg_dmx_pcr_value_high_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high));
	d.bitc.pcr_value_high = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high), d.all);
}

mt_u32  reg_get_dmx_pcr_value_high_pcr_value_high(mt_void)
{
	reg_dmx_pcr_value_high_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high));
  	return d.bitc.pcr_value_high;
}

mt_void reg_set_dmx_pcr_value_high_pcr_value_ch(mt_u32 data)
{
	reg_dmx_pcr_value_high_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high));
	d.bitc.pcr_value_ch = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high), d.all);	
}

mt_u32  reg_get_dmx_pcr_value_high_pcr_value_ch(mt_void)
{
	reg_dmx_pcr_value_high_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high));
  	return d.bitc.pcr_value_ch;
}

mt_void reg_set_dmx_pcr_value_high_dis_indicator(mt_u32 data)
{
	reg_dmx_pcr_value_high_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high));
	d.bitc.dis_indicator = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high), d.all);
}

mt_u32  reg_get_dmx_pcr_value_high_dis_indicator(mt_void)
{
	reg_dmx_pcr_value_high_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_pcr_value_high));
  	return d.bitc.dis_indicator;
}

/*----------------------------------------------------------------------------*/
/* register dmx_hwcg_mode function (read/write)                    			  */
/*----------------------------------------------------------------------------*/
void reg_set_dmx_hwcg_mode(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_hwcg_mode), data);
}

mt_u32  reg_get_dmx_hwcg_mode(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_hwcg_mode));
}

/*----------------------------------------------------------------------------*/
/* register dmx_bus_debug_chn_staddr function (read/write)                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_bus_debug_chn_staddr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_staddr + index * 0x08), data);
}

mt_u32  reg_get_dmx_bus_debug_chn_staddr(mt_u8 index)
{
	reg_dmx_bus_debug_chn_staddr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_staddr + index * 0x08));
  	return d.all;
}

mt_void reg_set_dmx_bus_debug_chn_staddr_bus_debug_chn_staddr(mt_u8 index, mt_u32 data)
{
	reg_dmx_bus_debug_chn_staddr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_staddr + index * 0x08));
	d.bitc.bus_debug_chn_staddr = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_staddr + index * 0x08), d.all);
}

mt_u32  reg_get_dmx_bus_debug_chn_staddr_bus_debug_chn_staddr(mt_u8 index)
{
	reg_dmx_bus_debug_chn_staddr_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_staddr + index * 0x08));
  	return d.bitc.bus_debug_chn_staddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bus_debug_chn_endaddr function (read/write)                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_bus_debug_chn_endaddr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_endaddr + index * 0x08), data);
}

mt_u32  reg_get_dmx_bus_debug_chn_endaddr(mt_u8 index)
{
	reg_dmx_bus_debug_chn_endaddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_endaddr + index * 0x08));
	return d.all;
}

mt_void reg_set_dmx_bus_debug_chn_endaddr_bus_debug_chn_endaddr(mt_u8 index, mt_u32 data)
{
	reg_dmx_bus_debug_chn_endaddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_endaddr + index * 0x08));
	d.bitc.bus_debug_chn_endaddr = data;
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_endaddr + index * 0x08), d.all);
}
mt_u32  reg_get_dmx_bus_debug_chn_endaddr_bus_debug_chn_endaddr(mt_u8 index)
{
	reg_dmx_bus_debug_chn_endaddr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_endaddr + index * 0x08));
  	return d.bitc.bus_debug_chn_endaddr;
}

/*----------------------------------------------------------------------------*/
/* register dmx_bus_debug_hit_addr function (read)                            */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_dmx_bus_debug_hit_addr(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_hit_addr));
}

mt_u32  reg_get_dmx_bus_debug_hit_addr_dmx_bus_debug_hit_addr(mt_void)
{
	reg_dmx_bus_debug_hit_addr_t d;
	d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_bus_debug_chn_endaddr));
	return d.bitc.bus_debug_hit_addr;
}

mt_u32  reg_get_dmx_sf_process_sta(mt_void)
{
	return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_sf_process_sta));
}

/*----------------------------------------------------------------------------*/
/*-------------------------------------------------*/
/* register dmx_t2mi  (read/write)                 */
/*-------------------------------------------------*/
void reg_set_dmx_t2mi_en(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_en), data);
}

mt_u32  reg_get_dmx_t2mi_en(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_en));
}

void reg_set_dmx_t2mi_int_mask(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_int_mask), data);
}

mt_u32  reg_get_dmx_t2mi_int_mask(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_int_mask));
}

void reg_set_dmx_t2mi_int_clr(mt_u32 data)
{
    *(volatile mt_u32 *)reg_dmx_t2mi_int_clr = data;
}

mt_u32 reg_get_dmx_t2mi_int_clr(void)
{
    return (*(volatile mt_u32 *)reg_dmx_t2mi_int_clr);
}

mt_u32  reg_get_dmx_t2mi_int_state(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_int_state));
}

void reg_set_dmx_t2mi_set1(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1), data);
}

mt_u32  reg_get_dmx_t2mi_set1(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
}

void reg_set_dmx_t2mi_set1_t2mi_pid(mt_u32 data)
{
    reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    d.bitc.t2mi_pid = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1), d.all);
}

mt_u32 reg_get_dmx_t2mi_set1_t2mi_pid(void)
{
	reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    return d.bitc.t2mi_pid;
}

void reg_set_dmx_t2mi_set1_t2mi_plp_id(mt_u32 data)
{
    reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    d.bitc.t2mi_plp_id = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1), d.all);
}

mt_u32 reg_get_dmx_t2mi_set1_t2mi_plp_id(void)
{
	reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    return d.bitc.t2mi_plp_id;
}

void reg_set_dmx_t2mi_set1_t2mi_output_ch(mt_u32 data)
{
    reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    d.bitc.t2mi_output_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1), d.all);
}

mt_u32 reg_get_dmx_t2mi_set1_t2mi_output_ch(void)
{
	reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    return d.bitc.t2mi_output_ch;
}

void reg_set_dmx_t2mi_set1_t2mi_input_ch(mt_u32 data)
{
    reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    d.bitc.t2mi_input_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1), d.all);	
}

mt_u32 reg_get_dmx_t2mi_set1_t2mi_input_ch(void)
{
	reg_dmx_t2mi_set1_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set1));
    return d.bitc.t2mi_input_ch;
}

void reg_set_dmx_t2mi_set2(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2), data);
}

mt_u32  reg_get_dmx_t2mi_set2(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2));
}

void reg_set_dmx_t2mi_set2_t2mi_pusi_det_en(mt_u32 data)
{
	reg_dmx_t2mi_set2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2));    
    d.bitc.t2mi_pusi_det_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2), d.all);
}

mt_u32 reg_get_dmx_t2mi_set2_t2mi_pusi_det_en(void)
{
	reg_dmx_t2mi_set2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2));
    return d.bitc.t2mi_pusi_det_en;
}

void reg_set_dmx_t2mi_set2_t2mi_swtsi_en(mt_u32 data)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    reg_dmx_t2mi_set2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2));
    d.bitc.t2mi_swtsi_en = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2), d.all);	
#endif
}

mt_u32 reg_get_dmx_t2mi_set2_t2mi_swtsi_en(void)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY4
	reg_dmx_t2mi_set2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2));
    return d.bitc.t2mi_swtsi_en;
#else
	return 0;
#endif
}

void reg_set_dmx_t2mi_set2_t2mi_src_ch(mt_u32 data)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    reg_dmx_t2mi_set2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2));
    d.bitc.t2mi_swtsi_src_ch = data;
    HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2), d.all);
#endif
}

mt_u32 reg_get_dmx_t2mi_set2_t2mi_src_ch(void)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY4
	reg_dmx_t2mi_set2_t d;
    d.all = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_t2mi_set2));
    return d.bitc.t2mi_swtsi_src_ch;
#else
	return 0;
#endif
}

/*----------------------------------------------------------------------------*/
/* register address cross-border                                              */
/*----------------------------------------------------------------------------*/
void reg_set_dmx_debug_trpp_sec_cfg(mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_debug_trpp_sec_cfg), data);
}

mt_u32  reg_get_dmx_debug_trpp_sec_cfg(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_debug_trpp_sec_cfg));
}

mt_u32  reg_get_dmx_debug_addr_sta(void)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_debug_addr_sta));
}

void reg_set_dmx_debug_chx_start_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_debug_ch0_start_addr + index * 0x08), data);
}

mt_u32  reg_get_dmx_debug_chx_start_addr(mt_u8 index)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_debug_ch0_start_addr + index * 0x08));
}

void reg_set_dmx_debug_chx_end_addr(mt_u8 index, mt_u32 data)
{
	HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_debug_ch0_end_addr + index * 0x08), data);
}

mt_u32  reg_get_dmx_debug_chx_end_addr(mt_u8 index)
{
    return HAL_GET_U32((volatile mt_u32 *)(reg_dmx_debug_ch0_end_addr + index * 0x08));
}

mt_u32 reg_dmx_get_clk(void)
{	
	ulong tsi_clk_reg = mt_get_clk_base() + 0xb004;
    return HAL_GET_U32((volatile mt_u32 *)(tsi_clk_reg));
}

void reg_dmx_set_clk(mt_u32 value)
{	
	ulong tsi_clk_reg = mt_get_clk_base() + 0xb004;
    HAL_PUT_U32((volatile mt_u32 *)(tsi_clk_reg), value);
}

mt_u32 reg_dmx_get_src(void)
{	
	ulong tsi_src_reg = mt_get_public_base() + 0x8008;
    return HAL_GET_U32((volatile mt_u32 *)(tsi_src_reg));
}

void reg_dmx_set_src(mt_u32 value)
{	
	ulong tsi_src_reg = mt_get_public_base() + 0x8008;
    HAL_PUT_U32((volatile mt_u32 *)(tsi_src_reg), value);
}


/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
void reg_set_tsi_ci_set_cfg(mt_u32 value)
{	
	HAL_PUT_U32((volatile mt_u32 *)(reg_tsi_ci_set_cfg),value);	
}

mt_u32 reg_get_tsi_ci_set_cfg(void)
{	
	return HAL_GET_U32((volatile mt_u32 *)(reg_tsi_ci_set_cfg));	
}

void reg_set_tsi_ci_cicam_cfg(mt_u32 value)
{	
	HAL_PUT_U32((volatile mt_u32 *)(reg_tsi_ci_cicam_set),value);	
}

mt_u32 reg_get_tsi_ci_cicam_cfg(void)
{	
	return HAL_GET_U32((volatile mt_u32 *)(reg_tsi_ci_cicam_set));	
}

void reg_set_tsi_ci_lln_num_start_cfg(mt_u32 value)
{	
	HAL_PUT_U32((volatile mt_u32 *)(reg_tsi_ci_lln_num_start),value);	
}

mt_u32 reg_get_tsi_ci_lln_num_start_cfg(void)
{	
	return HAL_GET_U32((volatile mt_u32 *)(reg_tsi_ci_lln_num_start));	
}

void reg_set_tsi_ci_swtsich_cfg(mt_u32 value)
{	
	HAL_PUT_U32((volatile mt_u32 *)(reg_tsi_swtsi_ch_set),value);	
}

mt_u32 reg_get_tsi_ci_swtsich_cfg(void)
{	
	return HAL_GET_U32((volatile mt_u32 *)(reg_tsi_swtsi_ch_set));	
}

mt_u32 reg_get_tsi_ci_status(void)
{	
	return HAL_GET_U32((volatile mt_u32 *)(reg_tsi_ci_status));	
}


#endif

/*add end*/

/*----------------------------------------------------------------------------*/
/* init function                                                              */
/*----------------------------------------------------------------------------*/
mt_void reg_dmx_init(mt_void)
{
    int i;
	int j = 0;

    reg_set_ts0_sample_ctrl((mt_u32)0x470001ff);
    reg_set_ts1_sample_ctrl((mt_u32)0x470001ff);
    reg_set_ts_stop_cnt_len((mt_u32)0x00000000);
    reg_set_swtsi_urgent_cfg((mt_u32)0x00000000);
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_lln_addr(i, (mt_u32)0x000000ff);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_control(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_af_cfg0(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_af_cfg1(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_af_cfg2(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_af_cfg3(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_af_cfg4(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_af_cfg5(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_RAMPORT_CNT; i++)
    {
        reg_set_swtsi_chn_dbuf_wrpoint(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_CHANNEL_CNT; i++)
    {
        reg_set_demux_slotn_cfg0(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_CHANNEL_CNT; i++)
    {
        reg_set_demux_slotn_cfg1(i, (mt_u32)0x00000000);
    }
    reg_set_demux_pause_cfg0((mt_u32)0x00000000);
    reg_set_demux_pause_cfg1((mt_u32)0x00000000);

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
    reg_set_tsi_ds_cw_op((mt_u32)0x00000000);
#endif

    reg_set_tsi_ades_ive0_init((mt_u32)0x00000000);
    reg_set_tsi_ades_ive1_init((mt_u32)0x00000000);
    reg_set_tsi_ades_ive2_init((mt_u32)0x00000000);
    reg_set_tsi_ades_ive3_init((mt_u32)0x00000000);
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_odd0(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_odd1(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_odd2(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_odd3(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_odd4(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_odd5(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_even0(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_even1(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_even2(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_even3(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_even4(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_even5(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_chn_tscfg(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ds_core(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_aes_ive(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ades_disc_mode(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_tsi_ades_pktmode(i, (mt_u32)0x00000000);
    }
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
    {
	    for (i=0; i<16; i++)
	    {
	        reg_set_tsi_algo_cw_ive_port(i, (mt_u32)0x00000000);
	    }
	    for (i=0; i<16; i++)
	    {
	        reg_set_tsi_keyslot_tab(i, (mt_u32)0x00000000);
	    }

	    //reg_set_dmx_ds_big_little_endian(0);
    }	
	
    for (i=0; i<128; i++)
    {
        reg_set_bufn_staddr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_bufn_size(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_bufn_disc_wptr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_bufn_ts_int_cfg(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_bufn_data_wptr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_bufn_cursec_len(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_filtern_config(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_funit_filter_data(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_funit_filter_mask(i, (mt_u32)0x00000000);
    }
    for (i=0; i<128; i++)
    {
        reg_set_funit_filter_mode(i, (mt_u32)0x00000000);
    }
    reg_set_trpp_channel_parse_en((mt_u32)0x00000000);
    reg_set_trpp_ch_clear_status((mt_u32)0x00000000);
    reg_set_trpp_bus_urgent((mt_u32)0x00000000);
    reg_set_trpp_channel_record_en((mt_u32)0x00000000);
    reg_set_trpp_sc_index_flt1_4((mt_u32)0x00000000);
    reg_set_trpp_sc_index_flt5_8((mt_u32)0x00000000);
    reg_set_trpp_sc_index_flt9_10((mt_u32)0x00000000);
    reg_set_trpp_sc_index_flt11_12((mt_u32)0x00000000);
    reg_set_trpp_sc_index_flt0((mt_u32)0x00000000);
    reg_set_trpp_esbuf_ch((mt_u32)0x0000FFFF);//the value of 0x0F is invalid for dmx's es buf channel, refer to DMX_AV_CHANNEL_CNT
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_property(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_parse_set(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_start_code1(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_frm_start_code_m1(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_start_code2(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_frm_start_code_m2(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_dscrpt_start_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_data_start_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_dscrpt_end_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_data_end_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_dscrpt_wr_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_data_wr_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_dscrpt_rd_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch_data_rd_addr(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info1(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info2(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info3(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info4(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info5(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info6(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info7(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info8(i, (mt_u32)0x00000000);
    }
    for (i=0; i<16; i++)
    {
        reg_set_trpp_ch1_ini_info9(i, (mt_u32)0x00000000);
    }
    for (i=0; i<DMX_REC_CNT; i++)
    {
        reg_set_trpp_ch_rec_set(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_rec_start_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_rec_end_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_rec_wr_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_rec_rd_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch11_ini_info1(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch11_ini_info2(i, (mt_u32)0x00000000); 
        reg_set_trpp_ch11_ini_info3(i, (mt_u32)0x00000000);
        reg_set_trpp_ch_ts_sn(i, (mt_u32)0x00000000);
		reg_set_trpp_ch11_lln_set0(i, (mt_u32)0x00000000);
		reg_set_trpp_ch11_lln_set1(i, (mt_u32)0x00000000);
		reg_set_trpp_ch11_lln_set2(i, (mt_u32)0x00000000);
		reg_set_trpp_ch11_lln_set3(i, (mt_u32)0x00000000);
		for (j=0; j<8; j++)
		{
			reg_set_trpp_ch11_lln_idx_set0(i, j, (mt_u32)0x00000000);
			reg_set_trpp_ch11_lln_idx_set1(i, j, (mt_u32)0x00000000);
		}

		reg_set_trpp_ch11_lln_full_0(i, (mt_u32)0x00000000);
		reg_set_trpp_ch11_lln_full_1(i, (mt_u32)0x00000000);
		reg_set_trpp_ch11_lln_rd_byte_set(i, (mt_u32)0x00000000);
    }
	
    for (i=0; i<DMX_REC_INDEX_CNT; i++)
    {
        reg_set_trpp_ch_idx_mode(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_idx_enable(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_idx_start_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_idx_end_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_idx_wr_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch_idx_rd_addr(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch15_ini_info1(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch15_ini_info2(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch15_ini_info3(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch15_ini_info4(i, (mt_u32)0x00000000);
		reg_set_trpp_ch15_ini_info5(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch15_ini_info6(i, (mt_u32)0x00000000);    
        reg_set_trpp_ch15_ini_info7(i, (mt_u32)0x00000000);
		reg_set_trpp_ch15_sc_flt_set0(i, (mt_u32)0x00000000);    
		reg_set_trpp_ch15_sc_flt_set1(i, (mt_u32)0x00000000);    
		reg_set_trpp_ch15_sc_flt_set2(i, (mt_u32)0x00000000);    
		reg_set_trpp_ch15_sc_flt_set3(i, (mt_u32)0x00000000);    
		reg_set_trpp_ch15_sc_flt_set4(i, (mt_u32)0x00000000);    
    }
	
    reg_set_ds_int_mask((mt_u32)0x00000000);
    reg_set_ds_int_edge((mt_u32)0x00000000);
    reg_set_ds_int_clr((mt_u32)0x00000000);
    reg_set_ds_int_state((mt_u32)0x00000000);
    reg_set_trpp0_int_mask((mt_u32)0x00000000);
    reg_set_trpp0_int_edge((mt_u32)0x00000000);
    reg_set_trpp0_int_clr((mt_u32)0x00000000);
    reg_set_trpp0_int_state((mt_u32)0x00000000);
    reg_set_trpp1_int_mask((mt_u32)0x00000000);
    reg_set_trpp1_int_edge((mt_u32)0x00000000);
    reg_set_trpp1_int_clr((mt_u32)0x00000000);
    reg_set_trpp1_int_state((mt_u32)0x00000000);
    /* read read-clear registers in order to set mirror variables */

	reg_set_dmx_hwcg_mode((mt_u32)0x00000000);
}

/*----------------------------------------------------------------------------*/
/* end of file                                                                */
/*----------------------------------------------------------------------------*/

