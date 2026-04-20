/*=============================================================================+
|                                                                              |
| Copyright 2013                                                               |
| Montage Inc. All right reserved.                                             |
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
#include <linux/version.h>
#include <linux/proc_fs.h>
#if !defined(CONFIG_LYNX_WM_MANAGER)
#include <net/mac80211.h>
#else	// CONFIG_LYNX_WM_MANAGER
#include "ctrl_intf.h"
#endif
#include "wlan_def.h"
#include "init.h"
#include "core.h"
#include "wci.h"
#include "mac_ctrl.h"
#include "hif.h"
#include "lynx_debug.h"

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define LYNX_DBG_SHOW_BSS			0x1
#define LYNX_DBG_SHOW_DBG_LV		0x2
#define LYNX_DBG_SHOW_CMD			0x3
#define LYNX_DBG_SHOW_HT_CAP		0x4
#define LYNX_DBG_SHOW_TX_PENDING	0x5
#define LYNX_DBG_SHOW_TX_RATE		0x6
#define LYNX_DBG_SHOW_RSSI			0x7
#define LYNX_DBG_SHOW_DBG_INFO		0x8
#define LYNX_DBG_SHOW_RSSI_AND_TX_RATE 0x9
#define LYNX_DBG_SHOW_LINK_ST 0xa
#define LYNX_DBG_INFO_SHOW			0x100
#define LYNX_DBG_INFO_SET			0x101
#define LYNX_DBG_INFO_ADDBA			0x102
#define LYNX_DBG_INFO_2040_COEX		0x103
#define LYNX_DBG_WM_SCAN_RESULTS	0x200

#define SEQ_FILE_BUFSIZE    PAGE_SIZE
#define MAX_CMD_STRING_LENGTH   256
#define LYNX_PROCFS_NAME "lynx"


/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#if defined(CONFIG_LYNX_DEBUG) && defined(CONFIG_LYNX_OS_LINUX)
/* proc file system only for linux */
struct lynx_dbg_data dbg_data;
#endif

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

#ifdef CONFIG_LYNX_DEBUG

static void dump_buf(char *buf, int len)
{
	int j;
	int dlen;

	if(len)
		dlen = len;
	else 
		dlen = 64;

	for(j=0; j<dlen; j++)
	{
		if(0 == (j & 0x1f))
			printk("\n%03x:", j);
		printk(" %02x",buf[j]&0xff);
	}
	printk("\n");
}

void lynx_dbg(enum LYNX_DEBUG_MASK lvl, const char *fmt, ...)
{
    va_list args;
    unsigned char line[160];

    if (!(dbg_data.dbg_mask & lvl))
        return;
    va_start(args, fmt);
    vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);
    printk("lynx: %s", line);
}

void lynx_dbg_dump(enum LYNX_DEBUG_MASK lvl, const char *msg, const char *prefix,
                    char *buf, size_t len)
{
    if (!(dbg_data.dbg_mask & lvl))
        return;
    if (msg)
        lynx_dbg(lvl, "%s", msg);
    dump_buf(buf, len);
}

#ifdef CONFIG_HOST_DEBUG_DEVICE
int lynx_handle_dbg_info(struct lynx_vif *vif, int item_id, int data_len, char *data)
{
	unsigned int *ptr = (unsigned int *)data;
	
	if(item_id == LYNX_DBG_WD_INFO)
	{
		printk("======== WD INFO ========\n");
		printk("[hacq]\topen=0x%04x\n", be32_to_cpu(ptr[0]));
		printk("[dump]\twrx=%04d wtx=%04d\n", be32_to_cpu(ptr[1]), be32_to_cpu(ptr[2]));
		printk("[block]\twrx=%04d wtx=%04d\n", be32_to_cpu(ptr[3]), be32_to_cpu(ptr[4]));
		printk("[table]\tsta=%04d ds=%04d\n", be32_to_cpu(ptr[5]), be32_to_cpu(ptr[6]));
		printk("[ps]\tmodem_ps=%04d modem_ps_policy=%04d\n", 
				be32_to_cpu(ptr[7]), be32_to_cpu(ptr[8]));
		printk("[bhdr]\twrx=%04d wtx=%04d ext=%04d\n", 
				be32_to_cpu(ptr[9]), be32_to_cpu(ptr[10]), be32_to_cpu(ptr[11]));
		printk("      \tfree_head=%04d free_tail=%04d free_count=%04d\n", 
				be32_to_cpu(ptr[12]), be32_to_cpu(ptr[13]), be32_to_cpu(ptr[14]));
		printk("      \textra_head=%04d extra_tail=%04d extra_count=%04d\n", 
				be32_to_cpu(ptr[15]), be32_to_cpu(ptr[16]), be32_to_cpu(ptr[17]));
		printk("[mask]\tint=0x%08x mon=0x%08x mon_old=0x%08x op_mon=0x%04x\n", be32_to_cpu(ptr[18]),
				be32_to_cpu(ptr[19]), be32_to_cpu(ptr[20]), be32_to_cpu(ptr[21]));
		printk("[ampdu]\twrx=0x%08x wtx=0x%08x min_mpdu_space=%d\n", be32_to_cpu(ptr[22]), 
				be32_to_cpu(ptr[23]), be32_to_cpu(ptr[24]));
		printk("[rate]\twrx=0x%08x wtx=0x%08x min_rateidx=%04d\n", 
				be32_to_cpu(ptr[25]), be32_to_cpu(ptr[26]), be32_to_cpu(ptr[27]));
		printk("[misc]\tch=%d bw=%d flag=0x%x ready=%d\n", be32_to_cpu(ptr[28]), 
				be32_to_cpu(ptr[29]), be32_to_cpu(ptr[30]), be32_to_cpu(ptr[31]));
		printk("**********statistic***********\n");
		printk("[desc]\twrx:%04d\n", be32_to_cpu(ptr[32]));
		printk("[pkts]\twrx:%08d drop:%04d\n", be32_to_cpu(ptr[33]), be32_to_cpu(ptr[34]));
		printk("      \twtx:%08d ret=%08d drop:%04d full=%04d\n", be32_to_cpu(ptr[35]), 
				be32_to_cpu(ptr[36]), be32_to_cpu(ptr[37]), be32_to_cpu(ptr[38]));
		printk("      \tfail_edca[M]:%04d fail_psba[M]:%04d fail[A]:%04d\n", be32_to_cpu(ptr[39]), 
				be32_to_cpu(ptr[40]), be32_to_cpu(ptr[41]));
		printk("[count]\tpre_tbtt_int=%08d ts0_int=%08d tsx_int=%08d beacon_tx_miss=%08d\n",
				be32_to_cpu(ptr[42]), be32_to_cpu(ptr[43]), be32_to_cpu(ptr[44]), 
				be32_to_cpu(ptr[45]));
		printk("       \twrx_buf_full=%04d wrx_desc_full=%04d wrx_fifo_full=%04d\n", 
				be32_to_cpu(ptr[46]), be32_to_cpu(ptr[47]), be32_to_cpu(ptr[48]));
		printk("       \twrx_recovery=%04d wtx_recovery=%04d\n", be32_to_cpu(ptr[49]), 
				be32_to_cpu(ptr[50]));
		printk("       \thrx_pkt=%08d htx_pkt=%08d htx_done=%08d\n", be32_to_cpu(ptr[51]), 
				be32_to_cpu(ptr[52]), be32_to_cpu(ptr[53]));
		printk("       \thost_detach=%04d host_deauth=%04d ap_deauth=%04d sta_deauth=%04d"
				" ts_err=%04d\n", be32_to_cpu(ptr[54]), be32_to_cpu(ptr[55]), be32_to_cpu(ptr[56]), 
				be32_to_cpu(ptr[57]), be32_to_cpu(ptr[58]));
		printk("==========================\n");
	}

	return 0;
}
#endif

void lynx_set_dbg_lvl(int lvl)
{
	dbg_data.dbg_mask = lvl;
	printk("%s(): dbg_data.mask=%x\n", __FUNCTION__, lvl);
}

unsigned int lynx_get_dbg_lvl(void)
{
	return dbg_data.dbg_mask;
}

struct lynx_vif *lynx_get_vif_by_idx(int bss_desc)
{
	int found=0;
	struct lynx_vif *vif=NULL;

	list_for_each_entry(vif, &dbg_data.lnx->vif_list, list) 
	{
		/* FIXME: only for debug */
		if(vif->fw_vif_idx == bss_desc) 
		{
			found = 1;
			break;
		}
	}
			
	if(!found)
		vif = NULL;

	return vif;
}

/* proc filesystem */

static inline int get_args(const char *string, char *argvs[])
{
    char *p;
    int n;

    argvs[0]=0;
    n = 0;
  
    p = (char *) string;
    while (*p == ' ')
        p++;
    while (*p)
    {
        argvs[n] = p;
        while (*p != ' ' && *p)
            p++;
        if (0==*p)
            goto out;
        *p++ = '\0';
        while (*p == ' ' && *p)
            p++;
out:
    n++;
    if (n == MAX_ARGV)
        break;
    }
    return n;
}

static int lynx_func(int argc, char *argv[])
{
    printk("argc:%d %s\n", argc, argv[0]);

	if(dbg_data.lnx == NULL)
		return 0;

    if(!strcmp(argv[0], "show")) 
	{
		if(argc < 2) 
		{
			dbg_data.show_item = LYNX_DBG_INFO_SHOW;
		} 
		else 
		{
			if (!strcmp(argv[1], "dbg_level")) 
				dbg_data.show_item = LYNX_DBG_SHOW_DBG_LV;
			else if (!strcmp(argv[1], "link_st"))
				dbg_data.show_item = LYNX_DBG_SHOW_LINK_ST;
			else if (!strcmp(argv[1], "bss")) 
				dbg_data.show_item = LYNX_DBG_SHOW_BSS;
			else if (!strcmp(argv[1], "ht_cap"))
				dbg_data.show_item = LYNX_DBG_SHOW_HT_CAP;
			else if (!strcmp(argv[1], "tx_pending"))  
				dbg_data.show_item = LYNX_DBG_SHOW_TX_PENDING;
			else if (!strcmp(argv[1],"tx_rate"))
				dbg_data.show_item = LYNX_DBG_SHOW_TX_RATE;
			else if (!strcmp(argv[1],"rssi"))
				dbg_data.show_item = LYNX_DBG_SHOW_RSSI;
			else if (!strcmp(argv[1],"rssi_and_tx_rate"))
				dbg_data.show_item = LYNX_DBG_SHOW_RSSI_AND_TX_RATE;
		}
	}
    else if(!strcmp(argv[0], "set")) 
	{
		if(argc <= 2)
		{
			dbg_data.show_item = LYNX_DBG_INFO_SET;
		}
		else
		{
    		if(!strcmp(argv[1], "bss"))
			{
				if(argc >= 5)
				{
					int bss_desc=0;
    				struct lynx_vif *vif;
					struct wm_bss *bss;
					int val=0;
					
					sscanf(argv[2], "%d", &bss_desc);

					vif = lynx_get_vif_by_idx(bss_desc);

					if(!vif)
						return 0;
					
					sscanf(argv[4], "%d", &val);
					bss = &vif->bss;

#ifdef CONFIG_UNIT_TEST
					unittest_bss_compare(vif, __FUNCTION__);
#endif
					
    				if(!strcmp(argv[3], "phy_cap"))
					{
						if(val <= 7)
							bss->phy_cap = val;
					}
    				else if(!strcmp(argv[3], "slottime"))
					{
						//if(val <= 2)
						bss->slottime = val;
    					cpu_to_be16s(&(bss->slottime));
					}
    				else if(!strcmp(argv[3], "ampdu_params"))
					{
						if(val <= 2)
							bss->ampdu_params = val;
					}
					
					/* trigger the vif update */
					lynx_wci_update_vif_cmd(vif);

#ifdef CONFIG_UNIT_TEST
					unittest_bss_backup(vif, __FUNCTION__);
#endif
				}
			}
			else if(!strcmp(argv[1], "dbg_level")) 
			{
				unsigned int val;
				
				if (argc < 2)
					return 0;
				sscanf(argv[2], "%x", &val);
				dbg_data.dbg_mask = val;
				printk("DBG level: 0x%lx\n",dbg_data.dbg_mask);
			}
			else if(!strcmp(argv[1], "ht_cap"))
			{
				struct lynx_vif *vif = NULL;
				unsigned int val;

				if((!dbg_data.lnx) || ((vif = lynx_vif_first(dbg_data.lnx)) == NULL) || (argc < 2))
					return 0;
				
				sscanf(argv[2], "%x", &val);

				lynx_vif_cfg(dbg_data.lnx, vif, VIF_CFG_HT_CAP, val);
				
			} else if (!strcmp(argv[1], "min_rateidx")) {
				struct lynx_vif *vif = NULL;
				unsigned int val;

				if ((!dbg_data.lnx) ||
					((vif = lynx_vif_first(dbg_data.lnx)) == NULL) ||
					(argc < 2))
					return 0;
				
				sscanf(argv[2], "%d", &val);

				lynx_vif_cfg(dbg_data.lnx, vif, VIF_CFG_MIN_RATEIDX, val);
			} else if (!strcmp(argv[1], "ps")) {
				struct lynx_vif *vif = NULL;
				unsigned int val;

				if ((!dbg_data.lnx) ||
					((vif = lynx_vif_first(dbg_data.lnx)) == NULL) ||
					(argc < 2))
					return 0;
				
				sscanf(argv[2], "%d", &val);

				lynx_vif_cfg(dbg_data.lnx, vif, VIF_CFG_PS, val);
				if (val == 0)
					vif->wdev.ps =false;
				else
					vif->wdev.ps =true;
			} else if (!strcmp(argv[1], "monitor_level")) {
				struct lynx_vif *vif = NULL;
				unsigned int val;

				if ((!dbg_data.lnx) ||
					((vif = lynx_vif_first(dbg_data.lnx)) == NULL) ||
					(argc < 2))
					return 0;
				
				sscanf(argv[2], "%d", &val);
#ifdef CONFIG_SUPPORT_MONITOR_MODE
				lynx_monitor_switch(vif,val);
#else
				printk("Not configure monitor mode at menuconfig.\n");
#endif /*CONFIG_SUPPORT_MONITOR_MODE*/
			}
		}
	}
    else if(!strcmp(argv[0], "addba"))
	{
		if(argc < 4)
		{
			dbg_data.show_item = LYNX_DBG_INFO_ADDBA;
		}
		else
		{
			int bss_desc=0, sta_idx=0, tid=0;
			struct lynx_vif *vif;
			
			sscanf(argv[1], "%d", &bss_desc);
			vif = lynx_get_vif_by_idx(bss_desc);

			if(!vif)
				return 0;
			
			sscanf(argv[2], "%d", &sta_idx);
			sscanf(argv[3], "%d", &tid);
			
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
	}
    else if(!strcmp(argv[0], "en_2040_coex"))
	{
		if(argc < 3)
		{
			dbg_data.show_item = LYNX_DBG_INFO_2040_COEX;
		}
		else
		{
			int bss_desc=0;
			int enable=0;
			struct lynx_vif *vif;
			
			sscanf(argv[1], "%d", &bss_desc);

			vif = lynx_get_vif_by_idx(bss_desc);

			if(!vif)
				return 0;
			
			sscanf(argv[2], "%d", &enable);

			lynx_wci_set_2040_coex_cmd(vif, enable);
		}
	}
#ifdef CONFIG_HOST_DEBUG_DEVICE
	else if(!strcmp(argv[0], "dbg"))
	{
		dbg_data.show_item = LYNX_DBG_SHOW_CMD;
		if(argc >= 4)
		{
			int item=LYNX_DBG_NO_WCI_CMD;
			int is_get;
			int bss_desc;
			struct lynx_vif *vif;
			unsigned char data[32];
			int data_len=0;
    		struct lynx_hif_device *hdev = (struct lynx_hif_device *)dbg_data.lnx->hif_priv;

			sscanf(argv[1], "%d", &bss_desc);

			if((vif = lynx_get_vif_by_idx(bss_desc)) == NULL)
				return 0;

			if(strcmp(argv[2], "get") == 0)
				is_get = 1;
			else if(strcmp(argv[2], "set") == 0)
				is_get = 0;
			else 
				return 0;

			if(strcmp(argv[3], "wd_info") == 0)
			{
				item = LYNX_DBG_WD_INFO;
			}
			else if(strcmp(argv[3], "drv_path") == 0)
			{
				struct lynx_queue *q_ptr = &dbg_data.lnx->queue;
				printk("[HDEV]\tflags=0x%x, tx_buf_cnt=%d, rx_skb_alloc_fail_cnt=%d\n", 
						hdev->flags, hdev->tx_buf_cnt, hdev->rx_skb_alloc_fail_cnt);
				printk("[QUEUE]\tflags=0x%x, prio=[%d:%d:%d:%d:%d}, free_count=[%d:%d:%d:%d:%d]\n",
						q_ptr->flags, q_ptr->priority[0], q_ptr->priority[1], q_ptr->priority[2], 
						q_ptr->priority[3], q_ptr->priority[4],
						q_ptr->wmm_queue[0].free_queue_counter, 
						q_ptr->wmm_queue[1].free_queue_counter, 
						q_ptr->wmm_queue[2].free_queue_counter,
						q_ptr->wmm_queue[3].free_queue_counter,
						q_ptr->wmm_queue[4].free_queue_counter);
			}

			if(item != LYNX_DBG_NO_WCI_CMD)
			{
				lynx_wci_dbg_info_cmd(vif, is_get, item, data_len, data);
				dbg_data.show_item = LYNX_DBG_SHOW_DBG_INFO;
				dbg_data.dbg_info_item = item;
			}
		}
		else
		{
			printk("dbg usage:\n");
			printk("\techo \"dbg <bss idx> <op: get/set> <info name>\" > /proc/lynx\n");
			printk("ex: echo \"dbg 0 get wd_info\" > /proc/lynx\n");
			printk("<info name> supports \"wd_info\", \"drv_path\"\n");
		}
	}
#endif // CONFIG_HOST_DEBUG_DEVICE
#ifdef CONFIG_LYNX_WM_MANAGER
	else if(!strcmp(argv[0], "wm"))
	{
		if(argc < 2)
			return 0;
		if(strcmp(argv[1], "show") == 0)
		{
			if(argc < 3)
				return 0;
			if(strcmp(argv[2], "scan_results") == 0)
			{
				dbg_data.show_item = LYNX_DBG_WM_SCAN_RESULTS;
			}
		}
		else
		{
			wm_cmd_set((argc - 1), &argv[1]);
		}
	}
#endif
	else if(!strcmp(argv[0], "bw40"))
	{
		int en_40mhz;
		struct lynx_vif *vif;
		if(argc < 2)
			return 0;
			
		if((vif = lynx_get_vif_by_idx(0)) == NULL)
			return 0;
			
		sscanf(argv[1], "%d", &en_40mhz);

		if(en_40mhz)
			dbg_data.lnx->en_40mhz = 1;
		else
			dbg_data.lnx->en_40mhz = 0;
	
		en_40mhz = cpu_to_be32(en_40mhz);
	
		lynx_wci_set_special_param_cmd(vif, WCI_SET_EN40MHZ, sizeof(en_40mhz), (char *)&en_40mhz);
	}
	else
	{
		dbg_data.show_item = LYNX_DBG_SHOW_CMD;
	}
    
    return 0;
}

ssize_t lynx_proc_write(struct file *file, const char *buffer, size_t count, loff_t *data)
{
    char buf[300];
    int rc;
    int argc ;
    char * argv[8] ;

    if((count > 0) && (count < 299)) {
        if(copy_from_user(buf, buffer, count))
            return -EFAULT;
        buf[count-1]='\0';
        argc = get_args((const char *)buf , argv);
        rc = lynx_func(argc, argv);
    }
    return count;
}

static int lynx_calc_tx_rate_avg(unsigned char rate_idx)
{
	int rate[] = {512, 1024, 2816, 5632, 3072, 4068, 6144, 9216, 12288, 18432, 24567, 27648, 
		(6.5)*512, 13*512, (19.5)*512, 26*512, 39*512, 52*512, 58*512, 65*512};
	/* rate[] = {CCK_1M, CCK_2M, CCK_5_5M, OFDM_6M, OFDM_9M, OFDM_12M, OFDM_18M, OFDM_24M, OFDM_36M, OFDM_48M, OFDM_54M,
		MCS_0, MCS_1, MCS_2, MCS_3, MCS_4, MCS_5, MCS_6, MCS_7} */
	/* MCS_0 ~ MCS_7 use 11n20 800ns GI */

	//printk("%s(): idx=%d, rate=%d\n", __FUNCTION__, rate_idx, rate[rate_idx]);

	return (rate[rate_idx]*2);
}

static int lynx_proc_show(struct seq_file *s, void *priv)
{
	int idx;
	
	if(dbg_data.lnx == NULL)
	{
		seq_printf(s, "Dev is not exist.\n");
		return 0;
	}
	idx = dbg_data.show_item;

	if(idx == LYNX_DBG_SHOW_DBG_LV) 
	{
    	seq_printf(s, "DBG level: 0x%lx\n", dbg_data.dbg_mask);
	}
	else if(idx == LYNX_DBG_SHOW_LINK_ST)
	{
		struct lynx_vif *vif;
		int ret = -ENODEV;
		if((vif = lynx_vif_first(dbg_data.lnx)) == NULL)
			return 0;
		lynx_wci_get_vif_link_st_cmd(vif);
		ret = os_wait_link_st_complete(vif->lnx);
		if(ret)
		{
			seq_printf(s,"Dev error.\n");
		}
		else
		{
			clear_bit(WCI_LINK_ST_RET,&vif->lnx->flag);
			seq_printf(s,"tx:%08d\n",vif->lnx->fw_count[0]);
			seq_printf(s,"rx:%08d\n",vif->lnx->fw_count[1]);
			seq_printf(s,"beacon:%08d\n",vif->lnx->fw_count[2]);
			
		}
	}
	else if(idx == LYNX_DBG_SHOW_CMD) 
	{
    	seq_printf(s, "show dbg_level/bss\n");
    	seq_printf(s, "set dbg_level/bss ...\n");
		seq_printf(s, "addba <bss_idx> <sta_idx> <tid>\n");
		seq_printf(s, "en_2040_coex <bss_idx> <enable>\n");
		seq_printf(s, "dbg <bss_idx> <get/set> <recovery/disassoc/...>\n");
	}
	else if(idx == LYNX_DBG_SHOW_BSS) 
	{
		struct lynx_vif *vif;
		struct wm_bss *bss;

		list_for_each_entry(vif, &dbg_data.lnx->vif_list, list) 
		{
			bss = &vif->bss;
    		seq_printf(s, "BSS %d :\n", bss->bss_desc);
    		seq_printf(s, "myaddr : %02x:%02x:%02x:%02x:%02x:%02x\n", bss->myaddr[0], 
				bss->myaddr[1], bss->myaddr[2], bss->myaddr[3], bss->myaddr[4], bss->myaddr[5]);
    		seq_printf(s, "bssid : %02x:%02x:%02x:%02x:%02x:%02x\n", bss->bssid[0], 
				bss->bssid[1], bss->bssid[2], bss->bssid[3], bss->bssid[4], bss->bssid[5]);
    		seq_printf(s, "ssid : %s\n", bss->ssid);
    		seq_printf(s, "role : %d\n", bss->role);
    		seq_printf(s, "phy_cap : 0x%x\n", bss->phy_cap);
    		seq_printf(s, "ht_capability : 0x%x\n", bss->ht_capability);
    		seq_printf(s, "auth_capability : 0x%x\n", bss->auth_capability);
    		seq_printf(s, "slottime : 0x%x\n", bss->slottime);
    		seq_printf(s, "flag : 0x%x\n", bss->flag);
		}
	}
	else if(idx == LYNX_DBG_SHOW_HT_CAP) 
	{
		struct lynx_vif *vif;
		struct wm_bss *bss;

		if((dbg_data.lnx) && ((vif = lynx_vif_first(dbg_data.lnx)) != NULL))
		{
			bss = &vif->bss;
    		seq_printf(s, "vif ht_capability : 0x%x\n\n", be16_to_cpu(vif->bss.ht_capability));
    		seq_printf(s, "IEEE80211_HT_CAP_DSSSCCK40 = 0x%x\n", IEEE80211_HT_CAP_DSSSCCK40);
    		seq_printf(s, "IEEE80211_HT_CAP_SUP_WIDTH_20_40 = 0x%x\n", IEEE80211_HT_CAP_SUP_WIDTH_20_40);
    		seq_printf(s, "IEEE80211_HT_CAP_GRN_FLD = 0x%x\n", IEEE80211_HT_CAP_GRN_FLD);
    		seq_printf(s, "IEEE80211_HT_CAP_SGI_40 = 0x%x\n", IEEE80211_HT_CAP_SGI_40);
    		seq_printf(s, "IEEE80211_HT_CAP_SGI_20 = 0x%x\n", IEEE80211_HT_CAP_SGI_20);
		}
	}
	else if(idx == LYNX_DBG_SHOW_TX_PENDING) 
	{
		struct lynx_hif_device *lynx_usb = (struct lynx_hif_device *)dbg_data.lnx->hif_priv;
		seq_printf(s, "%d\n", lynx_usb->tx_buf_cnt);
	}
   	else if(idx == LYNX_DBG_INFO_ADDBA) 
	{
		seq_printf(s, "addba <bss_idx> <sta_idx> <tid>\n");
	}
   	else if(idx == LYNX_DBG_INFO_2040_COEX) 
	{
		seq_printf(s, "en_2040_coex <bss_idx> <enable>\n");
	}
	else if(idx == LYNX_DBG_SHOW_TX_RATE)
	{
		int rate = 0;
		int sta_idx = 0;
		struct lynx_vif *vif;

		if((vif = lynx_vif_first(dbg_data.lnx)) == NULL)
			return 0;

		if(vif->sta_idx_map == 0)
			return 0;
		
		for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
		{
			if((vif->sta_idx_map & (1 << sta_idx)) != 0)
				break;
		}

		if((sta_idx >= LYNX_STA_MAX_NUM) || (dbg_data.lnx->sta_tx_rate[sta_idx] == 0))
			return 0;

		rate = lynx_calc_tx_rate_avg(dbg_data.lnx->sta_tx_rate[sta_idx] - 1);
		seq_printf(s, "sta[%d] tx_rate(%d) : %dk\n", sta_idx, dbg_data.lnx->sta_tx_rate[sta_idx], rate);
	}
	else if(idx == LYNX_DBG_SHOW_RSSI_AND_TX_RATE)
	{
		int rate = 0;
		int sta_idx = 0;
		struct lynx_vif *vif;
		struct lynx_sta *sta=NULL;
		struct lynx *lnx;

		if((vif = lynx_vif_first(dbg_data.lnx)) == NULL)
			return 0;

		if(vif->sta_idx_map == 0)
			return 0;
		
		for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
		{
			if((vif->sta_idx_map & (1 << sta_idx)) != 0)
				break;
		}

		if((sta_idx >= LYNX_STA_MAX_NUM) || (dbg_data.lnx->sta_tx_rate[sta_idx] == 0))
			return 0;

		lnx = vif->lnx;
		sta = &lnx->sta_list[sta_idx];


		if(sta->flags & LYNX_STA_VALID)
		{
			seq_printf(s, "sta[%d] last signal = %d dBm\n", sta_idx, bb_rssi_decode(sta->signal_last, RSSI_OFFSET));

			rate = lynx_calc_tx_rate_avg(dbg_data.lnx->sta_tx_rate[sta_idx] - 1);
			seq_printf(s, "sta[%d] tx_rate(%d) : %dk\n", sta_idx, dbg_data.lnx->sta_tx_rate[sta_idx], rate);
		}
		else
			return 0;

	}
	else if(idx == LYNX_DBG_SHOW_RSSI)
	{
		int sta_idx = 0;
		struct lynx_vif *vif;
		struct lynx_sta *sta=NULL;
		struct lynx *lnx;

		if((vif = lynx_vif_first(dbg_data.lnx)) == NULL)
			return 0;

		if(vif->sta_idx_map == 0)
			return 0;
		
		for(sta_idx=0; sta_idx<LYNX_STA_MAX_NUM; sta_idx++)
		{
			if((vif->sta_idx_map & (1 << sta_idx)) != 0)
				break;
		}

 		if((sta_idx >= LYNX_STA_MAX_NUM) || (dbg_data.lnx->sta_tx_rate[sta_idx] == 0))
			return 0;

		lnx = vif->lnx;
		sta = &lnx->sta_list[sta_idx];

		if(sta->flags & LYNX_STA_VALID)
			seq_printf(s, "sta[%d] last signal = %d dBm\n", sta_idx, bb_rssi_decode(sta->signal_last, RSSI_OFFSET));
	}	
	else if(idx == LYNX_DBG_SHOW_DBG_INFO)
	{
	}
#ifdef CONFIG_LYNX_WM_MANAGER
	else if(idx == LYNX_DBG_WM_SCAN_RESULTS)
	{
		wm_cmd_get(WM_GET_SCAN_RESULTS, s);
	}
#endif
 
	return 0;
}

static int lynx_proc_open(struct inode *inode, struct file *file)
{
#if 0
	return single_open_size(file, lynx_proc_show, NULL, SEQ_FILE_BUFSIZE);
#else
	int ret;
	struct seq_file *p;
	ret = single_open(file, lynx_proc_show, NULL);
	if(ret==0)
	{
		p = (struct seq_file *) file->private_data;
	 	   
		if(p->buf==NULL)
		{
			p->buf = kmalloc(SEQ_FILE_BUFSIZE, GFP_KERNEL);
	 	
			if(p->buf)
				p->size = SEQ_FILE_BUFSIZE;
	 	}
	}
	 	
	return ret;
#endif
}

static const struct file_operations lynx_fops = {
    .open       = lynx_proc_open,
    .write 		= lynx_proc_write,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

//int lynx_proc_init_fs(struct lynx *lnx)
int lynx_proc_init_fs(void)
{
	int ret = 0;
    dbg_data.proc_dir = proc_create(LYNX_PROCFS_NAME, S_IWUGO | S_IRUGO, NULL, &lynx_fops);
	
	if(dbg_data.proc_dir)
	{
		//dbg_data.lnx = lnx;
		dbg_data.dbg_mask = 0;
		//dbg_data.dbg_mask = LYNX_DEBUG_DEFAULT;
		dbg_data.dbg_mask =  LYNX_DBG_WLAN_CFG|LYNX_DBG_ERR|LYNX_DBG_ANDROID;
		//dbg_data.dbg_mask =  LYNX_DBG_WLAN_CFG|LYNX_DBG_WCI|LYNX_DBG_WCI_EVENT|LYNX_DBG_USB_PROBE|LYNX_DBG_ERR;
		dbg_data.show_item = LYNX_DBG_SHOW_CMD;
	}
	else
		ret = -ENOMEM;

    return ret;
}

//void lynx_proc_exit_fs(struct lynx *lnx)
void lynx_proc_exit_fs(void)
{
	if(dbg_data.lnx && dbg_data.proc_dir)
	{
		remove_proc_entry(LYNX_PROCFS_NAME, NULL);
		dbg_data.proc_dir = NULL;
		//dbg_data.lnx = NULL;
	}
}

#ifdef CONFIG_UNIT_TEST
int unittest_bss_compare(struct lynx_vif *vif, const char *ret_func)
{
	int ret = -1;

	if(vif)
	{
		if(memcmp(&vif->bss, &vif->bss_copy, sizeof(struct wm_bss)) == 0)
			ret = 0;
		else
		{
			printk("%s(): ERROR !! The vif->bss is overwritten!\n", ret_func);
			printk("The vif->bss :\n");
			dump_buf((char *)&vif->bss, sizeof(struct wm_bss));
			printk("The vif->bss_copy :\n");
			dump_buf((char *)&vif->bss_copy, sizeof(struct wm_bss));
		}
	}

	return ret;	
}

int unittest_bss_backup(struct lynx_vif *vif, const char *ret_func)
{
	printk("%s(): do unittest_bss_backup()\n", ret_func);
	memcpy(&vif->bss_copy, &vif->bss, sizeof(struct wm_bss));

	return 0;
}

#endif	// CONFIG_UNIT_TEST

#endif	// CONFIG_LYNX_DEBUG
