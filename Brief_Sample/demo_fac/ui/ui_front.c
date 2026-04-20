/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "ui_manager.h"

enum front_info_sub_idx
{
	IDC_FRONT_IR_TEST_SWITCH,
	IDC_FRONT_IR_KEY_VALUE,
	IDC_FRONT_BOTTON_TEST_SWITCH,
	IDC_FRONT_BOTTON_KEY_VALUE,
	IDC_FRONT_MAX
};

#define SWITCH_EVENT 0x05

MT_BOOL g_ir_switch = FALSE;
MT_BOOL g_frontend_switch = FALSE;

mt_s32 front_callback_event(mt_u16 evt_id, mt_u32 param)
{
	mt_s32 ret;

	ret = demo_av_set_mute(TRUE);

	return ret;
}

mt_s32 front_ir_switch(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	
	if(evt_id != SWITCH_EVENT)
		return MT_SUCCESS;

	memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
	if(g_ir_switch == TRUE)
	{
		g_ir_switch = FALSE;
		sprintf(sc->p_dis_text, "Ir test:                 <off>");
	}
	else
	{
		g_ir_switch = TRUE;
		sprintf(sc->p_dis_text, "Ir test:                 <on>");
	}

	return MT_SUCCESS;
}

mt_s32 front_ir_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_u32 input_key = para2;

	input_key = (input_key & 0xffff) << 16 | (input_key & 0xff0000) >> 8 | (input_key & 0xff000000) >> 24;
	printf("%s(line: %d), input key 0x%x\n", __FUNCTION__, __LINE__, input_key);

	if(g_ir_switch)
	{
//		printf("%s(line: %d), ir update\n", __FUNCTION__, __LINE__);
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "key value:    0x%08x", input_key);
	}

	return MT_SUCCESS;
}

mt_s32 front_frontend_switch(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	
	if(evt_id != SWITCH_EVENT)
		return MT_SUCCESS;

	printf("%s(line: %d), sc type %d\n", __FUNCTION__, __LINE__, sc->type);
	
	memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
	if(g_frontend_switch == TRUE)
	{
		g_frontend_switch = FALSE;
		sprintf(sc->p_dis_text, "Botton test:             <off>");
	}
	else
	{
		g_frontend_switch = TRUE;
		sprintf(sc->p_dis_text, "Botton test:             <on>");
	}

	return MT_SUCCESS;

}

mt_s32 front_frontend_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_u32 input_key = para2;

	printf("%s(line: %d), input key 0x%x\n", __FUNCTION__, __LINE__, input_key);

	if(g_frontend_switch)
	{
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "key value:    0x%02x", input_key);
	}

	return MT_SUCCESS;
}

mt_s32 open_front(mt_u32 para1, mt_u32 para2)
{
	MT_RECT rc;
	struct SubCmd sc = {0};
	
	rc.x = 150 + ROOT_ID_FRONT * 170;
	rc.y = 100;
	rc.w = 170;
	rc.h = 50;
	add_main_win("Front", ROOT_ID_FRONT, rc, front_callback_event);

	sc.m_cursor = get_main_win_index(ROOT_ID_FRONT); 

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Ir test:                 <off>");
	sc.cnt = 0;
	sc.type = IDC_FRONT_IR_TEST_SWITCH;
	sc.rc.x = 550;
	sc.rc.y = 200;
	sc.rc.w = 400;
	sc.rc.h = 50;
	sc.is_noncursor = FALSE;
	sc.callback_event = front_ir_switch;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "key value:    0x00000000");
	sc.type = IDC_FRONT_IR_KEY_VALUE;
	sc.rc.x += 100;
	sc.rc.y += sc.rc.h + 20;
	sc.rc.w = 300;
	sc.is_noncursor = TRUE;
	sc.callback_event = front_ir_update;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Botton test:             <off>");
	sc.type = IDC_FRONT_BOTTON_TEST_SWITCH;
	sc.rc.x = 550;
	sc.rc.y += sc.rc.h + 20;
	sc.rc.w = 400;
	sc.is_noncursor = FALSE;
	sc.callback_event = front_frontend_switch;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "key value:    0x00");
	sc.type = IDC_FRONT_BOTTON_KEY_VALUE;
	sc.rc.x += 100;
	sc.rc.y += sc.rc.h + 20;
	sc.rc.w = 300;
	sc.is_noncursor = TRUE;
	sc.callback_event = front_frontend_update;
	add_sub_win_cmd(&sc);
	
	return MT_SUCCESS;
}


