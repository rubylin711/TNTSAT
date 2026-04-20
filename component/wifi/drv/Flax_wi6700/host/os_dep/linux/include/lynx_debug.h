
#ifndef _LYNX_DBG_H_
#define _LYNX_DBG_H_

#define MAX_ARGV 8
#define LYNX_PROCFS_NAME "lynx"

enum LYNX_DEBUG_MASK {
	LYNX_DBG_CREDIT         = 0x1,
	/* hole */
	LYNX_DBG_WLAN_TX        = 0x4,          /* wlan tx */
	LYNX_DBG_WLAN_RX        = 0x8,          /* wlan rx */

	/* hole */
	/* hole */
	LYNX_DBG_HIF            = 0x40,
	LYNX_DBG_IRQ            = 0x80,         /* interrupt processing */

	/*WCI*/
	LYNX_DBG_WCI            = 0x100,        /* wci cmd to device */
	LYNX_DBG_WCI_EVENT      = 0x200,        /* wci event from device */
	/* hole */
	/* hole */

	LYNX_DBG_SCATTER        = 0x1000,       /* hif scatter tracing */
	LYNX_DBG_WLAN_CFG       = 0x2000,       /* cfg80211 i/f file tracing */
	LYNX_DBG_RAW_BYTES      = 0x4000,       /* dump tx/rx frames */
	LYNX_DBG_AGGR           = 0x8000,       /* aggregation */

	/*SDIO*/
	LYNX_DBG_SDIO           = 0x10000,
	LYNX_DBG_SDIO_DUMP      = 0x20000,
	LYNX_DBG_BOOT           = 0x40000,      /* driver init and fw boot */
	/* hole */

	/*USB*/
	LYNX_DBG_USB_PROBE      = 0x100000,
	LYNX_DBG_USB_RX         = 0x200000,
	LYNX_DBG_USB_TX         = 0x400000,
	LYNX_DBG_RECOVERY       = 0x800000,

	/* generic msg */
	LYNX_DBG_ERR            = 0x1000000,    /* Show err situation */
	LYNX_DBG_WARN           = 0x2000000,    /* Show warn situation */

	/* wlan manager */
	LYNX_DBG_WM				= 0x10000000,	/* wm_manager thread tracing */
	LYNX_DBG_ANDROID		= 0x20000000,	/* Android special function */

	LYNX_DBG_ANY            = 0xffffffff    /* enable all logs */
};

#define LYNX_DEBUG_DEFAULT LYNX_DBG_ANY
#ifdef CONFIG_LYNX_DEBUG

enum LYNX_DBG_INFO_ITEM	{
	LYNX_DBG_NO_WCI_CMD=0,
	LYNX_DBG_WD_INFO,
};

struct lynx_dbg_data {
	struct lynx *lnx;
	struct proc_dir_entry *proc_dir;
	unsigned long dbg_mask;	/* debug print level */
	int show_item;
	int dbg_info_item;	/* show which static info */
};

#ifdef CONFIG_HOST_DEBUG_DEVICE
int lynx_handle_dbg_info(struct lynx_vif *vif, int item_id, int data_len, char *data);
#endif
void lynx_set_dbg_lvl(int lvl);
unsigned int lynx_get_dbg_lvl(void);
int lynx_proc_init_fs(void);
void lynx_proc_exit_fs(void);

void lynx_dbg(enum LYNX_DEBUG_MASK lvl, const char *fmt, ...);
void lynx_dbg_dump(enum LYNX_DEBUG_MASK lvl, const char *msg, const char *prefix,
                   char *buf, size_t len);
#ifdef CONFIG_UNIT_TEST
int unittest_bss_compare(struct lynx_vif *vif, const char *ret_func);
int unittest_bss_backup(struct lynx_vif *vif, const char *ret_func);
#endif	// CONFIG_UNIT_TEST
#else	// CONFIG_LYNX_DEBUG
#define lynx_dbg(fmt, ...)
#define lynx_dbg_dump(lvl, msg, prefix, buf, len)
#endif	// CONFIG_LYNX_DEBUG

#endif	// _LYNX_DBG_H_
