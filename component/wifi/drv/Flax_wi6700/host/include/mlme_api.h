
enum {
	SYNC_LINK_STOP,
	SYNC_LINK_START,
};

#define DATAPATH_QUEUE_WAKUP	0x1
#define DATAPATH_CARRIER_ON		0x2

struct lynx_mlme_ops {
	void (*disconnect) (struct lynx_vif *vif, unsigned int sta_idx);
	int (*connect) (struct lynx_vif *vif, int channel, unsigned char *bssid, int assoc_req_ie_len, int assoc_resp_ie_len, unsigned char *ie_data);
	void (*scan_result) (struct lynx_vif *vif, int is_ok);
	int (*schedule_sync_link_st) (struct lynx_vif *vif, unsigned int op, int sec);
	void (*data_path_ctrl) (struct lynx_vif *vif, unsigned int flags);
	void (*mic_err)	(struct lynx_vif *vif, int sta_idx, int key_id, int is_broadcast);
	void (*info_vif_status)	(struct lynx_vif *vif, int is_attached, unsigned char *addr);
	void (*trigger_manager_start) (void);
};

void mlme_disconnect(struct lynx_vif *vif, unsigned int sta_idx);
int mlme_connect(struct lynx_vif *vif, int channel, unsigned char *bssid, int assoc_req_ie_len, int assoc_resp_ie_len, unsigned char *ie_data);
void mlme_scan_result(struct lynx_vif *vif, int is_ok);
int mlme_schedule_sync_link_st(struct lynx_vif *vif, unsigned int op, int sec);
void mlme_data_path_ctrl(struct lynx_vif *vif, unsigned int flags);
void mlme_mic_err(struct lynx_vif *vif, int sta_idx, int key_id, int is_broadcast);
void mlme_info_vif_status(struct lynx_vif *vif, int is_attached, unsigned char *addr);
void mlme_trigger_manager_start(struct lynx *lnx);

