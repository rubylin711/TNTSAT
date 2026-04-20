/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_manager.h"
#include "mt_unf_otp.h"

enum basic_info_sub_idx
{
	IDC_BASIC_INFO_SIGNAL_TYPE,
	IDC_BASIC_INFO_PLATFORM,
	IDC_BASIC_INFO_CHIP_ID,
	IDC_BASIC_INFO_SOFTWARE,
	IDC_BASIC_INFO_USB0_STATUS,
	IDC_BASIC_INFO_USB1_STATUS,
	IDC_BASIC_INFO_PIP_PLAY,
	IDC_BASIC_INFO_MAX
};

#define SIGNAL_TYPE_SWITCH 0x05

#define _val2str(val) _setStr(#val)
static const char* _setStr(const char* str)
{
  return str;
}

mt_s32 basic_get_platform(MT_CHIP_VERSION_E version, mt_char *name)
{
	mt_char *str = NULL;

	if(name == NULL)
		return MT_FAILURE;
	
	switch(version)
	{
		case MT_CHIP_SYMPHONY_A0:
			str = _val2str(MT_CHIP_SYMPHONY_A0);
      		break;
		case MT_CHIP_SYMPHONY_A1:
			str = _val2str(MT_CHIP_SYMPHONY_A1);
      		break;
		case MT_CHIP_SYMPHONY_A2:
			str = _val2str(MT_CHIP_SYMPHONY_A2);
      		break;
		case MT_CHIP_SYMPHONY3_A0:
			str = _val2str(MT_CHIP_SYMPHONY3_A0);
      		break;
		case MT_CHIP_SYMPHONY2_A0:
			str = _val2str(MT_CHIP_SYMPHONY2_A0);
      		break;
		case MT_CHIP_SYMPHONY2_A1:
			str = _val2str(MT_CHIP_SYMPHONY2_A1);
      		break;
		case MT_CHIP_SYMPHONY2_A2:
			str = _val2str(MT_CHIP_SYMPHONY2_A2);
      		break;
		case MT_CHIP_SYMPHONY2_A3:
			str = _val2str(MT_CHIP_SYMPHONY2_A3);
      		break;
		case MT_CHIP_SYMPHONY4_A0:
			str = _val2str(MT_CHIP_SYMPHONY4_A0);
      		break;
		case MT_CHIP_SYMPHONY4_A1:
			str = _val2str(MT_CHIP_SYMPHONY4_A1);
      		break;
		default:
			break;
	}

	sprintf(name, str);
	
	return MT_SUCCESS;
}

mt_s32 basic_get_build_time(mt_char *software, mt_char *build_time)
{
	mt_char *tmp = NULL;

	if(software == NULL || build_time == NULL)
		return MT_FAILURE;
	
	tmp = strstr(software, "Build Time:");
	memcpy(build_time, tmp + 12, strlen(tmp) - 13);

	return MT_SUCCESS;
}

mt_s32 basic_callback_event(mt_u16 evt_id, mt_u32 param)
{
	MT_RECT rc;

	rc.x = 490*1920/1280;
	rc.y = 200*1080/720;
	rc.w = 630*1920/1280;
	rc.h = 420*1080/720;

	demo_set_vo_window((void *)&rc);
	demo_av_set_mute(FALSE);

	return MT_SUCCESS;
}

mt_s32 basic_signal_type_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubWin *p = (struct SubWin *)&para1;

	if(evt_id != SIGNAL_TYPE_SWITCH)
		return MT_SUCCESS;
	
	if(strstr(p->sc[p->s_cursor].p_dis_text, "DVBS"))
	{
		memset(p->sc[p->s_cursor].p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(p->sc[p->s_cursor].p_dis_text, "Signal type:   <DVBC>");
		demo_rebuild_playback(0x21);	//MT_UNF_DMX_PORT_TSI_0
	}
	else
	{
		memset(p->sc[p->s_cursor].p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(p->sc[p->s_cursor].p_dis_text, "Signal type:   <DVBS>");
		demo_rebuild_playback(0x20);	//MT_UNF_DMX_PORT_TSI_0
	}

	return MT_SUCCESS;
}

mt_s32 basic_usb0_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_u32 status = para2;

	if(evt_id == 5)
		return MT_SUCCESS;
	
	memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
	if(status == 0)
		memcpy(sc->p_dis_text, "USB0:         disconnected", strlen("USB0:         disconnected"));
	else
		memcpy(sc->p_dis_text, "USB0:         connected", strlen("USB0:         connected"));
	
	return MT_SUCCESS;
}

mt_s32 basic_usb1_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_u32 status = para2;

	if(evt_id == 5)
		return MT_SUCCESS;
	
	memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
	if(status == 0)
		memcpy(sc->p_dis_text, "USB1:         disconnected", strlen("USB1:         disconnected"));
	else
		memcpy(sc->p_dis_text, "USB1:         connected", strlen("USB1:         connected"));
	
	return MT_SUCCESS;
}

mt_s32 open_basic(mt_u32 para1, mt_u32 para2)
{
	struct SubCmd sc = {0};
	MT_RECT rc;
	mt_u32 status;
	mt_sys_version_s stSysChipInfo;
	mt_char tmp[30]  = {0};
	mt_u32 chipid_h = 0, chipid_l = 0;
	
	printf("%s(line: %d), entry\n", __FUNCTION__, __LINE__);
	rc.x = 150 + ROOT_ID_BASIC * 170;
	rc.y = 100;
	rc.w = 170;
	rc.h = 50;
	add_main_win("Basic Info", ROOT_ID_BASIC, rc, basic_callback_event);

	sc.m_cursor = get_main_win_index(ROOT_ID_BASIC); 

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Signal type:   <DVBC>");
	sc.cnt = 0;
	sc.type = IDC_BASIC_INFO_SIGNAL_TYPE;
	sc.rc.x = 150;
	sc.rc.y = 200;
	sc.rc.w = 340;
	sc.rc.h = 50;
	sc.callback_event = basic_signal_type_update;
	add_sub_win_cmd(&sc);
	
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	mt_sys_get_version(&stSysChipInfo);
	basic_get_platform(stSysChipInfo.enChipVersion, tmp);
	sprintf(sc.p_dis_text, "Platform:  %s", tmp);
	sc.cnt = 0;
	sc.rc.y += sc.rc.h + 20;
	sc.type = IDC_BASIC_INFO_PLATFORM;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	MT_UNF_OTP_get_chipid(&chipid_h, &chipid_l);
	sprintf(sc.p_dis_text, "Chip id:      %08x%08x", chipid_h, chipid_l);
	sc.cnt = 0;
	sc.rc.y += sc.rc.h + 20;
	sc.type = IDC_BASIC_INFO_CHIP_ID;
	sc.callback_event = NULL;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	memset(tmp, 0, 30);
	basic_get_build_time(stSysChipInfo.aversion, tmp);
	sprintf(sc.p_dis_text, "Build time:  %s", tmp);
	sc.cnt = 0;
	sc.rc.y += sc.rc.h + 20;
	sc.type = IDC_BASIC_INFO_SOFTWARE;
	add_sub_win_cmd(&sc);

	mt_sys_read_register(0xbf060084, &status);
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "USB0:         %s", (status & 0x01) == 0 ? "disconnected" : "connected");
	sc.cnt = 0;
	sc.rc.y += sc.rc.h + 20;
	sc.type = IDC_BASIC_INFO_USB0_STATUS;	
	sc.callback_event = basic_usb0_update;
	add_sub_win_cmd(&sc);

	mt_sys_read_register(0xbf070084, &status);
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "USB1:         %s", (status & 0x01) == 0 ? "disconnected" : "connected");
	sc.cnt = 0;
	sc.rc.y += sc.rc.h + 20;
	sc.type = IDC_BASIC_INFO_USB1_STATUS;
	sc.callback_event = basic_usb1_update;
	add_sub_win_cmd(&sc);

	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sc.cnt = 0;
	sc.type = IDC_BASIC_INFO_PIP_PLAY;
	sc.rc.x = 490;
	sc.rc.y = 200;
	sc.rc.w = 630;
	sc.rc.h = 420;
	sc.is_usr_define = TRUE;
	sc.is_noncursor = TRUE;
	sc.usr_color = 0x00000000;
	sc.callback_event = NULL;
	add_sub_win_cmd(&sc);

	return MT_SUCCESS;
}


