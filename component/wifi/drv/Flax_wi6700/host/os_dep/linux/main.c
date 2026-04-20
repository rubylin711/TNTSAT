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

#include "init.h"
#include "wlan_def.h"
#include "core.h"
#include "wci.h"
#include "txrx.h"
#include "lynx_debug.h"
#include "hif.h"
#include "cfg80211.h"
#include "sdio.h"
#include "lynx_rev.h"
#include "mont_ioctl.h"
#include "mlme_api.h"

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
static int lynx_open(struct net_device *dev);
static int lynx_close(struct net_device *dev);
static struct net_device_stats *lynx_get_stats(struct net_device *dev);
static int lynx_set_features(struct net_device *dev, netdev_features_t features);
static void lynx_set_multicast_list(struct net_device *ndev);
static int lynx_change_mac(struct net_device *dev, void *addr);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

struct lynx *lynx_priv(struct net_device *dev)
{
    return ((struct lynx_vif *) netdev_priv(dev))->lnx;
}

static const struct net_device_ops lynx_netdev_ops = {
	.ndo_open				= lynx_open,
	.ndo_stop				= lynx_close,
	.ndo_do_ioctl			= lynx_ioctl,
	.ndo_start_xmit			= lynx_data_tx,
	.ndo_get_stats			= lynx_get_stats,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39))
	.ndo_set_features		= lynx_set_features,
#endif
	.ndo_set_rx_mode		= lynx_set_multicast_list,
	.ndo_set_mac_address	= lynx_change_mac,
};

#ifdef CONFIG_SUPPORT_MONITOR_MODE
int wlan_init_monitor(struct lynx_vif *vif)
{
	lynx_dbg(LYNX_DBG_ERR, "Init monitor mode.\n");
	vif->lnx->mon_mask = 0;
	lynx_wci_get_monitor_cmd(vif); /*get FW mask and set enable to FW when ifconfig up*/
	return 0;
}

int wlan_stop_monitor(struct lynx_vif *vif)
{
	/* Wait get original settings from FW*/
	if (vif->lnx->mon_mask == 0) {
		lynx_dbg(LYNX_DBG_ERR, "Wait to get original monitor mask from FW.\n");
		return -EAGAIN;
	}
	
	lynx_wci_set_monitor_cmd(vif,(~RX_MON_MODE)&(vif->lnx->mon_mask));
	vif->lnx->fw_monitor_en = 0;
	lynx_dbg(LYNX_DBG_ERR, "Close monitor mode.\n");
	return 0;
}

int wlan_start_monitor(struct lynx_vif *vif,u32 mon_mask)
{
	lynx_wci_set_monitor_cmd(vif,mon_mask); /* =>FW :wmac_set_monitor(lapp->wrx_mon_bmap)*/
	vif->lnx->fw_monitor_en = 1;
	printk("Enable monitor mode OK.\n");
	return 0;
}

int lynx_monitor_enable(struct lynx_vif *vif)
{
	u32 mon_mask = 0;
	/* Wait get original settings from FW*/
	if (vif->lnx->mon_mask == 0) {
		printk("Wait to get original monitor mask from FW \n");
		return -EAGAIN;
	}

	switch (vif->lnx->monitor_level) {
		case 1:
			printk("Monitor Level: BEACON/PROBE_REQ ONLY\n");
			mon_mask = ( RXF_BEACON_ALL | RXF_PROBE_REQ_ALL );
			break;
		case 2:
			printk("Monitor Level: MGMT ONLY\n");
			mon_mask = ( RX_MON_MGMT |
							RXF_GC_MGT_ALL | RXF_UC_MGT_ALL |
							RXF_BEACON_ALL | RXF_PROBE_REQ_ALL);
			break;
		case 3:
			printk("Monitor Level: ALL\n");
			mon_mask = ( RX_MON_MODE |
							RXF_GC_MGT_ALL | RXF_UC_MGT_ALL |
							RXF_GC_DAT_ALL | RXF_UC_DAT_ALL |
							RXF_BEACON_ALL | RXF_PROBE_REQ_ALL |
							vif->lnx->mon_mask);
			break;
		case 4:
			printk("Monitor Level: OMNICFG mode\n");
			// Monitor broadcast packet for ominiconfig SISO v2 and control packet for MIMO.
			mon_mask =( RX_MON_MODE |RX_MON_DATA_GROUPCAST|RXF_GC_DAT_ALL|RXF_UC_DAT_ALL|
						RX_MON_MGMT_PROBE_REQ|RXF_PROBE_REQ_ALL|
						RX_MON_CTRL|RX_MON_MIMO|RX_MON_DATA_UNICAST|RXF_UC_DAT_ALL);
			break;
		case 5:
			printk("Monitor Level: OMNICFG mode (include BEACON)\n");
			mon_mask =( RX_MON_MODE |RX_MON_DATA_GROUPCAST|RXF_GC_DAT_ALL|RXF_UC_DAT_ALL|
						RX_MON_MGMT_PROBE_REQ|RXF_PROBE_REQ_ALL|
						RX_MON_CTRL|RX_MON_MIMO|RX_MON_DATA_UNICAST|RXF_UC_DAT_ALL|
						RX_MON_MGMT_BEACON|RXF_BEACON_ALL);
			break;
		case 0:
		default:
			printk("Monitor Level: DATA ONLY\n");
			mon_mask = ( RX_MON_DATA |
							RXF_GC_DAT_ALL | RXF_UC_DAT_ALL);
			break;
	}
	return wlan_start_monitor(vif,mon_mask);
}

/*call from /proc/lynx  or mont_ctl API*/
int lynx_monitor_switch(struct lynx_vif *vif,u32 new_monitor_level)
{
	if(!test_bit(WLAN_ENABLED, &vif->flags)) {
		printk("Please ifconfig up the monitor interface first.\n");
		return -EIO;
	}

	if(vif->wdev.iftype != NL80211_IFTYPE_MONITOR){
		printk("Only support monitor mode network intrface to switch monitor level.\n");
		return -EOPNOTSUPP;
	}

	if (vif->lnx->mon_mask == 0) {
		printk("Wait to get original monitor mask from FW \n");
		printk("Please add the monitor mode interface first\n");
		return -EAGAIN;
	}

	if ((new_monitor_level < 0) || (new_monitor_level > 5)){
		printk("Switch monitor level failure.\n");
		printk("Switch monitor level 0-5 : \n" \
				"                (0: Monitor data frame ; 1: Monitor beacon/probe_req frame ;\n" \
				"                (2: Monitor mgmt frame  ; 3: Monitor all ; 4 : OMNICFG ; 5 : OMNICFG (with beacon)\n");
		return -EINVAL;
	}

	/*for speedup omnicfg ,not restart for "OMNICFG mode" to "OMNICFG mode (include BEACON)"*/
	if ((vif->lnx->monitor_level == 4) && (new_monitor_level == 5)){
		printk("Add beacon rx filter for OMNICFG mode.\n");
	}else{
		wlan_stop_monitor(vif);
	}

	vif->lnx->monitor_level = new_monitor_level;
	return lynx_monitor_enable(vif);
}

#endif /*CONFIG_SUPPORT_MONITOR_MODE*/

static int lynx_open(struct net_device *dev)
{
	struct lynx_vif *vif = netdev_priv(dev);
	lynx_dbg(LYNX_DBG_WARN, "%s(): vif->fw_vif_idx=%d\n", __FUNCTION__, vif->fw_vif_idx);

	set_bit(WLAN_ENABLED, &vif->flags);
	
#ifdef CONFIG_SUPPORT_MONITOR_MODE
	if(vif->wdev.iftype == NL80211_IFTYPE_MONITOR)
		return lynx_monitor_enable(vif);
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#if defined(CONFIG_LYNX_OS_LINUX)
    /*FIXME: must check the connect evt form device will set CONNECTED bit */
    if(test_bit(CONNECTED, &vif->flags)) 
	{
        netif_carrier_on(dev);
        netif_wake_queue(dev);
    } 
	else
	{
        netif_carrier_off(dev);
	}
#endif

#ifdef CONFIG_TIMER_HANDLER
	schedule_delayed_work(&vif->timer_work, (5 * HZ));
#endif // CONFIG_TIMER_HANDLER
    
    return 0;
}

static int lynx_close(struct net_device *dev)
{
	struct lynx_vif *vif = netdev_priv(dev);

	lynx_dbg(LYNX_DBG_WARN, "%s(): vif->fw_vif_idx=%d\n", __FUNCTION__, vif->fw_vif_idx);

	/* Stop netdev queues, needed during recovery */
	os_api_dsr_lock(&vif->if_lock);
	netif_stop_queue(vif->ndev);
	netif_carrier_off(vif->ndev);
	os_api_dsr_unlock(&vif->if_lock);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
	if(vif->wdev.iftype == NL80211_IFTYPE_MONITOR)
		wlan_stop_monitor(vif);
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

#if !defined(CONFIG_LYNX_WM_MANAGER)
    lynx_cfg80211_stop(vif);
#endif

	clear_bit(WLAN_ENABLED, &vif->flags);

#ifdef CONFIG_TIMER_HANDLER
	cancel_delayed_work(&vif->timer_work);
#endif

    return 0;
}

int lynx_data_tx(struct sk_buff *skb, struct net_device *dev)
{
    struct lynx *lnx        = lynx_priv(dev);
    struct lynx_vif *vif    = netdev_priv(dev);
    int ret;
	u16 ethertype;
#ifdef CONFIG_LYNX_STATICS
	int sta_idx=0;
	unsigned long *cb_ptr=NULL;
	struct lynx_sta *sta=NULL;
#endif	// CONFIG_LYNX_STATICS

    lynx_dbg(LYNX_DBG_WLAN_TX, "%s: skb=0x%p, data=0x%p, len=0x%x, vif_idx=%d\n", __func__,
            skb, skb->data, skb->len, vif->fw_vif_idx);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
    if(vif->wdev.iftype == NL80211_IFTYPE_MONITOR)
    {
		lynx_dbg(LYNX_DBG_ERR, "%s(): Monitor mode not support TX  \n",__FUNCTION__);
		goto tx_fail;
    }
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

    /* driver is in the detach state */
    if(test_bit(DESTROY_IN_PROGRESS, &lnx->flag))
        goto tx_fail;

    /* If target is not associated */
    if(!test_bit(CONNECTED, &vif->flags))
	{
		lynx_dbg(LYNX_DBG_WLAN_TX, "%s():  drop for the non-connected vif\n", __FUNCTION__);
        goto tx_fail;
	}

	/* only allow EAPoL frame */
    if(!test_bit(DATAPATH_EN, &vif->flags))
	{
		ethertype = (skb->data[12] << 8) | skb->data[13];
		if(ethertype != ETHERTYPE_EAPOL)
		{
			lynx_dbg(LYNX_DBG_WLAN_TX, "%s(): type 0x%x is not allowed when datapath is not authozied\n", __FUNCTION__, ethertype);
			goto tx_fail;
		}
	}

	lynx_dbg_dump(LYNX_DBG_WLAN_TX, "the data buf: ", "", skb->data, skb->len);

	if(skb_headroom(skb) < dev->needed_headroom) 
	{
		struct sk_buff *tmp_skb = skb;

		skb = skb_realloc_headroom(skb, dev->needed_headroom);
		/* skb_realloc_headroom seems not free the original skb */
		kfree_skb(tmp_skb);
		if(skb == NULL) 
		{
			lynx_dbg(LYNX_DBG_WLAN_TX, "%s(): drop for that the skb_realloc_headroom failed\n", __FUNCTION__);
			vif->net_stats.tx_dropped++;
			return 0;
        }
    }

#ifdef CONFIG_LYNX_STATICS
	/* prepare the static info */
	if((vif->nw_type == VIF_STA_MODE) || (vif->nw_type == VIF_P2P_CLIENT_MODE))
	{
		for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
		{
			if((1 << sta_idx) & vif->sta_idx_map)
				break;
		}
	}
	else
	{
		for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
		{
			if((1 << sta_idx) & vif->sta_idx_map)
			{
				sta = &lnx->sta_list[sta_idx];
				if((sta->flags & LYNX_STA_VALID) && (memcmp(sta->addr, skb->data, 6) == 0))
					break;
			}
		}
	}
#endif	// CONFIG_LYNX_STATICS

    /* Prepare to encapsulate wci */
	ret = lynx_data_wci_hdr_add(skb, dev, (WCI_H2D_PACKET | vif->fw_vif_idx));

	if(ret)
	{
		lynx_dbg(LYNX_DBG_WLAN_TX, "failed to add wci header\n");
		goto tx_fail;
    }

#ifdef CONFIG_LYNX_STATICS
	cb_ptr = (unsigned long *)&skb->cb[0];
	if(sta_idx < LYNX_STA_MAX_NUM)
	{
		cb_ptr[0] = (unsigned long)vif;
		cb_ptr[1] = (unsigned long)sta_idx;
	}
	else
	{
		cb_ptr[0] = 0;
		cb_ptr[1] = 0;
	}
#endif	// CONFIG_LYNX_STATICS

	if(lynx_data_queue_is_empty(lnx) != 0)
	{
		ret = lynx_tx_send(lnx, skb);
		if(ret == 0)
		{
			return 0;
		}
		else if(ret != -ENOMEM)
		{
        	lynx_dbg(LYNX_DBG_WLAN_TX, "%s(): direct tx fail\n", __FUNCTION__);
			goto tx_fail;
		}
	}

	/* enqueue when tx fail */
	if(lynx_data_enqueue(skb, vif, lynx_data_urgentq_check(skb)) == 0)
		return 0;
	else
		lynx_dbg(LYNX_DBG_WLAN_TX, "%s(): enqueue fail\n", __FUNCTION__);
		
tx_fail:
	kfree_skb(skb);

	vif->net_stats.tx_dropped++;
	vif->net_stats.tx_aborted_errors++;

	return 0;
}

static struct net_device_stats *lynx_get_stats(struct net_device *dev)
{
    struct lynx_vif *vif = netdev_priv(dev);

    return &vif->net_stats;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39))
static int lynx_set_features(struct net_device *dev, netdev_features_t features)
{
    lynx_dbg(LYNX_DBG_WARN, "Not Implement %s\n",__func__);

    return 0;
}
#endif

static void lynx_set_multicast_list(struct net_device *ndev)
{
    lynx_dbg(LYNX_DBG_WARN, "Not Implement %s\n",__func__);
}

static int lynx_change_mac(struct net_device *dev, void *addr)
{
	struct lynx_vif *vif = netdev_priv(dev);
	struct sockaddr *sa = addr;
	struct wm_bss *bss = &vif->bss;
	int ret = -EINVAL;

	if(is_valid_ether_addr(sa->sa_data))
	{
		ret = eth_mac_addr(dev, sa);

		if(ret == 0)
		{
#ifdef CONFIG_UNIT_TEST
			unittest_bss_compare(vif, __FUNCTION__);
#endif

			memcpy(vif->vif_addr, sa->sa_data, ETH_ALEN);
			memcpy(bss->myaddr, sa->sa_data, ETH_ALEN);
			lynx_wci_set_mac_cmd(vif, vif->vif_addr);

#ifdef CONFIG_UNIT_TEST
			unittest_bss_backup(vif, __FUNCTION__);
#endif
		}
	}

	return ret;
}

static void lynx_ethtool_info(struct net_device *dev, struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, KBUILD_MODNAME, sizeof(info->driver));
	strcpy(info->version, LYNX_DRIVER_REV);
	//strlcpy(info->bus_info, dev_name(dev), sizeof(info->bus_info));

	return;
}

static struct ethtool_ops lynx_ethtool_ops ={
	.get_drvinfo = lynx_ethtool_info,
};

void init_netdev(struct net_device *dev)
{
#if defined(CONFIG_LYNX_OS_LINUX)
	dev->netdev_ops = &lynx_netdev_ops;
	dev->priv_destructor = free_netdev;
	dev->ethtool_ops = &lynx_ethtool_ops;
#endif

#if (defined(CONFIG_LYNX_W2SDIO) || defined(CPTCFG_LYNX_W2SDIO))
    /* we try not to affect usb by sdio data alignment */
    dev->needed_headroom = SKB_DATA_OFFSET + SDIO_HDR_SIZE + DMA_ADDRESS_ALIGNMENT;
#else
    dev->needed_headroom = SKB_DATA_OFFSET;
#endif

    /*offload setup*/
#if 0   
    if(!test_bit())
        dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_RXCSUM;
#endif      
}

static void lynx_vif_sync_link_status_trigger(struct work_struct *work)
{
	struct lynx_vif *vif = container_of(work, struct lynx_vif, sync_work.work);
	int timer=1;

    if(!test_bit(WLAN_ENABLED, &vif->flags))
		return;

	lynx_wci_get_vif_link_st_cmd(vif);

	if(vif->sta_idx_map == 0)
		timer = 10;		// only check status every 10 sec, if no sta is connected

	schedule_delayed_work(&vif->sync_work, (timer * HZ));
}

#ifdef CONFIG_TIMER_HANDLER
static void lynx_vif_timer_work(struct work_struct *work)
{
	struct lynx_vif *vif = container_of(work, struct lynx_vif, timer_work.work);
	struct lynx *lnx = vif->lnx;

	if(!test_bit(WLAN_ENABLED, &vif->flags))
		return;

	/* item 1 : check the scan progress */
	if((vif->scan_req) && ((jiffies - vif->scan_start_time) >= (3*HZ)))
	{
		/* patch the case : driver doesn't reveive the scan done wci event */
		mlme_scan_result(vif, 0);
		lnx->scan_not_done_count++;
	}

	schedule_delayed_work(&vif->timer_work, (1 * HZ));
}
#endif // CONFIG_TIMER_HANDLER

static int lynx_vif_init(struct lynx_vif *vif)
{
#if 0
    setup_timer(&vif->sched_scan_timer, lynx_wci_sscan_timer, (unsigned long) vif);
#endif

	INIT_DELAYED_WORK(&vif->sync_work, lynx_vif_sync_link_status_trigger);
#ifdef CONFIG_TIMER_HANDLER
	INIT_DELAYED_WORK(&vif->timer_work, lynx_vif_timer_work);
#endif

    /* FIXME: does ENABLED bit needed in the vif */

    os_api_lock_init(&vif->if_lock);

    return 0;
}

void *lynx_interface_add(struct lynx *lnx, const char *name, unsigned char name_type,
                    enum nl80211_iftype type, u8 fw_vif_idx, u8 nw_type)
{
	struct net_device   *ndev;
	struct lynx_vif     *vif;
	char macaddr[6] = {0x00, 0x00, 0x00, 0x33, 0x44, 0x55};
	struct wm_bss *bss;
	struct device_configs *fw = &lnx->fw_config;
	int tsf_idx=0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	if((ndev = alloc_netdev(sizeof(struct lynx_vif), name, name_type, ether_setup)) == NULL)
#else
	if((ndev = alloc_netdev(sizeof(struct lynx_vif), name, ether_setup)) == NULL)
#endif
	{
		lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(%d): ndev can't alloc\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	vif = netdev_priv(ndev);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
	if( type == NL80211_IFTYPE_MONITOR )
	{
		ndev->type = ARPHRD_IEEE80211_RADIOTAP;
	}
#endif /*CONFIG_SUPPORT_MONITOR_MODE */

	lynx_dbg(LYNX_DBG_WLAN_CFG, "%s(): ndev:%p, type=%d, nw_type=%d\n", __FUNCTION__, ndev, type, nw_type);

    vif->lnx = lnx;
    vif->ndev = ndev;

#if !defined(CONFIG_LYNX_WM_MANAGER)
	ndev->ieee80211_ptr = &vif->wdev;
	vif->wdev.wiphy = lnx->wiphy;

    SET_NETDEV_DEV(ndev, wiphy_dev(vif->wdev.wiphy));

    vif->wdev.netdev = NULL;
#ifdef CONFIG_HOST_P2P
	if(type != NL80211_IFTYPE_P2P_DEVICE)
#endif /*CONFIG_HOST_P2P */
	{
		vif->wdev.netdev = ndev;
	}

    vif->wdev.iftype = type;
#endif	// CONFIG_LYNX_WM_MANAGER
    vif->fw_vif_idx = fw_vif_idx;
    vif->nw_type = nw_type;

    if (is_valid_ether_addr(lnx->mac_addr)) {
        memcpy(vif->vif_addr, lnx->mac_addr, ETH_ALEN);
    } else {
        memcpy(vif->vif_addr, macaddr, ETH_ALEN);
    }

	vif->vif_addr[5] += fw_vif_idx;
	if(ndev->dev_addr)
		memcpy(ndev->dev_addr, vif->vif_addr, ETH_ALEN);

#ifdef CONFIG_HOST_P2P
	if( type == NL80211_IFTYPE_P2P_DEVICE ){
		memcpy(vif->wdev.address, vif->vif_addr, ETH_ALEN);
		vif->action_cookie = 0;
	}
#endif /*CONFIG_HOST_P2P */

#ifdef CONFIG_HOST_P2P
	if( type != NL80211_IFTYPE_P2P_DEVICE )
#endif /*CONFIG_HOST_P2P */
	{
		init_netdev(ndev);
	}

    if(lynx_vif_init(vif))
        goto err;

#if defined(CONFIG_LYNX_OS_LINUX)
#if defined(CONFIG_LYNX_WM_MANAGER)
	rtnl_lock();
#endif	// CONFIG_LYNX_WM_MANAGER

#ifdef CONFIG_HOST_P2P
	if( type != NL80211_IFTYPE_P2P_DEVICE )
#endif /*CONFIG_HOST_P2P */
	{
		if(register_netdevice(ndev))
			goto err;
	}
#if defined(CONFIG_LYNX_WM_MANAGER)
    rtnl_unlock();
#endif	// CONFIG_LYNX_WM_MANAGER
#endif	// CONFIG_LYNX_OS_LINUX

    lnx->avail_idx_map &= ~BIT(fw_vif_idx);

    set_bit(DEV_ATTACHED, &vif->flags);
    clear_bit(WLAN_ENABLED, &vif->flags);
    clear_bit(CONNECTED, &vif->flags);
    clear_bit(CONNECT_PEND, &vif->flags);

    if (type == NL80211_IFTYPE_ADHOC)
        lnx->ibss_existing = 1;

    os_api_dsr_lock(&lnx->list_lock);
    list_add_tail(&vif->list, &lnx->vif_list);
    lnx->num_vif++;
    os_api_dsr_unlock(&lnx->list_lock);

	if(nw_type == VIF_STA_MODE)
		tsf_idx = 1;	/* All the case use tsf_idx = 1, except VIF_STA_MODE */
    
	/* fill the bss */   
	bss = &vif->bss;
    bss->tsf_idx = tsf_idx;
    bss->bss_desc = vif->fw_vif_idx;
	memcpy(bss->myaddr, vif->vif_addr, ETH_ALEN);
	if((vif->nw_type == VIF_AP_MODE) || (vif->nw_type == VIF_WDS_MODE))
		memcpy(bss->bssid, vif->vif_addr, ETH_ALEN);
    bss->flag |= (BSS_FLG_ENABLE | BSS_FLG_WMM);	/* FIXME: check all the flag */

#ifdef CONFIG_HOST_ROAMING
	if( type == NL80211_IFTYPE_STATION )
		bss->flag |=BSS_FLG_BEACON_REPORT;
#endif /*CONFIG_HOST_ROAMING*/
 
#ifdef CONFIG_HOST_P2P
	if( type == NL80211_IFTYPE_P2P_DEVICE )
		bss->flag |=BSS_FLG_P2P_DEVICE_MODE;
	else if( type == NL80211_IFTYPE_P2P_CLIENT )
		bss->flag |=BSS_FLG_P2P_CLIENT_MODE;
	else if( type == NL80211_IFTYPE_P2P_GO )
		bss->flag |=BSS_FLG_P2P_GO_MODE;
#endif /*CONFIG_HOST_P2P*/

    cpu_to_be32s(&(bss->flag));
	bss->phy_cap = fw->phy_cap;

	bss->role = vif->nw_type;
    cpu_to_be16s(&(bss->role));
	/* b mode */
	bss->tx_rate_code = 0x3b00;
    cpu_to_be16s(&(bss->tx_rate_code));
    bss->ibss_state = 0;
    bss->dtim_period = 1;

	bss->auth_capability |= AUTH_CAP_OPEN;
#ifdef CONFIG_HOST_WPS
	bss->auth_capability |= AUTH_CAP_WPS;
#endif /*CONFIG_HOST_WPS*/

    cpu_to_be32s(&(bss->auth_capability));

	bss->beacon_interval = 100; /* FIXME: connect action should set it */
    cpu_to_be16s(&(bss->beacon_interval));
	if(nw_type != VIF_IBSS_MODE)
		bss->dtim_period = 3; /* FIXME: connect action should set it */
	else
		bss->dtim_period = 1;

	if(vif->nw_type == VIF_IBSS_MODE)
		bss->slottime = SLOTTIME_20US;
	else
		bss->slottime = SLOTTIME_9US;
    cpu_to_be16s(&(bss->slottime));

	/* FIXME: check the ht_capability setting */
	if(bss->phy_cap & AP_CAP_11N)
	{
		bss->ht_capability = IEEE80211_HT_CAP_SGI_20;

		/* Let cfg80211 module's cfg80211_disable_40mhz_24ghz parm */
		/* to deside if fw enable 40MHz support in the 2.4GHz band or not.*/
		if((fw->bandwidth != BW40MHZ_SCN)
#if !defined(CONFIG_LYNX_WM_MANAGER)
			&& (lnx->wiphy->bands[NL80211_BAND_2GHZ]->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40)
#endif
			)
		{
			bss->ht_capability |= (IEEE80211_HT_CAP_DSSSCCK40|IEEE80211_HT_CAP_SUP_WIDTH_20_40|IEEE80211_HT_CAP_GRN_FLD|IEEE80211_HT_CAP_SGI_40);
		}

		cpu_to_be16s(&(bss->ht_capability));
	}

#if !defined(CONFIG_LYNX_WM_MANAGER)
	if (!(lnx->wiphy->bands[NL80211_BAND_2GHZ]->ht_cap.cap &  IEEE80211_HT_CAP_SUP_WIDTH_20_40))
		lynx_dbg(LYNX_DBG_WLAN_CFG, "Based cfg80211_disable_40mhz_24ghz to disable 40 MHZ\n");
#endif

	bss->ampdu_params = fw->ampdu_params;

#if 0
    unsigned int wme_acm;
    /* HT Capability */
    u8  support_mcs_set[16];    /* FIXME: No Need ? */
    u8  ssid[33];
    u8  ssid_len;
	u16 atim_window;
#endif

#ifdef CONFIG_HOST_P2P
	if( type == NL80211_IFTYPE_P2P_DEVICE )
	{
		memcpy(bss->ssid, P2P_WILDCARD_SSID, P2P_WILDCARD_SSID_LEN);
		bss->ssid_len = P2P_WILDCARD_SSID_LEN;
	}
	/* P2P doesn't support b rate */
	if
	( ( type == NL80211_IFTYPE_P2P_DEVICE )
	//||( type == NL80211_IFTYPE_P2P_CLIENT )
	//||( type == NL80211_IFTYPE_P2P_GO )
	)
	{
		bss->phy_cap &= ~AP_CAP_11B;
		bss->phy_cap |= AP_CAP_11G;

		if(bss->phy_cap & AP_CAP_11G)
			bss->tx_rate_code |= OFDM_RATES;
		
		if(bss->phy_cap & AP_CAP_11N)
			bss->tx_rate_code |= MCS_RATES;
		
		bss->tx_rate_code &= ~B_RATES;
		cpu_to_be16s(&(bss->tx_rate_code));
	}
#endif /*CONFIG_HOST_P2P*/

    lynx_wci_attach_cmd(vif);

#ifdef CONFIG_SUPPORT_MONITOR_MODE
	if( type == NL80211_IFTYPE_MONITOR )
		wlan_init_monitor(vif);
#endif /*CONFIG_SUPPORT_MONITOR_MODE */


#if !defined(CONFIG_LYNX_WM_MANAGER) && defined(CONFIG_LYNX_OS_LINUX)
    return &vif->wdev;
#else
    return vif;
#endif

err:
    free_netdev(ndev);
    return NULL;
}

//int lynx_interface_del(struct lynx *lnx, struct wireless_dev *wdev)
int lynx_interface_del(struct lynx *lnx, struct lynx_vif *vif)
{

#if !defined(CONFIG_LYNX_WM_MANAGER)
#if defined(CONFIG_LYNX_OS_LINUX)
	lynx_cfg80211_vif_cleanup(vif);
#endif
#else	// CONFIG_LYNX_WM_MANAGER
	/* TODO: del the vif in the wm_manager */
	clear_bit(DEV_ATTACHED, &vif->flags);
	clear_bit(CONNECT_PEND, &vif->flags);
	clear_bit(CONNECTED, &vif->flags);
	
#if defined(CONFIG_LYNX_OS_LINUX)
	netif_stop_queue(vif->ndev);
	netif_carrier_off(vif->ndev);
	unregister_netdevice(vif->ndev);
#endif	// CONFIG_LYNX_OS_LINUX

#endif	// CONFIG_LYNX_WM_MANAGER


	return 0;
}

