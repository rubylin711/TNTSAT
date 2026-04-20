/*=============================================================================+
|                                                                              |
| Copyright 2013                                                               |
| Acrospeed Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*!
*   \file
*   \brief
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <linux/module.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/interrupt.h>
#include <linux/ethtool.h>
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif
#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "lynx_debug.h"
#include "mont_ioctl.h"
#include "wci.h"
#include "utility.h"
#include "hif.h"
#include "hw.h"
#include "usb.h"

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define MAX_ARGV 8

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
unsigned int mp_test_firm = 0;

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern void lynx_wci_stop_cmd(struct lynx *lnx);
extern int lynx_android_priv_cmd(struct net_device *dev, struct ifreq *ifr, int cmd);

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
//static struct lynx *dbglnx;


struct mont_ioctl_cmd_data cmd_list[] = {
	{MONT_IOCTL_GET_CMD_LIST,	0, (PRIV_TYPE_CHAR | PRIV_RECV_BUF_SIZE)						 ,	"get_cmd_list"},
	{MONT_IOCTL_HOST_INFO,	0, (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE),	".host_info"},
	{MONT_IOCTL_DBG_LVL,	0, (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)								 ,	"dbg_lvl"},
	{MONT_IOCTL_BSS,(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"bss"},
	{MONT_IOCTL_HT_CAP,(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE),	"ht_cap"},
	{MONT_IOCTL_TX_PENDING,	0, (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)								 ,	"tx_pending"},
	{MONT_IOCTL_TX_RATE,	0, (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)								 ,	"tx_rate"},
	{MONT_IOCTL_RSSI,	0, (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)								 	 ,	"rssi"},
	{MONT_IOCTL_PS,	0, (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)										 ,	"ps"},
	{MONT_IOCTL_MIN_RATEIDX,	0, (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)							 ,	"min_rateidx"},
	{MONT_IOCTL_ADDBA,	 (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE),	 (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE) ,	"addba"},
	{MONT_IOCTL_EN_2040_COEX,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE),	"en_2040_coex"},
	{MONT_IOCTL_WT,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"wt"},
	{MONT_IOCTL_MP,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"mp"},
	{MONT_IOCTL_MP_START,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE),	"mp_start"},
	{MONT_IOCTL_WT,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"bb"},
	{MONT_IOCTL_WT,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"rf"},
	{MONT_IOCTL_WT,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"wd"},
	{MONT_IOCTL_WT,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"otp"},
	{MONT_IOCTL_MON_MASK,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"mon"},
	{MONT_IOCTL_MON_LEVEL,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"mon_level"},
	{MONT_IOCTL_BW40,	(PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE), (PRIV_TYPE_INT | PRIV_RECV_BUF_SIZE)	 ,	"bw40"},
};

module_param(mp_test_firm, uint, 0);
MODULE_PARM_DESC(mp_test_firm, "MP test start\n");

static int lynx_calc_tx_rate_avg(unsigned char rate_idx)
{
	int rate[] = {512, 1024, 2816, 5632, 3072, 4068, 6144, 9216, 12288, 18432, 24567, 27648, 6*512, 13*512, 19*512, 26*512, 39*512, 52*512, 58*512, 65*512};
	/* rate[] = {CCK_1M, CCK_2M, CCK_5_5M, OFDM_6M, OFDM_9M, OFDM_12M, OFDM_18M, OFDM_24M, OFDM_36M, OFDM_48M, OFDM_54M, MCS_0, MCS_1, MCS_2, MCS_3, MCS_4, MCS_5, MCS_6, MCS_7} */

	//printk("%s(): idx=%d, rate=%d\n", __FUNCTION__, rate_idx, rate[rate_idx]);

	return (rate[rate_idx]*2);
}

int init_handshake_data(struct mont_ioctl_data *cmd)
{
	memset((char *)cmd->data, 0, cmd->total_length);
	cmd->used_length=0;
	return 0;
}

int lynx_ioctl_host_info(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	struct lynx_vif *vif=netdev_priv(dev);
	struct lynx_hif_device *hif=(struct lynx_hif_device *)vif->lnx->hif_priv;

	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;
	init_handshake_data(cmd);
	sprintf((char *)cmd->data,"%d %d",hif->hif_type,mp_test_firm);
	cmd->used_length=strlen((char *)cmd->data);
	return 0;
}

int lynx_ioctl_mp_start(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
#ifndef CONFIG_MP_FW
	init_handshake_data(cmd);
	sprintf((char *)cmd->data + strlen((char *)cmd->data) , "Compile option is not including mp fw.\n");
	cmd->used_length=strlen((char *)cmd->data);
	return -EINVAL;
#else
	struct lynx_hif_device *hdev;
	struct lynx *lynx = lynx_priv(dev);
	unsigned int value;

	sscanf((char *)cmd->data, "%d", &value);
	//printk(KERN_CRIT "mp start = %x\n", value);
	mp_test_firm = value;
	hdev = lynx_alloc_hdev(sizeof(struct lynx_usb_device));
	hdev->lynx = lynx;

	lynx_wci_stop_cmd(hdev->lynx);
	msleep(300);
	lynx_free_hdev(hdev);
	hdev = NULL;
	return 0;
#endif
}

int lynx_ioctl_dynamic_access_device(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	struct lynx_vif *vif=netdev_priv(dev);

	lynx_wci_dynamic_access_cmd(vif, cmd);
	return 0;
}

int lynx_ioctl_en_2040_coex(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	int enable=0;
	char buf[512]={0};
	char *argv[MAX_ARGV];
	int argc;
	struct lynx_vif *vif=netdev_priv(dev);

	if(!cmd->used_length)
		goto en_2040_coex_help;
	else
	{
		memcpy(buf, (char *)cmd->data, cmd->used_length+1);
		init_handshake_data(cmd);
		argc=str2args(buf,argv," ",MAX_ARGV);
	}

	if(argc==1)
	{
		sscanf(argv[0], "%d", &enable);
		lynx_wci_set_2040_coex_cmd(vif, enable);
	}
	else
	{
		en_2040_coex_help:
			init_handshake_data(cmd);
			sprintf((char *)cmd->data + strlen((char *)cmd->data) , "en_2040_coex <bss_idx> <enable>\n");
			cmd->used_length=strlen((char *)cmd->data);
			return -EINVAL;
	}
	return 0;
}

int lynx_ioctl_addba(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	char buf[512]={0};
	char *argv[MAX_ARGV];
	int argc;
	struct lynx_vif *vif=netdev_priv(dev);
	int sta_idx=0, tid=0;
	if(!cmd->used_length)
		goto addba_help;
	else
	{
		memcpy(buf, (char *)cmd->data, cmd->used_length+1);
		init_handshake_data(cmd);
		argc=str2args(buf,argv," ",MAX_ARGV);
	}
	if(argc == 2)
	{
		sscanf(argv[0], "%d", &sta_idx);
		sscanf(argv[1], "%d", &tid);

		if(!((1 << sta_idx) & (vif->sta_idx_map)))
		{
			for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
			{
				if((1 << sta_idx) & (vif->sta_idx_map))
					break;
			}

			if(sta_idx >= LYNX_STA_MAX_NUM)
				return 0;
		}

		if(tid > 7)
			tid = 0;

		lynx_wci_send_addba_cmd(vif, sta_idx, tid);
	}
	else
	{
		addba_help:
			init_handshake_data(cmd);
			sprintf((char *)cmd->data + strlen((char *)cmd->data) , "addba <bss_idx> <sta_idx> <tid>\n");
			return -EINVAL;
	}
	return 0;
}

int lynx_ioctl_min_rateidx(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	unsigned int val;
	struct lynx_vif *vif=netdev_priv(dev);
	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;
	if(!cmd->used_length)
	{
		sprintf((char *)cmd->data, "min_rateidx val\n");
		cmd->used_length=strlen((char *)cmd->data);
		return -EINVAL;
	}
	else
	{
		sscanf((char *)cmd->data, "%d", &val);
		lynx_vif_cfg(vif->lnx, vif, VIF_CFG_MIN_RATEIDX, val);
	}
	return 0;
}


int lynx_ioctl_ps(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	unsigned int val;
	struct lynx_vif *vif=netdev_priv(dev);
	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;
	if(!cmd->used_length)
	{
		sprintf((char *)cmd->data,"ps val\n");
		cmd->used_length=strlen((char *)cmd->data);
		return -EINVAL;
	}
	else
	{
		sscanf((char *)cmd->data, "%d", &val);
		lynx_vif_cfg(vif->lnx, vif, VIF_CFG_PS, val);
		if (val == 0)
			vif->wdev.ps =false;
		else
			vif->wdev.ps =true;
	}
	return 0;
}

int lynx_ioctl_monitor_level(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	unsigned int val;
	struct lynx_vif *vif=netdev_priv(dev);
	if (NULL == vif){
		return -EINVAL;
	}

	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;
	if(!cmd->used_length){
		sprintf((char *)cmd->data,"monitor_level val\n");
		cmd->used_length=strlen((char *)cmd->data);
		return -EINVAL;
	}
	else{
		sscanf((char *)cmd->data, "%d", &val);
#ifdef CONFIG_SUPPORT_MONITOR_MODE
		lynx_monitor_switch(vif,val);
#else
		printk("Not configure monitor mode at menuconfig.\n");
#endif /*CONFIG_SUPPORT_MONITOR_MODE*/
	}
	return 0;
}

int lynx_ioctl_rssi(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	int sta_idx = 0;
	struct lynx_vif *vif=netdev_priv(dev);
	struct lynx_sta *sta=NULL;
	struct lynx *lnx;

	if(vif->sta_idx_map == 0)
		return 0;

	for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
	{
		if((vif->sta_idx_map & (1 << sta_idx)) != 0)
			break;
	}

	if((sta_idx >= LYNX_STA_MAX_NUM) || (vif->lnx->sta_tx_rate[sta_idx] == 0))
		return 0;

	init_handshake_data(cmd);
	lnx = vif->lnx;
	sta = &lnx->sta_list[sta_idx];

	if(sta->flags & LYNX_STA_VALID)
	{
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "sta[%d] last signal = %d dBm\n", sta_idx, bb_rssi_decode(sta->signal_last, RSSI_OFFSET));
		cmd->used_length=strlen((char *)cmd->data);
	}

	return 0;
}

int lynx_ioctl_tx_rate(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	int rate = 0;
	int sta_idx = 0;
	struct lynx_vif *vif=netdev_priv(dev);

	if(vif->sta_idx_map == 0)
		return 0;

	for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
	{
		if((vif->sta_idx_map & (1 << sta_idx)) != 0)
			break;
	}

	if((sta_idx >= LYNX_STA_MAX_NUM) || (vif->lnx->sta_tx_rate[sta_idx] == 0))
		return 0;
	init_handshake_data(cmd);
	rate = lynx_calc_tx_rate_avg(vif->lnx->sta_tx_rate[sta_idx] - 1);
	sprintf((char *)cmd->data + strlen((char *)cmd->data) , "sta[%d] tx_rate(%d) : %dk\n", sta_idx, vif->lnx->sta_tx_rate[sta_idx], rate);
	cmd->used_length=strlen((char *)cmd->data);
	return 0;
}
int lynx_ioctl_tx_pending(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	struct lynx_vif *vif=netdev_priv(dev);
	struct lynx_hif_device *lynx_usb;
	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;
	init_handshake_data(cmd);
	lynx_usb = (struct lynx_hif_device *)vif->lnx->hif_priv;
	sprintf((char *)cmd->data + strlen((char *)cmd->data) , "%d\n", lynx_usb->tx_buf_cnt);
	cmd->used_length=strlen((char *)cmd->data);
	return 0;
}

int lynx_ioctl_ht_cap(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	unsigned int value;

	struct lynx_vif *vif=netdev_priv(dev);

	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;

	if(!cmd->used_length)
	{
		struct wm_bss *bss;
		init_handshake_data(cmd);
		bss = &vif->bss;
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "vif ht_capability : 0x%x\n\n", be16_to_cpu(vif->bss.ht_capability));
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "IEEE80211_HT_CAP_DSSSCCK40 = 0x%x\n", IEEE80211_HT_CAP_DSSSCCK40);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "IEEE80211_HT_CAP_SUP_WIDTH_20_40 = 0x%x\n", IEEE80211_HT_CAP_SUP_WIDTH_20_40);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "IEEE80211_HT_CAP_GRN_FLD = 0x%x\n", IEEE80211_HT_CAP_GRN_FLD);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "IEEE80211_HT_CAP_SGI_40 = 0x%x\n", IEEE80211_HT_CAP_SGI_40);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "IEEE80211_HT_CAP_SGI_20 = 0x%x\n", IEEE80211_HT_CAP_SGI_20);
		cmd->used_length=strlen((char *)cmd->data);
	}
	else
	{
		sscanf((char *)cmd->data, "%x", &value);
		lynx_vif_cfg(vif->lnx, vif, VIF_CFG_HT_CAP, value);
		init_handshake_data(cmd);
	}
	return 0;
}
int lynx_ioctl_dbg_lvl(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
#ifdef CONFIG_LYNX_DEBUG
	unsigned int value;
	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;

	if(!cmd->used_length) //get
	{
		init_handshake_data(cmd);
		sprintf((char *)cmd->data, "%x", lynx_get_dbg_lvl());
		cmd->used_length=strlen((char *)cmd->data);
	}
	else //set
	{
		sscanf((char *)cmd->data,"%x",&value);
		lynx_set_dbg_lvl(value);
		init_handshake_data(cmd);
	}

	return 0;
#else

	return -EINVAL;
#endif
}

int lynx_ioctl_get_cmd_list(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	/* reference from iw_handler_get_private() */
	int ret = 0;
	int list_size = sizeof(cmd_list);

	if(!cmd->data)
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(%d): none return pointer\n", __FUNCTION__, __LINE__);
		ret = -EINVAL;
	}
	else if(cmd->total_length < list_size)
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(%d): the user space buffer is too small\n", __FUNCTION__, __LINE__);
		cmd->used_length = list_size;
		ret = -E2BIG;
	}
	else
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(%d): get cmd list ok\n", __FUNCTION__, __LINE__);
		cmd->used_length = list_size;

		if(copy_to_user((char *)cmd->data, cmd_list, list_size) != 0)
			ret = -EINVAL;
	}

	return ret;
}

int lynx_ioctl_bss(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	char buf[128]={0};
	int ret = -EINVAL;
	char *argv[MAX_ARGV];
	int argc;
	struct lynx_vif *vif=netdev_priv(dev);

	if(cmd->total_length < sizeof(unsigned int))
		return -E2BIG;

	if(!cmd->used_length) //show
	{
		struct wm_bss *bss;

		bss = &vif->bss;
		sprintf((char *)cmd->data + strlen((char *)cmd->data) ,"BSS %d :\n", bss->bss_desc);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "myaddr : %02x:%02x:%02x:%02x:%02x:%02x\n",
				bss->myaddr[0], bss->myaddr[1], bss->myaddr[2],
				bss->myaddr[3], bss->myaddr[4], bss->myaddr[5]);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "bssid : %02x:%02x:%02x:%02x:%02x:%02x\n",
				bss->bssid[0], bss->bssid[1], bss->bssid[2],
				bss->bssid[3], bss->bssid[4], bss->bssid[5]);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "ssid : %s\n", bss->ssid);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "role : %d\n", bss->role);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "phy_cap : 0x%x\n", bss->phy_cap);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "ht_capability : 0x%x\n", bss->ht_capability);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "auth_capability : 0x%x\n", bss->auth_capability);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "slottime : 0x%x\n", bss->slottime);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "flag : 0x%x\n", bss->flag);
		sprintf((char *)cmd->data + strlen((char *)cmd->data) , "ampdu_params: %d\n",bss->ampdu_params);
		ret =0;
	}
	else
	{
		memcpy(buf, (char *)cmd->data, cmd->used_length+1);
		argc=str2args(buf,argv," ",MAX_ARGV);
		init_handshake_data(cmd);
		if(argc >= 2)
		{
			struct wm_bss *bss;
			int val=0;

#ifdef CONFIG_UNIT_TEST
			unittest_bss_compare(vif, __FUNCTION__);
#endif
			bss = &vif->bss;
			sscanf(argv[1], "%d", &val);
			bss = &vif->bss;

			if(!strcmp(argv[0], "phy_cap"))
			{
				if(val <= 7)
					bss->phy_cap = val;
			}
			else if(!strcmp(argv[0], "slottime"))
			{
				//if(val <= 2)
				bss->slottime = val;
				cpu_to_be16s(&(bss->slottime));
			}
			else if(!strcmp(argv[0], "ampdu_params"))
			{
				if(val <= 2)
					bss->ampdu_params = val;
			}
			else
			{
				goto bss_help;
			}
			/* trigger the vif update */
			lynx_wci_update_vif_cmd(vif);
			ret=0;

#ifdef CONFIG_UNIT_TEST
			unittest_bss_backup(vif, __FUNCTION__);
#endif
		}
		else
		{
			goto bss_help;
		}
	}
    cmd->used_length = strlen((char *)cmd->data);

return ret;

bss_help:
	sprintf((char *)cmd->data + strlen((char *)cmd->data),"bss [option] value\n");
	sprintf((char *)cmd->data + strlen((char *)cmd->data),"bss\n\tShow bss information\n");
	sprintf((char *)cmd->data + strlen((char *)cmd->data),"bss phy_cap [val] \n\tSet phy_cap\n");
	sprintf((char *)cmd->data + strlen((char *)cmd->data),"bss slottime [val]\n\tSet slottime\n");
	sprintf((char *)cmd->data + strlen((char *)cmd->data),"bss ampdu_params [val]\n\tSet ampud_params\n");
	cmd->used_length = strlen((char *)cmd->data);
	ret=0;
	return ret;

}

int lynx_ioctl_mon_mask(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
#ifdef CONFIG_SUPPORT_MONITOR_MODE
	struct lynx_vif *vif = netdev_priv(dev);
	unsigned int value = 0;

	if(!cmd->used_length) {
        //get
        lynx_wci_get_monitor_cmd(vif);
        msleep(100);
        cmd->data = vif->lnx->mon_mask;
        printk("%s: get %x\n", __func__, vif->lnx->mon_mask);
	}
	else {
        //set
        value = cmd->data;
        lynx_wci_set_monitor_cmd(vif, value);
        printk("%s: set %x\n", __func__, value);
	}

	return 0;
#else
	return -EINVAL;
#endif
}
int lynx_ioctl_bw40(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd)
{
	struct lynx_vif *vif = netdev_priv(dev);
	int en_40mhz=0;

	if(!cmd->used_length) {
		return -EINVAL;
	}
	else
	{
		sscanf((u8*)cmd->data, "%d", &en_40mhz);

		if(en_40mhz)
			vif->lnx->en_40mhz = 1;
		else
			vif->lnx->en_40mhz = 0;

		en_40mhz = cpu_to_be32(en_40mhz);

		lynx_wci_set_special_param_cmd(vif, WCI_SET_EN40MHZ, sizeof(en_40mhz), (char *)&en_40mhz);
	}
	return 0;
}
int lynx_ioctl_ptiv(struct net_device *dev, struct ifreq *ifr)
{
	struct mont_ioctl_data cmd;
	int ret = -EINVAL;

	if(ifr && ifr->ifr_data)
	{
		if(copy_from_user((char *)&cmd, ifr->ifr_data, sizeof(struct mont_ioctl_data)) == 0)
		{
			if(cmd.cmd_id < sizeof(cmd_list)/sizeof(struct mont_ioctl_cmd_data))
			{
				lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): cmd = %s\n", __FUNCTION__, cmd_list[cmd.cmd_id].name);
				ret = lynx_private_handler[cmd.cmd_id](dev, ifr, &cmd);
				if(copy_to_user(ifr->ifr_data, &cmd, sizeof(struct mont_ioctl_data)) != 0)
					ret = -EINVAL;
			}
		}
	}

	return ret;
}


int lynx_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret=-EOPNOTSUPP;
	//RR
	if(cmd == SIOCDEVPRIVATE)
		ret = lynx_ioctl_ptiv(dev, ifr);
#if defined(CONFIG_ANDROID) && !defined(CONFIG_MT_CHIP_SYMPHONY4)
	else if(cmd == SIOCDEVPRIVATE+1)
		ret = lynx_android_priv_cmd(dev, ifr, cmd);
#endif
	else
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): not support command 0x%x\n", __FUNCTION__, cmd);

	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): ret = %d\n", __FUNCTION__, ret);

	return ret;
}

