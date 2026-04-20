/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
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

#include "mt_fe_common_ds6103.h"
#include "port_ds6103.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_i2c_ds6103.h"
#include "mt_module_debug.h"

#include "mt_mach/chipinfo.h"


//#define FOR_PORT_DS6103_CONNECT_SYCHRONOUS

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

#define MT_FE_DS6103_PIN_LEVEL_LOW 0
#define MT_FE_DS6103_PIN_LEVEL_HIGH 1
//#define MT_FE_DS6103_DISEQC_COMMAND_START_DELAY 30
#define MT_FE_DS6103_DISEQC_COMMAND_START_DELAY 50
#define MT_FE_DS6103_DISEQC_COMMAND_END_DELAY 50

#define MAX_TP_ONE_SCAN MAX_BS_TP_NUM_PER_SAT

#define DS6103_NL_NAME "dvbs_bs_nl"

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum
{
  DS6103_NL_OPS_CMD0,
  //DS6103_NL_OPS_CMD1,
  __DS6103_NL_OPS_MAX,
};

#define DS6103_NL_OPS_AMOUNT (__DS6103_NL_OPS_MAX)

enum
{
  DS6103_NL_ATTR_UNSPEC,
  DS6103_NL_ATTR_DATA0,
  //DS6103_NL_ATTR_DATA1,
  __DS6103_NL_ATTR_MAX,
};

#define DS6103_NL_FAMILY_ATTR_MAX (__DS6103_NL_ATTR_MAX - 1)
#define DS6103_NL_FAMILY_ATTR_AMOUNT (__DS6103_NL_ATTR_MAX)

struct ds6103_netlink_data
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

struct ds6103_netlink_pid_list
{
  struct list_head list;
  struct ds6103_netlink_data ds6103_nl_data;
  struct net *net;
};

static MT_FE_DS6103_Device_Handle g_ds6103_handle = NULL;

#ifdef CFG_SYNC_BLINDSCAN
/* blind scan -> found one TP -> wait user callback return */
static int event_status;
/* blindscan callback wait queue */
static wait_queue_head_t bs_cb_wq;
#endif

static MT_FE_LOCK_STATE ss2_status = MtFeLockState_Undef;

//mt_fe_ds6103_priv_handle p_priv = NULL;
static MT_FE_DS6103_Device_Handle dev_handle = NULL;
static mt_u8 g_dev_init_flag = 0;
static mt_u8 g_priv_count = 0;

extern MT_FE_RET mt_fe_dmd_ds6103_register_notify(void (*callback)(MT_FE_MSG msg, void *p_tp_info));

/* The crypto netlink socket */
//static struct sock *ds6103_nlsk;

mt_fe_ds6103_priv_handle g_ds6103_priv = NULL;
int g_i2c_ds6103 = 0;
static MT_U8 ds6103_notify_scan_status = 0;//0:normal, 1:finished or abort
mt_unf_fe_channel_info_t *ds6103_pg_channel_info = NULL;

//mmz_buffer_s sMBuf;

static int port_m88ds6103_blind_scan_cancel(void *handle);
static int port_m88ds6103_blind_scan_start(void *pArg);
static int port_m88ds6103_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info);
static int ds6103_nl_list_del(void);

static LIST_HEAD(ds6103_nl_pid_list);

static DEFINE_MUTEX(ds6103_nl_mutex);

static struct genl_ops ds6103_nl_ops[DS6103_NL_OPS_AMOUNT];

static struct genl_family ds6103_nl_family = {
    .name = DS6103_NL_NAME,
    .version = 0x1,
    .maxattr = DS6103_NL_FAMILY_ATTR_MAX,
    .netnsok = true,
    .module = THIS_MODULE,
    .ops = ds6103_nl_ops,
    .n_ops = ARRAY_SIZE(ds6103_nl_ops),
};

extern struct genl_family dvbs_nl_family;

//static struct ds6103_netlink_data g_ds6103_nl_data_snd = {0};

MT_FE_DS6103_Device_Handle mt_fe_ds6103_get_handle(void)
{
  return g_ds6103_handle;
}

static int ds6103_nl_sendmsg(mt_unf_fe_channel_info_t *p_channel_info)
{
  //mt_unf_fe_channel_info_t *p_channel_info = NULL;
  struct genl_info info;
  struct ds6103_netlink_data ds6103_nl_data_snd;
  struct sk_buff *skb;
  struct ds6103_netlink_data *ds6103_nl_data;
  struct ds6103_netlink_pid_list *ds6103_nl_pid;
  struct ds6103_netlink_pid_list *tmp;
  void *hdr;
  pid_t pid;
  int i = 0;

  if (mutex_lock_interruptible(&ds6103_nl_mutex))
    return -ERESTARTSYS;

  list_for_each_entry_safe(ds6103_nl_pid, tmp, &ds6103_nl_pid_list, list)
  {
    ds6103_nl_data = &(ds6103_nl_pid->ds6103_nl_data);
    pid = ds6103_nl_data->pid;

    skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb)
      return -ENOMEM;

    hdr = genlmsg_put(skb, 0 /* from kernel */,
                      ds6103_nl_data->seq, &ds6103_nl_family, 0, ds6103_nl_data->cmd);
    if (!hdr)
      goto out_nlmsg_free;

    /*
     * hdr is first nlattr or user header which size is genl_family->hdrsize,
     * if genl_family->hdrsize is 0, there is no user header
     */

    info.snd_portid = pid; /* to app */
    genl_info_net_set(&info, ds6103_nl_pid->net);

    NETLINK_CB(skb).portid = 0; /* from kernel */

    snprintf(ds6103_nl_data_snd.name, sizeof(ds6103_nl_data_snd.name), "kernel nl");
    ds6103_nl_data_snd.freq_khz = p_channel_info->frequency; //0xaabbccdd;
    ds6103_nl_data_snd.symbol_rate = p_channel_info->symbol_rate;
    ds6103_nl_data_snd.port_type = p_channel_info->port_type;
    ds6103_nl_data_snd.DataTsNumber = p_channel_info->DataTsNumber;
    ds6103_nl_data_snd.ts_id = p_channel_info->ts_id;
    ds6103_nl_data_snd.ts_index = p_channel_info->ts_index;
    for (i = 0; i < p_channel_info->DataTsNumber; i++)
    {
      ds6103_nl_data_snd.DataTsIdArray[i] = p_channel_info->DataTsIdArray[i];
    }

    ds6103_nl_data_snd.pid = pid;
    ds6103_nl_data_snd.seq = ds6103_nl_data->seq + 1;
    ds6103_nl_data_snd.cmd = ds6103_nl_data->cmd;
    ds6103_nl_data_snd.data01 = ds6103_nl_data->data01;

    //NLA_PUT_TYPE(skb, typeof(ds6103_nl_data_snd), ds6103_nl_data_snd.data01, ds6103_nl_data_snd);
    if (nla_put(skb, ds6103_nl_data_snd.data01, sizeof(ds6103_nl_data_snd), &ds6103_nl_data_snd))
      goto nla_put_failure;

    genlmsg_end(skb, hdr);

    genlmsg_reply(skb, &info);

    list_del(&(ds6103_nl_pid->list));
    kfree(ds6103_nl_pid);
  }

  mutex_unlock(&ds6103_nl_mutex);

  return 0;

nla_put_failure:
  genlmsg_cancel(skb, hdr);

out_nlmsg_free:
  nlmsg_free(skb);
  mutex_unlock(&ds6103_nl_mutex);

  return -ENOMEM;
}

static int ds6103_nl_list_del(void)
{
  struct ds6103_netlink_pid_list *ds6103_nl_pid;
  struct ds6103_netlink_pid_list *tmp;

  if (mutex_lock_interruptible(&ds6103_nl_mutex))
    return -ERESTARTSYS;

  list_for_each_entry_safe(ds6103_nl_pid, tmp, &ds6103_nl_pid_list, list)
  {
    list_del(&(ds6103_nl_pid->list));
    kfree(ds6103_nl_pid);
  }

  mutex_unlock(&ds6103_nl_mutex);

  return 0;
}

#if 0
static void pinmux_configure(void)
{
    u32 temp = 0;
    /* NIM */
    _mt_fe_read32_ds6103(0xbf13c010, &temp);
    if ((((temp >> 16) & 0x0f) != 0x02) || (((temp >> 20) & 0x0f) != 0x02))
    {
      temp &= ~(0xF << 16);
      temp &= ~(0xF << 20);
      temp |= (2 << 16);
      temp |= (2 << 20);
      _mt_fe_write32_ds6103(0xbf13c010, temp);
    }

    _mt_fe_read32_ds6103(0xbf138008, &temp);
    if (temp != 0x311)
    {
      temp = 0x311;
      _mt_fe_write32_ds6103(0xbf138008, temp);
    }
}
#endif

mt_unf_fe_fec_type_t port_m88ds6103_deparse_dvb_type(MT_FE_TYPE dvb_type)
{
  mt_unf_fe_fec_type_t temp;

  switch (dvb_type)
  {
  case MtFeType_DvbS:
    temp = MT_UNF_PORT_TYPE_DVBS;
    break;

  case MtFeType_DvbS2:
    temp = MT_UNF_PORT_TYPE_DVBS2;
    break;

  default:
    temp = MT_UNF_PORT_TYPE_DVBS_BUTT; //NIM_UNDEF;
    break;
  }

  return temp;
}

mt_unf_fe_fecrate_t port_m88ds6103_deparse_code_rate(MT_FE_CODE_RATE code_rate)
{
  mt_unf_fe_fecrate_t temp;

  switch (code_rate)
  {
  case MtFeCodeRate_1_4:
    temp = NIM_CR_1_4;
    break;

  case MtFeCodeRate_1_3:
    temp = NIM_CR_1_3;
    break;

  case MtFeCodeRate_2_5:
    temp = NIM_CR_2_5;
    break;

  case MtFeCodeRate_1_2:
    temp = NIM_CR_1_2;
    break;

  case MtFeCodeRate_3_5:
    temp = NIM_CR_3_5;
    break;

  case MtFeCodeRate_2_3:
    temp = NIM_CR_2_3;
    break;

  case MtFeCodeRate_3_4:
    temp = NIM_CR_3_4;
    break;

  case MtFeCodeRate_4_5:
    temp = NIM_CR_4_5;
    break;

  case MtFeCodeRate_5_6:
    temp = NIM_CR_5_6;
    break;

  case MtFeCodeRate_7_8:
    temp = NIM_CR_7_8;
    break;

  case MtFeCodeRate_8_9:
    temp = NIM_CR_8_9;
    break;

  case MtFeCodeRate_9_10:
    temp = NIM_CR_9_10;
    break;

  default:
    temp = NIM_CR_AUTO;
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

static int port_m88ds6103_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
	//MT_U32 for_scan = 0;
	MT_FE_RET ret = 0;
	MT_FE_TYPE dvb_type = MtFeType_Undef;
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	U32 freq_MHz = 1000;
	U32 symbol_rate_KSs = 27500;

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
			//ds6103_handle->demod_type = MtFeType_DvbS;
			dvb_type = MtFeType_DvbS;
		}
		else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
		{
			//ds6103_handle->demod_type = MtFeType_DvbS2;
			dvb_type = MtFeType_DvbS2;
		}
		else
		{
			//ds6103_handle->demod_type = MtFeType_DvbS2;
			dvb_type = MtFeType_DTV_Unknown;
		}

		printk("ds6103_channel set ss2: freq = %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
				para->connect_param.sat.freq,
				para->connect_param.sat.sym_rate,
				p_priv->for_scan,
				para->connect_param.sat.port_type,
				para->connect_param.sat.uc_param.use_uc);

		if (para->connect_param.sat.uc_param.use_uc)
		{
			ds6103_handle->lnb_cfg.bUnicable = 1;
			ds6103_handle->lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
			ds6103_handle->lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
			ds6103_handle->lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
			ds6103_handle->lnb_cfg.iUBVer = para->connect_param.sat.uc_param.ub_ver;
		}
		else
		{
			ds6103_handle->lnb_cfg.bUnicable = 0;
		}

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
		//priv->for_scan = p_channel_set_info->for_scan;

		//para->connect_param.sat.freq / 1000,
		//para->connect_param.sat.sym_rate,
		//dvb_type,
		//p_priv->for_scan
		freq_MHz = (para->connect_param.sat.freq + 500) / 1000;
		symbol_rate_KSs = para->connect_param.sat.sym_rate;

		ds6103_handle->tp_cfg.ucCurTsId = para->connect_param.sat.ts_id;

		ret = mt_fe_dmd_ds6103_connect(ds6103_handle, freq_MHz, symbol_rate_KSs, dvb_type);

		if (ret == MtFeErr_Ok)
		{
			/* set check lock delay time */
			if (dvb_type == MtFeType_DvbS)
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
			else if (dvb_type == MtFeType_DvbS2)
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
			printk("mt_fe_dmd_connect_ss2_ds6103 failed %d\n", ret);
		}
	}

	if (ret == MtFeErr_Ok)
	{
		return MT_SUCCESS;
	}

	return MT_FAILURE;
}

static void port_m88ds6103_set_22k_onoff(void *handle, MT_U8 onoff_22k, MT_U8 diseqc_out_when_lnb_off)
{
	U8 val_0xa1, val_0xa2;
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	ds6103_handle->dmd_get_reg(ds6103_handle, 0xa1, &val_0xa1);
	ds6103_handle->dmd_get_reg(ds6103_handle, 0xa2, &val_0xa2);

	if (onoff_22k == MtFe_True)
	{
		val_0xa1 |= 0x04;
		val_0xa1 &= ~0x03;
		val_0xa1 &= ~0x40;
		val_0xa2 &= ~0xc0;
	}
	else
	{
		if (diseqc_out_when_lnb_off == MT_FE_DS6103_PIN_LEVEL_HIGH)
		{
			val_0xa2 |= 0xc0;
		}
		else
		{
			val_0xa2 &= ~0xc0;
			val_0xa2 |= 0x80;
		}
	}

	ds6103_handle->dmd_set_reg(ds6103_handle, 0xa2, val_0xa2);
	ds6103_handle->dmd_set_reg(ds6103_handle, 0xa1, val_0xa1);

	printk("port_m88ds6103_set_22k_onoff %d - %s\n", onoff_22k, (onoff_22k == MtFe_True) ? "22K On" : "22K Off");
}

static void port_m88ds6103_set_lnb_voltage(void *handle, MT_FE_LNB_VOLTAGE voltage, MT_U8 vsel_pin_13v)
{
	MT_U8 val_0xa2;
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	ds6103_handle->dmd_get_reg(ds6103_handle, 0xa2, &val_0xa2);
	printk("port_m88ds6103_set_lnb_voltage %d - %dV\n", voltage, (voltage == MtFeLNB_13V) ? 13 : 18);

	if (vsel_pin_13v == MT_FE_DS6103_PIN_LEVEL_HIGH)
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

	ds6103_handle->dmd_set_reg(ds6103_handle, 0xa2, val_0xa2);
	val_0xa2 = 0;
	ds6103_handle->dmd_get_reg(ds6103_handle, 0xa2, &val_0xa2);
}

static void port_m88ds6103_set_lnb_onoff(void *handle, MT_U8 lnb_enable, mt_unf_fe_pin_config_para_t *pin_config)
{
	U8 val_0xa1, val_0xa2; //, pin, level, value;
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
	//pinmux_configure();

	ds6103_handle->dmd_get_reg(ds6103_handle, 0xa2, &val_0xa2);

	printk("port_m88ds6103_set_lnb_onoff %d - %s\n", lnb_enable, (lnb_enable == MtFe_True) ? "On" : "Off");

	if (lnb_enable != MtFe_True) // off
	{
		ds6103_handle->dmd_get_reg(ds6103_handle, 0xa1, &val_0xa1);
		val_0xa1 |= 0x40;
		ds6103_handle->dmd_set_reg(ds6103_handle, 0xa1, val_0xa1);

		if (pin_config->lnb_enable_by_mcu == 0)
		{
			/*set LNB_EN pin HIGH or LOW */
			if (pin_config->lnb_enable == MT_FE_DS6103_PIN_LEVEL_HIGH)
			{
				val_0xa2 &= ~0x02;
			}
			else
			{
				val_0xa2 |= 0x02;
			}
			//printk(" port_m88ds6103_set_lnb_onoff line:%d\n", __LINE__);
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
		if (pin_config->vsel_when_lnb_off == MT_FE_DS6103_PIN_LEVEL_HIGH)
		{
			val_0xa2 |= 0x01;
		}
		else
		{
			val_0xa2 &= ~0x01;
		}

		/*set DiseQc_OUT mode*/
		if (pin_config->diseqc_out_when_lnb_off == MT_FE_DS6103_PIN_LEVEL_HIGH)
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
			//printk("port_m88ds6103_set_lnb_onoff line:%d.\n", __LINE__);
			if (pin_config->lnb_enable == MT_FE_DS6103_PIN_LEVEL_HIGH)
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

	ds6103_handle->dmd_set_reg(ds6103_handle, 0xa2, val_0xa2);
}

static int port_m88ds6103_diseqc_sendmsg(void *handle, mt_unf_fe_diseqc_sendmsg_t *p_diseqc_sendmsg)
{
	MT_FE_RET ret = MtFeErr_Undef;

	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
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

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}

	if (p_priv->onoff_22k != 0)
	{
		/* 22k is opened , close it */
		port_m88ds6103_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

		/* make sure the voltage stable */
		//msleep(MT_FE_DS6103_DISEQC_COMMAND_START_DELAY);
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

	printk("DiSEqC send msg[0x%x, 0x%x, 0x%x, 0x%x], send size[%d]\n",
			msg.data_send[0],
			msg.data_send[1],
			msg.data_send[2],
			msg.data_send[3],
			msg.size_send);

	ret = mt_fe_dmd_ds6103_DiSEqC_send_msg(ds6103_handle, &msg);


	if (p_priv->onoff_22k != 0)
	{
		/* make sure the voltage stable */
		//msleep(MT_FE_DS6103_DISEQC_COMMAND_END_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

		/* 22k was closed, open it */
		port_m88ds6103_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
	}

	return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88ds6103_diseqc_send_tone_burst(void *handle, mt_u8 mode)
{
	MT_FE_RET ret = MtFeErr_Undef;

	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	p_priv->cur_diseqc.mode = 0; //PORT_DISEQC_BURST0;//p_diseqc_cmd->mode;

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}

	if (p_priv->onoff_22k != 0)
	{
		/* 22k is opened, close it */
		port_m88ds6103_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

		/* make sure the voltage stable */
		//msleep(MT_FE_DS6103_DISEQC_COMMAND_START_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms
	}

	mode = (mode == 0) ? MtFeDiSEqCToneBurst_Unmoulated : MtFeDiSEqCToneBurst_Moulated;
	ret = mt_fe_dmd_ds6103_DiSEqC_send_tone_burst(ds6103_handle, mode, 0);

	if (p_priv->onoff_22k != 0)
	{
		/* make sure the voltage stable */
		//msleep(MT_FE_DS6103_DISEQC_COMMAND_END_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

		/* 22k was closed, open it */
		port_m88ds6103_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
	}

	return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88ds6103_diseqc_recvmsg(void *handle, mt_unf_fe_diseqc_recvmsg_t *p_diseqc_recvmsg)
{
	MT_FE_RET ret = MtFeErr_Undef;

	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

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
		port_m88ds6103_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

		/* make sure the voltage stable */
		//msleep(MT_FE_DS6103_DISEQC_COMMAND_START_DELAY);
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
	msg.is_enable_receive = 1;
#endif

	ret = mt_fe_dmd_ds6103_DiSEqC_receive_msg(ds6103_handle, &msg);
	printk("DiSEqC recv msg[0x%x, 0x%x, 0x%x, 0x%x] recv size[%d]\n",
			msg.data_receive[0],
			msg.data_receive[1],
			msg.data_receive[2],
			msg.data_receive[3],
			msg.size_receive);

	p_diseqc_recvmsg->len = msg.size_receive;
	memcpy(p_diseqc_recvmsg->msg, msg.data_receive, p_diseqc_recvmsg->len);
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
		//msleep(MT_FE_DS6103_DISEQC_COMMAND_START_DELAY);
		usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

		/* 22k was colsed, open it */
		port_m88ds6103_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
	}

	return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88ds6103_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
	MT_FE_LOCK_STATE stat = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		if (ss2_status != MtFeLockState_Locked)
		{
			mt_fe_dmd_ds6103_get_lock_state(ds6103_handle, &stat);
		}
		else
		{
			mt_fe_dmd_ds6103_get_pure_lock(ds6103_handle, &stat);
		}

		if (stat == MtFeLockState_Locked)
		{
			int iCnt = 0, index = 0;

			if (ss2_status != stat)
				ss2_status = stat;

			p_status->param.connect_param.sat.DataTsNumber = ds6103_handle->tp_cfg.iTsCnt;
			p_status->param.connect_param.sat.ts_id = ds6103_handle->tp_cfg.ucCurTsId;

			for (iCnt = 0; iCnt < ds6103_handle->tp_cfg.iTsCnt; iCnt ++)
			{
				p_status->param.connect_param.sat.DataTsIdArray[iCnt] = ds6103_handle->tp_cfg.ucTsId[iCnt];

				if (ds6103_handle->tp_cfg.ucTsId[iCnt] == ds6103_handle->tp_cfg.ucCurTsId)
					index = iCnt;
			}

			p_status->param.connect_param.sat.ts_index = index;
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

static int port_m88ds6103_get_signal_quality(void *handle, MT_U32 *p_quality)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
	int ret = 0;
	U8 percent = 0;

	// For DVB-S or DVB-S2
	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		ret = mt_fe_dmd_ds6103_get_sat_quality(ds6103_handle, &percent);
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	*p_quality = percent;

	return MT_SUCCESS;
}

static int port_m88ds6103_get_ber(void *handle, MT_U32 *p_ber)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
	U32 err_packages = 0;
	U32 total_packages = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))// For DVB-S or DVB-S2
	{
		mt_fe_dmd_ds6103_get_per(ds6103_handle, &total_packages, &err_packages);
	}

	p_ber[0] = total_packages;
	p_ber[1] = err_packages;
	p_ber[2] = 0;

	return MT_SUCCESS;
}

static int port_m88ds6103_get_snr(void *handle, MT_U32 *p_snr)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
	S8 _snr = 0;
	int ret = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))// For DVB-S or DVB-S2
	{
		ret = mt_fe_dmd_ds6103_get_snr(ds6103_handle, &_snr);
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	*p_snr = _snr;

	return MT_SUCCESS;
}

static int port_m88ds6103_get_signal_strength(void *handle, MT_U32 *p_strength)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
	S8 _strength = 0;
	int ret = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))// For DVB-S or DVB-S2
	{
		ret = mt_fe_dmd_ds6103_get_strength(ds6103_handle, &_strength);
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	*p_strength = _strength;

	return MT_SUCCESS;
}

static void port_m88ds6103_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	if ((p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
		(p_sig_info->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		MT_FE_CHAN_INFO_DVBS2 ch_info;

		S32 tuner_offset_KHz = 0, carrieroffset_KHz = 0;
		U32 sym_rate_KSs = 27500;

		_mt_fe_dmd_ds6103_get_carrier_offset(ds6103_handle, &carrieroffset_KHz);

		//tuner_offset_KHz = mt_fe_tn_get_tuner_freq_offset_ds6103(ds6103_handle);
		if (ds6103_handle->tuner_cfg.tuner_get_offset != NULL)
			tuner_offset_KHz = ds6103_handle->tuner_cfg.tuner_get_offset(ds6103_handle);

		p_sig_info->sig_info.sat.freq = ds6103_handle->tp_cfg.iFreqKHz + tuner_offset_KHz - carrieroffset_KHz;

		//printk("%s[%d] -- SS2 freq[%d], carrier offset[%d], tuner_offset[%d], result[%d]\n", __FUNCTION__, __LINE__, 
		//        ct8k_handle->tp_cfg.iFreqKHz, carrieroffset_KHz, tuner_offset_KHz, p_sig_info->sig_info.sat.freq);

		_mt_fe_dmd_ds6103_get_sym_rate(ds6103_handle, &sym_rate_KSs);
		p_sig_info->sig_info.sat.symbol_rate = sym_rate_KSs;


		//printk("%s[%d] -- FE[%d], type[%d], SS2 freq[%d], sym_rate[%d]\n", __FUNCTION__, __LINE__, 
		//        gFeIndex, gFeList[gFeIndex], p_sig_info->sig_info.sat.freq, p_sig_info->sig_info.sat.symbol_rate);

		mt_fe_dmd_ds6103_get_channel_info(ds6103_handle, &ch_info);

		if (ds6103_handle->tp_cfg.mCurrentType == MtFeType_DvbS)
		{
			p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_SAT;
			p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_DVBS;
		}
		else if (ds6103_handle->tp_cfg.mCurrentType == MtFeType_DvbS2)
		{
			p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_SAT_2;
			p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_DVBS2;
		}
		else
		{
			p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
			p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_BUTT;
		}


		switch(ch_info.mod_mode)
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

		switch(ch_info.code_rate)
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
	}

	return;
}

static int port_m88ds6103_get_signal_agc(void *handle, mt_u32 *p_agc)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
	int ret = 0;
	mt_u32 tuner_gain = 0, strength = 0;

	if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		if (ds6103_handle->tuner_cfg.tuner_get_strength != NULL)
			ds6103_handle->tuner_cfg.tuner_get_strength(ds6103_handle, &tuner_gain, &strength);
		//ret = mt_fe_tn_get_gain_ds6103(ds6103_handle, &tuner_gain);

		if (ret < 0)
		{
			return MT_FAILURE;
		}

		p_agc[0] = tuner_gain;
		printk("drv tuner_gain[%d]\n", tuner_gain);
	}

	return MT_SUCCESS;
}

static int port_m88ds6103_standby(void *handle)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	mt_fe_dmd_ds6103_sleep(ds6103_handle);

	return MT_SUCCESS;
}

static int port_m88ds6103_wakeup(void *handle)
{
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	mt_fe_dmd_ds6103_wake_up(ds6103_handle);

	return MT_SUCCESS;
}

static int port_m88ds6103_get_default_timeout(void *handle, mt_u32 *timeout)
{
	*timeout = 120;

	return MT_SUCCESS;
}

static int port_m88ds6103_set_io(void *handle, MT_BOOL onoff)
{
  return MT_SUCCESS;
}

static int port_m88ds6103_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
	MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

	//MT_FE_RET ret = MtFeErr_Ok;
	//MT_FE_LOCK_STATE status = 0; /*clean warning*/
	//MT_U32 cnt = 0; /*clean warning*/

	//U8 tmp; /*clean warning*/
	MT_U8 for_bs = 0;
	MT_FE_TYPE dvb_type = MtFeType_Undef;
	MT_FE_BS_TP_INFO bs_tpinfo;
	MT_FE_TP_INFO tp_info;
	MT_U8 voltage = 0;
	MT_U8 count;

	p_priv->sig_type = para->sig_type;
	p_channel_info = &(para->channel_info);
	//pinmux_configure();

	//printk("----port_m88ds6103_channel_connect() log1, sig_type = %d\n", para->sig_type);

	if ((para->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
		(para->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
		(para->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
		(para->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
	{
		//if the blindscan is running, stop it first
		count = 0;
		//printk(KERN_ERR "[%s %d]ct8k_handle->global_cfg.bBsStatus=%d\n", __FUNCTION__, __LINE__, ct8k_handle->global_cfg.bBsStatus);
		while (1 == ds6103_handle->global_cfg.bBsStatus)
		{
			ds6103_handle->global_cfg.bCancelBs = TRUE;
			msleep(2);
			count++;
			if (count > 100)
			{
				break;
			}
		}
		
		//printk("%s() %d: para->connect_param.sat.port_type = %d\n", __FUNCTION__, __LINE__, para->connect_param.sat.port_type);
		if (MT_UNF_PORT_TYPE_DVBS == para->connect_param.sat.port_type)
		{
			dvb_type = MtFeType_DvbS;
		}
		else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
		{
			dvb_type = MtFeType_DvbS2;
		}
		else
		{
			dvb_type = MtFeType_DTV_Unknown;
		}

		printk("ds6103 connect: freq = %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
				para->connect_param.sat.freq,
				para->connect_param.sat.sym_rate,
				for_bs,
				para->connect_param.sat.port_type,
				para->connect_param.sat.uc_param.use_uc);

		if (p_priv->lnb_onoff == 0)
		{
			return MT_SUCCESS;
		}

		if (p_priv->lnb_polar != para->connect_param.sat.polarization)
		{
			voltage = (para->connect_param.sat.polarization == PORT_PORLAR_HORIZONTAL) ? MtFeLNB_18V : MtFeLNB_13V;
			port_m88ds6103_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
			p_priv->lnb_polar = (MT_U8)para->connect_param.sat.polarization;
			p_priv->lnb_voltage = voltage;
		}

		if (para->connect_param.sat.uc_param.use_uc)
		{
			ds6103_handle->lnb_cfg.bUnicable = 1;
			ds6103_handle->lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
			ds6103_handle->lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
			ds6103_handle->lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
			printk("drv line[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
					__LINE__,
					ds6103_handle->lnb_cfg.iBankIndex,
					ds6103_handle->lnb_cfg.iUBIndex,
					ds6103_handle->lnb_cfg.iUBFreqMHz);
		}
		else
		{
			ds6103_handle->lnb_cfg.bUnicable = 0;
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

			if (_mt_fe_dmd_ds6103_bs_connect(p_priv->ds6103_handle, &bs_tpinfo, 0, 1) == MtFeErr_Ok)
			{
				if (bs_tpinfo.tp_num > 0)
				{
					p_channel_info->frequency = bs_tpinfo.p_tp_info[0].freq_KHz;
					p_channel_info->symbol_rate = bs_tpinfo.p_tp_info[0].sym_rate_KSs;
					p_channel_info->port_type = port_m88ds6103_deparse_dvb_type(bs_tpinfo.p_tp_info[0].dvb_type);
					p_channel_info->fec_inner = port_m88ds6103_deparse_code_rate(bs_tpinfo.p_tp_info[0].code_rate);
					p_channel_info->lock = 1;
				}
				return MT_SUCCESS;
			}
		}
		else
		{
			p_priv->for_scan = 0;
			para->channel_set_info.for_scan = 0;
			port_m88ds6103_channel_set(p_priv, para);
#if 0
			//printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
			for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
			{
				mt_fe_dmd_ds6103_get_lock_state(ds6103_handle, &status);
				para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
				if (para->channel_info.lock)
				{
					//printk("ds6103 connect lock success\n");
					return MT_SUCCESS;
				}
				//_mt_sleep_ds6103(10);
				msleep(10);
			}

#if 1
			printk("%s[%d] ---- Unlock! Now dump all tuner registers\n", __FUNCTION__, __LINE__);

			for (cnt = 0; cnt < 0x100; cnt++)
			{
				ds6103_handle->tn_get_reg(ds6103_handle, (U8)cnt, &tmp);
				printk("\t%02x - %02x\n", (U8)cnt, tmp);
			}

			printk("\n");

			printk("%s[%d] ---- Unlock! Now dump all demod registers\n", __FUNCTION__, __LINE__);

			for (cnt = 0; cnt < 0x100; cnt++)
			{
				ds6103_handle->dmd_get_reg(ds6103_handle, (U8)cnt, &tmp);
				printk("\t%02x - %02x\n", (U8)cnt, tmp);
			}

			printk("\n");
#endif

			//return ERR_TIMEOUT;
#endif
			return MT_SUCCESS;
		}
	}

	return MT_FAILURE;
}

static int port_m88ds6103_ioctl(void *handle, mt_u32 cmd, ulong param)
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
  S32 tuner_offset_KHz = 0, carrieroffset_KHz = 0;
  U32 sym_rate_KSs = 27500;


  mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
  MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;
  //pinmux_configure();

  //printk("%s[%d]: cmd %d - param 0x%08x\n", __FUNCTION__, __LINE__, cmd, param);

  switch (cmd)
  {
  case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
    mt_fe_dmd_ds6103_get_lock_state(ds6103_handle, &status);

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
    port_m88ds6103_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
    p_priv->lnb_polar = (MT_U8)param;
    p_priv->lnb_voltage = voltage;
    break;

  case NIM_IOCTRL_SET_LNB_ONOFF:
    //printk("ker ds6103 line:%d param=0x%08x\n", __LINE__, param);

#if 0
    if (p_priv->lnb_onoff == param)
    {
      break;
    }
#endif

    //printk("NIM_IOCTRL_SET_LNB_ONOFF set %d\n", param);
    port_m88ds6103_set_lnb_onoff(handle, (MT_U8)param, &p_priv->cfg.pin_config);
    p_priv->lnb_onoff = param;
    if (param == 0)
    {
      break;
    }

    /* restore voltage */
    port_m88ds6103_set_lnb_voltage(handle, p_priv->lnb_voltage, p_priv->cfg.pin_config.vsel_when_13v);
    /* restore 22k */
    //if (p_priv->onoff_22k != 0)		// 181219
    {
      port_m88ds6103_set_22k_onoff(handle, p_priv->onoff_22k, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
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
      //port_m88ds6103_diseqc_ctrl(handle, &diseqc_cmd);
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
      //port_ds6103_lnb_sc_restore(handle, &p_priv->cfg.pin_config);
    }
    break;

  case NIM_IOCTRL_REMOVE_PROTECT:
    if (1 == p_priv->cfg.lnb_prot_by_mcu)
    {
      //port_ds6103_lnb_sc_remove(&p_priv->cfg.pin_config);
    }
    break;

  case NIM_IOCTRL_ENABLE_CHECK_PROTECT:
    if (1 == p_priv->cfg.lnb_prot_by_mcu)
    {
      //port_ds6103_lnb_sc_chk_enable(&p_priv->cfg.pin_config);
    }
    break;
#endif

  case NIM_IOCTRL_SET_22K_ONOFF:
    //printk("ker ds6103 ioctl line:%d param=0x%08x\n", __LINE__, param);
    if (p_priv->onoff_22k == param)
    {
      break;
    }
    port_m88ds6103_set_22k_onoff(handle, (MT_U8)param, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
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
    //port_ds6103_recover(handle);
    break;

  case NIM_IOCTRL_SET_CHANNEL_INFO:
    //rc = port_ds6103_set_chninfo(handle, param);
    return rc;
    break;
#endif

  case NIM_IOCTRL_GET_SIGNAL_INFO:
    port_m88ds6103_get_signal_info(handle, (mt_unf_fe_signal_info_t *)param);
    break;

#if 0
  case NIM_IOCTRL_GET_CHANNEL_INFO:
    if (param != 0)
      memcpy((MT_UNF_FE_CHANNEL_INFO_S*)param, &p_priv->cur_channel, sizeof(MT_UNF_FE_CHANNEL_INFO_S));
    break;
#endif

  case NIM_IOCTRL_SCAN_CANCEL:
    port_m88ds6103_blind_scan_cancel(p_priv);
    break;

  case NIM_IOCTRL_GET_SCAN_STATUS:
    //mutex_lock(&bs_notify_status_lock);
    if (param != 0)
      *((MT_U8 *)param) = ds6103_notify_scan_status;
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
    port_m88ds6103_diseqc_sendmsg(handle, p_sendmsg);
    break;

  case NIM_IOCTRL_DISEQC_SEND_TONEBURST:
    //p_diseqc_cmd = (mt_unf_fe_diseqc_sendmsg_t*)param;
    port_m88ds6103_diseqc_send_tone_burst(handle, param);
    break;

  case NIM_IOCTRL_DISEQC_RECVMSG:
    p_recvmsg = (mt_unf_fe_diseqc_recvmsg_t *)param;
    port_m88ds6103_diseqc_recvmsg(handle, p_recvmsg);
    break;
    //#endif

  case NIM_IOCTRL_SAT_BS_EVENT_PROCESSED:
    printk("%s[%d]: NIM_IOCTRL_SAT_BS_EVENT_PROCESSED\n", __FUNCTION__, __LINE__);
#ifdef CFG_SYNC_BLINDSCAN
    {
      event_status = 1;
      wake_up_interruptible(&bs_cb_wq);
    }
#endif
    break;

  case NIM_IOCTRL_GET_SAT_REAL_FREQ:
    _mt_fe_dmd_ds6103_get_carrier_offset(ds6103_handle, &carrieroffset_KHz);

    //tuner_offset_KHz = mt_fe_tn_get_tuner_freq_offset_ds6103(ds6103_handle);
    if (ds6103_handle->tuner_cfg.tuner_get_offset != NULL)
      tuner_offset_KHz = ds6103_handle->tuner_cfg.tuner_get_offset(ds6103_handle);

    if (param != 0)
      *((MT_U32 *)param) = ds6103_handle->tp_cfg.iFreqKHz + tuner_offset_KHz - carrieroffset_KHz;

    break;

  case NIM_IOCTRL_GET_SAT_REAL_SYM:
    _mt_fe_dmd_ds6103_get_sym_rate(ds6103_handle, &sym_rate_KSs);

    if (param != 0)
      *((MT_U32 *)param) = sym_rate_KSs;
    break;

  case NIM_IOCTRL_GET_SAT_FREQ_OFFSET:
    _mt_fe_dmd_ds6103_get_carrier_offset(ds6103_handle, &carrieroffset_KHz);

    //tuner_offset_KHz = mt_fe_tn_get_tuner_freq_offset_ds6103(ds6103_handle);
    if (ds6103_handle->tuner_cfg.tuner_get_offset != NULL)
      tuner_offset_KHz = ds6103_handle->tuner_cfg.tuner_get_offset(ds6103_handle);

    if (param != 0)
      *((MT_S32 *)param) = tuner_offset_KHz - carrieroffset_KHz;
    break;

  case NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_CNT:
    mt_fe_dmd_ds6103_get_lock_state(ds6103_handle, &status);

    if (status == MtFeLockState_Locked)
    {
      if (param != 0)
        *((MT_U8 *)param) = ds6103_handle->tp_cfg.iTsCnt;
    }
    else
    {
      if (param != 0)
        *((MT_U8 *)param) = 0;
    }
    break;

  case NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_LIST:
    {
      int iCnt = 0;

      mt_fe_dmd_ds6103_get_lock_state(ds6103_handle, &status);

      if (status == MtFeLockState_Locked)
      {
        for (iCnt = 0; iCnt < ds6103_handle->tp_cfg.iTsCnt; iCnt++)
        {
          ((MT_U8 *)param)[iCnt] = ds6103_handle->tp_cfg.ucTsId[iCnt];
        }
      }
    }
    break;

  case NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID:
    mt_fe_dmd_ds6103_set_ts(ds6103_handle, 0, (U8)param);
    //printk("%s[%d] ---- NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID: ts_id = %d\n", __FUNCTION__, __LINE__, param);
    break;

  case NIM_IOCTRL_SET_SUPER_SEARCH:
    ds6103_handle->tp_cfg.bSuperSearch = (param != 0) ? TRUE : FALSE;
    //printk("%s[%d] ---- Switch super search or blindscan mode -- %s\n", __FUNCTION__, __LINE__, (param != 0) ? "ON" : "OFF");
    break;

  case NIM_IOCTRL_UNICABLE_RETRY:
    mt_fe_dmd_ds6103_unicable_retry(ds6103_handle);
    break;

  case NIM_IOCTRL_SUSPEND:
    m88ds6103_suspend();
    break;

  case NIM_IOCTRL_RESUME:
    m88ds6103_resume();
    break;

  default:
    break;
  }

  return MT_SUCCESS;
}

static int port_m88ds6103_blind_scan_cancel(void *handle)
{
  mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
  MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

  MT_U8 count=0;

  printk("port_m88ds6103_blind_scan_cancel\n");
  mt_fe_dmd_ds6103_blindscan_abort(ds6103_handle, MtFe_True);

  //wait blind scan to stop(bBsStatus:0:not start or has finished, 1:runnig)
  while (1 == ds6103_handle->global_cfg.bBsStatus)
  {
    ds6103_handle->global_cfg.bCancelBs = TRUE;
    msleep(2);
    count++;
    if (count > 100)
    {
      break;
    }
  }

  event_status = 1;
  wake_up_interruptible(&bs_cb_wq);

  return MT_SUCCESS;
}

static const struct nla_policy ds6103_policy[DS6103_NL_FAMILY_ATTR_AMOUNT] = {
    [DS6103_NL_ATTR_DATA0] = {.type = NLA_BINARY, .len = sizeof(struct ds6103_netlink_data)},
};

static int ds6103_nl_recvmsg0(struct sk_buff *skb, struct genl_info *info)
{
  struct ds6103_netlink_data *ds6103_nl_data;
  struct ds6103_netlink_pid_list *ds6103_nl_pid;
  int err = 0;

  //printk("%s\n", __FUNCTION__);

  if (info->attrs[DS6103_NL_ATTR_DATA0])
  {
    ds6103_nl_data = nla_data(info->attrs[DS6103_NL_ATTR_DATA0]);
    ds6103_nl_pid = kzalloc(sizeof(struct ds6103_netlink_pid_list), GFP_KERNEL);
    if (ds6103_nl_pid == NULL)
      return MtFeErr_NoMemory;
    ds6103_nl_pid->net = sock_net(skb->sk);
    /* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
    ds6103_nl_pid->ds6103_nl_data = *ds6103_nl_data;
    //printk("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
    //printk("netlink : pid = %d\n", ds6103_nl_data->pid);
    //printk("netlink : seq = 0x%x\n", ds6103_nl_data->seq);
    //printk("netlink : cmd = %d\n", ds6103_nl_data->cmd);
    //printk("netlink : data01 = %d\n", ds6103_nl_data->data01);
    //printk("netlink : name : %s, freq = %u, sym = %u\n", ds6103_nl_data->name, ds6103_nl_data->freq_khz, ds6103_nl_data->symbol_rate);

    if (mutex_lock_interruptible(&ds6103_nl_mutex))
    {
      kfree(ds6103_nl_pid);
      return -ETIME;
    }

    list_add_tail(&(ds6103_nl_pid->list), &ds6103_nl_pid_list);
    mutex_unlock(&ds6103_nl_mutex);
  }

  return err;
}

static struct genl_ops ds6103_nl_ops[DS6103_NL_OPS_AMOUNT] = {
    [DS6103_NL_OPS_CMD0] = {
        .cmd = DS6103_NL_OPS_CMD0,
        .doit = ds6103_nl_recvmsg0,
        .policy = ds6103_policy,
    },
};

static int port_m88ds6103_blind_scan_start(void *pArg)
{
  S32 ret = 0;
  U32 i = 0;
  MT_FE_DS6103_Device_Handle ds6103_handle = g_ds6103_handle;
  fe_blindscan_param_t *p_scan_info = (fe_blindscan_param_t *)pArg;

  ds6103_handle = (MT_FE_DS6103_Device_Handle)g_ds6103_priv->ds6103_handle;

  if (dvbs_nl_family.id > 0)
  {
    ret = genl_unregister_family(&dvbs_nl_family);
  }

  ret = genl_register_family(&ds6103_nl_family);
  if (ret)
  {
    printk("genl_register_family failed, ret = 0x%08x\n", ret);

    return -MT_FAILURE;
  }

  memcpy(&dvbs_nl_family, &ds6103_nl_family, sizeof(ds6103_nl_family));

#if 0
  printk("ker ds6103: start-stop - start freq: %d, end_freq: %d\n",
         p_scan_info->start_freq, p_scan_info->stop_freq);
  printk("ker ds6103: use_uc[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
         p_scan_info->uc_param.use_uc,
         p_scan_info->uc_param.bank,
         p_scan_info->uc_param.user_band,
         p_scan_info->uc_param.ub_freq_mhz);
#endif

  ds6103_handle->lnb_cfg.bUnicable = p_scan_info->uc_param.use_uc;
  ds6103_handle->lnb_cfg.iBankIndex = p_scan_info->uc_param.bank;
  ds6103_handle->lnb_cfg.iUBIndex = p_scan_info->uc_param.user_band;
  ds6103_handle->lnb_cfg.iUBFreqMHz = p_scan_info->uc_param.ub_freq_mhz;
  ds6103_handle->lnb_cfg.iUBVer = p_scan_info->uc_param.ub_ver;
  ds6103_handle->lnb_cfg.bSpectrumInverted = p_scan_info->invert_spectrum;

  g_ds6103_priv->bs_info.tp_num = 0;

  //nim_lock(priv->drv_base);

  ret = mt_fe_dmd_ds6103_blindscan(ds6103_handle, 
                                   p_scan_info->start_freq / 1000, 
                                   p_scan_info->stop_freq / 1000, 
                                   &g_ds6103_priv->bs_info);

  //nim_unlock(priv->drv_base);

  //printk("nim_ds6103_service end. tp start %d end %d ret = %d\n",
  //g_ds6103_priv->scan_info.start_freq, g_ds6103_priv->scan_info.end_freq, ret);

  if (ret == MtFeErr_Ok)
  {
    p_scan_info = &g_ds6103_priv->scan_info;
    p_scan_info->channel_num_cur = g_ds6103_priv->bs_info.tp_num;
    //printk("port_m88ds6103_blind_scan get %u tp:\n", g_ds6103_priv->bs_info.tp_num);

    /* Bug 108019 */
    p_scan_info->channel_num_total = 0;

    for (i = 0; i < g_ds6103_priv->bs_info.tp_num; i++)
    {
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency =
          g_ds6103_priv->bs_info.p_tp_info[i].freq_KHz;
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate =
          g_ds6103_priv->bs_info.p_tp_info[i].sym_rate_KSs;
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type =
          port_m88ds6103_deparse_dvb_type(g_ds6103_priv->bs_info.p_tp_info[i].dvb_type);
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].fec_inner =
          port_m88ds6103_deparse_code_rate(g_ds6103_priv->bs_info.p_tp_info[i].code_rate);
      //printk("freq: %u, symrate:%u, type:%u\n",
      //p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency,
      //p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate,
      //p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type);
      p_scan_info->channel_num_total += 1;
    }

    //if you scan by dividing all channel to several "channel group", and then scan group by group
    //you should use p_scan_info->channel_num_cur to count scanned tp num for every scan.
    printk("total channel_num_cur: %u channel_num_total:%d\n", p_scan_info->channel_num_cur, p_scan_info->channel_num_total);
    ds6103_handle->global_cfg.bBsStatus = 0;
    return MT_SUCCESS;
  }

  ds6103_handle->global_cfg.bBsStatus = 0;

  return MT_FAILURE;
}

static int port_m88ds6103_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info)
{
  mt_fe_ds6103_priv_handle p_priv = (mt_fe_ds6103_priv_handle)handle;
  MT_FE_DS6103_Device_Handle ds6103_handle = p_priv->ds6103_handle;

  //mutex_lock(&bs_notify_status_lock);
  ds6103_notify_scan_status = 0;
  //mutex_unlock(&bs_notify_status_lock);

  ds6103_handle->global_cfg.bBsStatus = 1;		//start to blind scan
  ds6103_handle->global_cfg.bCancelBs = FALSE;	//not cancel

  p_priv->sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 42)
  kernel_thread(port_m88ds6103_blind_scan_start, p_scan_info, CLONE_KERNEL);
#else
  kthread_run(port_m88ds6103_blind_scan_start, (void *)p_scan_info, "port_m88ds6103_blind_scan");
#endif

  return MT_SUCCESS;
}

void port_m88ds6103_notify_to_up_layer(MT_FE_MSG msg, void *p_param)
{
  //U16 i = 0;
  mt_unf_fe_blind_scan_channel_info_t *p_bs_channel_info = NULL;

  if (p_param != NULL)
    p_bs_channel_info = (mt_unf_fe_blind_scan_channel_info_t *)p_param;

  if (msg == MtFeMsg_BSFinish || msg == MtFeMsg_BSAbort)
  {
    ds6103_notify_scan_status = 1;
    printk("ds6103_notify_scan_status = %d\n", ds6103_notify_scan_status);
  }
}

static void port_m88ds6103_register_to_drv_notify(MT_FE_MSG msg, void *p_param)
{
  MT_FE_TP_INFO *p_tp_info = p_param;
  //nim_channel_info_t channel_info;
  mt_unf_fe_blind_scan_channel_info_t bs_channel_info = {0};
  //S8 snr;
  //S8 s_strength;
  //unsigned int total_packages = 1;
  //unsigned int err_packages = 0;
  mt_unf_fe_channel_info_t channel_info = {0};
  int error;
  MT_FE_BS_TP_INFO *p_bs_info;
  int i, index = 0;

  //MT_FE_DS6103_Device_Handle ds6103_handle = g_ds6103_priv->ds6103_handle;

  /*
  if (port_m88ds6103_notify_to_up_layer == NULL)
  {
    return;
  }
  */

  //printk("ds6103_notify_function: msg ");

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
#if 0//def CFG_SYNC_BLINDSCAN
    init_waitqueue_head(&bs_cb_wq);

    event_status = 0;

    printk("%s[%d] -- TpFind, event_status = 0\n", __FUNCTION__, __LINE__);
#endif
    printk("BSTpFind: freq[%7d] sym[%5d]\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs);
    channel_info.frequency = p_tp_info[0].freq_KHz;
    channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    channel_info.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
    ds6103_nl_sendmsg(&channel_info);

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
#if 0//def CFG_SYNC_BLINDSCAN
    init_waitqueue_head(&bs_cb_wq);

    event_status = 0;
#endif
    printk("BSTpUnlock: freq[%7d] sym[%5d]\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs);
    bs_channel_info.frequency = p_tp_info[0].freq_KHz;
    bs_channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    bs_channel_info.lock = 0;
#if 0
    mt_fe_dmd_ds6103_get_strength(ds6103_handle, &s_strength);
    bs_channel_info.perf.agc =  s_strength;
    mt_fe_dmd_ds6103_get_sat_quality(ds6103_handle, &snr);
    bs_channel_info.perf.snr = snr;
    mt_fe_dmd_ds6103_get_per(ds6103_handle, &total_packages, &err_packages);
    bs_channel_info.perf.ber = (double)err_packages/(double)total_packages;
#endif
    port_m88ds6103_notify_to_up_layer(MtFeMsg_BSTpUnlock, &bs_channel_info);

    channel_info.frequency = p_tp_info[0].freq_KHz;
    channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    channel_info.port_type = MT_UNF_PORT_TYPE_DVBS_BUTT;
    ds6103_nl_sendmsg(&channel_info);

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
    bs_channel_info.nim_type = nim_ds6103_deparse_dvb_type(p_tp_info[0].dvb_type);
    bs_channel_info.param.dvbs.fec_inner = nim_ds6103_deparse_code_rate(p_tp_info[0].code_rate);
    mt_fe_dmd_ds6103_get_strength(ds6103_handle, &s_strength);
    bs_channel_info.param.dvbs.perf.agc =  s_strength;
    mt_fe_dmd_ds6103_get_sat_quality(ds6103_handle, &snr);
    bs_channel_info.param.dvbs.perf.snr = snr;
    mt_fe_dmd_ds6103_get_per(ds6103_handle, &total_packages, &err_packages);
    bs_channel_info.param.dvbs.perf.ber = (double)err_packages/(double)total_packages;
    */
    port_m88ds6103_notify_to_up_layer(MtFeMsg_BSTpLocked, &bs_channel_info);
    printk("BSTpLocked: freq[%7d] sym[%5d] %s\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs, (p_tp_info[0].dvb_type == MtFeType_DvbS2) ? "DVB-S2" : "DVB-S");

    ss2_status = MtFeLockState_Locked;

#ifdef CFG_SYNC_BLINDSCAN
    //init_waitqueue_head(&bs_cb_wq);//init it in attach now

    event_status = 0;
#endif

    //kobject_uevent();
    channel_info.frequency = p_tp_info[0].freq_KHz;
    channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    channel_info.port_type = port_m88ds6103_deparse_dvb_type(p_tp_info[0].dvb_type);

    channel_info.DataTsNumber = p_tp_info[0].iTsCnt;
    channel_info.ts_id = p_tp_info[0].ucCurTsId;

    for (i = 0; i < p_tp_info[0].iTsCnt; i++)
    {
      channel_info.DataTsIdArray[i] = p_tp_info[0].ucTsId[i];

      if (p_tp_info[0].ucTsId[i] == p_tp_info[0].ucCurTsId)
        index = i;
    }

    channel_info.ts_index = index;

    ds6103_nl_sendmsg(&channel_info);

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
    ds6103_nl_list_del();
    port_m88ds6103_notify_to_up_layer(MtFeMsg_BSAbort, NULL);
    break;

  case MtFeMsg_BSFinish:
    printk("BS Finished\n");

    p_bs_info = (MT_FE_BS_TP_INFO *)p_param;

    printk("Blind scan result, total %d TPs:\n", p_bs_info->tp_num);

    for (i = 0; i < p_bs_info->tp_num; i++)
    {
      printk("TP %2d -- %7d KHz, %5d KSs, %s\n", i + 1, p_bs_info->p_tp_info[i].freq_KHz, p_bs_info->p_tp_info[i].sym_rate_KSs, (p_bs_info->p_tp_info[i].dvb_type == MtFeType_DvbS2) ? "DVB-S2" : "DVB-S");
    }

    printk("\n");

    ds6103_nl_list_del();
    port_m88ds6103_notify_to_up_layer(MtFeMsg_BSFinish, NULL);
    break;

  default:
    break;
  }
}


#ifdef CONFIG_NET
int m88ds6103_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
	mt_u8 reg_val = 0;
	int ret = 0;
	mt_u32 temp = 0;
	mt_fe_ds6103_priv_handle p_priv = NULL;

	//printk(KERN_ERR "[%s %d]g_dev_init_flag=%d\n", __FUNCTION__, __LINE__, g_dev_init_flag);

	if (info->is_attach)
	{
		printk(KERN_ERR "[%s %d]has attach\n", __FUNCTION__, __LINE__);
		return MT_SUCCESS;
	}

	info->ops.connect = port_m88ds6103_channel_connect;
	info->ops.get_status = port_m88ds6103_get_status;
	info->ops.get_ber = port_m88ds6103_get_ber;
	info->ops.get_snr = port_m88ds6103_get_snr;
	info->ops.get_signal_strength = port_m88ds6103_get_signal_strength;
	info->ops.get_signal_quality = port_m88ds6103_get_signal_quality;
	info->ops.get_signal_agc = port_m88ds6103_get_signal_agc;

	info->ops.standby = port_m88ds6103_standby;
	info->ops.wakeup = port_m88ds6103_wakeup;
	info->ops.get_default_timeout = port_m88ds6103_get_default_timeout;
	info->ops.set_io = port_m88ds6103_set_io;
	info->ops.port_ioctl = port_m88ds6103_ioctl;
	info->ops.blind_scan = port_m88ds6103_blind_scan;

	if (!g_dev_init_flag)
	{
		switch (symphony_get_chip_rev())
		{
			case CHIP_SYMPHONY_A0:
			case CHIP_SYMPHONY_A1:
			case CHIP_SYMPHONY_A2:
			case CHIP_SYMPHONY3_A0:
				printk("%s[%d] -- Symphony1 or Symphony3 + DS6103\n", __FUNCTION__, __LINE__);
				break;

			case CHIP_SYMPHONY2_A0:
			case CHIP_SYMPHONY2_A1:
			case CHIP_SYMPHONY2_A2:
			case CHIP_SYMPHONY2_A3:
				printk("%s[%d] -- Symphony2 + DS6103\n", __FUNCTION__, __LINE__);

				/*read 0xbf5b0c00 before demod start working*/
				_mt_fe_read32_ds6103(0xbf5b0c00, &temp);

				/*apb address*/
				_mt_fe_read32_ds6103(0xbf138010, &temp);
				temp &= ~(0x3 << 0);
				_mt_fe_write32_ds6103(0xbf138010, temp);

				/*config demod mux*/
				_mt_fe_read32_ds6103(0xbf138020, &temp);
				temp &= ~(1 << 0);
				_mt_fe_write32_ds6103(0xbf138020, temp);

				/*config tuner i2c master pinmux*/
				_mt_fe_read32_ds6103(0xbf13c010, &temp);
				temp &= ~(0x0f << 16);
				temp &= ~(0x0f << 20);
				_mt_fe_write32_ds6103(0xbf13c010, temp);


				// 1. S2x_ts1_clk输入SW_PIN2_SEL BIT[19:16]=4,BIT[23:20]=4;对应寄存器：BF13C008.
				// 2. S2x_ts0_clk输入SW_PIN2_SEL BIT[15:12]=0,BIT[27:24]=0;对应寄存器：BF13C008.

				_mt_fe_read32_ds6103(0xbf13c008, &temp);
				temp &= ~(0xffff << 12);
				temp |= (0x04 << 16);
				temp |= (0x04 << 20);
				_mt_fe_write32_ds6103(0xbf13c008, temp);

				// enable ext TS0, 0xbf138008[12] = 0
				// enable ext TS3, 0xbf138008[20] = 0
				_mt_fe_read32_ds6103(0xbf138008, &temp);
				temp &= ~(0x01 << 20);
				temp &= ~(0x01 << 12);
				_mt_fe_write32_ds6103(0xbf138008, temp);

				// for 88 pin config
				_mt_fe_read32_ds6103(0xbf5d009c, &temp);
				temp &= ~0x3000000; // bit[25:24] = 0
				_mt_fe_write32_ds6103(0xbf5d009c, temp);

				_mt_fe_read32_ds6103(0xbf157000, &temp);
				temp &= ~0x0300000; // bit[21:20] = 0
				_mt_fe_write32_ds6103(0xbf157000, temp);

				break;

			case CHIP_SYMPHONY4_A0:
			case CHIP_SYMPHONY4_A1:
				printk("%s[%d] -- Symphony4 + DS6103\n", __FUNCTION__, __LINE__);

#if 0	// TS2 pinmux reference
				//BF13C048[3:0]=5//TS2_CLK
				_mt_fe_read32_ds6103(0xbf13c048, &temp);
				temp &= ~0x0F;
				temp |= 0x05;
				_mt_fe_write32_ds6103(0xbf13c048, temp);

				//BF5D0094[3]=1  //IF PAD设置为digital input
				_mt_fe_read32_ds6103(0xbf5d0094, &temp);
				temp |= 0x08;
				_mt_fe_write32_ds6103(0xbf5d0094, temp);

				//BF13C1A0[2:0]=1 //TS2_SYNC pin配置为GPIO
				_mt_fe_read32_ds6103(0xbf13c1a0, &temp);
				temp &= ~0x07;
				temp |= 0x01;
				_mt_fe_write32_ds6103(0xbf13c1a0, temp);

				//BF13C1A4[2:0]=2 //TS2_DATA
				_mt_fe_read32_ds6103(0xbf13c1a4, &temp);
				temp &= ~0x07;
				temp |= 0x02;
				_mt_fe_write32_ds6103(0xbf13c1a4, temp);

				//BF50B000[0]=1 //open clk
				_mt_fe_read32_ds6103(0xbf50b000, &temp);
				temp |= 0x01;
				_mt_fe_write32_ds6103(0xbf50b000, temp);

				//BF50B004[6:5]=00 //clk正沿
				_mt_fe_read32_ds6103(0xbf50b004, &temp);
				temp &= ~0x60;
				_mt_fe_write32_ds6103(0xbf50b004, temp);

				//BF50B00C[5]=1  //rest off
				_mt_fe_read32_ds6103(0xbf50b00c, &temp);
				temp |= 0x20;
				_mt_fe_write32_ds6103(0xbf50b00c, temp);

				//BF138008[0]=0 //TS2 input
				_mt_fe_read32_ds6103(0xbf138008, &temp);
				temp &= ~0x01;
				_mt_fe_write32_ds6103(0xbf138008, temp);

				//BF200020[31:0]=0xC7D801FF//TC2 CTR
				_mt_fe_write32_ds6103(0xbf200020, 0x0);
				_mt_fe_write32_ds6103(0xbf200020, 0xc7d801ff);

#elif 0	// TS3 Parallel
				// DEBUGB 0xBF13c124 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c124, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c124, temp);

				// DEBUGB 0xBF13c128 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c128, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c128, temp);

				// DEBUGB 0xBF13c12c 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c12c, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c12c, temp);

				// DEBUGB 0xBF13c130 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c130, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c130, temp);

				// DEBUGB 0xBF13c134 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c134, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c134, temp);

				// DEBUGB 0xBF13c164 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c164, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c164, temp);

				// DEBUGB 0xBF13c174 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c174, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c174, temp);

				// DEBUGB 0xBF13c178 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c178, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c178, temp);

				// DEBUGB 0xBF13c17c 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c17c, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c17c, temp);

				// DEBUGB 0xBF13c180 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c180, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c180, temp);

				// DEBUGB 0xBF13c184 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c184, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c184, temp);

				// DEBUGB 0xBF138008 12 12 1
				_mt_fe_read32_ds6103(0xbf138008, &temp);
				temp |= 0x1000;
				_mt_fe_write32_ds6103(0xbf138008, temp);

				// DEBUGB 0xBF50b000 0 0 1
				_mt_fe_read32_ds6103(0xbf50b000, &temp);
				temp |= 0x01;
				_mt_fe_write32_ds6103(0xbf50b000, temp);

				// DEBUGB 0xBF50b00c 6 6 1
				_mt_fe_read32_ds6103(0xbf50b00c, &temp);
				temp |= 0x40;
				_mt_fe_write32_ds6103(0xbf50b00c, temp);

				// DEBUGB 0xBF50b004 8 7 01
				_mt_fe_read32_ds6103(0xbf50b004, &temp);
				temp &= 0xfffffe7f;
				temp |= 0x00000080;
				_mt_fe_write32_ds6103(0xbf50b004, temp);

				// DEBUG 0xBF200030 0x87c001ff

				_mt_fe_write32_ds6103(0xbf200030, 0x0);
				_mt_fe_write32_ds6103(0xbf200030, 0x87c001ff);

#endif
				break;

			default:
				printk("%s[%d] -- Unknown chip\n", __FUNCTION__, __LINE__);
				break;
		}

		/*read 0xbf5b0c00 before demod start working*/

		//_mt_fe_read32_ds6103(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base()+ 0x00));
	}

	p_priv = kzalloc(sizeof(mt_fe_ds6103_priv_t), GFP_KERNEL);
	if (NULL == p_priv)
	{
		printk(KERN_ERR "%s[%d]kzalloc priv error\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	if (NULL == dev_handle)
	{
		dev_handle = kzalloc(sizeof(MT_FE_DS6103_DEVICE_SETTINGS), GFP_KERNEL);

		if (NULL == dev_handle)
		{
			kfree((void *)p_priv);
			g_ds6103_priv = NULL;
			printk(KERN_ERR "%s[%d]kzalloc dev_handle error\n", __FUNCTION__, __LINE__);
			return -ENOMEM;
		}
	}

	p_priv->ds6103_handle = dev_handle;
	info->handle = (void *)p_priv;
	g_ds6103_priv = p_priv;

	g_ds6103_handle = dev_handle;

	g_i2c_ds6103 = attr->demod_i2c_id;

	if (g_i2c_ds6103 == 1)
	{
		_mt_fe_read32_ds6103(0xbf50f818, &temp);
		temp &= ~0x2000000; // bit[25] = 0
		_mt_fe_write32_ds6103(0xbf50f818, temp);

		printk(KERN_ERR "%s[%d] ---- 0xbf50f818 = 0x%08x\n", __FUNCTION__, __LINE__, temp);

		_mt_fe_read32_ds6103(0xbf13c0fc, &temp);
		temp &= ~0x0F; // bit[3:0] = 2
		temp |= 0x02;
		_mt_fe_write32_ds6103(0xbf13c0fc, temp);

		printk(KERN_ERR "%s[%d] ---- 0xbf13c0fc = 0x%08x\n", __FUNCTION__, __LINE__, temp);

		_mt_fe_read32_ds6103(0xbf13c100, &temp);
		temp &= ~0x0F; // bit[3:0] = 2
		temp |= 0x02;
		_mt_fe_write32_ds6103(0xbf13c100, temp);

		printk(KERN_ERR "%s[%d] ---- 0xbf13c100 = 0x%08x\n", __FUNCTION__, __LINE__, temp);
	}

	printk(KERN_ERR "%s[%d] ---- g_i2c_ds6103 & demod_i2c_id[%d]\n", __FUNCTION__, __LINE__, g_i2c_ds6103);

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

	p_priv->cfg.pin_config.lnb_enable = 1;              //MT_FE_PIN_LEVEL_HIGH;
	p_priv->cfg.pin_config.vsel_when_13v = 0;           //MT_FE_PIN_LEVEL_LOW;
	p_priv->cfg.pin_config.vsel_when_lnb_off = 1;       //MT_FE_PIN_LEVEL_HIGH;
	p_priv->cfg.pin_config.diseqc_out_when_lnb_off = 0; //MT_FE_PIN_LEVEL_LOW;
	p_priv->cfg.pin_config.lnb_enable_by_mcu = 0;
	p_priv->cfg.pin_config.lnb_prot_by_mcu = 0;

	if (p_priv->cfg.freq_offset_limit == 0)
	{
		p_priv->cfg.freq_offset_limit = 4000;
	}

	if (NULL == ds6103_pg_channel_info)
	{
		ds6103_pg_channel_info = kzalloc(sizeof(mt_unf_fe_channel_info_t) * MAX_BS_TP_NUM_PER_SAT, GFP_KERNEL);
		if (ds6103_pg_channel_info == NULL)
		{
			kfree(dev_handle);
			kfree((void *)p_priv);
			g_ds6103_priv = NULL;
			return -ENOMEM;
		}
	}
	p_priv->scan_info.p_channel_info = ds6103_pg_channel_info;

	//p_priv->bs_info.bs_times = p_priv->cfg.bs_times;
	p_priv->bs_info.tp_max_num = MAX_TP_ONE_SCAN;
	if (NULL == p_priv->bs_info.p_tp_info)
	{
		p_priv->bs_info.p_tp_info = kzalloc(sizeof(MT_FE_TP_INFO) * p_priv->bs_info.tp_max_num, GFP_KERNEL);
		if (p_priv->bs_info.p_tp_info == NULL)
		{
			kfree(dev_handle);
			kfree((void *)p_priv);
			if (ds6103_pg_channel_info)
			{
				kfree(ds6103_pg_channel_info);
				ds6103_pg_channel_info = NULL;
			}

			g_ds6103_priv = NULL;
			return -ENOMEM;
		}
	}
	p_priv->bs_info.bs_algorithm = MT_FE_BS_ALGORITHM_A;

	if (!g_dev_init_flag)
	{
		mt_fe_dmd_ds6103_config_default(dev_handle);

		switch (symphony_get_chip_rev())
		{
			case CHIP_SYMPHONY4_A0:
			case CHIP_SYMPHONY4_A1:
				dev_handle->ts_cfg.iSerialDriverMode = 1;	// improve the TS driver capability to 12mA
				break;

			default:
				break;
		}
	}

	if ((attr->demod_addr == 0xD2) || 
		(attr->demod_addr == 0xD4) || 
		(attr->demod_addr == 0xD6) || 
		(attr->demod_addr == 0xD8))
	{
		dev_handle->demod_dev_addr = attr->demod_addr;
	}

	if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
		(attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
		(attr->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
		(attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
	{
		printk(KERN_ERR "[%s %d]tuner_type = %d, tuner_addr = 0x%02x\n", 
			   __FUNCTION__, __LINE__, attr->tuner_type, attr->tuner_addr);

		dev_handle->tuner_cfg.tuner_dev_addr = attr->tuner_addr;
		g_ds6103_priv = p_priv;
		if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TS6011)
		{
			mt_fe_dmd_ds6103_select_tuner(dev_handle, MtFeTn_Montage_TS6011);
			printk(KERN_ERR "----------------ds6103 select ss2 tuner TS6011-------------------------------\n");
		}
		else if (attr->tuner_type == MT_UNF_TUNER_TYPE_AV2012)
		{
			mt_fe_dmd_ds6103_select_tuner(dev_handle, MtFeTn_AV2012);
			printk(KERN_ERR "----------------ds6103 select ss2 tuner AV2012-------------------------------\n");
		}
		else if (attr->tuner_type == MT_UNF_TUNER_TYPE_AV2018)
		{
			mt_fe_dmd_ds6103_select_tuner(dev_handle, MtFeTn_AV2018);
			printk(KERN_ERR "----------------ds6103 select ss2 tuner AV2018-------------------------------\n");
		}
		else if (attr->tuner_type == MT_UNF_TUNER_TYPE_RDA5815M)
		{
			mt_fe_dmd_ds6103_select_tuner(dev_handle, MtFeTn_RDA_5815M);
			printk(KERN_ERR "----------------ds6103 select ss2 tuner RDA5815M-----------------------------\n");
		}
		else if (attr->tuner_type == MT_UNF_TUNER_TYPE_RDA5815S)
		{
			mt_fe_dmd_ds6103_select_tuner(dev_handle, MtFeTn_RDA_5815S);
			printk(KERN_ERR "----------------ds6103 select ss2 tuner RDA5815S-----------------------------\n");
		}

		mt_fe_dmd_ds6103_register_notify(port_m88ds6103_register_to_drv_notify);

#if 1
		if (dvbs_nl_family.id > 0)
		{
			ret = genl_unregister_family(&dvbs_nl_family);
		}

		ret = genl_register_family(&ds6103_nl_family);
		if (ret)
		{
			printk("genl_register_family failed, ret = 0x%08x\n", ret);
			kfree(dev_handle);
			kfree((void *)p_priv);
			if (ds6103_pg_channel_info)
			{
				kfree(ds6103_pg_channel_info);
				ds6103_pg_channel_info = NULL;
			}

			if (p_priv->bs_info.p_tp_info)
			{
				kfree(p_priv->bs_info.p_tp_info);
				p_priv->bs_info.p_tp_info = NULL;
			}

			//printk("genl_register_family failed, maybe already registered, ret = 0x%08x\n", ret);

			g_ds6103_priv = NULL;

			return -EINVAL;
		}

		memcpy(&dvbs_nl_family, &ds6103_nl_family, sizeof(ds6103_nl_family));
#else
		if (dvbs_nl_family.id <= 0)
		{
			printk("genl_register_family failed, maybe already registered, ret = 0x%08x\n", ret);

			ret = genl_register_family(&ds6103_nl_family);
			if (ret)
			{
				printk("genl_register_family failed, ret = 0x%08x\n", ret);
				kfree(dev_handle);
				kfree((void *)p_priv);
				if (ds6103_pg_channel_info)
				{
					kfree(ds6103_pg_channel_info);
					ds6103_pg_channel_info = NULL;
				}

				if (p_priv->bs_info.p_tp_info)
				{
					kfree(p_priv->bs_info.p_tp_info);
					p_priv->bs_info.p_tp_info = NULL;
				}

				//printk("genl_register_family failed, maybe already registered, ret = 0x%08x\n", ret);

				g_ds6103_priv = NULL;
				return -EINVAL;
			}

			memcpy(&dvbs_nl_family, &ds6103_nl_family, sizeof(struct genl_family));
		}
		else
		{
#if 0
			printk("%s[%d] ---- ds6103_nl_family: \n", __FUNCTION__, __LINE__);
			printk("ds6103_nl_family.id                 = %d\n", ds6103_nl_family.id);
			printk("ds6103_nl_family.name               = %s\n", ds6103_nl_family.name);
			printk("ds6103_nl_family.version            = %d\n", ds6103_nl_family.version);
			printk("ds6103_nl_family.maxattr            = %d\n", ds6103_nl_family.maxattr);
			printk("ds6103_nl_family.netnsok            = %d\n", ds6103_nl_family.netnsok);
			printk("ds6103_nl_family.module             = %d\n", ds6103_nl_family.module);
			printk("ds6103_nl_family.ops                = %d\n", ds6103_nl_family.ops);
			printk("ds6103_nl_family.n_ops              = %d\n", ds6103_nl_family.n_ops);


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

			//ds6103_nl_family.id = dvbs_nl_family.id;
			memcpy(&ds6103_nl_family, &dvbs_nl_family, sizeof(struct genl_family));

			ds6103_nl_family.module = THIS_MODULE;
			ds6103_nl_family.ops = ds6103_nl_ops;
			ds6103_nl_family.n_ops = ARRAY_SIZE(ds6103_nl_ops);
		}
#endif

#ifdef CFG_SYNC_BLINDSCAN
		init_waitqueue_head(&bs_cb_wq);
		event_status = 0;
#endif
	}

	if (!g_dev_init_flag)
	{
		dev_handle->ts_cfg.mTsMode = (attr->output_mode >= MT_UNF_FE_OUTPUT_MODE_SERIAL) ? MtFeTsOutMode_Serial : MtFeTsOutMode_Common;

		mt_fe_dmd_ds6103_init(dev_handle);

		printk("%s[%d] ---- demod_addr[%02x], tuner_addr[%02x]\n", __FUNCTION__, __LINE__, dev_handle->demod_dev_addr, dev_handle->tuner_cfg.tuner_dev_addr);
	}

	ss2_status = MtFeLockState_Undef;

	info->is_attach = 1;
	info->pre_attr.demod_dev_type = attr->demod_dev_type;
	g_dev_init_flag = 1;
	g_priv_count++;
	//printk(KERN_ERR "[%s %d]success\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

int m88ds6103_detach(frontend_info_s *info)
{
	mt_fe_ds6103_priv_handle p_priv = NULL;

	if (NULL == info)
	{
		printk(KERN_ERR "[%s %d]error, info is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	p_priv = (mt_fe_ds6103_priv_handle)(info->handle);
	if (NULL == p_priv)
	{
		printk(KERN_ERR "[%s %d]error, info is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	if (info->is_attach)
	{
		if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
			(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
			(p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
			(p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
		{
			genl_unregister_family(&ds6103_nl_family);
		}

		//printk(KERN_ERR "[%s %d]g_priv_count=%d\n", __FUNCTION__, __LINE__, g_priv_count);
		if (g_priv_count >= 1)
		{
			g_priv_count--;

			if (0 == g_priv_count)
			{
				if (dev_handle)
				{
					kfree(dev_handle);
					dev_handle = NULL;
				}
				g_dev_init_flag = 0;
			}
		}
		kfree(p_priv);
		info->is_attach = 0;
	}

	return MT_SUCCESS;
}

int m88ds6103_resume(void)
{
	mt_u8 reg_val = 0;
	//int ret = 0;
	mt_u32 temp = 0;

	//printk(KERN_ERR "[%s %d]g_dev_init_flag=%d\n", __FUNCTION__, __LINE__, g_dev_init_flag);

	if (1)//(!g_dev_init_flag)
	{
		switch (symphony_get_chip_rev())
		{
			case CHIP_SYMPHONY_A0:
			case CHIP_SYMPHONY_A1:
			case CHIP_SYMPHONY_A2:
			case CHIP_SYMPHONY3_A0:
				printk("%s[%d] -- Symphony1 or Symphony3 + DS6103\n", __FUNCTION__, __LINE__);
				break;

			case CHIP_SYMPHONY2_A0:
			case CHIP_SYMPHONY2_A1:
			case CHIP_SYMPHONY2_A2:
			case CHIP_SYMPHONY2_A3:
				printk("%s[%d] -- Symphony2 + DS6103\n", __FUNCTION__, __LINE__);

				/*read 0xbf5b0c00 before demod start working*/
				_mt_fe_read32_ds6103(0xbf5b0c00, &temp);

				/*apb address*/
				_mt_fe_read32_ds6103(0xbf138010, &temp);
				temp &= ~(0x3 << 0);
				_mt_fe_write32_ds6103(0xbf138010, temp);

				/*config demod mux*/
				_mt_fe_read32_ds6103(0xbf138020, &temp);
				temp &= ~(1 << 0);
				_mt_fe_write32_ds6103(0xbf138020, temp);

				/*config tuner i2c master pinmux*/
				_mt_fe_read32_ds6103(0xbf13c010, &temp);
				temp &= ~(0x0f << 16);
				temp &= ~(0x0f << 20);
				_mt_fe_write32_ds6103(0xbf13c010, temp);


				// 1. S2x_ts1_clk输入SW_PIN2_SEL BIT[19:16]=4,BIT[23:20]=4;对应寄存器：BF13C008.
				// 2. S2x_ts0_clk输入SW_PIN2_SEL BIT[15:12]=0,BIT[27:24]=0;对应寄存器：BF13C008.

				_mt_fe_read32_ds6103(0xbf13c008, &temp);
				temp &= ~(0xffff << 12);
				temp |= (0x04 << 16);
				temp |= (0x04 << 20);
				_mt_fe_write32_ds6103(0xbf13c008, temp);

				// enable ext TS0, 0xbf138008[12] = 0
				// enable ext TS3, 0xbf138008[20] = 0
				_mt_fe_read32_ds6103(0xbf138008, &temp);
				temp &= ~(0x01 << 20);
				temp &= ~(0x01 << 12);
				_mt_fe_write32_ds6103(0xbf138008, temp);

				// for 88 pin config
				_mt_fe_read32_ds6103(0xbf5d009c, &temp);
				temp &= ~0x3000000; // bit[25:24] = 0
				_mt_fe_write32_ds6103(0xbf5d009c, temp);

				_mt_fe_read32_ds6103(0xbf157000, &temp);
				temp &= ~0x0300000; // bit[21:20] = 0
				_mt_fe_write32_ds6103(0xbf157000, temp);

				break;

			case CHIP_SYMPHONY4_A0:
			case CHIP_SYMPHONY4_A1:
				/* fix issue 13008/13003 STB can not play after Standby
				* There are four sections that need to be set up before frontend works,
				* include PINMUX CLK ANALOG TS.
				* Two of the modules(PINMUX CLK) are backed up and restored in their own internal drivers, 
				* but the other two(ANALOG TS) are not. So it leads to problems
				* Frontend has to reset their Settings until the ANALOG/TS can fix the problem
				*/
				printk("%s[%d] -- Symphony4 + DS6103\n", __FUNCTION__, __LINE__);

				//BF5D0094[3]=1  //IF PAD设置为digital input
				_mt_fe_read32_ds6103(0xbf5d0094, &temp);
				temp |= 0x08;
				_mt_fe_write32_ds6103(0xbf5d0094, temp);

				//BF138008[0]=0 //TS2 input
				_mt_fe_read32_ds6103(0xbf138008, &temp);
				temp &= ~0x01;
				_mt_fe_write32_ds6103(0xbf138008, temp);

				//BF200020[31:0]=0xC7D801FF//TC2 CTR
				_mt_fe_write32_ds6103(0xbf200020, 0x0);
				_mt_fe_write32_ds6103(0xbf200020, 0xc7d801ff);
#if 0	// TS2 pinmux reference
				//BF13C048[3:0]=5//TS2_CLK
				_mt_fe_read32_ds6103(0xbf13c048, &temp);
				temp &= ~0x0F;
				temp |= 0x05;
				_mt_fe_write32_ds6103(0xbf13c048, temp);

				//BF5D0094[3]=1  //IF PAD设置为digital input
				_mt_fe_read32_ds6103(0xbf5d0094, &temp);
				temp |= 0x08;
				_mt_fe_write32_ds6103(0xbf5d0094, temp);

				//BF13C1A0[2:0]=1 //TS2_SYNC pin配置为GPIO
				_mt_fe_read32_ds6103(0xbf13c1a0, &temp);
				temp &= ~0x07;
				temp |= 0x01;
				_mt_fe_write32_ds6103(0xbf13c1a0, temp);

				//BF13C1A4[2:0]=2 //TS2_DATA
				_mt_fe_read32_ds6103(0xbf13c1a4, &temp);
				temp &= ~0x07;
				temp |= 0x02;
				_mt_fe_write32_ds6103(0xbf13c1a4, temp);

				//BF50B000[0]=1 //open clk
				_mt_fe_read32_ds6103(0xbf50b000, &temp);
				temp |= 0x01;
				_mt_fe_write32_ds6103(0xbf50b000, temp);

				//BF50B004[6:5]=00 //clk正沿
				_mt_fe_read32_ds6103(0xbf50b004, &temp);
				temp &= ~0x60;
				_mt_fe_write32_ds6103(0xbf50b004, temp);

				//BF50B00C[5]=1  //rest off
				_mt_fe_read32_ds6103(0xbf50b00c, &temp);
				temp |= 0x20;
				_mt_fe_write32_ds6103(0xbf50b00c, temp);

				//BF138008[0]=0 //TS2 input
				_mt_fe_read32_ds6103(0xbf138008, &temp);
				temp &= ~0x01;
				_mt_fe_write32_ds6103(0xbf138008, temp);

				//BF200020[31:0]=0xC7D801FF//TC2 CTR
				_mt_fe_write32_ds6103(0xbf200020, 0x0);
				_mt_fe_write32_ds6103(0xbf200020, 0xc7d801ff);

#elif 0	// TS3 Parallel
				// DEBUGB 0xBF13c124 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c124, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c124, temp);

				// DEBUGB 0xBF13c128 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c128, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c128, temp);

				// DEBUGB 0xBF13c12c 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c12c, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c12c, temp);

				// DEBUGB 0xBF13c130 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c130, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c130, temp);

				// DEBUGB 0xBF13c134 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c134, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c134, temp);

				// DEBUGB 0xBF13c164 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c164, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c164, temp);

				// DEBUGB 0xBF13c174 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c174, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c174, temp);

				// DEBUGB 0xBF13c178 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c178, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c178, temp);

				// DEBUGB 0xBF13c17c 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c17c, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c17c, temp);

				// DEBUGB 0xBF13c180 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c180, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c180, temp);

				// DEBUGB 0xBF13c184 3 0 0000
				_mt_fe_read32_ds6103(0xbf13c184, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_ds6103(0xbf13c184, temp);

				// DEBUGB 0xBF138008 12 12 1
				_mt_fe_read32_ds6103(0xbf138008, &temp);
				temp |= 0x1000;
				_mt_fe_write32_ds6103(0xbf138008, temp);

				// DEBUGB 0xBF50b000 0 0 1
				_mt_fe_read32_ds6103(0xbf50b000, &temp);
				temp |= 0x01;
				_mt_fe_write32_ds6103(0xbf50b000, temp);

				// DEBUGB 0xBF50b00c 6 6 1
				_mt_fe_read32_ds6103(0xbf50b00c, &temp);
				temp |= 0x40;
				_mt_fe_write32_ds6103(0xbf50b00c, temp);

				// DEBUGB 0xBF50b004 8 7 01
				_mt_fe_read32_ds6103(0xbf50b004, &temp);
				temp &= 0xfffffe7f;
				temp |= 0x00000080;
				_mt_fe_write32_ds6103(0xbf50b004, temp);

				// DEBUG 0xBF200030 0x87c001ff

				_mt_fe_write32_ds6103(0xbf200030, 0x0);
				_mt_fe_write32_ds6103(0xbf200030, 0x87c001ff);

#endif
				break;

			default:
				printk("%s[%d] -- Unknown chip\n", __FUNCTION__, __LINE__);
				break;
		}

		/*read 0xbf5b0c00 before demod start working*/

		//_mt_fe_read32_ds6103(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base()+ 0x00));
	}

	if (1) //(!g_dev_init_flag)
	{
		mt_fe_dmd_ds6103_init(dev_handle);

		//printk("%s[%d] ---- demod_addr[%02x], tuner_addr[%02x]\n", __FUNCTION__, __LINE__, dev_handle->demod_dev_addr, dev_handle->tuner_cfg.tuner_dev_addr);
	}

	ss2_status = MtFeLockState_Undef;

	//printk(KERN_ERR "[%s %d]success\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

int m88ds6103_suspend(void)
{
	return MT_SUCCESS;
}

#endif

