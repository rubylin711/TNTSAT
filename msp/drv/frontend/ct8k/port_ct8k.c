/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2018 Montage Technology Group Limited. All Rights Reserved. */
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
#include "mt_fe_tn_montage_ts6011.h"
//#include "mt_fe_sat_tn_montage_ts2022.h"
#include "mt_fe_common.h"
#include "port_ct8k.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_i2c_ct8k.h"

#include "mt_fe_tn_MxL603.h"
#include "./MxL6/MxL603_TunerCfg.h"
#include "mt_module_debug.h"

//#define FOR_PORT_CT8K_CONNECT_SYCHRONOUS

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

#define MT_FE_CT8K_PIN_LEVEL_LOW 0
#define MT_FE_CT8K_PIN_LEVEL_HIGH 1
//#define MT_FE_CT8K_DISEQC_COMMAND_START_DELAY 30
#define MT_FE_CT8K_DISEQC_COMMAND_START_DELAY 50
#define MT_FE_CT8K_DISEQC_COMMAND_END_DELAY 50

#define MAX_TP_ONE_SCAN MAX_BS_TP_NUM_PER_SAT

#define CT8K_NL_NAME "dvbs_bs_nl"

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum
{
  CT8K_NL_OPS_CMD0,
  //CT8K_NL_OPS_CMD1,
  __CT8K_NL_OPS_MAX,
};

#define CT8K_NL_OPS_AMOUNT (__CT8K_NL_OPS_MAX)

enum
{
  CT8K_NL_ATTR_UNSPEC,
  CT8K_NL_ATTR_DATA0,
  //CT8K_NL_ATTR_DATA1,
  __CT8K_NL_ATTR_MAX,
};

#define CT8K_NL_FAMILY_ATTR_MAX (__CT8K_NL_ATTR_MAX - 1)
#define CT8K_NL_FAMILY_ATTR_AMOUNT (__CT8K_NL_ATTR_MAX)

struct ct8k_netlink_data
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

struct ct8k_netlink_pid_list
{
  struct list_head list;
  struct ct8k_netlink_data ct8k_nl_data;
  struct net *net;
};

static MT_FE_CT8K_Device_Handle g_ct8k_handle = NULL;
//static mt_fe_ct8k_priv_handle p_priv = NULL;

static U8 gFeIndex = 0;
static U8 gFeCount = 0;

#define MAX_FE_NUM 5

static mt_unf_fe_sig_type_t gFeList[MAX_FE_NUM] = 
{
  MT_UNF_FE_SIG_TYPE_BUTT, //MT_UNF_FE_SIG_TYPE_DVBS_AUTO,
  MT_UNF_FE_SIG_TYPE_BUTT, //MT_UNF_FE_SIG_TYPE_CAB,
  MT_UNF_FE_SIG_TYPE_BUTT, //MT_UNF_FE_SIG_TYPE_J83B,
  MT_UNF_FE_SIG_TYPE_BUTT, //MT_UNF_FE_SIG_TYPE_DVBT_AUTO
  MT_UNF_FE_SIG_TYPE_BUTT
};

#ifdef CFG_SYNC_BLINDSCAN
/* blind scan -> found one TP -> wait user callback return */
static int event_status;
/* blindscan callback wait queue */
static wait_queue_head_t bs_cb_wq;
#endif

static MT_FE_CT8K_Device_Handle dev_handle = NULL;

static int bInitializedCT8K = 0;
static int bT2SharedMemory = 0;

extern MT_FE_RET mt_fe_dmd_register_notify_ct8k_ss2(void (*callback)(MT_FE_MSG msg, void *p_tp_info));

extern MXL_STATUS MxLWare603_OEM_ReadRegister(UINT8 devId, UINT8 RegAddr, UINT8 *DataPtr);

/* The crypto netlink socket */
//static struct sock *ct8k_nlsk;

mt_fe_ct8k_priv_handle g_ct8k_priv = NULL;
int g_i2c_ct8k = 0;
static MT_U8 ct8k_notify_scan_status = 0;//0:normal, 1:finished or abort
//mt_unf_fe_channel_info_t *ct8k_pg_channel_info = NULL;

mmz_buffer_s sMBuf;

void port_ct8k_config_dvbt2_memory(MT_FE_CT8K_Device_Handle handle);

static int port_m88ct8k_blind_scan_cancel(void *handle);
static int port_m88ct8k_blind_scan_start(void *pArg);
static int port_m88ct8k_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info);
static int ct8k_nl_list_del(void);

static LIST_HEAD(ct8k_nl_pid_list);

static DEFINE_MUTEX(ct8k_nl_mutex);

static struct genl_ops ct8k_nl_ops[CT8K_NL_OPS_AMOUNT];

static struct genl_family ct8k_nl_family = {
    .name = CT8K_NL_NAME,
    .version = 0x1,
    .maxattr = CT8K_NL_FAMILY_ATTR_MAX,
    .netnsok = true,
    .module = THIS_MODULE,
    .ops = ct8k_nl_ops,
    .n_ops = ARRAY_SIZE(ct8k_nl_ops),
};

extern struct genl_family dvbs_nl_family;

//static struct ct8k_netlink_data g_ct8k_nl_data_snd = {0};

MT_FE_CT8K_Device_Handle mt_fe_ct8k_get_handle(void)
{
  return g_ct8k_handle;
}

static int ct8k_nl_sendmsg(mt_unf_fe_channel_info_t *p_channel_info)
{
  //mt_unf_fe_channel_info_t *p_channel_info = NULL;
  struct genl_info info;
  struct ct8k_netlink_data ct8k_nl_data_snd;
  struct sk_buff *skb;
  struct ct8k_netlink_data *ct8k_nl_data;
  struct ct8k_netlink_pid_list *ct8k_nl_pid;
  struct ct8k_netlink_pid_list *tmp;
  void *hdr;
  pid_t pid;
  int i = 0;

  if (mutex_lock_interruptible(&ct8k_nl_mutex))
    return -ERESTARTSYS;

  list_for_each_entry_safe(ct8k_nl_pid, tmp, &ct8k_nl_pid_list, list)
  {
    ct8k_nl_data = &(ct8k_nl_pid->ct8k_nl_data);
    pid = ct8k_nl_data->pid;

    skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb)
      return -ENOMEM;

    hdr = genlmsg_put(skb, 0 /* from kernel */,
                      ct8k_nl_data->seq, &ct8k_nl_family, 0, ct8k_nl_data->cmd);
    if (!hdr)
      goto out_nlmsg_free;

    /*
     * hdr is first nlattr or user header which size is genl_family->hdrsize,
     * if genl_family->hdrsize is 0, there is no user header
     */

    info.snd_portid = pid; /* to app */
    genl_info_net_set(&info, ct8k_nl_pid->net);

    NETLINK_CB(skb).portid = 0; /* from kernel */

    snprintf(ct8k_nl_data_snd.name, sizeof(ct8k_nl_data_snd.name), "kernel nl");
    ct8k_nl_data_snd.freq_khz = p_channel_info->frequency; //0xaabbccdd;
    ct8k_nl_data_snd.symbol_rate = p_channel_info->symbol_rate;
    ct8k_nl_data_snd.port_type = p_channel_info->port_type;
    ct8k_nl_data_snd.DataTsNumber = p_channel_info->DataTsNumber;
    ct8k_nl_data_snd.ts_id = p_channel_info->ts_id;
    ct8k_nl_data_snd.ts_index = p_channel_info->ts_index;
    for (i = 0; i < p_channel_info->DataTsNumber; i++)
    {
      ct8k_nl_data_snd.DataTsIdArray[i] = p_channel_info->DataTsIdArray[i];
    }

    ct8k_nl_data_snd.pid = pid;
    ct8k_nl_data_snd.seq = ct8k_nl_data->seq + 1;
    ct8k_nl_data_snd.cmd = ct8k_nl_data->cmd;
    ct8k_nl_data_snd.data01 = ct8k_nl_data->data01;

    //NLA_PUT_TYPE(skb, typeof(ct8k_nl_data_snd), ct8k_nl_data_snd.data01, ct8k_nl_data_snd);
    if (nla_put(skb, ct8k_nl_data_snd.data01, sizeof(ct8k_nl_data_snd), &ct8k_nl_data_snd))
      goto nla_put_failure;

    genlmsg_end(skb, hdr);

    genlmsg_reply(skb, &info);

    list_del(&(ct8k_nl_pid->list));
    kfree(ct8k_nl_pid);
  }

  mutex_unlock(&ct8k_nl_mutex);

  return 0;

nla_put_failure:
  genlmsg_cancel(skb, hdr);

out_nlmsg_free:
  nlmsg_free(skb);
  mutex_unlock(&ct8k_nl_mutex);

  return -ENOMEM;
}

static int ct8k_nl_list_del(void)
{
  struct ct8k_netlink_pid_list *ct8k_nl_pid;
  struct ct8k_netlink_pid_list *tmp;

  if (mutex_lock_interruptible(&ct8k_nl_mutex))
    return -ERESTARTSYS;

  list_for_each_entry_safe(ct8k_nl_pid, tmp, &ct8k_nl_pid_list, list)
  {
    list_del(&(ct8k_nl_pid->list));
    kfree(ct8k_nl_pid);
  }

  mutex_unlock(&ct8k_nl_mutex);

  return 0;
}

#if 0
static void pinmux_configure(void)
{
    u32 temp = 0;
    /* NIM */
    temp = hal_get_u32((volatile unsigned long *)0xbf13c010);
    if ((((temp >> 16) & 0x0f) != 0x02) || (((temp >> 20) & 0x0f) != 0x02))
    {
      temp &= ~(0xF << 16);
      temp &= ~(0xF << 20);
      temp |= (2 << 16);
      temp |= (2 << 20);
      hal_put_u32((volatile unsigned long *)0xbf13c010, temp);
    }

    temp = hal_get_u32((volatile unsigned long *)0xbf138008);
    if (temp != 0x311)
    {
      temp = 0x311;
      hal_put_u32((volatile unsigned long *)0xbf138008, temp);
    }
}
#endif

mt_unf_fe_fec_type_t port_m88ct8k_deparse_dvb_type(MT_FE_TYPE dvb_type)
{
  mt_unf_fe_fec_type_t temp;

  switch (dvb_type)
  {
  case MtFeType_DVBS:
    temp = MT_UNF_PORT_TYPE_DVBS;
    break;

  case MtFeType_DVBS2:
    temp = MT_UNF_PORT_TYPE_DVBS2;
    break;

  default:
    temp = MT_UNF_PORT_TYPE_DVBS_BUTT; //NIM_UNDEF;
    break;
  }

  return temp;
}

mt_unf_fe_fecrate_t port_m88ct8k_deparse_code_rate(MT_FE_CODE_RATE code_rate)
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


// check_signal_type()
// input para: mt_unf_fe_sig_type_t sig_type
// output ret: int
//             2: DVB-S, DVB-S2, DVBS_AUTO
//             1: DVB-C, J83B, DVB-T, DVB-T2, DVBT_AUTO
//             0: other unsupported signal
//            -1: signal type mismatch
static int check_signal_type(MT_FE_CT8K_Device_Handle handle, mt_unf_fe_sig_type_t sig_type)
{
  MT_FE_TYPE dvb_type;

  //printk("%s[%d] -- FE[%d] type[%d], sig_type[%d], ss2_type[%d], ctt2_type[%d]\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex], sig_type, 
  //       handle->m_device_ss2.demod_type, 
  //       handle->m_device_ctt2.demod_type);

  if ((sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) || 
      (sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
      (sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
      (sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
  {
    dvb_type = handle->m_device_ss2.demod_type;

    if ((dvb_type == MtFeType_DVBS) || (dvb_type == MtFeType_DVBS2) || (dvb_type == MtFeType_DVBS_S2))
    {
      return 2;
    }
    else
    {
      return -1;
    }
  }
  else if (sig_type == MT_UNF_FE_SIG_TYPE_CAB)
  {
    dvb_type = handle->m_device_ctt2.demod_type;

    if (dvb_type == MtFeType_DVBC)
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }
  else if (sig_type == MT_UNF_FE_SIG_TYPE_J83B)
  {
    dvb_type = handle->m_device_ctt2.demod_type;

    if (dvb_type == MtFeType_J83B)
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }
  else if ((sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO) || 
           (sig_type == MT_UNF_FE_SIG_TYPE_DVB_T) || 
           (sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2) || 
           (sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2))) 
  {
    dvb_type = handle->m_device_ctt2.demod_type;

    if ((dvb_type == MtFeType_DVBT) || (dvb_type == MtFeType_DVBT2) || (dvb_type == MtFeType_DVBT_T2))
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return 0;
  }
}


static int port_m88ct8k_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
  //MT_U32 for_scan = 0;
  MT_FE_RET ret = 0;
  MT_FE_TYPE dvb_type = MtFeType_Undef;
  mt_unf_fe_channel_info_t *p_channel_info = NULL;
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  p_channel_info = &(para->channel_info);
  para->channel_info.lock = 0;

  //printk("%s[%d]: para->sig_type = %d\n", __FUNCTION__, __LINE__, para->sig_type);

  gFeList[gFeIndex] = para->sig_type;

  if ((para->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
      (para->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
  {
    if (MT_UNF_PORT_TYPE_DVBS == para->connect_param.sat.port_type)
    {
      //ct8k_handle->demod_type = MtFeType_DVBS;
      dvb_type = MtFeType_DVBS;
    }
    else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
    {
      //ct8k_handle->demod_type = MtFeType_DVBS2;
      dvb_type = MtFeType_DVBS2;
    }
    else
    {
      //ct8k_handle->demod_type = MtFeType_DVBS2;
      dvb_type = MtFeType_DVBS_S2;
    }

    printk("ct8k_channel set ss2: freq = %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
           para->connect_param.sat.freq,
           para->connect_param.sat.sym_rate,
           p_priv->for_scan,
           para->connect_param.sat.port_type,
           para->connect_param.sat.uc_param.use_uc);

    if (para->connect_param.sat.uc_param.use_uc)
    {
      ct8k_handle->m_device_ss2.lnb_cfg.bUnicable = 1;
      ct8k_handle->m_device_ss2.lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
      ct8k_handle->m_device_ss2.lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
      ct8k_handle->m_device_ss2.lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
      ct8k_handle->m_device_ss2.lnb_cfg.iUBVer = para->connect_param.sat.uc_param.ub_ver;
    }
    else
    {
      ct8k_handle->m_device_ss2.lnb_cfg.bUnicable = 0;
    }

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
    //priv->for_scan = p_channel_set_info->for_scan;

    //para->connect_param.sat.freq / 1000,
    //para->connect_param.sat.sym_rate,
    //dvb_type,
    //p_priv->for_scan
    ct8k_handle->m_device_ss2.input_params.input_freq_kHz = para->connect_param.sat.freq;
    ct8k_handle->m_device_ss2.input_params.symbol_rate_KSs = para->connect_param.sat.sym_rate;
    ct8k_handle->m_device_ss2.demod_type = dvb_type;

    ct8k_handle->m_device_ss2.tp_cfg.ucCurTsId = para->connect_param.sat.ts_id;

#if MT_FE_DMD_DVBS_S2_SUPPORT
    ret = mt_fe_dmd_connect_ss2_ct8k(ct8k_handle);
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
      printk("mt_fe_dmd_connect_ss2_ct8k failed %d\n", ret);
    }
  }
  else if ((para->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T) ||
           (para->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2) ||
           (para->sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO) ||
           (para->sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)))
  {
    port_ct8k_config_dvbt2_memory(ct8k_handle);

#if 0
    if (MT_UNF_PORT_TYPE_DVBT == para->connect_param.ter.port_type)
    //if (MT_UNF_FE_SIG_TYPE_DVB_T == para->sig_type)
    {
      dvb_type = MtFeType_DVBT;
      //printk("ct8k_channel_set() tt2: DVB-T, sig_type = %d, dvb_type = %d\n", para->sig_type, dvb_type);
    }
    else if (MT_UNF_PORT_TYPE_DVBT2 == para->connect_param.ter.port_type)
    //else if (MT_UNF_FE_SIG_TYPE_DVB_T2 == para->sig_type)
    {
      dvb_type = MtFeType_DVBT2;
      //printk("ct8k_channel_set() tt2: DVB-T2, sig_type = %d, dvb_type = %d\n", para->sig_type, dvb_type);
    }
    else
    {
      dvb_type = MtFeType_DVBT_T2;
      //printk("ct8k_channel_set() tt2: DVB-TT2, sig_type = %d, dvb_type = %d\n", para->sig_type, dvb_type);
    }
#else
    //dvb_type = MtFeType_DVBT;
    if (MT_UNF_FE_SIG_TYPE_DVB_T == para->sig_type)
    //if (MT_UNF_FE_SIG_TYPE_DVB_T == para->sig_type)
    {
      dvb_type = MtFeType_DVBT;
      //printk("ct8k_channel_set() tt2: DVB-T, sig_type = %d, dvb_type = %d\n", para->sig_type, dvb_type);
    }
    else if (MT_UNF_FE_SIG_TYPE_DVB_T2 == para->sig_type)
    //else if (MT_UNF_FE_SIG_TYPE_DVB_T2 == para->sig_type)
    {
      dvb_type = MtFeType_DVBT2;
      //printk("ct8k_channel_set() tt2: DVB-T2, sig_type = %d, dvb_type = %d\n", para->sig_type, dvb_type);
    }
    else
    {
      dvb_type = MtFeType_DVBT_T2;
      //printk("ct8k_channel_set() tt2: DVB-TT2, sig_type = %d, dvb_type = %d\n", para->sig_type, dvb_type);
    }
#endif

    printk("ct8k_channel_set() tt2: freq = %d, bandwidth = %d, type = %d, dvb_type = %d\n",
           para->connect_param.ter.freq,
           para->connect_param.ter.band_width,
           para->connect_param.ter.port_type,
           dvb_type);

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

    ct8k_handle->m_device_ctt2.input_params.input_freq_kHz = para->connect_param.ter.freq;
    //if (MT_UNF_PORT_TYPE_DVBT == para->connect_param.ter.port_type)
    if (MT_UNF_FE_SIG_TYPE_DVB_T == para->sig_type)
    {
      if (para->connect_param.ter.band_width == 6000)
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_6M;
      else if (para->connect_param.ter.band_width == 7000)
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_7M;
      else
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_8M;

      para->channel_set_info.lock_time = 2000;
    }
    else
    {
      if (para->connect_param.ter.band_width == 1700)
      {
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_1P7M;
        para->channel_set_info.lock_time = 3500;
      }
      else if (para->connect_param.ter.band_width == 5000)
      {
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_5M;
        para->channel_set_info.lock_time = 3250;
      }
      else if (para->connect_param.ter.band_width == 6000)
      {
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_6M;
        para->channel_set_info.lock_time = 3000;
      }
      else if (para->connect_param.ter.band_width == 7000)
      {
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_7M;
        para->channel_set_info.lock_time = 2500;
      }
      else
      {
        ct8k_handle->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_8M;
        para->channel_set_info.lock_time = 2000;
      }
    }

    ct8k_handle->m_device_ctt2.demod_type = dvb_type;

    ret = mt_fe_dmd_connect_ctt2_ct8k(ct8k_handle);
  }
  else if (para->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
  {
    dvb_type = MtFeType_DVBC;

    printk("ct8k_channel set c: freq = %d, symbol rate = %d, type = %d\n",
           para->connect_param.cab.freq,
           para->connect_param.cab.sym_rate,
           dvb_type);

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

    ct8k_handle->m_device_ctt2.input_params.input_freq_kHz = para->connect_param.cab.freq;
    ct8k_handle->m_device_ctt2.input_params.symbol_rate_KSs = para->connect_param.cab.sym_rate / 1000;

    switch (para->connect_param.cab.mod_type)
    {
    case MT_UNF_MOD_TYPE_QAM_16:
      ct8k_handle->m_device_ctt2.input_params.qam = 16;
      break;

    case MT_UNF_MOD_TYPE_QAM_32:
      ct8k_handle->m_device_ctt2.input_params.qam = 32;
      break;

    case MT_UNF_MOD_TYPE_QAM_128:
      ct8k_handle->m_device_ctt2.input_params.qam = 128;
      break;

    case MT_UNF_MOD_TYPE_QAM_256:
      ct8k_handle->m_device_ctt2.input_params.qam = 256;
      break;

    case MT_UNF_MOD_TYPE_QAM_64:
      ct8k_handle->m_device_ctt2.input_params.qam = 64;
      break;

    default:
      ct8k_handle->m_device_ctt2.input_params.qam = 0;
      printk("ct8k_channel set c: auto QAM\n");
      break;
    }

    ct8k_handle->m_device_ctt2.demod_type = dvb_type;

    ret = mt_fe_dmd_connect_ctt2_ct8k(ct8k_handle);

    para->channel_set_info.lock_time = 800;
  }
  else if (para->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
  {
    dvb_type = MtFeType_J83B;

    printk("ct8k_channel set b: freq = %d, symbol rate = %d, type = %d\n",
           para->connect_param.cab.freq,
           para->connect_param.cab.sym_rate,
           dvb_type);

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

    ct8k_handle->m_device_ctt2.input_params.input_freq_kHz = para->connect_param.cab.freq;
    ct8k_handle->m_device_ctt2.input_params.symbol_rate_KSs = para->connect_param.cab.sym_rate;

    switch (para->connect_param.cab.mod_type)
    {
    case MT_UNF_MOD_TYPE_QAM_256:
      ct8k_handle->m_device_ctt2.input_params.qam = 256;
      break;

    case MT_UNF_MOD_TYPE_QAM_64:
    default:
      ct8k_handle->m_device_ctt2.input_params.qam = 64;
      break;
    }

    ct8k_handle->m_device_ctt2.demod_type = dvb_type;

    ret = mt_fe_dmd_connect_ctt2_ct8k(ct8k_handle);

    para->channel_set_info.lock_time = 800;
  }

  if (ret == MtFeErr_Ok)
  {
    return MT_SUCCESS;
  }

  return MT_FAILURE;
}

static void port_m88ct8k_set_22k_onoff(void *handle, MT_U8 onoff_22k, MT_U8 diseqc_out_when_lnb_off)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
  U8 val_0xa1, val_0xa2;
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  if ((ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS)
   && (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)
   && (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
  {
    mt_fe_dmd_open_ss2_ct8k(ct8k_handle, MtFeType_DVBS);
  }

  _mt_fe_dmd_get_reg_ss2_ct8k(ct8k_handle, 0xa1, &val_0xa1);
  _mt_fe_dmd_get_reg_ss2_ct8k(ct8k_handle, 0xa2, &val_0xa2);

  if (onoff_22k == MtFe_True)
  {
    val_0xa1 |= 0x04;
    val_0xa1 &= ~0x03;
    val_0xa1 &= ~0x40;
    val_0xa2 &= ~0xc0;
  }
  else
  {
    if (diseqc_out_when_lnb_off == MT_FE_CT8K_PIN_LEVEL_HIGH)
    {
      val_0xa2 |= 0xc0;
    }
    else
    {
      val_0xa2 &= ~0xc0;
      val_0xa2 |= 0x80;
    }
  }

  _mt_fe_dmd_set_reg_ss2_ct8k(ct8k_handle, 0xa2, val_0xa2);
  _mt_fe_dmd_set_reg_ss2_ct8k(ct8k_handle, 0xa1, val_0xa1);

  printk("port_m88ct8k_set_22k_onoff %d - %s\n", onoff_22k, (onoff_22k == MtFe_True) ? "22K On" : "22K Off");
#endif
}

static void port_m88ct8k_set_lnb_voltage(void *handle, MT_FE_LNB_VOLTAGE voltage, MT_U8 vsel_pin_13v)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
  MT_U8 val_0xa2;
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  if ((ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
  {
    mt_fe_dmd_open_ss2_ct8k(ct8k_handle, MtFeType_DVBS);
  }

  _mt_fe_dmd_get_reg_ss2_ct8k(ct8k_handle, 0xa2, &val_0xa2);
  printk("port_m88ct8k_set_lnb_voltage %d - %dV\n", voltage, (voltage == MtFeLNB_13V) ? 13 : 18);

  if (vsel_pin_13v == MT_FE_CT8K_PIN_LEVEL_HIGH)
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

  _mt_fe_dmd_set_reg_ss2_ct8k(ct8k_handle, 0xa2, val_0xa2);
  val_0xa2 = 0;
  _mt_fe_dmd_get_reg_ss2_ct8k(ct8k_handle, 0xa2, &val_0xa2);
#endif
}

static void port_m88ct8k_set_lnb_onoff(void *handle, MT_U8 lnb_enable, mt_unf_fe_pin_config_para_t *pin_config)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
  U8 val_0xa1, val_0xa2; //, pin, level, value;
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  //pinmux_configure();

  if ((ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
  {
    mt_fe_dmd_open_ss2_ct8k(ct8k_handle, MtFeType_DVBS);
  }

  _mt_fe_dmd_get_reg_ss2_ct8k(ct8k_handle, 0xa2, &val_0xa2);

  printk("port_m88ct8k_set_lnb_onoff %d - %s\n", lnb_enable, (lnb_enable == MtFe_True) ? "On" : "Off");

  if (lnb_enable != MtFe_True) // off
  {
    _mt_fe_dmd_get_reg_ss2_ct8k(ct8k_handle, 0xa1, &val_0xa1);
    val_0xa1 |= 0x40;
    _mt_fe_dmd_set_reg_ss2_ct8k(ct8k_handle, 0xa1, val_0xa1);
    if (pin_config->lnb_enable_by_mcu == 0)
    {
      /*set LNB_EN pin HIGH or LOW */
      if (pin_config->lnb_enable == MT_FE_CT8K_PIN_LEVEL_HIGH)
      {
        val_0xa2 &= ~0x02;
      }
      else
      {
        val_0xa2 |= 0x02;
      }
      //printk(" port_m88ct8k_set_lnb_onoff line:%d\n", __LINE__);
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
    if (pin_config->vsel_when_lnb_off == MT_FE_CT8K_PIN_LEVEL_HIGH)
    {
      val_0xa2 |= 0x01;
    }
    else
    {
      val_0xa2 &= ~0x01;
    }

    /*set DiseQc_OUT mode*/
    if (pin_config->diseqc_out_when_lnb_off == MT_FE_CT8K_PIN_LEVEL_HIGH)
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
      //printk("port_m88ct8k_set_lnb_onoff line:%d.\n", __LINE__);
      if (pin_config->lnb_enable == MT_FE_CT8K_PIN_LEVEL_HIGH)
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

  _mt_fe_dmd_set_reg_ss2_ct8k(ct8k_handle, 0xa2, val_0xa2);

#endif
}

static int port_m88ct8k_diseqc_sendmsg(void *handle, mt_unf_fe_diseqc_sendmsg_t *p_diseqc_sendmsg)
{
  MT_FE_RET ret = MtFeErr_Undef;

#if MT_FE_DMD_DVBS_S2_SUPPORT
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
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
    port_m88ct8k_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

    /* make sure the voltage stable */
    //msleep(MT_FE_CT8K_DISEQC_COMMAND_START_DELAY);
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

  ret = mt_fe_dmd_DiSEqC_send_msg_ss2_ct8k(ct8k_handle, &msg);


  if (p_priv->onoff_22k != 0)
  {
    /* make sure the voltage stable */
    //msleep(MT_FE_CT8K_DISEQC_COMMAND_END_DELAY);
    usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

    /* 22k was closed, open it */
    port_m88ct8k_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
  }
#endif

  return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88ct8k_diseqc_send_tone_burst(void *handle, mt_u8 mode)
{
  MT_FE_RET ret = MtFeErr_Undef;

#if MT_FE_DMD_DVBS_S2_SUPPORT
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  p_priv->cur_diseqc.mode = 0; //PORT_DISEQC_BURST0;//p_diseqc_cmd->mode;

  if (p_priv->lnb_onoff == 0)
  {
    return MT_SUCCESS;
  }

  if (p_priv->onoff_22k != 0)
  {
    /* 22k is opened, close it */
    port_m88ct8k_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

    /* make sure the voltage stable */
    //msleep(MT_FE_CT8K_DISEQC_COMMAND_START_DELAY);
    usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms
  }

  mode = (mode == 0) ? MtFeDiSEqCToneBurst_Unmoulated : MtFeDiSEqCToneBurst_Moulated;
  ret = mt_fe_dmd_DiSEqC_send_tone_burst_ss2_ct8k(ct8k_handle, mode, 0);


  if (p_priv->onoff_22k != 0)
  {
    /* make sure the voltage stable */
    //msleep(MT_FE_CT8K_DISEQC_COMMAND_END_DELAY);
    usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

    /* 22k was closed, open it */
    port_m88ct8k_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
  }
#endif

  return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88ct8k_diseqc_recvmsg(void *handle, mt_unf_fe_diseqc_recvmsg_t *p_diseqc_recvmsg)
{
  MT_FE_RET ret = MtFeErr_Undef;

#if MT_FE_DMD_DVBS_S2_SUPPORT
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

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
    port_m88ct8k_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

    /* make sure the voltage stable */
    //msleep(MT_FE_CT8K_DISEQC_COMMAND_START_DELAY);
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

  ret = mt_fe_dmd_DiSEqC_receive_msg_ss2_ct8k(ct8k_handle, &msg);
  printk("DiSEqC recv msg[0x%x, 0x%x, 0x%x, 0x%x] recv size[%d]\n",
         msg.data_receive[0],
         msg.data_receive[1],
         msg.data_receive[2],
         msg.data_receive[3],
         msg.size_receive);

  p_diseqc_recvmsg->len = msg.size_receive;
  memcpy(p_diseqc_recvmsg->msg, msg.data_receive, p_diseqc_recvmsg->len);
  if (ret == MtFeErr_Ok)
    p_diseqc_recvmsg->status = MT_UNF_FE_DISEQC_RECV_OK;
  else if (ret == MtFeErr_TimeOut)
    p_diseqc_recvmsg->status = MT_UNF_FE_DISEQC_RECV_TIMEOUT;
  else
    p_diseqc_recvmsg->status = MT_UNF_FE_DISEQC_RECV_ERROR;

  if (p_priv->onoff_22k != 0)
  {
    /* make sure the voltage stable */
    //msleep(MT_FE_CT8K_DISEQC_COMMAND_START_DELAY);
     usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

    /* 22k was colsed, open it */
    port_m88ct8k_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
  }
#endif

  return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88ct8k_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  MT_FE_LOCK_STATE stat = 0;

#if 1
  int result = -1;

  result = check_signal_type(ct8k_handle, gFeList[gFeIndex]);

  // For DVB-C, J83.B, DVB-T or DVB-T2
  //if (ct8k_handle->m_device_ctt2.demod_type != MtFeType_Undef)
  if (result == 1)     // For DVB-C, J83.B, DVB-T or DVB-T2
  {
    mt_fe_dmd_get_lock_state_ctt2_ct8k(ct8k_handle, &stat);

    if (stat == MtFeLockState_Locked)
    {
      p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;

      if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBC;
      }
      else if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_J83B;
      }
      else if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBT;
      }
      else if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBT2;
      }
      else
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_BUTT;
      }

      //printk("%s[%d] ---- FE[%d], type[%d], CTT2 Locked!\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex]);
    }
    else
    {
      p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
      if (stat == MtFeLockState_Unlocked)
        p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
      else
        p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;
    }
  }
  else
  {
#if MT_FE_DMD_DVBS_S2_SUPPORT
    mt_fe_dmd_get_lock_state_ss2_ct8k(ct8k_handle, &stat);

    if (stat == MtFeLockState_Locked)
    {
      int iCnt = 0, index = 0;

      p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;

      p_status->param.connect_param.sat.DataTsNumber = ct8k_handle->m_device_ss2.tp_cfg.iTsCnt;
      p_status->param.connect_param.sat.ts_id = ct8k_handle->m_device_ss2.tp_cfg.ucCurTsId;

      for (iCnt = 0; iCnt < ct8k_handle->m_device_ss2.tp_cfg.iTsCnt; iCnt ++)
      {
        p_status->param.connect_param.sat.DataTsIdArray[iCnt] = ct8k_handle->m_device_ss2.tp_cfg.ucTsId[iCnt];

        if (ct8k_handle->m_device_ss2.tp_cfg.ucTsId[iCnt] == ct8k_handle->m_device_ss2.tp_cfg.ucCurTsId)
          index = iCnt;
      }

      p_status->param.connect_param.sat.ts_index = index;

      if (ct8k_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBS;
      }
      else if (ct8k_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS2)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBS2;
      }
      else
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_BUTT;
      }

      //printk("%s[%d] ---- FE[%d], type[%d], SS2 Locked!\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex]);
    }
    else
    {
      p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
      if (stat == MtFeLockState_Unlocked)
        p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
      else
        p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;

      p_status->param.channel_info.port_type = MT_UNF_FE_BUTT;
    }

#endif
  }

#else

  if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
      (p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
      (p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
      (p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
  {
#if MT_FE_DMD_DVBS_S2_SUPPORT
    mt_fe_dmd_get_lock_state_ss2_ct8k(ct8k_handle, &stat);

    if (stat == MtFeLockState_Locked)
    {
      p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;

      if (ct8k_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBS;
      }
      else if (ct8k_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS2)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBS2;
      }
      else
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_BUTT;
      }

      //printk("%s[%d] ---- FE[%d], type[%d], SS2 Locked!\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex]);
    }
    else
    {
      p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
      if (stat == MtFeLockState_Unlocked)
        p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
      else
        p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;

      p_status->param.channel_info.port_type = MT_UNF_FE_BUTT;
    }

#endif
  }
  else
  {
    mt_fe_dmd_get_lock_state_ctt2_ct8k(ct8k_handle, &stat);

    if (stat == MtFeLockState_Locked)
    {
      p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;

      if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBC;
      }
      else if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_J83B;
      }
      else if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBT;
      }
      else if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_DVBT2;
      }
      else
      {
        p_status->param.channel_info.port_type = MT_UNF_FE_BUTT;
      }

      //printk("%s[%d] ---- FE[%d], type[%d], CTT2 Locked!\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex]);
    }
    else
    {
      p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
      if (stat == MtFeLockState_Unlocked)
        p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
      else
        p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;
    }
  }
#endif

  return MT_SUCCESS;
}

static int port_m88ct8k_get_signal_quality(void *handle, MT_U32 *p_quality)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  int ret = -1, result = -1;
  U8 percent = 0;

  result = check_signal_type(ct8k_handle, gFeList[gFeIndex]);

  // For DVB-C, J83.B, DVB-T or DVB-T2
  //if (ct8k_handle->m_device_ctt2.demod_type != MtFeType_Undef)
  if (result == 1)
  {
    ret = mt_fe_dmd_get_quality_ctt2_ct8k(ct8k_handle, &percent);
  }
  //else if (ct8k_handle->m_device_ss2.demod_type != MtFeType_Undef) // For DVB-S or DVB-S2
  else if(result == 2)
  {
    ret = mt_fe_dmd_get_quality_ss2_ct8k(ct8k_handle, &percent);
  }

  if (ret < 0)
  {
    percent = 0;
    //return MT_FAILURE;
  }

  *p_quality = percent;

  //printk("%s[%d] -- check result = %d, quality = %d\n", __FUNCTION__, __LINE__, result, percent);

  return MT_SUCCESS;
}

static int port_m88ct8k_get_ber(void *handle, MT_U32 *p_ber)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  U32 err_packages = 0;
  U32 total_packages = 0xFFFFFF;
  int result = 0;

  result = check_signal_type(ct8k_handle, gFeList[gFeIndex]);

  // For DVB-C, J83.B, DVB-T or DVB-T2
  //if (ct8k_handle->m_device_ctt2.demod_type != MtFeType_Undef)
  if (result == 1)     // For DVB-C, J83.B, DVB-T or DVB-T2
  {
    mt_fe_dmd_get_per_ctt2_ct8k(ct8k_handle, &err_packages, &total_packages);
  }
  //else if (ct8k_handle->m_device_ss2.demod_type != MtFeType_Undef) // For DVB-S or DVB-S2
  else if(result == 2)
  {
    mt_fe_dmd_get_per_ct8k_ss2(ct8k_handle, &total_packages, &err_packages);
  }

  //printk("%s[%d] -- check result = %d, error = %d, total = %d\n", __FUNCTION__, __LINE__, result, err_packages, total_packages);

  p_ber[0] = total_packages;
  p_ber[1] = err_packages;
  p_ber[2] = 0;

  return MT_SUCCESS;
}

static int port_m88ct8k_get_snr(void *handle, MT_U32 *p_snr)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  S8 _snr = 0;
  int ret = 0;
  int result = 0;

  result = check_signal_type(ct8k_handle, gFeList[gFeIndex]);

  // For DVB-C, J83.B, DVB-T or DVB-T2
  //if (ct8k_handle->m_device_ctt2.demod_type != MtFeType_Undef)
  if (result == 1)     // For DVB-C, J83.B, DVB-T or DVB-T2
  {
    ret = mt_fe_dmd_get_snr_ctt2_ct8k(ct8k_handle, &_snr);
  }
  //else if (ct8k_handle->m_device_ss2.demod_type != MtFeType_Undef) // For DVB-S or DVB-S2
  else if(result == 2)
  {
    ret = mt_fe_dmd_get_snr_ct8k_ss2(ct8k_handle, &_snr);
  }

  if (ret < 0)
  {
    _snr = 0;
    //return MT_FAILURE;
  }

  *p_snr = _snr;

  //printk("%s[%d] -- check result = %d, snr = %d\n", __FUNCTION__, __LINE__, result, _snr);

  return MT_SUCCESS;
}

static int port_m88ct8k_get_signal_strength(void *handle, MT_U32 *p_strength)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  S8 _strength = 0;
  int ret = 0;
  int result = 0;

  result = check_signal_type(ct8k_handle, gFeList[gFeIndex]);

  // For DVB-C, J83.B, DVB-T or DVB-T2
  //if (ct8k_handle->m_device_ctt2.demod_type != MtFeType_Undef)
  if (result == 1)     // For DVB-C, J83.B, DVB-T or DVB-T2
  {
    ret = mt_fe_dmd_get_strength_ctt2_ct8k(ct8k_handle, &_strength);
  }
  //else if (ct8k_handle->m_device_ss2.demod_type != MtFeType_Undef) // For DVB-S or DVB-S2
  else if(result == 2)
  {
    ret = mt_fe_dmd_get_strength_ct8k_ss2(ct8k_handle, &_strength);
  }

  if (ret < 0)
  {
    _strength = 0;
    //return MT_FAILURE;
  }

  *p_strength = _strength;

  //printk("%s[%d] -- check result = %d, strength = %d\n", __FUNCTION__, __LINE__, result, _strength);

  return MT_SUCCESS;
}

static void port_m88ct8k_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  int result = 0;

  result = check_signal_type(ct8k_handle, gFeList[gFeIndex]);

  //if ((p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
  //    (p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
  //    (p_sig_info->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
  if(result == 2)
  {
    MT_FE_CHAN_INFO_DVBS2 ch_info;
    S32 tuner_offset_KHZ = 0, carrieroffset_KHz;
    U32 sym_rate_KSs = 27500;


    _mt_fe_dmd_get_carrier_offset_ct8k_ss2(ct8k_handle, &carrieroffset_KHz);

    ct8k_handle->m_device_ss2.tuner_cfg.tuner_get_offset(ct8k_handle, &tuner_offset_KHZ);
    p_sig_info->sig_info.sat.freq = ct8k_handle->m_device_ss2.tp_cfg.iFreqKHz + tuner_offset_KHZ - carrieroffset_KHz;

	//printk("%s[%d] -- SS2 freq[%d], carrier offset[%d], tuner_offset[%d], result[%d]\n", __FUNCTION__, __LINE__, 
    //        ct8k_handle->m_device_ss2.tp_cfg.iFreqKHz, carrieroffset_KHz, tuner_offset_KHZ, p_sig_info->sig_info.sat.freq);

    _mt_fe_dmd_get_sym_rate_ct8k_ss2(ct8k_handle, &sym_rate_KSs);
    p_sig_info->sig_info.sat.symbol_rate = sym_rate_KSs;


    //printk("%s[%d] -- FE[%d], type[%d], SS2 freq[%d], sym_rate[%d]\n", __FUNCTION__, __LINE__, 
    //        gFeIndex, gFeList[gFeIndex], p_sig_info->sig_info.sat.freq, p_sig_info->sig_info.sat.symbol_rate);

    mt_fe_dmd_get_channel_info_ct8k_ss2(ct8k_handle, &ch_info);

    if (ct8k_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS)
    {
      p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_SAT;
      p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_DVBS;
    }
    else if (ct8k_handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS2)
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
  else if(result == 1)
  {
    if ((gFeList[gFeIndex] == MT_UNF_FE_SIG_TYPE_DVB_T) || 
        (gFeList[gFeIndex] == MT_UNF_FE_SIG_TYPE_DVB_T2) || 
        (gFeList[gFeIndex] == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)))
    {
      p_sig_info->sig_info.ter.freq = ct8k_handle->m_device_ctt2.input_params.input_freq_kHz;

      switch(ct8k_handle->m_device_ctt2.input_params.demod_bandwidth)
      {
        case MtFeBandwidth_10M:     p_sig_info->sig_info.ter.band_width = 10000;    break;
        case MtFeBandwidth_1P7M:    p_sig_info->sig_info.ter.band_width =  1700;    break;
        case MtFeBandwidth_5M:      p_sig_info->sig_info.ter.band_width =  5000;    break;
        case MtFeBandwidth_6M:      p_sig_info->sig_info.ter.band_width =  6000;    break;
        case MtFeBandwidth_7M:      p_sig_info->sig_info.ter.band_width =  7000;    break;
        case MtFeBandwidth_8M:
        default:                    p_sig_info->sig_info.ter.band_width =  8000;    break;
      }

      //printk("%s[%d] -- FE[%d], type[%d], TT2 freq[%d], bandwidth[%d]\n", __FUNCTION__, __LINE__, 
      //        gFeIndex, gFeList[gFeIndex], p_sig_info->sig_info.ter.freq, p_sig_info->sig_info.ter.band_width);

      if (ct8k_handle->m_device_ctt2.demod_type == MtFeType_DVBT)
      {
        MT_FE_T_TPS_INFO tps_info;

        mt_fe_dmd_get_inform_t_ct8k(ct8k_handle, &tps_info);

        switch(tps_info.t_qam)
        {
          case MtFeModMode_Qpsk:      p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QPSK;      break;
          case MtFeModMode_16Qam:     p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_16;    break;
          case MtFeModMode_64Qam:     p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_64;    break;
          default:                    p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_AUTO;      break;
        }

        switch(tps_info.t_code)
        {
          case MtFeCodeRate_1_2:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_1_2;         break;
          case MtFeCodeRate_2_3:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_2_3;         break;
          case MtFeCodeRate_3_4:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_3_4;         break;
          case MtFeCodeRate_5_6:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_5_6;         break;
          case MtFeCodeRate_7_8:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_7_8;         break;
          default:                    p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_AUTO;        break;
        }

        switch(tps_info.t_guard)
        {
          case MtFeGuarInt_1P32:      p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_32;       break;
          case MtFeGuarInt_1P16:      p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_16;       break;
          case MtFeGuarInt_1P8:       p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_8;        break;
          case MtFeGuarInt_1P4:       p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_4;        break;
          default:                    p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_DEFALUT;    break;
        }

        switch(tps_info.t_fft)
        {
          case MtFeFFTMode_2K:        p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_2K;          break;
          case MtFeFFTMode_8K:        p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_8K;          break;
          default:                    p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_DEFAULT;     break;
        }

        p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
        p_sig_info->sig_info.ter.ter_type = MT_UNF_FE_DVBT;
      }
      else if (ct8k_handle->m_device_ctt2.demod_type == MtFeType_DVBT2)
      {
        MT_FE_T2_TPS_INFO tps_info;

        mt_fe_dmd_get_inform_t2_ct8k(ct8k_handle, &tps_info);

        switch(tps_info.t2_qam)
        {
          case MtFeModMode_Qpsk:      p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QPSK;      break;
          case MtFeModMode_16Qam:     p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_16;    break;
          case MtFeModMode_64Qam:     p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_64;    break;
          case MtFeModMode_256Qam:    p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_256;   break;
          default:                    p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_AUTO;      break;
        }

        switch(tps_info.t2_code)
        {
          case MtFeCodeRate_1_2:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_1_2;         break;
          case MtFeCodeRate_3_5:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_3_5;         break;
          case MtFeCodeRate_2_3:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_2_3;         break;
          case MtFeCodeRate_3_4:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_3_4;         break;
          case MtFeCodeRate_4_5:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_4_5;         break;
          case MtFeCodeRate_5_6:      p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_5_6;         break;
          default:                    p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_AUTO;        break;
        }

        switch(tps_info.t2_guard)
        {
          case MtFeGuarInt_1P32:      p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_32;       break;
          case MtFeGuarInt_1P16:      p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_16;       break;
          case MtFeGuarInt_1P8:       p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_8;        break;
          case MtFeGuarInt_1P4:       p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_4;        break;
          case MtFeGuarInt_1P128:     p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_128;      break;
          case MtFeGuarInt_19P128:    p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_19_128;     break;
          case MtFeGuarInt_19P256:    p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_19_256;     break;
          default:                    p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_DEFALUT;    break;
        }

        switch(tps_info.t2_fft)
        {
          case MtFeFFTMode_1K:        p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_1K;          break;
          case MtFeFFTMode_2K:        p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_2K;          break;
          case MtFeFFTMode_4K:        p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_4K;          break;
          case MtFeFFTMode_8K:        p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_8K;          break;
          case MtFeFFTMode_16K:       p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_16K;         break;
          case MtFeFFTMode_32K:       p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_32K;         break;
          case MtFeFFTMode_8E:        p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_8E;          break;
          case MtFeFFTMode_16E:       p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_16E;         break;
          case MtFeFFTMode_32E:       p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_32E;         break;
          default:                    p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_DEFAULT;     break;
        }

        switch(tps_info.t2_pp)
        {
          case MtFePilot_PP1:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP1;   break;
          case MtFePilot_PP2:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP2;   break;
          case MtFePilot_PP3:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP3;   break;
          case MtFePilot_PP4:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP4;   break;
          case MtFePilot_PP5:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP5;   break;
          case MtFePilot_PP6:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP6;   break;
          case MtFePilot_PP7:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP7;   break;
          case MtFePilot_PP8:         p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP8;   break;
          default:                    p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_BUTT;  break;
        }

        p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
        p_sig_info->sig_info.ter.ter_type = MT_UNF_FE_DVBT2;
      }
      else
      {
        p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_DVBT_AUTO;
        p_sig_info->sig_info.ter.ter_type = MT_UNF_FE_BUTT;
      }
    }
    else if (gFeList[gFeIndex] == MT_UNF_FE_SIG_TYPE_J83B)
    {
      p_sig_info->sig_info.cab.freq = ct8k_handle->m_device_ctt2.input_params.input_freq_kHz;

      switch(ct8k_handle->m_device_ctt2.input_params.qam)
      {
        case 256:
          p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_256;
          p_sig_info->sig_info.cab.symbol_rate = 5361;
          break;

        case  64:
        default:
          p_sig_info->sig_info.cab.mode_type =  MT_UNF_MOD_TYPE_QAM_64;
          p_sig_info->sig_info.cab.symbol_rate = 5057;
          break;
      }

      p_sig_info->sig_info.cab.iq_mode = 0;

      //printk("%s[%d] -- FE[%d], type[%d], J83B freq[%d], sym_rate[%d], qam[%d], qam_type[%d]\n", __FUNCTION__, __LINE__, 
      //        gFeIndex, gFeList[gFeIndex], p_sig_info->sig_info.cab.freq, p_sig_info->sig_info.cab.symbol_rate, 
      //        ct8k_handle->m_device_ctt2.input_params.qam, p_sig_info->sig_info.cab.mode_type);


      p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_J83B;

      if (ct8k_handle->m_device_ctt2.demod_type == MtFeType_J83B)
        p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_J83B;
      else
        p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_BUTT;
    }
    else
    {
      U16 symbol_rate_KSs = 0;
      S32 freq_offset = 0, sym_rate_offset = 0;

      mt_fe_dmd_get_offset_ct8k_c(ct8k_handle, &freq_offset, &sym_rate_offset);

      mt_fe_dmd_get_symbol_rate_ct8k_c(ct8k_handle, &symbol_rate_KSs);

      p_sig_info->sig_info.cab.freq = ct8k_handle->m_device_ctt2.input_params.input_freq_kHz - freq_offset;
      p_sig_info->sig_info.cab.symbol_rate = symbol_rate_KSs;

      switch(ct8k_handle->m_device_ctt2.input_params.qam)
      {
        case 256:
          p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_256;
          break;

        case 128:
          p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_128;
          break;

        case  32:
          p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_32;
          break;

        case  16:
          p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_16;
          break;

        case  64:
          p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_64;
          break;

        default:
          p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_AUTO;
          break;
      }

      p_sig_info->sig_info.cab.iq_mode = 0;

      //printk("%s[%d] -- FE[%d], type[%d], DVB-C freq[%d], sym_rate[%d], qam[%d], qam_type[%d]\n", __FUNCTION__, __LINE__, 
      //        gFeIndex, gFeList[gFeIndex], p_sig_info->sig_info.cab.freq, p_sig_info->sig_info.cab.symbol_rate, 
      //        ct8k_handle->m_device_ctt2.input_params.qam, p_sig_info->sig_info.cab.mode_type);

      p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_CAB;

      if (ct8k_handle->m_device_ctt2.demod_type == MtFeType_DVBC)
        p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_DVBC;
      else
        p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_BUTT;
    }
  }

  /*
  p_sig_info->frequency = handle->m_device_ctt2.input_params.input_freq_kHz;
  p_sig_info->bandwidth = handle->m_device_ctt2.input_params.demod_bandwidth;
  */
}

static int port_m88ct8k_get_signal_agc(void *handle, mt_u32 *p_agc)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  int ret = 0;
  mt_u32 tuner_gain = 0;
  int result = 0;

  result = check_signal_type(ct8k_handle, gFeList[gFeIndex]);

  //#if 1 // For DVB-C, DVB-T or DVB-T2
  if (result == 1)
  {
    if (ct8k_handle->m_device_ctt2.tuner_cfg.tuner_get_gain)
    {
      ret = ct8k_handle->m_device_ctt2.tuner_cfg.tuner_get_gain(ct8k_handle, &tuner_gain);

      if (ret < 0)
      {
        tuner_gain = -1;
        //return MT_FAILURE;
      }

      p_agc[0] = tuner_gain;
      //printk("drv tuner_gain[%d]\n", tuner_gain);
    }
    else
      p_agc[0] = 0;
  }
  //#else // For DVB-S or DVB-S2
  else if (result == 2)
  {
    if (ct8k_handle->m_device_ss2.tuner_cfg.tuner_get_gain)
    {
      ret = ct8k_handle->m_device_ss2.tuner_cfg.tuner_get_gain(ct8k_handle, &tuner_gain);

      if (ret < 0)
      {
        tuner_gain = -1;
        //return MT_FAILURE;
      }

      p_agc[0] = tuner_gain;
      //printk("drv tuner_gain[%d]\n", tuner_gain);
    }
    else
      p_agc[0] = 0;
  }
  //#endif

  //printk("%s[%d] -- check result = %d, agc = %d\n", __FUNCTION__, __LINE__, result, p_agc[0]);


  return MT_SUCCESS;
}

static int port_m88ct8k_standby(void *handle)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  mt_fe_dmd_sleep_ct8k(ct8k_handle);

  return MT_SUCCESS;
}

static int port_m88ct8k_wakeup(void *handle)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  mt_fe_dmd_wake_ct8k(ct8k_handle);

  return MT_SUCCESS;
}

static int port_m88ct8k_get_default_timeout(void *handle, mt_u32 *timeout)
{
  *timeout = 120;

  return MT_SUCCESS;
}

static int port_m88ct8k_set_io(void *handle, MT_BOOL onoff)
{
#if 0
  int ret = 0;
  ret = mt_fe_dmd_ca8k_cab_set_io(handle, onoff);
  if(ret < 0)
    return ret;
#endif
  return MT_SUCCESS;
}

static int port_m88ct8k_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
  mt_unf_fe_channel_info_t *p_channel_info = NULL;
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

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

  //printk("----port_m88ct8k_channel_connect() log1, sig_type = %d\n", para->sig_type);

  if ((para->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
      (para->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
  {
    //if the blindscan is running, stop it first
    count = 0;
    //printk(KERN_ERR "[%s %d]ct8k_handle->m_device_ss2.global_cfg.bBsStatus=%d\n", __FUNCTION__, __LINE__, ct8k_handle->m_device_ss2.global_cfg.bBsStatus);
    while (1 == ct8k_handle->m_device_ss2.global_cfg.bBsStatus)
    {
      ct8k_handle->m_device_ss2.global_cfg.bCancelBs = TRUE;
      msleep(2);
      count++;
      if (count > 100)
      {
        break;
      }
    }

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

    printk("ct8k connect: freq = %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
           para->connect_param.sat.freq,
           para->connect_param.sat.sym_rate,
           for_bs,
           para->connect_param.sat.port_type,
           para->connect_param.sat.uc_param.use_uc);

    //if (p_priv->lnb_onoff == 0)
    //{
    //  return MT_SUCCESS;
    //}

    if (p_priv->lnb_polar != para->connect_param.sat.polarization)
    {
      voltage = (para->connect_param.sat.polarization == PORT_PORLAR_HORIZONTAL) ? MtFeLNB_18V : MtFeLNB_13V;
      port_m88ct8k_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
      p_priv->lnb_polar = (MT_U8)para->connect_param.sat.polarization;
      p_priv->lnb_voltage = voltage;
    }

    if (para->connect_param.sat.uc_param.use_uc)
    {
      ct8k_handle->m_device_ss2.lnb_cfg.bUnicable = 1;
      ct8k_handle->m_device_ss2.lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
      ct8k_handle->m_device_ss2.lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
      ct8k_handle->m_device_ss2.lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
      printk("drv line[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
             __LINE__,
             ct8k_handle->m_device_ss2.lnb_cfg.iBankIndex,
             ct8k_handle->m_device_ss2.lnb_cfg.iUBIndex,
             ct8k_handle->m_device_ss2.lnb_cfg.iUBFreqMHz);
    }
    else
    {
      ct8k_handle->m_device_ss2.lnb_cfg.bUnicable = 0;
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

      if (_mt_fe_dmd_bs_connect_ct8k_ss2(p_priv->ct8k_handle, &bs_tpinfo) == MtFeErr_Ok)
      {
        if (bs_tpinfo.tp_num > 0)
        {
          p_channel_info->frequency = bs_tpinfo.p_tp_info[0].freq_KHz;
          p_channel_info->symbol_rate = bs_tpinfo.p_tp_info[0].sym_rate_KSs;
          p_channel_info->port_type = port_m88ct8k_deparse_dvb_type(bs_tpinfo.p_tp_info[0].dvb_type);
          p_channel_info->fec_inner = port_m88ct8k_deparse_code_rate(bs_tpinfo.p_tp_info[0].code_rate);
          p_channel_info->lock = 1;
        }
        return MT_SUCCESS;
      }
    }
    else
    {
      p_priv->for_scan = 0;
      para->channel_set_info.for_scan = 0;
      port_m88ct8k_channel_set(p_priv, para);
#if 0
      //printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
      for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
      {
        mt_fe_dmd_get_lock_state_ss2_ct8k(ct8k_handle, &status);
        para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
        if (para->channel_info.lock)
        {
          //printk("ct8k connect lock success\n");
          return MT_SUCCESS;
        }
        //mdelay(10);
        msleep(10);
      }

      return ERR_TIMEOUT;
#endif
      return MT_SUCCESS;
    }
#endif
  }
  else if ((para->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T) ||
           (para->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2) ||
           (para->sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO) ||
           (para->sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)))
  {
    //printk("----port_m88ct8k_channel_connect() log2, sig_type = %d, plp_id = %d\n", para->sig_type, para->connect_param.ter.plp_id);

#if 0
    if (MT_UNF_PORT_TYPE_DVBT == para->connect_param.ter.port_type)
    {
      dvb_type = MtFeType_DVBT;
    }
    else if (MT_UNF_PORT_TYPE_DVBT2 == para->connect_param.ter.port_type)
    {
      dvb_type = MtFeType_DVBT2;
    }
    else
    {
      dvb_type = MtFeType_DVBT_T2;
    }
#else
    if (MT_UNF_FE_SIG_TYPE_DVB_T == para->sig_type)
    {
      dvb_type = MtFeType_DVBT;
    }
    else if (MT_UNF_FE_SIG_TYPE_DVB_T2 == para->sig_type)
    {
      dvb_type = MtFeType_DVBT2;
    }
    else
    {
      dvb_type = MtFeType_DVBT_T2;
    }
#endif

    printk("----drv connect, dvb_type = %d\n", dvb_type);
    ct8k_handle->m_device_ctt2.input_params.plp_No = para->connect_param.ter.plp_id;

#if 0
    printk("port_m88ct8k_channel_connect() log 3, freq = %d, bandwidth = %d, type = %d\n",
            para->connect_param.ter.freq,
            para->connect_param.ter.band_width,
            para->connect_param.ter.port_type);
#endif

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
    para->channel_info.lock = 0;

    p_priv->for_scan = 0;
    para->channel_set_info.for_scan = 0;
    port_m88ct8k_channel_set(p_priv, para);

#if 0
    //printk("lock_timeout_ms :%d\n", para->channel_set_info.lock_time);
    for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
    {
      mt_fe_dmd_get_lock_state_ctt2_ct8k(ct8k_handle, &status);
      //printk("%s() %d: lock_status = %d, cnt = %4d - %4d\n", __FUNCTION__, __LINE__, status, cnt, para->channel_set_info.lock_time);
      para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
      if (para->channel_info.lock)
      {
        printk("----port_m88ct8k_channel_connect()----Locked! Type = %d\n", dvb_type);

#if 0
        printk("%s() %d: dump DVB-T2 demod registers start.\n", __FUNCTION__, __LINE__);
        for(cnt = 0; cnt < 0x100; cnt ++)
        {
          _mt_fe_dmd_get_reg_t2_ct8k(ct8k_handle, (U8)cnt, &tmp);
          printk("%s() %d: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
        }

        printk("%s() %d: dump DVB-T demod registers start.\n", __FUNCTION__, __LINE__);

        for(cnt = 0; cnt < 0x100; cnt ++)
        {
          _mt_fe_dmd_get_reg_t_ct8k(ct8k_handle, (U8)cnt, &tmp);
          printk("%s() %d: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
        }

        printk("%s() %d: dump DVB-T & DVB-T2 demod registers end.\n", __FUNCTION__, __LINE__);
#endif

        return MT_SUCCESS;
      }
      //mdelay(10);
      msleep(10);
    }
#endif
    //printk("ct8k connect ctt2 unlock, dev_addr = 0x%02x\n\n", ct8k_handle->m_device_ctt2.dmd_dev_addr);

#if 0
    printk("%s() %d: dump DVB-T2 demod registers start.\n", __FUNCTION__, __LINE__);
    for(cnt = 0; cnt < 0x100; cnt ++)
    {
      _mt_fe_dmd_get_reg_t2_ct8k(ct8k_handle, (U8)cnt, &tmp);
      printk("%s() %d: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
    }

    printk("%s() %d: dump DVB-T demod registers start.\n", __FUNCTION__, __LINE__);

    for(cnt = 0; cnt < 0x100; cnt ++)
    {
      _mt_fe_dmd_get_reg_t_ct8k(ct8k_handle, (U8)cnt, &tmp);
      printk("%s() %d: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
    }

    printk("%s() %d: dump DVB-T & DVB-T2 demod registers end.\n", __FUNCTION__, __LINE__);
#endif

    //return ERR_TIMEOUT;
    return MT_SUCCESS;
  }
  else if (para->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
  {
    dvb_type = MtFeType_DVBC;

#if 0
    printk("ct8k_channel set c: freq =  %d, symbol rate = %d, type = %d\n",
            para->connect_param.cab.freq,
            para->connect_param.cab.sym_rate,
            dvb_type);
#endif

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
    para->channel_info.lock = 0;

    p_priv->for_scan = 0;
    para->channel_set_info.for_scan = 0;
    port_m88ct8k_channel_set(p_priv, para);

#if 0
    //printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
    for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
    {
      mt_fe_dmd_get_lock_state_ctt2_ct8k(ct8k_handle, &status);
      para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
      if (para->channel_info.lock)
      {
        printk("%s() %d: DVB-C Locked!\n", __FUNCTION__, __LINE__);

#if 0
        printk("%s() %d: dump DVB-C demod registers start.\n", __FUNCTION__, __LINE__);
        for(cnt = 0; cnt < 0x100; cnt ++)
        {
          _mt_fe_dmd_get_reg_c_ct8k(ct8k_handle, (U8)cnt, &tmp);
          printk("%s() %d: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
        }
        printk("%s() %d: dump DVB-C demod registers end.\n", __FUNCTION__, __LINE__);
#endif
        return MT_SUCCESS;
      }
      //mdelay(10);
      msleep(10);
    }

#if 0
    printk("%s() %d: dump DVB-C demod registers start.\n", __FUNCTION__, __LINE__);
    for(cnt = 0; cnt < 0x100; cnt ++)
    {
      _mt_fe_dmd_get_reg_c_ct8k(ct8k_handle, (U8)cnt, &tmp);
      printk("%s() %d: 0x%02x - 0x%02x\n", __FUNCTION__, __LINE__, cnt, tmp);
    }
    printk("%s() %d: dump DVB-C demod registers end.\n", __FUNCTION__, __LINE__);
#endif

    return ERR_TIMEOUT;
#endif

    return MT_SUCCESS;
  }
  else if (para->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
  {
    dvb_type = MtFeType_J83B;

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
    para->channel_info.lock = 0;

    p_priv->for_scan = 0;
    para->channel_set_info.for_scan = 0;
    port_m88ct8k_channel_set(p_priv, para);

#if 0
    //printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
    for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
    {
      mt_fe_dmd_get_lock_state_ctt2_ct8k(ct8k_handle, &status);
      para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
      if (para->channel_info.lock)
      {
        printk("%s() %d: J83.B Locked!\n");
        return MT_SUCCESS;
      }
      //mdelay(10);
      msleep(10);
    }

    return ERR_TIMEOUT;
#endif

    return MT_SUCCESS;
  }

  return MT_FAILURE;
}

static void port_m88ct8k_get_dbg_info(MT_FE_CT8K_Device_Handle handle)
{
  U16 i = 0;
  U8 reg_data = 0;
  //U8 tc6800_rd_addr = 0x0;
  U8 reg_addr[256] = { 0 };
  //U8 p_buf[2] = {0, 0};

  printk("\n[%d] dump system registers 00 ~ ff (in hex)\n", __LINE__);

  for (i = 0; i <= 0xff; i++)
  {
    _mt_fe_ct8k_get_reg(handle->sys_dev_addr, i, &reg_data);
    printk("%02x - %02x ", i, reg_data);
    if ((i + 1) % 8 == 0)
      printk("\n");
  }

  printk("\n[%d] dump demod registers 00 ~ ff (in hex), ctt2_type = %d, ss2_type = %d\n", __LINE__, 
                handle->m_device_ctt2.demod_current_type, 
                handle->m_device_ss2.demod_current_type);

  if(handle->m_device_ctt2.demod_current_type != MtFeType_Undef)
  {
    if ((MtFeType_DVBT_T2 == handle->m_device_ctt2.demod_current_type) || (MtFeType_DVBT == handle->m_device_ctt2.demod_current_type))
    {
      for (i = 0; i <= 0xff; i++)
      {
        _mt_fe_ct8k_get_reg(handle->m_device_ctt2.dmd_dev_addr, i, &reg_data);
        printk("%02x - %02x ", i, reg_data);
        if ((i + 1) % 8 == 0)
          printk("\n");
      }
    }
    else if(MtFeType_DVBC == handle->m_device_ctt2.demod_current_type)
    {
      for (i = 0; i <= 0xff; i++)
      {
        _mt_fe_ct8k_get_reg(handle->m_device_ctt2.dmd_dev_addr, i, &reg_data);
        printk("%02x - %02x ", i, reg_data);
        if ((i + 1) % 8 == 0)
          printk("\n");
      }
    }
    else if(MtFeType_J83B == handle->m_device_ctt2.demod_current_type)
    {
      //for (i = 0; i < 0x600; i++)
      for (i = 0x500; i < 0x600; i++)
      {
        _mt_fe_dmd_get_page_reg_ct8k(handle, i, &reg_data);
        printk("%03x - %02x ", i, reg_data);
        if ((i + 1) % 8 == 0)
          printk("\n");
      }
    }

    printk("\n[%d], dump tuner registers...\n", __LINE__);

    if (MtFeTN_TC3800 == handle->m_device_ctt2.tuner_cfg.tuner_type)
    {
      mt_fe_i2c_repeat_enable_ct8k(handle);
      for(i = 0; i <= 0xff; i++)
      {
        reg_addr[i] = i;
        mt_fe_tn_read_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, &reg_addr[i], 1, &reg_data, 1);
        printk("%02x - %02x ", i, reg_data);
        if ((i + 1) % 8 == 0)
          printk("\n");
      }
      mt_fe_i2c_repeat_disable_ct8k(handle);
    }
#if 0
    else if (MtFeTN_TC6800 == handle->m_device_ctt2.tuner_cfg.tuner_type)
    {
      mt_fe_i2c_repeat_enable_ct8k(handle);
      for(i = 0x00; i <= 0xff; i++)
      {
        reg_addr[i] = i;
        mt_fe_tn_read_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, &reg_addr[i], 1, &reg_data, 1);
        printk("0x%02x\t0x%02x\n", i, reg_data);
        //if ((i + 1) % 8 == 0)
        //  printk("\n");
      }

      //printk("\n[%d], dump tuner extended registers...\n", __LINE__);

      //printk("\n[%d], dump 0x1a extended registers...\n", __LINE__);

      for(i = 0x00; i <= 0x05; i++)
      {
        p_buf[0] = 0x1a;
        p_buf[1] = i;
        mt_fe_tn_write_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, p_buf, 2);
        tc6800_rd_addr = 0x1b;
        mt_fe_tn_read_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
        printk("0x1a%02x\t0x%02x\n", i, reg_data);
      }

      //printk("\n[%d], dump 0x39 extended registers...\n", __LINE__);

      for(i = 0x00; i <= 0x7c; i++)
      {
        p_buf[0] = 0x39;
        p_buf[1] = i;
        mt_fe_tn_write_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, p_buf, 2);
        tc6800_rd_addr = 0x3a;
        mt_fe_tn_read_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
        printk("0x39%02x\t0x%02x\n", i, reg_data);
      }

      //printk("\n[%d], dump 0x4e extended registers...\n", __LINE__);

      for(i = 0x00; i <= 0x32; i++)
      {
        p_buf[0] = 0x4e;
        p_buf[1] = i;
        mt_fe_tn_write_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, p_buf, 2);
        tc6800_rd_addr = 0x4f;
        mt_fe_tn_read_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
        printk("0x4e%02x\t0x%02x\n", i, reg_data);
      }

      //printk("\n[%d], dump 0x53 extended registers...\n", __LINE__);

      for(i = 0x0; i <= 0x18; i++)
      {
        p_buf[0] = 0x53;
        p_buf[1] = i;
        mt_fe_tn_write_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, p_buf, 2);
        tc6800_rd_addr = 0x54;
        mt_fe_tn_read_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, &tc6800_rd_addr, 1, &reg_data, 1);
        printk("0x53%02x\t0x%02x\n", i, reg_data);
      }

      mt_fe_i2c_repeat_disable_ct8k(handle);
    }
#endif

    printk("\n\n");
  }

  if(handle->m_device_ss2.demod_current_type != MtFeType_Undef)
  {
    for (i = 0; i <= 0xff; i++)
    {
      _mt_fe_ct8k_get_reg(handle->m_device_ss2.demod_dev_addr, i, &reg_data);
      printk("%02x - %02x ", i, reg_data);
      if ((i + 1) % 8 == 0)
        printk("\n");
    }

    if ((MtFeTn_TS6011 == handle->m_device_ss2.tuner_cfg.tuner_type) || 
        (MtFeTn_TS2022 == handle->m_device_ss2.tuner_cfg.tuner_type))
    {
      mt_fe_i2c_repeat_enable_ct8k(handle);

      for(i = 0; i <= 0xff; i++)
      {
        reg_addr[i] = i;
        mt_fe_tn_read_ct8k(handle->m_device_ss2.tuner_cfg.tuner_dev_addr, &reg_addr[i], 1, &reg_data, 1);
        printk("%02x - %02x ", i, reg_data);
        if ((i + 1) % 8 == 0)
          printk("\n");
      }

      mt_fe_i2c_repeat_disable_ct8k(handle);
    }

    printk("\n\n");
  }
}


static int port_m88ct8k_ioctl(void *handle, mt_u32 cmd, mt_u32 param)
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

  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;
  //pinmux_configure();

  //printk("%s() %d: cmd %d - param 0x%08x\n", __FUNCTION__, __LINE__, cmd, param);

  switch (cmd)
  {
  case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
#if 1
    mt_fe_dmd_get_lock_state_ctt2_ct8k(ct8k_handle, &status);
#else
    mt_fe_dmd_get_lock_state_ss2_ct8k(ct8k_handle, &status);
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
    port_m88ct8k_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
    p_priv->lnb_polar = (MT_U8)param;
    p_priv->lnb_voltage = voltage;
    break;

  case NIM_IOCTRL_SET_LNB_ONOFF:
    //printk("ker ct8k line:%d param=0x%08x\n", __LINE__, param);

#if 0
    if (p_priv->lnb_onoff == param)
    {
      break;
    }
#endif

    //printk("NIM_IOCTRL_SET_LNB_ONOFF set %d\n", param);
    port_m88ct8k_set_lnb_onoff(handle, (MT_U8)param, &p_priv->cfg.pin_config);
    p_priv->lnb_onoff = param;
    if (param == 0)
    {
      break;
    }

    /* restore voltage */
    port_m88ct8k_set_lnb_voltage(handle, p_priv->lnb_voltage, p_priv->cfg.pin_config.vsel_when_13v);

    if (0 != p_priv->cur_diseqc.tx_len)
    {
      /* restore previte diseqc */
      //diseqc_cmd.p_tx_buf = tx_buf;
      memcpy(diseqc_cmd.p_tx_buf, p_priv->cur_diseqc.p_tx_buf, p_priv->cur_diseqc.tx_len);
      diseqc_cmd.mode = p_priv->cur_diseqc.mode;
      diseqc_cmd.result = p_priv->cur_diseqc.result;
      diseqc_cmd.tx_len = p_priv->cur_diseqc.tx_len;
      diseqc_cmd.rx_len = p_priv->cur_diseqc.rx_len;
      //port_m88ct8k_diseqc_ctrl(handle, &diseqc_cmd);
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
      //port_ct8k_lnb_sc_restore(handle, &p_priv->cfg.pin_config);
    }
    break;

  case NIM_IOCTRL_REMOVE_PROTECT:
    if (1 == p_priv->cfg.lnb_prot_by_mcu)
    {
      //port_ct8k_lnb_sc_remove(&p_priv->cfg.pin_config);
    }
    break;

  case NIM_IOCTRL_ENABLE_CHECK_PROTECT:
    if (1 == p_priv->cfg.lnb_prot_by_mcu)
    {
      //port_ct8k_lnb_sc_chk_enable(&p_priv->cfg.pin_config);
    }
    break;
#endif

  case NIM_IOCTRL_SET_22K_ONOFF:
    //printk("ker ct8k ioctl line:%d param=0x%08x\n", __LINE__, param);
    if (p_priv->onoff_22k == param)
    {
      break;
    }
    port_m88ct8k_set_22k_onoff(handle, (MT_U8)param, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
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
    //port_ct8k_recover(handle);
    break;

  case NIM_IOCTRL_SET_CHANNEL_INFO:
    //rc = port_ct8k_set_chninfo(handle, param);
    return rc;
    break;
#endif

  case NIM_IOCTRL_GET_SIGNAL_INFO:
    port_m88ct8k_get_signal_info(handle, (mt_unf_fe_signal_info_t *)param);
    break;

#if 0
  case NIM_IOCTRL_GET_CHANNEL_INFO:
    if (param != 0)
      memcpy((MT_UNF_FE_CHANNEL_INFO_S*)param, &p_priv->cur_channel, sizeof(MT_UNF_FE_CHANNEL_INFO_S));
    break;
#endif

  case NIM_IOCTRL_SCAN_CANCEL:
    port_m88ct8k_blind_scan_cancel(p_priv);
    break;

  case NIM_IOCTRL_GET_SCAN_STATUS:
    //mutex_lock(&bs_notify_status_lock);
    if (param != 0)
      *((MT_U8 *)param) = ct8k_notify_scan_status;
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
    port_m88ct8k_diseqc_sendmsg(handle, p_sendmsg);
    break;

  case NIM_IOCTRL_DISEQC_SEND_TONEBURST:
    //p_diseqc_cmd = (mt_unf_fe_diseqc_sendmsg_t*)param;
    port_m88ct8k_diseqc_send_tone_burst(handle, param);// 'param' is the target tone_burst
    break;

  case NIM_IOCTRL_DISEQC_RECVMSG:
    p_recvmsg = (mt_unf_fe_diseqc_recvmsg_t *)param;
    port_m88ct8k_diseqc_recvmsg(handle, p_recvmsg);
    break;
    //#endif

  case NIM_IOCTRL_T2_GET_PLP_NUM:
    if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
      mt_fe_dmd_get_plp_num_ct8k_t2(ct8k_handle, (U8 *)param); // 'param' is the pointer(address) of plp_num
    break;

  case NIM_IOCTRL_T2_SET_PLP_NO:
    if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
      mt_fe_dmd_select_plp_ct8k_t2(ct8k_handle, (U8)param); // 'param' is the target plp_no
    break;

  case NIM_IOCTRL_T_GET_HIERARCHY_NUM:
    if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
      mt_fe_dmd_get_hierarchy_ct8k_t(ct8k_handle, (U8 *)param); // 'param' is the pointer(address) of hierarchy_num
    break;

  case NIM_IOCTRL_T_SET_HIERARCHY_NO:
    if (ct8k_handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
      mt_fe_dmd_set_hierarchy_ct8k_t(ct8k_handle, (U8)param); // 'param' is the target hierarchy_id
    break;

  case NIM_IOCTRL_T2_GET_PLP_NO:
  case NIM_IOCTRL_T_GET_HIERARCHY_NO:
    if (param != 0)
      *((MT_U8 *)param) = ct8k_handle->m_device_ctt2.input_params.plp_No;
    break;

  case NIM_IOCTRL_SAT_BS_EVENT_PROCESSED:
    //printk("%s[%d]: NIM_IOCTRL_SAT_BS_EVENT_PROCESSED\n", __FUNCTION__, __LINE__);
#ifdef CFG_SYNC_BLINDSCAN
    {
      event_status = 1;
      wake_up_interruptible(&bs_cb_wq);
    }
#endif
    break;

  case NIM_IOCTRL_SET_MODULE_INDEX:
    param %= MAX_FE_NUM;    // to avoid overflow

    gFeIndex = param;

    //printk("%s[%d] -- set current FE index[%d], type[%d]\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex]);
    break;

  case NIM_IOCTRL_GET_TN_DBG_INFO:
    port_m88ct8k_get_dbg_info(ct8k_handle);

    if (ct8k_handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC6800)
    {
      U32 info2 = 0;

      ct8k_handle->m_device_ctt2.tuner_cfg.tuner_get_diagnose_info(ct8k_handle, (MT_U32 *)param, &info2);
    }
    break;

  case NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_CNT:
    if (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
    {
      mt_fe_dmd_get_lock_state_ct8k_ss2(ct8k_handle, &status);

      if (status == MtFeLockState_Locked)
      {
        if (param != 0)
          *((MT_U8 *)param) = ct8k_handle->m_device_ss2.tp_cfg.iTsCnt;
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
    if (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
    {
      int iCnt = 0;

      mt_fe_dmd_get_lock_state_ct8k_ss2(ct8k_handle, &status);

      if (status == MtFeLockState_Locked)
      {
        for (iCnt = 0; iCnt < ct8k_handle->m_device_ss2.tp_cfg.iTsCnt; iCnt++)
        {
          ((MT_U8 *)param)[iCnt] = ct8k_handle->m_device_ss2.tp_cfg.ucTsId[iCnt];
        }
      }
    }
  	break;

  case NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID:
    if (ct8k_handle->m_device_ss2.demod_current_type != MtFeType_Undef)
    {
      mt_fe_dmd_set_ts_ct8k_ss2(ct8k_handle, (U8)param);
      //printk("%s[%d] ---- NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID: ts_id = %d\n", __FUNCTION__, __LINE__, param);
    }
    break;

  default:
    break;
  }

  return MT_SUCCESS;
}

static int port_m88ct8k_blind_scan_cancel(void *handle)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  MT_U8 count=0;

  //printk(KERN_ERR "[%s %d]cancel blind scan\n", __FUNCTION__, __LINE__);
  mt_fe_dmd_blindscan_abort_ss2_ct8k(ct8k_handle, MtFe_True);

  //printk(KERN_ERR "[%s %d]ct8k_handle->m_device_ss2.global_cfg.bBsStatus=%d\n", __FUNCTION__, __LINE__, ct8k_handle->m_device_ss2.global_cfg.bBsStatus);
  while (1 == ct8k_handle->m_device_ss2.global_cfg.bBsStatus)
  {
    ct8k_handle->m_device_ss2.global_cfg.bCancelBs = TRUE;
    msleep(2);
    count++;
    if (count > 100)
    {
      break;
    }
  }

  //printk(KERN_ERR "[%s %d]bCancelBs=%d\n", __FUNCTION__, __LINE__, ct8k_handle->m_device_ss2.global_cfg.bCancelBs);
  event_status = 1;
  wake_up_interruptible(&bs_cb_wq);//to avoid wait timeout
#endif

  return MT_SUCCESS;
}

static const struct nla_policy ct8k_policy[CT8K_NL_FAMILY_ATTR_AMOUNT] = {
    [CT8K_NL_ATTR_DATA0] = {.type = NLA_BINARY, .len = sizeof(struct ct8k_netlink_data)},
};

static int ct8k_nl_recvmsg0(struct sk_buff *skb, struct genl_info *info)
{
  struct ct8k_netlink_data *ct8k_nl_data;
  struct ct8k_netlink_pid_list *ct8k_nl_pid;
  int err = 0;

  //printk("%s\n", __FUNCTION__);

  if (info->attrs[CT8K_NL_ATTR_DATA0])
  {
    ct8k_nl_data = nla_data(info->attrs[CT8K_NL_ATTR_DATA0]);
    ct8k_nl_pid = kzalloc(sizeof(struct ct8k_netlink_pid_list), GFP_KERNEL);
    if (ct8k_nl_pid == NULL)
      return MtFeErr_NoMemory;
    ct8k_nl_pid->net = sock_net(skb->sk);
    /* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
    ct8k_nl_pid->ct8k_nl_data = *ct8k_nl_data;
    //printk("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
    //printk("netlink : pid = %d\n", ct8k_nl_data->pid);
    //printk("netlink : seq = 0x%x\n", ct8k_nl_data->seq);
    //printk("netlink : cmd = %d\n", ct8k_nl_data->cmd);
    //printk("netlink : data01 = %d\n", ct8k_nl_data->data01);
    //printk("netlink : name : %s, freq = %u, sym = %u\n", ct8k_nl_data->name, ct8k_nl_data->freq_khz, ct8k_nl_data->symbol_rate);

    if (mutex_lock_interruptible(&ct8k_nl_mutex))
    {
      kfree(ct8k_nl_pid);
      return -ETIME;
    }

    list_add_tail(&(ct8k_nl_pid->list), &ct8k_nl_pid_list);
    mutex_unlock(&ct8k_nl_mutex);
  }

  return err;
}

static struct genl_ops ct8k_nl_ops[CT8K_NL_OPS_AMOUNT] = {
    [CT8K_NL_OPS_CMD0] = {
        .cmd = CT8K_NL_OPS_CMD0,
        .doit = ct8k_nl_recvmsg0,
        .policy = ct8k_policy,
    },
};

static int port_m88ct8k_blind_scan_start(void *pArg)
{
#if MT_FE_DMD_DVBS_S2_SUPPORT
  S32 ret = 0;
  U32 i = 0;
  MT_FE_CT8K_Device_Handle ct8k_handle = NULL;
  fe_blindscan_param_t *p_scan_info = (fe_blindscan_param_t *)pArg;

  ct8k_handle = (MT_FE_CT8K_Device_Handle)g_ct8k_priv->ct8k_handle;
  printk("ker ct8k: start-stop - start freq: %d, end_freq: %d\n",
         p_scan_info->start_freq, p_scan_info->stop_freq);
  printk("ker ct8k: use_uc[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
         p_scan_info->uc_param.use_uc,
         p_scan_info->uc_param.bank,
         p_scan_info->uc_param.user_band,
         p_scan_info->uc_param.ub_freq_mhz);


  if (dvbs_nl_family.id > 0)
  {
    ret = genl_unregister_family(&dvbs_nl_family);
  }

  ret = genl_register_family(&ct8k_nl_family);
  if (ret)
  {
    printk("genl_register_family failed, ret = 0x%08x\n", ret);

    return MT_FAILURE;
  }

  memcpy(&dvbs_nl_family, &ct8k_nl_family, sizeof(ct8k_nl_family));


  ct8k_handle->m_device_ss2.lnb_cfg.bUnicable = p_scan_info->uc_param.use_uc;
  ct8k_handle->m_device_ss2.lnb_cfg.iBankIndex = p_scan_info->uc_param.bank;
  ct8k_handle->m_device_ss2.lnb_cfg.iUBIndex = p_scan_info->uc_param.user_band;
  ct8k_handle->m_device_ss2.lnb_cfg.iUBFreqMHz = p_scan_info->uc_param.ub_freq_mhz;
  ct8k_handle->m_device_ss2.lnb_cfg.iUBVer = p_scan_info->uc_param.ub_ver;
  ct8k_handle->m_device_ss2.lnb_cfg.bSpectrumInverted = p_scan_info->invert_spectrum;

  g_ct8k_priv->bs_info.tp_num = 0;

  //nim_lock(priv->drv_base);

  ret = mt_fe_dmd_blindscan_ss2_ct8k(ct8k_handle,
                                     p_scan_info->start_freq / 1000,
                                     p_scan_info->stop_freq / 1000,
                                     &g_ct8k_priv->bs_info);

  //nim_unlock(priv->drv_base);

  //printk("nim_ct8k_service end. tp start %d end %d ret = %d\n",
  //g_ct8k_priv->scan_info.start_freq, g_ct8k_priv->scan_info.end_freq, ret);

  if (ret == MtFeErr_Ok)
  {
    p_scan_info = &g_ct8k_priv->scan_info;
    p_scan_info->channel_num_cur = g_ct8k_priv->bs_info.tp_num;
    //printk("port_m88ct8k_blind_scan get %u tp:\n", g_ct8k_priv->bs_info.tp_num);

    /* Bug 108019 */
    p_scan_info->channel_num_total = 0;

    for (i = 0; i < g_ct8k_priv->bs_info.tp_num; i++)
    {
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency =
          g_ct8k_priv->bs_info.p_tp_info[i].freq_KHz;
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate =
          g_ct8k_priv->bs_info.p_tp_info[i].sym_rate_KSs;
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type =
          port_m88ct8k_deparse_dvb_type(g_ct8k_priv->bs_info.p_tp_info[i].dvb_type);
      p_scan_info->p_channel_info[p_scan_info->channel_num_total].fec_inner =
          port_m88ct8k_deparse_code_rate(g_ct8k_priv->bs_info.p_tp_info[i].code_rate);
      //printk("freq: %u, symrate:%u, type:%u\n",
      //p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency,
      //p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate,
      //p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type);
      p_scan_info->channel_num_total += 1;
    }

    //if you scan by dividing all channel to several "channel group", and then scan group by group
    //you should use p_scan_info->channel_num_cur to count scanned tp num for every scan.
    printk("[%s %d]total channel_num_cur: %u channel_num_total:%d\n", __FUNCTION__, __LINE__, p_scan_info->channel_num_cur, p_scan_info->channel_num_total);
    ct8k_handle->m_device_ss2.global_cfg.bBsStatus = 0;
    return MT_SUCCESS;
  }

  ct8k_handle->m_device_ss2.global_cfg.bBsStatus = 0;
#endif
  return MT_FAILURE;
}

static int port_m88ct8k_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info)
{
  mt_fe_ct8k_priv_handle p_priv = (mt_fe_ct8k_priv_handle)handle;
  MT_FE_CT8K_Device_Handle ct8k_handle = p_priv->ct8k_handle;

  //mutex_lock(&bs_notify_status_lock);
  ct8k_notify_scan_status = 0;
  //mutex_unlock(&bs_notify_status_lock);
  ct8k_handle->m_device_ss2.global_cfg.bBsStatus = 1;
  ct8k_handle->m_device_ss2.global_cfg.bCancelBs = FALSE;

  //printk(KERN_ERR "[%s %d]enter\n", __FUNCTION__, __LINE__);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 42)
  kernel_thread(port_m88ct8k_blind_scan_start, p_scan_info, CLONE_KERNEL);
#else
  kthread_run(port_m88ct8k_blind_scan_start, (void *)p_scan_info, "port_m88ct8k_blind_scan");
#endif

  return MT_SUCCESS;
}

void port_m88ct8k_notify_to_up_layer(MT_FE_MSG msg, void *p_param)
{
  //U16 i = 0;
  mt_unf_fe_blind_scan_channel_info_t *p_bs_channel_info = NULL;

  if (p_param != NULL)
    p_bs_channel_info = (mt_unf_fe_blind_scan_channel_info_t *)p_param;

  if (msg == MtFeMsg_BSFinish || msg == MtFeMsg_BSAbort)
  {
    ct8k_notify_scan_status = 1;
    printk("ct8k_notify_scan_status = %d\n", ct8k_notify_scan_status);
  }
}

static void port_m88ct8k_register_to_drv_notify(MT_FE_MSG msg, void *p_param)
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

  //MT_FE_CT8K_Device_Handle ct8k_handle = g_ct8k_priv->ct8k_handle;

  /*
  if (port_m88ct8k_notify_to_up_layer == NULL)
  {
    return;
  }
  */

  //printk("ct8k_notify_function: msg ");

  switch (msg)
  {
  case MtFeMsg_BSStart:
    printk("BSTpStart\n");
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
    channel_info.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
    ct8k_nl_sendmsg(&channel_info);

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
    mt_fe_dmd_ct8k_get_strength(ct8k_handle, &s_strength);
    bs_channel_info.perf.agc =  s_strength;
    mt_fe_dmd_ct8k_get_sat_quality(ct8k_handle, &snr);
    bs_channel_info.perf.snr = snr;
    mt_fe_dmd_ct8k_get_per(ct8k_handle, &total_packages, &err_packages);
    bs_channel_info.perf.ber = (double)err_packages/(double)total_packages;
#endif
    port_m88ct8k_notify_to_up_layer(MtFeMsg_BSTpUnlock, &bs_channel_info);

    channel_info.frequency = p_tp_info[0].freq_KHz;
    channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    channel_info.port_type = MT_UNF_PORT_TYPE_DVBS_BUTT;
    ct8k_nl_sendmsg(&channel_info);

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
    bs_channel_info.nim_type = nim_ct8k_deparse_dvb_type(p_tp_info[0].dvb_type);
    bs_channel_info.param.dvbs.fec_inner = nim_ct8k_deparse_code_rate(p_tp_info[0].code_rate);
    mt_fe_dmd_ct8k_get_strength(ct8k_handle, &s_strength);
    bs_channel_info.param.dvbs.perf.agc =  s_strength;
    mt_fe_dmd_ct8k_get_sat_quality(ct8k_handle, &snr);
    bs_channel_info.param.dvbs.perf.snr = snr;
    mt_fe_dmd_ct8k_get_per(ct8k_handle, &total_packages, &err_packages);
    bs_channel_info.param.dvbs.perf.ber = (double)err_packages/(double)total_packages;
    */
    port_m88ct8k_notify_to_up_layer(MtFeMsg_BSTpLocked, &bs_channel_info);
    printk("[%s %d]BSTpLocked: freq[%7d] sym[%5d] %s\n", __FUNCTION__, __LINE__, p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs, (p_tp_info[0].dvb_type == MtFeType_DVBS2) ? "DVB-S2" : "DVB-S");

#ifdef CFG_SYNC_BLINDSCAN
    //init_waitqueue_head(&bs_cb_wq);

    event_status = 0;
#endif

    //kobject_uevent();
    channel_info.frequency = p_tp_info[0].freq_KHz;
    channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    channel_info.port_type = port_m88ct8k_deparse_dvb_type(p_tp_info[0].dvb_type);

    channel_info.DataTsNumber = p_tp_info[0].iTsCnt;
    channel_info.ts_id = p_tp_info[0].ucCurTsId;

    for (i = 0; i < p_tp_info[0].iTsCnt; i++)
    {
      channel_info.DataTsIdArray[i] = p_tp_info[0].ucTsId[i];

      if (p_tp_info[0].ucTsId[i] == p_tp_info[0].ucCurTsId)
        index = i;
    }

    channel_info.ts_index = index;

    ct8k_nl_sendmsg(&channel_info);

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
    ct8k_nl_list_del();
    port_m88ct8k_notify_to_up_layer(MtFeMsg_BSAbort, NULL);
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

    ct8k_nl_list_del();
    port_m88ct8k_notify_to_up_layer(MtFeMsg_BSFinish, NULL);
    break;

  default:
    break;
  }
}

void port_ct8k_config_dvbt2_memory(MT_FE_CT8K_Device_Handle handle)
{
  u32 dmd_offset_addr = 0;
  u32 dmdreg = 0;

  u32 dmd_base_addr_low = 0, dmd_base_addr_high = 0;

  u32 size = 1 << 22; //3879784;

  u32 addr = 1024 * 1024;

  if (handle->ulT2ShareMemAddr != 0)
  {
    return;
  }

  if (bT2SharedMemory != 0)
  {
    return;
  }

  if (sMBuf.u32StartPhyAddr == 0)
  {
    mt_drv_mmz_alloc_and_map("SYM2_NIM", MMZ_OTHERS, size, 8, &sMBuf);

    addr = sMBuf.u32StartPhyAddr;

    dmd_base_addr_low = ((addr & 0x0FFFFFFF) >> 22) & 0xFF;

    dmd_offset_addr = (addr & 0x0FFFFFFF) % (1 << 22);
    dmd_base_addr_high = (dmd_offset_addr >> 3) & 0x7FFFF;

    writel((dmd_base_addr_high << 8) | (dmd_base_addr_low), (volatile void __iomem *)0xbf13800c);

    dmdreg = readl((volatile void __iomem *)0xbf13800c);

    handle->ulT2ShareMemAddr = addr;

    printk("DVB-T2 demod share buf:  addr[0x%x], 0xbf13800c = [0x%08x]\n", addr, dmdreg);

    bT2SharedMemory = 1;
  }

  return;
}

void port_ct8k_release_dvbt2_memory(MT_FE_CT8K_Device_Handle handle)
{
  if (handle->ulT2ShareMemAddr == 0)
  {
    return;
  }

  if (sMBuf.u32StartPhyAddr != 0)
  {
    mt_drv_mmz_unmap_and_release(&sMBuf);
    sMBuf.u32StartPhyAddr = 0;

    handle->ulT2ShareMemAddr = 0;
  }

  return;
}

#ifdef CONFIG_NET
int m88ct8k_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
  //mt_u8 reg_val = 0;
  int ret = 0;
  mt_u32 temp = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0;
  mt_fe_ct8k_priv_handle p_priv = NULL;
  mt_unf_fe_channel_info_t *ct8k_pg_channel_info = NULL;

  info->ops.connect = port_m88ct8k_channel_connect;
  info->ops.get_status = port_m88ct8k_get_status;
  info->ops.get_ber = port_m88ct8k_get_ber;
  info->ops.get_snr = port_m88ct8k_get_snr;
  info->ops.get_signal_strength = port_m88ct8k_get_signal_strength;
  info->ops.get_signal_quality = port_m88ct8k_get_signal_quality;
  info->ops.get_signal_agc = port_m88ct8k_get_signal_agc;

  info->ops.standby = port_m88ct8k_standby;
  info->ops.wakeup = port_m88ct8k_wakeup;
  info->ops.get_default_timeout = port_m88ct8k_get_default_timeout;
  info->ops.set_io = port_m88ct8k_set_io;
  info->ops.port_ioctl = port_m88ct8k_ioctl;
  info->ops.blind_scan = port_m88ct8k_blind_scan;

  gFeList[gFeIndex] = attr->sig_type;

  if (info->is_attach)
  {
    return MT_SUCCESS;
  }

  if(bInitializedCT8K == 0)
  {
    /*read 0xbf5b0c00 before demod start working*/
    temp = readl((volatile void __iomem *)0xbf5b0c00);

    /*apb address*/
    temp = readl((volatile void __iomem *)0xbf138010);
    temp &= ~(0x3 << 0);
    writel(temp, (volatile void __iomem *)0xbf138010);

    /*config demod mux*/
    temp = readl((volatile void __iomem *)0xbf138020);
    temp &= ~(1 << 0);
    writel(temp, (volatile void __iomem *)0xbf138020);

    /*config tuner i2c master pinmux*/
    temp = readl((volatile void __iomem *)0xbf13c010);
    temp &= ~(0x0f << 16);
    temp &= ~(0x0f << 20);
    writel(temp, (volatile void __iomem *)0xbf13c010);

    //hal_put_u32(0xBF13C008, 0x02254110);

    // for 88 pin config
    temp = readl((volatile void __iomem *)0xbf5d009c);
    temp &= ~0x3000000; // bit[25:24] = 0
    writel(temp, (volatile void __iomem *)0xbf5d009c);

    temp = readl((volatile void __iomem *)0xbf157000);
    temp &= ~0x0300000; // bit[21:20] = 0
    writel(temp, (volatile void __iomem *)0xbf157000);
  }

  if(p_priv == NULL)
  {
    p_priv = kzalloc(sizeof(mt_fe_ct8k_priv_t), GFP_KERNEL);

    if (p_priv == NULL)
    {
      return -ENOMEM;
    }
  }

  if(dev_handle == NULL)
  {
    dev_handle = kzalloc(sizeof(MT_FE_CT8K_DEVICE_SETTINGS), GFP_KERNEL);

    if (dev_handle == NULL)
    {
      kfree((void *)p_priv);
      g_ct8k_priv = NULL;
      return -ENOMEM;
    }
  }

  p_priv->ct8k_handle = dev_handle;
  info->handle = (void *)p_priv;
  //g_ct8k_priv = p_priv;

  g_ct8k_handle = dev_handle;

  g_i2c_ct8k = attr->demod_i2c_id;

  memcpy(&(p_priv->cfg), &(attr->fe_config), sizeof(mt_unf_fe_config_para_t));

#if 0
  if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
  (attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
  (attr->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
  (attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
#else
  //if(bInitializedCT8K == 0)
#endif
  {
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

    if (NULL == ct8k_pg_channel_info)
    {
      ct8k_pg_channel_info = kzalloc(sizeof(mt_unf_fe_channel_info_t) * MAX_BS_TP_NUM_PER_SAT, GFP_KERNEL);
      if (ct8k_pg_channel_info == NULL)
      {
        kfree(dev_handle);
        kfree((void *)p_priv);
        g_ct8k_priv = NULL;
        return -ENOMEM;
      }
    }
    p_priv->scan_info.p_channel_info = ct8k_pg_channel_info;

    //p_priv->bs_info.bs_times = p_priv->cfg.bs_times;
    p_priv->bs_info.tp_max_num = MAX_TP_ONE_SCAN;
    if (NULL == p_priv->bs_info.p_tp_info)
    {
      p_priv->bs_info.p_tp_info = kzalloc(sizeof(MT_FE_TP_INFO) * p_priv->bs_info.tp_max_num, GFP_KERNEL);
      if (p_priv->bs_info.p_tp_info == NULL)
      {
        kfree(dev_handle);
        kfree((void *)p_priv);
        if (ct8k_pg_channel_info)
        {
          kfree(ct8k_pg_channel_info);
          ct8k_pg_channel_info = NULL;
        }

        g_ct8k_priv = NULL;
        return -ENOMEM;
      }
    }

    p_priv->bs_info.bs_algorithm = MT_FE_BS_ALGORITHM_A;
  }

  if(bInitializedCT8K == 0)
  {
    mt_fe_dmd_ct8k_config_default(dev_handle);

    dev_handle->sys_dev_addr = attr->demod_addr;                          //0xd0;
    dev_handle->m_device_ss2.tuner_cfg.tuner_dev_addr = attr->tuner_addr; //0xc0;
    dev_handle->sys_dev_xtal = MtFeXTALMode_27M;
    dev_handle->bSupportDualOutput = TRUE;

    dev_handle->m_device_ctt2.tuner_cfg.tuner_dev_addr = attr->fe_config.tun2_addr;

    //Nim在上电后，判断
    //(1) 如果BF500000[10:8] != 0x3，则允许mode1/2/3/4/5动态切换；
    //(2) 如果BF500000[10:8] = 0x3且clkgen_cpupll_reg = 0x1cb880c9，则禁止mode5/4; 只允许mode1/2/3动态切换；
    //(3) 如果BF500000[10:8] = 0x3且clkgen_cpupll_reg = 0x5cb881b9，则禁止任何模式动态切换；只能进入mode5;
    //(4) 如果BF500000[10:8] = 0x3且clkgen_cpupll_reg != 0x1cb880c9且clkgen_cpupll_reg != 0x5cb881b9，则禁止任何模式动态切换；只能进入mode4;

    tmp1 = readl((volatile void __iomem *)0xbf500000);
    tmp2 = readl((volatile void __iomem *)0xbf5d0048);

    //printk("%s[%d] -- 0xbf500000 = [0x%08x], 0xbf5d0048 = [0x%08x]\n", __FUNCTION__, __LINE__, tmp1, tmp2);

    tmp1 >>= 8;
    tmp1 &= 0x07;

    if (tmp1 != 0x03) // CPU 810MHz
    {
      dev_handle->mode_select = 0x1F; /* bit[4:0] = 0x1F, support mode 1~5 */
      //printk("%s[%d] -- No overclock, support mode 1~5\n", __FUNCTION__, __LINE__);

      tmp3 = readl((volatile void __iomem *)0xbf5d0098);
      if (tmp3 != 0xf000f00f)
      {
        writel(0xf000f00f, (volatile void __iomem *)0xbf5d0098);
        //printk("[%d] -- Set register 0xbf5d0098 from [0x%08x] to [0xf000f00f]\n", __LINE__, tmp3);
      }
    }
    else
    {
      if (tmp2 == 0x1cb880c9) // CPU 810MHz
      {
        dev_handle->mode_select = 0x07; /* bit[4:0] = 0x07, support mode 1~3 */
        //printk("%s[%d] -- CPU  810 MHz, support mode 1~3\n", __FUNCTION__, __LINE__);

        tmp3 = readl((volatile void __iomem *)0xbf5d0098);
        if (tmp3 != 0xf000f00f)
        {
          writel(0xf000f00f, (volatile void __iomem *)0xbf5d0098);
          //printk("[%d] -- Set register 0xbf5d0098 from [0x%08x] to [0xf000f00f]\n", __LINE__, tmp3);
        }
      }
      else if (tmp2 == 0x5cb881b9) // CPU 1GHz
      {
        dev_handle->mode_select = 0x10; /* bit[4:0] = 0x10, support mode 5 only */
        p_priv->bSupportJ83B       = 0; // can't support J83B
        //printk("%s[%d] -- CPU 1000 MHz, support mode 5 only\n", __FUNCTION__, __LINE__);
      }
      else // if ((tmp2 != 0x1cb880c9) && (tmp2 != 0x5cb881b9)) // CPU 1GHz
      {
        dev_handle->mode_select = 0x08; /* bit[4:0] = 0x08, support mode 4 only */
        p_priv->bSupportJ83B       = 0; // can't support J83B
        //printk("%s[%d] -- CPU 1000 MHz, support mode 4 only\n", __FUNCTION__, __LINE__);
      }
    }
  }

  if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2) || 
      (attr->sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO))
  {
    port_ct8k_config_dvbt2_memory(dev_handle);
  }

  if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_CAB) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_J83B) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2) || 
      (attr->sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO))
  {
    printk("%s[%d] -- tuner_type = %d[%02X], tun2_type = %d[%02X]\n", __FUNCTION__, __LINE__,
      attr->tuner_type, attr->tuner_addr,
      attr->fe_config.tun2_type, attr->fe_config.tun2_addr);

    if (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC3800)
    {
      mt_fe_dmd_select_tuner_ctt2_ct8k(dev_handle, MtFeTN_TC3800);
      //printk("----------------ct8k select ctt2 tuner TC3800-------------------------------\n");
    }
    else if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_MXL603) || 
             (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_MXL608) || 
             (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_MXL_608))
    {
      mt_fe_dmd_select_tuner_ctt2_ct8k(dev_handle, MtFeTN_MxL603);
      //printk("----------------ct8k select ctt2 tuner MxL603-------------------------------\n");
    }
    else // if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800)
    {
      mt_fe_dmd_select_tuner_ctt2_ct8k(dev_handle, MtFeTN_TC6800);
      //printk("----------------ct8k select ctt2 tuner TC6800-------------------------------\n");
    }
  }

  if ((attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
      (attr->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
      (attr->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
  {
    g_ct8k_priv = p_priv;
#if MT_FE_DMD_DVBS_S2_SUPPORT
    if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TS6011)
    {
      mt_fe_dmd_select_tuner_ss2_ct8k(dev_handle, MtFeTn_TS6011);
    }
    else if (attr->tuner_type == MT_UNF_TUNER_TYPE_RDA5815)
    {
      mt_fe_dmd_select_tuner_ss2_ct8k(dev_handle, MtFeTn_RDA5815M);
    }
#endif

    mt_fe_dmd_register_notify_ct8k_ss2(port_m88ct8k_register_to_drv_notify);

#if 1
    if (dvbs_nl_family.id > 0)
    {
      ret = genl_unregister_family(&dvbs_nl_family);
    }

    ret = genl_register_family(&ct8k_nl_family);
    if (ret)
    {
      printk("%s[%d] ---- genl_register_family failed, ret = 0x%08x\n", __FUNCTION__, __LINE__, ret);
      kfree(dev_handle);
      kfree((void *)p_priv);
      if (ct8k_pg_channel_info)
      {
        kfree(ct8k_pg_channel_info);
        ct8k_pg_channel_info = NULL;
      }

      if (p_priv->bs_info.p_tp_info)
      {
        kfree(p_priv->bs_info.p_tp_info);
        p_priv->bs_info.p_tp_info = NULL;
      }

      g_ct8k_priv = NULL;
      return -EINVAL;
    }

    memcpy(&dvbs_nl_family, &ct8k_nl_family, sizeof(ct8k_nl_family));

#else
#if 1
    if (dvbs_nl_family.id <= 0)
    {
      ret = genl_register_family(&ct8k_nl_family);
      if (ret)
      {
        printk("genl_register_family failed, ret=0x%08x\n", ret);
        kfree(dev_handle);
        kfree((void *)p_priv);
        if (ct8k_pg_channel_info)
        {
          kfree(ct8k_pg_channel_info);
          ct8k_pg_channel_info = NULL;
        }

        if (p_priv->bs_info.p_tp_info)
        {
          kfree(p_priv->bs_info.p_tp_info);
          p_priv->bs_info.p_tp_info = NULL;
        }

        g_ct8k_priv = NULL;
        return -EINVAL;
      }

      memcpy(&dvbs_nl_family, &ct8k_nl_family, sizeof(struct genl_family));
    }
    else
    {
      memcpy(&ct8k_nl_family, &dvbs_nl_family, sizeof(struct genl_family));

      ct8k_nl_family.module = THIS_MODULE;
      ct8k_nl_family.ops = ct8k_nl_ops;
      ct8k_nl_family.n_ops = ARRAY_SIZE(ct8k_nl_ops);
    }
#endif

    printk("ct8k_nl_family.id = 0x%x\n", ct8k_nl_family.id);
#endif

#ifdef CFG_SYNC_BLINDSCAN
    init_waitqueue_head(&bs_cb_wq);

    event_status = 0;
#endif
  }

  if(bInitializedCT8K == 0)
  {
    mt_fe_system_init_ct8k(dev_handle);
  }

  info->is_attach = 1;

  bInitializedCT8K = 1;

  gFeCount++;
  gFeIndex++;

  return MT_SUCCESS;
}

int m88ct8k_detach(frontend_info_s *info)
{
  mt_fe_ct8k_priv_handle p_priv = NULL;
  //MT_FE_CT8K_Device_Handle dev_handle = NULL;

  p_priv = info->handle;

  if (info->is_attach)
  {
#if 0
    p_priv = info->handle;
    dev_handle = p_priv->ct8k_handle;

    if (p_priv->bs_info.p_tp_info)
    {
      kfree(p_priv->bs_info.p_tp_info);
      p_priv->bs_info.p_tp_info = NULL;
    }

    kfree(dev_handle);
    kfree((void *)p_priv);
    if (ct8k_pg_channel_info)
    {
      kfree(ct8k_pg_channel_info);
      ct8k_pg_channel_info = NULL;
    }
#endif

    info->is_attach = 0;
  }

  gFeCount --;

#if 0
  g_ct8k_priv = NULL;

  genl_unregister_family(&ct8k_nl_family);
#endif

  if ((p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT) || 
      (p_priv->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) || 
      (p_priv->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)) || 
      (p_priv->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO))
  {
    genl_unregister_family(&ct8k_nl_family);
  }

  return MT_SUCCESS;
}
#endif

