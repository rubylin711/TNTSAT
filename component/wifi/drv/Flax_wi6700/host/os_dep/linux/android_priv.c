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
#include <linux/etherdevice.h>
#include <net/mac80211.h>
#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "lynx_debug.h"

#ifdef CONFIG_HOST_WPS
#ifdef CONFIG_HOST_P2P
#include "cfg80211.h"
#include "wci.h"
#define ANDROID_CMD_SET_AP_WPS_P2P_IE	"SET_AP_WPS_P2P_IE"
#endif /*CONFIG_HOST_P2P*/
#endif /*CONFIG_HOST_WPS*/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#define strnicmp	strncasecmp
#endif /* Linux kernel >= 4.0.0 */

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
/* reference from wpa_supplicant: defs.h */
enum set_band {
	WPA_SETBAND_AUTO,
	WPA_SETBAND_5G,
	WPA_SETBAND_2G
};

typedef struct android_wifi_priv_cmd {
	char __user *buf;
	int used_len;
	int total_len;
} android_wifi_priv_cmd;

#ifdef CONFIG_COMPAT
typedef struct _compat_android_wifi_priv_cmd {
	compat_caddr_t buf;
	int used_len;
	int total_len;
} compat_android_wifi_priv_cmd;
#endif /* CONFIG_COMPAT */

static int lynx_android_get_link_speed(struct net_device *dev, char *command, int total_len)
{
    struct lynx_vif *vif = netdev_priv(dev);
	struct lynx_sta *sta;
	int link_speed = 0;
	int bytes_written = 0;
	int i;
	/* link speed: Mbps */
	int rates[29] = {72, 65, 57, 43, 28, 21, 14, 7, 54, 48, 36, 24, 18, 12, 9, 6, 11, 5, 2, 1};
	int ht_rates[29] = {150, 135, 120, 90, 60, 45, 30, 15};

	for(i=0; i<LYNX_STA_MAX_NUM; i++)
	{
		if(vif->sta_idx_map & (1 << i))
		{
			sta = &vif->lnx->sta_list[i];
			
			if(sta->flags & LYNX_STA_VALID)
			{
					break;
			}
		}
	}

	if(i < LYNX_STA_MAX_NUM)
	{
		for(i=MCS_7; i>=CCK_1M; i--)
		{
			if(sta->support_rates & R_BIT(i))
			{
				if((sta->bandwidth != BW40MHZ_SCN) && (i > OFDM_54M))
					link_speed = ht_rates[i - MCS_0];
				else
					link_speed = rates[i-1];
				break;
			}
		}

		if(link_speed)
			bytes_written = snprintf(command, total_len, "LinkSpeed %d", link_speed);
	}

	return bytes_written;
}

int lynx_android_getband(struct net_device *dev, char *command, int total_len)
{
	int bytes_written = 0;

	/* only support 2.4G now */
	bytes_written = snprintf(command, total_len, "Band %d", WPA_SETBAND_2G);

	return bytes_written;
}

static int lynx_android_get_rssi(struct net_device *dev, char *command, int total_len)
{
    struct lynx_vif *vif = netdev_priv(dev);
	struct lynx_sta *sta;
	int bytes_written = 0;
	int i;

	for(i=0; i<LYNX_STA_MAX_NUM; i++)
	{
		if(vif->sta_idx_map & (1 << i))
		{
			sta = &vif->lnx->sta_list[i];
			
			if(sta->flags & LYNX_STA_VALID)
			{
					break;
			}
		}
	}

	if(i < LYNX_STA_MAX_NUM)
	{
		memcpy(command, vif->ssid, vif->ssid_len);
		bytes_written = vif->ssid_len;
		bytes_written += snprintf(&command[bytes_written], total_len, " rssi %d", sta->signal_avg);
	}

	return bytes_written;
}

//------------------------------------------------
#ifdef CONFIG_HOST_WPS
#ifdef CONFIG_HOST_P2P
#define ANDROID_CMD_SET_AP_WPS_P2P_IE	"SET_AP_WPS_P2P_IE"
static int lynx_android_set_wps_p2p_ie(struct net_device *dev, char *buf, int len,int type)
{
	struct lynx_vif *vif = netdev_priv(dev);
	struct lynx *lnx = vif->lnx;
	int res = -ENOMEM;

	if((NULL == buf)||(len <=0)){
		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set WPS/P2P ie fail (type %d)\n", __FUNCTION__, __LINE__,type);
		return 0;
	}

/*
	if(vif->wdev.iftype == NL80211_IFTYPE_P2P_GO ){
	lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): VIF is P2P_GO  (type %d)\n", __FUNCTION__, __LINE__,type);
	}else
*/
	if(vif->wdev.iftype == NL80211_IFTYPE_P2P_DEVICE ){
		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): VIF is P2P_DEVICE  (type %d)\n", __FUNCTION__, __LINE__,type);
	}else if(vif->wdev.iftype == NL80211_IFTYPE_STATION ){
		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): VIF is STATION ; Forward to P2P_DEVICE (type %d)\n", __FUNCTION__, __LINE__,type);
		vif = lynx_get_p2p_vif(lnx,0) ;
	}else{
		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Not Support ( IF type %d ,IE type %d)\n", __FUNCTION__, __LINE__,vif->wdev.iftype,type);
		return 0;
	}

	switch (type) {
		case 0x2: //PROBE_RESP ;P2P_DEVICE or P2P_GO
			lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set PROBE_RESP IE (len :%d) by Android priv cmd\n", __FUNCTION__, __LINE__,len);
			res = lynx_cfg80211_set_wps_p2p_ies(vif, (u8)WCI_FRAME_PROBE_RESP,buf,len,1);
			if (res){
				lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set WCI_FRAME_PROBE_RESP fail\n", __FUNCTION__, __LINE__);
			}
			break;
/*
		case 0x1: // BEACON : P2P_GO only
			if(vif->wdev.iftype == NL80211_IFTYPE_P2P_GO ){
				lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set BEACON IE by Android priv cmd\n", __FUNCTION__, __LINE__);
				res = lynx_cfg80211_set_wps_p2p_ies(vif, (u8)WCI_FRAME_BEACON,buf,len,1);
				if (res){
					lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set WCI_FRAME_BEACON fail\n", __FUNCTION__, __LINE__);
				}
			}
			break;

		case 0x4: //ASSOC_RESP: P2P_GO only
			if(vif->wdev.iftype == NL80211_IFTYPE_P2P_GO ){
				lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set ASSOC_RESP IE by Android priv cmd\n", __FUNCTION__, __LINE__);
				res = lynx_cfg80211_set_wps_p2p_ies(vif, (u8)WCI_FRAME_ASSOC_RESP,buf,len,1);
				if (res){
					lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set WCI_FRAME_ASSOC_RESP fail\n", __FUNCTION__, __LINE__);
				}
			}
			break;
*/
		default:
			lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Set Unknow IE (type %d) by Android priv cmd\n", __FUNCTION__, __LINE__,type);
			break;
	}

	return 0;
}
#endif /*CONFIG_HOST_P2P*/
#endif /*CONFIG_HOST_WPS*/
//-----------------------------------------------
int lynx_android_priv_cmd(struct net_device *dev, struct ifreq *ifr, int cmd)
{
#define PRIVATE_COMMAND_MAX_LEN	8192
    struct lynx_vif *vif = netdev_priv(dev);
	int ret = 0;
	char *command = NULL;
	int bytes_written = 0;
	android_wifi_priv_cmd priv_cmd;

	if((!ifr) || (!ifr->ifr_data))
	{
		ret = -EINVAL;
    	lynx_dbg(LYNX_DBG_ANDROID, "%s(): ifr->ifr_data is null\n", __FUNCTION__);
		goto exit;
	}

#ifdef CONFIG_COMPAT
	if(is_compat_task()) 
	{
		compat_android_wifi_priv_cmd compat_priv_cmd;

		if(copy_from_user(&compat_priv_cmd, ifr->ifr_data,
			sizeof(compat_android_wifi_priv_cmd))) 
		{
    		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): can't copy data\n", __FUNCTION__, __LINE__);
			ret = -EFAULT;
			goto exit;

		}
		priv_cmd.buf = compat_ptr(compat_priv_cmd.buf);
		priv_cmd.used_len = compat_priv_cmd.used_len;
		priv_cmd.total_len = compat_priv_cmd.total_len;
	} 
	else
#endif /* CONFIG_COMPAT */
	{
		if(copy_from_user(&priv_cmd, ifr->ifr_data, sizeof(android_wifi_priv_cmd))) 
		{
			ret = -EFAULT;
			lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): can't copy data\n", __FUNCTION__, __LINE__);
			goto exit;
		}
	}

	if((priv_cmd.total_len > PRIVATE_COMMAND_MAX_LEN) || (priv_cmd.total_len < 0)) 
	{
		lynx_dbg(LYNX_DBG_ANDROID, "%s: wrong priavte command len %d\n", __FUNCTION__, priv_cmd.total_len);
		ret = -EINVAL;
		goto exit;
	}

	command = kmalloc((priv_cmd.total_len + 1), GFP_KERNEL);

	if(!command)
	{
		lynx_dbg(LYNX_DBG_ANDROID, "%s: failed to allocate memory\n", __FUNCTION__);
		ret = -ENOMEM;
		goto exit;
	}

	if(copy_from_user(command, priv_cmd.buf, priv_cmd.total_len)) 
	{
		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): can't copy data\n", __FUNCTION__, __LINE__);
		ret = -EFAULT;
		goto exit;
	}

	command[priv_cmd.total_len] = '\0';

	lynx_dbg(LYNX_DBG_ANDROID, "%s: Android private cmd \"%s\" on %s\n", __FUNCTION__, command, ifr->ifr_name);

	if(!test_bit(WLAN_ENABLED, &vif->flags)){
		ret = 0;
		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): vif is not enable\n", __FUNCTION__, __LINE__);
		goto exit;
	}
	else if (strnicmp(command, "RSSI", 4) == 0) {
		bytes_written = lynx_android_get_rssi(dev, command, priv_cmd.total_len);
	}
	else if (strnicmp(command, "LINKSPEED", 9) == 0) {
		bytes_written = lynx_android_get_link_speed(dev, command, priv_cmd.total_len);
	}
	else if (strnicmp(command, "GETBAND", 7) == 0) {
		bytes_written = lynx_android_getband(dev, command, priv_cmd.total_len);
	}
	else if (strnicmp(command, "WLS_BATCHING", 12) == 0) 
	{
		/* FIXME: not support WLS_BATCHING now */
		lynx_dbg(LYNX_DBG_ANDROID, "%s: not support WLS_BATCHING now\n", __FUNCTION__);
	}
#ifdef CONFIG_HOST_WPS
#ifdef CONFIG_HOST_P2P
	else if (strnicmp(command, ANDROID_CMD_SET_AP_WPS_P2P_IE, strlen(ANDROID_CMD_SET_AP_WPS_P2P_IE)) == 0) {
		lynx_dbg(LYNX_DBG_ANDROID, "%s: Set P2P/WPS IE by Android priv cmd\n", __FUNCTION__);

		lynx_dbg(LYNX_DBG_ANDROID, "CMD total leng %d,IE Length %d\n",priv_cmd.total_len ,
			priv_cmd.total_len - (strlen(ANDROID_CMD_SET_AP_WPS_P2P_IE) + 3));


		bytes_written = lynx_android_set_wps_p2p_ie(dev, command + (strlen(ANDROID_CMD_SET_AP_WPS_P2P_IE) + 3), 
			priv_cmd.total_len - (strlen(ANDROID_CMD_SET_AP_WPS_P2P_IE) + 3), 
			*(command + (strlen(ANDROID_CMD_SET_AP_WPS_P2P_IE) + 3) - 2) - '0');
	}
#endif /*CONFIG_HOST_P2P*/
#endif /*CONFIG_HOST_WPS*/
	else{
		lynx_dbg(LYNX_DBG_ANDROID, "%s: not support cmd (%s)\n", __FUNCTION__, command);
	}

	if(bytes_written > 0) {
		if(bytes_written >= priv_cmd.total_len) {
			lynx_dbg(LYNX_DBG_ANDROID, "%s: bytes_written = %d\n", __FUNCTION__, bytes_written);
			bytes_written = priv_cmd.total_len;
		} 
		else {
			bytes_written++;
		}

		priv_cmd.used_len = bytes_written;

		if(copy_to_user(priv_cmd.buf, command, bytes_written)){
			lynx_dbg(LYNX_DBG_ANDROID, "%s: failed to copy data to user buffer\n", __FUNCTION__);
			ret = -EFAULT;
		}
	}
	else {
		lynx_dbg(LYNX_DBG_ANDROID, "%s(%d): Feedback data is empty\n", __FUNCTION__, __LINE__);
		command[0] = '\0';
	}

exit:

	if(command) 
		kfree(command);

	return ret;

}


