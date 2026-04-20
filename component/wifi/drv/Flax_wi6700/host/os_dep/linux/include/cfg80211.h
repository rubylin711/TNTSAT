
#define CFG80211_GFP GFP_ATOMIC

int lynx_cfg80211_init(struct lynx *lnx);
void lynx_cfg80211_stop(struct lynx_vif *vif);
struct lynx *lynx_cfg80211_create(void);
void lynx_cfg80211_cleanup(struct lynx *lnx);
void lynx_cfg80211_destroy(struct lynx *lnx);
void lynx_cfg80211_vif_stop(struct lynx_vif *vif);
void lynx_cfg80211_vif_cleanup(struct lynx_vif *vif);
int lynx_cfg80211_connect_event(struct lynx_vif *vif, u8 channel, u8 *bssid, u8 assoc_req_len, u8 assoc_resp_len, u8 *assoc_info);
void lynx_cfg80211_scan_complete(struct lynx_vif *vif, u8 isok);
int lynx_cfg80211_mic_err_event(struct lynx_vif *vif, u8 *addr, u32 key_id, u32 key_type);
void lynx_cfg80211_scan_complete_event(struct lynx_vif *vif, bool aborted);
#ifdef CONFIG_HOST_WPS
int  lynx_cfg80211_set_wps_p2p_ies(struct lynx_vif *vif,u8 mgmt_type,
                                        const u8 *ies, size_t ies_len,bool p2p_exist);
#endif /*CONFIG_HOST_WPS*/

