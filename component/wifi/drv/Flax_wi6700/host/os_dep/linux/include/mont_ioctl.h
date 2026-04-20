#ifndef __MONT_IOCTL_H__
#define __MONT_IOCTL_H__
#include <linux/wireless.h>

#define PRIV_BUF_SIZE_MASK		0x07FF
#define PRIV_TYPE_MASK			0xF000
#define PRIV_TYPE_CHAR			0x1000
#define PRIV_TYPE_INT			0x2000

#define DATA_LEN_MAX	1024
#define PRIV_RECV_BUF_SIZE DATA_LEN_MAX


struct mont_ioctl_cmd_data {
	unsigned int		cmd;				/* Number of the ioctl to issue */
	unsigned short		set_args;			/* Type and number of args */
	unsigned short		get_args;			/* Type and number of args */
	char				name[IFNAMSIZ];		/* Name of the extension */
};

struct mont_ioctl_data {
	/* Note: need to take care the 64bits/32bits alignment */
	int cmd_id;
	int used_length;
	int total_length;
	int padding;	/* padding for 64bits/32bits alignment */
	unsigned long data;
} __attribute__ ((packed));



/*cmd id*/
enum mont_ioctl_cmd_ids {
	MONT_IOCTL_GET_CMD_LIST = 0,	/* always be first */
	MONT_IOCTL_HOST_INFO,
	MONT_IOCTL_DBG_LVL,
	MONT_IOCTL_BSS,
	MONT_IOCTL_HT_CAP,
	MONT_IOCTL_TX_PENDING, /* 5 */
	MONT_IOCTL_TX_RATE,
	MONT_IOCTL_RSSI,
	MONT_IOCTL_MIN_RATEIDX,
	MONT_IOCTL_PS,
	MONT_IOCTL_ADDBA,     /* 10 */
	MONT_IOCTL_EN_2040_COEX,
	MONT_IOCTL_WT,
	MONT_IOCTL_MP,
	MONT_IOCTL_MP_START,
	MONT_IOCTL_BB,
	MONT_IOCTL_RF,       /* 15 */
	MONT_IOCTL_WD,
	MONT_IOCTL_OTP,
	MONT_IOCTL_MON_MASK,
	MONT_IOCTL_MON_LEVEL,
	MONT_IOCTL_BW40,
};

/* function name*/
int lynx_ioctl_get_cmd_list(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_dbg_lvl(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_bss(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_ht_cap(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_tx_pending(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_tx_rate(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_rssi(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_min_rateidx(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_ps(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_addba(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_en_2040_coex(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_dynamic_access_device(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_mp_start(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_host_info(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
//int lynx_ioctl_bb(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
//int lynx_ioctl_rf(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_mon_mask(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_monitor_level(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
int lynx_ioctl_bw40(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);
//int lynx_ioctl_fw_debug(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);


typedef int (*mont_ioctl_handler)(struct net_device *dev, struct ifreq *ifr, struct mont_ioctl_data *cmd);


static const mont_ioctl_handler lynx_private_handler[] =
{
	lynx_ioctl_get_cmd_list,	/* MONT_IOCTL_GET_CMD_LIST */
	lynx_ioctl_host_info,
	lynx_ioctl_dbg_lvl,
	lynx_ioctl_bss,
	lynx_ioctl_ht_cap,
	lynx_ioctl_tx_pending, /* 5 */
	lynx_ioctl_tx_rate,
	lynx_ioctl_rssi,
	lynx_ioctl_min_rateidx,
	lynx_ioctl_ps,
	lynx_ioctl_addba,  /* 10 */
	lynx_ioctl_en_2040_coex,
	lynx_ioctl_dynamic_access_device,
	lynx_ioctl_dynamic_access_device,
	lynx_ioctl_mp_start,
	lynx_ioctl_dynamic_access_device, /* 15 */
	lynx_ioctl_dynamic_access_device,
	lynx_ioctl_dynamic_access_device,
	lynx_ioctl_dynamic_access_device,
	lynx_ioctl_mon_mask,
	lynx_ioctl_monitor_level,
	lynx_ioctl_bw40, /* 20 */
 };



int lynx_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

#endif
