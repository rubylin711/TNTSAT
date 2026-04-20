/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "ui_manager.h"

enum avplay_info_sub_idx
{
	IDC_AVPLAY_PLAY,
	IDC_AVPLAY_MAX
};

mt_s32 avplay_callback_event(mt_u16 evt_id, mt_u32 param)
{
	MT_RECT rc;

	rc.x = 160*1920/1280;
	rc.y = 150*1080/720;
	rc.w = 960*1920/1280;
	rc.h = 540*1080/720;
	
	demo_set_vo_window((void *)&rc);
	demo_av_set_mute(FALSE);
	
	return MT_SUCCESS;
}

mt_s32 open_avplay(mt_u32 para1, mt_u32 para2)
{
	MT_RECT rc;
	struct SubCmd sc;
	
	rc.x = 150 + ROOT_ID_AVPLAY * 170;
	rc.y = 100;
	rc.w = 170;
	rc.h = 50;
	add_main_win("AVPlay", ROOT_ID_AVPLAY, rc, avplay_callback_event);

	sc.m_cursor = get_main_win_index(ROOT_ID_AVPLAY);
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sc.cnt = 0;
	sc.type = IDC_AVPLAY_PLAY;
	sc.rc.x = 160;
	sc.rc.y = 150;
	sc.rc.w = 960;
	sc.rc.h = 540;
	sc.is_noncursor = TRUE;
	sc.is_usr_define = TRUE;
	sc.usr_color = 0x00000000;
	add_sub_win_cmd(&sc);
	
	return MT_SUCCESS;
}


