/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "ui_manager.h"
#include "mt_unf_frontend.h"

enum dvbc_info_sub_idx
{
	IDC_DVBC_FREQUENCY,
	IDC_DVBC_SYMBOLRATE,
	IDC_DVBC_MODULATION,
	IDC_DVBC_SIGNAL_QULITY,
	IDC_DVBC_SIGNAL_STRENGTH,
	IDC_DVBC_MAX
};

mt_s32 dvbc_callback_event(mt_u16 evt_id, mt_u32 param)
{
	mt_s32 ret;

	ret = demo_av_set_mute(TRUE);

	return ret;
}

mt_s32 dvbc_signal_quality_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_unf_fe_status_t status = *(mt_unf_fe_status_t *)&para2;

	memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc->p_dis_text, "Qulity:      %d%%", status.param.channel_info.perf.snr);

	return MT_SUCCESS;
}

mt_s32 dvbc_signal_strength_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_unf_fe_status_t status = *(mt_unf_fe_status_t *)&para2;

	memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc->p_dis_text, "Strength:      %d%%", status.param.channel_info.perf.agc);

	return MT_SUCCESS;
}

mt_s32 open_dvbc(mt_u32 para1, mt_u32 para2)
{
	MT_RECT rc;
	struct SubCmd sc = {0};
	
	rc.x = 150 + ROOT_ID_DVBC * 170;
	rc.y = 100;
	rc.w = 170;
	rc.h = 50;
	add_main_win("DVBC", ROOT_ID_DVBC, rc, dvbc_callback_event);

	sc.m_cursor = get_main_win_index(ROOT_ID_DVBC); 

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Frequency:  530MHz");
	sc.cnt = 0;
	sc.type = IDC_DVBC_FREQUENCY;
	sc.rc.x = 550;
	sc.rc.y = 200;
	sc.rc.w = 600;
	sc.rc.h = 50;
	sc.is_usr_define = FALSE;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Symbol rate:    6875Kbd");
	sc.type = IDC_DVBC_SYMBOLRATE;
	sc.rc.y += sc.rc.h + 20;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Molulation:     64QAM");
	sc.type = IDC_DVBC_MODULATION;
	sc.rc.y += sc.rc.h + 20;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Qulity:      0%%");
	sc.type = IDC_DVBC_SIGNAL_QULITY;
	sc.rc.x = 400;
	sc.rc.y += sc.rc.h + 20;
	sc.rc.w = 400;
	sc.is_noncursor = TRUE;
	sc.callback_event = dvbc_signal_quality_update;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Strength:    0%%");
	sc.type = IDC_DVBC_SIGNAL_STRENGTH;
	sc.rc.x += sc.rc.w;
	sc.callback_event = dvbc_signal_strength_update;
	add_sub_win_cmd(&sc);
	
	return MT_SUCCESS;
}


