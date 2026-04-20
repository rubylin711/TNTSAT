
#include "init.h"
#include "core.h"
#include "wlan_def.h"
#include "ctrl_intf.h"
#include "lynx_debug.h"
#include "wm_vif.h"
#include "wm_msg.h"
#include "wm_ctrl.h"

#ifdef CONFIG_LYNX_WM_MANAGER

#if 0
static int calc_signal_level(int rssi)
{
#define NUM_LEVEL 100
#define MIN_RSSI -100
#define MAX_RSSI -55
	int level = 0;

	if (rssi <= MIN_RSSI) {
		level = 0;
	} else if (rssi >= MAX_RSSI) {
		level = NUM_LEVEL - 1;
	} else {
		level = (int)((float)(rssi - MIN_RSSI) * (NUM_LEVEL - 1) / (MAX_RSSI - MIN_RSSI));
	}

	return level;
}
#endif

static void wifi_ap_list_dump(struct seq_file *s)
{
	int i;
	struct neighbor_bss *nbss=NULL, *bss_list;
	int tkip=0, ccmp=0;

	if(s == NULL)
		return;

	bss_list = wm_ctrl_scan_results();
	
	seq_printf(s, "ap list:\n");

	for(i = 0; i < MAX_NEIGHBOR_BSS_NUM; i++)
	{
		nbss = &bss_list[i];
		
		if(!(nbss->flag & NBSS_ENABLE))
			continue;

		tkip=0;
		ccmp=0;

		seq_printf(s, "ssid=%s / bssid=%02x:%02x:%02x:%02x:%02x:%02x / ", nbss->ssid, 
				nbss->addr[0], nbss->addr[1], 
				nbss->addr[2], nbss->addr[3], 
				nbss->addr[4], nbss->addr[5]);
		seq_printf(s, "security=(");
		if(nbss->sec & NBSS_SEC_WPA2)
		{
			seq_printf(s, "wpa2 ");
			if(nbss->rsn_info)
			{
				if(nbss->rsn_info->pairwise_cipher & AUTH_CAP_CIPHER_CCMP)
					ccmp = 1;
				if(nbss->rsn_info->pairwise_cipher & AUTH_CAP_CIPHER_TKIP)
					tkip = 1;
			}
		}
		if(nbss->sec & NBSS_SEC_WPA)
		{
			seq_printf(s, "wpa");
			if(nbss->wpa_info)
			{
				if(nbss->wpa_info->pairwise_cipher & AUTH_CAP_CIPHER_CCMP)
					ccmp = 1;
				if(nbss->wpa_info->pairwise_cipher & AUTH_CAP_CIPHER_TKIP)
					tkip = 1;
			}
		}
		if(nbss->sec & NBSS_SEC_WEP)
			seq_printf(s, "WEP");
		if(nbss->sec & NBSS_SEC_NON)
			seq_printf(s, "NONE");
		seq_printf(s, ") / cipher=(");
		if(tkip)
			seq_printf(s, "TKIP ");
		if(ccmp)
			seq_printf(s, "CCMP");
		seq_printf(s, ") / channel=%d / rssi=%d \n", nbss->channel, nbss->rssi);
	}
}

int set_wifi_config_lynx(int argc, char *argv[])
{
	int i=0;
	int vif_idx=0, key_len=0;
	char *ssid=NULL, *key=NULL;
	unsigned int security=AUTH_CAP_OPEN, cipher=AUTH_CAP_CIPHER_NONE;

	/* FIXME: /proc only apply 8 argv. And then, bypass cipher config when test */

	for(i=0; i<argc; i++)
	{
		if(strcmp(argv[i], "vif_idx") == 0)
			sscanf(argv[++i], "%x", &vif_idx);
		else if(strcmp(argv[i], "ssid") == 0)
			ssid = argv[++i];
		else if(strcmp(argv[i], "security") == 0)
		{
			i++;
			if(strcmp(argv[i], "WPA2") == 0)
			{
				security = AUTH_CAP_WPA2;
				cipher = AUTH_CAP_CIPHER_CCMP;
			}
			else if(strcmp(argv[i], "WPA") == 0)
			{
				security = AUTH_CAP_WPA;
				cipher = AUTH_CAP_CIPHER_TKIP;
			}
			else if(strcmp(argv[i], "WEP") == 0)
				security = AUTH_CAP_WEP;

			/* other case is security = AUTH_CAP_OPEN */
		}
		else if(strcmp(argv[i], "cipher") == 0)
		{
			i++;
			if(strcmp(argv[i], "CCMP") == 0)
				cipher = AUTH_CAP_CIPHER_CCMP;
			else if(strcmp(argv[i], "TKIP") == 0)
				cipher = AUTH_CAP_CIPHER_TKIP;
			else if(strcmp(argv[i], "WEP40") == 0)
				cipher = AUTH_CAP_CIPHER_WEP40;
			else if(strcmp(argv[i], "WEP104") == 0)
				cipher = AUTH_CAP_CIPHER_WEP104;

			/* other case is security = AUTH_CAP_CIPHER_NONE */
		}
		else if(strcmp(argv[i], "key") == 0)
		{
			key = argv[++i];
			key_len = strlen(key);

			if(security == AUTH_CAP_WEP)
			{
				if((key_len == WLAN_WEP_40BIT_KEYLEN) || (key_len == (WLAN_WEP_40BIT_KEYLEN * 2)))
					cipher = AUTH_CAP_CIPHER_WEP40;
				else if((key_len == WLAN_WEP_104BIT_KEYLEN) || (key_len == (WLAN_WEP_104BIT_KEYLEN * 2)))
					cipher = AUTH_CAP_CIPHER_WEP104;
			}
		}
	}

	printk("%s(): ssid=%s, security=%x\n, cipher=%x\n", __FUNCTION__, ssid, security, cipher);
	if(key)
		printk("%s(): key_len=%d, key=%s\n", __FUNCTION__, key_len, key);

	/* TODO: addr? */
	wm_ctrl_set_config(vif_idx, NULL, ssid, security, cipher, key, key_len);
	return 0;
}

void set_wifi_connect_lynx(int vif_idx)
{
	/* TODO: support multiple bss */
	wm_ctrl_connect(vif_idx);
}

void set_wifi_disconnect_lynx(int vif_idx)
{
	/* TODO: support multiple bss */
	wm_ctrl_disconnect(vif_idx);
}


int wm_cmd_get(int cmd, struct seq_file *s)
{
	if(cmd == WM_GET_SCAN_RESULTS)
		wifi_ap_list_dump(s);
	else
		seq_printf(s, "WM: not support cmd %d\n", cmd);

	return 0;
}

int wm_cmd_set(int argc, char *argv[])
{
	if(argc < 1)
		return -1;

	if(strcmp(argv[0], "scan") == 0)
		wm_ctrl_scan(0);
	else if(strcmp(argv[0], "config") == 0)
		set_wifi_config_lynx(argc - 1, &argv[1]);
	else if(strcmp(argv[0], "connect") == 0)
		set_wifi_connect_lynx(0);
	else if(strcmp(argv[0], "disconnect") == 0)
		set_wifi_disconnect_lynx(0);
	else
		printk("%s(): not support cmd %s\n", __FUNCTION__, argv[0]);

	return 0;
}

#endif	// CONFIG_LYNX_WM_MANAGER
