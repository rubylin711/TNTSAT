/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_manager.h"

enum net_info_sub_idx
{
	IDC_NET_MAC_ADDRESS,
	IDC_NET_IP_ADDRESS,
	IDC_NET_IP_MASK,
	IDC_NET_GATEWAY,
	IDC_NET_GATEWAY_MASK,
	IDC_NET_DNS,
	IDC_NET_STATUS,
	IDC_FRONT_MAX
};

mt_s32 net_callback_event(mt_u16 evt_id, mt_u32 param)
{
	mt_s32 ret;

	ret = demo_av_set_mute(TRUE);

	return ret;
}

mt_s32 net_ip_addr_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_char *ip_addr = (mt_char*)&para2;

	if(ip_addr)
	{
		printf("%s(line: %d), %s\n", __FUNCTION__, __LINE__, ip_addr);
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "IP:                        %s", ip_addr);
	}

	return MT_SUCCESS;
}

mt_s32 net_ip_mask_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_char *ip_mask = (mt_char*)&para2;

	if(ip_mask)
	{
		printf("%s(line: %d), %s\n", __FUNCTION__, __LINE__, ip_mask);
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "IP Mask:                %s", ip_mask);
	}

	return MT_SUCCESS;
}

mt_s32 net_gateway_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_char *gateway = (mt_char*)&para2;

	if(gateway)
	{
		printf("%s(line: %d), %s\n", __FUNCTION__, __LINE__, gateway);
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "Gateway:              %s", gateway);
	}

	return MT_SUCCESS;
}

mt_s32 net_gateway_mask_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_char *mask = (mt_char*)&para2;

	if(mask)
	{
		printf("%s(line: %d), %s\n", __FUNCTION__, __LINE__, mask);
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "Gateway Mask:         %s", mask);
	}

	return MT_SUCCESS;
}

mt_s32 net_dns_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_char *dns = (mt_char*)&para2;

	if(dns)
	{
		printf("%s(line: %d), %s\n", __FUNCTION__, __LINE__, dns);
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "DNS:                        %s", dns);
	}

	return MT_SUCCESS;
}

mt_s32 net_status_update(mt_u16 evt_id, mt_u32 para1, mt_u32 para2)
{
	struct SubCmd *sc = (struct SubCmd *)&para1;
	mt_u16 status = *(mt_u16*)&para2;

	if(status == 0)
	{
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "Status:                     disconnected");
	}
	else
	{
		memset(sc->p_dis_text, 0, MAX_TEXT_LEN);
		sprintf(sc->p_dis_text, "Status:                     connected");
	}

	return MT_SUCCESS;
}


mt_s32 open_net(mt_u32 para1, mt_u32 para2)
{
	MT_RECT rc;
	struct SubCmd sc = {0};
	mt_char net_info[100] = {0};
	mt_char *p_tmp = NULL;
	
	rc.x = 150 + ROOT_ID_NET * 170;
	rc.y = 100;
	rc.w = 170;
	rc.h = 50;
	add_main_win("Net", ROOT_ID_NET, rc, net_callback_event);

	sc.m_cursor = get_main_win_index(ROOT_ID_NET); 

	demo_net_fetch_info(net_info);
	p_tmp = net_info;
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "MAC:                    %s", p_tmp);
	sc.cnt = 0;
	sc.type = IDC_NET_MAC_ADDRESS;
	sc.rc.x = 500;
	sc.rc.y = 200;
	sc.rc.w = 600;
	sc.rc.h = 50;
	add_sub_win_cmd(&sc);

	p_tmp += 18;
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "IP:                        %s", p_tmp);
	sc.type = IDC_NET_IP_ADDRESS;
	sc.rc.y += sc.rc.h;
	sc.callback_event = net_ip_addr_update;
	add_sub_win_cmd(&sc);

	p_tmp += 16;
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "IP Mask:                %s", p_tmp);
	sc.type = IDC_NET_IP_MASK;
	sc.rc.y += sc.rc.h;
	sc.callback_event = net_ip_mask_update;
	add_sub_win_cmd(&sc);

	p_tmp += 16;
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Gateway:              %s", p_tmp);
	sc.type = IDC_NET_GATEWAY;
	sc.rc.y += sc.rc.h;
	sc.callback_event = net_gateway_update;
	add_sub_win_cmd(&sc);

	p_tmp += 16;
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "Gateway Mask:         %s", p_tmp);
	sc.type = IDC_NET_GATEWAY_MASK;
	sc.rc.y += sc.rc.h;
	sc.callback_event = net_gateway_mask_update;
	add_sub_win_cmd(&sc);

	p_tmp += 16;
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	sprintf(sc.p_dis_text, "DNS:                        %s", p_tmp);
	sc.type = IDC_NET_DNS;
	sc.rc.y += sc.rc.h;
	sc.callback_event = net_dns_update;
	add_sub_win_cmd(&sc);

	p_tmp += 16;
	memset(sc.p_dis_text, 0, MAX_TEXT_LEN);
	if(*(mt_u16*)p_tmp == 0)
		sprintf(sc.p_dis_text, "Status:                     disconnected");
	else
		sprintf(sc.p_dis_text, "Status:                     connected");
	sc.type = IDC_NET_STATUS;
	sc.rc.y += sc.rc.h;
	sc.is_noncursor = TRUE;
	sc.callback_event = net_status_update;
	add_sub_win_cmd(&sc);

	return MT_SUCCESS;
}


