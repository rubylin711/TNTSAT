/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2020 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/spinlock.h>

//#include <net/netlink.h>
//#include <linux/security.h>
//#include <net/net_namespace.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include <net/sock.h>
#include <net/genetlink.h>

#include "mt_type.h" //flax

#include "mt_drv_mmz.h"

//#include <drv_frontend.h>
#include "mt_unf_frontend.h"
#include "mt_fe_tn_montage_ts6011.h"
//#include "mt_fe_sat_tn_montage_ts2022.h"
#include "mt_fe_common.h"
#include "port_cs8800.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_i2c_cs8800.h"

#include "mt_fe_tn_MxL603.h"
#include "./MxL6/MxL603_TunerCfg.h"
#include "mt_module_debug.h"

#include <linux/time.h>
#include "drv_gpio_ioctl.h"
#include "mt_common.h"

#include "mt_drv_clock.h"
#include "mt_drv_analog.h"

#include "mt_mach/chipinfo.h"
#include "hal_demux_regs.h"
#include "mach/otp_simple.h"

#define DEMO_BOARD_TEST_DISEQC2

//#define FOR_PORT_CS8800_CONNECT_SYCHRONOUS

/* Sychronous blind scan */
#define CFG_SYNC_BLINDSCAN
/* blind scan wait user callback timeout: 120s */
#define BLINDSCAN_WAIT_CB_TIMEOUT 120

#if 0
/*!
  GPIO direction, output
  */
#define GPIO_DIR_OUTPUT 0x0
/*!
  GPIO direction, input
  */
#define GPIO_DIR_INPUT 0x1

/*!
  GPIO output value, low
  */
#define GPIO_LEVEL_LOW 0x0
/*!
  GPIO output value, high
  */
#define GPIO_LEVEL_HIGH 0x1
#endif

#define MT_FE_CS8800_PIN_LEVEL_LOW 0
#define MT_FE_CS8800_PIN_LEVEL_HIGH 1
//#define MT_FE_CS8800_DISEQC_COMMAND_START_DELAY 30
#define MT_FE_CS8800_DISEQC_COMMAND_START_DELAY 50
#define MT_FE_CS8800_DISEQC_COMMAND_END_DELAY 50

#define MAX_TP_ONE_SCAN MAX_BS_TP_NUM_PER_SAT

#define CS8800_NL_NAME "dvbs_bs_nl"

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum
{
	CS8800_NL_OPS_CMD0,
	//CS8800_NL_OPS_CMD1,
	__CS8800_NL_OPS_MAX,
};

#define CS8800_NL_OPS_AMOUNT (__CS8800_NL_OPS_MAX)

enum
{
	CS8800_NL_ATTR_UNSPEC,
	CS8800_NL_ATTR_DATA0,
	//CS8800_NL_ATTR_DATA1,
	__CS8800_NL_ATTR_MAX,
};

#define CS8800_NL_FAMILY_ATTR_MAX (__CS8800_NL_ATTR_MAX - 1)
#define CS8800_NL_FAMILY_ATTR_AMOUNT (__CS8800_NL_ATTR_MAX)

struct cs8800_netlink_data
{
	pid_t pid;
	u32 seq;
	u32 freq_khz;
	u32 symbol_rate;
	u32 port_type;
	u8 DataTsNumber;
	u8 ts_id;
	u8 ts_index;
	u8 DataTsIdArray[32];
	u32 data01;
	u8 cmd;
	char name[32];
};

struct cs8800_netlink_pid_list
{
	struct list_head list;
	struct cs8800_netlink_data cs8800_nl_data;
	struct net *net;
};

static MT_FE_CS8800_Device_Handle g_cs8800_handle = NULL;

#ifdef CFG_SYNC_BLINDSCAN
/* blind scan -> found one TP -> wait user callback return */
static int event_status;
/* blindscan callback wait queue */
static wait_queue_head_t bs_cb_wq;
#endif

static MT_FE_LOCK_STATE ss2_status = MtFeLockState_Undef;

//mt_fe_cs8800_priv_handle p_priv = NULL;
static MT_FE_CS8800_Device_Handle dev_handle = NULL;
static mt_u8 g_dev_init_flag = 0;
static mt_u8 g_priv_count = 0;
static MT_BOOL  g_bSuperSearch = 0;
extern MT_FE_RET mt_fe_dmd_register_notify_cs8800_ss2(void (*callback)(MT_FE_MSG msg, void *p_tp_info));

extern MXL_STATUS MxLWare603_OEM_ReadRegister(UINT8 devId, UINT8 RegAddr, UINT8 *DataPtr);

/* The crypto netlink socket */
//static struct sock *cs8800_nlsk;

mt_fe_cs8800_priv_handle g_cs8800_priv = NULL;
int g_i2c_cs8800 = 0;
static MT_U8 cs8800_notify_scan_status = 0; //0:normal, 1:finished or abort
mt_unf_fe_channel_info_t *cs8800_pg_channel_info = NULL;

extern MT_BOOL g_bNeedReCali;

//mmz_buffer_s sMBuf;

static void port_m88cs8800_get_dbg_info(MT_FE_CS8800_Device_Handle handle);

int m88cs8800_suspend(void);
int m88cs8800_resume(void);

static int port_m88cs8800_blind_scan_cancel(void *handle);
static int port_m88cs8800_blind_scan_start(void *pArg);
static int port_m88cs8800_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info);
static int cs8800_nl_list_del(void);
static int port_m88cs8800_diseqc2_rx_get_bytes(void *p_param);
static int m88cs8800_set_scid_filter(mt_unf_fe_dss_scid_filter_t  *scid_filter);
static LIST_HEAD(cs8800_nl_pid_list);

static DEFINE_MUTEX(cs8800_nl_mutex);

static struct genl_ops cs8800_nl_ops[CS8800_NL_OPS_AMOUNT];

static struct genl_family cs8800_nl_family = {
	.name = CS8800_NL_NAME,
	.version = 0x1,
	.maxattr = CS8800_NL_FAMILY_ATTR_MAX,
	.netnsok = true,
	.module = THIS_MODULE,
	.ops = cs8800_nl_ops,
	.n_ops = ARRAY_SIZE(cs8800_nl_ops),
};

extern struct genl_family dvbs_nl_family;

//static struct cs8800_netlink_data g_cs8800_nl_data_snd = {0};

MT_FE_CS8800_Device_Handle mt_fe_cs8800_get_handle(void)
{
	return g_cs8800_handle;
}

static int cs8800_nl_sendmsg(mt_unf_fe_channel_info_t *p_channel_info)
{
	//mt_unf_fe_channel_info_t *p_channel_info = NULL;
	struct genl_info info;
	struct cs8800_netlink_data cs8800_nl_data_snd;
	struct sk_buff *skb;
	struct cs8800_netlink_data *cs8800_nl_data;
	struct cs8800_netlink_pid_list *cs8800_nl_pid;
	struct cs8800_netlink_pid_list *tmp;
	void *hdr;
	pid_t pid;
	int i = 0;

	if (mutex_lock_interruptible(&cs8800_nl_mutex))
	{
		return -ERESTARTSYS;
	}

	list_for_each_entry_safe(cs8800_nl_pid, tmp, &cs8800_nl_pid_list, list)
	{
		cs8800_nl_data = &(cs8800_nl_pid->cs8800_nl_data);
		pid = cs8800_nl_data->pid;

		skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
		if (!skb)
		{
			return -ENOMEM;
		}

		hdr = genlmsg_put(skb, 0 /* from kernel */,
						  cs8800_nl_data->seq, &cs8800_nl_family, 0, cs8800_nl_data->cmd);
		if (!hdr)
			goto out_nlmsg_free;

		/*
		 * hdr is first nlattr or user header which size is genl_family->hdrsize,
		 * if genl_family->hdrsize is 0, there is no user header
		 */

		info.snd_portid = pid; /* to app */
		genl_info_net_set(&info, cs8800_nl_pid->net);

		NETLINK_CB(skb).portid = 0; /* from kernel */

		snprintf(cs8800_nl_data_snd.name, sizeof(cs8800_nl_data_snd.name), "kernel nl");
		cs8800_nl_data_snd.freq_khz = p_channel_info->frequency; //0xaabbccdd;
		cs8800_nl_data_snd.symbol_rate = p_channel_info->symbol_rate;
		cs8800_nl_data_snd.port_type = p_channel_info->port_type;
		cs8800_nl_data_snd.DataTsNumber = p_channel_info->DataTsNumber;
		cs8800_nl_data_snd.ts_id = p_channel_info->ts_id;
		cs8800_nl_data_snd.ts_index = p_channel_info->ts_index;
		for (i = 0; i < p_channel_info->DataTsNumber; i++)
		{
			cs8800_nl_data_snd.DataTsIdArray[i] = p_channel_info->DataTsIdArray[i];
		}

		cs8800_nl_data_snd.pid = pid;
		cs8800_nl_data_snd.seq = cs8800_nl_data->seq + 1;
		cs8800_nl_data_snd.cmd = cs8800_nl_data->cmd;
		cs8800_nl_data_snd.data01 = cs8800_nl_data->data01;

		//NLA_PUT_TYPE(skb, typeof(cs8800_nl_data_snd), cs8800_nl_data_snd.data01, cs8800_nl_data_snd);
		if (nla_put(skb, cs8800_nl_data_snd.data01, sizeof(cs8800_nl_data_snd), &cs8800_nl_data_snd))
			goto nla_put_failure;

		genlmsg_end(skb, hdr);

		genlmsg_reply(skb, &info);

		list_del(&(cs8800_nl_pid->list));
		kfree(cs8800_nl_pid);
	}

	mutex_unlock(&cs8800_nl_mutex);

	return 0;

nla_put_failure:
	genlmsg_cancel(skb, hdr);

out_nlmsg_free:
	nlmsg_free(skb);
	mutex_unlock(&cs8800_nl_mutex);

	return -ENOMEM;
}

static int cs8800_nl_list_del(void)
{
	struct cs8800_netlink_pid_list *cs8800_nl_pid;
	struct cs8800_netlink_pid_list *tmp;

	if (mutex_lock_interruptible(&cs8800_nl_mutex))
		return -ERESTARTSYS;

	list_for_each_entry_safe(cs8800_nl_pid, tmp, &cs8800_nl_pid_list, list)
	{
		list_del(&(cs8800_nl_pid->list));
		kfree(cs8800_nl_pid);
	}

	mutex_unlock(&cs8800_nl_mutex);

	return 0;
}

#if 0
static void pinmux_configure(void)
{
	u32 temp = 0;

	/* NIM */
	_mt_fe_read32_cs8800(0xbf13c010, &temp);
	if ((((temp >> 16) & 0x0f) != 0x02) || (((temp >> 20) & 0x0f) != 0x02))
	{
		temp &= ~(0xF << 16);
		temp &= ~(0xF << 20);
		temp |= (2 << 16);
		temp |= (2 << 20);
		_mt_fe_write32_cs8800(0xbf13c010, temp);
	}

	_mt_fe_read32_cs8800(0xbf138008, &temp);
	if (temp != 0x311)
	{
		temp = 0x311;
		_mt_fe_write32_cs8800(0xbf138008, temp);
	}
}
#endif
static U8 port_m88cs8800_get_blindscan_status(MT_FE_CS8800_Device_Handle cs8800_handle)
{
	U8 ret = 0;
	unsigned long flags;
	
	spin_lock_irqsave(&cs8800_handle->blindscan_status_slock, flags);
	ret = cs8800_handle->m_device_ss2.global_cfg.bBsStatus;
	spin_unlock_irqrestore(&cs8800_handle->blindscan_status_slock, flags);
	return ret;
}
static void port_m88cs8800_set_blindscan_status(MT_FE_CS8800_Device_Handle cs8800_handle,U8 status)
{
	unsigned long flags;
	
	spin_lock_irqsave(&cs8800_handle->blindscan_status_slock, flags);
	cs8800_handle->m_device_ss2.global_cfg.bBsStatus = status;
	spin_unlock_irqrestore(&cs8800_handle->blindscan_status_slock, flags);	
}
mt_unf_fe_fec_type_t port_m88cs8800_deparse_dvb_type(MT_FE_TYPE dvb_type)
{
	mt_unf_fe_fec_type_t temp;

	switch (dvb_type)
	{
	case MtFeType_DVBS:
		temp = MT_UNF_FE_DVBS;
		break;

	case MtFeType_DVBS2:
		temp = MT_UNF_FE_DVBS2;
		break;

	default:
		temp = MT_UNF_FE_BUTT; //NIM_UNDEF;
		break;
	}

	return temp;
}

mt_unf_fe_fecrate_t port_m88cs8800_deparse_code_rate(MT_FE_CODE_RATE code_rate)
{
	mt_unf_fe_fecrate_t temp;

	switch (code_rate)
	{
	case MtFeCodeRate_1_4:
		temp = MT_UNF_FE_FEC_1_4;
		break;

	case MtFeCodeRate_1_3:
		temp = MT_UNF_FE_FEC_1_3;
		break;

	case MtFeCodeRate_2_5:
		temp = MT_UNF_FE_FEC_2_5;
		break;

	case MtFeCodeRate_1_2:
		temp = MT_UNF_FE_FEC_1_2;
		break;

	case MtFeCodeRate_3_5:
		temp = MT_UNF_FE_FEC_3_5;
		break;

	case MtFeCodeRate_2_3:
		temp = MT_UNF_FE_FEC_2_3;
		break;

	case MtFeCodeRate_3_4:
		temp = MT_UNF_FE_FEC_3_4;
		break;

	case MtFeCodeRate_4_5:
		temp = MT_UNF_FE_FEC_4_5;
		break;

	case MtFeCodeRate_5_6:
		temp = MT_UNF_FE_FEC_5_6;
		break;

	case MtFeCodeRate_7_8:
		temp = MT_UNF_FE_FEC_7_8;
		break;

	case MtFeCodeRate_8_9:
		temp = MT_UNF_FE_FEC_8_9;
		break;

	case MtFeCodeRate_9_10:
		temp = MT_UNF_FE_FEC_9_10;
		break;

	default:
		temp = MT_UNF_FE_FEC_UNDEF;
		break;
	}

	return temp;
}

#define SAT_C_MIN_KHZ (3000000)
#define SAT_C_MAX_KHZ (4200000)
#define SAT_KU_MIN_KHZ (10600000)
#define SAT_KU_MAX_KHZ (12750000)
#define SAT_DOWNLINK_FREQ_KU_MID (11700)
#define SAT_SYMBOLRATE_MAX (60000000) //(45000000)

static int port_m88cs8800_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
	//MT_U32 for_scan = 0;
	MT_FE_RET ret = 0;
	MT_FE_TYPE dvb_type = MtFeType_Undef;
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	p_channel_info = &(para->channel_info);
	para->channel_info.lock = 0;

	//printk("%s[%d]: para->sig_type = %d\n", __FUNCTION__, __LINE__, para->sig_type);

	if ((para->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(para->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(para->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(para->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		ss2_status = MtFeLockState_Unlocked;

		if (MT_UNF_PORT_TYPE_DVBS == para->connect_param.sat.port_type)
		{
			//cs8800_handle->demod_type = MtFeType_DVBS;
			dvb_type = MtFeType_DVBS;
		}
		else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
		{
			//cs8800_handle->demod_type = MtFeType_DVBS2;
			dvb_type = MtFeType_DVBS2;
		}
		else
		{
			//cs8800_handle->demod_type = MtFeType_DVBS2;
			dvb_type = MtFeType_DVBS_S2;
		}

		printk("cs8800_channel set ss2: freq = %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
			   para->connect_param.sat.freq,
			   para->connect_param.sat.sym_rate,
			   p_priv->for_scan,
			   para->connect_param.sat.port_type,
			   para->connect_param.sat.uc_param.use_uc);

		if ((para->connect_param.sat.uc_param.use_uc) && (0 == para->connect_param.sat.lnb_status))
		{
			cs8800_handle->m_device_ss2.lnb_cfg.bUnicable = 1;
			cs8800_handle->m_device_ss2.lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
			cs8800_handle->m_device_ss2.lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
			cs8800_handle->m_device_ss2.lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
			cs8800_handle->m_device_ss2.lnb_cfg.iUBVer = para->connect_param.sat.uc_param.ub_ver;
		}
		else
		{
			cs8800_handle->m_device_ss2.lnb_cfg.bUnicable = 0;
		}

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
		//priv->for_scan = p_channel_set_info->for_scan;


		if (para->connect_param.sat.PLS.PLSDetail.PLSType == 2)
		{
			u32 PLSGoldCode = para->connect_param.sat.PLS.PLSCodeAll & 0xFFFFFF;

			cs8800_handle->m_device_ss2.tp_cfg.bHavePLS = 1;
			mt_unf_fe_calc_PLS_gold_code(cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode, PLSGoldCode);
		}
		else if (para->connect_param.sat.PLS.PLSDetail.PLSType == 1)
		{
			cs8800_handle->m_device_ss2.tp_cfg.bHavePLS = 1;
			cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[0] = para->connect_param.sat.PLS.PLSDetail.PLSCode[0];
			cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[1] = para->connect_param.sat.PLS.PLSDetail.PLSCode[1];
			cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[2] = para->connect_param.sat.PLS.PLSDetail.PLSCode[2];
		}
		else
		{
			cs8800_handle->m_device_ss2.tp_cfg.bHavePLS = 0;
			cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[0] = 0x01;
			cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[1] = 0x00;
			cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[2] = 0x00;
		}


		cs8800_handle->m_device_ss2.input_params.input_freq_kHz = para->connect_param.sat.freq;
		cs8800_handle->m_device_ss2.input_params.symbol_rate_KSs = para->connect_param.sat.sym_rate;
		cs8800_handle->m_device_ss2.demod_type = dvb_type;

		cs8800_handle->m_device_ss2.tp_cfg.ucCurTsId = para->connect_param.sat.ts_id;

#if MT_FE_DMD_DVBS_S2_SUPPORT
		ret = mt_fe_dmd_connect_ss2_cs8800(cs8800_handle);
#endif

		if (ret == MtFeErr_Ok)
		{
			/* set check lock delay time */
			if (dvb_type == MtFeType_DVBS)
			{
				if (para->connect_param.sat.sym_rate >= 2000)
				{
					para->channel_set_info.lock_time = 1000; //1s
				}
				else
				{
					para->channel_set_info.lock_time = 3000; //3s
				}
			}
			else if (dvb_type == MtFeType_DVBS2)
			{
				if (para->connect_param.sat.sym_rate >= 10000)
				{
					para->channel_set_info.lock_time = 800; //800ms
				}
				else if (para->connect_param.sat.sym_rate >= 5000)
				{
					para->channel_set_info.lock_time = 1500; //1.5s
				}
				else if (para->connect_param.sat.sym_rate >= 2000)
				{
					para->channel_set_info.lock_time = 3000; //3s
				}
				else
				{
					para->channel_set_info.lock_time = 6000; //6s
				}
			}
			else
			{
				if (para->connect_param.sat.sym_rate >= 10000)
				{
					para->channel_set_info.lock_time = 1300; //1.3s
				}
				else if (para->connect_param.sat.sym_rate >= 4000)
				{
					para->channel_set_info.lock_time = 3200; //3.2s
				}
				else if (para->connect_param.sat.sym_rate >= 2000)
				{
					para->channel_set_info.lock_time = 3500; //3.5s
				}
				else
				{
					para->channel_set_info.lock_time = 8500; //8.5s
				}
			}
			//printk("[%s] line:%d lock_time %d\n", __func__, __LINE__, para->channel_set_info.lock_time);

			return MT_SUCCESS;
		}
		else
		{
			printk("mt_fe_dmd_connect_ss2_cs8800 failed %d\n", ret);
		}
	}
	else if (para->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
	{
		dvb_type = MtFeType_DVBC;

		printk("cs8800_channel set c: freq = %d, symbol rate = %d, type = %d\n",
			   para->connect_param.cab.freq,
			   para->connect_param.cab.sym_rate,
			   dvb_type);

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

		cs8800_handle->m_device_c_b.input_params.input_freq_kHz = para->connect_param.cab.freq;
		cs8800_handle->m_device_c_b.input_params.symbol_rate_KSs = para->connect_param.cab.sym_rate / 1000;

		switch (para->connect_param.cab.mod_type)
		{
			case MT_UNF_MOD_TYPE_QAM_16:
				cs8800_handle->m_device_c_b.input_params.qam = 16;
				break;

			case MT_UNF_MOD_TYPE_QAM_32:
				cs8800_handle->m_device_c_b.input_params.qam = 32;
				break;

			case MT_UNF_MOD_TYPE_QAM_128:
				cs8800_handle->m_device_c_b.input_params.qam = 128;
				break;

			case MT_UNF_MOD_TYPE_QAM_256:
				cs8800_handle->m_device_c_b.input_params.qam = 256;
				break;

			case MT_UNF_MOD_TYPE_QAM_64:
				cs8800_handle->m_device_c_b.input_params.qam = 64;
				break;

			default:
				cs8800_handle->m_device_c_b.input_params.qam = 0;
				break;
		}

		cs8800_handle->m_device_c_b.demod_type = dvb_type;

		ret = mt_fe_dmd_connect_c_b_cs8800(cs8800_handle);

		para->channel_set_info.lock_time = 800;
	}
	else if (para->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
	{
		dvb_type = MtFeType_J83B;

		printk("cs8800_channel set b: freq = %d, symbol rate = %d, type = %d\n",
			   para->connect_param.cab.freq,
			   para->connect_param.cab.sym_rate,
			   dvb_type);

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

		cs8800_handle->m_device_c_b.input_params.input_freq_kHz = para->connect_param.cab.freq;
		cs8800_handle->m_device_c_b.input_params.symbol_rate_KSs = para->connect_param.cab.sym_rate;

		switch (para->connect_param.cab.mod_type)
		{
			case MT_UNF_MOD_TYPE_QAM_256:
				cs8800_handle->m_device_c_b.input_params.qam = 256;
				break;

			case MT_UNF_MOD_TYPE_QAM_64:
			default:
				cs8800_handle->m_device_c_b.input_params.qam = 64;
				break;
		}

		cs8800_handle->m_device_c_b.demod_type = dvb_type;

		ret = mt_fe_dmd_connect_c_b_cs8800(cs8800_handle);

		para->channel_set_info.lock_time = 800;
	}

	if (ret == MtFeErr_Ok)
	{
		return MT_SUCCESS;
	}

	return MT_FAILURE;
}

static void port_m88cs8800_set_22k_onoff(void *handle, MT_U8 onoff_22k, MT_U8 diseqc_out_when_lnb_off)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
	U8 val_0xa1, val_0xa2;
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	if ((cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		mt_fe_dmd_open_ss2_cs8800(cs8800_handle, MtFeType_DVBS);
	}

	_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xa1, &val_0xa1);
	_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xa2, &val_0xa2);

	if (onoff_22k == MtFe_True)
	{
		val_0xa1 |= 0x04;
		val_0xa1 &= ~0x03;
		val_0xa1 &= ~0x40;
		val_0xa2 &= ~0xc0;
	}
	else
	{
		if (diseqc_out_when_lnb_off == MT_FE_CS8800_PIN_LEVEL_HIGH)
		{
			val_0xa2 |= 0xc0;
		}
		else
		{
			val_0xa2 &= ~0xc0;
			val_0xa2 |= 0x80;
		}
	}

	_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xa2, val_0xa2);
	_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xa1, val_0xa1);

	printk("port_m88cs8800_set_22k_onoff %d - %s\n", onoff_22k, (onoff_22k == MtFe_True) ? "22K On" : "22K Off");
#endif
}

static void port_m88cs8800_set_lnb_voltage(void *handle, MT_FE_LNB_VOLTAGE voltage, MT_U8 vsel_pin_13v)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
	MT_U8 val_0xa2;
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	if ((cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		mt_fe_dmd_open_ss2_cs8800(cs8800_handle, MtFeType_DVBS);
	}

	_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xa2, &val_0xa2);
	printk("port_m88cs8800_set_lnb_voltage %d - %dV\n", voltage, (voltage == MtFeLNB_13V) ? 13 : 18);

	if (vsel_pin_13v == MT_FE_CS8800_PIN_LEVEL_HIGH)
	{
		if (voltage == MtFeLNB_13V)
		{
			val_0xa2 |= 0x01;
		}
		else
		{
			val_0xa2 &= ~0x01;
		}
	}
	else
	{
		if (voltage == MtFeLNB_13V)
		{
			val_0xa2 &= ~0x01;
		}
		else
		{
			val_0xa2 |= 0x01;
		}
	}

	_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xa2, val_0xa2);
	val_0xa2 = 0;
	_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xa2, &val_0xa2);
#endif
}

static void port_m88cs8800_set_lnb_onoff(void *handle, MT_U8 lnb_enable, mt_unf_fe_pin_config_para_t *pin_config)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
	U8 val_0xa1, val_0xa2; //, pin, level, value;
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	//pinmux_configure();

	if ((cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		mt_fe_dmd_open_ss2_cs8800(cs8800_handle, MtFeType_DVBS);
	}

	_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xa2, &val_0xa2);

	printk("port_m88cs8800_set_lnb_onoff %d - %s\n", lnb_enable, (lnb_enable == MtFe_True) ? "On" : "Off");

	if (lnb_enable != MtFe_True) // off
	{
		_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xa1, &val_0xa1);
		val_0xa1 |= 0x40;
		_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xa1, val_0xa1);

		if (pin_config->lnb_enable_by_mcu == 0)
		{
			/*set LNB_EN pin HIGH or LOW */
			if (pin_config->lnb_enable == MT_FE_CS8800_PIN_LEVEL_HIGH)
			{
				val_0xa2 &= ~0x02;
			}
			else
			{
				val_0xa2 |= 0x02;
			}
			//printk(" port_m88cs8800_set_lnb_onoff line:%d\n", __LINE__);
		}
		else
		{
#if 0
			/* control by mcu's gpio */
			pin = pin_config->lnb_enable_pin;
			level = pin_config->lnb_enable;
			//hal_pinmux_gpio_enable(pin, TRUE);
			//gpio_ioctl(GPIO_CMD_IO_ENABLE, pin, TRUE);
			gpio_get_dir(pin, &value);
			if (GPIO_DIR_INPUT == value)
			{
				gpio_set_dir(pin, GPIO_DIR_OUTPUT);
				printk("off: set pin output\n");
			}
#if 1
			gpio_get_value(pin,&value);
			printk("off: get lnb_enable_pin value[%d]\n", value);
			if (value == level)
			{
				gpio_set_value(pin,(level == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
				printk("off: set lnb_enable_pin value[%d]\n", (level == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
			}
#else
			gpio_set_value(pin,(level == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
#endif
#endif
		}

		/*set V_SEL Pin mode*/
		if (pin_config->vsel_when_lnb_off == MT_FE_CS8800_PIN_LEVEL_HIGH)
		{
			val_0xa2 |= 0x01;
		}
		else
		{
			val_0xa2 &= ~0x01;
		}

		/*set DiseQc_OUT mode*/
		if (pin_config->diseqc_out_when_lnb_off == MT_FE_CS8800_PIN_LEVEL_HIGH)
		{
			val_0xa2 |= 0xc0;
		}
		else
		{
			val_0xa2 &= ~0xc0;
			val_0xa2 |= 0x80;
		}
	}
	else
	{
		if (pin_config->lnb_enable_by_mcu == 0)
		{
			//printk("port_m88cs8800_set_lnb_onoff line:%d.\n", __LINE__);
			if (pin_config->lnb_enable == MT_FE_CS8800_PIN_LEVEL_HIGH)
			{
				val_0xa2 |= 0x02;
			}
			else
			{
				val_0xa2 &= ~0x02;
			}
		}
		else
		{
#if 0
			/* control by mcu's gpio */
			pin = pin_config->lnb_enable_pin;
			level = pin_config->lnb_enable;
			printk("lnb_enable_pin[%d], level[%d]\n", pin, level);
			//hal_pinmux_gpio_enable(pin, TRUE);
			//printk("gpio_ioctl\n");
			//gpio_ioctl(GPIO_CMD_IO_ENABLE, pin, TRUE);
			printk("gpio_get_dir\n");
			gpio_get_dir(pin, &value);
			if (GPIO_DIR_INPUT == value)
			{
				printk("set lnb_enable_pin output\n");
				gpio_set_dir(pin, GPIO_DIR_OUTPUT);
			}
#if 1
			gpio_get_value(pin, &value);
			printk("off: get lnb_enable_pin value[%d]\n", value);
			if (value != level)
			{
				printk("set lnb_enable_pin value[%d]\n", level);
				gpio_set_value(pin, level);
			}
#else
			gpio_set_value(pin, level);
#endif
#endif
		}
	}

	_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xa2, val_0xa2);
#endif
}

static int port_m88cs8800_diseqc_sendmsg(void *handle, mt_unf_fe_diseqc_sendmsg_t *p_diseqc_sendmsg)
{
	MT_FE_RET ret = MtFeErr_Undef;

#if MT_FE_DMD_DVBS_S2_SUPPORT
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	MT_FE_DiSEqC_MSG msg;

	//return MT_SUCCESS;

	if (p_diseqc_sendmsg->len == 0) // || p_diseqc_sendmsg->p_tx_buf == NULL)
	{
		return MT_FAILURE;
	}

	if (p_diseqc_sendmsg->len > 8)
	{
		printk("ERROR:p_diseqc_sendmsg->len > 8\n");
		return MT_FAILURE;
	}

	memcpy(p_priv->cur_diseqc.p_tx_buf, p_diseqc_sendmsg->data, p_diseqc_sendmsg->len);

	p_priv->cur_diseqc.mode = PORT_DISEQC_BYTES; //p_diseqc_cmd->mode;
	p_priv->cur_diseqc.tx_len = p_diseqc_sendmsg->len;
	//p_priv->cur_diseqc.rx_len = p_diseqc_cmd->rx_len;

	p_priv->diseqc_2x = (p_diseqc_sendmsg->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? 1 : 0;

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}

	if (p_priv->onoff_22k != 0)
	{
		/* 22k is opened , close it */
		port_m88cs8800_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

		/* make sure the voltage stable */
		//msleep(MT_FE_CS8800_DISEQC_COMMAND_START_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms
	}

	memset(&msg, 0x00, sizeof(MT_FE_DiSEqC_MSG));
	msg.size_send = p_diseqc_sendmsg->len;
	memcpy(msg.data_send, p_diseqc_sendmsg->data, msg.size_send);

#if 0
	if (p_priv->diseqc_2x == 1)
	{
		msg.is_enable_receive = 1;
	}
#endif

	if ((p_priv->diseqc_2x == 1) && (p_priv->cfg.pin_config.diseqc_rx_mode == 0))
	{
		msg.is_enable_receive = 1;
	}

	printk("%s[%d] ---- DiSEqC send msg[0x%02x, 0x%02x, 0x%02x, 0x%02x], send size[%d], enable_receive[%d]\n", __FUNCTION__, __LINE__, 
		   msg.data_send[0],
		   msg.data_send[1],
		   msg.data_send[2],
		   msg.data_send[3],
		   msg.size_send, 
		   msg.is_enable_receive);

	ret = mt_fe_dmd_DiSEqC_send_msg_ss2_cs8800(cs8800_handle, &msg);

	if (p_priv->onoff_22k != 0)
	{
		/* make sure the voltage stable */
		//msleep(MT_FE_CS8800_DISEQC_COMMAND_END_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

		/* 22k was closed, open it */
		port_m88cs8800_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
	}
#endif

	return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88cs8800_diseqc_send_tone_burst(void *handle, mt_u8 mode)
{
	MT_FE_RET ret = MtFeErr_Undef;

#if MT_FE_DMD_DVBS_S2_SUPPORT
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	p_priv->cur_diseqc.mode = 0; //PORT_DISEQC_BURST0;//p_diseqc_cmd->mode;

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}

	if (p_priv->onoff_22k != 0)
	{
		/* 22k is opened, close it */
		port_m88cs8800_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

		/* make sure the voltage stable */
		//msleep(MT_FE_CS8800_DISEQC_COMMAND_START_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms
	}

	mode = (mode == 0) ? MtFeDiSEqCToneBurst_Unmoulated : MtFeDiSEqCToneBurst_Moulated;
	ret = mt_fe_dmd_DiSEqC_send_tone_burst_ss2_cs8800(cs8800_handle, mode, 0);

	if (p_priv->onoff_22k != 0)
	{
		/* make sure the voltage stable */
		//msleep(MT_FE_CS8800_DISEQC_COMMAND_END_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

		/* 22k was closed, open it */
		port_m88cs8800_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
	}
#endif

	return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88cs8800_diseqc_recvmsg(void *handle, mt_unf_fe_diseqc_recvmsg_t *p_diseqc_recvmsg)
{
	MT_FE_RET ret = MtFeErr_Undef;

#if MT_FE_DMD_DVBS_S2_SUPPORT
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	MT_FE_DiSEqC_MSG msg;
	//MT_FE_DiSEqC_TONE_BURST mode;

	p_priv->cur_diseqc.mode = PORT_DISEQC_BYTES;
	//p_priv->cur_diseqc.tx_len = p_diseqc_cmd->tx_len;
	p_priv->cur_diseqc.rx_len = p_diseqc_recvmsg->len;

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}

	if (p_priv->onoff_22k != 0)
	{
		/* 22k is opened , close it */
		port_m88cs8800_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

		/* make sure the voltage stable */
		//msleep(MT_FE_CS8800_DISEQC_COMMAND_START_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms
	}

	memset(&msg, 0x00, sizeof(MT_FE_DiSEqC_MSG));

#if 0
	if (p_priv->diseqc_2x == 1)
	{
		msg.is_enable_receive = 1;
	}
#else
	p_priv->diseqc_2x = 1;

	if ((p_priv->diseqc_2x == 1) && (p_priv->cfg.pin_config.diseqc_rx_mode == 0) && (p_diseqc_recvmsg->len > 0))
		msg.is_enable_receive = 1;
	else
		msg.is_enable_receive = 0;
#endif

	ret = mt_fe_dmd_DiSEqC_receive_msg_ss2_cs8800(cs8800_handle, &msg);

	if ((p_priv->diseqc_2x == 1) && (p_priv->cfg.pin_config.diseqc_rx_mode == 1) && (p_diseqc_recvmsg->len > 0)) //diseqc2
	{
		//RET_CODE rx_ret = MT_SUCCESS;
		int index = 0;
		//printk("rxget bytes\n");
		ret = port_m88cs8800_diseqc2_rx_get_bytes(p_priv);

		//printk("---- diseqc_rx_get_bytes() reclen:%d, return %d\n", p_diseqc_recvmsg->len, ret);

		if (ret == MtFeErr_Ok)
		{
			msg.size_receive = p_diseqc_recvmsg->len;
			//printk("DiSEqC recv msg[");
			for (index = 0; index < p_diseqc_recvmsg->len; index++)
			{
				msg.data_receive[index] = p_priv->diseqc_rx_buf[index];
				p_diseqc_recvmsg->msg[index] = p_priv->diseqc_rx_buf[index];
				// printk("0x%x, ",p_diseqc_recvmsg->msg[index]);
			}
			//printk("] recv size[%d]\n",p_diseqc_recvmsg->len);
		}
		else
		{
			printk("read gpio rx data fail\n");
		}
	}
	else
	{
		p_diseqc_recvmsg->len = msg.size_receive;
		memcpy(p_diseqc_recvmsg->msg, msg.data_receive, p_diseqc_recvmsg->len);
	}

	printk("DiSEqC recv msg[0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x] recv size[%d]\n",
		   msg.data_receive[0],
		   msg.data_receive[1],
		   msg.data_receive[2],
		   msg.data_receive[3],
		   msg.data_receive[4],
		   msg.data_receive[5],
		   msg.data_receive[6],
		   msg.data_receive[7],
		   msg.size_receive);

	if (ret == MtFeErr_Ok)
	{
		p_diseqc_recvmsg->status = MT_UNF_FE_DISEQC_RECV_OK;
	}
	else if (ret == MtFeErr_TimeOut)
	{
		p_diseqc_recvmsg->status = MT_UNF_FE_DISEQC_RECV_TIMEOUT;
	}
	else
	{
		p_diseqc_recvmsg->status = MT_UNF_FE_DISEQC_RECV_ERROR;
	}

	if (p_priv->onoff_22k != 0)
	{
		/* make sure the voltage stable */
		//msleep(MT_FE_CS8800_DISEQC_COMMAND_START_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

		/* 22k was colsed, open it */
		port_m88cs8800_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
	}
#endif

	return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}
static int port_m88cs8800_get_fast_lock(void *handle, mt_unf_fe_status_t *p_status)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	MT_FE_LOCK_STATE stat = 0;


	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		mt_fe_dmd_get_fast_lock_cs8800_ss2(cs8800_handle, &stat);
	}
	else
	{
		/*TODO*/
		printk("\n%s[%d] -- DVBC or J83B unsupport fast lock now !!\n", __FUNCTION__, __LINE__);
	}
	
	if (stat == MtFeLockState_Locked)
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;
	}
	else
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
		if (stat == MtFeLockState_Unlocked)
		{
			p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
		}
		else
		{
			p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;
		}
	}
	return MT_SUCCESS;
}
static int port_m88cs8800_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	MT_FE_LOCK_STATE stat = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
#if MT_FE_DMD_DVBS_S2_SUPPORT
		if (ss2_status != MtFeLockState_Locked)
		{
			if(cs8800_handle)  cs8800_handle->m_device_ss2.tp_cfg.bSuperSearch = g_bSuperSearch;
			mt_fe_dmd_get_lock_state_ss2_cs8800(cs8800_handle, &stat);
		}
		else
		{
			mt_fe_dmd_get_pure_lock_cs8800_ss2(cs8800_handle, &stat);
		}

		if (stat == MtFeLockState_Locked)
		{
			int iCnt = 0, index = 0;

			if (ss2_status != stat)
				ss2_status = stat;

			p_status->param.connect_param.sat.DataTsNumber = cs8800_handle->m_device_ss2.tp_cfg.iTsCnt;
			p_status->param.connect_param.sat.ts_id = cs8800_handle->m_device_ss2.tp_cfg.ucCurTsId;

			p_priv->cur_channel.DataTsNumber = cs8800_handle->m_device_ss2.tp_cfg.iTsCnt;
			p_priv->cur_channel.ts_id = cs8800_handle->m_device_ss2.tp_cfg.ucCurTsId;

			for (iCnt = 0; iCnt < cs8800_handle->m_device_ss2.tp_cfg.iTsCnt; iCnt++)
			{
				p_status->param.connect_param.sat.DataTsIdArray[iCnt] = cs8800_handle->m_device_ss2.tp_cfg.ucTsId[iCnt];
				p_priv->cur_channel.DataTsIdArray[iCnt] = cs8800_handle->m_device_ss2.tp_cfg.ucTsId[iCnt];

				if (cs8800_handle->m_device_ss2.tp_cfg.ucTsId[iCnt] == cs8800_handle->m_device_ss2.tp_cfg.ucCurTsId)
					index = iCnt;
			}

			p_status->param.connect_param.sat.ts_index = index;
			p_priv->cur_channel.ts_index = index;

			p_status->param.connect_param.sat.PLS.PLSDetail.PLSType = cs8800_handle->m_device_ss2.tp_cfg.bHavePLS ? 1 : 0;
			p_status->param.connect_param.sat.PLS.PLSDetail.PLSCode[0] = cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[0];
			p_status->param.connect_param.sat.PLS.PLSDetail.PLSCode[1] = cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[1];
			p_status->param.connect_param.sat.PLS.PLSDetail.PLSCode[2] = cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[2];

			p_priv->cur_channel.PLS.PLSDetail.PLSType = cs8800_handle->m_device_ss2.tp_cfg.bHavePLS ? 1 : 0;
			p_priv->cur_channel.PLS.PLSDetail.PLSCode[0] = cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[0];
			p_priv->cur_channel.PLS.PLSDetail.PLSCode[1] = cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[1];
			p_priv->cur_channel.PLS.PLSDetail.PLSCode[2] = cs8800_handle->m_device_ss2.tp_cfg.ucPLSCode[2];

#if 1 /*fix issue 29991*/
			// temp add check tsi sample sta reg 
			if(1)
			{
				mt_u32 tsi1_sta_reg = 0;
				mt_u8   tmp = 0;
				int count = 0;
				unsigned long chip_rev = symphony_get_chip_rev();

				if(CHIP_SYMPHONY6_A1 == chip_rev)
				{ 
					tsi1_sta_reg = reg_get_ts1_sample_sta();
					//printk("%s[%d] -- tsi1 sta = 0x%02x\n", __FUNCTION__, __LINE__, tsi1_sta_reg);
					while((tsi1_sta_reg & 0xc0000000) == 0x80000000) 
					{
					  #if 1
					   _mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xf0, &tmp);
					   tmp = tmp & 0xfd;    //bit1 = 0;
					   _mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xf0, tmp);
					   msleep(1);
                       tmp = tmp | 0x2;    //bit1 : 1
                       _mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xf0, tmp);
                       msleep(1);
					   #else
					   /*reg_set_ts1_sample_ctrl_ts_en(0);
					   msleep(1);
					   reg_set_ts1_sample_ctrl_ts_en(1);*/
					   val = HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl));
					   pr_err("[%s %d]reg_dmx_ts1_sample_ctrl:0x%x\n", __FUNCTION__, __LINE__,HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl)));
					   HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), 0);
					   pr_err("[%s %d]reg_dmx_ts1_sample_ctrl:0x%x\n", __FUNCTION__, __LINE__,HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl)));
					   delay_ms = 50;//10;
					   msleep(delay_ms);// 1ms failed  100ms OK
					   HAL_PUT_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl), val);
					   pr_err("[%s %d]reg_dmx_ts1_sample_ctrl:0x%x,delay_ms:%d\n", __FUNCTION__, __LINE__,HAL_GET_U32((volatile mt_u32 *)(reg_dmx_ts1_sample_ctrl)),delay_ms);
					   #endif
					   count ++;
					   if(count > 5) break;
					   tsi1_sta_reg = reg_get_ts1_sample_sta();
				 	   //printk("%s[ count = %d] -- tsi1 sta = 0x%02x\n", __FUNCTION__, count, tsi1_sta_reg);
					}
				}
			}
#endif			
		}
		else
		{
			printk("\n%s[%d] -- DVBS unlcoked !!\n", __FUNCTION__, __LINE__);
			if(0) // dump all reg
			{
				int i=0;
				mt_u8 reg_addr[256] = {0};
				mt_u8 reg_data = 0;
				for (i = 0; i <= 0xff; i++)
                		{
                        		_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, i, &reg_data);
                        		printk("%02x - %02x ", i, reg_data);
                        		if ((i + 1) % 8 == 0)
                                		printk(KERN_CONT"\n");
                		}
                		if (MtFeTn_TS6011 == cs8800_handle->m_device_ss2.tuner_cfg.tuner_type)
                		{
                        		printk("\n[%d] dump TS6011 registers 00 ~ ff (in hex)\n", __LINE__);
                        		for (i = 0; i <= 0xff; i++)
                        		{
                                		reg_addr[i] = i;
                                		_mt_fe_tn_get_reg_ss2_cs8800(cs8800_handle, reg_addr[i], &reg_data);
                                		printk("%02x - %02x ", i, reg_data);
                                		if ((i + 1) % 8 == 0)
                                        		printk(KERN_CONT"\n");
                        		}
				}
                	}	
		}
#endif
	}
	else
	{
		mt_fe_dmd_get_lock_state_c_b_cs8800(cs8800_handle, &stat);

		if (p_priv->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
		{
			U8 tmp = 0;
			//int iCnt = 0;

			_mt_fe_dmd_get_reg_c_cs8800(cs8800_handle, 0x85, &tmp);
			//printk("%s[%d] -- DVB-C 0x85 = 0x%02x\n", __FUNCTION__, __LINE__, tmp);

#if 0
			_mt_fe_dmd_get_reg_t2_cs8800(cs8800_handle, 0x04, &tmp);
			printk("\n%s[%d] -- Systemp 0x04 = 0x%02x\n", __FUNCTION__, __LINE__, tmp);

			_mt_fe_dmd_get_reg_t2_cs8800(cs8800_handle, 0x07, &tmp);
			printk("%s[%d] -- Systemp 0x07 = 0x%02x\n", __FUNCTION__, __LINE__, tmp);

			printk("\n%s[%d] -- Dump all DVB-C registers\n", __FUNCTION__, __LINE__);
			for (iCnt = 0; iCnt < 0x100; iCnt++)
			{
				_mt_fe_dmd_get_reg_c_cs8800(cs8800_handle, (U8)iCnt, &tmp);

				printk("%02x = %02x\n", iCnt, tmp);
			}

			printk("\n%s[%d] -- Dump all TC6800 registers\n", __FUNCTION__, __LINE__);
			for (iCnt = 0; iCnt < 0x100; iCnt++)
			{
				_mt_fe_tn_get_reg_c_b_cs8800(cs8800_handle, (U8)iCnt, &tmp);

				printk("%02x = %02x\n", iCnt, tmp);
			}
#endif
		}
	}

	if (stat == MtFeLockState_Locked)
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;
	}
	else
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
		if (stat == MtFeLockState_Unlocked)
		{
			p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
		}
		else
		{
			p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;
		}
	}

	//printk(KERN_ERR "lock=%d, reason=%d\n", p_status->lock_status, p_status->unlock_reason);
	return MT_SUCCESS;
}

static int port_m88cs8800_get_signal_quality(void *handle, MT_U32 *p_quality)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	int ret = 0;
	U8 percent = 0;

	// For DVB-S or DVB-S2
	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		ret = mt_fe_dmd_get_quality_ss2_cs8800(cs8800_handle, &percent);
	}
	else // For DVB-C or J83B
	{
		ret = mt_fe_dmd_get_quality_c_b_cs8800(cs8800_handle, &percent);
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	*p_quality = percent;

	return MT_SUCCESS;
}

static int port_m88cs8800_get_ber(void *handle, MT_U32 *p_ber)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	U32 err_packages = 0;
	U32 total_packages = 0xffff;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2))) // For DVB-S or DVB-S2
	{
		//mt_fe_dmd_get_per_cs8800_ss2(cs8800_handle, &err_packages, &total_packages);
		mt_fe_dmd_get_per_cs8800_ss2(cs8800_handle, &total_packages, &err_packages);
	}
	else // For DVB-C or J83B
	{
		mt_fe_dmd_get_per_c_b_cs8800(cs8800_handle, &err_packages, &total_packages);
	}

	p_ber[0] = total_packages;
	p_ber[1] = err_packages;
	p_ber[2] = 0;

	return MT_SUCCESS;
}

static int port_m88cs8800_get_pre_ber(void *handle, MT_U32 *p_ber)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	U32 err_packages = 0;
	U32 total_packages = 1024 * 64800;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2))) // For DVB-S or DVB-S2
	{
		mt_fe_dmd_get_pre_ber_cs8800_ss2(cs8800_handle, &err_packages, &total_packages);

		//printk("%s[%d] ---- err_packages = [%6d], total_packages = [%6d]\n", __FUNCTION__, __LINE__, err_packages, total_packages);
	}
	else // For DVB-C or J83B
	{
	}

	p_ber[0] = total_packages;
	p_ber[1] = err_packages;
	p_ber[2] = 0;

	//printk("%s[%d] ---- p_ber[0] = [%6d], p_ber[1] = [%6d], p_ber[2] = [%6d]\n", __FUNCTION__, __LINE__, p_ber[0], p_ber[1], p_ber[2]);

	return MT_SUCCESS;
}
static int port_m88cs8800_get_accurate_snr(void *handle, MT_S32 *p_snr)
{
    mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
    int ret = 0;
    S32 _snr = 0;
    
	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2))) // For DVB-S or DVB-S2
	{
		ret = mt_fe_dmd_get_accurate_snr_cs8800_ss2(cs8800_handle, &_snr);
	}
	else // For DVB-C or J83B
	{
		ret = mt_fe_dmd_get_accurate_snr_c_b_cs8800(cs8800_handle, &_snr);
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	*p_snr = _snr;

	return MT_SUCCESS;
}
static int port_m88cs8800_get_snr(void *handle, MT_U32 *p_snr)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	S8 _snr = 0;
	int ret = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2))) // For DVB-S or DVB-S2
	{
		ret = mt_fe_dmd_get_snr_cs8800_ss2(cs8800_handle, &_snr);
	}
	else // For DVB-C or J83B
	{
		ret = mt_fe_dmd_get_snr_c_b_cs8800(cs8800_handle, &_snr);
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	*p_snr = _snr;

	return MT_SUCCESS;
}

static int port_m88cs8800_get_signal_strength(void *handle, MT_U32 *p_strength)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	S8 _strength = 0;
	int ret = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2))) // For DVB-S or DVB-S2
	{
		ret = mt_fe_dmd_get_strength_cs8800_ss2(cs8800_handle, &_strength);
	}
	else // For DVB-C or J83B
	{
		ret = mt_fe_dmd_get_strength_c_b_cs8800(cs8800_handle, &_strength);
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	*p_strength = _strength;

	return MT_SUCCESS;
}

static void port_m88cs8800_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	if ((p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_sig_info->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		MT_FE_CHAN_INFO_DVBS2 ch_info;

		S32 tuner_offset_KHZ = 0, carrieroffset_KHz;
		U32 sym_rate_KSs = 27500;

		_mt_fe_dmd_get_carrier_offset_cs8800_ss2(cs8800_handle, &carrieroffset_KHz);

		cs8800_handle->m_device_ss2.tuner_cfg.tuner_get_offset(cs8800_handle, &tuner_offset_KHZ);
		p_sig_info->sig_info.sat.freq = cs8800_handle->m_device_ss2.tp_cfg.iFreqKHz + tuner_offset_KHZ - carrieroffset_KHz;

		//printk("%s[%d] -- SS2 freq[%d], carrier offset[%d], tuner_offset[%d], result[%d]\n", __FUNCTION__, __LINE__, 
		//        ct8k_handle->m_device_ss2.tp_cfg.iFreqKHz, carrieroffset_KHz, tuner_offset_KHZ, p_sig_info->sig_info.sat.freq);

		_mt_fe_dmd_get_sym_rate_cs8800_ss2(cs8800_handle, &sym_rate_KSs);
		p_sig_info->sig_info.sat.symbol_rate = sym_rate_KSs;

		//printk("%s[%d] -- FE[%d], type[%d], SS2 freq[%d], sym_rate[%d]\n", __FUNCTION__, __LINE__, 
		//        gFeIndex, gFeList[gFeIndex], p_sig_info->sig_info.sat.freq, p_sig_info->sig_info.sat.symbol_rate);

		mt_fe_dmd_get_channel_info_cs8800_ss2(cs8800_handle, &ch_info);

		if (cs8800_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS)
		{
			p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_SAT;
			p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_DVBS;
		}
		else if (cs8800_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS2)
		{
			p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_SAT_2;
			p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_DVBS2;
		}
		else
		{
			p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
			p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_BUTT;
		}

		switch (ch_info.mod_mode)
		{
			case MtFeModMode_Qpsk:        p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_QPSK;      break;
			case MtFeModMode_8psk:        p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_8PSK;      break;
			case MtFeModMode_16Apsk:      p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_16APSK;    break;
			case MtFeModMode_32Apsk:      p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_32APSK;    break;
			case MtFeModMode_64Apsk:      p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_64APSK;    break;
			case MtFeModMode_128Apsk:     p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_128APSK;   break;
			case MtFeModMode_256Apsk:     p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_256APSK;   break;
			case MtFeModMode_8Apsk_L:     p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_8APSK_L;   break;
			case MtFeModMode_16Apsk_L:    p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_16APSK_L;  break;
			case MtFeModMode_32Apsk_L:    p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_32APSK_L;  break;
			case MtFeModMode_64Apsk_L:    p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_64APSK_L;  break;
			case MtFeModMode_128Apsk_L:   p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_128APSK_L; break;
			case MtFeModMode_256Apsk_L:   p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_256APSK_L; break;
			default:                      p_sig_info->sig_info.sat.mode_type = MT_UNF_MOD_TYPE_AUTO;      break;
		}

		switch (ch_info.code_rate)
		{
			case MtFeCodeRate_1_4:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_1_4;      break;
			case MtFeCodeRate_1_3:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_1_3;      break;
			case MtFeCodeRate_2_5:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_2_5;      break;
			case MtFeCodeRate_1_2:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_1_2;      break;
			case MtFeCodeRate_3_5:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_3_5;      break;
			case MtFeCodeRate_2_3:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_2_3;      break;
			case MtFeCodeRate_3_4:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_3_4;      break;
			case MtFeCodeRate_4_5:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_4_5;      break;
			case MtFeCodeRate_5_6:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_5_6;      break;
			case MtFeCodeRate_7_8:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_7_8;      break;
			case MtFeCodeRate_8_9:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_8_9;      break;
			case MtFeCodeRate_9_10:       p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_9_10;     break;
			case MtFeCodeRate_5_9:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_5_9;      break;
			case MtFeCodeRate_7_9:        p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_7_9;      break;
			case MtFeCodeRate_4_15:       p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_4_15;     break;
			case MtFeCodeRate_7_15:       p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_7_15;     break;
			case MtFeCodeRate_8_15:       p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_8_15;     break;
			case MtFeCodeRate_11_15:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_11_15;    break;
			case MtFeCodeRate_13_18:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_13_18;    break;
			case MtFeCodeRate_9_20:       p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_9_20;     break;
			case MtFeCodeRate_11_20:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_11_20;    break;
			case MtFeCodeRate_23_36:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_23_36;    break;
			case MtFeCodeRate_25_36:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_25_36;    break;
			case MtFeCodeRate_11_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_11_45;    break;
			case MtFeCodeRate_13_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_13_45;    break;
			case MtFeCodeRate_14_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_14_45;    break;
			case MtFeCodeRate_26_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_26_45;    break;
			case MtFeCodeRate_28_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_28_45;    break;
			case MtFeCodeRate_29_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_29_45;    break;
			case MtFeCodeRate_31_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_31_45;    break;
			case MtFeCodeRate_32_45:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_32_45;    break;
			case MtFeCodeRate_77_90:      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_77_90;    break;
			default:                      p_sig_info->sig_info.sat.fec_rate = MT_UNF_FE_FEC_AUTO;     break;
		}

		p_sig_info->sig_info.sat.pilot_mode = ch_info.is_pilot_on;

		p_sig_info->sig_info.sat.roll_off = (mt_unf_fe_roll_off_t)ch_info.roll_off;

		//printk("%s[%d] ---- roll off = %d, %d\n", __FUNCTION__, __LINE__, p_sig_info->sig_info.sat.roll_off, ch_info.roll_off);
	}
	else if (p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
	{
		if (cs8800_handle->m_device_c_b.demod_type == MtFeType_J83B)
		{
			p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_J83B;
		}
		else
		{
			p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_BUTT;
		}

		p_sig_info->sig_info.cab.freq = cs8800_handle->m_device_c_b.input_params.input_freq_kHz;
		p_sig_info->sig_info.cab.symbol_rate = cs8800_handle->m_device_c_b.input_params.symbol_rate_KSs;

		switch (cs8800_handle->m_device_c_b.input_params.qam)
		{
			case 64:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_64;
				break;

			case 256:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_256;
				break;

			default:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_DEFAULT;
				break;
		}
	}
	else
	{
		if (cs8800_handle->m_device_c_b.demod_type == MtFeType_DVBC)
		{
			p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_DVBC;
		}
		else
		{
			p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_BUTT;
		}

		p_sig_info->sig_info.cab.freq = cs8800_handle->m_device_c_b.input_params.input_freq_kHz;
		p_sig_info->sig_info.cab.symbol_rate = cs8800_handle->m_device_c_b.input_params.symbol_rate_KSs;

		switch (cs8800_handle->m_device_c_b.input_params.qam)
		{
			case 16:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_16;
				break;

			case 32:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_32;
				break;

			case 64:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_64;
				break;

			case 128:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_128;
				break;

			case 256:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_256;
				break;

			default:
				p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_DEFAULT;
				break;
		}
	}

	/*
	p_sig_info->frequency = handle->m_device_c_b.input_params.input_freq_kHz;
	p_sig_info->bandwidth = handle->m_device_c_b.input_params.demod_bandwidth;
	*/
}

static int port_m88cs8800_get_signal_agc(void *handle, mt_u32 *p_agc)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	int ret = 0;
	mt_u32 tuner_gain = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		if (cs8800_handle->m_device_ss2.tuner_cfg.tuner_get_gain)
		{
			ret = cs8800_handle->m_device_ss2.tuner_cfg.tuner_get_gain(cs8800_handle, &tuner_gain);

			if (ret < 0)
			{
				return MT_FAILURE;
			}
			p_agc[0] = tuner_gain;
			printk("drv tuner_gain[%d]\n", tuner_gain);
		}
	}
	else // For DVB-C or J83B
	{
		if (cs8800_handle->m_device_c_b.tuner_cfg.tuner_get_gain)
		{
			ret = cs8800_handle->m_device_c_b.tuner_cfg.tuner_get_gain(cs8800_handle, &tuner_gain);

			if (ret < 0)
			{
				return MT_FAILURE;
			}

			p_agc[0] = tuner_gain;
			printk("drv tuner_gain[%d]\n", tuner_gain);
		}
	}

	return MT_SUCCESS;
}

static int port_m88cs8800_standby(void *handle)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	mt_fe_dmd_sleep_cs8800(cs8800_handle);

	return MT_SUCCESS;
}

static int port_m88cs8800_wakeup(void *handle)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	mt_fe_dmd_wake_cs8800(cs8800_handle);

	return MT_SUCCESS;
}

static int port_m88cs8800_get_default_timeout(void *handle, mt_u32 *timeout)
{
	*timeout = 120;

	return MT_SUCCESS;
}

static int port_m88cs8800_set_io(void *handle, MT_BOOL onoff)
{
#if 0
	int ret = 0;
	ret = mt_fe_dmd_ca8k_cab_set_io(handle, onoff);
	if (ret < 0)
		return ret;
#endif
	return MT_SUCCESS;
}
static void print_analog_status(int line)
{
   u32 analog_cadc_status = 0;
   u32 drv0_clk_status = 0;
   u32 analog_sadc_status = 0;
   u32 drv1_clk_status = 0;
   int ret = 0;
   ret = mt_analog_get_status(MT_ANA_CADC,&analog_cadc_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%d]MT_ANA_CADC status:0x%x \n", line,analog_cadc_status);
   }
   ret = mt_analog_get_status(MT_ANA_SADC,&analog_sadc_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%d]MT_ANA_SADC status:0x%x \n", line,analog_sadc_status);
   }
   ret = mt_analog_get_status(DRV0_CLK_ANA,&drv0_clk_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%d]DRV0_CLK_ANA status:0x%x \n", line,drv0_clk_status);
   }
   ret = mt_analog_get_status(DRV1_CLK_ANA,&drv1_clk_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%d]DRV1_CLK_ANA status:0x%x \n", line,drv1_clk_status);
   }
}
static void port_m88cs8800_check_and_enable_analog(void *handle,mt_unf_fe_sig_type_t sig_type)
{
   u32 analog_cadc_status = 0;
   u32 drv0_clk_status = 0;
   u32 analog_sadc_status = 0;
   u32 drv1_clk_status = 0;
   mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
   MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
   int ret = 0;

   ret = mt_analog_get_status(MT_ANA_CADC,&analog_cadc_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%s %d]MT_ANA_CADC status:0x%x \n", __FUNCTION__, __LINE__,analog_cadc_status);
   }
   ret = mt_analog_get_status(MT_ANA_SADC,&analog_sadc_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%s %d]MT_ANA_SADC status:0x%x \n", __FUNCTION__, __LINE__,analog_sadc_status);
   }
   ret = mt_analog_get_status(DRV0_CLK_ANA,&drv0_clk_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%s %d]DRV0_CLK_ANA status:0x%x \n", __FUNCTION__, __LINE__,drv0_clk_status);
   }
   ret = mt_analog_get_status(DRV1_CLK_ANA,&drv1_clk_status);
   if(ret == MT_SUCCESS)
   {
       printk("[%s %d]DRV1_CLK_ANA status:0x%x \n", __FUNCTION__, __LINE__,drv1_clk_status);
   }
   if(!(analog_cadc_status & drv0_clk_status))
   {
        cs8800_handle->bDVBCInitOk = FALSE;
   }
   if(!(analog_cadc_status & drv1_clk_status))
   {
        cs8800_handle->bJ83BInitOk = FALSE;
   }
   if(!(analog_sadc_status & drv1_clk_status))
   {
        cs8800_handle->bDVBSInitOk = FALSE;
   }
   if ((sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
   {
        if(cs8800_handle->bDVBSInitOk == FALSE)
        {
            mt_analog_enable(MT_ANA_SADC);
			mt_analog_enable(DRV1_CLK_ANA);//DVBS
            cs8800_handle->bDVBSInitOk = TRUE;
            printk("[%s %d]MT_ANA_SADC and DRV1_CLK_ANA enable \n", __FUNCTION__, __LINE__);
        }
   }
   else if (sig_type == MT_UNF_FE_SIG_TYPE_CAB)
   {
        if(cs8800_handle->bDVBCInitOk == FALSE)
        {
            mt_analog_enable(MT_ANA_CADC);
            mt_analog_enable(DRV0_CLK_ANA);//DVBC 
            cs8800_handle->bDVBCInitOk = TRUE;
            printk("[%s %d]MT_ANA_CADC and DRV0_CLK_ANA enable \n", __FUNCTION__, __LINE__);
        }
   }
   else if (sig_type == MT_UNF_FE_SIG_TYPE_J83B)
   {
        if(cs8800_handle->bJ83BInitOk == FALSE)
        {
            mt_analog_enable(MT_ANA_CADC);
            mt_analog_enable(DRV1_CLK_ANA);//DVBS J83B
            cs8800_handle->bJ83BInitOk = TRUE;
            printk("[%s %d]MT_ANA_CADC and DRV1_CLK_ANA enable \n", __FUNCTION__, __LINE__);
        }
   }
   else
   {
        printk(KERN_ERR "[%s %d]Unsupport Type!! \n", __FUNCTION__, __LINE__);
   }
   print_analog_status(__LINE__);
}
static int port_m88cs8800_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	//MT_FE_RET ret = MtFeErr_Ok;
	//MT_FE_LOCK_STATE status = 0;
	//MT_U32 cnt = 0;

	//U8 tmp;
	MT_U8 for_bs = 0;
	MT_FE_TYPE dvb_type = MtFeType_Undef;
	MT_FE_BS_TP_INFO bs_tpinfo;
	MT_FE_TP_INFO tp_info;
	MT_U8 voltage = 0;
	MT_U8 count;

	p_priv->sig_type = para->sig_type;
	p_channel_info = &(para->channel_info);
	//pinmux_configure();

	//printk("----port_m88cs8800_channel_connect() log1, sig_type = %d\n", para->sig_type);

	cs8800_handle->connect_mode = (int)para->connect_mode;
    
    /*Task 27399 Lowpower optimize*/
    port_m88cs8800_check_and_enable_analog(handle,p_priv->sig_type);
    
	if ((para->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(para->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(para->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(para->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		//if the blindscan is running, stop it first
		count = 0;
		//printk(KERN_ERR "[%s %d]bBsStatus=%d\n", __FUNCTION__, __LINE__, port_m88cs8800_get_blindscan_status(cs8800_handle));
		/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
		while (1 == port_m88cs8800_get_blindscan_status(cs8800_handle))
		{
			cs8800_handle->m_device_ss2.global_cfg.bCancelBs = TRUE;
			msleep(2);
			count++;
			if (count > 100)
			{
				break;
			}
		}
        
        /*fix issue27868 After J83B lock,DVBS can not lock anymore
        * RootCause:CS8800 demod J83B and DVBS can not work in the same time.
        * After J83B lock,DVBS firmware will lost.So when DVBS lock after J83B,DVBS firmware need be download again
        */
        if(cs8800_handle->m_device_c_b.demod_current_type == MtFeType_J83B)
            cs8800_handle->m_device_c_b.demod_current_type = MtFeType_Undef;
        
		//printk("%s() %d: para->connect_param.sat.port_type = %d\n", __FUNCTION__, __LINE__, para->connect_param.sat.port_type);
#if MT_FE_DMD_DVBS_S2_SUPPORT
		if (MT_UNF_PORT_TYPE_DVBS == para->connect_param.sat.port_type)
		{
			dvb_type = MtFeType_DVBS;
		}
		else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
		{
			dvb_type = MtFeType_DVBS2;
		}
		else
		{
			dvb_type = MtFeType_DVBS_S2;
		}

		printk("cs8800 connect: freq = %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
			   para->connect_param.sat.freq,
			   para->connect_param.sat.sym_rate,
			   for_bs,
			   para->connect_param.sat.port_type,
			   para->connect_param.sat.uc_param.use_uc);

		if (0 == para->connect_param.sat.lnb_status)
		{
#if 0
			if (p_priv->lnb_onoff == 0)
			{
				return MT_SUCCESS;
			}
#endif

			if (p_priv->lnb_polar != para->connect_param.sat.polarization)
			{
				voltage = (para->connect_param.sat.polarization == PORT_PORLAR_HORIZONTAL) ? MtFeLNB_18V : MtFeLNB_13V;
				port_m88cs8800_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
				p_priv->lnb_polar = (MT_U8)para->connect_param.sat.polarization;
				p_priv->lnb_voltage = voltage;
			}

			if (para->connect_param.sat.uc_param.use_uc)
			{
				cs8800_handle->m_device_ss2.lnb_cfg.bUnicable = 1;
				cs8800_handle->m_device_ss2.lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
				cs8800_handle->m_device_ss2.lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
				cs8800_handle->m_device_ss2.lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
				printk("drv line[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
					   __LINE__,
					   cs8800_handle->m_device_ss2.lnb_cfg.iBankIndex,
					   cs8800_handle->m_device_ss2.lnb_cfg.iUBIndex,
					   cs8800_handle->m_device_ss2.lnb_cfg.iUBFreqMHz);
			}
			else
			{
				cs8800_handle->m_device_ss2.lnb_cfg.bUnicable = 0;
			}
		}

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
		para->channel_info.lock = 0;

		if (for_bs)
		{
			p_priv->for_scan = 1;
			bs_tpinfo.tp_num = 0;
			bs_tpinfo.p_tp_info = &tp_info;
			bs_tpinfo.p_tp_info[0].dvb_type = dvb_type;
			bs_tpinfo.p_tp_info[0].freq_KHz = p_channel_info->frequency;
			bs_tpinfo.p_tp_info[0].sym_rate_KSs = para->connect_param.sat.sym_rate;

			if (_mt_fe_dmd_bs_connect_cs8800_ss2(p_priv->cs8800_handle, &bs_tpinfo) == MtFeErr_Ok)
			{
				if (bs_tpinfo.tp_num > 0)
				{
					p_channel_info->frequency = bs_tpinfo.p_tp_info[0].freq_KHz;
					p_channel_info->symbol_rate = bs_tpinfo.p_tp_info[0].sym_rate_KSs;
					p_channel_info->port_type = port_m88cs8800_deparse_dvb_type(bs_tpinfo.p_tp_info[0].dvb_type);
					p_channel_info->fec_inner = port_m88cs8800_deparse_code_rate(bs_tpinfo.p_tp_info[0].code_rate);
					p_channel_info->lock = 1;
				}
				return MT_SUCCESS;
			}
		}
		else
		{
			p_priv->for_scan = 0;
			para->channel_set_info.for_scan = 0;
			port_m88cs8800_channel_set(p_priv, para);
#if 0
			//printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
			for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
			{
				mt_fe_dmd_get_lock_state_ss2_cs8800(cs8800_handle, &status);
				para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
				if (para->channel_info.lock)
				{
					//printk("cs8800 connect lock success\n");
					return MT_SUCCESS;
				}
				//_mt_sleep_cs8800(10);
				msleep(10);
			}

			//return ERR_TIMEOUT;

#if 1
			port_m88cs8800_get_dbg_info(cs8800_handle);
#else
			printk("%s[%d]: dump DVB-S&S2 demod registers start.\n", __FUNCTION__, __LINE__);
			for (cnt = 0; cnt < 0x100; cnt++)
			{
				_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, (U8)cnt, &tmp);
				printk("%s[%d]: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
			}
			printk("%s[%d]: dump DVB-S&S2 demod registers end.\n", __FUNCTION__, __LINE__);
#endif

#endif
			return MT_SUCCESS;
		}
#endif
	}
	else if (para->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
	{
		dvb_type = MtFeType_DVBC;

#if 0
		printk("cs8800_channel set c: freq =  %d, symbol rate = %d, type = %d\n",
				para->connect_param.cab.freq,
				para->connect_param.cab.sym_rate,
				dvb_type);
#endif

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
		para->channel_info.lock = 0;

		p_priv->for_scan = 0;
		para->channel_set_info.for_scan = 0;
		port_m88cs8800_channel_set(p_priv, para);

#if 0
		printk(KERN_ERR "lock_timeout_ms :%d\n", para->channel_set_info.lock_time);
		for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
		{
			mt_fe_dmd_get_lock_state_c_b_cs8800(cs8800_handle, &status);
			para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
			if (para->channel_info.lock)
			{
				printk("%s[%d]: DVB-C Locked!\n", __FUNCTION__, __LINE__);

#if 0
				printk("%s() %d: dump DVB-C demod registers start.\n", __FUNCTION__, __LINE__);
				for (cnt = 0; cnt < 0x100; cnt++)
				{
					_mt_fe_dmd_get_reg_c_cs8800(cs8800_handle, (U8)cnt, &tmp);
					printk("%s() %d: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
				}
				printk("%s() %d: dump DVB-C demod registers end.\n", __FUNCTION__, __LINE__);
#endif
				return MT_SUCCESS;
			}
			//_mt_sleep_cs8800(10);
			msleep(10);
		}

#if 0
		port_m88cs8800_get_dbg_info(cs8800_handle);

		if (cs8800_handle->m_device_c_b.demod_current_type != MtFeType_Undef)
		{
			if (cs8800_handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800)
			{
				U32 info1 = 2, info2 = 0;

				cs8800_handle->m_device_c_b.tuner_cfg.tuner_get_diagnose_info(cs8800_handle, &info1, &info2);
			}
		}

#elif 0
		printk("%s[%d]: dump DVB-C demod registers start.\n", __FUNCTION__, __LINE__);
		for (cnt = 0; cnt < 0x100; cnt++)
		{
			_mt_fe_dmd_get_reg_c_cs8800(cs8800_handle, (U8)cnt, &tmp);
			printk("%s[%d]: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
		}
		printk("%s[%d]: dump DVB-C demod registers end.\n", __FUNCTION__, __LINE__);

		printk("%s[%d]: dump DVB-C tuner registers start.\n", __FUNCTION__, __LINE__);
		for (cnt = 0; cnt < 0x100; cnt++)
		{
			_mt_fe_tn_get_reg_c_b_cs8800(cs8800_handle, (U8)cnt, &tmp);
			printk("%s[%d]: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
		}
		printk("%s[%d]: dump DVB-C tuner registers end.\n", __FUNCTION__, __LINE__);

#endif

		//return ERR_TIMEOUT;
#endif

		return MT_SUCCESS;
	}
	else if (para->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
	{
		dvb_type = MtFeType_J83B;

        /*fix issue27868 After J83B lock,DVBS can not lock anymore
        * RootCause:CS8800 demod J83B and DVBS can not work in the same time.
        * After J83B lock,DVBS firmware will lost.So when DVBS lock after J83B,DVBS firmware need be download again
        */
        if ((cs8800_handle->m_device_ss2.demod_current_type == MtFeType_DVBS) || 
		    (cs8800_handle->m_device_ss2.demod_current_type == MtFeType_DVBS2) || 
		    (cs8800_handle->m_device_ss2.demod_current_type == MtFeType_DVBS2X)||
		    (cs8800_handle->m_device_ss2.demod_current_type == MtFeType_DVBS_S2))
        {
            cs8800_handle->m_device_ss2.demod_current_type = MtFeType_Undef;
        }
        
		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
		para->channel_info.lock = 0;

		p_priv->for_scan = 0;
		para->channel_set_info.for_scan = 0;
		port_m88cs8800_channel_set(p_priv, para);

#if 0
		//printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
		for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
		{
			mt_fe_dmd_get_lock_state_c_b_cs8800(cs8800_handle, &status);
			para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
			if (para->channel_info.lock)
			{
				printk("%s() %d: J83B Locked!\n");
				return MT_SUCCESS;
			}
			//_mt_sleep_cs8800(10);
			msleep(10);
		}

		return ERR_TIMEOUT;
#endif

		return MT_SUCCESS;
	}

	return MT_FAILURE;
}



static void port_m88cs8800_get_dbg_info(MT_FE_CS8800_Device_Handle handle)
{
	U16 i = 0,j = 0;
	U8 reg_data = 0;
	//U8 tc6800_rd_addr = 0x0;
	U8 reg_addr[256] = {0};
	//U8 p_buf[2] = {0, 0};

	printk("\n[%d] dump demod registers 00 ~ ff (in hex), c_b_type = %d, ss2_type = %d\n", __LINE__, 
		   handle->m_device_c_b.demod_current_type, 
		   handle->m_device_ss2.demod_current_type);

	if (handle->m_device_c_b.demod_current_type != MtFeType_Undef)
	{
		if (MtFeType_DVBC == handle->m_device_c_b.demod_current_type)
		{
			for (i = 0; i <= 0xff; i++)
			{
				_mt_fe_dmd_get_reg_c_cs8800(handle, (U8)i, &reg_data);
				printk(KERN_CONT"%02x - %02x ", i, reg_data);
				if ((i + 1) % 8 == 0)
					printk(KERN_CONT"\n");
			}
		}
		else if (MtFeType_J83B == handle->m_device_c_b.demod_current_type)
		{
			for (i = 0x500; i < 0x600; i++)
			{
				_mt_fe_dmd_get_reg_b_cs8800(handle, i, &reg_data);
				printk(KERN_CONT"%03x - %02x ", i, reg_data);
				if ((i + 1) % 8 == 0)
					printk(KERN_CONT"\n");
			}
		}

#if 0
		else if (MtFeTN_TC6800 == handle->m_device_c_b.tuner_cfg.tuner_type)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			for (i = 0x00; i <= 0xff; i++)
			{
				reg_addr[i] = i;
				mt_fe_tn_read_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, &reg_addr[i], 1, &reg_data, 1);
				printk("0x%02x\t0x%02x\n", i, reg_data);
				//if ((i + 1) % 8 == 0)
				//	printk("\n");
			}

			//printk("\n[%d], dump tuner extended registers...\n", __LINE__);

			//printk("\n[%d], dump 0x1a extended registers...\n", __LINE__);

			for (i = 0x00; i <= 0x05; i++)
			{
				p_buf[0] = 0x1a;
				p_buf[1] = i;
				mt_fe_tn_write_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, p_buf, 2);
				tc6800_rd_addr = 0x1b;
				mt_fe_tn_read_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
				printk("0x1a%02x\t0x%02x\n", i, reg_data);
			}

			//printk("\n[%d], dump 0x39 extended registers...\n", __LINE__);

			for (i = 0x00; i <= 0x7c; i++)
			{
				p_buf[0] = 0x39;
				p_buf[1] = i;
				mt_fe_tn_write_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, p_buf, 2);
				tc6800_rd_addr = 0x3a;
				mt_fe_tn_read_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
				printk("0x39%02x\t0x%02x\n", i, reg_data);
			}

			//printk("\n[%d], dump 0x4e extended registers...\n", __LINE__);

			for (i = 0x00; i <= 0x32; i++)
			{
				p_buf[0] = 0x4e;
				p_buf[1] = i;
				mt_fe_tn_write_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, p_buf, 2);
				tc6800_rd_addr = 0x4f;
				mt_fe_tn_read_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
				printk("0x4e%02x\t0x%02x\n", i, reg_data);
			}

			//printk("\n[%d], dump 0x53 extended registers...\n", __LINE__);

			for (i = 0x0; i <= 0x18; i++)
			{
				p_buf[0] = 0x53;
				p_buf[1] = i;
				mt_fe_tn_write_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, p_buf, 2);
				tc6800_rd_addr = 0x54;
				mt_fe_tn_read_ct8k(handle->m_device_c_b.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
				printk("0x53%02x\t0x%02x\n", i, reg_data);
			}

			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
#endif

		printk("\n\n");
	}

	if (handle->m_device_ss2.demod_current_type != MtFeType_Undef)
	{
		for (i = 0; i <= 0xff; i++)
		{
			_mt_fe_dmd_get_reg_ss2_cs8800(handle, i, &reg_data);
			printk(KERN_CONT"%02x - %02x ", i, reg_data);
			if ((i + 1) % 8 == 0)
				printk(KERN_CONT"\n");
		}
		for(j=0;j<6;j++)
		{
			_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0xfa, &reg_data);
			reg_data = j;
			_mt_fe_dmd_set_reg_ss2_cs8800(handle, 0xfa, reg_data);
			_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0xfa, &reg_data);
			printk(KERN_CONT"0xfa - %02x ", reg_data);
			for(i=0xf0;i<=0xf6;i++)
			{
				_mt_fe_dmd_get_reg_ss2_cs8800(handle, i, &reg_data);
				printk(KERN_CONT"%02x - %02x ", i, reg_data);
				
			}
			printk(KERN_CONT"\n");
		}
		if (MtFeTn_TS6011 == handle->m_device_ss2.tuner_cfg.tuner_type)
		{
                        printk("\n[%d] dump TS6011 registers 00 ~ ff (in hex)\n", __LINE__);
			for (i = 0; i <= 0xff; i++)
			{
				reg_addr[i] = i;
				_mt_fe_tn_get_reg_ss2_cs8800(handle, reg_addr[i], &reg_data);
				printk(KERN_CONT"%02x - %02x ", i, reg_data);
				if ((i + 1) % 8 == 0)
					printk(KERN_CONT"\n");
			}
		}

		printk("\n\n");
	}
}
static int port_m88cs8800_set_lowpower(void *handle,mt_unf_fe_sig_type_t sig_type)
{
    mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
    mt_unf_fe_status_t status;
    MT_FE_RET ret = MtFeErr_Undef;

    
    port_m88cs8800_get_status(handle,&status);
    if(status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
    {
        printk("[Err %s %d]Signal Type:0x%x is locking.Do not allow access to low power!!\n",__FUNCTION__,__LINE__,p_priv->sig_type);
        ret = MtFeErr_Fail;
        goto END;
    }
    else
    {
        if(sig_type != p_priv->sig_type)
        {
            printk("[%s %d]sig_type:0x%x is no match p_priv->sig_type:0x%x\n",__FUNCTION__,__LINE__,sig_type,p_priv->sig_type);
            sig_type = p_priv->sig_type;
        }
        if (sig_type == MT_UNF_FE_SIG_TYPE_CAB)
    	{
            
            if(cs8800_handle->bDVBCInitOk == TRUE)
            {
                mt_analog_disable(MT_ANA_CADC);
                mt_analog_disable(DRV0_CLK_ANA);//DVBC 
                
                cs8800_handle->bDVBCInitOk = FALSE;
                cs8800_handle->bJ83BInitOk = FALSE;
                printk(KERN_ERR "[%s %d]MT_ANA_CADC and DRV0_CLK_ANA disable \n", __FUNCTION__, __LINE__);
                ret = MtFeErr_Ok;
            }
            
        }
        if(sig_type == MT_UNF_FE_SIG_TYPE_J83B)
        {
            if(cs8800_handle->bJ83BInitOk == TRUE)
            {
                mt_analog_disable(MT_ANA_CADC);
                mt_analog_disable(DRV1_CLK_ANA);//DVBS J83B
                cs8800_handle->bJ83BInitOk = FALSE;
                cs8800_handle->bDVBCInitOk = FALSE;
                cs8800_handle->bDVBSInitOk = FALSE;
                printk(KERN_ERR "[%s %d]MT_ANA_CADC and DRV1_CLK_ANA disable \n", __FUNCTION__, __LINE__);
                ret = MtFeErr_Ok;
            }
        }
        if ((sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
    		(sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
    		(sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
    		(sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
        {
			/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
            if(1 == port_m88cs8800_get_blindscan_status(cs8800_handle))
            {
                printk("[Err %s %d]Doing Blind scan now.Do not allow access to low power!!\n",__FUNCTION__,__LINE__);
                ret = MtFeErr_Fail;
                goto END;
            }
            if(cs8800_handle->bDVBSInitOk == TRUE)
            {
                mt_analog_disable(MT_ANA_SADC);
    			mt_analog_disable(DRV1_CLK_ANA);//DVBS
                cs8800_handle->bDVBSInitOk = FALSE;
                cs8800_handle->bJ83BInitOk = FALSE;
                printk(KERN_ERR "[%s %d]MT_ANA_SADC and DRV1_CLK_ANA disable \n", __FUNCTION__, __LINE__);
                ret = MtFeErr_Ok;
            }
        }        
    }
    print_analog_status(__LINE__);
END:
    return ret;
}
static int port_m88cs8800_set_bbframe_packing_type(void *handle,mt_unf_fe_bbframe_packing_type_t bbframe_packing_type)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
    mt_unf_fe_status_t status;
    MT_FE_RET ret = MtFeErr_Undef;
	U8  val_reg1f4;

	port_m88cs8800_get_status(handle,&status);
    if(status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
    {
		if(bbframe_packing_type == PACKING_BBFRAME_INCLUDE_PADDING)
		{
			/*Set bbframe include padding*/
			_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xfa, 0x01);

			_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xf4, &val_reg1f4);
		    /*Clean bit5*/
		    val_reg1f4 &= 0xDF;
			/*Set bit5 to 1*/
			val_reg1f4 |= 0x20;
			_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xf4, val_reg1f4);

		}
		else
		{
			/*Set bbframe include padding*/
			_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xfa, 0x01);

			_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xf4, &val_reg1f4);
		    /*Clean bit5*/
		    val_reg1f4 &= 0xDF;

			_mt_fe_dmd_set_reg_ss2_cs8800(cs8800_handle, 0xf4, val_reg1f4);

		}
		_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xfa, &val_reg1f4);
		printk("0xfa ================= 0x%x\n", val_reg1f4);

		_mt_fe_dmd_get_reg_ss2_cs8800(cs8800_handle, 0xf4, &val_reg1f4);
		printk("0xf4 = 0x%x\n", val_reg1f4);
		ret = MtFeErr_Ok;
    }
    else
    {
        pr_err("[Err %s %d]Signal unlock set bbframe packing type falied!!\n",__FUNCTION__,__LINE__);
        ret = MtFeErr_Fail;
	}
	return ret;
}
static int port_m88cs8800_ioctl(void *handle, mt_u32 cmd, ulong param)
{
	MT_FE_LOCK_STATE status = 0;
	mt_unf_fe_diseqc_cmd_t diseqc_cmd = {0};
	//mt_unf_fe_diseqc_cmd_t *p_diseqc_cmd = NULL;
	//mt_unf_fe_scan_info_t *p_scan_info = NULL;
	mt_unf_fe_diseqc_sendmsg_t *p_sendmsg;
	mt_unf_fe_diseqc_recvmsg_t *p_recvmsg;
	fe_blindscan_param_t *p_scan_info = NULL;
	//U8 tx_buf[8] = {0};
	U8 voltage = 0;
	//U8 pin = 0;
	//U8 level = 0;
	//U8 value = 0;
	//int rc = 0;
	S32 tuner_offset_KHZ = 0, carrieroffset_KHz = 0;
	U32 sym_rate_KSs = 27500;
	U32 pre_ber[3];
	mt_u32 ts_gs_mode = 0;
	mt_unf_fe_dss_scid_filter_t *scid_filter;

	MT_FE_RET ret = MtFeErr_Undef;

	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
	//pinmux_configure();

	//printk("%s() %d: cmd %d - param 0x%08x\n", __FUNCTION__, __LINE__, cmd, param);

	switch (cmd)
	{
	case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
#if 1
		mt_fe_dmd_get_lock_state_c_b_cs8800(cs8800_handle, &status);
#else
		mt_fe_dmd_get_lock_state_ss2_cs8800(cs8800_handle, &status);
#endif
		if (param != 0)
			((mt_unf_fe_channel_info_t *)param)->lock = (status == MtFeLockState_Locked) ? 1 : 0;
		break;

	case NIM_IOCTRL_DISEQC1X:
		p_priv->diseqc_2x = 0;
		break;

	case NIM_IOCTRL_DISEQC2X:
		p_priv->diseqc_2x = 1;
		break;

	case NIM_IOCTRL_SET_PORLAR:
		if (p_priv->lnb_onoff == 0)
		{
			break;
		}

		if (p_priv->lnb_polar == param)
		{
			break;
		}

		voltage = (param == PORT_PORLAR_HORIZONTAL) ? MtFeLNB_18V : MtFeLNB_13V;
		port_m88cs8800_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
		p_priv->lnb_polar = (MT_U8)param;
		p_priv->lnb_voltage = voltage;
		break;

	case NIM_IOCTRL_SET_LNB_ONOFF:
		//printk("ker cs8800 line:%d param=0x%08x\n", __LINE__, param);

#if 0
	if (p_priv->lnb_onoff == param)
	{
		break;
	}
#endif

		//printk("NIM_IOCTRL_SET_LNB_ONOFF set %d\n", param);
		port_m88cs8800_set_lnb_onoff(handle, (MT_U8)param, &p_priv->cfg.pin_config);
		p_priv->lnb_onoff = param;
		if (param == 0)
		{
			break;
		}

		/* restore voltage */
		port_m88cs8800_set_lnb_voltage(handle, p_priv->lnb_voltage, p_priv->cfg.pin_config.vsel_when_13v);
		/* restore 22k */
		//if (p_priv->onoff_22k != 0)		// 181219
		{
			port_m88cs8800_set_22k_onoff(handle, p_priv->onoff_22k, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
		}

		if (0 != p_priv->cur_diseqc.tx_len)
		{
			/* restore previte diseqc */
			//diseqc_cmd.p_tx_buf = tx_buf;
			memcpy(diseqc_cmd.p_tx_buf, p_priv->cur_diseqc.p_tx_buf, p_priv->cur_diseqc.tx_len);
			diseqc_cmd.mode = p_priv->cur_diseqc.mode;
			diseqc_cmd.result = p_priv->cur_diseqc.result;
			diseqc_cmd.tx_len = p_priv->cur_diseqc.tx_len;
			diseqc_cmd.rx_len = p_priv->cur_diseqc.rx_len;
			//port_m88cs8800_diseqc_ctrl(handle, &diseqc_cmd);
		}
		break;

#if 0
	case NIM_IOCTRL_CHECK_LNB_SC_PROT:
		if (p_priv->cfg.lnb_prot_by_mcu == 1)
		{
			pin = p_priv->cfg.lnb_prot_pin;
			level = p_priv->cfg.lnb_prot_level;
			//hal_pinmux_gpio_enable(pin, TRUE);
			//gpio_ioctl(GPIO_CMD_IO_ENABLE, pin, TRUE);
			gpio_get_dir(pin,&value);

			if (GPIO_DIR_OUTPUT == value)
			{
				gpio_set_dir(pin, GPIO_DIR_INPUT);
			}

			gpio_get_value(pin,&value);
			if (value == level)
			{
				*((MT_U8 *)param) = PORT_LNB_SC_PROTING;
			}
			else
			{
				*((MT_U8 *)param) = PORT_LNB_SC_NO_PROTING;
			}
		}
		break;

	case NIM_IOCTRL_LNB_SC_PROT_RESTORE:
		if (1 == p_priv->cfg.lnb_prot_by_mcu)
		{
			//port_cs8800_lnb_sc_restore(handle, &p_priv->cfg.pin_config);
		}
		break;

	case NIM_IOCTRL_REMOVE_PROTECT:
		if (1 == p_priv->cfg.lnb_prot_by_mcu)
		{
			//port_cs8800_lnb_sc_remove(&p_priv->cfg.pin_config);
		}
		break;

	case NIM_IOCTRL_ENABLE_CHECK_PROTECT:
		if (1 == p_priv->cfg.lnb_prot_by_mcu)
		{
			//port_cs8800_lnb_sc_chk_enable(&p_priv->cfg.pin_config);
		}
		break;
#endif

	case NIM_IOCTRL_SET_22K_ONOFF:
		//printk("ker cs8800 ioctl line:%d param=0x%08x\n", __LINE__, param);
		if (p_priv->onoff_22k == param)
		{
			break;
		}
		port_m88cs8800_set_22k_onoff(handle, (MT_U8)param, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
		p_priv->onoff_22k = (u8)param;
		break;

	case NIM_IOCTRL_GET_PORLAR:
		if (param != 0)
			*((MT_U8 *)param) = p_priv->lnb_polar;
		break;

	case NIM_IOCTRL_GET_22K_ONOFF:
		if (param != 0)
			*((MT_U8 *)param) = p_priv->onoff_22k;
		break;

	case NIM_IOCTRL_GET_TN_VERSION:
		//*((MT_U8 *)param) = p_priv->cfg.tun_support;
		break;

#if 0
	case NIM_IOCTRL_RECOVER:
		//port_cs8800_recover(handle);
		break;

	case NIM_IOCTRL_SET_CHANNEL_INFO:
		//rc = port_cs8800_set_chninfo(handle, param);
		return rc;
		break;
#endif

	case NIM_IOCTRL_GET_SIGNAL_INFO:
		port_m88cs8800_get_signal_info(handle, (mt_unf_fe_signal_info_t *)param);
		break;

#if 0
	case NIM_IOCTRL_GET_CHANNEL_INFO:
		if (param != 0)
			memcpy((MT_UNF_FE_CHANNEL_INFO_S*)param, &p_priv->cur_channel, sizeof(MT_UNF_FE_CHANNEL_INFO_S));
		break;
#endif

	case NIM_IOCTRL_SCAN_CANCEL:
		port_m88cs8800_blind_scan_cancel(p_priv);
		break;

	case NIM_IOCTRL_GET_SCAN_STATUS:
		//mutex_lock(&bs_notify_status_lock);
		if (param != 0)
			*((MT_U8 *)param) = cs8800_notify_scan_status;
		//mutex_unlock(&bs_notify_status_lock);
		break;

	case NIM_IOCTRL_GET_SCAN_RESULT:
		p_scan_info = (fe_blindscan_param_t *)param;
		if (p_priv->scan_info.channel_num_total && param != 0)
		{
			memcpy((fe_blindscan_param_t *)param, &p_priv->scan_info, sizeof(fe_blindscan_param_t));
		}
		break;

	case NIM_IOCTRL_DISEQC_SENDMSG:
		p_sendmsg = (mt_unf_fe_diseqc_sendmsg_t *)param;
		port_m88cs8800_diseqc_sendmsg(handle, p_sendmsg);
		break;

	case NIM_IOCTRL_DISEQC_SEND_TONEBURST:
		//p_diseqc_cmd = (mt_unf_fe_diseqc_sendmsg_t*)param;
		port_m88cs8800_diseqc_send_tone_burst(handle, param);
		break;

	case NIM_IOCTRL_DISEQC_RECVMSG:
		p_recvmsg = (mt_unf_fe_diseqc_recvmsg_t *)param;
		ret = port_m88cs8800_diseqc_recvmsg(handle, p_recvmsg);
		if (MtFeErr_Ok != ret)
		{
			printk("status: %d\n", p_recvmsg->status);
			return MT_FAILURE;
		}
		break;
		//#endif

	case NIM_IOCTRL_SAT_BS_EVENT_PROCESSED:
#ifdef CFG_SYNC_BLINDSCAN
		{
			event_status = 1;
			wake_up_interruptible(&bs_cb_wq);
		}
#endif
		break;

	case NIM_IOCTRL_GET_SAT_REAL_FREQ:
		if (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
		{
			_mt_fe_dmd_get_carrier_offset_cs8800_ss2(cs8800_handle, &carrieroffset_KHz);

			cs8800_handle->m_device_ss2.tuner_cfg.tuner_get_offset(cs8800_handle, &tuner_offset_KHZ);
		}

		if (param != 0)
			*((MT_U32 *)param) = cs8800_handle->m_device_ss2.tp_cfg.iFreqKHz + tuner_offset_KHZ - carrieroffset_KHz;

		break;

	case NIM_IOCTRL_GET_SAT_REAL_SYM:
		if (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
		{
			_mt_fe_dmd_get_sym_rate_cs8800_ss2(cs8800_handle, &sym_rate_KSs);
		}
		else
		{
			sym_rate_KSs = 0;
		}

		if (param != 0)
			*((MT_U32 *)param) = sym_rate_KSs;
		break;

	case NIM_IOCTRL_GET_SAT_FREQ_OFFSET:
		if (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
		{
			_mt_fe_dmd_get_carrier_offset_cs8800_ss2(cs8800_handle, &carrieroffset_KHz);

			cs8800_handle->m_device_ss2.tuner_cfg.tuner_get_offset(cs8800_handle, &tuner_offset_KHZ);
		}

		if (param != 0)
			*((MT_S32 *)param) = tuner_offset_KHZ - carrieroffset_KHz;
		break;

	case NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_CNT:
		if (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
		{
			cs8800_handle->m_device_ss2.tp_cfg.bSuperSearch = g_bSuperSearch;
			mt_fe_dmd_get_lock_state_cs8800_ss2(cs8800_handle, &status);

			if (status == MtFeLockState_Locked)
			{
				if (param != 0)
					*((MT_U8 *)param) = cs8800_handle->m_device_ss2.tp_cfg.iTsCnt;
			}
			else
			{
				if (param != 0)
					*((MT_U8 *)param) = 0;
			}
		}
		else
		{
			if (param != 0)
				*((MT_U8 *)param) = 0;
		}
		break;

	case NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_LIST:
		if (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
		{
			int iCnt = 0;
			cs8800_handle->m_device_ss2.tp_cfg.bSuperSearch = g_bSuperSearch;
			mt_fe_dmd_get_lock_state_cs8800_ss2(cs8800_handle, &status);

			if (status == MtFeLockState_Locked)
			{
				for (iCnt = 0; iCnt < cs8800_handle->m_device_ss2.tp_cfg.iTsCnt; iCnt++)
				{
					((MT_U8 *)param)[iCnt] = cs8800_handle->m_device_ss2.tp_cfg.ucTsId[iCnt];
				}
			}
		}
		break;

	case NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID:
		if (cs8800_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
		{
			mt_fe_dmd_set_ts_cs8800_ss2(cs8800_handle, (U8)param);
			//printk("%s[%d] ---- NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID: ts_id = %d\n", __FUNCTION__, __LINE__, param);
		}
		break;

	case NIM_IOCTRL_TEST_FOR_GSE:
		mt_fe_dmd_cs8800_gse_mode_for_test(cs8800_handle, param);
		break;

	case NIM_IOCTRL_GET_TN_DBG_INFO:
		port_m88cs8800_get_dbg_info(cs8800_handle);

		if (cs8800_handle->m_device_c_b.demod_current_type != MtFeType_Undef)
		{
			if ((cs8800_handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800)
				&& (cs8800_handle->m_device_c_b.tuner_cfg.tuner_get_diagnose_info))
			{
				U32 info2 = 0;

				cs8800_handle->m_device_c_b.tuner_cfg.tuner_get_diagnose_info(cs8800_handle, (MT_U32 *)param, &info2);
			}
		}
		break;
       case NIM_IOCTRL_SET_TN_GLOBAL_RESET:
	   	port_m88cs8800_get_dbg_info(cs8800_handle);

		if ((cs8800_handle) && (cs8800_handle->m_device_c_b.demod_current_type != MtFeType_Undef))
		{
			if ((cs8800_handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800) 
				&& (cs8800_handle->m_device_c_b.tuner_cfg.tuner_set_application))
			{
				cs8800_handle->m_device_c_b.tuner_cfg.tuner_set_application(cs8800_handle, (MT_U8)(param & 0xff));
			}
		}
		break;
	case NIM_IOCTRL_SET_TN_PARAM:
#if 0
		printk("%s[%d] ---- Set tuner param: sig_type[%d], freq[%d KHz], symbol_rate[%d KSs]\n", __FUNCTION__, __LINE__, 
				(*(fe_tn_param_t *)param).sig_type, 
				(*(fe_tn_param_t *)param).freq_KHz, 
				(*(fe_tn_param_t *)param).sym_rate_KSs
			  );
#endif

		if (((*(fe_tn_param_t *)param).sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
			((*(fe_tn_param_t *)param).sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
			((*(fe_tn_param_t *)param).sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
			((*(fe_tn_param_t *)param).sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
		{
			if (cs8800_handle->m_device_ss2.tuner_cfg.tuner_set)
			{
				cs8800_handle->m_device_ss2.tuner_cfg.tuner_set(cs8800_handle, (*(fe_tn_param_t *)param).freq_KHz, (*(fe_tn_param_t *)param).sym_rate_KSs, 0);
			}
		}
		else
		{
			if (cs8800_handle->m_device_c_b.tuner_cfg.tuner_set)
			{
				cs8800_handle->m_device_c_b.tuner_cfg.tuner_set(cs8800_handle, (*(fe_tn_param_t *)param).freq_KHz, (*(fe_tn_param_t *)param).sym_rate_KSs, 0);
			}
		}
		break;

	case NIM_IOCTRL_GET_PRE_BER:
		port_m88cs8800_get_pre_ber(p_priv, pre_ber);
		//*((MT_U32 *)param) = pre_ber;
		((MT_U32 *)param)[0] = pre_ber[0];
		((MT_U32 *)param)[1] = pre_ber[1];
		((MT_U32 *)param)[2] = pre_ber[2];
		//printk("%s[%d] ---- param[0] = [%6d], param[1] = [%6d], param[2] = [%6d]\n", __FUNCTION__, __LINE__, *((MT_U32 *)param)[0], *((MT_U32 *)param)[1], *((MT_U32 *)param)[2]);
		break;

	case NIM_IOCTRL_SET_SUPER_SEARCH:
		cs8800_handle->m_device_ss2.tp_cfg.bSuperSearch = (param != 0) ? TRUE : FALSE;
		g_bSuperSearch = cs8800_handle->m_device_ss2.tp_cfg.bSuperSearch;
		printk("%s[%d] ---- Switch super search or blindscan mode -- %s\n", __FUNCTION__, __LINE__, (param != 0) ? "ON" : "OFF");
		break;

	case NIM_IOCTRL_UNICABLE_RETRY:
		mt_fe_unicable_retry_cs8800_ss2(cs8800_handle);
		break;

	case NIM_IOCTRL_SUSPEND:
		m88cs8800_suspend();
		break;

	case NIM_IOCTRL_RESUME:
		m88cs8800_resume();
		break;

	case NIM_IOCTRL_GET_TS_GSE_MODE:
		mt_fe_dmd_cs8800_get_ts_gs_mode(cs8800_handle, &ts_gs_mode);
		*((MT_U32 *)param) = ts_gs_mode;
		break;
	case NIM_IOCTRL_SET_GS_PACKAGE_MODE:
		mt_fe_dmd_cs8800_set_gs_package_mode(cs8800_handle, param);
		break;
	case NIM_IOCTRL_REGISTER_NLK_FAMILY:
		ret = port_m88cs8800_register_netlink_family();
		if (MT_SUCCESS != ret)
		{
			printk("REGISTER_NLK_FAMILY failed: %d\n", ret);
			return MT_FAILURE;
		}
		break;
    case NIM_IOCTRL_SET_LOWPOWER:
        /*Task 27399 Lowpower optimize*/
        ret = port_m88cs8800_set_lowpower(handle,param);
		if (MtFeErr_Ok != ret)
		{
			printk("NIM_IOCTRL_SET_LOWPOWER failed: %d\n", ret);
			return MT_FAILURE;
		}
        break;
    case NIM_IOCTRL_GET_ACCURATE_SNR:
        ret = port_m88cs8800_get_accurate_snr(handle,(MT_S32 *)param);
        break;
  case NIM_IOCTRL_DSS_SCID_FILTER:
	    scid_filter = (mt_unf_fe_dss_scid_filter_t *)param;
	    m88cs8800_set_scid_filter(scid_filter);
	    break;

	case NIM_IOCTRL_SET_BBFRAME_PADDING_ONOFF:
        ret = port_m88cs8800_set_bbframe_packing_type(handle,param);
		if (MtFeErr_Ok != ret)
		{
			printk("NIM_IOCTRL_SET_BBFRAME_PADDING_ONOFF failed: %d\n", ret);
			return MT_FAILURE;
		}
        break;
	case NIM_IOCTRL_GET_FAST_LOCK:
		ret = port_m88cs8800_get_fast_lock(handle,(mt_unf_fe_status_t *)param);
		break;
	default:
		break;
	}

	return MT_SUCCESS;
}
static int port_m88cs8800_blind_scan_cancel_new(void *handle,MT_U32 delay_ms,MT_U32 timeout_cnt)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;
       pid_t thread_pid = 0;
	MT_U8 count = 0;
       
	printk("port_m88cs8800_blind_scan_cancel_new\n");
       if (delay_ms == 0) delay_ms = 2;
   
      if (timeout_cnt == 0) timeout_cnt = 100;
    
	mt_fe_dmd_blindscan_abort_ss2_cs8800(cs8800_handle, MtFe_True);
    
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 42)
	//wait blind scan to stop(bBsStatus:0:not start or has finished, 1:runnig)
	/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
	while (1 == port_m88cs8800_get_blindscan_status(cs8800_handle))
	{
		cs8800_handle->m_device_ss2.global_cfg.bCancelBs = TRUE;
		msleep(delay_ms);
		count++;
		if (count > timeout_cnt)
		{
			break;
		}
	}
#else
     if(p_priv->bl_thread)
      {
        thread_pid = p_priv->bl_thread->pid;
        printk("[%s %d]kthread_%d will stop!\n",__FUNCTION__,__LINE__,thread_pid);
        kthread_stop(p_priv->bl_thread);
        printk("[%s %d]kthread_%d stop success!\n",__FUNCTION__,__LINE__,thread_pid);
		/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
		//cs8800_handle->m_device_ss2.global_cfg.bBsStatus = 0;
		port_m88cs8800_set_blindscan_status(cs8800_handle, 0);       
      }
      else
      {
        //wait blind scan to stop(bBsStatus:0:not start or has finished, 1:runnig)
        /*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
          while (1 == port_m88cs8800_get_blindscan_status(cs8800_handle))
          {
            cs8800_handle->m_device_ss2.global_cfg.bCancelBs = TRUE;
            msleep(delay_ms);
            count++;
            if (count > timeout_cnt)
            {
              break;
            }
          } 
      }    
#endif
	event_status = 1;
	wake_up_interruptible(&bs_cb_wq);
#endif

	return MT_SUCCESS;
}
static int port_m88cs8800_blind_scan_cancel(void *handle)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	MT_U8 count = 0;

	printk("port_m88cs8800_blind_scan_cancel\n");
	mt_fe_dmd_blindscan_abort_ss2_cs8800(cs8800_handle, MtFe_True);

	//wait blind scan to stop(bBsStatus:0:not start or has finished, 1:runnig)
	/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
	while (1 == port_m88cs8800_get_blindscan_status(cs8800_handle))
	{
		cs8800_handle->m_device_ss2.global_cfg.bCancelBs = TRUE;
		msleep(2);
		count++;
		if (count > 100)
		{
			break;
		}
	}

	event_status = 1;
	wake_up_interruptible(&bs_cb_wq);
#endif

	return MT_SUCCESS;
}

static const struct nla_policy cs8800_policy[CS8800_NL_FAMILY_ATTR_AMOUNT] = {
	[CS8800_NL_ATTR_DATA0] = {.type = NLA_BINARY, .len = sizeof(struct cs8800_netlink_data)},
};

static int cs8800_nl_recvmsg0(struct sk_buff *skb, struct genl_info *info)
{
	struct cs8800_netlink_data *cs8800_nl_data;
	struct cs8800_netlink_pid_list *cs8800_nl_pid;
	int err = 0;

	//printk("%s\n", __FUNCTION__);

	if (info->attrs[CS8800_NL_ATTR_DATA0])
	{
		cs8800_nl_data = nla_data(info->attrs[CS8800_NL_ATTR_DATA0]);
		cs8800_nl_pid = kzalloc(sizeof(struct cs8800_netlink_pid_list), GFP_KERNEL);
		if (cs8800_nl_pid == NULL)
			return MtFeErr_NoMemory;
		cs8800_nl_pid->net = sock_net(skb->sk);
		/* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
		cs8800_nl_pid->cs8800_nl_data = *cs8800_nl_data;
		//printk("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
		//printk("netlink : pid = %d\n", cs8800_nl_data->pid);
		//printk("netlink : seq = 0x%x\n", cs8800_nl_data->seq);
		//printk("netlink : cmd = %d\n", cs8800_nl_data->cmd);
		//printk("netlink : data01 = %d\n", cs8800_nl_data->data01);
		//printk("netlink : name : %s, freq = %u, sym = %u\n", cs8800_nl_data->name, cs8800_nl_data->freq_khz, cs8800_nl_data->symbol_rate);

		if (mutex_lock_interruptible(&cs8800_nl_mutex))
		{
			kfree(cs8800_nl_pid);
			return -ETIME;
		}

		list_add_tail(&(cs8800_nl_pid->list), &cs8800_nl_pid_list);
		mutex_unlock(&cs8800_nl_mutex);
	}

	return err;
}

static struct genl_ops cs8800_nl_ops[CS8800_NL_OPS_AMOUNT] = {
	[CS8800_NL_OPS_CMD0] = {
		.cmd = CS8800_NL_OPS_CMD0,
		.doit = cs8800_nl_recvmsg0,
		.policy = cs8800_policy,
	},
};

int port_m88cs8800_register_netlink_family(void)
{
	S32 ret = MT_SUCCESS;
	if (dvbs_nl_family.id > 0)
	{
		ret = genl_unregister_family(&dvbs_nl_family);
	}

	ret = genl_register_family(&cs8800_nl_family);
	if (ret)
	{
		printk("genl_register_family failed, ret = 0x%08x\n", ret);
		return MT_FAILURE;
	}

	memcpy(&dvbs_nl_family, &cs8800_nl_family, sizeof(cs8800_nl_family));
	/*printk("[%s %d] cs8800_nl_family.id :%d \n", __FUNCTION__, __LINE__, cs8800_nl_family.id);*/

	return ret;
}

static int port_m88cs8800_blind_scan_start(void *pArg)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
	S32 ret = 0;
	U32 i = 0;
	MT_FE_CS8800_Device_Handle cs8800_handle = g_cs8800_handle;
	fe_blindscan_param_t *p_scan_info = (fe_blindscan_param_t *)pArg;

	cs8800_handle = (MT_FE_CS8800_Device_Handle)g_cs8800_priv->cs8800_handle;

#if 0
	printk("ker cs8800: start-stop - start freq: %d, end_freq: %d\n",
		   p_scan_info->start_freq, p_scan_info->stop_freq);
	printk("ker cs8800: use_uc[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
		   p_scan_info->uc_param.use_uc,
		   p_scan_info->uc_param.bank,
		   p_scan_info->uc_param.user_band,
		   p_scan_info->uc_param.ub_freq_mhz);
#endif
    /*Fix issue 23875
    * Because the unregister and register actions will cause the value of the family id to change.
    * So provide a new API to deregister and register.
    * Avoid inconsistent API and DRV family ids
    */

	cs8800_handle->m_device_ss2.lnb_cfg.bUnicable = p_scan_info->uc_param.use_uc;
	cs8800_handle->m_device_ss2.lnb_cfg.iBankIndex = p_scan_info->uc_param.bank;
	cs8800_handle->m_device_ss2.lnb_cfg.iUBIndex = p_scan_info->uc_param.user_band;
	cs8800_handle->m_device_ss2.lnb_cfg.iUBFreqMHz = p_scan_info->uc_param.ub_freq_mhz;
	cs8800_handle->m_device_ss2.lnb_cfg.iUBVer = p_scan_info->uc_param.ub_ver;
	cs8800_handle->m_device_ss2.lnb_cfg.bSpectrumInverted = p_scan_info->invert_spectrum;

	g_cs8800_priv->bs_info.tp_num = 0;

	//nim_lock(priv->drv_base);

	ret = mt_fe_dmd_blindscan_ss2_cs8800(cs8800_handle,
										 p_scan_info->start_freq / 1000,
										 p_scan_info->stop_freq / 1000,
										 &g_cs8800_priv->bs_info);

	//nim_unlock(priv->drv_base);

	//printk("nim_cs8800_service end. tp start %d end %d ret = %d\n",
	//g_cs8800_priv->scan_info.start_freq, g_cs8800_priv->scan_info.end_freq, ret);

	if (ret == MtFeErr_Ok)
	{
		p_scan_info = &g_cs8800_priv->scan_info;
		p_scan_info->channel_num_cur = g_cs8800_priv->bs_info.tp_num;
		//printk("port_m88cs8800_blind_scan get %u tp:\n", g_cs8800_priv->bs_info.tp_num);

		/* Bug 108019 */
		p_scan_info->channel_num_total = 0;

		for (i = 0; i < g_cs8800_priv->bs_info.tp_num; i++)
		{
			p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency =
				g_cs8800_priv->bs_info.p_tp_info[i].freq_KHz;
			p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate =
				g_cs8800_priv->bs_info.p_tp_info[i].sym_rate_KSs;
			p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type =
				port_m88cs8800_deparse_dvb_type(g_cs8800_priv->bs_info.p_tp_info[i].dvb_type);
			p_scan_info->p_channel_info[p_scan_info->channel_num_total].fec_inner =
				port_m88cs8800_deparse_code_rate(g_cs8800_priv->bs_info.p_tp_info[i].code_rate);
			//printk("freq: %u, symrate:%u, type:%u\n",
			//p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency,
			//p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate,
			//p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type);
			p_scan_info->channel_num_total += 1;
		}

		//if you scan by dividing all channel to several "channel group", and then scan group by group
		//you should use p_scan_info->channel_num_cur to count scanned tp num for every scan.
		printk("total channel_num_cur: %u channel_num_total:%d\n", p_scan_info->channel_num_cur, p_scan_info->channel_num_total);
		/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
		//cs8800_handle->m_device_ss2.global_cfg.bBsStatus = 0;
		port_m88cs8800_set_blindscan_status(cs8800_handle, 0);
		return MT_SUCCESS;
	}
	/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
	//cs8800_handle->m_device_ss2.global_cfg.bBsStatus = 0;
	port_m88cs8800_set_blindscan_status(cs8800_handle, 0);
#endif
	return MT_FAILURE;
}

static int port_m88cs8800_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info)
{
	mt_fe_cs8800_priv_handle p_priv = (mt_fe_cs8800_priv_handle)handle;
	MT_FE_CS8800_Device_Handle cs8800_handle = p_priv->cs8800_handle;

	/*fix issue27746 slab-use-after-free*/
	/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
    if(port_m88cs8800_get_blindscan_status(cs8800_handle) != 0)
         port_m88cs8800_blind_scan_cancel_new(p_priv,100,1000);

	//mutex_lock(&bs_notify_status_lock);
	cs8800_notify_scan_status = 0;
	//mutex_unlock(&bs_notify_status_lock);
	
	p_priv->sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;

    /*Task 27399 Lowpower optimize*/
    port_m88cs8800_check_and_enable_analog(handle,p_priv->sig_type);
    
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 42)
	kernel_thread(port_m88cs8800_blind_scan_start, p_scan_info, CLONE_KERNEL);
#else
	p_priv->bl_thread = kthread_run(port_m88cs8800_blind_scan_start, (void *)p_scan_info, "port_m88cs8800_blind_scan");
    if (IS_ERR(p_priv->bl_thread)) {
       pr_err("%s[%d]Failed to create kernel thread.\n", __FUNCTION__, __LINE__);
       return PTR_ERR(p_priv->bl_thread);
    }
#endif
	/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
	//cs8800_handle->m_device_ss2.global_cfg.bBsStatus = 1;		//start to blind scan
	port_m88cs8800_set_blindscan_status(cs8800_handle, 1);
	cs8800_handle->m_device_ss2.global_cfg.bCancelBs = FALSE;	//not cancel

	return MT_SUCCESS;
}

void port_m88cs8800_notify_to_up_layer(MT_FE_MSG msg, void *p_param)
{
	//U16 i = 0;
	mt_unf_fe_blind_scan_channel_info_t *p_bs_channel_info = NULL;

	if (p_param != NULL)
		p_bs_channel_info = (mt_unf_fe_blind_scan_channel_info_t *)p_param;

	if (msg == MtFeMsg_BSFinish || msg == MtFeMsg_BSAbort)
	{
		cs8800_notify_scan_status = 1;
		printk("cs8800_notify_scan_status = %d\n", cs8800_notify_scan_status);
	}
}

static void port_m88cs8800_register_to_drv_notify(MT_FE_MSG msg, void *p_param)
{
	MT_FE_TP_INFO *p_tp_info = p_param;
	//nim_channel_info_t channel_info;
	mt_unf_fe_blind_scan_channel_info_t bs_channel_info = {0};
	//S8 snr;
	//S8 s_strength;
	//unsigned int total_packages = 1;
	//unsigned int err_packages = 0;
	mt_unf_fe_channel_info_t channel_info = {0};
	int error = 0;
	MT_FE_BS_TP_INFO *p_bs_info;
	int i, index = 0;

	//MT_FE_CS8800_Device_Handle cs8800_handle = g_cs8800_priv->cs8800_handle;

	/*
	if (port_m88cs8800_notify_to_up_layer == NULL)
	{
		return;
	}
	*/

	//printk("cs8800_notify_function: msg ");

	switch (msg)
	{
	case MtFeMsg_BSStart:
		printk("BSTpStart\n");
		ss2_status = MtFeLockState_Unlocked;
		break;

	case MtFeMsg_BSOneWinFinish:
		printk("BSOneWinFinish: freq %d\n", *(u32 *)p_param);
		break;

	case MtFeMsg_BSTpFind:
#if 0 //def CFG_SYNC_BLINDSCAN
		init_waitqueue_head(&bs_cb_wq);

		event_status = 0;

		printk("%s[%d] -- TpFind, event_status = 0\n", __FUNCTION__, __LINE__);
#endif
		printk("BSTpFind: freq[%7d] sym[%5d]\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs);
		channel_info.frequency = p_tp_info[0].freq_KHz;
		channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
		channel_info.port_type = MT_UNF_FE_DVBS_AUTO;
		cs8800_nl_sendmsg(&channel_info);

		ss2_status = MtFeLockState_Unlocked;

		//printk("%s[%d] -- TpFind, send msg\n", __FUNCTION__, __LINE__);

#if 0 //def CFG_SYNC_BLINDSCAN
		//printk("%s() %d: prepare to wait event, wq = 0x%08x, evt_status = %d\n", __FUNCTION__, __LINE__, &bs_cb_wq, event_status);

		error = wait_event_interruptible_timeout(bs_cb_wq, (event_status == 1), 5 * HZ);
		if (error == 0)
		{
			printk("%s[%d]: blindscan wait callback timeout!\n", __FUNCTION__, __LINE__);
		}
		else if (error == -ERESTARTSYS)
		{
			printk("%s[%d]: blindscan interrupted by a signal!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			printk("%s[%d]: wait event OK, evt_status = %d, error = %d\n", __FUNCTION__, __LINE__, event_status, error);
		}
#endif
		break;

	case MtFeMsg_BSTpUnlock:
#if 0 //def CFG_SYNC_BLINDSCAN
		init_waitqueue_head(&bs_cb_wq);

		event_status = 0;
#endif
		printk("BSTpUnlock: freq[%7d] sym[%5d]\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs);
		bs_channel_info.frequency = p_tp_info[0].freq_KHz;
		bs_channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
		bs_channel_info.lock = 0;
#if 0
		mt_fe_dmd_cs8800_get_strength(cs8800_handle, &s_strength);
		bs_channel_info.perf.agc =  s_strength;
		mt_fe_dmd_cs8800_get_sat_quality(cs8800_handle, &snr);
		bs_channel_info.perf.snr = snr;
		mt_fe_dmd_cs8800_get_per(cs8800_handle, &total_packages, &err_packages);
		bs_channel_info.perf.ber = (double)err_packages/(double)total_packages;
#endif
		port_m88cs8800_notify_to_up_layer(MtFeMsg_BSTpUnlock, &bs_channel_info);

		channel_info.frequency = p_tp_info[0].freq_KHz;
		channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
		channel_info.port_type = MT_UNF_FE_BUTT;
		cs8800_nl_sendmsg(&channel_info);

		ss2_status = MtFeLockState_Unlocked;

#if 0 //def CFG_SYNC_BLINDSCAN
		//printk("%s() %d: prepare to wait event, wq = 0x%08x, evt_status = %d\n", __FUNCTION__, __LINE__, &bs_cb_wq, event_status);

		error = wait_event_interruptible_timeout(bs_cb_wq, (event_status == 1), 10 * HZ);
		if (error == 0)
		{
			printk("%s: blindscan wait callback timeout!\n", __FUNCTION__);
		}
		else if (error == -ERESTARTSYS)
		{
			printk("%s: blindscan interrupted by a signal!\n", __FUNCTION__);
		}
		else
		{
			//printk("%s() %d: wait event OK or timeout, evt_status = %d, error = %d\n", __FUNCTION__, __LINE__, event_status, error);
		}
#endif
		break;

	case MtFeMsg_BSTpLocked:
		bs_channel_info.frequency = p_tp_info[0].freq_KHz;
		bs_channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
		bs_channel_info.lock = 1;

		/*
		bs_channel_info.nim_type = nim_cs8800_deparse_dvb_type(p_tp_info[0].dvb_type);
		bs_channel_info.param.dvbs.fec_inner = nim_cs8800_deparse_code_rate(p_tp_info[0].code_rate);
		mt_fe_dmd_cs8800_get_strength(cs8800_handle, &s_strength);
		bs_channel_info.param.dvbs.perf.agc =  s_strength;
		mt_fe_dmd_cs8800_get_sat_quality(cs8800_handle, &snr);
		bs_channel_info.param.dvbs.perf.snr = snr;
		mt_fe_dmd_cs8800_get_per(cs8800_handle, &total_packages, &err_packages);
		bs_channel_info.param.dvbs.perf.ber = (double)err_packages/(double)total_packages;
		*/
		port_m88cs8800_notify_to_up_layer(MtFeMsg_BSTpLocked, &bs_channel_info);
		printk("BSTpLocked: freq[%7d] sym[%5d] %s\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs, (p_tp_info[0].dvb_type == MtFeType_DVBS2) ? "DVB-S2" : "DVB-S");

		ss2_status = MtFeLockState_Locked;

#ifdef CFG_SYNC_BLINDSCAN
		//init_waitqueue_head(&bs_cb_wq);//init it in attach now

		event_status = 0;
#endif

		//kobject_uevent();
		channel_info.frequency = p_tp_info[0].freq_KHz;
		channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
		channel_info.port_type = port_m88cs8800_deparse_dvb_type(p_tp_info[0].dvb_type);

		channel_info.DataTsNumber = p_tp_info[0].iTsCnt;
		channel_info.ts_id = p_tp_info[0].ucCurTsId;

		for (i = 0; i < p_tp_info[0].iTsCnt; i++)
		{
			channel_info.DataTsIdArray[i] = p_tp_info[0].ucTsId[i];

			if (p_tp_info[0].ucTsId[i] == p_tp_info[0].ucCurTsId)
				index = i;
		}

		channel_info.ts_index = index;

		channel_info.PLS.PLSDetail.PLSType = p_tp_info[0].bHavePLS ? 1 : 0;
		channel_info.PLS.PLSDetail.PLSCode[0] = p_tp_info[0].ucPLSCode[0];
		channel_info.PLS.PLSDetail.PLSCode[1] = p_tp_info[0].ucPLSCode[1];
		channel_info.PLS.PLSDetail.PLSCode[2] = p_tp_info[0].ucPLSCode[2];

		cs8800_nl_sendmsg(&channel_info);

#ifdef CFG_SYNC_BLINDSCAN
		//printk("%s() %d: prepare to wait event, wq = 0x%08x, evt_status = %d\n", __FUNCTION__, __LINE__, &wq, event_status);

		error = wait_event_interruptible_timeout(bs_cb_wq, (event_status == 1), BLINDSCAN_WAIT_CB_TIMEOUT * HZ);
		if (error == 0)
		{
			//MT_ERR_FRONTEND("%s: blindscan wait callback timeout!\n", __FUNCTION__);
			printk("[%s %d]blindscan wait callback timeout!\n", __FUNCTION__, __LINE__);
		}
		else if (error == -ERESTARTSYS)
		{
			//MT_ERR_FRONTEND("%s: blindscan interrupted by a signal!\n", __FUNCTION__);
			printk("[%s %d]blindscan interrupted by a signal!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			//printk("%s[%d]: wait event returned error = %d, evt_status = %d\n", __FUNCTION__, __LINE__, error, event_status);
		}
#endif

		break;

	case MtFeMsg_BSAbort:
		printk("BSAbort \n");
		cs8800_nl_list_del();
		port_m88cs8800_notify_to_up_layer(MtFeMsg_BSAbort, NULL);
		break;

	case MtFeMsg_BSFinish:
		printk("BS Finished\n");

		p_bs_info = (MT_FE_BS_TP_INFO *)p_param;

		printk("Blind scan result, total %d TPs:\n", p_bs_info->tp_num);

		for (i = 0; i < p_bs_info->tp_num; i++)
		{
			printk("TP %2d -- %7d KHz, %5d KSs, %s\n", i + 1, p_bs_info->p_tp_info[i].freq_KHz, p_bs_info->p_tp_info[i].sym_rate_KSs, (p_bs_info->p_tp_info[i].dvb_type == MtFeType_DVBS2) ? "DVB-S2" : "DVB-S");
		}

		printk("\n");

		cs8800_nl_list_del();
		port_m88cs8800_notify_to_up_layer(MtFeMsg_BSFinish, NULL);
		break;

	default:
		break;
	}
}

#define PORT_M88CS8800_22KHZ_1_CYCLE (45)
#define PORT_M88CS8800_DISEQC_MSG_BIT0_HIGH_CYCLE_MIN (750)
#define PORT_M88CS8800_DISEQC_MSG_BIT0_HIGH_CYCLE_MAX (1300)
#define PORT_M88CS8800_DISEQC_MSG_BIT1_HIGH_CYCLE_MIN (300)
#define PORT_M88CS8800_DISEQC_MSG_BIT1_HIGH_CYCLE_MAX (700)
#define PORT_M88CS8800_DISEQC_MSG_BIT_HIGH_LOW_GAP (200)
#define PORT_M88CS8800_DISEQC_MSG_BYTE_DURATION_MIN (1300)
#define PORT_M88CS8800_DISEQC_MSG_BYTE_DURATION_MAX (1700)

static int port_m88cs8800_diseqc2_rx_get_bytes(void *p_param)
{
	MT_FE_RET ret = MtFeErr_Undef;
	//u32 i = 0;
	mt_fe_cs8800_priv_t *priv = (mt_fe_cs8800_priv_t *)p_param;

	u8 diseqc_rx_bit_index = 0;			  // valid bit index
	u8 diseqc_rx_bit_hi_cnt = 0;		  // valid bit high pulse cnt
	u8 diseqc_rx_bit_lo_cnt = 0;		  // valid bit low pulse cnt
	u8 diseqc_rx_bit_hi_duration = 0;	  // in bit high duration currently
	u8 diseqc_rx_bit_lo_duration = 0;	  // in bit low duration currently
	mt_u64 diseqc_rx_bit_hi_fst_time = 0; // the first time of bit high
	mt_u64 diseqc_rx_bit_hi_lst_time = 0; // the last time of bit high
	mt_u64 diseqc_rx_bit_lo_fst_time = 0; // the first time of bit low
	mt_u64 diseqc_rx_bit_lo_lst_time = 0; // the last time of bit low

	gpio_value_e val = 0;
	mt_u64 us0 = 0; //u32 second0 = 0, ms0 = 0, us0 = 0;
	mt_u64 us = 0;	//u32 second = 0, ms = 0, us = 0;
	mt_u64 delta = 0, hi_duration_time = 0, lo_duration_time = 0;
	u8 bRxStarted = 0;
	u8 rx_byte_index = 0;

	u32 start_or_not = 0;
#ifdef CONFIG_MT_CHIP_SYMPHONY6
	struct timespec64 gtv;
#else
    struct timeval gtv;
#endif
	unsigned long flag;

#if 0
	if ((priv->bSupportDvbS == 0) && (priv->bSupportDvbS2 == 0) && (priv->bSupportDvbSS2Auto == 0))
	{    // fix 112220
		CS8800_PRINTK("%d,%d,%d\n",priv->bSupportDvbS,priv->bSupportDvbS2,priv->bSupportDvbSS2Auto);//return MtFeErr_Fail;
	}
#endif

	//printk(" ---- Current Time[%d], rx_len = %d, gpio[%d]\n", us0, priv->cur_diseqc.rx_len, priv->cfg.pin_config.diseqc_rx_gpio_pin);

	memset(priv->diseqc_rx_buf, 0, 8);
	priv->diseqc_rx_cnt = 0;

	delta = 0;
	diseqc_rx_bit_lo_duration = 0;
	lo_duration_time = 0;

	diseqc_rx_bit_index = 0;
	diseqc_rx_bit_hi_cnt = 0;
	diseqc_rx_bit_lo_cnt = 0;
	diseqc_rx_bit_hi_duration = 0;
	diseqc_rx_bit_lo_duration = 0;
	diseqc_rx_bit_hi_fst_time = 0;
	diseqc_rx_bit_hi_lst_time = 0;
	diseqc_rx_bit_lo_fst_time = 0;
	diseqc_rx_bit_lo_lst_time = 0;

	local_irq_save(flag);

#ifdef CONFIG_MT_CHIP_SYMPHONY6
	ktime_get_real_ts64(&gtv);
	us0 = gtv.tv_sec * 1000 * 1000 + gtv.tv_nsec/1000;
#else
	do_gettimeofday(&gtv);
	us0 = gtv.tv_sec * 1000 * 1000 + gtv.tv_usec;
#endif


	do
	{
		drv_gpio_get_value(priv->cfg.pin_config.diseqc_rx_gpio_pin, &val);

#ifdef CONFIG_MT_CHIP_SYMPHONY6
		ktime_get_real_ts64(&gtv);
		us = gtv.tv_sec * 1000 * 1000 + gtv.tv_nsec/1000;
#else
		do_gettimeofday(&gtv);
		us = gtv.tv_sec * 1000 * 1000 + gtv.tv_usec;
#endif


		// udelay(3);

		if (val == 1) // bit high
		{
			//printk("[%d] ---- GPIO[%d] level ---111---! hi_duration = %d\n", us, priv->cfg.pin_config.diseqc_rx_gpio_pin, diseqc_rx_bit_hi_duration);
			start_or_not = 1;

			if (bRxStarted == 0)
			{
				bRxStarted = 1;
			}
			else
			{
				diseqc_rx_bit_hi_lst_time = us - us0; ////(second - second0) * 1000 * 1000 + (ms - ms0) * 1000 + (us - us0);
			}

			if (diseqc_rx_bit_hi_duration == 0) // start bit high duration, update the first and the last time, close bit low duration
			{
				diseqc_rx_bit_hi_fst_time = us - us0; ////(second - second0) * 1000 * 1000 + (ms - ms0) * 1000 + (us - us0);
				diseqc_rx_bit_hi_duration = 1;
			}
			else // already enter bit high duration, update the last time
			{
				hi_duration_time = diseqc_rx_bit_hi_lst_time - diseqc_rx_bit_hi_fst_time;
				diseqc_rx_bit_hi_cnt = hi_duration_time / 45;
			}
		}
		else // bit low
		{
			// printk("[%d] ---- GPIO[%d] level ---000---! hi_duration = %d\n",us, priv->cfg.pin_config.diseqc_rx_gpio_pin, diseqc_rx_bit_hi_duration);

			if (diseqc_rx_bit_hi_duration == 1) // start bit high duration, update the first and the last time, close bit low duration
			{
				diseqc_rx_bit_lo_fst_time = us - us0; ////(second - second0) * 1000 * 1000 + (ms - ms0) * 1000 + (us - us0);

				diseqc_rx_bit_lo_lst_time = diseqc_rx_bit_lo_fst_time;

				if ((diseqc_rx_bit_lo_lst_time - diseqc_rx_bit_hi_lst_time) >= PORT_M88CS8800_DISEQC_MSG_BIT_HIGH_LOW_GAP)
				{
					if (((diseqc_rx_bit_hi_lst_time - diseqc_rx_bit_hi_fst_time) >= PORT_M88CS8800_DISEQC_MSG_BIT1_HIGH_CYCLE_MIN) &&
						((diseqc_rx_bit_hi_lst_time - diseqc_rx_bit_hi_fst_time) <= PORT_M88CS8800_DISEQC_MSG_BIT1_HIGH_CYCLE_MAX))
					{
						if (diseqc_rx_bit_index < 7) // shift bits
						{
							priv->diseqc_rx_buf[rx_byte_index] |= 0x01;
							priv->diseqc_rx_buf[rx_byte_index] <<= 1;
						}
						else if (diseqc_rx_bit_index == 7)
						{
							priv->diseqc_rx_buf[rx_byte_index] |= 0x01;
						}

						diseqc_rx_bit_index++;

#if 0
						if (data_index < 128)
						{
							gpio_data[data_index].val = 1;
							gpio_data[data_index].delta = diseqc_rx_bit_hi_lst_time - diseqc_rx_bit_hi_fst_time;
							data_index++;
						}
#endif

						if (diseqc_rx_bit_index == 8) // total 8 bits
						{
							rx_byte_index++;
							priv->diseqc_rx_cnt++;
						}
						else if (diseqc_rx_bit_index == 9) // bit 9, DiSEqC 'P', one byte end
						{
							diseqc_rx_bit_index = 0;
						}
					}
					else if (((diseqc_rx_bit_hi_lst_time - diseqc_rx_bit_hi_fst_time) >= PORT_M88CS8800_DISEQC_MSG_BIT0_HIGH_CYCLE_MIN) &&
							 ((diseqc_rx_bit_hi_lst_time - diseqc_rx_bit_hi_fst_time) <= PORT_M88CS8800_DISEQC_MSG_BIT0_HIGH_CYCLE_MAX))
					{
						if (diseqc_rx_bit_index < 7) // shift bits
						{
							priv->diseqc_rx_buf[rx_byte_index] &= 0xFE;
							priv->diseqc_rx_buf[rx_byte_index] <<= 1;
						}
						else if (diseqc_rx_bit_index == 7)
						{
							priv->diseqc_rx_buf[rx_byte_index] &= 0xFE;
						}

						diseqc_rx_bit_index++;

#if 0
						if (data_index < 128)
						{
							gpio_data[data_index].val = 0;
							gpio_data[data_index].delta = diseqc_rx_bit_hi_lst_time - diseqc_rx_bit_hi_fst_time;
							data_index++;
						}
#endif

						if (diseqc_rx_bit_index == 8) // total 8 bits
						{
							rx_byte_index++;
							priv->diseqc_rx_cnt++;
						}
						else if (diseqc_rx_bit_index == 9) // bit 9, DiSEqC 'P', one byte end
						{
							diseqc_rx_bit_index = 0;
						}
					}
					else
					{
						memset(priv->diseqc_rx_buf, 0, 8);
						//printk("%lld,%lld\n",diseqc_rx_bit_hi_lst_time,diseqc_rx_bit_hi_fst_time);
						priv->diseqc_rx_cnt = 0;
						diseqc_rx_bit_index = 0;

						bRxStarted = 0;
						diseqc_rx_bit_hi_duration = 0;
						//data_index = 0;

						//gpio_set_dir(priv->cfg.pin_config.diseqc_rx_gpio_pin, GPIO_DIR_OUTPUT);

#if 0
						mtos_systime_get(&second0, &ms0, &us0);
						CT8K_DEBUG(3, "[%d] ---- Current Time[%5d.%03d.%03d], quit\n", __LINE__, second0, ms0, us0);

						nim_unlock(priv->drv_base);

						return ERR_FAILURE;
#else
						rx_byte_index = 0;
						diseqc_rx_bit_index = 0;

						continue;
#endif
					}

					diseqc_rx_bit_hi_duration = 0;
				}
			}
		}

		delta = us - us0; //(second - second0) * 1000 * 1000 + (ms - ms0) * 1000 + (us - us0);

		if (delta > (priv->cur_diseqc.rx_len * 1000 * 135 * 2 + 50 * 1000))
		{
			printk("---- timeout, break!st:%d\n", start_or_not);
			ret = MtFeErr_Fail;
			break;
		}
	} while (priv->diseqc_rx_cnt < priv->cur_diseqc.rx_len);

	local_irq_restore(flag);

	if (MtFeErr_Undef == ret)
	{
		if (priv->diseqc_rx_cnt == priv->cur_diseqc.rx_len)
			ret = MtFeErr_Ok;
		else
			ret = MtFeErr_Fail;
	}

#if 0
	for (diseqc_rx_bit_index = 0; diseqc_rx_bit_index < priv->diseqc_rx_cnt; diseqc_rx_bit_index++)
	{
		printk("DiSEqC byte[%d of %d]\n", diseqc_rx_bit_index + 1, priv->diseqc_rx_cnt);
		printk(" ---- 0x%02x\n",priv->diseqc_rx_buf[diseqc_rx_bit_index]);
	}
	printk("diseqc 2.0 rx byte end.\n");
#endif

	return ret;
}

void port_m88cs8800_diseqc2_rx_gpio_init(mt_u8 rx_gpio_pin)
{
#ifdef DEMO_BOARD_TEST_DISEQC2
	mt_u32 gpio_pinmux_val = 0;

	if (rx_gpio_pin == GPIO_18)
	{
		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xbf13c048));
		printk("gpio_pinmux_val1:0x%x\n", gpio_pinmux_val);
		gpio_pinmux_val = (gpio_pinmux_val & 0xfffffff0) | 0x1;
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xbf13c048), gpio_pinmux_val);
	}
	else if (rx_gpio_pin == AO_GPIO_2)
	{
		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xbf15b408));
		printk("aogpio_pinmux_val1:0x%x\n", gpio_pinmux_val);
		gpio_pinmux_val = (gpio_pinmux_val & 0xfffffff8) | 0x1;
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xbf15b408), gpio_pinmux_val);
	}
#endif

	drv_gpio_io_enable(rx_gpio_pin, TRUE);

	drv_gpio_set_dir(rx_gpio_pin, GPIO_DIR_INPUT);
}

#if 0
static void sym6_dump_demux_registers(mt_u32 line_no)
{
	//reg r 0xbf230000

	mt_u32 temp = 0, addr = 0xbf230000;
	mt_u16 offset = 0;

	printk("\n%s[%d] ---- %d\n", __FUNCTION__, __LINE__, line_no);

	for (offset = 0; offset < 16; offset += 4)
	{
		_mt_fe_read32_cs8800(addr + offset, &temp);
		printk("%s[%d] ---- 0x%08x = [0x%08x]\n", __FUNCTION__, __LINE__, addr + offset, temp);
	}

	printk("%s[%d]\n\n", __FUNCTION__, __LINE__);

	return;
}
#endif

#ifdef CONFIG_NET
int m88cs8800_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
	mt_u8 reg_val = 0;
	int ret = 0;
	mt_u32 temp = 0;
	mt_fe_cs8800_priv_handle p_priv = NULL;
	unsigned long chip_rev = symphony_get_chip_rev();
	//printk(KERN_ERR "%s[%d] ---- info = 0x%08x, g_dev_init_flag = %d\n", __FUNCTION__, __LINE__, info, g_dev_init_flag);

	if (info->is_attach)
	{
		printk(KERN_ERR "[%s %d]has attach\n", __FUNCTION__, __LINE__);
		return MT_SUCCESS;
	}

	if (attr->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
	{
		if(mt_otp_get_hw_bonding(MT_OTP_DVBC) != 0)
		{
			printk(KERN_ERR "DVBC OTP HW bonding disabled!\n");
			return -EACCES;
		}
	}
	if(attr->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
	{
		if(mt_otp_get_hw_bonding(MT_OTP_J83B) != 0)
		{
			printk(KERN_ERR "J83B OTP HW bonding disabled!\n");
			return -EACCES;
		}
	}
    if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
	(attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
	(attr->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
	(attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
    {
		if(mt_otp_get_hw_bonding(MT_OTP_DVBS_S2) != 0)
		{
			printk(KERN_ERR "DVBS OTP HW bonding disabled!\n");
			return -EACCES;
		}
	}
	info->ops.connect = port_m88cs8800_channel_connect;
	info->ops.get_status = port_m88cs8800_get_status;
	info->ops.get_ber = port_m88cs8800_get_ber;
	info->ops.get_snr = port_m88cs8800_get_snr;
	info->ops.get_signal_strength = port_m88cs8800_get_signal_strength;
	info->ops.get_signal_quality = port_m88cs8800_get_signal_quality;
	info->ops.get_signal_agc = port_m88cs8800_get_signal_agc;

	info->ops.standby = port_m88cs8800_standby;
	info->ops.wakeup = port_m88cs8800_wakeup;
	info->ops.get_default_timeout = port_m88cs8800_get_default_timeout;
	info->ops.set_io = port_m88cs8800_set_io;
	info->ops.port_ioctl = port_m88cs8800_ioctl;
	info->ops.blind_scan = port_m88cs8800_blind_scan;

	if (!g_dev_init_flag)
	{
//---
#if 0
		//1.	配置0xbf50f818[18]， 配置值为0x1    //将apb clk3的时钟切换到xtal
		_mt_fe_read32_cs8800(0xbf50f818, &temp);
		temp |= 0x40000;
		_mt_fe_write32_cs8800(0xbf50f818, temp); //+++ CRM: TOPCLK_CTRL6_REG

		//2.	等待1us
		_mt_delayus_cs8800(10);

		//3.	配置0xbf508004[7:6]，配置值为0x3  //将apb 时钟切换到 apb clk3，即xtal时钟
		_mt_fe_read32_cs8800(0xbf508004, &temp);
		temp |= 0xc0;
		_mt_fe_write32_cs8800(0xbf508004, temp); //+++ CRM: BUS_CLKSEL_REG

		//4.	等待1us
		_mt_delayus_cs8800(10);

		//5.	配置0xbf50f818[24]，配置值为0x1  //使能demo时钟
		_mt_fe_read32_cs8800(0xbf50f818, &temp);
		temp |= 0x1000000;
		_mt_fe_write32_cs8800(0xbf50f818, temp); //+++ CRM: TOPCLK_CTRL6_REG

		if ((MT_CHIP_SYMPHONY6_A0 <= stSysChipInfo.enChipVersion) && (MT_CHIP_SYMPHONY6_MAX >= stSysChipInfo.enChipVersion))
		{
			_mt_fe_write32_cs8800(0xbf50f818, 0x120001f); //+++ CRM: TOPCLK_CTRL6_REG

			//_mt_fe_write32_cs8800(0xbf50b000, 0x01);

			//_mt_fe_write32_cs8800(0xbf50b00c, 0xffffffff);
		}

		//6.	等待1us
		_mt_delayus_cs8800(10);

		//7.	配置0xbf508004[7:6]，配置值为0x0 //将apb 时钟切换到 apb clk0，即80MHz时钟
		_mt_fe_read32_cs8800(0xbf508004, &temp);
		temp &= ~0xc0;
		_mt_fe_write32_cs8800(0xbf508004, temp); //+++ CRM: BUS_CLKSEL_REG
#else
		mt_clk_enable(MT_CLK_DEMO);
#endif

		/*read 0xbf5b0c00 before demod start working*/

		//_mt_fe_read32_cs8800(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base() + 0x00));

		//sym6_dump_demux_registers(__LINE__);

#if 1
		if ((CHIP_SYMPHONY6_A0 <= chip_rev) && (CHIP_SYMPHONY6_MAX >= chip_rev))
		{
//---
//pinmux config in dtsi
#if 0
			_mt_fe_read32_cs8800(0xBF13C010, &temp);
			//printk("%s[%d] ---- Get 0xBF13C010 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0x110000;
			_mt_fe_write32_cs8800(0xBF13C010, temp);
			//printk("%s[%d] ---- Set 0xBF13C010 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);
#endif

#if 0
			_mt_fe_read32_cs8800(0xBF138008, &temp);
			//printk("%s[%d] ---- Get 0xBF138008 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0x200; // bit9 = 0
			_mt_fe_write32_cs8800(0xBF138008, temp);
			//printk("%s[%d] ---- Set 0xBF138008 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);
#endif

//---
//pinmux config in dtsi
#if 0
			_mt_fe_write32_cs8800(0xBF13C008, 0x02254110);
			//printk("%s[%d] ---- Set 0xBF13C008 = [0x%08x] ---\n", __FUNCTION__, __LINE__, 0x02254110);
#endif

//---
#if 0
			//ADAC:2Vrms
			//BF157000[31:0]=0x0006_6000	all analog on & 2vrm on
			_mt_fe_write32_cs8800(0xBF157000, 0x00066000);
			//printk("%s[%d] ---- Set 0xBF157000 = [0x%08x] ---\n", __FUNCTION__, __LINE__, 0x00066000);
#else
            /*Task 27399 Lowpower optimize*/
			//mt_analog_enable(MT_ANA_CADC);
			//mt_analog_enable(MT_ANA_SADC);
#endif

//---
#if 0
			//BF5D011C[25]=1	ephy  on and reset
			_mt_fe_read32_cs8800(0xBF5D011C, &temp);
			//printk("%s[%d] ---- Get 0xBF5D011C = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp |= 0x2000000;
			_mt_fe_write32_cs8800(0xBF5D011C, temp);
			//printk("%s[%d] ---- Set 0xBF5D011C = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);

			//wait 2ms
			_mt_delay_cs8800(2);


			//BF5D011C[7]=1, wait 1us, BF5D011C[7]=0,
			temp |= 0x80;
			_mt_fe_write32_cs8800(0xBF5D011C, temp);
			//printk("%s[%d] ---- Set 0xBF5D011C = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);

			_mt_delay_cs8800(1);

			temp &= ~0x80;
			_mt_fe_write32_cs8800(0xBF5D011C, temp);
			//printk("%s[%d] ---- Set 0xBF5D011C = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//wait 2ms
			_mt_delay_cs8800(2);

			//BF500018[1]=0,wait 1us,BF500018[1]=1,
			_mt_fe_read32_cs8800(0xBF500018, &temp);
			//printk("%s[%d] ---- Get 0xBF500018 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0x02;
			_mt_fe_write32_cs8800(0xBF500018, temp);
			//printk("%s[%d] ---- Set 0xBF500018 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);

			_mt_delay_cs8800(1);

			temp |= 0x02;
			_mt_fe_write32_cs8800(0xBF500018, temp);
			//printk("%s[%d] ---- Set 0xBF500018 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//BF5D0000<9>=_0	ADAC sel
			_mt_fe_read32_cs8800(0xBF5D0000, &temp);
			//printk("%s[%d] ---- Get 0xBF5D0000 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0x200;
			_mt_fe_write32_cs8800(0xBF5D0000, temp);
			//printk("%s[%d] ---- Set 0xBF5D0000 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//BF5D0000[11:10]=_11	ADAC ouput enable
			temp |= 0xC00;
			_mt_fe_write32_cs8800(0xBF5D0000, temp);
			//printk("%s[%d] ---- Set 0xBF5D0000 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);
#endif


//---
#if 0
			//BF5D009C=0x0000_0000	enable drv0/drv1
			_mt_fe_write32_cs8800(0xBF5D009C, 0x00000000);
			//printk("%s[%d] ---- Set 0xBF5D009C = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);
#else
            /*Task 27399 Lowpower optimize*/
			//mt_analog_enable(DRV0_CLK_ANA);//DVBC J83B
			//mt_analog_enable(DRV1_CLK_ANA);//DVBS
#endif

			//BF5D0048H[7:4]=_0000	enable drv0/drv1
			_mt_fe_read32_cs8800(0xBF5D0048, &temp);
			//printk("%s[%d] ---- Get 0xBF5D0048 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0xF0;
			_mt_fe_write32_cs8800(0xBF5D0048, temp);
			//printk("%s[%d] ---- Set 0xBF5D0048 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//BF5D0098[29:28]=_00	reduce chopping spur
			_mt_fe_read32_cs8800(0xBF5D0098, &temp);
			//printk("%s[%d] ---- Get 0xBF5D0098 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0x30000000;
			_mt_fe_write32_cs8800(0xBF5D0098, temp);
			//printk("%s[%d] ---- Set 0xBF5D0098 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


//---
#if 0
			//BF5D0030=0x1f6021cc	usb0 dig
			_mt_fe_write32_cs8800(0xBF5D0030, 0x1F6021CC);
			//printk("%s[%d] ---- Set 0xBF5D0030 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//BF5D0034=0x1f6021cc	usb1 dig
			_mt_fe_write32_cs8800(0xBF5D0034, 0x1F6021CC);
			//printk("%s[%d] ---- Set 0xBF5D0034 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//BF5D0024[31]=1	usb0 hub opt
			_mt_fe_read32_cs8800(0xBF5D0024, &temp);
			//printk("%s[%d] ---- Get 0xBF5D0024 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp |= 0x80000000;
			_mt_fe_write32_cs8800(0xBF5D0024, temp);
			//printk("%s[%d] ---- Set 0xBF5D0024 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);

			//BF5D002C[31]=1	usb1 hub opt
			_mt_fe_read32_cs8800(0xBF5D002C, &temp);
			//printk("%s[%d] ---- Get 0xBF5D002C = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp |= 0x80000000;
			_mt_fe_write32_cs8800(0xBF5D002C, temp);
			//printk("%s[%d] ---- Set 0xBF5D002C = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);
#endif

			//sym6_dump_demux_registers(__LINE__);


//---
//pinmux config in dtsi
#if 0
			// DVB-C pinmux
			//BF13C048[3:0] = 4	//C_AGC pinmux
			_mt_fe_read32_cs8800(0xBF13C048, &temp);
			temp &= 0xFFFFFFF0;
			temp |= 0x04;
			_mt_fe_write32_cs8800(0xBF13C048, temp);
#endif

			//BF5D0094[3] = 0	//IF PAD设置为ADC input
			_mt_fe_read32_cs8800(0xBF5D0094, &temp);
			temp &= 0xFFFFFFF7;
			_mt_fe_write32_cs8800(0xBF5D0094, temp);

#if 0
			//BF138008[0] = 1	//TS2 为DVBC Demod 输入
			_mt_fe_read32_cs8800(0xBF138008, &temp);
			temp |= 0x01;
			_mt_fe_write32_cs8800(0xBF138008, temp);
#else
			//BF138008[20] = 1	// CAM_S_SEL = 1
			//BF138008[12] = 0	// TS2 select TS2 from internal demod DVB-C/J.83B/DVB-S2_2
			_mt_fe_read32_cs8800(0xBF138008, &temp);
			temp |= (1 << 20);
			temp &= ~(1 << 12);
			_mt_fe_write32_cs8800(0xBF138008, temp);
#endif

//---
//pinmux config in dtsi
#if 0
			_mt_fe_write32_cs8800(0xBF13C04C, 0x3002);
			_mt_fe_write32_cs8800(0xBF13C050, 0x3002);

			//tuner i2c
			_mt_fe_write32_cs8800(0xBF13C07C, 0x0000);
			_mt_fe_write32_cs8800(0xBF13C080, 0x0000);
			//IF_AGC
			_mt_fe_write32_cs8800(0xBF13C084, 0x0000);

			//sym6_dump_demux_registers(__LINE__);

			// DVB-S/S2 pinmux
			//BF13C038[3:0] = 4
			//[11:10]设置为0（驱动改为最小）。
			_mt_fe_read32_cs8800(0xBF13C038, &temp);
			//temp &= 0xFFFFFFF0;
			temp &= 0xFFFFF3F0;
			temp |= 0x04;
			_mt_fe_write32_cs8800(0xBF13C038, temp);

			//BF13C03C[3:0] = 4
			_mt_fe_read32_cs8800(0xBF13C03C, &temp);
			temp &= 0xFFFFFFF0;
			temp |= 0x04;
			_mt_fe_write32_cs8800(0xBF13C03C, temp);

			//BF13C040[3:0] = 4
			_mt_fe_read32_cs8800(0xBF13C040, &temp);
			temp &= 0xFFFFFFF0;
			temp |= 0x04;
			_mt_fe_write32_cs8800(0xBF13C040, temp);

			//BF13C044[3:0] = 4 //LNB  pinmux
			_mt_fe_read32_cs8800(0xBF13C044, &temp);
			temp &= 0xFFFFFFF0;
			temp |= 0x04;
			_mt_fe_write32_cs8800(0xBF13C044, temp);
#endif

			//BF5D0094[0] = 0	//IQ PAD设置为ADC input
			_mt_fe_read32_cs8800(0xBF5D0094, &temp);
			temp &= 0xFFFFFFFE;
			_mt_fe_write32_cs8800(0xBF5D0094, temp);

#if 0
			//BF138008[4] = 1	//TS1 为DVBS Demod 输入
			_mt_fe_read32_cs8800(0xBF138008, &temp);
			temp |= 0x10;
			_mt_fe_write32_cs8800(0xBF138008, temp);
#else
			//BF138008[4] = 0	//TS1 select TS1 from internal demod DVB-S2_0
			_mt_fe_read32_cs8800(0xBF138008, &temp);
			temp &= ~(1 << 4);
			_mt_fe_write32_cs8800(0xBF138008, temp);
			if ((CHIP_SYMPHONY6_A1 <= chip_rev) && (CHIP_SYMPHONY6_MAX >= chip_rev))
			{
				_mt_fe_read32_cs8800(0xbf200080, &temp);
				temp |= 0x2490;     //bit13  bit10 bit7 bit4 = 1,    data bitwidth slecte 1:bit 0:2bit
 				_mt_fe_write32_cs8800(0xbf200080, temp);
			}

			/*fix issue32516 C-IF-AGC PIN drive_strength set to 1*/
			if ((CHIP_SYMPHONY6_A1 <= chip_rev) && (CHIP_SYMPHONY6_MAX >= chip_rev))
			{
				_mt_fe_read32_cs8800(0xBF13C084, &temp);
				temp &= ~(0XF000);
				temp |= 0x1000; //0x1000 0xF000
 				_mt_fe_write32_cs8800(0xBF13C084, temp);			
			}
			
#endif

//---
//pinmux config in dtsi
#if 0
			//AGC
			_mt_fe_write32_cs8800(0xBF13C088, 0x0000);

			// DISEQC
			_mt_fe_write32_cs8800(0xBF13C06C, 0x0000);
			_mt_fe_write32_cs8800(0xBF13C074, 0x0000);
			_mt_fe_write32_cs8800(0xBF13C078, 0x0000);
			_mt_fe_write32_cs8800(0xBF13C070, 0x0000);


			_mt_fe_write32_cs8800(0xBF13C07C, 0x0000);
			_mt_fe_write32_cs8800(0xBF13C080, 0x0000);
#endif

			mt_clk_set_attr("ts1_clk", 1);	/* 01: demos_ts1_clk */
		}

		//sym6_dump_demux_registers(__LINE__);

		//_mt_fe_read32_cs8800(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base() + 0x00));
#endif
	}

	p_priv = kzalloc(sizeof(mt_fe_cs8800_priv_t), GFP_KERNEL);
	if (NULL == p_priv)
	{
		printk(KERN_ERR "[%s %d]kzalloc priv error\n", __FUNCTION__, __LINE__);

		return -ENOMEM;
	}

	if (NULL == dev_handle)
	{
		dev_handle = kzalloc(sizeof(MT_FE_CS8800_DEVICE_SETTINGS), GFP_KERNEL);

		if (NULL == dev_handle)
		{
			kfree((void *)p_priv);
			p_priv = NULL;

			g_cs8800_priv = NULL;

			printk(KERN_ERR "[%s %d]kzalloc dev_handle error\n", __FUNCTION__, __LINE__);

			return -ENOMEM;
		}
	}

	p_priv->cs8800_handle = dev_handle;
	info->handle = (void *)p_priv;
	//g_cs8800_priv = p_priv;

	g_cs8800_handle = dev_handle;

	g_i2c_cs8800 = attr->demod_i2c_id;

	printk(KERN_ERR "[%s %d] ---- g_i2c_cs8800 & demod_i2c_id[%d]\n", __FUNCTION__, __LINE__, g_i2c_cs8800);

	memcpy(&(p_priv->cfg), &(attr->fe_config), sizeof(mt_unf_fe_config_para_t));

	p_priv->sig_type = attr->sig_type;
	p_priv->onoff_22k = 0;
	p_priv->bs_stop = FALSE;
	p_priv->diseqc_2x = 0;
	p_priv->lnb_polar = PORT_PORLAR_HORIZONTAL;
	p_priv->lnb_voltage = MtFeLNB_18V;
	p_priv->lnb_onoff = 0;
	p_priv->cur_diseqc.tx_len = 0;
	//p_priv->cur_diseqc.p_tx_buf = priv->diseqc_tx_buf;

	p_priv->cfg.pin_config.lnb_enable = 1;				//MT_FE_PIN_LEVEL_HIGH;
	p_priv->cfg.pin_config.vsel_when_13v = 0;			//MT_FE_PIN_LEVEL_LOW;
	p_priv->cfg.pin_config.vsel_when_lnb_off = 1;		//MT_FE_PIN_LEVEL_HIGH;
	p_priv->cfg.pin_config.diseqc_out_when_lnb_off = 0;	//MT_FE_PIN_LEVEL_LOW;
	p_priv->cfg.pin_config.lnb_enable_by_mcu = 0;
	p_priv->cfg.pin_config.lnb_prot_by_mcu = 0;

	p_priv->cfg.pin_config.diseqc_rx_mode = attr->fe_config.pin_config.diseqc_rx_mode;
	p_priv->cfg.pin_config.diseqc_rx_gpio_pin = attr->fe_config.pin_config.diseqc_rx_gpio_pin;

	if (p_priv->cfg.freq_offset_limit == 0)
	{
		p_priv->cfg.freq_offset_limit = 4000;
	}

	if (NULL == cs8800_pg_channel_info)
	{
		cs8800_pg_channel_info = kzalloc(sizeof(mt_unf_fe_channel_info_t) * MAX_BS_TP_NUM_PER_SAT, GFP_KERNEL);
		if (cs8800_pg_channel_info == NULL)
		{
			kfree(dev_handle);
			dev_handle = NULL;

			kfree((void *)p_priv);
			p_priv = NULL;

			g_cs8800_priv = NULL;

			return -ENOMEM;
		}
	}

	p_priv->scan_info.p_channel_info = cs8800_pg_channel_info;

	//p_priv->bs_info.bs_times = p_priv->cfg.bs_times;
	p_priv->bs_info.tp_max_num = MAX_TP_ONE_SCAN;
	if (NULL == p_priv->bs_info.p_tp_info)
	{
		p_priv->bs_info.p_tp_info = kzalloc(sizeof(MT_FE_TP_INFO) * p_priv->bs_info.tp_max_num, GFP_KERNEL);
		if (p_priv->bs_info.p_tp_info == NULL)
		{
			kfree(dev_handle);
			dev_handle = NULL;

			kfree((void *)p_priv);
			p_priv = NULL;

			if (cs8800_pg_channel_info)
			{
				kfree(cs8800_pg_channel_info);
				cs8800_pg_channel_info = NULL;
			}

			g_cs8800_priv = NULL;

			return -ENOMEM;
		}
	}
	p_priv->bs_info.bs_algorithm = MT_FE_BS_ALGORITHM_A;

	if (!g_dev_init_flag)
	{
		mt_fe_dmd_cs8800_config_default(dev_handle);
		dev_handle->sys_dev_xtal = MtFeXTALMode_27M;
	}

	if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_CAB) || 
		(attr->sig_type == MT_UNF_FE_SIG_TYPE_J83B))
	{
		printk(KERN_ERR "[%s %d]tuner_type = %d, tuner_addr = 0x%02x\n", 
			   __FUNCTION__, __LINE__, attr->tuner_type, attr->tuner_addr);

		dev_handle->m_device_c_b.tuner_cfg.tuner_dev_addr = attr->tuner_addr;

		if (attr->tuner_type == MT_UNF_TUNER_TYPE_MXL603)
		{
			mt_fe_dmd_select_tuner_c_b_cs8800(dev_handle, MtFeTN_MxL603);
			//printk("----------------cs8800 select ctt2 tuner MxL603-------------------------------\n");
		}
		else // if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800)
		{
			mt_fe_dmd_select_tuner_c_b_cs8800(dev_handle, MtFeTN_TC6800);
			printk(KERN_ERR "----------------cs8800 select ctt2 tuner TC6800-------------------------------\n");
		}
	}

	if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(attr->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
		(attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
	{
		printk(KERN_ERR "[%s %d]tuner_type = %d, tuner_addr = 0x%02x\n", 
			   __FUNCTION__, __LINE__, attr->tuner_type, attr->tuner_addr);

		dev_handle->m_device_ss2.tuner_cfg.tuner_dev_addr = attr->tuner_addr;
		g_cs8800_priv = p_priv;
		if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TS6011)
		{
			mt_fe_dmd_select_tuner_ss2_cs8800(dev_handle, MtFeTn_TS6011);
			//printk(KERN_ERR "----------------cs8800 select ss2 tuner TS6011-------------------------------\n");
		}
		else if (attr->tuner_type == MT_UNF_TUNER_TYPE_RT720)
		{
			mt_fe_dmd_select_tuner_ss2_cs8800(dev_handle, MtFeTn_RT720);
			//printk(KERN_ERR "----------------cs8800 select ss2 tuner RT720--------------------------------\n");
		}

		mt_fe_dmd_register_notify_cs8800_ss2(port_m88cs8800_register_to_drv_notify);

#if 1
		if (dvbs_nl_family.id > 0)
		{
			ret = genl_unregister_family(&dvbs_nl_family);
		}

		ret = genl_register_family(&cs8800_nl_family);
		if (ret)
		{
			printk("genl_register_family failed, ret = 0x%08x\n", ret);

			if (cs8800_pg_channel_info)
			{
				kfree(cs8800_pg_channel_info);
				cs8800_pg_channel_info = NULL;
			}

			if (p_priv->bs_info.p_tp_info)
			{
				kfree(p_priv->bs_info.p_tp_info);
				p_priv->bs_info.p_tp_info = NULL;
			}

			kfree(dev_handle);
			dev_handle = NULL;

			kfree((void *)p_priv);
			p_priv = NULL;

			//printk("genl_register_family failed, maybe already registered, ret = 0x%08x\n", ret);

			g_cs8800_priv = NULL;
			return -EINVAL;
		}

		memcpy(&dvbs_nl_family, &cs8800_nl_family, sizeof(cs8800_nl_family));

#else
		if (dvbs_nl_family.id <= 0)
		{
			ret = genl_register_family(&cs8800_nl_family);
			if (ret)
			{
				printk("genl_register_family failed, ret = 0x%08x\n", ret);
				kfree(dev_handle);
				kfree((void *)p_priv);
				if (cs8800_pg_channel_info)
				{
					kfree(cs8800_pg_channel_info);
					cs8800_pg_channel_info = NULL;
				}

				if (p_priv->bs_info.p_tp_info)
				{
					kfree(p_priv->bs_info.p_tp_info);
					p_priv->bs_info.p_tp_info = NULL;
				}

				//printk("genl_register_family failed, maybe already registered, ret = 0x%08x\n", ret);

				g_cs8800_priv = NULL;
				return -EINVAL;
			}

			memcpy(&dvbs_nl_family, &cs8800_nl_family, sizeof(struct genl_family));
		}
		else
		{
#if 0
			printk("%s[%d] ---- cs8800_nl_family: \n", __FUNCTION__, __LINE__);
			printk("cs8800_nl_family.id                 = %d\n", cs8800_nl_family.id);
			printk("cs8800_nl_family.name               = %s\n", cs8800_nl_family.name);
			printk("cs8800_nl_family.version            = %d\n", cs8800_nl_family.version);
			printk("cs8800_nl_family.maxattr            = %d\n", cs8800_nl_family.maxattr);
			printk("cs8800_nl_family.netnsok            = %d\n", cs8800_nl_family.netnsok);
			printk("cs8800_nl_family.module             = %d\n", cs8800_nl_family.module);
			printk("cs8800_nl_family.ops                = %d\n", cs8800_nl_family.ops);
			printk("cs8800_nl_family.n_ops              = %d\n", cs8800_nl_family.n_ops);


			printk("%s[%d] ---- dvbs_nl_family: \n", __FUNCTION__, __LINE__);
			printk("dvbs_nl_family.id                 = %d\n", dvbs_nl_family.id);
			printk("dvbs_nl_family.name               = %s\n", dvbs_nl_family.name);
			printk("dvbs_nl_family.version            = %d\n", dvbs_nl_family.version);
			printk("dvbs_nl_family.maxattr            = %d\n", dvbs_nl_family.maxattr);
			printk("dvbs_nl_family.netnsok            = %d\n", dvbs_nl_family.netnsok);
			printk("dvbs_nl_family.module             = %d\n", dvbs_nl_family.module);
			printk("dvbs_nl_family.ops                = %d\n", dvbs_nl_family.ops);
			printk("dvbs_nl_family.n_ops              = %d\n", dvbs_nl_family.n_ops);
#endif

			//cs8800_nl_family.id = dvbs_nl_family.id;
			memcpy(&cs8800_nl_family, &dvbs_nl_family, sizeof(struct genl_family));

			cs8800_nl_family.module = THIS_MODULE;
			cs8800_nl_family.ops = cs8800_nl_ops;
			cs8800_nl_family.n_ops = ARRAY_SIZE(cs8800_nl_ops);
		}
#endif

#ifdef CFG_SYNC_BLINDSCAN
		init_waitqueue_head(&bs_cb_wq);
		event_status = 0;
#endif

		if (p_priv->cfg.pin_config.diseqc_rx_mode == 1)
			port_m88cs8800_diseqc2_rx_gpio_init(p_priv->cfg.pin_config.diseqc_rx_gpio_pin);

		spin_lock_init(&dev_handle->blindscan_status_slock);
	}
    
    /*Task 27399 Lowpower optimize*/
    if (attr->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
	{
        
        if((dev_handle->bDVBCInitOk == FALSE)||(dev_handle->bSysInitOk == FALSE))
        {
            mt_analog_enable(MT_ANA_CADC);
            mt_analog_enable(DRV0_CLK_ANA);//DVBC 
            /*Task 29693,dvbs clock and sadc must be enable when initialize dvbc,if dvbs has not been initialized;*/
            mt_analog_enable(MT_ANA_SADC);
            mt_analog_enable(DRV1_CLK_ANA);//DVBS
            mt_fe_system_init_cs8800(dev_handle);
            dev_handle->bDVBCInitOk = TRUE;
            printk(KERN_ERR "[%s %d]MT_ANA_CADC and DRV0_CLK_ANA enable \n", __FUNCTION__, __LINE__);
            /*Task 27399 Lowpower optimize*/
            if(dev_handle->bDVBSInitOk == FALSE)
	    {
	      mt_analog_disable(MT_ANA_SADC);
	      mt_analog_disable(DRV1_CLK_ANA);//DVBS
	    }
        }
        
    }
    if(attr->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
    {
        if((dev_handle->bJ83BInitOk == FALSE)||(dev_handle->bSysInitOk == FALSE))
        {
            mt_analog_enable(MT_ANA_CADC);
            mt_analog_enable(DRV1_CLK_ANA);//DVBS J83B
            mt_fe_system_init_cs8800(dev_handle);
            dev_handle->bJ83BInitOk = TRUE;
            printk(KERN_ERR "[%s %d]MT_ANA_CADC and DRV1_CLK_ANA enable \n", __FUNCTION__, __LINE__);
        }
    }
    if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(attr->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
		(attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
    {
        
        if((dev_handle->bDVBSInitOk == FALSE)||(dev_handle->bSysInitOk == FALSE))
        {
            mt_analog_enable(MT_ANA_SADC);
			mt_analog_enable(DRV1_CLK_ANA);//DVBS
            mt_fe_system_init_cs8800(dev_handle);
            dev_handle->bDVBSInitOk = TRUE;
            printk(KERN_ERR "[%s %d]MT_ANA_SADC and DRV1_CLK_ANA enable \n", __FUNCTION__, __LINE__);
        }
    }
    print_analog_status(__LINE__);
	ss2_status = MtFeLockState_Undef;

	info->is_attach = 1;
	info->pre_attr.demod_dev_type = attr->demod_dev_type;
	g_dev_init_flag = 1;
	g_priv_count++;
	//printk(KERN_ERR "[%s %d]success\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

int m88cs8800_detach(frontend_info_s *info)
{
	mt_fe_cs8800_priv_handle p_priv = NULL;
       MT_FE_CS8800_Device_Handle cs8800_handle = NULL;
	if (NULL == info)
	{
		printk(KERN_ERR "[%s %d]error, info is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	p_priv = (mt_fe_cs8800_priv_handle)(info->handle);
	if (NULL == p_priv)
	{
		printk(KERN_ERR "[%s %d]error, info is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}
        cs8800_handle = p_priv->cs8800_handle;
	if (info->is_attach)
	{
		if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
			(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
			(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
			(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
		{
            /*fix issue27746 slab-use-after-free*/
			/*fix issue30857 Use spin locks to sync the bBsStatus of different processes */
            if(port_m88cs8800_get_blindscan_status(cs8800_handle) != 0)
                port_m88cs8800_blind_scan_cancel_new(p_priv,100,1000);
            
			mt_fe_dmd_close_ss2_cs8800(p_priv->cs8800_handle);

			genl_unregister_family(&cs8800_nl_family);
		}
		else if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_CAB) || 
				 (p_priv->sig_type == MT_UNF_FE_SIG_TYPE_J83B))
		{
			mt_fe_dmd_close_c_b_cs8800(p_priv->cs8800_handle);
		}


		if (p_priv->bs_info.p_tp_info)
		{
			kfree(p_priv->bs_info.p_tp_info);
			p_priv->bs_info.p_tp_info = NULL;
		}

		//printk(KERN_ERR "[%s %d]g_priv_count = %d, g_dev_init_flag = %d\n", __FUNCTION__, __LINE__, g_priv_count, g_dev_init_flag);
		if (g_priv_count >= 1)
		{
			g_priv_count--;

			if (0 == g_priv_count)
			{
              	if (cs8800_pg_channel_info)
		        {
			        kfree(cs8800_pg_channel_info);
			        cs8800_pg_channel_info = NULL;
		        }
				if (dev_handle)
				{
					kfree(dev_handle);
					dev_handle = NULL;
				}
				g_dev_init_flag = 0;
			}
		}

		kfree(p_priv);
		p_priv = NULL;

		info->is_attach = 0;
		info->handle = NULL;
		memset(&info->ops, 0, sizeof(demod_ops_s));
		memset(&info->pre_attr, 0, sizeof(mt_unf_fe_attr_t));
	}

	return MT_SUCCESS;
}

//int m88cs8800_resume(frontend_info_s *info, mt_unf_fe_attr_t *attr)
int m88cs8800_resume(void)
{
	mt_u8 reg_val = 0;
	//int ret = 0;
	mt_u32 temp = 0;
	
        unsigned long chip_rev = symphony_get_chip_rev();
	//printk(KERN_ERR "[%s %d]g_dev_init_flag=%d\n", __FUNCTION__, __LINE__, g_dev_init_flag);

    //if (!g_dev_init_flag)
	{

		mt_clk_enable(MT_CLK_DEMO);

		/*read 0xbf5b0c00 before demod start working*/

		//_mt_fe_read32_cs8800(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base() + 0x00));

		//sym6_dump_demux_registers(__LINE__);

		if ((CHIP_SYMPHONY6_A0 <= chip_rev) && (CHIP_SYMPHONY6_MAX >= chip_rev))
		{

			//BF5D0048H[7:4]=_0000	enable drv0/drv1
			_mt_fe_read32_cs8800(0xBF5D0048, &temp);
			//printk("%s[%d] ---- Get 0xBF5D0048 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0xF0;
			_mt_fe_write32_cs8800(0xBF5D0048, temp);
			//printk("%s[%d] ---- Set 0xBF5D0048 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//BF5D0098[29:28]=_00	reduce chopping spur
			_mt_fe_read32_cs8800(0xBF5D0098, &temp);
			//printk("%s[%d] ---- Get 0xBF5D0098 = [0x%08x]\n", __FUNCTION__, __LINE__, temp);
			temp &= ~0x30000000;
			_mt_fe_write32_cs8800(0xBF5D0098, temp);
			//printk("%s[%d] ---- Set 0xBF5D0098 = [0x%08x] ---\n", __FUNCTION__, __LINE__, temp);


			//BF5D0094[3] = 0	//IF PAD设置为ADC input
			_mt_fe_read32_cs8800(0xBF5D0094, &temp);
			temp &= 0xFFFFFFF7;
			_mt_fe_write32_cs8800(0xBF5D0094, temp);

			//BF138008[20] = 1	// CAM_S_SEL = 1
			//BF138008[12] = 0	// TS2 select TS2 from internal demod DVB-C/J.83B/DVB-S2_2
			_mt_fe_read32_cs8800(0xBF138008, &temp);
			temp |= (1 << 20);
			temp &= ~(1 << 12);
			_mt_fe_write32_cs8800(0xBF138008, temp);

			//BF5D0094[0] = 0	//IQ PAD设置为ADC input
			_mt_fe_read32_cs8800(0xBF5D0094, &temp);
			temp &= 0xFFFFFFFE;
			_mt_fe_write32_cs8800(0xBF5D0094, temp);

			//BF138008[4] = 0	//TS1 select TS1 from internal demod DVB-S2_0
			_mt_fe_read32_cs8800(0xBF138008, &temp);
			temp &= ~(1 << 4);
			_mt_fe_write32_cs8800(0xBF138008, temp);

			mt_clk_set_attr("ts1_clk", 1);	/* 01: demos_ts1_clk */

			if(1)    //lnb- pgio 30   t2_powser - gpio18
			{
				/*LNB_EN： GPIO30，高电平 ON，低电平OFF； */
				drv_gpio_io_enable(30, TRUE);
		    		drv_gpio_set_dir(30, 0);
				drv_gpio_set_value(30, 1);	
				/* T2天线保护电路：GPIO18，高电平ON，低电平OFF。*/
				drv_gpio_io_enable(18, TRUE);
		    		drv_gpio_set_dir(18, 0);
				drv_gpio_set_value(18, 1);
			}
			
		}

		//sym6_dump_demux_registers(__LINE__);

		//_mt_fe_read32_cs8800(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base() + 0x00));

	}

	if (!g_dev_init_flag)
	{
		mt_fe_system_init_cs8800(dev_handle);
		printk("%s[%d] ---- CS8800 resume system init OK\n", __FUNCTION__, __LINE__);
	}

	ss2_status = MtFeLockState_Undef;

#if 0
	if (dev_handle->m_device_c_b.demod_current_type != MtFeType_Undef)
	{
		printk("%s[%d] ---- CS8800 DVB-C or J83B connect resume\n", __FUNCTION__, __LINE__);

		mt_fe_dmd_connect_c_b_cs8800(dev_handle);
	}

	if (dev_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
	{
		printk("%s[%d] ---- CS8800 DVB-S or S2 connect resume\n", __FUNCTION__, __LINE__);

		mt_fe_dmd_connect_ss2_cs8800(dev_handle);
	}
#endif

	return MT_SUCCESS;
}

//int m88cs8800_suspend(frontend_info_s *info)
int m88cs8800_suspend(void)
{
	mt_fe_dmd_close_ss2_cs8800(dev_handle);

	mt_fe_dmd_close_c_b_cs8800(dev_handle);

	g_bNeedReCali			 = TRUE;

	//printk("%s[%d] ---- handle[%08x], g_bNeedReCali = [%d]\n", __FUNCTION__, __LINE__, dev_handle, g_bNeedReCali);
	if(1)    //lnb- pgio 30   t2_powser - gpio18
	{
		/*LNB_EN： GPIO30，高电平 ON，低电平OFF； */
		drv_gpio_io_enable(30, TRUE);
    		drv_gpio_set_dir(30, 0);
		drv_gpio_set_value(30, 0);	
		/* T2天线保护电路：GPIO18，高电平ON，低电平OFF。*/
		drv_gpio_io_enable(18, TRUE);
    		drv_gpio_set_dir(18, 0);
		drv_gpio_set_value(18, 0);
	}
	printk("%s[%d] ---- CS8800 suspend OK\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}


/*
FE BIT5 --- SET mode
EB[7:0] --- channel 15:8
EC[7:0]----channel[7:0]    16个通道（0 通道除外）
EE[4:0]+ ED[7:0]  value
EF[3:0] SAVE Pattern to 通道
*/

static int m88cs8800_set_scid_filter(mt_unf_fe_dss_scid_filter_t  *scid_filter)
{
	MT_FE_DS6113_DSS_SCID_FILTER_T  ds6113_dss_filter = {0};
	if(scid_filter == NULL)   return MT_FAILURE;

	ds6113_dss_filter.b_filter_mode = scid_filter->b_filter_mode;
	ds6113_dss_filter.tuner_id = scid_filter->tuner_id;
	memcpy(ds6113_dss_filter.u16_mask, scid_filter->u16_mask, DS6113_FILTER_MAX_DEPTH);
	memcpy(ds6113_dss_filter.u16_scid, scid_filter->u16_scid, DS6113_FILTER_MAX_DEPTH);
	
	return _mt_fe_dmd_cs8800_set_dss_scid_filter(dev_handle, &ds6113_dss_filter);
}


#endif
