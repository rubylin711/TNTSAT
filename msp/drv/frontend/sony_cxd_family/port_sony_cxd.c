/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage Technology Group Limited. All Rights Reserved. */
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

#include <linux/i2c.h>

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

//#include "mt_fe_common.h"
#include "port_sony_cxd.h"
#include "drv_frontend_ioctl.h"

#include "drv_gpio_ioctl.h"

#include "family_source/dvb_cable/sony_demod_dvbc_monitor.h"
#include "family_source/dvb_cable/sony_demod_dvbc2_monitor.h"
#include "family_source/dvb_terr/sony_demod_dvbt_monitor.h"
#include "family_source/dvb_terr/sony_demod_dvbt2_monitor.h"
#include "family_source/dvb_sat/sony_demod_dvbs_s2_monitor.h"
#include "family_source/j83b/sony_demod_j83b_monitor.h"

#include "family_source/tuner/terr_cable_MxL608/sony_tuner_MxL608.h"
#include "family_source/tuner/R850/sony_tuner_r850.h"
#include "family_source/tuner/R858C/sony_tuner_r858.h"
#include "family_source/tuner/R836/sony_tuner_r836.h"
#include "family_source/tuner/tc6800/sony_tuner_tc6800.h"

//#include "mt_fe_i2c_sony_cxd2856.h"

//#include "mt_fe_tn_MxL603.h"
//#include "./MxL6/MxL603_TunerCfg.h"
#include "mt_module_debug.h"

//#define FOR_PORT_SONY_CXD_CONNECT_SYCHRONOUS

/* Sychronous blind scan */
#define CFG_SYNC_BLINDSCAN
/* blind scan wait user callback timeout: 120s */
#define BLINDSCAN_WAIT_CB_TIMEOUT 120

#define MT_FE_SONY_CXD_PIN_LEVEL_LOW 0
#define MT_FE_SONY_CXD_PIN_LEVEL_HIGH 1
//#define MT_FE_SONY_CXD_DISEQC_COMMAND_START_DELAY 30
#define MT_FE_SONY_CXD_DISEQC_COMMAND_START_DELAY 50
#define MT_FE_SONY_CXD_DISEQC_COMMAND_END_DELAY 50

#define MAX_TP_ONE_SCAN MAX_BS_TP_NUM_PER_SAT

#define SONY_CXD_NL_NAME "sat_bs_nl"

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum
{
  SONY_CXD_NL_OPS_CMD0,
  //SONY_CXD_NL_OPS_CMD1,
  __SONY_CXD_NL_OPS_MAX,
};

#define SONY_CXD_NL_OPS_AMOUNT (__SONY_CXD_NL_OPS_MAX)

enum
{
  SONY_CXD_NL_ATTR_UNSPEC,
  SONY_CXD_NL_ATTR_DATA0,
  //SONY_CXD_NL_ATTR_DATA1,
  __SONY_CXD_NL_ATTR_MAX,
};

#define SONY_CXD_NL_FAMILY_ATTR_MAX (__SONY_CXD_NL_ATTR_MAX - 1)
#define SONY_CXD_NL_FAMILY_ATTR_AMOUNT (__SONY_CXD_NL_ATTR_MAX)

struct sony_cxd_netlink_data
{
  pid_t pid;
  u32 seq;
  u32 freq_khz;
  u32 symbol_rate;
  u32 port_type;
  u32 data01;
  u8 cmd;
  char name[32];
};

struct sony_cxd_netlink_pid_list
{
  struct list_head list;
  struct sony_cxd_netlink_data sony_cxd_nl_data;
  struct net *net;
};

static sony_example_driver_handle g_sony_cxd_handle[MAX_SONY_CXD_DEVICES] = {NULL, NULL};
static mt_fe_sony_cxd_priv_handle p_priv[MAX_SONY_CXD_DEVICES] = {NULL, NULL};
static sony_demod_create_param_t createParam[MAX_SONY_CXD_DEVICES];


static uint8_t gFeIndex = 0;
static uint8_t gFeCount[MAX_SONY_CXD_DEVICES] = {0,0};
static uint8_t gSonyIndex = 0;

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

static sony_example_driver_instance_t *dev_handle[MAX_SONY_CXD_DEVICES] = {NULL, NULL};

static int bInitialized_SONY_CXD[MAX_SONY_CXD_DEVICES] = {0, 0};

//extern sony_result_t mt_fe_dmd_register_notify_sony_cxd_ss2(void (*callback)(MT_FE_MSG msg, void *p_tp_info));

/* The crypto netlink socket */
//static struct sock *sony_cxd_nlsk;

mt_fe_sony_cxd_priv_handle g_sony_cxd_priv[MAX_SONY_CXD_DEVICES] = {NULL, NULL};
int g_i2c_sony_cxd = 0;
static uint8_t sony_cxd_notify_scan_status[MAX_SONY_CXD_DEVICES] = {0, 0};
mt_unf_fe_channel_info_t *sony_cxd_pg_channel_info[MAX_SONY_CXD_DEVICES] = {NULL, NULL};


static int port_sony_cxd_get_ber(void *handle, MT_U32 *p_ber);

//mmz_buffer_s sMBuf;

sony_i2c_t cxd_i2c;

void sony_cxd_get_sony_index_by_addr(uint8_t demod_addr);

extern R840_ErrCode R840_AGC_Slow(void); //Set AGC clock to 60Hz

extern sony_result_t sony_tuner_r858_get_tuner_num(sony_tuner_t *pTuner, R858_ExtTunerNum_Type *pExtTunerNum, R858_IntTunerNum_Type *pIntTunerNum);
extern void i2c_isr_onoff(int i2_id, int on);


#if 0
static int port_sony_cxd_blind_scan_cancel(void *handle);
static int port_sony_cxd_blind_scan_start(void *pArg);
static int port_sony_cxd_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info);
#endif

/*static int sony_cxd_nl_list_del(void);*/

static LIST_HEAD(sony_cxd_nl_pid_list);

static DEFINE_MUTEX(sony_cxd_nl_mutex);
#if 0
static struct genl_ops sony_cxd_nl_ops[SONY_CXD_NL_OPS_AMOUNT];


static struct genl_family sony_cxd_nl_family = {
    .name = SONY_CXD_NL_NAME,
    .version = 0x1,
    .maxattr = SONY_CXD_NL_FAMILY_ATTR_MAX,
    .netnsok = true,
    .module = THIS_MODULE,
    .ops = sony_cxd_nl_ops,
    .n_ops = ARRAY_SIZE(sony_cxd_nl_ops),
};
#endif
extern struct genl_family dvbs_nl_family;

/*static struct sony_cxd_netlink_data g_sony_cxd_nl_data_snd = {0}; clean warning*/

sony_example_driver_handle mt_fe_sony_cxd_get_handle(void)
{
  return g_sony_cxd_handle[gSonyIndex];
}

#if 0 /*clean warning  no used*/
static int sony_cxd_nl_sendmsg(mt_unf_fe_channel_info_t *p_channel_info)
{
  //mt_unf_fe_channel_info_t *p_channel_info = NULL;
  struct genl_info info;
  struct sony_cxd_netlink_data sony_cxd_nl_data_snd;
  struct sk_buff *skb;
  struct sony_cxd_netlink_data *sony_cxd_nl_data;
  struct sony_cxd_netlink_pid_list *sony_cxd_nl_pid;
  struct sony_cxd_netlink_pid_list *tmp;
  void *hdr;
  pid_t pid;

  if (mutex_lock_interruptible(&sony_cxd_nl_mutex))
    return -ERESTARTSYS;

  list_for_each_entry_safe(sony_cxd_nl_pid, tmp, &sony_cxd_nl_pid_list, list)
  {
    sony_cxd_nl_data = &(sony_cxd_nl_pid->sony_cxd_nl_data);
    pid = sony_cxd_nl_data->pid;

    skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb)
      return -ENOMEM;

    hdr = genlmsg_put(skb, 0 /* from kernel */,
                      sony_cxd_nl_data->seq, &sony_cxd2856_nl_family, 0, sony_cxd_nl_data->cmd);
    if (!hdr)
      goto out_nlmsg_free;

    /*
     * hdr is first nlattr or user header which size is genl_family->hdrsize,
     * if genl_family->hdrsize is 0, there is no user header
     */

    info.snd_portid = pid; /* to app */
    genl_info_net_set(&info, sony_cxd_nl_pid->net);

    NETLINK_CB(skb).portid = 0; /* from kernel */

    snprintf(sony_cxd_nl_data_snd.name, sizeof(sony_cxd_nl_data_snd.name), "kernel nl");
    sony_cxd_nl_data_snd.freq_khz = p_channel_info->frequency; //0xaabbccdd;
    sony_cxd_nl_data_snd.symbol_rate = p_channel_info->symbol_rate;
    sony_cxd_nl_data_snd.port_type = p_channel_info->port_type;
    sony_cxd_nl_data_snd.pid = pid;
    sony_cxd_nl_data_snd.seq = sony_cxd_nl_data->seq + 1;
    sony_cxd_nl_data_snd.cmd = sony_cxd_nl_data->cmd;
    sony_cxd_nl_data_snd.data01 = sony_cxd_nl_data->data01;

    //NLA_PUT_TYPE(skb, typeof(sony_cxd_nl_data_snd), sony_cxd_nl_data_snd.data01, sony_cxd_nl_data_snd);
    if (nla_put(skb, sony_cxd_nl_data_snd.data01, sizeof(sony_cxd_nl_data_snd), &sony_cxd_nl_data_snd))
      goto nla_put_failure;

    genlmsg_end(skb, hdr);

    genlmsg_reply(skb, &info);

    list_del(&(sony_cxd_nl_pid->list));
    kfree(sony_cxd_nl_pid);
  }

  mutex_unlock(&sony_cxd_nl_mutex);

  return 0;

nla_put_failure:
  genlmsg_cancel(skb, hdr);

out_nlmsg_free:
  nlmsg_free(skb);
  mutex_unlock(&sony_cxd_nl_mutex);

  return -ENOMEM;
}

static int sony_cxd2856_nl_list_del(void)
{
  struct sony_cxd2856_netlink_pid_list *sony_cxd2856_nl_pid;
  struct sony_cxd2856_netlink_pid_list *tmp;

  if (mutex_lock_interruptible(&sony_cxd2856_nl_mutex))
    return -ERESTARTSYS;

  list_for_each_entry_safe(sony_cxd2856_nl_pid, tmp, &sony_cxd2856_nl_pid_list, list)
  {
    list_del(&(sony_cxd2856_nl_pid->list));
    kfree(sony_cxd2856_nl_pid);
  }

  mutex_unlock(&sony_cxd2856_nl_mutex);

  return 0;
}
#endif

void sony_sleep_delay_ms(uint32_t ms)
{
  /*
    TODO:
        Delay ms.
   */

  if (ms <= 10)
    usleep_range(ms * 1000, ms * 1000 + 500);    //mdelay(ms);
  else
    msleep(ms);
}

sony_result_t _mt_i2c_write_sony(uint8_t dev_addr, uint8_t *w_buf, U16 w_byte)
{
  struct mt_i2c_msg mt_msg = { 0, };
  struct i2c_msg *msg = &mt_msg.msg;
  struct i2c_adapter *i2c = NULL;
  int ret = 0;

  i2c = i2c_get_adapter(g_i2c_sony_cxd);
  msg->addr = dev_addr;
  msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
  msg->buf = w_buf;
  msg->len = w_byte;
  mt_msg.rlen = 0;
  mt_msg.wlen = w_byte;
  mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER;
  ret = i2c_transfer(i2c, msg, 1);
  if (ret < 0)
  {
    printk("%s[%d] failed, dev_addr = 0x%02x, ret %d\n", __FUNCTION__, __LINE__, dev_addr, ret);
    return SONY_RESULT_ERROR_I2C;
  }

  return SONY_RESULT_OK;
}

sony_result_t _mt_i2c_read_sony(uint8_t dev_addr, uint8_t *w_buf, U16 w_byte, uint8_t *r_buf, U16 r_byte)
{
  // 8 bit Register Read Protocol:
  // +------+-+-----+-+-+----------+-+
  // |MASTER|S|SADDR|W|  |RegAddr   |
  // +------+-+-----+-+-+-----------+-+
  // |SLAVE |                          |A|               |A| |
  // +------+-+-----+-+-+-----------+-+
  // +------+-+-----+-+-+-----+--+-+
  // |MASTER|S|SADDR|R| |     |MN|P|
  // +------+-+-----+-+-+-----+--+-+
  // |SLAVE |         |A|Data |  | |
  // +------+---------+-+-----+--+-+
  // Legends: SADDR(I2c slave address), S(Start condition), MA(Master Ack), MN(Master NACK),
  // P(Stop condition)
  struct mt_i2c_msg mt_msg = { 0, };
  struct i2c_msg *msg = &mt_msg.msg;
  struct i2c_adapter *i2c = NULL;
  int ret = 0;

  uint8_t buf[130];

  if (w_buf)
    memcpy(buf, w_buf, w_byte);

  //*r_buf = *w_buf;

  i2c = i2c_get_adapter(g_i2c_sony_cxd);
  //printk("[%s ] line:%d i2c_bus 0x%08x\n", __func__, __LINE__, i2c);
  msg->addr = dev_addr;
  msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
  msg->buf = buf;
  msg->len = r_byte;
  mt_msg.rlen = r_byte;
  mt_msg.wlen = w_byte;
  mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER;
  ret = i2c_transfer(i2c, msg, 1);
  if (ret < 0)
  {
    printk("%s[%d] failed, dev_addr = 0x%02x, ret %d\n", __FUNCTION__, __LINE__, dev_addr, ret);
    return SONY_RESULT_ERROR_I2C;
  }

  if (r_buf)
    memcpy(r_buf, buf, r_byte);

#if 0
  if (w_byte > 1)
    printk("%s[%d] -- dev_addr[%02x], w_buf[%02x, %02x], r_buf[%02x]\n", __FUNCTION__, __LINE__, dev_addr, w_buf[0], w_buf[1], r_buf[0]);
  else
    printk("%s[%d] -- dev_addr[%02x], w_buf[%02x], r_buf[%02x]\n", __FUNCTION__, __LINE__, dev_addr, w_buf[0], r_buf[0]);
#endif

  return SONY_RESULT_OK;
}

sony_result_t Sony_I2C_Read(struct sony_i2c_t *pI2c, uint8_t deviceAddress, uint8_t *pData, uint32_t size, uint8_t mode)
{
  //return _mt_i2c_read_sony(deviceAddress, NULL, 0, pData, size);
  sony_result_t result;
  uint8_t w_len = 0, r_len = 0;

  w_len = (size >> 16) & 0xFF;
  r_len = size & 0xFF;

  //return _mt_i2c_read_sony(deviceAddress, pData, w_len, pData, r_len);
  result = _mt_i2c_read_sony(deviceAddress, pData, w_len, pData, r_len);

  return result;
}

sony_result_t Sony_I2C_Write(struct sony_i2c_t *pI2c, uint8_t deviceAddress, const uint8_t *pData, uint32_t size, uint8_t mode)
{
  sony_result_t result;

  //return _mt_i2c_write_sony(deviceAddress, pData, size);
  result = _mt_i2c_write_sony(deviceAddress, (uint8_t *)pData, size);

  return result;
}

sony_result_t Sony_I2C_ReadRegister(struct sony_i2c_t *pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t *pData, uint32_t size)
{
  sony_result_t result;

  //return _mt_i2c_read_sony(deviceAddress, &subAddress, 1, pData, size);
  result = _mt_i2c_read_sony(deviceAddress, &subAddress, 1, pData, size);

  return result;
}

sony_result_t Sony_I2C_WriteRegister(struct sony_i2c_t *pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t *pData, uint32_t size)
{
  sony_result_t result;

  uint8_t buf[130];

  buf[0] = subAddress;
  memcpy(&buf[1], pData, size);

  //return _mt_i2c_write_sony(deviceAddress, buf, size + 1);
  result = _mt_i2c_write_sony(deviceAddress, buf, size + 1);

  return result;
}

sony_result_t Sony_I2C_WriteOneRegister(struct sony_i2c_t *pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
{
  sony_result_t result;

  uint8_t buf[2];

  buf[0] = subAddress;
  buf[1] = data;

  //return _mt_i2c_write_sony(deviceAddress, buf, 2);
  result = _mt_i2c_write_sony(deviceAddress, buf, 2);

  //printk("%s[%d] ---- devAddr[0x%02x], RegAddr[0x%02x], RegData[0x%02x], result[%d]\n", __FUNCTION__, __LINE__, deviceAddress, subAddress, data, result);

  return result;
}

mt_unf_fe_fec_type_t port_sony_cxd_deparse_dvb_type(sony_dtv_system_t dtv_type)
{
  mt_unf_fe_fec_type_t temp;

  switch (dtv_type)
  {
  case SONY_DTV_SYSTEM_DVBS:
    temp = MT_UNF_FE_DVBS;
    break;

  case SONY_DTV_SYSTEM_DVBS2:
    temp = MT_UNF_FE_DVBS2;
    break;

  default:
    temp = MT_UNF_FE_BUTT; //NIM_UNDEF;
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

static int port_sony_cxd_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
  //MT_U32 for_scan = 0;
  sony_result_t ret = SONY_RESULT_OK, result = SONY_RESULT_OK;
  sony_dtv_system_t dtv_type = SONY_DTV_SYSTEM_ANY;
  mt_unf_fe_channel_info_t *p_channel_info = NULL;
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  //printk("%s[%d][%d] ---- handle = 0x%08x, sony_handle = 0x%08x\n", __FUNCTION__, __LINE__, gSonyIndex, handle, sony_cxd_handle);

  p_channel_info = &(para->channel_info);
  para->channel_info.lock = 0;

  gFeList[gFeIndex] = para->sig_type;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  if ((para->sig_type == MT_UNF_FE_SIG_TYPE_SAT) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_SAT_2) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_DVBS_AUTO) ||
      (para->sig_type == (MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2)))
  {
    sony_dvbs_s2_tune_param_t sony_ss2_param;

    if (MT_UNF_PORT_TYPE_DVBS == para->connect_param.sat.port_type)
    {
      dtv_type = SONY_DTV_SYSTEM_DVBS;
    }
    else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
    {
      dtv_type = SONY_DTV_SYSTEM_DVBS2;
    }
    else
    {
      dtv_type = SONY_DTV_SYSTEM_ANY;
    }

    printk("sony_cxd_channel set ss2: freq = %d, sym = %d, type = %d, use_uc = %d\n",
           para->connect_param.sat.freq,
           para->connect_param.sat.sym_rate,
           para->connect_param.sat.port_type,
           para->connect_param.sat.uc_param.use_uc);

    if (para->connect_param.sat.uc_param.use_uc)
    {
    }
    else
    {
    }

    //memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

    sony_ss2_param.centerFreqKHz = para->connect_param.sat.freq;
    sony_ss2_param.symbolRateKSps = para->connect_param.sat.sym_rate;
    sony_ss2_param.system = dtv_type;

    ret = sony_integ_dvbs_s2_Tune(&(sony_cxd_handle->integ), &sony_ss2_param);

    if (ret == SONY_RESULT_OK)
    {
      para->channel_info.lock = 1;

      //printk("%s[%d] ---- Sony DVB-S or S2 Locked!\n", __FUNCTION__, __LINE__);
    }
    else
    {
      para->channel_info.lock = 0;

      //printk("%s[%d] ---- Sony DVB-S or S2 Unlock!\n", __FUNCTION__, __LINE__);
    }

    para->channel_set_info.lock_time = 2000;
  }
  else
#endif
  if ((para->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2) ||
      (para->sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO) ||
      (para->sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)))
  {
    sony_dtv_bandwidth_t sony_tt2_bw = SONY_DTV_BW_8_MHZ;
    sony_dvbt2_profile_t profile = SONY_DVBT2_PROFILE_ANY;
    sony_dtv_system_t sony_tuned_system = SONY_DTV_SYSTEM_ANY;
    sony_dvbt2_profile_t sony_tuned_profile;

    switch (para->connect_param.ter.band_width)
    {
    case 1700:
      sony_tt2_bw = SONY_DTV_BW_1_7_MHZ;
      para->channel_set_info.lock_time = 5000;
      break;

    case 5000:
      sony_tt2_bw = SONY_DTV_BW_5_MHZ;
      para->channel_set_info.lock_time = 4000;
      break;

    case 6000:
      sony_tt2_bw = SONY_DTV_BW_6_MHZ;
      para->channel_set_info.lock_time = 3000;
      break;

    case 7000:
      sony_tt2_bw = SONY_DTV_BW_7_MHZ;
      para->channel_set_info.lock_time = 2500;
      break;

    case 8000:
    default:
      sony_tt2_bw = SONY_DTV_BW_8_MHZ;
      para->channel_set_info.lock_time = 2000;
      break;
    }

    //memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

    if (MT_UNF_FE_SIG_TYPE_DVB_T == para->sig_type)
    {
      dtv_type = SONY_DTV_SYSTEM_DVBT;
      ret = sony_integ_dvbt_BlindTune(&(sony_cxd_handle->integ),
                                      para->connect_param.ter.freq,
                                      sony_tt2_bw);

      if (ret == SONY_RESULT_OK)
      {
        para->channel_info.lock = 1;

        printk("%s[%d] ---- Sony DVB-T Locked!\n", __FUNCTION__, __LINE__);
      }
      else
      {
        printk("%s[%d] ---- Sony DVB-T Unlock!\n", __FUNCTION__, __LINE__);
      }

      sony_cxd_handle->demod.system = dtv_type;
    }
    else if (MT_UNF_FE_SIG_TYPE_DVB_T2 == para->sig_type)
    {
#if 0
      dtv_type = SONY_DTV_SYSTEM_DVBT2;

      ret = sony_integ_dvbt2_BlindTune(&(sony_cxd_handle->integ),
                                       para->connect_param.ter.freq,
                                       sony_tt2_bw,
                                       profile,
                                       &sony_tuned_profile);

      if (ret == SONY_RESULT_OK)
      {
        sony_result_t tmpResult;

        para->channel_info.lock = 1;
        printk("%s[%d] ---- Sony DVB-T2 Locked!\n", __FUNCTION__, __LINE__);

        tmpResult = sony_demod_dvbt2_monitor_DataPLPs(&(sony_cxd_handle->demod), 
                                                      para->connect_param.ter.data_plpid_array,
                                                      &(para->connect_param.ter.data_plp_number)
                                                     );

        if (tmpResult == SONY_RESULT_OK)
        {
          int cnt = 0;

          printk("%s[%d] ---- DVB-T2 total %d PLPs.\n", __FUNCTION__, __LINE__, para->connect_param.ter.data_plp_number);

          for (cnt = 0; cnt < para->connect_param.ter.data_plp_number; cnt++)
          {
            printk("%s[%d] --- PLP[%3d] ---- ID[%02x]\n", __FUNCTION__, __LINE__, cnt, para->connect_param.ter.data_plpid_array[cnt]);
          }
        }
      }
      else
      {
        sony_demod_lock_result_t stat = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

        sony_demod_dvbt2_CheckTSLock(&(sony_cxd_handle->demod), &stat);

        if (stat == SONY_DEMOD_LOCK_RESULT_LOCKED)
        {
          printk("%s[%d] ---- Sony DVB-T2 Locked!\n", __FUNCTION__, __LINE__);
        }
        else
          printk("%s[%d] ---- Sony DVB-T2 Unlock!\n", __FUNCTION__, __LINE__);
      }
#else
      sony_dvbt2_tune_param_t t2_tune_param;

      t2_tune_param.centerFreqKHz = para->connect_param.ter.freq;
	  if(para->connect_param.ter.channel_mode == MT_UNF_FE_TER_MODE_LITE)
		t2_tune_param.profile = SONY_DVBT2_PROFILE_LITE;
	  else
      	t2_tune_param.profile = SONY_DVBT2_PROFILE_BASE;
	  
      t2_tune_param.bandwidth = sony_tt2_bw;
      t2_tune_param.dataPlpId = para->connect_param.ter.plp_id;
	  printk("%s[%d] centerFreqKHz:%d,bandwidth:%d,profile:%d,dataPlpId:%d!\n", __FUNCTION__, __LINE__,t2_tune_param.centerFreqKHz
																			,t2_tune_param.bandwidth,t2_tune_param.profile,t2_tune_param.dataPlpId);
      ret = sony_integ_dvbt2_Tune(&(sony_cxd_handle->integ), &t2_tune_param);
      if (ret == SONY_RESULT_OK)
      {
        sony_result_t tmpResult;

        para->channel_info.lock = 1;
        printk("%s[%d] ---- Sony DVB-T2 Locked!\n", __FUNCTION__, __LINE__);

        tmpResult = sony_demod_dvbt2_monitor_DataPLPs(&(sony_cxd_handle->demod), 
                                                      para->connect_param.ter.data_plpid_array,
                                                      &(para->connect_param.ter.data_plp_number)
                                                     );

        if (tmpResult == SONY_RESULT_OK)
        {
          int cnt = 0;

          printk("%s[%d] ---- DVB-T2 total %d PLPs.\n", __FUNCTION__, __LINE__, para->connect_param.ter.data_plp_number);

          for (cnt = 0; cnt < para->connect_param.ter.data_plp_number; cnt++)
          {
            printk("%s[%d] --- PLP[%3d] ---- ID[%02x]\n", __FUNCTION__, __LINE__, cnt, para->connect_param.ter.data_plpid_array[cnt]);
          }
        }
		else
		{
			printk("%s[%d] sony_demod_dvbt2_monitor_DataPLPs failed :%d \n", __FUNCTION__, __LINE__, tmpResult);
		}
      }
      else
      {
        sony_demod_lock_result_t stat = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

        sony_demod_dvbt2_CheckTSLock(&(sony_cxd_handle->demod), &stat);

        if (stat == SONY_DEMOD_LOCK_RESULT_LOCKED)
        {
          printk("%s[%d] ---- Sony DVB-T2 Locked!\n", __FUNCTION__, __LINE__);
        }
        else
          printk("%s[%d] ---- Sony DVB-T2 Unlock!\n", __FUNCTION__, __LINE__);
      }

#endif

      dtv_type = SONY_DTV_SYSTEM_DVBT2;

      sony_cxd_handle->demod.system = dtv_type;
    }
    else
    {
      dtv_type = SONY_DTV_SYSTEM_ANY;

      ret = sony_integ_dvbt_t2_BlindTune(&(sony_cxd_handle->integ),
                                         para->connect_param.ter.freq,
                                         sony_tt2_bw,
                                         dtv_type,
                                         profile,
                                         &sony_tuned_system,
                                         &sony_tuned_profile);

      if (sony_tuned_system == SONY_DTV_SYSTEM_DVBT)
      {
        para->channel_info.lock = 1;

        if ((p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_R836) || 
            (p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_RAFAEL836))
        {
          sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x01);

          R840_AGC_Slow();

          sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x00);
        }

        para->connect_param.ter.port_type = MT_UNF_PORT_TYPE_DVBT;

        printk("%s[%d] ---- Sony DVB-T Locked!\n", __FUNCTION__, __LINE__);

        sony_cxd_handle->demod.system = SONY_DTV_SYSTEM_DVBT;
      }
      else if (sony_tuned_system == SONY_DTV_SYSTEM_DVBT2)
      {
        sony_result_t tmpResult;

        para->channel_info.lock = 1;

        if ((p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_R836) || 
            (p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_RAFAEL836))
        {
          sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x01);

          R840_AGC_Slow();

          sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x00);
        }

        printk("%s[%d] ---- Sony DVB-T2 Locked!\n", __FUNCTION__, __LINE__);

        tmpResult = sony_demod_dvbt2_monitor_DataPLPs(&(sony_cxd_handle->demod), 
                                                      para->connect_param.ter.data_plpid_array, 
                                                      &(para->connect_param.ter.data_plp_number)
                                                     );

        if (tmpResult == SONY_RESULT_OK)
        {
          int cnt = 0;

          printk("%s[%d] ---- DVB-T2 total %d PLPs.\n", __FUNCTION__, __LINE__, para->connect_param.ter.data_plp_number);

          for (cnt = 0; cnt < para->connect_param.ter.data_plp_number; cnt++)
          {
            printk("%s[%d] --- PLP[%3d] ---- ID[%02x]\n", __FUNCTION__, __LINE__, cnt, para->connect_param.ter.data_plpid_array[cnt]);
          }
        }

        if (para->connect_param.ter.data_plp_number)
        {
          result = sony_integ_dvbt2_Scan_SwitchDataPLP(&(p_priv->sony_cxd_handle)->integ, 1, (uint8_t)para->connect_param.ter.plp_id, sony_tuned_profile);
          printk("%s[%d] ---- DVB-T2 switch to PLP[%02x] %s!\n", __FUNCTION__, __LINE__, (uint8_t)para->connect_param.ter.plp_id, (result == SONY_RESULT_OK) ? "OK" : "failed");
        }

        sony_cxd_handle->demod.system = SONY_DTV_SYSTEM_DVBT2;
		if(sony_tuned_profile == SONY_DVBT2_PROFILE_LITE)
		{	
			para->connect_param.ter.channel_mode = MT_UNF_FE_TER_MODE_LITE;
			p_priv->param.connect_param.ter.channel_mode = MT_UNF_FE_TER_MODE_LITE;
		}
		else
		{
			para->connect_param.ter.channel_mode = MT_UNF_FE_TER_MODE_BASE;
			p_priv->param.connect_param.ter.channel_mode = MT_UNF_FE_TER_MODE_BASE;
		}
		printk("%s[%d] ---- DVB-T2 channel_mode %s .\n", __FUNCTION__, __LINE__, (para->connect_param.ter.channel_mode == MT_UNF_FE_TER_MODE_BASE) ? "base_profile" : "lite_profile");
      }
      else
      {
        para->channel_info.lock = 0;
        printk("%s[%d] ---- BlindTune unlock!\n", __FUNCTION__, __LINE__);

        sony_cxd_handle->demod.system = SONY_DTV_SYSTEM_ANY;
      }
    }

    printk("sony_cxd_channel_set() tt2: freq = %d, bandwidth = %d, type = %d, dtv_type = %d, ret = %d\n",
           para->connect_param.ter.freq,
           para->connect_param.ter.band_width,
           para->connect_param.ter.port_type,
           dtv_type,
           ret);
  }
  else if (para->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
  {
    sony_dvbc_tune_param_t sony_c_param;

    dtv_type = SONY_DTV_SYSTEM_DVBC;

    printk("sony_cxd_channel set c: freq = %d, symbol rate = %d, bandwidth = %d, type = %d\n",
           para->connect_param.cab.freq,
           para->connect_param.cab.sym_rate,
           para->connect_param.cab.band_width,
           dtv_type);

    //memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

    sony_c_param.centerFreqKHz = para->connect_param.cab.freq;
    //sony_c_param.bandwidth = para->connect_param.cab.band_width;
    switch (para->connect_param.ter.band_width)
    {
    case 6000:
      sony_c_param.bandwidth = SONY_DTV_BW_6_MHZ;
      break;

    case 7000:
      sony_c_param.bandwidth = SONY_DTV_BW_7_MHZ;
      break;

    case 8000:
    default:
      sony_c_param.bandwidth = SONY_DTV_BW_8_MHZ;
      break;
    }

	para->channel_set_info.lock_time = 1000;

    ret = sony_integ_dvbc_Tune(&(sony_cxd_handle->integ), &sony_c_param);

    if (ret == SONY_RESULT_OK)
    {
      para->channel_info.lock = 1;

#if 0
      if ((&p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_R836) || 
          (&p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_RAFAEL836))
      {
        sony_demod_I2cRepeaterEnable(&sony_cxd2856_handle->demod, 0x01);

        R840_AGC_Slow();

        sony_demod_I2cRepeaterEnable(&sony_cxd2856_handle->demod, 0x00);
      }
#endif

      //printk("%s[%d] ---- Sony DVB-C Locked!\n", __FUNCTION__, __LINE__);
    }
    else
    {
      para->channel_info.lock = 0;

      //printk("%s[%d] ---- Sony DVB-C Unlock!\n", __FUNCTION__, __LINE__);
    }

    para->channel_set_info.lock_time = 800;

    sony_cxd_handle->demod.system = dtv_type;
  }
  else if (para->sig_type == MT_UNF_FE_SIG_TYPE_J83B)
  {
    sony_j83b_tune_param_t sony_b_param;

    dtv_type = SONY_DTV_SYSTEM_J83B;

    printk("sony_cxd_channel set b: freq = %d, symbol rate = %d, type = %d\n",
           para->connect_param.cab.freq,
           para->connect_param.cab.sym_rate,
           dtv_type);

    //memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

    sony_b_param.centerFreqKHz = para->connect_param.cab.freq;
    //sony_b_param.bandwidth = para->connect_param.cab.band_width;
    sony_b_param.bandwidth = SONY_DTV_BW_J83B_5_06_5_36_MSPS;

    ret = sony_integ_j83b_Tune(&(sony_cxd_handle->integ), &sony_b_param);

    if (ret == SONY_RESULT_OK)
    {
      para->channel_info.lock = 1;

#if 0
      if ((&p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_R836) || 
          (&p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_RAFAEL836))
      {
        sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x01);

        R840_AGC_Slow();

        sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x00);
      }
#endif

      //printk("%s[%d] ---- Sony J.83B Locked!\n", __FUNCTION__, __LINE__);
    }
    else
    {
      para->channel_info.lock = 0;

      //printk("%s[%d] ---- Sony J.83B Unlock!\n", __FUNCTION__, __LINE__);
    }

    para->channel_set_info.lock_time = 1000;

    sony_cxd_handle->demod.system = dtv_type;
  }

  //sony_cxd_handle->demod.system = dtv_type;

  memcpy(&p_priv->param, para, sizeof(mt_unf_fe_connect_para_t));

  if (ret == SONY_RESULT_OK)
  {
    return MT_SUCCESS;
  }

  //return MT_FAILURE;
  return MT_SUCCESS;
}

#if 0 //def SONY_DEMOD_SUPPORT_DVBS_S2
static void port_sony_cxd_set_22k_onoff(void *handle, uint8_t onoff_22k, uint8_t diseqc_out_when_lnb_off)
{
  uint8_t val_0xa1, val_0xa2;

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_instance_t * sony_cxd_handle = p_priv->sony_cxd_handle;


  printk("port_sony_cxd_set_22k_onoff %d - %s\n", onoff_22k, (onoff_22k == 1) ? "22K On" : "22K Off");
}

static void port_sony_cxd_set_lnb_voltage(void *handle, MT_FE_LNB_VOLTAGE voltage, uint8_t vsel_pin_13v)
{

}

static void port_sony_cxd_set_lnb_onoff(void *handle, uint8_t lnb_enable, mt_unf_fe_pin_config_para_t *pin_config)
{
}

static int port_sony_cxd_diseqc_sendmsg(void *handle, mt_unf_fe_diseqc_sendmsg_t *p_diseqc_sendmsg)
{
  sony_result_t ret = SONY_RESULT_OK;

  return (ret == SONY_RESULT_OK) ? MT_SUCCESS : MT_FAILURE;
}

static int port_sony_cxd_diseqc_send_tone_burst(void *handle, uint8_t mode)
{
  sony_result_t ret = SONY_RESULT_OK;


  return (ret == SONY_RESULT_OK) ? MT_SUCCESS : MT_FAILURE;
}

static int port_sony_cxd_diseqc_recvmsg(void *handle, mt_unf_fe_diseqc_recvmsg_t *p_diseqc_recvmsg)
{
  sony_result_t ret = SONY_RESULT_OK;

  return (ret == SONY_RESULT_OK) ? MT_SUCCESS : MT_FAILURE;
}
#endif

static int port_sony_cxd_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;

  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  sony_demod_lock_result_t stat = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

  sony_result_t result = SONY_RESULT_OK;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd_handle->demod)->system);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
    sony_demod_dvbc_CheckTSLock(&(sony_cxd_handle->demod), &stat);
    p_status->param.sig_type = MT_UNF_FE_SIG_TYPE_CAB;
    p_status->param.channel_info.port_type = MT_UNF_FE_DVBC;
    break;

  case SONY_DTV_SYSTEM_DVBC2:
    sony_demod_dvbc2_CheckTSLock(&(sony_cxd_handle->demod), &stat);
    break;

  case SONY_DTV_SYSTEM_DVBT:
  	/*fix issue29887 in order to get lock faster
	* use sony_demod_dvbt_CheckDemodLock instead of sony_demod_dvbt_CheckTSLock
	*/
	sony_demod_dvbt_CheckDemodLock(&(sony_cxd_handle->demod), &stat);
    p_status->param.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
    p_status->param.channel_info.port_type = MT_UNF_FE_DVBT;
    break;

  case SONY_DTV_SYSTEM_DVBT2:
   	/*fix issue29887 in order to get lock faster
	* use sony_demod_dvbt2_CheckDemodLock instead of sony_demod_dvbt2_CheckTSLock
	*/
	sony_demod_dvbt2_CheckDemodLock(&(sony_cxd_handle->demod), &stat);
    p_status->param.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
    p_status->param.channel_info.port_type = MT_UNF_FE_DVBT2;
    break;

  case SONY_DTV_SYSTEM_J83B:
    sony_demod_j83b_CheckTSLock(&(sony_cxd_handle->demod), &stat);
    break;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  case SONY_DTV_SYSTEM_DVBS:
  case SONY_DTV_SYSTEM_DVBS2:
    sony_demod_dvbs_s2_CheckTSLock(&(sony_cxd_handle->demod), &stat);
    break;
#endif

  default:
    if ((p_status->param.sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO) ||
        (p_status->param.sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)))
    {
    }
    break;
  }

  if (stat == SONY_DEMOD_LOCK_RESULT_LOCKED)
  {
    p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;


    switch ((&sony_cxd_handle->demod)->system)
    {
      case SONY_DTV_SYSTEM_DVBC:
      case SONY_DTV_SYSTEM_DVBC2:
      case SONY_DTV_SYSTEM_DVBT:
      case SONY_DTV_SYSTEM_DVBT2:
        if ((p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_R836) || 
            (p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_RAFAEL836))
        {
          result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x01);

          R840_AGC_Slow();

          //printk("%s[%d] ---- R840_AGC_Slow()\n", __FUNCTION__, __LINE__);

          result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x00);
        }
        break;

      default:
        break;
    }

    //printk("%s[%d] ---- system[%d] Locked!\n", __FUNCTION__, __LINE__, (&sony_cxd2856_handle->demod)->system);
  }
  else
  {
    p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
    if (stat == SONY_DEMOD_LOCK_RESULT_UNLOCKED)
      p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
    else
      p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;

    //printk("%s[%d] ---- system[%d] Unlock!\n", __FUNCTION__, __LINE__, (&sony_cxd2856_handle->demod)->system);
  }

  return MT_SUCCESS;
}

static int port_sony_cxd_get_signal_quality(void *handle, MT_U32 *p_quality)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  uint8_t percent = 0;
  int32_t snr = 0;

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd_handle->demod)->system);

  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
    sony_demod_dvbc_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    percent = (uint8_t)(snr * 2);
    break;

  case SONY_DTV_SYSTEM_DVBC2:
    sony_demod_dvbc2_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    percent = (uint8_t)(snr * 2);
    break;

  case SONY_DTV_SYSTEM_DVBT:
    sony_demod_dvbt_monitor_Quality(&(sony_cxd_handle->demod), &percent);
    break;

  case SONY_DTV_SYSTEM_DVBT2:
    sony_demod_dvbt2_monitor_Quality(&(sony_cxd_handle->demod), &percent);
    break;

  case SONY_DTV_SYSTEM_J83B:
    sony_demod_j83b_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    percent = (uint8_t)(snr * 2);
    break;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  case SONY_DTV_SYSTEM_DVBS:
  case SONY_DTV_SYSTEM_DVBS2:
    sony_demod_dvbs_s2_monitor_CNR(&(sony_cxd_handle->demod), &snr);
    percent = (uint8_t)(snr * 3);
    break;
#endif

  default:
    break;
  }

  if (percent > 100)
    percent = 100;

  *p_quality = percent;

  return MT_SUCCESS;
}

static int port_sony_cxd_get_ber(void *handle, MT_U32 *p_ber)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  MT_U32 ber = 0;
  MT_U32 tmp = 1;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd2856_handle->demod)->system);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd2856_priv_handle)handle;
  sony_cxd_handle = p_priv->sony_cxd_handle;

  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
    sony_demod_dvbc_monitor_PER(&sony_cxd_handle->demod, &ber);
    break;

  case SONY_DTV_SYSTEM_DVBC2:
    sony_demod_dvbc2_monitor_PER(&sony_cxd_handle->demod, &ber);
    break;

  case SONY_DTV_SYSTEM_DVBT:
    sony_demod_dvbt_monitor_PER(&(sony_cxd_handle->demod), &ber);
    break;

  case SONY_DTV_SYSTEM_DVBT2:
    sony_demod_dvbt2_monitor_PER(&(sony_cxd_handle->demod), &ber);
    break;

  case SONY_DTV_SYSTEM_J83B:
    sony_demod_j83b_monitor_PER(&(sony_cxd_handle->demod), &ber);
    break;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  case SONY_DTV_SYSTEM_DVBS:
  case SONY_DTV_SYSTEM_DVBS2:
    sony_demod_dvbs_s2_monitor_PER(&(sony_cxd_handle->demod), &ber);
    break;
#endif

  default:
    break;
  }

  p_ber[0] = ber;
  p_ber[1] = 0;
  p_ber[2] = 0;

  while (p_ber[0] >= 10)
  {
    p_ber[0] /= 10;
    tmp *= 10;
    p_ber[2]++;
  }

  if (tmp > 10)
  {
    p_ber[1] = ber % tmp;
  }
  else
  {
    p_ber[1] = 0;
  }

  return MT_SUCCESS;
}
static int port_sony_cxd_get_accurate_snr(void *handle, MT_S32 *p_snr)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  int32_t snr = 0;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd_handle->demod)->system);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;


  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
    sony_demod_dvbc_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_DVBC2:
    sony_demod_dvbc2_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_DVBT:
    sony_demod_dvbt_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_DVBT2:
    sony_demod_dvbt2_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_J83B:
    sony_demod_j83b_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  case SONY_DTV_SYSTEM_DVBS:
  case SONY_DTV_SYSTEM_DVBS2:
    sony_demod_dvbs_s2_monitor_CNR(&(sony_cxd_handle->demod), &snr);
    break;
#endif

  default:
    break;
  }

  *p_snr = snr;

  return MT_SUCCESS;
}
static int port_sony_cxd_get_snr(void *handle, MT_U32 *p_snr)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  int32_t snr = 0;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd_handle->demod)->system);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  
  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
    sony_demod_dvbc_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_DVBC2:
    sony_demod_dvbc2_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_DVBT:
    sony_demod_dvbt_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_DVBT2:
    sony_demod_dvbt2_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

  case SONY_DTV_SYSTEM_J83B:
    sony_demod_j83b_monitor_SNR(&(sony_cxd_handle->demod), &snr);
    break;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  case SONY_DTV_SYSTEM_DVBS:
  case SONY_DTV_SYSTEM_DVBS2:
    sony_demod_dvbs_s2_monitor_CNR(&(sony_cxd_handle->demod), &snr);
    break;
#endif

  default:
    break;
  }

  *p_snr = snr / 1000;

  return MT_SUCCESS;
}

static int port_sony_cxd_get_signal_strength(void *handle, MT_U32 *p_strength)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;
  int32_t _strength = 0;
  sony_result_t result = SONY_RESULT_OK;
  mt_unf_fe_status_t status;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd_handle->demod)->system);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  port_sony_cxd_get_status(handle, &status);

  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
  case SONY_DTV_SYSTEM_DVBC2:
  case SONY_DTV_SYSTEM_DVBT:
  case SONY_DTV_SYSTEM_DVBT2:
  case SONY_DTV_SYSTEM_J83B:
    result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x01);

	(&sony_cxd_handle->tuner)->ReadRFLevel(&sony_cxd_handle->tuner, &_strength);

	result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x00);
    break;

  default:
    break;
  }

  if (status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
  {
    if (_strength < -70)
      _strength = -70;
    if (_strength > -25)
      _strength = -25;

    *p_strength = 10 + (_strength + 70) * 2; /* 10% ~ 100% */
  }
  else
  {
    if (_strength > -60)
      _strength = -60;
    if (_strength < -80)
      _strength = -80;

    *p_strength = (_strength + 80) / 2; /*  0% ~  10% */
  }

  return MT_SUCCESS;
}

static void port_sony_cxd_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  sony_result_t result = SONY_RESULT_OK;
  sony_dvbt2_l1pre_t dvbt2_l1pre;
  sony_dvbt2_plp_t dvbt2_plpinfo;
  sony_dvbt_tpsinfo_t dvbt_tpsinfo;

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  /*fix issue 26814 diff struct copy cause kernel panic*/
  //memcpy(&(p_sig_info->sig_info), &(p_priv->param.connect_param), sizeof(p_priv->param.connect_param));
  if((sony_cxd_handle->demod.system == SONY_DTV_SYSTEM_DVBT)||(sony_cxd_handle->demod.system == SONY_DTV_SYSTEM_DVBT2))
  {
    p_sig_info->sig_info.ter.freq = p_priv->param.connect_param.ter.freq;
    p_sig_info->sig_info.ter.band_width = p_priv->param.connect_param.ter.band_width;
    p_sig_info->sig_info.ter.enModType = p_priv->param.connect_param.ter.mode_type;
    p_sig_info->sig_info.ter.cell_id = p_priv->param.connect_param.ter.cell_id;
  }
  else
  {
    pr_err("ERR:%s[%d] ---- UNSUPPORT sony_cxd_handle->demod.system = %d\n", __FUNCTION__, __LINE__, sony_cxd_handle->demod.system);
  }

  //printk("%s[%d] ---- sony_cxd_handle->demod.system = %d\n", __FUNCTION__, __LINE__, sony_cxd_handle->demod.system);

  switch (sony_cxd_handle->demod.system)
  {
    case SONY_DTV_SYSTEM_DVBT:
      p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;

      p_sig_info->sig_info.ter.ter_type = MT_UNF_FE_DVBT;

      result = sony_demod_dvbt_monitor_TPSInfo(&(sony_cxd_handle->demod), 
                                      &dvbt_tpsinfo);

      switch (dvbt_tpsinfo.constellation)
      {
        case SONY_DVBT_CONSTELLATION_QPSK:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QPSK;
          break;

        case SONY_DVBT_CONSTELLATION_16QAM:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_16;
          break;

        case SONY_DVBT_CONSTELLATION_64QAM:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_64;
          break;

        case SONY_DVBT_CONSTELLATION_RESERVED_3:
        default:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_DEFAULT;
          break;
      }

      switch (dvbt_tpsinfo.hierarchy)
      {
        case SONY_DVBT_HIERARCHY_1:
          p_sig_info->sig_info.ter.enHierMod = MT_UNF_FE_HIERARCHY_ALHPA1;
          break;

        case SONY_DVBT_HIERARCHY_2:
          p_sig_info->sig_info.ter.enHierMod = MT_UNF_FE_HIERARCHY_ALHPA2;
          break;

        case SONY_DVBT_HIERARCHY_4:
          p_sig_info->sig_info.ter.enHierMod = MT_UNF_FE_HIERARCHY_ALHPA4;
          break;

        case SONY_DVBT_HIERARCHY_NON:
        default:
          p_sig_info->sig_info.ter.enHierMod = MT_UNF_FE_HIERARCHY_NO;
          break;
      }

      switch (dvbt_tpsinfo.rateHP)
      {
        case SONY_DVBT_CODERATE_1_2:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_1_2;
          break;

        case SONY_DVBT_CODERATE_2_3:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_2_3;
          break;

        case SONY_DVBT_CODERATE_3_4:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_3_4;
          break;

        case SONY_DVBT_CODERATE_5_6:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_5_6;
          break;

        case SONY_DVBT_CODERATE_7_8:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_7_8;
          break;

        case SONY_DVBT_CODERATE_RESERVED_5:
        case SONY_DVBT_CODERATE_RESERVED_6:
        case SONY_DVBT_CODERATE_RESERVED_7:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_RESERVED;
          break;

        default:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_UNDEF;
          break;
      }

      switch (dvbt_tpsinfo.rateLP)
      {
        case SONY_DVBT_CODERATE_1_2:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_1_2;
          break;

        case SONY_DVBT_CODERATE_2_3:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_2_3;
          break;

        case SONY_DVBT_CODERATE_3_4:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_3_4;
          break;

        case SONY_DVBT_CODERATE_5_6:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_5_6;
          break;

        case SONY_DVBT_CODERATE_7_8:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_7_8;
          break;

        case SONY_DVBT_CODERATE_RESERVED_5:
        case SONY_DVBT_CODERATE_RESERVED_6:
        case SONY_DVBT_CODERATE_RESERVED_7:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_RESERVED;
          break;

        default:
          p_sig_info->sig_info.ter.enLowPriFECRate = MT_UNF_FE_FEC_UNDEF;
          break;
      }

      switch (dvbt_tpsinfo.guard)
      {
        case SONY_DVBT_GUARD_1_4:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_4;
          break;

        case SONY_DVBT_GUARD_1_8:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_8;
          break;

        case SONY_DVBT_GUARD_1_16:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_16;
          break;

        case SONY_DVBT_GUARD_1_32:
        default:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_32;
          break;
      }

      switch (dvbt_tpsinfo.mode)
      {
        case SONY_DVBT_MODE_2K:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_2K;
          break;

        case SONY_DVBT_MODE_8K:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_8K;
          break;

        case SONY_DVBT_MODE_RESERVED_2:
        case SONY_DVBT_MODE_RESERVED_3:
        default:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_DEFAULT;
          break;
      }

      p_sig_info->sig_info.ter.cell_id = dvbt_tpsinfo.cellID;

      break;


    case SONY_DTV_SYSTEM_DVBT2:
      p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;

      p_sig_info->sig_info.ter.ter_type = MT_UNF_FE_DVBT2;

      result = sony_demod_dvbt2_monitor_L1Pre(&(sony_cxd_handle->demod), 
                                              &dvbt2_l1pre);

#if 0
      switch (dvbt2_l1pre.mod)
      {
        case SONY_DVBT2_L1POST_BPSK:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_BPSK;
          break;

        case SONY_DVBT2_L1POST_QPSK:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QPSK;
          break;

        case SONY_DVBT2_L1POST_QAM16:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_16;
          break;

        case SONY_DVBT2_L1POST_QAM64:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_64;
          break;

        case SONY_DVBT2_L1POST_C_RSVD1:
        case SONY_DVBT2_L1POST_C_RSVD2:
        case SONY_DVBT2_L1POST_C_RSVD3:
        case SONY_DVBT2_L1POST_C_RSVD4:
        case SONY_DVBT2_L1POST_C_RSVD5:
        case SONY_DVBT2_L1POST_C_RSVD6:
        case SONY_DVBT2_L1POST_C_RSVD7:
        case SONY_DVBT2_L1POST_C_RSVD8:
        case SONY_DVBT2_L1POST_C_RSVD9:
        case SONY_DVBT2_L1POST_C_RSVD10:
        case SONY_DVBT2_L1POST_C_RSVD11:
        case SONY_DVBT2_L1POST_C_RSVD12:
        case SONY_DVBT2_L1POST_CONSTELL_UNKNOWN:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_DEFAULT;
          break;
      }
#endif


      switch (dvbt2_l1pre.gi)
      {
        case SONY_DVBT2_G1_32:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_32;
          break;

        case SONY_DVBT2_G1_16:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_16;
          break;

        case SONY_DVBT2_G1_8:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_8;
          break;

        case SONY_DVBT2_G1_4:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_4;
          break;

        case SONY_DVBT2_G1_128:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_1_128;
          break;

        case SONY_DVBT2_G19_128:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_19_128;
          break;

        case SONY_DVBT2_G19_256:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_19_256;
          break;

        case SONY_DVBT2_G_RSVD1:
        case SONY_DVBT2_G_UNKNOWN:
        default:
          p_sig_info->sig_info.ter.enGuardIntv = MT_UNF_FE_GUARD_INTV_DEFALUT;
          break;
      }

      switch (dvbt2_l1pre.fftMode)
      {
        case SONY_DVBT2_M2K:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_2K;
          break;

        case SONY_DVBT2_M8K:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_8K;
          break;

        case SONY_DVBT2_M1K:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_1K;
          break;

        case SONY_DVBT2_M16K:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_16K;
          break;

        case SONY_DVBT2_M32K:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_32K;
          break;

        case SONY_DVBT2_M_RSVD1:
        case SONY_DVBT2_M_RSVD2:
        default:
          p_sig_info->sig_info.ter.enFFTMode = MT_UNF_FE_FFT_DEFAULT;
          break;
      }

      switch (dvbt2_l1pre.pp)
      {
        case SONY_DVBT2_PP1:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP1;
          break;

        case SONY_DVBT2_PP2:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP2;
          break;

        case SONY_DVBT2_PP3:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP3;
          break;

        case SONY_DVBT2_PP4:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP4;
          break;

        case SONY_DVBT2_PP5:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP5;
          break;

        case SONY_DVBT2_PP6:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP6;
          break;

        case SONY_DVBT2_PP7:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP7;
          break;

        case SONY_DVBT2_PP8:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_PP8;
          break;

        case SONY_DVBT2_PP_RSVD1:
        case SONY_DVBT2_PP_RSVD2:
        case SONY_DVBT2_PP_RSVD3:
        case SONY_DVBT2_PP_RSVD4:
        case SONY_DVBT2_PP_RSVD5:
        case SONY_DVBT2_PP_RSVD6:
        case SONY_DVBT2_PP_RSVD7:
        case SONY_DVBT2_PP_RSVD8:
        default:
          p_sig_info->sig_info.ter.enPilotPattern = MT_UNF_FE_T2_PILOT_PATTERN_BUTT;
          break;
      }

      p_sig_info->sig_info.ter.cell_id = dvbt2_l1pre.cellId;


      sony_demod_dvbt2_monitor_ActivePLP(&(sony_cxd_handle->demod),
                                         SONY_DVBT2_PLP_DATA,
                                         &dvbt2_plpinfo);

      switch (dvbt2_plpinfo.type)
      {
        case SONY_DVBT2_PLP_TYPE_COMMON:
          p_sig_info->sig_info.ter.enPLPType = MT_UNF_FE_T2_PLP_TYPE_COM;
          break;

        case SONY_DVBT2_PLP_TYPE_DATA1:
          p_sig_info->sig_info.ter.enPLPType = MT_UNF_FE_T2_PLP_TYPE_DAT1;
          break;

        case SONY_DVBT2_PLP_TYPE_DATA2:
          p_sig_info->sig_info.ter.enPLPType = MT_UNF_FE_T2_PLP_TYPE_DAT2;
          break;

        case SONY_DVBT2_PLP_TYPE_RSVD1:
        case SONY_DVBT2_PLP_TYPE_RSVD2:
        case SONY_DVBT2_PLP_TYPE_RSVD3:
        case SONY_DVBT2_PLP_TYPE_RSVD4:
        default:
          p_sig_info->sig_info.ter.enPLPType = MT_UNF_FE_T2_PLP_TYPE_BUTT;
          break;
      }

      switch (dvbt2_plpinfo.constell)
      {
        case SONY_DVBT2_QPSK:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QPSK;
          break;

        case SONY_DVBT2_QAM16:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_16;
          break;

        case SONY_DVBT2_QAM64:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_64;
          break;

        case SONY_DVBT2_QAM256:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_QAM_256;
          break;

        case SONY_DVBT2_CON_RSVD1:
        case SONY_DVBT2_CON_RSVD2:
        case SONY_DVBT2_CON_RSVD3:
        case SONY_DVBT2_CON_RSVD4:
        case SONY_DVBT2_CONSTELL_UNKNOWN:
          p_sig_info->sig_info.ter.enModType = MT_UNF_MOD_TYPE_DEFAULT;
          break;
      }

      switch (dvbt2_plpinfo.plpCr)
      {
        case SONY_DVBT2_R1_2:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_1_2;
          break;

        case SONY_DVBT2_R3_5:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_3_5;
          break;

        case SONY_DVBT2_R2_3:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_2_3;
          break;

        case SONY_DVBT2_R3_4:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_3_4;
          break;

        case SONY_DVBT2_R4_5:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_4_5;
          break;

        case SONY_DVBT2_R5_6:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_5_6;
          break;

        case SONY_DVBT2_R1_3:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_1_3;
          break;

        case SONY_DVBT2_R2_5:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_2_5;
          break;

        case SONY_DVBT2_PLP_CR_UNKNOWN:
        default:
          p_sig_info->sig_info.ter.enFECRate = MT_UNF_FE_FEC_UNDEF;
          break;
      }

      break;

    case SONY_DTV_SYSTEM_DVBC:
    case SONY_DTV_SYSTEM_DVBC2:
    case SONY_DTV_SYSTEM_J83B:
    default:
      break;
  }
}

static int port_sony_cxd_get_signal_agc(void *handle, mt_u32 *p_agc)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;
  int32_t _strength = 0;
  sony_result_t result = SONY_RESULT_OK;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd_handle->demod)->system);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
  case SONY_DTV_SYSTEM_DVBC2:
  case SONY_DTV_SYSTEM_DVBT:
  case SONY_DTV_SYSTEM_DVBT2:
  case SONY_DTV_SYSTEM_J83B:
    result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x01);

    (&sony_cxd_handle->tuner)->ReadRFLevel(&sony_cxd_handle->tuner, &_strength);

    result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x00);
    break;

  default:
    break;
  }
  /*fix issue27598 The agc format should be the same with other DEMODs*/
  if(p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800)
  {
    *p_agc = _strength*(-100);
  }
  else
  {
    *p_agc = _strength;
  }

  return MT_SUCCESS;
}

static int port_sony_cxd_get_default_timeout(void *handle, mt_u32 *timeout)
{
  *timeout = 2000;

  return MT_SUCCESS;
}

static int port_sony_cxd_set_io(void *handle, MT_BOOL onoff)
{
  return MT_SUCCESS;
}

static int port_sony_cxd_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
  int ret = 0;

  //mt_unf_fe_channel_info_t *p_channel_info = NULL;
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  ret = port_sony_cxd_channel_set(handle, para);

#if 0
  int i = 0;
  mt_unf_fe_status_t status;
  printk("%s[%d] ---- set OK. Lock time = %d\n", __FUNCTION__, __LINE__, para->channel_set_info.lock_time);
  

  if(para->channel_set_info.lock_time == 0)
    para->channel_set_info.lock_time = 2500;

  for (i = 0; i < para->channel_set_info.lock_time; i += 50)
  {
    msleep(50);

    port_sony_cxd2856_get_status(handle, &status);
    if (status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
    {
      printk("%s[%d] ---- Locked!\n", __FUNCTION__, __LINE__);
      return MT_SUCCESS;
    }
  }
#endif

  return MT_SUCCESS;
}

static int port_sony_cxd_standby(void *handle)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  //sony_integ_Shutdown(&(sony_cxd_handle->integ));
  sony_integ_Sleep(&(sony_cxd_handle->integ));

  return MT_SUCCESS;
}

static int port_sony_cxd_wakeup(void *handle)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  //sony_integ_Sleep(&(sony_cxd_handle->integ));
  port_sony_cxd_channel_connect(handle, &(p_priv->param));

  return MT_SUCCESS;
}

static int port_sony_cxd_get_dbg_info(void *handle)
{
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;

  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  sony_demod_lock_result_t stat = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

  sony_result_t result = SONY_RESULT_OK;

  //printk("%s[%d] ---- system[%d]\n", __FUNCTION__, __LINE__, (&sony_cxd_handle->demod)->system);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  //sony_cxd_handle = p_priv->sony_cxd_handle;

  printk(KERN_ERR "\t --------------------|-----------------|---------------\n");

  switch ((&sony_cxd_handle->demod)->system)
  {
  case SONY_DTV_SYSTEM_DVBC:
    sony_demod_dvbc_CheckTSLock(&(sony_cxd_handle->demod), &stat);

    printk(KERN_ERR "\t DVB-C  %s!\n", (stat == SONY_DEMOD_LOCK_RESULT_LOCKED) ? "Locked" : "Unlock");

    break;

  case SONY_DTV_SYSTEM_DVBC2:
    sony_demod_dvbc2_CheckTSLock(&(sony_cxd_handle->demod), &stat);

    printk(KERN_ERR "\t DVB-C2 %s!\n", (stat == SONY_DEMOD_LOCK_RESULT_LOCKED) ? "Locked" : "Unlock");

    break;

  case SONY_DTV_SYSTEM_DVBT:
    sony_demod_dvbt_CheckTSLock(&(sony_cxd_handle->demod), &stat);

    printk(KERN_ERR "\t DVB-T  %s!\n", (stat == SONY_DEMOD_LOCK_RESULT_LOCKED) ? "Locked" : "Unlock");

    {
      int32_t snr;

      result = sony_demod_dvbt_monitor_SNR(&(sony_cxd_handle->demod), &snr);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t SNR                 | SNR             | %d x 10^-3 dB\n", snr);
      }
      else
      {
        printk(KERN_ERR "\t SNR                 | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    {
      uint32_t ber;

      result = sony_demod_dvbt_monitor_PreViterbiBER(&(sony_cxd_handle->demod), &ber);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PreViterbiBER       | Pre-Viterbi BER | %u x 10^-7\n", ber);
      }
      else
      {
        printk(KERN_ERR "\t PreViterbiBER       | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    {
      uint32_t ber;

      result = sony_demod_dvbt_monitor_PreRSBER(&(sony_cxd_handle->demod), &ber);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PreRSBER            | Pre-RS BER      | %u x 10^-7\n", ber);
      }
      else
      {
        printk(KERN_ERR "\t PreRSBER            | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    {
      uint32_t per;

      result = sony_demod_dvbt_monitor_PER(&(sony_cxd_handle->demod), &per);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PER                 | PER             | %u x 10^-7\n", per);
      }
      else
      {
        printk(KERN_ERR "\t PER                 | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    {
      uint32_t pen;

      result = sony_demod_dvbt_monitor_PacketErrorNumber(&(sony_cxd_handle->demod), &pen);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PacketErrorNumber   | Packet Error    | %u\n", pen);
      }
      else
      {
        printk(KERN_ERR "\t PacketErrorNumber   | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    break;

  case SONY_DTV_SYSTEM_DVBT2:
    sony_demod_dvbt2_CheckTSLock(&(sony_cxd_handle->demod), &stat);

    printk(KERN_ERR "\t DVB-T2 %s!\n", (stat == SONY_DEMOD_LOCK_RESULT_LOCKED) ? "Locked" : "Unlock");

    {
      int32_t snr;

      result = sony_demod_dvbt2_monitor_SNR(&(sony_cxd_handle->demod), &snr);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t SNR                 | SNR             | %d x 10^-3 dB\n", snr);
      }
      else
      {
        printk(KERN_ERR "\t SNR                 | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }


    {
      uint32_t ber;

      result = sony_demod_dvbt2_monitor_PreLDPCBER(&(sony_cxd_handle->demod), &ber);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PreLDPCBER          | Pre-LDPC BER    | %u x 10^-7\n", ber);
      }
      else
      {
        printk(KERN_ERR "\t PreLDPCBER          | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    {
      uint32_t ber;

      result = sony_demod_dvbt2_monitor_PreBCHBER(&(sony_cxd_handle->demod), &ber);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PreBCHBER           | Pre-BCH BER     | %u x 10^-7\n", ber);
      }
      else
      {
        printk(KERN_ERR "\t PreBCHBER           | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    {
      uint32_t per;

      result = sony_demod_dvbt2_monitor_PER(&(sony_cxd_handle->demod), &per);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PER                 | PER             | %u x 10^-7\n", per);
      }
      else
      {
        printk(KERN_ERR "\t PER                 | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    {
      uint32_t pen;

      result = sony_demod_dvbt2_monitor_PacketErrorNumber(&(sony_cxd_handle->demod), &pen);

      if (result == SONY_RESULT_OK)
      {
        printk(KERN_ERR "\t PacketErrorNumber   | Packet Error    | %u\n", pen);
      }
      else
      {
        printk(KERN_ERR "\t PacketErrorNumber   | Error           | failed\n");
      }

      printk(KERN_ERR "\t --------------------|-----------------|---------------\n");
    }

    break;

  case SONY_DTV_SYSTEM_J83B:

    printk(KERN_ERR "\t J.83B  %s!\n", (stat == SONY_DEMOD_LOCK_RESULT_LOCKED) ? "Locked" : "Unlock");

    sony_demod_j83b_CheckTSLock(&(sony_cxd_handle->demod), &stat);
    break;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  case SONY_DTV_SYSTEM_DVBS:
  case SONY_DTV_SYSTEM_DVBS2:
    sony_demod_dvbs_s2_CheckTSLock(&(sony_cxd_handle->demod), &stat);

    printk(KERN_ERR "\t DVB-S/S2 %s!\n", (stat == SONY_DEMOD_LOCK_RESULT_LOCKED) ? "Locked" : "Unlock");

    break;
#endif

  default:
    break;
  }

  {
    uint32_t tmp = 0;

    tmp = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xbf200004));
    printk(KERN_ERR "\n\t 0xbf200004 = 0x%08x\n", tmp);

    tmp = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xbf200014));
    printk(KERN_ERR "\t 0xbf200014 = 0x%08x\n", tmp);

    tmp = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xbf200024));
    printk(KERN_ERR "\t 0xbf200024 = 0x%08x\n", tmp);

    tmp = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xbf200034));
    printk(KERN_ERR "\t 0xbf200034 = 0x%08x\n\n", tmp);
  }

  if (p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_R850)
  {
    R850_Dump_Data(R850_TUNER_1);
  }

  if (p_priv->cfg.tun2_type == MT_UNF_TUNER_TYPE_R858C)
  {
    R858_ExtTunerNum_Type ExtTunerNum;
    R858_IntTunerNum_Type IntTunerNum;

    result = sony_tuner_r858_get_tuner_num(&dev_handle[gSonyIndex]->tuner, &ExtTunerNum, &IntTunerNum);

    result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x01);

    R858_Dump_Data(ExtTunerNum, IntTunerNum);

    result = sony_demod_I2cRepeaterEnable(&sony_cxd_handle->demod, 0x00);
  }

  return 0;
}

static int port_sony_cxd_ioctl(void *handle, mt_u32 cmd, ulong param)
{
  /*sony_demod_lock_result_t demod_lock_status = 0;clean warning unused variable*/
  /*mt_unf_fe_diseqc_cmd_t diseqc_cmd = {0};clean warning unused variable*/
  //mt_unf_fe_diseqc_cmd_t *p_diseqc_cmd = NULL;
  //mt_unf_fe_scan_info_t *p_scan_info = NULL;
  /*mt_unf_fe_diseqc_sendmsg_t *p_sendmsg; clean warning unused variable*/
  /*/mt_unf_fe_diseqc_recvmsg_t *p_recvmsg; clean warning unused variable*/
  fe_blindscan_param_t *p_scan_info = NULL;
  //uint8_t tx_buf[8] = {0};
  /*uint8_t voltage = 0;  clean warning unused variable*/
  //uint8_t pin = 0;
  //uint8_t level = 0;
  //uint8_t value = 0;
  //int rc = 0;

  sony_dvbt2_l1pre_t dvbt2_l1pre;
  sony_dvbt_tpsinfo_t dvbt_tpsinfo;


  mt_unf_fe_status_t status;
  sony_result_t result = SONY_RESULT_OK;

  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;
  sony_dvbt2_profile_t sony_tuned_profile;
  //pinmux_configure();

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);


  //printk("%s() %d: cmd %d - param 0x%08x\n", __FUNCTION__, __LINE__, cmd, param);

  switch (cmd)
  {
  case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
    port_sony_cxd_get_status(handle, &status);

    if (param != 0)
      ((mt_unf_fe_channel_info_t *)param)->lock = (status.lock_status == MT_UNF_FE_SIGNAL_LOCKED) ? 1 : 0;
    break;

  case NIM_IOCTRL_DISEQC1X:
    break;

  case NIM_IOCTRL_DISEQC2X:
    break;

  case NIM_IOCTRL_SET_PORLAR:
    break;

  case NIM_IOCTRL_SET_LNB_ONOFF:
    break;

  case NIM_IOCTRL_SET_22K_ONOFF:
    break;

  case NIM_IOCTRL_GET_PORLAR:
    break;

  case NIM_IOCTRL_GET_22K_ONOFF:
    break;

  case NIM_IOCTRL_GET_TN_VERSION:
    break;

  case NIM_IOCTRL_GET_SIGNAL_INFO:
    if (param != 0)
      port_sony_cxd_get_signal_info(handle, (mt_unf_fe_signal_info_t *)param);
    break;

#if 0
  case NIM_IOCTRL_SCAN_CANCEL:
    port_sony_cxd2856_blind_scan_cancel(p_priv);
    break;
#endif

  case NIM_IOCTRL_GET_SCAN_STATUS:
    //mutex_lock(&bs_notify_status_lock);
    if (param != 0)
      *((uint8_t *)param) = sony_cxd_notify_scan_status[gSonyIndex];
    //mutex_unlock(&bs_notify_status_lock);
    break;

  case NIM_IOCTRL_GET_SCAN_RESULT:
    p_scan_info = (fe_blindscan_param_t *)param;
    if (p_priv->scan_info.channel_num_total && param != 0)
    {
      memcpy((fe_blindscan_param_t *)param, &p_priv->scan_info, sizeof(fe_blindscan_param_t));
    }
    break;

#if 0
  case NIM_IOCTRL_DISEQC_SENDMSG:
    p_sendmsg = (mt_unf_fe_diseqc_sendmsg_t *)param;
    port_sony_cxd_diseqc_sendmsg(handle, p_sendmsg);
    break;

  case NIM_IOCTRL_DISEQC_SEND_TONEBURST:
    //p_diseqc_cmd = (mt_unf_fe_diseqc_sendmsg_t*)param;
    port_sony_cxd_diseqc_send_tone_burst(handle, param); // 'param' is the target tone_burst
    break;

  case NIM_IOCTRL_DISEQC_RECVMSG:
    p_recvmsg = (mt_unf_fe_diseqc_recvmsg_t *)param;
    port_sony_cxd_diseqc_recvmsg(handle, p_recvmsg);
    break;
    //#endif
#endif

  case NIM_IOCTRL_T2_GET_PLP_NUM:
    if (param)
      *((uint32_t *)param) = p_priv->param.connect_param.ter.data_plp_number;

    //printk("%s[%d] ---- NIM_IOCTRL_T2_GET_PLP_NUM = %d, %d\n", __FUNCTION__, __LINE__, p_priv->param.connect_param.ter.data_plp_number, *((uint32_t *)param));
    break;

  case NIM_IOCTRL_T2_SET_PLP_NO:
  	if(p_priv->param.connect_param.ter.channel_mode == MT_UNF_FE_TER_MODE_LITE)
		sony_tuned_profile = SONY_DVBT2_PROFILE_LITE;
	else
		sony_tuned_profile = SONY_DVBT2_PROFILE_BASE;
    result = sony_integ_dvbt2_Scan_SwitchDataPLP(&(p_priv->sony_cxd_handle)->integ, 1, (uint8_t)param, sony_tuned_profile);
    printk("%s[%d] ---- DVB-T2 switch to PLP[%02x] %s!\n", __FUNCTION__, __LINE__, (uint8_t)param, (result == SONY_RESULT_OK) ? "OK" : "failed");
    break;

  case NIM_IOCTRL_T_T2_GET_CELL_ID:
    printk("%s[%d] ---- NIM_IOCTRL_T_T2_GET_CELL_ID\n", __FUNCTION__, __LINE__);

    switch ((&sony_cxd_handle->demod)->system)
    {
      case SONY_DTV_SYSTEM_DVBT:
        printk("%s[%d] ---- NIM_IOCTRL_T_T2_GET_CELL_ID DVB-T\n", __FUNCTION__, __LINE__);
        result = sony_demod_dvbt_monitor_TPSInfo(&(sony_cxd_handle->demod), 
                                        &dvbt_tpsinfo);


        printk("%s[%d] ---- sony_demod_dvbt_monitor_TPSInfo(), result = %d\n", __FUNCTION__, __LINE__, result);

        if (param)
          *((uint32_t *)param) = dvbt_tpsinfo.cellID;

        printk("%s[%d] ---- DVB-T CellId = 0x%04d\n", __FUNCTION__, __LINE__, dvbt_tpsinfo.cellID);
        break;

      case SONY_DTV_SYSTEM_DVBT2:
        result = sony_demod_dvbt2_monitor_L1Pre(&(sony_cxd_handle->demod), &dvbt2_l1pre);

        if (param)
          *((uint32_t *)param) = dvbt2_l1pre.cellId;

        break;

      default:/*Clean warning*/
        break;
    }

    break;

  case NIM_IOCTRL_T_GET_HIERARCHY_NUM:
    break;

  case NIM_IOCTRL_T_SET_HIERARCHY_NO:
    break;

  case NIM_IOCTRL_T2_GET_PLP_NO:
    if (param != 0)
    {
      sony_dvbt2_plp_t dvbt2_plpinfo;

      sony_demod_dvbt2_monitor_ActivePLP(&(sony_cxd_handle->demod), 
                           SONY_DVBT2_PLP_DATA, 
                           &dvbt2_plpinfo);

      *((MT_U8 *)param) = dvbt2_plpinfo.id;
    }
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
    param %= MAX_FE_NUM; // to avoid overflow

    gFeIndex = param;

    //printk("%s[%d] -- set current FE index[%d], type[%d]\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex]);
    break;

  case NIM_IOCTRL_GET_TN_DBG_INFO:
    port_sony_cxd_get_dbg_info(p_priv);
    break;

  case NIM_IOCTRL_SUSPEND:
    sony_cxd_suspend(p_priv);
    break;

  case NIM_IOCTRL_RESUME:
    sony_cxd_resume(p_priv);
    break;
  case NIM_IOCTRL_GET_ACCURATE_SNR:
    port_sony_cxd_get_accurate_snr(p_priv,(MT_S32 *)param);
    break;
  default:
    break;
  }

  return MT_SUCCESS;
}

#if 0
static int port_sony_cxd_blind_scan_cancel(void *handle)
{
#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  mt_fe_sony_cxd_priv_handle p_priv = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = p_priv->sony_cxd_handle;

  printk("port_sony_cxd_blind_scan_cancel\n");
  mt_fe_dmd_blindscan_abort_ss2_sony_cxd(sony_cxd_handle, 1);
#endif

  return MT_SUCCESS;
}
#endif

static const struct nla_policy sony_cxd_policy[SONY_CXD_NL_FAMILY_ATTR_AMOUNT] = {
    [SONY_CXD_NL_ATTR_DATA0] = {.type = NLA_BINARY, .len = sizeof(struct sony_cxd_netlink_data)},
};

__attribute__((unused))static int sony_cxd_nl_recvmsg0(struct sk_buff *skb, struct genl_info *info)
{
  struct sony_cxd_netlink_data *sony_cxd_nl_data;
  struct sony_cxd_netlink_pid_list *sony_cxd_nl_pid;
  int err = 0;

  //printk("%s\n", __FUNCTION__);

  if (info->attrs[SONY_CXD_NL_ATTR_DATA0])
  {
    sony_cxd_nl_data = nla_data(info->attrs[SONY_CXD_NL_ATTR_DATA0]);
    sony_cxd_nl_pid = kzalloc(sizeof(struct sony_cxd_netlink_pid_list), GFP_KERNEL);
    if (sony_cxd_nl_pid == NULL)
      return -1; // No memory, SONY_RESULT_ERROR_OVERFLOW
    sony_cxd_nl_pid->net = sock_net(skb->sk);
    /* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
    sony_cxd_nl_pid->sony_cxd_nl_data = *sony_cxd_nl_data;
    //printk("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
    //printk("netlink : pid = %d\n", sony_cxd_nl_data->pid);
    //printk("netlink : seq = 0x%x\n", sony_cxd_nl_data->seq);
    //printk("netlink : cmd = %d\n", sony_cxd_nl_data->cmd);
    //printk("netlink : data01 = %d\n", sony_cxd_nl_data->data01);
    //printk("netlink : name : %s, freq = %u, sym = %u\n", sony_cxd_nl_data->name, sony_cxd_nl_data->freq_khz, sony_cxd_nl_data->symbol_rate);

    if (mutex_lock_interruptible(&sony_cxd_nl_mutex))
    {
      kfree(sony_cxd_nl_pid);
      return -ETIME;
    }

    list_add_tail(&(sony_cxd_nl_pid->list), &sony_cxd_nl_pid_list);
    mutex_unlock(&sony_cxd_nl_mutex);
  }

  return err;
}
#if 0
static struct genl_ops sony_cxd_nl_ops[SONY_CXD_NL_OPS_AMOUNT] = {
    [SONY_CXD_NL_OPS_CMD0] = {
        .cmd = SONY_CXD_NL_OPS_CMD0,
        .doit = sony_cxd_nl_recvmsg0,
        .policy = sony_cxd_policy,
    },
};


static int port_sony_cxd_blind_scan_start(void *pArg)
{
#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
  S32 ret = 0;
  U32 i = 0;
  sony_example_driver_handle sony_cxd2856_handle = NULL;
  fe_blindscan_param_t *p_scan_info = (fe_blindscan_param_t *)pArg;

  sony_cxd2856_handle = (sony_example_driver_instance_t *)g_sony_cxd2856_priv[gSonyIndex]->sony_cxd2856_handle;
  printk("ker sony_cxd2856: start-stop - start freq: %d, end_freq: %d\n",
         p_scan_info->start_freq, p_scan_info->stop_freq);
  printk("ker sony_cxd2856: use_uc[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
         p_scan_info->uc_param.use_uc,
         p_scan_info->uc_param.bank,
         p_scan_info->uc_param.user_band,
         p_scan_info->uc_param.ub_freq_mhz);

#endif
  return MT_FAILURE;
}

static int port_sony_cxd2856_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info)
{
  //mutex_lock(&bs_notify_status_lock);
  sony_cxd2856_notify_scan_status[gSonyIndex] = 0;
  //mutex_unlock(&bs_notify_status_lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 42)
  kernel_thread(port_sony_cxd2856_blind_scan_start, p_scan_info, CLONE_KERNEL);
#else
  kthread_run(port_sony_cxd2856_blind_scan_start, (void *)p_scan_info, "port_sony_cxd2856_blind_scan");
#endif

  return MT_SUCCESS;
}


void port_sony_cxd2856_notify_to_up_layer(MT_FE_MSG msg, void *p_param)
{
  //U16 i = 0;
  mt_unf_fe_blind_scan_channel_info_t *p_bs_channel_info = NULL;

  if (p_param != NULL)
    p_bs_channel_info = (mt_unf_fe_blind_scan_channel_info_t *)p_param;

  if (msg == MtFeMsg_BSFinish || msg == MtFeMsg_BSAbort)
  {
    sony_cxd2856_notify_scan_status[gSonyIndex] = 1;
    printk("sony_cxd2856_notify_scan_status[%d] = %d\n", gSonyIndex, sony_cxd2856_notify_scan_status[gSonyIndex]);
  }
}

static void port_sony_cxd2856_register_to_drv_notify(MT_FE_MSG msg, void *p_param)
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

  int i;

  //sony_example_driver_instance_t * sony_cxd2856_handle = g_sony_cxd2856_priv[gSonyIndex]->sony_cxd2856_handle;

  /*
  if (port_sony_cxd2856_notify_to_up_layer == NULL)
  {
    return;
  }
  */

  //printk("sony_cxd2856_notify_function: msg ");

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
    sony_cxd2856_nl_sendmsg(&channel_info);

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
    mt_fe_dmd_sony_cxd2856_get_strength(sony_cxd2856_handle, &s_strength);
    bs_channel_info.perf.agc =  s_strength;
    mt_fe_dmd_sony_cxd2856_get_sat_quality(sony_cxd2856_handle, &snr);
    bs_channel_info.perf.snr = snr;
    mt_fe_dmd_sony_cxd2856_get_per(sony_cxd2856_handle, &total_packages, &err_packages);
    bs_channel_info.perf.ber = (double)err_packages/(double)total_packages;
#endif
    port_sony_cxd2856_notify_to_up_layer(MtFeMsg_BSTpUnlock, &bs_channel_info);

    channel_info.frequency = p_tp_info[0].freq_KHz;
    channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    channel_info.port_type = MT_UNF_PORT_TYPE_DVBS_BUTT;
    sony_cxd2856_nl_sendmsg(&channel_info);

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
    bs_channel_info.nim_type = nim_sony_cxd2856_deparse_dvb_type(p_tp_info[0].dtv_type);
    bs_channel_info.param.dvbs.fec_inner = nim_sony_cxd2856_deparse_code_rate(p_tp_info[0].code_rate);
    mt_fe_dmd_sony_cxd2856_get_strength(sony_cxd2856_handle, &s_strength);
    bs_channel_info.param.dvbs.perf.agc =  s_strength;
    mt_fe_dmd_sony_cxd2856_get_sat_quality(sony_cxd2856_handle, &snr);
    bs_channel_info.param.dvbs.perf.snr = snr;
    mt_fe_dmd_sony_cxd2856_get_per(sony_cxd2856_handle, &total_packages, &err_packages);
    bs_channel_info.param.dvbs.perf.ber = (double)err_packages/(double)total_packages;
    */
    port_sony_cxd2856_notify_to_up_layer(MtFeMsg_BSTpLocked, &bs_channel_info);
    printk("BSTpLocked: freq[%7d] sym[%5d] %s\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs, (p_tp_info[0].dtv_type == MtFeType_DVBS2) ? "DVB-S2" : "DVB-S");

#ifdef CFG_SYNC_BLINDSCAN
    init_waitqueue_head(&bs_cb_wq);

    event_status = 0;
#endif

    //kobject_uevent();
    channel_info.frequency = p_tp_info[0].freq_KHz;
    channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
    channel_info.port_type = port_sony_cxd2856_deparse_dvb_type(p_tp_info[0].dtv_type);
    sony_cxd2856_nl_sendmsg(&channel_info);

#ifdef CFG_SYNC_BLINDSCAN
    //printk("%s() %d: prepare to wait event, wq = 0x%08x, evt_status = %d\n", __FUNCTION__, __LINE__, &wq, event_status);

    error = wait_event_interruptible_timeout(bs_cb_wq, (event_status == 1), BLINDSCAN_WAIT_CB_TIMEOUT * HZ);
    if (error == 0)
    {
      //MT_ERR_FRONTEND("%s: blindscan wait callback timeout!\n", __FUNCTION__);
      printk("%s: blindscan wait callback timeout!\n", __FUNCTION__);
    }
    else if (error == -ERESTARTSYS)
    {
      //MT_ERR_FRONTEND("%s: blindscan interrupted by a signal!\n", __FUNCTION__);
      printk("%s: blindscan interrupted by a signal!\n", __FUNCTION__);
    }
    else
    {
      //printk("%s[%d]: wait event returned error = %d, evt_status = %d\n", __FUNCTION__, __LINE__, error, event_status);
    }
#endif

    break;

  case MtFeMsg_BSAbort:
    printk("BSAbort \n");
    sony_cxd2856_nl_list_del();
    port_sony_cxd2856_notify_to_up_layer(MtFeMsg_BSAbort, NULL);
    break;

  case MtFeMsg_BSFinish:
    printk("BS Finished\n");

    sony_cxd2856_nl_list_del();
    port_sony_cxd2856_notify_to_up_layer(MtFeMsg_BSFinish, NULL);
    break;

  default:
    break;
  }
}
#endif

#ifdef CONFIG_NET

sony_MxL608_t MxL608Tuner;
sony_r850_t R850Tuner;
sony_r858_t R858Tuner;
sony_r840_t R840Tuner;
sony_tc6800_t TC6800Tuner;


uint8_t sony_cxd_get_sony_index(mt_unf_fe_attr_t *attr)
{
  uint8_t index = 0, tmp = 0;

  for (index = 0; index < MAX_SONY_CXD_DEVICES; index++)
  {
    if (createParam[index].i2cAddressSLVT == attr->demod_addr)
    {
      break;
    }
  }

  if (index == MAX_SONY_CXD_DEVICES)
  {
    for (tmp = 0; tmp < MAX_SONY_CXD_DEVICES; tmp++)
    {
      if (bInitialized_SONY_CXD[tmp] == 0)
      {
        index = tmp;
        break;
      }
    }
  }

  return index;
}

void sony_cxd_get_sony_index_by_addr(uint8_t demod_addr)
{
  uint8_t index = 0;

  for (index = 0; index < MAX_SONY_CXD_DEVICES; index++)
  {
    if (createParam[index].i2cAddressSLVT == demod_addr)
    {
      gSonyIndex = index;

      break;
    }
  }

  return;
}

#if 0	// 20211228
void sony_cxd2856_MxL608_GPIO_init(int gpio_num)
{
	mt_u32 tmp = 0;

	//gpio14 ---- ant_on/off   out_1
	//pinmux set register 0xBF13C038[3:0] = 1
	//GPIO set register
	//GPIO_Mask: 0xBF0A000C[14] = 1, Write_EN: 0xBF0A0004[14] = 0, Set data 0xBF0A0000[14] = 0 or 1

	//gpio15 ---- demo_reset   out_1
	//pinmux set register 0xBF13C03C[3:0] = 1
	//GPIO set register
	//GPIO_Mask: 0xBF0A000C[15] = 1, Write_EN: 0xBF0A0004[15] = 0, Set data 0xBF0A0000[15] = 0 or 1

	//gpio16 ---- ant_short    in, 0: short_cut 1: open
	//pinmux set register 0xBF13C040[3:0] = 1
	//GPIO set register
	//GPIO_Mask: 0xBF0A000C[16] = 1, Read_EN: 0xBF0A0004[16] = 1, Get data 0xBF0A0008[16]     

	if (gpio_num == GPIO_14)
	{
		tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xBF13C038));
		tmp &= ~0x07;
		tmp |= 0x01;
		HAL_PUT_U32(SYMPHONY_IO_VA(0xBF13C038), tmp);
	}
	else if (gpio_num == GPIO_15)
	{
		tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xBF13C03C));
		tmp &= ~0x07;
		tmp |= 0x01;
		HAL_PUT_U32(SYMPHONY_IO_VA(0xBF13C03C), tmp);
	}
	else if (gpio_num == GPIO_16)
	{
		tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xBF13C040));
		tmp &= ~0x07;
		tmp |= 0x01;
		HAL_PUT_U32(SYMPHONY_IO_VA(0xBF13C040), tmp);
	}

	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xBF0A000C));
	if (gpio_num == GPIO_14)
	{
		tmp |= (1 << 14);
	}
	else if (gpio_num == GPIO_15)
	{
		tmp |= (1 << 15);
	}
	else if (gpio_num == GPIO_16)
	{
		tmp |= (1 << 16);
	}
	HAL_PUT_U32(SYMPHONY_IO_VA(0xBF0A000C), tmp);

	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xBF0A0004));
	if (gpio_num == GPIO_14)
	{
		tmp &= ~(1 << 14);
	}
	else if (gpio_num == GPIO_15)
	{
		tmp &= ~(1 << 15);
	}
	else if (gpio_num == GPIO_16)
	{
		tmp |= (1 << 16);
	}
	HAL_PUT_U32(SYMPHONY_IO_VA(0xBF0A0004), tmp);


	return;
}

void sony_cxd2856_MxL608_GPIO_power_on(int gpio_num)
{
	mt_u32 tmp = 0;

	if (gpio_num == GPIO_15)
	{
		tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xBF0A0000));
		tmp |= (1 << 15);
		HAL_PUT_U32(SYMPHONY_IO_VA(0xBF0A0000), tmp);

		printk("%s[%d] ---- GPIO[%d], 0xbf0a0000 = 0x%08x\n", __FUNCTION__, __LINE__, gpio_num, tmp);

		sony_sleep_delay_ms(80);
	}
}

void sony_cxd2856_MxL608_GPIO_power_off(int gpio_num)
{
	mt_u32 tmp = 0;

	if (gpio_num == GPIO_15)
	{
		tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xBF0A0000));
		tmp &= ~(1 << 15);
		HAL_PUT_U32(SYMPHONY_IO_VA(0xBF0A0000), tmp);

		printk("%s[%d] ---- GPIO[%d], 0xbf0a0000 = 0x%08x\n", __FUNCTION__, __LINE__, gpio_num, tmp);

		sony_sleep_delay_ms(80);
	}
}
#endif

int sony_cxd_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
  //uint8_t reg_val = 0;
  /*int ret = 0;clean warning unused variable*/
  mt_u32 tmp = 0;
  sony_result_t result = SONY_RESULT_OK;

  int bR858IntNum = 0;

  /*uint8_t enableSatTuner = 0;*//*Clean warning*/
  uint8_t enableLNBC = 0;
  uint8_t tuner_addr = 0xC0;

  uint8_t cur_sony_index = sony_cxd_get_sony_index(attr);

  if (cur_sony_index == MAX_SONY_CXD_DEVICES)
  {
    printk("%s[%d] ---- Too many (> %d) Sony CXD devices.\n", __FUNCTION__, __LINE__, MAX_SONY_CXD_DEVICES);

    return MT_FAILURE;
  }

  //printk("%s[%d] ---- cur_sony_index = %d\n", __FUNCTION__, __LINE__, cur_sony_index);

  gSonyIndex = cur_sony_index;

  if (info->is_attach)
  {
    return MT_SUCCESS;
  }

  if (bInitialized_SONY_CXD[gSonyIndex])
  {
    return MT_SUCCESS;
  }

  cxd_i2c.Read = Sony_I2C_Read;
  cxd_i2c.Write = Sony_I2C_Write;
  cxd_i2c.ReadRegister = Sony_I2C_ReadRegister;
  cxd_i2c.WriteRegister = Sony_I2C_WriteRegister;
  cxd_i2c.WriteOneRegister = Sony_I2C_WriteOneRegister;

  info->ops.connect = port_sony_cxd_channel_connect;
  info->ops.get_status = port_sony_cxd_get_status;
  info->ops.get_ber = port_sony_cxd_get_ber;
  info->ops.get_snr = port_sony_cxd_get_snr;
  info->ops.get_signal_strength = port_sony_cxd_get_signal_strength;
  info->ops.get_signal_quality = port_sony_cxd_get_signal_quality;
  info->ops.get_signal_agc = port_sony_cxd_get_signal_agc;

  info->ops.standby = port_sony_cxd_standby;
  info->ops.wakeup = port_sony_cxd_wakeup;
  info->ops.get_default_timeout = port_sony_cxd_get_default_timeout;
  info->ops.set_io = port_sony_cxd_set_io;
  info->ops.port_ioctl = port_sony_cxd_ioctl;
  //info->ops.blind_scan = port_sony_cxd_blind_scan;

#if 0 //defined(CONFIG_MT_CHIP_SYMPHONY4) // Sym4 + SONYCXD2856 + MxL608 pinmux
	// BF5D0094[0] = 1  //IQ PAD设置为digital input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf5d0094));
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf5d0094), tmp);

	// BF13C190[2:0] = 2 // TS1_CLK
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c190));
	tmp &= ~0x07;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c190), tmp);

	// BF13C194[2:0] = 2 // TS1_SYNC
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c194));
	tmp &= ~0x07;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c194), tmp);

	// BF13C198[2:0] = 2 // TS1_VALID
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c198));
	tmp &= ~0x07;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c198), tmp);

	// BF13C19C[2:0] = 2 // TS1_DATA
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c19c));
	tmp &= ~0x07;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c19c), tmp);

	// BF138008[4] = 0 //TS1 input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf138008));
	tmp &= ~0x10;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf138008), tmp);

	// BF50B000[0] = 1 //打开时钟
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b000));
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b000), tmp);

	// BF50B004[4:3] = 00 //ts1 clk 正沿
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b004));
	tmp &= ~0x18;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b004), tmp);

	//BF50B00C[4] = 1  //复位释放
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b00c));
	tmp |= 0x10;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b00c), tmp);


	//BF200010[31:0]=0xC7E101FF
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200010), 0);
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200010), 0xc7e101ff);
#elif 0 //defined(CONFIG_MT_CHIP_SYMPHONY4) // Sym4 + SONYCXD2856 + R850 pinmux TS2 3线 SYNC 
	// BF13C048[3:0]=5//TS2_CLK
	// BF5D0094[3]=1  //IF PAD设置为digital input
	// BF13C1A0[2:0]=3 //TS2_SYNC
	// BF13C1A4[2:0]=2 //TS2_DATA
	// BF50B004[6:5]=0/ ts2 clk 正沿
	// BF138008[0]=0 //TS2 input
	// BF200020[31:0]=0xC7F101FF//TC2 CTR
	// 读取BF200024 TS状态
#elif 0 //defined(CONFIG_MT_CHIP_SYMPHONY4) // Sym4 + SONYCXD2856 + R850 pinmux TS2 2线
	// BF13C048[3:0]=5//TS2_CLK
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c048));
	tmp &= 0xFFFFFFF0;
	tmp |= 0x05;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c048), tmp);

	// BF5D0094[3]=1  //IF PAD设置为digital input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf5d0094));
	tmp |= 0x08;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf5d0094), tmp);

	// BF13C1A0[2:0]=1 //TS2_SYNC pin配置为GPIO
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a0));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a0), tmp);

	// BF13C1A4[2:0]=2 //TS2_DATA
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a4));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a4), tmp);

	// BF50B000[0]=1 //open clk
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b000));
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b000), tmp);

	// BF50B004[6:5]=00 //clk正沿
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b004));
	tmp &= 0xFFFFFF9F;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b004), tmp);

	// BF50B00C[5]=1  //rest off
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b00c));
	tmp |= 0x20;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b00c), tmp);

	// BF138008[0]=0 //TS2 input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf138008));
	tmp &= ~0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf138008), tmp);

	// BF200020[31:0]=0xC7D801FF//TC2 CTR
	// 读取BF200024 TS状态

	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0);
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0xc7d801ff);
#elif 0 //defined(CONFIG_MT_CHIP_SYMPHONY4) // Sym4 + SONYCXD2856 + TC6800 pinmux TS2 3线 VALID
	// BF13C048[3:0]=5//TS2_CLK
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c048));
	tmp &= 0xFFFFFFF0;
	tmp |= 0x05;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c048), tmp);

	// BF5D0094[3]=1  //IF PAD设置为digital input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf5d0094));
	tmp |= 0x08;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf5d0094), tmp);

	// BF13C1A0[2:0]=2 //TS2_VALID
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a0));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a0), tmp);

	// BF13C1A4[2:0]=2 //TS2_DATA
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a4));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a4), tmp);

	// BF50B000[0]=1 //open clk
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b000));
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b000), tmp);

	// BF50B004[6:5]=00 //clk正沿
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b004));
	tmp &= 0xFFFFFF9F;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b004), tmp);

	// BF50B00C[5]=1  //rest off
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b00c));
	tmp |= 0x20;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b00c), tmp);

	// BF138008[0]=0 //TS2 input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf138008));
	tmp &= ~0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf138008), tmp);

	// BF200020[31:0]=0xC7EB11FF//TC2 CTR
	// 读取BF200024 TS状态

	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0);
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0xc7eb11ff);
#elif 0 //defined(CONFIG_MT_CHIP_SYMPHONY4) // Sym4 + SONYCXD2856 + R850/R836 pinmux TS2 3线 VALID, tested OK
	// BF13C048[3:0]=5//TS2_CLK
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c048));
	tmp &= 0xFFFFFFF0;
	tmp |= 0x05;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c048), tmp);

	// BF5D0094[3]=1  //IF PAD设置为digital input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf5d0094));
	tmp |= 0x08;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf5d0094), tmp);

	// BF13C1A0[2:0]=1 //TS2_SYNC pin配置为GPIO
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a0));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a0), tmp);

	// BF13C1A4[2:0]=2 //TS2_DATA
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a4));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a4), tmp);

	// BF50B000[0]=1 //open clk
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b000));
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b000), tmp);

	// BF50B004[6:5]=00 //clk正沿
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b004));
	tmp &= 0xFFFFFF9F;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b004), tmp);

	// BF50B00C[5]=1  //rest off
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b00c));
	tmp |= 0x20;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b00c), tmp);

	// BF138008[0]=0 //TS2 input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf138008));
	tmp &= ~0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf138008), tmp);

	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0);
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0xc7eb11ff);
#elif 0 //defined(CONFIG_MT_CHIP_SYMPHONY4) // Sym4 + SONYCXD2856 + R858C pinmux TS1 3线 VALID + TS2 3线VALID

	// BF13C048[3:0] = 5 // TS2_CLK
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c048));
	tmp &= 0xFFFFFFF0;
	tmp |= 0x05;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c048), tmp);

	// BF5D0094[0]=1	//IQ PAD设置为digital input
	// BF5D0094[3]=1  //IF PAD设置为digital input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf5d0094));
	tmp |= 0x09;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf5d0094), tmp);

	// BF13C190[2:0]=2 // TS1_CLK pinmux
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c190));
	tmp &= 0xFFFFFFF0;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c190), tmp);

	// BF13C198[2:0]=2 // TS1_VALID pinmux
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c198));
	tmp &= 0xFFFFFFF0;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c198), tmp);

	// BF13C19C[2:0]=2 // TS1_DATA pinmux
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c19c));
	tmp &= 0xFFFFFFF0;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c19c), tmp);


	// BF13C1A0[2:0]=1 //TS2_SYNC pin配置为GPIO
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a0));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a0), tmp);

	// BF13C1A4[2:0]=2 //TS2_DATA
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf13c1a4));
	tmp &= 0xFFFFFFF8;
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf13c1a4), tmp);

	// BF50B000[0]=1 //open clk
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b000));
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b000), tmp);

	// BF50B004[4:3] = 00 //ts1 clk 正沿
	// BF50B004[6:5] = 00 //ts2 clk 正沿
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b004));
	tmp &= 0xFFFFFF97;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b004), tmp);

	// BF50B00C[4]=1  //复位释放
	// BF50B00C[5]=1  //rest off
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf50b00c));
	//tmp |= 0x20;
	tmp |= 0x30;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf50b00c), tmp);

	// BF138008[4]=0 //TS1 input
	// BF138008[0]=0 //TS2 input
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf138008));
	tmp &= ~0x11;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf138008), tmp);

	// BF200010[31:0]=0xC7EB11FF//TC1 CTR
	// 读取BF200014 TS状态	

	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200010), 0);
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200010), 0xc7eb11ff);

	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0);
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf200020), 0xc7eb11ff);
#elif 1 //defined(CONFIG_MT_CHIP_SYMPHONY6) // Sym6 + SONYCXD + TC6800 pinmux TS3 4线
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf13c04c), 0x0003);
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf13c050), 0x0203);
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf13c054), 0x0003);
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf13c058), 0x0003);

	tmp = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf138008));
	tmp |= 0x10000;
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf138008), tmp);

	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200030), 0);
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200030), 0xc7fb01ff);

    /*fix issue 26710 no 5v output*/
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf13c048), 0x0001);/*PINMUX set to GPIO18*/

    /*ant 5v power on*/
    drv_gpio_io_enable(GPIO_18, GPIO_MASK_ENABLE);
    drv_gpio_set_dir(GPIO_18, GPIO_DIR_OUTPUT);
    drv_gpio_set_value(GPIO_18,GPIO_VALUE_HIGH_LEVEL);
    sony_sleep_delay_ms(5);/*delay 5ms*/
    drv_gpio_io_enable(GPIO_18, GPIO_MASK_DISABLE);
#endif

#if 0	// 20211228
	// 0xbf156000 bit[1] = 1
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf156000));
	tmp |= 0x02;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf156000), tmp);

	// 0xbf15b408 bit[2:0] = 1
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf15b408));
	tmp &= ~0x07;
	tmp |= 0x01;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf15b408), tmp);

	// 0xbf15500c bit[2] = 1
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf15500c));
	tmp |= 0x04;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf15500c), tmp);

	// 0xbf155004 bit[2] = 0
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf155004));
	tmp &= ~0x04;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf155004), tmp);

	// 0xbf155000 bit[2] = 1
	tmp = HAL_GET_U32(SYMPHONY_IO_VA(0xbf155000));
	tmp &= ~0x04;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf155000), tmp);

	sony_sleep_delay_ms(10);

	tmp |= 0x04;
	HAL_PUT_U32(SYMPHONY_IO_VA(0xbf155000), tmp);

	sony_cxd2856_MxL608_GPIO_init(GPIO_14);
	sony_cxd2856_MxL608_GPIO_init(GPIO_15);
	sony_cxd2856_MxL608_GPIO_init(GPIO_16);

	sony_cxd2856_MxL608_GPIO_power_off(GPIO_15);
	sony_cxd2856_MxL608_GPIO_power_on(GPIO_15);

#endif

  gFeList[gFeIndex] = attr->sig_type;

  //printk("%s[%d] ---- FE[%d] type = %d, tuner_type[%d], tuner_addr[0x%02x], tun2_type[%d], tun2_addr[0x%02x]\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex], 
  //        attr->tuner_type, attr->tuner_addr, attr->fe_config.tun2_type, attr->fe_config.tun2_addr);

  if (p_priv[gSonyIndex] == NULL)
  {
    //p_priv[gSonyIndex] = kzalloc(sizeof(mt_fe_sony_cxd_priv_t), GFP_KERNEL);
    p_priv[gSonyIndex] = vzalloc(sizeof(mt_fe_sony_cxd_priv_t));
    if (p_priv[gSonyIndex] == NULL)
      return -ENOMEM;
  }

  if (dev_handle[gSonyIndex] == NULL)
  {
    //dev_handle[gSonyIndex] = kzalloc(sizeof(sony_example_driver_instance_t), GFP_KERNEL);
    dev_handle[gSonyIndex] = vzalloc(sizeof(sony_example_driver_instance_t));
    if (dev_handle[gSonyIndex] == NULL)
    {
      //kfree((void *)p_priv[gSonyIndex]);
      vfree((void *)p_priv[gSonyIndex]);
      g_sony_cxd_priv[gSonyIndex] = NULL;
      return -ENOMEM;
    }
  }

  p_priv[gSonyIndex]->sony_cxd_handle = dev_handle[gSonyIndex];
  info->handle = (void *)p_priv[gSonyIndex];
  g_sony_cxd_priv[gSonyIndex] = p_priv[gSonyIndex];

  g_sony_cxd_handle[gSonyIndex] = dev_handle[gSonyIndex];

  g_i2c_sony_cxd = attr->demod_i2c_id;

  if (g_i2c_sony_cxd == 1)
  {
    tmp = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b420));
    tmp &= 0xFFFFFFF0;
    tmp |= 0x03;
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b420), tmp);

    tmp = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b424));
    tmp &= 0xFFFFFFF0;
    tmp |= 0x03;
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b424), tmp);
  }
  else
  {
    tmp = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b420));
    tmp &= 0xFFFFFFF0;
    tmp |= 0x02;
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b420), tmp);
  
    tmp = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b424));
    tmp &= 0xFFFFFFF0;
    tmp |= 0x02;
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf15b424), tmp);
  }

  memcpy(&(p_priv[gSonyIndex]->cfg), &(attr->fe_config), sizeof(mt_unf_fe_config_para_t));

  if (NULL == sony_cxd_pg_channel_info[gSonyIndex])
  {
    //sony_cxd_pg_channel_info[gSonyIndex] = kzalloc(sizeof(mt_unf_fe_channel_info_t) * MAX_BS_TP_NUM_PER_SAT, GFP_KERNEL);
    sony_cxd_pg_channel_info[gSonyIndex] = vzalloc(sizeof(mt_unf_fe_channel_info_t) * MAX_BS_TP_NUM_PER_SAT);
    if (sony_cxd_pg_channel_info[gSonyIndex] == NULL)
    {
      //kfree(dev_handle[gSonyIndex]);
      vfree(dev_handle[gSonyIndex]);
      //kfree((void *)p_priv[gSonyIndex]);
      vfree((void *)p_priv[gSonyIndex]);
      g_sony_cxd_priv[gSonyIndex] = NULL;
      return -ENOMEM;
    }
  }

  p_priv[gSonyIndex]->scan_info.p_channel_info = sony_cxd_pg_channel_info[gSonyIndex];

  if (bInitialized_SONY_CXD[gSonyIndex] == 0)
  {
#if 0 // tuner @ 20210106
#if defined(SONY_EXAMPLE_TUNER_ASCOT3)
    /* Use Sony ASCOT3/3I (Terrestrial tuner) */
    /* 16MHz Xtal, I2C slave address is 0xC0 */
    result = sony_tuner_ascot3_Create(&dev_handle[gSonyIndex]->tunerTerrCable, SONY_ASCOT3_XTAL_16000KHz,
        0xC0, &dev_handle[gSonyIndex]->i2c, 0, &dev_handle[gSonyIndex]->ascot3);
    if (result != SONY_RESULT_OK) {
        return result;
    }

    enableSatTuner = 0;
#else
    /* Use Sony HELENE (Terrestrial/Satellite combined tuner) */
    /* 16MHz Xtal, I2C slave address is 0xC0 */
    result = sony_tuner_helene_Create (&dev_handle[gSonyIndex]->tunerTerrCable, &dev_handle[gSonyIndex]->tunerSat,
        SONY_HELENE_XTAL_16000KHz, 0xC0, &dev_handle[gSonyIndex]->i2c, 0, &dev_handle[gSonyIndex]->helene);
    if (result != SONY_RESULT_OK) {
        return result;
    }

    enableSatTuner = 1;
#endif
#endif

    if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_MXL603) || 
        (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_MXL608) || 
        (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_MXL_608)) 
    {
      tuner_addr = attr->fe_config.tun2_addr;
      if (tuner_addr == 0)
        tuner_addr = 0xC0;

      printk("%s[%d] -- Select tuner MxL608, addr[%02x][%02x]\n", __FUNCTION__, __LINE__, tuner_addr, attr->fe_config.tun2_addr);

      result = sony_tuner_MxL608_Create(&dev_handle[gSonyIndex]->tuner, MXL608_XTAL_16MHz, tuner_addr, &cxd_i2c, 0, &MxL608Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create MxL608 failed!\n");
        return MT_FAILURE;
      }
    }
    else if ((attr->tuner_type == MT_UNF_TUNER_TYPE_MXL603) || 
             (attr->tuner_type == MT_UNF_TUNER_TYPE_MXL608) || 
             (attr->tuner_type == MT_UNF_TUNER_TYPE_MXL_608))
    {
      tuner_addr = attr->tuner_addr;
      if (tuner_addr == 0)
        tuner_addr = 0xC0;

      printk("%s[%d] -- Select tuner MxL608, addr[%02x][%02x]\n", __FUNCTION__, __LINE__, tuner_addr, attr->tuner_addr);

      result = sony_tuner_MxL608_Create(&dev_handle[gSonyIndex]->tuner, MXL608_XTAL_16MHz, tuner_addr, &cxd_i2c, 0, &MxL608Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create MxL608 failed!\n");
        return MT_FAILURE;
      }
    }
    else if (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_R850)
    {
      tuner_addr = attr->fe_config.tun2_addr;
      if (tuner_addr == 0)
        tuner_addr = 0xF8;

      printk("%s[%d] -- Select tuner R850, addr[%02x][%02x]\n", __FUNCTION__, __LINE__, tuner_addr, attr->fe_config.tun2_addr);

      result = sony_tuner_r850_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, 0, &R850Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create R850 failed!\n");
        return MT_FAILURE;
      }
    }
    else if (attr->tuner_type == MT_UNF_TUNER_TYPE_R850)
    {
      tuner_addr = attr->tuner_addr;
      if (tuner_addr == 0)
        tuner_addr = 0xF8;

      printk("%s[%d] -- Select tuner R850, addr[%02x][%02x]\n", __FUNCTION__, __LINE__, tuner_addr, attr->tuner_addr);

      result = sony_tuner_r850_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, 0, &R850Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create R850 failed!\n");
        return MT_FAILURE;
      }
    }
    else if (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_R858C)
    {
      tuner_addr = attr->fe_config.tun2_addr;
      if (tuner_addr == 0)
        tuner_addr = 0x34;

      printk("%s[%d] -- Select tuner R858C, addr[%02x][%02x]\n", __FUNCTION__, __LINE__, tuner_addr, attr->fe_config.tun2_addr);

      result = sony_tuner_r858_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, gSonyIndex, &R858Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create R858C failed!\n");
        return MT_FAILURE;
      }
    }
    else if (attr->tuner_type == MT_UNF_TUNER_TYPE_R858C)
    {
      tuner_addr = attr->tuner_addr;
      if (tuner_addr == 0)
        tuner_addr = 0x34;

      printk("%s[%d] -- Select tuner R858C, addr[%02x][%02x]\n", __FUNCTION__, __LINE__, tuner_addr, tuner_addr);

      result = sony_tuner_r858_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, gSonyIndex, &R858Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create R858C failed!\n");
        return MT_FAILURE;
      }
    }
    else if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_R836) || 
             (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_RAFAEL836))
    {
      tuner_addr = attr->fe_config.tun2_addr;
      if (tuner_addr == 0)
        tuner_addr = 0x34;

      //printk("%s[%d] -- Select tuner R836, addr[%02x][%02x], dev_handle[%d]->tunerTerrCable = 0x%08x\n", __FUNCTION__, __LINE__, tuner_addr, attr->fe_config.tun2_addr, gSonyIndex, (ulong)&(dev_handle[gSonyIndex]->tuner));

      result = sony_tuner_r840_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, 0, &R840Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create R836 failed!\n");
        return MT_FAILURE;
      }

      bR858IntNum = 0;

      //printk("%s[%d] -- dev_handle[%d]->tunerTerrCable = [0x%08x]\n", __FUNCTION__, __LINE__, gSonyIndex, (unsigned int)&(dev_handle[gSonyIndex]->tuner));
    }
    else if ((attr->tuner_type == MT_UNF_TUNER_TYPE_R836) || 
             (attr->tuner_type == MT_UNF_TUNER_TYPE_RAFAEL836))
    {
      tuner_addr = attr->tuner_addr;
      if (tuner_addr == 0)
        tuner_addr = 0x34;

      //printk("%s[%d] -- Select tuner R836, addr[%02x][%02x], dev_handle[%d]->tunerTerrCable = 0x%08x\n", __FUNCTION__, __LINE__, tuner_addr, attr->tuner_addr, gSonyIndex, (ulong)&(dev_handle[gSonyIndex]->tuner));

      result = sony_tuner_r840_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, 0, &R840Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create R836 failed!\n");
        return MT_FAILURE;
      }

      bR858IntNum = 0;

      //printk("%s[%d] -- dev_handle[%d]->tunerTerrCable = [0x%08x]\n", __FUNCTION__, __LINE__, gSonyIndex, (unsigned int)&(dev_handle[gSonyIndex]->tunerTerrCable));
    }
    else if (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800)
    {
      tuner_addr = attr->tuner_addr;
      if (tuner_addr == 0)
        tuner_addr = 0xc6;

      printk("%s[%d] -- Select tuner TC6800, addr[%02x][%02x] SonyIndex:%d\n", __FUNCTION__, __LINE__, tuner_addr, attr->tuner_addr, gSonyIndex);
      //printk("%s[%d] -- Select tuner TC6800, addr[%02x][%02x], dev_handle[%d]->tunerTerrCable = 0x%08x\n", __FUNCTION__, __LINE__, tuner_addr, attr->tuner_addr, gSonyIndex, &(dev_handle[gSonyIndex]->tuner));

      result = sony_tuner_tc6800_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, 0, &TC6800Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create TC6800 failed!\n");
        return MT_FAILURE;
      }

      bR858IntNum = 0;

     // printk("%s[%d] -- dev_handle[%d]->tuner = [0x%08x]\n", __FUNCTION__, __LINE__, gSonyIndex, &(dev_handle[gSonyIndex]->tuner));
    }
    else if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800)
    {
      tuner_addr = attr->tuner_addr;
      if (tuner_addr == 0)
        tuner_addr = 0xc6;

      printk("%s[%d] -- Select tuner TC6800, addr[%02x][%02x],gSonyIndex:%d\n", __FUNCTION__, __LINE__, tuner_addr, attr->tuner_addr, gSonyIndex);
      //printk("%s[%d] -- Select tuner TC6800, addr[%02x][%02x], dev_handle[%d]->tunerTerrCable = 0x%08x\n", __FUNCTION__, __LINE__, tuner_addr, attr->tuner_addr, gSonyIndex, &(dev_handle[gSonyIndex]->tuner));

      result = sony_tuner_tc6800_Create(&dev_handle[gSonyIndex]->tuner, tuner_addr, &cxd_i2c, 0, 0, &TC6800Tuner);
      if (result != SONY_RESULT_OK)
      {
        printk("Create TC6800 failed!\n");
        return MT_FAILURE;
      }

      bR858IntNum = 0;

      //printk("%s[%d] -- dev_handle[%d]->tunerTerrCable = [0x%08x]\n", __FUNCTION__, __LINE__, gSonyIndex, &(dev_handle[gSonyIndex]->tunerTerrCable));
    }

    p_priv[gSonyIndex]->cfg.tun2_type = attr->tuner_type;
    p_priv[gSonyIndex]->cfg.tun2_addr = tuner_addr;

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
    if (enableLNBC)
    {
#if defined(SONY_EXAMPLE_LNBC_ALLEGRO_A8290)
      /* Use Allegro A8290 LNB controller */
      /* I2C slave address is 0x10 */
      result = allegro_a8290_Create(&dev_handle[gSonyIndex]->lnbc, 0x10, &dev_handle[gSonyIndex]->i2c);
      if (result != SONY_RESULT_OK)
      {
        return MT_FAILURE;
      }
#elif defined(SONY_EXAMPLE_LNBC_ST_LNBH23)
      /* Use ST LNBH23 LNB controller */
      /* I2C slave address is 0x10 */
      result = st_lnbh23_Create(&dev_handle[gSonyIndex]->lnbc, 0x10, &dev_handle[gSonyIndex]->i2c);
      if (result != SONY_RESULT_OK)
      {
        return MT_FAILURE;
      }
#else
      /* Use Intersil ISL9492 LNB controller */
      /* I2C slave address is 0x10 */
      result = intersil_isl9492_Create(&dev_handle[gSonyIndex]->lnbc, 0x10, &dev_handle[gSonyIndex]->i2c);
      if (result != SONY_RESULT_OK)
      {
        return MT_FAILURE;
      }
#endif
    }
#endif

    createParam[gSonyIndex].xtalFreq = SONY_DEMOD_XTAL_24000KHz;                   /* 24MHz Xtal */
    createParam[gSonyIndex].i2cAddressSLVT = attr->demod_addr;
    if (attr->demod_addr == 0)
      createParam[gSonyIndex].i2cAddressSLVT = 0xD8;                               /* Default I2C slave address is 0xD8 */

    createParam[gSonyIndex].tunerI2cConfig = SONY_DEMOD_TUNER_I2C_CONFIG_REPEATER; /* I2C repeater is used */

    /* Construct sony_integ_t and sony_demod_t instances */
    result = sony_integ_Create(&dev_handle[gSonyIndex]->integ, &dev_handle[gSonyIndex]->demod, &createParam[gSonyIndex], &cxd_i2c, &dev_handle[gSonyIndex]->tuner,
                               (enableLNBC ? &dev_handle[gSonyIndex]->lnbc : NULL));        /* If the driver need not to control LNBC, NULL can be used. */
    if (result != SONY_RESULT_OK)
    {
      return MT_FAILURE;
    }


    /*
     * Initialize devices. (demod, tuner, LNBC)
     * Following this call the driver will be in SONY_DEMOD_STATE_SLEEP state.
     * From here you can call any Sleep or Tune APIs.
     * Note : Initialize API should only be called once at the start of the driver creation.
     *        Subsequent calls to any of the Tune API's do not require re-initialize.
     */
    result = sony_integ_Initialize(&dev_handle[gSonyIndex]->integ);
    if (result != SONY_RESULT_OK)
    {
      return MT_FAILURE;
    }

    /*
     * In default, IF frequency is set for Sony silicon tuners.
     * If non-Sony tuner is used, the user should set the IF frequency like as follows.
     */
#if 1
    {
      sony_demod_iffreq_config_t iffreqConfig;

      iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.6
      iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.6
      iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 4.2
      iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 4.8

      iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.5
      iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 3.6
      iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 3.6
      iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 4.2
      iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 4.8

      iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);  // 3.7
      iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);  // 4.9
      iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);  // 4.9
      iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.7
      iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 4.9

      iffreqConfig.configISDBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.55);
      iffreqConfig.configISDBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.15);
      iffreqConfig.configISDBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.75);

      iffreqConfig.configISDBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
      iffreqConfig.configJ83B_5_06_5_36 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.7
      iffreqConfig.configJ83B_5_60 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);      // 3.75

      if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_R858C) || 
          (attr->tuner_type == MT_UNF_TUNER_TYPE_R858C))
      {
        R858_ExtTunerNum_Type ExtTunerNum;
        R858_IntTunerNum_Type IntTunerNum;

        result = sony_tuner_r858_get_tuner_num(&dev_handle[gSonyIndex]->tuner, &ExtTunerNum, &IntTunerNum);

        if (result == SONY_RESULT_OK)
        {
          if (IntTunerNum == 1)
          {
            bR858IntNum = 0;
#if 1
            iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 3.6
            iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 3.6
            iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 4.2
            iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 4.8

            iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(1.9); // 3.5
            iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 3.6
            iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 3.6
            iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 4.2
            iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 4.8
#else
            iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 3.6
            iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 3.6
            iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 4.2
            iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 4.8

            iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(1.9); // 3.5
            iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 3.6
            iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 3.6
            iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 4.2
            iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 4.8
#endif

            iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);  // 3.7
            iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);  // 4.9
            iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);  // 4.9
            iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 3.7
            iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 4.9

            iffreqConfig.configISDBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
            iffreqConfig.configISDBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
            iffreqConfig.configISDBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);

            iffreqConfig.configISDBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
            iffreqConfig.configJ83B_5_06_5_36 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.7
            iffreqConfig.configJ83B_5_60 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);      // 3.75

            printk("%s[%d] ---- Config demod IF 5.5MHz\n", __FUNCTION__, __LINE__);

            bR858IntNum = 1;
          }
          else
          {
            printk("%s[%d] ---- Config demod IF 5MHz\n", __FUNCTION__, __LINE__);

            bR858IntNum = 0;
          }
        }
      }

      //printk("%s[%d][%d] ---- dev_handle[gSonyIndex]->demod = 0x%08x, iffreqConfig = 0x%08x\n", __FUNCTION__, __LINE__, gSonyIndex, &dev_handle[gSonyIndex]->demod, &iffreqConfig);

      result = sony_demod_SetIFFreqConfig(&dev_handle[gSonyIndex]->demod, &iffreqConfig);
      if (result != SONY_RESULT_OK)
      {
        printk("sony_demod_SetIFFreqConfig failed. (%d)\n", result);
        return MT_FAILURE;
      }
    }
#endif

    /*
     * In default, The setting is optimized for Sony silicon tuners.
     * If non-Sony tuner is used, the user should call following to
     * disable Sony silicon tuner optimized setting.
     */
    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TUNER_OPTIMIZE, SONY_DEMOD_TUNER_OPTIMIZE_NONSONY);
    if (result != SONY_RESULT_OK)
    {
      return MT_FAILURE;
    }

#if 0
    /*
     * For Sony HELENE tuner, the satellite IFAGC setting should be changed.
     * If non-Sony tuner is used, the user should do this setting depend on the IFAGC sense of the tuner.
     */
    result = sony_demod_SetConfig (&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_SAT_IFAGCNEG, 1);
    if (result != SONY_RESULT_OK) {
        return result;
    }
#endif

    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGCNEG, 0);
    // result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGCNEG, bR858IntNum);
    //result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGCNEG, bR858IntNum ? 1 : 0);
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_IFAGCNEG) failed. (%d)\n", result);
      return MT_FAILURE;
    }

    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_PARALLEL_SEL, (attr->output_mode < MT_UNF_FE_OUTPUT_MODE_SERIAL) ? 1 : 0);
    if (result != SONY_RESULT_OK)
    {
      printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_PARALLEL_SEL) failed. (%d)\n", result);
      return MT_FAILURE;
    }
    else
    {
      printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_PARALLEL_SEL -- [%d]) OK. (%d)\n", (attr->output_mode < MT_UNF_FE_OUTPUT_MODE_SERIAL) ? 1 : 0, result);
    }

    p_priv[gSonyIndex]->cfg.ts_mode = attr->output_mode;

#if 0
    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ, 5);
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ) failed. (%d)\n", result);
      return result;
    }

    //printk("%s[%d] ---- Set TS %s edge\n", __FUNCTION__, __LINE__, (s_sony_cxd2856_cfg[gSonyIndex].udvbt_tuner == NIM_SONY_CXD2856_TUNER_TC6800) ? "falling" : "rising");

    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSSYNC_ACTIVE_HI, (s_sony_cxd2856_cfg[gSonyIndex].udvbt_tuner == NIM_SONY_CXD2856_TUNER_TC6800) ? 0 : 1);
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSSYNC_ACTIVE_HI) failed. (%d)\n", result);
      return ERR_FAILURE;
    }

    //printk("%s[%d] ---- Set TS sync sync %s\n", __FUNCTION__, __LINE__, (s_sony_cxd2856_cfg[gSonyIndex].udvbt_tuner == NIM_SONY_CXD2856_TUNER_TC6800) ? "low" : "high");
#endif

#if 0
    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ, 4);
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ) failed. (%d)\n", result);
      return result;
    }
#endif

    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_SER_DATA_ON_MSB, 0); // Serial TS Data0 outout
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_SER_DATA_ON_MSB) failed. (%d)\n", result);
      return MT_FAILURE;
    }

#if 0
    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_CURRENT, 1); // TS CLK current
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CURRENT) failed. (%d)\n", result);
      return MT_FAILURE;
    }
#endif

    if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_R850) || 
        (attr->tuner_type == MT_UNF_TUNER_TYPE_R850) || 
        (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800) || 
        (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800))
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_CONT, 0);	// Serial TS CLK gated outout

      if (result != SONY_RESULT_OK)
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CONT -- [0]) failed. (%d)\n", result);
        return result;
      }
      else
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CONT -- [0]) OK. (%d)\n", result);
      }
    }
    else
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_CONT, 1);	// Serial TS CLK continuous outout(default)

      if (result != SONY_RESULT_OK)
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CONT -- [1]) failed. (%d)\n", result);
        return result;
      }
      else
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CONT -- [1]) OK. (%d)\n", result);
      }
    }

    if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800) || 
        (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800))
    {
#if 0
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_CONT, 0);	// Serial TS CLK gated outout
      if (result != SONY_RESULT_OK)
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CONT -- [0]) failed. (%d)\n", result);
        return result;
      }
	  else
	  {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CONT -- [0]) OK. (%d)\n", result);
	  }
#endif

      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_MASK, 3);	// Serial TS CLK always active(default)
      if (result != SONY_RESULT_OK)
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_MASK -- [3]) failed. (%d)\n", result);
        return result;
      }
	  else
	  {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_MASK -- [3]) OK. (%d)\n", result);
	  }

      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSVALID_MASK, 1);	// Serial TS Valid disabled during TS packet gap & TS parity (default)
      if (result != SONY_RESULT_OK)
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSVALID_MASK -- [1]) failed. (%d)\n", result);
        return result;
      }
	  else
	  {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSVALID_MASK -- [1]) OK. (%d)\n", result);
	  }

      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSERR_MASK, 0);	// Serial TS Valid disabled during TS packet gap & TS parity (default)
      if (result != SONY_RESULT_OK)
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSERR_MASK -- [0]) failed. (%d)\n", result);
        return result;
      }
	  else
	  {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSERR_MASK -- [0]) OK. (%d)\n", result);
	  }
      /*fix issue 26295 and 26072*/
      /*When 256QAM 5/6,the ts clk should be lager than 48MHz*/
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ, 1);
      if (result != SONY_RESULT_OK)
      {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ -- [4]) failed. (%d)\n", result);
        return result;
      }
	  else
	  {
        printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ -- [4]) OK. (%d)\n", result);
	  }
	}

#if 0	// 20240117
    if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800) || 
        (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800))
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_MASK, 3);	// Serial TS CLK disabled during TS packet gap
    }
    else
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_MASK, 0);	// Serial TS CLK always active(default)
    }

    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_MASK) failed. (%d)\n", result);
      return result;
    }
#endif

#if 0
    if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800) || 
        (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800))
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSVALID_MASK, 1);	// Serial TS Valid disabled during TS packet gap & TS parity (default)
    }
    else
    {
      //result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSVALID_MASK, 0);	// Serial TS Valid always active(default)
    }
#endif


#if 0 // Trigon Serial TS configurations
    if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800) || 
        (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800))
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_MASK, 1);	// Serial TS CLK disabled during TS packet gap
    }
    else
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_MASK, 0);	// Serial TS CLK always active(default)
    }

    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_MASK) failed. (%d)\n", result);
      return result;
    }

    if ((attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800) || 
        (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800))
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSVALID_MASK, 3);	// Serial TS Valid disabled during TS packet gap & TS parity (default)
    }
    else
    {
      //result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSVALID_MASK, 0);	// Serial TS Valid always active(default)
    }

    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSVALID_MASK) failed. (%d)\n", result);
      return result;
    }
#endif

#if 0
    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TS_PACKET_GAP, 0);	// Serial TS GAP 0
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TS_PACKET_GAP) failed. (%d)\n", result);
      return result;
    }
#endif

    if (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_R850)
    {
        result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 0);
    }
    else
    {
        result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 1);
    }

    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_IFAGC_ADC_FS) failed. (%d)\n", result);
      return MT_FAILURE;
    }
    else
	{
      printk(KERN_ERR "sony_demod_SetConfig (SONY_DEMOD_CONFIG_IFAGC_ADC_FS -- [4]) OK. (%d)\n", result);
	}

    /*
     * Configuration
     * If additional configuration is necessary, please use sony_demod_SetConfig here.
     * TS format, BER measurement period etc settings can be changed.
     * In detail, please check sony_demod.h, sony_demod_config_id_t definition.
     */

#if 0
    /* TS clock inversion setting */
    result = sony_demod_SetConfig (pTunerDemod, SONY_TUNERDEMOD_CONFIG_LATCH_ON_POSEDGE, 0);
    if (result != SONY_RESULT_OK) {
        return -1;
    }
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBT2
    if (attr->fe_config.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800)
    {
#if 1 // Enable T2 FEF
      result = sony_demod_GPIOSetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_GPIO_PIN_GPIO1, 1, SONY_DEMOD_GPIO_MODE_FEF_PART);
      if (result != SONY_RESULT_OK)
      {
        printk("sony_demod_GPIOSetConfig (SONY_DEMOD_GPIO_PIN_GPIO1, SONY_DEMOD_GPIO_MODE_FEF_PART) failed. (%d)\n", result);
        return ERR_FAILURE;
      }

      //printk("%s[%d] ---- Set GPIO1 to T2FEF mode\n", __FUNCTION__, __LINE__);
#endif

#if 0
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_GPIO0_CURRENT, 3); // Set GPIO0 current to 10mA
      if (result != SONY_RESULT_OK)
      {
        printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_GPIO0_CURRENT) failed. (%d)\n", result);
        return MT_FAILURE;
      }

      //printk("%s[%d] ---- Set GPIO0(T2FEF) current to 10mA\n", __FUNCTION__, __LINE__);
#endif

      result = sony_demod_GPIOSetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_GPIO_PIN_GPIO2, 1, SONY_DEMOD_GPIO_MODE_TS_OUTPUT);
      if (result != SONY_RESULT_OK)
   	  {
        printk("sony_demod_GPIOSetConfig (SONY_DEMOD_GPIO_PIN_GPIO2, SONY_DEMOD_GPIO_MODE_TS_OUTPUT) failed. (%d)\n", result);
        return ERR_FAILURE;
      }

      //printk("%s[%d] ---- Set GPIO2 to TSERR mode\n", __FUNCTION__, __LINE__);
    }
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
    if (enableLNBC)
    {
      /*
       * Configure demodulator and LNB controller.
       * It's depend on LNB controller's specification.
       * (Tone generation is supported or not.)
       */

      /*
       * Configure demod to output PWM.
       * The user can select envelope mode too.
       */
      result = sony_demod_sat_device_ctrl_DSQOUTSetting(&dev_handle[gSonyIndex]->demod, SONY_DSQOUT_MODE_PWM, 22);
      if (result != SONY_RESULT_OK)
      {
        return MT_FAILURE;
      }

      /*
       * Configure LNB controller not to generate tone by LNBC internally.
       * This setting should be synchronized to above demod setting.
       */
      result = dev_handle[gSonyIndex]->lnbc.SetConfig(&dev_handle[gSonyIndex]->lnbc, SONY_LNBC_CONFIG_ID_TONE_INTERNAL, 0);
      if (result != SONY_RESULT_OK)
      {
        return MT_FAILURE;
      }
    }
  }

#if 0
  {
    //mt_fe_dmd_register_notify_sony_cxd2856_ss2(port_sony_cxd2856_register_to_drv_notify);

    if (dvbs_nl_family.id > 0)
    {
      ret = genl_unregister_family(&dvbs_nl_family);
    }

    ret = genl_register_family(&sony_cxd2856_nl_family);
    if (ret)
    {
      //printk("genl_register_family_with_ops failed, ret = 0x%08x\n", ret);
      kfree(dev_handle[gSonyIndex]);
      kfree((void *)p_priv[gSonyIndex]);
      if (sony_cxd2856_pg_channel_info[gSonyIndex])
      {
        kfree(sony_cxd2856_pg_channel_info[gSonyIndex]);
        sony_cxd2856_pg_channel_info[gSonyIndex] = NULL;
      }

      g_sony_cxd2856_priv[gSonyIndex] = NULL;

      //printk("%s[%d] ---- genl_register_family() failed\n", __FUNCTION__, __LINE__);

      return -EINVAL;
    }

    memcpy(&dvbs_nl_family, &sony_cxd2856_nl_family, sizeof(sony_cxd2856_nl_family));

    //printk("sony_cxd2856_nl_family.id = 0x%x\n", sony_cxd2856_nl_family.id);
  }
#endif
#endif

  info->is_attach = 1;
  info->pre_attr.demod_dev_type = attr->demod_dev_type;
  bInitialized_SONY_CXD[gSonyIndex] = 1;

  gFeCount[gSonyIndex]++;

  return MT_SUCCESS;
}

int sony_cxd_detach(frontend_info_s *info)
{
  //mt_fe_sony_cxd_priv_handle p_priv[gSonyIndex] = NULL;
  //sony_example_driver_instance_t * dev_handle[gSonyIndex] = NULL;
	mt_fe_sony_cxd_priv_handle priv = NULL;
    sony_example_driver_handle sony_cxd_handle = NULL;
	if (NULL == info)
	{
		printk(KERN_ERR "[%s %d]error, info is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}
	priv = (mt_fe_sony_cxd_priv_handle)(info->handle);
	if (NULL == priv)
	{
		printk(KERN_ERR "[%s %d]error, p_priv is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}
    sony_cxd_handle = priv->sony_cxd_handle;
    if (NULL == sony_cxd_handle)
	{
		printk(KERN_ERR "[%s %d]error, sony_cxd2856_handle is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}
    sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);
    gFeCount[gSonyIndex]--;
    if ((info->is_attach) &&(gFeCount[gSonyIndex] == 0))
    {

        if(priv->scan_info.p_channel_info)
        {
           vfree(priv->scan_info.p_channel_info);
           priv->scan_info.p_channel_info = NULL;
           sony_cxd_pg_channel_info[gSonyIndex] = NULL;
        }

        vfree(sony_cxd_handle);
        sony_cxd_handle = NULL;
        dev_handle[gSonyIndex] = NULL;

        vfree(priv);
        priv = NULL;
        p_priv[gSonyIndex] = NULL;
#if 0
        p_priv[gSonyIndex] = info->handle;
        dev_handle[gSonyIndex] = p_priv[gSonyIndex]->sony_cxd2856_handle;

        if (p_priv[gSonyIndex]->bs_info.p_tp_info)
        {
          kfree(p_priv[gSonyIndex]->bs_info.p_tp_info);
          p_priv[gSonyIndex]->bs_info.p_tp_info = NULL;
        }

        kfree(dev_handle[gSonyIndex]);
        kfree((void *)p_priv[gSonyIndex]);
        if (sony_cxd_pg_channel_info[gSonyIndex])
        {
          kfree(sony_cxd_pg_channel_info[gSonyIndex]);
          sony_cxd_pg_channel_info = NULL;
        }
#endif

        info->is_attach = 0;
        bInitialized_SONY_CXD[gSonyIndex] = 0;
    }

  //sony_cxd2856_MxL608_GPIO_power_off(GPIO_15);

  

#if 0
  g_sony_cxd2856_priv[gSonyIndex] = NULL;

  genl_unregister_family(&sony_cxd2856_nl_family);
#endif

  return MT_SUCCESS;
}

//int sony_cxd_resume(frontend_info_s *info, mt_unf_fe_attr_t *attr)
//int sony_cxd_resume(frontend_info_s *info)
int sony_cxd_resume(void *handle)
{
  //uint8_t reg_val = 0;
  /*mt_u32 tmp = 0;clean warning unused variable*/
  sony_result_t result = SONY_RESULT_OK;
  int ret = MT_FAILURE;
  int bR858IntNum = 0;

  /*uint8_t enableSatTuner = 0;clean warning unused variable*/
  uint8_t enableLNBC = 0;
  /*uint8_t tuner_addr = 0xC0; clean warning unused variable*/

  //mt_fe_sony_cxd_priv_handle priv_handle = (mt_fe_sony_cxd2856_priv_handle)info->handle;
  mt_fe_sony_cxd_priv_handle priv_handle = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = priv_handle->sony_cxd_handle;

  //sony_cxd_MxL608_GPIO_power_on(GPIO_15);
 
  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //printk("%s[%d] ---- gSonyIndex = %d\n", __FUNCTION__, __LINE__, gSonyIndex);

  if (bInitialized_SONY_CXD[gSonyIndex])
  {
    return MT_SUCCESS;
  }
  /*fix issue30833*/
  i2c_isr_onoff(g_i2c_sony_cxd,  0);

  //printk("%s[%d] ---- FE[%d] type = %d, tuner_type[%d], tuner_addr[0x%02x], tun2_type[%d], tun2_addr[0x%02x]\n", __FUNCTION__, __LINE__, gFeIndex, gFeList[gFeIndex], 
  //        attr->tuner_type, attr->tuner_addr, attr->fe_config.tun2_type, attr->fe_config.tun2_addr);

  if (bInitialized_SONY_CXD[gSonyIndex] == 0)
  {


#if 0
    printk("%s[%d] -- dev_handle[%d]->integ = [0x%08x], dev_handle[%d]->demod = [0x%08x], createParam[%d] = [0x%08x], dev_handle[%d]->tuner = [0x%08x]\n", 
            __FUNCTION__, __LINE__, 
            gSonyIndex, (unsigned int)&(dev_handle[gSonyIndex]->integ), 
            gSonyIndex, (unsigned int)&(dev_handle[gSonyIndex]->demod), 
            gSonyIndex, (unsigned int)&(createParam[gSonyIndex]), 
            gSonyIndex, (unsigned int)&(dev_handle[gSonyIndex]->tuner));
#endif

    /*
     * Initialize devices. (demod, tuner, LNBC)
     * Following this call the driver will be in SONY_DEMOD_STATE_SLEEP state.
     * From here you can call any Sleep or Tune APIs.
     * Note : Initialize API should only be called once at the start of the driver creation.
     *        Subsequent calls to any of the Tune API's do not require re-initialize.
     */
    result = sony_integ_Initialize(&dev_handle[gSonyIndex]->integ);
    if (result != SONY_RESULT_OK)
    {
      //printk("%s[%d] ---- sony_integ_Initialize() failed, integ = 0x%08x\n", __FUNCTION__, __LINE__, (unsigned int)&dev_handle[gSonyIndex]->integ);
      ret =  MT_FAILURE;
	  goto err;
    }

    /*
     * In default, IF frequency is set for Sony silicon tuners.
     * If non-Sony tuner is used, the user should set the IF frequency like as follows.
     */
#if 1
    {
      sony_demod_iffreq_config_t iffreqConfig;

      iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.6
      iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.6
      iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 4.2
      iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 4.8

      iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.5
      iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 3.6
      iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 3.6
      iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 4.2
      iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);   // 4.8

      iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);  // 3.7
      iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);  // 4.9
      iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);  // 4.9
      iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.7
      iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 4.9

      iffreqConfig.configISDBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.55);
      iffreqConfig.configISDBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.15);
      iffreqConfig.configISDBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.75);

      iffreqConfig.configISDBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
      iffreqConfig.configJ83B_5_06_5_36 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.7
      iffreqConfig.configJ83B_5_60 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);      // 3.75

      if ((priv_handle->cfg.tun2_type == MT_UNF_TUNER_TYPE_R858C))
      {
        R858_ExtTunerNum_Type ExtTunerNum;
        R858_IntTunerNum_Type IntTunerNum;

        result = sony_tuner_r858_get_tuner_num(&dev_handle[gSonyIndex]->tuner, &ExtTunerNum, &IntTunerNum);

        if (result == SONY_RESULT_OK)
        {
          if (IntTunerNum == 1)
          {
            bR858IntNum = 0;
#if 1
            iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 3.6
            iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 3.6
            iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 4.2
            iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 4.8

            iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(1.9); // 3.5
            iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 3.6
            iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 3.6
            iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 4.2
            iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);   // 4.8
#else
            iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 3.6
            iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 3.6
            iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 4.2
            iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57); // 4.8

            iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(1.9); // 3.5
            iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 3.6
            iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 3.6
            iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 4.2
            iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.57);   // 4.8
#endif

            iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);  // 3.7
            iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);  // 4.9
            iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5);  // 4.9
            iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 3.7
            iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5.5); // 4.9

            iffreqConfig.configISDBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
            iffreqConfig.configISDBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
            iffreqConfig.configISDBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);

            iffreqConfig.configISDBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
            iffreqConfig.configJ83B_5_06_5_36 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5); // 3.7
            iffreqConfig.configJ83B_5_60 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);      // 3.75

            printk("%s[%d] ---- Config demod IF 5.5MHz\n", __FUNCTION__, __LINE__);

            bR858IntNum = 1;
          }
          else
          {
            printk("%s[%d] ---- Config demod IF 5MHz\n", __FUNCTION__, __LINE__);

            bR858IntNum = 0;
          }
        }
      }

      //printk("%s[%d][%d] ---- dev_handle[gSonyIndex]->demod = 0x%08x, iffreqConfig = 0x%08x\n", __FUNCTION__, __LINE__, gSonyIndex, &dev_handle[gSonyIndex]->demod, &iffreqConfig);

      result = sony_demod_SetIFFreqConfig(&dev_handle[gSonyIndex]->demod, &iffreqConfig);
      if (result != SONY_RESULT_OK)
      {
        printk("sony_demod_SetIFFreqConfig failed. (%d)\n", result);
        ret =  MT_FAILURE;
	  	goto err;
      }
    }
#endif

    /*
     * In default, The setting is optimized for Sony silicon tuners.
     * If non-Sony tuner is used, the user should call following to
     * disable Sony silicon tuner optimized setting.
     */
    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TUNER_OPTIMIZE, SONY_DEMOD_TUNER_OPTIMIZE_NONSONY);
    if (result != SONY_RESULT_OK)
    {
      printk("%s[%d] ---- sony_demod_SetConfig() SONY_DEMOD_CONFIG_TUNER_OPTIMIZE failed\n", __FUNCTION__, __LINE__);
      ret =  MT_FAILURE;
	  goto err;
    }

#if 0
    /*
     * For Sony HELENE tuner, the satellite IFAGC setting should be changed.
     * If non-Sony tuner is used, the user should do this setting depend on the IFAGC sense of the tuner.
     */
    result = sony_demod_SetConfig (&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_SAT_IFAGCNEG, 1);
    if (result != SONY_RESULT_OK) {
        return result;
    }
#endif

    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGCNEG, 0);
    // result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGCNEG, bR858IntNum);
    //result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGCNEG, bR858IntNum ? 1 : 0);
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_IFAGCNEG) failed. (%d)\n", result);
      ret =  MT_FAILURE;
	  goto err;
    }

    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_PARALLEL_SEL, (priv_handle->cfg.ts_mode < MT_UNF_FE_OUTPUT_MODE_SERIAL) ? 1 : 0);
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_PARALLEL_SEL) failed. (%d)\n", result);
      ret =  MT_FAILURE;
	  goto err;
    }

    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_SER_DATA_ON_MSB, 0); // Serial TS Data0 outout
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_SER_DATA_ON_MSB) failed. (%d)\n", result);
      ret =  MT_FAILURE;
	  goto err;
    }

#if 0
    result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_CURRENT, 1); // TS CLK current
    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CURRENT) failed. (%d)\n", result);
      return MT_FAILURE;
    }
#endif

    if (priv_handle->cfg.tun2_type == MT_UNF_TUNER_TYPE_R850)
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_CONT, 0);	// Serial TS CLK gated outout
    }
    else
    {
      result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_TSCLK_CONT, 1);	// Serial TS CLK continuous outout(default)
    }

    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TSCLK_CONT) failed. (%d)\n", result);
      ret =  MT_FAILURE;
	  goto err;
    }

    if (priv_handle->cfg.tun2_type == MT_UNF_TUNER_TYPE_R850)
    {
        result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 0);
    }
    else
    {
        result = sony_demod_SetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 1);
    }

    if (result != SONY_RESULT_OK)
    {
      printk("sony_demod_SetConfig (SONY_DEMOD_CONFIG_IFAGC_ADC_FS) failed. (%d)\n", result);
      ret =  MT_FAILURE;
	  goto err;
    }

    /*
     * Configuration
     * If additional configuration is necessary, please use sony_demod_SetConfig here.
     * TS format, BER measurement period etc settings can be changed.
     * In detail, please check sony_demod.h, sony_demod_config_id_t definition.
     */

#if 0
    /* TS clock inversion setting */
    result = sony_demod_SetConfig (pTunerDemod, SONY_TUNERDEMOD_CONFIG_LATCH_ON_POSEDGE, 0);
    if (result != SONY_RESULT_OK) {
        return -1;
    }
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBT2
    if (priv_handle->cfg.tun2_type == MT_UNF_TUNER_TYPE_M88TC6800)
    {
#if 1 // Enable T2 FEF
      result = sony_demod_GPIOSetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_GPIO_PIN_GPIO1, 1, SONY_DEMOD_GPIO_MODE_FEF_PART);
      if (result != SONY_RESULT_OK)
      {
        printk("sony_demod_GPIOSetConfig (SONY_DEMOD_GPIO_PIN_GPIO1, SONY_DEMOD_GPIO_MODE_FEF_PART) failed. (%d)\n", result);
        ret =  MT_FAILURE;
	  	goto err;
      }
#endif

      result = sony_demod_GPIOSetConfig(&dev_handle[gSonyIndex]->demod, SONY_DEMOD_GPIO_PIN_GPIO2, 1, SONY_DEMOD_GPIO_MODE_TS_OUTPUT);
      if (result != SONY_RESULT_OK)
   	  {
        printk("sony_demod_GPIOSetConfig (SONY_DEMOD_GPIO_PIN_GPIO2, SONY_DEMOD_GPIO_MODE_TS_OUTPUT) failed. (%d)\n", result);
        ret =  MT_FAILURE;
	  	goto err;
      }

      //printk("%s[%d] ---- Set GPIO2 to TSERR mode\n", __FUNCTION__, __LINE__);
    }
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
    if (enableLNBC)
    {
      /*
       * Configure demodulator and LNB controller.
       * It's depend on LNB controller's specification.
       * (Tone generation is supported or not.)
       */

      /*
       * Configure demod to output PWM.
       * The user can select envelope mode too.
       */
      result = sony_demod_sat_device_ctrl_DSQOUTSetting(&dev_handle[gSonyIndex]->demod, SONY_DSQOUT_MODE_PWM, 22);
      if (result != SONY_RESULT_OK)
      {
        ret =  MT_FAILURE;
	  	goto err;
      }

      /*
       * Configure LNB controller not to generate tone by LNBC internally.
       * This setting should be synchronized to above demod setting.
       */
      result = dev_handle[gSonyIndex]->lnbc.SetConfig(&dev_handle[gSonyIndex]->lnbc, SONY_LNBC_CONFIG_ID_TONE_INTERNAL, 0);
      if (result != SONY_RESULT_OK)
      {
        ret =  MT_FAILURE;
	  	goto err;
      }
    }
#endif
  }

  //printk("%s[%d] ---- handle = 0x%08x\n", __FUNCTION__, __LINE__, (unsigned int)handle);

  bInitialized_SONY_CXD[gSonyIndex] = 1;
  ret = MT_SUCCESS;
err:
  i2c_isr_onoff(g_i2c_sony_cxd,  1);
  return ret;
}

//int sony_cxd_suspend(frontend_info_s *info)
int sony_cxd_suspend(void *handle)
{
  //mt_fe_sony_cxd_priv_handle priv_handle = (mt_fe_sony_cxd_priv_handle)info->handle;
  mt_fe_sony_cxd_priv_handle priv_handle = (mt_fe_sony_cxd_priv_handle)handle;
  sony_example_driver_handle sony_cxd_handle = priv_handle->sony_cxd_handle;

  //printk("%s[%d] ---- handle = 0x%08x\n", __FUNCTION__, __LINE__, (unsigned int)handle);

  sony_cxd_get_sony_index_by_addr(sony_cxd_handle->demod.i2cAddressSLVT);

  //sony_cxd2856_MxL608_GPIO_power_off(GPIO_15);

  bInitialized_SONY_CXD[gSonyIndex] = 0;

  return MT_SUCCESS;
}

#endif
