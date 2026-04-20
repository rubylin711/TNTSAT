
/* CTRL's command */
enum{
	CTRL_CMD_CONFIG,
	CTRL_CMD_CONNECT,
	CTRL_CMD_DISCONNECT,
	CTRL_CMD_SCAN,
	CTRL_CMD_SCAN_RESULTS,
};

struct scan_bss {
	unsigned char ssid[33];
	unsigned char addr[WLAN_ADDR_LEN];
	unsigned short security;
	unsigned short cipher;
	int channel;
	int rssi;
};

struct wm_ctrl_conf {
	unsigned char mac_addr[6];
	unsigned char ssid[33];
	unsigned int security;
	unsigned int cipher;
	unsigned char key[64];
	int key_len;
	int key_idx;
};

/* ================= Control API Message ================= */
struct sk_buff *wm_ctrl_set_config(int vif_idx, unsigned char *addr, unsigned char *ssid, unsigned int security, unsigned int cipher, unsigned char *key, int key_len);
struct sk_buff *wm_ctrl_connect(int vif_idx);
struct sk_buff *wm_ctrl_disconnect(int vif_idx);
struct sk_buff *wm_ctrl_scan(int vif_idx);
struct neighbor_bss *wm_ctrl_scan_results(void);
int wm_ctrl_scan_result_num(void);

/* ================= WM_MANAGER Handle the CTRL Message ================= */
int wm_setup_security(struct wm_vif *vif, int cipher, int sec_type, char *key, int key_idx, int is_default_key, int clean_old);
void wm_ctrl_handler(struct wm_msg *msg, void *data);
