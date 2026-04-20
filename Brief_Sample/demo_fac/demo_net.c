/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mt_type.h"
#include "demo.h"
#include "ui_manager.h"


#define CMD_ETH0_IFCONFIG "ifconfig eth0"
#define CMD_ETH0_PORT_UP "ifconfig eth0 up"
#define CMD_ETH0_PORT_DOWN "ifconfig eth0 down"
#define CMD_ETH0_GET_IP "udhcpc -i eth0"
#define CMD_ETH0_SET_MAC "ifconfig eth0 hw ether"
#define ETH0_MAC_ADDR "00:11:22:33:44:55" //"00:1B:11:17:00:16"

#define MAX_WRITE_BUFF (50)
#define MAX_READ_BUFF (500)

net_info_t g_net_info = {0};

mt_s32 demo_net_parse_info(net_info_t *net_info)
{
	FILE *p_file = NULL;
	mt_char buffer[MAX_READ_BUFF] = {0};
	mt_s32 len = 0;
	mt_char *h_str = NULL, *t_str = NULL;
	
	p_file = popen("ifconfig eth0", "r");
	if(p_file)
	{
		len = fread(buffer, 1, MAX_READ_BUFF, p_file);
		if(len > 0)
		{
			len = len < MAX_READ_BUFF ? len : MAX_READ_BUFF;
			buffer[len] = '\0';
//			printf("%s(line: %d) read info(%d):\n%s\n", __FUNCTION__, __LINE__, len, buffer);
		}
		pclose(p_file);

		h_str = strstr(buffer, "RUNNING");
		if(h_str == NULL)
		{
			net_info->net_status = 0;
			return MT_FAILURE;
		}

		net_info->net_status = 1;
		h_str = strstr(buffer, "HWaddr ");
		if(h_str)
		{
			memcpy(net_info->net_mac, h_str + 7, 17);
		}
		printf("%s(line: %d), get mac: %s\n", __FUNCTION__, __LINE__, net_info->net_mac);
		h_str = strstr(buffer, "inet addr:");
		if(h_str)
		{
			t_str = strstr(h_str + 10, " ");
			memcpy(net_info->net_ip_addr, h_str + 10, t_str - h_str - 10);
			printf("%s(line: %d), get ip: %s\n", __FUNCTION__, __LINE__, net_info->net_ip_addr);
			
			h_str = strstr(h_str, "Mask:");
			if(h_str)
			{
				t_str = strstr(h_str, "\n");
				memcpy(net_info->net_ip_mask, h_str + 5, t_str - h_str - 5);
				printf("%s(line: %d), get ip mask: %s\n", __FUNCTION__, __LINE__, net_info->net_ip_mask);
			}
		}
	}

	memset(buffer, 0, MAX_READ_BUFF);

	p_file = popen("route -n", "r");
	if(p_file)
	{
		len = fread(buffer, 1, MAX_READ_BUFF, p_file);
		if(len > 0)
		{
			len = len < MAX_READ_BUFF ? len : MAX_READ_BUFF;
			buffer[len] = '\0';
			printf("%s(line: %d) read info(%d):\n%s\n", __FUNCTION__, __LINE__, len, buffer);
		}
		pclose(p_file);

		h_str = strstr(buffer, "Iface");
		if(h_str)
		{
			t_str = strstr(h_str, ".");
			if(t_str)
			{
				h_str = h_str + 6 + 15 + 1;	//'\n'+destination+' '
				memcpy(net_info->net_gateway, h_str, 15);
				printf("%s(line: %d), gateway: %s\n", __FUNCTION__, __LINE__, net_info->net_gateway);

				h_str = h_str + 1 + 15;
				memcpy(net_info->net_gate_mask, h_str, 15);
				printf("%s(line: %d), gateway mask: %s\n", __FUNCTION__, __LINE__, net_info->net_gate_mask);
			}
		}
	}

	memset(buffer, 0, MAX_READ_BUFF);
	p_file = popen("cat /etc/resolv.conf", "r");
	if(p_file)
	{
		len = fread(buffer, 1, MAX_READ_BUFF, p_file);
		if(len > 0)
		{
			len = len < MAX_READ_BUFF ? len : MAX_READ_BUFF;
			buffer[len] = '\0';
			printf("%s(line: %d) read info(%d):\n%s\n", __FUNCTION__, __LINE__, len, buffer);
		}
		pclose(p_file);

		h_str = strstr(buffer, "nameserver");
		if(h_str)
		{
			t_str = strstr(h_str, "\n");
			if(t_str)
			{
				h_str += 11;
				memcpy(net_info->net_dns, h_str, t_str - h_str);
			}
			else
			{
				h_str += 11;
				memcpy(net_info->net_dns, h_str + 11, len - (h_str - buffer));
			}
			printf("%s(line: %d), dns: %s\n", __FUNCTION__, __LINE__, net_info->net_dns);
		}
	}
	else
	{
		printf("%s(line: %d), open resov file failed.\n", __FUNCTION__, __LINE__);
	}

	return MT_SUCCESS;
}


mt_s32 demo_net_monitor(void)
{
	FILE *p_file = NULL;
	mt_char r_buffer[MAX_READ_BUFF] = {0};
	mt_s32 len = 0;
	
	while(1)
	{
		p_file = popen(CMD_ETH0_IFCONFIG, "r");
		if(p_file)
		{
			memset(r_buffer, 0, MAX_READ_BUFF);
			len = fread(r_buffer, 1, MAX_READ_BUFF, p_file);
			if(len > 0)
			{
				len = len < MAX_READ_BUFF ? len : MAX_READ_BUFF;
				r_buffer[len] = '\0';
//				printf("%s(line: %d) read info:\n%s\n", __FUNCTION__, __LINE__, r_buffer);
			}
			pclose(p_file);
			if(strstr(r_buffer, "RUNNING"))
			{
				if(g_net_info.net_status == 0)
				{
					system(CMD_ETH0_GET_IP);
					demo_net_parse_info(&g_net_info);
					manage_update_sub_event(ROOT_ID_NET, 1, g_net_info.net_ip_addr);
					manage_update_sub_event(ROOT_ID_NET, 2, g_net_info.net_ip_mask);
					manage_update_sub_event(ROOT_ID_NET, 3, g_net_info.net_gateway);
					manage_update_sub_event(ROOT_ID_NET, 4, g_net_info.net_gate_mask);
					manage_update_sub_event(ROOT_ID_NET, 5, g_net_info.net_dns);
					manage_update_sub_event(ROOT_ID_NET, 6, &g_net_info.net_status);
					manage_update_ui(UPDATE_MODULE, ROOT_ID_NET);
				}
			}
			else
			{
				if(g_net_info.net_status == 1)
				{
					g_net_info.net_status = 0;
					manage_update_sub_event(ROOT_ID_NET, 6, &g_net_info.net_status);
					manage_update_ui(UPDATE_MODULE, ROOT_ID_NET);
				}
			}
		}
		usleep(3*500*1000);
	}
}

mt_s32 demo_net_init(void)
{
	pthread_t net_task;

	sprintf(g_net_info.net_mac, ETH0_MAC_ADDR);
	sprintf(g_net_info.net_ip_addr, "0.0.0.0");
	sprintf(g_net_info.net_ip_mask, "0.0.0.0");
	sprintf(g_net_info.net_gateway, "0.0.0.0");
	sprintf(g_net_info.net_gate_mask, "0.0.0.0");
	sprintf(g_net_info.net_dns, "0.0.0.0");
	g_net_info.net_status = 0;
	
	//eth0 down
	system(CMD_ETH0_PORT_DOWN);

	//set eth0 mac
	system(CMD_ETH0_SET_MAC);

	//eth0 up
	system(CMD_ETH0_PORT_UP);

	pthread_create(&net_task, NULL, demo_net_monitor, NULL);

	return MT_SUCCESS;
	
}

mt_s32 demo_net_fetch_info(mt_char *net_info)
{
	if(net_info == NULL)
	{
		printf("%s(line: %d), param invalid.\n", __FUNCTION__, __LINE__);
		return MT_FAILURE;
	}

	memcpy(net_info, &g_net_info, sizeof(net_info_t));
	return MT_SUCCESS;
}


