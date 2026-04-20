
#if defined(CONFIG_LYNX_OS_LINUX) && !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#endif	// defined(CONFIG_LYNX_OS_LINUX)

#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "mlme_api.h"
#include "wci.h"

void mlme_disconnect(struct lynx_vif *vif, unsigned int sta_idx)
{
	struct lynx *lnx = vif->lnx;
	
	if(lnx && lnx->mlme_ops && lnx->mlme_ops->disconnect)
		lnx->mlme_ops->disconnect(vif, sta_idx);
}

int mlme_connect(struct lynx_vif *vif, int channel, unsigned char *bssid, int assoc_req_ie_len, int assoc_resp_ie_len, unsigned char *ie_data)
{
	struct lynx *lnx = vif->lnx;
	int ret = -1;

	if(lnx && lnx->mlme_ops && lnx->mlme_ops->connect)
	{
		ret = lnx->mlme_ops->connect(vif, channel, bssid, assoc_req_ie_len, 
									assoc_resp_ie_len, ie_data);
	}

	return ret;
}

void mlme_scan_result(struct lynx_vif *vif, int is_ok)
{
	struct lynx *lnx = vif->lnx;
	
	if(lnx && lnx->mlme_ops && lnx->mlme_ops->scan_result)
		lnx->mlme_ops->scan_result(vif, is_ok);
}

int mlme_schedule_sync_link_st(struct lynx_vif *vif, unsigned int op, int sec)
{
	struct lynx *lnx = vif->lnx;
	int ret = -1;
	
	if(lnx && lnx->mlme_ops && lnx->mlme_ops->schedule_sync_link_st)
		ret = lnx->mlme_ops->schedule_sync_link_st(vif, op, sec);

	return ret;
}

void mlme_data_path_ctrl(struct lynx_vif *vif, unsigned int flags)
{
	struct lynx *lnx = vif->lnx;
	
	if(lnx && lnx->mlme_ops && lnx->mlme_ops->data_path_ctrl)
		lnx->mlme_ops->data_path_ctrl(vif, flags);
}

void mlme_mic_err(struct lynx_vif *vif, int sta_idx, int key_id, int is_broadcast)
{
	struct lynx *lnx = vif->lnx;
	
	if(lnx && lnx->mlme_ops && lnx->mlme_ops->mic_err)
		lnx->mlme_ops->mic_err(vif, sta_idx, key_id, is_broadcast);
}

void mlme_info_vif_status(struct lynx_vif *vif, int is_attached, unsigned char *addr)
{
	struct lynx *lnx = vif->lnx;

	if(lnx && lnx->mlme_ops && lnx->mlme_ops->info_vif_status)
		lnx->mlme_ops->info_vif_status(vif, is_attached, addr);
}

void mlme_trigger_manager_start(struct lynx *lnx)
{
	if(lnx && lnx->mlme_ops && lnx->mlme_ops->trigger_manager_start)
		lnx->mlme_ops->trigger_manager_start();
}
